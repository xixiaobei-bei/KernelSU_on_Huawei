/*
 * download_image.h
 *
 * da_combine codec dsp img download
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

#ifndef __DOWNLOAD_IMAGE_H__
#define __DOWNLOAD_IMAGE_H__

#include <linux/firmware.h>
#include <linux/hisi/da_combine_dsp/da_combine_dsp_misc.h>

struct da_combine_dsp_img_dl_config {
	uint32_t dspif_clk_en_addr;
};

int da_combine_dsp_img_dl_init(struct da_combine_irq *irqmgr,
	const struct da_combine_dsp_img_dl_config *dl_config,
	const struct da_combine_dsp_config *config);
void da_combine_dsp_img_dl_deinit(void);
void da_combine_dsp_poweron_irq_handler(void);
int da_combine_fw_download(const struct da_combine_param_io_buf *param);
unsigned int da_combine_get_dsp_poweron_state(void);
int da_combine_fw_restore(void);

#endif

