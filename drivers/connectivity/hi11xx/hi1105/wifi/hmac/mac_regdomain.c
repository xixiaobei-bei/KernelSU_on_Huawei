

#include "mac_regdomain.h"
#include "mac_device.h"
#include "securec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_REGDOMAIN_C

const mac_supp_mode_table_stru g_bw_mode_table_2g[] = {
    /* 1  */
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS } },
    /* 2  */
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS } },
    /* 3  */
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS } },
    /* 4  */
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS } },
    /* 5  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_40MINUS } },
    /* 6  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_40MINUS } },
    /* 7  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_40MINUS } },
    /* 8  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_40MINUS } },
    /* 9  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_40MINUS } },
    /* 10 */
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS } },
    /* 11 */
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS } },
    /* 12 */
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS } },
    /* 13 */
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS } },
    /* 14 */
    { 1, { WLAN_BAND_WIDTH_20M } },
};
// see http://support.huawei.com/ecommunity/bbs/10212257.html
const mac_supp_mode_table_stru g_bw_mode_table_5g[] = {

#ifdef _PRE_WLAN_FEATURE_160M
    /* 36  */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSPLUS,
          WLAN_BAND_WIDTH_160PLUSPLUSPLUS
        }
    },
    /* 40  */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSPLUS,
          WLAN_BAND_WIDTH_160MINUSPLUSPLUS
        }
    },
    /* 44  */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSMINUS,
          WLAN_BAND_WIDTH_160PLUSMINUSPLUS
        }
    },
    /* 48  */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSMINUS,
          WLAN_BAND_WIDTH_160MINUSMINUSPLUS
        }
    },
    /* 52  */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSPLUS,
          WLAN_BAND_WIDTH_160PLUSPLUSMINUS
        }
    },
    /* 56  */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSPLUS,
          WLAN_BAND_WIDTH_160MINUSPLUSMINUS
        }
    },
    /* 60  */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSMINUS,
          WLAN_BAND_WIDTH_160PLUSMINUSMINUS
        }
    },
    /* 64  */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSMINUS,
          WLAN_BAND_WIDTH_160MINUSMINUSMINUS
        }
    },

    /* 100 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSPLUS,
          WLAN_BAND_WIDTH_160PLUSPLUSPLUS
        }
    },
    /* 104 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSPLUS,
          WLAN_BAND_WIDTH_160MINUSPLUSPLUS
        }
    },
    /* 108 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSMINUS,
          WLAN_BAND_WIDTH_160PLUSMINUSPLUS
        }
    },
    /* 112 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSMINUS,
          WLAN_BAND_WIDTH_160MINUSMINUSPLUS
        }
    },
    /* 116 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSPLUS,
          WLAN_BAND_WIDTH_160PLUSPLUSMINUS
        }
    },
    /* 120 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSPLUS,
          WLAN_BAND_WIDTH_160MINUSPLUSMINUS
        }
    },
    /* 124 */
    {   4,
        { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSMINUS,
          WLAN_BAND_WIDTH_160PLUSMINUSMINUS
        }
    },
    /* 128 */
    {   4,
        { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSMINUS,
          WLAN_BAND_WIDTH_160MINUSMINUSMINUS
        }
    },
#else
    /* 36  */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSPLUS } },
    /* 40  */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },
    /* 44  */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSMINUS } },
    /* 48  */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS } },
    /* 52  */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSPLUS } },
    /* 56  */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },
    /* 60  */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSMINUS } },
    /* 64  */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS } },

    /* 100 */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSPLUS } },
    /* 104 */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },
    /* 108 */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSMINUS } },
    /* 112 */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS } },
    /* 116 */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSPLUS } },
    /* 120 */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },
    /* 124 */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSMINUS } },
    /* 128 */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS } },
#endif

    /* 132 */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSPLUS } },
    /* 136 */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },
    /* 140 */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSMINUS } },
    /* 144 */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS } },

    /* 149 */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSPLUS } },
    /* 153 */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },
    /* 157 */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSMINUS } },
    /* 161 */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS } },
    /* 165 */ { 1, { WLAN_BAND_WIDTH_20M } },

    /* for JP 4.9G */
    /* 184 */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSPLUS } },
    /* 188 */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },
    /* 192 */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSMINUS } },
    /* 196 */ { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS } },
};


uint8_t mac_regdomain_get_channel_to_bw_mode_idx(uint8_t channel_number)
{
    uint8_t idx = 0;

    if (channel_number == 0) {
        oam_error_log1(0, OAM_SF_ANY,
                       "{mac_regdomain_get_channel_to_bw_mode_idx::unknow channel number=%d",
                       channel_number);
        return idx;
    }

    if (channel_number <= 14) { /* 信道号小于14 */
        idx = channel_number - 1;
    } else {
        if (channel_number <= 64) {    /* 信道号64 */
            idx = (uint8_t)((uint32_t)(channel_number - 36) >> 2);      /* 信道号减36，右移2 [0, 7] */
        } else if (channel_number <= 144) {    /* 信道号144 */
            idx = (uint8_t)((uint32_t)(channel_number - 100) >> 2) + 8; /* 信道号减100，右移2 [8, 19] */
        } else if (channel_number <= 165) {    /* 信道号165 */
            idx = (uint8_t)((uint32_t)(channel_number - 149) >> 2) + 20; /* 信道号减149，右移2 [20, 24] */
        } else if (channel_number <= 196) {    /* 信道号196 */
            idx = (uint8_t)((uint32_t)(channel_number - 184) >> 2) + 25; /* 信道号减184，右移2 [25, 28] */
        } else {
            oam_warning_log1(0, OAM_SF_ANY,
                             "{mac_regdomain_get_channel_to_bw_mode_idx::unknow channel=%d, force idx = chan 36",
                             channel_number);
            idx = 0;
        }
    }

    return idx;
}

