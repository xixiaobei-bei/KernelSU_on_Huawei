/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table silergy source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "als_para_table_silergy.h"

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

static sy3079_als_para_table sy3079_als_table[] = {
	{ AGS3, V4, SY3079, TS_PANEL_UNKNOWN, BLACK,
		{ 248, 900, 4000, 4000, 1120, 1000, 1000, 15184, 10000, 200 } },
	{ AGS3, V4, SY3079, TS_PANEL_UNKNOWN, WHITE,
		{ 166, 2500, 7000, 7000, 1188, 1000, 1000, 19120, 10000, 200 } },
};

static als_para_normal_table syh399_als_para[] = {
	{ BLM, V4, TS_PANEL_UNKNOWN, SYH399_PARA_SIZE,
		{ 66, 876, 300, 120, 300, 9039, 10000, -4954, 13681,
		24412, 1, 4315, 10000, 100, 600, 230, 32767 }
	},
};

sy3079_als_para_table *als_get_sy3079_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(sy3079_als_table))
		return NULL;
	return &(sy3079_als_table[id]);
}

sy3079_als_para_table *als_get_sy3079_first_table(void)
{
	return &(sy3079_als_table[0]);
}

uint32_t als_get_sy3079_table_count(void)
{
	return ARRAY_SIZE(sy3079_als_table);
}

als_para_normal_table *als_get_syh399_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(syh399_als_para))
		return NULL;
	return &(syh399_als_para[id]);
}

als_para_normal_table *als_get_syh399_first_table(void)
{
	hwlog_info("als_get_syh399_first_table enter\n");
	return &(syh399_als_para[0]);
}

uint32_t als_get_syh399_table_count(void)
{
	return ARRAY_SIZE(syh399_als_para);
}
