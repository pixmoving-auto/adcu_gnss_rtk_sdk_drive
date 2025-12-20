#ifndef _LIB_API_H
#define _LIB_API_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_RTCM_LEN      4096
#define MAX_MSG_LEN       4096 * 3
#define MAX_ENCRY_MSG_LEN 4096
#define MAX_GGA_LEN       256
#define MAX_CACHE_LEN     4096

#define CMCC_INCREMENT_STANDARD_RTCM   0x00            //vrs
#define CMCC_INCREMENT_ION_RESIDUAL    0x01            //vrs+4001
#define CMCC_INCREMENT_ION_I95         0x02            //vrs+3001
#define CMCC_INCREMENT_INTEGRITY       0x04            //vrs+完好性
#define CMCC_INCREMENT_EPH             0x08            //vrs+eph
#define CMCC_INCREMENT_1230            0x10            //vrs+1230
#define CMCC_INCREMENT_ALL             0xFF            //all increment

/* --------------------------------- CORS平台端口--------------------------------- */
typedef enum {
    CMCC_PORT_A = 0,  // CGCS2000
    CMCC_PORT_B = 1,  // WGS84
    CMCC_PORT_C = 2,  // ITRF2008
    CMCC_PORT_D = 3,  //
} ENUM_PORT;

/* --------------------------------- 多星多频源节点-------------------------------- */
typedef enum {
    NODE1  = 0,   // for 3 st&8 freq(no 1230)(no ephemeris)(centi-RTK)
    NODE2  = 1,   // for 4 st&4 freq(have 1230)(no ephemeris)(sub-RTK)
    NODE3  = 2,   // for 4 st&4/11 freq(no 1230)(no ephemeris)(sub-RTK/centi-RTK)(default)
    NODE4  = 3,   // for 4 st&13 freq(no 1230)(no ephemeris)(centi-RTK)
    NODE5  = 4,   // for 4 st&11 freq(have 1230)(no ephemeris)(centi-RTK)
    NODE6  = 5,   // for 4 st&11 freq(no 1230)(have ephemeris)(centi-RTK)
    NODE7  = 6,   // for 1 st&1 freq/1 gps RTD(no 1230)(no ephemeris)(sub-RTD)(need voluntarily configuration)
    NODE8  = 7,   // RTCM3.0
    NODE9  = 8,   // for 2 st&4 freq(no 1230)(no ephemeris)(centi-RTK)
    NODE10 = 9,   // for 4 st&4 freq(no 1230)(have ephemeris)(sub-RTK)
    NODE11 = 10,  // for 5 st&16 freq(no 1230)(no ephemeris)(centi-RTK)
} SOURCE_NODE;

/* --------------------------------- 网格模式-------------------------------- */
typedef enum {
    GRIDPOINT = 1,
    GRIDNETID = 2,
} ENUM_MODE;

