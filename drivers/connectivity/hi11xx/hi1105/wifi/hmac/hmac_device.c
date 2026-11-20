

#include "oam_ext_if.h"
#include "frw_ext_if.h"

#include "wlan_chip_i.h"
#include "wlan_chip.h"
#include "mac_device.h"
#include "mac_resource.h"
#include "mac_regdomain.h"
#include "mac_vap.h"

#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_vap.h"
#include "hmac_rx_filter.h"

#include "hmac_chan_mgmt.h"
#include "hmac_dfs.h"
#include "hmac_rx_filter.h"

#include "hmac_hcc_adapt.h"

#include "hmac_dfs.h"
#include "hmac_config.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_scan.h"
#include "hmac_rx_data.h"
#include "hmac_hcc_adapt.h"
#include "hmac_dfx.h"
#include "hmac_protection.h"
#include "hmac_stat.h"
#ifdef _PRE_WLAN_TCP_OPT
#include "mac_data.h"
#include "hmac_tcp_opt_struc.h"
#include "hmac_tcp_opt.h"
#endif
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/fb.h>
#include <linux/list.h>
#include "plat_pm_wlan.h"
#endif

#include "host_hal_device.h"

#include "securec.h"
#include "securectype.h"
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
#include "hmac_tcp_ack_buf.h"
#endif

#include "hisi_customize_wifi.h"
#ifdef _PRE_WLAN_FEATURE_MONITOR
#include "hmac_monitor.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_DEVICE_C

/* 2 全局变量定义 */
/*lint -e578*/ /*lint -e19*/
oal_module_license("GPL");
/*lint +e578*/ /*lint +e19*/

/* 3 函数实现 */

uint32_t hmac_device_exit(mac_board_stru *pst_board, mac_chip_stru *pst_chip, hmac_device_stru *pst_hmac_device)
{
    mac_device_stru *pst_device = NULL;
    uint32_t ret;
    hmac_vap_stru *pst_vap = NULL;
    mac_vap_stru *mac_vap = NULL;
    mac_cfg_down_vap_param_stru st_down_vap;
    uint8_t uc_vap_idx;

    if (pst_hmac_device == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_device_exit::pst_hmac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 扫描模块去初始化 */
    hmac_scan_exit(pst_hmac_device);

    pst_device = pst_hmac_device->pst_device_base_info;
    if (pst_device == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_device_exit::pst_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 亮暗屏去注册 */
#ifdef CONFIG_HAS_EARLYSUSPEND
    unregister_early_suspend(&pst_hmac_device->early_suspend);
#endif

    /* 由于配置vap初始化在HMAC做，所以配置VAP卸载也在HMAC做 */
    pst_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_device->uc_cfg_vap_id);
    if (pst_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_device_exit::pst_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_vap = &pst_vap->st_vap_base_info;
    ret = hmac_config_del_vap(mac_vap, sizeof(mac_cfg_down_vap_param_stru), (uint8_t *)&st_down_vap);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_device_exit::hmac_config_del_vap failed[%d].}", ret);
        return ret;
    }

    for (uc_vap_idx = 0; uc_vap_idx < pst_device->uc_vap_num; uc_vap_idx++) {
        /* 获取最右边一位为1的位数，此值即为vap的数组下标 */
        pst_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == NULL) {
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_device_exit::mac_res_get_hmac_vap failed vap_idx[%u].}", uc_vap_idx);
            continue;
        }

        ret = hmac_vap_destroy(pst_vap);
        if (ret != OAL_SUCC) {
            oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_device_exit::hmac_vap_destroy failed[%d].}", ret);
            return ret;
        }
        pst_device->auc_vap_id[uc_vap_idx] = 0;
    }

    /* 释放公共结构体 以及 对应衍生特性 */
    ret = mac_device_exit(pst_device);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{mac_chip_exit::p_device_destroy_fun failed[%d].}", ret);
        return ret;
    }

    /* 指向基础mac device的指针为空 */
    pst_hmac_device->pst_device_base_info = NULL;

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_chip_exit(mac_board_stru *pst_board, mac_chip_stru *pst_chip)
{
    hmac_device_stru *pst_hmac_device = NULL;
    uint32_t ret;
    uint8_t uc_device;

    if (oal_unlikely(oal_any_null_ptr2(pst_chip, pst_board))) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_chip_exit::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (uc_device = 0; uc_device < pst_chip->uc_device_nums; uc_device++) {
        pst_hmac_device = hmac_res_get_mac_dev(pst_chip->auc_device_id[uc_device]);

        /* 待挪动位置 释放资源 */
        hmac_res_free_mac_dev(pst_chip->auc_device_id[uc_device]);

        ret = hmac_device_exit(pst_board, pst_chip, pst_hmac_device);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_chip_exit::hmac_device_exit failed[%d].}", ret);
            return ret;
        }
    }

    ret = mac_chip_exit(pst_board, pst_chip);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_chip_exit::mac_chip_exit failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}


