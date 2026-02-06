/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: dsc algorithm
 * Author: yangbr
 * Create: 2020-01-22
 * Notes: null
 */
#include "dsc_algorithm.h"
#include "utilities/platform.h"
#include "utilities/hisi_debug.h"
#include "../hisi_fb.h"
#include "../hisi_dp.h"

/* used by pad and syno/hisi dp */
static struct rc_table_param g_rc_range_params_8bpc_8bpp_rgb_default[DSC_RC_RANGE_LENGTH] = {
	{ 0, 4, 2 }, { 0, 4, 0 }, { 1, 5, 0 }, { 1, 6, -2 }, { 3, 7, -4 },
	{ 3, 7, -6 }, { 3, 7, -8 }, { 3, 8, -8 }, { 3, 9, -8 }, { 3, 10, -10 },
	{ 5, 10, -10 }, { 5, 11, -12 }, { 5, 11, -12 }, { 9, 12, -12 }, { 12, 13, -12 },
};

/* used by teton and 37700 panel */
static struct rc_table_param g_rc_range_params_8bpc_12bpp_yuv422_default[DSC_RC_RANGE_LENGTH] = {
	{ 0, 4, 2 }, { 0, 4, 0 }, { 1, 5, 0 }, { 1, 6, -2 }, { 3, 7, -4 },
	{ 3, 7, -6 }, { 3, 7, -8 }, { 3, 8, -8 }, { 3, 9, -8 }, { 3, 10, -10 },
	{ 5, 10, -10 }, { 5, 11, -12 }, { 5, 11, -12 }, { 9, 12, -12 }, { 12, 13, -12 },
};

/* Default is for 8bpc/8bpp used by noah boe panel */
static struct rc_table_param g_rc_range_params_8bpc_16bpp_yuv422_default[DSC_RC_RANGE_LENGTH] = {
	{ 0, 2, 2 }, { 0, 4, 0 }, { 1, 5, 0 }, { 1, 6, -2 }, { 3, 7, -4 },
	{ 3, 7, -6 }, { 3, 7, -8 }, { 3, 8, -8 }, { 3, 8, -8 }, { 3, 9, -10 },
	{ 5, 9, -10 }, { 5, 9, -12 }, { 5, 9, -12 }, { 7, 10, -12 }, { 10, 11, -12 },
};

/* used by 36870 panel */
static struct rc_table_param g_rc_range_params_10bpc_10bpp_rgb_default[DSC_RC_RANGE_LENGTH] = {
	{ 0, 7, 2 }, { 4, 8, 0 }, { 5, 9, 0 }, { 6, 10, -2 }, { 7, 11, -4 },
	{ 7, 11, -6 }, { 7, 11, -8 }, { 7, 12, -8 }, { 7, 13, -8 }, { 7, 13, -10 },
	{ 9, 14, -10 }, { 9, 15, -10 }, { 9, 16, -12 }, { 11, 17, -12 }, { 17, 19, -12 },
};

/* used by hisi dp */
static struct rc_table_param g_rc_range_params_8bpc_12bpp_rgb[DSC_RC_RANGE_LENGTH] = {
	{ 0, 2, 2 }, { 0, 3, 0 }, { 1, 4, 0 }, { 1, 5, -2 }, { 3, 6, -4 },
	{ 3, 6, -6 }, { 3, 7, -8 }, { 3, 8, -8 }, { 3, 8, -8 }, { 3, 9, -10 },
	{ 5, 9, -10 }, { 5, 9, -10 }, { 5, 9, -12 }, { 7, 10, -12 }, { 10, 11, -12 },
};

static struct rc_table_param g_rc_range_params_8bpc_8bpp_rgb[DSC_RC_RANGE_LENGTH] = {
	{ 0, 4, 2 }, { 0, 4, 0 }, { 1, 5, 0 }, { 1, 6, -2 }, { 3, 7, -4 },
	{ 3, 7, -6 }, { 3, 7, -8 }, { 3, 8, -8 }, { 3, 9, -8 }, { 4, 10, -10 },
	{ 5, 10, -10 }, { 5, 11, -10 }, { 5, 11, -12 }, { 8, 12, -12 }, { 12, 13, -12 },
};

static struct rc_table_param g_rc_range_params_10bpc_8bpp_rgb[DSC_RC_RANGE_LENGTH] = {
	{ 0, 8, 2 }, { 4, 8, 0 }, { 5, 9, 0 }, { 5, 10, -2 }, { 7, 11, -4 },
	{ 7, 11, -6 }, { 7, 11, -8 }, { 7, 12, -8 }, { 7, 13, -8 }, { 8, 14, -10 },
	{ 9, 14, -10 }, { 9, 15, -10 }, { 9, 15, -12 }, { 12, 16, -12 }, { 16, 17, -12 },
};

static struct rc_table_param g_rc_range_params_10bpc_10bpp_rgb[DSC_RC_RANGE_LENGTH] = {
	{ 0, 7, 2 }, { 4, 8, 0 }, { 5, 9, 0 }, { 5, 10, -2 }, { 7, 11, -4 },
	{ 7, 11, -6 }, { 7, 11, -8 }, { 7, 12, -8 }, { 7, 13, -8 }, { 7, 13, -10 },
	{ 9, 14, -10 }, { 9, 14, -10 }, { 9, 15, -12 }, { 12, 15, -12 }, { 15, 16, -12 },
};

static struct rc_table_param g_rc_range_params_8bpc_12bpp_yuv422[DSC_RC_RANGE_LENGTH] = {
	{ 0, 4, 2 }, { 0, 4, 0 }, { 1, 5, 0 }, { 2, 6, -2 }, { 3, 7, -4 },
	{ 3, 7, -6 }, { 3, 7, -8 }, { 3, 8, -8 }, { 3, 9, -8 }, { 3, 10, -10 },
	{ 5, 10, -10 }, { 5, 11, -12 }, { 5, 11, -12 }, { 8, 12, -12 }, { 12, 13, -12 },
};

