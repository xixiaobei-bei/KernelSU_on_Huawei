

#ifndef __HMAC_EDCA_OPT_H__
#define __HMAC_EDCA_OPT_H__

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP

#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "mac_device.h"
#include "mac_common.h"
#include "oam_ext_if.h"

#define HMAC_EDCA_OPT_MIN_PKT_LEN 256 /* 小于该长度的ip报文不被统计，排除chariot控制报文 */

#define HMAC_EDCA_OPT_TIME_MS 30000 /* edca参数调整默认定时器 */

#define HMAC_EDCA_OPT_PKT_NUM ((HMAC_EDCA_OPT_TIME_MS) >> 3) /* 平均每毫秒报文个数 */

#define WLAN_EDCA_OPT_MAX_WEIGHT_STA (3)
#define WLAN_EDCA_OPT_WEIGHT_STA     (2)

#define HMAC_IDLE_CYCLE_TH (5) /* 连续空闲周期数门限 */

typedef enum {
    WLAN_TX_TCP_DATA = 0, /* 发送TCP data */
    WLAN_RX_TCP_DATA = 1, /* 接收TCP data */
    WLAN_TX_UDP_DATA = 2, /* 发送UDP data */
    WLAN_RX_UDP_DATA = 3, /* 接收UDP data */

    WLAN_TXRX_DATA_BUTT = 4,
} wlan_txrx_data_type_enum;

void hmac_edca_opt_rx_pkts_stat(uint16_t us_assoc_id, uint8_t uc_tidno, mac_ip_header_stru *pst_ip);
void hmac_edca_opt_tx_pkts_stat(mac_tx_ctl_stru *pst_tx_ctl, uint8_t uc_tidno, mac_ip_header_stru *pst_ip);
uint32_t hmac_edca_opt_timeout_fn(void *p_arg);
#endif /* end of _PRE_WLAN_FEATURE_EDCA_OPT_AP */

#endif /* end of hmac_edca_opt.h */
