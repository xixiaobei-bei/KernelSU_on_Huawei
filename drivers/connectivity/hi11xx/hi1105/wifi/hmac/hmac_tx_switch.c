

#include "hmac_tx_switch.h"
#include "hmac_config.h"
#include "hmac_user.h"
#include "hmac_tx_msdu_dscr.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TX_SWITCH_C

uint32_t g_device_ring_bitmap = 0; /* 大小保持与device ring pool size一致 */

OAL_STATIC OAL_INLINE uint8_t hmac_get_avail_tx_ring_index(void)
{
    uint8_t index = 0;

    while (index < DEVICE_TX_RING_MAX_NUM) {
        if ((g_device_ring_bitmap & BIT(index)) == 0) {
            g_device_ring_bitmap |= BIT(index);
            return index;
        }

        index++;
    }

    return DEVICE_TX_RING_MAX_NUM;
}

void hmac_set_avail_tx_ring_index(uint8_t index)
{
    if (index >= DEVICE_TX_RING_MAX_NUM) {
        return;
    }

    g_device_ring_bitmap &= ~(BIT(index));
}

OAL_STATIC uint32_t hmac_tx_switch_send_event(
    mac_vap_stru *mac_vap, hmac_tid_info_stru *tid_info, uint8_t mode, uint8_t ring_index)
{
    tx_state_switch_stru param = {
        .user_id = tid_info->user_index, .tid_no = tid_info->tid_no, .cmd = mode, .ring_index = ring_index,
    };

    return hmac_config_send_event(mac_vap, WLAN_CFGID_TX_SWITCH_START, sizeof(tx_state_switch_stru), (uint8_t *)&param);
}

OAL_STATIC uint32_t hmac_tx_mode_d2h_switch(hmac_tid_info_stru *tid_info)
{
    if (oal_atomic_read(&tid_info->ring_tx_mode) != DEVICE_RING_TX_MODE) {
        return OAL_SUCC;
    }

    /* device ring tx切换至host ring tx */
    oal_atomic_set(&tid_info->ring_tx_mode, D2H_SWITCHING_MODE);

    return hmac_tx_switch_send_event(mac_res_get_mac_vap(0), tid_info, HOST_RING_TX_MODE, 0);
}

OAL_STATIC uint32_t hmac_tx_mode_h2d_switch(hmac_tid_info_stru *tid_info)
{
    uint8_t avail_ring_index;

    if (oal_atomic_read(&tid_info->ring_tx_mode) != HOST_RING_TX_MODE) {
        return OAL_SUCC;
    }

    avail_ring_index = hmac_get_avail_tx_ring_index();
    if (avail_ring_index == DEVICE_TX_RING_MAX_NUM) {
        oam_warning_log0(0, OAM_SF_TX, "{hmac_tx_mode_h2d_switch::device ring run out!}");
        return OAL_FAIL;
    }

    /* host ring tx切换至device ring tx */
    oal_atomic_set(&tid_info->ring_tx_mode, H2D_SWITCHING_MODE);
    oal_atomic_set(&tid_info->device_ring_index, avail_ring_index);

    return hmac_tx_switch_send_event(mac_res_get_mac_vap(0), tid_info, DEVICE_RING_TX_MODE, avail_ring_index);
}

OAL_STATIC uint32_t hmac_tx_mode_switch_dump(hmac_tid_info_stru *tid_info)
{
    oam_warning_log1(0, 0, "{hmac_tx_mode_switch_dump::tx mode[%d]}", oal_atomic_read(&tid_info->ring_tx_mode));

    return hmac_tx_switch_send_event(mac_res_get_mac_vap(0), tid_info, TX_MODE_DEBUG_DUMP, 0);
}

