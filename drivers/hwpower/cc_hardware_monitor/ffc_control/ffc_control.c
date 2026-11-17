/*
 * ffc_control.c
 *
 * ffc control driver
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

#include <chipset_common/hwpower/hardware_monitor/ffc_control.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/battery/battery_temp.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG ffc_control
HWLOG_REGIST();

static struct ffc_ctrl_dev *g_ffc_ctrl_dev;

static struct ffc_ctrl_dev *get_ffc_ctrl_dev(void)
{
	if (!g_ffc_ctrl_dev) {
		hwlog_err("g_ffc_ctrl_dev is null\n");
		return NULL;
	}

	return g_ffc_ctrl_dev;
}

int ffc_get_buck_vterm(void)
{
	int tbat = 0;
	int i;
	int ichg_avg = power_platform_get_battery_current_avg();
	struct ffc_ctrl_dev *di = get_ffc_ctrl_dev();

	if (!di || !di->buck_term_para_flag)
		return 0;

	bat_temp_get_temperature(BAT_TEMP_MIXED, &tbat);
	for (i = 0; i < FFC_MAX_CHARGE_TERM; i++) {
		if ((tbat < di->buck_term_para[i].temp_low) ||
			(tbat > di->buck_term_para[i].temp_high))
			continue;
		if (ichg_avg > di->buck_term_para[i].ichg_thre) {
			hwlog_info("buck set vterm increase %d\n", di->buck_term_para[i].vterm_gain);
			return di->buck_term_para[i].vterm_gain;
		}
	}
	return 0;
}

int ffc_get_buck_iterm(void)
{
	int tbat = 0;
	int i;
	struct ffc_ctrl_dev *di = get_ffc_ctrl_dev();

	if (!di || !di->buck_term_para_flag)
		return 0;

	bat_temp_get_temperature(BAT_TEMP_MIXED, &tbat);
	for (i = 0; i < FFC_MAX_CHARGE_TERM; i++) {
		if ((tbat < di->buck_term_para[i].temp_low) ||
			(tbat > di->buck_term_para[i].temp_high))
			continue;
		hwlog_info("buck set iterm %d\n", di->buck_term_para[i].iterm);
		return di->buck_term_para[i].iterm;
	}
	return 0;
}

struct ffc_dc_vbat_para *ffc_get_dc_vbat_para(void)
{
	struct ffc_ctrl_dev *di = get_ffc_ctrl_dev();

	if (!di || !di->dc_vbat_para_flag)
		return NULL;

	return &di->dc_vbat_para;
}

static void ffc_parse_dc_vbat_para_dts(struct device_node *np,
	struct ffc_ctrl_dev *di)
{
	int len;
	int idata[FFC_DC_VBAT_TOTAL] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"dc_vbat_para", idata, 1, FFC_DC_VBAT_TOTAL);
	if (len < 0)
		return;

	di->dc_vbat_para.vbat_main_th = idata[FFC_DC_VBAT_MAIN_TH];
	di->dc_vbat_para.vbat_aux_th = idata[FFC_DC_VBAT_AUX_TH];
	di->dc_vbat_para_flag = true;
	hwlog_info("dc_vbat_para: vbat_main_th=%d, vbat_aux_th=%d\n",
		di->dc_vbat_para.vbat_main_th,
		di->dc_vbat_para.vbat_aux_th);
}

static void ffc_parse_buck_term_para_dts(struct device_node *np, struct ffc_ctrl_dev *di)
{
	int len;
	int row;
	int col;
	int idata[FFC_MAX_CHARGE_TERM * FFC_BUCK_TERM_TOTAL] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"buck_term_para", idata, FFC_MAX_CHARGE_TERM, FFC_BUCK_TERM_TOTAL);
	if (len < 0)
		return;

	for (row = 0; row < len / FFC_BUCK_TERM_TOTAL; row++) {
		col = row * FFC_BUCK_TERM_TOTAL + FFC_BUCK_TEMP_LOW;
		di->buck_term_para[row].temp_low = idata[col];
		col = row * FFC_BUCK_TERM_TOTAL + FFC_BUCK_TEMP_HIGH;
		di->buck_term_para[row].temp_high = idata[col];
		col = row * FFC_BUCK_TERM_TOTAL + FFC_BUCK_VTERM_GAIN;
		di->buck_term_para[row].vterm_gain = idata[col];
		col = row * FFC_BUCK_TERM_TOTAL + FFC_BUCK_ICHG_THRE;
		di->buck_term_para[row].ichg_thre = idata[col];
		col = row * FFC_BUCK_TERM_TOTAL + FFC_BUCK_ITERM;
		di->buck_term_para[row].iterm = idata[col];
	}
	di->buck_term_para_flag = true;

	for (row = 0; row < len / FFC_BUCK_TERM_TOTAL; row++)
		hwlog_info("buck_term_para[%d]: %d %d %d %d %d\n", row,
		di->buck_term_para[row].temp_low,
		di->buck_term_para[row].temp_high,
		di->buck_term_para[row].vterm_gain,
		di->buck_term_para[row].ichg_thre,
		di->buck_term_para[row].iterm);
}

static void ffc_ctrl_parse_dts(struct device_node *np, struct ffc_ctrl_dev *l_dev)
{
	ffc_parse_buck_term_para_dts(np, l_dev);
	ffc_parse_dc_vbat_para_dts(np, l_dev);
}

static int ffc_ctrl_probe(struct platform_device *pdev)
{
	struct ffc_ctrl_dev *l_dev = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_ffc_ctrl_dev = l_dev;
	np = pdev->dev.of_node;

	ffc_ctrl_parse_dts(np, l_dev);
	platform_set_drvdata(pdev, l_dev);

	return 0;
}

static int ffc_ctrl_remove(struct platform_device *pdev)
{
	struct ffc_ctrl_dev *l_dev = platform_get_drvdata(pdev);

	if (!l_dev)
		return -ENODEV;

	platform_set_drvdata(pdev, NULL);
	kfree(l_dev);
	g_ffc_ctrl_dev = NULL;

	return 0;
}

static const struct of_device_id ffc_ctrl_match_table[] = {
	{
		.compatible = "huawei,ffc_control",
		.data = NULL,
	},
	{},
};

static struct platform_driver ffc_ctrl_driver = {
	.probe = ffc_ctrl_probe,
	.remove = ffc_ctrl_remove,
	.driver = {
		.name = "huawei,ffc_control",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(ffc_ctrl_match_table),
	},
};

static int __init ffc_ctrl_init(void)
{
	return platform_driver_register(&ffc_ctrl_driver);
}

static void __exit ffc_ctrl_exit(void)
{
	platform_driver_unregister(&ffc_ctrl_driver);
}

device_initcall_sync(ffc_ctrl_init);
module_exit(ffc_ctrl_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ffc control driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
