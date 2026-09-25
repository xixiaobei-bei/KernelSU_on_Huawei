/*
 * ufs-pisces.c
 *
 * ufs configuration for pisces
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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
#include "dsm_ufs.h"
#include "ufs-hisi.h"
#include "ufs-kirin.h"
#include "ufshcd.h"
#include <linux/hisi/hisi_idle_sleep.h>
#include <linux/mfd/hisi_pmic.h>
#include <pmic_interface.h>
#include <soc_actrl_interface.h>
#include <soc_hsdt_crg_interface.h>
#include <soc_pmctrl_interface.h>
#include <soc_sctrl_interface.h>
#include <soc_ufs_sysctrl_interface.h>

#define POLLING_RETRY 2000
#define POLLING_RETRY_4W 40000
#define COMPLETION_TIMEOUT_MSEC 20
#define MPHY_BIT_REG_VCO_SW_VCAL BIT(7)

static const struct ufs_attr_cfg hisi_mphy_v200_pre_link_attr[] = {
	{ 0x0000156A, 0x00000002 }, /* PA_HSSeries */
	{ 0x00020009, 0x00000001 }, /* Rx SKP_DET_SEL, lane0 */
	{ 0x00030009, 0x00000001 }, /* Rx SKP_DET_SEL, lane1 */
	{ 0x000000DF, 0x00000003 }, /* VCO_AUTO_CHG */
	/* FPGA can only support HS Gear 1, Gear 4 for only test chip */
	{ 0x00000023, 0x00000004 }, /* TX_HSGEAR */
	{ 0x00010023, 0x00000004 }, /* TX_HSGEAR */
	{ 0x000200a3, 0x00000004 }, /* RX_HSGEAR */
	{ 0x000300a3, 0x00000004 }, /* RX_HSGEAR */
	{ 0x000200F1, 0x00000004 }, /* RX_SQ_VREF, lane0 ,RX_SQ_VREF_140mv */
	{ 0x000300F1, 0x00000004 }, /* RX_SQ_VREF, lane1 ,RX_SQ_VREF_140mv */
	{ 0x00020003, 0x0000000A }, /* AD_DIF_P_LS_TIMEOUT_VAL, lane0 */
	{ 0x00030003, 0x0000000A }, /* AD_DIF_P_LS_TIMEOUT_VAL, lane1 */
	{ 0x00020004, 0x00000064 }, /* AD_DIF_N_LS_TIMEOUT_VAL, lane0 */
	{ 0x00030004, 0x00000064 }, /* AD_DIF_N_LS_TIMEOUT_VAL, lane1 */
	{ 0x000200cf, 0x00000002 }, /* RX_STALL */
	{ 0x000300cf, 0x00000002 }, /* RX_STALL */
	{ 0x000200d0, 0x00000002 }, /* RX_SLEEP */
	{ 0x000300d0, 0x00000002 }, /* RX_SLEEP */
	{ 0x000200F0, 0x00000001 }, /* RX enable, lane0 */
	{ 0x000300F0, 0x00000001 }, /* RX enable, lane1 */
	{ 0x00000059, 0x0000000f }, /* reg 0x59,TX0 */
	{ 0x00010059, 0x0000000f }, /* reg 0x59,TX1 */
	{ 0x0000005A, 0x00000000 }, /* reg 0x5A,TX0 */
	{ 0x0001005A, 0x00000000 }, /* reg 0x5A,TX1 */
	{ 0x0000005B, 0x0000000f }, /* reg 0x5B,TX0 */
	{ 0x0001005B, 0x0000000f }, /* reg 0x5B,TX1 */
	{ 0x0000005C, 0x00000000 }, /* reg 0x5C,TX0 */
	{ 0x0001005C, 0x00000000 }, /* reg 0x5C,TX1 */
	{ 0x0000005D, 0x00000000 }, /* reg 0x5D,TX0 */
	{ 0x0001005D, 0x00000000 }, /* reg 0x5D,TX1 */
	{ 0x0000005E, 0x0000000a }, /* reg 0x5E,TX0 */
	{ 0x0001005E, 0x0000000a }, /* reg 0x5E,TX1 */
	{ 0x0000005F, 0x0000000a }, /* reg 0x5F,TX0 */
	{ 0x0001005F, 0x0000000a }, /* reg 0x5F,TX1 */
	{ 0x0000007A, 0x00000000 }, /* reg 0x7A,TX0 */
	{ 0x0001007A, 0x00000000 }, /* reg 0x7A,TX1 */
	{ 0x0000007B, 0x00000000 }, /* reg 0x7B,TX0 */
	{ 0x0001007B, 0x00000000 }, /* reg 0x7B,TX1 */
	/* Tell UFS that RX can exit H8 only when host TX exits H8 first */
	{ 0x000200C3, 0x00000004 }, /* RX_EXTERNAL_H8_EXIT_EN, lane0 */
	{ 0x000300C3, 0x00000004 }, /* RX_EXTERNAL_H8_EXIT_EN, lane1 */
	{ 0x000200f6, 0x00000001 }, /* RX_DLF Lane 0 */
	{ 0x000300f6, 0x00000001 }, /* RX_DLF Lane 1 */
	{ 0x000000C7, 0x00000003 }, /* measure the power, can close it */
	{ 0x000000C8, 0x00000003 }, /* measure the power, can close it */
	{ 0x000000c5, 0x00000003 }, /* RG_PLL_RXHS_EN */
	{ 0x000000c6, 0x00000003 }, /* RG_PLL_RXLS_EN */

	{ 0x000200E9, 0x00000000 }, /* RX_HS_DATA_VALID_TIMER_VAL0 */
	{ 0x000300E9, 0x00000000 }, /* RX_HS_DATA_VALID_TIMER_VAL0 */
	{ 0x000200EA, 0x00000010 }, /* RX_HS_DATA_VALID_TIMER_VAL1 */
	{ 0x000300EA, 0x00000010 }, /* RX_HS_DATA_VALID_TIMER_VAL1 */

	/* rx_setting_better_perform */
	{ 0x000200f4, 0x00000000 },
	{ 0x000300f4, 0x00000000 },
	{ 0x000200f3, 0x00000000 },
	{ 0x000300f3, 0x00000000 },
	{ 0x000200f2, 0x00000003 },
	{ 0x000300f2, 0x00000003 },
	{ 0x000200f6, 0x00000002 },
	{ 0x000300f6, 0x00000002 },
	{ 0x000200f5, 0x00000000 },
	{ 0x000300f5, 0x00000000 },
	{ 0x000200fc, 0x0000001f },
	{ 0x000300fc, 0x0000001f },
	{ 0x000200fd, 0x00000000 },
	{ 0x000300fd, 0x00000000 },
	{ 0x000200fb, 0x00000005 },
	{ 0x000300fb, 0x00000005 },
	{ 0x00020011, 0x00000011 },
	{ 0x00030011, 0x00000011 },
	/* need to check the value of 0x9529 in the next version of test chip */
	{ 0x00009529, 0x00000006 }, /* VS_DebugSaveConfigTime */
	{ 0x0000D014, 0x00000001 },
	/* update */ /* Hisi UniPro */
	{ 0, 0 },
};

