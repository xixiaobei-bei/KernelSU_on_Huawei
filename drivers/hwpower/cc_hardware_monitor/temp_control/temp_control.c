/*
 * temp_control.c
 *
 * huawei temperature control driver
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

#include "temp_control.h"
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_temp.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG temp_control
HWLOG_REGIST();

static struct temp_control_dev *g_temp_control_dev;

static struct temp_control_dev *temp_control_get_dev(void)
{
	if (!g_temp_control_dev) {
		hwlog_err("g_temp_control_dev is null\n");
		return NULL;
	}

	return g_temp_control_dev;
}

static void temp_control_dmd_report(struct temp_control_para_info *info,
	const char *buf)
{
	if (info->dmd_no <= 0)
		return;

	if (info->dmd_count++ >= info->dmd_max_cnt) {
		hwlog_info("dmd report over %d time\n", info->dmd_max_cnt);
		return;
	}

	power_dsm_report_dmd(POWER_DSM_BATTERY, info->dmd_no, buf);
}

static int temp_control_check_usb_port_temp(struct temp_control_dev *l_dev)
{
	char buf[POWER_DSM_BUF_SIZE_0128] = { 0 };
	int usb_temp;
	int index;
	struct common_hys_data hys;

	usb_temp = power_temp_get_average_value(POWER_TEMP_USB_PORT);
	hys.refer = usb_temp / POWER_TEMP_UNIT;
	hys.para_size = l_dev->usb_port_para_size;
	hys.para = l_dev->usb_port_hys;

	l_dev->usb_port_para_index = power_get_hysteresis_index(
		l_dev->usb_port_para_index, &hys);
	index = l_dev->usb_port_para_index;

	if (l_dev->usb_port_para[index].dmd_no > 0) {
		snprintf(buf, POWER_DSM_BUF_SIZE_0128 - 1,
			"t_usb %d is exceed %d, t_bat=%d chg_type=%d brand=%s volt=%d soc=%d\n",
			usb_temp / POWER_TEMP_UNIT,
			l_dev->usb_port_hys[index].refer_lth,
			power_platform_get_battery_temperature(),
			power_platform_get_charger_type(),
			power_platform_get_battery_brand(),
			power_platform_get_battery_voltage(),
			power_platform_get_battery_capacity());
		temp_control_dmd_report(&l_dev->usb_port_para[index], buf);
	}

	return l_dev->usb_port_para[index].iout_max;
}

static void temp_control_monitor_work(struct work_struct *work)
{
	struct temp_control_dev *l_dev = temp_control_get_dev();
	int delay = DELAY_TIME_FOR_SLOW_WORK;

	if (!l_dev)
		return;

	if (l_dev->usb_port_para_size != 0)
		temp_control_check_usb_port_temp(l_dev);

	schedule_delayed_work(&l_dev->monitor_work, msecs_to_jiffies(delay));
}

static void temp_control_start(struct temp_control_dev *l_dev)
{
	int delay = DELAY_TIME_FOR_SLOW_WORK;

	cancel_delayed_work(&l_dev->monitor_work);
	schedule_delayed_work(&l_dev->monitor_work, msecs_to_jiffies(delay));
}

static void temp_control_stop(struct temp_control_dev *l_dev)
{
	cancel_delayed_work(&l_dev->monitor_work);
	l_dev->usb_port_para_index = 0;
}

static int temp_control_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct temp_control_dev *l_dev = temp_control_get_dev();

	if (!l_dev)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_STOP_CHARGING:
		temp_control_stop(l_dev);
		break;
	case POWER_NE_START_CHARGING:
		temp_control_start(l_dev);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static void temp_control_parse_usb_port_para(struct device_node *np,
	struct temp_control_dev *l_dev)
{
	int row, col, len;
	int idata[TEMP_CTRL_PARA_LEVEL * TEMP_CTRL_PARA_TOTAL] = { 0 };

	l_dev->usb_port_para_index = 0;
	l_dev->usb_port_para_size = 0;

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"usb_port_para", idata, TEMP_CTRL_PARA_LEVEL, TEMP_CTRL_PARA_TOTAL);
	if (len < 0)
		return;

	l_dev->usb_port_para_size = len / TEMP_CTRL_PARA_TOTAL;

	for (row = 0; row < len / TEMP_CTRL_PARA_TOTAL; row++) {
		col = row * TEMP_CTRL_PARA_TOTAL + TEMP_CTRL_TEMP_LTH;
		l_dev->usb_port_hys[row].refer_lth = idata[col];
		col = row * TEMP_CTRL_PARA_TOTAL + TEMP_CTRL_TEMP_HTH;
		l_dev->usb_port_hys[row].refer_hth = idata[col];
		col = row * TEMP_CTRL_PARA_TOTAL + TEMP_CTRL_TEMP_BACK;
		l_dev->usb_port_hys[row].hys_value = idata[col];

		col = row * TEMP_CTRL_PARA_TOTAL + TEMP_CTRL_IOUT_MAX;
		l_dev->usb_port_para[row].iout_max = idata[col];
		col = row * TEMP_CTRL_PARA_TOTAL + TEMP_CTRL_DMD_NO;
		l_dev->usb_port_para[row].dmd_no = idata[col];
		col = row * TEMP_CTRL_PARA_TOTAL + TEMP_CTRL_DMD_MAX_CNT;
		l_dev->usb_port_para[row].dmd_max_cnt = idata[col];
	}
}

static void temp_control_parse_dts(struct device_node *np,
	struct temp_control_dev *l_dev)
{
	temp_control_parse_usb_port_para(np, l_dev);
}

static int temp_control_probe(struct platform_device *pdev)
{
	int ret;
	struct temp_control_dev *l_dev = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_temp_control_dev = l_dev;
	np = pdev->dev.of_node;
	temp_control_parse_dts(np, l_dev);
	l_dev->nb.notifier_call = temp_control_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_CHARGING, &l_dev->nb);
	if (ret)
		goto fail_free_mem;

	INIT_DELAYED_WORK(&l_dev->monitor_work, temp_control_monitor_work);
	platform_set_drvdata(pdev, l_dev);
	return 0;

fail_free_mem:
	kfree(l_dev);
	g_temp_control_dev = NULL;
	return ret;
}

static int temp_control_remove(struct platform_device *pdev)
{
	struct temp_control_dev *l_dev = platform_get_drvdata(pdev);

	if (!l_dev)
		return -ENODEV;

	cancel_delayed_work(&l_dev->monitor_work);
	power_event_bnc_unregister(POWER_BNT_CHARGING, &l_dev->nb);
	platform_set_drvdata(pdev, NULL);
	kfree(l_dev);
	g_temp_control_dev = NULL;

	return 0;
}

#ifdef CONFIG_PM
static int temp_control_suspend(struct platform_device *pdev,
	pm_message_t state)
{
	struct temp_control_dev *l_dev = platform_get_drvdata(pdev);

	hwlog_info("suspend begin\n");

	if (!l_dev)
		return 0;

	cancel_delayed_work_sync(&l_dev->monitor_work);
	return 0;
}

static int temp_control_resume(struct platform_device *pdev)
{
	hwlog_info("resume begin\n");
	return 0;
}
#endif /* CONFIG_PM */

static const struct of_device_id temp_control_match_table[] = {
	{
		.compatible = "huawei,temp_control",
		.data = NULL,
	},
	{},
};

static struct platform_driver temp_control_driver = {
	.probe = temp_control_probe,
	.remove = temp_control_remove,
#ifdef CONFIG_PM
	.suspend = temp_control_suspend,
	.resume = temp_control_resume,
#endif /* CONFIG_PM */
	.driver = {
		.name = "huawei,temp_control",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(temp_control_match_table),
	},
};

static int __init temp_control_init(void)
{
	return platform_driver_register(&temp_control_driver);
}

static void __exit temp_control_exit(void)
{
	platform_driver_unregister(&temp_control_driver);
}

device_initcall(temp_control_init);
module_exit(temp_control_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei temperature control driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
