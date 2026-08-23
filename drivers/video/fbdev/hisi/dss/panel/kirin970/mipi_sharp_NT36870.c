/* Copyright (c) 2016-2022, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *	 * Redistributions of source code must retain the above copyright
 *	   notice, this list of conditions and the following disclaimer.
 *	 * Redistributions in binary form must reproduce the above
 *	   copyright notice, this list of conditions and the following
 *	   disclaimer in the documentation and/or other materials provided
 *	   with the distribution.
 *	 * Neither the name of Code Aurora Forum, Inc. nor the names of its
 *	   contributors may be used to endorse or promote products derived
 *	   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
 /*lint -e551 -e551*/
#include "hisi_fb.h"
#include "../mipi_lcd_utils.h"
#include "lcdkit_ext.h"
#define DTS_COMP_SHARP_NT36870 "hisilicon,mipi_sharp_NT36870"
static int g_lcd_fpga_flag;
#define CONFIG_TP_ERR_DEBUG
//#define AS_EXT_LCD_ON_ASIC

/*lint -save -e569, -e574, -e527, -e572*/
/*******************************************************************************
** Power ON/OFF Sequence(sleep mode to Normal mode) begin
*/
static char CMD_1[] = {
	0xFF,
	0xe0,
};
static char CMD_2[] = {
	0xfb,
	0x01,
};
static char CMD_3[] = {
	0x55,
	0x2f,
};
static char CMD_4[] = {
	0xFF,
	0x10,
};
static char CMD_5[] = {
	0x01,
	0x00,
};

static char CMD_6[] = {
	0xFF,
	0x10,
};
static char CMD_7[] = {
	0xfb,
	0x01,
};
static char CMD_8[] = {
	0xd7,
	0x01,
};
static char CMD_9[] = {
	0x2a,
	0x00, 0x00, 0x05, 0x9f,
};
static char CMD_10[] = {
	0x2b,
	0x00, 0x00, 0x09, 0xff,
};
static char CMD_11[] = {
	0xba,
	0x01,
};
static char CMD_12[] = {
	0xe5,
	0x01,
};
static char CMD_13[] = {
	0x35,
	0x00,
};
static char CMD_14[] = {
	0x44,
	0x09,
	0xfc,
};

static char CMD_16[] = {
	0xb0,
	0x01,
};
static char CMD_17[] = {
	0xFF,
	0xD0,
};
static char CMD_18[] = {
	0xFB,
	0x01,
};
static char CMD_19[] = {
	0x09,
	0xEE,
};
static char CMD_20[] = {
	0x28,
	0x50,
};
static char CMD_21[] = {
	0x53,
	0x88,
};
static char CMD_22[] = {
	0x54,
	0x08,
};
static char CMD_23[] = {
	0x1C,
	0x99,
};
static char CMD_24[] = {
	0x1D,
	0x09,
};
static char CMD_25[] = {
	0xFF,
	0x10,
};
static char CMD_26[] = {
	0xFB,
	0x01,
};
static char CMD_27[] = {
	0x51,
	0x0F,
	0xFF,
};
static char CMD_28[] = {
	0x53,
	0x2C,
};
static char CMD_29[] = {
	0x55,
	0x00,
};
static char CMD_30[] = {
	0x5E,
	0x00,
};
static char CMD_31[] = {
	0x61,
	0x00,
	0x00,
};
static char CMD_32[] = {
	0x63,
	0x24,
};
static char CMD_33[] = {
	0x65,
	0x01,
};

static char CMD_34[] = {
	0xFF,
	0xF0,

};
static char CMD_35[] = {
	0xFB,
	0x01,

};
static char CMD_36[] = {
	0xEF,
	0x80,
};
static char CMD_37[] = {
	0xFF,
	0x10,
};
static char CMD_38[] = {
	0xFB,
	0x01,
};

static char CMD_39[] = {
	0x11,
	0x00,
};
static char CMD_40[] = {
	0x29,
	0x00,
};
static char CMD_41[] = {
	0xFF,
	0x2A,
};
static char CMD_42[] = {
	0xFB,
	0x01,
};
static char CMD_43[] = {
	0x42,
	0x4D,
};
static char CMD_44[] = {
	0xFF,
	0x23,
};

static char CMD_45[] = {
	0xFB,
	0x01,
};
static char CMD_46[] = {
	0x00,
	0x80,
};
static char CMD_47[] = {
	0x07,
	0x00,
};
static char CMD_48[] = {
	0x08,
	0x01,
};
static char CMD_49[] = {
	0x09,
	0x00,
};
static char CMD_50[] = {
	0xFF,
	0x26,
};
static char CMD_51[] = {
	0xFB,
	0x01,
};
static char CMD_52[] = {
	0x00,
	0xA1,
};
static char CMD_53[] = {
	0x03,
	0x02,
};
static char CMD_54[] = {
	0x04,
	0x20,
};
static char CMD_55[] = {
	0x01,
	0x10,
};
static char CMD_56[] = {
	0x07,
	0xD1,
};
static char CMD_57[] = {
	0x09,
	0x15,
};
static char CMD_58[] = {
	0x0C,
	0x03,
};
static char CMD_59[] = {
	0x0D,
	0x00,
};
static char CMD_60[] = {
	0x0E,
	0x01,
};
static char CMD_61[] = {
	0x0F,
	0x02,
};

static char CMD_62[] = {
	0x10,
	0x03,
};
static char CMD_63[] = {
	0x11,
	0x02,
};
static char CMD_64[] = {
	0x12,
	0x80,
};

//0x2E
static char CMD_65[] = {
	0x19,
	0x35,
};

//0xA4
static char CMD_66[] = {
	0x1A,
	0xF0,
};
static char CMD_67[] = {
	0x1B,
	0x00,
};
static char CMD_68[] = {
	0x1C,
	0xFA,
};

static char CMD_69[] = {
	0x1D,
	0x2E,
};
static char CMD_70[] = {
	0x1E,
	0x2B,
};
static char CMD_71[] = {
	0x23,
	0x01,
};
static char CMD_72[] = {
	0x24,
	0x00,
};
static char CMD_73[] = {
	0x25,
	0x01,
};
static char CMD_74[] = {
	0x26,
	0x05,
};
static char CMD_75[] = {
	0x27,
	0x06,
};
static char CMD_76[] = {
	0x28,
	0x05,
};
static char CMD_77[] = {
	0x29,
	0x00,
};
static char CMD_78[] = {
	0x2F,
	0x00,
};

//0x5D
static char CMD_79[] = {
	0x33,
	0x6B,
};

//0x48
static char CMD_80[] = {
	0x34,
	0xE0,
};
static char CMD_81[] = {
	0x35,
	0x00,
};
static char CMD_82[] = {
	0x37,
	0x5C,
};
static char CMD_83[] = {
	0x38,
	0xCF,
};
static char CMD_84[] = {
	0x6F,
	0x00,
};
static char CMD_85[] = {
	0x70,
	0x00,
};
static char CMD_86[] = {
	0x71,
	0x40,
};
static char CMD_87[] = {
	0x72,
	0x00,
};
static char CMD_88[] = {
	0x78,
	0x00,
};

static char CMD_89[] = {
	0x80,
	0x05,
};
static char CMD_90[] = {
	0x81,
	0x1B,
};
static char CMD_91[] = {
	0x82,
	0x00,
};

static char CMD_92[] = {
	0x83,
	0x09,
};
static char CMD_93[] = {
	0x84,
	0x05,
};
static char CMD_94[] = {
	0x85,
	0x03,

};
static char CMD_95[] = {
	0x86,
	0x05,
};

static char CMD_96[] = {
	0x87,
	0x03,
};
static char CMD_97[] = {
	0x88,
	0x10,
};
static char CMD_98[] = {
	0x89,
	0x00,
};
static char CMD_99[] = {
	0x8A,
	0x12,
};
static char CMD_100[] = {
	0x8B,
	0x00,
};

