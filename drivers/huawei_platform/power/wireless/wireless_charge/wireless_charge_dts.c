/*
 * wireless_charge_dts.c
 *
 * wireless charge driver, function as parsing dts
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#include <linux/of.h>
#include <linux/slab.h>

#include <huawei_platform/log/hw_log.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_ic_intf.h>
#include <huawei_platform/power/wireless/wireless_charger.h>

#define HWLOG_TAG wireless_dts
HWLOG_REGIST();

static int wlc_parse_u32_array(struct device_node *np, const char *prop,
	u32 row, u32 col, u32 *data)
{
	int ret;
	int len;

	if (!np || !prop || !data) {
		hwlog_err("parse_u32_array: para null\n");
		return -WLC_ERR_PARA_WRONG;
	}

	len = of_property_count_u32_elems(np, prop);
	if ((len <= 0) || (len % col) || (len > row * col)) {
		hwlog_err("parse_u32_array: %s invalid\n", prop);
		return -WLC_ERR_PARA_WRONG;
	}

	ret = of_property_read_u32_array(np, prop, data, len);
	if (ret) {
		hwlog_err("parse_u32_array: get %s fail\n", prop);
		return -WLC_ERR_PARA_WRONG;
	}

	return len;
}

static void wlc_parse_rx_temp_para(struct device_node *np, struct wlrx_dev_info *di)
{
	int i;
	int arr_len;
	u32 tmp_para[WLC_TEMP_PARA_LEVEL * WLC_TEMP_INFO_MAX] = { 0 };

	arr_len = wlc_parse_u32_array(np, "temp_para",
		WLC_TEMP_PARA_LEVEL, WLC_TEMP_INFO_MAX, tmp_para);
	if (arr_len <= 0)
		return;

	for (i = 0; i < arr_len / WLC_TEMP_INFO_MAX; i++) {
		di->temp_para[i].temp_lth = tmp_para[WLC_TEMP_INFO_MAX * i +
			WLC_TEMP_INFO_TEMP_LTH];
		di->temp_para[i].temp_hth = tmp_para[WLC_TEMP_INFO_MAX * i +
			WLC_TEMP_INFO_TEMP_HTH];
		di->temp_para[i].iout_max = tmp_para[WLC_TEMP_INFO_MAX * i +
			WLC_TEMP_INFO_IOUT_MAX];
		di->temp_para[i].temp_back = tmp_para[WLC_TEMP_INFO_MAX * i +
			WLC_TEMP_INFO_TEMP_BACK];

		hwlog_info("temp_para[%d] lth:%d hth:%d iout:%dmA back:%d\n",
			i, di->temp_para[i].temp_lth,
			di->temp_para[i].temp_hth, di->temp_para[i].iout_max,
			di->temp_para[i].temp_back);
	}
	di->flag.mon_imax = WLC_NEED_MON_IMAX;
}

static void wlc_parse_rx_time_para(struct device_node *np, struct wlrx_dev_info *di)
{
	int i;
	int arr_len;
	u32 tmp_para[WLC_TIME_PARA_LEVEL * WLC_TIME_INFO_MAX] = { 0 };

	arr_len = wlc_parse_u32_array(np, "time_para",
		WLC_TIME_PARA_LEVEL, WLC_TIME_INFO_MAX, tmp_para);
	if (arr_len <= 0)
		return;

	for (i = 0; i < arr_len / WLC_TIME_INFO_MAX; i++) {
		di->time_para[i].time_th = tmp_para[WLC_TIME_INFO_MAX * i +
			WLC_TIME_INFO_TIME_TH];
		di->time_para[i].iout_max = tmp_para[WLC_TIME_INFO_MAX * i +
			WLC_TIME_INFO_IOUT_MAX];

		hwlog_info("time_para[%d] lth:%ds iout:%dmA\n",
			i, di->time_para[i].time_th, di->time_para[i].iout_max);
	}
	di->flag.mon_imax = WLC_NEED_MON_IMAX;
}

static void wlc_parse_rx_time_temp_para(struct wlrx_dev_info *di)
{
	struct device_node *np = NULL;

	if (!di)
		return;
	di->flag.mon_imax = WLC_NO_NEED_MON_IMAX;

	np = wlrx_ic_get_dev_node(WLTRX_IC_MAIN);
	if (!np) {
		hwlog_err("parse_rx_time_temp_para: np null\n");
		return;
	}

	wlc_parse_rx_temp_para(np, di);
	wlc_parse_rx_time_para(np, di);
}

static void wlc_parse_iout_ctrl_para(struct device_node *np, struct wlrx_dev_info *di)
{
	int i;
	int para_id;
	int arr_len;
	u32 tmp_para[WLC_ICTRL_TOTAL * WLC_ICTRL_PARA_LEVEL] = { 0 };
	struct wireless_iout_ctrl_para *para = NULL;

	arr_len = wlc_parse_u32_array(np, "rx_iout_ctrl_para",
		WLC_ICTRL_PARA_LEVEL, WLC_ICTRL_TOTAL, tmp_para);
	if (arr_len <= 0) {
		di->iout_ctrl_data.ictrl_para_level = 0;
		return;
	}

	di->iout_ctrl_data.ictrl_para =
		kzalloc(sizeof(u32) * arr_len, GFP_KERNEL);
	if (!di->iout_ctrl_data.ictrl_para) {
		di->iout_ctrl_data.ictrl_para_level = 0;
		hwlog_err("parse_iout_ctrl_para: alloc ictrl_para failed\n");
		return;
	}

	di->iout_ctrl_data.ictrl_para_level = arr_len / WLC_ICTRL_TOTAL;
	for (i = 0; i < di->iout_ctrl_data.ictrl_para_level; i++) {
		para = &di->iout_ctrl_data.ictrl_para[i];
		para_id = WLC_ICTRL_BEGIN + WLC_ICTRL_TOTAL * i;
		para->iout_min = (int)tmp_para[para_id];
		para->iout_max = (int)tmp_para[++para_id];
		para->iout_set = (int)tmp_para[++para_id];
		hwlog_info("ictrl_para[%d], imin: %-4d imax: %-4d iset: %-4d\n",
			i, di->iout_ctrl_data.ictrl_para[i].iout_min,
			di->iout_ctrl_data.ictrl_para[i].iout_max,
			di->iout_ctrl_data.ictrl_para[i].iout_set);
	}
}

int wlc_parse_dts(struct device_node *np, struct wlrx_dev_info *di)
{
	if (!np || !di)
		return -EINVAL;

	wlc_parse_iout_ctrl_para(np, di);
	wlc_parse_rx_time_temp_para(di);

	return 0;
}
