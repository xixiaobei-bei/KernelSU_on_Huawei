/*
 * power_interface.c
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_interface.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_if
HWLOG_REGIST();

static struct power_if_device_info *g_power_if_info;

static const char * const g_power_if_op_user_table[POWER_IF_OP_USER_END] = {
	[POWER_IF_OP_USER_DEFAULT] = "default",
	[POWER_IF_OP_USER_RC] = "rc",
	[POWER_IF_OP_USER_HIDL] = "hidl",
	[POWER_IF_OP_USER_HEALTHD] = "healthd",
	[POWER_IF_OP_USER_BMS_LIMIT] = "bms_limit",
	[POWER_IF_OP_USER_CHARGE_MONITOR] = "charge_monitor",
	[POWER_IF_OP_USER_ATCMD] = "atcmd",
	[POWER_IF_OP_USER_THERMAL] = "thermal",
	[POWER_IF_OP_USER_RUNNING] = "running",
	[POWER_IF_OP_USER_FWK] = "fwk",
	[POWER_IF_OP_USER_APP] = "app",
	[POWER_IF_OP_USER_SHELL] = "shell",
	[POWER_IF_OP_USER_KERNEL] = "kernel",
	[POWER_IF_OP_USER_BSOH] = "bsoh",
	[POWER_IF_OP_USER_BMS_HEATING] = "bms_heating",
	[POWER_IF_OP_USER_BMS_AUTH] = "bms_auth",
};

static const char * const g_power_if_op_type_table[POWER_IF_OP_TYPE_END] = {
	[POWER_IF_OP_TYPE_SDP] = "sdp",
	[POWER_IF_OP_TYPE_DCP] = "dcp",
	[POWER_IF_OP_TYPE_OTG] = "otg",
	[POWER_IF_OP_TYPE_HVC] = "hvc",
	[POWER_IF_OP_TYPE_PD] = "pd",
	[POWER_IF_OP_TYPE_LVC] = "lvc",
	[POWER_IF_OP_TYPE_SC] = "sc",
	[POWER_IF_OP_TYPE_MAINSC] = "mainsc",
	[POWER_IF_OP_TYPE_AUXSC] = "auxsc",
	[POWER_IF_OP_TYPE_WL] = "wl",
	[POWER_IF_OP_TYPE_WL_SC] = "wl_sc",
	[POWER_IF_OP_TYPE_UVDM] = "uvdm",
	[POWER_IF_OP_TYPE_POWER_GPIO] = "power_gpio",
	[POWER_IF_OP_TYPE_POWER_FIRMWARE] = "power_firmware",
	[POWER_IF_OP_TYPE_ALL] = "all",
};

static const char * const g_power_if_sysfs_type_table[POWER_IF_SYSFS_END] = {
	[POWER_IF_SYSFS_ENABLE_CHARGER] = "enable_charger",
	[POWER_IF_SYSFS_VBUS_IIN_LIMIT] = "iin_limit",
	[POWER_IF_SYSFS_BATT_ICHG_LIMIT] = "ichg_limit",
	[POWER_IF_SYSFS_BATT_ICHG_RATIO] = "ichg_ratio",
	[POWER_IF_SYSFS_BATT_VTERM_DEC] = "vterm_dec",
	[POWER_IF_SYSFS_RT_TEST_TIME] = "rt_test_time",
	[POWER_IF_SYSFS_RT_TEST_RESULT] = "rt_test_result",
	[POWER_IF_SYSFS_HOTA_IIN_LIMIT] = "hota_iin_limit",
	[POWER_IF_SYSFS_STARTUP_IIN_LIMIT] = "startup_iin_limit",
	[POWER_IF_SYSFS_ENABLE] = "enable",
	[POWER_IF_SYSFS_READY] = "ready",
	[POWER_IF_SYSFS_VBUS_PWR_LIMIT] = "power_limit",
	[POWER_IF_SYSFS_VBUS_IIN_THERMAL] = "iin_thermal",
	[POWER_IF_SYSFS_VBUS_IIN_THERMAL_ALL] = "iin_thermal_all",
	[POWER_IF_SYSFS_BATT_ICHG_THERMAL] = "ichg_thermal",
	[POWER_IF_SYSFS_ADAPTOR_VOLTAGE] = "adap_volt",
	[POWER_IF_SYSFS_WIRELESS_THERMAL_CTRL] = "wl_thermal_ctrl",
	[POWER_IF_SYSFS_RTB_SUCCESS] = "rtb_success",
	[POWER_IF_SYSFS_IBUS] = "ibus",
	[POWER_IF_SYSFS_VBUS] = "vbus",
};

static const char *power_if_get_op_user_name(unsigned int index)
{
	if ((index >= POWER_IF_OP_USER_BEGIN) && (index < POWER_IF_OP_USER_END))
		return g_power_if_op_user_table[index];

	return "illegal op_user";
}

static const char *power_if_get_op_type_name(unsigned int index)
{
	if ((index >= POWER_IF_OP_TYPE_BEGIN) && (index < POWER_IF_OP_TYPE_END))
		return g_power_if_op_type_table[index];

	return "illegal op_type";
}

static const char *power_if_get_sysfs_type_name(unsigned int index)
{
	if ((index >= POWER_IF_SYSFS_BEGIN) && (index < POWER_IF_SYSFS_END))
		return g_power_if_sysfs_type_table[index];

	return "illegal sysfs_type";
}

static int power_if_get_op_user(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_power_if_op_user_table); i++) {
		if (!strcmp(str, g_power_if_op_user_table[i]))
			return i;
	}

	hwlog_err("invalid user_str=%s\n", str);
	return -1;
}

static int power_if_get_op_type(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_power_if_op_type_table); i++) {
		if (!strcmp(str, g_power_if_op_type_table[i]))
			return i;
	}

	hwlog_err("invalid type_str=%s\n", str);
	return -1;
}

static int power_if_check_op_user(unsigned int index)
{
	if ((index >= POWER_IF_OP_USER_BEGIN) && (index < POWER_IF_OP_USER_END))
		return 0;

	hwlog_err("invalid user=%d\n", index);
	return -1;
}

static int power_if_check_op_type(unsigned int index)
{
	if ((index >= POWER_IF_OP_TYPE_BEGIN) && (index < POWER_IF_OP_TYPE_END))
		return 0;

	hwlog_err("invalid type=%d\n", index);
	return -1;
}

static int power_if_check_sysfs_type(unsigned int index)
{
	if ((index >= POWER_IF_SYSFS_BEGIN) && (index < POWER_IF_SYSFS_END))
		return 0;

	hwlog_err("invalid sysfs_type=%d\n", index);
	return -1;
}

static int power_if_check_paras(unsigned int user, unsigned int type,
	unsigned int sysfs_type)
{
	if (power_if_check_op_user(user))
		return -1;

	if (power_if_check_op_type(type))
		return -1;

	if (power_if_check_sysfs_type(sysfs_type))
		return -1;

	return 0;
}

static struct power_if_ops *power_if_get_ops(unsigned int type)
{
	if (!g_power_if_info) {
		hwlog_err("g_power_if_info is null\n");
		return NULL;
	}

	if (!g_power_if_info->ops[type])
		return NULL;

	return g_power_if_info->ops[type];
}

static int power_if_operator_parse_pound(char *str, unsigned int *idx, unsigned int *val)
{
	char *tmp = NULL;

	if (!str || !idx || !val)
		return -1;

	tmp = strsep(&str, "#");
	if (!tmp) {
		hwlog_err("index not found\n");
		return -1;
	}
	if (kstrtouint(tmp, 0, idx))
		return -1;

	tmp = strsep(&str, "#");
	if (!tmp) {
		hwlog_err("value not found\n");
		return -1;
	}
	if (kstrtouint(tmp, 0, val))
		return -1;

	hwlog_info("parsed index: %u, value: %u\n", *idx, *val);
	return 0;
}

static int power_if_operator_get(unsigned int type, unsigned int sysfs_type,
	unsigned int *value)
{
	int ret = POWER_IF_ERRCODE_INVAID_OP;
	struct power_if_ops *l_ops = NULL;

	l_ops = power_if_get_ops(type);
	if (!l_ops)
		return ret;

	switch (sysfs_type) {
	case POWER_IF_SYSFS_ENABLE_CHARGER:
		if (l_ops->get_enable_charger) {
			ret = l_ops->get_enable_charger(value);
			hwlog_info("get enable charger=%d\n", *value);
		}
		break;
	case POWER_IF_SYSFS_VBUS_IIN_LIMIT:
		if (l_ops->get_iin_limit) {
			ret = l_ops->get_iin_limit(value);
			hwlog_info("get vbus iin_limit=%d\n", *value);
		}
		break;
	case POWER_IF_SYSFS_BATT_ICHG_LIMIT:
		if (l_ops->get_ichg_limit) {
			ret = l_ops->get_ichg_limit(value);
			hwlog_info("get battery ichg_limit=%d\n", *value);
		}
		break;
	case POWER_IF_SYSFS_BATT_ICHG_RATIO:
		if (l_ops->get_ichg_ratio) {
			ret = l_ops->get_ichg_ratio(value);
			hwlog_info("get battery ichg_ratio=%d\n", *value);
		}
		break;
	case POWER_IF_SYSFS_BATT_VTERM_DEC:
		if (l_ops->get_vterm_dec) {
			ret = l_ops->get_vterm_dec(value);
			hwlog_info("get battery vterm_dec=%d\n", *value);
		}
		break;
	case POWER_IF_SYSFS_RT_TEST_TIME:
		if (l_ops->get_rt_test_time) {
			ret = l_ops->get_rt_test_time(value);
			hwlog_info("get rt_test_time=%d\n", *value);
		}
		break;
	case POWER_IF_SYSFS_RT_TEST_RESULT:
		if (l_ops->get_rt_test_result) {
			ret = l_ops->get_rt_test_result(value);
			hwlog_info("get rt_test_result=%d\n", *value);
		}
		break;
	case POWER_IF_SYSFS_HOTA_IIN_LIMIT:
		if (l_ops->get_hota_iin_limit) {
			ret = l_ops->get_hota_iin_limit(value);
			hwlog_info("get hota_iin_limit=%d\n", *value);
		}
		break;
	case POWER_IF_SYSFS_STARTUP_IIN_LIMIT:
		if (l_ops->get_startup_iin_limit) {
			ret = l_ops->get_startup_iin_limit(value);
			hwlog_info("get startup_iin_limit=%d\n", *value);
		}
		break;
	case POWER_IF_SYSFS_ENABLE:
		if (l_ops->get_enable) {
			ret = l_ops->get_enable(value);
			hwlog_info("get enable=%d\n", *value);
		}
		break;
	case POWER_IF_SYSFS_READY:
		if (l_ops->get_ready) {
			ret = l_ops->get_ready(value);
			hwlog_info("get ready=%d\n", *value);
		}
		break;
	case POWER_IF_SYSFS_VBUS_PWR_LIMIT:
		if (l_ops->get_pwr_limit) {
			ret = l_ops->get_pwr_limit(value);
			hwlog_info("get pwr_limit=%d\n", *value);
		}
		break;
	case POWER_IF_SYSFS_BATT_ICHG_THERMAL:
		if (l_ops->get_ichg_thermal) {
			ret = l_ops->get_ichg_thermal(value);
			hwlog_info("get battery ichg_thermal=%d\n", *value);
		}
		break;
	case POWER_IF_SYSFS_ADAPTOR_VOLTAGE:
		if (l_ops->get_adap_volt) {
			ret = l_ops->get_adap_volt(value);
			hwlog_info("get adaptor voltage=%d\n", *value);
		}
		break;
	case POWER_IF_SYSFS_WIRELESS_THERMAL_CTRL:
		if (l_ops->get_wl_thermal_ctrl) {
			ret = l_ops->get_wl_thermal_ctrl((unsigned char *)value);
			hwlog_info("get wireless charger thermal control=%d\n", *value);
		}
		break;
	case POWER_IF_SYSFS_IBUS:
		if (l_ops->get_ibus) {
			ret = l_ops->get_ibus((int *)value);
			hwlog_info("get ibus=%d\n", *value);
		}
		break;
	case POWER_IF_SYSFS_VBUS:
		if (l_ops->get_vbus) {
			ret = l_ops->get_vbus((int *)value);
			hwlog_info("get vbus=%d\n", *value);
		}
		break;
	default:
		break;
	}

	return ret;
}

static int power_if_operator_set(unsigned int type, unsigned int sysfs_type,
	char *value_str)
{
	int ret = 0;
	unsigned int value = 0;
	unsigned int index;
	struct power_if_ops *l_ops = NULL;

	l_ops = power_if_get_ops(type);
	if (!l_ops)
		return ret;
	kstrtouint(value_str, 0, &value);

	switch (sysfs_type) {
	case POWER_IF_SYSFS_ENABLE_CHARGER:
		if (l_ops->set_enable_charger)
			ret = l_ops->set_enable_charger(value);
		break;
	case POWER_IF_SYSFS_VBUS_IIN_LIMIT:
		if (l_ops->set_iin_limit)
			ret = l_ops->set_iin_limit(value);
		break;
	case POWER_IF_SYSFS_BATT_ICHG_LIMIT:
		if (l_ops->set_ichg_limit)
			ret = l_ops->set_ichg_limit(value);
		break;
	case POWER_IF_SYSFS_BATT_ICHG_RATIO:
		if (l_ops->set_ichg_ratio)
			ret = l_ops->set_ichg_ratio(value);
		break;
	case POWER_IF_SYSFS_BATT_VTERM_DEC:
		if (l_ops->set_vterm_dec)
			ret = l_ops->set_vterm_dec(value);
		break;
	case POWER_IF_SYSFS_ENABLE:
		if (l_ops->set_enable)
			ret = l_ops->set_enable(value);
		break;
	case POWER_IF_SYSFS_READY:
		if (l_ops->set_ready)
			ret = l_ops->set_ready(value);
		break;
	case POWER_IF_SYSFS_VBUS_PWR_LIMIT:
		if (l_ops->set_pwr_limit)
			ret = l_ops->set_pwr_limit(value);
		break;
	case POWER_IF_SYSFS_VBUS_IIN_THERMAL:
		if (l_ops->set_iin_thermal &&
			!power_if_operator_parse_pound(value_str, &index, &value))
			ret = l_ops->set_iin_thermal(index, value);
		break;
	case POWER_IF_SYSFS_VBUS_IIN_THERMAL_ALL:
		if (l_ops->set_iin_thermal_all)
			ret = l_ops->set_iin_thermal_all(value);
		break;
	case POWER_IF_SYSFS_BATT_ICHG_THERMAL:
		if (l_ops->set_ichg_thermal)
			ret = l_ops->set_ichg_thermal(value);
		break;
	case POWER_IF_SYSFS_ADAPTOR_VOLTAGE:
		if (l_ops->set_adap_volt)
			ret = l_ops->set_adap_volt(value);
		break;
	case POWER_IF_SYSFS_WIRELESS_THERMAL_CTRL:
		if (l_ops->set_wl_thermal_ctrl)
			ret = l_ops->set_wl_thermal_ctrl(value);
		break;
	case POWER_IF_SYSFS_RTB_SUCCESS:
		if (l_ops->set_rtb_success)
			ret = l_ops->set_rtb_success(value);
		break;
	default:
		break;
	}

	return ret;
}

static int power_if_common_sysfs_get(unsigned int user, unsigned int type,
	unsigned int sysfs_type, char *buf)
{
	int ret;
	unsigned int value;
	unsigned int type_s;
	unsigned int type_e;
	char rd_buf[POWER_IF_RD_BUF_SIZE] = {0};

	if (power_if_check_paras(user, type, sysfs_type))
		return -1;

	hwlog_info("sysfs_get=%s user=%s type=%s\n",
		power_if_get_sysfs_type_name(sysfs_type),
		power_if_get_op_user_name(user),
		power_if_get_op_type_name(type));

	if (type == POWER_IF_OP_TYPE_ALL) {
		/* loop all type */
		type_s = POWER_IF_OP_TYPE_BEGIN;
		type_e = type;
	} else {
		/* specified one type */
		type_s = type;
		type_e = type + 1;
	}

	/* output all value */
	for (; type_s < type_e; type_s++) {
		value = 0;
		ret = power_if_operator_get(type_s, sysfs_type, &value);
		/* if op is invalid, must be skip output */
		if (ret == POWER_IF_ERRCODE_INVAID_OP)
			continue;

		memset(rd_buf, 0, POWER_IF_RD_BUF_SIZE);
		if (ret == 0)
			scnprintf(rd_buf, POWER_IF_RD_BUF_SIZE, "%s %d\n",
				power_if_get_op_type_name(type_s), value);
		else
			scnprintf(rd_buf, POWER_IF_RD_BUF_SIZE, "%s -1\n",
				power_if_get_op_type_name(type_s));

		strncat(buf, rd_buf, strlen(rd_buf));
	}

	return strlen(buf);
}

