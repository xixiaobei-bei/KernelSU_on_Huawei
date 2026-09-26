/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table vishay source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "als_para_table_vishay.h"

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

vcnl36658_als_para_table vcnl36658_als_para_diff_tp_color_table[] = {
	{ OTHER, OTHER, DEFAULT_TPLCD, OTHER,
	  { 110, 0, 2850, 0, 0, 0, 0, 6261, -1108, 1226, -4670, 21820, -11130,
	    1711, 0, 500, 1000, 1000, 710, 10, -159, 779, 1672, 1024, 1211,
	    24, 1500, 5500, 30000, 100 }
	},
	{ RAVEL, V3, DEFAULT_TPLCD, WHITE,
	  { 110, 0, 2850, 0, 0, 0, 0, 6261, -1108, 1226, -4670, 21820, -11130,
	    1711, 0, 500, 1000, 1000, 710, 10, -159, 779, 1672, 1024, 1211,
	    24, 1500, 5500, 30000, 100 }
	},
	{ RAVEL, V3, DEFAULT_TPLCD, BLACK,
	  { 110, 0, 2850, 0, 0, 0, 0, 6261, -1108, 1226, -4670, 21820, -11130,
	    1711, 0, 500, 1000, 1000, 710, 10, -159, 779, 1672, 1024, 1211,
	    24, 1500, 5500, 30000, 100 }
	},
	{ RAVEL, V3, DEFAULT_TPLCD, OTHER,
	  { 110, 0, 2850, 0, 0, 0, 0, 6261, -1108, 1226, -4670, 21820, -11130,
	    1711, 0, 500, 1000, 1000, 710, 10, -159, 779, 1672, 1024, 1211,
	    24, 1500, 5500, 30000, 100 }
	},
};

als_para_normal_table vcnl36832_als_para_diff_tp_color_table[] = {
	{ PCT, V3, TS_PANEL_UNKNOWN, VCNL36832_PARA_SIZE,
	  { -115, 1000, 5511, 13621, -4176, 5408, 2337, 2837, 2837, 15012,
	    2133, 20331, 7558, 10828, 513, 5000, 1000, 1111, 1260, 6, 170,
	    1342, 1000, 0, 0, 0, 0, 0, 0, 30000, 100 }
	},
};

als_para_normal_table veml32185_als_para[] = {
	{ HLK, V4, AUO_TPLCD, VEML32185_PARA_SIZE,
	  {15000, 3560, 5760, 714, 2996, 2960, 840, 150, 100, 500, 5000, 330 }
	},

	{ HLK, V4, BOE_TPLCD, VEML32185_PARA_SIZE,
	  {15000, 3560, 5760, 714, 2996, 2960, 840, 150, 100, 500, 5000, 330 }
	},

	{ HLK, V4, INX_TPLCD, VEML32185_PARA_SIZE,
	  {15000, 3560, 5760, 714, 2996, 2960, 840, 150, 100, 500, 5000, 330 }
	},

	{ HLK, V4, TCL_TPLCD, VEML32185_PARA_SIZE,
	  {15000, 3560, 5760, 714, 2996, 2960, 840, 150, 100, 500, 5000, 330 }
	},

	{ HLK, V4, TM_TPLCD, VEML32185_PARA_SIZE,
	  {15000, 3560, 5760, 714, 2996, 2960, 840, 150, 100, 500, 5000, 330 }
	},

	{ HLK, V4, TS_PANEL_UNKNOWN, VEML32185_PARA_SIZE,
	  {15000, 3560, 5760, 714, 2996, 2960, 840, 150, 100, 500, 5000, 330 }
	},
};

vcnl36658_als_para_table *als_get_vcnl36658_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(vcnl36658_als_para_diff_tp_color_table))
		return NULL;
	return &(vcnl36658_als_para_diff_tp_color_table[id]);
}

vcnl36658_als_para_table *als_get_vcnl36658_first_table(void)
{
	return &(vcnl36658_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_vcnl36658_table_count(void)
{
	return ARRAY_SIZE(vcnl36658_als_para_diff_tp_color_table);
}

als_para_normal_table *als_get_vcnl36832_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(vcnl36832_als_para_diff_tp_color_table))
		return NULL;
	return &(vcnl36832_als_para_diff_tp_color_table[id]);
}

als_para_normal_table *als_get_vcnl36832_first_table(void)
{
	return &(vcnl36832_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_vcnl36832_table_count(void)
{
	return ARRAY_SIZE(vcnl36832_als_para_diff_tp_color_table);
}

als_para_normal_table *als_get_veml32185_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(veml32185_als_para))
		return NULL;
	return &(veml32185_als_para[id]);
}

als_para_normal_table *als_get_veml32185_first_table(void)
{
	return &(veml32185_als_para[0]);
}

uint32_t als_get_veml32185_table_count(void)
{
	return ARRAY_SIZE(veml32185_als_para);
}
