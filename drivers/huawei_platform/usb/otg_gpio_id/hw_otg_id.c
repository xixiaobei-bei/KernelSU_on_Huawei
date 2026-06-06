/*
 * hw_otg_id.c
 *
 * gpio based for otgid driver
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
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/wakelock.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/delay.h>
#include <linux/of_address.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include <linux/of_gpio.h>
#include <linux/hisi/usb/hisi_usb.h>

#include <linux/mfd/hisi_pmic.h>
#include <pmic_interface.h>
#include <linux/hisi/hisi_adc.h>
#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_HUAWEI_YCABLE
#include <huawei_platform/usb/hw_ycable.h>
#endif
#include "hw_otg_id.h"

#define HWLOG_TAG otgid
HWLOG_REGIST();

static struct otg_gpio_id_dev *g_otg_gpio_id_dev;
static int g_startup_otg_sample;

#define DISABLE_USB_IRQ             0
#define FAIL                        (-1)
#define SAMPLING_TIME_OPTIMIZE_FLAG 1
#define SAMPLING_TIME_INTERVAL      10
#define SMAPLING_TIME_OPTIMIZE      5
#define VBATT_AVR_MAX_COUNT         10
#define ADC_VOLTAGE_LIMIT           150  /* unit mv */
#define ADC_VOLTAGE_MAX             1250 /* unit mv */
#define ADC_VOLTAGE_NEGATIVE        2000 /* unit mv */
#define USB_CHARGER_INSERTED        1
#define USB_CHARGER_REMOVE          0

int hw_get_otg_id_gpio_value(int *gpio_value)
{
	struct otg_gpio_id_dev *di = g_otg_gpio_id_dev;

	if (!di || !gpio_value) {
		hwlog_err("di or gpio_value is null\n");
		return -1;
	}

	*gpio_value = gpio_get_value(di->gpio);

	return 0;
}

static int hw_is_usb_cable_connected(void)
{
	struct otg_gpio_id_dev *di = g_otg_gpio_id_dev;
	int ret = 0;

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	if (di->fpga_flag == 0) {
#ifdef CONFIG_DEC_USB
		ret = (hisi_pmic_reg_read(PMIC_STATUS0_ADDR(0)) &
			(VBUS4P3_D10 | VBUS_COMP_VBAT_D20));
#else
		ret = hisi_pmic_get_vbus_status();
#endif /* CONFIG_DEC_USB */
	}

	hwlog_info("vbus_status=%d\n", ret);
	return ret;
}

static int hw_otg_id_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct otg_gpio_id_dev *di = g_otg_gpio_id_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return NOTIFY_OK;
	}

#ifdef CONFIG_DEC_USB
	if (event == DISABLE_USB_IRQ && di->otg_irq_enabled) {
		hwlog_info("disable the otg irq\n");
		disable_irq_nosync(di->irq);
		di->otg_irq_enabled = false;
	} else if (event != DISABLE_USB_IRQ && !di->otg_irq_enabled) {
		hwlog_info("enable the otg irq\n");
		enable_irq(di->irq);
		di->otg_irq_enabled = true;
	} else {
		hwlog_info("event %u when irq state %d\n",
			event, di->otg_irq_enabled);
	}
#else
	switch (event) {
	case CHARGER_TYPE_NONE:
		if (!di->otg_irq_enabled) {
			hwlog_info("enable the otg irq\n");
			enable_irq(di->irq);
			di->otg_irq_enabled = true;
		}
		break;

	/* fall-through */
	case CHARGER_TYPE_SDP:
	case CHARGER_TYPE_CDP:
	case CHARGER_TYPE_DCP:
	case CHARGER_TYPE_UNKNOWN:
		if (di->otg_irq_enabled) {
			hwlog_info("disable the otg irq\n");
			disable_irq_nosync(di->irq);
			di->otg_irq_enabled = false;
		}
		break;

	default:
		hwlog_info("ignore other event %ld when irq state %d\n",
			event, di->otg_irq_enabled);
		break;
	}
#endif /* CONFIG_DEC_USB */

	return NOTIFY_OK;
}

static int hw_otg_id_adc_sampling(struct otg_gpio_id_dev *di)
{
	int avgvalue;
	int vol_value;
	int sum = 0;
	int i;
	int sample_cnt = 0;

	/* get adc channel */
	if (di->otg_adc_channel == 0) {
		hwlog_info("get otg_adc_channel fail\n");
		return FAIL;
	}

	for (i = 0; i < VBATT_AVR_MAX_COUNT; i++) {
		vol_value = hisi_adc_get_value(di->otg_adc_channel);
		/* add some interval */
		if (di->sampling_time_optimize == SAMPLING_TIME_OPTIMIZE_FLAG &&
			g_startup_otg_sample == SAMPLE_DOING)
			udelay(SMAPLING_TIME_OPTIMIZE);
		else
			msleep(SAMPLING_TIME_INTERVAL);

		if (vol_value < 0) {
			hwlog_info("the value from adc is error\n");
			continue;
		}

		sum += vol_value;
		sample_cnt++;
	}

	if (sample_cnt == 0) {
		/* ESD cause the sample is always negative */
		avgvalue = ADC_VOLTAGE_NEGATIVE;
	} else {
		avgvalue = sum / sample_cnt;
	}

	hwlog_info("the average voltage of adc is %d\n", avgvalue);

	return avgvalue;
}

