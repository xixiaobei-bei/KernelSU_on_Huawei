/*
 * wireless_tx_pwm.c
 *
 * pwm driver for wireless reverse charging
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/consumer.h>
#include <huawei_platform/power/wireless/wireless_tx_pwm.h>
#include <chipset_common/hwpower/common_module/power_pwm.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>

#define HWLOG_TAG wireless_tx_pwm
HWLOG_REGIST();

#define WLTX_PWM_ROW 4
#define WLTX_PWM_COL 3

struct wltx_pwm_para {
	u32 volt;
	u32 duty;
	u32 period;
};

struct wltx_pwm_cfg {
	int pwm_level;
	struct wltx_pwm_para pwm_para[WLTX_PWM_ROW];
};

struct wltx_pwm_dev_info {
	struct device *dev;
	struct pwm_device *pwm_dev;
	struct wltx_pwm_cfg pwm_cfg;
	struct pinctrl *pinctrl;
	struct pinctrl_state *pinctrl_idle;
	struct pinctrl_state *pinctrl_default;
	unsigned int type;
};

static struct wltx_pwm_dev_info *g_wltx_pwm_di;

static int wltx_pwm_apply(struct wltx_pwm_dev_info *di, struct pwm_state *state)
{
	int ret;

	if (!di || !di->pinctrl || !di->pinctrl_default) {
		hwlog_err("pwm_apply: pinctrl is null\n");
		return -ENOTSUPP;
	}

	ret = pinctrl_select_state(di->pinctrl, di->pinctrl_default);
	if (ret) {
		hwlog_err("pwm_apply: pinctrl_select_state error\n");
		return -ENOTSUPP;
	}

	return power_pwm_apply_state(di->pwm_dev, state);
}

int wltx_pwm_set_volt(unsigned int ic_type, int vset)
{
	int i;
	struct pwm_state state;
	struct wltx_pwm_dev_info *di = g_wltx_pwm_di;

	if (!di)
		return -ENODEV;

	for (i = 0; i < di->pwm_cfg.pwm_level; i++) {
		if (vset == di->pwm_cfg.pwm_para[i].volt)
			break;
	}
	if (i >= di->pwm_cfg.pwm_level) {
		hwlog_err("pwm_set_volt: out of range\n");
		return -EINVAL;
	}
	hwlog_info("[pwm_set_volt] ic_type=%u vset=%u\n", ic_type, di->pwm_cfg.pwm_para[i].volt);

	state.period = di->pwm_cfg.pwm_para[i].period;
	state.duty_cycle = di->pwm_cfg.pwm_para[i].duty;
	state.polarity = PWM_POLARITY_NORMAL;
	state.enabled = true;

	return wltx_pwm_apply(di, &state);
}

void wltx_pwm_close(unsigned int ic_type)
{
	struct pwm_state state;
	struct wltx_pwm_dev_info *di = g_wltx_pwm_di;

	if (!di)
		return;

	hwlog_info("[pwm_close] ic_type=%u\n", ic_type);
	state.period = 0;
	state.duty_cycle = 0;
	state.polarity = PWM_POLARITY_NORMAL;
	state.enabled = true;
	(void)wltx_pwm_apply(di, &state);
	power_usleep(DT_USLEEP_5MS); /* for stable pwm */
	state.enabled = false;
	(void)wltx_pwm_apply(di, &state);
	if (pinctrl_select_state(di->pinctrl, di->pinctrl_idle) < 0)
		hwlog_err("pwm_close: set pins to idle state failed\n");
}

static int wltx_pwm_get_pinctrl(struct wltx_pwm_dev_info *di)
{
	di->pinctrl = pinctrl_get(di->dev);
	if (IS_ERR(di->pinctrl)) {
		hwlog_err("pwm_get_pinctrl: pinctrl_get failed\n");
		return -ENOTSUPP;
	}

	di->pinctrl_idle = pinctrl_lookup_state(di->pinctrl, PINCTRL_STATE_IDLE);
	if (IS_ERR(di->pinctrl_idle)) {
		hwlog_err("pwm_get_pinctrl: get idle state failed\n");
		return -ENOTSUPP;
	}

	di->pinctrl_default = pinctrl_lookup_state(di->pinctrl, PINCTRL_STATE_DEFAULT);
	if (IS_ERR(di->pinctrl_default)) {
		hwlog_err("pwm_get_pinctrl: get default state failed\n");
		return -ENOTSUPP;
	}

	return 0;
}

static int wltx_pwm_request(struct wltx_pwm_dev_info *di)
{
	di->pwm_dev = power_pwm_get_dev(di->dev, NULL);
	if (!di->pwm_dev)
		return -ENODEV;

	if (wltx_pwm_get_pinctrl(di)) {
		power_pwm_put(di->pwm_dev);
		return -ENODEV;
	}

	return 0;
}

static int wltx_pwm_parse_para(struct device_node *np, struct wltx_pwm_dev_info *di)
{
	int i;
	int arr_len;

	arr_len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"pwm_vset", (int *)di->pwm_cfg.pwm_para, WLTX_PWM_ROW, WLTX_PWM_COL);
	if (arr_len <= 0) {
		di->pwm_cfg.pwm_level = 0;
		return -EINVAL;
	}

	di->pwm_cfg.pwm_level = arr_len / WLTX_PWM_COL;
	for (i = 0; i < di->pwm_cfg.pwm_level; i++)
		hwlog_info("wltx pwm_para[%d] volt:%u duty:%u period:%u\n", i,
			di->pwm_cfg.pwm_para[i].volt, di->pwm_cfg.pwm_para[i].duty,
			di->pwm_cfg.pwm_para[i].period);

	return 0;
}

static int wltx_pwm_probe(struct platform_device *pdev)
{
	int ret;
	struct wltx_pwm_dev_info *di = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_wltx_pwm_di = di;
	di->dev = &pdev->dev;
	np = di->dev->of_node;
	platform_set_drvdata(pdev, di);

	ret = wltx_pwm_parse_para(np, di);
	if (ret)
		goto pwm_parse_fail;

	ret = wltx_pwm_request(di);
	if (ret)
		goto pwm_request_fail;

	return 0;

pwm_request_fail:
pwm_parse_fail:
	devm_kfree(&pdev->dev, di);
	g_wltx_pwm_di = NULL;
	platform_set_drvdata(pdev, NULL);
	return ret;
}

static int wltx_pwm_remove(struct platform_device *pdev)
{
	struct wltx_pwm_dev_info *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	power_pwm_put(di->pwm_dev);
	devm_kfree(&pdev->dev, di);
	g_wltx_pwm_di = NULL;
	return 0;
}

static const struct of_device_id wltx_pwm_match_table[] = {
	{
		.compatible = "huawei,wltx_pwm",
		.data = NULL,
	},
	{},
};

static struct platform_driver wltx_pwm_driver = {
	.probe = wltx_pwm_probe,
	.remove = wltx_pwm_remove,
	.driver = {
		.name = "huawei,wltx_pwm",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(wltx_pwm_match_table),
	},
};

static int __init wltx_pwm_init(void)
{
	return platform_driver_register(&wltx_pwm_driver);
}

static void __exit wltx_pwm_exit(void)
{
	platform_driver_unregister(&wltx_pwm_driver);
}

device_initcall_sync(wltx_pwm_init);
module_exit(wltx_pwm_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wireless pwm module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
