

#ifndef __OAL_NET_H__
#define __OAL_NET_H__

/* 其他头文件包含 */
#include "platform_spec.h"
#include "oal_types.h"
#include "oal_mm.h"
#include "oal_util.h"
#include "oal_schedule.h"
#include "oal_list.h"
#include "arch/oal_net.h"
/* 宏定义 */
#define OAL_IF_NAME_SIZE               16
#define OAL_NETBUF_DEFAULT_DATA_OFFSET 48 /* 5115上实际测得data比head大48，用于netbuf data指针复位 */

#ifdef _PRE_WINDOWS_SUPPORT
#define OAL_ASSOC_REQ_IE_OFFSET     24 /* 上报windows内核关联请求帧偏移量 */
#define OAL_ASSOC_RSP_IE_OFFSET     24 /* 上报windows内核关联响应帧偏移量 */
#else
#define OAL_ASSOC_REQ_IE_OFFSET     28 /* 上报内核关联请求帧偏移量 */
#define OAL_ASSOC_RSP_IE_OFFSET     30 /* 上报内核关联响应帧偏移量 */
#endif

#define OAL_ASSOC_RSP_FIXED_OFFSET  6 /* 关联响应帧帧长FIXED PARAMETERS偏移量 */
#define OAL_AUTH_IE_OFFSET          30
#define OAL_FT_ACTION_IE_OFFSET     40
#define OAL_SEQ_CTL_LEN             2 /* 序列控制字段长度 */
#define OAL_MAC_ADDR_LEN            6
#define OAL_PMKID_LEN               16
#define OAL_WPA_KEY_LEN             32
#define OAL_WPA_SEQ_LEN             16
#define OAL_WLAN_SA_QUERY_TR_ID_LEN 2
#define OAL_MAX_FT_ALL_LEN          518 /* MD:5 FT:257 RSN:256 */

#define OAL_NET_DEV_COUNTRY_CODE_LEN 4

#define OAL_STA_TX_MCS_LEN      16
#define OAL_STA_RX_MCS_LEN      16
#define OAL_STA_CHIP_VENDOR_LEN 3

#define OAL_VAP_CCA_LEN 2

#define OAL_ETH_HAEDWARE_ADDR_LEN 6

#define OAL_DHCP_CLIENT_ADDR_LEN      16
#define OAL_DHCP_SERVER_NAME_LEN      64
#define OAL_DHCP_BOOT_FILE_NAME_LEN   128
#define OAL_DHCP_VERIABLE_LEN_OPTIONS 4

#define OAL_SSID_ARRY_LEN 3
#define MAX_BLACKLIST_NUM 128
/*
 * Byte order/swapping support.
 */
#define OAL_LITTLE_ENDIAN 1234
#define OAL_BIG_ENDIAN    4321
#define OAL_BYTE_ORDER    OAL_BIG_ENDIAN

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && defined(_PRE_WLAN_FEATURE_DFR)
/* hi1102  由于内核注册端口28 NETLINK_HISI_WIFI_MSG已被占用，所以此处使用未使用的27 NETLINK_HISI_WIFI_PMF */
#define NETLINK_DEV_ERROR 27
#endif

/* 2.10 IP宏定义 */
#define WLAN_DSCP_PRI_SHIFT      2
#define WLAN_IP_PRI_SHIFT        5
#define WLAN_IPV6_PRIORITY_MASK  0x0FF00000
#define WLAN_IPV6_PRIORITY_SHIFT 20

/* 2.12 LLC SNAP宏定义 */
#define LLC_UI                    0x3
#define SNAP_LLC_FRAME_LEN        8
#define SNAP_LLC_LSAP             0xaa
#define SNAP_BRIDGE_SPANNING_TREE 0x42
#define SNAP_IBM_LAN_MANAGEMENT   0xf5
#define SNAP_RFC1042_ORGCODE_0    0x00
#define SNAP_RFC1042_ORGCODE_1    0x00
#define SNAP_RFC1042_ORGCODE_2    0x00
#define SNAP_BTEP_ORGCODE_0       0x00
#define SNAP_BTEP_ORGCODE_1       0x00
#define SNAP_BTEP_ORGCODE_2       0xf8

/* 2.13 ETHER宏定义 */
#define ETHER_ADDR_LEN 6 /* length of an Ethernet address */
#define ETHER_TYPE_LEN 2 /* length of the Ethernet type field */
#define ETHER_CRC_LEN  4 /* length of the Ethernet CRC */
#define ETHER_HDR_LEN  14
#define ETHER_MAX_LEN  1518
#define ETHER_MTU      (ETHER_MAX_LEN - ETHER_HDR_LEN - ETHER_CRC_LEN)

OAL_STATIC OAL_INLINE uint8_t a2x(const char c)
{
    if (c >= '0' && c <= '9') {
        return (uint8_t)oal_atoi(&c);
    }
    if (c >= 'a' && c <= 'f') {
        return (uint8_t)0xa + (uint8_t)(c - 'a');
    }
    if (c >= 'A' && c <= 'F') {
        return (uint8_t)0xa + (uint8_t)(c - 'A');
    }
    return 0;
}

/* convert a string,which length is 18, to a macaddress data type. */
#define MAC_STR_LEN            18
#define copy_str2mac(mac, str) \
    do {                                                                      \
        uint8_t i;                                                          \
        for (i = 0; i < OAL_MAC_ADDR_LEN; i++) {                              \
            (mac)[i] = (uint8_t)(a2x((str)[i * 3]) << 4) + a2x((str)[i * 3 + 1]); \
        }                                                                     \
    } while (0)

/* ip头到协议类型字段的偏移 */
#define IP_PROTOCOL_TYPE_OFFSET 9
#define IP_HDR_LEN              20

