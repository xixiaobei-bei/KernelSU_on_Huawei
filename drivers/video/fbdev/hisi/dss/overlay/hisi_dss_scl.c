/* Copyright (c) 2013-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#include "hisi_overlay_utils.h"
#include "../hisi_display_effect.h"
#include "../hisi_dpe_utils.h"
#include "../hisi_ovl_online_wb.h"
#include "../hisi_mmbuf_manager.h"
#include "../hisi_spr_dsc.h"
#include "../hisi_frame_rate_ctrl.h"

struct zoom_ratio {
	uint32_t h_ratio;
	bool     en_hscl;
	uint32_t acc_hscl;
	uint32_t v_ratio;
	bool     en_vscl;
	uint32_t h_v_order;
	uint32_t acc_vscl;
	uint32_t scf_en_vscl;
};

/* SCF_LUT_CHN coef_idx */
int g_scf_lut_chn_coef_idx[DSS_CHN_MAX_DEFINE] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

const int coef_lut_tap4[SCL_COEF_IDX_MAX][PHASE_NUM][TAP4] = {
	/* YUV_COEF_IDX */
	{
		{ 214, 599, 214, -3 },
		{ 207, 597, 223, -3 },
		{ 200, 596, 231, -3 },
		{ 193, 596, 238, -3 },
		{ 186, 595, 246, -3 },
		{ 178, 594, 255, -3 },
		{ 171, 593, 263, -3 },
		{ 165, 591, 271, -3 },
		{ 158, 589, 279, -2 },
		{ 151, 587, 288, -2 },
		{ 145, 584, 296, -1 },
		{ 139, 582, 304, -1 },
		{ 133, 578, 312, 1 },
		{ 127, 575, 321, 1 },
		{ 121, 572, 329, 2 },
		{ 115, 568, 337, 4 },
		{ 109, 564, 346, 5 },
		{ 104, 560, 354, 6 },
		{ 98, 555, 362, 9 },
		{ 94, 550, 370, 10 },
		{ 88, 546, 379, 11 },
		{ 84, 540, 387, 13 },
		{ 79, 535, 395, 15 },
		{ 74, 530, 403, 17 },
		{ 70, 524, 411, 19 },
		{ 66, 518, 419, 21 },
		{ 62, 512, 427, 23 },
		{ 57, 506, 435, 26 },
		{ 54, 499, 443, 28 },
		{ 50, 492, 451, 31 },
		{ 47, 486, 457, 34 },
		{ 43, 479, 465, 37 },
		{ 40, 472, 472, 40 },
		{ 214, 599, 214, -3 },
		{ 207, 597, 223, -3 },
		{ 200, 596, 231, -3 },
		{ 193, 596, 238, -3 },
		{ 186, 595, 246, -3 },
		{ 178, 594, 255, -3 },
		{ 171, 593, 263, -3 },
		{ 165, 591, 271, -3 },
		{ 158, 589, 279, -2 },
		{ 151, 587, 288, -2 },
		{ 145, 584, 296, -1 },
		{ 139, 582, 304, -1 },
		{ 133, 578, 312, 1 },
		{ 127, 575, 321, 1 },
		{ 121, 572, 329, 2 },
		{ 115, 568, 337, 4 },
		{ 109, 564, 346, 5 },
		{ 104, 560, 354, 6 },
		{ 98, 555, 362, 9 },
		{ 94, 550, 370, 10 },
		{ 88, 546, 379, 11 },
		{ 84, 540, 387, 13 },
		{ 79, 535, 395, 15 },
		{ 74, 530, 403, 17 },
		{ 70, 524, 411, 19 },
		{ 66, 518, 419, 21 },
		{ 62, 512, 427, 23 },
		{ 57, 506, 435, 26 },
		{ 54, 499, 443, 28 },
		{ 50, 492, 451, 31 },
		{ 47, 486, 457, 34 },
		{ 43, 479, 465, 37 },
		{ 40, 472, 472, 40 }
	},

	/* RGB_COEF_IDX */
	{
		{ 0, 1024,   0, 0 },
		{ 0, 1008,  16, 0 },
		{ 0,  992,  32, 0 },
		{ 0,  976,  48, 0 },
		{ 0,  960,  64, 0 },
		{ 0,  944,  80, 0 },
		{ 0,  928,  96, 0 },
		{ 0,  912, 112, 0 },
		{ 0,  896, 128, 0 },
		{ 0,  880, 144, 0 },
		{ 0,  864, 160, 0 },
		{ 0,  848, 176, 0 },
		{ 0,  832, 192, 0 },
		{ 0,  816, 208, 0 },
		{ 0,  800, 224, 0 },
		{ 0,  784, 240, 0 },
		{ 0,  768, 256, 0 },
		{ 0,  752, 272, 0 },
		{ 0,  736, 288, 0 },
		{ 0,  720, 304, 0 },
		{ 0,  704, 320, 0 },
		{ 0,  688, 336, 0 },
		{ 0,  672, 352, 0 },
		{ 0,  656, 368, 0 },
		{ 0,  640, 384, 0 },
		{ 0,  624, 400, 0 },
		{ 0,  608, 416, 0 },
		{ 0,  592, 432, 0 },
		{ 0,  576, 448, 0 },
		{ 0,  560, 464, 0 },
		{ 0,  544, 480, 0 },
		{ 0,  528, 496, 0 },
		{ 0,  512, 512, 0 },
		{ 0, 1024,   0, 0 },
		{ 0, 1008,  16, 0 },
		{ 0,  992,  32, 0 },
		{ 0,  976,  48, 0 },
		{ 0,  960,  64, 0 },
		{ 0,  944,  80, 0 },
		{ 0,  928,  96, 0 },
		{ 0,  912, 112, 0 },
		{ 0,  896, 128, 0 },
		{ 0,  880, 144, 0 },
		{ 0,  864, 160, 0 },
		{ 0,  848, 176, 0 },
		{ 0,  832, 192, 0 },
		{ 0,  816, 208, 0 },
		{ 0,  800, 224, 0 },
		{ 0,  784, 240, 0 },
		{ 0,  768, 256, 0 },
		{ 0,  752, 272, 0 },
		{ 0,  736, 288, 0 },
		{ 0,  720, 304, 0 },
		{ 0,  704, 320, 0 },
		{ 0,  688, 336, 0 },
		{ 0,  672, 352, 0 },
		{ 0,  656, 368, 0 },
		{ 0,  640, 384, 0 },
		{ 0,  624, 400, 0 },
		{ 0,  608, 416, 0 },
		{ 0,  592, 432, 0 },
		{ 0,  576, 448, 0 },
		{ 0,  560, 464, 0 },
		{ 0,  544, 480, 0 },
		{ 0,  528, 496, 0 },
		{ 0,  512, 512, 0 }
	}
};