static const struct ufs_attr_cfg hisi_mphy_v200_post_link_attr[] = {
	{ 0x00003000, 0x00000000 }, /* N_ DeviceID */
	{ 0x00003001, 0x00000001 }, /* N_DeviceID_valid */
	{ 0x00004020, 0x00000000 }, /* T_ConnectionState */
	{ 0x00004022, 0x00000000 }, /* T_PeerCPortID */
	{ 0x00004021, 0x00000001 }, /* T_PeerDeviceID */
	{ 0x00004023, 0x00000000 }, /* T_TrafficClass */
	{ 0x00004020, 0x00000001 }, /* T_ConnectionState */
	{ 0x0002000a, 0x0000001E }, /* RX H8_TIMEOUT_VAL, Lane 0 */
	{ 0x0003000a, 0x0000001E }, /* RX H8_TIMEOUT_VAL, Lane 1 */
	{ 0x000200EB, 0x00000064 }, /* AD_DIF_N_TIMEOUT_VAL */
	{ 0x000300EB, 0x00000064 }, /* AD_DIF_N_TIMEOUT_VAL */
	{ 0x0002000E, 0x000000F0 }, /* RX_H8_EXIT_TIME */
	{ 0x0003000E, 0x000000F0 }, /* RX_H8_EXIT_TIME */
	{ 0x000000da, 0x0000004B }, /* PLL_LOCK_TIME_VAL */
	{ 0x000000dd, 0x000000CB }, /* PLL_RSTB_TIME_VAL */
	{ 0, 0 },
};

static const struct ufs_attr_cfg hisi_mphy_v120_pre_link_attr[] = {
	{ 0x0000156A, 0x2 }, /* PA_HSSeries */

	{ 0x00000023, 0x00000004 },
	{ 0x00010023, 0x00000004 },
	{ 0x000200a3, 0x00000004 },
	{ 0x000300a3, 0x00000004 },
	{ 0x000200f1, 0x00000004 },
	{ 0x000300f1, 0x00000004 },
	{ 0x00020004, 0x00000064 },
	{ 0x00030004, 0x00000064 },
	{ 0x00020003, 0x0000000A },
	{ 0x00030003, 0x0000000A },
	{ 0x000200f0, 0x00000001 },
	{ 0x000300f0, 0x00000001 },

	{ 0x00020009, 0x00000001 }, /* Rx SKP_DET_SEL, lane0 */
	{ 0x00030009, 0x00000001 }, /* Rx SKP_DET_SEL, lane1 */
	{ 0x000000DF, 0x00000003 }, /* VCO_AUTO_CHG */
	{ 0x000000C7, 0x3 }, /* measure the power, can close it */
	{ 0x000000C8, 0x3 }, /* measure the power, can close it */
	{ 0x000200cf, 0x00000002 }, /* RX_STALL */
	{ 0x000300cf, 0x00000002 }, /* RX_STALL */
	{ 0x000200d0, 0x00000002 }, /* RX_SLEEP */
	{ 0x000300d0, 0x00000002 }, /* RX_SLEEP */
	{ 0x000200cc, 0x00000003 }, /* RX_HS_CLK_EN */
	{ 0x000300cc, 0x00000003 }, /* RX_HS_CLK_EN */
	{ 0x000200cd, 0x00000003 }, /* RX_LS_CLK_EN */
	{ 0x000300cd, 0x00000003 }, /* RX_LS_CLK_EN */
	{ 0x000000c5, 0x00000003 }, /* RG_PLL_RXHS_EN */
	{ 0x000000c6, 0x00000003 }, /* RG_PLL_RXLS_EN */
	{ 0x000200E9, 0x00000000 }, /* RX_HS_DATA_VALID_TIMER_VAL0 */
	{ 0x000300E9, 0x00000000 }, /* RX_HS_DATA_VALID_TIMER_VAL0 */
	{ 0x000200EA, 0x00000010 }, /* RX_HS_DATA_VALID_TIMER_VAL1 */
	{ 0x000300EA, 0x00000010 }, /* RX_HS_DATA_VALID_TIMER_VAL1 */
	{ 0x00001552, 0x0000004F }, /* PA_TxHsG1SyncLength */
	{ 0x00001554, 0x0000004F }, /* PA_TxHsG2SyncLength */
	{ 0x00001556, 0x0000004F }, /* PA_TxHsG3SyncLength */
	{ 0x00009509, 0x0000004F },
	{ 0x0000950A, 0x0000000F },
	{ 0x000200F4, 0x1 }, /* RX_EQ_SEL_R */
	{ 0x000300F4, 0x1 }, /* RX_EQ_SEL_R */
	{ 0x000200F2, 0x3 }, /* RX_EQ_SEL_C */
	{ 0x000300F2, 0x3 }, /* RX_EQ_SEL_C */
	{ 0x000200FB, 0x3 }, /* RX_VSEL */
	{ 0x000300FB, 0x3 }, /* RX_VSEL */
	{ 0x000200F6, 0x3 }, /* RX_DLF Lane 0 */
	{ 0x000300F6, 0x3 }, /* RX_DLF Lane 1 */
	{ 0x0002000a, 0x00000002 }, /* RX H8_TIMEOUT_VAL, Lane 0 */
	{ 0x0003000a, 0x00000002 }, /* RX H8_TIMEOUT_VAL, Lane 1 */
	{ 0x000000d4, 0x00000031 }, /* RG_PLL_DMY0 */
	{ 0x00000073, 0x00000004 }, /* TX_PHY_CONFIG II */
	{ 0x00010073, 0x00000004 }, /* TX_PHY_CONFIG II */
	{ 0x000200F0, 0x00000001 }, /* RX enable, lane0 */
	{ 0x000300F0, 0x00000001 }, /* RX enable, lane1 */
	{ 0x0000D014, 0x1 }, /* Unipro VS_MphyCfgUpdt */
	{ 0, 0 },
};

