/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
/*lint -e429 -e529*/
#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wunused-variable"
#include <securec.h>

#include "hisi_dp_drv.h"
#include "hisi_defs.h"
#include "dp_edid.h"
#include "dp_dtd_helper.h"

static const u8 edid_v1_other_descriptor_flag[2] = {0x00, 0x00};

static struct list_head *dptx_hdmi_list;
struct dptx_hdmi_vic {
	struct list_head list_node;
	u32 vic_id;
};
static u8 g_hdmi_vic_len;
static u8 g_hdmi_vic_real_len;

static int block_type(u8 *block)
{
	if ((block[0] == 0) && (block[1] == 0)) {
		/* Other descriptor */
		if ((block[2] != 0) || (block[4] != 0))
			return UNKNOWN_DESCRIPTOR;
		return block[3];
	}
	/* Detailed timing block */
	return DETAILED_TIMING_BLOCK;
}

static int parse_monitor_limits(struct dp_ctrl *dptx, u8 *block)
{
	struct edid_video *vid_info = NULL;

	vid_info = &(dptx->edid_info.video);
	/* Set up limit fields */
	vid_info->max_h_freq = H_MAX_RATE;
	vid_info->min_h_freq = H_MIN_RATE;
	vid_info->max_v_freq = V_MAX_RATE;
	vid_info->min_v_freq = V_MIN_RATE;
	vid_info->max_pixel_clock = (int)MAX_PIXEL_CLOCK;

	return 0;
}

static int parse_monitor_name(struct dp_ctrl *dptx, u8 *blockname, u32 size)
{
	u32 i, data0, data1, data2;
	u32 str_start, str_end;
	u8 data_tmp;
	int ret;

	str_start = 5;
	str_end = 0;
	data_tmp = 0;

	drm_check_and_return((size != DETAILED_TIMING_DESCRIPTION_SIZE), -EINVAL, ERR,
		"[DP] The length of monitor name is wrong!\n");

	if (size >= 7) {
		data0 = blockname[5];
		data1 = blockname[6];
		for (i = 7; i < size; i++) {
			data2 = blockname[i];
			if (((data0 == 0x0A) && (data1 == 0x20)) && (data2 == 0x20)) {
				str_end = i - 2;
				break;
			}
			data0 = data1;
			data1 = data2;
		}
	}

	if ((blockname[16] == 0x0A) && (blockname[17] == 0x20))
		str_end = 16;

	if (str_end < str_start)
		return 0;

	if (((str_end - str_start) > 0) && (dptx->edid_info.video.dp_monitor_descriptor != NULL)) {
		ret = memcpy_s(dptx->edid_info.video.dp_monitor_descriptor, MONTIOR_NAME_DESCRIPTION_SIZE,
			blockname + 5, (str_end - str_start));
		drm_check_and_return(ret != EOK, -EINVAL, ERR, "[DP] memcpy for monitor descriptor falied!");
		dptx->edid_info.video.dp_monitor_descriptor[str_end - str_start] = '\0';

		/* For hiding custom device info */
		for (i = str_start; i < str_end; i++)
			data_tmp += blockname[i];

		ret = memset_s(&blockname[5], (str_end - str_start), 0x0, (str_end - str_start));
		drm_check_and_return(ret != EOK, -EINVAL, ERR, "[DP] memset for blockname failed!");
		blockname[5] = data_tmp; /* For EDID checksum, we need reserve the sum for blocks. */
	}
	return 0;
}

static int check_exten_edid(u8 *exten)
{
	u32 i;
	u32 checksum = 0;

	for (i = 0; i < EDID_LENGTH; i++)
		checksum += exten[i];
	if (checksum & 0xFF) {
		HISI_DRM_ERR("[DP] Extension Data checksum failed - data is corrupt. Continuing anyway.\n");
		return -EINVAL;
	}
	/* Check Extension Tag */
	/* Header Tag stored in the first u8 */
	if (exten[0] != EXTENSION_HEADER) {
		HISI_DRM_ERR("[DP] Not CEA-EDID Timing Extension, Extension-Parsing will not continue!\n");
		return -EINVAL;
	}
	return 0;
}

