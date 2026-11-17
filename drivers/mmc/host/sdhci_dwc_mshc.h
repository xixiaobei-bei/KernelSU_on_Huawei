/*
 * hisi emmc sdhci controller interface.
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

#ifndef __SDHCI_DWC_MSHC_H
#define __SDHCI_DWC_MSHC_H

#include <linux/mmc/core.h>
#include <linux/workqueue.h>
#include <linux/device.h>
#include <linux/clk.h>
#include <linux/spinlock_types.h>

#include "sdhci.h"

/* sctrl register */
#define SCTRL_SCCTRL 0x0
#define SCTRL_CLKDIV9 0x274
#define SCTRL_AUTODIV 0x278
#define SCTRL_SCPEREN2 0x190
#define SCTRL_SCPERDIS2 0x194
#define SCTRL_SCPERCLKEN2 0x198
#define SCTRL_SCPERSTAT2 0x19C
#define SCTRL_SCPERRSTEN0 0x200
#define SCTRL_SCPERRSTDIS0 0x204
#define SCTRL_SCPERRSTSTAT0 0x208
#define IP_RST_EMMC (0x1 << 21)
#define IP_RST_EMMC_BASE (0x1 << 22)
/* capricorn add define start */
#define SCTRL_SCPERRSTEN1 0x20C
#define SCTRL_SCPERRSTDIS1 0x210
#define SCTRL_SCPERRSTSTAT1 0x214
#define SCTRL_SCCLKDIV10 0x280
#define SCTRL_SCPERCTRL7 0x31C
#define IP_RST_EMMC_ORLAN (0x1 << 15)
#define IP_DISRST_EMMC_ORLAN (0x1 << 15)
#define RG_EMMC_LHEN_IN_RET (0x1 << 1)
#define RG_EMMC_LHEN_IN_NOR (0x0 << 1)
#define RG_EMMC_LHEN_MASK (0x1 << 17)
#define GT_CLK_EMMC_SCTRL (0x1 << 8)
#define GT_CLK_EMMC_BUS (0x1 << 9)
#define SEL_CLK_EMMC_PPLL23 (0x1 << 10)
#define SEL_CLK_PPLL23_MASK (SEL_CLK_EMMC_PPLL23 << 16)

/* emmc1 sysctrl register */
#define MMC1_SYSCTRL_CTRL0 0x008
#define MMC1_SYCTRLS_CTRL1 0x00C
#define MMC1_SYSCTRL_PEREN0 0x300
#define MMC1_SYSCTRL_PERDIS0 0x304
#define MMC1_SYSCTRL_PERCLKEN0 0x308
#define MMC1_SYSCTRL_PERRSTEN0 0x310
#define MMC1_SYSCTRL_PERRSTDIS0 0x314
#define MMC1_SYSCTRL_PERRSTSTAT0 0x318
#define CMD_DLY_EN (0x1U << 31)
#define IP_PRST_EMMCPHY (0x1 << 4)

#define GT_HCLK_EMMC_OR (0x1 << 1)
#define GT_PCLK_EMMC_OR (0x1 << 2)
#define GT_TCLK_EMMC_OR (0x1 << 3)
#define GT_TCLK_EMMC_CQE_OR (0x1 << 5)

#define GT_CLK_EMMC_OR (GT_HCLK_EMMC_OR | GT_PCLK_EMMC_OR |	\
			GT_TCLK_EMMC_OR | GT_TCLK_EMMC_CQE_OR)
/* capricorn define end */

/* eMMC sysctrl register */
#define EMMC_SYS_CTRL0 0x0
#define GT_STROBE_RX_CLK 0x1
#define EMMC_SYS_CTRL1 0x4
#define CRESETN_RX 0x1
#define TUNING_SEL (0x1 << 8)
#define TUNING_EXTEND (0x1 << 9)

#define EMMC_SYS_MTCMOS_EN 0x8
#define PHY_MTCMOS_EN 0x1
#define CTRL_MTCMOS_EN (0x1 << 1)
#define EMMC_SYS_MTCMOS_ACK 0xC
#define PHY_MTCMOS_ACK 0x1
#define CTRL_MTCMOS_ACK (0x1 << 1)

