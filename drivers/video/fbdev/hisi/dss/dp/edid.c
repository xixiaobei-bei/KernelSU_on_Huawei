/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/

#include "edid.h"
#include "avgen.h"

static const uint8_t edid_v1_other_descriptor_flag[2] = {0x00, 0x00};

static struct list_head *dptx_hdmi_list = NULL;
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
	uint8_t* edid_t;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] dptx is NULL pointer\n");
		return -EINVAL;
	}

	edid_t = dptx->edid;
	HISI_FB_DEBUG("parse_edid +\n");
	/*Check if data has any error*/
	if (edid_t == NULL) {
		HISI_FB_ERR("[DP] Raw Data is invalid!(NULL error)\n");
		return -EINVAL;
	}

	if (((len/EDID_LENGTH) > 5) || ((len%EDID_LENGTH) != 0) || (len < EDID_LENGTH)) {
		HISI_FB_ERR("[DP] Raw Data length is invalid(not the size of (128 x N , N = [1-5]) uint8_t!");
		return -EINVAL;
	}

	/*Parse the EDID main part, check how many(count as ' ext_block_num ') Extension blocks there are to follow.*/
	ext_block_num = parse_main(dptx);

	if (ext_block_num > 0) {
		dptx->edid_info.Audio.extACount = 0;
		dptx->edid_info.Audio.extSpeaker = 0;
		dptx->edid_info.Audio.basicAudio = 0;
		dptx->edid_info.Audio.spec = NULL;
		/*Parse all Extension blocks */
		for (i = 0; i < ext_block_num; i++) {
			ret = parse_extension(dptx, edid_t + (EDID_LENGTH * (i + 1)));
			if ( ret != 0 ) {
				HISI_FB_ERR("[DP] Extension Parsing failed!Only returning the Main Part of this EDID!\n");
				break;
			}
		}
	} else if (ext_block_num < 0) {
		HISI_FB_ERR("[DP] Error occured while parsing, returning with NULL!");
		return -EINVAL;
	}
	HISI_FB_DEBUG("[DP] parse_edid -\n");

	for (i = 0; i < len;) {
		if (!(i % 16)) {
			printk(KERN_INFO "EDID [%04x]:  %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n",
				i, dptx->edid[i], dptx->edid[i + 1], dptx->edid[i + 2], dptx->edid[i + 3],
				dptx->edid[i + 4], dptx->edid[i + 5], dptx->edid[i + 6], dptx->edid[i + 7],
				dptx->edid[i + 8], dptx->edid[i + 9], dptx->edid[i + 10], dptx->edid[i + 11],
				dptx->edid[i + 12], dptx->edid[i + 13], dptx->edid[i + 14], dptx->edid[i + 15]);
		}

		i += 16;

		if (i == 128) {
			printk(KERN_INFO "<<<-------------------------------------------------------------->>> \n");
		}
	}
	return 0;
}

int check_main_edid(uint8_t* edid)
{
	uint32_t checksum = 0;
	int32_t i;
	if(edid == NULL) {
		HISI_FB_ERR("[DP] The pointer is NULL.\n");
		return -EINVAL;
	}
	/*Verify 0 checksum*/
	for (i = 0; i < EDID_LENGTH; i++)
		checksum += edid[i];
	if (checksum & 0xFF) {
		HISI_FB_ERR("[DP] EDID checksum failed - data is corrupt. Continuing anyway.\n");
		return -EINVAL;
	}

	/*Verify Header*/
	for(i = 0; i < EDID_HEADER_END + 1; i++)
	{
		if(edid[i] != edid_v1_header[i]) {
			HISI_FB_INFO("[DP] first uint8_ts don't match EDID version 1 header\n");
			return -EINVAL;
		}
	}

	return 0;
}

static int check_factory_info(uint8_t* edid)
{
	uint8_t i;
	uint8_t data_tmp;

	data_tmp = 0;

	if(edid == NULL) {
		HISI_FB_ERR("[DP] The pointer is NULL.\n");
		return -EINVAL;
	}

	/*For hiding custom device info*/
	for (i = EDID_FACTORY_START; i <= EDID_FACTORY_END; i++) {
		data_tmp += edid[i];
	}
	memset(&edid[EDID_FACTORY_START], 0x0, (EDID_FACTORY_END - EDID_FACTORY_START + 1));
	edid[EDID_FACTORY_START] = data_tmp; //For EDID checksum, we need reserve the sum for blocks.

	return 0;
}

