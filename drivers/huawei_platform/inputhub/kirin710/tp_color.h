/*
 * tp_color.h
 *
 * tp color header file
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __LINUX_LCDPARA_H__
#define __LINUX_LCDPARA_H__

#include "contexthub_boot.h"

//lint -save -e732 -e713
enum PHONE_TYPE {
	NEXT = 11,
	KNIGHT,
	EVA,
	FARADAY,
	VIENNA,
	VNS,
	EDISON,
	BTV,
	NATASHA,
	CHICAGO,
	CMB,
	LON,
	VICTORIA,
	VICKY,
	BLN,
	PRA,
	DUKE,
	WAS,
	ALPS,
	BLANC,
	EMILY,
	BKL = 32,
	NEO = 33,
	CHARLOTTE = 34,
	COL = 35,
	HIMA = 36,
	LAYA = 37,
	COR = 38,
	PAR = 39,
	RAVEL = 40,
	INE = 41,
	SNE = 42,
	JSN = 43,
	JKM = 44,
	HRY = 45,
	POT = 46,
	MAR = 47,
	STK = 48,
	GLK = 49,
	ASK = 50,
	LRA = 51,
	AQM = 52,
	AGS3 = 53,
	BLM = 54,
};

enum PHONE_VERSION {
	V3 = 10,
	V3_A = 101,
	V3_D = 102,
	V4 = 11,
	VN1,
	VN2,
	V1,
};

enum ts_panel_id {
	TS_PANEL_OFILIM 	 	= 0,
	TS_PANEL_EELY		 	= 1,
	TS_PANEL_TRULY	 		= 2,
	TS_PANEL_MUTTO	 		= 3,
	TS_PANEL_GIS		 	= 4,
	TS_PANEL_JUNDA	 		= 5,
	TS_PANEL_LENS 		 	= 6,
	TS_PANEL_YASSY	 		= 7,
	TS_PANEL_JDI 		 	= 6,
	TS_PANEL_SAMSUNG  		= 9,
	TS_PANEL_LG 		 	= 10,
	TS_PANEL_TIANMA 	 	= 11,
	TS_PANEL_CMI 		 	= 12,
	TS_PANEL_BOE  		 	= 13,
	TS_PANEL_CTC 		 	= 14,
	TS_PANEL_EDO 		 	= 15,
	TS_PANEL_SHARP	 		= 16,
	TS_PANEL_AUO 			= 17,
	TS_PANEL_TOPTOUCH 		= 18,
	TS_PANEL_BOE_BAK		= 19,
	TS_PANEL_CTC_BAK 		= 20,
	TS_PANEL_UNKNOWN 		= 0xFF,
};

#define DEFAULT_TPLCD 0

#define WHITE	0xE1
#define BLACK	0xD2
#define BLACK2   0x4B
#define PINK  	0xC3
#define PINK_GOLD 0x1E
#define RED		0xB4
#define YELLOW	0xA5
#define BLUE	0x96
#define GOLD  0x87
#define GRAY  0x78
#define BROWN  0x69
#define CAFE_2  0x5A
#define SILVER  0x3C
#define OTHER  0x00
#define VCNL36832_PARA_SIZE 31
#define STK3338_PARA_SIZE 9
#define LTR2568_PARA_SIZE 10

#endif /* __LINUX_LCDPARA_H__ */