static void hw_otg_id_intb_work(struct work_struct *work)
{
	int gpio_value;
	int avgvalue;
	static bool is_otg_has_inserted;
	struct otg_gpio_id_dev *di = g_otg_gpio_id_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	/* fix the different of schager V200 and V300 */
	if (!is_otg_has_inserted) {
		if (hw_is_usb_cable_connected()) {
			hwlog_info("vbus is inserted\n");
#ifdef CONFIG_HUAWEI_YCABLE
			avgvalue = hw_otg_id_adc_sampling(di);
			if (!ycable_is_charge_connect(avgvalue))
				return;
#else
			return;
#endif /* CONFIG_HUAWEI_YCABLE */
		}
	}

	gpio_value = gpio_get_value(di->gpio);
	if (gpio_value == 0) {
		avgvalue = hw_otg_id_adc_sampling(di);
		hwlog_info("send ID_FALL_EVENT avgvalue=%d\n", avgvalue);

		if ((avgvalue >= 0) && (avgvalue <= ADC_VOLTAGE_LIMIT)) {
			is_otg_has_inserted = true;
			hisi_usb_otg_event(ID_FALL_EVENT);
#ifdef CONFIG_HUAWEI_YCABLE
			ycable_handle_otg_event(ID_FALL_EVENT,
				!YCABLE_NEED_WAIT);
#endif /* CONFIG_HUAWEI_YCABLE */
		} else {
			is_otg_has_inserted = true;
#ifdef CONFIG_HUAWEI_YCABLE
			if (ycable_is_charge_connect(avgvalue)) {
				/*
				 * last cable plug out will call completion,
				 * so here reinit it
				 */
				ycable_init_devoff_completion();
				hisi_usb_otg_event(CHARGER_DISCONNECT_EVENT);
				hisi_usb_otg_event(ID_FALL_EVENT);
				ycable_handle_otg_event(ID_FALL_EVENT,
					YCABLE_NEED_WAIT);
			}
#endif /* CONFIG_HUAWEI_YCABLE */
		}
	} else {
		hwlog_info("send ID_RISE_EVENT\n");
		is_otg_has_inserted = false;
		hisi_usb_otg_event(ID_RISE_EVENT);
	}
}

static irqreturn_t hw_otg_id_irq_handle(int irq, void *dev_id)
{
	struct otg_gpio_id_dev *di = g_otg_gpio_id_dev;
	int gpio_value;

	if (!di) {
		hwlog_err("di is null\n");
		return IRQ_HANDLED;
	}

	disable_irq_nosync(di->irq);

	gpio_value = gpio_get_value(di->gpio);
	if (gpio_value == 0)
		irq_set_irq_type(di->irq, IRQF_TRIGGER_HIGH | IRQF_NO_SUSPEND);
	else
		irq_set_irq_type(di->irq, IRQF_TRIGGER_LOW | IRQF_NO_SUSPEND);

	enable_irq(di->irq);
	schedule_work(&di->otg_intb_work);

	return IRQ_HANDLED;
}

static int hw_otg_id_parse_dts(struct otg_gpio_id_dev *di,
	struct device_node *np)
{
	int ret = -1;

	ret = of_property_read_u32(np, "sampling_time_optimize",
		&(di->sampling_time_optimize));
	if (ret)
		hwlog_err("sampling_time_optimize dts read failed\n");

	hwlog_info("sampling_time_optimize=%d\n", di->sampling_time_optimize);

	ret = of_property_read_u32(np, "fpga_flag",
		&(di->fpga_flag));
	if (ret) {
		hwlog_err("fpga_flag dts read failed\n");
		di->fpga_flag = 0;
	}
	hwlog_info("fpga_flag=%d\n", di->fpga_flag);

	ret = of_property_read_u32(np, "otg_adc_channel",
		&(di->otg_adc_channel));
	if (ret) {
		hwlog_err("otg_adc_channel dts read failed\n");
		return ret;
	}
	hwlog_info("otg_adc_channel=%d\n", di->otg_adc_channel);

	return ret;
}