static char CMD_101[] = {
	0x8C,
	0x33,
};
static char CMD_102[] = {
	0x8D,
	0x42,
};
static char CMD_103[] = {
	0x8E,
	0x24,
};
static char CMD_104[] = {
	0x8F,
	0x55,
};
static char CMD_105[] = {
	0x90,
	0x66,
};
static char CMD_106[] = {
	0x91,
	0x77,
};
static char CMD_107[] = {
	0x93,
	0x00,
};
static char CMD_108[] = {
	0x99,
	0x24,
};
static char CMD_109[] = {
	0x9A,
	0x81,
};
static char CMD_110[] = {
	0x9B,
	0x01,
};

static char CMD_111[] = {
	0x9C,
	0x01,
};
static char CMD_112[] = {
	0x9D,
	0x01,
};
static char CMD_113[] = {
	0x9E,
	0x01,
};

static char CMD_114[] = {
	0xC3,
	0x00,
};
static char CMD_115[] = {
	0xC8,
	0x0A,
};
static char CMD_116[] = {
	0xC9,
	0x00,
};

//0xBA
static char CMD_117[] = {
	0xD0,
	0xD7,
};

//0x90
static char CMD_118[] = {
	0xD1,
	0xC1,
};
static char CMD_119[] = {
	0xC4,
	0x00,
};
static char CMD_120[] = {
	0xC5,
	0x01,
};
static char CMD_121[] = {
	0xCC,
	0x00,
};
static char CMD_122[] = {
	0xD4,
	0xBA,
};
static char CMD_123[] = {
	0xD5,
	0x17,
};
static char CMD_124[] = {
	0xFF,
	0x26,
};
static char CMD_125[] = {
	0xFB,
	0x01,
};
static char CMD_126[] = {
	0xDB,
	0x03,
};
static char CMD_127[] = {
	0xDC,
	0x00,
};
static char CMD_128[] = {
	0xDD,
	0x01,
};
static char CMD_129[] = {
	0xDE,
	0x02,
};
static char CMD_130[] = {
	0xDF,
	0x03,
};
static char CMD_131[] = {
	0xE0,
	0x02,
};
static char CMD_132[] = {
	0xE1,
	0x80,
};
static char CMD_133[] = {
	0xE8,
	0x6B,
};
static char CMD_134[] = {
	0xE9,
	0xE0,
};

//0x6B
static char CMD_135[] = {
	0xEC,
	0x2E,
};
//0x67
static char CMD_136[] = {
	0xED,
	0x2B,
};
static char CMD_137[] = {
	0xFF,
	0x10,
};
static char CMD_138[] = {
	0xFB,
	0x01,
};

static char CMD_PG25_1[] = {
	0xff,
	0x25,
};
static char CMD_PG25_2[] = {
	0xfb,
	0x01,
};
static char CMD_PG25_3[] = {
	0x05,
	0x00,
};

//0x80,
static char CMD_PG25_4[] = {
	0x0a,
	0x80,
};

//0x8c,
static char CMD_PG25_5[] = {
	0x0b,
	0x8C,
};
static char CMD_PG25_6[] = {
	0x0c,
	0x01,
};

static char CMD_PG24_1[] = {
	0xff,
	0x24,
};
static char CMD_PG24_2[] = {
	0xfb,
	0x01,
};

//0xC6-en vid drop
static char CMD_PG24_3[] = {
	0xC2,
	0xC6,
};

//0x63
static char CMD_PG24_4[] = {
	0xC1,
	0x13,
};

/////////////
/*
static char CMD_ADD_1[] = {
	0xff,
	0x26,
};
static char CMD_ADD_2[] = {
	0xfb,
	0x01,
};
static char CMD_ADD_3[] = {
	0x03,
	0x02,
};
static char CMD_ADD_EC[] = {
	0xec,
	0x35,
};
static char CMD_ADD_ED[] = {
	0xed,
	0x77,
};
*/
/////////////

static char compression_select[] = {
	0xC0,
	0x03,
};

static char vesa3x_10bit_video_slice8L_C3[] = {
	0xC3,
	0xAB,0x2A,0x00,0x08,0x02,0x00,0x02,0x20,0x01,0x04,0x00,0x0E,0x0D,0xB7,0x0B,0xAB,
};
static char vesa3x_10bit_video_slice8L_C4[] = {
	0xC4,
	0x0C,0xF0,
};

static char vesa3x_8bit_slice8L_C1[] = {
	0xC1,
	0x89,0x28,0x00,0x08,0x02,0x00,0x02,0x68,0x00,0xD5,0x00,0x0A,0x0D,0xB7,0x09,0x89,
};

static char vesa3x_8bit_slice8L_C2[] = {
	0xC2,
	0x10,0xF0,
};

static char vesa3_75x_slice8L_C1[] = {
	0xC1,
	0xAB,0x28,0x00,0x08,0x02,0x00,0x02,0x68,0x00,0xD5,0x00,0x0A,0x0D,0xB7,0x09,0x89,
};

static char vesa3_75x_slice8L_C2[] = {
	0xC2,
	0x10,0xF0,
};

//0x64,
//0xC8,
static char te_output[] = {
	0xB3,
	0x21,
};
static char frm_delay[] = {
	0xB4,
	0x21,
};

static char video_mode[] = {
	0xBB,
	0x13,
};

/*Param[1:2] = (VBP+VSA)[0:7]  VFP[0:7]*/
//tmp == 2:0x03,0x0A,0x0A,
//tmp == 3:0x03,0x32,0x1e,
//tmp == 4:0x03,0x58,0x1C,
static char video_control[] = {
	0x3B,
	0x03,0x58,0x1C,
};

static char cmd_mode[] = {
	0xBB,
	0x10,
};

static char get_compression_mode_8bit[] = {
	0x03,
	0x00,
};

static char get_compression_mode_10bit[] = {
	0x03,
	0x10,
};

/*
static struct dsi_cmd_desc init_buf_delay_cmds[] = {
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_ADD_1), CMD_ADD_1},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_ADD_2), CMD_ADD_2},
	{DTYPE_DCS_WRITE1, 0,5, WAIT_TYPE_MS,
		sizeof(CMD_ADD_3), CMD_ADD_3},
#if 0 //dashouzhang
	{DTYPE_DCS_WRITE1, 0,5, WAIT_TYPE_MS,
		sizeof(CMD_ADD_EC), CMD_ADD_EC},
	{DTYPE_DCS_WRITE1, 0,5, WAIT_TYPE_MS,
		sizeof(CMD_ADD_ED), CMD_ADD_ED},
#endif
};
*/

static struct dsi_cmd_desc disable_auto_vbpvfp_en_nodrop_cmds[] = {
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_PG25_1), CMD_PG25_1},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_PG25_2), CMD_PG25_2},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_PG25_3), CMD_PG25_3},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_PG25_4), CMD_PG25_4},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_PG25_5), CMD_PG25_5},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_PG25_6), CMD_PG25_6},

	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_PG24_1), CMD_PG24_1},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_PG24_2), CMD_PG24_2},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_PG24_3), CMD_PG24_3},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_PG24_4), CMD_PG24_4},
};

static struct dsi_cmd_desc lcd_display_init_10bit_video_3x_dsc_cmds[] = {
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US,
		sizeof(video_mode), video_mode},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(get_compression_mode_10bit), get_compression_mode_10bit},
};
static struct dsi_cmd_desc lcd_display_init_8bit_cmd_3x_dsc_cmds[] = {
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US,
		sizeof(cmd_mode), cmd_mode},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(get_compression_mode_8bit), get_compression_mode_8bit},
};

