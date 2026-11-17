

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
/*
 * 以下管制域数据库由工具生成:
 * Step 1: 从http://wireless.kernel.org/download/wireless-regdb/regulatory.bins/
 *         下载最新的管制域二进制文件regulatory.bin
 * Step 2: regdbdump regulatory.bin >> db.txt
 * Step 3: kernel/net/wireless/genregdb.awk db.txt >> wal_regdb.c
 */
/* 根据WIFI-2.4G-5G-国家码信道对应表更新管制域信息 */
#ifdef _PRE_WLAN_FEATURE_11D
#include "wal_regdb.h"

/* 某些产品上层无法获取国家码，下发的特定国家码 */
static const struct ieee80211_regdomain g_regdom_AA = {
    .alpha2 = "AA",
    .reg_rules = {
        REG_RULE(2402, 2472, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 17, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 160, 0, 23, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_AE = {
    .alpha2 = "AE",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 23, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_AF = {
    .alpha2 = "AF",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 27, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

/*
 * not restrict in CRDA
 * use world regulatory domain's max_eirp: 20
 * REF:
 * https://git.kernel.org/cgit/linux/kernel/git/linville/wireless-regdb.git/tree/db.txt
 */
static const struct ieee80211_regdomain g_regdom_AG = {
    .alpha2 = "AG",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 20, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_AI = {
    .alpha2 = "AI",
    .reg_rules = {
        REG_RULE(2402, 2472, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 27, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 27, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_AL = {
    .alpha2 = "AL",
    .reg_rules = {
        REG_RULE(2402, 2482, 20, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 27, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_AM = {
    .alpha2 = "AM",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 18, 0),
        REG_RULE(5250, 5330, 80, 0, 18, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_3_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_AN = {
    .alpha2 = "AN",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 27, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 27, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

/*
 * not restrict in CRDA
 * use world regulatory domain's max_eirp: 20
 * REF:
 * https://git.kernel.org/cgit/linux/kernel/git/linville/wireless-regdb.git/tree/db.txt
 */
static const struct ieee80211_regdomain g_regdom_AO = {
    .alpha2 = "AO",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 20, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_AR = {
    .alpha2 = "AR",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 3, 24, 0),
        REG_RULE(5250, 5330, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_AS = {
    .alpha2 = "AS",
    .reg_rules = {
        REG_RULE(2402, 2472, 40, 0, 30, 0),
        REG_RULE(5170, 5250, 80, 0, 24, 0),
        REG_RULE(5250, 5330, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_AT = {
    .alpha2 = "AT",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_AU = {
    .alpha2 = "AU",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5590, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5650, 5730, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_6_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_AW = {
    .alpha2 = "AW",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 27, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 27, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_AZ = {
    .alpha2 = "AZ",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 18, 0),
        REG_RULE(5250, 5330, 80, 0, 18, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 18, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_BA = {
    .alpha2 = "BA",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 27, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_BD = {
    .alpha2 = "BD",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_BE = {
    .alpha2 = "BE",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_BG = {
    .alpha2 = "BG",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_BH = {
    .alpha2 = "BH",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 20, 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_BJ = {
    .alpha2 = "BJ",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_BM = {
    .alpha2 = "BM",
    .reg_rules = {
        REG_RULE(2402, 2472, 40, 0, 30, 0),
        REG_RULE(5170, 5250, 80, 0, 24, 0),
        REG_RULE(5250, 5330, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_BL = {
    .alpha2 = "BL",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 18, 0),
        REG_RULE(5250, 5330, 80, 0, 18, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_3_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_BN = {
    .alpha2 = "BN",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 20, 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_BO = {
    .alpha2 = "BO",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 30, 0),
        REG_RULE(5170, 5250, 80, 0, 30, 0),
        REG_RULE(5250, 5330, 80, 0, 30, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 30, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_BR = {
    .alpha2 = "BR",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_BS = {
    .alpha2 = "BS",
    .reg_rules = {
        REG_RULE(2402, 2472, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 24, 0),
        REG_RULE(5250, 5330, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_BW = {
    .alpha2 = "BW",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_BY = {
    .alpha2 = "BY",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_BZ = {
    .alpha2 = "BZ",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 30, 0),
        REG_RULE(5170, 5250, 80, 0, 30, 0),
        REG_RULE(5250, 5330, 80, 0, 30, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 30, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_CA = {
    .alpha2 = "CA",
    .reg_rules = {
        REG_RULE(2402, 2472, 40, 3, 30, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5590, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5650, 5730, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_6_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_CD = {
    .alpha2 = "CD",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_CH = {
    .alpha2 = "CH",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_CI = {
    .alpha2 = "CI",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_CL = {
    .alpha2 = "CL",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 20, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_CM = {
    .alpha2 = "CM",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_CN = {
    .alpha2 = "CN",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 27, 0),
        REG_RULE(5170, 5250, 160, 0, 23, 0),
        REG_RULE(5250, 5330, 160, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 27, 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_CO = {
    .alpha2 = "CO",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 3, 30, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 23, 0)
    },
    .n_reg_rules = NUM_5_REG_RULE
};

/*
 * not restrict in CRDA
 * use world regulatory domain's max_eirp: 20
 * REF:
 * https://git.kernel.org/cgit/linux/kernel/git/linville/wireless-regdb.git/tree/db.txt
 */
static const struct ieee80211_regdomain g_regdom_CU = {
    .alpha2 = "CU",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 20, 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_CR = {
    .alpha2 = "CR",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 20, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_CS = {
    .alpha2 = "CS",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 27, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_CY = {
    .alpha2 = "CY",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_CZ = {
    .alpha2 = "CZ",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, NL80211_RRF_NO_OUTDOOR | 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_NO_OUTDOOR | NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_DE = {
    .alpha2 = "DE",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, NL80211_RRF_NO_OUTDOOR | 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_NO_OUTDOOR | NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_DK = {
    .alpha2 = "DK",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_DO = {
    .alpha2 = "DO",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 3, 30, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 3, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_DZ = {
    .alpha2 = "DZ",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_EC = {
    .alpha2 = "EC",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 3, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_EE = {
    .alpha2 = "EE",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_EG = {
    .alpha2 = "EG",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 40, 0, 23, 0),
        REG_RULE(5250, 5330, 40, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 40, 0, 23, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_ES = {
    .alpha2 = "ES",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, NL80211_RRF_NO_OUTDOOR | 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_NO_OUTDOOR | NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_ET = {
    .alpha2 = "ET",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 27, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_FI = {
    .alpha2 = "FI",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_FJ = {
    .alpha2 = "FJ",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5710, 5730, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_6_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_FR = {
    .alpha2 = "FR",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_GB = {
    .alpha2 = "GB",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_GD = {
    .alpha2 = "GD",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 30, 0),
        REG_RULE(5170, 5250, 80, 0, 17, 0),
        REG_RULE(5250, 5330, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_GF = {
    .alpha2 = "GF",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 27, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_GH = {
    .alpha2 = "GH",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_GL = {
    .alpha2 = "GL",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 27, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 27, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_GU = {
    .alpha2 = "GU",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 30, 0),
        REG_RULE(5170, 5250, 80, 0, 17, 0),
        REG_RULE(5250, 5330, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_GE = {
    .alpha2 = "GE",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 18, 0),
        REG_RULE(5250, 5330, 80, 0, 18, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 18, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_GR = {
    .alpha2 = "GR",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_GT = {
    .alpha2 = "GT",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 3, 30, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 3, 30, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_GY = {
    .alpha2 = "GY",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = 4
};

static const struct ieee80211_regdomain g_regdom_HK = {
    .alpha2 = "HK",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_HN = {
    .alpha2 = "HN",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_HR = {
    .alpha2 = "HR",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_HU = {
    .alpha2 = "HU",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_ID = {
    .alpha2 = "ID",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5815, 40, 0, 20, 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_IE = {
    .alpha2 = "IE",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_IL = {
    .alpha2 = "IL",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, NL80211_RRF_NO_OUTDOOR | 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_NO_OUTDOOR | NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_3_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_IN = {
    .alpha2 = "IN",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 20, 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_IQ = {
    .alpha2 = "IQ",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 30, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_IR = {
    .alpha2 = "IR",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 30, 0),
        REG_RULE(5250, 5330, 80, 0, 30, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 30, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_IS = {
    .alpha2 = "IS",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_IT = {
    .alpha2 = "IT",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_JM = {
    .alpha2 = "JM",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 3, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5710, 5730, 80, 3, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_6_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_JO = {
    .alpha2 = "JO",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 18, 0),
        REG_RULE(5490, 5710, 80, 0, 18, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 18, 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_JP = {
    .alpha2 = "JP",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_KE = {
    .alpha2 = "KE",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 30, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5815, 80, 0, 23, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_KH = {
    .alpha2 = "KH",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 27, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 27, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_KY = {
    .alpha2 = "KY",
    .reg_rules = {
        REG_RULE(2402, 2472, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 24, 0),
        REG_RULE(5250, 5330, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_KP = {
    .alpha2 = "KP",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 3, 30, 0),
        REG_RULE(5250, 5330, 80, 3, 30, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_KR = {
    .alpha2 = "KR",
    .reg_rules = {
        REG_RULE(2402, 2482, 20, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 3, 30, 0),
        REG_RULE(5250, 5330, 80, 3, 30, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 3, 30, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_KW = {
    .alpha2 = "KW",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, NL80211_RRF_NO_INDOOR | 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_KZ = {
    .alpha2 = "KZ",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 20, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

/*
 * not restrict in CRDA
 * use world regulatory domain's max_eirp: 20
 * REF:
 * https://git.kernel.org/cgit/linux/kernel/git/linville/wireless-regdb.git/tree/db.txt
 */
static const struct ieee80211_regdomain g_regdom_LA = {
    .alpha2 = "LA",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 20, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_LB = {
    .alpha2 = "LB",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_LI = {
    .alpha2 = "LI",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_LK = {
    .alpha2 = "LK",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_LS = {
    .alpha2 = "LS",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 27, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_LT = {
    .alpha2 = "LT",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_LU = {
    .alpha2 = "LU",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_LV = {
    .alpha2 = "LV",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_MA = {
    .alpha2 = "MA",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_3_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_MC = {
    .alpha2 = "MC",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 18, 0),
        REG_RULE(5250, 5330, 80, 0, 18, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 18, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_MD = {
    .alpha2 = "MD",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 27, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_ME = {
    .alpha2 = "ME",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 27, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_MG = {
    .alpha2 = "MG",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_MH = {
    .alpha2 = "MH",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 30, 0),
        REG_RULE(5170, 5250, 80, 0, 24, 0),
        REG_RULE(5250, 5330, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_MK = {
    .alpha2 = "MK",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_MO = {
    .alpha2 = "MO",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_MN = {
    .alpha2 = "MN",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 24, 0),
        REG_RULE(5250, 5330, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_MR = {
    .alpha2 = "MR",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 27, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_MU = {
    .alpha2 = "MU",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 24, 0),
        REG_RULE(5250, 5330, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 24, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

/*
 * not restrict in CRDA
 * use world regulatory domain's max_eirp: 20
 * REF:
 * https://git.kernel.org/cgit/linux/kernel/git/linville/wireless-regdb.git/tree/db.txt
 */
static const struct ieee80211_regdomain g_regdom_MV = {
    .alpha2 = "MV",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 20, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_MW = {
    .alpha2 = "MW",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 27, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 27, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_MT = {
    .alpha2 = "MT",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_MX = {
    .alpha2 = "MX",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 3, 30, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 3, 30, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_MY = {
    .alpha2 = "MY",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 3, 24, 0),
        REG_RULE(5250, 5330, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5650, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 24, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_NA = {
    .alpha2 = "NA",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5710, 5730, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_6_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_NG = {
    .alpha2 = "NG",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 20, 0),
    },
    .n_reg_rules = NUM_3_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_NI = {
    .alpha2 = "NI",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 30, 0),
        REG_RULE(5170, 5250, 80, 0, 24, 0),
        REG_RULE(5250, 5330, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_NL = {
    .alpha2 = "NL",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, NL80211_RRF_NO_OUTDOOR | 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_NO_OUTDOOR | NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_NO = {
    .alpha2 = "NO",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_NP = {
    .alpha2 = "NP",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_NZ = {
    .alpha2 = "NZ",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 30, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 3, 30, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_OM = {
    .alpha2 = "OM",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 3, 30, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_PA = {
    .alpha2 = "PA",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 3, 30, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 3, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_PE = {
    .alpha2 = "PE",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_PF = {
    .alpha2 = "PF",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5710, 5730, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_6_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_PG = {
    .alpha2 = "PG",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_PH = {
    .alpha2 = "PH",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 17, 0),
        REG_RULE(5250, 5330, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_PK = {
    .alpha2 = "PK",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_2_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_PL = {
    .alpha2 = "PL",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_PR = {
    .alpha2 = "PR",
    .reg_rules = {
        REG_RULE(2402, 2472, 40, 3, 30, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 3, 30, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_PY = {
    .alpha2 = "PY",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 24, 0),
        REG_RULE(5250, 5330, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_PT = {
    .alpha2 = "PT",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_QA = {
    .alpha2 = "QA",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 20, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_RO = {
    .alpha2 = "RO",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_RE = {
    .alpha2 = "RE",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 27, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

/*
 * NOT RIGHT YET
 * country RS: DFS-ETSI
 * (2400 - 2483.5 @ 40), (100 mW)
 * (5150 - 5350 @ 40), (200 mW), NO-OUTDOOR
 * (5470 - 5725 @ 20), (1000 mW), DFS
 * # 60 gHz band channels 1-4, ref: Etsi En 302 567
 * (57000 - 66000 @ 2160), (40)
 */
static const struct ieee80211_regdomain g_regdom_RS = {
    .alpha2 = "RS",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_RU = {
    .alpha2 = "RU",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5650, 5710, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 20, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_SA = {
    .alpha2 = "SA",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 3, 23, 0),
        REG_RULE(5250, 5330, 80, 3, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 3, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 20, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

/*
 * not restrict in CRDA
 * use world regulatory domain's max_eirp: 20
 * REF:
 * https://git.kernel.org/cgit/linux/kernel/git/linville/wireless-regdb.git/tree/db.txt
 */
static const struct ieee80211_regdomain g_regdom_SD = {
    .alpha2 = "SD",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 20, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_SN = {
    .alpha2 = "SN",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 17, 0),
        REG_RULE(5250, 5330, 80, 0, 24, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_3_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_SE = {
    .alpha2 = "SE",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_SG = {
    .alpha2 = "SG",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_SI = {
    .alpha2 = "SI",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_SK = {
    .alpha2 = "SK",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_SR = {
    .alpha2 = "SR",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_SV = {
    .alpha2 = "SV",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 20, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_SY = {
    .alpha2 = "SY",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 20, 0)
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_TH = {
    .alpha2 = "TH",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_TN = {
    .alpha2 = "TN",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5650, 80, 0, 24, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_TR = {
    .alpha2 = "TR",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 23, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 14, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_TT = {
    .alpha2 = "TT",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_TW = {
    .alpha2 = "TW",
    .reg_rules = {
        REG_RULE(2402, 2472, 40, 3, 30, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 17, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 3, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

/*
 * NOT CORRECT YET
 * country UA: DFS-ETSI
 * (2400 - 2483.5 @ 40), (20), NO-OUTDOOR
 * (5150 - 5350 @ 40), (20), NO-OUTDOOR
 * (5490 - 5670 @ 80), (20), DFS
 * (5735 - 5835 @ 80), (20)
 * # 60 gHz band channels 1-4, ref: Etsi En 302 567
 * (57000 - 66000 @ 2160), (40)
 */
static const struct ieee80211_regdomain g_regdom_UA = {
    .alpha2 = "UA",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, NL80211_RRF_NO_OUTDOOR | 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 3, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 20, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_US = {
    .alpha2 = "US",
    .reg_rules = {
        REG_RULE(2402, 2472, 40, 3, 30, 0),
        REG_RULE(5170, 5250, 160, 3, 17, 0),
        REG_RULE(5250, 5330, 160, 3, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 3, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_UY = {
    .alpha2 = "UY",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 3, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_UZ = {
    .alpha2 = "UZ",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 3, 30, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 3, 24, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_UG = {
    .alpha2 = "UG",
    .reg_rules = {
        REG_RULE(2402, 2472, 40, 0, 20, 0),
        REG_RULE(5270, 5330, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

/*
 * not restrict in CRDA
 * use world regulatory domain's max_eirp: 20
 * REF:
 * https://git.kernel.org/cgit/linux/kernel/git/linville/wireless-regdb.git/tree/db.txt
 */
static const struct ieee80211_regdomain g_regdom_VA = {
    .alpha2 = "VA",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 20, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_VI = {
    .alpha2 = "VI",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 30, 0),
        REG_RULE(5170, 5250, 80, 0, 24, 0),
        REG_RULE(5250, 5330, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_VE = {
    .alpha2 = "VE",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 30, 0),
        REG_RULE(5250, 5330, 80, 0, 30, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_VN = {
    .alpha2 = "VN",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 17, 0),
        REG_RULE(5250, 5330, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 30, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_YE = {
    .alpha2 = "YE",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 20, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_YT = {
    .alpha2 = "YT",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 0, 27, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_4_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_ZA = {
    .alpha2 = "ZA",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 3, 17, 0),
        REG_RULE(5250, 5330, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5710, 80, 3, 24, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 24, 0)
    },
    .n_reg_rules = NUM_5_REG_RULE
};

/*
 * not restrict in CRDA
 * use world regulatory domain's max_eirp: 20
 * REF:
 * https://git.kernel.org/cgit/linux/kernel/git/linville/wireless-regdb.git/tree/db.txt
 */
static const struct ieee80211_regdomain g_regdom_ZM = {
    .alpha2 = "ZM",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 20, 0),
    },
    .n_reg_rules = NUM_5_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_ZW = {
    .alpha2 = "ZW",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5590, 80, 0, 20, NL80211_RRF_DFS | 0),
        REG_RULE(5590, 5650, 80, 0, 20, 0),
        REG_RULE(5650, 5710, 80, 0, 20, NL80211_RRF_DFS | 0),
    },
    .n_reg_rules = NUM_6_REG_RULE
};

static const struct ieee80211_regdomain g_regdom_MM = {
    .alpha2 = "MM",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0),
        REG_RULE(5170, 5250, 80, 0, 20, 0),
        REG_RULE(5250, 5330, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5490, 5730, 80, 0, 23, NL80211_RRF_DFS | 0),
        REG_RULE(5735, 5835, 80, 0, 23, 0)
    },
    .n_reg_rules = NUM_5_REG_RULE
};

/* 特殊管制域 ，2g channel 1~13 5g 全雷达信道 channel 36~48 52~64 100~140 149~165 */
static const struct ieee80211_regdomain g_regdom_ZZ = {
    .alpha2 = "ZZ",
    .reg_rules = {
        REG_RULE(2402, 2482, 40, 0, 20, 0), // chan:1~13
        REG_RULE(5170, 5250, 80, 0, 20, NL80211_RRF_DFS | 0), // chan:36~52
        REG_RULE(5250, 5330, 80, 0, 20, NL80211_RRF_DFS | 0), // chan:52~64
        REG_RULE(5490, 5710, 80, 0, 20, NL80211_RRF_DFS | 0), // chan:100~140
        REG_RULE(5735, 5835, 80, 0, 20, NL80211_RRF_DFS | 0), // chan:149~165
    },
    .n_reg_rules = NUM_5_REG_RULE
};

/* country alpha2 code can search from http://en.wikipedia.org/wiki/ISO_3166-1_alpha-2 */
const struct ieee80211_regdomain *g_reg_regdb[] = {
    &g_regdom_AA,
    &g_regdom_AE,
    &g_regdom_AF,
    &g_regdom_AG,
    &g_regdom_AI,
    &g_regdom_AL,
    &g_regdom_AM,
    &g_regdom_AN,
    &g_regdom_AO,
    &g_regdom_AR,
    &g_regdom_AS,
    &g_regdom_AT,
    &g_regdom_AU,
    &g_regdom_AW,
    &g_regdom_AZ,
    &g_regdom_BA,
    &g_regdom_BD,
    &g_regdom_BE,
    &g_regdom_BG,
    &g_regdom_BH,
    &g_regdom_BJ,
    &g_regdom_BM,
    &g_regdom_BL,
    &g_regdom_BN,
    &g_regdom_BO,
    &g_regdom_BR,
    &g_regdom_BS,
    &g_regdom_BW,
    &g_regdom_BY,
    &g_regdom_BZ,
    &g_regdom_CA,
    &g_regdom_CD,
    &g_regdom_CH,
    &g_regdom_CI,
    &g_regdom_CL,
    &g_regdom_CM,
    &g_regdom_CN,
    &g_regdom_CO,
    &g_regdom_CU,
    &g_regdom_CR,
    &g_regdom_CS,
    &g_regdom_CY,
    &g_regdom_CZ,
    &g_regdom_DE,
    &g_regdom_DK,
    &g_regdom_DO,
    &g_regdom_DZ,
    &g_regdom_EC,
    &g_regdom_EE,
    &g_regdom_EG,
    &g_regdom_ES,
    &g_regdom_ET,
    &g_regdom_FI,
    &g_regdom_FJ,
    &g_regdom_FR,
    &g_regdom_GB,
    &g_regdom_GD,
    &g_regdom_GF,
    &g_regdom_GH,
    &g_regdom_GL,
    &g_regdom_GU,
    &g_regdom_GE,
    &g_regdom_GR,
    &g_regdom_GT,
    &g_regdom_GY,
    &g_regdom_HK,
    &g_regdom_HN,
    &g_regdom_HR,
    &g_regdom_HU,
    &g_regdom_ID,
    &g_regdom_IE,
    &g_regdom_IL,
    &g_regdom_IN,
    &g_regdom_IQ,
    &g_regdom_IR,
    &g_regdom_IS,
    &g_regdom_IT,
    &g_regdom_JM,
    &g_regdom_JO,
    &g_regdom_JP,
    &g_regdom_KE,
    &g_regdom_KH,
    &g_regdom_KY,
    &g_regdom_KP,
    &g_regdom_KR,
    &g_regdom_KW,
    &g_regdom_KZ,
    &g_regdom_LA,
    &g_regdom_LB,
    &g_regdom_LI,
    &g_regdom_LK,
    &g_regdom_LS,
    &g_regdom_LT,
    &g_regdom_LU,
    &g_regdom_LV,
    &g_regdom_MA,
    &g_regdom_MC,
    &g_regdom_MD,
    &g_regdom_ME,
    &g_regdom_MG,
    &g_regdom_MH,
    &g_regdom_MK,
    &g_regdom_MM,
    &g_regdom_MN,
    &g_regdom_MR,
    &g_regdom_MU,
    &g_regdom_MV,
    &g_regdom_MW,
    &g_regdom_MO,
    &g_regdom_MT,
    &g_regdom_MX,
    &g_regdom_MY,
    &g_regdom_NA,
    &g_regdom_NG,
    &g_regdom_NI,
    &g_regdom_NL,
    &g_regdom_NO,
    &g_regdom_NP,
    &g_regdom_NZ,
    &g_regdom_OM,
    &g_regdom_PA,
    &g_regdom_PE,
    &g_regdom_PF,
    &g_regdom_PG,
    &g_regdom_PH,
    &g_regdom_PK,
    &g_regdom_PL,
    &g_regdom_PY,
    &g_regdom_PR,
    &g_regdom_PT,
    &g_regdom_QA,
    &g_regdom_RO,
    &g_regdom_RE,
    &g_regdom_RS,
    &g_regdom_RU,
    &g_regdom_SA,
    &g_regdom_SD,
    &g_regdom_SN,
    &g_regdom_SE,
    &g_regdom_SG,
    &g_regdom_SI,
    &g_regdom_SK,
    &g_regdom_SR,
    &g_regdom_SV,
    &g_regdom_SY,
    &g_regdom_TH,
    &g_regdom_TN,
    &g_regdom_TR,
    &g_regdom_TT,
    &g_regdom_TW,
    &g_regdom_UA,
    &g_regdom_US,
    &g_regdom_UY,
    &g_regdom_UZ,
    &g_regdom_UG,
    &g_regdom_VA,
    &g_regdom_VI,
    &g_regdom_VE,
    &g_regdom_VN,
    &g_regdom_YE,
    &g_regdom_YT,
    &g_regdom_ZA,
    &g_regdom_ZM,
    &g_regdom_ZW,
    &g_regdom_ZZ,
};

int g_reg_regdb_size = oal_array_size(g_reg_regdb);
#endif
#endif
