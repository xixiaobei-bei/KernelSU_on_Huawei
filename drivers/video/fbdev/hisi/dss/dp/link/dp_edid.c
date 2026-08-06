/*
 * Copyright (c) 2016-2016, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#include "dp_edid.h"
#include "dp_dtd_helper.h"

static const uint8_t edid_v1_other_descriptor_flag[2] = {0x00, 0x00};

static struct list_head *dptx_hdmi_list;
struct dptx_hdmi_vic {
	struct list_head list_node;
	uint32_t vic_id;
};
static uint8_t g_hdmi_vic_len;
static uint8_t g_hdmi_vic_real_len;

int parse_edid(struct dp_ctrl *dptx, uint16_t len)
{
	int16_t i, ext_block_num;
	int ret;
	uint8_t *edid_t = NULL;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	edid_t = dptx->edid;
	DPU_FB_DEBUG("[DP] +\n");
	/* Check if data has any error */
	dpu_check_and_return((edid_t == NULL), -EINVAL, ERR, "[DP] Raw Data is invalid!(NULL error)!\n");

	if (((len / EDID_LENGTH) > 5) || ((len % EDID_LENGTH) != 0) || (len < EDID_LENGTH)) {
		DPU_FB_ERR("[DP] Raw Data length is invalid(not the size of (128 x N , N = [1-5]) uint8_t!");
		return -EINVAL;
	}

	/* Parse the EDID main part, check how many(count as ' ext_block_num ') Extension blocks there are to follow. */
	ext_block_num = parse_main(dptx);

	if ((ext_block_num > 0) && (len == ((ext_block_num + 1) * EDID_LENGTH))) {
		dptx->edid_info.Audio.extACount = 0;
		dptx->edid_info.Audio.extSpeaker = 0;
		dptx->edid_info.Audio.basicAudio = 0;
		dptx->edid_info.Audio.spec = NULL;
		/* Parse all Extension blocks */
		for (i = 0; i < ext_block_num; i++) {
			ret = parse_extension(dptx, edid_t + (EDID_LENGTH * (i + 1)));
			if (ret != 0) {
				DPU_FB_ERR(
					"[DP] Extension Parsing failed!Only returning the Main Part of this EDID!\n");
				break;
			}
		}
	} else if (ext_block_num < 0) {
		DPU_FB_ERR("[DP] Error occurred while parsing, returning with NULL!");
		return -EINVAL;
	}
	DPU_FB_DEBUG("[DP] -\n");

#ifdef CONFIG_DP_EDID_DEBUG
	for (i = 0; i < len;) {
		if (!(i % 16)) {
			printk(KERN_INFO "EDID [%04x]:  %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
				i, dptx->edid[i], dptx->edid[i + 1], dptx->edid[i + 2], dptx->edid[i + 3],
				dptx->edid[i + 4], dptx->edid[i + 5], dptx->edid[i + 6], dptx->edid[i + 7],
				dptx->edid[i + 8], dptx->edid[i + 9], dptx->edid[i + 10], dptx->edid[i + 11],
				dptx->edid[i + 12], dptx->edid[i + 13], dptx->edid[i + 14], dptx->edid[i + 15]);
		}

		i += 16;

		if (i == 128)
			printk(KERN_INFO "<<<-------------------------------------------------------------->>>\n");
	}
#endif
	return 0;
}

int check_main_edid(uint8_t *edid)
{
	uint32_t checksum = 0;
	int32_t i;

	dpu_check_and_return((edid == NULL), -EINVAL, ERR, "[DP] edid is NULL!\n");
	/* Verify 0 checksum */
	for (i = 0; i < EDID_LENGTH; i++)
		checksum += edid[i];
	if (checksum & 0xFF) {
		DPU_FB_ERR("[DP] EDID checksum failed - data is corrupt. Continuing anyway.\n");
		return -EINVAL;
	}

	/* Verify Header */
	for (i = 0; i < EDID_HEADER_END + 1; i++) {
		if (edid[i] != edid_v1_header[i]) {
			DPU_FB_INFO("[DP] first uint8_ts don't match EDID version 1 header\n");
			return -EINVAL;
		}
	}

	return 0;
}

static int check_factory_info(uint8_t *edid)
{
	uint8_t i;
	uint8_t data_tmp;

	data_tmp = 0;

	dpu_check_and_return((edid == NULL), -EINVAL, ERR, "[DP] edid is NULL!\n");

	/* For hiding custom device info */
	for (i = EDID_FACTORY_START; i <= EDID_FACTORY_END; i++)
		data_tmp += edid[i];
	memset(&edid[EDID_FACTORY_START], 0x0, (EDID_FACTORY_END - EDID_FACTORY_START + 1));
	edid[EDID_FACTORY_START] = data_tmp; // For EDID checksum, we need reserve the sum for blocks.

	return 0;
}