static struct dsi_cmd_desc lcd_display_init_cmds_part1[] = {
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US,
		sizeof(CMD_1), CMD_1},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US,
		sizeof(CMD_2), CMD_2},
	{DTYPE_DCS_LWRITE, 0,1, WAIT_TYPE_MS,
		sizeof(CMD_3), CMD_3},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US,
		sizeof(CMD_4), CMD_4},
	{DTYPE_DCS_WRITE, 0,20, WAIT_TYPE_MS,
		sizeof(CMD_5), CMD_5},
};

static struct dsi_cmd_desc lcd_display_init_cmds_part2[] = {
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_6), CMD_6},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_7), CMD_7},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_8), CMD_8},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US,
		sizeof(CMD_9), CMD_9},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US,
		sizeof(CMD_10), CMD_10},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_11), CMD_11},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_12), CMD_12},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_13), CMD_13},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US,
		sizeof(CMD_14), CMD_14},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_16), CMD_16},

	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(compression_select), compression_select},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US,
		sizeof(video_control), video_control},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US,
		sizeof(te_output), te_output},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US,
		sizeof(frm_delay), frm_delay},
};

static struct dsi_cmd_desc lcd_display_init_vesa3x_para[] = {
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US,
		sizeof(vesa3x_8bit_slice8L_C1), vesa3x_8bit_slice8L_C1},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US,
		sizeof(vesa3x_8bit_slice8L_C2), vesa3x_8bit_slice8L_C2},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US,
		sizeof(vesa3x_10bit_video_slice8L_C3), vesa3x_10bit_video_slice8L_C3},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US,
		sizeof(vesa3x_10bit_video_slice8L_C4), vesa3x_10bit_video_slice8L_C4},
};
static struct dsi_cmd_desc lcd_display_init_vesa3_75x_para[] = {
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US,
		sizeof(vesa3_75x_slice8L_C1), vesa3_75x_slice8L_C1},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US,
		sizeof(vesa3_75x_slice8L_C2), vesa3_75x_slice8L_C2},
};

static struct dsi_cmd_desc lcd_display_init_cmds_part3[] = {
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_17), CMD_17},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_18), CMD_18},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_19), CMD_19},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_20), CMD_20},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_21), CMD_21},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_22), CMD_22},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_23), CMD_23},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_24), CMD_24},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_25), CMD_25},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_26), CMD_26},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US,
		sizeof(CMD_27), CMD_27},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_28), CMD_28},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_29), CMD_29},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_30), CMD_30},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US,
		sizeof(CMD_31), CMD_31},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_32), CMD_32},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_33), CMD_33},
	{DTYPE_DCS_WRITE, 0,10, WAIT_TYPE_US,
		sizeof(CMD_34), CMD_34},
	{DTYPE_DCS_WRITE, 0,10, WAIT_TYPE_US,
		sizeof(CMD_35), CMD_35},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_36), CMD_36},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_37), CMD_37},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_38), CMD_38},
	{DTYPE_DCS_WRITE, 0,10, WAIT_TYPE_MS,
		sizeof(CMD_39), CMD_39},
	{DTYPE_DCS_WRITE, 0,10, WAIT_TYPE_US,
		sizeof(CMD_40), CMD_40},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_41), CMD_41},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_42), CMD_42},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_43), CMD_43},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_44), CMD_44},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_45), CMD_45},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_46), CMD_46},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_47), CMD_47},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_48), CMD_48},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_49), CMD_49},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_50), CMD_50},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_51), CMD_51},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_52), CMD_52},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_53), CMD_53},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_54), CMD_54},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_55), CMD_55},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_56), CMD_56},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_57), CMD_57},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_58), CMD_58},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_59), CMD_59},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_60), CMD_60},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_61), CMD_61},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_62), CMD_62},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_63), CMD_63},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_64), CMD_64},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_65), CMD_65},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_66), CMD_66},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_67), CMD_67},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_68), CMD_68},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_69), CMD_69},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_70), CMD_70},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_71), CMD_71},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_72), CMD_72},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_73), CMD_73},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_74), CMD_74},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_75), CMD_75},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_76), CMD_76},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_77), CMD_77},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_78), CMD_78},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_79), CMD_79},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_80), CMD_80},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_81), CMD_81},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_82), CMD_82},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_83), CMD_83},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_84), CMD_84},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_85), CMD_85},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_86), CMD_86},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_87), CMD_87},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_88), CMD_88},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_89), CMD_89},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_90), CMD_90},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_91), CMD_91},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_92), CMD_92},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_93), CMD_93},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_94), CMD_94},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_95), CMD_95},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_96), CMD_96},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_97), CMD_97},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_98), CMD_98},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_99), CMD_99},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_100), CMD_100},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_101), CMD_101},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_102), CMD_102},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_103), CMD_103},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_104), CMD_104},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_105), CMD_105},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_106), CMD_106},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_107), CMD_107},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_108), CMD_108},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_109), CMD_109},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_110), CMD_110},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_111), CMD_111},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_112), CMD_112},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_113), CMD_113},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_114), CMD_114},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_115), CMD_115},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_116), CMD_116},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_117), CMD_117},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_118), CMD_118},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_119), CMD_119},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_120), CMD_120},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_121), CMD_121},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_122), CMD_122},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_123), CMD_123},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_124), CMD_124},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_125), CMD_125},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_126), CMD_126},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_127), CMD_127},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_128), CMD_128},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_129), CMD_129},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_130), CMD_130},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_131), CMD_131},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_132), CMD_132},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_133), CMD_133},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_134), CMD_134},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_135), CMD_135},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_136), CMD_136},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_137), CMD_137},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(CMD_138), CMD_138},

};

static char display_off[] = {
	0x28,
};
static char sleep_in[] = {
	0x10,
};

static struct dsi_cmd_desc lcd_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 0, 60, WAIT_TYPE_MS,
		sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 0, 120, WAIT_TYPE_MS,
		sizeof(sleep_in), sleep_in},
};

/*******************************************************************************
** LCD VCC
*/
#define VCC_LCDIO_NAME		"lcdio-vcc"
#define VCC_LCDANALOG_NAME	"lcdanalog-vcc"

static struct regulator *vcc_lcdio;
static struct regulator *vcc_lcdanalog;

static struct vcc_desc lcd_vcc_init_cmds[] = {
	/* vcc get */
	{DTYPE_VCC_GET, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 0},
	{DTYPE_VCC_GET, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 0},

	/* vcc set voltage */
	{DTYPE_VCC_SET_VOLTAGE, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 3100000, 3100000, WAIT_TYPE_MS, 0},
	/* io set voltage */
	{DTYPE_VCC_SET_VOLTAGE, VCC_LCDIO_NAME, &vcc_lcdio, 1800000, 1800000, WAIT_TYPE_MS, 0},
};

static struct vcc_desc lcd_vcc_finit_cmds[] = {
	/* vcc put */
	{DTYPE_VCC_PUT, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 0},
	{DTYPE_VCC_PUT, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 0},
};

static struct vcc_desc lcd_vcc_enable_cmds[] = {
	/* vcc enable */
	{DTYPE_VCC_ENABLE, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 3},
	{DTYPE_VCC_ENABLE, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 3},
};

static struct vcc_desc lcd_vcc_disable_cmds[] = {
	/* vcc disable */
	{DTYPE_VCC_DISABLE, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 3},
	{DTYPE_VCC_DISABLE, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 3},
};


/*******************************************************************************
** LCD IOMUX
*/
static struct pinctrl_data pctrl;

static struct pinctrl_cmd_desc lcd_pinctrl_init_cmds[] = {
	{DTYPE_PINCTRL_GET, &pctrl, 0},
	{DTYPE_PINCTRL_STATE_GET, &pctrl, DTYPE_PINCTRL_STATE_DEFAULT},
	{DTYPE_PINCTRL_STATE_GET, &pctrl, DTYPE_PINCTRL_STATE_IDLE},
};

