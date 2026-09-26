/*
 * capricorn Synopsys Secure Digital Host Controller Interface.
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/bootdevice.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <linux/mmc/mmc.h>
#include <linux/slab.h>
#include <linux/scatterlist.h>
#include <linux/mmc/cmdq_hci.h>
#include <linux/pm_runtime.h>
#include <linux/mmc/card.h>
#include <linux/regulator/consumer.h>
#include <linux/i2c.h>
#include <linux/hisi/rpmb.h>
#include <soc_pmctrl_interface.h>

#include "sdhci-pltfm.h"
#include "sdhci_dwc_mshc.h"

#define DRIVER_NAME "sdhci-mshc"

static u32 xin_clk = SDHCI_MSHC_XIN_CLK_FREQ;

void sdhci_mshc_hardware_reset(struct sdhci_host *host)
{
	u32 count = 0;

	if (!host->sysctrl) {
		pr_err("%s: sysctrl is null, can't reset mmc!\n", __func__);
		return;
	}
	/* eMMC reset */
	sdhci_sctrl_writel(host, IP_RST_EMMC_ORLAN, SCTRL_SCPERRSTEN1);
	while (!(IP_RST_EMMC_ORLAN &
		 sdhci_sctrl_readl(host, SCTRL_SCPERRSTSTAT1))) {
		if (count > SDHCI_MAX_COUNT) {
			pr_err("%s: emmc reset timeout\n", __func__);
			break;
		}
		count++;
	}

	/* phy reset */
	sdhci_mmc_sys_writel(host, IP_PRST_EMMCPHY, MMC1_SYSCTRL_PERRSTEN0);
	if (host->quirks2 & SDHCI_QUIRK2_HISI_COMBO_PHY_TC)
		sdhci_i2c_writel(host, IP_RST_PHY_APB, SC_EMMC_RSTEN);
}

void sdhci_mshc_hardware_disreset(struct sdhci_host *host)
{
	int count = 0;

	if (!host->sysctrl) {
		pr_err("%s: sysctrl is null, can't reset mmc!\n", __func__);
		return;
	}

	/* phy dis-reset */
	sdhci_mmc_sys_writel(host, IP_PRST_EMMCPHY, MMC1_SYSCTRL_PERRSTDIS0);
	if (host->quirks2 & SDHCI_QUIRK2_HISI_COMBO_PHY_TC)
		sdhci_i2c_writel(host, IP_RST_PHY_APB, SC_EMMC_RSTDIS);

	/* eMMC dis-reset */
	sdhci_sctrl_writel(host, IP_DISRST_EMMC_ORLAN, SCTRL_SCPERRSTDIS1);
	while (IP_DISRST_EMMC_ORLAN & sdhci_sctrl_readl(host, SCTRL_SCPERRSTSTAT1)) {
		if (count > SDHCI_MAX_COUNT) {
			pr_err("emmc dis-reset timeout\n");
			break;
		}
		count++;
	}
}

