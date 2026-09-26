

#ifndef __OAL_LINUX_NET_H__
#define __OAL_LINUX_NET_H__

/* 其他头文件包含 */
#include <linux/version.h>
#include <net/iw_handler.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/hardirq.h>
#include <linux/bottom_half.h>
#include <linux/if.h>
#include <linux/etherdevice.h>
#include <linux/wireless.h>
#include <net/rtnetlink.h>
#include <net/netlink.h>
#include <linux/socket.h>
#include <linux/if.h>
#include <net/sch_generic.h>
#include <linux/if_vlan.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/rtnetlink.h>
#include <linux/netfilter_bridge.h>
#include <linux/if_arp.h>
#include <linux/in.h>
#include <linux/udp.h>
#include <linux/icmpv6.h>
#include <linux/ipv6.h>
#include <linux/inet.h>
#include <net/ip6_checksum.h>
#include <net/ipv6.h>
#include <net/arp.h>
#include "securec.h"
#include <net/ieee80211_radiotap.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
#include <net/sock.h>
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
#include <net/cfg80211.h>
#else
#include <net/wireless.h>
#endif
#include <linux/nl80211.h>
#include <linux/ieee80211.h>
#include <../net/wireless/nl80211.h>

#include <linux/kobject.h>

#include <linux/kernel_stat.h>

/* 宏定义 */
#define OAL_BITFIELD_LITTLE_ENDIAN 0
#define OAL_BITFIELD_BIG_ENDIAN    1

#define OAL_WLAN_SA_QUERY_TR_ID_LEN 2
/* wiphy  */
#define IEEE80211_HT_MCS_MASK_LEN 10
#define OAL_IF_NAME_SIZE          16
#define ETHER_ADDR_LEN            6 /* length of an Ethernet address */

/*
  2.10 IP宏定义
 */
#define IPV6_ADDR_MULTICAST         0x0002U
#define IPV6_ADDR_UNICAST           0x0001U
#define ipv6_addr_scope_type(scope) ((scope) << 16)
#define IPV6_ADDR_SCOPE_NODELOCAL   0x01
#define IPV6_ADDR_SCOPE_LINKLOCAL   0x02
#define IPV6_ADDR_SCOPE_SITELOCAL   0x05
#define IPV6_ADDR_SCOPE_ORGLOCAL    0x08
#define IPV6_ADDR_SCOPE_GLOBAL      0x0e
#define IPV6_ADDR_LOOPBACK          0x0010U
#define IPV6_ADDR_LINKLOCAL         0x0020U
#define IPV6_ADDR_SITELOCAL         0x0040U
#define IPV6_ADDR_RESERVED          0x2000U /* reserved address space */

#define ipv6_addr_mc_scope(a) \
    ((a)->s6_addr[1] & 0x0f) /* nonstandard */

/* NAPI */
#define NAPI_POLL_WEIGHT_MAX           64
#define NAPI_POLL_WEIGHT_LEV1          1
#define NAPI_POLL_WEIGHT_LEV2          4
#define NAPI_POLL_WEIGHT_LEV3          8
#define NAPI_STAT_PERIOD               1000
#define NAPI_WATER_LINE_LEV1           12300
#define NAPI_WATER_LINE_LEV2           24600
#define NAPI_WATER_LINE_LEV3           38400
#define NAPI_NETDEV_PRIV_QUEUE_LEN_MAX 4096

/* ether type */
#define ETHER_TYPE_PAE      0x888e /* EAPOL PAE/802.1x */
#define ETHER_TYPE_IP       0x0800 /* IP protocol */
#define ETHER_TYPE_AARP     0x80f3 /* Appletalk AARP protocol */
#define ETHER_TYPE_IPX      0x8137 /* IPX over DIX protocol */
#define ETHER_TYPE_ARP      0x0806 /* ARP protocol */
#define ETHER_TYPE_IPV6     0x86dd /* IPv6 */
#define ETHER_TYPE_TDLS     0x890d /* TDLS */
#define ETHER_TYPE_VLAN     0x8100 /* VLAN TAG protocol */
#define ETHER_TYPE_WAI      0x88b4 /* WAI/WAPI */
#define ETHER_LLTD_TYPE     0x88D9 /* LLTD */
#define ETHER_ONE_X_TYPE    0x888E /* 802.1x Authentication */
#define ETHER_TUNNEL_TYPE   0x88bd /* 自定义tunnel协议 */
#define ETHER_TYPE_PPP_DISC 0x8863 /* PPPoE discovery messages */
#define ETHER_TYPE_PPP_SES  0x8864 /* PPPoE session messages */
#define ETHER_TYPE_HIGAME_DC  0x0888 /* HIGAME DC */

#define ETH_SENDER_IP_ADDR_LEN 4 /* length of an Ethernet send ip address */
#define ETH_TARGET_IP_ADDR_LEN 4 /* length of an Ethernet target ip address */

typedef iw_handler oal_iw_handler;
#define oal_netif_running(_pst_net_dev)                              netif_running(_pst_net_dev)
#define oal_smp_mb()                                                 smp_mb()
#define oal_container_of(_member_ptr, _stru_type, _stru_member_name) \
    container_of(_member_ptr, _stru_type, _stru_member_name)

#define oal_netbuf_list_num(_pst_head)      ((_pst_head)->qlen)
#define oal_net_dev_priv(_pst_dev)          ((_pst_dev)->ml_priv)
#define oal_net_dev_wireless_priv(_pst_dev) (netdev_priv(_pst_dev))
#define oal_net_dev_wireless_dev(_pst_dev)  ((_pst_dev)->ieee80211_ptr)
#define oal_netbuf_next(_pst_buf)           ((_pst_buf)->next)
#define oal_netbuf_prev(_pst_buf)           ((_pst_buf)->prev)
#define oal_netbuf_head_next(_pst_buf_head) ((_pst_buf_head)->next)
#define oal_netbuf_head_prev(_pst_buf_head) ((_pst_buf_head)->prev)

#define oal_netbuf_protocol(_pst_buf) ((_pst_buf)->protocol)
#define oal_netbuf_last_rx(_pst_buf)  ((_pst_buf)->last_rx)
#define get_netbuf_payload(_pst_buf)  ((_pst_buf)->data)
#define oal_netbuf_dev(_pst_buf)      ((_pst_buf)->dev)
#define oal_netbuf_cb_size()     (sizeof(((oal_netbuf_stru *)0)->cb))
#define oal_netbuf_len(_pst_buf) ((_pst_buf)->len)
#define OAL_NETBUF_TAIL          skb_tail_pointer

#ifdef _PRE_WLAN_PKT_TIME_STAT
#define OAL_NETBUF_CB_ORIGIN 48
#endif
#define oal_netdevice_ops(_pst_dev)                     ((_pst_dev)->netdev_ops)
#define oal_netdevice_ops_open(_pst_netdev_ops)         ((_pst_netdev_ops)->ndo_open)
#define oal_netdevice_ops_stop(_pst_netdev_ops)         ((_pst_netdev_ops)->ndo_stop)
#define oal_netdevice_ops_start_xmit(_pst_netdev_ops)   ((_pst_netdev_ops)->ndo_start_xmit)
#define oal_netdevice_ops_set_mac_addr(_pst_netdev_ops) ((_pst_netdev_ops)->ndo_set_mac_address)
#define oal_netdevice_ops_tx_timeout(_pst_netdev_ops)   ((_pst_netdev_ops)->ndo_tx_timeout)
#define oal_netdevice_ops_set_mc_list(_pst_netdev_ops)  ((_pst_netdev_ops)->ndo_set_multicast_list)
#define oal_netdevice_ops_get_stats(_pst_netdev_ops)    ((_pst_netdev_ops)->ndo_get_stats)
#define oal_netdevice_ops_do_ioctl(_pst_netdev_ops)     ((_pst_netdev_ops)->ndo_do_ioctl)
#define oal_netdevice_ops_change_mtu(_pst_netdev_ops)   ((_pst_netdev_ops)->ndo_change_mtu)

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0))
#define oal_netdevice_last_rx(_pst_dev) ((_pst_dev)->last_rx)
#endif
#ifdef CONFIG_WIRELESS_EXT
#define oal_netdevice_wireless_handlers(_pst_dev) ((_pst_dev)->wireless_handlers)
#endif
#define oal_netdevice_rtnl_link_ops(_pst_dev)    ((_pst_dev)->rtnl_link_ops)
#define oal_netdevice_rtnl_link_state(_pst_dev)  ((_pst_dev)->rtnl_link_state)
#define oal_netdevice_mac_addr(_pst_dev)         ((_pst_dev)->dev_addr)
#define oal_netdevice_tx_queue_len(_pst_dev)     ((_pst_dev)->tx_queue_len)
#define oal_netdevice_tx_queue_num(_pst_dev)     ((_pst_dev)->num_tx_queues)
#define oal_netdevice_tx_queue(_pst_dev, _index) ((_pst_dev)->_tx[_index])
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
#define oal_netdevice_destructor(_pst_dev) ((_pst_dev)->priv_destructor)
#else
#define oal_netdevice_destructor(_pst_dev) ((_pst_dev)->destructor)
#endif
#define oal_netdevice_type(_pst_dev)   ((_pst_dev)->type)
#define oal_netdevice_name(_pst_dev)   ((_pst_dev)->name)
#define oal_netdevice_master(_pst_dev) ((_pst_dev)->master)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34)
#define oal_netdevice_qdisc(_pst_dev, pst_val) ((_pst_dev)->qdisc = pst_val)
#else
#define oal_netdevice_qdisc(_pst_dev, pst_val)
#endif
#define oal_netdevice_ifalias(_pst_dev)         ((_pst_dev)->ifalias)
#define oal_netdevice_wdev(_pst_dev)            ((_pst_dev)->ieee80211_ptr)
#define oal_netdevice_headroom(_pst_dev)        ((_pst_dev)->needed_headroom)
#define oal_netdevice_tailroom(_pst_dev)        ((_pst_dev)->needed_tailroom)
#define oal_netdevice_flags(_pst_dev)           ((_pst_dev)->flags)
#define oal_netdevice_addr_len(_pst_dev)        ((_pst_dev)->addr_len)
#define oal_netdevice_watchdog_timeo(_pst_dev)  ((_pst_dev)->watchdog_timeo)
#define oal_netdevice_hard_header_len(_pst_dev) ((_pst_dev)->hard_header_len)

#define oal_wireless_dev_net_dev(_pst_wireless_dev) ((_pst_wireless_dev)->netdev)
#define oal_wireless_dev_wiphy(_pst_wireless_dev)   ((_pst_wireless_dev)->wiphy)
#define oal_wireless_dev_if_type(_pst_wireless_dev) ((_pst_wireless_dev)->iftype)

#define OAL_IFF_RUNNING     IFF_RUNNING
#define OAL_SIOCIWFIRSTPRIV SIOCIWFIRSTPRIV

/* iw_priv参数类型OAL封装 */
#define OAL_IW_PRIV_TYPE_BYTE  IW_PRIV_TYPE_BYTE  /* Char as number */
#define OAL_IW_PRIV_TYPE_CHAR  IW_PRIV_TYPE_CHAR  /* Char as character */
#define OAL_IW_PRIV_TYPE_INT   IW_PRIV_TYPE_INT   /* 32 bits int */
#define OAL_IW_PRIV_TYPE_FLOAT IW_PRIV_TYPE_FLOAT /* struct iw_freq */
#define OAL_IW_PRIV_TYPE_ADDR  IW_PRIV_TYPE_ADDR  /* struct sockaddr */
#define OAL_IW_PRIV_SIZE_FIXED IW_PRIV_SIZE_FIXED /* Variable or fixed number of args */
#define OAL_IW_PRIV_SIZE_MASK  IW_PRIV_SIZE_MASK  /* Max number of those args */
#define OAL_IW_PRIV_TYPE256    256
#define OAL_IW_PRIV_TYPE1024   1024
#define OAL_IW_PRIV_TYPE40     40

