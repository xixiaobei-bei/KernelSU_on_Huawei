/*
 * wireless_rx_fod.c
 *
 * rx fod for wireless charging
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/wireless_charge/wireless_rx_fod.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_common.h>
#include <chipset_common/hwpower/protocol/wireless_protocol.h>

#define HWLOG_TAG wireless_rx_fod
HWLOG_REGIST();

struct wlrx_fod_dts {
	unsigned int status_cfg_len;
	u32 status_cfg[WLRX_SCN_END];
};

struct wlrx_fod_dev {
	int fod_status;
	struct wlrx_fod_dts *dts;
};

static struct wlrx_fod_dev *g_rx_fod_di[WLTRX_DRV_MAX];

static struct wlrx_fod_dev *wlrx_fod_get_di(unsigned int drv_type)
{
	if (!wltrx_is_drv_type_valid(drv_type)) {
		hwlog_err("get_di: drv_type=%u err\n", drv_type);
		return NULL;
	}

	return g_rx_fod_di[drv_type];
}

static unsigned int wlrx_fod_get_ic_type(unsigned int drv_type)
{
	return wltrx_get_dflt_ic_type(drv_type);
}

static int wlrx_fod_select_status_para(struct wlrx_fod_dev *di)
{
	enum wlrx_scene scn_id = wlrx_get_scene();

	if ((scn_id < 0) || (scn_id >= WLRX_SCN_END) || (di->dts->status_cfg[scn_id] <= 0))
		return -EINVAL;

	di->fod_status = di->dts->status_cfg[scn_id];
	return 0;
}

void wlrx_send_fod_status(unsigned int drv_type, unsigned int prot_type)
{
	int ret;
	struct wlrx_fod_dev *di = wlrx_fod_get_di(drv_type);

	if (!di || !di->dts || (di->dts->status_cfg_len <= 0))
		return;

	ret = wlrx_fod_select_status_para(di);
	if (ret)
		return;

	ret = wireless_send_fod_status(wlrx_fod_get_ic_type(drv_type),
		prot_type, di->fod_status);
	if (ret) {
		hwlog_err("send_fod_status: val=0x%x, failed\n", di->fod_status);
		return;
	}

	hwlog_info("[send_fod_status] val=0x%x, succ\n", di->fod_status);
}

static int wlrx_parse_fod_status(const struct device_node *np, struct wlrx_fod_dts *dts)
{
	int i, len;

	len = of_property_count_u32_elems(np, "fod_status");
	if ((len <= 0) || (len > WLRX_SCN_END)) {
		dts->status_cfg_len = 0;
		return 0;
	}
	dts->status_cfg_len = len;
	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"fod_status", (u32 *)dts->status_cfg, dts->status_cfg_len)) {
		dts->status_cfg_len = 0;
		return -EINVAL;
	}
	for (i = 0; i < dts->status_cfg_len; i++)
		hwlog_info("fod_status[%d]=0x%x\n", i, dts->status_cfg[i]);

	return 0;
}

static int wlrx_fod_parse_dts(const struct device_node *np, struct wlrx_fod_dts **dts)
{
	int ret;

	*dts = kzalloc(sizeof(**dts), GFP_KERNEL);
	if (!*dts)
		return -ENOMEM;

	ret = wlrx_parse_fod_status(np, *dts);
	if (ret)
		return ret;

	return 0;
}

static void wlrx_fod_kfree_dev(struct wlrx_fod_dev *di)
{
	if (!di)
		return;

	kfree(di->dts);
	kfree(di);
}

int wlrx_fod_init(unsigned int drv_type, const struct device_node *np)
{
	int ret;
	struct wlrx_fod_dev *di = NULL;

	if (!np || !wltrx_is_drv_type_valid(drv_type))
		return -EINVAL;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	ret = wlrx_fod_parse_dts(np, &di->dts);
	if (ret)
		goto exit;

	g_rx_fod_di[drv_type] = di;
	return 0;

exit:
	wlrx_fod_kfree_dev(di);
	return ret;
}

void wlrx_fod_deinit(unsigned int drv_type)
{
	if (!wltrx_is_drv_type_valid(drv_type))
		return;

	wlrx_fod_kfree_dev(g_rx_fod_di[drv_type]);
	g_rx_fod_di[drv_type] = NULL;
}