static struct pinctrl_cmd_desc lcd_pinctrl_normal_cmds[] = {
	{DTYPE_PINCTRL_SET, &pctrl, DTYPE_PINCTRL_STATE_DEFAULT},
};

static struct pinctrl_cmd_desc lcd_pinctrl_lowpower_cmds[] = {
	{DTYPE_PINCTRL_SET, &pctrl, DTYPE_PINCTRL_STATE_IDLE},
};

static struct pinctrl_cmd_desc lcd_pinctrl_finit_cmds[] = {
	{DTYPE_PINCTRL_PUT, &pctrl, 0},
};


/*
** Power ON/OFF Sequence end
*******************************************************************************/



/*******************************************************************************
** LCD GPIO
*/
#define GPIO_LCD_P5V8_ENABLE_NAME	"gpio_lcd_p5v8_enable"
#define GPIO_LCD_N5V8_ENABLE_NAME "gpio_lcd_n5v8_enable"
#define GPIO_LCD_RESET_NAME	"gpio_lcd_reset"
#define GPIO_LCD_BL_ENABLE_NAME	"gpio_lcd_bl_enable"
#define GPIO_LCD_TP1V8_NAME	"gpio_lcd_1v8"

static uint32_t gpio_lcd_p5v8_enable;
static uint32_t gpio_lcd_n5v8_enable;
static uint32_t gpio_lcd_reset;
static uint32_t gpio_lcd_bl_enable;
static uint32_t gpio_lcd_1v8;


#define GPIO_LCD_P5V5_ENABLE_NAME	"gpio_lcd_p5v5_enable"
#define GPIO_LCD_N5V5_ENABLE_NAME "gpio_lcd_n5v5_enable"
#define GPIO_LCD_ID0_NAME	"gpio_lcd_id0"
static uint32_t gpio_lcd_p5v5_enable;
static uint32_t gpio_lcd_n5v5_enable;
static uint32_t gpio_lcd_id0;

static struct gpio_desc fpga_lcd_gpio_request_cmds[] = {
	/* AVDD_5.8V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_P5V8_ENABLE_NAME, &gpio_lcd_p5v8_enable, 0},
	/* AVEE_-5.8V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_N5V8_ENABLE_NAME, &gpio_lcd_n5v8_enable, 0},
	/* reset */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	/* backlight enable */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_TP1V8_NAME, &gpio_lcd_1v8, 0},
};

static struct gpio_desc fpga_lcd_gpio_on_cmds[] = {
	/* lcd_1.8V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 10,
		GPIO_LCD_TP1V8_NAME, &gpio_lcd_1v8, 1},
	/* AVDD_5.8V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 10,
		GPIO_LCD_P5V8_ENABLE_NAME, &gpio_lcd_p5v8_enable, 1},
	/* AVEE_-5.8V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_LCD_N5V8_ENABLE_NAME, &gpio_lcd_n5v8_enable, 1},
	/* reset */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_US, 50,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 50,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 1},
	/* backlight enable */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 1},
};

static struct gpio_desc lcd_gpio_off_cmds[] = {
	/* backlight disable */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	/* AVEE_-5.8V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 3,
		GPIO_LCD_N5V8_ENABLE_NAME, &gpio_lcd_n5v8_enable, 0},
	/* AVDD_5.8V*/
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 3,
		GPIO_LCD_P5V8_ENABLE_NAME, &gpio_lcd_p5v8_enable, 0},
	/* reset */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 3,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	/* lcd_1.8V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_TP1V8_NAME, &gpio_lcd_1v8, 0},

	/* backlight enable input */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	/* AVEE_-5.8V input */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_N5V8_ENABLE_NAME, &gpio_lcd_n5v8_enable, 0},
	/* AVDD_5.8V input */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_P5V8_ENABLE_NAME, &gpio_lcd_p5v8_enable, 0},
	/* reset input */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 0,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	/* lcd_1.8V */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_TP1V8_NAME, &gpio_lcd_1v8, 0},

};

static struct gpio_desc lcd_gpio_free_cmds[] = {
	/* backlight enable */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	/* AVEE_-5.8V */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_N5V8_ENABLE_NAME, &gpio_lcd_n5v8_enable, 0},
	/* AVDD_5.8V */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_P5V8_ENABLE_NAME, &gpio_lcd_p5v8_enable, 0},
	/* lcd_1.8V */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_TP1V8_NAME, &gpio_lcd_1v8, 0},
	/* reset */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
};


static struct gpio_desc asic_lcd_gpio_request_cmds[] = {
	/* AVDD_5.5V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 0},
	/* AVEE_-5.5V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 0},
	/* backlight enable */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	/* reset */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	/*lcd_id0*/
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_ID0_NAME, &gpio_lcd_id0, 0},
};

static struct gpio_desc asic_lcd_gpio_normal_cmds_sub1[] = {
	/* AVDD_5.5V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 1},
	/* AVEE_-5.5V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 1},
	/* reset */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_US, 20,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_US, 20,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 1},
};

static struct gpio_desc asic_lcd_gpio_normal_cmds_sub2[] = {
	/* backlight enable */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 1},
	/* id0 */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_ID0_NAME, &gpio_lcd_id0, 0},
};

static struct gpio_desc asic_lcd_gpio_lowpower_cmds[] = {
	/* backlight enable */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	/* AVEE_-5.5V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 0},
	/* AVDD_5.5V*/
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 0},
	/* reset */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},

	/* backlight enable input */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 100,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	/* AVEE_-5.5V input */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 0},
	/* AVDD_5.5V input */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 0},
	/* reset input */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 100,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
};

static struct gpio_desc asic_lcd_gpio_free_cmds[] = {
	/* AVDD_5.5V */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 0},
	/* AVEE_-5.5V */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 0},
	/* backlight enable */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	/* reset */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	/*lcd_id0*/
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_ID0_NAME, &gpio_lcd_id0, 0},
};


/*******************************************************************************
**
*/
static int mipi_sharp_NT36870_panel_set_fastboot(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	if (g_lcd_fpga_flag == 0) {
		// lcd pinctrl normal
		pinctrl_cmds_tx(pdev, lcd_pinctrl_normal_cmds,
			ARRAY_SIZE(lcd_pinctrl_normal_cmds));
		// lcd gpio request
		gpio_cmds_tx(asic_lcd_gpio_request_cmds,
			ARRAY_SIZE(asic_lcd_gpio_request_cmds));
	} else {
		// lcd gpio request
		gpio_cmds_tx(fpga_lcd_gpio_request_cmds,
			ARRAY_SIZE(fpga_lcd_gpio_request_cmds));
	}

	// backlight on
	hisi_lcd_backlight_on(pdev);

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return 0;
}

