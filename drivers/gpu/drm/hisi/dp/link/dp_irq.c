/*
 * Copyright (c) 2016 Synopsys, Inc.
 *
 * Synopsys DP TX Linux Software Driver and documentation (hereinafter,
 * "Software") is an Unsupported proprietary work of Synopsys, Inc. unless
 * otherwise expressly agreed to in writing between Synopsys and you.
 *
 * The Software IS NOT an item of Licensed Software or Licensed Product under
 * any End User Software License Agreement or Agreement for Licensed Product
 * with Synopsys or any supplement thereto. You are permitted to use and
 * redistribute this Software in source and binary forms, with or without
 * modification, provided that redistributions of source code must retain this
 * notice. You may not view, use, disclose, copy or distribute this file or
 * any information contained herein except pursuant to this license grant from
 * Synopsys. If you do not agree with this notice, including the disclaimer
 * below, then you are not authorized to use the Software.
 *
 * THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS" BASIS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/*
 * Copyright (c) 2017 Hisilicon Tech. Co., Ltd. Integrated into the Hisilicon display system.
 */
#include <securec.h>

#include <drm/drm_crtc_helper.h>
#include <drm/drm_connector.h>

#include "hisi_dp_drv.h"
#include "hisi_defs.h"

#include "../controller/dp_core_interface.h"
#include "../controller/dp_avgen_base.h"

#include "dp_link_training.h"
#include "dp_irq.h"
#include "dp_edid.h"
#include "dp_aux.h"
#include "dp_dsc_algorithm.h"

#include "hisi_panel.h"
#include "hisi_mipi_dsi.h"
#include "hisi_drm_dpe_utils.h"
#include <linux/hisi/usb/hisi_usb.h>

/*lint -save -e* */
#define SAFE_MODE_TIMING_HACTIVE 640
/* The pixel clock of 640 * 480 = 25175. The saving pixel clock need 1/10 */
#define SAFE_MODE_TIMING_PIXEL_CLOCK 2517

#define DPTX_CHECK_TIME_PERIOD 2000

enum dp_event_type {
	DP_LINK_STATE_BAD = 0,
	DP_LINK_STATE_GOOD = 1
};

#if CONFIG_DP_ENABLE
extern void dp_send_event(enum dp_event_type event);
#endif

const struct dp_test_link_timming timming_table[] = {
	/* h_total, v_total, h_start, v_start, h_width, v_width, h_sync_width, v_sync_width, refresh_rate, v_mode,
	 * video_format
	 */
	{ link_timming_descriptor(1056, 628,  216, 27, 128, 4, 800,  600,  0,      9,  DMT) },
	{ link_timming_descriptor(1088, 517,  224, 31, 112, 8, 848,  480,  0,      14, DMT) },
	{ link_timming_descriptor(1344, 806,  296, 35, 136, 6, 1024, 768,  0,      16, DMT) },
	{ link_timming_descriptor(1440, 790,  112, 19, 32,  7, 1280, 768,  0,      22, DMT) },
	{ link_timming_descriptor(1664, 798,  320, 27, 128, 7, 1280, 768,  0,      23, DMT) },
	{ link_timming_descriptor(1440, 823,  112, 20, 32,  6, 1280, 800,  0,      27, DMT) },
	{ link_timming_descriptor(1800, 1000, 424, 39, 112, 3, 1280, 960,  0,      32, DMT) },
	{ link_timming_descriptor(1688, 1066, 360, 41, 112, 3, 1280, 1024, 0,      35, DMT) },
	{ link_timming_descriptor(1792, 795,  368, 24, 112, 6, 1360, 768,  0,      39, DMT) },
	{ link_timming_descriptor(1560, 1080, 112, 27, 32,  4, 1400, 1050, 0,      41, DMT) },
	{ link_timming_descriptor(2160, 1250, 496, 49, 192, 3, 1600, 1200, 0,      51, DMT) },
	{ link_timming_descriptor(2448, 1394, 528, 49, 200, 3, 1792, 1344, 0,      62, DMT) },
	{ link_timming_descriptor(2600, 1500, 552, 59, 208, 3, 1920, 1440, 0,      73, DMT) },
	{ link_timming_descriptor(2200, 1125, 192, 41, 44,  5, 1920, 1080, 120000, 63, VCEA) },
	{ link_timming_descriptor(2200, 1125, 192, 41, 44,  5, 1920, 1080, 0,      82, DMT) },
	{ link_timming_descriptor(800,  525,  144, 35, 96,  2, 640,  480,  0,      1,  VCEA) },
	{ link_timming_descriptor(1650, 750,  260, 25, 40,  5, 1280, 720,  0,      4,  VCEA) },
	{ link_timming_descriptor(1680, 831,  328, 28, 128, 6, 1280, 800,  0,      28, CVT) },
	{ link_timming_descriptor(1760, 1235, 112, 32, 32,  4, 1600, 1200, 0,      40, CVT) },
	{ link_timming_descriptor(2208, 1580, 112, 41, 32,  4, 2048, 536,  0,      41, CVT) },
};

static int dptx_cancel_detect_work(struct dp_ctrl *dptx)
{
	HISI_DRM_INFO("[DP] Cancel Detect work\n");

	if (dptx->dptx_detect_inited) {
		if (dptx->dptx_check_wq != NULL) {
			destroy_workqueue(dptx->dptx_check_wq);
			dptx->dptx_check_wq = NULL;
		}

		hrtimer_cancel(&dptx->dptx_hrtimer);

		dptx->dptx_detect_inited = false;
	}

	return 0;
}

static int handle_test_link_training(struct dp_ctrl *dptx)
{
	int retval;
	u8 lanes;
	u8 rate;
	struct video_params *vparams = NULL;
	struct dtd *mdtd = NULL;

	retval = dptx_read_dpcd(dptx, DP_TEST_LINK_RATE, &rate);
	if (retval)
		return retval;

	retval = dptx_bw_to_phy_rate(rate);
	if (retval < 0)
		return retval;

	rate = retval;

	retval = dptx_read_dpcd(dptx, DP_TEST_LANE_COUNT, &lanes);
	if (retval)
		return retval;

	HISI_DRM_INFO("[DP]: Strating link training rate=%d, lanes=%d\n",
		 rate, lanes);

	vparams = &dptx->vparams;
	mdtd = &vparams->mdtd;
	dptx->link.lanes = lanes;
	dptx->link.rate = rate;

	if (dptx->dptx_video_ts_calculate)
		retval = dptx->dptx_video_ts_calculate(dptx, vparams,
					mdtd->pixel_clock);
	if (retval)
		return retval;

	retval = dptx_link_training(dptx, rate, lanes);
	if (retval)
		HISI_DRM_ERR("[DP] Link training failed %d\n", retval);
	else
		HISI_DRM_INFO("[DP] Link training succeeded\n");

	return retval;
}