const int g_coef_lut_v1_tap4[SCL_COEF_IDX_MAX][PHASE_NUM][TAP4] = {
	/* YUV_COEF_IDX */
	{
		{ 214, 599, 214, -3 },
		{ 207, 597, 223, -3 },
		{ 200, 596, 231, -3 },
		{ 193, 596, 238, -3 },
		{ 186, 595, 246, -3 },
		{ 178, 594, 255, -3 },
		{ 171, 593, 263, -3 },
		{ 165, 591, 271, -3 },
		{ 158, 589, 279, -2 },
		{ 151, 587, 288, -2 },
		{ 145, 584, 296, -1 },
		{ 139, 582, 304, -1 },
		{ 133, 578, 312, 1 },
		{ 127, 575, 321, 1 },
		{ 121, 572, 329, 2 },
		{ 115, 568, 337, 4 },
		{ 109, 564, 346, 5 },
		{ 104, 560, 354, 6 },
		{ 98, 555, 362, 9 },
		{ 94, 550, 370, 10 },
		{ 88, 546, 379, 11 },
		{ 84, 540, 387, 13 },
		{ 79, 535, 395, 15 },
		{ 74, 530, 403, 17 },
		{ 70, 524, 411, 19 },
		{ 66, 518, 419, 21 },
		{ 62, 512, 427, 23 },
		{ 57, 506, 435, 26 },
		{ 54, 499, 443, 28 },
		{ 50, 492, 451, 31 },
		{ 47, 486, 457, 34 },
		{ 43, 479, 465, 37 },
		{ 40, 472, 472, 40 },
		{ 214, 599, 214, -3 },
		{ 207, 597, 223, -3 },
		{ 200, 596, 231, -3 },
		{ 193, 596, 238, -3 },
		{ 186, 595, 246, -3 },
		{ 178, 594, 255, -3 },
		{ 171, 593, 263, -3 },
		{ 165, 591, 271, -3 },
		{ 158, 589, 279, -2 },
		{ 151, 587, 288, -2 },
		{ 145, 584, 296, -1 },
		{ 139, 582, 304, -1 },
		{ 133, 578, 312, 1 },
		{ 127, 575, 321, 1 },
		{ 121, 572, 329, 2 },
		{ 115, 568, 337, 4 },
		{ 109, 564, 346, 5 },
		{ 104, 560, 354, 6 },
		{ 98, 555, 362, 9 },
		{ 94, 550, 370, 10 },
		{ 88, 546, 379, 11 },
		{ 84, 540, 387, 13 },
		{ 79, 535, 395, 15 },
		{ 74, 530, 403, 17 },
		{ 70, 524, 411, 19 },
		{ 66, 518, 419, 21 },
		{ 62, 512, 427, 23 },
		{ 57, 506, 435, 26 },
		{ 54, 499, 443, 28 },
		{ 50, 492, 451, 31 },
		{ 47, 486, 457, 34 },
		{ 43, 479, 465, 37 },
		{ 40, 472, 472, 40 }
	},

	/* RGB_COEF_IDX */
	{
		{ 0, 1024,   0, 0 },
		{ 0, 1008,  16, 0 },
		{ 0,  992,  32, 0 },
		{ 0,  976,  48, 0 },
		{ 0,  960,  64, 0 },
		{ 0,  944,  80, 0 },
		{ 0,  928,  96, 0 },
		{ 0,  912, 112, 0 },
		{ 0,  896, 128, 0 },
		{ 0,  880, 144, 0 },
		{ 0,  864, 160, 0 },
		{ 0,  848, 176, 0 },
		{ 0,  832, 192, 0 },
		{ 0,  816, 208, 0 },
		{ 0,  800, 224, 0 },
		{ 0,  784, 240, 0 },
		{ 0,  768, 256, 0 },
		{ 0,  752, 272, 0 },
		{ 0,  736, 288, 0 },
		{ 0,  720, 304, 0 },
		{ 0,  704, 320, 0 },
		{ 0,  688, 336, 0 },
		{ 0,  672, 352, 0 },
		{ 0,  656, 368, 0 },
		{ 0,  640, 384, 0 },
		{ 0,  624, 400, 0 },
		{ 0,  608, 416, 0 },
		{ 0,  592, 432, 0 },
		{ 0,  576, 448, 0 },
		{ 0,  560, 464, 0 },
		{ 0,  544, 480, 0 },
		{ 0,  528, 496, 0 },
		{ 0,  512, 512, 0 },
		{ 0, 1024,   0, 0 },
		{ 0, 1008,  16, 0 },
		{ 0,  992,  32, 0 },
		{ 0,  976,  48, 0 },
		{ 0,  960,  64, 0 },
		{ 0,  944,  80, 0 },
		{ 0,  928,  96, 0 },
		{ 0,  912, 112, 0 },
		{ 0,  896, 128, 0 },
		{ 0,  880, 144, 0 },
		{ 0,  864, 160, 0 },
		{ 0,  848, 176, 0 },
		{ 0,  832, 192, 0 },
		{ 0,  816, 208, 0 },
		{ 0,  800, 224, 0 },
		{ 0,  784, 240, 0 },
		{ 0,  768, 256, 0 },
		{ 0,  752, 272, 0 },
		{ 0,  736, 288, 0 },
		{ 0,  720, 304, 0 },
		{ 0,  704, 320, 0 },
		{ 0,  688, 336, 0 },
		{ 0,  672, 352, 0 },
		{ 0,  656, 368, 0 },
		{ 0,  640, 384, 0 },
		{ 0,  624, 400, 0 },
		{ 0,  608, 416, 0 },
		{ 0,  592, 432, 0 },
		{ 0,  576, 448, 0 },
		{ 0,  560, 464, 0 },
		{ 0,  544, 480, 0 },
		{ 0,  528, 496, 0 },
		{ 0,  512, 512, 0 }
	}
};

