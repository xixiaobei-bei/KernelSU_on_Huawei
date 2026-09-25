/*
 * battery_temp_fitting.c
 *
 * battery temperature fitting driver
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/thermal.h>
#include <linux/list.h>
#include <linux/memory.h>
#include <securec.h>
#include <dsm/dsm_pub.h>

#include <linux/power/hisi/hisi_battery_data.h>
#ifdef CONFIG_HISI_COUL
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#endif

#include <huawei_platform/power/power_interface.h>
#include <huawei_platform/power/power_dsm.h>
#include <huawei_platform/power/power_dts.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/battery_temp_fitting.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG batt_temp_fit
HWLOG_REGIST();

struct btf_dev *g_btf_dev;

/* tz group listed in descending preference */
struct btf_tz_group_lkup_tbl g_tz_grp_lkup_tbl[BTF_TZ_GROUP_END] = {
	{BTF_TZ_GROUP_USB_CHARGER, "usb_port", "charger"},
	{BTF_TZ_GROUP_CHARGER_RFBOARD, "charger", "rfboard"},
	{BTF_TZ_GROUP_USB_RFBOARD, "usb_port", "rfboard"},
};

static const char * const tz_group_name_lkup_tbl[BTF_TZ_GROUP_END] = {
	[BTF_TZ_GROUP_USB_CHARGER] = "usbport_charger",
	[BTF_TZ_GROUP_CHARGER_RFBOARD] = "charger_rfboard",
	[BTF_TZ_GROUP_USB_RFBOARD] = "usbport_rfboard",
};

static struct btf_dev *btf_get_dev(void)
{
	if (!g_btf_dev) {
		hwlog_err("g_btf_dev is null");
		return NULL;
	}

	return g_btf_dev;
}

bool btf_temp_fitted_eqr(void)
{
	struct btf_dev *di = btf_get_dev();

	if (!di)
		return false;

	return di->bat_temp_fitted;
}

struct btf_charge_temp_data *btf_charge_temp_data_eqr(void)
{
	struct btf_dev *di = btf_get_dev();

	if (!di)
		return NULL;

	if (!di->ready) {
		hwlog_info("btf module unready, no chg_temp_data available\n");
		return NULL;
	}

	if (!btf_temp_fitted_eqr())
		return NULL;

	return di->temp_para;
}

static void btf_temp_para_update(struct btf_dev *di)
{
	int i;
	struct chrg_para_lut *batt_param = hisi_battery_charge_params();

	if (!batt_param) {
		hwlog_err("get battery charging params fail!\n");
		return;
	}

	/* value -1 means "not designated" */
	for (i = 0; i < (batt_param->temp_len) / TEMP_PARA_TOTAL; i++) {
		if (di->temp_para[i].iin_temp == -1)
			di->temp_para[i].iin_temp = batt_param->temp_data[i][TEMP_PARA_IIN];
		if (di->temp_para[i].ichg_temp == -1)
			di->temp_para[i].ichg_temp = batt_param->temp_data[i][TEMP_PARA_ICHG];
		if (di->temp_para[i].vterm_temp == -1)
			di->temp_para[i].vterm_temp = batt_param->temp_data[i][TEMP_PARA_VTERM];
		if (di->temp_para[i].temp_back == -1)
			di->temp_para[i].temp_back = batt_param->temp_data[i][TEMP_PARA_TEMP_BACK];
	}
}

static void btf_check_ready(struct work_struct *work)
{
	struct btf_dev *di = btf_get_dev();

	if (!di)
		return;

	di->ready = true;
	btf_temp_para_update(di);
	hwlog_info("battery temp fitting ready\n");
}

static int _btf_dsm_report(struct btf_dev *di, int temp_adc, int temp_fitted)
{
	int ret;
	char dsm_buf[BTF_DSM_BUFF_SIZE_MAX] = { 0 };

	/* key info */
	ret = snprintf_s(dsm_buf, BTF_DSM_BUFF_SIZE_MAX, BTF_DSM_BUFF_SIZE_MAX - 1,
		"[BTF_SENSOR_DETACH]temp_batt_adc:%d; temp_batt_fit:%d; "
		"fit_case:%s\n", temp_adc, temp_fitted,
		tz_group_name_lkup_tbl[di->tz_group]);
	if (ret)
		return ret;

	/* report */
	return power_dsm_dmd_report(POWER_DSM_BATTERY, ERROR_BATT_TEMP_LOW,
		dsm_buf);
}