/* --------------------------------- 用户提示码 --------------------------------- */
typedef enum {
    CMCC_IDS_STATUS_OUT_OF_SERVICE_AREA     = 1001,  // 超出服务区域
    CMCC_IDS_STATUS_INVALID_GGA             = 1002,  // 无效的GGA数据
    CMCC_IDS_STATUS_INVALID_EPH             = 1003,  // 无效的星历数据
    CMCC_IDS_STATUS_UNDEFINED               = 1999,  // 未定义状态
    
    CMCC_IDS_STATUS_CAP_START_SUCCESS       = 2000,  // 启动服务成功
    CMCC_IDS_STATUS_AUTH_SUCCESS            = 2001,  // 认证成功
    CMCC_IDS_FREQ_ERROR                     = 2002,  // 频率错误
    
    CMCC_IDS_STATUS_NETWORK_ERROR           = 3001,  // 网络错误
    CMCC_IDS_STATUS_ACCT_ERROR              = 3002,  // 账户错误
    CMCC_IDS_STATUS_ACCT_EXPIRED            = 3003,  // 账户已过期
    CMCC_IDS_STATUS_ACCT_NEED_ACTIVE        = 3005,  // 账户需要激活
    CMCC_IDS_STATUS_SERVICE_STOP            = 3007,  // 服务已停止
    CMCC_IDS_STATUS_INVALID_ACCOUNT         = 3008,  // 无效账户
    CMCC_IDS_STATUS_AUTH_EXCEPTION          = 3009,  // 认证异常
    CMCC_IDS_STATUS_ACCTID_ERROR            = 3010,  // 账户ID错误
    CMCC_IDS_STATUS_ACCT_ALREADY_ACTIVATED  = 3011,  // 账户已激活
    CMCC_IDS_STATUS_ACCT_DISABLED           = 3012,  // 账户已禁用
    CMCC_IDS_STATUS_MOUNTPOINT_INVALID      = 3016,  // 挂载点无效
    CMCC_IDS_STATUS_TOO_MANY_FAILURE        = 3017,  // 失败次数过多
    CMCC_IDS_STATUS_NOT_IN_SERVICE_EREA     = 3018,  // 不在服务区域内
    CMCC_IDS_STATUS_NO_RTCM_IN_SERVICE_EREA = 3019,  // 服务区域无RTCM数据
    CMCC_IDS_DNSTOIP_ERROR                  = 3020,  // DNS解析IP错误
    CMCC_IDS_TIMEPOOL_BIND_FAIL             = 3021,  // 时间池绑定失败
    CMCC_IDS_ACCOUNT_LOGIN_RPCFAIL          = 3022,  // 账户登录RPC失败
    CMCC_IDS_TIMEPOOL_TIME_EXCEED           = 3023,  // 时间池时间超限
    CMCC_IDS_TIMEPOOL_NOTEXIST              = 3024,  // 时间池不存在
    CMCC_IDS_LOGIN_TYPE_ERROR               = 3025,  // 登录类型错误
    CMCC_IDS_ACCOUNT_NO_SERVICE_PERIOD      = 3026,  // 账户无服务时段
    CMCC_IDS_ACCOUNT_NO_MOUNTPOINT          = 3027,  // 账户无挂载点
    CMCC_IDS_ALREADY_CONNECTED              = 3028,  // 已连接
    CMCC_IDS_OVER_NODE_MAX_CAPACITY         = 3029,  // 超过节点最大容量
    CMCC_IDS_CORS_AUTH_FAILED               = 3030,  // CORS认证失败
    CMCC_IDS_BAD_DEVICEID                   = 3031,  // 设备ID错误
    CMCC_IDS_TERMINAL_ERROR                 = 3032,  // 终端错误
    CMCC_IDS_AUTH_TYPE_ERROR                = 3033,  // 认证类型错误
    CMCC_IDS_ENCRYPT_TYPE_ERROR             = 3034,  // 加密类型错误
    CMCC_IDS_CORS_CONNECT_FAILED            = 3035,  // CORS连接失败
    CMCC_IDS_CORS_LOGIN_FAILED              = 3036,  // CORS登录失败
    CMCC_IDS_MANUFACTURER_ERROR             = 3037,  // 厂商错误
    CMCC_IDS_OVER_POOL_MAX_CAPACITY         = 3038,  // 超过资源池最大容量
    CMCC_IDS_BRAND_ERROR                    = 3039,  // 品牌错误
    CMCC_IDS_PRODUCT_ERROR                  = 3040,  // 产品型号错误
    CMCC_IDS_ACCOUNTTYPE_ERROR              = 3041,  // 账户类型错误
    CMCC_IDS_LOGIN_PARAS_ERROR              = 3042,  // 登录参数错误
    CMCC_IDS_GGA_NOSESSION_ERROR            = 3043,  // GGA无会话错误
    CMCC_IDS_ACCPOOL_BIND_FAIL              = 3044,  // 账户池绑定失败
    CMCC_IDS_ACCPOOL_NOTEXIST               = 3045,  // 账户池不存在
    CMCC_IDS_POOL_NOTEXIST                  = 3046,  // 资源池不存在
    CMCC_IDS_POOL_NOTALLOW_LOGIN            = 3047,  // 资源池不允许登录
    CMCC_IDS_AUTHRIZATION_INVALID           = 3048,  // 授权无效
    CMCC_IDS_INVALID_FRAME_OR_EPOCH         = 3049,  // 无效的参考框架或历元
    CMCC_IDS_ACCOUNT_KICKING_ERROR          = 3054,  // 账户被踢出错误
    CMCC_IDS_STATUS_UNKNOWN_ERROR           = 3099,  // 未知错误
    
    CMCC_IDS_STATUS_NULL_ACCT               = 3100,  // 空账户
    CMCC_IDS_STATUS_INVALID_PARA            = 3101,  // 无效参数
    CMCC_IDS_Server_Unavailable             = 3102,  // 服务不可用
    CMCC_IDS_Login_Exception                = 3103,  // 登录异常
    CMCC_IDS_ACCOUNT_NoExpireTime           = 3104,  // 账户无过期时间
    CMCC_IDS_Login_ElseWhere                = 3105,  // 在其他地方登录
    CMCC_IDS_Biz_Support_Error              = 3106,  // 业务支持错误
    CMCC_IDS_Login_Limit                    = 3107,  // 登录限制
    CMCC_IDS_Get_Route_Failed               = 3108,  // 获取路由失败
    CMCC_IDS_Account_Not_Active             = 3109,  // 账户未激活
    CMCC_IDS_Account_Disabled               = 3110,  // 账户已禁用
    CMCC_IDS_Service_Time_RunOut            = 3111,  // 服务时间用尽
    CMCC_IDS_AppKey_DeviceId_NotMatch       = 3112,  // AppKey与设备ID不匹配
    
    CMCC_IDS_STATUS_CAP_START_FAILED        = 5001,  // 启动服务失败
} UserErrorCode_t; 

