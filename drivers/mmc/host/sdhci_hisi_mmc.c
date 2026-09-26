/*
 * HISI Secure Digital Host Controller Interface.
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
#include <soc_pmctrl_interface.h>
#include <linux/regulator/consumer.h>
#include <linux/hisi/rdr_pub.h>
#include <mntn_subtype_exception.h>
#include <linux/mmc/dsm_emmc.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/sdhci_hisi_mux_sdsim.h>

#include "sdhci-pltfm.h"
#include "sdhci_hisi_mmc.h"

#define DRIVER_NAME "sdhci-hisi"
struct sdhci_host *sdhci_host_from_sd_module;
struct sdhci_host *sdhci_sdio_host;

void sdhci_hisi_dumpregs(struct sdhci_host *host)
{
	dev_info(&host->mmc->class_dev, DRIVER_NAME ": tuning ctrl: 0x%08x\n", sdhci_readl(host, SDEMMC_CRG_CTRL));
}

void sdhci_hisi_dump_clk_reg(void)
{

}
int sdhci_check_himntn(void)
{
#ifdef CONFIG_HISILICON_PLATFORM_MAINTAIN
	return check_himntn(HIMNTN_SD2JTAG) ||
			check_himntn(HIMNTN_SD2DJTAG) ||
			check_himntn(HIMNTN_SD2UART6);
#else
		return 0;
#endif
}

void sdhci_sdio_card_detect(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = NULL;
	struct sdhci_hisi_data *sdhci_hisi = NULL;

	if (!host) {
		dev_err(&host->mmc->class_dev, "sdio detect, host is null,can not used to detect sdio\n");
		return;
	}

	if (host->mmc) {
		dev_err(&host->mmc->class_dev, "sdio_present = %d\n",
			host->mmc->sdio_present);
		host->mmc->sdio_present = 1;
	}

	pltfm_host = sdhci_priv(host);
	sdhci_hisi = pltfm_host->priv;
	queue_work(sdhci_hisi->card_workqueue, &sdhci_hisi->card_work);
}

void sdhci_sdio_card_detect_change(void)
{
	pr_err("sdhci sdio host start detect\n");
	sdhci_sdio_card_detect(sdhci_sdio_host);
}
EXPORT_SYMBOL(sdhci_sdio_card_detect_change);

static void sdhci_hisi_gpio_value_set(int value, int gpio_num)
{
	int err;

	err = gpio_request(gpio_num, "gpio_num");
	if (err < 0) {
		pr_err("Can`t request gpio number %d\n", gpio_num);
		return;
	}

	pr_info("%s mmc gpio num: %d, lever_value: %d\n", __func__, gpio_num, value);
	gpio_direction_output(gpio_num, value);
	gpio_set_value(gpio_num, value);
	gpio_free(gpio_num);
}

static unsigned int sdhci_hisi_get_min_clock(struct sdhci_host *host)
{
	return SDHCI_HISI_MIN_FREQ;
}

static unsigned int sdhci_hisi_get_timeout_clock(struct sdhci_host *host)
{
	u32 div;
	unsigned long freq;
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_hisi_data *sdhci_hisi = pltfm_host->priv;

	div = sdhci_readl(host, SDHCI_CLOCK_CONTROL);
	div = (div & CLK_CTRL_TIMEOUT_MASK) >> CLK_CTRL_TIMEOUT_SHIFT;

	freq = clk_get_rate(sdhci_hisi->ciu_clk);
	freq /= (u32)(1 << (CLK_CTRL_TIMEOUT_MIN_EXP + div));

	dev_info(&host->mmc->class_dev, "%s: freq=%lx\n", __func__, freq);

	return freq;
}

void sdhci_hisi_od_enable(struct sdhci_host *host)
{
	/* configure OD enable */
	pr_debug("%s: end!\n", __func__);
}

int sdhci_disable_open_drain(struct sdhci_host *host)
{
	pr_debug("%s\n", __func__);
	return 0;
}

static void sdhci_hisi_hw_reset(struct sdhci_host *host)
{
	sdhci_hisi_hardware_reset(host);
	sdhci_hisi_hardware_disreset(host);
}

int sdhci_chk_busy_before_send_cmd(struct sdhci_host *host,
	struct mmc_command *cmd)
{
	unsigned long timeout;

	/* We shouldn't wait for busy for stop commands */
	if ((cmd->opcode != MMC_STOP_TRANSMISSION) && (cmd->opcode != MMC_SEND_STATUS) &&
	    (cmd->opcode != MMC_GO_IDLE_STATE)) {
		/* Wait busy max 10 s */
		timeout = 10000;
		while (!(sdhci_readl(host, SDHCI_PRESENT_STATE) & SDHCI_DATA_0_LVL_MASK)) {
			if (timeout == 0) {
				dev_err(&host->mmc->class_dev, "%s: wait busy 10s time out.\n",
						mmc_hostname(host->mmc));
				sdhci_dumpregs(host);
				cmd->error = -ENOMSG;
				tasklet_schedule(&host->finish_tasklet);
				return -ENOMSG;
			}
			timeout--;
			mdelay(1);
		}
	}
	return 0;
}

void sdhci_hisi_set_version(struct sdhci_host *host)
{
	u16 ctrl;

	ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	ctrl |= SDHCI_CTRL_HOST_VER4_ENABLE;
	sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);
}

int sdhci_hisi_enable_dma(struct sdhci_host *host)
{
	u16 ctrl;

	if (host->runtime_suspended)
		return 0;

	ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	ctrl |= SDHCI_CTRL_HOST_VER4_ENABLE;
	if (host->flags & SDHCI_USE_64_BIT_DMA)
		ctrl |= SDHCI_CTRL_64BIT_ADDR;
	sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);

	ctrl = (u16)sdhci_readb(host, SDHCI_HOST_CONTROL);
	ctrl &= ~SDHCI_CTRL_DMA_MASK;
	if (host->flags & SDHCI_USE_ADMA)
		ctrl |= SDHCI_CTRL_ADMA2;
	else
		ctrl |= SDHCI_CTRL_SDMA;
	sdhci_writeb(host, (u8)ctrl, SDHCI_HOST_CONTROL);

	return 0;
}

static void sdhci_hisi_restore_transfer_para(struct sdhci_host *host)
{
	u16 mode;

	sdhci_hisi_enable_dma(host);
	sdhci_set_transfer_irqs(host);

	mode = SDHCI_TRNS_BLK_CNT_EN;
	mode |= SDHCI_TRNS_MULTI;
	if (host->flags & SDHCI_REQ_USE_DMA)
		mode |= SDHCI_TRNS_DMA;
	sdhci_writew(host, mode, SDHCI_TRANSFER_MODE);
	/* Set the DMA boundary value and block size */
	sdhci_writew(host, SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG,
		512), SDHCI_BLOCK_SIZE);
}

void sdhci_hisi_select_card_type(struct sdhci_host *host)
{

}

