/*
 * ufs-cancer.c
 *
 * ufs config for cancer
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#define pr_fmt(fmt) "ufshcd :" fmt

#include "dsm_ufs.h"
#include "ufs-kirin.h"
#include "ufshcd.h"
#include <linux/hisi/hisi_idle_sleep.h>
#include <linux/mfd/hisi_pmic.h>
#include <pmic_interface.h>
#include <soc_sctrl_interface.h>
#include <soc_ufs_sysctrl_interface.h>

void ufs_kirin_regulator_init(struct ufs_hba *hba)
{
}

void ufs_clk_init(struct ufs_hba *hba)
{
}

static void ufs_kirin_subsys_clk_config(struct ufs_kirin_host *host)
{
	writel(BIT(SOC_SCTRL_SCPERDIS4_gt_clk_ufs_subsys_START),
	       SOC_SCTRL_SCPERDIS4_ADDR(host->sysctrl));
	writel(0x003F0007, SOC_SCTRL_SCCLKDIV9_ADDR(host->sysctrl));
	writel(BIT(SOC_SCTRL_SCPEREN4_gt_clk_ufs_subsys_START),
	       SOC_SCTRL_SCPEREN4_ADDR(host->sysctrl));
}

static void ufs_kirin_device_reset_and_disreset(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = (struct ufs_kirin_host *)hba->priv;
	u32 reg = 0;

	if (host->caps & USE_HISI_MPHY_TC) {
		/* enable Device Reset BIT 6 for SC_RSTDIS */
		ufs_i2c_writel(hba, (unsigned int) BIT(6), SC_RSTDIS);
		ufs_i2c_readl(hba, &reg, SC_UFS_REFCLK_RST_PAD);
		/* BIT 2 | 10 for mask */
		reg = reg & (~(BIT(2) | BIT(10)));

		/*
		 * output enable, For EMMC to high dependence, open
		 * DA_UFS_OEN_RST and DA_UFS_OEN_REFCLK
		 */
		ufs_i2c_writel(hba, reg, SC_UFS_REFCLK_RST_PAD);
		/* delay 2 ms for ufs_i2c_write completed */
		mdelay(2);

		/* BIT 6 for disable Device Reset */
		ufs_i2c_writel(hba, (unsigned int) BIT(6), SC_RSTEN);
	} else {
		ufs_sys_ctrl_writel(host, MASK_UFS_DEVICE_RESET | 0,
				    UFS_DEVICE_RESET_CTRL); /* reset device */
		ufshcd_vops_vcc_power_on_off(hba);
		mdelay(1);
		/* disable Device Reset */
		ufs_sys_ctrl_writel(host,
			MASK_UFS_DEVICE_RESET | BIT_UFS_DEVICE_RESET,
			UFS_DEVICE_RESET_CTRL);
	}
	/* delay 10 ms to let the pulse propagate */
	mdelay(10);
}

static void ufs_kirin_bypass_ufs_clk_gate(struct ufs_kirin_host *host)
{
	ufs_sys_ctrl_set_bits(host, MASK_UFS_CLK_GATE_BYPASS,
		CLOCK_GATE_BYPASS);
	ufs_sys_ctrl_set_bits(host, MASK_UFS_SYSCRTL_BYPASS, UFS_SYSCTRL);
}

static void ufs_kirin_psw_config(struct ufs_kirin_host *host)
{
	/* open psw clk */
	ufs_sys_ctrl_set_bits(host, BIT_SYSCTRL_PSW_CLK_EN, PSW_CLK_CTRL);
	/* disable ufshc iso */
	ufs_sys_ctrl_clr_bits(host, BIT_UFS_PSW_ISO_CTRL, PSW_POWER_CTRL);
	/* phy iso is not effective */
	/* disable phy iso */
	ufs_sys_ctrl_clr_bits(host, BIT_UFS_PHY_ISO_CTRL, PHY_ISO_EN);
	/* notice iso disable */
	ufs_sys_ctrl_clr_bits(host, BIT_SYSCTRL_LP_ISOL_EN, HC_LP_CTRL);
}

static void ufs_kirin_check_hisi_mphy_and_bypass_sram(
	struct ufs_kirin_host *host)
{
	if (!(host->caps & USE_HISI_MPHY_TC)) {
		/* enable the MPHY's sram_bypass */
		ufs_sys_ctrl_clr_bits(
			host, PHY_SRAM_BYPASS_BIT, UFS_SYS_PHY_SRAM_MEM_CTRL_S);
		/* disable sram_ext_ld_done */
		ufs_sys_ctrl_clr_bits(host, SRAM_EXT_LD_DONE_BIT,
				      UFS_SYS_PHY_SRAM_MEM_CTRL_S);
	}
}

