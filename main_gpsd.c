# 作者: Teddy
# 邮箱: teddy@pixmoving.net
# 日期: 2025-12-20
# 版本: V1.0
# 描述:
#  1. 针对优控的ADCU A06 Orin 平台，接入了中国移动的 RTK差分服务,并使用SDK进行访问;
#  2. 该程序用于接收从Orin发送的RTCM数据,并将其写入到GNSS串口(/dev/ttyTHS0);
#  3. 同时,该程序还可以接收从GNSS设备(/dev/ttyTHS0)发送的RTCM数据,并将其打印到终端;
#  4. 该程序支持动态切换RTCM数据的发送和接收;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <gps.h>
#include <fcntl.h>
#include <termios.h>
#include <stdbool.h>
#include <math.h>

#include "libapi.h"

// 全局变量用于信号处理
static volatile bool running = true;
static volatile int rtk_authenticated = 0; // RTK认证状态: 0=未认证, 1=已认证(状态2001)
static int serial_fd = -1;   // 串口文件描述符，用于发送RTCM数据到GNSS设备(/dev/ttyTHS0)
static int serial_fd_ths1 = -1; // 串口文件描述符，用于接收从Orin发送的RTCM数据(/dev/ttyTHS1)

void signal_handler(int sig) {
    running = false;
    printf("\n收到信号 %d，正在关闭程序...\n", sig);
}

void printRtcmData(void *data, int length) {
    if (data && length > 0) {
        // 输出RTCM数据接收日志
        static int rtcm_count = 0;
        rtcm_count++;
        printf("[RTCM-%04d] 收到差分数据，长度=%d 字节\n", rtcm_count, length);
        
        // 将RTCM数据写入到GNSS串口(/dev/ttyTHS0)
        if (serial_fd != -1) {
            int write_len = write(serial_fd, data, length);
            if (write_len == length) {
                printf("[RTCM-%04d] 已成功写入GNSS串口(/dev/ttyTHS0)，字节数=%d\n", rtcm_count, write_len);
            } else {
                printf("[RTCM-%04d] 写入GNSS串口(/dev/ttyTHS0)失败，写入字节数=%d，预期=%d\n", rtcm_count, write_len, length);
            }
        } else {
            printf("[RTCM-%04d] GNSS串口(/dev/ttyTHS0)未打开，无法写入数据\n", rtcm_count);
        }
    }
}

void printEpheData(void *data, int length) {
    // 星历数据回调
    printf("收到星历数据，长度=%d\n", length);
}

// 初始化串口函数
int init_serial_port(const char *port, int baudrate) {
    int fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        perror("打开串口失败");
        return -1;
    }

    struct termios options;
    tcgetattr(fd, &options);

    // 设置波特率
    speed_t speed;
    switch (baudrate) {
        case 9600:
            speed = B9600;
            break;
        case 19200:
            speed = B19200;
            break;
        case 38400:
            speed = B38400;
            break;
        case 57600:
            speed = B57600;
            break;
        case 115200:
            speed = B115200;
            break;
        case 230400:
            speed = B230400;
            break;
        case 460800:
            speed = B460800;
            break;
        case 921600:
            speed = B921600;
            break;
        default:
            fprintf(stderr, "不支持的波特率: %d\n", baudrate);
            close(fd);
            return -1;
    }

    cfsetispeed(&options, speed);
    cfsetospeed(&options, speed);

    // 设置数据位: 8位
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    // 设置停止位: 1位
    options.c_cflag &= ~CSTOPB;

    // 设置校验位: 无
    options.c_cflag &= ~PARENB;

    // 设置硬件流控制: 无
    options.c_cflag &= ~CRTSCTS;

    // 设置软件流控制: 无
    options.c_iflag &= ~(IXON | IXOFF | IXANY);

    // 设置为原始模式
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;

    // 设置等待时间和最小接收字符数
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 10; // 1秒超时

    // 刷新端口
    tcflush(fd, TCIFLUSH);

    // 应用新的配置
    if (tcsetattr(fd, TCSANOW, &options) != 0) {
        perror("设置串口属性失败");
        close(fd);
        return -1;
    }

    printf("串口 %s 初始化成功，波特率: %d\n", port, baudrate);
    return fd;
}

