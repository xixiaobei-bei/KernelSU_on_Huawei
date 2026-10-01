/*
 * hi6555v2_driver codec driver.
 *
 * Copyright (c) 2015 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/clk.h>

#include "hi6555v2.h"

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
#include "hicodec_debug.h"
#endif

#define LOG_TAG "hi6555v2"

/* HI6555V2 REGISTER BASE ADDR */
static void __iomem *reg_base_addr[HI6555V2_REG_CNT];

static const struct hi6555v2_reg_page reg_page_array[] = {
	{PAGE_SoCCODEC, HI6555V2_SoCCODEC_START, 	HI6555V2_SoCCODEC_END, 	"PAGE_SoCCODEC"},
	{PAGE_ASPCFG, 	HI6555V2_ASPCFG_START, 		HI6555V2_ASPCFG_END, 	"PAGE_ASPCFG"},
	{PAGE_AO_IOC, 	HI6555V2_AOIOC_START, 		HI6555V2_AOIOC_END, 	"PAGE_AO_IOC"},
	{PAGE_PMU_CODEC,HI6555V2_PMUCODEC_START, 	HI6555V2_PMUCODEC_END, 	"PAGE_PMU_CODEC"},
	{PAGE_VIRCODEC, HI6555V2_VIRCODEC_START, 	HI6555V2_VIRCODEC_END, 	"PAGE_VIRCODEC"},
};

static bool _reg_value_valid(struct hi6555v2_priv *priv,
									unsigned int reg_type, unsigned int reg_value)
{
	bool is_valid = false;
	unsigned int reg_page_array_size = ARRAY_SIZE(reg_page_array);
	unsigned int i = 0;

	for (i = 0; i < reg_page_array_size; i++) {
		if ((reg_type ==reg_page_array[i].page_tag) &&
			((reg_value >= reg_page_array[i].page_reg_begin) && (reg_value <= reg_page_array[i].page_reg_end))) {
			is_valid = true;
			break;
		}
	}

	if (is_valid) {
		if ((PAGE_ASPCFG == reg_type || PAGE_SoCCODEC == reg_type) && priv->asp_pd) {
			HI6555V2_LOGE("asp power down\n");
			is_valid = false;
		}
	}
	else {
		HI6555V2_LOGE("reg_type:%d, reg_value:0x%x is invalid\n", reg_type, reg_value);
	}

	return is_valid;
}

static void _hi6555v2_runtime_info_print(struct hi6555v2_priv *priv)
{
	struct device *dev = NULL;

	WARN_ON(!priv);
	WARN_ON(!priv->codec);

	dev = priv->codec->dev;
	WARN_ON(!dev);

	HI6555V2_LOGD("get suspend usage_count:0x%x child_count:0x%x status:0x%x disable_depth:%d asp_subsys_clk:%d, asp_49m_clk:%d\n",
			atomic_read(&(dev->power.usage_count)),
			atomic_read(&(dev->power.child_count)),
			dev->power.runtime_status,
			dev->power.disable_depth,
			clk_get_enable_count(priv->asp_subsys_clk),
			clk_get_enable_count(priv->asp_49m_clk));
}

static int _hi6555v2_runtime_get_sync(struct hi6555v2_priv *priv, unsigned int reg_type)
{
	bool runtime_flag;
	struct device *dev;
	int pm_ret = 0;

	IN_FUNCTION;

	WARN_ON(!priv);
	WARN_ON(!priv->codec);
	WARN_ON(!priv->codec->dev);

	runtime_flag = priv->pm_runtime_support && (PAGE_SoCCODEC == reg_type || PAGE_ASPCFG == reg_type);
	dev = priv->codec->dev;

	if (runtime_flag) {
		pm_ret = pm_runtime_get_sync(dev);
		HI6555V2_LOGD("get pm resume  ret:%d\n", pm_ret);
		if (pm_ret < 0) {
			HI6555V2_LOGE("pm resume error, ret:%d\n", pm_ret);
			return pm_ret;
		}

		_hi6555v2_runtime_info_print(priv);
	}

	OUT_FUNCTION;
	return pm_ret;
}

static void _hi6555v2_runtime_put_sync(struct hi6555v2_priv *priv, unsigned int reg_type)
{
	bool runtime_flag;
	struct device *dev;

	IN_FUNCTION;

	WARN_ON(!priv);
	WARN_ON(!priv->codec);
	WARN_ON(!priv->codec->dev);

	runtime_flag = priv->pm_runtime_support && (PAGE_SoCCODEC == reg_type || PAGE_ASPCFG == reg_type);
	dev = priv->codec->dev;

	if (runtime_flag) {
		pm_runtime_mark_last_busy(dev);
		pm_runtime_put_autosuspend(dev);

		_hi6555v2_runtime_info_print(priv);
	}

	OUT_FUNCTION;
}

