

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_net.h"

#include "securec.h"

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <net/genetlink.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#endif

#include "mac_frame.h"
#include "mac_data.h"
#include "oal_cfg80211.h"
#include "oam_wdk.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAL_NET_C

#define OAL_HIGH_HALF_BYTE(a) (((a)&0xF0) >> 4)
#define OAL_LOW_HALF_BYTE(a)  ((a)&0x0F)

#define MAC_DHCP_UDP_SRC_PORT   68
#define MAC_DHCP_UDP_DES_PORT   67
#define OAL_EXCP_DATA_BUF_LEN  (64)

#ifdef _PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL

#define WLAN_TOS_TO_HCC_QUEUE(_tos) ( \
    (((_tos) == 0) || ((_tos) == 3)) ? WLAN_UDP_BE_QUEUE : (((_tos) == 1) || ((_tos) == 2)) ? \
    WLAN_UDP_BK_QUEUE : (((_tos) == 4) || ((_tos) == 5)) ? WLAN_UDP_VI_QUEUE : WLAN_UDP_VO_QUEUE)

#define WLAN_DATA_VIP_QUEUE (WLAN_HI_QUEUE)
#endif

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#if (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)

oal_net_device_stru *past_net_device[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT] = { OAL_PTR_NULL };

oal_net_stru init_net;

oal_sock_stru sock;

#endif

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*
 * 函 数 名  : oal_netbuf_is_dhcp_port
 * 功能描述  : 判断是否是dhcp帧 port
 * 输入参数  : 无
 * 输出参数  : 无
 * 返 回 值  : 无
 */
/*lint -e695*/
OAL_INLINE oal_bool_enum_uint8 oal_netbuf_is_dhcp_port(oal_udp_header_stru *pst_udp_hdr)
{
    if (OAL_UNLIKELY(pst_udp_hdr == NULL)) {
        OAL_WARN_ON(1);
        return OAL_FALSE;
    }

    if (((OAL_HOST2NET_SHORT(pst_udp_hdr->source) == MAC_DHCP_UDP_SRC_PORT)
         && (OAL_HOST2NET_SHORT(pst_udp_hdr->dest) == MAC_DHCP_UDP_DES_PORT))
        || ((OAL_HOST2NET_SHORT(pst_udp_hdr->source) == MAC_DHCP_UDP_DES_PORT)
                  && (OAL_HOST2NET_SHORT(pst_udp_hdr->dest) == MAC_DHCP_UDP_SRC_PORT))) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}
/*lint +e695*/

/*
 * 函 数 名  : oal_netbuf_is_nd
 * 功能描述  : 判断是否是nd帧
 * 输入参数  : 无
 * 输出参数  : 无
 * 返 回 值  : 无
 */
