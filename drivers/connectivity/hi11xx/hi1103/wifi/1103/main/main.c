

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#define HISI_LOG_TAG    "[WIFI_MAIN]"
#include "main.h"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "sdt_drv.h"
#elif ((_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)||(_PRE_OS_VERSION_WIN32_RAW == _PRE_OS_VERSION))
#include "mac_resource.h"
#endif

#if (defined(_PRE_PRODUCT_ID_HI110X_HOST))
#include "hmac_ext_if.h"
#include "wal_ext_if.h"
#include "dmac_ext_if.h"
#include "oal_kernel_file.h"
#include "oal_main.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "hisi_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#elif (defined(_PRE_PRODUCT_ID_HI110X_DEV))
/*TBD 以上头文件待回收*/
#include "oam_log.h"
#include "oal_sdio.h"
#include "oal_main.h"
#include "uart.h"
#include "oam_msgsendrecv.h"
#include "oam_data_send.h"
#include "uart.h"

#include "oal_hcc_slave_if.h"
#include "pm_extern.h"

#include "hal_chip.h"
#include "hal_ext_if.h"

#include "dmac_ext_if.h"
#include "dmac_alg.h"

#include "dmac_pm_sta.h"

#ifdef _PRE_WLAN_FEATURE_BTCOEX
#include "dmac_btcoex.h"
#endif

#ifdef _PRE_WLAN_ALG_ENABLE
#include "alg_ext_if.h"
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
/* g_puc_matrix_data在函数hi110x_device_main_init中申请，在hi1103_initialize_rf_sys函数中释放 */
oal_uint8 *g_puc_matrix_data = OAL_PTR_NULL;
oal_int32 g_device_main_return = OAL_SUCC;
oal_int32 g_device_main_err_line = 0;
#define RECORD_INIT_ERR(ret) do{g_device_main_return = (oal_int32)ret; g_device_main_err_line = __LINE__;}while(0)
#else
#define RECORD_INIT_ERR(ret)   do{OAL_REFERENCE(ret);}while(0)
#endif

#endif

#if (defined(_PRE_PLAT_FEATURE_CUSTOMIZE) && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE))
extern oal_int32 dmac_rx_custom_post_data_function(oal_uint8 stype,
                                             hcc_netbuf_stru* pst_hcc_netbuf, oal_uint8 *pst_context);
#endif // #if (defined(_PRE_PLAT_FEATURE_CUSTOMIZE) && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE))


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAIN_C
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#if ((!defined(_PRE_PRODUCT_ID_HI110X_DEV)) || (_PRE_OS_VERSION_WIN32_RAW == _PRE_OS_VERSION))
oal_void platform_module_exit(oal_uint16 us_bitmap);
OAL_STATIC oal_void builder_module_exit(oal_uint16 us_bitmap);
#endif
/*****************************************************************************
  3 函数实现
*****************************************************************************/


#if ((!defined(_PRE_PRODUCT_ID_HI110X_DEV)) || (_PRE_OS_VERSION_WIN32_RAW == _PRE_OS_VERSION))
OAL_STATIC oal_void builder_module_exit(oal_uint16 us_bitmap)
{
    #if (!defined(_PRE_PRODUCT_ID_HI110X_DEV))
        if (BIT8 & us_bitmap)
        {
            wal_main_exit_etc();
        }
        if (BIT7 & us_bitmap)
        {
            hmac_main_exit_etc();
        }

    #elif (!defined(_PRE_PRODUCT_ID_HI110X_HOST))

        #ifdef _PRE_WLAN_ALG_ENABLE
            if (BIT6 & us_bitmap)
            {
                alg_main_exit();
            }
        #endif

        if (BIT5 & us_bitmap)
        {
            dmac_main_exit();
        }
        if (BIT4 & us_bitmap)
        {
            hal_main_exit();
        }

        platform_module_exit(us_bitmap);

#endif

    return;
}
#endif

#if ((_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)&&(defined(_PRE_PRODUCT_ID_HI110X_HOST)))

