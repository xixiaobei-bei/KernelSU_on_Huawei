/*
 * ufs-aries.c
 *
 * ufs config for aries
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
#include <linux/hisi/lpcpu_idle_sleep.h>
#include <linux/types.h>
#include <soc_sctrl_interface.h>
#include <soc_ufs_sysctrl_interface.h>

void set_device_clk(struct ufs_hba *hba)
{
}

void hufs_regulator_init(struct ufs_hba *hba)
{
	struct device *dev = hba->dev;

	hba->vreg_info.vcc =
		devm_kzalloc(dev, sizeof(struct ufs_vreg), GFP_KERNEL);
	if (!hba->vreg_info.vcc) {
		dev_err(dev, "vcc alloc error\n");
		goto error;
	}

	hba->vreg_info.vcc->reg = devm_regulator_get(dev, "vcc");
	if (IS_ERR(hba->vreg_info.vcc->reg)) {
		dev_err(dev, "get regulator vcc failed\n");
		goto error;
	}

	/* 2950000 for vcc voltage */
	if (regulator_set_voltage(hba->vreg_info.vcc->reg, 2950000, 2950000)) {
		dev_err(dev, "set vcc voltage failed\n");
		goto error;
	}

	if (regulator_enable(hba->vreg_info.vcc->reg))
		dev_err(dev, "regulator vcc enable failed\n");

	/* hufs not adjust vcc voltage or current dynamically currently */
	hba->vreg_info.vcc->unused = true;

error:
	return;
}

static void ufs_kirin_ctrl_efuse_rhold(struct hufs_host *host)
{
	if (ufs_sctrl_readl(host, SCDEEPSLEEPED_OFFSET) & EFUSE_RHOLD_BIT)
		ufs_sctrl_writel(host,
			(MASK_UFS_MPHY_RHOLD | BIT_UFS_MPHY_RHOLD),
			UFS_DEVICE_RESET_CTRL);
}

void ufs_clk_init(struct ufs_hba *hba)
{
}

static bool ufs_kirin_need_memrepair(struct ufs_hba *hba)
{
	struct hufs_host *host = (struct hufs_host *)hba->priv;

	/* SCINNERSTAT register 0x3A0 */
	if ((BIT(0) & ufs_sctrl_readl(host, 0x3A0)) &&
		/* BIT 19 for SCEFUSEDATA3 efuse_data */
		(BIT(19) & ufs_sctrl_readl(host, 0x010))) {
		pr_err("%s need memrepair\n", __func__);
		return true;
	} else {
		pr_err("%s no need memrepair\n", __func__);
		return false;
	}
}

static void ufs_kirin_device_reset_and_disreset(struct ufs_hba *hba)
{
	struct hufs_host *host = (struct hufs_host *) hba->priv;
	u32 reg = 0;
	int ret;

	if (host->caps & USE_HUFS_MPHY_TC) {
		/* enable Device Reset */
		/* BIT 6 for SC_RSTDIS */
		ufs_i2c_writel(hba, (unsigned int) BIT(6), SC_RSTDIS);
		ufs_i2c_readl(hba, &reg, SC_UFS_REFCLK_RST_PAD);
		/* BIT 2 | 10 for spec reg */
		reg = reg & (~(BIT(2) | BIT(10)));
		/*
		 * output enable, For EMMC to high dependence, open
		 * DA_UFS_OEN_RST and DA_UFS_OEN_REFCLK
		 */
		ufs_i2c_writel(hba, reg, SC_UFS_REFCLK_RST_PAD);
		/*
		 * FPGA is not usable, only for open the clk, to match
		 * clk_unprepare_enble later in suspend&fullreset func
		 */
		ret = clk_prepare_enable(host->clk_ufsio_ref);
		if (ret) {
			pr_err("%s ,clk_prepare_enable failed\n", __func__);
			return;
		}
		/* wait 2 ms for */
		mdelay(2);
		/* BIT 6 for disable Device Reset */
		ufs_i2c_writel(hba, (unsigned int) BIT(6), SC_RSTEN);
	} else {
		/* reset device */
		ufs_sys_ctrl_writel(host, MASK_UFS_DEVICE_RESET | 0,
				    UFS_DEVICE_RESET_CTRL);
		ufshcd_vops_vcc_power_on_off(hba);
		ret = clk_prepare_enable(host->clk_ufsio_ref);
		if (ret) {
			pr_err("%s ,clk_prepare_enable failed\n", __func__);
			return;
		}

		mdelay(1);

		/* disable Device Reset */
		ufs_sys_ctrl_writel(host,
			MASK_UFS_DEVICE_RESET | BIT_UFS_DEVICE_RESET,
			UFS_DEVICE_RESET_CTRL);
	}
}