void ufs_soc_init(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = NULL;
	u32 reg;

	dev_info(hba->dev, "%s ++\n", __func__);

	host = (struct ufs_kirin_host *)hba->priv;

	/* eS LOW TEMP 207M */
	ufs_kirin_subsys_clk_config(host);

	ufs_kirin_check_hisi_mphy_and_bypass_sram(host);

	writel(1 << SOC_UFS_Sysctrl_UFS_UMECTRL_ufs_ies_en_mask_START,
	       SOC_UFS_Sysctrl_UFS_UMECTRL_ADDR(host->ufs_sys_ctrl));

	/* 16 bit shift for mask */
	writel((1 << (SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START + 16)) | 0,
	       SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));

	/* HC_PSW powerup */
	ufs_sys_ctrl_set_bits(host, BIT_UFS_PSW_MTCMOS_EN, PSW_POWER_CTRL);
	/* delay 10 us for HC_PSW powerup completed */
	udelay(10);

	/* notify PWR ready */
	ufs_sys_ctrl_set_bits(host, BIT_SYSCTRL_PWR_READY, HC_LP_CTRL);
	/* STEP 4 CLOSE MPHY REF CLOCK */
	ufs_sys_ctrl_clr_bits(host, BIT_SYSCTRL_REF_CLOCK_EN, PHY_CLK_CTRL);
	reg = ufs_sys_ctrl_readl(host, PHY_CLK_CTRL);
	reg = reg & (~MASK_SYSCTRL_REF_CLOCK_SEL) &
	      (~MASK_SYSCTRL_CFG_CLOCK_FREQ);
	if (host->caps & USE_HISI_MPHY_TC) {
		/* sysctrl ref clk for FPGA */
		reg = reg | 0x14;
	} else {
		/* syscfg clk is fixed 38.4Mhz */
		reg = reg | 0x26;
		/* set ref clk freq 38.4Mhz */
		reg = reg | SYSCTRL_REF_CLOCK_SEL;
	}
	ufs_sys_ctrl_writel(host, reg, PHY_CLK_CTRL); /* set cfg clk freq */

	if (!(host->caps & USE_HISI_MPHY_TC))
		/*
		 * Enable: pcs_pwr_stable_sc,pma_pwr_en_sc. Disable:
		 * pma_pwr_stable_sc,ref_clk_en_app, ref_clk_en_unipro
		 */
		ufs_sys_ctrl_writel(host, 0x4300, UFS_SYS_UFS_POWER_GATING);

	/* bypass ufs clk gate */
	ufs_kirin_bypass_ufs_clk_gate(host);

	ufs_kirin_psw_config(host);

	/* 16 bit shift for mask */
	writel((1 << (SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_arst_ufs_START + 16)) |
	       (1 << SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_arst_ufs_START),
	       SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));

	/* disable lp_reset_n */
	ufs_sys_ctrl_set_bits(host, BIT_SYSCTRL_LP_RESET_N, RESET_CTRL_EN);
	mdelay(1);

	/* open clock of M-PHY */
	ufs_sys_ctrl_set_bits(host, BIT_SYSCTRL_REF_CLOCK_EN, PHY_CLK_CTRL);

	ufs_kirin_device_reset_and_disreset(hba);

	/* disable EC bypass, make effective */
	ufs_sys_ctrl_clr_bits(host, WDP_BYPASS_EC_BIT, PHY_ISO_EN);
	ufs_sys_ctrl_set_bits(host, OVERALL_BYPASS_EC_BIT, UFS_SYS_MK2_CTRL);

	/* 16 bit shift for mask */
	writel((1 << (SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START + 16)) |
			(1 << SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START),
		SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));

	reg = readl(SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));
	if (reg & (1 << SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START))
		mdelay(1);

	/* set SOC_SCTRL_SCBAKDATA11_ADDR ufs bit to 1 when init */
	if (!ufshcd_is_auto_hibern8_allowed(hba))
		hisi_idle_sleep_vote(ID_UFS, 1);

	dev_info(hba->dev, "%s --\n", __func__);
}

int ufs_kirin_suspend_before_set_link_state(struct ufs_hba *hba,
	enum ufs_pm_op pm_op)
{
#ifdef FEATURE_KIRIN_UFS_PSW
	struct ufs_kirin_host *host = (struct ufs_kirin_host *)hba->priv;

	if (ufshcd_is_runtime_pm(pm_op))
		return 0;

	/* step1:store BUSTHRTL register */
	host->busthrtl_backup = ufshcd_readl(hba, UFS_REG_OCPTHRTL);
	/* enable PowerGating */
	ufshcd_rmwl(hba, LP_PGE, LP_PGE, UFS_REG_OCPTHRTL);
#endif
	return 0;
}

int ufs_kirin_resume_after_set_link_state(struct ufs_hba *hba,
	enum ufs_pm_op pm_op)
{
#ifdef FEATURE_KIRIN_UFS_PSW
	struct ufs_kirin_host *host = (struct ufs_kirin_host *)hba->priv;

	if (ufshcd_is_runtime_pm(pm_op))
		return 0;
	ufshcd_writel(hba, host->busthrtl_backup, UFS_REG_OCPTHRTL);
#endif
	return 0;
}

