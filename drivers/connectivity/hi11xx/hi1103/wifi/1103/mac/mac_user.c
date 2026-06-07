


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oam_ext_if.h"
#include "wlan_spec.h"
#include "mac_resource.h"
#include "mac_device.h"
#include "mac_user.h"
#include "securec.h"
#include "securectype.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_USER_C
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

mac_user_rom_stru  g_mac_user_rom[MAC_RES_MAX_USER_LIMIT];

#if (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1103_DEV)

oal_uint32 mac_user_update_ap_bandwidth_cap(mac_user_stru *pst_mac_user)
{
    mac_user_ht_hdl_stru    *pst_mac_ht_hdl = OAL_PTR_NULL;
    mac_vht_hdl_stru        *pst_mac_vht_hdl = OAL_PTR_NULL;

    if (pst_mac_user == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取HT和VHT结构体指针 */
    pst_mac_ht_hdl = &(pst_mac_user->st_ht_hdl);
    pst_mac_vht_hdl = &(pst_mac_user->st_vht_hdl);

    pst_mac_user->en_bandwidth_cap = WLAN_BW_CAP_20M;

    if (pst_mac_ht_hdl->en_ht_capable == OAL_TRUE) {
        if (pst_mac_ht_hdl->bit_secondary_chan_offset != MAC_SCN) {
            pst_mac_user->en_bandwidth_cap = WLAN_BW_CAP_40M;
        }
    }

    if (pst_mac_vht_hdl->en_vht_capable == OAL_TRUE) {
        /* en_channel_width的取值:VHT opern ie，0 -- 20/40M, 1 -- 80/160M, 同时兼容已废弃的2和3 */
        /* bit_supported_channel_width: VHT capabilities,0--非160,1--160M, 2--160 and 80+80 */
        if (pst_mac_vht_hdl->en_channel_width == WLAN_MIB_VHT_OP_WIDTH_80) {
            if ((0 == pst_mac_vht_hdl->bit_supported_channel_width) && (0 != pst_mac_vht_hdl->bit_extend_nss_bw_supp)) {
                pst_mac_user->en_bandwidth_cap =
                (pst_mac_ht_hdl->uc_chan_center_freq_seg2 != 0) ? WLAN_BW_CAP_160M : WLAN_BW_CAP_80M;
            } else {
                pst_mac_user->en_bandwidth_cap =
                (pst_mac_vht_hdl->uc_channel_center_freq_seg1 != 0) ? WLAN_BW_CAP_160M : WLAN_BW_CAP_80M;
            }
        } else if (pst_mac_vht_hdl->en_channel_width == WLAN_MIB_VHT_OP_WIDTH_160) {
            pst_mac_user->en_bandwidth_cap = WLAN_BW_CAP_160M;
        } else if (pst_mac_vht_hdl->en_channel_width == WLAN_MIB_VHT_OP_WIDTH_80PLUS80) {
            pst_mac_user->en_bandwidth_cap = WLAN_BW_CAP_80M;
        }
    }

    return OAL_SUCC;
}
#endif

#if (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1105_DEV)

oal_void mac_user_set_num_spatial_stream_160M(mac_user_stru *pst_mac_user, oal_uint8 uc_value)
{
    pst_mac_user->st_vht_hdl.bit_user_num_spatial_stream_160M = uc_value;
}


oal_uint8 mac_user_get_sta_cap_bandwidth_11ac(wlan_channel_band_enum_uint8 en_band, mac_user_ht_hdl_stru *pst_mac_ht_hdl,
                                                         mac_vht_hdl_stru *pst_mac_vht_hdl, mac_user_stru *pst_mac_user)
{
    wlan_bw_cap_enum_uint8        en_bandwidth_cap = WLAN_BW_CAP_20M;

    /* 2.4g band不应根据vht cap获取带宽信息 */
    if((en_band == WLAN_BAND_2G) && (pst_mac_ht_hdl->en_ht_capable == OAL_TRUE)) {
        en_bandwidth_cap = (pst_mac_ht_hdl->bit_supported_channel_width == WLAN_BW_CAP_40M) ?     \
                            WLAN_BW_CAP_40M : WLAN_BW_CAP_20M;
    } else {
        if (pst_mac_vht_hdl->bit_supported_channel_width == 0) {
            if ((pst_mac_vht_hdl->bit_extend_nss_bw_supp == WLAN_EXTEND_NSS_BW_SUPP0) ||
                (pst_mac_user->en_user_num_spatial_stream == WLAN_SINGLE_NSS)) {
                en_bandwidth_cap = WLAN_BW_CAP_80M;
            } else {
                en_bandwidth_cap = WLAN_BW_CAP_160M;
            }
        } else {
            en_bandwidth_cap = WLAN_BW_CAP_160M;
        }
    }
    return en_bandwidth_cap;
}

#endif


oal_uint32  mac_user_init_rom(mac_user_stru  *pst_mac_user, oal_uint16      us_user_idx)
{

    if (us_user_idx < MAC_RES_MAX_USER_LIMIT)
    {
        pst_mac_user->_rom = &g_mac_user_rom[us_user_idx];
        memset_s(pst_mac_user->_rom, OAL_SIZEOF(mac_user_rom_stru), 0, OAL_SIZEOF(mac_user_rom_stru));
    }
    mac_user_set_num_spatial_stream_160M(pst_mac_user, WLAN_NSS_LIMIT);
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11AX

oal_void mac_user_set_he_capable(mac_user_stru *pst_mac_user, oal_bool_enum_uint8 en_he_capable)
{

    mac_he_hdl_stru   *pst_he_hdl = OAL_PTR_NULL;

    if (OAL_PTR_NULL != pst_mac_user->_rom)
    {
        pst_he_hdl   = MAC_USER_HE_HDL_STRU(pst_mac_user);
        pst_he_hdl->en_he_capable = en_he_capable;
    }
}


oal_bool_enum_uint8 mac_user_get_he_capable(mac_user_stru *pst_mac_user)
{
    mac_he_hdl_stru   *pst_he_hdl = OAL_PTR_NULL;

    if (OAL_PTR_NULL != pst_mac_user->_rom)
    {
        pst_he_hdl   = MAC_USER_HE_HDL_STRU(pst_mac_user);
        return pst_he_hdl->en_he_capable;
    }

    return OAL_FALSE;
}


oal_void mac_user_get_he_hdl(mac_user_stru *pst_mac_user, mac_he_hdl_stru *pst_he_hdl)
{
    if (EOK != memcpy_s((oal_uint8 *)pst_he_hdl, OAL_SIZEOF(mac_he_hdl_stru),
                        (oal_uint8 *)(MAC_USER_HE_HDL_STRU(pst_mac_user)), OAL_SIZEOF(mac_he_hdl_stru))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "mac_user_get_he_hdl::memcpy fail!");
    }
}


oal_void mac_user_set_he_hdl(mac_user_stru *pst_mac_user, mac_he_hdl_stru *pst_he_hdl)
{
   if (EOK != memcpy_s((oal_uint8 *)(MAC_USER_HE_HDL_STRU(pst_mac_user)), OAL_SIZEOF(mac_he_hdl_stru), (oal_uint8 *)pst_he_hdl, OAL_SIZEOF(mac_he_hdl_stru))) {
       OAM_ERROR_LOG0(0, OAM_SF_ANY, "mac_user_set_he_hdl::memcpy fail!");
   }
}

#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

