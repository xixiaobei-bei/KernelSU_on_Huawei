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

#ifndef __DRM_DP_HELPER_ADDITIONS_H__
#define __DRM_DP_HELPER_ADDITIONS_H__
#include <linux/version.h>

/*
 * The following aren't yet defined in kernel headers
 */
#define DP_LINK_BW_8_1				0x1e
#define DP_TRAINING_PATTERN_4			7
#define DP_TPS4_SUPPORTED			BIT(7)

#define DP_PSR2_WITH_Y_COORD_IS_SUPPORTED  3      /* eDP 1.4a */

#define DP_DSC_SUPPORT                      0x060   /* DP 1.4 */
#define DP_DSC_DECOMPRESSION_IS_SUPPORTED  (1 << 0)

#define DP_DSC_REV                          0x061
#define DP_DSC_MAJOR_MASK                  (0xf << 0)
#define DP_DSC_MINOR_MASK                  (0xf << 4)
#define DP_DSC_MAJOR_SHIFT                 0
#define DP_DSC_MINOR_SHIFT                 4

#define DP_DSC_RC_BUF_BLK_SIZE              0x062
#define DP_DSC_RC_BUF_BLK_SIZE_1           0x0
#define DP_DSC_RC_BUF_BLK_SIZE_4           0x1
#define DP_DSC_RC_BUF_BLK_SIZE_16          0x2
#define DP_DSC_RC_BUF_BLK_SIZE_64          0x3

#define DP_DSC_RC_BUF_SIZE                  0x063

#define DP_DSC_SLICE_CAP_1                  0x064
#define DP_DSC_1_PER_DP_DSC_SINK           (1 << 0)
#define DP_DSC_2_PER_DP_DSC_SINK           (1 << 1)
#define DP_DSC_4_PER_DP_DSC_SINK           (1 << 3)
#define DP_DSC_6_PER_DP_DSC_SINK           (1 << 4)
#define DP_DSC_8_PER_DP_DSC_SINK           (1 << 5)
#define DP_DSC_10_PER_DP_DSC_SINK          (1 << 6)
#define DP_DSC_12_PER_DP_DSC_SINK          (1 << 7)

#define DP_DSC_LINE_BUF_BIT_DEPTH           0x065
#define DP_DSC_LINE_BUF_BIT_DEPTH_MASK     (0xf << 0)
#define DP_DSC_LINE_BUF_BIT_DEPTH_9        0x0
#define DP_DSC_LINE_BUF_BIT_DEPTH_10       0x1
#define DP_DSC_LINE_BUF_BIT_DEPTH_11       0x2
#define DP_DSC_LINE_BUF_BIT_DEPTH_12       0x3
#define DP_DSC_LINE_BUF_BIT_DEPTH_13       0x4
#define DP_DSC_LINE_BUF_BIT_DEPTH_14       0x5
#define DP_DSC_LINE_BUF_BIT_DEPTH_15       0x6
#define DP_DSC_LINE_BUF_BIT_DEPTH_16       0x7
#define DP_DSC_LINE_BUF_BIT_DEPTH_8        0x8

#define DP_DSC_BLK_PREDICTION_SUPPORT       0x066
#define DP_DSC_BLK_PREDICTION_IS_SUPPORTED (1 << 0)

#define DP_DSC_MAX_BITS_PER_PIXEL_LOW       0x067   /* eDP 1.4 */

#define DP_DSC_MAX_BITS_PER_PIXEL_HI        0x068   /* eDP 1.4 */

#define DP_DSC_DEC_COLOR_FORMAT_CAP         0x069
#define DP_DSC_RGB                         (1 << 0)
#define DP_DSC_YCBCR_444                    (1 << 1)
#define DP_DSC_YCBCR_422_SIMPLE             (1 << 2)
#define DP_DSC_YCBCR_422_NATIVE             (1 << 3)
#define DP_DSC_YCBCR_420_NATIVE             (1 << 4)

#define DP_DSC_DEC_COLOR_DEPTH_CAP          0x06A
#define DP_DSC_8_BPC                       (1 << 1)
#define DP_DSC_10_BPC                      (1 << 2)
#define DP_DSC_12_BPC                      (1 << 3)

