
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER

/* 1 头文件包含 */
#include "mac_data.h"
#include "hmac_resource.h"
#include "hmac_host_tx_data.h"
#include "hmac_tcp_ack_buf.h"
#include "hmac_tx_data.h"
#include "hmac_config.h"
#include "wlan_chip_i.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TCP_ACK_BUF_C

/* 2 全局变量定义 */
OAL_STATIC hmac_tcp_ack_buf_para_stru g_tcp_ack_buf_para;
OAL_STATIC hmac_tcp_ack_buf_stru g_tcp_ack_buf;



uint32_t hmac_tcp_ack_buff_config_1103(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_send_event(mac_vap, WLAN_CFGID_TCP_ACK_BUF, len, param);
}


uint32_t hmac_tcp_ack_buff_config_1106(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    mac_cfg_tcp_ack_buf_stru *tcp_ack_param = (mac_cfg_tcp_ack_buf_stru *)param;

    switch (tcp_ack_param->en_cmd) {
        case MAC_TCP_ACK_BUF_ENABLE:
            g_tcp_ack_buf_para.tcp_ack_buf[mac_vap->uc_vap_id] = tcp_ack_param->en_enable;
            break;
        case MAC_TCP_ACK_BUF_TIMEOUT:
            g_tcp_ack_buf_para.tcp_ack_timeout = tcp_ack_param->uc_timeout_ms;
            break;
        case MAC_TCP_ACK_BUF_MAX:
            g_tcp_ack_buf_para.tcp_ack_max_num = tcp_ack_param->uc_count_limit;
            break;
        default:
            break;
    }

    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_set_tcp_ack_buf::en_tcp_ack_buf=%d, uc_tcp_ack_timeout=%d, us_tcp_ack_max_num=%d}",
                     g_tcp_ack_buf_para.tcp_ack_buf[mac_vap->uc_vap_id],
                     g_tcp_ack_buf_para.tcp_ack_timeout,
                     g_tcp_ack_buf_para.tcp_ack_max_num);
    return OAL_SUCC;
}

uint32_t hmac_config_tcp_ack_buf(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return wlan_chip_config_tcp_ack_buf(mac_vap, len, param);
}


void hmac_tcp_ack_buf_free(hmac_tcp_ack_buf_stru *tcp_ack_buf)
{
    oal_netbuf_stru *netbuf = NULL;
    oal_spin_lock_bh(&tcp_ack_buf->spin_lock);
    /* 清空tcp ack缓存描述符 */
    while (oal_netbuf_list_empty(&tcp_ack_buf->hdr) != OAL_TRUE) {
        netbuf = oal_netbuf_delist(&(tcp_ack_buf->hdr));
        oal_netbuf_free(netbuf);
    }

    tcp_ack_buf->tcp_ack_num = 0;
    oal_spin_unlock_bh(&tcp_ack_buf->spin_lock);
}

void hmac_tcp_ack_buf_init_para()
{
    hmac_tcp_ack_buf_para_stru *tcp_ack_buf = &g_tcp_ack_buf_para;

    memset_s(tcp_ack_buf, sizeof(hmac_tcp_ack_buf_para_stru), 0, sizeof(hmac_tcp_ack_buf_para_stru));

    tcp_ack_buf->tcp_ack_max_num = TCP_ACK_BUF_MAX_NUM;
    tcp_ack_buf->tcp_ack_timeout = TCP_ACK_TIMEOUT;
}


void hmac_tcp_ack_buf_init_user(hmac_user_stru *hmac_user)
{
    hmac_tcp_ack_buf_stru *tcp_ack_buf = &g_tcp_ack_buf;
    mac_chip_stru *mac_chip = NULL;
    /* multi user不申请 */
    if (hmac_user->st_user_base_info.en_is_multi_user == OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_TX, "{hmac_tcp_ack_buf_init_user::is_multi_user}");
        return;
    }
    mac_chip = hmac_res_get_mac_chip(hmac_user->st_user_base_info.uc_chip_id);
    if (mac_chip == NULL) {
        return;
    }
    /* 非单用户不申请 */
    if (mac_chip->uc_assoc_user_cnt != 0) {
        oam_warning_log0(0, OAM_SF_TX, "{hmac_tcp_ack_buf_init_user::uc_assoc_user_cnt != 0}");
        return;
    }
    hmac_user->tcp_ack_buf = tcp_ack_buf;
    memset_s(tcp_ack_buf, sizeof(hmac_tcp_ack_buf_stru), 0, sizeof(hmac_tcp_ack_buf_stru));
    oal_spin_lock_init(&tcp_ack_buf->spin_lock);
    oal_netbuf_list_head_init(&tcp_ack_buf->hdr);
}

void hmac_tcp_ack_buf_exit_user(hmac_user_stru *hmac_user)
{
    hmac_tcp_ack_buf_stru *tcp_ack_buf = (hmac_tcp_ack_buf_stru *)hmac_user->tcp_ack_buf;

    if (tcp_ack_buf != NULL) {
        // 删除定时器
        if (tcp_ack_buf->tcp_ack_timer.en_is_registerd == OAL_TRUE) {
            frw_timer_immediate_destroy_timer_m(&(tcp_ack_buf->tcp_ack_timer));
        }

        /* 清空tcp ack缓存链表 */
        hmac_tcp_ack_buf_free(tcp_ack_buf);

        memset_s(tcp_ack_buf, sizeof(hmac_tcp_ack_buf_stru), 0, sizeof(hmac_tcp_ack_buf_stru));

        hmac_user->tcp_ack_buf = NULL;
    }
}

