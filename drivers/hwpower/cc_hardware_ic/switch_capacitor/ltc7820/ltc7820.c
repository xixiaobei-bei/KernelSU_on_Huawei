/*
 * ltc7820.c
 *
 * ltc7820 driver
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

#include "ltc7820.h"
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG ltc7820
HWLOG_REGIST();

static struct ltc7820_device_info *g_ltc7820_dev;

static int ltc7820_discharge(int enable, void *dev_data)
{
	return 0;
}

static int ltc7820_is_tsbat_disabled(void *dev_data)
{
	return 0;
}

static int ltc7820_config_watchdog_ms(int time, void *dev_data)
{
	return 0;
}

static int ltc7820_get_device_id(void *dev_data)
{
	return SWITCHCAP_LTC7820;
}

#ifdef POWER_MODULE_DEBUG_FUNCTION
static int ltc7820_set_freq(int freq)
{
	struct ltc7820_device_info *di = g_ltc7820_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	gpio_set_value(di->gpio_freq, freq);

	hwlog_info("set_freq freq=%d\n", freq);
	return 0;
}
#endif /* POWER_MODULE_DEBUG_FUNCTION */

static void ltc7820_int_en(bool en)
{
	struct ltc7820_device_info *di = g_ltc7820_dev;
	unsigned long flags;

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	spin_lock_irqsave(&di->int_lock, flags);
	if (en != di->is_int_en) {
		di->is_int_en = en;
		if (en)
			enable_irq(di->irq_int);
		else
			disable_irq_nosync(di->irq_int);
	}
	spin_unlock_irqrestore(&di->int_lock, flags);
}

static int ltc7820_charge_enable(int enable, void *dev_data)
{
	struct ltc7820_device_info *di = g_ltc7820_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (enable) {
		gpio_set_value(di->gpio_en, enable);
		ltc7820_int_en(!!enable);
		/* wait ltc7820 startup */
		msleep(100);
		di->init_finish_flag = LTC7820_INIT_FINISH;
	} else {
		ltc7820_int_en(!!enable);
		gpio_set_value(di->gpio_en, enable);
		di->init_finish_flag = LTC7820_NOT_INIT;
		di->int_notify_enable_flag = LTC7820_DISABLE_INT_NOTIFY;
	}

	hwlog_info("charge_enable enable=%d\n", enable);
	return 0;
}

static int ltc7820_is_device_close(void *dev_data)
{
	struct ltc7820_device_info *di = g_ltc7820_dev;
	int gpio_value;

	if (!di) {
		hwlog_err("di is null\n");
		return 1;
	}

	gpio_value = gpio_get_value(di->gpio_en);

	return (gpio_value == LTC7820_CHIP_ENABLE) ? 0 : 1;
}

static int ltc7820_charge_init(void *dev_data)
{
	struct ltc7820_device_info *di = g_ltc7820_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	hwlog_info("switchcap ltc7820 device id is %d\n",
		ltc7820_get_device_id(dev_data));

	return 0;
}

static int ltc7820_charge_exit(void *dev_data)
{
	struct ltc7820_device_info *di = g_ltc7820_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	return ltc7820_charge_enable(LTC7820_CHIP_DISABLE, dev_data);
}

static void ltc7820_interrupt_work(struct work_struct *work)
{
	struct ltc7820_device_info *di = NULL;
	struct nty_data *data = NULL;

	if (!work)
		return;

	di = container_of(work, struct ltc7820_device_info, irq_work);
	if (!di)
		return;

	data = &(di->nty_data);
	data->event1 = 0;
	data->event2 = 0;
	data->addr = 0;

	if (di->int_notify_enable_flag == LTC7820_ENABLE_INT_NOTIFY) {
		hwlog_info("ltc7820 chip error happened\n");
		power_event_anc_notify(POWER_ANT_SC_FAULT,
			POWER_NE_DC_FAULT_LTC7820, data);
	}

	/* clear irq */
	ltc7820_int_en(true);
}

static irqreturn_t ltc7820_interrupt(int irq, void *_di)
{
	struct ltc7820_device_info *di = _di;

	if (!di)
		return IRQ_HANDLED;

	if (di->chip_already_init == 0)
		hwlog_err("chip not init\n");

	if (di->init_finish_flag == LTC7820_INIT_FINISH)
		di->int_notify_enable_flag = LTC7820_ENABLE_INT_NOTIFY;
	else
		hwlog_info("ltc7820 ignore init int\n");

	hwlog_info("ltc7820 int happened\n");

	ltc7820_int_en(false);

	schedule_work(&di->irq_work);

	return IRQ_HANDLED;
}