uint32_t hmac_tx_mode_switch(mac_vap_stru *mac_vap, uint32_t *params)
{
    uint32_t ret;
    hmac_user_stru *hmac_user = NULL;
    uint8_t user_id = (uint8_t)params[0];
    uint8_t tid_no = (uint8_t)params[1];
    uint8_t mode = (uint8_t)params[2];

    if (tid_no == WLAN_TIDNO_BCAST) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_tx_mode_switch::tid[7] not supported}");
        return OAL_SUCC;
    }

    hmac_user = mac_res_get_hmac_user(user_id);
    if (hmac_user == NULL || hmac_user->st_user_base_info.en_user_asoc_state != MAC_USER_STATE_ASSOC) {
        oam_error_log1(0, OAM_SF_CFG, "{hmac_tx_mode_switch::user[%d] not associated!}", user_id);
        return OAL_FAIL;
    }

    oam_warning_log3(0, OAM_SF_CFG, "{hmac_tx_mode_switch::user[%d] tid[%d] switch mode[%d]}", user_id, tid_no, mode);

    switch (mode) {
        case HOST_RING_TX_MODE:
            ret = hmac_tx_mode_d2h_switch(&hmac_user->tx_tid_info[tid_no]);
            break;
        case DEVICE_RING_TX_MODE:
            ret = hmac_tx_mode_h2d_switch(&hmac_user->tx_tid_info[tid_no]);
            break;
        case TX_MODE_DEBUG_DUMP:
            ret = hmac_tx_mode_switch_dump(&hmac_user->tx_tid_info[tid_no]);
            break;
        default:
            break;
    }

    return OAL_SUCC;
}

OAL_STATIC void hmac_tx_mode_d2h_switch_process(hmac_user_stru *hmac_user, hmac_tid_info_stru *tid_info)
{
    hmac_set_avail_tx_ring_index(oal_atomic_read(&tid_info->device_ring_index));
    oal_atomic_set(&tid_info->device_ring_index, DEVICE_TX_RING_MAX_NUM);

    tid_info->tx_ring.base_ring_info.read_index = 0;
    tid_info->tx_ring.base_ring_info.write_index = 0;
    tid_info->tx_ring.release_index = 0;
    tid_info->tx_ring_device_info->msdu_info_word2.word2_bit.write_ptr = 0;
    tid_info->tx_ring_device_info->msdu_info_word3.word3_bit.read_ptr = 0;
    hmac_tx_reg_write_ring_info(hmac_user, tid_info, TID_CMDID_CREATE);
}

uint32_t hmac_tx_mode_switch_process(frw_event_mem_stru *event_mem)
{
    tx_state_switch_stru *tx_switch_param = NULL;
    hmac_user_stru *hmac_user = NULL;
    hmac_tid_info_stru *tid_info = NULL;

    if (event_mem == NULL) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_tx_complete_event_handler::event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    tx_switch_param = (tx_state_switch_stru *)(frw_get_event_stru(event_mem)->auc_event_data);
    hmac_user = mac_res_get_hmac_user(tx_switch_param->user_id);
    if (hmac_user == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_tx_mode_switch_process::hmac_user is NULL}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    tid_info = &hmac_user->tx_tid_info[tx_switch_param->tid_no];

    if (tx_switch_param->cmd == oal_atomic_read(&tid_info->ring_tx_mode)) {
        oam_warning_log3(0, 0, "{hmac_tx_switching::user[%d] tid[%d] target mode[%d] is the same as current tx mode",
            tx_switch_param->user_id, tx_switch_param->tid_no, tx_switch_param->cmd);
        return OAL_SUCC;
    }

    oam_warning_log3(0, 0, "{hmac_tx_mode_switch_process::user[%d] tid[%d] mode[%d]}",
        tx_switch_param->user_id, tx_switch_param->tid_no, tx_switch_param->cmd);

    if (tx_switch_param->cmd == HOST_RING_TX_MODE) {
        hmac_tx_mode_d2h_switch_process(hmac_user, tid_info);
    }

    oal_atomic_set(&tid_info->ring_tx_mode, tx_switch_param->cmd);

    frw_event_free_m(event_mem);

    return OAL_SUCC;
}

