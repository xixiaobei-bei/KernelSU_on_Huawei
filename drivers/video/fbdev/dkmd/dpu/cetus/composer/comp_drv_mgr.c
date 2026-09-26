/**
 * @file
 * Copyright (c) 2021-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <dpu/soc_dpu_define.h>

#include "comp_drv.h"
#include "dkmd_connector.h"
#include "dpu_utils.h"
#include "gfxdev_mgr.h"
#include "dpu_comp_vactive.h"

#define COMP_MGR_MAX_COUNT 7
static struct composer_private *g_comp_mgr[COMP_MGR_MAX_COUNT];

static struct composer_private *_get_composer(uint32_t scene_id)
{
	if (scene_id >= COMP_MGR_MAX_COUNT) {
		dpu_pr_err("scene_id %u is over count %u", scene_id, COMP_MGR_MAX_COUNT);
		return NULL;
	}

	return g_comp_mgr[scene_id];
}

static inline bool is_offline_scene(uint32_t scene_id)
{
	return (scene_id >= DPU_SCENE_OFFLINE_0 && scene_id <= DPU_SCENE_OFFLINE_2);
}

static inline bool is_online_scene(uint32_t scene_id)
{
	return (scene_id >= DPU_SCENE_ONLINE_0 && scene_id <= DPU_SCENE_ONLINE_3);
}

void comp_mgr_register_composer(struct composer_private *comp)
{
	if (!comp)
		return;

	if (comp->base.scene_id >= COMP_MGR_MAX_COUNT) {
		dpu_pr_err("scene_id %u is over count %u", comp->base.scene_id, COMP_MGR_MAX_COUNT);
		return;
	}

	g_comp_mgr[comp->base.scene_id] = comp;
}

static void comp_mgr_bind_connector(uint32_t scene_id, struct dkmd_connector_info *pinfo)
{
	struct composer_private *comp = _get_composer(scene_id);
	uint32_t vsync_bit;

	if (!comp) {
		dpu_pr_err("get composer fail, scene_id = %u", scene_id);
		return;
	}
	vsync_bit = is_mipi_cmd_panel(pinfo) ? DSI_INT_LCD_TE0 : DSI_INT_VSYNC;

	// if composer timeline isn't initialized
	if (comp->timeline.listening_isr_bit == 0) {
		dkmd_timline_init(&comp->timeline, "online_composer", comp, vsync_bit);
		dpu_isr_register_listener(&comp->isr_ctrl, comp->timeline.notifier, comp->timeline.listening_isr_bit, &comp->timeline);
	}

	// register vsync event to vsync isr
	if (comp->vsync_ctrl.listening_isr_bit == 0 && comp->vsync_ctrl.is_supported) {
		dpu_vsync_init(&comp->vsync_ctrl, &comp->attrs, vsync_bit);
		dpu_isr_register_listener(&comp->isr_ctrl, comp->vsync_ctrl.notifier, comp->vsync_ctrl.listening_isr_bit, &comp->vsync_ctrl);
	}

	dpu_comp_init_vactive(&comp->isr_ctrl, &comp->present_data, is_dp_panel(pinfo) ?  0 : DSI_INT_VACT0_START);

	dpu_sysfs_create(comp->base.pdev, &comp->attrs);
}

void comp_mgr_register_device(struct platform_device *pdev)
{
	uint32_t scene_id;
	struct dkmd_connector_info *pinfo = NULL;
	struct composer_private *comp = NULL;

	pinfo = platform_get_drvdata(pdev);
	scene_id = pinfo->scene_id;
	if (scene_id >= COMP_MGR_MAX_COUNT) {
		dpu_pr_err("scene_id = %u is over flow", scene_id);
		return;
	}

	comp = _get_composer(scene_id);
	if (!comp) {
		dpu_pr_err("composer driver scene_id = %u is not ready", scene_id);
		return;
	}

	/* add composer private data to device for ioctl */
	platform_device_add_data(comp->base.pdev, comp, sizeof(*comp));

	/* save connector device info */
	comp->connector_dev = pdev;
	comp->is_fpga = !!pinfo->fpga_flag;
	comp->base.connector_info = pinfo;
	comp_mgr_bind_connector(scene_id, pinfo);

	if (is_online_scene(scene_id))
		device_mgr_create_gfxdev(comp->base.pdev);
	else
		device_mgr_create_chrdev(comp->base.pdev);
}
EXPORT_SYMBOL(comp_mgr_register_device);

void comp_mgr_unregister_device(struct platform_device *pdev)
{
	uint32_t scene_id;
	struct dkmd_connector_info *pinfo = NULL;
	struct composer_private *comp = NULL;

	pinfo = platform_get_drvdata(pdev);
	scene_id = pinfo->scene_id;
	if (scene_id >= COMP_MGR_MAX_COUNT) {
		dpu_pr_err("scene_id = %u is over flow", scene_id);
		return;
	}

	comp = _get_composer(scene_id);
	if (!comp) {
		dpu_pr_err("comp scene_id: %u is null!", scene_id);
		return;
	}

	if (is_online_scene(scene_id))
		device_mgr_destroy_gfxdev(comp->base.pdev);
	else
		device_mgr_destroy_chrdev(comp->base.pdev);
}
EXPORT_SYMBOL(comp_mgr_unregister_device);

MODULE_LICENSE("GPL");