OAL_STATIC wlan_channel_bandwidth_enum_uint8 mac_regdomain_get_support_bw_mode_5g(
    const wlan_bw_cap_enum_uint8 cfg_bw, const uint8_t idx)
{
    wlan_channel_bandwidth_enum_uint8 bw_mode = WLAN_BAND_WIDTH_20M;
    if (cfg_bw == WLAN_BW_CAP_40M) {
        /* 针对40M做保护，防止数组越界, 165信道只能20M */
        if (g_bw_mode_table_5g[idx].uc_cnt > WLAN_BW_CAP_40M) {
            bw_mode = g_bw_mode_table_5g[idx].aen_supp_bw[WLAN_BW_CAP_40M]; /* 40M 数组第2列 */
        }
    } else if (cfg_bw == WLAN_BW_CAP_80M) {
        /* 针对80M做保护，防止数组越界, 165信道只能20M */
        if (g_bw_mode_table_5g[idx].uc_cnt > WLAN_BW_CAP_80M) {
            bw_mode = g_bw_mode_table_5g[idx].aen_supp_bw[WLAN_BW_CAP_80M]; /* 80M 数组第3列 */
        }
    } else {
        oam_error_log1(0, OAM_SF_ANY,
            "{mac_regdomain_get_support_bw_mode::5G not support bw_mode=%d, force to 20M", cfg_bw);
    }
    return bw_mode;
}


wlan_channel_bandwidth_enum_uint8 mac_regdomain_get_support_bw_mode(
    wlan_bw_cap_enum_uint8 cfg_bw, uint8_t channel)
{
    uint8_t idx;
    wlan_channel_bandwidth_enum_uint8 bw_mode = WLAN_BAND_WIDTH_20M;

    /*lint -save -e661 */
    if (channel <= 14) { /* 2G信道范围14以内，目前不支持2G查询，直接返回 */
        oam_error_log1(0, OAM_SF_ANY,
            "{mac_regdomain_get_support_bw_mode:: not support 2G!, channel=%d", channel);
        return bw_mode;
    }

    if (cfg_bw == WLAN_BW_CAP_20M) {
        return bw_mode;
    }

    idx = mac_regdomain_get_channel_to_bw_mode_idx(channel);
    /*lint -restore */
    return mac_regdomain_get_support_bw_mode_5g(cfg_bw, idx);
}

/*lint -restore */

uint32_t mac_regdomain_set_country(uint16_t len, uint8_t *param)
{
    mac_cfg_country_stru *country_param;
    mac_regdomain_info_stru *mac_regdom;
    uint8_t rc_num;
    uint32_t size;

    country_param = (mac_cfg_country_stru *)param;

    mac_regdom = (mac_regdomain_info_stru *)country_param->p_mac_regdom;

    /* 获取管制类的个数 */
    rc_num = mac_regdom->uc_regclass_num;

    /* 计算配置命令 */
    size = (uint32_t)(sizeof(mac_regclass_info_stru) * rc_num + MAC_RD_INFO_LEN);
    if (size > sizeof(mac_regdomain_info_stru)) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    /* 更新管制域信息 */
    if (memcpy_s((uint8_t *)&g_st_mac_regdomain, sizeof(g_st_mac_regdomain),
                 (uint8_t *)mac_regdom, size) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "mac_regdomain_set_country::memcpy fail!");
        return OAL_FAIL;
    }

    /* 更新信道的管制域信息 */
    mac_init_channel_list();

    return OAL_SUCC;
}


int8_t *mac_regdomain_get_country(void)
{
    return g_st_mac_regdomain.ac_country;
}

void mac_get_ext_chan_info(uint8_t pri20_channel_idx,
    wlan_channel_bandwidth_enum_uint8 bandwidth,
    mac_channel_list_stru *chan_info)
{
    uint8_t start_idx = pri20_channel_idx;
    uint8_t auc_wlan_band_width_chanels[WLAN_BAND_WIDTH_BUTT - 2][2] = { /* 2维，主次信道索引 */
        { 1, 0 }, { 2, 0 }, { 2, 1 }, { 4, 0 }, { 4, 2 }, { 4, 1 }, { 4, 3 },
#ifdef _PRE_WLAN_FEATURE_160M
        { 8, 0 }, { 8, 4 }, { 8, 2 }, { 8, 6 }, { 8, 1 }, { 8, 5 }, { 8, 3 }, { 8, 7 }
#endif
    };

    if (bandwidth >= WLAN_BAND_WIDTH_BUTT - 2) { /* butt减2转为带宽enum */
        chan_info->channels = 0;
        oam_error_log1(0, OAM_SF_DFS, "{mac_get_ext_chan_info::Invalid bandwidth %d.}", bandwidth);
    } else {
        chan_info->channels = (uint32_t)auc_wlan_band_width_chanels[bandwidth][0];
        start_idx = pri20_channel_idx - auc_wlan_band_width_chanels[bandwidth][1];
    }

    if (chan_info->channels) {
        if (memcpy_s(chan_info->ast_channels,
                     MAC_MAX_20M_SUB_CH * sizeof(mac_freq_channel_map_stru),
                     &g_ast_freq_map_5g[start_idx],
                     chan_info->channels * sizeof(mac_freq_channel_map_stru)) != EOK) {
            oam_error_log0(0, OAM_SF_DFS, "mac_get_ext_chan_info::memcpy fail!");
            return;
        }
    }
}


