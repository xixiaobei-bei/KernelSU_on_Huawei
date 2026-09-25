/*
 * opa2333p.c
 *
 * opa2333p driver
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/hisi/hisi_adc.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/direct_charger/direct_charger.h>
#include <huawei_platform/power/battery_voltage.h>
#include <huawei_platform/power/power_gpio.h>
#include "opa2333p.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG opa2333p
HWLOG_REGIST();

static struct opa2333p_device_info *g_opa2333p_dev;

static int opa2333p_get_hkadc_value_oneshot(unsigned int adc_channel)
{
	struct opa2333p_device_info *di = g_opa2333p_dev;
	int adc_value;
	int i;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	for (i = 0; i < HKADC_RETRY_TIMES; i++) {
		adc_value = hisi_adc_get_adc(adc_channel);

		if (adc_value < 0)
			hwlog_err("hisi adc read fail\n");
		else
			break;
	}

	return adc_value;
}

static int opa2333p_get_hkadc_value(unsigned int adc_channel)
{
	int sum = 0;
	int cnt = 0;
	int adc_value;
	int i;

	/* for adc debounce, 5: sampling times */
	for (i = 0; i <= 5; i++) {
		adc_value = opa2333p_get_hkadc_value_oneshot(adc_channel);
		if (adc_value >= 0) {
			sum += adc_value;
			cnt++;
		}
	}

	if (!cnt)
		return -EIO;

	return sum / cnt;
}

static int opa2333p_get_bus_voltage_mv(int *val)
{
	struct opa2333p_device_info *di = g_opa2333p_dev;
	int adc_value;

	if (!di || !val) {
		hwlog_err("di or val is null\n");
		return -1;
	}

	mutex_lock(&di->ntc_switch_lock);
	gpio_set_value(di->gpio_ntc_switch, HKADC_IN13_VBUS);
	adc_value = opa2333p_get_hkadc_value(di->adc_channel);
	mutex_unlock(&di->ntc_switch_lock);

	if (adc_value < 0) {
		hwlog_err("read adc_value[%d] fail\n", adc_value);
		return -EIO;
	}

	*val = adc_value * di->coef_vbus / HKADC_COEF_MULTIPLE;
	hwlog_info("adc_value=%d,vbus=%d\n", adc_value, *val);

	return 0;
}

static int opa2333p_get_current_ma(int *val)
{
	struct opa2333p_device_info *di = g_opa2333p_dev;
	int adc_value;

	if (!di || !val) {
		hwlog_err("di or val is null\n");
		return -1;
	}

	if (!di->channel_switch_en) {
		*val = 0;
		return 0;
	}

	mutex_lock(&di->ntc_switch_lock);
	gpio_set_value(di->gpio_ntc_switch, HKADC_IN13_IBUS);
	adc_value = opa2333p_get_hkadc_value(di->adc_channel);
	mutex_unlock(&di->ntc_switch_lock);

	if (adc_value < 0) {
		hwlog_err("read adc_value[%d] fail\n", adc_value);
		return -EIO;
	}

	*val = adc_value * di->coef_ibus / HKADC_COEF_MULTIPLE;
	hwlog_info("adc_value=%d,ibus=%d\n", adc_value, *val);

	return 0;
}

static int opa2333p_get_vbat_mv(void)
{
	return hw_battery_voltage(BAT_ID_MAX);
}

static int opa2333p_get_ibat_ma(int *val)
{
	if (!val)
		return -1;

	*val = -hisi_battery_current();

	return 0;
}

static int opa2333p_get_device_temp(int *temp)
{
	if (!temp)
		return -1;

	*temp = DEVICE_DEFAULT_TEMP;

	return 0;
}

static int opa2333p_gpio_init(void)
{
	struct opa2333p_device_info *di = g_opa2333p_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	gpio_set_value(di->gpio_cur_det, CURRENT_DET_DISABLE);

	return 0;
}

static int opa2333p_batinfo_exit(void)
{
	struct opa2333p_device_info *di = g_opa2333p_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	gpio_set_value(di->gpio_cur_det, CURRENT_DET_DISABLE);

	return 0;
}

static struct batinfo_ops opa2333p_batinfo_ops = {
	.init = opa2333p_gpio_init,
	.exit = opa2333p_batinfo_exit,
	.get_bat_btb_voltage = opa2333p_get_vbat_mv,
	.get_bat_package_voltage = opa2333p_get_vbat_mv,
	.get_vbus_voltage = opa2333p_get_bus_voltage_mv,
	.get_bat_current = opa2333p_get_ibat_ma,
	.get_ls_ibus = opa2333p_get_current_ma,
	.get_ls_temp = opa2333p_get_device_temp,
};

