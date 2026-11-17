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

static tmd2745_als_para_table tmd2745_als_para_diff_tp_color_table[] = {
	/* tp_color reserved for future use */
	/* AMS TMD2745: Extend-Data Format -- {D_Factor, B_Coef, C_Coef, D_Coef, is_min_algo, is_auto_gain} */
	/* Here use WAS as an example */
	{WAS, V4, DEFAULT_TPLCD, 0, {538, 95, 1729, 190, 0, 0, 0, 0, 0, 0}},

	{COL, V4, TS_PANEL_UNKNOWN, 0, {510, 142, 803, 60, 0, 0, 493, 482, 5000, 200}},
	{COL, V4, TS_PANEL_OFILIM, 0, {510, 142, 803, 60, 0, 0, 494, 482, 5000, 200}},
	{COL, V4, TS_PANEL_TRULY, 0, {510, 142, 803, 60, 0, 0, 495, 482, 5000, 200}},
	{COL, V4, TS_PANEL_LENS, 0, {510, 142, 803, 60, 0, 0, 496, 482, 5000, 200}},

};

tmd2745_als_para_table *als_get_tmd2745_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(tmd2745_als_para_diff_tp_color_table))
		return NULL;
	return &(tmd2745_als_para_diff_tp_color_table[id]);
}

tmd2745_als_para_table *als_get_tmd2745_first_table(void)
{
	return &(tmd2745_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_tmd2745_table_count(void)
{
	return ARRAY_SIZE(tmd2745_als_para_diff_tp_color_table);
}
