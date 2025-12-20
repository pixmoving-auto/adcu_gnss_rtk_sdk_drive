# RTK差分定位增强系统

## 项目概述

本项目是一个基于中国移动RTK差分服务的GNSS定位增强系统，专为NVIDIA Orin平台（优控ADCU A06）设计。该系统能够获取实时RTK差分数据并发送给GNSS设备，实现厘米级高精度定位。

## 功能特性

- **RTK差分数据处理**：接收中国移动RTK服务器返回的RTCM差分数据
- **GPSD集成**：通过GPSD服务（localhost:2947）获取GNSS原始数据
- **双串口配置**：支持两个串口（/dev/ttyTHS0和/dev/ttyTHS1），波特率460800
- **RTCM数据转发**：将服务器返回的RTCM差分数据写入GNSS设备串口
- **自动GPSD管理**：运行脚本自动检查并重启GPSD服务（无需密码）
- **实时状态监控**：输出RTK认证状态、定位质量等信息

## 文件结构

```
├── main_gpsd.c        # 主程序源代码
├── main_gpsd          # 编译后的可执行文件
├── run_rtk.sh         # 运行脚本
├── libcmcc_sdk.so     # 中国移动RTK SDK库
├── libapi.h           # SDK头文件
└── README.md          # 项目说明文档
```

## 安装与配置

### 1. 环境要求

- NVIDIA Orin平台（优控ADCU A06）
- Linux操作系统
- GPSD服务
- 中国移动RTK账号

### 2. 配置GPSD服务

确保GPSD服务已正确配置并运行：

```bash
sudo systemctl enable gpsd
sudo systemctl start gpsd
```

### 3. 配置sudo无密码权限

为了让脚本自动重启GPSD服务无需密码，创建以下配置文件：

```bash
sudo echo "$USER ALL=(ALL) NOPASSWD: /bin/systemctl restart gpsd, /bin/systemctl is-active gpsd" > /etc/sudoers.d/gpsd_no_password
sudo chmod 440 /etc/sudoers.d/gpsd_no_password
```

### 4. 编译程序

使用以下命令编译程序：

```bash
gcc -o main_gpsd main_gpsd.c -L. -lcmcc_sdk -lpthread -lgps -lm
```

## 使用方法

### 运行脚本

使用`run_rtk.sh`脚本启动程序：

```bash
./run_rtk.sh [串口设备] [波特率]
```

### 参数说明

- `串口设备`：GNSS设备的串口路径（可选，默认：/dev/ttyTHS0）
- `波特率`：串口通信波特率（可选，默认：460800）

### 示例

```bash
# 使用默认配置（/dev/ttyTHS0，460800）
./run_rtk.sh

# 指定串口和波特率
./run_rtk.sh /dev/ttyTHS1 460800
```

## 工作原理

1. **初始化**：程序启动时初始化GPSD连接和RTK SDK
2. **数据获取**：
   - 从GPSD获取GNSS定位数据（GGA格式）
   - 将GGA数据发送到中国移动RTK服务器
3. **差分数据处理**：
   - 接收服务器返回的RTCM差分数据
   - 通过`printRtcmData`函数将数据写入GNSS设备串口
4. **定位增强**：GNSS设备使用差分数据实现RTK固定解（定位质量：4）

## 核心函数说明

### main_gpsd.c

- `init_serial_port`：初始化串口参数（波特率、数据位、停止位等）
- `printRtcmData`：处理并发送RTCM差分数据到GNSS设备
- `printfStatusData`：显示RTK服务状态信息
- `generate_gga_string`：生成GGA格式定位数据

### run_rtk.sh

- 检查可执行文件和库文件
- 自动检查并重启GPSD服务
- 处理命令行参数
- 启动主程序

## 常见问题

### GPSD服务无法启动

检查GPSD配置文件`/etc/default/gpsd`，确保串口设置正确：

```bash
DEVICES="/dev/ttyTHS0"
GPSD_OPTIONS="-n"
```

### 串口设备忙

确保没有其他程序占用串口：

```bash
lsof /dev/ttyTHS0
```

### RTK认证失败

检查：
1. 网络连接是否正常
2. RTK账号是否有效且未过期
3. 是否在服务覆盖区域内

## 日志说明

程序运行时输出的关键信息：

- `[RTCM-xxxx] 已成功写入GNSS串口`：RTCM差分数据发送成功
- `RTK认证成功！可以开始发送GGA数据。`：RTK服务认证成功
- `定位质量: 4(RTK固定解)`：已成功实现RTK固定解

## 开发者信息

- **作者**：Teddy
- **邮箱**：teddy@pixmoving.net
- **版本**：V1.0
- **日期**：2025-12-20

## 版权声明

本项目使用中国移动RTK SDK，相关使用需遵守中国移动的服务条款。