#define DP_DSC_PEAK_THROUGHPUT              0x06B
#define DP_DSC_THROUGHPUT_MODE_0_MASK      (0xf << 0)
#define DP_DSC_THROUGHPUT_MODE_0_SHIFT     0
#define DP_DSC_THROUGHPUT_MODE_0_340       (1 << 0)
#define DP_DSC_THROUGHPUT_MODE_0_400       (2 << 0)
#define DP_DSC_THROUGHPUT_MODE_0_450       (3 << 0)
#define DP_DSC_THROUGHPUT_MODE_0_500       (4 << 0)
#define DP_DSC_THROUGHPUT_MODE_0_550       (5 << 0)
#define DP_DSC_THROUGHPUT_MODE_0_600       (6 << 0)
#define DP_DSC_THROUGHPUT_MODE_0_650       (7 << 0)
#define DP_DSC_THROUGHPUT_MODE_0_700       (8 << 0)
#define DP_DSC_THROUGHPUT_MODE_0_750       (9 << 0)
#define DP_DSC_THROUGHPUT_MODE_0_800       (10 << 0)
#define DP_DSC_THROUGHPUT_MODE_0_850       (11 << 0)
#define DP_DSC_THROUGHPUT_MODE_0_900       (12 << 0)
#define DP_DSC_THROUGHPUT_MODE_0_950       (13 << 0)
#define DP_DSC_THROUGHPUT_MODE_0_1000      (14 << 0)
#define DP_DSC_THROUGHPUT_MODE_1_MASK      (0xf << 4)
#define DP_DSC_THROUGHPUT_MODE_1_SHIFT     4
#define DP_DSC_THROUGHPUT_MODE_1_340       (1 << 4)
#define DP_DSC_THROUGHPUT_MODE_1_400       (2 << 4)
#define DP_DSC_THROUGHPUT_MODE_1_450       (3 << 4)
#define DP_DSC_THROUGHPUT_MODE_1_500       (4 << 4)
#define DP_DSC_THROUGHPUT_MODE_1_550       (5 << 4)
#define DP_DSC_THROUGHPUT_MODE_1_600       (6 << 4)
#define DP_DSC_THROUGHPUT_MODE_1_650       (7 << 4)
#define DP_DSC_THROUGHPUT_MODE_1_700       (8 << 4)
#define DP_DSC_THROUGHPUT_MODE_1_750       (9 << 4)
#define DP_DSC_THROUGHPUT_MODE_1_800       (10 << 4)
#define DP_DSC_THROUGHPUT_MODE_1_850       (11 << 4)
#define DP_DSC_THROUGHPUT_MODE_1_900       (12 << 4)
#define DP_DSC_THROUGHPUT_MODE_1_950       (13 << 4)
#define DP_DSC_THROUGHPUT_MODE_1_1000      (14 << 4)

#define DP_DSC_MAX_SLICE_WIDTH              0x06C

#define DP_DSC_SLICE_CAP_2                  0x06D
#define DP_DSC_16_PER_DP_DSC_SINK          (1 << 0)
#define DP_DSC_20_PER_DP_DSC_SINK          (1 << 1)
#define DP_DSC_24_PER_DP_DSC_SINK          (1 << 2)

#define DP_DSC_BITS_PER_PIXEL_INC           0x06F
#define DP_DSC_BITS_PER_PIXEL_1_16         0x0
#define DP_DSC_BITS_PER_PIXEL_1_8          0x1
#define DP_DSC_BITS_PER_PIXEL_1_4          0x2
#define DP_DSC_BITS_PER_PIXEL_1_2          0x3
#define DP_DSC_BITS_PER_PIXEL_1            0x4

#define DP_DSC_ENABLE            0x160   /* DP 1.4 */
#define DP_DSC_DECOMPRESSION_IS_ENABLED  (1 << 0)

#define DP_PSR_SUPPORT                      0x070   /* XXX 1.2? */
#define DP_PSR_IS_SUPPORTED                1
#define DP_PSR2_IS_SUPPORTED              2       /* eDP 1.4 */

#define DP_PSR_CAPS                         0x071   /* XXX 1.2? */
#define DP_PSR_NO_TRAIN_ON_EXIT            1
#define DP_PSR_SETUP_TIME_330              (0 << 1)
#define DP_PSR_SETUP_TIME_275              (1 << 1)
#define DP_PSR_SETUP_TIME_220              (2 << 1)
#define DP_PSR_SETUP_TIME_165              (3 << 1)
#define DP_PSR_SETUP_TIME_110              (4 << 1)
#define DP_PSR_SETUP_TIME_55               (5 << 1)
#define DP_PSR_SETUP_TIME_0                (6 << 1)
#define DP_PSR_NO_TRAIN_ON_EXIT            1
#define DP_PSR_SETUP_TIME_330              (0 << 1)
#define DP_PSR_SETUP_TIME_275              (1 << 1)
#define DP_PSR_SETUP_TIME_220              (2 << 1)
#define DP_PSR_SETUP_TIME_165              (3 << 1)
#define DP_PSR_SETUP_TIME_110              (4 << 1)
#define DP_PSR_SETUP_TIME_55               (5 << 1)
#define DP_PSR_SETUP_TIME_0                (6 << 1)
#define DP_PSR_SETUP_TIME_MASK             (7 << 1)
#define DP_PSR_SETUP_TIME_SHIFT            1
#define DP_PSR2_SU_Y_COORDINATE_REQUIRED   (1 << 4)  /* eDP 1.4a */
#define DP_PSR2_SU_GRANULARITY_REQUIRED    (1 << 5)  /* eDP 1.4b */

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

static inline bool
drm_dp_tps4_supported(const uint8_t dpcd[DP_RECEIVER_CAP_SIZE])
{
	return dpcd[DP_DPCD_REV] >= 0x14 &&
		dpcd[DP_MAX_DOWNSPREAD] & DP_TPS4_SUPPORTED;
}

static inline bool
drm_dp_tps3_supported(const uint8_t dpcd[DP_RECEIVER_CAP_SIZE])
{
         return dpcd[DP_DPCD_REV] >= 0x12 &&
                 dpcd[DP_MAX_LANE_COUNT] & DP_TPS3_SUPPORTED;
}
#define DP_FEC_CONFIGURATION 0x120
#define DP_FEC_STATUS               0x280
#define DP_FEC_ERROR_COUNT    0x281
#define DP_FEC_READY                 BIT(0)

#define DP_EXTENDED_RECEIVER_CAPABILITY_FIELD_PRESENT BIT(7)
#endif