static void btf_dsm_report(struct btf_dev *di, int temp_adc, int temp_fitted)
{
	static int temp_fitted_times;

	if (!di->bat_temp_fitted)
		temp_fitted_times = 0;

	if (!di->dmd_reported) {
		temp_fitted_times++;
		if (temp_fitted_times > BTF_DMD_REPORT_FIT_TIME_MAX) {
			if (_btf_dsm_report(di, temp_adc, temp_fitted)) {
				hwlog_err("dsm reported fail\n");
				return;
			}
			hwlog_info("dsm reported\n");
			temp_fitted_times = 0;
			di->dmd_reported = true;
			return;
		}
	}
}

static int btf_get_tz_temp(const char *tz_name, int *tz_temp_i)
{
	int ret;
	int temp;
	struct thermal_zone_device *tz = NULL;

	tz = thermal_zone_get_zone_by_name(tz_name);
	if (IS_ERR(tz)) {
		hwlog_err("get %s tz fail\n", tz_name);
		return -EINVAL;
	}

	/* do temp sampling */
	ret = thermal_zone_get_temp(tz, &temp);
	if (ret)
		return -EINVAL;

	/* convert 0.0001 degree to 1 degree */
	*tz_temp_i = temp / 1000;
	return 0;
}

static bool btf_fitting_necessity_check(struct btf_tz_temp *tz_data,
	int temp_meas)
{
	int i;
	bool ret = true;
	struct btf_dev *di = btf_get_dev();

	if (!di)
		return false;

	for (i = 0; i < di->linear_fit_para.coef_num - 1; i++)
		ret = ((tz_data[i].temp - temp_meas) > BTF_DO_LINE_FIT_TEMP_GAP) && ret;

	return ret;
}

static int btf_tz_temp_lkup(struct btf_tz_temp *tz_data, const char *tz_name,
	int tz_num_max)
{
	int i;

	for (i = 0; i < tz_num_max; i++) {
		if (!strcmp(tz_name, tz_data[i].name))
			return tz_data[i].temp;
	}

	hwlog_err("no temp data for %s\n", tz_name);
	return -EINVAL;
}

static enum btf_tz_group btf_sensor_group_select(struct btf_tz_temp *tz_data,
	int tz_num_max)
{
	int i;
	int temp1, temp2;

	for (i = 0; i < ARRAY_SIZE(g_tz_grp_lkup_tbl); i++) {
		temp1 = btf_tz_temp_lkup(tz_data, g_tz_grp_lkup_tbl[i].tz_name1,
			tz_num_max);
		temp2 = btf_tz_temp_lkup(tz_data, g_tz_grp_lkup_tbl[i].tz_name2,
			tz_num_max);
		if (temp1 < 0 || temp2 < 0)
			return BTF_TZ_GROUP_ILLEGAL;

		if (abs(temp1 - temp2) < BTF_TZ_GROUP_SELECT_TH)
			return g_tz_grp_lkup_tbl[i].tz_group;
	}

	return BTF_TZ_GROUP_ILLEGAL;
}

static int btf_get_group_tz_temp(struct btf_dev *di,
	struct btf_tz_temp *tz_data, int tz_group_max)
{
	int i;
	int ret;

	for (i = 0; i < di->linear_fit_para.coef_num; i++) {
		tz_data[i].name = di->linear_fit_para.tz_name[i];
		if (!strcmp(di->linear_fit_para.tz_name[i], "NULL")) {
			tz_data[i].temp = 1; /* constant item in linear polynomial */
			continue;
		}
		ret = btf_get_tz_temp(di->linear_fit_para.tz_name[i], &tz_data[i].temp);
		if (ret) {
			hwlog_err("tz %s eqr fail\n", di->linear_fit_para.tz_name[i]);
			return ret;
		}
	}

	return 0;
}

