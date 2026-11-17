/*
 * vbus_channel_charger.c
 *
 * charger otg for vbus channel driver
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

#include <chipset_common/hwpower/hardware_channel/vbus_channel_charger.h>
#include <chipset_common/hwpower/hardware_channel/vbus_channel.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG vbus_ch_charger
HWLOG_REGIST();

static struct charger_otg_dev *g_charger_otg_dev;

static const char * const g_charger_otg_device_id_table[] = {
	[OTG_DEVICE_ID_BQ2419X] = "bq2419x",
	[OTG_DEVICE_ID_BQ2429X] = "bq2429x",
	[OTG_DEVICE_ID_BQ2560X] = "bq2560x",
	[OTG_DEVICE_ID_BQ25882] = "bq25882",
	[OTG_DEVICE_ID_BQ25892] = "bq25892",
	[OTG_DEVICE_ID_HL7019] = "hl7019",
	[OTG_DEVICE_ID_RT9466] = "rt9466",
	[OTG_DEVICE_ID_RT9471] = "rt9471",
	[OTG_DEVICE_ID_HI6522] = "hi6522",
	[OTG_DEVICE_ID_HI6523] = "hi6523",
	[OTG_DEVICE_ID_HI6526] = "hi6526",
	[OTG_DEVICE_ID_DUAL_CHARGER] = "dual_charger",
	[OTG_DEVICE_ID_BD99954] = "bd99954",
	[OTG_DEVICE_ID_BQ25713] = "bq25713",
};

static int charger_otg_get_device_id(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_charger_otg_device_id_table); i++) {
		if (!strncmp(str, g_charger_otg_device_id_table[i], strlen(str)))
			return i;
	}

	return -1;
}

int charger_otg_ops_register(struct charger_otg_device_ops *ops)
{
	int dev_id;

	if (!g_charger_otg_dev || !ops || !ops->dev_name) {
		hwlog_err("g_charger_otg_dev or ops or dev_name is null\n");
		return -1;
	}

	dev_id = charger_otg_get_device_id(ops->dev_name);
	if (dev_id < 0) {
		hwlog_err("%s ops register fail\n", ops->dev_name);
		return -1;
	}

	g_charger_otg_dev->ops = ops;
	g_charger_otg_dev->dev_id = dev_id;

	hwlog_info("%d:%s ops register ok\n", dev_id, ops->dev_name);
	return 0;
}

static void charger_otg_wake_lock(struct charger_otg_dev *l_dev)
{
	if (!l_dev->otg_lock.active) {
		__pm_stay_awake(&l_dev->otg_lock);
		hwlog_info("wake lock\n");
	}
}

static void charger_otg_wake_unlock(struct charger_otg_dev *l_dev)
{
	if (l_dev->otg_lock.active) {
		__pm_relax(&l_dev->otg_lock);
		hwlog_info("wake unlock\n");
	}
}

static struct charger_otg_dev *charger_otg_get_dev(void)
{
	if (!g_charger_otg_dev) {
		hwlog_err("g_charger_otg_dev is null\n");
		return NULL;
	}

	return g_charger_otg_dev;
}

static void charger_otg_set_charger_enable(struct charger_otg_dev *l_dev,
	int enable)
{
	int ret;

	if (l_dev->ops && l_dev->ops->otg_set_charger_enable) {
		ret = l_dev->ops->otg_set_charger_enable(enable);
		if (ret)
			hwlog_err("set charger enable fail\n");
		else
			hwlog_info("set charger enable %d\n", enable);
	}
}

static void charger_otg_set_otg_enable(struct charger_otg_dev *l_dev,
	int enable)
{
	int ret;

	if (l_dev->ops && l_dev->ops->otg_set_enable) {
		ret = l_dev->ops->otg_set_enable(enable);
		if (ret)
			hwlog_err("set otg enable fail\n");
		else
			hwlog_info("set otg enable %d\n", enable);
	}
}

static void charger_otg_set_otg_current(struct charger_otg_dev *l_dev,
	int value)
{
	int ret;

	if (l_dev->ops && l_dev->ops->otg_set_current) {
		ret = l_dev->ops->otg_set_current(value);
		if (ret)
			hwlog_err("set otg current fail\n");
		else
			hwlog_info("set otg current %d\n", value);
	}
}

static void charger_otg_disable_watchdog(struct charger_otg_dev *l_dev,
	int disable_syswdt)
{
	int ret;

	if (l_dev->ops && l_dev->ops->otg_set_watchdog_timer) {
		ret = l_dev->ops->otg_set_watchdog_timer(false);
		if (ret)
			hwlog_err("otg disable watchdog fail\n");
		else
			hwlog_info("otg disable watchdog\n");
	}

	if (disable_syswdt)
		power_platform_charge_stop_sys_wdt();
}

static void charger_otg_kick_watchdog(struct charger_otg_dev *l_dev,
	int disable_syswdt)
{
	int ret;

	if (l_dev->ops && l_dev->ops->otg_reset_watchdog_timer) {
		ret = l_dev->ops->otg_reset_watchdog_timer();
		if (ret)
			hwlog_err("kick watchdog fail\n");
		else
			hwlog_info("kick watchdog\n");
	}

	if (disable_syswdt)
		power_platform_charge_feed_sys_wdt(CHARGER_OTG_SYS_WDT_TIMEOUT);
}

static void charger_otg_work(struct work_struct *work)
{
	struct charger_otg_dev *l_dev = charger_otg_get_dev();

	if (!l_dev)
		return;

	if (power_platform_get_cancel_work_flag()) {
		hwlog_info("charge already stop\n");
		return;
	}

	charger_otg_set_otg_enable(l_dev, true);
	charger_otg_kick_watchdog(l_dev, true);

	schedule_delayed_work(&l_dev->otg_work,
		msecs_to_jiffies(CHARGER_OTG_WORK_TIME));
}

static int charger_otg_start_config(int flag)
{
	struct charger_otg_dev *l_dev = charger_otg_get_dev();

	if (!l_dev)
		return -EINVAL;

	l_dev->mode = VBUS_CH_IN_OTG_MODE;

	charger_otg_wake_lock(l_dev);
	charger_otg_set_charger_enable(l_dev, false);
	charger_otg_set_otg_enable(l_dev, true);
	charger_otg_set_otg_current(l_dev, l_dev->otg_curr);
	if (!delayed_work_pending(&l_dev->otg_work))
		schedule_delayed_work(&l_dev->otg_work, msecs_to_jiffies(0));
	charger_otg_wake_unlock(l_dev);

	hwlog_info("start reverse_vbus flag=%d\n", flag);
	return 0;
}

static int charger_otg_stop_config(int flag)
{
	struct charger_otg_dev *l_dev = charger_otg_get_dev();

	if (!l_dev)
		return -EINVAL;

	l_dev->mode = VBUS_CH_NOT_IN_OTG_MODE;

	charger_otg_set_charger_enable(l_dev, false);
	charger_otg_set_otg_enable(l_dev, false);
	cancel_delayed_work_sync(&l_dev->otg_work);
	if (power_platform_get_sysfs_wdt_disable_flag())
		charger_otg_disable_watchdog(l_dev, false);

	hwlog_info("stop reverse_vbus flag=%d\n", flag);
	return 0;
}

static int charger_otg_open(unsigned int user, int flag)
{
	struct charger_otg_dev *l_dev = charger_otg_get_dev();

	if (!l_dev)
		return -EINVAL;

	if (charger_otg_start_config(flag))
		return -EINVAL;

	l_dev->user |= (1 << user);

	hwlog_info("user=%x open ok\n", l_dev->user);
	return 0;
}

static int charger_otg_close(unsigned int user, int flag, int force)
{
	struct charger_otg_dev *l_dev = charger_otg_get_dev();

	if (!l_dev)
		return -EINVAL;

	if (force) {
		charger_otg_set_otg_enable(l_dev, false);
		hwlog_info("force stop reverse_vbus\n");
		return 0;
	}

	l_dev->user &= (~(unsigned int)(1 << user));

	if (l_dev->user == VBUS_CH_NO_OP_USER) {
		if (charger_otg_stop_config(flag))
			return -EINVAL;
	}

	hwlog_info("user=%x close ok\n", l_dev->user);
	return 0;
}

static int charger_otg_get_state(unsigned int user, int *state)
{
	struct charger_otg_dev *l_dev = charger_otg_get_dev();

	if (!l_dev || !state)
		return -EINVAL;

	if (l_dev->user == VBUS_CH_NO_OP_USER)
		*state = VBUS_CH_STATE_CLOSE;
	else
		*state = VBUS_CH_STATE_OPEN;

	return 0;
}

static int charger_otg_get_mode(unsigned int user, int *mode)
{
	struct charger_otg_dev *l_dev = charger_otg_get_dev();

	if (!l_dev || !mode)
		return -EINVAL;

	*mode = l_dev->mode;

	return 0;
}

static int charger_otg_set_switch_mode(unsigned int user, int mode)
{
	struct charger_otg_dev *l_dev = charger_otg_get_dev();

	if (!l_dev)
		return -EINVAL;

	if (l_dev->ops && l_dev->ops->otg_set_switch_mode) {
		if (l_dev->ops->otg_set_switch_mode(mode))
			hwlog_err("set switch mode fail\n");
		else
			hwlog_info("set switch mode %d\n", mode);
	}

	return 0;
}

static struct vbus_ch_ops charger_otg_ops = {
	.type_name = "charger",
	.open = charger_otg_open,
	.close = charger_otg_close,
	.get_state = charger_otg_get_state,
	.get_mode = charger_otg_get_mode,
	.set_switch_mode = charger_otg_set_switch_mode,
	.set_voltage = NULL,
	.get_voltage = NULL,
};

static int charger_otg_parse_dts(struct device_node *np,
	struct charger_otg_dev *l_dev)
{
	/* get configuration of charger otg current */
	(void)power_dts_read_u32_compatible(power_dts_tag(HWLOG_TAG),
		"huawei,charging_core", "otg_curr",
		&l_dev->otg_curr, 1000); /* default is 1000ma */

	return 0;
}

