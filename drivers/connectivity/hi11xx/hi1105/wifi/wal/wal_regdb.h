

#ifndef __WAL_REGDB_H__
#define __WAL_REGDB_H__

#include "oal_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_REGDB_H

#define HI1103_SPECIFIC_COUNTRY_CODE_IDX 0

#define NUM_2_REG_RULE  2
#define NUM_3_REG_RULE  3
#define NUM_4_REG_RULE  4
#define NUM_5_REG_RULE  5
#define NUM_6_REG_RULE  6

extern const oal_ieee80211_regdomain_stru g_st_default_regdom;

const oal_ieee80211_regdomain_stru *wal_regdb_find_db(int8_t *str);
#endif /* end of wal_regdb.h */