/* iwconfig mode oal封装 */
#define OAL_IW_MODE_AUTO    IW_MODE_AUTO    /* Let the driver decides */
#define OAL_IW_MODE_ADHOC   IW_MODE_ADHOC   /* Single cell network */
#define OAL_IW_MODE_INFRA   IW_MODE_INFRA   /* Multi cell network, roaming, ... */
#define OAL_IW_MODE_MASTER  IW_MODE_MASTER  /* Synchronisation master or Access Point */
#define OAL_IW_MODE_REPEAT  IW_MODE_REPEAT  /* Wireless Repeater (forwarder) */
#define OAL_IW_MODE_SECOND  IW_MODE_SECOND  /* Secondary master/repeater (backup) */
#define OAL_IW_MODE_MONITOR IW_MODE_MONITOR /* Passive monitor (listen only) */
#define OAL_IW_MODE_MESH    IW_MODE_MESH    /* Mesh (IEEE 802.11s) network */

/* Transmit Power flags available */
#define OAL_IW_TXPOW_TYPE     IW_TXPOW_TYPE     /* Type of value */
#define OAL_IW_TXPOW_DBM      IW_TXPOW_DBM      /* Value is in dBm */
#define OAL_IW_TXPOW_MWATT    IW_TXPOW_MWATT    /* Value is in mW */
#define OAL_IW_TXPOW_RELATIVE IW_TXPOW_RELATIVE /* Value is in arbitrary units */
#define OAL_IW_TXPOW_RANGE    IW_TXPOW_RANGE    /* Range of value between min/max */

/* 主机与网络字节序转换 */
#define oal_host2net_short(_x) htons(_x)
#define oal_net2host_short(_x) ntohs(_x)
#define oal_host2net_long(_x)  htonl(_x)
#define oal_net2host_long(_x)  ntohl(_x)

#define OAL_INET_ECN_NOT_ECT INET_ECN_NOT_ECT
#define OAL_INET_ECN_ECT_1   INET_ECN_ECT_1
#define OAL_INET_ECN_ECT_0   INET_ECN_ECT_0
#define OAL_INET_ECN_CE      INET_ECN_CE
#define OAL_INET_ECN_MASK    INET_ECN_MASK

/* 提取vlan信息 */
#define oal_vlan_tx_tag_present(_skb) vlan_tx_tag_present(_skb)
#define oal_vlan_tx_tag_get(_skb)     vlan_tx_tag_get(_skb)

/* vlan宏定义 */
#define OAL_VLAN_VID_MASK  VLAN_VID_MASK  /* VLAN Identifier */
#define OAL_VLAN_PRIO_MASK VLAN_PRIO_MASK /* Priority Code Point */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34)
#define OAL_VLAN_PRIO_SHIFT VLAN_PRIO_SHIFT
#else
#define OAL_VLAN_PRIO_SHIFT 13
#endif

/* ARP protocol opcodes. */
#define OAL_ARPOP_REQUEST   ARPOP_REQUEST   /* ARP request          */
#define OAL_ARPOP_REPLY     ARPOP_REPLY     /* ARP reply            */
#define OAL_ARPOP_RREQUEST  ARPOP_RREQUEST  /* RARP request         */
#define OAL_ARPOP_RREPLY    ARPOP_RREPLY    /* RARP reply           */
#define OAL_ARPOP_INREQUEST ARPOP_InREQUEST /* InARP request        */
#define OAL_ARPOP_INREPLY   ARPOP_InREPLY   /* InARP reply          */
#define OAL_ARPOP_NAK       ARPOP_NAK       /* (ATM)ARP NAK         */

#define OAL_IPPROTO_UDP    IPPROTO_UDP    /* User Datagram Protocot */
#define OAL_IPPROTO_TCP    IPPROTO_TCP    /* Transmission Control Protocol */
#define OAL_IPPROTO_ICMPV6 IPPROTO_ICMPV6 /* ICMPv6 */

#define OAL_IEEE80211_MAX_SSID_LEN   32 /* 最大SSID长度 */
#define OAL_IEEE80211_EXTRA_SSID_LEN 4  /* 因上层逻辑需要增加的额外SSID长度 */
#define OAL_INIT_NET                 init_net
#define OAL_THIS_MODULE              THIS_MODULE
#define OAL_MSG_DONTWAIT             MSG_DONTWAIT

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34)
#define OAL_NL80211_MAX_NR_CIPHER_SUITES NL80211_MAX_NR_CIPHER_SUITES
#define OAL_NL80211_MAX_NR_AKM_SUITES    NL80211_MAX_NR_AKM_SUITES
#else
#define OAL_NL80211_MAX_NR_CIPHER_SUITES 5
#define OAL_NL80211_MAX_NR_AKM_SUITES    2
#define OAL_NL80211_MAX_NR_ARRY_LEN      3
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
#define oal_wdev_match(_netif, _req) (_netif->ieee80211_ptr == _req->wdev)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
#define oal_wdev_match(_netif, _req) (_netif == _req->dev)
#else
#define oal_wdev_match(_netif, _req) ((_netif)->ifindex == (_req)->ifidx)
#endif
/* 枚举定义 */
typedef gfp_t oal_gfp_enum_uint8;

#define OAL_NETDEV_TX_OK     NETDEV_TX_OK
#define OAL_NETDEV_TX_BUSY   NETDEV_TX_BUSY
#define OAL_NETDEV_TX_LOCKED NETDEV_TX_LOCKED

/*
 * oal_is_broadcast_ether_addr - Determine if the Ethernet address is broadcast
 * @addr: Pointer to a six-byte array containing the Ethernet address
 * Return true if the address is the broadcast address.
 */
#define oal_is_broadcast_ether_addr(addr) is_broadcast_ether_addr(addr)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
typedef netdev_tx_t oal_net_dev_tx_enum;
#else
typedef int32_t oal_net_dev_tx_enum;
#endif
/* 由于02 device是裸系统，需要使用uint8，所有02用uint8类型 51不改变 */
typedef uint8_t oal_nl80211_auth_type_enum_uint8;
typedef struct oal_cpu_usage_stat {
    uint64_t ull_user;
    uint64_t ull_nice;
    uint64_t ull_system;
    uint64_t ull_softirq;
    uint64_t ull_irq;
    uint64_t ull_idle;
    uint64_t ull_iowait;
    uint64_t ull_steal;
    uint64_t ull_guest;
} oal_cpu_usage_stat_stru;

struct oal_ether_header {
    uint8_t auc_ether_dhost[ETHER_ADDR_LEN];
    uint8_t auc_ether_shost[ETHER_ADDR_LEN];
    uint16_t us_ether_type;
} __OAL_DECLARE_PACKED;
typedef struct oal_ether_header oal_ether_header_stru;

/* linux 结构体 */
typedef struct sk_buff oal_netbuf_stru;
typedef struct sk_buff_head oal_netbuf_head_stru;
typedef struct net_device oal_net_device_stru;
typedef struct net_device_ops oal_net_device_ops_stru;
typedef struct ethtool_ops oal_ethtool_ops_stru;
typedef struct iw_priv_args oal_iw_priv_args_stru;
typedef struct iw_handler_def oal_iw_handler_def_stru;
typedef struct iw_point oal_iw_point_stru;
typedef struct iw_param oal_iw_param_stru;
typedef struct iw_freq oal_iw_freq_stru;
typedef struct iw_request_info oal_iw_request_info_stru;
typedef struct rtnl_link_ops oal_rtnl_link_ops_stru;
typedef struct sockaddr oal_sockaddr_stru;
typedef struct net_device_stats oal_net_device_stats_stru;
typedef struct ifreq oal_ifreq_stru;
typedef struct Qdisc oal_qdisc_stru;
typedef struct vlan_ethhdr oal_vlan_ethhdr_stru;
typedef struct wiphy oal_wiphy_stru;
typedef struct wireless_dev oal_wireless_dev_stru;
typedef struct cfg80211_ops oal_cfg80211_ops_stru;
typedef struct wiphy_vendor_command oal_wiphy_vendor_command_stru;
typedef struct nl80211_vendor_cmd_info oal_nl80211_vendor_cmd_info_stru;

typedef struct kobj_uevent_env oal_kobj_uevent_env_stru;
typedef struct iw_quality oal_iw_quality_stru;
typedef union iwreq_data oal_iwreq_data_union;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
typedef struct cfg80211_pmksa oal_cfg80211_pmksa_stru;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
typedef struct survey_info oal_survey_info_stru;
#endif

typedef struct key_params oal_key_params_stru;

typedef struct cfg80211_scan_request oal_cfg80211_scan_request_stru;
typedef struct cfg80211_ssid oal_cfg80211_ssid_stru;
typedef struct cfg80211_sched_scan_request oal_cfg80211_sched_scan_request_stru;
/* linux-2.6.34内核才有以下两个结构体，加密相关 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
typedef enum nl80211_mfp oal_nl80211_mfp_enum_uint8;
typedef struct cfg80211_connect_params oal_cfg80211_conn_stru;
typedef struct cfg80211_crypto_settings oal_cfg80211_crypto_settings_stru;
#else
/* linux-2.6.30内核没有这两个加密相关的结构体，命令下发上报也是内核透传，这里打桩 */
typedef struct cfg80211_crypto_settings {
    uint32_t wpa_versions;
    uint32_t cipher_group;
    int32_t n_ciphers_pairwise;
    uint32_t ciphers_pairwise[OAL_NL80211_MAX_NR_CIPHER_SUITES];
    int32_t n_akm_suites;
    uint32_t akm_suites[OAL_NL80211_MAX_NR_AKM_SUITES];

    oal_bool_enum_uint8 control_port;
    uint8_t auc_arry[OAL_NL80211_MAX_NR_ARRY_LEN];
} oal_cfg80211_crypto_settings_stru;

#endif

typedef struct work_struct oal_work_struct_stru;
typedef struct ieee80211_mgmt oal_ieee80211_mgmt_stru;
typedef struct ieee80211_channel oal_ieee80211_channel_stru;
typedef struct cfg80211_bss oal_cfg80211_bss_stru;
typedef struct rate_info oal_rate_info_stru;
typedef struct station_info oal_station_info_stru;
typedef struct station_parameters oal_station_parameters_stru;
typedef enum station_info_flags oal_station_info_flags;

typedef struct nlattr oal_nlattr_stru;
typedef struct genl_family oal_genl_family_stru;
typedef struct genl_multicast_group oal_genl_multicast_group_stru;
typedef struct cfg80211_registered_device oal_cfg80211_registered_device_stru;

typedef struct ieee80211_rate oal_ieee80211_rate;
typedef struct ieee80211_channel oal_ieee80211_channel;
typedef struct ieee80211_supported_band oal_ieee80211_supported_band;
typedef enum cfg80211_signal_type oal_cfg80211_signal_type;
typedef enum nl80211_channel_type oal_nl80211_channel_type;

typedef enum nl80211_chan_width oal_nl80211_chan_width;
typedef struct cfg80211_chan_def oal_cfg80211_chan_def;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
/* do nothing */
#else
enum nl80211_external_auth_action {
    NL80211_EXTERNAL_AUTH_START,
    NL80211_EXTERNAL_AUTH_ABORT,
};
#endif

typedef enum nl80211_external_auth_action oal_nl80211_external_auth_action;

typedef enum wiphy_params_flags oal_wiphy_params_flags;
typedef enum wiphy_flags oal_wiphy_flags;
typedef struct vif_params oal_vif_params_stru;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
typedef enum nl80211_key_type oal_nl80211_key_type;
#else
enum nl80211_key_type {
    NL80211_KEYTYPE_GROUP,
    NL80211_KEYTYPE_PAIRWISE,
    NL80211_KEYTYPE_PEERKEY,
    NUM_NL80211_KEYTYPES
};
typedef enum nl80211_key_type oal_nl80211_key_type;
#endif

typedef struct ieee80211_sta_ht_cap oal_ieee80211_sta_ht_cap;
typedef struct ieee80211_regdomain oal_ieee80211_regdomain_stru;
typedef struct cfg80211_update_ft_ies_params oal_cfg80211_update_ft_ies_stru;
typedef struct cfg80211_ft_event_params oal_cfg80211_ft_event_stru;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
/* To be implement! */
typedef struct cfg80211_beacon_data oal_beacon_data_stru;
typedef struct cfg80211_ap_settings oal_ap_settings_stru;
typedef struct bss_parameters oal_bss_parameters;
struct beacon_parameters {
    uint8_t *head, *tail;
    int32_t interval, dtim_period;
    int32_t head_len, tail_len;
};
typedef struct beacon_parameters oal_beacon_parameters;
#else
typedef struct beacon_parameters oal_beacon_parameters;
#endif
typedef struct ieee80211_channel_sw_ie oal_ieee80211_channel_sw_ie;
typedef struct ieee80211_msrment_ie oal_ieee80211_msrment_ie;
typedef struct ieee80211_mgmt oal_ieee80211_mgmt;

