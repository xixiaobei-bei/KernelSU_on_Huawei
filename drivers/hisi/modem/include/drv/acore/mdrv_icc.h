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
/* *
 * @brief   icc模块acore对外头文件，融合架构不对外提供。
 * @file    mdrv_icc.h
 * @author  tianfeilong
 * @version v1.0
 * @date    2019.12.17
 * @note    修改记录(版本号|修订日期|说明)
 * <ul><li>v1.0|2019.12.17|增加说明：该头文件融合架构不对外提供</li></ul>
 * @since MDRV1.0
 */
#ifndef __MDRV_ACORE_ICC_H__
#define __MDRV_ACORE_ICC_H__

#include "mdrv_public.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BSP_ERR_ICC_BASE
#define BSP_ERR_ICC_BASE (int)(0x80000000 | 0x10180000)
#endif

/* C核发生复位 */
#ifndef BSP_ERR_ICC_CCORE_RESETTING
#define BSP_ERR_ICC_CCORE_RESETTING (BSP_ERR_ICC_BASE + 0x12)
#endif

/* 通道满 */
#ifndef ICC_INVALID_NO_FIFO_SPACE
#define ICC_INVALID_NO_FIFO_SPACE (BSP_ERR_ICC_BASE + 0x13)
#endif

typedef enum icc_chan_mode {
    ICC_CHAN_MODE_STREAM = 0,
    ICC_CHAN_MODE_PACKET,
    ICC_CHAN_MODE_BUTT
} icc_chan_mode_e;

typedef unsigned int (*icc_event_cb)(unsigned int u32ChanID, unsigned int u32Event, void *Param);
typedef unsigned int (*icc_write_cb)(unsigned int u32ChanID);
typedef unsigned int (*icc_read_cb)(unsigned int u32ChanID, int u32Size);

typedef struct icc_chan_attr {
    unsigned int u32FIFOInSize;
    unsigned int u32FIFOOutSize;
    unsigned int u32Priority;
    icc_chan_mode_e enChanMode;
    unsigned int u32TimeOut;
    icc_event_cb event_cb;
    icc_write_cb write_cb;
    icc_read_cb read_cb;
} icc_chan_attr_s;


/* ***********************************************************************
 * 函 数 名  : mdrv_icc_open
 * 功能描述  :
 * 输入参数  :
 * u32ChanId: ICC 逻辑通道号
 * pChanAttr: ICC 通道属性，函数内只用其值
 * 输出参数  : 无
 * 返 回 值  :  0          操作成功。
 * 其他        操作失败。
 * ************************************************************************ */
int mdrv_icc_open(unsigned int u32ChanId, icc_chan_attr_s *pChanAttr);

/* ***********************************************************************
 * 函 数 名  : mdrv_icc_read
 * 功能描述  :
 * 输入参数  :
 * u32ChanId: ICC 逻辑通道号
 * pData:     数据存放地址
 * s32Size:   回调函数返回的数据长度
 * 输出参数  : pData       数据
 * 返 回 值  : 正值        数据长度。
 * 其他        操作失败。
 * ************************************************************************ */
int mdrv_icc_read(unsigned int u32ChanId, unsigned char *pData, int s32Size);

/* ***********************************************************************
 * 函 数 名  : mdrv_icc_write
 * 功能描述  :
 * 输入参数  :
 * u32ChanId: ICC 逻辑通道号
 * pData:     数据存放地址
 * s32Size:   数据长度
 * 输出参数  : 无
 * 返 回 值  : 与s32Size相等  操作成功。
 * 其他           操作失败。
 * ************************************************************************ */
int mdrv_icc_write(unsigned int u32ChanId, unsigned char *pData, int s32Size);


typedef enum tagMDRV_ICC_TYPE {
    MDRV_ICC_VT_VOIP, /* for LTE video phone use, ACORE<-->CCORE */
    MDRV_ICC_RCS_SERV,
    MDRV_ICC_GUOM4,            /* for GU OM use, ACORE<-->CCORE */
    MDRV_ICC_NRCCPU_APCPU_OSA, /* for OSA use, NRCcore<--> APcpu */
    MDRV_ICC_PCIE_SENSORHUB,   /* for A+B use, Ccore<--> Acore <-->Sensorhub */
    MDRV_ICC_CUST,  /* for MBB cust use, Ccore<--> Acore */
    MDRV_ICC_MAX
} icc_type_e;


typedef enum tagUDI_ICC_DEV_TYPE {
    UDI_ICC_GUOM4 = MDRV_ICC_GUOM4,
    UDI_ICC_NRCCPU_APCPU_OSA = MDRV_ICC_NRCCPU_APCPU_OSA,
    UDI_ICC_PCIE_SENSORHUB = MDRV_ICC_PCIE_SENSORHUB,
    UDI_ICC_CUST = MDRV_ICC_CUST,

    UDI_ICC_MAX
} udi_icc_dev_type_e;

#define DRV_ICC_OPEN(u32ChanId, pChanAttr) mdrv_icc_open(u32ChanId, pChanAttr)
#define DRV_ICC_READ(u32ChanId, pData, s32Size) mdrv_icc_read(u32ChanId, pData, s32Size)
#define DRV_ICC_WRITE(u32ChanId, pData, s32Size) mdrv_icc_write(u32ChanId, pData, s32Size)

/* ***********************************************************************
 * 函 数 名  : mdrv_icc_register_resume_cb
 * 功能描述  : 注册icc 唤醒A核时的钩子函数，AP在睡眠状态下被CP唤醒
 * 方便定位是被什么事件唤醒的
 * 输入参数  :
 * u32ChanId: icc channel id
 * funcptr_1: 回调函数
 * param    : 回调函数参数
 * 输出参数  : 无
 * 返 回 值  :  0          操作成功。
 * 其他        操作失败。
 * ************************************************************************ */
int mdrv_icc_register_resume_cb(unsigned int u32ChanId, funcptr_1 debug_routine, int param);

#ifdef __cplusplus
}
#endif
#endif
