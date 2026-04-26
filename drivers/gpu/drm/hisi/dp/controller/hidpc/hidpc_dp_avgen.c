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
#include <drm/drmP.h>
#include <securec.h>

#include "hisi_dp_drv.h"
#include "hisi_defs.h"
#include "../dp_avgen_base.h"
#include "hidpc_dp_avgen.h"
#include "hidpc_dp_core.h"
#include "hidpc_reg.h"
#include "../dp_core_interface.h"
#include "../../link/dp_irq.h"
#include "../../link/dp_aux.h"
#include "hisi_drm_dpe_utils.h"
#include "../../../dsc/dsc_algorithm_manager.h"
#include "../../link/dp_dsc_algorithm.h"
#include "../../link/dp_edid.h"
#include "../dsc/dsc_config_base.h"


#define OFFSET_FRACTIONAL_BITS 11

// Rounding up to the nearest multiple of a number
#define round_up_to_nearest(num, mult)\
	((((num) + (mult) - 1) / (mult)) * (mult))

#define PAYLOAD_MAX_SIZE 9

struct rc_range_param {
	u32 min_qp     :5;
	u32 max_qp     :5;
	u32 offset     :6;
};

/*lint -save -e* */
void dptx_audio_sdp_en(struct dp_ctrl *dptx)
{
	u32 reg;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	reg = (u32)dptx_readl(dptx, DPTX_SDP_VERTICAL_CTRL);
	reg |= DPTX_EN_AUDIO_STREAM_SDP;
	dptx_writel(dptx, DPTX_SDP_VERTICAL_CTRL, reg);

	reg = (u32)dptx_readl(dptx, DPTX_SDP_HORIZONTAL_CTRL);
	reg |= DPTX_EN_AUDIO_STREAM_SDP;
	dptx_writel(dptx, DPTX_SDP_HORIZONTAL_CTRL, reg);
}

void dptx_audio_timestamp_sdp_en(struct dp_ctrl *dptx)
{
	u32 reg;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	reg = (u32)dptx_readl(dptx, DPTX_SDP_VERTICAL_CTRL);
	reg |= DPTX_EN_AUDIO_TIMESTAMP_SDP;
	dptx_writel(dptx, DPTX_SDP_VERTICAL_CTRL, reg);
}

/*
 * This Function is used by audio team.
 */
void dptx_audio_infoframe_sdp_send(struct dp_ctrl *dptx)
{
	u32 reg;
	u32 audio_infoframe_header = AUDIO_INFOFREAME_HEADER;
	u32 audio_infoframe_data[3] = {0x00000710, 0x0, 0x0};
	u8 orig_sample_freq;
	u8 sample_freq;
	struct audio_params *aparams = NULL;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	aparams = &dptx->aparams;
	sample_freq = aparams->iec_samp_freq;
	orig_sample_freq = aparams->iec_orig_samp_freq;

	audio_infoframe_data[0] =
		audio_freq_to_infoframe_data(orig_sample_freq, sample_freq);
	audio_infoframe_data[0] |= (aparams->num_channels - 1);
	audio_infoframe_data[0] |=
		audio_num_channels_to_infoframe_data(aparams->num_channels);
	HISI_DRM_DEBUG("[DP] audio_infoframe_data[0] before = %x\n",
		       audio_infoframe_data[0]);
	switch (aparams->data_width) {
	case 16:
		HISI_DRM_DEBUG("[DP] data_width = 16\n");
		audio_infoframe_data[0] &= ~GENMASK(9, 8);
		audio_infoframe_data[0] |= 1 << 8;
		break;
	case 20:
		HISI_DRM_DEBUG("[DP] data_width = 20\n");
		audio_infoframe_data[0] &= ~GENMASK(9, 8);
		audio_infoframe_data[0] |= 2 << 8;
		break;
	case 24:
		HISI_DRM_DEBUG("[DP] data_width = 24\n");
		audio_infoframe_data[0] &= ~GENMASK(9, 8);
		audio_infoframe_data[0] |= 3 << 8;
		break;
	default:
		HISI_DRM_DEBUG("[DP] data_width not found\n");
		break;
	}

	HISI_DRM_DEBUG("[DP] audio_infoframe_data[0] after = %x\n",
		       audio_infoframe_data[0]);

	dptx->sdp_list[0].payload[0] = audio_infoframe_header;
	dptx_writel(dptx, DPTX_SDP_BANK, audio_infoframe_header);
	dptx_writel(dptx, DPTX_SDP_BANK + 4, audio_infoframe_data[0]);
	dptx_writel(dptx, DPTX_SDP_BANK + 8, audio_infoframe_data[1]);
	dptx_writel(dptx, DPTX_SDP_BANK + 12, audio_infoframe_data[2]);
	reg = (u32)dptx_readl(dptx, DPTX_SDP_VERTICAL_CTRL);
	reg |= DPTX_EN_AUDIO_INFOFRAME_SDP;
	dptx_writel(dptx, DPTX_SDP_VERTICAL_CTRL, reg);
}

static void dptx_disable_sdp(struct dp_ctrl *dptx, u32 *payload, int payload_size)
{
	int i;
	int ret;

	drm_check_and_void_return((payload == NULL), "[DP] payload is NULL\n");

	for (i = 0; i < DPTX_SDP_NUM; i++) {
		if (!memcmp(dptx->sdp_list[i].payload, payload, payload_size))
			ret = memset_s(dptx->sdp_list[i].payload, payload_size, 0, payload_size);
			drm_check_and_void_return(ret != EOK, "[DP] memset for payload failed!\n");
	}
}

static void dptx_enable_sdp(struct dp_ctrl *dptx, struct sdp_full_data *data)
{
	u32 i;
	u32 reg;
	int reg_num;
	u32 header;
	int sdp_offset;

	drm_check_and_void_return((data == NULL), "[DP] data is NULL\n");

	reg_num = 0;
	header = cpu_to_be32(data->payload[0]);
	for (i = 0; i < DPTX_SDP_NUM; i++)
		if (dptx->sdp_list[i].payload[0] == 0) {
			dptx->sdp_list[i].payload[0] = header;
			sdp_offset = i * DPTX_SDP_SIZE;
			reg_num = 0;
			while (reg_num < DPTX_SDP_LEN) {
				dptx_writel(dptx, DPTX_SDP_BANK + sdp_offset
					    + reg_num * 4,
					    cpu_to_be32(
							data->payload[reg_num])
					    );
				reg_num++;
			}
			switch (data->blanking) {
			case 0:
				reg = dptx_readl(dptx, DPTX_SDP_VERTICAL_CTRL);
				reg |= (1 << (2 + i));
				dptx_writel(dptx, DPTX_SDP_VERTICAL_CTRL, reg);
				break;
			case 1:
				reg = dptx_readl(dptx, DPTX_SDP_HORIZONTAL_CTRL);
				reg |= (1 << (2 + i));
				dptx_writel(dptx, DPTX_SDP_HORIZONTAL_CTRL,
					    reg);
				break;
			case 2:
				reg = dptx_readl(dptx, DPTX_SDP_VERTICAL_CTRL);
				reg |= (1 << (2 + i));
				dptx_writel(dptx, DPTX_SDP_VERTICAL_CTRL, reg);
				reg = dptx_readl(dptx, DPTX_SDP_HORIZONTAL_CTRL);
				reg |= (1 << (2 + i));
				dptx_writel(dptx, DPTX_SDP_HORIZONTAL_CTRL,
					    reg);
				break;
			}
			break;
		}
}

