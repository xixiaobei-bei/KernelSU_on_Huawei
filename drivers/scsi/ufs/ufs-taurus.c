/*
 * ufs-taurus.c
 *
 * ufs configuration for taurus
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2019. All rights reserved.
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

#define pr_fmt(fmt) "ufshcd :" fmt
#include <linux/mfd/hisi_pmic.h>
#include <soc_sctrl_interface.h>
#include <soc_ufs_sysctrl_interface.h>
#include <pmic_interface.h>
#include <linux/hisi/hisi_idle_sleep.h>
#include "ufshcd.h"
#include "ufs-kirin.h"
#include "dsm_ufs.h"

#define TX_ANA_CTRL_CONFIG_VAL 2
#define SNPS_ASIC_MPHY_CONFIG 3
#define GEAR_4 4
#define EQUALIZER_0DB 0
#define EQUALIZER_35DB 35
#define EQUALIZER_60DB 60

void ufs_kirin_regulator_init(struct ufs_hba *hba)
{
}

void ufs_clk_init(struct ufs_hba *hba)
{
}

static void ufs_set_memory_sd(struct ufs_kirin_host *host)
{
	u32 reg = ufs_sys_ctrl_readl(host, UFS_SYS_MEMORY_CTRL);

	reg &= ~UFS_MEM_CTRL_MASK;
	reg |= UFS_MEM_CTRL_VAL; /* value 0x850 */
	ufs_sys_ctrl_writel(host, reg, UFS_SYS_MEMORY_CTRL);
	reg = ufs_sys_ctrl_readl(host, UFS_SYS_MEMORY_BP_CTRL);
	reg &= ~UFS_BP_MEM_CTRL_MASK;
	reg |= UFS_BP_MEM_CTRL_VAL; /* value 0x850 */
	ufs_sys_ctrl_writel(host, reg, UFS_SYS_MEMORY_BP_CTRL);
}

static void ufs_set_clk_freq(struct ufs_kirin_host *host, u32 *reg)
{
	ufs_sys_ctrl_clr_bits(host, BIT_SYSCTRL_REF_CLOCK_EN, PHY_CLK_CTRL);
	*reg = ufs_sys_ctrl_readl(host, PHY_CLK_CTRL);
	*reg = *reg & (~MASK_SYSCTRL_REF_CLOCK_SEL) &
	       (~MASK_SYSCTRL_CFG_CLOCK_FREQ);
	if (host->caps & USE_HISI_MPHY_TC) {
		/* set ref clk freq 20Mhz */
		*reg = *reg | 0x14;
	} else {
		/* syscfg clk is fixed 38.4Mhz */
		*reg = *reg | 0x26;
		/* set ref clk freq 38.4Mhz */
		*reg = *reg | SYSCTRL_REF_CLOCK_SEL;
	}
	ufs_sys_ctrl_writel(host, *reg, PHY_CLK_CTRL); /* set cfg clk freq */
}

static void fpga_device_reset_disreset(struct ufs_hba *hba, u32 *reg)
{
	if (is_v200_mphy(hba)) {
		ufs_i2c_writel(hba, 0x20000, SC_RSTEN_V200);
		mdelay(2); /* wait 2 ms */
		ufs_i2c_writel(hba, 0x20000, SC_RSTDIS_V200);
	} else {
		/* enable Device Reset */
		ufs_i2c_writel(hba, (unsigned int)UFS_BIT_DEVICE_RSTDIS,
			       SC_RSTDIS);
		ufs_i2c_readl(hba, reg, SC_UFS_REFCLK_RST_PAD);
		*reg = *reg & (~(UFS_BIT_OEN_RST | UFS_BIT_OEN_REFCLK));
		/*
		 * output enable, For EMMC to high dependence, open
		 * DA_UFS_OEN_RST
		 * and DA_UFS_OEN_REFCLK
		 */
		ufs_i2c_writel(hba, *reg, SC_UFS_REFCLK_RST_PAD);

		mdelay(2); /* wait 2 ms */
		/* disable Device Reset */
		ufs_i2c_writel(hba, (unsigned int)UFS_BIT_DEVICE_RSTEN,
			       SC_RSTEN);
	}
}

