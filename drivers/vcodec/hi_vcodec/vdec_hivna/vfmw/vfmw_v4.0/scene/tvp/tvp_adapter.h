/*
 * tvp_adapter.h
 *
 * This is for vdec tvp adapter.
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

#ifndef __TVP_ADAPTER_H__
#define __TVP_ADAPTER_H__

#include "public.h"

#ifdef ENV_ARMLINUX_KERNEL
hi_s32  tvp_vdec_secure_init(void);
hi_s32  tvp_vdec_secure_exit(void);
hi_s32  tvp_vdec_suspend(void);
hi_s32  tvp_vdec_resume(void);
#endif

#define TEEC_OPERATION_PARA_INDEX_FIRST  1
#define TEEC_OPERATION_PARA_INDEX_SECOND 2
#define TEEC_OPERATION_PARA_INDEX_THIRD  3

#define TVP_PACKAGE_NAME_MAX_LENGTH      70

#endif

