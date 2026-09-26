/*
 * da_separate_i2s.c codec driver.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "da_separate_i2s.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include "linux/hisi/audio_log.h"

#define LOG_TAG "DA_separate_i2s"

enum da_separate_hub_pinctrl_state {
	STATE_DEFAULT = 0,
	STATE_IDLE,
};

struct i2s2_priv {
	struct pinctrl *pctrl;
	struct pinctrl_state *pin_default;
	struct pinctrl_state *pin_idle;
};

struct i2s_priv {
	struct i2s2_priv *i2s2_priv;
};

static struct i2s_priv i2s_privdata = { 0 };

static int set_i2s2_pinctrl_state(enum da_separate_hub_pinctrl_state state)
{
	int ret;
	struct i2s_priv *i2s_priv = &i2s_privdata;
	struct i2s2_priv *priv = i2s_priv->i2s2_priv;

	IN_FUNCTION;

	if (priv == NULL) {
		AUDIO_LOGE("can not set i2s2 state because priv is NULL");
		return -ENODEV;
	}

	switch (state) {
	case STATE_DEFAULT:
		ret = pinctrl_select_state(priv->pctrl, priv->pin_default);
		if (ret)
			AUDIO_LOGE("can not set pins to default state, ret: %d", ret);
		break;
	case STATE_IDLE:
		ret = pinctrl_select_state(priv->pctrl, priv->pin_idle);
		if (ret)
			AUDIO_LOGE("can not set pins to idle state, ret: %d", ret);
		break;
	default:
		ret = -EFAULT;
		AUDIO_LOGE("pinctrl state error: %d", state);
		break;
	}

	OUT_FUNCTION;

	return ret;

}

int da_separate_i2s_set_pinctrl_default_for_i2s2(void)
{
	int ret = set_i2s2_pinctrl_state(STATE_DEFAULT);

	IN_FUNCTION;

	if (ret) {
		AUDIO_LOGE("i2s2 pinctrl set default state fail");
		return ret;
	}

	AUDIO_LOGI("i2s2 pinctrl set default state ok");

	OUT_FUNCTION;

	return ret;
}

int da_separate_i2s_set_pinctrl_idle_for_i2s2(void)
{
	int ret = set_i2s2_pinctrl_state(STATE_IDLE);

	IN_FUNCTION;

	if (ret) {
		AUDIO_LOGE("i2s2 pinctrl set idle state fail");
		return ret;
	}

	AUDIO_LOGI("i2s2 pinctrl set idle state ok");

	OUT_FUNCTION;

	return ret;
}

static int i2s2_pinctrl_init(struct platform_device *pdev)
{
	struct pinctrl *p = NULL;
	struct pinctrl_state *state = NULL;
	struct i2s2_priv *priv = platform_get_drvdata(pdev);

	IN_FUNCTION;
	/* I2S2 iomux config */
	p = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(p)) {
		AUDIO_LOGE("can not get pinctrl");
		return PTR_ERR(p);
	}

	priv->pctrl = p;

	state = pinctrl_lookup_state(p, PINCTRL_STATE_DEFAULT);
	if (IS_ERR(state)) {
		AUDIO_LOGE("can not get default state (%li)", PTR_ERR(state));
		return PTR_ERR(state);
	}

	priv->pin_default = state;

	state = pinctrl_lookup_state(p, PINCTRL_STATE_IDLE);
	if (IS_ERR(state)) {
		AUDIO_LOGE("can not get idle state (%li)", PTR_ERR(state));
		return PTR_ERR(state);
	}

	priv->pin_idle = state;

	AUDIO_LOGI("pinctrl init ok");

	OUT_FUNCTION;

	return 0;
}

static void i2s2_pinctrl_deinit(struct platform_device *pdev)
{
	struct i2s2_priv *priv = platform_get_drvdata(pdev);

	IN_FUNCTION;

	devm_pinctrl_put(priv->pctrl);
	priv->pctrl = NULL;
	priv->pin_default = NULL;
	priv->pin_idle = NULL;

	AUDIO_LOGI("pinctrl deinit ok");

	OUT_FUNCTION;
}

/*lint -e429*/
static int da_separate_i2s2_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct i2s_priv *i2s_priv = &i2s_privdata;
	struct i2s2_priv *priv = NULL;

	AUDIO_LOGI("Begin");

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (priv == NULL) {
		AUDIO_LOGE("memory alloc failed");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, priv);

	ret = i2s2_pinctrl_init(pdev);
	if (ret) {
		AUDIO_LOGE("pinctrl init failed! err code: %d", ret);
		return ret;
	}

	i2s_priv->i2s2_priv = priv;

	AUDIO_LOGI("End");

	return ret;
}

/*lint +e429*/
static void da_separate_i2s2_shutdown(struct platform_device *pdev)
{
	struct i2s_priv *priv = &i2s_privdata;

	AUDIO_LOGI("Begin");

	i2s2_pinctrl_deinit(pdev);
	priv->i2s2_priv = NULL;

	AUDIO_LOGI("End");
}


static const struct of_device_id da_separate_i2s2_match[] = {
	{ .compatible = "hisilicon,codec-i2s2", },
	{},
};

static struct platform_driver da_separate_i2s2_driver = {
	.driver = {
		.name  = "da_separate-i2s2",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(da_separate_i2s2_match),
	},
	.probe  = da_separate_i2s2_probe,
	.shutdown = da_separate_i2s2_shutdown,
};

static int __init da_separate_i2s2_init(void)
{
	AUDIO_LOGI("Begin");

	return platform_driver_register(&da_separate_i2s2_driver);
}
module_init(da_separate_i2s2_init);

static void __exit da_separate_i2s2_exit(void)
{
	AUDIO_LOGI("Begin");
	platform_driver_unregister(&da_separate_i2s2_driver);
}
module_exit(da_separate_i2s2_exit);

MODULE_DESCRIPTION("ASoC da_separatex serise driver");
MODULE_LICENSE("GPL");

