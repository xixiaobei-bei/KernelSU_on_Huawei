/*
 * Copyright (c) 2018-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
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
/*lint -e548 -e574 -e676 -e679 -e732 -e838*/

#include "hisi_dss_iommu.h"
#include "hisi_drm_dpe_utils.h"
#include "hisi_drm_drv.h"
#include "hisi_overlay_utils.h"
#include "hisi_drm_dss_set_regs.h"
#include "hisi_drm_dss_global.h"
#include "hisi_defs.h"
#ifdef CONFIG_HISI_DSS_V600
#include "hisi_drm_smmuv3.h"
#endif

#include <securec.h>

#define ARSR2P_PHASE_NUM	(9)
#define ARSR2P_TAP4	(4)
#define ARSR2P_TAP6	(6)

#define ARSR2P_SCL_UP_OFFSET (0x48)
#define ARSR2P_COEF_H0_OFFSET (0x100)
#define ARSR2P_COEF_H1_OFFSET (0x200)

#define PHASE_NUM	(66)
#define TAP4	(4)
#define TAP5	(5)
#define TAP6	(6)
#define COEF_LUT_NUM	(2)

static int mid_array[DSS_CHN_MAX_DEFINE] = { 0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x2, 0x1, 0x3, 0x0};

/* SCF_LUT_CHN coef_idx */
int g_scf_lut_chn_coef_idx[DSS_CHN_MAX_DEFINE] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};


static const int coef_lut_tap4[SCL_COEF_IDX_MAX][PHASE_NUM][TAP4] = {
	/* YUV_COEF_IDX */
	{
		{ 214, 599, 214, -3},
		{ 207, 597, 223, -3},
		{ 200, 596, 231, -3},
		{ 193, 596, 238, -3},
		{ 186, 595, 246, -3},
		{ 178, 594, 255, -3},
		{ 171, 593, 263, -3},
		{ 165, 591, 271, -3},
		{ 158, 589, 279, -2},
		{ 151, 587, 288, -2},
		{ 145, 584, 296, -1},
		{ 139, 582, 304, -1},
		{ 133, 578, 312, 1},
		{ 127, 575, 321, 1},
		{ 121, 572, 329, 2},
		{ 115, 568, 337, 4},
		{ 109, 564, 346, 5},
		{ 104, 560, 354, 6},
		{ 98, 555, 362, 9},
		{ 94, 550, 370, 10},
		{ 88, 546, 379, 11},
		{ 84, 540, 387, 13},
		{ 79, 535, 395, 15},
		{ 74, 530, 403, 17},
		{ 70, 524, 411, 19},
		{ 66, 518, 419, 21},
		{ 62, 512, 427, 23},
		{ 57, 506, 435, 26},
		{ 54, 499, 443, 28},
		{ 50, 492, 451, 31},
		{ 47, 486, 457, 34},
		{ 43, 479, 465, 37},
		{ 40, 472, 472, 40},
		{ 214, 599, 214, -3},
		{ 207, 597, 223, -3},
		{ 200, 596, 231, -3},
		{ 193, 596, 238, -3},
		{ 186, 595, 246, -3},
		{ 178, 594, 255, -3},
		{ 171, 593, 263, -3},
		{ 165, 591, 271, -3},
		{ 158, 589, 279, -2},
		{ 151, 587, 288, -2},
		{ 145, 584, 296, -1},
		{ 139, 582, 304, -1},
		{ 133, 578, 312, 1},
		{ 127, 575, 321, 1},
		{ 121, 572, 329, 2},
		{ 115, 568, 337, 4},
		{ 109, 564, 346, 5},
		{ 104, 560, 354, 6},
		{ 98, 555, 362, 9},
		{ 94, 550, 370, 10},
		{ 88, 546, 379, 11},
		{ 84, 540, 387, 13},
		{ 79, 535, 395, 15},
		{ 74, 530, 403, 17},
		{ 70, 524, 411, 19},
		{ 66, 518, 419, 21},
		{ 62, 512, 427, 23},
		{ 57, 506, 435, 26},
		{ 54, 499, 443, 28},
		{ 50, 492, 451, 31},
		{ 47, 486, 457, 34},
		{ 43, 479, 465, 37},
		{ 40, 472, 472, 40}
	},

	/* RGB_COEF_IDX */
	{
		{ 0, 1024,   0, 0},
		{ 0, 1008,  16, 0},
		{ 0,  992,  32, 0},
		{ 0,  976,  48, 0},
		{ 0,  960,  64, 0},
		{ 0,  944,  80, 0},
		{ 0,  928,  96, 0},
		{ 0,  912, 112, 0},
		{ 0,  896, 128, 0},
		{ 0,  880, 144, 0},
		{ 0,  864, 160, 0},
		{ 0,  848, 176, 0},
		{ 0,  832, 192, 0},
		{ 0,  816, 208, 0},
		{ 0,  800, 224, 0},
		{ 0,  784, 240, 0},
		{ 0,  768, 256, 0},
		{ 0,  752, 272, 0},
		{ 0,  736, 288, 0},
		{ 0,  720, 304, 0},
		{ 0,  704, 320, 0},
		{ 0,  688, 336, 0},
		{ 0,  672, 352, 0},
		{ 0,  656, 368, 0},
		{ 0,  640, 384, 0},
		{ 0,  624, 400, 0},
		{ 0,  608, 416, 0},
		{ 0,  592, 432, 0},
		{ 0,  576, 448, 0},
		{ 0,  560, 464, 0},
		{ 0,  544, 480, 0},
		{ 0,  528, 496, 0},
		{ 0,  512, 512, 0},
		{ 0, 1024,   0, 0},
		{ 0, 1008,  16, 0},
		{ 0,  992,  32, 0},
		{ 0,  976,  48, 0},
		{ 0,  960,  64, 0},
		{ 0,  944,  80, 0},
		{ 0,  928,  96, 0},
		{ 0,  912, 112, 0},
		{ 0,  896, 128, 0},
		{ 0,  880, 144, 0},
		{ 0,  864, 160, 0},
		{ 0,  848, 176, 0},
		{ 0,  832, 192, 0},
		{ 0,  816, 208, 0},
		{ 0,  800, 224, 0},
		{ 0,  784, 240, 0},
		{ 0,  768, 256, 0},
		{ 0,  752, 272, 0},
		{ 0,  736, 288, 0},
		{ 0,  720, 304, 0},
		{ 0,  704, 320, 0},
		{ 0,  688, 336, 0},
		{ 0,  672, 352, 0},
		{ 0,  656, 368, 0},
		{ 0,  640, 384, 0},
		{ 0,  624, 400, 0},
		{ 0,  608, 416, 0},
		{ 0,  592, 432, 0},
		{ 0,  576, 448, 0},
		{ 0,  560, 464, 0},
		{ 0,  544, 480, 0},
		{ 0,  528, 496, 0},
		{ 0,  512, 512, 0}
	}
};

