/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table ams tmd3702 source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "als_para_table_ams_tsl2540.h"

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

static tsl2540_als_para_table tsl2540_als_para_diff_tp_color_table[] = {
	{ OCEAN, V3, DEFAULT_TPLCD, OTHER,
	 { 1461, 13314, 13005, 1739, -552, 3251, -857, 8170, 804, 4000, 250 }
	},
	{ NOAH, V3, DEFAULT_TPLCD, OTHER,
	 { 1484, 13119, 12932, 2199, 1364, 3233, -792, 6320, 690, 4000, 250 }
	},
};

tsl2540_als_para_table *als_get_tsl2540_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(tsl2540_als_para_diff_tp_color_table))
		return NULL;
	return &(tsl2540_als_para_diff_tp_color_table[id]);
}

tsl2540_als_para_table *als_get_tsl2540_first_table(void)
{
	return &(tsl2540_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_tsl2540_table_count(void)
{
	return ARRAY_SIZE(tsl2540_als_para_diff_tp_color_table);
}