uint32_t hmac_board_exit(mac_board_stru *pst_board)
{
    uint8_t uc_chip_idx;
    uint32_t ret;
    uint8_t uc_chip_id_bitmap;

    if (oal_unlikely(pst_board == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_board_exit::pst_board null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    uc_chip_id_bitmap = pst_board->uc_chip_id_bitmap;
    while (uc_chip_id_bitmap != 0) {
        /* 获取最右边一位为1的位数，此值即为chip的数组下标 */
        uc_chip_idx = oal_bit_find_first_bit_one_byte(uc_chip_id_bitmap);
        if (oal_unlikely(uc_chip_idx >= WLAN_CHIP_MAX_NUM_PER_BOARD)) {
            oam_error_log2(0, OAM_SF_ANY, "{hmac_board_exit::invalid uc_chip_idx[%d] uc_chip_id_bitmap=%d.}",
                           uc_chip_idx, uc_chip_id_bitmap);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }

        ret = hmac_chip_exit(pst_board, &(pst_board->ast_chip[uc_chip_idx]));
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_board_exit::mac_chip_exit failed[%d].}", ret);
            return ret;
        }

        /* 清除对应的bitmap位 */
        oal_bit_clear_bit_one_byte(&uc_chip_id_bitmap, uc_chip_idx);
        /* 清除对应的bitmap位 */
        oal_bit_clear_bit_one_byte(&pst_board->uc_chip_id_bitmap, uc_chip_idx);
    }

    /* 公共部分的初始化 */
    mac_board_exit(pst_board);

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_cfg_vap_init(mac_device_stru *pst_device)
{
    int8_t ac_vap_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    uint32_t ret;
    hmac_vap_stru *pst_vap = NULL;

    /* 初始化流程中，只初始化配置vap，其他vap需要通过配置添加 */
    /*lint -e413*/
    ret = mac_res_alloc_hmac_vap(&pst_device->uc_cfg_vap_id, OAL_OFFSET_OF(hmac_vap_stru, st_vap_base_info));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_device->uc_cfg_vap_id, OAM_SF_ANY,
                         "{hmac_cfg_vap_init::mac_res_alloc_hmac_vap failed[%d].}", ret);
        return ret;
    }
    /*lint +e413*/
    pst_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_device->uc_cfg_vap_id);
    if (pst_vap == NULL) {
        oam_warning_log0(pst_device->uc_cfg_vap_id, OAM_SF_ANY, "{hmac_cfg_vap_init::pst_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 把hmac_vap_stru结构体初始化赋值为0 */
    memset_s(pst_vap, sizeof(hmac_vap_stru), 0, sizeof(hmac_vap_stru));

    {
        mac_cfg_add_vap_param_stru st_param = { 0 }; /* 构造配置VAP参数结构体 */
        st_param.en_vap_mode = WLAN_VAP_MODE_CONFIG;
        st_param.bit_11ac2g_enable = OAL_TRUE;
        st_param.bit_disable_capab_2ght40 = OAL_FALSE;
        ret = hmac_vap_init(pst_vap,
                            pst_device->uc_chip_id,
                            pst_device->uc_device_id,
                            pst_device->uc_cfg_vap_id,
                            &st_param);
        if (ret != OAL_SUCC) {
            oam_warning_log1(pst_device->uc_cfg_vap_id, OAM_SF_ANY,
                             "{hmac_cfg_vap_init::hmac_vap_init failed[%d].}", ret);
            return ret;
        }
    }

    snprintf_s(ac_vap_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, MAC_NET_DEVICE_NAME_LENGTH - 1,
               "Hisilicon%d", pst_device->uc_chip_id);

    ret = hmac_vap_creat_netdev(pst_vap, ac_vap_netdev_name,
                                sizeof(ac_vap_netdev_name),
                                (int8_t *)(pst_device->auc_hw_addr));
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_device->uc_cfg_vap_id, OAM_SF_ANY,
                         "{hmac_cfg_vap_init::hmac_vap_creat_netdev failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

void hmac_do_suspend_action(mac_device_stru *pst_mac_device, uint8_t uc_in_suspend)
{
    hmac_device_stru *pst_hmac_device;
    uint32_t ret;
    mac_vap_stru *pst_cfg_mac_vap = NULL;
    mac_cfg_suspend_stru st_suspend;
    uint32_t is_wlan_poweron;

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_warning_log0(pst_mac_device->uc_cfg_vap_id, OAM_SF_ANY,
                         "{hmac_do_suspend_action::pst_hmac_device null.}");
        return;
    }

    oal_spin_lock(&pst_hmac_device->st_suspend_lock);

    pst_mac_device->uc_in_suspend = uc_in_suspend;

    pst_cfg_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->uc_cfg_vap_id);
    if (pst_cfg_mac_vap == NULL) {
        oam_warning_log0(pst_mac_device->uc_cfg_vap_id, OAM_SF_ANY, "{hmac_do_suspend_action::pst_vap null.}");
        oal_spin_unlock(&pst_hmac_device->st_suspend_lock);
        return;
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    is_wlan_poweron = wlan_pm_is_poweron();
#endif
    /* 开了host低功耗并且device已上电才需要将亮暗屏状态同步到device */
    if (g_wlan_host_pm_switch && is_wlan_poweron) {
        st_suspend.uc_in_suspend = uc_in_suspend;

        if ((uc_in_suspend == OAL_TRUE)
#ifdef _PRE_WLAN_FEATURE_WAPI
            && (pst_hmac_device->pst_device_base_info->uc_wapi != OAL_TRUE)
#endif
        ) {
            pst_hmac_device->pst_device_base_info->uc_arpoffload_switch = OAL_TRUE;
            st_suspend.uc_arpoffload_switch = OAL_TRUE;
        } else {
            pst_hmac_device->pst_device_base_info->uc_arpoffload_switch = OAL_FALSE;
            st_suspend.uc_arpoffload_switch = OAL_FALSE;
        }
        hmac_wake_lock();
        /***************************************************************************
            抛事件到DMAC层, 同步屏幕最新状态到DMAC
        ***************************************************************************/
        ret = hmac_config_send_event(pst_cfg_mac_vap,
                                     WLAN_CFGID_SUSPEND_ACTION_SYN,
                                     sizeof(mac_cfg_suspend_stru),
                                     (uint8_t *)&st_suspend);
        if (oal_unlikely(ret != OAL_SUCC)) {
            oam_warning_log1(0, OAM_SF_CFG, "{hmac_suspend_action::hmac_config_send_event failed[%d]}", ret);
        }
        hmac_wake_unlock();
    }
    oal_spin_unlock(&pst_hmac_device->st_suspend_lock);
}

#ifdef CONFIG_HAS_EARLYSUSPEND

void hmac_early_suspend(struct early_suspend *early_sup)
{
    hmac_device_stru *pst_hmac_device;

    pst_hmac_device = oal_container_of(early_sup, hmac_device_stru, early_suspend);
    hmac_do_suspend_action(pst_hmac_device->pst_device_base_info, OAL_TRUE);
}


void hmac_late_resume(struct early_suspend *early_sup)
{
    hmac_device_stru *pst_hmac_device;

    pst_hmac_device = oal_container_of(early_sup, hmac_device_stru, early_suspend);
    hmac_do_suspend_action(pst_hmac_device->pst_device_base_info, OAL_FALSE);
}
#endif
#endif  // end (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

uint32_t hmac_send_evt2wal(mac_vap_stru *pst_mac_vap, uint8_t uc_evtid,
    uint8_t *puc_evt, uint32_t evt_len)
{
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event = NULL;
    uint32_t ret;

    pst_event_mem = frw_event_alloc_m((uint16_t)evt_len);
    if (pst_event_mem == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_mgmt_scan_req_exception::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写事件 */
    pst_event = frw_get_event_stru(pst_event_mem);

    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CTX,
                       uc_evtid,
                       (uint16_t)evt_len,
                       FRW_EVENT_PIPELINE_STAGE_0,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    if (EOK != memcpy_s((void *)pst_event->auc_event_data, evt_len, (void *)puc_evt, evt_len)) {
        oam_error_log0(0, OAM_SF_SCAN, "hmac_send_evt2wal::memcpy fail!");
        frw_event_free_m(pst_event_mem);
        return OAL_FAIL;
    }

    /* 分发事件 */
    ret = frw_event_dispatch_event(pst_event_mem);
    frw_event_free_m(pst_event_mem);
    return ret;
}

uint32_t hmac_config_host_dev_init(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_device_stru *pst_mac_device = NULL;
    uint32_t loop;
    hmac_device_stru *pst_hmac_device = NULL;

    if (pst_mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_device_init:: pst_mac_device NULL pointer!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_device_init:: pst_mac_device[%d] NULL pointer!}",
                       pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (loop = 0; loop < MAC_MAX_SUPP_CHANNEL; loop++) {
        pst_mac_device->st_ap_channel_list[loop].us_num_networks = 0;
        pst_mac_device->st_ap_channel_list[loop].en_ch_type = MAC_CH_TYPE_NONE;
    }

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_hmac_device == NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_config_host_dev_init::pst_hmac_device[%d] null!}",
                       pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    return OAL_SUCC;
}


uint32_t hmac_config_host_dev_exit(mac_vap_stru *pst_mac_vap)
{
    hmac_device_stru *pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_hmac_device == NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_config_host_dev_exit::pst_hmac_device[%d] null!}",
                       pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef CONFIG_ARCH_KIRIN_PCIE
#ifndef _PRE_LINUX_TEST
    /* wifi 下电过程中释放rx ring相关资源 */
    hal_device_reset_rx_res();
#endif
#endif
    return OAL_SUCC;
}

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

