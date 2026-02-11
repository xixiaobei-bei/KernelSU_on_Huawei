/*
 * adapter_protocol.c
 *
 * adapter protocol driver
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>

#include <linux/power/hisi/hisi_bci_battery.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/adapter_protocol/adapter_protocol.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG adapter
HWLOG_REGIST();

static struct adapter_dev *g_adapter_dev;

static const char * const adapter_protocol_type_table[] = {
	[ADAPTER_PROTOCOL_SCP] = "hw_scp",
	[ADAPTER_PROTOCOL_FCP] = "hw_fcp",
};

static struct adapter_support_mode_data adapter_support_mode_table[] = {
	{ ADAPTER_SUPPORT_UNDEFINED, "undefined" },
	{ ADAPTER_SUPPORT_SCP_B_LVC, "scp_b_lvc" },
	{ ADAPTER_SUPPORT_SCP_B_SC, "scp_b_sc" },
	{ ADAPTER_SUPPORT_FCP, "fcp" },
};

static struct adapter_update_mode_data adapter_update_mode_table[] = {
	{ ADAPTER_TYPE_20V3P25A, ADAPTER_SUPPORT_SCP_B_SC },
};

static int adapter_check_protocol_type(int prot)
{
	if ((prot >= ADAPTER_PROTOCOL_BEGIN) && (prot < ADAPTER_PROTOCOL_END))
		return 0;

	hwlog_err("invalid protocol_type=%d\n", prot);
	return -1;
}

static int adapter_get_protocol_type(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(adapter_protocol_type_table); i++) {
		if (!strncmp(str, adapter_protocol_type_table[i], strlen(str)))
			return i;
	}

	hwlog_err("invalid protocol_type_str=%s\n", str);
	return -1;
}

static int adapter_get_support_mode_string(int mode, char *buf)
{
	unsigned int i;
	char rd_buf[ADAPTER_RD_BUF_SIZE] = {0};

	if (mode == ADAPTER_SUPPORT_UNDEFINED) {
		scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%s", "undefined ");
		return strlen(buf);
	}

	for (i = 0; i < ARRAY_SIZE(adapter_support_mode_table); i++) {
		if (mode & adapter_support_mode_table[i].mode) {
			memset(rd_buf, 0, ADAPTER_RD_BUF_SIZE);
			scnprintf(rd_buf, ADAPTER_RD_BUF_SIZE, "%s ",
				adapter_support_mode_table[i].mode_name);
			strncat(buf, rd_buf, strlen(rd_buf));
		}
	}

	return strlen(buf);
}

static struct adapter_dev *adapter_get_dev(void)
{
	if (!g_adapter_dev) {
		hwlog_err("g_adapter_dev is null\n");
		return NULL;
	}

	return g_adapter_dev;
}

static struct adapter_protocol_ops *adapter_get_protocol_ops(int prot)
{
	if (adapter_check_protocol_type(prot))
		return NULL;

	if (!g_adapter_dev || !g_adapter_dev->p_ops[prot]) {
		hwlog_err("g_adapter_dev or p_ops is null\n");
		return NULL;
	}

	return g_adapter_dev->p_ops[prot];
}

int adapter_protocol_ops_register(struct adapter_protocol_ops *ops)
{
	int type;

	if (!g_adapter_dev || !ops || !ops->type_name) {
		hwlog_err("g_adapter_dev or ops or type_name is null\n");
		return -1;
	}

	type = adapter_get_protocol_type(ops->type_name);
	if (type < 0) {
		hwlog_err("%s ops register fail\n", ops->type_name);
		return -1;
	}

	g_adapter_dev->p_ops[type] = ops;
	g_adapter_dev->total_ops++;

	hwlog_info("total_ops=%d type=%d:%s ops register ok\n",
		g_adapter_dev->total_ops, type, ops->type_name);

	return 0;
}

int adapter_soft_reset_master(int prot)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->soft_reset_master) {
		hwlog_err("soft_reset_master is null\n");
		return -1;
	}

	return l_ops->soft_reset_master();
}

int adapter_soft_reset_slave(int prot)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->soft_reset_slave) {
		hwlog_err("soft_reset_slave is null\n");
		return -1;
	}

	return l_ops->soft_reset_slave();
}

int adapter_detect_adapter_support_mode(int prot, int *mode)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops || !mode)
		return -1;

	if (!l_ops->detect_adapter_support_mode) {
		hwlog_err("detect_adapter_support_mode is null\n");
		return -1;
	}

	return l_ops->detect_adapter_support_mode(mode);
}

int adapter_get_support_mode(int prot, int *mode)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops || !mode)
		return -1;

	if (!l_ops->get_support_mode) {
		hwlog_err("get_support_mode is null\n");
		return -1;
	}

	return l_ops->get_support_mode(mode);
}

void adapter_update_adapter_support_mode(int prot, unsigned int *mode)
{
	int ret;
	unsigned int i;
	int adp_type = ADAPTER_TYPE_UNKNOWN;
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops || !mode)
		return;

	ret = adapter_get_adp_type(prot, &adp_type);
	if (ret) {
		hwlog_err("get adp type failed\n");
		return;
	}

	for (i = 0; i < ARRAY_SIZE(adapter_update_mode_table); i++) {
		if (adp_type == adapter_update_mode_table[i].adp_type) {
			*mode = adapter_update_mode_table[i].mode;
			break;
		}
	}

	if (i == ARRAY_SIZE(adapter_update_mode_table))
		hwlog_err("lookup adp type failed, no need to update\n");
}

int adapter_get_device_info(int prot)
{
	struct adapter_protocol_ops *l_ops = NULL;
	struct adapter_dev *l_dev = NULL;

	l_dev = adapter_get_dev();
	if (!l_dev)
		return -1;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->get_device_info) {
		hwlog_err("get_device_info is null\n");
		return -1;
	}

	return l_ops->get_device_info(&l_dev->info);
}

int adapter_show_device_info(int prot)
{
	struct adapter_dev *l_dev = NULL;

	l_dev = adapter_get_dev();
	if (!l_dev)
		return -1;

	switch (prot) {
	case ADAPTER_PROTOCOL_SCP:
		hwlog_info("support_mode=0x%x\n", l_dev->info.support_mode);
		hwlog_info("chip_id=0x%x\n", l_dev->info.chip_id);
		hwlog_info("vendor_id=0x%x\n", l_dev->info.vendor_id);
		hwlog_info("module_id=0x%x\n", l_dev->info.module_id);
		hwlog_info("serial_no=0x%x\n", l_dev->info.serial_no);
		hwlog_info("hwver=0x%x\n", l_dev->info.hwver);
		hwlog_info("fwver=0x%x\n", l_dev->info.fwver);
		hwlog_info("min_volt=%d\n", l_dev->info.min_volt);
		hwlog_info("max_volt=%d\n", l_dev->info.max_volt);
		hwlog_info("min_cur=%d\n", l_dev->info.min_cur);
		hwlog_info("max_cur=%d\n", l_dev->info.max_cur);
		hwlog_info("adp_type=%d\n", l_dev->info.adp_type);
		break;
	case ADAPTER_PROTOCOL_FCP:
		hwlog_info("support_mode=0x%x\n", l_dev->info.support_mode);
		hwlog_info("vendor_id=0x%x\n", l_dev->info.vendor_id);
		hwlog_info("volt_cap=%d\n", l_dev->info.volt_cap);
		hwlog_info("max_volt=%d\n", l_dev->info.max_volt);
		hwlog_info("max_pwr=%d\n", l_dev->info.max_pwr);
		break;
	default:
		break;
	}


	return 0;
}

int adapter_get_chip_vendor_id(int prot, int *id)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops || !id)
		return -1;

	if (!l_ops->get_chip_vendor_id) {
		hwlog_err("get_chip_vendor_id is null\n");
		return -1;
	}

	return l_ops->get_chip_vendor_id(id);
}

int adapter_set_output_enable(int prot, int enable)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->set_output_enable) {
		hwlog_err("set_output_enable is null\n");
		return -1;
	}

	return l_ops->set_output_enable(enable);
}

int adapter_set_reset(int prot, int enable)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->set_reset) {
		hwlog_err("set_reset is null\n");
		return -1;
	}

	return l_ops->set_reset(enable);
}

int adapter_set_output_voltage(int prot, int volt)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->set_output_voltage) {
		hwlog_err("set_output_voltage is null\n");
		return -1;
	}

	return l_ops->set_output_voltage(volt);
}

int adapter_get_output_voltage(int prot, int *volt)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops || !volt)
		return -1;

	if (!l_ops->get_output_voltage) {
		hwlog_err("get_output_voltage is null\n");
		return -1;
	}

	return l_ops->get_output_voltage(volt);
}

int adapter_set_output_current(int prot, int cur)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->set_output_current) {
		hwlog_err("set_output_current is null\n");
		return -1;
	}

	return l_ops->set_output_current(cur);
}

int adapter_get_output_current(int prot, int *cur)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops || !cur)
		return -1;

	if (!l_ops->get_output_current) {
		hwlog_err("get_output_current is null\n");
		return -1;
	}

	return l_ops->get_output_current(cur);
}

int adapter_get_output_current_set(int prot, int *cur)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops || !cur)
		return -1;

	if (!l_ops->get_output_current_set) {
		hwlog_err("get_output_current_set is null\n");
		return -1;
	}

	return l_ops->get_output_current_set(cur);
}

int adapter_get_min_voltage(int prot, int *volt)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops || !volt)
		return -1;

	if (!l_ops->get_min_voltage) {
		hwlog_err("get_min_voltage is null\n");
		return -1;
	}

	return l_ops->get_min_voltage(volt);
}

int adapter_get_max_voltage(int prot, int *volt)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops || !volt)
		return -1;

	if (!l_ops->get_max_voltage) {
		hwlog_err("get_max_voltage is null\n");
		return -1;
	}

	return l_ops->get_max_voltage(volt);
}

int adapter_get_min_current(int prot, int *cur)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops || !cur)
		return -1;

	if (!l_ops->get_min_current) {
		hwlog_err("get_min_current is null\n");
		return -1;
	}

	return l_ops->get_min_current(cur);
}

int adapter_get_max_current(int prot, int *cur)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops || !cur)
		return -1;

	if (!l_ops->get_max_current) {
		hwlog_err("get_max_current is null\n");
		return -1;
	}

	return l_ops->get_max_current(cur);
}

int adapter_get_adp_type(int prot, int *type)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops || !type)
		return -1;

	if (!l_ops->get_adp_type) {
		hwlog_err("get_adp_type is null\n");
		return -1;
	}

	return l_ops->get_adp_type(type);
}

int adapter_get_inside_temp(int prot, int *temp)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops || !temp)
		return -1;

	if (!l_ops->get_inside_temp) {
		hwlog_err("get_inside_temp is null\n");
		return -1;
	}

	return l_ops->get_inside_temp(temp);
}

int adapter_get_port_temp(int prot, int *temp)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops || !temp)
		return -1;

	if (!l_ops->get_port_temp) {
		hwlog_err("get_port_temp is null\n");
		return -1;
	}

	return l_ops->get_port_temp(temp);
}

int adapter_get_port_leakage_current_flag(int prot, int *flag)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops || !flag)
		return -1;

	if (!l_ops->get_port_leakage_current_flag) {
		hwlog_err("get_port_leakage_current_flag is null\n");
		return -1;
	}

	return l_ops->get_port_leakage_current_flag(flag);
}

int adapter_auth_encrypt_start(int prot,
	int key, unsigned char *hash, int size)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops || !hash)
		return -1;

	if (!l_ops->auth_encrypt_start) {
		hwlog_err("auth_encrypt_start is null\n");
		return -1;
	}

	return l_ops->auth_encrypt_start(key, hash, size);
}

int adapter_auth_encrypt_release(int prot)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->auth_encrypt_release) {
		hwlog_err("auth_encrypt_release is null\n");
		return -1;
	}

	return l_ops->auth_encrypt_release();
}

int adapter_set_usbpd_enable(int prot, int enable)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->set_usbpd_enable) {
		hwlog_err("set_usbpd_enable is null\n");
		return -1;
	}

	return l_ops->set_usbpd_enable(enable);
}

int adapter_set_default_state(int prot)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->set_default_state) {
		hwlog_err("set_default_state is null\n");
		return -1;
	}

	return l_ops->set_default_state();
}

int adapter_set_default_param(int prot)
{
	struct adapter_protocol_ops *l_ops = NULL;
	struct adapter_dev *l_dev = NULL;

	l_dev = adapter_get_dev();
	if (!l_dev)
		return -1;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->set_default_param) {
		hwlog_info("set_default_param is null\n");
		return -1;
	}

	memset(&l_dev->info, 0, sizeof(l_dev->info));

	return l_ops->set_default_param();
}

int adapter_set_init_data(int prot, struct adapter_init_data *data)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->set_init_data) {
		hwlog_err("set_init_data is null\n");
		return -1;
	}

	return l_ops->set_init_data(data);
}

int adapter_get_protocol_register_state(int prot)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->get_protocol_register_state) {
		hwlog_err("get_protocol_register_state is null\n");
		return -1;
	}

	return l_ops->get_protocol_register_state();
}

int adapter_get_slave_status(int prot)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->get_slave_status) {
		hwlog_err("get_slave_status is null\n");
		return -1;
	}

	return l_ops->get_slave_status();
}

int adapter_get_master_status(int prot)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->get_master_status) {
		hwlog_err("get_master_status is null\n");
		return -1;
	}

	return l_ops->get_master_status();
}

int adapter_stop_charging_config(int prot)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->stop_charging_config) {
		hwlog_err("stop_charging_config is null\n");
		return -1;
	}

	return l_ops->stop_charging_config();
}

bool adapter_is_accp_charger_type(int prot)
{
	struct adapter_protocol_ops *l_ops = NULL;

	l_ops = adapter_get_protocol_ops(prot);
	if (!l_ops)
		return false;

	if (!l_ops->is_accp_charger_type) {
		hwlog_err("is_accp_charger_type is null\n");
		return false;
	}

	return l_ops->is_accp_charger_type();
}

#ifdef CONFIG_SYSFS
#define ADAPTER_SYSFS_FIELD(_name, n, m, store) \
{ \
	.attr = __ATTR(_name, m, adapter_sysfs_show, store), \
	.name = ADAPTER_SYSFS_##n, \
}

#define ADAPTER_SYSFS_FIELD_RO(_name, n) \
	ADAPTER_SYSFS_FIELD(_name, n, 0440, NULL)

struct adapter_sysfs_field_info {
	struct device_attribute attr;
	u8 name;
};

static ssize_t adapter_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static struct adapter_sysfs_field_info adapter_sysfs_field_tbl[] = {
	ADAPTER_SYSFS_FIELD_RO(support_mode, SUPPORT_MODE),
	ADAPTER_SYSFS_FIELD_RO(chip_id, CHIP_ID),
	ADAPTER_SYSFS_FIELD_RO(vendor_id, VENDOR_ID),
	ADAPTER_SYSFS_FIELD_RO(module_id, MODULE_ID),
	ADAPTER_SYSFS_FIELD_RO(serial_no, SERIAL_NO),
	ADAPTER_SYSFS_FIELD_RO(hwver, HWVER),
	ADAPTER_SYSFS_FIELD_RO(fwver, FWVER),
	ADAPTER_SYSFS_FIELD_RO(min_volt, MIN_VOLT),
	ADAPTER_SYSFS_FIELD_RO(max_volt, MAX_VOLT),
	ADAPTER_SYSFS_FIELD_RO(min_cur, MIN_CUR),
	ADAPTER_SYSFS_FIELD_RO(max_cur, MAX_CUR),
	ADAPTER_SYSFS_FIELD_RO(adapter_type, ADP_TYPE),
};

#define ADAPTER_SYSFS_ATTRS_SIZE  (ARRAY_SIZE(adapter_sysfs_field_tbl) + 1)

static struct attribute *adapter_sysfs_attrs[ADAPTER_SYSFS_ATTRS_SIZE];

static const struct attribute_group adapter_sysfs_attr_group = {
	.attrs = adapter_sysfs_attrs,
};

static void adapter_sysfs_init_attrs(void)
{
	int s;
	int e = ARRAY_SIZE(adapter_sysfs_field_tbl);

	for (s = 0; s < e; s++)
		adapter_sysfs_attrs[s] = &adapter_sysfs_field_tbl[s].attr.attr;

	adapter_sysfs_attrs[e] = NULL;
}

static struct adapter_sysfs_field_info *adapter_sysfs_field_lookup(
	const char *name)
{
	int s;
	int e = ARRAY_SIZE(adapter_sysfs_field_tbl);

	for (s = 0; s < e; s++) {
		if (!strncmp(name, adapter_sysfs_field_tbl[s].attr.attr.name,
				strlen(name)))
			break;
	}

	if (s >= e)
		return NULL;

	return &adapter_sysfs_field_tbl[s];
}

static ssize_t adapter_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct adapter_sysfs_field_info *info = NULL;
	struct adapter_dev *l_dev = NULL;
	int len = 0;

	l_dev = adapter_get_dev();
	if (!l_dev)
		return -EINVAL;

	info = adapter_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case ADAPTER_SYSFS_SUPPORT_MODE:
		len = adapter_get_support_mode_string(l_dev->info.support_mode,
			buf);
		break;
	case ADAPTER_SYSFS_CHIP_ID:
		len = scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%x\n",
			l_dev->info.chip_id);
		break;
	case ADAPTER_SYSFS_VENDOR_ID:
		len = scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%x\n",
			l_dev->info.vendor_id);
		break;
	case ADAPTER_SYSFS_MODULE_ID:
		len = scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%x\n",
			l_dev->info.module_id);
		break;
	case ADAPTER_SYSFS_SERIAL_NO:
		len = scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%x\n",
			l_dev->info.serial_no);
		break;
	case ADAPTER_SYSFS_HWVER:
		len = scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%x\n",
			l_dev->info.hwver);
		break;
	case ADAPTER_SYSFS_FWVER:
		len = scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%x\n",
			l_dev->info.fwver);
		break;
	case ADAPTER_SYSFS_MIN_VOLT:
		len = scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%d\n",
			l_dev->info.min_volt);
		break;
	case ADAPTER_SYSFS_MAX_VOLT:
		len = scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%d\n",
			l_dev->info.max_volt);
		break;
	case ADAPTER_SYSFS_MIN_CUR:
		len = scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%d\n",
			l_dev->info.min_cur);
		break;
	case ADAPTER_SYSFS_MAX_CUR:
		len = scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%d\n",
			l_dev->info.max_cur);
		break;
	case ADAPTER_SYSFS_ADP_TYPE:
		len = scnprintf(buf, ADAPTER_RD_BUF_SIZE, "%d\n",
			l_dev->info.adp_type);
		break;
	default:
		hwlog_err("invalid sysfs_name\n");
		break;
	}

	return len;
}
#endif /* CONFIG_SYSFS */

