

#ifndef __HD_COMMAND_H__
#define __HD_COMMAND_H__

#include "wlan_customize.h"

/* Power save modes specified by the user */
typedef enum {
    NO_POWERSAVE = 0,
    MIN_FAST_PS = 1,  /* idle 时间采用ini中的上限 */
    MAX_FAST_PS = 2,  /* idle 时间采用ini中的下限 */
    AUTO_FAST_PS = 3, /* 根据算法自动调整idle时间用ini中的上限还是下限值 */
    MAX_PSPOLL_PS = 4,
    NUM_PS_MODE = 5
} ps_user_mode_enum;

/* MAC TX RX common report命令对应枚举 */
typedef enum {
    MAC_TX_COMMON_REPORT_INDEX = 0,
    MAC_TX_COMMON_REPORT_STATUS,
    MAC_TX_COMMON_REPORT_BW_MODE,
    MAC_TX_COMMON_REPORT_PROTOCOL_MODE,
    MAC_TX_COMMON_REPORT_FRAME_TYPE,
    MAC_TX_COMMON_REPORT_SUB_TYPE = 5,
    MAC_TX_COMMON_REPORT_APMDU,
    MAC_TX_COMMON_REPORT_PSDU,
    MAC_TX_COMMON_REPORT_HW_RETRY,
    MAC_TX_COMMON_REPORT_CTRL_REG_CLEAR,

    MAC_TX_COMMON_REPORT_DEBUG_BUTT
} mac_tx_common_report_debug_type_enum;
typedef uint8_t mac_tx_common_report_debug_type_enum_uint8;

typedef enum {
    MAC_RX_COMMON_REPORT_INDEX = 0,
    MAC_RX_COMMON_REPORT_STATUS,
    MAC_RX_COMMON_REPORT_BW_MODE,
    MAC_RX_COMMON_REPORT_PROTOCOL_MODE,
    MAC_RX_COMMON_REPORT_FRAME_TYPE,
    MAC_RX_COMMON_REPORT_SUB_TYPE = 5,
    MAC_RX_COMMON_REPORT_APMDU,
    MAC_RX_COMMON_REPORT_PSDU,
    MAC_RX_COMMON_REPORT_VAP_CHK,
    MAC_RX_COMMON_REPORT_BSS_CHK,
    MAC_RX_COMMON_REPORT_DIRECT_CHK = 10,
    MAC_RX_COMMON_REPORT_CTRL_REG_CLEAR,

    MAC_RX_COMMON_REPORT_DEBUG_BUTT
} mac_rx_common_report_debug_type_enum;
typedef uint8_t mac_rx_common_report_debug_type_enum_uint8;

typedef enum {
    PG_EFFICIENCY_STATISTICS_ENABLE = 0,
    PG_EFFICIENCY_INFO = 1,

    MAC_VAP_COMMON_SET_TXQ = 2,
    MAC_VAP_COMMON_TX_INFO = 3,
#ifdef _PRE_WLAN_FEATURE_MBO
    MBO_SWITCH = 4,                  /* 测试用例1:MBO特性开关,out-of-the-box test */
    MBO_CELL_CAP = 5,                /* 测试用例2:MBO Capability Indication test */
    MBO_ASSOC_DISALLOWED_SWITCH = 6, /* 测试用例6 AP通过带MBO IE指示不允许被关联 */
#endif

#ifdef _PRE_WLAN_FEATURE_HIEX
    HIEX_DEV_CAP = 7, /* HIEX device能力配置命令 */
    HIEX_DEBUG = 8,   /* HIEX特性debug开关 */
#endif

    RIFS_ENABLE = 9,            /* rifs能力使能 */
    GREENFIELD_ENABLE = 10,     /* GreenField能力使能 */
    PROTECTION_LOG_SWITCH = 11, /* 保护模式维测打印 */

#ifdef _PRE_WLAN_FEATURE_11AX
    DMAC_RX_FRAME_STATISTICS = 12, /* 接受帧信息统计 */
    DMAC_TX_FRAME_STATISTICS = 13, /* 发送帧信息统计 */
#endif
    AUTH_RSP_TIMEOUT = 14, /* 认证时配置auth 阶段超时时间 */
    FORBIT_OPEN_AUTH = 15, /* 禁止open入网 */
#ifdef _PRE_WLAN_RX_LISTEN_POWER_SAVING
    HAL_PS_DEBUG_SWITCH = 16,
#endif
    SU_PPDU_1XLTF_08US_GI_SWITCH = 17,
    HT_SELF_CURE_DEBUG = 18,
    USERCTL_BINDCPU = 19,  /* 用户指定硬中断和线程绑核 */
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
    MCAST_AMPDU_RETRY = 20, /* 组播聚合模式下配置组播帧重传次数 */
#endif

    MAC_COMMON_DEBUG_BUTT
} mac_common_debug_type_enum;
typedef uint8_t mac_common_debug_type_enum_uint8;