int parse_main(struct dp_ctrl *dptx)
{
	int16_t i;
	int ret;
	uint8_t *block = NULL;
	uint8_t *edid_t = NULL;
	struct edid_video *vid_info = NULL;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	dpu_check_and_return((dptx->edid == NULL), -EINVAL, ERR, "[DP] dptx->edid is NULL!\n");

	dptx->edid_info.Video.dptx_timing_list = NULL;
	dptx->edid_info.Video.extTiming = NULL;
	dptx->edid_info.Video.dp_monitor_descriptor = NULL;
	dptx->edid_info.Video.extVCount = 0;
	edid_t = dptx->edid;
	/* Initialize some fields */
	vid_info = &(dptx->edid_info.Video);
	vid_info->mainVCount = 0;
	vid_info->maxPixelClock = 0;

	if (check_main_edid(edid_t)) {
		DPU_FB_ERR("[DP] The main edid block is wrong.\n");
		return -EINVAL;
	}

	huawei_dp_save_edid(edid_t, DPTX_DEFAULT_EDID_BUFLEN);

	check_factory_info(edid_t);

	/* Check EDID version (usually 1.3) */
	DPU_FB_DEBUG("[DP] EDID version %d revision %d\n", (int)edid_t[EDID_STRUCT_VERSION],
		(int)edid_t[EDID_STRUCT_REVISION]);

	/* Check Display Image Size(Physical) */
	vid_info->maxHImageSize = (uint16_t)edid_t[H_MAX_IMAGE_SIZE];
	vid_info->maxVImageSize = (uint16_t)edid_t[V_MAX_IMAGE_SIZE];
	/* Alloc the name memory */
	if (vid_info->dp_monitor_descriptor == NULL) {
		vid_info->dp_monitor_descriptor =
			(char *)kzalloc(MONTIOR_NAME_DESCRIPTION_SIZE * sizeof(char), GFP_KERNEL);
		if (vid_info->dp_monitor_descriptor == NULL) {
			DPU_FB_ERR("[DP] dp_monitor_descriptor memory alloc fail\n");
			return -EINVAL;
		}
	}
	memset(vid_info->dp_monitor_descriptor, 0, MONTIOR_NAME_DESCRIPTION_SIZE);
	/* Parse the EDID Detailed Timing Descriptor */
	block = edid_t + DETAILED_TIMING_DESCRIPTIONS_START;
	/* EDID main part has a total of four Descriptor Block */
	for (i = 0; i < DETAILED_TIMING_DESCRIPTION_COUNT; i++, block += DETAILED_TIMING_DESCRIPTION_SIZE) {
		switch (block_type(block)) {
		case DETAILED_TIMING_BLOCK:
			ret = parse_timing_description(dptx, block);
			if (ret != 0) {
				DPU_FB_INFO("[DP] Timing Description Parsing failed!\n");
				return ret;
			}
			break;
		case MONITOR_LIMITS:
			ret = parse_monitor_limits(dptx, block);
			if (ret != 0) {
				DPU_FB_INFO("[DP] Parsing the monitor limit is failed!\n");
				return ret;
			}
			break;
		case MONITOR_NAME:
			ret = parse_monitor_name(dptx, block, DETAILED_TIMING_DESCRIPTION_SIZE);
			if (ret != 0) {
				DPU_FB_ERR("[DP] The monitor name parsing failed.\n");
				return ret;
			}
			break;
		default:
			break;
		}
	}

	DPU_FB_DEBUG("[DP] Extensions to follow:\t%d\n", (int)edid_t[EXTENSION_FLAG]);
	/* Return the number of following extension blocks */
	return (int)edid_t[EXTENSION_FLAG];
}

int check_exten_edid(uint8_t *exten)
{
	uint32_t i, checksum;

	checksum = 0;

	dpu_check_and_return((exten == NULL), -EINVAL, ERR, "[DP] exten is NULL!\n");

	for (i = 0; i < EDID_LENGTH; i++)
		checksum += exten[i];
	if (checksum & 0xFF) {
		DPU_FB_ERR("[DP] Extension Data checksum failed - data is corrupt. Continuing anyway.\n");
		return -EINVAL;
	}
	/* Check Extension Tag */
	/* ( Header Tag stored in the first uint8_t ) */
	if (exten[0] != EXTENSION_HEADER) {
		DPU_FB_ERR("[DP] Not CEA-EDID Timing Extension, Extension-Parsing will not continue!\n");
		return -EINVAL;
	}
	return 0;
}

