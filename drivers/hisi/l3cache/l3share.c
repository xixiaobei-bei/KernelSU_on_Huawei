#define pr_fmt(fmt) "l3share: " fmt

#include <asm/compiler.h>
#include <linux/compiler.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <soc_sctrl_interface.h>
#include <soc_acpu_baseaddr_interface.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <global_ddr_map.h>
#include <linux/perf_event.h>
#include <linux/slab.h>
#include <linux/ktime.h>
#include <linux/delay.h>

#define CREATE_TRACE_POINTS
#include <trace/events/l3_share.h>

#include "l3_common.h"

#define MAX_GROUP_NUM			4


bool req_pending_flag = false;
bool acp_enable_flag = false;
bool acp_first_flag = true;
DEFINE_MUTEX(l3c_acp_lock);



static BLOCKING_NOTIFIER_HEAD(l3c_chain_head);

int register_l3c_acp_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&l3c_chain_head, nb);
}

int unregister_l3c_acp_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&l3c_chain_head, nb);
}

int __l3c_notifier_call_chain(unsigned long val, int nr_to_call, int *nr_calls, void *v)
{
	int ret;

	ret = __blocking_notifier_call_chain(&l3c_chain_head, val, v,
						nr_to_call, nr_calls);

	return notifier_to_errno(ret);
}
int l3c_notifier_call_chain(unsigned long val ,void *v)
{
	return __l3c_notifier_call_chain(val, -1, NULL, v);
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
		"smc	#0\n"
		: "+r" (function_id)
		: "r" (arg0), "r" (arg1), "r" (arg2));

	return (int)function_id;
}


static void set_l3_share_flag(u32 power_value)
{
	writel(power_value, ioremap(SOC_SCTRL_SCBAKDATA9_ADDR(SOC_ACPU_SCTRL_BASE_ADDR), 0x4));
}

void l3_cache_enable_acp(unsigned int id, unsigned int size)
{
	atfd_l3share_smc((u64)L3_SHARE_SVC, (u64)CMD_ENABLE_ACP, (u64)id, (u64)size);

}

void l3_cache_disable_acp(unsigned int id)
{
	atfd_l3share_smc((u64)L3_SHARE_SVC, (u64)CMD_DISABLE_ACP, (u64)id, (u64)0);
}







DEFINE_MUTEX(l3c_share_lock);
static u32 g_acp_flag;

int l3_cache_request(struct l3_cache_request_params *request_params)
{
	int ret = 0;

	if (NULL == request_params){
		pr_err("%s request_params is NULL\n", __func__);
		return -EINVAL;
	}

	if (request_params->request_size <= 0 || request_params->request_size >= MAX_GROUP_NUM){
		pr_err("%s size must be 1<= size <=3\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&l3c_share_lock);
	trace_l3_cache_request_enter(request_params->id, request_params->request_size, g_acp_flag);
	if (g_acp_flag){
		pr_err("%s permit only one module can request L3 at the same time \n", __func__);
		ret = -EPERM;
		goto unlock;
	}

	if (0 == g_acp_flag){
		mutex_lock(&l3c_acp_lock);
		req_pending_flag = true;
		acp_enable_flag = false;
		g_acp_flag |= BIT(request_params->id);

		/* ASI default status is enabled */
		if (true == acp_first_flag){
			acp_first_flag = false;
			l3_cache_disable_acp(request_params->id);
			pr_debug("%s disable acp when first request pending \n", __func__);
		}
		mutex_unlock(&l3c_acp_lock);

		l3c_notifier_call_chain(L3C_ACP_PENDING, request_params);
		pr_debug("%s pending request id %d, g_acp_flag = 0x%x\n", __func__, request_params->id, g_acp_flag);
		trace_l3_cache_share_perf("acp pending", request_params->id, g_acp_flag);

		goto unlock;
	}

	g_acp_flag |= BIT(request_params->id);

	pr_debug("%s succ request id %d, g_acp_flag = 0x%x\n", __func__, request_params->id, g_acp_flag);
	trace_l3_cache_request_succ(request_params->id, g_acp_flag);

unlock:
	mutex_unlock(&l3c_share_lock);
	return ret;
}


int l3_cache_release(struct l3_cache_release_params *release_params)
{
	int ret = 0;

	if (NULL == release_params){
		pr_err("%s release_params is NULL\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&l3c_share_lock);
	trace_l3_cache_release_enter(release_params->id, g_acp_flag);
	if (0 == (g_acp_flag & BIT(release_params->id))){
		pr_err("%s  moduleid %d not request\n", __func__, release_params->id);
		ret = -EINVAL;
		goto unlock;
	}

	g_acp_flag &= (~BIT(release_params->id));
	if (0 == g_acp_flag){
		mutex_lock(&l3c_acp_lock);
		if (true == acp_enable_flag){
			acp_enable_flag = false;
			l3_cache_disable_acp(release_params->id);
			pr_debug("%s disable acp id %d, g_acp_flag = 0x%x\n", __func__, release_params->id, g_acp_flag);
			trace_l3_cache_share_perf("disable acp", release_params->id, g_acp_flag);
		}
		req_pending_flag = false;
		mutex_unlock(&l3c_acp_lock);

		l3c_notifier_call_chain(L3C_ACP_RELEASE, release_params);
	}
	pr_debug("%s succ release id %d, g_acp_flag = 0x%x\n", __func__, release_params->id, g_acp_flag);
	trace_l3_cache_release_succ(release_params->id, g_acp_flag);

unlock:
	mutex_unlock(&l3c_share_lock);
	return ret;
}


static int l3_share_probe(struct platform_device *pdev)
{
	int ret = 0;

	pr_err("%s enter\n", __func__);

	set_l3_share_flag(0x0);


	pr_err("%s probe sucess!\n", __func__);
	return ret;
}


static int l3_share_remove(struct platform_device *pdev)
{


	return 0;
}

static int l3_share_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int l3_share_resume(struct platform_device *pdev)
{
	return 0;
}


#define MODULE_NAME		"hisilicon,l3cache"
static const struct of_device_id l3_share_match[] = {
	{ .compatible = MODULE_NAME },
	{},
};

static struct platform_driver l3_share_drv = {
	.probe		= l3_share_probe,
	.remove		= l3_share_remove,
	.suspend	= l3_share_suspend,
	.resume		= l3_share_resume,
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
