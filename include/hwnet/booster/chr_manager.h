/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: This file is the internal header file for chr manager
 * Author: yaozhendong1@huawei.com
 * Create: 2019-12-10
 */

#ifndef _CHR_MANAGER_H
#define _CHR_MANAGER_H

#include <linux/jiffies.h>
#include <linux/timer.h>

#include "netlink_handle.h"

void chr_manager_init(notify_event *notify);

#endif