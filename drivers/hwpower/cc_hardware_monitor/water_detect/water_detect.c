/*
 * water_detect.c
 *
 * water intruded detect driver
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

#include <chipset_common/hwpower/hardware_monitor/water_detect.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_ui_ne.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG water_detect
HWLOG_REGIST();

static struct water_detect_dev *g_water_detect_dev;

static const char * const g_water_detect_type_table[] = {
	[WD_TYPE_USB_DP_DN] = "usb_dp_dn",
	[WD_TYPE_USB_ID] = "usb_id",
	[WD_TYPE_USB_GPIO] = "usb_gpio",
	[WD_TYPE_AUDIO_DP_DN] = "audio_dp_dn",
};

static int water_detect_check_type(int type)
{
	if ((type >= WD_TYPE_BEGIN) && (type < WD_TYPE_END))
		return 0;

	hwlog_err("invalid type=%d\n", type);
	return -1;
}

static int water_detect_get_type(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_water_detect_type_table); i++) {
		if (!strncmp(str, g_water_detect_type_table[i], strlen(str)))
			return i;
	}

	hwlog_err("invalid type_str=%s\n", str);
	return -1;
}

static const char *water_detect_get_type_name(int type)
{
	if ((type >= WD_TYPE_BEGIN) && (type < WD_TYPE_END))
		return g_water_detect_type_table[type];

	return "illegal type";
}

static struct water_detect_dev *water_detect_get_dev(void)
{
	if (!g_water_detect_dev) {
		hwlog_err("g_water_detect_dev is null\n");
		return NULL;
	}

	return g_water_detect_dev;
}

int water_detect_ops_register(struct water_detect_ops *ops)
{
	int type;

	if (!g_water_detect_dev || !ops || !ops->type_name) {
		hwlog_err("g_water_detect_dev or ops or type_name is null\n");
		return -1;
	}

	type = water_detect_get_type(ops->type_name);
	if (type < 0) {
		hwlog_err("%s ops register fail\n", ops->type_name);
		return -1;
	}

	g_water_detect_dev->ops[type] = ops;
	g_water_detect_dev->total_ops++;

	hwlog_info("total_ops=%d type=%d:%s ops register ok\n",
		g_water_detect_dev->total_ops, type, ops->type_name);
	return 0;
}

static void water_detect_set_intruded_status(unsigned int mode, unsigned int type)
{
	struct water_detect_dev *l_dev = water_detect_get_dev();

	if (!l_dev)
		return;

	if (mode == WD_OP_SET)
		l_dev->status |= (1 << type);
	if (mode == WD_OP_CLR)
		l_dev->status &= (~(unsigned int)(1 << type));
}

bool water_detect_get_intruded_status(void)
{
	struct water_detect_dev *l_dev = water_detect_get_dev();

	if (!l_dev)
		return false;

	return (l_dev->status > 0) ? true : false;
}

static bool water_detect_is_disabled(struct water_detect_dev *l_dev)
{
	if (power_cmdline_is_powerdown_charging_mode()) {
		hwlog_info("water detect disabled on pd charging mode\n");
		return true;
	}

	if (power_cmdline_is_factory_mode()) {
		hwlog_info("water detect disabled on factory mode\n");
		return true;
	}

	return false;
}

static void water_detect_report_dmd(const char *buf)
{
	char dsm_buff[POWER_DSM_BUF_SIZE_0128] = { 0 };

	if (buf)
		snprintf(dsm_buff, POWER_DSM_BUF_SIZE_0128 - 1,
			"water check is triggered : %s\n", buf);
	else
		snprintf(dsm_buff, POWER_DSM_BUF_SIZE_0128 - 1,
			"water check is triggered\n");

	power_dsm_report_dmd(POWER_DSM_BATTERY,
		ERROR_NO_WATER_CHECK_IN_USB, dsm_buff);
}

static void water_detect_report_uevent(unsigned int flag)
{
	int data = flag;

	power_ui_event_notify(POWER_UI_NE_WATER_STATUS, &data);
}

static void water_detect_monitor(struct water_detect_dev *l_dev,
	enum water_detect_type type)
{
	struct water_detect_ops *l_ops = NULL;

	if (water_detect_check_type(type))
		return;

	if (water_detect_is_disabled(l_dev))
		return;

	l_ops = l_dev->ops[type];
	if (!l_ops || !l_ops->is_water_intruded) {
		hwlog_err("l_ops or is_water_intruded is null\n");
		return;
	}

	if (l_ops->is_water_intruded(l_ops->dev_data)) {
		water_detect_set_intruded_status(WD_OP_SET, type);
		water_detect_report_uevent(WD_NON_STBY_MOIST);
		water_detect_report_dmd(water_detect_get_type_name(type));
	} else {
		water_detect_set_intruded_status(WD_OP_CLR, type);
	}
}

static int water_detect_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct water_detect_dev *l_dev = water_detect_get_dev();

	if (!l_dev)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_WD_REPORT_DMD:
		water_detect_report_dmd((char *)data);
		break;
	case POWER_NE_WD_REPORT_UEVENT:
		water_detect_report_uevent(*(unsigned int *)data);
		break;
	case POWER_NE_WD_DETECT_BY_USB_DP_DN:
		if (l_dev->enable.usb_dp_dn)
			water_detect_monitor(l_dev, WD_TYPE_USB_DP_DN);
		break;
	case POWER_NE_WD_DETECT_BY_USB_ID:
		if (l_dev->enable.usb_id)
			water_detect_monitor(l_dev, WD_TYPE_USB_ID);
		break;
	case POWER_NE_WD_DETECT_BY_USB_GPIO:
		if (l_dev->enable.usb_gpio)
			water_detect_monitor(l_dev, WD_TYPE_USB_GPIO);
		break;
	case POWER_NE_WD_DETECT_BY_AUDIO_DP_DN:
		if (l_dev->enable.audio_dp_dn)
			water_detect_monitor(l_dev, WD_TYPE_AUDIO_DP_DN);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static void water_detect_parse_dts(void)
{
	(void)power_dts_read_u32_compatible(power_dts_tag(HWLOG_TAG),
		"huawei,charger", "wd_by_usb_dp_dn",
		&g_water_detect_dev->enable.usb_dp_dn, 0);
	(void)power_dts_read_u32_compatible(power_dts_tag(HWLOG_TAG),
		"huawei,charger", "wd_by_usb_id",
		&g_water_detect_dev->enable.usb_id, 0);
	(void)power_dts_read_u32_compatible(power_dts_tag(HWLOG_TAG),
		"huawei,charger", "wd_by_usb_gpio",
		&g_water_detect_dev->enable.usb_gpio, 0);
	(void)power_dts_read_u32_compatible(power_dts_tag(HWLOG_TAG),
		"huawei,charger", "wd_by_audio_dp_dn",
		&g_water_detect_dev->enable.audio_dp_dn, 0);
}

static int __init water_detect_init(void)
{
	int ret;
	struct water_detect_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_water_detect_dev = l_dev;
	water_detect_parse_dts();

	l_dev->nb.notifier_call = water_detect_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_WD, &l_dev->nb);
	if (ret)
		goto fail_free_mem;

	return 0;

fail_free_mem:
	kfree(l_dev);
	g_water_detect_dev = NULL;
	return ret;
}

static void __exit water_detect_exit(void)
{
	if (!g_water_detect_dev)
		return;

	power_event_bnc_unregister(POWER_BNT_WD, &g_water_detect_dev->nb);
	kfree(g_water_detect_dev);
	g_water_detect_dev = NULL;
}

subsys_initcall(water_detect_init);
module_exit(water_detect_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("water intruded detect driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