void sdhci_hisi_set_uhs_signaling(struct sdhci_host *host, unsigned int timing)
{
	u16 ctrl;

	ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	/* Select Bus Speed Mode for host */
	ctrl &= ~SDHCI_CTRL_UHS_MASK;
	if (timing == MMC_TIMING_MMC_HS200)
		ctrl |= SDHCI_CTRL_HS_HS200;
	else if (timing == MMC_TIMING_MMC_HS400)
		ctrl |= SDHCI_CTRL_HS_HS400;
	else if (timing == MMC_TIMING_MMC_HS)
		ctrl |= SDHCI_CTRL_HS_SDR;
	else if (timing == MMC_TIMING_MMC_DDR52)
		ctrl |= SDHCI_CTRL_HS_DDR;

	sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);
}

static void sdhci_hisi_init_tuning_para(struct sdhci_hisi_data *sdhci_hisi)
{
	sdhci_hisi->tuning_count = 0;
	sdhci_hisi->tuning_phase_record = 0;
	sdhci_hisi->tuning_phase_best = TUNING_PHASE_INVALID;

	sdhci_hisi->tuning_move_phase = TUNING_PHASE_INVALID;
	sdhci_hisi->tuning_move_count = 0;
	sdhci_hisi->tuning_phase_max = 10;
	sdhci_hisi->tuning_phase_min = 0;
	sdhci_hisi->tuning_loop_max = TUNING_LOOP_COUNT;

}

static void sdhci_hisi_set_tuning_phase(struct sdhci_host *host, u32 val)
{
	u32 reg_val;

	reg_val = sdhci_readl(host, SDHCI_CLOCK_CONTROL);
	reg_val &= ~SDHCI_CLOCK_INT_EN;
	sdhci_writel(host, reg_val, SDHCI_CLOCK_CONTROL);

	reg_val = sdhci_readl(host, SDEMMC_CRG_CTRL);
	reg_val &= ~TUNING_SAMPLE_MASK;
	reg_val |= (val << TUNING_SAMPLE_MASKBIT);
	sdhci_writel(host, reg_val, SDEMMC_CRG_CTRL);

	reg_val = sdhci_readl(host, SDHCI_CLOCK_CONTROL);
	reg_val |= SDHCI_CLOCK_INT_EN;
	sdhci_writel(host, reg_val, SDHCI_CLOCK_CONTROL);
}

static void sdhci_hisi_save_tuning_phase(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_hisi_data *sdhci_hisi = pltfm_host->priv;

	sdhci_hisi->tuning_phase_record |= 0x1 << (sdhci_hisi->tuning_count);
}

static void sdhci_hisi_add_tuning_phase(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_hisi_data *sdhci_hisi = pltfm_host->priv;

	sdhci_hisi->tuning_count++;

	sdhci_hisi_set_tuning_phase(host, sdhci_hisi->tuning_count);
}

#define cror(data, shift) ((data >> shift) | (data << (10 - shift)))
static int sdhci_hisi_find_best_phase(struct sdhci_host *host)
{
	u64 tuning_phase_record;
	u32 loop = 0;
	u32 max_loop;
	u32 max_continuous_len = 0;
	u32 current_continuous_len = 0;
	u32 max_continuous_start = 0;
	u32 current_continuous_start = 0;
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_hisi_data *sdhci_hisi = pltfm_host->priv;

	max_loop = sdhci_hisi->tuning_loop_max;

	dev_err(&host->mmc->class_dev, "hisi tuning_phase_record: 0x%llx,\n",
									sdhci_hisi->tuning_phase_record);
	tuning_phase_record = sdhci_hisi->tuning_phase_record;
	/* bit0 and bit9 continuous in circles */
	if ((tuning_phase_record & (0x1 << sdhci_hisi->tuning_phase_min)) &&
		(tuning_phase_record & (0x1 << (sdhci_hisi->tuning_phase_max - 1))))
		max_loop = sdhci_hisi->tuning_loop_max * SDHCI_SAM_PHASE_MID;

	do {
		if (tuning_phase_record & 0x1) {
			if (!current_continuous_len)
				current_continuous_start = loop;

			current_continuous_len++;
			if (loop == max_loop - 1) {
				if (current_continuous_len > max_continuous_len) {
					max_continuous_len = current_continuous_len;
					max_continuous_start = current_continuous_start;
				}
			}
		} else {
			if (current_continuous_len) {
				if (current_continuous_len > max_continuous_len) {
					max_continuous_len = current_continuous_len;
					max_continuous_start = current_continuous_start;
				}

				current_continuous_len = 0;
			}
		}

		loop++;
		tuning_phase_record = cror(tuning_phase_record, 1);
	} while (loop < max_loop);

	if (!max_continuous_len)
		return -1;

	sdhci_hisi->tuning_phase_best = max_continuous_start + max_continuous_len / 2;
	dev_err(&host->mmc->class_dev, "%s: soft tuning best phase:0x%x\n",
			dev_name(sdhci_hisi->dev), sdhci_hisi->tuning_phase_best);

	return 0;
}

static int sdhci_hisi_tuning_move_clk(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_hisi_data *sdhci_hisi = pltfm_host->priv;
	int loop = 0;
	int ret = 0;
	int move_step = 2;

	/* soft tuning fail or error then return error */
	if (sdhci_hisi->tuning_phase_best >= sdhci_hisi->tuning_loop_max) {
		dev_err(&host->mmc->class_dev, "%s: soft tuning fail, can not move tuning, tuning_init_sample=%d\n",
				__func__, sdhci_hisi->tuning_phase_best);
		return -1;
	}
	/*
	  * first move tuning, set soft tuning optimum sample. When second or more move
	  * tuning, use the sample near optimum sample
	  */
	for (loop = 0; loop < 2; loop++) {
		sdhci_hisi->tuning_move_count++;
		sdhci_hisi->tuning_move_phase = sdhci_hisi->tuning_phase_best
			+ ((sdhci_hisi->tuning_move_count % 2) ? move_step : -move_step)
			   * (sdhci_hisi->tuning_move_count / 2);

		if ((sdhci_hisi->tuning_move_phase >= sdhci_hisi->tuning_phase_max)
			|| (sdhci_hisi->tuning_move_phase < sdhci_hisi->tuning_phase_min)) {
			continue;
		} else {
			break;
		}
	}

	if ((sdhci_hisi->tuning_move_phase >= sdhci_hisi->tuning_loop_max)
		|| (sdhci_hisi->tuning_move_phase < sdhci_hisi->tuning_phase_min)) {
		dev_err(&host->mmc->class_dev, "%s: tuning move fail.\n", __func__);
		sdhci_hisi->tuning_move_phase = TUNING_PHASE_INVALID;
		ret = -1;
	} else {
		sdhci_hisi_set_tuning_phase(host, sdhci_hisi->tuning_move_phase);
		dev_err(&host->mmc->class_dev, "%s: tuning move to phase=%d\n",
								__func__, sdhci_hisi->tuning_move_phase);
	}

	return ret;
}