static void ufs_open_clk_for_mphy(
	struct ufs_hba *hba, struct ufs_kirin_host *host, u32 *reg)
{
	ufs_sys_ctrl_set_bits(host, BIT_SYSCTRL_REF_CLOCK_EN, PHY_CLK_CTRL);
	if (host->caps & USE_HISI_MPHY_TC) {
		fpga_device_reset_disreset(hba, reg);
	} else {
		/* reset device */
		ufs_sys_ctrl_writel(host, MASK_UFS_DEVICE_RESET | 0,
				    UFS_DEVICE_RESET_CTRL);
		ufshcd_vops_vcc_power_on_off(hba);

		mdelay(1);

		ufs_sys_ctrl_writel(
			host, MASK_UFS_DEVICE_RESET | BIT_UFS_DEVICE_RESET,
			UFS_DEVICE_RESET_CTRL); /* disable Device Reset */
	}
	mdelay(10); /* wait 10 ms */
}

static void ufs_cfg_ip_reset(struct ufs_kirin_host *host, u32 *reg)
{
	writel((1 << (SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START +
		      BIT_ENABLE_MASK)) |
		       (1 << SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START),
	       SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));

	*reg = readl(SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));
	if (*reg & (1 << SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START))
		mdelay(1);
}

static void ufs_clk_config_init(struct ufs_kirin_host *host)
{
	/* eS LOW TEMP 224M */
	writel(BIT(SOC_SCTRL_SCPERDIS4_gt_clk_ufs_subsys_START),
	       SOC_SCTRL_SCPERDIS4_ADDR(host->sysctrl));
	writel(0x003F0006, SOC_SCTRL_SCCLKDIV9_ADDR(host->sysctrl));
	writel(BIT(SOC_SCTRL_SCPEREN4_gt_clk_ufs_subsys_START),
	       SOC_SCTRL_SCPEREN4_ADDR(host->sysctrl));
}

void ufs_soc_init(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = NULL;
	u32 reg;

	host = (struct ufs_kirin_host *)hba->priv;

	dev_info(hba->dev, "%s ++\n", __func__);

	ufs_clk_config_init(host);

	if (!(host->caps & USE_HISI_MPHY_TC)) {
		/* enable the MPHY's sram_bypass */
		ufs_sys_ctrl_clr_bits(host, PHY_SRAM_BYPASS_BIT,
				      UFS_SYS_PHY_SRAM_MEM_CTRL_S);
		/* disable sram_ext_ld_done */
		ufs_sys_ctrl_clr_bits(host, SRAM_EXT_LD_DONE_BIT,
				      UFS_SYS_PHY_SRAM_MEM_CTRL_S);
	}

	writel(1 << SOC_UFS_Sysctrl_UFS_UMECTRL_ufs_ies_en_mask_START,
	       SOC_UFS_Sysctrl_UFS_UMECTRL_ADDR(host->ufs_sys_ctrl));

	writel((1 << (SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START +
		      BIT_ENABLE_MASK)) |
		       0,
	       SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));

	/* HC_PSW powerup */
	ufs_sys_ctrl_set_bits(host, BIT_UFS_PSW_MTCMOS_EN, PSW_POWER_CTRL);
	udelay(10); /* wait 10 us */
	/* notify PWR ready */
	ufs_sys_ctrl_set_bits(host, BIT_SYSCTRL_PWR_READY, HC_LP_CTRL);

	/* set memory SD signal 0, exit lowpower */
	ufs_set_memory_sd(host);

	/* STEP 4 CLOSE MPHY REF CLOCK */
	ufs_set_clk_freq(host, &reg);

	if (!(host->caps & USE_HISI_MPHY_TC))
		/*
		 * Enable: pcs_pwr_stable_sc,pma_pwr_en_sc. Disable:
		 * pma_pwr_stable_sc,ref_clk_en_app, ref_clk_en_unipro
		 */
		ufs_sys_ctrl_writel(host, 0x4300, UFS_SYS_UFS_POWER_GATING);

	/* bypass ufs clk gate */
	ufs_sys_ctrl_set_bits(host, MASK_UFS_CLK_GATE_BYPASS,
			      CLOCK_GATE_BYPASS);
	ufs_sys_ctrl_set_bits(host, MASK_UFS_SYSCRTL_BYPASS, UFS_SYSCTRL);

	/* open psw clk */
	ufs_sys_ctrl_set_bits(host, BIT_SYSCTRL_PSW_CLK_EN, PSW_CLK_CTRL);
	/* disable ufshc iso */
	ufs_sys_ctrl_clr_bits(host, BIT_UFS_PSW_ISO_CTRL, PSW_POWER_CTRL);
	/* phy iso is not effective */
	/* disable phy iso */
	ufs_sys_ctrl_clr_bits(host, BIT_UFS_PHY_ISO_CTRL, PHY_ISO_EN);
	/* notice iso disable */
	ufs_sys_ctrl_clr_bits(host, BIT_SYSCTRL_LP_ISOL_EN, HC_LP_CTRL);

	writel((1 << (SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_arst_ufs_START +
		      BIT_ENABLE_MASK)) |
		       (1 << SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_arst_ufs_START),
	       SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));

	/* disable lp_reset_n */
	ufs_sys_ctrl_set_bits(host, BIT_SYSCTRL_LP_RESET_N, RESET_CTRL_EN);
	mdelay(1);

	/* open clock of M-PHY */
	ufs_open_clk_for_mphy(hba, host, &reg);

	/* disable EC bypass, make effective */
	ufs_sys_ctrl_clr_bits(host, WDP_BYPASS_EC_BIT, PHY_ISO_EN);
	ufs_sys_ctrl_set_bits(host, OVERALL_BYPASS_EC_BIT, UFS_SYS_MK2_CTRL);

	ufs_cfg_ip_reset(host, &reg);

	/* set SOC_SCTRL_SCBAKDATA11_ADDR ufs bit to 1 when init */
	if (!ufshcd_is_auto_hibern8_allowed(hba))
		hisi_idle_sleep_vote(ID_UFS, 1);

	dev_info(hba->dev, "%s --\n", __func__);
}

