

#ifndef __CHR_USER_H__
#define __CHR_USER_H__

/* Other Include Head File */
#include "chr_errno.h"

/* 宏定义 */
/* 主动上报 */
/* 芯片校准异常事件上报 */
#define CHR_CHIP_CALI_ERR_REPORT_EVENTID 909003001
/* 芯片平台异常事件上报 */
#define CHR_PLATFORM_EXCEPTION_EVENTID 909003002
/* 芯片关联失败事件上报 */
#define CHR_WIFI_CONNECT_FAIL_REPORT_EVENTID 909003003
/* 芯片主动断开事件上报 */
#define CHR_WIFI_DISCONNECT_REPORT_EVENTID 909003004
/* wifi温控时间上报 */
#define CHR_WIFI_TEMP_PROTECT_REPORT_EVENTID 909003005
/* wifi PLL 失锁 */
#define CHR_WIFI_PLL_LOST_REPORT_EVENTID 909003006
/* wifi 漫游失败事件上报 */
#define CHR_WIFI_ROAM_FAIL_REPORT_EVENTID 909009102
/* wifi 漫游统计事件上报 */
#define CHR_WIFI_ROAM_INFO_REPORT_EVENTID 909009105
/* wifi 11ax数据统计上报 */
#define CHR_WIFI_OFDMA_STAT_EVENTID 909009126
/* wifi 周期上报 */
#define CHR_WIFI_PERIODIC_REPORT_EVENTID 909009521
/* PC国家码更新上报 */
#define CHR_WIFI_COUNTRYCODE_UPDATE_EVENTID 609003001
/* PC IWAR RR性能流程上报 */
#define CHR_WIFI_IWARE_RR_EVENTID 609003002
/* PC E2PROM上报 */
#define CHR_READ_EEPROM_ERROR_EVENTID 609009001
/* PC S3&S4 CHR上报 */
#define CHR_PLATFORM_S3S4_EVENTID 609009002
/* HiD2D ACS方案统计信息CHR上报 */
#define CHR_HID2D_ACS_INFO_REPORT_EVENTID 909001011

/* 主动查询 */
/* wifi功率速率统计上报 */
#define CHR_WIFI_POW_RATE_COUNT_QUERY_EVENTID 909009519
/* wifi工作时间统计上报 */
#define CHR_WIFI_WORK_TIME_INFO_EVENTID 909009520
/* wifi工作时间细分和实际收发包统计上报 */
#define CHR_WIFI_WORKTIME_AND_TX_RX_COUNT_INFO_EVENTID 909009599

/* wifi打开关闭失败 */
#define CHR_WIFI_OPEN_CLOSE_FAIL_QUERY_EVENTID 909002021
/* wifi异常断开 */
#define CHR_WIFI_DISCONNECT_QUERY_EVENTID 909002022
/* wifi连接异常 */
#define CHR_WIFI_CONNECT_FAIL_QUERY_EVENTID 909002023
/* wifi上网失败 */
#define CHR_WIFI_WEB_FAIL_QUERY_EVENTID 909002024
/* wifi上网慢 */
#define CHR_WIFI_WEB_SLOW_QUERY_EVENTID 909002025
/* wifi上报beacon */
#define CHR_WIFI_BEACON_EVENTID 909002027
/* wifi上报twt参数信息 */
#define CHR_WIFI_TWT_QUERY_EVENTID 909009127

/* BT软件异常上报 */
#define CHR_BT_CHIP_SOFT_ERROR_EVENTID 913900003

#define CHR_EXCEPTION_MODULE_NAME_LEN 10
#define CHR_EXCEPTION_ERROR_CODE_LEN  20

/* STRUCT定义 */
typedef struct chr_chip_excption_event_info_tag {
    uint32_t ul_module;
    uint32_t ul_plant;
    uint32_t ul_subplant;
    uint32_t ul_errid;
} chr_platform_exception_event_info_stru;

typedef struct chr_scan_exception_event_info_tag {
    int sub_event_id;
    char module_name[CHR_EXCEPTION_MODULE_NAME_LEN];
    char error_code[CHR_EXCEPTION_ERROR_CODE_LEN];
} chr_scan_exception_event_info_stru;

typedef struct chr_connect_exception_event_info_tag {
    int sub_event_id;
    char platform_module_name[CHR_EXCEPTION_MODULE_NAME_LEN];
    char error_code[CHR_EXCEPTION_ERROR_CODE_LEN];
} chr_connect_exception_event_info_stru;

#ifdef _PRE_CONFIG_HW_CHR
#define CHR_ERR_DATA_MAX_NUM 0x20
#define CHR_ERR_DATA_MAX_LEN (sizeof(uint32_t) * CHR_ERR_DATA_MAX_NUM)
typedef struct {
    uint32_t errno;
    uint16_t errlen;
    uint16_t flag : 1;
    uint16_t resv : 15;
} chr_errno_with_arg_stru;

typedef enum {
    CHR_DEVICE = 0x0,
    CHR_HOST = 0x1,
    CHR_DEVICE_P2P = 0x2,
    CHR_HOST_P2P = 0x3,
    CHR_DEVICE_AP = 0x4,
    CHR_HOST_AP = 0x5,
    /* chr扩展专区:增加flags */
    CHR_REPORT_EXT_WIFI_INFO = 0x8000,
    /* 适配上层,增加chr上报结束flag */
    CHR_REPORT_FINISH = 0xFFFF
} chr_report_flags;
typedef uint16_t chr_report_flags_enum_uint16;

typedef uint32_t (*chr_get_wifi_info)(uint32_t);

extern void chr_dev_exception_callback(void *buff, uint16_t len);
extern int32_t __chr_exception_para(uint32_t chr_errno, uint8_t *chr_ptr, uint16_t chr_len);
extern int32_t __chr_exception_para_q(uint32_t chr_errno, chr_report_flags_enum_uint16 chr_flag,
    uint8_t *chr_ptr, uint16_t chr_len);
extern void chr_host_callback_register(chr_get_wifi_info pfunc);
extern void chr_host_callback_unregister(void);

#define chr_exception(errno)                         __chr_exception_para(errno, NULL, 0)
#define chr_exception_p(chr_errno, chr_ptr, chr_len) __chr_exception_para(chr_errno, chr_ptr, chr_len)
#define chr_exception_q(chr_errno, chr_flag, chr_ptr, chr_len) __chr_exception_para_q(chr_errno, chr_flag, chr_ptr, chr_len)
#define chr_exception_report(excption_event, module, plant, subplant, errid)                 \
    do {                                                                                     \
        chr_platform_exception_event_info_stru chr_platform_exception_event_info;            \
        chr_platform_exception_event_info.ul_module = module;                                \
        chr_platform_exception_event_info.ul_plant = plant;                                  \
        chr_platform_exception_event_info.ul_subplant = subplant;                            \
        chr_platform_exception_event_info.ul_errid = errid;                                  \
        chr_exception_p(excption_event, (uint8_t *)(&chr_platform_exception_event_info),   \
                        sizeof(chr_platform_exception_event_info_stru));                 \
    } while (0)
#else
#define chr_exception(chr_errno)
#define chr_exception_p(chr_errno, chr_ptr, chr_len)
#define chr_exception_q(chr_errno, chr_flag, chr_ptr, chr_len)
#define chr_exception_report(excption_event, module, plant, subplant, errid)
#endif
#endif
