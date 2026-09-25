

/* 1 头文件包含 */
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "hal_ext_if.h"
#include "mac_device.h"
#include "mac_resource.h"
#include "mac_regdomain.h"
#ifdef _PRE_WLAN_FEATURE_HIEX
#include "mac_hiex.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_DEVICE_C

/* 2 全局变量定义 */
/* mac模块板子的全局控制变量 */
mac_board_stru g_st_mac_board;
#ifdef _PRE_WLAN_FEATURE_IP_FILTER
mac_rx_icmp_filter_struc g_st_icmp_filter;
#endif

oal_uint32 g_ul_ddr_freq = 0;
mac_device_custom_cfg_stru g_st_mac_device_custom_cfg;
oal_uint8 g_uc_ddr_custom_switch = 0;

/* 每个chip下的mac device部分能力定制化 */
/* 02和51每个chip下只有1个device，51双芯片时每个chip的能力是相等的 */
/* 03 DBDC开启时mac device个数取2(考虑静态DBDC), 否则取1 */
/* 默认初始化值为动态DBDC，取HAL Device0的能力赋值 */
mac_device_capability_stru g_st_mac_device_capability[WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP] = {
    {
        /* nss num */
        WLAN_HAL0_NSS_NUM,      /* 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
        /* mib_bw_supp_width */
        WLAN_HAL0_BW_MAX_WIDTH, /* TBD:03 ini FPGA 20_40M,ASIC 160M */
        /* NB */
        WLAN_HAL0_NB_IS_EN,
        /* 1024QAM */
        WLAN_HAL0_1024QAM_IS_EN,

        /* 80211 MC */
        WLAN_HAL0_11MC_IS_EN,
        /* ldpc coding */
        WLAN_HAL0_LDPC_IS_EN,
        /* tx stbc */
        WLAN_HAL0_TX_STBC_IS_EN, /* 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
        /* rx stbc */
        WLAN_HAL0_RX_STBC_IS_EN, /* 该值需与hal device保持一致，定制化打开后须统一刷成一致 */

        /* su bfer */
        WLAN_HAL0_SU_BFER_IS_EN, /* 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
        /* su bfee */
        WLAN_HAL0_SU_BFEE_IS_EN, /* 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
        /* mu bfer */
        WLAN_HAL0_MU_BFER_IS_EN, /* 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
        /* mu bfee */
        WLAN_HAL0_MU_BFEE_IS_EN, /* 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
        /* 11ax */
        WLAN_HAL0_11AX_IS_EN,
    },
#if (WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP > 1)
    {
        0,
    }
#endif
};
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_BOARD)
mac_blacklist_info_stru g_ast_blacklist;
#elif (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_CHIP)
mac_blacklist_info_stru g_ast_blacklist[WLAN_CHIP_MAX_NUM_PER_BOARD];
#elif (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_DEVICE)
mac_blacklist_info_stru g_ast_blacklist[WLAN_SERVICE_DEVICE_SUPPORT_MAX_NUM_SPEC];
#elif (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_VAP)

mac_blacklist_info_stru g_ast_blacklist[WLAN_SERVICE_DEVICE_SUPPORT_MAX_NUM_SPEC * WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];

#endif
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
mac_cfg_pk_mode_stru g_st_wifi_pk_mode_status = { 0 }; /* PK mode模式 */
#endif
/* 3 函数实现 */
#ifdef _PRE_WLAN_FEATURE_DFS

oal_void mac_dfs_init(mac_device_stru *pst_mac_device)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* 软件雷达检测默认关闭 */
    mac_dfs_set_dfs_enable(pst_mac_device, OAL_TRUE);
    /* debug默认关闭 */
    mac_dfs_set_debug_level(pst_mac_device, 0);
#endif
}
#endif /* #ifdef _PRE_WLAN_FEATURE_DFS */
oal_uint32 mac_board_init_etc(void)
{
    memset_s(g_pst_mac_board, OAL_SIZEOF(mac_board_stru), 0, OAL_SIZEOF(mac_board_stru));
    return OAL_SUCC;
}


oal_void mac_random_mac_oui_init(mac_device_stru *pst_mac_device)
{
    /* 初始化随机mac oui为0(3个字节都是0),
     * 确保只有系统下发有效mac oui才进行随机mac地址扫描(在随机mac扫描开关打开的情况下)
     */
    pst_mac_device->en_is_random_mac_addr_scan = OAL_FALSE;
    pst_mac_device->auc_mac_oui[0] = 0x0;
    pst_mac_device->auc_mac_oui[1] = 0x0;
    pst_mac_device->auc_mac_oui[2] = 0x0;
}

#ifdef _PRE_WLAN_FEATURE_HIEX