void sdhci_combo_phy_init(struct sdhci_host *host)
{
	u32 reg_val;
	int retry = WAIT_STATUS_MAX_RETRY;

	reg_val = sdhci_phy_readl(host, COMBO_PHY_PHYINITCTRL);
	reg_val |= BIST_CLK;
	sdhci_phy_writel(host, reg_val, COMBO_PHY_PHYINITCTRL);

	/* clear data output enable delay code */
	sdhci_phy_writel(host, 0x0, COMBO_PHY_DATENA_DLY);

	reg_val = PUPD_EN_DATA | PUPD_EN_STROBE | PUPD_EN_CMD | PULLUP_DATA | PULLUP_CMD;
	sdhci_phy_writel(host, reg_val, COMBO_PHY_IOCTL_PUPD);

	/* set drv 40o */
	reg_val = EMMC_RONSEL_OR;
	sdhci_phy_writel(host, reg_val, COMBO_PHY_IOCTL_RONSEL_1);
	reg_val = sdhci_phy_readl(host, COMBO_PHY_IOCTL_RONSEL_2);
	reg_val |= EMMC_RONSEL_2;
	sdhci_phy_writel(host, reg_val, COMBO_PHY_IOCTL_RONSEL_2);

	/* enable phy input output */
	reg_val = DA_EMMC_E | DA_EMMC_IE;
	sdhci_phy_writel(host, reg_val, COMBO_PHY_IOCTL_IOE);

	sdhci_sctrl_writel(host, GT_CLK_EMMC_SCTRL, SCTRL_SCPERDIS2);
	udelay(100);
	reg_val = sdhci_phy_readl(host, COMBO_PHY_DLY_CTL1);
	reg_val |= INV_TX_CLK;
	sdhci_phy_writel(host, reg_val, COMBO_PHY_DLY_CTL1);
	sdhci_sctrl_writel(host, GT_CLK_EMMC_SCTRL, SCTRL_SCPEREN2);
	while (retry--) {
		if (GT_CLK_EMMC_SCTRL & sdhci_sctrl_readl(host, SCTRL_SCPERSTAT2))
			break;
		udelay(100);
	}
	if (retry < 0)
		pr_err("%s open emmc clk gate timeout\n", __func__);

	/* do ZQ Calibration and so on, to be finish */
	sdhci_combo_phy_zq_cal(host);
}

void sdhci_mshc_config_strobe_clk(struct sdhci_host *host, unsigned int timing)
{
	u16 reg_u16 = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	u32 reg_u32;
	u32 delaymeas_code;

	reg_u16 &= ~SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, reg_u16, SDHCI_CLOCK_CONTROL);

	if (timing == MMC_TIMING_MMC_DDR52 || timing == MMC_TIMING_MMC_HS400) {
		reg_u32 = sdhci_mmc_sys_readl(host, MMC1_SYSCTRL_CTRL0);
		reg_u32 |= CMD_DLY_EN;
		sdhci_mmc_sys_writel(host, reg_u32, MMC1_SYSCTRL_CTRL0);

		reg_u32 = sdhci_phy_readl(host, COMBO_PHY_DLY_CTL);
		delaymeas_code = reg_u32 & DLY_CODE_1T_MASK;

		/* set tx_clk_as_1/4T, cancel tx inv */
		reg_u32 = sdhci_phy_readl(host, COMBO_PHY_DLY_CTL1);
		reg_u32 &= ~(DLY_1_CODE_MASK << DLY_1_CODE_SHIFT);
		delaymeas_code = delaymeas_code / 4;
		reg_u32 |= ((DLY_1_CODE_MASK & (delaymeas_code)) << DLY_1_CODE_SHIFT);
		reg_u32 &= ~INV_TX_CLK;
		sdhci_phy_writel(host, reg_u32, COMBO_PHY_DLY_CTL1);
	} else {
		reg_u32 = sdhci_mmc_sys_readl(host, MMC1_SYSCTRL_CTRL0);
		reg_u32 &= ~CMD_DLY_EN;
		sdhci_mmc_sys_writel(host, reg_u32, MMC1_SYSCTRL_CTRL0);
	}
	reg_u16 = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	reg_u16 |= SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, reg_u16, SDHCI_CLOCK_CONTROL);
}

#ifdef CONFIG_PM_SLEEP
/*
 * sdhci_mshc_suspend - Suspend method for the driver
 * @dev: Address of the device structure
 * Returns 0 on success and error value on error
 * Put the device in a low power state.
 */
