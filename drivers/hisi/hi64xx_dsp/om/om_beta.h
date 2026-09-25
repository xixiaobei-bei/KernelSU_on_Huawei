/*
 * om_beta.h
 *
 * anc beta module for hi64xx codec
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


#ifndef __HI64XX_DSP_ANC_BETA_H__
#define __HI64XX_DSP_ANC_BETA_H__

#include <hi64xx_dsp_interface.h>
#include <linux/types.h>

int hi64xx_dsp_beta_init(void);
void hi64xx_dsp_beta_deinit(void);
int hi64xx_report_pa_buffer_reverse(const void *data);
void hi64xx_anc_beta_generate_path(enum hook_pos pos,
	const char *base_path, char *full_path, unsigned long full_path_len);
void hi64xx_set_voice_hook_switch(unsigned short permission);
void hi64xx_dsp_beta_deinit(void);
int hi64xx_anc_beta_start_hook(const void *data);
int hi64xx_anc_beta_stop_hook(const void *data);
int hi64xx_anc_beta_upload_log(const void *data);
int hi64xx_dsp_beta_init(void);
int hi64xx_dsm_beta_dump_file(const void *data, bool create_dir);
int hi64xx_dsm_beta_log_upload(const void *data);
int hi64xx_dsm_report_with_creat_dir(const void *data);
int hi64xx_dsm_report_without_creat_dir(const void *data);
int hi64xx_dsm_dump_file_without_creat_dir(const void *data);
int hi64xx_virtual_btn_beta_dump_file(const void *data,
	unsigned int len, bool create_dir);
int hi64xx_upload_virtual_btn_beta(const void *data);
int hi64xx_wakeup_err_msg(const void *data);

#endif