void mipi_sharp_NT36870_mode_switch(struct hisi_fb_data_type *hisifd, uint8_t mode_switch_to)
{
	struct hisi_panel_info *pinfo = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("NULL hisifd .\n");
		return;
	}

	pinfo = &(hisifd->panel_info);
	if (NULL == pinfo) {
		HISI_FB_ERR("NULL pinfo .\n");
		return;
	}

	if (!pinfo->panel_mode_swtich_support) {
		HISI_FB_INFO("not srpport .\n");
		return;
	}

	HISI_FB_DEBUG("+ .\n");
	if (mode_switch_to == MODE_10BIT_VIDEO_3X) {
		mipi_dsi_cmds_tx(lcd_display_init_10bit_video_3x_dsc_cmds, \
			ARRAY_SIZE(lcd_display_init_10bit_video_3x_dsc_cmds), hisifd->mipi_dsi0_base);
	} else if (mode_switch_to == MODE_8BIT) {
		mipi_dsi_cmds_tx(lcd_display_init_8bit_cmd_3x_dsc_cmds, \
			ARRAY_SIZE(lcd_display_init_8bit_cmd_3x_dsc_cmds), hisifd->mipi_dsi0_base);
	}

	HISI_FB_DEBUG("- .\n");
	return;
}
static int mipi_sharp_NT36870_panel_on(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *mipi_dsi0_base = NULL;
	uint32_t status = 0;
	uint32_t try_times = 0;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);
	pinfo = &(hisifd->panel_info);
	BUG_ON(pinfo == NULL);

	HISI_FB_INFO("fb%d, +.\n", hisifd->index);

	pinfo = &(hisifd->panel_info);
	mipi_dsi0_base = hisifd->mipi_dsi0_base;

	if (pinfo->lcd_init_step == LCD_INIT_POWER_ON) {
		// check lcd power state
		if (g_lcd_fpga_flag == 0) {
			// lcd vcc enable
			vcc_cmds_tx(pdev, lcd_vcc_enable_cmds,
				ARRAY_SIZE(lcd_vcc_enable_cmds));
			// lcd pinctrl normal
			pinctrl_cmds_tx(pdev, lcd_pinctrl_normal_cmds,
				ARRAY_SIZE(lcd_pinctrl_normal_cmds));

			// lcd gpio request
			gpio_cmds_tx(asic_lcd_gpio_request_cmds, \
				ARRAY_SIZE(asic_lcd_gpio_request_cmds));

			// lcd gpio normal
			gpio_cmds_tx(asic_lcd_gpio_normal_cmds_sub1, \
				ARRAY_SIZE(asic_lcd_gpio_normal_cmds_sub1));
		} else {
			gpio_cmds_tx(fpga_lcd_gpio_request_cmds, \
				ARRAY_SIZE(fpga_lcd_gpio_request_cmds));
			gpio_cmds_tx(fpga_lcd_gpio_on_cmds, \
				ARRAY_SIZE(fpga_lcd_gpio_on_cmds));
		}

		pinfo->lcd_init_step = LCD_INIT_MIPI_LP_SEND_SEQUENCE;

	} else if (pinfo->lcd_init_step == LCD_INIT_MIPI_LP_SEND_SEQUENCE) {
		if (g_lcd_fpga_flag == 0) {
			// lcd gpio normal
			gpio_cmds_tx(asic_lcd_gpio_normal_cmds_sub2, \
				ARRAY_SIZE(asic_lcd_gpio_normal_cmds_sub2));
		}

		mipi_dsi_cmds_tx(lcd_display_init_cmds_part1, \
			ARRAY_SIZE(lcd_display_init_cmds_part1), mipi_dsi0_base);
		mipi_dsi_cmds_tx(disable_auto_vbpvfp_en_nodrop_cmds, \
			ARRAY_SIZE(disable_auto_vbpvfp_en_nodrop_cmds), mipi_dsi0_base);

		mipi_dsi_cmds_tx(lcd_display_init_cmds_part2, \
			ARRAY_SIZE(lcd_display_init_cmds_part2), mipi_dsi0_base);

		if (pinfo->ifbc_type == IFBC_TYPE_VESA3X_DUAL) {
			mipi_dsi_cmds_tx(lcd_display_init_vesa3x_para, \
					ARRAY_SIZE(lcd_display_init_vesa3x_para), mipi_dsi0_base);
			if (pinfo->current_mode == MODE_10BIT_VIDEO_3X) {
				mipi_dsi_cmds_tx(lcd_display_init_10bit_video_3x_dsc_cmds, \
					ARRAY_SIZE(lcd_display_init_10bit_video_3x_dsc_cmds), mipi_dsi0_base);
			} else if (pinfo->current_mode == MODE_8BIT) {
				mipi_dsi_cmds_tx(lcd_display_init_8bit_cmd_3x_dsc_cmds, \
					ARRAY_SIZE(lcd_display_init_8bit_cmd_3x_dsc_cmds), mipi_dsi0_base);
			}
		} else if (pinfo->ifbc_type == IFBC_TYPE_VESA3_75X_DUAL) {
			mipi_dsi_cmds_tx(lcd_display_init_vesa3_75x_para, \
					ARRAY_SIZE(lcd_display_init_vesa3_75x_para), mipi_dsi0_base);
		}

		mipi_dsi_cmds_tx(lcd_display_init_cmds_part3, \
			ARRAY_SIZE(lcd_display_init_cmds_part3), mipi_dsi0_base);
	#ifdef CONFIG_TP_ERR_DEBUG
		lcdkit_notifier_call_chain(LCDKIT_TS_RESUME_DEVICE, NULL);
	#endif

		// check lcd power state
		outp32(mipi_dsi0_base + MIPIDSI_GEN_HDR_OFFSET, 0x0A06);
		status = inp32(mipi_dsi0_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		while (status & 0x10) {
			udelay(50);
			if (++try_times > 100) {
				try_times = 0;
				HISI_FB_ERR("fb%d, Read lcd power status timeout!\n", hisifd->index);
				break;
			}
			status = inp32(mipi_dsi0_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		}
		status = inp32(mipi_dsi0_base + MIPIDSI_GEN_PLD_DATA_OFFSET);
		HISI_FB_INFO("fb%d, SHARP_NT36870_LCD Power State = 0x%x.\n", hisifd->index, status);

		pinfo->lcd_init_step = LCD_INIT_MIPI_HS_SEND_SEQUENCE;

	} else if (pinfo->lcd_init_step == LCD_INIT_MIPI_HS_SEND_SEQUENCE) {
	#ifdef CONFIG_TP_ERR_DEBUG
		lcdkit_notifier_call_chain(LCDKIT_TS_AFTER_RESUME, NULL);
	#endif
	} else {
		HISI_FB_ERR("failed to init lcd!\n");
	}

	// backlight on
	hisi_lcd_backlight_on(pdev);
	HISI_FB_DEBUG("fb%d, -!\n", hisifd->index);

	return 0;
}


static int mipi_sharp_NT36870_panel_off(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *mipi_dsi0_base = NULL;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);
	pinfo = &(hisifd->panel_info);

	mipi_dsi0_base = hisifd->mipi_dsi0_base;

	HISI_FB_INFO("fb%d, +.\n", hisifd->index);

	if (pinfo->lcd_uninit_step == LCD_UNINIT_MIPI_HS_SEND_SEQUENCE) {
	#ifdef CONFIG_TP_ERR_DEBUG
		lcdkit_notifier_call_chain(LCDKIT_TS_EARLY_SUSPEND, NULL);
	#endif
		hisi_lcd_backlight_off(pdev);

		mipi_dsi_cmds_tx(lcd_display_off_cmds, \
			ARRAY_SIZE(lcd_display_off_cmds), mipi_dsi0_base);
		pinfo->lcd_uninit_step = LCD_UNINIT_MIPI_LP_SEND_SEQUENCE;
	#ifdef CONFIG_TP_ERR_DEBUG
		lcdkit_notifier_call_chain(LCDKIT_TS_BEFORE_SUSPEND, NULL);
		lcdkit_notifier_call_chain(LCDKIT_TS_SUSPEND_DEVICE, NULL);
	#endif
	} else if (pinfo->lcd_uninit_step == LCD_UNINIT_MIPI_LP_SEND_SEQUENCE) {
		pinfo->lcd_uninit_step = LCD_UNINIT_POWER_OFF;

	} else if (pinfo->lcd_uninit_step == LCD_UNINIT_POWER_OFF) {
		if (g_lcd_fpga_flag==0) {
			// lcd gpio lowpower
			gpio_cmds_tx(asic_lcd_gpio_lowpower_cmds, \
				ARRAY_SIZE(asic_lcd_gpio_lowpower_cmds));
			// lcd gpio free
			gpio_cmds_tx(asic_lcd_gpio_free_cmds, \
				ARRAY_SIZE(asic_lcd_gpio_free_cmds));
			// lcd pinctrl lowpower
			pinctrl_cmds_tx(pdev,lcd_pinctrl_lowpower_cmds,
				ARRAY_SIZE(lcd_pinctrl_lowpower_cmds));

			mdelay(3);
			// lcd vcc disable
			vcc_cmds_tx(pdev, lcd_vcc_disable_cmds,
				ARRAY_SIZE(lcd_vcc_disable_cmds));
		} else {
			// lcd gpio lowpower
			gpio_cmds_tx(lcd_gpio_off_cmds, \
				ARRAY_SIZE(lcd_gpio_off_cmds));
			// lcd gpio free
			gpio_cmds_tx(lcd_gpio_free_cmds, \
				ARRAY_SIZE(lcd_gpio_free_cmds));
		}
	} else {
		HISI_FB_ERR("failed to uninit lcd!\n");
	}

	HISI_FB_INFO("fb%d, -.\n", hisifd->index);

	return 0;
}