static void ufs_kirin_subsys_clk_config(struct hufs_host *host)
{
	writel(BIT(SOC_SCTRL_SCPERDIS4_gt_clk_ufs_subsys_START),
	       SOC_SCTRL_SCPERDIS4_ADDR(host->sysctrl));
	writel(0x003F0009, SOC_SCTRL_SCCLKDIV9_ADDR(host->sysctrl));
	writel(BIT(SOC_SCTRL_SCPEREN4_gt_clk_ufs_subsys_START),
	       SOC_SCTRL_SCPEREN4_ADDR(host->sysctrl));
}

static void ufs_kirin_bypass_ufs_clk_gate(struct hufs_host *host)
{
	ufs_sys_ctrl_set_bits(host, MASK_UFS_CLK_GATE_BYPASS,
		CLOCK_GATE_BYPASS);
	ufs_sys_ctrl_set_bits(host, MASK_UFS_SYSCRTL_BYPASS, UFS_SYSCTRL);
}

static void ufs_kirin_psw_config(struct hufs_host *host)
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

void ufs_soc_init(struct ufs_hba *hba)
{
	struct hufs_host *host = (struct hufs_host *) hba->priv;
	u32 reg;

	pr_info("%s ++\n", __func__);

	/* CS LOW TEMP 207M */
	ufs_kirin_subsys_clk_config(host);

	writel(1 << SOC_UFS_Sysctrl_UFS_UMECTRL_ufs_ies_en_mask_START,
		SOC_UFS_Sysctrl_UFS_UMECTRL_ADDR(host->ufs_sys_ctrl));

	/* 16 for mask bit set 1 */
	writel((1 << (SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START + 16)) | 0,
		SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));

	/* efuse indicates enable rhold or not */
	ufs_kirin_ctrl_efuse_rhold(host);

	/* HC_PSW powerup */
	ufs_sys_ctrl_set_bits(host, BIT_UFS_PSW_MTCMOS_EN, PSW_POWER_CTRL);
	/* delay 10 us for HC_PSW powerup completed */
	udelay(10);
	/* notify PWR ready */
	ufs_sys_ctrl_set_bits(host, BIT_SYSCTRL_PWR_READY, HC_LP_CTRL);
	/* STEP 4 CLOSE MPHY REF CLOCK */
	ufs_sys_ctrl_clr_bits(host, BIT_SYSCTRL_REF_CLOCK_EN, PHY_CLK_CTRL);
	/* Bit[4:2], div =4 and 16 for mask */
	reg = ((0x3 << 2) | (0x7 << (2 + 16)));
	writel(reg, SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));
	reg = ufs_sys_ctrl_readl(host, PHY_CLK_CTRL);
	reg = reg & (~MASK_SYSCTRL_REF_CLOCK_SEL) &
		(~MASK_SYSCTRL_CFG_CLOCK_FREQ);
	if (host->caps & USE_HUFS_MPHY_TC)
		reg = reg | 0x14;
	else
		/* BUS 207M / 4 = 0x34 syscfg clk */
		reg = reg | 0x34;
	ufs_sys_ctrl_writel(host, reg, PHY_CLK_CTRL); /* set cfg clk freq */

	/*
	 * enable ref_clk_en override(bit5) & override value = 1(bit4),
	 * with mask
	 */
	ufs_sys_ctrl_writel(host, 0x00300030, UFS_DEVICE_RESET_CTRL);

	/* bypass ufs clk gate */
	ufs_kirin_bypass_ufs_clk_gate(host);

	ufs_kirin_psw_config(host);
	/* 16 for mask bit set 1 */
	writel((1 << (SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_arst_ufs_START + 16)) |
		(1 << SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_arst_ufs_START),
		SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));

	/* disable lp_reset_n */
	ufs_sys_ctrl_set_bits(host, BIT_SYSCTRL_LP_RESET_N, RESET_CTRL_EN);
	mdelay(1);

	/* open clock of M-PHY */
	ufs_sys_ctrl_set_bits(host, BIT_SYSCTRL_REF_CLOCK_EN, PHY_CLK_CTRL);

	ufs_kirin_device_reset_and_disreset(hba);
	/* delay 40 ms to let the pulse propagate */
	mdelay(40);

	/* 16 for mask bit set 1 */
	writel((1 << (SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START + 16)) |
		(1 << SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START),
		SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));

	reg = readl(SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));
	if (reg & (1 << SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START))
		mdelay(1);

	host->need_memrepair = ufs_kirin_need_memrepair(hba);

	/* set SOC_SCTRL_SCBAKDATA11_ADDR ufs bit to 1 when init */
	lpcpu_idle_sleep_vote(ID_UFS, 1);

	pr_info("%s --\n", __func__);
}

