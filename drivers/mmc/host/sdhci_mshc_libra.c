/*
 * libra Synopsys Secure Digital Host Controller Interface.
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
#include <linux/hisi/rdr_pub.h>

#include <linux/mmc/dsm_emmc.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sd.h>
#include <linux/hisi/rpmb.h>
#include <soc_pmctrl_interface.h>
#include <mntn_subtype_exception.h>

#include "sdhci-pltfm.h"
#include "sdhci_dwc_mshc.h"

static u32 xin_clk = SDHCI_MSHC_XIN_CLK_FREQ;

void sdhci_mshc_hardware_reset(struct sdhci_host *host)
{
	u32 count = 0;

	if (!host->sysctrl) {
		pr_err("%s: sysctrl is null, can't reset mmc!\n", __func__);
		return;
	}

	/* eMMC reset */
	sdhci_sctrl_writel(host, IP_RST_EMMC, SCTRL_SCPERRSTEN0);
	while (!((unsigned int)IP_RST_EMMC &
		 sdhci_sctrl_readl(host, SCTRL_SCPERRSTSTAT0))) {
		if (count > SDHCI_MAX_COUNT) {
			pr_err("emmc reset timeout\n");
			break;
		}
		count++;
	}

	/* phy reset */
	sdhci_mmc_sys_writel(host, IP_PRST_EMMC_PHY_MASK, EMMC_SYS_CRG_CFG1);
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
	sdhci_mmc_sys_writel(host, IP_PRST_EMMC_PHY | IP_PRST_EMMC_PHY_MASK,
		EMMC_SYS_CRG_CFG1);
	if (host->quirks2 & SDHCI_QUIRK2_HISI_COMBO_PHY_TC)
		sdhci_i2c_writel(host, IP_RST_PHY_APB, SC_EMMC_RSTDIS);

	/* eMMC dis-reset */
	sdhci_sctrl_writel(host, IP_RST_EMMC, SCTRL_SCPERRSTDIS0);
	while ((unsigned int)IP_RST_EMMC &
		sdhci_sctrl_readl(host, SCTRL_SCPERRSTSTAT0)) {
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

	reg_val = sdhci_mmc_sys_readl(host, EMMC_SYS_LHEN_INB);
	reg_val |= INB_IO_LATCH_MODE;
	sdhci_mmc_sys_writel(host, reg_val, EMMC_SYS_LHEN_INB);

	reg_val = sdhci_phy_readl(host, COMBO_PHY_PHYINITCTRL);
	reg_val |= BIST_CLK;
	sdhci_phy_writel(host, reg_val, COMBO_PHY_PHYINITCTRL);

	/* clear data output enable delay code */
	sdhci_phy_writel(host, 0x0, COMBO_PHY_DATENA_DLY);

	reg_val = PUPD_EN_DATA | PUPD_EN_STROBE | PUPD_EN_CMD
			| PULLUP_DATA | PULLUP_CMD;
	sdhci_phy_writel(host, reg_val, COMBO_PHY_IOCTL_PUPD);

	/* set drv 40o */
	reg_val = EMMC_RONSEL_0;
	sdhci_phy_writel(host, reg_val, COMBO_PHY_IOCTL_RONSEL_1);
	reg_val = sdhci_phy_readl(host, COMBO_PHY_IOCTL_RONSEL_2);
	reg_val |= EMMC_RONSEL_2;
	sdhci_phy_writel(host, reg_val, COMBO_PHY_IOCTL_RONSEL_2);

	/* enable phy input output */
	reg_val = DA_EMMC_E | DA_EMMC_IE;
	sdhci_phy_writel(host, reg_val, COMBO_PHY_IOCTL_IOE);

	reg_val = sdhci_phy_readl(host, COMBO_PHY_DLY_CTL1);
	reg_val |= INV_TX_CLK;
	sdhci_phy_writel(host, reg_val, COMBO_PHY_DLY_CTL1);

	/* do ZQ Calibration and so on, to be finish */
	sdhci_combo_phy_zq_cal(host);
}

