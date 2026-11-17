/*
 * direct_charge_debug.c
 *
 * debug for direct charge module
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

#include <linux/types.h>
#include <chipset_common/hwpower/common_module/power_temp.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <huawei_platform/power/direct_charger/direct_charger.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG direct_charge_debug
HWLOG_REGIST();

#define OUT_BUF_SIZE  256
#define TMP_BUF_SIZE  32

static ssize_t dc_dbg_vstep_para_store(void *dev_data,
	const char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;
	int paras = DC_VSTEP_PARA_LEVEL * DC_VSTEP_INFO_MAX;
	char out_buf[OUT_BUF_SIZE] = { 0 };
	char tmp_buf[TMP_BUF_SIZE] = { 0 };
	int i;

	if (!di)
		return -EINVAL;

	/* [0] [1] [2] [3]: four group parameters */
	if (sscanf(buf, "%d %d %d %d %d %d %d %d",
		&di->vstep_para[0].curr_gap, &di->vstep_para[0].vstep,
		&di->vstep_para[1].curr_gap, &di->vstep_para[1].vstep,
		&di->vstep_para[2].curr_gap, &di->vstep_para[2].vstep,
		&di->vstep_para[3].curr_gap, &di->vstep_para[3].vstep) != paras)
		return -EINVAL;

	for (i = 0; i < DC_VSTEP_PARA_LEVEL; i++) {
		memset(tmp_buf, 0, TMP_BUF_SIZE);
		scnprintf(tmp_buf, TMP_BUF_SIZE, "%d   %d\n",
			di->vstep_para[i].curr_gap, di->vstep_para[i].vstep);
		strncat(out_buf, tmp_buf, strlen(tmp_buf));
	}

	hwlog_info("dc_vstep_para:\n%s\n", out_buf);
	return size;
}

static ssize_t dc_dbg_vstep_para_show(void *dev_data, char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;
	char out_buf[OUT_BUF_SIZE] = { 0 };
	char tmp_buf[TMP_BUF_SIZE] = { 0 };
	int i;

	if (!di)
		return 0;

	for (i = 0; i < DC_VSTEP_PARA_LEVEL; i++) {
		memset(tmp_buf, 0, TMP_BUF_SIZE);
		scnprintf(tmp_buf, TMP_BUF_SIZE, "%d   %d\n",
			di->vstep_para[i].curr_gap, di->vstep_para[i].vstep);
		strncat(out_buf, tmp_buf, strlen(tmp_buf));
	}

	return scnprintf(buf, size, "dc_vstep_para:\n%s\n", out_buf);
}

static ssize_t dc_dbg_time_para_store(void *dev_data,
	const char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;
	int paras = DC_TIME_PARA_LEVEL * DC_TIME_INFO_MAX;
	char out_buf[OUT_BUF_SIZE] = { 0 };
	char tmp_buf[TMP_BUF_SIZE] = { 0 };
	int i;

	if (!di)
		return -EINVAL;

	/* [0] [1] [2] [3] [4]: five group parameters */
	if (sscanf(buf, "%d %d %d %d %d %d %d %d %d %d",
		&di->time_para[0].time_th, &di->time_para[0].ibat_max,
		&di->time_para[1].time_th, &di->time_para[1].ibat_max,
		&di->time_para[2].time_th, &di->time_para[2].ibat_max,
		&di->time_para[3].time_th, &di->time_para[3].ibat_max,
		&di->time_para[4].time_th, &di->time_para[4].ibat_max) != paras)
		return -EINVAL;

	for (i = 0; i < DC_TIME_PARA_LEVEL; i++) {
		memset(tmp_buf, 0, TMP_BUF_SIZE);
		scnprintf(tmp_buf, TMP_BUF_SIZE, "%d   %d\n",
			di->time_para[i].time_th, di->time_para[i].ibat_max);
		strncat(out_buf, tmp_buf, strlen(tmp_buf));
	}

	di->time_para_parse_ok = 1; /* set parse ok flag */
	hwlog_info("dc_time_para:\n%s\n", out_buf);
	return size;
}