int ufs_kirin_suspend(struct ufs_hba *hba, enum ufs_pm_op pm_op)
{
	struct ufs_kirin_host *host = (struct ufs_kirin_host *)hba->priv;

	/* set SOC_SCTRL_SCBAKDATA11_ADDR ufs bit to 0 when idle */
	if (!ufshcd_is_auto_hibern8_allowed(hba))
		hisi_idle_sleep_vote(ID_UFS, 0);

	if (ufshcd_is_runtime_pm(pm_op))
		return 0;

	if (host->in_suspend) {
		WARN_ON(1);
		return 0;
	}

	ufs_sys_ctrl_clr_bits(host, BIT_SYSCTRL_REF_CLOCK_EN, PHY_CLK_CTRL);
	/* delay 10 us for ufs_sys_ctrl_clr_bits completed */
	udelay(10);
	hisi_pmic_reg_write(PMIC_CLK_UFS_EN_ADDR(0), 0);

	host->in_suspend = true;

	return 0;
}

int ufs_kirin_resume(struct ufs_hba *hba, enum ufs_pm_op pm_op)
{
	struct ufs_kirin_host *host = (struct ufs_kirin_host *)hba->priv;

	/* set SOC_SCTRL_SCBAKDATA11_ADDR ufs bit to 1 when busy */
	if (!ufshcd_is_auto_hibern8_allowed(hba))
		hisi_idle_sleep_vote(ID_UFS, 1);

	if (!host->in_suspend)
		return 0;
	hisi_pmic_reg_write(PMIC_CLK_UFS_EN_ADDR(0), 1);
	/* 250us to ensure the clk stable */
	udelay(250);
	ufs_sys_ctrl_set_bits(host, BIT_SYSCTRL_REF_CLOCK_EN, PHY_CLK_CTRL);

	host->in_suspend = false;
	return 0;
}

void ufs_kirin_device_hw_reset(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = (struct ufs_kirin_host *)hba->priv;

	if (likely(!(host->caps & USE_HISI_MPHY_TC)))
		ufs_sys_ctrl_writel(
			host, MASK_UFS_DEVICE_RESET | 0, UFS_DEVICE_RESET_CTRL);
	else
		/* BIT 6 for SC_RSTDIS */
		ufs_i2c_writel(hba, (unsigned int)BIT(6), SC_RSTDIS);
	ufshcd_vops_vcc_power_on_off(hba);
	mdelay(1);
	if (likely(!(host->caps & USE_HISI_MPHY_TC)))
		ufs_sys_ctrl_writel(host,
			MASK_UFS_DEVICE_RESET | BIT_UFS_DEVICE_RESET,
			UFS_DEVICE_RESET_CTRL);
	else
		/* BIT 6 for SC_RSTEN */
		ufs_i2c_writel(hba, (unsigned int)BIT(6), SC_RSTEN);
	/* some device need at least 40ms */
	mdelay(40);
}