const int coef_lut_tap5[SCL_COEF_IDX_MAX][PHASE_NUM][TAP5] = {
	/* YUV_COEF_IDX */
	{
		{ 98, 415, 415,  98, -2 },
		{ 95, 412, 418, 103, -4 },
		{ 91, 408, 422, 107, -4 },
		{ 87, 404, 426, 111, -4 },
		{ 84, 399, 430, 115, -4 },
		{ 80, 395, 434, 119, -4 },
		{ 76, 390, 438, 124, -4 },
		{ 73, 386, 440, 128, -3 },
		{ 70, 381, 444, 132, -3 },
		{ 66, 376, 448, 137, -3 },
		{ 63, 371, 451, 142, -3 },
		{ 60, 366, 455, 146, -3 },
		{ 57, 361, 457, 151, -2 },
		{ 54, 356, 460, 156, -2 },
		{ 51, 351, 463, 161, -2 },
		{ 49, 346, 465, 165, -1 },
		{ 46, 341, 468, 170, -1 },
		{ 43, 336, 470, 175, 0 },
		{ 41, 331, 472, 180, 0 },
		{ 38, 325, 474, 186, 1 },
		{ 36, 320, 476, 191, 1 },
		{ 34, 315, 477, 196, 2 },
		{ 32, 309, 479, 201, 3 },
		{ 29, 304, 481, 206, 4 },
		{ 27, 299, 481, 212, 5 },
		{ 26, 293, 482, 217, 6 },
		{ 24, 288, 484, 222, 6 },
		{ 22, 282, 484, 228, 8 },
		{ 20, 277, 485, 233, 9 },
		{ 19, 271, 485, 238, 11 },
		{ 17, 266, 485, 244, 12 },
		{ 16, 260, 485, 250, 13 },
		{ 14, 255, 486, 255, 14 },
		{ -94, 608, 608, -94, -4 },
		{ -94, 594, 619, -91, -4 },
		{ -96, 579, 635, -89, -5 },
		{ -96, 563, 650, -87, -6 },
		{ -97, 548, 665, -85, -7 },
		{ -97, 532, 678, -82, -7 },
		{ -98, 516, 693, -79, -8 },
		{ -97, 500, 705, -75, -9 },
		{ -97, 484, 720, -72, -11 },
		{ -97, 468, 733, -68, -12 },
		{ -96, 452, 744, -63, -13 },
		{ -95, 436, 755, -58, -14 },
		{ -94, 419, 768, -53, -16 },
		{ -93, 403, 779, -48, -17 },
		{ -92, 387, 789, -42, -18 },
		{ -90, 371, 799, -36, -20 },
		{ -89, 355, 809, -29, -22 },
		{ -87, 339, 817, -22, -23 },
		{ -86, 324, 826, -15, -25 },
		{ -84, 308, 835,  -8, -27 },
		{ -82, 293, 842,   0, -29 },
		{ -80, 277, 849,   9, -31 },
		{ -78, 262, 855,  18, -33 },
		{ -75, 247, 860,  27, -35 },
		{ -73, 233, 865,  36, -37 },
		{ -71, 218, 870,  46, -39 },
		{ -69, 204, 874,  56, -41 },
		{ -66, 190, 876,  67, -43 },
		{ -64, 176, 879,  78, -45 },
		{ -62, 163, 882,  89, -48 },
		{ -59, 150, 883, 100, -50 },
		{ -57, 137, 883, 112, -51 },
		{ -55, 125, 884, 125, -55 }
	},

	/* RGB_COEF_IDX */
	{
		{ 0, 512,  512, 0, 0 },
		{ 0, 496,  528, 0, 0 },
		{ 0, 480,  544, 0, 0 },
		{ 0, 464,  560, 0, 0 },
		{ 0, 448,  576, 0, 0 },
		{ 0, 432,  592, 0, 0 },
		{ 0, 416,  608, 0, 0 },
		{ 0, 400,  624, 0, 0 },
		{ 0, 384,  640, 0, 0 },
		{ 0, 368,  656, 0, 0 },
		{ 0, 352,  672, 0, 0 },
		{ 0, 336,  688, 0, 0 },
		{ 0, 320,  704, 0, 0 },
		{ 0, 304,  720, 0, 0 },
		{ 0, 288,  736, 0, 0 },
		{ 0, 272,  752, 0, 0 },
		{ 0, 256,  768, 0, 0 },
		{ 0, 240,  784, 0, 0 },
		{ 0, 224,  800, 0, 0 },
		{ 0, 208,  816, 0, 0 },
		{ 0, 192,  832, 0, 0 },
		{ 0, 176,  848, 0, 0 },
		{ 0, 160,  864, 0, 0 },
		{ 0, 144,  880, 0, 0 },
		{ 0, 128,  896, 0, 0 },
		{ 0, 112,  912, 0, 0 },
		{ 0,  96,  928, 0, 0 },
		{ 0,  80,  944, 0, 0 },
		{ 0,  64,  960, 0, 0 },
		{ 0,  48,  976, 0, 0 },
		{ 0,  32,  992, 0, 0 },
		{ 0,  16, 1008, 0, 0 },
		{ 0,   0, 1024, 0, 0 },
		{ 0, 512,  512, 0, 0 },
		{ 0, 496,  528, 0, 0 },
		{ 0, 480,  544, 0, 0 },
		{ 0, 464,  560, 0, 0 },
		{ 0, 448,  576, 0, 0 },
		{ 0, 432,  592, 0, 0 },
		{ 0, 416,  608, 0, 0 },
		{ 0, 400,  624, 0, 0 },
		{ 0, 384,  640, 0, 0 },
		{ 0, 368,  656, 0, 0 },
		{ 0, 352,  672, 0, 0 },
		{ 0, 336,  688, 0, 0 },
		{ 0, 320,  704, 0, 0 },
		{ 0, 304,  720, 0, 0 },
		{ 0, 288,  736, 0, 0 },
		{ 0, 272,  752, 0, 0 },
		{ 0, 256,  768, 0, 0 },
		{ 0, 240,  784, 0, 0 },
		{ 0, 224,  800, 0, 0 },
		{ 0, 208,  816, 0, 0 },
		{ 0, 192,  832, 0, 0 },
		{ 0, 176,  848, 0, 0 },
		{ 0, 160,  864, 0, 0 },
		{ 0, 144,  880, 0, 0 },
		{ 0, 128,  896, 0, 0 },
		{ 0, 112,  912, 0, 0 },
		{ 0,  96,  928, 0, 0 },
		{ 0,  80,  944, 0, 0 },
		{ 0,  64,  960, 0, 0 },
		{ 0,  48,  976, 0, 0 },
		{ 0,  32,  992, 0, 0 },
		{ 0,  16, 1008, 0, 0 },
		{ 0,   0, 1024, 0, 0 }
	}
};

const int g_coef_lut_v1_tap5[SCL_COEF_IDX_MAX][PHASE_NUM][TAP5] = {
	/* YUV_COEF_IDX */
	{
		{ -94, 608, 608, -94, -4 },
		{ -94, 594, 619, -91, -4 },
		{ -96, 579, 635, -89, -5 },
		{ -96, 563, 650, -87, -6 },
		{ -97, 548, 665, -85, -7 },
		{ -97, 532, 678, -82, -7 },
		{ -98, 516, 693, -79, -8 },
		{ -97, 500, 705, -75, -9 },
		{ -97, 484, 720, -72, -11 },
		{ -97, 468, 733, -68, -12 },
		{ -96, 452, 744, -63, -13 },
		{ -95, 436, 755, -58, -14 },
		{ -94, 419, 768, -53, -16 },
		{ -93, 403, 779, -48, -17 },
		{ -92, 387, 789, -42, -18 },
		{ -90, 371, 799, -36, -20 },
		{ -89, 355, 809, -29, -22 },
		{ -87, 339, 817, -22, -23 },
		{ -86, 324, 826, -15, -25 },
		{ -84, 308, 835,  -8, -27 },
		{ -82, 293, 842,   0, -29 },
		{ -80, 277, 849,   9, -31 },
		{ -78, 262, 855,  18, -33 },
		{ -75, 247, 860,  27, -35 },
		{ -73, 233, 865,  36, -37 },
		{ -71, 218, 870,  46, -39 },
		{ -69, 204, 874,  56, -41 },
		{ -66, 190, 876,  67, -43 },
		{ -64, 176, 879,  78, -45 },
		{ -62, 163, 882,  89, -48 },
		{ -59, 150, 883, 100, -50 },
		{ -57, 137, 883, 112, -51 },
		{ -55, 125, 884, 125, -55 },
		{ 98, 415, 415,  98, -2 },
		{ 95, 412, 418, 103, -4 },
		{ 91, 408, 422, 107, -4 },
		{ 87, 404, 426, 111, -4 },
		{ 84, 399, 430, 115, -4 },
		{ 80, 395, 434, 119, -4 },
		{ 76, 390, 438, 124, -4 },
		{ 73, 386, 440, 128, -3 },
		{ 70, 381, 444, 132, -3 },
		{ 66, 376, 448, 137, -3 },
		{ 63, 371, 451, 142, -3 },
		{ 60, 366, 455, 146, -3 },
		{ 57, 361, 457, 151, -2 },
		{ 54, 356, 460, 156, -2 },
		{ 51, 351, 463, 161, -2 },
		{ 49, 346, 465, 165, -1 },
		{ 46, 341, 468, 170, -1 },
		{ 43, 336, 470, 175, 0 },
		{ 41, 331, 472, 180, 0 },
		{ 38, 325, 474, 186, 1 },
		{ 36, 320, 476, 191, 1 },
		{ 34, 315, 477, 196, 2 },
		{ 32, 309, 479, 201, 3 },
		{ 29, 304, 481, 206, 4 },
		{ 27, 299, 481, 212, 5 },
		{ 26, 293, 482, 217, 6 },
		{ 24, 288, 484, 222, 6 },
		{ 22, 282, 484, 228, 8 },
		{ 20, 277, 485, 233, 9 },
		{ 19, 271, 485, 238, 11 },
		{ 17, 266, 485, 244, 12 },
		{ 16, 260, 485, 250, 13 },
		{ 14, 255, 486, 255, 14 }
	},

	/* RGB_COEF_IDX */
	{
		{ 0, 512,  512, 0, 0 },
		{ 0, 496,  528, 0, 0 },
		{ 0, 480,  544, 0, 0 },
		{ 0, 464,  560, 0, 0 },
		{ 0, 448,  576, 0, 0 },
		{ 0, 432,  592, 0, 0 },
		{ 0, 416,  608, 0, 0 },
		{ 0, 400,  624, 0, 0 },
		{ 0, 384,  640, 0, 0 },
		{ 0, 368,  656, 0, 0 },
		{ 0, 352,  672, 0, 0 },
		{ 0, 336,  688, 0, 0 },
		{ 0, 320,  704, 0, 0 },
		{ 0, 304,  720, 0, 0 },
		{ 0, 288,  736, 0, 0 },
		{ 0, 272,  752, 0, 0 },
		{ 0, 256,  768, 0, 0 },
		{ 0, 240,  784, 0, 0 },
		{ 0, 224,  800, 0, 0 },
		{ 0, 208,  816, 0, 0 },
		{ 0, 192,  832, 0, 0 },
		{ 0, 176,  848, 0, 0 },
		{ 0, 160,  864, 0, 0 },
		{ 0, 144,  880, 0, 0 },
		{ 0, 128,  896, 0, 0 },
		{ 0, 112,  912, 0, 0 },
		{ 0,  96,  928, 0, 0 },
		{ 0,  80,  944, 0, 0 },
		{ 0,  64,  960, 0, 0 },
		{ 0,  48,  976, 0, 0 },
		{ 0,  32,  992, 0, 0 },
		{ 0,  16, 1008, 0, 0 },
		{ 0,   0, 1024, 0, 0 },
		{ 0, 512,  512, 0, 0 },
		{ 0, 496,  528, 0, 0 },
		{ 0, 480,  544, 0, 0 },
		{ 0, 464,  560, 0, 0 },
		{ 0, 448,  576, 0, 0 },
		{ 0, 432,  592, 0, 0 },
		{ 0, 416,  608, 0, 0 },
		{ 0, 400,  624, 0, 0 },
		{ 0, 384,  640, 0, 0 },
		{ 0, 368,  656, 0, 0 },
		{ 0, 352,  672, 0, 0 },
		{ 0, 336,  688, 0, 0 },
		{ 0, 320,  704, 0, 0 },
		{ 0, 304,  720, 0, 0 },
		{ 0, 288,  736, 0, 0 },
		{ 0, 272,  752, 0, 0 },
		{ 0, 256,  768, 0, 0 },
		{ 0, 240,  784, 0, 0 },
		{ 0, 224,  800, 0, 0 },
		{ 0, 208,  816, 0, 0 },
		{ 0, 192,  832, 0, 0 },
		{ 0, 176,  848, 0, 0 },
		{ 0, 160,  864, 0, 0 },
		{ 0, 144,  880, 0, 0 },
		{ 0, 128,  896, 0, 0 },
		{ 0, 112,  912, 0, 0 },
		{ 0,  96,  928, 0, 0 },
		{ 0,  80,  944, 0, 0 },
		{ 0,  64,  960, 0, 0 },
		{ 0,  48,  976, 0, 0 },
		{ 0,  32,  992, 0, 0 },
		{ 0,  16, 1008, 0, 0 },
		{ 0,   0, 1024, 0, 0 }
	}
};

