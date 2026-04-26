/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table ams tsl2591 source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "als_para_table_ams_tsl2591.h"

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

static tsl2591_als_para_table tsl2591_als_para_diff_tp_color_table[] = {
	{ HRY, V4, TS_PANEL_UNKNOWN,
	 { 12300, 1550, 3056, 5256, 3239, 8259, 500, 0,
		530, 280, 491, 86, 61, 7500, 300 } },
	{ POT, V4, TS_PANEL_UNKNOWN,
	 { 12300, 1550, 3056, 5256, 3239, 8259, 500, 0,
		530, 280, 491, 86, 61, 7500, 300 } },
	{ STK, V4, BOE_TPLCD,
	 { 6135, 1590, 4458, 7756, 5943, 16046, 500, 0,
		625, 155, 491, 86, 59, 5000, 300 } },
	{ STK, V4, INX_TPLCD,
	 { 8485, 1590, 3616, 6472, 3079, 8005, 500, 0,
		530, 180, 491, 86, 59, 5000, 300 } },
	{ STK, V4, TCL_TPLCD,
	 { 7754, 1631, 2963, 5333, 3158, 8211, 470, 0,
		530, 180, 491, 86, 57, 5000, 300 } },
	{ STK, V4, TM_TPLCD,
	 { 7754, 1631, 2963, 5333, 3158, 8211, 470, 0,
		530, 180, 491, 86, 57, 5000, 300 } },
	{ STK, V4, TS_PANEL_UNKNOWN,
	 { 7754, 1631, 2963, 5333, 3158, 8211, 470, 0,
		530, 180, 491, 86, 57, 5000, 300 } },
	{ ASK, V4, BOE_TPLCD,
	 { 6135, 1590, 4774, 8594, 4140, 12421, 450, 0,
		530, 180, 491, 86, 58, 5000, 300 } },
	{ ASK, V4, TM_TPLCD,
	 { 6135, 1590, 5552, 10050, 4140, 12421, 470, 0,
		530, 180, 491, 86, 58, 5000, 300 } },
	{ ASK, V4, LG_TPLCD,
	 { 6135, 1590, 3736, 6650, 4140, 12421, 450, 0,
		530, 180, 491, 86, 58, 5000, 300 } },
	{ ASK, V4, INX_TPLCD, // LTPS TP
	 { 6135, 1590, 4413, 8098, 3506, 9467, 450, 0,
		530, 180, 491, 86, 58, 5000, 300 } },
	{ ASK, V4, INX_TPLCD2, // A-SI TP
	 { 6135, 1590, 4774, 8594, 3752, 10131, 450, 0,
		530, 180, 491, 86, 58, 5000, 300 } },
	{ ASK, V4, TS_PANEL_UNKNOWN,
	 { 6135, 1590, 5552, 10050, 4140, 12421, 470, 0,
		530, 180, 491, 86, 58, 5000, 300 } },
};

tsl2591_als_para_table *als_get_tsl2591_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(tsl2591_als_para_diff_tp_color_table))
		return NULL;
	return &(tsl2591_als_para_diff_tp_color_table[id]);
}

tsl2591_als_para_table *als_get_tsl2591_first_table(void)
{
	return &(tsl2591_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_tsl2591_table_count(void)
{
	return ARRAY_SIZE(tsl2591_als_para_diff_tp_color_table);
}
