/*
 * sd, sdio, driver interface.
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
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
#include "dw_mmc_hi3xxx_helper.h"

#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/sdio.h>
#include <linux/of_address.h>
#include <linux/pm_runtime.h>
#include <pmic_interface.h>
#include <linux/mfd/hisi_pmic.h>

#ifdef CONFIG_HUAWEI_EMMC_DSM
#include <linux/mmc/dsm_emmc.h>
#endif
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

#include "dw_mmc.h"
#include "dw_mmc_hisi.h"
#include "dw_mmc-pltfm.h"
#include "dw_mmc_hi3xxx_common.h"

#ifdef CONFIG_HUAWEI_DSM
static struct dsm_client *wifi_client;
#endif

#ifdef CONFIG_SD_SDIO_CRC_RETUNING
static void dw_mci_sd_crc_retuning_set(struct dw_mci *host, int sam_phase_val,
	int *use_sam_dly, int *enable_shift)
{
	/* used for libra,cancer. but not for aries */
	if (host->use_samdly_flag) {
		if (host->use_samdly_range[0] <= sam_phase_val &&
			sam_phase_val <= host->use_samdly_range[1])
			*use_sam_dly = 1;
		if (host->enable_shift_range[0] <= sam_phase_val &&
			sam_phase_val <= host->enable_shift_range[1])
			*enable_shift = 1;
		dev_info(host->dev, "%s:use_samdly_range_min:%d,use_samdly_range_max:%d!\n",
			__func__, host->use_samdly_range[0],
			host->use_samdly_range[1]);
		return;
	}

	/* used for aries only */
	if (host->clk_change == true) {
		/* clk_change tuning use_sam_dly range */
		if (sam_phase_val >= 8 && sam_phase_val <= 12)
			*use_sam_dly = 1;
		/* clk_change tuning enable_shift range */
		if (sam_phase_val >= 2 && sam_phase_val <= 8)
			*enable_shift = 1;
		dev_info(host->dev, "%s:sd data crc, need retune!\n", __func__);
		return;
	}
	/* default tuning use_sam_dly range */
	if (sam_phase_val >= 4 && sam_phase_val <= 8)
		*use_sam_dly = 1;
	/* default tuning enable_shift range */
	if (sam_phase_val >= 2 && sam_phase_val <= 8)
		*enable_shift = 1;
	dev_info(host->dev, "%s:no data crc occur in aries, no need retune!\n", __func__);
}

static void dw_mci_sdio_crc_retuning_set(struct dw_mci *host, int sam_phase_val,
	int *use_sam_dly, int *enable_shift)
{
	/* used for libra,cancer. but not for aries */
	if (host->use_samdly_flag) {
		if (host->use_samdly_range[0] <= sam_phase_val &&
			sam_phase_val <= host->use_samdly_range[1])
			*use_sam_dly = 1;
		if (host->enable_shift_range[0] <= sam_phase_val &&
			sam_phase_val <= host->enable_shift_range[1])
			*enable_shift = 1;
		return;
	}

	/* used for aries only */
	if (host->clk_change == true) {
		/* clk_change tuning use_sam_dly range */
		if (sam_phase_val >= 8 && sam_phase_val <= 12)
			*use_sam_dly = 1;
		/* clk_change tuning enable_shift range */
		if (sam_phase_val >= 2 && sam_phase_val <= 8)
			*enable_shift = 1;
		return;
	}
	/* default tuning use_sam_dly range */
	if (sam_phase_val >= 8 && sam_phase_val <= 12)
		*use_sam_dly = 1;
	/* default tuning enable_shift range */
	if (sam_phase_val >= 2 && sam_phase_val <= 8)
		*enable_shift = 1;
}
#endif

static void dw_mci_shift_set(struct dw_mci *host, int sam_phase_val, int d_value,
	int *use_sam_dly, int *enable_shift)
{
	*use_sam_dly = 0;
	*enable_shift = 0;

#ifdef CONFIG_SD_SDIO_CRC_RETUNING
	dw_mci_sd_crc_retuning_set(
		host, sam_phase_val, use_sam_dly, enable_shift);
#else
	/* only used in boston_cs platform */
	if (host->is_cs_timing_config == IS_CS_TIMING_CONFIG) {
		/* boston_cs tuning use_sam_dly range */
		if (sam_phase_val >= 4 && sam_phase_val <= 8)
			*use_sam_dly = 1;
		/* boston_cs tuning enable_shift range */
		if (sam_phase_val >= 2 && sam_phase_val <= 6)
			*enable_shift = 1;
		return;
	}

	/*
	 * setting enable_shift range. adapt for libra,
	 * cancer and later chipsets
	 */
	if (host->enable_shift_flag) {
		if (host->enable_shift_range[0] <= sam_phase_val &&
			sam_phase_val <= host->enable_shift_range[1])
			*enable_shift = 1;
	} else {
		/* default tuning enable_shift range */
		if (4 + d_value <= sam_phase_val && sam_phase_val <= 12 + d_value)
			*enable_shift = 1;
	}

	/*
	 * setting use_sample_dly range. adapt for libra,
	 * cancer and later chipsets
	 */
	if (host->use_samdly_flag) {
		if (host->use_samdly_range[0] <= sam_phase_val &&
			sam_phase_val <= host->use_samdly_range[1])
			*use_sam_dly = 1;
	} else {
		/* default tuning use_sam_dly range */
		if (11 + 2 * d_value <= sam_phase_val && sam_phase_val <= 14 + 2 * d_value)
			*use_sam_dly = 1;
	}
#endif
}

static void dw_mci_shift_sdio_set(struct dw_mci *host, int sam_phase_val, int d_value,
	int *use_sam_dly, int *enable_shift)
{
	*use_sam_dly = 0;
	*enable_shift = 0;

#ifdef CONFIG_SD_SDIO_CRC_RETUNING
	dw_mci_sdio_crc_retuning_set(
		host, sam_phase_val, use_sam_dly, enable_shift);
#else
	/* only used in boston_cs platform */
	if (host->is_cs_timing_config == IS_CS_TIMING_CONFIG) {
		/* boston_cs tuning use_sam_dly range */
		if (sam_phase_val >= 8 && sam_phase_val <= 12)
			*use_sam_dly = 1;
		/* boston_cs tuning enable_shift range */
		if (sam_phase_val >= 2 && sam_phase_val <= 8)
			*enable_shift = 1;
		return;
	}

	/* setting enable_shift range. adapt for libra,cancer and later chipsets */
	if (host->enable_shift_flag) {
		if (host->enable_shift_range[0] <= sam_phase_val &&
			sam_phase_val <= host->enable_shift_range[1])
			*enable_shift = 1;
	} else {
		/* other chipsets tuning enable_shift range */
		if (sam_phase_val >= 4 && sam_phase_val <= 12)
			*enable_shift = 1;
	}

	/* setting use_sample_dly range. adapt for libra,cancer and later chipsets */
	if (host->use_samdly_flag) {
		if (host->use_samdly_range[0] <= sam_phase_val &&
			sam_phase_val <= host->use_samdly_range[1])
			*use_sam_dly = 1;
	} else {
		/* other chipsets tuning use_sam_dly range */
		if (sam_phase_val >= 11 && sam_phase_val <= 14)
			*use_sam_dly = 1;
	}
#endif
}