int parse_main(struct dp_ctrl *dptx)
{
	int16_t i;
	int ret;
	uint8_t* block;
	uint8_t* edid_t;
	struct edid_video *vid_info;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] The pointer is NULL.\n");
		return -EINVAL;
	}
	if (dptx->edid == NULL) {
		HISI_FB_ERR("[DP] The edid pointer is NULL.\n");
		return -EINVAL;
	}

	dptx->edid_info.Video.dptx_timing_list = NULL;
	dptx->edid_info.Video.extTiming = NULL;
	dptx->edid_info.Video.dp_monitor_descriptor = NULL;
	dptx->edid_info.Video.extVCount = 0;
	edid_t = dptx->edid;
	/*Initialize some fields*/
	vid_info = &(dptx->edid_info.Video);
	vid_info->mainVCount = 0;
	vid_info->maxPixelClock = 0;

	if (check_main_edid(edid_t)) {
		HISI_FB_ERR("[DP] The main edid block is wrong.\n");
		return -EINVAL;
	}

	check_factory_info(edid_t);

	/*Check EDID version (usually 1.3)*/
	HISI_FB_DEBUG("[DP] EDID version %d revision %d\n", (int)edid_t[EDID_STRUCT_VERSION],
										(int)edid_t[EDID_STRUCT_REVISION]);

	/*Check Display Image Size(Physical)*/
	vid_info->maxHImageSize = (uint16_t)edid_t[H_MAX_IMAGE_SIZE];
	vid_info->maxVImageSize = (uint16_t)edid_t[V_MAX_IMAGE_SIZE];
	/*Alloc the name memory*/
	vid_info->dp_monitor_descriptor = (char *)kzalloc((MONTIOR_NAME_DESCRIPTION_SIZE + 1) * sizeof(char), GFP_KERNEL);
	if (vid_info->dp_monitor_descriptor == NULL) {
		HISI_FB_ERR("[DP] dp_monitor_descriptor memory alloc fail\n");
		return -EINVAL;
	}
	memset(vid_info->dp_monitor_descriptor, 0, MONTIOR_NAME_DESCRIPTION_SIZE + 1);
	/*Parse the EDID Detailed Timing Descriptor*/
	block = edid_t + DETAILED_TIMING_DESCRIPTIONS_START;
	/*EDID main part has a total of four Descriptor Block*/
	for (i = 0; i < DETAILED_TIMING_DESCRIPTION_COUNT; i++,
                            block += DETAILED_TIMING_DESCRIPTION_SIZE)
	{
		switch (block_type(block))
		{
			case DETAILED_TIMING_BLOCK :
				ret = parse_timing_description(dptx, block);
				if (ret != 0) {
					HISI_FB_INFO("[DP] Timing Description Parsing failed!\n");
					return ret;
				}
				break;
			case MONITOR_LIMITS :
				ret = parse_monitor_limits(dptx, block);
				if (ret != 0) {
					HISI_FB_INFO("[DP] Parsing the monitor limit is failed!\n");
					return ret;
				}
				break;
			case MONITOR_NAME :
				ret = parse_monitor_name(dptx, block, DETAILED_TIMING_DESCRIPTION_SIZE);
				if (ret != 0) {
					HISI_FB_ERR("[DP] The monitor name parsing failed.\n");
					return ret;
				}
				break;
			default :
				break;
		}
	}

	HISI_FB_DEBUG("[DP] Extensions to follow:\t%d\n", (int)edid_t[EXTENSION_FLAG]);
	/*Return the number of following extension blocks*/
	return (int)edid_t[EXTENSION_FLAG];
}

int check_exten_edid(uint8_t* exten)
{
	uint32_t i, checksum;
	checksum = 0;
	if (exten == NULL) {
		HISI_FB_ERR("[DP] The pointer is null.\n");
		return -EINVAL;
	}

	for (i = 0; i < EDID_LENGTH; i++)
		checksum += exten[i];
	if (checksum & 0xFF) {
		HISI_FB_ERR("[DP] Extension Data checksum failed - data is corrupt. Continuing anyway.\n");
		return -EINVAL;
	}
	/*Check Extension Tag*/
	/*( Header Tag stored in the first uint8_t )*/
	if (exten[0] != EXTENSION_HEADER) {
		HISI_FB_ERR("[DP] Not CEA-EDID Timing Extension, Extension-Parsing will not continue!\n");
		return -EINVAL;
	}
	return 0;
}