static const struct ufs_attr_cfg hisi_mphy_v120_post_link_attr[] = {
	{ 0x00003000, 0x0 }, /* N_ DeviceID */
	{ 0x00003001, 0x1 }, /* N_DeviceID_valid */
	{ 0x00004020, 0x0 }, /* T_ConnectionState */
	{ 0x00004022, 0x0 }, /* T_PeerCPortID */
	{ 0x00004021, 0x1 }, /* T_PeerDeviceID */
	{ 0x00004023, 0x0 }, /* T_TrafficClass */
	{ 0x00004020, 0x1 }, /* T_ConnectionState */
	{ 0x0002000a, 0x0000001E }, /* RX H8_TIMEOUT_VAL, Lane 0 */
	{ 0x0003000a, 0x0000001E }, /* RX H8_TIMEOUT_VAL, Lane 1 */
	{ 0x000200EB, 0x64 }, /* AD_DIF_N_TIMEOUT_VAL */
	{ 0x000300EB, 0x64 }, /* AD_DIF_N_TIMEOUT_VAL */
	{ 0x0002000E, 0xF0 }, /* RX_H8_EXIT_TIME */
	{ 0x0003000E, 0xF0 }, /* RX_H8_EXIT_TIME */
	{ 0x000000ca, 0x3 },
	{ 0x000015a8, 10 }, /* PA_TActivate */
	{ 0, 0 },
};

/* ASIC */
static const struct ufs_attr_cfg hisi_mphy_v300_pre_link_attr[] = {
	{ 0x0000156A, 0x00000002 }, /* PA_HSSeries */
	{ 0x00020009, 0x00000001 }, /* Rx SKP_DET_SEL, lane0 */
	{ 0x00030009, 0x00000001 }, /* Rx SKP_DET_SEL, lane1 */
	{ 0x000000DF, 0x00000002 }, /* VCO_AUTO_CHG */
	{ 0x00020003, 0x0000000A }, /* AD_DIF_P_LS_TIMEOUT_VAL, lane0 */
	{ 0x00030003, 0x0000000A }, /* AD_DIF_P_LS_TIMEOUT_VAL, lane1 */
	{ 0x00020004, 0x00000064 }, /* AD_DIF_N_LS_TIMEOUT_VAL, lane0 */
	{ 0x00030004, 0x00000064 }, /* AD_DIF_N_LS_TIMEOUT_VAL, lane1 */
	{ 0x00020017, 0x00000084 }, /* RX_AFE_CTRL lane 0, HW calibration bit2 and 7 */
	{ 0x00030017, 0x00000084 }, /* RX_AFE_CTRL lane 1, HW calibration bit2 and 7 */
	{ 0x00000057, 0x00000000 }, /* AFE_CONFIG_IX lane 0 */
	{ 0x00010057, 0x00000000 }, /* AFE_CONFIG_IX lane 1 */
	{ 0x00000059, 0x00000022 }, /* AFE_CONFIG_XII lane 0 */
	{ 0x00010059, 0x00000022 }, /* AFE_CONFIG_XII lane 1 */
	{ 0x0000007B, 0x00000023 }, /*  TX_AFE_CONFIG_II lane 0 */
	{ 0x0001007B, 0x00000023 }, /*  TX_AFE_CONFIG_II lane 1 */
	{ 0x000200F0, 0x00000001 }, /* RX enable, lane0 */
	{ 0x000300F0, 0x00000001 }, /* RX enable, lane1 */
	{ 0x000200C7, 0x1 }, /* enable override */
	{ 0x000300C7, 0x1 }, /* enable override */
	{ 0x0002009D, 0x1 }, /* RX_HS_Equalizer_Setting_Capability lane 0 */
	{ 0x0003009D, 0x1 }, /* RX_HS_Equalizer_Setting_Capability lane 1 */
	{ 0x000200C7, 0x0 }, /* disable override */
	{ 0x000300C7, 0x0 }, /* disable override */
	{ 0x1564, 0xFA }, /* PA_TxTrailingClocks */
	{ 0x0000D014, 0x00000001 }, { 0, 0 },
};

