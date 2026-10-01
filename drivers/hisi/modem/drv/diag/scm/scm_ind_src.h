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

#ifndef _SCM_IND_SRC_H_
#define _SCM_IND_SRC_H_

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include <mdrv.h>
#include <mdrv_diag_system.h>
#include <osl_types.h>
#include <bsp_dump.h>
#include <asm/dma-mapping.h>
#include <linux/dma-mapping.h>
#include <asm/io.h>
#include <linux/gfp.h>
#include <linux/mm.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#define SCM_CODE_SRC_BD_NUM         (4*1024)        /* 链式通道BD的个数 */
#define SCM_CODE_SRC_RD_NUM         (4*1024)        /* 链式通道RD的个数 */

#define SCM_CODER_SRC_BDSIZE        (sizeof(SOCP_BD_DATA_STRU)*SCM_CODE_SRC_BD_NUM)   /* 链式通道的BD空间 */
#define SCM_CODER_SRC_RDSIZE        (sizeof(SOCP_RD_DATA_STRU)*SCM_CODE_SRC_RD_NUM)   /* 链式通道的RD空间 */

#define SCM_HISI_HEADER_MAGIC               (0x48495349) /*HISI*/

#define DIAG_CODER_SRC_IND_BUF_A_SIZE               (1024*1024)
#define SCM_CODER_SRC_IND_BUFFER_SIZE               (DIAG_CODER_SRC_IND_BUF_A_SIZE)

#define SCM_CODER_SRC_MAX_LEN       (4*1024)        /* 编码通道最大发送数据长度  */

#define SCM_CODER_SRC_RD_THRESHOLD  (0)             /* 编码源通道RD阈值 */


#ifdef ENABLE_DIAG_FIX_ADDR
#ifndef SCM_MEMMAP
#define SCM_MEMMAP(addr,len)            (ioremap_wc(addr,len))
#endif
#endif

/*4字节对齐*/
#define ALIGN_DDR_WITH_4BYTE(len)       (((len) + 3)&(~3))
/*8字节对齐*/
#define ALIGN_DDR_WITH_8BYTE(len)       (((len) + 7)&(~7))

typedef u32 SCM_CHANNEL_INIT_ENUM_U32;

enum  SCM_CHANNEL_INIT_ENUM
{
    SCM_CHANNEL_UNINIT = 0,             /* 未初始化 */
    SCM_CHANNEL_INIT_SUCC,              /* 初始化成功 */
    SCM_CHANNEL_MEM_FAIL,               /* 初始化申请内存错误 */
    SCM_CHANNEL_CFG_FAIL,               /* 初始化通道配置 */
    SCM_CHANNEL_START_FAIL,             /* 通道开启错误 */
    SCM_CHANNEL_INIT_BUTT
};


typedef struct
{
    SCM_CHANNEL_INIT_ENUM_U32       enInitState;    /* 通道初始化状态，初始化后自动修改 */
    SOCP_CODER_SRC_ENUM_U32         enChannelID;    /* 编码源通道ID，固定配置 */
    SOCP_CODER_DST_ENUM_U32         enDstCHID;      /* 编码目的通道ID */
    SOCP_DATA_TYPE_ENUM_UIN32       enDataType;     /* 数据来源类型 */
    SOCP_ENCSRC_CHNMODE_ENUM_UIN32  enCHMode;       /* 通道类型 */
    SOCP_CHAN_PRIORITY_ENUM_UIN32   enCHLevel;      /* 通道优先级 */
	SOCP_TRANS_ID_EN_ENUM_UINT32    enTransIdEn;    /* SOCP Trans Id使能位 */
	SOCP_PTR_IMG_EN_ENUM_UINT32     enPtrImgEn;     /* SOCP 指针镜像使能位 */
    u32                             ulSrcBufLen;    /* 编码源通道数据空间大小 */
    u32                             ulRDBufLen;     /* 编码源通道RD数据空间大小 */
    u8                              *pucSrcBuf;     /* 编码源通道数据空间内存指针 */
    u8                              *pucSrcPHY;
    u8                              *pucRDBuf;      /* 编码源通道RD数据空间内存指针 */
    u8                              *pucRDPHY;
    unsigned long                   pRptrImgPhyAddr; /* 编码源通道读指针镜像物理地址 */
    unsigned long                   pRptrImgVirtAddr;/* 编码源通道读指针镜像虚拟地址 */
}SCM_CODER_SRC_CFG_STRU;

u32 scm_init_ind_src_buff(void);
u32 scm_create_ind_src_buff(u8 **pBufVir, u8 **pBufPhy, u32 ulLen);
u32 scm_send_ind_src_data(u8 *pucSendDataAddr, u32 ulSendLen);
u32 scm_ind_src_chan_init(void);
u32 scm_ind_src_chan_cfg(SCM_CODER_SRC_CFG_STRU *pstCfg);
unsigned long scm_ind_src_phy_to_virt(u8 * phyAddr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif


