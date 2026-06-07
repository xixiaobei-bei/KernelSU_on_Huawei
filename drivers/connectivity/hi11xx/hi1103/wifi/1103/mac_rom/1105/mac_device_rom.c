

/* 1 头文件包含 */
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "hal_ext_if.h"
#include "mac_device.h"
#include "mac_resource.h"
#include "mac_regdomain.h"
#include "securec.h"
#include "securectype.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_DEVICE_ROM_C

/* 2 全局变量定义 */
#ifdef _PRE_WLAN_FEATURE_WMMAC
oal_bool_enum_uint8 g_en_wmmac_switch_etc = OAL_TRUE;
#endif

/* 动态/静态DBDC */
/* 这里指的是每个chip上mac device的频道能力 */
oal_uint8 g_auc_mac_device_radio_cap[WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP] = {
    MAC_DEVICE_2G_5G,
#if (WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP > 1)
    MAC_DEVICE_DISABLE
#endif
};

mac_board_stru *g_pst_mac_board = &g_st_mac_board;
mac_device_capability_stru *g_pst_mac_device_capability = &g_st_mac_device_capability[0];

/* 3 函数实现 */

wlan_mib_vht_supp_width_enum_uint8 mac_device_trans_bandwith_to_vht_capinfo(wlan_bw_cap_enum_uint8 en_max_op_bd)
{
    switch (en_max_op_bd) {
        case WLAN_BW_CAP_20M:
        case WLAN_BW_CAP_40M:
        case WLAN_BW_CAP_80M:
            return WLAN_MIB_VHT_SUPP_WIDTH_80;
        case WLAN_BW_CAP_160M:
            return WLAN_MIB_VHT_SUPP_WIDTH_160;
        case WLAN_BW_CAP_80PLUS80:
            return WLAN_MIB_VHT_SUPP_WIDTH_80PLUS80;
        default:
            OAM_ERROR_LOG1(0, OAM_SF_ANY,
                           "{mac_device_trans_bandwith_to_vht_capinfo::bandwith[%d] is invalid.}",
                           en_max_op_bd);
            return WLAN_MIB_VHT_SUPP_WIDTH_BUTT;
    }
}


oal_uint32 mac_device_check_5g_enable(oal_uint8 uc_device_id)
{
    oal_uint8        uc_device_id_per_chip;
    mac_device_stru *pst_mac_device;

    pst_mac_device = mac_res_get_dev_etc(uc_device_id);

    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "mac_device_check_5g_enable::get dev fail uc_device_id[%d]", uc_device_id);
        return OAL_FALSE;
    }

    /* 03两个业务device,00 01,取不同定制化,51双芯片00 11,取同一个定制化 */
    uc_device_id_per_chip = uc_device_id - pst_mac_device->uc_chip_id;

    return !!(g_auc_mac_device_radio_cap[uc_device_id_per_chip] & MAC_DEVICE_5G);
}
oal_uint32 mac_chip_init_etc(mac_chip_stru *pst_chip, oal_uint8 uc_device_max)
{
    pst_chip->uc_assoc_user_cnt = 0;
    pst_chip->uc_active_user_cnt = 0;

#ifdef _PRE_WLAN_FEATURE_RX_AGGR_EXTEND
    pst_chip->pst_rx_aggr_extend = (mac_chip_rx_aggr_extend_stru *)oal_memalloc(sizeof(mac_chip_rx_aggr_extend_stru));
    if (pst_chip->pst_rx_aggr_extend == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_data_acq_mem_alloc::puc_start_addr null.}");
        return OAL_FAIL;
    }
#endif

    /* 保存device数量 */
    pst_chip->uc_device_nums = uc_device_max;

    /* 初始化最后再将state置为TRUE */
    pst_chip->en_chip_state = OAL_TRUE;

    return OAL_SUCC;
}
oal_void mac_device_set_beacon_interval_etc(mac_device_stru *pst_mac_device, oal_uint32 ul_beacon_interval)
{
    pst_mac_device->ul_beacon_interval = ul_beacon_interval;
}

oal_void mac_chip_inc_assoc_user(mac_chip_stru *pst_mac_chip)
{
    pst_mac_chip->uc_assoc_user_cnt++;
    OAM_WARNING_LOG1(0, OAM_SF_UM, "{mac_chip_inc_assoc_user::uc_asoc_user_cnt[%d].}", pst_mac_chip->uc_assoc_user_cnt);
    if (pst_mac_chip->uc_assoc_user_cnt == 0xFF) {
        OAM_ERROR_LOG0(0, OAM_SF_UM, "{mac_chip_inc_assoc_user::uc_asoc_user_cnt=0xFF now!}");
        oam_report_backtrace();
    }
}

