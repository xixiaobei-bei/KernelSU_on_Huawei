

#include "wlan_types.h"

#include "oal_net.h"
#include "oal_cfg80211.h"
#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "plat_pm_wlan.h"

#include "wlan_chip_i.h"
#include "mac_device.h"
#include "mac_vap.h"
#include "mac_ie.h"
#include "mac_resource.h"
#include "hmac_device.h"
#include "hmac_resource.h"
#include "hmac_ext_if.h"
#include "hmac_vap.h"
#include "hmac_p2p.h"
#include "hmac_dfs.h"
#include "hmac_roam_main.h"

#include "wal_linux_cfg80211.h"
#include "wal_linux_event.h"
#include "wal_ext_if.h"
#include "wal_regdb.h"
#include "wal_linux_cfgvendor.h"
#ifdef _PRE_CONFIG_HISI_S3S4_POWER_STATE
#include "wal_dfx.h"
#endif

#include "securec.h"
#include "securectype.h"
#include "hmac_tx_data.h"
#include "wal_linux_netdev_ops.h"
#include "hmac_11r.h"
#include "hmac_11i.h"
#include "wal_cfg_ioctl.h"
#include "hmac_dfx.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID              OAM_FILE_ID_WAL_LINUX_CFG80211_C
#define WAL_BCN_BSSID_LENGTH_ADDR  37

#define HI1151_A_RATES      (g_hi1151_rates + 4)
#define HI1151_A_RATES_SIZE 8
#define HI1151_G_RATES      (g_hi1151_rates + 0)
#define HI1151_G_RATES_SIZE 12

/* 设备支持的速率 */
OAL_STATIC oal_ieee80211_rate g_hi1151_rates[] = {
    RATETAB_ENT(10, 0x1, 0),
    RATETAB_ENT(20, 0x2, 0),
    RATETAB_ENT(55, 0x4, 0),
    RATETAB_ENT(110, 0x8, 0),
    RATETAB_ENT(60, 0x10, 0),
    RATETAB_ENT(90, 0x20, 0),
    RATETAB_ENT(120, 0x40, 0),
    RATETAB_ENT(180, 0x80, 0),
    RATETAB_ENT(240, 0x100, 0),
    RATETAB_ENT(360, 0x200, 0),
    RATETAB_ENT(480, 0x400, 0),
    RATETAB_ENT(540, 0x800, 0),
};

/* 2.4G 频段 */
OAL_STATIC oal_ieee80211_channel g_ast_supported_channels_2ghz_info[] = {
    CHAN2G(1, 2412, 0),
    CHAN2G(2, 2417, 0),
    CHAN2G(3, 2422, 0),
    CHAN2G(4, 2427, 0),
    CHAN2G(5, 2432, 0),
    CHAN2G(6, 2437, 0),
    CHAN2G(7, 2442, 0),
    CHAN2G(8, 2447, 0),
    CHAN2G(9, 2452, 0),
    CHAN2G(10, 2457, 0),
    CHAN2G(11, 2462, 0),
    CHAN2G(12, 2467, 0),
    CHAN2G(13, 2472, 0),
    CHAN2G(14, 2484, 0),
};

/* 5G 频段 */
OAL_STATIC oal_ieee80211_channel g_ast_supported_channels_5ghz_info[] = {
    CHAN5G(36, 0),
    CHAN5G(40, 0),
    CHAN5G(44, 0),
    CHAN5G(48, 0),
    CHAN5G(52, 0),
    CHAN5G(56, 0),
    CHAN5G(60, 0),
    CHAN5G(64, 0),
    CHAN5G(100, 0),
    CHAN5G(104, 0),
    CHAN5G(108, 0),
    CHAN5G(112, 0),
    CHAN5G(116, 0),
    CHAN5G(120, 0),
    CHAN5G(124, 0),
    CHAN5G(128, 0),
    CHAN5G(132, 0),
    CHAN5G(136, 0),
    CHAN5G(140, 0),
    CHAN5G(144, 0),
    CHAN5G(149, 0),
    CHAN5G(153, 0),
    CHAN5G(157, 0),
    CHAN5G(161, 0),
    CHAN5G(165, 0),
    /* 4.9G */
    CHAN4_9G(184, 0),
    CHAN4_9G(188, 0),
    CHAN4_9G(192, 0),
    CHAN4_9G(196, 0),
};

/* 设备支持的加密套件 */
OAL_STATIC const uint32_t g_ast_wlan_supported_cipher_suites[] = {
    WLAN_CIPHER_SUITE_WEP40,
    WLAN_CIPHER_SUITE_WEP104,
    WLAN_CIPHER_SUITE_TKIP,
    WLAN_CIPHER_SUITE_CCMP,
#ifdef _PRE_WLAN_FEATURE_GCMP_256_CCMP256
    WLAN_CIPHER_SUITE_GCMP,
    WLAN_CIPHER_SUITE_GCMP_256,
    WLAN_CIPHER_SUITE_CCMP_256,
#endif
    WLAN_CIPHER_SUITE_AES_CMAC,
    WLAN_CIPHER_SUITE_BIP_CMAC_256,
    WLAN_CIPHER_SUITE_SMS4,
    WLAN_CIPHER_SUITE_BIP_GMAC_256,
};

extern const wal_ioctl_mode_map_stru g_ast_mode_map[];

/* 2.4G 频段信息 */
oal_ieee80211_supported_band g_st_supported_band_2ghz_info = {
    .channels = g_ast_supported_channels_2ghz_info,
    .n_channels = sizeof(g_ast_supported_channels_2ghz_info) / sizeof(oal_ieee80211_channel),
    .bitrates = HI1151_G_RATES,
    .n_bitrates = HI1151_G_RATES_SIZE,
    .ht_cap = {
        .ht_supported = OAL_TRUE,
        .cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40,
    },
};

/* 5G 频段信息 */
OAL_STATIC oal_ieee80211_supported_band g_st_supported_band_5ghz_info = {
    .channels = g_ast_supported_channels_5ghz_info,
    .n_channels = sizeof(g_ast_supported_channels_5ghz_info) / sizeof(oal_ieee80211_channel),
    .bitrates = HI1151_A_RATES,
    .n_bitrates = HI1151_A_RATES_SIZE,
    .ht_cap = {
        .ht_supported = OAL_TRUE,
        .cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40,
    },
    .vht_cap = {
        .vht_supported = OAL_TRUE,
        .cap = IEEE80211_VHT_CAP_SHORT_GI_80 | IEEE80211_VHT_CAP_HTC_VHT |
               IEEE80211_VHT_CAP_SHORT_GI_160 | IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ,
    },
};
#define IEEE80211_IFACE_TYPE_STA_MAX_LIMIT 2
#define IEEE80211_IFACE_TYPE_P2P_GO_CLI_MAX_LIMIT 2
#define IEEE80211_IFACE_TYPE_P2P_DEV_MAX_LIMIT 1
OAL_STATIC oal_ieee80211_iface_limit g_sta_p2p_limits[] = {
    {
        .max = IEEE80211_IFACE_TYPE_STA_MAX_LIMIT,
        .types = BIT(NL80211_IFTYPE_STATION),
    },
    {
        .max = IEEE80211_IFACE_TYPE_P2P_GO_CLI_MAX_LIMIT,
        .types = BIT(NL80211_IFTYPE_P2P_GO) | BIT(NL80211_IFTYPE_P2P_CLIENT),
    },
    {
        .max = IEEE80211_IFACE_TYPE_P2P_DEV_MAX_LIMIT,
        .types = BIT(NL80211_IFTYPE_P2P_DEVICE),
    },
};
#define IEEE80211_IFACE_COMBINATION_STA_P2P_DIFF_CHAN_NUMS 2
#define IEEE80211_IFACE_COMBINATION_STA_P2P_MAX_INTERFACES 3
OAL_STATIC oal_ieee80211_iface_combination g_sta_p2p_iface_combinations[] = {
    {
        .num_different_channels = IEEE80211_IFACE_COMBINATION_STA_P2P_DIFF_CHAN_NUMS,
        .max_interfaces = IEEE80211_IFACE_COMBINATION_STA_P2P_MAX_INTERFACES,
        .limits = g_sta_p2p_limits,
        .n_limits = oal_array_size(g_sta_p2p_limits),
    },
};

/* There isn't a lot of sense in it, but you can transmit anything you like */
static const struct ieee80211_txrx_stypes
    g_wal_cfg80211_default_mgmt_stypes[NUM_NL80211_IFTYPES] = {
        [NL80211_IFTYPE_ADHOC] = {
            .tx = 0xffff,
            .rx = BIT(IEEE80211_STYPE_ACTION >> BIT_OFFSET_4)
        },
        [NL80211_IFTYPE_STATION] = {
            .tx = 0xffff, .rx = BIT(IEEE80211_STYPE_ACTION >> BIT_OFFSET_4)
            | BIT(IEEE80211_STYPE_PROBE_REQ >> BIT_OFFSET_4)
#ifdef _PRE_WLAN_FEATURE_SAE
            | BIT(IEEE80211_STYPE_AUTH >> BIT_OFFSET_4)
#endif
        },
        [NL80211_IFTYPE_AP] = {
            .tx = 0xffff,
            .rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> BIT_OFFSET_4) |  //lint !e572
            BIT(IEEE80211_STYPE_REASSOC_REQ >> BIT_OFFSET_4) | BIT(IEEE80211_STYPE_PROBE_REQ >> BIT_OFFSET_4) |
            BIT(IEEE80211_STYPE_DISASSOC >> BIT_OFFSET_4) | BIT(IEEE80211_STYPE_AUTH >> BIT_OFFSET_4) |
            BIT(IEEE80211_STYPE_DEAUTH >> BIT_OFFSET_4) | BIT(IEEE80211_STYPE_ACTION >> BIT_OFFSET_4)
        },
        [NL80211_IFTYPE_AP_VLAN] = { /* copy AP */
            .tx = 0xffff,
            .rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> BIT_OFFSET_4) |  //lint !e572
            BIT(IEEE80211_STYPE_REASSOC_REQ >> BIT_OFFSET_4) | BIT(IEEE80211_STYPE_PROBE_REQ >> BIT_OFFSET_4) |
            BIT(IEEE80211_STYPE_DISASSOC >> BIT_OFFSET_4) | BIT(IEEE80211_STYPE_AUTH >> BIT_OFFSET_4) |
            BIT(IEEE80211_STYPE_DEAUTH >> BIT_OFFSET_4) | BIT(IEEE80211_STYPE_ACTION >> BIT_OFFSET_4)
        },
        [NL80211_IFTYPE_P2P_CLIENT] = {
            .tx = 0xffff,
            .rx = BIT(IEEE80211_STYPE_ACTION >> BIT_OFFSET_4) | BIT(IEEE80211_STYPE_PROBE_REQ >> BIT_OFFSET_4)
        },
        [NL80211_IFTYPE_P2P_GO] = {
            .tx = 0xffff,
            .rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> BIT_OFFSET_4) |  //lint !e572
            BIT(IEEE80211_STYPE_REASSOC_REQ >> BIT_OFFSET_4) | BIT(IEEE80211_STYPE_PROBE_REQ >> BIT_OFFSET_4) |
            BIT(IEEE80211_STYPE_DISASSOC >> BIT_OFFSET_4) | BIT(IEEE80211_STYPE_AUTH >> BIT_OFFSET_4) |
            BIT(IEEE80211_STYPE_DEAUTH >> BIT_OFFSET_4) | BIT(IEEE80211_STYPE_ACTION >> BIT_OFFSET_4)
        },
        [NL80211_IFTYPE_P2P_DEVICE] = {
            .tx = 0xffff,
            .rx = BIT(IEEE80211_STYPE_ACTION >> BIT_OFFSET_4) | BIT(IEEE80211_STYPE_PROBE_REQ >> BIT_OFFSET_4)
        },
};

uint8_t g_uc_cookie_array_bitmap = 0; /* 每个bit 表示cookie array 中是否使用，1 - 已使用；0 - 未使用 */
cookie_arry_stru g_cookie_array[WAL_COOKIE_ARRAY_SIZE];


OAL_STATIC uint32_t wal_find_wmm_uapsd(uint8_t *puc_wmm_ie)
{
    /* 判断 WMM UAPSD 是否使能 */
    if (puc_wmm_ie[1] < MAC_WMM_QOS_INFO_POS) {
        return OAL_FALSE;
    }

    if (puc_wmm_ie[MAC_WMM_QOS_INFO_POS] & BIT7) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_hide_ssid(uint8_t *puc_ssid_ie, uint8_t uc_ssid_len)
{
    return (oal_bool_enum_uint8)((puc_ssid_ie == NULL) || (uc_ssid_len == 0) || (puc_ssid_ie[0] == '\0'));
}


uint32_t wal_cfg80211_open_wmm(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_open_wmm::pst_mac_vap/puc_param is null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 针对配置vap做保护 */
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_CONFIG) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{wal_cfg80211_open_wmm::this is config vap! can't get info.}");
        return OAL_FAIL;
    }

    /* host侧,开关WMM，更新mib信息位中的Qos位置 */
    return hmac_config_open_wmm(pst_mac_vap, us_len, puc_param);
}


void wal_parse_wpa_wpa2_ie(oal_beacon_parameters *pst_beacon_info,
                           mac_beacon_param_stru *pst_beacon_param)
{
    oal_ieee80211_mgmt *pst_mgmt;
    uint16_t us_capability_info;
    uint8_t *puc_ie_tmp = NULL;
    int32_t l_ret = EOK;

    /* 判断是否加密 */
    pst_mgmt = (oal_ieee80211_mgmt *)pst_beacon_info->head;

    us_capability_info = pst_mgmt->u.beacon.capab_info;
    pst_beacon_param->en_privacy = OAL_FALSE;

    if (us_capability_info & WLAN_WITP_CAPABILITY_PRIVACY) {
        pst_beacon_param->en_privacy = OAL_TRUE;

        /* 查找 RSN 信息元素 */
        puc_ie_tmp = mac_find_ie(MAC_EID_RSN, pst_beacon_info->tail, pst_beacon_info->tail_len);
        if (puc_ie_tmp != NULL) {
            l_ret += memcpy_s(pst_beacon_param->auc_rsn_ie, MAC_MAX_RSN_LEN,
                puc_ie_tmp, MAC_IE_HDR_LEN + puc_ie_tmp[1]);
        }

        /* 查找 WPA 信息元素，并解析出认证类型 */
        puc_ie_tmp = mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_OUITYPE_WPA,
            pst_beacon_info->tail, pst_beacon_info->tail_len);
        if (puc_ie_tmp != NULL) {
            l_ret += memcpy_s(pst_beacon_param->auc_wpa_ie, MAC_MAX_RSN_LEN,
                puc_ie_tmp, MAC_IE_HDR_LEN + puc_ie_tmp[1]);
        }
    }
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_parse_wpa_wpa2_ie::memcpy fail!");
    }
}


uint32_t wal_parse_wmm_ie(oal_net_device_stru *pst_dev,
                          mac_vap_stru *pst_mac_vap, oal_beacon_parameters *pst_beacon_info)
{
    uint8_t *puc_wmm_ie;
    uint16_t us_len = sizeof(uint8_t);
    uint8_t uc_wmm = OAL_TRUE;
    uint32_t ret;
    uint32_t uapsd = OAL_TRUE;
    uint32_t cmd_id;
    uint32_t offset = 0;

    /*  查找wmm_ie  */
    puc_wmm_ie = mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WMM,
                                    pst_beacon_info->tail, pst_beacon_info->tail_len);
    if (puc_wmm_ie == NULL) {
        /* wmm ie未找到，则说明wmm 关 */
        uc_wmm = OAL_FALSE;
    /*  找到wmm ie，顺便判断下uapsd是否使能 */
    } else {
        if (OAL_FALSE == wal_find_wmm_uapsd(puc_wmm_ie)) {
            /* 对应UAPSD 关 */
            uapsd = OAL_FALSE;
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_parse_wmm_ie::uapsd is disabled!!}");
        }

        /* 该功能目前按照驱动设置来实现，后续需要结合上层设置，TBD */
        if (IS_P2P_GO(pst_mac_vap)) {
            uapsd = WLAN_FEATURE_UAPSD_IS_OPEN;
            oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                          "{wal_parse_wmm_ie:: (1103)It is a Go, set uapsd = WLAN_FEATURE_UAPSD_IS_OPEN)}");
        }
        oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                      "{wal_parse_wmm_ie:: uapsd = %d)}", uapsd);

        ret = wal_get_cmd_id("uapsd_en_cap", &cmd_id, &offset);
        if (ret != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_ANY, "wal_parse_wmm_ie:find uapsd_en_cap cmd is fail");
            return ret;
        }
        wal_process_cmd_params(pst_dev, cmd_id, &uapsd);
    }
    /*  wmm 开启/关闭 标记  */
    ret = wal_cfg80211_open_wmm(pst_mac_vap, us_len, &uc_wmm);
    if (ret != OAL_SUCC) {
        ret = OAL_FAIL;
        oam_warning_log0(0, OAM_SF_TX, "{wal_parse_wmm_ie::can not open wmm!}\r\n");
    }

    return ret;
}


OAL_STATIC uint32_t wal_cfg80211_add_vap_config(mac_cfg_add_vap_param_stru *add_vap_param,
                                                mac_vap_stru *cfg_mac_vap,
                                                oal_net_device_stru *cfg_net_dev,
                                                wlan_vap_mode_enum_uint8 vap_mode,
                                                wlan_p2p_mode_enum_uint8 p2p_mode)
{
    oal_net_device_stru *net_dev = NULL;
    wal_msg_stru *rsp_msg = NULL;
    wal_msg_write_stru write_msg;
    uint32_t err_code;
    int32_t ret;

    /* 获取mac device */
    net_dev = add_vap_param->pst_net_dev;
    oal_netdevice_flags(net_dev) &= ~OAL_IFF_RUNNING; /* 将net device的flag设为down */

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 填写消息 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_ADD_VAP, sizeof(mac_cfg_add_vap_param_stru));
    ((mac_cfg_add_vap_param_stru *)write_msg.auc_value)->pst_net_dev = net_dev;
    ((mac_cfg_add_vap_param_stru *)write_msg.auc_value)->en_vap_mode = vap_mode;
    ((mac_cfg_add_vap_param_stru *)write_msg.auc_value)->uc_cfg_vap_indx = cfg_mac_vap->uc_vap_id;
    ((mac_cfg_add_vap_param_stru *)write_msg.auc_value)->en_p2p_mode = p2p_mode;
    ((mac_cfg_add_vap_param_stru *)write_msg.auc_value)->is_primary_vap = 0;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wal_vap_get_wlan_mode_para(&write_msg);
#endif
    /* 发送消息 */
    ret = wal_send_cfg_event(cfg_net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_add_vap_param_stru),
                             (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(cfg_mac_vap->uc_vap_id, 0, "{wal_cfg80211_add_vap_config::return err code %d!}", ret);
        return (uint32_t)ret;
    }

    /* 读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(cfg_mac_vap->uc_vap_id, OAM_SF_ANY,
            "{wal_cfg80211_add_vap_config::hmac add vap fail, err_code[%u]!}\r\n", err_code);
        return err_code;
    }
    wal_set_random_mac_to_mib(net_dev); /* set random mac to mib ; for hi1102-cb */

    return OAL_SUCC;
}


uint32_t wal_cfg80211_add_vap(mac_cfg_add_vap_param_stru *add_vap_param)
{
    
    oal_net_device_stru *net_dev = NULL;
    oal_net_device_stru *cfg_net_dev = NULL;
    oal_wireless_dev_stru *wdev = NULL;
    mac_wiphy_priv_stru *wiphy_priv = NULL;
    mac_vap_stru *cfg_mac_vap = NULL;
    hmac_vap_stru *cfg_hmac_vap = NULL;
    mac_device_stru *mac_device = NULL;
    wlan_vap_mode_enum_uint8 vap_mode = add_vap_param->en_vap_mode;
    wlan_p2p_mode_enum_uint8 p2p_mode = add_vap_param->en_p2p_mode;
    uint32_t ret;

    /* 获取mac device */
    net_dev = add_vap_param->pst_net_dev;
    wdev = net_dev->ieee80211_ptr;
    wiphy_priv = (mac_wiphy_priv_stru *)oal_wiphy_priv(wdev->wiphy);
    mac_device = wiphy_priv->pst_mac_device;
    cfg_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(mac_device->uc_cfg_vap_id);
    if (oal_unlikely(cfg_mac_vap == NULL)) {
        oam_warning_log1(0, 0, "{wal_cfg80211_add_vap::cfg_mac_vap null vap_id:%d}", mac_device->uc_cfg_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_device->uc_cfg_vap_id);
    if (oal_unlikely(cfg_hmac_vap == NULL)) {
        oam_warning_log1(0, 0, "{wal_cfg80211_add_vap::cfg_hmac_vap null vapid:%d}", mac_device->uc_cfg_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    cfg_net_dev = cfg_hmac_vap->pst_net_device;
    if (oal_unlikely(cfg_net_dev == NULL)) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_add_vap::pst_mac_device is null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (vap_mode == WLAN_VAP_MODE_BSS_AP) {
        wdev->iftype = NL80211_IFTYPE_AP;
    } else if (vap_mode == WLAN_VAP_MODE_BSS_STA) {
        wdev->iftype = NL80211_IFTYPE_STATION;
    }
    if (p2p_mode == WLAN_P2P_DEV_MODE) {
        wdev->iftype = NL80211_IFTYPE_P2P_DEVICE;
    } else if (p2p_mode == WLAN_P2P_CL_MODE) {
        wdev->iftype = NL80211_IFTYPE_P2P_CLIENT;
    } else if (p2p_mode == WLAN_P2P_GO_MODE) {
        wdev->iftype = NL80211_IFTYPE_P2P_GO;
    }

    ret = wal_cfg80211_add_vap_config(add_vap_param, cfg_mac_vap, cfg_net_dev, vap_mode, p2p_mode);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return OAL_SUCC;
}


uint32_t wal_cfg80211_del_vap(mac_cfg_del_vap_param_stru *pst_del_vap_param)
{
    wal_msg_write_stru st_write_msg;
    wal_msg_stru *pst_rsp_msg = NULL;
    oal_net_device_stru *pst_net_dev = NULL;
    mac_vap_stru *pst_mac_vap = NULL;

    if (oal_unlikely(pst_del_vap_param == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_vap::pst_del_vap_param null ptr !}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_net_dev = pst_del_vap_param->pst_net_dev;

    /* 设备在up状态不允许删除，必须先down */
    if (oal_unlikely(0 != (OAL_IFF_RUNNING & oal_netdevice_flags(pst_net_dev)))) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_cfg80211_del_vap::device is busy, please down it first %d!}\r\n",
            oal_netdevice_flags(pst_net_dev));
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    pst_mac_vap = oal_net_dev_priv(pst_net_dev);
    if (oal_unlikely(pst_mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_del_vap::can't get mac vap from netdevice priv data!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设备在up状态不允许删除，必须先down */
    if ((pst_mac_vap->en_vap_state != MAC_VAP_STATE_INIT) &&
        (pst_mac_vap->en_vap_state != MAC_VAP_STATE_STA_SCAN_COMP)) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_cfg80211_del_vap::device is busy, please down it first %d!}\r\n",
            pst_mac_vap->en_vap_state);
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    /* 不需要释放net_device 结构下的wireless_dev 成员 */
    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    /* 初始化删除vap 参数 */
    ((mac_cfg_del_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_net_dev;
    ((mac_cfg_del_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = pst_del_vap_param->en_p2p_mode;
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DEL_VAP, sizeof(mac_cfg_del_vap_param_stru));

    if (OAL_SUCC != wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_del_vap_param_stru),
                                       (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_vap::wal_send_cfg_event fail!}");
        return OAL_EFAIL;
    }

    if (OAL_SUCC != wal_check_and_release_msg_resp(pst_rsp_msg)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_vap::wal_check_and_release_msg_resp fail!}");
        return OAL_EFAIL;
    }

    return OAL_SUCC;
}


uint32_t wal_cfg80211_vowifi_report(frw_event_mem_stru *pst_event_mem)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
    frw_event_stru *pst_event;
    hmac_vap_stru *pst_hmac_vap;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_vowifi_report::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_hmac_vap = mac_res_get_hmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log1(0, OAM_SF_TX, "{wal_cfg80211_vowifi_report::pst_hmac_vap null.vap_id[%d]}",
            pst_event->st_event_hdr.uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 上报vowifi切换申请 */
    oal_cfg80211_vowifi_report(pst_hmac_vap->pst_net_device, GFP_KERNEL);
#endif /* (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)) */

    return OAL_SUCC;
}


uint32_t wal_cfg80211_cac_report(frw_event_mem_stru *pst_event_mem)
{
    return OAL_SUCC;
}


OAL_STATIC oal_bool_enum wal_check_support_basic_rate_6m(uint8_t *puc_supported_rates_ie,
                                                         uint8_t uc_supported_rates_num,
                                                         uint8_t *puc_extended_supported_rates_ie,
                                                         uint8_t uc_extended_supported_rates_num)
{
    uint8_t uc_loop;
    oal_bool_enum en_support = OAL_FALSE;
    for (uc_loop = 0; uc_loop < uc_supported_rates_num; uc_loop++) {
        if (puc_supported_rates_ie == NULL) {
            break;
        }
        if (puc_supported_rates_ie[BYTE_OFFSET_2 + uc_loop] == 0x8c) {
            en_support = OAL_TRUE;
        }
    }

    for (uc_loop = 0; uc_loop < uc_extended_supported_rates_num; uc_loop++) {
        if (puc_extended_supported_rates_ie == NULL) {
            break;
        }
        if (puc_extended_supported_rates_ie[BYTE_OFFSET_2 + uc_loop] == 0x8c) {
            en_support = OAL_TRUE;
        }
    }

    return en_support;
}

OAL_STATIC OAL_INLINE uint32_t wal_parse_protocol_mode_rate_nums_check(uint8_t uc_rate_nums,
    uint8_t uc_supported_rates_num, uint8_t uc_extended_supported_rates_num, wlan_protocol_enum_uint8 *pen_protocol)
{
    if (uc_rate_nums < uc_supported_rates_num) {
        oam_error_log2(0, 0, "{wal_parse_protocol_mode::rate_num[%d], ext_rate_num[%d]}",
            uc_supported_rates_num, uc_extended_supported_rates_num);
        *pen_protocol = WLAN_PROTOCOL_BUTT;
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_parse_protocol_mode_by_vht_ht_ie(uint8_t *puc_vht_ie,
    uint8_t *puc_ht_ie, wlan_protocol_enum_uint8 *pen_protocol)
{
    if (puc_vht_ie != NULL) {
        /* 设置AP 为11ac 模式 */
        *pen_protocol = WLAN_VHT_MODE;
        return OAL_SUCC;
    }
    if (puc_ht_ie != NULL) {
        /* 设置AP 为11n 模式 */
        *pen_protocol = WLAN_HT_MODE;
        return OAL_SUCC;
    }
    return OAL_FAIL;
}


OAL_STATIC uint32_t wal_parse_protocol_mode(wlan_channel_band_enum_uint8 en_band,
    oal_beacon_parameters *pst_beacon_info, uint8_t *puc_ht_ie, uint8_t *puc_vht_ie,
    uint8_t *puc_he_ie, wlan_protocol_enum_uint8 *pen_protocol, uint8_t he_flag)
{
    uint8_t *puc_supported_rates_ie = NULL;
    uint8_t *puc_extended_supported_rates_ie = NULL;
    uint8_t uc_supported_rates_num = 0;
    uint8_t uc_extended_supported_rates_num = 0;
    uint16_t us_offset;
    uint8_t uc_rate_nums;
    if ((puc_he_ie != NULL) || (he_flag == OAL_TRUE)) {
#ifdef _PRE_WLAN_FEATURE_11AX
        if (g_wlan_spec_cfg->feature_11ax_is_open) {
            /* 设置AP 为11ax模式 */
            *pen_protocol = WLAN_HE_MODE;
            return OAL_SUCC;
        }
#endif
    }
    if (wal_parse_protocol_mode_by_vht_ht_ie(puc_vht_ie, puc_ht_ie, pen_protocol) == OAL_SUCC) {
        return OAL_SUCC;
    }

    /* hostapd 先设置频段，后设置add beacon 因此在add beacon流程中，可以使用频段信息和速率信息设置AP 模式(a/b/g) */
    if (en_band == WLAN_BAND_5G) {
        *pen_protocol = WLAN_LEGACY_11A_MODE;
        return OAL_SUCC;
    }
    if (en_band == WLAN_BAND_2G) {
        us_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
        puc_supported_rates_ie = mac_find_ie(MAC_EID_RATES,
            pst_beacon_info->head + BYTE_OFFSET_24 + us_offset, pst_beacon_info->head_len - us_offset);
        if (puc_supported_rates_ie != NULL) {
            uc_supported_rates_num = puc_supported_rates_ie[1];
        }
        puc_extended_supported_rates_ie =
            mac_find_ie(MAC_EID_XRATES, pst_beacon_info->tail, pst_beacon_info->tail_len);
        if (puc_extended_supported_rates_ie != NULL) {
            uc_extended_supported_rates_num = puc_extended_supported_rates_ie[1];
        }

        uc_rate_nums = uc_supported_rates_num + uc_extended_supported_rates_num;
        // 此处若直接使用uc_supported_rates_num + uc_extended_supported_rates_num起不到长度保护作用，仍会溢出
        if (OAL_FAIL == wal_parse_protocol_mode_rate_nums_check(uc_rate_nums, uc_supported_rates_num,
            uc_extended_supported_rates_num, pen_protocol)) {
            return OAL_FAIL;
        }

        if (uc_rate_nums == 4) { /* 4表示11b的速率 */
            *pen_protocol = WLAN_LEGACY_11B_MODE;
            return OAL_SUCC;
        } else if (uc_rate_nums == 8) { /* 8表示11g的速率 */
            *pen_protocol = WLAN_LEGACY_11G_MODE;
            return OAL_SUCC;
        } else if (uc_rate_nums == 12) { /* 12表示11g混合速率 */
            /* 根据基本速率区分为 11gmix1 还是 11gmix2 */
            /* 如果基本速率集支持 6M , 则判断为 11gmix2 */
            *pen_protocol = WLAN_MIXED_ONE_11G_MODE;
            if (OAL_TRUE == wal_check_support_basic_rate_6m(puc_supported_rates_ie, uc_supported_rates_num,
                puc_extended_supported_rates_ie, uc_extended_supported_rates_num)) {
                *pen_protocol = WLAN_MIXED_TWO_11G_MODE;
            }
            return OAL_SUCC;
        }
    }

    /* 其他情况，认为配置不合理 */
    *pen_protocol = WLAN_PROTOCOL_BUTT;

    return OAL_FAIL;
}


OAL_STATIC uint8_t* wal_parse_he_ie(mac_vap_stru *pst_mac_vap, uint8_t *puc_flag,
    oal_beacon_parameters  *pst_beacon_info)
{
    uint8_t *puc_he_ie = NULL;

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        /* 定制化开关控制05 device能力起在ax模式，03不起在ax模式 */
        if (g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_11ax_aput_switch == OAL_TRUE) {
            *puc_flag = OAL_TRUE;
        }
        /* 通过hostapd下发ax能力 */
        puc_he_ie = mac_find_ie_ext_ie(MAC_EID_HE, MAC_EID_EXT_HE_CAP,
                                       pst_beacon_info->tail, pst_beacon_info->tail_len);
    }
#endif
    return puc_he_ie;
}


OAL_STATIC void wal_he_flag_handle(mac_vap_stru *pst_mac_vap, uint8_t **ppuc_he_ie, uint8_t *ul_he_flag)
{
    mac_device_stru *p_mac_dev = NULL;
    if (g_wlan_spec_cfg->feature_slave_ax_is_support == OAL_TRUE) {
        return;
    }

    if (*ppuc_he_ie == NULL && *ul_he_flag == OAL_FALSE) {
        return;
    }

    p_mac_dev = (mac_device_stru *)mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (p_mac_dev == NULL) {
        return;
    }

    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_he_flag_handle::p2p_scenes = %d!}\r\n",
        p_mac_dev->st_p2p_info.p2p_scenes);
    if (p_mac_dev->st_p2p_info.p2p_scenes == MAC_P2P_SCENES_LOW_LATENCY &&
        OAL_TRUE == mac_vap_can_not_start_he_protocol(pst_mac_vap)) {
        *ppuc_he_ie = NULL;
        *ul_he_flag = OAL_FALSE;
    }
}

OAL_STATIC void wal_go_protocol_handle(mac_vap_stru *pst_mac_vap, mac_beacon_param_stru *pst_beacon_param)
{
    if (!IS_P2P_GO(pst_mac_vap)) {
        return;
    }
    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP || pst_mac_vap->en_vap_state == MAC_VAP_STATE_PAUSE) {
        pst_beacon_param->en_protocol = pst_mac_vap->en_protocol;
        return;
    }
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        if (pst_beacon_param->en_protocol == WLAN_HE_MODE) {
            return;
        }
    }
#endif

    if (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G) {
        pst_beacon_param->en_protocol =
            ((pst_mac_vap->st_cap_flag.bit_11ac2g == OAL_TRUE) ? WLAN_VHT_MODE : WLAN_HT_MODE);
    }
    if (OAL_TRUE == mac_vap_avoid_dbac_close_vht_protocol(pst_mac_vap)) {
        pst_beacon_param->en_protocol = WLAN_HT_MODE;
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_go_protocol_handle::close vht = %d!}\r\n",
            pst_beacon_param->en_protocol);
    }
}