static unsigned int _hi6555v2_reg_read(struct hi6555v2_priv *priv, unsigned int reg)
{
	volatile unsigned int ret = 0;
	unsigned int reg_type = 0;
	unsigned int reg_value = 0;
	unsigned long flags = 0;
	int pm_ret;

	reg_type  = reg & PAGE_TYPE_MASK;
	reg_value = reg & PAGE_VALUE_MASK;

	if (!_reg_value_valid(priv, reg_type, reg_value)) {
		HI6555V2_LOGE("invalid reg:0x%pK\n", (void *)(unsigned long)reg);
		return INVALID_REG_VALUE;
	}

	pm_ret = _hi6555v2_runtime_get_sync(priv, reg_type);
	if (pm_ret < 0 ) {
		HI6555V2_LOGE("runtime resume fail");
		return INVALID_REG_VALUE;
	}

	spin_lock_irqsave(&priv->lock, flags);

	switch (reg_type) {
	case PAGE_SoCCODEC:
		ret = readl(reg_base_addr[HI6555V2_SOCCODEC] + reg_value);
		HI6555V2_LOGD("PAGE_SoCCODEC: offset = 0x%x, value = 0x%x\n", reg_value, ret);
		break;
	case PAGE_PMU_CODEC:
		ret = hisi_pmic_reg_read(reg_value);
		HI6555V2_LOGD("PAGE_PMUCODEC: offset = 0x%x, value = 0x%x\n", reg_value, ret);
		break;
	case PAGE_ASPCFG:
		ret = readl(reg_base_addr[HI6555V2_ASPCFG] + reg_value);
		HI6555V2_LOGD("PAGE_ASPCFG: offset = 0x%x, value = 0x%x\n", reg_value, ret);
		break;
	case PAGE_AO_IOC:
		ret = readl(reg_base_addr[HI6555V2_AOIOC] + reg_value);
		HI6555V2_LOGD("PAGE_AO_IOC: offset = 0x%x, value = 0x%x\n", reg_value, ret);
		break;
	case PAGE_VIRCODEC:
		ret = priv->v_codec_reg[reg_value];
		HI6555V2_LOGD("PAGE_VIRCODEC: offset = 0x%x, ret = 0x%x\n", reg_value, ret);
		break;
	default:
		HI6555V2_LOGE("reg=0x%pK\n", (void *)(unsigned long)reg);
		ret = INVALID_REG_VALUE;
		break;
	}

	spin_unlock_irqrestore(&priv->lock, flags);

	_hi6555v2_runtime_put_sync(priv, reg_type);

	return ret;
}

static void _hi6555v2_reg_write(struct hi6555v2_priv *priv, unsigned int reg, unsigned int value)
{
	unsigned int reg_type = 0;
	unsigned int reg_value = 0;
	unsigned long flags = 0;
	int ret = 0;

	reg_type  = reg & PAGE_TYPE_MASK;
	reg_value = reg & PAGE_VALUE_MASK;

	if (!_reg_value_valid(priv, reg_type, reg_value)) {
		HI6555V2_LOGE("invalid reg:0x%pK, value:%d\n", (void *)(unsigned long)reg, value);
		return;
	}

    ret = _hi6555v2_runtime_get_sync(priv, reg_type);
	if (ret < 0 ) {
		HI6555V2_LOGE("runtime resume fail");
		return;
	}

	spin_lock_irqsave(&priv->lock, flags);

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	hicodec_debug_reg_rw_cache(reg, value, HICODEC_DEBUG_FLAG_WRITE);
#endif

	switch (reg_type) {
	case PAGE_SoCCODEC:
		HI6555V2_LOGD("PAGE_SoCCODEC: offset = 0x%x, value = 0x%x\n", reg_value, value);
		writel(value, reg_base_addr[HI6555V2_SOCCODEC] + reg_value);
		break;
	case PAGE_PMU_CODEC:
		HI6555V2_LOGD("PAGE_PMUCODEC: offset = 0x%x, value = 0x%x\n", reg_value, value);
		hisi_pmic_reg_write(reg_value, value);
		break;
	case PAGE_ASPCFG:
		HI6555V2_LOGD("PAGE_ASPCFG: offset = 0x%x, value = 0x%x\n", reg_value, value);
		writel(value, reg_base_addr[HI6555V2_ASPCFG] + reg_value);
		break;
	case PAGE_AO_IOC:
		HI6555V2_LOGD("PAGE_AO_IOC: offset = 0x%x, value = 0x%x\n", reg_value, value);
		writel(value, reg_base_addr[HI6555V2_AOIOC] + reg_value);
		break;
	case PAGE_VIRCODEC:
		HI6555V2_LOGD("PAGE_VIRCODEC: offset = 0x%x, value = 0x%x\n", reg_value, value);
		priv->v_codec_reg[reg_value] = value;
		break;
	default:
		HI6555V2_LOGE("reg=0x%pK, value=0x%x\n", (void *)(unsigned long)reg, value);
		break;
	}
	spin_unlock_irqrestore(&priv->lock, flags);

	_hi6555v2_runtime_put_sync(priv, reg_type);
}