/* CCMP加密字节数 */
#define WLAN_CCMP_ENCRYP_LEN 16
/* CCMP256加密字节数 */
#define WLAN_CCMP256_GCMP_ENCRYP_LEN 24

/* is address mcast? */
#define ether_is_multicast(_a) ((uint8_t)(*(_a))&0x01)

/* is address bcast? */
#define ether_is_broadcast(_a) \
    ((_a)[0] == 0xff &&        \
        (_a)[1] == 0xff &&     \
        (_a)[2] == 0xff &&     \
        (_a)[3] == 0xff &&     \
        (_a)[4] == 0xff &&     \
        (_a)[5] == 0xff)

#define ether_is_all_zero(_a) \
    ((_a)[0] == 0x00 &&       \
        (_a)[1] == 0x00 &&    \
        (_a)[2] == 0x00 &&    \
        (_a)[3] == 0x00 &&    \
        (_a)[4] == 0x00 &&    \
        (_a)[5] == 0x00)

#define ether_is_ipv4_multicast(_a) (((_a)[0]) == 0x01 && \
                                     ((_a)[1]) == 0x00 && \
                                     ((_a)[2]) == 0x5e)

#define WLAN_DATA_VIP_TID WLAN_TIDNO_BCAST

/* wiphy  */
#define IEEE80211_HT_MCS_MASK_LEN 10

/* ICMP codes for neighbour discovery messages */
#define OAL_NDISC_ROUTER_SOLICITATION     133
#define OAL_NDISC_ROUTER_ADVERTISEMENT    134
#define OAL_NDISC_NEIGHBOUR_SOLICITATION  135
#define OAL_NDISC_NEIGHBOUR_ADVERTISEMENT 136
#define OAL_NDISC_REDIRECT                137

#define OAL_ND_OPT_TARGET_LL_ADDR 2
#define OAL_ND_OPT_SOURCE_LL_ADDR 1
#define OAL_IPV6_ADDR_ANY         0x0000U
#define OAL_IPV6_ADDR_MULTICAST   0x0002U
#define OAL_IPV6_MAC_ADDR_LEN     16


#define OAL_IPV4_ADDR_SIZE   4
#define OAL_IPV6_ADDR_SIZE   16
#define OAL_IP_ADDR_MAX_SIZE OAL_IPV6_ADDR_SIZE

/* IPv4多播范围: 224.0.0.0--239.255.255.255 */
#define oal_ipv4_is_multicast(_a) ((uint8_t)((_a)[0]) >= 224 && ((uint8_t)((_a)[0]) <= 239))

/* IPv4永久组地址判断: 224.0.0.0～224.0.0.255为永久组地址 */
#define OAL_IPV4_PERMANET_GROUP_ADDR    0x000000E0
#define oal_ipv4_is_permanent_group(_a) ((((_a)&0x00FFFFFF) ^ OAL_IPV4_PERMANET_GROUP_ADDR) == 0)

/* IPv6组播地址: FFXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX(第一个字节全一) */
#define oal_ipv6_is_multicast(_a) ((uint8_t)((_a)[0]) == 0xff)
/* ipv6 组播mac地址 */
#define ether_is_ipv6_multicast(_a) (((_a)[0]) == 0x33 && ((_a)[1]) == 0x33)
/* IPv6未指定地址: ::/128 ,该地址仅用于接口还没有被分配IPv6地址时与其它节点
   通讯作为源地址,例如在重复地址检测DAD中会出现. */
#define oal_ipv6_is_unspecified_addr(_a) \
    ((_a)[0] == 0x00 && (_a)[1] == 0x00 && (_a)[2] == 0x00 && (_a)[3] == 0x00 &&   \
     (_a)[4] == 0x00 && (_a)[5] == 0x00 && (_a)[6] == 0x00 && (_a)[7] == 0x00 &&   \
     (_a)[8] == 0x00 && (_a)[9] == 0x00 && (_a)[10] == 0x00 && (_a)[11] == 0x00 && \
     (_a)[12] == 0x00 && (_a)[13] == 0x00 && (_a)[14] == 0x00 && (_a)[15] == 0x00)

/* IPv6链路本地地址: 最高10位值为1111111010, 例如:FE80:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX  */
#define oal_ipv6_is_link_local_addr(_a) (((_a)[0] == 0xFE) && ((_a)[1] >> 6 == 2))

/* IGMP record type */
#define MAC_IGMP_QUERY_TYPE    0x11
#define MAC_IGMPV1_REPORT_TYPE 0x12
#define MAC_IGMPV2_REPORT_TYPE 0x16
#define MAC_IGMPV2_LEAVE_TYPE  0x17
#define MAC_IGMPV3_REPORT_TYPE 0x22

/* Is packet type is either leave or report */
#define is_igmp_report_leave_packet(type) ( \
    ((type) == MAC_IGMPV1_REPORT_TYPE) || \
    ((type) == MAC_IGMPV2_REPORT_TYPE) || \
    ((type) == MAC_IGMPV2_LEAVE_TYPE) || \
    ((type) == MAC_IGMPV3_REPORT_TYPE))

/* V3 group record types [grec_type] */
#define IGMPV3_MODE_IS_INCLUDE   1
#define IGMPV3_MODE_IS_EXCLUDE   2
#define IGMPV3_CHANGE_TO_INCLUDE 3
#define IGMPV3_CHANGE_TO_EXCLUDE 4
#define IGMPV3_ALLOW_NEW_SOURCES 5
#define IGMPV3_BLOCK_OLD_SOURCES 6