OAL_STATIC oal_uint32 host_test_get_chip_msg(oal_void)
{
    oal_uint32             ul_return;
    mac_chip_stru         *pst_chip;
    frw_event_mem_stru    *pst_event_mem;
    frw_event_stru        *pst_event;             /* 事件结构体 */
    oal_uint32             ul_dev_id;
	oal_netbuf_stru       *pst_netbuf;
    dmac_tx_event_stru    *pst_ctx_event;
    oal_uint8             *pst_mac_rates_11g;
    /** 待补充 ***/

    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_tx_event_stru));
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAL_IO_PRINT("host_test_get_chip_msg: hmac_init_event_process_etc FRW_EVENT_ALLOC result = OAL_PTR_NULL.\n");
        return OAL_FAIL;
    }

    /* 申请netbuf内存 */
    pst_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_netbuf))
    {
        OAL_IO_PRINT("host_test_get_chip_msg: hmac_init_event_process_etc OAL_MEM_NETBUF_ALLOC result = OAL_PTR_NULL.\n");
        return OAL_FAIL;
    }

    pst_event                 = frw_get_event_stru(pst_event_mem);
    pst_ctx_event             = (dmac_tx_event_stru *)pst_event->auc_event_data;
    pst_ctx_event->pst_netbuf = pst_netbuf;
    pst_mac_rates_11g = (oal_uint8*)oal_netbuf_data(pst_ctx_event->pst_netbuf);
    pst_chip = (mac_chip_stru *)(pst_mac_rates_11g + sizeof(mac_data_rate_stru) * MAC_DATARATES_PHY_80211G_NUM);

    ul_dev_id = (oal_uint32) oal_queue_dequeue(&(g_pst_mac_res->st_dev_res.st_queue));
    /* 0为无效值 */
    if (0 == ul_dev_id)
    {
        OAL_IO_PRINT("host_test_get_chip_msg:oal_queue_dequeue return 0!");
        FRW_EVENT_FREE(pst_event_mem);
        return OAL_FAIL;
    }
    pst_chip->auc_device_id[0] = (oal_uint8)(ul_dev_id - 1);

    /* 根据ul_chip_ver，通过hal_chip_init_by_version函数获得 */
    pst_chip->uc_device_nums = 1;
    pst_chip->uc_chip_id = 0;
    pst_chip->en_chip_state = OAL_TRUE;

    /* 由hal_chip_get_version函数得到,1102 02需要SOC提供寄存器后实现 */
    pst_chip->ul_chip_ver = WLAN_CHIP_VERSION_HI1151V100H;

    pst_chip->pst_hal_chip = OAL_PTR_NULL;


    ul_return = hmac_init_event_process_etc(pst_event_mem);
    if(OAL_UNLIKELY(ul_return != OAL_SUCC))
    {
        OAL_IO_PRINT("host_test_get_chip_msg: hmac_init_event_process_etc  ul_return != OAL_SUCC\n");
        FRW_EVENT_FREE(pst_event_mem);
        oal_netbuf_free(pst_netbuf);
        return OAL_FAIL;
    }

    return OAL_SUCC;

}
#endif

#if  (defined(HI1102_EDA))

OAL_STATIC oal_uint32 device_test_create_cfg_vap(oal_void)
{
    oal_uint32          ul_return;
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru     *pst_event;

    pst_event_mem = FRW_EVENT_ALLOC(0);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAL_IO_PRINT("device_test_create_cfg_vap: hmac_init_event_process_etc FRW_EVENT_ALLOC result = OAL_PTR_NULL.\n");
        return OAL_FAIL;
    }

    ul_return = dmac_init_event_process(pst_event_mem);
    if (OAL_SUCC != ul_return)
    {
        OAL_IO_PRINT("device_test_create_cfg_vap: dmac_init_event_process result = fale.\n");
        FRW_EVENT_FREE(pst_event_mem);
        return OAL_FAIL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_event->st_event_hdr.uc_device_id = 0;

    ul_return = dmac_cfg_vap_init_event(pst_event_mem);
    if (OAL_SUCC != ul_return)
    {
        FRW_EVENT_FREE(pst_event_mem);
        return ul_return;
    }

	FRW_EVENT_FREE(pst_event_mem);

    return OAL_SUCC;
}
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)


oal_void platform_module_exit(oal_uint16 us_bitmap)
{
    if (BIT3 & us_bitmap)
    {
        frw_main_exit();
    }

    if (BIT1 & us_bitmap)
    {
        oam_main_exit();
    }
    if (BIT0 & us_bitmap)
    {
        oal_main_exit();
    }
    return;
}


