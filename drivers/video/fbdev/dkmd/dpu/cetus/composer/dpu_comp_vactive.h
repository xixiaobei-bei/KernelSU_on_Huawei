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
#ifndef DPU_COMP_VACTIVE_H
#define DPU_COMP_VACTIVE_H

#include "dpu_isr.h"

static inline void dpu_comp_vactive_set_start_flag(uint32_t *start_flag, uint32_t val)
{
	*start_flag += val;
}

void dpu_comp_init_vactive(struct dpu_isr *isr, struct comp_present_data *data, uint32_t vactive_listening_bit);
void dpu_comp_vactive_wait_event(struct composer_private *comp);

#endif