int ufs_kirin_suspend_before_set_link_state(
	struct ufs_hba *hba, enum ufs_pm_op pm_op)
{
#ifdef FEATURE_KIRIN_UFS_PSW
	struct ufs_kirin_host *host = NULL;

	host = hba->priv;

	if (ufshcd_is_runtime_pm(pm_op))
		return 0;

	/* step1:store BUSTHRTL register */
	host->busthrtl_backup = ufshcd_readl(hba, UFS_REG_OCPTHRTL);
	/* enable PowerGating */
	ufshcd_rmwl(hba, LP_PGE, LP_PGE, UFS_REG_OCPTHRTL);
#endif
	return 0;
}

int ufs_kirin_resume_after_set_link_state(
	struct ufs_hba *hba, enum ufs_pm_op pm_op)
{
#ifdef FEATURE_KIRIN_UFS_PSW
	struct ufs_kirin_host *host = NULL;

	host = hba->priv;

	if (ufshcd_is_runtime_pm(pm_op))
		return 0;

	ufshcd_writel(hba, host->busthrtl_backup, UFS_REG_OCPTHRTL);
#endif
	return 0;
}

int ufs_kirin_suspend(struct ufs_hba *hba, enum ufs_pm_op pm_op)
{
	struct ufs_kirin_host *host = NULL;

	host = hba->priv;

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
	udelay(10); /* wait 10 us */
	hisi_pmic_reg_write(PMIC_CLK_UFS_EN_ADDR(0), 0);

	host->in_suspend = true;

	return 0;
}

