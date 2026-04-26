/*
 * soc_mad.c codec driver.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "soc_mad.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include "linux/hisi/audio_log.h"

#include "soc_mad_reg.h"

#define LOG_TAG "soc_mad"

enum reg_type {
	MAD_REG = 0,
	REG_TYPE_NUM,
};

struct mad_priv {
	struct pinctrl *pctrl;
	struct pinctrl_state *pin_default;
	struct pinctrl_state *pin_idle;
	void __iomem *reg_base_addr[REG_TYPE_NUM];
	unsigned int mad_mode;
};

static struct mad_priv mad_data;

static const struct of_device_id mad_match[] = {
	{ .compatible = "hisilicon,soc-mad", },
	{},
};

void soc_mad_set_pinctrl_state(unsigned int mode)
{
	int ret = 0;

	if (mode >= MODE_CNT) {
		AUDIO_LOGE("pinctrl state error: %d", mode);
		return;
	}

	if (mad_data.pctrl == NULL) {
		AUDIO_LOGE("pin ctrl is null");
		return;
	}

	AUDIO_LOGI("set pin state mode: %d", mode);

	if (mode == HIGH_FREQ_MODE)
		ret = pinctrl_select_state(mad_data.pctrl, mad_data.pin_default);
	else if (mode == LOW_FREQ_MODE)
		ret = pinctrl_select_state(mad_data.pctrl, mad_data.pin_idle);
	else
		AUDIO_LOGE("invalid mode: %d", mode);

	if (ret)
		AUDIO_LOGE("set pinctrl state error, ret: %d, mode: %d", ret, mode);
}

void soc_mad_select_din(unsigned int mode)
{
	unsigned int i;
	for (i = MAD_REG; i < REG_TYPE_NUM; i++) {
		if (!mad_data.reg_base_addr[i]) {
			AUDIO_LOGE("reg base addr has not been maped");
			return;
		}
	}

	if (mode == LOW_FREQ_MODE)
		writel(0x0, mad_data.reg_base_addr[MAD_REG] + MAD_MUX_SEL);
	else if(mode == HIGH_FREQ_MODE)
		writel(0x2, mad_data.reg_base_addr[MAD_REG] + MAD_MUX_SEL);
	else
		AUDIO_LOGE("invalid mode: %d", mode);
}

static void mad_pinctrl_deinit(struct platform_device *pdev)
{
	struct mad_priv *priv = platform_get_drvdata(pdev);

	pinctrl_put(priv->pctrl);
	priv->pctrl = NULL;
	priv->pin_default = NULL;
	priv->pin_idle = NULL;

	AUDIO_LOGI("pinctrl deinit ok");
}

static int mad_pinctrl_init(struct platform_device *pdev)
{
	struct pinctrl *pin = NULL;
	struct pinctrl_state *pin_state = NULL;
	struct mad_priv *priv = platform_get_drvdata(pdev);

	pin = pinctrl_get(&pdev->dev);
	if (IS_ERR(pin)) {
		AUDIO_LOGE("can not get pinctrl");
		return -EFAULT;
	}
	priv->pctrl = pin;

	pin_state = pinctrl_lookup_state(pin, PINCTRL_STATE_DEFAULT);
	if (IS_ERR(pin_state)) {
		AUDIO_LOGE("can not get default state (%li)", PTR_ERR(pin_state));
		goto pin_init_err;
	}
	priv->pin_default = pin_state;

	pin_state = pinctrl_lookup_state(pin, PINCTRL_STATE_IDLE);
	if (IS_ERR(pin_state)) {
		AUDIO_LOGE("can not get idle state (%li)", PTR_ERR(pin_state));
		goto pin_init_err;
	}
	priv->pin_idle = pin_state;
	AUDIO_LOGI("pinctrl init ok");

	return 0;

pin_init_err:
	mad_pinctrl_deinit(pdev);

	return -EFAULT;
}

static void mad_reg_resource_deinit(struct platform_device *pdev)
{
	struct mad_priv *priv = platform_get_drvdata(pdev);
	unsigned int i;

	for (i = MAD_REG; i < REG_TYPE_NUM; i++) {
		if (priv->reg_base_addr[i]) {
			iounmap(priv->reg_base_addr[i]);
			priv->reg_base_addr[i] = NULL;
		}
	}
}

static int mad_reg_resource_init(struct platform_device *pdev)
{
	struct resource *res = NULL;
	struct mad_priv *priv = NULL;
	unsigned int i;

	priv = platform_get_drvdata(pdev);
	for (i = MAD_REG; i < REG_TYPE_NUM; i++) {
		res = platform_get_resource(pdev, IORESOURCE_MEM, i);
		if (!res) {
			AUDIO_LOGE("get addr resource error");
			return -EFAULT;
		}

		priv->reg_base_addr[i] = (char * __force)(ioremap(res->start, resource_size(res))); //lint !e446
		if (!priv->reg_base_addr[i]) {
			AUDIO_LOGE("addr ioremap fail");
			goto resource_init_err;
		}
	}

	return 0;

resource_init_err:
	mad_reg_resource_deinit(pdev);
	return -EFAULT;
}

static int mad_probe(struct platform_device *pdev)
{
	int ret;
	unsigned int val;

	AUDIO_LOGI("mad driver probe start");
	platform_set_drvdata(pdev, &mad_data);

	if (!of_property_read_u32(pdev->dev.of_node, "mad_mode", &val)) {
		AUDIO_LOGI("mad mode: %d", val);
		if (val >= MAD_MODE_CNT)
			mad_data.mad_mode = MAD_MODE_INVALID;
		else
			mad_data.mad_mode = val;
	} else {
		AUDIO_LOGE("read mad mode error");
		mad_data.mad_mode = MAD_MODE_INVALID;
	}

	ret = mad_pinctrl_init(pdev);
	if (ret) {
		AUDIO_LOGE("mad pin init failed, ret: %d", ret);
		return ret;
	}

	ret = mad_reg_resource_init(pdev);
	if (ret) {
		mad_pinctrl_deinit(pdev);
		AUDIO_LOGE("mad reg init failed, ret: %d", ret);
		return ret;
	}

	AUDIO_LOGI("mad driver probe ok");
	return 0;
}

static int mad_remove(struct platform_device *pdev)
{
	mad_pinctrl_deinit(pdev);
	mad_reg_resource_deinit(pdev);

	return 0;
}

static struct platform_driver mad_driver = {
	.driver = {
		.name  = "soc-mad",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(mad_match),
	},
	.probe  = mad_probe,
	.remove = mad_remove,
};

static int __init soc_mad_init(void)
{
	AUDIO_LOGI("soc_mad_init");
	return platform_driver_register(&mad_driver);
}

module_init(soc_mad_init);

static void __exit soc_mad_exit(void)
{
	platform_driver_unregister(&mad_driver);
}

module_exit(soc_mad_exit);

MODULE_DESCRIPTION("audio soc mad driver");
MODULE_LICENSE("GPL");