/* Is packet type is either leave or report */
#define is_igmpv3_mode(type) ( \
    ((type) == IGMPV3_MODE_IS_INCLUDE) || \
    ((type) == IGMPV3_MODE_IS_EXCLUDE) || \
    ((type) == IGMPV3_CHANGE_TO_INCLUDE) || \
    ((type) == IGMPV3_CHANGE_TO_EXCLUDE) || \
    ((type) == IGMPV3_ALLOW_NEW_SOURCES) || \
    ((type) == IGMPV3_BLOCK_OLD_SOURCES))

/* IGMP record type */
#define MLD_QUERY_TYPE    130
#define MLDV1_REPORT_TYPE 131
#define MLDV1_DONE_TYPE   132
#define MLDV2_REPORT_TYPE 143

/* Is packet type is either leave or report */
#define is_mld_report_leave_packet(type) ( \
    ((type) == MLDV1_REPORT_TYPE) || \
    ((type) == MLDV1_DONE_TYPE) || \
    ((type) == MLDV2_REPORT_TYPE))
/* MLD V2 group record types [grec_type] */
#define MLDV2_MODE_IS_INCLUDE   1
#define MLDV2_MODE_IS_EXCLUDE   2
#define MLDV2_CHANGE_TO_INCLUDE 3
#define MLDV2_CHANGE_TO_EXCLUDE 4
#define MLDV2_ALLOW_NEW_SOURCES 5
#define MLDV2_BLOCK_OLD_SOURCES 6

#define is_mldv2_mode(type) ( \
    ((type) == MLDV2_MODE_IS_INCLUDE) || \
    ((type) == MLDV2_MODE_IS_EXCLUDE) || \
    ((type) == MLDV2_CHANGE_TO_INCLUDE) || \
    ((type) == MLDV2_CHANGE_TO_EXCLUDE) || \
    ((type) == MLDV2_ALLOW_NEW_SOURCES) || \
    ((type) == MLDV2_BLOCK_OLD_SOURCES))

/* Calculate the group record length */
#define igmpv3_grp_rec_len(x) (8 + (((x)->us_grec_nsrcs + (x)->uc_grec_auxwords) << 2))
#define mldv2_grp_rec_len(x)  (sizeof(mac_mld_v2_group_record_stru) + \
                               ((x)->us_grec_srcaddr_num << 4) + \
                               (x)->uc_grec_auxwords)

/* Probe Rsp APP IE长度超过该值，发送帧netbuf采用大包 */
#define OAL_MGMT_NETBUF_APP_PROBE_RSP_IE_LEN_LIMIT 450

#ifdef _PRE_WLAN_FEATURE_SPECIAL_PKT_LOG
/* dhcp */
#define DHCP_CHADDR_LEN 16
#define SERVERNAME_LEN  64
#define BOOTFILE_LEN    128

/* DHCP message type */
#define DHCP_DISCOVER 1
#define DHCP_OFFER    2
#define DHCP_REQUEST  3
#define DHCP_ACK      5
#define DHCP_NAK      6

#define DHO_PAD         0
#define DHO_IPADDRESS   50
#define DHO_MESSAGETYPE 53
#define DHO_SERVERID    54
#define DHO_END         255

#define dns_get_qr_from_flag(flag)     ((uint8_t)(((flag & 0x8000U) > 0) ? 1 : 0))
#define dns_get_opcode_from_flag(flag) ((uint8_t)((flag & 0x7400U) >> 11))
#define dns_get_rcode_from_flag(flag)  ((uint8_t)(flag & 0x000fU))

#define DNS_MAX_DOMAIN_LEN 100

#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68
#define DNS_SERVER_PORT  53
#endif

/*
 * 枚举名  : oal_mem_state_enum_uint8
 * 协议表格:
 * 枚举说明: 内存块状态
 */
typedef enum {
    OAL_MEM_STATE_FREE = 0, /* 该内存空闲 */
    OAL_MEM_STATE_ALLOC,    /* 该内存已分配 */

    OAL_MEM_STATE_BUTT
} oal_mem_state_enum;
typedef uint8_t oal_mem_state_enum_uint8;

/* 枚举定义 */
/* 以下不区分操作系统 */
/* 内核下发的扫描类型 */
typedef enum {
    OAL_PASSIVE_SCAN = 0,
    OAL_ACTIVE_SCAN = 1,

    OAL_SCAN_BUTT
} oal_scan_enum;
typedef uint8_t oal_scan_enum_uint8;

/* 内核下发的扫描频段 */
typedef enum {
    OAL_SCAN_2G_BAND = 1,
    OAL_SCAN_5G_BAND = 2,
    OAL_SCAN_ALL_BAND = 3,

    OAL_SCAN_BAND_BUTT
} oal_scan_band_enum;
typedef uint8_t oal_scan_band_enum_uint8;

typedef enum {
    OAL_IEEE80211_MLME_AUTH = 0,        /* MLME下发认证帧相关内容 */
    OAL_IEEE80211_MLME_ASSOC_RSP = 1,   /* MLME下发关联帧相关内容 */
    OAL_IEEE80211_MLME_REASSOC_RSP = 2, /* MLME下发重关联帧相关内容 */
    OAL_IEEE80211_MLME_NUM
} en_mlme_type_enum;
typedef uint8_t en_mlme_type_enum_uint8;

