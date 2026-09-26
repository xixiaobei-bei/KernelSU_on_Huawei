/*
 * npu_platform.c
 *
 * about npu platform
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include "npu_platform.h"
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

#include "npu_log.h"
#include "npu_user_common.h"
#include "npu_platform_register.h"
#include "npu_adapter.h"
#include "npu_reg.h"
#include "npu_dfx.h"
#include "npu_resmem.h"
#include "npu_gic.h"
#include "npu_feature.h"
#include "npu_irq.h"
#include "npu_bbit_debugfs.h"
#include "npu_iova_dev.h"

struct npu_mem_desc g_resmem_s_desc[BINARY_IDX_S_CNT];

int npu_plat_init_adapter(struct npu_platform_info *plat_info)
{
	plat_info->adapter.pm_ops.npu_open = npu_plat_pm_open;
	plat_info->adapter.pm_ops.npu_power_up = npu_plat_pm_powerup;
	plat_info->adapter.pm_ops.npu_release = npu_plat_pm_release;
	plat_info->adapter.pm_ops.npu_power_down = npu_plat_pm_powerdown;
	plat_info->adapter.res_ops.npu_irq_change_route = NULL;
	plat_info->adapter.res_ops.npu_cqsq_share_alloc = NULL;
	plat_info->adapter.res_ops.npu_trigger_irq = NULL;
	plat_info->adapter.res_ops.npu_mailbox_send = npu_plat_res_mailbox_send;
	plat_info->adapter.res_ops.npu_ctrl_core = npu_plat_res_ctrl_core;
	return 0;
}

int npu_plat_parse_dtsi(struct platform_device *pdev,
	struct npu_platform_info *plat_info)
{
	struct device_node *root = NULL;
	struct device_node *module_np = NULL;
	int ret;

	root = pdev->dev.of_node;

	module_np = of_get_child_by_name(root, "npu0");

	ret = npu_plat_parse_feature_switch(module_np, plat_info);
	cond_return_error(ret != 0, ret,
		"npu_parse_platform_feature_switch failed\n");

	if (plat_info->dts_info.feature_switch[NPU_FEATURE_GIC_SUPPORT] == 1) {
		ret = npu_plat_parse_gic(module_np, plat_info);
		cond_return_error(ret != 0, ret, "npu_parse_platform_gic failed\n");
	}

	ret = npu_plat_parse_reg_desc(pdev, plat_info);
	cond_return_error(ret != 0, ret, "npu_plat_parse_reg_desc failed\n");
	ret = npu_plat_map_reg(pdev, plat_info);
	cond_return_error(ret != 0, ret, "npu_plat_map_reg failed\n");

	ret = npu_plat_parse_irq(pdev, plat_info);
	cond_goto_error(ret != 0, fail_after_parse_reg, ret, ret, "npu_plat_parse_irq failed\n");

	ret = npu_plat_parse_dump_region_desc(root, plat_info);
	if (ret != 0)
		npu_drv_warn("npu_plat_parse_dump_regs_desc failed\n");

	ret = npu_plat_set_resmem(plat_info);
	cond_goto_error(ret != 0, fail_after_parse_reg, ret, ret, "npu_plat_set_resmem failed\n");

	module_np = of_get_child_by_name(root, "log");
	ret = npu_plat_parse_dfx_desc(module_np, plat_info,
		&plat_info->resmem_info.dfx_desc[NPU_DFX_DEV_LOG],
		NPU_LOG_MEM_IDX);
	cond_goto_error(ret != 0, fail_after_parse_reg, ret, ret,
		"npu_plat_parse_dfx_desc for log failed\n");

	module_np = of_get_child_by_name(root, "profile");
	ret = npu_plat_parse_dfx_desc(module_np, plat_info,
		&plat_info->resmem_info.dfx_desc[NPU_DFX_DEV_PROFILE],
		NPU_PROF_MEM_IDX);
	cond_goto_error(ret != 0, fail_after_parse_reg, ret, ret,
		"npu_plat_parse_dfx_desc for profile failed\n");

	module_np = of_get_child_by_name(root, "blackbox");
	ret = npu_plat_parse_dfx_desc(module_np, plat_info,
		&plat_info->resmem_info.dfx_desc[NPU_DFX_DEV_BLACKBOX],
		NPU_BBOX_MEM_IDX);
	cond_goto_error(ret != 0, fail_after_parse_reg, ret, ret,
		"npu_plat_parse_dfx_desc for blackbox failed\n");

	return 0;
fail_after_parse_reg:
	(void)npu_plat_unmap_reg(pdev, plat_info);
	return ret;
}

int npu_plat_init_spec(struct npu_platform_info *plat_info)
{
	plat_info->spec.stream_max = NPU_MAX_STREAM_ID;
	plat_info->spec.event_max = NPU_MAX_EVENT_ID + NPU_MAX_HWTS_EVENT_ID;
	plat_info->spec.notify_max = NPU_MAX_HWTS_NOTIFY_ID;
	plat_info->spec.model_max = NPU_MAX_MODEL_ID;
	plat_info->spec.aicore_max = NPU_PLAT_AICORE_MAX;
	plat_info->spec.aicpu_max = NPU_PLAT_AICPU_MAX;
	plat_info->spec.calc_sq_max = NPU_MAX_SQ_NUM;
	plat_info->spec.calc_sq_depth = NPU_MAX_SQ_DEPTH;
	plat_info->spec.calc_cq_max = NPU_MAX_CQ_NUM;
	plat_info->spec.calc_cq_depth = NPU_MAX_CQ_DEPTH;
	plat_info->spec.dfx_sq_max = NPU_MAX_DFX_SQ_NUM;
	plat_info->spec.dfx_cq_max = NPU_MAX_DFX_CQ_NUM;
	plat_info->spec.dfx_sq_depth = NPU_MAX_DFX_SQ_DEPTH;
	plat_info->spec.dfx_cq_depth = NPU_MAX_DFX_CQ_DEPTH;
	plat_info->spec.doorbell_stride = NPU_PLAT_DOORBELL_STRIDE;
	return 0;
}

static DEFINE_MUTEX(npu_platform_mutex);

static const struct of_device_id npu_of_match[] = {
	{ .compatible = "hisilicon,npu", },
	{},
};
MODULE_DEVICE_TABLE(of, npu_of_match);

int npu_platform_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

int npu_platform_resume(struct platform_device *pdev)
{
	return 0;
}

struct npu_platform_info *s_platform_info;


void npu_debugfs_init(void)
{

}

int npu_platform_probe(struct platform_device *pdev)
{
	int ret;
	struct npu_platform_info *platform_info = NULL;

	npu_drv_warn("enter\n");
	platform_info = kzalloc(sizeof(struct npu_platform_info), GFP_KERNEL);
	cond_return_error(platform_info == NULL, -ENOMEM,
		"kzalloc plat_info failed\n");

	platform_info->pdev = &pdev->dev;
	platform_info->plat_type = NPU_PLAT_DEVICE;
	platform_info->env_type = NPU_PLAT_TYPE_ASIC;

	ret = npu_plat_parse_dtsi(pdev, platform_info);
	if (ret != 0) {
		npu_drv_err("prase dtsi failed\n");
		goto PROB_FAIL;
	}

	ret = npu_plat_init_adapter(platform_info);
	if (ret != 0) {
		npu_drv_err("npu_plat_init_adapter failed\n");
		goto PROB_FAIL;
	}

	ret = npu_plat_init_spec(platform_info);
	if (ret != 0) {
		npu_drv_err("npu_plat_init_spec failed\n");
		goto PROB_FAIL;
	}

	ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
	if (ret != 0)
		npu_drv_err("mask coherent failed\n");
	npu_debugfs_init();
	npu_drv_warn("npu prob success\n");

	s_platform_info = platform_info;
	return 0;

PROB_FAIL:
	kfree(platform_info);
	platform_info = NULL;
	s_platform_info = NULL;
	return ret;
}

struct npu_platform_info *npu_plat_get_info(void)
{
	return s_platform_info;
}
EXPORT_SYMBOL(npu_plat_get_info);

int npu_platform_remove(struct platform_device *pdev)
{
	struct npu_platform_info *plat_info = NULL;

	npu_drv_debug("npu_device_remove start\n");

	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_info("npu_plat_get_info failed\n");
		return 0;
	}

	npu_plat_unmap_reg(pdev, plat_info);
	npu_drv_info("npu_device_remove succeed\n");
	kfree(plat_info);

	return 0;
}

static int npu_dev_pm_suspend(struct device *dev)
{
	int ret;

	unused(dev);
	ret = npu_plat_dev_pm_suspend();
	if (ret != 0)
		npu_drv_err("npu suspend fail");

	return ret;
}

static int npu_dev_pm_resume(struct device *dev)
{
	unused(dev);
	return 0;
}

static const struct dev_pm_ops npu_dev_pm_ops = {
	.suspend = npu_dev_pm_suspend,
	.resume  = npu_dev_pm_resume,
};

static struct platform_driver npu_platform_driver = {
	.probe = npu_platform_probe,
	.remove = npu_platform_remove,
	.suspend = npu_platform_suspend,
	.resume = npu_platform_resume,
	.driver = {
		.name = "npu_platform",
		.of_match_table = npu_of_match,
		.pm = &npu_dev_pm_ops,
	},
};

static int __init npu_platform_init(void)
{
	int ret;

	if (npu_plat_bypass_status() == NPU_BYPASS)
		return -1;

	npu_drv_warn("started\n");
	ret = platform_driver_register(&npu_platform_driver);
	if (ret) {
		npu_drv_err("insmod devdrv platform driver fail\n");
		return ret;
	}
	npu_drv_warn("succeed\n");

	ret = npu_iova_init();
	if (ret) {
		npu_drv_err("npu_iova_init fail\n");
		platform_driver_unregister(&npu_platform_driver);
		return ret;
	}

	return ret;
}
module_init(npu_platform_init);

static void __exit npu_platform_exit(void)
{
	if (npu_plat_bypass_status() == NPU_BYPASS)
		return;
	platform_driver_unregister(&npu_platform_driver);
	npu_iova_exit();
}
module_exit(npu_platform_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("NPU driver");
MODULE_VERSION("V1.0");
