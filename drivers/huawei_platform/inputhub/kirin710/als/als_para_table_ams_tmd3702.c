/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table ams tmd3702 source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "als_para_table_ams_tmd3702.h"

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

static tmd3702_als_para_table tmd3702_als_para_diff_tp_color_table[] = {
	{OTHER, OTHER, DEFAULT_TPLCD, OTHER,
	 {50, 804, 1250, -1130, 1720, -1820, 0, 804, 1250, -1130, 1720, -1820, 0, 8, 9463, 0,
	  0, 1534, 5814, 0, 0, 1962, 264, 0, 0, 0, 0, 4000, 200} },
	{HIMA, V3, DEFAULT_TPLCD, GRAY,
	 {50, 804, 1250, -1130, 1720, -1820, 0, 804, 1250, -1130, 1720, -1820, 0, 8, 9463, 0,
	  0, 1534, 5814, 0, 0, 1962, 264, 8670, 6623, 4495, 18640, 4000, 200} },
	{HIMA, V3, DEFAULT_TPLCD, BLACK,
	 {50, 804, 1250, -1130, 1720, -1820, 0, 804, 1250, -1130, 1720, -1820, 0, 8, 9463, 0,
	  0, 1534, 5814, 0, 0, 1962, 264, 8670, 6623, 4495, 18640, 4000, 200} },
	{HIMA, V3, DEFAULT_TPLCD, BLACK2,
	 {50, 804, 1250, -1130, 1720, -1820, 0, 804, 1250, -1130, 1720, -1820, 0, 8, 9463, 0,
	  0, 1534, 5814, 0, 0, 1962, 264, 8670, 6623, 4495, 18640, 4000, 200} },
	{HIMA, V3, DEFAULT_TPLCD, GOLD,
	 {50, 800, 1820, -1930, 4600, -4020, 0, 800, 1820, -1930, 4600, -4020, 0, 10, 7706, 0,
	  0, 1519, 4187, 0, 0, 2123, 266, 5504, 4679, 3925, 13640, 4000, 200} },

};

tmd3702_als_para_table *als_get_tmd3702_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(tmd3702_als_para_diff_tp_color_table))
		return NULL;
	return &(tmd3702_als_para_diff_tp_color_table[id]);
}

tmd3702_als_para_table *als_get_tmd3702_first_table(void)
{
	return &(tmd3702_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_tmd3702_table_count(void)
{
	return ARRAY_SIZE(tmd3702_als_para_diff_tp_color_table);
}