static void opa2333p_int_en(bool en)
{
	struct opa2333p_device_info *di = g_opa2333p_dev;
	unsigned long flags;

	if (!di || di->irq_int <= 0) {
		hwlog_err("di is null or irq_int invalid\n");
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

static int opa2333p_recv_dc_status_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct opa2333p_device_info *di = NULL;
	int vol = 0;

	if (!nb) {
		hwlog_err("nb is null\n");
		return NOTIFY_OK;
	}

	di = container_of(nb, struct opa2333p_device_info, nb);
	if (!di) {
		hwlog_err("di is null\n");
		return NOTIFY_OK;
	}

	switch (event) {
	case LVC_STATUS_CHARGING:
		hwlog_info("direct_charge_lvc charging\n");
		di->channel_switch_en = true;
		gpio_set_value(di->gpio_cur_det, CURRENT_DET_DISABLE);
		opa2333p_int_en(false);
		break;
	case SC_STATUS_CHARGING:
		hwlog_info("direct_charge_sc charging\n");
		if (di->under_current_detect) {
			/* switch adc path to voltage */
			(void)opa2333p_get_bus_voltage_mv(&vol);
			gpio_set_value(di->gpio_cur_det, CURRENT_DET_ENABLE);
			di->channel_switch_en = false;
			opa2333p_int_en(true);
		} else {
			gpio_set_value(di->gpio_cur_det, CURRENT_DET_DISABLE);
		}
		break;
	case DC_STATUS_STOP_CHARGE:
		hwlog_info("direct_charge stop charge\n");
		gpio_set_value(di->gpio_cur_det, CURRENT_DET_DISABLE);
		if (di->under_current_detect) {
			di->channel_switch_en = true;
			opa2333p_int_en(false);
		}
		break;
	default:
		hwlog_info("unknow direct_charge notify\n");
		break;
	}

	return NOTIFY_OK;
}

static irqreturn_t opa2333p_interrupt(int irq, void *data)
{
	hwlog_info("current detect int\n");

	opa2333p_int_en(false);

	return IRQ_HANDLED;
}

static void opa2333p_interrupt_parse(struct opa2333p_device_info *di,
	struct device_node *np)
{
	if (power_gpio_config_interrupt(power_gpio_tag(HWLOG_TAG), np,
		"gpio_int", "opa2333p_int", &di->gpio_int, &di->irq_int))
		return;

	if (request_irq(di->irq_int, opa2333p_interrupt,
		IRQF_TRIGGER_LOW, "opa2333p_int_irq", di)) {
		di->irq_int = -1;
		gpio_free(di->gpio_int);
	}
}

static int opa2333p_prase_dts(struct opa2333p_device_info *di,
	struct device_node *np)
{
	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "adc_channel",
		&di->adc_channel, HKADC_DEFAULT_CHANNEL))
		return -EINVAL;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "coef_ibus",
		&di->coef_ibus, HKADC_DEFAULT_IBUS))
		return -EINVAL;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "coef_vbus",
		&di->coef_vbus, HKADC_DEFAULT_VBUS))
		return -EINVAL;

	if (power_gpio_config_output(power_gpio_tag(HWLOG_TAG), np,
		"gpio_cur_det", "gpio_cur_det",
		&di->gpio_cur_det, CURRENT_DET_DISABLE))
		return -EINVAL;

	if (power_gpio_config_output(power_gpio_tag(HWLOG_TAG), np,
		"gpio_ntc_switch", "gpio_ntc_switch",
		&di->gpio_ntc_switch, HKADC_IN13_IBUS)) {
		gpio_free(di->gpio_cur_det);
		return -EINVAL;
	}

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"under_current_detect", &di->under_current_detect, 0);

	return 0;
}

static int opa2333p_probe(struct platform_device *pdev)
{
	int ret;
	struct opa2333p_device_info *di = NULL;
	struct device_node *np = NULL;

	hwlog_info("probe begin\n");

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_opa2333p_dev = di;
	di->dev = &pdev->dev;
	di->channel_switch_en = true;
	np = di->dev->of_node;
	mutex_init(&di->ntc_switch_lock);

	if (opa2333p_prase_dts(di, np))
		return -EINVAL;

	if (di->under_current_detect) {
		opa2333p_interrupt_parse(di, np);
		di->is_int_en = true;
		spin_lock_init(&di->int_lock);
		opa2333p_int_en(false);
	}

	ret = lvc_batinfo_ops_register(&opa2333p_batinfo_ops);
	if (ret) {
		hwlog_err("opa2333p batinfo ops register fail\n");
		goto prase_dts_fail;
	}

	ret = sc_batinfo_ops_register(&opa2333p_batinfo_ops);
	if (ret) {
		hwlog_err("opa2333p batinfo ops register fail\n");
		goto prase_dts_fail;
	}

	di->nb.notifier_call = opa2333p_recv_dc_status_notifier_call;
	ret = direct_charge_notifier_chain_register(&di->nb);
	if (ret) {
		hwlog_err("register scp_charge_stage notifier failed\n");
		goto prase_dts_fail;
	}

	platform_set_drvdata(pdev, di);
	hwlog_info("probe end\n");
	return 0;

prase_dts_fail:
	gpio_free(di->gpio_cur_det);
	gpio_free(di->gpio_ntc_switch);
	return ret;
}

static int opa2333p_remove(struct platform_device *pdev)
{
	struct opa2333p_device_info *di = platform_get_drvdata(pdev);

	hwlog_info("remove begin\n");

	if (!di)
		return -ENODEV;

	if (di->gpio_cur_det)
		gpio_free(di->gpio_cur_det);

	if (di->gpio_ntc_switch)
		gpio_free(di->gpio_ntc_switch);

	if (di->irq_int > 0)
		free_irq(di->irq_int, di);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	direct_charge_notifier_chain_unregister(&di->nb);
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_opa2333p_dev = NULL;

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id opa2333p_of_match[] = {
	{
		.compatible = "huawei,opa2333p",
		.data = NULL,
	},
	{},
};

static struct platform_driver opa2333p_driver = {
	.probe = opa2333p_probe,
	.remove = opa2333p_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "huawei_opa2333p",
		.of_match_table = of_match_ptr(opa2333p_of_match),
	},
};

static int __init opa2333p_init(void)
{
	return platform_driver_register(&opa2333p_driver);
}

static void __exit opa2333p_exit(void)
{
	platform_driver_unregister(&opa2333p_driver);
}

module_init(opa2333p_init);
module_exit(opa2333p_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("opa2333p module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
