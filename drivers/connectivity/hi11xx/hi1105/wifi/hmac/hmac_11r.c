
#include "hmac_11r.h"
#include "oal_types.h"
#include "mac_mib.h"
#include "hmac_fsm.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_11R_C

#ifdef _PRE_WLAN_FEATURE_11R

uint32_t hmac_config_set_ft_ies(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    mac_cfg80211_ft_ies_stru *pst_mac_ft_ies;
    oal_app_ie_stru st_ft_ie;
    uint32_t ret;
    uint16_t us_md_id;

    if (oal_any_null_ptr2(pst_mac_vap, puc_param)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_ft_ies::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_ft_ies::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_hmac_vap->bit_11r_enable != OAL_TRUE) {
        return OAL_SUCC;
    }

    pst_mac_ft_ies = (mac_cfg80211_ft_ies_stru *)puc_param;
    ret = mac_mib_get_md_id(pst_mac_vap, &us_md_id);
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_ft_ies::get_md_id fail[%d].}", ret);
        return ret;
    }

    if (us_md_id != pst_mac_ft_ies->us_mdid) {
        oam_error_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_set_ft_ies::UNEXPECTED mdid[%d/%d].}", pst_mac_ft_ies->us_mdid, us_md_id);
        return OAL_FAIL;
    }

    st_ft_ie.en_app_ie_type = OAL_APP_FT_IE;
    st_ft_ie.ie_len = pst_mac_ft_ies->us_len;
    if (EOK != memcpy_s(st_ft_ie.auc_ie, WLAN_WPS_IE_MAX_SIZE, pst_mac_ft_ies->auc_ie, pst_mac_ft_ies->us_len)) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_set_ft_ies::memcpy fail!");
        return OAL_FAIL;
    }
    ret = hmac_config_set_app_ie_to_vap(pst_mac_vap, &st_ft_ie, OAL_APP_FT_IE);
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_ft_ies::set_app_ie FAIL[%d].}", ret);
        return ret;
    }

    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_ft_ies::set_app_ie OK LEN[%d].}",
        pst_mac_ft_ies->us_len);

    hmac_roam_reassoc(pst_hmac_vap);

    return OAL_SUCC;
}


uint32_t hmac_roam_preauth(hmac_vap_stru *pst_hmac_vap)
{
    hmac_roam_info_stru              *pst_roam_info = NULL;
    uint32_t                        ret;
    uint8_t                         uc_vap_id;

    if (pst_hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_preauth::vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    uc_vap_id = pst_hmac_vap->st_vap_base_info.uc_vap_id;
    pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
    if (pst_roam_info == NULL) {
        oam_error_log0(uc_vap_id, OAM_SF_ROAM, "{hmac_roam_preauth::roam info null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    /* 漫游开关没有开时，不处理 */
    if (pst_roam_info->uc_enable == 0) {
        oam_error_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
            "{hmac_roam_preauth::roam disabled.}");
        return OAL_ERR_CODE_ROAM_DISABLED;
    }

    /* 主状态机为非CONNECTING状态，失败 */
    if (pst_roam_info->en_main_state != ROAM_MAIN_STATE_CONNECTING) {
        oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
            "{hmac_roam_preauth::roam_en_main_state=[%d] not ROAM_MAIN_STATE_CONNECTING, return.}",
            pst_roam_info->en_main_state);
        return OAL_ERR_CODE_ROAM_STATE_UNEXPECT;
    }

    /* CONNECT状态机为非WAIT_JOIN状态，失败 */
    if (pst_roam_info->st_connect.en_state != ROAM_CONNECT_STATE_WAIT_PREAUTH_COMP) {
        oam_error_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
            "{hmac_roam_preauth::connect state[%d] error.}", pst_roam_info->st_connect.en_state);
        return OAL_ERR_CODE_ROAM_STATE_UNEXPECT;
    }

    if (pst_hmac_vap->bit_11r_private_preauth != OAL_TRUE) {
        oam_error_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
            "{hmac_roam_preauth::not support preauth.}");
        return OAL_FAIL;
    }

    if (mac_mib_get_ft_trainsistion(&pst_hmac_vap->st_vap_base_info) == OAL_TRUE) {
        oam_warning_log1(uc_vap_id, OAM_SF_ROAM,
            "{hmac_roam_preauth::ft_over_ds=[%d],to send preauth!}",
            mac_mib_get_ft_over_ds(&pst_hmac_vap->st_vap_base_info));
        ret = hmac_roam_connect_ft_preauth(pst_hmac_vap);
        if (ret != OAL_SUCC) {
            oam_warning_log1(uc_vap_id, OAM_SF_ROAM,
                "{hmac_roam_preauth::hmac_roam_connect_ft_preauth FAIL[%d]!}", ret);
            return ret;
        }
    }

    return OAL_SUCC;
}


