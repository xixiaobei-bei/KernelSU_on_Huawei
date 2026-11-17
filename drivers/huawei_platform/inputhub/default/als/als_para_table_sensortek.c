/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table sensortek source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */
#include "als_para_table_sensortek.h"

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

als_para_normal_table stk3338_als_para_diff_tp_color_table[] = {
	{ PCT, V3, TS_PANEL_UNKNOWN, STK3338_PARA_SIZE,
	  { 1000, 3000, 2000, 1002, 1001, 1000, 4000, 30000, 100 }
	},
	{ SPN, V3, BOE_TPLCD, STK3338_PARA_SIZE,
	  { 1000, 8000, 4800, 115, 191, 360, 4577, 7500, 200 }
	},
	{ SPN, V3, TM_TPLCD, STK3338_PARA_SIZE,
	  { 1000, 6000, 2800, 152, 225, 360, 4578, 7500, 200 }
	},
	{ SPN, V3, TS_PANEL_UNKNOWN, STK3338_PARA_SIZE,
	  { 1000, 8000, 4800, 115, 191, 360, 4579, 7500, 200 }
	},
	{ SEA, V3, TS_PANEL_UNKNOWN, STK3338_PARA_SIZE,
	  { 1000, 6500, 3000, 144, 195, 390, 4200, 7500, 200 }
	},

	{ JNY, V3, TS_PANEL_UNKNOWN, STK3338_PARA_SIZE,
	  { 1000, 3000, 6000, 135, 188, 280, 5300, 15000, 300 }
	},

	{ CDY, V3, TM_TPLCD, STK3338_PARA_SIZE,
	  { 1000, 3000, 1800, 94, 117, 149, 10518, 7500, 50 }
	},
	{ CDY, V3, BOE_TPLCD, STK3338_PARA_SIZE,
	  { 1000, 5000, 3000, 270, 380, 748, 2107, 7500, 50 }
	},
	{ CDY, V3, CSOT_TPLCD, STK3338_PARA_SIZE,
	  { 1000, 5600, 3300, 210, 295, 1000, 2024, 5200, 50 }
	},

	{ AQM, V3, TS_PANEL_UNKNOWN, STK3338_PARA_SIZE,
	  { 1000, 6500, 3000, 98, 131, 258, 7250, 5000, 300 }
	},

	{ ELSA, V3, TS_PANEL_UNKNOWN, STK3338_PARA_SIZE,
	  { 1000, 4200, 2800, 71, 88, 102, 25418, 7500, 200 }
	},

	{ LION, V3, TS_PANEL_UNKNOWN, STK3338_PARA_SIZE,
	  { 1000, 3900, 2200, 66, 76, 90, 27250, 7500, 300 }
	},

	{ LION, V4, TS_PANEL_UNKNOWN, STK3338_PARA_SIZE,
	  { 1000, 3900, 2200, 66, 76, 90, 27250, 7500, 300 }
	},

	{ LION, VN1, TS_PANEL_UNKNOWN, STK3338_PARA_SIZE,
	  { 1000, 3900, 2200, 66, 76, 90, 27250, 7500, 300 }
	},

	{ TETON, V3, TS_PANEL_UNKNOWN, STK3338_PARA_SIZE,
	  { 1000, 4200, 2800, 66, 70, 89, 4621, 4000, 250 }
	},
};

als_para_normal_table stk32670_als_para[] = {
	{ ANNA, V3, DEFAULT_TPLCD, STK32670_PARA_SIZE,
	  { 1000, 1720, 1000, 42, 48, 48, 13471, 8635, 4000, 250 }
	},

	{ ANG, V3, VISI_TPLCD, STK32670_PARA_SIZE,
	  { 1000, 1400, 700, 31, 42, 43, 10221, 7344, 4000, 250 }
	},

	{ ANG, V3, BOE_TPLCD, STK32670_PARA_SIZE,
	  { 1000, 1700, 700, 28, 36, 38, 10123, 7270, 4000, 250 }
	},

	{ ANG, V3, DEFAULT_TPLCD, STK32670_PARA_SIZE,
	  { 0 }
	},
	{ TETON, V3, BOE_TPLCD, STK32670_PARA_SIZE,
	  { 1000, 1900, 1000, 115, 115, 145, 16789, 19414, 4000, 250 }
	},
	{ TETON, V3, SAMSUNG_TPLCD, STK32670_PARA_SIZE,
	  { 1000, 1500, 1000, 39, 45, 45, 5089, 5266, 4000, 0 }
	},

	{ JER, V3, BOE_TPLCD, STK32670_PARA_SIZE,
	  { 1000, 1500, 1000, 28, 38, 37, 9357, 6471, 10000, 100 }
	},

	{ JER, V3, VISI_TPLCD, STK32670_PARA_SIZE,
	  { 1000, 1500, 1000, 20, 25, 26, 12972, 9229, 10000, 100 }
	},

	{ EDIN, V3, BOE_TPLCD, STK32670_PARA_SIZE,
	  { 1000, 1500, 1000, 28, 38, 38, 9248, 6248, 10000, 100 }
	},

	{ EDIN, V3, VISI_TPLCD, STK32670_PARA_SIZE,
	  { 1000, 1300, 850, 18, 23, 24, 14285, 9234, 10000, 100 }
	},
};

als_para_normal_table stk32671_als_para[] = {
	{ TEL, V3, DEFAULT_TPLCD, STK32671_PARA_SIZE,
	  { 1000, 270, 190, 155, 180, 180, 19019, 4000, 250 }
	},
};

als_para_normal_table *als_get_stk3338_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(stk3338_als_para_diff_tp_color_table))
		return NULL;
	return &(stk3338_als_para_diff_tp_color_table[id]);
}

als_para_normal_table *als_get_stk3338_first_table(void)
{
	return &(stk3338_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_stk3338_table_count(void)
{
	return ARRAY_SIZE(stk3338_als_para_diff_tp_color_table);
}

als_para_normal_table *als_get_stk32670_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(stk32670_als_para))
		return NULL;
	return &(stk32670_als_para[id]);
}

als_para_normal_table *als_get_stk32670_first_table(void)
{
	return &(stk32670_als_para[0]);
}

uint32_t als_get_stk32670_table_count(void)
{
	return ARRAY_SIZE(stk32670_als_para);
}

als_para_normal_table *als_get_stk32671_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(stk32671_als_para))
		return NULL;
	return &(stk32671_als_para[id]);
}

als_para_normal_table *als_get_stk32671_first_table(void)
{
	return &(stk32671_als_para[0]);
}

uint32_t als_get_stk32671_table_count(void)
{
	return ARRAY_SIZE(stk32671_als_para);
}