/* Workaround: PWM-amplitude reduce & PMC and H8's glitch */
static void ufs_kirin_mphy_amplitude_glitch_workaround(struct ufs_hba *hba)
{
	uint16_t value3;
	uint16_t value4;
	uint32_t reg;
	struct ufs_kirin_host *host = (struct ufs_kirin_host *)hba->priv;
	/* 2 count for tx_ana_ctrl_leg_pull_en and tx_ana_ctrl_post */
	uint16_t (*table)[2] = NULL;

	/* 2 count for tx_ana_ctrl_leg_pull_en and tx_ana_ctrl_post */
	uint16_t table_0db[][2] = { { 252, 0 },  { 252, 0 },  { 252, 0 },
				    { 255, 0 },  { 255, 0 },  { 1020, 0 },
				    { 1020, 0 }, { 1023, 0 }, { 1023, 0 },
				    { 4092, 0 }, { 4092, 0 } };
	/* 2 count for tx_ana_ctrl_leg_pull_en and tx_ana_ctrl_post */
	uint16_t table_35db[][2] = { { 252, 3 },  { 252, 3 },  { 252, 3 },
				     { 255, 3 },  { 255, 6 },  { 1020, 6 },
				     { 1020, 6 }, { 1023, 6 }, { 1023, 6 },
				     { 4092, 6 }, { 4092, 7 } };
	/* 2 count for tx_ana_ctrl_leg_pull_en and tx_ana_ctrl_post */
	uint16_t table_60db[][2] = { { 252, 6 },   { 252, 7 },   { 252, 7 },
				     { 255, 7 },   { 255, 14 },  { 1020, 14 },
				     { 1020, 14 }, { 1023, 14 }, { 1023, 15 },
				     { 4092, 15 }, { 4092, 15 } };

	table = &table_35db[0];
	if (host->tx_equalizer == 0)
		table = &table_0db[0];
	else if (host->tx_equalizer == TX_EQUALIZER_60DB)
		table = &table_60db[0];

	reg = ufs_kirin_mphy_read(hba, 0x200C);
	reg = reg & 0xF; /* RAWCMN_DIG_TX_CAL_CODE[3:0] */
	if (reg >= ARRAY_SIZE(table_35db))
		reg = 0;
	value3 = table[reg][0] << 1;
	value4 = table[reg][1];
	/* LANEN_DIG_ANA_TX_EQ_OVRD_OUT_0[14:1](tx_ana_ctrl_leg_pull_en) */
	ufs_kirin_mphy_write(hba, 0x10A3, value3);
	/* LANEN_DIG_ANA_TX_EQ_OVRD_OUT_0[14:1](tx_ana_ctrl_leg_pull_en) */
	ufs_kirin_mphy_write(hba, 0x11A3, value3);
	/* LANEN_DIG_ANA_TX_EQ_OVRD_OUT_3[8:0](tx_ana_ctrl_post) */
	ufs_kirin_mphy_write(hba, 0x10A6, value4);
	/* LANEN_DIG_ANA_TX_EQ_OVRD_OUT_3[8:0](tx_ana_ctrl_post) */
	ufs_kirin_mphy_write(hba, 0x11A6, value4);

	/* LANEN_DIG_ANA_TX_EQ_OVRD_OUT_1 */
	ufs_kirin_mphy_write(hba, 0x10A4, 0);
	/* LANEN_DIG_ANA_TX_EQ_OVRD_OUT_1 */
	ufs_kirin_mphy_write(hba, 0x11A4, 0);
	/* LANEN_DIG_ANA_TX_EQ_OVRD_OUT_2 */
	ufs_kirin_mphy_write(hba, 0x10A5, 0);
	/* LANEN_DIG_ANA_TX_EQ_OVRD_OUT_2 */
	ufs_kirin_mphy_write(hba, 0x11A5, 0);
	/* LANEN_DIG_ANA_TX_EQ_OVRD_OUT_4 */
	ufs_kirin_mphy_write(hba, 0x10A7, 0);
	/* LANEN_DIG_ANA_TX_EQ_OVRD_OUT_4 */
	ufs_kirin_mphy_write(hba, 0x11A7, 0);
	/* LANEN_DIG_ANA_TX_EQ_OVRD_OUT_5 */
	ufs_kirin_mphy_write(hba, 0x10A8, 0);
	/* LANEN_DIG_ANA_TX_EQ_OVRD_OUT_5 */
	ufs_kirin_mphy_write(hba, 0x11A8, 0);
	/* shift 15 for spec bit */
	value3 |= (1 << 15);
	/* LANEN_DIG_ANA_TX_EQ_OVRD_OUT_0[15] = 1 */
	ufs_kirin_mphy_write(hba, 0x10A3, value3);
	/* LANEN_DIG_ANA_TX_EQ_OVRD_OUT_0[15] = 1 */
	ufs_kirin_mphy_write(hba, 0x11A3, value3);

	/* LANEN_DIG_ANA_TX_EQ_OVRD_OUT_0[0](TX_ANA_LOAD_CLK) */
	ufs_kirin_mphy_write(hba, 0x10A3, value3 | 1);
	/* LANEN_DIG_ANA_TX_EQ_OVRD_OUT_0[0](TX_ANA_LOAD_CLK) */
	ufs_kirin_mphy_write(hba, 0x11A3, value3 | 1);
	/* LANEN_DIG_ANA_TX_EQ_OVRD_OUT_0[0](TX_ANA_LOAD_CLK) */
	ufs_kirin_mphy_write(hba, 0x10A3, value3);
	/* LANEN_DIG_ANA_TX_EQ_OVRD_OUT_0[0](TX_ANA_LOAD_CLK) */
	ufs_kirin_mphy_write(hba, 0x11A3, value3);
}

static void ufs_kirin_config_rx_status(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = (struct ufs_kirin_host *)hba->priv;
	uint32_t value1 = 0;
	uint32_t value2 = 0;

	if (host->caps & RX_CANNOT_DISABLE) {
		ufshcd_dme_get(hba, UIC_ARG_MIB_SEL(RXOVRCTRL1, 0x4), &value1);
		ufshcd_dme_get(hba, UIC_ARG_MIB_SEL(RXOVRCTRL1, 0x5), &value2);
		/*
		 * bit[5:4] = 2b'00, not do override, let the FSM control the
		 *            RX status, normally during H8, the RX will be
		 *            disabled to save power. CS chip will use this
		 *            configure, which is default also.
		 * bit[5:4] = 2b'01, do override, not disable RX in any status,
		 *            include H8, which cause high power consume,
		 *            ES chip need this bugfix, otherwise the RX
		 *            will not work again if enabled after disable.
		 * bit[5:4] = 2b'11, do override, disable RX in any status,
		 *            link startup will fail if configured this.
		 */
		value1 |= BIT_RX_DISABLE_OVR_EN_WR;
		value2 |= BIT_RX_DISABLE_OVR_EN_WR;
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RXOVRCTRL1, 0x4), value1);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RXOVRCTRL1, 0x5), value2);
	}
}