OAL_STATIC uint32_t wal_parse_ht_vht_ie(mac_vap_stru *pst_mac_vap,
                                        oal_beacon_parameters *pst_beacon_info,
                                        mac_beacon_param_stru *pst_beacon_param)
{
    uint8_t *puc_ht_ie, *puc_vht_ie;
    uint8_t *puc_he_ie = NULL;
    uint32_t ret;
    mac_frame_ht_cap_stru *pst_ht_cap = NULL;
    mac_vht_cap_info_stru *pst_vht_cap = NULL;
    uint8_t he_flag = OAL_FALSE;

    puc_ht_ie = mac_find_ie(MAC_EID_HT_CAP, pst_beacon_info->tail, pst_beacon_info->tail_len);
    puc_vht_ie = mac_find_ie(MAC_EID_VHT_CAP, pst_beacon_info->tail, pst_beacon_info->tail_len);
    puc_he_ie = wal_parse_he_ie(pst_mac_vap, &he_flag, pst_beacon_info);
    wal_he_flag_handle(pst_mac_vap, &puc_he_ie, &he_flag);

    /* 解析协议模式 */
    ret = wal_parse_protocol_mode(pst_mac_vap->st_channel.en_band, pst_beacon_info,
        puc_ht_ie, puc_vht_ie, puc_he_ie, &pst_beacon_param->en_protocol, he_flag);
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_parse_ht_vht_ie::return err code!}\r\n", ret);
        return ret;
    }

    wal_go_protocol_handle(pst_mac_vap, pst_beacon_param);

    /* 如果ht vht均为空，返回 */
    if ((puc_ht_ie == NULL) && (puc_vht_ie == NULL)) {
        return OAL_SUCC;
    }
    /* 解析short gi能力 */
    if (puc_ht_ie != NULL) {
        /* 使用ht cap ie中数据域的2个字节 */
        if (puc_ht_ie[1] < sizeof(mac_frame_ht_cap_stru)) {
            oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                "{wal_parse_ht_vht_ie::invalid ht cap ie len[%d]!}\r\n", puc_ht_ie[1]);
            return OAL_SUCC;
        }

        pst_ht_cap = (mac_frame_ht_cap_stru *)(puc_ht_ie + MAC_IE_HDR_LEN);

        pst_beacon_param->en_shortgi_20 = (uint8_t)pst_ht_cap->bit_short_gi_20mhz;
        pst_beacon_param->en_shortgi_40 = 0;

        if ((pst_mac_vap->st_channel.en_bandwidth > WLAN_BAND_WIDTH_20M) &&
            (pst_mac_vap->st_channel.en_bandwidth != WLAN_BAND_WIDTH_BUTT)) {
            pst_beacon_param->en_shortgi_40 = (uint8_t)pst_ht_cap->bit_short_gi_40mhz;
        }
    }

    if (puc_vht_ie == NULL) {
        return OAL_SUCC;
    }

    /* 使用vht cap ie中数据域的4个字节 */
    if (puc_vht_ie[1] < sizeof(mac_vht_cap_info_stru)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, 0, "{wal_parse_ht_vht_ie::invalid ht cap ie len[%d]!}", puc_vht_ie[1]);
        return OAL_SUCC;
    }

    pst_vht_cap = (mac_vht_cap_info_stru *)(puc_vht_ie + MAC_IE_HDR_LEN);

    pst_beacon_param->en_shortgi_80 = 0;

    if ((pst_mac_vap->st_channel.en_bandwidth > WLAN_BAND_WIDTH_40MINUS) &&
        (pst_mac_vap->st_channel.en_bandwidth != WLAN_BAND_WIDTH_BUTT)) {
        pst_beacon_param->en_shortgi_80 = pst_vht_cap->bit_short_gi_80mhz;
    }

    return OAL_SUCC;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
OAL_STATIC int32_t wal_cfg80211_sched_scan_stop(oal_wiphy_stru *pst_wiphy,
                                                oal_net_device_stru *pst_netdev,
                                                uint64_t reqid);

#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
OAL_STATIC int32_t wal_cfg80211_sched_scan_stop(oal_wiphy_stru *pst_wiphy,
                                                oal_net_device_stru *pst_netdev);
#else
/* do nothing */
#endif


OAL_STATIC int32_t wal_p2p_stop_roc(mac_vap_stru *pst_mac_vap, oal_net_device_stru *pst_netdev)
{
    hmac_vap_stru *pst_hmac_vap = NULL;
    hmac_device_stru *pst_hmac_device;
    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
            "{wal_p2p_stop_roc:: pst_hmac_device[%d] null!}\r\n", pst_mac_vap->uc_device_id);
        return -OAL_EFAIL;
    }

    /* tx mgmt roc 优先级低,可以被自己的80211 roc以及80211 scan打断 */
    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN) {
        if (pst_hmac_device->st_scan_mgmt.en_is_scanning != OAL_TRUE) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
                "{wal_p2p_stop_roc::not in scan state but vap is listen state!}");
            return OAL_SUCC;
        }

        pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
        if (oal_unlikely(pst_hmac_vap == NULL)) {
            oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{wal_p2p_stop_roc:: pst_hmac_vap null!}\r\n");
            return -OAL_EFAIL;
        }
        pst_hmac_vap->en_wait_roc_end = OAL_TRUE;
        oal_init_completion(&(pst_hmac_vap->st_roc_end_ready));
        wal_force_scan_complete(pst_netdev, OAL_TRUE);
        if (0 == oal_wait_for_completion_timeout(&(pst_hmac_vap->st_roc_end_ready),
            (uint32_t)oal_msecs_to_jiffies(200))) {       /* 200ms */
            oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{wal_p2p_stop_roc::cancel old roc timout!}");
            return -OAL_EFAIL;
        }
    }
    return OAL_SUCC;
}


OAL_STATIC mac_vap_stru *wal_find_another_listen_vap(mac_device_stru *pst_mac_device,
    uint8_t uc_vap_id_self)
{
    uint8_t uc_vap_idx;
    mac_vap_stru *pst_mac_vap = NULL;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_mac_vap == NULL) {
            continue;
        }

        if (uc_vap_id_self == pst_mac_vap->uc_vap_id) {
            continue;
        }

        if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN) {
            return pst_mac_vap;
        }
    }

    return NULL;
}


OAL_STATIC void wal_another_vap_stop_p2p_listen(mac_device_stru *pst_mac_device, uint8_t uc_self_vap_id)
{
    mac_vap_stru *pst_listen_mac_vap = NULL;
    hmac_vap_stru *pst_listen_hmac_vap = NULL;
    int32_t l_ret;

    pst_listen_mac_vap = wal_find_another_listen_vap(pst_mac_device, uc_self_vap_id);
    if (pst_listen_mac_vap == NULL) {
        oam_info_log0(uc_self_vap_id, OAM_SF_CFG,
            "{wal_another_vap_stop_p2p_listen::not find another p2p listen vap.}");
        return;
    }

    pst_listen_hmac_vap = mac_res_get_hmac_vap(pst_listen_mac_vap->uc_vap_id);
    if (oal_unlikely(pst_listen_hmac_vap == NULL)) {
        oam_error_log1(uc_self_vap_id, OAM_SF_CFG,
            "{wal_another_vap_stop_p2p_listen::fail to get hmac vap, listen vap id[%d]}",
            pst_listen_mac_vap->uc_vap_id);
        return;
    }

    oam_warning_log1(uc_self_vap_id, OAM_SF_CFG,
        "{wal_another_vap_stop_p2p_listen::begin to stop p2p listen, listen vap id[%d]}",
        pst_listen_mac_vap->uc_vap_id);

    l_ret = wal_p2p_stop_roc(pst_listen_mac_vap, pst_listen_hmac_vap->pst_net_device);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_error_log1(uc_self_vap_id, OAM_SF_CFG,
            "{wal_another_vap_stop_p2p_listen::fail to stop p2p listen, listen vap id[%d]}",
            pst_listen_mac_vap->uc_vap_id);
        return;
    }
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 wal_cfg80211_scan_param_check(oal_wiphy_stru *pst_wiphy,
    oal_cfg80211_scan_request_stru *pst_request, oal_net_device_stru *pst_netdev)
{
    if (oal_any_null_ptr2(pst_wiphy, pst_request)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_scan_param_check::scan failed, null ptr!}");
        return OAL_FALSE;
    }

    /* 判断扫描传入内存长度不能大于后续缓存空间大小，避免拷贝内存越界 */
    if (pst_request->ie_len > WLAN_WPS_IE_MAX_SIZE) {
        oam_error_log1(0, OAM_SF_CFG, "{wal_cfg80211_scan_param_check:: scan ie is too large to save. [%d]!}",
            pst_request->ie_len);
        return OAL_FALSE;
    }

    pst_netdev = pst_request->wdev->netdev;
    if (pst_netdev == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_scan_param_check::scan failed! pst_netdev null}");
        return OAL_FALSE;
    }
    return OAL_TRUE;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 wal_cfg80211_dfr_and_s3s4_param_check(void)
{
#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_dfr_and_s3s4_param_check::dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return OAL_FALSE;
    }
#endif  // #ifdef _PRE_WLAN_FEATURE_DFR

#ifdef  _PRE_CONFIG_HISI_S3S4_POWER_STATE
    if (g_st_recovery_info.device_s3s4_process_flag) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_dfr_and_s3s4_param_check::s3s4_process_status[%d]!}",
                         g_st_recovery_info.device_s3s4_process_flag);
        return OAL_FALSE;
    }
#endif

    return OAL_TRUE;
}

OAL_STATIC OAL_INLINE void wal_cfg80211_scan_sched_scan_stop(hmac_scan_stru *pst_scan_mgmt,
    oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev, mac_vap_stru *pst_mac_vap)
{
    /* 如果当前调度扫描在运行，先暂停调度扫描 */
    if (pst_scan_mgmt->pst_sched_scan_req != NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{wal_cfg80211_scan_sched_scan_stop::stop sched scan,before normal scan}");
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
        wal_cfg80211_sched_scan_stop(pst_wiphy, pst_netdev, 0);
#else
        wal_cfg80211_sched_scan_stop(pst_wiphy, pst_netdev);
#endif
    }
}

OAL_STATIC int32_t wal_cfg80211_scan_pre_para_check(oal_wiphy_stru *wiphy,
    oal_cfg80211_scan_request_stru *request, oal_net_device_stru *netdev)
{
    mac_vap_stru *mac_vap = NULL;

    if (wal_cfg80211_scan_param_check(wiphy, request, netdev) == OAL_FALSE) {
        return -OAL_EFAIL;
    }

    netdev = request->wdev->netdev;

    if (wal_cfg80211_dfr_and_s3s4_param_check() == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_scan:: dfr or s3s4 is processing!}");
        return -OAL_EFAIL;
    }

    /* 通过net_device 找到对应的mac_device_stru 结构 */
    mac_vap = oal_net_dev_priv(netdev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_scan::scan failed! pst_mac_vap null!}");
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC int32_t wal_cfg80211_scan(oal_wiphy_stru *pst_wiphy,
                                     oal_cfg80211_scan_request_stru *pst_request)
{
    hmac_device_stru *pst_hmac_device = NULL;
    mac_vap_stru *pst_mac_vap = NULL;
    hmac_scan_stru *scan_mgmt = NULL;
    oal_net_device_stru *pst_netdev = NULL;

    if (wal_cfg80211_scan_pre_para_check(pst_wiphy, pst_request, pst_netdev) != OAL_SUCC) {
        return -OAL_EFAIL;
    }

    pst_netdev = pst_request->wdev->netdev;
    /* 通过net_device 找到对应的mac_device_stru 结构 */
    pst_mac_vap = oal_net_dev_priv(pst_netdev);
#ifdef _PRE_WLAN_FEATURE_WAPI
    if (is_p2p_scan_req(pst_request) && (OAL_TRUE == hmac_user_is_wapi_connected(pst_mac_vap->uc_device_id))) {
        oam_warning_log0(0, OAM_SF_CFG, "{stop p2p scan under wapi!}");
        goto fail;
    }
#endif /* #ifdef _PRE_WLAN_FEATURE_WAPI */

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_error_log1(0, OAM_SF_CFG, "{wal_cfg80211_scan::hmac_device null device_id:%d!}", pst_mac_vap->uc_device_id);
        goto fail;
    }
#ifdef _PRE_WLAN_FEATURE_MONITOR
    if (pst_hmac_device->sniffer_mode != WLAN_SINFFER_OFF) { // sniffer抓包过程不处理扫描请求
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_scan::sniffer enable, scan abort!}");
        return -OAL_EFAIL;
    }
#endif
    scan_mgmt = &(pst_hmac_device->st_scan_mgmt);
    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
        "{wal_cfg80211_scan::start a new normal scan. n_channels[%d], ie_len[%d], n_ssid[%d]}",
        pst_request->n_channels, pst_request->ie_len, pst_request->n_ssids);
#ifdef _PRE_EMU
    pst_request->n_channels = 1;
#endif
    /* gong 不可以重叠扫描, 若上次未结束，需要返回busy  */
    /* 等待上次的扫描请求完成 */
    if (oal_wait_event_interruptible_m(scan_mgmt->st_wait_queue, (scan_mgmt->pst_request == NULL)) < 0) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{wal_cfg80211_scan::start a new scan failed, wait return error.}");
        /*lint -e801*/
        goto fail;
        /*lint +e801*/
    }

    /* p2p normal scan自己可以打断自己上一次的roc */
    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_scan::stop roc scan, before normal scan.}");
        if (wal_p2p_stop_roc(pst_mac_vap, pst_netdev) < 0) {
            /*lint -e801*/
            goto fail;
            /*lint +e801*/
        }
    }

    if (hmac_get_feature_switch(HMAC_MIRACAST_SINK_SWITCH)) {
        /* 如果有其他VAP处于P2P listen状态，则先取消listen */
        wal_another_vap_stop_p2p_listen(pst_hmac_device->pst_device_base_info, pst_mac_vap->uc_vap_id);
    }

    if ((pst_mac_vap->en_vap_state >= MAC_VAP_STATE_STA_JOIN_COMP &&
        pst_mac_vap->en_vap_state <= MAC_VAP_STATE_STA_WAIT_ASOC)) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_scan::stop normal scan while connecting.}");
        /*lint -e801*/
        goto fail;
        /*lint +e801*/
    }

    /* 保存当前内核下发的扫描请求到本地 */
    scan_mgmt->pst_request = pst_request;
    wal_cfg80211_scan_sched_scan_stop(scan_mgmt, pst_wiphy, pst_netdev, pst_mac_vap);

    /* 进入扫描 */
    if (OAL_SUCC != wal_scan_work_func(scan_mgmt, pst_netdev, pst_request)) {
        scan_mgmt->pst_request = NULL;
        return -OAL_EFAIL;
    }

    return OAL_SUCC;

fail:
    return -OAL_EFAIL;
}


OAL_STATIC void wal_set_crypto_info(hmac_conn_param_stru *pst_conn_param, oal_cfg80211_conn_stru *pst_sme)
{
    uint8_t *puc_ie_tmp = NULL;
    uint8_t *puc_wep_key = NULL;

#ifdef _PRE_WLAN_FEATURE_WAPI
    if (pst_sme->crypto.wpa_versions == WITP_WAPI_VERSION) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_cfg80211_connect::crypt ver is wapi!");
        pst_conn_param->uc_wapi = OAL_TRUE;
    } else {
        pst_conn_param->uc_wapi = OAL_FALSE;
    }
#endif

    if (pst_sme->privacy) {
        if ((pst_sme->key_len != 0) && (pst_sme->key != NULL)) {
            /* 设置wep加密信息 */
            pst_conn_param->uc_wep_key_len = pst_sme->key_len;
            pst_conn_param->uc_wep_key_index = pst_sme->key_idx;

            puc_wep_key = (uint8_t *)oal_memalloc(pst_sme->key_len);
            if (puc_wep_key == NULL) {
                oam_error_log1(0, OAM_SF_CFG, "{wal_set_crypto_info::puc_wep_key(%d) alloc mem return null ptr!}",
                               (uint32_t)(pst_sme->key_len));
                return;
            }
            if (EOK != memcpy_s(puc_wep_key, (uint32_t)pst_sme->key_len,
                                (uint8_t *)pst_sme->key, (uint32_t)(pst_sme->key_len))) {
                oam_error_log0(0, OAM_SF_CFG, "wal_set_crypto_info::memcpy fail!");
                oal_free(puc_wep_key);
                return;
            }
            pst_conn_param->puc_wep_key = puc_wep_key;

            return;
        }

        /* 优先查找 RSN 信息元素 */
        puc_ie_tmp = mac_find_ie(MAC_EID_RSN, (uint8_t *)pst_sme->ie, (int32_t)pst_sme->ie_len);
        if (puc_ie_tmp != NULL) {
            mac_ie_get_rsn_cipher(puc_ie_tmp, &pst_conn_param->st_crypto);

            oam_warning_log2(0, OAM_SF_WPA, "wal_set_crypto_info::get_rsn_cipher pair_suite[0]:0x%x pair_suite[1]:0x%x",
                             pst_conn_param->st_crypto.aul_pair_suite[0], pst_conn_param->st_crypto.aul_pair_suite[1]);
            oam_warning_log2(0, OAM_SF_WPA, "wal_set_crypto_info::aul_akm_suite[0]=0x%x, aul_akm_suite[1]=0x%x",
                pst_conn_param->st_crypto.aul_akm_suite[0], pst_conn_param->st_crypto.aul_akm_suite[1]);
        } else {
            /* WPA */
            puc_ie_tmp = mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_OUITYPE_WPA,
                (uint8_t *)pst_sme->ie, (int32_t)pst_sme->ie_len);
            if (puc_ie_tmp != NULL) {
                mac_ie_get_wpa_cipher(puc_ie_tmp, &pst_conn_param->st_crypto);
                oam_warning_log2(0, OAM_SF_WPA,
                    "wal_set_crypto_info::get_wpa_cipher pair_suite[0]:0x%x pair_suite[1]:0x%x",
                    pst_conn_param->st_crypto.aul_pair_suite[0], pst_conn_param->st_crypto.aul_pair_suite[1]);
            }
        }
    }
}