const int coef_lut_tap6[SCL_COEF_IDX_MAX][PHASE_NUM][TAP6] = {
	/* YUV_COEF_IDX */
	{
		{ 2, 264, 500, 264, 2, -8 },
		{ 2, 257, 499, 268, 6, -8 },
		{ 1, 252, 498, 274, 8, -9 },
		{  -1, 246,  498, 281,   9,  -9 },
		{  -2, 241,  497, 286,  12, -10 },
		{  -3, 235,  497, 292,  13, -10 },
		{  -5, 230,  496, 298,  15, -10 },
		{  -6, 225,  495, 303,  18, -11 },
		{  -7, 219,  494, 309,  20, -11 },
		{  -7, 213,  493, 314,  23, -12 },
		{  -9, 208,  491, 320,  26, -12 },
		{ -10, 203,  490, 325,  28, -12 },
		{ -10, 197,  488, 331,  31, -13 },
		{ -10, 192,  486, 336,  33, -13 },
		{ -12, 186,  485, 342,  36, -13 },
		{ -12, 181,  482, 347,  39, -13 },
		{ -13, 176,  480, 352,  42, -13 },
		{ -14, 171,  478, 358,  45, -14 },
		{ -14, 166,  476, 363,  48, -15 },
		{ -14, 160,  473, 368,  52, -15 },
		{ -14, 155,  470, 373,  55, -15 },
		{ -15, 150,  467, 378,  59, -15 },
		{ -15, 145,  464, 383,  62, -15 },
		{ -16, 141,  461, 388,  65, -15 },
		{ -16, 136,  458, 393,  68, -15 },
		{ -16, 131,  455, 398,  72, -16 },
		{ -16, 126,  451, 402,  77, -16 },
		{ -16, 122,  448, 407,  79, -16 },
		{ -16, 117,  444, 411,  84, -16 },
		{ -17, 113,  441, 416,  87, -16 },
		{ -17, 108,  437, 420,  92, -16 },
		{ -17, 104,  433, 424,  96, -16 },
		{ -17, 100,  429, 429, 100, -17 },
		{-187, 105, 1186, 105, -187, 2 },
		{-182,  86, 1186, 124, -192, 2 },
		{-176,  67, 1185, 143, -197, 2 },
		{-170,  49, 1182, 163, -202, 2 },
		{-166,  32, 1180, 184, -207, 1 },
		{-160,  15, 1176, 204, -212, 1 },
		{-155,  -2, 1171, 225, -216, 1 },
		{-149, -18, 1166, 246, -221, 0 },
		{-145, -34, 1160, 268, -225, 0 },
		{-139, -49, 1153, 290, -230,  -1 },
		{-134, -63, 1145, 312, -234,  -2 },
		{-129, -78, 1137, 334, -238,  -2 },
		{-124, -91, 1128, 357, -241,  -5 },
		{-119, -104, 1118, 379, -245,  -5 },
		{-114, -117, 1107, 402, -248,  -6 },
		{-109, -129, 1096, 425, -251,  -8 },
		{-104, -141, 1083, 448, -254,  -8 },
		{-100, -152, 1071, 471, -257,  -9 },
		{ -95, -162, 1057, 494, -259, -11 },
		{ -90, -172, 1043, 517, -261, -13 },
		{ -86, -181, 1028, 540, -263, -14 },
		{ -82, -190, 1013, 563, -264, -16 },
		{ -77, -199,  997, 586, -265, -18 },
		{ -73, -207,  980, 609, -266, -19 },
		{ -69, -214,  963, 632, -266, -22 },
		{ -65, -221,  945, 655, -266, -24 },
		{ -62, -227,  927, 678, -266, -26 },
		{ -58, -233,  908, 700, -265, -28 },
		{ -54, -238,  889, 722, -264, -31 },
		{ -51, -243,  870, 744, -262, -34 },
		{ -48, -247,  850, 766, -260, -37 },
		{ -45, -251,  829, 787, -257, -39 },
		{ -42, -255,  809, 809, -255, -42 }
	},

	/* RGB_COEF_IDX */
	{
		{ 0, 0, 1024,   0, 0, 0 },
		{ 0, 0, 1008,  16, 0, 0 },
		{ 0, 0,  992,  32, 0, 0 },
		{ 0, 0,  976,  48, 0, 0 },
		{ 0, 0,  960,  64, 0, 0 },
		{ 0, 0,  944,  80, 0, 0 },
		{ 0, 0,  928,  96, 0, 0 },
		{ 0, 0,  912, 112, 0, 0 },
		{ 0, 0,  896, 128, 0, 0 },
		{ 0, 0,  880, 144, 0, 0 },
		{ 0, 0,  864, 160, 0, 0 },
		{ 0, 0,  848, 176, 0, 0 },
		{ 0, 0,  832, 192, 0, 0 },
		{ 0, 0,  816, 208, 0, 0 },
		{ 0, 0,  800, 224, 0, 0 },
		{ 0, 0,  784, 240, 0, 0 },
		{ 0, 0,  768, 256, 0, 0 },
		{ 0, 0,  752, 272, 0, 0 },
		{ 0, 0,  736, 288, 0, 0 },
		{ 0, 0,  720, 304, 0, 0 },
		{ 0, 0,  704, 320, 0, 0 },
		{ 0, 0,  688, 336, 0, 0 },
		{ 0, 0,  672, 352, 0, 0 },
		{ 0, 0,  656, 368, 0, 0 },
		{ 0, 0,  640, 384, 0, 0 },
		{ 0, 0,  624, 400, 0, 0 },
		{ 0, 0,  608, 416, 0, 0 },
		{ 0, 0,  592, 432, 0, 0 },
		{ 0, 0,  576, 448, 0, 0 },
		{ 0, 0,  560, 464, 0, 0 },
		{ 0, 0,  544, 480, 0, 0 },
		{ 0, 0,  528, 496, 0, 0 },
		{ 0, 0,  512, 512, 0, 0 },
		{ 0, 0, 1024,   0, 0, 0 },
		{ 0, 0, 1008,  16, 0, 0 },
		{ 0, 0,  992,  32, 0, 0 },
		{ 0, 0,  976,  48, 0, 0 },
		{ 0, 0,  960,  64, 0, 0 },
		{ 0, 0,  944,  80, 0, 0 },
		{ 0, 0,  928,  96, 0, 0 },
		{ 0, 0,  912, 112, 0, 0 },
		{ 0, 0,  896, 128, 0, 0 },
		{ 0, 0,  880, 144, 0, 0 },
		{ 0, 0,  864, 160, 0, 0 },
		{ 0, 0,  848, 176, 0, 0 },
		{ 0, 0,  832, 192, 0, 0 },
		{ 0, 0,  816, 208, 0, 0 },
		{ 0, 0,  800, 224, 0, 0 },
		{ 0, 0,  784, 240, 0, 0 },
		{ 0, 0,  768, 256, 0, 0 },
		{ 0, 0,  752, 272, 0, 0 },
		{ 0, 0,  736, 288, 0, 0 },
		{ 0, 0,  720, 304, 0, 0 },
		{ 0, 0,  704, 320, 0, 0 },
		{ 0, 0,  688, 336, 0, 0 },
		{ 0, 0,  672, 352, 0, 0 },
		{ 0, 0,  656, 368, 0, 0 },
		{ 0, 0,  640, 384, 0, 0 },
		{ 0, 0,  624, 400, 0, 0 },
		{ 0, 0,  608, 416, 0, 0 },
		{ 0, 0,  592, 432, 0, 0 },
		{ 0, 0,  576, 448, 0, 0 },
		{ 0, 0,  560, 464, 0, 0 },
		{ 0, 0,  544, 480, 0, 0 },
		{ 0, 0,  528, 496, 0, 0 },
		{ 0, 0,  512, 512, 0, 0 }
	}
};

