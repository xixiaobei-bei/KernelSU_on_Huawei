/*
 * hi_cdc_ssi.c
 *
 * codec ssi driver
 *
 * Copyright (c) 2014-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/hisi/da_combine/hi_cdc_ssi.h>

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/pm_runtime.h>
#include <linux/types.h>
#include <linux/hisi/audio_log.h>
#include <linux/pinctrl/consumer.h>
#include <rdr_audio_adapter.h>

/*lint -e750 -e730 -e838 -e529 -e438 -e778 -e826 -e774 -e747 -e527 -e456 -e454 -e455*/

#define LOG_TAG "hi_cdc_ssi"
#define SSI_DEFAULT_PAGE                1
#define DA_COMBINE_CFG_BASE_ADDR            0x20007000
#define DA_COMBINE_PAGE_SELECT_MASK_SSI     0xFF
#define DA_COMBINE_REG32_OFFSET_MASK_SSI    0xFC
#define DA_COMBINE_REG_VAL_BIT              8
#define ALIGNED_4BYTE_DETECTION         0x03
#define DA_COMBINE_REG_VAL_MASK             0xFF
#define DA_COMBINE_PAGE_SELECT_REG_0_SSI    0x1FD
#define DA_COMBINE_PAGE_SELECT_REG_1_SSI    0x1FE
#define DA_COMBINE_PAGE_SELECT_REG_2_SSI    0x1FF
#define DA_COMBINE_RAM2AXI_RD_DATA0         (DA_COMBINE_CFG_BASE_ADDR + 0x23)
#define DA_COMBINE_RAM2AXI_RD_DATA1         (DA_COMBINE_CFG_BASE_ADDR + 0x24)
#define DA_COMBINE_RAM2AXI_RD_DATA2         (DA_COMBINE_CFG_BASE_ADDR + 0x25)
#define DA_COMBINE_RAM2AXI_RD_DATA3         (DA_COMBINE_CFG_BASE_ADDR + 0x26)

struct hi_cdcssi_priv {
	struct device *dev;
	struct clk *codec_ssi_clk;
	struct clk *pmu_audio_clk;
	struct pinctrl *pctrl;
	struct pinctrl_state *pin_default;
	struct pinctrl_state *pin_idle;
	struct mutex sr_rw_lock;
	unsigned int ssi_page_sel;
	void __iomem *ssi_base;
	bool pm_runtime_support;
};

static struct hi_cdcssi_priv *g_pdata;

void ssi_select_reg_page(unsigned int reg)
{
	unsigned int reg_page = reg & (~DA_COMBINE_PAGE_SELECT_MASK_SSI);

	if (g_pdata->ssi_page_sel == reg_page)
		return;

	g_pdata->ssi_page_sel = reg_page;

	reg_page = reg_page >> DA_COMBINE_REG_VAL_BIT;
	writel(reg_page & DA_COMBINE_REG_VAL_MASK, g_pdata->ssi_base +
		(DA_COMBINE_PAGE_SELECT_REG_0_SSI << 2));

	reg_page = reg_page >> DA_COMBINE_REG_VAL_BIT;
	writel(reg_page & DA_COMBINE_REG_VAL_MASK, g_pdata->ssi_base +
		(DA_COMBINE_PAGE_SELECT_REG_1_SSI << 2));

	reg_page = reg_page >> DA_COMBINE_REG_VAL_BIT;
	writel(reg_page & DA_COMBINE_REG_VAL_MASK, g_pdata->ssi_base +
		(DA_COMBINE_PAGE_SELECT_REG_2_SSI << 2));
}