static int handle_test_read_video_timming(struct dp_ctrl *dptx,
	struct dp_test_link_timming *timming)
{
	int retval;
	u8 bytes[19] = {0};

	retval = dptx_read_bytes_from_dpcd(dptx, DP_TEST_H_TOTAL_MSB, bytes, sizeof(bytes));
	if (retval)
		return retval;

	timming->h_total = (bytes[0] << 8) | bytes[1];
	timming->v_total = (bytes[2] << 8) | bytes[3];
	timming->h_start = (bytes[4] << 8) | bytes[5];
	timming->v_start = (bytes[6] << 8) | bytes[7];
	timming->h_sync_width = ((bytes[8] & (~(1 << 7))) << 8) | bytes[9];
	timming->v_sync_width = ((bytes[10] & (~(1 << 7))) << 8) | bytes[11];
	timming->h_width = (bytes[12] << 8) | bytes[13];
	timming->v_width = (bytes[14] << 8) | bytes[15];
	timming->refresh_rate = bytes[18] * 1000;

	HISI_DRM_INFO("[DP] h_total = %d\n", timming->h_total);
	HISI_DRM_INFO("[DP] v_total = %d\n", timming->v_total);
	HISI_DRM_INFO("[DP] h_start = %d\n", timming->h_start);
	HISI_DRM_INFO("[DP] v_start = %d\n", timming->v_start);
	HISI_DRM_INFO("[DP] h_sync_width = %d\n", timming->h_sync_width);
	HISI_DRM_INFO("[DP] v_sync_width = %d\n", timming->v_sync_width);
	HISI_DRM_INFO("[DP] h_width = %d\n", timming->h_width);
	HISI_DRM_INFO("[DP] v_width = %d\n", timming->v_width);
	HISI_DRM_INFO("[DP] refresh_rate = %d\n", timming->refresh_rate);

	return 0;
}

static int handle_test_get_video_mode(struct dp_test_link_timming timming, u8 *vmode,
	enum video_format_type *video_format)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(timming_table); i++) {
		if (timming.h_total == timming_table[i].h_total &&
			timming.v_total == timming_table[i].v_total &&
			timming.h_start == timming_table[i].h_start &&
			timming.v_start == timming_table[i].v_start &&
			timming.h_width == timming_table[i].h_width &&
			timming.v_width == timming_table[i].v_width &&
			timming.h_sync_width == timming_table[i].h_sync_width &&
			timming.v_sync_width == timming_table[i].v_sync_width &&
			timming.refresh_rate == timming_table[i].refresh_rate) {
			*vmode = timming_table[i].v_mode;
			*video_format = timming_table[i].video_format;
			return 0;
		}
	}
	HISI_DRM_INFO("[DP] Unknown video mode\n");
	return -EINVAL;

}

static int handle_test_link_video_timming(struct dp_ctrl *dptx, int stream)
{
	int retval;
	enum video_format_type video_format;
	u8 vmode;
	struct video_params *vparams = NULL;
	struct dtd mdtd;
	struct dp_test_link_timming timming = {0};

	retval = handle_test_read_video_timming(dptx, &timming);
	if (retval)
		return retval;

	retval = handle_test_get_video_mode(timming, &vmode, &video_format);
	if (retval)
		return retval;

	if (!convert_code_to_dtd(&mdtd, vmode, timming.refresh_rate, video_format)) {
		HISI_DRM_INFO("[DP]: Invalid video mode value %d\n", vmode);
		return -EINVAL;
	}
	vparams = &dptx->vparams;
	vparams->mdtd = mdtd;
	vparams->refresh_rate = timming.refresh_rate;
	if (dptx->dptx_video_ts_calculate) {
		retval = dptx->dptx_video_ts_calculate(dptx, vparams, mdtd.pixel_clock);
		if (retval)
			return retval;
       }

	/* MMCM */
	if (dptx->dptx_resolution_switch)
		dptx->dptx_resolution_switch(dptx->dp_dev, HOT_PLUG_TEST);
	if (dptx->dptx_video_timing_change)
		dptx->dptx_video_timing_change(dptx, 0);
	return 0;
}

static int handle_test_get_audio_channel_count(struct dp_ctrl *dptx, u8 test_audio_ch_count)
{
	struct audio_params *aparams = NULL;
	u8  audio_ch_count;

	aparams = &dptx->aparams;
	switch (test_audio_ch_count) {
	case DP_TEST_AUDIO_CHANNEL1:
		HISI_DRM_INFO("[DP] DP_TEST_AUDIO_CHANNEL1\n");
		audio_ch_count = 1;
		break;
	case DP_TEST_AUDIO_CHANNEL2:
		HISI_DRM_INFO("[DP] DP_TEST_AUDIO_CHANNEL2\n");
		audio_ch_count = 2;
		break;
	case DP_TEST_AUDIO_CHANNEL3:
		HISI_DRM_INFO("[DP] DP_TEST_AUDIO_CHANNEL3\n");
		audio_ch_count = 3;
		break;
	case DP_TEST_AUDIO_CHANNEL4:
		HISI_DRM_INFO("[DP] DP_TEST_AUDIO_CHANNEL4\n");
		audio_ch_count = 4;
		break;
	case DP_TEST_AUDIO_CHANNEL5:
		HISI_DRM_INFO("[DP] DP_TEST_AUDIO_CHANNEL5\n");
		audio_ch_count = 5;
		break;
	case DP_TEST_AUDIO_CHANNEL6:
		HISI_DRM_INFO("[DP] DP_TEST_AUDIO_CHANNEL6\n");
		audio_ch_count = 6;
		break;
	case DP_TEST_AUDIO_CHANNEL7:
		HISI_DRM_INFO("[DP] DP_TEST_AUDIO_CHANNEL7\n");
		audio_ch_count = 7;
		break;
	case DP_TEST_AUDIO_CHANNEL8:
		HISI_DRM_INFO("[DP] DP_TEST_AUDIO_CHANNEL8\n");
		audio_ch_count = 8;
		break;
	default:
		HISI_DRM_INFO("[DP] Invalid TEST_AUDIO_CHANNEL_COUNT\n");
		return -EINVAL;
	}
	HISI_DRM_INFO("[DP] test_audio_ch_count = %d\n", audio_ch_count);
	aparams->num_channels = audio_ch_count;

	return 0;
}