static void dptx_fill_sdp(struct dp_ctrl *dptx, struct sdp_full_data *data)
{

	drm_check_and_void_return((data == NULL), "[DP] data is NULL\n");

	if (data->en == 1)
		dptx_enable_sdp(dptx, data);
	else
		dptx_disable_sdp(dptx, data->payload, PAYLOAD_MAX_SIZE);
}

void dptx_vsd_ycbcr420_send(struct dp_ctrl *dptx, u8 enable)
{
	struct sdp_full_data vsc_data;
	struct video_params *vparams = NULL;
	int i;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	vparams = &dptx->vparams;

	vsc_data.en = enable;
	for (i = 0; i < 9; i++) {
		if (i == 0) {
			vsc_data.payload[i] = 0x00070513;
		} else if (i == 5) {
			switch (vparams->bpc) {
			case COLOR_DEPTH_8:
				vsc_data.payload[i] = 0x30010000;
				break;
			case COLOR_DEPTH_10:
				vsc_data.payload[i] = 0x30020000;
				break;
			case COLOR_DEPTH_12:
				vsc_data.payload[i] = 0x30030000;
				break;
			case COLOR_DEPTH_16:
				vsc_data.payload[i] = 0x30040000;
				break;
			default:
				break;
			}
		} else {
			vsc_data.payload[i] = 0x0;
		}
	}
	vsc_data.blanking = 0;
	vsc_data.cont = 1;

	dptx_fill_sdp(dptx, &vsc_data);
}

static u32 dptx_en_audio_data(int ch_num)
{
	u32 data_en = 0;

	switch (ch_num) {
	case 1:
		data_en = DPTX_EN_AUDIO_CH_1;
		break;
	case 2:
		data_en = DPTX_EN_AUDIO_CH_2;
		break;
	case 3:
		data_en = DPTX_EN_AUDIO_CH_3;
		break;
	case 4:
		data_en = DPTX_EN_AUDIO_CH_4;
		break;
	case 5:
		data_en = DPTX_EN_AUDIO_CH_5;
		break;
	case 6:
		data_en = DPTX_EN_AUDIO_CH_6;
		break;
	case 7:
		data_en = DPTX_EN_AUDIO_CH_7;
		break;
	case 8:
		data_en = DPTX_EN_AUDIO_CH_8;
		break;
	default:
		break;
	}
	return data_en;
}

void dptx_en_audio_channel(struct dp_ctrl *dptx, int ch_num, int enable)
{
	u32 reg;
	u32 data_en = 0;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	reg = (u32)dptx_readl(dptx, DPTX_AUD_CONFIG1);
	reg &= ~DPTX_AUD_CONFIG1_DATA_EN_IN_MASK;
	data_en = dptx_en_audio_data(ch_num);

	if (enable)
		reg |= data_en << DPTX_AUD_CONFIG1_DATA_EN_IN_SHIFT;
	else
		reg &= ~(data_en << DPTX_AUD_CONFIG1_DATA_EN_IN_SHIFT);

	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);
}

void dptx_video_reset(struct dp_ctrl *dptx, int enable, int stream)
{
	u32 reg;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	reg = (u32)dptx_readl(dptx, DPTX_SRST_CTRL);
	if (enable)
		reg |= DPTX_SRST_VIDEO_RESET_N((u32)stream);
	else
		reg &= ~DPTX_SRST_VIDEO_RESET_N((u32)stream);
	dptx_writel(dptx, DPTX_SRST_CTRL, reg);
}

void dptx_audio_mute(struct dp_ctrl *dptx)
{
	u32 reg;
	struct audio_params *aparams = NULL;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	aparams = &dptx->aparams;
	reg = (u32)dptx_readl(dptx, DPTX_AUD_CONFIG1);

	if (aparams->mute == 1)
		reg |= DPTX_AUDIO_MUTE;
	else
		reg &= ~DPTX_AUDIO_MUTE;
	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);
}

void dptx_audio_config(struct dp_ctrl *dptx)
{
	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	dptx_audio_core_config(dptx);

	dptx_audio_sdp_en(dptx);
	dptx_audio_timestamp_sdp_en(dptx);

	if (dptx->dptx_audio_infoframe_sdp_send)
		dptx->dptx_audio_infoframe_sdp_send(dptx);
}

void dptx_audio_core_config(struct dp_ctrl *dptx)
{
	struct audio_params *aparams = NULL;
	u32 reg;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	aparams = &dptx->aparams;

	dptx_audio_inf_type_change(dptx);
	if (dptx->dptx_audio_num_ch_change)
		dptx->dptx_audio_num_ch_change(dptx);
	dptx_audio_data_width_change(dptx);

	reg = (u32)dptx_readl(dptx, DPTX_AUD_CONFIG1);
	reg &= ~DPTX_AUD_CONFIG1_ATS_VER_MASK;
	reg |= aparams->ats_ver << (unsigned int)DPTX_AUD_CONFIG1_ATS_VER_SHFIT;
	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);

	dptx_en_audio_channel(dptx, aparams->num_channels, 1);
}


void dptx_audio_inf_type_change(struct dp_ctrl *dptx)
{
	struct audio_params *aparams = NULL;
	u32 reg;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	aparams = &dptx->aparams;

	reg = (u32)dptx_readl(dptx, DPTX_AUD_CONFIG1);
	reg &= ~DPTX_AUD_CONFIG1_INF_TYPE_MASK;
	reg |= aparams->inf_type << (unsigned int)DPTX_AUD_CONFIG1_INF_TYPE_SHIFT;
	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);
}

void dptx_audio_num_ch_change(struct dp_ctrl *dptx)
{
	u32 reg;
	u32 num_ch_map;
	struct audio_params *aparams = NULL;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	aparams = &dptx->aparams;

	reg = (u32)dptx_readl(dptx, DPTX_AUD_CONFIG1);
	reg &= ~DPTX_AUD_CONFIG1_NCH_MASK;

	if (aparams->num_channels > 0 && aparams->num_channels <= 8)
		num_ch_map = aparams->num_channels - 1;
	else
		num_ch_map = DPTX_AUD_CONFIG1_NCH_DEFAULT_VALUE;

	reg |= num_ch_map << (unsigned int)DPTX_AUD_CONFIG1_NCH_SHIFT;
	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);
}

