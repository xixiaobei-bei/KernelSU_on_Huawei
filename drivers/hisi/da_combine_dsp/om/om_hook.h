/*
 * om_hook.h
 *
 * hook module for da_combine codec
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

#ifndef __DA_COMBINE_DSP_OM_HOOK_H__
#define __DA_COMBINE_DSP_OM_HOOK_H__

#include <linux/hisi/da_combine/asp_dma.h>
#include <linux/hisi/da_combine/da_combine_irq.h>
#include <linux/hisi/da_combine_dsp/da_combine_dsp_misc.h>

#define HOOK_PATH_BETA_CLUB "/data/log/codec_dsp/beta_club/"

int da_combine_dsp_hook_init(struct da_combine_irq *irqmgr, unsigned int codec_type);
void da_combine_dsp_hook_deinit(void);
void da_combine_stop_hook_route(void);
int da_combine_set_dsp_hook_bw(unsigned short bandwidth);
int da_combine_set_dsp_hook_sponsor(unsigned short sponsor);
void da_combine_dsp_dump_to_file(const char *buf, unsigned int size, const char *path);
int da_combine_dsp_create_hook_dir(const char *path);
int da_combine_set_hook_path(const struct da_combine_param_io_buf *param);
int da_combine_start_hook(const struct da_combine_param_io_buf *param);
int da_combine_stop_hook(const struct da_combine_param_io_buf *param);
void da_combine_stop_dsp_hook(void);
void da_combine_stop_dspif_hook(void);
#ifdef ENABLE_DA_COMBINE_HIFI_DEBUG
int da_combine_set_hook_bw(const struct da_combine_param_io_buf *param);
int da_combine_set_hook_sponsor(const struct da_combine_param_io_buf *param);
int da_combine_set_dir_count(const struct da_combine_param_io_buf *param);
int da_combine_start_mad_test(const struct da_combine_param_io_buf *param);
int da_combine_stop_mad_test(const struct da_combine_param_io_buf *param);
void da_combine_set_supend_time(void);
int da_combine_wakeup_test(const struct da_combine_param_io_buf *param);
#endif

#endif