int parse_extension_timing_description(struct dp_ctrl *dptx, uint8_t *dtd_block, uint32_t dtd_begin, uint16_t dtd_total)
{
	uint32_t i;
	int ret;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	dpu_check_and_return((dtd_block == NULL), -EINVAL, ERR, "[DP] dtd_block is NULL!\n");

	if ((dtd_begin + 1 + dtd_total * DETAILED_TIMING_DESCRIPTION_SIZE) > EDID_LENGTH) {
		DPU_FB_ERR("[DP] The dtd total number 0x%x is out of the limit\n", dtd_total);
		return -EINVAL;
	}

	for (i = 0; i < dtd_total; i++, dtd_block += DETAILED_TIMING_DESCRIPTION_SIZE) {
		switch (block_type(dtd_block)) {
		case DETAILED_TIMING_BLOCK:
			ret = parse_timing_description(dptx, dtd_block);
			if (ret != 0) {
				DPU_FB_ERR("[DP] Timing Description Parsing failed!");
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

int parse_extension(struct dp_ctrl *dptx, uint8_t *exten)
{
	int ret;
	uint8_t *dtd_block = NULL;
	uint8_t *cea_block = NULL;
	uint8_t dtd_start_byte = 0;
	uint8_t cea_data_block_collection = 0;
	uint16_t dtd_total = 0;
	struct edid_video *vid_info = NULL;
	struct edid_audio *aud_info = NULL;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	dpu_check_and_return((exten == NULL), -EINVAL, ERR, "[DP] exten is NULL!\n");

	vid_info = &(dptx->edid_info.Video);
	aud_info = &(dptx->edid_info.Audio);

	ret = check_exten_edid(exten);
	if (ret) {
		DPU_FB_ERR("[DP] The check_exten_edid failed.\n");
		return ret;
	}
	/*
	 * Get uint8_t number (decimal) within this block where the 18-uint8_t DTDs begin.
	 * ( Number data stored in the third uint8_t )
	 */

	if (exten[2] == 0x00) {
		DPU_FB_INFO("[DP] There are no DTDs present in this extension block and no non-DTD data.\n");
		return -EINVAL;
	} else if (exten[2] == 0x04) {
		dtd_start_byte = 0x04;
	} else {
		cea_data_block_collection = 0x04;
		dtd_start_byte = exten[2];
	}
	/*
	 * Get Number of Native DTDs present, other Version 2+ information
	 * DTD Total stored in the fourth uint8_t )
	 */
	if (aud_info->basicAudio != 1)
		aud_info->basicAudio = (0x40 & exten[3]) >> 6;
	huawei_dp_imonitor_set_param(DP_PARAM_BASIC_AUDIO, &(aud_info->basicAudio));

	dtd_total = LOWER_NIBBLE(exten[3]);
	// Parse DTD in Extension
	dtd_block = exten + dtd_start_byte;
	if (dtd_total != (EDID_LENGTH - dtd_start_byte - 1) / DETAILED_TIMING_DESCRIPTION_SIZE) {
		DPU_FB_INFO("[DP] The number of native DTDs is not equal the size\n");
		dtd_total = (EDID_LENGTH - dtd_start_byte - 1) / DETAILED_TIMING_DESCRIPTION_SIZE;
	}

	ret = parse_extension_timing_description(dptx, dtd_block, dtd_start_byte, dtd_total);
	if (ret) {
		DPU_FB_ERR("[DP] Parse the extension block timing information fail.\n");
		return ret;
	}
	/* Parse CEA Data Block Collection */
	if (cea_data_block_collection == 0x04) {
		cea_block = exten + cea_data_block_collection;
		ret = parse_cea_data_block(dptx, cea_block, dtd_start_byte);
		if (ret != 0) {
			DPU_FB_ERR("[DP] CEA data block Parsing failed!\n");
			return ret;
		}
	}

	return 0;
}
/*lint -e429*/
int parse_timing_description(struct dp_ctrl *dptx, uint8_t *dtd)
{
	int retval;
	uint32_t fps = 0;
	struct edid_video *vid_info = NULL;
	struct timing_info *node = NULL;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	dpu_check_and_return((dtd == NULL), -EINVAL, ERR, "[DP] dtd is NULL!\n");

	vid_info = &(dptx->edid_info.Video);

	if (vid_info->dptx_timing_list == NULL) {
		vid_info->dptx_timing_list = kzalloc(sizeof(struct list_head), GFP_KERNEL);
		if (vid_info->dptx_timing_list == NULL) {
			DPU_FB_ERR("[DP] vid_info->dptx_timing_list is NULL");
			return -EINVAL;
		}
		INIT_LIST_HEAD(vid_info->dptx_timing_list);

		vid_info->mainVCount = 0;
		/* Initial Max Value */
		vid_info->maxHPixels = H_ACTIVE;
		vid_info->maxVPixels = V_ACTIVE;
		vid_info->maxPixelClock = PIXEL_CLOCK;
	}

	/* Get Max Value by comparing all values */
	if ((vid_info->maxHPixels < H_ACTIVE) && (vid_info->maxVPixels <= V_ACTIVE) &&
		(vid_info->maxPixelClock < PIXEL_CLOCK)) {
		vid_info->maxHPixels = H_ACTIVE;
		vid_info->maxVPixels = V_ACTIVE;
		vid_info->maxPixelClock = PIXEL_CLOCK;
	}
	// node
	node = kzalloc(sizeof(struct timing_info), GFP_KERNEL);
	if (node != NULL) {
		node->hActivePixels = H_ACTIVE;
		node->hBlanking = H_BLANKING;
		node->hSyncOffset = H_SYNC_OFFSET;
		node->hSyncPulseWidth = H_SYNC_WIDTH;
		node->hBorder = H_BORDER;
		node->hSize = H_SIZE;

		node->vActivePixels = V_ACTIVE;
		node->vBlanking = V_BLANKING;
		node->vSyncOffset = V_SYNC_OFFSET;
		node->vSyncPulseWidth = V_SYNC_WIDTH;
		node->vBorder = V_BORDER;
		node->vSize = V_SIZE;

		node->pixelClock = PIXEL_CLOCK;

		node->inputType = INPUT_TYPE; // need to modify later
		node->interlaced = INTERLACED;
		node->vSyncPolarity = V_SYNC_POLARITY;
		node->hSyncPolarity = H_SYNC_POLARITY;
		node->syncScheme = SYNC_SCHEME;
		node->schemeDetail = SCHEME_DETAIL;

		vid_info->mainVCount += 1;
		list_add_tail(&node->list_node, vid_info->dptx_timing_list);
	} else {
		DPU_FB_ERR("[DP] kzalloc struct dptx_hdmi_vic fail!\n");
		return -EINVAL;
	}

	retval = dptx_calc_fps(node, &fps);
	if (retval)
		DPU_FB_INFO("[DP] fps calc error");

	DPU_FB_INFO("[DP] The timinginfo %d: hActivePixels is %d, vActivePixels is %d, pixel clock = %llu, fps = %d\n",
		vid_info->mainVCount, node->hActivePixels, node->vActivePixels, node->pixelClock, fps);
	huawei_dp_imonitor_set_param_timing(node->hActivePixels, node->vActivePixels, node->pixelClock, 0);

	return 0;
}

int parse_timing_description_by_vesaid(struct edid_video *vid_info, uint8_t vesa_id)
{
	int retval;
	uint32_t fps = 0;
	struct dtd mdtd;
	struct timing_info *node = NULL;

	dpu_check_and_return((vid_info == NULL), -EINVAL, ERR, "[DP] vid_info is NULL!\n");

	if (!convert_code_to_dtd(&mdtd, vesa_id, 60000, VCEA)) {
		DPU_FB_INFO("[DP] Invalid video mode value %d\n",
						vesa_id);
		return -EINVAL;
	}

	if (mdtd.interlaced) {
		DPU_FB_INFO("[DP] Don't Support interlace mode %d\n",
						vesa_id);
		return -EINVAL;
	}

	if (vid_info->dptx_timing_list == NULL) {
		vid_info->dptx_timing_list = kzalloc(sizeof(struct list_head), GFP_KERNEL);
		if (vid_info->dptx_timing_list == NULL) {
			DPU_FB_ERR("[DP] vid_info->dptx_timing_list is NULL");
			return -EINVAL;
		}
		INIT_LIST_HEAD(vid_info->dptx_timing_list);

		vid_info->mainVCount = 0;
		/* Initial Max Value */
		vid_info->maxHPixels = mdtd.h_active;
		vid_info->maxVPixels = mdtd.v_active;
		vid_info->maxPixelClock = mdtd.pixel_clock / 10;
	}

	/* Get Max Value by comparing all values */
	if ((mdtd.h_active > vid_info->maxHPixels) && (mdtd.v_active >= vid_info->maxVPixels) &&
		((mdtd.pixel_clock / 10) > vid_info->maxPixelClock)) {
		vid_info->maxHPixels = mdtd.h_active;
		vid_info->maxVPixels = mdtd.v_active;
		vid_info->maxPixelClock = mdtd.pixel_clock / 10;
	}

	// node
	node = kzalloc(sizeof(struct timing_info), GFP_KERNEL);
	if (node != NULL) {
		node->hActivePixels = mdtd.h_active;
		node->hBlanking = mdtd.h_blanking;
		node->hSyncOffset = mdtd.h_sync_offset;
		node->hSyncPulseWidth = mdtd.h_sync_pulse_width;
		node->hSize = mdtd.h_image_size;

		node->vActivePixels = mdtd.v_active;
		node->vBlanking = mdtd.v_blanking;
		node->vSyncOffset = mdtd.v_sync_offset;
		node->vSyncPulseWidth = mdtd.v_sync_pulse_width;
		node->vSize = mdtd.v_image_size;

		node->pixelClock = mdtd.pixel_clock / 10;  // Edid pixel clock is 1/10 of dtd filled timing.
		node->interlaced = mdtd.interlaced;
		node->vSyncPolarity = mdtd.v_sync_polarity;
		node->hSyncPolarity = mdtd.h_sync_polarity;

		vid_info->mainVCount += 1;
		list_add_tail(&node->list_node, vid_info->dptx_timing_list);
	} else {
		DPU_FB_ERR("[DP] kzalloc struct dptx_hdmi_vic fail!\n");
		return -EINVAL;
	}

	retval = dptx_calc_fps(node, &fps);
	if (retval)
		DPU_FB_INFO("[DP] fps calc error");

	DPU_FB_INFO("[DP] The timinginfo %d: hActivePixels is %d, vActivePixels is %d, pixel clock = %llu, fps = %d\n",
		vid_info->mainVCount, node->hActivePixels, node->vActivePixels, node->pixelClock, fps);
	huawei_dp_imonitor_set_param_timing(node->hActivePixels, node->vActivePixels, node->pixelClock, vesa_id);

	return 0;
}

int parse_hdmi_vic_id(uint8_t vic_id)
{
	struct dptx_hdmi_vic *node = NULL;

	if (g_hdmi_vic_real_len >= g_hdmi_vic_len) {
		DPU_FB_ERR("[DP] The g_hdmi_vic_real_len is more than g_hdmi_vic_len.\n");
		return -EINVAL;
	}

	if (dptx_hdmi_list == NULL) {
		dptx_hdmi_list = kzalloc(sizeof(struct list_head), GFP_KERNEL);
		if (dptx_hdmi_list == NULL) {
			DPU_FB_ERR("[DP] dptx_hdmi_list is NULL");
			return -EINVAL;
		}
		INIT_LIST_HEAD(dptx_hdmi_list);
	}

	// node
	node = kzalloc(sizeof(struct dptx_hdmi_vic), GFP_KERNEL);
	if (node != NULL) {
		node->vic_id = vic_id;
		DPU_FB_INFO("[DP] vic_id = %d!\n", vic_id);
		list_add_tail(&node->list_node, dptx_hdmi_list);
		g_hdmi_vic_real_len++;
	} else {
		DPU_FB_ERR("[DP] kzalloc struct dptx_hdmi_vic fail!\n");
	}

	return 0;
}
/*lint +e429*/
int parse_audio_spec_info(struct edid_audio *aud_info, struct edid_audio_info *spec_info, uint8_t *cea_data_block)
{
	if ((EXTEN_AUDIO_FORMAT <= 8) && (EXTEN_AUDIO_FORMAT >= 1)) {
		dpu_check_and_return((!aud_info || !spec_info), -EINVAL, ERR, "[DP] aud_info or spec_info is NULL!\n");
		/* Set up SAD fields */
		spec_info->format = EXTEN_AUDIO_FORMAT;
		spec_info->channels =  EXTEN_AUDIO_MAX_CHANNELS;
		spec_info->sampling = EXTEN_AUDIO_SAMPLING;
		if (EXTEN_AUDIO_FORMAT == 1)
			spec_info->bitrate = EXTEN_AUDIO_LPCM_BIT;
		else
			spec_info->bitrate = EXTEN_AUDIO_BITRATE;
		aud_info->extACount += 1;

		DPU_FB_INFO(
			"[DP] parse audio spec success. format(0x%x), channels(0x%x), sampling(0x%x), bitrate(0x%x)\n",
			spec_info->format, spec_info->channels, spec_info->sampling, spec_info->bitrate);
	}

	return 0;
}

int parse_extension_audio_tag(struct edid_audio *aud_info, uint8_t *cea_data_block, uint8_t temp_length)
{
	uint8_t i, xa;
	void *temp_ptr = NULL;

	dpu_check_and_return((aud_info == NULL), -EINVAL, ERR, "[DP] aud_info is NULL!\n");
	dpu_check_and_return((cea_data_block == NULL), -EINVAL, ERR, "[DP] cea_data_block is NULL!\n");
	dpu_check_and_return((temp_length < 1), -EINVAL, ERR, "[DP] The input param temp_length is wrong!\n");

	for (i = 0; i < (temp_length) / 3; i++) {
		xa = aud_info->extACount;

		if (xa == 0) {
			/* Initial audio part */
			if (aud_info->spec != NULL) {
				DPU_FB_ERR("[DP] The spec of audio is error.\n");
				return -EINVAL;
			}
			aud_info->spec = kzalloc(sizeof(struct edid_audio_info), GFP_KERNEL);
			if (aud_info->spec == NULL) {
				DPU_FB_ERR("[DP] malloc Audio Spec part failed!\n");
				return -EINVAL;
			}
		} else {
			/* Add memory as needed with error handling */
			temp_ptr = kzalloc((xa + 1) * sizeof(struct edid_audio_info), GFP_KERNEL);
			if (temp_ptr == NULL) {
				DPU_FB_ERR("[DP] Realloc Audio Spec part failed!\n");
				return -EINVAL;
			}
			if (aud_info->spec == NULL) {
				DPU_FB_ERR("[DP] The spec is NULL.\n");
				kfree(temp_ptr);
				return -EINVAL;
			}
			memcpy(temp_ptr, aud_info->spec, xa * sizeof(struct edid_audio_info));
			kfree(aud_info->spec);
			aud_info->spec = NULL;
			aud_info->spec = temp_ptr;
		}
		if (parse_audio_spec_info(aud_info, &(aud_info->spec[xa]), cea_data_block)) {
			DPU_FB_ERR("[DP] parse the audio spec info fail.\n");
			return -EINVAL;
		}

		cea_data_block += 3;
	}
	return 0;
}

int parse_extension_video_tag(struct edid_video *vid_info, uint8_t *cea_data_block, uint8_t length)
{
	uint8_t i;

	dpu_check_and_return((vid_info == NULL), -EINVAL, ERR, "[DP] vid_info is NULL!\n");
	dpu_check_and_return((cea_data_block == NULL), -EINVAL, ERR, "[DP] cea_data_block is NULL!\n");
	dpu_check_and_return((length < 1), -EINVAL, ERR, "[DP] The input param length is wrong!\n");

	vid_info->extTiming = kzalloc(length * sizeof(struct ext_timing), GFP_KERNEL);
	if (vid_info->extTiming == NULL) {
		DPU_FB_ERR("[DP] extTiming memory alloc fail\n");
		return -EINVAL;
	}
	memset(vid_info->extTiming, 0x0, length * sizeof(struct ext_timing));
	vid_info->extVCount = 0;

	for (i = 0; i < length; i++) {
		if (EXTEN_VIDEO_CODE != 0) {
			/* Set up SVD fields */
			vid_info->extTiming[i].extFormatCode = EXTEN_VIDEO_CODE;
			vid_info->extVCount += 1;

			parse_timing_description_by_vesaid(vid_info, vid_info->extTiming[i].extFormatCode);
		}
		cea_data_block += 1;
	}
	return 0;
}

static void parse_extension_vsdb_after_latency(uint8_t *cea_data_block, uint8_t length,
	uint8_t pos_after_latency)
{
	uint8_t i;
	bool b3dpresent = false;

	dpu_check_and_no_retval((cea_data_block == NULL), ERR, "[DP] cea_data_block is NULL!\n");

	for (i = pos_after_latency; i < length; i++) {
		if (i == pos_after_latency) {
			b3dpresent = (cea_data_block[i] & 0x80) >> 7;
		} else if (i == pos_after_latency + 1) {
			g_hdmi_vic_len = (cea_data_block[i] & 0xE0) >> 5;
			if (g_hdmi_vic_len == 0) {
				DPU_FB_INFO("[DP] This EDID don't include HDMI additional video format (2).\n");
				return;
			}
			g_hdmi_vic_real_len = 0;
		} else if (i <= pos_after_latency + 1 + g_hdmi_vic_len) {
			parse_hdmi_vic_id(cea_data_block[i]);
		} else {
			return;
		}
	}
}

static void parse_hdmi_vsdb_timing_list(struct edid_video *vid_info)
{

	dpu_check_and_no_retval((vid_info == NULL), ERR, "[DP] vid_info is NULL!\n");
	dpu_check_and_no_retval((dptx_hdmi_list == NULL), ERR, "[DP] dptx_hdmi_list is NULL!\n");

	kfree(dptx_hdmi_list);
	dptx_hdmi_list = NULL;
}

int parse_extension_vsdb_tag(struct edid_video *vid_info, uint8_t *cea_data_block, uint8_t length)
{
	uint8_t i;
	uint32_t ieee_flag;
	uint32_t hdmi_cec_port;
	uint8_t max_tmds_clock;
	uint8_t latency_fields;
	uint8_t interlaced_latency_fields;
	uint8_t hdmi_video_present;
	uint8_t vesa_id;
	bool support_ai = false;

	vesa_id = 0;
	g_hdmi_vic_real_len = 0;
	g_hdmi_vic_len = 0;
	latency_fields = 0;
	interlaced_latency_fields = 0;

	dpu_check_and_return((vid_info == NULL), -EINVAL, ERR, "[DP] vid_info is NULL!\n");
	dpu_check_and_return((cea_data_block == NULL), -EINVAL, ERR, "[DP] cea_data_block is NULL!\n");
	dpu_check_and_return((length < 5), -EINVAL, ERR, "[DP] VSDB length isn't correct!\n");

	ieee_flag = 0;
	ieee_flag = (cea_data_block[0]) | (cea_data_block[1] << 8) | (cea_data_block[2] << 16);

	if (ieee_flag != 0x000c03) {
		DPU_FB_INFO("[DP] This block isn't belong to HDMI block: %x.\n", ieee_flag);
		return 0;
	}

	hdmi_cec_port = ((cea_data_block[3] << 8) | (cea_data_block[4]));

	for (i = 5; i < MIN(length, 8); i++) {
		switch (i) {
		case 5:
			support_ai = (cea_data_block[i] & 0x80) >> 7;
			break;
		case 6:
			max_tmds_clock = cea_data_block[i];
			break;
		case 7:
			latency_fields = (cea_data_block[i] & 0x80) >> 7;
			interlaced_latency_fields = (cea_data_block[i] & 0x40) >> 6;
			hdmi_video_present = (cea_data_block[i] & 0x20) >> 5;
			if (hdmi_video_present == 0) {
				DPU_FB_INFO("[DP] This EDID don't include HDMI additional video format (1).\n");
				return 0;
			}
			break;
		default:
			break;
		}
	}

	if (2 * LATENCY_PRESENT + 8 < length)
		parse_extension_vsdb_after_latency(cea_data_block, length, 2 * LATENCY_PRESENT + 8);

	if (g_hdmi_vic_len == 0)
		return 0;
	DPU_FB_INFO("[DP] vic_id real length  = %d , vic length = %d !\n", g_hdmi_vic_real_len, g_hdmi_vic_len);

	parse_hdmi_vsdb_timing_list(vid_info);

	return 0;
}

int parse_cea_data_block(struct dp_ctrl *dptx, uint8_t *cea_data, uint8_t dtd_start)
{
	uint8_t total_length, block_length;
	uint8_t *cea_data_block = cea_data;
	struct edid_video *vid_info = NULL;
	struct edid_audio *aud_info = NULL;
	/* exTlist *extlist; */
	/* Initialize some fields */

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	dpu_check_and_return((cea_data == NULL), -EINVAL, ERR, "[DP] cea_data is NULL!\n");
	dpu_check_and_return((dtd_start > (EDID_LENGTH - 1)), -EINVAL, ERR,
		"[DP] The start of dtd is out of limit!\n");

	total_length = 4;
	vid_info = &(dptx->edid_info.Video);
	aud_info = &(dptx->edid_info.Audio);
	/* Parse CEA Data Block Collection */
	while (total_length < dtd_start) {
		/* Get length(total number of following uint8_ts of a certain tag) */
		block_length = GET_CEA_DATA_BLOCK_LEN(cea_data_block);
		/* Get tag code */
		switch (GET_CEA_DATA_BLOCK_TAG(cea_data_block)) {
		case EXTENSION_AUDIO_TAG:
			/* Block type tag combined with data length takes 1 uint8_t*/
			cea_data_block += 1;
			/* Each Short Audio Descriptor(SAD) takes 3 uint8_ts*/
			if (parse_extension_audio_tag(aud_info, cea_data_block, block_length)) {
				DPU_FB_ERR("[DP] parse_extension_audio_tag fail.\n");
				return -EINVAL;
			}
			cea_data_block += block_length;
			break;
		case EXTENSION_VIDEO_TAG:
			/* Block type tag combined with data length takes 1 uint8_t*/
			cea_data_block += 1;
			/* Each Short Video Descriptor(SVD) takes 1 uint8_t*/
			if (parse_extension_video_tag(vid_info, cea_data_block, block_length)) {
				DPU_FB_ERR("[DP] parse_extension_video_tag fail.\n");
				return -EINVAL;
			}
			cea_data_block += block_length;
			break;
		case EXTENSION_VENDOR_TAG:
			cea_data_block += 1;
			if (parse_extension_vsdb_tag(vid_info, cea_data_block, block_length)) {
				DPU_FB_ERR("[DP] parse_extension_vsdb_tag fail.\n");
				return -EINVAL;
			}
			cea_data_block += block_length;
			break;
		case EXTENSION_SPEAKER_TAG:
			/*
			 * If the extension has Speaker Allocation Data,
			 * there should only be one
			 */
			cea_data_block += 1;
			aud_info->extSpeaker = EXTEN_SPEAKER;
			/* Speaker Allocation Data takes 3 uint8_ts */
			cea_data_block += 3;
			break;
		default:
			cea_data_block += block_length + 1;
			break;
		}
		total_length = total_length + block_length + 1;
	}

	return 0;
}

int block_type(uint8_t *block)
{
	dpu_check_and_return((block == NULL), -EINVAL, ERR, "[DP] block is NULL!\n");

	if ((block[0] == 0) && (block[1] == 0)) {
		/* Other descriptor */
		if ((block[2] != 0) || (block[4] != 0))
			return UNKNOWN_DESCRIPTOR;
		return block[3];
	}
	/* Detailed timing block */
	return DETAILED_TIMING_BLOCK;
}

int parse_monitor_limits(struct dp_ctrl *dptx, uint8_t *block)
{
	struct edid_video *vid_info = NULL;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	dpu_check_and_return((block == NULL), -EINVAL, ERR, "[DP] block is NULL!\n");

	vid_info = &(dptx->edid_info.Video);
	/* Set up limit fields */
	vid_info->maxHFreq = H_MAX_RATE;
	vid_info->minHFreq = H_MIN_RATE;
	vid_info->maxVFreq = V_MAX_RATE;
	vid_info->minVFreq = V_MIN_RATE;
	vid_info->maxPixelClock = (int)MAX_PIXEL_CLOCK;

	return 0;
}

/*
 * EDID Display Descriptors[7]
 * Bytes	Description
 * 0-1	0, indicates Display Descriptor (cf. Detailed Timing Descriptor).
 * 2	0, reserved
 * 3	Descriptor type. FA-FF currently defined. 00-0F reserved for vendors.
 * 4	0, reserved, except for Display Range Limits Descriptor.
 * 5-17	Defined by descriptor type. If text, code page 437 text,
 * terminated (if less than 13 bytes) with LF and padded with SP.
 */
int parse_monitor_name(struct dp_ctrl *dptx, uint8_t *blockname, uint32_t size)
{
	uint32_t i;
	uint32_t str_start = 5;
	uint32_t str_end = 0;
	uint8_t data_tmp = 0;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	dpu_check_and_return((blockname == NULL), -EINVAL, ERR, "[DP] blockname is NULL!\n");
	dpu_check_and_return((size != DETAILED_TIMING_DESCRIPTION_SIZE), -EINVAL, ERR,
		"[DP] The length of monitor name is wrong!\n");

	for (i = str_start; i < (size - 1); i++) {
		if ((blockname[i] == 0x0A) && (blockname[i + 1] == 0x20)) {
			str_end = i;
			break;
		}
	}

	if (str_end == 0)
		str_end = size;

	if (str_end < str_start) {
		DPU_FB_ERR("[DP] The length of monitor name is wrong\n");
		return -EINVAL;
	}

	if (((str_end - str_start) < MONTIOR_NAME_DESCRIPTION_SIZE) &&
		(dptx->edid_info.Video.dp_monitor_descriptor != NULL)) {
		memcpy(dptx->edid_info.Video.dp_monitor_descriptor, &(blockname[str_start]), (str_end - str_start));
		dptx->edid_info.Video.dp_monitor_descriptor[str_end - str_start] = '\0';

		/* For hiding custom device info */
		for (i = str_start; i < str_end; i++)
			data_tmp += blockname[i];

		memset(&blockname[str_start], 0x0, (str_end - str_start));
		/* For EDID checksum, we need reserve the sum for blocks. */
		blockname[str_start] = data_tmp;
	} else {
		DPU_FB_ERR("[DP] The length of monitor name is wrong\n");
		return -EINVAL;
	}

	return 0;
}

int release_edid_info(struct dp_ctrl *dptx)
{
	struct timing_info *dptx_timing_node = NULL;
	struct timing_info *_node_ = NULL;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	if (dptx->edid_info.Video.dptx_timing_list != NULL) {
		list_for_each_entry_safe(dptx_timing_node, _node_, dptx->edid_info.Video.dptx_timing_list, list_node) {
			list_del(&dptx_timing_node->list_node);
			kfree(dptx_timing_node);
		}

		kfree(dptx->edid_info.Video.dptx_timing_list);
		dptx->edid_info.Video.dptx_timing_list = NULL;
	}

	if (dptx->edid_info.Video.extTiming != NULL) {
		kfree(dptx->edid_info.Video.extTiming);
		dptx->edid_info.Video.extTiming = NULL;
	}

	if (dptx->edid_info.Video.dp_monitor_descriptor != NULL) {
		kfree(dptx->edid_info.Video.dp_monitor_descriptor);
		dptx->edid_info.Video.dp_monitor_descriptor = NULL;
	}

	if (dptx->edid_info.Audio.spec != NULL) {
		kfree(dptx->edid_info.Audio.spec);
		dptx->edid_info.Audio.spec = NULL;
	}

	memset(&(dptx->edid_info), 0, sizeof(struct edid_information));

	return 0;
}

static void vary_vfp(struct dtd *dtd_by_code, uint8_t code, uint32_t refresh_rate)
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

bool convert_code_to_dtd(struct dtd *mdtd, uint8_t code, uint32_t refresh_rate, uint8_t video_format)
{
	struct dtd dtd_by_code;

	dpu_check_and_return((mdtd == NULL), false, ERR, "[DP] mdtd is NULL!\n");

	dwc_dptx_dtd_reset(mdtd);

	switch (video_format) {
	case VCEA:
		if (code == 0 || code >= dtd_array_size(cea_modes_dtd)) {
			DPU_FB_INFO("[DP] code error, code is %u\n", code);
			return false;
		}
		dtd_by_code = cea_modes_dtd[code];
		if (dtd_by_code.pixel_clock == 0) { /* empty timing */
			DPU_FB_INFO("[DP] Empty Timing\n");
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
			DPU_FB_INFO("[DP] code error, code is %u\n", code);
			return false;
		}
		dtd_by_code = cvt_modes_dtd[code];
		if (dtd_by_code.pixel_clock == 0) { /* empty timing */
			DPU_FB_INFO("[DP] Empty Timing\n");
			return false;
		}
		break;

	case DMT:
		if (code == 0 || code >= dtd_array_size(dmt_modes_dtd)) {
			DPU_FB_INFO("[DP] code error, code is %u\n", code);
			return false;
		}
		dtd_by_code = dmt_modes_dtd[code];
		if (dtd_by_code.pixel_clock == 0) { /* empty timing */
			DPU_FB_INFO("[DP] Empty Timing\n");
			return false;
		}
		break;

	default:
		DPU_FB_INFO("[DP] Video Format is ERROR\n");
		return false;
	}

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

	return true;
}
