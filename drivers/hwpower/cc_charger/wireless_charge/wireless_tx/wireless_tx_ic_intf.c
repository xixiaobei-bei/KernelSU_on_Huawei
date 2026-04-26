/*
 * wireless_tx_ic_intf.c
 *
 * wireless tx ic interfaces
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

#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_ic_intf.h>

#define HWLOG_TAG wireless_tx_ic_intf
HWLOG_REGIST();

static struct wltx_ic_ops *g_wltx_ic_ops[WLTRX_IC_MAX];

int wltx_ic_ops_register(struct wltx_ic_ops *ops, unsigned int type)
{
	if (!ops || !wltrx_ic_is_type_valid(type)) {
		hwlog_err("ops_register: invalid para\n");
		return -EINVAL;
	}
	if (g_wltx_ic_ops[type]) {
		hwlog_err("ops_register: type=%u, already registered\n", type);
		return -EEXIST;
	}

	g_wltx_ic_ops[type] = ops;
	hwlog_info("[ops_register] succ, type=%u\n", type);
	return 0;
}

bool wltx_ic_is_ops_registered(unsigned int type)
{
	if (!wltrx_ic_is_type_valid(type))
		return false;

	if (g_wltx_ic_ops[type])
		return true;

	return false;
}

static struct wltx_ic_ops *wltx_ic_get_ops(unsigned int type)
{
	if (!wltrx_ic_is_type_valid(type)) {
		hwlog_err("get_ops: ic_type=%u err\n", type);
		return NULL;
	}

	return g_wltx_ic_ops[type];
}

struct device_node *wltx_ic_get_dev_node(unsigned int type)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->get_dev_node)
		return ops->get_dev_node(ops->dev_data);

	return NULL;
}

int wltx_ic_fw_update(unsigned int type)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->fw_update)
		return ops->fw_update(ops->dev_data);

	return -EPERM;
}

int wltx_ic_prev_power_supply(unsigned int type, bool flag)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->prev_psy)
		return ops->prev_psy(flag, ops->dev_data);

	return -EPERM;
}

int wltx_ic_chip_init(unsigned int type, unsigned int client)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->chip_init)
		return ops->chip_init(client, ops->dev_data);

	return -EPERM;
}

void wltx_ic_chip_reset(unsigned int type)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->chip_reset)
		ops->chip_reset(ops->dev_data);
}

void wltx_ic_chip_enable(unsigned int type, bool flag)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->chip_enable)
		ops->chip_enable(flag, ops->dev_data);
}

int wltx_ic_mode_enable(unsigned int type, bool flag)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->mode_enable)
		return ops->mode_enable(flag, ops->dev_data);

	return -EPERM;
}

int wltx_ic_activate_chip(unsigned int type)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->activate_chip)
		return ops->activate_chip(ops->dev_data);

	return -EPERM;
}

int wltx_ic_set_open_flag(unsigned int type, bool flag)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->set_open_flag)
		return ops->set_open_flag(flag, ops->dev_data);

	return -EPERM;
}

int wltx_ic_set_stop_config(unsigned int type)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->set_stop_cfg)
		return ops->set_stop_cfg(ops->dev_data);

	return -EPERM;
}

bool wltx_ic_is_rx_disconnect(unsigned int type)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->is_rx_discon)
		return ops->is_rx_discon(ops->dev_data);

	return true;
}

bool wltx_ic_is_in_tx_mode(unsigned int type)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->is_in_tx_mode)
		return ops->is_in_tx_mode(ops->dev_data);

	return false;
}

bool wltx_ic_is_in_rx_mode(unsigned int type)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->is_in_rx_mode)
		return ops->is_in_rx_mode(ops->dev_data);

	return false;
}

int wltx_ic_get_vrect(unsigned int type, u16 *vrect)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->get_vrect)
		return ops->get_vrect(vrect, ops->dev_data);

	return -EPERM;
}

int wltx_ic_get_vin(unsigned int type, u16 *vin)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->get_vin)
		return ops->get_vin(vin, ops->dev_data);

	return -EPERM;
}

int wltx_ic_get_iin(unsigned int type, u16 *iin)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->get_iin)
		return ops->get_iin(iin, ops->dev_data);

	return -EPERM;
}

int wltx_ic_get_temp(unsigned int type, s16 *temp)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->get_temp)
		return ops->get_temp(temp, ops->dev_data);

	return -EPERM;
}

int wltx_ic_get_fop(unsigned int type, u16 *fop)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->get_fop)
		return ops->get_fop(fop, ops->dev_data);

	return -EPERM;
}

int wltx_ic_get_cep(unsigned int type, s8 *cep)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->get_cep)
		return ops->get_cep(cep, ops->dev_data);

	return -EPERM;
}

int wltx_ic_get_duty(unsigned int type, u8 *duty)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->get_duty)
		return ops->get_duty(duty, ops->dev_data);

	return -EPERM;
}

int wltx_ic_get_ptx(unsigned int type, u32 *ptx)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->get_ptx)
		return ops->get_ptx(ptx, ops->dev_data);

	return -EPERM;
}

int wltx_ic_get_prx(unsigned int type, u32 *prx)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->get_prx)
		return ops->get_prx(prx, ops->dev_data);

	return -EPERM;
}

int wltx_ic_get_ploss(unsigned int type, s32 *ploss)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->get_ploss)
		return ops->get_ploss(ploss, ops->dev_data);

	return -EPERM;
}

int wltx_ic_get_ploss_id(unsigned int type, u8 *id)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->get_ploss_id)
		return ops->get_ploss_id(id, ops->dev_data);

	return -EPERM;
}

int wltx_ic_get_ping_freq(unsigned int type, u16 *freq)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->get_ping_freq)
		return ops->get_ping_freq(freq, ops->dev_data);

	return -EPERM;
}

int wltx_ic_get_ping_interval(unsigned int type, u16 *interval)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->get_ping_interval)
		return ops->get_ping_interval(interval, ops->dev_data);

	return -EPERM;
}

int wltx_ic_get_min_fop(unsigned int type, u16 *min_fop)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->get_min_fop)
		return ops->get_min_fop(min_fop, ops->dev_data);

	return -EPERM;
}

int wltx_ic_get_max_fop(unsigned int type, u16 *max_fop)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->get_max_fop)
		return ops->get_max_fop(max_fop, ops->dev_data);

	return -EPERM;
}

int wltx_ic_get_full_bridge_ith(unsigned int type, u16 *ith)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->get_full_bridge_ith)
		return ops->get_full_bridge_ith(ith, ops->dev_data);

	return -EPERM;
}

int wltx_ic_set_ping_freq(unsigned int type, u16 freq)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->set_ping_freq)
		return ops->set_ping_freq(freq, ops->dev_data);

	return -EPERM;
}

int wltx_ic_set_ping_interval(unsigned int type, u16 interval)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->set_ping_interval)
		return ops->set_ping_interval(interval, ops->dev_data);

	return -EPERM;
}

int wltx_ic_set_min_fop(unsigned int type, u16 min_fop)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->set_min_fop)
		return ops->set_min_fop(min_fop, ops->dev_data);

	return -EPERM;
}

int wltx_ic_set_max_fop(unsigned int type, u16 max_fop)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->set_max_fop)
		return ops->set_max_fop(max_fop, ops->dev_data);

	return -EPERM;
}

int wltx_ic_set_ilim(unsigned int type, u16 ilim)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->set_ilim)
		return ops->set_ilim(ilim, ops->dev_data);

	return -EPERM;
}

int wltx_ic_set_fod_coef(unsigned int type, u16 ploss_th, u8 ploss_cnt)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->set_fod_coef)
		return ops->set_fod_coef(ploss_th, ploss_cnt, ops->dev_data);

	return -EPERM;
}

int wltx_ic_set_rp_dm_to(unsigned int type, u16 timeout)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->set_rp_dm_to)
		return ops->set_rp_dm_to(timeout, ops->dev_data);

	return -EPERM;
}

int wltx_ic_set_vset(unsigned int type, int vset)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->set_vset)
		return ops->set_vset(vset, ops->dev_data);

	return -EPERM;
}

int wltx_ic_set_bridge(unsigned int type, unsigned int v_ask, unsigned int bridge)
{
	struct wltx_ic_ops *ops = wltx_ic_get_ops(type);

	if (ops && ops->set_bridge)
		return ops->set_bridge(v_ask, bridge, ops->dev_data);

	return -EPERM;
}
