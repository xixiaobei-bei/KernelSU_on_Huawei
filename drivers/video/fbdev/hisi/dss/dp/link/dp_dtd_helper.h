/* Copyright (c) 2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#ifndef __DP_DTD_HELPER_H__
#define __DP_DTD_HELPER_H__

#ifndef dtd_array_size
#define dtd_array_size(_A) (sizeof(_A) / sizeof((_A)[0]))
#endif /* dtd_array_size */

#define detailed_timing_descriptor(his, vis, ha, va, hb, vb, hso, vso, hspw, vspw, hp, vp, il, pc, pri) \
	.h_image_size = his, .v_image_size = vis, .h_active = ha, .v_active = va, .h_blanking = hb, .v_blanking = vb, \
	.h_sync_offset = hso, .v_sync_offset = vso, .h_sync_pulse_width = hspw, .v_sync_pulse_width = vspw, \
	.h_sync_polarity = hp, .v_sync_polarity = vp, .interlaced = il, .pixel_clock = pc, .pixel_repetition_input = pri

static const struct dtd cea_modes_dtd[] = {
	/* 0 - dummy, cea_code start at 1 */
	{ },
	/* 1 - 640x480p @ 59.94/60Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 640, 480, 160, 45, 16, 10, 96, 2, 0, 0, 0, 25175, 0) },
	/* 2 - 720x480p @ 59.94/60Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 720, 480, 138, 45, 16, 9, 62, 6, 0, 0, 0, 27000, 0) },
	/* 3 - 720x480p @ 59.94/60Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 720, 480, 138, 45, 16, 9, 62, 6, 0, 0, 0, 27000, 0) },
	/* 4 - 1280x720p @ 59.94/60Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1280, 720, 370, 30, 110, 5, 40, 5, 0, 0, 0, 74250, 0) },
	/* 5 - 1920x1080i @ 59.94/60Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1920, 540, 280, 22, 88, 2, 44, 5, 1, 1, 1, 74250, 0) },
	/* 6 - 720(1440)x480i @ 59.94/60Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1440, 240, 276, 22, 38, 4, 124, 3, 0, 0, 1, 27000, 0) },
	/* 7 - 720(1440)x480i @ 59.94/60Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1440, 240, 276, 22, 38, 4, 124, 3, 0, 0, 1, 27000, 0) },
	/* 8 - 720(1440)x240p @ 59.826/60.054/59.886/60.115Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1440, 240, 276, 23, 38, 5, 124, 3, 0, 0, 0, 27000, 0) },
	/* 9 - 720(1440)x240p @59.826/60.054/59.886/60.115Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1440, 240, 276, 23, 38, 5, 124, 3, 0, 0, 0, 27000, 0) },
	/* 10 - 2880x480i @ 59.94/60Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 2880, 240, 552, 22, 76, 4, 248, 3, 0, 0, 1, 54000, 0) },
	/* 11 - 2880x480i @ 59.94/60Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 2880, 240, 552, 22, 76, 4, 248, 3, 0, 0, 1, 54000, 0) },
	/* 12 - 2880x240p @ 59.826/60.054/59.886/60.115Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 2880, 240, 552, 23, 76, 5, 248, 3, 0, 0, 0, 54000, 0) },
	/* 13 - 2880x240p @ 59.826/60.054/59.886/60.115Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 2880, 240, 552, 23, 76, 5, 248, 3, 0, 0, 0, 54000, 0) },
	/* 14 - 1440x480p @ 59.94/60Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1440, 480, 276, 45, 32, 9, 124, 6, 0, 0, 0, 54000, 0) },
	/* 15 - 1440x480p @ 59.94/60Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1440, 480, 276, 45, 32, 9, 124, 6, 0, 0, 0, 54000, 0) },
	/* 16 - 1920x1080p @ 59.94/60Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1920, 1080, 280, 45, 88, 4, 44, 5, 0, 0, 0, 148500, 0) },
	/* 17 - 720x576p @ 50Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 720, 576, 144, 49, 12, 5, 64, 5, 0, 0, 0, 27000, 0) },
	/* 18 - 720x576p @ 50Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 720, 576, 144, 49, 12, 5, 64, 5, 0, 0, 0, 27000, 0) },
	/* 19 - 1280x720p @ 50Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1280, 720, 700, 30, 440, 5, 40, 5, 1, 1, 0, 74250, 0) },
	/* 20 - 1920x1080i @ 50Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1920, 540, 720, 22, 528, 2, 44, 5, 1, 1, 1, 74250, 0) },
	/* 21 - 720(1440)x576i @ 50Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1440, 288, 288, 24, 24, 2, 126, 3, 0, 0, 1, 27000, 0) },
	/* 22 - 720(1440)x576i @ 50Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1440, 288, 288, 24, 24, 2, 126, 3, 0, 0, 1, 27000, 0) },
	/* 23 - 720(1440)x288p @ 50Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1440, 288, 288, 26, 24, 4, 126, 3, 0, 0, 0, 27000, 0) },
	/* 24 - 720(1440)x288p @ 50Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1440, 288, 288, 26, 24, 4, 126, 3, 0, 0, 0, 27000, 0) },
	/* 25 - 2880x576i @ 50Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 2880, 288, 576, 24, 48, 2, 252, 3, 0, 0, 1, 54000, 0) },
	/* 26 - 2880x576i @ 50Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 2880, 288, 576, 24, 48, 2, 252, 3, 0, 0, 1, 54000, 0) },
	/* 27 - 2880x288p @ 50Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 2880, 288, 576, 26, 48, 4, 252, 3, 0, 0, 0, 54000, 0) },
	/* 28 - 2880x288p @ 50Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 2880, 288, 576, 26, 48, 4, 252, 3, 0, 0, 0, 54000, 0) },
	/* 29 - 1440x576p @ 50Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1440, 576, 288, 49, 24, 5, 128, 5, 0, 0, 0, 54000, 0) },
	/* 30 - 1440x576p @ 50Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1440, 576, 288, 49, 24, 5, 128, 5, 0, 0, 0, 54000, 0) },
	/* 31 - 1920x1080p @ 50Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1920, 1080, 720, 45, 528, 4, 44, 5, 1, 1, 0, 148500, 0) },
	/* 32 - 1920x1080p @ 23.976/24Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1920, 1080, 830, 45, 638, 4, 44, 5, 1, 1, 0, 74250, 0) },
	/* 33 - 1920x1080p @ 25Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1920, 1080, 720, 45, 528, 4, 44, 5, 1, 1, 0, 74250, 0) },
	/* 34 - 1920x1080p @ 29.97/30Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1920, 1080, 280, 45, 88, 4, 44, 5, 1, 1, 0, 74250, 0) },
	/* 35 - 2880x480p @ 60Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 2880, 480, 552, 45, 64, 9, 248, 6, 0, 0, 0, 108000, 0) },
	/* 36 - 2880x480p @ 60Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 2880, 480, 552, 45, 64, 9, 248, 6, 0, 0, 0, 108000, 0) },
	/* 37 - 2880x576p @ 50Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 2880, 576, 576, 49, 48, 5, 256, 5, 0, 0, 0, 108000, 0) },
	/* 38 - 2880x576p @ 50Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 2880, 576, 576, 49, 48, 5, 256, 5, 0, 0, 0, 108000, 0) },
	/* 39 - 1920x1080i (1250 total) @ 50Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1920, 540, 384, 85, 32, 23, 168, 5, 1, 1, 1, 72000, 0) },
	/* 40 - 1920x1080i @ 100Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1920, 540, 720, 22, 528, 2, 44, 5, 1, 1, 1, 148500, 0) },
	/* 41 - 1280x720p @ 100Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1280, 720, 700, 30, 440, 5, 40, 5, 1, 1, 0, 148500, 0) },
	/* 42 - 720x576p @ 100Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 720, 576, 144, 49, 12, 5, 64, 5, 0, 0, 0, 54000, 0) },
	/* 43 - 720x576p @ 100Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 720, 576, 144, 49, 12, 5, 64, 5, 0, 0, 0, 54000, 0) },
	/* 44 - 720(1440)x576i @ 100Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1440, 288, 288, 24, 24, 2, 126, 3, 0, 0, 1, 54000, 0) },
	/* 45 - 720(1440)x576i @ 100Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1440, 288, 288, 24, 24, 2, 126, 3, 0, 0, 1, 54000, 0) },
	/* 46 - 1920x1080i @ 119.88/120Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1920, 540, 288, 22, 88, 2, 44, 5, 1, 1, 1, 148500, 0) },
	/* 47 - 1280x720p @ 119.88/120Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1280, 720, 370, 30, 110, 5, 40, 5, 1, 1, 0, 148500, 0) },
	/* 48 - 720x480p @ 119.88/120Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 720, 480, 138, 45, 16, 9, 62, 6, 0, 0, 0, 54000, 0) },
	/* 49 - 720x480p @ 119.88/120Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 720, 480, 138, 45, 16, 9, 62, 6, 0, 0, 0, 54000, 0) },
	/* 50 - 720(1440)x480i @ 119.88/120Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1440, 240, 276, 22, 38, 4, 124, 3, 0, 0, 1, 54000, 0) },
	/* 51 - 720(1440)x480i @ 119.88/120Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1440, 240, 276, 22, 38, 4, 124, 3, 0, 0, 1, 54000, 0) },
	/* 52 - 720X576p @ 200Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 720, 576, 144, 49, 12, 5, 64, 5, 0, 0, 0, 108000, 0) },
	/* 53 - 720X576p @ 200Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 720, 576, 144, 49, 12, 5, 64, 5, 0, 0, 0, 108000, 0) },
	/* 54 - 720(1440)x576i @ 200Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1440, 288, 288, 24, 24, 2, 126, 3, 0, 0, 1, 108000, 0) },
	/* 55 - 720(1440)x576i @ 200Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1440, 288, 288, 24, 24, 2, 126, 3, 0, 0, 1, 108000, 0) },
	/* 56 - 720x480p @ 239.76/240Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 720, 480, 138, 45, 16, 9, 62, 6, 0, 0, 0, 108000, 0) },
	/* 57 - 720x480p @ 239.76/240Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 720, 480, 138, 45, 16, 9, 62, 6, 0, 0, 0, 108000, 0) },
	/* 58 - 720(1440)x480i @ 239.76/240Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1440, 240, 276, 22, 38, 4, 124, 3, 0, 0, 1, 108000, 0) },
	/* 59 - 720(1440)x480i @ 239.76/240Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1440, 240, 276, 22, 38, 4, 124, 3, 0, 0, 1, 108000, 0) },
	/* 60 - 1280x720p @ 23.97/24Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1280, 720, 2020, 30, 1760, 5, 40, 5, 1, 1, 0, 59400, 0) },
	/* 61 - 1280x720p @ 25Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1280, 720, 2680, 30, 2420, 5, 40, 5, 1, 1, 0, 74250, 0) },
	/* 62 - 1280x720p @ 29.97/30Hz  16:9 */
	{ detailed_timing_descriptor(16, 9, 1280, 720, 2020, 30, 1760, 5, 40, 5, 1, 1, 0, 74250, 0) },
	/* 63 - 1920x1080p @ 119.88/120Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1920, 1080, 280, 45, 88, 4, 44, 5, 1, 1, 0, 297000, 0) },
	/* 64 - 1920x1080p @ 100Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1920, 1080, 720, 45, 528, 4, 44, 5, 1, 1, 0, 297000, 0) },
	/* 65 - 1280x720p @ 23.97/24Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1280, 720, 2020, 30, 1760, 5, 40, 5, 1, 1, 0, 59400, 0) },
	/* 66 - 1280x720p @ 25Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1280, 720, 2680, 30, 2420, 5, 40, 5, 1, 1, 0, 74250, 0) },
	/* 67 - 1280x720p @ 29.97/30Hz  4:3 */
	{ detailed_timing_descriptor(4, 3, 1280, 720, 2020, 30, 1760, 5, 40, 5, 1, 1, 0, 74250, 0) },
	/* 68 - 1280x720p @ 50Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1280, 720, 700, 30, 440, 5, 40, 5, 1, 1, 0, 74250, 0) },
	/* 69 - 1280x720p @ 59.94/60Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1280, 720, 370, 30, 110, 5, 40, 5, 0, 0, 0, 74250, 0) },
	/* 70 - 1280x720p @ 100Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1280, 720, 700, 30, 440, 5, 40, 5, 1, 1, 0, 148500, 0) },
	/* 71 - 1280x720p @ 119.88/120Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1280, 720, 370, 30, 110, 5, 40, 5, 1, 1, 0, 148500, 0) },
	/* 72 - 1920x1080p @ 23.976/24Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1920, 1080, 830, 45, 638, 4, 44, 5, 1, 1, 0, 74250, 0) },
	/* 73 - 1920x1080p @ 25Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1920, 1080, 720, 45, 528, 4, 44, 5, 1, 1, 0, 74250, 0) },
	/* 74 - 1920x1080p @ 29.97/30Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1920, 1080, 280, 45, 88, 4, 44, 5, 1, 1, 0, 74250, 0) },
	/* 75 - 1920x1080p @ 50Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1920, 1080, 720, 45, 528, 4, 44, 5, 1, 1, 0, 148500, 0) },
	/* 76 - 1920x1080p @ 59.94/60Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1920, 1080, 280, 45, 88, 4, 44, 5, 0, 0, 0, 148500, 0) },
	/* 77 - 1920x1080p @ 100Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1920, 1080, 720, 45, 528, 4, 44, 5, 1, 1, 0, 297000, 0) },
	/* 78 - 1920x1080p @ 119.88/120Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 1920, 1080, 280, 45, 88, 4, 44, 5, 1, 1, 0, 297000, 0) },
	/* 79 - 1680x720p @ 23.98Hz/24Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1680, 720, 1620, 30, 1360, 5, 40, 5, 1, 1, 0, 59400, 0) },
	/* 80 - 1680x720p @ 25Hz 16:9*/
	{ detailed_timing_descriptor(16, 9, 1680, 720, 1488, 30, 1228, 5, 40, 5, 1, 1, 0, 59400, 0) },
	/* 81 - 1680x720p @ 29.97Hz/30Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1680, 720, 960, 30, 700, 5, 40, 5, 1, 1, 0, 59400, 0) },
	/* 82 - 1680x720p @ 50Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1680, 720, 520, 30, 260, 5, 40, 5, 1, 1, 0, 82500, 0) },
	/* 83 - 1680x720p @ 59.94Hz/60Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1680, 720, 520, 30, 260, 5, 40, 5, 1, 1, 0, 99000, 0) },
	/* 84 - 1680x720p @ 100Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1680, 720, 320, 105, 60, 5, 40, 5, 1, 1, 0, 165000, 0) },
	/* 85 - 1680x720p @ 119.88/120Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 1680, 720, 320, 105, 60, 5, 40, 5, 1, 1, 0, 198000, 0) },
	/* 86 - 2560x1080p @ 23.98Hz/24Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 2560, 1080, 1190, 20, 998, 4, 44, 5, 1, 1, 0, 99000, 0) },
	/* 87 - 2560x1080p @ 25Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 2560, 1080, 640, 45, 448, 4, 44, 5, 1, 1, 0, 90000, 0) },
	/* 88 - 2560x1080p @ 29.97Hz/30Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 2560, 1080, 960, 45, 768, 4, 44, 5, 1, 1, 0, 118800, 0) },
	/* 89 - 2560x1080p @ 50Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 2560, 1080, 740, 45, 548, 4, 44, 5, 1, 1, 0, 185625, 0) },
	/* 90 - 2560x1080p @ 59.94Hz/60Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 2560, 1080, 440, 20, 248, 4, 44, 5, 1, 1, 0, 198000, 0) },
	/* 91 - 2560x1080p @ 100Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 2560, 1080, 410, 170, 218, 4, 44, 5, 1, 1, 0, 371250, 0) },
	/* 92 - 2560x1080p @ 119.88/120Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 2560, 1080, 740, 170, 548, 4, 44, 5, 1, 1, 0, 495000, 0) },
	/* 93 - 3840x2160p @  23.98Hz/24Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 3840, 2160, 1660, 90, 1276, 8, 88, 10, 1, 1, 0, 297000, 0) },
	/* 94 - 3840x2160p @  25Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 3840, 2160, 1440, 90, 1056, 8, 88, 10, 1, 1, 0, 297000, 0) },
	/* 95 - 3840x2160p @  29.97Hz/30Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 3840, 2160, 560, 90, 176, 8, 88, 10, 0, 0, 0, 297000, 0) },
	/* 96 - 3840x2160p @  50Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 3840, 2160, 1440, 90, 1056, 8, 88, 10, 1, 1, 0, 594000, 0) },
	/* 97 - 3840x2160p @  59.94Hz/60Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 3840, 2160, 560, 90, 176, 8, 88, 10, 0, 0, 0, 594000, 0) },
	/* 98 - 4096x2160p @ 23.98Hz/24Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 4096, 2160, 1404, 90, 1020, 8, 88, 10, 1, 1, 0, 297000, 0) },
	/* 99 - 4096x2160p @ 25Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 4096, 2160, 1184, 90, 968, 8, 88, 10, 1, 1, 0, 297000, 0) },
	/* 100 - 4096x2160p @ 29.97Hz/30Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 4096, 2160, 304, 90, 88, 8, 88, 10, 1, 1, 0, 297000, 0) },
	/* 101 - 4096x2160p @ 50Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 4096, 2160, 1184, 90, 968, 8, 88, 10, 1, 1, 0, 594000, 0) },
	/* 102 - 4096x2160p @ 59.94Hz/60Hz 16:9 */
	{ detailed_timing_descriptor(16, 9, 4096, 2160, 304, 90, 88, 8, 88, 10, 1, 1, 0, 594000, 0) },
	/* 103 - 3840x2160p @  23.98Hz/24Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 3840, 2160, 1660, 90, 1276, 8, 88, 10, 1, 1, 0, 297000, 0) },
	/* 104 - 3840x2160p @  25Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 3840, 2160, 1440, 90, 1056, 8, 88, 10, 1, 1, 0, 297000, 0) },
	/* 105 - 3840x2160p @  29.97Hz/30Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 3840, 2160, 560, 90, 176, 8, 88, 10, 0, 0, 0, 297000, 0) },
	/* 106 - 3840x2160p @  50Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 3840, 2160, 1440, 90, 1056, 8, 88, 10, 1, 1, 0, 594000, 0) },
	/* 107 - 3840x2160p @  59.94Hz/60Hz 4:3 */
	{ detailed_timing_descriptor(4, 3, 3840, 2160, 560, 90, 176, 8, 88, 10, 0, 0, 0, 594000, 0) },
};