oal_void mac_chip_dec_assoc_user(mac_chip_stru *pst_mac_chip)
{
    OAM_WARNING_LOG1(0, OAM_SF_UM, "{mac_chip_dec_assoc_user::uc_asoc_user_cnt[%d].}", pst_mac_chip->uc_assoc_user_cnt);
    if (pst_mac_chip->uc_assoc_user_cnt == 0) {
        OAM_ERROR_LOG0(0, OAM_SF_UM, "{mac_chip_dec_assoc_user::uc_assoc_user_cnt is already zero.}");
        oam_report_backtrace();
    } else {
        pst_mac_chip->uc_assoc_user_cnt--;
    }
}

oal_void mac_chip_inc_active_user(mac_chip_stru *pst_mac_chip)
{
    pst_mac_chip->uc_active_user_cnt++;
}

oal_void mac_chip_dec_active_user(mac_chip_stru *pst_mac_chip)
{
    if (pst_mac_chip->uc_active_user_cnt == 0) {
        OAM_ERROR_LOG0(0, OAM_SF_UM, "{mac_chip_dec_active_user::uc_active_user_cnt is already zero.}");
    } else {
        pst_mac_chip->uc_active_user_cnt--;
    }
}


oal_void *mac_device_get_all_rates_etc(mac_device_stru *pst_dev)
{
    return (oal_void *)pst_dev->st_mac_rates_11g;
}

#ifdef _PRE_WLAN_FEATURE_DFS

oal_void mac_dfs_set_cac_enable(mac_device_stru *pst_mac_device, oal_bool_enum_uint8 en_val)
{
    pst_mac_device->st_dfs.st_dfs_info.en_cac_switch = en_val;
}





oal_void mac_dfs_set_offchan_number(mac_device_stru *pst_mac_device, oal_uint32 ul_val)
{
    pst_mac_device->st_dfs.st_dfs_info.uc_offchan_num = (oal_uint8)ul_val;
}




oal_void mac_dfs_set_dfs_enable(mac_device_stru *pst_mac_device, oal_bool_enum_uint8 en_val)
{
    pst_mac_device->st_dfs.st_dfs_info.en_dfs_switch = en_val;

    /* 如果 软件雷达检测使能 关闭，则关闭CAC检测 */
    if (en_val == OAL_FALSE) {
        pst_mac_device->st_dfs.st_dfs_info.en_cac_switch = OAL_FALSE;
    }
}


oal_bool_enum_uint8 mac_dfs_get_dfs_enable(mac_device_stru *pst_mac_device)
{
    if (pst_mac_device->en_max_band == WLAN_BAND_5G) {
        return pst_mac_device->st_dfs.st_dfs_info.en_dfs_switch;
    }

    return OAL_FALSE;
}


oal_void mac_dfs_set_debug_level(mac_device_stru *pst_mac_device, oal_uint8 uc_debug_lev)
{
    pst_mac_device->st_dfs.st_dfs_info.uc_debug_level = uc_debug_lev;
}


oal_void mac_dfs_set_cac_time(mac_device_stru *pst_mac_device, oal_uint32 ul_time_ms, oal_bool_enum_uint8 en_waether)
{
    if (en_waether) {
        pst_mac_device->st_dfs.st_dfs_info.ul_dfs_cac_in_5600_to_5650_time_ms = ul_time_ms;
    } else {
        pst_mac_device->st_dfs.st_dfs_info.ul_dfs_cac_outof_5600_to_5650_time_ms = ul_time_ms;
    }
}



oal_void mac_dfs_set_opern_chan_time(mac_device_stru *pst_mac_device, oal_uint32 ul_time_ms)
{
    pst_mac_device->st_dfs.st_dfs_info.us_dfs_off_chan_cac_opern_chan_dwell_time = (oal_uint16)ul_time_ms;
}


oal_void mac_dfs_set_off_chan_time(mac_device_stru *pst_mac_device, oal_uint32 ul_time_ms)
{
    pst_mac_device->st_dfs.st_dfs_info.us_dfs_off_chan_cac_off_chan_dwell_time = (oal_uint16)ul_time_ms;
}

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

