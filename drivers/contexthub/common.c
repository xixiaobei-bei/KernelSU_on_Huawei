/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: Contexthub common driver.
 * Author: Huawei
 * Create: 2017-07-21
 */
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/time.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include "inputhub_api.h"

#ifdef __LLT_UT__
#define STATIC
#else
#define STATIC static
#endif

#define TIMEZONE_DWORK_DELAYTIME (4*HZ)
#define TIMEZONE_MAX_TRY_CNT (7)

struct delayed_work g_timezone_dwork;

int get_contexthub_dts_status(void)
{
	int len = 0;
	struct device_node *node = NULL;
	const char *status = NULL;
	static int ret;
	static int once;

	if (once) {
		pr_info("[%s]status[%d]\n", __func__, ret);
		return ret;
	}

	node = of_find_compatible_node(NULL, NULL, "hisilicon,contexthub_status");
	if (node != NULL) {
		status = of_get_property(node, "status", &len);
		if (status == NULL) {
			pr_err("[%s]of_get_property status err\n", __func__);
			return -EINVAL;
		}

		if (strstr(status, "disabled")) {
			pr_info("[%s][disabled]\n", __func__);
			ret = -EINVAL;
		}
	}

	once = 1;
	pr_info("[%s][enabled]\n", __func__);
	return ret;
}


STATIC int ctxhub_plt_remove(struct platform_device *pdev)
{
	return 0;
}


STATIC int ctxhub_plt_pm_suspend(struct device *dev)
{
	return 0;
}


STATIC void ctxhub_plt_send_timezone(void)
{
	write_info_t winfo;
	int ret;
	int timezone = sys_tz.tz_minuteswest * 60;

	if (timezone == 0) {
		return;
	}

	pr_info("[%s]timezone[%d]\n", __func__,timezone);
	winfo.tag = TAG_SYS;
	winfo.cmd = CMD_SYS_TIMEZONE_REQ;
	winfo.wr_len = sizeof(timezone);
	winfo.wr_buf = &timezone;
	ret = write_customize_cmd(&winfo, NULL, true);
	if (ret) {
		pr_warn("smartplt:[%s]ret[%d]\n", __func__, ret);
	}
}


STATIC void timezone_update_func(struct work_struct *data)
{
	static unsigned int try_cnt = 0;
	pr_devel("smartplt:[%s]try_cnt[%u]tz_minuteswest[%d]\n", __func__,try_cnt,
		sys_tz.tz_minuteswest);
	if (try_cnt < TIMEZONE_MAX_TRY_CNT) {
		if (sys_tz.tz_minuteswest) {
			ctxhub_plt_send_timezone();
			pr_info("smartplt:[%s]try_cnt[%u]tz_minuteswest[%d]\n", __func__,try_cnt,
			sys_tz.tz_minuteswest);
		}else {
			queue_delayed_work(system_freezable_power_efficient_wq, &g_timezone_dwork,TIMEZONE_DWORK_DELAYTIME);
			try_cnt++;
		}
	}
}


STATIC int ctx_fb_notifier(struct notifier_block *nb,
	unsigned long action, void *data)
{
	if (NULL == data)
		return NOTIFY_OK;

	switch (action) {
	case FB_EVENT_BLANK:
	{
		struct fb_event *event = data;
		int *blank = event->data;

		if (registered_fb[0] != event->info) { /* only main screen on/off info send to hub */
			return NOTIFY_OK;
		}

		switch (*blank) {
		case FB_BLANK_UNBLANK: /* screen on */
			ctxhub_plt_send_timezone();
			break;
		default:
			break;
		}
		break;
	}
	default:
		break;
	}

	return NOTIFY_OK;
}


static struct notifier_block ctx_fb_notify = {
	.notifier_call = ctx_fb_notifier,
};


STATIC int ctxhub_plt_probe(struct platform_device *pdev)
{
	int ret = get_contexthub_dts_status();
	if (ret) {
		return ret;
	}

	if (false == of_device_is_available(pdev->dev.of_node)) {
		return -ENODEV;
	}

	INIT_DELAYED_WORK(&g_timezone_dwork,timezone_update_func);
	queue_delayed_work(system_freezable_power_efficient_wq, &g_timezone_dwork, TIMEZONE_DWORK_DELAYTIME);
	fb_register_client(&ctx_fb_notify);
	pr_info("[%s]\n", __func__);
	return 0;
}


STATIC int ctxhub_plt_pm_resume(struct device *dev)
{
	return 0;
}


static const struct of_device_id ctxhub_plt_dev_id[] = {
	{ .compatible = "hisilicon,smart-plt" },
	{},
};
MODULE_DEVICE_TABLE(of, ctxhub_plt_dev_id);


struct dev_pm_ops ctxhub_plt_pm_ops = {
	.suspend = ctxhub_plt_pm_suspend,
	.resume  = ctxhub_plt_pm_resume,
};


static struct platform_driver ctxhub_plt_platdrv = {
	.driver = {
		.name	= "contexthub-platform",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(ctxhub_plt_dev_id),
		.pm = &ctxhub_plt_pm_ops,
	},
	.probe = ctxhub_plt_probe,
	.remove  = ctxhub_plt_remove,
};


STATIC int __init ctxhub_plt_init(void)
{
	return platform_driver_register(&ctxhub_plt_platdrv);
}


STATIC void __exit ctxhub_plt_exit(void)
{
	platform_driver_unregister(&ctxhub_plt_platdrv);
}


late_initcall_sync(ctxhub_plt_init);
module_exit(ctxhub_plt_exit);