int ufs_kirin_resume(struct ufs_hba *hba, enum ufs_pm_op pm_op)
{
	struct ufs_kirin_host *host = NULL;

	host = hba->priv;

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
	struct ufs_kirin_host *host = NULL;

	host = hba->priv;
	if (likely(!(host->caps & USE_HISI_MPHY_TC))) {
		ufs_sys_ctrl_writel(host, MASK_UFS_DEVICE_RESET | 0,
				    UFS_DEVICE_RESET_CTRL);
		ufshcd_vops_vcc_power_on_off(hba);
		mdelay(1);
	} else if (!is_v200_mphy(hba)) {
		ufs_i2c_writel(hba, (unsigned int)UFS_BIT_DEVICE_RSTDIS,
			       SC_RSTDIS);
		mdelay(1);
	} else {
		ufs_i2c_writel(hba, 0x20000, SC_RSTEN_V200);
		mdelay(2); /* wait 2 ms */
	}

	if (likely(!(host->caps & USE_HISI_MPHY_TC)))
		ufs_sys_ctrl_writel(host, MASK_UFS_DEVICE_RESET |
						  BIT_UFS_DEVICE_RESET,
				    UFS_DEVICE_RESET_CTRL);
	else if (!is_v200_mphy(hba))
		ufs_i2c_writel(hba, (unsigned int)UFS_BIT_DEVICE_RSTEN,
			       SC_RSTEN);
	else
		ufs_i2c_writel(hba, 0x20000, SC_RSTDIS_V200);

	mdelay(10); /* wait 10 ms */
}

static u16 table_0db[][TX_ANA_CTRL_CONFIG_VAL] = {
	/* tx_ana_ctrl_leg_pull_en, tx_ana_ctrl_post */
	{ 252, 0 },  { 252, 0 },  { 252, 0 },  { 255, 0 },
	{ 255, 0 },  { 1020, 0 }, { 1020, 0 }, { 1023, 0 },
	{ 1023, 0 }, { 4092, 0 }, { 4092, 0 }
};

static u16 table_35db[][TX_ANA_CTRL_CONFIG_VAL] = { { 252, 3 },  { 252, 3 },
						    { 252, 3 },  { 255, 3 },
						    { 255, 6 },  { 1020, 6 },
						    { 1020, 6 }, { 1023, 6 },
						    { 1023, 6 }, { 4092, 6 },
						    { 4092, 7 } };

static u16 table_60db[][TX_ANA_CTRL_CONFIG_VAL] = { { 252, 6 },   { 252, 7 },
						    { 252, 7 },   { 255, 7 },
						    { 255, 14 },  { 1020, 14 },
						    { 1020, 14 }, { 1023, 14 },
						    { 1023, 15 }, { 4092, 15 },
						    { 4092, 15 } };

