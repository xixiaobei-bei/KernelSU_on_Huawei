/*
 * bias_bl_utils.c
 *
 * bias_bl_utils driver for mtk paltform i2c and dts
 * operation function
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include "bias_bl_utils.h"
#include "bias_bl_common.h"
#include "lcd_kit_disp.h"
#ifdef CONFIG_HUAWEI_DEV_SELFCHECK
#include <huawei_platform/dev_detect/hw_dev_detect.h>
#endif
#if defined CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

static struct dsm_client *get_lcd_dsm_client(void)
{
#if defined CONFIG_HUAWEI_DSM
	return lcd_kit_get_lcd_dsm_client();
#else
	return NULL;
#endif
}

static void lcd_set_hw_dev_detect(void)
{
#ifdef CONFIG_HUAWEI_DEV_SELFCHECK
	set_hw_dev_detect_result(DEV_DETECT_DC_DC);
#endif
}

static struct bias_bl_common_ops bias_bl_common_ops = {
	.get_lcd_dsm_client = get_lcd_dsm_client,
	.set_hw_dev_detect = lcd_set_hw_dev_detect,
};

void bias_bl_ops_init(void)
{
	bias_bl_adapt_register(&bias_bl_common_ops);
}
