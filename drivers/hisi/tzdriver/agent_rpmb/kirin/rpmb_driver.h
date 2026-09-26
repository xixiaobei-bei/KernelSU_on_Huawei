/*
 * rpmb_driver.h
 *
 * function declarment for rpmb
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
 */

#ifndef __RPMB_DRIVER_H
#define __RPMB_DRIVER_H

#include <linux/hisi/rpmb.h>

#ifdef CONFIG_VENDOR_RPMB
int vendor_rpmb_ioctl_cmd(enum func_id id, enum rpmb_op_type operation,
	struct storage_blk_ioc_rpmb_data *storage_data);

#else
static inline int
vendor_rpmb_ioctl_cmd(enum func_id id, enum rpmb_op_type operation,
		    struct storage_blk_ioc_rpmb_data *storage_data)
{
	return 0xFF08;
}
#endif

#if defined(CONFIG_VENDOR_RPMB) && !defined(CONFIG_RPMB_REQ_LOCK_DISABLE)
static inline void rpmb_driver_counter_lock(void)
{
	mutex_lock(&rpmb_counter_lock);
}

static inline void rpmb_driver_counter_unlock(void)
{
	mutex_unlock(&rpmb_counter_lock);
}
#else
static inline void rpmb_driver_counter_lock(void)
{
}

static inline void rpmb_driver_counter_unlock(void)
{
}
#endif

int rpmb_ioctl_cmd(enum func_id id, enum rpmb_op_type operation,
	struct storage_blk_ioc_rpmb_data *storage_data);
#endif