OAL_STATIC oal_bool_enum_uint8 wal_is_p2p_device(oal_net_device_stru *pst_net_device)
{
    mac_vap_stru *pst_mac_vap = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;

    pst_mac_vap = oal_net_dev_priv(pst_net_device);
    if (pst_mac_vap == NULL) {
        return OAL_TRUE;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if ((pst_hmac_vap != NULL) &&
        (pst_hmac_vap->pst_p2p0_net_device != NULL) &&
        (pst_net_device == pst_hmac_vap->pst_p2p0_net_device)) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}


OAL_STATIC void free_connect_param_resource(hmac_conn_param_stru *pst_conn_param)
{
    if (pst_conn_param->puc_wep_key != NULL) {
        oal_free(pst_conn_param->puc_wep_key);
        pst_conn_param->puc_wep_key = NULL;
    }
    if (pst_conn_param->puc_ie != NULL) {
        oal_free(pst_conn_param->puc_ie);
        pst_conn_param->puc_ie = NULL;
    }
}
static int32_t wal_cfg80211_connect_set_bssid_param(uint8_t *bssid, oal_cfg80211_conn_stru *pst_sme)
{
    if (pst_sme->bssid) {
        if (memcpy_s(bssid, OAL_MAC_ADDR_LEN, (uint8_t *)pst_sme->bssid, OAL_MAC_ADDR_LEN) != EOK) {
            oam_error_log0(0, OAM_SF_ASSOC, "wal_cfg80211_connect_set_bssid_param::memcpy fail!");
            return -OAL_EFAIL;
        }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
    } else if (pst_sme->bssid_hint) {
        if (memcpy_s(bssid, OAL_MAC_ADDR_LEN, (uint8_t *)pst_sme->bssid_hint, OAL_MAC_ADDR_LEN) != EOK) {
            oam_error_log0(0, OAM_SF_ASSOC, "wal_cfg80211_connect_set_bssid_param::memcpy fail!");
            return -OAL_EFAIL;
        }
    } else {
#else
    } else {
#endif
        oam_warning_log0(0, OAM_SF_ASSOC, "{wal_cfg80211_connect_set_bssid_param::bssid and bssid_hint is NULL.}");
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC int32_t wal_cfg80211_connect_pre_para_check(oal_wiphy_stru *wiphy,
    oal_net_device_stru *net_device, oal_cfg80211_conn_stru *sme)
{
    if (oal_any_null_ptr3(wiphy, net_device, sme)) {
        oam_error_log0(0, OAM_SF_ANY,
            "{wal_cfg80211_connect::connect failed, input params null!}\r\n");

        return -OAL_EINVAL;
    }

    if (wal_cfg80211_dfr_and_s3s4_param_check() == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_connect:: dfr or s3s4 is processing!}");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_MONITOR
OAL_STATIC int32_t wal_cfg80211_connect_sniffer_handle(oal_net_device_stru *net_device)
{
    hmac_device_stru *hmac_device = NULL;
    mac_vap_stru *mac_vap = NULL;

    /* 通过net_device 找到对应的mac_device_stru 结构 */
    mac_vap = oal_net_dev_priv(net_device);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_connect_sniffer_handle::pst_mac_vap null!}");
        return -OAL_EFAIL;
    }

    hmac_device = hmac_res_get_mac_dev(mac_vap->uc_device_id);
    if (hmac_device == NULL) {
        oam_error_log1(0, OAM_SF_CFG, "{wal_cfg80211_connect_sniffer_handle::hmac_device null device_id:%d!}",
            mac_vap->uc_device_id);
        return -OAL_EFAIL;
    }

    if (hmac_device->sniffer_mode != WLAN_SINFFER_OFF) { // sniffer抓包过程不处理关联请求
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_connect_sniffer_handle::sniffer enable, connect abort!}");
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}
#endif


OAL_STATIC int32_t wal_cfg80211_connect(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_net_device,
    oal_cfg80211_conn_stru *pst_sme)
{
    hmac_conn_param_stru st_mac_conn_param;
    int32_t ret;
    uint8_t *puc_ie = NULL;

    ret = wal_cfg80211_connect_pre_para_check(pst_wiphy, pst_net_device, pst_sme);
    if (ret != OAL_SUCC) {
        return ret;
    }
#ifdef _PRE_WLAN_FEATURE_MONITOR
    if (wal_cfg80211_connect_sniffer_handle(pst_net_device) != OAL_SUCC) {
        return -OAL_EFAIL;
    }
#endif

    if (wal_is_p2p_device(pst_net_device)) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_cfg80211_connect:connect stop, p2p device should not connect.");
        return -OAL_EINVAL;
    }
    /* 初始化驱动连接参数 */
    memset_s(&st_mac_conn_param, sizeof(hmac_conn_param_stru), 0, sizeof(hmac_conn_param_stru));

    /* 解析内核下发的 bssid */
    ret = wal_cfg80211_connect_set_bssid_param(st_mac_conn_param.auc_bssid, pst_sme);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ASSOC, "wal_cfg80211_connect_set_bssid_param:: fail!");
        return ret;
    }
    /* 解析内核下发的 ssid */
    st_mac_conn_param.uc_ssid_len = (uint8_t)pst_sme->ssid_len;
    ret = memcpy_s(st_mac_conn_param.auc_ssid, sizeof(st_mac_conn_param.auc_ssid),
                   (uint8_t *)pst_sme->ssid, st_mac_conn_param.uc_ssid_len);

    /* 解析内核下发的安全相关参数 */
    /* 设置认证类型 */
    st_mac_conn_param.en_auth_type = pst_sme->auth_type;

    /* 设置加密能力 */
    st_mac_conn_param.en_privacy = pst_sme->privacy;

    /* 获取内核下发的pmf是使能的结果 */
    st_mac_conn_param.en_mfp = pst_sme->mfp;

    if (pst_sme->channel != NULL) {
        st_mac_conn_param.uc_channel = oal_ieee80211_frequency_to_channel(pst_sme->channel->center_freq);
    }

    oam_warning_log4(0, OAM_SF_ANY,
        "{wal_cfg80211_connect::start a new connect, ssid_len[%d], auth_type[%d], privacy[%d], mfp[%d]}\r\n",
        pst_sme->ssid_len, pst_sme->auth_type, pst_sme->privacy, pst_sme->mfp);

    /* 设置加密参数 */
    wal_set_crypto_info(&st_mac_conn_param, pst_sme);
    if ((st_mac_conn_param.puc_wep_key == NULL) && (pst_sme->key_len > 0) && (pst_sme->key != NULL)) {
        oam_warning_log0(0, OAM_SF_ASSOC, "{wal_cfg80211_connect::puc_wep_key is NULL.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 设置关联P2P/WPS ie */
    st_mac_conn_param.ie_len = (uint32_t)(pst_sme->ie_len);
    if ((st_mac_conn_param.ie_len > 0) && (pst_sme->ie != NULL)) {
        puc_ie = (uint8_t *)oal_memalloc(pst_sme->ie_len);
        if (puc_ie == NULL) {
            oam_error_log1(0, OAM_SF_CFG, "{wal_scan_work_func::puc_ie(%d) alloc mem return null ptr!}",
                           (uint32_t)(pst_sme->ie_len));
            free_connect_param_resource(&st_mac_conn_param);
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }
        ret += memcpy_s(puc_ie, pst_sme->ie_len, (uint8_t *)pst_sme->ie, st_mac_conn_param.ie_len);
        st_mac_conn_param.puc_ie = puc_ie;
    }
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ASSOC, "wal_cfg80211_connect::memcpy fail!");
        free_connect_param_resource(&st_mac_conn_param);
        return -OAL_EFAIL;
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    wlan_pm_set_timeout(WLAN_SLEEP_LONG_CHECK_CNT);
#endif

    
    wal_force_scan_abort_then_scan_comp(pst_net_device);

    
    /* 抛事件给驱动，启动关联 */
    ret = wal_cfg80211_start_connect(pst_net_device, &st_mac_conn_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_connect::wal_cfg80211_start_connect fail %d!}\r\n", ret);
        /* 事件下发失败在WAL释放，下发成功，无论succ fail HMAC会释放 */
        if (oal_value_ne_all2(ret, -OAL_EFAIL, -OAL_ETIMEDOUT)) {
            free_connect_param_resource(&st_mac_conn_param);
        }
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC int32_t wal_cfg80211_disconnect(oal_wiphy_stru *pst_wiphy,
                                           oal_net_device_stru *pst_net_device,
                                           uint16_t us_reason_code)
{
    mac_cfg_kick_user_param_stru st_mac_cfg_kick_user_param;
    int32_t l_ret;

    mac_user_stru *pst_mac_user = NULL;
    mac_vap_stru *pst_mac_vap = NULL;

    if (oal_any_null_ptr2(pst_wiphy, pst_net_device)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_disconnect::pst_wiphy or pst_netdev is null!}");
        return -OAL_EINVAL;
    }

    oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_disconnect:: deauth us_reason_code=%d}", us_reason_code);

    if (wal_cfg80211_dfr_and_s3s4_param_check() == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_disconnect:: dfr or s3s4 is processing!}");
        return -OAL_EFAIL;
    }

    pst_mac_vap = oal_net_dev_priv(pst_net_device);
    if (pst_mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_disconnect::mac vap has already been del!}");
        oal_io_print("wal_cfg80211_disconnect:: mac vap has already been del!ifname %s\r\n", pst_net_device->name);
        return OAL_SUCC;
    }

    /* 填写和sta关联的ap mac 地址 */
    pst_mac_user = mac_res_get_mac_user(pst_mac_vap->us_assoc_vap_id);
    if (pst_mac_user == NULL) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_cfg80211_disconnect::mac_res_get_mac_user pst_mac_user is null, user idx[%d]!}\r\n",
            pst_mac_vap->us_assoc_vap_id);
        return OAL_SUCC;
    }

    /* 解析内核下发的connect参数 */
    memset_s(&st_mac_cfg_kick_user_param, sizeof(mac_cfg_kick_user_param_stru),
             0, sizeof(mac_cfg_kick_user_param_stru));

    /* 解析内核下发的去关联原因  */
    st_mac_cfg_kick_user_param.us_reason_code = us_reason_code;

    memcpy_s(st_mac_cfg_kick_user_param.auc_mac_addr, WLAN_MAC_ADDR_LEN,
             pst_mac_user->auc_user_mac_addr, WLAN_MAC_ADDR_LEN);

    l_ret = wal_cfg80211_start_disconnect(pst_net_device, &st_mac_cfg_kick_user_param);
    if (l_ret != OAL_SUCC) {
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC int32_t wal_cfg80211_add_key(oal_wiphy_stru *pst_wiphy,
                                        oal_net_device_stru *pst_netdev,
                                        uint8_t uc_key_index,
                                        bool en_pairwise,
                                        const uint8_t *puc_mac_addr,
                                        oal_key_params_stru *pst_params)
{
    wal_msg_write_stru write_msg;
    mac_addkey_param_stru payloadParams;
    wal_msg_stru *p_rsp_msg = NULL;
    int32_t ret = EOK;

    /* 1.1 入参检查 */
    if (oal_any_null_ptr3(pst_wiphy, pst_netdev, pst_params)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_add_key::pst_wiphy or pst_netdev or pst_params is null!}");
        return -OAL_EINVAL;
    }

    /* 1.2 key长度检查，防止拷贝越界 */
    if ((pst_params->key_len > OAL_WPA_KEY_LEN) || (pst_params->key_len < 0) ||
        (pst_params->seq_len > OAL_WPA_SEQ_LEN) || (pst_params->seq_len < 0)) {
        oam_error_log2(0, OAM_SF_ANY, "{wal_cfg80211_add_key::Param Check ERROR! key_len[%x]  seq_len[%x]!}\r\n",
                       (int32_t)pst_params->key_len, (int32_t)pst_params->seq_len);
        return -OAL_EINVAL;
    }

    /* 2.1 消息参数准备 */
    memset_s(&payloadParams, sizeof(payloadParams), 0, sizeof(payloadParams));
    payloadParams.uc_key_index = uc_key_index;

    memset_s(payloadParams.auc_mac_addr, WLAN_MAC_ADDR_LEN, 0, WLAN_MAC_ADDR_LEN);
    if (puc_mac_addr != NULL) {
        /* 不能使用内核下发的mac指针，可能被释放，需要拷贝到本地再使用 */
        ret += memcpy_s(payloadParams.auc_mac_addr, WLAN_MAC_ADDR_LEN, puc_mac_addr, WLAN_MAC_ADDR_LEN);
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44)
    payloadParams.en_pairwise = en_pairwise;
#else
    payloadParams.en_pairwise = (puc_mac_addr != NULL) ? OAL_TRUE : OAL_FALSE;
#endif
#else
    payloadParams.en_pairwise = en_pairwise;
#endif

    /* 2.2 获取相关密钥值 */
    payloadParams.st_key.key_len = pst_params->key_len;
    payloadParams.st_key.seq_len = pst_params->seq_len;
    payloadParams.st_key.cipher = pst_params->cipher;
    ret += memcpy_s(payloadParams.st_key.auc_key, OAL_WPA_KEY_LEN, pst_params->key, (uint32_t)pst_params->key_len);
    oam_warning_log4(0, OAM_SF_ANY, "{wal_cfg80211_add_key::key_len:%d, key:0X%X, cipher:0X%X, ret=0X%X!}\r\n",
        pst_params->key_len, pst_params->key, pst_params->cipher, ret);
    ret += memcpy_s(payloadParams.st_key.auc_seq, OAL_WPA_SEQ_LEN, pst_params->seq, (uint32_t)pst_params->seq_len);
    oam_warning_log4(0, OAM_SF_ANY, "{wal_cfg80211_add_key::puc_mac_addr:0X%X, seq_len:%d, seq:0X%X, ret =0X%X}\r\n",
        puc_mac_addr, pst_params->seq_len, pst_params->seq, ret);

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 3.1 填写 msg 消息头 */
    write_msg.en_wid = WLAN_CFGID_ADD_KEY;
    write_msg.us_len = sizeof(mac_addkey_param_stru);

    /* 3.2 填写 msg 消息体 */
    ret += memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value),
        &payloadParams, sizeof(mac_addkey_param_stru));
    if (ret != EOK) {
        oam_warning_log1(0, OAM_SF_ANY, "wal_cfg80211_add_key::memcpy fail! ret[%X]", ret);
    }

    /* 由于消息中使用了局部变量指针，因此需要将发送该函数设置为同步，否则hmac处理时会使用野指针 */
    if (OAL_SUCC != wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_addkey_param_stru),
                                       (uint8_t *)&write_msg, OAL_TRUE, &p_rsp_msg)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_add_key::wal_send_cfg_event fail!}");
        return -OAL_EFAIL;
    }
    if (OAL_SUCC != wal_check_and_release_msg_resp(p_rsp_msg)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_add_key::wal_check_and_release_msg_resp fail!}");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC int32_t wal_cfg80211_get_key(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
    uint8_t uc_key_index, bool en_pairwise, const uint8_t *puc_mac_addr,
    void *cookie, void (*callback)(void *cookie, oal_key_params_stru *))
{
    wal_msg_write_stru st_write_msg;
    hmac_getkey_param_stru st_payload_params = { 0 };
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN];
    wal_msg_stru *pst_rsp_msg = NULL;
    int32_t l_ret = EOK;

    /* 1.1 入参检查 */
    if (oal_any_null_ptr4(pst_wiphy, pst_netdev, cookie, callback)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_get_key::wiphy or netdev or cookie or callback is null!}");
        return -OAL_EINVAL;
    }

    /* 2.1 消息参数准备 */
    st_payload_params.pst_netdev = pst_netdev;
    st_payload_params.uc_key_index = uc_key_index;

    if (puc_mac_addr != NULL) {
        /* 不能使用内核下发的mac指针，可能被释放，需要拷贝到本地再使用 */
        l_ret += memcpy_s(auc_mac_addr, WLAN_MAC_ADDR_LEN, puc_mac_addr, WLAN_MAC_ADDR_LEN);
        st_payload_params.puc_mac_addr = auc_mac_addr;
    } else {
        st_payload_params.puc_mac_addr = NULL;
    }

    st_payload_params.en_pairwise = en_pairwise;
    st_payload_params.cookie = cookie;
    st_payload_params.callback = callback;

    oam_info_log2(0, OAM_SF_ANY, "{wal_cfg80211_get_key::key_idx:%d, en_pairwise:%d!}\r\n",
        uc_key_index, st_payload_params.en_pairwise);
    if (puc_mac_addr != NULL) {
        oam_info_log4(0, OAM_SF_ANY, "{wal_cfg80211_get_key::MAC ADDR: %02X:XX:XX:%02X:%02X:%02X!}\r\n",
                      puc_mac_addr[0], puc_mac_addr[3], puc_mac_addr[4], puc_mac_addr[5]);
    } else {
        oam_info_log0(0, OAM_SF_ANY, "{wal_cfg80211_get_key::MAC ADDR IS null!}\r\n");
    }
    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 3.1 填写 msg 消息头 */
    st_write_msg.en_wid = WLAN_CFGID_GET_KEY;
    st_write_msg.us_len = sizeof(hmac_getkey_param_stru);

    /* 3.2 填写 msg 消息体 */
    l_ret += memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                      &st_payload_params, sizeof(hmac_getkey_param_stru));
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_cfg80211_get_key::memcpy fail!");
        return -OAL_EINVAL;
    }

    /* 由于消息中使用了局部变量指针，因此需要将发送该函数设置为同步，否则hmac处理时会使用野指针 */
    if (OAL_SUCC != wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(hmac_getkey_param_stru),
                                       (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_get_key::wal_send_cfg_event fail.}");
        return -OAL_EINVAL;
    }

    if (OAL_SUCC != wal_check_and_release_msg_resp(pst_rsp_msg)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_get_key::wal_check_and_release_msg_resp fail.}");
        return -OAL_EINVAL;
    }

    return OAL_SUCC;
}


OAL_STATIC int32_t wal_cfg80211_remove_key(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
    uint8_t uc_key_index, bool en_pairwise, const uint8_t *puc_mac_addr)
{
    mac_removekey_param_stru st_payload_params = { 0 };
    wal_msg_write_stru st_write_msg = { 0 };
    wal_msg_stru *pst_rsp_msg = NULL;
    int32_t l_ret = EOK;

    /* 1.1 入参检查 */
    if (oal_any_null_ptr2(pst_wiphy, pst_netdev)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_remove_key::pst_wiphy or pst_netdev is null!}");
        return -OAL_EINVAL;
    }

    if (wal_cfg80211_dfr_and_s3s4_param_check() == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_remove_key:: dfr or s3s4 is processing!}");
        return OAL_SUCC;
    }

    /* 2.1 消息参数准备 */
    st_payload_params.uc_key_index = uc_key_index;
    memset_s(st_payload_params.auc_mac_addr, OAL_MAC_ADDR_LEN, 0, OAL_MAC_ADDR_LEN);
    if (puc_mac_addr != NULL) {
        /* 不能使用内核下发的mac指针，可能被释放，需要拷贝到本地再使用 */
        l_ret += memcpy_s(st_payload_params.auc_mac_addr, OAL_MAC_ADDR_LEN, puc_mac_addr, WLAN_MAC_ADDR_LEN);
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    st_payload_params.en_pairwise = en_pairwise;
#else
    st_payload_params.en_pairwise = en_pairwise;
#endif

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 3.1 填写 msg 消息头 */
    st_write_msg.en_wid = WLAN_CFGID_REMOVE_KEY;
    st_write_msg.us_len = sizeof(mac_removekey_param_stru);

    /* 3.2 填写 msg 消息体 */
    l_ret += memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                      &st_payload_params, sizeof(mac_removekey_param_stru));
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_cfg80211_remove_key::memcpy fail!");
        return -OAL_EFAIL;
    }

    if (OAL_SUCC != wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_removekey_param_stru),
                                       (uint8_t *)&st_write_msg,
                                       OAL_TRUE, &pst_rsp_msg)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_remove_key::wal_send_cfg_event fail.}");
        return -OAL_EFAIL;
    }

    if (OAL_SUCC != wal_check_and_release_msg_resp(pst_rsp_msg)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_remove_key::wal_check_and_release_msg_resp fail.}");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC int32_t wal_cfg80211_set_default_key(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
    uint8_t uc_key_index, bool en_unicast, bool en_multicast)
{
    uint8_t  idx = 0;
    uint32_t cmd_id;
    uint32_t params[CMD_PARAMS_MAX_CNT] = { 0 };
    uint32_t offset;
    uint32_t ret;

    /* 1.1 入参检查 */
    if (oal_unlikely(oal_any_null_ptr2(pst_wiphy, pst_netdev))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_set_default_key::wiphy or netdev ptr is null!}");
        return -OAL_EINVAL;
    }

    /* 2.1 参数准备 */
    /* set_default_key x(key_index) 0|1(en_unicast) 0|1(multicast) */
    params[idx++] = uc_key_index;
    params[idx++] = en_unicast;
    params[idx++] = en_multicast;

    ret = wal_get_cmd_id("set_default_key", &cmd_id, &offset);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "wal_cfg80211_set_default_key:find set_default_key cmd is fail");
        return ret;
    }

    return wal_process_cmd_params(pst_netdev, cmd_id, params);
}


int32_t wal_cfg80211_set_default_mgmt_key(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
    uint8_t uc_key_index)
{
    uint8_t  idx = 0;
    uint32_t cmd_id;
    uint32_t params[CMD_PARAMS_MAX_CNT] = { 0 };
    uint32_t offset;
    uint32_t ret;

    /* 1.1 入参检查 */
    if (oal_unlikely(oal_any_null_ptr2(pst_wiphy, pst_netdev))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_set_default_mgmt_key::wiphy or netdev is null!}");
        return -OAL_EINVAL;
    }
    /* 2.1 参数准备 */
    /* set_default_key x(key_index) 0|1(en_unicast) 0|1(multicast) */
    params[idx++] = uc_key_index;
    params[idx++] = OAL_FALSE;
    params[idx++] = OAL_TRUE;

    ret = wal_get_cmd_id("set_default_key", &cmd_id, &offset);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "wal_cfg80211_set_default_mgmt_key:find set_default_key cmd is fail");
        return ret;
    }

    return wal_process_cmd_params(pst_netdev, cmd_id, params);
}

uint8_t wal_cfg80211_convert_value_to_vht_width(int32_t l_channel_value)
{
    uint8_t uc_channel_vht_width = WLAN_MIB_VHT_OP_WIDTH_20_40;

    switch (l_channel_value) {
        case 20:  /* 20M带宽 */
        case 40:  /* 40M带宽 */
            uc_channel_vht_width = WLAN_MIB_VHT_OP_WIDTH_20_40;
            break;
        case 80:  /* 80M带宽 */
            uc_channel_vht_width = WLAN_MIB_VHT_OP_WIDTH_80;
            break;
        case 160: /* 160M带宽 */
            uc_channel_vht_width = WLAN_MIB_VHT_OP_WIDTH_160;
            break;
        default:
            break;
    }

    return uc_channel_vht_width;
}


OAL_STATIC int32_t wal_cfg80211_set_wiphy_params(oal_wiphy_stru *pst_wiphy, uint32_t changed)
{
    /* 通过HOSTAPD 设置RTS 门限，分片门限 采用接口wal_ioctl_set_frag， wal_ioctl_set_rts */
    oam_warning_log0(0, OAM_SF_CFG,
        "{wal_cfg80211_set_wiphy_params::should not call this function.call wal_ioctl_set_frag/wal_ioctl_set_rts!}");
    return OAL_SUCC;
}


OAL_STATIC int32_t wal_cfg80211_set_ssid(oal_net_device_stru *pst_netdev,
                                         uint8_t *puc_ssid_ie,
                                         uint8_t uc_ssid_len)
{
    wal_msg_write_stru st_write_msg;
    mac_cfg_ssid_param_stru *pst_ssid_param;
    int32_t l_ret;

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SSID, sizeof(mac_cfg_ssid_param_stru));

    pst_ssid_param = (mac_cfg_ssid_param_stru *)(st_write_msg.auc_value);
    pst_ssid_param->uc_ssid_len = uc_ssid_len;
    if (EOK != memcpy_s(pst_ssid_param->ac_ssid, sizeof(pst_ssid_param->ac_ssid),
        (int8_t *)puc_ssid_ie, uc_ssid_len)) {
        oam_error_log0(0, OAM_SF_ANY, "wal_cfg80211_set_ssid::memcpy fail!");
        return -OAL_EFAIL;
    }

    l_ret = wal_send_cfg_event(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_ssid_param_stru),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_set_ssid::fail to send ssid cfg msg, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC int32_t wal_cfg80211_set_auth_mode(oal_net_device_stru *pst_netdev, uint8_t en_auth_algs)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;

    if ((en_auth_algs != WLAN_WITP_AUTH_OPEN_SYSTEM) &&
        (en_auth_algs != WLAN_WITP_AUTH_SHARED_KEY)
#ifdef _PRE_WLAN_FEATURE_SAE
        && (en_auth_algs != WLAN_WITP_AUTH_SAE)
        && (en_auth_algs != WLAN_WITP_AUTH_TBPEKE)
#endif
        && (en_auth_algs != WLAN_WITP_AUTH_AUTOMATIC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_set_auth_mode::en_auth_algs error[%d].}", en_auth_algs);
        return -OAL_EFAIL;
    }

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_AUTH_MODE, sizeof(uint32_t));
    *((uint32_t *)(st_write_msg.auc_value)) = en_auth_algs; /* 设置配置命令参数 */

    l_ret = wal_send_cfg_event(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint32_t),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (l_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_cfg80211_set_auth_mode::fail to send auth_tpye cfg msg, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC int32_t wal_cfg80211_fill_beacon_param(oal_net_device_stru *pst_netdev,
                                                  oal_beacon_data_stru *pst_beacon_info,
                                                  mac_beacon_param_stru *pst_beacon_param)
{
    oal_beacon_parameters st_beacon_info_tmp;
    uint8_t *puc_beacon_info_tmp = NULL;
    uint32_t beacon_head_len, beacon_tail_len, offset, ret;
    uint8_t uc_vap_id;
    mac_vap_stru *pst_mac_vap = NULL;
    int32_t l_ret = EOK;

    if (oal_any_null_ptr3(pst_netdev, pst_beacon_info, pst_beacon_param)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::netdev or beacon_info, beacon_param is null");
        return -OAL_EINVAL;
    }
    /* 获取vap id */
    pst_mac_vap = oal_net_dev_priv(pst_netdev);
    if (pst_mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::pst_mac_vap is null}");
        return -OAL_EINVAL;
    }

    uc_vap_id = pst_mac_vap->uc_vap_id;
    /*****************************************************************************
        1.安全配置ie消息等
    *****************************************************************************/
    if (oal_any_null_ptr2(pst_beacon_info->tail, pst_beacon_info->head)) {
        oam_error_log0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::tail or head is null!}");
        return -OAL_EINVAL;
    }

    offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    /* oal_ieee80211_mgmt 前面公共部分size为 MAC_80211_FRAME_LEN(24)  */
    if (pst_beacon_info->head_len < (offset + MAC_80211_FRAME_LEN)) {
        oam_error_log1(0, OAM_SF_CFG, "{wal_cfg80211_fill_beacon_param::pst_beacon_info head_len[%d] error.}",
            pst_beacon_info->head_len);
        return -OAL_EINVAL;
    }

    beacon_head_len = (uint32_t)pst_beacon_info->head_len;
    beacon_tail_len = (uint32_t)pst_beacon_info->tail_len;

    /* 运算溢出保护 */
    if ((beacon_head_len + beacon_tail_len) < beacon_head_len) {
        oam_error_log2(0, 0, "{wal_cfg80211_fill_beacon_param::beacon_head_len:%d, beacon_tail_len:%d. len abnormal.}",
            beacon_head_len, beacon_tail_len);
        return -OAL_EINVAL;
    }

    puc_beacon_info_tmp = (uint8_t *)(oal_memalloc(beacon_head_len + beacon_tail_len));
    if (puc_beacon_info_tmp == NULL) {
        oam_error_log0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::puc_beacon_info_tmp memalloc failed.}");
        return -OAL_EINVAL;
    } else {
        l_ret += memcpy_s(puc_beacon_info_tmp, beacon_head_len + beacon_tail_len,
                          pst_beacon_info->head, beacon_head_len);
        l_ret += memcpy_s(puc_beacon_info_tmp + beacon_head_len, beacon_tail_len,
                          pst_beacon_info->tail, beacon_tail_len);
    }
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "wal_cfg80211_fill_beacon_param::memcpy fail!");
        oal_free(puc_beacon_info_tmp);
        return -OAL_EINVAL;
    }

    /* 为了复用51的解析接口，将新内核结构中的内容赋值给51接口识别的结构体，进而获取信息元素 */
    memset_s(&st_beacon_info_tmp, sizeof(st_beacon_info_tmp), 0, sizeof(st_beacon_info_tmp));
    st_beacon_info_tmp.head = puc_beacon_info_tmp;
    st_beacon_info_tmp.head_len = (int32_t)beacon_head_len;
    st_beacon_info_tmp.tail = puc_beacon_info_tmp + beacon_head_len;
    st_beacon_info_tmp.tail_len = (int32_t)beacon_tail_len;

    /* 获取 WPA/WPA2 信息元素 */
    wal_parse_wpa_wpa2_ie(&st_beacon_info_tmp, pst_beacon_param);

    /* 此接口需要修改，linux上没问题，但是win32有错 TBD */
    ret = wal_parse_ht_vht_ie(pst_mac_vap, &st_beacon_info_tmp, pst_beacon_param);
    if (ret != OAL_SUCC) {
        oam_warning_log0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::failed to parse HT/VHT ie!}");
        oal_free(puc_beacon_info_tmp);
        return -OAL_EINVAL;
    }

    /* 解析是否隐藏SSID */
    if (puc_beacon_info_tmp[WAL_BCN_BSSID_LENGTH_ADDR] == 0) {
        pst_beacon_param->uc_hidden_ssid = 1;
    }

    /* 配置Wmm信息元素 */
    if (wal_parse_wmm_ie(pst_netdev, pst_mac_vap, &st_beacon_info_tmp) != OAL_SUCC) {
        oam_warning_log0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::Failed to parse wmm ie!}");
        oal_free(puc_beacon_info_tmp);
        return -OAL_EINVAL;
    }
    /* 释放临时申请的内存 */
    oal_free(puc_beacon_info_tmp);

#ifdef _PRE_WLAN_FEATURE_11D
    /* 对日本14信道作特殊判断，只在11b模式下才能启用14，非11b模式 降为11b */
    if ((pst_mac_vap->st_channel.uc_chan_number == 14) && (pst_beacon_param->en_protocol != WLAN_LEGACY_11B_MODE)) {
        oam_error_log1(uc_vap_id, OAM_SF_ANY,
            "{wal_cfg80211_fill_beacon_param::ch 14 should in 11b, but is %d!}", pst_beacon_param->en_protocol);
        oam_error_log0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::change protocol to 11b!}");
        pst_beacon_param->en_protocol = WLAN_LEGACY_11B_MODE;
    }
#endif

    return OAL_SUCC;
}