static ssize_t dc_dbg_time_para_show(void *dev_data, char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;
	char out_buf[OUT_BUF_SIZE] = { 0 };
	char tmp_buf[TMP_BUF_SIZE] = { 0 };
	int i;

	if (!di)
		return 0;

	for (i = 0; i < DC_TIME_PARA_LEVEL; i++) {
		memset(tmp_buf, 0, TMP_BUF_SIZE);
		scnprintf(tmp_buf, TMP_BUF_SIZE, "%d   %d\n",
			di->time_para[i].time_th, di->time_para[i].ibat_max);
		strncat(out_buf, tmp_buf, strlen(tmp_buf));
	}

	return scnprintf(buf, size, "dc_time_para:\n%s\n", out_buf);
}

static ssize_t dc_dbg_total_ibus_show(void *dev_data, char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;
	int ibus = 0;

	if (!di)
		return -EINVAL;

	(void)dcm_get_ic_ibus(di->local_mode, CHARGE_MULTI_IC, &ibus);
	return scnprintf(buf, size, "%d\n", ibus);
}

static ssize_t dc_dbg_main_ibus_show(void *dev_data, char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;
	int ibus = 0;

	if (!di)
		return -EINVAL;

	(void)dcm_get_ic_ibus(di->local_mode, CHARGE_IC_MAIN, &ibus);
	return scnprintf(buf, size, "%d\n", ibus);
}

static ssize_t dc_dbg_aux_ibus_show(void *dev_data, char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;
	int ibus = 0;

	if (!di)
		return -EINVAL;

	(void)dcm_get_ic_ibus(di->local_mode, CHARGE_IC_AUX, &ibus);
	return scnprintf(buf, size, "%d\n", ibus);
}

static ssize_t dc_dbg_main_vbus_show(void *dev_data, char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;
	int vbus = 0;

	if (!di)
		return -EINVAL;

	(void)dcm_get_ic_vbus(di->local_mode, CHARGE_IC_MAIN, &vbus);
	return scnprintf(buf, size, "%d\n", vbus);
}

static ssize_t dc_dbg_aux_vbus_show(void *dev_data, char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;
	int vbus = 0;

	if (!di)
		return -EINVAL;

	(void)dcm_get_ic_vbus(di->local_mode, CHARGE_IC_AUX, &vbus);
	return scnprintf(buf, size, "%d\n", vbus);
}

static ssize_t dc_dbg_main_vout_show(void *dev_data, char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;
	int vout = 0;

	if (!di)
		return -EINVAL;

	(void)dcm_get_ic_vout(di->local_mode, CHARGE_IC_MAIN, &vout);
	return scnprintf(buf, size, "%d\n", vout);
}

static ssize_t dc_dbg_aux_vout_show(void *dev_data, char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;
	int vout = 0;

	if (!di)
		return -EINVAL;

	(void)dcm_get_ic_vout(di->local_mode, CHARGE_IC_AUX, &vout);
	return scnprintf(buf, size, "%d\n", vout);
}

static ssize_t dc_dbg_main_ibat_show(void *dev_data, char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;
	int ibat = 0;

	if (!di)
		return -EINVAL;

	/* DBC sc bat current calibration use, ibat need be calibrated */
	(void)dcm_get_ic_ibat(di->local_mode, CHARGE_IC_MAIN, &ibat);
	return scnprintf(buf, size, "%d\n", ibat);
}

static ssize_t dc_dbg_aux_ibat_show(void *dev_data, char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;
	int ibat = 0;

	if (!di)
		return -EINVAL;

	/* DBC sc bat current calibration use, ibat need be calibrated */
	(void)dcm_get_ic_ibat(di->local_mode, CHARGE_IC_AUX, &ibat);
	return scnprintf(buf, size, "%d\n", ibat);
}

static ssize_t dc_dbg_main_vbat_show(void *dev_data, char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;
	int vbat;

	if (!di)
		return -EINVAL;

	vbat = dcm_get_ic_vbtb(di->local_mode, CHARGE_IC_MAIN);
	return scnprintf(buf, size, "%d\n", vbat);
}