void printfStatusData(int status) {
    printf("RTK状态: %d\n", status);
    
    // 处理各种状态码
    switch(status) {
        case 1999:
            printf("RTK SDK初始化成功\n");
            break;
        case 2000:
            printf("RTK SDK启动成功\n");
            break;
        case 2001:
            rtk_authenticated = 1;
            printf("RTK认证成功！可以开始发送GGA数据。\n");
            break;
        case 3001:
            printf("网络错误\n");
            break;
        case 3002:
            printf("账户错误\n");
            break;
        case 3003:
            printf("账户已过期\n");
            break;
        case 3005:
            printf("账户需要激活\n");
            break;
        case 3007:
            printf("服务停止\n");
            break;
        case 3008:
            printf("无效账户\n");
            break;
        case 3010:
            printf("账户ID错误\n");
            break;
        case 3012:
            printf("账户被禁用\n");
            break;
        case 3016:
            printf("挂载点无效\n");
            break;
        case 3017:
            printf("失败次数过多\n");
            break;
        case 3018:
            printf("不在服务区域\n");
            break;
        case 3019:
            printf("服务区域无RTCM数据\n");
            break;
        case 3020:
            printf("DNS解析错误\n");
            break;
        case 3021:
            printf("时间池绑定失败\n");
            break;
        case 3022:
            printf("账户登录RPC失败\n");
            break;
        case 3023:
            printf("时间池时间超限\n");
            break;
        case 3024:
            printf("时间池不存在\n");
            break;
        case 3025:
            printf("登录类型错误\n");
            break;
        case 3026:
            printf("账户无服务期限\n");
            break;
        case 3027:
            printf("账户无挂载点\n");
            break;
        case 3028:
            printf("已连接\n");
            break;
        case 3029:
            printf("超过节点最大容量\n");
            break;
        case 3030:
            printf("CORS认证失败\n");
            break;
        case 3031:
            printf("设备ID错误\n");
            break;
        case 3032:
            printf("终端错误\n");
            break;
        case 3033:
            printf("认证类型错误\n");
            break;
        case 3034:
            printf("加密类型错误\n");
            break;
        case 3035:
            printf("CORS连接失败\n");
            break;
        case 3036:
            printf("CORS登录失败\n");
            break;
        case 3037:
            printf("制造商错误\n");
            break;
        case 3038:
            printf("超过池最大容量\n");
            break;
        case 3039:
            printf("品牌错误\n");
            break;
        case 3040:
            printf("产品错误\n");
            break;
        case 3041:
            printf("账户类型错误\n");
            break;
        case 3042:
            printf("登录参数错误\n");
            break;
        case 3043:
            printf("GGA无会话错误\n");
            break;
        case 3044:
            printf("账户池绑定失败\n");
            break;
        case 3045:
            printf("账户池不存在\n");
            break;
        case 3046:
            printf("池不存在\n");
            break;
        case 3047:
            printf("池不允许登录\n");
            break;
        case 3048:
            printf("授权无效\n");
            break;
        case 3049:
            printf("无效的框架或历元\n");
            break;
        case 3054:
            printf("账户踢出错误\n");
            break;
        case 3099:
            printf("未知错误\n");
            break;
        case 3100:
            printf("空账户\n");
            break;
        case 3101:
            printf("无效参数\n");
            break;
        case 3102:
            printf("服务器不可用\n");
            break;
        case 3103:
            printf("登录异常\n");
            break;
        case 3104:
            printf("账户无过期时间\n");
            break;
        case 3105:
            printf("在其他地方登录\n");
            break;
        case 3106:
            printf("业务支持错误\n");
            break;
        case 3107:
            printf("登录限制\n");
            break;
        case 3108:
            printf("获取路由失败\n");
            break;
        case 3109:
            printf("账户未激活\n");
            break;
        case 3110:
            printf("账户被禁用\n");
            break;
        case 3111:
            printf("服务时间用完\n");
            break;
        case 3112:
            printf("AppKey与设备ID不匹配\n");
            break;
        case 5001:
            printf("SDK启动失败\n");
            break;
        default:
            printf("未知状态码: %d\n", status);
            break;
    }
}

