/*
 *  Hisilicon K3 SOC camera driver source file
 *
 *  Copyright (C) Huawei Technology Co., Ltd.
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __HW_ALAN_KERNEL_DRIVERIC_INTERFACE_H__
#define __HW_ALAN_KERNEL_DRIVERIC_INTERFACE_H__

#include <linux/videodev2.h>
#include <media/huawei/camera.h>
#include <media/huawei/driver_ic_cfg.h>
#include <media/videobuf2-core.h>

#include "hwcam_intf.h"

typedef struct _tag_hwdriveric_notify_vtbl hwdriveric_notify_vtbl_t;
typedef struct _tag_hwdriveric_notify_intf hwdriveric_notify_intf_t;

typedef struct _tag_hwdriveric_notify_vtbl
{
    void (*error)(hwdriveric_notify_intf_t* i, hwdriveric_event_t* driveric_ev);
} hwdriveric_notify_vtbl_t;

typedef struct _tag_hwdriveric_notify_intf
{
    hwdriveric_notify_vtbl_t*                        vtbl;
} hwdriveric_notify_intf_t;

typedef struct _tag_hwdriveric_vtbl hwdriveric_vtbl_t;
typedef struct _tag_hwdriveric_intf hwdriveric_intf_t;

typedef struct _tag_hwdriveric_vtbl
{
    char const* (*get_name)(const hwdriveric_intf_t *i);
    int (*power_on)(const hwdriveric_intf_t *i);
    int (*power_off)(const hwdriveric_intf_t *i);
    int (*driveric_get_dt_data) (const hwdriveric_intf_t *, struct device_node *);
    int (*driveric_register_attribute)(const hwdriveric_intf_t *, struct device *);
    int (*init)(const hwdriveric_intf_t *i);
} hwdriveric_vtbl_t;

typedef struct _tag_hwdriveric_intf
{
    hwdriveric_vtbl_t*                            vtbl;
} hwdriveric_intf_t;

typedef struct _tag_driveric
{
    hwdriveric_intf_t                  intf;
    char                               name[HWDRIVERIC_NAME_SIZE];
    int                                position;
    int                                i2c_index;
    hwdriveric_notify_intf_t           *notify;
    void                               *pdata;
} driveric_t;

static inline char const*
hwdriveric_intf_get_name(const hwdriveric_intf_t* i)
{
    return i->vtbl->get_name(i);
}

static inline int
hwdriveric_intf_power_on(const hwdriveric_intf_t* i)
{
    return i->vtbl->power_on(i);
}

static inline int
hwdriveric_intf_power_off(const hwdriveric_intf_t* i)
{
    return i->vtbl->power_off(i);
}

static inline void
hwdriveric_intf_notify_error(hwdriveric_notify_intf_t *i, hwdriveric_event_t *driveric_ev)
{
    return i->vtbl->error(i,driveric_ev);
}

extern int32_t hwdriveric_register(struct platform_device* pdev, const hwdriveric_intf_t* i,hwdriveric_notify_intf_t** notify);
void hwdriveric_unregister(hwdriveric_intf_t* driveric_intf);
#endif // __HW_ALAN_KERNEL_DRIVERIC_INTERFACE_H__