static int mipi_sharp_NT36870_panel_remove(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	if (!hisifd) {
		return 0;
	}
	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	if (g_lcd_fpga_flag == 0) {
		// lcd vcc finit
		vcc_cmds_tx(pdev, lcd_vcc_finit_cmds,
			ARRAY_SIZE(lcd_vcc_finit_cmds));

		// lcd pinctrl finit
		pinctrl_cmds_tx(pdev, lcd_pinctrl_finit_cmds,
			ARRAY_SIZE(lcd_pinctrl_finit_cmds));
	}

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);
	return 0;
}

static int mipi_sharp_NT36870_panel_set_backlight(struct platform_device *pdev, uint32_t bl_level)
{
	struct hisi_fb_data_type *hisifd = NULL;
	int ret = 0;
	static char last_bl_level=0;

	char bl_level_adjust[2] = {
		0x51,
		0x00,
	};

	struct dsi_cmd_desc lcd_bl_level_adjust[] = {
		{DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
			sizeof(bl_level_adjust), bl_level_adjust},
	};

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	if (hisifd->panel_info.bl_set_type & BL_SET_BY_PWM) {

		ret = hisi_pwm_set_backlight(hisifd, bl_level);

	} else if (hisifd->panel_info.bl_set_type & BL_SET_BY_BLPWM) {

		ret = hisi_blpwm_set_backlight(hisifd, bl_level);

	} else if (hisifd->panel_info.bl_set_type & BL_SET_BY_SH_BLPWM) {

		ret = hisi_sh_blpwm_set_backlight(hisifd, bl_level);

	} else if (hisifd->panel_info.bl_set_type & BL_SET_BY_MIPI) {

		bl_level_adjust[1] = bl_level * 255 / hisifd->panel_info.bl_max;

		if (last_bl_level != bl_level_adjust[1]){
			last_bl_level = bl_level_adjust[1];
			mipi_dsi_cmds_tx(lcd_bl_level_adjust, \
				ARRAY_SIZE(lcd_bl_level_adjust), hisifd->mipi_dsi0_base);
			}
		HISI_FB_DEBUG("lw bl_level_adjust[1] = %d.\n",bl_level_adjust[1]);

	} else {
		HISI_FB_ERR("fb%d, not support this bl_set_type(%d)!\n",
			hisifd->index, hisifd->panel_info.bl_set_type);
	}

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;

}

static char lcd_disp_x[] = {
	0x2A,
	0x00, 0x00,0x05,0x9F
};

static char lcd_disp_y[] = {
	0x2B,
	0x00, 0x00,0x09,0xFF
};

static struct dsi_cmd_desc set_display_address[] = {
	{DTYPE_DCS_LWRITE, 0, 5, WAIT_TYPE_US,
		sizeof(lcd_disp_x), lcd_disp_x},
	{DTYPE_DCS_LWRITE, 0, 5, WAIT_TYPE_US,
		sizeof(lcd_disp_y), lcd_disp_y},
};

static int mipi_sharp_NT36870_panel_set_display_region(struct platform_device *pdev,
	struct dss_rect *dirty)
{
	struct hisi_fb_data_type *hisifd = NULL;

	BUG_ON(pdev == NULL || dirty == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	lcd_disp_x[1] = (dirty->x >> 8) & 0xff;
	lcd_disp_x[2] = dirty->x & 0xff;
	lcd_disp_x[3] = ((dirty->x + dirty->w - 1) >> 8) & 0xff;
	lcd_disp_x[4] = (dirty->x + dirty->w - 1) & 0xff;
	lcd_disp_y[1] = (dirty->y >> 8) & 0xff;
	lcd_disp_y[2] = dirty->y & 0xff;
	lcd_disp_y[3] = ((dirty->y + dirty->h - 1) >> 8) & 0xff;
	lcd_disp_y[4] = (dirty->y + dirty->h - 1) & 0xff;

	HISI_FB_DEBUG("x[1] = 0x%2x, x[2] = 0x%2x, x[3] = 0x%2x, x[4] = 0x%2x.\n",
		lcd_disp_x[1], lcd_disp_x[2], lcd_disp_x[3], lcd_disp_x[4]);
	HISI_FB_DEBUG("y[1] = 0x%2x, y[2] = 0x%2x, y[3] = 0x%2x, y[4] = 0x%2x.\n",
		lcd_disp_y[1], lcd_disp_y[2], lcd_disp_y[3], lcd_disp_y[4]);

	mipi_dsi_cmds_tx(set_display_address, \
		ARRAY_SIZE(set_display_address), hisifd->mipi_dsi0_base);
	return 0;
}

static ssize_t mipi_sharp_NT36870_lcd_model_show(struct platform_device *pdev,
	char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;
	ssize_t ret = 0;

	if (NULL == pdev) {
		HISI_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	ret = snprintf(buf, PAGE_SIZE, "sharp_NT36870 CMD TFT Boston\n");

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static ssize_t mipi_sharp_panel_lcd_check_reg_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;
	char __iomem *mipi_dsi0_base = NULL;
	uint32_t read_value[5] = {0};
	uint32_t expected_value[5] = {0x9c, 0x00, 0x7, 0x00, 0x00};
	uint32_t read_mask[5] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	char* reg_name[5] = {"power mode", "MADCTR", "pixel format", "image mode", "mipi error"};
	char lcd_reg_0a[] = {0x0a};
	char lcd_reg_0b[] = {0x0b};
	char lcd_reg_0c[] = {0x0c};
	char lcd_reg_0d[] = {0x0d};
	char lcd_reg_ab[] = {0xab};

	struct dsi_cmd_desc lcd_check_reg[] = {
		{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
			sizeof(lcd_reg_0a), lcd_reg_0a},
		{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
			sizeof(lcd_reg_0b), lcd_reg_0b},
		{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
			sizeof(lcd_reg_0c), lcd_reg_0c},
		{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
			sizeof(lcd_reg_0d), lcd_reg_0d},
		{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
			sizeof(lcd_reg_ab), lcd_reg_ab},
	};

	struct mipi_dsi_read_compare_data data = {
		.read_value = read_value,
		.expected_value = expected_value,
		.read_mask = read_mask,
		.reg_name = reg_name,
		.log_on = 1,
		.cmds = lcd_check_reg,
		.cnt = ARRAY_SIZE(lcd_check_reg),
	};

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	mipi_dsi0_base = hisifd->mipi_dsi0_base;

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);
	if (!mipi_dsi_read_compare(&data, mipi_dsi0_base)) {
		ret = snprintf(buf, PAGE_SIZE, "OK\n");
	} else {
		ret = snprintf(buf, PAGE_SIZE, "ERROR\n");
	}
	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}


/*******************************************************************************
**
*/
static struct hisi_panel_info g_panel_info = {0};
static struct hisi_fb_panel_data g_panel_data = {
	.panel_info = &g_panel_info,
	.set_fastboot = mipi_sharp_NT36870_panel_set_fastboot,
	.on = mipi_sharp_NT36870_panel_on,
	.off = mipi_sharp_NT36870_panel_off,
	.remove = mipi_sharp_NT36870_panel_remove,
	.set_backlight = mipi_sharp_NT36870_panel_set_backlight,
	.set_display_region = mipi_sharp_NT36870_panel_set_display_region,

