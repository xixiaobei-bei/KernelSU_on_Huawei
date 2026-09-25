/*
 * vbat_hkadc.c
 *
 * get battery voltage by hkadc
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#include <linux/module.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/hisi/hisi_adc.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/battery_voltage.h>
#include <huawei_platform/power/huawei_charger.h>
#ifdef CONFIG_HISI_COUL
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#endif
#include "vbat_hkadc.h"

#define HWLOG_TAG vbat_hkadc
HWLOG_REGIST();

static struct vbat_hkadc_info *g_vbat_hkadc_di;
static long g_v_offset_a = V_OFFSET_A_DEFAULT;
static long g_v_offset_b = V_OFFSET_B_DEFAULT;
static long g_v_offset_c = V_OFFSET_C_DEFAULT;

static int __init early_parse_vbat_hkadc_cmdline(char *p)
{
	char *token = NULL;

	if (!p) {
		hwlog_err("cmdline is null\n");
		return -1;
	}

	hwlog_info("vbat_hkadc=%s\n", p);

	token = strsep(&p, ",");
	if (token) {
		/* 10: decimal base */
		if (kstrtoul(token, 10, &g_v_offset_a) != 0)
			return -1;
	}

	token = strsep(&p, ",");
	if (token) {
		/* 10: decimal base */
		if (kstrtol(token, 10, &g_v_offset_b) != 0)
			return -1;
	}

	token = strsep(&p, ",");
	if (token) {
		/* 10: decimal base */
		if (kstrtol(token, 10, &g_v_offset_c) != 0)
			return -1;
	}

	if (g_v_offset_a < V_OFFSET_A_MIN || g_v_offset_a > V_OFFSET_A_MAX) {
		g_v_offset_a = V_OFFSET_A_DEFAULT;
		hwlog_err("v_offset_a invalid\n");
	}

	if (g_v_offset_b < V_OFFSET_B_MIN || g_v_offset_b > V_OFFSET_B_MAX) {
		g_v_offset_b = V_OFFSET_B_DEFAULT;
		hwlog_err("v_offset_b invalid\n");
	}

	if (g_v_offset_c < V_OFFSET_C_MIN || g_v_offset_c > V_OFFSET_C_MAX) {
		g_v_offset_c = V_OFFSET_C_DEFAULT;
		hwlog_err("v_offset_c invalid\n");
	}

	hwlog_info("parse cmdline: v_offset_a=%ld,v_offset_b=%ld,v_offset_c=%ld\n",
		g_v_offset_a, g_v_offset_b, g_v_offset_c);
	return 0;
}

early_param("vbat_hkadc", early_parse_vbat_hkadc_cmdline);

/*
 * Vout = Vbat_up * k1 + Vbat_down * k2 + offset
 *
 * k1 = R1/R4 * ((R4+R3)/(R1+R2))
 * k2 = (R1*(R4+R3)) / (R4*(R1+R2)) - R3/R4
 *
 * under normal: R1=R3,R2=R4; Vout = Vbat_up * R1 / R2
 *
 * This formula from opa2333p.
 */
static int get_cali_vbat_hkadc_mv(void)
{
	int i;
	int adc_vbat;
	int vol_temp;
	int bat0_vol_offset;
	s64 temp;
	int vol_cali;

	if (!g_vbat_hkadc_di) {
		hwlog_err("g_vbat_hkadc_di is null\n");
		return -1;
	}

	for (i = 0; i < VBAT_HKADC_RETRY_TIMES; i++) {
		adc_vbat = hisi_adc_get_adc(g_vbat_hkadc_di->adc_channel);

		if (adc_vbat < 0)
			hwlog_err("hisi adc read fail\n");
		else
			break;
	}

	if (adc_vbat < 0)
		return -1;

	vol_temp = adc_vbat * (g_vbat_hkadc_di->coef) /
		VBAT_HKADC_COEF_MULTIPLE;

	bat0_vol_offset = g_v_offset_c * hisi_battery_voltage() /
		VBAT_CALI_MULTIPLE;

	/* get voltage(mv) by calibration */
	temp = vol_temp * (s64)(g_v_offset_a);
	/* g_v_offset_b uV to mV */
	vol_cali = (int)div_s64(temp, VBAT_CALI_MULTIPLE) +
		g_v_offset_b / 1000 + bat0_vol_offset;

	hwlog_info("adc_vbat=%d, coef=%d, vol_cali=%d, bat0_vol_offset=%d\n",
		adc_vbat, g_vbat_hkadc_di->coef, vol_cali, bat0_vol_offset);
	hwlog_info("v_offset_a=%ld, v_offset_b=%ld, v_offset_c=%ld\n",
		g_v_offset_a, g_v_offset_b, g_v_offset_c);

	return vol_cali;
}

