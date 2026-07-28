
/*
 *  Hisilicon K3 SOC camera driver source file
 *
 *  Copyright (C) Huawei Technology Co., Ltd.
 *
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

#ifndef __HW_ALAN_KERNEL_FPGA_INTERFACE_H__
#define __HW_ALAN_KERNEL_FPGA_INTERFACE_H__

#include <linux/videodev2.h>
#include <media/huawei/camera.h>
#include <media/huawei/fpga_cfg.h>
#include <media/videobuf2-core.h>

#include "hwcam_intf.h"

typedef struct _tag_hwfpga_notify_vtbl hwfpga_notify_vtbl_t;
typedef struct _tag_hwfpga_notify_intf hwfpga_notify_intf_t;

typedef struct _tag_hwfpga_notify_vtbl
{
    void (*error)(hwfpga_notify_intf_t* i, hwfpga_event_t* fpga_ev);
} hwfpga_notify_vtbl_t;

typedef struct _tag_hwfpga_notify_intf
{
    hwfpga_notify_vtbl_t*                        vtbl;
} hwfpga_notify_intf_t;

typedef struct _tag_hwfpga_vtbl hwfpga_vtbl_t;
typedef struct _tag_hwfpga_intf hwfpga_intf_t;

typedef struct _tag_hwfpga_vtbl
{
    char const* (*get_name)(const hwfpga_intf_t *i);
    int (*power_on)(const hwfpga_intf_t *i);
    int (*power_off)(const hwfpga_intf_t *i);
    int (*load_firmware)(const hwfpga_intf_t *i);
    int (*enable)(const hwfpga_intf_t *i);
    int (*disable)(const hwfpga_intf_t *i);
    int (*init)(const hwfpga_intf_t *i);
    int (*close)(const hwfpga_intf_t *i);
    int (*fpga_get_dt_data) (const hwfpga_intf_t *, struct device_node *);
    int (*fpga_register_attribute)(const hwfpga_intf_t *, struct device *);
} hwfpga_vtbl_t;

typedef struct _tag_hwfpga_intf
{
    hwfpga_vtbl_t*                            vtbl;
} hwfpga_intf_t;

typedef struct _tag_fpga
{
    hwfpga_intf_t                  intf;
    char 				           name[HWFPGA_NAME_SIZE];
    hwfpga_notify_intf_t          *notify;
} fpga_t;

static inline char const*
hwfpga_intf_get_name(const hwfpga_intf_t* i)
{
    return i->vtbl->get_name(i);
}

static inline int
hwfpga_intf_power_on(const hwfpga_intf_t* i)
{
    return i->vtbl->power_on(i);
}

static inline int
hwfpga_intf_power_off(const hwfpga_intf_t* i)
{
    return i->vtbl->power_off(i);
}

static inline int
hwfpga_intf_load_firmware(const hwfpga_intf_t *i)
{
    return i->vtbl->load_firmware(i);
}

static inline int
hwfpga_intf_enable(const hwfpga_intf_t *i)
{
    return i->vtbl->enable(i);
}

static inline void
hwfpga_intf_notify_error(hwfpga_notify_intf_t *i, hwfpga_event_t *fpga_ev)
{
    return i->vtbl->error(i,fpga_ev);
}

extern int32_t hwfpga_register(struct platform_device* pdev, const hwfpga_intf_t* i,hwfpga_notify_intf_t** notify);
void hwfpga_unregister(hwfpga_intf_t* fpga_intf);
#endif // __HW_ALAN_KERNEL_FPGA_INTERFACE_H__