static int parse_timing_description(struct dp_ctrl *dptx, u8 *dtd)
{
	struct edid_video *vid_info = NULL;
	struct timing_info *node = NULL;

	vid_info = &(dptx->edid_info.video);

	if (vid_info->dptx_timing_list == NULL) {
		vid_info->dptx_timing_list = kzalloc(sizeof(struct list_head), GFP_KERNEL);
		if (vid_info->dptx_timing_list == NULL) {
			HISI_DRM_ERR("[DP] vid_info->dptx_timing_list is NULL");
			return -EINVAL;
		}
		INIT_LIST_HEAD(vid_info->dptx_timing_list);

		vid_info->main_v_count = 0;
		/* Initial Max Value */
		vid_info->max_h_pixels = H_ACTIVE;
		vid_info->max_v_pixels = V_ACTIVE;
		vid_info->max_pixel_clock = PIXEL_CLOCK;
	}

	/* Get Max Value by comparing all values */
	if ((vid_info->max_h_pixels <= H_ACTIVE) &&
		(vid_info->max_v_pixels <= V_ACTIVE) &&
		(vid_info->max_pixel_clock < PIXEL_CLOCK)) {
		vid_info->max_h_pixels = H_ACTIVE;
		vid_info->max_v_pixels = V_ACTIVE;
		vid_info->max_pixel_clock = PIXEL_CLOCK;
	}
	/* node */
	node = kzalloc(sizeof(struct timing_info), GFP_KERNEL);
	if (node != NULL) {
		node->h_active_pixels = H_ACTIVE;
		node->h_blanking = H_BLANKING;
		node->h_sync_offset = H_SYNC_OFFSET;
		node->h_sync_pulse_width = H_SYNC_WIDTH;
		node->h_border = H_BORDER;
		node->h_size = H_SIZE;

		node->v_active_pixels = V_ACTIVE;
		node->v_blanking = V_BLANKING;
		node->v_sync_offset = V_SYNC_OFFSET;
		node->v_sync_pulse_width = V_SYNC_WIDTH;
		node->v_border = V_BORDER;
		node->v_size = V_SIZE;

		node->pixel_clock = PIXEL_CLOCK;

		node->input_type = INPUT_TYPE; /* need to modify later */
		node->interlaced = INTERLACED;
		node->v_sync_polarity = V_SYNC_POLARITY;
		node->h_sync_polarity = H_SYNC_POLARITY;
		node->sync_scheme = SYNC_SCHEME;
		node->scheme_detail = SCHEME_DETAIL;

		vid_info->main_v_count += 1;
		list_add_tail(&node->list_node, vid_info->dptx_timing_list);
	} else {
		kfree(vid_info->dptx_timing_list);
		vid_info->dptx_timing_list = NULL;
		HISI_DRM_ERR("[DP] kzalloc struct dptx_hdmi_vic fail!\n");
		return -EINVAL;
	}

	HISI_DRM_INFO("[DP] The timinginfo %d: pixel clock = %llu",
			vid_info->main_v_count, node->pixel_clock);
	HISI_DRM_INFO("h_active_pixels is %d, v_active_pixels is %d,",
			node->h_active_pixels, node->v_active_pixels);
	dp_imonitor_set_param_timing(node->h_active_pixels, node->v_active_pixels,
		node->pixel_clock, 0);

	return 0;
}

static int parse_extension_timing_description(struct dp_ctrl *dptx, u8 *dtd_block,
	u32 dtd_begin, u16 dtd_total)
{
	u32 i;
	int ret;

	if ((dtd_begin + 1 + dtd_total * DETAILED_TIMING_DESCRIPTION_SIZE) > EDID_LENGTH) {
		HISI_DRM_ERR("[DP] The dtd total number 0x%x is out of the limit\n", dtd_total);
		return -EINVAL;
	}

	for (i = 0; i < dtd_total; i++, dtd_block += DETAILED_TIMING_DESCRIPTION_SIZE) {
		switch (block_type(dtd_block)) {
		case DETAILED_TIMING_BLOCK:
			ret = parse_timing_description(dptx, dtd_block);
			if (ret != 0) {
				HISI_DRM_ERR("[DP] Timing Description Parsing failed!");
				return ret;
			}
			break;
		case MONITOR_LIMITS:
			parse_monitor_limits(dptx, dtd_block);
			break;
		default:
			break;
		}
	}
	return 0;
}

static int parse_timing_description_by_vesaid(struct edid_video *vid_info, u8 vesa_id)
{
	struct dtd mdtd;
	struct timing_info *node = NULL;

	if (!convert_code_to_dtd(&mdtd, vesa_id, 60000, VCEA)) {
		HISI_DRM_ERR("[DP] Invalid video mode value %d\n", vesa_id);
		return -EINVAL;
	}

	if (mdtd.interlaced) {
		HISI_DRM_INFO("[DP] Don't Support interlace mode %d\n",
						vesa_id);
		return -EINVAL;
	}

	if (vid_info->dptx_timing_list == NULL) {
		vid_info->dptx_timing_list = kzalloc(sizeof(struct list_head), GFP_KERNEL);
		if (vid_info->dptx_timing_list == NULL) {
			HISI_DRM_ERR("[DP] vid_info->dptx_timing_list is NULL");
			return -EINVAL;
		}
		INIT_LIST_HEAD(vid_info->dptx_timing_list);

		vid_info->main_v_count = 0;
		/* Initial Max Value */
		vid_info->max_h_pixels = mdtd.h_active;
		vid_info->max_v_pixels = mdtd.v_active;
		vid_info->max_pixel_clock = mdtd.pixel_clock / 10;
	}

	/* Get Max Value by comparing all values */
	if ((mdtd.h_active > vid_info->max_h_pixels) &&
	    (mdtd.v_active >= vid_info->max_v_pixels) &&
		((mdtd.pixel_clock / 10) > vid_info->max_pixel_clock)) {
		vid_info->max_h_pixels = mdtd.h_active;
		vid_info->max_v_pixels = mdtd.v_active;
		vid_info->max_pixel_clock = mdtd.pixel_clock / 10;
	}

	/* node */
	node = kzalloc(sizeof(struct timing_info), GFP_KERNEL);
	if (node != NULL) {
		node->h_active_pixels = mdtd.h_active;
		node->h_blanking = mdtd.h_blanking;
		node->h_sync_offset = mdtd.h_sync_offset;
		node->h_sync_pulse_width = mdtd.h_sync_pulse_width;
		node->h_size = mdtd.h_image_size;

		node->v_active_pixels = mdtd.v_active;
		node->v_blanking = mdtd.v_blanking;
		node->v_sync_offset = mdtd.v_sync_offset;
		node->v_sync_pulse_width = mdtd.v_sync_pulse_width;
		node->v_size = mdtd.v_image_size;

		node->pixel_clock = mdtd.pixel_clock / 10; /* Edid pixel clock is 1/10 of dtd filled timing. */
		node->interlaced = mdtd.interlaced;
		node->v_sync_polarity = mdtd.v_sync_polarity;
		node->h_sync_polarity = mdtd.h_sync_polarity;

		vid_info->main_v_count += 1;
		list_add_tail(&node->list_node, vid_info->dptx_timing_list);
	} else {
		kfree(vid_info->dptx_timing_list);
		vid_info->dptx_timing_list = NULL;
		HISI_DRM_ERR("[DP] kzalloc struct dptx_hdmi_vic fail!\n");
		return -EINVAL;
	}

	HISI_DRM_INFO("[DP] The timinginfo %d: pixel clock = %llu",
		vid_info->main_v_count, node->pixel_clock);
	HISI_DRM_INFO("h_active_pixels is %d, v_active_pixels is %d,",
		node->h_active_pixels, node->v_active_pixels);
	dp_imonitor_set_param_timing(node->h_active_pixels, node->v_active_pixels,
		node->pixel_clock, vesa_id);

	return 0;
}

