/*
 * download_image.h
 *
 * hi64xx codec dsp img download
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
#include <linux/hisi/hi64xx_dsp/hi64xx_dsp_misc.h>

struct hi64xx_dsp_img_dl_config {
	uint32_t dspif_clk_en_addr;
};

int hi64xx_dsp_img_dl_init(struct hi64xx_irq *irqmgr,
	const struct hi64xx_dsp_img_dl_config *dl_config,
	const struct hi64xx_dsp_config *config);
void hi64xx_dsp_img_dl_deinit(void);
void hi64xx_dsp_poweron_irq_handler(void);
int hi64xx_fw_download(const struct hi64xx_param_io_buf *param);
unsigned int hi64xx_get_dsp_poweron_state(void);

#endif

