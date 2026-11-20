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
/*lint -e547*/

#ifndef __DRM_DP_HELPER_ADDITIONS_H__
#define __DRM_DP_HELPER_ADDITIONS_H__
#include <linux/version.h>

#include "hisi_defs.h"

/**
 * Training patterns and bandwidths for dp1.4
 */
#define DPTX_PHYIF_CTRL_TPS_NONE	0
#define DPTX_PHYIF_CTRL_TPS_1		1
#define DPTX_PHYIF_CTRL_TPS_2		2
#define DPTX_PHYIF_CTRL_TPS_3		3
#define DPTX_PHYIF_CTRL_TPS_4		4
#define DPTX_PHYIF_CTRL_RATE_RBR	0x0
#define DPTX_PHYIF_CTRL_RATE_HBR	0x1
#define DPTX_PHYIF_CTRL_RATE_HBR2	0x2
#define DPTX_PHYIF_CTRL_RATE_HBR3	0x3

#define FHD_TIMING_H_ACTIVE 1920
#define FHD_TIMING_V_ACTIVE 1080

/*
 * The following are cut from (hisi)drm_dp_helper.h
 */
#define DP_ADJUST_POST_CCURSOR2		0x20c

#define DP_SYMBOL_ERROR_COUNT_LANE0_L 0x210
#define DP_SYMBOL_ERROR_COUNT_LANE0_H 0x211
#define DP_SYMBOL_ERROR_COUNT_LANE1_L 0x212
#define DP_SYMBOL_ERROR_COUNT_LANE1_H 0x213
#define DP_SYMBOL_ERROR_COUNT_LANE2_L 0x214
#define DP_SYMBOL_ERROR_COUNT_LANE2_H 0x215
#define DP_SYMBOL_ERROR_COUNT_LANE3_L 0x216
#define DP_SYMBOL_ERROR_COUNT_LANE3_H 0x217

#define DP_TEST_PHY_PATTERN         0x248

#define DPTX_PHY_TEST_PATTERN_SEL_MASK	GENMASK(2, 0)

#define DPTX_NO_TEST_PATTERN_SEL		0
#define DPTX_D102_WITHOUT_SCRAMBLING	1
#define DPTX_SYMBOL_ERROR_MEASUREMENT_COUNT	2
#define DPTX_TEST_PATTERN_PRBS7		3
#define DPTX_80BIT_CUSTOM_PATTERN_TRANS		4
#define DPTX_CP2520_HBR2_COMPLIANCE_EYE_PATTERN	5
#define DPTX_CP2520_HBR2_COMPLIANCE_PATTERN_2	6
#define DPTX_CP2520_HBR2_COMPLIANCE_PATTERN_3	7

#define DP_DPCD_REV_EXT 0x2200

/*
 * The following aren't yet defined in kernel headers
 */
#define DP_LINK_BW_8_1				0x1e
#define DP_TRAINING_PATTERN_4			7
#define DP_TPS4_SUPPORTED			BIT(7)

#define DP_PSR2_WITH_Y_COORD_IS_SUPPORTED  3      /* eDP 1.4a */

#define DP_DSC_DEC_COLOR_FORMAT_CAP         0x069
#define DP_DSC_RGB                         (1 << 0)
#define DP_DSC_YCBCR_444                    (1 << 1)
#define DP_DSC_YCBCR_422_SIMPLE             (1 << 2)
#define DP_DSC_YCBCR_422_NATIVE             (1 << 3)
#define DP_DSC_YCBCR_420_NATIVE             (1 << 4)

#define DP_DSC_ENABLE            0x160   /* DP 1.4 */
#define DP_DSC_DECOMPRESSION_IS_ENABLED  (1 << 0)

#define DP_TEST_LINK_AUDIO_PATTERN		BIT(5)
#define DP_TEST_H_TOTAL_MSB                     0x222
#define DP_TEST_H_TOTAL_LSB			0x223
#define DP_TEST_V_TOTAL_MSB                     0x224
#define DP_TEST_V_TOTAL_LSB			0x225
#define DP_TEST_H_START_MSB			0x226
#define DP_TEST_H_START_LSB			0x227
#define DP_TEST_V_START_MSB			0x228
#define DP_TEST_V_START_LSB			0x229
#define DP_TEST_H_SYNC_WIDTH_MSB		0x22A
#define DP_TEST_H_SYNC_WIDTH_LSB		0x22B
#define DP_TEST_V_SYNC_WIDTH_MSB		0x22C
#define DP_TEST_V_SYNC_WIDTH_LSB		0x22D
#define DP_TEST_H_WIDTH_MSB			0x22E
#define DP_TEST_H_WIDTH_LSB			0x22F
#define DP_TEST_V_WIDTH_MSB			0x230
#define DP_TEST_V_WIDTH_LSB			0x231
#define DP_TEST_PHY_PATTERN			0x248
#define DP_TEST_PATTERN_NONE			0x0
#define DP_TEST_PATTERN_COLOR_RAMPS		0x1
#define DP_TEST_PATTERN_BW_VERITCAL_LINES	0x2
#define DP_TEST_PATTERN_COLOR_SQUARE		0x3