void dptx_audio_data_width_change(struct dp_ctrl *dptx)
{
	u32 reg;
	struct audio_params *aparams = NULL;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	aparams = &dptx->aparams;

	reg = (u32)dptx_readl(dptx, DPTX_AUD_CONFIG1);
	reg &= ~DPTX_AUD_CONFIG1_DATA_WIDTH_MASK;
	reg |= aparams->data_width << (unsigned int)DPTX_AUD_CONFIG1_DATA_WIDTH_SHIFT;
	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);
}

/*
 * Video Generation
 */
void dptx_video_timing_change(struct dp_ctrl *dptx, int stream)
{
	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	if (dptx->dptx_disable_default_video_stream)
		dptx->dptx_disable_default_video_stream(dptx, stream);

	if (dptx->dsc) {
		if (dptx->dptx_dsc_check_rx_cap)
			dptx->dptx_dsc_check_rx_cap(dptx);

		if (dptx->dptx_dsc_para_init)
			dptx->dptx_dsc_para_init(dptx);

		if (dptx->dptx_dsc_cfg)
			dptx->dptx_dsc_cfg(dptx);
	}

	if (dptx->dptx_video_core_config)
		dptx->dptx_video_core_config(dptx, stream);
	if (dptx->dptx_video_ts_change)
		dptx->dptx_video_ts_change(dptx, stream);
	if (dptx->dptx_enable_default_video_stream)
		dptx->dptx_enable_default_video_stream(dptx, stream);
}

int dptx_change_video_mode_tu_fail(struct dp_ctrl *dptx)
{
	struct dtd *mdtd = NULL;

	drm_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");
	mdtd = &dptx->vparams.mdtd;

	if ((mdtd->pixel_clock > 500000) && (mdtd->h_active >= 3840)) { /* 4k 60HZ */
		if (((dptx->link.lanes == 2) && (dptx->link.rate == DPTX_PHYIF_CTRL_RATE_HBR2)) ||
			((dptx->link.lanes == 4) && (dptx->link.rate == DPTX_PHYIF_CTRL_RATE_HBR)))
			return 95; /* 4k 30HZ */
	}
	if (mdtd->h_active > 1920)
		return 16; /* 1920*1080 */
	if (mdtd->h_active > 1280)
		return 4; /* 1280*720 */
	if (mdtd->h_active > 640)
		return 1; /* 640*480 */
	return 1;
}

static int dptx_video_mode_change_by_dmode(struct dp_ctrl *dptx,
		struct drm_display_mode *drm_mode)
{
	int retval;
	struct video_params *vparams = NULL;
	struct dtd mdtd = {0};

	vparams = &dptx->vparams;

	drmmode_to_dtd(&mdtd, drm_mode);

	vparams->mdtd = mdtd;
	vparams->m_fps = drm_mode->vrefresh ? drm_mode->vrefresh :
		drm_mode_vrefresh(drm_mode);
	retval = dptx_video_ts_calculate(dptx, vparams, mdtd.pixel_clock);

	HISI_DRM_INFO("[DP] The mode is changed as [%d * %d @%d], ret = %d\n",
		      drm_mode->hdisplay, drm_mode->vdisplay,
		      vparams->m_fps, retval);

	return retval;
}

int dptx_change_video_mode_by_drm(struct dp_ctrl *dptx,
		struct drm_display_mode *drm_mode)
{
	struct video_params *vparams = NULL;
	int retval;