/* hostapd 下发私有命令 */
enum HWIFI_IOCTL_CMD {
    /*
     * IOCTL_CMD的起始值由0修改为0x8EE0，修改原因：51 WiFi模块和类似于dhdutil之类的其他模块共用同一个ioctl通道，
     * 而51命令的枚举值从0开始，其他模块下发的ioctl命令也包含从0开始的这部分，这样就会同时“组播”到自己的模块和WiFi模块，
     * 从而对WiFi模块的功能产生影响。所以将51 WiFi模块命令的枚举值调整到0x8EE0起，便规避了其他模块命令的影响。
     */
    HWIFI_IOCTL_CMD_GET_STA_ASSOC_REQ_IE = 0x8EE0, /* get sta assocate request ie */
    HWIFI_IOCTL_CMD_SET_AP_AUTH_ALG,               /* set auth alg to driver */
    HWIFI_IOCTL_CMD_SET_COUNTRY,                   /* 设置国家码 */
    HWIFI_IOCTL_CMD_SET_SSID,                      /* 设置ssid */
    HWIFI_IOCTL_CMD_SET_MAX_USER,                  /* 设置最大用户数 */
    HWIFI_IOCTL_CMD_SET_FREQ,                      /* 设置频段 */
    HWIFI_IOCTL_CMD_SET_WPS_IE,                    /* 设置AP WPS 信息元素 */
    HWIFI_IOCTL_CMD_PRIV_CONNECT,                  /* linux-2.6.30 sta发起connect */
    HWIFI_IOCTL_CMD_PRIV_DISCONNECT,               /* linux-2.6.30 sta发起disconnect */
    HWIFI_IOCTL_CMD_SET_FRAG,                      /* 设置分片门限值 */
    HWIFI_IOCTL_CMD_SET_RTS,                       /* 设置RTS 门限值 */
    HWIFI_IOCTL_CMD_PRIV_KICK_USER,          /* AP剔除用户 */
    HWIFI_IOCTL_CMD_SET_VENDOR_IE,           /* AP 添加私有IE接口，原为HWIFI_IOCTL_CMD_SET_OKC_IE，是hilink设置okc ie专用 */
    HWIFI_IOCTL_CMD_SET_WHITE_LST_SSIDHIDEN, /* 设置hlink白名单 */
    HWIFI_IOCTL_CMD_FBT_SCAN,                /* 启动或停止hilink fbt侦听 */
    HWIFI_IOCTL_CMD_GET_ALL_STA_INFO,        /* 获取所有已关联STA链路信息 */
    HWIFI_IOCTL_CMD_GET_STA_INFO_BY_MAC,     /* 获取指定已关联STA链路信息 */
    HWIFI_IOCTL_CMD_GET_CUR_CHANNEL,         /* 获取工作信道 */
    HWIFI_IOCTL_CMD_SET_SCAN_STAY_TIME,      /* 设置扫描时工作信道驻留时间和侦听信道驻留时间 */
    HWIFI_IOCTL_CMD_SET_BEACON, /* hostapd只配置加密参数私有接口 */
    HWIFI_IOCTL_CMD_SET_MLME, /* 设置MLME操作（认证、关联） */
    HWIFI_IOCTL_CMD_GET_NEIGHB_INFO,      /* 获取邻居AP扫描信息 */
    HWIFI_IOCTL_CMD_GET_HW_STAT,          /* 获取硬件流量统计 */
    HWIFI_IOCTL_CMD_GET_WME_STAT,         /* 获取WME队列统计 */
    HWIFI_IOCTL_CMD_GET_STA_11V_ABILITY,  /* 获取指定STA的11v能力信息 */
    HWIFI_IOCTL_CMD_11V_CHANGE_AP,        /* 通知sta切换到指定ap */
    HWIFI_IOCTL_CMD_GET_STA_11K_ABILITY,  /* 获取指定STA的11v能力信息 */
    HWIFI_IOCTL_CMD_SET_STA_BCN_REQUEST,  /* 通知STA的上报beacon信息 */
    HWIFI_IOCTL_CMD_GET_SNOOP_TABLE,      /* 获取组播组及其成员mac */
    HWIFI_IOCTL_CMD_GET_ALL_STA_INFO_EXT, /* 获取所有已关联STA链路信息，包含增量部分 */
    HWIFI_IOCTL_CMD_GET_VAP_WDS_INFO,     /* 获取WDS VAP节点信息 */
    HWIFI_IOCTL_CMD_GET_STA_11H_ABILITY,  /* 获取指定STA的11h能力信息 */
    HWIFI_IOCTL_CMD_GET_STA_11R_ABILITY,  /* 获取指定STA的11r能力信息 */
    HWIFI_IOCTL_CMD_GET_TX_DELAY_AC,      /* 获取指定所有AC的发送时延信息 */
    HWIFI_IOCTL_CMD_GET_CAR_INFO,         /* 获取指定device下面的car限速配置信息 */
    HWIFI_IOCTL_CMD_GET_BLKWHTLST,        /* 获取黑白名单 */

    HWIFI_IOCTL_CMD_NUM
};

enum APP_IE_TYPE {
    OAL_APP_BEACON_IE = 0,
    OAL_APP_PROBE_REQ_IE = 1,
    OAL_APP_PROBE_RSP_IE = 2,
    OAL_APP_ASSOC_REQ_IE = 3,
    OAL_APP_ASSOC_RSP_IE = 4,
    OAL_APP_FT_IE = 5,
    OAL_APP_REASSOC_REQ_IE = 6,
    OAL_APP_VENDOR_IE = 9,

    OAL_APP_IE_NUM
};
typedef uint8_t en_app_ie_type_uint8;

typedef enum _wlan_net_queue_type_ {
    WLAN_HI_QUEUE = 0,
    WLAN_NORMAL_QUEUE,

    WLAN_TCP_DATA_QUEUE,
    WLAN_TCP_ACK_QUEUE,

    WLAN_UDP_BK_QUEUE,
    WLAN_UDP_BE_QUEUE,
    WLAN_UDP_VI_QUEUE,
    WLAN_UDP_VO_QUEUE,

    WLAN_NET_QUEUE_BUTT
} wlan_net_queue_type;

