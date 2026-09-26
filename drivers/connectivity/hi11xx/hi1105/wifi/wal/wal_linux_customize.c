

#include "wal_linux_customize.h"

#include "wlan_chip_i.h"

#include "wal_linux_ioctl.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID      OAM_FILE_ID_WAL_LINUX_CUSTOMIZE_C

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/* 每次上下电由配置vap完成的定制化只配置一次，wlan p2p iface不再重复配置 */
OAL_STATIC uint8_t g_uc_cfg_once_flag = OAL_FALSE;


void hwifi_config_init_force(void)
{
    /* 重新上电时置为FALSE */
    g_uc_cfg_once_flag = OAL_FALSE;

    hwifi_config_host_global_ini_param();
}


int32_t wal_set_custom_process_func(void)
{
    struct custom_process_func_handler *pst_custom_process_func_handler;

    pst_custom_process_func_handler = oal_get_custom_process_func();
    if (pst_custom_process_func_handler == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_set_auto_freq_process_func get handler failed!}");
    } else {
        pst_custom_process_func_handler->p_custom_cali_func = wal_custom_cali;
    }

    return OAL_SUCC;
}
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */


uint32_t wal_custom_cali(void)
{
    return wlan_chip_custom_cali();
}

void hwifi_config_host_global_ini_param(void)
{
    wlan_chip_host_global_ini_param_init();
}


uint32_t hwifi_force_refresh_rf_params(oal_net_device_stru *pst_net_dev)
{
    /* update params */
    if (wlan_chip_force_update_custom_params() != OAL_SUCC) {
        return OAL_FAIL;
    }
    /* send data to device */
    return wlan_chip_init_nvram_main(pst_net_dev);
}