unsigned int ssi_reg_read8(unsigned int reg)
{
	unsigned int val;
	int ret;

	mutex_lock(&g_pdata->sr_rw_lock);

	if (g_pdata->pm_runtime_support) {
		ret = pm_runtime_get_sync(g_pdata->dev);
		if (ret < 0) {
			AUDIO_LOGE("pm resume error, ret: %d", ret);
			mutex_unlock(&g_pdata->sr_rw_lock);
			rdr_system_error(RDR_AUDIO_RUNTIME_SYNC_FAIL_MODID, 0, 0);

			return 0;
		}
	}

	ssi_select_reg_page(reg);
	readl((void *)g_pdata->ssi_base + ((reg & (DA_COMBINE_REG_VAL_MASK)) << 2));
	val = readl((void *)g_pdata->ssi_base + ((reg & (DA_COMBINE_REG_VAL_MASK)) << 2));

	if (g_pdata->pm_runtime_support) {
		pm_runtime_mark_last_busy(g_pdata->dev);
		pm_runtime_put_autosuspend(g_pdata->dev);
	}

	mutex_unlock(&g_pdata->sr_rw_lock);

	return val;
}

unsigned int ssi_reg_read32(unsigned int reg)
{
	unsigned int ret;
	int pm_ret;

	mutex_lock(&g_pdata->sr_rw_lock);

	if (g_pdata->pm_runtime_support) {
		pm_ret = pm_runtime_get_sync(g_pdata->dev);
		if (pm_ret < 0) {
			AUDIO_LOGE("pm resume error, pm ret: %d", pm_ret);
			mutex_unlock(&g_pdata->sr_rw_lock);
			rdr_system_error(RDR_AUDIO_RUNTIME_SYNC_FAIL_MODID, 0, 0);

			return 0;
		}
	}

	ssi_select_reg_page(reg);
	readl((void *)g_pdata->ssi_base +
		((reg & (DA_COMBINE_REG32_OFFSET_MASK_SSI)) << 2));
	readl((void *)g_pdata->ssi_base +
		((reg & (DA_COMBINE_REG32_OFFSET_MASK_SSI)) << 2));

	ssi_select_reg_page(DA_COMBINE_RAM2AXI_RD_DATA0);
	readl((void *)g_pdata->ssi_base +
		(((DA_COMBINE_RAM2AXI_RD_DATA3) & (DA_COMBINE_REG_VAL_MASK)) << 2));
	ret = readl((void *)g_pdata->ssi_base +
		(((DA_COMBINE_RAM2AXI_RD_DATA3) & (DA_COMBINE_REG_VAL_MASK)) << 2));
	readl((void *)g_pdata->ssi_base +
		(((DA_COMBINE_RAM2AXI_RD_DATA2) & (DA_COMBINE_REG_VAL_MASK)) << 2));
	ret = (ret << 8) + (0xFF & readl((void *)g_pdata->ssi_base +
		(((DA_COMBINE_RAM2AXI_RD_DATA2) & (DA_COMBINE_REG_VAL_MASK)) << 2)));
	readl((void *)g_pdata->ssi_base +
		(((DA_COMBINE_RAM2AXI_RD_DATA1) & (DA_COMBINE_REG_VAL_MASK)) << 2));
	ret = (ret << 8) + (0xFF & readl((void *)g_pdata->ssi_base +
		(((DA_COMBINE_RAM2AXI_RD_DATA1) & (DA_COMBINE_REG_VAL_MASK)) << 2)));
	readl((void *)g_pdata->ssi_base +
		(((DA_COMBINE_RAM2AXI_RD_DATA0) & (DA_COMBINE_REG_VAL_MASK)) << 2));
	ret = (ret << 8) + (0xFF & readl((void *)g_pdata->ssi_base +
		(((DA_COMBINE_RAM2AXI_RD_DATA0) & (DA_COMBINE_REG_VAL_MASK)) << 2)));

	if (g_pdata->pm_runtime_support) {
		pm_runtime_mark_last_busy(g_pdata->dev);
		pm_runtime_put_autosuspend(g_pdata->dev);
	}

	mutex_unlock(&g_pdata->sr_rw_lock);

	return ret;
}

