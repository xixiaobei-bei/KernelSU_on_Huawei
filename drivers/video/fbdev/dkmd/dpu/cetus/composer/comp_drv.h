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

#ifndef DPU_COMPOSER_DRV_H
#define DPU_COMPOSER_DRV_H

#include <linux/platform_device.h>
#include <dkmd_dpu.h>

#include "dpu_comp.h"
#include "dkmd_timeline.h"
#include "dkmd_buf_sync.h"
#include "dpu_vsync.h"
#include "dpu_sysfs.h"
#include "dpu_isr.h"

#define COMP_FRAME_MAX_COUNT 3
#define get_frame_idx(idx) ((idx) % COMP_FRAME_MAX_COUNT)

struct composer_match_data {
	int scene_id;
	int (*of_device_setup) (struct platform_device *pdev);
	void (*of_device_release) (struct platform_device *pdev);
};

struct comp_frame {
	struct dkmd_dma_buf layer_dma_buf[DISP_LAYER_MAX_COUNT];
	struct disp_frame in_frame;
};

struct comp_present_data {
	struct comp_frame frames[COMP_FRAME_MAX_COUNT];
	uint32_t using_idx;
	uint32_t displaying_idx;
	uint32_t displayed_idx;

	wait_queue_head_t vactive_start_wq;
	uint32_t vactive_start_flag;
	uint32_t vactive_end_flag;
	uint32_t vactive_listening_bit;
};

struct composer_private {
	struct composer base;
	bool is_fpga;

	char __iomem *dpu_base;
	char __iomem *peri_crg_base;

	struct regulator *dsssubsys_regulator;
	struct regulator *vivobus_regulator;
	struct regulator *media1_subsys_regulator;
	struct regulator *regulator_smmu_tcu;

	struct clk *aclk_gate_dss;
	struct clk *pclk_gate_dss;
	struct clk *clk_gate_edc;

	struct dpu_isr isr_ctrl;
	struct dpu_vsync vsync_ctrl;
	struct dkmd_timeline timeline;
	struct comp_present_data present_data;

	struct platform_device *connector_dev;

	struct dpu_attr attrs;
};

static inline struct composer_private *to_composer_private(struct platform_device *pdev)
{
	struct composer *comp = platform_get_drvdata(pdev);

	return container_of(comp, struct composer_private, base);
}

void comp_mgr_register_composer(struct composer_private *composer);

#endif