void hmac_device_cap_init_customize(hmac_device_stru *pst_hmac_dev)
{
    pst_hmac_dev->en_ap_support_160m = wlan_chip_is_aput_support_160m();
    oam_warning_log1(0, 0, "hmac_device_cap_init_customize::APUT 160M enable[%d]", pst_hmac_dev->en_ap_support_160m);
}
#endif


OAL_STATIC void hmac_device_init_ext(hmac_device_stru *pst_hmac_device)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_spin_lock_init(&pst_hmac_device->st_suspend_lock);
#ifdef CONFIG_HAS_EARLYSUSPEND
    pst_hmac_device->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 20; /* 20 level value */
    pst_hmac_device->early_suspend.suspend = hmac_early_suspend;
    pst_hmac_device->early_suspend.resume = hmac_late_resume;
    register_early_suspend(&pst_hmac_device->early_suspend);
#endif
    oal_spin_lock(&pst_hmac_device->st_suspend_lock);
    pst_hmac_device->pst_device_base_info->uc_in_suspend = OAL_FALSE;
    oal_spin_unlock(&pst_hmac_device->st_suspend_lock);
#endif

    /* 扫描模块初始化 */
    hmac_scan_init(pst_hmac_device);

    /* 初始化P2P 等待队列 */
    oal_wait_queue_init_head(&(pst_hmac_device->st_netif_change_event));
    /* 初始化低功耗帧过滤统计数据查询等待队列 */
    oal_wait_queue_init_head(&(pst_hmac_device->st_psm_flt_stat_query.st_wait_queue));
