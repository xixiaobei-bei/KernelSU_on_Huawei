/*
 * wireless_dts.c
 *
 * wireless charge driver, function as parsing dts
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

#include <linux/of.h>
#include <linux/slab.h>

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/power_dts.h>
#include <huawei_platform/power/power_cmdline.h>
#include <huawei_platform/power/wireless_charger.h>

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

static void wlc_parse_basic_antifake_kid(struct device_node *np,
	struct wireless_charge_device_info *di)
{
	if (power_cmdline_is_factory_mode()) {
		di->dts.antifake_kid = 0;
		hwlog_info("antifake_kid=%d\n", di->dts.antifake_kid);
		return;
	}

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"antifake_key_index", (u32 *)&di->dts.antifake_kid, 1);
	if ((di->dts.antifake_kid >= WC_AF_TOTAL_KEY_NUM) ||
		(di->dts.antifake_kid < 0))
		di->dts.antifake_kid = 1;
	hwlog_info("antifake_kid=%d\n", di->dts.antifake_kid);
}

static int wlc_parse_basic_dts(struct device_node *np,
	struct wireless_charge_device_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"hvc_need_5vbst", (u32 *)&di->hvc_need_5vbst, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"bst5v_ignore_vbus_only",
		(u32 *)&di->bst5v_ignore_vbus_only, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"standard_tx_adaptor", (u32 *)&di->standard_tx_adaptor,
		WIRELESS_UNKOWN);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_vout_err_ratio", (u32 *)&di->rx_vout_err_ratio,
		RX_VOUT_ERR_RATIO);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_iout_min", (u32 *)&di->rx_iout_min, RX_IOUT_MIN);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_iout_step", (u32 *)&di->rx_iout_step, RX_IOUT_REG_STEP);
	wlc_parse_basic_antifake_kid(np, di);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ui_max_pwr", (u32 *)&di->dts.ui_max_pwr, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"fod_status", &di->fod_status, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"pmax", &di->pmax, WLC_PMAX_DEFAULT_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ignore_qval", &di->ignore_qval, 0);
	return 0;
}

static void wlc_parse_rx_temp_para(struct device_node *np,
	struct wireless_charge_device_info *di)
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

static void wlc_parse_rx_time_para(struct device_node *np,
	struct wireless_charge_device_info *di)
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

static void wlc_parse_rx_time_temp_para(
	struct wireless_charge_device_info *di)
{
	struct device_node *np = NULL;

	if (!di)
		return;
	di->flag.mon_imax = WLC_NO_NEED_MON_IMAX;

	if (!di->ops || !di->ops->dev_node)
		return;

	np = di->ops->dev_node();
	if (!np) {
		hwlog_err("parse_rx_time_temp_para: rx_dev_node null\n");
		return;
	}

	wlc_parse_rx_temp_para(np, di);
	wlc_parse_rx_time_para(np, di);
}

static void wlc_parse_interfer_para_id(struct device_node *np,
	struct wireless_charge_device_info *di, int id)
{
	long data = 0;
	const char *str = NULL;
	struct wireless_interfer_para *para = NULL;

	if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
		np, "interference_para", id, &str)) {
		di->interfer_data.total_src = 0;
		return;
	}

	if (kstrtol(str, 0, &data) < 0)
		return;
	para = &di->interfer_data.interfer_para[id / WLC_INTERFER_TOTAL];
	switch (id % WLC_INTERFER_TOTAL) {
	case WLC_INTERFER_SRC_OPEN:
		para->src_open = (u8)data;
		break;
	case WLC_INTERFER_SRC_CLOSE:
		para->src_close = (u8)data;
		break;
	case WLC_INTERFER_TX_FIXED_FOP:
		para->tx_fixed_fop = (int)data;
		break;
	case WLC_INTERFER_TX_VOUT_LIMIT:
		para->tx_vout_limit = (int)data;
		break;
	case WLC_INTERFER_RX_VOUT_LIMIT:
		para->rx_vout_limit = (int)data;
		break;
	case WLC_INTERFER_RX_IOUT_LIMIT:
		para->rx_iout_limit = (int)data;
		break;
	default:
		hwlog_err("parse_interfer_para_id_data: mismatch\n");
	}
}

static void wlc_parse_interfer_para(struct device_node *np,
	struct wireless_charge_device_info *di)
{
	int i;
	int str_len;

	str_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"interference_para", WLC_INTERFER_PARA_LEVEL,
		WLC_INTERFER_TOTAL);
	if (str_len < 0) {
		di->interfer_data.total_src = 0;
		return;
	}

	di->interfer_data.interfer_src_state = 0;
	di->interfer_data.total_src = str_len / WLC_INTERFER_TOTAL;
	for (i = 0; i < str_len; i++)
		wlc_parse_interfer_para_id(np, di, i);

	for (i = 0; i < di->interfer_data.total_src; i++)
		hwlog_info("interfer_para[%d] [src] open:0x%-2x close:0x%-2x\t"
			"[limit] fop:%-3d vtx:%-5d vrx:%-5d irx: %-4d\n",
			i, di->interfer_data.interfer_para[i].src_open,
			di->interfer_data.interfer_para[i].src_close,
			di->interfer_data.interfer_para[i].tx_fixed_fop,
			di->interfer_data.interfer_para[i].tx_vout_limit,
			di->interfer_data.interfer_para[i].rx_vout_limit,
			di->interfer_data.interfer_para[i].rx_iout_limit);
}

static void wlc_parse_segment_para(struct device_node *np,
	struct wireless_charge_device_info *di)
{
	int i;
	int para_id;
	int arr_len;
	struct wireless_segment_para *para = NULL;
	u32 tmp_para[WLC_SEGMENT_PARA_TOTAL * WLC_SEGMENT_PARA_LEVEL] = { 0 };

	arr_len = wlc_parse_u32_array(np, "segment_para",
		WLC_SEGMENT_PARA_LEVEL, WLC_SEGMENT_PARA_TOTAL, tmp_para);
	if (arr_len <= 0) {
		di->segment_data.segment_para_level = 0;
		return;
	}

	di->segment_data.segment_para_level = arr_len / WLC_SEGMENT_PARA_TOTAL;
	for (i = 0; i < di->segment_data.segment_para_level; i++) {
		para = &di->segment_data.segment_para[i];
		para_id = WLC_SEGMENT_PARA_START + WLC_SEGMENT_PARA_TOTAL * i;
		para->soc_min = (int)tmp_para[para_id];
		para->soc_max = (int)tmp_para[++para_id];
		para->tx_vout_limit = (int)tmp_para[++para_id];
		para->rx_vout_limit = (int)tmp_para[++para_id];
		para->rx_iout_limit = (int)tmp_para[++para_id];
		hwlog_info("segment_para[%d], soc_min: %-3d soc_max: %-3d\t"
			"vtx_limit: %-5d vrx_limit: %-5d irx_limit: %-4d\n",
			i, di->segment_data.segment_para[i].soc_min,
			di->segment_data.segment_para[i].soc_max,
			di->segment_data.segment_para[i].tx_vout_limit,
			di->segment_data.segment_para[i].rx_vout_limit,
			di->segment_data.segment_para[i].rx_iout_limit);
	}
}

static void wlc_parse_iout_ctrl_para(struct device_node *np,
	struct wireless_charge_device_info *di)
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

static void wlc_assign_basic_pmode_para(
	struct wireless_mode_para *para, const char *str, int id)
{
	long data = 0;

	if ((id % WLC_MODE_INFO_TOTAL) == WLC_MODE_NAME) {
		para->mode_name = str;
		return;
	}
	if (kstrtol(str, 0, &data) < 0)
		return;

	switch (id % WLC_MODE_INFO_TOTAL) {
	case WLC_MODE_TX_VOUT_MIN:
		para->tx_vout_min = (int)data;
		break;
	case WLC_MODE_TX_IOUT_MIN:
		para->tx_iout_min = (int)data;
		break;
	case WLC_MODE_TX_VOUT:
		para->ctrl_para.tx_vout = (int)data;
		break;
	case WLC_MODE_RX_VOUT:
		para->ctrl_para.rx_vout = (int)data;
		break;
	case WLC_MODE_RX_IOUT:
		para->ctrl_para.rx_iout = (int)data;
		break;
	default:
		break;
	}
}

static void wlc_assign_extra_pmode_para(
	struct wireless_mode_para *para, const char *str, int id)
{
	long data = 0;

	if (kstrtol(str, 0, &data) < 0)
		return;

	switch (id % WLC_MODE_INFO_TOTAL) {
	case WLC_MODE_VRECT_LOW_TH:
		para->vrect_low_th = (int)data;
		break;
	case WLC_MODE_TBATT:
		para->tbatt = (int)data;
		break;
	case WLC_MODE_EXPECT_CABLE_DETECT:
		para->expect_cable_detect = (s8)data;
		break;
	case WLC_MODE_EXPECT_CERT:
		para->expect_cert = (s8)data;
		break;
	case WLC_MODE_ICON_TYPE:
		para->icon_type = (u8)data;
		break;
	case WLC_MODE_MAX_TIME:
		para->max_time = (int)data;
		break;
	case WLC_MODE_EXPECT_MODE:
		para->expect_mode = (s8)data;
		break;
	default:
		break;
	}
}

static void wlc_show_mode_para(struct wireless_charge_device_info *di)
{
	int i;

	for (i = 0; i < di->mode_data.total_mode; i++)
		hwlog_info("mode_para[%d] name:%-4s vtx_min:%-5d itx_min:%-4d\t"
			"vtx:%-5d vrx:%-5d irx:%-4d vrect_lth:%-5d tbatt:%-3d\t"
			"expect_cable_detect: %-2d expect_cert: %-2d\t"
			"icon_type:%d max_time:%-4d expect_mode: %-2d\n",
			i, di->mode_data.mode_para[i].mode_name,
			di->mode_data.mode_para[i].tx_vout_min,
			di->mode_data.mode_para[i].tx_iout_min,
			di->mode_data.mode_para[i].ctrl_para.tx_vout,
			di->mode_data.mode_para[i].ctrl_para.rx_vout,
			di->mode_data.mode_para[i].ctrl_para.rx_iout,
			di->mode_data.mode_para[i].vrect_low_th,
			di->mode_data.mode_para[i].tbatt,
			di->mode_data.mode_para[i].expect_cable_detect,
			di->mode_data.mode_para[i].expect_cert,
			di->mode_data.mode_para[i].icon_type,
			di->mode_data.mode_para[i].max_time,
			di->mode_data.mode_para[i].expect_mode);
}

static int wlc_parse_pmode_para(struct device_node *np,
	struct wireless_charge_device_info *di)
{
	int i;
	int str_len;
	size_t size;
	const char *str = NULL;
	struct wireless_mode_para *para = NULL;

	str_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"rx_mode_para", WLC_MODE_TYPE_MAX, WLC_MODE_INFO_TOTAL);
	if (str_len <= 0) {
		di->mode_data.total_mode = 0;
		return -WLC_ERR_PARA_WRONG;
	}

	di->mode_data.total_mode = str_len / WLC_MODE_INFO_TOTAL;
	size = sizeof(*(di->mode_data.mode_para)) * di->mode_data.total_mode;
	di->mode_data.mode_para = kzalloc(size, GFP_KERNEL);

	if (!di->mode_data.mode_para) {
		di->mode_data.total_mode = 0;
		hwlog_err("parse_mode_para: alloc mode_para failed\n");
		return -WLC_ERR_NO_SPACE;
	}

	for (i = 0; i < str_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "rx_mode_para", i, &str)) {
			di->mode_data.total_mode = 0;
			return -WLC_ERR_PARA_WRONG;
		}
		para = &di->mode_data.mode_para[i / WLC_MODE_INFO_TOTAL];
		wlc_assign_basic_pmode_para(para, str, i);
		wlc_assign_extra_pmode_para(para, str, i);
	}

	wlc_show_mode_para(di);

	return 0;
}

static void wlc_parse_tx_prop_para_id(
	struct wireless_tx_prop_para *para, const char *str, int id)
{
	long data = 0;

	if ((id % WLC_TX_PROP_TOTAL) == WLC_TX_TYPE_NAME) {
		para->type_name = str;
		return;
	}

	if (kstrtol(str, 0, &data) < 0)
		return;

	switch (id % WLC_TX_PROP_TOTAL) {
	case WLC_TX_ADAPTOR_TYPE:
		para->tx_type = (u8)data;
		break;
	case WLC_TX_NEED_CABLE_DETECT:
		para->need_cable_detect = (u8)data;
		break;
	case WLC_TX_NEED_CERT:
		para->need_cert = (u8)data;
		break;
	case WLC_TX_DEFAULT_VOUT:
		para->tx_default_vout = (int)data;
		break;
	case WLC_TX_DEFAULT_IOUT:
		para->tx_default_iout = (int)data;
		break;
	default:
		hwlog_err("parse_tx_prop_para_id: invalid para\n");
	}
}

static void wlc_show_tx_prop_para(struct wireless_charge_device_info *di)
{
	int i;

	for (i = 0; i < di->tx_prop_data.total_prop_type; i++)
		hwlog_info("tx_prop[%d] type:0x%-2x name:%-7s\t"
			"need_cable_detect:%d need_cert:%d\t"
			"tx_default_vout:%-5d tx_default_iout:%-4d\n",
			i, di->tx_prop_data.tx_prop[i].tx_type,
			di->tx_prop_data.tx_prop[i].type_name,
			di->tx_prop_data.tx_prop[i].need_cable_detect,
			di->tx_prop_data.tx_prop[i].need_cert,
			di->tx_prop_data.tx_prop[i].tx_default_vout,
			di->tx_prop_data.tx_prop[i].tx_default_iout);
}

static int wlc_parse_tx_prop_para(struct device_node *np,
	struct wireless_charge_device_info *di)
{
	int i;
	int str_len;
	size_t size;
	const char *str = NULL;
	struct wireless_tx_prop_para *para = NULL;

	str_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"tx_prop", WLC_TX_TYPE_MAX, WLC_TX_PROP_TOTAL);
	if (str_len < 0) {
		di->tx_prop_data.total_prop_type = 0;
		return -WLC_ERR_PARA_WRONG;
	}

	di->tx_prop_data.total_prop_type = str_len / WLC_TX_PROP_TOTAL;
	size = sizeof(*(di->tx_prop_data.tx_prop)) *
		di->tx_prop_data.total_prop_type;
	di->tx_prop_data.tx_prop = kzalloc(size, GFP_KERNEL);
	if (!di->tx_prop_data.tx_prop) {
		di->tx_prop_data.total_prop_type = 0;
		hwlog_err("parse_tx_prop_para: alloc tx_prop failed\n");
		return -WLC_ERR_NO_SPACE;
	}

	for (i = 0; i < str_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "tx_prop", i, &str)) {
			di->tx_prop_data.total_prop_type = 0;
			return -WLC_ERR_PARA_WRONG;
		}
		para = &di->tx_prop_data.tx_prop[i / WLC_TX_PROP_TOTAL];
		wlc_parse_tx_prop_para_id(para, str, i);
	}

	wlc_show_tx_prop_para(di);

	return 0;
}

static int wlc_parse_product_para(struct device_node *np,
	struct wireless_charge_device_info *di)
{
	u32 tmp_para[WLC_CTRL_PARA_TOTAL] = { 0 };

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"product_para", tmp_para, WLC_CTRL_PARA_TOTAL))
		return -WLC_ERR_PARA_WRONG;

	di->product_para.tx_vout = (int)tmp_para[WLC_CTRL_TX_VOUT];
	di->product_para.rx_vout = (int)tmp_para[WLC_CTRL_RX_VOUT];
	di->product_para.rx_iout = (int)tmp_para[WLC_CTRL_RX_IOUT];
	hwlog_info("product_para vtx_max:%dmV vrx_max:%dmV irx_max:%dmA\n",
		di->product_para.tx_vout, di->product_para.rx_vout,
		di->product_para.rx_iout);

	return 0;
}

static int wlc_parse_vmode_para(struct device_node *np,
	struct wireless_charge_device_info *di)
{
	int i;
	size_t size;
	int arr_len;
	u32 tmp_para[WLC_VMODE_TOTAL * WLC_VMODE_TYPE_LEVEL] = { 0 };

	arr_len = wlc_parse_u32_array(np, "volt_mode",
		WLC_VMODE_TYPE_LEVEL, WLC_VMODE_TOTAL, tmp_para);
	if (arr_len <= 0) {
		di->volt_mode_data.total_volt_mode = 0;
		return -WLC_ERR_PARA_WRONG;
	}

	di->volt_mode_data.total_volt_mode = arr_len / WLC_VMODE_TOTAL;
	size = sizeof(*(di->volt_mode_data.volt_mode)) *
		di->volt_mode_data.total_volt_mode;
	di->volt_mode_data.volt_mode = kzalloc(size, GFP_KERNEL);
	if (!di->volt_mode_data.volt_mode) {
		di->volt_mode_data.total_volt_mode = 0;
		hwlog_err("parse_vmode_para: alloc volt_mode failed\n");
		return -WLC_ERR_NO_SPACE;
	}

	for (i = 0; i < di->volt_mode_data.total_volt_mode; i++) {
		di->volt_mode_data.volt_mode[i].mode_type =
			(u8)tmp_para[WLC_VMODE_TYPE + WLC_VMODE_TOTAL * i];
		di->volt_mode_data.volt_mode[i].tx_vout =
			(int)tmp_para[WLC_VMODE_TX_VOUT + WLC_VMODE_TOTAL * i];
		hwlog_info("vmode[%d], mode_type: %d tx_vout: %-5d\n",
			i, di->volt_mode_data.volt_mode[i].mode_type,
			di->volt_mode_data.volt_mode[i].tx_vout);
	}

	return 0;
}

int wlc_parse_dts(struct device_node *np,
	struct wireless_charge_device_info *di)
{
	int ret;

	if (!np || !di)
		return -EINVAL;

	if (wlc_parse_basic_dts(np, di))
		return -EINVAL;
	wlc_parse_interfer_para(np, di);
	wlc_parse_segment_para(np, di);
	wlc_parse_iout_ctrl_para(np, di);
	wlc_parse_rx_time_temp_para(di);

	ret = wlc_parse_pmode_para(np, di);
	if (ret) {
		hwlog_err("parse_dts: get rx_mode_para failed\n");
		return -EINVAL;
	}
	ret = wlc_parse_tx_prop_para(np, di);
	if (ret) {
		hwlog_err("parse_dts: get tx_act failed\n");
		return -EINVAL;
	}
	ret = wlc_parse_product_para(np, di);
	if (ret) {
		hwlog_err("parse_dts: get product_para failed\n");
		return -EINVAL;
	}
	ret = wlc_parse_vmode_para(np, di);
	if (ret) {
		hwlog_err("parse_dts: get volt_mode failed\n");
		return -EINVAL;
	}

	return 0;
}
