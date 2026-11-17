/*
 * utils.c -- da combine v5 codec driver
 *
 * Copyright (c) 2018 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#include "utils.h"
#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
#include "debug.h"
#endif
#include "linux/hisi/audio_log.h"
#include "linux/hisi/da_combine/da_combine_utils.h"
#include "linux/hisi/da_combine/da_combine_v5.h"
#include "linux/hisi/da_combine/da_combine_v5_regs.h"
#include "linux/hisi/da_combine/da_combine_v5_type.h"

struct reg_page {
	unsigned int offest;
	unsigned int begin;
	unsigned int end;
};

static const struct reg_page da_combine_v5_reg_info[] = {
	{ CODEC_BASE_ADDR_PAGE_IO, CODEC_ADDR_PAGE_IO_START, CODEC_ADDR_PAGE_IO_END },
	{ CODEC_BASE_ADDR_PAGE_CFG, CODEC_ADDR_PAGE_CFG_START, CODEC_ADDR_PAGE_CFG_END },
	{ CODEC_BASE_ADDR_PAGE_ANA, ADDR_ANA_OFFSET_START, ADDR_ANA_OFFSET_END },
	{ CODEC_BASE_ADDR_PAGE_DIG, ADDR_DIG_OFFSET_START, ADDR_DIG_OFFSET_END },

	{ BASE_ADDR_PAGE_VIRTUAL, VIR_UP, VIR_CNT - 1 },
	{ ADDR_OCRAM_BASE, ADDR_OCRAM_START, ADDR_OCRAM_END },
	{ ADDR_TCM_BASE, ADDR_TCM_START, ADDR_TCM_END },

	{ BASE_ADDR_PAGE_WDOG, ADDR_WDOG_OFFSET_START, ADDR_WDOG_OFFSET_END },
	{ BASE_ADDR_PAGE_SCTRL, ADDR_SCTRL_OFFSET_START, ADDR_SCTRL_OFFSET_END },
	{ BASE_ADDR_PAGE_UART, ADDR_UART_OFFSET_START, ADDR_UART_OFFSET_END },
	{ BASE_ADDR_PAGE_TIMER0, ADDR_TIMER0_OFFSET_START, ADDR_TIMER0_OFFSET_END },
	{ BASE_ADDR_PAGE_TIMER1, ADDR_TIMER1_OFFSET_START, ADDR_TIMER1_OFFSET_END },

	{ BASE_ADDR_PAGE_GPIO0, ADDR_GPIO0_OFFSET_START, ADDR_GPIO0_OFFSET_END },
	{ BASE_ADDR_PAGE_GPIO1, ADDR_GPIO1_OFFSET_START, ADDR_GPIO1_OFFSET_END },
	{ BASE_ADDR_PAGE_GPIO2, ADDR_GPIO2_OFFSET_START, ADDR_GPIO2_OFFSET_END },

	{ BASE_ADDR_PAGE_TIMER32K, ADDR_TIMER32K_OFFSET_START, ADDR_TIMER32K_OFFSET_END },
	{ BASE_ADDR_PAGE_I2C_MST, ADDR_I2C_MST_OFFSET_START, ADDR_I2C_MST_OFFSET_END },
	{ BASE_ADDR_PAGE_USB, ADDR_USB_OFFSET_START, ADDR_USB_OFFSET_END },
	{ BASE_ADDR_PAGE_EDMA, ADDR_EDMA_OFFSET_START, ADDR_EDMA_OFFSET_END },
	{ BASE_ADDR_PAGE_PLL_TEST, ADDR_PLL_TEST_OFFSET_START, ADDR_PLL_TEST_OFFSET_END },
	{ BASE_ADDR_PAGE_DSPIF, ADDR_DSPIF_OFFSET_START, ADDR_DSPIF_OFFSET_END },
};

void write_reg_array(struct snd_soc_component *codec,
	const struct reg_config *reg_array, size_t len)
{
	unsigned int i;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return;
	}

	if (reg_array == NULL) {
		AUDIO_LOGE("reg array is null");
		return;
	}

	for (i = 0; i < len; i++) {
		if (reg_array[i].update_bits)
			da_combine_update_bits(codec, reg_array[i].addr,
				reg_array[i].mask, reg_array[i].val);
		else
			snd_soc_component_write(codec, reg_array[i].addr,
				reg_array[i].val);
	}
}

void write_reg_seq_array(struct snd_soc_component *codec,
	const struct reg_seq_config *reg_seq_array, size_t len)
{
	unsigned int i;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return;
	}

	if (reg_seq_array == NULL) {
		AUDIO_LOGE("reg array is null");
		return;
	}

	for (i = 0; i < len; i++) {
		if (reg_seq_array[i].cfg.update_bits)
			da_combine_update_bits(codec, reg_seq_array[i].cfg.addr,
				reg_seq_array[i].cfg.mask, reg_seq_array[i].cfg.val);
		else
			snd_soc_component_write(codec, reg_seq_array[i].cfg.addr,
				reg_seq_array[i].cfg.val);

		switch (reg_seq_array[i].type) {
		case RANGE_SLEEP:
			usleep_range(reg_seq_array[i].us,
				reg_seq_array[i].us + reg_seq_array[i].us / 10);
			break;
		case MSLEEP:
			msleep(reg_seq_array[i].us / 1000);
			break;
		case MDELAY:
			mdelay((unsigned long)(reg_seq_array[i].us / 1000));
			break;
		default:
			break;
		}
	}
}

static unsigned int virtual_reg_read(struct da_combine_v5_platform_data *platform_data,
	unsigned int addr)
{
	unsigned int ret = 0;
	unsigned long flag;
	unsigned int *map = platform_data->board_config.mic_map;

	spin_lock_irqsave(&platform_data->v_rw_lock, flag);

	switch (addr) {
	case VIRTUAL_DOWN_REG:
		ret = platform_data->virtual_reg[VIR_UP];
		break;
	case VIRTUAL_UP_REG:
		ret = platform_data->virtual_reg[VIR_DOWN];
		break;
	case VIRTUAL_EXTERN_REG:
		ret = platform_data->virtual_reg[VIR_EXTERN];
		break;
	case VIRTUAL_MAIN_MIC_CALIB_REG:
		ret = platform_data->mic_calib_value[map[MAIN_MIC1]];
		break;
	case VIRTUAL_MAIN_MIC2_CALIB_REG:
		ret = platform_data->mic_calib_value[map[MAIN_MIC2]];
		break;
	case VIRTUAL_SUB_MIC_CALIB_REG:
		ret = platform_data->mic_calib_value[map[SUB_MIC1]];
		break;
	case VIRTUAL_SUB_MIC2_CALIB_REG:
		ret = platform_data->mic_calib_value[map[SUB_MIC2]];
		break;
	case VIRTUAL_PC_DOWN_REG:
		ret = platform_data->virtual_reg[VIR_PC];
		break;
	default:
		AUDIO_LOGE("read failed: virtual reg addr is not existed\n");
		break;
	}

	spin_unlock_irqrestore(&platform_data->v_rw_lock, flag);

	return ret;
}

static bool is_reg_valid(unsigned int reg)
{
	unsigned int val = reg & (~CODEC_BASE_ADDR);
	size_t len = ARRAY_SIZE(da_combine_v5_reg_info);
	unsigned int begin;
	unsigned int end;
	unsigned int i;

	for (i = 0; i < len; i++) {
		begin = da_combine_v5_reg_info[i].begin + da_combine_v5_reg_info[i].offest;
		end = da_combine_v5_reg_info[i].end + da_combine_v5_reg_info[i].offest;
		if (val >= begin && val <= end)
			return true;
	}

	AUDIO_LOGE("invalid reg: 0x%x, begin: 0x%x, end: 0x%x\n",
		val, begin, end);
	return false;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
unsigned int da_combine_v5_reg_read_by_codec(struct snd_soc_codec *codec,
	unsigned int reg)
{
	return da_combine_v5_reg_read(&codec->component, reg);
}

int da_combine_v5_reg_write_by_codec(struct snd_soc_codec *codec,
	unsigned int reg, unsigned int value)
{
	return da_combine_v5_reg_write(&codec->component, reg, value);
}
#endif

unsigned int da_combine_v5_reg_read(struct snd_soc_component *codec,
	unsigned int reg)
{
	unsigned int ret = 0;
	unsigned int reg_mask;
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	if (!is_reg_valid(reg)) {
		AUDIO_LOGE("invalid reg: 0x%x", reg);
		return 0;
	}

	reg_mask = reg & 0xFFFFF000;
	if (reg_mask == BASE_ADDR_PAGE_CFG || reg_mask == BASE_ADDR_PAGE_IO) {
		reg = reg | CODEC_BASE_ADDR;
	} else if (reg_mask == BASE_ADDR_PAGE_VIRTUAL) {
		ret = virtual_reg_read(platform_data, reg);
		return ret;
	}

	if (platform_data->resmgr == NULL) {
		AUDIO_LOGE("platform_data->resmgr is null");
		return 0;
	}

	da_combine_resmgr_request_reg_access(platform_data->resmgr, reg);
	ret = hi_cdcctrl_reg_read(platform_data->cdc_ctrl, reg);
	da_combine_resmgr_release_reg_access(platform_data->resmgr, reg);

	return ret;
}

static void virtual_reg_write(struct da_combine_v5_platform_data *platform_data,
	unsigned int addr, unsigned int value)
{
	unsigned long flag;
	unsigned int *map = platform_data->board_config.mic_map;

	spin_lock_irqsave(&platform_data->v_rw_lock, flag);

	switch (addr) {
	case VIRTUAL_DOWN_REG:
		platform_data->virtual_reg[VIR_UP] = value;
		break;
	case VIRTUAL_UP_REG:
		platform_data->virtual_reg[VIR_DOWN] = value;
		break;
	case VIRTUAL_EXTERN_REG:
		platform_data->virtual_reg[VIR_EXTERN] = value;
		break;
	case VIRTUAL_MAIN_MIC_CALIB_REG:
		platform_data->mic_calib_value[map[MAIN_MIC1]] = value;
		break;
	case VIRTUAL_MAIN_MIC2_CALIB_REG:
		platform_data->mic_calib_value[map[MAIN_MIC2]] = value;
		break;
	case VIRTUAL_SUB_MIC_CALIB_REG:
		platform_data->mic_calib_value[map[SUB_MIC1]] = value;
		break;
	case VIRTUAL_SUB_MIC2_CALIB_REG:
		platform_data->mic_calib_value[map[SUB_MIC2]] = value;
		break;
	case VIRTUAL_PC_DOWN_REG:
		platform_data->virtual_reg[VIR_PC] = value;
		break;
	default:
		AUDIO_LOGE("write failed: virtual reg addr is not existed\n");
		break;
	}

	spin_unlock_irqrestore(&platform_data->v_rw_lock, flag);
}

int da_combine_v5_reg_write(struct snd_soc_component *codec,
	unsigned int reg, unsigned int value)
{
	int ret;
	unsigned int reg_mask;
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	if (!is_reg_valid(reg)) {
		AUDIO_LOGE("invalid reg: 0x%x", reg);
		return -EINVAL;
	}

	reg_mask = reg & 0xFFFFF000;
	if (reg_mask == BASE_ADDR_PAGE_CFG || reg_mask == BASE_ADDR_PAGE_IO) {
		reg = reg | CODEC_BASE_ADDR;
	} else if (reg_mask == BASE_ADDR_PAGE_VIRTUAL) {
		virtual_reg_write(platform_data, reg, value);
		return 0;
	}

	if (platform_data->resmgr == NULL) {
		AUDIO_LOGE("platform_data->resmgr null");
		return -EFAULT;
	}

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	hicodec_debug_reg_rw_cache(reg, value, HICODEC_DEBUG_FLAG_WRITE);
#endif

	da_combine_resmgr_request_reg_access(platform_data->resmgr, reg);
	ret = hi_cdcctrl_reg_write(platform_data->cdc_ctrl, reg, value);
	da_combine_resmgr_release_reg_access(platform_data->resmgr, reg);

	return ret;
}