static const struct ufs_attr_cfg hisi_mphy_v300_post_link_attr[] = {
	{ 0x00003000, 0x00000000 }, /* N_DeviceID */
	{ 0x00003001, 0x00000001 }, /* N_DeviceID_valid */
	{ 0x00004020, 0x00000000 }, /* T_ConnectionState */
	{ 0x00004022, 0x00000000 }, /* T_PeerCPortID */
	{ 0x00004021, 0x00000001 }, /* T_PeerDeviceID */
	{ 0x00004023, 0x00000000 }, /* T_TrafficClass */
	{ 0x00004020, 0x00000001 }, /* T_ConnectionState */
	{ 0x0002000A, 0x0000001E }, /* RX H8_TIMEOUT_VAL, Lane 0 */
	{ 0x0003000A, 0x0000001E }, /* RX H8_TIMEOUT_VAL, Lane 1 */
	{ 0x000200EB, 0x00000064 }, /* AD_DIF_N_TIMEOUT_VAL */
	{ 0x000300EB, 0x00000064 }, /* AD_DIF_N_TIMEOUT_VAL */
	{ 0x0002000E, 0x000000F0 }, /* RX_H8_EXIT_TIME */
	{ 0x0003000E, 0x000000F0 }, /* RX_H8_EXIT_TIME */
	{ 0x000000DF, 0x00000003 }, /* VCO_AUTO_CHG */
	{ 0x000015A8, 0x00000005 }, /* PA_TActivate */
	{ 0, 0 },
};

void ufs_kirin_regulator_init(struct ufs_hba *hba)
{
}

void ufs_clk_init(struct ufs_hba *hba)
{
}

static void ufs_release_reset_subsys_crg(struct ufs_kirin_host *host)
{
	u32 value = 0;
	int retry = POLLING_RETRY;

	sys_ctrl_set_bits(host, BIT(IP_RST_UFS_SUBSYS_CRG),
			  SOC_SCTRL_SCPERRSTDIS1);
	while (retry--) {
		value = readl(SOC_SCTRL_SCPERRSTSTAT1_ADDR(host->sysctrl));
		if (!(value & IP_RST_UFS_SUBSYS_CRG))
			break;
		udelay(1);
	}
	if (retry < 0)
		pr_err("UFS Sub-sys CRG reset failed\n");
}

static void ufs_polling_pmc_idle(struct ufs_kirin_host *host)
{
	u32 value = 0;
	int retry = POLLING_RETRY;

	/* Polling the PMC idle ack0 register */
	while (retry--) {
		value = readl(
			SOC_PMCTRL_NOC_POWER_IDLEACK_0_ADDR(host->pmctrl));
		if (!(value & NOC_UFS_POWER_IDLEACK_0))
			break;
		udelay(1);
	}
	if (retry < 0)
		pr_err("UFS Power idle exit failed\n");

	/* Polling the PMC idle status */
	retry = POLLING_RETRY;
	while (retry--) {
		value = readl(SOC_PMCTRL_NOC_POWER_IDLE_0_ADDR(host->pmctrl));
		if (!(value & NOC_UFS_POWER_IDLE_0))
			break;
		udelay(1);
	}
	if (retry < 0)
		pr_err("UFS Power idle exit status err\n");
}

static void ufs_ip_reset_for_hybrid(struct ufs_kirin_host *host)
{
	u32 reg;

	writel(1 << SOC_UFS_Sysctrl_UFS_UMECTRL_ufs_ies_en_mask_START,
	       SOC_UFS_Sysctrl_UFS_UMECTRL_ADDR(host->ufs_sys_ctrl));
	writel((1 << (SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START +
		      BIT_ENABLE_MASK)) |
		       0,
	       SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));

	writel((1 << (SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_arst_ufs_START +
		      BIT_ENABLE_MASK)) |
		       (1 << SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_arst_ufs_START),
	       SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));

	writel(1 << (SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START +
		     BIT_ENABLE_MASK),
	       SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));
	udelay(100); /* wait 100 us */
	writel((1 << (SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START +
		      BIT_ENABLE_MASK)) |
		       (1 << SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START),
	       SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));
	reg = readl(SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));
	if (reg & (1 << SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START))
		mdelay(1);
}

