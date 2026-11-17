/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: specific MTK-4G functions for BootDetector.
 * Author: Gong Chen <gongchen4@huawei.com>
 * Create: 2020-11-22
 */
#include <hwbootfail/chipsets/bootfail_mtk_4g.h>

#include <linux/printk.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <soc/mediatek/log_store_kernel.h>

#include <hwbootfail/chipsets/bootfail_mtk.h>
#include <hwbootfail/chipsets/common/bootfail_common.h>
#include "adapter_mtk.h"

#define AP_PRESS_TIMEOUT 6

#define BOOT_PHASE_DEV_PATH        LONG_PRESS_DEV_PATH
#define BOOT_PHASE_OFFSET          0x4FFC00             /* 5MB - 1KB */

static u32 g_last_boot_phase;
static struct boot_context *g_boot_context = NULL;
static u32 g_boot_phase = BOOT_PHASE_LK;

void init_boot_context(void)
{
	unsigned int addr;
	unsigned int size;
	struct sram_log_header *bf_sram_header = NULL;

	bf_sram_header = get_sram_header();
	if (bf_sram_header != NULL)
		g_boot_context = &bf_sram_header->boot_cxt;
}

static void set_boot_phase_to_disk(struct work_struct *work)
{
	write_part(BOOT_PHASE_DEV_PATH, BOOT_PHASE_OFFSET,
		(const char*)&g_boot_phase, sizeof(u32));
}

static DECLARE_WORK(set_boot_phase_work, &set_boot_phase_to_disk);

void set_boot_phase(u32 phase)
{
	if (g_boot_context != NULL)
		g_boot_context->boot_phase = phase;

	g_boot_phase = phase;

	schedule_work(&set_boot_phase_work);
}

u32 get_boot_phase(void)
{
	return g_boot_phase;
}

static void ap_press6s_callback(unsigned long data)
{
	print_err("Press pwrkey 6s happened\n");
	save_long_press_info();
}

DEFINE_TIMER(pwrkey_timer, ap_press6s_callback, 0, AP_PRESS_TIMEOUT);

void check_pwrkey_long_press(unsigned long pressed)
{
	if (pressed == 1)
		mod_timer(&pwrkey_timer, jiffies + AP_PRESS_TIMEOUT * HZ);
	else if (pressed == 0)
		del_timer_sync(&pwrkey_timer);
}