void printLogData(void* buf, int length) {
    printf("%s", (char*)buf);
}

// 将度分格式转换为度
double dm_to_degrees(double dm) {
    int degrees = (int)(dm / 100);
    double minutes = dm - degrees * 100;
    return degrees + minutes / 60.0;
}

// 获取定位质量
const char* get_quality_description(int quality) {
    switch (quality) {
        case 0: return "无效定位";
        case 1: return "GPS定位";
        case 2: return "DGPS定位";
        case 3: return "PPS定位";
        case 4: return "RTK固定解";
        case 5: return "RTK浮点解";
        case 6: return "航位推算";
        case 7: return "手动输入模式";
        case 8: return "模拟模式";
        default: return "未知定位质量";
    }
}

// 生成GGA字符串 - 改进版，更符合实际设备输出格式
int generate_gga_string(struct gps_data_t *gpsdata, char *gga_buffer, size_t buffer_size) {
    if (gpsdata->fix.mode < MODE_2D) {
        return -1; // 没有有效定位
    }
    
    // 获取当前时间（UTC时间）
    time_t now = time(NULL);
    struct tm *tm_info = gmtime(&now); // 使用UTC时间，因为GGA数据使用UTC
    
    // 纬度转换：度 -> 度分格式
    double lat_deg = fabs(gpsdata->fix.latitude);
    int lat_deg_int = (int)lat_deg;
    double lat_min = (lat_deg - lat_deg_int) * 60.0;
    char lat_dir = (gpsdata->fix.latitude >= 0) ? 'N' : 'S';
    
    // 经度转换：度 -> 度分格式
    double lon_deg = fabs(gpsdata->fix.longitude);
    int lon_deg_int = (int)lon_deg;
    double lon_min = (lon_deg - lon_deg_int) * 60.0;
    char lon_dir = (gpsdata->fix.longitude >= 0) ? 'E' : 'W';
    
    // 定位质量指示器 - 映射GPSD status到NMEA quality
    int quality = 0;
    if (gpsdata->fix.mode >= MODE_2D) {
        // 使用gps_fix_t.status获取更详细的定位质量
        switch (gpsdata->fix.status) {
            case STATUS_GPS:      // 普通GPS
                quality = 1;
                break;
            case STATUS_DGPS:     // DGPS
                quality = 2;
                break;
            case STATUS_RTK_FIX:  // RTK固定解
                quality = 4;
                break;
            case STATUS_RTK_FLT:  // RTK浮点解
                quality = 5;
                break;
            case STATUS_DR:       // 航位推算
            case STATUS_GNSSDR:   // GNSS+航位推算
                quality = 6;
                break;
            default:              // 默认使用模式判断
                quality = 1;
        }
    }
    
    // 使用的卫星数量
    int satellites = gpsdata->satellites_used;
    if (satellites < 0) satellites = 0;
    
    // HDOP（水平精度因子）
    double hdop = gpsdata->dop.hdop;
    if (hdop < 0.1 || hdop > 99.9) hdop = 1.0; // 合理范围
    
    // 海拔高度
    double altitude = gpsdata->fix.altitude;
    if (altitude < -1000.0 || altitude > 10000.0) altitude = 0.0; // 合理范围
    
    // 大地水准面高度（通常为0）
    double geoid_sep = 0.0;
    
    // 生成GGA字符串 - 使用$GNGGA表示多系统GNSS
    // 格式：$GNGGA,时间,纬度,纬度方向,经度,经度方向,定位质量,卫星数,HDOP,海拔,M,大地水准面高度,M,差分年龄,差分站ID*校验和
    int len = snprintf(gga_buffer, buffer_size,
        "$GNGGA,%02d%02d%02d.000,%02d%09.6f,%c,%03d%09.6f,%c,%d,%02d,%.2f,%.3f,M,%.3f,M,,",
        tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
        lat_deg_int, lat_min, lat_dir,
        lon_deg_int, lon_min, lon_dir,
        quality, satellites, hdop, altitude, geoid_sep);
    
    // 计算校验和
    if (len > 0 && len < buffer_size) {
        unsigned char checksum = 0;
        char *p = gga_buffer + 1; // 跳过'$'
        
        // 计算从'$'后到'*'前的所有字符的异或校验和
        while (*p && *p != '*') {
            checksum ^= (unsigned char)*p++;
        }
        
        // 添加校验和（格式：*HH\r\n）
        snprintf(gga_buffer + len, buffer_size - len, "*%02X\r\n", checksum);
        return 0;
    }
    
    return -1;
}