	.lcd_model_show = mipi_sharp_NT36870_lcd_model_show,
	.lcd_check_reg  = mipi_sharp_panel_lcd_check_reg_show,
};

/*******************************************************************************
**
*/
static int lcd_voltage_relevant_init(struct platform_device *pdev)
{
	int ret = 0;

	if (g_lcd_fpga_flag == 0) {
		ret = vcc_cmds_tx(pdev, lcd_vcc_init_cmds,
			ARRAY_SIZE(lcd_vcc_init_cmds));
		if (ret != 0) {
			HISI_FB_ERR("LCD vcc init failed!\n");
			return ret;
		}

		ret = pinctrl_cmds_tx(pdev, lcd_pinctrl_init_cmds,
			ARRAY_SIZE(lcd_pinctrl_init_cmds));
		if (ret != 0) {
			HISI_FB_ERR("Init pinctrl failed, defer\n");
			return ret;
		}

		if (is_fastboot_display_enable()) {
			vcc_cmds_tx(pdev, lcd_vcc_enable_cmds,
				ARRAY_SIZE(lcd_vcc_enable_cmds));
		}
	}

	return ret;
}

static void panel_mode_swtich_para_store(struct hisi_panel_info *pinfo)
{
	if (NULL == pinfo) {
		return;
	}

	/*59Hz, soc-ic vporch ok*/
	pinfo->ldi.hbp_store_vid = 50;
	pinfo->ldi.hfp_store_vid = 200;
	pinfo->ldi.hpw_store_vid = 50;
	pinfo->ldi.vbp_store_vid = 70;
	pinfo->ldi.vfp_store_vid = 28;
	pinfo->ldi.vpw_store_vid = 18;
	pinfo->ldi.pxl_clk_store_vid = 332 * 1000000UL;

	pinfo->ldi.hbp_store_cmd = pinfo->ldi.h_back_porch;
	pinfo->ldi.hfp_store_cmd = pinfo->ldi.h_front_porch;
	pinfo->ldi.hpw_store_cmd = pinfo->ldi.h_pulse_width;
	pinfo->ldi.vbp_store_cmd = pinfo->ldi.v_back_porch ;
	pinfo->ldi.vfp_store_cmd = pinfo->ldi.v_front_porch;
	pinfo->ldi.vpw_store_cmd = pinfo->ldi.v_pulse_width;
	pinfo->ldi.pxl_clk_store_cmd = pinfo->pxl_clk_rate;

	return;
}

static int mipi_sharp_NT36870_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_panel_info *pinfo = NULL;
	struct device_node *np = NULL;
	uint32_t bl_type = 0;
	uint32_t lcd_display_type = 0;
	uint32_t lcd_ifbc_type = 0;