static struct rc_table_param g_rc_range_params_8bpc_14bpp_yuv422[DSC_RC_RANGE_LENGTH] = {
	{ 0, 3, 2 }, { 0, 4, 0 }, { 1, 5, 0 }, { 2, 6, -2 }, { 3, 7, -4 },
	{ 3, 7, -6 }, { 3, 7, -8 }, { 3, 8, -8 }, { 3, 9, -8 }, { 3, 9, -10 },
	{ 5, 10, -10 }, { 5, 10, -10 }, { 5, 11, -12 }, { 7, 11, -12 }, { 11, 12, -12 },
};

static struct rc_table_param g_rc_range_params_8bpc_16bpp_yuv422[DSC_RC_RANGE_LENGTH] = {
	{ 0, 2, 2 }, { 0, 4, 0 }, { 1, 5, 0 }, { 2, 6, -2 }, { 3, 7, -4 },
	{ 3, 7, -6 }, { 3, 7, -8 }, { 3, 8, -8 }, { 3, 8, -8 }, { 3, 9, -10 },
	{ 5, 9, -10 }, { 5, 9, -10 }, { 5, 9, -12 }, { 7, 10, -12 }, { 10, 11, -12 },
};

static struct rc_table_param g_rc_range_params_10bpc_16bpp_yuv422[DSC_RC_RANGE_LENGTH] = {
	{ 0, 4, 2 }, { 2, 6, 0 }, { 3, 7, 0 }, { 5, 9, -2 }, { 5, 10, -4 },
	{ 6, 11, -6 }, { 6, 11, -8 }, { 7, 12, -8 }, { 7, 12, -8 }, { 7, 13, -10 },
	{ 8, 13, -10 }, { 8, 13, -10 }, { 9, 13, -12 }, { 11, 14, -12 }, { 14, 15, -12 },
};

static uint32_t g_rc_buf_threshold[DSC_THRESH_LENGTH] = {
	896, 1792, 2688, 3584, 4480, 5376, 6272, 6720, 7168, 7616, 7744, 7872, 8000, 8064,
};

static void dsc_calc_chunk_size(struct dsc_info *dsc_info)
{
	dpu_check_and_no_retval((dsc_info == NULL), ERR, "NULL POINTER");

	if (dsc_info->format == DSC_RGB)
		dsc_info->chunk_size = round1(dsc_info->slice_width * dsc_info->dsc_bpp, BITS_PER_CHUNK);
	else
		dsc_info->chunk_size = round1((dsc_info->slice_width / 2) * dsc_info->dsc_bpp, BITS_PER_CHUNK);
	range_check("chunk_size", dsc_info->chunk_size, 0, 65535);
	DPU_FB_DEBUG("[DSC]PPS chunk_size = %u\n", dsc_info->chunk_size);
}

static void dsc_calc_groups_per_line(struct dsc_info *dsc_info)
{
	dpu_check_and_no_retval((dsc_info == NULL), ERR, "NULL POINTER");
	if (dsc_info->format == DSC_RGB)
		dsc_info->groups_per_line =
			round1(dsc_info->slice_width, PIXELS_PER_GROUP);
	else
		dsc_info->groups_per_line =
			round1((dsc_info->slice_width >> 1), PIXELS_PER_GROUP);
	DPU_FB_DEBUG("[DSC]PPS groups_per_line = %u\n", dsc_info->groups_per_line);
}

static void dsc_calc_first_line_bpg_offset(struct dsc_info *dsc_info)
{
	uint8_t first_line_bpg_offset;
	uint16_t slice_height = dsc_info->slice_height;
	uint32_t uncompressed_bpg_rate;

	dpu_check_and_no_retval((dsc_info == NULL), ERR, "NULL POINTER");
	DPU_FB_DEBUG("[DSC]PPS slice_height = %u\n", slice_height);
	/*
	 * first_line_bpg_offset: Specifies the number of additional bits
	 * that are allocated for each group on the first line of a slice.
	 * Those magic numbers were assigned by the DSC protocol
	 */
	if (dsc_info->format == DSC_YUV422)
		uncompressed_bpg_rate = 3 * dsc_info->dsc_bpc * 4;
	else
		uncompressed_bpg_rate = (3 * dsc_info->dsc_bpc + (dsc_info->format == DSC_RGB ? 2 : 0)) * 3;
	DPU_FB_DEBUG("[DSC]PPS uncompressed_bpg_rate = %u\n", uncompressed_bpg_rate);
	if (slice_height >= 8)
		first_line_bpg_offset = 12 + ((9 * min(34, slice_height - 8)) / 100);
	else
		first_line_bpg_offset = 2 * (slice_height - 1);
	first_line_bpg_offset = range_clamp(first_line_bpg_offset, 0, (uint32_t)(uncompressed_bpg_rate - 3 * dsc_info->dsc_bpp));
	range_check("first_line_bpg_offset", first_line_bpg_offset, 0, 31);
	DPU_FB_DEBUG("[DSC]PPS first_line_bpg_offset = %u\n", first_line_bpg_offset);
	dsc_info->first_line_bpg_offset = first_line_bpg_offset;
}

static void dsc_calc_second_line_bpg_offset(struct dsc_info *dsc_info)
{
	uint32_t uncompressed_bpg_rate;

	dpu_check_and_no_retval((dsc_info == NULL), ERR, "NULL POINTER");
	if (dsc_info->format == DSC_YUV422)
		uncompressed_bpg_rate = 3 * dsc_info->dsc_bpc * 4;
	else
		uncompressed_bpg_rate = (3 * dsc_info->dsc_bpc + (dsc_info->format == DSC_RGB ? 2 : 0)) * 3;
	/*
	 * second_line_bpg_offset: The second line of a slice in 4:2:0 mode does not code as efficiently as
	 * subsequent lines due to the lack of vertical prediction and ICH upper neighboring pixels. To maintain
	 * uniform visual quality across a slice, it is important to provide an extra bit allocation for
	 * the second line in 4:2:0 mode. Empirical results have shown that a value of 12bpg works
	 * well in general. If slice_height < 8, the recommended second_line_bpg_offset is equal
	 * to 2 * (slice_height - 1).
	 * Those magic numbers were assigned by the DSC protocol.
	 */
	if (dsc_info->format == DSC_YUV420) {
		dsc_info->second_line_bpg_offset = 12;
		if (dsc_info->slice_height < 8)
			dsc_info->second_line_bpg_offset = 2 * (dsc_info->slice_height - 1);
	} else {
		dsc_info->second_line_bpg_offset = 0;
	}
	dsc_info->second_line_bpg_offset = range_clamp(dsc_info->second_line_bpg_offset, 0,
		(uint32_t)(uncompressed_bpg_rate - 3 * dsc_info->dsc_bpp));
	range_check("second_line_bpg_offset", dsc_info->second_line_bpg_offset, 0, 31);
	DPU_FB_DEBUG("[DSC]PPS second_line_bpg_offset = %u\n", dsc_info->second_line_bpg_offset);
}