typedef struct sock oal_sock_stru;
typedef struct net oal_net_stru;
typedef struct module oal_module_stru;
typedef struct nlmsghdr oal_nlmsghdr_stru;

typedef struct ethhdr oal_ethhdr;
typedef struct nf_hook_ops oal_nf_hook_ops_stru;
typedef struct net_bridge_port oal_net_bridge_port;

typedef struct ipv6hdr oal_ipv6hdr_stru;
typedef struct icmp6hdr oal_icmp6hdr_stru;
typedef struct in6_addr oal_in6_addr;
typedef struct nd_msg oal_nd_msg_stru;
typedef struct nd_opt_hdr oal_nd_opt_hdr;
typedef struct netlink_skb_parms oal_netlink_skb_parms;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
/* linux 3.10.44 kernel not defined */
#define ll_allocated_space(dev) \
    ((((dev)->hard_header_len + (dev)->needed_headroom + (dev)->needed_tailroom) & ~(15)) + 16)
#endif

#define OAL_LL_ALLOCATED_SPACE ll_allocated_space

/* netlink相关 */
#define OAL_NLMSG_ALIGNTO             NLMSG_ALIGNTO
#define oal_nlmsg_align(_len)         NLMSG_ALIGN(_len)
#define OAL_NLMSG_HDRLEN              NLMSG_HDRLEN
#define oal_nlmsg_length(_len)        NLMSG_LENGTH(_len)
#define oal_nlmsg_space(_len)         NLMSG_SPACE(_len)
#define oal_nlmsg_data(_nlh)          NLMSG_DATA(_nlh)
#define oal_nlmsg_payload(_nlh, _len) NLMSG_PAYLOAD(_nlh, _len)
#define oal_nlmsg_ok(_nlh, _len)      NLMSG_OK(_nlh, _len)
#define oal_netlink_cb(_skb)          NETLINK_CB(_skb)

#define oal_nla_for_each_attr(pos, head, len, rem) nla_for_each_attr(pos, head, len, rem)
#define oal_nla_for_each_nested(pos, nla, rem) nla_for_each_nested(pos, nla, rem)

#define OAL_AF_BRIDGE AF_BRIDGE /* Multiprotocol bridge     */
#define OAL_PF_BRIDGE OAL_AF_BRIDGE

/* Bridge Hooks */
/* After promisc drops, checksum checks. */
#define OAL_NF_BR_PRE_ROUTING NF_BR_PRE_ROUTING
/* If the packet is destined for this box. */
#define OAL_NF_BR_LOCAL_IN NF_BR_LOCAL_IN
/* If the packet is destined for another interface. */
#define OAL_NF_BR_FORWARD NF_BR_FORWARD
/* Packets coming from a local process. */
#define OAL_NF_BR_LOCAL_OUT NF_BR_LOCAL_OUT
/* Packets about to hit the wire. */
#define OAL_NF_BR_POST_ROUTING NF_BR_POST_ROUTING
/* Not really a hook, but used for the ebtables broute table */
#define OAL_NF_BR_BROUTING NF_BR_BROUTING
#define OAL_NF_BR_NUMHOOKS NF_BR_NUMHOOKS

/* Responses from hook functions. */
#define OAL_NF_DROP        NF_DROP
#define OAL_NF_ACCEPT      NF_ACCEPT
#define OAL_NF_STOLEN      NF_STOLEN
#define OAL_NF_QUEUE       NF_QUEUE
#define OAL_NF_REPEAT      NF_REPEAT
#define OAL_NF_STOP        NF_STOP
#define OAL_NF_MAX_VERDICT NF_STOP

typedef struct {
    uint16_t us_ar_hrd; /* format of hardware address */
    uint16_t us_ar_pro; /* format of protocol address */

    uint8_t uc_ar_hln; /* length of hardware address */
    uint8_t uc_ar_pln; /* length of protocol address */
    uint16_t us_ar_op; /* ARP opcode (command) */

    uint8_t auc_ar_sha[ETHER_ADDR_LEN];         /* sender hardware address */
    uint8_t auc_ar_sip[ETH_SENDER_IP_ADDR_LEN]; /* sender IP address */
    uint8_t auc_ar_tha[ETHER_ADDR_LEN];         /* target hardware address */
    uint8_t auc_ar_tip[ETH_TARGET_IP_ADDR_LEN]; /* target IP address */
} oal_eth_arphdr_stru;

typedef struct ieee80211_iface_limit oal_ieee80211_iface_limit;
typedef struct ieee80211_iface_combination oal_ieee80211_iface_combination;

typedef struct {
    uint8_t uc_napi_enable; /* NAPI使能 */
    uint8_t uc_gro_enable;  /* GRO使能 */
    uint8_t uc_napi_weight; /* 每次调度NAPI处理数据包个数限制 */
    uint8_t uc_state;
    struct napi_struct st_napi;
    oal_netbuf_head_stru st_rx_netbuf_queue;
    uint32_t queue_len_max;
    uint32_t period_pkts;  /* 统计周期pps,以便调整napi参数 */
    uint32_t period_start; /* 统计周期起始时间戳 */
} oal_netdev_priv_stru;

/* UNION定义 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
#define HISI_IEEE80211_BAND_2GHZ NL80211_BAND_2GHZ
#define HISI_IEEE80211_BAND_5GHZ NL80211_BAND_5GHZ
#else
#define HISI_IEEE80211_BAND_2GHZ IEEE80211_BAND_2GHZ
#define HISI_IEEE80211_BAND_5GHZ IEEE80211_BAND_5GHZ
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4,7,0)) */

/* 函数声明 */
OAL_STATIC OAL_INLINE void oal_skb_checksum_help(oal_netbuf_stru *netbuf)
{
    skb_checksum_help(netbuf);
}

OAL_STATIC OAL_INLINE void oal_netbuf_copy_queue_mapping(oal_netbuf_stru *to, const oal_netbuf_stru *from)
{
    skb_copy_queue_mapping(to, from);
}

/*
 * 函 数 名  : oal_netbuf_put
 * 功能描述  : 在缓冲区尾部增加数据
 * 输入参数  : pst_netbuf: 缓冲区结构体指针
 *             ul_len: 需要增加数据的长度
 */
OAL_STATIC OAL_INLINE uint8_t *oal_netbuf_put(oal_netbuf_stru *pst_netbuf, uint32_t ul_len)
{
    return skb_put(pst_netbuf, ul_len);
}

/*
 * 函 数 名  : oal_netbuf_push
 * 功能描述  : 在缓冲区开头增加数据
 */
OAL_STATIC OAL_INLINE uint8_t *oal_netbuf_push(oal_netbuf_stru *pst_netbuf, uint32_t ul_len)
{
    return skb_push(pst_netbuf, ul_len);
}

/*
 * 函 数 名  : oal_netbuf_pull
 * 功能描述  : 从skb头部取出数据
 */
OAL_STATIC OAL_INLINE uint8_t *oal_netbuf_pull(oal_netbuf_stru *pst_netbuf, uint32_t ul_len)
{
    if (oal_unlikely(pst_netbuf == NULL)) {
        oal_warn_on(1);
        return NULL;
    }

    if (ul_len > pst_netbuf->len) {
        return NULL;
    }

    pst_netbuf->len -= ul_len;

    return (pst_netbuf->data += ul_len);
}
/*
 * 函 数 名  : oal_ieee80211_channel_to_frequency
 * 功能描述  : 将信道转换成频率
 * 输入参数  : int32_t l_channel      :信道号
 *             enum ieee80211_band band :频段
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
OAL_STATIC OAL_INLINE int32_t oal_ieee80211_channel_to_frequency(int32_t l_channel, enum nl80211_band band)
#else
OAL_STATIC OAL_INLINE int32_t oal_ieee80211_channel_to_frequency(int32_t l_channel, enum ieee80211_band band)
#endif
{
    /* see 802.11 17.3.8.3.2 and Annex J there are overlapping channel numbers in 5GHz and 2GHz bands */
    if (l_channel <= 0) {
        return 0; /* not supported */
    }

    /*
     *                                wifi信道和频率的对应关系
     *        2G频段                5G频段               5G频段             5G频段
     *    信道     频率(MHz)   信道     频率(MHz)   信道     频率(MHz)   信道     频率(MHz)
     *      1       2412       183        4915        7        5035       64        5320
     *      2       2417       184        4920        8        5040      100        5500
     *      3       2422       185        4925        9        5045      104        5520
     *      4       2427       187        4935       11        5055      108        5540
     *      5       2432       188        4940       12        5060      112        5560
     *      6       2437       189        4945       16        5080      116        5580
     *      7       2442       192        4960       34        5170      120        5600
     *      8       2447       196        4980       36        5180      124        5620
     *      9       2452                             38        5190      128        5640
     *     10       2457                             40        5200      132        5660
     *     11       2462                             42        5210      136        5680
     *     12       2467                             44        5220      140        5700
     *     13       2472                             46        5230      149        5745
     *     14       2484                             48        5240      153        5765
     *                                               52        5260      157        5785
     *                                               56        5280      161        5805
     *                                               60        5300      165        5825
     */
    switch (band) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
        case NL80211_BAND_2GHZ:
#else
        case IEEE80211_BAND_2GHZ:
#endif
        {
            if (l_channel == 14) { // 2G频段下信道号14对应的频率为2484
                return 2484;
            } else if (l_channel < 14) { // 小于14的信道号下对应的频率 = 基数2407 + 信道 * 5
                return 2407 + l_channel * 5;
            }
            break;
        }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
        case NL80211_BAND_5GHZ:
#else
        case IEEE80211_BAND_5GHZ:
#endif
        {
            if (l_channel >= 182 && l_channel <= 196) { // 5G频段下信道在[182, 196]之间的频率 = 基数4000 + 信道号 * 5
                return 4000 + l_channel * 5;
            } else {
                return 5000 + l_channel * 5; // 5G频段下信道小于182对应的频率 = 基数5000 + 信道号 * 5
            }
        }
        default:
            /* not supported other BAND */
            return 0;
    }

    /* not supported */
    return 0;
}

OAL_STATIC OAL_INLINE int32_t oal_ieee80211_frequency_to_channel(int32_t center_freq)
{
    int32_t channel;

    /* see 802.11 17.3.8.3.2 and Annex J */
    if (center_freq == 0) { // 频率为0，对应的信道号为0
        channel = 0;
    } else if (center_freq == 2484) { // 频率为2484对应信道号为14
        channel = 14;
    } else if (center_freq < 2484) { // 频率小于2484对应的信道号 = （频率 - 2407）/ 5;
        channel = (center_freq - 2407) / 5;
    } else if (center_freq >= 4910 && center_freq <= 4980) {  // 频率在[4910, 4980]对应的信道号 =(频率 - 4000)/ 5;
        channel = (center_freq - 4000) / 5;
    } else if (center_freq >= 5180 && center_freq <= 5825) {  // 频率在[5180, 5825]对应的信道号 =(频率 - 5000)/ 5;
        channel = (center_freq - 5000) / 5;
    } else if (center_freq >= 5955 && center_freq <= 7115) {  // 频率在[5955, 7115]对应的信道号 =（频率 - 5950) / 5;
        channel = (center_freq - 5950) / 5;
    } else if (center_freq >= 58320 && center_freq <= 64800) { // 频率在[58320, 64800]对应的信道号 =(频率 - 5616)/ 2160;
        channel = (center_freq - 56160) / 2160;
    } else { // 频率大于64800对应信道号为0
        channel = 0;
    }
    return channel;
}

/*
 * 函 数 名  : oal_netbuf_get_bitfield
 * 功能描述  : 识别IP报文的大小端
 */