static int charger_otg_probe(struct platform_device *pdev)
{
	int ret;
	struct charger_otg_dev *l_dev = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	l_dev = devm_kzalloc(&pdev->dev, sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_charger_otg_dev = l_dev;
	l_dev->pdev = pdev;
	l_dev->dev = &pdev->dev;

	ret = charger_otg_parse_dts(l_dev->dev->of_node, l_dev);
	if (ret)
		goto fail_parse_dts;

	ret = vbus_ch_ops_register(&charger_otg_ops);
	if (ret)
		goto fail_register_ops;

	wakeup_source_init(&l_dev->otg_lock, "otg_wakelock");
	INIT_DELAYED_WORK(&l_dev->otg_work, charger_otg_work);
	platform_set_drvdata(pdev, l_dev);
	return 0;

fail_register_ops:
fail_parse_dts:
	devm_kfree(&pdev->dev, l_dev);
	g_charger_otg_dev = NULL;
	return ret;
}

static int charger_otg_remove(struct platform_device *pdev)
{
	struct charger_otg_dev *l_dev = platform_get_drvdata(pdev);

	if (!l_dev)
		return -EINVAL;

	wakeup_source_trash(&l_dev->otg_lock);
	cancel_delayed_work(&l_dev->otg_work);
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, l_dev);
	g_charger_otg_dev = NULL;
	return 0;
}