/* net_device ioctl结构体定义 */
/* hostapd/wpa_supplicant 下发的信息元素结构 */
/* 该结构为事件内存池大小，保存从hostapd/wpa_supplicant下发的ie 信息 */
/* 注意: 整个结构体长度为事件内存池大小，如果事件内存池有修改，则需要同步修改app 数据结构 */
struct oal_app_ie {
    uint32_t ie_len;
    en_app_ie_type_uint8 en_app_ie_type;
    uint8_t auc_rsv[3];
    /* auc_ie 中保存信息元素，长度 = (事件内存池大小 - 保留长度) */
    uint8_t auc_ie[WLAN_WPS_IE_MAX_SIZE];
};
typedef struct oal_app_ie oal_app_ie_stru;

/*
 * wal 层到hmac侧传递使用该数据，1103 WLAN_WPS_IE_MAX_SIZE扩容到608字节，超过事件队列大小，
 * wal到hmac ie data采用指针传递
 */
struct oal_w2h_app_ie {
    uint32_t ie_len;
    en_app_ie_type_uint8 en_app_ie_type;
    uint8_t auc_rsv[3];
    uint8_t *puc_data_ie;
    uint32_t delivery_time; /* wal to hmac的事件开始的时间戳 */
} __OAL_DECLARE_PACKED;
typedef struct oal_w2h_app_ie oal_w2h_app_ie_stru;

struct oal_mlme_ie {
    en_mlme_type_enum_uint8 en_mlme_type; /* MLME类型 */
    uint8_t uc_seq;                     /* 认证帧序列号 */
    uint16_t us_status;                 /* 状态码 */
    uint8_t auc_macaddr[OAL_MAC_ADDR_LEN];
    uint16_t us_ie_len;
    uint8_t auc_ie[WLAN_WPS_IE_MAX_SIZE];
};
typedef struct oal_mlme_ie oal_mlme_ie_stru;

struct oal_w2h_mlme_ie {
    en_mlme_type_enum_uint8 en_mlme_type; /* MLME类型 */
    uint8_t uc_seq;                     /* 认证帧序列号 */
    uint16_t us_status;                 /* 状态码 */
    uint8_t auc_macaddr[OAL_MAC_ADDR_LEN];
    uint16_t us_ie_len;
    uint8_t *puc_data_ie;
};
typedef struct oal_w2h_mlme_ie oal_w2h_mlme_ie_stru;

/*
 * Structure of the IP frame
 */
struct oal_ip_header {
#if (_PRE_LITTLE_CPU_ENDIAN == _PRE_CPU_ENDIAN) /* LITTLE_ENDIAN */
    uint8_t us_ihl : 4,
              uc_version_ihl : 4;
#else
    uint8_t uc_version_ihl : 4,
              us_ihl : 4;
#endif
    /* liuming modifed proxyst end */
    uint8_t uc_tos;
    uint16_t us_tot_len;
    uint16_t us_id;
    uint16_t us_frag_off;
    uint8_t uc_ttl;
    uint8_t uc_protocol;
    uint16_t us_check;
    uint32_t saddr;
    uint32_t daddr;
    /* The options start here. */
} __OAL_DECLARE_PACKED;
typedef struct oal_ip_header oal_ip_header_stru;

typedef struct {
    uint16_t us_sport;
    uint16_t us_dport;
    uint32_t seqnum;
    uint32_t acknum;
    uint8_t uc_offset;
    uint8_t uc_flags;
    uint16_t us_window;
    uint16_t us_check;
    uint16_t us_urgent;
} oal_tcp_header_stru;

typedef struct {
    uint16_t source;
    uint16_t dest;
    uint16_t len;
    uint16_t check;
} oal_udp_header_stru;

/*
 *    Header in on cable format
 */
struct mac_igmp_header {
    uint8_t uc_type;
    uint8_t uc_code; /* For newer IGMP */
    uint16_t us_csum;
    uint32_t ul_group;
} __OAL_DECLARE_PACKED;
typedef struct mac_igmp_header mac_igmp_header_stru;