static void ufs_kirin_unipro_vs_mphy_disable(struct ufs_hba *hba)
{
	uint32_t value1 = 0;

	ufshcd_dme_get(hba, UIC_ARG_MIB_SEL(0xD0C1, 0x0), &value1);
	if (value1 != 0x1)
		pr_warn("Warring!!! Unipro VS_mphy_disable is 0x%x\n", value1);
}

static int ufs_kirin_check_is_h8(struct ufs_hba *hba, int err_msg)
{
	int err = err_msg;

	if (likely(!hba->host->is_emulator)) {
		err = ufs_kirin_check_hibern8(hba);
		if (err)
			pr_err("ufs_kirin_check_hibern8 error\n");
		return err;
	}
	return err;
}

static void ufs_kirin_ctrl_efuse_rhold(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = (struct ufs_kirin_host *)hba->priv;

	if (ufs_sctrl_readl(host, SCDEEPSLEEPED_OFFSET) & EFUSE_RHOLD_BIT) {
		/* MPHY RXRHOLDCTRLOPT */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RXRHOLDCTRLOPT, 0x4), 0x2);
		/* MPHY RXRHOLDCTRLOPT */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RXRHOLDCTRLOPT, 0x5), 0x2);
	} else {
		/* MPHY RXRHOLDCTRLOPT */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RXRHOLDCTRLOPT, 0x4), 0);
		/* MPHY RXRHOLDCTRLOPT */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RXRHOLDCTRLOPT, 0x5), 0);
	}
}

static void ufs_kirin_set_rx_hs_g1_prepare_length_capability(
	struct ufs_hba *hba)
{
	/* RX_HS_G1_PREPARE_LENGTH_CAPABILITY */
	/* Gear1 Synclength */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RX_HS_G1_PREPARE_LENGTH_CAPABILITY, 0x4), 0xF);
	/* Gear1 Synclength */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RX_HS_G1_PREPARE_LENGTH_CAPABILITY, 0x5), 0xF);
	/* Unipro VS_MphyCfgUpdt */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD085, 0x0), 0x1);
}

static void ufs_kirin_pa_tx_hs_adapt_type_config(struct ufs_hba *hba)
{
	/* PA_TxHsAdaptType: no adapt */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_TXHSADAPTTYPE, 0x0), 0x3);
	/* PA_TxHsAdaptTypeInPa_Init: no adapt */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_ADAPTAFTERLRSTINPA_INIT, 0x0), 0x3);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(TX_AMPLITUDE_CAPABILITY, 0x0), 0x2); /* only LA */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(TX_AMPLITUDE_CAPABILITY, 0x1), 0x2); /* only LA */
	/* Unipro VS_MphyCfgUpdt */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD085, 0x0), 0x1);
}

static void ufs_kirin_slow_process(struct ufs_hba *hba)
{
	uint32_t value1;
	uint32_t value2;

	value1 = ufs_kirin_mphy_read(hba, 0x401e);
	value2 = ufs_kirin_mphy_read(hba, 0x411e);
	ufs_kirin_mphy_write(hba, 0x401e, value1 | 0x1);
	ufs_kirin_mphy_write(hba, 0x411e, value2 | 0x1);
	value1 = ufs_kirin_mphy_read(hba, 0x401f);
	value2 = ufs_kirin_mphy_read(hba, 0x411f);
	ufs_kirin_mphy_write(hba, 0x401f, value1 | 0x1);
	ufs_kirin_mphy_write(hba, 0x411f, value2 | 0x1);
}

static void ufs_kirin_set_synclength(struct ufs_hba *hba)
{
	/* Gear3 Synclength */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RX_HS_G3_SYNC_LENGTH_CAPABILITY, 0x4), 0x4F);
	/* Gear3 Synclength */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RX_HS_G3_SYNC_LENGTH_CAPABILITY, 0x5), 0x4F);

	/* Gear2 Synclength */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RX_HS_G2_SYNC_LENGTH_CAPABILITY, 0x4), 0x4F);
	/* Gear2 Synclength */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RX_HS_G2_SYNC_LENGTH_CAPABILITY, 0x5), 0x4F);

	/* Gear1 Synclength */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RX_HS_G1_SYNC_LENGTH_CAPABILITY, 0x4), 0x4F);
	/* Gear1 Synclength */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RX_HS_G1_SYNC_LENGTH_CAPABILITY, 0x5), 0x4F);
}

