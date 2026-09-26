/*
 * wireless_tx_open_by_client.c
 *
 * enable tx by client interfaces for wireless reverse charging
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

#include <linux/notifier.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/power/hisi/bci_battery.h>
#include <huawei_platform/power/wireless/wireless_transmitter.h>
#include <chipset_common/hwpower/accessory/wireless_lightstrap.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>

#define HWLOG_TAG wireless_tx_client_open
HWLOG_REGIST();

static bool can_soc_do_reverse_charging(void)
{
	struct wltx_dev_info *di = wltx_get_dev_info();
	int soc = bci_show_capacity();

	if ((wlrx_get_wired_channel_state() == WIRED_CHANNEL_OFF) &&
		(soc <= WL_TX_SOC_MIN) && (di->is_keyboard_online == false)) {
		hwlog_info("[%s] capacity is out of range\n", __func__);
		wireless_tx_set_tx_status(WL_TX_STATUS_SOC_ERROR);
		return false;
	}

	return true;
}

static bool client_can_do_reverse_charging(void)
{
	if (!can_soc_do_reverse_charging())
		return false;

	return true;
}

static void client_reinit_tx_chip(void)
{
	if (lightstrap_online_state())
		lightstrap_reinit_tx_chip();
}

static bool client_need_specify_pwr_src(void)
{
	if (lightstrap_online_state())
		return true;

	return false;
}

static enum wltx_pwr_src client_specify_pwr_src(void)
{
	if (lightstrap_online_state())
		return lightstrap_specify_pwr_src();

	return PWR_SRC_NULL;
}

static struct wltx_logic_ops g_client_logic_ops = {
	.type                 = WLTX_OPEN_BY_CLIENT,
	.can_rev_charge_check = client_can_do_reverse_charging,
	.need_specify_pwr_src = client_need_specify_pwr_src,
	.specify_pwr_src      = client_specify_pwr_src,
	.reinit_tx_chip       = client_reinit_tx_chip,
};

static int __init wltx_open_by_client_init(void)
{
	return wireless_tx_logic_ops_register(&g_client_logic_ops);
}

module_init(wltx_open_by_client_init);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wireless_tx_open_by_client module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
