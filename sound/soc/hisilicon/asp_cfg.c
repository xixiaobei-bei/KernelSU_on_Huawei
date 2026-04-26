/*
 * asp_cfg.c
 *
 * asp dma driver
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.s
 */

#include "asp_cfg.h"

#include <linux/module.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/hwspinlock.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include <linux/hisi/audio_log.h>

#define LOG_TAG "asp_cfg"


enum {
	USB_H2X,
	ASP_H2X
};


struct asp_cfg_priv {
	struct device *dev;
	struct clk *asp_subsys_clk;
	struct regulator_bulk_data regu;
	int irq;
	spinlock_t lock;
	spinlock_t h2x_lock;
	struct resource *res;
	void __iomem *asp_cfg_reg_base_addr;
	unsigned int asp_h2x_module_count;
	bool usb_h2x_enable;
	bool asp_h2x_enable;
};

static struct asp_cfg_priv *g_asp_cfg_priv;


static unsigned int asp_cfg_reg_read(unsigned int reg)
{
	struct asp_cfg_priv *priv = g_asp_cfg_priv;
	unsigned int ret;
	unsigned long flag_sft = 0;

	if (!priv) {
		AUDIO_LOGE("priv is null");
		return 0;
	}

	spin_lock_irqsave(&priv->lock, flag_sft);

	ret = readl(priv->asp_cfg_reg_base_addr + reg);

	spin_unlock_irqrestore(&priv->lock, flag_sft);

	return ret;
}

static void asp_cfg_reg_write(unsigned int reg, unsigned int value)
{
	struct asp_cfg_priv *priv = g_asp_cfg_priv;
	unsigned long flag_sft = 0;

	if (!priv) {
		AUDIO_LOGE("priv is null");
		return;
	}

	spin_lock_irqsave(&priv->lock, flag_sft);

	writel(value, priv->asp_cfg_reg_base_addr + reg);

	spin_unlock_irqrestore(&priv->lock, flag_sft);
}

static void asp_cfg_reg_set_bit(unsigned int reg, unsigned int offset)
{
	struct asp_cfg_priv *priv = g_asp_cfg_priv;
	unsigned int value;
	unsigned long flag_sft = 0;

	if (!priv) {
		AUDIO_LOGE("priv is null");
		return;
	}

	spin_lock_irqsave(&priv->lock, flag_sft);

	value = readl(priv->asp_cfg_reg_base_addr + reg);
	value |= BIT(offset);
	writel(value, priv->asp_cfg_reg_base_addr + reg);

	spin_unlock_irqrestore(&priv->lock, flag_sft);
}

static void asp_cfg_reg_clr_bit(unsigned int reg, unsigned int offset)
{
	struct asp_cfg_priv *priv = g_asp_cfg_priv;
	unsigned int value;
	unsigned long flag_sft = 0;

	if (!priv) {
		AUDIO_LOGE("priv is null");
		return;
	}

	spin_lock_irqsave(&priv->lock, flag_sft);

	value = readl(priv->asp_cfg_reg_base_addr + reg);
	value &= ~BIT(offset);
	writel(value, priv->asp_cfg_reg_base_addr + reg);

	spin_unlock_irqrestore(&priv->lock, flag_sft);
}

static void h2x_module_set(unsigned int module, bool enable)
{
	struct asp_cfg_priv *priv = g_asp_cfg_priv;
	unsigned long flag_sft = 0;

	if (!priv) {
		AUDIO_LOGE("priv is null");
		return;
	}

	spin_lock_irqsave(&priv->h2x_lock, flag_sft);
	AUDIO_LOGI("module %u, enable %d", module, enable);
	if (module == ASP_H2X)
		priv->asp_h2x_enable = enable;
	else
		priv->usb_h2x_enable = enable;

	if (priv->asp_h2x_enable || priv->usb_h2x_enable) {
		asp_cfg_reg_write(ASP_CFG_R_RST_CTRLDIS_REG, RST_ASP_H2X_BIT);
		asp_cfg_reg_write(ASP_CFG_R_GATE_EN_REG, CLK_HDMI_MCLK_BIT);
	} else {
		asp_cfg_reg_write(ASP_CFG_R_GATE_DIS_REG, CLK_HDMI_MCLK_BIT);
		asp_cfg_reg_write(ASP_CFG_R_RST_CTRLEN_REG, RST_ASP_H2X_BIT);
	}
	spin_unlock_irqrestore(&priv->h2x_lock, flag_sft);
}

