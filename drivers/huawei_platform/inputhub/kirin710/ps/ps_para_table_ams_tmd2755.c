/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table ams source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */


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
#include "ps_para_table_ams_tmd2755.h"

static tmd2755_ps_para_table tmd2755_ps_para_diff_tp_color_table[] = {
	/* tp_color reserved for future use */
	/*
	 *  AMS TMD2755: Extend-Data Format:
	 *  {PPULSES, binsrc_target, THRESHOLD_L, THRESHOLD_H, pgain1, pgain2, PPULSE_LEN, driver}
	 */
	{PHONE_TYPE_MGA, V4, TS_PANEL_TXD,  0, {60, 31, 70, 150, 1, 5, 52, 8}},
	{PHONE_TYPE_MGA, V4, TS_PANEL_BOE,  0, {36, 31, 70, 135, 1, 5, 40, 7}},
	{PHONE_TYPE_MGA, V4, TS_PANEL_LENS, 0, {40, 31, 60, 140, 1, 5, 40, 8}},
	{PHONE_TYPE_MGA, V4, TS_PANEL_KING, 0, {60, 31, 70, 170, 1, 5, 70, 8}},
	{PHONE_TYPE_MGA, V4, TS_PANEL_TIANMA, 0, {64, 31, 70, 135, 1, 5, 52, 8}},
};

tmd2755_ps_para_table *ps_get_tmd2755_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(tmd2755_ps_para_diff_tp_color_table))
		return NULL;
	return &(tmd2755_ps_para_diff_tp_color_table[id]);
}

tmd2755_ps_para_table *ps_get_tmd2755_first_table(void)
{
	return &(tmd2755_ps_para_diff_tp_color_table[0]);
}

uint32_t ps_get_tmd2755_table_count(void)
{
	return ARRAY_SIZE(tmd2755_ps_para_diff_tp_color_table);
}
