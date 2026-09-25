/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 *  Description: hisee flash driver
 *  Author : security-ap
 *  Create : 2017/9/15
 */

#include <linux/completion.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/kthread.h>
#include <linux/hisi/hisi_efuse.h>
#include "hisi_flash_hisee_otp.h"

static struct task_struct *g_flash_hisee_otp_task;
int (*g_write_hisee_otp_fn)(void);
static DECLARE_COMPLETION(g_hisee_complete);
static enum tag_efuse_log_level g_efuse_print_level = LOG_LEVEL_ERROR;

static int flash_hisee_otp_value(void)
{
	if (g_write_hisee_otp_fn)
		return g_write_hisee_otp_fn();
	else
		return OK;
}

static int flash_otp_task(void *arg)
{
	int ret;

	wait_for_completion(&g_hisee_complete);
	ret = flash_hisee_otp_value();

	g_flash_hisee_otp_task = NULL;
	return ret;
}

/* resets the complete->done field to 0 ("not done") */
void reinit_hisee_complete(void)
{
	reinit_completion(&g_hisee_complete);
}

/* signals a thread waiting on this completion */
void release_hisee_complete(void)
{
	complete(&g_hisee_complete);
}

bool flash_otp_task_is_started(void)
{
	if (!g_flash_hisee_otp_task)
		return false;

	if (IS_ERR(g_flash_hisee_otp_task))
		return false;

	return true;
}

void register_flash_hisee_otp_fn(int (*fn_ptr) (void))
{
	if (fn_ptr)
		g_write_hisee_otp_fn = fn_ptr;
}

void creat_flash_otp_thread(void)
{
	if (!g_flash_hisee_otp_task) {
		g_flash_hisee_otp_task = kthread_run(flash_otp_task,
						     NULL, "flash_otp_task");
		if (IS_ERR(g_flash_hisee_otp_task)) {
			g_flash_hisee_otp_task = NULL;
			if (g_efuse_print_level >= LOG_LEVEL_ERROR)
				pr_err("%s:create flash_otp_task failed\n",
				       __func__);
		}
	}
}