void sdhci_mshc_config_strobe_clk(struct sdhci_host *host, unsigned int timing)
{
	u32 reg_val;
	u16 reg_u16;

	reg_val = sdhci_mmc_sys_readl(host, EMMC_SYS_CTRL0);

	if (timing == MMC_TIMING_MMC_HS400)
		reg_val &= ~GT_STROBE_RX_CLK;
	else
		reg_val |= GT_STROBE_RX_CLK;

	sdhci_mmc_sys_writel(host, reg_val, EMMC_SYS_CTRL0);

	if (timing == MMC_TIMING_MMC_DDR52 || timing == MMC_TIMING_MMC_HS400) {
		reg_u16 = sdhci_readw(host, SDHCI_HOST_CONTROL2);
		reg_u16 |= SDHCI_CTRL_INV_DATA;
		sdhci_writew(host, reg_u16, SDHCI_HOST_CONTROL2);
	}
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
	u32 reg_val;

	dev_info(dev, "%s: suspend +\n", __func__);

	ret = sdhci_suspend_host(host);
	if (ret)
		return ret;

	/* enable phy io retention */
	sdhci_mmc_sys_writel(host, 0, EMMC_SYS_LHEN_INB);
	sdhci_mmc_sys_writel(host, IN_IO_LATCH_MODE, EMMC_SYS_LHEN_IN);

	clk_disable_unprepare(sdhci_mshc->clk);

	reg_val = sdhci_phy_readl(host, COMBO_PHY_DLY_CTL1);
	reg_val &= ~INV_TX_CLK;
	sdhci_phy_writel(host, reg_val, COMBO_PHY_DLY_CTL1);

	/* close phy, cq ,hclk clock */
	sdhci_mmc_sys_writel(host, EMMC_SYS_GT_CLK_MASK, EMMC_SYS_CRG_CFG1);

	sdhci_mshc_hardware_reset(host);

	/* enable controller and phy ISO */
	sdhci_mmc_sys_writel(host, CTRL_ISO_EN | PHY_ISO_EN, EMMC_SYS_PHY_ISOEN);

	/* close controller and phy power */
	sdhci_mmc_sys_writel(host, 0, EMMC_SYS_MTCMOS_EN);

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
	struct sdhci_host *host = dev_get_drvdata(dev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_mshc_data *sdhci_mshc = pltfm_host->priv;
	u32 reg_val;
	u32 count = 0;

	dev_info(dev, "%s: resume +\n", __func__);

	/* open controller and phy power */
	sdhci_mmc_sys_writel(host, PHY_MTCMOS_EN | CTRL_MTCMOS_EN, EMMC_SYS_MTCMOS_EN);
	do {
		if (count++ > 1000) { /* wait power on 1ms */
			pr_err("eMMC wait power on timeout\n");
			break;
		}
		udelay(1);
		reg_val = sdhci_mmc_sys_readl(host, EMMC_SYS_MTCMOS_ACK);
	} while (!((reg_val & PHY_MTCMOS_ACK) && (reg_val & CTRL_MTCMOS_ACK)));

	/* close controller and phy ISO */
	sdhci_mmc_sys_writel(host, 0, EMMC_SYS_PHY_ISOEN);

	sdhci_mshc_hardware_disreset(host);

	/* enable phy, cq ,hclk clock */
	sdhci_mmc_sys_writel(host, EMMC_SYS_GT_CLK_MASK | EMMC_SYS_GT_CLK,
		EMMC_SYS_CRG_CFG1);

	reg_val = sdhci_phy_readl(host, COMBO_PHY_DLY_CTL1);
	reg_val |= INV_TX_CLK;
	sdhci_phy_writel(host, reg_val, COMBO_PHY_DLY_CTL1);

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

	/* close phy io retention */
	sdhci_mmc_sys_writel(host, 0, EMMC_SYS_LHEN_IN);
	sdhci_mmc_sys_writel(host, INB_IO_LATCH_MODE, EMMC_SYS_LHEN_INB);

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
	u32 reg_val;
	u16 reg_u16;

	ret = sdhci_mshc_runtime_suspend_host(host);

	reg_u16 = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	reg_u16 &= ~SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, reg_u16, SDHCI_CLOCK_CONTROL);
	udelay(5);

	reg_val = sdhci_phy_readl(host, COMBO_PHY_DLY_CTL1);
	reg_val &= ~INV_TX_CLK;
	sdhci_phy_writel(host, reg_val, COMBO_PHY_DLY_CTL1);

	if (!IS_ERR(sdhci_mshc->clk))
		clk_disable(sdhci_mshc->clk);

	/* close phy, cq ,hclk clock */
	sdhci_mmc_sys_writel(host, EMMC_SYS_GT_CLK_MASK, EMMC_SYS_CRG_CFG1);

	return ret;
}


