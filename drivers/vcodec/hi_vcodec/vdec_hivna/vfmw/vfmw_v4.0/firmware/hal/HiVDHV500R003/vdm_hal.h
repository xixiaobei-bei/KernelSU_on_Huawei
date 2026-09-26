/*
 * vdm_hal.h
 *
 * This is vdm hal interface export header file.
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

#ifndef _VDM_HAL_HEADER_
#define _VDM_HAL_HEADER_

#include "vdm_drv.h"

#define VDMHAL_OK                  0
#define VDMHAL_ERR                (-1)

#define MAX_SLICE_SLOT_NUM         200

#define MAX_IMG_WIDTH_IN_MB        512
#define MAX_IMG_HALF_HEIGHT_IN_MB  256
#define MAX_IMG_HEIGHT_IN_MB      (MAX_IMG_HALF_HEIGHT_IN_MB * 2)
#define MAX_MB_NUM_IN_PIC         (MAX_IMG_WIDTH_IN_MB * MAX_IMG_HEIGHT_IN_MB)

#define MAX_SLOT_WIDTH             4096
#define MSG_SLOT_RATIO             4
#define DEFAULT_HEAD_BUF_NUM       8
#define MAX_HEAD_BUF_NUM           12
#define MAX_RESET_TIMES            100
#define MAX_HEVC_PIX_WIDTH         4096
#ifndef HIVCODEC_PLATFORM_ECONOMIC
#define MAX_STRIDE                ((1024 * MAX_SLOT_WIDTH / 64 + ((1024) - 1)) & (~((1024) - 1)))
#else
#define ALIGN_LEN                 128
#define MAX_STRIDE                (((MAX_SLOT_WIDTH + ALIGN_LEN - 1) / ALIGN_LEN + 1) * ALIGN_LEN * 16)
#endif
/* Register read/write interface */
/* mfde register read/write */
#define read_vreg(reg, dat, vdh_id) \
	do { \
		if ((vdh_id) < MAX_VDH_NUM) \
			(dat) = readl(((volatile SINT32 *)((SINT8*) g_hw_mem[(vdh_id)].vdm_reg_vir_addr + (reg)))); \
		else \
			dprint(PRN_ALWS, "%s: read_vreg but Vdh : %d is more than MAX_VDH_NUM : %d\n", \
			__func__, (vdh_id), MAX_VDH_NUM); \
	} while (0)

#define write_vreg(reg, dat, vdh_id) \
	do { \
		if ((vdh_id) < MAX_VDH_NUM) \
			writel ((dat), ((volatile SINT32 *)((SINT8*) g_hw_mem[(vdh_id)].vdm_reg_vir_addr + (reg)))); \
		else \
			dprint(PRN_ALWS, "%s: write_vreg but Vdh : %d is more than MAX_VDH_NUM : %d\n", \
			__func__, (vdh_id), MAX_VDH_NUM); \
	} while (0)

/* message pool read/write */
#define rd_msgword(vir_addr, dat) \
	do { \
		(dat) = *((volatile SINT32*)(vir_addr)); \
	} while (0)

#define wr_msgword(vir_addr, dat) \
	do { \
		*((volatile SINT32*)((SINT8*)(vir_addr))) = (dat); \
	} while (0)

/* condition check */
#define vdmhal_assert_ret(cond, else_print) \
	do { \
		if (!(cond)) { \
			dprint(PRN_FATAL, "%s %d: %s\n", __func__, __LINE__, (else_print)); \
			return VDMHAL_ERR; \
		} \
	} while (0)

#endif
