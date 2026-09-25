/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2019. All rights reserved.
 * Description: omxvdec export header file
 * Notice:This file is referenced by both component and driver
 * If modified, both sides need to be recompiled!!!
 * Author: yangyichang
 * Create: 2014-11-26
 */

#ifndef __DRV_OMXVDEC_H__
#define __DRV_OMXVDEC_H__

#include <linux/ioctl.h>
#include "hi_type.h"

#define OMXVDEC_NAME                    "hi_vdec"
#define PATH_LEN                        256

/*
 * IOCTL for interaction with omx components
 * For Compat USER 32 -> KER 64, all pointer menbers set in last in structure.
 * NOTICE: MUST NOT ALTER the member sequence of this structure
 */
typedef struct hi_omxvdec_ioctl_msg {
	hi_s32 chan_num;
	hi_s32 in_size;
	hi_s32 out_size;
	hi_void *in;
	hi_void *out;
} vdec_ioctl_msg;

// Modified for 64-bit platform
typedef struct hi_compat_ioctl_msg {
	hi_s32 chan_num;
	hi_s32 in_size;
	hi_s32 out_size;
	compat_ulong_t in;
	compat_ulong_t out;
} compat_ioctl_msg;

#define VDEC_IOCTL_MAGIC 'v'

#define VDEC_IOCTL_SET_CLK_RATE    \
	_IO(VDEC_IOCTL_MAGIC, 20)

#define VDEC_IOCTL_GET_VDM_HWSTATE \
	_IO(VDEC_IOCTL_MAGIC, 21)

#define VDEC_IOCTL_SCD_PROC        \
	_IO(VDEC_IOCTL_MAGIC, 22)

#define VDEC_IOCTL_VDM_PROC        \
	_IO(VDEC_IOCTL_MAGIC, 23)

#define VDEC_IOCTL_LOCK_HW         \
	_IO(VDEC_IOCTL_MAGIC, 24)

#define VDEC_IOCTL_UNLOCK_HW       \
	_IO(VDEC_IOCTL_MAGIC, 25)

#define VDEC_IOCTL_IOMMU_MAP       \
	_IO(VDEC_IOCTL_MAGIC, 26)

#define VDEC_IOCTL_IOMMU_UNMAP     \
	_IO(VDEC_IOCTL_MAGIC, 27)

#define VDEC_IOCTL_GET_ACTIVE_REG   \
	_IO(VDEC_IOCTL_MAGIC, 28)

#define VDEC_IOCTL_DEC_START       \
	_IO(VDEC_IOCTL_MAGIC, 29)

#define VDEC_IOCTL_DEC_IRQ_QUERY   \
	_IO(VDEC_IOCTL_MAGIC, 30)

#define VDEC_IOCTL_DEC_CANCEL      \
	_IO(VDEC_IOCTL_MAGIC, 31)

#define VDEC_IOCTL_SCD_START      \
	_IO(VDEC_IOCTL_MAGIC, 32)

#define VDEC_IOCTL_SCD_DEV_CANCEL  \
	_IO(VDEC_IOCTL_MAGIC, 33)

#endif