int hufs_suspend_before_set_link_state(struct ufs_hba *hba,
	enum ufs_pm_op pm_op)
{
#ifdef FEATURE_KIRIN_UFS_PSW
	struct hufs_host *host = (struct hufs_host *)hba->priv;

	if (ufshcd_is_runtime_pm(pm_op))
		return 0;

	/* step1:store BUSTHRTL register */
	host->busthrtl_backup = ufshcd_readl(hba, UFS_REG_OCPTHRTL);
	/* enable PowerGating */
	ufshcd_rmwl(hba, LP_PGE, LP_PGE, UFS_REG_OCPTHRTL);
#endif
	return 0;
}

int hufs_resume_after_set_link_state(struct ufs_hba *hba,
					  enum ufs_pm_op pm_op)
{
#ifdef FEATURE_KIRIN_UFS_PSW
	struct hufs_host *host = (struct hufs_host *)hba->priv;

	if (ufshcd_is_runtime_pm(pm_op))
		return 0;

	ufshcd_writel(hba, host->busthrtl_backup, UFS_REG_OCPTHRTL);
#endif
	return 0;
}

static void ufs_kirin_memrepair_suspend(struct ufs_hba *hba)
{
	struct hufs_host *host = (struct hufs_host *)hba->priv;

	pr_err("%s ufs need memory repair\n", __func__);
	/* clear ufs_clkrst_bypass and ufs_tick_div */
	ufs_sys_ctrl_writel(host, 0x00E00000, UFS_CRG_UFS_CFG);
	udelay(1);
	ufs_sys_ctrl_set_bits(host, BIT_UFS_PSW_ISO_CTRL, PSW_POWER_CTRL);
}

static void ufs_kirin_memrepair_resume(struct ufs_hba *hba)
{
	struct hufs_host *host = (struct hufs_host *)hba->priv;
	int i;

	pr_err("%s ufs need memory repair\n", __func__);
	ufs_sys_ctrl_clr_bits(host, BIT_UFS_PSW_ISO_CTRL, PSW_POWER_CTRL);
	/* 400 for max repair times */
	for (i = 0; i < 400; i++) {
		/* BIT 19 for mask set */
		if (BIT(19) & ufs_pctrl_readl(host, 0x18C))
			break;
		udelay(1);
	}
	/* 400 for max repair times */
	if (i == 400)
		pr_err("%s ufs memory repair timeout\n", __func__);
	/* enable ufs_clkrst_bypass and ufs_tick_div */
	ufs_sys_ctrl_writel(host, 0x00E000E0, UFS_CRG_UFS_CFG);
}

