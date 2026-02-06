/*
 * l3cache_share.c
 *
 * L3cache Share Driver
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
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
#define pr_fmt(fmt) "l3share: " fmt
#include <asm/compiler.h>
#include <linux/compiler.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <global_ddr_map.h>
#include <linux/perf_event.h>
#include <linux/slab.h>
#include <linux/ktime.h>
#include <linux/delay.h>
#include <securec.h>

#define CREATE_TRACE_POINTS
#include <trace/events/l3cache_share.h>
#include "l3cache_common.h"

#define MAX_GROUP_NUM			4

DEFINE_MUTEX(g_l3c_share_lock);
static u32 g_acp_flag;

#ifdef CONFIG_L3CACHE_SHARE_PERF
bool g_req_pending_flag;
bool g_acp_enable_flag;
bool g_acp_first_flag = true;
DEFINE_MUTEX(g_l3c_acp_lock);
#endif

static BLOCKING_NOTIFIER_HEAD(l3share_chain_head);

int register_l3share_acp_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&l3share_chain_head, nb);
}

int unregister_l3share_acp_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&l3share_chain_head, nb);
}

static int __l3share_notifier_call_chain(unsigned long val,
					 int nr_to_call, int *nr_calls, void *v)
{
	int ret;

	ret = __blocking_notifier_call_chain(&l3share_chain_head, val, v,
					     nr_to_call, nr_calls);

	return notifier_to_errno(ret);
}

static int l3share_notifier_call_chain(unsigned long val, void *v)
{
	return __l3share_notifier_call_chain(val, -1, NULL, v);
}

noinline int atfd_l3share_smc(u64 _function_id, u64 _arg0, u64 _arg1, u64 _arg2)
{
	register u64 function_id asm("x0") = _function_id;
	register u64 arg0 asm("x1") = _arg0;
	register u64 arg1 asm("x2") = _arg1;
	register u64 arg2 asm("x3") = _arg2;

	asm volatile (
		__asmeq("%0", "x0")
		__asmeq("%1", "x1")
		__asmeq("%2", "x2")
		__asmeq("%3", "x3")
		"smc #0\n"
		: "+r" (function_id)
		: "r" (arg0), "r" (arg1), "r" (arg2));

	return (int)function_id;
}

void l3_cache_enable_acp(unsigned int id, unsigned int size)
{
	atfd_l3share_smc((u64)L3_SHARE_SVC, (u64)CMD_ENABLE_ACP,
			 (u64)id, (u64)size);
}

void l3_cache_disable_acp(unsigned int id)
{
	atfd_l3share_smc((u64)L3_SHARE_SVC, (u64)CMD_DISABLE_ACP,
			 (u64)id, (u64)0);
}

static unsigned int g_l3share_enable_flag;


int l3_cache_request(struct l3_cache_request_params *request_params)
{
	int ret = 0;

	if (request_params == NULL) {
		pr_err("%s request_params is NULL\n", __func__);
		return -EINVAL;
	}

	if (request_params->request_size <= 0 ||
	    request_params->request_size > MAX_GROUP_NUM) {
		pr_err("%s size must be 1<= size <=4\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&g_l3c_share_lock);
	if (g_l3share_enable_flag == 0) {
		pr_err("%s l3share not enabled\n", __func__);
		ret = -EPERM;
		goto unlock;
	}
	pr_debug("%s enter request id %d, size = %d, g_acp_flag = 0x%x\n",
		 __func__, request_params->id, request_params->request_size,
		 g_acp_flag);
	trace_l3_cache_request_enter(request_params->id,
				     request_params->request_size, g_acp_flag);
	if (g_acp_flag != 0) {
		pr_err("%s permit only one module can request L3 at the same time\n",
		       __func__);
		ret = -EPERM;
		goto unlock;
	}

	if (g_acp_flag == 0) {

#ifdef CONFIG_L3CACHE_SHARE_PERF
		mutex_lock(&g_l3c_acp_lock);
		g_req_pending_flag = true;
		g_acp_enable_flag = false;
		g_acp_flag |= BIT((u32)(request_params->id));

		/* ASI default status is enabled */
		if (g_acp_first_flag) {
			g_acp_first_flag = false;
			l3_cache_disable_acp(request_params->id);
			pr_debug("%s disable acp when first request pending\n",
				 __func__);
		}
		mutex_unlock(&g_l3c_acp_lock);

		l3share_notifier_call_chain(L3SHARE_MON_START, request_params);
		pr_debug("%s pending request id %d, g_acp_flag = 0x%x\n",
			 __func__, request_params->id, g_acp_flag);
		trace_l3_cache_share_perf("acp pending",
					  request_params->id, g_acp_flag);

		goto unlock;