uint32_t hmac_roam_rx_ft_action(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    dmac_wlan_crx_event_stru st_crx_event;

    memset_s(&st_crx_event, sizeof(dmac_wlan_crx_event_stru), 0, sizeof(dmac_wlan_crx_event_stru));
    st_crx_event.pst_netbuf = pst_netbuf;

    hmac_roam_connect_rx_mgmt(pst_hmac_vap, &st_crx_event);
    return OAL_SUCC;
}


void mac_set_md_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len)
{
    if (OAL_TRUE != mac_mib_get_ft_trainsistion(mac_vap)) {
        *puc_ie_len = 0;
        return;
    }

    /***************************************************************************
                    ---------------------------------------------------
                    |Element ID | Length | MDID| FT Capbility & Policy|
                    ---------------------------------------------------
           Octets:  |1          | 1      | 2   |  1                   |
                    ---------------------------------------------------
    ***************************************************************************/
    *puc_buffer = MAC_EID_MOBILITY_DOMAIN;
    *(puc_buffer + 1) = 3;  /* 长度 3 */
    *(puc_buffer + BYTE_OFFSET_2) = mac_mib_get_ft_mdid(mac_vap) & 0xFF;
    *(puc_buffer + BYTE_OFFSET_3) = (mac_mib_get_ft_mdid(mac_vap) >> BYTE_OFFSET_8) & 0xFF;
    *(puc_buffer + BYTE_OFFSET_4) = 0;

    /***************************************************************************
                    ------------------------------------------
                    |FT over DS| RRP Capability | Reserved   |
                    ------------------------------------------
             Bits:  |1         | 1              | 6          |
                    ------------------------------------------
    ***************************************************************************/
    if (OAL_TRUE == mac_mib_get_ft_over_ds(mac_vap)) {
        *(puc_buffer + BYTE_OFFSET_4) += 1;
    }
    if (OAL_TRUE == mac_mib_get_ft_resource_req(mac_vap)) {
        *(puc_buffer + BYTE_OFFSET_4) += 2; /*  ft resource surport place bit 2 */
    }
    *puc_ie_len = 5; /* 长度 5 */
}


void mac_set_ft_ie(void *pst_vap, uint8_t *puc_buffer, uint16_t *pus_ie_len)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

    if (OAL_TRUE != mac_mib_get_ft_trainsistion(pst_mac_vap)) {
        *pus_ie_len = 0;
        return;
    }

    /*********************************************************************************************
              ------------------------------------------------------------------------------
              |Element ID | Length | MIC Control | MIC | ANonce | SNonce | Optional Params |
              ------------------------------------------------------------------------------
     Octets:  |1          | 1      | 2           |  16 |   32   |   32   |   variable      |
              ------------------------------------------------------------------------------
    ***********************************************************************************************/
    *puc_buffer = MAC_EID_FT;

    *pus_ie_len = 84;  /* 总长度 84 */
}
void mac_set_rde_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len)
{
    /*********************************************************************************************
              -------------------------------------------------------------------
              |Element ID | Length | RDE Identifier | Res Dscr Cnt| Status Code |
              -------------------------------------------------------------------
     Octets:  |1          | 1      | 1              |  1          |   2         |
              -------------------------------------------------------------------
    ***********************************************************************************************/
    *puc_buffer = MAC_EID_RDE;
    *(puc_buffer + 1) = 4;  /* 长度 4 */
    *(puc_buffer + BYTE_OFFSET_2) = 0;
    /* count对应RDE后跟随的RIC descriptor IE个数 */
    *(puc_buffer + BYTE_OFFSET_3) = 0;
    *(puc_buffer + BYTE_OFFSET_4) = 0;
    *(puc_buffer + BYTE_OFFSET_5) = 0;

    *puc_ie_len = 6;  /* 长度 6 */
}