int hufs_suspend(struct ufs_hba *hba, enum ufs_pm_op pm_op)
{
	struct hufs_host *host = (struct hufs_host *)hba->priv;
	/* set SOC_SCTRL_SCBAKDATA11_ADDR ufs bit to 0 when idle */
	lpcpu_idle_sleep_vote(ID_UFS, 0);

	if (ufshcd_is_runtime_pm(pm_op))
		return 0;

	if (host->in_suspend) {
		WARN_ON(1);
		return 0;
	}

	ufs_sys_ctrl_clr_bits(host, BIT_SYSCTRL_REF_CLOCK_EN, PHY_CLK_CTRL);
	/* delay 10 us for ufs_sys_ctrl_clr_bits completed */
	udelay(10);
	/* set ref_dig_clk override of PHY PCS to 0 */
	ufs_sys_ctrl_writel(host, 0x00100000, UFS_DEVICE_RESET_CTRL);
	/* close device's refclk */
	clk_disable_unprepare(host->clk_ufsio_ref);

	if (host->need_memrepair)
		ufs_kirin_memrepair_suspend(hba);

	host->in_suspend = true;

	return 0;
}

int hufs_resume(struct ufs_hba *hba, enum ufs_pm_op pm_op)
{
	struct hufs_host *host = (struct hufs_host *)hba->priv;
	int ret;

	/* set SOC_SCTRL_SCBAKDATA11_ADDR ufs bit to 1 when busy */
	lpcpu_idle_sleep_vote(ID_UFS, 1);

	if (!host->in_suspend)
		return 0;

	if (host->need_memrepair)
		ufs_kirin_memrepair_resume(hba);

	/* open device's refclk */
	ret = clk_prepare_enable(host->clk_ufsio_ref);
	if (ret) {
		pr_err("%s ,clk_prepare_enable failed\n", __func__);
		return ret;
	}

	udelay(1);
	/* set ref_dig_clk override of PHY PCS to 1 */
	ufs_sys_ctrl_writel(host, 0x00100010, UFS_DEVICE_RESET_CTRL);
	/* delay 10 us for set ref_dig_clk completed */
	udelay(10);
	ufs_sys_ctrl_set_bits(host, BIT_SYSCTRL_REF_CLOCK_EN, PHY_CLK_CTRL);

	host->in_suspend = false;
	return 0;
}

void hufs_device_hw_reset(struct ufs_hba *hba)
{
	struct hufs_host *host = (struct hufs_host *)hba->priv;

	if (likely(!(host->caps & USE_HUFS_MPHY_TC)))
		ufs_sys_ctrl_writel(host, MASK_UFS_DEVICE_RESET | 0,
			UFS_DEVICE_RESET_CTRL);
	else
		/* BIT 6 for SC_RSTDIS */
		ufs_i2c_writel(hba, (unsigned int)BIT(6), SC_RSTDIS);
	ufshcd_vops_vcc_power_on_off(hba);
	mdelay(1);

	if (likely(!(host->caps & USE_HUFS_MPHY_TC)))
		ufs_sys_ctrl_writel(host, MASK_UFS_DEVICE_RESET |
					  BIT_UFS_DEVICE_RESET,
				    UFS_DEVICE_RESET_CTRL);
	else
		/* BIT 6 for SC_RSTEN */
		ufs_i2c_writel(hba, (unsigned int) BIT(6), SC_RSTEN);
	/* some device need at least 40ms */
	mdelay(40);
}

static int ufs_kirin_check_is_h8(struct ufs_hba *hba)
{
	int err = 0;

	if (likely(!hba->host->is_emulator)) {
		err = hufs_check_hibern8(hba);
		if (err)
			pr_err("hufs_check_hibern8 error\n");
		return err;
	}
	return err;
}

