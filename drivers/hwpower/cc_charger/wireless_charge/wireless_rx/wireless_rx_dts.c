/*
 * wireless_rx_dts.c
 *
 * parse dts for wireless charging
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
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_dts.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_auth.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_pctrl.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_pmode.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_interfere.h>

#define HWLOG_TAG wireless_rx_dts
HWLOG_REGIST();

/* for product_para */
#define WLRX_PRODUCT_CFG_LEN           3
/* for segment_para */
#define WLRX_SOC_CFG_ROW               5
#define WLRX_SOC_CFG_COL               5
/* for interference_para */
#define WLRX_INTERFERE_CFG_ROW         8
#define WLRX_INTERFERE_CFG_COL         6

/* for antifake_kid_para */
#define WLRX_AF_MIN_KID                0
#define WLRX_AF_MAX_KID                10
#define WLRX_AF_FAC_KID                0 /* factory key id */
#define WLRX_AF_DFT_KID                1

/* for vmode_para */
#define WLRX_VMODE_CFG_ROW             5
#define WLRX_VMODE_CFG_COL             2

static struct wlrx_dts *g_wlrx_dts;

struct wlrx_dts *wlrx_get_dts(void)
{
	return g_wlrx_dts;
}

static void wlrx_parse_antifake_kid(const struct device_node *np, struct wlrx_dts *dts)
{
	if (power_cmdline_is_factory_mode()) {
		dts->antifake_kid = WLRX_AF_FAC_KID;
		hwlog_info("antifake_kid=%d\n", dts->antifake_kid);
		return;
	}

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"antifake_key_index", (u32 *)&dts->antifake_kid, WLRX_AF_DFT_KID);
	if ((dts->antifake_kid > WLRX_AF_MAX_KID) ||
		(dts->antifake_kid < WLRX_AF_MIN_KID))
		dts->antifake_kid = WLRX_AF_DFT_KID;
	hwlog_info("antifake_kid=%d\n", dts->antifake_kid);
}

static int wlrx_parse_product_cfg(const struct device_node *np, struct wlrx_dts *dts)
{
	dts->product_cfg = kzalloc(sizeof(*dts->product_cfg), GFP_KERNEL);
	if (!dts->product_cfg)
		return -ENOMEM;

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"product_para", (u32 *)dts->product_cfg, WLRX_PRODUCT_CFG_LEN))
		return -EINVAL;

	hwlog_info("[product_cfg] vtx_max:%dmV vrx_max:%dmV irx_max:%dmA\n",
		dts->product_cfg->vtx, dts->product_cfg->vrx, dts->product_cfg->irx);
	return 0;
}

static int wlrx_parse_basic_cfg(const struct device_node *np, struct wlrx_dts *dts)
{
	int ret;

	wlrx_parse_antifake_kid(np, dts);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ui_max_pwr", (u32 *)&dts->ui_pmax_lth, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"product_max_pwr", (u32 *)&dts->product_pmax_hth, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"acc_det_pwr", (u32 *)&dts->accdet_pmax_lth, 27000); /* default_lth:27w */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"hvc_need_5vbst", (u32 *)&dts->hvc_need_5vbst, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"bst5v_ignore_vbus_only", (u32 *)&dts->bst5v_ignore_vbus_only, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_vout_err_ratio", (u32 *)&dts->rx_vout_err_ratio, 81); /* default_ratio:81% */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_iout_min", (u32 *)&dts->rx_imin, 150); /* default_imin:150mA */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_iout_step", (u32 *)&dts->rx_istep, 100); /* default_istep:100mA */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"pmax", &dts->pmax, 20); /* default_pmax:20*2=40w */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ignore_qval", &dts->ignore_qval, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sample_delay_time", &dts->sample_delay_time, 3000); /* default_delay:3s */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_high_pwr_wltx", &dts->support_high_pwr_wltx, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"wired_sw_dflt_on", (u32 *)&dts->wired_sw_dflt_on, 0);

	ret = wlrx_parse_product_cfg(np, dts);
	if (ret)
		return ret;

	return 0;
}

static int wlrx_pmode_cfg_str2int(const char *str, int *pmode, int i)
{
	if (kstrtoint(str, 0, &pmode[(i - 1) % WLRX_PMODE_CFG_COL]))
		return -EINVAL;

	return 0;
}

static int wlrx_parse_pmode_cfg(const struct device_node *np, struct wlrx_dts *dts)
{
	int i, len, size;
	const char *tmp_str = NULL;
	struct wlrx_pmode *pmode = NULL;

	len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"rx_mode_para", WLRX_PMODE_CFG_ROW, WLRX_PMODE_CFG_COL);
	if (len <= 0)
		return -EINVAL;

	size = sizeof(*dts->pmode_cfg) * (len / WLRX_PMODE_CFG_COL);
	dts->pmode_cfg = kzalloc(size, GFP_KERNEL);
	if (!dts->pmode_cfg)
		return -ENOMEM;

	dts->pmode_cfg_level = len / WLRX_PMODE_CFG_COL;
	for (i = 0; i < len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np,
			"rx_mode_para", i, &tmp_str))
			return -EINVAL;
		if ((i % WLRX_PMODE_CFG_COL) == 0) { /* 0: pmode name */
			dts->pmode_cfg[i / WLRX_PMODE_CFG_COL].name = tmp_str;
			continue;
		}
		if (wlrx_pmode_cfg_str2int(tmp_str,
			(int *)&dts->pmode_cfg[i / WLRX_PMODE_CFG_COL].vtx_min, i))
			return -EINVAL;
	}
	for (i = 0; i < dts->pmode_cfg_level; i++) {
		pmode = &dts->pmode_cfg[i];
		hwlog_info("pmode[%d] name:%-4s vtx_min:%-5d itx_min:%-4d\t"
			"vtx:%-5d vrx:%-5d irx:%-4d vrect_lth:%-5d tbatt:%-3d\t"
			"cable:%-2d auth:%-2d icon:%d timeout:%-4d expect_mode:%-2d\n",
			i, pmode->name, pmode->vtx_min, pmode->itx_min,
			pmode->vtx, pmode->vrx, pmode->irx, pmode->vrect_lth,
			pmode->tbatt, pmode->cable, pmode->auth, pmode->icon,
			pmode->timeout, pmode->expect_mode);
	}
	return 0;
}