static int parse_audio_spec_info(struct edid_audio *aud_info, struct edid_audio_info *spec_info,
		u8 *cdblock)
{
	if ((EXTEN_AUDIO_FORMAT <= 8) && (EXTEN_AUDIO_FORMAT >= 1)) {
		/* Set up SAD fields */
		spec_info->format = EXTEN_AUDIO_FORMAT;
		spec_info->channels =  EXTEN_AUDIO_MAX_CHANNELS;
		spec_info->sampling = EXTEN_AUDIO_SAMPLING;
		if (EXTEN_AUDIO_FORMAT == 1)
			spec_info->bitrate = EXTEN_AUDIO_LPCM_BIT;
		else
			spec_info->bitrate = EXTEN_AUDIO_BITRATE;
		aud_info->ext_a_count += 1;

		HISI_DRM_INFO("[DP] parse audio spec success");
		HISI_DRM_INFO("[DP] format(0x%x), channels(0x%x)",
			spec_info->format, spec_info->channels);
		HISI_DRM_INFO("[DP] sampling(0x%x), bitrate(0x%x)",
			spec_info->sampling, spec_info->bitrate);
	}
	return 0;
}

static int parse_extension_audio_tag(struct edid_audio *aud_info, u8 *cdblock, u8 templ)
{
	u8 i;
	u8 xa;
	void *temp_ptr = NULL;
	int ret;

	drm_check_and_return((templ < 1), -EINVAL, ERR, "[DP] The input param tempL is wrong!\n");

	for (i = 0; i < (templ) / 3; i++) {
		xa = aud_info->ext_a_count;

		if (xa == 0) {
			/* Initial audio part */
			if (aud_info->spec != NULL) {
				HISI_DRM_ERR("[DP] The spec of audio is error\n");
				return -EINVAL;
			}
			aud_info->spec = kzalloc(sizeof(struct edid_audio_info), GFP_KERNEL);
			if (aud_info->spec == NULL) {
				HISI_DRM_ERR("[DP] malloc Audio Spec part failed\n");
				return -EINVAL;
			}
		} else {
			/* Add memory as needed with error handling */
			temp_ptr = kzalloc((xa + 1) * sizeof(struct edid_audio_info), GFP_KERNEL);
			if (temp_ptr == NULL) {
				HISI_DRM_ERR("[DP] Realloc Audio Spec part failed\n");
				return -EINVAL;
			}

			if (aud_info->spec == NULL) {
				HISI_DRM_ERR("[DP] The spec is NULL\n");
				kfree(temp_ptr);
				return -EINVAL;
			}
			ret = memcpy_s(temp_ptr, xa * sizeof(struct edid_audio_info), aud_info->spec, xa * sizeof(struct edid_audio_info));
			if (ret != EOK) {
				kfree(temp_ptr);
				HISI_DRM_ERR("[DP] memcpy_s failed");
				return -EINVAL;
			}

			kfree(aud_info->spec);
			aud_info->spec = temp_ptr;
		}
		if (parse_audio_spec_info(aud_info, &(aud_info->spec[xa]), cdblock)) {
			kfree(aud_info->spec);
			aud_info->spec = NULL;
			HISI_DRM_ERR("[DP] parse the audio spec info fail\n");
			return -EINVAL;
		}

		cdblock += 3;
	}
	return 0;
}

