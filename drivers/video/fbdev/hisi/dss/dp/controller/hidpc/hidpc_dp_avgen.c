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

#include "hidpc_dp_avgen.h"
#include "../dp_avgen_base.h"
#include "hidpc_dp_core.h"
#include "hidpc_reg.h"
#include "../dp_core_interface.h"
#include "../../link/dp_mst_topology.h"
#include "hisi_dp.h"
#include "hisi_fb.h"
#include "hisi_fb_def.h"
#include "../../../dsc/dsc_algorithm_manager.h"
#include "../../link/dp_dsc_algorithm.h"
#include "../../link/dp_edid.h"
#include "../dsc/dsc_config_base.h"
#if defined(CONFIG_DP_HDCP)
#include "hdcp/hdcp_common.h"
#endif

#define OFFSET_FRACTIONAL_BITS 11

/*lint -save -e* */
void dptx_audio_sdp_en(struct dp_ctrl *dptx)
{
	uint32_t reg;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	reg = (uint32_t)dptx_readl(dptx, DPTX_SDP_VERTICAL_CTRL);
	reg |= DPTX_EN_AUDIO_STREAM_SDP;
	dptx_writel(dptx, DPTX_SDP_VERTICAL_CTRL, reg);

	reg = (uint32_t)dptx_readl(dptx, DPTX_SDP_HORIZONTAL_CTRL);
	reg |= DPTX_EN_AUDIO_STREAM_SDP;
	dptx_writel(dptx, DPTX_SDP_HORIZONTAL_CTRL, reg);
}

void dptx_audio_timestamp_sdp_en(struct dp_ctrl *dptx)
{
	uint32_t reg;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	reg = (uint32_t)dptx_readl(dptx, DPTX_SDP_VERTICAL_CTRL);
	reg |= DPTX_EN_AUDIO_TIMESTAMP_SDP;
	dptx_writel(dptx, DPTX_SDP_VERTICAL_CTRL, reg);
}

void dptx_audio_infoframe_sdp_send(struct dp_ctrl *dptx)
{
	uint32_t reg = 0;
	uint8_t sample_freq_cfg = 0;
	uint8_t data_width_cfg = 0;
	uint8_t num_channels_cfg = 0;
	uint8_t speaker_map_cfg = 0;
	uint32_t audio_infoframe_header = AUDIO_INFOFREAME_HEADER;
	uint32_t audio_infoframe_data[3] = {0x0, 0x0, 0x0};

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	sample_freq_cfg = dptx_audio_get_sample_freq_cfg(&dptx->aparams);
	audio_infoframe_data[0] |= sample_freq_cfg << DPTX_AUDIO_SAMPLE_FREQ_SHIFT;

	data_width_cfg = dptx_audio_get_data_width_cfg(&dptx->aparams);
	audio_infoframe_data[0] |= data_width_cfg << DPTX_AUDIO_SAMPLE_SIZE_SHIFT;

	num_channels_cfg = dptx_audio_get_num_channels_cfg(&dptx->aparams, &dptx->edid_info.Audio);
	audio_infoframe_data[0] |= num_channels_cfg << DPTX_AUDIO_CHANNEL_CNT_SHIFT;

	speaker_map_cfg = dptx_audio_get_speaker_map_cfg(&dptx->aparams, &dptx->edid_info.Audio);
	audio_infoframe_data[0] |= speaker_map_cfg << DPTX_AUDIO_SPEAKER_MAPPING_SHIFT;

	dptx->sdp_list[0].payload[0] = audio_infoframe_header;
	dptx_writel(dptx, DPTX_SDP_BANK, audio_infoframe_header);
	/* Synosys FAE luheng:
	 * set reg offset 0x604 to all zero. When infoframe is zero, sink just check stream head.
	 * Otherwire sink would checkout if inforame equal stream head info
	 */
	dptx_writel(dptx, DPTX_SDP_BANK + 4, audio_infoframe_data[0]);
	dptx_writel(dptx, DPTX_SDP_BANK + 8, audio_infoframe_data[1]);
	dptx_writel(dptx, DPTX_SDP_BANK + 12, audio_infoframe_data[2]);

	reg = (uint32_t)dptx_readl(dptx, DPTX_SDP_VERTICAL_CTRL);
	reg |= DPTX_EN_AUDIO_INFOFRAME_SDP;
	dptx_writel(dptx, DPTX_SDP_VERTICAL_CTRL, reg);
}