#define EMMC_SYS_LHEN_IN 0x18
#define IN_IO_LATCH_MODE 0x1
#define EMMC_LHEN_IN 0x3F
#define EMMC_SYS_LHEN_INB 0x1C
#define INB_IO_LATCH_MODE 0x1
#define EMMC_LHEN_INB 0x3F

#define EMMC_SYS_PHY_ISOEN 0x20
#define CTRL_ISO_EN 0x1
#define PHY_ISO_EN (0x1 << 1)

#define EMMC_SYS_CRG_CFG1 0x2C
#define GT_HCLK_EMMC (0x1 << 1)
#define GT_PCLK_EMMC_PHY (0x1 << 3)
#define GT_TCLK_EMMC (0x1 << 4)
#define GT_TCLK_EMMC_CQE (0x1 << 5)
#define IP_HRST_EMMC (0x1 << 6)
#define IP_PRST_EMMC_PHY (0x1 << 8)
#define IP_TRST_EMMC (0x1 << 9)
#define IP_TRST_EMMC_CQE (0x1 << 10)
#define EMMC_SYS_GT_CLK (GT_HCLK_EMMC | GT_PCLK_EMMC_PHY | GT_TCLK_EMMC_CQE)
#define EMMC_SYS_GT_CLK_MASK (EMMC_SYS_GT_CLK << 16)
#define EMMC_SYS_IP_RST (IP_HRST_EMMC | IP_PRST_EMMC_PHY | IP_TRST_EMMC_CQE)
#define EMMC_SYS_IP_RST_MASK (EMMC_SYS_IP_RST << 16)
#define IP_PRST_EMMC_PHY_MASK (IP_PRST_EMMC_PHY << 16)

#define SDHCI_VENDOR_SPECIFIC_AREA 0xE8
/* the offset is the value of SDHCI_VENDOR_SPECIFIC_AREA */
#define SDHCI_VENDOR_SPECIFIC_AREA_OFFSET 0x500

#define SDHCI_MSHC_CTRL_R (SDHCI_VENDOR_SPECIFIC_AREA_OFFSET + 0x8)
#define CMD_CONFLICT_CHECK 0x1
#define SW_CG_DIS (0x1 << 4)
#define ACCESS_ALL_REGION (0x1 << 5)
#define SDHCI_MBIU_CTRL (SDHCI_VENDOR_SPECIFIC_AREA_OFFSET + 0x10)
#define WRITE_OSRC_LMT_MASK (0x7 << 24)
#define WRITE_OSRC_LMT (0x3 << 24)
#define SDHCI_DEBUG_PORT1 (SDHCI_VENDOR_SPECIFIC_AREA_OFFSET + 0x20)
#define SDHCI_DEBUG_PORT2 (SDHCI_VENDOR_SPECIFIC_AREA_OFFSET + 0x24)
#define SDHCI_EMMC_CTRL_R (SDHCI_VENDOR_SPECIFIC_AREA_OFFSET + 0x2c)
#define CARD_IS_EMMC 0x1
#define ENH_STROBE_ENABLE (0x1 << 8)
#define SDHCI_AT_CTRL_R (SDHCI_VENDOR_SPECIFIC_AREA_OFFSET + 0x40)
#define SW_TUNE_EN (0x1 << 4)
#define SDHCI_AT_STAT_R (SDHCI_VENDOR_SPECIFIC_AREA_OFFSET + 0x44)
#define CENTER_PH_CODE_MASK 0xFF

#define SDHCI_VENDOR2_SPECIFIC_AREA 0xEA

/* combo phy register */
#define WAIT_STATUS_MAX_RETRY 10
#define COMBO_PHY_PHYINITCTRL 0x4
#define INIT_EN 0x1
#define DLYMEAS_EN (0x1 << 2)
#define ZCAL_EN (0x1 << 3)
#define BIST_CLK (0x1 << 17)
#define COMBO_PHY_PHYINITSTAT 0x8
/* This register specify the control the ZQ calibration */
#define COMBO_PHY_IMPCTRL 0x24
#define IMPCTRL_ZCOM_NUM_SHIFT 12
#define IMPCTRL_ZCOM_NUM_MASK 0xF
#define IMPCTRL_ZCOM_RSP_DLY_MASK 0x3F