static int power_if_common_sysfs_set(unsigned int user, unsigned int type,
	unsigned int sysfs_type, char *value_str)
{
	int ret = 0;
	unsigned int type_s;
	unsigned int type_e;

	if (power_if_check_paras(user, type, sysfs_type))
		return -1;

	hwlog_info("sysfs_set=%s user=%s type=%s value=%s\n",
		power_if_get_sysfs_type_name(sysfs_type),
		power_if_get_op_user_name(user),
		power_if_get_op_type_name(type), value_str);

	if (type == POWER_IF_OP_TYPE_ALL) {
		/* loop all type */
		type_s = POWER_IF_OP_TYPE_BEGIN;
		type_e = type;
	} else {
		/* specified one type */
		type_s = type;
		type_e = type + 1;
	}

	for (; type_s < type_e; type_s++)
		ret |= power_if_operator_set(type_s, sysfs_type, value_str);

	return ret;
}

int power_if_kernel_sysfs_get(unsigned int type, unsigned int sysfs_type,
	unsigned int *value)
{
	hwlog_info("sysfs_get=%s user=%s type=%s\n",
		power_if_get_sysfs_type_name(sysfs_type),
		power_if_get_op_user_name(POWER_IF_OP_USER_KERNEL),
		power_if_get_op_type_name(type));

	return power_if_operator_get(type, sysfs_type, value);
}

