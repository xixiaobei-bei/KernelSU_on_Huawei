/*
 * drv_venc_efl.h
 *
 * This is for venc drv.
 *
 * Copyright (c) 2009-2020 Huawei Technologies CO., Ltd.
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

#ifndef __DRV_VENC_EFL_H__
#define __DRV_VENC_EFL_H__

#include "drv_common.h"
#include "drv_mem.h"

typedef enum {
	YUV420_SEMIPLANAR     = 0,
	YUV420_PLANAR         = 3,
	YUV422_PLANAR         = 4,
	YUV422_PACKAGE        = 6,
	RGB_32BIT             = 8,
	YUV420_SEMIPLANAR_CMP = 10,
} color_format_t;

typedef struct {
	mem_buffer_t internal_buffer;
	mem_buffer_t image_buffer;
	mem_buffer_t stream_buffer[MAX_SLICE_NUM];
	mem_buffer_t stream_head_buffer;
} venc_buffer_info_t;

int32_t venc_drv_alloc_encode_done_info_buffer(struct file  *file);
int32_t venc_drv_free_encode_done_info_buffer(const struct file  *file);
int32_t venc_drv_encode(struct encode_info *encode_info, struct venc_fifo_buffer *buffer);
int32_t venc_drv_create_queue(void);
void venc_drv_destroy_queue(void);
int32_t venc_drv_open_vedu(void);
int32_t venc_drv_close_vedu(void);
int32_t venc_drv_suspend_vedu(void);
int32_t venc_drv_resume_vedu(void);
int32_t venc_drv_resume(struct platform_device *pdev);
int32_t venc_drv_suspend(struct platform_device *pdev, pm_message_t state);
void venc_drv_init_pm(void);
void venc_drv_deinit_pm(void);
void venc_drv_print_pm_info(void);

#endif