oal_int32 platform_module_init(oal_void)
{
    oal_int32  l_return   = OAL_FAIL;
#if ((!defined(_PRE_PRODUCT_ID_HI110X_DEV)) || (_PRE_OS_VERSION_WIN32_RAW == _PRE_OS_VERSION))
    oal_uint16  us_bitmap  = 0;
#endif

    l_return = oal_main_init();
    if (OAL_SUCC != l_return)
    {
        OAL_IO_PRINT("platform_module_init: oal_main_init_etc return error code: %d\r\n", l_return);
        RECORD_INIT_ERR(l_return);
        return l_return;
    }
#if (!defined(HI1102_EDA) && !defined(HI110x_EDA))
    l_return = oam_main_init();
    if (OAL_SUCC != l_return)
    {
        OAL_IO_PRINT("platform_module_init: oam_main_init_etc return error code: %d\r\n", l_return);
        RECORD_INIT_ERR(l_return);
#if ((!defined(_PRE_PRODUCT_ID_HI110X_DEV)) || (_PRE_OS_VERSION_WIN32_RAW == _PRE_OS_VERSION))
        us_bitmap = BIT0;
        builder_module_exit(us_bitmap);
#endif
        return l_return;
    }

#endif
    l_return = frw_main_init();
    if (OAL_SUCC != l_return)
    {
        OAL_IO_PRINT("platform_module_init: frw_main_init_etc return error code: %d\r\n", l_return);
        RECORD_INIT_ERR(l_return);
#if ((!defined(_PRE_PRODUCT_ID_HI110X_DEV)) || (_PRE_OS_VERSION_WIN32_RAW == _PRE_OS_VERSION))
        us_bitmap = BIT0 | BIT1 | BIT2;
        builder_module_exit(us_bitmap);
#endif
        return l_return;
    }

    /*启动完成后，输出打印*/
    OAL_IO_PRINT("platform_module_init:: platform_main_init finish!\r\n");

    return OAL_SUCC;
}


OAL_STATIC oal_int32  device_module_init(oal_void)
{
    oal_int32  l_return  = OAL_FAIL;
#if ((!defined(_PRE_PRODUCT_ID_HI110X_DEV)) || (_PRE_OS_VERSION_WIN32_RAW == _PRE_OS_VERSION))
    oal_uint16 us_bitmap = 0;
#endif

    l_return = hal_main_init();
    if (OAL_SUCC != l_return)
    {
        OAL_IO_PRINT("device_module_init: hal_main_init return error code: %d", l_return);
        return l_return;
    }

    l_return = dmac_main_init();
    if (OAL_SUCC != l_return)
    {
        OAL_IO_PRINT("device_module_init: dmac_main_init return error code: %d", l_return);
#if ((!defined(_PRE_PRODUCT_ID_HI110X_DEV)) || (_PRE_OS_VERSION_WIN32_RAW == _PRE_OS_VERSION))
        us_bitmap = BIT4;
        builder_module_exit(us_bitmap);
#endif
        return l_return;
    }

#if (!defined(HI1102_EDA) && !defined(HI110x_EDA))
#if defined(_PRE_WLAN_ALG_ENABLE) || defined(_PRE_WLAN_CHIP_TEST_ALG)
    l_return = alg_main_init();
    if (OAL_SUCC != l_return)
    {
        OAL_IO_PRINT("device_module_init: alg_main_init return error code : %d", l_return);
#if ((!defined(_PRE_PRODUCT_ID_HI110X_DEV)) || (_PRE_OS_VERSION_WIN32_RAW == _PRE_OS_VERSION))
        us_bitmap = BIT4 | BIT5;
        builder_module_exit(us_bitmap);
#endif
        return l_return;
    }
#endif
#endif

    /*启动完成后，输出打印*/
    OAL_IO_PRINT("device_module_init finish!\r\n");

    return OAL_SUCC;
}
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
extern oal_int32 wlan_pm_open_etc(oal_void);
extern oal_uint32 wlan_pm_close_etc(oal_void);

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE


extern oal_bool_enum wlan_pm_switch_etc;
extern oal_uint8 wlan_device_pm_switch;
extern oal_uint8 wlan_ps_mode;
extern oal_uint8 wlan_min_fast_ps_idle;
extern oal_uint8 wlan_max_fast_ps_idle;
extern oal_uint8 wlan_auto_ps_thresh_screen_on;
extern oal_uint8 wlan_auto_ps_thresh_screen_off;
extern oal_uint8 wlan_fast_ps_mode_dyn_ctl;
#ifdef _PRE_WLAN_RF_AUTOCALI
extern oal_uint8 autocali_switch;
#endif
extern oal_uint8 g_auc_mac_device_radio_cap[];
#ifdef _PRE_WLAN_DOWNLOAD_PM
extern oal_uint16 download_rate_limit_pps_etc;
#endif
#if defined(_PRE_FEATURE_PLAT_LOCK_CPUFREQ) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
extern oal_bool_enum_uint8  g_uc_lock_max_cpu_freq_switch;
#endif
oal_uint32 hwifi_cfg_host_global_init_param(oal_void)
{
    oal_uint8      uc_cmd_idx;
    oal_uint8      uc_device_idx;
    oal_int32      l_priv_value = 0;
    oal_int32      l_ret        = OAL_FAIL;

    /*************************** 低功耗定制化 *****************************/
    /*由于device 低功耗开关不是true false,而host是,先取定制化的值赋给device开关,再根据此值给host低功耗开关赋值 */
    wlan_device_pm_switch = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_POWERMGMT_SWITCH);
    wlan_ps_mode          = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_PS_MODE);
    wlan_fast_ps_mode_dyn_ctl = ((MAX_FAST_PS == wlan_ps_mode) ? 1 : 0);
    wlan_min_fast_ps_idle   = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_MIN_FAST_PS_IDLE);
    wlan_max_fast_ps_idle   = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_MAX_FAST_PS_IDLE);
    wlan_auto_ps_thresh_screen_on     = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_AUTO_FAST_PS_THRESH_SCREENON);
    wlan_auto_ps_thresh_screen_off    = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_AUTO_FAST_PS_THRESH_SCREENOFF);

    wlan_pm_switch_etc = (wlan_device_pm_switch == 1 || wlan_device_pm_switch == 4) ? OAL_TRUE : OAL_FALSE;

    /*************************** 私有定制化 *******************************/
    uc_cmd_idx = WLAN_CFG_PRIV_DBDC_RADIO_0;
    for (uc_device_idx = 0; uc_device_idx < WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP; uc_device_idx++)
    {
        l_ret = hwifi_get_init_priv_value(uc_cmd_idx, &l_priv_value);
        if (OAL_SUCC == l_ret)
        {
            /* 定制化 RADIO_0高4bit 给dbdc软件开关用 */
            l_priv_value = (oal_int32)((oal_uint32)l_priv_value & 0x0F);
            wlan_service_device_per_chip[uc_device_idx] = (oal_uint8)(oal_uint32)l_priv_value;
        }

        uc_cmd_idx++;
    }
    /* 同步host侧业务device */
    memcpy_s(g_auc_mac_device_radio_cap, WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP,
             wlan_service_device_per_chip, WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP);