/* hci asic mphy specific configuration */
static int ufs_kirin_dme_setup_hci_asic_mphy(struct ufs_hba *hba)
{
	int err;

	struct ufs_kirin_host *host = (struct ufs_kirin_host *)hba->priv;

	pr_info("%s ++\n", __func__);

	/* Unipro VS_mphy_disable */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD0C1, 0x0), 0x1);

	ufs_kirin_config_rx_status(hba);

	ufs_kirin_ctrl_efuse_rhold(hba);

	/* Unipro VS_MphyCfgUpdt */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD085, 0x0), 0x1);

	/* MPHY CBREFCLKCTRL2, indicate refclk is open when calibration */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBREFCLKCTRL2, 0x0), 0x80);
	/* Unipro VS_MphyCfgUpdt */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD085, 0x0), 0x1);

	/* MPHY CBCRCTRL: enable CR port */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBCRCTRL, 0x0), 0x1);
	/* Unipro VS_MphyCfgUpdt */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD085, 0x0), 0x1);

	/*
	 * Workaround: PWM-amplitude reduce & PMC and H8's glitch clean begin
	 * RAWCMN_DIG_AON_CMN_SUP_OVRD_IN[5:4] = 2'b11,open phy clk during H8
	 *
	 * Workaround: PWM-amplitude reduce & PMC and H8's glitch clean end
	 */
	ufs_kirin_mphy_write(hba, 0x203B, 0x30);

	/*
	 * Workaround: clear P-N abnormal common voltage
	 * LANEN_ANA_TX_OVRD_MEAS[5]=0,LANEN_DIG_ANA_TX_OVRD_MEAS[4]=1
	 */
	ufs_kirin_mphy_write(hba, 0x10e0, 0x10);
	ufs_kirin_mphy_write(hba, 0x11e0, 0x10);

	/* close AFE calibration */
	ufs_kirin_mphy_write(hba, 0x401c, 0x0004);
	ufs_kirin_mphy_write(hba, 0x411c, 0x0004);

	/* slow process */
	ufs_kirin_slow_process(hba);

	err = ufs_update_hc_fw(hba);
	if (err) {
		pr_err("phy firmware update error\n");
		return err;
	}
	/* PA_HSSeries */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_HSSERIES, 0x0), 0x2);
	/* MPHY CBRATESEL */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBRATESEL, 0x0), 0x1);
	/* Unipro VS_MphyCfgUpdt */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD085, 0x0), 0x1);

	/* MPHY RXSQCONTROL rx0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RXSQCONTROL, 0x4), 0x1);
	/* MPHY RXSQCONTROL rx1 */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RXSQCONTROL, 0x5), 0x1);
	/* Unipro VS_MphyCfgUpdt */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD085, 0x0), 0x1);

	if (host->caps & RX_VCO_VREF)
		/* SUP_ANA_BG1: rx_vco_vref = 501mV */
		ufs_kirin_mphy_write(hba, 0x0042, 0x28);
	/* CBENBLCPBATTRWR */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBENBLCPBATTRWR, 0x0), 0x1);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD085, 0x0), 0x1);

	ufs_kirin_set_rx_hs_g1_prepare_length_capability(hba);

	ufs_kirin_pa_tx_hs_adapt_type_config(hba);

	/* RX_Hibern8Time_Capability */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RX_HIBERN8TIME_CAPABILITY, 0x4), 0xA);
	/* RX_Hibern8Time_Capability */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RX_HIBERN8TIME_CAPABILITY, 0x5), 0xA);
	/* RX_Min_ActivateTime */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RX_MIN_ACTIVATETIME_CAPABILITY, 0x4), 0xA);
	/* RX_Min_ActivateTime */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RX_MIN_ACTIVATETIME_CAPABILITY, 0x5), 0xA);

	ufs_kirin_set_synclength(hba);

	/* Thibernate Tx */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(TX_HIBERN8TIME_CAPABILITY, 0x0), 0x5);
	/* Thibernate Tx */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(TX_HIBERN8TIME_CAPABILITY, 0x1), 0x5);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD085, 0x0), 0x1);

	/* CBENBLCPBATTRWR */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBENBLCPBATTRWR, 0x0), 0x0);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD085, 0x0), 0x1);

	/* Unipro VS_mphy_disable */
	ufs_kirin_unipro_vs_mphy_disable(hba);

	/* Unipro VS_mphy_disable */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD0C1, 0x0), 0x0);

	err = ufs_kirin_check_is_h8(hba, err);

	if (likely(!hba->host->is_emulator))
		ufs_kirin_mphy_amplitude_glitch_workaround(hba);

	/* disable override ref_clk_en */
	ufs_kirin_mphy_write(hba, 0x203B, 0x0);

	pr_info("%s --\n", __func__);

	return err;
}