oal_bool_enum_uint8 oal_netbuf_is_nd(oal_ipv6hdr_stru *pst_ipv6hdr)
{
    oal_icmp6hdr_stru *pst_icmp6hdr;

    if (OAL_UNLIKELY(pst_ipv6hdr == NULL)) {
        OAL_WARN_ON(1);
        return OAL_FALSE;
    }
    if (OAL_IPPROTO_ICMPV6 == pst_ipv6hdr->nexthdr) {
        pst_icmp6hdr = (oal_icmp6hdr_stru *)(pst_ipv6hdr + 1);

        if ((pst_icmp6hdr->icmp6_type == MAC_ND_RSOL) ||
            (pst_icmp6hdr->icmp6_type == MAC_ND_RADVT) ||
            (pst_icmp6hdr->icmp6_type == MAC_ND_NSOL) ||
            (pst_icmp6hdr->icmp6_type == MAC_ND_NADVT) ||
            (pst_icmp6hdr->icmp6_type == MAC_ND_RMES)) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}

/*
 * 函 数 名  : oal_netbuf_is_dhcp6
 * 功能描述  : 判断是否是DHCP6帧
 * 输入参数  : 无
 * 输出参数  : 无
 * 返 回 值  : 无
 */
oal_bool_enum_uint8 oal_netbuf_is_dhcp6(oal_ipv6hdr_stru *pst_ipv6hdr)
{
    oal_udp_header_stru *pst_udp_hdr;

    if (OAL_UNLIKELY(pst_ipv6hdr == NULL)) {
        OAL_WARN_ON(1);
        return OAL_FALSE;
    }
    if (pst_ipv6hdr->nexthdr == MAC_UDP_PROTOCAL) {
        pst_udp_hdr = (oal_udp_header_stru *)(pst_ipv6hdr + 1);

        if (((OAL_HOST2NET_SHORT(pst_udp_hdr->source) == MAC_IPV6_UDP_SRC_PORT)
             && (OAL_HOST2NET_SHORT(pst_udp_hdr->dest) == MAC_IPV6_UDP_DES_PORT))
            || ((OAL_HOST2NET_SHORT(pst_udp_hdr->source) == MAC_IPV6_UDP_DES_PORT)
                      && (OAL_HOST2NET_SHORT(pst_udp_hdr->dest) == MAC_IPV6_UDP_SRC_PORT))) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}

#ifdef _PRE_WLAN_FEATURE_FLOWCTL
/*
 * 函 数 名  : oal_netbuf_get_txtid
 * 功能描述  : 从lan过来报文的业务识别
 * 输入参数  : 无
 * 输出参数  : 无
 * 返 回 值  : 无
 */
oal_void oal_netbuf_get_txtid(oal_netbuf_stru *pst_buf, oal_uint8 *puc_tos)
{
    oal_ether_header_stru *pst_ether_header;
    oal_ip_header_stru *pst_ip;
    oal_vlan_ethhdr_stru *pst_vlan_ethhdr;
    oal_uint32 ul_ipv6_hdr;
    oal_uint32 ul_pri;
    oal_uint16 us_vlan_tci;
    oal_uint8 uc_tid = 0;
#ifdef _PRE_WLAN_FEATURE_SCHEDULE
    oal_tcp_header_stru *pst_tcp;
#endif

    if (OAL_UNLIKELY((pst_buf == NULL) || (puc_tos == NULL))) {
        OAL_WARN_ON(1);
        return;
    }

    /* 获取以太网头 */
    pst_ether_header = (oal_ether_header_stru *)oal_netbuf_data(pst_buf);

    switch (pst_ether_header->us_ether_type) {
            /*lint -e778*/ / * 屏蔽Info-- Constant expression evaluates to 0 in operation '&' * /
        case OAL_HOST2NET_SHORT(ETHER_TYPE_IP) :
            /* 从IP TOS字段寻找优先级 */
            /* ----------------------------------------------------------------------
                tos位定义
             ----------------------------------------------------------------------
            | bit7~bit5 | bit4 |  bit3  |  bit2  |   bit1   | bit0 |
            | 包优先级  | 时延 | 吞吐量 | 可靠性 | 传输成本 | 保留 |
             ---------------------------------------------------------------------- */
            pst_ip = (oal_ip_header_stru *)(pst_ether_header + 1); /* 偏移一个以太网头，取ip头 */

            uc_tid = pst_ip->uc_tos >> WLAN_IP_PRI_SHIFT;
#ifdef _PRE_WLAN_FEATURE_SCHEDULE
            /* 对于chariot信令报文进行特殊处理，防止断流 */
            if (pst_ip->uc_protocol == MAC_TCP_PROTOCAL) {
                pst_tcp = (oal_tcp_header_stru *)(pst_ip + 1);

                if ((OAL_HOST2NET_SHORT(MAC_CHARIOT_NETIF_PORT) == pst_tcp->us_dport)
                    || (OAL_HOST2NET_SHORT(MAC_CHARIOT_NETIF_PORT) == pst_tcp->us_sport)) {
                    uc_tid = WLAN_TIDNO_VOICE;
                }
            }
#endif
            break;

        case OAL_HOST2NET_SHORT(ETHER_TYPE_IPV6):
            /* 从IPv6 traffic class字段获取优先级 */
            /* ----------------------------------------------------------------------
                IPv6包头 前32为定义
             -----------------------------------------------------------------------
            | 版本号 | traffic class   | 流量标识 |
            | 4bit   | 8bit(同ipv4 tos)|  20bit   |
            ----------------------------------------------------------------------- */
            ul_ipv6_hdr = *((oal_uint32 *)(pst_ether_header + 1)); /* 偏移一个以太网头，取ip头 */

            ul_pri = (OAL_NET2HOST_LONG(ul_ipv6_hdr) & WLAN_IPV6_PRIORITY_MASK) >> WLAN_IPV6_PRIORITY_SHIFT;

            uc_tid = (oal_uint8)(ul_pri >> WLAN_IP_PRI_SHIFT);

            break;

        case OAL_HOST2NET_SHORT(ETHER_TYPE_PAE):
            uc_tid = WLAN_DATA_VIP_TID;
            break;

#ifdef _PRE_WLAN_FEATURE_WAPI
        case OAL_HOST2NET_SHORT(ETHER_TYPE_WAI):
            uc_tid = WLAN_DATA_VIP_TID;
            break;
#endif

        case OAL_HOST2NET_SHORT(ETHER_TYPE_VLAN):
            /* 获取vlan tag的优先级 */
            pst_vlan_ethhdr = (oal_vlan_ethhdr_stru *)oal_netbuf_data(pst_buf);

            /* ------------------------------------------------------------------
                802.1Q(VLAN) TCI(tag control information)位定义
             -------------------------------------------------------------------
            |Priority | DEI  | Vlan Identifier |
            | 3bit    | 1bit |      12bit      |
             ------------------------------------------------------------------ */
            us_vlan_tci = OAL_NET2HOST_SHORT(pst_vlan_ethhdr->h_vlan_TCI);

            uc_tid = us_vlan_tci >> OAL_VLAN_PRIO_SHIFT; /* 右移13位，提取高3位优先级 */

            break;

        /*lint +e778*/
        default:
            break;
    }

    /* 出参赋值 */
    *puc_tos = uc_tid;

    return;
}
#endif

/*
 * 函 数 名  : oal_netbuf_is_tcp_ack
 * 功能描述  : 判断ipv4 tcp报文是否为tcp 关键帧，包括TCP ACK\TCP SYN帧等
 * 输入参数  : 无
 * 输出参数  : 无
 * 返 回 值  : 无
 */
oal_bool_enum_uint8 oal_netbuf_is_tcp_ack(oal_ip_header_stru *pst_ip_hdr)
{
    oal_tcp_header_stru *pst_tcp_hdr;
    oal_uint32 ul_ip_pkt_len;
    oal_uint32 ul_ip_hdr_len;
    oal_uint32 ul_tcp_hdr_len;

    if (OAL_UNLIKELY(pst_ip_hdr == NULL)) {
        OAL_WARN_ON(1);
        return OAL_FALSE;
    }

    pst_tcp_hdr = (oal_tcp_header_stru *)(pst_ip_hdr + 1);
    ul_ip_pkt_len = OAL_NET2HOST_SHORT(pst_ip_hdr->us_tot_len);
    ul_ip_hdr_len = (OAL_LOW_HALF_BYTE(pst_ip_hdr->us_ihl)) << 2;
    ul_tcp_hdr_len = (OAL_HIGH_HALF_BYTE(pst_tcp_hdr->uc_offset)) << 2;

    if (ul_tcp_hdr_len + ul_ip_hdr_len == ul_ip_pkt_len) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*
 * 函 数 名  : oal_netbuf_is_icmp
 * 功能描述  : 判断ipv4 是否为icmp报文
 * 输入参数  : pst_ip_hdr: IP报文头部
 * 输出参数  : 无
 * 返 回 值  : 无
 */
oal_bool_enum_uint8 oal_netbuf_is_icmp(oal_ip_header_stru *pst_ip_hdr)
{
    oal_uint8 uc_protocol;

    if (OAL_UNLIKELY(pst_ip_hdr == NULL)) {
        OAL_WARN_ON(1);
        return OAL_FALSE;
    }

    uc_protocol = pst_ip_hdr->uc_protocol;

    /* ICMP报文检查 */
    if (uc_protocol == MAC_ICMP_PROTOCAL) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

#ifdef _PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL

/*
 * 函 数 名  : oal_netbuf_is_tcp_ack6
 * 功能描述  : 判断ipv6 tcp报文是否为tcp ack
 * 输入参数  : 无
 * 输出参数  : 无
 * 返 回 值  : 无
 */
oal_bool_enum_uint8 oal_netbuf_is_tcp_ack6(oal_ipv6hdr_stru *pst_ipv6hdr)
{
    oal_tcp_header_stru *pst_tcp_hdr;
    oal_uint32 ul_ip_pkt_len;
    oal_uint32 ul_tcp_hdr_len;

    if (OAL_UNLIKELY(pst_ipv6hdr == NULL)) {
        OAL_WARN_ON(1);
        return OAL_FALSE;
    }

    pst_tcp_hdr = (oal_tcp_header_stru *)(pst_ipv6hdr + 1);
    ul_ip_pkt_len = OAL_NET2HOST_SHORT(pst_ipv6hdr->payload_len); /* ipv6 净载荷, ipv6报文头部固定为40字节 */
    ul_tcp_hdr_len = (OAL_HIGH_HALF_BYTE(pst_tcp_hdr->uc_offset)) << 2;

    if (ul_tcp_hdr_len == ul_ip_pkt_len) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*
 * 函 数 名  : oal_netbuf_select_queue
 * 功能描述  : 为以太网来包选择合的subqueue入队
 * 输入参数  : 无
 * 输出参数  : 无
 * 返 回 值  : 无
 */
oal_uint16 oal_netbuf_select_queue(oal_netbuf_stru *pst_buf)
{
    oal_ether_header_stru *pst_ether_header;
    oal_ip_header_stru *pst_ip;
    oal_ipv6hdr_stru *pst_ipv6;
    oal_udp_header_stru *pst_udp_hdr;
#ifdef _PRE_WLAN_FEATURE_SCHEDULE
    oal_tcp_header_stru *pst_tcp_hdr;
#endif
    oal_vlan_ethhdr_stru *pst_vlan_ethhdr;
    oal_uint32 ul_ipv6_hdr;
    oal_uint32 ul_pri;
    oal_uint16 us_vlan_tci;
    oal_uint8 uc_tos;
    oal_uint8 us_queue = WLAN_NORMAL_QUEUE;

    if (OAL_UNLIKELY(pst_buf == NULL)) {
        OAL_WARN_ON(1);
        return WLAN_NET_QUEUE_BUTT;
    }

    /* 获取以太网头 */
    pst_ether_header = (oal_ether_header_stru *)oal_netbuf_data(pst_buf);

    switch (pst_ether_header->us_ether_type) {
            /*lint -e778*/ /* 屏蔽Info-- Constant expression evaluates to 0 in operation '&' */
        case OAL_HOST2NET_SHORT(ETHER_TYPE_IP):

            pst_ip = (oal_ip_header_stru *)(pst_ether_header + 1); /* 偏移一个以太网头，取ip头 */

            /* 对udp报文区分qos入队 */
            if (pst_ip->uc_protocol == MAC_UDP_PROTOCAL) {
                /* 从IP TOS字段寻找优先级 */
                /* ----------------------------------------------------------------------
                        tos位定义
                     ----------------------------------------------------------------------
                    | bit7~bit5 | bit4 |  bit3  |  bit2  |   bit1   | bit0 |
                    | 包优先级  | 时延 | 吞吐量 | 可靠性 | 传输成本 | 保留 |
                     ---------------------------------------------------------------------- */
                uc_tos = pst_ip->uc_tos >> WLAN_IP_PRI_SHIFT;
                us_queue = WLAN_TOS_TO_HCC_QUEUE(uc_tos);

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

        case OAL_HOST2NET_SHORT(ETHER_TYPE_IPV6):

            /* 从IPv6 traffic class字段获取优先级 */
            /* ----------------------------------------------------------------------
                IPv6包头 前32为定义
             -----------------------------------------------------------------------
            | 版本号 | traffic class   | 流量标识 |
            | 4bit   | 8bit(同ipv4 tos)|  20bit   |
            ----------------------------------------------------------------------- */
            pst_ipv6 = (oal_ipv6hdr_stru *)(pst_ether_header + 1); /* 偏移一个以太网头，取ip头 */
            ul_ipv6_hdr = (*(oal_uint32 *)pst_ipv6);

            if (pst_ipv6->nexthdr == MAC_UDP_PROTOCAL) { /* UDP报文 */
                ul_pri = (OAL_NET2HOST_LONG(ul_ipv6_hdr) & WLAN_IPV6_PRIORITY_MASK) >> WLAN_IPV6_PRIORITY_SHIFT;
                uc_tos = (oal_uint8)(ul_pri >> WLAN_IP_PRI_SHIFT);
                us_queue = WLAN_TOS_TO_HCC_QUEUE(uc_tos);
            } else if (pst_ipv6->nexthdr == MAC_TCP_PROTOCAL) { /* TCP报文 */
                if (oal_netbuf_is_tcp_ack6(pst_ipv6) == OAL_TRUE) {
                    us_queue = WLAN_TCP_ACK_QUEUE;
                } else {
                    us_queue = WLAN_TCP_DATA_QUEUE;
                }
            }

            /* 如果是DHCPV6帧，则进入WLAN_DATA_VIP_QUEUE队列缓存 */
            else if (oal_netbuf_is_dhcp6((oal_ipv6hdr_stru *)(pst_ether_header + 1)) == OAL_TRUE) {
                us_queue = WLAN_DATA_VIP_QUEUE;
            }
            break;

        case OAL_HOST2NET_SHORT(ETHER_TYPE_PAE):
            /* 如果是EAPOL帧，则进入VO队列发送 */
            us_queue = WLAN_DATA_VIP_QUEUE;
            break;

        /* TDLS帧处理，建链保护，入高优先级TID队列 */
        case OAL_HOST2NET_SHORT(ETHER_TYPE_TDLS):
            us_queue = WLAN_DATA_VIP_QUEUE;
            break;

        /* PPPOE帧处理，建链保护(发现阶段, 会话阶段)，入高优先级TID队列 */
        case OAL_HOST2NET_SHORT(ETHER_TYPE_PPP_DISC):
        case OAL_HOST2NET_SHORT(ETHER_TYPE_PPP_SES):
            us_queue = WLAN_DATA_VIP_QUEUE;
            break;

#ifdef _PRE_WLAN_FEATURE_WAPI
        case OAL_HOST2NET_SHORT(ETHER_TYPE_WAI):
            us_queue = WLAN_DATA_VIP_QUEUE;
            break;
#endif

        case OAL_HOST2NET_SHORT(ETHER_TYPE_VLAN):

            /* 获取vlan tag的优先级 */
            pst_vlan_ethhdr = (oal_vlan_ethhdr_stru *)oal_netbuf_data(pst_buf);

            /* ------------------------------------------------------------------
                802.1Q(VLAN) TCI(tag control information)位定义
             -------------------------------------------------------------------
            |Priority | DEI  | Vlan Identifier |
            | 3bit    | 1bit |      12bit      |
             ------------------------------------------------------------------ */
            us_vlan_tci = OAL_NET2HOST_SHORT(pst_vlan_ethhdr->h_vlan_TCI);

            uc_tos = us_vlan_tci >> OAL_VLAN_PRIO_SHIFT; /* 右移13位，提取高3位优先级 */
            us_queue = WLAN_TOS_TO_HCC_QUEUE(uc_tos);

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

#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && defined(_PRE_WLAN_FEATURE_DFR)

struct dev_excp_globals {
    oal_sock_stru *nlsk;
    oal_uint8 mode;
    oal_uint8 *data;
    oal_uint32 usepid;
};

struct dev_netlink_msg_hdr_stru {
    oal_uint32 cmd;
    oal_uint32 len;
};

struct dev_excp_globals dev_excp_handler_data;

/*
 * Prototype    : dev_netlink_rev
 * Description  : receive netlink date from app
 * Input        : void
 * Return Value : void
 */
void dev_netlink_rev(oal_netbuf_stru *skb)
{
    oal_netbuf_stru *pst_skb;
    oal_nlmsghdr_stru *pst_nlh;
    struct dev_netlink_msg_hdr_stru st_msg_hdr;
    oal_uint32 ul_len;
    oal_int32 ret;

    if (skb == NULL) {
        OAL_IO_PRINT("WIFI DFR:para fail\n");
        return;
    }

    OAL_IO_PRINT("WIFI DFR:dev_kernel_netlink_recv.\n");

    pst_skb = OAL_PTR_NULL;
    pst_nlh = OAL_PTR_NULL;

    memset_s(dev_excp_handler_data.data, OAL_EXCP_DATA_BUF_LEN, 0, OAL_EXCP_DATA_BUF_LEN);
    pst_skb = oal_netbuf_get(skb);
    if (pst_skb->len >= OAL_NLMSG_SPACE(0)) {
        pst_nlh = oal_nlmsg_hdr(pst_skb);
        /* 检测报文长度正确性 */
        if (!OAL_NLMSG_OK(pst_nlh, pst_skb->len)) {
            OAL_IO_PRINT("[ERROR]invaild netlink buff data packge data len = :%u,skb_buff data len = %u\n",
                         pst_nlh->nlmsg_len, pst_skb->len);
            kfree_skb(pst_skb);
            return;
        }
        ul_len = OAL_NLMSG_PAYLOAD(pst_nlh, 0);
        /* 后续需要拷贝sizeof(st_msg_hdr),故判断之 */
        if (ul_len < sizeof(st_msg_hdr)) {
            OAL_IO_PRINT("[ERROR]invaild netlink buff len:%u,max len:%u\n", ul_len, OAL_EXCP_DATA_BUF_LEN);
            kfree_skb(pst_skb);
            return;
        }

        ret = memcpy_s(dev_excp_handler_data.data, OAL_EXCP_DATA_BUF_LEN, OAL_NLMSG_DATA(pst_nlh), ul_len);
        if (ret != EOK) {
            OAL_IO_PRINT("memcpy_s error, destlen=%u, srclen=%u\n", OAL_EXCP_DATA_BUF_LEN, ul_len);
            kfree_skb(pst_skb);
            return;
        }

        ret = memcpy_s((void *)&st_msg_hdr, sizeof(st_msg_hdr), dev_excp_handler_data.data, sizeof(st_msg_hdr));
        if (ret != EOK) {
            OAL_IO_PRINT("memcpy_s error\n");
            kfree_skb(pst_skb);
            return;
        }

        if (0 == st_msg_hdr.cmd) {
            dev_excp_handler_data.usepid = pst_nlh->nlmsg_pid; /* pid of sending process */
            OAL_IO_PRINT("WIFI DFR:pid is [%d], msg is [%s]\n",
                         dev_excp_handler_data.usepid, &dev_excp_handler_data.data[sizeof(st_msg_hdr)]);
        }
    }
    kfree_skb(pst_skb);

    return;
}

/*
 * Prototype    : dev_netlink_create
 * Description  : create netlink for device exception
 * Input        : void
 * Return Value : int
 */

oal_int32 dev_netlink_create(void)
{
    dev_excp_handler_data.nlsk = oal_netlink_kernel_create(&OAL_INIT_NET, NETLINK_DEV_ERROR, 0,
                                                           dev_netlink_rev, OAL_PTR_NULL, OAL_THIS_MODULE);
    if (dev_excp_handler_data.nlsk == OAL_PTR_NULL) {
        OAL_IO_PRINT("WIFI DFR:fail to create netlink socket \n");
        return -OAL_EFAIL;
    }

    OAL_IO_PRINT("WIFI DFR:suceed to create netlink socket %p \n", dev_excp_handler_data.nlsk);
    return OAL_SUCC;
}

/*
 * Prototype    : dev_netlink_send
 * Description  : send netlink data
 * Input        : void
 * Return Value : int
 */
oal_int32 dev_netlink_send(oal_uint8 *data, oal_int data_len)
{
    oal_netbuf_stru *skb;
    oal_nlmsghdr_stru *nlh;
    oal_uint32 ret;
    oal_uint len;

    if (OAL_UNLIKELY(data == NULL)) {
        OAL_WARN_ON(1);
        return -OAL_EFAIL;
    }

    if (data_len < 0) {
        OAL_IO_PRINT("WIFI DFR:dev error:data_len[%ld].\n", data_len);
        return -OAL_EFAIL;
    }

    len = OAL_NLMSG_SPACE(((oal_uint)data_len));
    skb = alloc_skb(len, GFP_KERNEL);
    if (skb == NULL) {
        OAL_IO_PRINT("WIFI DFR:dev error: allocate failed, len[%lu].\n", len);
        return -OAL_EFAIL;
    }
    nlh = oal_nlmsg_put(skb, 0, 0, 0, data_len, 0);

    OAL_IO_PRINT("WIFI DFR: data[%s].\n", data);

    ret = memcpy_s(OAL_NLMSG_DATA(nlh), data_len, data, data_len);
    if (ret != EOK) {
        OAL_IO_PRINT("WIFI DFR: buf data load failed.\n");
        kfree_skb(skb);
        return -OAL_EFAIL;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
    OAL_NETLINK_CB(skb).portid = 0; /* from kernel */
#endif
    if (dev_excp_handler_data.nlsk == OAL_PTR_NULL) {
        OAL_IO_PRINT("WIFI DFR: NULL Pointer_sock.\n");
        kfree_skb(skb);
        return -OAL_EFAIL;
    }

    ret = oal_netlink_unicast(dev_excp_handler_data.nlsk, skb, dev_excp_handler_data.usepid, MSG_DONTWAIT);
    if (ret <= 0) {
        OAL_IO_PRINT("WIFI DFR:send dev error netlink msg, ret = %d \n", ret);
    }

    return ret;
}

/*
 * Prototype    : init_dev_excp_handler
 * Description  : init dev exception handler
 * Input        : void
 * Return Value : int
 */
oal_int32 init_dev_excp_handler(oal_void)
{
    oal_int32 ret;

    OAL_IO_PRINT("DFR: into init_exception_enable_handler\n");

    memset_s((oal_uint8 *)&dev_excp_handler_data, OAL_SIZEOF(dev_excp_handler_data),
             0, OAL_SIZEOF(dev_excp_handler_data));

    dev_excp_handler_data.data = (oal_uint8 *)kzalloc(OAL_EXCP_DATA_BUF_LEN, GFP_KERNEL);
    if (dev_excp_handler_data.data == OAL_PTR_NULL) {
        OAL_IO_PRINT("DFR: alloc dev_excp_handler_data.puc_data fail, len = %d.\n", OAL_EXCP_DATA_BUF_LEN);
        dev_excp_handler_data.data = OAL_PTR_NULL;
        return -OAL_EFAIL;
    }

    memset_s(dev_excp_handler_data.data, OAL_EXCP_DATA_BUF_LEN, 0, OAL_EXCP_DATA_BUF_LEN);

    ret = dev_netlink_create();
    if (ret < 0) {
        kfree(dev_excp_handler_data.data);
        OAL_IO_PRINT("init_dev_err_kernel init is ERR!\n");
        return -OAL_EFAIL;
    }

    OAL_IO_PRINT("DFR: init_exception_enable_handler init ok.\n");

    return OAL_SUCC;
}

/*
 * Prototype    : deinit_dev_excp_handler
 * Description  : deinit dev exception handler
 * Input        : void
 * Return Value : void
 */
oal_void deinit_dev_excp_handler(oal_void)
{
    if (dev_excp_handler_data.nlsk != OAL_PTR_NULL) {
        oal_netlink_kernel_release(dev_excp_handler_data.nlsk);
        dev_excp_handler_data.usepid = 0;
    }

    if (dev_excp_handler_data.data != OAL_PTR_NULL) {
        kfree(dev_excp_handler_data.data);
    }

    OAL_IO_PRINT("DFR: deinit ok.\n");

    return;
}
#else
oal_int32 dev_netlink_create(void)
{
    return OAL_SUCC;
}

oal_int32 dev_netlink_send(oal_uint8 *data, oal_int data_len)
{
    return OAL_SUCC;
}
oal_int32 init_dev_excp_handler(oal_void)
{
    return OAL_SUCC;
}
oal_void deinit_dev_excp_handler(oal_void)
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

#ifdef _PRE_WLAN_FEATURE_FLOWCTL
oal_module_symbol(oal_netbuf_get_txtid);
#endif

oal_module_symbol(init_dev_excp_handler);
oal_module_symbol(deinit_dev_excp_handler);

/*lint +e19*/

