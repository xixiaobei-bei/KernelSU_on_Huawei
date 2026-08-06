/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: als tp color header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2012-01-06
 */

#ifndef __ALS_TP_COLOR_H__
#define __ALS_TP_COLOR_H__

#include <linux/types.h>

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
	LON, /* 22 */
	VICTORIA,
	VICKY,
	BLN,
	PRA, /* 26 */
	DUKE,
	WAS,
	ALPS, /* 29 */
	BLANC,
	EMILY, /* 31 */
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
	EVER = 45,
	TNY = 46,
	ELLE = 47,
	VOGUE = 48,
	PCT = 49,
	TAHITI = 50,
	SCM = 51,
	YAL = 52,
	HLK = 53,
	VRD = 54,
	SPN = 55,
	SEA = 56,
	TAURUS = 57,
	LION = 58,
	MARX = 59,
	OXF = 60,
	WLZ = 61,
	JNY = 62,
	BAH3 = 63,
	ELSA = 64,
	ANNA = 65,
	EDIN = 66,
	BMH = 67,
	JEF = 68,
	JER = 69,
	CDY = 70,
	TEL = 71, /* TELL */
	MXW = 72, /* MAXWELL */
	FRL = 73, /* FRANKLIN */
	AQM = 74, /* AQUAMAN */
	ELSAP = 75,
	OCEAN = 76,
	NOAH = 77,
	NOAHP = 78,
	WGR = 79,
	SCMR = 80,
	KRJ = 81,
	TETON = 82,
	JSC = 83,
	ANG = 84,
	BRQ = 85,
	JADE = 86,
	BAH3DV = 87,
	MARXC = 88,
};

enum PHONE_VERSION {
	V3 = 10, /* decimal base */
	V3_A = 101,
	V3_D = 102,
	V4 = 11,
	VN1,
	VN2,
	V1,
};

enum ts_panel_id {
	TS_PANEL_OFILIM = 0,
	TS_PANEL_EELY = 1,
	TS_PANEL_TRULY = 2,
	TS_PANEL_MUTTO = 3,
	TS_PANEL_GIS = 4,
	TS_PANEL_JUNDA = 5,
	TS_PANEL_LENS = 6,
	TS_PANEL_YASSY = 7,
	TS_PANEL_JDI = 6,
	TS_PANEL_SAMSUNG = 9,
	TS_PANEL_LG = 10,
	TS_PANEL_TIANMA = 11,
	TS_PANEL_CMI = 12,
	TS_PANEL_BOE = 13,
	TS_PANEL_CTC = 14,
	TS_PANEL_EDO = 15,
	TS_PANEL_SHARP = 16,
	TS_PANEL_AUO = 17,
	TS_PANEL_TOPTOUCH = 18,
	TS_PANEL_BOE_BAK = 19,
	TS_PANEL_CTC_BAK = 20,
	TS_PANEL_UNKNOWN = 0xFF,
};

#define DEFAULT_TPLCD 0

#define WHITE     0xE1
#define BLACK     0xD2
#define BLACK2    0x4B
#define PINK      0xC3
#define PINK_GOLD 0x1E
#define RED       0xB4
#define YELLOW    0xA5
#define BLUE      0x96
#define GOLD      0x87
#define GRAY      0x78
#define BROWN     0x69
#define CAFE_2    0x5A
#define SILVER    0x3C
#define OTHER     0x00

int get_tpcolor_from_nv(int32_t tag);

#endif