int parse_extension_video_tag(struct edid_video *vid_info, u8 *cdblock, u8 length)
{
	u8 i;
	int ret;

	drm_check_and_return((length < 1), -EINVAL, ERR, "[DP] The input param tempL is wrong!\n");

	vid_info->ext_timing = kcalloc(length, sizeof(struct ext_timing), GFP_KERNEL);
	if (vid_info->ext_timing == NULL) {
		HISI_DRM_ERR("[DP] ext_timing memory alloc fail\n");
		return -EINVAL;
	}
	ret = memset_s(vid_info->ext_timing, length * sizeof(struct ext_timing), 0x0, length * sizeof(struct ext_timing));
	if (ret != EOK) {
		kfree(vid_info->ext_timing);
		vid_info->ext_timing = NULL;
		HISI_DRM_ERR("memset failed");
		return -EINVAL;
	}
	vid_info->ext_v_count = 0;

	for (i = 0; i < length; i++) {
		if (EXTEN_VIDEO_CODE != 0) {
			/* Set up SVD fields */
			vid_info->ext_timing[i].ext_format_code = EXTEN_VIDEO_CODE;
			vid_info->ext_v_count += 1;
			parse_timing_description_by_vesaid(vid_info,
				vid_info->ext_timing[i].ext_format_code);
		}
		cdblock += 1;
	}
	return 0;
}

static int parse_hdmi_vic_id(u8 vic_id)
{
	struct dptx_hdmi_vic *node = NULL;

	if (g_hdmi_vic_real_len >= g_hdmi_vic_len) {
		HISI_DRM_ERR("[DP] The g_hdmi_vic_real_len is more than g_hdmi_vic_len.\n");
		return -EINVAL;
	}

	if (dptx_hdmi_list == NULL) {
		dptx_hdmi_list = kzalloc(sizeof(struct list_head), GFP_KERNEL);
		if (dptx_hdmi_list == NULL) {
			HISI_DRM_ERR("[DP] dptx_hdmi_list is NULL");
			return -EINVAL;
		}
		INIT_LIST_HEAD(dptx_hdmi_list);
	}

	/* node */
	node = kzalloc(sizeof(struct dptx_hdmi_vic), GFP_KERNEL);
	if (node != NULL) {
		node->vic_id = vic_id;
		HISI_DRM_INFO("[DP] vic_id = %d!\n", vic_id);
		list_add_tail(&node->list_node, dptx_hdmi_list);
		g_hdmi_vic_real_len++;
	} else {
		HISI_DRM_ERR("[DP] kzalloc struct dptx_hdmi_vic fail!\n");
	}

	return 0;
}

static void parse_extension_vsdb_after_latency(u8 *cdblock, u8 length, u8 pos_after_latency)
{
	u8 i;
	bool b3dpresent = false;

	drm_check_and_void_return((cdblock == NULL), "[DP] cdblock is NULL!\n");

	for (i = pos_after_latency; i < length; i++) {
		if (i == pos_after_latency) {
			b3dpresent = (cdblock[i] & 0x80) >> 7;
		} else if (i == pos_after_latency + 1) {
			g_hdmi_vic_len = (cdblock[i] & 0xE0) >> 5;
			if (g_hdmi_vic_len == 0) {
				HISI_DRM_INFO("[DP] This EDID don't include HDMI additional video format (2).\n");
				return;
			}
			g_hdmi_vic_real_len = 0;
		} else if (i <= pos_after_latency + 1 + g_hdmi_vic_len) {
			parse_hdmi_vic_id(cdblock[i]);
		} else {
			return;
		}
	}
}

static void parse_hdmi_vsdb_timing_list(struct edid_video *vid_info)
{
	struct dptx_hdmi_vic *hdmi_vic_node = NULL;
	struct dptx_hdmi_vic *_node_ = NULL;

	drm_check_and_void_return((dptx_hdmi_list == NULL), "[DP] dptx_hdmi_list is NULL!\n");

#ifdef SUPPORT_HDMI_VSDB_BLOCK
	list_for_each_entry_safe(hdmi_vic_node, _node_, dptx_hdmi_list, list_node) {
		switch (hdmi_vic_node->vic_id) {
		case 1:
			parse_timing_description_by_vesaid(vid_info, 95);
			break;
		case 2:
			parse_timing_description_by_vesaid(vid_info, 94);
			break;
		case 3:
			parse_timing_description_by_vesaid(vid_info, 93);
			break;
		case 4:
			parse_timing_description_by_vesaid(vid_info, 98);
			break;
		default:
			HISI_DRM_ERR("[DP] hdmi_vic_node is illegal!\n");
			break;
		}
		list_del(&hdmi_vic_node->list_node);
		kfree(hdmi_vic_node);
	}
#else
	UNUSED(hdmi_vic_node);
	UNUSED(_node_);
#endif
	kfree(dptx_hdmi_list);
	dptx_hdmi_list = NULL;
}

