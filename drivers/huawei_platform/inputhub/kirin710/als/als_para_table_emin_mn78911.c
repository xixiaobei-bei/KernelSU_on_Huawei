/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table ams source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "als_para_table_emin_mn78911.h"

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

static mn78911_als_para_table mn78911_als_para_diff_tp_color_table[] = {
	/* tp_color reserved for future use */
	/*
	 * AMS mn78911: Extend-Data Format:
	 * {
	 * mn_sensor.als.als_aintt_ll
	 * mn_sensor.als.als_acycle_ll
	 * mn_sensor.als.als_ag_ll2l_thd
	 * mn_sensor.als.offset_gain
	 * mn_sensor.als.scale_gain
	 * mn_sensor.als.factory.lux_per_count
	 * mn_sensor.als.factory.lux_per_lux
	 * mn_sensor.als.als_ag_l2m_thd
	 * mn_sensor.als.als_aintt_h
	 * mn_sensor.als.als_aintt_m
	 * mn_sensor.als.als_aintt_l
	 * middle_visible_data
	 * middle_ref_data}
	 */
	{EVE, V4, TS_PANEL_TIANMA, 0,
		{7, 6, 200, 220, 500, 260, 1000, 200, 15, 13, 11, 7815, 41}
	},
	{EVE, V4, TS_PANEL_SKYWORTH, 0,
		{7, 6, 200, 200, 400, 220, 1000, 200, 15, 13, 11, 9230, 41}
	},
};

mn78911_als_para_table *als_get_mn78911_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(mn78911_als_para_diff_tp_color_table))
		return NULL;
	return &(mn78911_als_para_diff_tp_color_table[id]);
}

mn78911_als_para_table *als_get_mn78911_first_table(void)
{
	return &(mn78911_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_mn78911_table_count(void)
{
	return ARRAY_SIZE(mn78911_als_para_diff_tp_color_table);
}
