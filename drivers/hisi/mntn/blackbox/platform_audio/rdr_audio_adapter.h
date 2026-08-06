/*
 * rdr_audio_adapter.h
 *
 * audio rdr adpter
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

#ifndef __RDR_AUDIO_ADAPTER_H__
#define __RDR_AUDIO_ADAPTER_H__

#include <linux/hisi/rdr_pub.h>
#include <mntn_subtype_exception.h>

#define RDR_FNAME_LEN 128UL

/* rdr modid for dsp from 0x84000000(HISI_BB_MOD_HIFI_START) to 0x84ffffff(HISI_BB_MOD_HIFI_END) */
enum rdr_audio_modid {
	RDR_AUDIO_MODID_START = HISI_BB_MOD_HIFI_START,
	RDR_AUDIO_SOC_WD_TIMEOUT_MODID,
	RDR_AUDIO_CODEC_WD_TIMEOUT_MODID,
	RDR_AUDIO_NOC_MODID = 0x84000021,
	RDR_AUDIO_CODEC_CRASH_MODID_START,
	RDR_AUDIO_CODEC_ERR_MODID,
	RDR_AUDIO_SLIMBUS_LOSTSYNC_MODID,
	RDR_AUDIO_RUNTIME_SYNC_FAIL_MODID,
	RDR_AUDIO_CODEC_CRASH_MODID_END,
	RDR_AUDIO_MODID_END = HISI_BB_MOD_HIFI_END
};

enum core_type {
	CODECDSP,
	SOCDSP
};

#define RDR_CODECDSP_STACK_TO_MEM_SIZE 512
#define PARSER_CODEC_TRACE_SIZE 2300
#define RDR_CODECDSP_CPUVIEW_TO_MEM_SIZE 512
#define RDR_SOCDSP_STACK_TO_MEM_SIZE 4096
#define PARSER_SOCDSP_TRACE_SIZE 15920
#define CODEC_CPUVIEW_DETAIL_MAX_NUM 100
#define SOCDSP_CPUVIEW_DETAIL_MAX_NUM 1024
#define CPUVIEW_ONE_INFO_LEN 128
#define CPUVIEW_REGION_INFO_LEN 64
#define CPUVIEW_TASK_INFO_LEN 32
#define CPUVIEW_INTERRPUT_INFO_LEN 32

#define PARSER_CODEC_CPUVIEW_LOG_SIZE (CODEC_CPUVIEW_DETAIL_MAX_NUM * CPUVIEW_ONE_INFO_LEN)
#define PARSER_SOCDSP_CPUVIEW_LOG_SIZE (SOCDSP_CPUVIEW_DETAIL_MAX_NUM * CPUVIEW_ONE_INFO_LEN + 180)
#define SOCDSP_ORIGINAL_CPUVIEW_SIZE \
	(sizeof(struct cpuview_details) + (sizeof(struct cpuview_slice_record) * SOCDSP_CPUVIEW_DETAIL_MAX_NUM))

struct cpuview_slice_record {
	unsigned int target : 2;
	unsigned int target_id : 5;
	unsigned int action : 1;
	unsigned int time_stamp : 24;
};

struct cpuview_details {
	unsigned short curr_idx;
	unsigned short rpt_idx;
	struct cpuview_slice_record records[0];
};

typedef int (*LOG_PARSE_FUNC)(const char *, unsigned int, char *, unsigned int, unsigned int);
struct parse_log {
	unsigned int original_offset;
	unsigned int original_log_size;
	unsigned int parse_log_size;
	LOG_PARSE_FUNC parse_func;
};

int rdr_audio_write_file(const char *name, const char *data, unsigned int size);
int parse_dsp_cpuview(const char *original_buf, unsigned int original_buf_size,
	char *parsed_buf, unsigned int parsed_buf_size, unsigned int core_type);
int parse_dsp_trace(const char *original_data, unsigned int original_data_size,
	char *parsed_data, unsigned int parsed_data_size, unsigned int core_type);

#endif /* __RDR_AUDIO_ADAPTER_H__ */