static int parse_extension_vsdb_tag(struct edid_video *vid_info, u8 *cdblock, u8 length)
{
	u8 i;
	uint32_t ieee_flag;

	u8 latency_fields_present;
	u8 i_latency_fields_present;
	u8 hdmi_video_present;
	u8 latency_present;

	g_hdmi_vic_real_len = 0;
	g_hdmi_vic_len = 0;
	latency_fields_present = 0;
	i_latency_fields_present = 0;

	drm_check_and_return((length < 5), -EINVAL, ERR, "[DP] VSDB length isn't correct!\n");

	ieee_flag = 0;
	ieee_flag = (cdblock[0]) | (cdblock[1] << 8) | (cdblock[2] << 16);

	if (ieee_flag != 0x000c03) {
		HISI_DRM_INFO("[DP] This block isn't belong to HDMI block: %x.\n", ieee_flag);
		return 0;
	}

	if (length > 7) {
		i = 7;
		latency_fields_present = (cdblock[i] & 0x80) >> 7;
		i_latency_fields_present = (cdblock[i] & 0x40) >> 6;
		hdmi_video_present = (cdblock[i] & 0x20) >> 5;
		if (hdmi_video_present == 0) {
			HISI_DRM_INFO("[DP] This EDID don't include HDMI additional video format (1).\n");
			return 0;
		}
	}

	latency_present = latency_fields_present + i_latency_fields_present;

	if (length > 2 * latency_present + 8)
		parse_extension_vsdb_after_latency(cdblock, length, 2 * latency_present + 8);

	if (g_hdmi_vic_len == 0)
		return 0;
	HISI_DRM_INFO("[DP] vic_id real length  = %d , vic length = %d !\n", g_hdmi_vic_real_len, g_hdmi_vic_len);

	parse_hdmi_vsdb_timing_list(vid_info);

	return 0;
}

static int parse_cea_data_block(struct dp_ctrl *dptx, u8 *cea_data, u8 dtd_start)
{
	u8 total_length, block_length;
	u8 *cdblock = cea_data;
	struct edid_video *vid_info = NULL;
	struct edid_audio *aud_info = NULL;
	/* exTlist *extlist; */
	/* Initialize some fields */
	drm_check_and_return((dtd_start > (EDID_LENGTH - 1)), -EINVAL, ERR, "[DP] The start of dtd is out of limit!\n");

	total_length = 4;
	vid_info = &(dptx->edid_info.video);
	aud_info = &(dptx->edid_info.audio);
	/* Parse CEA Data Block Collection */
	while (total_length < dtd_start) {
		/* Get length(total number of following u8s of a certain tag) */
		block_length = GET_CEA_DATA_BLOCK_LEN(cdblock);
		/* Get tag code */
		switch (GET_CEA_DATA_BLOCK_TAG(cdblock)) {
		case EXTENSION_AUDIO_TAG:
			/* Block type tag combined with data length takes 1 u8 */
			cdblock += 1;
			/* Each Short Audio Descriptor(SAD) takes 3 u8s */
			if (parse_extension_audio_tag(aud_info, cdblock, block_length)) {
				HISI_DRM_ERR("[DP] parse_extension_audio_tag fail\n");
				return -EINVAL;
			}
			cdblock += block_length;
			break;
		case EXTENSION_VIDEO_TAG:
			/* Block type tag combined with data length takes 1 u8 */
			cdblock += 1;
			/* Each Short Video Descriptor(SVD) takes 1 u8 */
			if (parse_extension_video_tag(vid_info, cdblock, block_length)) {
				HISI_DRM_ERR("[DP] parse_extension_video_tag fail\n");
				return -EINVAL;
			}
			cdblock += block_length;
			break;
		case EXTENSION_VENDOR_TAG:
			cdblock += 1;
			if (parse_extension_vsdb_tag(vid_info, cdblock, block_length)) {
				HISI_DRM_ERR("[DP] parse_extension_vsdb_tag fail\n");
				return -EINVAL;
			}
			cdblock += block_length;
			break;
		case EXTENSION_SPEAKER_TAG:
			/*
			 * If the extension has Speaker Allocation Data,
			 * there should only be one
			 */
			cdblock += 1;
			aud_info->ext_speaker = EXTEN_SPEAKER;
			/* Speaker Allocation Data takes 3 u8s */
			cdblock += 3;
			break;
		default:
			cdblock += block_length + 1;
			break;
		}
		total_length = total_length + block_length + 1;
	}

	return 0;
}

