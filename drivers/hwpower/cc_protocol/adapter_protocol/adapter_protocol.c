/*
 * adapter_protocol.c
 *
 * adapter protocol driver
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
#include <chipset_common/hwpower/protocol/adapter_protocol.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG adapter_prot
HWLOG_REGIST();

static struct adapter_protocol_dev *g_adapter_protocol_dev;

static const char * const g_adapter_protocol_type_table[] = {
	[ADAPTER_PROTOCOL_SCP] = "hw_scp",
	[ADAPTER_PROTOCOL_FCP] = "hw_fcp",
	[ADAPTER_PROTOCOL_PD] = "hw_pd",
	[ADAPTER_PROTOCOL_UVDM] = "hw_uvdm",
};

static struct adapter_update_mode_data g_adapter_update_mode_table[] = {
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

	for (i = 0; i < ARRAY_SIZE(g_adapter_protocol_type_table); i++) {
		if (!strncmp(str, g_adapter_protocol_type_table[i], strlen(str)))
			return i;
	}

	hwlog_err("invalid protocol_type_str=%s\n", str);
	return -1;
}

struct adapter_protocol_dev *adapter_get_protocol_dev(void)
{
	if (!g_adapter_protocol_dev) {
		hwlog_err("g_adapter_protocol_dev is null\n");
		return NULL;
	}

	return g_adapter_protocol_dev;
}

static struct adapter_protocol_ops *adapter_get_protocol_ops(int prot)
{
	if (adapter_check_protocol_type(prot))
		return NULL;

	if (!g_adapter_protocol_dev || !g_adapter_protocol_dev->p_ops[prot]) {
		hwlog_err("g_adapter_protocol_dev or p_ops is null\n");
		return NULL;
	}

	return g_adapter_protocol_dev->p_ops[prot];
}

int adapter_protocol_ops_register(struct adapter_protocol_ops *ops)
{
	int type;

	if (!g_adapter_protocol_dev || !ops || !ops->type_name) {
		hwlog_err("g_adapter_protocol_dev or ops or type_name is null\n");
		return -1;
	}

	type = adapter_get_protocol_type(ops->type_name);
	if (type < 0) {
		hwlog_err("%s ops register fail\n", ops->type_name);
		return -1;
	}

	g_adapter_protocol_dev->p_ops[type] = ops;
	g_adapter_protocol_dev->total_ops++;

	hwlog_info("total_ops=%d type=%d:%s ops register ok\n",
		g_adapter_protocol_dev->total_ops, type, ops->type_name);

	return 0;
}

int adapter_soft_reset_master(int prot)
{
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

	if (!l_ops)
		return -1;

	if (!l_ops->soft_reset_slave) {
		hwlog_err("soft_reset_slave is null\n");
		return -1;
	}

	return l_ops->soft_reset_slave();
}

int adapter_hard_reset_master(int prot)
{
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

	if (!l_ops)
		return -1;

	if (!l_ops->hard_reset_master) {
		hwlog_err("hard_reset_master is null\n");
		return -1;
	}

	return l_ops->hard_reset_master();
}

int adapter_detect_adapter_support_mode(int prot, int *mode)
{
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

	if (!l_ops || !mode)
		return;

	ret = adapter_get_adp_type(prot, &adp_type);
	if (ret) {
		hwlog_err("get adp type failed\n");
		return;
	}

	for (i = 0; i < ARRAY_SIZE(g_adapter_update_mode_table); i++) {
		if (adp_type == g_adapter_update_mode_table[i].adp_type) {
			*mode = g_adapter_update_mode_table[i].mode;
			break;
		}
	}

	if (i == ARRAY_SIZE(g_adapter_update_mode_table))
		hwlog_err("lookup adp type failed, no need to update\n");
}

int adapter_get_device_info(int prot)
{
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);
	struct adapter_protocol_dev *l_dev = adapter_get_protocol_dev();

	if (!l_ops || !l_dev)
		return -1;

	if (!l_ops->get_device_info) {
		hwlog_err("get_device_info is null\n");
		return -1;
	}

	return l_ops->get_device_info(&l_dev->info);
}

int adapter_show_device_info(int prot)
{
	struct adapter_protocol_dev *l_dev = adapter_get_protocol_dev();

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

	if (!l_ops || !id)
		return -1;

	if (!l_ops->get_chip_vendor_id) {
		hwlog_err("get_chip_vendor_id is null\n");
		return -1;
	}

	return l_ops->get_chip_vendor_id(id);
}

int adapter_get_chip_serial_num(int prot, int *num)
{
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

	if (!l_ops || !num)
		return -1;

	if (!l_ops->get_chip_serial_num) {
		hwlog_err("get_chip_serial_num is null\n");
		return -1;
	}

	return l_ops->get_chip_serial_num(num);
}

int adapter_set_output_enable(int prot, int enable)
{
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

	if (!l_ops)
		return -1;

	if (!l_ops->set_output_enable) {
		hwlog_err("set_output_enable is null\n");
		return -1;
	}

	return l_ops->set_output_enable(enable);
}

int adapter_set_output_mode(int prot, int enable)
{
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

	if (!l_ops)
		return -1;

	if (!l_ops->set_output_mode) {
		hwlog_err("set_output_mode is null\n");
		return -1;
	}

	return l_ops->set_output_mode(enable);
}

int adapter_set_reset(int prot, int enable)
{
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

	if (!l_ops || !cur)
		return -1;

	if (!l_ops->get_max_current) {
		hwlog_err("get_max_current is null\n");
		return -1;
	}

	return l_ops->get_max_current(cur);
}

int adapter_get_power_drop_current(int prot, int *cur)
{
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

	if (!l_ops || !cur)
		return -1;

	if (!l_ops->get_power_drop_current) {
		hwlog_err("get_power_drop_current is null\n");
		return -1;
	}

	return l_ops->get_power_drop_current(cur);
}

int adapter_get_power_curve_num(int prot, int *num)
{
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

	if (!l_ops || !num)
		return -1;

	if (!l_ops->get_power_curve_num) {
		hwlog_err("get_power_curve_num is null\n");
		return -1;
	}

	return l_ops->get_power_curve_num(num);
}

int adapter_get_power_curve(int prot, int *val, unsigned int size)
{
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

	if (!l_ops || !val)
		return -1;

	if (!l_ops->get_power_curve) {
		hwlog_err("get_power_curve is null\n");
		return -1;
	}

	return l_ops->get_power_curve(val, size);
}

int adapter_get_adp_type(int prot, int *type)
{
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);
	struct adapter_protocol_dev *l_dev = adapter_get_protocol_dev();

	if (!l_ops || !l_dev)
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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

	if (!l_ops)
		return -1;

	if (!l_ops->set_init_data) {
		hwlog_err("set_init_data is null\n");
		return -1;
	}

	return l_ops->set_init_data(data);
}

int adapter_set_slave_power_mode(int prot, int mode)
{
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

	if (!l_ops)
		return -1;

	if (!l_ops->set_slave_power_mode) {
		hwlog_err("set_slave_power_mode is null\n");
		return -1;
	}

	return l_ops->set_slave_power_mode(mode);
}

int adapter_set_rx_reduce_voltage(int prot)
{
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

	if (!l_ops)
		return -1;

	if (!l_ops->set_rx_reduce_voltage) {
		hwlog_err("set_rx_reduce_voltage is null\n");
		return -1;
	}

	return l_ops->set_rx_reduce_voltage();
}

int adapter_get_protocol_register_state(int prot)
{
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

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
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

	if (!l_ops)
		return false;

	if (!l_ops->is_accp_charger_type) {
		hwlog_err("is_accp_charger_type is null\n");
		return false;
	}

	return l_ops->is_accp_charger_type();
}

bool adapter_is_undetach_cable(int prot)
{
	struct adapter_protocol_ops *l_ops = adapter_get_protocol_ops(prot);

	if (!l_ops || !l_ops->is_undetach_cable)
		return false;

	return l_ops->is_undetach_cable();
}

static int __init adapter_protocol_init(void)
{
	struct adapter_protocol_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_adapter_protocol_dev = l_dev;
	return 0;
}

static void __exit adapter_protocol_exit(void)
{
	if (!g_adapter_protocol_dev)
		return;

	kfree(g_adapter_protocol_dev);
	g_adapter_protocol_dev = NULL;
}

subsys_initcall_sync(adapter_protocol_init);
module_exit(adapter_protocol_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("adapter protocol driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