void ufs_soc_init(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = NULL;

	host = (struct ufs_kirin_host *)hba->priv;

	dev_info(hba->dev, "%s ++\n", __func__);

	/* 1. Set SD and DS to 0 */
	writel(0x00030000, SOC_ACTRL_AO_MEM_CTRL2_ADDR(host->actrl));

	/* 2. release reset of UFS subsystem CRG */
	ufs_release_reset_subsys_crg(host);

	/* 3. enable the clk of UFS side of NOC async bridge */
	hsdt_crg_set_bits(host, BIT(GT_CLK_UFS_NOC_ASYNCBRG),
			  SOC_HSDT_CRG_PEREN1);
	/* release the reset of UFS side of NOC async bridge */
	hsdt_crg_set_bits(host, BIT(IP_RST_UFS_NOC_ASYNCBRG),
			  SOC_HSDT_CRG_PERRSTDIS0);

	/* 5. enable ip_rst_ufs_subsys, reset ufs sub-system */
	sys_ctrl_set_bits(host, BIT(IP_RST_UFS_SUBSYS), SOC_SCTRL_SCPERRSTEN1);
	/* 6. setup ufs sub-system clk to 224MHz */
	writel(0x003F0006, SOC_SCTRL_SCCLKDIV9_ADDR(host->sysctrl));

	/* 10. Release MPHY related ISO */
	writel(0x00210000, SOC_ACTRL_ISO_EN_GROUP0_PERI_ADDR(host->actrl));

	/* 11. Enable clk_ufs_subsys clock */
	writel(BIT(SOC_SCTRL_SCPEREN4_gt_clk_ufs_subsys_START),
	       SOC_SCTRL_SCPEREN4_ADDR(host->sysctrl));

	/*
	 * 12. handshake of UFS powerdomain
	 * setup PMC registers, exit ldle mode
	 */
	writel(0x00020000, SOC_PMCTRL_NOC_POWER_IDLEREQ_0_ADDR(host->pmctrl));

	ufs_polling_pmc_idle(host);

	/* device reset */
	ufs_kirin_device_hw_reset(hba);

	/* 15. release ufs mast-reset */
	sys_ctrl_set_bits(host, BIT(IP_RST_UFS_SUBSYS), SOC_SCTRL_SCPERRSTDIS1);
	/* release internal resets */
	if (ufshcd_is_hisi_ufs_hc(hba))
		writel(0x01FF01FF,
		       SOC_UFS_Sysctrl_UFS_RESET_CTRL_ADDR(host->ufs_sys_ctrl));

	mdelay(1);

	if (!ufshcd_is_hisi_ufs_hc(hba))
		ufs_ip_reset_for_hybrid(host);

	/* set SOC_SCTRL_SCBAKDATA11_ADDR ufs bit to 1 when init */
	if (!ufshcd_is_auto_hibern8_allowed(hba))
		hisi_idle_sleep_vote(ID_UFS, 1);
	dev_info(hba->dev, "%s --\n", __func__);
}

int ufs_kirin_suspend_before_set_link_state(
	struct ufs_hba *hba, enum ufs_pm_op pm_op)
{
	return 0;
}

int ufs_kirin_resume_after_set_link_state(
	struct ufs_hba *hba, enum ufs_pm_op pm_op)
{
	return 0;
}

static void mphy_iso_enable(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = hba->priv;

	/* MPHY power off on FPGA */
	if (host->caps & USE_HISI_MPHY_TC)
		writel(0x00010001,
		       SOC_ACTRL_ISO_EN_GROUP0_PERI_ADDR(host->actrl));
	else
		writel(0x00200020,
		       SOC_ACTRL_ISO_EN_GROUP0_PERI_ADDR(host->actrl));
}

static void mphy_iso_disable(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = hba->priv;

	if (host->caps & USE_HISI_MPHY_TC)
		writel(0x00010000,
		       SOC_ACTRL_ISO_EN_GROUP0_PERI_ADDR(host->actrl));
	else
		writel(0x00200000,
		       SOC_ACTRL_ISO_EN_GROUP0_PERI_ADDR(host->actrl));
}

static void ufs_suspend_polling_dmc(
	struct ufs_kirin_host *host, struct ufs_hba *hba)
{
	int retry = POLLING_RETRY;
	int value;

	/* Polling the PMC idle ack0 register */
	while (retry--) {
		value = readl(
			SOC_PMCTRL_NOC_POWER_IDLEACK_0_ADDR(host->pmctrl));
		if (value & NOC_UFS_POWER_IDLEACK)
			break;
		udelay(1);
	}
	if (retry < 0)
		dev_err(hba->dev, "UFS Power idle failed\n");

	/* Polling the PMC idle status */
	retry = POLLING_RETRY;
	while (retry--) {
		value = readl(SOC_PMCTRL_NOC_POWER_IDLE_0_ADDR(host->pmctrl));
		if (value & NOC_UFS_POWER_IDLE)
			break;
		udelay(1);
	}
	if (retry < 0)
		dev_err(hba->dev, "UFS Power idle status err\n");
}

int ufs_kirin_suspend(struct ufs_hba *hba, enum ufs_pm_op pm_op)
{
	struct ufs_kirin_host *host = NULL;
	u32 value;

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
	udelay(10); /* wait 10 us */

	/* Step 8 */
	init_completion(&hba->reg_ram_store_done);
	ufshcd_writel(hba, 0x1, UFS_CFG_RAM_CTRL);

	/* Step 13. check if the store is complete */
	if (!wait_for_completion_timeout(
		    &hba->reg_ram_store_done,
		    msecs_to_jiffies(COMPLETION_TIMEOUT_MSEC))) {
		dev_err(hba->dev, "UFS registers saving is not finished\n");
		return -1;
	}

	/* Step 14. clear bit0 of ufs_cfg_ram_ctrl */
	value = ufshcd_readl(hba, UFS_CFG_RAM_CTRL);
	value &= (~0x01);
	ufshcd_writel(hba, value, UFS_CFG_RAM_CTRL);

	/* Step 15. enable MPHY ISO */
	mphy_iso_enable(hba);

	/*
	 * Step 16. enable NOC powerdomain to low power mode
	 * set bit1 to 1 with mask bit
	 */
	writel(0x00020002, SOC_PMCTRL_NOC_POWER_IDLEREQ_0_ADDR(host->pmctrl));
	ufs_suspend_polling_dmc(host, hba);

	/* Step 17. enable ip_rst_ufs_subsys, reset ufs sub-system */
	sys_ctrl_set_bits(host, BIT(IP_RST_UFS_SUBSYS), SOC_SCTRL_SCPERRSTEN1);

	/* Step 18. disable input clk of UFS sub-system */
	sys_ctrl_set_bits(host, GT_CLK_UFS_SUBSYS, SOC_SCTRL_SCPERDIS4);

	/* Step 19. enable reset of UFS sub-system CRG */
	sys_ctrl_set_bits(host, BIT(IP_RST_UFS_SUBSYS_CRG),
			  SOC_SCTRL_SCPERRSTEN1);

	/* Step 20. enable RET of dual-rail RAM, set DS =1 , SD = 0 */
	writel(0x00030002, SOC_ACTRL_AO_MEM_CTRL2_ADDR(host->actrl));

	hisi_pmic_reg_write(PMIC_CLK_UFS_EN_ADDR(0), 0);

	host->in_suspend = true;

	return 0;
}