static void dsc_calc_initial_scale_value(struct dsc_info *dsc_info)
{
	dpu_check_and_no_retval((dsc_info == NULL), ERR, "NULL POINTER");
	/*
	 * initial_scale_value: Shrinks the effective RC model range at the beginning of a slice to maximize tracking
	 * ability. It is recommended to use a value of rc_model_size / (rc_model_size -
	 * initial_offset), keeping in mind that initial_scale_value has three fractional bits.
	 * Those magic numbers were assigned by the DSC protocol.
	 */
	dsc_info->initial_scale_value =
		8 * dsc_info->rc_model_size / (dsc_info->rc_model_size - dsc_info->initial_offset);
	if (dsc_info->groups_per_line < dsc_info->initial_scale_value - 8)
		dsc_info->initial_scale_value = dsc_info->groups_per_line + 8;
	range_check("initial_scale_value", dsc_info->initial_scale_value, 0, 63);
	DPU_FB_DEBUG("[DSC]PPS initial_scale_value = %u\n", dsc_info->initial_scale_value);
}

struct rc_model_set {
	uint8_t format;
	uint8_t dsc_dpp;
	uint32_t initial_offset;
	uint32_t initial_xmit_delay;
	uint32_t param;
};

static struct rc_model_set rc_model_gen_rc[RC_MODEL_GEN_RC_COUNT] = {
	{ DSC_RGB, DSC_8BPP, INITIAL_OFFSET_8BPP, INITIAL_DELAY_8BPP, 0 },
	{ DSC_RGB, DSC_10BPP, INITIAL_OFFSET_10BPP, INITIAL_DELAY_10BPP, 0 },
	{ DSC_YUV422, DSC_12BPP, INITIAL_OFFSET_12BPP_YUV422_DEFAULT, INITIAL_DELAY_12BPP_YUV422_DEFAULT, 0 },
	{ DSC_YUV422, DSC_16BPP, INITIAL_OFFSET_16BPP_YUV422_DEFAULT, INITIAL_DELAY_16BPP_YUV422_DEFAULT, 0 },
};

static uint32_t rc_model_range_yuv422_limit[RC_MODEL_YUV422_DEFAULT] = { DSC_16BPP, DSC_14BPP, DSC_12BPP };
static struct rc_model_set rc_model_yuv422_default[RC_MODEL_YUV422_DEFAULT] = {
	{ DSC_YUV422, DSC_16BPP, INITIAL_OFFSET_16BPP_YUV422_DEFAULT, 0, 0 },
	{ DSC_YUV422, DSC_14BPP, INITIAL_OFFSET_12BPP_YUV422, 1792, 0 },
	{ DSC_YUV422, DSC_12BPP, INITIAL_OFFSET_12BPP_YUV422, 0, 0 },
};

static uint32_t rc_model_range_rgb_limit[RC_MODEL_RGB_DEFAULT] = { DSC_12BPP, DSC_10BPP, DSC_8BPP, DSC_6BPP };
static struct rc_model_set rc_model_rgb_default[RC_MODEL_RGB_DEFAULT] = {
	{ DSC_RGB, DSC_12BPP, INITIAL_OFFSET_12BPP, 0, 0 },
	{ DSC_RGB, DSC_10BPP, INITIAL_OFFSET_10BPP, 1792, 0 },
	{ DSC_RGB, DSC_8BPP, INITIAL_OFFSET_8BPP, 256, 0 },
	{ DSC_RGB, DSC_6BPP, INITIAL_OFFSET_8BPP, 0, 0 },
};