static void dw_mci_shift_set_helper(struct dw_mci *host, int type, int timing,
		int sam_phase_val, int *enable_shift, int *use_sam_dly, int d_value)
{
	/* enable_shift and use_sam_dly setting code */
	/* warning! different with K3V3 */
	switch (type) {
	case DW_MCI_EMMC_ID:
		switch (timing) {
		case MMC_TIMING_UHS_DDR50:
			/* MMC_TIMING_UHS_DDR50 tuning enable_shift range */
			if (sam_phase_val >= 4 && sam_phase_val <= 12)
				*enable_shift = 1;
			break;
		case MMC_TIMING_MMC_HS200:
			/* MMC_TIMING_MMC_HS200 tuning enable_shift range */
			if (sam_phase_val >= 4 && sam_phase_val <= 12)
				*enable_shift = 1;
			/* MMC_TIMING_MMC_HS200 tuning use_sam_dly range */
			if (sam_phase_val >= 11 && sam_phase_val <= 14)
				*use_sam_dly = 1;
			break;
		default:
			break;
		}
		break;
	case DW_MCI_SD_ID:
		switch (timing) {
		case MMC_TIMING_UHS_SDR50:
			/* MMC_TIMING_UHS_SDR50 tuning enable_shift range */
			if (4 + d_value <= sam_phase_val && sam_phase_val <= 12 + d_value)
				*enable_shift = 1;
			break;
		case MMC_TIMING_UHS_SDR104:
			dw_mci_shift_set(host, sam_phase_val, d_value,
				use_sam_dly, enable_shift);
			break;
		case MMC_TIMING_MMC_HS200:
			dw_mci_shift_set(host, sam_phase_val, d_value,
				use_sam_dly, enable_shift);
			break;
		default:
			break;
		}
		break;
	case DW_MCI_SDIO_ID:
		switch (timing) {
		case MMC_TIMING_UHS_SDR12:
			break;
		case MMC_TIMING_UHS_DDR50:
			/* MMC_TIMING_UHS_DDR50 tuning enable_shift range */
			if (sam_phase_val >= 4 && sam_phase_val <= 12)
				*enable_shift = 1;
			break;
		case MMC_TIMING_UHS_SDR50:
			/* MMC_TIMING_UHS_SDR50 tuning enable_shift range */
			if (sam_phase_val >= 4 && sam_phase_val <= 12)
				*enable_shift = 1;
			break;
		case MMC_TIMING_UHS_SDR104:
			dw_mci_shift_sdio_set(host, sam_phase_val, d_value,
				use_sam_dly, enable_shift);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void dw_mci_hs_set_timing(
	struct dw_mci *host, int id, int timing, int sam_phase, int clk_div)
{
	int cclk_div, sam_phase_val;
	unsigned int drv_phase, sam_dly;
	int sam_phase_max, sam_phase_min;
	int reg_value;
	int enable_shift = 0;
	int use_sam_dly = 0;
	int d_value = 0;
	struct dw_mci_slot *slot = host->cur_slot;

	if ((host->hw_mmc_id == DWMMC_SD_ID) && (timing == MMC_TIMING_LEGACY))
		cclk_div = hs_timing_config[id][timing][CCLK_DIV];
	else
		cclk_div = clk_div;
	if (host->hw_mmc_id == DWMMC_SD_ID)
		d_value = cclk_div - hs_timing_config[id][timing][CCLK_DIV];
	drv_phase = hs_timing_config[id][timing][DRV_PHASE];
	sam_dly = hs_timing_config[id][timing][SAM_DLY] + d_value;
	sam_phase_max = hs_timing_config[id][timing][SAM_PHASE_MAX] + 2 * d_value;
	sam_phase_min = hs_timing_config[id][timing][SAM_PHASE_MIN];

	if (sam_phase == -1)
		sam_phase_val = (sam_phase_max + sam_phase_min) / SAM_PHASE_MID;
	else
		sam_phase_val = sam_phase;

	dw_mci_shift_set_helper(host, id, timing, sam_phase_val,
		&enable_shift, &use_sam_dly, d_value);

	/* first disabl clk */
	mci_writel(host, GPIO, 0x0);
	udelay(50);

	reg_value = sdmmc_uhs_reg_ext_value(
		(unsigned int)sam_phase_val, sam_dly, drv_phase);
	mci_writel(host, UHS_REG_EXT, reg_value);

	mci_writel(host, ENABLE_SHIFT, enable_shift);

	reg_value = sdmmc_gpio_value((unsigned int)cclk_div, (unsigned int)use_sam_dly);
	udelay(50);
	mci_writel(host, GPIO, (unsigned int)reg_value | GPIO_CLK_ENABLE);

	if (!(slot && slot->sdio_wakelog_switch))
		dev_info(host->dev, "id=%d,timing=%d,UHS_REG_EXT=0x%x,"
			"ENABLE_SHIFT=0x%x,GPIO=0x%x\n",
			id, timing, mci_readl(host, UHS_REG_EXT),
			mci_readl(host, ENABLE_SHIFT), mci_readl(host, GPIO));
}

#ifdef CONFIG_HUAWEI_DSM
void dw_mci_dsm_dump(struct dw_mci *host, int err_num)
{
	/* no need to dump message, use hisi dump */
}
EXPORT_SYMBOL(dw_mci_dsm_dump);

void dsm_wifi_client_notify(int dsm_id, const char *format, ...)
{
	char buf[LOG_BUF_SIZE] = {0};
	char client_name[] = "dsm_wifi";
	int size;
	va_list args;

	if (wifi_client == NULL) {
		wifi_client = dsm_find_client((char *)client_name);
		if (wifi_client == NULL)
			return;
	}
	if (format) {
		va_start(args, format);
		size = vsnprintf(buf, LOG_BUF_SIZE, format, args);
		va_end(args);
		if (size < 0)
			return;
	}
	if (!dsm_client_ocuppy(wifi_client)) {
		dsm_client_record(wifi_client, buf);
		dsm_client_notify(wifi_client, dsm_id);
	} else {
		pr_err("dsm client notify failed, dsm id is %d\n", dsm_id);
	}
}
#endif

void dw_mci_set_cd(struct dw_mci *host)
{
	if (!host)
		return;

	if (host->slot[0] && host->slot[0]->mmc) {
		dev_dbg(&host->slot[0]->mmc->class_dev, "sdio_present = %d\n",
			host->slot[0]->mmc->sdio_present);
		host->slot[0]->mmc->sdio_present = 1;
	}
}

void dw_mci_sdio_card_detect(struct dw_mci *host)
{
	if (!host) {
		pr_err("sdio detect, host is null,can not used to detect sdio\n");
		return;
	}

	dw_mci_set_cd(host);

	queue_work(host->card_workqueue, &host->card_work);
}

void dw_mci_sdio_card_detect_change(void)
{
	dw_mci_sdio_card_detect(sdio_host);
}
EXPORT_SYMBOL(dw_mci_sdio_card_detect_change);

void dw_mci_set_timeout(struct dw_mci *host)
{
	/* timeout (maximum) */
	mci_writel(host, TMOUT, 0xffffffff);
}

static void dw_mci_hs_tuning_clear_flags(struct dw_mci *host)
{
	host->tuning_sample_flag = 0;
}

#ifdef CONFIG_SDIO_HI_CLOCK_RETUNING
static void dw_mci_hs_get_timing_config_normal(struct dw_mci *host)
{
	struct device_node *np = host->dev->of_node;
	int ret;

	ret = of_property_read_u32_array(np, "use_samdly_range",
		&(host->use_samdly_range[0]), DW_MMC_MIN_SZ_ARRAY_READ);
	if (ret) {
		dev_info(host->dev, "%s:get use_sample_dly range range failed!\n", __func__);
		host->use_samdly_range[RANGE_MIN] = 4;
		host->use_samdly_range[RANGE_MAX] = 8;
	} else {
		dev_info(host->dev, "%s:use_samdly_range_min:%d,use_samdly_range_max:%d!\n",
			__func__, host->use_samdly_range[RANGE_MIN],
			host->use_samdly_range[RANGE_MAX]);
	}

	ret = of_property_read_u32_array(np, "enable_shift_range",
		&(host->enable_shift_range[0]), DW_MMC_MIN_SZ_ARRAY_READ);
	if (ret) {
		dev_info(host->dev, "%s:get enable_shift range range failed!\n", __func__);
		host->enable_shift_range[RANGE_MIN] = 4;
		host->enable_shift_range[RANGE_MAX] = 8;
	} else {
		dev_info(host->dev, "%s:ena_shift_range_min:%d,ena_shift_range_max:%d!\n",
			__func__, host->enable_shift_range[RANGE_MIN],
			host->enable_shift_range[RANGE_MAX]);
	}
}

static void dw_mci_hs_get_timing_config_high_speed_clk(struct dw_mci *host)
{
	struct device_node *np = host->dev->of_node;
	int ret;

	ret = of_property_read_u32_array(np, "use_samdly_range_hi",
		&(host->use_samdly_range[0]), DW_MMC_MIN_SZ_ARRAY_READ);
	if (ret) {
		dev_info(host->dev, "%s:get use_samdly_range_hi range range failed!\n", __func__);
		host->use_samdly_range[RANGE_MIN] = 9;
		host->use_samdly_range[RANGE_MAX] = 18;
	} else {
		dev_info(host->dev, "%s:use_samdly_range_hi_min:%d,use_samdly_range_hi_max:%d!\n",
			__func__, host->use_samdly_range[RANGE_MIN],
			host->use_samdly_range[RANGE_MIN]);
	}

	ret = of_property_read_u32_array(np, "enable_shift_range_hi",
		&(host->enable_shift_range[0]), DW_MMC_MIN_SZ_ARRAY_READ);
	if (ret) {
		dev_info(host->dev, "%s:get enable_shift_range_hi range range failed!\n", __func__);
		host->enable_shift_range[RANGE_MIN] = 8;
		host->enable_shift_range[RANGE_MAX] = 17;
	} else {
		dev_info(host->dev, "%s:ena_shift_range_hi_min:%d,ena_shift_range_hi_max:%d!\n",
			__func__, host->enable_shift_range[RANGE_MIN],
			host->enable_shift_range[RANGE_MAX]);
	}
}

/*
 * Use 960M clock to work clean tuning flag
 * switch clock to 960M
 */
static void dw_mci_hs_960_clock_source_retuning(struct dw_mci *host)
{
	struct dw_mci_hs_priv_data *priv = host->priv;

	memcpy(hs_timing_config[DWMMC_SDIO_ID][MMC_TIMING_UHS_SDR104],
		hs_timing_960_clock_source_capricorn,
		sizeof(hs_timing_960_clock_source_capricorn));

	host->tuning_init_sample = host->tuning_init_sample / SAM_PHASE_MID;
	priv->old_timing = -1;
	priv->in_resume = STATE_KEEP_PWR;
	host->sd_reinit = 0;

	dw_mci_hs_get_timing_config_normal(host);
	dw_mci_hs_set_clk_helper(host, &host->cur_slot->mmc->ios);

	if (!IS_ERR(host->volt_hold_sdio_clk))
		clk_disable_unprepare(host->volt_hold_sdio_clk);
}

/*
 * Use 1920M clock to work clean tuning flag
 * switch clock to 1920M
 */
static void dw_mci_hs_1920_clock_source_retuning(struct dw_mci *host)
{
	struct dw_mci_hs_priv_data *priv = host->priv;

	memcpy(hs_timing_config[DWMMC_SDIO_ID][MMC_TIMING_UHS_SDR104],
		hs_timing_1920_clock_source_capricorn,
		sizeof(hs_timing_1920_clock_source_capricorn));

	priv->old_timing = -1;
	priv->in_resume = STATE_KEEP_PWR;
	host->sd_reinit = 0;

	dw_mci_hs_get_timing_config_high_speed_clk(host);
	dw_mci_hs_set_clk_helper(host, &host->cur_slot->mmc->ios);

	if (!IS_ERR(host->volt_hold_sdio_clk)) {
		if (clk_prepare_enable(host->volt_hold_sdio_clk))
			dev_err(host->dev, "volt_hold_sdio_clk clk_prepare_enable failed\n");
	}
}
#endif

#ifdef CONFIG_SDIO_HI_CLOCK_RETUNING
static void dw_mci_hs_tuning_all_pass(
	struct dw_mci *host, int *ret, int sample_max, int timing)
{
	if ((host->tuning_flag_all_pass == TUNING_FLAG_ALL_PASS) &&
		(host->hw_mmc_id == DWMMC_SDIO_ID)) {
		if (host->data_crc_flag == 1) {
			host->tuning_init_sample = 0;
			host->data_crc_flag = 0;
		} else if (((DW_MMC_TUNING_MOVE_OFFSET << (unsigned int)sample_max) -
			DW_MMC_SAMPLE_OFFSET) == host->tuning_sample_flag) {
			host->all_pass_flag = 1;
		}

		dw_mci_hs_960_clock_source_retuning(host);
		host->tuning_flag_all_pass = 0;
		dev_info(host->dev, "tuning OK using high speed clk: timing is "
			"%d, tuning sample = %d, tuning_flag = 0x%x\n",
			timing, host->tuning_init_sample,
			host->tuning_sample_flag);
	} else if (((DW_MMC_TUNING_MOVE_OFFSET << (unsigned int)sample_max) -
		DW_MMC_SAMPLE_OFFSET) == host->tuning_sample_flag &&
		   (host->hw_mmc_id == DWMMC_SDIO_ID)) {
		host->tuning_flag_all_pass = TUNING_FLAG_ALL_PASS;
		dw_mci_hs_1920_clock_source_retuning(host);
		dev_info(host->dev, "all tuning sample return ok,try to retuning\n");
		*ret = TUNING_ERR_ALL_PHASE_PASS;
	}
}
#endif

static int dw_mci_hs_find_best_value(struct dw_mci *host, int timing,
	int sample_min, int sample_max)
{
	const struct dw_mci_drv_data *drv_data = host->drv_data;
	int i, j;
	unsigned int mask;
	int mask_lenth;
	int ret = 0;

	for (mask_lenth = (((unsigned int)(sample_max - sample_min) >>
		DW_MMC_SAMPLE_OFFSET) << DW_MMC_SAMPLE_OFFSET) + DW_MMC_SAMPLE_OFFSET;
		mask_lenth >= DW_MMC_SAMPLE_OFFSET; mask_lenth -= DW_MMC_TUNING_MOVE_OFFSET) {
		mask = (DW_MMC_SAMPLE_OFFSET << (unsigned int)mask_lenth) - DW_MMC_SAMPLE_OFFSET;
		for (i = (sample_min + sample_max - mask_lenth + DW_MMC_SAMPLE_OFFSET)
			/ DW_MMC_TUNING_MOVE_OFFSET, j = DW_MMC_SAMPLE_OFFSET;
			(i <= sample_max - mask_lenth + DW_MMC_SAMPLE_OFFSET) && (i >= sample_min);
			i = ((sample_min + sample_max - mask_lenth + DW_MMC_SAMPLE_OFFSET) /
			DW_MMC_TUNING_MOVE_OFFSET) + ((j % DW_MMC_TUNING_MOVE_OFFSET) ?
			DW_MMC_TUNING_MOVE_LEFT : DW_MMC_TUNING_MOVE_RIGHT) *
			(j / DW_MMC_TUNING_MOVE_OFFSET)) {
			if ((host->tuning_sample_flag & (mask << (unsigned int)i)) == (mask << (unsigned int)i)) {
				host->tuning_init_sample = i + mask_lenth / DW_MMC_TUNING_MOVE_OFFSET;
				break;
			}
			j++;
		}

		if (host->tuning_init_sample != -1) {
			if ((host->hw_mmc_id == DWMMC_SD_ID) && (mask_lenth < 3) && (drv_data->slowdown_clk)) {
				dev_info(host->dev, "sd card tuning need slow down clk, timing is %d, tuning_flag = 0x%x\n",
					timing, host->tuning_sample_flag);
				return -1;
			}
			dev_info(host->dev, "tuning OK: timing is %d, tuning sample = %d, tuning_flag = 0x%x",
				timing, host->tuning_init_sample, host->tuning_sample_flag);

			ret = 0;
#ifdef CONFIG_SDIO_HI_CLOCK_RETUNING
			dw_mci_hs_tuning_all_pass(host, &ret, sample_max, timing);
#endif

#ifdef CONFIG_SD_TIMEOUT_RESET
			host->set_sd_data_tras_timeout = 0;
			host->set_sdio_data_tras_timeout = 0;
#endif
			break;
		}
	}

	return ret;
}

/* tuning finish, select the best sam_del */
static void dw_mci_hs_tuning_finish_handle(struct dw_mci *host,
	int timing, int sample_min, int sample_max, int id, int *ret)
{
	if (host->tuning_init_sample == -1) {
		host->tuning_init_sample = (sample_min + sample_max) / SAM_PHASE_MID;
		dev_info(host->dev, "tuning err: no good sam_del, timing is %d, tuning_flag = 0x%x",
			timing, host->tuning_sample_flag);
#ifdef CONFIG_HUAWEI_EMMC_DSM
		if (host->hw_mmc_id == DWMMC_EMMC_ID)
			DSM_EMMC_LOG(host->cur_slot->mmc, DSM_EMMC_TUNING_ERROR,
				"%s:eMMC tuning error: timing is %d, tuning_flag = 0x%x\n",
				__func__, timing, host->tuning_sample_flag);
#endif
#ifdef CONFIG_HUAWEI_DSM
		if (host->hw_mmc_id == DWMMC_SDIO_ID)
			dsm_wifi_client_notify(DSM_SDIO_TUNNING_ERR, "sdio tuning error: timing %d, flag 0x%x\n",
				timing, host->tuning_sample_flag);
#endif
		*ret = -1;
	}

	dw_mci_hs_set_timing(host, id, timing, host->tuning_init_sample, host->current_div);
}

/*
 * By tuning, find the best timing condition
 * 1 -- tuning is not finished. And this function should be called again
 * 0 -- Tuning successfully.
 * If this function be called again, another round of tuning would be start
 * -1 -- Tuning failed. Maybe slow down the clock and call this function again
 */
int dw_mci_hs_tuning_find_condition_helper(struct dw_mci *host, int timing)
{
	struct dw_mci_hs_priv_data *priv = host->priv;
	int id = priv->id;
	int sample_min, sample_max;
	int ret = 0;
	int d_value = 0;

	if (host->hw_mmc_id == DWMMC_SD_ID) {
		d_value = host->current_div - hs_timing_config[id][timing][CCLK_DIV];
		if (timing == MMC_TIMING_SD_HS) {
			sample_max = hs_timing_config[id][timing][SAM_PHASE_MAX] + d_value;
			sample_min = hs_timing_config[id][timing][SAM_PHASE_MIN] + d_value;
		} else if ((timing == MMC_TIMING_UHS_SDR50) ||
			   (timing == MMC_TIMING_UHS_SDR104) ||
			   (timing == MMC_TIMING_MMC_HS200)) {
			sample_max = hs_timing_config[id][timing][SAM_PHASE_MAX] +
				DW_MMC_TUNING_MOVE_OFFSET * d_value;
			sample_min = hs_timing_config[id][timing][SAM_PHASE_MIN];
		} else {
			sample_max = hs_timing_config[id][timing][SAM_PHASE_MAX];
			sample_min = hs_timing_config[id][timing][SAM_PHASE_MIN];
		}
	} else {
		sample_max = hs_timing_config[id][timing][SAM_PHASE_MAX];
		sample_min = hs_timing_config[id][timing][SAM_PHASE_MIN];
	}

	if (sample_max == sample_min) {
		host->tuning_init_sample = (sample_max + sample_min) / SAM_PHASE_MID;
		dw_mci_hs_set_timing(host, id, timing, host->tuning_init_sample, host->current_div);
		dev_info(host->dev, "no need tuning: timing is %d, tuning sample = %d",
			timing, host->tuning_init_sample);
		return 0;
	}

	if (host->tuning_current_sample == -1) {
		dw_mci_hs_tuning_clear_flags(host);

		/* set the first sam del as the min_sam_del */
		host->tuning_current_sample = sample_min;
		/* a trick for next "++" */
		host->tuning_current_sample--;
	}

	if (host->tuning_current_sample >= sample_max) {
		/* set sam del to -1, for next tuning */
		host->tuning_current_sample = -1;

		host->tuning_init_sample = -1;
		ret = dw_mci_hs_find_best_value(host, timing, sample_min, sample_max);
		if (ret == -1)
			return -1;
		dw_mci_hs_tuning_finish_handle(host, timing, sample_min, sample_max, id, &ret);
		return ret;
	}

	host->tuning_current_sample++;
	dw_mci_hs_set_timing(host, id, timing, host->tuning_current_sample, host->current_div);

	return 1;
}

int dw_mci_sdio_wakelog_switch(struct mmc_host *mmc, bool enable)
{
	struct dw_mci_slot *slot = NULL;

	if (!mmc)
		return -1;

	slot = mmc_priv(mmc);
	if (!slot)
		return -1;

	if (enable)
		slot->sdio_wakelog_switch = 1;
	else
		slot->sdio_wakelog_switch = 0;

	slot->sdio_wakelog_switch = slot->sdio_wakelog_switch &&
		(MMC_CAP2_SUPPORT_WIFI & (mmc->caps2));

	return slot->sdio_wakelog_switch;
}
EXPORT_SYMBOL(dw_mci_sdio_wakelog_switch);

#ifdef CONFIG_SD_SDIO_CRC_RETUNING
/*
 * name:dw_mci_hisi_change_timing_config_cancer
 * func: the soc platform of cancer when sd or sdio CRC issue,
 * will excute here to set SDR104 timing config for retuning
 */
static void dw_mci_hisi_change_timing_config_cancer(struct dw_mci *host)
{
	if (host->hw_mmc_id == DWMMC_SD_ID) {
		memcpy(hs_timing_config[DWMMC_SD_ID][MMC_TIMING_UHS_SDR104],
			hs_timing_config_cancer_cs_sd_ppll2,
			sizeof(hs_timing_config_cancer_cs_sd_ppll2)); /* unsafe_function_ignore: memcpy */
		host->use_samdly_range[RANGE_MIN] = 5;
		host->use_samdly_range[RANGE_MAX] = 8;
		host->enable_shift_range[RANGE_MIN] = 4;
		host->enable_shift_range[RANGE_MAX] = 8;
	} else if (host->hw_mmc_id == DWMMC_SDIO_ID) {
		memcpy(hs_timing_config[DWMMC_SDIO_ID][MMC_TIMING_UHS_SDR104],
			hs_timing_config_cancer_cs_sdio_ppll2,
			sizeof(hs_timing_config_cancer_cs_sdio_ppll2)); /* unsafe_function_ignore: memcpy */
		host->use_samdly_range[RANGE_MIN] = 5;
		host->use_samdly_range[RANGE_MAX] = 8;
		host->enable_shift_range[RANGE_MIN] = 4;
		host->enable_shift_range[RANGE_MAX] = 8;
	}
}

/*
 * name: dw_mci_hisi_change_timing_config_capricorn
 * func: the soc platform of capricorn when sd or sdio CRC issue,
 * will excute here to set SDR104 timing config for retuning
 */
static void dw_mci_hisi_change_timing_config_capricorn(struct dw_mci *host)
{
	if (host->hw_mmc_id == DWMMC_SD_ID) {
		memcpy(hs_timing_config[DWMMC_SD_ID][MMC_TIMING_UHS_SDR104],
			hs_timing_960_clock_source_capricorn,
			sizeof(hs_timing_960_clock_source_capricorn)); /* unsafe_function_ignore: memcpy */
		host->use_samdly_range[RANGE_MIN] = 4;
		host->use_samdly_range[RANGE_MAX] = 8;
		host->enable_shift_range[RANGE_MIN] = 4;
		host->enable_shift_range[RANGE_MAX] = 8;
	} else if (host->hw_mmc_id == DWMMC_SDIO_ID) {
		memcpy(hs_timing_config[DWMMC_SDIO_ID][MMC_TIMING_UHS_SDR104],
			hs_timing_960_clock_source_capricorn,
			sizeof(hs_timing_960_clock_source_capricorn)); /* unsafe_function_ignore: memcpy */
		host->use_samdly_range[RANGE_MIN] = 4;
		host->use_samdly_range[RANGE_MAX] = 8;
		host->enable_shift_range[RANGE_MIN] = 4;
		host->enable_shift_range[RANGE_MAX] = 8;
	}
}

/*
 * name:dw_mci_hisi_change_timing_config_libra
 * func: the soc platform of libra when sd or sdio CRC issue,
 * will excute here to set SDR104 timing config for retuning
 */
static void dw_mci_hisi_change_timing_config_libra(struct dw_mci *host)
{
	if (host->hw_mmc_id == DWMMC_SD_ID) {
		memcpy(hs_timing_config[DWMMC_SD_ID][MMC_TIMING_UHS_SDR104],
			hs_timing_config_libra_sd_ppll2,
			sizeof(hs_timing_config_libra_sd_ppll2)); /* unsafe_function_ignore: memcpy */
		host->use_samdly_range[RANGE_MIN] = 5;
		host->use_samdly_range[RANGE_MAX] = 8;
		host->enable_shift_range[RANGE_MIN] = 3;
		host->enable_shift_range[RANGE_MAX] = 8;
	} else if (host->hw_mmc_id == DWMMC_SDIO_ID) {
		memcpy(hs_timing_config[DWMMC_SDIO_ID][MMC_TIMING_UHS_SDR104],
			hs_timing_config_libra_sdio_ppll2,
			sizeof(hs_timing_config_libra_sdio_ppll2)); /* unsafe_function_ignore: memcpy */
		host->use_samdly_range[RANGE_MIN] = 5;
		host->use_samdly_range[RANGE_MAX] = 8;
		host->enable_shift_range[RANGE_MIN] = 3;
		host->enable_shift_range[RANGE_MAX] = 8;
	}
}

/*
 * name:dw_mci_hisi_change_timing_config_taurus
 * func: the soc platform of taurus when sd or sdio CRC issue,
 * will excute here to set SDR104 timing config for retuning
 */
static void dw_mci_hisi_change_timing_config_taurus(struct dw_mci *host)
{
	if (host->hw_mmc_id == DWMMC_SD_ID) {
		memcpy(hs_timing_config[DWMMC_SD_ID][MMC_TIMING_UHS_SDR104],
			hs_timing_config_taurus_sd_ppll2,
			sizeof(hs_timing_config_taurus_sd_ppll2)); /* unsafe_function_ignore: memcpy */
		host->use_samdly_range[RANGE_MIN] = 6;
		host->use_samdly_range[RANGE_MAX] = 8;
		host->enable_shift_range[RANGE_MIN] = 4;
		host->enable_shift_range[RANGE_MAX] = 8;
	} else if (host->hw_mmc_id == DWMMC_SDIO_ID) {
		memcpy(hs_timing_config[DWMMC_SDIO_ID][MMC_TIMING_UHS_SDR104],
			hs_timing_config_taurus_sdio_ppll2,
			sizeof(hs_timing_config_taurus_sdio_ppll2)); /* unsafe_function_ignore: memcpy */
		host->use_samdly_range[RANGE_MIN] = 8;
		host->use_samdly_range[RANGE_MAX] = 14;
		host->enable_shift_range[RANGE_MIN] = 8;
		host->enable_shift_range[RANGE_MAX] = 14;
	}
}

/*
 * name:dw_mci_hisi_change_timing_config_aries
 * func: the soc platform of aries when sd or sdio CRC issue,
 * will excute here to set SDR104 timing config for retuning
 */
static void dw_mci_hisi_change_timing_config_aries(struct dw_mci *host)
{
	if (host->hw_mmc_id == DWMMC_SD_ID) {
		memcpy(hs_timing_config[DWMMC_SD_ID][MMC_TIMING_UHS_SDR104],
			hs_timing_config_aries_cs_sd_ppll3,
			sizeof(hs_timing_config_aries_cs_sd_ppll3)); /* unsafe_function_ignore: memcpy */
	} else if (host->hw_mmc_id == DWMMC_SDIO_ID) {
		memcpy(hs_timing_config[DWMMC_SDIO_ID][MMC_TIMING_UHS_SDR104],
			hs_timing_config_aries_cs_sdio_ppll3,
			sizeof(hs_timing_config_aries_cs_sdio_ppll3)); /* unsafe_function_ignore: memcpy */
	}
}

void dw_mci_hs_change_timing_config(struct dw_mci *host)
{
	struct device_node *np = host->dev->of_node;

	if (of_device_is_compatible(np, "hisilicon,capricorn-dw-mshc"))
		dw_mci_hisi_change_timing_config_capricorn(host);
	else if (of_device_is_compatible(np, "hisilicon,cancer-dw-mshc"))
		dw_mci_hisi_change_timing_config_cancer(host);
	else if (of_device_is_compatible(np, "hisilicon,libra-dw-mshc"))
		dw_mci_hisi_change_timing_config_libra(host);
	else if (of_device_is_compatible(np, "hisilicon,taurus-dw-mshc"))
		dw_mci_hisi_change_timing_config_taurus(host);
	else
		dw_mci_hisi_change_timing_config_aries(host);
}
#endif

int dw_mci_hs_tuning_move_helper(struct dw_mci *host, int timing, int start)
{
	struct dw_mci_hs_priv_data *priv = host->priv;
	int id = priv->id;
	int sample_min, sample_max;
	int loop;
	struct dw_mci_slot *slot = host->cur_slot;

	sample_max = hs_timing_config[id][timing][SAM_PHASE_MAX];
	sample_min = hs_timing_config[id][timing][SAM_PHASE_MIN];

	if (sample_max == sample_min) {
		dev_info(host->dev, "id = %d, tuning move return\n", id);
		return 0;
	}

	if (start)
		host->tuning_move_count = 0;

	for (loop = 0; loop < 2; loop++) {
		host->tuning_move_count++;
		host->tuning_move_sample =
			host->tuning_init_sample +
			((host->tuning_move_count % DW_MMC_TUNING_MOVE_OFFSET) ?
			 DW_MMC_TUNING_MOVE_RIGHT : DW_MMC_TUNING_MOVE_LEFT) *
			(host->tuning_move_count / SAM_PHASE_MID);

		if ((host->tuning_move_sample > sample_max) ||
			(host->tuning_move_sample < sample_min)) {
			continue;
		} else {
			break;
		}
	}

	if ((host->tuning_move_sample > sample_max) ||
		(host->tuning_move_sample < sample_min)) {
		dw_mci_hs_set_timing(host, id, timing, host->tuning_init_sample, host->current_div);
		dev_info(host->dev, "id = %d, tuning move end to init del_sel %d\n", id,
			host->tuning_init_sample);
		return 0;
	}
	dw_mci_hs_set_timing(host, id, timing, host->tuning_move_sample, host->current_div);

	if (!(slot && slot->sdio_wakelog_switch))
		dev_info(host->dev, "id = %d, tuning move to current del_sel %d\n",
			id, host->tuning_move_sample);

	return 1;
}

static int dw_mci_tuning_prepare(struct dw_mci_slot *slot, u32 opcode,
	int *blksz, u32 *arg, u32 *flags)
{
	struct mmc_host *mmc = slot->mmc;
	struct dw_mci *host = slot->host;
	int id = host->hw_mmc_id;
	const u8 *tuning_blk_pattern = NULL;

	if (opcode == MMC_SEND_TUNING_BLOCK_HS200) {
		if (mmc->ios.bus_width == MMC_BUS_WIDTH_8) {
			tuning_blk_pattern = tuning_blk_pattern_8bit;
			*blksz = DW_MMC_HS200_TUNING_BLKSZ_8_BIT;
		} else if (mmc->ios.bus_width == MMC_BUS_WIDTH_4) {
			tuning_blk_pattern = tuning_blk_pattern_4bit;
			*blksz = DW_MMC_HS200_TUNING_BLKSZ_4_BIT;
		} else {
			return -EINVAL;
		}
	} else if (opcode == MMC_SEND_TUNING_BLOCK) {
		tuning_blk_pattern = tuning_blk_pattern_4bit;
		*blksz = DW_MMC_TUNING_BLKSZ;
	} else if (opcode == MMC_READ_SINGLE_BLOCK) {
		if (id == 0) /* emmc ddr50 */
			*arg = EMMC_PATTERN_ADDRESS;
		*blksz = DW_MMC_READ_SINGLE_TUNING_BLKSZ;
	} else if (opcode == SD_IO_RW_EXTENDED) {
		*arg = 0x200004; /* set Byte/block count and register address */
		*flags = MMC_RSP_SPI_R5 | MMC_RSP_R5 | MMC_CMD_ADTC;
		*blksz = DW_MMC_SDIO_RW_TUNING_BLKSZ;
	} else {
		dev_err(&mmc->class_dev, "Undefined command %u for tuning\n", opcode);
		return -EINVAL;
	}

	return 0;
}

static int dw_mci_send_tuning(struct dw_mci_slot *slot, u32 opcode, int blksz,
	u8 *tuning_blk, u32 arg, unsigned int flags)
{
	struct mmc_host *mmc = slot->mmc;
	struct dw_mci *host = slot->host;
	const struct dw_mci_drv_data *drv_data = host->drv_data;
	unsigned int tuning_loop = MAX_TUNING_LOOP;
	int ret;
	struct mmc_request mrq = {0};
	struct mmc_command cmd = {0};
	struct mmc_data data = {0};
	struct scatterlist sg;

	do {
		cmd.opcode = opcode;
		cmd.arg = arg;
		cmd.flags = flags;

		data.blksz = blksz;
		data.blocks = 1;
		data.flags = MMC_DATA_READ;
		data.sg = &sg;
		data.sg_len = 1;

		sg_init_one(&sg, tuning_blk, blksz);
		dw_mci_set_timeout(host);

		mrq.cmd = &cmd;
		mrq.stop = NULL;
		mrq.data = &data;

		ret = drv_data->tuning_find_condition(host, mmc->ios.timing);
		if (ret == -1) {
			if ((host->hw_mmc_id == DWMMC_SD_ID) &&
				(drv_data->slowdown_clk)) {
				ret = drv_data->slowdown_clk(host, mmc->ios.timing);
				if (ret)
					break;
			} else {
				break;
			}
		} else if (!ret) {
			break;
		}
#ifdef CONFIG_SDIO_HI_CLOCK_RETUNING
		if (ret == TUNING_ERR_ALL_PHASE_PASS)
			break;
#endif

		mmc_wait_for_req(mmc, &mrq);

		if (!cmd.error && !data.error) {
			drv_data->tuning_set_current_state(host, 1);
		} else {
			drv_data->tuning_set_current_state(host, 0);
			dev_dbg(&mmc->class_dev,
				"Tuning error: cmd.error:%d, data.error:%d\n",
				cmd.error, data.error);
		}
	} while (tuning_loop--);

	return ret;
}

int dw_mci_priv_execute_tuning_helper(struct dw_mci_slot *slot, u32 opcode,
	struct dw_mci_tuning_data *tuning_data)
{
#ifdef CONFIG_SDIO_HI_CLOCK_RETUNING
	int retry_num = 1;
#endif
	struct mmc_host *mmc = slot->mmc;
	struct dw_mci *host = slot->host;
	int ret;
	u8 *tuning_blk = NULL;
	int blksz;
	u32 arg = 0;
	unsigned int flags = MMC_RSP_R1 | MMC_CMD_ADTC;

#ifdef CONFIG_SDIO_HI_CLOCK_RETUNING
retry:
	arg = 0;
	ret;
	flags = MMC_RSP_R1 | MMC_CMD_ADTC;
#endif

	ret = dw_mci_tuning_prepare(slot, opcode, &blksz, &arg, &flags);
	if (ret)
		return -EINVAL;

	tuning_blk = kzalloc(blksz, GFP_KERNEL);
	if (!tuning_blk)
		return -ENOMEM;

	if ((!host->drv_data->tuning_find_condition) ||
		(!host->drv_data->tuning_set_current_state)) {
		dev_err(&slot->mmc->class_dev, "no tuning find condition method\n");
		goto out;
	}

	pm_runtime_get_sync(mmc_dev(slot->mmc));

	host->flags |= DWMMC_IN_TUNING;
	host->flags &= ~DWMMC_TUNING_DONE;

	ret = dw_mci_send_tuning(slot, opcode, blksz, tuning_blk, arg, flags);

	host->flags &= ~DWMMC_IN_TUNING;
	if (!ret)
		host->flags |= DWMMC_TUNING_DONE;

	host->tuning_move_start = 1;
out:
	kfree(tuning_blk);

	pm_runtime_mark_last_busy(mmc_dev(mmc));
	pm_runtime_put_autosuspend(mmc_dev(mmc));

#ifdef CONFIG_SDIO_HI_CLOCK_RETUNING
	if ((ret == TUNING_ERR_ALL_PHASE_PASS) && retry_num) {
		retry_num = 0;
		goto retry;
	}
#endif

	return ret;
}

void dw_mci_reg_dump_status_summary(struct dw_mci *host)
{
	u32 status, mintsts;

	dev_err(host->dev, ": ============== STATUS DUMP ================\n");
	dev_err(host->dev, ": cmd_status:      0x%08x\n", host->cmd_status);
	dev_err(host->dev, ": data_status:     0x%08x\n", host->data_status);
	dev_err(host->dev, ": pending_events:  0x%08lx\n", host->pending_events);
	dev_err(host->dev, ": completed_events:0x%08lx\n", host->completed_events);
	dev_err(host->dev, ": state:           %d\n", host->state);
	dev_err(host->dev, ": ===========================================\n");

	/* summary */
	mintsts = mci_readl(host, MINTSTS);
	status = mci_readl(host, STATUS);

	/*
	 * 0x8:if true means data transfer done;
	 * 0x4: command done;
	 * 0x1 << 9: data0 busy.
	 */
	dev_err(host->dev, "CMD%d, ARG=0x%08x, intsts : %s, status : %s.\n",
		mci_readl(host, CMD) & 0x3F, mci_readl(host, CMDARG),
		mintsts & 0x8 ? "Data transfer done" : mintsts & 0x4 ? "Command Done" : "refer to dump",
		status & (0x1 << 9) ? "dat0 busy" : "refer to dump");
	dev_err(host->dev, ": RESP0:	0x%08x\n", mci_readl(host, RESP0));
	dev_err(host->dev, ": RESP1:	0x%08x\n", mci_readl(host, RESP1));
	dev_err(host->dev, ": RESP2:	0x%08x\n", mci_readl(host, RESP2));
	dev_err(host->dev, ": RESP3:	0x%08x\n", mci_readl(host, RESP3));
	dev_err(host->dev, ": host : cmd_status=0x%08x, data_status=0x%08x.\n",
		host->cmd_status, host->data_status);
	dev_err(host->dev, ": host : pending_events=0x%08lx, completed_events=0x%08lx.\n",
		host->pending_events, host->completed_events);
	dw_mci_dump_crg(host);
	dev_err(host->dev, ": ===========================================\n");
}

void dw_mci_reg_dump(struct dw_mci *host)
{
	struct dw_mci_hs_priv_data *priv = host->priv;

	dev_err(host->dev, ": ============== REGISTER DUMP ==============\n");
	dev_err(host->dev, ": CTRL:	0x%08x\n", mci_readl(host, CTRL));
	dev_err(host->dev, ": PWREN:	0x%08x\n", mci_readl(host, PWREN));
	dev_err(host->dev, ": CLKDIV:	0x%08x\n", mci_readl(host, CLKDIV));
	dev_err(host->dev, ": CLKSRC:	0x%08x\n", mci_readl(host, CLKSRC));
	dev_err(host->dev, ": CLKENA:	0x%08x\n", mci_readl(host, CLKENA));
	dev_err(host->dev, ": TMOUT:	0x%08x\n", mci_readl(host, TMOUT));
	dev_err(host->dev, ": CTYPE:	0x%08x\n", mci_readl(host, CTYPE));
	dev_err(host->dev, ": BLKSIZ:	0x%08x\n", mci_readl(host, BLKSIZ));
	dev_err(host->dev, ": BYTCNT:	0x%08x\n", mci_readl(host, BYTCNT));
	dev_err(host->dev, ": INTMSK:	0x%08x\n", mci_readl(host, INTMASK));
	dev_err(host->dev, ": CMDARG:	0x%08x\n", mci_readl(host, CMDARG));
	dev_err(host->dev, ": CMD:	0x%08x\n", mci_readl(host, CMD));
	dev_err(host->dev, ": MINTSTS:	0x%08x\n", mci_readl(host, MINTSTS));
	dev_err(host->dev, ": RINTSTS:	0x%08x\n", mci_readl(host, RINTSTS));
	dev_err(host->dev, ": STATUS:	0x%08x\n", mci_readl(host, STATUS));
	dev_err(host->dev, ": FIFOTH:	0x%08x\n", mci_readl(host, FIFOTH));
	dev_err(host->dev, ": CDETECT:	0x%08x\n", mci_readl(host, CDETECT));
	dev_err(host->dev, ": WRTPRT:	0x%08x\n", mci_readl(host, WRTPRT));
	dev_err(host->dev, ": GPIO:	0x%08x\n", mci_readl(host, GPIO));
	dev_err(host->dev, ": TCBCNT:	0x%08x\n", mci_readl(host, TCBCNT));
	dev_err(host->dev, ": TBBCNT:	0x%08x\n", mci_readl(host, TBBCNT));
	dev_err(host->dev, ": DEBNCE:	0x%08x\n", mci_readl(host, DEBNCE));
	dev_err(host->dev, ": USRID:	0x%08x\n", mci_readl(host, USRID));
	dev_err(host->dev, ": VERID:	0x%08x\n", mci_readl(host, VERID));
	dev_err(host->dev, ": HCON:	0x%08x\n", mci_readl(host, HCON));
	dev_err(host->dev, ": UHS_REG:	0x%08x\n", mci_readl(host, UHS_REG));
	dev_err(host->dev, ": BMOD:	0x%08x\n", mci_readl(host, BMOD));
	dev_err(host->dev, ": PLDMND:	0x%08x\n", mci_readl(host, PLDMND));
	if (host->dma_64bit_address == SDMMC_32_BIT_DMA) {
		dev_err(host->dev, ": DBADDR:	0x%08x\n", mci_readl(host, DBADDR));
		dev_err(host->dev, ": IDSTS:    0x%08x\n", mci_readl(host, IDSTS));
		dev_err(host->dev, ": IDINTEN:	0x%08x\n", mci_readl(host, IDINTEN));
		dev_err(host->dev, ": DSCADDR:	0x%08x\n", mci_readl(host, DSCADDR));
		dev_err(host->dev, ": BUFADDR:	0x%08x\n", mci_readl(host, BUFADDR));
	} else {
		dev_err(host->dev, ": DBADDRL:	0x%08x\n", mci_readl(host, DBADDRL));
		dev_err(host->dev, ": DBADDRU:	0x%08x\n", mci_readl(host, DBADDRU));
		dev_err(host->dev, ": IDSTS:    0x%08x\n", mci_readl(host, IDSTS64));
		dev_err(host->dev, ": IDINTEN:	0x%08x\n", mci_readl(host, IDINTEN64));
	}
	dev_err(host->dev, ": CDTHRCTL:         0x%08x\n", mci_readl(host, CDTHRCTL));
	dev_err(host->dev, ": UHS_REG_EXT:	0x%08x\n", mci_readl(host, UHS_REG_EXT));

	if (priv->print_reg == 1) {
		dev_err(host->dev, ": sctrl PER 16: 0x%08x\n", readl(sys_base + SCTRL_SCPERSTATUS16_ADDR));
		dev_err(host->dev, ": sctrl PER 17: 0x%08x\n", readl(sys_base + SCTRL_SCPERSTATUS17_ADDR));
		dev_err(host->dev, ": sctrl CLK DIV 10: 0x%08x\n", readl(sys_base + SCTRL_SCCLKDIV10_ADDR));
		dev_err(host->dev, ": sctrl CLK DIV 11: 0x%08x\n", readl(sys_base + SCTRL_SCCLKDIV11_ADDR));
		dev_err(host->dev, ": sctrl AUTO DIV: 0x%08x\n", readl(sys_base + SCTRL_SCMMC_AUTODIV_ADDR));
		dev_err(host->dev, ": sctrl CLK DIV 0: 0x%08x\n", readl(sys_base + SCTRL_SCCLKDIV0_ADDR));
		dev_err(host->dev, ": sctrl PPLL0 CTRL0: 0x%08x\n", readl(sys_base + SCTRL_SCPPLLCTRL0_ADDR));
		dev_err(host->dev, ": sctrl PPLL0 CTRL1: 0x%08x\n", readl(sys_base + SCTRL_SCPPLLCTRL1_ADDR));
		dev_err(host->dev, ": sctrl PPLL0 SSC: 0x%08x\n", readl(sys_base + SCTRL_SCPPLLSSCCTRL_ADDR));
		dev_err(host->dev, ": sctrl PPLL0 STAT: 0x%08x\n", readl(sys_base + SCTRL_SCPPLLSTAT_ADDR));
		dev_err(host->dev, ": sctrl PER STAT2: 0x%08x\n", readl(sys_base + SCTRL_SCPERSTAT2_ADDR));
		dev_err(host->dev, ": pctrl STAT2: 0x%08x\n", readl(pctrl_base + PCTRL_PERI_STAT2_ADDR));
		dev_err(host->dev, ": buck0 VSET: 0x%08x\n", hisi_pmic_reg_read(BUCK0_VSET_ADDR));
		dev_err(host->dev, ": buck0 VSET ECO: 0x%08x\n", hisi_pmic_reg_read(BUCK0_VSET_ECO_ADDR));
		dev_err(host->dev, ": LDO30: 0x%08x\n", hisi_pmic_reg_read(LDO30_VSET_ADDR));
	}
	dw_mci_reg_dump_status_summary(host);
}

#ifdef CONFIG_MMC_HISI_TRACE
void dw_mci_reg_dump_summary_fortrace(struct mmc_host *mmc)
{
	/* summary */
	mintsts = mci_readl(host, MINTSTS);
	status = mci_readl(host, STATUS);

	/*
	 * 0x8:if true means data transfer done;
	 * 0x4: command done;
	 * 0x1 << 9: data0 busy.
	 */
	mmc_trace_comm_record(mmc,
		"CMD%d, ARG=0x%x, intsts : %s, status : %s.\n",
		mci_readl(host, CMD) & 0x3F, mci_readl(host, CMDARG),
		mintsts & 0x8 ? "Data transfer done" : mintsts & 0x4 ? "Command Done" : "refer to dump",
		status & (0x1 << 9) ? "dat0 busy" : "refer to dump");
	mmc_trace_comm_record(mmc, ": RESP0:	0x%x\n", mci_readl(host, RESP0));
	mmc_trace_comm_record(mmc, ": RESP1:	0x%x\n", mci_readl(host, RESP1));
	mmc_trace_comm_record(mmc, ": RESP2:	0x%x\n", mci_readl(host, RESP2));
	mmc_trace_comm_record(mmc, ": RESP3:	0x%x\n", mci_readl(host, RESP3));
	mmc_trace_comm_record(mmc, ": host : cmd_status=0x%x, data_status=0x%x\n",
		host->cmd_status, host->data_status);
	mmc_trace_comm_record(mmc, ": host : pending_events=0x%x, completed_events=0x%x\n",
		host->pending_events, host->completed_events);
}

void dw_mci_reg_dump_fortrace(struct mmc_host *mmc)
{
	u32 status, mintsts;
	struct dw_mci_slot *slot = mmc_priv(mmc);
	struct dw_mci *host = slot->host;

	pm_runtime_get_sync(mmc_dev(mmc));
	mmc_trace_comm_record(mmc, ": =========== REGISTER DUMP (%s)===========\n", mmc_hostname(mmc));
	mmc_trace_comm_record(mmc, ": CTRL:	0x%x\n", mci_readl(host, CTRL));
	mmc_trace_comm_record(mmc, ": PWREN:	0x%x\n", mci_readl(host, PWREN));
	mmc_trace_comm_record(mmc, ": CLKDIV:	0x%x\n", mci_readl(host, CLKDIV));
	mmc_trace_comm_record(mmc, ": CLKSRC:	0x%x\n", mci_readl(host, CLKSRC));
	mmc_trace_comm_record(mmc, ": CLKENA:	0x%x\n", mci_readl(host, CLKENA));
	mmc_trace_comm_record(mmc, ": TMOUT:	0x%x\n", mci_readl(host, TMOUT));
	mmc_trace_comm_record(mmc, ": CTYPE:	0x%x\n", mci_readl(host, CTYPE));
	mmc_trace_comm_record(mmc, ": BLKSIZ:	0x%x\n", mci_readl(host, BLKSIZ));
	mmc_trace_comm_record(mmc, ": BYTCNT:	0x%x\n", mci_readl(host, BYTCNT));
	mmc_trace_comm_record(mmc, ": INTMSK:	0x%x\n", mci_readl(host, INTMASK));
	mmc_trace_comm_record(mmc, ": CMDARG:	0x%x\n", mci_readl(host, CMDARG));
	mmc_trace_comm_record(mmc, ": CMD:	0x%x\n", mci_readl(host, CMD));
	mmc_trace_comm_record(mmc, ": MINTSTS:	0x%x\n", mci_readl(host, MINTSTS));
	mmc_trace_comm_record(mmc, ": RINTSTS:	0x%x\n", mci_readl(host, RINTSTS));
	mmc_trace_comm_record(mmc, ": STATUS:	0x%x\n", mci_readl(host, STATUS));
	mmc_trace_comm_record(mmc, ": FIFOTH:	0x%x\n", mci_readl(host, FIFOTH));
	mmc_trace_comm_record(mmc, ": CDETECT:	0x%x\n", mci_readl(host, CDETECT));
	mmc_trace_comm_record(mmc, ": WRTPRT:	0x%x\n", mci_readl(host, WRTPRT));
	mmc_trace_comm_record(mmc, ": GPIO:	0x%x\n", mci_readl(host, GPIO));
	mmc_trace_comm_record(mmc, ": TCBCNT:	0x%x\n", mci_readl(host, TCBCNT));
	mmc_trace_comm_record(mmc, ": TBBCNT:	0x%x\n", mci_readl(host, TBBCNT));
	mmc_trace_comm_record(mmc, ": DEBNCE:	0x%x\n", mci_readl(host, DEBNCE));
	mmc_trace_comm_record(mmc, ": USRID:	0x%x\n", mci_readl(host, USRID));
	mmc_trace_comm_record(mmc, ": VERID:	0x%x\n", mci_readl(host, VERID));
	mmc_trace_comm_record(mmc, ": HCON:	0x%x\n", mci_readl(host, HCON));
	mmc_trace_comm_record(mmc, ": UHS_REG:	0x%x\n", mci_readl(host, UHS_REG));
	mmc_trace_comm_record(mmc, ": BMOD:	0x%08x\n", mci_readl(host, BMOD));
	mmc_trace_comm_record(mmc, ": PLDMND:	0x%x\n", mci_readl(host, PLDMND));
	mmc_trace_comm_record(mmc, ": DBADDR:	0x%x\n", mci_readl(host, DBADDR));
	mmc_trace_comm_record(mmc, ": IDSTS:	0x%x\n", mci_readl(host, IDSTS));
	mmc_trace_comm_record(mmc, ": IDINTEN:	0x%x\n", mci_readl(host, IDINTEN));
	mmc_trace_comm_record(mmc, ": DSCADDR:	0x%x\n", mci_readl(host, DSCADDR));
	mmc_trace_comm_record(mmc, ": BUFADDR:	0x%x\n", mci_readl(host, BUFADDR));
	mmc_trace_comm_record(mmc, ": CDTHRCTL: 0x%x\n", mci_readl(host, CDTHRCTL));
	mmc_trace_comm_record(mmc, ": UHS_REG_EXT: 0x%x\n", mci_readl(host, UHS_REG_EXT));
	mmc_trace_comm_record(mmc, ": cmd_status:      0x%x\n", host->cmd_status);
	mmc_trace_comm_record(mmc, ": data_status:     0x%x\n", host->data_status);
	mmc_trace_comm_record(mmc, ": pending_events:  0x%x\n", host->pending_events);
	mmc_trace_comm_record(mmc, ": completed_events:0x%x\n", host->completed_events);
	mmc_trace_comm_record(mmc, ": state:           %d\n", host->state);

	dw_mci_reg_dump_summary_fortrace(mmc);

	pm_runtime_mark_last_busy(mmc_dev(mmc));
	pm_runtime_put_autosuspend(mmc_dev(mmc));
}
#endif

bool dw_mci_stop_abort_cmd(struct mmc_command *cmd)
{
	u32 op = cmd->opcode;

	/* cmd->arg >> 9: check [25:9]:register address; */
	if ((op == MMC_STOP_TRANSMISSION) || (op == MMC_GO_IDLE_STATE) ||
		(op == MMC_GO_INACTIVE_STATE) ||
		((op == SD_IO_RW_DIRECT) && (cmd->arg & DW_MMC_SET_WRITE_FLAG) &&
			((cmd->arg >> 9) & 0x1FFFF) == SDIO_CCCR_ABORT))
		return true;
	return false;
}

bool dw_mci_wait_reset(struct device *dev, struct dw_mci *host, unsigned int reset_val)
{
	unsigned long timeout = jiffies + msecs_to_jiffies(50);
	unsigned int ctrl;

	ctrl = mci_readl(host, CTRL);
	ctrl |= reset_val;
	mci_writel(host, CTRL, ctrl);

	/* wait till resets clear */
	do {
		if (!(mci_readl(host, CTRL) & reset_val))
			return true;
	} while (time_before(jiffies, timeout));

	dev_err(dev, "Timeout resetting block (ctrl %#x)\n", ctrl);

	return false;
}

int mci_send_cmd(struct dw_mci_slot *slot, u32 cmd, u32 arg)
{
	struct dw_mci *host = slot->host;
	unsigned long timeout = jiffies + msecs_to_jiffies(100);
	unsigned int cmd_status = 0;

	mci_writel(host, CMDARG, arg);
	wmb(); /* Optimization barrier */
	mci_writel(host, CMD, SDMMC_CMD_START | cmd);
	while (time_before(jiffies, timeout)) {
		cmd_status = mci_readl(host, CMD);
		if (!(cmd_status & SDMMC_CMD_START))
			return 0;
	}

	if (!dw_mci_wait_reset(host->dev, host, SDMMC_CTRL_RESET))
		return 1;

	timeout = jiffies + msecs_to_jiffies(100);
	mci_writel(host, CMD, SDMMC_CMD_START | cmd);
	while (time_before(jiffies, timeout)) {
		cmd_status = mci_readl(host, CMD);
		if (!(cmd_status & SDMMC_CMD_START))
			return 0;
	}

	dev_err(&slot->mmc->class_dev,
		"Timeout sending command (cmd %#x arg %#x status %#x)\n", cmd,
		arg, cmd_status);

	return 1;
}

void dw_mci_ciu_reset(struct device *dev, struct dw_mci *host)
{
	struct dw_mci_slot *slot = host->cur_slot;

	if (slot) {
		if (!dw_mci_wait_reset(dev, host, SDMMC_CTRL_RESET))
			dev_info(dev, "dw_mci_wait_reset failed\n");

		(void)mci_send_cmd(
			slot, SDMMC_CMD_UPD_CLK | SDMMC_CMD_PRV_DAT_WAIT, 0);
	}
}

bool dw_mci_fifo_reset(struct device *dev, struct dw_mci *host)
{
	unsigned long timeout = jiffies + msecs_to_jiffies(1000);
	unsigned int ctrl;
	bool result = false;

	do {
		result = dw_mci_wait_reset(host->dev, host, SDMMC_CTRL_FIFO_RESET);
		if (!result)
			break;

		ctrl = mci_readl(host, STATUS);
		if (!(ctrl & SDMMC_STATUS_DMA_REQ)) {
			result = dw_mci_wait_reset(host->dev, host, SDMMC_CTRL_FIFO_RESET);
			if (result) {
				/*
				 * clear exception raw interrupts can not be
				 * handled exfifo full => RXDR interrupt rising
				 */
				ctrl = mci_readl(host, RINTSTS);
				ctrl = ctrl & ~(mci_readl(host, MINTSTS));
				if (ctrl)
					mci_writel(host, RINTSTS, ctrl);

				return true;
			}
		}
	} while (time_before(jiffies, timeout));

	dev_err(dev, "%s: Timeout while resetting host controller after err\n", __func__);

	return false;
}

u32 dw_mci_prep_stop(struct dw_mci *host, struct mmc_command *cmd)
{
	struct mmc_command *stop = &host->stop;
	const struct dw_mci_drv_data *drv_data = host->drv_data;
	u32 cmdr = cmd->opcode;

	memset(stop, 0, sizeof(struct mmc_command)); /* unsafe_function_ignore: memset */

	if (cmdr == SD_IO_RW_EXTENDED) {
		stop->opcode = SD_IO_RW_DIRECT;
		stop->arg = DW_MMC_SET_WRITE_FLAG;
		stop->arg |= (cmd->arg >> 28) & 0x7; /* Function number */
		stop->arg |= SDIO_CCCR_ABORT << 9; /* Register address */
		stop->flags = MMC_RSP_SPI_R5 | MMC_RSP_R5 | MMC_CMD_AC;
	} else {
		stop->opcode = MMC_STOP_TRANSMISSION;
		stop->arg = 0;
		stop->flags = MMC_RSP_R1B | MMC_CMD_AC;
	}

	cmdr = stop->opcode | SDMMC_CMD_STOP | SDMMC_CMD_RESP_CRC |
	       SDMMC_CMD_RESP_EXP;

	/* Use hold bit register */
	if (drv_data && drv_data->prepare_command)
		drv_data->prepare_command(host, &cmdr);

	return cmdr;
}

bool dw_mci_wait_data_busy(struct dw_mci *host, struct mmc_request *mrq)
{
	u32 status;
	unsigned long timeout = jiffies + msecs_to_jiffies(500);

	do {
		status = mci_readl(host, STATUS);
		if (!(status & SDMMC_STATUS_BUSY))
			return true;

		usleep_range(10, 20);
	} while (time_before(jiffies, timeout));

	/* card is checked every 1s by CMD13 at least */
	if (mrq->cmd->opcode == MMC_SEND_STATUS)
		return true;

	dev_info(host->dev, "status is busy, reset ctrl\n");

	if (!dw_mci_wait_reset(host->dev, host, SDMMC_CTRL_RESET))
		return false;

	/* After CTRL Reset, Should be needed clk val to CIU */
	if (host->cur_slot)
		(void)mci_send_cmd(host->cur_slot,
			SDMMC_CMD_UPD_CLK | SDMMC_CMD_PRV_DAT_WAIT, 0);

	timeout = jiffies + msecs_to_jiffies(500);
	do {
		status = mci_readl(host, STATUS);
		if (!(status & SDMMC_STATUS_BUSY))
			return true;

		usleep_range(10, 20);
	} while (time_before(jiffies, timeout));

	dev_err(host->dev, "Data[0]: data is busy\n");

	return false;
}

int dw_mci_start_signal_voltage_switch(
	struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct dw_mci_slot *slot = mmc_priv(mmc);
	struct dw_mci *host = slot->host;
	const struct dw_mci_drv_data *drv_data = host->drv_data;
	int err = -ENOSYS;

	if (drv_data && drv_data->start_signal_voltage_switch)
		err = drv_data->start_signal_voltage_switch(mmc, ios);

	return err;
}

void dw_mci_slowdown_clk(struct mmc_host *mmc, int timing)
{
	struct dw_mci_slot *slot = mmc_priv(mmc);
	struct dw_mci *host = slot->host;
	const struct dw_mci_drv_data *drv_data = host->drv_data;

	if (host->flags & DWMMC_TUNING_DONE)
		host->flags &= ~DWMMC_TUNING_DONE;

	if (drv_data->slowdown_clk) {
		if (host->sd_reinit)
			return;
		host->sd_reinit = 1;
		pm_runtime_get_sync(mmc_dev(mmc));
		drv_data->slowdown_clk(host, timing);
		pm_runtime_mark_last_busy(mmc_dev(mmc));
		pm_runtime_put_autosuspend(mmc_dev(mmc));
	}
}

#ifdef CONFIG_SD_TIMEOUT_RESET
static void dw_mci_set_clk_peri_08v(struct dw_mci *host)
{
	/* When sd data transmission error,volt peri volt to 0.8v in libra */
	if ((host->hw_mmc_id == DWMMC_SD_ID) &&
		!IS_ERR(host->volt_hold_sd_clk) &&
		(host->volt_hold_clk_sd != VOLT_HOLD_CLK_08V) &&
		(host->cur_slot->mmc->ios.timing == MMC_TIMING_UHS_SDR104)) {
		host->volt_hold_clk_sd = VOLT_HOLD_CLK_08V;
		host->set_sd_data_tras_timeout = VOLT_TO_1S;
		/* dw_mci_work_volt_mmc_func */
		queue_work(host->card_workqueue, &host->work_volt_mmc);
	}

	if ((host->hw_mmc_id == DWMMC_SDIO_ID) &&
		!IS_ERR(host->volt_hold_sdio_clk) &&
		(host->volt_hold_clk_sdio != VOLT_HOLD_CLK_08V) &&
		(host->cur_slot->mmc->ios.timing == MMC_TIMING_UHS_SDR104)) {
		host->volt_hold_clk_sdio = VOLT_HOLD_CLK_08V;
		host->set_sdio_data_tras_timeout = VOLT_TO_1S;
		queue_work(host->card_workqueue, &host->work_volt_mmc);
	}
}
#endif

void dw_mci_timeout_timer(unsigned long data)
{
	struct dw_mci *host = (struct dw_mci *)data;
	struct mmc_request *mrq = NULL;

	if (!host)
		return;

	spin_lock(&host->lock);
	if (host->mrq) {
		mrq = host->mrq;
		dev_vdbg(host->dev, "time out host->mrq = %pK\n", host->mrq);

		dev_err(host->dev, "Timeout waiting for hardware interrupt. state = %d\n", host->state);
#ifdef CONFIG_HUAWEI_DSM
		if (host->hw_mmc_id == DWMMC_SDIO_ID)
			dsm_wifi_client_notify(DSM_SDIO_DATA_TIMEOUT,
				"sdio data timeout state is %d\n", host->state);
#endif
		dw_mci_reg_dump(host);

#ifdef CONFIG_SD_TIMEOUT_RESET
		dw_mci_set_clk_peri_08v(host);
#endif

		host->sg = NULL;
		host->data = NULL;
		host->cmd = NULL;

		switch (host->state) {
		case STATE_IDLE:
			break;
		case STATE_SENDING_CMD:
			mrq->cmd->error = -ENOMEDIUM;
			if (!mrq->data)
				break;
		/* fall through */
		case STATE_SENDING_DATA:
			mrq->data->error = -ENOMEDIUM;
			dw_mci_stop_dma(host);
			break;
		case STATE_DATA_BUSY:
		case STATE_DATA_ERROR:
			if (mrq->data->error == -EINPROGRESS)
				mrq->data->error = -ENOMEDIUM;
		/* fall through */
		case STATE_SENDING_STOP:
			if (mrq->stop)
				mrq->stop->error = -ENOMEDIUM;
			break;
		default:
			break;
		}

		host->sd_hw_timeout = 1;

		dw_mci_request_end(host, mrq);
	}
	spin_unlock(&host->lock);

	if (host->flags & TIMEOUT_ENABLE_PIO) {
		host->use_dma = TRANS_MODE_PIO;
		pr_err("%s dma err, use pio instead\n", __func__);
	}
}

void dw_mci_clean_queue(struct dw_mci *host, struct dw_mci_slot *slot)
{
	struct mmc_request *mrq = slot->mrq;

	if (mrq) {
		if (mrq == host->mrq) {
			host->data = NULL;
			host->cmd = NULL;

			switch (host->state) {
			case STATE_IDLE:
				break;
			case STATE_SENDING_CMD:
				mrq->cmd->error = -ENOMEDIUM;
				if (!mrq->data)
					break;
			/* fall through */
			case STATE_SENDING_DATA:
				mrq->data->error = -ENOMEDIUM;
				dw_mci_stop_dma(host);
				break;
			case STATE_DATA_BUSY:
			case STATE_DATA_ERROR:
				if (mrq->data->error == -EINPROGRESS)
					mrq->data->error = -ENOMEDIUM;
				if (!mrq->stop)
					break;
			/* fall through */
			case STATE_SENDING_STOP:
				if (mrq->stop)
					mrq->stop->error = -ENOMEDIUM;
				break;
			default:
				break;
			}

			dw_mci_request_end(host, mrq);
		} else {
			list_del(&slot->queue_node);
			mrq->cmd->error = -ENOMEDIUM;
			if (mrq->data)
				mrq->data->error = -ENOMEDIUM;
			if (mrq->stop)
				mrq->stop->error = -ENOMEDIUM;

#ifdef CONFIG_HUAWEI_EMMC_DSM
			if (host->hw_mmc_id == DWMMC_EMMC_ID)
				if (!del_timer(&host->rw_to_timer))
					dev_info(host->dev, "inactive timer\n");
#endif
			if (del_timer(&host->timer))
				dev_info(host->dev, "del_timer failed\n");
			spin_unlock(&host->lock);
			mmc_request_done(slot->mmc, mrq);
			spin_lock(&host->lock);
		}
	}
}

void dw_mci_work_routine_card(struct work_struct *work)
{
	struct dw_mci *host = container_of(work, struct dw_mci, card_work);
	unsigned int i;
	struct dw_mci_slot *slot = NULL;
	struct mmc_host *mmc = NULL;
	int present;

	for (i = 0; i < host->num_slots; i++) {
		slot = host->slot[i];
		mmc = slot->mmc;

		present = dw_mci_get_cd(mmc);
		while (present != slot->last_detect_state) {
			dev_err(&slot->mmc->class_dev, "card %s\n",
				present ? "inserted" : "removed");

			spin_lock_bh(&host->lock);

			/* Card change detected */
			slot->last_detect_state = present;

			/* Mark card as present if applicable */
			if (present)
				set_bit(DW_MMC_CARD_PRESENT, &slot->flags);

			dw_mci_clean_queue(host, slot);

			/* Power down slot */
			if (!present) {
				clear_bit(DW_MMC_CARD_PRESENT, &slot->flags);

				/*
				 * Clear down the FIFO - doing so generates a
				 * block interrupt, hence setting the
				 * scatter-gather pointer to NULL.
				 */
				sg_miter_stop(&host->sg_miter);
				host->sg = NULL;

				dw_mci_fifo_reset(host->dev, host);
				dw_mci_ciu_reset(host->dev, host);
#ifdef CONFIG_MMC_DW_IDMAC
				dw_mci_idmac_reset(host);
#endif
			}

			spin_unlock_bh(&host->lock);
			present = dw_mci_get_cd(mmc);
		}

		mmc_detect_change(slot->mmc,
			msecs_to_jiffies(host->pdata->detect_delay_ms));
	}
}

bool mci_wait_reset(struct device *dev, struct dw_mci *host)
{
	unsigned long timeout = jiffies + msecs_to_jiffies(50);
	unsigned int ctrl;
	unsigned int mask;

	mask = SDMMC_CTRL_RESET | SDMMC_CTRL_FIFO_RESET | SDMMC_CTRL_DMA_RESET;
	mci_writel(host, CTRL, mask);

	/* wait till resets clear */
	do {
		ctrl = mci_readl(host, CTRL);
		if (!(ctrl & mask))
			return true;
	} while (time_before(jiffies, timeout));

	dev_err(dev, "Timeout resetting block (ctrl %#x)\n", ctrl);

	return false;
}

static bool mci_hi3xxx_wait_reset(struct dw_mci *host)
{
	unsigned long timeout = jiffies + msecs_to_jiffies(50);
	unsigned int ctrl;
	unsigned int mask;

	mask = SDMMC_CTRL_RESET | SDMMC_CTRL_FIFO_RESET | SDMMC_CTRL_DMA_RESET;
	mci_writel(host, CTRL, mask);

	/* wait till resets clear */
	do {
		ctrl = mci_readl(host, CTRL);
		if (!(ctrl & mask))
			return true;
	} while (time_before(jiffies, timeout));

	dev_err(host->dev, "Timeout resetting block (ctrl %#x)\n", ctrl);

	return false;
}

void dw_mci_hi3xxx_reset_restore(struct dw_mci *host)
{
	unsigned int ctype;
	unsigned int clkena, clkdiv;
	unsigned int uhs_reg, uhs_reg_ext;
	unsigned int enable_shift;
	unsigned int gpio;
	unsigned int fifoth;
	unsigned int timeout;
	unsigned int cardthrctrl;

	ctype = mci_readl(host, CTYPE);
	clkena = mci_readl(host, CLKENA);
	clkdiv = mci_readl(host, CLKDIV);
	fifoth = mci_readl(host, FIFOTH);
	timeout = mci_readl(host, TMOUT);
	cardthrctrl = mci_readl(host, CDTHRCTL);
	uhs_reg = mci_readl(host, UHS_REG);
	uhs_reg_ext = mci_readl(host, UHS_REG_EXT);
	enable_shift = mci_readl(host, ENABLE_SHIFT);
	gpio = mci_readl(host, GPIO);

	udelay(20);

	dw_mci_hs_set_rst_m(host, DW_MCI_HS_DO_RESET);
	dw_mci_hs_set_controller(host, DW_MCI_HS_DO_RESET);

	if (!IS_ERR(host->ciu_clk))
		clk_disable_unprepare(host->ciu_clk);

	dw_mci_hs_set_rst_m(host, DW_MCI_HS_DO_UNRESET);

	if (!IS_ERR(host->ciu_clk))
		if (clk_prepare_enable(host->ciu_clk))
			dev_err(host->dev, "ciu_clk clk_prepare_enable failed\n");

	dw_mci_hs_set_controller(host, DW_MCI_HS_DO_UNRESET);
	udelay(20);
	mci_hi3xxx_wait_reset(host);

	mci_writel(host, CTYPE, ctype);
	mci_writel(host, FIFOTH, fifoth);
	mci_writel(host, TMOUT, timeout);
	mci_writel(host, CDTHRCTL, cardthrctrl);
	mci_writel(host, UHS_REG, uhs_reg);
	mci_writel(host, GPIO, 0x0);
	udelay(10);
	mci_writel(host, UHS_REG_EXT, uhs_reg_ext);
	mci_writel(host, ENABLE_SHIFT, enable_shift);
	mci_writel(host, GPIO, gpio | GPIO_CLK_ENABLE);

	dw_mci_hi3xxx_reset_defalut_regist(host, clkena, clkdiv);
}