#else
		l3_cache_enable_acp(request_params->id,
				    request_params->request_size);
#endif
	}

	g_acp_flag |= BIT((u32)(request_params->id));

	pr_debug("%s succ request id %d, g_acp_flag = 0x%x\n",
		 __func__, request_params->id, g_acp_flag);
	trace_l3_cache_request_succ(request_params->id, g_acp_flag);

unlock:
	mutex_unlock(&g_l3c_share_lock);
	return ret;
}

int l3_cache_release(struct l3_cache_release_params *release_params)
{
	int ret = 0;

	if (release_params == NULL) {
		pr_err("%s release_params is NULL\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&g_l3c_share_lock);
	pr_debug("%s enter release id %d, g_acp_flag = 0x%x\n",
		 __func__, release_params->id,  g_acp_flag);
	trace_l3_cache_release_enter(release_params->id, g_acp_flag);
	if ((g_acp_flag & BIT((u32)(release_params->id))) == 0) {
		pr_err("%s  moduleid %d not request\n", __func__,
		       release_params->id);
		ret = -EINVAL;
		goto unlock;
	}

	g_acp_flag &= (~BIT((u32)(release_params->id)));
	if (g_acp_flag == 0) {
#ifdef CONFIG_L3CACHE_SHARE_PERF
		mutex_lock(&g_l3c_acp_lock);
		if (g_acp_enable_flag) {
			g_acp_enable_flag = false;
			l3_cache_disable_acp(release_params->id);
			pr_debug("%s disable acp id %d, g_acp_flag = 0x%x\n",
				 __func__, release_params->id, g_acp_flag);
			trace_l3_cache_share_perf("disable acp",
						  release_params->id,
						  g_acp_flag);
		}
		g_req_pending_flag = false;
		mutex_unlock(&g_l3c_acp_lock);

		l3share_notifier_call_chain(L3SHARE_MON_STOP, release_params);
#else
		l3_cache_disable_acp(release_params->id);
#endif

	}
	pr_debug("%s succ release id %d, g_acp_flag = 0x%x\n",
		 __func__, release_params->id, g_acp_flag);
	trace_l3_cache_release_succ(release_params->id, g_acp_flag);

unlock:
	mutex_unlock(&g_l3c_share_lock);
	return ret;
}

static int l3_share_probe(struct platform_device *pdev)
{
	int ret = 0;

	pr_err("%s enter\n", __func__);

	g_l3share_enable_flag = 1;


	pr_err("%s probe success!\n", __func__);
	return ret;
}

static int l3_share_remove(struct platform_device *pdev)
{

	return 0;
}

#define MODULE_NAME		"hisi,l3share"
static const struct of_device_id l3_share_match[] = {
	{ .compatible = MODULE_NAME },
	{},
};

static struct platform_driver l3_share_drv = {
	.probe		= l3_share_probe,
	.remove		= l3_share_remove,
	.driver = {
		.name	= MODULE_NAME,
		.of_match_table = of_match_ptr(l3_share_match),
	},
};

static int __init l3_share_init(void)
{
	return platform_driver_register(&l3_share_drv);
}

static void __exit l3_share_exit(void)
{
	platform_driver_unregister(&l3_share_drv);
}

module_init(l3_share_init);
module_exit(l3_share_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Hisi L3share Driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