/* Workaround: PWM-amplitude reduce & PMC and H8's glitch */
static void mphy_amplitude_glitch_workaround(struct ufs_hba *hba)
{
	u16 value3;
	u16 value4;
	u32 reg;
	struct ufs_kirin_host *host = hba->priv;
	u16(*table)[TX_ANA_CTRL_CONFIG_VAL] = NULL;

	table = &table_35db[0];
	if (host->tx_equalizer == 0)
		table = &table_0db[0];
	else if (host->tx_equalizer == EQUALIZER_60DB) /* 6.0 db */
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

	value3 |= (1 << 15); /* LANEN_DIG_ANA_TX_EQ_OVRD_OUT_0[15] equals 1 */
	ufs_kirin_mphy_write(hba, 0x10A3, value3);
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

static void ufs_snps_asic_mphy_slow_process(struct ufs_hba *hba)
{
	u32 value1, value2;

	value1 = ufs_kirin_mphy_read(hba, 0x401e);
	value2 = ufs_kirin_mphy_read(hba, 0x411e);
	ufs_kirin_mphy_write(hba, 0x401e, value1 | 0x1);
	ufs_kirin_mphy_write(hba, 0x411e, value2 | 0x1);
	value1 = ufs_kirin_mphy_read(hba, 0x401f);
	value2 = ufs_kirin_mphy_read(hba, 0x411f);
	ufs_kirin_mphy_write(hba, 0x401f, value1 | 0x1);
	ufs_kirin_mphy_write(hba, 0x411f, value2 | 0x1);
}

u32 snps_asic_mphy_attr[][SNPS_ASIC_MPHY_CONFIG] = {
	{ 0x8113, 0x0, 0x1 }, /* CBENBLCPBATTRWR */
	{ 0xD085, 0x0, 0x1 },
	/* RX_HS_G1_PREPARE_LENGTH_CAPABILITY */
	{ 0x008c, 0x4, 0xF }, /* Gear1 Synclength */
	{ 0x008c, 0x5, 0xF }, /* Gear1 Synclength */
	{ 0xD085, 0x0, 0x1 }, /* Unipro VS_MphyCfgUpdt */
	{ 0x15d4, 0x0, 0x3 }, /* PA_TxHsAdaptType: no adapt */
	{ 0x15d5, 0x0, 0x3 }, /* PA_TxHsAdaptTypeInPa_Init: no adapt */
	{ 0x0005, 0x0, 0x2 }, /* only LA */
	{ 0x0005, 0x1, 0x2 }, /* only LA */
	{ 0xD085, 0x0, 0x1 }, /* Unipro VS_MphyCfgUpdt */
	{ 0x0092, 0x4, 0xA }, /* RX_Hibern8Time_Capability */
	{ 0x0092, 0x5, 0xA }, /* RX_Hibern8Time_Capability */
	{ 0x008f, 0x4, 0xA }, /* RX_Min_ActivateTime */
	{ 0x008f, 0x5, 0xA }, /* RX_Min_ActivateTime */
	{ 0x0095, 0x4, 0x4F }, /* Gear3 Synclength */
	{ 0x0095, 0x5, 0x4F }, /* Gear3 Synclength */
	{ 0x0094, 0x4, 0x4F }, /* Gear2 Synclength */
	{ 0x0094, 0x5, 0x4F }, /* Gear2 Synclength */
	{ 0x008B, 0x4, 0x4F }, /* Gear1 Synclength */
	{ 0x008B, 0x5, 0x4F }, /* Gear1 Synclength */
	{ 0x000F, 0x0, 0x5 }, /* Thibernate Tx */
	{ 0x000F, 0x1, 0x5 }, /* Thibernate Tx */
	{ 0xD085, 0x0, 0x1 },
	{ 0x8113, 0x0, 0x0 }, /* CBENBLCPBATTRWR */
	{ 0xD085, 0x0, 0x1 }
};

static void ufs_mphy_rxrhold_ctrl_opt(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = hba->priv;

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

static void ufs_unipro_vs_mphy_disable(struct ufs_hba *hba)
{
	u32 value = 0;

	/* Unipro VS_mphy_disable */
	ufshcd_dme_get(hba, UIC_ARG_MIB_SEL(0xD0C1, 0x0), &value);
	if (value != 0x1)
		pr_warn("Warring!!! Unipro VS_mphy_disable is 0x%x\n", value);

	/* Unipro VS_mphy_disable */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD0C1, 0x0), 0x0);
}

/* snps asic mphy specific configuration */
int ufs_kirin_dme_setup_snps_asic_mphy(struct ufs_hba *hba)
{
	int err;
	int i;

	struct ufs_kirin_host *host = NULL;

	host = hba->priv;

	pr_info("%s ++\n", __func__);

	/* Unipro VS_mphy_disable */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD0C1, 0x0), 0x1);

	ufs_mphy_rxrhold_ctrl_opt(hba);

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
	 * RAWCMN_DIG_AON_CMN_SUP_OVRD_IN[5:4] = 2'b11, open phy clk during
	 * H8
	 */
	ufs_kirin_mphy_write(hba, 0x203B, 0x30);

	/*
	 * Workaround: clear P-N abnormal common voltage begin
	 * LANEN_ANA_TX_OVRD_MEAS[5] is 0,LANEN_DIG_ANA_TX_OVRD_MEAS[4] is 1
	 */
	ufs_kirin_mphy_write(hba, 0x10e0, 0x10);
	/* LANEN_ANA_RX_OVRD_MEAS[5] is 0,LANEN_DIG_ANA_TX_OVRD_MEAS[4] is 1 */
	ufs_kirin_mphy_write(hba, 0x11e0, 0x10);

	/* close AFE calibration */
	ufs_kirin_mphy_write(hba, 0x401c, 0x0004);
	ufs_kirin_mphy_write(hba, 0x411c, 0x0004);

	/* slow process */
	ufs_snps_asic_mphy_slow_process(hba);

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

	for (i = 0; i < ARRAY_SIZE(snps_asic_mphy_attr); i++)
		ufshcd_dme_set(
			hba, UIC_ARG_MIB_SEL(snps_asic_mphy_attr[i][0],
					     snps_asic_mphy_attr[i][1]),
			snps_asic_mphy_attr[i][SNPS_ASIC_MPHY_CONFIG - 1]);

	ufs_unipro_vs_mphy_disable(hba);

	if (likely(!hba->host->is_emulator)) {
		err = ufs_kirin_check_hibern8(hba);
		if (err)
			pr_err("ufs_kirin_check_hibern8 error\n");

		mphy_amplitude_glitch_workaround(hba);
	}

	/* disable override ref_clk_en */
	ufs_kirin_mphy_write(hba, 0x203B, 0x0);

	pr_info("%s --\n", __func__);

	return err;
}

