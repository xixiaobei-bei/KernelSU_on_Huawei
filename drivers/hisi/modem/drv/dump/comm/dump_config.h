/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef __DUMP_EXC_CTRL_H__
#define __DUMP_EXC_CTRL_H__

#include "osl_types.h"
#include "bsp_dump.h"
#include "acore_nv_stru_drv.h"
#include "nv_stru_drv.h"
#ifdef BSP_CONFIG_PHONE_TYPE
#include "../../adrv/adrv.h"
#else
#include <linux/hisi/rdr_pub.h>
#endif

#include <bsp_print.h>


/**************************************************************************
  OTHERS定义
**************************************************************************/
#define dump_debug(fmt, ...)    (bsp_debug("<%s> "fmt, __FUNCTION__, ##__VA_ARGS__))
#define dump_warning(fmt, ...)  (bsp_wrn("<%s> "fmt, __FUNCTION__, ##__VA_ARGS__))
#define dump_error(fmt, ...)    (bsp_err("<%s> "fmt, __FUNCTION__, ##__VA_ARGS__))
#define dump_fetal(fmt, ...)    (bsp_fatal("<%s> "fmt, __FUNCTION__, ##__VA_ARGS__))
#define dump_ok(fmt, ...)       (bsp_err(fmt,##__VA_ARGS__))


void dump_product_type_init(void);
dump_product_type_t dump_get_product_type(void);
DUMP_FILE_CFG_STRU* dump_get_file_cfg(void);
NV_DUMP_STRU* dump_get_feature_cfg(void);
enum EDITION_KIND dump_get_edition_type(void);
void dump_config_init(void);
s32 dump_check_reset_timestamp(u32 modid);

/*modem ap flag 定义*/
#define DUMP_INIT_FLAG_CONFIG                 (0x5B5B0000)
#define DUMP_INIT_FLAG_BASEINFO               (0x5B5B0001)
#define DUMP_INIT_FLAG_SAVETASK               (0x5B5B0002)
#define DUMP_INIT_FLAG_RDR_REG                (0x5B5B0003)
#define DUMP_INIT_FLAG_MDMAP                  (0x5B5B0004)
#define DUMP_INIT_FLAG_MDMCP                  (0x5B5B0005)
#define DUMP_INIT_FLAG_APR                    (0x5B5B0005)
#define DUMP_INIT_FLAG_DONE                   (0x5B5B0006)

void dump_set_exc_flag(u32 flag);
u32 dump_check_has_error(void);
u32 dump_get_init_phase(void);
void dump_set_init_phase(u32 flag);
dump_access_mdmddr_type_t dump_get_access_mdmddr_type(void);
void dump_memcpy(u32 * dst, const u32 * src, u32 len);

#endif