typedef enum {
    MAC_VAP_11AX_DEBUG_PRINT_LOG,
    MAC_VAP_11AX_DEBUG_PRINT_RX_LOG,
    MAC_VAP_11AX_DEBUG_HE_TB_PPDU_TID_NUM,
    MAC_VAP_11AX_DEBUG_HE_TB_PPDU_HTC_ORGER,
    MAC_VAP_11AX_DEBUG_HE_TB_PPDU_HTC_VALUE,
    MAC_VAP_11AX_DEBUG_HE_TB_PPDU_FIX_POWER,
    MAC_VAP_11AX_DEBUG_HE_TB_PPDU_POWER_VALUE,
    MAC_VAP_11AX_DEBUG_HE_TB_PPDU_DISABLE_BA_CHECK,
    MAC_VAP_11AX_DEBUG_DISABLE_MU_EDCA,
    MAC_VAP_11AX_DEBUG_MANUAL_CFO,
    MAC_VAP_11AX_DEBUG_BSRP_CFG,
    MAC_VAP_11AX_DEBUG_BSRP_TID,
    MAC_VAP_11AX_DEBUG_BSRP_QUEUE_SIZE,
    MAC_VAP_11AX_DEBUG_MAC_PADDING,
    MAC_VAP_11AX_DEBUG_POW_TEST,
    MAC_VAP_11AX_DEBUG_TB_PPDU_LEN,
    MAC_VAP_11AX_DEBUG_TB_PPDU_AC,
    MAC_VAP_11AX_DEBUG_PRINT_RX_TRIG_FRAME,
    MAC_VAP_11AX_DEBUG_OM_AUTH_FLAG,
    MAC_VAP_11AX_DEBUG_TOM_BW_FLAG,
    MAC_VAP_11AX_DEBUG_TOM_NSS_FLAG,
    MAC_VAP_11AX_DEBUG_TOM_UL_MU_DISABLE_FLAG,
    MAC_VAP_11AX_DEBUG_WIFI6_SELF_CURE_FLAG,
    MAC_VAP_11AX_DEBUG_UORA_OCW_UPDATE,
    MAC_VAP_11AX_DEBUG_UORA_OBO_BYPASS,
    MAC_VAP_11AX_DEBUG_BSS_COLOR,
    MAC_VAP_11AX_DEBUG_TX_Q_STATUS,

    MAC_VAP_11AX_DEBUG_BUTT
} mac_vap_11ax_debug_type_enum;
typedef uint8_t mac_vap_11ax_debug_type_enum_uint8;

typedef enum {
    MAC_PS_TYPE_DBAC = BIT0,
    MAC_PS_TYPE_BTCOEX = BIT1,
    MAC_PS_TYPE_CMD = BIT2,

    MAC_PS_TYPE_BUTT
} mac_ps_type_enum;

typedef enum {
    MAC_PROBE_RESP_MODE_ACTIVE = 0, /* 驱动收到probe req后，主动组probe resp包并回复到空口 */
    MAC_PROBE_RESP_MODE_PASSIVE = 1, /* 驱动收到probe req后，顺从上层的决定选择性回复probe resp帧 */
    MAC_PROBE_RESP_MODE_BUTT
} mac_probe_resp_mode_enum;
typedef uint8_t mac_probe_resp_mode_enum_uint8; /* hd_event */

