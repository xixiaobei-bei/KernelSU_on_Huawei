

#ifndef __MAC_REGDOMAIN_H__
#define __MAC_REGDOMAIN_H__

#include "oal_ext_if.h"
#include "wlan_spec.h"
#include "wlan_types.h"
#include "hd_event.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_REGDOMAIN_H

// 此处不加extern "C" UT编译不过
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define MAC_GET_CH_BIT(val) (1 << (val))

/* 默认管制域最大发送功率 */
#define MAC_RC_DEFAULT_MAX_TX_PWR 30

#define MAC_RD_BMAP_SIZE 32

#define MAC_COUNTRY_SIZE 2 /* 国家码是2个字符 */

#define MAC_INVALID_RC 255 /* 无效的管制类索引值 */

#define MAC_SEC_CHAN_INDEX_OFFSET_START_FREQ_5 1
#define MAC_AFFECTED_CHAN_OFFSET_START_FREQ_5  0
#define MAC_SEC_CHAN_INDEX_OFFSET_START_FREQ_2 4
#define MAC_AFFECTED_CHAN_OFFSET_START_FREQ_2  3

#define MAX_CHANNEL_NUM_FREQ_2G 14 /* 2G频段最大的信道号 */

#define MAC_MAX_20M_SUB_CH 8 /* VHT160中，20MHz信道总个数 */

#ifdef _PRE_WLAN_FEATURE_MBO
#define MAC_GLOBAL_OPERATING_CLASS_115_CH_NUMS 4 /* global operating class 115对应的信道集个数 */
#endif
/* 6GHz频段: 信道号对应的信道索引值 */
typedef enum {
    MAC_CHANNEL6G1 = 0,
    MAC_CHANNEL6G5 = 1,
    MAC_CHANNEL6G9 = 2,
    MAC_CHANNEL6G13 = 3,
    MAC_CHANNEL6G17 = 4,
    MAC_CHANNEL6G21 = 5,
    MAC_CHANNEL6G25 = 6,
    MAC_CHANNEL6G29 = 7,
    MAC_CHANNEL6G33 = 8,
    MAC_CHANNEL6G37 = 9,
    MAC_CHANNEL6G41 = 10,
    MAC_CHANNEL6G45 = 11,
    MAC_CHANNEL6G49 = 12,
    MAC_CHANNEL6G53 = 13,
    MAC_CHANNEL6G57 = 14,
    MAC_CHANNEL6G61 = 15,
    MAC_CHANNEL6G65 = 16,
    MAC_CHANNEL6G69 = 17,
    MAC_CHANNEL6G73 = 18,
    MAC_CHANNEL6G77 = 19,
    MAC_CHANNEL6G81 = 20,
    MAC_CHANNEL6G85 = 21,
    MAC_CHANNEL6G89 = 22,
    MAC_CHANNEL6G93 = 23,
    MAC_CHANNEL6G97 = 24,
    MAC_CHANNEL6G101 = 25,
    MAC_CHANNEL6G105 = 26,
    MAC_CHANNEL6G109 = 27,
    MAC_CHANNEL6G113 = 28,
    MAC_CHANNEL6G117 = 29,
    MAC_CHANNEL6G121 = 30,
    MAC_CHANNEL6G125 = 31,
    MAC_CHANNEL6G129 = 32,
    MAC_CHANNEL6G133 = 33,
    MAC_CHANNEL6G137 = 34,
    MAC_CHANNEL6G141 = 35,
    MAC_CHANNEL6G145 = 36,
    MAC_CHANNEL6G149 = 37,
    MAC_CHANNEL6G153 = 38,
    MAC_CHANNEL6G157 = 39,
    MAC_CHANNEL6G161 = 40,
    MAC_CHANNEL6G165 = 41,
    MAC_CHANNEL6G169 = 42,
    MAC_CHANNEL6G173 = 43,
    MAC_CHANNEL6G177 = 44,
    MAC_CHANNEL6G181 = 45,
    MAC_CHANNEL6G185 = 46,
    MAC_CHANNEL6G189 = 47,
    MAC_CHANNEL6G193 = 48,
    MAC_CHANNEL6G197 = 49,
    MAC_CHANNEL6G201 = 50,
    MAC_CHANNEL6G205 = 51,
    MAC_CHANNEL6G209 = 52,
    MAC_CHANNEL6G213 = 53,
    MAC_CHANNEL6G217 = 54,
    MAC_CHANNEL6G221 = 55,
    MAC_CHANNEL6G225 = 56,
    MAC_CHANNEL6G229 = 57,
    MAC_CHANNEL6G233 = 58,
    MAC_CHANNEL_FREQ_6_BUTT = 59,
} mac_channel_freq_6_enum;