oal_bool_enum_uint8 mac_is_cover_dfs_channel(uint8_t band,
    wlan_channel_bandwidth_enum_uint8 bandwidth,
    uint8_t channel_num)
{
    mac_channel_list_stru chan_info;
    uint8_t channel_idx = 0xff;
    uint32_t i;

    if (band != MAC_RC_START_FREQ_5) {
        return OAL_FALSE;
    }

    if (mac_get_channel_idx_from_num(band, channel_num, OAL_FALSE, &channel_idx) != OAL_SUCC) {
        return OAL_FALSE;
    }

    mac_get_ext_chan_info(channel_idx, bandwidth, &chan_info);

    for (i = 0; i < chan_info.channels; i++) {
        if (mac_is_ch_in_radar_band(band, chan_info.ast_channels[i].uc_idx)) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}


oal_bool_enum mac_regdomain_channel_is_support_bw(wlan_channel_bandwidth_enum_uint8 cfg_bw,
    uint8_t channel)
{
    uint8_t idx;
    uint8_t bw_loop;
    mac_supp_mode_table_stru supp_mode_table;
    wlan_channel_band_enum_uint8 channel_band;

    if (channel == 0) {
        oam_error_log0(0, OAM_SF_ANY, "{mac_regdomain_channel_is_support_bw::channel not set yet!");
        return OAL_FALSE;
    }

    if (cfg_bw == WLAN_BAND_WIDTH_20M) {
        return OAL_TRUE;
    }

    channel_band = mac_get_band_by_channel_num(channel);
    /*lint -save -e661 */
    idx = mac_regdomain_get_channel_to_bw_mode_idx(channel);
    if ((channel_band == WLAN_BAND_2G) &&
        (idx < sizeof(g_bw_mode_table_2g) / sizeof(g_bw_mode_table_2g[0]))) {
        supp_mode_table = g_bw_mode_table_2g[idx];
    } else if ((channel_band == WLAN_BAND_5G) &&
               (idx < sizeof(g_bw_mode_table_5g) / sizeof(g_bw_mode_table_5g[0]))) {
        supp_mode_table = g_bw_mode_table_5g[idx];
    } else {
        return OAL_FALSE;
    }

    for (bw_loop = 0; bw_loop < supp_mode_table.uc_cnt; bw_loop++) {
        if (cfg_bw == supp_mode_table.aen_supp_bw[bw_loop]) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}


wlan_channel_bandwidth_enum_uint8 mac_regdomain_get_bw_by_channel_bw_cap(uint8_t channel,
    wlan_bw_cap_enum_uint8 bw_cap)
{
    uint8_t idx;
    wlan_channel_bandwidth_enum_uint8 channel_bw = WLAN_BAND_WIDTH_20M;
    idx = mac_regdomain_get_channel_to_bw_mode_idx(channel);

    if (channel <= 14) { /* 14为2G最大信道号 */
        if (idx < (sizeof(g_bw_mode_table_2g) / sizeof(g_bw_mode_table_2g[0]))) {
            channel_bw = g_bw_mode_table_2g[idx].aen_supp_bw[bw_cap];
        }
    } else {
        if (idx < (sizeof(g_bw_mode_table_5g) / sizeof(g_bw_mode_table_5g[0]))) {
            channel_bw = g_bw_mode_table_5g[idx].aen_supp_bw[bw_cap];
        }
    }
    return channel_bw;
}


uint32_t mac_regdomain_get_bw_mode_by_cmd(int8_t *arg, uint8_t channel,
    wlan_channel_bandwidth_enum_uint8 *bw_mode)
{
    uint8_t index;
    wlan_channel_bandwidth_enum_uint8 en_bw_mode = WLAN_BAND_WIDTH_20M;

    /* 根据主信道号获取其对应的索引 */
    index = mac_regdomain_get_channel_to_bw_mode_idx(channel);

    if (oal_strcmp("160", arg) == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{mac_regdomain_get_bw_mode_by_cmd::Not support 160M in HiD2D mode!}");
        return OAL_FAIL;
    } else if (oal_strcmp("80", arg) == 0) {
        if (channel <= 14 || channel == 165) { /* 信道号小于等于为14，或等于165 */
            oam_warning_log0(0, OAM_SF_ANY, "{mac_regdomain_get_bw_mode_by_cmd:::Not support 80M!}");
            return OAL_FAIL;
        }
        en_bw_mode = g_bw_mode_table_5g[index].aen_supp_bw[2]; /* 80M 数组第2列 */
    } else if (oal_strcmp("40", arg) == 0) {
        if (channel == 14 || channel == 165) { /* 信道号为14，或165 */
            oam_warning_log0(0, OAM_SF_ANY, "{mac_regdomain_get_bw_mode_by_cmd:::Not support 40M!}");
            return OAL_FAIL;
        }
        if (channel < 14) { /* 信道号小于14 */
            en_bw_mode = g_bw_mode_table_2g[index].aen_supp_bw[1];
        } else {
            en_bw_mode = g_bw_mode_table_5g[index].aen_supp_bw[1];
        }
    } else if (oal_strcmp("20", arg) == 0) {
        if (channel <= 14) { /* 信道号小于等于14 */
            en_bw_mode = g_bw_mode_table_2g[index].aen_supp_bw[0];
        } else {
            en_bw_mode = g_bw_mode_table_5g[index].aen_supp_bw[0];
        }
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{mac_regdomain_get_bw_mode_by_cmd::invalid bandwidth!}");
        return OAL_FAIL;
    }

    *bw_mode = en_bw_mode;

    return OAL_SUCC;
}

/* 管制域信息全局变量 */
mac_regdomain_info_stru g_st_mac_regdomain;

/* 6G信道号列表，初始化其管制类为无效 */
mac_channel_info_stru g_ast_channel_list_6g[MAC_CHANNEL_FREQ_6_BUTT] = {
    { 1,   MAC_INVALID_RC },
    { 5,   MAC_INVALID_RC },
    { 9,   MAC_INVALID_RC },
    { 13,  MAC_INVALID_RC },
    { 17,  MAC_INVALID_RC },
    { 21,  MAC_INVALID_RC },
    { 25,  MAC_INVALID_RC },
    { 29,  MAC_INVALID_RC },
    { 33,  MAC_INVALID_RC },
    { 37,  MAC_INVALID_RC },
    { 41,  MAC_INVALID_RC },
    { 45,  MAC_INVALID_RC },
    { 49,  MAC_INVALID_RC },
    { 53,  MAC_INVALID_RC },
    { 57,  MAC_INVALID_RC },
    { 61,  MAC_INVALID_RC },
    { 65,  MAC_INVALID_RC },
    { 69,  MAC_INVALID_RC },
    { 73,  MAC_INVALID_RC },
    { 77,  MAC_INVALID_RC },
    { 81,  MAC_INVALID_RC },
    { 85,  MAC_INVALID_RC },
    { 89,  MAC_INVALID_RC },
    { 93,  MAC_INVALID_RC },
    { 97,  MAC_INVALID_RC },
    { 101, MAC_INVALID_RC },
    { 105, MAC_INVALID_RC },
    { 109, MAC_INVALID_RC },
    { 113, MAC_INVALID_RC },
    { 117, MAC_INVALID_RC },
    { 121, MAC_INVALID_RC },
    { 125, MAC_INVALID_RC },
    { 129, MAC_INVALID_RC },
    { 133, MAC_INVALID_RC },
    { 137, MAC_INVALID_RC },
    { 141, MAC_INVALID_RC },
    { 145, MAC_INVALID_RC },
    { 149, MAC_INVALID_RC },
    { 153, MAC_INVALID_RC },
    { 157, MAC_INVALID_RC },
    { 161, MAC_INVALID_RC },
    { 165, MAC_INVALID_RC },
    { 169, MAC_INVALID_RC },
    { 173, MAC_INVALID_RC },
    { 177, MAC_INVALID_RC },
    { 181, MAC_INVALID_RC },
    { 185, MAC_INVALID_RC },
    { 189, MAC_INVALID_RC },
    { 193, MAC_INVALID_RC },
    { 197, MAC_INVALID_RC },
    { 201, MAC_INVALID_RC },
    { 205, MAC_INVALID_RC },
    { 209, MAC_INVALID_RC },
    { 213, MAC_INVALID_RC },
    { 217, MAC_INVALID_RC },
    { 221, MAC_INVALID_RC },
    { 225, MAC_INVALID_RC },
    { 229, MAC_INVALID_RC },
    { 233, MAC_INVALID_RC },
};

/* 6G频段 信道与中心频率映射 */
const mac_freq_channel_map_stru g_ast_freq_map_6g[MAC_CHANNEL_FREQ_6_BUTT] = {
    { 5955, 1,   0 },
    { 5975, 5,   1 },
    { 5995, 9,   2 },
    { 6015, 13,  3 },
    { 6035, 17,  4 },
    { 6055, 21,  5 },
    { 6075, 25,  6 },
    { 6095, 29,  7 },
    { 6115, 33,  8 },
    { 6135, 37,  9 },
    { 6155, 41,  10 },
    { 6175, 45,  11 },
    { 6195, 49,  12 },
    { 6215, 53,  13 },
    { 6235, 57,  14 },
    { 6255, 61,  15 },
    { 6275, 65,  16 },
    { 6295, 69,  17 },
    { 6315, 73,  18 },
    { 6335, 77,  19 },
    { 6355, 81,  20 },
    { 6375, 85,  21 },
    { 6395, 89,  22 },
    { 6415, 93,  23 },
    { 6435, 97,  24 },
    { 6455, 101, 25 },
    { 6475, 105, 26 },
    { 6495, 109, 27 },
    { 6515, 113, 28 },
    { 6535, 117, 29 },
    { 6555, 121, 30 },
    { 6575, 125, 31 },
    { 6595, 129, 32 },
    { 6615, 133, 33 },
    { 6635, 137, 34 },
    { 6655, 141, 35 },
    { 6675, 145, 36 },
    { 6695, 149, 37 },
    { 6715, 153, 38 },
    { 6735, 157, 39 },
    { 6755, 161, 40 },
    { 6775, 165, 41 },
    { 6795, 169, 42 },
    { 6815, 173, 43 },
    { 6835, 177, 44 },
    { 6855, 181, 45 },
    { 6875, 185, 46 },
    { 6895, 189, 47 },
    { 6915, 193, 48 },
    { 6935, 197, 49 },
    { 6955, 201, 50 },
    { 6975, 205, 51 },
    { 6995, 209, 52 },
    { 7015, 213, 53 },
    { 7035, 217, 54 },
    { 7055, 221, 55 },
    { 7075, 225, 56 },
    { 7095, 229, 57 },
    { 7115, 233, 58 },
};

/* 信道号列表，初始化其管制类为无效 */
mac_channel_info_stru g_ast_channel_list_5g[MAC_CHANNEL_FREQ_5_BUTT] = {
    { 36,  MAC_INVALID_RC },
    { 40,  MAC_INVALID_RC },
    { 44,  MAC_INVALID_RC },
    { 48,  MAC_INVALID_RC },
    { 52,  MAC_INVALID_RC },
    { 56,  MAC_INVALID_RC },
    { 60,  MAC_INVALID_RC },
    { 64,  MAC_INVALID_RC },
    { 100, MAC_INVALID_RC },
    { 104, MAC_INVALID_RC },
    { 108, MAC_INVALID_RC },
    { 112, MAC_INVALID_RC },
    { 116, MAC_INVALID_RC },
    { 120, MAC_INVALID_RC },
    { 124, MAC_INVALID_RC },
    { 128, MAC_INVALID_RC },
    { 132, MAC_INVALID_RC },
    { 136, MAC_INVALID_RC },
    { 140, MAC_INVALID_RC },
    { 144, MAC_INVALID_RC },
    { 149, MAC_INVALID_RC },
    { 153, MAC_INVALID_RC },
    { 157, MAC_INVALID_RC },
    { 161, MAC_INVALID_RC },
    { 165, MAC_INVALID_RC },
    { 184, MAC_INVALID_RC },
    { 188, MAC_INVALID_RC },
    { 192, MAC_INVALID_RC },
    { 196, MAC_INVALID_RC },
};

mac_channel_info_stru g_ast_channel_list_2g[MAC_CHANNEL_FREQ_2_BUTT] = {
    { 1,  MAC_INVALID_RC },
    { 2,  MAC_INVALID_RC },
    { 3,  MAC_INVALID_RC },
    { 4,  MAC_INVALID_RC },
    { 5,  MAC_INVALID_RC },
    { 6,  MAC_INVALID_RC },
    { 7,  MAC_INVALID_RC },
    { 8,  MAC_INVALID_RC },
    { 9,  MAC_INVALID_RC },
    { 10, MAC_INVALID_RC },
    { 11, MAC_INVALID_RC },
    { 12, MAC_INVALID_RC },
    { 13, MAC_INVALID_RC },
    { 14, MAC_INVALID_RC },
};

/* 5G频段 信道与中心频率映射 */
const mac_freq_channel_map_stru g_ast_freq_map_5g[MAC_CHANNEL_FREQ_5_BUTT] = {
    { 5180, 36,  0 },
    { 5200, 40,  1 },
    { 5220, 44,  2 },
    { 5240, 48,  3 },
    { 5260, 52,  4 },
    { 5280, 56,  5 },
    { 5300, 60,  6 },
    { 5320, 64,  7 },
    { 5500, 100, 8 },
    { 5520, 104, 9 },
    { 5540, 108, 10 },
    { 5560, 112, 11 },
    { 5580, 116, 12 },
    { 5600, 120, 13 },
    { 5620, 124, 14 },
    { 5640, 128, 15 },
    { 5660, 132, 16 },
    { 5680, 136, 17 },
    { 5700, 140, 18 },
    { 5720, 144, 19 },
    { 5745, 149, 20 },
    { 5765, 153, 21 },
    { 5785, 157, 22 },
    { 5805, 161, 23 },
    { 5825, 165, 24 },
    /* for JP 4.9G */
    { 4920, 184, 25 },
    { 4940, 188, 26 },
    { 4960, 192, 27 },
    { 4980, 196, 28 },
};

/* 2.4G频段 信道与中心频率映射 */
const mac_freq_channel_map_stru g_ast_freq_map_2g[MAC_CHANNEL_FREQ_2_BUTT] = {
    { 2412, 1,  0 },
    { 2417, 2,  1 },
    { 2422, 3,  2 },
    { 2427, 4,  3 },
    { 2432, 5,  4 },
    { 2437, 6,  5 },
    { 2442, 7,  6 },
    { 2447, 8,  7 },
    { 2452, 9,  8 },
    { 2457, 10, 9 },
    { 2462, 11, 10 },
    { 2467, 12, 11 },
    { 2472, 13, 12 },
    { 2484, 14, 13 },
};


void mac_get_regdomain_info(mac_regdomain_info_stru **rd_info)
{
    if (rd_info == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{mac_get_regdomain_info::ppst_rd_info null.}");
        return;
    }
    *rd_info = &g_st_mac_regdomain;

    return;
}


void mac_init_regdomain(void)
{
    int8_t  default_country[] = "99";
    mac_regclass_info_stru *regclass = NULL;
    int32_t ret;

    ret = memcpy_s(g_st_mac_regdomain.ac_country, sizeof(g_st_mac_regdomain.ac_country),
        default_country, sizeof(default_country));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "mac_init_regdomain::memcpy fail!");
        return;
    }

    /* 初始默认的管制类个数为2 */
    g_st_mac_regdomain.uc_regclass_num = 2;

    /* 初始化管制类1 */
    regclass = &(g_st_mac_regdomain.ast_regclass[0]);

    regclass->en_start_freq = MAC_RC_START_FREQ_2;
    regclass->en_ch_spacing = MAC_CH_SPACING_5MHZ;
    regclass->uc_behaviour_bmap = 0;
    regclass->uc_coverage_class = 0;
    regclass->uc_max_reg_tx_pwr = MAC_RC_DEFAULT_MAX_TX_PWR;
    regclass->us_max_tx_pwr = MAC_RC_DEFAULT_MAX_TX_PWR * 10; /* db 转为0.1db乘10 */

    regclass->channel_bmap = MAC_GET_CH_BIT(MAC_CHANNEL1) | MAC_GET_CH_BIT(MAC_CHANNEL2) |
        MAC_GET_CH_BIT(MAC_CHANNEL3) | MAC_GET_CH_BIT(MAC_CHANNEL4) | MAC_GET_CH_BIT(MAC_CHANNEL5) |
        MAC_GET_CH_BIT(MAC_CHANNEL6) | MAC_GET_CH_BIT(MAC_CHANNEL7) | MAC_GET_CH_BIT(MAC_CHANNEL8) |
        MAC_GET_CH_BIT(MAC_CHANNEL9) | MAC_GET_CH_BIT(MAC_CHANNEL10) | MAC_GET_CH_BIT(MAC_CHANNEL11) |
        MAC_GET_CH_BIT(MAC_CHANNEL12) | MAC_GET_CH_BIT(MAC_CHANNEL13);

    /* 初始化管制类2 */
    regclass = &(g_st_mac_regdomain.ast_regclass[1]);

    regclass->en_start_freq = MAC_RC_START_FREQ_5;
    regclass->en_ch_spacing = MAC_CH_SPACING_20MHZ;
    regclass->uc_behaviour_bmap = 0;
    regclass->uc_coverage_class = 0;
    regclass->uc_max_reg_tx_pwr = MAC_RC_DEFAULT_MAX_TX_PWR;
    regclass->us_max_tx_pwr = MAC_RC_DEFAULT_MAX_TX_PWR * 10; /* db 转为0.1db乘10 */

    regclass->channel_bmap = MAC_GET_CH_BIT(MAC_CHANNEL36) | MAC_GET_CH_BIT(MAC_CHANNEL40) |
        MAC_GET_CH_BIT(MAC_CHANNEL44) | MAC_GET_CH_BIT(MAC_CHANNEL48) | MAC_GET_CH_BIT(MAC_CHANNEL52) |
        MAC_GET_CH_BIT(MAC_CHANNEL56) | MAC_GET_CH_BIT(MAC_CHANNEL60) | MAC_GET_CH_BIT(MAC_CHANNEL64) |
        MAC_GET_CH_BIT(MAC_CHANNEL100) | MAC_GET_CH_BIT(MAC_CHANNEL104) | MAC_GET_CH_BIT(MAC_CHANNEL108) |
        MAC_GET_CH_BIT(MAC_CHANNEL112) | MAC_GET_CH_BIT(MAC_CHANNEL116) | MAC_GET_CH_BIT(MAC_CHANNEL120) |
        MAC_GET_CH_BIT(MAC_CHANNEL124) | MAC_GET_CH_BIT(MAC_CHANNEL128) | MAC_GET_CH_BIT(MAC_CHANNEL132) |
        MAC_GET_CH_BIT(MAC_CHANNEL136) | MAC_GET_CH_BIT(MAC_CHANNEL140) | MAC_GET_CH_BIT(MAC_CHANNEL144) |
        MAC_GET_CH_BIT(MAC_CHANNEL149) | MAC_GET_CH_BIT(MAC_CHANNEL153) | MAC_GET_CH_BIT(MAC_CHANNEL157) |
        MAC_GET_CH_BIT(MAC_CHANNEL161) | MAC_GET_CH_BIT(MAC_CHANNEL165) | MAC_GET_CH_BIT(MAC_CHANNEL184) |
        MAC_GET_CH_BIT(MAC_CHANNEL188) | MAC_GET_CH_BIT(MAC_CHANNEL192) | MAC_GET_CH_BIT(MAC_CHANNEL196);
}

