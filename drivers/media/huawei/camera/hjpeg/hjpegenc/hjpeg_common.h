/*
 * hjpeg_common.h
 *
 * provide Macro and struct for jpeg.
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * lixiuhua <aimee.li@hisilicon.com>
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
 */


#ifndef _INCLUDE_HJPEG_COMMON_H
#define _INCLUDE_HJPEG_COMMON_H

#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/proc_fs.h>
#include <linux/wait.h>

//lint -save -e547

//#include "cam_log.h"
/** jpgenc head offset need write in the first 4 bytes of input buffer
if JPGENC_HEAD_OFFSET small than 4, func jpgenc_add_header must be modify */

#define JPGENC_HEAD_SIZE                     640
#define JPGENC_HEAD_OFFSET                   11

#define MAX_JPEG_BUFFER_SIZE                 (64 * 1024 * 1024) /* 64MB=8192x8192 */
#define JPGENC_RESTART_INTERVAL              0
#define JPGENC_RESTART_INTERVAL_ON           1
#define WAIT_ENCODE_TIMEOUT                  10000
#define MICROSECOND_PER_SECOND              (1000000)

#define CHECK_ALIGN(value,align) (value%align == 0)
#pragma GCC visibility push(default)
#ifndef ALIGN_DOWN
#define ALIGN_DOWN(value, al) ((unsigned int )(value) & ~((al) - 1))
#endif
#pragma GCC visibility pop

#define MASK0(name)  (((unsigned int)1<<(name##_LEN))-1)
#define MASK1(name)  ((((unsigned int)1<<(name##_LEN))-1) << (name##_OFFSET))

/* operation on the field of a variable */
#define REG_GET_FIELD(reg, name) \
        (((reg) >> (name##_OFFSET)) & MASK0(name))

#define REG_SET_FIELD(reg, name, val) \
        (reg = ((reg) & ~MASK1(name)) | (((val) & MASK0(name)) << (name##_OFFSET)))


static inline
void set_reg_val(void __iomem *addr, u32 value)
{
    iowrite32(value, addr);
}
static inline
u32 get_reg_val(void __iomem *addr)
{
    return ioread32(addr);
}

#define SET_FIELD_TO_VAL(reg, field, value) REG_SET_FIELD(reg, field, value)

#define SET_FIELD_TO_REG(reg, field, value) \
    do { \
        unsigned int v = get_reg_val(reg); \
        SET_FIELD_TO_VAL(v, field, value); \
        set_reg_val(reg, v); \
    } while(0)

typedef enum _format_e
{
    JPGENC_FORMAT_YUV422 = 0x0,
    JPGENC_FORMAT_YUV420 = 0x10,
    JPGENC_FORMAT_BIT    = 0xF0,
}format_e;

enum JPEG_CLK_TYPE {
    JPEG_FUNC_CLK = 0,
    JPEG_CLK_MAX
};

typedef enum _chip_type_e
{
    CT_ES = 0,
    CT_CS,
} chip_type_e;

typedef enum _power_controller_e
{
    PC_DRV = 0,
    PC_HISP,
    PC_SELF,
    PC_MAX,
}power_controller_e;

typedef enum _smmu_bypass_e
{
    BYPASS_NO = 0,
    BYPASS_YES,
}smmu_bypass_e;

typedef enum _smmu_type_e
{
  ST_ISP_SMMU = 0,
  ST_SMMU,
  ST_MAX,
}smmu_type_e;

typedef struct _phy_pgd
{
    unsigned int                                phy_pgd_base;
    unsigned int                                phy_pgd_fama_ptw_msb;
    unsigned int                                phy_pgd_fama_bps_msb_ns;
}phy_pgd_t;


typedef struct _tag_hjpeg_cvdr_prop
{
    u32           type;
    u32           rd_port;
    u32           wr_port;
    u32           flag;
    u32           wr_limiter;
    u32           rd_limiter;
    u32           allocated_du;
}hjpeg_cvdr_prop_t;


// struct for all phy base address config in dts and ioremap virtual address
typedef struct _tag_hjpeg_hw_ctl
{
    void __iomem *                              jpegenc_viraddr;
    phys_addr_t                                 jpegenc_phyaddr;    //JPEG REG base
    u32                                         jpegenc_mem_size;
    void __iomem *                              cvdr_viraddr;
    phys_addr_t                                 cvdr_phyaddr;     // CVDR REG base
    u32                                         cvdr_mem_size;
    void __iomem *                              smmu_viraddr;
    phys_addr_t                                 smmu_phyaddr;     // SMMU REG base
    u32                                         smmu_mem_size;
    void __iomem *                              top_viraddr;
    phys_addr_t                                 top_phyaddr;      // TOP REG base
    u32                                         top_mem_size;
    void __iomem *                              subctrl_viraddr;
    phys_addr_t                                 subctrl_phyaddr;    // sub ctrl REG base
    u32                                         subctrl_mem_size;

    u32                                         power_controller;     // config in dts,enum:power_controller_e
    u32                                         smmu_bypass;          // config in dts, 0 non-bypass 1 bypass
    u32                                         smmu_type;            // config in dts, enum:smmu_type_e
    u32                                         chip_type;            // config in dts, 0 ES,1 CS  enum:chip_type_e
    hjpeg_cvdr_prop_t                           cvdr_prop;            // cvdr property defined in dts;
    u32                                         stream_id[3];
    void*                                       jpg_smmu_rwerraddr_virt; // SMMU error address
}hjpeg_hw_ctl_t;

#endif  // _INCLUDE_HJPEG_COMMON_H
//lint -restore
