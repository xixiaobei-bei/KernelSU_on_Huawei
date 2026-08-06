

#include "oal_net.h"
#include "mac_frame.h"

#include "oal_cfg80211.h"
#include "oam_wdk.h"
#include "securec.h"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <net/genetlink.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAL_NET_C

#define oal_high_half_byte(a) (((a)&0xF0) >> 4)
#define oal_low_half_byte(a)  ((a)&0x0F)
#define MAC_DHCP_UDP_SRC_PORT 68
#define MAC_DHCP_UDP_DES_PORT 67

#define wlan_tos_to_hcc_queue(_tos) ( \
    (((_tos) == 0) || ((_tos) == 3)) ? \
    WLAN_UDP_BE_QUEUE : (((_tos) == 1) || ((_tos) == 2)) ? \
    WLAN_UDP_BK_QUEUE : (((_tos) == 4) || ((_tos) == 5)) ? \
    WLAN_UDP_VI_QUEUE : WLAN_UDP_VO_QUEUE)

#define WLAN_DATA_VIP_QUEUE WLAN_HI_QUEUE

/* 全局变量定义 */
#if (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
oal_net_device_stru *g_past_net_device[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT] = { NULL };
oal_net_stru init_net;
oal_sock_stru sock;
#endif

/*
 * 功能描述  : 判断是否是dhcp帧 port
 */
/*lint -e695*/
oal_bool_enum_uint8 oal_netbuf_is_dhcp_port(oal_udp_header_stru *udp_hdr)
{
    if (oal_unlikely(udp_hdr == NULL)) {
        oal_warn_on(1);
        return OAL_FALSE;
    }

    return (((oal_host2net_short(udp_hdr->source) == MAC_DHCP_UDP_SRC_PORT) &&
        (oal_host2net_short(udp_hdr->dest) == MAC_DHCP_UDP_DES_PORT)) ||
        ((oal_host2net_short(udp_hdr->source) == MAC_DHCP_UDP_DES_PORT) &&
        (oal_host2net_short(udp_hdr->dest) == MAC_DHCP_UDP_SRC_PORT)));
} /*lint +e695*/

/*
 * 功能描述  : 判断是否是nd帧
 */
