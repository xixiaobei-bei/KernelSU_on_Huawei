/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table liteon source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "als_para_table_liteon.h"

#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <securec.h>

#include "als_tp_color.h"
#include "contexthub_boot.h"
#include "contexthub_route.h"

/*
 * Although the GRAY and Black TP's RGB ink is same ,
 * but some product may has both the GRAY
 * and Black TP,so must set the als para for  GRAY and Black TP
 */
ltr582_als_para_table ltr582_als_para_diff_tp_color_table[] = {
	{ OTHER, OTHER, DEFAULT_TPLCD, OTHER,
	  { 1, 4, 1200, 400, 2635, 2649, 2990, 758, 150, 155, 2200, 1993, 4496,
	    1180, 2350, 1970, 500, 850, 0, 0, 0, 0, 0, 0, 1000, 1000 }
	},
	{ EMILY, V3, DEFAULT_TPLCD, BLACK,
	  { 1, 4, 1500, 100, 1690, 1910, 2410, 740, 50, 155, 2200, 1993, 4496,
	    1100, 2350, 2070, 500, 800, 5082, 7687, 7674, 318, 4188, 7970,
	    4000, 250 }
	},
	{ EMILY, V3, DEFAULT_TPLCD, BLACK2,
	  { 1, 4, 1500, 100, 1690, 1910, 2410, 740, 50, 155, 2200, 1993, 4496,
	    1100, 2350, 2070, 500, 800, 5082, 7687, 7674, 318, 4188, 7970,
	    4000, 250 }
	},
	{ EMILY, V3, DEFAULT_TPLCD, GRAY,
	  { 1, 4, 1500, 100, 1690, 1910, 2410, 740, 50, 155, 2200, 1993, 4496,
	    1100, 2350, 2070, 500, 800, 5082, 7687, 7674, 318, 4188, 7970,
	    4000, 250 }
	},
	{ EMILY, V3, DEFAULT_TPLCD, WHITE,
	  { 1, 4, 1500, 100, 1690, 1910, 2410, 740, 50, 155, 2200, 1993, 4496,
	    1100, 2350, 2070, 500, 800, 5082, 7687, 7674, 318, 4188, 7970,
	    4000, 250 }
	},
	{ EMILY, V3, DEFAULT_TPLCD, BLUE,
	  { 1, 4, 1500, 100, 1690, 1910, 2410, 740, 50, 155, 2200, 1993, 4496,
	    1100, 2350, 2070, 500, 800, 5082, 7687, 7674, 318, 4188, 7970,
	    4000, 250 }
	},
	{ CHARLOTTE, V3, DEFAULT_TPLCD, BLACK,
	  { 1, 4, 1800, 100, 2050, 2050, 2650, 740, 25, 155, 2200, 1993, 4496,
	    1180, 2350, 1970, 500, 850, 4798, 6990, 7177, 632, 4394, 7800,
	    4000, 250 }
	},
	{ CHARLOTTE, V3, DEFAULT_TPLCD, BLACK2,
	  { 1, 4, 1800, 100, 2050, 2050, 2650, 740, 25, 155, 2200, 1993, 4496,
	    1180, 2350, 1970, 500, 850, 4798, 6990, 7177, 632, 4394, 7800,
	    4000, 250 }
	},
	{ CHARLOTTE, V3, DEFAULT_TPLCD, GRAY,
	  { 1, 4, 1800, 100, 2050, 2050, 2650, 740, 25, 155, 2200, 1993, 4496,
	    1180, 2350, 1970, 500, 850, 4798, 6990, 7177, 632, 4394, 7800,
	    4000, 250 }
	},
	{ CHARLOTTE, V3, DEFAULT_TPLCD, WHITE,
	  { 1, 4, 1800, 100, 2050, 2050, 2650, 740, 25, 155, 2200, 1993, 4496,
	    1180, 2350, 1970, 500, 850, 4798, 6990, 7177, 632, 4394, 7800,
	    4000, 250 }
	},
	{ CHARLOTTE, V3, DEFAULT_TPLCD, BLUE,
	  { 1, 4, 1800, 100, 2050, 2050, 2650, 740, 25, 155, 2200, 1993, 4496,
	    1180, 2350, 1970, 500, 850, 4798, 6990, 7177, 632, 4394, 7800,
	    4000, 250 }
	},
	{ CHARLOTTE, V3, DEFAULT_TPLCD, PINK_GOLD,
	  { 1, 4, 1800, 100, 2050, 2050, 2650, 740, 25, 155, 2200, 1993, 4496,
	    1180, 2350, 1970, 500, 850, 4798, 6990, 7177, 632, 4394, 7800,
	    4000, 250 }
	},
	{ CHARLOTTE, VN1, DEFAULT_TPLCD, BLACK,
	  { 1, 4, 1400, 100, 1600, 1800, 2200, 740, 25, 155, 2200, 1993, 4496,
	    1180, 2350, 1970, 500, 850, 4112, 6954, 6295, 513, 4394, 7800,
	    4000, 250 }
	},
	{ CHARLOTTE, VN1, DEFAULT_TPLCD, BLACK2,
	  { 1, 4, 1400, 100, 1300, 1450, 1800, 740, 25, 155, 2200, 1993, 4496,
	    1180, 2350, 1970, 500, 850, 5494, 8766, 8273, 558, 4394, 7800,
	    4000, 250 }
	},
	{ CHARLOTTE, VN1, DEFAULT_TPLCD, BROWN,
	  { 1, 4, 1300, 100, 1850, 2120, 2730, 740, 25, 155, 2200, 1993, 4496,
	    1180, 2350, 1970, 500, 850, 4115, 5870, 6223, 220, 4394, 7800,
	    4000, 250 }
	},
	{ CHARLOTTE, VN2, DEFAULT_TPLCD, BLACK,
	  { 1, 4, 1300, 100, 1850, 2120, 2730, 740, 25, 155, 2200, 1993, 4496,
	    1180, 2350, 1970, 500, 850, 4115, 5870, 6223, 220, 4394, 7800,
	    4000, 250 }
	},
	{ CHARLOTTE, VN2, DEFAULT_TPLCD, BLACK2,
	  { 1, 4, 1400, 100, 1300, 1450, 1800, 740, 25, 155, 2200, 1993, 4496,
	    1180, 2350, 1970, 500, 850, 5494, 8766, 8273, 558, 4394, 7800,
	    4000, 250 }
	},
	{ CHARLOTTE, VN2, DEFAULT_TPLCD, BROWN,
	  { 1, 4, 1300, 100, 1850, 2120, 2730, 740, 25, 155, 2200, 1993, 4496,
	    1180, 2350, 1970, 500, 850, 4115, 5870, 6223, 220, 4394, 7800,
	    4000, 250 }
	},
};

