
/*lint -e558 -e559 -e620*/
#include <securec.h>

#include "dsc_algorithm.h"
#include "utilities/platform.h"
#include "utilities/hisi_debug.h"
#include "../hisi_dp_drv.h"
#include "../hisi_defs.h"

static struct rc_table_param g_rc_range_params_8bpc_8bpp[DSC_RC_RANGE_LENGTH] = {
	{ 0, 4, 2 }, { 0, 4, 0 }, { 1, 5, 0 }, { 1, 6, -2 }, { 3, 7, -4 },
	{ 3, 7, -6 }, { 3, 7, -8 }, { 3, 8, -8 }, { 3, 9, -8 }, { 3, 10, -10 },
	{ 5, 10, -10 }, { 5, 11, -12 }, { 5, 11, -12 }, { 9, 12, -12 }, { 12, 13, -12 },
};

static struct rc_table_param g_rc_range_params_8bpc_12bpp_yuv[DSC_RC_RANGE_LENGTH] = {
	{ 0, 4, 2 }, { 0, 4, 0 }, { 1, 5, 0 }, { 1, 6, -2 }, { 3, 7, -4 },
	{ 3, 7, -6 }, { 3, 7, -8 }, { 3, 8, -8 }, { 3, 9, -8 }, { 3, 10, -10 },
	{ 5, 10, -10 }, { 5, 11, -12 }, { 5, 11, -12 }, { 9, 12, -12 }, { 12, 13, -12 },
};

static struct rc_table_param g_rc_range_params_10bpc_10bpp[DSC_RC_RANGE_LENGTH] = {
	{ 0, 7, 2 }, { 4, 8, 0 }, { 5, 9, 0 }, { 6, 10, -2 }, { 7, 11, -4 },
	{ 7, 11, -6 }, { 7, 11, -8 }, { 7, 12, -8 }, { 7, 13, -8 }, { 7, 13, -10 },
	{ 9, 14, -10 }, { 9, 15, -10 }, { 9, 16, -12 }, { 11, 17, -12 }, { 17, 19, -12 },
};

static struct rc_table_param g_rc_range_params_8bpc_12bpp[DSC_RC_RANGE_LENGTH] = {
	{ 0, 2, 2 }, { 0, 3, 0 }, { 1, 4, 0 }, { 1, 5, -2 }, { 3, 6, -4 },
	{ 3, 6, -6 }, { 3, 7, -8 }, { 3, 8, -8 }, { 3, 8, -8 }, { 3, 9, -10 },
	{ 5, 9, -10 }, { 5, 9, -10 }, { 5, 9, -12 }, { 7, 10, -12 }, { 10, 11, -12 },
};

static uint32_t g_rc_buf_threshold[DSC_THRESH_LENGTH] = {
	896, 1792, 2688, 3584, 4480, 5376, 6272, 6720, 7168, 7616, 7744, 7872, 8000, 8064,
};

static void dsc_calc_chunk_size(struct dsc_info *dsc_info)
{
	if (dsc_info->format == DSC_RGB)
		dsc_info->chunk_size = round1((dsc_info->slice_width + 1 - 1) * dsc_info->dsc_bpp, 8);
	else
		dsc_info->chunk_size  = round1(((dsc_info->slice_width + 1 - 1) / 2) * dsc_info->dsc_bpp, 8);

	HISI_DRM_DEBUG("[DSC]PPS chunk_size = %u\n", dsc_info->chunk_size);
}

static void dsc_calc_groups_per_line(struct dsc_info *dsc_info)
{
	if (dsc_info->format == DSC_RGB)
		dsc_info->groups_per_line = (dsc_info->slice_width + 1 - 1) / PIXELS_PER_GROUP;
	else
		dsc_info->groups_per_line = ((dsc_info->slice_width + 1 - 1) >> 1) / PIXELS_PER_GROUP;

	HISI_DRM_DEBUG("[DSC]PPS groups_per_line = %u\n", dsc_info->groups_per_line);
}