static const int coef_luv_tap5[SCL_COEF_IDX_MAX][PHASE_NUM][TAP5] = {
	/* YUV_COEF_IDX */
	{
		{  98, 415, 415,  98, -2},
		{  95, 412, 418, 103, -4},
		{  91, 408, 422, 107, -4},
		{  87, 404, 426, 111, -4},
		{  84, 399, 430, 115, -4},
		{  80, 395, 434, 119, -4},
		{  76, 390, 438, 124, -4},
		{  73, 386, 440, 128, -3},
		{  70, 381, 444, 132, -3},
		{  66, 376, 448, 137, -3},
		{  63, 371, 451, 142, -3},
		{  60, 366, 455, 146, -3},
		{  57, 361, 457, 151, -2},
		{  54, 356, 460, 156, -2},
		{  51, 351, 463, 161, -2},
		{  49, 346, 465, 165, -1},
		{  46, 341, 468, 170, -1},
		{  43, 336, 470, 175, 0},
		{  41, 331, 472, 180, 0},
		{  38, 325, 474, 186, 1},
		{  36, 320, 476, 191, 1},
		{  34, 315, 477, 196, 2},
		{  32, 309, 479, 201, 3},
		{  29, 304, 481, 206, 4},
		{  27, 299, 481, 212, 5},
		{  26, 293, 482, 217, 6},
		{  24, 288, 484, 222, 6},
		{  22, 282, 484, 228, 8},
		{  20, 277, 485, 233, 9},
		{  19, 271, 485, 238, 11},
		{  17, 266, 485, 244, 12},
		{  16, 260, 485, 250, 13},
		{  14, 255, 486, 255, 14},
		{ -94, 608, 608, -94, -4},
		{ -94, 594, 619, -91, -4},
		{ -96, 579, 635, -89, -5},
		{ -96, 563, 650, -87, -6},
		{ -97, 548, 665, -85, -7},
		{ -97, 532, 678, -82, -7},
		{ -98, 516, 693, -79, -8},
		{ -97, 500, 705, -75, -9},
		{ -97, 484, 720, -72, -11},
		{ -97, 468, 733, -68, -12},
		{ -96, 452, 744, -63, -13},
		{ -95, 436, 755, -58, -14},
		{ -94, 419, 768, -53, -16},
		{ -93, 403, 779, -48, -17},
		{ -92, 387, 789, -42, -18},
		{ -90, 371, 799, -36, -20},
		{ -89, 355, 809, -29, -22},
		{ -87, 339, 817, -22, -23},
		{ -86, 324, 826, -15, -25},
		{ -84, 308, 835,  -8, -27},
		{ -82, 293, 842,   0, -29},
		{ -80, 277, 849,   9, -31},
		{ -78, 262, 855,  18, -33},
		{ -75, 247, 860,  27, -35},
		{ -73, 233, 865,  36, -37},
		{ -71, 218, 870,  46, -39},
		{ -69, 204, 874,  56, -41},
		{ -66, 190, 876,  67, -43},
		{ -64, 176, 879,  78, -45},
		{ -62, 163, 882,  89, -48},
		{ -59, 150, 883, 100, -50},
		{ -57, 137, 883, 112, -51},
		{ -55, 125, 884, 125, -55}
	 },

	/* RGB_COEF_IDX */
	{
		{0, 512,  512, 0, 0},
		{0, 496,  528, 0, 0},
		{0, 480,  544, 0, 0},
		{0, 464,  560, 0, 0},
		{0, 448,  576, 0, 0},
		{0, 432,  592, 0, 0},
		{0, 416,  608, 0, 0},
		{0, 400,  624, 0, 0},
		{0, 384,  640, 0, 0},
		{ 0, 368,  656, 0, 0},
		{ 0, 352,  672, 0, 0},
		{ 0, 336,  688, 0, 0},
		{ 0, 320,  704, 0, 0},
		{ 0, 304,  720, 0, 0},
		{ 0, 288,  736, 0, 0},
		{ 0, 272,  752, 0, 0},
		{ 0, 256,  768, 0, 0},
		{ 0, 240,  784, 0, 0},
		{ 0, 224,  800, 0, 0},
		{ 0, 208,  816, 0, 0},
		{ 0, 192,  832, 0, 0},
		{ 0, 176,  848, 0, 0},
		{ 0, 160,  864, 0, 0},
		{ 0, 144,  880, 0, 0},
		{ 0, 128,  896, 0, 0},
		{ 0, 112,  912, 0, 0},
		{ 0,  96,  928, 0, 0},
		{ 0,  80,  944, 0, 0},
		{ 0,  64,  960, 0, 0},
		{ 0,  48,  976, 0, 0},
		{ 0,  32,  992, 0, 0},
		{ 0,  16, 1008, 0, 0},
		{ 0,   0, 1024, 0, 0},
		{ 0, 512,  512, 0, 0},
		{ 0, 496,  528, 0, 0},
		{ 0, 480,  544, 0, 0},
		{ 0, 464,  560, 0, 0},
		{ 0, 448,  576, 0, 0},
		{ 0, 432,  592, 0, 0},
		{ 0, 416,  608, 0, 0},
		{ 0, 400,  624, 0, 0},
		{ 0, 384,  640, 0, 0},
		{ 0, 368,  656, 0, 0},
		{ 0, 352,  672, 0, 0},
		{ 0, 336,  688, 0, 0},
		{ 0, 320,  704, 0, 0},
		{ 0, 304,  720, 0, 0},
		{ 0, 288,  736, 0, 0},
		{ 0, 272,  752, 0, 0},
		{ 0, 256,  768, 0, 0},
		{ 0, 240,  784, 0, 0},
		{ 0, 224,  800, 0, 0},
		{ 0, 208,  816, 0, 0},
		{ 0, 192,  832, 0, 0},
		{ 0, 176,  848, 0, 0},
		{ 0, 160,  864, 0, 0},
		{ 0, 144,  880, 0, 0},
		{ 0, 128,  896, 0, 0},
		{ 0, 112,  912, 0, 0},
		{ 0,  96,  928, 0, 0},
		{ 0,  80,  944, 0, 0},
		{ 0,  64,  960, 0, 0},
		{ 0,  48,  976, 0, 0},
		{ 0,  32,  992, 0, 0},
		{ 0,  16, 1008, 0, 0},
		{ 0,   0, 1024, 0, 0}
	}
};

static const int coef_lut_tap6[SCL_COEF_IDX_MAX][PHASE_NUM][TAP6] = {
	/* YUV_COEF_IDX */
	{
		{ 2, 264, 500, 264, 2, -8},
		{ 2, 257, 499, 268, 6, -8},
		{ 1, 252, 498, 274, 8, -9},
		{ -1, 246,  498, 281,   9,  -9},
		{ -2, 241,  497, 286,  12, -10},
		{ -3, 235,  497, 292,  13, -10},
		{ -5, 230,  496, 298,  15, -10},
		{ -6, 225,  495, 303,  18, -11},
		{ -7, 219,  494, 309,  20, -11},
		{ -7, 213,  493, 314,  23, -12},
		{ -9, 208,  491, 320,  26, -12},
		{ -10, 203,  490, 325,  28, -12},
		{ -10, 197,  488, 331,  31, -13},
		{ -10, 192,  486, 336,  33, -13},
		{ -12, 186,  485, 342,  36, -13},
		{ -12, 181,  482, 347,  39, -13},
		{ -13, 176,  480, 352,  42, -13},
		{ -14, 171,  478, 358,  45, -14},
		{ -14, 166,  476, 363,  48, -15},
		{ -14, 160,  473, 368,  52, -15},
		{ -14, 155,  470, 373,  55, -15},
		{ -15, 150,  467, 378,  59, -15},
		{ -15, 145,  464, 383,  62, -15},
		{ -16, 141,  461, 388,  65, -15},
		{ -16, 136,  458, 393,  68, -15},
		{ -16, 131,  455, 398,  72, -16},
		{ -16, 126,  451, 402,  77, -16},
		{ -16, 122,  448, 407,  79, -16},
		{ -16, 117,  444, 411,  84, -16},
		{ -17, 113,  441, 416,  87, -16},
		{ -17, 108,  437, 420,  92, -16},
		{ -17, 104,  433, 424,  96, -16},
		{ -17, 100,  429, 429, 100, -17},
		{-187, 105, 1186, 105, -187, 2},
		{-182,  86, 1186, 124, -192, 2},
		{-176,  67, 1185, 143, -197, 2},
		{-170,  49, 1182, 163, -202, 2},
		{-166,  32, 1180, 184, -207, 1},
		{-160,  15, 1176, 204, -212, 1},
		{-155,  -2, 1171, 225, -216, 1},
		{-149, -18, 1166, 246, -221, 0},
		{-145, -34, 1160, 268, -225, 0},
		{-139, -49, 1153, 290, -230,  -1},
		{-134, -63, 1145, 312, -234,  -2},
		{-129, -78, 1137, 334, -238,  -2},
		{-124, -91, 1128, 357, -241,  -5},
		{-119, -104, 1118, 379, -245,  -5},
		{-114, -117, 1107, 402, -248,  -6},
		{-109, -129, 1096, 425, -251,  -8},
		{-104, -141, 1083, 448, -254,  -8},
		{-100, -152, 1071, 471, -257,  -9},
		{ -95, -162, 1057, 494, -259, -11},
		{ -90, -172, 1043, 517, -261, -13},
		{ -86, -181, 1028, 540, -263, -14},
		{ -82, -190, 1013, 563, -264, -16},
		{ -77, -199,  997, 586, -265, -18},
		{ -73, -207,  980, 609, -266, -19},
		{ -69, -214,  963, 632, -266, -22},
		{ -65, -221,  945, 655, -266, -24},
		{ -62, -227,  927, 678, -266, -26},
		{ -58, -233,  908, 700, -265, -28},
		{ -54, -238,  889, 722, -264, -31},
		{ -51, -243,  870, 744, -262, -34},
		{ -48, -247,  850, 766, -260, -37},
		{ -45, -251,  829, 787, -257, -39},
		{ -42, -255,  809, 809, -255, -42}
	},

	/* RGB_COEF_IDX */
	{
		{ 0, 0, 1024,   0, 0, 0},
		{ 0, 0, 1008,  16, 0, 0},
		{ 0, 0,  992,  32, 0, 0},
		{ 0, 0,  976,  48, 0, 0},
		{ 0, 0,  960,  64, 0, 0},
		{ 0, 0,  944,  80, 0, 0},
		{ 0, 0,  928,  96, 0, 0},
		{ 0, 0,  912, 112, 0, 0},
		{ 0, 0,  896, 128, 0, 0},
		{ 0, 0,  880, 144, 0, 0},
		{ 0, 0,  864, 160, 0, 0},
		{ 0, 0,  848, 176, 0, 0},
		{ 0, 0,  832, 192, 0, 0},
		{ 0, 0,  816, 208, 0, 0},
		{ 0, 0,  800, 224, 0, 0},
		{ 0, 0,  784, 240, 0, 0},
		{ 0, 0,  768, 256, 0, 0},
		{ 0, 0,  752, 272, 0, 0},
		{ 0, 0,  736, 288, 0, 0},
		{ 0, 0,  720, 304, 0, 0},
		{ 0, 0,  704, 320, 0, 0},
		{ 0, 0,  688, 336, 0, 0},
		{ 0, 0,  672, 352, 0, 0},
		{ 0, 0,  656, 368, 0, 0},
		{ 0, 0,  640, 384, 0, 0},
		{ 0, 0,  624, 400, 0, 0},
		{ 0, 0,  608, 416, 0, 0},
		{ 0, 0,  592, 432, 0, 0},
		{ 0, 0,  576, 448, 0, 0},
		{ 0, 0,  560, 464, 0, 0},
		{ 0, 0,  544, 480, 0, 0},
		{ 0, 0,  528, 496, 0, 0},
		{ 0, 0,  512, 512, 0, 0},
		{ 0, 0, 1024,   0, 0, 0},
		{ 0, 0, 1008,  16, 0, 0},
		{ 0, 0,  992,  32, 0, 0},
		{ 0, 0,  976,  48, 0, 0},
		{ 0, 0,  960,  64, 0, 0},
		{ 0, 0,  944,  80, 0, 0},
		{ 0, 0,  928,  96, 0, 0},
		{ 0, 0,  912, 112, 0, 0},
		{ 0, 0,  896, 128, 0, 0},
		{ 0, 0,  880, 144, 0, 0},
		{ 0, 0,  864, 160, 0, 0},
		{ 0, 0,  848, 176, 0, 0},
		{ 0, 0,  832, 192, 0, 0},
		{ 0, 0,  816, 208, 0, 0},
		{ 0, 0,  800, 224, 0, 0},
		{ 0, 0,  784, 240, 0, 0},
		{ 0, 0,  768, 256, 0, 0},
		{ 0, 0,  752, 272, 0, 0},
		{ 0, 0,  736, 288, 0, 0},
		{ 0, 0,  720, 304, 0, 0},
		{ 0, 0,  704, 320, 0, 0},
		{ 0, 0,  688, 336, 0, 0},
		{ 0, 0,  672, 352, 0, 0},
		{ 0, 0,  656, 368, 0, 0},
		{ 0, 0,  640, 384, 0, 0},
		{ 0, 0,  624, 400, 0, 0},
		{ 0, 0,  608, 416, 0, 0},
		{ 0, 0,  592, 432, 0, 0},
		{ 0, 0,  576, 448, 0, 0},
		{ 0, 0,  560, 464, 0, 0},
		{ 0, 0,  544, 480, 0, 0},
		{ 0, 0,  528, 496, 0, 0},
		{ 0, 0,  512, 512, 0, 0}
	}
};

