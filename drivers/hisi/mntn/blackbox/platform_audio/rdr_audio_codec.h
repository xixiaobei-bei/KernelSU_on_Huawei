/*
 * rdr_audio_codec.h
 *
 * audio codec rdr
 *
 * Copyright (c) 2015-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __RDR_AUDIO_CODEC_H__
#define __RDR_AUDIO_CODEC_H__

#include <linux/hisi/rdr_pub.h>
#include <linux/types.h>

int rdr_audio_codec_init(void);
void rdr_audio_codec_exit(void);
void rdr_audio_codec_dump(unsigned int modid, char *pathname, pfn_cb_dump_done pfb);
void rdr_audio_codec_reset(unsigned int modid, unsigned int etype, u64 coreid);
void rdr_codec_dsp_watchdog_process(void);
void rdr_audio_codec_err_process(void);
void rdr_audio_clear_reboot_times(void);
void rdr_audio_register_get_ap_reset_cfg_cb(bool (*cb)(void));

#endif /* __RDR_AUDIO_CODEC_H__ */