static struct dc_ic_ops ltc7820_sysinfo_ops = {
	.dev_name = "ltc7820",
	.ic_init = ltc7820_charge_init,
	.ic_exit = ltc7820_charge_exit,
	.ic_enable = ltc7820_charge_enable,
	.ic_discharge = ltc7820_discharge,
	.is_ic_close = ltc7820_is_device_close,
	.get_ic_id = ltc7820_get_device_id,
	.config_ic_watchdog = ltc7820_config_watchdog_ms,
	.get_ic_status = ltc7820_is_tsbat_disabled,
};

static int ltc7820_gpio_init(struct ltc7820_device_info *di)
{
	int ret;

	ret = power_gpio_config_input(di->dev_node, "gpio_freq",
		"ltc7820_gpio_freq", &di->gpio_freq);
	if (ret)
		return ret;

	/* wait 10us for gpio_freq_direction switched from output to input */
	usleep_range(10, 20);

	/* check device, gpio pull low means device exits */
	ret = gpio_get_value(di->gpio_freq);
	if (ret) {
		hwlog_err("ltc7820 device disabled\n");
		gpio_free(di->gpio_freq);
		return ret;
	}

	ret = gpio_direction_output(di->gpio_freq, LTC7820_FREQ_DISABLE);
	if (ret) {
		hwlog_err("gpio set output fail\n");
		gpio_free(di->gpio_freq);
		return ret;
	}

	ret = power_gpio_config_output(di->dev_node, "gpio_en",
		"ltc7820_gpio_en", &di->gpio_en, LTC7820_CHIP_DISABLE);
	if (ret) {
		gpio_free(di->gpio_freq);
		return ret;
	}

	return 0;
}

static int ltc7820_irq_init(struct ltc7820_device_info *di)
{
	int ret;

	ret = power_gpio_config_interrupt(di->dev_node, "gpio_int",
		"ltc7820_gpio_int", &di->gpio_int, &di->irq_int);
	if (ret)
		return ret;

	ret = request_irq(di->irq_int, ltc7820_interrupt,
		IRQF_TRIGGER_FALLING, "ltc7820_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_int = -1;
		gpio_free(di->gpio_int);
		return ret;
	}

	return 0;
}

static int ltc7820_probe(struct platform_device *pdev)
{
	int ret;
	struct ltc7820_device_info *di = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_ltc7820_dev = di;
	di->pdev = pdev;
	di->dev = &pdev->dev;
	di->dev_node = di->dev->of_node;

	INIT_WORK(&di->irq_work, ltc7820_interrupt_work);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), di->dev_node,
		"ic_role", &di->ic_role, CHARGE_IC_TYPE_MAIN);

	ret = ltc7820_gpio_init(di);
	if (ret)
		goto ltc7820_fail_0;

	ret = ltc7820_irq_init(di);
	if (ret)
		goto ltc7820_fail_1;

	spin_lock_init(&di->int_lock);
	di->is_int_en = true;

	ltc7820_int_en(false);

	ret = dc_ic_ops_register(SC_MODE, di->ic_role, &ltc7820_sysinfo_ops);
	if (ret) {
		hwlog_err("ltc7820 sysinfo ops register fail\n");
		goto ltc7820_fail_2;
	}

	di->chip_already_init = 1;
	platform_set_drvdata(pdev, di);
	return 0;

ltc7820_fail_2:
	free_irq(di->irq_int, di);
	gpio_free(di->gpio_int);
ltc7820_fail_1:
	gpio_free(di->gpio_en);
	gpio_free(di->gpio_freq);
ltc7820_fail_0:
	devm_kfree(&pdev->dev, di);
	g_ltc7820_dev = NULL;

	return ret;
}

static int ltc7820_remove(struct platform_device *pdev)
{
	struct ltc7820_device_info *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	if (di->gpio_en)
		gpio_free(di->gpio_en);

	if (di->gpio_freq)
		gpio_free(di->gpio_freq);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	if (di->irq_int)
		free_irq(di->irq_int, di);

	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_ltc7820_dev = NULL;

	return 0;
}

static const struct of_device_id ltc7820_match_table[] = {
	{
		.compatible = "huawei,ltc7820",
		.data = NULL,
	},
	{},
};

static struct platform_driver ltc7820_driver = {
	.probe = ltc7820_probe,
	.remove = ltc7820_remove,
	.driver = {
		.name = "huawei,ltc7820",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(ltc7820_match_table),
	},
};

static int __init ltc7820_init(void)
{
	return platform_driver_register(&ltc7820_driver);
}

static void __exit ltc7820_exit(void)
{
	platform_driver_unregister(&ltc7820_driver);
}

module_init(ltc7820_init);
module_exit(ltc7820_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ltc7820 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