/*
 * in the array of pinhole_als_para_diff_tp_color_table,
 * these figures represent the parameter for the als sensor
 */
pinhole_als_para_table pinhole_als_para_diff_tp_color_table[] = {
	{ BLN, V4, APDS9922, TS_PANEL_UNKNOWN,
	  { 1200, 400, 268, 327, 400, 1, 0, 0, 0, 0 }
	},
	{ BLN, V4, APDS9922, TS_PANEL_OFILIM,
	  { 1200, 400, 314, 380, 536, 1, 0, 0, 0, 0 }
	},
	{ BLN, V4, APDS9922, TS_PANEL_LENS,
	  { 1200, 400, 268, 327, 400, 1, 0, 0, 0, 0 }
	},
	{ BLN, V4, APDS9922, TS_PANEL_EELY,
	  { 1300, 400, 303, 348, 560, 1, 0, 0, 0, 0 }
	},

	{ BLN, V4, LTR578, TS_PANEL_UNKNOWN,
	  { 1200, 300, 570, 570, 930, 1, 0, 0, 0, 0 }
	},
	{ BLN, V4, LTR578, TS_PANEL_OFILIM,
	  { 1200, 300, 600, 600, 800, 1, 0, 0, 0, 0 }
	},
	{ BLN, V4, LTR578, TS_PANEL_LENS,
	  { 1200, 300, 450, 530, 620, 1, 0, 0, 0, 0 }
	},
	{ BLN, V4, LTR578, TS_PANEL_EELY,
	  { 1200, 300, 570, 570, 930, 1, 0, 0, 0, 0 }
	},

	{ PRA, V4, APDS9922, TS_PANEL_UNKNOWN,
	  { 980, 350, 306, 366, 558, 1, 0, 0, 0, 0 }
	},
	{ PRA, V4, APDS9922, TS_PANEL_OFILIM,
	  { 980, 350, 330, 366, 620, 1, 0, 0, 0, 0 }
	},
	{ PRA, V4, APDS9922, TS_PANEL_TRULY,
	  { 980, 350, 306, 366, 558, 1, 0, 0, 0, 0 }
	},
	{ PRA, V4, APDS9922, TS_PANEL_EELY,
	  { 980, 350, 310, 390, 656, 1, 0, 0, 0, 0 }
	},

	{ PRA, V4, LTR578, TS_PANEL_UNKNOWN,
	  { 1150, 350, 600, 600, 724, 1, 0, 0, 0, 0 }
	},
	{ PRA, V4, LTR578, TS_PANEL_OFILIM,
	  { 1150, 350, 600, 600, 800, 1, 0, 0, 0, 0 }
	},
	{ PRA, V4, LTR578, TS_PANEL_TRULY,
	  { 1150, 350, 600, 600, 724, 1, 0, 0, 0, 0 }
	},
	{ PRA, V4, LTR578, TS_PANEL_EELY,
	  { 1150, 350, 480, 560, 900, 1, 0, 0, 0, 0 }
	},

	{ WAS, V4, APDS9922, DEFAULT_TPLCD,
	  { 1100, 320, 250, 220, 500, 0, 0, 0, 0, 0 }
	},
	{ WAS, V4, APDS9922, TM_TPLCD,
	  { 1100, 300, 246, 220, 438, 0, 0, 0, 0, 0 }
	},
	{ WAS, V4, APDS9922, AUO_TPLCD,
	  { 1100, 300, 262, 204, 430, 0, 0, 0, 0, 0 }
	},
	{ WAS, V4, APDS9922, EBBG_TPLCD,
	  { 1200, 300, 215, 214, 460, 0, 0, 0, 0, 0 }
	},
	{ WAS, V4, APDS9922, JDI_TPLCD,
	  { 1100, 300, 240, 225, 460, 0, 0, 0, 0, 0 }
	},

	{ WAS, V4, LTR578, DEFAULT_TPLCD,
	  { 1300, 320, 500, 500, 800, 0, 0, 0, 0, 0 }
	},
	{ WAS, V4, LTR578, TM_TPLCD,
	  { 1350, 320, 550, 525, 900, 0, 0, 0, 0, 0 }
	},
	{ WAS, V4, LTR578, AUO_TPLCD,
	  { 1200, 320, 553, 500, 861, 0, 0, 0, 0, 0 }
	},
	{ WAS, V4, LTR578, EBBG_TPLCD,
	  { 1350, 320, 530, 510, 820, 0, 0, 0, 0, 0 }
	},
	{ WAS, V4, LTR578, JDI_TPLCD,
	  { 1200, 320, 510, 490, 850, 0, 0, 0, 0, 0 }
	},

	{ COL, V4, APDS9922, TS_PANEL_UNKNOWN,
	  { 709, 249, 190, 185, 220, 0, 2857, 2273, 15000, 0 }
	},
	{ COL, V4, APDS9922, TS_PANEL_OFILIM,
	  { 710, 249, 190, 185, 220, 0, 2857, 2273, 15000, 0 }
	},
	{ COL, V4, APDS9922, TS_PANEL_TRULY,
	  { 711, 249, 190, 185, 220, 0, 2857, 2273, 15000, 0 }
	},
	{ COL, V4, APDS9922, TS_PANEL_LENS,
	  { 712, 249, 190, 185, 220, 0, 2857, 2273, 15000, 0 }
	},

	{ COL, V4, LTR578, TS_PANEL_UNKNOWN,
	  { 613, 205, 348, 327, 395, 0, 1656, 1199, 15000, 0 }
	},
	{ COL, V4, LTR578, TS_PANEL_OFILIM,
	  { 614, 205, 348, 327, 395, 0, 1656, 1199, 15000, 0 }
	},
	{ COL, V4, LTR578, TS_PANEL_TRULY,
	  { 615, 205, 348, 327, 395, 0, 1656, 1199, 15000, 0 }
	},
	{ COL, V4, LTR578, TS_PANEL_LENS,
	  { 616, 205, 348, 327, 395, 0, 1656, 1199, 15000, 0 }
	},

	{ COR, V4, LTR578, TS_PANEL_UNKNOWN,
	  { 516, 140, 406, 393, 500, 0, 1138, 770, 15000, 100 }
	},

	{ PAR, V4, LTR578, TS_PANEL_UNKNOWN,
	  { 1261, 353, 713, 684, 888, 0, 628, 552, 15000, 100 }
	},
	{ PAR, V4, APDS9922, TS_PANEL_UNKNOWN,
	  { 867, 429, 256, 318, 550, 0, 918, 1699, 15000, 0 }
	},

	{ INE, V4, LTR578, TS_PANEL_UNKNOWN,
	  { 1359, 491, 720, 751, 953, 0, 630, 633, 30000, 10 }
	},
	{ INE, V4, APDS9922, TS_PANEL_UNKNOWN,
	  { 1518, 465, 418, 401, 541, 0, 1193, 1263, 30000, 10 }
	},

	{ SNE, V4, LTR578, TS_PANEL_UNKNOWN,
	  { 1460, 514, 672, 694, 813, 0, 790, 838, 30000, 10 }
	},
	{ SNE, V4, APDS9922, TS_PANEL_UNKNOWN,
	  { 1504, 431, 380, 458, 572, 0, 1165, 1414, 30000, 10 }
	},

	{ JSN, V4, LTR578, TS_PANEL_UNKNOWN,
	  { 800, 269, 342, 308, 389, 0, 1631, 2030, 17000, 300 }
	},
	{ JSN, V4, APDS9922, TS_PANEL_UNKNOWN,
	  { 895, 294, 175, 145, 185, 0, 3375, 4270, 17000, 300 }
	},
	{ JKM, V4, LTR578, TS_PANEL_UNKNOWN,
	  { 1804, 537, 320, 398, 688, 0, 1148, 2124, 15000, 0 }
	},
	{ JKM, V4, APDS9922, TS_PANEL_UNKNOWN,
	  { 1804, 537, 320, 398, 688, 0, 1148, 2124, 15000, 0 }
	},
};