const int g_coef_lut_v1_tap6[SCL_COEF_IDX_MAX][PHASE_NUM][TAP6] = {
	/* YUV_COEF_IDX */
	{
		{-187, 105, 1186, 105, -187, 2 },
		{-182,  86, 1186, 124, -192, 2 },
		{-176,  67, 1185, 143, -197, 2 },
		{-170,  49, 1182, 163, -202, 2 },
		{-166,  32, 1180, 184, -207, 1 },
		{-160,  15, 1176, 204, -212, 1 },
		{-155,  -2, 1171, 225, -216, 1 },
		{-149, -18, 1166, 246, -221, 0 },
		{-145, -34, 1160, 268, -225, 0 },
		{-139, -49, 1153, 290, -230,  -1 },
		{-134, -63, 1145, 312, -234,  -2 },
		{-129, -78, 1137, 334, -238,  -2 },
		{-124, -91, 1128, 357, -241,  -5 },
		{-119, -104, 1118, 379, -245,  -5 },
		{-114, -117, 1107, 402, -248,  -6 },
		{-109, -129, 1096, 425, -251,  -8 },
		{-104, -141, 1083, 448, -254,  -8 },
		{-100, -152, 1071, 471, -257,  -9 },
		{ -95, -162, 1057, 494, -259, -11 },
		{ -90, -172, 1043, 517, -261, -13 },
		{ -86, -181, 1028, 540, -263, -14 },
		{ -82, -190, 1013, 563, -264, -16 },
		{ -77, -199,  997, 586, -265, -18 },
		{ -73, -207,  980, 609, -266, -19 },
		{ -69, -214,  963, 632, -266, -22 },
		{ -65, -221,  945, 655, -266, -24 },
		{ -62, -227,  927, 678, -266, -26 },
		{ -58, -233,  908, 700, -265, -28 },
		{ -54, -238,  889, 722, -264, -31 },
		{ -51, -243,  870, 744, -262, -34 },
		{ -48, -247,  850, 766, -260, -37 },
		{ -45, -251,  829, 787, -257, -39 },
		{ -42, -255,  809, 809, -255, -42 },
		{-187, 105, 1186, 105, -187, 2 },
		{-182,  86, 1186, 124, -192, 2 },
		{-176,  67, 1185, 143, -197, 2 },
		{-170,  49, 1182, 163, -202, 2 },
		{-166,  32, 1180, 184, -207, 1 },
		{-160,  15, 1176, 204, -212, 1 },
		{-155,  -2, 1171, 225, -216, 1 },
		{-149, -18, 1166, 246, -221, 0 },
		{-145, -34, 1160, 268, -225, 0 },
		{-139, -49, 1153, 290, -230,  -1 },
		{-134, -63, 1145, 312, -234,  -2 },
		{-129, -78, 1137, 334, -238,  -2 },
		{-124, -91, 1128, 357, -241,  -5 },
		{-119, -104, 1118, 379, -245,  -5 },
		{-114, -117, 1107, 402, -248,  -6 },
		{-109, -129, 1096, 425, -251,  -8 },
		{-104, -141, 1083, 448, -254,  -8 },
		{-100, -152, 1071, 471, -257,  -9 },
		{ -95, -162, 1057, 494, -259, -11 },
		{ -90, -172, 1043, 517, -261, -13 },
		{ -86, -181, 1028, 540, -263, -14 },
		{ -82, -190, 1013, 563, -264, -16 },
		{ -77, -199,  997, 586, -265, -18 },
		{ -73, -207,  980, 609, -266, -19 },
		{ -69, -214,  963, 632, -266, -22 },
		{ -65, -221,  945, 655, -266, -24 },
		{ -62, -227,  927, 678, -266, -26 },
		{ -58, -233,  908, 700, -265, -28 },
		{ -54, -238,  889, 722, -264, -31 },
		{ -51, -243,  870, 744, -262, -34 },
		{ -48, -247,  850, 766, -260, -37 },
		{ -45, -251,  829, 787, -257, -39 },
		{ -42, -255,  809, 809, -255, -42 }
	},

	/* RGB_COEF_IDX */
	{
		{ 0, 0, 1024,   0, 0, 0 },
		{ 0, 0, 1008,  16, 0, 0 },
		{ 0, 0,  992,  32, 0, 0 },
		{ 0, 0,  976,  48, 0, 0 },
		{ 0, 0,  960,  64, 0, 0 },
		{ 0, 0,  944,  80, 0, 0 },
		{ 0, 0,  928,  96, 0, 0 },
		{ 0, 0,  912, 112, 0, 0 },
		{ 0, 0,  896, 128, 0, 0 },
		{ 0, 0,  880, 144, 0, 0 },
		{ 0, 0,  864, 160, 0, 0 },
		{ 0, 0,  848, 176, 0, 0 },
		{ 0, 0,  832, 192, 0, 0 },
		{ 0, 0,  816, 208, 0, 0 },
		{ 0, 0,  800, 224, 0, 0 },
		{ 0, 0,  784, 240, 0, 0 },
		{ 0, 0,  768, 256, 0, 0 },
		{ 0, 0,  752, 272, 0, 0 },
		{ 0, 0,  736, 288, 0, 0 },
		{ 0, 0,  720, 304, 0, 0 },
		{ 0, 0,  704, 320, 0, 0 },
		{ 0, 0,  688, 336, 0, 0 },
		{ 0, 0,  672, 352, 0, 0 },
		{ 0, 0,  656, 368, 0, 0 },
		{ 0, 0,  640, 384, 0, 0 },
		{ 0, 0,  624, 400, 0, 0 },
		{ 0, 0,  608, 416, 0, 0 },
		{ 0, 0,  592, 432, 0, 0 },
		{ 0, 0,  576, 448, 0, 0 },
		{ 0, 0,  560, 464, 0, 0 },
		{ 0, 0,  544, 480, 0, 0 },
		{ 0, 0,  528, 496, 0, 0 },
		{ 0, 0,  512, 512, 0, 0 },
		{ 0, 0, 1024,   0, 0, 0 },
		{ 0, 0, 1008,  16, 0, 0 },
		{ 0, 0,  992,  32, 0, 0 },
		{ 0, 0,  976,  48, 0, 0 },
		{ 0, 0,  960,  64, 0, 0 },
		{ 0, 0,  944,  80, 0, 0 },
		{ 0, 0,  928,  96, 0, 0 },
		{ 0, 0,  912, 112, 0, 0 },
		{ 0, 0,  896, 128, 0, 0 },
		{ 0, 0,  880, 144, 0, 0 },
		{ 0, 0,  864, 160, 0, 0 },
		{ 0, 0,  848, 176, 0, 0 },
		{ 0, 0,  832, 192, 0, 0 },
		{ 0, 0,  816, 208, 0, 0 },
		{ 0, 0,  800, 224, 0, 0 },
		{ 0, 0,  784, 240, 0, 0 },
		{ 0, 0,  768, 256, 0, 0 },
		{ 0, 0,  752, 272, 0, 0 },
		{ 0, 0,  736, 288, 0, 0 },
		{ 0, 0,  720, 304, 0, 0 },
		{ 0, 0,  704, 320, 0, 0 },
		{ 0, 0,  688, 336, 0, 0 },
		{ 0, 0,  672, 352, 0, 0 },
		{ 0, 0,  656, 368, 0, 0 },
		{ 0, 0,  640, 384, 0, 0 },
		{ 0, 0,  624, 400, 0, 0 },
		{ 0, 0,  608, 416, 0, 0 },
		{ 0, 0,  592, 432, 0, 0 },
		{ 0, 0,  576, 448, 0, 0 },
		{ 0, 0,  560, 464, 0, 0 },
		{ 0, 0,  544, 480, 0, 0 },
		{ 0, 0,  528, 496, 0, 0 },
		{ 0, 0,  512, 512, 0, 0 }
	}
};

