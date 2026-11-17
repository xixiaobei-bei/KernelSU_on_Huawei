
#ifndef __MAC_DEVICE_COMMON_H__
#define __MAC_DEVICE_COMMON_H__

#define MAC_DATARATES_PHY_80211G_NUM 12

#define MAC_RX_BA_LUT_BMAP_LEN ((HAL_MAX_RX_BA_LUT_SIZE + 7) >> 3)
#define MAC_TX_BA_LUT_BMAP_LEN ((HAL_MAX_TX_BA_LUT_SIZE + 7) >> 3)

#define MAC_SCAN_CHANNEL_INTERVAL_DEFAULT         6   /* 间隔6个信道，切回工作信道工作一段时间 */
#define MAC_WORK_TIME_ON_HOME_CHANNEL_DEFAULT     110 /* 背景扫描时，返回工作信道工作的时间 */

#define MAC_MAX_IP_FILTER_BTABLE_SIZE 512 /* rx ip数据包过滤功能的黑名单大小 */

/* DMAC SCANNER 扫描模式 */
#define MAC_SCAN_FUNC_MEAS       0x1
#define MAC_SCAN_FUNC_STATS      0x2
#define MAC_SCAN_FUNC_RADAR      0x4 // 未用
#define MAC_SCAN_FUNC_BSS        0x8
#define MAC_SCAN_FUNC_P2P_LISTEN 0x10
#define MAC_SCAN_FUNC_ALL        \
    (MAC_SCAN_FUNC_MEAS | MAC_SCAN_FUNC_STATS | MAC_SCAN_FUNC_RADAR | MAC_SCAN_FUNC_BSS)

/* 扫描完成事件返回状态码 */
typedef enum {
    MAC_SCAN_SUCCESS = 0,    /* 扫描成功 */
    MAC_SCAN_PNO = 1,        /* pno扫描结束 */
    MAC_SCAN_TIMEOUT = 2,    /* 扫描超时 */
    MAC_SCAN_REFUSED = 3,    /* 扫描被拒绝 */
    MAC_SCAN_ABORT = 4,      /* 终止扫描 */
    MAC_SCAN_ABORT_SYNC = 5, /* 扫描被终止同步状态，用于上层去关联命令时强制abort不往内核上报等dmac响应abort往上报 */
    MAC_SCAN_STATUS_BUTT,    /* 无效状态码，初始化时使用此状态码 */
} mac_scan_status_enum;
typedef uint8_t mac_scan_status_enum_uint8;

typedef enum {
    MAC_CH_TYPE_NONE = 0,
    MAC_CH_TYPE_PRIMARY = 1,
    MAC_CH_TYPE_SECONDARY = 2,
} mac_ch_type_enum;
typedef uint8_t mac_ch_type_enum_uint8;

/* SDT操作模式枚举 */
typedef enum {
    MAC_SDT_MODE_WRITE = 0,
    MAC_SDT_MODE_READ,
    MAC_SDT_MODE_WRITE16,
    MAC_SDT_MODE_READ16,

    MAC_SDT_MODE_BUTT
} mac_sdt_rw_mode_enum;
typedef uint8_t mac_sdt_rw_mode_enum_uint8;

typedef enum {
    MAC_CHAN_NOT_SUPPORT = 0,      /* 管制域不支持该信道 */
    MAC_CHAN_AVAILABLE_ALWAYS,     /* 信道一直可以使用 */
    MAC_CHAN_AVAILABLE_TO_OPERATE, /* 经过检测(CAC, etc...)后，该信道可以使用 */
    MAC_CHAN_DFS_REQUIRED,         /* 该信道需要进行雷达检测 */
    MAC_CHAN_BLOCK_DUE_TO_RADAR,   /* 由于检测到雷达导致该信道变的不可用 */

    MAC_CHAN_STATUS_BUTT
} mac_chan_status_enum;
typedef uint8_t mac_chan_status_enum_uint8;

/* device reset同步子类型枚举 */
typedef enum {
    MAC_RESET_SWITCH_SET_TYPE,
    MAC_RESET_SWITCH_GET_TYPE,
    MAC_RESET_STATUS_GET_TYPE,
    MAC_RESET_STATUS_SET_TYPE,
    MAC_RESET_SWITCH_SYS_TYPE = MAC_RESET_SWITCH_SET_TYPE,
    MAC_RESET_STATUS_SYS_TYPE = MAC_RESET_STATUS_SET_TYPE,

    MAC_RESET_SYS_TYPE_BUTT
} mac_reset_sys_type_enum;
typedef uint8_t mac_reset_sys_type_enum_uint8;

/* rx ip数据包过滤的配置命令 */
typedef enum {
    MAC_IP_FILTER_ENABLE = 0,        /* 开/关ip数据包过滤功能 */
    MAC_IP_FILTER_UPDATE_BTABLE = 1, /* 更新黑名单 */
    MAC_IP_FILTER_CLEAR = 2,         /* 清除黑名单 */

    MAC_IP_FILTER_BUTT
} mac_ip_filter_cmd_enum;
typedef uint8_t mac_ip_filter_cmd_enum_uint8;

/* 黑名单条目格式 */
typedef struct {
    uint16_t us_port; /* 目的端口号，以主机字节序格式存储 */
    uint8_t uc_protocol;
    uint8_t uc_resv;
    // uint32_t                  filter_cnt;     /* 目前未接受"统计过滤包数量"的需求，此成员暂不使用 */
} mac_ip_filter_item_stru;

/* rx方向指定数据包过滤配置命令 */
typedef enum {
    MAC_ICMP_FILTER = 1, /* icmp数据包过滤功能 */

    MAC_FILTER_ID_BUTT
} mac_assigned_filter_id_enum;
typedef uint8_t mac_assigned_filter_id_enum_uint8;

/* 指定filter配置命令格式 */
typedef struct {
    mac_assigned_filter_id_enum_uint8 en_filter_id;
    oal_bool_enum_uint8 en_enable; /* 下发功能使能标志 */
    uint8_t uc_resv[NUM_2_BYTES];
} mac_assigned_filter_cmd_stru;

typedef enum {
    MAC_RX_ICMP_FILTER_STOPED = 0,   // 功能关闭，未使能、或者其他状况不允许过滤动作。
    MAC_RX_ICMP_FILTER_WORKING = 1,  // 功能打开，按照规则正常过滤
    MAC_RX_ICMP_FILTER_BUTT
} mac_icmp_filter_state_enum;

typedef uint8_t mac_icmp_filter_state_enum_uint8;

typedef struct {
    mac_assigned_filter_id_enum_uint8 en_filter_id;
    mac_icmp_filter_state_enum_uint8 en_state;  // 功能状态：过滤、非过滤等
    uint8_t uc_resv[NUM_2_BYTES];
} mac_rx_icmp_filter_struc;

#ifdef _PRE_WLAN_FEATURE_APF
#define APF_PROGRAM_MAX_LEN 512
#define APF_VERSION         2
#endif

#endif /* end of mac_device_common.h */