/*lint -e580*/
int power_if_kernel_sysfs_set(unsigned int type, unsigned int sysfs_type,
	unsigned int value)
{
	char value_str[POWER_IF_RD_BUF_SIZE] = {0};

	snprintf(value_str, POWER_IF_RD_BUF_SIZE, "%d", value);
	return power_if_common_sysfs_set(POWER_IF_OP_USER_KERNEL, type,
		sysfs_type, value_str);
}
EXPORT_SYMBOL_GPL(power_if_kernel_sysfs_set);
/*lint +e580*/

#ifdef CONFIG_SYSFS
static ssize_t power_if_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t power_if_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info power_if_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(power_if, 0640, POWER_IF_SYSFS_ENABLE_CHARGER, enable_charger),
	power_sysfs_attr_rw(power_if, 0640, POWER_IF_SYSFS_VBUS_IIN_LIMIT, iin_limit),
	power_sysfs_attr_rw(power_if, 0640, POWER_IF_SYSFS_BATT_ICHG_LIMIT, ichg_limit),
	power_sysfs_attr_rw(power_if, 0640, POWER_IF_SYSFS_BATT_ICHG_RATIO, ichg_ratio),
	power_sysfs_attr_rw(power_if, 0640, POWER_IF_SYSFS_BATT_VTERM_DEC, vterm_dec),
	power_sysfs_attr_ro(power_if, 0440, POWER_IF_SYSFS_RT_TEST_TIME, rt_test_time),
	power_sysfs_attr_ro(power_if, 0440, POWER_IF_SYSFS_RT_TEST_RESULT, rt_test_result),
	power_sysfs_attr_ro(power_if, 0440, POWER_IF_SYSFS_HOTA_IIN_LIMIT, hota_iin_limit),
	power_sysfs_attr_ro(power_if, 0440, POWER_IF_SYSFS_STARTUP_IIN_LIMIT, startup_iin_limit),
	power_sysfs_attr_rw(power_if, 0640, POWER_IF_SYSFS_ENABLE, enable),
	power_sysfs_attr_rw(power_if, 0640, POWER_IF_SYSFS_READY, ready),
	power_sysfs_attr_rw(power_if, 0640, POWER_IF_SYSFS_VBUS_PWR_LIMIT, power_limit),
	power_sysfs_attr_wo(power_if, 0220, POWER_IF_SYSFS_VBUS_IIN_THERMAL, iin_thermal),
	power_sysfs_attr_wo(power_if, 0220, POWER_IF_SYSFS_VBUS_IIN_THERMAL_ALL, iin_thermal_all),
	power_sysfs_attr_rw(power_if, 0640, POWER_IF_SYSFS_BATT_ICHG_THERMAL, ichg_thermal),
	power_sysfs_attr_rw(power_if, 0640, POWER_IF_SYSFS_ADAPTOR_VOLTAGE, adap_volt),
	power_sysfs_attr_rw(power_if, 0640, POWER_IF_SYSFS_WIRELESS_THERMAL_CTRL, wl_thermal_ctrl),
	power_sysfs_attr_wo(power_if, 0220, POWER_IF_SYSFS_RTB_SUCCESS, rtb_success),
	power_sysfs_attr_ro(power_if, 0440, POWER_IF_SYSFS_IBUS, ibus),
	power_sysfs_attr_ro(power_if, 0440, POWER_IF_SYSFS_VBUS, vbus),
};

