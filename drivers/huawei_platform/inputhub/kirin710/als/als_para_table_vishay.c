/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table vishay source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "als_para_table_vishay.h"

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

static vcnl36658_als_para_table vcnl36658_als_para_diff_tp_color_table[] = {
	{ OTHER, OTHER, DEFAULT_TPLCD, OTHER,
	{ -115, 1000, 5510, 13621, -4177, 5408, -234, 2838, 2838, 15, -2, 20331, -7559, 10828, -513,
	500, 1000, 1000, 1260, 6, -171, 1342, 1000, 0, 0, 0, 0, 0, 0, 0, 0 } },
	{ RAVEL, V3, DEFAULT_TPLCD, WHITE,
	{ -115, 1000, 5510, 13621, -4177, 5408, -234, 2838, 2838, 15, -2, 20331, -7559, 10828, -513,
	500, 1000, 1000, 3780, 6, -171, 3500, 1000, 0, 0, 0, 0, 0, 0, 0, 0 } },
	{ RAVEL, V3, DEFAULT_TPLCD, BLACK,
	{ -115, 1000, 5510, 13621, -4177, 5408, -234, 2838, 2838, 15, -2, 20331, -7559, 10828, -513,
	500, 1000, 1000, 3780, 6, -171, 3500, 1000, 0, 0, 0, 0, 0, 0, 0, 0 } },
	{ RAVEL, V3, DEFAULT_TPLCD, OTHER,
	{ -115, 1000, 5510, 13621, -4177, 5408, -234, 2838, 2838, 15, -2, 20331, -7559, 10828, -513,
	500, 1000, 1000, 3780, 6, -171, 3500, 1000, 0, 0, 0, 0, 0, 0, 0, 0 } },
};

static als_para_normal_table vcnl36832_als_para_diff_tp_color_table[] = {
	{ GLK, V3, TS_PANEL_UNKNOWN, VCNL36832_PARA_SIZE,
	{ -115, 1000, 5511, 13621, -4176, 5408, 2337, 2837, 2837, 15012, 2133,
	20331, 7558, 10828, 513, 5000, 1000, 1111, 1260, 6, 170, 1342, 1000,
	0, 0, 0, 0, 0, 0, 30000, 100} },
};

vcnl36658_als_para_table *als_get_vcnl36658_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(vcnl36658_als_para_diff_tp_color_table))
		return NULL;
	return &(vcnl36658_als_para_diff_tp_color_table[id]);
}

vcnl36658_als_para_table *als_get_vcnl36658_first_table(void)
{
	return &(vcnl36658_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_vcnl36658_table_count(void)
{
	return ARRAY_SIZE(vcnl36658_als_para_diff_tp_color_table);
}

als_para_normal_table *als_get_vcnl36832_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(vcnl36832_als_para_diff_tp_color_table))
		return NULL;
	return &(vcnl36832_als_para_diff_tp_color_table[id]);
}

als_para_normal_table *als_get_vcnl36832_first_table(void)
{
	return &(vcnl36832_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_vcnl36832_table_count(void)
{
	return ARRAY_SIZE(vcnl36832_als_para_diff_tp_color_table);
}
