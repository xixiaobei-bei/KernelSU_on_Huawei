/* Copyright (c) 2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __CORE_INTERFACE_H__
#define __CORE_INTERFACE_H__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include "hisi_dp.h"

void dptx_ctrl_layer_init(struct dp_ctrl *dptx);

/**
 * mst
 */
void dptx_initiate_mst_act(struct dp_ctrl *dptx);
void dptx_clear_vcpid_table(struct dp_ctrl *dptx);
void dptx_set_vcpid_table_slot(struct dp_ctrl *dptx, uint32_t slot, uint32_t stream);
void dptx_mst_enable(struct dp_ctrl *dptx);

#endif