void mac_set_tspec_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len, uint8_t uc_tid)
{
    uint8_t         uc_len = 0;
    mac_ts_info_stru *pst_ts_info;

    /***********************************************************************************************
              --------------------------------------------------------------------------------------
              |Element ID|Length|TS Info|Nominal MSDU Size|Max MSDU Size|Min Serv Itvl|Max Serv Itvl|
              ---------------------------------------------------------------------------------------
     Octets:  |1         | 1(55)| 3     |  2              |   2         |4            |4            |
              ---------------------------------------------------------------------------------------
              | Inactivity Itvl | Suspension Itvl | Serv Start Time |Min Data Rate | Mean Data Rate |
              ---------------------------------------------------------------------------------------
     Octets:  |4                | 4               | 4               |4             |  4             |
              ---------------------------------------------------------------------------------------
              |Peak Data Rate|Burst Size|Delay Bound|Min PHY Rate|Surplus BW Allowance  |Medium Time|
              ---------------------------------------------------------------------------------------
     Octets:  |4             |4         | 4         | 4          |  2                   |2          |
              ---------------------------------------------------------------------------------------

    ***********************************************************************************************/
    *puc_buffer = MAC_EID_TSPEC;
    *(puc_buffer + 1) = 55;  /* 长度 55 */
    uc_len += 2;  /* 偏移 2 长度加元素ID  */

    memset_s(puc_buffer + uc_len, *(puc_buffer + 1), 0, *(puc_buffer + 1));

    pst_ts_info = (mac_ts_info_stru *)(puc_buffer + uc_len);  // TS Info
    uc_len += 3; /* TS Info 3 */

    pst_ts_info->bit_tsid = uc_tid;
    pst_ts_info->bit_direction = 3;  /* bidirection 3 */
    pst_ts_info->bit_apsd = 1;
    pst_ts_info->bit_user_prio = uc_tid;

    *(uint16_t *)(puc_buffer + uc_len) = 0x812c;  // Nominal MSDU Size
    uc_len += 28;  /* MSDU Size 28 */

    *(uint32_t *)(puc_buffer + uc_len) = 0x0001D4C0;  // Mean Data Rate
    uc_len += 16;  /* MSDU Size 16 */

    *(uint16_t *)(puc_buffer + uc_len) = 0x3000;  // Surplus BW Allowance

    *puc_ie_len = 57;  /* 总长度 57 */
}

#endif  // _PRE_WLAN_FEATURE_11R

#ifdef _PRE_WLAN_FEATURE_ADAPTIVE11R

uint32_t mac_ie_get_rsn_akm_tail_pos(uint8_t *ie,
                                     int32_t *akm_tail_offset,
                                     int32_t *akm_cnt_offset)
{
    uint8_t *ie_bak;
    uint16_t suites_count;
    uint16_t idx;
    uint32_t len;

    if (ie == NULL) {
        return OAL_FAIL;
    }

    /*************************************************************************/
    /*                  RSN Element Format                                   */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Version | Group Cipher Suite | Pairwise Suite */
    /* --------------------------------------------------------------------- */
    /* |     1     |    1   |    2    |         4          |       2         */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /*  Count| Pairwise Cipher Suite List | AKM Suite Count | AKM Suite List */
    /* --------------------------------------------------------------------- */
    /*       |         4*m                |     2           |   4*n          */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* |RSN Capabilities|PMKID Count|PMKID List|Group Management Cipher Suite */
    /* --------------------------------------------------------------------- */
    /* |        2       |    2      |   16 *s  |               4           | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    ie_bak = ie;

    ie_bak += BYTE_OFFSET_4;

    /* Group Cipher Suite */
    ie_bak += BYTE_OFFSET_4;

    suites_count = ie_bak[0] + ie_bak[1] * BYTE_OFFSET_8;
    ie_bak += BYTE_OFFSET_2;

    /* Pairwise Cipher Suite 最多存2个 */
    for (idx = 0; idx < suites_count; idx++) {
        ie_bak += BYTE_OFFSET_4;
    }

    len = (uint32_t)(ie_bak - ie);
    if (len > ie[1] + 2) {  /* 总长度 +2 */
        return OAL_FAIL;
    }

    *akm_cnt_offset = (int32_t)(ie_bak - ie);

    suites_count = ie_bak[0] + ie_bak[1] * BYTE_OFFSET_8;
    ie_bak += BYTE_OFFSET_2;

    /* AKM Suite 最多存2个 */
    for (idx = 0; idx < suites_count; idx++) {
        ie_bak += BYTE_OFFSET_4;
    }

    len = (uint32_t)(ie_bak - ie);
    if (len > ie[1] + 2) { /* 2: Element ID + Length */
        return OAL_FAIL;
    }

    *akm_tail_offset = (int32_t)(ie_bak - ie);

    return OAL_SUCC;
}