#ifdef _PRE_WLAN_RF_AUTOCALI
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_CALI_AUTOCALI_MASK, &l_priv_value);
    OAL_IO_PRINT("hwifi_cfg_host_global_init_param: auto_cali[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (OAL_SUCC == l_ret)
    {
        autocali_switch = !!l_priv_value;
    }
#endif //#ifdef _PRE_WLAN_RF_AUTOCALI
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_BW_MAX_WITH, &l_priv_value);
    OAL_IO_PRINT("hwifi_cfg_host_global_init_param: bw max with[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (OAL_SUCC == l_ret)
    {
        g_pst_mac_device_capability[0].en_channel_width = (wlan_bw_cap_enum_uint8)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_LDPC_CODING, &l_priv_value);
    OAL_IO_PRINT("hwifi_cfg_host_global_init_param: ldpc coding[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (OAL_SUCC == l_ret)
    {
        g_pst_mac_device_capability[0].en_ldpc_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_RX_STBC, &l_priv_value);
    OAL_IO_PRINT("hwifi_cfg_host_global_init_param: rx stbc[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (OAL_SUCC == l_ret)
    {
        g_pst_mac_device_capability[0].en_rx_stbc_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_TX_STBC, &l_priv_value);
    OAL_IO_PRINT("hwifi_cfg_host_global_init_param: tx stbc[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (OAL_SUCC == l_ret)
    {
        g_pst_mac_device_capability[0].en_tx_stbc_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_SU_BFER, &l_priv_value);
    OAL_IO_PRINT("hwifi_cfg_host_global_init_param: su bfer[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (OAL_SUCC == l_ret)
    {
        g_pst_mac_device_capability[0].en_su_bfmer_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_SU_BFEE, &l_priv_value);
    OAL_IO_PRINT("hwifi_cfg_host_global_init_param: su bfee[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (OAL_SUCC == l_ret)
    {
        g_pst_mac_device_capability[0].en_su_bfmee_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_MU_BFER, &l_priv_value);
    OAL_IO_PRINT("hwifi_cfg_host_global_init_param: mu bfer[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (OAL_SUCC == l_ret)
    {
        g_pst_mac_device_capability[0].en_mu_bfmer_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_MU_BFEE, &l_priv_value);
    OAL_IO_PRINT("hwifi_cfg_host_global_init_param: mu bfee[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (OAL_SUCC == l_ret)
    {
        g_pst_mac_device_capability[0].en_mu_bfmee_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_1024_QAM, &l_priv_value);
    OAL_IO_PRINT("hwifi_cfg_host_global_init_param: 1024 qam[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (OAL_SUCC == l_ret)
    {
        g_pst_mac_device_capability[0].en_1024qam_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }
#ifdef _PRE_WLAN_DOWNLOAD_PM
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_DOWNLOAD_RATE_LIMIT_PPS, &l_priv_value);
    OAL_IO_PRINT("hwifi_cfg_host_global_init_param:download rx drop threshold[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (OAL_SUCC == l_ret)
    {
        download_rate_limit_pps_etc = (oal_uint16)l_priv_value;
    }
#endif

#if defined(_PRE_FEATURE_PLAT_LOCK_CPUFREQ) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_LOCK_MAX_CPU_FREQ, &l_priv_value);
    OAL_IO_PRINT("hwifi_cfg_host_global_init_param:lock_max_cpu_freq[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (OAL_SUCC == l_ret)
    {
        g_uc_lock_max_cpu_freq_switch = (oal_bool_enum_uint8)!!l_priv_value;
    }
#endif
    return OAL_SUCC;
}

#endif //#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE


oal_int32  host_module_init_etc(oal_void)
{
    oal_int32  l_return = OAL_FAIL;
    oal_uint16 us_bitmap = 0;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
     oal_int32            l_priv_value          = 0;
     oal_bool_enum_uint8  en_cali_first_power_on = OAL_TRUE;

    /* 读定制化配置文件&NVRAM */
    hwifi_custom_host_read_cfg_init();
    /* 配置host全局变量值 */
    hwifi_cfg_host_global_init_param();
#endif // #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#endif /* if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE */

    l_return = hmac_main_init_etc();
    if (OAL_SUCC != l_return)
    {
        OAL_IO_PRINT("host_module_init_etc: hmac_main_init_etc return error code:%d\r\n", l_return);
        return l_return;
    }

    l_return = wal_main_init_etc();
    if (OAL_SUCC != l_return)
    {
        OAL_IO_PRINT("host_module_init_etc:wal_main_init_etc return error code:%d\r\n", l_return);
        us_bitmap = BIT7;
        builder_module_exit(us_bitmap);
        return l_return;
    }


#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)&&(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    custom_cali_done_etc = OAL_FALSE;

    #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    l_return = hwifi_get_init_priv_value(WLAN_CFG_PRIV_CALI_DATA_MASK, &l_priv_value);
    if (OAL_SUCC == l_return)
    {
        en_cali_first_power_on = !!(HI1103_CALI_FIST_POWER_ON_MASK & (oal_uint32)l_priv_value);

        OAL_IO_PRINT("host_module_init_etc:en_cali_first_power_on pri_val[0x%x]first_pow[%d]\r\n", l_priv_value, en_cali_first_power_on);
    }
    #endif

    /* 开机校准和定制化参数下发 */
    if (OAL_TRUE == en_cali_first_power_on)
    {
        wlan_pm_open_etc();
    }
#endif

    /*启动完成后，输出打印*/
    OAL_IO_PRINT("host_module_init_etc:: host_main_init finish!\r\n");

    return OAL_SUCC;
}
#endif

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV))
oal_int32 hi110x_device_module_init(oal_void)
{
#if ((!defined(_PRE_PRODUCT_ID_HI110X_DEV)) || (_PRE_OS_VERSION_WIN32_RAW == _PRE_OS_VERSION))
    oal_uint16  us_bitmap;
#endif
    oal_int32   l_return;

#ifdef _PRE_WLAN_FPGA_ABB5
    hal_abb5_init(WLAN_RF_CHANNEL_ZERO);
    hal_abb5_init(WLAN_RF_CHANNEL_ONE);
#endif

    l_return = device_module_init();
    if (OAL_SUCC != l_return)
    {
        OAL_IO_PRINT("hi110x_device_module_init: device_module_init return error code: %d\r\n", l_return);
#if ((!defined(_PRE_PRODUCT_ID_HI110X_DEV)) || (_PRE_OS_VERSION_WIN32_RAW == _PRE_OS_VERSION))
        us_bitmap = BIT0 | BIT1 | BIT2 | BIT3;
        builder_module_exit(us_bitmap);
#endif
        return l_return;
    }

    /*启动完成后，输出打印*/
    OAL_IO_PRINT("hi110x_device_module_init:: device_module_init finish!\r\n");
    OAM_WARNING_LOG0(0, OAM_SF_ANY, "hi110x_device_module_init finish!!!");

    return OAL_SUCC;
}


oal_int32  hi110x_device_main_init(oal_void)
{
    oal_int32  l_return  = OAL_FAIL;

    l_return = platform_module_init();
    if (OAL_SUCC != l_return)
    {
        OAL_IO_PRINT("hi110x_device_main_init: platform_module_init return error code: %d\r\n", l_return);
        return l_return;
    }

#ifndef HI110x_EDA
    /* 申请3个64K的pktram，用来存放校准数据(IQ的H矩阵，DPD corram数据，pktmem发送单音的数据) */
    g_puc_matrix_data = (oal_uint8 *)OAL_MEM_SAMPLE_NETBUF_ALLOC(WLAN_INIT_MATRIZ_DATA_BANKS);
    if (OAL_PTR_NULL == g_puc_matrix_data)
    {
        OAM_ERROR_LOG0(0, 0, "{hi110x_device_main_init:matrix data room alloc failed.}\r\n");
    }
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    l_return = hcc_rx_register(hcc_get_default_handler(), HCC_ACTION_TYPE_CUSTOMIZE,
                          dmac_rx_custom_post_data_function, NULL);
#else
    l_return = hi110x_device_module_init();
#endif //#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#else
    l_return = hi110x_device_module_init();
#endif

    if (OAL_SUCC != l_return)
    {
        RECORD_INIT_ERR(l_return);
        return l_return;
    }

#if (!defined(HI1102_EDA)&&!defined(HI110x_EDA))
    /*device_ready:调用HCC接口通知Hmac,Dmac已经完成初始化 TBD*/
    l_return = hcc_bus_send_message2host(D2H_MSG_WLAN_READY);
    if(OAL_SUCC != l_return)
    {
        RECORD_INIT_ERR(l_return);
        return l_return;
    }
#endif
    return l_return;

}


oal_void device_main_init(oal_void)
{
    /* init */
    extern volatile oal_uint32     g_ulGpioIntCount;

    oal_int32 l_return = OAL_FAIL;
    l_return = hi110x_device_main_init();
    if (OAL_SUCC != l_return)
    {
        OAL_IO_PRINT("device_main_init: hi110x_device_main_init return error code: %d\r\n", l_return);
        /* 初始化失败不退出主程序，等待重启 */
        for (;;)
            ;
    }
    OAL_IO_PRINT("device_main_init: hi110x_device_main_init succ!!\r\n");
#if 1
#if (SUB_SYSTEM == SUB_SYS_WIFI)
    PM_WLAN_IsrRegister();
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_DEV))
    PM_WLAN_FuncRegister(device_psm_main_function, dmac_psm_check_hw_txq_state, dmac_psm_check_txrx_state,
                         dmac_psm_clean_state, dmac_psm_save_start_dma, dmac_psm_save_ps_state,
                         dmac_psm_recover_no_powerdown, dmac_psm_recover_start_dma, dmac_psm_recover_powerdown,
                         dmac_psm_sync_tsf_to_sta, dmac_psm_sync_tsf_to_ap, dmac_psm_is_fake_queues_empty, dmac_check_all_sleep_time,
                         dmac_psm_slave_sleep_callback, dmac_psm_slave_wkup_callback);
#endif
#endif
#endif
    g_ulGpioIntCount = 0;
}






oal_uint8 device_psm_main_function(oal_void)
{
#if defined(_PRE_WLAN_FEATURE_BTCOEX) || defined(_PRE_WLAN_FEATURE_SMARTANT)
    dmac_device_stru        *pst_dmac_device;
    hal_to_dmac_device_stru *pst_hal_device;
    hal_to_dmac_chip_stru   *pst_hal_chip    = OAL_PTR_NULL;
#endif

#ifdef _PRE_WLAN_FEATURE_SMARTANT
    oal_uint32 ul_result;
#endif

#if defined(_PRE_WLAN_FEATURE_BTCOEX) || defined(_PRE_WLAN_FEATURE_SMARTANT)
    pst_dmac_device = dmac_res_get_mac_dev(0);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{device_psm_main_function::pst_device[id:0] is NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_hal_device = pst_dmac_device->past_hal_device[HAL_DEVICE_ID_MASTER];

    hal_chip_get_chip(pst_hal_device->uc_chip_id, &pst_hal_chip);
#endif

    device_main_function();

#ifdef _PRE_WLAN_FEATURE_SMARTANT
    hal_dual_antenna_switch(pst_hal_device, pst_hal_chip->ul_dual_antenna_status, 1, &ul_result);
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    hal_btcoex_process_bt_status(pst_hal_chip, 0);
#endif

    return OAL_SUCC;
}

#elif (defined(_PRE_PRODUCT_ID_HI110X_HOST))
#include "hmac_vap.h"
#include "oal_hcc_host_if.h"


oal_int32  hi110x_host_main_init(oal_void)
{
    oal_int32  l_return = OAL_FAIL;

#ifdef _PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL
    hcc_flowctl_get_device_mode_register_etc(hmac_flowctl_check_device_is_sta_mode_etc);
    hcc_flowctl_operate_subq_register_etc(hmac_vap_net_start_subqueue_etc, hmac_vap_net_stop_subqueue_etc);
#else
    hcc_tx_flow_ctrl_cb_register_etc(hmac_vap_net_stopall_etc, hmac_vap_net_startall_etc);
#endif

    l_return = host_module_init_etc();
    if (OAL_SUCC != l_return)
    {
        OAL_IO_PRINT("hi110x_host_main_init: host_module_init_etc return error code: %d", l_return);
        return l_return;
    }

#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
    wal_hipriv_register_inetaddr_notifier_etc();
    wal_hipriv_register_inet6addr_notifier_etc();
#endif

    /*启动完成后，输出打印*/
    OAL_IO_PRINT("hi110x_host_main_init:: hi110x_host_main_init finish!\n");

    return OAL_SUCC;

}

oal_void  hi110x_host_main_exit(oal_void)
{
    oal_uint16 us_bitmap = 0;

#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
    wal_hipriv_unregister_inetaddr_notifier_etc();
    wal_hipriv_unregister_inet6addr_notifier_etc();
#endif

    us_bitmap =  BIT6 | BIT7 | BIT8;
    builder_module_exit(us_bitmap);

    return ;
}
#endif




/*lint -e578*//*lint -e19*/
#if (defined(_PRE_PRODUCT_ID_HI110X_HOST))
#ifndef CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT

#include "board.h"
#include "oneimage.h"

oal_int32 g_wifi_init_flag_etc = 0;
oal_int32 g_wifi_init_ret_etc;
/*built-in*/
OAL_STATIC ssize_t  wifi_sysfs_set_init(struct kobject *dev, struct kobj_attribute *attr, const char *buf, size_t count)
{
    char            mode[128] = {0};

    if (NULL == buf)
    {
        OAL_IO_PRINT("buf is null r failed!%s\n",__FUNCTION__);
        return 0;
    }

    if (NULL == attr)
    {
        OAL_IO_PRINT("attr is null r failed!%s\n",__FUNCTION__);
        return 0;
    }

    if (NULL == dev)
    {
        OAL_IO_PRINT("dev is null r failed!%s\n",__FUNCTION__);
        return 0;
    }

    if ((sscanf(buf, "%20s", mode) != 1))
    {
        OAL_IO_PRINT("set value one param!\n");
        return -OAL_EINVAL;
    }

    if(sysfs_streq("init", mode))
    {
        /*init*/
        if(0 == g_wifi_init_flag_etc)
        {
            g_wifi_init_ret_etc = hi110x_host_main_init();
            g_wifi_init_flag_etc = 1;
        }
        else
        {
            OAL_IO_PRINT("double init!\n");
        }
    }
    else
    {
        OAL_IO_PRINT("invalid input:%s\n",mode);
    }

    return count;
}

OAL_STATIC ssize_t  wifi_sysfs_get_init(struct kobject *dev, struct kobj_attribute *attr, char*buf)
{
    int ret = 0;

    if (NULL == buf)
    {
        OAL_IO_PRINT("buf is null r failed!%s\n",__FUNCTION__);
        return 0;
    }

    if (NULL == attr)
    {
        OAL_IO_PRINT("attr is null r failed!%s\n",__FUNCTION__);
        return 0;
    }

    if (NULL == dev)
    {
        OAL_IO_PRINT("dev is null r failed!%s\n",__FUNCTION__);
        return 0;
    }

    if(1 == g_wifi_init_flag_etc)
    {
        if(OAL_SUCC == g_wifi_init_ret_etc)
        {
            ret +=  snprintf_s(buf + ret , PAGE_SIZE - ret, (PAGE_SIZE - ret) - 1, "running\n");
        }
        else
        {
            ret +=  snprintf_s(buf + ret , PAGE_SIZE - ret, (PAGE_SIZE - ret) - 1, "boot failed ret=%d\n", g_wifi_init_ret_etc);
        }
    }
    else
    {
        ret +=  snprintf_s(buf + ret , PAGE_SIZE - ret, (PAGE_SIZE - ret) - 1, "uninit\n");
    }

    return ret;
}
STATIC struct kobj_attribute dev_attr_wifi =
    __ATTR(wifi, S_IRUGO | S_IWUSR, wifi_sysfs_get_init, wifi_sysfs_set_init);
OAL_STATIC struct attribute *wifi_init_sysfs_entries[] = {
        &dev_attr_wifi.attr,
        NULL
};

OAL_STATIC struct attribute_group wifi_init_attribute_group = {
        .attrs = wifi_init_sysfs_entries,
};

oal_int32  wifi_sysfs_init_etc(oal_void)
{
    oal_int32 ret = 0;
    oal_kobject*     pst_root_boot_object = NULL;

    if(false == is_hisi_chiptype_etc(BOARD_VERSION_HI1103))
    {
        return OAL_SUCC;
    }

    pst_root_boot_object = oal_get_sysfs_root_boot_object_etc();
    if(NULL == pst_root_boot_object)
    {
        OAL_IO_PRINT("[E]get root boot sysfs object failed!\n");
        return -OAL_EBUSY;
    }

    ret = sysfs_create_group(pst_root_boot_object,&wifi_init_attribute_group);
    if (ret)
    {
        OAL_IO_PRINT("sysfs create plat boot group fail.ret=%d\n",ret);
        ret = -OAL_ENOMEM;
        return ret;
    }

    return ret;
}

oal_void  wifi_sysfs_exit_etc(oal_void)
{
    /*need't exit,built-in*/
    return;
}
oal_module_init(wifi_sysfs_init_etc);
oal_module_exit(wifi_sysfs_exit_etc);
#endif
#else
oal_module_init(hi110x_host_main_init);
oal_module_exit(hi110x_host_main_exit);
#endif
#endif
oal_module_license("GPL");
/*lint +e578*//*lint +e19*/





#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