#ifdef CONFIG_PM
static int charger_otg_suspend(struct platform_device *pdev,
	pm_message_t state)
{
	struct charger_otg_dev *l_dev = charger_otg_get_dev();

	if (!l_dev)
		return 0;

	if (l_dev->mode == VBUS_CH_IN_OTG_MODE) {
		if (!l_dev->otg_lock.active)
			cancel_delayed_work(&l_dev->otg_work);

		charger_otg_disable_watchdog(l_dev, true);
	}

	return 0;
}

static int charger_otg_resume(struct platform_device *pdev)
{
	return 0;
}
#endif /* CONFIG_PM */

static void charger_otg_shutdown(struct platform_device *pdev)
{
	struct charger_otg_dev *l_dev = charger_otg_get_dev();

	if (!l_dev)
		return;

	charger_otg_set_otg_enable(l_dev, false);
	cancel_delayed_work(&l_dev->otg_work);
}

static const struct of_device_id charger_otg_match_table[] = {
	{
		.compatible = "huawei,vbus_channel_charger",
		.data = NULL,
	},
	{},
};

static struct platform_driver charger_otg_driver = {
	.probe = charger_otg_probe,
	.remove = charger_otg_remove,
#ifdef CONFIG_PM
	.suspend = charger_otg_suspend,
	.resume = charger_otg_resume,
#endif /* CONFIG_PM */
	.shutdown = charger_otg_shutdown,
	.driver = {
		.name = "huawei,vbus_channel_charger",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(charger_otg_match_table),
	},
};

static int __init charger_otg_init(void)
{
	return platform_driver_register(&charger_otg_driver);
}

static void __exit charger_otg_exit(void)
{
	platform_driver_unregister(&charger_otg_driver);
}

fs_initcall(charger_otg_init);
module_exit(charger_otg_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("charger for vbus channel driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