static int __init adapter_init(void)
{
#ifdef CONFIG_SYSFS
	int ret;
	struct class *power_class = NULL;
#endif /* CONFIG_SYSFS */
	struct adapter_dev *l_dev = NULL;

	hwlog_info("probe begin\n");

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_adapter_dev = l_dev;

#ifdef CONFIG_SYSFS
	adapter_sysfs_init_attrs();
	power_class = hw_power_get_class();
	if (power_class) {
		l_dev->dev = device_create(power_class, NULL, 0, NULL,
			"adapter");
		if (IS_ERR(l_dev->dev)) {
			hwlog_err("sysfs device create failed\n");
			ret = PTR_ERR(l_dev->dev);
			goto fail_create_device;
		}

		ret = sysfs_create_group(&l_dev->dev->kobj,
			&adapter_sysfs_attr_group);
		if (ret) {
			hwlog_err("sysfs group create failed\n");
			goto fail_create_sysfs;
		}
	}
#endif /* CONFIG_SYSFS */

	hwlog_info("probe end\n");
	return 0;

#ifdef CONFIG_SYSFS
fail_create_sysfs:
fail_create_device:
	kfree(l_dev);
	g_adapter_dev = NULL;

	return ret;
#endif /* CONFIG_SYSFS */
}

static void __exit adapter_exit(void)
{
	hwlog_info("remove begin\n");

#ifdef CONFIG_SYSFS
	sysfs_remove_group(&g_adapter_dev->dev->kobj,
		&adapter_sysfs_attr_group);
	kobject_del(&g_adapter_dev->dev->kobj);
	kobject_put(&g_adapter_dev->dev->kobj);
#endif /* CONFIG_SYSFS */

	kfree(g_adapter_dev);
	g_adapter_dev = NULL;

	hwlog_info("remove ok\n");
}

subsys_initcall_sync(adapter_init);
module_exit(adapter_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("adapter protocol driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