	if (!dptx || !drm_mode) {
		HISI_DRM_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	vparams = &dptx->vparams;

	retval = dptx_video_mode_change_by_dmode(dptx, drm_mode);
	if (retval) {
		HISI_DRM_ERR("[DP] Change mode error!\n");
		return retval;
	}

	if (dptx_check_low_temperature(dptx)) {
		if ((vparams->mdtd.h_active > FHD_TIMING_H_ACTIVE) ||
		    (vparams->mdtd.v_active > FHD_TIMING_V_ACTIVE)) {
			vparams->video_format = VCEA;
			vparams->mode = 16; /* Siwtch to 1080p on PC mode */
			HISI_DRM_INFO(
			"[DP] Video mode is changed by low temperature!\n");

			retval = dptx_video_mode_change(dptx, vparams->mode);
			if (retval) {
				HISI_DRM_ERR("[DP] Change mode error!\n");
				return retval;
			}
		}
	}

	return 0;
}

static int dptx_calculate_hblank_interval(struct dp_ctrl *dptx)
{
	struct video_params *vparams = NULL;
	int pixel_clk;
	u16 h_blank;
	u32 link_clk;
	u8 rate;
	int hblank_interval;

	drm_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");

	vparams = &dptx->vparams;
	pixel_clk = vparams->mdtd.pixel_clock;
	h_blank = vparams->mdtd.h_blanking;
	rate = dptx->link.rate;

	switch (rate) {
	case DPTX_PHYIF_CTRL_RATE_RBR:
		link_clk = 40500;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR:
		link_clk = 67500;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		link_clk = 135000;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		link_clk = 202500;
		break;
	default:
		WARN(1, "Invalid rate 0x%x\n", rate);
		return -EINVAL;
	}

	if (pixel_clk == 0) {
		HISI_DRM_ERR("pixel_clk is 0\n");
		return -EINVAL;
	}

	hblank_interval = h_blank * link_clk / pixel_clk;

	return hblank_interval;
}

void dptx_mst_enable(struct dp_ctrl *dptx)
{
	u32 reg;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	reg = dptx_readl(dptx, DPTX_CCTL);
	reg |= (dptx->mst ? DPTX_CCTL_ENABLE_MST_MODE : 0);
	dptx_writel(dptx, DPTX_CCTL, reg);
}

int dptx_fec_enable(struct dp_ctrl *dptx)
{
	int retval;
	u32 reg;
	u8 result;

	drm_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");

	if (dptx->fec) {
		/* Forward Error Correction flow */
		reg = dptx_readl(dptx, DPTX_CCTL);
		reg |= DPTX_CCTL_ENH_FRAME_FEC_EN;
		dptx_writel(dptx, DPTX_CCTL, reg);

		// Set FEC_READY on the sink side
		retval = dptx_write_dpcd(dptx, DP_FEC_CONFIGURATION, DP_FEC_READY);
		if (retval)
			return retval;

		// Enable forward error correction
		reg = dptx_readl(dptx, DPTX_CCTL);
		reg |= DPTX_CCTL_ENABLE_FEC;
		dptx_writel(dptx, DPTX_CCTL, reg);

		HISI_DRM_DEBUG("Enabling Forward Error Correction\n");

		retval = dptx_read_dpcd(dptx, DP_FEC_STATUS, &result);
		if (retval)
			HISI_DRM_DEBUG("DPCD read failed\n");

		retval = dptx_read_dpcd(dptx, DP_FEC_ERROR_COUNT, &result);
		if (retval)
			HISI_DRM_DEBUG("DPCD read failed\n");
	}

	return 0;
}

void dptx_fec_disable(struct dp_ctrl *dptx)
{
	u32 reg;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	// Disable forward error correction
	reg = dptx_readl(dptx, DPTX_CCTL);
	reg &= ~DPTX_CCTL_ENABLE_FEC;
	dptx_writel(dptx, DPTX_CCTL, reg);
}

static const u8 vcea_fractional_vblank_modes[] = {
	5, 6, 7, 10, 11, 20,
	21, 22, 39, 25, 26,
	40, 44, 45, 46, 50,
	51, 54, 55, 58, 59
};

static void dptx_video_set_five_config(struct dp_ctrl *dptx, int stream)
{
	u32 reg = 0;
	u8 vmode;
	struct dtd *mdtd = NULL;
	int i;

	mdtd = &dptx->vparams.mdtd;
	vmode = dptx->vparams.mode;

	/* Configure video_config1 register */
	if (dptx->vparams.video_format == VCEA) {
		for (i = 0; i < ARRAY_SIZE(vcea_fractional_vblank_modes); i++) {
			if (vmode == vcea_fractional_vblank_modes[i]) {
				reg |= DPTX_VIDEO_CONFIG1_IN_OSC_EN;
				break;
			}
		}
	}

	if (mdtd->interlaced == 1)
		reg |= DPTX_VIDEO_CONFIG1_O_IP_EN;

	reg |= mdtd->h_active << DPTX_VIDEO_H_ACTIVE_SHIFT;
	reg |= mdtd->h_blanking << DPTX_VIDEO_H_BLANK_SHIFT;
	dptx_writel(dptx, DPTX_VIDEO_CONFIG1_N(stream), reg);

	/* Configure video_config2 register */
	reg = 0;
	reg |= mdtd->v_active << DPTX_VIDEO_V_ACTIVE_SHIFT;
	reg |= mdtd->v_blanking << DPTX_VIDEO_V_BLANK_SHIFT;
	dptx_writel(dptx, DPTX_VIDEO_CONFIG2_N(stream), reg);

	/* Configure video_config3 register */
	reg = 0;
	reg |= mdtd->h_sync_offset << DPTX_VIDEO_H_FRONT_PORCH;
	reg |= mdtd->h_sync_pulse_width << DPTX_VIDEO_H_SYNC_WIDTH;
	dptx_writel(dptx, DPTX_VIDEO_CONFIG3_N(stream), reg);

	/* Configure video_config4 register */
	reg = 0;
	reg |= mdtd->v_sync_offset << DPTX_VIDEO_V_FRONT_PORCH;
	reg |= mdtd->v_sync_pulse_width << DPTX_VIDEO_V_SYNC_WIDTH;
	dptx_writel(dptx, DPTX_VIDEO_CONFIG4_N(stream), reg);

	/* Configure video_config5 register */
	if (dptx->dptx_video_ts_change)
		dptx->dptx_video_ts_change(dptx, stream);
}


void dptx_video_core_config(struct dp_ctrl *dptx, int stream)
{
	struct dtd *mdtd = NULL;
	u32 reg;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	mdtd = &dptx->vparams.mdtd;

	dptx_video_set_core_bpc(dptx, stream);

	/* Single, dual, or quad pixel */
	reg = dptx_readl(dptx, DPTX_VSAMPLE_CTRL_N(stream));
	reg &= ~DPTX_VSAMPLE_CTRL_MULTI_PIXEL_MASK;
	reg |= dptx->multipixel << DPTX_VSAMPLE_CTRL_MULTI_PIXEL_SHIFT;
	dptx_writel(dptx, DPTX_VSAMPLE_CTRL_N(stream), reg);

	/* Configure DPTX_VSAMPLE_POLARITY_CTRL register */
	reg = 0;

	if (mdtd->h_sync_polarity == 1)
		reg |= DPTX_POL_CTRL_H_SYNC_POL_EN;
	if (mdtd->v_sync_polarity == 1)
		reg |= DPTX_POL_CTRL_V_SYNC_POL_EN;

	dptx_writel(dptx, DPTX_VSAMPLE_POLARITY_CTRL_N(stream), reg);

	/* Configure video_config1-5 register */
	dptx_video_set_five_config(dptx, stream);

	/* Configure video_msa1 register */
	reg = 0;
	reg |= (mdtd->h_blanking - mdtd->h_sync_offset)
		<< DPTX_VIDEO_MSA1_H_START_SHIFT;
	reg |= (mdtd->v_blanking - mdtd->v_sync_offset)
		<< DPTX_VIDEO_MSA1_V_START_SHIFT;
	dptx_writel(dptx, DPTX_VIDEO_MSA1_N(stream), reg);

	dptx_video_set_sink_bpc(dptx, stream);

	reg = dptx_calculate_hblank_interval(dptx);
	reg |= (DPTX_VIDEO_HBLANK_INTERVAL_ENABLE << DPTX_VIDEO_HBLANK_INTERVAL_SHIFT);
	dptx_writel(dptx, DPTX_VIDEO_HBLANK_INTERVAL, reg);

}

static u32 slot_count_get(bool fec, int lanes, u32 tu)
{
	u8 fec_slot_count = 0;
	u32 slot_count;

	if (fec)
		fec_slot_count = (lanes == 1) ? 13 : 7;

	slot_count = (tu > 0) ? (tu + 1 + fec_slot_count) : (tu + fec_slot_count);
	return slot_count;
}

static int dptx_calculate_dsc_init_threshold(struct dp_ctrl *dptx,
	struct video_params *vparams)
{
	u32 link_pixel_clock_ratio;
	u16 pixle_push_rate;
	u16 slot_count;
	u32 link_clk;
	u16 dsc_bpp;

	dsc_bpp = vparams->dp_dsc_info.dsc_info.dsc_bpp;

	slot_count = (u16)slot_count_get(dptx->fec, dptx->link.lanes, vparams->aver_bytes_per_tu);
	slot_count = round_up_to_nearest(slot_count, 4);

	switch (dptx->link.rate) {
	case DPTX_PHYIF_CTRL_RATE_RBR:
		link_clk = 40500;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR:
		link_clk = 67500;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		link_clk = 135000;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		link_clk = 202500;
		break;
	default:
		WARN(1, "Invalid rate 0x%x\n", dptx->link.rate);
		return -EINVAL;
	}

	if (dsc_bpp == 0) {
		HISI_DRM_ERR("dsc_bpp is 0\n");
		return -EINVAL;
	}