typedef struct {
    oal_bool_enum_uint8 en_probe_resp_enable;    /* 该bool开关控制probe_resp_status是否可以被改动 */
    mac_probe_resp_mode_enum_uint8 en_probe_resp_status;    /* probe resp回复的模式 */
    uint8_t auc_rsv[NUM_2_BYTES];
} mac_probe_resp_ctrl_stru;

/* 打印接收报文的rssi信息的调试开关相关的结构体 */
typedef struct {
    uint16_t us_data_len;        /*  单音采样点个数  */
    uint8_t uc_tone_tran_switch; /*  单音发送开关  */
    uint8_t uc_chain_idx;        /*  单音发送通道号  */
} mac_tone_transmit_stru;

/* 打印接收报文的rssi信息的调试开关相关的结构体 */
typedef struct {
    oal_bool_enum_uint8 en_debug_switch;          /* 打印总开关 */
    oal_bool_enum_uint8 en_rssi_debug_switch;     /* 打印接收报文的rssi信息的调试开关 */
    oal_bool_enum_uint8 en_snr_debug_switch;      /* 打印接收报文的snr信息的调试开关 */
    oal_bool_enum_uint8 en_trlr_debug_switch;     /* 打印接收报文的trailer信息的调试开关 */
    oal_bool_enum_uint8 en_evm_debug_switch;      /* 打印接收报文的evm信息的调试开关 */
    oal_bool_enum_uint8 en_sp_reuse_debug_switch; /* 打印接收报文的spatial reuse信息的调试开关 */
    uint8_t auc_resv[BYTE_OFFSET_2];
    uint32_t curr_rx_comp_isr_count;     /* 一轮间隔内，接收完成中断的产生个数 */
    uint32_t rx_comp_isr_interval;       /* 间隔多少个接收完成中断打印一次rssi信息 */
    mac_tone_transmit_stru st_tone_tran;      /* 单音发送参数 */
    uint8_t auc_trlr_sel_info[BYTE_OFFSET_5];           /* trailer选择上报计数, 一个字节高4bit指示trlr or vect,低4个bit指示选择 */
    uint8_t uc_trlr_sel_num;                /* 记录单次命令输入选项的最大值 */
    uint8_t uc_iq_cali_switch;              /* iq校准调试命令  */
    oal_bool_enum_uint8 en_pdet_debug_switch; /* 打印芯片上报pdet值的调试开关 */
    oal_bool_enum_uint8 en_tsensor_debug_switch;
    uint8_t uc_force_work_switch;
    uint8_t uc_dfr_reset_switch;         /* dfr_reset调试命令: 高4bit为reset_mac_submod, 低4bit为reset_hw_type */
    uint8_t uc_fsm_info_switch;          /* hal fsm debug info */
    uint8_t uc_report_radar_switch;      /* radar上报开关 */
    uint8_t uc_extlna_chg_bypass_switch; /* 功耗测试关闭外置LNA开关: 0/1/2:no_bypass/dyn_bypass/force_bypass */
    uint8_t uc_edca_param_switch;        /* EDCA参数设置开关 */
    uint8_t uc_edca_aifsn;               /* edca参数AIFSN */
    uint8_t uc_edca_cwmin;               /* edca参数CWmin */
    uint8_t uc_edca_cwmax;               /* edca参数CWmax */
    uint16_t us_edca_txoplimit;          /* edca参数TXOP limit */
} mac_phy_debug_switch_stru; /* hd_event */

typedef struct {
    uint8_t uc_key_index;
    oal_bool_enum_uint8 en_unicast;
    oal_bool_enum_uint8 en_multicast;
    uint8_t auc_resv1[1];
} mac_setdefaultkey_param_stru;

typedef struct {
    uint16_t us_aid;
    uint16_t us_rudir;
} dmac_atcmdsrv_set_ru_info; /* hd_event */

typedef struct wal_atcmdsrv_wifi_rumode_info {
    uint16_t us_aid;
    uint16_t us_rudir;
} st_wal_atcmdsrv_wifi_rumode_info; /* hd_event */

/* SDT读取和设置寄存器的结构体；WTBD:移到common */
typedef struct {
    uint32_t addr;
    uint16_t us_len;
    uint8_t uc_mode;
    uint8_t auc_resv[1];
    uint32_t reg_val;
} dmac_sdt_reg_frame_stru; /* hd_event */