/* This register specify the ZQ calibration result */
#define COMBO_PHY_IMPSTATUS 0x28
#define IMPSTATUS_PDRV_SHIFT 16
#define IMPSTATUS_DRV_MASK 0x3F

#define COMBO_PHY_CMD_DLY_CTRL 0x230
#define COMBO_PHY_DATENA_DLY 0x23C
#define COMBO_PHY_PHYCTRL2 0x248
#define PHY_DLYMEAS_UPDATE (0x1 << 14)
#define COMBO_PHY_DLY_CTL 0x250
#define DLY_CPDE_1T_MASK 0x7FF
#define DLY_CODE_1T_MASK 0x7FF

#define COMBO_PHY_DLY_CTL1 0x254
#define DLY_3_CODE_MASK 0x3FF
#define DLY_3_CODE_SHIFT 8
#define DLY_1_CODE_MASK 0x3FF
#define DLY_1_CODE_SHIFT 21
#define INV_TX_CLK (1UL << 31)

#define COMBO_PHY_DLY_CTL2 0x258
#define COMBO_PHY_IOCTL_PUPD 0x260
#define PUPD_EN_DATA 0xFF
#define PUPD_EN_STROBE 0x100
#define PUPD_EN_CMD 0x200
#define PULLUP_DATA (0xFF << 16)
#define PULLUP_CMD (0x200 << 16)

#define COMBO_PHY_IOCTL_RONSEL_1 0x264
#define EMMC_RONSEL_0 0x7FF
#define EMMC_RONSEL_1 (0x7FF << 16)
#define EMMC_RONSEL_OR 0x7FF
#define COMBO_PHY_IOCTL_RONSEL_2 0x268
#define EMMC_RONSEL_2 0x7FF
#define COMBO_PHY_IOCTL_IOE 0x26c
#define DA_EMMC_E 0x400
#define DA_EMMC_IE (0x7FF << 16)

#define COMBO_PHY_IOCTL_TO 0x27c
#define AD_EMMC_I (0x7FF << 16)

/* USE COMBO PHY testchip */
#define TC_COMBO_PHY_REG_BASE 0x21000
#define TC_SC_APB_IF 0x23000
#define SC_EMMC_RSTEN (TC_SC_APB_IF + 0x3C)
#define SC_EMMC_RSTDIS (TC_SC_APB_IF + 0x40)
#define SC_EMMC_RSTSTAT (TC_SC_APB_IF + 0x44)
#define IP_RST_PHY_APB (0x1 << 4)

#define CLK_CTRL_TIMEOUT_SHIFT 16
#define CLK_CTRL_TIMEOUT_MASK (0xf << CLK_CTRL_TIMEOUT_SHIFT)
#define CLK_CTRL_TIMEOUT_MIN_EXP 13

#define SDHCI_MSHC_MIN_FREQ 400000
#define SDHCI_MSHC_XIN_CLK_FREQ 200000000

#define TUNING_PHASE_INVALID (-1)

#define MMC_I2C_SLAVE 0x5A
#define MMC_I2C_BUS 3
#define SDHCI_MAX_COUNT 0xFFF
#define SDHCI_DMD_ERR_MASK 0xffffffff
#define SDHCI_DSM_ERR_INT_STATUS 16
#define SDHCI_ERR_CMD_INDEX_MASK 0x3f
#define SDHCI_SAM_PHASE_MID 2
#define SDHCI_TUNING_MOVE_STEP 2
#define SDHCI_TUNING_RECORD 0x1
#define SDHCI_TUNING_MAX 63
#define CMDQ_SEND_STATUS_CQSSC1 0x10100
#define SDHCI_ADMA_SUPPORT_BIT 64
#define SDHCI_SDMA_SUPPORT_BIT 32
#define MMC_BLOCK_SIZE 512
#define SDHCI_PINCTL_CLEAR_BIT (0xF << 4)
#define SDHCI_WRITE_PINCTL_FLAG (0x7 << 4)
#define MMC_I2C_ADAPTER_NUM 2
#define SDHCI_I2C_MASTER_LENTH 8