int ufs_kirin_link_startup_pre_change(struct ufs_hba *hba)
{
	int err = 0;
	uint32_t value = 0;
	uint32_t reg;
	struct ufs_kirin_host *host = (struct ufs_kirin_host *)hba->priv;

	pr_info("%s ++\n", __func__);

	/* for hisi MPHY */
	hisi_mphy_updata_temp_sqvref(hba, host);

	if (!(host->caps & USE_HISI_MPHY_TC)) {
		err = ufs_kirin_dme_setup_hci_asic_mphy(hba);
		if (err)
			return err;
	}

	/* disable auto H8 */
	reg = ufshcd_readl(hba, REG_CONTROLLER_AHIT);
	reg = reg & (~UFS_AHIT_AH8ITV_MASK);
	ufshcd_writel(hba, reg, REG_CONTROLLER_AHIT);

	/* for hisi MPHY */
	hisi_mphy_updata_vswing_fsm_ocs5(hba, host);

	/* Unipro PA_Local_TX_LCC_Enable */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_LOCAL_TX_LCC_ENABLE, 0x0), 0x0);

	/* enlarge the VS_AdjustTrailingClocks and VS_DebugSaveConfigTime */
	/* Unipro VS_AdjustTrailingClocks */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)0xd086, 0x0), 0xF0);
	/* Unipro VS_DebugSaveConfigTime */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)0xd0a0, 0x0), 0x3);
	/*
	 * Unipro PA_AdaptAfterLRSTInPA_INIT, use PA_PeerRxHsAdaptInitial value
	 */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)PA_ADAPTAFTERLRSTINPA_INIT, 0x0), 0x3);

	/* close Unipro VS_Mk2ExtnSupport */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD0AB, 0x0), 0x0);
	ufshcd_dme_get(hba, UIC_ARG_MIB_SEL(0xD0AB, 0x0), &value);
	if (value)
		/* Ensure close success */
		pr_warn("Warring!!! close VS_Mk2ExtnSupport failed\n");
	if (!(host->caps & USE_HISI_MPHY_TC)) {
		if (host->tx_equalizer == TX_EQUALIZER_35DB) {
			ufshcd_dme_set(
				hba, UIC_ARG_MIB_SEL((u32)0x0037, 0x0), 0x1);
			ufshcd_dme_set(
				hba, UIC_ARG_MIB_SEL((u32)0x0037, 0x1), 0x1);
		} else if (host->tx_equalizer == TX_EQUALIZER_60DB) {
			ufshcd_dme_set(
				hba, UIC_ARG_MIB_SEL((u32)0x0037, 0x0), 0x2);
			ufshcd_dme_set(
				hba, UIC_ARG_MIB_SEL((u32)0x0037, 0x1), 0x2);
		}
	}
	/* for hisi MPHY */
	hisi_mphy_busdly_config(hba, host);

	pr_info("%s --\n", __func__);

	return err;
}

static void ufs_kirin_hisi_mphy_link_post_config(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = (struct ufs_kirin_host *)hba->priv;
	uint32_t tx_lane_num = 1;
	uint32_t rx_lane_num = 1;

	if (host->caps & USE_HISI_MPHY_TC) {
		/* set the PA_TActivate to 128. need to check in ASIC... */
		/* H8's workaround */
		/* 5 for lane addr */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_TACTIVATE, 0x0), 5);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x80da, 0x0), 0x2d);

		ufshcd_dme_get(hba, UIC_ARG_MIB(PA_CONNECTEDTXDATALANES), &tx_lane_num);
		ufshcd_dme_get(hba, UIC_ARG_MIB(PA_CONNECTEDRXDATALANES), &rx_lane_num);

		/* RX_MC_PRESENT */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00c2, 0x4), 0x0);
		if (tx_lane_num > 1 && rx_lane_num > 1)
			ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00c2, 0x5), 0x0);
	}
}

void set_device_clk(struct ufs_hba *hba)
{
	/* 0: 19.2M, 1: 38.4M */
	const int ufs_refclk_val = 0;

	dev_info(hba->dev, "ref clk %s\n", ufs_refclk_val ? "38.4M" : "19.2M");

	/* close the device clk */
	hisi_pmic_reg_write(PMIC_CLK_UFS_EN_ADDR(0), 0);
	/* 0: 19.2M, 1: 38.4M */
	hisi_pmic_reg_write(PMIC_CLK_UFS_FRE_CTRL1_ADDR(0), 0);
	/* open the device clk */
	hisi_pmic_reg_write(PMIC_CLK_UFS_EN_ADDR(0), 1);
	/* delay 2 ms for open device clk completed */
	mdelay(2);
}

int ufs_kirin_link_startup_post_change(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = (struct ufs_kirin_host *)hba->priv;

	pr_info("%s ++\n", __func__);

	/* Unipro DL_AFC0CreditThreshold */
	ufshcd_dme_set(hba, UIC_ARG_MIB(DL_AFC0CREDITTHRESHOLD), 0x0);
	/* Unipro DL_TC0OutAckThreshold */
	ufshcd_dme_set(hba, UIC_ARG_MIB(DL_TC0OUTACKTHRESHOLD), 0x0);
	/* Unipro DL_TC0TXFCThreshold */
	ufshcd_dme_set(hba, UIC_ARG_MIB(DL_TC0TXFCTHRESHOLD), 0x9);

	/* for hisi MPHY */
	ufs_kirin_hisi_mphy_link_post_config(hba);

	if (host->caps & BROKEN_CLK_GATE_BYPASS) {
		/* not bypass ufs clk gate */
		ufs_sys_ctrl_clr_bits(
			host, MASK_UFS_CLK_GATE_BYPASS, CLOCK_GATE_BYPASS);
		ufs_sys_ctrl_clr_bits(
			host, MASK_UFS_SYSCRTL_BYPASS, UFS_SYSCTRL);
	}

	if (host->hba->caps & UFSHCD_CAP_AUTO_HIBERN8)
		/* disable power-gating in auto hibernate 8 */
		ufshcd_rmwl(hba, (LP_AH8_PGE | LP_PGE), 0, UFS_REG_OCPTHRTL);

	/* select received symbol cnt */
	ufshcd_dme_set(hba, UIC_ARG_MIB(0xd09a), 0x80000000);
	/* reset counter0 and enable */
	ufshcd_dme_set(hba, UIC_ARG_MIB(0xd09c), 0x00000005);

	pr_info("%s --\n", __func__);

	return 0;
}

