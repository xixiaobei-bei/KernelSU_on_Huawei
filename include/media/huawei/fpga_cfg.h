/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Hisilicon K3 SOC camera driver source file
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

#ifndef __HW_ALAN_KERNEL_CAM_FPGA_CFG_H__
#define __HW_ALAN_KERNEL_CAM_FPGA_CFG_H__

#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <media/huawei/camera.h>

typedef enum _tag_hwfpga_config_type {
	CAM_FPGA_POWERON,
	CAM_FPGA_POWEROFF,
	CAM_FPGA_LOADFW,
	CAM_FPGA_ENABLE,
	CAM_FPGA_DISABLE,
	CAM_FPGA_INITIAL,
	CAM_FPGA_CLOSE,
	CAM_FPGA_CHECKDEVICE,
}hwfpga_config_type_t;

enum {
	HWFPGA_NAME_SIZE                          =   32,
	HWFPGA_V4L2_EVENT_TYPE                    =   V4L2_EVENT_PRIVATE_START + 0x00080000,

	HWFPGA_HIGH_PRIO_EVENT                    =   0x1500,
	HWFPGA_MIDDLE_PRIO_EVENT                  =   0x2000,
	HWFPGA_LOW_PRIO_EVENT                     =   0x3000,
};

typedef struct _tag_hwfpga_config_data {
	uint32_t cfgtype;
}hwfpga_config_data_t;

typedef struct _tag_hwfpga_info {
	char                                        name[HWFPGA_NAME_SIZE];
}hwfpga_info_t;

typedef enum _tag_hwfpga_event_kind {
	HWFPGA_INFO_ERROR,
}hwfpga_event_kind_t;

typedef struct _tag_hwfpga_event {
	hwfpga_event_kind_t                          kind;
	union { // can ONLY place 10 int fields here.
		struct {
			uint32_t                            id;
		}error;
	}data;
}hwfpga_event_t;

#define HWFPGA_IOCTL_GET_INFO                _IOR('F', BASE_VIDIOC_PRIVATE + 17, hwfpga_info_t)
#define HWFPGA_IOCTL_CONFIG               	_IOWR('F', BASE_VIDIOC_PRIVATE + 19, hwfpga_config_data_t)

#endif // __HW_ALAN_KERNEL_CAM_FPGA_CFG_H__