int ufs_kirin_link_startup_pre_change(struct ufs_hba *hba)
{
	int err = 0;
	u32 value = 0;
	u32 reg;
	struct ufs_kirin_host *host = NULL;

	host = hba->priv;

	pr_info("%s ++\n", __func__);

	/* for hisi MPHY */
	if ((host->caps & USE_HISI_MPHY_TC)) {
		if (is_v200_mphy(hba))
			hisi_mphy_v200_updata(hba, host);
		else
			hisi_mphy_updata_temp_sqvref(hba, host);
	}

	if (!(host->caps & USE_HISI_MPHY_TC)) {
		err = ufs_kirin_dme_setup_snps_asic_mphy(hba);
		if (err)
			return err;
	}

	/* disable auto H8 */
	reg = ufshcd_readl(hba, REG_CONTROLLER_AHIT);
	reg = reg & (~UFS_AHIT_AH8ITV_MASK);
	ufshcd_writel(hba, reg, REG_CONTROLLER_AHIT);

	/* for hisi MPHY */
	if ((host->caps & USE_HISI_MPHY_TC)) {
		if (!is_v200_mphy(hba))
			hisi_mphy_updata_vswing_fsm_ocs5(hba, host);
		else
			hisi_mphy_v200_updata_vswing_fsm(hba, host);
	}

	/* Unipro PA_Local_TX_LCC_Enable */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_LOCAL_TX_LCC_ENABLE, 0x0), 0x0);

	/*
	 * enlarge the VS_AdjustTrailingClocks and VS_DebugSaveConfigTime
	 * Unipro VS_AdjustTrailingClocks
	 */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)0xd086, 0x0), 0xF0);
	/* Unipro VS_DebugSaveConfigTime */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)0xd0a0, 0x0), 0x3);
	/*
	 * Unipro PA_AdaptAfterLRSTInPA_INIT, use PA_PeerRxHsAdaptInitial
	 * value
	 */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)PA_ADAPTAFTERLRSTINPA_INIT, 0x0), 0x3);
	/* close Unipro VS_Mk2ExtnSupport */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD0AB, 0x0), 0x0);
	ufshcd_dme_get(hba, UIC_ARG_MIB_SEL(0xD0AB, 0x0), &value);
	if (value)
		/* Ensure close success */
		pr_warn("Warring!!! close VS_Mk2ExtnSupport failed\n");

	/* for hisi MPHY */
	hisi_mphy_busdly_config(hba, host);
	pr_info("%s --\n", __func__);

	return err;
}

static void hisi_mphy_link_post_config(
	struct ufs_hba *hba, struct ufs_kirin_host *host)
{
	u32 tx_lane_num = 1;
	u32 rx_lane_num = 1;

	if (host->caps & USE_HISI_MPHY_TC) {
		/*
		 * set the PA_TActivate to 128. need to check in ASIC
		 * H8's workaround
		 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_TACTIVATE, 0x0), 0x5);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x80da, 0x0), 0x2d);

		ufshcd_dme_get(hba, UIC_ARG_MIB(PA_CONNECTEDTXDATALANES), &tx_lane_num);
		ufshcd_dme_get(hba, UIC_ARG_MIB(PA_CONNECTEDRXDATALANES), &rx_lane_num);

		/* RX_MC_PRESENT */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00c2, 0x4), 0x0);
		if (tx_lane_num > 1 && rx_lane_num > 1)
			ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00c2, 0x5),
				       0x0); /* RX_MC_PRESENT */
	}
}