int sdhci_mshc_suspend(struct device *dev)
{
	int ret;
	struct sdhci_host *host = dev_get_drvdata(dev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_mshc_data *sdhci_mshc = pltfm_host->priv;

	dev_info(dev, "%s: suspend +\n", __func__);

	ret = sdhci_suspend_host(host);
	if (ret)
		return ret;

	clk_disable_unprepare(sdhci_mshc->clk);

	/* enable phy io retention */
	sdhci_sctrl_writel(host, RG_EMMC_LHEN_IN_RET | RG_EMMC_LHEN_MASK, SCTRL_SCPERCTRL7);

	sdhci_mshc_hardware_reset(host);

	dev_info(dev, "%s: suspend -\n", __func__);

	return 0;
}

/*
 * sdhci_mshc_resume - Resume method for the driver
 * @dev: Address of the device structure
 * Returns 0 on success and error value on error
 * Resume operation after suspend
 */
int sdhci_mshc_resume(struct device *dev)
{
	int ret;
	int retry = WAIT_STATUS_MAX_RETRY;
	struct sdhci_host *host = dev_get_drvdata(dev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_mshc_data *sdhci_mshc = pltfm_host->priv;

	dev_info(dev, "%s: resume +\n", __func__);

	sdhci_mshc_hardware_disreset(host);

	/* enable phy io normal */
	sdhci_sctrl_writel(host, RG_EMMC_LHEN_MASK, SCTRL_SCPERCTRL7);

	sdhci_sctrl_writel(host, GT_CLK_EMMC_BUS, SCTRL_SCPEREN2);
	while (retry--) {
		if (GT_CLK_EMMC_BUS & sdhci_sctrl_readl(host, SCTRL_SCPERSTAT2))
			break;
		udelay(100);
	}
	if (retry < 0)
		pr_err("%s open  emmc clk bus timeout\n", __func__);

	sdhci_sctrl_writel(host, SEL_CLK_EMMC_PPLL23 |
		SEL_CLK_PPLL23_MASK, SCTRL_SCCLKDIV10);

	/* disrest phy, cq ,hclk clock */
	sdhci_mmc_sys_writel(host, GT_CLK_EMMC_OR, MMC1_SYSCTRL_PEREN0);

	ret = clk_set_rate(sdhci_mshc->clk, xin_clk);
	if (ret)
		dev_err(dev, "Error setting desired xin_clk=%u, get clk=%lu\n",
			xin_clk, clk_get_rate(sdhci_mshc->clk));
	ret = clk_prepare_enable(sdhci_mshc->clk);
	if (ret) {
		dev_err(dev, "Cannot enable SD clock\n");
		return ret;
	}
	pr_debug("%s: sdhci_mshc->clk=%lu\n", __func__,
		clk_get_rate(sdhci_mshc->clk));

	sdhci_combo_phy_init(host);

	pr_debug("%s: host->mmc->ios.clock=%u, timing=%u\n", __func__,
		host->mmc->ios.clock, host->mmc->ios.timing);

	sdhci_mshc->need_delay_measure = true;
	ret = sdhci_resume_host(host);
	if (ret)
		return ret;
	sdhci_mshc_resume_then_tuning_move(dev);

	dev_info(dev, "%s: resume -\n", __func__);

	return 0;
}
#endif /* ! CONFIG_PM_SLEEP */

#ifdef CONFIG_PM
int sdhci_mshc_runtime_suspend(struct device *dev)
{
	struct sdhci_host *host = dev_get_drvdata(dev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_mshc_data *sdhci_mshc = pltfm_host->priv;
	int ret;

	ret = sdhci_mshc_runtime_suspend_host(host);

	if (!IS_ERR(sdhci_mshc->clk))
		clk_disable(sdhci_mshc->clk);
	udelay(100);

	/* close phy, cq ,hclk clock */
	sdhci_mmc_sys_writel(host, GT_CLK_EMMC_OR, MMC1_SYSCTRL_PERDIS0);

	return ret;
}


int sdhci_mshc_runtime_resume(struct device *dev)
{
	struct sdhci_host *host = dev_get_drvdata(dev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_mshc_data *sdhci_mshc = pltfm_host->priv;

	/* enable phy, cq ,hclk clock */
	sdhci_mmc_sys_writel(host, GT_CLK_EMMC_OR, MMC1_SYSCTRL_PEREN0);

	if (!IS_ERR(sdhci_mshc->clk)) {
		if (clk_enable(sdhci_mshc->clk))
			pr_warn("%s: clk_enable sdhci_arasan->clk failed\n", __func__);
	}

	return sdhci_mshc_runtime_resume_host(host);
}
#endif