static int handle_test_get_audio_sample_params(struct dp_ctrl *dptx, u8 test_audio_smaple_range)
{
	struct audio_params *aparams = NULL;
	u8  orig_sample_freq, sample_freq;
	u32 audio_clock_freq;

	aparams = &dptx->aparams;
	switch (test_audio_smaple_range) {
	case DP_TEST_AUDIO_SAMPLING_RATE_32:
		HISI_DRM_INFO("[DP] DP_TEST_AUDIO_SAMPLING_RATE_32\n");
		orig_sample_freq = 12;
		sample_freq = 3;
		audio_clock_freq = 320;
		break;
	case DP_TEST_AUDIO_SAMPLING_RATE_44_1:
		HISI_DRM_INFO("[DP] DP_TEST_AUDIO_SAMPLING_RATE_44_1\n");
		orig_sample_freq = 15;
		sample_freq = 0;
		audio_clock_freq = 441;
		break;
	case DP_TEST_AUDIO_SAMPLING_RATE_48:
		HISI_DRM_INFO("[DP] DP_TEST_AUDIO_SAMPLING_RATE_48\n");
		orig_sample_freq = 13;
		sample_freq = 2;
		audio_clock_freq = 480;
		break;
	case DP_TEST_AUDIO_SAMPLING_RATE_88_2:
		HISI_DRM_INFO("[DP] DP_TEST_AUDIO_SAMPLING_RATE_88_2\n");
		orig_sample_freq = 7;
		sample_freq = 8;
		audio_clock_freq = 882;
		break;
	case DP_TEST_AUDIO_SAMPLING_RATE_96:
		HISI_DRM_INFO("[DP] DP_TEST_AUDIO_SAMPLING_RATE_96\n");
		orig_sample_freq = 5;
		sample_freq = 10;
		audio_clock_freq = 960;
		break;
	case DP_TEST_AUDIO_SAMPLING_RATE_176_4:
		HISI_DRM_INFO("[DP] DP_TEST_AUDIO_SAMPLING_RATE_176_4\n");
		orig_sample_freq = 3;
		sample_freq = 12;
		audio_clock_freq = 1764;
		break;
	case DP_TEST_AUDIO_SAMPLING_RATE_192:
		HISI_DRM_INFO("[DP] DP_TEST_AUDIO_SAMPLING_RATE_192\n");
		orig_sample_freq = 1;
		sample_freq = 14;
		audio_clock_freq = 1920;
		break;
	default:
		HISI_DRM_INFO("[DP] Invalid TEST_AUDIO_SAMPLING_RATE\n");
		return -EINVAL;
	}
	HISI_DRM_INFO("[DP] sample_freq = %d\n", sample_freq);
	HISI_DRM_INFO("[DP] orig_sample_freq = %d\n", orig_sample_freq);

	aparams->iec_samp_freq = sample_freq;
	aparams->iec_orig_samp_freq = orig_sample_freq;

	return 0;
}

static int handle_test_link_audio_pattern(struct dp_ctrl *dptx)
{
	int retval;
	u8 test_audio_mode, test_audio_smaple_range, test_audio_ch_count;

	retval = dptx_read_dpcd(dptx, DP_TEST_AUDIO_MODE, &test_audio_mode);
	if (retval)
		return retval;

	HISI_DRM_INFO("[DP] test_audio_mode= %d\n", test_audio_mode);

	test_audio_smaple_range = test_audio_mode &
		DP_TEST_AUDIO_SAMPLING_RATE_MASK;
	test_audio_ch_count = (test_audio_mode & DP_TEST_AUDIO_CH_COUNT_MASK)
		>> DP_TEST_AUDIO_CH_COUNT_SHIFT;

	retval = handle_test_get_audio_channel_count(dptx, test_audio_ch_count);
	if (retval)
		return retval;

	retval = handle_test_get_audio_sample_params(dptx, test_audio_smaple_range);
	if (retval)
		return retval;

	if (dptx->dptx_audio_num_ch_change)
		dptx->dptx_audio_num_ch_change(dptx);
	if (dptx->dptx_audio_infoframe_sdp_send)
		dptx->dptx_audio_infoframe_sdp_send(dptx);

	return retval;
}

static int handle_get_dynamic_range(u8 misc, struct video_params *vparams)
{
	u8 dynamic_range;

	dynamic_range = (misc & DP_TEST_DYNAMIC_RANGE_MASK)
			>> DP_TEST_DYNAMIC_RANGE_SHIFT;
	switch (dynamic_range) {
	case DP_TEST_DYNAMIC_RANGE_VESA:
		HISI_DRM_INFO("[DP] Change video dynamic range to VESA\n");
		vparams->dynamic_range = VESA;
		break;
	case DP_TEST_DYNAMIC_RANGE_CEA:
		HISI_DRM_INFO("[DP] Change video dynamic range to CEA\n");
		vparams->dynamic_range = CEA;
		break;
	default:
		HISI_DRM_INFO("[DP] Invalid TEST_BIT_DEPTH\n");
		return -EINVAL;
	}
	return 0;
}

static int handle_get_colorimetry(u8 misc, struct video_params *vparams)
{
	u8 ycbcr_coeff;

	ycbcr_coeff = (misc & DP_TEST_YCBCR_COEFF_MASK)
			>> DP_TEST_YCBCR_COEFF_SHIFT;

	switch (ycbcr_coeff) {
	case DP_TEST_YCBCR_COEFF_ITU601:
		HISI_DRM_INFO("[DP]Change video colorimetry to ITU601\n");
		vparams->colorimetry = ITU601;
		break;
	case DP_TEST_YCBCR_COEFF_ITU709:
		HISI_DRM_INFO("[DP] Change video colorimetry to ITU709:\n");
		vparams->colorimetry = ITU709;
		break;
	default:
		HISI_DRM_INFO("[DP] Invalid TEST_BIT_DEPTH\n");
		return -EINVAL;
	}
	return 0;
}

static int handle_get_pix_enc(u8 misc, struct video_params *vparams)
{
	u8 color_format;

	color_format = misc & DP_TEST_COLOR_FORMAT_MASK;

	switch (color_format) {
	case DP_TEST_COLOR_FORMAT_RGB:
		HISI_DRM_INFO("[DP] DP_TEST_COLOR_FORMAT_RGB\n");
		vparams->pix_enc = RGB;
		break;
	case DP_TEST_COLOR_FORMAT_YCBCR422:
		HISI_DRM_INFO("[DP] DP_TEST_COLOR_FORMAT_YCBCR422\n");
		vparams->pix_enc = YCBCR422;
		break;
	case DP_TEST_COLOR_FORMAT_YCBCR444:
		HISI_DRM_INFO("[DP] DP_TEST_COLOR_FORMAT_YCBCR444\n");
		vparams->pix_enc = YCBCR444;
		break;
	default:
		HISI_DRM_INFO("[DP] Invalid  DP_TEST_COLOR_FORMAT\n");
		return -EINVAL;
	}
	return 0;
}

