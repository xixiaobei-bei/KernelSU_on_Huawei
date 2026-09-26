/*
 * bt_dev.h
 *
 * api for bt data parse
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef BT_DEV_H
#define BT_DEV_H

#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/workqueue.h>

#include "bt_common.h"
#include "bt_type.h"
#include "bt_dev.h"

int32 bt_dev_log_level(void);
void data_str_printf(int len, const uint8 *ptr);
void bt_get_core_reference(struct bt_core_s **core_data);
#endif