int sdhci_mshc_runtime_resume(struct device *dev)
{
	struct sdhci_host *host = dev_get_drvdata(dev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_mshc_data *sdhci_mshc = pltfm_host->priv;
	u32 reg_val;

	/* enable phy, cq ,hclk clock */
	sdhci_mmc_sys_writel(host, EMMC_SYS_GT_CLK_MASK | EMMC_SYS_GT_CLK, EMMC_SYS_CRG_CFG1);

	if (!IS_ERR(sdhci_mshc->clk)) {
		if (clk_enable(sdhci_mshc->clk))
			pr_warn("%s: clk_enable sdhci_arasan->clk failed\n", __func__);
	}

	reg_val = sdhci_phy_readl(host, COMBO_PHY_DLY_CTL1);
	reg_val |= INV_TX_CLK;
	sdhci_phy_writel(host, reg_val, COMBO_PHY_DLY_CTL1);

	return sdhci_mshc_runtime_resume_host(host);
}
#endif

#ifdef CONFIG_HISI_BB
int sdhci_coldboot_rdr_regester(void)
{
	struct rdr_exception_info_s einfo;
	u32 ret;

	/* initialization failure exception registration. */
	memset(&einfo, 0, sizeof(struct rdr_exception_info_s));
	/* initialization failed RDR_MODID_MMC_COLDBOOT */
	einfo.e_modid = RDR_MODID_MMC_COLDBOOT;
	einfo.e_modid_end = RDR_MODID_MMC_COLDBOOT;
	/* highest processing level */
	einfo.e_process_priority = RDR_ERR;
	/* restart now */
	einfo.e_reboot_priority = RDR_REBOOT_NOW;
	/* notify the AP to save the log. To be modified */
	einfo.e_notify_core_mask = RDR_AP;
	/* notify the AP to reset the status and reset system */
	einfo.e_reset_core_mask = RDR_AP;
	/* exception re-entry is not allowed(no duplicate processing). */
	einfo.e_reentrant = (u32)RDR_REENTRANT_DISALLOW;
	/* exception type initialization failed */
	einfo.e_exce_type = MMC_S_EXCEPTION;
	einfo.e_exce_subtype = MMC_EXCEPT_COLDBOOT;
	/* exceptions occur at eMMC. */
	einfo.e_from_core = RDR_EMMC;
	einfo.e_upload_flag = (u32)RDR_UPLOAD_NO;
	memcpy(einfo.e_from_module, "RDR_MMC_COLDBOOT", sizeof("RDR_MMC_COLDBOOT"));
	memcpy(einfo.e_desc, "RDR_MMC_RESET fail.", sizeof("RDR_MMC_RESET fail."));

	ret = rdr_register_exception(&einfo);
	if (ret != RDR_MODID_MMC_COLDBOOT) {
		pr_err(" register exception faild [0x%x]\n", einfo.e_modid);
		return -1;
	}

	return 0;
}
#endif

#ifdef CONFIG_HUAWEI_EMMC_DSM
void sdhci_dsm_set_host_status(struct sdhci_host *host, u32 error_bits)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_mshc_data *sdhci_mshc = pltfm_host->priv;

	sdhci_mshc->cmd_data_status |= error_bits;
}

