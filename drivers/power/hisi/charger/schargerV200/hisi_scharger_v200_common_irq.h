/*
 * hisi_scharger_v200_common_irq.h
 *
 * HI6522 charger driver common config and irq handler header
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#ifndef _LINUX_HI6522_CHARGER_IRQ_H
#define _LINUX_HI6522_CHARGER_IRQ_H
#include <linux/workqueue.h>
void hi6522_irq_work_handle(struct work_struct *work);
struct hi6522_device_info *get_hi6522_dev(void);
int hi6522_read_block(struct hi6522_device_info *di, u8 *value, u8 reg,
				unsigned int num_bytes);
int hi6522_write_byte(u8 reg, u8 value);
int hi6522_read_byte(u8 reg, u8 *value);
int hi6522_write_mask(u8 reg, u8 mask, u8 shift, u8 value);
int hi6522_read_mask(u8 reg, u8 mask, u8 shift, u8 *value);
#endif