#ifdef _PRE_WLAN_FEATURE_11D

uint32_t mac_set_country_ie_2g(mac_regdomain_info_stru *rd_info,
    uint8_t *buffer,
    uint8_t *len)
{
    uint8_t               rc_idx;
    uint8_t               lsb_bit_position;
    mac_regclass_info_stru *reg_class = NULL;
    uint32_t              ret;
    uint8_t               lenth = 0;

    for (rc_idx = 0; rc_idx < rd_info->uc_regclass_num; rc_idx++) {
        /* 获取 Regulatory Class */
        reg_class = &(rd_info->ast_regclass[rc_idx]);

        /* 如果频段不匹配 */
        if (reg_class->en_start_freq != MAC_RC_START_FREQ_2) {
            continue;
        }

        /* 异常检查，信道位图为0表示此管制域没有信道存在，不能少 */
        if (reg_class->channel_bmap == 0) {
            continue;
        }

        /* 获取信道位图的最低一位, 返回0代表bit0置1 */
        lsb_bit_position = oal_bit_find_first_bit_four_byte(reg_class->channel_bmap);

        /* 获取信道号，例如Channel_Map为1100，其对应的索引值为2与3，再由索引值找到信道号 */
        ret = mac_get_channel_num_from_idx(MAC_RC_START_FREQ_2, lsb_bit_position, OAL_FALSE, &buffer[lenth++]);
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_ANY,
                "{mac_set_country_ie_2g::mac_get_channel_num_from_idx failed[%d].}", ret);
            return ret;
        }

        /* 获取信道数 */
        buffer[lenth++] = (uint8_t)oal_bit_get_num_four_byte(reg_class->channel_bmap);

        /* 获取最大功率 */
        buffer[lenth++] = reg_class->uc_max_reg_tx_pwr;
    }

    *len = lenth;
    return OAL_SUCC;
}


