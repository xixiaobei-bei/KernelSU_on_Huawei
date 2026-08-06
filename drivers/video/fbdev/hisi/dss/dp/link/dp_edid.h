/* Copyright (c) 2016-2016, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef __DP_EDID_H__
#define __DP_EDID_H__

#include <linux/string.h>
#include "hisi_fb.h"
#include "hisi_dp.h"
#include "dp_irq.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmacro-redefined"

#define EDID_BLOCK_LENGTH 128
#define EDID_NUM 256

static const uint8_t edid_v1_header[8] = {0x00, 0xff, 0xff, 0xff,
								   0xff, 0xff, 0xff, 0x00};

#define UPPER_NIBBLE(x) \
		((0xF0 & (x)) >> 4)

#define LOWER_NIBBLE(x) \
		(0xF & (x))

#define COMBINE_HI_8LO(hi, lo) \
		((((uint8_t)hi) << 8) | (uint8_t)lo)

#define COMBINE_HI_4LO(hi, lo) \
		((((uint8_t)hi) << 4) | (uint8_t)lo)

#define GET_ARRAY_LEN(array) \
			(ARRAY_SIZE(array))

#define GET_CEA_DATA_BLOCK_TAG(x) \
			(((uint8_t)0xE0 & x[0]) >> 5)

#define GET_CEA_DATA_BLOCK_LEN(x) \
			((uint8_t)0x1F & x[0])

#define EDID_LENGTH				   0x80 //lint !e547

#define EDID_HEADER				   0x00
#define EDID_HEADER_END			   0x07

#define EDID_FACTORY_START			   0x08
#define EDID_FACTORY_END			0x11

#define EDID_STRUCT_VERSION		   0x12
#define EDID_STRUCT_REVISION	   0x13

#define EXTENSION_FLAG			   0x7e

#define DETAILED_TIMING_DESCRIPTIONS_START	0x36
#define DETAILED_TIMING_DESCRIPTION_SIZE	18
#define MONTIOR_NAME_DESCRIPTION_SIZE	 14
#define DETAILED_TIMING_DESCRIPTION_COUNT	4

#define PIXEL_CLOCK_LO	(uint8_t)(dtd[0])
#define PIXEL_CLOCK_HI	(uint8_t)(dtd[1])
#define PIXEL_CLOCK		(COMBINE_HI_8LO(PIXEL_CLOCK_HI, PIXEL_CLOCK_LO))

#define H_ACTIVE_LO		(uint8_t)(dtd[2])

#define H_BLANKING_LO	(uint8_t)(dtd[3])

#define H_ACTIVE_HI		UPPER_NIBBLE((uint8_t)dtd[4])

#define H_ACTIVE		COMBINE_HI_8LO(H_ACTIVE_HI, H_ACTIVE_LO)

#define H_BLANKING_HI	LOWER_NIBBLE((uint8_t)dtd[4])

#define H_BLANKING		COMBINE_HI_8LO(H_BLANKING_HI, H_BLANKING_LO)

#define V_ACTIVE_LO	   (uint8_t)(dtd[5])

#define V_BLANKING_LO  (uint8_t)(dtd[6])

#define V_ACTIVE_HI	   UPPER_NIBBLE((uint8_t)dtd[7])

#define V_ACTIVE	   COMBINE_HI_8LO(V_ACTIVE_HI, V_ACTIVE_LO)

#define V_BLANKING_HI  LOWER_NIBBLE((uint8_t)dtd[7])

#define V_BLANKING	   COMBINE_HI_8LO(V_BLANKING_HI, V_BLANKING_LO)

#define H_SYNC_OFFSET_LO	(uint8_t)(dtd[8])
#define H_SYNC_WIDTH_LO		(uint8_t)(dtd[9])

#define V_SYNC_OFFSET_LO	UPPER_NIBBLE((uint8_t)dtd[10])
#define V_SYNC_WIDTH_LO		LOWER_NIBBLE((uint8_t)dtd[10])

#define V_SYNC_WIDTH_HI		((uint8_t)dtd[11] & (0x1 | 0x2))
#define V_SYNC_OFFSET_HI	(((uint8_t)dtd[11] & (0x4 | 0x8)) >> 2)

#define H_SYNC_WIDTH_HI		(((uint8_t)dtd[11] & (0x10 | 0x20)) >> 4)
#define H_SYNC_OFFSET_HI	(((uint8_t)dtd[11] & (0x40 | 0x80)) >> 6)

#define V_SYNC_WIDTH		COMBINE_HI_4LO(V_SYNC_WIDTH_HI, V_SYNC_WIDTH_LO)
#define V_SYNC_OFFSET		COMBINE_HI_4LO(V_SYNC_OFFSET_HI, V_SYNC_OFFSET_LO)

#define H_SYNC_WIDTH		COMBINE_HI_8LO(H_SYNC_WIDTH_HI, H_SYNC_WIDTH_LO)
#define H_SYNC_OFFSET		COMBINE_HI_8LO(H_SYNC_OFFSET_HI, H_SYNC_OFFSET_LO)

#define H_SIZE_LO			(uint8_t)(dtd[12])
#define V_SIZE_LO			(uint8_t)(dtd[13])

#define H_SIZE_HI			UPPER_NIBBLE((uint8_t)dtd[14])
#define V_SIZE_HI			LOWER_NIBBLE((uint8_t)dtd[14])

#define H_SIZE				COMBINE_HI_8LO(H_SIZE_HI, H_SIZE_LO)
#define V_SIZE				COMBINE_HI_8LO(V_SIZE_HI, V_SIZE_LO)

#define H_BORDER			(uint8_t)(dtd[15])
#define V_BORDER			(uint8_t)(dtd[16])

#define FLAGS				(uint8_t)(dtd[17])

#define INTERLACED			((FLAGS & 0x80) >> 7)
#define V_SYNC_POLARITY ((FLAGS & 0x4) >> 2)
#define H_SYNC_POLARITY ((FLAGS & 0x2) >> 1)
#define INPUT_TYPE			(((FLAGS & 0x60) >> 4) | ((FLAGS & 0x1)))
#define SYNC_SCHEME			((FLAGS & 0x18) >> 3)
#define SCHEME_DETAIL		((FLAGS & 0x6) >> 1)

#define OTHER_DESCRIPTOR_DATA	5
#define MONITOR_NAME			0xfc
#define MONITOR_LIMITS			0xfd
#define MONITOR_SERIAL_NUMBER	0xff
#define UNKNOWN_DESCRIPTOR		-1
#define DETAILED_TIMING_BLOCK	-2

#define V_MIN_RATE				block[5]
#define V_MAX_RATE				block[6]
#define H_MIN_RATE				block[7]
#define H_MAX_RATE				block[8]
#define MAX_PIXEL_CLOCK			(((int)block[9]) * 10)
#define H_MAX_IMAGE_SIZE		0x15
#define V_MAX_IMAGE_SIZE		0x16

#define EXTENSION_HEADER		   0x02

#define EXTENSION_AUDIO_TAG		   1
#define EXTENSION_VIDEO_TAG		   2
#define EXTENSION_VENDOR_TAG	   3
#define EXTENSION_SPEAKER_TAG	   4

#define EXTEN_AUDIO_FORMAT_LPCM 1
#define EXTEN_AUDIO_FORMAT		   ((0x78 & cea_data_block[0]) >> 3)
#define EXTEN_AUDIO_MAX_CHANNELS   ((0x7 & cea_data_block[0]) + 1)
#define EXTEN_AUDIO_SAMPLING	   (0x7F & cea_data_block[1])
#define EXTEN_AUDIO_LPCM_BIT	   (0x7 & cea_data_block[2])
#define EXTEN_AUDIO_BITRATE		   ((uint8_t)cea_data_block[2] * 8)

#define EXTEN_VIDEO_NATIVE		   (((uint8_t)0x80 & *cea_data_block) >> 7)
#define EXTEN_VIDEO_CODE		   ((uint8_t)0x7F & *cea_data_block)

#define EXTEN_SPEAKER			   (0x7F & cea_data_block[0])

#define LATENCY_PRESENT	(latency_fields + interlaced_latency_fields)

/* EDID Audio Data Block */
#define AUDIO_TAG		1
#define VIDEO_TAG		2
#define EDID_TAG_MASK		GENMASK(7, 5)
#define EDID_TAG_SHIFT		5
#define EDID_SIZE_MASK		GENMASK(4, 0)
#define EDID_SIZE_SHIFT		0