static int btf_do_linear_fitting(int temp_meas, int *temp_fit)
{
	int i;
	int ret;
	int row;
	struct btf_tz_temp tz_data[BTF_MAX_LINEAR_COEF_NUM];
	struct btf_dev *di = btf_get_dev();

	if (!di)
		return -ENODEV;

	di->tz_group = BTF_TZ_GROUP_ILLEGAL;
	memset_s(tz_data, sizeof(tz_data), 0, sizeof(tz_data));

	/* get temp data of given sensors */
	ret = btf_get_group_tz_temp(di, tz_data, BTF_MAX_LINEAR_COEF_NUM);
	if (ret) {
		if (!di->ready && di->fake_btf_by_unready) {
			hwlog_info("btf unready, return fake val\n");
			*temp_fit = di->fake_btf_val;
			return 0;
		}

		return -EINVAL;
	}

	/* check the necessity of fitting */
	ret = btf_fitting_necessity_check(tz_data, temp_meas);
	if (!ret) {
		hwlog_info("fit not necessary\n");
		return -EPERM;
	}

	/* select sensor group for fitting */
	di->tz_group = btf_sensor_group_select(tz_data,
		di->linear_fit_para.coef_num - 1);
	if (di->tz_group == BTF_TZ_GROUP_ILLEGAL) {
		hwlog_err("illegal tz_group\n");
		return -EPERM;
	}

	/* do fitting */
	row = di->tz_group;
	*temp_fit = 0;
	for (i = 0; i < di->linear_fit_para.coef_num; i++)
		*temp_fit += (di->linear_fit_para.coef[row][i] * tz_data[i].temp);
	*temp_fit = *temp_fit / BTF_RESOULUTION;

	return 0;
}

static void btf_validity_check_for_bt_measured(int *temp)
{
	int ret;
	int temp_adc;
	int temp_fitted;
	struct btf_dev *di = btf_get_dev();

	if (!di)
		return;

	if (*temp < BTF_TEMP_LTH) {
		switch (di->fitting_mode) {
		case BTF_MODE_LINEAR:
			ret = btf_do_linear_fitting(*temp, &temp_fitted);
			break;
		default:
			hwlog_err("illegal fit mode\n");
			di->bat_temp_fitted = false;
			return;
		}
		if (ret) {
			di->bat_temp_fitted = false;
			btf_dsm_report(di, temp_adc, temp_fitted);
			return;
		}

		temp_adc = *temp;
		*temp = temp_fitted;
		di->bat_temp_fitted = true;
		btf_dsm_report(di, temp_adc, temp_fitted);
		return;
	}
	di->bat_temp_fitted = false;
}

static int btf_apply_charge_limitation(struct btf_dev *di, int temp_fit)
{
	int ret;
	int lvc_i_limit_now;
	int sc_i_limit_now;
	int wldc_pwr_limit_now;

	/* read limitation now */
	ret = power_if_kernel_sysfs_get(POWER_IF_OP_TYPE_LVC,
			POWER_IF_SYSFS_VBUS_IIN_LIMIT, &lvc_i_limit_now);
	if (ret)
		return ret;
	ret = power_if_kernel_sysfs_get(POWER_IF_OP_TYPE_SC,
			POWER_IF_SYSFS_VBUS_IIN_LIMIT, &sc_i_limit_now);
	if (ret)
		return ret;
	ret = power_if_kernel_sysfs_get(POWER_IF_OP_TYPE_WL_SC,
			POWER_IF_SYSFS_VBUS_PWR_LIMIT, &wldc_pwr_limit_now);
	if (ret)
		return ret;

	/* set limitation on DC/WLDC/DCP current and wldc power */
	if (lvc_i_limit_now > di->dc_i_limit)
		ret = power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_LVC,
			POWER_IF_SYSFS_VBUS_IIN_LIMIT, di->dc_i_limit);
	if (ret)
		return ret;
	if (sc_i_limit_now > di->dc_i_limit)
		ret = power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_SC,
			POWER_IF_SYSFS_VBUS_IIN_LIMIT, di->dc_i_limit);
	if (ret)
		return ret;
	if (wldc_pwr_limit_now > di->wldc_pwr_limit)
		ret = power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_WL_SC,
			POWER_IF_SYSFS_VBUS_PWR_LIMIT, di->wldc_pwr_limit);

	return ret;
}

