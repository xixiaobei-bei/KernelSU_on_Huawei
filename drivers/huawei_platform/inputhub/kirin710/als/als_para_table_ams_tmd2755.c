/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table ams source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "als_para_table_ams.h"

#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <securec.h>

#include "tp_color.h"
#include "contexthub_boot.h"
#include "contexthub_route.h"

static tmd2755_als_para_table tmd2755_als_para_diff_tp_color_table[] = {
	/* tp_color reserved for future use */
	/*
	 *    AMS TMD2755: Extend-Data Format:
	 *    { CoefA, DFA, CoefB, DFB, CoefC, DFC,
	 *    Diff1,  Diff2, middle_vis, middle_ref}
	 */
	{PHONE_TYPE_MGA, V4, TS_PANEL_TXD,  0,
		{0.29, 2950.413, 0.65, 8476.19, 1.996, 23645, 0.2, 1.1, 2608, 37}
	},
	{PHONE_TYPE_MGA, V4, TS_PANEL_BOE,  0,
		{0.1, 1318.75, 0.6, 5208.333, 0.284, 19431, 0.2, 1.1, 3085, 41}
	},
	{PHONE_TYPE_MGA, V4, TS_PANEL_LENS, 0,
		{0.4095, 2413.627, 0.7687, 9909.71, -2.490136, 21958.12, 0.2, 1.138, 2058, 28}
	},
	{PHONE_TYPE_MGA, V4, TS_PANEL_KING, 0,
		{-0.633, 851.663, -0.333, 2055.989, 0.491, 20100, 0.45, 0.9, 2194, 24}
	},
	{PHONE_TYPE_MGA, V4, TS_PANEL_TIANMA, 0,
		{0.307, 1598.133, -176.198, 11.869, -8.606, 11431.56, 0.45, 1.15, 4061, 41}
	},
};

tmd2755_als_para_table *als_get_tmd2755_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(tmd2755_als_para_diff_tp_color_table))
		return NULL;
	return &(tmd2755_als_para_diff_tp_color_table[id]);
}

tmd2755_als_para_table *als_get_tmd2755_first_table(void)
{
	return &(tmd2755_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_tmd2755_table_count(void)
{
	return ARRAY_SIZE(tmd2755_als_para_diff_tp_color_table);
}
