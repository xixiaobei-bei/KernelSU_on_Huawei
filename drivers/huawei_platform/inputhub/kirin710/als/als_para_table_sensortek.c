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

#include "tp_color.h"
#include "contexthub_boot.h"
#include "contexthub_route.h"

static stk3321_als_para_table stk3321_als_table[] = {
/*
 * STK3321 Extend Data Format
 * { als_ratio, a_light_ratio, day_light_ratio,
 * a_light_slope, day_light_slope, cwf_light_slope,
 * middle_als_data, offset_max, offset_min }
 */
	{ AGS3, V4, STK3321, TS_PANEL_UNKNOWN, BLACK,
		{ 1000, 3000, 2000, 7002, 7000, 7001, 517, 10000, 200 } },
	{ AGS3, V4, STK3321, TS_PANEL_UNKNOWN, WHITE,
		{ 1000, 3000, 2000, 7002, 7000, 7001, 500, 10000, 200 } },
};

static stk3235_als_para_table stk3235_als_table[] = {
/*
 * STK3235 Extend Data Format
 * { a_light_ratio, cwf_light_ratio,
 * a_light_slope, day_light_slope, cwf_light_slope,
 * middle_als_data, offset_max, offset_min }
 */
	{ AGS3, V4, STK3235, TS_PANEL_UNKNOWN, BLACK,
		{ 3000, 2000, 1002, 1000, 1001, 3864, 10000, 200 } },
	{ AGS3, V4, STK3235, TS_PANEL_UNKNOWN, WHITE,
		{ 2250, 1800, 600, 600, 600, 5916, 10000, 200 } },
};

static als_para_normal_table stk3338_als_para_diff_tp_color_table[] = {
	{ GLK, V3, AUO_TPLCD, STK3338_PARA_SIZE,
	 { 1000, 3000, 6000, 135, 188, 280, 8033, 7500, 200} },
	{ GLK, V3, CTC_TPLCD, STK3338_PARA_SIZE,
	 { 1000, 3000, 6000, 135, 188, 280, 8033, 7500, 200} },
	{ GLK, V3, TM_TPLCD, STK3338_PARA_SIZE,
	 { 1000, 3000, 6000, 135, 188, 280, 8033, 7500, 200} },
	{ GLK, V3, BOE_TPLCD, STK3338_PARA_SIZE,
	 { 1000, 3000, 6000, 135, 188, 280, 8033, 7500, 200} },
	{ GLK, V3, TS_PANEL_UNKNOWN, STK3338_PARA_SIZE,
	 { 1000, 3000, 6000, 135, 188, 280, 8033, 7500, 200} },
	{ LRA, V3, TS_PANEL_UNKNOWN, STK3338_PARA_SIZE,
	 { 1000, 6500, 3000, 98, 131, 258, 7250, 5000, 300} },
};

stk3321_als_para_table *als_get_stk3321_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(stk3321_als_table))
		return NULL;
	return &(stk3321_als_table[id]);
}

stk3321_als_para_table *als_get_stk3321_first_table(void)
{
	return &(stk3321_als_table[0]);
}

uint32_t als_get_stk3321_table_count(void)
{
	return ARRAY_SIZE(stk3321_als_table);
}

stk3235_als_para_table *als_get_stk3235_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(stk3235_als_table))
		return NULL;
	return &(stk3235_als_table[id]);
}

stk3235_als_para_table *als_get_stk3235_first_table(void)
{
	return &(stk3235_als_table[0]);
}

uint32_t als_get_stk3235_table_count(void)
{
	return ARRAY_SIZE(stk3235_als_table);
}

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