static void dsc_calc_rc_model_size(struct dsc_info *dsc_info)
{
	int i;

	dpu_check_and_no_retval((dsc_info == NULL), ERR, "NULL POINTER");
	/*
	 * Initial transmission delay. Specifies the number of pixel times that the encoder waits
	 * before transmitting data from its rate buffer. When native_422 or native_420 = 1,
	 * the units are container pixel times.
	 * Those magic numbers were assigned by the DSC protocol.
	 */
	if (dsc_info->gen_rc_params == DSC_NOT_GENERATE_RC_PARAMETERS) {
		for (i = 0; i < RC_MODEL_GEN_RC_COUNT; ++i) {
			if (dsc_info->format == rc_model_gen_rc[i].format && dsc_info->dsc_bpp == rc_model_gen_rc[i].dsc_dpp) {
				DPU_FB_DEBUG("[DSC] PPS format = %d, dsc_bpp = %d\n", dsc_info->format, dsc_info->dsc_bpp);
				dsc_info->initial_offset = rc_model_gen_rc[i].initial_offset;
				dsc_info->initial_xmit_delay = rc_model_gen_rc[i].initial_xmit_delay;
				break;
			}
		}
	} else {
		dsc_info->initial_xmit_delay = (4096 + dsc_info->dsc_bpp / 2) / dsc_info->dsc_bpp;
		if (dsc_info->format == DSC_YUV422) {
			for (i = 0; i < RC_MODEL_YUV422_DEFAULT; ++i) {
				if (dsc_info->dsc_bpp >= rc_model_range_yuv422_limit[i]) {
					DPU_FB_DEBUG("[DSC] PPS format = %d, dsc_bpp = %d\n", dsc_info->format, dsc_info->dsc_bpp);
					dsc_info->initial_offset = rc_model_yuv422_default[i].initial_offset - (uint32_t)((dsc_info->dsc_bpp -
						rc_model_yuv422_default[i].dsc_dpp) * rc_model_yuv422_default[i].param);
					break;
				}
			}
		} else {
			for (i = 0; i < RC_MODEL_RGB_DEFAULT; ++i) {
				if (dsc_info->dsc_bpp >= rc_model_range_rgb_limit[i]) {
					DPU_FB_DEBUG("[DSC] PPS format = %d, dsc_bpp = %d\n", dsc_info->format, dsc_info->dsc_bpp);
					dsc_info->initial_offset = rc_model_rgb_default[i].initial_offset -
						(uint32_t)((dsc_info->dsc_bpp - rc_model_rgb_default[i].dsc_dpp) * rc_model_rgb_default[i].param);
					break;
				}
			}
		}
	}

	range_check("rc_model_size", dsc_info->rc_model_size, 0, 65535);
	range_check("initial_xmit_delay", dsc_info->initial_xmit_delay, 0, 1023);
	range_check("initial_offset", dsc_info->initial_offset, 0, dsc_info->rc_model_size);
	DPU_FB_DEBUG("[DSC] PPS DSC rc_model_size is %u\n", dsc_info->rc_model_size);
	DPU_FB_DEBUG("[DSC] PPS DSC initial_offset is %u\n", dsc_info->initial_offset);
	DPU_FB_DEBUG("[DSC] PPS DSC initial_delay is %u\n", dsc_info->initial_xmit_delay);
}

/*
 * \brief
 *    dsc_calc_offset() - calc offset value at a specific group
 *
 * \param dsc_info
 *    Pointer to DSC configuration structure
 * \param group_count
 *    Group to compute offset for
 * \return
 *    Offset value for the group grpcnt
 */
static uint32_t dsc_calc_offset(const struct dsc_info *dsc_info, uint32_t group_count)
{
	uint32_t offset;
	uint32_t group_count_id;

	group_count_id = round1(dsc_info->initial_xmit_delay, PIXELS_PER_GROUP);
	if (group_count <= group_count_id)
		offset = group_count * PIXELS_PER_GROUP * dsc_info->dsc_bpp;
	else
		offset = group_count_id * PIXELS_PER_GROUP * dsc_info->dsc_bpp - (((group_count - group_count_id) *
			dsc_info->slice_bpg_offset) >> OFFSET_FRACTIONAL_BIT);

	if (group_count <= dsc_info->groups_per_line)
		offset += group_count * dsc_info->first_line_bpg_offset;
	else
		offset += dsc_info->groups_per_line * dsc_info->first_line_bpg_offset - (((group_count - dsc_info->groups_per_line)
			* dsc_info->nfl_bpg_offset) >> OFFSET_FRACTIONAL_BIT);

	return offset;
}

static void dsc_calc_initial_dec_delay(struct dsc_info *dsc_info)
{
	uint32_t min_rate_buffer_size;
	uint32_t hrddelay;
	uint32_t max_offset;
	uint32_t offset;

	dpu_check_and_no_retval((dsc_info == NULL), ERR, "NULL POINTER");
	switch (dsc_info->format) {
	case DSC_RGB:
		min_rate_buffer_size = (dsc_info->rc_model_size - dsc_info->initial_offset) +
			(dsc_info->initial_xmit_delay * dsc_info->dsc_bpp) +
			(dsc_info->groups_per_line * dsc_info->first_line_bpg_offset);
		break;
	case DSC_YUV422:
		max_offset = dsc_calc_offset(dsc_info, round1(dsc_info->initial_xmit_delay, PIXELS_PER_GROUP));
		offset = dsc_calc_offset(dsc_info, dsc_info->groups_per_line);
		max_offset = max(max_offset, offset);
		offset = dsc_calc_offset(dsc_info, 2 * dsc_info->groups_per_line);
		max_offset = max(max_offset, offset);
		min_rate_buffer_size = dsc_info->rc_model_size - dsc_info->initial_offset + max_offset;
		break;
	default:
		DPU_FB_ERR("[DSC]format error\n");
		min_rate_buffer_size = 0;
		break;
	}
	hrddelay = round1(min_rate_buffer_size, dsc_info->dsc_bpp);
	dsc_info->initial_dec_delay = hrddelay - dsc_info->initial_xmit_delay;
	range_check("initial_dec_delay", dsc_info->initial_dec_delay, 0, 65535);
	DPU_FB_DEBUG("[DSC]PPS hrddelay = %u\n", hrddelay);
	DPU_FB_DEBUG("[DSC] PPS DSC min_rate_buffer_size is %u\n", min_rate_buffer_size);
	DPU_FB_DEBUG("[DSC]PPS initial_dec_delay = %u\n", dsc_info->initial_dec_delay);
}

static void dsc_calc_scale_decrement_interval(struct dsc_info *dsc_info)
{
	dpu_check_and_no_retval((dsc_info == NULL), ERR, "NULL POINTER");
	/*
	 * scale_decrement_interval: Specifies the number of group times between
	 * decrementing the rcXformScale factor at the beginning of a slice.
	 * those magic numbers were assigned by the DSC protocol
	 */
	if (dsc_info->initial_scale_value > 8)
		dsc_info->scale_decrement_interval = dsc_info->groups_per_line / (dsc_info->initial_scale_value - 8);
	else
		dsc_info->scale_decrement_interval = 4095;

	range_check("scale_decrement_interval", dsc_info->scale_decrement_interval, 0, 4095);
	DPU_FB_DEBUG("[DSC]PPS scale_decrement_interval = %u\n", dsc_info->scale_decrement_interval);
}