OAL_STATIC int32_t wal_cfg80211_change_beacon(oal_wiphy_stru *pst_wiphy,
                                              oal_net_device_stru *pst_netdev,
                                              oal_beacon_data_stru *pst_beacon_info)
{
    mac_beacon_param_stru st_beacon_param; /* beacon info struct */
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;

    /* 参数合法性检查 */
    if (oal_any_null_ptr3(pst_wiphy, pst_netdev, pst_beacon_info)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_change_beacon:: \
            pst_wiphy or pst_netdev or pst_beacon_info is null!}");
        return -OAL_EINVAL;
    }

    if (wal_cfg80211_dfr_and_s3s4_param_check() == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_change_beacon:: dfr or s3s4 is processing!}");
        return OAL_SUCC;
    }

    /* 初始化beacon interval 和DTIM_PERIOD 参数 */
    memset_s(&st_beacon_param, sizeof(mac_beacon_param_stru), 0, sizeof(mac_beacon_param_stru));

    l_ret = wal_cfg80211_fill_beacon_param(pst_netdev, pst_beacon_info, &st_beacon_param);
    if (l_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_cfg80211_change_beacon::failed to fill beacon param, error[%d]}", l_ret);
        return -OAL_EINVAL;
    }

    /* 设置操作类型 */
    st_beacon_param.en_operation_type = MAC_SET_BEACON;

    /* 填写 msg 消息头 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_CONFIG_BEACON, sizeof(mac_beacon_param_stru));

    /* 填写 msg 消息体 */
    if (EOK != memcpy_s(st_write_msg.auc_value, sizeof(mac_beacon_param_stru),
                        &st_beacon_param, sizeof(mac_beacon_param_stru))) {
        oam_error_log0(0, OAM_SF_ANY, "wal_cfg80211_change_beacon::memcpy fail!");
        return -OAL_EFAIL;
    }

    /* 发送消息 */
    l_ret = wal_send_cfg_event(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_beacon_param_stru),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_change_beacon::Fail to start addset beacon, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC int32_t wal_cfg80211_convert_width_to_value(int32_t l_channel_width)
{
    int32_t l_channel_width_value = 0;

    switch (l_channel_width) {
        case WIFI_CHAN_WIDTH_20:
        case WIFI_CHAN_WIDTH_40:
            l_channel_width_value = 20; /* 20M */
            break;
        case WIFI_CHAN_WIDTH_80:
            l_channel_width_value = 40; /* 40M */
            break;
        case WIFI_CHAN_WIDTH_160:
        case WIFI_CHAN_WIDTH_80P80:
            l_channel_width_value = 80; /* 80M */
            break;
        case WIFI_CHAN_WIDTH_5:
            l_channel_width_value = 160; /* 160M */
            break;
        default:
            break;
    }

    return l_channel_width_value;
}
#define WLAN_CHAN_OFFSET_PLUS_2 2
#define WLAN_CHAN_OFFSET_MINUS_2 (-2)
OAL_STATIC wlan_channel_bandwidth_enum_uint8 wal_cfg80211_set_channel_cal_bw(oal_net_device_stru *netdev)
{
    int32_t channel_center_freq0;
    int32_t channel_center_freq1;
    int32_t bandwidth_value;
    uint8_t  vht_width;
    int32_t channel;
    wlan_channel_bandwidth_enum_uint8  channl_band_width;
    oal_ieee80211_channel *p_chan_stru = NULL;
    mac_vap_stru          *p_mac_vap = NULL;
    mac_device_stru       *p_mac_dev = NULL;

    p_mac_vap = oal_net_dev_priv(netdev);

    p_chan_stru = netdev->ieee80211_ptr->preset_chandef.chan;
    channel     = p_chan_stru->hw_value;

    /* 进行内核带宽值和WITP 带宽值转换 */
    channel_center_freq0 = oal_ieee80211_frequency_to_channel(netdev->ieee80211_ptr->preset_chandef.center_freq1);
    channel_center_freq1 = oal_ieee80211_frequency_to_channel(netdev->ieee80211_ptr->preset_chandef.center_freq2);
    bandwidth_value = wal_cfg80211_convert_width_to_value(netdev->ieee80211_ptr->preset_chandef.width);
    vht_width = wal_cfg80211_convert_value_to_vht_width(bandwidth_value);

    if (bandwidth_value == 0) {
        oam_error_log1(0, 0, "{wal_cfg80211_cal_bw::chan width invalid, bandwidth=%d",
            netdev->ieee80211_ptr->preset_chandef.width);
        return -OAL_EINVAL;
    }

    if (bandwidth_value == 80 || bandwidth_value == 160) { /* 80 160  为上层下发带宽参数 */
        channl_band_width = mac_get_bandwith_from_center_freq_seg0_seg1(vht_width, (uint8_t)channel,
            (uint8_t)channel_center_freq0, (uint8_t)channel_center_freq1);
    } else if (bandwidth_value == 40) { /* 40为上层下发带宽参数 */
        switch (channel_center_freq0 - channel) {
            case WLAN_CHAN_OFFSET_MINUS_2:
                channl_band_width = WLAN_BAND_WIDTH_40MINUS;
                break;
            case WLAN_CHAN_OFFSET_PLUS_2:
                channl_band_width = WLAN_BAND_WIDTH_40PLUS;
                break;
            default:
                channl_band_width = WLAN_BAND_WIDTH_20M;
                break;
        }
    } else {
        channl_band_width = WLAN_BAND_WIDTH_20M;
    }

    p_mac_dev = (mac_device_stru *)mac_res_get_dev(p_mac_vap->uc_device_id);
    if (p_mac_dev != NULL && p_mac_dev->st_p2p_info.p2p_scenes == MAC_P2P_SCENES_LOW_LATENCY) {
        mac_vap_p2p_bw_back_to_40m(p_mac_vap, channel, &channl_band_width);
    }
    return channl_band_width;
}
#ifdef _PRE_WLAN_CHBA_MGMT
int32_t wal_chba_set_channel(oal_net_device_stru *net_dev)
{
    mac_cfg_channel_param_stru *pst_channel_param = NULL;
    wal_msg_write_stru st_write_msg;
    mac_vap_stru *pst_mac_vap = NULL;
    wal_msg_stru *pst_rsp_msg = NULL;
    uint8_t uc_vap_id;
    int32_t l_ret;
    uint32_t err_code;
    /* 获取vap id */
    pst_mac_vap = oal_net_dev_priv(net_dev);
    uc_vap_id = pst_mac_vap->uc_vap_id;

    pst_channel_param = (mac_cfg_channel_param_stru *)(st_write_msg.auc_value);
    pst_channel_param->uc_channel = 36;
    pst_channel_param->en_band = WLAN_BAND_5G;
    pst_channel_param->en_bandwidth = WLAN_BAND_WIDTH_80PLUSPLUS;

    oam_warning_log3(uc_vap_id, OAM_SF_ANY, "{wal_chba_set_channel::uc_channel=%d, en_band=%d, en_bandwidth=%d.}",
        pst_channel_param->uc_channel, pst_channel_param->en_band, pst_channel_param->en_bandwidth);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_SET_CHANNEL, sizeof(mac_cfg_channel_param_stru));

    /* 发送消息 */
    l_ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_channel_param_stru),
        (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_P2P, "{wal_chba_set_channel::wal_send_cfg_event return err:%d}", l_ret);
        return -OAL_EFAIL;
    }

    /* 读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(uc_vap_id, OAM_SF_ANY,
            "{wal_chba_set_channel::wal_check_and_release_msg_resp fail return err code:%u}", err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#endif

OAL_STATIC int32_t wal_cfg80211_set_channel_info(oal_wiphy_stru *pst_wiphy,
                                                 oal_net_device_stru *pst_netdev)
{
    mac_cfg_channel_param_stru *pst_channel_param = NULL;
    oal_ieee80211_channel *pst_channel = NULL;
    wlan_channel_bandwidth_enum_uint8 en_bandwidth;
    wal_msg_write_stru st_write_msg;
    uint32_t err_code;
    int32_t l_channel;
    int32_t l_center_freq1;
    int32_t l_center_freq2;
    wal_msg_stru *pst_rsp_msg = NULL;
    int32_t l_ret;
    mac_vap_stru *pst_mac_vap = oal_net_dev_priv(pst_netdev);
    uint8_t uc_vap_id;
    int32_t l_bandwidth = pst_netdev->ieee80211_ptr->preset_chandef.width;

    /* 获取vap id */
    uc_vap_id = pst_mac_vap->uc_vap_id;

    l_center_freq1 = pst_netdev->ieee80211_ptr->preset_chandef.center_freq1;
    l_center_freq2 = pst_netdev->ieee80211_ptr->preset_chandef.center_freq2;
    pst_channel = pst_netdev->ieee80211_ptr->preset_chandef.chan;
    l_channel = pst_channel->hw_value;

    oam_warning_log4(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_set_channel_info::\
        l_bandwidth = %d, l_center_freq1 = %d, l_center_freq2 = %d, l_channel = %d.}",
        l_bandwidth, l_center_freq1, l_center_freq2, l_channel);

    /* 判断信道在不在管制域内 */
    l_ret = (int32_t)mac_is_channel_num_valid(pst_channel->band, (uint8_t)l_channel,
        (uint8_t)(pst_channel->center_freq >= 5955)); /* 5955是6G的起始中心频点 */
    if (l_ret != OAL_SUCC) {
        oam_warning_log2(uc_vap_id, OAM_SF_ANY,
            "{wal_cfg80211_set_channel_info::chan num invalid. band, ch num [%d] [%d]!}", pst_channel->band, l_channel);
        return -OAL_EINVAL;
    }

    en_bandwidth = wal_cfg80211_set_channel_cal_bw(pst_netdev);

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 填写消息 */
    pst_channel_param = (mac_cfg_channel_param_stru *)(st_write_msg.auc_value);
    memset_s(pst_channel_param, sizeof(mac_cfg_channel_param_stru), 0, sizeof(mac_cfg_channel_param_stru));
    pst_channel_param->uc_channel = (uint8_t)pst_channel->hw_value;
    pst_channel_param->en_band = pst_channel->band;
    pst_channel_param->en_bandwidth = en_bandwidth;

    oam_warning_log3(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_set_channel::uc_channel=%d, en_band=%d, en_bandwidth=%d.}",
                     pst_channel_param->uc_channel, pst_channel_param->en_band, pst_channel_param->en_bandwidth);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_SET_CHANNEL, sizeof(mac_cfg_channel_param_stru));

    /* 发送消息 */
    l_ret = wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_channel_param_stru),
                               (uint8_t *)&st_write_msg,
                               OAL_TRUE, &pst_rsp_msg);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_P2P, "{wal_cfg80211_set_channel_info::wal_send_cfg_event return err:%d}", l_ret);
        return -OAL_EFAIL;
    }

    /* 读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(uc_vap_id, OAM_SF_ANY,
            "{wal_cfg80211_set_channel_info::wal_check_and_release_msg_resp fail return err code:%u}", err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC int32_t wal_cfg80211_start_ap(oal_wiphy_stru *pst_wiphy,
                                         oal_net_device_stru *pst_netdev,
                                         oal_ap_settings_stru *pst_ap_settings)
{
    mac_beacon_param_stru st_beacon_param; /* beacon info struct */
    wal_msg_write_stru st_write_msg;
    mac_vap_stru *pst_mac_vap = NULL;
    oal_beacon_data_stru *pst_beacon_info = NULL;
    uint8_t *puc_ssid_ie = NULL;
    int32_t l_ret;
    uint8_t uc_ssid_len;
    uint8_t uc_vap_id;
    uint8_t auc_ssid_ie[NUM_32_BYTES];
    int32_t l_ssid_len;
    oal_netdev_priv_stru *pst_netdev_priv = NULL;
    wlan_auth_alg_mode_enum_uint8 en_auth_algs;
    int32_t l_memcpy_ret;

    oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_start_ap::enter here.}");

    /* 参数合法性检查 */
    if (oal_any_null_ptr3(pst_wiphy, pst_netdev, pst_ap_settings)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_start_ap::pst_wiphy or pst_netdev or pst_ap_settings is null!}");
        return -OAL_EINVAL;
    }

    /* 获取vap id */
    pst_mac_vap = oal_net_dev_priv(pst_netdev);
    if (pst_mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_start_ap::mac_vap is null}");
        return -OAL_EINVAL;
    }

    uc_vap_id = pst_mac_vap->uc_vap_id;

    /*****************************************************************************
        1.设置信道
    *****************************************************************************/
    l_ret = wal_cfg80211_set_channel_info(pst_wiphy, pst_netdev);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::failed to set channel, return err:%d}", l_ret);
        return -OAL_EFAIL;
    }

    /*****************************************************************************
        2.1 设置ssid信息
    *****************************************************************************/
    l_ssid_len = pst_ap_settings->ssid_len;
    if ((l_ssid_len > 32) || (l_ssid_len <= 0)) {    /* 32 ssid字符串最大长度 */
        oam_warning_log1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::ssid len error, len[%d].}", l_ssid_len);
        return -OAL_EFAIL;
    }
    memset_s(auc_ssid_ie, sizeof(auc_ssid_ie), 0, sizeof(auc_ssid_ie));
    l_memcpy_ret = memcpy_s(auc_ssid_ie, sizeof(auc_ssid_ie), pst_ap_settings->ssid, (uint32_t)l_ssid_len);
    puc_ssid_ie = auc_ssid_ie;
    uc_ssid_len = (uint8_t)l_ssid_len;

    if (uc_ssid_len != 0) {
        l_ret = wal_cfg80211_set_ssid(pst_netdev, puc_ssid_ie, uc_ssid_len);
        if (l_ret != OAL_SUCC) {
            oam_warning_log1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::fail send ssid cfg msg, err[%d]}", l_ret);
            return -OAL_EFAIL;
        }
    }

    /*****************************************************************************
        2.2 设置auth mode信息
    *****************************************************************************/
    en_auth_algs = pst_ap_settings->auth_type;

    l_ret = wal_cfg80211_set_auth_mode(pst_netdev, en_auth_algs);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(uc_vap_id, OAM_SF_ANY,
            "{wal_cfg80211_start_ap::fail to send auth_tpye cfg msg, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    /*****************************************************************************
        3.设置beacon时间间隔、tim period以及安全配置消息等
    *****************************************************************************/
    /* 初始化beacon interval 和DTIM_PERIOD 参数 */
    memset_s(&st_beacon_param, sizeof(mac_beacon_param_stru), 0, sizeof(mac_beacon_param_stru));
    st_beacon_param.l_interval = pst_ap_settings->beacon_interval;
    st_beacon_param.l_dtim_period = pst_ap_settings->dtim_period;
    st_beacon_param.uc_hidden_ssid = (pst_ap_settings->hidden_ssid == 1);

    oam_warning_log3(0, OAM_SF_ANY,
        "{wal_cfg80211_fill_beacon_param::beacon_interval=%d, dtim_period=%d, hidden_ssid=%d!}",
        pst_ap_settings->beacon_interval, pst_ap_settings->dtim_period, pst_ap_settings->hidden_ssid);
    pst_beacon_info = &(pst_ap_settings->beacon);
    l_ret = wal_cfg80211_fill_beacon_param(pst_netdev, pst_beacon_info, &st_beacon_param);
    if (l_ret != OAL_SUCC) {
        oam_error_log1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::failed to fill beacon param, error[%d]}", l_ret);
        return -OAL_EINVAL;
    }

    /* 设置操作类型 */
    st_beacon_param.en_operation_type = MAC_ADD_BEACON;

    /* 填写 msg 消息头 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_CONFIG_BEACON, sizeof(mac_beacon_param_stru));

    /* 填写 msg 消息体 */
    l_memcpy_ret += memcpy_s(st_write_msg.auc_value, sizeof(mac_beacon_param_stru),
                             &st_beacon_param, sizeof(mac_beacon_param_stru));
    if (l_memcpy_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_cfg80211_start_ap::memcpy fail!");
        return -OAL_EFAIL;
    }

    /* 发送消息 */
    l_ret = wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_beacon_param_stru),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::fail to start addset beacon, err[%d]}", l_ret);
        return -OAL_EFAIL;
    }
    /*****************************************************************************
        4.启动ap
    *****************************************************************************/
    l_ret = wal_start_vap(pst_netdev);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_start_ap::failed to start ap, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    pst_netdev_priv = (oal_netdev_priv_stru *)oal_net_dev_wireless_priv(pst_netdev);
    if ((pst_netdev_priv->uc_napi_enable) && (!pst_netdev_priv->uc_state)) {
        oal_napi_enable(&pst_netdev_priv->st_napi);
        pst_netdev_priv->uc_state = 1;
    }

    return OAL_SUCC;
}


OAL_STATIC int32_t wal_cfg80211_stop_ap(oal_wiphy_stru *pst_wiphy,
                                        oal_net_device_stru *pst_netdev)
{
    wal_msg_write_stru st_write_msg;
    mac_vap_stru *pst_mac_vap = NULL;
    int32_t l_ret;
    uint8_t uc_vap_id;
    oal_netdev_priv_stru *pst_netdev_priv = NULL;

    wlan_p2p_mode_enum_uint8 en_p2p_mode;
    oal_wireless_dev_stru *pst_wdev = NULL;
    /* 参数合法性检查 */
    if (oal_any_null_ptr2(pst_wiphy, pst_netdev)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap::wiphy or netdev!}");
        return -OAL_EINVAL;
    }

    if (wal_cfg80211_dfr_and_s3s4_param_check() == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap:: dfr or s3s4 is processing!}");
        return OAL_SUCC;
    }

    /* 获取vap id */
    pst_mac_vap = oal_net_dev_priv(pst_netdev);
    if (pst_mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap::can't get mac vap from netdevice priv data!}");
        return -OAL_EINVAL;
    }

    uc_vap_id = pst_mac_vap->uc_vap_id;

    /* 判断是否为非ap模式 */
    if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_error_log0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_stop_ap::vap is not in ap mode!}");
        return -OAL_EINVAL;
    }

    /* 如果netdev不是running状态，则不需要down */
    if ((oal_netdevice_flags(pst_netdev) & OAL_IFF_RUNNING) == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap::vap is already down!}\r\n");
        return OAL_SUCC;
    }

    /*****************************************************************************
        发送消息，停用ap
    *****************************************************************************/
    /* 填写消息 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DOWN_VAP, sizeof(mac_cfg_start_vap_param_stru));

    pst_wdev = pst_netdev->ieee80211_ptr;
    en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode(pst_wdev->iftype);
    if (en_p2p_mode == WLAN_P2P_BUTT) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap::wal_wireless_iftype_to_mac_p2p_mode return BUFF}");
        return -OAL_EINVAL;
    }
    oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_stop_ap::en_p2p_mode=%u}\r\n", en_p2p_mode);

    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = en_p2p_mode;

    ((mac_cfg_start_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_netdev;

    /* 发送消息 */
    l_ret = wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_start_vap_param_stru),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_stop_ap::failed to stop ap, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    pst_netdev_priv = (oal_netdev_priv_stru *)oal_net_dev_wireless_priv(pst_netdev);
    if (pst_netdev_priv->uc_napi_enable) {
#ifndef WIN32
        oal_netbuf_list_purge(&pst_netdev_priv->st_rx_netbuf_queue);
#endif
        oal_napi_disable(&pst_netdev_priv->st_napi);
        pst_netdev_priv->uc_state = 0;
    }

    return OAL_SUCC;
}


OAL_STATIC int32_t wal_cfg80211_change_bss(oal_wiphy_stru *pst_wiphy,
                                           oal_net_device_stru *pst_netdev,
                                           oal_bss_parameters *pst_bss_params)
{
    return OAL_SUCC;
}


OAL_STATIC void wal_cfg80211_print_sched_scan_req_info(oal_cfg80211_sched_scan_request_stru *pst_request)
{
    int8_t ac_tmp_buff[200]; /* 200字节的buff */
    int32_t l_loop;
    int32_t l_ret;

    /* 打印基本参数 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
    oam_warning_log4(0, OAM_SF_SCAN,
        "wal_cfg80211_print_sched_scan_req_info::channels[%d],interval[%d]ms,flags[%d],rssi_thold[%d]",
        pst_request->n_channels, pst_request->scan_plans[0].interval,
        pst_request->flags,
        pst_request->min_rssi_thold);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    oam_warning_log4(0, OAM_SF_SCAN,
        "wal_cfg80211_print_sched_scan_req_info::channels[%d],interval[%d]ms,flags[%d],rssi_thold[%d]",
        pst_request->n_channels, pst_request->interval,
        pst_request->flags,
        pst_request->min_rssi_thold);
#else
    oam_warning_log4(0, OAM_SF_SCAN,
        "wal_cfg80211_print_sched_scan_req_info::channels[%d],interval[%d]ms,flags[%d],rssi_thold[%d]",
        pst_request->n_channels, pst_request->interval,
        pst_request->flags,
        pst_request->rssi_thold);
#endif

    /* 打印ssid集合的信息 */
    for (l_loop = 0; l_loop < pst_request->n_match_sets; l_loop++) {
        memset_s(ac_tmp_buff, sizeof(ac_tmp_buff), 0, sizeof(ac_tmp_buff));
        l_ret = snprintf_s(ac_tmp_buff, sizeof(ac_tmp_buff), sizeof(ac_tmp_buff) - 1,
                           "mactch_sets[%d] info, ssid_len[%d], ssid: %.32s.\n",
                           l_loop, pst_request->match_sets[l_loop].ssid.ssid_len,
                           pst_request->match_sets[l_loop].ssid.ssid);
        if (l_ret < 0) {
            oam_error_log0(0, OAM_SF_SCAN, "wal_cfg80211_print_sched_scan_req_info::snprintf_s error!");
            return;
        }

        oam_print(ac_tmp_buff);
    }

    for (l_loop = 0; l_loop < pst_request->n_ssids; l_loop++) {
        memset_s(ac_tmp_buff, sizeof(ac_tmp_buff), 0, sizeof(ac_tmp_buff));
        l_ret = snprintf_s(ac_tmp_buff, sizeof(ac_tmp_buff), sizeof(ac_tmp_buff) - 1,
                           "ssids[%d] info, ssid_len[%d], ssid: %.32s.\n",
                           l_loop, pst_request->ssids[l_loop].ssid_len, pst_request->ssids[l_loop].ssid);
        if (l_ret < 0) {
            oam_error_log0(0, OAM_SF_SCAN, "wal_cfg80211_print_sched_scan_req_info::snprintf_s error!");
            return;
        }

        oam_print(ac_tmp_buff);
    }

    return;
}


OAL_STATIC oal_bool_enum_uint8 wal_pno_scan_with_assigned_ssid(oal_cfg80211_ssid_stru *pst_ssid,
                                                               oal_cfg80211_ssid_stru *pst_ssid_list,
                                                               int32_t l_count)
{
    int32_t l_loop;

    if (oal_any_null_ptr2(pst_ssid, pst_ssid_list)) {
        return OAL_FALSE;
    }

    for (l_loop = 0; l_loop < l_count; l_loop++) {
        if ((pst_ssid->ssid_len == pst_ssid_list[l_loop].ssid_len) &&
            (0 == oal_memcmp(pst_ssid->ssid, pst_ssid_list[l_loop].ssid, pst_ssid->ssid_len))) {
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}


OAL_STATIC int32_t wal_cfg80211_sched_scan_start(oal_wiphy_stru *pst_wiphy,
                                                 oal_net_device_stru *pst_netdev,
                                                 oal_cfg80211_sched_scan_request_stru *pst_request)
{
    hmac_device_stru *pst_hmac_device = NULL;
    hmac_scan_stru *pst_scan_mgmt = NULL;
    mac_vap_stru *pst_mac_vap = NULL;
    oal_cfg80211_ssid_stru *pst_ssid_tmp = NULL;
    oal_cfg80211_ssid_stru *pst_scan_ssid_list = NULL;
    mac_pno_scan_stru st_pno_scan_info;
    int32_t l_loop;
    uint32_t ret;
    int32_t l_ret;

    /* 参数合法性检查 */
    if (oal_any_null_ptr3(pst_wiphy, pst_netdev, pst_request)) {
        oam_error_log0(0, 0, "{wal_cfg80211_sched_scan_start::input param null!}");
        return -OAL_EINVAL;
    }

    if (wal_cfg80211_dfr_and_s3s4_param_check() == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_sched_scan_start:: dfr or s3s4 is processing!}");
        return -OAL_EFAIL;
    }

    /* 通过net_device 找到对应的mac_device_stru 结构 */
    pst_mac_vap = oal_net_dev_priv(pst_netdev);
    if (pst_mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_sched_scan_start:: pst_mac_vap is null!}");
        return -OAL_EINVAL;
    }

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_sched_scan_start:: pst_mac_device is null!}");
        return -OAL_EINVAL;
    }

    pst_scan_mgmt = &(pst_hmac_device->st_scan_mgmt);

    /* 如果当前设备处于扫描状态，abort当前扫描 */
    if (pst_scan_mgmt->pst_request != NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_sched_scan_start:: device is busy, stop current scan!}");

        wal_force_scan_abort_then_scan_comp(pst_netdev);
    }

    /* 检测内核下发的需要匹配的ssid集合的个数是否合法 */
    if (pst_request->n_match_sets <= 0) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{wal_cfg80211_sched_scan_start::match_sets = %d!}",
            pst_request->n_match_sets);
        return -OAL_EINVAL;
    }

    /* 初始化pno扫描的结构体信息 */
    memset_s(&st_pno_scan_info, sizeof(st_pno_scan_info), 0, sizeof(st_pno_scan_info));

    /* 将内核下发的匹配的ssid集合复制到本地 */
    pst_scan_ssid_list = NULL;
    if (pst_request->n_ssids > 0) {
        pst_scan_ssid_list = pst_request->ssids;
    }
    for (l_loop = 0; l_loop < oal_min(pst_request->n_match_sets, MAX_PNO_SSID_COUNT); l_loop++) {
        pst_ssid_tmp = &(pst_request->match_sets[l_loop].ssid);
        if (pst_ssid_tmp->ssid_len >= WLAN_SSID_MAX_LEN) {
            oam_warning_log1(0, 0, "{wal_cfg80211_sched_scan_start:: wrong ssid_len[%d]!}", pst_ssid_tmp->ssid_len);
            continue;
        }
        l_ret = memcpy_s(st_pno_scan_info.ast_match_ssid_set[l_loop].auc_ssid, WLAN_SSID_MAX_LEN,
                         pst_ssid_tmp->ssid, pst_ssid_tmp->ssid_len);
        if (l_ret != EOK) {
            oam_error_log0(0, OAM_SF_SCAN, "wal_cfg80211_sched_scan_start::memcpy fail!");
        }

        st_pno_scan_info.ast_match_ssid_set[l_loop].auc_ssid[pst_ssid_tmp->ssid_len] = '\0';
        st_pno_scan_info.ast_match_ssid_set[l_loop].en_scan_ssid =
            wal_pno_scan_with_assigned_ssid(pst_ssid_tmp, pst_scan_ssid_list, pst_request->n_ssids);
        st_pno_scan_info.l_ssid_count++;
    }

    /* 其它参数赋值 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    st_pno_scan_info.l_rssi_thold = pst_request->min_rssi_thold;
#else
    st_pno_scan_info.l_rssi_thold = pst_request->rssi_thold;
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)) */
    st_pno_scan_info.pno_scan_interval = PNO_SCHED_SCAN_INTERVAL; /* 驱动自己定义为30s */
    st_pno_scan_info.uc_pno_scan_repeat = MAX_PNO_REPEAT_TIMES;

    /* 保存当前的PNO调度扫描请求指针 */
    pst_scan_mgmt->pst_sched_scan_req = pst_request;
    pst_scan_mgmt->en_sched_scan_complete = OAL_FALSE;

    /* 维测打印上层下发的调度扫描请求参数信息 */
    wal_cfg80211_print_sched_scan_req_info(pst_request);

    /* 下发pno扫描请求到hmac */
    ret = wal_cfg80211_start_sched_scan(pst_netdev, &st_pno_scan_info);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, 0, "{wal_cfg80211_sched_scan_start::wal_cfg80211_start_sched_scan fail[%d]!}", ret);
        return -OAL_EBUSY;
    }

    return OAL_SUCC;
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
OAL_STATIC int32_t wal_cfg80211_sched_scan_stop(oal_wiphy_stru *pst_wiphy,
                                                oal_net_device_stru *pst_netdev,
                                                uint64_t reqid)

#else
OAL_STATIC int32_t wal_cfg80211_sched_scan_stop(oal_wiphy_stru *pst_wiphy,
                                                oal_net_device_stru *pst_netdev)
#endif
{
    /* 参数合法性检查 */
    if (oal_any_null_ptr2(pst_wiphy, pst_netdev)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_sched_scan_stop::input param pointer is null!}");
        return -OAL_EINVAL;
    }

    return wal_stop_sched_scan(pst_netdev);
}

static int32_t wal_cfg80211_change_virtual_intf_iftype_sta_handle(oal_net_device_stru *net_dev,
    enum nl80211_iftype iftype)
{
    if (net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_AP) {
        /* 下电APUT，切换netdev状态为station */
        return wal_netdev_stop_ap(net_dev);
    }

    // P2P_DEVICE 提前创建，不需要通过wpa_supplicant 创建
    net_dev->ieee80211_ptr->iftype = iftype;
    oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf::change to station}\r\n");
    return OAL_SUCC;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
int32_t wal_cfg80211_change_virtual_intf(oal_wiphy_stru *pst_wiphy,
                                         oal_net_device_stru *pst_net_dev,
                                         enum nl80211_iftype en_type,
                                         oal_vif_params_stru *pst_params)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
int32_t wal_cfg80211_change_virtual_intf(oal_wiphy_stru *pst_wiphy,
                                         oal_net_device_stru *pst_net_dev,
                                         enum nl80211_iftype en_type,
                                         uint32_t *pul_flags,
                                         oal_vif_params_stru *pst_params)
#else
int32_t wal_cfg80211_change_virtual_intf(oal_wiphy_stru *pst_wiphy,
                                         int32_t l_ifindex,
                                         enum nl80211_iftype en_type,
                                         uint32_t *pul_flags,
                                         oal_vif_params_stru *pst_params)
#endif
{
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
    wlan_vap_mode_enum_uint8 en_vap_mode;
    mac_cfg_del_vap_param_stru st_del_vap_param;
    mac_cfg_add_vap_param_stru st_add_vap_param;
    mac_vap_stru *pst_mac_vap = NULL;
    mac_device_stru *pst_mac_device = NULL;
    int8_t ac_p2p_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    /* 1.1 入参检查 */
    if (pst_net_dev == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf::pst_dev is null!}\r\n");
        return -OAL_EINVAL;
    }

    if (wal_cfg80211_dfr_and_s3s4_param_check() == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf:: dfr or s3s4 is processing!}");
        return OAL_SUCC;
    }

    if (oal_any_null_ptr2(pst_wiphy, pst_params)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf::wiphy or params ptr is null!}");
        return -OAL_EINVAL;
    }

    /* 检查VAP 当前模式和目的模式是否相同，如果相同则直接返回 */
    if (pst_net_dev->ieee80211_ptr->iftype == en_type) {
        oam_warning_log1(0, 0, "{wal_cfg80211_change_virtual_intf::same iftype[%d], do not need change!}", en_type);
        return OAL_SUCC;
    }

    oam_warning_log2(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf::[%d][%d]}\r\n",
                     (pst_net_dev->ieee80211_ptr->iftype), en_type);

    oal_io_print("wal_cfg80211_change_virtual_intf,dev_name is:%s\n", pst_net_dev->name);

    switch (en_type) {
        case NL80211_IFTYPE_MONITOR:
        case NL80211_IFTYPE_WDS:
        case NL80211_IFTYPE_MESH_POINT:
        case NL80211_IFTYPE_ADHOC:
            oam_error_log1(0, 0, "{wal_cfg80211_change_virtual_intf::currently do not sup this type[%d]}", en_type);
            return -OAL_EINVAL;

        case NL80211_IFTYPE_STATION:
            return wal_cfg80211_change_virtual_intf_iftype_sta_handle(pst_net_dev, en_type);
        case NL80211_IFTYPE_P2P_CLIENT:
            en_vap_mode = WLAN_VAP_MODE_BSS_STA;
            en_p2p_mode = WLAN_P2P_CL_MODE;
            break;
        case NL80211_IFTYPE_AP:
        case NL80211_IFTYPE_AP_VLAN:
            en_vap_mode = WLAN_VAP_MODE_BSS_AP;
            en_p2p_mode = WLAN_LEGACY_VAP_MODE;
            break;
        case NL80211_IFTYPE_P2P_GO:
            en_vap_mode = WLAN_VAP_MODE_BSS_AP;
            en_p2p_mode = WLAN_P2P_GO_MODE;
            break;
        default:
            oam_error_log1(0, 0, "{wal_cfg80211_change_virtual_intf::currently do not sup this type[%d]}", en_type);
            return -OAL_EINVAL;
    }

    if (en_type == NL80211_IFTYPE_AP) {
        return wal_setup_ap(pst_net_dev);
    }

    /* 设备为P2P 设备才需要进行change virtual interface */
    pst_mac_vap = oal_net_dev_priv(pst_net_dev);
    if (pst_mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf::can't get mac vap from netdevice priv}");
        return -OAL_EINVAL;
    }

    if (IS_LEGACY_VAP(pst_mac_vap)) {
        pst_net_dev->ieee80211_ptr->iftype = en_type;
        return OAL_SUCC;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log1(0, OAM_SF_CFG,
            "{wal_cfg80211_change_virtual_intf::mac_res_get_dev fail dev_id[%d].}", pst_mac_vap->uc_device_id);
        return -OAL_EINVAL;
    }

    /* 通过device id获取netdev名字 */
    snprintf_s(ac_p2p_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, MAC_NET_DEVICE_NAME_LENGTH - 1,
               "p2p%d", pst_mac_device->uc_device_id);

    if (0 == (oal_strcmp(ac_p2p_netdev_name, pst_net_dev->name))) {
        /* 解决异常情况下,wpa_supplicant下发p2p0设备切换到p2p go/cli模式导致fastboot的问题 */
        oam_warning_log0(0, 0, "{wal_cfg80211_change_virtual_intf::p2p0 netdevice can not change to P2P CLI/GO.}");
        return -OAL_EINVAL;
    }

    /* 如果当前模式和目的模式不同，则需要:
       1. 停止 VAP
       2. 删除 VAP
       3. 重新创建对应模式VAP
       4. 启动VAP
    */
    /* 停止VAP */
    wal_netdev_stop(pst_net_dev);

    memset_s(&st_del_vap_param, sizeof(st_del_vap_param), 0, sizeof(st_del_vap_param));
    /* 删除VAP */
    st_del_vap_param.pst_net_dev = pst_net_dev;
    /* 设备p2p 模式需要从net_device 中获取 */
    st_del_vap_param.en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode(pst_net_dev->ieee80211_ptr->iftype);
    if (wal_cfg80211_del_vap(&st_del_vap_param)) {
        return -OAL_EFAIL;
    }

    memset_s(&st_add_vap_param, sizeof(st_add_vap_param), 0, sizeof(st_add_vap_param));
    /* 重新创建对应模式VAP */
    st_add_vap_param.pst_net_dev = pst_net_dev;
    st_add_vap_param.en_vap_mode = en_vap_mode;
    st_add_vap_param.en_p2p_mode = en_p2p_mode;

    if (wal_cfg80211_add_vap(&st_add_vap_param) != OAL_SUCC) {
        oam_error_log0(0, 0, "{wal_cfg80211_change_virtual_intf::wal_cfg80211_add_vap fail.}");
        return -OAL_EFAIL;
    }

    /* 启动VAP */
    wal_netdev_open(pst_net_dev, OAL_FALSE);

    pst_net_dev->ieee80211_ptr->iftype = en_type;
    return OAL_SUCC;
}


