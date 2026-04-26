/*
 * soundtrigger_socdsp_pcm.h
 *
 * soundtrigger pcm buffer driver
 *
 * Copyright (c) 2014-2020 Huawei Technologies Co., Ltd.
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

#ifndef __SOUNDTRIGGER_SOCDSP_PCM_H__
#define __SOUNDTRIGGER_SOCDSP_PCM_H__

#define ONEMIC_CHN 1
#ifdef CONFIG_DSP_VAD_WAKEUP
#define MAX_MICNUM_CHN 8
#else
#define MAX_MICNUM_CHN 2
#endif

enum socdsp_wakeup_mode {
	NORMAL_WAKEUP_MODE = 0,
	LP_WAKEUP_MODE,
	WAKEUP_MODE_BUTT,
};

void soundtrigger_socdsp_pcm_flag_init(uint32_t wakeup_mode);
int soundtrigger_socdsp_pcm_fastbuffer_filled(uint32_t wakeup_mode, uint32_t fast_len);
int soundtrigger_socdsp_pcm_init(uint32_t channel_num);
int soundtrigger_socdsp_pcm_deinit(void);
int soundtrigger_socdsp_pcm_elapsed(uint32_t wakeup_mode, uint32_t start, int buffer_len);

#endif /* __SOUNDTRIGGER_SOCDSP_PCM_H__ */
