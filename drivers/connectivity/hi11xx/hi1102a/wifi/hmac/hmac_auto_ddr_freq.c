

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 1 头文件包含 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST)
#include "hmac_auto_ddr_freq.h"
#include <linux/pm_qos.h>
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_AUTO_DDR_FREQ_C

/* 2 全局变量定义 */
hmac_auto_ddr_freq_mgmt_stru g_st_auto_ddr_freq_mgmt = { 0 };

/* 3 函数实现 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST)

OAL_STATIC oal_void hmac_remove_ddr_freq_req(oal_void)
{
    if (g_st_auto_ddr_freq_mgmt.pst_auto_ddr_freq == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_add_ddr_freq_req::remove ddr freq req from kernel FAIL!}");
        return;
    }

    pm_qos_remove_request(g_st_auto_ddr_freq_mgmt.pst_auto_ddr_freq);

    OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_remove_ddr_freq_req::remove ddr freq req from kernel SUCC!}");
}


OAL_STATIC oal_void hmac_add_ddr_freq_req(oal_void)
{
    if (g_st_auto_ddr_freq_mgmt.pst_auto_ddr_freq == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_add_ddr_freq_req::add ddr freq req to kernel FAIL!}");
        return;
    }

    pm_qos_add_request(g_st_auto_ddr_freq_mgmt.pst_auto_ddr_freq, PM_QOS_MEMORY_THROUGHPUT, HMAC_DDR_MAX_FREQ);

    OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_add_ddr_freq_req::add ddr freq req to kernel SUCC!}");
}


oal_void hmac_auto_ddr_init(oal_void)
{
    g_st_auto_ddr_freq_mgmt.pst_auto_ddr_freq = kzalloc(sizeof(struct pm_qos_request), GFP_KERNEL);
    if (g_st_auto_ddr_freq_mgmt.pst_auto_ddr_freq == OAL_PTR_NULL) {
        OAL_IO_PRINT("{hmac_auto_ddr_init::auto ddr init failed!}");
        return;
    }

    g_st_auto_ddr_freq_mgmt.en_auto_ddr_freq_switch = OAL_TRUE;
}


oal_void hmac_auto_ddr_exit(oal_void)
{
    hmac_remove_ddr_freq_req();
    kfree(g_st_auto_ddr_freq_mgmt.pst_auto_ddr_freq);

    g_st_auto_ddr_freq_mgmt.pst_auto_ddr_freq = OAL_PTR_NULL;
    g_st_auto_ddr_freq_mgmt.en_auto_ddr_freq_switch = OAL_FALSE;
}


OAL_STATIC hmac_auto_ddr_freq_update_enum hmac_get_ddr_freq_update_type(oal_uint32 ul_total_pps)
{
    if (ul_total_pps > g_st_auto_ddr_freq_mgmt.ul_auto_ddr_freq_pps_th_high &&
        !g_st_auto_ddr_freq_mgmt.en_ddr_freq_updated) {
        g_st_auto_ddr_freq_mgmt.en_ddr_freq_updated = OAL_TRUE;
        return HMAC_AUTO_DDR_FREQ_ADD_REQUEST;
    } else if (ul_total_pps < g_st_auto_ddr_freq_mgmt.ul_auto_ddr_freq_pps_th_low &&
               g_st_auto_ddr_freq_mgmt.en_ddr_freq_updated) {
        g_st_auto_ddr_freq_mgmt.en_ddr_freq_updated = OAL_FALSE;
        return HMAC_AUTO_DDR_FREQ_REMOVE_REQUEST;
    } else {
        return HMAC_AUTO_DDR_FREQ_RETURN;
    }
}


oal_void hmac_auto_set_ddr_freq(oal_uint32 ul_total_pps)
{
    hmac_auto_ddr_freq_update_enum en_update_type;

    if (!g_st_auto_ddr_freq_mgmt.en_auto_ddr_freq_switch) {
        return;
    }

    en_update_type = hmac_get_ddr_freq_update_type(ul_total_pps);

    switch (en_update_type) {
        case HMAC_AUTO_DDR_FREQ_ADD_REQUEST:
            hmac_add_ddr_freq_req();
            break;
        case HMAC_AUTO_DDR_FREQ_REMOVE_REQUEST:
            hmac_remove_ddr_freq_req();
            break;
        case HMAC_AUTO_DDR_FREQ_RETURN:
        default:
            return;
    }
}

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