OAL_STATIC int32_t wal_cfg80211_add_station(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_dev,
    const uint8_t *puc_mac,
    oal_station_parameters_stru *pst_sta_parms)
{
    return OAL_SUCC;
}


OAL_STATIC int32_t wal_cfg80211_del_station(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_dev,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    struct station_del_parameters *params
#else
    uint8_t *puc_mac
#endif
    )
{
    mac_vap_stru *pst_mac_vap = NULL;
    mac_cfg_kick_user_param_stru st_kick_user_param;
    int32_t int_user_count_ok = 0;
    int32_t int_user_count_fail = 0;
    int32_t uint_ret = OAL_FAIL;
    uint8_t auc_mac_boardcast[OAL_MAC_ADDR_LEN];
    uint16_t us_reason_code = MAC_INACTIVITY;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    uint8_t *puc_mac;
    if (params == NULL) {
        return -OAL_EFAUL;
    }
    puc_mac = (uint8_t *)params->mac;

    us_reason_code = params->reason_code;
#endif

    if (oal_any_null_ptr2(pst_wiphy, pst_dev)) {
        return -OAL_EFAUL;
    }
#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_del_station::dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return OAL_SUCC;
    }

#endif  // #ifdef _PRE_WLAN_FEATURE_DFR

    pst_mac_vap = oal_net_dev_priv(pst_dev);
    if (pst_mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_station::can't get mac vap from netdevice priv data!}\r\n");
        return -OAL_EFAUL;
    }

    /* 判断是否是AP模式 */
    if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_error_log1(pst_mac_vap->uc_vap_id, 0, "{wal_cfg80211_del_station::vap_mode:%d!}", pst_mac_vap->en_vap_mode);
        return -OAL_EINVAL;
    }

    if (puc_mac == NULL) {
        memset_s(auc_mac_boardcast, OAL_MAC_ADDR_LEN, 0xff, OAL_MAC_ADDR_LEN);
        puc_mac = auc_mac_boardcast;
    }
    st_kick_user_param.us_reason_code = us_reason_code;
    if (EOK != memcpy_s(st_kick_user_param.auc_mac_addr, WLAN_MAC_ADDR_LEN, puc_mac, OAL_MAC_ADDR_LEN)) {
        oam_error_log0(0, OAM_SF_ANY, "wal_cfg80211_del_station::memcpy fail!");
    }
    uint_ret = wal_cfg80211_start_disconnect(pst_dev, &st_kick_user_param);
    if (uint_ret != OAL_SUCC) {
        /* 由于删除的时候可能用户已经删除，此时再进行用户查找，会返回错误，输出ERROR打印，修改为warning */
        oam_warning_log1(pst_mac_vap->uc_vap_id, 0, "{wal_cfg80211_del_station::config_kick_user fail[%d]!}", uint_ret);
        int_user_count_fail++;
    } else {
        int_user_count_ok++;
    }

    if (int_user_count_fail > 0) {
        return -OAL_EINVAL;
    }

    return OAL_SUCC;
}


OAL_STATIC int32_t wal_cfg80211_change_station(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_dev,
    const uint8_t *puc_mac,
    oal_station_parameters_stru *pst_sta_parms)
{
    return OAL_SUCC;
}


OAL_STATIC void wal_fill_station_rate_info(oal_station_info_stru *sta_info,
    oal_station_info_stru *stats)
{
    /* 11ac模式下 由于私有速率的存在内核无法识别 修改为通过legacy上报 更改相应的标志位 */
    if ((stats->txrate.flags & RATE_INFO_FLAGS_VHT_MCS) || (stats->txrate.flags & MAC_RATE_INFO_FLAGS_HE_MCS)) {
        sta_info->txrate.legacy = stats->txrate.legacy;      /* dmac已将单位修改为100kbps */
        /* 清除标志位RATE_INFO_FLAGS_VHT_MCS RATE_INFO_FLAGS_MCS */
        sta_info->txrate.flags = stats->txrate.flags & 0x7C;
    } else {
        sta_info->txrate.legacy = (uint16_t)(stats->txrate.legacy * 10); /* 内核中单位为100kbps */
        sta_info->txrate.flags  = stats->txrate.flags;
    }

    sta_info->txrate.mcs = stats->txrate.mcs;
    sta_info->txrate.nss = stats->txrate.nss;
    sta_info->txrate.bw  = stats->txrate.bw;

    /* 上报接收速率 */
    sta_info->filled |= BIT(NL80211_STA_INFO_RX_BITRATE);
    sta_info->rxrate.legacy = stats->rxrate.legacy;
}

#define QUERY_STATION_INFO_TIME (5 * OAL_TIME_HZ)

OAL_STATIC void wal_cfg80211_fill_station_info(oal_station_info_stru *pst_sta_info,
                                               oal_station_info_stru *pst_stats)
{
    /* 适配linux 4.0.修改为BIT(NL80211_STA_INFO_SIGNAL)等. */
    pst_sta_info->filled |= BIT(NL80211_STA_INFO_SIGNAL);
    pst_sta_info->signal = pst_stats->signal;

    pst_sta_info->filled |= BIT(NL80211_STA_INFO_RX_PACKETS);
    pst_sta_info->filled |= BIT(NL80211_STA_INFO_TX_PACKETS);
    pst_sta_info->rx_packets = pst_stats->rx_packets;
    pst_sta_info->tx_packets = pst_stats->tx_packets;

    pst_sta_info->filled |= BIT(NL80211_STA_INFO_RX_BYTES);
    pst_sta_info->filled |= BIT(NL80211_STA_INFO_TX_BYTES);
    pst_sta_info->rx_bytes = pst_stats->rx_bytes;
    pst_sta_info->tx_bytes = pst_stats->tx_bytes;

    pst_sta_info->filled |= BIT(NL80211_STA_INFO_TX_RETRIES);
    pst_sta_info->filled |= BIT(NL80211_STA_INFO_TX_FAILED);
    pst_sta_info->filled |= BIT(NL80211_STA_INFO_RX_DROP_MISC);
    pst_sta_info->tx_retries = pst_stats->tx_retries;
    pst_sta_info->tx_failed = pst_stats->tx_failed;
    pst_sta_info->rx_dropped_misc = pst_stats->rx_dropped_misc;

    pst_sta_info->filled |= BIT(NL80211_STA_INFO_TX_BITRATE);

    wal_fill_station_rate_info(pst_sta_info, pst_stats);
}

OAL_STATIC void wal_cfg80211_fill_station_info_ext(oal_station_info_stru *sta_info,
    hmac_vap_stru *hmac_vap)
{
#ifdef CONFIG_HW_GET_EXT_SIG
    sta_info->filled |= BIT(NL80211_STA_INFO_NOISE);
    sta_info->noise = hmac_vap->station_info.noise;

    sta_info->filled |= BIT(NL80211_STA_INFO_SNR);
    sta_info->snr = oal_max(hmac_vap->st_station_info_extend.c_snr_ant0,
        hmac_vap->st_station_info_extend.c_snr_ant1);

    sta_info->filled |= BIT(NL80211_STA_INFO_CNAHLOAD);
    sta_info->chload = hmac_vap->station_info.chload;
#endif

#ifdef CONFIG_HW_GET_EXT_SIG_ULDELAY
    sta_info->filled |= BIT(NL80211_STA_INFO_UL_DELAY);
    sta_info->ul_delay = hmac_vap->station_info.ul_delay;
#endif

    sta_info->filled |= BIT(NL80211_STA_INFO_BEACON_SIGNAL_AVG);
    sta_info->rx_beacon_signal_avg = hmac_vap->beacon_rssi;
}

uint8_t wal_cfg80211_get_station_filter(mac_vap_stru *pst_mac_vap, uint8_t *puc_mac)
{
    hmac_user_stru *pst_hmac_user = NULL;
    uint32_t current_time = (uint32_t)oal_time_get_stamp_ms();
    uint32_t runtime;
    uint32_t get_station_threshold;
    mac_device_stru *pst_mac_dev = NULL;

    pst_hmac_user = mac_vap_get_hmac_user_by_addr(pst_mac_vap, puc_mac);
    if (pst_hmac_user == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_get_station_filter::user is null.}");
        return OAL_FALSE;
    }

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_dev == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_get_station_filter::dev is null.}");
        return OAL_FALSE;
    }
    
    if (IS_LEGACY_STA(pst_mac_vap) &&
        (pst_mac_vap->pst_vowifi_cfg_param != NULL &&
         pst_mac_vap->pst_vowifi_cfg_param->en_vowifi_mode == VOWIFI_DISABLE_REPORT)) { /* 亮屏且vowifi正在使用时 */
        get_station_threshold = WAL_VOWIFI_GET_STATION_THRESHOLD;
    } else {
        if (!IS_LEGACY_VAP(pst_mac_vap)) {
            get_station_threshold = WAL_CAST_SCREEN_GET_STATION_THRESHOLD;
        } else {
            get_station_threshold = WAL_GET_STATION_THRESHOLD;
        }
    }

    runtime = (uint32_t)oal_time_get_runtime(pst_hmac_user->rssi_last_timestamp, current_time);
    if (get_station_threshold > runtime) {
        return OAL_FALSE;
    }

    pst_hmac_user->rssi_last_timestamp = current_time;
    return OAL_TRUE;
}


OAL_STATIC int32_t wal_cfg80211_get_station(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_dev, const uint8_t *puc_mac,
    oal_station_info_stru *pst_sta_info)
{
    mac_vap_stru *pst_mac_vap = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    dmac_query_station_info_request_event *pst_query_station_info = NULL;
    wal_msg_write_stru st_write_msg;
    signed long i_leftime;
    int32_t l_ret;
    uint8_t uc_vap_id;

    if (oal_any_null_ptr4(pst_wiphy, pst_dev, puc_mac, pst_sta_info)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_get_station::input params null!}");
        return -OAL_EINVAL;
    }

    pst_mac_vap = oal_net_dev_priv(pst_dev);
    if (pst_mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_get_station::oal_net_dev_priv, return null!}");
        return -OAL_EINVAL;
    }

    uc_vap_id = pst_mac_vap->uc_vap_id;
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_get_station::mac_res_get_hmac_vap fail.}");
        return -OAL_EINVAL;
    }

    /* 固定时间内最多更新一次RSSI */
    if (OAL_FALSE == wal_cfg80211_get_station_filter(&pst_hmac_vap->st_vap_base_info, (uint8_t *)puc_mac)) {
        wal_cfg80211_fill_station_info(pst_sta_info, &pst_hmac_vap->station_info);
        wal_cfg80211_fill_station_info_ext(pst_sta_info, pst_hmac_vap);
        return OAL_SUCC;
    }

    pst_query_station_info = (dmac_query_station_info_request_event *)st_write_msg.auc_value;
    pst_query_station_info->query_event = OAL_QUERY_STATION_INFO_EVENT;
    oal_set_mac_addr(pst_query_station_info->auc_query_sta_addr, (uint8_t *)puc_mac);

    pst_hmac_vap->station_info_query_completed_flag = OAL_FALSE;

    /********************************************************************************
        抛事件到wal层处理 ，对于低功耗需要做额外处理，不能像下层抛事件，直接起定时器
        低功耗会在接收beacon帧的时候主动上报信息。
    ********************************************************************************/
    /* 3.1 填写 msg 消息头 */
    st_write_msg.en_wid = WLAN_CFGID_QUERY_STATION_STATS;
    st_write_msg.us_len = sizeof(dmac_query_station_info_request_event);

    /* 3.2 填写 msg 消息体 */
    if (EOK != memcpy_s(st_write_msg.auc_value, sizeof(dmac_query_station_info_request_event),
                        pst_query_station_info, sizeof(dmac_query_station_info_request_event))) {
        oam_error_log0(0, OAM_SF_ANY, "wal_cfg80211_get_station::memcpy fail!");
        return -OAL_EFAIL;
    }

    l_ret = wal_send_cfg_event(pst_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(dmac_query_station_info_request_event),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(uc_vap_id, 0, "{wal_cfg80211_get_station::wal_send_cfg_event return err code %d!}", l_ret);
        return -OAL_EFAIL;
    }
    /*lint -e730*/ /* info, boolean argument to function */
    i_leftime = oal_wait_event_interruptible_timeout_m(pst_hmac_vap->query_wait_q,
        (OAL_TRUE == pst_hmac_vap->station_info_query_completed_flag), QUERY_STATION_INFO_TIME);
    /*lint +e730*/
    if (i_leftime == 0) {
        /* 超时还没有上报扫描结束 */
        oam_warning_log1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_get_station::query info wait for %ld ms timeout!}",
                         ((QUERY_STATION_INFO_TIME * HMAC_S_TO_MS) / OAL_TIME_HZ));
        return -OAL_EINVAL;
    } else if (i_leftime < 0) {
        /* 定时器内部错误 */
        oam_warning_log1(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_get_station::query info wait for %ld ms error!}",
                         ((QUERY_STATION_INFO_TIME * HMAC_S_TO_MS) / OAL_TIME_HZ));
        return -OAL_EINVAL;
    } else {
        /* 正常结束  */
        wal_cfg80211_fill_station_info(pst_sta_info, &pst_hmac_vap->station_info);
        wal_cfg80211_fill_station_info_ext(pst_sta_info, pst_hmac_vap);
        return OAL_SUCC;
    }
}


OAL_STATIC int32_t wal_cfg80211_dump_station(oal_wiphy_stru *pst_wiphy,
                                             oal_net_device_stru *pst_dev,
                                             int32_t int_index,
                                             uint8_t *puc_mac,
                                             oal_station_info_stru *pst_sta_info)
{
    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_is_p2p_group_exist(mac_device_stru *pst_mac_device)
{
    if (OAL_SUCC != hmac_check_p2p_vap_num(pst_mac_device, WLAN_P2P_GO_MODE) ||
        OAL_SUCC != hmac_check_p2p_vap_num(pst_mac_device, WLAN_P2P_CL_MODE)) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}


OAL_STATIC void wal_set_p2p_group_del_status(hmac_vap_stru *pst_hmac_vap,
    oal_net_device_stru *pst_net_dev)
{
    hmac_device_stru *pst_hmac_device;

    pst_hmac_device = hmac_res_get_mac_dev(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (oal_unlikely(pst_hmac_device == NULL)) {
        oam_error_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
            "{wal_set_p2p_group_del_status::hmac_device is null, device id[%d]}\r\n",
            pst_hmac_vap->st_vap_base_info.uc_device_id);
        return;
    }

    oam_warning_log3(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
        "{wal_set_p2p_group_del_status:: push deleting inf task to work queue, device id[%d], \
        vap_mode[%d], p2p_mode[%d]}\r\n",
        pst_hmac_vap->st_vap_base_info.uc_device_id, pst_hmac_vap->st_vap_base_info.en_vap_mode,
        pst_hmac_vap->st_vap_base_info.en_p2p_mode);

    hmac_set_p2p_status(&pst_hmac_device->p2p_intf_status, P2P_STATUS_IF_DELETING);
}


uint32_t wal_del_p2p_group(mac_device_stru *pst_mac_device)
{
    uint8_t uc_vap_idx;
    mac_vap_stru *pst_mac_vap = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    oal_net_device_stru *pst_net_dev = NULL;
    mac_cfg_del_vap_param_stru st_del_vap_param;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (oal_unlikely(pst_mac_vap == NULL)) {
            oam_warning_log1(0, OAM_SF_P2P, "{wal_del_p2p_group::get mac vap resource fail! vap id is %d}",
                             pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (oal_unlikely(pst_hmac_vap == NULL)) {
            oam_warning_log1(0, OAM_SF_P2P, "{wal_del_p2p_group::get hmac vap resource fail! vap id is %d}",
                             pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        pst_net_dev = pst_hmac_vap->pst_net_device;
        if (oal_unlikely(pst_net_dev == NULL)) {
            oam_warning_log1(0, OAM_SF_P2P, "{wal_del_p2p_group::get net device fail! vap id is %d}",
                             pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if (IS_P2P_GO(pst_mac_vap) || IS_P2P_CL(pst_mac_vap)) {
            memset_s(&st_del_vap_param, sizeof(st_del_vap_param), 0, sizeof(st_del_vap_param));
            oal_io_print("wal_del_p2p_group:: ifname %s\r\n", pst_net_dev->name);
            st_del_vap_param.pst_net_dev = pst_net_dev;
            st_del_vap_param.en_vap_mode = pst_mac_vap->en_vap_mode;
            st_del_vap_param.en_p2p_mode = mac_get_p2p_mode(pst_mac_vap);
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{wal_del_p2p_group:: vap mode[%d], p2p mode[%d]}",
                             st_del_vap_param.en_vap_mode, st_del_vap_param.en_p2p_mode);
            /* 删除已经存在的P2P group */
            wal_force_scan_complete(pst_net_dev, OAL_TRUE);
            wal_stop_vap(pst_net_dev);

            if (wal_cfg80211_del_vap(&st_del_vap_param) == OAL_SUCC) {
                wal_set_p2p_group_del_status(pst_hmac_vap, pst_net_dev);
                /* 启动linux work queue 删除net_device，不要直接调用unregister_netdev以防wal_netdev_stop死循环 */
                pst_hmac_vap->pst_del_net_device = pst_net_dev;
                oal_workqueue_schedule(&(pst_hmac_vap->st_del_virtual_inf_worker));
            }
        }
    }
    return OAL_SUCC;
}


uint32_t wal_cfg80211_register_netdev(oal_net_device_stru *pst_net_dev)
{
    uint8_t uc_rollback_lock = OAL_FALSE;
    uint32_t ret;
    oal_netdev_priv_stru *pst_netdev_priv = NULL;

    
    if (rtnl_is_locked()) {
        rtnl_unlock();
        uc_rollback_lock = OAL_TRUE;
    }

    pst_netdev_priv = (oal_netdev_priv_stru *)oal_net_dev_wireless_priv(pst_net_dev);
#ifdef _PRE_CONFIG_HISI_110X_NAPI_DISABLE
    pst_netdev_priv->uc_napi_enable     = OAL_FALSE;
    pst_netdev_priv->uc_gro_enable      = OAL_FALSE;
#else
    pst_netdev_priv->uc_napi_enable = OAL_TRUE;
    pst_netdev_priv->uc_gro_enable = OAL_TRUE;
#endif
    pst_netdev_priv->uc_napi_weight = NAPI_POLL_WEIGHT_LEV1;
    pst_netdev_priv->queue_len_max = NAPI_NETDEV_PRIV_QUEUE_LEN_MAX;
    pst_netdev_priv->uc_state = 0;
    pst_netdev_priv->period_pkts = 0;
    pst_netdev_priv->period_start = 0;

    oal_netbuf_list_head_init(&pst_netdev_priv->st_rx_netbuf_queue);
    oal_netif_napi_add(pst_net_dev, &pst_netdev_priv->st_napi, hmac_rxdata_polling, NAPI_POLL_WEIGHT_LEV1);

    /* 内核注册net_device, 只返回0 */
    ret = (uint32_t)hmac_net_register_netdev(pst_net_dev);

    if (uc_rollback_lock) {
        rtnl_lock();
    }
    return ret;
}


void wal_cfg80211_unregister_netdev(oal_net_device_stru *pst_net_dev)
{
    uint8_t uc_rollback_lock = OAL_FALSE;

    if (rtnl_is_locked()) {
        rtnl_unlock();
        uc_rollback_lock = OAL_TRUE;
    }
    /* 去注册netdev */
    oal_net_unregister_netdev(pst_net_dev);

    if (uc_rollback_lock) {
        rtnl_lock();
    }
}


OAL_STATIC void wal_cfg80211_add_p2p_interface_init(oal_net_device_stru *pst_net_dev, mac_device_stru *pst_mac_device)
{
    uint8_t auc_primary_mac_addr[WLAN_MAC_ADDR_LEN] = {0};    /* MAC地址 */

    if ((oal_strncmp("p2p-p2p0", pst_net_dev->name, OAL_STRLEN("p2p-p2p0")) != 0) &&
        (oal_strncmp("p2p-wlan0", pst_net_dev->name, OAL_STRLEN("p2p-wlan0")) != 0)) {
        return;
    }

    if (oal_unlikely(pst_mac_device->st_p2p_info.pst_primary_net_device == NULL)) {
        oal_random_ether_addr(auc_primary_mac_addr);
        auc_primary_mac_addr[MAC_ADDR_0] &= (~0x02);
        auc_primary_mac_addr[MAC_ADDR_1] = 0x11;
        auc_primary_mac_addr[MAC_ADDR_2] = 0x02;
    } else {
        if (oal_unlikely(oal_netdevice_mac_addr(pst_mac_device->st_p2p_info.pst_primary_net_device) == NULL)) {
            oam_error_log0(0, OAM_SF_ANY,
                "{wal_cfg80211_add_p2p_interface_init() pst_primary_net_device; addr is null}\r\n");
            return;
        }

        if (memcpy_s(auc_primary_mac_addr, WLAN_MAC_ADDR_LEN,
            oal_netdevice_mac_addr(pst_mac_device->st_p2p_info.pst_primary_net_device), WLAN_MAC_ADDR_LEN) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_add_p2p_interface_init::memcpy fail}");
            return;
        }
    }

    oal_set_mac_addr((uint8_t *)oal_netdevice_mac_addr(pst_net_dev), auc_primary_mac_addr);
    pst_net_dev->dev_addr[MAC_ADDR_0] |= 0x02;
    pst_net_dev->dev_addr[MAC_ADDR_4] ^= 0x80;
}

OAL_STATIC void wal_cfg80211_add_virtual_intf_netdev_init(oal_net_device_stru *pst_net_dev,
                                                          oal_wireless_dev_stru *pst_wdev)
{
    /* 对新创建的net_device 初始化对应参数 */
#ifdef CONFIG_WIRELESS_EXT
    pst_net_dev->wireless_handlers = &g_st_iw_handler_def;
#endif
    pst_net_dev->netdev_ops = &g_st_wal_net_dev_ops;

    pst_net_dev->ethtool_ops = &g_st_wal_ethtool_ops;

    oal_netdevice_destructor(pst_net_dev) = oal_net_free_netdev;
    /* host发送报文使能csum硬化功能 */
    if (g_trx_switch.tx_switch == HOST_TX) {
        pst_net_dev->features    |= NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;
        pst_net_dev->hw_features |= NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;
    }
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 44))
    oal_netdevice_master(pst_net_dev) = NULL;
#endif

    oal_netdevice_ifalias(pst_net_dev) = NULL;
    oal_netdevice_watchdog_timeo(pst_net_dev) = 5;
    oal_netdevice_wdev(pst_net_dev) = pst_wdev;
    oal_netdevice_qdisc(pst_net_dev, NULL);
}


static uint32_t wal_cfg80211_param_check(oal_wiphy_stru *wiphy, const char *name, oal_vif_params_stru *params)
{
    uint32_t ret = OAL_TRUE;
#ifdef  _PRE_CONFIG_HISI_S3S4_POWER_STATE
    if (g_st_recovery_info.device_s3s4_process_flag) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_add_virtual_intf::s3s4 is processing!}\r\n");
        ret = OAL_FALSE;
    }
#endif

    if (oal_any_null_ptr3(wiphy, name, params)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_add_virtual_intf::wiphy or name or params is null!}");
        ret = OAL_FALSE;
    }

    return ret;
}


