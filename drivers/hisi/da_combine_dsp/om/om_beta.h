/*
 * om_beta.h
 *
 * anc beta module for da_combine codec
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


#ifndef __DA_COMBINE_DSP_OM_BETA_H__
#define __DA_COMBINE_DSP_OM_BETA_H__

#include "da_combine_dsp_interface.h"
#include <linux/types.h>

int da_combine_dsp_beta_init(void);
void da_combine_dsp_beta_deinit(void);
int da_combine_report_pa_buffer_reverse(const void *data);
void da_combine_anc_beta_generate_path(enum hook_pos pos,
	const char *base_path, char *full_path, unsigned long full_path_len);
void da_combine_set_voice_hook_switch(unsigned short permission);
void da_combine_dsp_beta_deinit(void);
int da_combine_anc_beta_start_hook(const void *data);
int da_combine_anc_beta_stop_hook(const void *data);
int da_combine_anc_beta_upload_log(const void *data);
int da_combine_dsp_beta_init(void);
int da_combine_dsm_beta_dump_file(const void *data, bool create_dir);
int da_combine_dsm_beta_log_upload(const void *data);
int da_combine_dsm_report_with_creat_dir(const void *data);
int da_combine_dsm_report_without_creat_dir(const void *data);
int da_combine_dsm_dump_file_without_creat_dir(const void *data);
int da_combine_virtual_btn_beta_dump_file(const void *data,
	unsigned int len, bool create_dir);
int da_combine_upload_virtual_btn_beta(const void *data);
int da_combine_wakeup_err_msg(const void *data);

#endif

