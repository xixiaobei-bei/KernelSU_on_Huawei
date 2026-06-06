#include <linux/irq.h>
#include <linux/irqchip/arm-gic-v3.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/cpu.h>
#include <linux/kthread.h>
#include <linux/dma-mapping.h>

#include "drv_log.h"
#include "devdrv_user_common.h"
#include "npu_platform_resource.h"
#include "npu_platform_register.h"
#include "npu_adapter.h"

#include "npu_reg.h"
#include "npu_dfx.h"
#include "npu_resmem.h"
#include "npu_gic.h"
#include "npu_feature.h"
#include "npu_irq.h"

int devdrv_plat_init_adapter(
	struct devdrv_platform_info *plat_info)
{
	DEVDRV_PLAT_GET_PM_OPEN(plat_info) = devdrv_plat_pm_open;
	DEVDRV_PLAT_GET_PM_POWER_UP(plat_info) = devdrv_plat_pm_powerup;
	DEVDRV_PLAT_GET_PM_RELEASE(plat_info) = devdrv_plat_pm_release;
	DEVDRV_PLAT_GET_PM_POWER_DOWN(plat_info) = devdrv_plat_pm_powerdown;
	DEVDRV_PLAT_GET_RES_FW_PROC(plat_info) = devdrv_plat_res_fw_proc;
	DEVDRV_PLAT_GET_RES_CHG_ROUTE(plat_info) = NULL;
	DEVDRV_PLAT_GET_RES_SQCQ_ALLOC(plat_info) = NULL;
	DEVDRV_PLAT_GET_RES_TIRG_IRQ(plat_info) = NULL;
	DEVDRV_PLAT_GET_RES_MAILBOX_SEND(plat_info) = devdrv_plat_res_mailbox_send;
	return 0;
}

int devdrv_plat_parse_dtsi(struct platform_device *pdev,
	struct devdrv_platform_info *plat_info)
{
	struct device_node *root = NULL;
	struct device_node *module_np =NULL;
	int ret;

	root = pdev->dev.of_node;
	ret = devdrv_plat_parse_reg_desc(pdev, plat_info);
	if (ret != 0) {
		devdrv_drv_err("devdrv_plat_parse_reg_desc failed.\n");
		return ret;
	}

	ret = devdrv_plat_parse_irq(pdev, plat_info);
	if (ret != 0) {
		devdrv_drv_err("devdrv_plat_parse_irq failed.\n");
		return ret;
	}

	ret = devdrv_plat_parse_resmem_desc(root, plat_info);
	if (ret != 0) {
		devdrv_drv_err("devdrv_plat_parse_resmem_desc failed.\n");
		return ret;
	}

	module_np = of_get_child_by_name(root, "davinci0");
	ret = devdrv_plat_parse_gic(module_np, plat_info);
	if (ret != 0) {
		devdrv_drv_err("devdrv_parse_platform_gic failed.\n");
		return ret;
	}

	ret = devdrv_plat_parse_feature_switch(module_np, plat_info);
	if (ret != 0) {
		devdrv_drv_err("devdrv_parse_platform_feature_switch failed.\n");
		return ret;
	}

	ret = devdrv_plat_parse_resmem_usage(module_np, plat_info);
	if (ret != 0) {
		devdrv_drv_err("devdrv_parse_resmem_usage failed.\n");
		return ret;
	}

	module_np = of_get_child_by_name(root, "log");
	ret = devdrv_plat_parse_dfx_desc(module_np, plat_info,
		&DEVDRV_PLAT_GET_DFX_DESC(plat_info, DEVDRV_DFX_DEV_LOG));
	if (ret != 0) {
		devdrv_drv_err("devdrv_plat_parse_dfx_desc for log failed.\n");
		return ret;
	}

	module_np = of_get_child_by_name(root, "profile");
	ret = devdrv_plat_parse_dfx_desc(module_np, plat_info,
		&DEVDRV_PLAT_GET_DFX_DESC(plat_info, DEVDRV_DFX_DEV_PROFILE));
	if (ret != 0) {
		devdrv_drv_err("devdrv_plat_parse_dfx_desc for profile failed.\n");
		return ret;
	}

	module_np = of_get_child_by_name(root, "blackbox");
	ret = devdrv_plat_parse_dfx_desc(module_np, plat_info,
		&DEVDRV_PLAT_GET_DFX_DESC(plat_info, DEVDRV_DFX_DEV_BLACKBOX));
	if (ret != 0) {
		devdrv_drv_err("devdrv_plat_parse_dfx_desc for blackbox failed.\n");
		return ret;
	}

	return 0;
}