/*
 *  Group record format
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |  Record Type  |  Aux Data Len |     Number of Sources (N)     |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                       Multicast Address                       |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                       Source Address [1]                      |
 *    +-                                                             -+
 *    |                       Source Address [2]                      |
 *    +-                                                             -+
 *    .                               .                               .
 *    .                               .                               .
 *    .                               .                               .
 *    +-                                                             -+
 *    |                       Source Address [N]                      |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                                                               |
 *    .                                                               .
 *    .                         Auxiliary Data                        .
 *    .                                                               .
 *    |                                                               |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
struct mac_igmp_v3_grec {
    uint8_t uc_grec_type;
    uint8_t uc_grec_auxwords;
    uint16_t us_grec_nsrcs;
    uint32_t ul_grec_group_ip;
} __OAL_DECLARE_PACKED;
typedef struct mac_igmp_v3_grec mac_igmp_v3_grec_stru;

/*
 * IGMPv3 report format
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |  Type = 0x22  |    Reserved   |           Checksum            |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |           Reserved            |  Number of Group Records (M)  |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                                                               |
 *   .                                                               .
 *   .                        Group Record [1]                       .
 *   .                                                               .
 *   |                                                               |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                                                               |
 *   .                                                               .
 *   .                        Group Record [2]                       .
 *   .                                                               .
 *   |                                                               |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                               .                               |
 *   .                               .                               .
 *   |                               .                               |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                                                               |
 *   .                                                               .
 *   .                        Group Record [M]                       .
 *   .                                                               .
 *   |                                                               |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
struct mac_igmp_v3_report {
    uint8_t uc_type;
    uint8_t uc_resv1;
    uint16_t us_csum;
    uint16_t us_resv2;
    uint16_t us_ngrec;
} __OAL_DECLARE_PACKED;
typedef struct mac_igmp_v3_report mac_igmp_v3_report_stru;

struct mac_mld_v1_head {
    uint8_t uc_type;
    uint8_t uc_code;
    uint16_t us_check_sum;
    uint16_t us_max_response_delay;
    uint16_t us_reserved;
    uint8_t auc_group_ip[OAL_IPV6_ADDR_SIZE];
} __OAL_DECLARE_PACKED;
typedef struct mac_mld_v1_head mac_mld_v1_head_stru;

struct mac_mld_v2_report {
    uint8_t uc_type;
    uint8_t uc_code;
    uint16_t us_check_sum;
    uint16_t us_reserved;
    uint16_t us_group_address_num;
} __OAL_DECLARE_PACKED;
typedef struct mac_mld_v2_report mac_mld_v2_report_stru;

struct mac_mld_v2_group_record {
    uint8_t uc_grec_type;
    uint8_t uc_grec_auxwords;                  // 辅助数据长度
    uint16_t us_grec_srcaddr_num;              //  原地址个数
    uint8_t auc_group_ip[OAL_IPV6_ADDR_SIZE];  // 组播组地址
} __OAL_DECLARE_PACKED;
typedef struct mac_mld_v2_group_record mac_mld_v2_group_record_stru;

struct mac_vlan_tag {
    uint16_t us_tpid;           // tag   ID
    uint16_t bit_user_pri : 3,  // 帧的优先级
               bit_cfi : 1,
               bit_vlan_id : 12;  // 可配置的VLAN ID
} __OAL_DECLARE_PACKED;
typedef struct mac_vlan_tag mac_vlan_tag_stru;

/* WIN32和linux共用结构体  */
typedef struct {
    uint8_t uc_type;
    uint8_t uc_len;
    uint8_t uc_addr[OAL_ETH_HAEDWARE_ADDR_LEN]; /* hardware address */
} oal_eth_icmp6_lladdr_stru;

typedef struct {
    uint8_t op;                                     /* packet opcode type */
    uint8_t htype;                                  /* hardware addr type */
    uint8_t hlen;                                   /* hardware addr length */
    uint8_t hops;                                   /* gateway hops */
    uint32_t xid;                                   /* transaction ID */
    uint16_t secs;                                  /* seconds since boot began */
    uint16_t flags;                                 /* flags */
    uint32_t ciaddr;                                /* client IP address */
    uint32_t yiaddr;                                /* 'your' IP address */
    uint32_t siaddr;                                /* server IP address */
    uint32_t giaddr;                                /* gateway IP address */
    uint8_t chaddr[OAL_DHCP_CLIENT_ADDR_LEN];       /* client hardware address */
    uint8_t sname[OAL_DHCP_SERVER_NAME_LEN];        /* server host name */
    uint8_t file[OAL_DHCP_BOOT_FILE_NAME_LEN];      /* boot file name */
    uint8_t options[OAL_DHCP_VERIABLE_LEN_OPTIONS]; /* variable-length options field */
} oal_dhcp_packet_stru;

#ifdef _PRE_WLAN_FEATURE_SPECIAL_PKT_LOG
typedef struct {
    uint16_t id;      /* transaction id */
    uint16_t flags;   /* message future */
    uint16_t qdcount; /* question record count */
    uint16_t ancount; /* answer record count */
    uint16_t nscount; /* authority record count */
    uint16_t arcount; /* additional record count */
} oal_dns_hdr_stru;

typedef enum {
    OAL_NS_Q_REQUEST = 0,  /* request */
    OAL_NS_Q_RESPONSE = 1, /* response */
} oal_ns_qrcode;

typedef enum {
    OAL_NS_O_QUERY = 0,  /* Standard query. */
    OAL_NS_O_IQUERY = 1, /* Inverse query (deprecated/unsupported). */
} oal_ns_opcode;

/*
 * Currently defined response codes.
 */
typedef enum {
    OAL_NS_R_NOERROR = 0, /* No error occurred. */
} oal_ns_rcode;

typedef enum {
    OAL_NS_T_INVALID = 0, /* Cookie. */
    OAL_NS_T_A = 1,       /* Host address. */
} oal_ns_type;
#endif

/* 不分平台通用结构体 */
typedef struct {
    uint8_t auc_ssid[OAL_IEEE80211_MAX_SSID_LEN]; /* ssid array */
    uint8_t uc_ssid_len;                          /* length of the array */
    uint8_t auc_arry[OAL_SSID_ARRY_LEN];
} oal_ssids_stru;

struct hostap_sta_link_info {
    uint8_t addr[OAL_MAC_ADDR_LEN];
    uint8_t rx_rssi;     /* 0 ~ 100 ,0xff为无效值 */
    uint8_t tx_pwr;      /* 0 ~ 100 */
    uint32_t rx_rate;    /* avr nego rate in kpbs */
    uint32_t tx_rate;    /* avr nego rate in kpbs */
    uint32_t rx_minrate; /* min nego rx rate in last duration, clean to 0 when app read over */
    uint32_t rx_maxrate; /* max nego rx rate in last duration, clean to 0 when app read over */
    uint32_t tx_minrate; /* min nego tx rate in last duration, clean to 0 when app read over */
    uint32_t tx_maxrate; /* max nego tx rate in last duration, clean to 0 when app read over */
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint32_t tx_frames_rty;  /* tx frame retry cnt */
    uint32_t tx_frames_all;  /* tx total frame cnt */
    uint32_t tx_frames_fail; /* tx fail */
    uint32_t snr;            /* snr */
};