/* Established timing blocks */
#define ET1_800X600_60HZ	BIT(0)
#define ET1_800X600_56HZ	BIT(1)
#define ET1_640X480_75HZ	BIT(2)
#define ET1_640X480_72HZ	BIT(3)
#define ET1_640X480_67HZ	BIT(4)
#define ET1_640X480_60HZ	BIT(5)
#define ET1_720X400_88HZ	BIT(6)
#define ET1_720X400_70HZ	BIT(7)

#define ET2_1280X1024_75HZ	BIT(0)
#define ET2_1024X768_75HZ	BIT(1)
#define ET2_1024X768_70HZ	BIT(2)
#define ET2_1024X768_60HZ	BIT(3)
#define ET2_1024X768_87HZ	BIT(4)
#define ET2_832X624_75HZ	BIT(5)
#define ET2_800X600_75HZ	BIT(6)
#define ET2_800X600_72HZ	BIT(7)

#define ET3_1152X870_75HZ	BIT(7)

int parse_edid(struct dp_ctrl *dptx, uint16_t len);
int release_edid_info(struct dp_ctrl *dptx);
bool convert_code_to_dtd(struct dtd *mdtd, uint8_t code, uint32_t refresh_rate, uint8_t video_format);

int parse_main(struct dp_ctrl *dptx);
int parse_extension(struct dp_ctrl *dptx, uint8_t *exten);
int parse_timing_description(struct dp_ctrl *dptx, uint8_t *dtd);
int parse_cea_data_block(struct dp_ctrl *dptx, uint8_t *cea_data, uint8_t dtd_start);
int parse_monitor_limits(struct dp_ctrl *dptx, uint8_t *block);
int parse_monitor_name(struct dp_ctrl *dptx, uint8_t *blockname, uint32_t size);
int block_type(uint8_t *block);

#endif /* DP_EDID_H */
#pragma GCC diagnostic pop