void ssi_reg_write8(unsigned int reg, unsigned int val)
{
	int ret;

	mutex_lock(&g_pdata->sr_rw_lock);

	if (g_pdata->pm_runtime_support) {
		ret = pm_runtime_get_sync(g_pdata->dev);
		if (ret < 0) {
			AUDIO_LOGE("pm resume error, ret: %d", ret);
			mutex_unlock(&g_pdata->sr_rw_lock);
			rdr_system_error(RDR_AUDIO_RUNTIME_SYNC_FAIL_MODID, 0, 0);

			return;
		}
	}

	ssi_select_reg_page(reg);

	writel(val & DA_COMBINE_REG_VAL_MASK, (void *)g_pdata->ssi_base +
		((reg & DA_COMBINE_REG_VAL_MASK) << 2));

	if (g_pdata->pm_runtime_support) {
		pm_runtime_mark_last_busy(g_pdata->dev);
		pm_runtime_put_autosuspend(g_pdata->dev);
	}

	mutex_unlock(&g_pdata->sr_rw_lock);
}

void ssi_reg_write32(unsigned int reg, unsigned int val)
{
	int ret;

	mutex_lock(&g_pdata->sr_rw_lock);

	if (reg & ALIGNED_4BYTE_DETECTION) {
		AUDIO_LOGE("reg is 0x%pK, it's not alignment", (void *)(uintptr_t)reg);
		mutex_unlock(&g_pdata->sr_rw_lock);

		return;
	}

	if (g_pdata->pm_runtime_support) {
		ret = pm_runtime_get_sync(g_pdata->dev);
		if (ret < 0) {
			AUDIO_LOGE("pm resume error, ret: %d", ret);
			mutex_unlock(&g_pdata->sr_rw_lock);
			rdr_system_error(RDR_AUDIO_RUNTIME_SYNC_FAIL_MODID, 0, 0);

			return;
		}
	}

	ssi_select_reg_page(reg);

	writel(val & DA_COMBINE_REG_VAL_MASK,
		(void *)g_pdata->ssi_base + ((reg & DA_COMBINE_REG_VAL_MASK) << 2));
	writel((val >> 8) & DA_COMBINE_REG_VAL_MASK,
		(void *)g_pdata->ssi_base + (((reg + 1) & DA_COMBINE_REG_VAL_MASK) << 2));
	writel((val >> 16) & DA_COMBINE_REG_VAL_MASK,
		(void *)g_pdata->ssi_base + (((reg + 2) & DA_COMBINE_REG_VAL_MASK) << 2));
	writel((val >> 24) & DA_COMBINE_REG_VAL_MASK,
		(void *)g_pdata->ssi_base + (((reg + 3) & DA_COMBINE_REG_VAL_MASK) << 2));

	if (g_pdata->pm_runtime_support) {
		pm_runtime_mark_last_busy(g_pdata->dev);
		pm_runtime_put_autosuspend(g_pdata->dev);
	}

	mutex_unlock(&g_pdata->sr_rw_lock);
}

static int get_cdcssi_clk(struct device *dev, struct hi_cdcssi_priv *priv)
{
	int ret;

	priv->codec_ssi_clk = devm_clk_get(dev, "clk_codecssi");
	if (IS_ERR(priv->codec_ssi_clk)) {
		AUDIO_LOGE("clk get: ssi clk not found");
		ret = PTR_ERR(priv->codec_ssi_clk);
		return ret;
	}

	ret = clk_prepare_enable(priv->codec_ssi_clk);
	if (ret != 0) {
		AUDIO_LOGE("ssi clk: clk prepare enable failed");
		return ret;
	}

	return 0;
}

static int get_pinctrl(struct device *dev, struct hi_cdcssi_priv *priv)
{
	priv->pctrl = devm_pinctrl_get(dev);
	if (IS_ERR(priv->pctrl)) {
		AUDIO_LOGE("could not get pinctrl");
		return -EIO;
	}

	priv->pin_default = pinctrl_lookup_state(priv->pctrl, PINCTRL_STATE_DEFAULT);
	if (IS_ERR(priv->pin_default)) {
		AUDIO_LOGE("could not get default state (%li)", PTR_ERR(priv->pin_default));
		return -EIO;
	}

	priv->pin_idle = pinctrl_lookup_state(priv->pctrl, PINCTRL_STATE_IDLE);
	if (IS_ERR(priv->pin_idle)) {
		AUDIO_LOGE("could not get idle state (%li)", PTR_ERR(priv->pin_idle));
		return -EIO;
	}

	if (pinctrl_select_state(priv->pctrl, priv->pin_default)) {
		AUDIO_LOGE("could not set pins to default state");
		return -EIO;
	}

	return 0;
}