int parse_extension_timing_description(struct dp_ctrl * dptx, uint8_t* dtdBlock, uint32_t dtdBegin, uint16_t DTDtotal)
{
	uint32_t i;
	int ret;

	if ((dptx == NULL) || (dtdBlock == NULL)) {
		HISI_FB_ERR("[DP] The pointer is NULL.\n");
		return -EINVAL;
	}

	if ((dtdBegin + 1 + DTDtotal * DETAILED_TIMING_DESCRIPTION_SIZE) > EDID_LENGTH) {
		HISI_FB_ERR("[DP] The dtd total number 0x%x is out of the limit\n", DTDtotal);
		return -EINVAL;
	}

	for (i = 0; i < DTDtotal; i++, dtdBlock += DETAILED_TIMING_DESCRIPTION_SIZE)
	{
		switch (block_type(dtdBlock))
		{
			case DETAILED_TIMING_BLOCK:
				ret = parse_timing_description(dptx, dtdBlock);
				if (ret != 0) {
					HISI_FB_ERR("[DP] Timing Description Parsing failed!");
					return ret;
				}
				break;
			case MONITOR_LIMITS :
				parse_monitor_limits(dptx, dtdBlock);
				break;
			default :
				break;
		}
	}
	return 0;
}

int parse_extension(struct dp_ctrl * dptx, uint8_t* exten)
{
	int ret;
	uint8_t* dtdBlock;
	uint8_t* ceaBlock;
	uint8_t dtd_start_byte = 0;
	uint8_t cea_data_block_collection = 0;
	uint16_t DTDtotal =0;
	struct edid_video *vid_info;
	struct edid_audio *aud_info;

	if ((dptx == NULL) || (exten == NULL)) {
		HISI_FB_ERR("[DP] The pointer is NULL.\n");
		return -EINVAL;
	}

	vid_info = &(dptx->edid_info.Video);
	aud_info = &(dptx->edid_info.Audio);

	ret = check_exten_edid(exten);
	if (ret) {
		HISI_FB_ERR("[DP] The check_exten_edid failed.\n");
		return ret;
	}
	/*
	* Get uint8_t number (decimal) within this block where the 18-uint8_t DTDs begin.
	* ( Number data stored in the third uint8_t )
	*/

	if (exten[2] == 0x00) {
		HISI_FB_INFO("[DP] There are no DTDs present in this extension block and no non-DTD data.\n");
		return -EINVAL;
	} else if (exten[2] == 0x04) {
		dtd_start_byte = 0x04;
	} else {
		cea_data_block_collection = 0x04;
		dtd_start_byte = exten[2];
	}
	/*
	  *  Get Number of Native DTDs present, other Version 2+ information
	  *  DTD Total stored in the fourth uint8_t )
	  */
	if(1 != aud_info->basicAudio) {
		aud_info->basicAudio = (0x40 & exten[3] ) >> 6;
	}
	dp_imonitor_set_param(DP_PARAM_BASIC_AUDIO, &(aud_info->basicAudio));

	DTDtotal = LOWER_NIBBLE(exten[3]);
	//Parse DTD in Extension
	dtdBlock = exten + dtd_start_byte;
	if (DTDtotal != (EDID_LENGTH - dtd_start_byte - 1)/DETAILED_TIMING_DESCRIPTION_SIZE) {
		HISI_FB_INFO("[DP] The number of native DTDs is not equal the size\n");
		DTDtotal = (EDID_LENGTH - dtd_start_byte - 1)/DETAILED_TIMING_DESCRIPTION_SIZE;
	}

	ret = parse_extension_timing_description(dptx, dtdBlock, dtd_start_byte, DTDtotal);
	if (ret) {
		HISI_FB_ERR("[DP] Parse the extension block timing information fail.\n");
		return ret;
	}
	/*Parse CEA Data Block Collection*/
	if (cea_data_block_collection == 0x04) {
		ceaBlock = exten + cea_data_block_collection;
		ret = parse_cea_data_block(dptx, ceaBlock, dtd_start_byte);
		if (ret != 0) {
			HISI_FB_ERR("[DP] CEA data block Parsing failed!\n");
			return ret;
		}
	}

	return 0;
}
/*lint -e429*/
int parse_timing_description(struct dp_ctrl *dptx, uint8_t *dtd)
{
	struct edid_video *vid_info;
	struct timing_info *node = NULL;

	if ((dptx == NULL) || (dtd == NULL)) {
		HISI_FB_ERR("[DP] The pointer is NULL.\n");
		return -EINVAL;
	}

	vid_info = &(dptx->edid_info.Video);

	if (vid_info->dptx_timing_list == NULL) {
		vid_info->dptx_timing_list = kzalloc(sizeof(struct list_head), GFP_KERNEL);
		if (NULL == vid_info->dptx_timing_list) {
			HISI_FB_ERR("[DP] vid_info->dptx_timing_list is NULL");
			return -EINVAL;
		}
		INIT_LIST_HEAD(vid_info->dptx_timing_list);

		vid_info->mainVCount = 0;
		/*Initial Max Value*/
		vid_info->maxHPixels = H_ACTIVE;
		vid_info->maxVPixels = V_ACTIVE;
	}

	/*Get Max Value by comparing all values*/
	if ((H_ACTIVE > vid_info->maxHPixels) && (V_ACTIVE > vid_info->maxVPixels)) {
		vid_info->maxHPixels = H_ACTIVE;
		vid_info->maxVPixels = V_ACTIVE;
	}
	//node
	node = kzalloc(sizeof(struct timing_info), GFP_KERNEL);
	if (node) {
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

		node->inputType = INPUT_TYPE;//need to modify later
		node->interlaced = INTERLACED;
		node->vSyncPolarity = V_SYNC_POLARITY;
		node->hSyncPolarity = H_SYNC_POLARITY;
		node->syncScheme = SYNC_SCHEME;
		node->schemeDetail = SCHEME_DETAIL;

		vid_info->mainVCount += 1;
		list_add_tail(&node->list_node, vid_info->dptx_timing_list);
	} else {
		HISI_FB_ERR("[DP] kzalloc struct dptx_hdmi_vic fail!\n");
		return -EINVAL;
	}

	HISI_FB_INFO("[DP] The timinginfo (%d): hActivePixels is %d, vActivePixels is %d, pixel clock = %llu\n",
		vid_info->mainVCount, node->hActivePixels, node->vActivePixels, node->pixelClock);
	dp_imonitor_set_param_timing(node->hActivePixels, node->vActivePixels, node->pixelClock, 0);

	return 0;
}