OAL_STATIC uint32_t wal_set_nl80211_work_mode(enum nl80211_iftype type, wlan_p2p_mode_enum_uint8 *p2p_mode,
    wlan_vap_mode_enum_uint8 *vap_mode, oal_wireless_dev_stru **wdev, oal_net_device_stru *net_dev)
{
    switch (type) {
        case NL80211_IFTYPE_ADHOC:
        case NL80211_IFTYPE_AP_VLAN:
        case NL80211_IFTYPE_WDS:
        case NL80211_IFTYPE_MESH_POINT:
        case NL80211_IFTYPE_MONITOR:
            oam_error_log1(0, OAM_SF_CFG, "{wal_n80211_set_work_mode::Unsupported interface type[%d]!}", type);
            return OAL_FAIL;

        case NL80211_IFTYPE_P2P_DEVICE:
            *wdev = net_dev->ieee80211_ptr;
            return OAL_TRUE;

        case NL80211_IFTYPE_P2P_CLIENT:
            *vap_mode = WLAN_VAP_MODE_BSS_STA;
            *p2p_mode = WLAN_P2P_CL_MODE;
            break;
        case NL80211_IFTYPE_STATION:
            *vap_mode = WLAN_VAP_MODE_BSS_STA;
            *p2p_mode = WLAN_LEGACY_VAP_MODE;
            break;
        case NL80211_IFTYPE_P2P_GO:
            *vap_mode = WLAN_VAP_MODE_BSS_AP;
            *p2p_mode = WLAN_P2P_GO_MODE;
            break;
        case NL80211_IFTYPE_AP:
            *vap_mode = WLAN_VAP_MODE_BSS_AP;
            *p2p_mode = WLAN_LEGACY_VAP_MODE;
            break;
        default:
            oam_error_log1(0, OAM_SF_CFG, "{wal_n80211_set_work_mode::Unsupported interface type[%d]!}", type);
            return OAL_FAIL;
    }
    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_cfg80211_wait_del(hmac_device_stru *hmac_device)
{
    int32_t timeout;
    uint8_t rollback_lock = 0;
    if (rtnl_is_locked()) {
        rtnl_unlock();
        rollback_lock = OAL_TRUE;
    }
    oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_wait_del:Released the lock and wait till IF_DEL is complete!}");
    timeout = oal_wait_event_interruptible_timeout_m(hmac_device->st_netif_change_event,
        (hmac_get_p2p_status(hmac_device->p2p_intf_status, P2P_STATUS_IF_DELETING) == OAL_FALSE),
        oal_msecs_to_jiffies(WAL_MAX_WAIT_TIME));

    /* put back the rtnl_lock again */
    if (rollback_lock) {
        rtnl_lock();
    }

    if (timeout > 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_wait_del::IF DEL is Success!}\r\n");
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_wait_del::timeount < 0, return -EAGAIN!}\r\n");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_cfg80211_search_dev(oal_net_device_stru **net_dev, const char *name,
    mac_device_stru *mac_device, oal_wireless_dev_stru **wdev)
{
    hmac_device_stru *hmac_device = NULL;
    *net_dev = wal_config_get_netdev(name, OAL_STRLEN(name));
    if (*net_dev != NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_search_dev::the net_device is already exist!}\r\n");
        /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
        oal_dev_put(*net_dev);
        *wdev = (*net_dev)->ieee80211_ptr;
        return OAL_TRUE;
    }

    /* 添加net_device 前先判断当前是否正在删除net_device 状态，如果正在删除net_device，则等待删除完成，再添加 */
    hmac_device = hmac_res_get_mac_dev(mac_device->uc_device_id);
    if (hmac_device == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_search_dev::hmac_device is null!}", mac_device->uc_device_id);
        return OAL_FAIL;
    }

    if (hmac_get_p2p_status(hmac_device->p2p_intf_status, P2P_STATUS_IF_DELETING) == OAL_TRUE) {
        /* 等待删除完成 */
        if (wal_cfg80211_wait_del(hmac_device) != OAL_SUCC) {
            return OAL_FAIL;
        }
    }

    /* 检查wifi 驱动中，P2P group 是否已经创建，如果P2P group 已经创建，则将该P2P group 删除，并且重新创建P2P group */
    if (OAL_TRUE == wal_is_p2p_group_exist(mac_device)) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_search_dev::found exist p2p group, delet it first!}\r\n");
        if (OAL_SUCC != wal_del_p2p_group(mac_device)) {
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

/* 申请内存 */
OAL_STATIC uint32_t wal_cfg80211_mem_prep(const char *name, oal_net_device_stru **net_dev,
    oal_wireless_dev_stru **wdev)
{
    int8_t local_name[OAL_IF_NAME_SIZE] = { 0 };
    /* 申请内存 */
    if (EOK != memcpy_s(local_name, OAL_IF_NAME_SIZE, name, OAL_STRLEN(name))) {
        oam_error_log0(0, OAM_SF_ANY, "wal_cfg80211_mem_prep::memcpy fail!");
    }

     /* 此函数第一个入参代表私有长度，此处不涉及为0 */
    *net_dev = oal_net_alloc_netdev_mqs(sizeof(oal_netdev_priv_stru), local_name, oal_ether_setup,
        WLAN_NET_QUEUE_BUTT, 1);
    if (oal_unlikely(*net_dev == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_mem_prep::net_dev null ptr error!}");
        return OAL_FAIL;
    }

    *wdev = (oal_wireless_dev_stru *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, sizeof(oal_wireless_dev_stru), OAL_FALSE);
    if (oal_unlikely(*wdev == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_mem_prep::alloc mem, wdev null ptr!}");
        /* 异常处理，释放内存 */
        oal_net_free_netdev(*net_dev);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

static uint8_t g_cfg_vap_id = 0;


OAL_STATIC uint32_t wal_cfg80211_get_vap_info(hmac_vap_stru **cfg_hmac_vap)
{
    mac_vap_stru *cfg_mac_vap = NULL;
    uint8_t cfg_vap_id = g_cfg_vap_id;

    cfg_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(cfg_vap_id);
    if (cfg_mac_vap == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_get_vap_info::cfg_mac_vap is null vap_id:%d!}", cfg_vap_id);
        return OAL_FAIL;
    }
    *cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(cfg_vap_id);
    if (*cfg_hmac_vap == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_get_vap_info::cfg_hmac_vap is null vap_id:%d!}", cfg_vap_id);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/* 异常处理 */
OAL_STATIC void wal_cfg80211_exception_handle(oal_wireless_dev_stru *wdev, oal_net_device_stru *net_dev)
{
    /* 先去注册，后释放 */
    wal_cfg80211_unregister_netdev(net_dev);
    oal_mem_free_m(wdev, OAL_FALSE);
}


OAL_STATIC uint32_t wal_cfg80211_infomation_proc_func(oal_net_device_stru *net_dev, wlan_p2p_mode_enum_uint8 p2p_mode,
    wlan_vap_mode_enum_uint8 vap_mode, oal_wireless_dev_stru *wdev, hmac_vap_stru *cfg_hmac_vap)
{
    int32_t ret;
    mac_cfg_add_vap_param_stru *add_vap_param = NULL;
    wal_msg_write_stru write_msg;
    wal_msg_stru *rsp_msg = NULL;
    mac_vap_stru *mac_vap = NULL;
    oal_net_device_stru *cfg_net_dev = cfg_hmac_vap->pst_net_device;
    /* 填写消息 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_ADD_VAP, sizeof(mac_cfg_add_vap_param_stru));
    add_vap_param = (mac_cfg_add_vap_param_stru *)(write_msg.auc_value);
    add_vap_param->pst_net_dev = net_dev;
    add_vap_param->en_vap_mode = vap_mode;
    add_vap_param->uc_cfg_vap_indx = g_cfg_vap_id;
    add_vap_param->en_p2p_mode = p2p_mode;
    add_vap_param->is_primary_vap = 0;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wal_vap_get_wlan_mode_para(&write_msg);
#endif

    /* 发送消息 */
    ret = wal_send_cfg_event(cfg_net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH +
        sizeof(mac_cfg_add_vap_param_stru), (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_infomation_proc_func::wal_send_cfg_event err code:%d!}", ret);
        wal_cfg80211_exception_handle(wdev, net_dev);
        return OAL_FAIL;
    }

    /* 读取返回的错误码 */
    if (wal_check_and_release_msg_resp(rsp_msg) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY,
            "{wal_cfg80211_infomation_proc_func::wal_cfg80211_infomation_proc_func fail:ul_err_code!}");
        wal_cfg80211_exception_handle(wdev, net_dev);
        return OAL_FAIL;
    }

    wal_set_random_mac_to_mib(net_dev); /* set random mac to mib ; for hi1102-cb */

    /* 设置netdevice的MAC地址，MAC地址在HMAC层被初始化到MIB中 */
    mac_vap = oal_net_dev_priv(net_dev);
    if (oal_unlikely(mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ANY,
            "{wal_cfg80211_infomation_proc_func::oal_net_dev_priv(net_dev) is null ptr.}");
        wal_cfg80211_exception_handle(wdev, net_dev);
        return OAL_FAIL;
    }
    oal_set_mac_addr((uint8_t *)oal_netdevice_mac_addr(net_dev), mac_mib_get_StationID(mac_vap));

    /* 设置VAP UP */
    wal_netdev_open(net_dev, OAL_FALSE);

    oam_warning_log1(0, OAM_SF_CFG, "{wal_cfg80211_infomation_proc_func::succ. vap_id[%d]!}\r\n",
        mac_vap->uc_vap_id);
    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_cfg80211_set_info_func(oal_wiphy_stru *wiphy, mac_device_stru **mac_device, const char *name,
    enum nl80211_iftype type, oal_vif_params_stru *params)
{
    mac_wiphy_priv_stru *wiphy_priv = NULL;
    /* 1.1 入参检查 */
    if (wal_cfg80211_param_check(wiphy, name, params) != OAL_TRUE) {
        return OAL_FAIL;
    }
    /* 入参检查无异常后赋值，并调用OAL统一接口 */
    wiphy_priv = oal_wiphy_priv(wiphy);
    if (wiphy_priv == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_set_info_func::wiphy_priv is null!}\r\n");
        return OAL_FAIL;
    }
    *mac_device = wiphy_priv->pst_mac_device;
    if ((*mac_device) == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_set_info_func::mac_device is null!}\r\n");
        return OAL_FAIL;
    }

    oam_warning_log1(0, OAM_SF_CFG, "{wal_cfg80211_set_info_func::type[%d]!}\r\n", type);
    /* 提示:SDT无法打印%s字符串形式 */
    oal_io_print("wal_cfg80211_set_info_func, device name is:%s\n", name);
    return OAL_SUCC;
}

OAL_STATIC void wal_cfg80211_set_net_device_info(oal_wireless_dev_stru *wdev, oal_net_device_stru *net_dev,
    enum nl80211_iftype type, mac_device_stru *mac_device)
{
    /* 对netdevice进行赋值 */
    wal_cfg80211_add_virtual_intf_netdev_init(net_dev, wdev);

    wdev->iftype = type;
    wdev->wiphy = mac_device->pst_wiphy;
    wdev->netdev = net_dev; /* 给wdev 中的net_device 赋值 */
}

OAL_STATIC uint32_t wal_cfg80211_ready_register(wlan_p2p_mode_enum_uint8 p2p_mode, wlan_vap_mode_enum_uint8 vap_mode,
    mac_device_stru *mac_device, oal_wireless_dev_stru *wdev, oal_net_device_stru *net_dev)
{
    uint32_t ret;
    if (wlan_vap_mode_legacy_vap(p2p_mode, vap_mode)) {
        /* 如果创建wlan0， 则保存wlan0 为主net_device,p2p0 和p2p-p2p0 MAC 地址从主netdevice 获取 */
        if (mac_device->st_p2p_info.pst_primary_net_device == NULL) {
            /* 创建wlan0 在加载wifi 驱动时，不需要通过该接口 */
            oam_error_log0(0, 0, "{wal_cfg80211_ready_register::should not go here!}\r\n");
            oal_mem_free_m(wdev, OAL_FALSE);
            oal_net_free_netdev(net_dev);
            return OAL_FAIL;
        }
    }

    /* 防止上层不下发mac addr,导致驱动侧p2p-p2p0-x mac addr未初始化 */
    wal_cfg80211_add_p2p_interface_init(net_dev, mac_device);

    oal_netdevice_flags(net_dev) &= ~OAL_IFF_RUNNING; /* 将net device的flag设为down */

    ret = wal_cfg80211_register_netdev(net_dev);
    if (oal_unlikely(ret != OAL_SUCC)) {
        /* 注册不成功，释放资源 */
        oal_mem_free_m(wdev, OAL_FALSE);
        oal_net_free_netdev(net_dev);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
OAL_STATIC oal_wireless_dev_stru *wal_cfg80211_add_virtual_intf(oal_wiphy_stru *wiphy, const char *name,
    unsigned char name_assign_type, enum nl80211_iftype type, oal_vif_params_stru *params)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
OAL_STATIC oal_wireless_dev_stru *wal_cfg80211_add_virtual_intf(oal_wiphy_stru *wiphy, const char *name,
    unsigned char name_assign_type, enum nl80211_iftype type, uint32_t *pul_flags, oal_vif_params_stru *params)
#else
OAL_STATIC oal_wireless_dev_stru *wal_cfg80211_add_virtual_intf(oal_wiphy_stru *wiphy, const char *name,
    enum nl80211_iftype type, uint32_t *pul_flags, oal_vif_params_stru *params)
#endif
{
    oal_wireless_dev_stru *wdev = NULL;
    wlan_p2p_mode_enum_uint8 p2p_mode;
    wlan_vap_mode_enum_uint8 vap_mode;
    oal_net_device_stru *net_dev = NULL;
    mac_device_stru *mac_device = NULL;
    hmac_vap_stru *cfg_hmac_vap = NULL;
    uint32_t ret;

    // 入参检测，检测无误后设置数据
    if (wal_cfg80211_set_info_func(wiphy, &mac_device, name, type, params) != OAL_SUCC) {
        return ERR_PTR(-OAL_EBUSY);
    }

    net_dev = mac_device->st_p2p_info.pst_p2p_net_device;
    ret = wal_set_nl80211_work_mode(type, &p2p_mode, &vap_mode, &wdev, net_dev);
    if (ret == OAL_FAIL) {
        return ERR_PTR(-OAL_EBUSY);
    } else if (ret == OAL_TRUE) {
        return wdev;
    }

    /* 如果创建的net device已经存在，直接返回 */
    /* 根据dev_name找到dev */
    ret = wal_cfg80211_search_dev(&net_dev, name, mac_device, &wdev);
    if (ret == OAL_FAIL) {
        return ERR_PTR(-OAL_EBUSY);
    } else if (ret == OAL_TRUE) {
        return wdev;
    }

    /* 获取配置VAP 结构 */
    g_cfg_vap_id = mac_device->uc_cfg_vap_id;
    if (wal_cfg80211_get_vap_info(&cfg_hmac_vap) != OAL_SUCC) {
        return ERR_PTR(-OAL_EBUSY);
    }

    /* 申请内存 */
    if (wal_cfg80211_mem_prep(name, &net_dev, &wdev) != OAL_SUCC) {
        return ERR_PTR(-OAL_EBUSY);
    }

    memset_s(wdev, sizeof(oal_wireless_dev_stru), 0, sizeof(oal_wireless_dev_stru));
    wal_cfg80211_set_net_device_info(wdev, net_dev, type, mac_device);
    if (wal_cfg80211_ready_register(p2p_mode, vap_mode, mac_device, wdev, net_dev) != OAL_SUCC) {
        return ERR_PTR(-OAL_EBUSY);
    }

    /* 抛事件到wal层处理 */
    ret = wal_cfg80211_infomation_proc_func(net_dev, p2p_mode, vap_mode, wdev, cfg_hmac_vap);
    g_cfg_vap_id = 0;  // 清除 cfg_vap_indx
    if (ret != OAL_SUCC) {
        return ERR_PTR(-OAL_EBUSY);
    }

    oam_warning_log1(0, OAM_SF_CFG, "{wal_cfg80211_add_virtual_intf::succ. type[%d]!}\r\n", type);
    return wdev;
}

OAL_STATIC oal_bool_enum_uint8 wal_cfg80211_check_is_primary_netdev(oal_wiphy_stru *wiphy,
    oal_net_device_stru *net_dev)
{
    mac_device_stru *mac_device = NULL;
    mac_wiphy_priv_stru *wiphy_priv = NULL;
    wiphy_priv = oal_wiphy_priv(wiphy);
    if (wiphy_priv == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_check_is_primary_netdev::pst_wiphy_priv is null!}");
        return OAL_FALSE;
    }
    mac_device = wiphy_priv->pst_mac_device;
    if (mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_check_is_primary_netdev::pst_mac_device is null!}");
        return OAL_FALSE;
    }
    return mac_device->st_p2p_info.pst_primary_net_device == net_dev;
}

OAL_STATIC int32_t wal_cfg80211_del_virtual_intf(oal_wiphy_stru *pst_wiphy, oal_wireless_dev_stru *pst_wdev)
{
    /* 异步去注册net_device */
    wal_msg_write_stru st_write_msg;
    wal_msg_stru *pst_rsp_msg = NULL;
    int32_t l_ret;
    oal_net_device_stru *pst_net_dev = NULL;
    mac_vap_stru *pst_mac_vap = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    hmac_device_stru *pst_hmac_device = NULL;

    if (oal_unlikely(oal_any_null_ptr2(pst_wiphy, pst_wdev))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::pst_wiphy or pst_wdev is null !}");
        return -OAL_EINVAL;
    }

    if (wal_cfg80211_dfr_and_s3s4_param_check() == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf:: dfr or s3s4 is processing!}");
        return OAL_SUCC;
    }

    pst_net_dev = pst_wdev->netdev;
    if (pst_net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::pst_net_dev is null by netdev, mode[%d]!}");
        return -OAL_EINVAL;
    }

    pst_mac_vap = oal_net_dev_priv(pst_net_dev);
    if (pst_mac_vap == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::mac_vap is null by netdev, mode[%d]!}\r\n",
            pst_net_dev->ieee80211_ptr->iftype);
        return -OAL_EINVAL;
    }
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log1(0, 0, "{wal_cfg80211_del_virtual_intf::get_hmac_vap fail.vap_id[%u]}", pst_mac_vap->uc_vap_id);
        return -OAL_EINVAL;
    }
    if (wal_cfg80211_check_is_primary_netdev(pst_wiphy, pst_net_dev)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::cannot del primary netdev}");
        return -OAL_EINVAL;
    }
    oal_net_tx_stop_all_queues(pst_net_dev);
    wal_netdev_stop(pst_net_dev);
    /* 在释放完net_device 后释放wireless device */
    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    /* 初始化删除vap 参数 */
    ((mac_cfg_del_vap_param_stru *)st_write_msg.auc_value)->pst_net_dev = pst_net_dev;
    pst_wdev = pst_net_dev->ieee80211_ptr;
    if (wal_wireless_iftype_to_mac_p2p_mode(pst_wdev->iftype) == WLAN_P2P_BUTT) {
        oam_error_log0(0, 0, "{wal_cfg80211_del_virtual_intf::wal_wireless_iftype_to_mac_p2p_mode return BUTT}");
        return -OAL_EINVAL;
    }

    ((mac_cfg_del_vap_param_stru *)st_write_msg.auc_value)->en_p2p_mode = mac_get_p2p_mode(pst_mac_vap);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DEL_VAP, sizeof(mac_cfg_del_vap_param_stru));

    /* 设置删除net_device 标识 */
    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_warning_log1(0, 0, "{wal_cfg80211_del_virtual_intf::hmac_device is null!}", pst_mac_vap->uc_device_id);
        return -OAL_EINVAL;
    }
    hmac_set_p2p_status(&pst_hmac_device->p2p_intf_status, P2P_STATUS_IF_DELETING);

    /* 启动linux work 删除net_device */
    pst_hmac_vap->pst_del_net_device = pst_net_dev;
    oal_workqueue_schedule(&(pst_hmac_vap->st_del_virtual_inf_worker));

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_del_vap_param_stru),
        (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg);
    if (OAL_SUCC != wal_check_and_release_msg_resp(pst_rsp_msg)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::wal_check_and_release_msg_resp fail}");
    }
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_del_virtual_intf::return err code %d}\r\n", l_ret);
        l_ret = -OAL_EFAIL;
    }
    oam_warning_log2(0, OAM_SF_ANY,
        "{wal_cfg80211_del_virtual_intf::pst_hmac_device->p2p_intf_status %d, del result: %d}\r\n",
        pst_hmac_device->p2p_intf_status, l_ret);
    return l_ret;
}
//#endif


OAL_STATIC int32_t wal_cfg80211_mgmt_tx_cancel_wait(oal_wiphy_stru *pst_wiphy,
    oal_wireless_dev_stru *pst_wdev, uint64_t ull_cookie)
{
    oal_net_device_stru *pst_netdev = NULL;
    mac_vap_stru *pst_mac_vap = NULL;

    if (pst_wdev == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx_cancel_wait::pst_wdev is Null!}");
        return -OAL_EFAIL;
    }

    pst_netdev = pst_wdev->netdev;
    if (pst_netdev == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx_cancel_wait::pst_netdev is Null!}");
        return -OAL_EFAIL;
    }

    pst_mac_vap = oal_net_dev_priv(pst_netdev);
    if (pst_mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_CFG,
            "{wal_cfg80211_mgmt_tx_cancel_wait::can't get mac vap from netdevice priv data!}");
        return -OAL_EFAIL;
    }

    return wal_p2p_stop_roc(pst_mac_vap, pst_netdev);
}


OAL_STATIC int32_t wal_cfg80211_set_pmksa(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_net_device, oal_cfg80211_pmksa_stru *pmksa)
{
    wal_msg_write_stru st_write_msg = { 0 };
    mac_cfg_pmksa_param_stru *pst_cfg_pmksa = NULL;
    int32_t l_ret;
    int32_t l_memcpy_ret;

    if (oal_any_null_ptr3(pst_wiphy, pst_net_device, pmksa)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_set_pmksa::wiphy or netdev or pmksa is null!}");
        return -OAL_EINVAL;
    }

    if (oal_any_null_ptr2(pmksa->bssid, pmksa->pmkid)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_set_pmksa::bssid or pmkid is null!}");
        return -OAL_EINVAL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_SET_PMKSA, sizeof(mac_cfg_pmksa_param_stru));
    pst_cfg_pmksa = (mac_cfg_pmksa_param_stru *)st_write_msg.auc_value;
    l_memcpy_ret = memcpy_s(pst_cfg_pmksa->auc_bssid, WLAN_MAC_ADDR_LEN, pmksa->bssid, WLAN_MAC_ADDR_LEN);
    l_memcpy_ret += memcpy_s(pst_cfg_pmksa->auc_pmkid, WLAN_PMKID_LEN, pmksa->pmkid, WLAN_PMKID_LEN);
    if (l_memcpy_ret != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "wal_cfg80211_set_pmksa::memcpy fail!");
        return -OAL_EFAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_device, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_pmksa_param_stru), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (l_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_CFG, "{wal_cfg80211_set_pmksa::wal_send_cfg_event fail[%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC int32_t wal_cfg80211_del_pmksa(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_net_device, oal_cfg80211_pmksa_stru *pmksa)
{
    wal_msg_write_stru st_write_msg = { 0 };
    mac_cfg_pmksa_param_stru *pst_cfg_pmksa = NULL;
    int32_t l_ret;
    int32_t l_memcpy_ret;

    if (oal_any_null_ptr3(pst_wiphy, pst_net_device, pmksa)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_del_pmksa::param null!}\r\n");
        return -OAL_EINVAL;
    }

    if (oal_any_null_ptr2(pmksa->bssid, pmksa->pmkid)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_del_pmksa::param null!}");
        return -OAL_EINVAL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_DEL_PMKSA, sizeof(mac_cfg_pmksa_param_stru));
    pst_cfg_pmksa = (mac_cfg_pmksa_param_stru *)st_write_msg.auc_value;
    l_memcpy_ret = memcpy_s(pst_cfg_pmksa->auc_bssid, WLAN_MAC_ADDR_LEN, pmksa->bssid, WLAN_MAC_ADDR_LEN);
    l_memcpy_ret += memcpy_s(pst_cfg_pmksa->auc_pmkid, WLAN_PMKID_LEN, pmksa->pmkid, WLAN_PMKID_LEN);
    if (l_memcpy_ret != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "wal_cfg80211_del_pmksa::memcpy fail!");
        return -OAL_EFAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_device, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_pmksa_param_stru), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (l_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_CFG, "{wal_cfg80211_del_pmksa::wal_send_cfg_event fail[%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC int32_t wal_cfg80211_flush_pmksa(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_net_device)
{
    wal_msg_write_stru st_write_msg = { 0 };
    int32_t l_ret;
    mac_vap_stru *pst_mac_vap = NULL;

    if (oal_any_null_ptr2(pst_wiphy, pst_net_device)) {
        oam_error_log0(0, OAM_SF_CFG,
                       "{wal_cfg80211_flush_pmksa::param null!}");
        return -OAL_EINVAL;
    }

    /* 通过net_device 找到对应的mac_vap_stru 结构，以防wal_alloc_cfg_event返回ERROR */
    pst_mac_vap = oal_net_dev_priv(pst_net_device);
    if (pst_mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_flush_pmksa::can't get mac vap from netdevice priv data!}");
        return -OAL_EFAIL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_FLUSH_PMKSA, 0);

    l_ret = wal_send_cfg_event(pst_net_device, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH, (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_cfg80211_flush_pmksa::wal_send_cfg_event fail[%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC void wal_drv_remain_on_channel_debug_info(mac_vap_stru *pst_mac_vap,
    int32_t l_channel, uint32_t duration, uint64_t *pull_cookie, wlan_ieee80211_roc_type_uint8 en_roc_type)
{
    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
        "{wal_drv_remain_on_channel::SUCC! l_channel=%d, duration=%d, roc type=%d!}\r\n",
        l_channel, duration, en_roc_type);
}
OAL_STATIC int32_t wal_drv_check_vap_state(mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap,
    wlan_ieee80211_roc_type_uint8 en_roc_type)
{
    mac_vap_stru *pst_other_vap = NULL;
    uint8_t uc_vap_idx;
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_other_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_other_vap == NULL) {
            oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
                "{wal_drv_remain_on_channel::vap is null! vap id is %d}", pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }
        if (oal_value_in_valid_range(pst_other_vap->en_vap_state,
            MAC_VAP_STATE_STA_JOIN_COMP, MAC_VAP_STATE_STA_WAIT_ASOC)
            || (pst_other_vap->en_vap_state == MAC_VAP_STATE_ROAMING)
        ) {
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
                             "{wal_drv_remain_on_channel::invalid ROC[%d] for roaming or connecting state[%d]!}",
                             en_roc_type, pst_other_vap->en_vap_state);
            return -OAL_EFAIL;
        }
    }
    return OAL_SUCC;
}

OAL_STATIC int32_t wal_drv_remain_on_channel(oal_wiphy_stru *pst_wiphy, oal_wireless_dev_stru *pst_wdev,
    struct ieee80211_channel *pst_chan, uint32_t duration, uint64_t *pull_cookie,
    wlan_ieee80211_roc_type_uint8 en_roc_type)
{
    wal_msg_write_stru st_write_msg = { 0 };
    wal_msg_stru *pst_rsp_msg = NULL;
    uint32_t err_code;
    oal_net_device_stru *pst_netdev = NULL;
    hmac_remain_on_channel_param_stru st_remain_on_channel = { 0 };
    uint16_t us_center_freq;
    int32_t l_channel;
    int32_t l_ret;
    mac_device_stru *pst_mac_device = NULL;
    mac_vap_stru *pst_mac_vap = NULL;

    /* 1.1 入参检查 */
    if (oal_any_null_ptr4(pst_wiphy, pst_wdev, pst_chan, pull_cookie)) {
        oam_error_log0(0, OAM_SF_P2P, "{wal_drv_remain_on_channel::wiphy/wdev/chan/cookie is null!}");
        return -OAL_EINVAL;
    }

    pst_netdev = pst_wdev->netdev;
    if (pst_netdev == NULL) {
        oam_error_log0(0, OAM_SF_P2P, "{wal_drv_remain_on_channel::pst_netdev ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    /* 通过net_device 找到对应的mac_device_stru 结构 */
    pst_mac_vap = oal_net_dev_priv(pst_netdev);
    if (pst_mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_P2P, "{wal_drv_remain_on_channel::can't get mac vap from netdevice priv data!}\r\n");
        return -OAL_EINVAL;
    }

    pst_mac_device = (mac_device_stru *)mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log0(0, OAM_SF_P2P, "{wal_drv_remain_on_channel::pst_mac_device ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_WAPI
    if (OAL_TRUE == hmac_user_is_wapi_connected(pst_mac_vap->uc_device_id)) {
        oam_warning_log0(0, OAM_SF_CFG, "{stop p2p remaining under wapi!}");
        return -OAL_EINVAL;
    }
#endif /* #ifdef _PRE_WLAN_FEATURE_WAPI */

    /* check all vap state in case other vap is connecting now */
    if (wal_drv_check_vap_state(pst_mac_device, pst_mac_vap, en_roc_type) != OAL_SUCC) {
        return -OAL_EFAIL;
    }

    /* tx mgmt roc 优先级低,可以被自己的80211 roc以及80211 scan打断 */
    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
            "{wal_drv_remain_on_channel::new roc type[%d],cancel old roc!}", en_roc_type);
        l_ret = wal_p2p_stop_roc(pst_mac_vap, pst_netdev);
        if (l_ret < 0) {
            return -OAL_EFAIL;
        }
    }

    /* 2.1 消息参数准备 */
    us_center_freq = pst_chan->center_freq;
    l_channel = (int32_t)oal_ieee80211_frequency_to_channel((int32_t)us_center_freq);

    st_remain_on_channel.uc_listen_channel = (uint8_t)l_channel;
    st_remain_on_channel.listen_duration = duration;
    st_remain_on_channel.st_listen_channel = *pst_chan;
    st_remain_on_channel.en_listen_channel_type = WLAN_BAND_WIDTH_20M;
    st_remain_on_channel.en_roc_type = en_roc_type;

    if (pst_chan->band == HISI_IEEE80211_BAND_2GHZ) {
        st_remain_on_channel.en_band = WLAN_BAND_2G;
    } else if (pst_chan->band == HISI_IEEE80211_BAND_5GHZ) {
        st_remain_on_channel.en_band = WLAN_BAND_5G;
    } else {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
            "{wal_drv_remain_on_channel::wrong band en_roc_type[%d]!}\r\n", pst_chan->band);
        return -OAL_EINVAL;
    }

    if (en_roc_type == IEEE80211_ROC_TYPE_NORMAL) {
        /* 设置cookie 值 */ /* cookie值上层调用需要判断是否是这次的发送导致的callback */
        *pull_cookie = ++pst_mac_device->st_p2p_info.ull_last_roc_id;
        if (*pull_cookie == 0) {
            *pull_cookie = ++pst_mac_device->st_p2p_info.ull_last_roc_id;
        }

        /* 保存cookie 值，下发给HMAC 和DMAC */
        st_remain_on_channel.ull_cookie = pst_mac_device->st_p2p_info.ull_last_roc_id;
    }
    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 3.1 填写 msg 消息头 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_REMAIN_ON_CHANNEL,
        sizeof(hmac_remain_on_channel_param_stru));

    /* 3.2 填写 msg 消息体 */
    if (EOK != memcpy_s(st_write_msg.auc_value, WAL_MSG_WRITE_MAX_LEN,
                        &st_remain_on_channel, sizeof(hmac_remain_on_channel_param_stru))) {
        oam_error_log0(0, OAM_SF_P2P, "wal_drv_remain_on_channel::memcpy fail!");
    }

    /* 3.3 发送消息 */
    l_ret = wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH +
        sizeof(hmac_remain_on_channel_param_stru), (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg);
    if (l_ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
            "{wal_drv_remain_on_channel::wal_send_cfg_event return err code:[%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    /* 4.1 读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{wal_drv_remain_on_channel::wal_send_cfg_event return err code:[%u]!}", err_code);
        return -OAL_EFAIL;
    }

    if (en_roc_type == IEEE80211_ROC_TYPE_NORMAL) {
        /* 上报暂停在指定信道成功 */
        oal_cfg80211_ready_on_channel(pst_wdev, *pull_cookie, pst_chan, duration, GFP_KERNEL);
    }
    // 封装函数降低行数
    wal_drv_remain_on_channel_debug_info(pst_mac_vap, l_channel, duration, pull_cookie, en_roc_type);

    return OAL_SUCC;
}