static void dsc_calc_first_line_bpg_offset(struct dsc_info *dsc_info)
{
	u8 first_line_bpg_offset;
	u16 slice_height = dsc_info->slice_height;

	HISI_DRM_DEBUG("[DSC]PPS slice_height = %u\n", slice_height);
	/*
	 * first_line_bpg_offset: Specifies the number of additional bits
	 * that are allocated for each group on the first line of a slice.
	 * Those magice numbers were assigned by the DSC protocol
	 */
	if (slice_height >= 8)
		first_line_bpg_offset = 12 +  ((9 * min(34, slice_height - 8)) / 100);
	else
		first_line_bpg_offset = 2 * (slice_height - 1);
	HISI_DRM_DEBUG("[DSC]PPS first_line_bpg_offset = %u\n", first_line_bpg_offset);
	dsc_info->first_line_bpg_offset = first_line_bpg_offset;
}

static void dsc_calc_second_line_bpg_offset(struct dsc_info *dsc_info)
{
	if (dsc_info->format == YCBCR420) {
		if (dsc_info->slice_height < 8)
			dsc_info->second_line_bpg_offset = 2 * (dsc_info->slice_height - 1);
	} else {
		dsc_info->second_line_bpg_offset = 0;
	}
	HISI_DRM_DEBUG("[DSC] %s PPS second_line_bpg_offset = %d\n", dsc_info->second_line_bpg_offset);
}

static void dsc_calc_rc_model_size(struct dsc_info *dsc_info)
{
	dsc_info->rc_model_size = RC_MODEL_SIZE;
	if (dsc_info->format == DSC_RGB) {
		if (dsc_info->dsc_bpp == DSC_8BPP) {
			dsc_info->initial_offset = INITIAL_OFFSET_8BPP;
			dsc_info->initial_xmit_delay = INITIAL_DELAY_8BPP;
		} else if (dsc_info->dsc_bpp == DSC_10BPP) {
			dsc_info->initial_offset = INITIAL_OFFSET_10BPP;
			dsc_info->initial_xmit_delay = INITIAL_DELAY_10BPP;
		} else if (dsc_info->dsc_bpp == DSC_12BPP) {
			dsc_info->initial_offset = INITIAL_OFFSET_12BPP;
			dsc_info->initial_xmit_delay = INITIAL_DELAY_12BPP;
		} else {
			HISI_DRM_ERR("[DSC] dptx_calc_rc_model_size error\n");
		}
	} else {
		if (dsc_info->dsc_bpp == DSC_12BPP && dsc_info->dsc_bpc == DSC_8BPC) {
			dsc_info->initial_offset = INITIAL_OFFSET_12BPP_8BPC_YUV422;
			dsc_info->initial_xmit_delay = INITIAL_DELAY_12BPP_8BPC_YUV422;
		}
	}

	HISI_DRM_DEBUG("[DSC] PPS DSC rc_model_size is %u\n", dsc_info->rc_model_size);
	HISI_DRM_DEBUG("[DSC] PPS DSC initial_offset is %u\n", dsc_info->initial_offset);
	HISI_DRM_DEBUG("[DSC] PPS DSC initial_delay is %u\n", dsc_info->initial_xmit_delay);
}

static void dsc_calc_initial_dec_delay(struct dsc_info *dsc_info)
{
	u32 min_rate_buffer_size;
	u32 hrddelay;

	switch (dsc_info->format) {
	case DSC_RGB:
		min_rate_buffer_size = (dsc_info->rc_model_size - dsc_info->initial_offset) +
			(dsc_info->initial_xmit_delay * dsc_info->dsc_bpp) +
			(dsc_info->groups_per_line * dsc_info->first_line_bpg_offset);
		break;
	case DSC_YUV:
		min_rate_buffer_size = (dsc_info->rc_model_size - dsc_info->initial_offset) +
			(dsc_info->initial_xmit_delay * dsc_info->dsc_bpp) +
			(dsc_info->groups_per_line * dsc_info->first_line_bpg_offset);
		break;
	default:
		break;
	}
	hrddelay = round1(min_rate_buffer_size, dsc_info->dsc_bpp);

	dsc_info->initial_dec_delay = hrddelay - dsc_info->initial_xmit_delay;
	HISI_DRM_DEBUG("[DSC]PPS hrddelay = %u\n", hrddelay);
	HISI_DRM_DEBUG("[DSC] PPS DSC min_rate_buffer_size is %u\n", min_rate_buffer_size);
	HISI_DRM_DEBUG("[DSC]PPS initial_dec_delay = %u\n", dsc_info->initial_dec_delay);
}