#ifdef _PRE_WLAN_TCP_OPT
    pst_hmac_device->sys_tcp_tx_ack_opt_enable = DEFAULT_TX_TCP_ACK_OPT_ENABLE;
    pst_hmac_device->sys_tcp_rx_ack_opt_enable = DEFAULT_RX_TCP_ACK_OPT_ENABLE;
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    hmac_device_cap_init_customize(pst_hmac_device);
#endif
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
    hmac_tcp_ack_buf_init_para();
#endif
    hmac_stat_init_device_stat(pst_hmac_device);

#ifdef _PRE_WLAN_FEATURE_MONITOR
    hmac_monitor_init(pst_hmac_device);
#endif
}


uint32_t hmac_device_init(uint8_t *puc_device_id, mac_chip_stru *pst_chip)
{
    uint8_t uc_dev_id;
    mac_device_stru *pst_mac_device = NULL;
    hmac_device_stru *pst_hmac_device = NULL;
    uint32_t ret, loop;

    /* 申请公共mac device结构体 */
    ret = mac_res_alloc_hmac_dev(&uc_dev_id);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_device_init::mac_res_alloc_dmac_dev failed[%d].}", ret);
        return OAL_FAIL;
    }

    /* 获取mac device结构体指针 */
    pst_mac_device = mac_res_get_dev(uc_dev_id);
    if (pst_mac_device == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_device_init::pst_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = mac_device_init(pst_mac_device, pst_chip->chip_ver, pst_chip->uc_chip_id, uc_dev_id);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ANY, "{hmac_device_init::mac_device_init failed[%d], chip_ver[0x%x].}",
            ret, pst_chip->chip_ver);
        mac_res_free_dev(uc_dev_id);
        return ret;
    }

    /* 申请hmac device资源 */
    if (oal_unlikely(hmac_res_alloc_mac_dev(uc_dev_id) != OAL_SUCC)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_device_init::hmac_res_alloc_mac_dev failed.}");
        return OAL_FAIL;
    }

    /* 获取hmac device，并进行相关参数赋值 */
    pst_hmac_device = hmac_res_get_mac_dev(uc_dev_id);
    if (oal_unlikely(pst_hmac_device == NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_device_init::pst_hmac_device[%] null!}", uc_dev_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 结构体初始化 */
    memset_s(pst_hmac_device, sizeof(hmac_device_stru), 0, sizeof(hmac_device_stru));

    pst_hmac_device->pst_device_base_info = pst_mac_device;

    hmac_device_init_ext(pst_hmac_device);

    for (loop = 0; loop < MAC_MAX_SUPP_CHANNEL; loop++) {
        pst_mac_device->st_ap_channel_list[loop].us_num_networks = 0;
        pst_mac_device->st_ap_channel_list[loop].en_ch_type = MAC_CH_TYPE_NONE;
    }

    /* 出参赋值，CHIP中需要保存该device id */
    *puc_device_id = uc_dev_id;

    /* 配置vap初始化 */
    ret = hmac_cfg_vap_init(pst_mac_device);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_chip_init::hmac_cfg_vap_init failed[%d].}", ret);
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_DFS
    hmac_dfs_init(pst_mac_device);
