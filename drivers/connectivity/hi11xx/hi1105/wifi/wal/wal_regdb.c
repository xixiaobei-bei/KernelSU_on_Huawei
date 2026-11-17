

#include "wal_regdb.h"
#include "oam_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_REGDB_C

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "wal_regdb_110x.c"

/* 默认管制域信息 */
const oal_ieee80211_regdomain_stru g_st_default_regdom = {
#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY
    .n_reg_rules = 6, // 6个规则
#else
    .n_reg_rules = 4, // 4个规则
#endif
    .alpha2 = "99",
    .reg_rules = {
#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY
        REG_RULE(2402, 2482, 40, 6, 25, 0), // chan:1~13
        REG_RULE(5170, 5250, 160, 6, 25, 0), // chan:36~52
        REG_RULE(5250, 5330, 160, 6, 25, NL80211_RRF_DFS | 0), // chan:52~64 DFS Passive scan
        REG_RULE(5490, 5710, 160, 6, 25, NL80211_RRF_DFS | 0), // chan:100~140 DFS Passive scan
        REG_RULE(5735, 5835, 160, 6, 25, 0), // chan:149~165
        REG_RULE(4910, 4990, 80, 6, 25, 0), // chan:184~196
#else
        /* IEEE 802.11b/g, 信道 1..13 */
        REG_RULE(2412 - 10, 2472 + 10, 40, 6, 25, 0), /*lint !e651*/
        /* 信道 36 - 64 */
        REG_RULE(5150 - 10, 5350 + 10, 160, 6, 25, 0),
        /* 信道 100 - 165 */
        REG_RULE(5470 - 10, 5850 + 10, 160, 6, 25, 0),
        /* IEEE 802.11 信道 184,188,192,196 ，对于日本4.9G */
        REG_RULE(4920 - 10, 4980 + 10, 80, 6, 25, 0),
#endif
    }
};

#elif (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
/* Win32下代码只封装几个国家的管制域信息， 为了UT， */
static const oal_ieee80211_regdomain_stru g_regdom_au = {
    5,
    { 'A', 'U' },
    { 0, 0 },
    {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 3, 24, 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    }
};

static const oal_ieee80211_regdomain_stru g_regdom_at = {
    4,
    { 'A', 'T' },
    { 0, 0 },
    {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
    }
};

static const oal_ieee80211_regdomain_stru g_regdom_cn = {
    2,
    { 'C', 'N' },
    { 0, 0 },
    {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    }
};

static const oal_ieee80211_regdomain_stru g_regdom_jp = {
    7,
    { 'J', 'P' },
    { 0, 0 },
    {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(2474, 2494, 20, 0, 20, NL80211_RRF_NO_OFDM | 0),
        REG_RULE(4910, 4990, 40, 0, 23, 0),
        REG_RULE(5030, 5090, 80, 0, 23, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
    }
};

static const oal_ieee80211_regdomain_stru g_regdom_gb = {
    4,
    { 'G', 'B' },
    { 0, 0 },
    {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 27, NL80211_RRF_DFS | 0),
    }
};

static const oal_ieee80211_regdomain_stru g_regdom_us = {
    6,
    { 'U', 'S' },
    { 0, 0 },
    {
        REG_RULE(2402, 2472, 40, 3, 27, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5600, 80, 3, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5650, 5710, 80, 3, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
};

const oal_ieee80211_regdomain_stru *g_reg_regdb[] = {
    &g_regdom_au,
    &g_regdom_at,
    &g_regdom_cn,
    &g_regdom_jp,
    &g_regdom_gb,
    &g_regdom_us,
};

int g_reg_regdb_size = oal_array_size(g_reg_regdb);

/* 默认管制域信息 */
const oal_ieee80211_regdomain_stru g_st_default_regdom = {
    4,            /* n_reg_rules */
    { '9', '9' }, /* alpha2 */
    { 0, 0 },
    {
        /* reg_rules */
        /* IEEE 802.11b/g, 信道 1..13 */
        REG_RULE(2402, 2482, 40, 6, 20, 0),

        /* 信道 36 - 64 */
        REG_RULE(5150 - 10, 5350 + 10, 40, 6, 20, (0)),

        /* 信道 100 - 165 */
        REG_RULE(5470 - 10, 5850 + 10, 40, 6, 20, (0)),

        /* IEEE 802.11 信道 184,188,192,196 在日本使用 4.9G */
        REG_RULE(4920 - 10, 4980 + 10, 40, 6, 23, (0)),
    },
};

#endif

#ifdef _PRE_WLAN_FEATURE_11D

const oal_ieee80211_regdomain_stru *wal_regdb_find_db(int8_t *str)
{
    int32_t db_idx;

    if (oal_unlikely(str == NULL)) {
        return NULL;
    }

    /* 默认管制域 */
    if ((str[0] == '9') && (str[1] == '9')) {
        return &g_st_default_regdom;
    }

    for (db_idx = 0; db_idx < g_reg_regdb_size; db_idx++) {
        if ((str[0] == g_reg_regdb[db_idx]->alpha2[0]) &&
            (str[1] == g_reg_regdb[db_idx]->alpha2[1])) {
            return g_reg_regdb[db_idx];
        }
    }
    /* 找不到国家码信息时，返回一个特定国家码 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oam_warning_log4(0, OAM_SF_ANY,
                     "{wal_regdb_find_db::regdomain %c,%c was not found,return default regdomain %c,%c.}\r\n",
                     str[0], str[1], g_reg_regdb[HI1103_SPECIFIC_COUNTRY_CODE_IDX]->alpha2[0],
                     g_reg_regdb[HI1103_SPECIFIC_COUNTRY_CODE_IDX]->alpha2[1]);
    return g_reg_regdb[HI1103_SPECIFIC_COUNTRY_CODE_IDX];
#else
    return NULL;
#endif
}
#endif