static void dsc_calc_initial_scale_value(struct dsc_info *dsc_info)
{
	dsc_info->initial_scale_value = 8 * dsc_info->rc_model_size /
		(dsc_info->rc_model_size - dsc_info->initial_offset);

	if (dsc_info->groups_per_line < dsc_info->initial_scale_value - 8)
		dsc_info->initial_scale_value = dsc_info->groups_per_line + 8;
	HISI_DRM_DEBUG("[DSC]PPS initial_scale_value = %u\n", dsc_info->initial_scale_value);
}

static void dsc_calc_scale_decrement_interval(struct dsc_info *dsc_info)
{
	if (dsc_info->initial_scale_value > 8)
		dsc_info->scale_decrement_interval = dsc_info->groups_per_line / (dsc_info->initial_scale_value - 8);
	else
		dsc_info->scale_decrement_interval = 4095;

	HISI_DRM_DEBUG("[DSC] %s PPS scale_decrement_interval = %d\n", __func__, dsc_info->scale_decrement_interval);
}

static void dsc_calc_nfl_bpg_offset(struct dsc_info *dsc_info)
{
	dsc_info->nfl_bpg_offset =
		(dsc_info->first_line_bpg_offset << OFFSET_FRACTIONAL_BITS) / (dsc_info->slice_height - 1);

	if (dsc_info->slice_height > 1)
		dsc_info->nfl_bpg_offset = (int)round1((dsc_info->first_line_bpg_offset << OFFSET_FRACTIONAL_BITS),
			(dsc_info->slice_height - 1));
	else
		dsc_info->nfl_bpg_offset = 0;
	HISI_DRM_DEBUG("[DSC]PPS nfl_bpg_offset = %u\n", dsc_info->nfl_bpg_offset);
}

static void dsc_calc_final_offset(struct dsc_info *dsc_info)
{
	u8 max_se_size_y, max_se_size_c;
	u32 num_extra_mux_bits;
	u32 mux_word_size;
	int slice_bits;

	if (dsc_info->dsc_bpc == DSC_16BPC)
		max_se_size_y = max_se_size_c = 64;
	else
		max_se_size_y = max_se_size_c = dsc_info->dsc_bpc * 4 + 4;
	/*
	 * The muxWordSize is determined by the bits_per_component value:
	 * When bits_per_component is 12, 14, or 16bpc, muxWordSize shall be equal to 64 bits
	 * When bits_per_component is 8 or 10bpc, muxWordSize shall be equal to 48 bits
	 * those magice numbers were assigned by the DSC protocol
	 */
	mux_word_size = (dsc_info->dsc_bpc < DSC_12BPC) ? 48 : 64;
	if (dsc_info->format == DSC_RGB)
		num_extra_mux_bits = (mux_word_size + max_se_size_y - 2) + 2 * (mux_word_size + max_se_size_c - 2);
	else
		num_extra_mux_bits = 4 * mux_word_size + max_se_size_y + 3 * (max_se_size_c - 4) - 2;
	slice_bits = 8 * dsc_info->chunk_size * dsc_info->slice_height;

	while ((num_extra_mux_bits > 0) && ((slice_bits - num_extra_mux_bits) % mux_word_size))
		num_extra_mux_bits--;

	dsc_info->final_offset = dsc_info->rc_model_size -
		((dsc_info->initial_xmit_delay * dsc_info->dsc_bpp * 16 + 8)>>4) + num_extra_mux_bits;
	HISI_DRM_DEBUG("[DSC]PPS slice_bits = %u\n", slice_bits);
	HISI_DRM_DEBUG("[DSC]PPS final_offset = %u\n", dsc_info->final_offset);
	HISI_DRM_DEBUG("[DSC]PPS num_extra_mux_bits = %u\n", num_extra_mux_bits);
}