oal_void mac_dfs_set_offchan_cac_enable(mac_device_stru *pst_mac_device, oal_bool_enum_uint8 en_val)
{
    pst_mac_device->st_dfs.st_dfs_info.en_offchan_cac_switch = en_val;
}


oal_bool_enum_uint8 mac_dfs_get_offchan_cac_enable(mac_device_stru *pst_mac_device)
{
    mac_regdomain_info_stru *pst_rd_info = OAL_PTR_NULL;

    mac_get_regdomain_info_etc(&pst_rd_info);
    if (pst_rd_info->en_dfs_domain == MAC_DFS_DOMAIN_ETSI) {
        return pst_mac_device->st_dfs.st_dfs_info.en_offchan_cac_switch;
    }

    return OAL_FALSE;
}



oal_bool_enum_uint8 mac_dfs_get_cac_enable(mac_device_stru *pst_mac_device)
{
    return pst_mac_device->st_dfs.st_dfs_info.en_cac_switch;
}



oal_uint8 mac_dfs_get_debug_level(mac_device_stru *pst_mac_device)
{
    return pst_mac_device->st_dfs.st_dfs_info.uc_debug_level;
}



oal_void mac_dfs_set_off_cac_time(mac_device_stru *pst_mac_device,
                                  oal_uint32 ul_time_ms,
                                  oal_bool_enum_uint8 en_waether)
{
    if (en_waether) {
        pst_mac_device->st_dfs.st_dfs_info.ul_off_chan_cac_in_5600_to_5650_time_ms = ul_time_ms;
    } else {
        pst_mac_device->st_dfs.st_dfs_info.ul_off_chan_cac_outof_5600_to_5650_time_ms = ul_time_ms;
    }
}



oal_void mac_dfs_set_next_radar_ch(mac_device_stru *pst_mac_device,
                                   oal_uint8 uc_ch,
                                   wlan_channel_bandwidth_enum_uint8 en_width)
{
    pst_mac_device->st_dfs.st_dfs_info.en_next_ch_width_type = en_width;
    pst_mac_device->st_dfs.st_dfs_info.uc_custom_next_chnum = uc_ch;
}


oal_void mac_dfs_set_ch_bitmap(mac_device_stru *pst_mac_device, oal_uint32 ul_ch_bitmap)
{
    pst_mac_device->st_dfs.st_dfs_info.ul_custom_chanlist_bitmap = ul_ch_bitmap;
}
#endif


oal_void mac_dfs_set_non_occupancy_period_time(mac_device_stru *pst_mac_device, oal_uint32 ul_time)
{
    pst_mac_device->st_dfs.st_dfs_info.ul_dfs_non_occupancy_period_time_ms = ul_time;
}

#endif

/*lint -e19*/
oal_module_symbol(mac_device_set_beacon_interval_etc);
oal_module_symbol(mac_chip_inc_active_user);
oal_module_symbol(mac_chip_dec_active_user);
oal_module_symbol(mac_chip_inc_assoc_user);
oal_module_symbol(mac_chip_dec_assoc_user);
oal_module_symbol(mac_chip_init_etc);

oal_module_symbol(mac_device_check_5g_enable);
oal_module_symbol(g_auc_mac_device_radio_cap);
oal_module_symbol(g_pst_mac_board);

#ifdef _PRE_WLAN_FEATURE_DFS
oal_module_symbol(mac_dfs_set_cac_enable);
oal_module_symbol(mac_dfs_set_offchan_cac_enable);
oal_module_symbol(mac_dfs_get_offchan_cac_enable);
oal_module_symbol(mac_dfs_set_offchan_number);
oal_module_symbol(mac_dfs_get_cac_enable);
oal_module_symbol(mac_dfs_set_dfs_enable);
oal_module_symbol(mac_dfs_get_dfs_enable);
oal_module_symbol(mac_dfs_set_debug_level);
oal_module_symbol(mac_dfs_get_debug_level);
oal_module_symbol(mac_dfs_set_cac_time);
oal_module_symbol(mac_dfs_set_off_cac_time);
oal_module_symbol(mac_dfs_set_opern_chan_time);
oal_module_symbol(mac_dfs_set_off_chan_time);
oal_module_symbol(mac_dfs_set_next_radar_ch);
oal_module_symbol(mac_dfs_set_ch_bitmap);
oal_module_symbol(mac_dfs_set_non_occupancy_period_time);
#endif /*lint +e19*/