typedef enum {
    MAC_DBDC_CHANGE_HAL_DEV = 0, /* vap change hal device hal vap */
    MAC_DBDC_SWITCH = 1,         /* DBDC软件开关 */
    MAC_FAST_SCAN_SWITCH = 2,    /* 并发扫描开关 */
    MAC_DBDC_STATUS = 3,         /* DBDC状态查询 */

    MAC_DBDC_CMD_BUTT
} mac_dbdc_cmd_enum;
typedef uint8_t mac_dbdc_cmd_enum_uint8; /* hd_event */

typedef struct {
    uint32_t cmd_bit_map;
    uint8_t uc_dst_hal_dev_id; /* 需要迁移到的hal device id */
    uint8_t uc_dbdc_enable;
    oal_bool_enum_uint8 en_fast_scan_enable; /* 是否可以并发,XX原因即使硬件支持也不能快速扫描 */
    uint8_t uc_dbdc_status;
} mac_dbdc_debug_switch_stru; /* hd_event */

enum {
    FREQ_SET_MODE = 0,
    /* sync ini data */
    FREQ_SYNC_DATA = 1,
    /* for device debug */
    FREQ_SET_FREQ = 2,
    FREQ_SET_PLAT_FREQ = 3,
    FREQ_GET_FREQ = 4,
    FREQ_SET_FREQ_TC_EN = 5,
    FREQ_SET_FREQ_TC_EXIT = 6,
    FREQ_SET_BUTT
};
typedef uint8_t freq_sync_enum_uint8;

typedef struct {
    uint32_t speed_level;    /* 吞吐量门限 */
    uint32_t cpu_freq_level; /* CPU频率level */
} device_level_stru;

typedef struct {
    uint8_t uc_set_type;
    uint8_t uc_set_freq;
    uint8_t uc_device_freq_enable;
    uint8_t uc_resv;
    device_level_stru st_device_data[DEV_WORK_FREQ_LVL_NUM];
} config_device_freq_h2d_stru; /* hd_event */


typedef struct {               /* hi1102-cb */
    uint8_t uc_shortgi_type; /* shortgi 20/40/80     */
    uint8_t uc_enable;       /* 1:enable; 0:disable  */
    uint8_t auc_resv[NUM_2_BYTES];
} shortgi_cfg_stru; /* hd_event */

/* 配置发送描述符内部元素结构体 */
typedef enum {
    RF_PAYLOAD_ALL_ZERO = 0,
    RF_PAYLOAD_ALL_ONE,
    RF_PAYLOAD_RAND,
    RF_PAYLOAD_BUTT
} mac_rf_payload_enum;
typedef uint8_t mac_rf_payload_enum_uint8;

typedef struct {
    uint8_t uc_param; /* 查询或配置信息 */
    wlan_phy_protocol_enum_uint8 en_protocol_mode;
    mac_rf_payload_enum_uint8 en_payload_flag;
    wlan_tx_ack_policy_enum_uint8 en_ack_policy;
    uint32_t payload_len;
    uint32_t write_index;
    uint32_t base_addr_lsb;
    uint32_t base_addr_msb;
} mac_cfg_tx_comp_stru; /* hd_event */

/* 芯片验证，控制帧/管理帧类型 */
typedef enum {
    MAC_TEST_MGMT_BCST = 0,               /* 非beacon广播管理帧 */
    MAC_TEST_MGMT_MCST = 1,               /* 非beacon组播管理帧 */
    MAC_TEST_ATIM_UCST = 2,               /* 单播ATIM帧 */
    MAC_TEST_UCST = 3,                    /* 单播管理帧 */
    MAC_TEST_CTL_BCST = 4,                /* 广播控制帧 */
    MAC_TEST_CTL_MCST = 5,                /* 组播控制帧 */
    MAC_TEST_CTL_UCST = 6,                /* 单播控制帧 */
    MAC_TEST_ACK_UCST = 7,                /* ACK控制帧 */
    MAC_TEST_CTS_UCST = 8,                /* CTS控制帧 */
    MAC_TEST_RTS_UCST = 9,                /* RTS控制帧 */
    MAC_TEST_BA_UCST = 10,                /* BA控制帧 */
    MAC_TEST_CF_END_UCST = 11,            /* CF-End控制帧 */
    MAC_TEST_TA_RA_EUQAL = 12,            /* RA,TA相同帧 */
    MAC_TEST_FTM = 13,                    /* FTM测量帧 */
    MAC_TEST_MGMT_ACTION = 14,            /* 发送任意Action帧 */
    MAC_TEST_MGMT_BEACON_INCLUDE_IE = 15, /* Beacon 帧中尾部插入任意IE信息元素 */
    MAC_TEST_MAX_TYPE_NUM
} mac_test_frame_type;
typedef uint8_t mac_test_frame_type_enum_uint8;