static int get_compensate_vbat_hkadc_mv(void)
{
	int vol_cali;
	int vol_comp;
	int cur;

	vol_cali = get_cali_vbat_hkadc_mv();
	cur = hisi_battery_current();

	/* cur:mA, r_pcb:uOhm */
	vol_comp = vol_cali + cur * (g_vbat_hkadc_di->r_pcb) / 1000000;

	hwlog_info("cur=%d, vol_comp=%d\n", cur, vol_comp);

	return vol_comp;
}

static struct hw_batt_vol_ops vbat_hkadc_ops = {
	.get_batt_vol = get_compensate_vbat_hkadc_mv,
};

#ifdef CONFIG_SYSFS
static ssize_t vbat_hkadc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t vbat_hkadc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info vbat_hkadc_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(vbat_hkadc, 0640, VBAT_HKADC_SYSFS_V_OFFSET_A, v_offset_a),
	power_sysfs_attr_rw(vbat_hkadc, 0640, VBAT_HKADC_SYSFS_V_OFFSET_B, v_offset_b),
	power_sysfs_attr_rw(vbat_hkadc, 0640, VBAT_HKADC_SYSFS_V_OFFSET_C, v_offset_c),
	power_sysfs_attr_ro(vbat_hkadc, 0440, VBAT_HKADC_SYSFS_BAT1_CALI_VOL, bat1_cali_vol),
};

#define VBAT_HKADC_SYSFS_ATTRS_SIZE  ARRAY_SIZE(vbat_hkadc_sysfs_field_tbl)

static struct attribute *vbat_hkadc_sysfs_attrs[VBAT_HKADC_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group vbat_hkadc_sysfs_attr_group = {
	.attrs = vbat_hkadc_sysfs_attrs,
};

static void vbat_hkadc_sysfs_init_attrs(void)
{
	int i;

	for (i = 0; i < VBAT_HKADC_SYSFS_ATTRS_SIZE; i++)
		vbat_hkadc_sysfs_attrs[i] = &vbat_hkadc_sysfs_field_tbl[i].attr.attr;

	vbat_hkadc_sysfs_attrs[i] = NULL;
}

static struct power_sysfs_attr_info *vbat_hkadc_sysfs_field_lookup(
	const char *name)
{
	int i;

	for (i = 0; i < VBAT_HKADC_SYSFS_ATTRS_SIZE; i++) {
		if (!strncmp(name, vbat_hkadc_sysfs_field_tbl[i].attr.attr.name,
			strlen(name)))
			break;
	}

	if (i >= VBAT_HKADC_SYSFS_ATTRS_SIZE)
		return NULL;

	return &vbat_hkadc_sysfs_field_tbl[i];
}

static ssize_t vbat_hkadc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	int len = 0;

	info = vbat_hkadc_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case VBAT_HKADC_SYSFS_V_OFFSET_A:
		len = snprintf(buf, PAGE_SIZE, "%ld\n", g_v_offset_a);
		break;
	case VBAT_HKADC_SYSFS_V_OFFSET_B:
		len = snprintf(buf, PAGE_SIZE, "%ld\n", g_v_offset_b);
		break;
	case VBAT_HKADC_SYSFS_V_OFFSET_C:
		len = snprintf(buf, PAGE_SIZE, "%ld\n", g_v_offset_c);
		break;
	case VBAT_HKADC_SYSFS_BAT1_CALI_VOL:
		len = snprintf(buf, PAGE_SIZE, "%d\n",
			get_compensate_vbat_hkadc_mv());
		break;
	default:
		break;
	}

	return len;
}