/* 5GHz频段: 信道号对应的信道索引值 */
typedef enum {
    MAC_CHANNEL36 = 0,
    MAC_CHANNEL40 = 1,
    MAC_CHANNEL44 = 2,
    MAC_CHANNEL48 = 3,
    MAC_CHANNEL52 = 4,
    MAC_CHANNEL56 = 5,
    MAC_CHANNEL60 = 6,
    MAC_CHANNEL64 = 7,
    MAC_CHANNEL100 = 8,
    MAC_CHANNEL104 = 9,
    MAC_CHANNEL108 = 10,
    MAC_CHANNEL112 = 11,
    MAC_CHANNEL116 = 12,
    MAC_CHANNEL120 = 13,
    MAC_CHANNEL124 = 14,
    MAC_CHANNEL128 = 15,
    MAC_CHANNEL132 = 16,
    MAC_CHANNEL136 = 17,
    MAC_CHANNEL140 = 18,
    MAC_CHANNEL144 = 19,
    MAC_CHANNEL149 = 20,
    MAC_CHANNEL153 = 21,
    MAC_CHANNEL157 = 22,
    MAC_CHANNEL161 = 23,
    MAC_CHANNEL165 = 24,
    MAC_CHANNEL184 = 25,
    MAC_CHANNEL188 = 26,
    MAC_CHANNEL192 = 27,
    MAC_CHANNEL196 = 28,
    MAC_CHANNEL_FREQ_5_BUTT = 29,
} mac_channel_freq_5_enum;
typedef uint8_t mac_channel_freq_5_enum_uint8;

/* 2.4GHz频段: 信道号对应的信道索引值 */
typedef enum {
    MAC_CHANNEL1 = 0,
    MAC_CHANNEL2 = 1,
    MAC_CHANNEL3 = 2,
    MAC_CHANNEL4 = 3,
    MAC_CHANNEL5 = 4,
    MAC_CHANNEL6 = 5,
    MAC_CHANNEL7 = 6,
    MAC_CHANNEL8 = 7,
    MAC_CHANNEL9 = 8,
    MAC_CHANNEL10 = 9,
    MAC_CHANNEL11 = 10,
    MAC_CHANNEL12 = 11,
    MAC_CHANNEL13 = 12,
    MAC_CHANNEL14 = 13,
    MAC_CHANNEL_FREQ_2_BUTT = 14,
} mac_channel_freq_2_enum;
typedef uint8_t mac_channel_freq_2_enum_uint8;

// note:change alg/acs/acs_cmd_resp.h as while
typedef enum {
    MAC_RC_DFS = BIT0,
    MAC_RC_NO_OUTDOOR = BIT1,
    MAC_RC_NO_INDOOR = BIT2,
} mac_behaviour_bmap_enum;

#define MAC_MAX_SUPP_CHANNEL (uint8_t) MAC_CHANNEL_FREQ_5_BUTT

/* 管制类结构体: 每个管制类保存的信息 */
/* 管制域配置命令结构体 */
typedef struct {
    void *p_mac_regdom;
} mac_cfg_country_stru;

/* channel info结构体 */
typedef struct {
    uint8_t uc_chan_number; /* 信道号 */
    uint8_t uc_reg_class;   /* 管制类在管制域中的索引号 */
    uint8_t auc_resv[2]; /* 预留2字节 */
} mac_channel_info_stru;

typedef struct {
    uint16_t us_freq;  /* 中心频率，单位MHz */
    uint8_t uc_number; /* 信道号 */
    uint8_t uc_idx;    /* 信道索引(软件用) */
} mac_freq_channel_map_stru;

typedef struct {
    uint32_t channels;
    mac_freq_channel_map_stru ast_channels[MAC_MAX_20M_SUB_CH];
} mac_channel_list_stru;

typedef struct {
    uint8_t uc_cnt;
    wlan_channel_bandwidth_enum_uint8 aen_supp_bw[WLAN_BW_CAP_BUTT];
} mac_supp_mode_table_stru;

extern mac_regdomain_info_stru g_st_mac_regdomain;
extern mac_channel_info_stru g_ast_channel_list_5g[];
extern mac_channel_info_stru g_ast_channel_list_2g[];
extern const mac_freq_channel_map_stru g_ast_freq_map_5g[];
extern const mac_freq_channel_map_stru g_ast_freq_map_2g[];

void mac_get_regdomain_info(mac_regdomain_info_stru **rd_info);
void mac_init_regdomain(void);
void mac_init_channel_list(void);
uint32_t mac_get_channel_num_from_idx(uint8_t band, uint8_t idx, oal_bool_enum_uint8 is_6ghz, uint8_t *channel_num);
uint32_t mac_get_channel_idx_from_num(uint8_t band, uint8_t channel_num,
    oal_bool_enum_uint8 is_6ghz, uint8_t *channel_idx);
uint32_t mac_is_channel_idx_valid(uint8_t band, uint8_t ch_idx, oal_bool_enum_uint8 is_6ghz);
uint32_t mac_is_channel_num_valid(uint8_t band, uint8_t ch_num, oal_bool_enum_uint8 is_6ghz);