uint32_t hmac_tx_tcp_ack_buf_send(void *p_arg)
{
    hmac_tcp_ack_buf_stru *tcp_ack_buf = NULL;
    hmac_user_stru *hmac_user = (hmac_user_stru *)p_arg;
    oal_netbuf_stru *netbuf = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    oal_netbuf_head_stru st_head_t;
    uint32_t netbuf_num = 0;

    tcp_ack_buf = (hmac_tcp_ack_buf_stru *)hmac_user->tcp_ack_buf;
    if (tcp_ack_buf == NULL) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_tx_tcp_ack_buf_send::tcp_ack_buf is NULL }");
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_vap = tcp_ack_buf->hmac_vap;
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_tx_tcp_ack_buf_send::hmac_vap is NULL }");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 自旋锁 */
    oal_spin_lock_bh(&tcp_ack_buf->spin_lock);
    oal_netbuf_head_init(&st_head_t);
    oal_netbuf_queue_splice_tail_init(&tcp_ack_buf->hdr, &st_head_t);
    tcp_ack_buf->tcp_ack_num = 0;
    oal_spin_unlock_bh(&tcp_ack_buf->spin_lock);
    while (oal_netbuf_list_empty(&st_head_t) != OAL_TRUE) {
        netbuf = oal_netbuf_delist_nolock(&(st_head_t));
        if (oal_unlikely(netbuf == NULL)) {
            return OAL_SUCC;
        }
#ifdef CONFIG_ARCH_KIRIN_PCIE
        if (hmac_host_tx_data(&hmac_vap->st_vap_base_info, hmac_user, netbuf) != OAL_SUCC) {
            oal_netbuf_free(netbuf);
            oam_error_log1(0, OAM_SF_TX, "{hmac_tx_tcp_ack_buf_send::netbuf_num=%d }", netbuf_num);
            continue;
        }
#endif
        netbuf_num++;
    }
    return OAL_SUCC;
}


OAL_STATIC oal_bool_enum_uint8 hmac_tcp_ack_check_need_buf(oal_netbuf_stru *netbuf)
{
    oal_ip_header_stru *ip = NULL;
    oal_ether_header_stru *eth_hdr = NULL;
    // 只有tcp ack帧才能走入后面的处理
    eth_hdr = (oal_ether_header_stru *)oal_netbuf_data(netbuf);
    if (oal_host2net_short(ETHER_TYPE_IP) != eth_hdr->us_ether_type) {
        return OAL_FALSE;
    }

    ip = (oal_ip_header_stru *)(eth_hdr + 1);
    if (ip->uc_protocol != MAC_TCP_PROTOCAL) {
        return OAL_FALSE;
    }

    if (oal_netbuf_is_tcp_ack(ip) != OAL_TRUE) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

uint32_t hmac_tx_tcp_ack_buf_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf)
{
    hmac_tcp_ack_buf_stru *tcp_ack_buf = NULL;
    hmac_tcp_ack_buf_para_stru *tcp_ack_buf_para = &g_tcp_ack_buf_para;

    if (tcp_ack_buf_para->tcp_ack_buf[hmac_vap->st_vap_base_info.uc_vap_id] == OAL_FALSE) {
        return HMAC_TX_PASS;
    }

    tcp_ack_buf = (hmac_tcp_ack_buf_stru *)hmac_user->tcp_ack_buf;
    if (tcp_ack_buf == NULL) {
        return HMAC_TX_PASS;
    }

    if (hmac_tcp_ack_check_need_buf(netbuf) != OAL_TRUE) {
        return HMAC_TX_PASS;
    }

    if (tcp_ack_buf->tcp_ack_num == 0) {
        /* 启动定时器 */
        tcp_ack_buf->hmac_vap = hmac_vap;
        frw_timer_create_timer_m(&tcp_ack_buf->tcp_ack_timer, hmac_tx_tcp_ack_buf_send,
            tcp_ack_buf_para->tcp_ack_timeout, hmac_user, OAL_TRUE, OAM_MODULE_ID_HMAC,
            hmac_vap->st_vap_base_info.core_id);
    }

    // 直接缓存
    oal_spin_lock_bh(&tcp_ack_buf->spin_lock);
    oal_netbuf_list_tail_nolock(&tcp_ack_buf->hdr, netbuf);
    tcp_ack_buf->tcp_ack_num++;
    oal_spin_unlock_bh(&tcp_ack_buf->spin_lock);
    oam_info_log2(0, OAM_SF_TX, "{hmac_tx_tcp_ack_buf_process::tcp_ack_num=%d, tcp_ack_max_num=%d }",
                  tcp_ack_buf->tcp_ack_num, tcp_ack_buf_para->tcp_ack_max_num);
    // 若已缓存的报文达到了上限，就发送
    if (tcp_ack_buf->tcp_ack_num >= tcp_ack_buf_para->tcp_ack_max_num) {
        /* 立即删除定时器 */
        tcp_ack_buf->hmac_vap = hmac_vap;
        frw_timer_immediate_destroy_timer_m(&tcp_ack_buf->tcp_ack_timer);
        hmac_tx_tcp_ack_buf_send(hmac_user);
    }
    return HMAC_TX_BUFF;
}

#endif

