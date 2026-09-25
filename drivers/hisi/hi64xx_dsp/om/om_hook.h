/*
 * om_hook.h
 *
 * hook module for hi64xx codec
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

#ifndef __HI64XX_DSP_HOOK_H__
#define __HI64XX_DSP_HOOK_H__

#include <linux/hisi/hi64xx/asp_dma.h>
#include <linux/hisi/hi64xx/hi64xx_irq.h>
#include <linux/hisi/hi64xx_dsp/hi64xx_dsp_misc.h>

#define HOOK_PATH_BETA_CLUB "/data/log/codec_dsp/beta_club/"

int hi64xx_dsp_hook_init(struct hi64xx_irq *irqmgr, unsigned int codec_type);
void hi64xx_dsp_hook_deinit(void);
void hi64xx_stop_hook_route(void);
int hi64xx_set_dsp_hook_bw(unsigned short bandwidth);
int hi64xx_set_dsp_hook_sponsor(unsigned short sponsor);
void hi64xx_dsp_dump_to_file(const char *buf, unsigned int size, const char *path);
int hi64xx_dsp_create_hook_dir(const char *path);
int hi64xx_set_hook_path(const struct hi64xx_param_io_buf *param);
int hi64xx_start_hook(const struct hi64xx_param_io_buf *param);
int hi64xx_stop_hook(const struct hi64xx_param_io_buf *param);
void hi64xx_stop_dsp_hook(void);
void hi64xx_stop_dspif_hook(void);
#ifdef ENABLE_HI64XX_HIFI_DEBUG
int hi64xx_set_hook_bw(const struct hi64xx_param_io_buf *param);
int hi64xx_set_hook_sponsor(const struct hi64xx_param_io_buf *param);
int hi64xx_set_dir_count(const struct hi64xx_param_io_buf *param);
int hi64xx_start_mad_test(const struct hi64xx_param_io_buf *param);
int hi64xx_stop_mad_test(const struct hi64xx_param_io_buf *param);
void hi64xx_set_supend_time(void);
int hi64xx_wakeup_test(const struct hi64xx_param_io_buf *param);
#endif

#endif