static void dsc_calc_slice_bpg_offset(struct dsc_info *dsc_info)
{
	u8 max_se_size;
	u8 bpc;
	u16 group_per_line;
	u32 num_extra_mux_bits;
	u32 mux_word_size;
	int slice_bits;
	int groups_total;

	bpc = dsc_info->dsc_bpc;
	mux_word_size = (bpc < DSC_12BPC) ? 48 : 64;
	group_per_line = dsc_info->groups_per_line;
	groups_total = group_per_line * dsc_info->slice_height;
	HISI_DRM_DEBUG("[DSC]PPS groups_total = %u\n", groups_total);

	if (bpc == DSC_16BPC)
		max_se_size = 64;
	else
		max_se_size = bpc * 4 + 4;

	if (dsc_info->format == DSC_RGB)
		num_extra_mux_bits = (mux_word_size + max_se_size - 2) + 2 * (mux_word_size + max_se_size - 2);
	else
		num_extra_mux_bits = 4 * mux_word_size + max_se_size + 3 * (max_se_size - 4) - 2;

	slice_bits = 8 * dsc_info->chunk_size * dsc_info->slice_height;
	while ((num_extra_mux_bits > 0) && ((slice_bits - num_extra_mux_bits) % mux_word_size))
		num_extra_mux_bits--;
	dsc_info->slice_bpg_offset = round1((1 << OFFSET_FRACTIONAL_BITS) *
		(dsc_info->rc_model_size - dsc_info->initial_offset + num_extra_mux_bits), groups_total);

	HISI_DRM_DEBUG("[DSC]PPS slice_bpg_offset = %u\n", dsc_info->slice_bpg_offset);
}

static void dsc_calc_scale_increment_interval(struct dsc_info *dsc_info)
{
	int final_scale_value;

	final_scale_value = dsc_info->rc_model_size / (dsc_info->rc_model_size - dsc_info->final_offset);
	final_scale_value = 8 * dsc_info->rc_model_size / (dsc_info->rc_model_size - dsc_info->final_offset);
	HISI_DRM_DEBUG("[DSC]PPS final_scale_value = %d\n", final_scale_value);
	/*
	 * scale_increment_interval:At the end of the slice, it is also desirable to shrink the effective RC model range
	 * to maximize tracking ability. It is recommended to use a value equal to
	 * (final_offset / (nfl_bpg_offset + slice_bpg_offset)) / (8 * (finalScaleValue - 1.125)),
	 * where finalScaleValue is equal to rc_model_size / (rc_model_size - final_offset).
	 * If finalScaleValue is less than or equal to 9, a value of 0 should be used to disable
	 * the scale increment at the end of the slice. If the calculation for scale_increment_interval
	 * results in a value that exceeds 65535, a smaller slice height should be used so that the
	 * programmed scale_increment_interval fits within a 16-bit field.
	 * those magice numbers were assigned by the DSC protocol.
	 */
	dsc_info->scale_increment_interval = (1 << OFFSET_FRACTIONAL_BITS) *
		(dsc_info->final_offset / (((8 * final_scale_value) - 9) *
		 (dsc_info->nfl_bpg_offset + dsc_info->slice_bpg_offset)));

	if (final_scale_value > 9) {
		dsc_info->scale_increment_interval = (1 << OFFSET_FRACTIONAL_BITS) * dsc_info->final_offset /
			((final_scale_value - 9) * (dsc_info->nfl_bpg_offset + dsc_info->slice_bpg_offset));
	} else {
		dsc_info->scale_increment_interval = 0;
	}
	HISI_DRM_DEBUG("[DSC]PPS scale_increment_interval = %u\n", dsc_info->scale_increment_interval);

	if (dsc_info->scale_increment_interval > 65535)
		HISI_DRM_INFO("[DSC] Scale increment interval exceeds 65535\n");
}