/* c0, c1, c2, c3 */
static const int coef_auv_scl_up_tap4[ARSR2P_PHASE_NUM][ARSR2P_TAP4] = {
	{ -3, 254, 6, -1},
	{ -9, 255, 13, -3},
	{ -18, 254, 27, -7},
	{ -23, 245, 44, -10},
	{ -27, 233, 64, -14},
	{ -29, 218, 85, -18},
	{ -29, 198, 108, -21},
	{ -29, 177, 132, -24},
	{ -27, 155, 155, -27}
};

/* c0, c1, c2, c3 */
static const int coef_auv_scl_down_tap4[ARSR2P_PHASE_NUM][ARSR2P_TAP4] = {
	{ 31, 194, 31, 0},
	{ 23, 206, 44, -17},
	{ 14, 203, 57, -18},
	{ 6, 198, 70, -18},
	{ 0, 190, 85, -19},
	{ -5, 180, 99, -18},
	{ -10, 170, 114, -18},
	{ -13, 157, 129, -17},
	{ -15, 143, 143, -15}
};

/* c0, c1, c2, c3, c4, c5 */
static const int coef_y_scl_up_tap6[ARSR2P_PHASE_NUM][ARSR2P_TAP6] = {
	{ 0, -3, 254, 6, -1, 0},
	{ 4, -12, 252, 15, -5, 2},
	{ 7, -22, 245, 31, -9, 4},
	{ 10, -29, 234, 49, -14, 6},
	{ 12, -34, 221, 68, -19, 8},
	{ 13, -37, 206, 88, -24, 10},
	{ 14, -38, 189, 108, -29, 12},
	{ 14, -38, 170, 130, -33, 13},
	{ 14, -36, 150, 150, -36, 14}
};

static const int coef_y_scl_down_tap6[ARSR2P_PHASE_NUM][ARSR2P_TAP6] = {
	{ -22, 43, 214, 43, -22, 0},
	{ -18, 29, 205, 53, -23, 10},
	{ -16, 18, 203, 67, -25, 9},
	{ -13, 9, 198, 80, -26, 8},
	{ -10, 0, 191, 95, -27, 7},
	{ -7, -7, 182, 109, -27, 6},
	{ -5, -14, 174, 124, -27, 4},
	{ -2, -18, 162, 137, -25, 2},
	{ 0, -22, 150, 150, -22, 0}
};


static void aif_bw_sort(dss_aif_bw_t a[], int n)
{
	int i = 0;
	int j = 0;
	dss_aif_bw_t tmp;
	for (; i < n; ++i) {
		for (j = i; j < n - 1; ++j) {
			if (a[j].bw > a[j+1].bw) {
				tmp = a[j];
				a[j] = a[j+1];
				a[j+1] = tmp;
			}
		}
	}
}

struct cnt_info {
	int axi0_cnt;
	int axi1_cnt;
	int chn_idx;
};

static void hisi_dss_aif_handler(struct hisi_drm_crtc *hisi_crtc)
{
	int i;
	uint32_t bw_sum = 0;
	uint32_t tmp = 0;
	dss_aif_bw_t *aif_bw = NULL;
	dss_aif_bw_t aif_bw_tmp[DSS_CHN_MAX_DEFINE] = {{0}};

	int rch_cnt = 0;
	struct cnt_info cnt_info;

	cnt_info.axi0_cnt = 0;
	cnt_info.axi1_cnt = 0;
	cnt_info.chn_idx = 0;

	aif_bw = hisi_crtc->dss_module.aif_bw;

	for (i = 0; i < DSS_CHN_MAX_DEFINE; i++) {
		if (aif_bw[i].is_used) {
			aif_bw_tmp[rch_cnt++] = aif_bw[i];
			bw_sum += aif_bw[i].bw;
		}
	}

	aif_bw_sort(aif_bw_tmp, rch_cnt);

	for (i = 0; i < DSS_CHN_MAX_DEFINE; i++) {
		if (aif_bw_tmp[i].is_used != 1)
			continue;

		tmp += aif_bw_tmp[i].bw;

		if (tmp <= (bw_sum / 2)) {
			aif_bw_tmp[i].axi_sel = AXI_CHN0;
			if (cnt_info.axi0_cnt >= AXI0_MAX_DSS_CHN_THRESHOLD) {
#ifdef CONFIG_HISI_DSS_V510
				aif_bw_tmp[i - AXI0_MAX_DSS_CHN_THRESHOLD].axi_sel = AXI_CHN1;
#else
				aif_bw_tmp[i - AXI0_MAX_DSS_CHN_THRESHOLD].axi_sel = AXI_CHN0;
#endif
				cnt_info.axi1_cnt++;
				cnt_info.axi0_cnt--;
			}
			cnt_info.axi0_cnt++;
		} else {
#ifdef CONFIG_HISI_DSS_V510
			aif_bw_tmp[i].axi_sel = AXI_CHN1;
#else
			aif_bw_tmp[i].axi_sel = AXI_CHN0;
#endif
			cnt_info.axi1_cnt++;
		}

		cnt_info.chn_idx = aif_bw_tmp[i].chn_idx;
		aif_bw[cnt_info.chn_idx] = aif_bw_tmp[i];

		HISI_DRM_DEBUG("aif0, chn_idx=%d, axi_sel=%d, bw=%llu, is_used=%d\n",
			cnt_info.chn_idx, aif_bw_tmp[i].axi_sel, aif_bw_tmp[i].bw, aif_bw_tmp[i].is_used);
	}
}