void dptx_hdr_infoframe_set_reg(struct dp_ctrl *dptx, uint8_t enable)
{
	int i;
	uint32_t reg;
	struct sdp_full_data hdr_sdp_data;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");
	dpu_check_and_no_retval((!dptx->dptx_enable), ERR, "[DP] dptx has already off.\n");

	dptx_config_hdr_payload(&hdr_sdp_data, &dptx->hdr_infoframe, enable);

	for (i = 0; i < DPTX_SDP_LEN; i++) {
		DPU_FB_DEBUG("[DP] hdr_sdp_data.payload[%d]: %x\n", i, hdr_sdp_data.payload[i]);
		dptx_writel(dptx, DPTX_SDP_BANK +  4 * i, hdr_sdp_data.payload[i]);
		reg = (uint32_t)dptx_readl(dptx, DPTX_SDP_BANK + 4 * i);
	}

	reg = (uint32_t)dptx_readl(dptx, DPTX_SDP_VERTICAL_CTRL);
	reg |= DPTX_EN_HDR_INFOFRAME_SDP | DPTX_FIXED_PRIORITY_ARBITRATION;
	dptx_writel(dptx, DPTX_SDP_VERTICAL_CTRL, reg);
}

void dptx_disable_sdp(struct dp_ctrl *dptx, uint32_t *payload, uint32_t len)
{
	int i;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] dptx is NULL\n");
	dpu_check_and_no_retval((payload == NULL), ERR, "[DP] payload is NULL\n");

	for (i = 0; i < DPTX_SDP_NUM; i++)
		if (!memcmp(dptx->sdp_list[i].payload, payload, len))
			memset(dptx->sdp_list[i].payload, 0, sizeof(dptx->sdp_list[i].payload));
}

void dptx_enable_sdp(struct dp_ctrl *dptx, struct sdp_full_data *data)
{
	uint32_t i;
	uint32_t reg;
	int reg_num;
	uint32_t header;
	int sdp_offset;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] dptx is NULL\n");
	dpu_check_and_no_retval((data == NULL), ERR, "[DP] data is NULL\n");

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

void dptx_fill_sdp(struct dp_ctrl *dptx, struct sdp_full_data *data)
{
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] dptx is NULL\n");
	dpu_check_and_no_retval((data == NULL), ERR, "[DP] data is NULL\n");

	if (data->en == 1)
		dptx_enable_sdp(dptx, data);
	else
		dptx_disable_sdp(dptx, data->payload, sizeof(data->payload));
}

void dptx_en_audio_channel(struct dp_ctrl *dptx, int ch_num, int enable)
{
	uint32_t reg;
	uint32_t data_en = 0;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	reg = (uint32_t)dptx_readl(dptx, DPTX_AUD_CONFIG1);
	reg &= ~DPTX_AUD_CONFIG1_DATA_EN_IN_MASK;

	if (enable) {
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
		reg |= data_en << DPTX_AUD_CONFIG1_DATA_EN_IN_SHIFT;
	} else {
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
		reg &= ~(data_en << DPTX_AUD_CONFIG1_DATA_EN_IN_SHIFT);
	}
	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);
}

void dptx_video_reset(struct dp_ctrl *dptx, int enable, int stream)
{
	uint32_t reg;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	reg = (uint32_t)dptx_readl(dptx, DPTX_SRST_CTRL);
	if (enable)
		reg |= DPTX_SRST_VIDEO_RESET_N((uint32_t)stream);
	else
		reg &= ~DPTX_SRST_VIDEO_RESET_N((uint32_t)stream);
	dptx_writel(dptx, DPTX_SRST_CTRL, reg);
}

void dptx_audio_mute(struct dp_ctrl *dptx)
{
	uint32_t reg;
	struct audio_params *aparams = NULL;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	aparams = &dptx->aparams;
	reg = (uint32_t)dptx_readl(dptx, DPTX_AUD_CONFIG1);

	if (aparams->mute == 1)
		reg |= DPTX_AUDIO_MUTE;
	else
		reg &= ~DPTX_AUDIO_MUTE;
	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);
}

void dptx_audio_config(struct dp_ctrl *dptx)
{
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	dptx_audio_core_config(dptx);

	dptx_audio_sdp_en(dptx);
	dptx_audio_timestamp_sdp_en(dptx);

	if (dptx->dptx_audio_infoframe_sdp_send)
		dptx->dptx_audio_infoframe_sdp_send(dptx);
}