int parse_timing_description_by_vesaid(struct edid_video *vid_info, uint8_t vesa_id)
{
	struct dtd mdtd;
	struct timing_info *node = NULL;

	if (vid_info == NULL) {
		HISI_FB_ERR("[DP] The pointer is NULL.\n");
		return -EINVAL;
	}

	if (!dptx_dtd_fill(&mdtd, vesa_id, 60000, VCEA)) {
		HISI_FB_ERR("[DP] Invalid video mode value %d\n",
						vesa_id);
		return -EINVAL;
	}

	if (vid_info->dptx_timing_list == NULL) {
		vid_info->dptx_timing_list = kzalloc(sizeof(struct list_head), GFP_KERNEL);
		if (NULL == vid_info->dptx_timing_list) {
			HISI_FB_ERR("[DP] vid_info->dptx_timing_list is NULL");
			return -EINVAL;
		}
		INIT_LIST_HEAD(vid_info->dptx_timing_list);

		vid_info->mainVCount = 0;
		/*Initial Max Value*/
		vid_info->maxHPixels = mdtd.h_active;
		vid_info->maxVPixels = mdtd.v_active;
	}

	/*Get Max Value by comparing all values*/
	if ((mdtd.h_active > vid_info->maxHPixels) && (mdtd.v_active > vid_info->maxVPixels)) {
		vid_info->maxHPixels = mdtd.h_active;
		vid_info->maxVPixels = mdtd.v_active;
	}

	//node
	node = kzalloc(sizeof(struct timing_info), GFP_KERNEL);
	if (node) {
		node->hActivePixels = mdtd.h_active;
		node->hBlanking = mdtd.h_blanking;
		node->hSyncOffset = mdtd.h_sync_offset;
		node->hSyncPulseWidth = mdtd.h_sync_pulse_width;
		node->hSize = mdtd.h_image_size ;

		node->vActivePixels = mdtd.v_active;
		node->vBlanking = mdtd.v_blanking;
		node->vSyncOffset = mdtd.v_sync_offset;
		node->vSyncPulseWidth = mdtd.v_sync_pulse_width;
		node->vSize = mdtd.v_image_size;

		node->pixelClock = mdtd.pixel_clock / 10;  //Edid pixel clock is 1/10 of dtd filled timing.
		node->interlaced = mdtd.interlaced;
		node->vSyncPolarity = mdtd.v_sync_polarity;
		node->hSyncPolarity = mdtd.h_sync_polarity;

		vid_info->mainVCount += 1;
		list_add_tail(&node->list_node, vid_info->dptx_timing_list);
	} else {
		HISI_FB_ERR("[DP] kzalloc struct dptx_hdmi_vic fail!\n");
		return -EINVAL;
	}

	HISI_FB_INFO("[DP] The timinginfo (%d): hActivePixels is %d, vActivePixels is %d, pixel clock = %llu\n",
		vid_info->mainVCount, node->hActivePixels, node->vActivePixels, node->pixelClock);
	dp_imonitor_set_param_timing(node->hActivePixels, node->vActivePixels, node->pixelClock, vesa_id);

	return 0;
}

