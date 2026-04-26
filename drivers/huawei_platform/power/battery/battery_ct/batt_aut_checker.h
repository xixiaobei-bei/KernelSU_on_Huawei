/*
 * batt_aut_checker.h
 *
 * battery checker head file
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#ifndef _BATT_AUT_CHECKER_H_
#define _BATT_AUT_CHECKER_H_

#include <linux/stddef.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/power/hisi/battery_data.h>
#include <linux/power/hisi/coul/coul_event.h>
#include <linux/notifier.h>
#include <huawei_platform/log/hw_log.h>

#include "batt_info_util.h"

enum res_type {
	RES_CT = 0,
	RES_SN,
	RES_ACT,
};

enum batt_act_state {
	SIGN_FAIL = 0,
	ACT_PASS,
	RETROFIT_PASS,
};

enum nv_sn_version {
	ILLEGAL_BIND_VERSION = 0,
	RAW_BIND_VERSION,
	PAIR_BIND_VERSION,
	LEFT_UNUSED_VERSION,
};

#define MAX_SN_LEN                                  20
#define MAX_RAW_SN_LEN                              16
#define MAX_SN_BUFF_LENGTH                          5

struct binded_info {
	unsigned int version;
	char info[MAX_SN_BUFF_LENGTH][MAX_SN_LEN];
};

enum batt_source {
	BATTERY_ORIGINAL = 0,
	BATTERY_SPARE_PART,
	BATTERY_UNKNOWN,
};

enum batt_aut_util_type {
	UTIL_LOCAL_TYPE = 0,
	UTIL_BSC_TYPE,
	UTIL_BYC_TYPE,
};

enum batt_match_type {
	BATTERY_UNREMATCHABLE = 0,
	BATTERY_REMATCHABLE = 1,
};

enum batt_cert_state {
	CERT_UNREADY = 0,
	CERT_READY = 1,
};

enum batt_safe_info_t {
	BATT_CHARGE_CYCLES = 0,
	BATT_MATCH_ABILITY,
	BATT_SPARE_PART,
	BATT_CERT_READY,
};

enum ic_cr {
	IC_PASS = 0,
	IC_FAIL_UNMATCH,        /* IC information is not right for product */
	IC_FAIL_UNKOWN,         /* IC unkown(communicate error or data error) */
	IC_FAIL_MEM_STATUS,     /* IC memory check failed */
};

enum key_cr {
	KEY_PASS = 0,
	KEY_FAIL_TIMEOUT,       /* checking key timeout */
	KEY_UNREADY,            /* key is under checking */
	KEY_FAIL_UNMATCH,       /* key is unmatch */
	KEY_FAIL_IC,            /* ic is bad */
	KEY_FAIL_ACT,           /* failed to activate signature */
};

enum sn_cr {
	SN_PASS = 0,
	SN_OBD_REMATCH,         /* old board rematch new battery */
	SN_OBT_REMATCH,         /* old battery rematch new board */
	SN_NN_REMATCH,          /* new board & new battery rematch */
	SN_FAIL_TIMEOUT,        /* get SN timeout */
	SN_FAIL_NV,             /* Read or Write NV fail */
	SN_UNREADY,             /* SN is under checking */
	SN_FAIL_IC,             /* SN get from IC timeout */
	SN_OO_UNMATCH,          /* old board & old battery unmatch */
	SN_SNS_UNMATCH,         /* sns of batterys umatch */
	SN_TO_REMATCH,
};

enum check_mode {
	FACTORY_CHECK_MODE = 0,
	COMMERCIAL_CHECK_MODE = 15,
};

struct batt_ct_ops {
	enum batt_ic_type (*get_ic_type)(void);
	int (*get_ic_uuid)(struct platform_device *pdev,
		const unsigned char **uuid, unsigned int *uuid_len);
	int (*get_batt_type)(struct platform_device *pdev,
		const unsigned char **type, unsigned int *type_len);
	int (*get_batt_sn)(struct platform_device *pdev, struct power_genl_attr *res,
		const unsigned char **sn, unsigned int *sn_len_bits);
	int (*prepare)(struct platform_device *pdev, enum res_type type,
		struct power_genl_attr *res);
	int (*certification)(struct platform_device *pdev, struct power_genl_attr *res,
		enum key_cr *result);
	int (*set_act_signature)(struct platform_device *pdev,
		enum res_type type, const struct power_genl_attr *res);
	int (*set_batt_safe_info)(struct platform_device *pdev,
		enum batt_safe_info_t type, void *value);
	int (*get_batt_safe_info)(struct platform_device *pdev,
		enum batt_safe_info_t type, void *value);
	void (*power_down)(struct platform_device *pdev);
};

struct batt_ct_ops_list {
	struct list_head node;
	struct platform_device *ic_dev;
	int (*ct_ops_register)(struct platform_device *, struct batt_ct_ops *);
	void (*ic_memory_release)(struct platform_device *);
};

/* return macro */
#define BATTERY_DRIVER_FAIL                         (-1)
#define BATTERY_DRIVER_SUCCESS                      0

/* IC memory protection mode */
#define READ_PROTECTION                             0x80
#define WRITE_PROTECTION                            0x40
#define EPROM_EMULATION_MODE                        0x20
#define AUTHENTICATION_PROTECTION                   0x10
#define NO_PROTECTION                               0x00