#endif

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_chip_init(mac_chip_stru *pst_chip, uint8_t uc_chip_id)
{
    uint8_t uc_device;
    uint32_t ret;
    uint8_t uc_device_max;

    oam_info_log0(0, OAM_SF_ANY, "{hmac_chip_init::func enter.}");

    pst_chip->uc_chip_id = uc_chip_id;

    /* OAL接口获取支持device个数 */
    uc_device_max = oal_chip_get_device_num(pst_chip->chip_ver);
    if (uc_device_max == 0) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_chip_init::device max num is zero.}");
        return OAL_FAIL;
    }

    if (uc_device_max > WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP) {
        oam_error_log2(0, OAM_SF_ANY, "{hmac_chip_init::device max num is %d,more than %d.}",
                       uc_device_max, WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP);
        return OAL_FAIL;
    }

    for (uc_device = 0; uc_device < uc_device_max; uc_device++) {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
        if (g_auc_mac_device_radio_cap[uc_device] == MAC_DEVICE_CAP_DISABLE) {
            oam_error_log1(0, OAM_SF_ANY, "{hmac_chip_init::mac device id[%d] disable.}", uc_device);
            continue;
        }
#endif  // #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
        /* hmac device结构初始化 */
        ret = hmac_device_init(&pst_chip->auc_device_id[uc_device], pst_chip);
        if (oal_unlikely(ret != OAL_SUCC)) {
            oam_error_log1(0, OAM_SF_ANY, "{hmac_chip_init::hmac_device_init failed[%d].}", ret);
            return OAL_FAIL;
        }
    }

    mac_chip_init(pst_chip, uc_device_max);

    oam_info_log0(0, OAM_SF_ANY, "{hmac_chip_init::func out.}");

    return OAL_SUCC;
}