static void dsc_calc_nfl_bpg_offset(struct dsc_info *dsc_info)
{
	dpu_check_and_no_retval((dsc_info == NULL), ERR, "NULL POINTER");
	/*
	 * Specifies the number of bits (including fractional bits) that are deallocated for each
	 * group, for groups after the first line of a slice. If the first line has an additional bit budget,
	 * the additional bits that are allocated must come out of the budget for coding the
	 * remainder of the slice. Therefore, the value must be programmed to
	 * first_line_bpg_offset / (slice_height - 1), then rounded up to 16 fractional bits.
	 * Those magic numbers were assigned by the DSC protocol.
	 */
	if (dsc_info->slice_height > 1)
		dsc_info->nfl_bpg_offset =
		(uint32_t)round1((dsc_info->first_line_bpg_offset << OFFSET_FRACTIONAL_BIT), (dsc_info->slice_height - 1));
	else
		dsc_info->nfl_bpg_offset = 0;
	if (dsc_info->nfl_bpg_offset > 65535)
		DPU_FB_ERR("nfl_bpg_offset is too large for this slice height\n");
	DPU_FB_DEBUG("[DSC]PPS nfl_bpg_offset = %u\n", dsc_info->nfl_bpg_offset);
}

static void dsc_calc_final_offset(struct dsc_info *dsc_info)
{
	uint8_t max_se_size;
	uint32_t num_extra_mux_bits;
	uint32_t mux_word_size;
	uint32_t slice_bits;

	dpu_check_and_no_retval((dsc_info == NULL), ERR, "NULL POINTER");
	/*
	 * The muxWordSize is determined by the bits_per_component value:
	 * When bits_per_component is 12, 14, or 16bpc, muxWordSize shall be equal to 64 bits
	 * When bits_per_component is 8 or 10bpc, muxWordSize shall be equal to 48 bits
	 * those magic numbers were assigned by the DSC protocol
	 */
	if (dsc_info->dsc_bpc == DSC_16BPC)
		max_se_size = DSC_16BPC * 4;
	else
		max_se_size = dsc_info->dsc_bpc * 4 + 4;
	mux_word_size = (dsc_info->dsc_bpc < DSC_12BPC) ? 48 : 64;
	if (dsc_info->format == DSC_RGB)
		num_extra_mux_bits = 3 * mux_word_size + 3 * max_se_size - 6;
	else if (dsc_info->format == DSC_YUV422)
		num_extra_mux_bits = 4 * mux_word_size + 4 * max_se_size - 14;
	else
		num_extra_mux_bits = 3 * mux_word_size + 3 * max_se_size - 10;
	slice_bits = 8 * dsc_info->chunk_size * dsc_info->slice_height;
	while ((num_extra_mux_bits > 0) && ((slice_bits - num_extra_mux_bits) % mux_word_size))
		num_extra_mux_bits--;
	dsc_info->final_offset =
		dsc_info->rc_model_size -
		((dsc_info->initial_xmit_delay * dsc_info->dsc_bpp * 16 + 8) >> 4) + num_extra_mux_bits;
	range_check("final_offset", dsc_info->final_offset, 0, 65535);
	DPU_FB_DEBUG("[DSC]PPS slice_bits = %u\n", slice_bits);
	DPU_FB_DEBUG("[DSC]PPS final_offset = %u\n", dsc_info->final_offset);
	DPU_FB_DEBUG("[DSC]PPS num_extra_mux_bits = %u\n", num_extra_mux_bits);
}

static void dsc_calc_slice_bpg_offset(struct dsc_info *dsc_info)
{
	uint8_t max_se_size;
	uint8_t bpc;
	uint16_t group_per_line;
	uint32_t num_extra_mux_bits;
	uint32_t groups_total;
	uint32_t mux_word_size;
	uint32_t slice_bits;

	dpu_check_and_no_retval((dsc_info == NULL), ERR, "NULL POINTER");
	bpc = dsc_info->dsc_bpc;
	mux_word_size = (bpc < DSC_12BPC) ? 48 : 64;
	group_per_line = dsc_info->groups_per_line;
	groups_total = group_per_line * dsc_info->slice_height;
	DPU_FB_DEBUG("[DSC]PPS groups_total = %u\n", groups_total);
	/*
	 * Specifies the number of bits (including fractional bits) that are deallocated for
	 * each group to enforce the slice constraint (i.e., the final buffer model fullness cannot
	 * exceed the initial transmission delay times bits per group), while allowing
	 * a programmable initial_offset. If the initial rate control (RC) model condition is not
	 * completely full, the difference between the initial RC model offset and size
	 * (initial_offset and rc_model_size, respectively) must be accounted for.
	 * The slice_bpg_offset parameter provides a means to resolve this difference. This
	 * parameter also allows the RC algorithm to account for bits that might be lost to
	 * SSM at the end of a slice. The value must be programmed to (rc_model_size -
	 * initial_offset + numExtraMuxBits) / groupsTotal, then rounded up to
	 * 16 fractional bits.
	 * Those magic numbers were assigned by the DSC protocol.
	 */
	if (bpc == DSC_16BPC)
		max_se_size = DSC_16BPC * 4;
	else
		max_se_size = bpc * 4 + 4;

	if (dsc_info->format == DSC_RGB)
		num_extra_mux_bits = 3 * mux_word_size + 3 * max_se_size - 6;
	else if (dsc_info->format == DSC_YUV422)
		num_extra_mux_bits = 4 * mux_word_size + 4 * max_se_size - 14;
	else
		num_extra_mux_bits = 3 * mux_word_size + 3 * max_se_size - 10;

	slice_bits = 8 * dsc_info->chunk_size * dsc_info->slice_height;
	while ((num_extra_mux_bits > 0) && ((slice_bits - num_extra_mux_bits) % mux_word_size))
		num_extra_mux_bits--;
	dsc_info->slice_bpg_offset =
		round1((1 << OFFSET_FRACTIONAL_BIT) *
		(dsc_info->rc_model_size - dsc_info->initial_offset + num_extra_mux_bits), groups_total);
	range_check("slice_bpg_offset", dsc_info->slice_bpg_offset, 0, 65535);
	DPU_FB_DEBUG("[DSC]PPS slice_bpg_offset = %u\n", dsc_info->slice_bpg_offset);
}