static uint8_t *hmac_scan_frame_find_rsn_locate(uint8_t **tagged_para, uint32_t *tagged_para_len,
                                                uint8_t *mgmt_frame, uint16_t *frame_len)
{
    uint8_t *rsn_ie = NULL;
    /* 找到 RSN IE 的位置 rsn_ie  */
    *tagged_para = mgmt_frame + MAC_80211_FRAME_LEN +
        (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN);
    *tagged_para_len = *frame_len - MAC_80211_FRAME_LEN -
        (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN);

    rsn_ie = mac_find_ie(MAC_EID_RSN, *tagged_para, *tagged_para_len);
    return rsn_ie;
}

static void hmac_scan_calc_frame_len_after_rsn(uint8_t *mgmt_frame, uint16_t *frame_len, uint8_t *rsn_ie,
                                               uint32_t *rsn_ie_len, uint32_t *rsn_tail_len)
{
    uint32_t frm_len_before_rsn;
    uint32_t frm_len_after_rsn;
    /* 把 netbuf 分为两半: 前面一半是 80211帧头到RSN IE(不含), 后面一半是RSN IE(含)以后的 */
    frm_len_before_rsn = (rsn_ie - mgmt_frame);
    frm_len_after_rsn = *frame_len - frm_len_before_rsn;

    /* RSN IE(长度ul_rsn_ie_len)和 RSN IE后面的内容(长度ul_rsn_tail_len)共同组成后面一半 */
    *rsn_ie_len = rsn_ie[1] + 2; /* 2: EID_RSN + Length */
    *rsn_tail_len = frm_len_after_rsn - *rsn_ie_len;
}