OAL_STATIC OAL_INLINE uint8_t oal_netbuf_get_bitfield(void)
{
    union bitfield {
        uint8_t uc_byte;
        struct {
            uint8_t high : 4,
                      low : 4;
        } bits;
    } un_bitfield;

    un_bitfield.uc_byte = 0x12;
    if (un_bitfield.bits.low == 0x2) {
        return OAL_BITFIELD_LITTLE_ENDIAN;
    } else {
        return OAL_BITFIELD_BIG_ENDIAN;
    }
}

OAL_STATIC OAL_INLINE void oal_set_netbuf_prev(oal_netbuf_stru *pst_buf, oal_netbuf_stru *pst_prev)
{
    if (oal_unlikely(pst_buf == NULL)) {
        oal_warn_on(1);
        return;
    }
    pst_buf->prev = pst_prev;
}

OAL_STATIC OAL_INLINE oal_netbuf_stru *oal_get_netbuf_prev(oal_netbuf_stru *pst_buf)
{
    return ((pst_buf != NULL) ? pst_buf->prev : NULL);
}

/* arm64 尾指针变成了偏移 */
#ifndef CONFIG_ARM64
/* tail指针操作请使用skb_put，避免使用该函数 */
OAL_STATIC OAL_INLINE void oal_set_netbuf_tail(oal_netbuf_stru *pst_buf, uint8_t *tail)
{
    if (oal_unlikely(pst_buf == NULL)) {
        oal_warn_on(1);
        return;
    }
    pst_buf->tail = tail;
}
#endif

OAL_STATIC OAL_INLINE void oal_set_netbuf_next(oal_netbuf_stru *pst_buf, oal_netbuf_stru *next)
{
    if (pst_buf == NULL) {
        return;
    } else {
        pst_buf->next = next;
    }
}

OAL_STATIC OAL_INLINE oal_netbuf_stru *oal_get_netbuf_next(oal_netbuf_stru *pst_buf)
{
    return ((pst_buf != NULL) ? pst_buf->next : NULL);
}

/* arm64 尾指针变成了偏移 */
#ifndef CONFIG_ARM64
/* tail指针操作请使用skb_put，避免使用该函数 */
OAL_STATIC OAL_INLINE void oal_set_single_netbuf_tail(oal_netbuf_stru *pst_netbuf, uint8_t *puc_tail)
{
    if (oal_unlikely(pst_netbuf == NULL)) {
        oal_warn_on(1);
        return;
    }
    pst_netbuf->tail = puc_tail;
}
#endif

OAL_STATIC OAL_INLINE oal_ieee80211_channel_stru *oal_ieee80211_get_channel(oal_wiphy_stru *pst_wiphy,
                                                                            int32_t ul_freq)
{
    return ieee80211_get_channel(pst_wiphy, ul_freq);
}

/* BEGIN : Linux wiphy 结构相关的处理函数 */
OAL_STATIC OAL_INLINE oal_wiphy_stru *oal_wiphy_new(oal_cfg80211_ops_stru *ops, int32_t sizeof_priv)
{
    return wiphy_new(ops, sizeof_priv);
}

OAL_STATIC OAL_INLINE int32_t oal_wiphy_register(oal_wiphy_stru *pst_wiphy)
{
    return wiphy_register(pst_wiphy);
}

OAL_STATIC OAL_INLINE void oal_wiphy_unregister(oal_wiphy_stru *pst_wiphy)
{
    return wiphy_unregister(pst_wiphy);
}

OAL_STATIC OAL_INLINE void oal_wiphy_free(oal_wiphy_stru *pst_wiphy)
{
    wiphy_free(pst_wiphy);
}

OAL_STATIC OAL_INLINE void *oal_wiphy_priv(oal_wiphy_stru *pst_wiphy)
{
    return wiphy_priv(pst_wiphy);
}


OAL_STATIC OAL_INLINE void oal_wiphy_apply_custom_regulatory(oal_wiphy_stru *pst_wiphy,
                                                             const oal_ieee80211_regdomain_stru *regd)
{
    wiphy_apply_custom_regulatory(pst_wiphy, regd);
}

/* END : Linux wiphy 结构相关的处理函数 */
/*
 * 函 数 名  : oal_eth_type_trans
 * 功能描述  : 获取协议模式
 * 输入参数  : pst_netbuf: skb指针
 *             pst_device: net device结构体指针
 */
OAL_STATIC OAL_INLINE uint16_t oal_eth_type_trans(oal_netbuf_stru *pst_netbuf, oal_net_device_stru *pst_device)
{
    return eth_type_trans(pst_netbuf, pst_device);
}

/*
 * 函 数 名  : oal_ether_setup
 * 功能描述  : 网络设备启动接口
 * 输入参数  : oal_net_device_stru:网络设备引用
 */
OAL_STATIC OAL_INLINE void oal_ether_setup(oal_net_device_stru *p_net_device)
{
    if (p_net_device == NULL) {
        return;
    }

    ether_setup(p_net_device);

    return;
}

/*
 * 函 数 名  : oal_dev_get_by_name
 * 功能描述  : 根据名字寻找netdevice
 */
OAL_STATIC OAL_INLINE oal_net_device_stru *oal_dev_get_by_name(const char *pc_name)
{
    return dev_get_by_name(&init_net, pc_name);
}

/*
 * 函 数 名  : oal_dev_put
 * 功能描述  : 调用oal_dev_get_by_name后需要调用dev_put,是net_dev的引用计数减1
 * 输入参数  : _pst_dev: 指向net_dev的指针
 */
#define oal_dev_put(_pst_dev) dev_put(_pst_dev)

/*
 * 函 数 名  : oal_net_close_dev
 * 功能描述  : down调 netdev
 */
OAL_STATIC OAL_INLINE void oal_net_close_dev(oal_net_device_stru *pst_netdev)
{
    rtnl_lock();
    dev_close(pst_netdev);
    rtnl_unlock();
}

/*
 * 函 数 名  : oal_net_alloc_netdev
 * 功能描述  : 分配网络设备
 * 输入参数  : ul_sizeof_priv: 私有结构空间长度
 *           : puc_name 设备名称
 *           : p_set_up:启动函数指针
 */
