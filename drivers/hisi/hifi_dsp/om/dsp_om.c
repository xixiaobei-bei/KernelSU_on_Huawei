/*
 * dsp_om.c
 *
 * socdsp om.
 *
 * Copyright (c) 2013-2020 Huawei Technologies Co., Ltd.
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

#include "dsp_om.h"

#include <linux/io.h>
#include <linux/hisi/audio_log.h>

#include "dsp_misc.h"
#include "om_bigdata.h"
#include "om_debug.h"

struct dsp_om {
	bool dsp_loaded;
	unsigned int *dsp_loaded_indicate_addr;
	unsigned int dsp_loaded_sign;
	unsigned int print_level;
	unsigned int *dsp_print_level_addr;
	unsigned int *dsp_timestamp_addr;
};

static struct dsp_om g_om_priv;
/*lint -e548 */
static bool check_dsp_img_loaded(void)
{
	bool dsp_loaded = false;

	g_om_priv.dsp_loaded_sign = (unsigned int)readl(g_om_priv.dsp_loaded_indicate_addr);

	if (g_om_priv.dsp_loaded_sign > 0)
		loge("dsp img loaded error: 0x%x\n", g_om_priv.dsp_loaded_sign);
	else
		dsp_loaded = true;

	return dsp_loaded;
}

bool is_dsp_img_loaded(void)
{
	if (!g_om_priv.dsp_loaded)
		loge("dsp img isn't loaded, errno is 0x%x\n", g_om_priv.dsp_loaded_sign);

	return g_om_priv.dsp_loaded;
}

void om_set_print_level(unsigned int level)
{
	g_om_priv.print_level = level;
}

unsigned int om_get_print_level(void)
{
	return g_om_priv.print_level;
}

unsigned int om_get_dsp_timestamp(void)
{
	unsigned int time = 0;

	if (g_om_priv.dsp_timestamp_addr)
		time = (unsigned int)readl(g_om_priv.dsp_timestamp_addr);
	else
		printk(LOG_TAG"time stamp reg not init\n"); /* can't use logx */

	return time;
}

void dsp_om_init(struct platform_device *pdev, unsigned char *unsec_virt_addr)
{
	int ret;

	IN_FUNCTION;

	if (!pdev || !unsec_virt_addr) {
		loge("para is null\n");
		return;
	}

	memset(&g_om_priv, 0, sizeof(g_om_priv));

	if (dsp_misc_get_platform_type() == DSP_PLATFORM_FPGA)
		g_om_priv.print_level = PRINT_LEVEL_ERROR;
	else
		g_om_priv.print_level = PRINT_LEVEL_INFO;

	g_om_priv.dsp_timestamp_addr = (unsigned int *)ioremap(SYS_TIME_STAMP_REG, 0x4); /*lint !e446 */
	if (!g_om_priv.dsp_timestamp_addr) {
		printk(LOG_TAG"time stamp reg ioremap error\n"); /* can't use logx */
		return;
	}

	g_om_priv.dsp_print_level_addr = (unsigned int *)(unsec_virt_addr +
		(DRV_DSP_UART_LOG_LEVEL - DSP_UNSEC_BASE_ADDR));
	g_om_priv.dsp_loaded_indicate_addr = (unsigned int *)(unsec_virt_addr +
		(DRV_DSP_LOADED_INDICATE - DSP_UNSEC_BASE_ADDR));

	*(unsigned int *)g_om_priv.dsp_print_level_addr = PRINT_LEVEL_INFO;
	g_om_priv.dsp_loaded = check_dsp_img_loaded();

	ret = om_debug_init(unsec_virt_addr);
	if (ret != 0) {
		loge("om debug init error\n");
		iounmap(g_om_priv.dsp_timestamp_addr);
		g_om_priv.dsp_timestamp_addr = NULL;
		return;
	}

	ret = om_bigdata_init(pdev);
	if (ret != 0) {
		loge("om bigdata init error\n");
		om_debug_deinit();
		iounmap(g_om_priv.dsp_timestamp_addr);
		g_om_priv.dsp_timestamp_addr = NULL;
		return;
	}

	OUT_FUNCTION;
}

void dsp_om_deinit(struct platform_device *dev)
{
	IN_FUNCTION;

	if (g_om_priv.dsp_timestamp_addr != NULL) {
		iounmap(g_om_priv.dsp_timestamp_addr);
		g_om_priv.dsp_timestamp_addr = NULL;
	}

	om_debug_deinit();

	om_bigdata_deinit();

	OUT_FUNCTION;
}
/*lint +e548 */