uint32_t mac_set_country_ie_5g(mac_regdomain_info_stru *rd_info,
    uint8_t *buffer, uint8_t *len)
{
    mac_regclass_info_stru      *reg_class = NULL;
    mac_country_reg_field_stru  *reg_field = NULL;
    uint8_t                    chan_idx;
    uint8_t                    chan_num = 0;
    uint8_t                    lenth = 0;
    uint32_t                   ret;

    for (chan_idx = 0; chan_idx < MAC_CHANNEL_FREQ_5_BUTT; chan_idx++) {
        reg_class = mac_get_channel_idx_rc_info(MAC_RC_START_FREQ_5, chan_idx, OAL_FALSE);
        if (reg_class == NULL) {
            continue;
        }

        ret = mac_get_channel_num_from_idx(MAC_RC_START_FREQ_5, chan_idx, OAL_FALSE, &chan_num);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{mac_set_country_ie_5g::mac_get_channel_num_from_idx failed[%d].}", ret);
        }
        reg_field = (mac_country_reg_field_stru *)buffer;

        reg_field->uc_first_channel = chan_num;
        reg_field->uc_channel_num = 1;
        reg_field->us_max_tx_pwr = reg_class->uc_max_reg_tx_pwr;

        buffer += MAC_COUNTRY_REG_FIELD_LEN;

        lenth += MAC_COUNTRY_REG_FIELD_LEN;
    }

    *len = lenth;
    return OAL_SUCC;
}

