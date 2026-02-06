/*
 * lcdkit_debug_hs.c
 *
 * lcdkit debug hisi function for lcd driver
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

#include "hisi_fb.h"
#include "lcdkit_dbg.h"

extern struct platform_device *g_hisi_pdev;
void *lcdkit_get_dsi_ctrl_pdata(void)
{
	return platform_get_drvdata(g_hisi_pdev);
}

int lcdkit_lock(void *pdata)
{
	struct hisi_fb_data_type* hisifd = pdata;

	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on)
	{
		LCDKIT_ERR("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}
	return 0;

err_out:
	up(&hisifd->blank_sem);
	return -EFAULT;
}
void lcdkit_release(void *pdata)
{
	struct hisi_fb_data_type* hisifd = pdata;
	up(&hisifd->blank_sem);
}

void lcdkit_debug_set_vsp_vsn(void* vcc_cmds, int cnt)
{
	int i;
	struct vcc_desc* cm = NULL;
	cm = (struct vcc_desc*) vcc_cmds;

	for (i = 0; i < cnt; i++)
	{
		if (cm->dtype == DTYPE_VCC_SET_VOLTAGE)
		{
			if (!strncmp(cm->id, VCC_LCDKIT_BIAS_NAME, strlen(cm->id)))
			{
				cm->min_uV = lcdkit_dbg.lcdkit_panel_bias;
				cm->max_uV = lcdkit_dbg.lcdkit_panel_bias;
			}
			else if (!strncmp(cm->id, VCC_LCDKIT_VSP_NAME, strlen(cm->id)))
			{
				cm->min_uV = lcdkit_dbg.lcdkit_panel_vsp;
				cm->max_uV = lcdkit_dbg.lcdkit_panel_vsp;
			}
			else if (!strncmp(cm->id, VCC_LCDKIT_VSN_NAME, strlen(cm->id)))
			{
				cm->min_uV = lcdkit_dbg.lcdkit_panel_vsn;
				cm->max_uV = lcdkit_dbg.lcdkit_panel_vsn;
			}
		}

		cm++;
	}
}