static int handle_get_bpc(u8 misc, struct video_params *vparams)
{
	u8 bpc;

	bpc = (misc & DP_TEST_BIT_DEPTH_MASK) >> DP_TEST_BIT_DEPTH_SHIFT;

	switch (bpc) {
	case DP_TEST_BIT_DEPTH_6:
		vparams->bpc = COLOR_DEPTH_6;
		HISI_DRM_INFO("[DP] TEST_BIT_DEPTH_6\n");
		break;
	case DP_TEST_BIT_DEPTH_8:
		vparams->bpc = COLOR_DEPTH_8;
		HISI_DRM_INFO("[DP] TEST_BIT_DEPTH_8\n");
		break;
	case DP_TEST_BIT_DEPTH_10:
		vparams->bpc = COLOR_DEPTH_10;
		HISI_DRM_INFO("[DP] TEST_BIT_DEPTH_10\n");
		break;
	case DP_TEST_BIT_DEPTH_12:
		vparams->bpc = COLOR_DEPTH_12;
		HISI_DRM_INFO("[DP] TEST_BIT_DEPTH_12\n");
		break;
	case DP_TEST_BIT_DEPTH_16:
		vparams->bpc = COLOR_DEPTH_16;
		HISI_DRM_INFO("[DP] TEST_BIT_DEPTH_16\n");
		break;
	default:
		HISI_DRM_INFO("[DP] Invalid TEST_BIT_DEPTH\n");
		return -EINVAL;
	}
	return 0;
}

static int handle_get_pattern_mode(u8 pattern, struct video_params *vparams)
{

	switch (pattern) {
	case DP_TEST_PATTERN_NONE:
		HISI_DRM_INFO("[DP] TEST_PATTERN_NONE %d\n", pattern);
		break;
	case DP_TEST_PATTERN_COLOR_RAMPS:
		HISI_DRM_INFO("[DP] TEST_PATTERN_COLOR_RAMPS %d\n", pattern);
		vparams->pattern_mode = RAMP;
		HISI_DRM_INFO("[DP] Change video pattern to RAMP\n");
		break;
	case DP_TEST_PATTERN_BW_VERITCAL_LINES:
		HISI_DRM_INFO("[DP] TEST_PATTERN_BW_VERTICAL_LINES %d\n", pattern);
		break;
	case DP_TEST_PATTERN_COLOR_SQUARE:
		HISI_DRM_INFO("[DP] TEST_PATTERN_COLOR_SQUARE %d\n", pattern);
		vparams->pattern_mode = COLRAMP;
		HISI_DRM_INFO("[DP] Change video pattern to COLRAMP\n");
		break;
	default:
		HISI_DRM_INFO("[DP] Invalid TEST_PATTERN %d\n", pattern);
		return -EINVAL;
	}
	return 0;
}

static int handle_test_link_video_pattern(struct dp_ctrl *dptx, int stream)
{
	int retval;
	u8 misc;
	u8 pattern;
	struct video_params *vparams = NULL;
	struct dtd *mdtd = NULL;

	vparams = &dptx->vparams;
	mdtd = &vparams->mdtd;

	retval = dptx_read_dpcd(dptx, DP_TEST_PATTERN, &pattern);
	if (retval)
		return retval;

	retval = dptx_read_dpcd(dptx, DP_TEST_MISC, &misc);
	if (retval)
		return retval;

	retval = handle_get_dynamic_range(misc, vparams);
	if (retval)
		return retval;

	retval = handle_get_colorimetry(misc, vparams);
	if (retval)
		return retval;

	retval = handle_get_pix_enc(misc, vparams);
	if (retval)
		return retval;

	retval = handle_get_bpc(misc, vparams);
	if (retval)
		return retval;

	if (dptx->dptx_video_ts_calculate)
		retval = dptx->dptx_video_ts_calculate(dptx, vparams, mdtd->pixel_clock);
	if (retval)
		return retval;

	if (dptx->dptx_video_bpc_change)
		dptx->dptx_video_bpc_change(dptx, stream);
	HISI_DRM_INFO("[DP] Change bits per component to %d\n", vparams->bpc);

	if (dptx->dptx_video_ts_change)
		dptx->dptx_video_ts_change(dptx, stream);

	retval = handle_get_pattern_mode(pattern, vparams);
	if (retval)
		return retval;

	retval = handle_test_link_video_timming(dptx, stream);
	if (retval)
		return retval;

	return 0;
}

static int handle_automated_test_request(struct dp_ctrl *dptx)
{
	int retval;
	u8 test;

	retval = dptx_read_dpcd(dptx, DP_TEST_REQUEST, &test);
	if (retval)
		return retval;

	if (test & DP_TEST_LINK_TRAINING) {
		HISI_DRM_INFO("[DP]: DP_TEST_LINK_TRAINING\n");

		retval = dptx_write_dpcd(dptx, DP_TEST_RESPONSE, DP_TEST_ACK);
		if (retval)
			return retval;

		retval = handle_test_link_training(dptx);
		if (retval)
			return retval;
	}

	if (test & DP_TEST_LINK_VIDEO_PATTERN) {
		HISI_DRM_INFO("[DP]:DP_TEST_LINK_VIDEO_PATTERN\n");

		retval = dptx_write_dpcd(dptx, DP_TEST_RESPONSE, DP_TEST_ACK);
		if (retval)
			return retval;

		dp_send_cable_notification(dptx, HOT_PLUG_TEST_OUT);

		retval = handle_test_link_video_pattern(dptx, 0);
		if (retval)
			return retval;
	}

	if (test & DP_TEST_LINK_AUDIO_PATTERN) {
		HISI_DRM_INFO("[DP]:DP_TEST_LINK_AUDIO_PATTERN\n");

		retval = dptx_write_dpcd(dptx, DP_TEST_RESPONSE, DP_TEST_ACK);
		if (retval)
			return retval;

		retval = handle_test_link_audio_pattern(dptx);
		if (retval)
			return retval;
	}

	if (test & DP_TEST_LINK_EDID_READ) {
		/* Invalid, this should happen on HOTPLUG */
		HISI_DRM_INFO("[DP]:DP_TEST_LINK_EDID_READ\n");
		return -ENOTSUPP;
	}

	if (test & DP_TEST_LINK_PHY_TEST_PATTERN) {
		HISI_DRM_INFO("[DP]:DP_TEST_LINK_PHY_TEST_PATTERN\n");

		if (dptx->dptx_triger_media_transfer)
			dptx->dptx_triger_media_transfer(dptx, false);

		retval = dptx_write_dpcd(dptx, DP_TEST_RESPONSE, DP_TEST_ACK);
		if (retval)
			return retval;

		if (dptx->dptx_phy_set_ssc)
			dptx->dptx_phy_set_ssc(dptx, true);

		dptx_link_adjust_drive_settings(dptx, NULL);  // Test only

		if (dptx->handle_test_link_phy_pattern) {
			retval = dptx->handle_test_link_phy_pattern(dptx);
			if (retval)
				return retval;
		}
	}

	return 0;
}