oal_void mac_hiex_cap_init(mac_device_stru *pst_mac_device)
{
    if (memcpy_s(&pst_mac_device->st_hiex_cap, OAL_SIZEOF(mac_hiex_cap_stru), &g_st_default_hiex_cap,
        OAL_SIZEOF(mac_hiex_cap_stru)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "mac_hiex_cap_init::hiex cap memcpy fail!");
    }
}
#endif

OAL_STATIC oal_void mac_device_p2p_info_init(mac_p2p_info_stru *p_p2p_info)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    p_p2p_info->uc_p2p_device_num = 0;
    p_p2p_info->uc_p2p_goclient_num = 0;
    p_p2p_info->pst_primary_net_device = OAL_PTR_NULL; /* 初始化主net_device 为空指针 */
#endif
}


oal_uint32 mac_device_init_etc(mac_device_stru *pst_mac_device,
                               oal_uint32 ul_chip_ver,
                               oal_uint8 uc_chip_id,
                               oal_uint8 uc_device_id)
{
    oal_uint8 uc_device_id_per_chip;

    if (pst_mac_device == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(pst_mac_device, OAL_SIZEOF(mac_device_stru), 0, OAL_SIZEOF(mac_device_stru));

    /* 初始化device的索引 */
    pst_mac_device->uc_chip_id = uc_chip_id;
    pst_mac_device->uc_device_id = uc_device_id;

    /* 初始化device级别的一些参数 */
    pst_mac_device->en_max_bandwidth = WLAN_BAND_WIDTH_BUTT;
    pst_mac_device->en_max_band = WLAN_BAND_BUTT;
    pst_mac_device->uc_max_channel = 0;
    pst_mac_device->ul_beacon_interval = WLAN_BEACON_INTVAL_DEFAULT;
    pst_mac_device->en_delayed_shift = OAL_FALSE;
    pst_mac_device->en_delayed_shift_down_prot = OAL_FALSE;

    pst_mac_device->st_bss_id_list.us_num_networks = 0;

    /* 03两个业务device,00 01,取不同定制化,51双芯片00 11,取同一个定制化 */
    uc_device_id_per_chip = uc_device_id - uc_chip_id;
    if (uc_device_id_per_chip >= WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP) {
        OAM_ERROR_LOG3(0, OAM_SF_ANY, "{mac_device_init_etc::pst_mac_device device id[%d] chip id[%d] >support[%d].}",
            uc_device_id, uc_chip_id, WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP);
        uc_device_id_per_chip = 0;
    }

    /* 将定制化的信息保存到mac device结构体下 */
    /* 初始化mac device的能力 */
    if (EOK != memcpy_s(&pst_mac_device->st_device_cap, OAL_SIZEOF(mac_device_capability_stru),
                        &g_pst_mac_device_capability[uc_device_id_per_chip], OAL_SIZEOF(mac_device_capability_stru))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "mac_device_init_etc::memcpy fail!");
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_SMPS
    /* 对于03来说，另一个device只支持SISO,配置为MIMO SAVE状态是否没有影响，再确认; 这些能力在m2s切换中会动态变化 */
    MAC_DEVICE_GET_MODE_SMPS(pst_mac_device) = WLAN_MIB_MIMO_POWER_SAVE_MIMO;
#endif

    pst_mac_device->en_device_state = OAL_TRUE;

#ifdef _PRE_WALN_FEATURE_LUT_RESET
    pst_mac_device->en_reset_switch = OAL_TRUE;
#else
    pst_mac_device->en_reset_switch = OAL_FALSE;
#endif

    /* 默认关闭wmm,wmm超时计数器设为0 */
    pst_mac_device->en_wmm = OAL_TRUE;

    /* 根据芯片版本初始化device能力信息 */
    switch (ul_chip_ver) {
        case WLAN_CHIP_VERSION_HI1151V100H:
#ifdef _PRE_WLAN_FEATURE_11AX
            pst_mac_device->en_protocol_cap = WLAN_PROTOCOL_CAP_HE;
            pst_mac_device->en_band_cap =
                mac_device_check_5g_enable(uc_device_id) ? WLAN_BAND_CAP_2G_5G : WLAN_BAND_CAP_2G;
#else
            pst_mac_device->en_protocol_cap = WLAN_PROTOCOL_CAP_VHT;
            pst_mac_device->en_band_cap =
                mac_device_check_5g_enable(uc_device_id) ? WLAN_BAND_CAP_2G_5G : WLAN_BAND_CAP_2G;
#endif
            break;
        case WLAN_CHIP_VERSION_HI1151V100L:
#ifdef _PRE_WLAN_FEATURE_11AX
            pst_mac_device->en_protocol_cap = WLAN_PROTOCOL_CAP_HE;
            pst_mac_device->en_band_cap =
                mac_device_check_5g_enable(uc_device_id) ? WLAN_BAND_CAP_2G_5G : WLAN_BAND_CAP_2G;
#else
            pst_mac_device->en_protocol_cap = WLAN_PROTOCOL_CAP_VHT;
            pst_mac_device->en_band_cap = WLAN_BAND_CAP_2G;
#endif
            break;

        default:
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "{mac_device_init_etc::ul_chip_ver is not supportted[0x%x].}", ul_chip_ver);
            return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    /* 初始化vap num统计信息 */
    pst_mac_device->uc_vap_num = 0;
    pst_mac_device->uc_sta_num = 0;
#ifdef _PRE_WLAN_FEATURE_P2P
    mac_device_p2p_info_init(&pst_mac_device->st_p2p_info);
#endif

    /* 初始化默认管制域 */
    mac_init_regdomain_etc();

    /* 初始化信道列表 */
    mac_init_channel_list_etc();

    /* 初始化复位状态 */
    MAC_DEV_RESET_IN_PROGRESS(pst_mac_device, OAL_FALSE);
    pst_mac_device->us_device_reset_num = 0;

    /* 默认关闭DBAC特性 */
#ifdef _PRE_WLAN_FEATURE_DBAC
    pst_mac_device->en_dbac_enabled = OAL_TRUE;
#endif

    pst_mac_device->en_dbdc_running = OAL_FALSE;

#ifdef _PRE_SUPPORT_ACS
    memset_s(&pst_mac_device->st_acs_switch, OAL_SIZEOF(pst_mac_device->st_acs_switch),
             0, OAL_SIZEOF(pst_mac_device->st_acs_switch));
#endif

    mac_set_2040bss_switch(pst_mac_device, OAL_FALSE);

#ifdef _PRE_PRODUCT_ID_HI110X_DEV
    pst_mac_device->uc_in_suspend = OAL_FALSE;
#endif

    pst_mac_device->uc_arpoffload_switch = OAL_FALSE;

    pst_mac_device->uc_wapi = OAL_FALSE;

    /* AGC绑定通道默认为自适应   */
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    pst_mac_device->uc_scan_count = 0;
#endif

    mac_random_mac_oui_init(pst_mac_device);

#ifdef _PRE_WLAN_FEATURE_DFS
    mac_dfs_init(pst_mac_device);
#endif /* #ifdef _PRE_WLAN_FEATURE_DFS */

#ifdef _PRE_WLAN_FEATURE_HIEX
    mac_hiex_cap_init(pst_mac_device);
#endif

    return OAL_SUCC;
}


oal_uint32 mac_device_exit_etc(mac_device_stru *pst_device)
{
    if (OAL_UNLIKELY(pst_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_device_exit::pst_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_device->uc_vap_num = 0;
    pst_device->uc_sta_num = 0;
    pst_device->st_p2p_info.uc_p2p_device_num = 0;
    pst_device->st_p2p_info.uc_p2p_goclient_num = 0;

#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_DEVICE)
    memset_s(&(g_ast_blacklist[pst_device->uc_device_id]), OAL_SIZEOF(mac_blacklist_info_stru),
             0, OAL_SIZEOF(mac_blacklist_info_stru));
#endif

    mac_res_free_dev_etc(pst_device->uc_device_id);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_device_set_state_etc(pst_device, OAL_FALSE);
#endif

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11AX

oal_uint8 mac_device_trans_bandwith_to_he_capinfo(wlan_bw_cap_enum_uint8 en_max_op_bd)
{
    oal_uint8 uc_he_band_width_set = 0;
    switch (en_max_op_bd) {
        case WLAN_BW_CAP_20M:
            return uc_he_band_width_set;
        case WLAN_BW_CAP_40M:
            uc_he_band_width_set |= BIT0; /* Bit0:指示2.4G支持40MHz */
            return uc_he_band_width_set;
        case WLAN_BW_CAP_80M:
            uc_he_band_width_set |= BIT0 | BIT1; /* Bit1:指示5G支持40MHz\80MHz */
            return uc_he_band_width_set;
        case WLAN_BW_CAP_160M:
            uc_he_band_width_set |= BIT0 | BIT1 | BIT2; /* Bit2:指示5G支持160MHz */
            return uc_he_band_width_set;
        case WLAN_BW_CAP_80PLUS80:
            uc_he_band_width_set |= BIT0 | BIT1 | BIT2 | BIT3; /* Bit3:指示5G支持80+80MHz */
            return uc_he_band_width_set;
        default:
            OAM_ERROR_LOG1(0, OAM_SF_ANY,
                           "{mac_device_trans_bandwith_to_he_capinfo::bandwith[%d] is invalid.}", en_max_op_bd);
            return WLAN_MIB_VHT_SUPP_WIDTH_BUTT;
    }
}
#endif


oal_uint32 mac_chip_exit_etc(mac_board_stru *pst_board, mac_chip_stru *pst_chip)
{
    if (OAL_UNLIKELY(pst_chip == OAL_PTR_NULL || pst_board == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_chip_init::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_CHIP)
    memset_s(&(g_ast_blacklist[pst_chip->uc_chip_id]), OAL_SIZEOF(mac_blacklist_info_stru),
             0, OAL_SIZEOF(mac_blacklist_info_stru));
#endif

#ifdef _PRE_WLAN_FEATURE_RX_AGGR_EXTEND
    if (pst_chip->pst_rx_aggr_extend != OAL_PTR_NULL) {
        oal_free(pst_chip->pst_rx_aggr_extend);
        pst_chip->pst_rx_aggr_extend = OAL_PTR_NULL;
    }
#endif
    pst_chip->uc_device_nums = 0;

    /* destroy流程最后将状态置为FALSE */
    pst_chip->en_chip_state = OAL_FALSE;

    return OAL_SUCC;
}


oal_uint32 mac_board_exit_etc(mac_board_stru *pst_board)
{
    if (OAL_UNLIKELY(pst_board == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_board_exit_etc::pst_board null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_BOARD)
    memset_s(&g_ast_blacklist, OAL_SIZEOF(mac_blacklist_info_stru), 0, OAL_SIZEOF(mac_blacklist_info_stru));
#endif

    return OAL_SUCC;
}
oal_void mac_device_set_dfr_reset_etc(mac_device_stru *pst_mac_device, oal_uint8 uc_device_reset_in_progress)
{
    pst_mac_device->uc_device_reset_in_progress = uc_device_reset_in_progress;
}

oal_void mac_device_set_state_etc(mac_device_stru *pst_mac_device, oal_uint8 en_device_state)
{
    pst_mac_device->en_device_state = en_device_state;
}

#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL != _PRE_WLAN_FEATURE_BLACKLIST_NONE)

oal_void mac_blacklist_get_pointer(wlan_vap_mode_enum_uint8 en_vap_mode,
                                   oal_uint8 uc_chip_id, oal_uint8 uc_dev_id,
                                   oal_uint8 uc_vap_id, mac_blacklist_info_stru **pst_blacklist_info)
{
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_VAP)
    oal_uint8 uc_device_index;
    oal_uint8 uc_vap_index;
    oal_uint16 us_array_index = 0;
#endif

#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_BOARD)
    *pst_blacklist_info = &g_ast_blacklist;
#elif (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_CHIP)
    *pst_blacklist_info = &g_ast_blacklist[uc_chip_id];
#elif (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_DEVICE)
    *pst_blacklist_info = &g_ast_blacklist[uc_dev_id];
#elif (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_VAP)
    uc_device_index = uc_dev_id;

    uc_vap_index = uc_vap_id;

    if (en_vap_mode == WLAN_VAP_MODE_BSS_AP || en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        us_array_index = uc_device_index * WLAN_VAP_SUPPORT_MAX_NUM_LIMIT + uc_vap_index;
        if (us_array_index >= WLAN_SERVICE_DEVICE_SUPPORT_MAX_NUM_SPEC * WLAN_VAP_SUPPORT_MAX_NUM_LIMIT) {
            OAM_ERROR_LOG2(uc_vap_index, OAM_SF_ANY,
                           "{mac_blacklist_get_pointer::en_vap_mode=%d, vap index=%d is wrong.}",
                           en_vap_mode, us_array_index);
            return;
        }
    } else {
        OAM_ERROR_LOG1(uc_vap_index, OAM_SF_ANY, "{mac_blacklist_get_pointer::en_vap_mode=%d is wrong.}", en_vap_mode);
        return;
    }

    *pst_blacklist_info = &g_ast_blacklist[us_array_index];
    g_ast_blacklist[us_array_index].uc_blacklist_vap_index = uc_vap_index;
    g_ast_blacklist[us_array_index].uc_blacklist_device_index = uc_device_index;

#endif
}
#endif

/*lint -e19*/
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL != _PRE_WLAN_FEATURE_BLACKLIST_NONE)
oal_module_symbol(mac_blacklist_get_pointer);
#endif
#ifdef _PRE_WLAN_FEATURE_DFS
oal_module_symbol(mac_dfs_init);
#endif /* #ifdef _PRE_WLAN_FEATURE_DFS */
oal_module_symbol(mac_device_exit_etc);
oal_module_symbol(mac_chip_exit_etc);
oal_module_symbol(mac_board_exit_etc);
oal_module_symbol(mac_device_set_dfr_reset_etc);
oal_module_symbol(mac_device_set_state_etc);
oal_module_symbol(mac_board_init_etc);
oal_module_symbol(mac_device_init_etc); /*lint +e19*/