static int get_resource(struct platform_device *pdev, struct resource *resource,
	struct hi_cdcssi_priv *priv)
{
	resource = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (resource == NULL) {
		AUDIO_LOGE("get IORESOURCE_MEM failed");
		return -ENXIO;
	}

	priv->ssi_base = ioremap(resource->start, resource_size(resource));
	if (priv->ssi_base == NULL) {
		AUDIO_LOGE("remap base address %pK failed", (void *)(uintptr_t)resource->start);
		return -ENXIO;
	}

	return 0;
}

static void put_cdcssi_clk(struct hi_cdcssi_priv *priv)
{
	clk_disable_unprepare(priv->codec_ssi_clk);
}

static int hi_cdcssi_probe(struct platform_device *pdev)
{
	int ret;
	struct hi_cdcssi_priv *priv = NULL;
	struct device *dev = &pdev->dev;
	struct resource *resource = NULL;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (priv == NULL) {
		AUDIO_LOGE("devm kzalloc failed");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, priv);
	g_pdata = priv;
	g_pdata->dev = dev;

	priv->ssi_page_sel = SSI_DEFAULT_PAGE;
	mutex_init(&priv->sr_rw_lock);

	ret = get_cdcssi_clk(dev, priv);
	if (ret != 0)
		goto clk_err;

	mdelay(1);

	ret = get_pinctrl(dev, priv);
	if (ret < 0)
		goto pinctrl_err;

	ret = get_resource(pdev, resource, priv);
	if (ret < 0)
		goto resource_err;

	if (of_property_read_bool(dev->of_node, "pm_runtime_support"))
		priv->pm_runtime_support = true;

	AUDIO_LOGI("pm runtime support: %d", priv->pm_runtime_support);

	if (priv->pm_runtime_support) {
		pm_runtime_use_autosuspend(&pdev->dev);
		pm_runtime_set_autosuspend_delay(&pdev->dev, 200); /* 200ms */
		pm_runtime_set_active(&pdev->dev);
		pm_runtime_enable(&pdev->dev);
	}

	return 0;

resource_err:
	pinctrl_put(priv->pctrl);
pinctrl_err:
	put_cdcssi_clk(priv);
clk_err:
	mutex_destroy(&priv->sr_rw_lock);

	return ret;
}

static void put_resource(struct hi_cdcssi_priv *priv)
{
	iounmap(priv->ssi_base);
}

