

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_net.h"
#include "wlan_types.h"
#include "mac_vap.h"
#include "mac_data.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_DATA_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68
#define DNS_SERVER_PORT  53
/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_bool_enum_uint8 mac_is_dhcp_port(mac_ip_header_stru *pst_ip_hdr)
{
    udp_hdr_stru *pst_udp_hdr = OAL_PTR_NULL;
    /* DHCP判断标准: udp协议，ip头部fragment offset字段为0，目的端口号为67或68 */
    if ((pst_ip_hdr->uc_protocol == MAC_UDP_PROTOCAL) && ((pst_ip_hdr->us_frag_off & 0xFF1F) == 0)) {
        pst_udp_hdr = (udp_hdr_stru *)(pst_ip_hdr + 1);

        if ((OAL_NET2HOST_SHORT(pst_udp_hdr->us_des_port) == DHCP_SERVER_PORT)
            || (OAL_NET2HOST_SHORT(pst_udp_hdr->us_des_port) == DHCP_CLIENT_PORT)) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}


oal_bool_enum_uint8 mac_is_dns_frame(mac_ip_header_stru *pst_ip_hdr)
{
    udp_hdr_stru *pst_udp_hdr = OAL_PTR_NULL;
    /* DNS判断标准: udp协议，ip头部fragment offset字段为0，目的端口号为53 */
    if ((pst_ip_hdr->uc_protocol == MAC_UDP_PROTOCAL) && ((pst_ip_hdr->us_frag_off & 0xFF1F) == 0)) {
        pst_udp_hdr = (udp_hdr_stru *)(pst_ip_hdr + 1);

        if (OAL_NET2HOST_SHORT(pst_udp_hdr->us_des_port) == DNS_SERVER_PORT) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}


oal_bool_enum_uint8 mac_is_nd(oal_ipv6hdr_stru *pst_ipv6hdr)
{
    oal_icmp6hdr_stru *pst_icmp6hdr = OAL_PTR_NULL;

    if (pst_ipv6hdr->nexthdr == OAL_IPPROTO_ICMPV6) {
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


oal_bool_enum_uint8 mac_is_dhcp6(oal_ipv6hdr_stru *pst_ipv6hdr)
{
    udp_hdr_stru *pst_udp_hdr = OAL_PTR_NULL;

    if (pst_ipv6hdr->nexthdr == MAC_UDP_PROTOCAL) {
        pst_udp_hdr = (udp_hdr_stru *)(pst_ipv6hdr + 1);

        if ((pst_udp_hdr->us_des_port == OAL_HOST2NET_SHORT(MAC_IPV6_UDP_DES_PORT))
            || (pst_udp_hdr->us_des_port == OAL_HOST2NET_SHORT(MAC_IPV6_UDP_SRC_PORT))) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}

mac_data_type_enum_uint8 mac_get_arp_type_by_arphdr(oal_eth_arphdr_stru *pst_rx_arp_hdr)
{
    if (OAL_NET2HOST_SHORT(pst_rx_arp_hdr->us_ar_op) == MAC_ARP_REQUEST) {
        return MAC_DATA_ARP_REQ;
    } else if (OAL_NET2HOST_SHORT(pst_rx_arp_hdr->us_ar_op) == MAC_ARP_RESPONSE) {
        return MAC_DATA_ARP_RSP;
    }

    return MAC_DATA_BUTT;
}


oal_uint8 mac_get_data_type_from_8023(oal_uint8 *puc_frame_hdr, mac_netbuff_payload_type uc_hdr_type)
{
    mac_ip_header_stru *pst_ip = OAL_PTR_NULL;
    oal_uint8 *puc_frame_body = OAL_PTR_NULL;
    oal_uint16 us_ether_type;
    oal_uint8 uc_datatype = MAC_DATA_BUTT;

    if (puc_frame_hdr == OAL_PTR_NULL) {
        return uc_datatype;
    }

    if (uc_hdr_type == MAC_NETBUFF_PAYLOAD_ETH) {
        us_ether_type = ((mac_ether_header_stru *)puc_frame_hdr)->us_ether_type;
        puc_frame_body = puc_frame_hdr + (oal_uint16)OAL_SIZEOF(mac_ether_header_stru);
    } else if (uc_hdr_type == MAC_NETBUFF_PAYLOAD_SNAP) {
        us_ether_type = ((mac_llc_snap_stru *)puc_frame_hdr)->us_ether_type;
        puc_frame_body = puc_frame_hdr + (oal_uint16)OAL_SIZEOF(mac_llc_snap_stru);
    } else {
        return uc_datatype;
    }

    switch (us_ether_type) {
            /* lint -e778 */ /* 屏蔽Info -- Constant expression evaluates to 0 in operation '&' */
        case OAL_HOST2NET_SHORT(ETHER_TYPE_IP):
            /* 从IP TOS字段寻找优先级 */
            /* ----------------------------------------------------------------------
                tos位定义
             ----------------------------------------------------------------------
            | bit7~bit5 | bit4 |  bit3  |  bit2  |   bit1   | bit0 |
            | 包优先级  | 时延 | 吞吐量 | 可靠性 | 传输成本 | 保留 |
             ---------------------------------------------------------------------- */
            pst_ip = (mac_ip_header_stru *)puc_frame_body; /* 偏移一个以太网头，取ip头 */

            if (mac_is_dhcp_port(pst_ip) == OAL_TRUE) {
                uc_datatype = MAC_DATA_DHCP;
            } else if (pst_ip->uc_protocol == MAC_TCP_PROTOCAL) {
                mac_tcp_header_stru *pst_tcp_hdr;
                pst_tcp_hdr = (mac_tcp_header_stru *)(pst_ip + 1);
                if (oal_ntoh_16(pst_tcp_hdr->us_sport) == MAC_WFD_RTSP_PORT) {
                    return MAC_DATA_RTSP;
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
            /* 如果是ND帧，则进入VO队列发送 */
            if (mac_is_nd((oal_ipv6hdr_stru *)puc_frame_body) == OAL_TRUE) {
                uc_datatype = MAC_DATA_ND;
            } else if (mac_is_dhcp6((oal_ipv6hdr_stru *)puc_frame_body) == OAL_TRUE) {
                uc_datatype = MAC_DATA_DHCPV6;
            }
            break;

        case OAL_HOST2NET_SHORT(ETHER_TYPE_PAE):
            /* 如果是EAPOL帧，则进入VO队列发送 */
            uc_datatype = MAC_DATA_EAPOL; /* eapol */
            break;

        case OAL_HOST2NET_SHORT(ETHER_TYPE_WAI):
            uc_datatype = MAC_DATA_WAPI;
            break;

        case OAL_HOST2NET_SHORT(ETHER_TYPE_ARP):
            /* 如果是ARP帧，则进入VO队列发送 */
            uc_datatype = mac_get_arp_type_by_arphdr((oal_eth_arphdr_stru *)puc_frame_body);
            break;

        /*lint +e778*/
        default:
            uc_datatype = MAC_DATA_BUTT;
            break;
    }

    return uc_datatype;
}


oal_uint8 mac_get_data_type_from_80211(oal_netbuf_stru *pst_netbuff, oal_uint16 us_mac_hdr_len)
{
    oal_uint8 uc_datatype;
    mac_llc_snap_stru *pst_snap = OAL_PTR_NULL;

    if (pst_netbuff == OAL_PTR_NULL) {
        return MAC_DATA_BUTT;
    }

    pst_snap = (mac_llc_snap_stru *)(oal_netbuf_data(pst_netbuff) + us_mac_hdr_len);

    uc_datatype = mac_get_data_type_from_8023((oal_uint8 *)pst_snap, MAC_NETBUFF_PAYLOAD_SNAP);

    return uc_datatype;
}


oal_uint8 mac_get_data_type(oal_netbuf_stru *pst_netbuff)
{
    oal_uint8 uc_datatype;
    mac_llc_snap_stru *pst_snap = OAL_PTR_NULL;

    if (pst_netbuff == OAL_PTR_NULL) {
        return MAC_DATA_BUTT;
    }

    pst_snap = (mac_llc_snap_stru *)oal_netbuf_payload(pst_netbuff);
    if (pst_snap == OAL_PTR_NULL) {
        return MAC_DATA_BUTT;
    }

    uc_datatype = mac_get_data_type_from_8023((oal_uint8 *)pst_snap, MAC_NETBUFF_PAYLOAD_SNAP);

    return uc_datatype;
}


oal_uint16 mac_get_eapol_keyinfo(oal_netbuf_stru *pst_netbuff)
{
    oal_uint8 uc_datatype;
    oal_uint8 *puc_payload = OAL_PTR_NULL;

    uc_datatype = mac_get_data_type(pst_netbuff);
    if (uc_datatype != MAC_DATA_EAPOL) {
        return 0;
    }

    puc_payload = oal_netbuf_payload(pst_netbuff);
    if (puc_payload == OAL_PTR_NULL) {
        return 0;
    }
    return *(oal_uint16 *)(puc_payload + OAL_EAPOL_INFO_POS);
}


oal_uint8 mac_get_eapol_type(oal_netbuf_stru *pst_netbuff)
{
    oal_uint8 uc_datatype;
    oal_uint8 *puc_payload = OAL_PTR_NULL;

    uc_datatype = mac_get_data_type(pst_netbuff);
    if (uc_datatype != MAC_DATA_EAPOL) {
        return 0;
    }

    puc_payload = oal_netbuf_payload(pst_netbuff);
    if (puc_payload == OAL_PTR_NULL) {
        return 0;
    }
    return *(puc_payload + OAL_EAPOL_TYPE_POS);
}


oal_bool_enum_uint8 mac_is_eapol_key_ptk(mac_eapol_header_stru *pst_eapol_header)
{
    mac_eapol_key_stru *pst_key = OAL_PTR_NULL;

    if (pst_eapol_header->uc_type == IEEE802_1X_TYPE_EAPOL_KEY) {
        if ((oal_uint16)(OAL_NET2HOST_SHORT(pst_eapol_header->us_length)) >=
            (oal_uint16)OAL_SIZEOF(mac_eapol_key_stru)) {
            pst_key = (mac_eapol_key_stru *)(pst_eapol_header + 1);

            if (pst_key->auc_key_info[1] & WPA_KEY_INFO_KEY_TYPE) {
                return OAL_TRUE;
            }
        }
    }
    return OAL_FALSE;
}


oal_bool_enum_uint8 mac_is_eapol_key_ptk_4_4(oal_netbuf_stru *pst_netbuff)
{
    mac_eapol_header_stru *pst_eapol_header = OAL_PTR_NULL;
    mac_eapol_key_stru *pst_eapol_key = OAL_PTR_NULL;

    if ((mac_get_data_type(pst_netbuff) == MAC_DATA_EAPOL)) {
        pst_eapol_header = (mac_eapol_header_stru *)(oal_netbuf_payload(pst_netbuff) + OAL_SIZEOF(mac_llc_snap_stru));
        if (mac_is_eapol_key_ptk(pst_eapol_header) == OAL_TRUE) {
            pst_eapol_key = (mac_eapol_key_stru *)(pst_eapol_header + 1);
            if ((pst_eapol_key->auc_key_data_length[0] == 0)
                && (pst_eapol_key->auc_key_data_length[1] == 0)) {
                return OAL_TRUE;
            }
        }
    }

    return OAL_FALSE;
}
oal_bool_enum_uint8 mac_eapol_key_check_len(mac_eapol_key_stru *pst_eapol_key)
{
    if (pst_eapol_key->auc_key_data_length[0] != 0 || pst_eapol_key->auc_key_data_length[1] != 0) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

mac_eapol_type_enum_uint8 mac_get_eapol_key_type(oal_uint8 *pst_payload)
{
    mac_eapol_header_stru *pst_eapol_header;
    mac_eapol_key_stru *pst_eapol_key = OAL_PTR_NULL;
    oal_bool_enum_uint8 en_key_ack_set = OAL_FALSE;
    oal_bool_enum_uint8 en_key_mic_set = OAL_FALSE;
    oal_bool_enum_uint8 en_key_len_set = OAL_FALSE;
    mac_eapol_type_enum_uint8 en_eapol_type = MAC_EAPOL_PTK_BUTT;

    /* 调用此接口,需保证已默认识别Ethertype为0x888E, EAPOL */
    /* 入参payload为LLC或者Ether头后 */
    pst_eapol_header = (mac_eapol_header_stru *)(pst_payload);

    if (mac_is_eapol_key_ptk(pst_eapol_header) == OAL_TRUE) {
        pst_eapol_key = (mac_eapol_key_stru *)(pst_eapol_header + 1);

        if (pst_eapol_key->auc_key_info[1] & WPA_KEY_INFO_KEY_ACK) {
            en_key_ack_set = OAL_TRUE;
        }

        if (pst_eapol_key->auc_key_info[0] & WPA_KEY_INFO_KEY_MIC) {
            en_key_mic_set = OAL_TRUE;
        }

        if (mac_eapol_key_check_len(pst_eapol_key) == OAL_TRUE) {
            en_key_len_set = OAL_TRUE;
        }

        /* ack设置,表明STA发起期望响应 */
        if (en_key_ack_set == OAL_TRUE) {
            if ((en_key_mic_set == OAL_FALSE) && (en_key_len_set == OAL_FALSE)) {
                /* 1/4识别:ACK=1,MIC=0,LEN=0 */
                en_eapol_type = MAC_EAPOL_PTK_1_4;
            } else if ((en_key_mic_set == OAL_TRUE) && (en_key_len_set == OAL_TRUE)) {
                /* 3/4识别:ACK=1,MIC=1,LEN=1 */
                en_eapol_type = MAC_EAPOL_PTK_3_4;
            }
        } else {
            if ((en_key_mic_set == OAL_TRUE) && (en_key_len_set == OAL_TRUE)) {
                /* 2/4识别:ACK=0,MIC=0,LEN=1 */
                en_eapol_type = MAC_EAPOL_PTK_2_4;
            } else if ((en_key_mic_set == OAL_TRUE) && (en_key_len_set == OAL_FALSE)) {
                /* 4/4识别:ACK=0,MIC=1,LEN=0 */
                en_eapol_type = MAC_EAPOL_PTK_4_4;
            }
        }
    }

    return en_eapol_type;
}

oal_uint8 mac_get_dhcp_type(oal_uint8 *puc_pos, oal_uint8 *puc_packet_end)
{
    oal_uint8 *puc_opt = OAL_PTR_NULL;
    while ((puc_pos < puc_packet_end) && (*puc_pos != 0xFF)) {
        puc_opt = puc_pos++;
        if (*puc_opt == 0) {
            continue; /* Padding */
        }
        puc_pos += *puc_pos + 1;
        if (puc_pos >= puc_packet_end) {
            break;
        }
        if ((*puc_opt == 53) && (puc_opt[1] != 0)) { /* 53是dhcp otion, 1是下标，0表示为空 */
            return puc_opt[2]; /* 2是type的下标 */
        }
    }
    return 0xFF;  // unknow type
}


oal_uint8 mac_get_dhcp_frame_type(oal_ip_header_stru *pst_rx_ip_hdr)
{
    oal_uint8 *puc_pos;
    oal_dhcp_packet_stru *pst_rx_dhcp_hdr;
    oal_uint8 *puc_packet_end;

    puc_pos = (oal_uint8 *)pst_rx_ip_hdr;
    puc_pos += (puc_pos[0] & 0x0F) << 2; /* 2是左移两位，point udp header */
    pst_rx_dhcp_hdr = (oal_dhcp_packet_stru *)(puc_pos + 8); /* 8是dhcphdr偏移量 */

    puc_packet_end = (oal_uint8 *)pst_rx_ip_hdr + OAL_NET2HOST_SHORT(pst_rx_ip_hdr->us_tot_len);
    puc_pos = &(pst_rx_dhcp_hdr->options[4]); /* 4是下标 */

    return mac_get_dhcp_type(puc_pos, puc_packet_end);
}

oal_module_symbol(mac_is_dhcp_port);
oal_module_symbol(mac_is_dhcp6);
oal_module_symbol(mac_is_nd);
oal_module_symbol(mac_get_data_type_from_8023);
oal_module_symbol(mac_is_eapol_key_ptk);
oal_module_symbol(mac_get_data_type_from_80211);
oal_module_symbol(mac_get_data_type);
oal_module_symbol(mac_get_dhcp_frame_type);
oal_module_symbol(mac_get_eapol_key_type);