static int hisi_dss_aif_ch_config(struct dss_module_reg *dss_module, int chn_idx)
{
	dss_aif_t *aif = NULL;
	dss_aif_bw_t *aif_bw = NULL;
	int mid = 0;

	aif = &(dss_module->aif[chn_idx]);
	dss_module->aif_ch_used[chn_idx] = 1;

	aif_bw = &(dss_module->aif_bw[chn_idx]);

	if (aif_bw->is_used != 1) {
		HISI_DRM_ERR("aif_bw->is_used %d is invalid!", aif_bw->is_used);
		return -EINVAL;
	}

	mid = mid_array[chn_idx];
	if (mid < 0 || mid > 0xb) {
		HISI_DRM_ERR("mid %d is invalid!", mid);
		return -EINVAL;
	}

	HISI_DRM_DEBUG("axi_sel=%d\n", aif_bw->axi_sel);
#ifdef CONFIG_HISI_DSS_V510
	aif->aif_ch_ctl = set_bits32(aif->aif_ch_ctl, aif_bw->axi_sel, 1, 0);
	aif->aif_ch_ctl = set_bits32(aif->aif_ch_ctl, (uint32_t)mid, 4, 4);
#else
	aif->aif_ch_ctl = set_bits32(aif->aif_ch_ctl, aif_bw->axi_sel, 2, 0);
#endif
	return 0;
}

static int hisi_dss_aif_config(struct hisi_drm_crtc *hisi_crtc)
{
	int i;
	int ret = 0;
	dss_aif_bw_t *aif_bw;
	struct dss_module_reg *dss_module;

	dss_module = &(hisi_crtc->dss_module);
	aif_bw = dss_module->aif_bw;

	hisi_dss_aif_handler(hisi_crtc);

	for (i = 0; i < DSS_CHN_MAX_DEFINE; i++) {
		if (aif_bw[i].is_used) {
			ret = hisi_dss_aif_ch_config(dss_module, i);
			drm_check_and_return(ret != 0, ret, ERR, "hisi_dss_aif_ch_config failed");

			hisi_dss_aif_ch_set_reg(hisi_crtc, dss_module->aif_ch_base[i],
				&(dss_module->aif[i]));
		}
	}

	return ret;
}

static int hisi_dss_ovl_base_config(struct dss_module_reg *dss_module, u32 xres,
	u32 yres, int ovl_idx)
{
	dss_ovl_t *ovl = NULL;
	uint32_t ovl_bg_color;
	int block_size = 0x7FFF;

	ovl = &(dss_module->ov[ovl_idx]);
	dss_module->ov_used[ovl_idx] = 1;

	ovl->ovl_size = set_bits32(ovl->ovl_size, DSS_WIDTH(xres), 15, 0);
	ovl->ovl_size = set_bits32(ovl->ovl_size, DSS_HEIGHT(yres), 15, 16);
#ifdef CONFIG_HISI_FB_OV_BASE_USED
	ovl_bg_color = 0xFFFF0000;
#else
	ovl_bg_color = 0x00000000;
#endif
	ovl->ovl_bg_color = set_bits32(ovl->ovl_bg_color, ovl_bg_color, 32, 0);
	ovl->ovl_bg_color_alpha = set_bits32(ovl->ovl_bg_color_alpha, 0x3FF, 32, 0);
	ovl->ovl_dst_startpos = set_bits32(ovl->ovl_dst_startpos, 0x0, 32, 0);
	ovl->ovl_dst_endpos = set_bits32(ovl->ovl_dst_endpos, DSS_WIDTH(xres), 15, 0);
	ovl->ovl_dst_endpos = set_bits32(ovl->ovl_dst_endpos, DSS_HEIGHT(yres), 15, 16);
	ovl->ovl_gcfg = set_bits32(ovl->ovl_gcfg, 0x1, 1, 0);
	ovl->ovl_gcfg = set_bits32(ovl->ovl_gcfg, 0x1, 1, 16);
	ovl->ovl_block_size = set_bits32(ovl->ovl_block_size, block_size, 15, 16);
	return 0;
}


static int hisi_dss_mctl_ov_config(struct dss_module_reg *dss_module,
	int ovl_idx, bool is_first_ov_block)
{
	dss_mctl_t *mctl = NULL;
	dss_mctl_sys_t *mctl_sys = NULL;

	if ((ovl_idx < DSS_OVL0) || (ovl_idx >= DSS_OVL_IDX_MAX)) {
		HISI_DRM_ERR("ovl_idx is invalid");
		return -EINVAL;
	}

	/* MCTL_MUTEX */
	mctl = &(dss_module->mctl[ovl_idx]);
	dss_module->mctl_used[ovl_idx] = 1;

	if (ovl_idx == DSS_OVL0) {
		mctl->ctl_mutex_itf = set_bits32(mctl->ctl_mutex_itf, 0x1, 2, 0);
		mctl->ctl_mutex_dbuf = set_bits32(mctl->ctl_mutex_dbuf, 0x1, 2, 0);
	} else if (ovl_idx == DSS_OVL1) {
		mctl->ctl_mutex_itf = set_bits32(mctl->ctl_mutex_itf, 0x2, 2, 0);
		mctl->ctl_mutex_dbuf = set_bits32(mctl->ctl_mutex_dbuf, 0x2, 2, 0);
	}

	mctl->ctl_mutex_ov = set_bits32(mctl->ctl_mutex_ov, 1 << (uint32_t)ovl_idx, 4, 0);

	/* MCTL_SYS */
	mctl_sys = &(dss_module->mctl_sys);
	dss_module->mctl_sys_used = 1;

	/* ov base pattern */
	mctl_sys->chn_ov_sel[ovl_idx] = set_bits32(mctl_sys->chn_ov_sel[ovl_idx], 0xE, 4, 0);
	mctl_sys->chn_ov_sel_used[ovl_idx] = 1;

	if ((ovl_idx == DSS_OVL0) || (ovl_idx == DSS_OVL1)) {
		if (is_first_ov_block) {
			mctl_sys->ov_flush_en[ovl_idx] =
				set_bits32(mctl_sys->ov_flush_en[ovl_idx], 0xd, 4, 0);
		} else {
			mctl_sys->ov_flush_en[ovl_idx] =
				set_bits32(mctl_sys->ov_flush_en[ovl_idx], 0x1, 1, 0);
		}
		mctl_sys->ov_flush_en_used[ovl_idx] = 1;
	} else {
		mctl_sys->ov_flush_en[ovl_idx] =
			set_bits32(mctl_sys->ov_flush_en[ovl_idx], 0x1, 1, 0);
		mctl_sys->ov_flush_en_used[ovl_idx] = 1;
	}

	return 0;
}

static void hisi_dss_mctl_ov_set_ctl_dbg_reg(struct hisi_drm_crtc *hisi_crtc,
	char __iomem *mctl_base)
{
	if (hisi_crtc->enable_cmdlist) {
		set_reg(mctl_base + MCTL_CTL_DBG, 0xB03A20, 32, 0);
		set_reg(mctl_base + MCTL_CTL_TOP, 0x1, 32, 0);
	} else {
		set_reg(mctl_base + MCTL_CTL_DBG, 0xB13A00, 32, 0);
		if (hisi_crtc->crtc_id == PRIMARY_IDX)
			set_reg(mctl_base + MCTL_CTL_TOP, 0x2, 32, 0);
		else if (hisi_crtc->crtc_id == EXTERNAL_IDX)
			set_reg(mctl_base + MCTL_CTL_TOP, 0x3, 32, 0);
		else
			;
	}
}

static void hisi_dss_mctl_ov_set_reg(struct hisi_drm_crtc *hisi_crtc,
	char __iomem *mctl_base, dss_mctl_t *s_mctl, int ovl_idx)
{
	drm_check_and_void_return(mctl_base == NULL, "mctl_base is NULL");

	if ((ovl_idx < DSS_OVL0) || (ovl_idx >= DSS_OVL_IDX_MAX)) {
		HISI_DRM_ERR("ovl_idx  is is out of the range !\n");
		return;
	}

	if ((ovl_idx == DSS_OVL0) || (ovl_idx == DSS_OVL1)) {
		hisi_set_reg(hisi_crtc, mctl_base + MCTL_CTL_MUTEX_DBUF,
			s_mctl->ctl_mutex_dbuf, 32, 0);
		hisi_dss_mctl_ov_set_ctl_dbg_reg(hisi_crtc, mctl_base);
	}

	hisi_set_reg(hisi_crtc, mctl_base + MCTL_CTL_MUTEX_OV, s_mctl->ctl_mutex_ov, 32, 0);
}