als_para_normal_table ltr2568_als_para_diff_tp_color_table[] = {
	{ PCT, V3, TS_PANEL_UNKNOWN, LTR2568_PARA_SIZE,
	  { 1200, 400, 251, 327, 499, 0, 0, 0, 30000, 100 }
	},
	{ SPN, V3, BOE_TPLCD, LTR2568_PARA_SIZE,
	  { 420, 130, 530, 615, 472, 0, 7170, 977, 7500, 200 }
	},
	{ SPN, V3, TM_TPLCD, LTR2568_PARA_SIZE,
	  { 420, 130, 530, 615, 472, 0, 7170, 978, 7500, 200 }
	},
	{ SPN, V3, TS_PANEL_UNKNOWN, LTR2568_PARA_SIZE,
	  { 420, 130, 530, 615, 472, 0, 7170, 979, 7500, 200 }
	},
	{ SEA, V3, TS_PANEL_UNKNOWN, LTR2568_PARA_SIZE,
	  { 5000, 1000, 236, 236, 236, 0, 3500, 330, 7500, 200 }
	},

	{ JNY, V3, TS_PANEL_UNKNOWN, LTR2568_PARA_SIZE,
	  { 230, 50, 512, 514, 418, 0, 7700, 1280, 15000, 300 }
	},

	{ CDY, V3, TM_TPLCD, LTR2568_PARA_SIZE,
	  { 150, 30, 406, 403, 363, 0, 5639, 477, 4000, 50 }
	},
	{ CDY, V3, BOE_TPLCD, LTR2568_PARA_SIZE,
	  { 550, 50, 99, 120, 127, 0, 1978, 252, 7500, 50 }
	},
	{ CDY, V3, CSOT_TPLCD, LTR2568_PARA_SIZE,
	  { 300, 50, 118, 185, 150, 0, 3454, 238, 5500, 50 }
	},

	{ AQM, V3, TS_PANEL_UNKNOWN, LTR2568_PARA_SIZE,
	  { 500, 200, 755, 710, 424, 0, 8280, 1110, 5000, 300 }
	},

	{ TETON, V3, TS_PANEL_UNKNOWN, LTR2568_PARA_SIZE,
	  { 280, 100, 2500, 3050, 2850, 0, 2917, 2639, 4000, 250 }
	},
};