static int sdhci_hisi_tuning_move(struct sdhci_host *host, int is_move_strobe, int flag)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_hisi_data *sdhci_hisi = pltfm_host->priv;

	/* set tuning_strobe_move_count to 0, next tuning move will begin from optimum sample */
	if (flag) {
		sdhci_hisi->tuning_move_count = 0;
		return 0;
	}

	if (is_move_strobe) {
		dev_err(&host->mmc->class_dev, "%s sd need not strobe tuning move\n", __func__);
		return 0;
	} else {
		return sdhci_hisi_tuning_move_clk(host);
	}
}

static const u8 tuning_blk_pattern_4bit[] = {
	0xff, 0x0f, 0xff, 0x00, 0xff, 0xcc, 0xc3, 0xcc,
	0xc3, 0x3c, 0xcc, 0xff, 0xfe, 0xff, 0xfe, 0xef,
	0xff, 0xdf, 0xff, 0xdd, 0xff, 0xfb, 0xff, 0xfb,
	0xbf, 0xff, 0x7f, 0xff, 0x77, 0xf7, 0xbd, 0xef,
	0xff, 0xf0, 0xff, 0xf0, 0x0f, 0xfc, 0xcc, 0x3c,
	0xcc, 0x33, 0xcc, 0xcf, 0xff, 0xef, 0xff, 0xee,
	0xff, 0xfd, 0xff, 0xfd, 0xdf, 0xff, 0xbf, 0xff,
	0xbb, 0xff, 0xf7, 0xff, 0xf7, 0x7f, 0x7b, 0xde,
};

static void sdhci_hisi_tuning_before_set(struct sdhci_host *host)
{
	u16 reg_val;

	reg_val = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	reg_val &= ~SDHCI_CTRL_TUNED_CLK;
	sdhci_writew(host, reg_val, SDHCI_HOST_CONTROL2);
}

static void sdhci_hisi_send_tuning(struct sdhci_host *host, u32 opcode,
	u8 *tuning_blk, int blksz, const u8 *tuning_blk_pattern)
{
	struct mmc_request mrq = {NULL};
	struct mmc_command cmd = {0};
	struct mmc_data data = {0};
	struct scatterlist sg;

	cmd.opcode = opcode;
	cmd.arg = 0;
	cmd.flags = MMC_RSP_R1 | MMC_CMD_ADTC;
	cmd.error = 0;

	data.blksz = blksz;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;
	data.sg = &sg;
	data.sg_len = 1;
	data.error = 0;
	data.timeout_ns = 50000000; /* 50ms */

	sg_init_one(&sg, tuning_blk, blksz);

	mrq.cmd = &cmd;
	mrq.data = &data;

	mmc_wait_for_req(host->mmc, &mrq);
	/*
	 * no cmd or data error and tuning data is ok,
	 * then set sample flag
	 */
	if (!cmd.error && !data.error && tuning_blk &&
		(memcmp(tuning_blk, tuning_blk_pattern, sizeof(blksz)) == 0))
		sdhci_hisi_save_tuning_phase(host);
}

static int sdhci_hisi_soft_tuning(struct sdhci_host *host, u32 opcode, bool set)
{
	struct sdhci_hisi_data *sdhci_hisi = ((struct sdhci_pltfm_host *)sdhci_priv(host))->priv;
	int tuning_loop_counter;
	int ret;
	const u8 *tuning_blk_pattern = NULL;
	u8 *tuning_blk = NULL;
	int blksz = 0;
	u32 hw_tuning_phase;

	hw_tuning_phase = TUNING_SAMPLE_MASK & sdhci_readl(host, SDEMMC_CRG_CTRL);
	hw_tuning_phase = hw_tuning_phase >> TUNING_SAMPLE_MASKBIT;
	dev_err(&host->mmc->class_dev, "%s: crg ctrl_reg :0x%x hardware tuning value: 0x%x\n",
							dev_name(sdhci_hisi->dev),
							sdhci_readl(host, SDEMMC_CRG_CTRL), hw_tuning_phase);

	sdhci_hisi_init_tuning_para(sdhci_hisi);
	tuning_loop_counter = sdhci_hisi->tuning_loop_max;

	host->flags |= SDHCI_EXE_SOFT_TUNING;
	dev_err(&host->mmc->class_dev, "%s: %s: now, start tuning soft...\n",
								dev_name(sdhci_hisi->dev), __func__);

	tuning_blk_pattern = tuning_blk_pattern_4bit;
	blksz = BLKSZ_TUNING;
	tuning_blk = kzalloc(blksz, GFP_KERNEL);
	if (!tuning_blk) {
		ret = -ENOMEM;
		goto err;
	}

	sdhci_hisi_tuning_before_set(host);
	sdhci_hisi_set_tuning_phase(host, 0);

	while (tuning_loop_counter--) {
		sdhci_hisi_send_tuning(host, opcode, tuning_blk, blksz, tuning_blk_pattern);
		sdhci_hisi_add_tuning_phase(host);
	}

	if (tuning_blk)
		kfree(tuning_blk);

	ret = sdhci_hisi_find_best_phase(host);
	if (ret) {
		dev_err(&host->mmc->class_dev, "can not find best phase\n");
		goto err;
	}

	if (set) {
		sdhci_hisi_set_tuning_phase(host, sdhci_hisi->tuning_phase_best);
	} else {
		sdhci_hisi_set_tuning_phase(host, hw_tuning_phase);
		sdhci_hisi->tuning_phase_best = hw_tuning_phase;
	}

err:
	/* restore block size after tuning */
	sdhci_writew(host, SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG,
		MMC_BLOCK_SIZE), SDHCI_BLOCK_SIZE);

	host->flags &= ~SDHCI_EXE_SOFT_TUNING;
	return ret;
}

/*
 * Synopsys controller's max busy timeout is 6.99s. Although we do not
 * use the timeout clock, but the function mmc_do_calc_max_discard use it,
 * if the the card's default erase timeout is more than 6.99s, the discard
 * cannot be supported.
 * As we set the request timeout 30s, so we set a busy timout value which
 * is less than it, choose 1 << 29, the busy clk is 20M,
 * timeout 1 << 29 / 20000 = 26.8s
 */
u32 sdhci_hisi_get_max_timeout_count(struct sdhci_host *host)
{
	return 1 << 29;
}

static void sdhci_hisi_set_pinctrl_idle(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_hisi_data *sdhci_hisi = pltfm_host->priv;
	int ret;

	if (sdhci_check_himntn()) {
		dev_info(&host->mmc->class_dev, "%s himntn enable sd pinctrl return\n", __func__);
		return;
	}

	/* set pin to idle, skip emmc for vccq keeping power always on */
	if (sdhci_hisi->mux_sdsim) {
		config_sdsim_gpio_mode(SDSIM_MODE_SD_IDLE);
	} else {
		if ((sdhci_hisi->pinctrl) && (sdhci_hisi->pins_idle)) {
			ret = pinctrl_select_state(sdhci_hisi->pinctrl, sdhci_hisi->pins_idle);
			if (ret)
				dev_info(&host->mmc->class_dev, "%s could not set idle pins\n", __func__);
		}
	}
}

