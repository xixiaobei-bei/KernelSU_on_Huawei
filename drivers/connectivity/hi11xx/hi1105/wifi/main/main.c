

/* 1 头文件包含 */
#define HISI_LOG_TAG "[WIFI_MAIN]"
#include "main.h"
#include "oal_kernel_file.h"
#include "oal_hcc_host_if.h"

#include "oneimage.h"
#include "wlan_chip_i.h"

#include "hmac_vap.h"
#include "hmac_cali_mgmt.h"
#include "wal_ext_if.h"
#include "host_hal_ext_if.h"

#include "plat_pm_wlan.h"
#include "hisi_customize_wifi.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAIN_C

void platform_module_exit(uint16_t us_bitmap);
OAL_STATIC void builder_module_exit(uint16_t us_bitmap);
#ifdef _PRE_WINDOWS_SUPPORT
oal_wait_queue_head_stru g_wlan_cali_complete_wq;
#endif

/* 3 函数实现 */

OAL_STATIC void builder_module_exit(uint16_t us_bitmap)
{
    if (us_bitmap & BIT8) {
        wal_main_exit();
    }
    if (us_bitmap & BIT7) {
        hmac_main_exit();
    }

    return;
}

#if (defined(HI1102_EDA))

OAL_STATIC uint32_t device_test_create_cfg_vap(void)
{
    uint32_t ret;
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event;

    pst_event_mem = frw_event_alloc_m(0);
    if (oal_unlikely(pst_event_mem == NULL)) {
        oal_io_print("device_test_create_cfg_vap: hmac_init_event_process frw_event_alloc_m result=NULL.\n");
        return OAL_FAIL;
    }

    ret = dmac_init_event_process(pst_event_mem);
    if (ret != OAL_SUCC) {
        oal_io_print("device_test_create_cfg_vap: dmac_init_event_process result = fale.\n");
        frw_event_free_m(pst_event_mem);
        return OAL_FAIL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_event->st_event_hdr.uc_device_id = 0;

    ret = dmac_cfg_vap_init_event(pst_event_mem);
    if (ret != OAL_SUCC) {
        frw_event_free_m(pst_event_mem);
        return ret;
    }

    frw_event_free_m(pst_event_mem);

    return OAL_SUCC;
}
#endif

#define MAIN_WAL_WAIT_CALI_COMPLETE_TIME  15000

/*
 * 函 数 名  : host_rf_cali_init
 * 功能描述  : 开机校准流程
 */
void host_rf_cali_init(void)
{
    oal_bool_enum_uint8 cali_first_pwr_on = OAL_TRUE;
#ifndef _PRE_LINUX_TEST
    hi110x_board_info *bd_info = get_hi110x_board_info();
#endif
    oal_init_completion(hmac_get_wlan_cali_completed_addr());
    oal_wait_queue_init_head(&g_wlan_cali_complete_wq); /* queues init */

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    g_custom_cali_done = OAL_FALSE;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    cali_first_pwr_on = wlan_chip_first_power_on_mark();
#endif
    /* 开机校准和定制化参数下发 */
    if (cali_first_pwr_on == OAL_TRUE) {
        wlan_pm_open();
#ifndef _PRE_LINUX_TEST
        /*  PCIE总线工作模式:开机校准完成后，立即置位开机上电校准完成信号量  */
        if (bd_info->wlan_download_channel == MODE_PCIE) {
            wlan_chip_first_powon_cali_completed();
        }
#endif
        /* 等待开机校准数据上传完成 */
        if (oal_wait_for_completion_timeout(hmac_get_wlan_cali_completed_addr(),
                                            (uint32_t)oal_msecs_to_jiffies(MAIN_WAL_WAIT_CALI_COMPLETE_TIME)) == 0) {
            oam_error_log1(0, OAM_SF_ANY, "{host_module_init::wait [%d]ms g_wlan_cali_completed timeout)!}",
                MAIN_WAL_WAIT_CALI_COMPLETE_TIME);
        }
        wlan_pm_close();
    }
    hmac_set_wlan_first_powon_mark(OAL_FALSE);
    oal_wait_queue_wake_up(&g_wlan_cali_complete_wq);
#endif
}

int32_t host_module_init(void)
{
    int32_t l_return;
    uint16_t us_bitmap = 0;

    if (wlan_chip_ops_init() != OAL_SUCC) {
        oal_io_print("host_module_init: wlan_chip_ops_init return fail\r\n");
        return OAL_FAIL;
    }

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wlan_chip_custom_host_cali_data_init();
    register_wifi_customize(wlan_chip_hcc_customize_h2d_data_cfg);

    /* 读定制化配置文件&NVRAM */
    wlan_chip_custom_host_read_cfg_init();
    /* 配置host全局变量值 */
    wlan_chip_host_global_init_param();
#endif  // #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

    l_return = hal_main_init();
    if (l_return != OAL_SUCC) {
        oal_io_print("host_module_init: hal_main_init return error code:%d\r\n", l_return);
        return l_return;
    }

    l_return = hmac_main_init();
    if (l_return != OAL_SUCC) {
        oal_io_print("host_module_init: hmac_main_init return error code:%d\r\n", l_return);
        return l_return;
    }

    l_return = wal_main_init();
    if (l_return != OAL_SUCC) {
        oal_io_print("host_module_init:wal_main_init return error code:%d\r\n", l_return);
        us_bitmap = BIT7;
        builder_module_exit(us_bitmap);
        return l_return;
    }
    host_rf_cali_init();
    /* 启动完成后，输出打印 */
    oal_io_print("host_module_init:: host_main_init finish!\r\n");

    return OAL_SUCC;
}

static const oal_wlan_cfg_stru g_wlan_spec_cfg_hi1103 = {
    .feature_m2s_modem_is_open = OAL_FALSE,
    .feature_priv_closed_is_open = OAL_TRUE,
    .feature_hiex_is_open = OAL_FALSE,
    .rx_listen_ps_is_open = OAL_FALSE,
    .feature_11ax_is_open = OAL_FALSE,
    .feature_twt_is_open = OAL_FALSE,
    .feature_11ax_er_su_dcm_is_open = OAL_FALSE,
    .feature_11ax_uora_is_supported = OAL_FALSE,
    .feature_ftm_is_open = OAL_FALSE,
    .feature_psm_dfx_ext_is_open = OAL_TRUE,
    .feature_wow_opt_is_open  = OAL_FALSE,
    .feature_dual_wlan_is_supported = OAL_FALSE,
    .feature_ht_self_cure_is_open = OAL_TRUE,
    .feature_slave_ax_is_support = OAL_FALSE,
    .longer_than_16_he_sigb_support = OAL_FALSE,
    .p2p_go_max_user_num = WLAN_P2P_GO_ASSOC_USER_MAX_NUM_1103,
    .max_asoc_user = WLAN_ASSOC_USER_MAX_NUM_HI1103,
    .max_active_user = WLAN_ASSOC_USER_MAX_NUM_HI1103,
    .max_user_limit = WLAN_ASSOC_USER_MAX_NUM_HI1103 + WLAN_MULTI_USER_MAX_NUM_LIMIT,
    .invalid_user_id = WLAN_ASSOC_USER_MAX_NUM_HI1103 + WLAN_MULTI_USER_MAX_NUM_LIMIT,
    .max_tx_ba = WLAN_MAX_TX_BA_1103,
    .max_rx_ba = WLAN_MAX_RX_BA_1103,
    .other_bss_id = WLAN_HAL_OHTER_BSS_ID_1103,
    .max_tx_ampdu_num = WLAN_AMPDU_TX_MAX_NUM,
    .max_rf_num = 2,
    .feature_txq_ns_support = OAL_FALSE,
};

static const oal_wlan_cfg_stru g_wlan_spec_cfg_hi1105 = {
    .feature_m2s_modem_is_open = OAL_TRUE,
    .feature_priv_closed_is_open = OAL_FALSE,
    .feature_hiex_is_open = OAL_TRUE,
    .rx_listen_ps_is_open = OAL_TRUE,
    .feature_11ax_is_open = OAL_TRUE,
    .feature_twt_is_open = OAL_TRUE,
    .feature_11ax_er_su_dcm_is_open = OAL_TRUE,
    .feature_11ax_uora_is_supported = OAL_FALSE,
    .feature_ftm_is_open = OAL_TRUE,
    .feature_psm_dfx_ext_is_open = OAL_TRUE,
    .feature_wow_opt_is_open  = OAL_FALSE,
    .feature_dual_wlan_is_supported = OAL_TRUE,
    .feature_ht_self_cure_is_open = OAL_TRUE,
    .feature_slave_ax_is_support = OAL_FALSE,
    .longer_than_16_he_sigb_support = OAL_FALSE,
    .p2p_go_max_user_num = WLAN_P2P_GO_ASSOC_USER_MAX_NUM_1105,
    .max_asoc_user = WLAN_ASSOC_USER_MAX_NUM_HI1105,
    .max_active_user = WLAN_ASSOC_USER_MAX_NUM_HI1105,
    .max_user_limit = WLAN_ASSOC_USER_MAX_NUM_HI1105 + WLAN_MULTI_USER_MAX_NUM_LIMIT,
    .invalid_user_id = WLAN_ASSOC_USER_MAX_NUM_HI1105 + WLAN_MULTI_USER_MAX_NUM_LIMIT,
    .max_tx_ba = WLAN_MAX_TX_BA_1105,
    .max_rx_ba = WLAN_MAX_RX_BA_1105,
    .other_bss_id = WLAN_HAL_OHTER_BSS_ID_1105,
    .max_tx_ampdu_num = WLAN_AMPDU_TX_MAX_NUM,
    .max_rf_num = 2,
    .feature_txq_ns_support = OAL_FALSE,
};

static const oal_wlan_cfg_stru g_wlan_spec_cfg_hi1106 = {
    .feature_m2s_modem_is_open = OAL_TRUE,
    .feature_priv_closed_is_open = OAL_FALSE,
    .feature_hiex_is_open = OAL_TRUE,
    .rx_listen_ps_is_open = OAL_TRUE,
    .feature_11ax_is_open = OAL_TRUE,
    .feature_twt_is_open = OAL_TRUE,
    .feature_11ax_er_su_dcm_is_open = OAL_TRUE,
    .feature_11ax_uora_is_supported = OAL_TRUE,
    .feature_ftm_is_open = OAL_TRUE,
    .feature_psm_dfx_ext_is_open = OAL_TRUE,
    .feature_wow_opt_is_open  = OAL_FALSE,
    .feature_dual_wlan_is_supported = OAL_TRUE,
    .feature_ht_self_cure_is_open = OAL_FALSE,
    .feature_slave_ax_is_support = OAL_TRUE,
    .longer_than_16_he_sigb_support = OAL_TRUE,
    .p2p_go_max_user_num = WLAN_P2P_GO_ASSOC_USER_MAX_NUM_1106,  /* 1106 P2P最大接入用户数为8 */
    .max_asoc_user = WLAN_ASSOC_USER_MAX_NUM,
    .max_active_user = WLAN_ASSOC_USER_MAX_NUM,
    .max_user_limit = MAC_RES_MAX_USER_LIMIT,
    .invalid_user_id = MAC_RES_MAX_USER_LIMIT,
    .max_tx_ba = WLAN_MAX_TX_BA_1106,
    .max_rx_ba = WLAN_MAX_RX_BA_1106,
    .other_bss_id = WLAN_HAL_OHTER_BSS_ID_1106,
    .max_tx_ampdu_num = WLAN_AMPDU_TX_MAX_NUM_1106,
    .max_rf_num = 4,
    .feature_txq_ns_support = OAL_TRUE,
};

static const oal_wlan_cfg_stru g_wlan_spec_cfg_bisheng = {
    .feature_m2s_modem_is_open = OAL_TRUE,
    .feature_priv_closed_is_open = OAL_FALSE,
    .feature_hiex_is_open = OAL_TRUE,
    .rx_listen_ps_is_open = OAL_TRUE,
    .feature_11ax_is_open = OAL_TRUE,
    .feature_twt_is_open = OAL_TRUE,
    .feature_11ax_er_su_dcm_is_open = OAL_TRUE,
    .feature_11ax_uora_is_supported = OAL_TRUE,
    .feature_ftm_is_open = OAL_TRUE,
    .feature_psm_dfx_ext_is_open = OAL_TRUE,
    .feature_wow_opt_is_open  = OAL_FALSE,
    .feature_dual_wlan_is_supported = OAL_TRUE,
    .feature_ht_self_cure_is_open = OAL_FALSE,
    .feature_slave_ax_is_support = OAL_TRUE,
    .longer_than_16_he_sigb_support = OAL_TRUE,
    .p2p_go_max_user_num = WLAN_P2P_GO_ASSOC_USER_MAX_NUM_BISHENG,  /* BISHENG P2P最大接入用户数为8 */
    .max_asoc_user = WLAN_ASSOC_USER_MAX_NUM,
    .max_active_user = WLAN_ASSOC_USER_MAX_NUM,
    .max_user_limit = MAC_RES_MAX_USER_LIMIT,
    .invalid_user_id = MAC_RES_MAX_USER_LIMIT,
    .max_tx_ba = WLAN_MAX_TX_BA_BISHENG,
    .max_rx_ba = WLAN_MAX_RX_BA_BISHENG,
    .other_bss_id = WLAN_HAL_OHTER_BSS_ID_BISHENG,
    .max_tx_ampdu_num = WLAN_AMPDU_TX_MAX_NUM_BISHENG,
    .max_rf_num = 2,
};

#ifndef _PRE_LINUX_TEST
// 区分不同芯片host侧规格全局变量控制开关
const oal_wlan_cfg_stru *g_wlan_spec_cfg;
#else
// UT通过host侧产品宏控制指向对应全局变量
#if (_PRE_PRODUCT_VERSION == 1103)
oal_wlan_cfg_stru *g_wlan_spec_cfg = &g_wlan_spec_cfg_hi1103;
#elif (_PRE_PRODUCT_VERSION == 1105)
oal_wlan_cfg_stru *g_wlan_spec_cfg = &g_wlan_spec_cfg_hi1105;
#elif (_PRE_PRODUCT_VERSION == 1106)
oal_wlan_cfg_stru *g_wlan_spec_cfg = &g_wlan_spec_cfg_hi1106;
#elif (_PRE_PRODUCT_VERSION == bisheng)
oal_wlan_cfg_stru *g_wlan_spec_cfg = &g_wlan_spec_cfg_bisheng;
#else
// 其他芯片型号暂不处理，待适配
#endif
#endif

static int32_t wifi_feature_switch_cfg_init(void)
{
#ifndef _PRE_LINUX_TEST
    if (is_hisi_chiptype(BOARD_VERSION_HI1103)) {
        g_wlan_spec_cfg = &g_wlan_spec_cfg_hi1103;
    } else if (is_hisi_chiptype(BOARD_VERSION_HI1105)) {
        g_wlan_spec_cfg = &g_wlan_spec_cfg_hi1105;
    } else if (is_hisi_chiptype(BOARD_VERSION_HI1106)) {
        g_wlan_spec_cfg = &g_wlan_spec_cfg_hi1106;
    } else if (is_hisi_chiptype(BOARD_VERSION_BISHENG)) {
        g_wlan_spec_cfg = &g_wlan_spec_cfg_bisheng;
    } else {
        oal_io_print("hi110x wifi unsupport chitype!\n");
        return -OAL_EFAIL;
    }
#endif
    return OAL_SUCC;
}


int32_t hi110x_host_main_init(void)
{
    int32_t l_return;

    hcc_flowctl_get_device_mode_register(hmac_flowctl_check_device_is_sta_mode);
    hcc_flowctl_operate_subq_register(hmac_vap_net_start_subqueue, hmac_vap_net_stop_subqueue);
    /* wifi相关编译选项初始化设置 */
    l_return = wifi_feature_switch_cfg_init();
    if (l_return != OAL_SUCC) {
        oal_io_print("hi110x_host_main_init: host_module_init wifi_feature_switch_cfg_init fail\n");
        return l_return;
    }

    l_return = host_module_init();
    if (l_return != OAL_SUCC) {
        oal_io_print("hi110x_host_main_init: host_module_init return error code: %d\n", l_return);
        return l_return;
    }

    wal_hipriv_register_inetaddr_notifier();
    wal_hipriv_register_inet6addr_notifier();

    /* 启动完成后，输出打印 */
    oal_io_print("hi110x_host_main_init:: hi110x_host_main_init finish!\n");

    return OAL_SUCC;
}


void hi110x_host_main_exit(void)
{
    uint16_t us_bitmap;

    wal_hipriv_unregister_inetaddr_notifier();
    wal_hipriv_unregister_inet6addr_notifier();

    us_bitmap = BIT6 | BIT7 | BIT8;
    builder_module_exit(us_bitmap);

    return;
}

/*lint -e578*/ /*lint -e19*/
#ifndef CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT

#include "board.h"

int32_t g_wifi_init_flag = 0;
int32_t g_wifi_init_ret;
/* built-in */
OAL_STATIC ssize_t wifi_sysfs_set_init(struct kobject *dev, struct kobj_attribute *attr, const char *buf, size_t count)
{
    char mode[128] = { 0 }; /* array lenth 128 */

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((sscanf_s(buf, "%20s", mode, sizeof(mode)) != 1)) {
        oal_io_print("set value one param!\n");
        return -OAL_EINVAL;
    }

    if (sysfs_streq("init", mode)) {
        /* init */
        if (g_wifi_init_flag == 0) {
            g_wifi_init_ret = hi110x_host_main_init();
            g_wifi_init_flag = 1;
        } else {
            oal_io_print("double init!\n");
        }
    } else {
        oal_io_print("invalid input:%s\n", mode);
    }

    return count;
}

OAL_STATIC ssize_t wifi_sysfs_get_init(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (g_wifi_init_flag == 1) {
        if (g_wifi_init_ret == OAL_SUCC) {
            ret += snprintf_s(buf + ret, PAGE_SIZE - ret, (PAGE_SIZE - ret) - 1, "running\n");
        } else {
            ret += snprintf_s(buf + ret, PAGE_SIZE - ret, (PAGE_SIZE - ret) - 1,
                              "boot failed ret=%d\n", g_wifi_init_ret);
        }
    } else {
        ret += snprintf_s(buf + ret, PAGE_SIZE - ret, (PAGE_SIZE - ret) - 1, "uninit\n");
    }

    return ret;
}
STATIC struct kobj_attribute g_dev_attr_wifi =
    __ATTR(wifi, S_IRUGO | S_IWUSR, wifi_sysfs_get_init, wifi_sysfs_set_init);
OAL_STATIC struct attribute *g_wifi_init_sysfs_entries[] = {
    &g_dev_attr_wifi.attr,
    NULL
};

OAL_STATIC struct attribute_group g_wifi_init_attribute_group = {
    .attrs = g_wifi_init_sysfs_entries,
};

int32_t wifi_sysfs_init(void)
{
    int32_t ret;
    oal_kobject *pst_root_boot_object = NULL;

    if ((is_hisi_chiptype(BOARD_VERSION_HI1103) == false) &&
        (is_hisi_chiptype(BOARD_VERSION_HI1105) == false) &&
        (is_hisi_chiptype(BOARD_VERSION_HI1106) == false) &&
        (is_hisi_chiptype(BOARD_VERSION_BISHENG) == false)) {
        return OAL_SUCC;
    }

    pst_root_boot_object = oal_get_sysfs_root_boot_object();
    if (pst_root_boot_object == NULL) {
        oal_io_print("[E]get root boot sysfs object failed!\n");
        return -OAL_EBUSY;
    }

    ret = sysfs_create_group(pst_root_boot_object, &g_wifi_init_attribute_group);
    if (ret) {
        oal_io_print("sysfs create plat boot group fail.ret=%d\n", ret);
        ret = -OAL_ENOMEM;
        return ret;
    }

/* if TV && built-in, init dd here */
#ifdef _PRE_PRODUCT_HI3751V811
    oal_init_completion(hmac_get_wlan_cali_completed_addr());
#endif

    return ret;
}

void wifi_sysfs_exit(void)
{
    /* need't exit,built-in */
    return;
}
oal_module_init(wifi_sysfs_init);
oal_module_exit(wifi_sysfs_exit);
#endif
#else
oal_module_init(hi110x_host_main_init);
oal_module_exit(hi110x_host_main_exit);
#endif

oal_module_license("GPL");
/*lint +e578*/ /*lint +e19*/