void sdhci_dsm_work(struct work_struct *work)
{
	struct mmc_card *card = NULL;
	struct sdhci_mshc_data *sdhci_mshc = container_of(work, struct sdhci_mshc_data, dsm_work);
	struct sdhci_host *host = (struct sdhci_host *)sdhci_mshc->data;
	u32 error_bits, opcode;
	u64 para;
	unsigned long flags;

	spin_lock_irqsave(&sdhci_mshc->sdhci_dsm_lock, flags);
	para = sdhci_mshc->para;
	sdhci_mshc->para = 0;
	spin_unlock_irqrestore(&sdhci_mshc->sdhci_dsm_lock, flags);
	card = host->mmc->card;
	opcode = para & SDHCI_ERR_CMD_INDEX_MASK;
	error_bits = ((para >> SDHCI_DSM_ERR_INT_STATUS) & SDHCI_DMD_ERR_MASK);
	if (error_bits & SDHCI_INT_TIMEOUT)
		DSM_EMMC_LOG(card, DSM_EMMC_RW_TIMEOUT_ERR,
			"opcode:%u failed, status:0x%x\n", opcode, error_bits);
	else if (error_bits & SDHCI_INT_DATA_CRC)
		DSM_EMMC_LOG(card, DSM_EMMC_DATA_CRC,
			"opcode:%u failed, status:0x%x\n", opcode, error_bits);
	else if (error_bits & SDHCI_INT_CRC)
		DSM_EMMC_LOG(card, DSM_EMMC_COMMAND_CRC,
			"opcode:%u failed, status:0x%x\n", opcode, error_bits);
	else
		DSM_EMMC_LOG(card, DSM_EMMC_HOST_ERR,
			"opcode:%u failed, status:0x%x\n", opcode, error_bits);
}

static inline void sdhci_dsm_host_error_filter(
	struct sdhci_host *host, struct mmc_request *mrq, u32 *error_bits)
{
	if (mrq->cmd) {
		if (mrq->cmd->opcode == MMC_SEND_TUNING_BLOCK_HS200) {
			*error_bits = 0;
		} else if (host->mmc->ios.clock <= 400000UL) { /* set mmc clk 400k */
			if (((mrq->cmd->opcode == SD_IO_RW_DIRECT) ||
				    (mrq->cmd->opcode == SD_SEND_IF_COND) ||
				    (mrq->cmd->opcode == SD_IO_SEND_OP_COND) ||
				    (mrq->cmd->opcode == MMC_APP_CMD)))
				*error_bits = 0;
			else if (mrq->cmd->opcode == MMC_SEND_STATUS)
				*error_bits &= ~SDHCI_INT_CRC;
		}
	}
}

void sdhci_dsm_handle(struct sdhci_host *host, struct mmc_request *mrq)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_mshc_data *sdhci_mshc = pltfm_host->priv;
	u32 error_bits = sdhci_mshc->cmd_data_status;

	if (error_bits) {
		sdhci_mshc->cmd_data_status = 0;
		sdhci_dsm_host_error_filter(host, mrq, &error_bits);
		if (error_bits) {
			sdhci_mshc->para = (((unsigned long)error_bits << SDHCI_DSM_ERR_INT_STATUS) |
				((mrq->cmd ? mrq->cmd->opcode : 0) & SDHCI_ERR_CMD_INDEX_MASK));
			queue_work(system_freezable_wq, &sdhci_mshc->dsm_work);
		}
	}
}

void sdhci_dsm_report(struct mmc_host *host, struct mmc_request *mrq)
{
	struct sdhci_host *sdhci_host = mmc_priv(host);

	sdhci_dsm_set_host_status(sdhci_host, SDHCI_INT_TIMEOUT);
	sdhci_dsm_handle(sdhci_host, mrq);
}
#endif
