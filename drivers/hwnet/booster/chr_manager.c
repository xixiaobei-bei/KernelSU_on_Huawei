/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019. All rights reserved.
 * Description: This module is to report chr information to booster
 * Author: yaozhendong1@huawei.com
 * Create: 2019-12-10
 */

#include "chr_manager.h"

#ifdef CONFIG_HUAWEI_BASTET
#include <huawei_platform/net/bastet/bastet_fastgrab.h>
#endif

#define EXPIRE_TIME (30 * HZ)

static notify_event *g_notifier = NULL;
static struct timer_list g_timer;

static void stat_report(unsigned long sync)
{
#ifdef CONFIG_HUAWEI_BASTET
	regist_msg_fun(g_notifier);
#endif
}

void chr_manager_init(notify_event *notify)
{
	init_timer(&g_timer);
	g_timer.data = 0;
	g_timer.function = stat_report;
	mod_timer(&g_timer, jiffies + EXPIRE_TIME);
	g_notifier = notify;
}