/* --------------------------------- 星座系统 --------------------------------- */
typedef enum{
    CMCC_SYS_GPS = 0x01, /** navigation system: GPS */
    CMCC_SYS_GLO = 0x04, /** navigation system: GLONASS */
    CMCC_SYS_GAL = 0x08, /** navigation system: Galileo */
    CMCC_SYS_QZS = 0X10, /** navigation system: QZSS */
    CMCC_SYS_BDS = 0x20, /** navigation system: BeiDou */
    CMCC_SYS_ALL = 0xFF, /** navigation system: all */
}CmccSys;

/* --------------------------------- 星历数据范围 --------------------------------- */
typedef enum{
    SYS_SCOPE_CHINA = 0, /**  system scope : China */
    SYS_SCOPE_WORLD = 1, /**  system scope : World */
}SysScope;


typedef enum{
    RTCM_ONLY = 0, /**  仅含RTCM */
    EPH_ONLY = 1,  /**  仅含星历 */
    RTCM_EPH = 2,  /**  RTCM与星历均包含 */
} RtcmDatatype;

/*---------------------------------电离层、UTC、历书请求 预留------------------------*/
typedef enum{
    EPH_ALM_ION_UTC_NONE = 0x00,  /** 无请求 */
    EPH_ALM = 0x01,               /** 历书  */
    EPH_ION_UTC = 0x02,           /** 电离层、UTC   */
    EPH_ALM_ION_UTC = 0x03,       /** 电离层、UTC、历书 */
}EphAlmIonUtcDemond;

typedef enum{
    COOR_V3 = 0,
    COOR_V2 = 1,                 /** 行业对齐坐标转换 */
}ENUM_COOR;