void hisi_dss_scl_init(const char __iomem *scl_base, dss_scl_t *s_scl)
{
	dpu_check_and_no_retval(!scl_base, ERR, "scl_base is NULL\n");
	dpu_check_and_no_retval(!s_scl, ERR, "s_scl is NULL\n");

	memset(s_scl, 0, sizeof(dss_scl_t));

	s_scl->en_hscl_str = inp32(scl_base + SCF_EN_HSCL_STR);
	s_scl->en_vscl_str = inp32(scl_base + SCF_EN_VSCL_STR);
	s_scl->h_v_order = inp32(scl_base + SCF_H_V_ORDER);
	s_scl->input_width_height = inp32(scl_base + SCF_INPUT_WIDTH_HEIGHT);
	s_scl->output_width_height = inp32(scl_base + SCF_OUTPUT_WIDTH_HEIGHT);
	s_scl->en_hscl = inp32(scl_base + SCF_EN_HSCL);
	s_scl->en_vscl = inp32(scl_base + SCF_EN_VSCL);
	s_scl->acc_hscl = inp32(scl_base + SCF_ACC_HSCL);
	s_scl->inc_hscl = inp32(scl_base + SCF_INC_HSCL);
	s_scl->inc_vscl = inp32(scl_base + SCF_INC_VSCL);
	s_scl->en_mmp = inp32(scl_base + SCF_EN_MMP);
}

void hisi_dss_scl_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *scl_base, dss_scl_t *s_scl)
{
	dpu_check_and_no_retval(!scl_base, DEBUG, "scl_base is NULL\n");
	dpu_check_and_no_retval(!s_scl, DEBUG, "s_scl is NULL\n");

	if (dpufd) {
		dpufd->set_reg(dpufd, scl_base + SCF_EN_HSCL_STR, s_scl->en_hscl_str, 32, 0);
		dpufd->set_reg(dpufd, scl_base + SCF_EN_VSCL_STR, s_scl->en_vscl_str, 32, 0);
		dpufd->set_reg(dpufd, scl_base + SCF_H_V_ORDER, s_scl->h_v_order, 32, 0);
		dpufd->set_reg(dpufd, scl_base + SCF_INPUT_WIDTH_HEIGHT, s_scl->input_width_height, 32, 0);
		dpufd->set_reg(dpufd, scl_base + SCF_OUTPUT_WIDTH_HEIGHT, s_scl->output_width_height, 32, 0);
		dpufd->set_reg(dpufd, scl_base + SCF_EN_HSCL, s_scl->en_hscl, 32, 0);
		dpufd->set_reg(dpufd, scl_base + SCF_EN_VSCL, s_scl->en_vscl, 32, 0);
		dpufd->set_reg(dpufd, scl_base + SCF_ACC_HSCL, s_scl->acc_hscl, 32, 0);
		dpufd->set_reg(dpufd, scl_base + SCF_INC_HSCL, s_scl->inc_hscl, 32, 0);
		dpufd->set_reg(dpufd, scl_base + SCF_INC_VSCL, s_scl->inc_vscl, 32, 0);
		dpufd->set_reg(dpufd, scl_base + SCF_EN_MMP, s_scl->en_mmp, 32, 0);
	} else {
		set_reg(scl_base + SCF_EN_HSCL_STR, s_scl->en_hscl_str, 32, 0);
		set_reg(scl_base + SCF_EN_VSCL_STR, s_scl->en_vscl_str, 32, 0);
		set_reg(scl_base + SCF_H_V_ORDER, s_scl->h_v_order, 32, 0);
		set_reg(scl_base + SCF_INPUT_WIDTH_HEIGHT, s_scl->input_width_height, 32, 0);
		set_reg(scl_base + SCF_OUTPUT_WIDTH_HEIGHT, s_scl->output_width_height, 32, 0);
		set_reg(scl_base + SCF_EN_HSCL, s_scl->en_hscl, 32, 0);
		set_reg(scl_base + SCF_EN_VSCL, s_scl->en_vscl, 32, 0);
		set_reg(scl_base + SCF_ACC_HSCL, s_scl->acc_hscl, 32, 0);
		set_reg(scl_base + SCF_INC_HSCL, s_scl->inc_hscl, 32, 0);
		set_reg(scl_base + SCF_INC_VSCL, s_scl->inc_vscl, 32, 0);
		set_reg(scl_base + SCF_EN_MMP, s_scl->en_mmp, 32, 0);
	}
}

