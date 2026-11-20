/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: dsc algorithm manager
 * Author: yangbr
 * Create: 2020-01-22
 * Notes: null
 */
#ifndef __DPTX_DSC_ALGORITHM_H__
#define __DPTX_DSC_ALGORITHM_H__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include "../hisi_fb_def.h"
#include "utilities/platform.h"

#define DSC_OUTPUT_MODE 24

enum DSC_VER {
	DSC_VERSION_V_1_1 = 0,
	DSC_VERSION_V_1_2
};

enum pixel_format {
	DSC_RGB = 0,
	DSC_YUV,
	DSC_YUV420,
	DSC_YUV422,
	DSC_YUV444,
};

enum gen_rc_params {
	DSC_NOT_GENERATE_RC_PARAMETERS = 0,
	DSC_GENERATE_RC_PARAMETERS,
};

struct rc_table_param {
	uint32_t min_qp : 5;
	uint32_t max_qp : 5;
	uint32_t offset : 6;
};

struct input_dsc_info {
	enum DSC_VER dsc_version;
	enum pixel_format format;
	uint32_t pic_width;
	uint32_t pic_height;
	uint32_t slice_width;
	uint32_t slice_height;
	uint32_t dsc_bpp;
	uint32_t dsc_bpc;
	uint32_t linebuf_depth;
	uint32_t block_pred_enable;
	uint32_t gen_rc_params;
};

struct dsc_info {
	uint32_t dsc_version_major;
	uint32_t dsc_version_minor;
	enum pixel_format format;
	uint32_t dsc_bpc;
	uint32_t linebuf_depth;
	uint16_t block_pred_enable;
	uint16_t gen_rc_params;
	uint16_t convert_rgb;
	uint16_t simple_422;
	uint16_t native_420;
	uint16_t native_422;
	uint32_t dsc_bpp;
	uint32_t pic_height;
	uint32_t pic_width;
	uint32_t slice_height;
	uint32_t slice_width;

	uint32_t chunk_size;
	uint32_t initial_xmit_delay;
	uint32_t initial_dec_delay;
	uint32_t initial_scale_value;
	uint32_t scale_increment_interval;
	uint32_t scale_decrement_interval;
	uint32_t first_line_bpg_offset;
	uint32_t second_line_bpg_offset;
	uint32_t nfl_bpg_offset;
	uint32_t slice_bpg_offset;
	uint32_t initial_offset;
	uint32_t final_offset;
	uint32_t flatness_min_qp;
	uint32_t flatness_max_qp;
	uint32_t groups_per_line;

	uint32_t rc_model_size;
	uint32_t rc_edge_factor;
	uint32_t rc_quant_incr_limit0;
	uint32_t rc_quant_incr_limit1;
	uint32_t rc_tgt_offset_hi;
	uint32_t rc_tgt_offset_lo;

	uint32_t rc_buf_thresh[DSC_THRESH_LENGTH];
	struct rc_table_param rc_range[DSC_RC_RANGE_LENGTH];
};

struct hisi_dsc_algorithm {
	void (*vesa_dsc_info_calc)(const struct input_dsc_info *input_para,
		struct dsc_info *output_para, struct dsc_info *exist_para);
};

void init_dsc_algorithm(struct hisi_dsc_algorithm *p);

#endif
