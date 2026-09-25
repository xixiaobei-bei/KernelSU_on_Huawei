/*
 * om.h
 *
 * om module for hi64xx codec
 *
 * Copyright (c) 2015-2019 Huawei Technologies Co., Ltd.
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

#ifndef __HI64XX_DSP_OM_H__
#define __HI64XX_DSP_OM_H__

#include <linux/hisi/hi64xx/asp_dma.h>
#include <linux/hisi/hi64xx/hi64xx_irq.h>
#include <linux/hisi/hi64xx_dsp/hi64xx_dsp_misc.h>

#define OM_HI64XX_DUMP_OCRAM_NAME "codec_log.bin"
#define OM_HI64XX_DUMP_RAM_LOG_PATH "codechifi_logs/"

int hi64xx_dsp_om_init(const struct hi64xx_dsp_config *config,
	struct hi64xx_irq *irqmgr);
void hi64xx_dsp_om_deinit(void);
void hi64xx_dsp_dump_no_path(void);
void hi64xx_set_only_printed_enable(bool enable);
void hi64xx_dsp_dump_with_path(const char *path);
void hi64xx_save_log(void);

#endif
