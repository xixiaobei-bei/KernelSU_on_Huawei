/*
 * hwois.h
 *
 * driver for hwois
 *
 * Copyright (c) 2014-2020 Huawei Technologies Co., Ltd.
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

#ifndef _HISI_OIS_H_
#define _HISI_OIS_H_
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <media/v4l2-subdev.h>
#include <media/huawei/ois_cfg.h>
#include "cam_log.h"
#include "hwcam_intf.h"

struct hw_ois_vtbl;

/* ois controler struct define */
struct hw_ois_info {
	const char *ois_name;
	unsigned int slave_address;
	int ois_support;
};

struct hw_ois_intf_t {
	struct hw_ois_vtbl *vtbl;
	struct v4l2_subdev *subdev;
};

struct ois_t {
	struct hw_ois_intf_t intf;
	struct hw_ois_info *ois_info;
};

struct hw_ois_t {
	struct v4l2_subdev subdev;
	struct platform_device *pdev;
	struct hw_ois_intf_t *intf;
	struct hw_ois_info *ois_info;
	struct mutex lock;
};

/* ois function table */
struct hw_ois_vtbl {
	int (*ois_match_id)(struct hw_ois_intf_t *, void *);
	int (*ois_config)(struct hw_ois_t *, void *);
	int (*ois_i2c_read)(struct hw_ois_intf_t *, void *);
	int (*ois_i2c_write)(struct hw_ois_intf_t *, void *);
	int (*ois_ioctl)(struct hw_ois_intf_t *, void *);
};

/* extern function declare */
int hw_ois_register(struct platform_device *pdev, struct hw_ois_intf_t *intf,
	struct hw_ois_info *hw_ois_info);
void hw_ois_unregister(struct v4l2_subdev *subdev);
int hw_ois_config(struct hw_ois_t *hw_ois, void *arg);
int hw_ois_get_dt_data(struct platform_device *pdev, struct ois_t *ois);

#endif