static ssize_t dc_dbg_aux_vbat_show(void *dev_data, char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;
	int vbat;

	if (!di)
		return -EINVAL;

	vbat = dcm_get_ic_vbtb(di->local_mode, CHARGE_IC_AUX);
	return scnprintf(buf, size, "%d\n", vbat);
}

static ssize_t dc_dbg_main_ic_temp_show(void *dev_data, char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;
	int ic_temp = 0;

	if (!di)
		return -EINVAL;

	(void)dcm_get_ic_temp(di->local_mode, CHARGE_IC_MAIN, &ic_temp);
	return scnprintf(buf, size, "%d\n", ic_temp);
}

static ssize_t dc_dbg_aux_ic_temp_show(void *dev_data, char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;
	int ic_temp = 0;

	if (!di)
		return -EINVAL;

	(void)dcm_get_ic_temp(di->local_mode, CHARGE_IC_AUX, &ic_temp);
	return scnprintf(buf, size, "%d\n", ic_temp);
}

static ssize_t dc_dbg_main_ic_id_show(void *dev_data, char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;
	int ic_id;

	if (!di)
		return -EINVAL;

	ic_id = dcm_get_ic_id(di->local_mode, CHARGE_IC_MAIN);
	return scnprintf(buf, size, "%d\n", ic_id);
}

static ssize_t dc_dbg_aux_ic_id_show(void *dev_data, char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;
	int ic_id;

	if (!di)
		return -EINVAL;

	ic_id = dcm_get_ic_id(di->local_mode, CHARGE_IC_AUX);
	return scnprintf(buf, size, "%d\n", ic_id);
}

static ssize_t dc_dbg_main_ic_name_show(void *dev_data, char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;
	const char *ic_name = NULL;

	if (!di)
		return -EINVAL;

	ic_name = dcm_get_ic_name(di->local_mode, CHARGE_IC_MAIN);
	return scnprintf(buf, PAGE_SIZE, "%s\n", ic_name);
}

static ssize_t dc_dbg_aux_ic_name_show(void *dev_data, char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;
	const char *ic_name = NULL;

	if (!di)
		return -EINVAL;

	ic_name = dcm_get_ic_name(di->local_mode, CHARGE_IC_AUX);
	return scnprintf(buf, PAGE_SIZE, "%s\n", ic_name);
}

static ssize_t dc_dbg_aux_tbat_show(void *dev_data, char *buf, size_t size)
{
	int aux_tbat = power_temp_get_raw_value(POWER_TEMP_BTB_NTC_AUX);

	return scnprintf(buf, PAGE_SIZE, "%d\n", aux_tbat);
}

static ssize_t dc_dbg_tadapt_show(void *dev_data, char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;

	if (!di)
		return -EINVAL;

	return scnprintf(buf, PAGE_SIZE, "%d\n", di->tadapt);
}

static ssize_t dc_dbg_vadapt_show(void *dev_data, char *buf, size_t size)
{
	struct direct_charge_device *di = dev_data;

	if (!di)
		return -EINVAL;

	return scnprintf(buf, PAGE_SIZE, "%d\n", di->vadapt);
}

static ssize_t dc_dbg_tusb_show(void *dev_data, char *buf, size_t size)
{
	int tusb = power_temp_get_average_value(POWER_TEMP_USB_PORT) / POWER_TEMP_UNIT;

	return scnprintf(buf, PAGE_SIZE, "%d\n", tusb);
}