typedef enum{
    EPHPROTO_RTCM = 0,                 /** RTCM格式星历 */
    EPHPROTO_UBX  = 1,                 /** UBX格式星历 */
    EPHPROTO_UCT  = 2,                 /** 预留 */
    EPHPROTO_MXT  = 3,                 /** 预留 */
    EPHPROTO_SDBP = 4,                 /** 预留 */
}ENUM_EPHPROTO;

typedef void (*SdkRtcmData)(void* data, int length);
typedef void (*SdkRtcmStatus)(int status);
typedef void (*SdkRtcmLog)(void* buf, int length);

typedef struct _rtcm_data_response_ {
    SdkRtcmData cb_rtcmdata;
} RtcmDataResponse;

typedef struct _rtcm_status_response_ {
    SdkRtcmStatus cb_status;
} RtcmStatusResponse;

typedef struct _rtcm_log_response_ {
    SdkRtcmLog cb_rtcmlog;
} RtcmLogResponse;

/**
 * @brief : get SDK version
 *
 * @return:
 *   a pointer to version
 */
const char* getSdkVersion(void);

/* This is a non-blocking API for send GGA to CORS
 *
 * @param[in]  ggaRawBuf : data to send in ASCII character
 * @param[in]  length    : data len to send in ASCII character
 *
 * @return:
 *   0 if success
 *  -1 if fail
 */
int sendGGA(const char* ggaRawBuf, const int length);

int sendIonoRquest(const char* ggaRawBuf, const int length);

int sendEphReq(const char* ggaRawBuf, const int length);

int sendEphRequest(SysScope scope,int cmccsyss);

/**
 * @brief : start SDK statemachine
 * @param[in]  data_rsp  : include a callback function for pass the rtcm data
 * @param[in]  status_rsp: include a callback function for pass the rtcm status
 * @return:
 *   void
 */
void startSdk(RtcmDataResponse* data_rsp, RtcmStatusResponse* status_rsp);

void setEphCallback(RtcmDataResponse* eph_data);

void startSdkWithLog(RtcmDataResponse* data_rsp, RtcmStatusResponse* status_rsp, RtcmLogResponse* log_data);
/**
 * @brief : stop SDK statemachine
 * @return:
 *   void
 */
void stopSdk();

/**
 * @brief : set new server ip
 * @return:
 *   void
 */
void setServerIP(const char* server_ip);

/**
 * @brief : choose server port mannually
 * @return:
 *   void
 */
void setServerPort(int server_port);

/**
 * @brief : login through personal usrname&password
 * @return:
 *   void
 */
void setRtcmUserInfo(const char* user, const char* pwd);

/**
 * @brief : login through device id
 * @return:
 *   void
 */
void setDeviceIDInfo(const char* device_id);

/**
 * @brief : login through enterprise usrname&password&id,id can be NULL
 * @return:
 *   void
 */
void setEtprUserInfo(const char* user, const char* pwd, const char* device_id);
void setTimeUserInfo(const char* user, const char* pwd, const char* device_id);
void setAKASInfo(const char* app_key, const char* app_secret, const char* device_id);

/**
 * @brief : set new source id
 * @return:
 *   void
 */
void setMountPoint(const char* mountpoint);

/**
 * @brief :choose multipile satellites& frequencie node
 * @return:
 *   void
 */
void setSourceNode(SOURCE_NODE sourceNode);

void getExpireDate(char* pDate, int length);
void getFrequency(char* pFreq, int length);

void setLogThreshold(int logThrd);
void setRtcmFreq(int FreqPort);
void setRtcmUserEpoch(const char* frame, const char* epoch);
void setGridNetMode(ENUM_MODE gridnetmode);
void setIncrementalService(int service);
void setEphProto(ENUM_EPHPROTO ephproto);
void setCoorTran(ENUM_COOR cortran);
void setDiagnoShellPath(char *path);
#if defined(TLS_DEBUG)
void setTlsLogLevel(int level);
#endif
#ifdef __cplusplus
}
#endif

#endif  //_LIB_API_H
