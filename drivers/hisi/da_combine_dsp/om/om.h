/*
 * om.h
 *
 * om module for da_combine codec
 *
 * Copyright (c) 2015 Huawei Technologies Co., Ltd.
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

#ifndef __DA_COMBINE_DSP_OM_H__
#define __DA_COMBINE_DSP_OM_H__

#include <linux/hisi/da_combine/asp_dma.h>
#include <linux/hisi/da_combine/da_combine_irq.h>
#include <linux/hisi/da_combine_dsp/da_combine_dsp_misc.h>

#define OM_DA_COMBINE_DUMP_OCRAM_NAME "codec_log.bin"
#define OM_DA_COMBINE_DUMP_RAM_LOG_PATH "codechifi_logs/"

int da_combine_dsp_om_init(const struct da_combine_dsp_config *config,
	struct da_combine_irq *irqmgr);
void da_combine_dsp_om_deinit(void);
void da_combine_dsp_dump_no_path(void);
void da_combine_set_only_printed_enable(bool enable);
void da_combine_dsp_dump_with_path(const char *path);
void da_combine_save_log(void);

#endif