void ufs_kirin_pwr_change_pre_change(struct ufs_hba *hba,
	struct ufs_pa_layer_attr *dev_req_params)
{
	uint32_t value;
	u32 equalizer;
	struct ufs_kirin_host *host = (struct ufs_kirin_host *)hba->priv;

	pr_info("%s ++\n", __func__);
#ifdef CONFIG_HISI_DEBUG_FS
	pr_info("device manufacturer_id is 0x%x\n", hba->manufacturer_id);
#endif

	/* tx 3.5 gear num : 4 */
	equalizer = (dev_req_params->gear_tx == 4) ? 0 : TX_EQUALIZER_35DB;
	sel_equalizer_by_device(hba, &equalizer);
	if (host->tx_equalizer != equalizer) {
		host->tx_equalizer = equalizer;
		if (likely(!hba->host->is_emulator))
			ufs_kirin_mphy_amplitude_glitch_workaround(hba);
	}

	/*
	 * ARIES platform need to set SaveConfigTime to 0x13, and change sync
	 * length to maximum value
	 */
	/* VS_DebugSaveConfigTime */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)0xD0A0), 0x13);
	/* g1 sync length */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)PA_TXHSG1SYNCLENGTH), 0x4f);
	/* g2 sync length */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)PA_TXHSG2SYNCLENGTH), 0x4f);
	/* g3 sync length */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)PA_TXHSG3SYNCLENGTH), 0x4f);

	ufshcd_dme_get(hba, UIC_ARG_MIB(PA_HIBERN8TIME), &value);
	if (value < 0xA)
		ufshcd_dme_set(hba, UIC_ARG_MIB((u32)PA_HIBERN8TIME), 0xA);
	/* PA_Tactivate */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)PA_TACTIVATE), 0xA);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xd085, 0x0), 0x01);

	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_TXSKIP), 0x0); /* PA_TxSkip */

	/* PA_PWRModeUserData0 = 8191, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_PWRMODEUSERDATA0), 8191);
	/* PA_PWRModeUserData1 = 65535, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_PWRMODEUSERDATA1), 65535);
	/* PA_PWRModeUserData2 = 32767, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_PWRMODEUSERDATA2), 32767);
	/* DME_FC0ProtectionTimeOutVal = 8191, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)DME_FC0_PROTECTION_TIMEOUT_VAL), 8191);
	/* DME_TC0ReplayTimeOutVal = 65535, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)DME_TC0_REPLAY_TIMEOUT_VAL), 65535);
	/* DME_AFC0ReqTimeOutVal = 32767, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)DME_AFC0_REQ_TIMEOUT_VAL), 32767);
	/* PA_PWRModeUserData3 = 8191, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_PWRMODEUSERDATA3), 8191);
	/* PA_PWRModeUserData4 = 65535, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_PWRMODEUSERDATA4), 65535);
	/* PA_PWRModeUserData5 = 32767, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_PWRMODEUSERDATA5), 32767);
	/* DME_FC1ProtectionTimeOutVal = 8191, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)DME_FC1_PROTECTION_TIMEOUT_VAL), 8191);
	/* DME_TC1ReplayTimeOutVal = 65535, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)DME_TC1_REPLAY_TIMEOUT_VAL), 65535);
	/* DME_AFC1ReqTimeOutVal = 32767, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)DME_AFC1_REQ_TIMEOUT_VAL), 32767);

	pr_info("%s --\n", __func__);
}

/*
 * Soc init will reset host controller, all register value will lost
 * including memory address, doorbell and AH8 AGGR
 */
void ufs_kirin_full_reset(struct ufs_hba *hba)
{
#ifdef CONFIG_HUAWEI_UFS_DSM
	dsm_ufs_disable_volt_irq(hba);
#endif
	disable_irq(hba->irq);

	/*
	 * Cancer platform need a full reset when error handler occurs.
	 * If a request sending in ufshcd_queuecommand passed through
	 * ufshcd_state check. And eh may reset host controller, a NOC
	 * error happens. 1000ms sleep is enough for waiting those requests.
	 */
	msleep(1000);

	ufs_soc_init(hba);

	enable_irq(hba->irq);
#ifdef CONFIG_HUAWEI_UFS_DSM
	dsm_ufs_enable_volt_irq(hba);
#endif
}