static void sdhci_hisi_set_pinctrl_normal(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_hisi_data *sdhci_hisi = pltfm_host->priv;
	int ret;

	if (sdhci_check_himntn()) {
		dev_info(&host->mmc->class_dev, "%s himntn enable sd pinctrl return\n", __func__);
		return;
	}

	if (sdhci_hisi->mux_sdsim) {
		config_sdsim_gpio_mode(SDSIM_MODE_SD_NORMAL);
	} else {
		if ((sdhci_hisi->pinctrl) && (sdhci_hisi->pins_default)) {
			ret = pinctrl_select_state(sdhci_hisi->pinctrl, sdhci_hisi->pins_default);
			if (ret)
				dev_info(&host->mmc->class_dev, "%s could not set default pins\n", __func__);
		}
	}
}

static void sdhci_hisi_power_off(struct sdhci_host *host)
{
	struct mmc_host *mmc = host->mmc;
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_hisi_data *sdhci_hisi = pltfm_host->priv;
	int ret = 0;

	sdhci_hisi_set_pinctrl_idle(host);

	sdhci_writeb(host, 0, SDHCI_POWER_CONTROL);
	if (!IS_ERR(mmc->supply.vmmc)) {
		ret = regulator_disable(mmc->supply.vmmc);
		if (ret)
			dev_info(&host->mmc->class_dev, "%s mmc vmmc disable failure\n", __func__);
	}

	if (!IS_ERR(mmc->supply.vqmmc)) {
		ret = regulator_disable(mmc->supply.vqmmc);
		if (ret)
			dev_info(&host->mmc->class_dev, "%s mmc vqmmc disable failure\n", __func__);
	}

	if (sdhci_hisi->lever_shift)
		sdhci_hisi_gpio_value_set(LEVER_SHIFT_3_0V, LEVER_SHIFT_GPIO);
}

static void sdhci_hisi_power_up(struct sdhci_host *host)
{
	struct mmc_host *mmc = host->mmc;
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_hisi_data *sdhci_hisi = pltfm_host->priv;
	int ret = 0;

	if (!IS_ERR(mmc->supply.vqmmc)) {
		if (sdhci_hisi->mux_sdsim)
			/* set vqmmc 1.8V */
			ret = regulator_set_voltage(mmc->supply.vqmmc, 1800000, 1800000);
		else
			/* set vqmmc 2.95V */
			ret = regulator_set_voltage(mmc->supply.vqmmc, 2950000, 2950000);
		if (ret)
			dev_info(&host->mmc->class_dev, "%s mmc vqmmc set failure\n", __func__);

		ret = regulator_enable(mmc->supply.vqmmc);
		if (ret)
			dev_info(&host->mmc->class_dev, "%s mmc vqmmc enable failure\n", __func__);
		usleep_range(1000, 1500);
	}

	if (sdhci_hisi->lever_shift)
		sdhci_hisi_gpio_value_set(LEVER_SHIFT_1_8V, LEVER_SHIFT_GPIO);

	if (!IS_ERR(mmc->supply.vmmc)) {
		if (sdhci_hisi->mux_sdsim && sdhci_hisi->mux_vcc_status == MUX_SDSIM_VCC_STATUS_1_8_0_V)
			/* set vmmc 1.8V */
			ret = regulator_set_voltage(mmc->supply.vmmc, 1800000, 1800000);
		else
			/* set vmmc 2.95V */
			ret = regulator_set_voltage(mmc->supply.vmmc, 2950000, 2950000);
		if (ret)
			dev_info(&host->mmc->class_dev, "%s mmc vmmc set failure\n", __func__);

		ret = regulator_enable(mmc->supply.vmmc);
		if (ret)
			dev_info(&host->mmc->class_dev, "%s mmc vmmc enable failure\n", __func__);
		usleep_range(1000, 1500);
	}

	sdhci_writeb(host, SDHCI_POWER_ON, SDHCI_POWER_CONTROL);
	sdhci_hisi_set_pinctrl_normal(host);
}

static void sdhci_hisi_set_power(struct sdhci_host *host, unsigned char mode, unsigned short vdd)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_hisi_data *sdhci_hisi = pltfm_host->priv;

	if (sdhci_hisi->old_power_mode == mode) /* no need change power */
		return;

	switch (mode) {
	case MMC_POWER_OFF:
		dev_info(&host->mmc->class_dev, "%s: %s set io to lowpower\n", dev_name(sdhci_hisi->dev), __func__);
		sdhci_hisi_power_off(host);
		break;
	case MMC_POWER_UP:
		dev_info(&host->mmc->class_dev, "%s: %s set io to normal\n", dev_name(sdhci_hisi->dev), __func__);
		sdhci_hisi_power_up(host);
		break;
	case MMC_POWER_ON:
		break;
	default:
		dev_info(&host->mmc->class_dev, "%s: %s unknown power supply mode\n", dev_name(sdhci_hisi->dev), __func__);
		break;
	}
	sdhci_hisi->old_power_mode = mode;
}

static int sdhci_hisi_get_cd_gpio(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_hisi_data *sdhci_hisi = pltfm_host->priv;
	int status;

	/* cd_vol = 1 means sdcard gpio detect pin active-high */
	if (sdhci_hisi->cd_vol)
		status = !gpio_get_value(sdhci_hisi->gpio_cd);
	else /* cd_vol = 0 means sdcard gpio detect pin active-low */
		status = gpio_get_value(sdhci_hisi->gpio_cd);

	dev_info(&host->mmc->class_dev, "%s: %s sd status = %d from gpio_get_value(gpio_cd)\n", dev_name(sdhci_hisi->dev), __func__, status);

	status = sd_sim_detect_run(host, status, MODULE_SD, SLEEP_MS_TIME_FOR_DETECT_UNSTABLE);

	if (!status)
		dev_info(&host->mmc->class_dev, "%s: %s sd card is present\n", dev_name(sdhci_hisi->dev), __func__);
	else
		dev_info(&host->mmc->class_dev, "%s: %s sd card is not present\n", dev_name(sdhci_hisi->dev), __func__);
	return !status;
}

