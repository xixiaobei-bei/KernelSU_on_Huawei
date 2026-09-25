/*
 * wireless_protocol.c
 *
 * wireless protocol driver
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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
#include <huawei_platform/power/power_sysfs.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/wireless/wireless_protocol.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG wireless_prot
HWLOG_REGIST();

static struct wireless_protocol_dev *g_wireless_protocol_dev;

static const char * const g_wireless_protocol_type_table[] = {
	[WIRELESS_PROTOCOL_QI] = "hw_qi",
	[WIRELESS_PROTOCOL_A4WP] = "hw_a4wp",
};

static int wireless_check_protocol_type(int prot)
{
	if ((prot >= WIRELESS_PROTOCOL_BEGIN) && (prot < WIRELESS_PROTOCOL_END))
		return 0;

	hwlog_err("invalid protocol_type=%d\n", prot);
	return -1;
}

static int wireless_get_protocol_type(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_wireless_protocol_type_table); i++) {
		if (!strncmp(str, g_wireless_protocol_type_table[i], strlen(str)))
			return i;
	}

	hwlog_err("invalid protocol_type_str=%s\n", str);
	return -1;
}

struct wireless_protocol_dev *wireless_get_protocol_dev(void)
{
	if (!g_wireless_protocol_dev) {
		hwlog_err("g_wireless_protocol_dev is null\n");
		return NULL;
	}

	return g_wireless_protocol_dev;
}

static struct wireless_protocol_ops *wireless_get_protocol_ops(int prot)
{
	if (wireless_check_protocol_type(prot))
		return NULL;

	if (!g_wireless_protocol_dev || !g_wireless_protocol_dev->p_ops[prot]) {
		hwlog_err("g_wireless_protocol_dev or p_ops is null\n");
		return NULL;
	}

	return g_wireless_protocol_dev->p_ops[prot];
}

int wireless_protocol_ops_register(struct wireless_protocol_ops *ops)
{
	int type;

	if (!g_wireless_protocol_dev || !ops || !ops->type_name) {
		hwlog_err("g_wireless_protocol_dev or ops or type_name is null\n");
		return -1;
	}

	type = wireless_get_protocol_type(ops->type_name);
	if (type < 0) {
		hwlog_err("%s ops register fail\n", ops->type_name);
		return -1;
	}

	g_wireless_protocol_dev->p_ops[type] = ops;
	g_wireless_protocol_dev->total_ops++;

	hwlog_info("total_ops=%d type=%d:%s ops register ok\n",
		g_wireless_protocol_dev->total_ops, type, ops->type_name);

	return 0;
}

int wireless_send_rx_vout(int prot, int rx_vout)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->send_rx_vout) {
		hwlog_err("send_rx_vout is null\n");
		return -1;
	}

	return l_ops->send_rx_vout(rx_vout);
}

int wireless_send_rx_iout(int prot, int rx_iout)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->send_rx_iout) {
		hwlog_err("send_rx_iout is null\n");
		return -1;
	}

	return l_ops->send_rx_iout(rx_iout);
}

int wireless_send_rx_boost_succ(int prot)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->send_rx_boost_succ) {
		hwlog_err("send_rx_boost_succ is null\n");
		return -1;
	}

	return l_ops->send_rx_boost_succ();
}

int wireless_send_rx_ready(int prot)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->send_rx_ready) {
		hwlog_err("send_rx_ready is null\n");
		return -1;
	}

	return l_ops->send_rx_ready();
}

int wireless_send_tx_capability(int prot, u8 *cap, int len)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops || !cap)
		return -1;

	if (!l_ops->send_tx_capability) {
		hwlog_err("send_tx_capability is null\n");
		return -1;
	}

	return l_ops->send_tx_capability(cap, len);
}

int wireless_send_tx_fw_version(int prot, u8 *fw, int len)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->send_tx_fw_version) {
		hwlog_err("send_tx_fw_version is null\n");
		return -1;
	}

	return l_ops->send_tx_fw_version(fw, len);
}

int wireless_send_cert_confirm(int prot, bool succ_flag)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->send_cert_confirm) {
		hwlog_err("send_cert_confirm is null\n");
		return -1;
	}

	return l_ops->send_cert_confirm(succ_flag);
}

int wireless_send_charge_state(int prot, u8 state)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->send_charge_state) {
		hwlog_err("send_charge_state is null\n");
		return -1;
	}

	return l_ops->send_charge_state(state);
}

int wireless_send_fod_status(int prot, int status)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->send_fod_status) {
		hwlog_err("send_fod_status is null\n");
		return -1;
	}

	return l_ops->send_fod_status(status);
}

int wireless_send_charge_mode(int prot, u8 mode)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->send_charge_mode) {
		hwlog_err("send_charge_mode is null\n");
		return -1;
	}

	return l_ops->send_charge_mode(mode);
}

int wireless_set_fan_speed_limit(int prot, u8 limit)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->set_fan_speed_limit) {
		hwlog_err("set_fan_speed_limit is null\n");
		return -1;
	}

	return l_ops->set_fan_speed_limit(limit);
}

int wireless_set_rpp_format(int prot, u8 power)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->set_rpp_format) {
		hwlog_err("set_rpp_format is null\n");
		return -1;
	}

	return l_ops->set_rpp_format(power);
}

int wireless_get_ept_type(int prot, u16 *type)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops || !type)
		return -1;

	if (!l_ops->get_ept_type) {
		hwlog_err("get_ept_type is null\n");
		return -1;
	}

	return l_ops->get_ept_type(type);
}

int wireless_get_rpp_format(int prot, u8 *format)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops || !format)
		return -1;

	if (!l_ops->get_rpp_format) {
		hwlog_err("get_rpp_format is null\n");
		return -1;
	}

	return l_ops->get_rpp_format(format);
}

char *wireless_get_tx_fw_version(int prot)
{
	struct wireless_protocol_ops *l_ops = NULL;
	struct wireless_protocol_dev *l_dev = NULL;

	l_dev = wireless_get_protocol_dev();
	if (!l_dev)
		return NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops)
		return NULL;

	if (!l_ops->get_tx_fw_version) {
		hwlog_err("get_tx_fw_version is null\n");
		return NULL;
	}

	memset(l_dev->info.tx_fwver, 0, WIRELESS_TX_FWVER_LEN);
	if (l_ops->get_tx_fw_version(l_dev->info.tx_fwver,
		WIRELESS_TX_FWVER_LEN))
		return NULL;

	return l_dev->info.tx_fwver;
}

void wireless_get_tx_bigdata_info(int prot)
{
	struct wireless_protocol_ops *l_ops = NULL;
	struct wireless_protocol_dev *l_dev = NULL;

	l_dev = wireless_get_protocol_dev();
	if (!l_dev)
		return;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops)
		return;

	if (!l_ops->get_tx_bd_info) {
		hwlog_err("get_tx_bd_info is null\n");
		return;
	}

	l_ops->get_tx_bd_info(l_dev->info.tx_bd_info,
		sizeof(l_dev->info.tx_bd_info));
}

int wireless_get_tx_id(int prot)
{
	struct wireless_protocol_ops *l_ops = NULL;
	struct wireless_protocol_dev *l_dev = NULL;

	l_dev = wireless_get_protocol_dev();
	if (!l_dev)
		return -1;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->get_tx_id) {
		hwlog_err("get_tx_id is null\n");
		return -1;
	}

	if (l_ops->get_tx_id(&l_dev->info.tx_id))
		return -1;

	return l_dev->info.tx_id;
}

int wireless_get_tx_type(int prot)
{
	struct wireless_protocol_ops *l_ops = NULL;
	struct wireless_protocol_dev *l_dev = NULL;

	l_dev = wireless_get_protocol_dev();
	if (!l_dev)
		return WIRELESS_TX_TYPE_UNKNOWN;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops)
		return WIRELESS_TX_TYPE_UNKNOWN;

	if (!l_ops->get_tx_type) {
		hwlog_err("get_tx_type is null\n");
		return WIRELESS_TX_TYPE_UNKNOWN;
	}

	if (l_ops->get_tx_type(&l_dev->info.tx_type))
		return WIRELESS_TX_TYPE_UNKNOWN;

	return l_dev->info.tx_type;
}

int wireless_get_tx_adapter_type(int prot)
{
	struct wireless_protocol_ops *l_ops = NULL;
	struct wireless_protocol_dev *l_dev = NULL;

	l_dev = wireless_get_protocol_dev();
	if (!l_dev)
		return -1;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->get_tx_adapter_type) {
		hwlog_err("get_tx_adapter_type is null\n");
		return -1;
	}

	if (l_ops->get_tx_adapter_type(&l_dev->info.tx_adp_type))
		return -1;

	return l_dev->info.tx_adp_type;
}

int wireless_get_tx_capability(int prot, struct wireless_protocol_tx_cap *cap)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops || !cap)
		return -1;

	if (!l_ops->get_tx_capability) {
		hwlog_err("get_tx_capability is null\n");
		return -1;
	}

	return l_ops->get_tx_capability(cap);
}

int wireless_auth_encrypt_start(int prot,
	int key, u8 *random, int r_size, u8 *hash, int h_size)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops || !random || !hash)
		return -1;

	if (!l_ops->auth_encrypt_start) {
		hwlog_err("auth_encrypt_start is null\n");
		return -1;
	}

	return l_ops->auth_encrypt_start(key, random, r_size, hash, h_size);
}

int wireless_fix_tx_fop(int prot, int fop)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->fix_tx_fop) {
		hwlog_err("fix_tx_fop is null\n");
		return -1;
	}

	return l_ops->fix_tx_fop(fop);
}

int wireless_unfix_tx_fop(int prot)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->unfix_tx_fop) {
		hwlog_err("unfix_tx_fop is null\n");
		return -1;
	}

	return l_ops->unfix_tx_fop();
}

void wireless_reset_dev_info(int prot)
{
	struct wireless_protocol_ops *l_ops = NULL;
	struct wireless_protocol_dev *l_dev = NULL;

	l_dev = wireless_get_protocol_dev();
	if (!l_dev)
		return;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops)
		return;

	memset(&l_dev->info, 0, sizeof(l_dev->info));

	if (!l_ops->reset_dev_info) {
		hwlog_err("reset_dev_info is null\n");
		return;
	}
	l_ops->reset_dev_info();
}

int wireless_acc_set_tx_dev_state(int prot, u8 state)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->acc_set_tx_dev_state) {
		hwlog_err("acc_set_tx_dev_state is null\n");
		return -1;
	}

	return l_ops->acc_set_tx_dev_state(state);
}

int wireless_acc_set_tx_dev_info_cnt(int prot, u8 cnt)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops)
		return -1;

	if (!l_ops->acc_set_tx_dev_info_cnt) {
		hwlog_err("acc_set_tx_dev_info_cnt is null\n");
		return -1;
	}

	return l_ops->acc_set_tx_dev_info_cnt(cnt);
}

int wireless_acc_get_tx_dev_state(int prot, u8 *state)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops || !state)
		return -1;

	if (!l_ops->acc_get_tx_dev_state) {
		hwlog_err("acc_get_tx_dev_state is null\n");
		return -1;
	}

	return l_ops->acc_get_tx_dev_state(state);
}

int wireless_acc_get_tx_dev_no(int prot, u8 *no)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops || !no)
		return -1;

	if (!l_ops->acc_get_tx_dev_no) {
		hwlog_err("acc_get_tx_dev_no is null\n");
		return -1;
	}

	return l_ops->acc_get_tx_dev_no(no);
}

int wireless_acc_get_tx_dev_mac(int prot, u8 *mac, int len)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops || !mac)
		return -1;

	if (!l_ops->acc_get_tx_dev_mac) {
		hwlog_err("acc_get_tx_dev_mac is null\n");
		return -1;
	}

	return l_ops->acc_get_tx_dev_mac(mac, len);
}

int wireless_acc_get_tx_dev_model_id(int prot, u8 *id, int len)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops || !id)
		return -1;

	if (!l_ops->acc_get_tx_dev_model_id) {
		hwlog_err("acc_get_tx_dev_model_id is null\n");
		return -1;
	}

	return l_ops->acc_get_tx_dev_model_id(id, len);
}

int wireless_acc_get_tx_dev_submodel_id(int prot, u8 *id)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops || !id)
		return -1;

	if (!l_ops->acc_get_tx_dev_submodel_id) {
		hwlog_err("acc_get_tx_dev_submodel_id is null\n");
		return -1;
	}

	return l_ops->acc_get_tx_dev_submodel_id(id);
}

int wireless_acc_get_tx_dev_version(int prot, u8 *ver)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops || !ver)
		return -1;

	if (!l_ops->acc_get_tx_dev_version) {
		hwlog_err("acc_get_tx_dev_version is null\n");
		return -1;
	}

	return l_ops->acc_get_tx_dev_version(ver);
}

int wireless_acc_get_tx_dev_business(int prot, u8 *bus)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops || !bus)
		return -1;

	if (!l_ops->acc_get_tx_dev_business) {
		hwlog_err("acc_get_tx_dev_business is null\n");
		return -1;
	}

	return l_ops->acc_get_tx_dev_business(bus);
}

int wireless_acc_get_tx_dev_info_cnt(int prot, u8 *cnt)
{
	struct wireless_protocol_ops *l_ops = NULL;

	l_ops = wireless_get_protocol_ops(prot);
	if (!l_ops || !cnt)
		return -1;

	if (!l_ops->acc_get_tx_dev_info_cnt) {
		hwlog_err("acc_get_tx_dev_info_cnt is null\n");
		return -1;
	}

	return l_ops->acc_get_tx_dev_info_cnt(cnt);
}

static int __init wireless_protocol_init(void)
{
	struct wireless_protocol_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_wireless_protocol_dev = l_dev;

	return 0;
}

static void __exit wireless_protocol_exit(void)
{
	if (!g_wireless_protocol_dev)
		return;

	kfree(g_wireless_protocol_dev);
	g_wireless_protocol_dev = NULL;
}

subsys_initcall_sync(wireless_protocol_init);
module_exit(wireless_protocol_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wireless protocol driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