OAL_STATIC OAL_INLINE oal_net_device_stru *oal_net_alloc_netdev(uint32_t ul_sizeof_priv, const char *puc_name,
                                                                void *p_set_up)
{
    if (oal_any_null_ptr2(puc_name, p_set_up)) {
        return NULL;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    return alloc_netdev(ul_sizeof_priv, puc_name, NET_NAME_UNKNOWN, p_set_up);
#else
    return alloc_netdev(ul_sizeof_priv, puc_name, p_set_up);
#endif
}

/*
 * 函 数 名  : oal_net_alloc_netdev_mqs
 * 功能描述  : 分配网络设备,包含多个队列
 * 输入参数  : ul_sizeof_priv: 私有结构空间长度
 *           : puc_name 设备名称
 *           : p_set_up:启动函数指针
 */
OAL_STATIC OAL_INLINE oal_net_device_stru *oal_net_alloc_netdev_mqs(uint32_t ul_sizeof_priv, const char *puc_name,
                                                                    void *p_set_up, uint32_t ul_txqs,
                                                                    uint32_t ul_rxqs)
{
    if (oal_any_null_ptr2(puc_name, p_set_up)) {
        return NULL;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    return alloc_netdev_mq(ul_sizeof_priv, puc_name, NET_NAME_UNKNOWN, p_set_up, ul_txqs);
#else
    return alloc_netdev_mq(ul_sizeof_priv, puc_name, p_set_up, ul_txqs);
#endif
}

OAL_STATIC OAL_INLINE void oal_net_tx_wake_all_queues(oal_net_device_stru *pst_dev)
{
    if (pst_dev == NULL) {
        return;
    }

    return netif_tx_wake_all_queues(pst_dev);
}

OAL_STATIC OAL_INLINE void oal_net_tx_stop_all_queues(oal_net_device_stru *pst_dev)
{
    if (pst_dev == NULL) {
        return;
    }

    return netif_tx_stop_all_queues(pst_dev);
}

/*
 * 函 数 名  : oal_net_wake_subqueue
 * 功能描述  : wake网络设备的某个subqueue
 */
OAL_STATIC OAL_INLINE void oal_net_wake_subqueue(oal_net_device_stru *pst_dev, uint16_t us_queue_idx)
{
    if (pst_dev == NULL) {
        return;
    }

    return netif_wake_subqueue(pst_dev, us_queue_idx);
}

/*
 * 函 数 名  : oal_net_stop_subqueue
 * 功能描述  : 暂停网络设备的某个subqueue
 */
OAL_STATIC OAL_INLINE void oal_net_stop_subqueue(oal_net_device_stru *pst_dev, uint16_t us_queue_idx)
{
    if (pst_dev == NULL) {
        return;
    }

    return netif_stop_subqueue(pst_dev, us_queue_idx);
}

/*
 * 函 数 名  : oal_net_free_netdev
 * 功能描述  : 释放网络设备
 * 输入参数  : ul_sizeof_priv: 私有结构空间长度
 *           : puc_name 设备名称
 *           : p_set_up:启动函数指针
 */
OAL_STATIC OAL_INLINE void oal_net_free_netdev(oal_net_device_stru *pst_netdev)
{
    if (pst_netdev == NULL) {
        return;
    }

    free_netdev(pst_netdev);
}

/*
 * 函 数 名  : oal_net_register_netdev
 * 功能描述  : 注册网络设备
 */
OAL_STATIC OAL_INLINE int32_t oal_net_register_netdev(oal_net_device_stru *p_net_device)
{
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    const uint16_t us_needed_headroom_size = 76;
#else
    const uint16_t us_needed_headroom_size = 64;
#endif
    const uint16_t us_needed_tailroom_size = 32;

    if (p_net_device == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    /* 扩展ETHER HEAD并且4bytes对齐 多预留4字节空间 */
    oal_netdevice_headroom(p_net_device) = us_needed_headroom_size;
#else
    oal_netdevice_headroom(p_net_device) = us_needed_headroom_size;
#endif
    oal_netdevice_tailroom(p_net_device) = us_needed_tailroom_size;

    return register_netdev(p_net_device);
}

/*
 * 函 数 名  : oal_net_unregister_netdev
 * 功能描述  : 去注册网络设备
 */
OAL_STATIC OAL_INLINE void oal_net_unregister_netdev(oal_net_device_stru *p_net_device)
{
    if (p_net_device == NULL) {
        return;
    }

    unregister_netdev(p_net_device);
}

/*
 * 函 数 名  : oal_net_device_priv
 * 功能描述  : 返回netdev的私有成员
 */
OAL_STATIC OAL_INLINE void *oal_net_device_priv(oal_net_device_stru *pst_net_dev)
{
    if (pst_net_dev == NULL) {
        return NULL;
    }
    return netdev_priv(pst_net_dev);
}

OAL_STATIC OAL_INLINE int32_t oal_net_device_open(oal_net_device_stru *pst_dev)
{
    if (oal_unlikely(pst_dev == NULL)) {
        oal_warn_on(1);
        return OAL_FAIL;
    }

    pst_dev->flags |= OAL_IFF_RUNNING;

    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE int32_t oal_net_device_close(oal_net_device_stru *pst_dev)
{
    if (oal_unlikely(pst_dev == NULL)) {
        oal_warn_on(1);
        return OAL_FAIL;
    }

    pst_dev->flags &= ~OAL_IFF_RUNNING;

    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE int32_t oal_net_device_set_macaddr(oal_net_device_stru *pst_dev, void *pst_addr)
{
    oal_sockaddr_stru *pst_mac = NULL;
    /* sa_data成员是总大小为14字节的协议地址buff */
    const uint32_t ul_protocol_addr_len = 6;
    const uint32_t ul_net_device_mac_addr_len = 6;

    if (oal_unlikely((pst_dev == NULL) || (pst_addr == NULL))) {
        oal_warn_on(1);
        return OAL_FAIL;
    }

    pst_mac = (oal_sockaddr_stru *)pst_addr;

    memcpy_s(pst_dev->dev_addr, ul_net_device_mac_addr_len, pst_mac->sa_data, ul_protocol_addr_len);

    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE int32_t oal_net_device_init(oal_net_device_stru *pst_dev)
{
    return OAL_SUCC;
}

/*
 * 函 数 名  : oal_net_device_get_stats
 * 功能描述  : net device的统计函数
 * 输入参数  : net device指针
 * 返 回 值  : 统计结果指针
 */
OAL_STATIC OAL_INLINE oal_net_device_stats_stru *oal_net_device_get_stats(oal_net_device_stru *pst_dev)
{
    oal_net_device_stats_stru *pst_stats = NULL;

    if (oal_unlikely(pst_dev == NULL)) {
        oal_warn_on(1);
        return NULL;
    }

    pst_stats = &pst_dev->stats;

    pst_stats->tx_errors = 0;
    pst_stats->tx_dropped = 0;
    pst_stats->tx_packets = 0;
    pst_stats->rx_packets = 0;
    pst_stats->rx_errors = 0;
    pst_stats->rx_dropped = 0;
    pst_stats->rx_crc_errors = 0;

    return pst_stats;
}

/*
 * 函 数 名  : oal_net_device_ioctl
 * 功能描述  : net device的ioctl函数
 */
OAL_STATIC OAL_INLINE int32_t oal_net_device_ioctl(oal_net_device_stru *pst_dev, oal_ifreq_stru *pst_ifr,
                                                   int32_t ul_cmd)
{
    return -OAL_EINVAL;
}

OAL_STATIC OAL_INLINE int32_t oal_net_device_multicast_list(oal_net_device_stru *pst_dev)
{
    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE int32_t oal_net_device_change_mtu(oal_net_device_stru *pst_dev, int32_t l_mtu)
{
    if (oal_unlikely(pst_dev == NULL)) {
        oal_warn_on(1);
        return OAL_FAIL;
    }
    pst_dev->mtu = (uint32_t)l_mtu;
    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE int32_t oal_net_device_hardstart(oal_netbuf_stru *pst_skb, oal_net_device_stru *pst_dev)
{
    return OAL_SUCC;
}

/* 在dev.c中定义，用来在中断上下文或者非中断上下文中释放skb */
extern void dev_kfree_skb_any(struct sk_buff *skb);

/*
 * 函 数 名  : oal_netbuf_reserve
 * 功能描述  : 将报文结构体的data指针和tail指针同时下移
 * 输入参数  : pst_netbuf报文结构体指针
 *             l_len: 预留长度
 */
OAL_STATIC OAL_INLINE void oal_netbuf_reserve(oal_netbuf_stru *pst_netbuf, int32_t l_len)
{
    if (oal_unlikely(pst_netbuf == NULL)) {
        oal_warn_on(1);
        return;
    }
    skb_reserve(pst_netbuf, l_len);
}

/*
 * 函 数 名  : _oal_netbuf_alloc
 * 功能描述  : 为netbuf申请内存
 * 输入参数  : ul_size: 分配内存的大小
 *             l_reserve: data跟指针头之间要预留的长度
 *             l_align: 需要几字节对齐
 */
OAL_STATIC OAL_INLINE oal_netbuf_stru *_oal_netbuf_alloc(uint32_t ul_size, int32_t l_reserve, uint32_t l_align)
{
    oal_netbuf_stru *pst_netbuf = NULL;
    uint32_t ul_offset;

    /* 保证data部分的size不会再字节对齐后小于预先想分配的大小 */
    if (l_align) {
        ul_size += (l_align - 1);
    }

    // do not use dev_alloc_skb which designed for RX
    pst_netbuf = dev_alloc_skb(ul_size);
    if (oal_unlikely(pst_netbuf == NULL)) {
        return NULL;
    }

    skb_reserve(pst_netbuf, l_reserve);

    if (l_align) {
        /* 计算为了能使4字节对齐的偏移量 */
        ul_offset = (int32_t)(((uintptr_t)pst_netbuf->data) % l_align);
        if (ul_offset) {
            skb_reserve (pst_netbuf, (int)(l_align - ul_offset));
        }
    }

    return pst_netbuf;
}


#ifdef _PRE_SKB_TRACE
#define oal_netbuf_alloc(size, l_reserve, l_align)                     \
    ({                                                                 \
        oal_netbuf_stru *__no_pst_netbuf;                              \
        __no_pst_netbuf = _oal_netbuf_alloc(size, l_reserve, l_align); \
        if (__no_pst_netbuf) {                                         \
            mem_trace_add_node ((uintptr_t)(__no_pst_netbuf));    \
        }                                                              \
        __no_pst_netbuf;                                               \
    })
#else
#define oal_netbuf_alloc(size, l_reserve, l_align) _oal_netbuf_alloc(size, l_reserve, l_align)
#endif

/*
 * 函 数 名  : oal_netbuf_free
 * 功能描述  : 释放报文结构体内存空间, 非硬中断环境下使用
 */
OAL_STATIC OAL_INLINE uint32_t _oal_netbuf_free(oal_netbuf_stru *pst_netbuf)
{
    if (oal_unlikely(pst_netbuf == NULL)) {
        oal_warn_on(1);
        return OAL_FAIL;
    }
    // 适配PCIE中断上下文释放内存
    dev_kfree_skb_any(pst_netbuf);

    return OAL_SUCC;
}

#ifdef _PRE_SKB_TRACE
#define oal_netbuf_free(pst_netbuf)                           \
    ({                                                        \
        mem_trace_delete_node ((uintptr_t)(pst_netbuf)); \
        _oal_netbuf_free(pst_netbuf);                         \
    })
#else
#define oal_netbuf_free(pst_netbuf) _oal_netbuf_free(pst_netbuf)
#endif

/*
 * 函 数 名  : oal_netbuf_free_any
 * 功能描述  : 释放报文结构体内存空间，可用于中断环境
 */
OAL_STATIC OAL_INLINE void _oal_netbuf_free_any(oal_netbuf_stru *pst_netbuf)
{
    if (oal_unlikely(pst_netbuf == NULL)) {
        oal_warn_on(1);
        return;
    }
    dev_kfree_skb_any(pst_netbuf);
}

#ifdef _PRE_SKB_TRACE
#define oal_netbuf_free_any(pst_netbuf)                       \
    ({                                                        \
        mem_trace_delete_node ((uintptr_t)(pst_netbuf)); \
        _oal_netbuf_free_any(pst_netbuf);                     \
    })
#else
#define oal_netbuf_free_any(pst_netbuf) _oal_netbuf_free_any(pst_netbuf)
#endif

/*
 * 函 数 名  : oal_netbuf_unshare
 * 功能描述  : 判断一个skb是否为克隆的，是则copy一份新的skb，否则直接返回传入的skb
 * 输入参数  : pst_netbuf: skb结构体指针
 *             en_pri: 内存分配的优先级
 */
OAL_STATIC OAL_INLINE oal_netbuf_stru *oal_netbuf_unshare(oal_netbuf_stru *pst_netbuf, oal_gfp_enum_uint8 en_pri)
{
    if (oal_unlikely(pst_netbuf == NULL)) {
        oal_warn_on(1);
        return NULL;
    }
    return skb_unshare(pst_netbuf, en_pri);
}

/*
 * 函 数 名  : _oal_netbuf_copy
 * 功能描述  : 拷贝一份新的skb
 */
OAL_STATIC OAL_INLINE oal_netbuf_stru *_oal_netbuf_copy(oal_netbuf_stru *pst_netbuf, oal_gfp_enum_uint8 en_priority)
{
    if (oal_unlikely(pst_netbuf == NULL)) {
        oal_warn_on(1);
        return NULL;
    }
    return skb_copy(pst_netbuf, en_priority);
}

#ifdef _PRE_SKB_TRACE
#define oal_netbuf_copy(pst_netbuf, en_priority)                  \
    ({                                                            \
        oal_netbuf_stru *__no_pst_copy_netbuf;                    \
        __no_pst_copy_netbuf = skb_copy(pst_netbuf, en_priority); \
        if (__no_pst_copy_netbuf) {                               \
            mem_trace_add_node((uintptr_t)__no_pst_copy_netbuf);  \
        }                                                         \
        __no_pst_copy_netbuf;                                     \
    })
#else
#define oal_netbuf_copy(pst_netbuf, en_priority) _oal_netbuf_copy(pst_netbuf, en_priority)
#endif

/*
 * 函 数 名  : oal_netbuf_data
 * 功能描述  : 获取skb数据头部
 */
OAL_STATIC OAL_INLINE uint8_t *oal_netbuf_data(oal_netbuf_stru *pst_netbuf)
{
    return ((pst_netbuf != NULL) ? pst_netbuf->data : NULL);
}

/*
 * 函 数 名  : oal_netbuf_header
 * 功能描述  : 获取skb数据头部
 */
OAL_STATIC OAL_INLINE uint8_t *oal_netbuf_header(oal_netbuf_stru *pst_netbuf)
{
    return ((pst_netbuf != NULL) ? pst_netbuf->data : NULL);
}

OAL_STATIC OAL_INLINE uint8_t *oal_netbuf_payload(oal_netbuf_stru *pst_netbuf)
{
    return ((pst_netbuf != NULL) ? pst_netbuf->data : NULL);
}

OAL_STATIC OAL_INLINE uint8_t *oal_netbuf_end(oal_netbuf_stru *pst_netbuf)
{
    return skb_end_pointer(pst_netbuf);
}

/*
 * 函 数 名  : oal_netbuf_get_len
 * 功能描述  : 获取skb数据长度
 */
OAL_STATIC OAL_INLINE uint32_t oal_netbuf_get_len(oal_netbuf_stru *pst_netbuf)
{
    return pst_netbuf->len;
}

OAL_STATIC OAL_INLINE uint32_t oal_netbuf_headroom(const oal_netbuf_stru *pst_netbuf)
{
    return skb_headroom(pst_netbuf);
}

/*
 * 函 数 名  : oal_netbuf_tailroom
 * 功能描述  : 获取尾部空间大小
 */
OAL_STATIC OAL_INLINE uint32_t oal_netbuf_tailroom(const oal_netbuf_stru *pst_netbuf)
{
    return skb_tailroom(pst_netbuf);
}

/*
 * 函 数 名  : oal_netbuf_realloc
 * 功能描述  : skb头部内存空间扩充
 * 输入参数  : pst_netbuf: skb结构体指针
 *             ul_headroom: 想要扩展的头部空间大小
 */
#ifdef _PRE_SKB_TRACE
#define oal_netbuf_realloc_headroom(pst_netbuf, ul_headroom)            \
    ({                                                                  \
        oal_netbuf_stru *__no_pst_skb = pst_netbuf;                     \
        if (pskb_expand_head(pst_netbuf, ul_headroom, 0, GFP_ATOMIC)) { \
            oal_netbuf_free(pst_netbuf);                                \
            __no_pst_skb = NULL;                                \
        }                                                               \
                                                                        \
        __no_pst_skb;                                                   \
    })
#else
OAL_STATIC OAL_INLINE oal_netbuf_stru *oal_netbuf_realloc_headroom(oal_netbuf_stru *pst_netbuf, uint32_t ul_headroom)
{
    if (pskb_expand_head(pst_netbuf, ul_headroom, 0, GFP_ATOMIC)) {
        oal_netbuf_free(pst_netbuf);
        pst_netbuf = NULL;
    }

    return pst_netbuf;
}
#endif

/*
 * 函 数 名  : oal_netbuf_realloc_tailroom
 * 功能描述  : skb尾部空间扩充
 */
#ifdef _PRE_SKB_TRACE
#define oal_netbuf_realloc_tailroom(pst_netbuf, ul_tailroom)                         \
    ({                                                                               \
        oal_netbuf_stru *__no_tmp_skb = NULL;                                \
        if (oal_likely(!pskb_expand_head(pst_netbuf, 0, ul_tailroom, GFP_ATOMIC))) { \
            __no_tmp_skb = pst_netbuf;                                               \
        } else {                                                                     \
            oal_netbuf_free_any(pst_netbuf);                                         \
        }                                                                            \
                                                                                     \
        __no_tmp_skb;                                                                \
    })
#else
OAL_STATIC OAL_INLINE oal_netbuf_stru *oal_netbuf_realloc_tailroom(oal_netbuf_stru *pst_netbuf, uint32_t ul_tailroom)
{
    if (oal_likely(!pskb_expand_head(pst_netbuf, 0, ul_tailroom, GFP_ATOMIC))) {
        return pst_netbuf;
    }

    /* unlikely path */
    oal_netbuf_free_any(pst_netbuf);

    return NULL;
}
#endif

/*
 * 函 数 名  : oal_netbuf_cb
 * 功能描述  : 返回skb中的cb字段
 */
OAL_STATIC OAL_INLINE uint8_t *oal_netbuf_cb(oal_netbuf_stru *pst_netbuf)
{
    return ((pst_netbuf != NULL) ? pst_netbuf->cb : NULL);
}

OAL_STATIC OAL_INLINE void oal_netbuf_add_to_list(oal_netbuf_stru *pst_buf, oal_netbuf_stru *pst_prev,
                                                  oal_netbuf_stru *pst_next)
{
    pst_buf->next = pst_next;
    pst_buf->prev = pst_prev;
    pst_next->prev = pst_buf;
    pst_prev->next = pst_buf;
}

OAL_STATIC OAL_INLINE void oal_netbuf_add_to_list_tail(oal_netbuf_stru *pst_buf, oal_netbuf_head_stru *pst_head)
{
    skb_queue_tail(pst_head, pst_buf);
}

OAL_STATIC OAL_INLINE uint32_t oal_netbuf_list_len(oal_netbuf_head_stru *pst_head)
{
    return skb_queue_len(pst_head);
}

/*
 * 函 数 名  : oal_netbuf_delete
 * 功能描述  : 删除链表中的skb
 */
OAL_STATIC OAL_INLINE void oal_netbuf_delete(oal_netbuf_stru *pst_buf, oal_netbuf_head_stru *pst_list_head)
{
    skb_unlink(pst_buf, pst_list_head);
}


/*
 * 函 数 名  : oal_spin_lock_bh_head
 * 功能描述  : 利用内核skb head原生锁
 * 输入参数  : pst_lock:自旋锁地址
 */
OAL_STATIC OAL_INLINE void oal_spin_lock_head_bh(oal_netbuf_head_stru *netbuf_head)
{
    spin_lock_bh(&netbuf_head->lock);
}

/*
 * 函 数 名  : oal_spin_unlock_bh_head
 * 功能描述  : Spinlock在软中断以及内核线程等核心态上下文环境下的解锁操作。利用内核skb head原生锁
 */
OAL_STATIC OAL_INLINE void oal_spin_unlock_head_bh(oal_netbuf_head_stru *netbuf_head)
{
    spin_unlock_bh(&netbuf_head->lock);
}


OAL_STATIC OAL_INLINE void __netbuf_unlink(struct sk_buff *skb, struct sk_buff_head *list)
{
    struct sk_buff *next = NULL;
    struct sk_buff *prev = NULL;

    list->qlen--;
    next = skb->next;
    prev = skb->prev;
    skb->next = skb->prev = NULL;
    next->prev = prev;
    prev->next = next;
}

/*
 * 函 数 名  : oal_netbuf_delist
 * 功能描述  : skb链表出队
 */
OAL_STATIC OAL_INLINE oal_netbuf_stru *oal_netbuf_delist(oal_netbuf_head_stru *pst_list_head)
{
    return skb_dequeue(pst_list_head);
}

/*
 * 函 数 名  : oal_netbuf_delist_nolock
 * 功能描述  : skb链表出队
 */
OAL_STATIC OAL_INLINE oal_netbuf_stru *oal_netbuf_delist_nolock(oal_netbuf_head_stru *pst_list_head)
{
    return __skb_dequeue(pst_list_head);
}

/*
 * 函 数 名  : oal_netbuf_addlist
 * 功能描述  : skb链表从头部入队
 */
OAL_STATIC OAL_INLINE void oal_netbuf_addlist(oal_netbuf_head_stru *pst_list_head,
                                              oal_netbuf_stru *netbuf)
{
    return skb_queue_head(pst_list_head, netbuf);
}

/*
 * 函 数 名  : oal_netbuf_list_purge
 * 功能描述  : skb链表清空
 */
OAL_STATIC OAL_INLINE void oal_netbuf_list_purge(oal_netbuf_head_stru *pst_list_head)
{
    skb_queue_purge(pst_list_head);
}

/*
 * 函 数 名  : oal_netbuf_list_empty
 * 功能描述  : 判断skb list是否为空
 */
OAL_STATIC OAL_INLINE int32_t oal_netbuf_list_empty(const oal_netbuf_head_stru *pst_list_head)
{
    return skb_queue_empty(pst_list_head);
}

/*
 * 函 数 名  : oal_netbuf_list_head_init
 * 功能描述  : 初始化skb队列头
 */
OAL_STATIC OAL_INLINE void oal_netbuf_list_head_init(oal_netbuf_head_stru *pst_list_head)
{
    skb_queue_head_init(pst_list_head);
}

/*
 * 函 数 名  : oal_netbuf_list_next
 * 功能描述  : 返回链表中指定节点的下一个节点
 */
OAL_STATIC OAL_INLINE oal_netbuf_stru *oal_netbuf_list_next(const oal_netbuf_stru *pst_buf)
{
    return pst_buf->next;
}

/*
 * 函 数 名  : oal_netbuf_list_tail
 * 功能描述  : add a netbuf to skb list
 */
OAL_STATIC OAL_INLINE void oal_netbuf_list_tail(oal_netbuf_head_stru *list, oal_netbuf_stru *newsk)
{
    skb_queue_tail(list, newsk);
}

/*
 * 函 数 名  : oal_netbuf_list_tail_nolock
 * 功能描述  : add a netbuf to skb list
 */
OAL_STATIC OAL_INLINE void oal_netbuf_list_tail_nolock(oal_netbuf_head_stru *list, oal_netbuf_stru *newsk)
{
    __skb_queue_tail(list, newsk);
}

/*
 * 函 数 名  : oal_netbuf_splice_init
 * 功能描述  : join two skb lists and reinitialise the emptied list
 * 输入参数  : @list: the new list to add
 *             @head: the place to add it in the first list
 * 输出参数  : The list at @list is reinitialised
 */
OAL_STATIC OAL_INLINE void oal_netbuf_splice_init(oal_netbuf_head_stru *list, oal_netbuf_head_stru *head)
{
    skb_queue_splice_init(list, head);
}
/*
 * 函 数 名  : oal_netbuf_queue_splice_tail_init
 * 功能描述  :  join two skb lists and reinitialise the emptied list
 * 输入参数  : @list: the new list to add
 *             @head: the place to add it in the first list
 * 输出参数  : The list at @list is reinitialised
 */
OAL_STATIC OAL_INLINE void oal_netbuf_queue_splice_tail_init(oal_netbuf_head_stru *list, oal_netbuf_head_stru *head)
{
    skb_queue_splice_tail_init(list, head);
}

/*
 * 函 数 名  : oal_netbuf_delist_tail
 * 功能描述  : remove skb from list tail
 * 输入参数  : @head: the place to add it in the first list
 * 输出参数  : The list at @list is reinitialised
 * 返 回 值  : the netbuf removed from the list
 */
OAL_STATIC OAL_INLINE oal_netbuf_stru *oal_netbuf_delist_tail(oal_netbuf_head_stru *head)
{
    return skb_dequeue_tail(head);
}

/*
 * 函 数 名  : oal_netbuf_splice_sync
 * 功能描述  : move head buffs to list
 * 输入参数  : @list: the new list to add
 *             @head: the place to add it in the first list
 * 输出参数  : The list at @list is reinitialised
 */
OAL_STATIC OAL_INLINE void oal_netbuf_splice_sync(oal_netbuf_head_stru *list, oal_netbuf_head_stru *head)
{
    oal_netbuf_stru *netbuf = NULL;
    for (;;) {
        netbuf = oal_netbuf_delist_tail(head);
        if (netbuf == NULL) {
            break;
        }
        oal_netbuf_addlist(list, netbuf);
    }
}

/*
 * 函 数 名  : oal_netbuf_head_init
 * 功能描述  : init netbuf list
 */
OAL_STATIC OAL_INLINE void oal_netbuf_head_init(oal_netbuf_head_stru *list)
{
    skb_queue_head_init(list);
}

OAL_STATIC OAL_INLINE oal_netbuf_stru *oal_netbuf_peek(oal_netbuf_head_stru *pst_head)
{
    return skb_peek(pst_head);
}

/*
 * 函 数 名  : oal_netbuf_tail
 * 功能描述  : 返回skb链表中的最后一个元素
 */
OAL_STATIC OAL_INLINE oal_netbuf_stru *oal_netbuf_tail(oal_netbuf_head_stru *pst_head)
{
    return skb_peek_tail(pst_head);
}

/*
 * 函 数 名  : oal_netbuf_free_list
 * 功能描述  : 从链表中释放制定个数的skb
 */
OAL_STATIC OAL_INLINE uint32_t oal_netbuf_free_list(oal_netbuf_head_stru *pst_head, uint32_t ul_num)
{
    uint32_t ul_index;
    uint32_t ul_ret;

    for (ul_index = 0; ul_index < ul_num; ul_index++) {
        ul_ret = oal_netbuf_free(oal_netbuf_delist(pst_head));
        if (ul_ret != OAL_SUCC) {
            return ul_ret;
        }
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : oal_netbuf_get_appointed_netbuf
 * 功能描述  : 获取当前netbuf元素后的第n个元素
 * 输入参数  : (1)起始查找节点
 *             (2)向后查找的个数
 * 输出参数  : 指向期望的netbuf的指针
 * 返 回 值  : 期望的betbuf元素的指针或空指针
 */
OAL_STATIC OAL_INLINE uint32_t oal_netbuf_get_appointed_netbuf(oal_netbuf_stru *pst_netbuf, uint8_t uc_num,
                                                               oal_netbuf_stru **pst_expect_netbuf)
{
    uint8_t uc_buf_num;

    if (oal_unlikely(oal_any_null_ptr2(pst_netbuf, pst_expect_netbuf))) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    *pst_expect_netbuf = NULL;

    for (uc_buf_num = 0; uc_buf_num < uc_num; uc_buf_num++) {
        *pst_expect_netbuf = oal_get_netbuf_next(pst_netbuf);

        if (*pst_expect_netbuf == NULL) {
            break;
        }

        pst_netbuf = *pst_expect_netbuf;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : oal_netbuf_clone
 * 功能描述  : skb克隆，复制头部 ，data指向同一块区域
 */
OAL_STATIC OAL_INLINE oal_netbuf_stru *oal_netbuf_clone(oal_netbuf_stru *pst_buf)
{
    int32_t l_flags = GFP_KERNEL;

    if (in_interrupt() || irqs_disabled()) {
        l_flags = GFP_ATOMIC;
    }

    return skb_clone(pst_buf, l_flags);
}

/*
 * 函 数 名  : oal_netbuf_decrease_user
 * 功能描述  : 将skb的引用计数减一
 */
OAL_STATIC OAL_INLINE uint32_t oal_netbuf_decrease_user(oal_netbuf_stru *pst_buf)
{
    if (oal_unlikely(pst_buf == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    refcount_dec(&(pst_buf->users));
#else
    atomic_dec(&(pst_buf->users));
#endif

    return OAL_SUCC;
}

/*
 * 函 数 名  : oal_netbuf_increase_user
 * 功能描述  : 将skb的引用计数+1
 */
OAL_STATIC OAL_INLINE uint32_t oal_netbuf_increase_user(oal_netbuf_stru *pst_buf)
{
    if (oal_unlikely(pst_buf == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    refcount_inc(&(pst_buf->users));
#else
    atomic_inc(&(pst_buf->users));
#endif

    return OAL_SUCC;
}

/*
 * 函 数 名  : oal_netbuf_read_user
 * 功能描述  : 读取netbuf引用计数
 */
OAL_STATIC OAL_INLINE uint32_t oal_netbuf_read_user(oal_netbuf_stru *pst_buf)
{
    if (oal_unlikely(pst_buf == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    return refcount_read(&(pst_buf->users));
#else
    return (uint32_t)atomic_read(&(pst_buf->users));
#endif
}

/*
 * 函 数 名  : oal_netbuf_set_user
 * 功能描述  : 设置netbuf引用计数
 */
OAL_STATIC OAL_INLINE void oal_netbuf_set_user(oal_netbuf_stru *pst_buf, uint32_t refcount)
{
    if (oal_unlikely(pst_buf == NULL)) {
        return;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    refcount_set(&(pst_buf->users), refcount);
#else
    atomic_set(&(pst_buf->users), (int32_t)refcount);
#endif
}

/*
 * 函 数 名  : oal_netbuf_get_buf_num
 * 功能描述  : 获取netbuf双向链表中buf的个数
 */
OAL_STATIC OAL_INLINE uint32_t oal_netbuf_get_buf_num(oal_netbuf_head_stru *pst_netbuf_head)
{
    return pst_netbuf_head->qlen;
}

OAL_STATIC OAL_INLINE oal_netbuf_stru *oal_netbuf_get(oal_netbuf_stru *pst_netbuf)
{
    return skb_get(pst_netbuf);
}

/*
 * 函 数 名  : oal_netbuf_queue_purge
 * 功能描述  : 释放skb链中的所有skb
 */
OAL_STATIC OAL_INLINE void oal_netbuf_queue_purge(oal_netbuf_head_stru *pst_netbuf_head)
{
    skb_queue_purge(pst_netbuf_head);
}

/*
 * 函 数 名  : oal_netbuf_copy_expand
 * 功能描述  : 复制skb以及数据，并扩充头部和尾部空间
 */
OAL_STATIC OAL_INLINE oal_netbuf_stru *oal_netbuf_copy_expand(oal_netbuf_stru *pst_netbuf,
                                                              int32_t ul_headroom,
                                                              int32_t ul_tailroom,
                                                              oal_gfp_enum_uint8 en_gfp_mask)
{
    return skb_copy_expand(pst_netbuf, ul_headroom, ul_tailroom, en_gfp_mask);
}

#ifdef _PRE_SKB_TRACE
#define oal_netif_rx_hw(pst_netbuf)                                            \
    ({                                                                         \
        mem_trace_delete_node ((uintptr_t)(pst_netbuf));                  \
        !in_interrupt() ? netif_rx_ni(pst_netbuf) : netif_rx(pst_netbuf); \
    })
#else
OAL_STATIC OAL_INLINE int32_t oal_netif_rx_hw(oal_netbuf_stru *pst_netbuf)
{
    if (!in_interrupt()) {
        return netif_rx_ni(pst_netbuf);
    } else {
        return netif_rx(pst_netbuf);
    }
}
#endif

/*
 * 函 数 名  : oal_netif_rx
 * 功能描述  : 将skb发给桥
 */
#ifdef _PRE_SKB_TRACE
#define oal_netif_rx(pst_netbuf)                              \
    ({                                                        \
        mem_trace_delete_node ((uintptr_t)(pst_netbuf)); \
        netif_rx(pst_netbuf);                                 \
    })
#else
OAL_STATIC OAL_INLINE int32_t oal_netif_rx(oal_netbuf_stru *pst_netbuf)
{
    return netif_rx(pst_netbuf);
}
#endif

/*
 * 函 数 名  : oal_netif_rx_ni
 * 功能描述  : 将skb发给桥  !in_interrupt()
 */
#ifdef _PRE_SKB_TRACE
#define oal_netif_rx_ni(pst_netbuf)                           \
    ({                                                        \
        mem_trace_delete_node ((uintptr_t)(pst_netbuf)); \
        netif_rx_ni(pst_netbuf);                              \
    })
#else
OAL_STATIC OAL_INLINE int32_t oal_netif_rx_ni(oal_netbuf_stru *pst_netbuf)
{
    return netif_rx_ni(pst_netbuf);
}
#endif

OAL_STATIC OAL_INLINE void oal_local_bh_disable(void)
{
    local_bh_disable();
}

OAL_STATIC OAL_INLINE void oal_local_bh_enable(void)
{
    local_bh_enable();
}

OAL_STATIC OAL_INLINE void oal_napi_schedule(struct napi_struct *napi)
{
    napi_schedule(napi);
#ifndef _PRE_PRODUCT_HI3751V811
#ifndef CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT
    if (local_softirq_pending()) {
        do_softirq();
    }
#endif
#endif
}

OAL_STATIC OAL_INLINE void oal_napi_gro_receive(struct napi_struct *napi, oal_netbuf_stru *pst_netbuf)
{
    napi_gro_receive(napi, pst_netbuf);
}

OAL_STATIC OAL_INLINE void oal_netif_receive_skb(oal_netbuf_stru *pst_netbuf)
{
    netif_receive_skb(pst_netbuf);
}

OAL_STATIC OAL_INLINE void oal_napi_complete(struct napi_struct *napi)
{
    napi_complete(napi);
}

OAL_STATIC OAL_INLINE void oal_netif_napi_add(struct net_device *dev, struct napi_struct *napi,
                                              int (*poll)(struct napi_struct *, int), int weight)
{
    netif_napi_add(dev, napi, poll, weight);
}

OAL_STATIC OAL_INLINE void oal_napi_disable(struct napi_struct *napi)
{
    napi_disable(napi);
}

OAL_STATIC OAL_INLINE void oal_napi_enable(struct napi_struct *napi)
{
    napi_enable(napi);
}

/*
 * 函 数 名  : oal_cpu_clock
 * 功能描述  : 获取时间戳
 */
OAL_STATIC OAL_INLINE uint64_t oal_cpu_clock(void)
{
    return cpu_clock(UINT_MAX);
}

OAL_STATIC OAL_INLINE int32_t oal_netbuf_expand_head(oal_netbuf_stru *netbuf,
                                                     int32_t nhead, int32_t ntail,
                                                     int32_t gfp_mask)
{
    return pskb_expand_head(netbuf, nhead, ntail, gfp_mask);
}

OAL_STATIC OAL_INLINE oal_sock_stru *oal_netlink_kernel_create(oal_net_stru *pst_net, int32_t l_unit,
                                                               uint32_t ul_groups,
                                                               void (*input)(oal_netbuf_stru *pst_netbuf),
                                                               oal_mutex_stru *pst_cb_mutex,
                                                               oal_module_stru *pst_module)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    // host evn netlink_kernel_create API is changed
    struct netlink_kernel_cfg cfg;

    // C01分支同步而来:不初始化会导致内核随机踩内存
    memset_s(&cfg, sizeof(cfg), 0, sizeof(cfg));
    cfg.groups = 0;
    cfg.input = input;
    cfg.cb_mutex = NULL;

    return netlink_kernel_create(pst_net, l_unit, &cfg);
#else
    return netlink_kernel_create(pst_net, l_unit, ul_groups, input, pst_cb_mutex, pst_module);
#endif
}

OAL_STATIC OAL_INLINE void oal_netlink_kernel_release(oal_sock_stru *pst_sock)
{
    netlink_kernel_release(pst_sock);
}

OAL_STATIC OAL_INLINE oal_nlmsghdr_stru *oal_nlmsg_hdr(oal_netbuf_stru *pst_netbuf)
{
    return (oal_nlmsghdr_stru *)oal_netbuf_header(pst_netbuf);
}

OAL_STATIC OAL_INLINE oal_nlmsghdr_stru *oal_nlmsg_put(oal_netbuf_stru *pst_netbuf, uint32_t ul_pid,
                                                       uint32_t ul_seq, int32_t l_type,
                                                       int32_t l_payload, int32_t l_flags)
{
    return nlmsg_put(pst_netbuf, ul_pid, ul_seq, l_type, l_payload, l_flags);
}

/*
 * 函 数 名  : oal_nla_put_u32
 * 功能描述  : 给netlink数据中添加32位的信息
 */
OAL_STATIC OAL_INLINE int32_t oal_nla_put_u32(oal_netbuf_stru *pst_skb, int32_t l_attrtype, uint32_t ul_value)
{
    return nla_put_u32(pst_skb, l_attrtype, ul_value);
}

/*
 * 函 数 名  : oal_nla_put_u16
 * 功能描述  : 给netlink数据中添加16位的信息
 */
OAL_STATIC OAL_INLINE int32_t oal_nla_put_u16(oal_netbuf_stru *skb, int32_t attrtype, uint16_t value)
{
    return nla_put_u16(skb, attrtype, value);
}

/*
 * 函 数 名  : oal_nla_put
 * 功能描述  : 给netlink消息中添加8位的信息
 */
OAL_STATIC OAL_INLINE int32_t oal_nla_put(oal_netbuf_stru *pst_skb, int32_t l_attrtype,
                                          int32_t l_attrlen, const void *p_data)
{
    return nla_put(pst_skb, l_attrtype, l_attrlen, p_data);
}

/*
 * nla_put_nohdr - Add a netlink attribute without header
 * @skb: socket buffer to add attribute to
 * @attrlen: length of attribute payload
 * @data: head of attribute payload
 *
 * Returns -EMSGSIZE if the tailroom of the skb is insufficient to store
 * the attribute payload.
 */
OAL_STATIC OAL_INLINE int32_t oal_nla_put_nohdr(oal_netbuf_stru *pst_skb, int32_t l_attrlen, const void *p_data)
{
    return nla_put_nohdr(pst_skb, l_attrlen, p_data);
}

/*
 * 函 数 名  : oal_nlmsg_new
 * 功能描述  : 分配一个新的netlink消息
 */
OAL_STATIC OAL_INLINE oal_netbuf_stru *oal_nlmsg_new(int32_t payload, oal_gfp_enum_uint8 flags)
{
    return nlmsg_new(payload, flags);
}

/*
 * 函 数 名  : oal_nlmsg_free
 * 功能描述  : 释放netlink消息
 */
OAL_STATIC OAL_INLINE void oal_nlmsg_free(oal_netbuf_stru *pst_skb)
{
    return nlmsg_free(pst_skb);
}

/*
 * 函 数 名  : oal_genlmsg_multicast
 * 功能描述  : 调用netlink广播发送函数来发送netlink消息
 */
OAL_STATIC OAL_INLINE int32_t oal_genlmsg_multicast(oal_netbuf_stru *pst_skb, uint32_t ul_pid,
                                                    uint32_t ul_group, oal_gfp_enum_uint8 flags)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0))
    return genlmsg_multicast(pst_skb, ul_pid, ul_group, flags);
#else
    /* Linux 新版本已经提供其他接口上报信息，不需要通过genlmsg_multicast 上报 */
    return OAL_SUCC;
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0)) */
}

/*
 * 函 数 名  : oal_genlmsg_put
 * 功能描述  : 给netlink消息中添加对应命令
 */
OAL_STATIC OAL_INLINE void *oal_genlmsg_put(oal_netbuf_stru *pst_skb, uint32_t ul_pid, uint32_t ul_seq,
                                            oal_genl_family_stru *pst_family, int32_t flags, uint8_t cmd)
{
    return genlmsg_put(pst_skb, ul_pid, ul_seq, pst_family, flags, cmd);
}

/*
 * 函 数 名  : oal_nla_nest_start
 * 功能描述  : 开始处理netlink消息的起始位置
 */
OAL_STATIC OAL_INLINE oal_nlattr_stru *oal_nla_nest_start(oal_netbuf_stru *pst_skb, int32_t l_attrtype)
{
    return nla_nest_start(pst_skb, l_attrtype);
}

/*
 * 函 数 名  : oal_genlmsg_cancel
 * 功能描述  : 取消netlink消息封装
 */
OAL_STATIC OAL_INLINE void oal_genlmsg_cancel(oal_netbuf_stru *pst_skb, void *pst_hdr)
{
    return genlmsg_cancel(pst_skb, pst_hdr);
}

/*
 * 函 数 名  : oal_nla_nest_end
 * 功能描述  : 完成netlink消息的所有填充
 */
OAL_STATIC OAL_INLINE int32_t oal_nla_nest_end(oal_netbuf_stru *pst_skb, oal_nlattr_stru *pst_start)
{
    return nla_nest_end(pst_skb, pst_start);
}

/*
 * 函 数 名  : oal_genlmsg_end
 * 功能描述  : netlink消息结束
 */
OAL_STATIC OAL_INLINE int32_t oal_genlmsg_end(oal_netbuf_stru *pst_skb, void *pst_hdr)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0))
    return genlmsg_end(pst_skb, pst_hdr);
#else
    genlmsg_end(pst_skb, pst_hdr);
    return OAL_SUCC;
#endif
}

/*
 * 函 数 名  : oal_nla_data
 * 功能描述  : 获取 netlink 属性的payload
 */
OAL_STATIC OAL_INLINE void *oal_nla_data(const oal_nlattr_stru *pst_nla)
{
    return nla_data(pst_nla);
}

/*
 * 函 数 名  : oal_nla_get_u8
 * 功能描述  : 获取 netlink 属性的payload，转换为u8 类型
 */
OAL_STATIC OAL_INLINE uint32_t oal_nla_get_u8(const oal_nlattr_stru *pst_nla)
{
    return nla_get_u8(pst_nla);
}

/*
 * 函 数 名  : oal_nla_get_u16
 * 功能描述  : 获取 netlink 属性的payload，转换为u16 类型
 */
OAL_STATIC OAL_INLINE uint32_t oal_nla_get_u16(const oal_nlattr_stru *pst_nla)
{
    return nla_get_u16(pst_nla);
}

/*
 * 函 数 名  : oal_nla_get_u32
 * 功能描述  : 获取 netlink 属性的payload，转换为u32 类型
 */
OAL_STATIC OAL_INLINE uint32_t oal_nla_get_u32(const oal_nlattr_stru *pst_nla)
{
    return nla_get_u32(pst_nla);
}

/*
 * 函 数 名  : oal_nla_total_size
 * 功能描述  : 获取 netlink 属性的总长度
 */
OAL_STATIC OAL_INLINE uint32_t oal_nla_total_size(const oal_nlattr_stru *pst_nla)
{
    int32_t payload = nla_len(pst_nla);
    return nla_total_size(payload);
}

/*
 * 函 数 名  : oal_nla_len
 * 功能描述  : 获取 netlink 属性的长度
 */
OAL_STATIC OAL_INLINE int oal_nla_len(const oal_nlattr_stru *pst_nla)
{
    return nla_len(pst_nla);
}

/*
 * 函 数 名  : oal_nla_type
 * 功能描述  : 获取 netlink 属性的类型
 */
OAL_STATIC OAL_INLINE int oal_nla_type(const oal_nlattr_stru *pst_nla)
{
    return nla_type(pst_nla);
}

OAL_STATIC OAL_INLINE oal_cfg80211_registered_device_stru *oal_wiphy_to_dev(oal_wiphy_stru *pst_wiphy)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0))
    return wiphy_to_dev(pst_wiphy);
#else
    return wiphy_to_rdev(pst_wiphy);
#endif
}

#ifdef _PRE_SKB_TRACE
#define oal_netlink_unicast(pst_sock, pst_netbuf, ul_pid, l_nonblock) \
    ({                                                                \
        mem_trace_delete_node ((uintptr_t)(pst_netbuf));         \
        netlink_unicast(pst_sock, pst_netbuf, ul_pid, l_nonblock);    \
    })
#else
OAL_STATIC OAL_INLINE int32_t oal_netlink_unicast(oal_sock_stru *pst_sock, oal_netbuf_stru *pst_netbuf,
                                                  uint32_t ul_pid, int32_t l_nonblock)
{
    return netlink_unicast(pst_sock, pst_netbuf, ul_pid, l_nonblock);
}
#endif

/*
 * 函 数 名  : oal_netlink_broadcast
 * 功能描述  : 封装内核netlink广播发送函数
 * 输入参数  : pst_sock:    Netlink接口的socket
 *             pst_netbuf:  数据消息内存
 *             ul_pid:      Pid值
 *             ul_group:    组播组
 *             en_gfp:      不可以睡眠的原子上下文分配内存
 */
#ifdef _PRE_SKB_TRACE
#define oal_netlink_broadcast(pst_sock, pst_netbuf, ul_pid, ul_group, en_gfp) \
    ({                                                                        \
        mem_trace_delete_node ((uintptr_t)(pst_netbuf));                 \
        netlink_broadcast(pst_sock, pst_netbuf, ul_pid, ul_group, en_gfp);    \
    })
#else
OAL_STATIC OAL_INLINE int32_t oal_netlink_broadcast(oal_sock_stru *pst_sock, oal_netbuf_stru *pst_netbuf,
                                                    uint32_t ul_pid, int32_t ul_group, oal_gfp_enum_uint8 en_gfp)
{
    return netlink_broadcast(pst_sock, pst_netbuf, ul_pid, ul_group, en_gfp);
}
#endif

/*
 * 函 数 名  : oal_netbuf_copydata
 * 功能描述  : 将skb中的内容先偏移ul_offset后 按指定长度拷贝到指定内从中
 */
OAL_STATIC OAL_INLINE int32_t oal_netbuf_copydata(oal_netbuf_stru *pst_netbuf_sc, uint32_t ul_offset,
                                                  void *p_dst, uint32_t ul_len)
{
    return skb_copy_bits(pst_netbuf_sc, ul_offset, p_dst, ul_len);
}

/*
 * 函 数 名  : oal_netbuf_trim
 * 功能描述  : 剥去skb中尾部的信息
 */
OAL_STATIC OAL_INLINE void oal_netbuf_trim(oal_netbuf_stru *pst_netbuf, uint32_t ul_len)
{
    return skb_trim(pst_netbuf, pst_netbuf->len - ul_len);
}

/*
 * 函 数 名  : oal_netbuf_concat
 * 功能描述  : 向netbu_head的尾部串接netbuf
 */
OAL_STATIC OAL_INLINE void oal_netbuf_concat(oal_netbuf_stru *pst_netbuf_head, oal_netbuf_stru *pst_netbuf)
{
    if (skb_is_nonlinear(pst_netbuf_head)) {
        oal_io_print("oal_netbuf_concat:pst_netbuf_head not linear ");
    }

    if (memcpy_s(skb_tail_pointer(pst_netbuf_head), skb_tailroom(pst_netbuf_head),
                 pst_netbuf->data, pst_netbuf->len) != EOK) {
        oal_io_print("not enough space for concat, destlen=%u, srclen=%u",
                     skb_tailroom(pst_netbuf_head), pst_netbuf->len);
    } else {
        skb_put(pst_netbuf_head, pst_netbuf->len);
    }

    dev_kfree_skb(pst_netbuf);
}

/*
 * 函 数 名  : oal_netbuf_set_len
 * 功能描述  : 将skb的数据内容长度设置为指定长度
 */
OAL_STATIC OAL_INLINE void oal_netbuf_set_len(oal_netbuf_stru *pst_netbuf, uint32_t ul_len)
{
    if (pst_netbuf->len > ul_len) {
        skb_trim(pst_netbuf, ul_len);
    } else {
        skb_put(pst_netbuf, (ul_len - pst_netbuf->len));
    }
}

OAL_STATIC OAL_INLINE void oal_netbuf_init(oal_netbuf_stru *pst_netbuf, uint32_t ul_len)
{
    oal_netbuf_set_len(pst_netbuf, ul_len);
    pst_netbuf->protocol = ETH_P_CONTROL;
}

/*
 * 函 数 名  : oal_hi_kernel_wdt_clear
 * 功能描述  : 狗复位
 */
OAL_STATIC OAL_INLINE void oal_hi_kernel_wdt_clear(void)
{
}

/*
 * 函 数 名  : oal_in_aton
 * 功能描述  : IP 地址转换（字符串 -> 数值）
 */
OAL_STATIC OAL_INLINE uint32_t oal_in_aton(uint8_t *pul_str)
{
    return (in_aton(pul_str));
}

/*
 * 函 数 名  : oal_ipv6_addr_copy
 * 功能描述  : 拷贝ipv6地址
 */
OAL_STATIC OAL_INLINE void oal_ipv6_addr_copy(oal_in6_addr *pst_ipv6_dst, oal_in6_addr *pst_ipv6_src)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
#else
    ipv6_addr_copy(pst_ipv6_dst, pst_ipv6_src);
#endif
}

/*
 * 函 数 名  : oal_dev_hard_header
 * 功能描述  : 构造以太头
 */
OAL_STATIC OAL_INLINE int32_t oal_dev_hard_header(oal_netbuf_stru *pst_nb,
                                                  oal_net_device_stru *pst_net_dev,
                                                  uint16_t us_type,
                                                  uint8_t *puc_addr_d,
                                                  uint8_t *puc_addr_s,
                                                  uint32_t ul_len)
{
    return dev_hard_header(pst_nb, pst_net_dev, us_type, puc_addr_d, puc_addr_s, ul_len);
}

/*
 * 函 数 名  : oal_csum_ipv6_magic
 * 功能描述  : 生成ipv6的magic
 */
OAL_STATIC OAL_INLINE uint16_t oal_csum_ipv6_magic(oal_in6_addr *pst_ipv6_s,
                                                   oal_in6_addr *pst_ipv6_d,
                                                   uint32_t ul_len,
                                                   uint16_t us_proto,
                                                   uint32_t ul_sum)
{
    return csum_ipv6_magic(pst_ipv6_s, pst_ipv6_d, ul_len, us_proto, ul_sum);
}

/*
 * 函 数 名  : oal_csum_partial
 * 功能描述  : 计算check sum
 */
OAL_STATIC OAL_INLINE uint32_t oal_csum_partial(const void *p_buff,
                                                int32_t l_len,
                                                uint32_t ul_sum)
{
    return csum_partial(p_buff, l_len, ul_sum);
}

/*
 * 函 数 名  : oal_ipv6_addr_type
 * 功能描述  : 获取ipv6地址的类型
 */
OAL_STATIC OAL_INLINE int32_t oal_ipv6_addr_type(oal_in6_addr *pst_ipv6)
{
#ifdef _PRE_WLAN_FEATURE_SUPPORT_IPV6
    return ipv6_addr_type(pst_ipv6);
#else /* 不支持ipv6产品，相关函数无法获取，直接返回OAL_IPV6_ADDR_RESERVED 0x2000U */
    return (int32_t)IPV6_ADDR_RESERVED;
#endif
}
/*
 * 函 数 名  : oal_pskb_may_pull
 * 功能描述  : 保证skb->data包含ul_len指指示的空间，如果没有，则从 skb_shinfo(skb)->frags[]中
 *             拷贝一份数据。
 */
OAL_STATIC OAL_INLINE int32_t oal_pskb_may_pull(oal_netbuf_stru *pst_nb, uint32_t ul_len)
{
    return pskb_may_pull(pst_nb, ul_len);
}

/*
 * 函 数 名  : oal_arp_create
 * 功能描述  : 构造arp帧
 */
OAL_STATIC OAL_INLINE oal_netbuf_stru *oal_arp_create(int32_t l_type, int32_t l_ptype, uint32_t ul_dest_ip,
                                                      oal_net_device_stru *pst_dev, uint32_t ul_src_ip,
                                                      uint8_t *puc_dest_hw,
                                                      uint8_t *puc_src_hw,
                                                      uint8_t *puc_target_hw)
{
    return arp_create(l_type, l_ptype, ul_dest_ip, pst_dev, ul_src_ip, puc_dest_hw, puc_src_hw, puc_target_hw);
}

/* get the queue index of the input skbuff */
OAL_STATIC OAL_INLINE uint16_t oal_skb_get_queue_mapping(oal_netbuf_stru *pst_skb)
{
    return skb_get_queue_mapping(pst_skb);
}

OAL_STATIC OAL_INLINE void oal_skb_set_queue_mapping(oal_netbuf_stru *pst_skb, uint16_t queue_mapping)
{
    skb_set_queue_mapping(pst_skb, queue_mapping);
}

/*
 * 函 数 名  : oal_notice_netif_rx
 * 功能描述  : 产品挂载接收通告, 上送SKB报文
 */
OAL_STATIC OAL_INLINE void oal_notice_netif_rx(oal_netbuf_stru *pst_netbuf)
{
    return;
}

/*
 * 函 数 名  : oal_notice_sta_join_result
 * 功能描述  : 产品挂载接收通告, VAP STA关联情况
 */
OAL_STATIC OAL_INLINE uint32_t oal_notice_sta_join_result(uint8_t uc_chip_id, oal_bool_enum_uint8 en_succ)
{
    return OAL_SUCC;
}
#endif /* end of oal_net.h */
