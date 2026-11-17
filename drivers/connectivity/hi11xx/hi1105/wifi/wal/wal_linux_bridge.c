

#include "oam_ext_if.h"
#include "frw_ext_if.h"

#include "hmac_vap.h"
#include "mac_vap.h"
#include "mac_resource.h"
#include "mac_data.h"
#include "hmac_ext_if.h"

#include "wal_main.h"
#include "wal_linux_bridge.h"

#include "hmac_device.h"
#include "hmac_resource.h"

#ifdef _PRE_WLAN_RR_PERFORMENCE_DEBUG
#include "hmac_rr_performance.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_BRIDGE_C


uint8_t g_sk_pacing_shift = 8;
oal_net_dev_tx_enum wal_bridge_vap_xmit(oal_netbuf_stru *pst_buf, oal_net_device_stru *pst_dev)
{
    host_start_record_performance(TX_XMIT_PROC);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)) && defined(_PRE_WLAN_1103_TCP_SMALL_QUEUE_BUGFIX)

#if defined(_PRE_PRODUCT_HI1620S_KUNPENG) || defined(_PRE_PRODUCT_HI3751V811)

#else
    /* We need a bit of data queued to build aggregates properly, so
     * instruct the TCP stack to allow more than a single ms of data
     * to be queued in the stack. The value is a bit-shift of 1
     * second, so 6 is 16ms of queued data. Only affects local TCP UL
     * sockets.
     */
    sk_pacing_shift_update(pst_buf->sk, g_sk_pacing_shift);
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_GSO
    if (skb_linearize(pst_buf)) {
        oam_warning_log0(0, OAM_SF_RX, "{wal_bridge_vap_xmit::[GSO] failed at skb_linearize");
        dev_kfree_skb_any(pst_buf);
        return NETDEV_TX_OK;
    }
#endif
#ifdef _PRE_WLAN_RR_PERFORMENCE_DEBUG
    /* RR性能检测发送流程host起始位置打点 */
    hmac_rr_tx_w2h_timestamp();
#endif
    return hmac_bridge_vap_xmit(pst_buf, pst_dev);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