int parse_hdmi_vic_id(uint8_t vic_id)
{
	struct dptx_hdmi_vic *node = NULL;

	if (g_hdmi_vic_real_len >= g_hdmi_vic_len) {
		HISI_FB_ERR("[DP] The g_hdmi_vic_real_len is more than g_hdmi_vic_len.\n");
		return -EINVAL;
	}

	if (NULL == dptx_hdmi_list) {
		dptx_hdmi_list = kzalloc(sizeof(struct list_head), GFP_KERNEL);
		if (NULL == dptx_hdmi_list) {
			HISI_FB_ERR("[DP] dptx_hdmi_list is NULL");
			return -EINVAL;
		}
		INIT_LIST_HEAD(dptx_hdmi_list);
	}

	//node
	node = kzalloc(sizeof(struct dptx_hdmi_vic), GFP_KERNEL);
	if (node) {
		node->vic_id = vic_id;
		HISI_FB_INFO("[DP] vic_id = %d!\n", vic_id);
		list_add_tail(&node->list_node, dptx_hdmi_list);
		g_hdmi_vic_real_len ++;
	} else {
		HISI_FB_ERR("[DP] kzalloc struct dptx_hdmi_vic fail!\n");
	}

	return 0;
}
/*lint +e429*/
int parse_audio_spec_info(struct edid_audio *aud_info, struct edid_audio_info *spec_info, uint8_t* cDblock)
{
	if ((cDblock == NULL) || (spec_info == NULL)) {
		HISI_FB_ERR("[DP] The pointer is NULL.\n");
		return -EINVAL;
	}
	if ((EXTEN_AUDIO_FORMAT <= 8) && (EXTEN_AUDIO_FORMAT >= 1)) {
		/*Set up SAD fields*/
		spec_info->format = EXTEN_AUDIO_FORMAT;
		spec_info->channels =  EXTEN_AUDIO_MAX_CHANNELS;
		spec_info->sampling = EXTEN_AUDIO_SAMPLING;
		if (EXTEN_AUDIO_FORMAT == 1)
			spec_info->bitrate = EXTEN_AUDIO_LPCM_BIT;
		else
			spec_info->bitrate = EXTEN_AUDIO_BITRATE;
		aud_info->extACount += 1;

		HISI_FB_INFO("[DP] parse audio spec success. format(0x%x), channels(0x%x), sampling(0x%x), bitrate(0x%x)\n",
			spec_info->format, spec_info->channels, spec_info->sampling, spec_info->bitrate);
	}

	return 0;
}

