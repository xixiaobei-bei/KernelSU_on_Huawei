/*
 * wireless_rx_interfere.c
 *
 * interference handler of wireless charging
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

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_dts.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_pctrl.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_interfere.h>

#define HWLOG_TAG wireless_rx_intfr
HWLOG_REGIST();

#define WLRX_INTFR_TIMEOUT          3000 /* ms */

struct wlrx_intfr_info {
	u8 src;
	struct delayed_work work;
	struct wlrx_intfr intfr;
};

static struct wlrx_intfr_info *g_wlrx_intfr_di;

u8 wlrx_get_intfr_src(void)
{
	if (!g_wlrx_intfr_di)
		return 0;

	return g_wlrx_intfr_di->src;
}

int wlrx_get_intfr_fixed_fop(void)
{
	if (!g_wlrx_intfr_di)
		return WLRX_INTFR_DFT_FIXED_FOP;

	return g_wlrx_intfr_di->intfr.fixed_fop;
}

int wlrx_get_intfr_vtx(void)
{
	if (!g_wlrx_intfr_di)
		return 0;

	return g_wlrx_intfr_di->intfr.vtx;
}

int wlrx_get_intfr_vrx(void)
{
	if (!g_wlrx_intfr_di)
		return 0;

	return g_wlrx_intfr_di->intfr.vrx;
}

int wlrx_get_intfr_irx(void)
{
	if (!g_wlrx_intfr_di)
		return 0;

	return g_wlrx_intfr_di->intfr.irx;
}

void wlrx_update_intfr_pctrl(struct wlrx_pctrl *pctrl)
{
	struct wlrx_intfr_info *di = g_wlrx_intfr_di;

	if (!pctrl || !di)
		return;

	if (di->intfr.vtx > 0)
		pctrl->vtx = min(pctrl->vtx, di->intfr.vtx);
	if (di->intfr.vrx > 0)
		pctrl->vrx = min(pctrl->vrx, di->intfr.vrx);
	if (di->intfr.irx > 0)
		pctrl->irx = min(pctrl->irx, di->intfr.irx);
}

static void wlrx_intfr_work(struct work_struct *work)
{
	int i;
	struct wlrx_intfr intfr;
	struct wlrx_dts *dts = wlrx_get_dts();
	struct wlrx_intfr_info *di = container_of(work,
		struct wlrx_intfr_info, work.work);

	if (!di || !dts || !dts->product_cfg || !dts->intfr_cfg)
		return;

	intfr.fixed_fop = WLRX_INTFR_DFT_FIXED_FOP;
	intfr.vtx = dts->product_cfg->vtx;
	intfr.vrx = dts->product_cfg->vrx;
	intfr.irx = dts->product_cfg->irx;
	for (i = 0; i < dts->intrf_src_level; i++) {
		if (!test_bit(i, (unsigned long *)&di->src))
			continue;
		if (dts->intfr_cfg[i].fixed_fop >= 0)
			intfr.fixed_fop = dts->intfr_cfg[i].fixed_fop;
		if (dts->intfr_cfg[i].vtx > 0)
			intfr.vtx = min(intfr.vtx, dts->intfr_cfg[i].vtx);
		if (dts->intfr_cfg[i].vrx > 0)
			intfr.vrx = min(intfr.vrx, dts->intfr_cfg[i].vrx);
		if (dts->intfr_cfg[i].irx > 0)
			intfr.irx = min(intfr.irx, dts->intfr_cfg[i].irx);
	}

	di->intfr.fixed_fop = intfr.fixed_fop;
	di->intfr.vtx = intfr.vtx;
	di->intfr.vrx = intfr.vrx;
	di->intfr.irx = intfr.irx;
	hwlog_info("[intfr_settings] fop=%d tx_vmax=%d rx_vmax=%d imax=%d\n",
		di->intfr.fixed_fop, di->intfr.vtx, di->intfr.vrx, di->intfr.irx);
}

void wlrx_handle_intfr_settings(u8 src_state)
{
	int i;
	struct wlrx_dts *dts = wlrx_get_dts();
	struct wlrx_intfr_info *di = g_wlrx_intfr_di;

	if (!di || !dts || !dts->intfr_cfg)
		return;

	for (i = 0; i < dts->intrf_src_level; i++) {
		if (src_state == dts->intfr_cfg[i].src_open) {
			di->src |= BIT(i);
			break;
		} else if (src_state == dts->intfr_cfg[i].src_close) {
			di->src &= ~BIT(i);
			break;
		}
	}
	if (i >= dts->intrf_src_level)
		return;

	if (delayed_work_pending(&di->work))
		return;

	hwlog_info("delay %dms update settings\n", WLRX_INTFR_TIMEOUT);
	schedule_delayed_work(&di->work, msecs_to_jiffies(WLRX_INTFR_TIMEOUT));
}

void wlrx_clear_intfr_settings(void)
{
	struct wlrx_intfr_info *di = g_wlrx_intfr_di;

	if (!di)
		return;

	di->src = 0;
	schedule_delayed_work(&di->work, msecs_to_jiffies(0));
}

static int __init wlrx_intfr_init(void)
{
	const char *status = NULL;
	struct wlrx_intfr_info *di = NULL;

	if (power_dts_read_string_compatible(power_dts_tag(HWLOG_TAG),
		"huawei,wireless_charger", "status", &status) ||
		!strcmp(status, "disabled"))
		return 0;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_wlrx_intfr_di = di;
	INIT_DELAYED_WORK(&di->work, wlrx_intfr_work);
	return 0;
}

static void __exit wlrx_intfr_exit(void)
{
	kfree(g_wlrx_intfr_di);
	g_wlrx_intfr_di = NULL;
}

module_init(wlrx_intfr_init);
module_exit(wlrx_intfr_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wireless rx interfere driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
