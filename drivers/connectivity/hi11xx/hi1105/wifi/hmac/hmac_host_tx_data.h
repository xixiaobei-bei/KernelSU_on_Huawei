

#ifndef __HMAC_HOST_TX_DATA_H__
#define __HMAC_HOST_TX_DATA_H__

#include "mac_vap.h"
#include "hmac_vap.h"
#include "hmac_user.h"
#include "hmac_tid.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_HOST_TX_DATA_H

uint32_t hmac_host_tx(mac_vap_stru *mac_vap, oal_netbuf_stru *netbuf);
uint32_t hmac_tx_update_freq_timeout(void *arg);
uint32_t hmac_host_tx_tid_enqueue(
    hmac_tid_info_stru *hmac_tid_info, oal_netbuf_stru *netbuf, hmac_tid_operate_type_enum type);
uint32_t hmac_host_ring_tx(mac_vap_stru *mac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf);

OAL_STATIC OAL_INLINE uint32_t hmac_host_tx_data(
    mac_vap_stru *mac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf)
{
    uint8_t tid;
    uint32_t ret;
    if (!hmac_get_tid_schedule_mgmt()->enabled) {
        return hmac_host_ring_tx(mac_vap, hmac_user, netbuf);
    } else {
        tid = MAC_GET_CB_WME_TID_TYPE((mac_tx_ctl_stru *)oal_netbuf_cb(netbuf));
        ret = hmac_host_tx_tid_enqueue(&hmac_user->tx_tid_info[tid], netbuf, OPERATE_TYPE_TAIL);

        host_end_record_performance(1, TX_XMIT_PROC);
        return ret;
    }
}

#endif
