/*
 * hjpeg_debug.h
 *
 * provide interface for debug.
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


#ifndef __HJPEG_DEBUG_H_INCLUDE__
#define __HJPEG_DEBUG_H_INCLUDE__

#define DUMP_REGS 0
#define DUMP_ERR_DATA 0

#define POWER_CTRL_HW_INTF      (0)
#define POWER_CTRL_CFG_REGS     (1)
#define POWER_CTRL_INTERFACE    POWER_CTRL_HW_INTF

#if (POWER_CTRL_INTERFACE==POWER_CTRL_CFG_REGS)
int cfg_map_reg_base(void);
int cfg_unmap_reg_base(void);

int cfg_powerup_regs(void);
int cfg_powerdn_regs(void);
#endif//(POWER_CTRL_INTERFACE==POWER_CTRL_CFG_REGS)

#if DUMP_REGS
int dump_cvdr_reg(u32 chip_type, void __iomem * viraddr);
int dump_jpeg_reg(u32 chip_type, void __iomem * viraddr);
#endif//DUMP_REGS

#if defined( HISP120_CAMERA )
int hjpeg_120_map_reg(void);
void hjpeg_120_unmap_reg(void);
void hjpeg_120_dump_reg(void);
#endif

#endif//__HJPEG_DEBUG_H_INCLUDE__
