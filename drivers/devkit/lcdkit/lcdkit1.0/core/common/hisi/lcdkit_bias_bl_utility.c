/*
 * lcdkit_bias_bl_utility.c
 *
 * lcdkit bias bl utility function for lcd driver
 *
 * Copyright (c) 2018-2020 Huawei Technologies Co., Ltd.
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

#include "lcdkit_bias_bl_utility.h"
#include "lcdkit_disp.h"
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif

struct device_node* lcdkit_lcd_np = NULL;

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
void set_lcd_bias_dev_flag(void)
{
	set_hw_dev_flag(DEV_I2C_DC_DC);
	return;
}
#endif

int lcdkit_get_backlight_ic_name(char* buf, int len)
{
	struct device_node* np = NULL;
	char *tmp_name = NULL;
	int name_len;

	if(buf == NULL)
	{
		return -1;
	}
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_LCDKIT_PANEL_TYPE);
	if(!np)
	{
		printk("NOT FOUND device node %s!\n", DTS_COMP_LCDKIT_PANEL_TYPE);
		return -1;
	}

	tmp_name = (char*)of_get_property(np, "lcd-bl-ic-name", NULL);
	if(NULL == tmp_name)
	{
		return -1;
	}

	name_len = strlen(tmp_name);
	if(name_len == 0)
	{
		return -1;
	}

	if(name_len < len)
	{
		memcpy(buf, tmp_name, name_len + 1);
	}
	else
	{
		memcpy(buf, tmp_name, len-1);
	}
	buf[len-1] = 0;
	return 0;
}
int lcdkit_get_bias_ic_name(char* buf, int len)
{
	struct device_node* np = NULL;
	char *tmp_name = NULL;
	int name_len;

	if(buf == NULL)
	{
		return -1;
	}
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_LCDKIT_PANEL_TYPE);
	if(!np)
	{
		printk("NOT FOUND device node %s!\n", DTS_COMP_LCDKIT_PANEL_TYPE);
		return -1;
	}

	tmp_name = (char*)of_get_property(np, "lcd-bias-ic-name", NULL);
	if(NULL == tmp_name)
	{
		return -1;
	}

	name_len = strlen(tmp_name);
	if(name_len == 0)
	{
		return -1;
	}

	if(name_len < len)
	{
		memcpy(buf, tmp_name, name_len + 1);
	}
	else
	{
		memcpy(buf, tmp_name, len-1);
	}
	buf[len-1] = 0;

	return 0;
}

struct device_node* lcdkit_get_lcd_node(void)
{
	return lcdkit_lcd_np;
}

void lcdkit_set_lcd_node(const struct device_node* pnode)
{
	if (pnode) {
		printk("lcdkit_set_lcd_node ok!\n");
		lcdkit_lcd_np = pnode;
	} else {
		printk("lcdkit pnode is NULL Point!\n");
	}
	return;
}

uint32_t lcdkit_check_lcd_plugin(void)
{
	return g_fake_lcd_flag;
}