struct hostap_all_sta_link_info {
    unsigned long buf_cnt; /* input: sta_info count provided  */
    unsigned long sta_cnt; /* outpu: how many sta really */
    unsigned int cur_channel;
    struct hostap_sta_link_info *sta_info; /* output */
};
typedef struct hostap_sta_link_info oal_net_sta_link_info_stru;
typedef struct hostap_all_sta_link_info oal_net_all_sta_link_info_stru;

/* sta包含增量信息结构体 */
struct hostap_sta_link_info_ext {
    uint8_t uc_auth_st;              /* 认证状态 */
    oal_bool_enum_uint8 en_band;       /* 工作频段 */
    oal_bool_enum_uint8 en_wmm_switch; /* wmm是否使能 */
    uint8_t uc_ps_st;                /* 节能状态 */
    uint8_t uc_sta_num;              /* 空间流数 */
    uint8_t uc_work_mode;            /* 协议模式 */
    int8_t c_noise;                  /* 节点的噪声值 */
    uint8_t auc_resv[1];
    uint32_t ul_associated_time; /* 用户已连接的时长 */
};

struct hostap_all_sta_link_info_ext {
    unsigned long buf_cnt; /* input: sta_info count provided  */
    unsigned long sta_cnt; /* outpu: how many sta really */
    unsigned int cur_channel;
    struct hostap_sta_link_info *sta_info;
    struct hostap_sta_link_info_ext *sta_info_ext;
};
typedef struct hostap_sta_link_info_ext oal_net_sta_link_info_ext_stru;
typedef struct hostap_all_sta_link_info_ext oal_net_all_sta_link_info_ext_stru;

/* lint -e958 */ /* 屏蔽字节对齐错误 */
/* RRM ENABLED CAP信息元素结构体 */
struct oal_rrm_enabled_cap_ie {
    uint8_t bit_link_cap : 1,                  /* bit0: Link Measurement capability enabled */
            bit_neighbor_rpt_cap : 1,          /* bit1: Neighbor Report capability enabled */
            bit_parallel_cap : 1,              /* bit2: Parallel Measurements capability enabled */
            bit_repeat_cap : 1,                /* bit3: Repeated Measurements capability enabled */
            bit_bcn_passive_cap : 1,           /* bit4: Beacon Passive Measurements capability enabled */
            bit_bcn_active_cap : 1,            /* bit5: Beacon Active Measurements capability enabled */
            bit_bcn_table_cap : 1,             /* bit6: Beacon Table Measurements capability enabled */
            bit_bcn_meas_rpt_cond_cap : 1;     /* bit7: Beacon Measurement Reporting Conditions capability enabled */
    uint8_t bit_frame_cap : 1,                 /* bit8: Frame Measurement capability enabled */
            bit_chn_load_cap : 1,              /* bit9: Channel Load Measurement capability enabled */
            bit_noise_histogram_cap : 1,       /* bit10: Noise Histogram Measurement capability enabled */
            bit_stat_cap : 1,          /* bit11: Statistics Measurement capability enabled */
            bit_lci_cap : 1,           /* bit12: LCI Measurement capability enabled */
            bit_lci_azimuth_cap : 1,   /* bit13: LCI Azimuth capability enabled */
            bit_tsc_cap : 1,           /* bit14: Transmit Stream/Category Measurement capability enabled */
            bit_triggered_tsc_cap : 1; /* bit15: Triggered  Transmit Stream/Category Measurement capability enabled */
    uint8_t bit_ap_chn_rpt_cap : 1,    /* bit16: AP Channel Report capability enabled */
            bit_rm_mib_cap : 1,                     /* bit17: RM MIB capability enabled */
            bit_oper_chn_max_meas_duration : 3,     /* bit18-20: Operating Channel Max Measurement Duration */
            bit_non_oper_chn_max_meas_duration : 3; /* bit21-23: Non-operating Channel Max Measurement Durationg */
    uint8_t bit_meas_pilot_cap : 3,                 /* bit24-26: Measurement Pilot capability */
            bit_meas_pilot_trans_info_cap : 1,      /* bit27: Pilot Transmission Information capability enabled */
            bit_neighbor_rpt_tsf_offset_cap : 1,    /* bit28: Neighbor Report TSF Offset capability enabled */
            bit_rcpi_cap : 1,                       /* bit29: RCPI Measurement capability enabled */
            bit_rsni_cap : 1,                       /* bit30: RSNI Measurement capability enabled */
            bit_bss_avg_access_dly : 1;             /* bit31: BSS Average Access Delay capability enabled */
    uint8_t bit_avail_admission_capacity_cap : 1,   /* bit32: BSS Available Admission Capacity capability enabled */
            bit_antenna_cap : 1,                    /* bit33: Antenna capability enabled */
            bit_ftm_range_report_cap : 1,           /* bit34: FTM range report capability enabled */
            bit_rsv : 5;                            /* bit35-39: Reserved */
} __OAL_DECLARE_PACKED;
typedef struct oal_rrm_enabled_cap_ie oal_rrm_enabled_cap_ie_stru;
/*lint +e958*/
/* 邻居AP列表的BSS描述信息结构体 */
struct oal_bssid_infomation {
    uint8_t bit_ap_reachability : 2,           /* AP的可到达性 */
            bit_security : 1,                  /* 该AP的加密规则与当前连接是否一致 */
            bit_key_scope : 1,                 /* 该AP的认证信息是否与当前上报一直 */
            bit_spectrum_mgmt : 1,             /* 能力位: 支持频谱管理 */ /* 能力位字段与beacon定义一致 */
            bit_qos : 1,                       /* 能力位: 支持QOS */
            bit_apsd : 1,                      /* 能力位: 支持APSD */
            bit_radio_meas : 1;                /* 能力位: 波长测量 */
    uint8_t bit_delay_block_ack : 1,           /* 能力位: 阻塞延迟应答 */
            bit_immediate_block_ack : 1,       /* 能力位: 阻塞立即应答 */
            bit_mobility_domain : 1,           /* 该AP的beacon帧中是否含有MDE，且与此次上报一致 */
            bit_high_throughput : 1,           /* 该AP的beacon帧中是否含有高吞吐量元素，且与此次上报一致 */
            bit_resv1 : 4;                     /* 预留 */
    uint8_t bit_resv2;
    uint8_t bit_resv3;
} __OAL_DECLARE_PACKED;
typedef struct oal_bssid_infomation oal_bssid_infomation_stru;