int handle_sink_request(struct dp_ctrl *dptx)
{
	int retval;
	u8 vector;
	u8 bytes[1] = {0};

	drm_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	retval = dptx_link_check_status(dptx);
	if (retval)
		return retval;

	retval = dptx_read_bytes_from_dpcd(dptx, DP_DEVICE_SERVICE_IRQ_VECTOR_ESI0, bytes, sizeof(bytes));
	if (retval)
		return retval;

	retval = dptx_read_dpcd(dptx, DP_DEVICE_SERVICE_IRQ_VECTOR, &vector);
	if (retval)
		return retval;

	HISI_DRM_INFO("[DP]: IRQ_VECTOR: 0x%02x\n", vector);
	dp_imonitor_set_param(DP_PARAM_IRQ_VECTOR, &vector);

	/* handle sink interrupts */
	if (!vector)
		return 0;

	if ((vector | bytes[0]) & DP_REMOTE_CONTROL_COMMAND_PENDING)
		HISI_DRM_WARN(
			"[DP]: DP_REMOTE_CONTROL_COMMAND_PENDING: Not yet implemented");

	if ((vector | bytes[0]) & DP_AUTOMATED_TEST_REQUEST) {
		HISI_DRM_INFO("[DP]: DP_AUTOMATED_TEST_REQUEST");
		retval = handle_automated_test_request(dptx);
		if (retval) {
			HISI_DRM_ERR("[DP] Automated test request failed\n");
			if (retval == -ENOTSUPP) {
				retval = dptx_write_dpcd(dptx, DP_TEST_RESPONSE, DP_TEST_NAK);
				if (retval)
					return retval;
			}
		}
	}

	if ((vector | bytes[0]) & DP_CP_IRQ) {
		HISI_DRM_WARN("[DP] DP_CP_IRQ");
		if (retval)
			return retval;
		/* Check Re-authentication Request and Link integrity
		 * Failure bits in Bstatus
		 */
	}

	if ((vector | bytes[0]) & DP_MCCS_IRQ)  {
		HISI_DRM_WARN("[DP]: DP_MCCS_IRQ: Not yet implemented");
		retval = -ENOTSUPP;
	}

	if ((vector | bytes[0]) & DP_SINK_SPECIFIC_IRQ) {
		HISI_DRM_WARN("[DP]: DP_SINK_SPECIFIC_IRQ: Not yet implemented");
		retval = -ENOTSUPP;
	}

	return retval;
}

int handle_hotunplug(struct hisi_dp_device *dp_dev)
{
	struct dp_ctrl *dptx = NULL;

	drm_check_and_return((dp_dev == NULL), -EINVAL, ERR, "[DP] hisifd is NULL!\n");

	HISI_DRM_INFO("[DP] +.\n");

	dptx = &dp_dev->dp;

	drm_check_and_return(!dptx->video_transfer_enable, 0, ERR, "already unplug");

	dptx->video_transfer_enable = false;
	dptx->max_edid_timing_hactive = 0;
	dptx->dummy_dtds_present = false;

	dptx_cancel_detect_work(dptx);
	if (dptx->dptx_link_close_stream)
		dptx->dptx_link_close_stream(dptx);

	release_edid_info(dptx);
	atomic_set(&dptx->sink_request, 0);
	atomic_set(&dptx->aux.event, 0);
	dptx->link.trained = false;
	dptx->dsc = false;
	dptx->fec = false;

	dp_imonitor_set_param(DP_PARAM_TIME_STOP, NULL);
	HISI_DRM_INFO("[DP] -.\n");
	return 0;
}

static int dptx_read_edid_block(void *data, u8 *edid,
		unsigned int block, size_t length)
{
	struct dp_ctrl *dptx = data;
	int retval;
	int retry = 0;

	u8 offset = block * length;
	u8 segment = block >> 1;

	drm_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	HISI_DRM_INFO("[DP] block=%d.\n", block);
	if (segment != 0) {
		retval = dptx_write_bytes_to_i2c(dptx, 0x30, &segment, 1);
		if (retval) {
			HISI_DRM_ERR("[DP] failed to  dptx_write_bytes_to_i2c 1!\n");
			return retval;
		}
	}
again:
	retval = dptx_write_bytes_to_i2c(dptx, DDC_ADDR, &offset, 1);
	if (retval) {
		HISI_DRM_ERR("[DP] failed to write bytes to i2c 2!\n");
		return retval;
	}

	retval = dptx_read_bytes_from_i2c(dptx, DDC_ADDR, edid, length);

	if ((retval == -EINVAL) && !retry) {
		HISI_DRM_ERR("[DP] failed to  dptx_read_bytes_from_i2c 2!\n");
		retry = 1;
		goto again;
	}
	dp_imonitor_set_param(DP_PARAM_EDID + block, edid);
	dptx_i2c_address_only(dptx, DDC_ADDR);

	return 0;
}

static int dptx_read_edid(struct dp_ctrl *dptx)
{
	int edid_try_count = 0;
	u8 *edid = NULL;

	kfree(dptx->edid);
	dptx->edid = NULL;

edid_test_retry:
	edid = (u8*)drm_do_get_edid(&dptx->dp_dev->connector,
			       dptx_read_edid_block, dptx);
	dptx->edid = edid;
	if (!edid) {
		if (edid_try_count <= dptx->edid_try_count) {
			HISI_DRM_INFO("[DP] Read edid block failed, try %d",
				      edid_try_count);
			mdelay(dptx->edid_try_delay);
			edid_try_count += 1;
			goto edid_test_retry;
		} else {
			HISI_DRM_ERR("[DP] failed to dptx_read_edid_block!\n");
		}
	} else {
		HISI_DRM_INFO("[DP] Get EDID info, cnt=%u", edid[0x7e]);
		return  ((edid[0x7e] + 1) * EDID_LENGTH);
	}
	return 0;
}