static int parse_extension(struct dp_ctrl *dptx, u8 *exten)
{
	int ret;
	u8 *dtd_block = NULL;
	u8 *cea_block = NULL;
	u8 dtd_start_byte = 0;
	u8 cea_data_block_collection = 0;
	u16 dtd_total;
	struct edid_audio *aud_info = NULL;

	aud_info = &(dptx->edid_info.audio);

	ret = check_exten_edid(exten);
	if (ret) {
		HISI_DRM_ERR("[DP] The check_exten_edid failed\n");
		return ret;
	}
	/*
	 * Get u8 number (decimal) within this block where the 18-u8 DTDs begin.
	 * (Number data stored in the third u8)
	 */
	if (exten[2] == 0x00) {
		HISI_DRM_INFO("[DP] There are no DTDs present in this extension block and no non-DTD data\n");
		return -EINVAL;
	} else if (exten[2] == 0x04) {
		dtd_start_byte = 0x04;
	} else {
		cea_data_block_collection = 0x04;
		dtd_start_byte = exten[2];
	}
	/*
	 * Get Number of Native DTDs present, other Version 2+ information
	 * DTD Total stored in the fourth u8
	 */
	if (aud_info->basic_audio != 1)
		aud_info->basic_audio = (0x40 & exten[3]) >> 6;

	dp_imonitor_set_param(DP_PARAM_BASIC_AUDIO, &(aud_info->basic_audio));

	dtd_total = LOWER_NIBBLE(exten[3]);
	/* Parse DTD in Extension */
	dtd_block = exten + dtd_start_byte;
	if (dtd_total != (EDID_LENGTH - dtd_start_byte - 1) /
		DETAILED_TIMING_DESCRIPTION_SIZE) {
		HISI_DRM_INFO("[DP] The number of native DTDs is not equal the size\n");
		dtd_total = (EDID_LENGTH - dtd_start_byte - 1) /
			DETAILED_TIMING_DESCRIPTION_SIZE;
	}

	ret = parse_extension_timing_description(dptx, dtd_block,
		dtd_start_byte, dtd_total);
	if (ret) {
		HISI_DRM_ERR("[DP] Parse the extension block timing information fail.\n");
		return ret;
	}
	/* Parse CEA Data Block Collection */
	if (cea_data_block_collection == 0x04) {
		cea_block = exten + cea_data_block_collection;
		ret = parse_cea_data_block(dptx, cea_block, dtd_start_byte);
		if (ret != 0) {
			HISI_DRM_ERR("[DP] CEA data block Parsing failed!\n");
			return ret;
		}
	}

	return 0;
}

static int check_main_edid(u8 *edid)
{
	u32 checksum = 0;
	s32 i;

	/* Verify 0 checksum */
	for (i = 0; i < EDID_LENGTH; i++)
		checksum += edid[i];
	if (checksum & 0xFF) {
		HISI_DRM_ERR("[DP] EDID checksum failed - data is corrupt. Continuing anyway.\n");
		return -EINVAL;
	}

	/* Verify Header */
	for (i = 0; i < EDID_HEADER_END + 1; i++) {
		if (edid[i] != edid_v1_header[i]) {
			HISI_DRM_INFO("[DP] first u8s don't match EDID version 1 header\n");
			return -EINVAL;
		}
	}

	return 0;
}

static int parse_descriptor_block(struct dp_ctrl *dptx, u8 *block)
{
	int ret;
	s16 i;

	/* EDID main part has a total of four Descriptor Block */
	for (i = 0; i < DETAILED_TIMING_DESCRIPTION_COUNT; i++,
			block += DETAILED_TIMING_DESCRIPTION_SIZE) {
		switch (block_type(block)) {
		case DETAILED_TIMING_BLOCK:
			ret = parse_timing_description(dptx, block);
			if (ret != 0) {
				HISI_DRM_INFO("[DP] Timing Description Parsing failed!\n");
				return ret;
			}
			break;
		case MONITOR_LIMITS:
			ret = parse_monitor_limits(dptx, block);
			if (ret != 0) {
				HISI_DRM_INFO("[DP] Parsing the monitor limit is failed!\n");
				return ret;
			}
			break;
		case MONITOR_NAME:
			ret = parse_monitor_name(dptx, block, DETAILED_TIMING_DESCRIPTION_SIZE);
			if (ret != 0) {
				HISI_DRM_ERR("[DP] The monitor name parsing failed.\n");
				return ret;
			}
			break;
		default:
			break;
		}
	}
	return 0;
}

static int parse_main(struct dp_ctrl *dptx)
{
	int ret;
	u8 *block = NULL;
	u8 *edid_t = NULL;
	struct edid_video *vid_info = NULL;

	dptx->edid_info.video.dptx_timing_list = NULL;
	dptx->edid_info.video.ext_timing = NULL;
	dptx->edid_info.video.dp_monitor_descriptor = NULL;
	dptx->edid_info.video.ext_v_count = 0;
	edid_t = dptx->edid;
	/* Initialize some fields */
	vid_info = &(dptx->edid_info.video);
	vid_info->main_v_count = 0;
	vid_info->max_pixel_clock = 0;

	if (check_main_edid(edid_t)) {
		HISI_DRM_ERR("[DP] The main edid block is wrong.\n");
		return -EINVAL;
	}

	/* Check EDID version (usually 1.3) */
	HISI_DRM_DEBUG("[DP] EDID version %d revision %d\n", (int)edid_t[EDID_STRUCT_VERSION],
										(int)edid_t[EDID_STRUCT_REVISION]);

	/* Check Display Image Size(Physical) */
	vid_info->max_h_image_size = (u16)edid_t[H_MAX_IMAGE_SIZE];
	vid_info->max_v_image_size = (u16)edid_t[V_MAX_IMAGE_SIZE];
	/* Alloc the name memory */
	vid_info->dp_monitor_descriptor =
		kzalloc((MONTIOR_NAME_DESCRIPTION_SIZE + 1) * sizeof(char),
		GFP_KERNEL);
	if (vid_info->dp_monitor_descriptor == NULL) {
		HISI_DRM_ERR("[DP] dp_monitor_descriptor memory alloc fail\n");
		return -EINVAL;
	}
	ret = memset_s(vid_info->dp_monitor_descriptor, MONTIOR_NAME_DESCRIPTION_SIZE + 1, 0, MONTIOR_NAME_DESCRIPTION_SIZE + 1);
	if (ret != EOK) {
		kfree(vid_info->dp_monitor_descriptor);
		vid_info->dp_monitor_descriptor = NULL;
		HISI_DRM_ERR("memset failed!");
		return -EINVAL;
	}
	/* Parse the EDID Detailed Timing Descriptor */
	block = edid_t + DETAILED_TIMING_DESCRIPTIONS_START;

	/* EDID main part has a total of four Descriptor Block */
	ret = parse_descriptor_block(dptx, block);
	if (ret) {
		kfree(vid_info->dp_monitor_descriptor);
		vid_info->dp_monitor_descriptor = NULL;
		return ret;
	}

	HISI_DRM_DEBUG("[DP] Extensions to follow:\t%d\n", (int)edid_t[EXTENSION_FLAG]);
	/* Return the number of following extension blocks */
	return (int)edid_t[EXTENSION_FLAG];
}

