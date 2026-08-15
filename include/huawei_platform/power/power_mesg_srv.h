/*
 * power_mesg_srv.h
 *
 * power netlink service head file
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _POWER_MESG_SRV_H_
#define _POWER_MESG_SRV_H_

#include <linux/types.h>

enum {
	POWER_GENL_UNUSED = 0, /* 0 is invalid attr type in kernel(nla_parse) */
	POWER_GENL_ATTR0,
	POWER_GENL_ATTR1,
	__POWER_GENL_ATTR_NUM,
};

#define TOTAL_POWER_GENL_ATTR       (__POWER_GENL_ATTR_NUM - 1)
#define BATT_INFO_DEVICE_ATTR       POWER_GENL_ATTR0
#define BATT_INFO_DATA_ATTR         POWER_GENL_ATTR1
#define POWER_GENL_RAW_DATA_ATTR    POWER_GENL_ATTR1
#define POWER_GENL_MAX_ATTR_INDEX   (__POWER_GENL_ATTR_NUM - 1)

enum batt_info_subcmd {
	CT_PREPARE = 0,
	SN_PREPARE,
}; /* should not larger than 255 */

struct nl_dev_info {
	unsigned char id_in_grp;
	unsigned char chks_in_grp;
	unsigned char id_of_grp;
	unsigned char ic_type;
	unsigned char subcmd;
};

enum batt_ic_type {
	LOCAL_IC_TYPE = 0,
	MAXIM_DS28EL15_TYPE = 1,
	MAXIM_DS28EL16_TYPE = 2,
	NXP_A1007_TYPE = 3,
}; /* should not larger than 255 */

struct batt_res {
	const unsigned char *data;
	unsigned int len;
	int type;
};

/*
 *cmd: 00-49 is for battery
 *      50-99 is for wireless charging
 *      100-149 is for directly charging
 *      max is 255
 */
enum {
	BATT_INFO_CMD = 0,
	BATT_DMD_CMD = 10,
	BOARD_INFO_CMD = 40,
	BATT_FINAL_RESULT_CMD = 49,
	POWER_CMD_ADAPTOR_ANTIFAKE_HASH = 50,
	POWER_CMD_WC_ANTIFAKE_HASH = 60,
	POWER_CMD_TOTAL_NUM = 256,
};

enum result_stat {
	FINAL_RESULT_PASS = 0,
	FINAL_RESULT_CRASH,
	FINAL_RESULT_FAIL,
	__FINAL_RESULT_MAX,
};

enum {
	NEW_BOARD_MESG_INDEX = 0,
	OLD_BOARD_MESG_INDEX,
};

#define PRESENT80_CACHE_LEN     10
#define A1007_UID_SIZE          9

struct a1007_mac_src {
	uint8_t uid[A1007_UID_SIZE];
	uint8_t mac_context[PRESENT80_CACHE_LEN];
};

#ifdef BATTCT_KERNEL_SRV_SHARE_VAR
/* all element should be same strlen */
const char *result_status_mesg[] = {
	[FINAL_RESULT_PASS]  = "AuthenticationPass_",
	[FINAL_RESULT_CRASH] = "AuthenticationCrash",
	[FINAL_RESULT_FAIL]  = "AuthenticationFail_",
};

const char *board_info_mesg = {
	"BoardInformationMessage",
};

const char *board_info_cb_mesg[] = {
	[NEW_BOARD_MESG_INDEX] = "IFeelYong",
	[OLD_BOARD_MESG_INDEX] = "IFeelOld_",
};
#endif /* BATTCT_KERNEL_SRV_SHARE_VAR */

#endif /* _POWER_MESG_SRV_H_ */