static void dp_h2x_on(void)
{
	h2x_module_set(ASP_H2X, true);
}

static void dp_h2x_off(void)
{
	h2x_module_set(ASP_H2X, false);
}

int usb_h2x_on(void)
{
	int ret;
	struct asp_cfg_priv *priv = g_asp_cfg_priv;
	struct clk *asp_subsys_clk = NULL;

	WARN_ON(!priv);

	ret = regulator_bulk_enable(CONSUMER_ASP, &priv->regu);
	if (ret != 0) {
		AUDIO_LOGE("couldn't enable regulators %d", ret);
		return -EFAULT;
	}

	asp_subsys_clk = priv->asp_subsys_clk;
	if (asp_subsys_clk) {
		ret = clk_prepare_enable(asp_subsys_clk);
		if (ret != 0) {
			AUDIO_LOGE("asp_subsys_clk enable fail, error: %d", ret);
			ret = regulator_bulk_disable(CONSUMER_ASP,
				&priv->regu);
			if (ret != 0)
				AUDIO_LOGE("fail to disable regulator, ret: %d",
					ret);

			return -EFAULT;
		}
	}

	h2x_module_set(USB_H2X, true);
	AUDIO_LOGI("exit");

	return ret;
}

int usb_h2x_off(void)
{
	struct asp_cfg_priv *priv = g_asp_cfg_priv;
	struct clk *asp_subsys_clk = NULL;
	int ret;

	if (!priv) {
		AUDIO_LOGE("priv is null");
		return -EINVAL;
	}

	asp_subsys_clk = priv->asp_subsys_clk;
	h2x_module_set(USB_H2X, false);

	if (asp_subsys_clk)
		clk_disable_unprepare(asp_subsys_clk);

	ret = regulator_bulk_disable(CONSUMER_ASP, &priv->regu);
	if (ret != 0)
		AUDIO_LOGE("fail to disable regulator, ret: %d", ret);

	AUDIO_LOGI("exit");

	return 0;
}

static void asp_cfg_h2x_module_enable(void)
{
	struct asp_cfg_priv *priv = g_asp_cfg_priv;

	if (priv->asp_h2x_module_count == 0)
		dp_h2x_on();

	priv->asp_h2x_module_count++;
	AUDIO_LOGI("asp_h2x_module_count: %u", priv->asp_h2x_module_count);
}

static void asp_cfg_h2x_module_disable(void)
{
	struct asp_cfg_priv *priv = g_asp_cfg_priv;

	priv->asp_h2x_module_count--;
	AUDIO_LOGI("asp_h2x_module_count: %u", priv->asp_h2x_module_count);
	if (priv->asp_h2x_module_count == 0)
		dp_h2x_off();
}

void asp_cfg_hdmi_clk_sel(unsigned int value)
{
	asp_cfg_reg_write(ASP_CFG_R_HDMI_CLK_SEL_REG, value);
}

void asp_cfg_div_clk(unsigned int value)
{
	asp_cfg_reg_write(ASP_CFG_R_DIV_SPDIF_CLKSEL_REG, value);
}

void asp_cfg_enable_hdmi_interrupeter(void)
{
	asp_cfg_reg_set_bit(ASP_CFG_R_INTR_NS_EN_REG,
		ASP_CFG_ASP_HDMI_INT_OFFSET);
}

void asp_cfg_disable_hdmi_interrupeter(void)
{
	asp_cfg_reg_clr_bit(ASP_CFG_R_INTR_NS_EN_REG,
		ASP_CFG_ASP_HDMI_INT_OFFSET);
}

