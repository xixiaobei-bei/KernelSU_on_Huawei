/*
 * wireless_rx_ic_intf.c
 *
 * wireless rx ic interfaces
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

#include <chipset_common/hwpower/wireless_charge/wireless_rx_ic_intf.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG wireless_rx_ic_intf
HWLOG_REGIST();

static struct wlrx_ic_ops *g_wlrx_ic_ops[WLTRX_IC_MAX];

int wlrx_ic_ops_register(struct wlrx_ic_ops *ops, unsigned int type)
{
	if (!ops || !wltrx_ic_is_type_valid(type)) {
		hwlog_err("ops_register: invalid para\n");
		return -EINVAL;
	}
	if (g_wlrx_ic_ops[type]) {
		hwlog_err("ops_register: type=%u, already registered\n", type);
		return -EEXIST;
	}

	g_wlrx_ic_ops[type] = ops;
	hwlog_info("[ops_register] succ, type=%u\n", type);
	return 0;
}

bool wlrx_ic_is_ops_registered(unsigned int type)
{
	if (!wltrx_ic_is_type_valid(type))
		return false;

	if (g_wlrx_ic_ops[type])
		return true;

	return false;
}

static struct wlrx_ic_ops *wlrx_ic_get_ops(unsigned int type)
{
	if (!wltrx_ic_is_type_valid(type)) {
		hwlog_err("get_ops: ic_type=%u err\n", type);
		return NULL;
	}

	return g_wlrx_ic_ops[type];
}

struct device_node *wlrx_ic_get_dev_node(unsigned int type)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->get_dev_node)
		return ops->get_dev_node(ops->dev_data);

	return NULL;
}

int wlrx_ic_fw_update(unsigned int type)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->fw_update)
		return ops->fw_update(ops->dev_data);

	return -EPERM;
}

int wlrx_ic_chip_init(unsigned int ic_type,
	unsigned int init_type, unsigned int tx_type)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(ic_type);

	if (ops && ops->chip_init)
		return ops->chip_init(init_type, tx_type, ops->dev_data);

	return -EPERM;
}

void wlrx_ic_chip_reset(unsigned int type)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->chip_reset)
		ops->chip_reset(ops->dev_data);
}

void wlrx_ic_chip_enable(unsigned int type, bool flag)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->chip_enable)
		ops->chip_enable(flag, ops->dev_data);
}

bool wlrx_ic_is_chip_enable(unsigned int type)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->is_chip_enable)
		return ops->is_chip_enable(ops->dev_data);

	return false;
}

void wlrx_ic_sleep_enable(unsigned int type, bool flag)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->sleep_enable)
		ops->sleep_enable(flag, ops->dev_data);
}

bool wlrx_ic_is_sleep_enable(unsigned int type)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->is_sleep_enable)
		return ops->is_sleep_enable(ops->dev_data);

	return false;
}

void wlrx_ic_ext_pwr_prev_ctrl(unsigned int type, bool flag)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->ext_pwr_prev_ctrl)
		ops->ext_pwr_prev_ctrl(flag, ops->dev_data);
}

void wlrx_ic_ext_pwr_post_ctrl(unsigned int type, bool flag)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->ext_pwr_post_ctrl)
		ops->ext_pwr_post_ctrl(flag, ops->dev_data);
}

void wlrx_ic_pmic_vbus_handler(unsigned int type, bool flag)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->pmic_vbus_handler)
		ops->pmic_vbus_handler(flag, ops->dev_data);
}

bool wlrx_ic_is_tx_exist(unsigned int type)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->is_tx_exist)
		return ops->is_tx_exist(ops->dev_data);

	return false;
}

int wlrx_ic_kick_watchdog(unsigned int type)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->kick_watchdog)
		return ops->kick_watchdog(ops->dev_data);

	return -EPERM;
}

void wlrx_ic_stop_charging(unsigned int type)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->stop_charging)
		ops->stop_charging(ops->dev_data);
}

void wlrx_ic_send_ept(unsigned int type, int ept_type)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->send_ept)
		ops->send_ept(ept_type, ops->dev_data);
}

int wlrx_ic_get_chip_info(unsigned int type, char *buf, int len)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->get_chip_info)
		return ops->get_chip_info(buf, len, ops->dev_data);

	return -EPERM;
}

int wlrx_ic_get_vrect(unsigned int type, int *vrect)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->get_vrect)
		return ops->get_vrect(vrect, ops->dev_data);

	return -EPERM;
}

int wlrx_ic_get_vout(unsigned int type, int *vout)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->get_vout)
		return ops->get_vout(vout, ops->dev_data);

	return -EPERM;
}

int wlrx_ic_get_iout(unsigned int type, int *iout)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->get_iout)
		return ops->get_iout(iout, ops->dev_data);

	return -EPERM;
}

int wlrx_ic_get_imax(unsigned int type, int *rx_imax)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->get_imax)
		return ops->get_imax(rx_imax, ops->dev_data);

	if (rx_imax)
		*rx_imax = 5000; /* default 5000mA when chip is ok */
	return -EPERM;
}

int wlrx_ic_get_vout_reg(unsigned int type, int *vreg)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->get_vout_reg)
		return ops->get_vout_reg(vreg, ops->dev_data);

	return -EPERM;
}

int wlrx_ic_get_vfc_reg(unsigned int type, int *vfc_reg)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->get_vfc_reg)
		return ops->get_vfc_reg(vfc_reg, ops->dev_data);

	return -EPERM;
}

int wlrx_ic_get_fop(unsigned int type, int *fop)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->get_fop)
		return ops->get_fop(fop, ops->dev_data);

	return -EPERM;
}

int wlrx_ic_get_temp(unsigned int type, int *temp)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->get_temp)
		return ops->get_temp(temp, ops->dev_data);

	return -EPERM;
}

int wlrx_ic_get_cep(unsigned int type, int *cep)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->get_cep)
		return ops->get_cep(cep, ops->dev_data);

	return -EPERM;
}

void wlrx_ic_get_bst_delay_time(unsigned int type, int *time)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->get_bst_delay_time)
		ops->get_bst_delay_time(time, ops->dev_data);
}

int wlrx_ic_get_fod_coef(unsigned int type, char *fod_str, int len)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->get_fod_coef)
		return ops->get_fod_coef(fod_str, len, ops->dev_data);

	return -EPERM;
}

int wlrx_ic_set_vfc(unsigned int type, int vfc)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->set_vfc)
		return ops->set_vfc(vfc, ops->dev_data);

	return -EPERM;
}

int wlrx_ic_set_vout(unsigned int type, int vout)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->set_vout)
		return ops->set_vout(vout, ops->dev_data);

	return -EPERM;
}

int wlrx_ic_set_fod_coef(unsigned int type, const char *fod_str)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->set_fod_coef)
		return ops->set_fod_coef(fod_str, ops->dev_data);

	return -EPERM;
}

void wlrx_ic_set_pu_shell_flag(unsigned int type, bool flag)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->set_pu_shell_flag)
		ops->set_pu_shell_flag(flag, ops->dev_data);
}

bool wlrx_ic_need_chk_pu_shell(unsigned int type)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->need_chk_pu_shell)
		return ops->need_chk_pu_shell(ops->dev_data);

	return false;
}

char *wlrx_ic_read_nvm_info(unsigned int type, int sec)
{
	struct wlrx_ic_ops *ops = wlrx_ic_get_ops(type);

	if (ops && ops->read_nvm_info)
		return ops->read_nvm_info(sec, ops->dev_data);

	return NULL;
}