static int btf_release_charge_limitation(struct btf_dev *di, int temp_fit)
{
	int ret;
	int lvc_i_limit_now;
	int sc_i_limit_now;
	int wldc_pwr_limit_now;

	/* read limitation now */
	ret = power_if_kernel_sysfs_get(POWER_IF_OP_TYPE_LVC,
			POWER_IF_SYSFS_VBUS_IIN_LIMIT, &lvc_i_limit_now);
	if (ret)
		return ret;
	ret = power_if_kernel_sysfs_get(POWER_IF_OP_TYPE_SC,
			POWER_IF_SYSFS_VBUS_IIN_LIMIT, &sc_i_limit_now);
	if (ret)
		return ret;
	ret = power_if_kernel_sysfs_get(POWER_IF_OP_TYPE_WL_SC,
			POWER_IF_SYSFS_VBUS_PWR_LIMIT, &wldc_pwr_limit_now);
	if (ret)
		return ret;

	/* release limitation on DC/WLDC/DCP current and wldc power */
	if (lvc_i_limit_now == di->dc_i_limit)
		ret = power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_LVC,
				POWER_IF_SYSFS_VBUS_IIN_LIMIT, BTF_IMPOSSIBE_IBAT);
	if (ret)
		return ret;
	if (sc_i_limit_now == di->dc_i_limit)
		ret = power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_SC,
				POWER_IF_SYSFS_VBUS_IIN_LIMIT, BTF_IMPOSSIBE_IBAT);
	if (ret)
		return ret;
	if (wldc_pwr_limit_now == di->wldc_pwr_limit)
		ret = power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_WL_SC,
				POWER_IF_SYSFS_VBUS_PWR_LIMIT, BTF_IMPOSSIBLE_CHG_PWR);

	return ret;
}

int btf_try_fit(int temp_in)
{
	int ret;
	int temp_in_degree = temp_in / MDEGREE_P_DEGREE;
	struct btf_dev *di = btf_get_dev();

	if (!di)
		return temp_in;

	btf_validity_check_for_bt_measured(&temp_in_degree);

	/* before module ready, return fitting temp. no limit operation */
	if (!di->ready)
		return temp_in_degree * MDEGREE_P_DEGREE;

	if (di->bat_temp_fitted) {
		hwlog_info("temp_org = %d; temp_fit = %d\n",
			temp_in, temp_in_degree * MDEGREE_P_DEGREE);
		ret = btf_apply_charge_limitation(di, temp_in_degree);
		if (ret)
			return temp_in;

		return temp_in_degree * MDEGREE_P_DEGREE;
	}

	ret = btf_release_charge_limitation(di, temp_in_degree);
	if (ret)
		hwlog_err("battery_temp_fitting limitation release fail\n");

	return temp_in;
}

static int btf_charging_temp_para_parse(struct device_node *np,
	struct btf_dev *di)
{
	int i;
	int ret;
	int r, c;
	int array_len;
	long idata_dec;
	const char *idata = NULL;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"temp_para", TEMP_PARA_LEVEL, TEMP_PARA_TOTAL);
	if (array_len < 0)
		return -EINVAL;

	for (i = 0; i < array_len; i++) {
		r = i / TEMP_PARA_TOTAL;
		c = i % TEMP_PARA_TOTAL;
		ret = power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np,
			"temp_para", i, &idata);
		if (ret)
			return ret;

		ret = kstrtol(idata, DECIMAL_BASE, &idata_dec);
		if (ret) {
			hwlog_err("temp_para kstrtol opreration fail\n");
			return -EINVAL;
		}

		switch (c) {
		case TEMP_PARA_TEMP_MIN:
			di->temp_para[r].temp_min = idata_dec;
			break;
		case TEMP_PARA_TEMP_MAX:
			di->temp_para[r].temp_max = idata_dec;
			break;
		case TEMP_PARA_IIN:
			di->temp_para[r].iin_temp = idata_dec;
			break;
		case TEMP_PARA_ICHG:
			di->temp_para[r].ichg_temp = idata_dec;
			break;
		case TEMP_PARA_VTERM:
			di->temp_para[r].vterm_temp = idata_dec;
			break;
		case TEMP_PARA_TEMP_BACK:
			di->temp_para[r].temp_back = idata_dec;
			break;
		default:
			hwlog_err("invalid temp param item\n");
			break;
		}
	}

	return 0;
}

static void btf_dc_cur_limit_parse(struct device_node *np,
	struct btf_dev *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"dc_i_limit", &di->dc_i_limit, BTF_IMPOSSIBE_IBAT);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"wldc_pwr_limit", &di->wldc_pwr_limit, BTF_IMPOSSIBLE_CHG_PWR);
}

static int btf_exception_handler_dts_parse(struct device_node *np,
	struct btf_dev *di)
{
	if (btf_charging_temp_para_parse(np, di))
		return -EINVAL;

