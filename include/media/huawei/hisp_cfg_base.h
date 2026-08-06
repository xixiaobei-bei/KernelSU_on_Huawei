/*
 *  Hisilicon K3 SOC camera driver source file
 *
 *  Copyright (C) Huawei Technology Co., Ltd.
 *
 * Author:
 * Email:
 * Date:	  2014-11-11
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

#ifndef __HW_KERNEL_HWCAM_HISP_CFG_BASE_H__
#define __HW_KERNEL_HWCAM_HISP_CFG_BASE_H__

#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <media/huawei/camera.h>
#include <linux/compat.h>
#include <linux/iommu.h>

enum {
	HISP_NAME_SIZE = 32,
	HISP_V4L2_EVENT_TYPE = V4L2_EVENT_PRIVATE_START + 0x00090000,

	HISP_HIGH_PRIO_EVENT = 0x1500,
	HISP_MIDDLE_PRIO_EVENT = 0x2000,
	HISP_LOW_PRIO_EVENT = 0x3000,
};

typedef enum _tag_hisp_event_kind {
	HISP_RPMSG_CB,
} hisp_event_kind_t;

typedef struct _tag_hisp_event {
	hisp_event_kind_t kind;
} hisp_event_t;

enum mapType
{
	MAP_TYPE_DYNAMIC = 0,
	MAP_TYPE_RAW2YUV,
	MAP_TYPE_STATIC,
	MAP_TYPE_STATIC_SEC,
	MAP_TYPE_DYNAMIC_CARVEOUT,
	MAP_TYPE_STATIC_ISP_SEC,
	MAP_TYPE_MAX,
};

typedef struct addr_params
{
	uint32_t moduleAddr;
	uint32_t iova;
	uint32_t sharedFd;
	uint32_t type;
	uint32_t prot;
	uint32_t size;
	void *vaddr;
	size_t offset_in_pool;
	size_t pool_align_size;
	uint32_t security_isp_mode;
	uint32_t isApCached;
}addr_param_t;

struct hisp_cfg_data {
	int cfgtype;
	int mode;
	int isSecure;
	union {
		addr_param_t param;
		struct { // for buffer operation
			int share_fd;
			uint32_t buf_size;
		};
		uint32_t cfgdata[4];
	};
};

enum hisp_config_type {
	HISP_CONFIG_POWER_ON = 0,
	HISP_CONFIG_POWER_OFF,
	HISP_CONFIG_GET_MAP_ADDR,
	HISP_CONFIG_UNMAP_ADDR,
	HISP_CONFIG_INIT_MEMORY_POOL,
	HISP_CONFIG_DEINIT_MEMORY_POOL,
	HISP_CONFIG_ALLOC_MEM,
	HISP_CONFIG_FREE_MEM,
	HISP_CONFIG_ISP_TURBO,
	HISP_CONFIG_ISP_NORMAL,
	HISP_CONFIG_ISP_LOWPOWER,
	HISP_CONFIG_ISP_ULTRALOW,
	HISP_CONFIG_R8_TURBO,
	HISP_CONFIG_R8_NORMAL,
	HISP_CONFIG_R8_LOWPOWER,
	HISP_CONFIG_R8_ULTRALOW,
	HISP_CONFIG_PROC_TIMEOUT,
	HISP_CONFIG_GET_SEC_ISPFW_SIZE,
	HISP_CONFIG_SET_SEC_ISPFW_BUFFER,
	HISP_CONFIG_RELEASE_SEC_ISPFW_BUFFER,
	HISP_CONFIG_SET_MDC_BUFFER,
	HISP_CONFIG_RELEASE_MDC_BUFFER,
	HISP_CONFIG_MAX_INDEX
};

typedef struct _tag_hisp_info {
	char name[HISP_NAME_SIZE];
} hisp_info_t;

typedef struct _hisp_system_time_t{
    struct timeval s_timeval;
    unsigned int s_system_couter_rate;
    unsigned long long s_system_counter;

}hisp_system_time_t;

#define HISP_IOCTL_POWER_ON   _IO('A', BASE_VIDIOC_PRIVATE + 0x01)
#define HISP_IOCTL_POWER_OFF  _IO('A', BASE_VIDIOC_PRIVATE + 0x02)
#define HISP_IOCTL_GET_INFO   _IOR('A', BASE_VIDIOC_PRIVATE + 0x05, hisp_info_t)
#define HISP_IOCTL_CFG_ISP    _IOWR('A', BASE_VIDIOC_PRIVATE + 0x06, \
				struct hisp_cfg_data)
#define HISP_IOCTL_GET_SYSTEM_TIME   _IOR('A', BASE_VIDIOC_PRIVATE + 0x07, hisp_system_time_t)
#endif /* __HW_KERNEL_HWCAM_HISP_CFG_BASE_H__ */