void dptx_audio_core_config(struct dp_ctrl *dptx)
{
	struct audio_params *aparams = NULL;
	uint32_t reg;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	aparams = &dptx->aparams;

	dptx_audio_inf_type_change(dptx);
	if (dptx->dptx_audio_num_ch_change)
		dptx->dptx_audio_num_ch_change(dptx);
	dptx_audio_data_width_change(dptx);

	reg = (uint32_t)dptx_readl(dptx, DPTX_AUD_CONFIG1);
	reg &= ~DPTX_AUD_CONFIG1_ATS_VER_MASK;
	reg |= aparams->ats_ver << (unsigned int)DPTX_AUD_CONFIG1_ATS_VER_SHFIT;
	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);

	dptx_en_audio_channel(dptx, aparams->num_channels, 1);
}


void dptx_audio_inf_type_change(struct dp_ctrl *dptx)
{
	struct audio_params *aparams = NULL;
	uint32_t reg;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	aparams = &dptx->aparams;

	reg = (uint32_t)dptx_readl(dptx, DPTX_AUD_CONFIG1);
	reg &= ~DPTX_AUD_CONFIG1_INF_TYPE_MASK;
	reg |= aparams->inf_type << (unsigned int)DPTX_AUD_CONFIG1_INF_TYPE_SHIFT;
	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);
}

void dptx_audio_num_ch_change(struct dp_ctrl *dptx)
{
	uint32_t reg = 0;
	uint32_t num_ch_map = 0;
	struct audio_params *aparams = NULL;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	aparams = &dptx->aparams;

	reg = (uint32_t)dptx_readl(dptx, DPTX_AUD_CONFIG1);
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
	uint32_t reg;
	struct audio_params *aparams = NULL;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	aparams = &dptx->aparams;

	reg = (uint32_t)dptx_readl(dptx, DPTX_AUD_CONFIG1);
	reg &= ~DPTX_AUD_CONFIG1_DATA_WIDTH_MASK;
	reg |= aparams->data_width << (unsigned int)DPTX_AUD_CONFIG1_DATA_WIDTH_SHIFT;
	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);
}

/*
 * Video Generation
 */
void dptx_video_timing_change(struct dp_ctrl *dptx, int stream)
{
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	if (dptx->dptx_disable_default_video_stream)
		dptx->dptx_disable_default_video_stream(dptx, stream);
	if (dptx->dptx_video_core_config)
		dptx->dptx_video_core_config(dptx, stream);
	if (dptx->dptx_video_ts_change)
		dptx->dptx_video_ts_change(dptx, stream);
	if (dptx->dptx_enable_default_video_stream)
		dptx->dptx_enable_default_video_stream(dptx, stream);
}


int dptx_fec_enable(struct dp_ctrl *dptx, bool fec_enable)
{
	int retval;
	uint32_t reg;
	uint8_t result;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");

	if (fec_enable) {
		/* Forward Error Correction flow */
		reg = dptx_readl(dptx, DPTX_CCTL);
		reg |= DPTX_CCTL_ENH_FRAME_FEC_EN;
		dptx_writel(dptx, DPTX_CCTL, reg);

		/* Set FEC_READY on the sink side */
		retval = dptx_write_dpcd(dptx, DP_FEC_CONFIGURATION, DP_FEC_READY);
		if (retval)
			return retval;

		/* Enable forward error correction */
		reg = dptx_readl(dptx, DPTX_CCTL);
		reg |= DPTX_CCTL_ENABLE_FEC;
		dptx_writel(dptx, DPTX_CCTL, reg);

		DPU_FB_DEBUG("Enabling Forward Error Correction\n");

		retval = dptx_read_dpcd(dptx, DP_FEC_STATUS, &result);
		if (retval)
			DPU_FB_DEBUG("DPCD read failed\n");

		retval = dptx_read_dpcd(dptx, DP_FEC_ERROR_COUNT, &result);
		if (retval)
			DPU_FB_DEBUG("DPCD read failed\n");
	} else {
		/* Disable forward error correction */
		reg = dptx_readl(dptx, DPTX_CCTL);
		reg &= ~DPTX_CCTL_ENABLE_FEC;
		dptx_writel(dptx, DPTX_CCTL, reg);

		msleep(100); /* wait 100ms to close fec */
	}

	return 0;
}

static const uint8_t vcea_fractional_vblank_modes[] = {
	5, 6, 7, 10, 11, 20,
	21, 22, 39, 25, 26,
	40, 44, 45, 46, 50,
	51, 54, 55, 58, 59
};