static int sdhci_hisi_get_cd(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_hisi_data *sdhci_hisi = pltfm_host->priv;

	/* If sd to jtag func enabled, make the SD always not present */
	if (sdhci_check_himntn()) {
		dev_info(&host->mmc->class_dev, "%s: %s sd status set to 1 here because jtag is enabled\n", dev_name(sdhci_hisi->dev), __func__);
		return 0;
	}

	if (sdhci_hisi->sdhci_host_id == SDHCI_SDIO_ID) {
		dev_err(&host->mmc->class_dev, "%s: %s sdio present = %d\n",
				dev_name(sdhci_hisi->dev), __func__, host->mmc->sdio_present);
		return host->mmc->sdio_present;
	}

	if (sdhci_hisi->card_detect == CARD_UNDETECT)
		sdhci_hisi->card_detect = sdhci_hisi_get_cd_gpio(host);

	return  sdhci_hisi->card_detect;
}

static struct sdhci_ops sdhci_hisi_ops = {
	.get_min_clock = sdhci_hisi_get_min_clock,
	.set_clock = sdhci_set_clock,
	.enable_dma = sdhci_hisi_enable_dma,
	.get_max_clock = sdhci_pltfm_clk_get_max_clock,
	.get_timeout_clock = sdhci_hisi_get_timeout_clock,
	.set_bus_width = sdhci_set_bus_width,
	.reset = sdhci_reset,
	.set_uhs_signaling = sdhci_hisi_set_uhs_signaling,
	.tuning_soft = sdhci_hisi_soft_tuning,
	.tuning_move = sdhci_hisi_tuning_move,
	.check_busy_before_send_cmd = sdhci_chk_busy_before_send_cmd,
	.restore_transfer_para = sdhci_hisi_restore_transfer_para,
	.hw_reset = sdhci_hisi_hw_reset,
	.select_card_type = sdhci_hisi_select_card_type,
	.dumpregs = sdhci_hisi_dumpregs,
	.get_max_timeout_count = sdhci_hisi_get_max_timeout_count,
	.set_power = sdhci_hisi_set_power,
	.sdhci_get_cd = sdhci_hisi_get_cd,
};

static struct sdhci_pltfm_data sdhci_hisi_pdata = {
	.ops = &sdhci_hisi_ops,
	.quirks = SDHCI_QUIRK_CAP_CLOCK_BASE_BROKEN | SDHCI_QUIRK_BROKEN_TIMEOUT_VAL,
	.quirks2 = SDHCI_QUIRK2_PRESET_VALUE_BROKEN | SDHCI_QUIRK2_USE_1_8_V_VMMC,
};

#ifdef CONFIG_PM_SLEEP
/*
 * sdhci_hisi_suspend - Suspend method for the driver
 * @dev:	Address of the device structure
 * Returns 0 on success and error value on error
 *
 * Put the device in a low power state.
 */
