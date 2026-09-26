/*
 * sysconfig.h
 *
 * This is for system config.
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

#ifndef __VFMW_SYSCONFIG_HEADER__
#define __VFMW_SYSCONFIG_HEADER__

#include "vfmw.h"

/* valid vdh num */
#define MAX_VDH_NUM               2

#ifdef ENV_SOS_KERNEL
#define HEAP_SEC_DRM_BASE  0x60000000U
#define HEAP_SEC_DRM_SIZE  0x20000000U // (512*1024*1024)
#endif

/* register offset */
#define SCD_REG_OFFSET            0xc000

#define SOFTRST_REQ_OFFSET        0xcc0c // (0xf80c)
#define SOFTRST_OK_OFFSET         0xcc10 // (0xf810)

#define ALL_RESET_CTRL_BIT        0
#define MFDE_RESET_CTRL_BIT       1
#define SCD_RESET_CTRL_BIT        2
#define BPD_RESET_CTRL_BIT        3

#define ALL_RESET_OK_BIT          0
#define MFDE_RESET_OK_BIT         1
#define SCD_RESET_OK_BIT          2
#define BPD_RESET_OK_BIT          3

#define ALL_RESET_CTRL_MASK       (1 << ALL_RESET_CTRL_BIT)
#define MFDE_RESET_CTRL_MASK      (1 << MFDE_RESET_CTRL_BIT)
#define SCD_RESET_CTRL_MASK       (1 << SCD_RESET_CTRL_BIT)
#define BPD_RESET_CTRL_MASK       (1 << BPD_RESET_CTRL_BIT)

#define ALL_RESET_OK_MASK         (1 << ALL_RESET_OK_BIT)
#define MFDE_RESET_OK_MASK        (1 << MFDE_RESET_OK_BIT)
#define SCD_RESET_OK_MASK         (1 << SCD_RESET_OK_BIT)
#define BPD_RESET_OK_MASK         (1 << BPD_RESET_OK_BIT)

/* FPGA flag */
extern UINT32  g_is_fpga;

/* register base addr & range */
extern UINT32  g_vdh_reg_base_addr;
extern UINT32  g_scd_reg_base_addr;
extern UINT32  g_vdh_reg_range;
extern UINT32  g_soft_rst_req_addr;
extern UINT32  g_soft_rst_ok_addr;

/* smmu page table base addr */
extern UINT64  g_smmu_page_base;

/* peri crg base addr */
extern UINT32  g_pericrg_reg_base_addr;

/* irq num */
extern UINT32  g_vdec_irq_num_norm;
extern UINT32  g_vdec_irq_num_prot;
extern UINT32  g_vdec_irq_num_safe;

#endif