static void dsc_calc_scale_increment_interval(struct dsc_info *dsc_info)
{
	int final_scale_value;

	dpu_check_and_no_retval((dsc_info == NULL), ERR, "NULL POINTER");
	dpu_check_and_no_retval((dsc_info->rc_model_size == dsc_info->final_offset), ERR, "zero devider");
	final_scale_value = 8 * dsc_info->rc_model_size / (dsc_info->rc_model_size - dsc_info->final_offset);
	DPU_FB_DEBUG("[DSC]PPS final_scale_value = %d\n", final_scale_value);
	/*
	 * scale_increment_interval:At the end of the slice, it is also desirable to shrink the effective RC model range
	 * to maximize tracking ability. It is recommended to use a value equal to
	 * (final_offset / (nfl_bpg_offset + slice_bpg_offset)) / (8 * (finalScaleValue - 1.125)),
	 * where finalScaleValue is equal to rc_model_size / (rc_model_size - final_offset).
	 * If finalScaleValue is less than or equal to 9, a value of 0 should be used to disable
	 * the scale increment at the end of the slice. If the calculation for scale_increment_interval
	 * results in a value that exceeds 65535, a smaller slice height should be used so that the
	 * programmed scale_increment_interval fits within a 16-bit field.
	 * those magic numbers were assigned by the DSC protocol.
	 */
	dpu_check_and_no_retval((dsc_info->nfl_bpg_offset + dsc_info->slice_bpg_offset == 0), ERR, "zero devider");
	dsc_info->scale_increment_interval =
		(1 << OFFSET_FRACTIONAL_BIT) *
		(dsc_info->final_offset / (((8 * final_scale_value) - 9) *
		(dsc_info->nfl_bpg_offset + dsc_info->slice_bpg_offset)));
	if (final_scale_value > 9)
		dsc_info->scale_increment_interval =
		(1 << OFFSET_FRACTIONAL_BIT) * dsc_info->final_offset /
		((final_scale_value - 9) * (dsc_info->nfl_bpg_offset + dsc_info->slice_bpg_offset));
	else
		dsc_info->scale_increment_interval = 0;
	DPU_FB_DEBUG("[DSC]PPS scale_increment_interval = %u\n", dsc_info->scale_increment_interval);
	if (dsc_info->scale_increment_interval > 65535)
		DPU_FB_INFO("[DSC] Scale increment interval exceeds 65535\n");
}

static void dsc_calc_flatness_qp(struct dsc_info *dsc_info)
{
	dpu_check_and_no_retval((dsc_info == NULL), ERR, "NULL POINTER");
	if (dsc_info->dsc_bpc == DSC_8BPC) {
		dsc_info->flatness_min_qp = FLATNESS_MIN_QP_8BPC;
		dsc_info->flatness_max_qp = FLATNESS_MAX_QP_8BPC;
	} else if (dsc_info->dsc_bpc == DSC_10BPC) {
		dsc_info->flatness_min_qp = FLATNESS_MIN_QP_10BPC;
		dsc_info->flatness_max_qp = FLATNESS_MAX_QP_10BPC;
	} else if (dsc_info->dsc_bpc == DSC_12BPC) {
		dsc_info->flatness_min_qp = FLATNESS_MIN_QP_12BPC;
		dsc_info->flatness_max_qp = FLATNESS_MAX_QP_12BPC;
	} else {
		DPU_FB_ERR("[DSC] dsc_calc_flatness_qp error\n");
	}
	range_check("flatness_min_qp", dsc_info->flatness_min_qp, 0, 31);
	range_check("flatness_max_qp", dsc_info->flatness_max_qp, 0, 31);
	DPU_FB_DEBUG("[DSC]PPS flatness_min_qp = %u\n", dsc_info->flatness_min_qp);
	DPU_FB_DEBUG("[DSC]PPS flatness_max_qp = %u\n", dsc_info->flatness_max_qp);
}

struct rc_range_set {
	uint8_t gen_rc;
	uint8_t format;
	uint8_t dsc_bpc;
	uint8_t dsc_bpp;
	uint32_t rc_table_len;
	struct rc_table_param *p_rc_table;
};

static struct rc_range_set rc_range_array[RC_RANGE_COUNT] = {
	{ DSC_NOT_GENERATE_RC_PARAMETERS, DSC_RGB,
		DSC_8BPC, DSC_8BPP, DSC_RC_RANGE_LENGTH, g_rc_range_params_8bpc_8bpp_rgb_default },
	{ DSC_NOT_GENERATE_RC_PARAMETERS, DSC_RGB,
		DSC_10BPC, DSC_10BPP, DSC_RC_RANGE_LENGTH, g_rc_range_params_10bpc_10bpp_rgb_default },
	{ DSC_NOT_GENERATE_RC_PARAMETERS, DSC_YUV422,
		DSC_8BPC, DSC_12BPP, DSC_RC_RANGE_LENGTH, g_rc_range_params_8bpc_12bpp_yuv422_default },
	{ DSC_NOT_GENERATE_RC_PARAMETERS, DSC_YUV422,
		DSC_8BPC, DSC_16BPP, DSC_RC_RANGE_LENGTH, g_rc_range_params_8bpc_16bpp_yuv422_default },
	{ DSC_GENERATE_RC_PARAMETERS, DSC_RGB,
		DSC_8BPC, DSC_8BPP, DSC_RC_RANGE_LENGTH, g_rc_range_params_8bpc_8bpp_rgb },
	{ DSC_GENERATE_RC_PARAMETERS, DSC_RGB,
		DSC_8BPC, DSC_12BPP, DSC_RC_RANGE_LENGTH, g_rc_range_params_8bpc_12bpp_rgb },
	{ DSC_GENERATE_RC_PARAMETERS, DSC_RGB,
		DSC_10BPC, DSC_8BPP, DSC_RC_RANGE_LENGTH, g_rc_range_params_10bpc_8bpp_rgb },
	{ DSC_GENERATE_RC_PARAMETERS, DSC_RGB,
		DSC_10BPC, DSC_10BPP, DSC_RC_RANGE_LENGTH, g_rc_range_params_10bpc_10bpp_rgb },
	{ DSC_GENERATE_RC_PARAMETERS, DSC_YUV422,
		DSC_8BPC, DSC_12BPP, DSC_RC_RANGE_LENGTH, g_rc_range_params_8bpc_12bpp_yuv422 },
	{ DSC_GENERATE_RC_PARAMETERS, DSC_YUV422,
		DSC_8BPC, DSC_14BPP, DSC_RC_RANGE_LENGTH, g_rc_range_params_8bpc_14bpp_yuv422 },
	{ DSC_GENERATE_RC_PARAMETERS, DSC_YUV422,
		DSC_8BPC, DSC_16BPP, DSC_RC_RANGE_LENGTH, g_rc_range_params_8bpc_16bpp_yuv422 },
	{ DSC_GENERATE_RC_PARAMETERS, DSC_YUV422,
		DSC_10BPC, DSC_16BPP, DSC_RC_RANGE_LENGTH, g_rc_range_params_10bpc_16bpp_yuv422 },
};