static const struct dtd cvt_modes_dtd[] = {
	/* 0 - dummy, cvt_code start at 1 */
	{ },
	/* 1 - */
	{ detailed_timing_descriptor(4, 3, 640, 480, 160, 20, 8, 1, 32, 8, 1, 0, 0, 23750, 0) },
	/* 2 - */
	{ detailed_timing_descriptor(4, 3, 800, 600, 224, 24, 31, 3, 81, 4, 1, 0, 0, 38250, 0) },
	/* 3 - */
	{ detailed_timing_descriptor(4, 3, 1024, 768, 304, 30, 48, 3, 104, 4, 0, 1, 0, 63500, 0) },
	/* 4 - */
	{ detailed_timing_descriptor(4, 3, 1280, 960, 416, 36, 80, 3, 128, 4, 0, 1, 0, 101250, 0) },
	/* 5 - */
	{ detailed_timing_descriptor(4, 3, 1400, 1050, 464, 39, 88, 3, 144, 4, 0, 1, 0, 121750, 0) },
	/* 6 - */
	{ detailed_timing_descriptor(4, 3, 1600, 1200, 560, 45, 112, 3, 68, 4, 0, 1, 0, 161000, 0) },
	/* 7 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 8 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 9 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 10 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 11 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 12 - */
	{ detailed_timing_descriptor(4, 3, 1280, 1024, 432, 39, 80, 3, 136, 7, 0, 1, 0, 109000, 0) },
	/* 13 - */
	{ detailed_timing_descriptor(4, 3, 1280, 768, 384, 30, 64, 3, 128, 7, 0, 1, 0, 79500, 0) },
	/* 14 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 15 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 16 - */
	{ detailed_timing_descriptor(4, 3, 1280, 720, 384, 28, 64, 3, 128, 5, 0, 1, 0, 74500, 0) },
	/* 17 - */
	{ detailed_timing_descriptor(4, 3, 1360, 768, 416, 30, 72, 3, 136, 5, 0, 1, 0, 84750, 0) },
	/* 18 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 19 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 20 - */
	{ detailed_timing_descriptor(4, 3, 1920, 1080, 656, 40, 128, 3, 200, 5, 0, 1, 0, 173000, 0) },
	/* 21 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 22 - */
	{ detailed_timing_descriptor(4, 3, 2560, 1440, 928, 53, 192, 3, 272, 5, 0, 1, 0, 312250, 0) },
	/* 23 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 24 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 25 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 26 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 27 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 28 - */
	{ detailed_timing_descriptor(4, 3, 1280, 800, 400, 31, 72, 3, 128, 6, 0, 1, 0, 83500, 0) },
	/* 29 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 30 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 31 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 32 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 33 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 34 - */
	{ detailed_timing_descriptor(4, 3, 1920, 1200, 672, 45, 136, 3, 200, 6, 0, 1, 0, 193250, 0) },
	/* 35 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 36 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 37 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 38 - */
	{ detailed_timing_descriptor(4, 3, 3840, 2400, 80, 69, 320, 3, 424, 6, 0, 1, 0, 580128, 0) },
	/* 39 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 40 - */
	{ detailed_timing_descriptor(4, 3, 1600, 1200, 160, 35, 48, 3, 32, 4, 1, 0, 0, 124076, 0) },
	/* 41 - */
	{ detailed_timing_descriptor(4, 3, 2048, 1536, 160, 44, 48, 3, 32, 4, 1, 0, 0, 208000, 0) },
};