int devdrv_plat_init_spec(struct devdrv_platform_info *plat_info)
{
	DEVDRV_PLAT_GET_STREAM_MAX(plat_info) = DEVDRV_PLAT_STREAM_MAX;
	DEVDRV_PLAT_GET_EVENT_MAX(plat_info) = DEVDRV_PLAT_EVENT_MAX;
	DEVDRV_PLAT_GET_NOTIFY_MAX(plat_info) = DEVDRV_PLAT_NOTIFY_MAX;
	DEVDRV_PLAT_GET_MODEL_MAX(plat_info) = DEVDRV_PLAT_MODEL_MAX;
	DEVDRV_PLAT_GET_AICORE_MAX(plat_info) = DEVDRV_PLAT_AICORE_MAX;
	DEVDRV_PLAT_GET_AICPU_MAX(plat_info) = DEVDRV_PLAT_AICPU_MAX;
	DEVDRV_PLAT_GET_CALC_SQ_MAX(plat_info) = DEVDRV_PLAT_CALC_SQ_MAX;
	DEVDRV_PLAT_GET_CALC_SQ_DEPTH(plat_info) = DEVDRV_PLAT_CALC_SQ_DEPTH;
	DEVDRV_PLAT_GET_CALC_CQ_MAX(plat_info) = DEVDRV_PLAT_CALC_CQ_MAX;
	DEVDRV_PLAT_GET_CALC_CQ_DEPTH(plat_info) = DEVDRV_PLAT_CALC_CQ_DEPTH;
	DEVDRV_PLAT_GET_DFX_SQ_MAX(plat_info) = DEVDRV_PLAT_DFX_SQ_MAX;
	DEVDRV_PLAT_GET_DFX_CQ_MAX(plat_info) = DEVDRV_PLAT_DFX_CQ_MAX;
	DEVDRV_PLAT_GET_DFX_SQCQ_DEPTH(plat_info) = DEVDRV_PLAT_DFX_SQCQ_DEPTH;
	DEVDRV_PLAT_GET_DOORBELL_STRIDE(plat_info) = DEVDRV_PLAT_DOORBELL_STRIDE;
	return 0;
}

static DEFINE_MUTEX(devdrv_platform_mutex);

static const struct of_device_id npu_of_match[] = {
	{ .compatible = "hisi,npu",},
	{},
};
MODULE_DEVICE_TABLE(of, npu_of_match);

int devdrv_platform_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

int devdrv_platform_resume(struct platform_device *pdev)
{
	return 0;
}

static struct devdrv_platform_info *s_platform_info = NULL;

int devdrv_platform_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct devdrv_platform_info *platform_info = NULL;

	platform_info = kzalloc(sizeof(struct devdrv_platform_info), GFP_KERNEL);
	if (platform_info == NULL) {
		devdrv_drv_err("kzalloc plat_info failed.\n");
		return -ENOMEM;
	}

	DEVDRV_PLAT_GET_PDEV(platform_info) = &pdev->dev;
	DEVDRV_PLAT_GET_TYPE(platform_info) = DEVDRV_PLAT_DEVICE;
	DEVDRV_PLAT_GET_ENV(platform_info) = DEVDRV_PLAT_TYPE_ASIC;
	DEVDRV_PLAT_GET_HARDWARE(platform_info) = (u32)SOC_HARDWARE_VERSION;

	ret = devdrv_plat_parse_dtsi(pdev, platform_info);
	if (ret != 0) {
		devdrv_drv_err("prase dtsi failed.\n");
		 goto PROB_FAIL;
	}

	ret = devdrv_plat_init_adapter(platform_info);
	if (ret != 0) {
		devdrv_drv_err("devdrv_plat_init_adapter failed.\n");
		goto PROB_FAIL;
	}

	ret = devdrv_plat_init_spec(platform_info);
	if (ret != 0) {
		devdrv_drv_err("devdrv_plat_init_spec failed.\n");
		goto PROB_FAIL;
	}

	ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64)); //lint !e598 !e648
	if (ret != 0) {
		devdrv_drv_err("mask coherent failed.\n");
	}

	s_platform_info = platform_info;
	return 0;

PROB_FAIL:
    kfree(platform_info);
    platform_info = NULL;
    return ret;
}

struct devdrv_platform_info* devdrv_plat_get_info(void)
{
	return s_platform_info;
}
EXPORT_SYMBOL(devdrv_plat_get_info);

int devdrv_platform_remove(struct platform_device *pdev)
{
	struct devdrv_platform_info *plat_info = NULL;

	devdrv_drv_debug("devdrv_device_remove start.\n");

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_info("devdrv_plat_get_info failed.\n");
		return 0;
	}

	devdrv_plat_unmap_reg(pdev, plat_info);

	devdrv_drv_info("devdrv_device_remove succeed.\n");

	kfree(plat_info);

	return 0;
}

static struct platform_driver devdrv_platform_driver = {
	.probe = devdrv_platform_probe,
	.remove = devdrv_platform_remove,
	.suspend = devdrv_platform_suspend,
	.resume = devdrv_platform_resume,
	.driver = {
		.name = "npu_platform",
		.of_match_table = npu_of_match,
	},
};

static int __init devdrv_platform_init(void)
{
	int ret;

	devdrv_drv_debug("devdrv_platform_init started\n");
	ret = platform_driver_register(&devdrv_platform_driver);
	if (ret) {
		devdrv_drv_err("insmod devdrv platform driver fail\n");
		return ret;
	}
	devdrv_drv_debug("devdrv_platform_init succeed\n");

	return ret;
}
module_init(devdrv_platform_init);

static void __exit devdrv_platform_exit(void)
{
	platform_driver_unregister(&devdrv_platform_driver);
}
module_exit(devdrv_platform_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("DAVINCI driver");
MODULE_VERSION("V1.0");