static void ufs_kirin_check_and_config_rate(struct ufs_hba *hba)
{
	struct hufs_host *host = (struct hufs_host *)hba->priv;

	if (host->caps & USE_RATE_B) {
		/* PA_HSSeries */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_HSSERIES, 0x0), 0x2);
		/* MPHY CBRATESEL */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBRATESEL, 0x0), 0x1);
		/* MPHY CBOVRCTRL2 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBOVRCTRL2, 0x0), 0x2D);
		/* MPHY CBOVRCTRL3 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBOVRCTRL3, 0x0), 0x1);

		/* MPHY CBOVRCTRL4 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBOVRCTRL4, 0x0), 0x98);
		/* MPHY CBOVRCTRL5 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBOVRCTRL5, 0x0), 0x1);

		/* Unipro VS_MphyCfgUpdt */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD085, 0x0), 0x1);
		/* MPHY RXOVRCTRL4 rx0 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RXOVRCTRL4, 0x4), 0x58);
		/* MPHY RXOVRCTRL4 rx1 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RXOVRCTRL4, 0x5), 0x58);
		/* MPHY RXOVRCTRL5 rx0 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RXOVRCTRL5, 0x4), 0xB);
		/* MPHY RXOVRCTRL5 rx1 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RXOVRCTRL5, 0x5), 0xB);
		/* MPHY RXSQCONTROL rx0 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RXSQCONTROL, 0x4), 0x1);
		/* MPHY RXSQCONTROL rx1 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RXSQCONTROL, 0x5), 0x1);
		/* Unipro VS_MphyCfgUpdt */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD085, 0x0), 0x1);
	} else {
		/* PA_HSSeries */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_HSSERIES, 0x0), 0x1);
		/* MPHY CBRATESEL */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBRATESEL, 0x0), 0x0);
		/* MPHY CBOVRCTRL2 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBOVRCTRL2, 0x0), 0x4C);
		/* MPHY CBOVRCTRL3 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBOVRCTRL3, 0x0), 0x1);

		/* MPHY CBOVRCTRL4 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBOVRCTRL4, 0x0), 0x82);
		/* MPHY CBOVRCTRL5 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBOVRCTRL5, 0x0), 0x1);

		/* Unipro VS_MphyCfgUpdt */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD085, 0x0), 0x1);
		/* MPHY RXOVRCTRL4 rx0 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RXOVRCTRL4, 0x4), 0x18);
		/* MPHY RXOVRCTRL4 rx1 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RXOVRCTRL4, 0x5), 0x18);
		/* MPHY RXOVRCTRL5 rx0 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RXOVRCTRL5, 0x4), 0xD);
		/* MPHY RXOVRCTRL5 rx1 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RXOVRCTRL5, 0x5), 0xD);
		/* MPHY RXSQCONTROL rx0 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RXSQCONTROL, 0x4), 0x1);
		/* MPHY RXSQCONTROL rx1 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RXSQCONTROL, 0x5), 0x1);
		/* Unipro VS_MphyCfgUpdt */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD085, 0x0), 0x1);
	}
}

/* hci asic mphy specific configuration */
static int ufs_kirin_dme_setup_hci_asic_mphy(struct ufs_hba *hba)
{
	uint32_t value = 0;
	int err;

	pr_info("%s ++\n", __func__);

	/* Unipro VS_mphy_disable */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD0C1, 0x0), 0x1);

	ufs_kirin_check_and_config_rate(hba);

	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBENBLCPBATTRWR, 0x0), 0x1);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD085, 0x0), 0x1);

	/* RX_Hibern8Time_Capability */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RX_HIBERN8TIME_CAPABILITY, 0x4), 0xA);
	/* RX_Hibern8Time_Capability */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RX_HIBERN8TIME_CAPABILITY, 0x5), 0xA);
	/* RX_Min_ActivateTime */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RX_MIN_ACTIVATETIME_CAPABILITY, 0x4), 0xA);
	/* RX_Min_ActivateTime */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RX_MIN_ACTIVATETIME_CAPABILITY, 0x5), 0xA);

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
	/* Thibernate Tx */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(TX_HIBERN8TIME_CAPABILITY, 0x0), 0x5);
	/* Thibernate Tx */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(TX_HIBERN8TIME_CAPABILITY, 0x1), 0x5);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD085, 0x0), 0x1);

	/* Unipro VS_mphy_disable */
	ufshcd_dme_get(hba, UIC_ARG_MIB_SEL(0xD0C1, 0x0), &value);
	if (value != 0x1)
		pr_warn("Warring!!! Unipro VS_mphy_disable is 0x%x\n", value);

	/* Unipro VS_mphy_disable */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD0C1, 0x0), 0x0);

	err = ufs_kirin_check_is_h8(hba);

	pr_info("%s --\n", __func__);
	return err;
}