	if (vparams->mdtd.pixel_clock == 0) {
		HISI_DRM_ERR("pix_clock is 0\n");
		return -EINVAL;
	}

	pixle_push_rate = (8 / dsc_bpp) * dptx->link.lanes;
	link_pixel_clock_ratio = link_clk / vparams->mdtd.pixel_clock;

	vparams->init_threshold = (u8)(pixle_push_rate * link_pixel_clock_ratio * slot_count);

	if (vparams->init_threshold < 32) {
		vparams->init_threshold = 32;
		HISI_DRM_DEBUG("Set init_threshold for dsc to %d\n", vparams->init_threshold);
	}
	return 0;
}

/* (bpc, encoding) -> (mst_t1_val) mapping */
static const struct t1_map t1_maps[] = {
	{COLOR_DEPTH_6, RGB, 72000 / 36},
	{COLOR_DEPTH_6, YCBCR420, 72000 / 36},
	{COLOR_DEPTH_8, RGB, 16000 / 12},
	{COLOR_DEPTH_8, YCBCR420, 16000 / 12},
	{COLOR_DEPTH_8, YCBCR422, 8000 / 4},
	{COLOR_DEPTH_10, RGB, 64000 / 60},
	{COLOR_DEPTH_10, YCBCR420, 64000 / 60},
	{COLOR_DEPTH_10, YCBCR422, 32000 / 20},
	{COLOR_DEPTH_12, RGB, 32000 / 36},
	{COLOR_DEPTH_12, YCBCR420, 32000 / 36},
	{COLOR_DEPTH_12, YCBCR422, 16000 / 12},
	{COLOR_DEPTH_16, RGB, 8000 / 12},
	{COLOR_DEPTH_16, YCBCR420, 8000 / 12},
	{COLOR_DEPTH_16, YCBCR422, 4000 / 4},
};

static int dptx_calculate_mst_t1(struct video_params *vparams, int *mst_t1)
{
	int i;
	int encoding = vparams->pix_enc;

	*mst_t1 = 0;
	for (i = 0; i < ARRAY_SIZE(t1_maps); i++) {
		if (vparams->bpc == t1_maps[i].colordep && encoding == t1_maps[i].encoding) {
			*mst_t1 = t1_maps[i].mst_t1_val;
			break;
		}
	}

	if (i == ARRAY_SIZE(t1_maps)) {
		HISI_DRM_ERR("Invalid bpc %d\n", vparams->bpc);
		return -EINVAL;
	}

	return 0;
}

static int dptx_calculate_mst_slot_count(struct dp_ctrl *dptx,
	struct video_params *vparams, int link_rate)
{
	struct dtd *mdtd = NULL;
	u32 tu_mst;
	u32 tu_frac_mst;
	int numerator;
	int denominator;
	s64 fixp;
	int slot_count;

	mdtd = &vparams->mdtd;
	numerator = mdtd->pixel_clock * 3 * 10;  // synopsys set value;
	denominator = (link_rate / 10) * dptx->link.lanes * 100 * 1000;

	HISI_DRM_INFO("[DP] MST: pixel_clock=%llu, MST: numerator=%d, denominator=%d\n",
		mdtd->pixel_clock, numerator, denominator);

	fixp = drm_fixp_from_fraction(numerator * 64, denominator);
	tu_mst = drm_fixp2int(fixp);
	vparams->aver_bytes_per_tu = tu_mst;

	fixp &= DRM_FIXED_DECIMAL_MASK;
	fixp *= 64;
	tu_frac_mst = drm_fixp2int_ceil(fixp);
	vparams->aver_bytes_per_tu_frac = tu_frac_mst;

	HISI_DRM_INFO("[DP] MST: tu = %d, tu_frac = %d\n", tu_mst,
		tu_frac_mst);

	slot_count = slot_count_get(dptx->fec, dptx->link.lanes, tu_mst);
	return slot_count;
}

static int dptx_calculate_mst_t3(struct dp_ctrl *dptx,
	struct video_params *vparams,
	u16 slot_count)
{
	int lane_count;
	int num_lanes_divisor;
	int slot_count_adjust;
	int mst_t3;

	lane_count = dptx->link.lanes;

	if (lane_count == 1) {
		num_lanes_divisor = 4;
		slot_count_adjust = 3;
	} else if (lane_count == 2) {
		num_lanes_divisor = 2;
		slot_count_adjust = 1;
	} else {
		num_lanes_divisor = 1;
		slot_count_adjust = 0;
	}

	mst_t3 = ((slot_count + slot_count_adjust) / num_lanes_divisor) + 8;
	return mst_t3;
}

static int dptx_calculate_mst_init_threshold(struct dp_ctrl *dptx,
	struct video_params *vparams, int link_rate, int link_clk)
{
	int mst_t1;
	int mst_t2;
	int mst_t3;
	u16 slot_count;
	int pixel_clock;

	pixel_clock = vparams->mdtd.pixel_clock;

	dptx_calculate_mst_t1(vparams, &mst_t1);

	if (vparams->pix_enc == YCBCR420)
		pixel_clock = pixel_clock / 2;

	if (pixel_clock == 0) {
		HISI_DRM_ERR("pix_clock is 0\n");
		return -EINVAL;
	}

	mst_t2 = (link_clk * 1000 / pixel_clock);

	slot_count = dptx_calculate_mst_slot_count(dptx, vparams, link_rate);
	mst_t3 = dptx_calculate_mst_t3(dptx, vparams, slot_count);

	vparams->init_threshold = mst_t1 * mst_t2 * mst_t3 / (1000 * 1000);
	HISI_DRM_INFO("[DP] t1 = %d, t2 =%d, tu = %d, vparams->init_threshold = %d\n",
		mst_t1, mst_t2, mst_t3, vparams->init_threshold);