static void dsc_calc_flatness_qp(struct dsc_info *dsc_info)
{
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
		HISI_DRM_ERR("[DSC] dptx_calc_rc_model_size error\n");
	}
	HISI_DRM_DEBUG("[DSC]PPS flatness_min_qp = %u\n", dsc_info->flatness_min_qp);
	HISI_DRM_DEBUG("[DSC]PPS flatness_max_qp = %u\n", dsc_info->flatness_max_qp);
}

static void dsc_calc_rc_range(struct dsc_info *dsc_info)
{
	int i;
	int ret;
	if (dsc_info->format == DSC_RGB) {
		if (dsc_info->dsc_bpp == DSC_8BPP && dsc_info->dsc_bpc == DSC_8BPC) {
			HISI_DRM_DEBUG("[DSC] %s PPS g_rc_range_params_8bpc_8bpp\n", __func__);
			ret = memcpy_s(&(dsc_info->rc_range[0]), sizeof(struct rc_table_param) * DSC_RC_RANGE_LENGTH,
				&(g_rc_range_params_8bpc_8bpp[0]), sizeof(struct rc_table_param) * DSC_RC_RANGE_LENGTH);
			drm_check_and_void_return(ret != EOK, "[DSC] memcpy for rc_range failed!\n");
		} else if (dsc_info->dsc_bpp == DSC_12BPP && dsc_info->dsc_bpc == DSC_8BPC) {
			HISI_DRM_DEBUG("[DSC] %s PPS g_rc_range_params_8bpc_12bpp\n", __func__);
			ret = memcpy_s(&(dsc_info->rc_range[0]), sizeof(struct rc_table_param) * DSC_RC_RANGE_LENGTH,
				&(g_rc_range_params_8bpc_12bpp[0]), sizeof(struct rc_table_param) * DSC_RC_RANGE_LENGTH);
			drm_check_and_void_return(ret != EOK, "[DSC] memcpy for rc_range failed!\n");
		} else if (dsc_info->dsc_bpp == DSC_10BPP && dsc_info->dsc_bpc == DSC_10BPC) {
			HISI_DRM_DEBUG("[DSC] %s PPS g_rc_range_params_10bpc_10bpp\n", __func__);
			ret = memcpy_s(&(dsc_info->rc_range[0]), sizeof(struct rc_table_param) * DSC_RC_RANGE_LENGTH,
				&(g_rc_range_params_10bpc_10bpp[0]), sizeof(struct rc_table_param) * DSC_RC_RANGE_LENGTH);
			drm_check_and_void_return(ret != EOK, "[DSC] memcpy for rc_range failed!\n");
		} else {
			HISI_DRM_ERR("[DSC] dsc calc rc range bpp value error\n");
		}
	} else {
		HISI_DRM_DEBUG("[DSC] %s PPS g_rc_range_params_8bpc_12bpp_yuv\n", __func__);
		if (dsc_info->dsc_bpp == DSC_12BPP && dsc_info->dsc_bpc == DSC_8BPC) {
			ret = memcpy_s(&(dsc_info->rc_range[0]), sizeof(struct rc_table_param) * DSC_RC_RANGE_LENGTH,
				&(g_rc_range_params_8bpc_12bpp_yuv[0]), sizeof(struct rc_table_param) * DSC_RC_RANGE_LENGTH);
			drm_check_and_void_return(ret != EOK, "[DSC] memcpy for rc_range failed!\n");
		}
	}

	for (i = 0; i < DSC_THRESH_LENGTH; ++i)
		dsc_info->rc_buf_thresh[i] = (g_rc_buf_threshold[i] >> 6);
}

static void dsc_calc_rc_range_params(struct dsc_info *dsc_info)
{
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
		HISI_DRM_ERR("[DSC] dptx_calc_rc_range_bal error\n");
	}
	HISI_DRM_DEBUG("[DSC]PPS RC_EDGE_FACTOR = %u\n", dsc_info->rc_edge_factor);
	HISI_DRM_DEBUG("[DSC]PPS RC_TGT_OFFSET_LOW = %u\n", dsc_info->rc_tgt_offset_lo);
	HISI_DRM_DEBUG("[DSC]PPS RC_TGT_OFFSET_HIGH = %u\n", dsc_info->rc_tgt_offset_hi);
	HISI_DRM_DEBUG("[DSC]PPS RC_QUANT_INCR_LIMIT0 = %u\n", dsc_info->rc_quant_incr_limit0);
	HISI_DRM_DEBUG("[DSC]PPS RC_QUANT_INCR_LIMIT1 = %u\n", dsc_info->rc_quant_incr_limit1);
}

