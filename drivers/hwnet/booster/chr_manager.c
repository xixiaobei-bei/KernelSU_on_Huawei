/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: This module is to report chr information to booster
 * Author: yaozhendong1@huawei.com
 * Create: 2019-12-10
 */

#include "chr_manager.h"
#include <linux/version.h>

#ifdef CONFIG_HUAWEI_BASTET
#include <huawei_platform/net/bastet/bastet_fastgrab.h>
#endif

#define EXPIRE_TIME (30 * HZ)

static notify_event *g_notifier = NULL;
static struct timer_list g_timer;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0)
static void stat_report(struct timer_list *sync)
#else
static void stat_report(unsigned long sync)
#endif
{
#ifdef CONFIG_HUAWEI_BASTET
	regist_msg_fun(g_notifier);
#endif
}

void chr_manager_init(notify_event *notify)
{
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0)
	timer_setup(&g_timer, stat_report, 0);
	#else
	init_timer(&g_timer);
	g_timer.data = 0;
	g_timer.function = stat_report;
	#endif
	mod_timer(&g_timer, jiffies + EXPIRE_TIME);
	g_notifier = notify;
}

