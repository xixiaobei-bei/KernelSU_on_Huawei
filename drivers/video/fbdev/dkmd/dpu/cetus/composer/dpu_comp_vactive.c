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
#include <linux/delay.h>
#include "comp_drv.h"
#include "dpu_comp_vactive.h"
#include "dkmd_connector.h"
#include "dpu_utils.h"

#define FPGA_VACTIVE_TIMEOUT_MS 10000
#define ASIC_VACTIVE_TIMEOUT_MS 300

static int comp_vactive_isr_notify(struct notifier_block *self, unsigned long action, void *data)
{
	struct comp_present_data *present_data = (struct comp_present_data *)data;

	if (action != present_data->vactive_listening_bit) {
		dpu_pr_info("action = 0x%llx, vactive_listening_bit = 0x%x", action, present_data->vactive_listening_bit);
		return 0;
	}

	dpu_comp_vactive_set_start_flag(&present_data->vactive_start_flag, 1);
	wake_up_interruptible_all(&present_data->vactive_start_wq);

	return 0;
}

static struct notifier_block vactive_isr_notifier = {
	.notifier_call = comp_vactive_isr_notify,
};

void dpu_comp_init_vactive(struct dpu_isr *isr, struct comp_present_data *data, uint32_t vactive_listening_bit)
{
	if (data->vactive_listening_bit != 0)
		return;

	init_waitqueue_head(&data->vactive_start_wq);
	data->vactive_start_flag = 0;
	data->vactive_listening_bit = vactive_listening_bit;

	dpu_isr_register_listener(isr, &vactive_isr_notifier, data->vactive_listening_bit, data);
}

void dpu_comp_vactive_wait_event(struct composer_private *comp)
{
	struct comp_present_data *data = NULL;
	uint32_t timeout = ASIC_VACTIVE_TIMEOUT_MS;
	uint32_t prev_vactive_start_flag;
	struct timeval vactive_tv;
	int ret;
	int times = 0;

	dpu_assert_if_cond(comp == NULL);

	// tracing timestamp
	dpu_trace_ts_begin(&vactive_tv);

	if (comp->is_fpga)
		timeout = FPGA_VACTIVE_TIMEOUT_MS;

	data = &comp->present_data;
	while (times < 50) {
		if (is_mipi_cmd_panel(comp->base.connector_info)) {
			ret = wait_event_interruptible_timeout(data->vactive_start_wq, data->vactive_start_flag, msecs_to_jiffies(timeout));
		} else {
			prev_vactive_start_flag = data->vactive_start_flag;
			ret = wait_event_interruptible_timeout(data->vactive_start_wq, \
					((prev_vactive_start_flag == 0) || (prev_vactive_start_flag != data->vactive_start_flag)), msecs_to_jiffies(timeout));
		}

		if (ret == -ERESTARTSYS) {
			++times;
			mdelay(1);
			continue;
		}

		if (is_mipi_cmd_panel(comp->base.connector_info))
			dpu_comp_vactive_set_start_flag(&data->vactive_start_flag, 0);

		break;
	}

	dpu_trace_ts_end(&vactive_tv, "fb present");
}