static const struct dtd dmt_modes_dtd[] = {
	/* 0 - dummy, dmt_code start at 1 */
	{ },
	/* 1 - HISilicon timing */
	{ detailed_timing_descriptor(4, 3, 3600, 1800, 120, 128, 20, 2, 20, 2, 0, 0, 0, 645500, 0) },
	/* 2 - */
	{ detailed_timing_descriptor(4, 3, 3840, 2160, 160, 62, 48, 3, 32, 5, 0, 0, 0, 533000, 0) },
	/* 3 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 4 - */
	{ detailed_timing_descriptor(4, 3, 640, 480, 144, 29, 8, 2, 96, 2, 0, 0, 0, 25175, 0) },
	/* 5 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 6 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 7 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 8 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 9 - */
	{ detailed_timing_descriptor(4, 3, 800, 600, 256, 28, 40, 1, 128, 4, 1, 1, 0, 40000, 0) },
	/* 10 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 11 - */
	{ detailed_timing_descriptor(4, 3, 800, 600, 256, 25, 16, 1, 80, 3, 1, 1, 0, 49500, 0) },
	/* 12 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 13 - */
	{ detailed_timing_descriptor(4, 3, 800, 600, 160, 36, 48, 3, 32, 4, 1, 0, 0, 73250, 0) },
	/* 14 - 848x480p@60Hz */
	{ detailed_timing_descriptor(4, 3, 848, 480, 240, 37, 16, 6, 112, 8, 1, 1, 0, 33750, 0) },
	/* 15 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 16 - */
	{ detailed_timing_descriptor(4, 3, 1024, 768, 320, 38, 24, 3, 136, 6, 0, 0, 0, 65000, 0) },
	/* 17 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 18 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 19 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 20 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 21 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 22 - */
	{ detailed_timing_descriptor(4, 3, 1280, 768, 160, 22, 48, 3, 32, 7, 1, 0, 0, 68250, 0) },
	/* 23 - */
	{ detailed_timing_descriptor(4, 3, 1280, 768, 384, 30, 64, 3, 128, 7, 0, 1, 0, 79500, 0) },
	/* 24 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 25 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 26 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 27 - */
	{ detailed_timing_descriptor(4, 3, 1280, 800, 160, 23, 48, 3, 32, 6, 1, 0, 0, 71000, 0) },
	/* 28 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 29 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 30 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 31 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 32 - */
	{ detailed_timing_descriptor(4, 3, 1280, 960, 520, 40, 96, 1, 112, 3, 1, 1, 0, 108000, 0) },
	/* 33 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 34 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 35 - */
	{ detailed_timing_descriptor(4, 3, 1280, 1024, 408, 42, 48, 1, 112, 3, 1, 1, 0, 108000, 0) },
	/* 36 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 37 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 38 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 39 - */
	{ detailed_timing_descriptor(4, 3, 1360, 768, 432, 27, 64, 3, 112, 6, 1, 1, 0, 85500, 0) },
	/* 40 - */
	{ detailed_timing_descriptor(4, 3, 1360, 768, 160, 45, 48, 3, 32, 5, 1, 0, 0, 148250, 0) },
	/* 41 - */
	{ detailed_timing_descriptor(4, 3, 1400, 1050, 160, 30, 48, 3, 32, 4, 1, 0, 0, 101000, 0) },
	/* 42 - */
	{ detailed_timing_descriptor(4, 3, 1400, 1050, 464, 39, 88, 3, 144, 4, 0, 1, 0, 121750, 0) },
	/* 43 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 44 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 45 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 46 - */
	{ detailed_timing_descriptor(4, 3, 1440, 900, 160, 26, 48, 3, 32, 6, 1, 0, 0, 88750, 0) },
	/* 47 - */
	{ detailed_timing_descriptor(4, 3, 1440, 900, 464, 34, 80, 3, 152, 6, 0, 1, 0, 106500, 0) },
	/* 48 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 49 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 50 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 51 - */
	{ detailed_timing_descriptor(4, 3, 1600, 1200, 560, 50, 64, 1, 192, 3, 1, 1, 0, 162000, 0) },
	/* 52 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 53 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 54 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 55 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 56 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 57 - */
	{ detailed_timing_descriptor(4, 3, 1680, 1050, 160, 30, 48, 3, 32, 6, 1, 0, 0, 119000, 0) },
	/* 58 - */
	{ detailed_timing_descriptor(4, 3, 1680, 1050, 560, 39, 104, 3, 176, 6, 0, 1, 0, 146250, 0) },
	/* 59 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 60 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 61 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 62 - */
	{ detailed_timing_descriptor(4, 3, 1792, 1344, 656, 50, 128, 1, 200, 3, 0, 1, 0, 204750, 0) },
	/* 63 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 64 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 65 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 66 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 67 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 68 - */
	{ detailed_timing_descriptor(4, 3, 1920, 1200, 160, 35, 48, 3, 32, 6, 1, 0, 0, 154000, 0) },
	/* 69 - */
	{ detailed_timing_descriptor(4, 3, 1920, 1200, 672, 45, 136, 3, 200, 6, 0, 1, 0, 193250, 0) },
	/* 70 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 71 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 72 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 73 - */
	{ detailed_timing_descriptor(4, 3, 1920, 1440, 680, 60, 128, 1, 208, 3, 0, 1, 0, 234000, 0) },
	/* 74 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 75 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 76 - */
	{ detailed_timing_descriptor(4, 3, 2560, 1600, 160, 46, 48, 3, 32, 6, 1, 0, 0, 193250, 0) },
	/* 77 - */
	{ detailed_timing_descriptor(4, 3, 2560, 1600, 940, 58, 192, 3, 280, 6, 0, 1, 0, 193250, 0) },
	/* 78 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 79 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 80 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 81 - */
	{ detailed_timing_descriptor(4, 3, 1366, 768, 426, 30, 70, 3, 142, 3, 1, 1, 0, 85500, 0) },
	/* 82 - */
	{ detailed_timing_descriptor(4, 3, 1920, 1080, 280, 45, 88, 4, 44, 5, 1, 1, 0, 148500, 0) },
	/* 83 - */
	{ detailed_timing_descriptor(4, 3, 1600, 900, 200, 100, 24, 1, 80, 3, 1, 1, 0, 108000, 0) },
	/* 84 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 85 - Details to be added. */
	{ detailed_timing_descriptor(4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
	/* 86 - */
	{ detailed_timing_descriptor(4, 3, 1366, 768, 134, 32, 14, 1, 56, 3, 1, 1, 0, 72000, 0) },
	/* 87 - */
	{ detailed_timing_descriptor(4, 3, 4096, 2160, 80, 62, 8, 48, 32, 8, 1, 0, 0, 556744, 0) },
	/* 88 - */
	{ detailed_timing_descriptor(4, 3, 4096, 2160, 80, 62, 8, 48, 32, 8, 1, 0, 0, 556188, 0) },
};

#endif /* DP_DTD_HELPER_H */