static int hisi_dss_scl_coef_set(struct dpu_fb_data_type *dpufd,
	int col, int module_base)
{
	int ret;
	char __iomem *addr = NULL;
	const int **coef_lut = NULL;
	struct coef_lut_tap lut_tap_addr = { PHASE_NUM, col};

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}

	switch (col) {
	case TAP6:
		coef_lut = (const int **)coef_lut_tap6[SCL_COEF_YUV_IDX];
		addr = dpufd->dss_base + module_base + DSS_SCF_H0_Y_COEF_OFFSET;
		break;
	case TAP5:
		coef_lut = (const int **)coef_lut_tap5[SCL_COEF_YUV_IDX];
		addr = dpufd->dss_base + module_base + DSS_SCF_Y_COEF_OFFSET;
		break;
	case TAP4:
		coef_lut = (const int **)coef_lut_tap4[SCL_COEF_YUV_IDX];
		addr = dpufd->dss_base + module_base + DSS_SCF_UV_COEF_OFFSET;
		break;
	default:
		return -EINVAL;
	}

	ret = hisi_dss_scl_write_coefs(dpufd, false, addr, coef_lut, lut_tap_addr);

	return ret;
}

static int hisi_dss_v1_scl_coef_set(struct dpu_fb_data_type *dpufd,
	int col, int module_base)
{
	int ret;
	char __iomem *addr = NULL;
	const int **coef_lut = NULL;
	struct coef_lut_tap lut_tap_addr = { PHASE_NUM, col};

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}

	switch (col) {
	case TAP6:
		coef_lut = (const int **)g_coef_lut_v1_tap6[SCL_COEF_YUV_IDX];
		addr = dpufd->dss_base + module_base + DSS_SCF_H0_Y_COEF_OFFSET;
		break;
	case TAP5:
		coef_lut = (const int **)g_coef_lut_v1_tap5[SCL_COEF_YUV_IDX];
		addr = dpufd->dss_base + module_base + DSS_SCF_Y_COEF_OFFSET;
		break;
	case TAP4:
		coef_lut = (const int **)g_coef_lut_v1_tap4[SCL_COEF_YUV_IDX];
		addr = dpufd->dss_base + module_base + DSS_SCF_UV_COEF_OFFSET;
		break;
	default:
		return -EINVAL;
	}

	ret = hisi_dss_scl_write_coefs(dpufd, false, addr, coef_lut, lut_tap_addr);

	return ret;
}

static int hisi_dss_scl_write_coefs_sub(struct dpu_fb_data_type *dpufd,
	int col, int chn_idx, int module_base)
{
	int ret;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}

	if (chn_idx == DSS_RCHN_V1) {
		ret = hisi_dss_v1_scl_coef_set(dpufd, col, module_base);
		if (ret < 0) {
			DPU_FB_ERR("fail to set channel v1 scl coefficients\n");
			return -EINVAL;
		}
	} else {
		ret = hisi_dss_scl_coef_set(dpufd, col, module_base);
		if (ret < 0) {
			DPU_FB_ERR("fail to set scl coefficients\n");
			return -EINVAL;
		}
	}

	return ret;
}

int hisi_dss_scl_write_coefs(struct dpu_fb_data_type *dpufd, bool enable_cmdlist,
	char __iomem *addr, const int **p, struct coef_lut_tap lut_tap_addr)
{
	int groups[3] = {0};  /* set scaling factor */
	int offset;
	int valid_num;
	int i;
	int j;
	int k;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL Point!\n");
	dpu_check_and_return(!addr, -EINVAL, ERR, "addr is NULL Point!\n");
	dpu_check_and_return(!p, -EINVAL, ERR, "p is NULL Point!\n");
	dpu_check_and_return(((lut_tap_addr.row != PHASE_NUM) ||
		(lut_tap_addr.col < TAP4 || lut_tap_addr.col > TAP6)), -EINVAL, ERR,
		"SCF filter coefficients is err, phase_num = %d, tap_num = %d\n", lut_tap_addr.row, lut_tap_addr.col);

	/* byte */
	offset = (lut_tap_addr.col == TAP4) ? 8 : 16;
	valid_num = (offset == 16) ? 3 : 2;

	for (i = 0; i < lut_tap_addr.row; i++) {
		for (j = 0; j < lut_tap_addr.col; j += 2) {
			if ((lut_tap_addr.col % 2) && (j == lut_tap_addr.col - 1))
				groups[j / 2] = (*((int *)p + i * lut_tap_addr.col + j) & 0xFFF) | (0 << 16);
			else
				groups[j / 2] = (*((int *)p + i * lut_tap_addr.col + j) & 0xFFF) |
					(*((int *)p + i * lut_tap_addr.col + j + 1) << 16);
		}

		for (k = 0; k < valid_num; k++) {
			if (enable_cmdlist)
				dpufd->set_reg(dpufd, addr + offset * i + k * sizeof(int), groups[k], 32, 0);
			else
				set_reg(addr + offset * i + k * sizeof(int), groups[k], 32, 0);

			groups[k] = 0;
		}
	}

	return 0;
}

int hisi_dss_scl_coef_on(struct dpu_fb_data_type *dpufd, bool enable_cmdlist, int coef_lut_idx)
{
	int i;
	uint32_t module_base;
	int ret;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}

	for (i = 0; i < DSS_CHN_MAX_DEFINE; i++) {
		module_base = g_dss_module_base[i][MODULE_SCL_LUT];
		if (module_base != 0) {
			g_scf_lut_chn_coef_idx[i] = coef_lut_idx;
			ret = hisi_dss_scl_write_coefs_sub(dpufd, TAP6, i, module_base);
			if (ret < 0)
				DPU_FB_ERR("Error to write H0_Y_COEF coefficients\n");

			if (i == DSS_RCHN_V0) {
				hisi_dss_arsr2p_coef_on(dpufd, enable_cmdlist);
				continue;
			}

			ret = hisi_dss_scl_write_coefs_sub(dpufd, TAP5, i, module_base);
			if (ret < 0)
				DPU_FB_ERR("Error to write Y_COEF coefficients\n");

			ret = hisi_dss_scl_write_coefs_sub(dpufd, TAP4, i, module_base);
			if (ret < 0)
				DPU_FB_ERR("Error to write UV_COEF coefficients\n");
		}
	}

	return 0;
}