	btf_dc_cur_limit_parse(np, di);

	return 0;
}

static int btf_linear_para_parse(struct device_node *np,
	struct btf_dev *di)
{
	int i;
	int ret;
	int r, c;
	int array_len;
	long idata_dec;
	const char *idata = NULL;

	ret = power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"linear_coef_num", &di->linear_fit_para.coef_num, 0);
	if (ret || (di->linear_fit_para.coef_num > BTF_MAX_LINEAR_COEF_NUM))
		return -EINVAL;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"linear_fit_para", BTF_MAX_LINEAR_FIT_CASE + 1,
		di->linear_fit_para.coef_num);
	if (array_len < 0)
		return -EINVAL;

	di->linear_fit_para.tz_group_num =
		array_len / di->linear_fit_para.coef_num - 1;

	/* parse sensor name */
	for (i = 0; i < di->linear_fit_para.coef_num; i++) {
		ret = power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np,
			"linear_fit_para", i, &idata);
		if (ret)
			return ret;
		di->linear_fit_para.tz_name[i] = idata;
	}

	/* parse linear fitting coefficients */
	for (i = di->linear_fit_para.coef_num; i < array_len; i++) {
		r = i / di->linear_fit_para.coef_num - 1;
		c = i % di->linear_fit_para.coef_num;
		ret = power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np,
			"linear_fit_para", i, &idata);
		if (ret)
			return ret;

		ret = kstrtol(idata, DECIMAL_BASE, &idata_dec);
		if (ret) {
			hwlog_err("linear fitting para kstrtol fail\n");
			return -EINVAL;
		}

		di->linear_fit_para.coef[r][c] = idata_dec;
	}

	return 0;
}

static int btf_parse_dts(struct device_node *np)
{
	int ret;
	struct btf_dev *di = btf_get_dev();

	if (!di)
		return -ENODEV;

	/* waiting time for all module ready */
	power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"t_wait_for_ready", &di->t_wait_for_ready, BTF_WAIT_BTF_READY);

	/* return fake fitted temp when batt_temp_fit unready at booting */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"fake_btf_by_unready", &di->fake_btf_by_unready, 0);

	if (di->fake_btf_by_unready) {
		ret = power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
			"fake_btf_val", &di->fake_btf_val, 0);
		if (ret)
			return ret;
	}

	/* batt_temp_fitting mode: 1->linear_fitting */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"fitting_mode", &di->fitting_mode, BTF_MODE_LINEAR);

	switch (di->fitting_mode) {
	case BTF_MODE_LINEAR:
		ret = btf_linear_para_parse(np, di);
		if (ret)
			return ret;
		break;
	default:
		hwlog_err("unsupported batt temp fit mode\n");
		return -EINVAL;
	}

	/* exception handler parameter parse */
	ret = btf_exception_handler_dts_parse(np, di);
	if (ret)
		return ret;

	return 0;
}

static int btf_probe(struct platform_device *pdev)
{
	int ret;
	struct btf_dev *di = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;
	g_btf_dev = di;
	np = pdev->dev.of_node;

	ret = btf_parse_dts(np);
	if (ret)
		goto fail_free_mem;

	INIT_DELAYED_WORK(&di->check_ready, btf_check_ready);
	schedule_delayed_work(&di->check_ready,
		msecs_to_jiffies(di->t_wait_for_ready));

	return 0;

fail_free_mem:
	kfree(di);
	g_btf_dev = NULL;
	return ret;
}

static int btf_remove(struct platform_device *pdev)
{
	struct btf_dev *di = btf_get_dev();

	if (!di)
		return -ENODEV;

	kfree(di);
	g_btf_dev = NULL;

	return 0;
}

static const struct of_device_id btf_match_table[] = {
	{
		.compatible = "huawei,battery_temp_fitting",
		.data = NULL,
	},
	{},
};

static struct platform_driver btf_driver = {
	.probe = btf_probe,
	.remove = btf_remove,
	.driver = {
		.name = "huawei,battery_temp_fitting",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(btf_match_table),
	},
};

static int __init btf_init(void)
{
	return platform_driver_register(&btf_driver);
}

static void __exit btf_exit(void)
{
	platform_driver_unregister(&btf_driver);
}

subsys_initcall_sync(btf_init);
module_exit(btf_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("driver for battery temperature fitting");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
