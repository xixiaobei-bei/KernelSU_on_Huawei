/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2021. All rights reserved.
 * Description: hhee exception driver source file
 * Create: 2016/12/6
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/kthread.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/util.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <linux/compiler.h>
#include <linux/interrupt.h>
#include <asm/compiler.h>
#include <linux/debugfs.h>
#include "hhee.h"
#include "hhee_msg.h"

struct rdr_exception_info_s hhee_excetption_info[] = {
	{
		.e_modid            = MODID_AP_S_HHEE_PANIC,
		.e_modid_end        = MODID_AP_S_HHEE_PANIC,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority  = RDR_REBOOT_NOW,
		.e_notify_core_mask = RDR_AP,
		.e_reset_core_mask  = RDR_AP,
		.e_from_core        = RDR_AP,
		.e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type        = AP_S_HHEE_PANIC,
		.e_upload_flag      = (u32)RDR_UPLOAD_YES,
		.e_from_module      = "RDR HHEE PANIC",
		.e_desc             = "RDR HHEE PANIC"
	}/*lint !e785*/
};

int hhee_panic_handle(unsigned int len, void *buf)
{
	pr_err("hhee panic trigger system_error\n");
	(void)len;
	if (buf != NULL) {
		pr_err("El2 trigger panic, buf should be null\n");
	}

	rdr_syserr_process_for_ap((u32)MODID_AP_S_HHEE_PANIC, 0ULL, 0ULL);
	return 0;
}

/* check hhee enable */
static int g_hhee_enable = HHEE_ENABLE;
int hhee_check_enable(void)
{
	return g_hhee_enable;
}
EXPORT_SYMBOL(hhee_check_enable);

#define CPU_MASK         0xF
static void reset_hhee_irq_counters(void)
{
	struct arm_smccc_res res;
	arm_smccc_hvc((unsigned long)HHEE_MONITORLOG_RESET_COUNTERS,
		0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, &res);
}

void hkip_clean_counters(void)
{
	reset_hkip_irq_counters();
	reset_hhee_irq_counters();
}

static int hhee_irq_get(struct platform_device *pdev)
{
	int ret;
	int irq;
	struct device *dev = &pdev->dev;

	/* irq num get and register */
	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(dev, "get irq failed\n");
		return -ENXIO;
	}
	ret = devm_request_irq(dev, (unsigned int)irq,
			       hhee_irq_handle, 0ul, "hkip-hhee", pdev);
	if (ret < 0) {
		dev_err(dev, "devm request irq failed\n");
		return -EINVAL;
	}
	if(cpu_online(CPU_MASK) && (irq_set_affinity(irq, cpumask_of(CPU_MASK)) < 0))
		dev_err(dev, "set affinity failed\n");

	return 0;
}

static int hkip_hhee_probe(struct platform_device *pdev)
{
	int ret;
	unsigned int read_cnt = 0;
	struct device *dev = &pdev->dev;

	if (HHEE_DISABLE == hhee_check_enable())
		return 0;

	ret = hhee_irq_get(pdev);
	if (ret < 0)
		goto err_free_hhee;
	/* rdr struct register */
	ret = (s32)rdr_register_exception(&hhee_excetption_info[0]);
	if (!ret)
		dev_err(dev, "register hhee exception fail\n");

	hhee_module_init();

	ret = hhee_logger_init();
	if (ret < 0) {
		ret = -EINVAL;
		goto err_free_hhee;
	}


	ret = hhee_msg_init();
	if (ret)
		goto err_free_hhee;

	ret = hhee_msg_register_handler(HHEE_MSG_ID_CRASH, hhee_panic_handle);
	if (ret)
		goto err_free_hhee;

	/*
	 * Ignore the return value
	 * HHEE using 0 as default instead of
	 */
	ret = of_property_read_u32_index(pdev->dev.of_node, "hhee_read_cnt",
					 0, &read_cnt);
	if (ret) {
		dev_err(dev, "cannot get hhee_read_cnt\n");
		return -EINVAL;
	}

	(void)hhee_fn_hvc((unsigned long)HHEE_HVC_PAGERD_COUNT, read_cnt, 0ul, 0ul);

	pr_info("hhee probe done\n");
	return 0;

err_free_hhee:
	return ret;
}

static int hkip_hhee_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id hkip_hhee_of_match[] = {
	{.compatible = "hkip,hkip-hhee"},
	{},
};

static struct platform_driver hkip_hhee_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "hkip-hhee",
		.of_match_table = of_match_ptr(hkip_hhee_of_match),
	},
	.probe = hkip_hhee_probe,
	.remove = hkip_hhee_remove,
};

static int __init hkip_hhee_cmd_get(char *arg)
{
	if (!arg)
		return -EINVAL;

	if (!strncmp(arg, "false", strlen("false"))){
		g_hhee_enable = HHEE_DISABLE;
	}

	pr_err("hhee enable = %d.\n", g_hhee_enable);
	return 0;
}
early_param("hhee_enable", hkip_hhee_cmd_get);

static int __init hkip_hhee_device_init(void)
{
	int ret;

	pr_info("hhee probe init\n");
	ret = platform_driver_register(&hkip_hhee_driver);
	if (ret)
		pr_err("register hhee driver fail\n");

	return ret;
}

static void __exit hkip_hhee_device_exit(void)
{
	platform_driver_unregister(&hkip_hhee_driver);
}

module_init(hkip_hhee_device_init);
module_exit(hkip_hhee_device_exit);

MODULE_DESCRIPTION("hhee driver");
MODULE_ALIAS("hhee module");
MODULE_LICENSE("GPL");