void drmmode_to_dtd(struct dtd *mdtd, struct drm_display_mode *newmode)
{
	drm_check_and_void_return((mdtd == NULL), "[DP] mdtd is NULL!\n");
	drm_check_and_void_return((newmode == NULL), "[DP] newmode is NULL!\n");

	mdtd->pixel_clock = newmode->clock;
	mdtd->h_active = newmode->hdisplay;
	mdtd->v_active = newmode->vdisplay;
	mdtd->h_sync_offset = newmode->hsync_start - newmode->hdisplay;
	mdtd->h_sync_pulse_width = newmode->hsync_end - newmode->hsync_start;
	mdtd->h_blanking = newmode->htotal - newmode->hsync_end +
		mdtd->h_sync_offset + mdtd->h_sync_pulse_width;
	mdtd->v_sync_offset = newmode->vsync_start - newmode->vdisplay;
	mdtd->v_sync_pulse_width = newmode->vsync_end - newmode->vsync_start;
	mdtd->v_blanking = newmode->vtotal - newmode->vsync_end +
		mdtd->v_sync_offset + mdtd->v_sync_pulse_width;

	if (newmode->flags & DRM_MODE_FLAG_INTERLACE) {
		mdtd->interlaced = 1;
		mdtd->v_active *= 2;
	} else {
		mdtd->interlaced = 0;
	}

	mdtd->h_sync_polarity =
		(newmode->flags & DRM_MODE_FLAG_PHSYNC) ? 1 : 0;

	mdtd->v_sync_polarity =
		(newmode->flags & DRM_MODE_FLAG_PVSYNC) ? 1 : 0;

	HISI_DRM_DEBUG("[DP] clock %dkHz, h_active=%d, v_active=%d, " \
		"h_sync_offset=%d, h_sync_pulse_width=%d, h_blanking=%d, " \
		"v_sync_offset=%d, v_sync_pulse_width=%d, v_blanking=%d, " \
		"interlaced=%d, h_sync_polarity=%d, v_sync_polarity=%d",
		       mdtd->pixel_clock,
		       mdtd->h_active,
		       mdtd->v_active,
		       mdtd->h_sync_offset,
		       mdtd->h_sync_pulse_width,
		       mdtd->h_blanking,
		       mdtd->v_sync_offset,
		       mdtd->v_sync_pulse_width,
		       mdtd->v_blanking,
		       mdtd->interlaced,
		       mdtd->h_sync_polarity,
		       mdtd->v_sync_polarity);
}

static void dptx_set_edid_timing(struct dp_ctrl *dptx,
	struct timing_info *per_timing_info)
{
	struct dtd *mdtd = NULL;

	mdtd = &dptx->vparams.mdtd;

	mdtd->pixel_repetition_input = 0;
	mdtd->pixel_clock = per_timing_info->pixel_clock;

	mdtd->h_active = per_timing_info->h_active_pixels;
	mdtd->h_blanking = per_timing_info->h_blanking;
	mdtd->h_sync_offset = per_timing_info->h_sync_offset;
	mdtd->h_sync_pulse_width = per_timing_info->h_sync_pulse_width;

	mdtd->h_image_size = per_timing_info->h_size;

	mdtd->v_active = per_timing_info->v_active_pixels;
	mdtd->v_blanking = per_timing_info->v_blanking;
	mdtd->v_sync_offset = per_timing_info->v_sync_offset;
	mdtd->v_sync_pulse_width = per_timing_info->v_sync_pulse_width;

	mdtd->v_image_size = per_timing_info->v_size;

	mdtd->interlaced = per_timing_info->interlaced;

	mdtd->v_sync_polarity = per_timing_info->v_sync_polarity;
	mdtd->h_sync_polarity = per_timing_info->h_sync_polarity;

	mdtd->pixel_clock *= 10;
	if (mdtd->interlaced == 1)
		mdtd->v_active /= 2;

	dptx->max_edid_timing_hactive = per_timing_info->h_active_pixels;

	HISI_DRM_INFO("[DP] The choosed DTD: pixel_clock is %llu, interlaced is %d, h_active is %d, v_active is %d\n",
			mdtd->pixel_clock, mdtd->interlaced, mdtd->h_active, mdtd->v_active);
	HISI_DRM_DEBUG("[DP] DTD pixel_clock: %llu interlaced: %d\n",
		 mdtd->pixel_clock, mdtd->interlaced);
	HISI_DRM_DEBUG("[DP] h_active: %d h_blanking: %d h_sync_offset: %d\n",
		 mdtd->h_active, mdtd->h_blanking, mdtd->h_sync_offset);
	HISI_DRM_DEBUG("[DP] h_sync_pulse_width: %d h_image_size: %d h_sync_polarity: %d\n",
		 mdtd->h_sync_pulse_width, mdtd->h_image_size,
		 mdtd->h_sync_polarity);
	HISI_DRM_DEBUG("[DP] v_active: %d v_blanking: %d v_sync_offset: %d\n",
		 mdtd->v_active, mdtd->v_blanking, mdtd->v_sync_offset);
	HISI_DRM_DEBUG("[DP] v_sync_pulse_width: %d v_image_size: %d v_sync_polarity: %d\n",
		 mdtd->v_sync_pulse_width, mdtd->v_image_size,
		 mdtd->v_sync_polarity);

	dp_imonitor_set_param(DP_PARAM_MAX_WIDTH,   &(mdtd->h_active));
	dp_imonitor_set_param(DP_PARAM_MAX_HIGH,    &(mdtd->v_active));
	dp_imonitor_set_param(DP_PARAM_PIXEL_CLOCK, &(mdtd->pixel_clock));
}

static int dptx_choose_edid_timing(struct dp_ctrl *dptx, bool *bsafemode)
{
	struct timing_info *per_timing_info = NULL;
	struct timing_info *per_timing_info_vr = NULL;
	struct timing_info *dptx_timing_node = NULL;
	struct timing_info *_node_ = NULL;

	u32 default_hactive;
	uint64_t default_pixel_clock;
	u8 m_fps;

	if (dptx->edid_info.video.dptx_timing_list != NULL) {
		*bsafemode = true;
		return 0;
	}

	default_hactive = SAFE_MODE_TIMING_HACTIVE;
	default_pixel_clock = SAFE_MODE_TIMING_PIXEL_CLOCK;
	list_for_each_entry_safe(dptx_timing_node, _node_,
		dptx->edid_info.video.dptx_timing_list, list_node) {
		if ((dptx_timing_node->interlaced != 1) &&
			(dptx_timing_node->v_blanking <= VBLANKING_MAX) &&
			(dptx->dptx_video_ts_calculate && !dptx->dptx_video_ts_calculate(dptx,
				&dptx->vparams, (dptx_timing_node->pixel_clock * 10)))) {

			if (dptx_timing_node->h_sync_pulse_width < 6) {
				HISI_DRM_INFO("[DP] Double Pixel mode, h_pulse_width should be more than 6.\n");
				continue;
			}

			if ((dptx_timing_node->h_active_pixels >= default_hactive) &&
				(dptx_timing_node->pixel_clock > default_pixel_clock)) {
				default_hactive = dptx_timing_node->h_active_pixels;
				default_pixel_clock = dptx_timing_node->pixel_clock;
				per_timing_info = dptx_timing_node;
			}
			m_fps = (u32)(dptx_timing_node->pixel_clock * 10000 /
					((dptx_timing_node->h_active_pixels + dptx_timing_node->h_blanking)
					* (dptx_timing_node->v_active_pixels + dptx_timing_node->v_blanking)));

			if ((dptx->dptx_vr == true) && (m_fps == 70) &&
				(dptx_timing_node->h_active_pixels >= SAFE_MODE_TIMING_HACTIVE))
				per_timing_info_vr = dptx_timing_node;
		}
	}

	if ((per_timing_info_vr != NULL) && (dptx->dptx_vr == true))
		per_timing_info = per_timing_info_vr;

	if ((default_hactive == SAFE_MODE_TIMING_HACTIVE) || (per_timing_info == NULL)) {
		*bsafemode = true;
		return 0;
	}
	dptx_set_edid_timing(dptx, per_timing_info);
	*bsafemode = false;

	return 0;
}