static int hi_cdcssi_remove(struct platform_device *pdev)
{
	struct hi_cdcssi_priv *priv = platform_get_drvdata(pdev);
	struct device *dev = NULL;

	dev = &pdev->dev;

	if (priv->pm_runtime_support) {
		pm_runtime_resume(dev);
		pm_runtime_disable(dev);
	}

	put_resource(priv);
	pinctrl_put(priv->pctrl);
	put_cdcssi_clk(priv);
	if (priv->pm_runtime_support)
		pm_runtime_set_suspended(dev);

	mutex_destroy(&priv->sr_rw_lock);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int hi_cdcssi_suspend(struct device *device)
{
	int ret;
	struct platform_device *pdev = to_platform_device(device);
	struct hi_cdcssi_priv *priv = platform_get_drvdata(pdev);
	struct device *dev = NULL;

	dev = &pdev->dev;

	mutex_lock(&priv->sr_rw_lock);

	if (priv->pm_runtime_support) {
		ret = pm_runtime_get_sync(device);
		if (ret < 0) {
			AUDIO_LOGE("pm resume error, ret: %d", ret);
			mutex_unlock(&priv->sr_rw_lock);
			rdr_system_error(RDR_AUDIO_RUNTIME_SYNC_FAIL_MODID, 0, 0);
			return ret;
		}
	}
	AUDIO_LOGI("usage count: 0x%x status: 0x%x disable depth: %u clk: %d",
		atomic_read(&(device->power.usage_count)),
		device->power.runtime_status, device->power.disable_depth,
		clk_get_enable_count(priv->codec_ssi_clk));

	ret = pinctrl_select_state(priv->pctrl, priv->pin_idle);
	if (ret != 0) {
		mutex_unlock(&priv->sr_rw_lock);
		AUDIO_LOGE("could not set pins to idle state");
	}

	clk_disable_unprepare(priv->codec_ssi_clk);

	return ret;
}

static int hi_cdcssi_resume(struct device *device)
{
	int ret;
	struct platform_device *pdev = to_platform_device(device);
	struct hi_cdcssi_priv *priv = platform_get_drvdata(pdev);
	struct device *dev = NULL;

	dev = &pdev->dev;

	ret = clk_prepare_enable(priv->codec_ssi_clk);
	if (ret != 0)
		AUDIO_LOGE("ssi clk: clk prepare enable failed");

	ret = pinctrl_select_state(priv->pctrl, priv->pin_default);
	if (ret != 0)
		AUDIO_LOGE("could not set pins to default state");

	if (priv->pm_runtime_support) {
		pm_runtime_mark_last_busy(device);
		pm_runtime_put_autosuspend(device);

		pm_runtime_disable(device);
		pm_runtime_set_active(device);
		pm_runtime_enable(device);
	}

	AUDIO_LOGI("usage count: 0x%x status: 0x%x disable depth: %u clk: %d",
		atomic_read(&(device->power.usage_count)),
		device->power.runtime_status, device->power.disable_depth,
		clk_get_enable_count(priv->codec_ssi_clk));

	mutex_unlock(&priv->sr_rw_lock);

	return ret;
}
#endif

#ifdef CONFIG_PM
static int hi_cdcssi_runtime_suspend(struct device *device)
{
	int ret;
	struct platform_device *pdev = to_platform_device(device);
	struct hi_cdcssi_priv *priv = platform_get_drvdata(pdev);

	ret = pinctrl_select_state(priv->pctrl, priv->pin_idle);
	if (ret != 0) {
		mutex_unlock(&priv->sr_rw_lock);
		AUDIO_LOGE("could not set pins to idle state");
	}

	clk_disable_unprepare(priv->codec_ssi_clk);

	return ret;
}

static int hi_cdcssi_runtime_resume(struct device *device)
{
	int ret;
	struct platform_device *pdev = to_platform_device(device);
	struct hi_cdcssi_priv *priv = platform_get_drvdata(pdev);

	ret = clk_prepare_enable(priv->codec_ssi_clk);
	if (ret != 0)
		AUDIO_LOGE("ssi clk: clk prepare enable failed");

	ret = pinctrl_select_state(priv->pctrl, priv->pin_default);
	if (ret != 0)
		AUDIO_LOGE("could not set pins to default state");

	mdelay(1);

	return ret;
}
#endif

static const struct dev_pm_ops cdcssi_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(hi_cdcssi_suspend, hi_cdcssi_resume)
	SET_RUNTIME_PM_OPS(hi_cdcssi_runtime_suspend, hi_cdcssi_runtime_resume, NULL)
};

static const struct of_device_id codecssi_match[] = {
	{ .compatible = "hisilicon,codecssi", },
	{},
};
MODULE_DEVICE_TABLE(of, codecssi_match);

static struct platform_driver codecssi_driver = {
	.probe = hi_cdcssi_probe,
	.remove = hi_cdcssi_remove,
	.driver = {
		.name = "hisilicon,codecssi",
		.owner = THIS_MODULE,
		.pm = &cdcssi_pm_ops,
		.of_match_table = codecssi_match,
	},
};

static int __init hi_cdcssi_init(void)
{
	int ret;

	ret = platform_driver_register(&codecssi_driver);
	if (ret != 0)
		AUDIO_LOGE("driver register failed");

	return ret;
}

static void __exit hi_cdcssi_exit(void)
{
	platform_driver_unregister(&codecssi_driver);
}
fs_initcall(hi_cdcssi_init);
module_exit(hi_cdcssi_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("codecssi controller");