int parse_extension_audio_tag(struct edid_audio *aud_info, uint8_t* cDblock, uint8_t tempL)
{
	uint8_t i, xa;
	void *temp_ptr;
	if ((aud_info == NULL) ||(cDblock == NULL)) {
		HISI_FB_ERR("[DP] The pointer is NULL.\n");
		return -EINVAL;
	}

	if(tempL < 1) {
		HISI_FB_ERR("[DP] The input param tempL is wrong! \n");
		return -EINVAL;
	}

	for (i = 0; i < (tempL) / 3; i++) {
		xa = aud_info->extACount;

		if (xa == 0) {
			/*Initial audio part*/
			if (aud_info->spec != NULL) {
				HISI_FB_ERR("[DP] The spec of audio is error.\n");
				return -EINVAL;
			}
			aud_info->spec = kzalloc(sizeof(struct edid_audio_info), GFP_KERNEL);
			if (aud_info->spec == NULL) {
				HISI_FB_ERR("[DP] malloc Audio Spec part failed!\n");
				return -EINVAL;
			}
		} else {
			/*Add memory as needed with error handling*/
			temp_ptr = kzalloc((xa + 1) * sizeof(struct edid_audio_info), GFP_KERNEL);
			if (temp_ptr == NULL) {
				HISI_FB_ERR("[DP] Realloc Audio Spec part failed!\n");
				return -EINVAL;
			} else {
				if(aud_info->spec == NULL) {
					HISI_FB_ERR("[DP] The spec is NULL.\n");
					kfree(temp_ptr);
					return -EINVAL;
				}
				memcpy(temp_ptr, aud_info->spec, xa * sizeof(struct edid_audio_info));
				kfree(aud_info->spec);
				aud_info->spec = NULL;
				aud_info->spec = temp_ptr;
			}
		}
		if (parse_audio_spec_info(aud_info, &(aud_info->spec[xa]), cDblock)) {
			HISI_FB_ERR("[DP] parse the audio spec info fail.\n");
			return -EINVAL;
		}

		cDblock += 3;
	}
	return 0;
}

int parse_extension_video_tag(struct edid_video *vid_info, uint8_t* cDblock, uint8_t length)
{
	uint8_t i;
	if ((vid_info == NULL) || (cDblock == NULL)) {
		HISI_FB_ERR("[DP] The pointer is NULL.\n");
		return -EINVAL;
	}

	if(length < 1) {
		HISI_FB_ERR("[DP] The input param tempL is wrong! \n");
		return -EINVAL;
	}

	vid_info->extTiming = kzalloc(length * sizeof(struct ext_timing), GFP_KERNEL);
	memset(vid_info->extTiming, 0x0, length * sizeof(struct ext_timing));
	vid_info->extVCount = 0;

	for (i = 0; i < length; i++) {
		if (EXTEN_VIDEO_CODE != 0) {
			/*Set up SVD fields*/
			vid_info->extTiming[i].extFormatCode = EXTEN_VIDEO_CODE;
			vid_info->extVCount += 1;
		}
		cDblock += 1;
	}
	return 0;
}

