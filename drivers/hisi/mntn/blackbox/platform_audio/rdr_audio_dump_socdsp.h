/*
 * rdr_audio_dump_socdsp.h
 *
 * audio dump socdsp driver
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

#ifndef __RDR_AUDIO_DUMP_SOCDSP_H__
#define __RDR_AUDIO_DUMP_SOCDSP_H__

#include <linux/hisi/rdr_pub.h>
#include <linux/types.h>

#define COMPILE_TIME_BUFF_SIZE 24
#define ICC_DEBUG_PROTECT_WORD1 0xCDCDDCDC
#define ICC_DEBUG_PROTECT_WORD2 0xEFEFFEFE
#define RDR_COMMENT_LEN 128UL
#define ICC_SHARE_FAMA_ADDR_OFFSET 0
#define ICC_STAT_MSG_NUM 50

void rdr_audio_soc_dump(unsigned int modid, char *pathname, pfn_cb_dump_done pfb);
void audio_rdr_nmi_notify_dsp(void);
bool is_dsp_power_on(void);
int soc_dump_thread(void *arg);

#endif /* __RDR_AUDIO_DUMP_SOCDSP_H__ */

