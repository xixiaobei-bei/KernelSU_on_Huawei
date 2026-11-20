/*
 * wireless_rx_pctrl.c
 *
 * power(vtx,vrx,irx) control of wireless charging
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

#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_dts.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_pctrl.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_plim.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_interfere.h>

#define HWLOG_TAG wireless_rx_pctrl
HWLOG_REGIST();

static void wlrx_update_soc_pctrl(struct wlrx_pctrl *pctrl)
{
	int i, soc;
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!dts || !dts->soc_cfg)
		return;

	soc = power_platform_get_battery_capacity();
	for (i = 0; i < dts->soc_cfg_level; i++) {
		if ((soc < dts->soc_cfg[i].soc_min) ||
			(soc > dts->soc_cfg[i].soc_max))
			continue;
		pctrl->vtx = min(dts->soc_cfg[i].vtx, pctrl->vtx);
		pctrl->vrx = min(dts->soc_cfg[i].vrx, pctrl->vrx);
		pctrl->irx = min(dts->soc_cfg[i].irx, pctrl->irx);
		break;
	}
}

void wlrx_update_pctrl(struct wlrx_pctrl *pctrl)
{
	if (!pctrl)
		return;

	wlrx_update_soc_pctrl(pctrl);
	wlrx_update_plim_pctrl(pctrl);
	wlrx_update_intfr_pctrl(pctrl);
}