static void ufs_sub_sys_crg_reset_release(struct ufs_hba *hba)
{
	u32 value = 0;
	int retry = POLLING_RETRY;
	struct ufs_kirin_host *host = hba->priv;

	sys_ctrl_set_bits(host, BIT(IP_RST_UFS_SUBSYS_CRG),
			  SOC_SCTRL_SCPERRSTDIS1);
	while (retry--) {
		value = readl(SOC_SCTRL_SCPERRSTSTAT1_ADDR(host->sysctrl));
		if (!(value & BIT(IP_RST_UFS_SUBSYS_CRG)))
			return;
		udelay(1);
	}
	dev_err(hba->dev, "UFS Sub-sys CRG reset failed\n");
}

static int wait_mphy_pll_lock(struct ufs_hba *hba)
{
	int retry = POLLING_RETRY_4W;
	u32 value = 0;
	struct ufs_kirin_host *host = hba->priv;

	if (host->caps & USE_HISI_MPHY_TC)
		return 0;

	while (retry--) {
		value = readl(
			SOC_UFS_Sysctrl_HIUFS_DEBUG_ADDR(host->ufs_sys_ctrl));
		if (value & MPHY_PLL_LOCK)
			return 0;
		udelay(1);
	}
	dev_err(hba->dev, "MPHY PLL Lock err\n");

	return -1;
}

static void ufs_polling_pmc_idle_resume(struct ufs_hba *hba)
{
	u32 value = 0;
	int retry = POLLING_RETRY;
	struct ufs_kirin_host *host = hba->priv;

	/* Polling the PMC idle ack0 register */
	while (retry--) {
		value = readl(
			SOC_PMCTRL_NOC_POWER_IDLEACK_0_ADDR(host->pmctrl));
		if (!(value & NOC_UFS_POWER_IDLEACK))
			break;
		udelay(1);
	}
	if (retry < 0)
		dev_err(hba->dev, "UFS Power idle exit failed\n");

	/* Polling the PMC idle status */
	retry = POLLING_RETRY;
	while (retry--) {
		value = readl(SOC_PMCTRL_NOC_POWER_IDLE_0_ADDR(host->pmctrl));
		if (!(value & NOC_UFS_POWER_IDLE))
			break;
		udelay(1);
	}
	if (retry < 0)
		dev_err(hba->dev, "UFS Power idle exit status err\n");
}

static int ufs_restore_register_and_check(struct ufs_hba *hba)
{
	init_completion(&hba->reg_ram_restore_done);
	ufshcd_writel(hba, UFS_CONFIG_RAM_ENABLE, UFS_CFG_RAM_CTRL);

	if (!wait_for_completion_timeout(
		    &hba->reg_ram_restore_done,
		    msecs_to_jiffies(COMPLETION_TIMEOUT_MSEC))) {
		dev_err(hba->dev, "UFS registers restore is not finished\n");
		return -1;
	}

	return 0;
}

int ufs_kirin_resume(struct ufs_hba *hba, enum ufs_pm_op pm_op)
{
	int ret;
	u32 value;
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

	/* Step 1. release the reset of UFS subsys CRG */
	ufs_sub_sys_crg_reset_release(hba);

	/* Step 2. enable the clk of UFS side of NOC async bridge */
	hsdt_crg_set_bits(host, BIT(GT_CLK_UFS_NOC_ASYNCBRG),
			  SOC_HSDT_CRG_PEREN1);
	/* release the reset of UFS side of NOC async bridge */
	hsdt_crg_set_bits(host, BIT(IP_RST_UFS_NOC_ASYNCBRG),
			  SOC_HSDT_CRG_PERRSTDIS0);

	/* Step 4. disable RET of dual-rail RAM, DS = 0, SD = 0 */
	writel(0x00030000, SOC_ACTRL_AO_MEM_CTRL2_ADDR(host->actrl));

	/* Step 5. Reset UFS_SUBSYS */
	sys_ctrl_set_bits(host, BIT(IP_RST_UFS_SUBSYS), SOC_SCTRL_SCPERRSTEN1);

	/* Step 6. setup ufs sub-system clk to 224MHz */
	writel(0x003F0006, SOC_SCTRL_SCCLKDIV9_ADDR(host->sysctrl));
	udelay(1);

	/* Step 7. Release the reset of UFS_SUBSYS */
	sys_ctrl_set_bits(host, BIT(IP_RST_UFS_SUBSYS), SOC_SCTRL_SCPERRSTDIS1);

	/* Step 8. Enable clk_ufs_subsys clock */
	writel(BIT(SOC_SCTRL_SCPEREN4_gt_clk_ufs_subsys_START),
	       SOC_SCTRL_SCPEREN4_ADDR(host->sysctrl));

	/* Step 9. handshake of UFS powerdomain */
	writel(NOC_UFS_POWER_IDLEREQ_MASK,
	       SOC_PMCTRL_NOC_POWER_IDLEREQ_0_ADDR(host->pmctrl));
	ufs_polling_pmc_idle_resume(hba);

	/* Step 10. enable UFS CFG clk */
	writel(0x00080008,
	       SOC_UFS_Sysctrl_CRG_UFS_CFG1_ADDR(host->ufs_sys_ctrl));

	/* Step 11. release internal release of UFS Subsys */
	writel(0x01FF01FF,
	       SOC_UFS_Sysctrl_UFS_RESET_CTRL_ADDR(host->ufs_sys_ctrl));

	/* Step 12. disable UFS CFG clk */
	writel(0x00080000,
	       SOC_UFS_Sysctrl_CRG_UFS_CFG1_ADDR(host->ufs_sys_ctrl));

	/* Step 13. disable MPHY ISO */
	mphy_iso_disable(hba);

	/* Step 14. enable UFS CFG clk */
	writel(0x00080008,
	       SOC_UFS_Sysctrl_CRG_UFS_CFG1_ADDR(host->ufs_sys_ctrl));

	/* Step 15. wait for UFS MPHY PLL lock */
	udelay(MPHY_PLL_LOCK_WAIT_TIME);

	/*
	 * Step 1. and Step 5.
	 * request UFSHCI and MPHY to restore register from dual-rail RAM and
	 * check if the restore is complete
	 */
	ret = ufs_restore_register_and_check(hba);
	if (ret)
		return ret;

	/* Step 6. */
	value = ufshcd_readl(hba, UFS_CFG_RAM_CTRL);
	value &= (~0x02);
	ufshcd_writel(hba, value, UFS_CFG_RAM_CTRL);

	/* Step 8. */
	hisi_uic_write_reg(hba, HIBERNATE_EXIT_MODE, 0x1);

	host->in_suspend = false;

	return 0;
}