int hi6555v2_base_addr_map(struct platform_device *pdev)
{
	struct resource *res = NULL;
	unsigned int i;

	IN_FUNCTION;

	if (!pdev) {
		return -EINVAL;
	}

	for (i = 0; i < HI6555V2_REG_CNT; i++) {
		res = platform_get_resource(pdev, IORESOURCE_MEM, i);
		if (!res) {
			HI6555V2_LOGE("platform_get_resource %d err\n", i);
			return -ENOENT;
		}

		reg_base_addr[i] = (char * __force)(ioremap(res->start, resource_size(res)));
		if (!reg_base_addr[i]) {
			HI6555V2_LOGE("cannot map register memory\n");
			return -ENOMEM;
		}
	}

	OUT_FUNCTION;

	return 0;
}

void hi6555v2_base_addr_unmap(void)
{
	unsigned int i;

	IN_FUNCTION;
	for (i = 0; i < HI6555V2_REG_CNT; i++) {
		if (reg_base_addr[i]) {
			iounmap(reg_base_addr[i]);
			reg_base_addr[i] = NULL;
		}
	}
	OUT_FUNCTION;
}

unsigned int hi6555v2_reg_read(struct snd_soc_codec *codec, unsigned int reg)
{
	volatile unsigned int ret;
	struct hi6555v2_priv *priv;

	if (!codec) {
		HI6555V2_LOGE("codec is null\n");
		return INVALID_REG_VALUE;
	}

	priv = snd_soc_codec_get_drvdata(codec);
	if (!priv) {
		HI6555V2_LOGE("priv is null\n");
		return INVALID_REG_VALUE;
	}

	ret = _hi6555v2_reg_read(priv, reg);
	if (INVALID_REG_VALUE == ret) {
		HI6555V2_LOGE("reg 0x%pK read value 0x%x is invalid\n", (void *)(unsigned long)reg, ret);
	}

	return ret;
}

int hi6555v2_reg_write(struct snd_soc_codec *codec, unsigned int reg, unsigned int value)
{
	int ret = 0;
	struct hi6555v2_priv *priv;

	if (!codec) {
		HI6555V2_LOGE("codec is null\n");
		return -EINVAL;
	}

	priv = snd_soc_codec_get_drvdata(codec);
	if (!priv) {
		HI6555V2_LOGE("priv is null\n");
		return -EINVAL;
	}

	_hi6555v2_reg_write(priv, reg, value);

	return ret;
}

int hi6555v2_reg_update(unsigned int reg, unsigned int mask, unsigned int value)
{
	int change;
	unsigned int old, new;

	struct snd_soc_codec *codec = hi6555v2_get_codec();
	if (!codec) {
		HI6555V2_LOGE("codec is NULL\n");
		return -EIO;
	}

	old = hi6555v2_reg_read(codec, reg);
	new = (old & ~mask) | (value & mask);
	change = old != new;
	if (change)
		hi6555v2_reg_write(codec, reg, new);

	return change;
}

void hi6555v2_set_reg_bits(unsigned int reg, unsigned int value)
{
	unsigned int val = 0;
	struct snd_soc_codec *codec = hi6555v2_get_codec();
	if (!codec) {
		HI6555V2_LOGE("codec is NULL\n");
		return;
	}

	val = hi6555v2_reg_read(codec, reg) | (value);
	hi6555v2_reg_write(codec, reg, val);
}

void hi6555v2_clr_reg_bits(unsigned int reg, unsigned int value)
{
	unsigned int val = 0;
	struct snd_soc_codec *codec = hi6555v2_get_codec();

	if (!codec) {
		HI6555V2_LOGE("codec is NULL\n");
		return;
	}

	val = hi6555v2_reg_read(codec, reg) & ~(value);
	hi6555v2_reg_write(codec, reg, val);
}