/*
 * struct sdhci_mshc_data
 * @clk:                        Pointer to the sd clock
 * @clock:                      record current clock rate
 * @tuning_current_sample:      record current sample when soft tuning
 * @tuning_init_sample:         record the optimum sample of soft tuning
 * @tuning_sample_count:        record the tuning count for soft tuning
 * @tuning_move_sample:         record the move sample when data or cmd error occor
 * @tuning_move_count:          record the move count
 * @tuning_sample_flag:         record the sample OK or NOT of soft tuning
 * @tuning_strobe_init_sample:  default strobe sample
 * @tuning_strobe_move_sample:  record the strobe move sample
 *                              when data or cmd error occor
 * @tuning_strobe_move_count:   record the strobe move count
 */
struct sdhci_mshc_data {
	struct clk *clk;
	unsigned int clock;

	u32 enhanced_strobe_enabled;

	int tuning_128_flag;
	int tuning_loop_max;

	unsigned int tuning_count;
	int tuning_phase_best;
	u64 tuning_phase_record_low;
	u64 tuning_phase_record_high;

	int tuning_move_phase;
	int tuning_move_count;
	int tuning_phase_max;
	int tuning_phase_min;

	int tuning_strobe_phase_init;
	int tuning_strobe_move_phase;
	int tuning_strobe_move_count;
	int tuning_strobe_phase_max;
	int tuning_strobe_phase_min;

	bool need_delay_measure;
#ifdef CONFIG_HUAWEI_EMMC_DSM
	u64 para;
	u32 cmd_data_status;
	void *data;
	struct work_struct dsm_work;
	spinlock_t sdhci_dsm_lock;
#endif
};

extern void sdhci_dumpregs(struct sdhci_host *host);
extern int sdhci_get_cmd_err(u32 intmask);
extern int sdhci_get_data_err(u32 intmask);

#ifdef CONFIG_HUAWEI_EMMC_DSM
extern void sdhci_dsm_handle(struct sdhci_host *host, struct mmc_request *mrq);
extern void sdhci_dsm_set_host_status(struct sdhci_host *host, u32 error_bits);
#endif

void sdhci_i2c_writel(struct sdhci_host *host, u32 val, u32 addr);
u32 sdhci_i2c_readl(struct sdhci_host *host, u32 addr);
void sdhci_combo_phy_zq_cal(struct sdhci_host *host);
void sdhci_mshc_set_tuning_strobe_phase(struct sdhci_host *host, u32 val);
int sdhci_mshc_runtime_suspend_host(struct sdhci_host *host);
int sdhci_mshc_runtime_resume_host(struct sdhci_host *host);
void sdhci_mshc_resume_then_tuning_move(struct device *dev);

void sdhci_mshc_hardware_reset(struct sdhci_host *host);
void sdhci_combo_phy_init(struct sdhci_host *host);
void sdhci_mshc_config_strobe_clk(struct sdhci_host *host, unsigned int timing);
int sdhci_mshc_suspend(struct device *dev);
int sdhci_mshc_resume(struct device *dev);
void sdhci_mshc_hardware_disreset(struct sdhci_host *host);
int sdhci_mshc_runtime_suspend(struct device *dev);
int sdhci_mshc_runtime_resume(struct device *dev);
int sdhci_coldboot_rdr_regester(void);
void sdhci_dsm_work(struct work_struct *work);

static inline void sdhci_phy_writel(struct sdhci_host *host, u32 val, u32 reg)
{
	if (host->quirks2 & SDHCI_QUIRK2_HISI_COMBO_PHY_TC)
		sdhci_i2c_writel(host, val, TC_COMBO_PHY_REG_BASE + reg);
	else
		writel(val, host->mmc_phy + reg);
}

static inline u32 sdhci_phy_readl(struct sdhci_host *host, u32 reg)
{
	if (host->quirks2 & SDHCI_QUIRK2_HISI_COMBO_PHY_TC)
		return sdhci_i2c_readl(host, TC_COMBO_PHY_REG_BASE + reg);

	return readl(host->mmc_phy + reg);
}

#define sdhci_mmc_sys_writel(host, val, reg)                                   \
	writel((val), (host)->mmc_sys + (reg))
#define sdhci_mmc_sys_readl(host, reg) readl((host)->mmc_sys + (reg))

#endif