int parse_extension_vsdb_tag(struct edid_video *vid_info, uint8_t* cDblock, uint8_t length)
{
	uint8_t i;
	uint32_t IEEE_FLAG;
	uint32_t HDMI_CEC_PORT;
	uint8_t Max_TMDS_Clock;
	uint8_t Latency_Fields_Present;
	uint8_t I_Latency_Fields_Present;
	uint8_t HDMI_VIDEO_Present;
	uint8_t VESA_ID;
	bool support_ai;
	bool b3dpresent;
	//struct dptx_hdmi_vic *hdmi_vic_node, *_node_;

	VESA_ID = 0;
	g_hdmi_vic_real_len = g_hdmi_vic_len = 0;
	Latency_Fields_Present = I_Latency_Fields_Present = 0;
	if ((vid_info == NULL) || (cDblock == NULL)) {
		HISI_FB_ERR("[DP] The pointer is NULL.\n");
		return -EINVAL;
	}

	if (length < 5) {
		HISI_FB_ERR("[DP] VSDB length isn't correct.\n");
		return -EINVAL;
	}

	IEEE_FLAG = 0;
	IEEE_FLAG = (cDblock[0]) | (cDblock[1] << 8) | (cDblock[2] << 16);

	if (IEEE_FLAG != 0x000c03) {
		HISI_FB_INFO("[DP] This block isn't belong to HDMI block: %x.\n", IEEE_FLAG);
		return 0;
	}

	HDMI_CEC_PORT = ((cDblock[3] << 8) | (cDblock[4]));

	for (i = 5; i < length; i++) {
		switch (i) {
		case 5:
			support_ai = (cDblock[i] & 0x80) >> 7;
			break;
		case 6:
			Max_TMDS_Clock = cDblock[i];
			break;
		case 7:
			Latency_Fields_Present = (cDblock[i] & 0x80) >> 7;
			I_Latency_Fields_Present = (cDblock[i] & 0x40) >> 6;
			HDMI_VIDEO_Present = (cDblock[i] & 0x20) >> 5;
			if (0 == HDMI_VIDEO_Present) {
				HISI_FB_INFO("[DP] This EDID don't include HDMI additional video format (1).\n");
				return 0;
			}
			break;
		case 8:
			if (Latency_Present == Present_None) {
				b3dpresent = (cDblock[i] & 0x80) >> 7;
			}
			break;
		case 9:
			if (Latency_Present == Present_None) {
				g_hdmi_vic_len = (cDblock[i] & 0xE0) >> 5;
				if (g_hdmi_vic_len == 0) {
					HISI_FB_INFO("[DP] This EDID don't include HDMI additional video format (2).\n");
					return 0;
				}
				g_hdmi_vic_real_len = 0;
			}
			break;
		case 10:
			if (Latency_Present == Present_One) {
				b3dpresent = (cDblock[i] & 0x80) >> 7;
			} else if (Latency_Present == Present_None) {
				parse_hdmi_vic_id(cDblock[i]);
			}
			break;
		case 11:
			if (Latency_Present == Present_One) {
				g_hdmi_vic_len = (cDblock[i] & 0xE0) >> 5;
				if (g_hdmi_vic_len == 0) {
					HISI_FB_INFO("[DP] This EDID don't include HDMI additional video format (2).\n");
					return 0;
				}
				g_hdmi_vic_real_len = 0;
			} else if (Latency_Present == Present_None) {
				parse_hdmi_vic_id(cDblock[i]);
			}
			break;
		case 12:
			if (Latency_Present == Present_Both) {
				b3dpresent = (cDblock[i] & 0x80) >> 7;
			} else {
				parse_hdmi_vic_id(cDblock[i]);
			}
			break;
		case 13:
			if (Latency_Present == Present_Both) {
				g_hdmi_vic_len = (cDblock[i] & 0xE0) >> 5;
				if (g_hdmi_vic_len == 0) {
					HISI_FB_INFO("[DP] This EDID don't include HDMI additional video format (2).\n");
					return 0;
				}
				g_hdmi_vic_real_len = 0;
			} else {
				parse_hdmi_vic_id(cDblock[i]);
			}
			break;
		case 14:
		case 15:
		case 16:
		case 17:
			parse_hdmi_vic_id(cDblock[i]);
			break;
		default:
			break;
		}
	}

	HISI_FB_INFO("[DP] vic_id real length  = %d , vic length = %d !\n", g_hdmi_vic_real_len, g_hdmi_vic_len);

	return 0;
}

int parse_cea_data_block(struct dp_ctrl *dptx, uint8_t* ceaData, uint8_t dtdStart)
{
	uint8_t totalLength, blockLength;
	uint8_t* cDblock = ceaData;
	struct edid_video *vid_info;
	struct edid_audio *aud_info;
	/* exTlist *extlist; */
	/*Initialize some fields*/
	if ((dptx == NULL) || (ceaData == NULL)) {
		HISI_FB_ERR("[DP] The pointer is NULL.\n");
		return -EINVAL;
	}
	if (dtdStart > (EDID_LENGTH - 1)) {
		HISI_FB_ERR("[DP] The start of dtd is out of limit.\n");
		return -EINVAL;
	}

	totalLength = 4;
	vid_info = &(dptx->edid_info.Video);
	aud_info = &(dptx->edid_info.Audio);
	/*Parse CEA Data Block Collection*/
	do
	{
		/*Get length(total number of following uint8_ts of a certain tag)*/
		blockLength = GET_CEA_DATA_BLOCK_LEN(cDblock);
		/*Get tag code*/
		switch (GET_CEA_DATA_BLOCK_TAG(cDblock))
		{
			case EXTENSION_AUDIO_TAG:
				/* Block type tag combined with data length takes 1 uint8_t*/
				cDblock += 1;
				/* Each Short Audio Descriptor(SAD) takes 3 uint8_ts*/
				if (parse_extension_audio_tag(aud_info, cDblock, blockLength)) {
					HISI_FB_ERR("[DP] parse_extension_audio_tag fail.\n");
					return -EINVAL;
				}
				cDblock += blockLength;
				break;
			case EXTENSION_VIDEO_TAG:
				/* Block type tag combined with data length takes 1 uint8_t*/
				cDblock += 1;
				/* Each Short Video Descriptor(SVD) takes 1 uint8_t*/
				if (parse_extension_video_tag(vid_info, cDblock, blockLength)) {
					HISI_FB_ERR("[DP] parse_extension_video_tag fail.\n");
					return -EINVAL;
				}
				cDblock += blockLength;
				break;
			case EXTENSION_VENDOR_TAG:
				cDblock += 1;
				if (parse_extension_vsdb_tag(vid_info, cDblock, blockLength)) {
					HISI_FB_ERR("[DP] parse_extension_vsdb_tag fail.\n");
					return -EINVAL;
				}
				cDblock += blockLength;
				break;
			case EXTENSION_SPEAKER_TAG:
				/*
				  * If the extension has Speaker Allocation Data,
				  * there should only be one
				  */
				cDblock += 1;
				aud_info->extSpeaker = EXTEN_SPEAKER;
				/*Speaker Allocation Data takes 3 uint8_ts*/
				cDblock += 3;
				break;
			default:
				cDblock += blockLength + 1;
				break;
		}
		totalLength = totalLength + blockLength + 1;
	}while(totalLength < dtdStart);

	return 0;
}

