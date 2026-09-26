/*
 * omxvdec.h
 *
 * This is vdec driver interface.
 *
 * Copyright (c) 2017-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __OMXVDEC_H__
#define __OMXVDEC_H__

#include "platform.h"
#include "public.h"
#include "drv_omxvdec.h"
#include "memory.h"

#define OMXVDEC_VERSION         2017032300
#define MAX_OPEN_COUNT          32
#define IORE_MAP_PARA           4

typedef struct {
	hi_u32 open_count;
	atomic_t nor_chan_num;
	atomic_t sec_chan_num;
	struct mutex omxvdec_mutex;
	struct mutex vdec_mutex_scd;
	struct mutex vdec_mutex_vdh;
	struct mutex vdec_mutex_sec_scd;
	struct mutex vdec_mutex_sec_vdh;
	hi_bool device_locked;
	struct cdev cdev;
	struct device *device;
} omx_vdec_entry;

typedef struct {
	hi_s32 share_fd;
	UADDR  iova;
	hi_u64 vir_addr;
	hi_u32 iova_size;
} vdec_buffer_record;

omx_vdec_entry omx_vdec_get_entry(void);

#endif