static ssize_t vbat_hkadc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val = 0;
	struct power_sysfs_attr_info *info = NULL;

	info = vbat_hkadc_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case VBAT_HKADC_SYSFS_V_OFFSET_A:
		if ((kstrtol(buf, 10, &val) < 0) ||
			(val < V_OFFSET_A_MIN) || (val > V_OFFSET_A_MAX)) {
			hwlog_err("store v_offset_a=%ld\n", val);
			return -EINVAL;
		}
		hwlog_info("v_offset_a_sysfs_store,val=%ld\n", val);
		g_v_offset_a = val;
		break;
	case VBAT_HKADC_SYSFS_V_OFFSET_B:
		if ((kstrtol(buf, 10, &val) < 0) ||
			(val < V_OFFSET_B_MIN) || (val > V_OFFSET_B_MAX)) {
			hwlog_err("store v_offset_b=%ld\n", val);
			return -EINVAL;
		}
		hwlog_info("v_offset_b_sysfs_store,val=%ld\n", val);
		g_v_offset_b = val;
		break;
	case VBAT_HKADC_SYSFS_V_OFFSET_C:
		if ((kstrtol(buf, 10, &val) < 0) ||
			(val < V_OFFSET_C_MIN) || (val > V_OFFSET_C_MAX)) {
			hwlog_err("store v_offset_c=%ld\n", val);
			return -EINVAL;
		}
		hwlog_info("v_offset_c_sysfs_store,val=%ld\n", val);
		g_v_offset_c = val;
		break;
	default:
		break;
	}

	return count;
}

static void vbat_hkadc_sysfs_create_group(struct device *dev)
{
	vbat_hkadc_sysfs_init_attrs();
	power_sysfs_create_link_group("hw_power", "charger", "vbat_hkadc",
		dev, &vbat_hkadc_sysfs_attr_group);
}

static void vbat_hkadc_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "vbat_hkadc",
		dev, &vbat_hkadc_sysfs_attr_group);
}
#else
static inline void vbat_hkadc_sysfs_create_group(struct device *dev)
{
}

static inline void vbat_hkadc_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int vbat_hkadc_parse_dts(struct device_node *np,
	struct vbat_hkadc_info *di)
{
	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"adc_channel", &di->adc_channel, 0))
		return -EINVAL;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"coef", &di->coef, 0))
		return -EINVAL;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"r_pcb", &di->r_pcb, 0))
		return -EINVAL;

	return 0;

}

static int vbat_hkadc_probe(struct platform_device *pdev)
{
	struct vbat_hkadc_info *di = NULL;
	struct device_node *np = NULL;
	int ret = -1;

	hwlog_info("probe begin\n");

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_vbat_hkadc_di = di;
	di->pdev = pdev;
	di->dev = &pdev->dev;
	np = pdev->dev.of_node;

	ret = vbat_hkadc_parse_dts(np, di);
	if (ret)
		goto fail_parse_dts;

	ret = hw_battery_voltage_ops_register(&vbat_hkadc_ops, "hisi_hkadc");
	if (ret)
		hwlog_err("hw_battery_voltage ops register failed\n");

	vbat_hkadc_sysfs_create_group(di->dev);
	platform_set_drvdata(pdev, di);

	hwlog_info("probe end\n");
	return 0;

fail_parse_dts:
	devm_kfree(&pdev->dev, di);
	g_vbat_hkadc_di = NULL;

	return ret;
}

static int vbat_hkadc_remove(struct platform_device *pdev)
{
	struct vbat_hkadc_info *info = platform_get_drvdata(pdev);

	hwlog_info("remove begin\n");

	if (!info)
		return -ENODEV;

	vbat_hkadc_sysfs_remove_group(info->dev);
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, info);
	g_vbat_hkadc_di = NULL;

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id vbat_hkadc_match_table[] = {
	{
		.compatible = "huawei,vbat_hkadc",
		.data = NULL,
	},
	{},
};

static struct platform_driver vbat_hkadc_driver = {
	.probe = vbat_hkadc_probe,
	.remove = vbat_hkadc_remove,
	.driver = {
		.name = "huawei,vbat_hkadc",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(vbat_hkadc_match_table),
	},
};

static int __init vbat_hkadc_init(void)
{
	return platform_driver_register(&vbat_hkadc_driver);
}

static void __exit vbat_hkadc_exit(void)
{
	platform_driver_unregister(&vbat_hkadc_driver);
}

fs_initcall_sync(vbat_hkadc_init);
module_exit(vbat_hkadc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei vbat_hkadc module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