void sc_dbg_register(void *dev_data)
{
	power_dbg_ops_register("sc_time_para", dev_data,
		(power_dbg_show)dc_dbg_time_para_show,
		(power_dbg_store)dc_dbg_time_para_store);
	power_dbg_ops_register("sc_vstep_para", dev_data,
		(power_dbg_show)dc_dbg_vstep_para_show,
		(power_dbg_store)dc_dbg_vstep_para_store);
	power_dbg_ops_register("total_ibus", dev_data,
		(power_dbg_show)dc_dbg_total_ibus_show, NULL);
	power_dbg_ops_register("main_ibus", dev_data,
		(power_dbg_show)dc_dbg_main_ibus_show, NULL);
	power_dbg_ops_register("main_vbus", dev_data,
		(power_dbg_show)dc_dbg_main_vbus_show, NULL);
	power_dbg_ops_register("main_vout", dev_data,
		(power_dbg_show)dc_dbg_main_vout_show, NULL);
	power_dbg_ops_register("main_ibat", dev_data,
		(power_dbg_show)dc_dbg_main_ibat_show, NULL);
	power_dbg_ops_register("main_vbat", dev_data,
		(power_dbg_show)dc_dbg_main_vbat_show, NULL);
	power_dbg_ops_register("main_ic_temp", dev_data,
		(power_dbg_show)dc_dbg_main_ic_temp_show, NULL);
	power_dbg_ops_register("main_ic_id", dev_data,
		(power_dbg_show)dc_dbg_main_ic_id_show, NULL);
	power_dbg_ops_register("main_ic_name", dev_data,
		(power_dbg_show)dc_dbg_main_ic_name_show, NULL);
	power_dbg_ops_register("aux_ibus", dev_data,
		(power_dbg_show)dc_dbg_aux_ibus_show, NULL);
	power_dbg_ops_register("aux_vbus", dev_data,
		(power_dbg_show)dc_dbg_aux_vbus_show, NULL);
	power_dbg_ops_register("aux_vout", dev_data,
		(power_dbg_show)dc_dbg_aux_vout_show, NULL);
	power_dbg_ops_register("aux_ibat", dev_data,
		(power_dbg_show)dc_dbg_aux_ibat_show, NULL);
	power_dbg_ops_register("aux_vbat", dev_data,
		(power_dbg_show)dc_dbg_aux_vbat_show, NULL);
	power_dbg_ops_register("aux_ic_temp", dev_data,
		(power_dbg_show)dc_dbg_aux_ic_temp_show, NULL);
	power_dbg_ops_register("aux_ic_id", dev_data,
		(power_dbg_show)dc_dbg_aux_ic_id_show, NULL);
	power_dbg_ops_register("aux_ic_name", dev_data,
		(power_dbg_show)dc_dbg_aux_ic_name_show, NULL);
	power_dbg_ops_register("aux_tbat", dev_data,
		(power_dbg_show)dc_dbg_aux_tbat_show, NULL);
	power_dbg_ops_register("tadapt", dev_data,
		(power_dbg_show)dc_dbg_tadapt_show, NULL);
	power_dbg_ops_register("vadapt", dev_data,
		(power_dbg_show)dc_dbg_vadapt_show, NULL);
	power_dbg_ops_register("tusb", dev_data,
		(power_dbg_show)dc_dbg_tusb_show, NULL);
}

void lvc_dbg_register(void *dev_data)
{
	power_dbg_ops_register("lvc_time_para", dev_data,
		(power_dbg_show)dc_dbg_time_para_show,
		(power_dbg_store)dc_dbg_time_para_store);
	power_dbg_ops_register("lvc_ibus", dev_data,
		(power_dbg_show)dc_dbg_main_ibus_show, NULL);
	power_dbg_ops_register("lvc_vbus", dev_data,
		(power_dbg_show)dc_dbg_main_vbus_show, NULL);
	power_dbg_ops_register("lvc_ibat", dev_data,
		(power_dbg_show)dc_dbg_main_ibat_show, NULL);
	power_dbg_ops_register("lvc_vbat", dev_data,
		(power_dbg_show)dc_dbg_main_vbat_show, NULL);
	power_dbg_ops_register("lvc_ic_id", dev_data,
		(power_dbg_show)dc_dbg_main_ic_id_show, NULL);
	power_dbg_ops_register("lvc_ic_name", dev_data,
		(power_dbg_show)dc_dbg_main_ic_name_show, NULL);
	power_dbg_ops_register("lvc_vadapt", dev_data,
		(power_dbg_show)dc_dbg_vadapt_show, NULL);
}