oal_bool_enum_uint8 oal_netbuf_is_nd(oal_ipv6hdr_stru *ipv6hdr)
{
    oal_icmp6hdr_stru *icmp6hdr = NULL;

    if (oal_unlikely(ipv6hdr == NULL)) {
        oal_warn_on(1);
        return OAL_FALSE;
    }
    if (ipv6hdr->nexthdr == OAL_IPPROTO_ICMPV6) {
        icmp6hdr = (oal_icmp6hdr_stru *)(ipv6hdr + 1);
        if ((icmp6hdr->icmp6_type == MAC_ND_RSOL) ||
            (icmp6hdr->icmp6_type == MAC_ND_RADVT) ||
            (icmp6hdr->icmp6_type == MAC_ND_NSOL) ||
            (icmp6hdr->icmp6_type == MAC_ND_NADVT) ||
            (icmp6hdr->icmp6_type == MAC_ND_RMES)) {
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}

/*
 * 功能描述  : 判断是否是DHCP6帧
 */
oal_bool_enum_uint8 oal_netbuf_is_dhcp6(oal_ipv6hdr_stru *ipv6hdr)
{
    oal_udp_header_stru *udp_hdr = NULL;

    if (oal_unlikely(ipv6hdr == NULL)) {
        oal_warn_on(1);
        return OAL_FALSE;
    }
    if (ipv6hdr->nexthdr == MAC_UDP_PROTOCAL) {
        udp_hdr = (oal_udp_header_stru *)(ipv6hdr + 1);
        if (((oal_host2net_short(udp_hdr->source) == MAC_IPV6_UDP_SRC_PORT) &&
             (oal_host2net_short(udp_hdr->dest) == MAC_IPV6_UDP_DES_PORT)) ||
            ((oal_host2net_short(udp_hdr->source) == MAC_IPV6_UDP_DES_PORT) &&
             (oal_host2net_short(udp_hdr->dest) == MAC_IPV6_UDP_SRC_PORT))) {
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}

/*
 * 函 数 名  : oal_netbuf_is_tcp_ack
 * 功能描述  : 判断ipv4 tcp报文是否为tcp 关键帧，包括TCP ACK\TCP SYN帧等
 */
oal_bool_enum_uint8 oal_netbuf_is_tcp_ack(oal_ip_header_stru *ip_hdr)
{
    oal_tcp_header_stru *tcp_hdr = NULL;
    uint32_t ip_pkt_len;
    uint32_t ip_hdr_len;
    uint32_t tcp_hdr_len;

    if (oal_unlikely(ip_hdr == NULL)) {
        oal_warn_on(1);
        return OAL_FALSE;
    }

    tcp_hdr     = (oal_tcp_header_stru *)(ip_hdr + 1);
    ip_pkt_len  = oal_net2host_short(ip_hdr->us_tot_len);
    ip_hdr_len  = (oal_low_half_byte(ip_hdr->us_ihl)) << 2; /* 2 协议规范 */
    tcp_hdr_len = (oal_high_half_byte(tcp_hdr->uc_offset)) << 2; /* 2 协议规范 */
    if (tcp_hdr_len + ip_hdr_len == ip_pkt_len) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*
 * 功能描述  : 判断ipv4 是否为icmp报文
 * 输入参数  : pst_ip_hdr: IP报文头部
 */
oal_bool_enum_uint8 oal_netbuf_is_icmp(oal_ip_header_stru *ip_hdr)
{
    if (oal_unlikely(ip_hdr == NULL)) {
        oal_warn_on(1);
        return OAL_FALSE;
    }

    /* ICMP报文检查 */
    if (ip_hdr->uc_protocol == MAC_ICMP_PROTOCAL) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*
 * 功能描述  : 判断ipv6 tcp报文是否为tcp ack
 */
oal_bool_enum_uint8 oal_netbuf_is_tcp_ack6(oal_ipv6hdr_stru *ipv6hdr)
{
    oal_tcp_header_stru *tcp_hdr = NULL;
    uint32_t ip_pkt_len;
    uint32_t tcp_hdr_len;

    if (oal_unlikely(ipv6hdr == NULL)) {
        oal_warn_on(1);
        return OAL_FALSE;
    }

    tcp_hdr = (oal_tcp_header_stru *)(ipv6hdr + 1);
    /* ipv6 净载荷, ipv6报文头部固定为40字节 */
    ip_pkt_len = oal_net2host_short(ipv6hdr->payload_len);
    tcp_hdr_len = (oal_high_half_byte(tcp_hdr->uc_offset)) << 2; /* 2 协议规范 */
    if (tcp_hdr_len == ip_pkt_len) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*
 * 功能描述  : 为以太网来包选择合的subqueue入队
 */
uint16_t oal_netbuf_select_queue(oal_netbuf_stru *pst_buf)
{
    oal_ether_header_stru *pst_ether_header = NULL;
    oal_ip_header_stru *pst_ip = NULL;
    oal_ipv6hdr_stru *pst_ipv6 = NULL;
    oal_udp_header_stru *pst_udp_hdr = NULL;
#ifdef _PRE_WLAN_FEATURE_SCHEDULE
    oal_tcp_header_stru *pst_tcp_hdr = NULL;
#endif
    oal_vlan_ethhdr_stru *pst_vlan_ethhdr = NULL;
    uint32_t ul_ipv6_hdr;
    uint32_t ul_pri;
    uint16_t us_vlan_tci;
    uint8_t uc_tos;
    uint8_t us_queue = WLAN_NORMAL_QUEUE;

    if (oal_unlikely(pst_buf == NULL)) {
        oal_warn_on(1);
        return WLAN_NET_QUEUE_BUTT;
    }

    /* 获取以太网头 */
    pst_ether_header = (oal_ether_header_stru *)oal_netbuf_data(pst_buf);

    switch (pst_ether_header->us_ether_type) {
            /*lint -e778*/ /* 屏蔽Info-- Constant expression evaluates to 0 in operation '&' */
        case oal_host2net_short(ETHER_TYPE_IP):

            pst_ip = (oal_ip_header_stru *)(pst_ether_header + 1); /* 偏移一个以太网头，取ip头 */

            /* 对udp报文区分qos入队 */
            if (pst_ip->uc_protocol == MAC_UDP_PROTOCAL) {
                /* 从IP TOS字段寻找优先级 */
                /*
                 * ----------------------------------------------------------------------
                 *     tos位定义
                 *  ----------------------------------------------------------------------
                 * | bit7~bit5 | bit4 |  bit3  |  bit2  |   bit1   | bit0 |
                 * | 包优先级  | 时延 | 吞吐量 | 可靠性 | 传输成本 | 保留 |
                 *  ----------------------------------------------------------------------
                 */
                uc_tos = pst_ip->uc_tos >> WLAN_IP_PRI_SHIFT;
                us_queue = wlan_tos_to_hcc_queue(uc_tos);

                /* 如果是DHCP帧，则进入DATA_HIGH_QUEUE */
                pst_udp_hdr = (oal_udp_header_stru *)(pst_ip + 1);
                if (((pst_ip->us_frag_off & 0xFF1F) == 0) && (oal_netbuf_is_dhcp_port(pst_udp_hdr) == OAL_TRUE)) {
                    us_queue = WLAN_DATA_VIP_QUEUE;
                }
            } else if (pst_ip->uc_protocol == MAC_TCP_PROTOCAL) { /* 区分TCP ack与TCP data报文 */
                if (oal_netbuf_is_tcp_ack(pst_ip) == OAL_TRUE) {
                    us_queue = WLAN_TCP_ACK_QUEUE;
                } else {
                    us_queue = WLAN_TCP_DATA_QUEUE;
                }
            }
            break;

        case oal_host2net_short(ETHER_TYPE_IPV6):

            /* 从IPv6 traffic class字段获取优先级 */
            /*
             * ----------------------------------------------------------------------
             *     IPv6包头 前32为定义
             *  -----------------------------------------------------------------------
             * | 版本号 | traffic class   | 流量标识 |
             * | 4bit   | 8bit(同ipv4 tos)|  20bit   |
             * -----------------------------------------------------------------------
             */
            pst_ipv6 = (oal_ipv6hdr_stru *)(pst_ether_header + 1); /* 偏移一个以太网头，取ip头 */
            ul_ipv6_hdr = (*(uint32_t *)pst_ipv6);

            if (pst_ipv6->nexthdr == MAC_UDP_PROTOCAL) { /* UDP报文 */
                ul_pri = (oal_net2host_long(ul_ipv6_hdr) & WLAN_IPV6_PRIORITY_MASK) >> WLAN_IPV6_PRIORITY_SHIFT;
                uc_tos = (uint8_t)(ul_pri >> WLAN_IP_PRI_SHIFT);
                us_queue = wlan_tos_to_hcc_queue(uc_tos);
            } else if (pst_ipv6->nexthdr == MAC_TCP_PROTOCAL) { /* TCP报文 */
                if (oal_netbuf_is_tcp_ack6(pst_ipv6) == OAL_TRUE) {
                    us_queue = WLAN_TCP_ACK_QUEUE;
                } else {
                    us_queue = WLAN_TCP_DATA_QUEUE;
                }
            } else if (oal_netbuf_is_dhcp6((oal_ipv6hdr_stru *)(pst_ether_header + 1)) == OAL_TRUE) {
                us_queue = WLAN_DATA_VIP_QUEUE; /* 如果是DHCPV6帧，则进入WLAN_DATA_VIP_QUEUE队列缓存 */
            }
            break;

        case oal_host2net_short(ETHER_TYPE_PAE):
            /* 如果是EAPOL帧，则进入VO队列发送 */
            us_queue = WLAN_DATA_VIP_QUEUE;
            break;

        /* TDLS帧处理，建链保护，入高优先级TID队列 */
        case oal_host2net_short(ETHER_TYPE_TDLS):
            us_queue = WLAN_DATA_VIP_QUEUE;
            break;

        /* PPPOE帧处理，建链保护(发现阶段, 会话阶段)，入高优先级TID队列 */
        case oal_host2net_short(ETHER_TYPE_PPP_DISC):
        case oal_host2net_short(ETHER_TYPE_PPP_SES):
            us_queue = WLAN_DATA_VIP_QUEUE;
            break;

#ifdef _PRE_WLAN_FEATURE_WAPI
        case oal_host2net_short(ETHER_TYPE_WAI):
            us_queue = WLAN_DATA_VIP_QUEUE;
            break;
#endif

        case oal_host2net_short(ETHER_TYPE_VLAN):

            /* 获取vlan tag的优先级 */
            pst_vlan_ethhdr = (oal_vlan_ethhdr_stru *)oal_netbuf_data(pst_buf);

            /*
             * ------------------------------------------------------------------
             *     802.1Q(VLAN) TCI(tag control information)位定义
             *  -------------------------------------------------------------------
             * |Priority | DEI  | Vlan Identifier |
             * | 3bit    | 1bit |      12bit      |
             *  ------------------------------------------------------------------
             */
            us_vlan_tci = oal_net2host_short(pst_vlan_ethhdr->h_vlan_TCI);

            uc_tos = us_vlan_tci >> OAL_VLAN_PRIO_SHIFT; /* 右移13位，提取高3位优先级 */
            us_queue = wlan_tos_to_hcc_queue(uc_tos);

            break;

        /*lint +e778*/
        default:
            us_queue = WLAN_NORMAL_QUEUE;
            break;
    }

    return us_queue;
}

/*lint -e19*/
oal_module_symbol(oal_netbuf_is_tcp_ack6);
oal_module_symbol(oal_netbuf_select_queue);
/*lint +e19*/

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && defined(_PRE_WLAN_FEATURE_DFR)
struct dev_excp_globals {
    oal_sock_stru *nlsk;
    uint8_t mode;
    uint8_t *data;
    uint32_t usepid;
};

struct dev_netlink_msg_hdr_stru {
    uint32_t cmd;
    uint32_t len;
};

struct dev_excp_globals g_dev_excp_handler_data;

/*
 * Prototype    : dev_netlink_rev
 * Description  : receive netlink date from app
 */
#define OAL_EXCP_DATA_BUF_LEN 64

void dev_netlink_rev(oal_netbuf_stru *skb)
{
#ifdef _PRE_WINDOWS_SUPPORT
    return;
#else
    oal_netbuf_stru *pst_skb = NULL;
    oal_nlmsghdr_stru *pst_nlh = NULL;
    struct dev_netlink_msg_hdr_stru st_msg_hdr;
    uint32_t ul_len;
    int32_t ret;

    if (skb == NULL) {
        oal_io_print("WIFI DFR:para fail\n");
        return;
    }

    oal_io_print("WIFI DFR:dev_kernel_netlink_recv.\n");

    memset_s(g_dev_excp_handler_data.data, OAL_EXCP_DATA_BUF_LEN, 0, OAL_EXCP_DATA_BUF_LEN);
    pst_skb = oal_netbuf_get(skb);
    if (pst_skb->len >= oal_nlmsg_space(0)) {
        pst_nlh = oal_nlmsg_hdr(pst_skb);
        /* 检测报文长度正确性 */
        if (!oal_nlmsg_ok(pst_nlh, pst_skb->len)) {
            oal_io_print("[ERROR]invaild netlink buff data packge data len = :%u,skb_buff data len = %u\n",
                         pst_nlh->nlmsg_len, pst_skb->len);
            goto fail;
        }
        ul_len = oal_nlmsg_payload(pst_nlh, 0);
        /* 后续需要拷贝sizeof(st_msg_hdr),故判断之 */
        if (ul_len < sizeof(st_msg_hdr)) {
            oal_io_print("[ERROR]invaild netlink buff len:%u,max len:%u\n", ul_len, OAL_EXCP_DATA_BUF_LEN);
            goto fail;
        }

        ret = memcpy_s(g_dev_excp_handler_data.data, OAL_EXCP_DATA_BUF_LEN, oal_nlmsg_data(pst_nlh), ul_len);
        if (ret != EOK) {
            oal_io_print("memcpy_s error, destlen=%u, srclen=%u\n", OAL_EXCP_DATA_BUF_LEN, ul_len);
            goto fail;
        }

        ret = memcpy_s((void *)&st_msg_hdr, sizeof(st_msg_hdr), g_dev_excp_handler_data.data, sizeof(st_msg_hdr));
        if (ret != EOK) {
            oal_io_print("memcpy_s error\n");
            goto fail;
        }

        if (st_msg_hdr.cmd == 0) {
            g_dev_excp_handler_data.usepid = pst_nlh->nlmsg_pid; /* pid of sending process */
            oal_io_print("WIFI DFR:pid is [%d], msg is [%s]\n",
                         g_dev_excp_handler_data.usepid, &g_dev_excp_handler_data.data[sizeof(st_msg_hdr)]);
        }
    }
fail:
    kfree_skb(pst_skb);
    return;
#endif
}

/*
 * Prototype    : dev_netlink_create
 * Description  : create netlink for device exception
 */
int32_t dev_netlink_create(void)
{
    g_dev_excp_handler_data.nlsk = oal_netlink_kernel_create(&OAL_INIT_NET, NETLINK_DEV_ERROR, 0,
                                                             dev_netlink_rev, NULL, OAL_THIS_MODULE);
    if (g_dev_excp_handler_data.nlsk == NULL) {
        oal_io_print("WIFI DFR:fail to create netlink socket \n");
        return -OAL_EFAIL;
    }

    oal_io_print("WIFI DFR:suceed to create netlink socket %p \n", g_dev_excp_handler_data.nlsk);
    return OAL_SUCC;
}

/*
 * Prototype    : dev_netlink_send
 * Description  : send netlink data
 */
int32_t dev_netlink_send(uint8_t *data, int data_len)
{
#ifdef _PRE_WINDOWS_SUPPORT
    return OAL_SUCC;
#else
    oal_netbuf_stru *skb = NULL;
    oal_nlmsghdr_stru *nlh = NULL;
    uint32_t ret;
    oal_uint len;

    if (oal_unlikely(data == NULL)) {
        oal_warn_on(1);
        return -OAL_EFAIL;
    }

    if (data_len < 0) {
        oal_io_print("WIFI DFR:dev error:data_len[%d].\n", data_len);
        return -OAL_EFAIL;
    }

    len = oal_nlmsg_space(((oal_uint)data_len));
    skb = alloc_skb(len, GFP_KERNEL);
    if (skb == NULL) {
        oal_io_print("WIFI DFR:dev error: allocate failed, len[%lu].\n", len);
        return -OAL_EFAIL;
    }
    nlh = oal_nlmsg_put(skb, 0, 0, 0, data_len, 0);

    oal_io_print("WIFI DFR: data[%s].\n", data);

    ret = memcpy_s(oal_nlmsg_data(nlh), data_len, data, data_len);
    if (ret != EOK) {
        oal_io_print("WIFI DFR: buf data load failed.\n");
        kfree_skb(skb);
        return -OAL_EFAIL;
    }

#if (KERNEL_VERSION(2, 6, 34) <= LINUX_VERSION_CODE)
    oal_netlink_cb(skb).portid = 0; /* from kernel */
#endif
    if (g_dev_excp_handler_data.nlsk == NULL) {
        oal_io_print("WIFI DFR: NULL Pointer_sock.\n");
        kfree_skb(skb);
        return -OAL_EFAIL;
    }

    ret = oal_netlink_unicast(g_dev_excp_handler_data.nlsk, skb, g_dev_excp_handler_data.usepid, MSG_DONTWAIT);
    if (ret <= 0) {
        oal_io_print("WIFI DFR:send dev error netlink msg, ret = %d \n", ret);
    }

    return ret;
#endif
}

/*
 * Prototype    : init_dev_excp_handler
 * Description  : init dev exception handler
 */
int32_t init_dev_excp_handler(void)
{
    int32_t ret;

    oal_io_print("DFR: into init_exception_enable_handler\n");

    memset_s((uint8_t *)&g_dev_excp_handler_data, sizeof(g_dev_excp_handler_data),
             0, sizeof(g_dev_excp_handler_data));

    g_dev_excp_handler_data.data = (uint8_t *)kzalloc(OAL_EXCP_DATA_BUF_LEN, GFP_KERNEL);
    if (g_dev_excp_handler_data.data == NULL) {
        oal_io_print("DFR: alloc g_dev_excp_handler_data.puc_data fail, len = %d.\n", OAL_EXCP_DATA_BUF_LEN);
        g_dev_excp_handler_data.data = NULL;
        return -OAL_EFAIL;
    }

    memset_s(g_dev_excp_handler_data.data, OAL_EXCP_DATA_BUF_LEN, 0, OAL_EXCP_DATA_BUF_LEN);

    ret = dev_netlink_create();
    if (ret < 0) {
        kfree(g_dev_excp_handler_data.data);
        g_dev_excp_handler_data.data = NULL;
        oal_io_print("init_dev_err_kernel init is ERR!\n");
        return -OAL_EFAIL;
    }

    oal_io_print("DFR: init_exception_enable_handler init ok.\n");

    return OAL_SUCC;
}

/*
 * Prototype    : deinit_dev_excp_handler
 * Description  : deinit dev exception handler
 */
void deinit_dev_excp_handler(void)
{
    if (g_dev_excp_handler_data.nlsk != NULL) {
        oal_netlink_kernel_release(g_dev_excp_handler_data.nlsk);
        g_dev_excp_handler_data.usepid = 0;
    }

    if (g_dev_excp_handler_data.data != NULL) {
        kfree(g_dev_excp_handler_data.data);
    }

    oal_io_print("DFR: deinit ok.\n");

    return;
}
#else
int32_t dev_netlink_create(void)
{
    return OAL_SUCC;
}

int32_t dev_netlink_send(uint8_t *data, int data_len)
{
    return OAL_SUCC;
}
int32_t init_dev_excp_handler(void)
{
    return OAL_SUCC;
}
void deinit_dev_excp_handler(void)
{
    return;
}
#endif

/*lint -e19*/
oal_module_symbol(oal_netbuf_is_dhcp_port);
oal_module_symbol(oal_netbuf_is_dhcp6);
oal_module_symbol(oal_netbuf_is_nd);
oal_module_symbol(oal_netbuf_is_tcp_ack);
oal_module_symbol(oal_netbuf_is_icmp);
oal_module_symbol(init_dev_excp_handler);
oal_module_symbol(deinit_dev_excp_handler); /*lint +e19*/