#define POWER_IF_SYSFS_ATTRS_SIZE  ARRAY_SIZE(power_if_sysfs_field_tbl)

static struct attribute *power_if_sysfs_attrs[POWER_IF_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group power_if_sysfs_attr_group = {
	.attrs = power_if_sysfs_attrs,
};

static ssize_t power_if_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		power_if_sysfs_field_tbl, POWER_IF_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	return power_if_common_sysfs_get(POWER_IF_OP_USER_DEFAULT,
		POWER_IF_OP_TYPE_ALL, info->name, buf);
}

static ssize_t power_if_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	char user_name[POWER_IF_RD_BUF_SIZE] = {0};
	char type_name[POWER_IF_RD_BUF_SIZE] = {0};
	char value_str[POWER_IF_RD_BUF_SIZE] = {0};
	int user;
	int type;

	info = power_sysfs_lookup_attr(attr->attr.name,
		power_if_sysfs_field_tbl, POWER_IF_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	/* reserve 2 bytes to prevent buffer overflow */
	if (count >= (POWER_IF_RD_BUF_SIZE - 2)) {
		hwlog_err("input too long\n");
		return -EINVAL;
	}

	/* 3: the fields of "user type value" */
	if (sscanf(buf, "%s %s %s", user_name, type_name, value_str) != 3) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	user = power_if_get_op_user(user_name);
	if (user < 0)
		return -EINVAL;

	type = power_if_get_op_type(type_name);
	if (type < 0)
		return -EINVAL;

	power_if_common_sysfs_set(user, type, info->name, value_str);

	return count;
}