void dptx_link_params_reset(struct dp_ctrl *dptx)
{
	drm_check_and_void_return((dptx == NULL), "[DP] dptx is NULL!\n");

	dptx->cr_fail = false; // harutk ---ntr
	dptx->mst = false;
	dptx->ssc_en = true;
	dptx->fec = false;
	dptx->dsc = false;
	dptx->streams = 1;

	dptx_video_params_reset(&dptx->vparams);
	dptx_audio_params_reset(&dptx->aparams);
}

static int dptx_get_device_caps(struct dp_ctrl *dptx)
{
	u8 rev = 0;
	int retval;

	retval = dptx_read_dpcd(dptx, DP_DPCD_REV, &rev);
	if (retval) {
		/*
		 * Abort bringup
		 * Reset core and try again
		 * Abort all aux, and other work, reset the core
		 */
		HISI_DRM_ERR("[DP] failed to dptx_read_dpcd DP_DPCD_REV, retval=%d.\n", retval);
		return retval;
	}
	HISI_DRM_DEBUG("[DP] Revision %x.%x .\n", (rev & 0xf0) >> 4, rev & 0xf);

	retval = memset_s(dptx->rx_caps, DPTX_RECEIVER_CAP_SIZE, 0, DPTX_RECEIVER_CAP_SIZE);
	drm_check_and_return(retval != EOK, -EINVAL, ERR, "[DP] memset for rx_caps failed!\n");
	retval = dptx_read_bytes_from_dpcd(dptx, DP_DPCD_REV,
		dptx->rx_caps, DPTX_RECEIVER_CAP_SIZE);
	if (retval) {
		HISI_DRM_ERR("[DP] failed to DP_DPCD_REV, retval=%d.\n", retval);
		return retval;
	}
	dp_imonitor_set_param(DP_PARAM_DPCD_RX_CAPS, dptx->rx_caps);

	if (dptx->rx_caps[DP_TRAINING_AUX_RD_INTERVAL] &
		DP_EXTENDED_RECEIVER_CAPABILITY_FIELD_PRESENT) {
		retval = dptx_read_bytes_from_dpcd(dptx, DP_DPCD_REV_EXT,
			dptx->rx_caps, DPTX_RECEIVER_CAP_SIZE);
		if (retval) {
			HISI_DRM_ERR("[DP] DP_DPCD_REV_EXT failed, retval=%d.\n", retval);
			return retval;
		}
	}

	return 0;
}

static int dptx_link_get_device_caps(struct dp_ctrl *dptx)
{
	int retval;

	/* set sink device power state to D0 */
	retval = dptx_write_dpcd(dptx, DP_SET_POWER, DP_SET_POWER_D0);
	if (retval) {
		HISI_DRM_ERR("[DP] failed to dptx_write_dpcd DP_SET_POWER, DP_SET_POWER_D0 %d", retval);
		return retval;
	}
	mdelay(1);

	/* get rx_caps */
	retval = dptx_get_device_caps(dptx);
	if (retval) {
		HISI_DRM_ERR("[DP] Check device capability failed.\n");
		return retval;
	}

	/* get dsc capability, info in rx_caps */
	retval = dptx_dsc_initial(dptx);
	if (retval) {
		HISI_DRM_ERR("[DP] Failed to get dsc info %d", retval);
		return retval;
	}
	return 0;
}

static int dptx_get_test_request(struct dp_ctrl *dptx)
{
	int retval;
	u8 blocks = 0;
	u8 test = 0;
	u8 vector = 0;
	u8 checksum = 0;

	retval = dptx_read_dpcd(dptx, DP_DEVICE_SERVICE_IRQ_VECTOR, &vector);
	if (retval) {
		HISI_DRM_ERR("[DP] failed to  dptx_read_dpcd DP_DEVICE_SERVICE_IRQ_VECTOR, retval=%d", retval);
		return retval;
	}

	if (vector & DP_AUTOMATED_TEST_REQUEST) {
		HISI_DRM_INFO("[DP] DP_AUTOMATED_TEST_REQUEST");
		retval = dptx_read_dpcd(dptx, DP_TEST_REQUEST, &test);
		if (retval) {
			HISI_DRM_ERR("[DP] failed to dptx_read_dpcd DP_TEST_REQUEST, retval=%d.\n", retval);
			return retval;
		}

		if (test & DP_TEST_LINK_EDID_READ) {
			blocks = dptx->edid[126];
			checksum = dptx->edid[127 + EDID_BLOCK_LENGTH * blocks];

			retval = dptx_write_dpcd(dptx, DP_TEST_EDID_CHECKSUM, checksum);
			if (retval) {
				HISI_DRM_ERR("[DP] DP_TEST_EDID_CHECKSUM failed, retval=%d.\n", retval);
				return retval;
			}

			retval = dptx_write_dpcd(dptx, DP_TEST_RESPONSE, DP_TEST_EDID_CHECKSUM_WRITE);
			if (retval) {
				HISI_DRM_ERR("[DP] failed to dptx_write_dpcd DP_TEST_RESPONSE, retval=%d.\n", retval);
				return retval;
			}
		}
	}

	return 0;
}

static int dptx_video_config(struct dp_ctrl *dptx, int stream)
{
	struct video_params *vparams = NULL;
	struct dtd *mdtd = NULL;

	vparams = &dptx->vparams;
	mdtd = &vparams->mdtd;

	if (!convert_code_to_dtd(mdtd, vparams->mode,
			   vparams->refresh_rate, vparams->video_format))
		return -EINVAL;

	if (dptx->dptx_video_core_config)
		dptx->dptx_video_core_config(dptx, stream);
	return 0;
}

/**
 * default link params and controller reset
 * reset mst/fec/dec control params here
 */
static int dptx_link_reset(struct dp_ctrl *dptx)
{
	int retval;

	dptx_link_params_reset(dptx);

	retval = dptx_video_config(dptx, 0);
	if (retval) {
		HISI_DRM_ERR("[DP] video config fail\n");
		return retval;
	}

	if (dptx->dptx_link_core_reset)
		dptx->dptx_link_core_reset(dptx);

	return 0;
}


