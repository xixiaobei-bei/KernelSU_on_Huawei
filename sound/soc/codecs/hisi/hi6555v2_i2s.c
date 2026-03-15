/*
 * hi6555v2 i2s pinctrl driver.
 *
 * Copyright (c) 2015 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/printk.h>
#include <linux/pinctrl/consumer.h>

#include "hi6555v2_utility.h"
#include "hi6555v2_i2s.h"

#define LOG_TAG "hi6555v2_i2s"

static struct hi6555v2_i2s_priv hi6555v2_i2s_privdata = {0};

static struct hi6555v2_i2s_priv* _get_privdata(void)
{
	return &hi6555v2_i2s_privdata;
}

static int _set_i2s2_pinctrl_state(enum hi6555v2_pinctrl_state state)
{
	int ret;
	struct hi6555v2_i2s_priv *i2s_priv = _get_privdata();
	struct hi6555v2_i2s2_priv *priv;

	IN_FUNCTION;

	WARN_ON(!i2s_priv);

	priv = i2s_priv->i2s2_priv;

	if (!priv) {
		HI6555V2_LOGE("can not set i2s2 state because priv is NULL\n");
		return -ENODEV;
	}

	switch (state) {
	case STATE_DEFAULT:
		ret = pinctrl_select_state(priv->pctrl, priv->pin_default);
		if (ret) {
			HI6555V2_LOGE("can not set pins to default state, ret=%d\n", ret);
		}
		break;
	case STATE_IDLE:
		ret = pinctrl_select_state(priv->pctrl, priv->pin_idle);
		if (ret) {
			HI6555V2_LOGE("can not set pins to idle state, ret=%d\n", ret);
		}
		break;
	default:
		ret = -EFAULT;
		HI6555V2_LOGE("pinctrl state error:%d\n", state);
		break;
	}

	OUT_FUNCTION;

	return ret;

}

int hi6555v2_i2s2_set_pinctrl_default(void)
{
	int ret;

	IN_FUNCTION;

	ret = _set_i2s2_pinctrl_state(STATE_DEFAULT);
	if (ret) {
		HI6555V2_LOGE("i2s2 pinctrl set default state fail\n");
		return ret;
	}

	HI6555V2_LOGI("i2s2 pinctrl set default state ok\n");

	OUT_FUNCTION;

	return ret;
}

int hi6555v2_i2s2_set_pinctrl_idle(void)
{
	int ret;

	IN_FUNCTION;

	ret = _set_i2s2_pinctrl_state(STATE_IDLE);

	if (ret) {
		HI6555V2_LOGE("i2s2 pinctrl set idle state fail\n");
		return ret;
	}

	HI6555V2_LOGI("i2s2 pinctrl set idle state ok\n");

	OUT_FUNCTION;

	return ret;
}

static int _i2s2_pinctrl_init(struct platform_device *pdev)
{
	struct pinctrl *p;
	struct pinctrl_state *state;
	struct hi6555v2_i2s2_priv *priv;

	IN_FUNCTION;
	priv = platform_get_drvdata(pdev);
	WARN_ON(!priv);

	/* I2S2 iomux config */
	p = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(p)) {
		HI6555V2_LOGE("can not get pinctrl\n");
		return PTR_ERR(p);
	}

	priv->pctrl = p;

	state = pinctrl_lookup_state(p, PINCTRL_STATE_DEFAULT);
	if (IS_ERR(state)) {
		HI6555V2_LOGE("can not get default state (%li)\n", PTR_ERR(state));
		return PTR_ERR(state);
	}

	priv->pin_default = state;

	state = pinctrl_lookup_state(p, PINCTRL_STATE_IDLE);
	if (IS_ERR(state)) {
		HI6555V2_LOGE("can not get idle state (%li)\n", PTR_ERR(state));
		return PTR_ERR(state);
	}

	priv->pin_idle = state;

	HI6555V2_LOGI("pinctrl init ok\n");

	OUT_FUNCTION;

	return 0;
}

static void _i2s2_pinctrl_deinit(struct platform_device *pdev)
{
	struct hi6555v2_i2s2_priv *priv;

	IN_FUNCTION;
	priv = platform_get_drvdata(pdev);
	WARN_ON(!priv);

	devm_pinctrl_put(priv->pctrl);

	HI6555V2_LOGI("pinctrl deinit ok\n");

	OUT_FUNCTION;
}

/*lint -e429*/
static int hi6555v2_i2s2_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct hi6555v2_i2s_priv *i2s_priv = _get_privdata();
	struct hi6555v2_i2s2_priv *priv;

	HI6555V2_LOGI("Begin\n");
	WARN_ON(!i2s_priv);

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		HI6555V2_LOGE("memory alloc failed\n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, priv);

	ret = _i2s2_pinctrl_init(pdev);
	if (ret) {
		HI6555V2_LOGE("pinctrl init failed! err code 0x%x\n", ret);
		return ret;
	}

	i2s_priv->i2s2_priv = priv;

	HI6555V2_LOGI("End\n");
	return ret;
}
/*lint +e429*/

static void hi6555v2_i2s2_shutdown(struct platform_device *pdev)
{
	struct hi6555v2_i2s_priv *priv = _get_privdata();

	HI6555V2_LOGI("Begin\n");
	WARN_ON(!priv);

	_i2s2_pinctrl_deinit(pdev);
	priv->i2s2_priv = NULL;

	HI6555V2_LOGI("End\n");
}


static const struct of_device_id hi6555v2_i2s2_match[] = {
	{ .compatible = "hisilicon,codec-i2s2", },
	{},
};

static struct platform_driver hi6555v2_i2s2_driver = {
	.driver = {
		.name  = "hi6555v2-i2s2",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hi6555v2_i2s2_match),
	},
	.probe  = hi6555v2_i2s2_probe,
	.shutdown = hi6555v2_i2s2_shutdown,
};

static int __init hi6555v2_i2s2_init(void)
{
	HI6555V2_LOGI("Begin\n");
	return platform_driver_register(&hi6555v2_i2s2_driver);
}
module_init(hi6555v2_i2s2_init);

static void __exit hi6555v2_i2s2_exit(void)
{
	HI6555V2_LOGI("Begin\n");
	platform_driver_unregister(&hi6555v2_i2s2_driver);
}
module_exit(hi6555v2_i2s2_exit);

MODULE_DESCRIPTION("hi6555v2 i2s pinctrl driver");
MODULE_AUTHOR("liupan <liupan21@huawei.com>");
MODULE_LICENSE("GPL");