static void dsc_calc_rc_range(struct dsc_info *dsc_info)
{
	int i = 0;

	dpu_check_and_no_retval((dsc_info == NULL), ERR, "NULL POINTER");

	for (; i < DSC_THRESH_LENGTH; ++i)
		dsc_info->rc_buf_thresh[i] = (g_rc_buf_threshold[i] >> 6);
	for (i = 0; i < RC_RANGE_COUNT; ++i) {
		if (rc_range_array[i].gen_rc == dsc_info->gen_rc_params &&
			rc_range_array[i].format == dsc_info->format &&
			rc_range_array[i].dsc_bpc == dsc_info->dsc_bpc &&
			rc_range_array[i].dsc_bpp == dsc_info->dsc_bpp) {
			DPU_FB_DEBUG("[DSC]PPS rc_range gen_rc:%d, format:%d, dsc_bpc:%d, dsc_bpp:%d\n",
				dsc_info->gen_rc_params, dsc_info->format, dsc_info->dsc_bpc, dsc_info->dsc_bpp);
			memcpy(dsc_info->rc_range, rc_range_array[i].p_rc_table,
				sizeof(struct rc_table_param) * rc_range_array[i].rc_table_len);
			break;
		}
	}
}

static void dsc_calc_rc_range_params(struct dsc_info *dsc_info)
{
	dpu_check_and_no_retval((dsc_info == NULL), ERR, "NULL POINTER");

	dsc_info->rc_edge_factor = RC_EDGE_FACTOR;
	dsc_info->rc_tgt_offset_lo = RC_TGT_OFFSET_LOW;
	dsc_info->rc_tgt_offset_hi = RC_TGT_OFFSET_HIGH;

	if (dsc_info->dsc_bpc == DSC_8BPC) {
		dsc_info->rc_quant_incr_limit0 = RC_QUANT_INCR_LIMIT0_8BPC;
		dsc_info->rc_quant_incr_limit1 = RC_QUANT_INCR_LIMIT1_8BPC;
	} else if (dsc_info->dsc_bpc == DSC_10BPC) {
		dsc_info->rc_quant_incr_limit0 = RC_QUANT_INCR_LIMIT0_10BPC;
		dsc_info->rc_quant_incr_limit1 = RC_QUANT_INCR_LIMIT1_10BPC;
	} else if (dsc_info->dsc_bpc == DSC_12BPC) {
		dsc_info->rc_quant_incr_limit0 = RC_QUANT_INCR_LIMIT0_12BPC;
		dsc_info->rc_quant_incr_limit1 = RC_QUANT_INCR_LIMIT1_12BPC;
	} else {
		DPU_FB_ERR("[DSC]BPC error\n");
	}
	range_check("rc_tgt_offset_hi", dsc_info->rc_tgt_offset_hi, 0, 15);
	range_check("rc_tgt_offset_lo", dsc_info->rc_tgt_offset_lo, 0, 15);
	range_check("rc_edge_factor", dsc_info->rc_edge_factor, 0, 15);
	range_check("rc_quant_incr_limit1", dsc_info->rc_quant_incr_limit1, 0, 31);
	range_check("rc_quant_incr_limit0", dsc_info->rc_quant_incr_limit0, 0, 31);
	DPU_FB_DEBUG("[DSC]PPS RC_EDGE_FACTOR = %u\n", dsc_info->rc_edge_factor);
	DPU_FB_DEBUG("[DSC]PPS RC_TGT_OFFSET_LOW = %u\n", dsc_info->rc_tgt_offset_lo);
	DPU_FB_DEBUG("[DSC]PPS RC_TGT_OFFSET_HIGH = %u\n", dsc_info->rc_tgt_offset_hi);
	DPU_FB_DEBUG("[DSC]PPS RC_QUANT_INCR_LIMIT0 = %u\n", dsc_info->rc_quant_incr_limit0);
	DPU_FB_DEBUG("[DSC]PPS RC_QUANT_INCR_LIMIT1 = %u\n", dsc_info->rc_quant_incr_limit1);
}