static int32_t hmac_scan_attach_akm_suite_after_rsn(uint8_t *puc_cowork_ie, uint32_t rsn_ie_akm_offset,
                                                    oal_sta_ap_cowork_ie_beacon **cowork_ie,
                                                    int32_t *len, uint8_t *rsn_ie)
{
    oal_sta_ap_cowork_akm_para_stru akm_suite_attatch[4] = { /* 共4种akm suite */
        { HMAC_RSN_AKM_FT_OVER_8021X_VAL,        MAC_RSN_AKM_FT_OVER_8021X },
        { HMAC_RSN_AKM_FT_PSK_VAL,               MAC_RSN_AKM_FT_PSK },
        { HMAC_RSN_AKM_FT_OVER_SAE_VAL,          MAC_RSN_AKM_FT_OVER_SAE },
        { HMAC_RSN_AKM_FT_OVER_8021X_SHA384_VAL, MAC_RSN_AKM_FT_OVER_8021X_SHA384 }
    };
    uint32_t idx;
    int32_t ret;

   /* 附加 AKM SUITE(FT) */
    *cowork_ie = (oal_sta_ap_cowork_ie_beacon *)puc_cowork_ie;
    for (idx = 0; idx < sizeof(akm_suite_attatch) / sizeof(akm_suite_attatch[0]); idx++) {
        if (((*cowork_ie)->capa_bitmap.akm_suite_val & 0xff) == akm_suite_attatch[idx].real_akm_val) {
            ret = memcpy_s(&(rsn_ie[rsn_ie_akm_offset]), sizeof(akm_suite_attatch[idx].akm_suite),
                &(akm_suite_attatch[idx].akm_suite), sizeof(akm_suite_attatch[idx].akm_suite));
            if (ret < 0) {
                return OAL_FAIL;
            }
            *len += ret;
            break;
        } else if (idx == sizeof(akm_suite_attatch) / sizeof(akm_suite_attatch[0]) - 1) {
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

static int32_t hmac_scan_set_mdie_after_rsn_ie(oal_sta_ap_cowork_ie_beacon *cowork_ie,
                                               uint32_t rsn_ie_akm_offset, uint32_t rsn_ie_len,
                                               uint8_t *rsn_ie, uint8_t *rsn_tail)
{
    oal_cowork_md_ie md_ie;
    int32_t rsn_ie_end_pos;
    int32_t ret;
    /* 当前管理帧不携带 MDIE, 则根据互通IE 构造 MDIE 并将其附于 RSN IE之后 */
    memset_s(&md_ie, sizeof(md_ie), 0, sizeof(md_ie));
    md_ie.tag_num = MAC_EID_MOBILITY_DOMAIN;
    md_ie.tag_len = 0x03;
    if (cowork_ie->capa_bitmap.mdid_flag != 0) {
        md_ie.mdid = cowork_ie->capa_bitmap.mdid;
    }
    md_ie.support_res_req_proto = cowork_ie->capa_bitmap.support_res_req_proto;
    md_ie.ft_over_ds = cowork_ie->capa_bitmap.ft_over_ds;

    rsn_ie_end_pos = rsn_ie_akm_offset + BYTE_OFFSET_4;
    ret = memcpy_s(&(rsn_ie[rsn_ie_end_pos]), (rsn_ie_len - rsn_ie_akm_offset),
        rsn_tail, (rsn_ie_len - rsn_ie_akm_offset));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_scan_set_mdie_after_rsn_ie::memcpy fail!");
    }

    rsn_ie_end_pos += (rsn_ie_len - rsn_ie_akm_offset);

    ret = memcpy_s(&rsn_ie[rsn_ie_end_pos], sizeof(md_ie), &md_ie, sizeof(md_ie));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_scan_set_mdie_after_rsn_ie::memcpy fail!");
    }
    rsn_ie_end_pos += sizeof(md_ie);

    return rsn_ie_end_pos;
}


int32_t hmac_scan_attach_akm_suite_to_rsn_ie(uint8_t *mgmt_frame, uint16_t *frame_len)
{
    uint8_t *tagged_para;
    uint32_t tagged_para_len;
    uint8_t *rsn_ie, *puc_cowork_ie, *p_md_ie;
    uint32_t rsn_ie_len, rsn_tail_len;
    uint8_t *rsn_tail, *rsn_ie_bak;
    uint32_t rsn_ie_akm_offset = 0;
    uint32_t rsn_ie_akm_cnt_offset = 0;
    int32_t rsn_ie_end_pos;
    int32_t ret;
    oal_sta_ap_cowork_ie_beacon *pst_cowork_ie;

    if ((((mac_ieee80211_frame_stru *)mgmt_frame)->st_frame_control.bit_sub_type != WLAN_BEACON) &&
        (((mac_ieee80211_frame_stru *)mgmt_frame)->st_frame_control.bit_sub_type != WLAN_PROBE_RSP)) {
        return OAL_SUCC;
    }

    rsn_ie = hmac_scan_frame_find_rsn_locate(&tagged_para, &tagged_para_len, mgmt_frame, frame_len);
    puc_cowork_ie = mac_find_vendor_ie(MAC_WLAN_OUI_HUAWEI,
                                       MAC_WLAN_OUI_TYPE_HAUWEI_COWORK, tagged_para, tagged_para_len);
    if (oal_any_null_ptr2(rsn_ie, puc_cowork_ie)) {
        return OAL_SUCC;
    }

    /* 目前AP侧不会携带dc_roaming IE，sizeof的时候需要减去这个长度 */
    if ((puc_cowork_ie[1] + MAC_IE_HDR_LEN) <
        (sizeof(oal_sta_ap_cowork_ie_beacon) - sizeof(oal_cowork_ie_dc_roaming_stru))) {
        oam_warning_log1(0, OAM_SF_ANY, "hmac_scan_attach_akm_suite_to_rsn_ie::incorrect ie len[%d]", puc_cowork_ie[1]);
        return OAL_SUCC;
    }
    hmac_scan_calc_frame_len_after_rsn(mgmt_frame, frame_len, rsn_ie, &rsn_ie_len, &rsn_tail_len);

    /* 暂存RSN IE后面的内容(长度ul_rsn_tail_len) */
    rsn_ie_bak = rsn_ie;
    ret = mac_ie_get_rsn_akm_tail_pos(rsn_ie_bak, &rsn_ie_akm_offset, &rsn_ie_akm_cnt_offset);
    if (ret != OAL_SUCC) {
        return OAL_FAIL;
    }

    rsn_tail = (int8_t *)kmalloc(rsn_tail_len + (rsn_ie_len - rsn_ie_akm_offset), GFP_KERNEL);
    if (oal_unlikely(rsn_tail == NULL)) {
        return OAL_FAIL;
    }
    ret = memcpy_s(rsn_tail, rsn_tail_len + (rsn_ie_len - rsn_ie_akm_offset),
                   (int8_t *)(rsn_ie + rsn_ie_akm_offset),
                   rsn_tail_len + (rsn_ie_len - rsn_ie_akm_offset));
    if (hmac_scan_attach_akm_suite_after_rsn(puc_cowork_ie, rsn_ie_akm_offset, &pst_cowork_ie,
                                             &ret, rsn_ie) != OAL_SUCC) {
        kfree(rsn_tail);
        return OAL_FAIL;
    }

    /* 更新 AKM COUNT 字段为 2 */
    rsn_ie[rsn_ie_akm_cnt_offset] = 0x02;

    /* 更新 RSN IE 的长度字段 加4 */
    rsn_ie[1] += 4;

    p_md_ie = mac_find_ie(MAC_EID_MOBILITY_DOMAIN, tagged_para, tagged_para_len);
    if (p_md_ie != NULL) {
        /* 把此前保存到 rsn_tail 中的内容拷贝到附加的 AKM SUITE 之后 */
        ret += memcpy_s(&(rsn_ie[rsn_ie_akm_offset + BYTE_OFFSET_4]),
                        rsn_tail_len + (rsn_ie_len - rsn_ie_akm_offset),
                        rsn_tail, rsn_tail_len + (rsn_ie_len - rsn_ie_akm_offset));

        /* 更新代码中维护的管理帧的长度字段 加4 */
        (*frame_len) += 4;

        kfree(rsn_tail);
        return OAL_SUCC;
    }

    rsn_ie_end_pos = hmac_scan_set_mdie_after_rsn_ie(pst_cowork_ie, rsn_ie_akm_offset, rsn_ie_len,
                                                     rsn_ie, rsn_tail);

    ret += memcpy_s(&(rsn_ie[rsn_ie_end_pos]), rsn_tail_len,
                    rsn_tail + (rsn_ie_len - rsn_ie_akm_offset), rsn_tail_len);
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_scan_attach_akm_suite_to_rsn_ie::memcpy fail!");
        kfree(rsn_tail);
        return OAL_FAIL;
    }

    /* 更新管理帧的长度字段 加9 (4+5) */
    (*frame_len) += 9;
    kfree(rsn_tail);

    return OAL_SUCC;
}