static int calculate_h_ratio(dss_layer_t *layer, dss_rect_t *src_rect,
	dss_rect_t *dst_rect, struct zoom_ratio *img_zoom_ratio)
{
	int chn_idx;
	dss_block_info_t *pblock_info = &(layer->block_info);
	uint32_t chn_idx_temp;

	chn_idx = layer->chn_idx;

	chn_idx_temp = DSS_RCHN_V0;

	if (chn_idx == chn_idx_temp)
		dst_rect->h = src_rect->h;  /* set dst height to src height, disable vertical scaling in v0scf */

	if (chn_idx == chn_idx_temp && pblock_info->h_ratio_arsr2p && pblock_info->h_ratio) {
		img_zoom_ratio->h_ratio = pblock_info->h_ratio;
		img_zoom_ratio->en_hscl = true;  /* v0 channel, both scf and arsr2p are enabled */
		return 0;
	} else if (chn_idx == chn_idx_temp && !pblock_info->h_ratio && pblock_info->h_ratio_arsr2p) {
		return 0;   /* v0 channel , vscf is not supported, just break; */
	}

	if (pblock_info && (pblock_info->h_ratio != 0) && (pblock_info->h_ratio != SCF_INC_FACTOR)) {
		img_zoom_ratio->h_ratio = pblock_info->h_ratio;
		img_zoom_ratio->en_hscl = true;
		return 0;
	}

	if (chn_idx == chn_idx_temp)
		/* disable horizental scaling up */
		dst_rect->w = (src_rect->w > dst_rect->w ? dst_rect->w : src_rect->w);

	if ((src_rect->w == dst_rect->w) || (DSS_HEIGHT(dst_rect->w) == 0))
		return 0;

	img_zoom_ratio->en_hscl = true;

	if ((src_rect->w < SCF_MIN_INPUT) || (dst_rect->w < SCF_MIN_OUTPUT)) {
		DPU_FB_ERR("src_rect->w[%d] small than 16, or dst_rect->w[%d] small than 16\n",
			src_rect->w, dst_rect->w);
		return -EINVAL;
	}

	img_zoom_ratio->h_ratio = (DSS_HEIGHT(src_rect->w) * SCF_INC_FACTOR + SCF_INC_FACTOR / 2 -
		img_zoom_ratio->acc_hscl) / DSS_HEIGHT(dst_rect->w);

	if ((dst_rect->w > (src_rect->w * SCF_UPSCALE_MAX)) || (src_rect->w > (dst_rect->w * SCF_DOWNSCALE_MAX))) {
		DPU_FB_ERR("%s[%d, %d, %d, %d] src_rect[%d, %d, %d, %d], dst_rect[%d, %d, %d, %d]\n",
		"width out of range, original_src_rec", layer->src_rect.x, layer->src_rect.y,
		layer->src_rect.w, layer->src_rect.h, src_rect->x, src_rect->y,
		src_rect->w, src_rect->h, dst_rect->x, dst_rect->y, dst_rect->w, dst_rect->h);
		return -EINVAL;
	}

	return 0;
}

static int calculate_v_ratio(dss_layer_t *layer, dss_rect_t src_rect,
	dss_rect_t dst_rect, struct zoom_ratio *img_zoom_ratio)
{
	if ((src_rect.h == dst_rect.h) || (DSS_HEIGHT(dst_rect.h) == 0))
		return 0;

	img_zoom_ratio->scf_en_vscl = 1;
	img_zoom_ratio->en_vscl = true;

	img_zoom_ratio->v_ratio = (DSS_HEIGHT(src_rect.h) * SCF_INC_FACTOR + SCF_INC_FACTOR / 2 -
		img_zoom_ratio->acc_vscl) / DSS_HEIGHT(dst_rect.h);

	if ((dst_rect.h > (src_rect.h * SCF_UPSCALE_MAX)) || (src_rect.h > (dst_rect.h * SCF_DOWNSCALE_MAX))) {
		DPU_FB_ERR("height out of range, original_src_rec[%d, %d, %d, %d] "
			"src_rect[%d, %d, %d, %d], dst_rect[%d, %d, %d, %d]\n",
			layer->src_rect.x, layer->src_rect.y,
			layer->src_rect.w, layer->src_rect.h,
			src_rect.x, src_rect.y, src_rect.w, src_rect.h,
			dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h);
		return -EINVAL;
	}

	return 0;
}

static void set_scl_reg(struct dpu_fb_data_type *dpufd, dss_layer_t *layer,
	dss_rect_t src_rect, dss_rect_t dst_rect, struct zoom_ratio *img_zoom_ratio)
{
	int chn_idx;
	dss_scl_t *scl = NULL;
	bool has_pixel_alpha = false;

	chn_idx = layer->chn_idx;
	scl = &(dpufd->dss_module.scl[chn_idx]);
	dpufd->dss_module.scl_used[chn_idx] = 1;

	has_pixel_alpha = hal_format_has_alpha(layer->img.format);

	scl->en_hscl_str = set_bits32(scl->en_hscl_str, 0x0, 1, 0);

	if (img_zoom_ratio->v_ratio >= 2 * SCF_INC_FACTOR) {
		if (has_pixel_alpha)
			scl->en_vscl_str = set_bits32(scl->en_vscl_str, 0x3, 2, 0);
		else
			scl->en_vscl_str = set_bits32(scl->en_vscl_str, 0x1, 2, 0);
	} else {
		scl->en_vscl_str = set_bits32(scl->en_vscl_str, 0x0, 1, 0);
	}

	if (src_rect.h > dst_rect.h)
		img_zoom_ratio->scf_en_vscl = 0x3;  /* zoom ratio */

	scl->h_v_order = set_bits32(scl->h_v_order, img_zoom_ratio->h_v_order, 1, 0);
	scl->input_width_height = set_bits32(scl->input_width_height,
		DSS_HEIGHT(src_rect.h), 13, 0);
	scl->input_width_height = set_bits32(scl->input_width_height,
		DSS_WIDTH(src_rect.w), 13, 16);
	scl->output_width_height = set_bits32(scl->output_width_height,
		DSS_HEIGHT(dst_rect.h), 13, 0);
	scl->output_width_height = set_bits32(scl->output_width_height,
		DSS_WIDTH(dst_rect.w), 13, 16);
	scl->en_hscl = set_bits32(scl->en_hscl, (img_zoom_ratio->en_hscl ? 0x1 : 0x0), 1, 0);
	scl->en_vscl = set_bits32(scl->en_vscl, img_zoom_ratio->scf_en_vscl, 2, 0);
	scl->acc_hscl = set_bits32(scl->acc_hscl, img_zoom_ratio->acc_hscl, 31, 0);
	scl->inc_hscl = set_bits32(scl->inc_hscl, img_zoom_ratio->h_ratio, 24, 0);
	scl->inc_vscl = set_bits32(scl->inc_vscl, img_zoom_ratio->v_ratio, 24, 0);
	scl->en_mmp = set_bits32(scl->en_mmp, 0x1, 1, 0);
	scl->fmt = layer->img.format;
}

int hisi_dss_scl_config(struct dpu_fb_data_type *dpufd,
	dss_layer_t *layer, dss_rect_t *aligned_rect, bool rdma_stretch_enable)
{
	int ret;
	dss_rect_t src_rect;
	dss_rect_t dst_rect;
	dss_block_info_t *pblock_info = NULL;
	struct zoom_ratio img_zoom_ratio = {0};

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is nullptr!\n");
	dpu_check_and_return(!layer, -EINVAL, ERR, "layer is nullptr!\n");
	if (dpufd->index == MEDIACOMMON_PANEL_IDX)
		return 0;

	src_rect = (aligned_rect != NULL) ? (*aligned_rect) : (layer->src_rect);
	dst_rect = layer->dst_rect;
	pblock_info = &(layer->block_info);

	if (pblock_info && pblock_info->both_vscfh_arsr2p_used)
		dst_rect = pblock_info->arsr2p_in_rect;

	ret = calculate_h_ratio(layer, &src_rect, &dst_rect, &img_zoom_ratio);
	if (ret < 0) {
		DPU_FB_ERR("calculate_h_ratio failed\n");
		return -EINVAL;
	}

	ret = calculate_v_ratio(layer, src_rect, dst_rect, &img_zoom_ratio);
	if (ret < 0) {
		DPU_FB_ERR("calculate_v_ratio failed\n");
		return -EINVAL;
	}

	if (!(img_zoom_ratio.en_hscl) && !(img_zoom_ratio.en_vscl))
		return 0;

	/* scale down, do hscl first; scale up, do vscl first */
	img_zoom_ratio.h_v_order = (src_rect.w > dst_rect.w) ? 0 : 1;

	if (pblock_info && (pblock_info->acc_hscl != 0))
		img_zoom_ratio.acc_hscl = pblock_info->acc_hscl;

	set_scl_reg(dpufd, layer, src_rect, dst_rect, &img_zoom_ratio);

	return 0;
}