int hufs_link_startup_pre_change(struct ufs_hba *hba)
{
	int err;
	uint32_t value = 0;
	uint32_t reg;
	struct hufs_host *host = (struct hufs_host *)hba->priv;

	pr_info("%s ++\n", __func__);

	/* for hufs MPHY */
	hufs_mphy_updata_temp_sqvref(hba, host);

	/*
	 * ARIES's FPGA not maintain anymore, so don't need to add
	 * the new "ufs-kirin-use-hci-mphy-ac" in so many product 's dts,
	 * and use hci-mphy-ac in branch by default here
	 */
	err = ufs_kirin_dme_setup_hci_asic_mphy(hba);
	if (err)
		return err;

	/* disable auto H8 */
	reg = ufshcd_readl(hba, REG_CONTROLLER_AHIT);
	reg = reg & (~UFS_AHIT_AH8ITV_MASK);
	ufshcd_writel(hba, reg, REG_CONTROLLER_AHIT);

	/* for hufs MPHY */
	hufs_mphy_updata_vswing_fsm_ocs5(hba, host);

	/* Unipro PA_Local_TX_LCC_Enable */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_LOCAL_TX_LCC_ENABLE, 0x0), 0x0);
	/* close Unipro VS_Mk2ExtnSupport */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD0AB, 0x0), 0x0);
	ufshcd_dme_get(hba, UIC_ARG_MIB_SEL(0xD0AB, 0x0), &value);
	if (value)
		/* Ensure close success */
		pr_warn("Warring!!! close VS_Mk2ExtnSupport failed\n");

	/* FPGA with HUFS PHY not configure equalizer */
	if (host->tx_equalizer == TX_EQUALIZER_35DB) {
		hufs_mphy_write(hba, 0x1002, 0xAC78);
		hufs_mphy_write(hba, 0x1102, 0xAC78);
		hufs_mphy_write(hba, 0x1003, 0x2440);
		hufs_mphy_write(hba, 0x1103, 0x2440);
	} else if (host->tx_equalizer == TX_EQUALIZER_60DB) {
		hufs_mphy_write(hba, 0x1002, 0xAA78);
		hufs_mphy_write(hba, 0x1102, 0xAA78);
		hufs_mphy_write(hba, 0x1003, 0x2640);
		hufs_mphy_write(hba, 0x1103, 0x2640);
	}
	/* for hufs MPHY */
	hufs_mphy_busdly_config(hba, host);

	pr_info("%s --\n", __func__);

	return err;
}

static void hufs_mphy_link_post_config(struct ufs_hba *hba)
{
	struct hufs_host *host = (struct hufs_host *)hba->priv;
	uint32_t tx_lane_num = 1; /* config tx and rx lane is one */
	uint32_t rx_lane_num = 1;

	if (host->caps & USE_HUFS_MPHY_TC) {
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
			ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00c2, 0x5), 0x0);
	}
}

int hufs_link_startup_post_change(struct ufs_hba *hba)
{
	struct hufs_host *host = (struct hufs_host *)hba->priv;

	pr_info("%s ++\n", __func__);
	/* Unipro DL_AFC0CreditThreshold */
	ufshcd_dme_set(hba, UIC_ARG_MIB(DL_AFC0CREDITTHRESHOLD), 0x0);
	/* Unipro DL_TC0OutAckThreshold */
	ufshcd_dme_set(hba, UIC_ARG_MIB(DL_TC0OUTACKTHRESHOLD), 0x0);
	/* Unipro DL_TC0TXFCThreshold */
	ufshcd_dme_set(hba, UIC_ARG_MIB(DL_TC0TXFCTHRESHOLD), 0x9);
	/* for hufs MPHY */
	hufs_mphy_link_post_config(hba);

	if (host->caps & BROKEN_CLK_GATE_BYPASS) {
		/* not bypass ufs clk gate */
		ufs_sys_ctrl_clr_bits(host, MASK_UFS_CLK_GATE_BYPASS,
				      CLOCK_GATE_BYPASS);
		ufs_sys_ctrl_clr_bits(host, MASK_UFS_SYSCRTL_BYPASS,
				      UFS_SYSCTRL);
	}

	if (host->hba->caps & UFSHCD_CAP_AUTO_HIBERN8)
		/* disable power-gating in auto hibernate 8 */
		ufshcd_rmwl(hba, LP_AH8_PGE, 0, UFS_REG_OCPTHRTL);

	/* select received symbol cnt */
	ufshcd_dme_set(hba, UIC_ARG_MIB(0xd09a), 0x80000000);
	/* reset counter0 and enable */
	ufshcd_dme_set(hba, UIC_ARG_MIB(0xd09c), 0x00000005);

	pr_info("%s --\n", __func__);

	return 0;
}

