/*
 * wireless_protocol.c
 *
 * wireless protocol driver
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
#include <chipset_common/hwpower/protocol/wireless_protocol.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>

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
	return -EINVAL;
}

static int wireless_get_protocol_type(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_wireless_protocol_type_table); i++) {
		if (!strncmp(str, g_wireless_protocol_type_table[i], strlen(str)))
			return i;
	}

	hwlog_err("invalid protocol_type_str=%s\n", str);
	return -EINVAL;
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
		return -EINVAL;
	}

	type = wireless_get_protocol_type(ops->type_name);
	if (type < 0) {
		hwlog_err("%s ops register fail\n", ops->type_name);
		return -ENOTSUPP;
	}

	g_wireless_protocol_dev->p_ops[type] = ops;
	g_wireless_protocol_dev->total_ops++;

	hwlog_info("total_ops=%u type=%d:%s ops register ok\n",
		g_wireless_protocol_dev->total_ops, type, ops->type_name);

	return 0;
}

int wireless_send_charge_event(unsigned int ic_type, int prot, u8 evt)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->send_rx_evt) {
		hwlog_err("send_rx_evt is null\n");
		return -ENOTSUPP;
	}

	return l_ops->send_rx_evt(ic_type, evt);
}

int wireless_send_rx_vout(unsigned int ic_type, int prot, int rx_vout)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->send_rx_vout) {
		hwlog_err("send_rx_vout is null\n");
		return -ENOTSUPP;
	}

	return l_ops->send_rx_vout(ic_type, rx_vout);
}

int wireless_send_rx_iout(unsigned int ic_type, int prot, int rx_iout)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->send_rx_iout) {
		hwlog_err("send_rx_iout is null\n");
		return -ENOTSUPP;
	}

	return l_ops->send_rx_iout(ic_type, rx_iout);
}

int wireless_send_rx_boost_succ(unsigned int ic_type, int prot)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->send_rx_boost_succ) {
		hwlog_err("send_rx_boost_succ is null\n");
		return -ENOTSUPP;
	}

	return l_ops->send_rx_boost_succ(ic_type);
}

int wireless_send_rx_ready(unsigned int ic_type, int prot)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->send_rx_ready) {
		hwlog_err("send_rx_ready is null\n");
		return -ENOTSUPP;
	}

	return l_ops->send_rx_ready(ic_type);
}

int wireless_send_tx_capability(unsigned int ic_type, int prot, u8 *cap, int len)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops || !cap)
		return -EINVAL;

	if (!l_ops->send_tx_capability) {
		hwlog_err("send_tx_capability is null\n");
		return -ENOTSUPP;
	}

	return l_ops->send_tx_capability(ic_type, cap, len);
}

int wireless_send_tx_alarm(unsigned int ic_type, int prot, u8 *alarm, int len)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops || !alarm)
		return -EINVAL;

	if (!l_ops->send_tx_alarm) {
		hwlog_err("send_tx_alarm is null\n");
		return -ENOTSUPP;
	}

	return l_ops->send_tx_alarm(ic_type, alarm, len);
}

int wireless_send_tx_fw_version(unsigned int ic_type, int prot, u8 *fw, int len)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops || !fw)
		return -EINVAL;

	if (!l_ops->send_tx_fw_version) {
		hwlog_err("send_tx_fw_version is null\n");
		return -ENOTSUPP;
	}

	return l_ops->send_tx_fw_version(ic_type, fw, len);
}

int wireless_send_cert_confirm(unsigned int ic_type, int prot, bool succ_flag)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->send_cert_confirm) {
		hwlog_err("send_cert_confirm is null\n");
		return -ENOTSUPP;
	}

	return l_ops->send_cert_confirm(ic_type, succ_flag);
}

int wireless_send_charge_state(unsigned int ic_type, int prot, u8 state)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->send_charge_state) {
		hwlog_err("send_charge_state is null\n");
		return -ENOTSUPP;
	}

	return l_ops->send_charge_state(ic_type, state);
}

int wireless_send_fod_status(unsigned int ic_type, int prot, int status)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->send_fod_status) {
		hwlog_err("send_fod_status is null\n");
		return -ENOTSUPP;
	}

	return l_ops->send_fod_status(ic_type, status);
}

int wireless_send_charge_mode(unsigned int ic_type, int prot, u8 mode)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->send_charge_mode) {
		hwlog_err("send_charge_mode is null\n");
		return -ENOTSUPP;
	}

	return l_ops->send_charge_mode(ic_type, mode);
}

int wireless_set_fan_speed_limit(unsigned int ic_type, int prot, u8 limit)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->set_fan_speed_limit) {
		hwlog_err("set_fan_speed_limit is null\n");
		return -ENOTSUPP;
	}

	return l_ops->set_fan_speed_limit(ic_type, limit);
}

int wireless_set_rpp_format(unsigned int ic_type, int prot, u8 power)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->set_rpp_format) {
		hwlog_err("set_rpp_format is null\n");
		return -ENOTSUPP;
	}

	return l_ops->set_rpp_format(ic_type, power);
}

int wireless_get_ept_type(unsigned int ic_type, int prot, u16 *type)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops || !type)
		return -EINVAL;

	if (!l_ops->get_ept_type) {
		hwlog_err("get_ept_type is null\n");
		return -ENOTSUPP;
	}

	return l_ops->get_ept_type(ic_type, type);
}

int wireless_get_rpp_format(unsigned int ic_type, int prot, u8 *format)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops || !format)
		return -EINVAL;

	if (!l_ops->get_rpp_format) {
		hwlog_err("get_rpp_format is null\n");
		return -ENOTSUPP;
	}

	return l_ops->get_rpp_format(ic_type, format);
}

char *wireless_get_tx_fw_version(unsigned int ic_type, int prot)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);
	struct wireless_protocol_dev *l_dev = wireless_get_protocol_dev();

	if (!l_ops || !l_dev)
		return NULL;

	if (!l_ops->get_tx_fw_version) {
		hwlog_err("get_tx_fw_version is null\n");
		return NULL;
	}

	memset(l_dev->info.tx_fwver, 0, WIRELESS_TX_FWVER_LEN);
	if (l_ops->get_tx_fw_version(ic_type, l_dev->info.tx_fwver,
		WIRELESS_TX_FWVER_LEN))
		return NULL;

	return l_dev->info.tx_fwver;
}

int wireless_get_tx_bigdata_info(unsigned int ic_type, int prot)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);
	struct wireless_protocol_dev *l_dev = wireless_get_protocol_dev();

	if (!l_ops || !l_dev)
		return -EINVAL;

	if (!l_ops->get_tx_bigdata_info) {
		hwlog_err("get_tx_bigdata_info is null\n");
		return -ENOTSUPP;
	}

	return l_ops->get_tx_bigdata_info(ic_type, l_dev->info.tx_bd_info,
		sizeof(l_dev->info.tx_bd_info));
}

int wireless_get_tx_id(unsigned int ic_type, int prot)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);
	struct wireless_protocol_dev *l_dev = wireless_get_protocol_dev();

	if (!l_ops || !l_dev)
		return -EINVAL;

	if (!l_ops->get_tx_id) {
		hwlog_err("get_tx_id is null\n");
		return -ENOTSUPP;
	}

	if (l_ops->get_tx_id(ic_type, &l_dev->info.tx_id))
		return -ENOTSUPP;

	return l_dev->info.tx_id;
}

int wireless_get_tx_type(unsigned int ic_type, int prot)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);
	struct wireless_protocol_dev *l_dev = wireless_get_protocol_dev();

	if (!l_ops || !l_dev)
		return WIRELESS_TX_TYPE_UNKNOWN;

	if (!l_ops->get_tx_type) {
		hwlog_err("get_tx_type is null\n");
		return WIRELESS_TX_TYPE_UNKNOWN;
	}

	if (l_ops->get_tx_type(ic_type, &l_dev->info.tx_type))
		return WIRELESS_TX_TYPE_UNKNOWN;

	return l_dev->info.tx_type;
}

int wireless_get_tx_adapter_type(unsigned int ic_type, int prot)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);
	struct wireless_protocol_dev *l_dev = wireless_get_protocol_dev();

	if (!l_ops || !l_dev)
		return -EINVAL;

	if (!l_ops->get_tx_adapter_type) {
		hwlog_err("get_tx_adapter_type is null\n");
		return -ENOTSUPP;
	}

	if (l_ops->get_tx_adapter_type(ic_type, &l_dev->info.tx_adp_type))
		return -ENOTSUPP;

	return l_dev->info.tx_adp_type;
}

int wireless_get_tx_capability(unsigned int ic_type, int prot,
	struct wireless_protocol_tx_cap *cap)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops || !cap)
		return -EINVAL;

	if (!l_ops->get_tx_capability) {
		hwlog_err("get_tx_capability is null\n");
		return -ENOTSUPP;
	}

	return l_ops->get_tx_capability(ic_type, cap);
}

int wireless_get_tx_max_power(unsigned int ic_type, int prot, int *power)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops || !power)
		return -EINVAL;

	if (!l_ops->get_tx_max_power) {
		hwlog_err("get_tx_max_power is null\n");
		return -ENOTSUPP;
	}

	return l_ops->get_tx_max_power(ic_type, power);
}

int wireless_get_tx_adapter_capability(unsigned int ic_type, int prot, int *vout, int *iout)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops || !vout || !iout)
		return -EINVAL;

	if (!l_ops->get_tx_adapter_capability) {
		hwlog_err("get_tx_adapter_capability is null\n");
		return -ENOTSUPP;
	}

	return l_ops->get_tx_adapter_capability(ic_type, vout, iout);
}

int wireless_get_tx_cable_type(unsigned int ic_type, int prot, int *type, int *iout)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops || !type || !iout)
		return -EINVAL;

	if (!l_ops->get_tx_cable_type) {
		hwlog_err("get_tx_cable_type is null\n");
		return -ENOTSUPP;
	}

	return l_ops->get_tx_cable_type(ic_type, type, iout);
}

int wireless_get_tx_fop_range(unsigned int ic_type, int prot,
	struct wireless_protocol_tx_fop_range *fop)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops || !fop)
		return -EINVAL;

	if (!l_ops->get_tx_fop_range) {
		hwlog_err("get_tx_fop_range is null\n");
		return -ENOTSUPP;
	}

	return l_ops->get_tx_fop_range(ic_type, fop);
}

int wireless_auth_encrypt_start(unsigned int ic_type, int prot, int key, u8 *random,
	int r_size, u8 *hash, int h_size)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops || !random || !hash)
		return -EINVAL;

	if (!l_ops->auth_encrypt_start) {
		hwlog_err("auth_encrypt_start is null\n");
		return -ENOTSUPP;
	}

	return l_ops->auth_encrypt_start(ic_type, key, random, r_size, hash, h_size);
}

int wireless_fix_tx_fop(unsigned int ic_type, int prot, int fop)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->fix_tx_fop) {
		hwlog_err("fix_tx_fop is null\n");
		return -ENOTSUPP;
	}

	return l_ops->fix_tx_fop(ic_type, fop);
}

int wireless_unfix_tx_fop(unsigned int ic_type, int prot)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->unfix_tx_fop) {
		hwlog_err("unfix_tx_fop is null\n");
		return -ENOTSUPP;
	}

	return l_ops->unfix_tx_fop(ic_type);
}

int wireless_reset_dev_info(unsigned int ic_type, int prot)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);
	struct wireless_protocol_dev *l_dev = wireless_get_protocol_dev();

	if (!l_ops || !l_dev)
		return -EINVAL;

	memset(&l_dev->info, 0, sizeof(l_dev->info));

	if (!l_ops->reset_dev_info) {
		hwlog_err("reset_dev_info is null\n");
		return -ENOTSUPP;
	}

	return l_ops->reset_dev_info(ic_type);
}

int wireless_acc_set_tx_dev_state(unsigned int ic_type, int prot, u8 state)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->acc_set_tx_dev_state) {
		hwlog_err("acc_set_tx_dev_state is null\n");
		return -ENOTSUPP;
	}

	return l_ops->acc_set_tx_dev_state(ic_type, state);
}

int wireless_acc_set_tx_dev_info_cnt(unsigned int ic_type, int prot, u8 cnt)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->acc_set_tx_dev_info_cnt) {
		hwlog_err("acc_set_tx_dev_info_cnt is null\n");
		return -ENOTSUPP;
	}

	return l_ops->acc_set_tx_dev_info_cnt(ic_type, cnt);
}

int wireless_acc_get_tx_dev_state(unsigned int ic_type, int prot, u8 *state)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops || !state)
		return -EINVAL;

	if (!l_ops->acc_get_tx_dev_state) {
		hwlog_err("acc_get_tx_dev_state is null\n");
		return -ENOTSUPP;
	}

	return l_ops->acc_get_tx_dev_state(ic_type, state);
}

int wireless_acc_get_tx_dev_no(unsigned int ic_type, int prot, u8 *no)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops || !no)
		return -EINVAL;

	if (!l_ops->acc_get_tx_dev_no) {
		hwlog_err("acc_get_tx_dev_no is null\n");
		return -ENOTSUPP;
	}

	return l_ops->acc_get_tx_dev_no(ic_type, no);
}

int wireless_acc_get_tx_dev_mac(unsigned int ic_type, int prot, u8 *mac, int len)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops || !mac)
		return -EINVAL;

	if (!l_ops->acc_get_tx_dev_mac) {
		hwlog_err("acc_get_tx_dev_mac is null\n");
		return -ENOTSUPP;
	}

	return l_ops->acc_get_tx_dev_mac(ic_type, mac, len);
}

int wireless_acc_get_tx_dev_model_id(unsigned int ic_type, int prot, u8 *id, int len)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops || !id)
		return -EINVAL;

	if (!l_ops->acc_get_tx_dev_model_id) {
		hwlog_err("acc_get_tx_dev_model_id is null\n");
		return -ENOTSUPP;
	}

	return l_ops->acc_get_tx_dev_model_id(ic_type, id, len);
}

int wireless_acc_get_tx_dev_submodel_id(unsigned int ic_type, int prot, u8 *id)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops || !id)
		return -EINVAL;

	if (!l_ops->acc_get_tx_dev_submodel_id) {
		hwlog_err("acc_get_tx_dev_submodel_id is null\n");
		return -ENOTSUPP;
	}

	return l_ops->acc_get_tx_dev_submodel_id(ic_type, id);
}

int wireless_acc_get_tx_dev_version(unsigned int ic_type, int prot, u8 *ver)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops || !ver)
		return -EINVAL;

	if (!l_ops->acc_get_tx_dev_version) {
		hwlog_err("acc_get_tx_dev_version is null\n");
		return -ENOTSUPP;
	}

	return l_ops->acc_get_tx_dev_version(ic_type, ver);
}

int wireless_acc_get_tx_dev_business(unsigned int ic_type, int prot, u8 *bus)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops || !bus)
		return -EINVAL;

	if (!l_ops->acc_get_tx_dev_business) {
		hwlog_err("acc_get_tx_dev_business is null\n");
		return -ENOTSUPP;
	}

	return l_ops->acc_get_tx_dev_business(ic_type, bus);
}

int wireless_acc_get_tx_dev_info_cnt(unsigned int ic_type, int prot, u8 *cnt)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops || !cnt)
		return -EINVAL;

	if (!l_ops->acc_get_tx_dev_info_cnt) {
		hwlog_err("acc_get_tx_dev_info_cnt is null\n");
		return -ENOTSUPP;
	}

	return l_ops->acc_get_tx_dev_info_cnt(ic_type, cnt);
}

int wireless_set_tx_cipherkey(unsigned int ic_type, int prot, u8 *cipherkey, int len)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops || !cipherkey)
		return -EINVAL;

	if (!l_ops->set_tx_cipherkey) {
		hwlog_err("set_tx_cipherkey is null\n");
		return -ENOTSUPP;
	}

	return l_ops->set_tx_cipherkey(ic_type, cipherkey, len);
}

int wireless_get_rx_random(unsigned int ic_type, int prot, u8 *random, int len)
{
	struct wireless_protocol_ops *l_ops = wireless_get_protocol_ops(prot);

	if (!l_ops || !random)
		return -EINVAL;

	if (!l_ops->get_rx_random) {
		hwlog_err("get_rx_random is null\n");
		return -ENOTSUPP;
	}

	return l_ops->get_rx_random(ic_type, random, len);
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