static void ufs_kirin_device_hw_reset_hybrid(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = hba->priv;

	if (likely(!(host->caps & USE_HISI_MPHY_TC))) {
		ufs_sys_ctrl_writel(host, MASK_UFS_DEVICE_RESET | 0,
				    UFS_DEVICE_RESET_CTRL);
		mdelay(1);
	} else if (!is_v200_mphy(hba)) {
		ufs_i2c_writel(hba, (unsigned int)UFS_BIT_DEVICE_RSTDIS,
			       SC_RSTDIS);
		mdelay(1);
	} else {
		ufs_i2c_writel(hba, 0x20000, SC_RSTEN_V200);
		mdelay(2); /* wait 2 ms */
	}

	if (likely(!(host->caps & USE_HISI_MPHY_TC))) {
		ufs_sys_ctrl_writel(host, MASK_UFS_DEVICE_RESET |
						  BIT_UFS_DEVICE_RESET,
				    UFS_DEVICE_RESET_CTRL);
	} else if (!is_v200_mphy(hba)) {
		ufs_i2c_writel(hba, (unsigned int)UFS_BIT_DEVICE_RSTEN,
			       SC_RSTEN);
	} else {
		ufs_i2c_writel(hba, 0x20000, SC_RSTDIS_V200);
	}
	mdelay(10); /* wait 10 ms */
}

void ufs_kirin_device_hw_reset(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = NULL;

	host = hba->priv;

	if (!ufshcd_is_hisi_ufs_hc(hba))
		return ufs_kirin_device_hw_reset_hybrid(hba);

	if (likely(!(host->caps & USE_HISI_MPHY_TC))) {
		/*
		 * Enable device reset, bit[10] is rst_device_rst_n
		 * bit[26] is bitmask
		 */
		writel(0x04000000,
		       SOC_ACTRL_BITMSK_NONSEC_CTRL1_ADDR((host->actrl)));
		ufshcd_vops_vcc_power_on_off(hba);
		/* wait 2 ms */
		mdelay(2);
		/* Disable device reset */
		writel(0x04000400,
		       SOC_ACTRL_BITMSK_NONSEC_CTRL1_ADDR((host->actrl)));
	} else {
		if (!is_v200_mphy(hba)) {
			ufs_i2c_writel(hba, (unsigned int)UFS_BIT_DEVICE_RSTDIS,
				       SC_RSTDIS);
			mdelay(1);
			ufs_i2c_writel(hba, (unsigned int)UFS_BIT_DEVICE_RSTEN,
				       SC_RSTEN);
		} else {
			ufs_i2c_writel(hba, 0x20000, SC_RSTEN_V200);
			mdelay(2); /* wait 2 ms */
			ufs_i2c_writel(hba, 0x20000, SC_RSTDIS_V200);
		}
	}

	mdelay(10); /* wait 10 ms */
}

int ufs_kirin_link_startup_pre_change(struct ufs_hba *hba)
{
	int err = 0;
	struct ufs_attr_cfg *cfg = NULL;
	struct ufs_kirin_host *host = NULL;

	host = hba->priv;

	dev_info(hba->dev, "%s ++\n", __func__);

	/* for FPGA hisi MPHY */
	if ((host->caps & USE_HISI_MPHY_TC)) {
		if (is_v200_mphy(hba)) {
			cfg = hisi_mphy_v200_pre_link_attr;
			hisi_set_each_cfg_attr(hba, cfg);
			mdelay(40); /* wait 40 ms */
		} else {
			cfg = hisi_mphy_v120_pre_link_attr;
			hisi_set_each_cfg_attr(hba, cfg);
		}
	} else {
		/* for ASIC hisi MPHY and emu */
		cfg = hisi_mphy_v300_pre_link_attr;
		hisi_set_each_cfg_attr(hba, cfg);
		err = wait_mphy_init_done(hba);
		if (err)
			return err;
	}

	/* for hisi MPHY */
	if ((host->caps & USE_HISI_MPHY_TC)) {
		if (is_v200_mphy(hba))
			hisi_mphy_v200_updata_vswing_fsm(hba, host);
		else
			hisi_mphy_updata_vswing_fsm_ocs5(hba, host);

		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)PA_ADAPTAFTERLRSTINPA_INIT, 0x0), 0x1);

		pr_info("%s --\n", __func__);

		return err;
	}
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
			/* RX_MC_PRESENT */
			ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00c2, 0x5), 0x0);
	}
}