#define DP_TEST_80BIT_CUSTOM_PATTERN_0		0x250
#define DP_TEST_80BIT_CUSTOM_PATTERN_1		0x251
#define DP_TEST_80BIT_CUSTOM_PATTERN_2		0x252
#define DP_TEST_80BIT_CUSTOM_PATTERN_3		0x253
#define DP_TEST_80BIT_CUSTOM_PATTERN_4		0x254
#define DP_TEST_80BIT_CUSTOM_PATTERN_5		0x255
#define DP_TEST_80BIT_CUSTOM_PATTERN_6		0x256
#define DP_TEST_80BIT_CUSTOM_PATTERN_7		0x257
#define DP_TEST_80BIT_CUSTOM_PATTERN_8		0x258
#define DP_TEST_80BIT_CUSTOM_PATTERN_9		0x259

#define DP_TEST_PHY_PATTERN_SEL_MASK		GENMASK(2, 0)
#define DP_TEST_PHY_PATTERN_NONE		0x0
#define DP_TEST_PHY_PATTERN_D10			0x1
#define DP_TEST_PHY_PATTERN_SEMC		0x2
#define DP_TEST_PHY_PATTERN_PRBS7		0x3
#define DP_TEST_PHY_PATTERN_CUSTOM		0x4
#define DP_TEST_PHY_PATTERN_CP2520_1		0x5
#define DP_TEST_PHY_PATTERN_CP2520_2		0x6
#define DP_TEST_PHY_PATTERN_CP2520_3_TPS4	0x7

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmacro-redefined"

#define DP_TEST_MISC				0x232
#define DP_TEST_COLOR_FORMAT_MASK		GENMASK(2, 1)
#define DP_TEST_BIT_DEPTH_MASK                  GENMASK(7, 5)
#define DP_TEST_BIT_DEPTH_SHIFT            5
#define DP_TEST_BIT_DEPTH_6			0x0
#define DP_TEST_BIT_DEPTH_8			0x1
#define DP_TEST_BIT_DEPTH_10			0x2
#define DP_TEST_BIT_DEPTH_12			0x3
#define DP_TEST_BIT_DEPTH_16			0x4

#define DP_TEST_DYNAMIC_RANGE_SHIFT             3
#define DP_TEST_DYNAMIC_RANGE_MASK		BIT(3)
#define DP_TEST_YCBCR_COEFF_SHIFT		4
#define DP_TEST_YCBCR_COEFF_MASK		BIT(4)

#define DP_TEST_DYNAMIC_RANGE_VESA		0x0
#define DP_TEST_DYNAMIC_RANGE_CEA               0x1

#pragma GCC diagnostic pop

#define DP_TEST_COLOR_FORMAT_RGB	        0x0
#define DP_TEST_COLOR_FORMAT_YCBCR422           0x2
#define DP_TEST_COLOR_FORMAT_YCBCR444		0x4
#define DP_TEST_YCBCR_COEFF_ITU601		0x0
#define DP_TEST_YCBCR_COEFF_ITU709		0x1

#define DP_TEST_AUDIO_MODE			0x271
#define DP_TEST_AUDIO_SAMPLING_RATE_MASK GENMASK(3, 0)
#define DP_TEST_AUDIO_CH_COUNT_SHIFT 4
#define DP_TEST_AUDIO_CH_COUNT_MASK GENMASK(7, 4)

#define DP_TEST_AUDIO_SAMPLING_RATE_32		0x0
#define DP_TEST_AUDIO_SAMPLING_RATE_44_1	0x1
#define DP_TEST_AUDIO_SAMPLING_RATE_48		0x2
#define DP_TEST_AUDIO_SAMPLING_RATE_88_2	0x3
#define DP_TEST_AUDIO_SAMPLING_RATE_96		0x4
#define DP_TEST_AUDIO_SAMPLING_RATE_176_4	0x5
#define DP_TEST_AUDIO_SAMPLING_RATE_192		0x6

#define DP_TEST_AUDIO_CHANNEL1			0x0
#define DP_TEST_AUDIO_CHANNEL2			0x1
#define DP_TEST_AUDIO_CHANNEL3			0x2
#define DP_TEST_AUDIO_CHANNEL4			0x3
#define DP_TEST_AUDIO_CHANNEL5			0x4
#define DP_TEST_AUDIO_CHANNEL6			0x5
#define DP_TEST_AUDIO_CHANNEL7			0x6
#define DP_TEST_AUDIO_CHANNEL8			0x7

#define DP_EDP_14A                         0x04    /* eDP 1.4a */
#define DP_EDP_14B                         0x05    /* eDP 1.4b */

#define DP_DS_MAX_BPC_MASK                 (3 << 0)
#define DP_DS_8BPC                         0
#define DP_DS_10BPC                        1
#define DP_DS_12BPC                        2
#define DP_DS_16BPC                        3

#define DP_FEC_CONFIGURATION 0x120
#define DP_FEC_STATUS               0x280
#define DP_FEC_ERROR_COUNT    0x281
#define DP_FEC_READY                 BIT(0)

#define DP_EXTENDED_RECEIVER_CAPABILITY_FIELD_PRESENT BIT(7)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
static inline bool
drm_dp_tps4_supported(const uint8_t dpcd[DP_RECEIVER_CAP_SIZE], int size)
{
	if (size <= DP_DPCD_REV || size <= DP_MAX_DOWNSPREAD)
		HISI_DRM_ERR("[DP] exceed max size of dpcd!");
	return dpcd[DP_DPCD_REV] >= 0x14 &&
		dpcd[DP_MAX_DOWNSPREAD] & DP_TPS4_SUPPORTED;
}
#endif
#endif

/*lint +e547*/