void dptx_video_core_config(struct dp_ctrl *dptx, int stream)
{
	struct video_params *vparams = NULL;
	struct dtd *mdtd = NULL;
	uint32_t reg;
	uint8_t vmode;
	int i;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	vparams = &dptx->vparams;
	mdtd = &vparams->mdtd;

	vmode = vparams->mode;

	dptx_video_set_core_bpc(dptx, stream);
	/* Configure DPTX_VSAMPLE_POLARITY_CTRL register */
	reg = 0;

	if (mdtd->h_sync_polarity == 1)
		reg |= DPTX_POL_CTRL_H_SYNC_POL_EN;
	if (mdtd->v_sync_polarity == 1)
		reg |= DPTX_POL_CTRL_V_SYNC_POL_EN;

	dptx_writel(dptx, DPTX_VSAMPLE_POLARITY_CTRL_N(stream), reg);

	reg = 0;

	/* Configure video_config1 register */
	if (vparams->video_format == VCEA) {
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

	/* Configure video_msa1 register */
	reg = 0;
	reg |= (mdtd->h_blanking - mdtd->h_sync_offset)
		<< DPTX_VIDEO_MSA1_H_START_SHIFT;
	reg |= (mdtd->v_blanking - mdtd->v_sync_offset)
		<< DPTX_VIDEO_MSA1_V_START_SHIFT;
	dptx_writel(dptx, DPTX_VIDEO_MSA1_N(stream), reg);

	dptx_video_set_sink_bpc(dptx, stream);
}


int dptx_video_ts_calculate(struct dp_ctrl *dptx, int lane_num, int rate,
			    int bpc, int encoding, int pixel_clock)
{
	struct video_params *vparams = NULL;
	struct dtd *mdtd = NULL;
	int link_rate;
	int retval = 0;
	int ts;
	int tu;
	int tu_frac;
	int color_dep;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL\n");

	vparams = &dptx->vparams;
	mdtd = &vparams->mdtd;

	link_rate = dptx_br_to_link_rate(rate);
	if (link_rate < 0) {
		DPU_FB_DEBUG("Invalid rate param = %d\n", rate);
		return -EINVAL;
	}

	color_dep = dptx_get_color_depth(bpc, encoding);

	if (lane_num * link_rate == 0) {
		DPU_FB_ERR("[DP] lane_num = %d, link_rate = %d", lane_num, link_rate);
		return -EINVAL;
	}

	ts = (8 * color_dep * pixel_clock) / (lane_num * link_rate);
	tu  = ts / 1000;
	huawei_dp_imonitor_set_param(DP_PARAM_TU, &tu);

	if (tu >= 65) {
		DPU_FB_INFO("[DP] tu: %d > 65", tu);
		return -EINVAL;
	}

	tu_frac = ts / 100 - tu * 10;

	if (tu < 6)
		vparams->init_threshold = 32;
	else if ((encoding == RGB || encoding == YCBCR444) &&
		 mdtd->h_blanking <= 80)
		vparams->init_threshold = 12;
	else
		vparams->init_threshold = 15;

	DPU_FB_INFO("[DP] tu = %d\n", tu);

	vparams->aver_bytes_per_tu = (uint8_t)tu;

	vparams->aver_bytes_per_tu_frac = (uint8_t)tu_frac;

	return retval;
}

void dptx_video_ts_change(struct dp_ctrl *dptx, int stream)
{
	uint32_t reg;
	struct video_params *vparams = NULL;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	vparams = &dptx->vparams;

	reg = (uint32_t)dptx_readl(dptx, DPTX_VIDEO_CONFIG5_N(stream));
	reg = reg & (~DPTX_VIDEO_CONFIG5_TU_MASK);
	reg = reg | (vparams->aver_bytes_per_tu <<
			DPTX_VIDEO_CONFIG5_TU_SHIFT);
	reg = reg & (~DPTX_VIDEO_CONFIG5_TU_FRAC_MASK_SST);
	reg = reg | (vparams->aver_bytes_per_tu_frac <<
		       DPTX_VIDEO_CONFIG5_TU_FRAC_SHIFT_SST);

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

static int dptx_video_get_bpc_mapping(struct dp_ctrl *dptx, uint8_t *bpc_mapping)
{
	int i;
	uint8_t bpc = 0;
	enum pixel_enc_type pix_enc;
	struct video_params *vparams = NULL;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");

	vparams = &dptx->vparams;
	bpc = vparams->bpc;
	pix_enc = vparams->pix_enc;
	*bpc_mapping = 0; /* set default value of bpc_mapping */

	for (i = 0; i < ARRAY_SIZE(hidpc_video_mapping); i++) {
		struct video_bpc_mapping vmap = hidpc_video_mapping[i];

		if (pix_enc == vmap.pix_enc && bpc == vmap.bpc) {
			*bpc_mapping = vmap.bpc_mapping;
			break;
		}
	}

	if (i == ARRAY_SIZE(hidpc_video_mapping))
		DPU_FB_ERR("[DP] No corresponding bpc_mapping\n");

	return 0;
}


void dptx_video_bpc_change(struct dp_ctrl *dptx, int stream)
{
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	dptx_video_set_core_bpc(dptx, stream);
	dptx_video_set_sink_bpc(dptx, stream);
}

void dptx_video_set_core_bpc(struct dp_ctrl *dptx, int stream)
{
	uint32_t reg;
	uint8_t bpc_mapping;
	int retval;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

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
	uint32_t reg_msa2;
	uint8_t col_mapping;
	uint8_t colorimetry;
	uint8_t dynamic_range;
	struct video_params *vparams = NULL;
	enum pixel_enc_type pix_enc;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

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
	default:
		break;
	}

	reg_msa2 |= (col_mapping << DPTX_VIDEO_VMSA2_COL_SHIFT);
	dptx_writel(dptx, DPTX_VIDEO_MSA2_N(stream), reg_msa2);
}

void dptx_video_set_sink_bpc(struct dp_ctrl *dptx, int stream)
{
	uint32_t reg_msa2, reg_msa3;
	uint8_t bpc_mapping = 0;
	uint8_t bpc = 0;
	struct video_params *vparams = NULL;
	enum pixel_enc_type pix_enc;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

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
	uint32_t vsamplectrl;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	vsamplectrl = dptx_readl(dptx, DPTX_VSAMPLE_CTRL_N(stream));
	vsamplectrl &= ~DPTX_VSAMPLE_CTRL_STREAM_EN;
	dptx_writel(dptx, DPTX_VSAMPLE_CTRL_N(stream), vsamplectrl);

	if ((dp_get_dptx_feature_status(dptx)) && (dptx->dptx_detect_inited)) {
		DPU_FB_INFO("[DP] Cancel dptx detect err count when disable video stream.\n");
		hrtimer_cancel(&dptx->dptx_hrtimer);
	}
}

void dptx_enable_default_video_stream(struct dp_ctrl *dptx, int stream)
{
	uint32_t vsamplectrl;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	vsamplectrl = dptx_readl(dptx, DPTX_VSAMPLE_CTRL_N(stream));
	vsamplectrl |= DPTX_VSAMPLE_CTRL_STREAM_EN;
	dptx_writel(dptx, DPTX_VSAMPLE_CTRL_N(stream), vsamplectrl);

	if ((dp_get_dptx_feature_status(dptx)) && (dptx->dptx_detect_inited)) {
		DPU_FB_INFO("[DP] restart dptx detect err count when enable video stream.\n");
		hrtimer_restart(&dptx->dptx_hrtimer);
	}
}

static void dptx_timing_config(struct dp_ctrl *dptx)
{
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	/* video data can' be sent before DPTX timing configure. */
	if (dptx->dptx_triger_media_transfer)
		dptx->dptx_triger_media_transfer(dptx, false);

	dptx_video_reset(dptx, 1, 0);
	mdelay(100);
	dptx_video_reset(dptx, 0, 0);

	/* Update DP reg configue */
	if (dptx->dptx_video_timing_change)
		dptx->dptx_video_timing_change(dptx, 0); /* dptx video reg depends on dss pixel clock. */


	if (dptx->dptx_audio_config)
		dptx->dptx_audio_config(dptx); /* dptx audio reg depends on phy status(P0) */

	if (dptx->dptx_triger_media_transfer)
		dptx->dptx_triger_media_transfer(dptx, true);
}

int dptx_link_timing_config(struct dpu_fb_data_type *dpufd, struct dp_ctrl *dptx)
{
	int retval = 0;
	struct video_params *vparams = NULL;

	dpu_check_and_return((dpufd == NULL), -EINVAL, ERR, "[DP] dpufd is NULL\n");
	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL\n");

	/* enable fec */
	if (dptx->fec && dptx->dptx_fec_enable) {
		retval = dptx->dptx_fec_enable(dptx, true);
		if (retval) {
			DPU_FB_ERR("[DP] fec enable failed!\n");
			return retval;
		}
	}


	/* Enable audio SDP */
	dptx_audio_sdp_en(dptx);
	dptx_audio_timestamp_sdp_en(dptx);

	/* enable VCS if YCBCR420 is enabled */
	vparams = &dptx->vparams;

	/* DP update device to HWC and configue DSS */
	if (dp_get_dptx_feature_status(dptx))
		retval = dptx_update_dss_and_hwc(dptx, HOT_PLUG_IN_VR);
	else
		retval = dptx_update_dss_and_hwc(dptx, HOT_PLUG_IN);
	if (retval)
		return retval;

	dptx_timing_config(dptx);

	return 0;
}

int dptx_triger_media_transfer(struct dp_ctrl *dptx, bool benable)
{
	struct audio_params *aparams = NULL;
	struct video_params *vparams = NULL;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");

	aparams = &(dptx->aparams);
	vparams = &(dptx->vparams);

	if (dptx->video_transfer_enable) {
		if (benable) {
			enable_ldi(dptx->dpufd);
			dptx_en_audio_channel(dptx, aparams->num_channels, 1);
		} else {
			dptx_en_audio_channel(dptx, aparams->num_channels, 0);
			disable_ldi(dptx->dpufd);
			if (vparams->m_fps > 0)
				mdelay(1000 / vparams->m_fps + 10);
			else
				return -1;
		}
	}
	return 0;
}

int dptx_resolution_switch(struct dpu_fb_data_type *dpufd, enum dptx_hot_plug_type etype)
{
	struct dtd *mdtd = NULL;
	struct dp_ctrl *dptx = NULL;
	struct dpu_panel_info *pinfo = NULL;
	int ret;

	dpu_check_and_return((dpufd == NULL), -EINVAL, ERR, "[DP] dpufd is NULL\n");

	dptx = &(dpufd->dp);
	mdtd = &(dptx->vparams.mdtd);
	pinfo = &(dpufd->panel_info);

	ret = dptx_update_panel_info(dpufd);
	if (ret < 0)
		return ret;

	pinfo->ldi.hsync_plr = 1 - mdtd->h_sync_polarity;
	pinfo->ldi.vsync_plr = 1 - mdtd->v_sync_polarity;

	dptx_debug_resolution_info(dpufd);

	ret = dptx_dss_plugin(dpufd, etype);
	return ret;
}

void dptx_link_set_lane_after_training(struct dp_ctrl *dptx)
{
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	if (dptx->dptx_phy_enable_xmit)
		dptx->dptx_phy_enable_xmit(dptx, dptx->link.lanes, true);
	dptx->link.trained = true;

#if defined(CONFIG_DP_HDCP)
	hdcp_read_te_info(dptx);
#endif

	if (dptx->dptx_video_ts_change)
		dptx->dptx_video_ts_change(dptx, 0);
	DPU_FB_INFO("[DP] Link training succeeded rate=%d lanes=%d\n",
		 dptx->link.rate, dptx->link.lanes);
}

void dptx_link_close_stream(struct dpu_fb_data_type *dpufd, struct dp_ctrl *dptx)
{
	dpu_check_and_no_retval((dpufd == NULL), ERR, "[DP] dpufd is NULL\n");
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] dptx is NULL\n");

	if (dptx->fec && dptx->dptx_fec_enable)
		dptx->dptx_fec_enable(dptx, false);

	dptx_video_params_reset(&dptx->vparams);
	dptx_audio_params_reset(&dptx->aparams);

	/* Notify hwc */
	dp_send_cable_notification(dptx, (dp_get_dptx_feature_status(dptx)) ? HOT_PLUG_OUT_VR : HOT_PLUG_OUT);

	/* Disable DSS */
	if (dpufd->hpd_release_sub_fnc)
		dpufd->hpd_release_sub_fnc(dpufd->fbi);

	/* Disable DPTX */
	if (dptx->dptx_disable_default_video_stream)
		dptx->dptx_disable_default_video_stream(dptx, 0);
	/* Clear xmit enables */
	if (dptx->dptx_phy_enable_xmit)
		dptx->dptx_phy_enable_xmit(dptx, 4, false);
	/* Power down all lanes */
	dptx_phy_set_lanes_power(dptx, false);
	/* Disable AUX Block */
	if (dptx->dptx_aux_disreset)
		dptx->dptx_aux_disreset(dptx, false);
}