static int hw_otg_id_probe(struct platform_device *pdev)
{
	int ret;
	int avgvalue;
	int adc_voltage_min = ADC_VOLTAGE_LIMIT;
	struct device_node *np = NULL;
	struct device *dev = NULL;
	struct otg_gpio_id_dev *di = NULL;

	hwlog_info("probe begin\n");

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_otg_gpio_id_dev = di;
	platform_set_drvdata(pdev, di);

	di->pdev = pdev;
	dev = &pdev->dev;
	np = dev->of_node;

	ret = hw_otg_id_parse_dts(di, np);
	if (ret)
		goto fail_parse_dts;

	di->otg_irq_enabled = true;
	di->otg_nb.notifier_call = hw_otg_id_notifier_call;

#ifdef CONFIG_DEC_USB
	ret = hisi_usb_otg_irq_notifier_register(&di->otg_nb);
#else
	ret = hisi_charger_type_notifier_register(&di->otg_nb);
#endif /* CONFIG_DEC_USB */

	if (ret) {
		hwlog_err("charger_type_notifier register failed\n");
		goto fail_parse_dts;
	}

	INIT_WORK(&di->otg_intb_work, hw_otg_id_intb_work);

	di->gpio = of_get_named_gpio(np, "otg-gpio", 0);
	hwlog_info("gpio=%d\n", di->gpio);

	if (!gpio_is_valid(di->gpio)) {
		hwlog_err("gpio is not valid\n");
		ret = -EINVAL;
		goto fail_register_notifier;
	}

	ret = gpio_request(di->gpio, "otg_gpio_irq");
	if (ret < 0) {
		hwlog_err("gpio request fail\n");
		goto fail_register_notifier;
	}

	avgvalue = hw_otg_id_adc_sampling(di);
	g_startup_otg_sample = SAMPLE_DONE;
#ifdef CONFIG_HUAWEI_YCABLE
	if (ycable_is_support())
		adc_voltage_min = ycable_get_gpio_adc_min();
#endif /* CONFIG_HUAWEI_YCABLE */

	hwlog_info("avgvalue=%d, adc_min=%d\n", avgvalue, adc_voltage_min);

	if ((avgvalue > adc_voltage_min) && (avgvalue <= ADC_VOLTAGE_MAX)) {
		ret = gpio_direction_output(di->gpio, 1); /* 1: high */
		if (ret) {
			hwlog_err("gpio set output fail\n");
			goto fail_free_gpio;
		}
	} else {
		ret = gpio_direction_input(di->gpio);
		if (ret) {
			hwlog_err("gpio set input fail\n");
			goto fail_free_gpio;
		}
	}

	di->irq = gpio_to_irq(di->gpio);
	if (di->irq < 0) {
		hwlog_err("gpio map to irq fail\n");
		ret = -EINVAL;
		goto fail_free_gpio;
	}

	if (hw_is_usb_cable_connected())
		hw_otg_id_notifier_call(NULL, !USB_CHARGER_INSERTED, NULL);
	else
		hw_otg_id_notifier_call(NULL, !USB_CHARGER_REMOVE, NULL);

	ret = request_irq(di->irq, hw_otg_id_irq_handle,
		IRQF_TRIGGER_LOW | IRQF_NO_SUSPEND | IRQF_ONESHOT,
		"otg_gpio_irq", NULL);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		goto fail_free_gpio;
	}

	if (hw_is_usb_cable_connected()) {
#ifdef CONFIG_DEC_USB
		hw_otg_id_notifier_call(NULL, !USB_CHARGER_INSERTED, NULL);
#else
		hw_otg_id_notifier_call(NULL, CHARGER_TYPE_SDP, NULL);
#endif /* CONFIG_DEC_USB */
	}

	/* check the otg status when the phone poweron */
	ret = gpio_get_value(di->gpio);
	if (ret == 0)
		schedule_work(&di->otg_intb_work);

	hwlog_info("probe end\n");
	return 0;

fail_free_gpio:
	gpio_free(di->gpio);
fail_register_notifier:
#ifdef CONFIG_DEC_USB
	hisi_usb_otg_irq_notifier_unregister(&di->otg_nb);
#else
	hisi_charger_type_notifier_unregister(&di->otg_nb);
#endif /* CONFIG_DEC_USB */
fail_parse_dts:
	devm_kfree(&pdev->dev, di);
	g_otg_gpio_id_dev = NULL;

	return ret;
}

static int hw_otg_id_remove(struct platform_device *pdev)
{
	struct otg_gpio_id_dev *di = g_otg_gpio_id_dev;

	hwlog_info("remove begin\n");

	if (!di)
		return -ENODEV;

#ifdef CONFIG_DEC_USB
	hisi_usb_otg_irq_notifier_unregister(&di->otg_nb);
#else
	hisi_charger_type_notifier_unregister(&di->otg_nb);
#endif /* CONFIG_DEC_USB */

	free_irq(di->irq, pdev);
	gpio_free(di->gpio);
	devm_kfree(&pdev->dev, di);
	g_otg_gpio_id_dev = NULL;

	hwlog_info("remove end\n");
	return 0;
}


static const struct of_device_id hw_otg_id_of_match[] = {
	{
		.compatible = "huawei,usbotg-by-id",
		.data = NULL,
	},
	{},
};

static struct platform_driver hw_otg_id_drv = {
	.probe = hw_otg_id_probe,
	.remove = hw_otg_id_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "hw_otg_id",
		.of_match_table = hw_otg_id_of_match,
	},
};

static int __init hw_otg_id_init(void)
{
	return platform_driver_register(&hw_otg_id_drv);
}

static void __exit hw_otg_id_exit(void)
{
	platform_driver_unregister(&hw_otg_id_drv);
}

device_initcall_sync(hw_otg_id_init);
module_exit(hw_otg_id_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("gpio based for otgid driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