static void hisi_dss_ovl_set_reg(struct hisi_drm_crtc *hisi_crtc,
	char __iomem *ovl_base, dss_ovl_t *s_ovl, int ovl_idx)
{
	int i = 0;

	drm_check_and_void_return(ovl_base == NULL, "ovl_base is NULL");

	hisi_set_reg(hisi_crtc, ovl_base + OV8_REG_DEFAULT, 0x1, 32, 0);
	hisi_set_reg(hisi_crtc, ovl_base + OV8_REG_DEFAULT, 0x0, 32, 0);
	hisi_set_reg(hisi_crtc, ovl_base + OV8_BLOCK_DBG, 0x4, 32, 0);

	hisi_set_reg(hisi_crtc, ovl_base + OV_SIZE, s_ovl->ovl_size, 32, 0);
	hisi_set_reg(hisi_crtc, ovl_base + OV_BG_COLOR_RGB, s_ovl->ovl_bg_color, 32, 0);
	hisi_set_reg(hisi_crtc, ovl_base + OV_BG_COLOR_A, s_ovl->ovl_bg_color_alpha, 32, 0);
	hisi_set_reg(hisi_crtc, ovl_base + OV_DST_STARTPOS, s_ovl->ovl_dst_startpos, 32, 0);
	hisi_set_reg(hisi_crtc, ovl_base + OV_DST_ENDPOS, s_ovl->ovl_dst_endpos, 32, 0);
	hisi_set_reg(hisi_crtc, ovl_base + OV_GCFG, s_ovl->ovl_gcfg, 32, 0);

	for (i = 0; i < OV_8LAYER_NUM; i++) {
		if (s_ovl->ovl_layer_used[i] == 1) {
			hisi_set_reg(hisi_crtc, ovl_base + OV_LAYER0_POS + i * 0x60,
				s_ovl->ovl_layer[i].layer_pos, 32, 0);
			hisi_set_reg(hisi_crtc, ovl_base + OV_LAYER0_SIZE + i * 0x60,
				s_ovl->ovl_layer[i].layer_size, 32, 0);
			hisi_set_reg(hisi_crtc, ovl_base + OV_LAYER0_PATTERN_RGB + i * 0x60,
				s_ovl->ovl_layer[i].layer_pattern, 32, 0);
			hisi_set_reg(hisi_crtc, ovl_base + OV_LAYER0_PATTERN_A + i * 0x60,
				s_ovl->ovl_layer[i].layer_pattern_alpha, 32, 0);
			hisi_set_reg(hisi_crtc, ovl_base + OV_LAYER0_ALPHA_MODE + i * 0x60,
				s_ovl->ovl_layer[i].layer_alpha, 32, 0);
			hisi_set_reg(hisi_crtc, ovl_base + OV_LAYER0_ALPHA_A + i * 0x60,
				s_ovl->ovl_layer[i].layer_alpha_a, 32, 0);
			hisi_set_reg(hisi_crtc, ovl_base + OV_LAYER0_CFG + i * 0x60,
				s_ovl->ovl_layer[i].layer_cfg, 32, 0);
			hisi_set_reg(hisi_crtc, ovl_base + OV_LAYER0_PSPOS + i * 0x60,
				s_ovl->ovl_layer_pos[i].layer_pspos, 32, 0);
			hisi_set_reg(hisi_crtc, ovl_base + OV_LAYER0_PEPOS + i * 0x60,
				s_ovl->ovl_layer_pos[i].layer_pepos, 32, 0);
		} else {
			hisi_set_reg(hisi_crtc, ovl_base + OV_LAYER0_POS + i * 0x60,
				s_ovl->ovl_layer[i].layer_pos, 32, 0);
			hisi_set_reg(hisi_crtc, ovl_base + OV_LAYER0_SIZE + i * 0x60,
				s_ovl->ovl_layer[i].layer_size, 32, 0);
			hisi_set_reg(hisi_crtc, ovl_base + OV_LAYER0_CFG + i * 0x60,
				s_ovl->ovl_layer[i].layer_cfg, 32, 0);
		}
	}
	hisi_set_reg(hisi_crtc, ovl_base + OV8_BLOCK_SIZE, s_ovl->ovl_block_size, 32, 0);
}

static void hisi_dss_mctl_sys_set_reg(struct hisi_drm_crtc *hisi_crtc,
	char __iomem *mctl_sys_base, dss_mctl_sys_t *s_mctl_sys, int ovl_idx)
{
	int k = 0;

	drm_check_and_void_return(mctl_sys_base == NULL, "mctl_sys_base is NULL");

	if ((ovl_idx < DSS_OVL0) || (ovl_idx >= DSS_OVL_IDX_MAX)) {
		HISI_DRM_ERR("ovl_idx is out of the range!\n");
		return;
	}

	if (s_mctl_sys->chn_ov_sel_used[ovl_idx]) {
		hisi_set_reg(hisi_crtc, mctl_sys_base + MCTL_RCH_OV0_SEL + (ovl_idx * 0x4),
			s_mctl_sys->chn_ov_sel[ovl_idx], 32, 0);
		hisi_set_reg(hisi_crtc, mctl_sys_base + MCTL_RCH_OV0_SEL1 + (ovl_idx * 0x4),
			s_mctl_sys->chn_ov_sel1[ovl_idx], 32, 0);
	}

	for (k = 0; k < DSS_WCH_MAX; k++) {
		if (s_mctl_sys->wch_ov_sel_used[k]) {
			hisi_set_reg(hisi_crtc, mctl_sys_base + MCTL_WCH_OV2_SEL + (k * 0x4),
				s_mctl_sys->wchn_ov_sel[k], 32, 0);
		}
	}

	if (s_mctl_sys->ov_flush_en_used[ovl_idx]) {
		hisi_set_reg(hisi_crtc, mctl_sys_base + MCTL_OV0_FLUSH_EN + (ovl_idx * 0x4),
			s_mctl_sys->ov_flush_en[ovl_idx], 32, 0);
	}
}

static int hisi_dss_ov_module_set_regs(struct hisi_drm_crtc *hisi_crtc, int ovl_idx,
	bool enable_cmdlist, int task_end, int last, bool is_first_ov_block)
{

	struct dss_module_reg *dss_module = NULL;
	struct hisi_cmdlist *cmdlist_info = NULL;
	int i;
	int ret = 0;
	uint32_t tmp = 0;

	dss_module = &(hisi_crtc->dss_module);
	i = ovl_idx;
	if ((i < 0) || (i >= DSS_OVL_IDX_MAX)) {
		HISI_DRM_ERR("ovl_idx More than Array Upper Limit!\n");
		return -1;
	}

	cmdlist_info = &(hisi_crtc->cmdlist_info);
	if (enable_cmdlist) {
		/* add ov cmdlist */
		tmp = (0x1 << (uint32_t)(DSS_CMDLIST_OV0 + ovl_idx));
		cmdlist_info->cmdlist_idx = DSS_CMDLIST_OV0 + ovl_idx;

		ret = hisi_cmdlist_create_node(cmdlist_info, tmp);
		drm_check_and_return(ret != 0, ret, ERR, "hisi_cmdlist_create_node failed");
	}

	if (dss_module->mctl_used[i] == 1)
		hisi_dss_mctl_ov_set_reg(hisi_crtc, dss_module->mctl_base[i],
			&(dss_module->mctl[i]), ovl_idx);

	if (dss_module->ov_used[i] == 1)
		hisi_dss_ovl_set_reg(hisi_crtc, dss_module->ov_base[i],
			&(dss_module->ov[i]), ovl_idx);

	if (dss_module->mctl_sys_used == 1)
		hisi_dss_mctl_sys_set_reg(hisi_crtc, dss_module->mctl_sys_base,
			&(dss_module->mctl_sys), ovl_idx);

	return 0;
}

static void hisi_dss_mif_on(struct dss_hw_ctx *ctx)
{
	char __iomem *dss_base;
	char __iomem *mif_base;

	dss_base = ctx->dss_base;
	if (!dss_base) {
		HISI_DRM_ERR("dss_base is NULL!\n");
		return;
	}
#ifdef CONFIG_HISI_DSS_V600
	return;
#endif
	mif_base = ctx->dss_base + DSS_MIF_OFFSET;

	set_reg(mif_base + MIF_ENABLE, 0x1, 1, 0);

	set_reg(dss_base + MIF_CH0_OFFSET + MIF_CTRL0, 0x1, 1, 0);
	set_reg(dss_base + MIF_CH1_OFFSET + MIF_CTRL0, 0x1, 1, 0);
	set_reg(dss_base + MIF_CH2_OFFSET + MIF_CTRL0, 0x1, 1, 0);
	set_reg(dss_base + MIF_CH3_OFFSET + MIF_CTRL0, 0x1, 1, 0);
	set_reg(dss_base + MIF_CH4_OFFSET + MIF_CTRL0, 0x1, 1, 0);
	set_reg(dss_base + MIF_CH5_OFFSET + MIF_CTRL0, 0x1, 1, 0);
	set_reg(dss_base + MIF_CH6_OFFSET + MIF_CTRL0, 0x1, 1, 0);
	set_reg(dss_base + MIF_CH7_OFFSET + MIF_CTRL0, 0x1, 1, 0);
	set_reg(dss_base + MIF_CH8_OFFSET + MIF_CTRL0, 0x1, 1, 0);
	set_reg(dss_base + MIF_CH9_OFFSET + MIF_CTRL0, 0x1, 1, 0);
	set_reg(dss_base + MIF_CH10_OFFSET + MIF_CTRL0, 0x1, 1, 0);
	set_reg(dss_base + MIF_CH11_OFFSET + MIF_CTRL0, 0x1, 1, 0);
	set_reg(mif_base + AIF_CMD_RELOAD, 0x1, 1, 0);
}