/* sys node information show macro */
#define BATT_ID_PRINT_SIZE_PER_CHAR                 3

/* NVME macro */
#define MAX_SN_LEN_BITS                             32
#define NV_VERSION_INDEX                            0

/* Battery maxium current&voltage initialization value */
#define MAX_CHARGE_CURRENT                          10000
#define MAX_CHARGE_VOLTAGE                          10000

#define HEXBITS                                     4
#define BYTEBITS                                    8

#define MAC_RESOURCE_TPYE0                          0
#define MAC_RESOURCE_TPYE1                          1

#define ERR_NO_STRING_SIZE                          128
#define DSM_BATTERY_DETECT_BUFFSIZE                 1024

#define dev_get_drv_data(data, dev)                                      \
do {                                                                    \
	data = dev_get_drvdata(dev);                                    \
	if (!(data))                                                    \
		hwlog_err("get drvdata failed in %s\n", __func__);      \
} while (0)

#define BATT_INVALID_CYCLES (-1)
#define BATT_COUL_CYCLES_RATIO 100
#define BATT_IC_CYCLES_RATIO 10
#define BATT_RELATIVE_CYCLES_RATIO 10
#define BATT_HEX_STR_BYTE 3
#define BATT_SN_READ_RETRY 5
#define BATT_SPARE_LIMIT_CYC 3
#define BATT_FREE_LIMIT_CYC 15
#define BATT_MAX_SIGN_LEN 64
#define BATT_MAX_KEY_ID_LEN 10
#define BATT_ACT_SIGN_ID "0x7500"
#define BATT_RETROFIT_SIGN_ID "0x7501"

struct batt_act_request {
	char act_key_id[BATT_MAX_KEY_ID_LEN];
	char retrofit_key_id[BATT_MAX_KEY_ID_LEN];
	char act_signature[BATT_MAX_SIGN_LEN];
	unsigned int act_sign_len;
	char ecc_signature[BATT_MAX_SIGN_LEN];
	unsigned int ecc_sign_len;
};

struct batt_chk_rslt {
	enum ic_cr ic_status;
	enum key_cr key_status;
	enum sn_cr sn_status;
	enum check_mode check_mode;
};

struct batt_chk_data;
typedef void (*release_chk_data_t)(struct platform_device *pdev);

struct batt_ct_wrapper_ops {
	enum batt_ic_type (*get_ic_type)(struct batt_chk_data *data);
	int (*get_ic_uuid)(struct batt_chk_data *data,
		const unsigned char **uuid, unsigned int *uuid_len);
	int (*get_batt_type)(struct batt_chk_data *data,
		const unsigned char **type, unsigned int *type_len);
	int (*get_batt_sn)(struct batt_chk_data *data, struct power_genl_attr *res,
		const unsigned char **sn, unsigned int *sn_len_bits);
	int (*prepare)(struct batt_chk_data *data, enum res_type type,
		struct power_genl_attr *res);
	int (*certification)(struct batt_chk_data *data, struct power_genl_attr *res,
		enum key_cr *result);
	int (*set_act_signature)(struct batt_chk_data *data, enum res_type type,
		const struct power_genl_attr *res);
	int (*set_batt_safe_info)(struct batt_chk_data *data,
		enum batt_safe_info_t type, void *value);
	int (*get_batt_safe_info)(struct batt_chk_data *data,
		enum batt_safe_info_t type, void *value);
	void (*power_down)(struct batt_chk_data *data);
};

struct batt_chk_data {
	unsigned char id_in_grp;
	unsigned char chks_in_grp;
	unsigned char id_of_grp;
	unsigned int sn_len;
	unsigned int free_cycles;
	unsigned int spare_cycles;
	int coul_cycles;
	int ic_cycles;
	enum batt_match_type batt_rematch_onboot;
	enum batt_match_type batt_rematch_current;
	const unsigned char *sn;
	struct platform_device *ic;
	struct work_struct check_key_work;
	struct work_struct check_sn_work;
	struct completion key_prepare_ready;
	struct completion sn_prepare_ready;
	struct completion act_sign_complete;
	struct notifier_block batt_cycles_listener;
	struct power_genl_attr key_res;
	struct power_genl_attr sn_res;
	enum batt_ic_type ic_type;
	struct batt_ct_ops ic_ops;
	struct batt_ct_wrapper_ops bco;
	struct batt_chk_rslt result;
	struct batt_checker_entry entry;
	release_chk_data_t release;
	struct work_struct act_signature_work;
	enum batt_source batt_src_onboot;
	enum batt_source batt_src_current;
	enum batt_act_state act_state;
};

void init_battery_check_result(struct batt_chk_rslt *result);
void add_to_aut_ic_list(struct batt_ct_ops_list *entry);
const struct of_device_id *get_battery_checkers_match_table(void);
enum batt_aut_util_type checker_util_type(enum batt_ic_type ic_type);
void bsc_key_func(struct work_struct *work);
void bsc_sn_func(struct work_struct *work);
int bsc_register_cycles_listener(struct batt_chk_data *drv_data);
void byc_key_func(struct work_struct *work);
void byc_sn_func(struct work_struct *work);
void byc_act_sign_func(struct work_struct *work);
int byc_register_cycles_listener(struct batt_chk_data *drv_data);

#endif /* _BATT_AUT_CHECKER_H_ */