#endif


void mac_init_channel_list(void)
{
    uint8_t                ch_idx;
    uint8_t                rc_num;
    uint8_t                freq;
    uint8_t                rc_idx;
    mac_regdomain_info_stru *rd_info;
    mac_regclass_info_stru  *rc_info = NULL;

    rd_info = &g_st_mac_regdomain;

    /* 先初始化所有信道的管制类为无效 */
    for (ch_idx = 0; ch_idx < MAC_CHANNEL_FREQ_2_BUTT; ch_idx++) {
        g_ast_channel_list_2g[ch_idx].uc_reg_class = MAC_INVALID_RC;
    }

    for (ch_idx = 0; ch_idx < MAC_CHANNEL_FREQ_5_BUTT; ch_idx++) {
        g_ast_channel_list_5g[ch_idx].uc_reg_class = MAC_INVALID_RC;
    }

    for (ch_idx = 0; ch_idx < MAC_CHANNEL_FREQ_6_BUTT; ch_idx++) {
        g_ast_channel_list_6g[ch_idx].uc_reg_class = MAC_INVALID_RC;
    }
    /* 然后根据管制域更新信道的管制类信息 */
    rc_num = rd_info->uc_regclass_num;

    /* 更新2G频段上信道的管制类信息 */
    freq = MAC_RC_START_FREQ_2;

    for (rc_idx = 0; rc_idx < rc_num; rc_idx++) {
        rc_info = &(rd_info->ast_regclass[rc_idx]);

        for (ch_idx = 0; ch_idx < MAC_CHANNEL_FREQ_2_BUTT; ch_idx++) {
            if (mac_is_ch_supp_in_regclass(rc_info, freq, ch_idx) == OAL_TRUE) {
                g_ast_channel_list_2g[ch_idx].uc_reg_class = rc_idx;
            }
        }
    }

    /* 更新5G频段上信道的管制类信息 */
    freq = MAC_RC_START_FREQ_5;

    for (rc_idx = 0; rc_idx < rc_num; rc_idx++) {
        rc_info = &(rd_info->ast_regclass[rc_idx]);

        for (ch_idx = 0; ch_idx < MAC_CHANNEL_FREQ_5_BUTT; ch_idx++) {
            if (mac_is_ch_supp_in_regclass(rc_info, freq, ch_idx) == OAL_TRUE) {
                g_ast_channel_list_5g[ch_idx].uc_reg_class = rc_idx;
            }
        }
        /* 6G频段管制域暂不明确，初始化为非管制 */
        for (ch_idx = 0; ch_idx < MAC_CHANNEL_FREQ_6_BUTT; ch_idx++) {
            g_ast_channel_list_6g[ch_idx].uc_reg_class = rc_idx;
        }
    }
}


