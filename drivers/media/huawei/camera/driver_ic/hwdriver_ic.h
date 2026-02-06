/*
 * hwdriver.h
 *
 * camera driver source file
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#ifndef __HW_ALAN_KERNEL_DRIVERIC_INTERFACE_H__
#define __HW_ALAN_KERNEL_DRIVERIC_INTERFACE_H__

#include <linux/videodev2.h>
#include <media/huawei/camera.h>
#include <media/huawei/driver_ic_cfg.h>
#include <media/videobuf2-core.h>

#include "hwcam_intf.h"

struct hwdriveric_notify_vtbl_t {
	void (*error)(struct hwdriveric_notify_intf_t *i,
		hwdriveric_event_t *driveric_ev);
};

struct hwdriveric_notify_intf_t {
	struct hwdriveric_notify_vtbl_t *vtbl;
};

struct hwdriveric_vtbl_t {
	char const* (*get_name)(const struct hwdriveric_intf_t *i);
	int (*power_on)(const struct hwdriveric_intf_t *i);
	int (*power_off)(const struct hwdriveric_intf_t *i);
	int (*driveric_get_dt_data)(const struct hwdriveric_intf_t *,
		struct device_node *);
	int (*driveric_register_attribute)(const struct hwdriveric_intf_t *,
		struct device *);
	int (*init)(const struct hwdriveric_intf_t *i);
};

struct hwdriveric_intf_t {
	struct hwdriveric_vtbl_t *vtbl;
};

struct driveric_t {
	struct hwdriveric_intf_t         intf;
	char                             name[HWDRIVERIC_NAME_SIZE];
	int                              position;
	int                              i2c_index;
	struct hwdriveric_notify_intf_t  *notify;
	void                             *pdata;
};

static inline char const *hwdriveric_intf_get_name(
	const struct hwdriveric_intf_t *i)
{
	return i->vtbl->get_name(i);
}

static inline int hwdriveric_intf_power_on(
	const struct hwdriveric_intf_t *i)
{
	return i->vtbl->power_on(i);
}

static inline int hwdriveric_intf_power_off(
	const struct hwdriveric_intf_t *i)
{
	return i->vtbl->power_off(i);
}

static inline void hwdriveric_intf_notify_error(
	struct hwdriveric_notify_intf_t *i,
	hwdriveric_event_t *driveric_ev)
{
	return i->vtbl->error(i, driveric_ev);
}

int32_t hwdriveric_register(
	struct platform_device *pdev,
	const struct hwdriveric_intf_t *i,
	struct hwdriveric_notify_intf_t **notify);
void hwdriveric_unregister(struct platform_device *pdev);
#endif // __HW_ALAN_KERNEL_DRIVERIC_INTERFACE_H__