void asp_cfg_hdmi_module_enable(void)
{
	/* reset */
	asp_cfg_h2x_module_enable();
	asp_cfg_reg_write(ASP_CFG_R_RST_CTRLDIS_REG, RST_ASP_HDMI_BIT);

	/* enable clk */
	asp_cfg_reg_write(ASP_CFG_R_GATE_EN_REG, CLK_HDMI_HCLK_BIT);
	asp_cfg_reg_write(ASP_CFG_R_GATE_EN_REG, CLK_AUDIO_PLL_BIT);

	/* enable hdmimclk_div & hdmirefclk_div */
	asp_cfg_reg_set_bit(ASP_CFG_R_GATE_CLKDIV_EN_REG,
		ASP_CFG_GT_HDMIREF_DIV_OFFSET);
}

void asp_cfg_hdmi_module_disable(void)
{
	/* disable clk */
	asp_cfg_reg_write(ASP_CFG_R_GATE_DIS_REG, CLK_AUDIO_PLL_BIT);
	asp_cfg_reg_write(ASP_CFG_R_GATE_DIS_REG, CLK_HDMI_HCLK_BIT);

	/* disable hdmimclk_div & hdmirefclk_div */
	asp_cfg_reg_clr_bit(ASP_CFG_R_GATE_CLKDIV_EN_REG,
		ASP_CFG_GT_HDMIREF_DIV_OFFSET);

	/* enable reset */
	asp_cfg_h2x_module_disable();
}

#ifdef DP_AUDIO_ASP_HDMI_I2S
void asp_cfg_dp_module_enable(void)
{
	/* enable clk */
	asp_cfg_reg_write(ASP_CFG_R_GATE_EN_REG, CLK_HDMI_BCLK_BIT);

	/* enable hdmirefclk_div */
	asp_cfg_reg_set_bit(ASP_CFG_R_GATE_CLKDIV_EN_REG,
		ASP_CFG_GT_HDMIADWS_CLK_DIV_OFFSET);
	AUDIO_LOGI("add asp cfg dp module i2s clk enable");
}

void asp_cfg_dp_module_disable(void)
{
	/* disable clk */
	asp_cfg_reg_write(ASP_CFG_R_GATE_DIS_REG, CLK_HDMI_BCLK_BIT);

	/* disable hdmirefclk_div */
	asp_cfg_reg_clr_bit(ASP_CFG_R_GATE_CLKDIV_EN_REG,
		ASP_CFG_GT_HDMIADWS_CLK_DIV_OFFSET);
	AUDIO_LOGI("sub asp cfg dp module i2s clk disable");
}

#else
void asp_cfg_dp_module_enable(void)
{
	/* enable clk */
	asp_cfg_reg_write(ASP_CFG_R_GATE_EN_REG, CLK_GT_SPDIF_BIT);
	asp_cfg_reg_write(ASP_CFG_R_GATE_EN_REG, CLK_SPDIF_HCLK_BIT);

	/* enable hdmirefclk_div */
	asp_cfg_reg_set_bit(ASP_CFG_R_GATE_CLKDIV_EN_REG,
		ASP_CFG_GT_SPDIF_BCLK_DIV_OFFSET);
	AUDIO_LOGI("asp_cfg");
}

void asp_cfg_dp_module_disable(void)
{
	/* disable clk */
	asp_cfg_reg_write(ASP_CFG_R_GATE_DIS_REG, CLK_GT_SPDIF_BIT);
	asp_cfg_reg_write(ASP_CFG_R_GATE_DIS_REG, CLK_SPDIF_HCLK_BIT);

	/* disable hdmirefclk_div */
	asp_cfg_reg_clr_bit(ASP_CFG_R_GATE_CLKDIV_EN_REG,
		ASP_CFG_GT_SPDIF_BCLK_DIV_OFFSET);
	AUDIO_LOGI("asp_cfg");
}
#endif /* DP_AUDIO_ASP_HDMI_I2S */

