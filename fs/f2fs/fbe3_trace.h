/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: header file for fbe3_trace.c
 * Author: LAI Xinyi
 * Create: 2020-09-01
 */

#ifndef FBE3_TRACE_H
#define FBE3_TRACE_H

#include "sdp_internal.h"
#include <linux/types.h>

#define ECE_BIG_DATA_UPLOAD_CODE 940013004
#define SECE_BIG_DATA_UPLOAD_CODE 940013005
#define USER_ID 65535
#define ECE_GEN_METADATA 100
#define SECE_GEN_METADATA 101
#define GEN_METADATA_SUC 0
#define GEN_METADATA_FAIL 1

struct fbe3_metadata_stat {
	uint8_t fbe3_enable;
	uint8_t msp_enable;
	uint8_t scene_type;
	uint8_t result;
	int hisi_delay;
	uint32_t user_id;
	int hisi_err_code;
	int vold_err_code;
};

void gen_meta_upload_bigdata(int type, int ret, uint64_t hisi_delay);

#endif