OAL_STATIC int32_t wal_cfg80211_remain_on_channel(oal_wiphy_stru *pst_wiphy,
    oal_wireless_dev_stru *pst_wdev, struct ieee80211_channel *pst_chan, uint32_t duration, uint64_t *pull_cookie)
{
    return wal_drv_remain_on_channel(pst_wiphy, pst_wdev, pst_chan, duration,
                                     pull_cookie, IEEE80211_ROC_TYPE_NORMAL);
}


OAL_STATIC int32_t wal_cfg80211_cancel_remain_on_channel(oal_wiphy_stru *pst_wiphy,
    oal_wireless_dev_stru *pst_wdev, uint64_t ull_cookie)
{
    wal_msg_write_stru st_write_msg = { 0 };
    hmac_remain_on_channel_param_stru st_cancel_remain_on_channel = { 0 };
    wal_msg_stru *pst_rsp_msg = NULL;
    uint32_t err_code;
    oal_net_device_stru *pst_netdev = NULL;
    int32_t l_ret;

    if (oal_any_null_ptr2(pst_wiphy, pst_wdev)) {
        oam_error_log0(0, OAM_SF_P2P,
            "{wal_cfg80211_cancel_remain_on_channel::pst_wiphy or pst_wdev is null!}");
        return -OAL_EINVAL;
    }

    pst_netdev = pst_wdev->netdev;
    if (pst_netdev == NULL) {
        oam_error_log0(0, OAM_SF_P2P, "{wal_cfg80211_cancel_remain_on_channel::pst_netdev ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    oam_warning_log1(0, OAM_SF_P2P, "wal_cfg80211_cancel_remain_on_channel[0x%x].", ull_cookie);

    /* 3.1 填写 msg 消息头 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CFG80211_CANCEL_REMAIN_ON_CHANNEL,
        sizeof(hmac_remain_on_channel_param_stru));

    /* 3.2 填写 msg 消息体 */
    if (EOK != memcpy_s(st_write_msg.auc_value, sizeof(hmac_remain_on_channel_param_stru),
                        &st_cancel_remain_on_channel, sizeof(hmac_remain_on_channel_param_stru))) {
        oam_error_log0(0, OAM_SF_P2P, "wal_cfg80211_cancel_remain_on_channel::memcpy fail!");
        return -OAL_EFAIL;
    }

    /* 3.3 发送消息 */
    l_ret = wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(hmac_remain_on_channel_param_stru),
                               (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_P2P,
            "{wal_cfg80211_cancel_remain_on_channel::wal_send_cfg_event return err code: [%d]!}", l_ret);
        return -OAL_EFAIL;
    }

    /* 4.1 读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_P2P,
            "{wal_cfg80211_cancel_remain_on_channel::wal_check_and_release_msg_resp fail return err code:%u!}",
            err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


uint8_t wal_find_oldest_cookie(cookie_arry_stru *pst_cookie_array)
{
    uint8_t uc_loops;
    uint8_t uc_target_index = 0;

    /* 外部保证只有在cookie用满的情况下才会调用该函数，因此初始index选0即可 */
    for (uc_loops = 1; uc_loops < WAL_COOKIE_ARRAY_SIZE; uc_loops++) {
        if (oal_time_after32(pst_cookie_array[uc_target_index].record_time,
            pst_cookie_array[uc_loops].record_time)) {
            uc_target_index = uc_loops;
        }
    }

    return uc_target_index;
}


void wal_check_cookie_timeout(cookie_arry_stru *pst_cookie_array,
    uint8_t *puc_cookie_bitmap, uint32_t current_time)
{
    uint8_t uc_loops = 0;
    cookie_arry_stru *pst_tmp_cookie = NULL;

    oam_warning_log0(0, OAM_SF_CFG, "{wal_check_cookie_timeout::time_out!}\r\n");
    for (uc_loops = 0; uc_loops < WAL_COOKIE_ARRAY_SIZE; uc_loops++) {
        pst_tmp_cookie = &pst_cookie_array[uc_loops];
        if (oal_time_after32(OAL_TIME_JIFFY,
            pst_tmp_cookie->record_time + oal_msecs_to_jiffies(WAL_MGMT_TX_TIMEOUT_MSEC))) {
            /* cookie array 中保存的cookie 值超时 */
            /* 清空cookie array 中超时的cookie */
            pst_tmp_cookie->record_time = 0;
            pst_tmp_cookie->ull_cookie = 0;
            /* 清除占用的cookie bitmap位 */
            oal_bit_clear_bit_one_byte(puc_cookie_bitmap, uc_loops);
        }
    }
}


OAL_STATIC uint32_t wal_del_cookie_from_array(cookie_arry_stru *pst_cookie_array,
                                              uint8_t *puc_cookie_bitmap, uint8_t uc_cookie_idx)
{
    cookie_arry_stru *pst_tmp_cookie = NULL;

    /* 清除对应cookie bitmap 位 */
    oal_bit_clear_bit_one_byte(puc_cookie_bitmap, uc_cookie_idx);

    /* 清空cookie array 中超时的cookie */
    pst_tmp_cookie = &pst_cookie_array[uc_cookie_idx];
    pst_tmp_cookie->ull_cookie = 0;
    pst_tmp_cookie->record_time = 0;
    return OAL_SUCC;
}


uint32_t wal_add_cookie_to_array(cookie_arry_stru *pst_cookie_array,
    uint8_t *puc_cookie_bitmap, uint64_t *pull_cookie, uint8_t *puc_cookie_idx)
{
    uint8_t uc_idx;
    cookie_arry_stru *pst_tmp_cookie = NULL;

    if (*puc_cookie_bitmap == WAL_COOKIE_FULL_MASK) {
        /* cookie array 满，返回错误 */
        oam_warning_log0(0, OAM_SF_CFG, "{wal_add_cookie_to_array::array full!}\r\n");
        return OAL_FAIL;
    }

    /* 将cookie 添加到array 中 */
    uc_idx = oal_bit_find_first_zero_one_byte(*puc_cookie_bitmap);
    oal_bit_set_bit_one_byte(puc_cookie_bitmap, uc_idx);

    pst_tmp_cookie = &pst_cookie_array[uc_idx];
    pst_tmp_cookie->ull_cookie = *pull_cookie;
    pst_tmp_cookie->record_time = OAL_TIME_JIFFY;

    *puc_cookie_idx = uc_idx;
    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_check_cookie_from_array(uint8_t *puc_cookie_bitmap, uint8_t uc_cookie_idx)
{
    /* 从cookie bitmap中查找相应的cookie index，如果位图为0，表示已经被del */
    if (*puc_cookie_bitmap & (BIT(uc_cookie_idx))) {
        return OAL_SUCC;
    }
    /* 找不到则返回FAIL */
    return OAL_FAIL;
}

OAL_STATIC oal_netbuf_stru *wal_mgmt_tx_prepare_netbuf(const uint8_t *buff, size_t len)
{
    oal_netbuf_stru *netbuf;
    int32_t ret;

    /*  申请netbuf 空间 */
    netbuf = (oal_netbuf_stru *)oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, len, OAL_NETBUF_PRIORITY_MID);
    /* Reserved Memory pool tried for high priority frames */
    if (netbuf == NULL && len <= WLAN_MEM_NETBUF_SIZE2) {
        netbuf = (oal_netbuf_stru *)oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF,
            WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    }

    if (netbuf == NULL) {
        oam_error_log1(0, OAM_SF_P2P, "{wal_mgmt_tx_prepare_netbuf::alloc netbuf failed, len %d.}", len);
        return NULL;
    }

    /* 填充netbuf */
    ret = memcpy_s(oal_netbuf_header(netbuf), len, buff, len);
    if (ret != EOK) {
        oam_error_log1(0, OAM_SF_P2P, "{wal_mgmt_tx_prepare_netbuf::memcpy fail!errno[%d]}", ret);
        oal_netbuf_free(netbuf);
        return NULL;
    }

    oal_netbuf_put(netbuf, len);
    oal_mem_netbuf_trace(netbuf, OAL_TRUE);
    return netbuf;
}


OAL_STATIC uint32_t wal_mgmt_do_tx_status_report(uint32_t wait, oal_mgmt_tx_stru *mgmt_tx)
{
    signed long i_leftime;

    /*lint -e730*/
    i_leftime = oal_wait_event_interruptible_timeout_m(mgmt_tx->st_wait_queue,
        (oal_bool_enum_uint8)(OAL_TRUE == mgmt_tx->mgmt_tx_complete),
        (uint32_t)oal_msecs_to_jiffies(wait));
    if (i_leftime == 0) {
        /* 定时器超时 */
        oam_warning_log0(0, OAM_SF_ANY, "{wal_mgmt_do_tx_status_report::mgmt tx timeout!}");
        return OAL_FAIL;
    } else if (i_leftime < 0) {
        /* 定时器内部错误 */
        oam_warning_log0(0, OAM_SF_ANY, "{wal_mgmt_do_tx_status_report::mgmt tx timer error!}");
        return OAL_FAIL;
    } else {
        /* 正常结束  */
        return mgmt_tx->mgmt_tx_status;
    }
}


OAL_STATIC uint32_t wal_mgmt_do_tx(oal_net_device_stru *netdev, hmac_mgmt_frame_stru *mgmt_tx_param, bool offchan,
                                   uint32_t wait, const uint8_t *buff, size_t len)
{
    mac_vap_stru *mac_vap = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    oal_mgmt_tx_stru *mgmt_tx = NULL;
    int32_t ret;

    mac_vap = oal_net_dev_priv(netdev);
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_mgmt_do_tx::can't get mac vap from netdevice priv data.}");
        return OAL_FAIL;
    }

    if (!IS_P2P_GO(mac_vap)) {
        // 如果不是Go，则做如下判断
        if (offchan == OAL_TRUE && mac_vap->en_vap_state != MAC_VAP_STATE_STA_LISTEN) {
            oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                "{wal_mgmt_do_tx::pst_mac_vap state[%d]not in listen!}", mac_vap->en_vap_state);
            return OAL_ERR_CODE_WRONG_CHANNEL;  // 软件不再重传,告诉上层此次tx mgmt失败结束
        }
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_mgmt_do_tx::pst_hmac_vap ptr is null!}");
        return OAL_FAIL;
    }

    mgmt_tx_param->frame = wal_mgmt_tx_prepare_netbuf(buff, len);
    if (mgmt_tx_param->frame == NULL) {
        return OAL_FAIL;
    }

    mgmt_tx = &(hmac_vap->st_mgmt_tx);
    mgmt_tx->mgmt_tx_complete = OAL_FALSE;
    mgmt_tx->mgmt_tx_status = OAL_FAIL;

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    ret = wal_cfg80211_start_req(netdev, mgmt_tx_param, sizeof(hmac_mgmt_frame_stru),
                                 WLAN_CFGID_CFG80211_MGMT_TX, OAL_FALSE);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oal_netbuf_free(mgmt_tx_param->frame); // need rsp参数传入false，返回fail说明事件没有抛成功，需要释放netbuf。
        mgmt_tx_param->frame = NULL;
        oam_warning_log1(0, OAM_SF_ANY, "{wal_mgmt_do_tx::wal_send_cfg_event return err code %d!}", ret);
        return OAL_FAIL;
    }

    return wal_mgmt_do_tx_status_report(wait, mgmt_tx);
}

OAL_STATIC int32_t wal_cfg80211_mgmt_tx_bss_ap_need_offchan(hmac_vap_stru *pst_hmac_vap,
    hmac_mgmt_frame_stru *pst_mgmt_tx, mac_device_stru *pst_mac_device,
    oal_wireless_dev_stru *pst_roc_wireless_dev, bool *pen_need_offchan)
{
    // 此处应该是channel number，而不是channel index
    oam_warning_log3(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
        "{wal_cfg80211_mgmt_tx::p2p mode[%d](0=Legacy, 1=Go, 2=Dev, 3=Gc), tx mgmt vap chan[%d], mgmt tx chan[%d]",
        pst_hmac_vap->st_vap_base_info.en_p2p_mode, pst_hmac_vap->st_vap_base_info.st_channel.uc_chan_number,
        pst_mgmt_tx->channel);

    if ((pst_hmac_vap->st_vap_base_info.st_channel.uc_chan_number != pst_mgmt_tx->channel) &&
        IS_P2P_GO(&pst_hmac_vap->st_vap_base_info)) {
        if (pst_mac_device->st_p2p_info.pst_p2p_net_device == NULL) {
            oam_error_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, 0, "{wal_cfg80211_mgmt_tx::go,p2p dev null");
            return -OAL_EINVAL;
        }
        // 使用p2p0 roc
        pst_roc_wireless_dev = oal_netdevice_wdev(pst_mac_device->st_p2p_info.pst_p2p_net_device);
        *pen_need_offchan = OAL_TRUE;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_cfg80211_set_net_info(oal_wireless_dev_stru *wdev, oal_net_device_stru **netdev,
    mac_vap_stru **mac_vap, mac_device_stru **mac_device, hmac_vap_stru **hmac_vap)
{
    /* 通过net_device 找到对应的mac_device_stru 结构 */
    *netdev = wdev->netdev;
    if (*netdev == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::netdev ptr is null!}\r\n");
        return OAL_FAIL;
    }

    *mac_vap = oal_net_dev_priv(*netdev);
    if (*mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::can't get mac vap from netdevice priv data!}\r\n");
        return OAL_FAIL;
    }

    *mac_device = (mac_device_stru *)mac_res_get_dev((*mac_vap)->uc_device_id);
    if (*mac_device == NULL) {
        return OAL_FAIL;
    }

    *hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap((*mac_vap)->uc_vap_id);
    if (*hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx::hmac_vap ptr is null!}\r\n");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_cfg80211_set_channel_msg(oal_ieee80211_channel *chan_info, mac_vap_stru *mac_vap,
    const oal_ieee80211_mgmt *mgmt, oal_ieee80211_channel *chan, size_t len)
{
#ifdef _PRE_WLAN_FEATURE_SAE
    if (chan_info == NULL) {
        mac_bss_dscr_stru *bss_dscr;
        uint8_t mac_addr[WLAN_MAC_ADDR_LEN];

        if (!IS_STA(mac_vap)) {
            oam_error_log2(mac_vap->uc_vap_id, 0, "{wal_cfg80211_mgmt_tx::chan null! vap_mode[%d], p2p_mode[%d]}",
                mac_vap->en_vap_mode, mac_vap->en_p2p_mode);
            return OAL_FAIL;
        }

        /* SAE发送auth commit/comfirm 帧，下发的pst_channel入参是NULL,
         * 需要根据目的MAC在扫描结果中查找信道信息 */
        oam_warning_log0(0, OAM_SF_SAE, "{wal_cfg80211_mgmt_tx::STA tx frame, find channel from scan result}");
        /* 判断传入的帧长度是否够长 */
        if (len < MAC_80211_FRAME_LEN) {
            oam_warning_log1(mac_vap->uc_vap_id, 0, "{wal_cfg80211_mgmt_tx::auth frame len too short.}", len);
            return OAL_FAIL;
        }

        if (EOK != memcpy_s(mac_addr, WLAN_MAC_ADDR_LEN, mgmt->da, WLAN_MAC_ADDR_LEN)) {
            return OAL_FAIL;
        }
        bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(mac_vap, mac_addr);
        if (bss_dscr == NULL) {
            oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_SAE,
                "{wal_cfg80211_mgmt_tx::can not find [%02X:XX:XX:XX:%02X:%02X] from scan result.}",
                mac_addr[0], mac_addr[4], mac_addr[5]); // [4 5] ：mac addr 的最后两位数据
            return OAL_FAIL;
        }
        chan->band = bss_dscr->st_channel.en_band;
        chan->center_freq = oal_ieee80211_channel_to_frequency(bss_dscr->st_channel.uc_chan_number,
            bss_dscr->st_channel.en_band);
        chan->hw_value = bss_dscr->st_channel.uc_chan_number;
    } else {
        *chan = *chan_info;
    }
#else
    if (chan_info == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_tx:: chan_info ptr is null!}");
        return OAL_FAIL;
    }
    *chan = *chan_info;
#endif /* _PRE_WLAN_FEATURE_SAE */
    return OAL_SUCC;
}


OAL_STATIC void wal_cfg80211_set_vap_state(hmac_vap_stru *hmac_vap, uint8_t vap_state, uint8_t last_vap_state)
{
    mac_vap_stru *base_info = &hmac_vap->st_vap_base_info;
    /* 发送失败，处理超时帧的bitmap */
    wal_check_cookie_timeout(g_cookie_array, &g_uc_cookie_array_bitmap, OAL_TIME_JIFFY);

    oam_warning_log1(base_info->uc_vap_id, OAM_SF_CFG,
        "{wal_cfg80211_mgmt_tx::vap status[%d], tx mgmt timeout}\r\n", vap_state);

    if (vap_state == MAC_VAP_STATE_STA_LISTEN) {
        if (base_info->en_p2p_mode != WLAN_LEGACY_VAP_MODE) {
            mac_vap_state_change(base_info, last_vap_state);
        } else {
            mac_vap_state_change(base_info, base_info->en_last_vap_state);
        }

        /* 抛事件到DMAC ，返回监听信道 */
        hmac_p2p_send_listen_expired_to_device(hmac_vap);
    }
}


OAL_STATIC void wal_cfg80211_swithchover_offchan(uint32_t flags, mac_vap_stru *base_info, bool *need_offchan,
    int32_t channel, bool offchan)
{
    if ((offchan == OAL_TRUE) && (flags & WIPHY_FLAG_OFFCHAN_TX)) {
        *need_offchan = OAL_TRUE;
    }
    if ((base_info->en_p2p_mode == WLAN_LEGACY_VAP_MODE) &&
        (base_info->en_vap_state == MAC_VAP_STATE_UP) &&
        (channel == base_info->st_channel.uc_chan_number)) {
        *need_offchan = OAL_FALSE;
    }
}


OAL_STATIC int32_t wal_cfg80211_mgmt_tx(oal_wiphy_stru *wiphy, oal_wireless_dev_stru *wdev,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    struct cfg80211_mgmt_tx_params *params,
#else
    oal_ieee80211_channel *chan_info, bool offchan, uint32_t wait, const uint8_t *buf, size_t len,
    bool no_cck, bool dont_wait_for_ack,
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)) */
    uint64_t *pull_cookie)
{
    oal_net_device_stru *netdev = NULL;
    mac_device_stru *mac_device = NULL;
    mac_vap_stru *mac_vap = NULL;
    const oal_ieee80211_mgmt *mgmt = NULL;
    int32_t ret;
    uint32_t rst;
    hmac_mgmt_frame_stru mgmt_frame;
    uint8_t cookie_idx;
    uint8_t retry;
    mac_p2p_info_stru *p2p_info = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    oal_mgmt_tx_stru *mgmt_tx = NULL;
    oal_wireless_dev_stru *roc_wireless_dev = wdev;
    unsigned long start_time_stamp;
    bool need_offchan = OAL_FALSE;
    uint32_t temp_flag;
    oal_ieee80211_channel chan = { 0 };

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    oal_ieee80211_channel *chan_info;
    const uint8_t *buf;
    oal_size_t len;
    uint32_t wait;
    bool offchan;
    if (params == NULL) {
        return -OAL_EINVAL;
    }
    chan_info = params->chan;
    buf = params->buf;
    len = params->len;
    offchan = params->offchan;
    wait = params->wait;
    /* 1106 FPGA信道切换时间长，需要增加监听时间以保证管理帧发送时间,回片后可删除 */
    wait = wlan_chip_update_cfg80211_mgmt_tx_wait_time(wait);
#endif

    /* 1.1 入参检查 */
    if (oal_any_null_ptr4(wiphy, wdev, pull_cookie, buf)) {
        oam_error_log0(0, 0, "{wal_cfg80211_mgmt_tx::para null:wiphy/wdev/pull_cookie/buf is null}");
        return -OAL_EINVAL;
    }
    rst = wal_cfg80211_set_net_info(wdev, &netdev, &mac_vap, &mac_device, &hmac_vap);
    if (rst != OAL_SUCC) {
        return -OAL_EINVAL;
    }
    p2p_info = &mac_device->st_p2p_info;
    *pull_cookie = p2p_info->ull_send_action_id++; /* cookie值上层调用需要判断是否是这次的发送导致的callback */
    if (*pull_cookie == 0) {
        *pull_cookie = p2p_info->ull_send_action_id++;
    }
    mgmt = (const struct ieee80211_mgmt *)buf;

    temp_flag = oal_ieee80211_is_probe_resp(mgmt->frame_control);
    temp_flag = !hmac_get_feature_switch(HMAC_MIRACAST_SINK_SWITCH) ? temp_flag :
        (temp_flag && (mac_vap->st_probe_resp_ctrl.en_probe_resp_status == MAC_PROBE_RESP_MODE_ACTIVE));
    if (temp_flag) {
        *pull_cookie = 0; /* set cookie default value */
        /* host should not send PROE RESPONSE, device will send immediately when receive probe request packet */
        oal_cfg80211_mgmt_tx_status(wdev, *pull_cookie, buf, len, OAL_TRUE, GFP_KERNEL);
        return OAL_SUCC;
    }
    rst = wal_cfg80211_set_channel_msg(chan_info, mac_vap, mgmt, &chan, len);
    if (rst != OAL_SUCC) {
        return -OAL_EINVAL;
    }
    /* 2.1 消息参数准备 */
    memset_s(&mgmt_frame, sizeof(mgmt_frame), 0, sizeof(mgmt_frame));
    mgmt_frame.channel = oal_ieee80211_frequency_to_channel(chan.center_freq);

    if (WAL_COOKIE_FULL_MASK == g_uc_cookie_array_bitmap) {
        cookie_idx = wal_find_oldest_cookie(g_cookie_array);
        wal_del_cookie_from_array(g_cookie_array, &g_uc_cookie_array_bitmap, cookie_idx);
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx::cookies is [0x%x] after clear}",
            g_uc_cookie_array_bitmap);
    }

    ret = wal_add_cookie_to_array(g_cookie_array, &g_uc_cookie_array_bitmap, pull_cookie, &cookie_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, 0, "{wal_cfg80211_mgmt_tx::Fail to add cooki, ret[%d]!}", ret);
        return -OAL_EINVAL;
    } else {
        mgmt_frame.mgmt_frame_id = cookie_idx;
    }

    mgmt_tx = &(hmac_vap->st_mgmt_tx);
    mgmt_tx->mgmt_tx_complete = OAL_FALSE;
    mgmt_tx->mgmt_tx_status = OAL_FAIL;

    switch (hmac_vap->st_vap_base_info.en_vap_mode) {
        case WLAN_VAP_MODE_BSS_AP:
            if (wal_cfg80211_mgmt_tx_bss_ap_need_offchan(hmac_vap, &mgmt_frame, mac_device,
                roc_wireless_dev, &need_offchan) == -OAL_EINVAL) {
                return -OAL_EINVAL;
            }
            break;
        case WLAN_VAP_MODE_BSS_STA: /* P2P CL DEV */
            wal_cfg80211_swithchover_offchan(wiphy->flags, &hmac_vap->st_vap_base_info, &need_offchan,
                mgmt_frame.channel, offchan);
            break;
        default:
            break;
    }

    if ((need_offchan == OAL_TRUE) && (chan.center_freq == 0)) {
        oam_error_log1(mac_vap->uc_vap_id, 0, "{wal_cfg80211_mgmt_tx::need offchan but chan null} ", offchan);
        return -OAL_EINVAL;
    }

    if (wait == 0) {
        wait = WAL_MGMT_TX_TIMEOUT_MSEC;
        oam_warning_log1(mac_vap->uc_vap_id, 0, "{wal_cfg80211_mgmt_tx::wait time is 0, set to %d ms}", wait);
    }

    oam_warning_log4(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
        "{wal_cfg80211_mgmt_tx::offchan[%d].chan[%d]vap state[%d],wait[%d]}",
        need_offchan, mgmt_frame.channel, hmac_vap->st_vap_base_info.en_vap_state, wait);

    /* 需要offchannel,按照入参切到相应的信道XXms */
    if (need_offchan == OAL_TRUE) {
        ret = wal_drv_remain_on_channel(wiphy, roc_wireless_dev, &chan, wait, pull_cookie, IEEE80211_ROC_TYPE_MGMT_TX);
        if (ret != OAL_SUCC) {
            oam_warning_log4(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                "{wal_cfg80211_mgmt_tx::wal_drv_remain_on_channel[%d]!!!offchannel[%d].channel[%d],vap state[%d]}\r\n",
                ret, need_offchan, mgmt_frame.channel, hmac_vap->st_vap_base_info.en_vap_state);
            oal_cfg80211_mgmt_tx_status(wdev, *pull_cookie, buf, len, OAL_FALSE, GFP_KERNEL);
            return OAL_SUCC;
        }
    }

    start_time_stamp = OAL_TIME_JIFFY;

    retry = 0;
    /* 发送失败，则尝试重传 */
    do {
        ret = wal_mgmt_do_tx(netdev, &mgmt_frame, need_offchan, wait, buf, len);
        retry++;
    } while (oal_value_ne_all2(ret, OAL_SUCC, OAL_ERR_CODE_WRONG_CHANNEL) && (retry <= WAL_MGMT_TX_RETRY_CNT)
             && time_before(OAL_TIME_JIFFY, start_time_stamp + oal_msecs_to_jiffies(wait))); /*lint !e666*/
    // 给第三方发送Device Discoverability Response等帧时，可能会由于对方不再侦听而超时失败。
    if (retry > WAL_MGMT_TX_RETRY_CNT) {
        oam_warning_log3(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
            "{wal_cfg80211_mgmt_tx::retry count[%d]>max[%d],tx status[%d],stop tx mgmt}\r\n",
            retry, WAL_MGMT_TX_RETRY_CNT, ret);
    }

    if (ret != OAL_SUCC) {
        wal_cfg80211_set_vap_state(hmac_vap, mac_vap->en_vap_state, mac_device->st_p2p_info.en_last_vap_state);
        oal_cfg80211_mgmt_tx_status(wdev, *pull_cookie, buf, len, OAL_FALSE, GFP_KERNEL);
    } else {
        /* 正常结束  */
        *pull_cookie = g_cookie_array[mgmt_tx->mgmt_frame_id].ull_cookie;
        wal_del_cookie_from_array(g_cookie_array, &g_uc_cookie_array_bitmap, mgmt_tx->mgmt_frame_id);
        oal_cfg80211_mgmt_tx_status(wdev, *pull_cookie, buf, len, OAL_TRUE, GFP_KERNEL);
    }

    // 给第三方发送Device Discoverability Response等帧时，可能会由于对方不再侦听而超时失败。
    oam_warning_log3(0, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx::tx status[%d], retry cnt[%d]}, delta_time[%d]",
        ret, retry, oal_jiffies_to_msecs(OAL_TIME_JIFFY - start_time_stamp));

    return OAL_SUCC;
}


uint32_t wal_cfg80211_mgmt_tx_status(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    dmac_crx_mgmt_tx_status_stru *pst_mgmt_tx_status_param = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    oal_mgmt_tx_stru *pst_mgmt_tx = NULL;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_mgmt_tx_status::pst_event_mem is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_hmac_vap = mac_res_get_hmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log1(0, OAM_SF_TX, "{wal_cfg80211_mgmt_tx_status::pst_hmac_vap null.vap_id[%d]}",
            pst_event->st_event_hdr.uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mgmt_tx_status_param = (dmac_crx_mgmt_tx_status_stru *)(pst_event->auc_event_data);
    pst_mgmt_tx = &(pst_hmac_vap->st_mgmt_tx);
    pst_mgmt_tx->mgmt_tx_complete = OAL_TRUE;
    pst_mgmt_tx->mgmt_tx_status = pst_mgmt_tx_status_param->uc_dscr_status;
    pst_mgmt_tx->mgmt_frame_id = pst_mgmt_tx_status_param->mgmt_frame_id;

    /* 找不到相应的cookie值，说明已经超时被处理，不需要再唤醒 */
    if (OAL_SUCC == wal_check_cookie_from_array(&g_uc_cookie_array_bitmap, pst_mgmt_tx->mgmt_frame_id)) {
        /* 让编译器优化时保证OAL_WAIT_QUEUE_WAKE_UP在最后执行 */
        oal_smp_mb();
        oal_wait_queue_wake_up_interrupt(&pst_mgmt_tx->st_wait_queue);
    }

    return OAL_SUCC;
}

/* P2P 补充缺失的CFG80211接口 */
void wal_cfg80211_mgmt_frame_register(struct wiphy *wiphy, struct wireless_dev *wdev,
    uint16_t frame_type, bool reg)
{
    if (wiphy == NULL || wdev == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_mgmt_frame_register::wiphy or wdev is null}");
        return;
    }
    return;
}