unsigned int asp_cfg_get_irq_value(void)
{
	return asp_cfg_reg_read(ASP_CFG_R_INTR_NS_INI_REG);
}

static int asp_cfg_priv_init(struct platform_device *pdev,
	struct device *dev, struct asp_cfg_priv *priv)
{
	int ret;

	priv->res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!priv->res) {
		AUDIO_LOGE("get asp_cfg resource failed");
		return -ENOENT;
	}

	priv->regu.supply = "asp-supply";
	ret = devm_regulator_bulk_get(dev, CONSUMER_ASP, &(priv->regu));
	if (ret != 0) {
		AUDIO_LOGE("couldn't get asp_cfg regulators %d", ret);
		return -EFAULT;
	}

	priv->asp_subsys_clk = devm_clk_get(dev, "clk_asp_subsys");
	if (IS_ERR_OR_NULL(priv->asp_subsys_clk)) {
		AUDIO_LOGE("devm_clk_get: clk_asp_subsys not found");
		return -EFAULT;
	}

	priv->asp_cfg_reg_base_addr = devm_ioremap(dev, priv->res->start,
		resource_size(priv->res));
	if (!priv->asp_cfg_reg_base_addr) {
		AUDIO_LOGE("asp cfg reg addr ioremap failed");
		return -ENOMEM;
	}

	return 0;
}

static int asp_cfg_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = NULL;
	struct asp_cfg_priv *priv = NULL;

	if (!pdev) {
		AUDIO_LOGE("pdev is null");
		return -EINVAL;
	}

	dev = &pdev->dev;
	AUDIO_LOGI("probe begin");

	priv = devm_kzalloc(dev, sizeof(struct asp_cfg_priv), GFP_KERNEL);
	if (!priv) {
		AUDIO_LOGE("malloc asp_cfg platform data failed");
		return -ENOMEM;
	}

	ret = asp_cfg_priv_init(pdev, dev, priv);
	if (ret != 0)
		return ret;

	AUDIO_LOGI("res->start.%pK", (void *)(uintptr_t)priv->res->start);
	AUDIO_LOGI("asp_cfg_reg_base_addr.%pK",
		(void *)priv->asp_cfg_reg_base_addr);

	spin_lock_init(&priv->lock);
	spin_lock_init(&priv->h2x_lock);

	priv->dev = dev;

	platform_set_drvdata(pdev, priv);

	g_asp_cfg_priv = priv;

	AUDIO_LOGI("probe end");

	return ret;
}

static int asp_cfg_remove(struct platform_device *pdev)
{
	struct asp_cfg_priv *priv =
		(struct asp_cfg_priv *)platform_get_drvdata(pdev);

	if (!priv)
		return 0;

	if (priv->asp_cfg_reg_base_addr)
		devm_iounmap(priv->dev, priv->asp_cfg_reg_base_addr);

	g_asp_cfg_priv = NULL;

	AUDIO_LOGI("asp cfg driver remove succ");

	return 0;
}

static const struct of_device_id g_of_asp_cfg_match[] = {
	{ .compatible = "hisilicon,asp-cfg", },
	{},
};
MODULE_DEVICE_TABLE(of, g_of_asp_cfg_match);

static struct platform_driver g_asp_cfg_driver = {
	.driver = {
		.name = "asp_cfg_drv",
		.owner = THIS_MODULE,
		.of_match_table = g_of_asp_cfg_match,
	},
	.probe = asp_cfg_probe,
	.remove = asp_cfg_remove,
};

static int __init asp_cfg_init(void)
{
	return platform_driver_register(&g_asp_cfg_driver);
}
module_init(asp_cfg_init);

static void __exit asp_cfg_exit(void)
{
	platform_driver_unregister(&g_asp_cfg_driver);
}
module_exit(asp_cfg_exit);

MODULE_DESCRIPTION("ASP CFG Driver");
MODULE_LICENSE("GPL v2");

