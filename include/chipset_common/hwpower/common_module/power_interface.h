/*
 * power_interface.h
 *
 * interface for power module
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _POWER_INTERFACE_H_
#define _POWER_INTERFACE_H_

#define POWER_IF_RD_BUF_SIZE  64
#define POWER_IF_WR_BUF_SIZE  256

enum power_if_sysfs_type {
	POWER_IF_SYSFS_BEGIN = 0,
	POWER_IF_SYSFS_ENABLE_CHARGER = POWER_IF_SYSFS_BEGIN,
	POWER_IF_SYSFS_VBUS_IIN_LIMIT,
	POWER_IF_SYSFS_BATT_ICHG_LIMIT,
	POWER_IF_SYSFS_BATT_ICHG_RATIO,
	POWER_IF_SYSFS_BATT_VTERM_DEC,
	POWER_IF_SYSFS_RT_TEST_TIME,
	POWER_IF_SYSFS_RT_TEST_RESULT,
	POWER_IF_SYSFS_HOTA_IIN_LIMIT,
	POWER_IF_SYSFS_STARTUP_IIN_LIMIT,
	POWER_IF_SYSFS_ENABLE,
	POWER_IF_SYSFS_READY,
	POWER_IF_SYSFS_VBUS_PWR_LIMIT,
	POWER_IF_SYSFS_VBUS_IIN_THERMAL,
	POWER_IF_SYSFS_VBUS_IIN_THERMAL_ALL,
	POWER_IF_SYSFS_BATT_ICHG_THERMAL,
	POWER_IF_SYSFS_ADAPTOR_VOLTAGE,
	POWER_IF_SYSFS_WIRELESS_THERMAL_CTRL,
	POWER_IF_SYSFS_RTB_SUCCESS,
	POWER_IF_SYSFS_IBUS,
	POWER_IF_SYSFS_VBUS,
	POWER_IF_SYSFS_END,
};

enum power_if_op_user {
	POWER_IF_OP_USER_BEGIN = 0,
	POWER_IF_OP_USER_DEFAULT = POWER_IF_OP_USER_BEGIN, /* for default */
	POWER_IF_OP_USER_RC, /* for rc file */
	POWER_IF_OP_USER_HIDL, /* for hidl interface */
	POWER_IF_OP_USER_HEALTHD, /* for healthd daemon */
	POWER_IF_OP_USER_BMS_LIMIT, /* for bms_limit daemon */
	POWER_IF_OP_USER_CHARGE_MONITOR, /* for charge_monitor native */
	POWER_IF_OP_USER_ATCMD, /* for atcmd native */
	POWER_IF_OP_USER_THERMAL, /* for thermal daemon */
	POWER_IF_OP_USER_RUNNING, /* for running apk */
	POWER_IF_OP_USER_FWK, /* for framework */
	POWER_IF_OP_USER_APP, /* for app */
	POWER_IF_OP_USER_SHELL, /* for shell command */
	POWER_IF_OP_USER_KERNEL, /* for kernel space */
	POWER_IF_OP_USER_BSOH, /* for bsoh daemon */
	POWER_IF_OP_USER_BMS_HEATING, /* for bms_heating daemon */
	POWER_IF_OP_USER_BMS_AUTH, /* for bms_auth daemon */
	POWER_IF_OP_USER_END,
};

enum power_if_op_type {
	POWER_IF_OP_TYPE_BEGIN = 0,
	POWER_IF_OP_TYPE_SDP = POWER_IF_OP_TYPE_BEGIN, /* sdp */
	POWER_IF_OP_TYPE_DCP, /* standard */
	POWER_IF_OP_TYPE_OTG, /* otg */
	POWER_IF_OP_TYPE_HVC, /* hvc */
	POWER_IF_OP_TYPE_PD, /* pd */
	POWER_IF_OP_TYPE_LVC, /* lvc */
	POWER_IF_OP_TYPE_SC, /* sc */
	POWER_IF_OP_TYPE_MAINSC, /* mainsc */
	POWER_IF_OP_TYPE_AUXSC, /* auxsc */
	POWER_IF_OP_TYPE_WL, /* wireless */
	POWER_IF_OP_TYPE_WL_SC, /* wireless sc */
	POWER_IF_OP_TYPE_UVDM, /* uvdm */
	POWER_IF_OP_TYPE_POWER_GPIO, /* power gpio */
	POWER_IF_OP_TYPE_POWER_FIRMWARE, /* power firmware */
	POWER_IF_OP_TYPE_ALL, /* for all type */
	POWER_IF_OP_TYPE_END,
};

enum power_if_error_code {
	POWER_IF_ERRCODE_INVAID_OP = -22,
	POWER_IF_ERRCODE_FAIL = -1,
	POWER_IF_ERRCODE_PASS = 0,
};

/* power interface operator */
struct power_if_ops {
	const char *type_name;
	int (*set_enable_charger)(unsigned int value);
	int (*get_enable_charger)(unsigned int *value);
	int (*set_iin_limit)(unsigned int value);
	int (*get_iin_limit)(unsigned int *value);
	int (*set_ichg_limit)(unsigned int value);
	int (*get_ichg_limit)(unsigned int *value);
	int (*set_ichg_ratio)(unsigned int value);
	int (*get_ichg_ratio)(unsigned int *value);
	int (*set_vterm_dec)(unsigned int value);
	int (*get_vterm_dec)(unsigned int *value);
	int (*get_rt_test_time)(unsigned int *value);
	int (*get_rt_test_result)(unsigned int *value);
	int (*get_hota_iin_limit)(unsigned int *value);
	int (*get_startup_iin_limit)(unsigned int *value);
	int (*set_enable)(unsigned int value);
	int (*get_enable)(unsigned int *value);
	int (*set_pwr_limit)(unsigned int value);
	int (*get_pwr_limit)(unsigned int *value);
	int (*set_iin_thermal)(unsigned int index, unsigned int value);
	int (*set_iin_thermal_all)(unsigned int value);
	int (*set_ichg_thermal)(unsigned int value);
	int (*get_ichg_thermal)(unsigned int *value);
	int (*set_adap_volt)(int value);
	int (*get_adap_volt)(int *value);
	int (*set_wl_thermal_ctrl)(unsigned char value);
	int (*get_wl_thermal_ctrl)(unsigned char *value);
	int (*set_rtb_success)(unsigned int value);
	int (*set_ready)(unsigned int value);
	int (*get_ready)(unsigned int *value);
	int (*get_ibus)(int *value);
	int (*get_vbus)(int *value);
};

/* power interface info */
struct power_if_device_info {
	struct device *dev;
	unsigned int total_ops;
	struct power_if_ops *ops[POWER_IF_OP_TYPE_END];
};

int power_if_ops_register(struct power_if_ops *ops);
int power_if_kernel_sysfs_get(unsigned int type, unsigned int sysfs_type,
	unsigned int *value);
int power_if_kernel_sysfs_set(unsigned int type, unsigned int sysfs_type,
	unsigned int value);

#endif /* _POWER_INTERFACE_H_ */