int32_t wal_cfg80211_set_bitrate_mask(struct wiphy *wiphy,
                                      struct net_device *dev,
                                      const uint8_t *peer,
                                      const struct cfg80211_bitrate_mask *mask)
{
    return OAL_SUCC;
}


OAL_STATIC int32_t wal_cfg80211_start_p2p_device(oal_wiphy_stru *pst_wiphy,
                                                 oal_wireless_dev_stru *pst_wdev)
{
    return -OAL_EFAIL;
}


OAL_STATIC void wal_cfg80211_stop_p2p_device(oal_wiphy_stru *pst_wiphy,
                                             oal_wireless_dev_stru *pst_wdev)
{
    return;
}


static int32_t wal_cfg80211_set_power_mgmt(oal_wiphy_stru *pst_wiphy,
                                           oal_net_device_stru *pst_netdev,
                                           bool enabled, int32_t timeout)
{
    wal_msg_write_stru st_write_msg;
    mac_cfg_ps_open_stru *pst_sta_pm_open = NULL;
    int32_t l_ret;
    mac_vap_stru *pst_mac_vap = NULL;

    if (oal_unlikely(oal_any_null_ptr2(pst_wiphy, pst_netdev))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_set_power_mgmt::pst_wiphy or pst_wdev is null!}");
        return -OAL_EINVAL;
    }

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_STA_PM_ON, sizeof(mac_cfg_ps_open_stru));

    pst_mac_vap = oal_net_dev_priv(pst_netdev);
    if (oal_unlikely(pst_mac_vap == NULL)) {
        oam_warning_log0(0, OAM_SF_PWR, "{wal_cfg80211_set_power_mgmt::get mac vap failed.}");
        return OAL_SUCC;
    }

    /* P2P dev不下发 */
    if (IS_P2P_DEV(pst_mac_vap)) {
        oam_warning_log0(0, OAM_SF_PWR, "wal_cfg80211_set_power_mgmt:vap is p2p dev return");
        return OAL_SUCC;
    }

    oam_warning_log3(0, OAM_SF_PWR, "{wal_cfg80211_set_power_mgmt::vap mode[%d]p2p mode[%d]set pm:[%d]}",
        pst_mac_vap->en_vap_mode, pst_mac_vap->en_p2p_mode, enabled);
    pst_sta_pm_open = (mac_cfg_ps_open_stru *)(st_write_msg.auc_value);
    /* MAC_STA_PM_SWITCH_ON / MAC_STA_PM_SWITCH_OFF */
    pst_sta_pm_open->uc_pm_enable = enabled;
    pst_sta_pm_open->uc_pm_ctrl_type = MAC_STA_PM_CTRL_TYPE_HOST;

    l_ret = wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_ps_open_stru), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_set_power_mgmt::fail to send pm cfg msg, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_11R


OAL_STATIC int32_t wal_cfg80211_update_ft_ies(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
    oal_cfg80211_update_ft_ies_stru *pst_fties)
{
    wal_msg_write_stru st_write_msg;
    mac_cfg80211_ft_ies_stru *pst_mac_ft_ies;
    wal_msg_stru *pst_rsp_msg;
    uint32_t err_code;
    int32_t l_ret;

    if (oal_any_null_ptr3(pst_wiphy, pst_netdev, pst_fties)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_update_ft_ies::param is null.}\r\n");

        return -OAL_EINVAL;
    }

    if ((pst_fties->ie == NULL) || (pst_fties->ie_len == 0) || (pst_fties->ie_len >= MAC_MAX_FTE_LEN)) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_cfg80211_update_ft_ies::unexpect ie or len[%d].}\r\n", pst_fties->ie_len);

        return -OAL_EINVAL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    st_write_msg.en_wid = WLAN_CFGID_SET_FT_IES;
    st_write_msg.us_len = sizeof(mac_cfg80211_ft_ies_stru);

    pst_mac_ft_ies = (mac_cfg80211_ft_ies_stru *)st_write_msg.auc_value;
    pst_mac_ft_ies->us_mdid = pst_fties->md;
    pst_mac_ft_ies->us_len = pst_fties->ie_len;
    if (EOK != memcpy_s(pst_mac_ft_ies->auc_ie, pst_fties->ie_len, pst_fties->ie, pst_fties->ie_len)) {
        oam_error_log0(0, OAM_SF_ANY, "wal_cfg80211_update_ft_ies::memcpy fail!");
        return -OAL_EFAIL;
    }

    l_ret = wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE, \
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg80211_ft_ies_stru), \
        (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY,
            "{wal_cfg80211_update_ft_ies::wal_send_cfg_event: return err code %d!}\r\n", l_ret);
        return -OAL_EFAIL;
    }
    err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_cfg80211_update_ft_ies::wal_check_and_release_msg_resp fail return err code:[%u]!}\r\n",
            err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#endif  // _PRE_WLAN_FEATURE_11R


int32_t wal_cfg80211_dump_survey(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
                                 int32_t l_idx, oal_survey_info_stru *pst_info)
{
    if (wal_cfg80211_dfr_and_s3s4_param_check() == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_dump_survey:: dfr or s3s4 is processing!}");
        return -OAL_EFAIL;
    }

    return hmac_cfg80211_dump_survey(pst_wiphy, pst_netdev, l_idx, pst_info);
}

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
OAL_STATIC void wal_cfg80211_abort_scan(oal_wiphy_stru *pst_wiphy, oal_wireless_dev_stru *pst_wdev)
{
    oal_net_device_stru *pst_netdev;

    /* 1.1 入参检查 */
    if (oal_any_null_ptr2(pst_wiphy, pst_wdev)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_abort_scan::wiphy or wdev is null!}\r\n");
        return;
    }

    pst_netdev = pst_wdev->netdev;
    if (pst_netdev == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_abort_scan::netdev is null!}\r\n");
        return;
    }
    oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_abort_scan::enter!}\r\n");
    wal_force_scan_abort_then_scan_comp(pst_netdev);
    return;
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#ifdef _PRE_WLAN_FEATURE_SAE

OAL_STATIC int32_t wal_cfg80211_external_auth(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
    oal_cfg80211_external_auth_stru *pst_external_auth_params)
{
    uint32_t ret;
    int32_t l_ret;
    hmac_external_auth_req_stru st_ext_auth;

    memset_s(&st_ext_auth, sizeof(st_ext_auth), 0, sizeof(st_ext_auth));
    st_ext_auth.us_status = pst_external_auth_params->status;
    l_ret = memcpy_s(st_ext_auth.auc_bssid, WLAN_MAC_ADDR_LEN, pst_external_auth_params->bssid, WLAN_MAC_ADDR_LEN);
    st_ext_auth.st_ssid.uc_ssid_len = oal_min(pst_external_auth_params->ssid.ssid_len, OAL_IEEE80211_MAX_SSID_LEN);
    l_ret += memcpy_s(st_ext_auth.st_ssid.auc_ssid, OAL_IEEE80211_MAX_SSID_LEN,
                      pst_external_auth_params->ssid.ssid, st_ext_auth.st_ssid.uc_ssid_len);
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_SAE, "wal_cfg80211_external_auth::memcpy fail!");
        return -OAL_EFAIL;
    }

    oam_warning_log4(0, OAM_SF_SAE, "{wal_cfg80211_external_auth::status %d, bssid[%02X:XX:XX:XX:%02X:%02X]}",
        st_ext_auth.us_status, st_ext_auth.auc_bssid[MAC_ADDR_0], st_ext_auth.auc_bssid[MAC_ADDR_4],
        st_ext_auth.auc_bssid[MAC_ADDR_5]);

    ret = wal_cfg80211_do_external_auth(pst_netdev, &st_ext_auth);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_SAE, "{wal_cfg80211_external_auth::do external auth fail. ret %d}", ret);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}
#endif /* _PRE_WLAN_FEATURE_SAE */
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0) */

/* 不同操作系统函数指针结构体方式不同 */
OAL_STATIC oal_cfg80211_ops_stru g_wal_cfg80211_ops = {
    .scan = wal_cfg80211_scan,
    .connect = wal_cfg80211_connect,
    .disconnect = wal_cfg80211_disconnect,
    .add_key = wal_cfg80211_add_key,
    .get_key = wal_cfg80211_get_key,
    .del_key = wal_cfg80211_remove_key,
    .set_default_key = wal_cfg80211_set_default_key,
    .set_default_mgmt_key = wal_cfg80211_set_default_mgmt_key,
    .set_wiphy_params = wal_cfg80211_set_wiphy_params,
    .change_beacon = wal_cfg80211_change_beacon,
    .start_ap = wal_cfg80211_start_ap,
    .stop_ap = wal_cfg80211_stop_ap,
    .change_bss = wal_cfg80211_change_bss,
    .sched_scan_start = wal_cfg80211_sched_scan_start,
    .sched_scan_stop = wal_cfg80211_sched_scan_stop,
    .change_virtual_intf = wal_cfg80211_change_virtual_intf,
    .add_station = wal_cfg80211_add_station,
    .del_station = wal_cfg80211_del_station,
    .change_station = wal_cfg80211_change_station,
    .get_station = wal_cfg80211_get_station,
    .dump_station = wal_cfg80211_dump_station,
    .dump_survey = wal_cfg80211_dump_survey,
    .set_pmksa = wal_cfg80211_set_pmksa,
    .del_pmksa = wal_cfg80211_del_pmksa,
    .flush_pmksa = wal_cfg80211_flush_pmksa,
    .remain_on_channel = wal_cfg80211_remain_on_channel,
    .cancel_remain_on_channel = wal_cfg80211_cancel_remain_on_channel,
    .mgmt_tx = wal_cfg80211_mgmt_tx,
    .mgmt_frame_register = wal_cfg80211_mgmt_frame_register,
    .set_bitrate_mask = wal_cfg80211_set_bitrate_mask,
    .add_virtual_intf = wal_cfg80211_add_virtual_intf,
    .del_virtual_intf = wal_cfg80211_del_virtual_intf,
    .mgmt_tx_cancel_wait = wal_cfg80211_mgmt_tx_cancel_wait,
    .start_p2p_device = wal_cfg80211_start_p2p_device,
    .stop_p2p_device = wal_cfg80211_stop_p2p_device,
    .set_power_mgmt = wal_cfg80211_set_power_mgmt,
#ifdef _PRE_WLAN_FEATURE_11R
    .update_ft_ies = wal_cfg80211_update_ft_ies,
#endif  // _PRE_WLAN_FEATURE_11R
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    .abort_scan = wal_cfg80211_abort_scan,
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)) */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#ifdef _PRE_WLAN_FEATURE_SAE
    .external_auth = wal_cfg80211_external_auth,
#endif /* _PRE_WLAN_FEATURE_SAE */
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0) */
};


void wal_cfg80211_reset_bands(uint8_t uc_dev_id)
{
    int i;

    /*
     * 每次更新国家码,flags都会被修改,且上次修改的值不会被清除,
     * 相当于每次修改的国家码都会生效,因此更新国家需要清除flag标志
     */
    for (i = 0; i < g_st_supported_band_2ghz_info.n_channels; i++) {
        g_st_supported_band_2ghz_info.channels[i].flags = 0;
    }

    if (mac_device_band_is_support(uc_dev_id, MAC_DEVICE_CAP_5G) == OAL_TRUE) {
        for (i = 0; i < g_st_supported_band_5ghz_info.n_channels; i++) {
            g_st_supported_band_5ghz_info.channels[i].flags = 0;
        }
    }
}


void wal_cfg80211_save_bands(uint8_t uc_dev_id)
{
    int i;

    /* 每次更新国家码,flags都会被修改,且上次修改的值不会被清除,
       相当于每次修改的国家码都会生效,因此更新国家需要清除flag标志，
       每次设置国家码flag 后，保存当前设置到orig_flags中
    */
    for (i = 0; i < g_st_supported_band_2ghz_info.n_channels; i++) {
        g_st_supported_band_2ghz_info.channels[i].orig_flags = g_st_supported_band_2ghz_info.channels[i].flags;
    }

    if (mac_device_band_is_support(uc_dev_id, MAC_DEVICE_CAP_5G) == OAL_TRUE) {
        for (i = 0; i < g_st_supported_band_5ghz_info.n_channels; i++) {
            g_st_supported_band_5ghz_info.channels[i].orig_flags = g_st_supported_band_5ghz_info.channels[i].flags;
        }
    }
}

OAL_STATIC int32_t wal_init_dual_wlan_netdev(mac_device_stru *mac_device, oal_wiphy_stru *wiphy,
    uint8_t dev_id)
{
    const char *netdev_name = "wlan1";
    if (!g_wlan_spec_cfg->feature_dual_wlan_is_supported) {
        oal_io_print("{wal_init_dual_wlan_netdev::dual wlan unsupported, Not create hwlan netdev!}");
        return OAL_SUCC;
    }
    return wal_init_wlan_netdev(wiphy, netdev_name);
}

OAL_STATIC uint32_t wal_cfg80211_init_netdev(mac_device_stru *pst_device, oal_wiphy_stru *pst_wiphy,
    uint8_t uc_dev_id)
{
    int32_t l_return;
    int8_t ac_vap_netdev_name[MAC_NET_DEVICE_NAME_LENGTH] = {0};

    oal_io_print("wal_init_wlan_netdev wlan and p2p[%d].\n", uc_dev_id);
    /* 主路辅路的netdev规格:主路wlan0 p2p0,辅路wlan1 p2p1 */
    snprintf_s(ac_vap_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, MAC_NET_DEVICE_NAME_LENGTH - 1, "wlan%d", uc_dev_id);

    l_return = wal_init_wlan_netdev(pst_wiphy, ac_vap_netdev_name);
    if (l_return != OAL_SUCC) {
        oal_io_print("wal_init_wlan_netdev wlan[%d] failed.l_return:%d\n", uc_dev_id, l_return);
        return (uint32_t)l_return;
    }

    snprintf_s(ac_vap_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, MAC_NET_DEVICE_NAME_LENGTH - 1, "p2p%d", uc_dev_id);

    l_return = wal_init_wlan_netdev(pst_wiphy, ac_vap_netdev_name);
    if (l_return != OAL_SUCC) {
        oal_io_print("wal_init_wlan_netdev p2p[%d] failed.l_return:%d\n", uc_dev_id, l_return);

        /* 释放wlan网络设备资源 */
        oal_mem_free_m(oal_netdevice_wdev(pst_device->st_p2p_info.pst_primary_net_device), OAL_TRUE);
        oal_net_unregister_netdev(pst_device->st_p2p_info.pst_primary_net_device);
        return (uint32_t)l_return;
    }
    l_return = wal_init_dual_wlan_netdev(pst_device, pst_wiphy, uc_dev_id);
    if (l_return != OAL_SUCC) {
        oal_io_print("wal_init_dual_wlan_netdev wlan1 failed.l_return:%d\n", l_return);
        oal_mem_free_m(oal_netdevice_wdev(pst_device->st_p2p_info.pst_p2p_net_device), OAL_TRUE);
        oal_net_unregister_netdev(pst_device->st_p2p_info.pst_p2p_net_device);
        oal_mem_free_m(oal_netdevice_wdev(pst_device->st_p2p_info.pst_primary_net_device), OAL_TRUE);
        oal_net_unregister_netdev(pst_device->st_p2p_info.pst_primary_net_device);
        return (uint32_t)l_return;
    }
#ifdef _PRE_WLAN_CHBA_MGMT
    /* 注册chba device */
    snprintf_s(ac_vap_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, MAC_NET_DEVICE_NAME_LENGTH - 1, "chba%d", uc_dev_id);
    l_return = wal_init_chba_netdev(pst_wiphy, ac_vap_netdev_name);
    if (l_return != OAL_SUCC) {
        oal_io_print("wal_init_chba_netdev chba failed.l_return:%d\n", l_return);
        oal_mem_free_m(oal_netdevice_wdev(pst_device->st_p2p_info.pst_p2p_net_device), OAL_TRUE);
        oal_net_unregister_netdev(pst_device->st_p2p_info.pst_p2p_net_device);
        oal_mem_free_m(oal_netdevice_wdev(pst_device->st_p2p_info.pst_primary_net_device), OAL_TRUE);
        oal_net_unregister_netdev(pst_device->st_p2p_info.pst_primary_net_device);
        return (uint32_t)l_return;
    }
#endif
#ifdef _PRE_WLAN_FEATURE_NAN
    /* 注册NAN netdev */
    wal_init_nan_netdev(pst_wiphy);
#endif
    return OAL_SUCC;
}


uint32_t wal_cfg80211_init(void)
{
    uint32_t chip, chip_max_num;
    uint8_t uc_device, uc_dev_id;
    int32_t l_return;
    mac_device_stru *pst_device = NULL;
    mac_board_stru *pst_hmac_board = NULL;
    oal_wiphy_stru *pst_wiphy = NULL;
    mac_wiphy_priv_stru *pst_wiphy_priv = NULL;
    hmac_board_get_instance(&pst_hmac_board);

    chip_max_num = oal_bus_get_chip_num();

    for (chip = 0; chip < chip_max_num; chip++) {
        for (uc_device = 0; uc_device < pst_hmac_board->ast_chip[chip].uc_device_nums; uc_device++) {
            /* 获取device_id */
            uc_dev_id = pst_hmac_board->ast_chip[chip].auc_device_id[uc_device];

            pst_device = mac_res_get_dev(uc_dev_id);
            if (oal_unlikely(pst_device == NULL)) {
                oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_init::mac_res_get_dev,pst_dev null!}\r\n");
                return OAL_FAIL;
            }

            pst_device->pst_wiphy = oal_wiphy_new(&g_wal_cfg80211_ops, sizeof(mac_wiphy_priv_stru));

            if (pst_device->pst_wiphy == NULL) {
                oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_init::oal_wiphy_new failed!}\r\n");
                return OAL_FAIL;
            }

            /* 初始化wiphy 结构体内容 */
            pst_wiphy = pst_device->pst_wiphy;

            pst_wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION) | BIT(NL80211_IFTYPE_AP)
                                         | BIT(NL80211_IFTYPE_P2P_CLIENT) | BIT(NL80211_IFTYPE_P2P_GO)
                                         | BIT(NL80211_IFTYPE_P2P_DEVICE);

            pst_wiphy->iface_combinations = g_sta_p2p_iface_combinations;
            pst_wiphy->n_iface_combinations = oal_array_size(g_sta_p2p_iface_combinations);
            pst_wiphy->mgmt_stypes = g_wal_cfg80211_default_mgmt_stypes;
            pst_wiphy->max_remain_on_channel_duration = 5000; /* 5000ms */
            pst_wiphy->flags |= WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL | WIPHY_FLAG_OFFCHAN_TX;
            pst_wiphy->flags |= WIPHY_FLAG_HAVE_AP_SME; /* 设置GO 能力位 */

            /* 1102注册支持pno调度扫描能力相关信息 */
            pst_wiphy->max_sched_scan_ssids = MAX_PNO_SSID_COUNT;
            pst_wiphy->max_match_sets = MAX_PNO_SSID_COUNT;
            pst_wiphy->max_sched_scan_ie_len = WAL_MAX_SCAN_IE_LEN;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
            pst_wiphy->max_sched_scan_reqs = 1;
#else
            pst_wiphy->flags |= WIPHY_FLAG_SUPPORTS_SCHED_SCAN;
#endif

            pst_wiphy->max_scan_ssids = WLAN_SCAN_REQ_MAX_SSID;

            pst_wiphy->max_scan_ie_len = WAL_MAX_SCAN_IE_LEN;
            pst_wiphy->cipher_suites = g_ast_wlan_supported_cipher_suites;
            pst_wiphy->n_cipher_suites = sizeof(g_ast_wlan_supported_cipher_suites) / sizeof(uint32_t);

            /* 不使能节能 */
            pst_wiphy->flags &= ~WIPHY_FLAG_PS_ON_BY_DEFAULT;

            /* wifi 驱动上报支持FW_ROAM,关联时(cfg80211_connect)候使用bssid_hint 替代bssid。 */
            pst_wiphy->flags |= WIPHY_FLAG_SUPPORTS_FW_ROAM;
#ifdef _PRE_WLAN_FEATURE_DFS_OFFLOAD
            pst_wiphy->flags |= 0x200;
#endif

            /* linux 3.14 版本升级，管制域重新修改 */
            pst_wiphy->regulatory_flags |= REGULATORY_CUSTOM_REG;

#ifdef _PRE_WLAN_FEATURE_SAE
            pst_wiphy->features |= NL80211_FEATURE_SAE; /* 驱动支持SAE 特性 */
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)) // TBD 注册6G频段信息
            pst_wiphy->bands[NL80211_BAND_2GHZ] = &g_st_supported_band_2ghz_info; /* 支持的频带信息 2.4G */
            if (mac_device_band_is_support(uc_dev_id, MAC_DEVICE_CAP_5G) == OAL_TRUE) {
                pst_wiphy->bands[NL80211_BAND_5GHZ] = &g_st_supported_band_5ghz_info; /* 支持的频带信息 5G */
            }
#else
            pst_wiphy->bands[IEEE80211_BAND_2GHZ] = &g_st_supported_band_2ghz_info; /* 支持的频带信息 2.4G */
            if (mac_device_band_is_support(uc_dev_id, MAC_DEVICE_CAP_5G) == OAL_TRUE) {
                pst_wiphy->bands[IEEE80211_BAND_5GHZ] = &g_st_supported_band_5ghz_info; /* 支持的频带信息 5G */
            }
#endif
            pst_wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;

            oal_wiphy_apply_custom_regulatory(pst_wiphy, &g_st_default_regdom);

            wal_cfgvendor_init(pst_wiphy);
            oal_io_print("wiphy_register start.\n");
            l_return = oal_wiphy_register(pst_wiphy);
            if (l_return != 0) {
                oal_wiphy_free(pst_device->pst_wiphy);
                oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_init::oal_wiphy_register failed!}\r\n");
                return (uint32_t)l_return;
            }

            /* P2P add_virtual_intf 传入wiphy 参数，在wiphy priv 指针保存wifi 驱动mac_devie_stru 结构指针 */
            pst_wiphy_priv = (mac_wiphy_priv_stru *)(oal_wiphy_priv(pst_wiphy));
            pst_wiphy_priv->pst_mac_device = pst_device;

            l_return = wal_cfg80211_init_netdev(pst_device, pst_wiphy, uc_dev_id);
            if (l_return != OAL_SUCC)
                return (uint32_t)l_return;
        }
    }

    return OAL_SUCC;
}


void wal_cfg80211_exit(void)
{
    uint32_t chip;
    uint8_t uc_device;
    uint8_t uc_dev_id;
    mac_device_stru *pst_device = NULL;
    uint32_t chip_max_num;
    mac_board_stru *pst_hmac_board = NULL;

    hmac_board_get_instance(&pst_hmac_board);

    chip_max_num = oal_bus_get_chip_num(); /* 这个地方待确定 */

    for (chip = 0; chip < chip_max_num; chip++) {
        for (uc_device = 0; uc_device < pst_hmac_board->ast_chip[chip].uc_device_nums; uc_device++) {
            /* 获取device_id */
            uc_dev_id = pst_hmac_board->ast_chip[chip].auc_device_id[uc_device];

            pst_device = mac_res_get_dev(uc_dev_id);
            if (pst_device == NULL) {
                oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_init::mac_res_get_dev pst_device is null!}\r\n");
                return;
            }
            wal_cfgvendor_deinit(pst_device->pst_wiphy);

            /* 注销注册 wiphy device */
            oal_wiphy_unregister(pst_device->pst_wiphy);

            /* 卸载wiphy device */
            oal_wiphy_free(pst_device->pst_wiphy);
        }
    }

    return;
}


uint32_t wal_cfg80211_init_evt_handle(frw_event_mem_stru *pst_event_mem)
{
    uint32_t ret;

    ret = wal_cfg80211_init();
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{wal_cfg80211_init_evt_handle::wal_cfg80211_init return err code[%d]!}", ret);
    }
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_M2S

uint32_t wal_cfg80211_m2s_status_report(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event;
    dmac_m2s_complete_syn_stru *pst_m2s_switch_comp_status;
    wlan_m2s_mgr_vap_stru *pst_m2s_vap_mgr;
    hmac_vap_stru *pst_hmac_vap;
    uint8_t uc_vap_idx;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{wal_cfg80211_m2s_status_report::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;

    /* 获取配置vap */
    pst_hmac_vap = mac_res_get_hmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_M2S,
            "{wal_cfg80211_m2s_status_report::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_m2s_switch_comp_status = (dmac_m2s_complete_syn_stru *)(pst_event->auc_event_data);

    /* 需要上层提供oal接口，TBD */
    oam_warning_log4(0, OAM_SF_M2S,
        "{wal_cfg80211_m2s_status_report::vap num[%d]device result[%d]state[%d]m2s_mode[%d].}",
        pst_m2s_switch_comp_status->pri_data.mss_result.uc_vap_num,
        pst_m2s_switch_comp_status->pri_data.mss_result.en_m2s_result,
        pst_m2s_switch_comp_status->uc_m2s_state, pst_m2s_switch_comp_status->pri_data.mss_result.uc_m2s_mode);

    for (uc_vap_idx = 0; uc_vap_idx < pst_m2s_switch_comp_status->pri_data.mss_result.uc_vap_num; uc_vap_idx++) {
        pst_m2s_vap_mgr = &(pst_m2s_switch_comp_status->ast_m2s_comp_vap[uc_vap_idx]);

        oam_warning_log3(0, OAM_SF_M2S,
                         "{wal_cfg80211_m2s_status_report::Notify to host, ap index[%d]action type[%d]state[%d].}",
                         uc_vap_idx, pst_m2s_vap_mgr->en_action_type, pst_m2s_vap_mgr->en_m2s_result);

        oam_warning_log4(0, OAM_SF_M2S,
                         "{wal_cfg80211_m2s_status_report::Notify to host, ap addr->%02x:XX:XX:%02x:%02x:%02x.}",
                         pst_m2s_vap_mgr->auc_user_mac_addr[MAC_ADDR_0],
                         pst_m2s_vap_mgr->auc_user_mac_addr[MAC_ADDR_3],
                         pst_m2s_vap_mgr->auc_user_mac_addr[MAC_ADDR_4],
                         pst_m2s_vap_mgr->auc_user_mac_addr[MAC_ADDR_5]);
    }

    if (!pst_hmac_vap->pst_net_device) {
        oam_warning_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_M2S,
            "{wal_cfg80211_m2s_status_report::net dev is null, vap maybe deleted!}");
        return OAL_SUCC;
    }

    /* 上报内核 */
    oal_cfg80211_m2s_status_report(pst_hmac_vap->pst_net_device, GFP_KERNEL,
                                   (uint8_t *)pst_m2s_switch_comp_status, sizeof(dmac_m2s_complete_syn_stru));

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH

uint32_t wal_cfg80211_tas_rssi_access_report(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event;
    hmac_vap_stru *pst_hmac_vap;
    uint8_t uc_vap_idx;
    dmac_tas_rssi_notify_stru *pst_tas_rssi_comp_status;
    int32_t l_tas_state;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_tas_rssi_access_report::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;

    /* 获取配置vap */
    uc_vap_idx = pst_event->st_event_hdr.uc_vap_id;
    pst_hmac_vap = mac_res_get_hmac_vap(uc_vap_idx);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(uc_vap_idx, OAM_SF_ANY, "{wal_cfg80211_tas_rssi_access_report::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_tas_rssi_comp_status = (dmac_tas_rssi_notify_stru *)(pst_event->auc_event_data);

    l_tas_state = board_get_wifi_tas_gpio_state();
    oam_warning_log3(uc_vap_idx, OAM_SF_ANY, "{wal_cfg80211_tas_rssi_access_report::core[%d] c_ant%d_rssi[%d].}",
                     pst_tas_rssi_comp_status->l_core_idx, l_tas_state, pst_tas_rssi_comp_status->l_rssi);

    /* 上报内核 */
    oal_cfg80211_tas_rssi_access_report(pst_hmac_vap->pst_net_device, GFP_KERNEL, (uint8_t *)pst_tas_rssi_comp_status,
                                        sizeof(dmac_tas_rssi_notify_stru));

    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP

uint32_t wal_cfg80211_hid2d_seqnum_report(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event;
    hmac_vap_stru *pst_hmac_vap;
    uint8_t uc_vap_idx;
    uint16_t *pus_seq_num;
    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_hid2d_seqnum_report::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;

    /* 获取配置vap */
    uc_vap_idx = pst_event->st_event_hdr.uc_vap_id;
    pst_hmac_vap = mac_res_get_hmac_vap(uc_vap_idx);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(uc_vap_idx, OAM_SF_ANY, "{wal_cfg80211_hid2d_seqnum_report::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pus_seq_num = (uint16_t *)(pst_event->auc_event_data);

    oam_warning_log1(uc_vap_idx, OAM_SF_ANY, "{wal_cfg80211_hid2d_seqnum_report::seq_num[%d].}", *pus_seq_num);
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    /* 上报内核 */
    oal_cfg80211_hid2d_seqnum_report(pst_hmac_vap->pst_net_device, GFP_KERNEL, (uint8_t *)pus_seq_num,
                                     sizeof(uint16_t));
#endif
    return OAL_SUCC;
}
#endif
