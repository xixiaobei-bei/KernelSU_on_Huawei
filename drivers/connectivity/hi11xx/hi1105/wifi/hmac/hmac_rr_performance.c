

#ifdef _PRE_WLAN_RR_PERFORMENCE_DEBUG
#include "hmac_rr_performance.h"

/* 保存RR性能host打点 */
hmac_perform_host_timestamp_t g_host_rr_perform_timestamp;


uint64_t hmac_get_ktimestamp(void)
{
    ktime_t ktime;
    uint64_t time_us;

    ktime = ktime_get_real();
    time_us = (uint64_t)ktime_to_us(ktime);

    return time_us;
}

void hmac_rr_tx_w2h_timestamp(void)
{
    if (g_host_rr_perform_timestamp.rr_switch == OAL_SWITCH_ON) {
        g_host_rr_perform_timestamp.tx_w2hmac_timestamp = hmac_get_ktimestamp();
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_rr_tx_w2h_timestamp::tx_w2hmac_timestamp = %llu}\n",
                         g_host_rr_perform_timestamp.tx_w2hmac_timestamp);
    }
}

void hmac_rr_tx_h2d_timestamp(void)
{
    if (g_host_rr_perform_timestamp.rr_switch == OAL_SWITCH_ON) {
        g_host_rr_perform_timestamp.tx_hmac2d_timestamp = hmac_get_ktimestamp();
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_rr_tx_h2d_timestamp::tx_hmac2d_timestamp = %llu}\r\n",
                         g_host_rr_perform_timestamp.tx_hmac2d_timestamp);
    }
}

void hmac_rr_rx_d2h_timestamp(void)
{
    if (g_host_rr_perform_timestamp.rr_switch == OAL_SWITCH_ON) {
        g_host_rr_perform_timestamp.rx_d2hmac_timestamp = hmac_get_ktimestamp();
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_rr_rx_d2h_timestamp::rx_d2hmac_timestamp = %llu}\r\n",
                         g_host_rr_perform_timestamp.rx_d2hmac_timestamp);
    }
}

void hmac_rr_rx_h2w_timestamp(void)
{
    if (g_host_rr_perform_timestamp.rr_switch == OAL_SWITCH_ON) {
        g_host_rr_perform_timestamp.rx_hmac2w_timestamp = hmac_get_ktimestamp();
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_rr_rx_h2w_timestamp::rx_hmac2w_timestamp = %llu}\r\n",
                         g_host_rr_perform_timestamp.rx_hmac2w_timestamp);
    }
}
#endif
