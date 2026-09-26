

#ifndef __OAM_EXT_IF_H__
#define __OAM_EXT_IF_H__

/* 其他头文件包含 */
#include "platform_spec.h"
#include "oal_types.h"
#include "oal_net.h"
#include "oam_log.h"
#include "wlan_types.h"

#include "oam_wdk.h"
#include "oam_config.h"
#include "chr_user.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAM_EXT_IF_H

/* 日志宏定义 */
/* 该接口需要严格控制调用，主要用于中断上半部和OAM OAL等模块频繁触发的异常分支 */
#define oam_wifi_log_para_press(vap_id, feature_id, fileid, lev) \
    (((fileid) & 0xFFFF) | (((feature_id) & 0xFF) << 16) | (((vap_id) & 0xF) << 24) | (((lev) & 0xF) << 28))

#ifdef _PRE_WLAN_DFT_LOG

#define oam_io_printk(fmt, ...) \
    oam_log_console_printk(THIS_FILE_ID, (uint16_t)__LINE__, OAL_FUNC_NAME, fmt, ##__VA_ARGS__)

#define oam_info_log0(_uc_vap_id, _en_feature_id, fmt)                                                        \
    oam_log_print0(oam_wifi_log_para_press(_uc_vap_id, _en_feature_id, THIS_FILE_ID, OAM_LOG_LEVEL_INFO), \
        (uint16_t)__LINE__, fmt)
#define oam_info_log1(_uc_vap_id, _en_feature_id, fmt, p1)                                                    \
    oam_log_print1(oam_wifi_log_para_press(_uc_vap_id, _en_feature_id, THIS_FILE_ID, OAM_LOG_LEVEL_INFO), \
        (uint16_t)__LINE__, fmt, (oal_uint)(p1))
#define oam_info_log2(_uc_vap_id, _en_feature_id, fmt, p1, p2)                                                \
    oam_log_print2(oam_wifi_log_para_press(_uc_vap_id, _en_feature_id, THIS_FILE_ID, OAM_LOG_LEVEL_INFO), \
        (uint16_t)__LINE__, fmt, (oal_uint)(p1), (oal_uint)(p2))
#define oam_info_log3(_uc_vap_id, _en_feature_id, fmt, p1, p2, p3)                                            \
    oam_log_print3(oam_wifi_log_para_press(_uc_vap_id, _en_feature_id, THIS_FILE_ID, OAM_LOG_LEVEL_INFO), \
        (uint16_t)__LINE__, fmt, (oal_uint)(p1), (oal_uint)(p2), (oal_uint)(p3))
#define oam_info_log4(_uc_vap_id, _en_feature_id, fmt, p1, p2, p3, p4)                                        \
    oam_log_print4(oam_wifi_log_para_press(_uc_vap_id, _en_feature_id, THIS_FILE_ID, OAM_LOG_LEVEL_INFO), \
        (uint16_t)__LINE__, fmt, (oal_uint)(p1), (oal_uint)(p2), (oal_uint)(p3), (oal_uint)(p4))

#define oam_warning_log0(_uc_vap_id, _en_feature_id, fmt)                                                        \
    oam_log_print0(oam_wifi_log_para_press(_uc_vap_id, _en_feature_id, THIS_FILE_ID, OAM_LOG_LEVEL_WARNING), \
        (uint16_t)__LINE__, fmt)
#define oam_warning_log1(_uc_vap_id, _en_feature_id, fmt, p1)                                                    \
    oam_log_print1(oam_wifi_log_para_press(_uc_vap_id, _en_feature_id, THIS_FILE_ID, OAM_LOG_LEVEL_WARNING), \
        (uint16_t)__LINE__, fmt, (oal_uint)(p1))
#define oam_warning_log2(_uc_vap_id, _en_feature_id, fmt, p1, p2)                                                \
    oam_log_print2(oam_wifi_log_para_press(_uc_vap_id, _en_feature_id, THIS_FILE_ID, OAM_LOG_LEVEL_WARNING), \
        (uint16_t)__LINE__, fmt, (oal_uint)(p1), (oal_uint)(p2))
#define oam_warning_log3(_uc_vap_id, _en_feature_id, fmt, p1, p2, p3)                                            \
    oam_log_print3(oam_wifi_log_para_press(_uc_vap_id, _en_feature_id, THIS_FILE_ID, OAM_LOG_LEVEL_WARNING), \
        (uint16_t)__LINE__, fmt, (oal_uint)(p1), (oal_uint)(p2), (oal_uint)(p3))
#define oam_warning_log4(_uc_vap_id, _en_feature_id, fmt, p1, p2, p3, p4)                                        \
    oam_log_print4(oam_wifi_log_para_press(_uc_vap_id, _en_feature_id, THIS_FILE_ID, OAM_LOG_LEVEL_WARNING), \
        (uint16_t)__LINE__, fmt, (oal_uint)(p1), (oal_uint)(p2), (oal_uint)(p3), (oal_uint)(p4))

#define oam_error_log0(_uc_vap_id, _en_feature_id, fmt)                                                        \
    oam_log_print0(oam_wifi_log_para_press(_uc_vap_id, _en_feature_id, THIS_FILE_ID, OAM_LOG_LEVEL_ERROR), \
        (uint16_t)__LINE__, fmt)
#define oam_error_log1(_uc_vap_id, _en_feature_id, fmt, p1)                                                    \
    oam_log_print1(oam_wifi_log_para_press(_uc_vap_id, _en_feature_id, THIS_FILE_ID, OAM_LOG_LEVEL_ERROR), \
        (uint16_t)__LINE__, fmt, (oal_uint)(p1))
#define oam_error_log2(_uc_vap_id, _en_feature_id, fmt, p1, p2)                                                \
    oam_log_print2(oam_wifi_log_para_press(_uc_vap_id, _en_feature_id, THIS_FILE_ID, OAM_LOG_LEVEL_ERROR), \
        (uint16_t)__LINE__, fmt, (oal_uint)(p1), (oal_uint)(p2))
#define oam_error_log3(_uc_vap_id, _en_feature_id, fmt, p1, p2, p3)                                            \
    oam_log_print3(oam_wifi_log_para_press(_uc_vap_id, _en_feature_id, THIS_FILE_ID, OAM_LOG_LEVEL_ERROR), \
        (uint16_t)__LINE__, fmt, (oal_uint)(p1), (oal_uint)(p2), (oal_uint)(p3))
#define oam_error_log4(_uc_vap_id, _en_feature_id, fmt, p1, p2, p3, p4)                                        \
    oam_log_print4(oam_wifi_log_para_press(_uc_vap_id, _en_feature_id, THIS_FILE_ID, OAM_LOG_LEVEL_ERROR), \
        (uint16_t)__LINE__, fmt, (oal_uint)(p1), (oal_uint)(p2), (oal_uint)(p3), (oal_uint)(p4))

#else
#define oam_excp_record(_uc_vap_id, _excp_id)
#define oam_io_printk(fmt, ...)
#define oam_info_log0(_uc_vap_id, _en_feature_id, fmt)
#define oam_info_log1(_uc_vap_id, _en_feature_id, fmt, p1)
#define oam_info_log2(_uc_vap_id, _en_feature_id, fmt, p1, p2)
#define oam_info_log3(_uc_vap_id, _en_feature_id, fmt, p1, p2, p3)
#define oam_info_log4(_uc_vap_id, _en_feature_id, fmt, p1, p2, p3, p4)
#define oam_warning_log0(_uc_vap_id, _en_feature_id, fmt)
#define oam_warning_log1(_uc_vap_id, _en_feature_id, fmt, p1)
#define oam_warning_log2(_uc_vap_id, _en_feature_id, fmt, p1, p2)
#define oam_warning_log3(_uc_vap_id, _en_feature_id, fmt, p1, p2, p3)
#define oam_warning_log4(_uc_vap_id, _en_feature_id, fmt, p1, p2, p3, p4)
#define oam_error_log0(_uc_vap_id, _en_feature_id, fmt)
#define oam_error_log1(_uc_vap_id, _en_feature_id, fmt, p1)
#define oam_error_log2(_uc_vap_id, _en_feature_id, fmt, p1, p2)
#define oam_error_log3(_uc_vap_id, _en_feature_id, fmt, p1, p2, p3)
#define oam_error_log4(_uc_vap_id, _en_feature_id, fmt, p1, p2, p3, p4)

#endif

#define OAM_DIFF_LOG0 oam_warning_log0
#define OAM_DIFF_LOG1 oam_warning_log1
#define OAM_DIFF_LOG2 oam_warning_log2
#define OAM_DIFF_LOG3 oam_warning_log3
#define OAM_DIFF_LOG4 oam_warning_log4

/* 2.1 TRACE相关宏定义 */
/* 统计信息宏定义 */ /*lint -e506*/ /*lint -e774*/
#define oam_stat_dev_incr(_uc_dev_id, _member, _num)

/* 为了对数组下标进行保护，必须要传入数组长度(第4个参数) */
#define oam_stat_dev_array_incr(_uc_dev_id, _member, _uc_array_index, _uc_array_max_size)

#define oam_stat_dev_update(_uc_dev_id, _member, _uc_q_id, _uc_index, _ul_val)

#define oam_stat_dev_get_val(_uc_dev_id, _member, _pul_val)

#define oam_stat_dev_set_val(_uc_dev_id, _member, _uc_val)

#define oam_stat_vap_incr(_uc_vap_id, _member, _num)                                 \
    do {                                                                             \
        if (_uc_vap_id < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT) {                           \
            g_oam_stat_info.ast_vap_stat_info[_uc_vap_id].ul_##_member += (_num); \
        }                                                                            \
    } while (0)

#define oam_stat_mgmt_incr(_uc_vap_id, _uc_mgmt_id, _member)

#define oam_stat_user_incr(_uc_user_id, _member, _num)

#define oam_stat_tid_incr(_uc_user_id, _uc_tid_num, _member, _num)

#define oam_stat_get_stat_all() (&g_oam_stat_info)

#define oam_tid_ampdu_stats_incr(_member, _cnt)

#define OAM_PRINT_FORMAT_LENGTH 256 /* 打印格式字符串的最大长度 */

/* 日志结构体相关宏，用于oam_log_info_stru */
#define OAM_FUNC_NAME_MAX_LEN       48   /* 函数名的最大长度 */
#define OAM_LOG_INFO_MAX_LEN        100  /* 日志信息最大长度 */
#define OAM_LOG_INFO_IN_MEM_MAX_NUM 5000 /* oam模块最多保存5000条log信息，超过以后从头自动覆盖 */
#define OAM_LOG_ENTRY_PARA_NUM      4

#define OAM_SEQ_TRACK_NUM 128

/* device级别的各类中断错误最大个数 */
#define OAM_MAC_ERROR_TYPE_MAX_NUM 25
#define OAM_SOC_IRQ_MAX_NUM        5
#define OAM_PMU_BUCK_IRQ_MAX_NUM   5
#define OAM_MAC_SUB_IRQ_MAX_NUM    11

/* 内存块信息除掉最后一个成员占用的大小 */
#define OAM_MEMBLOCK_INFO_STRU_LEN 8

/* oam_stats_device_subtype_irq_stru前三个成员占用空间大小 */
#define OAM_FIRST_THREE_MEMBER_LEN 4

/* 每一个事件队列中事件个数最大值 */
#define OAM_MAX_EVENT_NUM_IN_EVENT_QUEUE 8

/* OAM模块申请skb时，头部和尾部为sdt分别预留8个和1个字节 */
#define OAM_RESERVE_SKB_HEADROOM_LEN 8
#define OAM_RESERVE_SKB_TAILROOM_LEN 1
#define OAM_RESERVE_SKB_LEN          (OAM_RESERVE_SKB_HEADROOM_LEN + OAM_RESERVE_SKB_TAILROOM_LEN)

/* 字符串以0结尾上报，实际字符串内容最大长度 */
#define OAM_REPORT_MAX_STRING_LEN (WLAN_SDT_NETBUF_MAX_PAYLOAD - 1) /* 以\0结束 */

/* 统计相关宏 */
#define OAM_MAC_ERROR_TYPE_CNT   25
#define OAM_RX_DSCR_QUEUE_CNT    2
#define OAM_IRQ_FREQ_STAT_MEMORY 50

#define OAM_TID_TRACK_NUM 4

/* 性能维测相关宏定义 */
#ifdef _PRE_WLAN_DFT_STAT
#define OAM_PHY_STAT_NODE_ENABLED_MAX_NUM 4
#define OAM_PHY_STAT_RX_PPDU_CNT          8
#define OAM_PHY_STAT_ITEM_MIN_IDX         1
#define OAM_PHY_STAT_ITEM_MAX_IDX         16
#define OAM_MACHW_STAT_RX_MEMBER_CNT      18
#define OAM_MACHW_STAT_TX_MEMBER_CNT      8
#define OAM_PSM_STAT_CNT                  10
#define OAM_UAPSD_STAT_CNT                11
#define OAM_TID_STAT_CNT                  14
#endif

#define OAM_OTA_RX_DSCR_TYPE OAM_OTA_TYPE_RX_DSCR_PILOT
#define OAM_OTA_TX_DSCR_TYPE OAM_OTA_TYPE_TX_DSCR_PILOT

/* 枚举定义 */
/* data type */
typedef enum {
    OAM_DATA_TYPE_LOG = 0x1,
    OAM_DATA_TYPE_OTA = 0x2,
    OAM_DATA_TYPE_TRACE = 0x3,
    OAM_DATA_TYPE_EVENT = 0x4,
    OAM_DATA_TYPE_MEM_RW = 0x9,
    OAM_DATA_TYPE_REG_RW = 0x15,
    OAM_DATA_TYPE_CFG = 0x0b,
    OAM_DATA_TYPE_GVAR_RW = 0x27, /* global value read or write */
    OAM_DATA_TYPE_STRING = 0x28,  /* report string  */

    OAM_DATA_TYPE_DEVICE_LOG = 0x40,
#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)
    OAM_DATA_TYPE_SAMPLE = 0x45,
#endif
    OAM_DATA_TYPE_DPD = 0x55,
    OAM_DATA_TYPE_BUTT
} oam_data_type_enum;
typedef uint8_t oam_data_type_enum_uint8;

/* SDT驱动侧向SDT APP侧发送数据的类型 */
typedef enum {
    OAM_PRIMID_TYPE_PC_TO_DEV_DATA = 1,
    OAM_PRIMID_TYPE_DEV_ACK,
    OAM_PRIMID_TYPE_DEV_ACTIVE_UPLOAD,
    OAM_PRIMID_TYPE_OUTPUT_CONTENT,

    SDT_DRV_PRIMID_TYPE_BUTT
} oam_primid_type_enum;
typedef uint8_t oam_primid_type_enum_uint8;

typedef enum {
    OAM_MODULE_ID_ALL,
    OAM_MODULE_ID_WAL,
    OAM_MODULE_ID_HMAC,
    OAM_MODULE_ID_DMAC,

    OAM_MODULE_ID_HAL,
    OAM_MODULE_ID_OAM,
    OAM_MODULE_ID_OAL,
    OAM_MODULE_ID_FRW,
    OAM_MODULE_ID_ALG,
    OAM_MODULE_ID_PLAT,
    OAM_MODULE_ID_SDT,

    OAM_MODULE_ID_BUTT
} oam_module_id_enum;
typedef uint16_t oam_module_id_enum_uint16;

/* 日志级别 */
typedef enum {
    OAM_LOG_LEVEL_ERROR = 1, /* ERROR级别打印 */
    OAM_LOG_LEVEL_WARNING,   /* WARNING级别打印 */
    OAM_LOG_LEVEL_INFO,      /* INFO级别打印 */

    OAM_LOG_LEVEL_BUTT
} oam_log_level_enum;
typedef uint8_t oam_log_level_enum_uint8;

#define OAM_LOG_DEFAULT_LEVEL OAM_LOG_LEVEL_WARNING

/* 配置文件，配置的类型 */
typedef enum {
    OAM_CFG_TYPE_MAX_ASOC_USER = 0,

    OAM_CFG_TYPE_BUTT
} oam_cfg_type_enum;
typedef uint16_t oam_cfg_type_enum_uint16;

/* 打印模式定义 */
typedef enum {
    OAM_OUTPUT_TYPE_CONSOLE = 0, /* 控制台输出 */
    OAM_OUTPUT_TYPE_SDT,         /* 输出到SDT */

    OAM_OUTPUT_TYPE_BUTT
} oam_output_type_enum;
typedef uint8_t oam_output_type_enum_uint8;

/* event上报事件消息类型 */
typedef enum {
    OAM_EVENT_WID_QUERY,            /* 配置命令消息，查询命令 */
    OAM_EVENT_WID_WRITE,            /* 配置命令消息，写命令 */
    OAM_EVENT_INTERNAL,             /* WiTP内部各层之间的事件消息 */
    OAM_EVENT_VAP_STATE_CHANGE,     /* VAP状态迁移事件消息 */
    OAM_EVENT_PAUSE_OR_RESUME_TID,  /* 暂停或者恢复tid */
    OAM_EVENT_PAUSE_OR_RESUME_USER, /* 暂停或者恢复user消息 */
    OAM_EVENT_VAP_CHANGE_CHANNEL,   /* VAP切换信道 */
    OAM_EVENT_USER_INFO_CHANGE,     /* 用户状态信息或者能力信息变化 */
    OAM_EVENT_USER_TXRX_PARAM,      /* 用户收发参数 */
    OAM_EVENT_REGISTER = 9,         /* 寄存器 */

    OAM_EVENT_TYPE_BUTT
} oam_event_type_enum;
typedef uint16_t oam_event_type_enum_uint16;

/* OTA上报事件类型 */
typedef enum {
    /* 默认关闭上报 */
    OAM_OTA_TYPE_RX_DSCR,     /* 接收描述符 */
    OAM_OTA_TYPE_TX_DSCR,     /* 发送描述符 */
    OAM_OTA_TYPE_80211_FRAME, /* 80211帧(不包括beacon) */
    OAM_OTA_TYPE_BEACON,      /* beacon */
    OAM_OTA_TYPE_ETH_FRAME,   /* 以太网帧 */
    OAM_OTA_TYPE_RX_DMAC_CB,
    OAM_OTA_TYPE_RX_HMAC_CB,
    OAM_OTA_TYPE_TX_CB,
    OAM_OTA_TYPE_TIMER_TRACK, /* 定时器创建删除的行号和文件号追踪 ,已废弃 */

    /* 默认打开上报 */
    OAM_OTA_TYPE_IRQ,         /* 中断信息 */
    OAM_OTA_TYPE_TIMER,       /* 软件定时器信息 */
    OAM_OTA_TYPE_MEMPOOL,     /* 某一内存池及所有子池使用信息 */
    OAM_OTA_TYPE_MEMBLOCK,    /* 某一内存池的所有内存块使用信息及内容，或者任意连续内存块的内容 */
    OAM_OTA_TYPE_EVENT_QUEUE, /* 当前所有存在事件的事件队列中的事件个数和事件头信息 */
    OAM_OTA_TYPE_MPDU_NUM,
    OAM_OTA_TYPE_PHY_STAT,          /* phy收发包统计值 */
    OAM_OTA_TYPE_MACHW_STAT,        /* mac收发包统计值 */
    OAM_OTA_TYPE_MGMT_STAT,         /* 管理帧统计 */
    OAM_OTA_TYPE_DBB_ENV_PARAM,     /* 空口环境类维测参数 */
    OAM_OTA_TYPE_USR_QUEUE_STAT,    /* 用户队列统计信息 */
    OAM_OTA_TYPE_VAP_STAT,          /* vap吞吐相关统计信息 */
    OAM_OTA_TYPE_USER_THRPUT_PARAM, /* 影响用户实时吞吐相关的统计信息 */
    OAM_OTA_TYPE_AMPDU_STAT,        /* ampdu业务流程统计信息 */
    OAM_OTA_TYPE_HARDWARE_INFO,     /* hal mac相关信息 */
    OAM_OTA_TYPE_USER_QUEUE_INFO,   /* 用户队列信息 */

    /* VAP与USER等结构体的信息，上报整个结构体内存 */
    OAM_OTA_TYPE_HMAC_VAP,
    OAM_OTA_TYPE_DMAC_VAP,
    OAM_OTA_TYPE_HMAC_USER,
    OAM_OTA_TYPE_MAC_USER,
    OAM_OTA_TYPE_DMAC_USER,

    /* hmac 与 dmac vap中部分成员的大小 */
    OAM_OTA_TYPE_HMAC_VAP_MEMBER_SIZE,
    OAM_OTA_TYPE_DMAC_VAP_MEMBER_SIZE,

    /* 三种级别的统计信息,这三个必放到一起!!! */
    OAM_OTA_TYPE_DEV_STAT_INFO,
    OAM_OTA_TYPE_VAP_STAT_INFO,
    OAM_OTA_TYPE_USER_STAT_INFO,

    OAM_OTA_TYPE_PHY_BANK1_INFO, /* PHY寄存器 BANK1 信息 */
    OAM_OTA_TYPE_PHY_BANK2_INFO, /* PHY寄存器 BANK2 信息 */
    OAM_OTA_TYPE_PHY_BANK3_INFO, /* PHY寄存器 BANK3 信息 */
    OAM_OTA_TYPE_PHY_BANK4_INFO, /* PHY寄存器 BANK4 信息 */
    /* 03新增PHY寄存器5、6 */
    OAM_OTA_TYPE_PHY_BANK5_INFO, /* PHY寄存器 BANK5 信息 */
    OAM_OTA_TYPE_PHY_BANK6_INFO, /* PHY寄存器 BANK6 信息 */
    OAM_OTA_TYPE_PHY0_CTRL_INFO, /* PHY寄存器 PHY0_CTRL 信息 */
    OAM_OTA_TYPE_PHY1_CTRL_INFO, /* PHY寄存器 PHY1_CTRL 信息 */

    OAM_OTA_TYPE_MAC_BANK0_INFO, /* MAC寄存器 BANK0 信息 */
    OAM_OTA_TYPE_MAC_BANK1_INFO, /* MAC寄存器 BANK1 信息 */
    OAM_OTA_TYPE_MAC_BANK2_INFO, /* MAC寄存器 BANK2 信息 */
    OAM_OTA_TYPE_MAC_BANK3_INFO, /* MAC寄存器 BANK3 信息（存在几字节信息不能读取） */
    OAM_OTA_TYPE_MAC_BANK4_INFO, /* MAC寄存器 BANK4 信息 */

    OAM_OTA_TYPE_RF_REG_INFO,  /* RF寄存器 */
    OAM_OTA_TYPE_SOC_REG_INFO, /* SOC寄存器 */

    /* 03 MAC寄存器列表 */
    OAM_OTA_TYPE_MAC_CTRL0_BANK_INFO, /* MAC寄存器 BANK0 信息 */
    OAM_OTA_TYPE_MAC_CTRL1_BANK_INFO, /* MAC寄存器 BANK1 信息 */
    OAM_OTA_TYPE_MAC_CTRL2_BANK_INFO, /* MAC寄存器 BANK2 信息 */
    OAM_OTA_TYPE_MAC_RD0_BANK_INFO,   /* MAC寄存器 BANK3 信息（存在几字节信息不能读取） */
    OAM_OTA_TYPE_MAC_RD1_BANK_INFO,   /* MAC寄存器 BANK4 信息 */
    OAM_OTA_TYPE_MAC_LUT0_BANK_INFO,  /* MAC寄存器 BANK5 信息 */
    OAM_OTA_TYPE_MAC_WLMAC_CTRL_INFO, /* MAC寄存器 wlmac_ctrl 信息 */
    OAM_OTA_TYPE_RF0_REG_INFO,        /* 03 RF0寄存器 */
    OAM_OTA_TYPE_RF1_REG_INFO,        /* 03 RF1寄存器 */

    /* 03 vector维测新增上报 */
    OAM_OTA_TYPE_RX_HDR,
    OAM_OTA_TYPE_RX_HDR_WORD0_INFO = OAM_OTA_TYPE_RX_HDR, /* trailer word0信息 */
    OAM_OTA_TYPE_RX_HDR_WORD1_INFO, /* trailer word1信息 */
    OAM_OTA_TYPE_RX_HDR_WORD2_INFO, /* trailer word2信息 */
    OAM_OTA_TYPE_RX_HDR_WORD3_INFO, /* trailer word3信息 */
    OAM_OTA_TYPE_RX_HDR_WORD4_INFO, /* trailer word4信息 */
    OAM_OTA_TYPE_RX_HDR_WORD5_INFO, /* trailer word5信息 */
    OAM_OTA_TYPE_RX_HDR_WORD6_INFO, /* trailer word6信息 */
    OAM_OTA_TYPE_RX_HDR_WORD7_INFO, /* trailer word7信息 */

    /* 03 trailer维测新增上报 */
    OAM_OTA_TYPE_RX_TRLR,
    OAM_OTA_TYPE_RX_TRLR_WORD0_INFO = OAM_OTA_TYPE_RX_TRLR,  /* trailer word0信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD1_INFO,  /* trailer word1信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD2_INFO,  /* trailer word2信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD3_INFO,  /* trailer word3信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD4_INFO,  /* trailer word4信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD5_INFO,  /* trailer word5信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD6_INFO,  /* trailer word6信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD7_INFO,  /* trailer word7信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD8_INFO,  /* trailer word8信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD9_INFO,  /* trailer word9信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD10_INFO, /* trailer word10信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD11_INFO, /* trailer word11信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD12_INFO, /* trailer word12信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD13_INFO, /* trailer word13信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD14_INFO, /* trailer word14信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD15_INFO, /* trailer word15信息 */

    /* 03新增PHY寄存器ABB_CALI_WL_CTRL */
    OAM_OTA_TYPE_ABB_CALI_WL_CTRL0_INFO, /* PHY寄存器 ABB_CALI_WL_CTRL0 信息 */
    OAM_OTA_TYPE_ABB_CALI_WL_CTRL1_INFO, /* PHY寄存器 ABB_CALI_WL_CTRL1 信息 */

    /* 03 SOC维测新增上报 */
    OAM_OTA_TYPE_SOC_RF_W_C0_CTL_INFO, /* SOC寄存器 RF_W_C0_CTL 信息 */
    OAM_OTA_TYPE_SOC_RF_W_C1_CTL_INFO, /* SOC寄存器 RF_W_C1_CTL 信息 */
    OAM_OTA_TYPE_SOC_W_CTL_INFO,       /* SOC寄存器 W_CTL 信息 */
    OAM_OTA_TYPE_SOC_COEX_CTL_INFO,    /* SOC寄存器 COEX_CTL 信息 */
    OAM_OTA_TYPE_SOC_RESERVE1_INFO,    /* SOC寄存器 Reserve1 信息 */
    OAM_OTA_TYPE_SOC_RESERVE2_INFO,    /* SOC寄存器 Reserve2 信息 */
    OAM_OTA_TYPE_SOC_RESERVE3_INFO,    /* SOC寄存器 Reserve3 信息 */
    OAM_OTA_TYPE_SOC_RESERVE4_INFO,    /* SOC寄存器 Reserve4 信息 */

    /* 03PILOT寄存器 */
    OAM_OTA_TYPE_PHY_BANK1_PILOT_INFO, /* PHY寄存器 BANK1 信息 */
    OAM_OTA_TYPE_PHY_BANK2_PILOT_INFO, /* PHY寄存器 BANK2 信息 */
    OAM_OTA_TYPE_PHY_BANK3_PILOT_INFO, /* PHY寄存器 BANK3 信息 */
    OAM_OTA_TYPE_PHY_BANK4_PILOT_INFO, /* PHY寄存器 BANK4 信息 */
    OAM_OTA_TYPE_PHY_BANK5_PILOT_INFO, /* PHY寄存器 BANK5 信息 */
    OAM_OTA_TYPE_PHY_BANK6_PILOT_INFO, /* PHY寄存器 BANK6 信息 */
    OAM_OTA_TYPE_PHY0_CTRL_PILOT_INFO, /* PHY寄存器 PHY0_CTRL 信息 */
    OAM_OTA_TYPE_PHY1_CTRL_PILOT_INFO, /* PHY寄存器 PHY1_CTRL 信息 */

    OAM_OTA_TYPE_MAC_CTRL0_BANK_PILOT_INFO, /* MAC寄存器 BANK0 信息 */
    OAM_OTA_TYPE_MAC_CTRL1_BANK_PILOT_INFO, /* MAC寄存器 BANK1 信息 */
    OAM_OTA_TYPE_MAC_CTRL2_BANK_PILOT_INFO, /* MAC寄存器 BANK2 信息 */
    OAM_OTA_TYPE_MAC_CTRL3_BANK_INFO,       /* MAC寄存器 BANK3 信息 存在几字节信息不能读取 */
    OAM_OTA_TYPE_MAC_RD0_BANK_PILOT_INFO,   /* MAC寄存器 BANK3 信息（存在几字节信息不能读取） */
    OAM_OTA_TYPE_MAC_RD1_BANK_PILOT_INFO,   /* MAC寄存器 BANK4 信息 */
    OAM_OTA_TYPE_MAC_LUT0_BANK_PILOT_INFO,  /* MAC寄存器 BANK5 信息 */
    OAM_OTA_TYPE_MAC_WLMAC_CTRL_PILOT_INFO, /* MAC寄存器 wlmac_ctrl 信息 */

    OAM_OTA_TYPE_ABB_CALI_WL_CTRL0_PILOT_INFO, /* PHY寄存器 ABB_CALI_WL_CTRL0 信息 */
    OAM_OTA_TYPE_ABB_CALI_WL_CTRL1_PILOT_INFO, /* PHY寄存器 ABB_CALI_WL_CTRL1 信息 */

    OAM_OTA_TYPE_SOC_RF_W_C0_CTL_PILOT_INFO, /* SOC寄存器 RF_W_C0_CTL 信息 */
    OAM_OTA_TYPE_SOC_RF_W_C1_CTL_PILOT_INFO, /* SOC寄存器 RF_W_C1_CTL 信息 */
    OAM_OTA_TYPE_SOC_W_CTL_PILOT_INFO,       /* SOC寄存器 W_CTL 信息 */
    OAM_OTA_TYPE_SOC_COEX_CTL_PILOT_INFO,    /* SOC寄存器 COEX_CTL 信息 */

    OAM_OTA_TYPE_RX_DSCR_PILOT, /* 接收描述符 */
    OAM_OTA_TYPE_TX_DSCR_PILOT, /* 发送描述符 */

    /* 06 MSDU描述符 */
    OAM_OTA_TYPE_RX_MSDU_DSCR,                      /* 接收MPDU描述符 */
    OAM_OTA_TYPE_RX_PPDU_DSCR,                      /* 接收PPDU描述符 */
    OAM_OTA_TYPE_TX_PPDU_DSCR,                      /* 发送PPDU描述符 */
    OAM_OTA_TYPE_TX_COMPLETE_PPDU_DSCR,             /* 发送完成PPDU描述符 */
    OAM_OTA_TYPE_ALG_TX1_TYPE,                      /* 给算法预留上报接口 */
    OAM_OTA_TYPE_ALG_RX1_TYPE,                      /* 给算法预留上报接口 */

    OAM_OTA_TYPE_BA_INFO,                           /* 发送后BA_INFO   */
    OAM_OTA_TYPE_TX_RING_INFO,
    OAM_OTA_TYPE_RX_RING_INFO,

    /* 06 PHY寄存器 */
    OAM_OTA_TYPE_PHY_TIME_CRG_CTL_RB_INFO,         /* PHY寄存器 时域CRG */
    OAM_OTA_TYPE_PHY_PHY_GLB_REG_BANK_INFO,        /* PHY寄存器 全局控制 */
    OAM_OTA_TYPE_PHY_CALI_TEST_REG_BANK_0_INFO,    /* PHY寄存器 RF校准BANK0 */
    OAM_OTA_TYPE_PHY_CALI_TEST_REG_BANK_1_INFO_0,  /* PHY寄存器 RF校准  BANK1 */
    OAM_OTA_TYPE_PHY_CALI_TEST_REG_BANK_1_INFO_1,  /* PHY寄存器 RF校准  BANK1 */
    OAM_OTA_TYPE_PHY_RX_TIME_DOMAIN_REG_BANK_INFO, /* PHY寄存器 除AGC外的RX方向时域处理 */
    OAM_OTA_TYPE_PHY_RX_AGC_REG_BANK_INFO,         /* PHY寄存器 RX_AGC  */
    OAM_OTA_TYPE_PHY_TX_DFE_REG_BANK_INFO,         /* PHY寄存器 TX方向时域处理  */
    OAM_OTA_TYPE_PHY_DPD_REG_BANK_INFO_0,          /* PHY寄存器 DPD  */
    OAM_OTA_TYPE_PHY_DPD_REG_BANK_INFO_1,          /* PHY寄存器 DPD  */
    OAM_OTA_TYPE_PHY_DFX_REG_BANK_INFO,            /* PHY寄存器 维测配置、寄存器上报  */
    OAM_OTA_TYPE_PHY_RF_CTRL_REG_BANK_0_INFO_0,    /* PHY寄存器 RF/ABB相关接口配置  */
    OAM_OTA_TYPE_PHY_RF_CTRL_REG_BANK_0_INFO_1,    /* PHY寄存器 RF/ABB相关接口配置  */
    OAM_OTA_TYPE_PHY_RF_CTRL_REG_BANK_1_INFO_0,    /* PHY寄存器 RF/ABB相关接口配置2ch，3ch  */
    OAM_OTA_TYPE_PHY_RF_CTRL_REG_BANK_1_INFO_1,    /* PHY寄存器 RF/ABB相关接口配置2ch，3ch  */
    OAM_OTA_TYPE_PHY_TPC_REG_BANK_INFO,            /* PHY寄存器 TPC档位控制  */
    OAM_OTA_TYPE_PHY_DOTB_REG_BANK_INFO,           /* PHY寄存器 11B相关配置  */
    OAM_OTA_TYPE_PHY_RX_FREQ_CRG_CTL_RB_INFO,      /* PHY寄存器 RX 频域CRG，已删除  */
    OAM_OTA_TYPE_PHY_RX_FREQ_DOMAIN_REG_BANK_INFO, /* PHY寄存器 RX方向频域处理  */
    OAM_OTA_TYPE_PHY_RX_CHN_DEC_REG_BANK_INFO_0,   /* PHY寄存器 译码器 */
    OAM_OTA_TYPE_PHY_RX_CHN_DEC_REG_BANK_INFO_1,   /* PHY寄存器 译码器 */
    OAM_OTA_TYPE_PHY_RX_MIMO_DET_REG_BANK_INFO_0,  /* PHY寄存器 解调 */
    OAM_OTA_TYPE_PHY_RX_MIMO_DET_REG_BANK_INFO_1,  /* PHY寄存器 解调 */
    OAM_OTA_TYPE_PHY_TX_FREQ_REG_BANK_INFO_0,      /* PHY寄存器 TX方向频域处理 */
    OAM_OTA_TYPE_PHY_TX_FREQ_REG_BANK_INFO_1,      /* PHY寄存器 TX方向频域处理 */
    OAM_OTA_TYPE_PHY_ABB_ADC_CALI_RB_0_INFO,       /* PHY寄存器 ADC校准 */
    OAM_OTA_TYPE_PHY_ABB_ADC_CALI_RB_1_INFO,       /* PHY寄存器 ADC校准 */
    OAM_OTA_TYPE_PHY_ABB_ADC_CALI_RB_2_INFO,       /* PHY寄存器 ADC校准 */
    OAM_OTA_TYPE_PHY_ABB_ADC_CALI_RB_3_INFO,       /* PHY寄存器 ADC校准 */
    OAM_OTA_TYPE_PHY_ABB_DAC_CALI_RB_0_INFO_0,     /* PHY寄存器 DAC校准 */
    OAM_OTA_TYPE_PHY_ABB_DAC_CALI_RB_0_INFO_1,     /* PHY寄存器 DAC校准 */
    OAM_OTA_TYPE_PHY_ABB_DAC_CALI_RB_1_INFO_0,     /* PHY寄存器 DAC校准 */
    OAM_OTA_TYPE_PHY_ABB_DAC_CALI_RB_1_INFO_1,     /* PHY寄存器 DAC校准 */
    OAM_OTA_TYPE_PHY_RAP_REG_BANK_INFO,            /* PHY寄存器 单雷达PSD天线 */

    /* 06 MAC寄存器 */
    OAM_OTA_TYPE_MAC_CFG_HOST,                     /* MAC寄存器 HOST配置寄存器 */
    OAM_OTA_TYPE_MAC_CFG_DMAC0,                    /* MAC寄存器 DMAC配置寄存器0 */
    OAM_OTA_TYPE_MAC_CFG_DMAC1,                    /* MAC寄存器 DMAC配置寄存器1 */
    OAM_OTA_TYPE_MAC_CFG_DMAC2,                    /* MAC寄存器 DMAC配置寄存器2 */
    OAM_OTA_TYPE_MAC_CFG_DMAC3,                    /* MAC寄存器 DMAC配置寄存器3 */
    OAM_OTA_TYPE_MAC_CFG_DMAC4,                    /* MAC寄存器 DMAC配置寄存器4 */
    OAM_OTA_TYPE_MAC_CFG_DMAC5,                    /* MAC寄存器 DMAC配置寄存器5 */
    OAM_OTA_TYPE_MAC_CFG_SMAC,                     /* MAC寄存器 SMAC配置寄存器 */
    OAM_OTA_TYPE_MAC_RPT_HOST,                     /* MAC寄存器 HOST统计寄存器 */
    OAM_OTA_TYPE_MAC_RPT_DMAC,                     /* MAC寄存器 DMAC统计寄存器 */
    OAM_OTA_TYPE_MAC_PRT_SMAC,                     /* MAC寄存器 SMAC统计寄存器 */

    /* 06 RF寄存器 */
    OAM_OTA_TYPE_RF0_REG_INFO_0,                   /* 06 RF0 寄存器 */
    OAM_OTA_TYPE_RF0_REG_INFO_1,                   /* 06 RF0 寄存器 */
    OAM_OTA_TYPE_RF0_PLL_REG_INFO,                 /* 06 RF0 寄存器 PLL */
    OAM_OTA_TYPE_RF1_REG_INFO_0,                   /* 06 RF1 寄存器 */
    OAM_OTA_TYPE_RF1_REG_INFO_1,                   /* 06 RF1 寄存器 */
    OAM_OTA_TYPE_RF1_PLL_REG_INFO,                 /* 06 RF1 寄存器 PLL */
    OAM_OTA_TYPE_RF2_REG_INFO_0,                   /* 06 RF2 寄存器 */
    OAM_OTA_TYPE_RF2_REG_INFO_1,                   /* 06 RF2 寄存器 */
    OAM_OTA_TYPE_RF2_PLL_REG_INFO,                 /* 06 RF2 寄存器 PLL */
    OAM_OTA_TYPE_RF3_REG_INFO_0,                   /* 06 RF3 寄存器 */
    OAM_OTA_TYPE_RF3_REG_INFO_1,                   /* 06 RF3 寄存器 */
    OAM_OTA_TYPE_RF3_PLL_REG_INFO,                 /* 06 RF3 寄存器 PLL */

    /* 06 SOC寄存器 */
    OAM_OTA_TYPE_SOC_GLB_CTL_0,
    OAM_OTA_TYPE_SOC_GLB_CTL_1,
    OAM_OTA_TYPE_SOC_PMU_CMU_CTL,
    OAM_OTA_TYPE_SOC_PMU2_TS_EF,
    OAM_OTA_TYPE_SOC_W_CTL,
    OAM_OTA_TYPE_SOC_WCPU_CTL,
    OAM_OTA_TYPE_SOC_WL_RF_ABB_C0,
    OAM_OTA_TYPE_SOC_WL_RF_ABB_C1,
    OAM_OTA_TYPE_SOC_WL_RF_ABB_C2,
    OAM_OTA_TYPE_SOC_WL_RF_ABB_C3,
    OAM_OTA_TYPE_SOC_WL_RF_ABB_COM,
    OAM_OTA_TYPE_SOC_PCIE_CTRL,
    OAM_OTA_TYPE_SOC_HOST_CTRL_0,
    OAM_OTA_TYPE_SOC_HOST_CTRL_1,
    OAM_OTA_TYPE_SOC_COEX_CTRL,

    OAM_OTA_TYPE_BUTT
} oam_ota_type_enum;
typedef uint8_t oam_ota_type_enum_uint8;

/* 区分版本，修改需要和sdt工具同步 */
typedef enum {
    OAM_OTA_TYPE_CHIP_INVALID = 0,
    OAM_OTA_TYPE_1151_DEV = 1,
    OAM_OTA_TYPE_1102_HOST,
    OAM_OTA_TYPE_1151_HOST,
    OAM_OTA_TYPE_1102_DEV,
    OAM_OTA_TYPE_1103_HOST = 5,
    OAM_OTA_TYPE_1103_DEV,
    OAM_OTA_TYPE_1102A_HOST,
    OAM_OTA_TYPE_1102A_DEV,
    OAM_OTA_TYPE_1105_DEV = 11,
    OAM_OTA_TYPE_1105_HOST,
    OAM_OTA_TYPE_1106_DEV = 13,
    OAM_OTA_TYPE_1106_HOST = 14,
} oam_ota_board_type_enum;

typedef uint8_t oam_ota_board_type_enum_uint8;

/* 定时器追踪类型 */
typedef enum {
    OAM_TIMER_TRACK_TYPE_CREATE = 0,
    OAM_TIMER_TRACK_TYPE_DESTROY,
    OAM_TIMER_TRACK_TYPE_IMMEDIATE_DESTROY,

    OAM_TIMER_TRACK_TYPE_BUTT
} oam_timer_track_type_enum;
typedef uint8_t oam_timer_track_type_enum_uint8;

/* 单用户跟踪相关枚举 */
/* 单用户跟踪内容，帧内容，CB字段，描述符 */
typedef enum {
    OAM_USER_TRACK_CONTENT_FRAME = 0,
    OAM_USER_TRACK_CONTENT_CB,
    OAM_USER_TRACK_CONTENT_DSCR,

    OAM_USER_TRACK_CONTENT_BUTT
} oam_user_track_content_enum;
typedef uint8_t oam_user_track_content_enum_uint8;

/* 单用户跟踪帧类型:数据和管理 */
typedef enum {
    OAM_USER_TRACK_FRAME_TYPE_MGMT = 0,
    OAM_USER_TRACK_FRAME_TYPE_DATA,

    OAM_USER_TRACK_FRAME_TYPE_BUTT
} oam_user_track_frame_type_enum;
typedef uint8_t oam_user_track_frame_type_enum_uint8;

/* 跟踪用户信息变化类型，event类型信息，状态变化驱动 */
typedef enum {
    OAM_USER_INFO_CHANGE_TYPE_ASSOC_STATE = 0, /* 用户关联状态 */
    OAM_USER_INFO_CHANGE_TYPE_TX_PROTOCOL,     /* 发送数据帧使用的协议模式 */
    OAM_USER_INFO_CHANGE_TYPE_RX_PROTOCOL,     /* 接收数据帧使用的协议模式 */
    OAM_USER_INFO_CHANGE_TYPE_BUTT
} oam_user_info_change_type_enum;
typedef uint8_t oam_user_info_change_type_enum_uint8;

/* 跟踪用户收发参数,event类型信息,配置命令驱动 */
typedef enum {
    OAM_USER_TXRX_PARAM_TYPE_RSSI = 0,
    OAM_USER_TXRX_PARAM_TYPE_RATE,

    OAM_USER_TXRX_PARAM_TYPE_BUTT
} oam_user_txrx_param_type_enum;
typedef uint8_t oam_user_txrx_param_type_enum_uint8;

/* 统计信息上报类型，分为device级别，vap级别，user级别 */
typedef enum {
    OAM_STATS_TYPE_DEVICE,
    OAM_STATS_TYPE_VAP,
    OAM_STATS_TYPE_USER,

    OAM_STATS_TYPE_BUTT
} oam_stats_type_enum;
typedef uint8_t oam_stats_type_enum_uint8;

/* device级别统计信息上报子类型， */
typedef enum {
    OAM_STATS_DEVICE_SUBTYPE_IRQ,
    OAM_STATS_DEVICE_SUBTYPE_TIMER,
    OAM_STATS_DEVICE_SUBTYPE_MEMPOOL,
    OAM_STATS_DEVICE_SUBTYPE_EVENT_QUEUE,

    OAM_STATS_DEVICE_SUBTYPE_BUTT
} oam_stats_device_subtype_enum;
typedef uint8_t oam_stats_device_subtype_enum_uint8;

/* ALARM上报事件类型 */
typedef enum {
    OAM_ALARM_TYPE_MEM_OVER_LEAK,
    OAM_ALARM_TYPE_PKT_RECV_LOST,
    OAM_ALARM_TYPE_PKT_SEND_LOST,

    OAM_ALARM_TYPE_BUTT
} oam_alarm_type_enum;
typedef uint16_t oam_alarm_type_enum_uint16;

/* 内存块信息上报类型 */
typedef enum {
    OAM_MEMBLOCK_INFO_TYPE_UNIFORM,     /* 某一个内存池的所有内存块信息 */
    OAM_MEMBLOCK_INFO_TYPE_NON_UNIFORM, /* 业务流程中需要查看的任意连续内存的信息 */

    OAM_MEMBLOCK_INFO_TYPE_BUTT
} oam_memblock_info_type_enum;
typedef uint8_t oam_memblock_info_type_enum_uint8;

/* STURCT上报事件类型 */
typedef enum {
    OAM_STRUCT_TYPE_DEVICE,
    OAM_STRUCT_TYPE_HMAC_VAP,
    OAM_STRUCT_TYPE_MAC_VAP,
    OAM_STRUCT_TYPE_DMAC_VAP,
    OAM_STRUCT_TYPE_HMAC_USER,
    OAM_STRUCT_TYPE_MAC_USER,
    OAM_STRUCT_TYPE_DMAC_USER,
    OAM_STRUCT_TYPE_TID,

    OAM_STRUCT_TYPE_BUTT
} oam_struct_type_enum;

/* SDT打印beacon帧和beacon帧接收描述符的开关类型,其中beacon帧的打印收发流程都适用 */
typedef enum {
    OAM_SDT_PRINT_BEACON_RXDSCR_TYPE_BEACON = 1, /* 打印beacon帧，不打印beacon帧的接收描述符 */
    OAM_SDT_PRINT_BEACON_RXDSCR_TYPE_RXDSCR,     /* 打印beacon帧的接收描述符，不打印beacon帧 */
    OAM_SDT_PRINT_BEACON_RXDSCR_TYPE_BOTH,       /* 打印beacon帧以及接收描述符 */

    OAM_SDT_PRINT_BEACON_RXDSCR_TYPE_BUTT
} oam_sdt_print_beacon_rxdscr_type_enum;
typedef uint8_t oam_sdt_print_beacon_rxdscr_type_enum_uint8;


/* 表明帧是属于接收流程还是发送流程的枚举 */
typedef enum {
    OAM_OTA_FRAME_DIRECTION_TYPE_TX = 0,
    OAM_OTA_FRAME_DIRECTION_TYPE_RX,

    OAM_OTA_FRAME_DIRECTION_TYPE_BUTT
} oam_ota_frame_direction_type_enum;
typedef uint8_t oam_ota_frame_direction_type_enum_uint8;

typedef enum {
    OAM_NL_CMD_SDT = 0, /* SDT模块 */
    OAM_NL_CMD_HUT,     /* HUT模块 */
    OAM_NL_CMD_ALG,     /* ALG模块 */
    OAM_NL_CMD_ACS,     /* ACS模块 */
    OAM_NL_CMD_DAQ,
    OAM_NL_CMD_REG,  /* 寄存器读取模块 */
    OAM_NL_CMD_PSTA, /* proxysta */

    OAM_NL_CMD_BUTT
} oam_nl_cmd_enum;
typedef uint8_t oam_nl_cmd_enum_uint8;

/* 特性宏的缩写见gst_oam_feature_list */
typedef enum {
    OAM_SF_SCAN = 0,
    OAM_SF_AUTH,
    OAM_SF_ASSOC,
    OAM_SF_FRAME_FILTER,
    OAM_SF_WMM,

    OAM_SF_DFS = 5,
    OAM_SF_NETWORK_MEASURE,
    OAM_SF_ENTERPRISE_VO,
    OAM_SF_HOTSPOTROAM,
    OAM_SF_NETWROK_ANNOUNCE,

    OAM_SF_NETWORK_MGMT = 10,
    OAM_SF_NETWORK_PWS,
    OAM_SF_PROXYARP,
    OAM_SF_TDLS,
    OAM_SF_CALIBRATE,

    OAM_SF_EQUIP_TEST = 15,
    OAM_SF_CRYPTO,
    OAM_SF_WPA,
    OAM_SF_WEP,
    OAM_SF_WPS,

    OAM_SF_PMF = 20,
    OAM_SF_WAPI,
    OAM_SF_BA,
    OAM_SF_AMPDU,
    OAM_SF_AMSDU,

    OAM_SF_STABILITY = 25,
    OAM_SF_TCP_OPT,
    OAM_SF_ACS,
    OAM_SF_AUTORATE,
    OAM_SF_TXBF,

    OAM_SF_DYN_RECV = 30, /* dynamin recv */
    OAM_SF_VIVO,          /* video_opt voice_opt */
    OAM_SF_MULTI_USER,
    OAM_SF_MULTI_TRAFFIC,
    OAM_SF_ANTI_INTF,

    OAM_SF_EDCA = 35,
    OAM_SF_SMART_ANTENNA,
    OAM_SF_TPC,
    OAM_SF_TX_CHAIN,
    OAM_SF_RSSI,

    OAM_SF_WOW = 40,
    OAM_SF_GREEN_AP,
    OAM_SF_PWR, /* psm uapsd fastmode */
    OAM_SF_SMPS,
    OAM_SF_TXOP,

    OAM_SF_WIFI_BEACON = 45,
    OAM_SF_KA_AP, /* keep alive ap */
    OAM_SF_MULTI_VAP,
    OAM_SF_2040, /* 20m+40m coex */
    OAM_SF_DBAC,

    OAM_SF_PROXYSTA = 50,
    OAM_SF_UM,  /* user managment */
    OAM_SF_P2P, /* P2P 特性 */
    OAM_SF_M2U,
    OAM_SF_IRQ, /* top half */

    OAM_SF_TX = 55,
    OAM_SF_RX,
    OAM_SF_DUG_COEX,
    OAM_SF_CFG, /* wal dmac config函数 */
    OAM_SF_FRW, /* frw层 */

    OAM_SF_KEEPALIVE = 60,
    OAM_SF_COEX,
    OAM_SF_HS20, /* HotSpot 2.0特性 */
    OAM_SF_MWO_DET,
    OAM_SF_CCA_OPT,

    OAM_SF_ROAM = 65, /* roam module, #ifdef _PRE_WLAN_FEATURE_ROAM */
    OAM_SF_DFT,
    OAM_SF_DFR,
    OAM_SF_RRM,
    OAM_SF_VOWIFI,

    OAM_SF_OPMODE = 70,
    OAM_SF_M2S,
    OAM_SF_DBDC,
    OAM_SF_HILINK,
    OAM_SF_WDS, /* WDS 特性 */

    OAM_SF_WMMAC = 75,
    OAM_SF_USER_EXTEND,
    OAM_SF_PKT_CAP, /* 抓包特性 */
    OAM_SF_SOFT_CRYPTO,

    OAM_SF_CAR,  /* 限速特性  */
    OAM_SF_11AX, /* 11AX 特性 */
    OAM_SF_CSA,
    OAM_SF_CONN, /* ONT日志 */
    OAM_SF_CHAN, /* ONT日志 */
    OAM_SF_CUSTOM,

    OAM_SF_QOS,
    OAM_SF_INI,
#ifdef _PRE_WLAN_FEATURE_HIEX
    OAM_SF_HIEX,
#else
    OAM_SF_RESERVE5,
#endif
    OAM_SF_RESERVE6,
    OAM_SF_RESERVE7,
    OAM_SF_RESERVE8,
    OAM_SF_RESERVE9,
    OAM_SF_RESERVE10,
    OAM_SF_ANY, /*
                 * rifs protection shortgi frag datarate countrycode
                 * coustom_security startup_time lsig monitor wds
                 * hidessid
                 */
#ifdef _PRE_WLAN_FEATURE_FTM
    OAM_SF_FTM,
#endif
#ifdef _PRE_WLAN_FEATURE_WATERFILLING
    OAM_SF_WATERFILLING,
#endif
    OAM_SF_SAE,

    OAM_SOFTWARE_FEATURE_BUTT
} oam_feature_enum;
typedef uint8_t oam_feature_enum_uint8;

typedef enum {
    OAM_RATELIMIT_TYPE_PRINTK = 0,
    OAM_RATELIMIT_TYPE_LOG,
    OAM_RATELIMIT_TYPE_DSCR,
    OAM_RATELIMIT_TYPE_FRAME_WLAN,
    OAM_RATELIMIT_TYPE_FRAME_ETH,
    OAM_RATELIMIT_TYPE_CB,
    OAM_RATELIMIT_TYPE_BUTT
} oam_ratelimit_type_enum;
typedef uint8_t oam_ratelimit_type_enum_uint8;

#define OAM_EXCP_STATS_TIMEOUT 5000
/* 该枚举类型不建议频繁添加,只适用于异常原因明确的分支,周期上报异常触发日志 */
typedef enum {
    OAM_HAL_MAC_ERROR_PARA_CFG_ERR = 0,            /* 描述符参数配置异常,包括AMPDU长度配置不匹配,AMPDU中MPDU长度超长,sub msdu num错误 */
    OAM_HAL_MAC_ERROR_RXBUFF_LEN_TOO_SMALL = 1,    /* 接收非AMSDU帧长大于RxBuff大小异常 */
    OAM_HAL_MAC_ERROR_BA_ENTRY_NOT_FOUND = 2,      /* 未找到BA会话表项异常0 */
    OAM_HAL_MAC_ERROR_PHY_TRLR_TIME_OUT = 3,       /* PHY_RX_TRAILER超时 */
    OAM_HAL_MAC_ERROR_PHY_RX_FIFO_OVERRUN = 4,     /* PHY_RX_FIFO满写异常 */
    OAM_HAL_MAC_ERROR_TX_DATAFLOW_BREAK = 5,       /* 发送帧数据断流 */
    OAM_HAL_MAC_ERROR_RX_FSM_ST_TIMEOUT = 6,       /* RX_FSM状态机超时 */
    OAM_HAL_MAC_ERROR_TX_FSM_ST_TIMEOUT = 7,       /* TX_FSM状态机超时 */
    OAM_HAL_MAC_ERROR_RX_HANDLER_ST_TIMEOUT = 8,   /* RX_HANDLER状态机超时 */
    OAM_HAL_MAC_ERROR_TX_HANDLER_ST_TIMEOUT = 9,   /* TX_HANDLER状态机超时 */
    OAM_HAL_MAC_ERROR_TX_INTR_FIFO_OVERRUN = 10,   /* TX 中断FIFO满写 */
    OAM_HAL_MAC_ERROR_RX_INTR_FIFO_OVERRUN = 11,   /* RX中断 FIFO满写 */
    OAM_HAL_MAC_ERROR_HIRX_INTR_FIFO_OVERRUN = 12, /* HIRX中断FIFO满写 */
    OAM_HAL_MAC_ERROR_RX_Q_EMPTY = 13,             /* 接收到普通优先级帧但此时RX BUFFER指针为空 */
    OAM_HAL_MAC_ERROR_HIRX_Q_EMPTY = 14,           /* 接收到高优先级帧但此时HI RX BUFFER指针为空 */
    OAM_HAL_MAC_ERROR_BUS_RLEN_ERR = 15,           /* 总线读请求长度为0异常 */
    OAM_HAL_MAC_ERROR_BUS_RADDR_ERR = 16,          /* 总线读请求地址无效异常 */
    OAM_HAL_MAC_ERROR_BUS_WLEN_ERR = 17,           /* 总线写请求长度为0异常 */
    OAM_HAL_MAC_ERROR_BUS_WADDR_ERR = 18,          /* 总线写请求地址无效异常 */
    OAM_HAL_MAC_ERROR_TX_ACBK_Q_OVERRUN = 19,      /* tx acbk队列fifo满写 */
    OAM_HAL_MAC_ERROR_TX_ACBE_Q_OVERRUN = 20,      /* tx acbe队列fifo满写 */
    OAM_HAL_MAC_ERROR_TX_ACVI_Q_OVERRUN = 21,      /* tx acvi队列fifo满写 */
    OAM_HAL_MAC_ERROR_TX_ACVO_Q_OVERRUN = 22,      /* tx acv0队列fifo满写 */
    OAM_HAL_MAC_ERROR_TX_HIPRI_Q_OVERRUN = 23,     /* tx hipri队列fifo满写 */
    OAM_HAL_MAC_ERROR_MATRIX_CALC_TIMEOUT = 24,    /* matrix计算超时 */
    OAM_HAL_MAC_ERROR_CCA_TIMEOUT = 25,            /* cca超时 */
    OAM_HAL_MAC_ERROR_DCOL_DATA_OVERLAP = 26,      /* 数采overlap告警 */
    OAM_HAL_MAC_ERROR_BEACON_MISS = 27,            /* 连续发送beacon失败 */
    OAM_HAL_MAC_ERROR_UNKOWN_28 = 28,
    OAM_HAL_MAC_ERROR_UNKOWN_29 = 29,
    OAM_HAL_MAC_ERROR_UNKOWN_30 = 30,
    OAM_HAL_MAC_ERROR_UNKOWN_31 = 31,

    OAM_HAL_SOC_ERROR_BUCK_OCP = 32, /* PMU BUCK过流中断 */
    OAM_HAL_SOC_ERROR_BUCK_SCP,      /* PMU BUCK短路中断 */
    OAM_HAL_SOC_ERROR_OCP_RFLDO1,    /* PMU RFLDO1过流中断 */
    OAM_HAL_SOC_ERROR_OCP_RFLDO2,    /* PMU RFLDO2过流中断 */
    OAM_HAL_SOC_ERROR_OCP_CLDO,      /* PMU CLDO过流中断 */
    OAM_HAL_SOC_ERROR_RF_OVER_TEMP,  /* RF过热中断 */
    OAM_HAL_SOC_ERROR_CMU_UNLOCK,    /* CMU PLL失锁中断 */
    OAM_HAL_SOC_ERROR_PCIE_SLV_ERR,

    OAM_EXCP_TYPE_BUTT
} oam_excp_type_enum;
typedef uint8_t oam_excp_type_enum_uint8;

typedef enum {
    OAM_EXCP_STATUS_INIT = 0,  /* 初始状态:上报完成后切换至初始状态 */
    OAM_EXCP_STATUS_REFRESHED, /* 有更新，可上报； */
    OAM_EXCP_STATUS_BUTT
} oam_excp_status_enum;
typedef uint8_t oam_excp_status_enum_uint8;

extern uint8_t g_bcast_addr[WLAN_MAC_ADDR_LEN];
#define BROADCAST_MACADDR g_bcast_addr

/* STRUCT定义 */
/* SDT上传接口的注册函数类型 */
typedef int32_t (*oam_sdt_up_func)(oal_netbuf_stru *pst_netbuf);
typedef int32_t (*oam_sdt_report_data_func)(oal_netbuf_stru *pst_netbuf,
                                            oam_data_type_enum_uint8 en_type,
                                            oam_primid_type_enum_uint8 en_prim);
typedef int32_t (*oam_sdt_status_func)(void);

/* 其他模块调用SDT钩子函数 用结构体 */
typedef struct {
    int32_t (*p_sdt_report_data_func)(oal_netbuf_stru *pst_netbuf, oam_data_type_enum_uint8 en_type,
                                      oam_primid_type_enum_uint8 en_prim);
    int32_t (*p_sdt_get_wq_len_func)(void);
} oam_sdt_func_hook_stru;

/* 外部函数声明 */
extern oam_sdt_func_hook_stru g_oam_sdt_func_hook;

/* 其他模块调用WAL钩子函数 用结构体 */
typedef struct {
    int32_t (*p_wal_recv_mem_data_func)(uint8_t *puc_buf, uint16_t us_len);
    int32_t (*p_wal_recv_reg_data_func)(uint8_t *puc_buf, uint16_t us_len);
    int32_t (*p_wal_recv_cfg_data_func)(uint8_t *puc_buf, uint16_t us_len);
    int32_t (*p_wal_recv_global_var_func)(uint8_t *puc_buf, uint16_t us_len);
#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)
    int32_t (*p_wal_recv_sample_data_func)(uint8_t *puc_buf, uint16_t us_len);
#endif
} oam_wal_func_hook_stru;

extern oam_wal_func_hook_stru g_oam_wal_func_hook;

/* VAPID 日志级别结构体 */
typedef struct {
    oam_feature_enum_uint8 en_feature_id;
    oam_log_level_enum_uint8 en_log_level;
    oal_switch_enum_uint8 en_switch;
    uint8_t uc_resv;
} oam_cfg_log_level_stru;

typedef struct {
    uint8_t bit_vap_id : 4;
    uint8_t bit_log_level : 4;
} oam_vap_log_level_stru;

/* 日志消息结构体 */
typedef struct {
    oam_vap_log_level_stru st_vap_log_level; /* vapid + loglevel */
    oam_feature_enum_uint8 en_feature_id;    /* 特性id */
    uint16_t us_resv;
    uint16_t us_file_id;  /* 文件号 */
    uint16_t us_line_num; /* 行号 */
    uint32_t ul_tick;     /* 时间戳，精度ms */
    int32_t al_param[OAM_LOG_ENTRY_PARA_NUM];
} oam_log_info_stru;

/* OTA头部信息 */
typedef struct {
    uint32_t ul_tick;                  /* 时间戳，精度ms */
    oam_ota_type_enum_uint8 en_ota_type; /* OTA类型 */
    uint8_t uc_frame_hdr_len;          /* 如果是帧，代表帧头长度；如果是其它类型，此字段没有用 */
    uint16_t us_ota_data_len;          /* OTA数据长度，如果是帧，代表帧头和帧体的总长度 */
    uint8_t auc_user_macaddr[WLAN_MAC_ADDR_LEN];
    oam_ota_frame_direction_type_enum_uint8 en_frame_direction; /* 表明帧是属于发送流程还是接收流程，只有ota_type是帧的时候这个字段才有意义 */
    uint8_t auc_resv[1];                                      /* 区分产品 */
} oam_ota_hdr_stru;

/* OTA数据结构体 */
typedef struct {
    oam_ota_hdr_stru st_ota_hdr; /* OTA头部信息 */
    uint8_t auc_ota_data[4];   /* OTA数据固定4个字节 */
} oam_ota_stru;

/* 消息上报给SDT封装的结构体 */
/* EVENT消息体最大长度 */
#define OAM_EVENT_INFO_MAX_LEN 48

typedef struct {
    uint32_t ul_tick;                       /* 消息时间 */
    oam_event_type_enum_uint16 en_event_type; /* 消息类型，WID，抛事件，状态转移等 */
    oam_module_id_enum_uint16 en_module;      /* 模块 */
    uint8_t auc_user_macaddr[WLAN_MAC_ADDR_LEN];
    uint8_t uc_vap_id;   /* vap id */
    uint8_t auc_resv[1]; /* 保留 */
} oam_event_hdr_stru;

typedef struct {
    oam_event_hdr_stru st_event_hdr;                 /* EVENT头部信息 */
    int8_t auc_event_info[OAM_EVENT_INFO_MAX_LEN]; /* 消息内容，最多保存50个字节 */
} oam_event_stru;

#define OAM_EVENT_STRU_SIZE sizeof(oam_event_stru)

/* 内存池使用信息查询 */
typedef struct {
    uint8_t uc_mem_pool_id;
    uint8_t uc_subpool_id;
    uint8_t auc_resv[2];
    uint16_t us_mem_pool_total_cnt;
    uint16_t us_mem_pool_used_cnt;
    uint16_t us_subpool_total_cnt;
    uint16_t us_subpool_free_cnt;
} oam_stats_mempool_stru;

/* 内存块使用信息，包括标准内存块信息，和任意某一块连续内存信息 */
typedef struct {
    uint8_t uc_pool_id; /* 如果是任意连续内存 */
    uint8_t uc_subpool_id;
    uint8_t uc_user_cnt; /* 本内存块的引用计数 */
    uint8_t auc_resv[1];
    uint32_t ul_file_id;        /* 申请该内存块的文件号 */
    uint32_t ul_alloc_line_num; /* 申请该内存块的行号 */
} oam_memblock_info_stru;

/* 事件队列使用情况信息结构体 */
typedef struct {
    uint8_t en_type;      /* 事件类型 */
    uint8_t uc_sub_type;  /* 事件子类型 */
    uint16_t us_length;   /* 事件长度 */
    uint8_t en_pipeline;  /* 事件分段号 */
    uint8_t uc_chip_id;   /* 芯片ID */
    uint8_t uc_device_id; /* 设备ID */
    uint8_t uc_vap_id;    /* VAP ID */
} oam_event_hdr_info_stru;

/* 设备级别统计信息结构体 */
typedef struct {
    /* 中断个数统计信息 */
    /* 总的中断请求个数 */
    uint32_t ul_irq_cnt;

    /* SOC五类中断计数 */
    uint32_t ul_pmu_err_irq_cnt;      /* 电源错误中断 */
    uint32_t ul_rf_over_temp_irq_cnt; /* RF过热中断 */
    uint32_t ul_unlock_irq_cnt;       /* CMU PLL失锁中断 */
    uint32_t ul_mac_irq_cnt;          /* Mac业务中断 */
    uint32_t ul_pcie_slv_err_irq_cnt; /* PCIE总线错误中断 */

    /* pmu/buck错误子中断源计数 */
    uint32_t ul_buck_ocp_irq_cnt;   /* BUCK过流中断 */
    uint32_t ul_buck_scp_irq_cnt;   /* BUCK短路中断 */
    uint32_t ul_ocp_rfldo1_irq_cnt; /* PMU RFLDO1过流中断 */
    uint32_t ul_ocp_rfldo2_irq_cnt; /* PMU RFLDO2过流中断 */
    uint32_t ul_ocp_cldo_irq_cnt;   /* PMU CLDO过流中断 */

    /* MAC子中断源计数(与MACINTERRUPT_STATUS寄存器对应) */
    uint32_t ul_rx_complete_cnt;                           /* 数据帧接收完成 */
    uint32_t ul_tx_complete_cnt;                           /* 发送完成 */
    uint32_t ul_hi_rx_complete_cnt;                        /* 管理帧、控制帧接收完成 */
    uint32_t ul_error_cnt;                                 /* error中断 */
    uint32_t aul_tbtt_cnt[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT]; /* tbtt中断,0~3代表ap的，4代表sta的 */
    uint32_t ul_abort_intr_start;
    uint32_t ul_abort_intr_end;
    uint32_t ul_radar_detect_cnt; /* 检测到雷达 */
    uint32_t ul_pa_tx_suspended;
    uint32_t ul_de_authentication;
    uint32_t ul_one_packet;
    uint32_t ul_abort_intr_done;
    uint32_t ul_ch_statistics_cnt; /* 信道统计完成中断 */
    uint32_t ul_wow_prep_done;
    uint32_t ul_other_int_cnt; /* 其它 */

    /* MAC错误中断类型, 注意此处枚举值与错误中断状态寄存器的位一一对应 ! */
    uint32_t aul_mac_error_cnt[OAM_MAC_ERROR_TYPE_CNT];

    /* 接收中断上报频度统计 */
    uint32_t ul_normal_rx_idx; /* 当前中断时间戳所在位置 */
    uint32_t ul_hi_rx_idx; /* 当前中断时间戳所在位置 */
    uint32_t aul_rx_freq[OAM_RX_DSCR_QUEUE_CNT][OAM_IRQ_FREQ_STAT_MEMORY]; /* 接收频度统计，时间戳精度10ns */
    uint32_t ul_rx_freq_stop_flag; /* 接收频度标志，一旦上报fifo overun之后则停止统计 */

    /* 中断时延统计, 单位10ns */
    uint32_t ul_max_delay;   /* 最大时长 */
    uint32_t ul_min_delay;   /* 最小时长 */
    uint32_t ul_delay_sum;   /* 总时长 */
    uint32_t ul_timer_start; /* 开始时间记录 */
} oam_device_stat_info_stru;

/* VAP级别统计信息结构 */
typedef struct {
    /* 接收包统计 */
    /* 发往lan的数据包统计 */
    uint32_t ul_rx_pkt_to_lan;   /* 接收流程发往以太网的数据包数目，MSDU */
    uint32_t ul_rx_bytes_to_lan; /* 接收流程发往以太网的字节数 */

    /* 接收流程遇到严重错误(描述符异常等)，释放所有MPDU的统计 */
    uint32_t ul_rx_ppdu_dropped; /* 硬件上报的vap_id错误，释放的MPDU个数 */

    /* 软件统计的接收到MPDU个数，正常情况下应该与MAC硬件统计值相同 */
    uint32_t ul_rx_mpdu_total_num; /* 接收流程上报到软件进行处理的MPDU总个数 */

    /* MPDU级别进行处理时发生错误释放MPDU个数统计 */
    uint32_t ul_rx_ta_check_dropped;        /* 检查发送端地址异常，释放一个MPDU */
    uint32_t ul_rx_da_check_dropped;        /* 检查目的端地址异常，释放一个MPDU */
    uint32_t ul_rx_phy_perform_dropped;     /* 测试phy性能，将帧直接释放 */
    uint32_t ul_rx_key_search_fail_dropped;
    uint32_t ul_rx_tkip_mic_fail_dropped;   /* 接收描述符status为 tkip mic fail释放MPDU */
    uint32_t ul_rx_replay_fail_dropped;     /* 重放攻击，释放一个MPDU */
    uint32_t ul_rx_11i_check_fail_dropped;
    uint32_t ul_rx_wep_check_fail_dropped;
    uint32_t ul_rx_alg_process_dropped;    /* 算法处理返回失败 */
    uint32_t ul_rx_null_frame_dropped;     /* 接收到空帧释放(之前节能特性已经对其进行处理) */
    uint32_t ul_rx_abnormal_dropped;       /* 其它异常情况释放MPDU */
    uint32_t ul_rx_mgmt_abnormal_dropped;  /* 接收到管理帧出现异常，比如vap或者dev为空等 */
    uint32_t ul_rx_ack_dropped;            /* ack直接过滤掉 */
    uint32_t ul_rx_pspoll_process_dropped; /* 处理ps-poll的时候释放包 */
    uint32_t ul_rx_bar_process_dropped;    /* 处理block ack req释放包 */
    uint32_t ul_rx_abnormal_cnt;           /* 处理MPDU时发现异常的次数，并非丢包数目 */

    uint32_t ul_rx_no_buff_dropped;              /* 组播帧或者wlan_to_wlan流程申请buff失败 */
    uint32_t ul_rx_defrag_process_dropped;       /* 去分片处理失败 */
    uint32_t ul_rx_de_mic_fail_dropped;          /* mic码校验失败 */
    uint32_t ul_rx_portvalid_check_fail_dropped; /* 接收到数据帧，安全检查失败释放MPDU */

    /* 接收到组播帧个数 */
    uint32_t ul_rx_mcast_cnt;

    /* 管理帧统计 */
    uint32_t aul_rx_mgmt_num[WLAN_MGMT_SUBTYPE_BUTT]; /* VAP接收到各种管理帧的数目统计 */

    /* 发送包统计 */
    /* 从lan接收到的数据包统计 */
    uint32_t ul_tx_pkt_num_from_lan; /* 从lan过来的包数目,MSDU */
    uint32_t ul_tx_bytes_from_lan;   /* 从lan过来的字节数 */

    /* 发送流程发生异常导致释放的数据包统计，MSDU级别 */
    uint32_t ul_tx_abnormal_msdu_dropped; /* 异常情况释放MSDU个数，指vap或者user为空等异常 */
    uint32_t ul_tx_security_check_faild;  /* 安全检查失败释放MSDU */
    uint32_t ul_tx_abnormal_mpdu_dropped; /* 异常情况释放MPDU个数，指vap或者user为空等异常 */
    uint32_t ul_tx_uapsd_process_dropped; /* UAPSD特性处理失败，释放MPDU个数 */
    uint32_t ul_tx_psm_process_dropped;   /* PSM特性处理失败，释放MPDU个数 */
    uint32_t ul_tx_alg_process_dropped;   /* 算法处理认为应该丢包，释放MPDU个数 */

    /* 发送完成中发送成功与失败的数据包统计，MPDU级别 */
    uint32_t ul_tx_mpdu_succ_num;      /* 发送MPDU总个数 */
    uint32_t ul_tx_mpdu_fail_num;      /* 发送MPDU失败个数 */
    uint32_t ul_tx_ampdu_succ_num;     /* 发送成功的AMPDU总个数 */
    uint32_t ul_tx_ampdu_bytes;        /* 发送AMPDU总字节数 */
    uint32_t ul_tx_mpdu_in_ampdu;      /* 属于AMPDU的MPDU发送总个数 */
    uint32_t ul_tx_ampdu_fail_num;     /* 发送AMPDU失败个数 */
    uint32_t ul_tx_mpdu_fail_in_ampdu; /* 属于AMPDU的MPDU发送失败个数 */

    /* 组播转单播发送流程统计 */
    uint32_t ul_tx_m2u_ucast_cnt;    /* 组播转单播 单播发送成功个数 */
    uint32_t ul_tx_m2u_ucast_droped; /* 组播转单播 单播发送失败个数 */
    uint32_t ul_tx_m2u_mcast_cnt;    /* 组播转单播 组播发送成功个数 */
    uint32_t ul_tx_m2u_mcast_droped; /* 组播转单播 组播发送成功个数 */

    /* 管理帧统计 */
    uint32_t aul_tx_mgmt_num_sw[WLAN_MGMT_SUBTYPE_BUTT]; /* VAP挂到硬件队列上的各种管理帧的数目统计 */
    uint32_t aul_tx_mgmt_num_hw[WLAN_MGMT_SUBTYPE_BUTT]; /* 发送完成中各种管理帧的数目统计 */
} oam_vap_stat_info_stru;

/* USER级别统计信息结构 */
typedef struct {
    /* 接收包统计，从某个用户处接收到的包统计(TO DS) */
    uint32_t ul_rx_mpdu_num;   /* 从某个用户接收到的MPDU数目 */
    uint32_t ul_rx_mpdu_bytes; /* 某个用户接收到的MPDU字节数 */

    /* 发送包统计，发送给某个用户的包统计(FROM DS)，粒度是TID(具体到从VAP的某个TID发出) */
    uint32_t aul_tx_mpdu_succ_num[WLAN_TIDNO_BUTT];      /* 发送MPDU总个数 */
    uint32_t aul_tx_mpdu_bytes[WLAN_TIDNO_BUTT];         /* 发送MPDU总字节数 */
    uint32_t aul_tx_mpdu_fail_num[WLAN_TIDNO_BUTT];      /* 发送MPDU失败个数 */
    uint32_t aul_tx_ampdu_succ_num[WLAN_TIDNO_BUTT];     /* 发送AMPDU总个数 */
    uint32_t aul_tx_ampdu_bytes[WLAN_TIDNO_BUTT];        /* 发送AMPDU总字节数 */
    uint32_t aul_tx_mpdu_in_ampdu[WLAN_TIDNO_BUTT];      /* 属于AMPDU的MPDU发送总个数 */
    uint32_t aul_tx_ampdu_fail_num[WLAN_TIDNO_BUTT];     /* 发送AMPDU失败个数 */
    uint32_t aul_tx_mpdu_fail_in_ampdu[WLAN_TIDNO_BUTT]; /* 属于AMPDU的MPDU发送失败个数 */
    uint32_t ul_tx_ppdu_retries;                         /* 空口重传的PPDU计数 */
} oam_user_stat_info_stru;

/* 统计结构信息，包括设备级别，vap级别，user级别的各种统计信息 */
typedef struct {
    /* 设备级别的统计信息 */
    oam_device_stat_info_stru ast_dev_stat_info[WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC];

    /* VAP级别的统计信息 */
    oam_vap_stat_info_stru ast_vap_stat_info[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];

    /* USER级别的统计信息 */
    oam_user_stat_info_stru ast_user_stat_info[WLAN_USER_MAX_USER_LIMIT];
} oam_stat_info_stru;

/*lint -e717*/
#define oam_sdt_stat_incr(_cnt)   \
    do {                          \
        g_sdt_stat_info._cnt++; \
    } while (0)

enum oam_filter_type_enum {
    OAM_FLT_PASS = 0,
    OAM_FLT_DROP,
    OAM_FLT_BUTT,
};

typedef struct {
    uint32_t ul_nlk_sd_cnt;       /* 上报消息总数:含失败个数 */
    uint32_t ul_nlk_sd_fail;      /* 发送失败统计 */
    uint32_t ul_wq_len;           /* 队列中消息个数 */
    uint32_t ul_filter_cnt;       /* 过滤消息个数 */
    oal_bool_enum en_filter_switch; /* 过滤开关状态 */
    uint8_t uc_recv[3];
} oam_sdt_stat_info_stru;

extern oam_stat_info_stru g_oam_stat_info;
extern oam_sdt_stat_info_stru g_sdt_stat_info;

/* 保存配置文件相关信息结构 */
typedef struct {
    oam_cfg_type_enum_uint16 en_cfg_type;
    int8_t *pc_section; /* 配置项所属的section字符串 */
    int8_t *pc_key;     /* 配置项在配置文件中的字符串表示 */
    int32_t l_val;      /* 配置项的值 */
} oam_cfg_data_stru;

/* 异常统计结构体:当ul_reported_cnt与ul_record_cnt相同时 */
typedef struct {
    uint32_t en_status;     /* 每类异常类型是否刷新 */
    uint32_t ul_record_cnt; /* 记录上一次更新的次数 */
} oam_excp_record_stru;

/* oam异常统计 */
typedef struct {
    uint32_t en_status; /* VAP级别异常统计是否有刷新 */
    oam_excp_record_stru ast_excp_record_tbl[OAM_EXCP_TYPE_BUTT];
} oam_exception_ctx_stru;

/* VAP日志开关与特性日志级别 */
typedef struct {
    oam_log_level_enum_uint8 aen_vap_log_level[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];  /* VAP日志开关，INFO级别关闭时恢复 */
    /* 特性日志级别 */
    oam_log_level_enum_uint8 aen_feature_log_level[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT][OAM_SOFTWARE_FEATURE_BUTT];
} oam_vap_log_stru;

/* 日志模块全局结构体 */
typedef struct {
    oal_switch_enum_uint8 en_global_log_switch;               /* 日志全局开关 */
    oam_vap_log_stru st_vap_log_info;                         /* VAP日志开关与特性日志级别 */
} oam_log_ctx_stru;

/* ota开关枚举, 注意，新加开关时，请加在后面 */
typedef enum {
    OAM_OTA_SWITCH_BEACON = 0, /* 此处不区分收发方向 */
    OAM_OTA_SWITCH_RX_PPDU_DSCR = 1,
    OAM_OTA_SWITCH_RX_MSDU_DSCR = 2,
    OAM_OTA_SWITCH_TX_PPDU_DSCR = 3,
    OAM_OTA_SWITCH_TX_COMPLETE_PPDU_DSCR = 4,
    OAM_OTA_SWITCH_RX_80211_FRAME = 5,
    OAM_OTA_SWITCH_TX_80211_FRAME = 6,
    OAM_OTA_SWITCH_RX_CB = 7,
    OAM_OTA_SWITCH_TX_CB = 8,
    OAM_OTA_SWITCH_RX_ETH_FRAME = 9,
    OAM_OTA_SWITCH_TX_ETH_FRAME = 10,
    OAM_OTA_SWITCH_ALG_RX = 11,
    OAM_OTA_SWITCH_ALG_TX = 12,
    OAM_OTA_SWITCH_BUTT,
} oam_ota_switch_enum;

/* oam的ota模块，具体某一种ota的控制开关 */
typedef struct {
    oal_switch_enum_uint8 ota_switch[OAM_OTA_SWITCH_BUTT];

    /* 定义ota_switch_mib作为ota总开关，只要有一种OTA不开，则不进ota上报流程，避免消耗mips */
    /* ota_switch_mib中各个bit的定义请参考oam_ota_switch_enum，例子如下: */
    /* |      ...       | TX_COMPLETE_PPDU_DSCR | TX_PPDU_DSCR | RX_MSDU_DSCR | RX_PPDU_DSCR | BEACON | */
    /* | bit15 - bit5   |          bit4         |     bit3     |     bit2     |      bit1    |  bit0  | */
    uint32_t golble_switch;
} oam_ota_ctx_stru;

/* 设置OTA上报开关需要的参数 */
typedef struct {
    uint8_t ota_type;
    uint8_t ota_switch;
} oam_ota_switch_param_stru;

/* 80211帧上报控制结构 */
typedef struct {
    uint8_t bit_content : 1,
               bit_cb : 1,
               bit_dscr : 1,
               bit_ba_info : 1,
               bit_msdu_dscr : 1,
               bit_mu_dscr : 1,
               bit_himit_dscr : 1,
               bit_rsv : 1;
} oam_80211_frame_ctx_stru;

typedef union {
    uint8_t value;
    oam_80211_frame_ctx_stru frame_ctx;
} oam_80211_frame_ctx_union;
/* 单用户跟踪控制开关结构 */
typedef struct {
    /* 80211广播\组播管理帧上报控制结构 */
    oam_80211_frame_ctx_union ast_80211_mcast_mgmt_ctx[OAM_OTA_FRAME_DIRECTION_TYPE_BUTT];
    /* 80211广播\组播数据帧上报控制结构 */
    oam_80211_frame_ctx_union ast_80211_mcast_data_ctx[OAM_OTA_FRAME_DIRECTION_TYPE_BUTT];
    /* 单播帧开关，不同用户开关可以不同 */
    oam_80211_frame_ctx_union ast_80211_ucast_mgmt_ctx[WLAN_USER_MAX_USER_LIMIT][OAM_OTA_FRAME_DIRECTION_TYPE_BUTT];
    /* 单播帧开关，不同用户开关可以不同 */
    oam_80211_frame_ctx_union ast_80211_ucast_data_ctx[WLAN_USER_MAX_USER_LIMIT][OAM_OTA_FRAME_DIRECTION_TYPE_BUTT];
    /* probe request和probe response太多，单独搞个开关 */
    oam_80211_frame_ctx_union aen_80211_probe_switch[OAM_OTA_FRAME_DIRECTION_TYPE_BUTT];
    /* 802.3数据帧上报控制结构 */
    oal_switch_enum_uint8 aen_eth_data_ctx[WLAN_USER_MAX_USER_LIMIT][OAM_OTA_FRAME_DIRECTION_TYPE_BUTT];
    oal_switch_enum_uint8 en_tx_mcast_dhcp_arp_switch; /* 特殊帧上报开关 */
    /* 数据帧流程打印总开关，只要有一个用户的数据开关打开，总开关就开，否则关闭，避免消耗mips */
    oal_switch_enum_uint8 aen_data_global_switch[OAM_OTA_FRAME_DIRECTION_TYPE_BUTT];
    oal_switch_enum_uint8 aen_user_info_change_switch[WLAN_USER_MAX_USER_LIMIT][OAM_USER_INFO_CHANGE_TYPE_BUTT];
} oam_user_track_ctx_stru;

#define OAM_FILE_PATH_LENGTH 256 /* 用于保存LOG文件地址的字符串最大长度,路径最长为255字节，最后需要有一字节存放\0 */

typedef struct {
    uint8_t auc_resv[3];
    oam_output_type_enum_uint8 en_output_type;   /* 输出方式 */

    /* LOG模块管理上下文 */
    oam_log_ctx_stru st_log_ctx;

    /* 某一种OTA消息的控制结构 */
    oam_ota_ctx_stru ast_ota_ctx;

    oam_exception_ctx_stru st_exception_ctx[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];

    /* 单用户跟踪管理上下文 */
    oam_user_track_ctx_stru st_user_track_ctx;
} oam_mng_ctx_stru;

/* 统计与性能维测相关的结构定义 */
#ifdef _PRE_WLAN_DFT_STAT

/* 反应空口环境繁忙程度的参数结构，有PHY的参数也有MAC的参数 */
typedef struct {
    uint16_t us_mac_ch_stats_period_us; /* mac进行信道空闲时间统计的周期，20ms，20000us */
    uint16_t us_mac_pri20_free_time_us; /* 统计周期内主20M信道空闲时间，us */
    uint16_t us_mac_pri40_free_time_us; /* 统计周期内主40M信道空闲时间，us */
    uint16_t us_mac_pri80_free_time_us; /* 统计周期内主80M信道空闲时间,us */
    uint16_t us_mac_ch_rx_time_us;      /* 统计周期内接收占用的时间，us */
    uint16_t us_mac_ch_tx_time_us;      /* 统计周期内发送占用的时间，us */

    uint8_t uc_phy_ch_estimate_time_ms; /* phy测量信道空闲时功率的周期，ms */
    int8_t c_phy_pri20_idle_power_dbm;  /* 统计周期内主20M信道空闲时的功率，一直不空闲就不统计 */
    int8_t c_phy_pri40_idle_power_dbm;  /* 统计周期内主40M信道空闲时的功率，一直不空闲就不统计 */
    int8_t c_phy_pri80_idle_power_dbm;  /* 统计周期内主80M信道空闲时的功率，一直不空闲就不统计 */

    uint32_t aul_beacon_miss_max_num[WLAN_SERVICE_AP_MAX_NUM_PER_DEVICE]; /* mac记录的每个AP BEACON MISS历史最大值 */
    uint32_t non_directed_frame_num;
} oam_stats_dbb_env_param_stru;

/* phy的收发包数目统计值，包括用户指定需要的4个，和phy一直统计的8个按照协议模式的统计值 */
typedef struct {
    uint32_t aul_user_requested_stat_cnt[OAM_PHY_STAT_NODE_ENABLED_MAX_NUM];

    /* phy统计的接收到11b,ht,vht,legecy ofdm帧正确和错误的个数 */
    uint32_t aul_phy_stat_rx_ppdu_cnt[OAM_PHY_STAT_RX_PPDU_CNT];
} oam_stats_phy_stat_stru;

/* mac统计收发包数目 */
typedef struct {
    /* rx统计 */
    uint32_t ul_machw_rx_ampdu_count;
    uint32_t ul_machw_rx_passed_mpdu_in_ampdu_cnt;
    uint32_t ul_machw_rx_failed_mpdu_in_ampdu_cnt;
    uint32_t ul_machw_rx_octects_in_ampdu;
    uint32_t ul_machw_rx_delimit_fail_count;
    uint32_t ul_machw_rx_dup_mpdu_cnt;
    uint32_t ul_machw_rx_passed_mpdu_cnt;
    uint32_t ul_machw_rx_failed_mpdu_cnt;
    uint32_t ul_machw_rx_bcn_cnt;
    uint32_t ul_machw_rx_phy_err_mac_passed_cnt;
    uint32_t ul_machw_rx_phy_longer_err_cnt;
    uint32_t ul_machw_rx_phy_shorter_err_cnt;
    uint32_t ul_machw_rx_filtered_cnt;

    /* tx统计 */
    uint32_t ul_machw_tx_hi_pri_mpdu_cnt;
    uint32_t ul_machw_tx_normal_pri_mpdu_cnt;
    uint32_t ul_machw_tx_ampdu_count;
    uint32_t ul_machw_tx_mpdu_inampdu_count;
    uint32_t ul_machw_tx_octects_in_ampdu;
    uint32_t ul_machw_tx_bcn_count;
    uint32_t ul_machw_tx_normal_pri_retry_cnt;
    uint32_t ul_machw_tx_hi_pri_retry_cnt;
} oam_stats_machw_stat_stru;

/* 管理帧收发数目统计，包括硬件统计的发送beacon帧数目和高优先级队列发送数目 */
typedef struct {
    uint32_t machw_stat_tx_bcn_cnt;
    uint32_t machw_stat_tx_hi_cnt;
    uint32_t aul_sw_rx_mgmt_cnt[WLAN_MGMT_SUBTYPE_BUTT];
    uint32_t aul_sw_tx_mgmt_cnt[WLAN_MGMT_SUBTYPE_BUTT];
    uint32_t aul_tx_complete_mgmt_cnt[WLAN_MGMT_SUBTYPE_BUTT];
} oam_stats_mgmt_stat_stru;

/* 用户队列统计信息，包括节能队列和tid队列 */
typedef struct {
    uint32_t aul_uapsd_stat[OAM_UAPSD_STAT_CNT];
    uint32_t aul_psm_stat[OAM_PSM_STAT_CNT];
    uint32_t aul_tid_stat[WLAN_TID_MAX_NUM][OAM_TID_STAT_CNT];
} oam_stats_usr_queue_stat_stru;

/* vap吞吐统计信息结构，包个数，字节数，平均速率(字节数/上报周期) */
typedef struct {
    uint32_t ul_lan_tx_pkts;  /* 以太网过来的包数目 */
    uint32_t ul_lan_tx_bytes; /* 以太网过来的字节数，不算以太网头 */
    uint32_t ul_drv_tx_pkts;  /* 驱动发包数目，交给硬件的数目 */
    uint32_t ul_drv_tx_bytes; /* 驱动发包字节数，不算80211头尾 */
    uint32_t ul_hw_tx_pkts;   /* 发送完成中断上报发送成功的个数 */
    uint32_t ul_hw_tx_bytes;  /* 发送完成中断上报发送成功字节数 */

    uint32_t ul_drv_rx_pkts;  /* 驱动接收包数目 */
    uint32_t ul_drv_rx_bytes; /* 驱动接收字节数，不包括80211头尾 */
    uint32_t ul_lan_rx_pkts;  /* 驱动交给以太网包数目 */
    uint32_t ul_lan_rx_bytes; /* 驱动交给以太网字节数，不包括以太网头 */
    uint32_t ul_app_rx_pkts;  /* 驱动交给以APP包数目 */
    uint32_t ul_app_rx_bytes; /* 驱动交给以APP字节数，不包括以太网头 */

    /* 平均速率 单位: kbps */
    uint32_t ul_lan_tx_rate; /* lan发包平均速率 */
    uint32_t ul_drv_tx_rate; /* 驱动发包平均速率 */
    uint32_t ul_hw_tx_rate;  /* 发送完成上报成功的发包平均速率 */
    uint32_t ul_drv_rx_rate; /* 驱动收包平均速率 */
    uint32_t ul_lan_rx_rate; /* lan收包平均速率 */
} oam_stats_vap_stat_stru;
#endif

/* 函数声明    OAM模块对外提供的接口 */
extern oam_mng_ctx_stru g_oam_mng_ctx;

extern int32_t oam_main_init(void);
extern void oam_main_exit(void);
extern uint32_t oam_print(const char *pc_string);
extern uint32_t oam_set_output_type(oam_output_type_enum_uint8 en_output_type);
extern uint32_t oam_get_output_type(oam_output_type_enum_uint8 *pen_output_type);

void oam_log_set_global_switch(oal_switch_enum_uint8 en_log_switch);
extern uint32_t oam_log_set_feature_level(uint8_t uc_vap_id,
                                          oam_feature_enum_uint8 en_feature_id,
                                          oam_log_level_enum_uint8 en_log_level);
extern uint32_t oam_get_feature_id(uint8_t *puc_feature_name,
                                   oam_feature_enum_uint8 *puc_feature_id);
extern uint32_t oam_show_feature_list(void);
void oam_log_set_vap_level(uint8_t uc_vap_id, oam_log_level_enum_uint8 en_log_level);
extern uint32_t oam_log_print0(uint32_t ul_para,
                               uint16_t us_line_num,
                               int8_t *pc_string);

extern uint32_t oam_log_print1(uint32_t ul_para,
                               uint16_t us_line_num,
                               int8_t *pc_string,
                               oal_uint l_param1);

extern uint32_t oam_log_print2(uint32_t ul_para,
                               uint16_t us_line_num,
                               int8_t *pc_string,
                               oal_uint l_param1,
                               oal_uint l_param2);

extern uint32_t oam_log_print3(uint32_t ul_para,
                               uint16_t us_line_num,
                               int8_t *pc_string,
                               oal_uint l_param1,
                               oal_uint l_param2,
                               oal_uint l_param3);

extern uint32_t oam_log_print4(uint32_t ul_para,
                               uint16_t us_line_num,
                               int8_t *pc_string,
                               oal_uint l_param1,
                               oal_uint l_param2,
                               oal_uint l_param3,
                               oal_uint l_param4);

extern uint32_t oam_log_console_printk(uint16_t us_file_no,
                                       uint16_t us_line_num,
                                       const int8_t *pc_func_name,
                                       const int8_t *pc_fmt, ...);
extern uint32_t oam_ota_report(uint8_t *puc_param_one_addr,
                               uint16_t us_param_one_len,
                               uint8_t *puc_param_two_addr,
                               uint16_t us_param_two_len,
                               oam_ota_type_enum_uint8 en_ota_type);
void oam_ota_set_switch(uint8_t ota_switch_id, uint8_t ota_switch);
uint32_t oam_ota_get_global_switch(void);
oal_switch_enum_uint8 oam_ota_get_switch(uint8_t ota_switch_id);
extern uint32_t oam_report_eth_frame(uint8_t *puc_user_mac_addr,
                                     uint8_t *puc_eth_frame_hdr_addr,
                                     uint16_t us_eth_frame_len,
                                     oam_ota_frame_direction_type_enum_uint8 en_eth_frame_direction);
extern uint32_t oam_report_dscr(uint8_t *puc_user_macaddr,
                                uint8_t *puc_dscr_addr,
                                uint16_t us_dscr_len,
                                oam_ota_type_enum_uint8 en_ota_type);
extern uint32_t oam_report_beacon(uint8_t *puc_beacon_hdr_addr,
                                  uint8_t uc_beacon_hdr_len,
                                  uint8_t *puc_beacon_body_addr,
                                  uint16_t us_beacon_len,
                                  oam_ota_frame_direction_type_enum_uint8 en_beacon_direction);
extern void oam_report_backtrace(void);
extern uint32_t oam_report_80211_frame(uint8_t *puc_user_macaddr,
                                       uint8_t *puc_mac_hdr_addr,
                                       uint8_t uc_mac_hdr_len,
                                       uint8_t *puc_mac_body_addr,
                                       uint16_t us_mac_frame_len,
                                       oam_ota_frame_direction_type_enum_uint8 en_frame_direction);
extern uint32_t oam_report_80211_mcast_get_switch(oam_ota_frame_direction_type_enum_uint8 en_mcast_direction,
                                                  oam_user_track_frame_type_enum_uint8 en_frame_type,
                                                  oam_80211_frame_ctx_union *oam_frame_report_un);
extern uint32_t oam_report_80211_ucast_get_switch(oam_ota_frame_direction_type_enum_uint8 en_ucast_direction,
                                                  oam_user_track_frame_type_enum_uint8 en_frame_type,
                                                  oam_80211_frame_ctx_union *oam_frame_report_un,
                                                  uint16_t us_user_idx);
extern uint32_t oam_report_80211_probe_get_switch(oam_ota_frame_direction_type_enum_uint8 en_probereq_direction,
                                                  oam_80211_frame_ctx_union *oam_frame_report_un);
extern uint32_t oam_report_80211_probe_set_switch(oam_ota_frame_direction_type_enum_uint8 en_probe_direction,
                                                  oam_80211_frame_ctx_union *oam_switch_param);
extern uint32_t oam_report_eth_frame_get_switch(uint16_t us_user_idx,
                                                oam_ota_frame_direction_type_enum_uint8 en_eth_direction,
                                                oal_switch_enum_uint8 *pen_eth_switch);
extern uint32_t oam_report_netbuf_cb(uint8_t *user_mac_addr,
                                     uint8_t *netbuf_cb,
                                     uint16_t cb_len,
                                     oam_ota_type_enum_uint8 en_ota_type);
extern uint32_t oam_report_dft_params(uint8_t *puc_user_mac_addr,
                                      uint8_t *puc_param,
                                      uint16_t us_param_len,
                                      oam_ota_type_enum_uint8 en_type);
void oam_report_set_all_switch(oal_switch_enum_uint8 en_switch);
extern uint32_t oam_report_80211_ucast_set_switch(oam_ota_frame_direction_type_enum_uint8 en_ucast_direction,
                                                  oam_user_track_frame_type_enum_uint8 en_frame_type,
                                                  oam_80211_frame_ctx_union *oam_switch_param,
                                                  uint16_t us_user_idx);
extern uint32_t oam_report_eth_frame_set_switch(uint16_t us_user_idx,
                                                oal_switch_enum_uint8 en_switch,
                                                oam_ota_frame_direction_type_enum_uint8 en_eth_direction);
extern uint32_t oam_report_80211_mcast_set_switch(oam_ota_frame_direction_type_enum_uint8 en_mcast_direction,
                                                  oam_user_track_frame_type_enum_uint8 en_frame_type,
                                                  oam_80211_frame_ctx_union *oam_frame_report);
extern uint32_t oam_report_dhcp_arp_set_switch(oal_switch_enum_uint8 en_switch);
extern oal_switch_enum_uint8 oam_report_dhcp_arp_get_switch(void);
extern oal_switch_enum_uint8 oam_report_data_get_global_switch(oam_ota_frame_direction_type_enum_uint8 en_direction);


/* SDT模块对外接口 */
extern void oam_sdt_func_fook_register(oam_sdt_func_hook_stru *pfun_st_oam_sdt_hook);
extern void oam_sdt_func_fook_unregister(void);
extern void oam_wal_func_fook_register(oam_wal_func_hook_stru *pfun_st_oam_wal_hook);
extern void oam_wal_func_fook_unregister(void);
extern uint32_t oam_report_data2sdt(oal_netbuf_stru *pst_netbuf,
                                    oam_data_type_enum_uint8 en_type,
                                    oam_primid_type_enum_uint8 en_prim);
extern oal_netbuf_stru *oam_alloc_data2sdt(uint16_t us_data_len);

/* STATISTICS模块对外接口 */
extern uint32_t oam_stats_report_timer_info_to_sdt(uint8_t *puc_timer_addr,
                                                   uint8_t uc_timer_len);
extern uint32_t oam_stats_report_event_queue_info_to_sdt(uint8_t *puc_event_queue_addr,
                                                         uint16_t us_event_queue_info_len);
extern uint32_t oam_stats_report_info_to_sdt(oam_ota_type_enum_uint8 en_ota_type);
extern void oam_stats_clear_stat_info(void);
extern uint32_t oam_stats_report_usr_info(uint16_t us_usr_id);
extern uint32_t oam_stats_clear_user_stat_info(uint16_t us_usr_id);

extern uint32_t oam_stats_clear_vap_stat_info(uint8_t uc_vap_id);

extern uint32_t oam_stats_report_mempool_info_to_sdt(uint8_t uc_pool_id,
                                                     uint16_t us_pool_total_cnt,
                                                     uint16_t us_pool_used_cnt,
                                                     uint8_t uc_subpool_id,
                                                     uint16_t us_subpool_total_cnt,
                                                     uint16_t us_subpool_free_cnt);
extern uint32_t oam_stats_report_memblock_info_to_sdt(uint8_t *puc_origin_data,
                                                      uint8_t uc_user_cnt,
                                                      uint8_t uc_pool_id,
                                                      uint8_t uc_subpool_id,
                                                      uint16_t us_len,
                                                      uint32_t ul_file_id,
                                                      uint32_t ul_alloc_line_num);
extern void oam_stats_report_irq_info_to_sdt(uint8_t *puc_irq_info_addr,
                                             uint16_t us_irq_info_len);

/* 与配置文件相关的函数声明 */
extern int32_t oam_cfg_get_item_by_id(oam_cfg_type_enum_uint16 en_cfg_type);
extern void oam_netlink_ops_register(oam_nl_cmd_enum_uint8 en_type,
                                     uint32_t(*p_func)(uint8_t *puc_data, uint32_t ul_len));
extern void oam_netlink_ops_unregister(oam_nl_cmd_enum_uint8 en_type);
extern int32_t oam_netlink_kernel_send(uint8_t *puc_data, uint32_t ul_data_len,
                                       oam_nl_cmd_enum_uint8 en_type);
extern int32_t oam_netlink_kernel_send_ex(uint8_t *puc_data_1st, uint8_t *puc_data_2nd,
                                          uint32_t ul_len_1st, uint32_t ul_len_2nd,
                                          oam_nl_cmd_enum_uint8 en_type);

OAL_STATIC OAL_INLINE oal_switch_enum_uint8 oam_get_log_switch(uint32_t ul_para)
{
    uint8_t uc_vap_id = (ul_para >> 24) & 0xf; /* delocalize 24 */
    oam_feature_enum_uint8 en_feature_id = (ul_para >> 16) & 0xff; /* delocalize 16 */
    oam_log_level_enum_uint8 en_log_level = (ul_para >> 28) & 0xf; /* delocalize 28 */

    /* 从执行性能角度考虑，提前判断特性开关，大部分info日志可以在第一层命令就直接退出 */
    if (g_oam_mng_ctx.st_log_ctx.en_global_log_switch == OAL_SWITCH_OFF) {
        return OAL_SWITCH_OFF;
    } else if (en_log_level > g_oam_mng_ctx.st_log_ctx.st_vap_log_info.aen_vap_log_level[uc_vap_id]) {
        return OAL_SWITCH_OFF;
    } else if (en_log_level >
               g_oam_mng_ctx.st_log_ctx.st_vap_log_info.aen_feature_log_level[uc_vap_id][en_feature_id]) {
        return OAL_SWITCH_OFF;
    }

    return OAL_SWITCH_ON;
}

#endif /* end of oam_ext_if.h */
