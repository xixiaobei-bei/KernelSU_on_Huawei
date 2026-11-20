/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table st source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "als_para_table_st.h"

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

vd6281_als_para_table vd6281_als_para_diff_tp_color_table[] = {
	{ LAYA, V4, DEFAULT_TPLCD, BLACK,
	  {25727, 4044, 9520, 0, 4589, 0, 4000, 250 }
	},
	{ BAH3DV, V3, TS_PANEL_UNKNOWN, WHITE,
		{ 25728, 4044, 9520, 0, 4589, 0, 10000, 0 }
	},
	{ BAH3DV, V3, TS_PANEL_UNKNOWN, BLACK,
		{ 25729, 4044, 9520, 0, 4589, 0, 10000, 0 }
	},
	{ MARXC, V3, TS_PANEL_UNKNOWN, WHITE,
		{ 8833, 12748, 4176, 0, 96, 0, 10000, 0 }
	},
	{ MARXC, V3, TS_PANEL_UNKNOWN, BLACK,
		{ 6874, 9265, 2569, 0, 100, 0, 10000, 0 }
	},
};

vd6281_als_para_table *als_get_vd6281_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(vd6281_als_para_diff_tp_color_table))
		return NULL;
	return &(vd6281_als_para_diff_tp_color_table[id]);
}

vd6281_als_para_table *als_get_vd6281_first_table(void)
{
	return &(vd6281_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_vd6281_table_count(void)
{
	return ARRAY_SIZE(vd6281_als_para_diff_tp_color_table);
}
