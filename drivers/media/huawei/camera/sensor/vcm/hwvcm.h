/*
 * hwvcm.h
 *
 * driver for hwvcm
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

#ifndef _HISI_VCM_H_
#define _HISI_VCM_H_
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
#include <media/huawei/vcm_cfg.h>
#include "cam_log.h"
#include "hwcam_intf.h"

struct hw_vcm_vtbl;

/* vcm controler struct define */
struct hw_vcm_info {
	const char *vcm_name;
	unsigned int slave_address;
	int index;
	int data_type;
};

struct hw_vcm_intf_t {
	struct hw_vcm_vtbl *vtbl;
	struct v4l2_subdev *subdev;
};

struct vcm_t {
	struct hw_vcm_intf_t intf;
	struct hw_vcm_info *vcm_info;
};

struct hw_vcm_t {
	struct v4l2_subdev subdev;
	struct platform_device *pdev;
	struct hw_vcm_intf_t *intf;
	struct hw_vcm_info *vcm_info;
	struct mutex lock;
};

/* vcm function table */
struct hw_vcm_vtbl {
	int (*vcm_match_id)(struct hw_vcm_intf_t *, void *);
	int (*vcm_config)(struct hw_vcm_t *, void *);
	int (*vcm_i2c_read)(struct hw_vcm_intf_t *, void *);
	int (*vcm_i2c_write)(struct hw_vcm_intf_t *, void *);
	int (*vcm_ioctl)(struct hw_vcm_intf_t *, void *);
};

/* extern function declare */
int hw_vcm_register(struct platform_device *pdev, struct hw_vcm_intf_t *intf,
	struct hw_vcm_info *hw_vcm_info);
void hw_vcm_unregister(struct v4l2_subdev *subdev);
int hw_vcm_config(struct hw_vcm_t *hw_vcm, void *arg);
int hw_vcm_get_dt_data(struct platform_device *pdev, struct vcm_t *vcm);

#endif