static void wlrx_parse_soc_cfg(const struct device_node *np, struct wlrx_dts *dts)
{
	int i, len, size;

	len = power_dts_read_u32_count(power_dts_tag(HWLOG_TAG), np,
		"segment_para", WLRX_SOC_CFG_ROW, WLRX_SOC_CFG_COL);
	if (len <= 0)
		return;

	size = sizeof(*dts->soc_cfg) * (len / WLRX_SOC_CFG_COL);
	dts->soc_cfg = kzalloc(size, GFP_KERNEL);
	if (!dts->soc_cfg)
		return;

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"segment_para", (u32 *)dts->soc_cfg, len))
		return;

	dts->soc_cfg_level = len / WLRX_SOC_CFG_COL;
	for (i = 0; i < dts->soc_cfg_level; i++)
		hwlog_info("soc_cfg[%d] soc_min:%-3d soc_max:%-3d\t"
			"vtx: %-5d vrxt: %-5d irx: %-4d\n", i,
			dts->soc_cfg[i].soc_min, dts->soc_cfg[i].soc_max,
			dts->soc_cfg[i].vtx, dts->soc_cfg[i].vrx,
			dts->soc_cfg[i].irx);
}

static void wlrx_parse_interfer_cfg(const struct device_node *np, struct wlrx_dts *dts)
{
	int i, len, level, size;

	len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"interference_para", WLRX_INTERFERE_CFG_ROW, WLRX_INTERFERE_CFG_COL);
	if (len < 0)
		return;

	level = len / WLRX_INTERFERE_CFG_COL;
	size = sizeof(*dts->intfr_cfg) * level;
	dts->intfr_cfg = kzalloc(size, GFP_KERNEL);
	if (!dts->intfr_cfg)
		return;

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"interference_para", (int *)dts->intfr_cfg, level,
		WLRX_INTERFERE_CFG_COL);
	if (len <= 0)
		return;

	dts->intrf_src_level = level;
	for (i = 0; i < dts->intrf_src_level; i++)
		hwlog_info("interfer_cfg[%d] [src] open:0x%-2x close:0x%-2x\t"
			"[limit] fop:%-3d vtx:%-5d vrx:%-5d irx:%-4d\n", i,
			dts->intfr_cfg[i].src_open, dts->intfr_cfg[i].src_close,
			dts->intfr_cfg[i].fixed_fop, dts->intfr_cfg[i].vtx,
			dts->intfr_cfg[i].vrx, dts->intfr_cfg[i].irx);
}

static int wlrx_parse_vmode_cfg(const struct device_node *np, struct wlrx_dts *dts)
{
	int i, len, size;

	len = power_dts_read_u32_count(power_dts_tag(HWLOG_TAG), np,
		"volt_mode", WLRX_VMODE_CFG_ROW, WLRX_VMODE_CFG_COL);
	if (len <= 0)
		return -EINVAL;

	size = sizeof(*dts->vmode_cfg) * (len / WLRX_VMODE_CFG_COL);
	dts->vmode_cfg = kzalloc(size, GFP_KERNEL);
	if (!dts->vmode_cfg)
		return -ENOMEM;

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"volt_mode", (u32 *)dts->vmode_cfg, len))
		return -EINVAL;

	dts->vmode_cfg_level = len / WLRX_VMODE_CFG_COL;
	for (i = 0; i < dts->vmode_cfg_level; i++)
		hwlog_info("vmode[%d], id: %u vtx: %-5d\n",
			i, dts->vmode_cfg[i].id, dts->vmode_cfg[i].vtx);

	return 0;
}

void wlrx_kfree_dts(void)
{
	if (!g_wlrx_dts)
		return;

	kfree(g_wlrx_dts->product_cfg);
	kfree(g_wlrx_dts->pmode_cfg);
	kfree(g_wlrx_dts->vmode_cfg);
	kfree(g_wlrx_dts->soc_cfg);
	kfree(g_wlrx_dts->intfr_cfg);
	kfree(g_wlrx_dts);
	g_wlrx_dts = NULL;
}

int wlrx_parse_dts(const struct device_node *np)
{
	int ret;
	struct wlrx_dts *dts = NULL;

	if (!np)
		return -EINVAL;

	dts = kzalloc(sizeof(*dts), GFP_KERNEL);
	if (!dts)
		return -ENOMEM;

	g_wlrx_dts = dts;
	ret = wlrx_parse_basic_cfg(np, dts);
	if (ret)
		goto exit;

	ret = wlrx_parse_pmode_cfg(np, dts);
	if (ret)
		goto exit;

	ret = wlrx_parse_vmode_cfg(np, dts);
	if (ret)
		goto exit;

	wlrx_parse_soc_cfg(np, dts);
	wlrx_parse_interfer_cfg(np, dts);
	return 0;

exit:
	wlrx_kfree_dts();
	return -EINVAL;
}