ltr2594_als_para_table ltr2594_als_para[] = {
	{ ELSA, V3, DEFAULT_TPLCD, BLACK,
	  { 1000, 4247, 3710, 528, 2603, 1945, 521, 1285, 2965, 355, 1636, -1556,
	  6214, 3925, 1577, 3153, 3584, 30000, 30 }
	},
};

als_para_normal_table ltr311_als_para[] = {
	{ TEL, V3, TS_PANEL_UNKNOWN, LTR311_PARA_SIZE,
	  { 1906, 502, 59, 37, 30, 0, 817, 56, 10000, 100, 400, 155, 25 }
	},
};

ltr582_als_para_table *als_get_ltr582_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(ltr582_als_para_diff_tp_color_table))
		return NULL;
	return &(ltr582_als_para_diff_tp_color_table[id]);
}

ltr582_als_para_table *als_get_ltr582_first_table(void)
{
	return &(ltr582_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_ltr582_table_count(void)
{
	return ARRAY_SIZE(ltr582_als_para_diff_tp_color_table);
}

pinhole_als_para_table *als_get_pinhole_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(pinhole_als_para_diff_tp_color_table))
		return NULL;
	return &(pinhole_als_para_diff_tp_color_table[id]);
}

pinhole_als_para_table *als_get_pinhole_first_table(void)
{
	return &(pinhole_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_pinhole_table_count(void)
{
	return ARRAY_SIZE(pinhole_als_para_diff_tp_color_table);
}

als_para_normal_table *als_get_ltr2568_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(ltr2568_als_para_diff_tp_color_table))
		return NULL;
	return &(ltr2568_als_para_diff_tp_color_table[id]);
}

als_para_normal_table *als_get_ltr2568_first_table(void)
{
	return &(ltr2568_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_ltr2568_table_count(void)
{
	return ARRAY_SIZE(ltr2568_als_para_diff_tp_color_table);
}

ltr2594_als_para_table *als_get_ltr2594_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(ltr2594_als_para))
		return NULL;
	return &(ltr2594_als_para[id]);
}

ltr2594_als_para_table *als_get_ltr2594_first_table(void)
{
	return &(ltr2594_als_para[0]);
}

uint32_t als_get_ltr2594_table_count(void)
{
	return ARRAY_SIZE(ltr2594_als_para);
}

als_para_normal_table *als_get_ltr311_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(ltr311_als_para))
		return NULL;
	return &(ltr311_als_para[id]);
}

als_para_normal_table *als_get_ltr311_first_table(void)
{
	return &(ltr311_als_para[0]);
}

uint32_t als_get_ltr311_table_count(void)
{
	return ARRAY_SIZE(ltr311_als_para);
}