static int sdhci_hisi_suspend(struct device *dev)
{
	int ret;
	struct sdhci_host *host = dev_get_drvdata(dev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_hisi_data *sdhci_hisi = pltfm_host->priv;

	dev_info(dev, "%s: suspend +\n", __func__);

	ret = sdhci_suspend_host(host);
	if (ret)
		return ret;


	clk_disable_unprepare(sdhci_hisi->ciu_clk);
	sdhci_hisi_hardware_reset(host);

	dev_info(dev, "%s: suspend -\n", __func__);

	return 0;
}

/*
 * sdhci_hisi_resume - Resume method for the driver
 * @dev:	Address of the device structure
 * Returns 0 on success and error value on error
 *
 * Resume operation after suspend
 */
static int sdhci_hisi_resume(struct device *dev)
{
	int ret;
	struct sdhci_host *host = dev_get_drvdata(dev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_hisi_data *sdhci_hisi = pltfm_host->priv;

	dev_info(dev, "%s: resume +\n", __func__);

	sdhci_hisi_hardware_disreset(host);

	ret = clk_prepare_enable(sdhci_hisi->ciu_clk);
	if (ret) {
		dev_err(dev, "Cannot enable SD clock.\n");
		return ret;
	}
	dev_info(dev, "%s: sdhci_hisi->clk=%lu.\n", __func__, clk_get_rate(sdhci_hisi->ciu_clk));
	dev_info(dev, "%s: host->mmc->ios.clock=%d, timing=%d.\n", __func__, host->mmc->ios.clock, host->mmc->ios.timing);

	ret = sdhci_resume_host(host);
	if (ret)
		return ret;
	/*
	 * use soft tuning sample send wake up cmd then retuning
	 * phase set need to after sdhci_resume_host,because phase may
	 * be clear by host control2 set.The flow need to be provided
	 * by soc,fix me;
	 */
	if (host->mmc->ios.timing >= MMC_TIMING_MMC_HS200) {
		dev_info(dev, "%s: tuning_move_sample=%d, tuning_init_sample=%d\n",
			__func__, sdhci_hisi->tuning_move_phase, sdhci_hisi->tuning_phase_best);
		if (sdhci_hisi->tuning_move_phase == TUNING_PHASE_INVALID)
			sdhci_hisi->tuning_move_phase = sdhci_hisi->tuning_phase_best;
		sdhci_hisi_set_tuning_phase(host, sdhci_hisi->tuning_move_phase);

		dev_err(dev, "resume,host_clock = %d, mmc_clock = %d\n", host->clock, host->mmc->ios.clock);
	}

	dev_info(dev, "%s: resume -\n", __func__);

	return 0;
}
#endif /* ! CONFIG_PM_SLEEP */

#ifdef CONFIG_PM
int sdhci_hisi_runtime_suspend_host(struct sdhci_host *host)
{
	unsigned long flags;

	mmc_retune_timer_stop(host->mmc);
	mmc_retune_needed(host->mmc);

	spin_lock_irqsave(&host->lock, flags);
	host->ier &= SDHCI_INT_CARD_INT;
	sdhci_writel(host, host->ier, SDHCI_INT_ENABLE);
	sdhci_writel(host, host->ier, SDHCI_SIGNAL_ENABLE);
	spin_unlock_irqrestore(&host->lock, flags);

	synchronize_hardirq(host->irq);

	spin_lock_irqsave(&host->lock, flags);
	host->runtime_suspended = true;
	spin_unlock_irqrestore(&host->lock, flags);

	return 0;
}

static int sdhci_hisi_runtime_suspend(struct device *dev)
{
	struct sdhci_host *host = dev_get_drvdata(dev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_hisi_data *sdhci_hisi = pltfm_host->priv;
	int ret = 0;
	u16 reg_u16;

	ret = sdhci_hisi_runtime_suspend_host(host);

	reg_u16 = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	reg_u16 &= ~SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, reg_u16, SDHCI_CLOCK_CONTROL);
	udelay(5);

	if (!IS_ERR(sdhci_hisi->ciu_clk))
		clk_disable_unprepare(sdhci_hisi->ciu_clk);

	return ret;
}

static int sdhci_hisi_runtime_resume_host(struct sdhci_host *host)
{
	unsigned long flags;
	u16 reg_val;

	sdhci_set_default_irqs(host);

	reg_val = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	reg_val |= SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, reg_val, SDHCI_CLOCK_CONTROL);

	spin_lock_irqsave(&host->lock, flags);
	host->runtime_suspended = false;
	spin_unlock_irqrestore(&host->lock, flags);

	return 0;
}

static int sdhci_hisi_runtime_resume(struct device *dev)
{
	struct sdhci_host *host = dev_get_drvdata(dev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_hisi_data *sdhci_hisi = pltfm_host->priv;


	if (!IS_ERR(sdhci_hisi->ciu_clk)) {
		if (clk_prepare_enable(sdhci_hisi->ciu_clk))
			pr_warn("%s: %s: clk_prepare_enable sdhci_hisi->ciu_clk failed.\n", dev_name(dev), __func__);
	}

	return sdhci_hisi_runtime_resume_host(host);
}
#endif

static const struct dev_pm_ops sdhci_hisi_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(sdhci_hisi_suspend, sdhci_hisi_resume)
	SET_RUNTIME_PM_OPS(sdhci_hisi_runtime_suspend, sdhci_hisi_runtime_resume,
						   NULL)
};

static void sdhci_get_driver_strength(struct sdhci_hisi_data *sdhci_hisi, struct device_node *np)
{
	u32 value = 0;

	if (of_property_read_u32(np, "driverstrength_clk", &value)) {
		dev_info(sdhci_hisi->dev, "%s: driverstrength_clk property not found, use default\n", dev_name(sdhci_hisi->dev));
		value = SD_CLK_DRIVER_DEFAULT;
	}
	sd_clk_driver_strength = value;
	dev_info(sdhci_hisi->dev, "%s: driverstrength_clk = 0x%x\n", dev_name(sdhci_hisi->dev), sd_clk_driver_strength);

	if (of_property_read_u32(np, "driverstrength_cmd", &value)) {
		dev_info(sdhci_hisi->dev, "%s: driverstrength_cmd property not found use default\n", dev_name(sdhci_hisi->dev));
		value = SD_CMD_DRIVER_DEFAULT;
	}
	sd_cmd_driver_strength = value;
	dev_info(sdhci_hisi->dev, "%s: driverstrength_cmd = 0x%x\n", dev_name(sdhci_hisi->dev), sd_cmd_driver_strength);

	if (of_property_read_u32(np, "driverstrength_data", &value)) {
		dev_info(sdhci_hisi->dev, "%s: driverstrength_data property not found,  as default\n", dev_name(sdhci_hisi->dev));
		value = SD_DATA_DRIVER_DEFAULT;
	}
	sd_data_driver_strength = value;
	dev_info(sdhci_hisi->dev, "%s: driverstrength_data = 0x%x\n", dev_name(sdhci_hisi->dev), sd_data_driver_strength);
}

static void sdhci_hisi_populate_dt(struct platform_device *pdev)
{
	struct sdhci_host *host = platform_get_drvdata(pdev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_hisi_data *sdhci_hisi = pltfm_host->priv;
	struct device_node *np = pdev->dev.of_node;
	u32 value = 0;

	if (!np) {
		dev_err(&pdev->dev, "can not find device node\n");
		return;
	}
	if (of_property_read_u32(np, "cd-vol", &value)) {
		dev_info(&pdev->dev, "%s: %s cd-vol property not found, using value of 0 as default\n", dev_name(sdhci_hisi->dev), __func__);
		value = 0;
	}
	sdhci_hisi->cd_vol = value;

	if (of_property_read_u32(np, "mux-sdsim", &value)) {
		dev_info(&pdev->dev, "%s: %s mux-sdsim property not found, using value of 0 as default\n", dev_name(sdhci_hisi->dev), __func__);
		value = 0;
	}
	sdhci_hisi->mux_sdsim = value;
	dev_info(&pdev->dev, "%s dts mux-sdsim = %d\n", dev_name(sdhci_hisi->dev), sdhci_hisi->mux_sdsim);

	if (of_find_property(np, "pins_detect_enable", NULL)) {
		dev_info(&pdev->dev, "%s %s pins_detect_enable found\n", dev_name(sdhci_hisi->dev), __func__);
		sdhci_hisi->flags |= PINS_DETECT_ENABLE;
	}

	if (of_find_property(np, "sdio_support_uhs", NULL)) {
		dev_info(&pdev->dev, "sdhci:%d %s find sdio_support_uhs in dts\n", sdhci_hisi->sdhci_host_id,  __func__);
		host->mmc->caps |= (MMC_CAP_UHS_SDR12 | MMC_CAP_UHS_SDR25 | MMC_CAP_UHS_SDR50 | MMC_CAP_UHS_SDR104);
	}

	if (of_find_property(np, "caps2-support-wifi", NULL)) {
		dev_info(&pdev->dev, "sdhci:%d %s find caps2-support-wifi in dts\n", sdhci_hisi->sdhci_host_id,  __func__);
		host->mmc->caps2 |= MMC_CAP2_SUPPORT_WIFI;
	}

	if (of_find_property(np, "caps2-wifi-support-cmd11", NULL)
		&& of_property_count_u8_elems(np, "caps2-wifi-support-cmd11") <= 0) {
		dev_info(&pdev->dev, "sdhci:%d %s find wifi support cmd11 in dts\n",
						sdhci_hisi->sdhci_host_id,  __func__);
		host->mmc->caps2 |= MMC_CAP2_SUPPORT_WIFI_CMD11;
	}

	if (of_find_property(np, "caps-wifi-no-lowpwr", NULL)
		&& of_property_count_u8_elems(np, "caps-wifi-no-lowpwr") <= 0) {
		dev_info(&pdev->dev, "sdhci:%d %s find wifi support no_lowpwr in dts\n",
						sdhci_hisi->sdhci_host_id,  __func__);
		host->mmc->caps2 |= MMC_CAP2_WIFI_NO_LOWPWR;
	}

	sdhci_hisi->mux_vcc_status = MUX_SDSIM_VCC_STATUS_2_9_5_V;
	if (sdhci_hisi->mux_sdsim == 1) {

		sdhci_get_driver_strength(sdhci_hisi, np);

		value = GPIO_DEFAULT_NUMBER_FOR_SD_CLK;
		if (of_property_read_u32(np, "gpionumber_sdclk", &value))
			dev_info(&pdev->dev, "%s gpionumber_sdclk property not found\n", dev_name(sdhci_hisi->dev));
		register_gpio_number_group(value);
	}
}

static void sdhci_hisi_get_pinctrl(struct platform_device *pdev, struct sdhci_hisi_data *sdhci_hisi)
{
	sdhci_hisi->pinctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(sdhci_hisi->pinctrl)) {
		dev_info(&pdev->dev, "%s: could not get pinctrl\n", dev_name(sdhci_hisi->dev));
		sdhci_hisi->pinctrl = NULL;
	}

	if (sdhci_hisi->pinctrl) {
		sdhci_hisi->pins_default = pinctrl_lookup_state(sdhci_hisi->pinctrl, PINCTRL_STATE_DEFAULT);
		/* enable pins to be muxed in and configured */
		if (IS_ERR(sdhci_hisi->pins_default)) {
			dev_info(&pdev->dev, "%s: could not get default pinstate\n",  dev_name(sdhci_hisi->dev));
			sdhci_hisi->pins_default = NULL;
		}
		sdhci_hisi->pins_idle = pinctrl_lookup_state(sdhci_hisi->pinctrl, PINCTRL_STATE_IDLE);
		if (IS_ERR(sdhci_hisi->pins_idle)) {
			dev_info(&pdev->dev,  "%s: could not get default pinstate\n", dev_name(sdhci_hisi->dev));
			sdhci_hisi->pins_idle = NULL;
		}
	}
}

static int sdhci_hisi_clock_init(struct sdhci_host *host, struct platform_device *pdev, struct sdhci_hisi_data *sdhci_hisi)
{
	int ret;
	struct device_node *np = pdev->dev.of_node;

	return 0;
	sdhci_hisi->biu_clk = devm_clk_get(&pdev->dev, "biu");
	if (IS_ERR(sdhci_hisi->biu_clk)) {
		dev_err(&pdev->dev, "biu clock not found.\n");
		ret = PTR_ERR(sdhci_hisi->biu_clk);
		return ret;
	}

	ret = clk_prepare_enable(sdhci_hisi->biu_clk);
	if (ret) {
		dev_err(&pdev->dev, "Unable to enable biu clock.\n");
		return ret;
	}

	sdhci_hisi->ciu_clk = devm_clk_get(&pdev->dev, "ciu");
	if (IS_ERR(sdhci_hisi->ciu_clk)) {
		dev_err(&pdev->dev, "ciu clock not found.\n");
		ret = PTR_ERR(sdhci_hisi->ciu_clk);
		return ret;
	}

	if (of_device_is_available(np)) {
		if (of_property_read_u32(np, "board-mmc-bus-clk", &host->clock)) {
			dev_err(&pdev->dev, "board-mmc-bus-clk cannot get from dts, use the default value!\n");
			host->clock = SDHCI_HISI_CLK_FREQ;
		}
	}

	ret = clk_set_rate(sdhci_hisi->ciu_clk, host->clock);
	if (ret)
		dev_err(&pdev->dev, "Error setting desired host->clock=%u, get clk=%lu\n",
			host->clock, clk_get_rate(sdhci_hisi->ciu_clk));
	ret = clk_prepare_enable(sdhci_hisi->ciu_clk);
	if (ret) {
		dev_err(&pdev->dev, "Unable to enable SD clock.\n");
		return ret;
	}
	dev_info(&pdev->dev, "%s: ciu_clk=%lu\n", __func__, clk_get_rate(sdhci_hisi->ciu_clk));

	return 0;
}

/*
 * BUG: device rename krees old name, which would be realloced for other
 * device, pdev->name points to freed space, driver match may cause a panic
 * for wrong device
 */
static int sdhci_hisi_rename(struct platform_device *pdev, struct sdhci_hisi_data *sdhci_hisi)
{
	int ret = -1;
	static const char *const hi_mci0 = "hi_mci.0";
	static const char *const hi_mci1 = "hi_mci.1";
	static const char *const hi_mci2 = "hi_mci.2";

	switch (sdhci_hisi->sdhci_host_id) {
	case SDHCI_EMMC_ID:
		pdev->name = hi_mci0;
		ret = device_rename(&pdev->dev, hi_mci0);
		if (ret < 0) {
			dev_err(&pdev->dev, "dev set name %s fail\n", hi_mci0);
			goto out;
		}
		break;
	case SDHCI_SD_ID:
		pdev->name = hi_mci1;
		ret = device_rename(&pdev->dev, hi_mci1);
		if (ret < 0) {
			dev_err(&pdev->dev, "dev set name %s fail\n", hi_mci0);
			goto out;
		}

		break;
	case SDHCI_SDIO_ID:
		pdev->name = hi_mci2;
		ret = device_rename(&pdev->dev, hi_mci2);
		if (ret < 0) {
			dev_err(&pdev->dev, "dev set name %s fail\n", hi_mci2);
			goto out;
		}

		break;
	default:
		dev_err(&pdev->dev, "%s: sdhci host id out of range!!!\n", dev_name(&pdev->dev));
		goto out;
	}

out:
	dev_err(&pdev->dev, "%s: %s: ret = %d\n", dev_name(&pdev->dev), __func__, ret);
	return ret;
}

void sdhci_hisi_work_routine_card(struct work_struct *work)
{
	struct sdhci_hisi_data *sdhci_hisi = container_of(work, struct sdhci_hisi_data, card_work);
	struct sdhci_host *host = sdhci_hisi->host;
	struct mmc_host *mmc = host->mmc;

	sdhci_hisi->card_detect = sdhci_hisi_get_cd_gpio(host);
	pr_err("%s: %s card %s\n", __func__, dev_name(sdhci_hisi->dev), sdhci_hisi->card_detect ? "inserted" : "removed");

	pm_runtime_get_sync(mmc_dev(mmc));
	if (mmc->ops->card_event)
		mmc->ops->card_event(host->mmc);

	pm_runtime_mark_last_busy(mmc_dev(mmc));
	pm_runtime_put_autosuspend(mmc_dev(mmc));

	mmc_detect_change(mmc, msecs_to_jiffies(200));
}

static irqreturn_t sdhci_hisi_card_detect(int irq, void *data)
{
	struct sdhci_hisi_data *sdhci_hisi = (struct sdhci_hisi_data *)data;

	queue_work(sdhci_hisi->card_workqueue, &sdhci_hisi->card_work);
	return IRQ_HANDLED;
}

static void sdhci_hisi_set_external_host(struct sdhci_hisi_data *sdhci_hisi)
{
	struct sdhci_host *host = sdhci_hisi->host;

	if (sdhci_hisi->sdhci_host_id == SDHCI_SD_ID) {
		sdhci_host_from_sd_module = host;
		dev_err(sdhci_hisi->dev, "%s: %s sdhci_host_from_sd_module is inited\n",
									dev_name(sdhci_hisi->dev), __func__);
		sema_init(&sem_mux_sdsim_detect, 1);
	} else if (sdhci_hisi->sdhci_host_id == SDHCI_SDIO_ID) {
		sdhci_sdio_host = host;
		dev_err(sdhci_hisi->dev, "%s: %s sdio host for exteral is inited\n",
									dev_name(sdhci_hisi->dev), __func__);
	}
}

static int sdhci_hisi_cd_detect_init(struct platform_device *pdev, struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_hisi_data *sdhci_hisi = pltfm_host->priv;
	struct device_node *np =  pdev->dev.of_node;
	int err;
	int gpio;

	if (sdhci_hisi->sdhci_host_id != SDHCI_SD_ID)
		return 0;

	gpio = of_get_named_gpio(np, "cd-gpio", 0);
	dev_err(&pdev->dev, "%s: %s gpio = %d\n", dev_name(sdhci_hisi->dev), __func__, gpio);

	if (gpio_is_valid(gpio)) {
		if (devm_gpio_request_one(&pdev->dev, gpio, GPIOF_IN, "sdhci-hisi-cd")) {
			dev_err(&pdev->dev, "%s: %s gpio [%d] request failed\n",
						dev_name(sdhci_hisi->dev), __func__, gpio);
		} else {
			sdhci_hisi->gpio_cd = gpio;
			err = devm_request_irq(&pdev->dev, gpio_to_irq(gpio), sdhci_hisi_card_detect,
					IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING |
					IRQF_NO_SUSPEND | IRQF_SHARED, DRIVER_NAME, sdhci_hisi);
			if (err)
				pr_err("%s: %s request gpio irq error\n", dev_name(sdhci_hisi->dev), __func__);
		}
	} else {
		dev_info(&pdev->dev, "%s: %s cd gpio not available",
					dev_name(sdhci_hisi->dev), __func__);
	}
	return 0;
}

static int sdhci_hisi_probe(struct platform_device *pdev)
{
	int ret;
	struct sdhci_host *host = NULL;
	struct sdhci_pltfm_host *pltfm_host = NULL;
	struct sdhci_hisi_data *sdhci_hisi = NULL;

	sdhci_hisi = devm_kzalloc(&pdev->dev, sizeof(*sdhci_hisi), GFP_KERNEL);
	if (!sdhci_hisi)
		return -ENOMEM;

	sdhci_hisi->dev = &pdev->dev;
	sdhci_hisi->card_detect = CARD_UNDETECT;
	sdhci_hisi->sdhci_host_id = of_alias_get_id(sdhci_hisi->dev->of_node, "mshc");
	pr_err("%s: hisi sdhci host id : %d\n", dev_name(sdhci_hisi->dev), sdhci_hisi->sdhci_host_id);

	ret = sdhci_hisi_rename(pdev, sdhci_hisi);
	if (ret)
		goto err_mshc_free;

	host = sdhci_pltfm_init(pdev, &sdhci_hisi_pdata, sizeof(*sdhci_hisi));
	if (IS_ERR(host)) {
		ret = PTR_ERR(host);
		goto err_mshc_free;
	}

	pltfm_host = sdhci_priv(host);
	pltfm_host->priv = sdhci_hisi;
	sdhci_hisi->host = host;

	ret = sdhci_hisi_get_resource(pdev, host);
	if (ret)
		goto err_pltfm_free;

	sdhci_get_of_property(pdev);
	sdhci_hisi_populate_dt(pdev);
	sdhci_hisi_get_pinctrl(pdev, sdhci_hisi);
	sdhci_hisi_cd_detect_init(pdev, host);
	sdhci_hisi_set_external_host(sdhci_hisi);

	ret = sdhci_hisi_clock_init(host, pdev, sdhci_hisi);
	if (ret)
		goto err_pltfm_free;

	pltfm_host->clk = sdhci_hisi->ciu_clk;

	sdhci_hisi_hardware_reset(host);
	sdhci_hisi_hardware_disreset(host);

	if (sdhci_hisi->sdhci_host_id != SDHCI_EMMC_ID)
		host->quirks2 |= SDHCI_QUIRK2_HOST_NO_CMD23;

	host->quirks2 |= SDHCI_QUIRK2_HOST_NO_CMD23;
	host->quirks |= SDHCI_QUIRK_NO_ENDATTR_IN_NOPDESC;
	host->mmc->caps2 |= MMC_CAP2_NO_PRESCAN_POWERUP;
	if (host->mmc->pm_caps & MMC_PM_KEEP_POWER) {
		host->mmc->pm_flags |= MMC_PM_KEEP_POWER;
		host->quirks2 |= SDHCI_QUIRK2_HOST_OFF_CARD_ON;
	}

	if (!(host->quirks2 & SDHCI_QUIRK2_BROKEN_64_BIT_DMA))
		host->dma_mask = DMA_BIT_MASK(64);
	else
		host->dma_mask = DMA_BIT_MASK(32);

	mmc_dev(host->mmc)->dma_mask = &host->dma_mask;

	sdhci_hisi->card_workqueue = alloc_workqueue("sdhci-hisi-card", WQ_MEM_RECLAIM, 1);
	if (!sdhci_hisi->card_workqueue)
		goto clk_disable_all;

	INIT_WORK(&sdhci_hisi->card_work, sdhci_hisi_work_routine_card);

	ret = sdhci_add_host(host);
	if (ret)
		goto err_workqueue;

	pm_runtime_set_active(&pdev->dev);
	pm_runtime_enable(&pdev->dev);
	pm_runtime_set_autosuspend_delay(&pdev->dev, 50);
	pm_runtime_use_autosuspend(&pdev->dev);
	pm_suspend_ignore_children(&pdev->dev, 1);

	pr_err("%s: %s: done! ret = %d\n", dev_name(sdhci_hisi->dev), __func__, ret);

	return 0;

err_workqueue:
	destroy_workqueue(sdhci_hisi->card_workqueue);
clk_disable_all:
	clk_disable_unprepare(sdhci_hisi->ciu_clk);
err_pltfm_free:
	sdhci_pltfm_free(pdev);
err_mshc_free:
	devm_kfree(&pdev->dev, sdhci_hisi);

	pr_err("%s: error = %d!\n", __func__, ret);

	return ret;
}

static int sdhci_hisi_remove(struct platform_device *pdev)
{
	struct sdhci_host *host = platform_get_drvdata(pdev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_hisi_data *sdhci_hisi = pltfm_host->priv;

	dev_info(&pdev->dev, "%s:\n", __func__);

	pm_runtime_get_sync(&pdev->dev);
	sdhci_remove_host(host, 1);
	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_disable(&pdev->dev);

	destroy_workqueue(sdhci_hisi->card_workqueue);
	clk_disable_unprepare(sdhci_hisi->ciu_clk);
	clk_disable_unprepare(sdhci_hisi->biu_clk);

	sdhci_pltfm_free(pdev);

	return 0;
}

static const struct of_device_id sdhci_hisi_of_match[] = {
	{.compatible = "hisilicon,scorpio-sdhci-hisi"},
	{}
};

MODULE_DEVICE_TABLE(of, sdhci_hisi_of_match);

static struct platform_driver sdhci_hisi_driver = {
	.driver = {
			   .name = "sdhci_hw",
			   .of_match_table = sdhci_hisi_of_match,
			   .pm = &sdhci_hisi_dev_pm_ops,
			   },
	.probe = sdhci_hisi_probe,
	.remove = sdhci_hisi_remove,
};

module_platform_driver(sdhci_hisi_driver);

MODULE_DESCRIPTION("Driver for the hisi SDHCI Controller");
MODULE_AUTHOR("Lili <lili17@huawei.com>");
MODULE_LICENSE("GPL");