static void hisi_dss_smmu_on(struct dss_hw_ctx *ctx)
{
	void __iomem *smmu_base;
	uint32_t fama_ptw_msb;

	int idx0 = 0;
	int idx1 = 0;
	int idx2 = 0;

	if (!ctx->dss_base) {
		HISI_DRM_ERR("dss_base is NULL!\n");
		return;
	}

	smmu_base = ctx->dss_base + DSS_SMMU_OFFSET;
#ifdef CONFIG_DRM_DSS_IOMMU
	set_reg(smmu_base + SMMU_SCR, 0x0, 1, 0);  /* global bypass cancel */
#else
	set_reg(smmu_base + SMMU_SCR, 0x1, 1, 0);  /* global bypass cancel */
#endif
	set_reg(smmu_base + SMMU_SCR, 0x1, 8, 20); /* ptw_mid */
	set_reg(smmu_base + SMMU_SCR, 0xf, 4, 16); /* pwt_pf */
	set_reg(smmu_base + SMMU_SCR, 0x7, 3, 3);  /* interrupt cachel1 cach3l2 en */
	set_reg(smmu_base + SMMU_LP_CTRL, 0x1, 1, 0);  /* auto_clk_gt_en */
	set_reg(smmu_base + SMMU_CB_TTBCR, 0x1, 1, 0);
	set_reg(smmu_base + SMMU_ERR_RDADDR, 0x7FF00000, 32, 0);
	set_reg(smmu_base + SMMU_ERR_WRADDR, 0x7FFF0000, 32, 0);

	idx0 = 36; /* debug stream id */
	idx1 = 37; /* cmd unsec stream id */
	idx2 = 38; /* cmd sec stream id */

	set_reg(smmu_base + SMMU_SMRx_NS + idx0 * 0x4, 0x1d, 32, 0);
	set_reg(smmu_base + SMMU_SMRx_NS + idx1 * 0x4, 0x1d, 32, 0);
	set_reg(smmu_base + SMMU_SMRx_NS + idx2 * 0x4, 0x1d, 32, 0);

	set_reg(smmu_base + SMMU_RLD_EN0_NS, 0xffffffff, 32, 0);
	set_reg(smmu_base + SMMU_RLD_EN1_NS, 0xffffffcf, 32, 0);
	set_reg(smmu_base + SMMU_RLD_EN2_NS, 0x00ffffff, 32, 0);

#ifdef CONFIG_DRM_DSS_IOMMU
	/* TTBR0 */
	set_reg(smmu_base + SMMU_CB_TTBR0,
		(uint32_t)hisi_dss_domain_get_ttbr(ctx->pdev), 32, 0);

	fama_ptw_msb = (hisi_dss_domain_get_ttbr(ctx->pdev) >> 32) & 0x7F;
	set_reg(smmu_base + SMMU_FAMA_CTRL0, 0x80, 14, 0);
	set_reg(smmu_base + SMMU_FAMA_CTRL1, fama_ptw_msb, 7, 0);
#endif
}

static int hisi_dss_arsr2p_write_coefs(struct hisi_drm_crtc *hisi_crtc, bool enable_cmdlist,
	char __iomem *addr, const int **p, int row, int col)
{
	int coef_value = 0;
	int coef_num = 0;
	int i = 0;
	int j = 0;

	drm_check_and_return(addr == NULL, -EINVAL, ERR, "addr is NULL");

	if ((row != ARSR2P_PHASE_NUM) || ((col != ARSR2P_TAP4) && (col != ARSR2P_TAP6))) {
		HISI_DRM_ERR("arsr2p filter coefficients is err, arsr2p_phase_num = %d, " \
			"arsr2p_tap_num = %d\n", row, col);
		return -EINVAL;
	}
	coef_num = (col == ARSR2P_TAP4 ? 2 : 3);

	for (i = 0; i < row; i++) {
		for (j = 0; j < 2; j++) {
			if (coef_num == 2) {
				coef_value = (*((int *)p + i * col + j * coef_num) & 0x1FF) |
					((*((int *)p + i * col + j * coef_num + 1)  & 0x1FF) << 9);
			} else {
				coef_value = (*((int *)p + i * col + j * coef_num) & 0x1FF) |
					((*((int *)p + i * col + j * coef_num + 1)  & 0x1FF) << 9) |
					((*((int *)p + i * col + j * coef_num + 2)  & 0x1FF) << 18);
			}

			hisi_set_reg(hisi_crtc, addr + 0x8 * i + j * 0x4, coef_value, 32, 0);
		}
	}
	return 0;
}

static void hisi_dss_arsr2p_write_config_coefs(struct hisi_drm_crtc *hisi_crtc, bool enable_cmdlist,
	char __iomem *addr, const int **scl_down, const int **scl_up, int row, int col)
{
	int ret;

	ret = hisi_dss_arsr2p_write_coefs(hisi_crtc, enable_cmdlist, addr, scl_down, row, col);
	if (ret < 0) {
		HISI_DRM_ERR("Error to write COEF_SCL_DOWN coefficients.\n");
		return;
	}

	ret = hisi_dss_arsr2p_write_coefs(hisi_crtc, enable_cmdlist, addr +
		ARSR2P_SCL_UP_OFFSET, scl_up, row, col);
	if (ret < 0) {
		HISI_DRM_ERR("Error to write COEF_SCL_UP coefficients.\n");
		return;
	}
}

static void hisi_dss_arsr2p_coef_on(struct hisi_drm_crtc *hisi_crtc, bool enable_cmdlist)
{
	uint32_t module_base = 0;
	char __iomem *arsr2p_base;
	char __iomem *coefy_v = NULL;
	char __iomem *coefa_v = NULL;
	char __iomem *coefuv_v = NULL;
	struct dss_hw_ctx *ctx;

	ctx = hisi_crtc->ctx;

	module_base = g_dss_module_base[DSS_RCHN_V0][MODULE_ARSR2P_LUT];
	coefy_v = ctx->dss_base + module_base + ARSR2P_LUT_COEFY_V_OFFSET;
	coefa_v = ctx->dss_base + module_base + ARSR2P_LUT_COEFA_V_OFFSET;
	coefuv_v = ctx->dss_base + module_base + ARSR2P_LUT_COEFUV_V_OFFSET;
	arsr2p_base = ctx->dss_base + g_dss_module_base[DSS_RCHN_V0][MODULE_ARSR2P];

	/* COEFY_V COEFY_H */
	hisi_dss_arsr2p_write_config_coefs(hisi_crtc, enable_cmdlist, coefy_v, (const int **)coef_y_scl_down_tap6,
		(const int **)coef_y_scl_up_tap6, ARSR2P_PHASE_NUM, ARSR2P_TAP6);
	hisi_dss_arsr2p_write_config_coefs(hisi_crtc, enable_cmdlist,
		coefy_v + ARSR2P_COEF_H0_OFFSET, (const int **)coef_y_scl_down_tap6,
		(const int **)coef_y_scl_up_tap6, ARSR2P_PHASE_NUM, ARSR2P_TAP6);
	hisi_dss_arsr2p_write_config_coefs(hisi_crtc, enable_cmdlist,
		coefy_v + ARSR2P_COEF_H1_OFFSET, (const int **)coef_y_scl_down_tap6,
		(const int **)coef_y_scl_up_tap6, ARSR2P_PHASE_NUM, ARSR2P_TAP6);

	/* COEFA_V COEFA_H */
	hisi_dss_arsr2p_write_config_coefs(hisi_crtc, enable_cmdlist,
		coefa_v, (const int **)coef_auv_scl_down_tap4,
		(const int **)coef_auv_scl_up_tap4, ARSR2P_PHASE_NUM, ARSR2P_TAP4);
	hisi_dss_arsr2p_write_config_coefs(hisi_crtc, enable_cmdlist,
		coefa_v + ARSR2P_COEF_H0_OFFSET, (const int **)coef_auv_scl_down_tap4,
		(const int **)coef_auv_scl_up_tap4, ARSR2P_PHASE_NUM, ARSR2P_TAP4);
	hisi_dss_arsr2p_write_config_coefs(hisi_crtc, enable_cmdlist,
		coefa_v + ARSR2P_COEF_H1_OFFSET, (const int **)coef_auv_scl_down_tap4,
		(const int **)coef_auv_scl_up_tap4, ARSR2P_PHASE_NUM, ARSR2P_TAP4);

	/* COEFUV_V COEFUV_H */
	hisi_dss_arsr2p_write_config_coefs(hisi_crtc, enable_cmdlist,
		coefuv_v, (const int **)coef_auv_scl_down_tap4,
		(const int **)coef_auv_scl_up_tap4, ARSR2P_PHASE_NUM, ARSR2P_TAP4);
	hisi_dss_arsr2p_write_config_coefs(hisi_crtc, enable_cmdlist,
		coefuv_v + ARSR2P_COEF_H0_OFFSET, (const int **)coef_auv_scl_down_tap4,
		(const int **)coef_auv_scl_up_tap4, ARSR2P_PHASE_NUM, ARSR2P_TAP4);
	hisi_dss_arsr2p_write_config_coefs(hisi_crtc, enable_cmdlist,
		coefuv_v + ARSR2P_COEF_H1_OFFSET, (const int **)coef_auv_scl_down_tap4,
		(const int **)coef_auv_scl_up_tap4, ARSR2P_PHASE_NUM, ARSR2P_TAP4);
}