	HISI_FB_INFO("mipi_sharp_NT36870_probe+!\n");
	g_mipi_lcd_name = SHARP_2LANE_NT36870;
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_SHARP_NT36870);
	if (!np) {
		HISI_FB_ERR("not found device node %s!\n", DTS_COMP_SHARP_NT36870);
		goto err_return;
	}

	ret = of_property_read_u32(np, LCD_DISPLAY_TYPE_NAME, &lcd_display_type);
	if (ret) {
		HISI_FB_ERR("get lcd_display_type failed!\n");
		lcd_display_type = PANEL_MIPI_CMD;
	}

	ret = of_property_read_u32(np, LCD_IFBC_TYPE_NAME, &lcd_ifbc_type);
	if (ret) {
		HISI_FB_ERR("get ifbc_type failed!\n");
		lcd_ifbc_type = IFBC_TYPE_NONE;
	}

	ret = of_property_read_u32(np, LCD_BL_TYPE_NAME, &bl_type);
	if (ret) {
		HISI_FB_ERR("get lcd_bl_type failed!\n");
		bl_type = BL_SET_BY_MIPI;
	}
	HISI_FB_INFO("bl_type=0x%x.", bl_type);

	if (hisi_fb_device_probe_defer(lcd_display_type, bl_type))
		goto err_probe_defer;

	HISI_FB_DEBUG("+.\n");

	ret = of_property_read_u32(np, FPGA_FLAG_NAME, &g_lcd_fpga_flag);//lint !e64
	if (ret) {
		HISI_FB_WARNING("need to get g_lcd_fpga_flag resource in fpga, not needed in asic!\n");
	}

	if (g_lcd_fpga_flag == 1) {
		gpio_lcd_p5v8_enable = of_get_named_gpio(np, "gpios", 1);
		gpio_lcd_n5v8_enable = of_get_named_gpio(np, "gpios", 2);
		gpio_lcd_reset = of_get_named_gpio(np, "gpios", 3);
		gpio_lcd_bl_enable = of_get_named_gpio(np, "gpios", 4);
		gpio_lcd_1v8 = of_get_named_gpio(np, "gpios", 0);
	} else {
		gpio_lcd_p5v5_enable = of_get_named_gpio(np, "gpios", 0);
		gpio_lcd_n5v5_enable = of_get_named_gpio(np, "gpios", 1);
		gpio_lcd_reset = of_get_named_gpio(np, "gpios", 2);
		gpio_lcd_bl_enable = of_get_named_gpio(np, "gpios", 3);
		gpio_lcd_id0 = of_get_named_gpio(np, "gpios", 4);
	}

	pdev->id = 1;
	//init lcd panel info
	pinfo = g_panel_data.panel_info;
	memset(pinfo, 0, sizeof(struct hisi_panel_info));
	pinfo->xres = 1440;
	pinfo->yres = 2560;
	pinfo->width = 75;
	pinfo->height = 133;
	pinfo->orientation = LCD_PORTRAIT;
	pinfo->bpp = LCD_RGB888;
	pinfo->bgr_fmt = LCD_RGB;
	pinfo->bl_set_type = bl_type;

	pinfo->bl_min = 1;
	pinfo->bl_max = 255;
	pinfo->bl_default = 102;

	pinfo->frc_enable = 0;
	pinfo->esd_enable = 0;
	pinfo->esd_skip_mipi_check = 0;
	pinfo->lcd_uninit_step_support = 1;

	/*for 10bit_video - 8bit_cmd mode switch*/
	pinfo->panel_mode_swtich_support = 1;

	pinfo->current_mode = MODE_10BIT_VIDEO_3X;
	pinfo->mode_switch_to = pinfo->current_mode;

	if (pinfo->current_mode == MODE_10BIT_VIDEO_3X) {
		pinfo->type = PANEL_MIPI_VIDEO;
	} else {
		pinfo->type = lcd_display_type;
	}

	if (g_lcd_fpga_flag == 1) {
		if (pinfo->type == PANEL_MIPI_CMD) {
			pinfo->ldi.h_back_porch = 96;//0x6ff
			pinfo->ldi.h_front_porch = 1068; //108
			pinfo->ldi.h_pulse_width = 48;
			pinfo->ldi.v_back_porch = 12;
			pinfo->ldi.v_front_porch = 14;
			pinfo->ldi.v_pulse_width = 4;

			pinfo->mipi.dsi_bit_clk = 240;
			pinfo->mipi.dsi_bit_clk_val1 = 220;
			pinfo->mipi.dsi_bit_clk_val2 = 230;
			pinfo->mipi.dsi_bit_clk_val3 = 240;
			pinfo->mipi.dsi_bit_clk_val4 = 250;
			pinfo->mipi.dsi_bit_clk_val5 = 260;
			pinfo->dsi_bit_clk_upt_support = 0;
			pinfo->mipi.dsi_bit_clk_upt = pinfo->mipi.dsi_bit_clk;
			pinfo->pxl_clk_rate = 20* 1000000UL;
		} else {
			pinfo->ldi.h_back_porch = 96;//0x6ff
			pinfo->ldi.h_front_porch = 1068; //108
			pinfo->ldi.h_pulse_width = 48;
			pinfo->ldi.v_back_porch = 12;
			pinfo->ldi.v_front_porch = 14;
			pinfo->ldi.v_pulse_width = 4;

			pinfo->mipi.dsi_bit_clk = 240;
			pinfo->mipi.dsi_bit_clk_val1 = 220;
			pinfo->mipi.dsi_bit_clk_val2 = 230;
			pinfo->mipi.dsi_bit_clk_val3 = 240;
			pinfo->mipi.dsi_bit_clk_val4 = 250;
			pinfo->mipi.dsi_bit_clk_val5 = 260;
			pinfo->dsi_bit_clk_upt_support = 0;
			pinfo->mipi.dsi_bit_clk_upt = pinfo->mipi.dsi_bit_clk;
			pinfo->pxl_clk_rate = 20* 1000000UL;
		}

	} else {
		if (pinfo->type == PANEL_MIPI_CMD) {
			//ldi
			pinfo->ldi.h_back_porch = 66;//96
			pinfo->ldi.h_front_porch = 78;//108
			pinfo->ldi.h_pulse_width = 30;//48
			pinfo->ldi.v_back_porch = 32;
			pinfo->ldi.v_front_porch = 30;
			pinfo->ldi.v_pulse_width = 18;

			//mipi
			pinfo->mipi.dsi_bit_clk = 750;
			pinfo->dsi_bit_clk_upt_support = 0;
			pinfo->mipi.dsi_bit_clk_upt = pinfo->mipi.dsi_bit_clk;
			pinfo->pxl_clk_rate = 277 * 1000000UL;
		} else {
			int tmp = 4;
			if (tmp == 3) {
				pinfo->ldi.h_back_porch = 50;
				pinfo->ldi.h_front_porch = 240;
				pinfo->ldi.h_pulse_width = 50;
				pinfo->ldi.v_back_porch = 32;
				pinfo->ldi.v_front_porch = 30;
				pinfo->ldi.v_pulse_width = 18;
				pinfo->pxl_clk_rate = 332 * 1000000UL;
			} else if (tmp == 4) {
				pinfo->ldi.h_back_porch = 40;
				pinfo->ldi.h_front_porch = 180;
				pinfo->ldi.h_pulse_width = 40;
				pinfo->ldi.v_back_porch = 70;
				pinfo->ldi.v_front_porch = 28;
				pinfo->ldi.v_pulse_width = 18;
				pinfo->pxl_clk_rate = 332 * 1000000UL;
			}
			pinfo->mipi.dsi_bit_clk = 750;
			pinfo->dsi_bit_clk_upt_support = 0;
			pinfo->mipi.dsi_bit_clk_upt = pinfo->mipi.dsi_bit_clk;

		}
	}

	if (pinfo->panel_mode_swtich_support) {
		panel_mode_swtich_para_store(pinfo);
	}

	pinfo->mipi.dsi_version = DSI_1_1_VERSION;

	pinfo->mipi.clk_post_adjust = 215;
	pinfo->mipi.clk_pre_adjust= 0;
	pinfo->mipi.clk_t_hs_prepare_adjust= 0;
	pinfo->mipi.clk_t_lpx_adjust= 0;
	pinfo->mipi.clk_t_hs_trial_adjust= 0;
	pinfo->mipi.clk_t_hs_exit_adjust= 0;
	pinfo->mipi.clk_t_hs_zero_adjust= 0;
	pinfo->mipi.non_continue_en = 1;

	//mipi
	pinfo->mipi.lane_nums = DSI_2_LANES;//DSI_3_LANES;
	pinfo->mipi.color_mode = DSI_24BITS_1;//DSI_DSC24_COMPRESSED_DATA

	pinfo->mipi.vc = 0;
	pinfo->mipi.max_tx_esc_clk = 10 * 1000000;
	pinfo->mipi.burst_mode = DSI_BURST_SYNC_PULSES_1;
	pinfo->mipi.phy_mode = CPHY_MODE;

	pinfo->vsync_ctrl_type = 0;//VSYNC_CTRL_ISR_OFF | VSYNC_CTRL_MIPI_ULPS | VSYNC_CTRL_CLK_OFF;

	if (pinfo->type == PANEL_MIPI_CMD) {
		pinfo->dirty_region_updt_support = 0;
		pinfo->dirty_region_info.left_align = -1;
		pinfo->dirty_region_info.right_align = -1;
		pinfo->dirty_region_info.top_align = 32;
		pinfo->dirty_region_info.bottom_align = 32;
		pinfo->dirty_region_info.w_align = -1;
		pinfo->dirty_region_info.h_align = -1;
		pinfo->dirty_region_info.w_min = 1440;
		pinfo->dirty_region_info.h_min = 32;
		pinfo->dirty_region_info.top_start = -1;
		pinfo->dirty_region_info.bottom_start = -1;
	}
	/* The host processor must wait for more than 15us from the end of write data transfer to a command 2Ah/2Bh */
	if (pinfo->dirty_region_updt_support == 1) {
		pinfo->mipi.hs_wr_to_time = 17000;        /* measured in nS */
	}

	//IFBC_TYPE_VESA3X_DUAL & IFBC_TYPE_VESA3_75X_DUAL are both supported
	pinfo->ifbc_type = IFBC_TYPE_VESA3X_DUAL;//IFBC_TYPE_VESA3_75X_DUAL;//
	pinfo->pxl_clk_rate_div = 3;

	get_vesa_dsc_para(pinfo, pinfo->current_mode);

	if (g_lcd_fpga_flag == 0) {
		if (pinfo->pxl_clk_rate_div > 1) {
			pinfo->ldi.h_back_porch /= pinfo->pxl_clk_rate_div;
			pinfo->ldi.h_front_porch /= pinfo->pxl_clk_rate_div;
			pinfo->ldi.h_pulse_width /= pinfo->pxl_clk_rate_div;
		}
	}

	ret = lcd_voltage_relevant_init(pdev);
	if (ret != 0) {
		goto err_return;
	}

	ret = platform_device_add_data(pdev, &g_panel_data,
		sizeof(struct hisi_fb_panel_data));
	if (ret) {
		HISI_FB_ERR("platform_device_add_data failed!\n");
		goto err_device_put;
	}

	hisi_fb_add_device(pdev);

	HISI_FB_DEBUG("-.\n");

	return 0;

err_device_put:
	platform_device_put(pdev);
err_return:
	return ret;
err_probe_defer:
	return -EPROBE_DEFER;

}

static const struct of_device_id hisi_panel_match_table[] = {
	{
		.compatible = DTS_COMP_SHARP_NT36870,
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, hisi_panel_match_table);

static struct platform_driver this_driver = {
	.probe = mipi_sharp_NT36870_probe,
	.remove = NULL,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = "mipi_sharp_NT36870",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hisi_panel_match_table),
	},
};

static int __init mipi_sharp_NT36870_panel_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&this_driver);
	if (ret) {
		HISI_FB_ERR("platform_driver_register failed, error=%d!\n", ret);
		return ret;
	}

	return ret;
}

/*lint -restore*/
module_init(mipi_sharp_NT36870_panel_init);
/*lint +e551 +e551*/