int block_type(uint8_t* block)
{
	if (block == NULL) {
		HISI_FB_ERR("[DP] The pointer is NULL.\n");
		return -EINVAL;
	}
	if ((block[0] == 0) && (block[1] == 0)) {
		/*Other descriptor*/
		if ((block[2] != 0) || (block[4] != 0))
			return UNKNOWN_DESCRIPTOR;
		return block[3];
	} else {
		/*Detailed timing block*/
		return DETAILED_TIMING_BLOCK;
	}
}

int parse_monitor_limits(struct dp_ctrl* dptx, uint8_t* block)
{
	struct edid_video *vid_info;
	if ((dptx == NULL) || (block == NULL)) {
		HISI_FB_ERR("[DP] The pointer is NULL.\n");
		return -EINVAL;
	}

	vid_info = &(dptx->edid_info.Video);
	/*Set up limit fields*/
	vid_info->maxHFreq = H_MAX_RATE;
	vid_info->minHFreq = H_MIN_RATE;
	vid_info->maxVFreq = V_MAX_RATE;
	vid_info->minVFreq = V_MIN_RATE;
	vid_info->maxPixelClock = (int)MAX_PIXEL_CLOCK;

	return 0;
}

int parse_monitor_name(struct dp_ctrl* dptx, uint8_t* blockname, uint32_t size)
{
	uint32_t i, data0, data1, data2;
	uint32_t str_start, str_end;
	uint8_t data_tmp;

	str_start = 5;
	str_end = 0;
	data_tmp = 0;
	if ((dptx == NULL) || (blockname == NULL)) {
		HISI_FB_ERR("[DP] The pointer is NULL.\n");
		return -EINVAL;
	}
	if (size != DETAILED_TIMING_DESCRIPTION_SIZE) {
		HISI_FB_ERR("[DP] The length of monitor name is wrong.\n");
		return -EINVAL;
	}
	if (size >= 7) {
		data0 = blockname[5];
		data1 = blockname[6];
		for (i = 7; i < size; i++)
		{
			data2 = blockname[i];
			if(((data0== 0x0A) && (data1 == 0x20)) && (data2 == 0x20)) {
				str_end = i - 2;
				break;
			}
			data0 = data1;
			data1 = data2;
		}
	}
	if((blockname[16] == 0x0A) && (blockname[17] == 0x20))
		str_end = 16;

	if (str_end < str_start) {
		return 0;
	}

	if (((str_end - str_start) > 0) && (dptx->edid_info.Video.dp_monitor_descriptor != NULL)) {
		memcpy(dptx->edid_info.Video.dp_monitor_descriptor, blockname + 5, (str_end - str_start));
		dptx->edid_info.Video.dp_monitor_descriptor[str_end - str_start] = '\0';

		/*For hiding custom device info*/
		for (i = str_start; i < str_end; i++) {
			data_tmp += blockname[i];
		}
		memset(&blockname[5], 0x0, (str_end - str_start));
		blockname[5] = data_tmp; //For EDID checksum, we need reserve the sum for blocks.
	}

	return 0;
}

int release_edid_info(struct dp_ctrl *dptx)
{
	struct timing_info *dptx_timing_node, *_node_;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] The pointer is NULL.\n");
		return -EINVAL;
	}

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

	if (dptx->edid_info.Video.dp_monitor_descriptor !=NULL) {
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
