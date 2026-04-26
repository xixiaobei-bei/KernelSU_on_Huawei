/*
 * da_combine_mad.h
 *
 * da_combine mad driver
 *
 * Copyright (c) 2019-2020 Huawei Technologies CO., Ltd.
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

#ifndef __DA_COMBINE_MAD_H__
#define __DA_COMBINE_MAD_H__

#include <linux/hisi/da_combine/da_combine_irq.h>

void da_combine_hook_pcm_handle(void);
int da_combine_mad_init(struct da_combine_irq *irq);
void da_combine_mad_request_irq(void);
void da_combine_mad_free_irq(void);
void da_combine_mad_deinit(void);

#endif /* __DA_COMBINE_MAD_H__ */