	return 0;
}

static int dptx_calculate_t1(struct dp_ctrl *dptx, struct video_params *vparams, int *t1)
{
	int lane_num;
	int encoding;

	lane_num = dptx->link.lanes;
	encoding = vparams->pix_enc;

	switch (vparams->bpc) {
	case COLOR_DEPTH_6:
		*t1 = (4 * 1000 / 9) * lane_num;
		break;
	case COLOR_DEPTH_8:
		if (encoding == YCBCR422) {
			*t1 = (1 * 1000 / 2) * lane_num;
		} else if (encoding == YONLY) {
			*t1 = lane_num * 1000;
		} else {
			if (dptx->multipixel == DPTX_MP_DUAL_PIXEL)
				*t1 = (1 * 1000 / 3) * lane_num;
			else
				*t1 = (3 * 1000 / 16) * lane_num;
		}
		break;
	case COLOR_DEPTH_10:
		if (encoding == YCBCR422)
			*t1 = (2 * 1000 / 5) * lane_num;
		else if (encoding == YONLY)
			*t1 = (4 * 1000 / 5) * lane_num;
		else
			*t1 = (4 * 1000 / 15) * lane_num;
		break;
	case COLOR_DEPTH_12:
		if (encoding == YCBCR422) {
			if (dptx->multipixel == DPTX_MP_DUAL_PIXEL)
				*t1 = (1 * 1000 / 6) * lane_num;
			else
				*t1 = (1 * 1000 / 3) * lane_num;
		} else if (encoding == YONLY) {
			*t1 = (2 * 1000 / 3) * lane_num;
		} else {
			*t1 = (2 * 1000 / 9) * lane_num;
		}
		break;
	case COLOR_DEPTH_16:
		if (encoding == YONLY)
			*t1 = (1 * 1000 / 2) * lane_num;

		if ((encoding != YONLY) && (encoding != YCBCR422) &&
			(dptx->multipixel == DPTX_MP_DUAL_PIXEL))
			*t1 = (1 * 1000 / 6) * lane_num;
		else
			*t1 = (1 * 1000 / 4) * lane_num;
		break;
	default:
		HISI_DRM_DEBUG("Invalid param BPC = %d\n", vparams->bpc);
		return -EINVAL;
	}
	return 0;
}

static int dptx_calculate_init_threshold(struct dp_ctrl *dptx, struct video_params *vparams,
	int link_clk, int tu)
{
	struct dtd *mdtd = NULL;
	int t1 = 0;
	int t2 = 0;
	int retval;
	int encoding;
	int pix_clock;

	mdtd = &vparams->mdtd;
	encoding = vparams->pix_enc;
	pix_clock = mdtd->pixel_clock;

	// Single Pixel Mode
	if (dptx->multipixel == DPTX_MP_SINGLE_PIXEL) {
		if (tu < 6)
			vparams->init_threshold = 32;
		else if (mdtd->h_blanking <= 40 && encoding == YCBCR420)
			vparams->init_threshold = 3;
		else if (mdtd->h_blanking <= 80  && encoding != YCBCR420)
			vparams->init_threshold = 12;
		else
			vparams->init_threshold = 16;
	} else {
	// Multiple Pixel Mode
		retval = dptx_calculate_t1(dptx, vparams, &t1);
		if (retval)
			return -EINVAL;

		if (encoding == YCBCR420)
			pix_clock = pix_clock / 2;

		if (pix_clock == 0) {
			HISI_DRM_ERR("pix_clock is 0\n");
			return -EINVAL;
		}

		t2 = (link_clk * 1000 / pix_clock);

		vparams->init_threshold = t1 * t2 * tu / (1000 * 1000);
	}
	return 0;
}

int dptx_video_ts_calculate(struct dp_ctrl *dptx, struct video_params *vparams,
	int pixel_clock)
{
	int retval = 0;
	int ts;
	int tu;
	int color_dep;
	int link_rate;
	int link_clk;

	drm_check_and_return((dptx == NULL || vparams == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");

	link_clk = dptx_br_to_link_clk(dptx->link.rate);
	link_rate = dptx_br_to_link_rate(dptx->link.rate);

	vparams->mdtd.pixel_clock = pixel_clock;

	if (dptx->link.lanes * link_rate == 0) {
		HISI_DRM_ERR("[DP] lane_num = %d, link_rate = %d", dptx->link.lanes, link_rate);
		return -EINVAL;
	}

	color_dep = dptx_get_color_depth(vparams->bpc, vparams->pix_enc);
	if (dptx->dsc)
		color_dep = dptx->vparams.dp_dsc_info.dsc_info.dsc_bpp;

	ts = (8 * color_dep * pixel_clock) / (dptx->link.lanes * link_rate);
	tu = ts / 1000;
	dp_imonitor_set_param(DP_PARAM_TU, &tu);

	if (tu >= 65) {
		HISI_DRM_INFO("[DP] tu %d > 65", tu);
		return -EINVAL;
	}

	// Calculate init_threshold for DSC mode
	if (dptx->dsc) {
		dptx_calculate_dsc_init_threshold(dptx, vparams);
		HISI_DRM_DEBUG("calculated init_threshold for dsc = %d\n", vparams->init_threshold);
	} else {
		// Calculate init_threshold for non DSC mode
		retval = dptx_calculate_init_threshold(dptx, vparams, link_clk, tu);
		if (retval) {
			HISI_DRM_ERR("[DP] Set init_threshold error\n");
			return -EINVAL;
		}
	}

	HISI_DRM_INFO("[DP] color_dep = %d, tu = %d, init_threshold = %d", color_dep, tu, vparams->init_threshold);

	vparams->aver_bytes_per_tu = (u8)tu;
	vparams->aver_bytes_per_tu_frac = (u8)(ts / 100 - tu * 10);

	if (dptx->mst)
		dptx_calculate_mst_init_threshold(dptx, vparams, link_rate, link_clk);

	return retval;
}


void dptx_video_ts_change(struct dp_ctrl *dptx, int stream)
{
	u32 reg;
	struct video_params *vparams = NULL;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	vparams = &dptx->vparams;

	reg = (u32)dptx_readl(dptx, DPTX_VIDEO_CONFIG5_N(stream));
	reg = reg & (~DPTX_VIDEO_CONFIG5_TU_MASK);
	reg = reg | (vparams->aver_bytes_per_tu <<
			DPTX_VIDEO_CONFIG5_TU_SHIFT);
	if (dptx->mst) {
		reg = reg & (~DPTX_VIDEO_CONFIG5_TU_FRAC_MASK_MST);
		reg = reg | (vparams->aver_bytes_per_tu_frac <<
			     DPTX_VIDEO_CONFIG5_TU_FRAC_SHIFT_MST);
	} else {
		reg = reg & (~DPTX_VIDEO_CONFIG5_TU_FRAC_MASK_SST);
		reg = reg | (vparams->aver_bytes_per_tu_frac <<
			     DPTX_VIDEO_CONFIG5_TU_FRAC_SHIFT_SST);
	}

	reg = reg & (~DPTX_VIDEO_CONFIG5_INIT_THRESHOLD_MASK);
	reg = reg | (vparams->init_threshold <<
			DPTX_VIDEO_CONFIG5_INIT_THRESHOLD_SHIFT);

	dptx_writel(dptx, DPTX_VIDEO_CONFIG5_N(stream), reg);
}

/* hidpc register manual: register VSAMPLE_CTRL(0x300) bit:20:16 */
static const struct video_bpc_mapping hidpc_video_mapping[] = {
	{RGB, COLOR_DEPTH_6, 0},
	{RGB, COLOR_DEPTH_8, 1},
	{RGB, COLOR_DEPTH_10, 2},
	{RGB, COLOR_DEPTH_12, 3},
	{RGB, COLOR_DEPTH_16, 4},
	{YCBCR444, COLOR_DEPTH_8, 5},
	{YCBCR444, COLOR_DEPTH_10, 6},
	{YCBCR444, COLOR_DEPTH_12, 7},
	{YCBCR444, COLOR_DEPTH_16, 8},
	{YCBCR422, COLOR_DEPTH_8, 9},
	{YCBCR422, COLOR_DEPTH_10, 10},
	{YCBCR422, COLOR_DEPTH_12, 11},
	{YCBCR422, COLOR_DEPTH_16, 12},
	{YCBCR420, COLOR_DEPTH_8, 13},
	{YCBCR420, COLOR_DEPTH_10, 14},
	{YCBCR420, COLOR_DEPTH_12, 15},
	{YCBCR420, COLOR_DEPTH_16, 16},
	{YONLY, COLOR_DEPTH_8, 17},
	{YONLY, COLOR_DEPTH_10, 18},
	{YONLY, COLOR_DEPTH_12, 19},
	{YONLY, COLOR_DEPTH_16, 20},
	{RAW, COLOR_DEPTH_8, 23},
	{RAW, COLOR_DEPTH_10, 24},
	{RAW, COLOR_DEPTH_12, 25},
	{RAW, COLOR_DEPTH_16, 27},
};

static int dptx_video_get_bpc_mapping(struct dp_ctrl *dptx, u8 *bpc_mapping)
{
	int i;
	u8 bpc;
	enum pixel_enc_type pix_enc;
	struct video_params *vparams = NULL;
	struct video_bpc_mapping vmap;

	drm_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");

	vparams = &dptx->vparams;
	bpc = vparams->bpc;
	pix_enc = vparams->pix_enc;
	*bpc_mapping = 0; /* set default value of bpc_mapping */

	for (i = 0; i < ARRAY_SIZE(hidpc_video_mapping); i++) {
		vmap = hidpc_video_mapping[i];

		if (pix_enc == vmap.pix_enc && bpc == vmap.bpc) {
			*bpc_mapping = vmap.bpc_mapping;
			break;
		}
	}

	if (i == ARRAY_SIZE(hidpc_video_mapping))
		HISI_DRM_ERR("[DP] No corresponding bpc_mapping\n");

	return 0;
}

void dptx_video_bpc_change(struct dp_ctrl *dptx, int stream)
{
	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	dptx_video_set_core_bpc(dptx, stream);
	dptx_video_set_sink_bpc(dptx, stream);
}

void dptx_video_set_core_bpc(struct dp_ctrl *dptx, int stream)
{
	u32 reg;
	u8 bpc_mapping;
	int retval;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	retval = dptx_video_get_bpc_mapping(dptx, &bpc_mapping);
	if (retval)
		return;

	reg = dptx_readl(dptx, DPTX_VSAMPLE_CTRL_N(stream));
	reg &= ~DPTX_VSAMPLE_CTRL_VMAP_BPC_MASK;

	if (dptx->dsc)
		reg |= (1 << DPTX_VG_CONFIG1_BPC_SHIFT);
	else
		reg |= (bpc_mapping << DPTX_VSAMPLE_CTRL_VMAP_BPC_SHIFT);

	dptx_writel(dptx, DPTX_VSAMPLE_CTRL_N(stream), reg);
}

void dptx_video_set_sink_col(struct dp_ctrl *dptx, int stream)
{
	u32 reg_msa2;
	u8 col_mapping;
	u8 colorimetry;
	u8 dynamic_range;
	struct video_params *vparams = NULL;
	enum pixel_enc_type pix_enc;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	vparams = &dptx->vparams;
	pix_enc = vparams->pix_enc;
	colorimetry = vparams->colorimetry;
	dynamic_range = vparams->dynamic_range;

	reg_msa2 = dptx_readl(dptx, DPTX_VIDEO_MSA2_N(stream));
	reg_msa2 &= ~DPTX_VIDEO_VMSA2_COL_MASK;

	col_mapping = 0;

	/* According to Table 2-94 of DisplayPort spec 1.3 */
	switch (pix_enc) {
	case RGB:
		if (dynamic_range == CEA)
			col_mapping = 4;
		else if (dynamic_range == VESA)
			col_mapping = 0;
		break;
	case YCBCR422:
		if (colorimetry == ITU601)
			col_mapping = 5;
		else if (colorimetry == ITU709)
			col_mapping = 13;
		break;
	case YCBCR444:
		if (colorimetry == ITU601)
			col_mapping = 6;
		else if (colorimetry == ITU709)
			col_mapping = 14;
		break;
	case RAW:
		col_mapping = 1;
		break;
	case YCBCR420:
	case YONLY:
		break;
	}

	reg_msa2 |= (col_mapping << DPTX_VIDEO_VMSA2_COL_SHIFT);
	dptx_writel(dptx, DPTX_VIDEO_MSA2_N(stream), reg_msa2);
}

void dptx_video_set_sink_bpc(struct dp_ctrl *dptx, int stream)
{
	uint32_t reg_msa2, reg_msa3;
	u8 bpc_mapping, bpc;
	struct video_params *vparams = NULL;
	enum pixel_enc_type pix_enc;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	vparams = &dptx->vparams;
	pix_enc = vparams->pix_enc;
	bpc = vparams->bpc;

	reg_msa2 = dptx_readl(dptx, DPTX_VIDEO_MSA2_N(stream));
	reg_msa3 = dptx_readl(dptx, DPTX_VIDEO_MSA3_N(stream));

	reg_msa2 &= ~DPTX_VIDEO_VMSA2_BPC_MASK;
	reg_msa3 &= ~DPTX_VIDEO_VMSA3_PIX_ENC_MASK;

	bpc_mapping = dptx_get_sink_bpc_mapping(pix_enc, bpc);

	switch (pix_enc) {
	case RGB:
	case YCBCR444:
	case YCBCR422:
		break;
	case YCBCR420:
		reg_msa3 |= 1 << DPTX_VIDEO_VMSA3_PIX_ENC_YCBCR420_SHIFT;
		break;
	case YONLY:
		/* According to Table 2-94 of DisplayPort spec 1.3 */
		reg_msa3 |= 1 << DPTX_VIDEO_VMSA3_PIX_ENC_SHIFT;
		break;
	case RAW:
		 /* According to Table 2-94 of DisplayPort spec 1.3 */
		reg_msa3 |= (1 << DPTX_VIDEO_VMSA3_PIX_ENC_SHIFT);
		break;
	default:
		break;
	}

	reg_msa2 |= (bpc_mapping << DPTX_VIDEO_VMSA2_BPC_SHIFT);

	dptx_writel(dptx, DPTX_VIDEO_MSA2_N(stream), reg_msa2);
	dptx_writel(dptx, DPTX_VIDEO_MSA3_N(stream), reg_msa3);
	dptx_video_set_sink_col(dptx, stream);
}


void dptx_disable_default_video_stream(struct dp_ctrl *dptx, int stream)
{
	u32 vsamplectrl;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	vsamplectrl = dptx_readl(dptx, DPTX_VSAMPLE_CTRL_N(stream));
	vsamplectrl &= ~DPTX_VSAMPLE_CTRL_STREAM_EN;
	dptx_writel(dptx, DPTX_VSAMPLE_CTRL_N(stream), vsamplectrl);

	if ((dptx->dptx_vr) && (dptx->dptx_detect_inited)) {
		HISI_DRM_INFO("[DP] Cancel dptx detect err count.\n");
		hrtimer_cancel(&dptx->dptx_hrtimer);
	}
}

void dptx_enable_default_video_stream(struct dp_ctrl *dptx, int stream)
{
	u32 vsamplectrl;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	vsamplectrl = dptx_readl(dptx, DPTX_VSAMPLE_CTRL_N(stream));
	vsamplectrl |= DPTX_VSAMPLE_CTRL_STREAM_EN;
	dptx_writel(dptx, DPTX_VSAMPLE_CTRL_N(stream), vsamplectrl);

	if ((dptx->dptx_vr) && (dptx->dptx_detect_inited)) {
		HISI_DRM_INFO("[DP] restart dptx detect err count when enable video stream.\n");
		hrtimer_restart(&dptx->dptx_hrtimer);
	}
}

void dptx_timing_config(struct dp_ctrl *dptx)
{
	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	/* video data can' be sent before DPTX timing configure. */
	if (dptx->dptx_triger_media_transfer)
		dptx->dptx_triger_media_transfer(dptx, false);

	dptx_video_reset(dptx, 1, 0);
	mdelay(100);
	dptx_video_reset(dptx, 0, 0);

	/* Update DP reg configue */
	if (dptx->dptx_video_timing_change) {
		dptx_video_timing_change(dptx, 0); /* dptx video reg depends on dss pixel clock. */

		if (dptx->mst)
			dptx_video_timing_change(dptx, 1); /* dptx video reg depends on dss pixel clock. */
	}

	if (dptx->dsc)
		dptx_dsc_enable(dptx);

	if (dptx->dptx_audio_config)
		dptx->dptx_audio_config(dptx); /* dptx audio reg depends on phy status(P0) */

	if (dptx->dptx_triger_media_transfer)
		dptx->dptx_triger_media_transfer(dptx, true);
}

int dptx_link_timing_config(struct dp_ctrl *dptx)
{
	int retval;
	struct video_params *vparams = NULL;

	drm_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL\n");

	/* enable fec */
	retval = dptx_fec_enable(dptx);
	if (retval) {
		HISI_DRM_ERR("[DP] fec enable failed!\n");
		return retval;
	}

	// Enable audio SDP
	dptx_audio_sdp_en(dptx);
	dptx_audio_timestamp_sdp_en(dptx);

	// enable VCS if YCBCR420 is enabled
	vparams = &dptx->vparams;
	if (vparams->pix_enc == YCBCR420)
		dptx_vsd_ycbcr420_send(dptx, 1);

	return 0;
}

int dptx_triger_media_transfer(struct dp_ctrl *dptx, bool benable)
{
	struct audio_params *aparams = NULL;
	struct video_params *vparams = NULL;

	drm_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");

	aparams = &(dptx->aparams);
	vparams = &(dptx->vparams);

	if (dptx->video_transfer_enable) {
		if (benable) {
			enable_dp_ldi(dptx->dss_base);
			dptx_en_audio_channel(dptx, aparams->num_channels, 1);
		} else {
			dptx_en_audio_channel(dptx, aparams->num_channels, 0);
			disable_dp_ldi(dptx->dss_base);
			if (vparams->m_fps > 0) {
				mdelay(1000 / vparams->m_fps + 10);
			} else {
				HISI_DRM_INFO("vparams->m_fps = %d", vparams->m_fps);
				mdelay(50);
				return -1;
			}
		}
	}
	return 0;
}

int dptx_resolution_switch(struct hisi_dp_device *dp_dev, enum dptx_hot_plug_type etype)
{
	struct dtd *mdtd = NULL;
	struct dp_ctrl *dptx = NULL;
	struct video_params *vparams = NULL;

	HISI_DRM_INFO("[DP] +");
	drm_check_and_return((dp_dev == NULL), -EINVAL, ERR, "[DP] dp_dev is NULL\n");

	dptx = &(dp_dev->dp);
	vparams = &(dptx->vparams);
	mdtd = &(dptx->vparams.mdtd);

	HISI_DRM_INFO("dptx_resolution %d*%d@%d",
		mdtd->h_active, mdtd->v_active, vparams->m_fps);
	dp_imonitor_set_param(DP_PARAM_WIDTH, &(mdtd->h_active));
	dp_imonitor_set_param(DP_PARAM_HIGH,  &(mdtd->v_active));
	dp_imonitor_set_param(DP_PARAM_FPS,   &(vparams->m_fps));

	dp_send_cable_notification(dptx, etype);
	HISI_DRM_INFO("[DP] -");
	return 0;
}

void dptx_link_set_lane_after_training(struct dp_ctrl *dptx)
{
	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	if (dptx->dptx_phy_enable_xmit)
		dptx->dptx_phy_enable_xmit(dptx, dptx->link.lanes, true);
	dptx->link.trained = true;

	if (dptx->dptx_video_ts_change)
		dptx->dptx_video_ts_change(dptx, 0);
	HISI_DRM_INFO("[DP] Link training succeeded rate=%d lanes=%d\n",
		 dptx->link.rate, dptx->link.lanes);
}

void dptx_link_close_stream(struct dp_ctrl *dptx)
{
	drm_check_and_void_return((dptx == NULL), "[DP] dptx is NULL\n");

	dptx_fec_disable(dptx);
	msleep(100);

	dptx_video_params_reset(&dptx->vparams);
	dptx_audio_params_reset(&dptx->aparams);

	/* Notify hwc */
	dp_send_cable_notification(dptx, (dptx->dptx_vr) ? HOT_PLUG_OUT_VR : HOT_PLUG_OUT);

	/* Disable DPTX */
	if (dptx->dptx_disable_default_video_stream)
		dptx->dptx_disable_default_video_stream(dptx, 0);
	/* Clear xmit enables */
	if (dptx->dptx_phy_enable_xmit)
		dptx->dptx_phy_enable_xmit(dptx, DPTX_DEFAULT_LINK_LANES, false);
	/* Power down all lanes */
	dptx_phy_set_lanes_power(dptx, false);
	/* Disable AUX Block */
	if (dptx->dptx_aux_disreset)
		dptx->dptx_aux_disreset(dptx, false);
}