uint8_t hmac_scan_extend_mgmt_len_needed(oal_netbuf_stru *netbuf, uint16_t netbuf_len)
{
    uint8_t *mgmt_frame = (uint8_t *)oal_netbuf_data(netbuf);
    uint16_t frame_len;
    uint16_t len_offset = MAC_80211_FRAME_LEN + MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    const uint8_t akm_val = DEFAULT_AKM_VALUE;
    uint8_t *tagged_para;
    uint32_t tagged_para_len;
    uint8_t *rsn_ie;
    uint8_t *rsn_ie_bak;
    uint8_t *md_ie;
    uint8_t *puc_cowork_ie;
    uint32_t idx;
    uint32_t rsn_ie_akm_offset = 0;
    uint32_t rsn_ie_akm_cnt_offset = 0;
    uint32_t ret;
    oal_sta_ap_cowork_ie_beacon *pst_cowork_ie;
    oal_sta_ap_cowork_akm_para_stru akm_suite_attatch[4] = { /* 共4种akm suite */
        { HMAC_RSN_AKM_FT_OVER_8021X_VAL,        MAC_RSN_AKM_FT_OVER_8021X },
        { HMAC_RSN_AKM_FT_PSK_VAL,               MAC_RSN_AKM_FT_PSK },
        { HMAC_RSN_AKM_FT_OVER_SAE_VAL,          MAC_RSN_AKM_FT_OVER_SAE },
        { HMAC_RSN_AKM_FT_OVER_8021X_SHA384_VAL, MAC_RSN_AKM_FT_OVER_8021X_SHA384 }
    };

    if (netbuf_len < (len_offset + sizeof(mac_scanned_result_extend_info_stru))) {
        oam_warning_log1(0, OAM_SF_SCAN, "{hmac_scan_extend_mgmt_len_needed:: netbuf len[%d] err.}", netbuf_len);
        return akm_val;
    }

    frame_len = netbuf_len - sizeof(mac_scanned_result_extend_info_stru);
    tagged_para = mgmt_frame + len_offset;
    tagged_para_len = frame_len - len_offset;

    puc_cowork_ie = mac_find_vendor_ie(MAC_WLAN_OUI_HUAWEI,
                                       MAC_WLAN_OUI_TYPE_HAUWEI_COWORK, tagged_para, tagged_para_len);
    rsn_ie = mac_find_ie(MAC_EID_RSN, tagged_para, tagged_para_len);
    md_ie = mac_find_ie(MAC_EID_MOBILITY_DOMAIN, tagged_para, tagged_para_len);

    /* 条件之一: 必须携带互通IE和RSN IE */
    if ((puc_cowork_ie == NULL) || (rsn_ie == NULL)) {
        return akm_val;
    }

    /* 条件之二: 互通IE的 Adaptive 11r字段必须使能 */
    pst_cowork_ie = (oal_sta_ap_cowork_ie_beacon *)puc_cowork_ie;
    // 目前AP侧不会携带dc_roaming IE，sizeof的时候需要减去这个长度
    if (puc_cowork_ie[1] + MAC_IE_HDR_LEN <
        (sizeof(oal_sta_ap_cowork_ie_beacon) - sizeof(oal_cowork_ie_dc_roaming_stru))) {
        oam_error_log1(0, OAM_SF_ANY, "hmac_scan_extend_mgmt_len_needed::cowork ie len[%d] fail!", puc_cowork_ie[1]);
        return akm_val;
    }
    if (pst_cowork_ie->capa_bitmap.support_adaptive_11r == 0) {
        return akm_val;
    }

    /* 条件之三: RSN IE的 AKM SUITE COUNT 必须为 1 */
    rsn_ie_bak = rsn_ie;
    ret = mac_ie_get_rsn_akm_tail_pos(rsn_ie_bak, &rsn_ie_akm_offset, &rsn_ie_akm_cnt_offset);
    if ((ret != OAL_SUCC) || (rsn_ie[rsn_ie_akm_cnt_offset] != 0x01) ||
        (rsn_ie[rsn_ie_akm_cnt_offset + 1] != 0x00)) {
        return akm_val;
    }

    /* 条件之四: RSN IE的AKM SUITE必须不支持11R */
    for (idx = 0; idx < sizeof(akm_suite_attatch) / sizeof(akm_suite_attatch[0]); idx++) {
        if (akm_suite_attatch[idx].akm_suite == CIPHER_SUITE_SELECTOR(rsn_ie[rsn_ie_akm_offset - BYTE_OFFSET_4],
            rsn_ie[rsn_ie_akm_offset - BYTE_OFFSET_3], rsn_ie[rsn_ie_akm_offset - BYTE_OFFSET_2],
            rsn_ie[rsn_ie_akm_offset - BYTE_OFFSET_1])) {
            return akm_val;
        }
    }

    /* 条件之五: 互通IE里面携带的 cowork_ie.capa_bitmap.akm_suite_val必须支持FT */
    for (idx = 0; idx < sizeof(akm_suite_attatch) / sizeof(akm_suite_attatch[0]); idx++) {
        if ((pst_cowork_ie->capa_bitmap.akm_suite_val & 0xff) == akm_suite_attatch[idx].real_akm_val) {
            /* 如果管理帧携带 MDIE, 则只需扩展 AKM Suite 的长度 4; 否则需扩展 (AKM Suite + MDIE) 的长度 */
            return ((md_ie == NULL) ? (4 + sizeof(oal_cowork_md_ie)) : 4);
        }
    }

    return akm_val;
}
#endif

