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


#ifndef __DRV_NV_ID_ACORE_H__
#define __DRV_NV_ID_ACORE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

//[NV elf编译项目]去除重复定义 xuziheng/306651 20171207
#ifndef NV_DEFINE

#ifndef s8
typedef signed char          s8;
#endif
#ifndef u8
typedef unsigned char        u8;
#endif
#ifndef s16
typedef signed short         s16;
#endif
#ifndef u16
typedef unsigned short       u16;
#endif
#ifndef s32
typedef signed int           s32;
#endif
#ifndef u32
typedef unsigned int         u32;
#endif
#ifndef s64
typedef signed long long     s64;
#endif
#ifndef u64
typedef unsigned long long   u64;
#endif

#endif

/*
 *  NV ID 的添加按从小到大排列
 */

typedef enum
{
    NV_ID_DRV_USB_FEATURE                  = 0xd123,    //NV_USB_FEATURE
    NV_ID_DRV_USB_DBG                      = 0xd12c,    //DRV_USB_DBG_STRU
    NV_ID_DRV_CCORE_RESET                  = 0xd134,    //DRV_CCORE_RESET_STRU
    NV_ID_DRV_DUMP_FILE                    = 0xd144,    //DUMP_FILE_CFG_STRU
    NV_ID_DRV_DLOCK                        = 0xd155,    //DRV_DLOCK_CFG_STRU
	
	//定义在ccore_nv_id_drv.h中 xuziheng/306651 20171207
#ifndef NV_DEFINE
    NV_ID_DRV_DVS_CONFIG                   = 0xd157,    //ST_DVS_CONFIG_STRU
#endif
    NV_ID_DRV_NOC_AP                       = 0xd16c,    //DRV_NOC_AP_CFG_STRU
    NV_ID_DMSS_KENERL			   		   = 0xd180,    //DRV_DMSS_CFG_STRU

    NV_ID_DRV_ID_ACORE_MAX                 = 0xd1ff
}NV_ID_DRV_ACORE_ENUM;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