static int hisi_dss_scl_write_coefs(struct hisi_drm_crtc *hisi_crtc, bool enable_cmdlist,
	char __iomem *addr, const int **p, int row, int col)
{
	int groups[3] = {0};
	int offset;
	int valid_num;
	uint32_t i;
	uint32_t j;
	uint32_t k;

	drm_check_and_return(addr == NULL, -EINVAL, ERR, "addr is NULL");

	if ((row != PHASE_NUM) || (col < TAP4 || col > TAP6)) {
		HISI_DRM_ERR("SCF filter coefficients is err, phase_num = %d, tap_num = %d\n",
			row, col);
		return -EINVAL;
	}

	/*byte*/
	offset = (col == TAP4) ? 8 : 16;
	valid_num = (offset == 16) ? 3 : 2;

	for (i = 0; i < row; i++) {
		for (j = 0; j < col; j += 2) {
			if ((col % 2) && (j == col - 1)) {
				groups[j / 2] = (*((int *)p + i * col + j) & 0xFFF) | (0 << 16);
			} else {
				groups[j / 2] = (*((int *)p + i * col + j) & 0xFFF) |
					(*((int *)p + i * col + j + 1) << 16);
			}
		}

		for (k = 0; k < valid_num; k++) {
			hisi_set_reg(hisi_crtc, addr + offset * i + k * sizeof(int), groups[k], 32, 0);
			groups[k] = 0;
		}
	}

	return 0;
}


static void hisi_dss_scl_coef_on(struct hisi_drm_crtc *hisi_crtc, bool enable_cmdlist, int coef_lut_idx)
{
	int i;
	uint32_t module_base = 0;
	char __iomem *h0_y_addr = NULL;
	char __iomem *y_addr = NULL;
	char __iomem *uv_addr = NULL;
	int ret = 0;
	struct dss_hw_ctx *ctx;

	ctx = hisi_crtc->ctx;

	if (!ctx->dss_base) {
		HISI_DRM_ERR("dss_base is NULL!\n");
		return;
	}

	for (i = 0; i < DSS_CHN_MAX_DEFINE; i++) {
		module_base = g_dss_module_base[i][MODULE_SCL_LUT];
		if (module_base != 0) {
			h0_y_addr = ctx->dss_base + module_base + DSS_SCF_H0_Y_COEF_OFFSET;
			y_addr = ctx->dss_base + module_base + DSS_SCF_Y_COEF_OFFSET;
			uv_addr = ctx->dss_base + module_base + DSS_SCF_UV_COEF_OFFSET;

			g_scf_lut_chn_coef_idx[i] = coef_lut_idx;

			ret = hisi_dss_scl_write_coefs(hisi_crtc, enable_cmdlist, h0_y_addr,
				(const int **)coef_lut_tap6[coef_lut_idx], PHASE_NUM, TAP6);
			if (ret < 0)
				HISI_DRM_ERR("Error to write H0_Y_COEF coefficients.\n");

			if (i == DSS_RCHN_V0) {
				hisi_dss_arsr2p_coef_on(hisi_crtc, enable_cmdlist);
				continue;
			}

			ret = hisi_dss_scl_write_coefs(hisi_crtc, enable_cmdlist, y_addr,
				(const int **)coef_luv_tap5[coef_lut_idx], PHASE_NUM, TAP5);
			if (ret < 0)
				HISI_DRM_ERR("Error to write Y_COEF coefficients.\n");

			ret = hisi_dss_scl_write_coefs(hisi_crtc, enable_cmdlist, uv_addr,
				(const int **)coef_lut_tap4[coef_lut_idx], PHASE_NUM, TAP4);
			if (ret < 0)
				HISI_DRM_ERR("Error to write UV_COEF coefficients.\n");
		}
	}
}

static void hisi_dss_on(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = hisi_crtc->ctx;

	if (ctx->dss_sr_refcnt++ == 0) {
		hisi_dss_mif_on(ctx);
#ifndef CONFIG_DRM_SMMU_V3
		hisi_dss_smmu_on(ctx);
#endif
		hisi_dss_scl_coef_on(hisi_crtc, false, SCL_COEF_YUV_IDX);
		if (hisi_crtc->enable_cmdlist)
			hisi_dss_cmdlist_qos_on(ctx->dss_base);
	}
#ifdef CONFIG_DRM_SMMU_V3
	/* offline need vote smmu connect, but not for mdc */
	hisi_drm_smmuv3_on(hisi_crtc);
#endif

	HISI_DRM_INFO("dss_sr_refcnt=%d\n", ctx->dss_sr_refcnt);
}

int hisi_dss_off(struct hisi_drm_crtc *hisi_crtc)
{
	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL!\n");
		return -EINVAL;
	}

	if (!hisi_crtc->ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return -EINVAL;
	}

	hisi_crtc->ctx->dss_sr_refcnt--;
	HISI_DRM_INFO("dss_sr_refcnt=%d", hisi_crtc->ctx->dss_sr_refcnt);

	return 0;
}

static void hisi_dss_mctl_on(struct hisi_drm_crtc *hisi_crtc)
{
	char __iomem *mctl_base = NULL;
	char __iomem *mctl_sys_base = NULL;
	int i;
	int tmp;
	struct dss_hw_ctx *ctx = hisi_crtc->ctx;

	int mctl_idx = hisi_crtc->mclt_idx;

	if ((mctl_idx < DSS_MCTL0) || (mctl_idx >= DSS_MCTL_IDX_MAX)) {
		HISI_DRM_ERR("mctl_idx is invalid");
		return;
	}

	mctl_base = ctx->dss_base + g_dss_module_ovl_base[mctl_idx][MODULE_MCTL_BASE];
	mctl_sys_base = ctx->dss_base + DSS_MCTRL_SYS_OFFSET;

	set_reg(mctl_base + MCTL_CTL_EN, 0x1, 32, 0);
	if ((mctl_idx == DSS_MCTL0) || (mctl_idx == DSS_MCTL1))
		set_reg(mctl_base + MCTL_CTL_MUTEX_ITF, mctl_idx + 1, 32, 0);

	if (hisi_crtc->enable_cmdlist) {
		tmp = MCTL_MOD_DBG_CH_NUM + MCTL_MOD_DBG_OV_NUM +
			MCTL_MOD_DBG_DBUF_NUM + MCTL_MOD_DBG_SCF_NUM;
		for (i = 0; i < tmp; i++) {
			if (i < MCTL_MOD_DBG_CH_NUM)
				set_reg(mctl_sys_base + MCTL_MOD0_DBG + i * 0x4, 0xA8000, 32, 0);
			else
				set_reg(mctl_sys_base + MCTL_MOD0_DBG + i * 0x4, 0xA0000, 32, 0);
		}

		for (i = 0; i < MCTL_MOD_DBG_ITF_NUM; i++)
			set_reg(mctl_sys_base + MCTL_MOD17_DBG +
				i * 0x4, 0xA0F00, 32, 0);

		set_reg(mctl_sys_base + MCTL_MOD19_DBG, 0xA8000, 32, 0);
		if (ctx->is_pre_enabled)
			set_reg(mctl_base + MCTL_CTL_TOP, 0x1, 32, 0);

	} else {
		set_reg(mctl_base + MCTL_CTL_DBG, 0xB13A00, 32, 0);
		if (is_mipi_cmd_mode(hisi_crtc)) {
			set_reg(mctl_base + MCTL_CTL_TOP, 0x1, 32, 0);
		} else {
			if (mctl_idx == DSS_MCTL0)
				set_reg(mctl_base + MCTL_CTL_TOP, 0x2, 32, 0);
			else if (mctl_idx == DSS_MCTL1)
				set_reg(mctl_base + MCTL_CTL_TOP, 0x3, 32, 0);
			else
				set_reg(mctl_base + MCTL_CTL_TOP, 0x1, 32, 0);
		}
	}
}