#ifdef _PRE_WLAN_FEATURE_11D
uint32_t mac_set_country_ie_2g(mac_regdomain_info_stru *rd_info, uint8_t *buffer, uint8_t *len);
uint32_t mac_set_country_ie_5g(mac_regdomain_info_stru *rd_info, uint8_t *buffer, uint8_t *len);
#endif

uint32_t mac_regdomain_set_country(uint16_t len, uint8_t *param);
int8_t *mac_regdomain_get_country(void);
mac_regclass_info_stru *mac_get_channel_idx_rc_info(uint8_t band, uint8_t ch_idx, oal_bool_enum_uint8 is_6ghz);
mac_regclass_info_stru *mac_get_channel_num_rc_info(uint8_t band, uint8_t ch_num, oal_bool_enum_uint8 is_6ghz);
void mac_get_ext_chan_info(uint8_t pri20_channel_idx,
    wlan_channel_bandwidth_enum_uint8 bandwidth, mac_channel_list_stru *chan_info);
oal_bool_enum_uint8 mac_is_cover_dfs_channel(uint8_t band,
    wlan_channel_bandwidth_enum_uint8 bandwidth, uint8_t channel_num);
wlan_channel_bandwidth_enum_uint8 mac_regdomain_get_support_bw_mode(
    wlan_bw_cap_enum_uint8 chan_width, uint8_t channel);
oal_bool_enum mac_regdomain_channel_is_support_bw(wlan_channel_bandwidth_enum_uint8 cfg_bw,
    uint8_t channel);
wlan_channel_bandwidth_enum_uint8 mac_regdomain_get_bw_by_channel_bw_cap(uint8_t channel,
    wlan_bw_cap_enum_uint8 bw_cap);
uint32_t mac_regdomain_get_bw_mode_by_cmd(int8_t *arg, uint8_t channel,
    wlan_channel_bandwidth_enum_uint8 *bw_mode);



OAL_STATIC OAL_INLINE wlan_channel_band_enum_uint8 mac_get_band_by_channel_num(uint8_t channel_num)
{
    return ((channel_num > MAX_CHANNEL_NUM_FREQ_2G) ? WLAN_BAND_5G : WLAN_BAND_2G);
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_ch_supp_in_regclass(mac_regclass_info_stru *rc_info,
    uint8_t freq, uint8_t ch_idx)
{
    if (rc_info == NULL) {
        return OAL_FALSE;
    }

    if (rc_info->en_start_freq != freq) {
        return OAL_FALSE;
    }

    if ((rc_info->channel_bmap & MAC_GET_CH_BIT(ch_idx)) != 0) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


OAL_STATIC OAL_INLINE uint8_t mac_get_num_supp_channel(wlan_channel_band_enum_uint8 band)
{
    switch (band) {
        case WLAN_BAND_2G: /* 2.4GHz */
            return (uint8_t)MAC_CHANNEL_FREQ_2_BUTT;

        case WLAN_BAND_5G: /* 5GHz */
            return (uint8_t)MAC_CHANNEL_FREQ_5_BUTT;

        default:
            return 0;
    }
}

OAL_STATIC OAL_INLINE uint8_t mac_get_sec_ch_idx_offset(wlan_channel_band_enum_uint8 band)
{
    switch (band) {
        case WLAN_BAND_2G: /* 2.4GHz */
            return (uint8_t)MAC_SEC_CHAN_INDEX_OFFSET_START_FREQ_2;

        case WLAN_BAND_5G: /* 5GHz */
            return (uint8_t)MAC_SEC_CHAN_INDEX_OFFSET_START_FREQ_5;

        default:
            return 0;
    }
}

OAL_STATIC OAL_INLINE uint8_t mac_get_affected_ch_idx_offset(wlan_channel_band_enum_uint8 band)
{
    switch (band) {
        case WLAN_BAND_2G: /* 2.4GHz */
            return (uint8_t)MAC_AFFECTED_CHAN_OFFSET_START_FREQ_2;

        case WLAN_BAND_5G: /* 5GHz */
            return (uint8_t)MAC_AFFECTED_CHAN_OFFSET_START_FREQ_5;

        default:
            return 0;
    }
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_rc_dfs_req(mac_regclass_info_stru *rc_info)
{
    if ((rc_info->uc_behaviour_bmap & MAC_RC_DFS) == 0) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_ch_in_radar_band(mac_rc_start_freq_enum_uint8 band,
    uint8_t chan_idx)
{
    mac_regclass_info_stru *rc_info;

    rc_info = mac_get_channel_idx_rc_info(band, chan_idx, OAL_FALSE);
    if (oal_unlikely(rc_info == NULL)) {
        return OAL_FALSE;
    }

    return mac_is_rc_dfs_req(rc_info);
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_dfs_channel(uint8_t band, uint8_t channel_num)
{
    uint8_t channel_idx = 0xff;

    if (mac_get_channel_idx_from_num(band, channel_num, OAL_FALSE, &channel_idx) != OAL_SUCC) {
        return OAL_FALSE;
    }

    if (mac_is_ch_in_radar_band(band, channel_idx) == OAL_FALSE) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of mac_regdomain.h */