static int dptx_link_get_device_edid(struct dp_ctrl *dptx, bool *bsafemode, u32 *edid_info_size)
{
	int retval;
	char *monitor_name_info = NULL;

	retval = dptx_read_edid(dptx);
	if (retval < EDID_BLOCK_LENGTH) {
		HISI_DRM_ERR("[DP] failed to  dptx_read_edid, retval=%d", retval);
		dp_imonitor_set_param(DP_PARAM_READ_EDID_FAILED, &retval);
		*edid_info_size = 0;
		*bsafemode = true;
	} else {
		*edid_info_size = retval;
	}

	retval = parse_edid(dptx, *edid_info_size);
	if (retval) {
		HISI_DRM_ERR("[DP] EDID Parser fail, display safe mode\n");
		*bsafemode = true;
	}

	if (dptx->edid_info.video.dp_monitor_descriptor != NULL) {
		monitor_name_info = dptx->edid_info.video.dp_monitor_descriptor;
		if (!(strncmp("HUAWEIAV02", monitor_name_info, strlen("HUAWEIAV02"))) ||
			!(strncmp("HUAWEIAV03", monitor_name_info, strlen("HUAWEIAV03")))) {
			dptx->dptx_vr = true;
			HISI_DRM_INFO("[DP] The display is VR.\n");
		}
	}

	/*
	 * The TEST_EDID_READ is asserted on HOTPLUG. Check for it and
	 * handle it here.
	 */
	retval = dptx_get_test_request(dptx);
	if (retval) {
		HISI_DRM_ERR("[DP] Check test request failed.\n");
		return retval;
	}

	return 0;
}

static int dptx_link_choose_timing(struct dp_ctrl *dptx, bool bsafe_mode, int edid_info_size)
{
	struct video_params *vparams = NULL;
	struct dtd mdtd;
	int retval;
	u8 rev = 0;
	int i;
	u8 code;

	vparams = &dptx->vparams;

	if (!bsafe_mode)
		dptx_choose_edid_timing(dptx, &bsafe_mode);

	HISI_DRM_INFO("[DP] edid_info_size %d.\n", edid_info_size);
	if (bsafe_mode) {
		dp_imonitor_set_param(DP_PARAM_SAFE_MODE, &bsafe_mode);
		if (edid_info_size) {
			code = 1; // resolution: 640*480
			vparams->video_format = VCEA;
			dp_imonitor_set_param_resolution(&code, &(vparams->video_format));
			HISI_DRM_INFO("[DP] code %u, vparams->video_format %u.\n", code, vparams->video_format);
			/* If edid is parsed error, DP transfer 640*480 firstly! */
			convert_code_to_dtd(&mdtd, code, vparams->refresh_rate, vparams->video_format);
		} else {
			vparams->video_format = VCEA;
			HISI_DRM_INFO("[DP] code %u, vparams->video_format %u.\n", code, vparams->video_format);
			/* If edid can't be got, DP transfer 1024*768 firstly! */
			convert_code_to_dtd(&mdtd, 16, vparams->refresh_rate, vparams->video_format);

			retval = dptx_read_dpcd(dptx, DP_DOWNSTREAMPORT_PRESENT, &rev);
			if (retval) {
				HISI_DRM_ERR("[DP] failed to dp read DP_DOWNSTREAMPORT_PRESENT, retval=%d.\n", retval);
				return retval;
			}

			if (((rev & DP_DWN_STRM_PORT_TYPE_MASK) >> 1) != 0x01) {
				dptx->edid_info.audio.basic_audio = 0x1;
				HISI_DRM_INFO("[DP] If DFP port don't belong to analog(VGA/DVI-I)");
				HISI_DRM_INFO("[DP] update audio capabilty\n");
				dp_imonitor_set_param(DP_PARAM_BASIC_AUDIO, &(dptx->edid_info.audio.basic_audio));
			}
		}
		retval = memcpy_s(&(dptx->vparams.mdtd), sizeof(mdtd), &mdtd, sizeof(mdtd));
		drm_check_and_return(retval != EOK, -EINVAL, ERR, "[DP] memcpy for mdtd failed!");
	}

	if (dptx->dptx_video_ts_calculate)
		retval = dptx->dptx_video_ts_calculate(dptx, vparams, vparams->mdtd.pixel_clock);

	if (dptx->mst) {
		if ((vparams->mdtd.h_active > FHD_TIMING_H_ACTIVE) || (vparams->mdtd.v_active > FHD_TIMING_V_ACTIVE)) {
			vparams->video_format = VCEA;
			for (i = 0; i < dptx->streams; i++) {
				retval = dptx_video_mode_change(dptx, 16);
				if (retval)
					return retval;
			}
		}
	} else {
		if (retval) {
			HISI_DRM_INFO("[DP] Can't change to the preferred video mode: frequency = %llu\n",
				vparams->mdtd.pixel_clock);
		} else {
			HISI_DRM_DEBUG("[DP] pixel_frequency=%llu.\n", vparams->mdtd.pixel_clock);
		}
	}

	return 0;
}

int handle_hotplug(struct hisi_dp_device *dp_dev)
{
	int retval;
	u32 edid_info_size = 0;
	struct video_params *vparams = NULL;
	struct dp_ctrl *dptx = NULL;
	bool bsafe_mode = false;

	HISI_DRM_INFO("[DP] +.\n");

	drm_check_and_return((dp_dev == NULL), -EINVAL, ERR, "[DP] hisifd is NULL!\n");

	dptx = &(dp_dev->dp);
	drm_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	drm_check_and_return((!dptx->dptx_enable), -EINVAL, ERR, "[DP] dptx has already off!\n");

	if (dptx->video_transfer_enable) {
		HISI_DRM_INFO("video_transfer is already enable.\n");
		return 0;
	}

	dp_imonitor_set_param(DP_PARAM_TIME_START, NULL);

	vparams = &dptx->vparams;

	/*
	 * default link params and controller reset
	 * reset mst/fec/dec control params here
	 */
	retval = dptx_link_reset(dptx);
	if (retval)
		return retval;

	/* get rx_caps */
	retval = dptx_link_get_device_caps(dptx);
	if (retval)
		return retval;

	/* get edid */
	retval = dptx_link_get_device_edid(dptx, &bsafe_mode, &edid_info_size);
	if (retval)
		return retval;

	/* No other IRQ should be set on hotplug */
	retval = dptx_link_training(dptx, dptx->max_rate, dptx->max_lanes);
	if (retval) {
		HISI_DRM_ERR("[DP] failed to  dptx_link_training, retval=%d.\n", retval);
		dp_imonitor_set_param(DP_PARAM_LINK_TRAINING_FAILED, &retval);
		return retval;
	}
	usleep_range(1000, 2000);

	/* choose timing */
	retval = dptx_link_choose_timing(dptx, bsafe_mode, edid_info_size);
	if (retval)
		return retval;

	/* config dss and dp core */
	if (dptx->dptx_link_timing_config)
		retval = dptx->dptx_link_timing_config(dptx);
	if (retval)
		return retval;

	dptx->video_transfer_enable = true;
	wake_up_interruptible(&dptx->transfer_wait);

	HISI_DRM_INFO("[DP] -.\n");

	return 0;
}
/*lint -restore*/