static int hisi_overlay_fastboot(struct hisi_drm_crtc *hisi_crtc)
{
	int ret;
	enum dss_chn_idx rchn_to_clear;

#ifdef CONFIG_HISI_DSS_V510
	rchn_to_clear = DSS_RCHN_D0;
#else
	rchn_to_clear = DSS_RCHN_D2;
#endif
	ret = memcpy_s(&(hisi_crtc->dss_module_default.rdma[rchn_to_clear]), sizeof(dss_rdma_t),
		&(hisi_crtc->dss_module_default.rdma[DSS_RCHN_D3]), sizeof(dss_rdma_t));
	drm_check_and_return(ret != EOK, -EINVAL, ERR, "rdma copy failed!");
	ret = memcpy_s(&(hisi_crtc->dss_module_default.dfc[rchn_to_clear]), sizeof(dss_dfc_t),
		&(hisi_crtc->dss_module_default.dfc[DSS_RCHN_D3]), sizeof(dss_dfc_t));
	drm_check_and_return(ret != EOK, -EINVAL, ERR, "dfc copy failed!");
	ret = memcpy_s(&(hisi_crtc->dss_module_default.ov[DSS_OVL0].ovl_layer[0]), sizeof(dss_ovl_layer_t),
		&(hisi_crtc->dss_module_default.ov[DSS_OVL0].ovl_layer[1]), sizeof(dss_ovl_layer_t));
	drm_check_and_return(ret != EOK, ret, ERR, "ov copy failed!");

	ret = memset_s(&(hisi_crtc->dss_module_default.mctl_ch[rchn_to_clear]), sizeof(dss_mctl_ch_t), 0, sizeof(dss_mctl_ch_t));
	drm_check_and_return(ret != EOK, -EINVAL, ERR, "mctl_ch memset failed!");
	ret = memset_s(&(hisi_crtc->dss_module_default.mctl[DSS_OVL0]), sizeof(dss_mctl_t), 0, sizeof(dss_mctl_t));
	drm_check_and_return(ret != EOK, -EINVAL, ERR, "mctl memset failed!");

	hisi_crtc->dss_module_default.mctl_sys.chn_ov_sel[DSS_OVL0] = 0xFFFFFFFF;
	hisi_crtc->dss_module_default.mctl_sys.ov_flush_en[DSS_OVL0] = 0x0;

	if (hisi_crtc->vactive0_start_flag == 0)
		hisi_crtc->vactive0_start_flag = 1;

	return 0;
}

static int hisi_flush_ov_base_frame(struct hisi_drm_crtc *hisi_crtc)
{
	int ret;

	HISI_DRM_INFO("+");
	ret = hisi_dss_module_init(hisi_crtc);
	drm_check_and_return(ret != 0, ret, ERR, "hisi_dss_module_init failed");

	hisi_dss_mctl_mutex_lock(hisi_crtc);
	ret = hisi_ov_base_config(hisi_crtc);
	if (ret != 0) {
		HISI_DRM_ERR("hisi_dss_module_init failed! ret = %d\n", ret);
		hisi_dss_mctl_mutex_unlock(hisi_crtc);
		return ret;
	}
	hisi_dss_mctl_mutex_unlock(hisi_crtc);
	crtc_single_frame_update(hisi_crtc);

	hisi_crtc->frame_no++;
	hisi_crtc->layer_mask = 0;
	hisi_crtc->pre_layer_mask = 0;

	return ret;
}

int hisi_ov_base_config(struct hisi_drm_crtc *hisi_crtc)
{
	int i, ret;
	int ovl_idx;
	struct drm_display_mode *adj_mode = NULL;
	bool enable_cmdlist = false;

	drm_check_and_return(hisi_crtc == NULL, -EINVAL, ERR, "hisi_crtc is NULL");

	enable_cmdlist = hisi_crtc->enable_cmdlist;
	adj_mode = &hisi_crtc->base.state->adjusted_mode;
	ovl_idx =  hisi_crtc->ovl_idx;

	for (i = 0; i < DSS_CHN_MAX; i++) {
		if (hisi_crtc->pre_layer_mask & (1 << i))
			hisi_dss_disable_channel(hisi_crtc, i);
	}

	ret = hisi_dss_aif_config(hisi_crtc);
	drm_check_and_return(ret != 0, ret, ERR, "hisi_dss_aif_config failed");

	ret = hisi_dss_ovl_base_config(&hisi_crtc->dss_module,
		adj_mode->hdisplay, adj_mode->vdisplay, ovl_idx);
	drm_check_and_return(ret != 0, ret, ERR, "hisi_dss_ovl_base_config failed");

	ret = hisi_dss_mctl_ov_config(&hisi_crtc->dss_module, ovl_idx, true);
	drm_check_and_return(ret != 0, ret, ERR, "hisi_dss_mctl_ov_config failed");

	ret = hisi_dss_ov_module_set_regs(hisi_crtc, ovl_idx, enable_cmdlist, 0, 0, true);
	drm_check_and_return(ret != 0, ret, ERR, "hisi_dss_ov_module_set_regs failed");

	return 0;
}

int hisi_dss_module_init(struct hisi_drm_crtc *hisi_crtc)
{
	int ret;

	drm_check_and_return(hisi_crtc == NULL, -EINVAL, ERR, "hisi_crtc is NULL");

	ret = memcpy_s(&(hisi_crtc->dss_module), sizeof(struct dss_module_reg),
		&(hisi_crtc->dss_module_default), sizeof(struct dss_module_reg));
	if (ret != EOK) {
		HISI_DRM_ERR("dss module copy failed!");
		return -EINVAL;
	}
	return 0;
}

int hisi_overlay_on(struct hisi_drm_crtc *hisi_crtc)
{
	struct dss_hw_ctx *ctx = NULL;
	int ret;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL!\n");
		return -EINVAL;
	}

	ctx = hisi_crtc->ctx;
	if (!ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return -EINVAL;
	}

	hisi_crtc->vactive0_start_flag = 1;

	hisi_dss_on(hisi_crtc);
	hisi_dss_mctl_on(hisi_crtc);

	if (ctx->is_pre_enabled) {
		hisi_overlay_fastboot(hisi_crtc);
#ifdef CONFIG_HISI_DSS_V510
		hisi_crtc->pre_layer_mask = (1 << DSS_RCHN_V0);
#else
		hisi_crtc->pre_layer_mask = (1 << DSS_RCHN_D2);
#endif
	} else if (hisi_crtc->crtc_id == EXTERNAL_IDX) {
		ret = hisi_flush_ov_base_frame(hisi_crtc);
		drm_check_and_return(ret != 0, ret, ERR, "hisi_flush_ov_base_frame failed");
	}

	return 0;
}

int hisi_dss_mctl_mutex_lock(struct hisi_drm_crtc *hisi_crtc)
{
	void __iomem *mctl_base;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL!\n");
		return -1;
	}

	if (!hisi_crtc->ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return -1;
	}

	if (!hisi_crtc->ctx->dss_base) {
		HISI_DRM_ERR("dss_base is NULL!\n");
		return -1;
	}

	mctl_base = hisi_crtc->ctx->dss_base +
		g_dss_module_ovl_base[hisi_crtc->ovl_idx][MODULE_MCTL_BASE];

	set_reg(mctl_base + MCTL_CTL_MUTEX, 0x1, 1, 0);

	return 0;
}

int hisi_dss_mctl_mutex_unlock(struct hisi_drm_crtc *hisi_crtc)
{
	void __iomem *mctl_base;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is NULL!\n");
		return -1;
	}

	if (!hisi_crtc->ctx) {
		HISI_DRM_ERR("ctx is NULL!\n");
		return -1;
	}

	if (!hisi_crtc->ctx->dss_base) {
		HISI_DRM_ERR("dss_base is NULL!\n");
		return -1;
	}

	mctl_base = hisi_crtc->ctx->dss_base +
		g_dss_module_ovl_base[hisi_crtc->ovl_idx][MODULE_MCTL_BASE];

	set_reg(mctl_base + MCTL_CTL_MUTEX, 0x0, 1, 0);

	return 0;
}

/*lint +e548 +e574 +e676 +e679 +e732 +e838*/