static struct device *power_if_sysfs_create_group(void)
{
	power_sysfs_init_attrs(power_if_sysfs_attrs,
		power_if_sysfs_field_tbl, POWER_IF_SYSFS_ATTRS_SIZE);
	return power_sysfs_create_group("hw_power", "interface",
		&power_if_sysfs_attr_group);
}

static void power_if_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &power_if_sysfs_attr_group);
}
#else
static inline struct device *power_if_sysfs_create_group(void)
{
	return NULL;
}

static inline void power_if_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

int power_if_ops_register(struct power_if_ops *ops)
{
	int type;

	if (!g_power_if_info || !ops || !ops->type_name) {
		hwlog_err("g_power_if_info or ops is null\n");
		return -EPERM;
	}

	type = power_if_get_op_type(ops->type_name);
	if (type < 0) {
		hwlog_err("%s ops register fail\n", ops->type_name);
		return -EPERM;
	}

	g_power_if_info->ops[type] = ops;
	g_power_if_info->total_ops++;

	hwlog_info("total_ops=%d type=%d:%s ops register ok\n",
		g_power_if_info->total_ops, type, ops->type_name);

	return 0;
}

static int __init power_interface_init(void)
{
	struct power_if_device_info *info = NULL;

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	g_power_if_info = info;
	info->dev = power_if_sysfs_create_group();

	return 0;
}

static void __exit power_interface_exit(void)
{
	struct power_if_device_info *info = g_power_if_info;

	if (!info)
		return;

	power_if_sysfs_remove_group(info->dev);
	kfree(info);
	g_power_if_info = NULL;
}

fs_initcall_sync(power_interface_init);
module_exit(power_interface_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("power interface module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