int parse_edid(struct dp_ctrl *dptx, u16 len)
{
	s16 i;
	s16 ext_block_num;
	int ret;
	u8 *edid_t = NULL;

	drm_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	edid_t = dptx->edid;
	HISI_DRM_DEBUG("parse edid +\n");
	/* Check if data has any error */
	drm_check_and_return((edid_t == NULL), -EINVAL, ERR, "[DP] Raw Data is invalid!(NULL error)!\n");

	if (((len / EDID_LENGTH) > 5) || ((len % EDID_LENGTH) != 0) || (len < EDID_LENGTH)) {
		HISI_DRM_ERR("[DP] Raw Data length is invalid! len = %d", len);
		return -EINVAL;
	}

	/* Parse the EDID main part, check how many(count as ' ext_block_num ') Extension blocks
	 * there are to follow.
	 */
	ext_block_num = parse_main(dptx);

	if ((ext_block_num > 0) && (len == ((ext_block_num + 1) * EDID_LENGTH))) {
		dptx->edid_info.audio.ext_a_count = 0;
		dptx->edid_info.audio.ext_speaker = 0;
		dptx->edid_info.audio.basic_audio = 0;
		dptx->edid_info.audio.spec = NULL;
		/* Parse all Extension blocks */
		for (i = 0; i < ext_block_num; i++) {
			ret = parse_extension(dptx, edid_t + (EDID_LENGTH * (i + 1)));
			if (ret != 0) {
				HISI_DRM_ERR(
					"[DP] Extension Parsing failed!Only returning the Main Part of this EDID!\n");
				break;
			}
		}
	} else if (ext_block_num < 0) {
		HISI_DRM_ERR("[DP] Error occured while parsing, returning with NULL!");
		return -EINVAL;
	}
	HISI_DRM_DEBUG("[DP] parse edid -\n");

#ifdef CONFIG_DP_EDID_DEBUG
	for (i = 0; i < len;) {
		if (!(i % 16))
			HISI_DRM_INFO("EDID [%04x]:%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x " \
						  "%02x %02x %02x %02x %02x %02x\n",
				i, dptx->edid[i], dptx->edid[i + 1], dptx->edid[i + 2], dptx->edid[i + 3],
				dptx->edid[i + 4], dptx->edid[i + 5], dptx->edid[i + 6], dptx->edid[i + 7],
				dptx->edid[i + 8], dptx->edid[i + 9], dptx->edid[i + 10], dptx->edid[i + 11],
				dptx->edid[i + 12], dptx->edid[i + 13], dptx->edid[i + 14], dptx->edid[i + 15]);

		i += 16;

		if (i == 128)
			HISI_DRM_INFO("<<<-------------------------------------------------------------->>>\n");
	}
#endif
	return 0;
}

int release_edid_info(struct dp_ctrl *dptx)
{
	struct timing_info *dptx_timing_node = NULL;
	struct timing_info *_node_ = NULL;
	int ret;

	drm_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	if (dptx->edid_info.video.dptx_timing_list != NULL) {
		list_for_each_entry_safe(dptx_timing_node, _node_, dptx->edid_info.video.dptx_timing_list, list_node) {
			list_del(&dptx_timing_node->list_node);
			kfree(dptx_timing_node);
		}

		kfree(dptx->edid_info.video.dptx_timing_list);
		dptx->edid_info.video.dptx_timing_list = NULL;
	}

	if (dptx->edid_info.video.ext_timing != NULL) {
		kfree(dptx->edid_info.video.ext_timing);
		dptx->edid_info.video.ext_timing = NULL;
	}

	if (dptx->edid_info.video.dp_monitor_descriptor != NULL) {
		kfree(dptx->edid_info.video.dp_monitor_descriptor);
		dptx->edid_info.video.dp_monitor_descriptor = NULL;
	}

	if (dptx->edid_info.audio.spec != NULL) {
		kfree(dptx->edid_info.audio.spec);
		dptx->edid_info.audio.spec = NULL;
	}

	ret = memset_s(&(dptx->edid_info), sizeof(struct edid_information), 0, sizeof(struct edid_information));
	drm_check_and_return(ret != EOK, -EINVAL, ERR, "memset failed!");
	return 0;
}