uint32_t mac_get_channel_num_from_idx(uint8_t band,
    uint8_t idx,
    oal_bool_enum_uint8 is_6ghz,
    uint8_t *channel_num)
{
    switch (band) {
        case MAC_RC_START_FREQ_2:
            if (idx >= MAC_CHANNEL_FREQ_2_BUTT) {
                return OAL_ERR_CODE_ARRAY_OVERFLOW;
            }

            *channel_num = g_ast_channel_list_2g[idx].uc_chan_number;
            break;
        case MAC_RC_START_FREQ_5:
            if (is_6ghz == OAL_FALSE) {
                if (idx >= MAC_CHANNEL_FREQ_5_BUTT) {
                    return OAL_ERR_CODE_ARRAY_OVERFLOW;
                }

                *channel_num = g_ast_channel_list_5g[idx].uc_chan_number;
            } else {
                if (idx >= MAC_CHANNEL_FREQ_6_BUTT) {
                    return OAL_ERR_CODE_ARRAY_OVERFLOW;
                }

                *channel_num =  g_ast_channel_list_6g[idx].uc_chan_number;
            }
            break;
        default:
            return OAL_ERR_CODE_INVALID_CONFIG;
    }

    return OAL_SUCC;
}


uint32_t mac_get_channel_idx_from_num(uint8_t band,
    uint8_t channel_num,
    oal_bool_enum_uint8 is_6ghz,
    uint8_t *channel_idx)
{
    mac_channel_info_stru  *channel = NULL;
    uint8_t               total_channel_num = 0;
    uint8_t               idx;

    /* 根据频段获取信道信息 */
    switch (band) {
        case MAC_RC_START_FREQ_2:
            channel = g_ast_channel_list_2g;
            total_channel_num = (uint8_t)MAC_CHANNEL_FREQ_2_BUTT;
            break;
        case MAC_RC_START_FREQ_5:
            if (is_6ghz == OAL_FALSE) {
                channel = g_ast_channel_list_5g;
                total_channel_num = (uint8_t)MAC_CHANNEL_FREQ_5_BUTT;
            } else {
                channel = g_ast_channel_list_6g;
                total_channel_num = (uint8_t)MAC_CHANNEL_FREQ_6_BUTT;
            }
            break;
        default:
            return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 检查信道索引号 */
    for (idx = 0; idx < total_channel_num; idx++) {
        if (channel[idx].uc_chan_number == channel_num) {
            *channel_idx = idx;
            return OAL_SUCC;
        }
    }

    return OAL_ERR_CODE_INVALID_CONFIG;
}


uint32_t mac_is_channel_idx_valid(uint8_t band, uint8_t ch_idx, oal_bool_enum_uint8 is_6ghz)
{
    uint8_t               max_ch_idx;
    mac_channel_info_stru  *ch_info = NULL;

    switch (band) {
        case MAC_RC_START_FREQ_2:
            max_ch_idx = MAC_CHANNEL_FREQ_2_BUTT;
            ch_info = &(g_ast_channel_list_2g[ch_idx]);
            break;
        case MAC_RC_START_FREQ_5:
            if (is_6ghz == OAL_FALSE) {
                max_ch_idx = MAC_CHANNEL_FREQ_5_BUTT;
                ch_info = &(g_ast_channel_list_5g[ch_idx]);
            } else {
                max_ch_idx = MAC_CHANNEL_FREQ_6_BUTT;
                ch_info = &(g_ast_channel_list_6g[ch_idx]);
            }
            break;
        default:
            return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (ch_idx >= max_ch_idx) {
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    if (ch_info->uc_reg_class != MAC_INVALID_RC) {
        return OAL_SUCC;
    }

    return OAL_ERR_CODE_INVALID_CONFIG;
}


uint32_t mac_is_channel_num_valid(uint8_t band, uint8_t ch_num, oal_bool_enum_uint8 is_6ghz)
{
    uint8_t  ch_idx;
    uint32_t ret;

    ret = mac_get_channel_idx_from_num(band, ch_num, is_6ghz, &ch_idx);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = mac_is_channel_idx_valid(band, ch_idx, is_6ghz);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return OAL_SUCC;
}


mac_regclass_info_stru *mac_get_channel_idx_rc_info(uint8_t band, uint8_t ch_idx, oal_bool_enum_uint8 is_6ghz)
{
    uint8_t               max_ch_idx;
    mac_channel_info_stru  *ch_info = NULL;

    switch (band) {
        case MAC_RC_START_FREQ_2:
            max_ch_idx = MAC_CHANNEL_FREQ_2_BUTT;
            ch_info = &(g_ast_channel_list_2g[ch_idx]);
            break;
        case MAC_RC_START_FREQ_5:
            if (is_6ghz == OAL_FALSE) {
                max_ch_idx = MAC_CHANNEL_FREQ_5_BUTT;
                ch_info = &(g_ast_channel_list_5g[ch_idx]);
            } else {
                max_ch_idx = MAC_CHANNEL_FREQ_6_BUTT;
                ch_info = &(g_ast_channel_list_6g[ch_idx]);
            }
            break;
        default:
            return NULL;
    }

    if (ch_idx >= max_ch_idx) {
        return NULL;
    }

    if (ch_info->uc_reg_class == MAC_INVALID_RC) {
        return NULL;
    }

    return &(g_st_mac_regdomain.ast_regclass[ch_info->uc_reg_class]);
}


mac_regclass_info_stru *mac_get_channel_num_rc_info(uint8_t band, uint8_t ch_num, oal_bool_enum_uint8 is_6ghz)
{
    uint8_t channel_idx;

    if (mac_get_channel_idx_from_num(band, ch_num, is_6ghz, &channel_idx) != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ANY,
                         "{mac_get_channel_num_rc_info::get_channel_idx_from_num failed. band:%d, ch_num:%d",
                         band, ch_num);

        return NULL;
    }

    return mac_get_channel_idx_rc_info(band, channel_idx, is_6ghz);
}

