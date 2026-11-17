/*
 * dsp lpp.h
 *
 * head of dsp lpp.c
 *
 * Copyright (c) 2015-2020 Huawei Technologies CO., Ltd.
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

#ifndef __DSP_LPP_H__
#define __DSP_LPP_H__

#include <linux/list.h>

int dsp_lpp_init(unsigned char *base_virt);
void dsp_lpp_deinit(void);
void rcv_dsp_play_done_msg(struct list_head *node);
void release_update_buff_wakelock(void);
int soc_dsp_wakeup_read_thread(uintptr_t arg);
int soc_dsp_wakeup_pcm_read_thread(uintptr_t arg);
int soc_dsp_wakeup_para_write_thread(uintptr_t arg);

#endif /* __DSP_LPP_H__ */

