
#ifndef HMAC_TCP_ACK_BUF_H
#define HMAC_TCP_ACK_BUF_H

#include "hmac_user.h"
#include "hmac_vap.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TCP_ACK_BUF_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 2 宏定义 */
#define TCP_ACK_BUF_MAX_NUM           20  // 默认缓存的最大tcp ack个数
#define TCP_ACK_TIMEOUT               10 // tcp cak缓存机制的定时器周期 ms

typedef struct {
    uint8_t tcp_ack_buf[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
    uint8_t tcp_ack_timeout;
    uint16_t tcp_ack_max_num;
} hmac_tcp_ack_buf_para_stru;
typedef struct {
    oal_netbuf_head_stru hdr;
    uint16_t tcp_ack_num;
    uint8_t resv[BYTE_OFFSET_2];
    hmac_vap_stru *hmac_vap;
    frw_timeout_stru tcp_ack_timer;
    oal_spin_lock_stru spin_lock;
} hmac_tcp_ack_buf_stru;

uint32_t hmac_tcp_ack_buff_config_1103(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
uint32_t hmac_tcp_ack_buff_config_1106(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
void hmac_tcp_ack_buf_init_user(hmac_user_stru *hmac_user);
void hmac_tcp_ack_buf_exit_user(hmac_user_stru *hmac_user);
uint32_t hmac_tx_tcp_ack_buf_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *netbuf);
void hmac_tcp_ack_buf_init_para(void);
uint32_t hmac_config_tcp_ack_buf(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

