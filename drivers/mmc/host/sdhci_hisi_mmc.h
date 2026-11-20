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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __SDHCI_HISI_SCORPIO_H
#define __SDHCI_HISI_SCORPIO_H

/* hsdt1 sd reset */
#define PERRSTEN0	0x20
#define PERRSTDIS0	0x24
#define IP_RST_SD	(0x1 << 1)
#define IP_HRST_SD	(0x1 << 0)

#define PERRSTEN1 0x2C
#define PERRSTDIS1 0x30
#define PERRSTSTAT1 0x34

#define IP_HRST_SDIO (0x1 << 2)
#define IP_RST_SDIO (0x1 << 3)

#define CLKDIV1	0xAC
#define SDCLK_3_2 0x0
#define PPLL2	0x1
#define SDPLL	0x2
#define MASK_PLL	(0x3 << 5)
#define MASK_DIV_SDPLL	(0xF << 1)

#define PEREN0	0x0
#define PERDIS0	0x4
#define PERSTAT0	0xC
#define GT_HCLK_SD	0x1

#define PEREN1	0x10
#define PERDIS1	0x14
#define PERSTAT1	0x1C

#define SDEMMC_CRG_CTRL	0x80
#define TUNING_SAMPLE_MASK	(0x1F << 8)
#define TUNING_SAMPLE_MASKBIT	0x8

#define CLK_CTRL_TIMEOUT_SHIFT 16
#define CLK_CTRL_TIMEOUT_MASK (0xf << CLK_CTRL_TIMEOUT_SHIFT)
#define CLK_CTRL_TIMEOUT_MIN_EXP 13

#define SDHCI_HISI_MIN_FREQ 100000
#define SDHCI_HISI_CLK_FREQ 200000000
#define MUX_SDSIM_VCC_STATUS_2_9_5_V 0
#define MUX_SDSIM_VCC_STATUS_1_8_0_V 1
#define LEVER_SHIFT_GPIO  90
#define LEVER_SHIFT_3_0V 0
#define LEVER_SHIFT_1_8V 1


#define TUNING_PHASE_INVALID (-1)
#define TUNING_LOOP_COUNT 10

#define BLKSZ_TUNING 64
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

/**
 * struct sdhci_hisi_data
 * @clk:						Pointer to the sd clock
 * @clock:					record current clock rate
 * @tuning_current_sample:		record current sample when soft tuning
 * @tuning_init_sample:			record the optimum sample of soft tuning
 * @tuning_sample_count:		record the tuning count for soft tuning
 * @tuning_move_sample:		record the move sample when data or cmd error occor
 * @tuning_move_count:		record the move count
 * @tuning_sample_flag:			record the sample OK or NOT of soft tuning
 * @tuning_strobe_init_sample:	default strobe sample
 * @tuning_strobe_move_sample:	record the strobe move sample when data or cmd error occor
 * @tuning_strobe_move_count:	record the strobe move count
 */
struct sdhci_hisi_data {
	struct sdhci_host *host;
	struct clk *ciu_clk;
	struct clk *biu_clk;
	unsigned int clock;
	/* pinctrl handles */
	struct pinctrl		*pinctrl;
	struct pinctrl_state	*pins_default;
	struct pinctrl_state	*pins_idle;
	struct workqueue_struct	*card_workqueue;
	struct work_struct	card_work;
	struct device	*dev;

	int tuning_loop_max;
	unsigned int tuning_count;
	int tuning_phase_best;
	int  tuning_phase_record;

	int tuning_move_phase;
	int tuning_move_count;
	int tuning_phase_max;
	int tuning_phase_min;

	int old_power_mode;
	int mux_sdsim;
	int mux_vcc_status;
	unsigned int	lever_shift;

	int sdhci_host_id;
#define SDHCI_EMMC_ID	0
#define SDHCI_SD_ID		1
#define SDHCI_SDIO_ID	2
	unsigned int	flags;
#define PINS_DETECT_ENABLE	(1 << 0)	/* NM card 4-pin detect control */
	int card_detect;
#define CARD_REMOVED 0
#define CARD_ON 1
#define CARD_UNDETECT 2
	unsigned int cd_vol;
	int gpio_cd;
};

extern int check_himntn(int feature);
extern void sdhci_dumpregs(struct sdhci_host *host);
extern int config_sdsim_gpio_mode(enum sdsim_gpio_mode gpio_mode);
extern void sdhci_set_default_irqs(struct sdhci_host *host);

void sdhci_hisi_hardware_reset(struct sdhci_host *host);
void sdhci_hisi_hardware_disreset(struct sdhci_host *host);
int sdhci_hisi_get_resource(struct platform_device *pdev, struct sdhci_host *host);

#define sdhci_mmc_sys_writel(host, val, reg)                                   \
	writel((val), (host)->mmc_sys + (reg))
#define sdhci_mmc_sys_readl(host, reg) readl((host)->mmc_sys + (reg))

#define sdhci_hsdt1crg_writel(host, val, reg)                                   \
	writel((val), (host)->hsdt1crg + (reg))
#define sdhci_hsdt1crg_readl(host, reg) readl((host)->hsdt1crg + (reg))


#endif