typedef struct {
    uint8_t auc_sta_mac[OAL_MAC_ADDR_LEN]; /* 要获取的sta的mac */
    oal_bool_enum_uint8 en_support_11h;      /* 保存sta是否支持11h，为出参 */
    uint8_t reserve;
} oal_hilink_get_sta_11h_ability;

typedef struct {
    uint8_t auc_sta_mac[OAL_MAC_ADDR_LEN]; /* 要获取的sta的mac */
    oal_bool_enum_uint8 en_support_11r;      /* 保存sta是否支持11r，为出参 */
    uint8_t reserve;
} oal_hilink_get_sta_11r_ability;

typedef struct {
    uint8_t uc_blkwhtlst_cnt; /* 黑白名单个数 */
    uint8_t uc_mode;          /* 黑白名单模式 */
    uint8_t auc_resv[2];
    uint8_t auc_blkwhtlst_mac[MAX_BLACKLIST_NUM][OAL_MAC_ADDR_LEN];
} oal_blkwhtlst_stru;

/* net_device ioctl结构体定义 */
typedef struct oal_net_dev_ioctl_data_tag {
    int32_t l_cmd; /* 命令号 */
    union {
        struct {
            uint8_t auc_mac[OAL_MAC_ADDR_LEN];
            uint8_t auc_rsv[2];
            uint32_t ul_buf_size; /* 用户空间ie 缓冲大小 */
            uint8_t *puc_buf;     /* 用户空间ie 缓冲地址 */
        } assoc_req_ie;             /* AP 模式，用于获取STA 关联请求ie 信息 */

        struct {
            uint32_t auth_alg;
        } auth_params;

        struct {
            uint8_t auc_country_code[OAL_NET_DEV_COUNTRY_CODE_LEN];
        } country_code;

        uint8_t ssid[OAL_IEEE80211_MAX_SSID_LEN + OAL_IEEE80211_EXTRA_SSID_LEN];
        uint32_t ul_vap_max_user;

        struct {
            int32_t l_freq;
            int32_t l_channel;
            int32_t l_ht_enabled;
            int32_t l_sec_channel_offset;
            int32_t l_vht_enabled;
            int32_t l_center_freq1;
            int32_t l_center_freq2;
            int32_t l_bandwidth;
        } freq;

        oal_app_ie_stru st_app_ie; /* beacon ie,index 0; proberesp ie index 1; assocresp ie index 2 */

        struct {
            int32_t l_freq;    /* ap所在频段，与linux-2.6.34内核中定义不同 */
            uint32_t ssid_len; /* SSID 长度 */
            uint32_t ie_len;

            uint8_t *puc_ie;
            const uint8_t *puc_ssid;  /* 期望关联的AP SSID  */
            const uint8_t *puc_bssid; /* 期望关联的AP BSSID  */

            uint8_t en_privacy;                          /* 是否加密标志 */
            oal_nl80211_auth_type_enum_uint8 en_auth_type; /* 认证类型，OPEN or SHARE-KEY */

            uint8_t uc_wep_key_len;         /* WEP KEY长度 */
            uint8_t uc_wep_key_index;       /* WEP KEY索引 */
            const uint8_t *puc_wep_key; /* WEP KEY密钥 */

            oal_cfg80211_crypto_settings_stru st_crypto; /* 密钥套件信息 */
        } cfg80211_connect_params;
        struct {
            uint8_t auc_mac[OAL_MAC_ADDR_LEN];
            uint16_t us_reason_code; /* 去关联 reason code */
        } kick_user_params;

        oal_net_all_sta_link_info_stru all_sta_link_info;

        oal_hilink_get_sta_11h_ability fbt_get_sta_11h_ability;

        int32_t l_frag; /* 分片门限值 */
        int32_t l_rts;  /* RTS 门限值 */
        oal_net_all_sta_link_info_ext_stru all_sta_link_info_ext;

        oal_blkwhtlst_stru st_blkwhtlst;
    } pri_data;
} oal_net_dev_ioctl_data_stru;

/* 函数声明 */
extern oal_bool_enum_uint8 oal_netbuf_is_dhcp_port(oal_udp_header_stru *pst_udp_hdr);
extern oal_bool_enum_uint8 oal_netbuf_is_nd(oal_ipv6hdr_stru *pst_ipv6hdr);
extern oal_bool_enum_uint8 oal_netbuf_is_dhcp6(oal_ipv6hdr_stru *pst_ether_hdr);

extern oal_bool_enum_uint8 oal_netbuf_is_tcp_ack6(oal_ipv6hdr_stru *pst_ipv6hdr);
extern uint16_t oal_netbuf_select_queue(oal_netbuf_stru *pst_buf);
extern oal_bool_enum_uint8 oal_netbuf_is_tcp_ack(oal_ip_header_stru *pst_ip_hdr);
extern oal_bool_enum_uint8 oal_netbuf_is_icmp(oal_ip_header_stru *pst_ip_hdr);
extern int32_t dev_netlink_send(uint8_t *data, int data_len);
extern int32_t init_dev_excp_handler(void);
extern void deinit_dev_excp_handler(void);
#endif /* end of oal_net.h */