static void get_dsc_basic_info(const struct input_dsc_info *input_para, struct dsc_info *output_para)
{
	HISI_DRM_DEBUG("[DSC] +\n");
	drm_check_and_void_return((input_para == NULL), "[DP] NULL Pointer\n");
	drm_check_and_void_return((output_para == NULL), "[DP] NULL Pointer\n");

	output_para->dsc_version_major = 1 >> DP_DSC_MAJOR_SHIFT;
	if (input_para->dsc_version == DSC_VERSION_V_1_1)
		output_para->dsc_version_minor = 1;
	else if (input_para->dsc_version == DSC_VERSION_V_1_2)
		output_para->dsc_version_minor = 2;

	output_para->dsc_bpc = input_para->dsc_bpc;
	output_para->dsc_bpp = input_para->dsc_bpp;
	output_para->pic_width = input_para->pic_width;
	output_para->pic_height = input_para->pic_height;
	output_para->slice_width = input_para->slice_width;
	output_para->slice_height = input_para->slice_height;
	output_para->block_pred_enable = input_para->block_pred_enable;
	output_para->format = input_para->format;
	output_para->linebuf_depth = input_para->linebuf_depth;
	if (output_para->format == DSC_RGB) {
		output_para->native_422 = 0;
		output_para->native_420 = 0;
	} else {
		HISI_DRM_DEBUG("[DSC] native_422 = 1\n");
		output_para->native_422 = 1;
	}
}

static void vesa_dsc_pps_calculation(struct dsc_info *dsc_info)
{
	drm_check_and_void_return((dsc_info == NULL), "[DP] NULL Pointer\n");

	dsc_calc_chunk_size(dsc_info);
	dsc_calc_groups_per_line(dsc_info);
	dsc_calc_first_line_bpg_offset(dsc_info);
	dsc_calc_second_line_bpg_offset(dsc_info);
	dsc_calc_rc_model_size(dsc_info);
	dsc_calc_initial_dec_delay(dsc_info);
	dsc_calc_initial_scale_value(dsc_info);
	dsc_calc_scale_decrement_interval(dsc_info);
	dsc_calc_nfl_bpg_offset(dsc_info);
	dsc_calc_final_offset(dsc_info);
	dsc_calc_slice_bpg_offset(dsc_info);
	dsc_calc_scale_increment_interval(dsc_info);
	dsc_calc_flatness_qp(dsc_info);
	dsc_calc_rc_range(dsc_info);
	dsc_calc_rc_range_params(dsc_info);
}

void vesa_dsc_info_calc(const struct input_dsc_info *input_para, struct dsc_info *output_para,
	struct dsc_info *exist_para)
{
	int ret;
	HISI_DRM_DEBUG("[DSC] +\n");
	if (exist_para != NULL && input_para == NULL) {
		HISI_DRM_DEBUG("[DSC] copy vesa dsc info calc!\n");
		ret = memcpy_s(output_para, sizeof(struct dsc_info), exist_para, sizeof(struct dsc_info));
		drm_check_and_void_return(ret != EOK, "memcpy failed");
	} else {
		get_dsc_basic_info(input_para, output_para);
		vesa_dsc_pps_calculation(output_para);
		vesa_dsc_info_print(output_para);
	}
}

void init_dp_dsc_algorithm(struct dp_dsc_algorithm *p)
{
	HISI_DRM_DEBUG("[DSC] +\n");
	if (!p) {
		HISI_DRM_DEBUG("p is null!!!");
		return;
	}
	p->vesa_dsc_info_calc = vesa_dsc_info_calc;
}

/*lint +e558 +e559 +e620*/
