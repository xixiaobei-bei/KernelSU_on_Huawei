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
#include "comp_dev.h"
#include "comp_drv.h"

#include "dkmd_release_fence.h"
#include "dkmd_buf_sync.h"
#include "dpu_utils.h"
#include "dpu_cmdlist.h"
#include "dpu_dacc.h"
#include "dkmd_connector.h"
#include "dpu_comp_vactive.h"
#include "dkmd_acquire_fence.h"

int dpu_comp_create_fence(struct platform_device *pdev)
{
	struct composer_private *comp = to_composer_private(pdev);

	if (!comp)
		return -1;

	return 0;
}

int dpu_comp_on(struct platform_device *pdev)
{
	struct composer_private *comp = to_composer_private(pdev);

	if (!comp)
		return -1;

	return 0;
}

int dpu_comp_off(struct platform_device *pdev)
{
	struct composer_private *comp = to_composer_private(pdev);

	if (!comp)
		return -1;

	return 0;
}

static void dpu_comp_start_frame(struct composer_private *comp, struct disp_frame *frame)
{
	dpu_cmdlist_commit(comp->dpu_base, comp->base.scene_id, frame->cmdlist_id);
	dpu_dacc_config_scene(comp->dpu_base, comp->base.scene_id, 1); // enable cmdlist
}

static void dpu_comp_signal_frame_update(struct composer_private *comp)
{
	if (is_dp_panel(comp->base.connector_info)) {
		set_reg(comp->dpu_base + 0x708, 0x1, 1, 10); /* enable dp timingGen */
	} else {
		set_reg(DPU_DSI_LDI_FRM_MSK_UP_ADDR(comp->dpu_base), 0x1, 1, 0);
		set_reg(DPU_DSI_LDI_CTRL_ADDR(comp->dpu_base), 0x1, 1, 0);
	}
}

int dpu_comp_present(struct platform_device *pdev, void *in_frame)
{
	int i;
	struct composer_private *comp = to_composer_private(pdev);
	struct disp_frame *frame = (struct disp_frame *)in_frame;
	struct disp_layer *layer = NULL;
	struct comp_frame *using_frame = dpu_comp_get_using_frame(&comp->present_data);

	/* 1, lock dma buf and wait layer acquired fence */
	for (i = 0; i < frame->layer_count; ++i) {
		layer = &frame->layer[i];

		dkmd_buf_sync_lock_dma_buf(&comp->timeline, &using_frame->layer_dma_buf[i], layer->shard_fd);
		(void)dkmd_acquire_fence_wait(layer->acquired_fence, ACQUIRE_FENCE_TIMEOUT_MSEC);
		dkmd_acquire_fence_release(layer->acquired_fence);
	}

	/* 2, wait vactive isr */
	dpu_comp_vactive_wait_event(comp);

	/* 3, increase timeline step value, normally inc step is 1.
	 *    vsync isr will increase the step with pt_value.
	 */
	dkmd_timeline_inc_step(&comp->timeline);

	/* 4, start this frame for cmdlist and dacc */
	dpu_comp_start_frame(comp, frame);

	/* 5, signal frame update */
	dpu_comp_signal_frame_update(comp);

	return 0;
}