uint32_t hmac_board_init(mac_board_stru *pst_board)
{
    uint8_t uc_chip;
    uint32_t ret;
    uint32_t chip_max_num;

    mac_board_init();

    /* chip支持的最大数由PCIe总线处理提供; */
    chip_max_num = oal_bus_get_chip_num();

    for (uc_chip = 0; uc_chip < chip_max_num; uc_chip++) {
        ret = hmac_chip_init(&pst_board->ast_chip[uc_chip], uc_chip);
        if (ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_ANY,
                             "{hmac_init_event_process::hmac_chip_init failed[%d], uc_chip_id[%d].}",
                             ret, uc_chip);
            return OAL_FAIL;
        }

        oal_bit_set_bit_one_byte(&pst_board->uc_chip_id_bitmap, uc_chip);
    }
    return OAL_SUCC;
}


void hmac_device_create_random_mac_addr(mac_device_stru *pst_mac_dev, mac_vap_stru *pst_mac_vap)
{
    hmac_device_stru *pst_hmac_dev = NULL;
    oal_bool_enum_uint8 en_is_random_mac_addr_scan;

    if (!IS_LEGACY_STA(pst_mac_vap)) {
        return;
    }

    pst_hmac_dev = hmac_res_get_mac_dev(pst_mac_dev->uc_device_id);
    if (pst_hmac_dev == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, 0, "hmac_device_create_random_mac_addr::hmac device is null");
        return;
    }

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    en_is_random_mac_addr_scan = g_wlan_cust.uc_random_mac_addr_scan;
#else
    en_is_random_mac_addr_scan = pst_hmac_dev->st_scan_mgmt.en_is_random_mac_addr_scan;
#endif

    if (en_is_random_mac_addr_scan == OAL_FALSE) {
        return;
    }

    oal_random_ether_addr(pst_hmac_dev->st_scan_mgmt.auc_random_mac);
    pst_hmac_dev->st_scan_mgmt.auc_random_mac[0] = pst_mac_dev->auc_mac_oui[0] & 0xfe; /* 保证是单播mac */
    pst_hmac_dev->st_scan_mgmt.auc_random_mac[1] = pst_mac_dev->auc_mac_oui[1];
    pst_hmac_dev->st_scan_mgmt.auc_random_mac[2] = pst_mac_dev->auc_mac_oui[2];  // 将mac地址oui的2字节值赋值给随机mac的2字节

    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                     "{hmac_device_create_random_mac_addr::rand_mac_addr[%02X:XX:XX:%02X:%02X:%02X].}",
                     pst_hmac_dev->st_scan_mgmt.auc_random_mac[0],
                     pst_hmac_dev->st_scan_mgmt.auc_random_mac[3],   // 随机MAC的3字节
                     pst_hmac_dev->st_scan_mgmt.auc_random_mac[4],   // 随机MAC的4字节
                     pst_hmac_dev->st_scan_mgmt.auc_random_mac[5]);  // 随机MAC的5字节

    return;
}