static void vary_vfp(struct dtd *dtd_by_code, u8 code, u32 refresh_rate)
{
	if (code == 8 || code == 9) {
		dtd_by_code->v_blanking = (refresh_rate == 59940) ? 22 : 23;
		dtd_by_code->v_sync_offset = (refresh_rate == 59940) ? 4 : 5;
	} else if (code == 12 || code == 13) {
		dtd_by_code->v_blanking = (refresh_rate == 60054) ? 22 : 23;
		dtd_by_code->v_sync_offset = (refresh_rate == 60054) ? 4 : 5;
	} else if (code == 23 || code == 24) {
		dtd_by_code->v_blanking = (refresh_rate == 50080) ?
			24 : ((refresh_rate == 49920) ? 25 : 26);
		dtd_by_code->v_sync_offset = (refresh_rate == 50080) ?
			2 : ((refresh_rate == 49920) ? 3 : 4);
	} else if (code == 27 || code == 28) {
		dtd_by_code->v_blanking = (refresh_rate == 50080) ?
			24 : ((refresh_rate == 49920) ? 25 : 26);
		dtd_by_code->v_sync_offset = (refresh_rate == 50080) ?
			2 : ((refresh_rate == 49920) ? 3 : 4);
	}
}

static bool code_to_dtd(struct dtd dtd_by_code, u8 code, u32 modes_dtd_size)
{
	if (code == 0 || code >= modes_dtd_size) {
		HISI_DRM_INFO("[DP] code error, code is %d, code_max is %d\n", code, modes_dtd_size);
		return false;
	}

	if (dtd_by_code.pixel_clock == 0) { // empty timing
		HISI_DRM_INFO("[DP] Empty Timing\n");
		return false;
	}

	return true;
}

static void set_mdtd(struct dtd *mdtd, struct dtd dtd_by_code)
{
	mdtd->pixel_repetition_input = dtd_by_code.pixel_repetition_input;
	mdtd->pixel_clock  = dtd_by_code.pixel_clock;
	mdtd->h_active = dtd_by_code.h_active;
	mdtd->h_blanking = dtd_by_code.h_blanking;
	mdtd->h_sync_offset = dtd_by_code.h_sync_offset;
	mdtd->h_sync_pulse_width = dtd_by_code.h_sync_pulse_width;
	mdtd->h_image_size = dtd_by_code.h_image_size;
	mdtd->v_active = dtd_by_code.v_active;
	mdtd->v_blanking = dtd_by_code.v_blanking;
	mdtd->v_sync_offset = dtd_by_code.v_sync_offset;
	mdtd->v_sync_pulse_width = dtd_by_code.v_sync_pulse_width;
	mdtd->v_image_size = dtd_by_code.v_image_size;
	mdtd->interlaced = dtd_by_code.interlaced;
	mdtd->v_sync_polarity = dtd_by_code.v_sync_polarity;
	mdtd->h_sync_polarity = dtd_by_code.h_sync_polarity;
}

bool convert_code_to_dtd(struct dtd *mdtd, u8 code, u32 refresh_rate, u8 video_format)
{
	struct dtd dtd_by_code;
	bool retval = false;

	drm_check_and_return((mdtd == NULL), false, ERR, "[DP] mdtd is NULL!\n");

	dwc_dptx_dtd_reset(mdtd);

	switch (video_format) {
	case VCEA:
		if (code == 0 || code >= dtd_array_size(cea_modes_dtd)) {
			HISI_DRM_INFO("[DP] code error, code is %u\n", code);
			return false;
		}
		dtd_by_code = cea_modes_dtd[code];
		if (dtd_by_code.pixel_clock == 0) { /* empty timing */
			HISI_DRM_INFO("[DP] Empty Timing\n");
			return false;
		}
		/*
		 * For certain VICs the spec allows the vertical
		 * front porch to vary by one or two lines.
		 */
		vary_vfp(&dtd_by_code, code, refresh_rate);
		break;

	case CVT:
		if (code == 0 || code >= dtd_array_size(cvt_modes_dtd)) {
			HISI_DRM_INFO("[DP] code error, code is %u\n", code);
			return false;
		}
		dtd_by_code = cvt_modes_dtd[code];
		retval = code_to_dtd(dtd_by_code, code, dtd_array_size(cvt_modes_dtd));
		if (dtd_by_code.pixel_clock == 0) { /* empty timing */
			HISI_DRM_INFO("[DP] Empty Timing\n");
			return false;
		}
		break;

	case DMT:
		if (code == 0 || code >= dtd_array_size(dmt_modes_dtd)) {
			HISI_DRM_INFO("[DP] code error, code is %u\n", code);
			return false;
		}
		dtd_by_code = dmt_modes_dtd[code];
		retval = code_to_dtd(dtd_by_code, code, dtd_array_size(dmt_modes_dtd));
		if (dtd_by_code.pixel_clock == 0) { /* empty timing */
			HISI_DRM_INFO("[DP] Empty Timing\n");
			return false;
		}
		break;

	default:
		HISI_DRM_INFO("[DP] Video Format is ERROR\n");
		return false;
	}

	set_mdtd(mdtd, dtd_by_code);
	return true;
}

#pragma GCC diagnostic pop
/*lint +e429 +e529*/