static void get_dsc_basic_info(const struct input_dsc_info *input_para, struct dsc_info *output_para)
{
	dpu_check_and_no_retval((input_para == NULL || output_para == NULL), ERR, "NULL POINTER");

	output_para->dsc_version_major = 1 >> DP_DSC_MAJOR_SHIFT;
	if (input_para->dsc_version == DSC_VERSION_V_1_1) {
		range_check("linebuf_depth", input_para->linebuf_depth, 8, 13);
		output_para->dsc_version_minor = 1;
		if (input_para->dsc_bpc != DSC_8BPC && input_para->dsc_bpc != DSC_10BPC && input_para->dsc_bpc != DSC_12BPC)
			DPU_FB_ERR("bits_per_component must be either 8, 10, or 12\n");
	} else if (input_para->dsc_version == DSC_VERSION_V_1_2) {
		range_check("linebuf_depth", input_para->linebuf_depth, 8, 16);
		output_para->dsc_version_minor = 2;
		if (input_para->dsc_bpc != DSC_8BPC && input_para->dsc_bpc != DSC_10BPC && input_para->dsc_bpc != DSC_12BPC &&
			input_para->dsc_bpc != DSC_14BPC && input_para->dsc_bpc != DSC_16BPC)
			DPU_FB_ERR("bits_per_component must be either 8, 10, 12, 14, or 16\n");
	}

	output_para->dsc_bpc = input_para->dsc_bpc;
	output_para->dsc_bpp = input_para->dsc_bpp;
	range_check("pic_width", input_para->pic_width, 1, 65535);
	range_check("pic_height", input_para->pic_height, 1, 65535);
	output_para->pic_width = input_para->pic_width;
	output_para->pic_height = input_para->pic_height;
	output_para->slice_width = input_para->slice_width;
	output_para->slice_height = input_para->slice_height;
	range_check("block_pred_enable", input_para->block_pred_enable, 0, 1);
	output_para->block_pred_enable = input_para->block_pred_enable;
	output_para->format = input_para->format;
	/*
	 * linebuf_depth:Contains the line buffer bit depth used to generate the bitstream. If a component’s
	 * bit depth (after color space conversion; see Section 6.1) is greater than this value,
	 * the line storage rounds the reconstructed values to this number of bits.
	 */
	output_para->linebuf_depth = input_para->linebuf_depth;
	output_para->gen_rc_params = input_para->gen_rc_params;
	if (output_para->format == DSC_RGB) {
		output_para->convert_rgb = 1;
		output_para->native_422 = 0;
		output_para->native_420 = 0;
		output_para->simple_422 = 0;
	} else {
		DPU_FB_DEBUG("[DSC] native_422 = 1\n");
		output_para->convert_rgb = 0;
		output_para->native_422 = 1;
		output_para->simple_422 = 0;
		output_para->native_420 = 0;
	}
}

static void vesa_dsc_init(struct dsc_info *dsc_info)
{
	dsc_info->rc_model_size = RC_MODEL_SIZE;
	if (dsc_info->format == DSC_RGB) {
		if (dsc_info->dsc_bpp == DSC_8BPP) { // used by syno/hisi dp 8bpc_8bpp
			dsc_info->initial_offset = INITIAL_OFFSET_8BPP;
			dsc_info->initial_xmit_delay = INITIAL_DELAY_8BPP;
		} else if (dsc_info->dsc_bpp == DSC_10BPP) { // used by 36870 panel 10bpc_10bpp
			dsc_info->initial_offset = INITIAL_OFFSET_10BPP;
			dsc_info->initial_xmit_delay = INITIAL_DELAY_10BPP_RGB_DEFAULT;
		} else if (dsc_info->dsc_bpp == DSC_12BPP) {
			dsc_info->initial_offset = INITIAL_OFFSET_12BPP_RGB_DEFAULT;
			dsc_info->initial_xmit_delay = INITIAL_DELAY_12BPP_RGB_DEFAULT;
		} else {
			DPU_FB_ERR("[DSC] dsc_calc_rc_model_size error\n");
		}
	} else if (dsc_info->format == DSC_YUV422) {
		if (dsc_info->dsc_bpp == DSC_12BPP) { // used by 37700 panel 8bpc_12bpp
			dsc_info->initial_offset = INITIAL_OFFSET_12BPP_YUV422_DEFAULT;
			dsc_info->initial_xmit_delay = INITIAL_DELAY_12BPP_YUV422_DEFAULT;
		} else if (dsc_info->dsc_bpp == DSC_14BPP) {
			dsc_info->initial_offset = INITIAL_OFFSET_14BPP_YUV422_DEFAULT;
			dsc_info->initial_xmit_delay = INITIAL_DELAY_14BPP_YUV422_DEFAULT;
		} else if (dsc_info->dsc_bpp == DSC_16BPP) { // used by noah panel 8bpc_16bpp
			dsc_info->initial_offset = INITIAL_OFFSET_16BPP_YUV422_DEFAULT;
			dsc_info->initial_xmit_delay = INITIAL_DELAY_16BPP_YUV422_DEFAULT;
		}
	}
}

static void vesa_dsc_pps_calculation(struct dsc_info *dsc_info)
{
	dpu_check_and_no_retval((dsc_info == NULL), ERR, "NULL POINTER");

	dsc_calc_chunk_size(dsc_info);
	dsc_calc_groups_per_line(dsc_info);
	dsc_calc_first_line_bpg_offset(dsc_info);
	dsc_calc_second_line_bpg_offset(dsc_info);
	dsc_calc_initial_scale_value(dsc_info);
	dsc_calc_rc_model_size(dsc_info);

	dsc_calc_scale_decrement_interval(dsc_info);
	dsc_calc_nfl_bpg_offset(dsc_info);
	dsc_calc_final_offset(dsc_info);
	dsc_calc_slice_bpg_offset(dsc_info);
	dsc_calc_initial_dec_delay(dsc_info);
	dsc_calc_scale_increment_interval(dsc_info);
	dsc_calc_flatness_qp(dsc_info);
	dsc_calc_rc_range(dsc_info);
	dsc_calc_rc_range_params(dsc_info);
}

static void vesa_dsc_info_calc(const struct input_dsc_info *input_para,
	struct dsc_info *output_para, struct dsc_info *exist_para)
{
	DPU_FB_DEBUG("[DSC] +!\n");
	dpu_check_and_no_retval(!output_para, ERR, "NULL POINTER");

	if (exist_para != NULL && input_para == NULL) {
		memcpy(output_para, exist_para, sizeof(struct dsc_info));
	} else {
		get_dsc_basic_info(input_para, output_para);
		vesa_dsc_init(output_para);
		vesa_dsc_pps_calculation(output_para);
		vesa_dsc_info_print(output_para);
	}
}

void init_dsc_algorithm(struct hisi_dsc_algorithm *p)
{
	dpu_check_and_no_retval(!p, ERR, "NULL POINTER");

	p->vesa_dsc_info_calc = vesa_dsc_info_calc;
}
