/*
 * mmc debug function definition
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef DRIVER_MMC_HOST_HISI_MMC_DEBUG_H
#define DRIVER_MMC_HOST_HISI_MMC_DEBUG_H

#ifdef CONFIG_MMC_CQ_HCI
void cmdq_clear_dcmd_debug_flag(void);
void cmdq_dcmd_wait(void);
u32 cmdq_trigger_irq_err(u32 err);
u32 cmdq_trigger_irq_resend(u32 err_info, u32 mask);
u32 cmdq_trigger_resend(u32 val, u32 mask);
int cmdq_halt_trigger_reset(struct cmdq_host *cq_host);
int cmdq_retry_trigger_reset(struct cmdq_host *cq_host);
int cmdq_clear_task_trigger_reset(struct cmdq_host *cq_host);
#endif
#endif