#define MAC_TEST_INCLUDE_FRAME_BODY_LEN 64
typedef struct {
    uint8_t auc_mac_ra[WLAN_MAC_ADDR_LEN];
    uint8_t uc_pkt_num;
    mac_test_frame_type_enum_uint8 en_frame_type;
    uint8_t uc_frame_body[MAC_TEST_INCLUDE_FRAME_BODY_LEN]; /* 64 帧体长度 */
    uint8_t uc_vap_idx;
    uint8_t uc_frame_cnt;
    uint8_t uc_frame_body_length;
    uint8_t uc_resv[1];
} mac_cfg_send_frame_param_stru;  /* hd_event */

typedef struct {
    uint8_t uc_aggr_num_switch; /* 控制聚合个数开关 */
    uint8_t uc_aggr_num;        /* 聚合个数 */
    uint8_t auc_resv[NUM_2_BYTES];
} mac_cfg_aggr_num_stru; /* hd_event */


typedef enum {
    MAC_DYN_CALI_CFG_SET_EN_REALTIME_CALI_ADJUST,
    MAC_DYN_CALI_CFG_SET_2G_DSCR_INT,
    MAC_DYN_CALI_CFG_SET_5G_DSCR_INT,
    MAC_DYN_CALI_CFG_SET_CHAIN_INT,
    MAC_DYN_CALI_CFG_SET_PDET_MIN_TH,
    MAC_DYN_CALI_CFG_SET_PDET_MAX_TH,
    MAC_DYN_CALI_CFG_BUFF,
} mac_dyn_cali_cfg_enum;
typedef uint8_t mac_dyn_cali_cfg_enum_uint8;

/* 动态校准参数枚举，参数值 */
typedef struct {
    mac_dyn_cali_cfg_enum_uint8 en_dyn_cali_cfg; /* 配置命令枚举 */
    uint8_t uc_resv;                           /* 字节对齐 */
    uint16_t us_value;                         /* 配置参数值 */
} mac_ioctl_dyn_cali_param_stru; /* hd_event */

typedef struct {
    uint8_t uc_cmd_type;
    uint8_t uc_len;
    uint16_t us_cfg_id;
    uint32_t value;
} mac_cfg_set_tlv_stru;

typedef struct {
    uint16_t us_cfg_id;
    uint8_t uc_cmd_cnt;
    uint8_t uc_len;
    uint16_t us_set_id[MAC_STR_CMD_MAX_SIZE];
    uint32_t value[MAC_STR_CMD_MAX_SIZE];
} mac_cfg_set_str_stru;

typedef enum {
    MAC_MEMINFO_USER = 0,
    MAC_MEMINFO_VAP,
    MAC_MEMINFO_POOL_INFO,
    MAC_MEMINFO_POOL_DBG,

    MAC_MEMINFO_DSCR,
    MAC_MEMINFO_NETBUFF,
    MAC_MEMINFO_SDIO_TRX,
    MAC_MEMINFO_SAMPLE_ALLOC,
    MAC_MEMINFO_SAMPLE_FREE,
    MAC_MEMINFO_ALL,

    MAC_MEMINFO_BUTT
} mac_meminfo_cmd_enum;
typedef uint8_t mac_meminfo_cmd_enum_uint8;

typedef struct {
    mac_meminfo_cmd_enum_uint8 uc_meminfo_type;
    uint8_t uc_object_index;
} mac_cfg_meminfo_stru;

#endif /* end of hd_command.h */