void set_device_clk(struct ufs_hba *hba)
{
	/* 0: 19.2M, 1: 38.4M */
	int ufs_refclk_val = 0;

	dev_info(hba->dev, "ref clk %s\n", ufs_refclk_val ? "38.4M" : "19.2M");

	/* close the device clk */
	hisi_pmic_reg_write(PMIC_CLK_UFS_EN_ADDR(0), 0);
	/* choose the device clk 38.4Mhz when device spec is 3.0 */
	hisi_pmic_reg_write(PMIC_CLK_UFS_FRE_CTRL1_ADDR(0), ufs_refclk_val);
	/* open the device clk */
	hisi_pmic_reg_write(PMIC_CLK_UFS_EN_ADDR(0), 1);

	mdelay(2); /* wait 2 ms */
}

int ufs_kirin_link_startup_post_change(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = NULL;

	host = hba->priv;

	pr_info("%s ++\n", __func__);

	if (!(host->caps & USE_HISI_MPHY_TC)) {
		/* Unipro DL_AFC0CreditThreshold */
		ufshcd_dme_set(hba, UIC_ARG_MIB(DL_AFC0CREDITTHRESHOLD), 0x0);
		/* Unipro DL_TC0OutAckThreshold */
		ufshcd_dme_set(hba, UIC_ARG_MIB(DL_TC0OUTACKTHRESHOLD), 0x0);
		/* Unipro DL_TC0TXFCThreshold */
		ufshcd_dme_set(hba, UIC_ARG_MIB(DL_TC0TXFCTHRESHOLD), 0x9);
	} else {
		/* for hisi MPHY */
		if (!is_v200_mphy(hba)) {
			/* Unipro DL_AFC0CreditThreshold */
			ufshcd_dme_set(hba, UIC_ARG_MIB(DL_AFC0CREDITTHRESHOLD),
				       0x0);
			/* Unipro DL_TC0OutAckThreshold */
			ufshcd_dme_set(hba, UIC_ARG_MIB(DL_TC0OUTACKTHRESHOLD),
				       0x0);
			/* Unipro DL_TC0TXFCThreshold */
			ufshcd_dme_set(hba, UIC_ARG_MIB(DL_TC0TXFCTHRESHOLD),
				       0x9);
			hisi_mphy_link_post_config(hba, host);
		} else {
			hisi_mphy_v200_link_post_config(hba, host);
		}
	}

	if (host->caps & BROKEN_CLK_GATE_BYPASS) {
		/* not bypass ufs clk gate */
		ufs_sys_ctrl_clr_bits(host, MASK_UFS_CLK_GATE_BYPASS,
				      CLOCK_GATE_BYPASS);
		ufs_sys_ctrl_clr_bits(host, MASK_UFS_SYSCRTL_BYPASS,
				      UFS_SYSCTRL);
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

void ufs_kirin_pwr_change_pre_change(
	struct ufs_hba *hba, struct ufs_pa_layer_attr *dev_req_params)
{
	u32 value = 0;
	u32 equalizer;
	struct ufs_kirin_host *host = NULL;

	host = hba->priv;

	pr_info("%s ++\n", __func__);
#ifdef CONFIG_HISI_DEBUG_FS
	pr_info("device manufacturer_id is 0x%x\n", hba->manufacturer_id);
#endif

	equalizer = (dev_req_params->gear_tx == GEAR_4) ? EQUALIZER_0DB :
							  EQUALIZER_35DB;
	sel_equalizer_by_device(hba, &equalizer);
	if (host->tx_equalizer != equalizer) {
		host->tx_equalizer = equalizer;
		if (likely(!hba->host->is_emulator))
			mphy_amplitude_glitch_workaround(hba);
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
		ufshcd_dme_set(hba, UIC_ARG_MIB((u32)PA_HIBERN8TIME),
			       0xA); /* PA_Hibern8Time */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)PA_TACTIVATE), 0xA); /* PA_Tactivate */
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

	if ((host->caps & USE_HISI_MPHY_TC)) {
		if (is_v200_mphy(hba))
			hisi_mphy_v200_pwr_change_pre_config(hba, host);
	}
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
	 * error happens.
	 */
	msleep(1000); /* 1000ms sleep is enough for waiting those requests. */

	ufs_soc_init(hba);

	enable_irq(hba->irq);
#ifdef CONFIG_HUAWEI_UFS_DSM
	dsm_ufs_enable_volt_irq(hba);
#endif
}