void set_device_clk(struct ufs_hba *hba)
{
}

static void ufs_hcd_dme_set_value(struct ufs_hba *hba)
{
	u32 val;

	ufshcd_dme_get(hba, UIC_ARG_MIB(PA_TXHSG1SYNCLENGTH), &val);
	/* sync length devided into four step 4B 4C 4D 4E */
	if (val < 0x4B)
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_TXHSG1SYNCLENGTH, 0x0), 0x4B);

	ufshcd_dme_get(hba, UIC_ARG_MIB(PA_TXHSG2SYNCLENGTH), &val);
	if (val < 0x4C)
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_TXHSG2SYNCLENGTH, 0x0), 0x4C);

	ufshcd_dme_get(hba, UIC_ARG_MIB(PA_TXHSG3SYNCLENGTH), &val);
	if (val < 0x4D)
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_TXHSG3SYNCLENGTH, 0x0), 0x4D);

	ufshcd_dme_get(hba, UIC_ARG_MIB(PA_TXHSG4SYNCLENGTH), &val);
	if (val < 0x4E)
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_TXHSG4SYNCLENGTH, 0x0), 0x4E);
}

int ufs_kirin_link_startup_post_change(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = NULL;
	struct ufs_attr_cfg *cfg = NULL;
	u32 value, value_bak;

	host = hba->priv;

	pr_info("%s ++\n", __func__);

	/* for hisi asic mphy and emu, use USE_HISI_MPHY_ASIC on ASIC later */
	if (!(host->caps & USE_HISI_MPHY_TC) || hba->host->is_emulator) {
		cfg = hisi_mphy_v300_post_link_attr;
		hisi_set_each_cfg_attr(hba, cfg);
		hisi_uic_read_reg(hba, PA_TACTIVATE, &value);
		/* add 0x3 * 100us time compensation for tactivate */
		hisi_uic_write_reg(hba, PA_TACTIVATE, value + 0x3);
	}

	if ((host->caps & USE_HISI_MPHY_TC)) {
		if (is_v200_mphy(hba)) {
			hisi_mphy_v200_link_post_config(hba, host);
			cfg = hisi_mphy_v200_post_link_attr;
			hisi_set_each_cfg_attr(hba, cfg);

			ufs_hcd_dme_set_value(hba);

			ufshcd_dme_get(hba, UIC_ARG_MIB_SEL(0x00ba, 0x0),
				       &value_bak);
			ufshcd_dme_get(hba, UIC_ARG_MIB_SEL(0x00ae, 0x0),
				       &value);
			ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00ae, 0x0),
				       value | MPHY_BIT_REG_VCO_SW_VCAL);
			ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00ba, 0x0),
				       value_bak);
		} else {
			hisi_mphy_link_post_config(hba, host);
			cfg = hisi_mphy_v120_post_link_attr;
			hisi_set_each_cfg_attr(hba, cfg);
		}
	}

	if (host->caps & BROKEN_CLK_GATE_BYPASS) {
#ifndef CONFIG_HISI_UFS_HC
		/* not bypass ufs clk gate */
		ufs_sys_ctrl_clr_bits(host, MASK_UFS_CLK_GATE_BYPASS,
				      CLOCK_GATE_BYPASS);
		ufs_sys_ctrl_clr_bits(host, MASK_UFS_SYSCRTL_BYPASS,
				      UFS_SYSCTRL);
#endif
	}

	/* select received symbol cnt */
	ufshcd_dme_set(hba, UIC_ARG_MIB(0xd09a), 0x80000000);
	/* reset counter0 and enable */
	ufshcd_dme_set(hba, UIC_ARG_MIB(0xd09c), 0x00000005);

	pr_info("%s --\n", __func__);

	return 0;
}

/**
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

/* compatible with old platform */
void ufs_kirin_pwr_change_pre_change(
	struct ufs_hba *hba, struct ufs_pa_layer_attr *dev_req_params)
{
	u32 value;
	struct ufs_kirin_host *host = NULL;

	host = hba->priv;

	if (ufshcd_is_hisi_ufs_hc(hba))
		return ufs_hisi_kirin_pwr_change_pre_change(hba,
							    dev_req_params);

	pr_info("%s ++\n", __func__);
#ifdef CONFIG_HISI_DEBUG_FS
	pr_info("device manufacturer_id is 0x%x\n", hba->manufacturer_id);
#endif
	/*
	 * ARIES platform need to set SaveConfigTime to 0x13, and change sync
	 * length to maximum value
	 */
	/* VS_DebugSaveConfigTime */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)0xD0A0), 0x13);
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)PA_TXHSG1SYNCLENGTH),
		       0x4f); /* g1 sync length */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)PA_TXHSG2SYNCLENGTH),
		       0x4f); /* g2 sync length */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)PA_TXHSG3SYNCLENGTH),
		       0x4f); /* g3 sync length */

	ufshcd_dme_get(hba, UIC_ARG_MIB(PA_HIBERN8TIME), &value);
	if (value < 0xA)
		/* PA_Hibern8Time */
		ufshcd_dme_set(hba, UIC_ARG_MIB((u32)PA_HIBERN8TIME), 0xA);
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