// 从GPSD数据中提取GGA句子
int extract_gga_from_raw(struct gps_data_t *gpsdata, char *gga_buffer, size_t buffer_size) {
    // GPSD库可能不直接提供NMEA字符串
    // 对于简单实现，我们返回失败，让程序使用generate_gga_string生成GGA
    return -1;
}

int main(int argc, char *argv[]) {
    struct gps_data_t gpsdata;
    int ret;
    char gga_buffer[256];
    char expire_day[128] = {0};
    char rtcm_freq[128]  = {0};
    // 串口相关变量已删除
    
    // 串口相关命令行参数解析已删除
    
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // 初始化GPSD连接
    printf("正在连接到GPSD服务...\n");
    ret = gps_open("localhost", "2947", &gpsdata);
    if (ret != 0) {
        fprintf(stderr, "无法连接到GPSD: %s\n", gps_errstr(ret));
        return 1;
    }
    
    // 设置GPS数据流 - 使用WATCH_ENABLE来获取所有数据
    gps_stream(&gpsdata, WATCH_ENABLE, NULL);
    
    // 等待GPS数据
    printf("等待GPS定位数据...\n");
    int timeout = 30; // 30秒超时
    int got_fix = 0;
    
    while (timeout-- > 0 && running) {
        // 等待数据可用
        if (!gps_waiting(&gpsdata, 1000000)) { // 等待1秒
            printf(".");
            fflush(stdout);
            continue;
        }
        
        // 读取GPS数据
        char message[256];
        if (gps_read(&gpsdata, message, sizeof(message)) == -1) {
            fprintf(stderr, "读取GPS数据失败\n");
            continue;
        }
        
        // 检查是否有定位
        if (gpsdata.fix.mode >= MODE_2D) {
            printf("\nGPS定位成功！\n");
            printf("位置: 纬度=%.6f, 经度=%.6f, 海拔=%.2fm\n", 
                   gpsdata.fix.latitude, gpsdata.fix.longitude, gpsdata.fix.altitude);
            printf("卫星数: %d, HDOP=%.1f, 模式=%s\n", 
                   gpsdata.satellites_used, gpsdata.dop.hdop,
                   (gpsdata.fix.mode == MODE_3D) ? "3D" : "2D");
            got_fix = 1;
            break;
        }
        
        printf(".");
        fflush(stdout);
    }
    
    if (!got_fix) {
        // 即使没有定位，也继续运行，但显示警告
        printf("\n警告: 未获得GPS定位，但将继续运行程序\n");
        printf("当前状态: 模式=%d, 卫星=%d\n", 
               gpsdata.fix.mode, gpsdata.satellites_used);
        
        // 检查是否超时
        if (timeout <= 0) {
            fprintf(stderr, "GPS定位超时，请检查GPS设备\n");
            gps_close(&gpsdata);
            return 1;
        }
    }
    
    // 初始化GNSS串口(/dev/ttyTHS0)，波特率460800
    serial_fd = init_serial_port("/dev/ttyTHS0", 460800);
    if (serial_fd < 0) {
        fprintf(stderr, "无法初始化GNSS串口(/dev/ttyTHS0)，程序将继续运行但无法发送RTCM数据\n");
    }
    
    // 初始化Orin串口(/dev/ttyTHS1)，波特率460800
    serial_fd_ths1 = init_serial_port("/dev/ttyTHS1", 460800);
    if (serial_fd_ths1 < 0) {
        fprintf(stderr, "无法初始化Orin串口(/dev/ttyTHS1)，程序将继续运行但无法从Orin读取RTCM数据\n");
    }
    
    // 初始化RTK SDK
    RtcmDataResponse   rtcm_data       = {printRtcmData};
    RtcmDataResponse   ephe_data       = {printEpheData};
    RtcmStatusResponse status_data     = {printfStatusData};
    RtcmLogResponse    log_data        = {printLogData};
    
    printf("正在初始化RTK SDK...\n");
    setLogThreshold(1);
    startSdkWithLog(&rtcm_data, &status_data, &log_data);
    setRtcmUserEpoch("ITRF2008", "2016.00");
    setDeviceIDInfo("cmcc-pix-rtktest");
    printf("开始发送实时GGA数据到RTK服务器...\n");
    printf("按Ctrl+C停止程序\n\n");
    int send_count = 0;
    int gga_source = 0; // 0=从原始数据提取, 1=生成
    int send_interval = 1; // 固定发送间隔为1秒（1Hz频率）
    int min_interval = 1; // 最小发送间隔（秒）- 1秒（1Hz）
    int max_interval = 1; // 最大发送间隔（秒）- 1秒（1Hz）
    static int auth_wait_needed = 0; // 认证后需要等待的标志
    time_t last_send_time = 0; // 上次发送时间
    int initial_wait = 2; // 认证成功后初始等待时间（秒）
    
    while (running) {
        // 从Orin串口(/dev/ttyTHS1)读取RTCM数据并转发到GNSS串口(/dev/ttyTHS0)
        if (serial_fd_ths1 != -1 && serial_fd != -1) {
            char buffer[1024];
            ssize_t bytes_read = read(serial_fd_ths1, buffer, sizeof(buffer));
            if (bytes_read > 0) {
                ssize_t bytes_written = write(serial_fd, buffer, bytes_read);
                if (bytes_written == bytes_read) {
                    printf("[转发] 从Orin读取并转发到GNSS，字节数=%ld\n", bytes_written);
                } else {
                    printf("[转发] 从Orin读取但转发失败，读取=%ld字节，写入=%ld字节\n", bytes_read, bytes_written);
                }
            }
        }
        
        // 如果认证成功且需要等待，先等待初始时间
        if (rtk_authenticated && auth_wait_needed == 0) {
            printf("RTK认证成功，等待%d秒避免初始频率过高...\n", initial_wait);
            sleep(initial_wait);
            auth_wait_needed = 1;
            last_send_time = time(NULL); // 记录等待后的时间
            printf("开始以固定1Hz频率发送GGA数据...\n");
        }
        
        // 尝试读取GPS数据
        char message[256];
        int read_result = -1;
        
        // 检查是否有GPS数据可用
        if (gps_waiting(&gpsdata, 100000)) { // 等待100ms
            read_result = gps_read(&gpsdata, message, sizeof(message));
        }
        
        // 无论是否有新数据，都尝试发送GGA
        // 尝试从原始数据提取GGA
        if (extract_gga_from_raw(&gpsdata, gga_buffer, sizeof(gga_buffer)) == 0) {
            gga_source = 0;
        } 
        // 如果提取失败，生成GGA
        else if (generate_gga_string(&gpsdata, gga_buffer, sizeof(gga_buffer)) == 0) {
            gga_source = 1;
        } else {
            // 没有有效定位数据
            printf("等待有效定位数据...\n");
            sleep(1);
            continue;
        }
        
        // 检查是否达到发送间隔
        time_t current_time = time(NULL);
        if (last_send_time > 0 && (current_time - last_send_time) < send_interval) {
            // 还未到发送时间，等待剩余时间
            int wait_time = send_interval - (current_time - last_send_time);
            if (wait_time > 0) {
                sleep(wait_time);
                current_time = time(NULL); // 更新当前时间
            }
        }
        
        // 发送GGA数据
        int result = sendGGA(gga_buffer, strlen(gga_buffer));
        if (result >= 0) {
            send_count++;
            last_send_time = current_time; // 更新上次发送时间
            
            // 显示发送信息
            printf("[%d] 发送GGA数据成功 (固定1Hz): %s", send_count, gga_buffer);
            if (gga_source == 0) {
                printf(" (从GPSD原始数据提取)\n");
            } else {
                printf(" (根据定位数据生成)\n");
            }
            
            // 显示当前定位状态
            if (gpsdata.fix.mode >= MODE_2D) {
                // 获取定位质量代码
                int quality = 0;
                switch (gpsdata.fix.status) {
                    case STATUS_GPS:      // 普通GPS
                        quality = 1;
                        break;
                    case STATUS_DGPS:     // DGPS
                        quality = 2;
                        break;
                    case STATUS_RTK_FIX:  // RTK固定解
                        quality = 4;
                        break;
                    case STATUS_RTK_FLT:  // RTK浮点解
                        quality = 5;
                        break;
                    case STATUS_DR:       // 航位推算
                    case STATUS_GNSSDR:   // GNSS+航位推算
                        quality = 6;
                        break;
                    default:              // 默认使用模式判断
                        quality = 1;
                }
                
                printf("  当前位置: 纬度=%.6f°, 经度=%.6f°, 海拔=%.2fm\n", 
                       gpsdata.fix.latitude, gpsdata.fix.longitude, gpsdata.fix.altitude);
                printf("  卫星: %d颗, HDOP=%.1f, 模式=%s, 定位质量: %d(%s)\n", 
                       gpsdata.satellites_used, gpsdata.dop.hdop,
                       (gpsdata.fix.mode == MODE_3D) ? "3D" : "2D",
                       quality, get_quality_description(quality));
            }
        } else {
            // 处理错误码
            if (result == 2002) {
                fprintf(stderr, "发送GGA数据失败: 频率过高（错误码=%d）\n", result);
                // 显示GGA数据以便调试
                printf("尝试发送的GGA数据: %s\n", gga_buffer);
                // 等待1秒后继续
                sleep(1);
            } else {
                fprintf(stderr, "发送GGA数据失败: 错误码=%d\n", result);
                // 显示GGA数据以便调试
                printf("尝试发送的GGA数据: %s\n", gga_buffer);
            }
        }
        
        // 等待到下一个发送周期
        if (send_interval > 0) {
            sleep(send_interval);
        }
    }
    
    // 清理
    printf("\n正在关闭GPSD连接...\n");
    gps_stream(&gpsdata, WATCH_DISABLE, NULL);
    gps_close(&gpsdata);
    
    printf("正在停止RTK SDK...\n");
    stopSdk();
    
    // 关闭串口
    if (serial_fd != -1) {
        close(serial_fd);
        printf("已关闭GNSS串口(/dev/ttyTHS0)\n");
    }
    if (serial_fd_ths1 != -1) {
        close(serial_fd_ths1);
        printf("已关闭Orin串口(/dev/ttyTHS1)\n");
    }
    
    printf("程序已停止。总共发送了 %d 次GGA数据。\n", send_count);
    return 0;
}
