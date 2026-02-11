

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 1 头文件包含 */
#include "hmac_tx_opt.h"
#include "hmac_vap.h"
#include "mac_vap.h"
#include "mac_device.h"
#include "hmac_config.h"
#include "hmac_statistic_data_flow.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TX_OPT_C

/* 2 全局变量定义 */
oal_bool_enum_uint8 g_en_tx_opt = OAL_FALSE;
oal_uint8 g_uc_opt_switch_cnt = 0;

/* 3 函数实现 */

oal_void hmac_set_tx_opt_switch_cnt(oal_uint8 uc_opt_switch_cnt)
{
    g_uc_opt_switch_cnt = uc_opt_switch_cnt;
}


OAL_STATIC oal_uint32 hmac_sync_tx_opt_switch(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_tx_opt)
{
    oal_uint32 ul_ret;
    mac_cfg_tx_opt st_tx_opt;

    g_en_tx_opt = en_tx_opt;
    g_uc_opt_switch_cnt = 0;

    st_tx_opt.en_txopt_on = en_tx_opt;

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "hmac_sync_tx_opt_switch: txopt_on[%d]", st_tx_opt.en_txopt_on);

    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_TX_OPT_SYN, OAL_SIZEOF(mac_cfg_tx_opt), (oal_uint8 *) &st_tx_opt);
    if (OAL_UNLIKELY(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_sync_tx_opt_switch::hmac_config_send_event failed[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ


OAL_STATIC oal_bool_enum_uint8 hmac_get_tx_opt_status(oal_uint32 ul_tx_large_pps)
{
    if (ul_tx_large_pps > g_st_wifi_tx_opt.us_txopt_th_pps_high) {
        return OAL_TRUE;
    } else if (ul_tx_large_pps < g_st_wifi_tx_opt.us_txopt_th_pps_low){
        return OAL_FALSE;
    } else {
        return g_en_tx_opt;
    }
}


OAL_STATIC oal_bool_enum_uint8 hmac_tx_opt_enable(mac_device_stru *pst_mac_device, oal_uint32 ul_tx_large_pps)
{
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;

    if (mac_device_calc_up_vap_num(pst_mac_device) != 1 ||
        mac_device_find_up_vap(pst_mac_device, &pst_mac_vap) != OAL_SUCC) {
        return OAL_FALSE;
    }

    if (!IS_LEGACY_VAP(pst_mac_vap) ||
        (IS_AP(pst_mac_vap) && pst_mac_vap->us_user_nums != 1)) {
        return OAL_FALSE;
    }

    return hmac_get_tx_opt_status(ul_tx_large_pps);
}


OAL_STATIC oal_bool_enum_uint8 hmac_tx_opt_change_state(oal_bool_enum_uint8 en_tx_opt)
{
    if (g_en_tx_opt == en_tx_opt) {
        g_uc_opt_switch_cnt = 0;
        return OAL_FALSE;
    }

    g_uc_opt_switch_cnt++;

    /* 尝试开启时立刻关闭; 尝试关闭时连续3次与原状态不一致才关闭 */
    return en_tx_opt || g_uc_opt_switch_cnt >= MAX_TX_OPT_SWITCH_CNT;
}


oal_void hmac_tx_opt_switch(oal_uint32 ul_tx_large_pps)
{
    oal_bool_enum_uint8 en_tx_opt;
    mac_vap_stru       *pst_mac_vap = OAL_PTR_NULL;
    mac_device_stru    *pst_mac_device = OAL_PTR_NULL;

    if (!g_st_wifi_tx_opt.en_txopt_on) {
        return;
    }

    pst_mac_device = mac_res_get_dev(0);
    en_tx_opt = hmac_tx_opt_enable(pst_mac_device, ul_tx_large_pps);

    if (hmac_tx_opt_change_state(en_tx_opt)) {
        pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->uc_cfg_vap_id);
        if (pst_mac_vap == OAL_PTR_NULL) {
            OAM_ERROR_LOG0(pst_mac_device->uc_cfg_vap_id, OAM_SF_ANY, "{hmac_tx_opt_switch::pst_mac_vap NULL}");
            return;
        }

        hmac_sync_tx_opt_switch(pst_mac_vap, en_tx_opt);
    }
}
#endif
#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