void hufs_pwr_change_pre_change(struct ufs_hba *hba,
	struct ufs_pa_layer_attr *dev_req_params)
{
	uint32_t value = 0;

	pr_info("%s ++\n", __func__);
#ifdef CONFIG_HISI_DEBUG_FS
	pr_info("device manufacturer_id is 0x%x\n", hba->manufacturer_id);
#endif
	/*
	 * ARIES platform need to set SaveConfigTime to 0x13, and change sync
	 * length to maximum value
	 */
	/* VS_DebugSaveConfigTime */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32) 0xD0A0), 0x13);
	/* g1 sync length */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32) PA_TXHSG1SYNCLENGTH), 0x4f);
	/* g2 sync length */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32) PA_TXHSG2SYNCLENGTH), 0x4f);
	/* g3 sync length */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32) PA_TXHSG3SYNCLENGTH), 0x4f);
	/* PA_Hibern8Time */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32) PA_HIBERN8TIME), 0xA);
	/* PA_Tactivate */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32) PA_TACTIVATE), 0xA);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xd085, 0x0), 0x01);

	ufshcd_dme_get(hba, UIC_ARG_MIB(PA_TACTIVATE), &value);
	if (value < 0x7)
		/* update PaTactive */
		ufshcd_dme_set(hba, UIC_ARG_MIB(PA_TACTIVATE), 0x7);
	/* PA_TxSkip */
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_TXSKIP), 0x0);

	/* PA_PWRModeUserData0 = 8191, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_PWRMODEUSERDATA0), 8191);
	/* PA_PWRModeUserData1 = 65535, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_PWRMODEUSERDATA1), 65535);
	/* PA_PWRModeUserData2 = 32767, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_PWRMODEUSERDATA2), 32767);
	/* DME_FC0ProtectionTimeOutVal = 8191, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32) DME_FC0_PROTECTION_TIMEOUT_VAL), 8191);
	/* DME_TC0ReplayTimeOutVal = 65535, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32) DME_TC0_REPLAY_TIMEOUT_VAL), 65535);
	/* DME_AFC0ReqTimeOutVal = 32767, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32) DME_AFC0_REQ_TIMEOUT_VAL), 32767);
	/* PA_PWRModeUserData3 = 8191, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_PWRMODEUSERDATA3), 8191);
	/* PA_PWRModeUserData4 = 65535, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_PWRMODEUSERDATA4), 65535);
	/* PA_PWRModeUserData5 = 32767, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_PWRMODEUSERDATA5), 32767);
	/* DME_FC1ProtectionTimeOutVal = 8191, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32) DME_FC1_PROTECTION_TIMEOUT_VAL), 8191);
	/* DME_TC1ReplayTimeOutVal = 65535, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32) DME_TC1_REPLAY_TIMEOUT_VAL), 65535);
	/* DME_AFC1ReqTimeOutVal = 32767, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32) DME_AFC1_REQ_TIMEOUT_VAL), 32767);

	pr_info("%s --\n", __func__);
}

/*
 * Soc init will reset host controller, all register value will lost
 * including memory address, doorbell and AH8 AGGR
 */
void hufs_full_reset(struct ufs_hba *hba)
{
#ifdef CONFIG_HUAWEI_UFS_DSM
	dsm_ufs_disable_volt_irq(hba);
#endif
	disable_irq(hba->irq);

	/* wait for 1000 ms to be sure axi entered to idle state */
	msleep(1000);
	ufs_soc_init(hba);
	enable_irq(hba->irq);
#ifdef CONFIG_HUAWEI_UFS_DSM
	dsm_ufs_enable_volt_irq(hba);
#endif
}

void hufs_pre_hce_notify(struct ufs_hba *hba)
{
	struct hufs_host *host = (struct hufs_host *)hba->priv;

	BUG_ON(!host->pericrg || !host->ufs_sys_ctrl || !host->pctrl ||
		!host->sysctrl || !host->pmctrl);

	return;
}
