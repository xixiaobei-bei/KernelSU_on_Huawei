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
#include "product_config.h"
#include <linux/version.h>
#include <linux/gfp.h>
#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/vmalloc.h>
#include "socp_balong.h"
#include <linux/clk.h>
#include "bsp_version.h"
#include "bsp_dump.h"
#include "bsp_nvim.h"
/* log2.0 2014-03-19 Begin:*/
#include "acore_nv_stru_drv.h"
#include "bsp_print.h"
#include "bsp_slice.h"
#include "bsp_softtimer.h"
#include "socp_ind_delay.h"
/* log2.0 2014-03-19 End*/

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <linux/sched/types.h>
#endif

#include <securec.h>


SOCP_GBL_STATE g_strSocpStat = {0};
EXPORT_SYMBOL(g_strSocpStat);
SOCP_DEBUG_INFO_S g_stSocpDebugInfo;
EXPORT_SYMBOL(g_stSocpDebugInfo);

 struct socp_enc_dst_log_cfg *SocpLogConfig =NULL ;
/* 任务优先级定义 */
#define SOCP_ENCSRC_TASK_PRO    79
#define SOCP_ENCDST_TASK_PRO    81
#define SOCP_DECSRC_TASK_PRO    79
#define SOCP_DECDST_TASK_PRO    81
/* SOCP基地址 */
u32 g_SocpRegBaseAddr = 0;
/* 中断处理函数 */
u32 socp_app_int_handler(void);

spinlock_t lock;

u32 g_ulThrowout = 0;
u32 g_SocpEnableState[SOCP_MAX_ENCDST_CHN] = {0};  /* socp目的端数据上报控制状态 */
u32 socp_version = 0;
u32 g_ulSocpDebugTraceCfg = 0;
u32 g_strSocpDeflateStatus=0;

#define SOCP_MAX_ENC_DST_COUNT      100
struct socp_enc_dst_stat_s
{
    u32 ulIntStartSlice;
    u32 ulIntEndSlice;
    u32 ulTaskStartSlice;
    u32 ulTaskEndSlice;
    u32 ulReadDoneStartSlice;
    u32 ulReadDoneEndSlice;
};
u32 g_ulEncDstStatCount;

struct socp_enc_dst_stat_s g_stEncDstStat[SOCP_MAX_ENC_DST_COUNT];

extern struct socp_enc_dst_log_cfg g_stEncDstBufLogConfig;

/* 检测是否初始化 */
s32 socp_check_init(void)
{
    if (!g_strSocpStat.bInitFlag)
    {
        socp_error("The module has not been initialized!\n");
        return BSP_ERR_SOCP_NOT_INIT;
    }
    else
    {
        return BSP_OK;
    }
}

/* 检测地址起始结束有效性 */
s32 socp_check_buf_addr(unsigned long start, unsigned long end)
{
    if (start >= end)
    {
        socp_error("The buff is invalid!\n");\
        return BSP_ERR_SOCP_INVALID_PARA;\
    }
    else
    {
        return BSP_OK;
    }
}

/* 检测通道类型 */
s32 socp_check_chan_type(u32 para, u32 type)
{
    if (type != para)
    {
        socp_error("The channel type is invalid!\n");
        return BSP_ERR_SOCP_INVALID_CHAN;
    }
    else
    {
        return BSP_OK;
    }
}

/* 检测通道ID是否有效 */
s32 socp_check_chan_id(u32 para, u32 id)
{
    if (para >= id)
    {
        socp_error("The channel id is invalid!\n");
        return BSP_ERR_SOCP_INVALID_CHAN;
    }
    else
    {
        return BSP_OK;
    }
}

/* 检查编码源通道ID有效性 */
s32 socp_check_encsrc_chan_id(u32 id)
{
    if ((id >= SOCP_CCORE_ENCSRC_CHN_BASE) && (id < (SOCP_CCORE_ENCSRC_CHN_BASE + SOCP_CCORE_ENCSRC_CHN_NUM)))
    {
        socp_error("The src channel id is invalid!\n");
        return BSP_ERR_SOCP_INVALID_CHAN;
    }
    else
    {
        return BSP_OK;
    }
}

/* 检测是否8字节对齐 */
s32 socp_check_8bytes_align(unsigned long para)
{
    if (0 != (para%8))
    {
        socp_error("The parameter is not 8 bytes aligned!\n");
        return BSP_ERR_SOCP_NOT_8BYTESALIGN;
    }
    else
    {
        return BSP_OK;
    }
}

/* 检测通道优先级有效性 */
s32 socp_check_chan_priority(u32 para)
{
    if (para >= SOCP_CHAN_PRIORITY_BUTT)
    {
        socp_error("The src channele priority[%d] is valid!\n", para);
        return BSP_ERR_SOCP_INVALID_PARA;
    }
    else
    {
        return BSP_OK;
    }
}

/* 检测socp数据类型 */
s32 socp_check_data_type(u32 para)
{
    if (para >= SOCP_DATA_TYPE_BUTT)
    {
        socp_error("channel data type[%d] is invalid!\n", para);
        return BSP_ERR_SOCP_INVALID_PARA;
    }
    else
    {
        return BSP_OK;
    }
}

s32 socp_check_encsrc_alloc(u32 id)
{
    if(SOCP_CHN_ALLOCATED != g_strSocpStat.sEncSrcChan[id].u32AllocStat)
    {
       socp_error("encoder src[%d] is not allocated!\n", id);
       return BSP_ERR_SOCP_INVALID_CHAN;\
    }
    else
    {
        return BSP_OK;
    }
}

s32 socp_check_encdst_set(u32 id)
{
    if(SOCP_CHN_SET != g_strSocpStat.sEncDstChan[id].u32SetStat)
    {
        socp_error("encdst channel[%d] set failed!\n", id);
        return BSP_ERR_SOCP_INVALID_CHAN;
    }
    else
    {
        return BSP_OK;
    }
}

s32 socp_check_decsrc_set(u32 id)
{
    if(SOCP_CHN_SET!= g_strSocpStat.sDecSrcChan[id].u32SetStat)
    {
        socp_error("decsrc channel[%d] set failed!\n", id);
        return BSP_ERR_SOCP_INVALID_CHAN;
    }
    else
    {
        return BSP_OK;
    }
}

s32 socp_check_decdst_alloc(u32 id)
{
    if(SOCP_CHN_ALLOCATED!= g_strSocpStat.sDecDstChan[id].u32AllocStat)
    {
        socp_error("decdst channel[%d] is alloc failed!\n", id);
        return BSP_ERR_SOCP_INVALID_CHAN;
    }
    else
    {
        return BSP_OK;
    }
}

s32 socp_check_data_type_en(u32 param)
{
    if (param >= SOCP_DATA_TYPE_EN_BUTT)
    {
        socp_error("the data type en is valid, para is %d!\n", param);
        return BSP_ERR_SOCP_INVALID_PARA;
    }
    else
    {
        return BSP_OK;
    }
}

s32 socp_check_enc_debug_en(u32 param)
{
    if (param >= SOCP_ENC_DEBUG_EN_BUTT)
    {
        socp_error("the enc src debug en is invalid, para is %d!\n", param);
        return BSP_ERR_SOCP_INVALID_PARA;
    }
    else
    {
        return BSP_OK;
    }
}

void socp_global_reset(void)
{
    SOCP_REG_SETBITS(SOCP_REG_GBLRST, 1, 1, 0x1);
}

void socp_encsrc_headerr_irq_enable(void)
{
    //enable encode head error of all channels.
    SOCP_REG_WRITE(SOCP_REG_APP_MASK1, 0);
}

void socp_global_ctrl_init(void)
{
    unsigned int i;

    /* 初始化全局状态结构体 */
    /* 任务ID初始化 */
    g_strSocpStat.u32EncSrcTskID     = 0;
    g_strSocpStat.u32DecDstTskID     = 0;
    g_strSocpStat.u32EncDstTskID     = 0;
    g_strSocpStat.u32DecSrcTskID     = 0;
    /* 包头错误标志初始化 */
    g_strSocpStat.u32IntEncSrcHeader = 0;
    g_strSocpStat.u32IntEncSrcRD     = 0;
    g_strSocpStat.u32IntDecDstTfr    = 0;
    g_strSocpStat.u32IntDecDstOvf    = 0;
    g_strSocpStat.u32IntEncDstTfr    = 0;
    g_strSocpStat.u32IntEncDstOvf    = 0;
    g_strSocpStat.u32IntDecSrcErr    = 0;
    g_strSocpStat.compress_isr       = NULL;

    for(i=0; i<SOCP_MAX_ENCSRC_CHN; i++)
    {
        g_strSocpStat.sEncSrcChan[i].u32ChanID      = i;
        g_strSocpStat.sEncSrcChan[i].u32ChanEn      = SOCP_CHN_DISABLE;
        g_strSocpStat.sEncSrcChan[i].u32AllocStat   = SOCP_CHN_UNALLOCATED;
        g_strSocpStat.sEncSrcChan[i].u32LastRdSize  = 0;
        g_strSocpStat.sEncSrcChan[i].u32DestChanID  = 0xff;
        g_strSocpStat.sEncSrcChan[i].u32BypassEn    = 0;
        g_strSocpStat.sEncSrcChan[i].ePriority      = SOCP_CHAN_PRIORITY_3;
        g_strSocpStat.sEncSrcChan[i].eDataType      = SOCP_DATA_TYPE_BUTT;
        g_strSocpStat.sEncSrcChan[i].eDataTypeEn    = SOCP_DATA_TYPE_EN_BUTT;
        g_strSocpStat.sEncSrcChan[i].eDebugEn       = SOCP_ENC_DEBUG_EN_BUTT;
        g_strSocpStat.sEncSrcChan[i].event_cb       = BSP_NULL;
        g_strSocpStat.sEncSrcChan[i].rd_cb          = BSP_NULL;
    }

    for(i=0; i<SOCP_MAX_ENCDST_CHN; i++)
    {
        g_strSocpStat.sEncDstChan[i].u32ChanID      = i;
        g_strSocpStat.sEncDstChan[i].u32Thrh        = 0;
        g_strSocpStat.sEncDstChan[i].u32SetStat     = SOCP_CHN_UNSET;
        g_strSocpStat.sEncDstChan[i].event_cb       = BSP_NULL;
        g_strSocpStat.sEncDstChan[i].read_cb        = BSP_NULL;
        g_strSocpStat.sEncDstChan[i].eChnEvent      = (SOCP_EVENT_ENUM_UIN32)0;
        g_strSocpStat.sEncDstChan[i].struCompress.bcompress = SOCP_NO_COMPRESS;

        memset_s(&g_strSocpStat.sEncDstChan[i].struCompress.ops,sizeof(g_strSocpStat.sEncDstChan[i].struCompress.ops),
              0x0,sizeof(g_strSocpStat.sEncDstChan[i].struCompress.ops));
    }

    for(i=0; i<SOCP_MAX_DECSRC_CHN; i++)
    {
        g_strSocpStat.sDecSrcChan[i].u32ChanID      = i;
        g_strSocpStat.sDecSrcChan[i].u32ChanEn      = SOCP_CHN_DISABLE;
        g_strSocpStat.sDecSrcChan[i].eDataTypeEn    = SOCP_DATA_TYPE_EN_BUTT;
        g_strSocpStat.sDecSrcChan[i].u32SetStat     = SOCP_CHN_UNSET;
        g_strSocpStat.sDecSrcChan[i].event_cb       = BSP_NULL;
        g_strSocpStat.sDecSrcChan[i].rd_cb          = BSP_NULL;
    }

    for(i=0; i<SOCP_MAX_DECDST_CHN; i++)
    {
        g_strSocpStat.sDecDstChan[i].u32ChanID      = i;
        g_strSocpStat.sDecDstChan[i].u32AllocStat   = SOCP_CHN_UNALLOCATED;
        g_strSocpStat.sDecDstChan[i].eDataType      = SOCP_DATA_TYPE_BUTT;
        g_strSocpStat.sDecDstChan[i].event_cb       = BSP_NULL;
        g_strSocpStat.sDecDstChan[i].read_cb        = BSP_NULL;
    }

    return;
}

s32 socp_clk_enable(void)
{

    return BSP_OK;
}


/*****************************************************************************
* 函 数 名  : socp_get_idle_buffer
*
* 功能描述  : 查询空闲缓冲区
*
* 输入参数  :  pRingBuffer       待查询的环形buffer
                    pRWBuffer         输出的环形buffer
*
* 输出参数  : 无
*
* 返 回 值  :  无
*****************************************************************************/
void socp_get_idle_buffer(SOCP_RING_BUF_S *pRingBuffer, SOCP_BUFFER_RW_STRU *pRWBuffer)
{
    	if(pRingBuffer->u32Write < pRingBuffer->u32Read)
    	{
        	/* 读指针大于写指针，直接计算 */
        	pRWBuffer->pBuffer = (char *)(pRingBuffer->Start + (u32)pRingBuffer->u32Write);
        	pRWBuffer->u32Size = (u32)(pRingBuffer->u32Read - pRingBuffer->u32Write - 1);
        	pRWBuffer->pRbBuffer = (char *)BSP_NULL;
        	pRWBuffer->u32RbSize = 0;
    	}
    	else
    	{
        	/* 写指针大于读指针，需要考虑回卷 */
        	if(pRingBuffer->u32Read != 0)
        	{
            		pRWBuffer->pBuffer = (char *)((unsigned long)pRingBuffer->Start + (u32)pRingBuffer->u32Write);
            		pRWBuffer->u32Size = (u32)(pRingBuffer->End - (pRingBuffer->Start + pRingBuffer->u32Write) + 1);
            		pRWBuffer->pRbBuffer = (char *)pRingBuffer->Start;
            		pRWBuffer->u32RbSize = pRingBuffer->u32Read - 1;
        	}
        	else
        	{
            		pRWBuffer->pBuffer = (char *)((unsigned long)pRingBuffer->Start + (u32)pRingBuffer->u32Write);
            		pRWBuffer->u32Size = (u32)(pRingBuffer->End - (pRingBuffer->Start + pRingBuffer->u32Write));
            		pRWBuffer->pRbBuffer = (char *)BSP_NULL;
            		pRWBuffer->u32RbSize = 0;
        	}
    	}

    return;
}

/*****************************************************************************
* 函 数 名  : socp_get_data_buffer
*
* 功能描述  : 获取空闲缓冲区的数据
*
* 输入参数  :  pRingBuffer       待查询的环形buffer
                    pRWBuffer         输出的环形buffer
*
* 输出参数  : 无
*
* 返 回 值  :  无
*****************************************************************************/
void socp_get_data_buffer(SOCP_RING_BUF_S *pRingBuffer, SOCP_BUFFER_RW_STRU *pRWBuffer)
{
    if(pRingBuffer->u32Read <= pRingBuffer->u32Write)
    {
        /* 写指针大于读指针，直接计算 */
        pRWBuffer->pBuffer = (char *)((unsigned long)pRingBuffer->Start + (u32)pRingBuffer->u32Read);
        pRWBuffer->u32Size = (u32)(pRingBuffer->u32Write - pRingBuffer->u32Read);
        pRWBuffer->pRbBuffer = (char *)BSP_NULL;
        pRWBuffer->u32RbSize = 0;
    }
    else
    {
        /* 读指针大于写指针，需要考虑回卷 */
        pRWBuffer->pBuffer = (char *)((unsigned long)pRingBuffer->Start + (u32)pRingBuffer->u32Read);
        pRWBuffer->u32Size = (u32)((unsigned long)pRingBuffer->End - ((unsigned long)pRingBuffer->Start + pRingBuffer->u32Read) + 1);
        pRWBuffer->pRbBuffer = (char *)pRingBuffer->Start;
        pRWBuffer->u32RbSize = pRingBuffer->u32Write;
    }
    return;
}

/*****************************************************************************
* 函 数 名  : socp_write_done
*
* 功能描述  : 更新缓冲区的写指针
*
* 输入参数  :  pRingBuffer       待更新的环形buffer
                    u32Size          更新的数据长度
*
* 输出参数  : 无
*
* 返 回 值  :  无
*****************************************************************************/
void socp_write_done(SOCP_RING_BUF_S *pRingBuffer, u32 u32Size)
{
    u32 tmp_size;
    // pRingBuffer->u32Write = (pRingBuffer->u32Write+u32Size) % pRingBuffer->u32Length;

    tmp_size = (u32)(pRingBuffer->End - (pRingBuffer->Start + pRingBuffer->u32Write) + 1);
    if(tmp_size > u32Size)
    {
    	pRingBuffer->u32Write += u32Size;
    }
    else
    {
    	u32 rb_size = u32Size - tmp_size;
    	pRingBuffer->u32Write =  rb_size;
    }

    return;
}
/*****************************************************************************
* 函 数 名  : socp_read_done
*
* 功能描述  : 更新缓冲区的读指针
*
* 输入参数  :  pRingBuffer       待更新的环形buffer
                    u32Size          更新的数据长度
*
* 输出参数  : 无
*
* 返 回 值  :  无
*****************************************************************************/
void socp_read_done(SOCP_RING_BUF_S *pRingBuffer, u32 u32Size)
{
	pRingBuffer->u32Read += u32Size;
    	if(pRingBuffer->u32Read > (u32)(pRingBuffer->End - pRingBuffer->Start))
    	{
        	pRingBuffer->u32Read -= pRingBuffer->u32Length;
    	}
}

/*****************************************************************************
* 函 数 名  : bsp_socp_clean_encsrc_chan
*
* 功能描述  : 清空编码源通道，同步V9 SOCP接口
*
* 输入参数  : enSrcChanID       编码通道号
*
* 输出参数  : 无
*
* 返 回 值  : BSP_OK
*****************************************************************************/
u32 bsp_socp_clean_encsrc_chan(SOCP_CODER_SRC_ENUM_U32 enSrcChanID)
{
    u32 ulResetFlag;
    u32 i;
    u32 ulChanID;
    u32 ulChanType;
    u32 ret;

    ulChanID    = SOCP_REAL_CHAN_ID(enSrcChanID);
    ulChanType  = SOCP_REAL_CHAN_TYPE(enSrcChanID);

    if((ret=(u32)socp_check_chan_type(ulChanType, SOCP_CODER_SRC_CHAN)) != BSP_OK)
    {
        return ret;
    }

    if((ret=(u32)socp_check_encsrc_chan_id(ulChanID)) != BSP_OK)
    {
        return ret;
    }

    /* 复位通道 */
    SOCP_REG_SETBITS(SOCP_REG_ENCRST, ulChanID, 1, 1);

    /* 等待通道自清 */
    for(i=0; i< SOCP_RESET_TIME; i++)
    {
        ulResetFlag = SOCP_REG_GETBITS(SOCP_REG_ENCRST, ulChanID, 1);
        if(0 == ulResetFlag)
        {
            break;
        }
    }

    if(SOCP_RESET_TIME == i)
    {
        socp_error("Socp src channel[%d] clean failed!\n", ulChanID);
    }

    return BSP_OK;
}



/***************************************************************************************
* 函 数 名  : socp_reset_chan_reg_wr_addr
*
* 功能描述  : socp复位通道地址和读写指针寄存器，内部区分socp32位和64位寻址
*
* 输入参数  : ChanId: 通道号，包括通道类型和通道ID
              type: 通道类型，区分编码通道和解码通道
              Enc_pChan: 编码通道参数结构体
              Dec_pChan: 解码通道参数结构体
*
* 输出参数  : 无
*
* 返 回 值  : 无
****************************************************************************************/
void socp_reset_chan_reg_wr_addr(u32 ChanId, u32 Type, SOCP_ENCSRC_CHAN_S *Enc_pChan, SOCP_DECSRC_CHAN_S *Dec_pChan)
{
    if(Type == SOCP_CODER_SRC_CHAN)   // 编码通道
    {
        SOCP_REG_WRITE(SOCP_REG_ENCSRC_BUFADDR_L(ChanId),(u32)Enc_pChan->sEncSrcBuf.Start);
        SOCP_REG_WRITE(SOCP_REG_ENCSRC_BUFADDR_H(ChanId),(u32)(((u64)Enc_pChan->sEncSrcBuf.Start)>>32));
        SOCP_REG_WRITE(SOCP_REG_ENCSRC_BUFWPTR(ChanId),0);
        SOCP_REG_WRITE(SOCP_REG_ENCSRC_BUFRPTR(ChanId), 0);
         /* 更新读写指针*/
        g_strSocpStat.sEncSrcChan[ChanId].sEncSrcBuf.u32Read  = 0;
        g_strSocpStat.sEncSrcChan[ChanId].sEncSrcBuf.u32Write = 0;

        /* 如果是用链表缓冲区，则配置RDbuffer的起始地址和长度 */
        if(SOCP_ENCSRC_CHNMODE_LIST == Enc_pChan->eChnMode)
        {
            SOCP_REG_WRITE(SOCP_REG_ENCSRC_RDQADDR_L(ChanId),(u32)Enc_pChan->sRdBuf.Start);
            SOCP_REG_WRITE(SOCP_REG_ENCSRC_RDQADDR_H(ChanId),(u32)(((u64)Enc_pChan->sRdBuf.Start)>>32));
            SOCP_REG_WRITE(SOCP_REG_ENCSRC_RDQRPTR(ChanId), 0);
            SOCP_REG_WRITE(SOCP_REG_ENCSRC_RDQWPTR(ChanId), 0);
            /*lint -save -e647*/
            SOCP_REG_SETBITS(SOCP_REG_ENCSRC_RDQCFG(ChanId), 0, 16, Enc_pChan->sRdBuf.u32Length);
            SOCP_REG_SETBITS(SOCP_REG_ENCSRC_RDQCFG(ChanId), 16, 16, 0);
            /*lint -restore +e647*/
            /* 更新读写指针*/
            g_strSocpStat.sEncSrcChan[ChanId].sRdBuf.u32Read  = 0;
            g_strSocpStat.sEncSrcChan[ChanId].sRdBuf.u32Write = 0;
        }
    }

    else if(Type == SOCP_DECODER_SRC_CHAN)   // 解码通道
    {
    	/* 使用配置参数进行配置 */
    	SOCP_REG_WRITE(SOCP_REG_DECSRC_BUFADDR_L(ChanId), (u32)Dec_pChan->sDecSrcBuf.Start);
    	SOCP_REG_WRITE(SOCP_REG_DECSRC_BUFADDR_H(ChanId), (u32)(((u64)Dec_pChan->sDecSrcBuf.Start)>>32));
    	SOCP_REG_WRITE(SOCP_REG_DECSRC_BUFWPTR(ChanId), 0);
    	SOCP_REG_WRITE(SOCP_REG_DECSRC_BUFRPTR(ChanId), 0);
    	/* 更新对应通道的读写指针*/
    	g_strSocpStat.sDecSrcChan[ChanId].sDecSrcBuf.u32Read  = 0;
    	g_strSocpStat.sDecSrcChan[ChanId].sDecSrcBuf.u32Write = 0;
    }

}


/*****************************************************************************
* 函 数 名  : socp_reset_enc_chan
*
* 功能描述  : 复位编码通道
*
* 输入参数  : u32ChanID       编码通道号
*
* 输出参数  : 无
*
* 返 回 值  : 释放成功与否的标识码
*****************************************************************************/
s32 socp_reset_enc_chan(u32 u32ChanID)
{
    u32  ResetFlag;
    u32  i;
    SOCP_ENCSRC_CHAN_S *pChan;

    pChan = &g_strSocpStat.sEncSrcChan[u32ChanID];

    /* 复位通道 */
    SOCP_REG_SETBITS(SOCP_REG_ENCRST, u32ChanID, 1, 1);

    /* 等待通道自清 */
    for(i=0; i<SOCP_RESET_TIME; i++)
    {
        ResetFlag = SOCP_REG_GETBITS(SOCP_REG_ENCRST, u32ChanID, 1);
        if(0 == ResetFlag)
        {
            break;
        }

        if(SOCP_RESET_TIME == i)
        {
            socp_error("socp channel[%d] reset failed!\n", u32ChanID);
        }
    }

    socp_reset_chan_reg_wr_addr(u32ChanID, SOCP_CODER_SRC_CHAN, pChan, NULL);

    /*配置其它参数*/
	/*lint -save -e647*/
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG1(u32ChanID), 1, 2, pChan->eChnMode);
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG1(u32ChanID), 4, 4, pChan->u32DestChanID);
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG1(u32ChanID), 8, 2, pChan->ePriority);
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG1(u32ChanID), 10, 1, pChan->u32BypassEn);
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG1(u32ChanID), 16, 8, pChan->eDataType);
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG1(u32ChanID), 11, 1, pChan->eDataTypeEn);
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG1(u32ChanID), 31, 1, pChan->eDebugEn);

    /*如果通道是启动状态，使能通道*/
    if(pChan->u32ChanEn)
    {
        SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG1(u32ChanID), 0, 1, 1);
    }
	/*lint -restore +e647*/
    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : socp_reset_dec_chan
*
* 功能描述  : 复位解码通道
*
* 输入参数  : u32ChanID       解码通道号
*
* 输出参数  : 无
*
* 返 回 值  : 释放成功与否的标识码
*****************************************************************************/
s32 socp_reset_dec_chan(u32 u32ChanID)
{
    u32  u32ResetFlag;
    u32  i;
    SOCP_DECSRC_CHAN_S *pChan;

    if(u32ChanID >= SOCP_MAX_DECSRC_CHN)
    {
        return BSP_ERROR;
    }

    pChan = &g_strSocpStat.sDecSrcChan[u32ChanID];

    /* 复位通道 */
    SOCP_REG_SETBITS(SOCP_REG_DECRST, u32ChanID, 1, 1);

    /* 等待通道自清 */
    for(i=0; i<SOCP_RESET_TIME; i++)
    {
        u32ResetFlag = SOCP_REG_GETBITS(SOCP_REG_DECRST, u32ChanID, 1);
        if(0 == u32ResetFlag)
        {
            break;
        }
        if((SOCP_RESET_TIME -1) == i)
        {
            socp_error("socp_reset_dec_chan 0x%x failed!\n", u32ChanID);
        }
    }

    socp_reset_chan_reg_wr_addr(u32ChanID, SOCP_DECODER_SRC_CHAN, NULL, pChan);
	/*lint -save -e647*/
    SOCP_REG_SETBITS(SOCP_REG_DECSRC_BUFCFG0(u32ChanID), 0, 16, pChan->sDecSrcBuf.u32Length);
    SOCP_REG_SETBITS(SOCP_REG_DECSRC_BUFCFG0(u32ChanID), 31, 1, pChan->eDataTypeEn);
	/*lint -restore +e647*/
    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : socp_soft_free_encdst_chan
*
* 功能描述  : 软释放编码目的通道
*
* 输入参数  : u32EncDstChanId       编码通道号
*
* 输出参数  : 无
*
* 返 回 值  : 释放成功与否的标识码
*****************************************************************************/
s32 socp_soft_free_encdst_chan(u32 u32EncDstChanId)
{
    u32 u32ChanID;
    u32 u32ChanType;
    SOCP_ENCDST_CHAN_S *pChan;
    u32 ret;

    u32ChanID   = SOCP_REAL_CHAN_ID(u32EncDstChanId);
    u32ChanType = SOCP_REAL_CHAN_TYPE(u32EncDstChanId);

    if((ret=socp_check_chan_type(u32ChanType, SOCP_CODER_DEST_CHAN)) != BSP_OK)
    {
        return ret;
    }
    if((ret=socp_check_chan_id(u32ChanID, SOCP_MAX_ENCDST_CHN)) != BSP_OK)
    {
        return ret;
    }

    pChan = &g_strSocpStat.sEncDstChan[u32ChanID];

    /* 写入起始地址到目的buffer起始地址寄存器*/
    SOCP_REG_WRITE(SOCP_REG_ENCDEST_BUFADDR_L(u32ChanID), (u32)pChan->sEncDstBuf.Start);
	SOCP_REG_WRITE(SOCP_REG_ENCDEST_BUFADDR_H(u32ChanID), (u32)(((u64)pChan->sEncDstBuf.Start)>>32));
    SOCP_REG_WRITE(SOCP_REG_ENCDEST_BUFRPTR(u32ChanID), 0);
    SOCP_REG_WRITE(SOCP_REG_ENCDEST_BUFWPTR(u32ChanID), 0);

    g_strSocpStat.sEncDstChan[u32ChanID].sEncDstBuf.u32Write = 0; /* [false alarm]:u32ChanID已经在函数入口处经过判断 */
    g_strSocpStat.sEncDstChan[u32ChanID].sEncDstBuf.u32Read = 0;/* [false alarm]:u32ChanID已经在函数入口处经过判断 */


    g_strSocpStat.sEncDstChan[u32ChanID].u32SetStat = SOCP_CHN_UNSET;/* [false alarm]:u32ChanID已经在函数入口处经过判断 */


    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : socp_soft_free_decsrc_chan
*
* 功能描述  : 软释放解码源通道
*
* 输入参数  : u32DecSrcChanId       解码通道号
*
* 输出参数  : 无
*
* 返 回 值  : 释放成功与否的标识码
*****************************************************************************/
s32 socp_soft_free_decsrc_chan(u32 u32DecSrcChanId)
{
    u32 u32ChanID;
    u32 u32ChanType;
    SOCP_DECSRC_CHAN_S *pDecSrcChan;
    u32 ret;

    u32ChanID   = SOCP_REAL_CHAN_ID(u32DecSrcChanId);
    u32ChanType = SOCP_REAL_CHAN_TYPE(u32DecSrcChanId);

    if((ret=socp_check_chan_type(u32ChanType, SOCP_DECODER_SRC_CHAN)) != BSP_OK)
    {
        return ret;
    }
    if((ret=socp_check_chan_id(u32ChanID, SOCP_MAX_DECSRC_CHN)) != BSP_OK)
    {
        return ret;
    }

    pDecSrcChan = &g_strSocpStat.sDecSrcChan[u32ChanID];

    	/* 写入起始地址到目的buffer起始地址寄存器*/
   	SOCP_REG_WRITE(SOCP_REG_DECSRC_BUFWPTR(u32ChanID), 0);
    SOCP_REG_WRITE(SOCP_REG_DECSRC_BUFADDR_L(u32ChanID), (u32)pDecSrcChan->sDecSrcBuf.Start);
    SOCP_REG_WRITE(SOCP_REG_DECSRC_BUFADDR_H(u32ChanID), (u32)(((u64)pDecSrcChan->sDecSrcBuf.Start)>>32));
    SOCP_REG_WRITE(SOCP_REG_DECSRC_BUFRPTR(u32ChanID), 0);

    g_strSocpStat.sDecSrcChan[u32ChanID].sDecSrcBuf.u32Write = 0;/* [false alarm]:u32ChanID已经在函数入口处经过判断 */
    g_strSocpStat.sDecSrcChan[u32ChanID].sDecSrcBuf.u32Read = 0;/* [false alarm]:u32ChanID已经在函数入口处经过判断 */

    g_strSocpStat.sDecSrcChan[u32ChanID].u32SetStat = SOCP_CHN_UNSET;/* [false alarm]:u32ChanID已经在函数入口处经过判断 */

    return BSP_OK;
}

/* cov_verified_start */
/*****************************************************************************
* 函 数 名  : socp_get_enc_rd_size
*
* 功能描述  :  获取编码源通道RDbuffer
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 释放成功与否的标识码
*****************************************************************************/
u32 socp_get_enc_rd_size(u32 u32ChanID)
{
    SOCP_BUFFER_RW_STRU Buff;
    u32          PAddr;

    SOCP_REG_READ(SOCP_REG_ENCSRC_RDQRPTR(u32ChanID), PAddr);
    g_strSocpStat.sEncSrcChan[u32ChanID].sRdBuf.u32Read = PAddr;
    SOCP_REG_READ(SOCP_REG_ENCSRC_RDQWPTR(u32ChanID), PAddr);
    g_strSocpStat.sEncSrcChan[u32ChanID].sRdBuf.u32Write= PAddr;

    socp_get_data_buffer(&g_strSocpStat.sEncSrcChan[u32ChanID].sRdBuf, &Buff);
    return (Buff.u32Size + Buff.u32RbSize);
}

/*****************************************************************************
* 函 数 名   : socp_encsrc_rd_handler
*
* 功能描述  :  编码源通道RDbuffer中断处理函数
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 无
*****************************************************************************/
void socp_encsrc_rd_handler(u32 RdSize, u32 i)
{
    u32 u32ChanId;

    if (RdSize == g_strSocpStat.sEncSrcChan[i].u32LastRdSize)
    {
        if (g_strSocpStat.sEncSrcChan[i].rd_cb)
        {
            u32ChanId = SOCP_CHAN_DEF(SOCP_CODER_SRC_CHAN, i);
            (void)g_strSocpStat.sEncSrcChan[i].rd_cb(u32ChanId);

            g_stSocpDebugInfo.sSocpDebugEncSrc.u32SocpEncSrcTskRdCbCnt[i]++;
        }

        g_strSocpStat.sEncSrcChan[i].u32LastRdSize = 0;
    }
    else
    {
        g_strSocpStat.sEncSrcChan[i].u32LastRdSize = RdSize;
    }

    return;
}
/* cov_verified_stop */

/*****************************************************************************
* 函 数 名  : socp_encsrc_task
*
* 功能描述  : 模块任务函数:编码源中断，双核
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 无
*****************************************************************************/
int socp_encsrc_task(void * data)
{
    u32 i;
    u32 IntHeadState = 0;
    u32 u32ChanId;
    unsigned long lock_flag;
    /* coverity[no_escape] */
    do{
        /* 超时或者被中断，非正常返回 */
        if(0 != down_interruptible(&g_strSocpStat.u32EncSrcSemID))
        {
            continue;
        }

        spin_lock_irqsave(&lock, lock_flag);
        IntHeadState = g_strSocpStat.u32IntEncSrcHeader;
        g_strSocpStat.u32IntEncSrcHeader = 0;
        g_strSocpStat.u32IntEncSrcRD = 0;
        spin_unlock_irqrestore(&lock, lock_flag);

        /* 处理编码包头'HISI'检验错误*/
        if (IntHeadState)
        {
            for (i = 0; i < SOCP_MAX_ENCSRC_CHN; i++)
            {
                /* 检测通道是否申请*/
                if (SOCP_CHN_ALLOCATED == g_strSocpStat.sEncSrcChan[i].u32AllocStat)
                {
                    if (IntHeadState & ((u32)1 << i))
                    {
                    	socp_crit("EncSrcHeaderError ChanId = %d",i);						
                        if(g_strSocpStat.sEncSrcChan[i].event_cb)
                        {
                            g_stSocpDebugInfo.sSocpDebugEncSrc.u32SocpEncSrcTskHeadCbOriCnt[i]++;
                            u32ChanId = SOCP_CHAN_DEF(SOCP_CODER_SRC_CHAN, i);
                            (void)g_strSocpStat.sEncSrcChan[i].event_cb(u32ChanId, SOCP_EVENT_PKT_HEADER_ERROR, 0);
                            g_stSocpDebugInfo.sSocpDebugEncSrc.u32SocpEncSrcTskHeadCbCnt[i]++;
                        }
                    }
                }
            }
        }
    } while (1);

    //return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : socp_encdst_task
*
* 功能描述  : 模块任务函数:编码目的，App核
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 无
*****************************************************************************/
int socp_encdst_task(void * data)
{
    u32 i;
    u32 IntTfrState = 0;
    u32 IntOvfState = 0;
    u32 IntThresholdOvfState = 0;
    u32 u32ChanId =0 ;
    unsigned long lock_flag;
    u32 read;
    u32 write;

    /* coverity[no_escape] */
    do{
        /* 超时或者被中断，非正常返回 */
        if(0 != down_interruptible(&g_strSocpStat.u32EncDstSemID))
        {
            continue;
        }

        spin_lock_irqsave(&lock, lock_flag);
        IntTfrState = g_strSocpStat.u32IntEncDstTfr;
        g_strSocpStat.u32IntEncDstTfr = 0;
        IntOvfState = g_strSocpStat.u32IntEncDstOvf;
        g_strSocpStat.u32IntEncDstOvf = 0;
        IntThresholdOvfState = g_strSocpStat.u32IntEncDstThresholdOvf;
        g_strSocpStat.u32IntEncDstThresholdOvf = 0;
        spin_unlock_irqrestore(&lock, lock_flag);

        /* 处理编码传输完成中断*/
        if (IntTfrState)
        {
            for (i = 0; i < SOCP_MAX_ENCDST_CHN; i++)
            {
                /* 检测通道是否配置*/
                if (SOCP_CHN_SET == g_strSocpStat.sEncDstChan[i].u32SetStat)
                {
                    if (IntTfrState & ((u32)1 << i))
                    {
                        if (g_strSocpStat.sEncDstChan[i].read_cb)
                        {
                            /*lint -save -e732*/
                            SOCP_REG_READ(SOCP_REG_ENCDEST_BUFRPTR(i), read);
                            SOCP_REG_READ(SOCP_REG_ENCDEST_BUFWPTR(i), write);
                            /*lint -restore +e732*/
                            SOCP_DEBUG_TRACE(SOCP_DEBUG_READ_DONE, read, write, 0, 0);
                            g_stSocpDebugInfo.sSocpDebugEncDst.u32SocpEncDstTskTrfCbOriCnt[i]++;
                            u32ChanId = SOCP_CHAN_DEF(SOCP_CODER_DEST_CHAN, i);
                            if(i == 1){
                                g_stEncDstStat[g_ulEncDstStatCount].ulTaskStartSlice = bsp_get_slice_value();
                            }
                            (void)g_strSocpStat.sEncDstChan[i].read_cb(u32ChanId);
                            if(i == 1){
                                g_stEncDstStat[g_ulEncDstStatCount].ulTaskEndSlice = bsp_get_slice_value();
                            }

                            g_stSocpDebugInfo.sSocpDebugEncDst.u32SocpEncDstTskTrfCbCnt[i]++;
                        }
                    }
                }
            }
        }

        /* 处理编码目的 buffer 溢出中断*/
        if (IntOvfState)
        {
            for (i = 0; i < SOCP_MAX_ENCDST_CHN; i++)
            {
                /* 检测通道是否配置*/
                if (SOCP_CHN_SET == g_strSocpStat.sEncDstChan[i].u32SetStat)
                {
                    if (IntOvfState & ((u32)1 << i))
                    {
                        if (g_strSocpStat.sEncDstChan[i].event_cb)
                        {
                            g_stSocpDebugInfo.sSocpDebugEncDst.u32SocpEncDstTskOvfCbOriCnt[i]++;
                            u32ChanId = SOCP_CHAN_DEF(SOCP_CODER_DEST_CHAN, i);
                            (void)g_strSocpStat.sEncDstChan[i].event_cb(u32ChanId, SOCP_EVENT_OUTBUFFER_OVERFLOW, 0);

                            g_stSocpDebugInfo.sSocpDebugEncDst.u32SocpEncDstTskOvfCbCnt[i]++;
                        }
                        if (g_strSocpStat.sEncDstChan[i].read_cb)
                        {
                            /*lint -save -e732*/
                            SOCP_REG_READ(SOCP_REG_ENCDEST_BUFRPTR(i), read);
                            SOCP_REG_READ(SOCP_REG_ENCDEST_BUFWPTR(i), write);
                            /*lint -restore +e732*/
                            SOCP_DEBUG_TRACE(SOCP_DEBUG_READ_DONE, read, write, 0, 0);
                            g_stSocpDebugInfo.sSocpDebugEncDst.u32SocpEncDstTskTrfCbOriCnt[i]++;
                            u32ChanId = SOCP_CHAN_DEF(SOCP_CODER_DEST_CHAN, i);
                            (void)g_strSocpStat.sEncDstChan[i].read_cb(u32ChanId);

                            g_stSocpDebugInfo.sSocpDebugEncDst.u32SocpEncDstTskTrfCbCnt[i]++;
                        }
                    }
                }
            }
        }

        /* 处理编码目的 buffer 阈值溢出中断*/
        if (IntThresholdOvfState)
        {
            for (i = 0; i < SOCP_MAX_ENCDST_CHN; i++)
            {
                /* 检测通道是否配置*/
                if (SOCP_CHN_SET == g_strSocpStat.sEncDstChan[i].u32SetStat)
                {
                    if (IntThresholdOvfState & ((u32)1 << (i + SOCP_ENC_DST_BUFF_THRESHOLD_OVF_BEGIN)))
                    {
                        if (g_strSocpStat.sEncDstChan[i].event_cb)
                        {
                            g_stSocpDebugInfo.sSocpDebugEncDst.u32SocpEncDstTskOvfCbOriCnt[i]++;
                            u32ChanId = SOCP_CHAN_DEF(SOCP_CODER_DEST_CHAN, i);
                            (void)g_strSocpStat.sEncDstChan[i].event_cb(u32ChanId, SOCP_EVENT_OUTBUFFER_THRESHOLD_OVERFLOW, 0);

                            g_stSocpDebugInfo.sSocpDebugEncDst.u32SocpEncDstTskOvfCbCnt[i]++;
                        }
                        if (g_strSocpStat.sEncDstChan[i].read_cb)
                        {
                            /*lint -save -e732*/
                            SOCP_REG_READ(SOCP_REG_ENCDEST_BUFRPTR(i), read);
                            SOCP_REG_READ(SOCP_REG_ENCDEST_BUFWPTR(i), write);
                            /*lint -restore +e732*/
                            SOCP_DEBUG_TRACE(SOCP_DEBUG_READ_DONE, read, write, 0, 0);
                            g_stSocpDebugInfo.sSocpDebugEncDst.u32SocpEncDstTskTrfCbOriCnt[i]++;
                            u32ChanId = SOCP_CHAN_DEF(SOCP_CODER_DEST_CHAN, i);
                            (void)g_strSocpStat.sEncDstChan[i].read_cb(u32ChanId);

                            g_stSocpDebugInfo.sSocpDebugEncDst.u32SocpEncDstTskTrfCbCnt[i]++;
                        }
                    }
                }
            }
        }

    } while (1);

    //return BSP_OK;
}

/* cov_verified_start */
/*****************************************************************************
* 函 数 名  : socp_decsrc_event_handler
*
* 功能描述  : 解码源通道事件处理函数
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 无
*****************************************************************************/
void  socp_decsrc_event_handler(u32 id, u32 secIntState)
{
    u32 u32ChanId = SOCP_CHAN_DEF(SOCP_DECODER_SRC_CHAN, id);

    if (g_strSocpStat.sDecSrcChan[id].event_cb)
    {
        if (secIntState & 0x10)
        {
            g_stSocpDebugInfo.sSocpDebugDecSrc.u32SocpDecSrcTskErrCbOriCnt[id]++;

            (void)g_strSocpStat.sDecSrcChan[id].event_cb(u32ChanId, SOCP_EVENT_DECODER_UNDERFLOW, 0);

            g_stSocpDebugInfo.sSocpDebugDecSrc.u32SocpDecSrcTskErrCbCnt[id]++;
        }

        if (secIntState & 0x100)
        {
            g_stSocpDebugInfo.sSocpDebugDecSrc.u32SocpDecSrcTskErrCbOriCnt[id]++;

            (void)g_strSocpStat.sDecSrcChan[id].event_cb(u32ChanId, SOCP_EVENT_HDLC_HEADER_ERROR, 0);

            g_stSocpDebugInfo.sSocpDebugDecSrc.u32SocpDecSrcTskErrCbCnt[id]++;
        }

        if (secIntState & 0x1000)
        {
            g_stSocpDebugInfo.sSocpDebugDecSrc.u32SocpDecSrcTskErrCbOriCnt[id]++;

            (void)g_strSocpStat.sDecSrcChan[id].event_cb(u32ChanId, SOCP_EVENT_DATA_TYPE_ERROR, 0);

            g_stSocpDebugInfo.sSocpDebugDecSrc.u32SocpDecSrcTskErrCbCnt[id]++;
        }

        if (secIntState & 0x10000)
        {
            g_stSocpDebugInfo.sSocpDebugDecSrc.u32SocpDecSrcTskErrCbOriCnt[id]++;

            (void)g_strSocpStat.sDecSrcChan[id].event_cb(u32ChanId, SOCP_EVENT_CRC_ERROR, 0);

            g_stSocpDebugInfo.sSocpDebugDecSrc.u32SocpDecSrcTskErrCbCnt[id]++;
        }

        if (secIntState & 0x100000)
        {
            g_stSocpDebugInfo.sSocpDebugDecSrc.u32SocpDecSrcTskErrCbOriCnt[id]++;

            (void)g_strSocpStat.sDecSrcChan[id].event_cb(u32ChanId, SOCP_EVENT_PKT_LENGTH_ERROR, 0);

            g_stSocpDebugInfo.sSocpDebugDecSrc.u32SocpDecSrcTskErrCbCnt[id]++;
        }
    }
}

/*****************************************************************************
* 函 数 名  : socp_decsrc_handler
*
* 功能描述  : 解码源通道中断处理函数
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 无
*****************************************************************************/
void  socp_decsrc_handler(void)
{
    u32 IntState, secIntState;
    u32 u32ChanId;
    u32 i;

    if(g_strSocpStat.u32IntDecSrcErr)
    {
        IntState = g_strSocpStat.u32IntDecSrcErr;
        g_strSocpStat.u32IntDecSrcErr = 0;

        for(i=0;i<SOCP_MAX_DECSRC_CHN;i++)
        {
            /* 检测通道是否配置*/

            if(SOCP_CHN_SET == g_strSocpStat.sDecSrcChan[i].u32SetStat)
            {
                secIntState = IntState>>i;
                u32ChanId = SOCP_CHAN_DEF(SOCP_DECODER_SRC_CHAN, i);

                if(g_strSocpStat.sDecSrcChan[i].rd_cb)
                {
                    if(secIntState & 0x1)
                    {
                        (void)g_strSocpStat.sDecSrcChan[i].rd_cb(u32ChanId);
                    }
                }

                socp_decsrc_event_handler(i, secIntState);
            }
        }
    }
}
/* cov_verified_stop */

/*****************************************************************************
* 函 数 名  : socp_decsrc_task
*
* 功能描述  : 模块任务函数:解码源，A核
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 无
*****************************************************************************/
int socp_decsrc_task(void * data)
{
    unsigned long lock_flag;
    /* coverity[no_escape] */
    do{
        /* 超时或者被中断，非正常返回 */
        if(0 != down_interruptible(&g_strSocpStat.u32DecSrcSemID))
        {
            continue;
        }
        spin_lock_irqsave(&lock, lock_flag);
        /* 处理解码源中断*/
        socp_decsrc_handler();
        spin_unlock_irqrestore(&lock, lock_flag);
    }while(1);

    //return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : socp_decdst_task
*
* 功能描述  : 模块任务函数:解码目的，双核
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 无
*****************************************************************************/
int socp_decdst_task(void * data)
{
    u32 i;
    u32 IntTfrState = 0;
    u32 IntOvfState = 0;
    u32 u32ChanId;
    unsigned long lock_flag;
    /* coverity[no_escape] */
    do{
        /* 超时或者被中断，非正常返回 */
        if(0 != down_interruptible(&g_strSocpStat.u32DecDstSemID))
        {
            continue;
        }

        spin_lock_irqsave(&lock, lock_flag);
        IntTfrState = g_strSocpStat.u32IntDecDstTfr;
        g_strSocpStat.u32IntDecDstTfr = 0;
        IntOvfState = g_strSocpStat.u32IntDecDstOvf;
        g_strSocpStat.u32IntDecDstOvf = 0;
        spin_unlock_irqrestore(&lock, lock_flag);

        /* 处理解码传输完成中断*/
        if (IntTfrState)
        {
            for (i = 0; i < SOCP_MAX_DECDST_CHN; i++)
            {
                /* 检测通道是否申请*/
                if (SOCP_CHN_ALLOCATED == g_strSocpStat.sDecDstChan[i].u32AllocStat)
                {
                    if (IntTfrState & ((u32)1 << i))
                    {
                        if (g_strSocpStat.sDecDstChan[i].read_cb)
                        {
                            g_stSocpDebugInfo.sSocpDebugDecDst.u32SocpDecDstTskTrfCbOriCnt[i]++;

                            u32ChanId = SOCP_CHAN_DEF(SOCP_DECODER_DEST_CHAN, i);
                            (void)g_strSocpStat.sDecDstChan[i].read_cb(u32ChanId);

                            g_stSocpDebugInfo.sSocpDebugDecDst.u32SocpDecDstTskTrfCbCnt[i]++;
                        }
                    }
                }
            }
        }

        /* 处理解码目的 buffer 溢出中断*/
        if (IntOvfState)
        {
            for (i = 0; i < SOCP_MAX_DECDST_CHN; i++)
            {
                /* 检测通道是否申请*/
                if (SOCP_CHN_ALLOCATED == g_strSocpStat.sDecDstChan[i].u32AllocStat)
                {
                    if (IntOvfState & ((u32)1 << i))
                    {
                        if (g_strSocpStat.sDecDstChan[i].event_cb)
                        {
                            g_stSocpDebugInfo.sSocpDebugDecDst.u32SocpDecDstTskOvfCbOriCnt[i]++;

                            u32ChanId = SOCP_CHAN_DEF(SOCP_DECODER_DEST_CHAN, i);
                            (void)g_strSocpStat.sDecDstChan[i].event_cb(u32ChanId, SOCP_EVENT_OUTBUFFER_OVERFLOW, 0);

                            g_stSocpDebugInfo.sSocpDebugDecDst.u32SocpDecDstTskOvfCbCnt[i]++;
                        }
                    }
                }
            }
        }
    } while (1);

    //return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : socp_create_task
*
* 功能描述  : socp任务创建函数
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 创建成功与否的标识码
*****************************************************************************/
s32 socp_create_task( s8 * puchName,
                        unsigned long * pu32TaskID,
                        socp_task_entry pfnFunc,
                        u32 u32Priority,
                        u32 u32StackSize,
                        void * pParam)
{
    struct task_struct  *tsk;
    struct sched_param  param;

    tsk = kthread_run(pfnFunc, pParam, puchName);
    if (IS_ERR(tsk))
    {
        socp_error("create kthread failed!\n");
        return BSP_ERROR;
    }

    param.sched_priority = u32Priority;
    if (BSP_OK != sched_setscheduler(tsk, SCHED_FIFO, &param))
    {
        socp_error("Creat Task %s sched_setscheduler Error\n", puchName);
        return BSP_ERROR;
    }

    *pu32TaskID = (unsigned long)tsk;

    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : socp_init_task
*
* 功能描述  : 创建编解码任务
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 创建成功与否的标识码
*****************************************************************************/
s32 socp_init_task(void)
{
    u32 aulArguments[4] = {0,0,0,0};

    /* 编码源通道任务*/
    sema_init(&g_strSocpStat.u32EncSrcSemID, 0);
    if(!g_strSocpStat.u32EncSrcTskID)
    {
        if ( BSP_OK != socp_create_task( "EncSrc", (unsigned long *)&g_strSocpStat.u32EncSrcTskID, (socp_task_entry)socp_encsrc_task,
                            SOCP_ENCSRC_TASK_PRO, 0x1000, aulArguments) )
        {
            socp_error("create socp_encsrc_task failed.\n");
            return BSP_ERR_SOCP_TSK_CREATE;
        }
    }

    /* 编码输出通道任务*/
    sema_init(&g_strSocpStat.u32EncDstSemID, 0);
    if(!g_strSocpStat.u32EncDstTskID)
    {
        if ( BSP_OK != socp_create_task( "EncDst", (unsigned long *)&g_strSocpStat.u32EncDstTskID, (socp_task_entry)socp_encdst_task,
                            SOCP_ENCDST_TASK_PRO, 0x1000, aulArguments) )
        {
            socp_error("create socp_encdst_task failed.\n");
            return BSP_ERR_SOCP_TSK_CREATE;
        }
    }

    /* 解码源通道任务*/
    sema_init(&g_strSocpStat.u32DecSrcSemID, 0);
    if(!g_strSocpStat.u32DecSrcTskID)
    {
        if ( BSP_OK != socp_create_task( "DecSrc", (unsigned long *)&g_strSocpStat.u32DecSrcTskID, (socp_task_entry)socp_decsrc_task,
                            SOCP_DECSRC_TASK_PRO, 0x1000, aulArguments) )
        {
            socp_error("create socp_DecSrc_task failed.\n");
            return BSP_ERR_SOCP_TSK_CREATE;
        }
    }

    /* 解码目的通道任务*/
    sema_init(&g_strSocpStat.u32DecDstSemID, 0);
    if(!g_strSocpStat.u32DecDstTskID)
    {
        if ( BSP_OK != socp_create_task( "DecDst", (unsigned long *)&g_strSocpStat.u32DecDstTskID, (socp_task_entry)socp_decdst_task,
                            SOCP_DECDST_TASK_PRO, 0x1000, aulArguments) )
        {
            socp_error("create socp_DecDst_task failed.\n");
            return BSP_ERR_SOCP_TSK_CREATE;
        }
    }

    return BSP_OK;
}

/*****************************************************************************
* 函 数 名   : socp_handler_encsrc
*
* 功能描述  : 编码源通道处理函数，RD处理由上层完成，驱动RD中断可以不做处理
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 无
*****************************************************************************/
void  socp_handler_encsrc(void)
{
    u32 IntFlag   = 0;
    u32 IntState  = 0;
    int bHandle  = BSP_FALSE;
    u32 i = 0;

    /*read and clear the interrupt flags*/
    SOCP_REG_READ(SOCP_REG_GBL_INTSTAT, IntFlag);
    /* 处理包头错误 */
    if (IntFlag & SOCP_APP_ENC_FLAGINT_MASK)
    {
        SOCP_REG_READ(SOCP_REG_APP_INTSTAT1, IntState);
        SOCP_REG_WRITE(SOCP_REG_ENC_RAWINT1, IntState);

        g_strSocpStat.u32IntEncSrcHeader |= IntState;
        bHandle = BSP_TRUE;

        for (i = 0; i < SOCP_MAX_ENCSRC_CHN; i++)
        {
            if (IntState & ((u32)1 << i))
            {
                /* debug模式屏蔽包头错误中断 */
                if(SOCP_REG_GETBITS(SOCP_REG_ENCSRC_BUFCFG1(i), 31, 1))/*lint !e647*/
                {
                    SOCP_REG_SETBITS(SOCP_REG_APP_MASK1, i, 1,1);
                }
                g_stSocpDebugInfo.sSocpDebugEncSrc.u32SocpEncSrcIsrHeadIntCnt[i]++;
            }
        }
    }

    /*不再处理RD完成中断，初始化时保持屏蔽 */

    if(bHandle)
    {
        up(&g_strSocpStat.u32EncSrcSemID);
    }

    return ;
}


/*****************************************************************************
* 函 数 名   : socp_handler_encdst
*
* 功能描述  : 编码目的中断处理函数
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 无
*****************************************************************************/
/*lint -save -e550*/
void socp_handler_encdst(void)
{
    u32  IntFlag = 0;
    u32  IntState = 0;
    int  bHandle = BSP_FALSE;
    u32  i;
    u32  mask;
    u32  mask2;
    u32  write;

    unsigned long lock_flag;
    int countFlag = BSP_FALSE;
    u32  ModeState = 0;

    /*编码目的传输中断*/
    SOCP_REG_READ(SOCP_REG_GBL_INTSTAT, IntFlag);
    if (IntFlag & SOCP_APP_ENC_TFRINT_MASK)
    {
        spin_lock_irqsave(&lock, lock_flag);
        SOCP_REG_READ(SOCP_REG_ENC_INTSTAT0, IntState);
        SOCP_REG_READ(SOCP_REG_ENC_MASK0, mask);
        SOCP_REG_WRITE(SOCP_REG_ENC_MASK0, (IntState | mask));   // mask int 2011.7.27 by yangzhi
        /* 屏蔽溢出中断 */
        SOCP_REG_READ(SOCP_REG_ENC_MASK2, mask2);
        SOCP_REG_WRITE(SOCP_REG_ENC_MASK2, ((IntState << 16) | mask2));
        SOCP_REG_WRITE(SOCP_REG_ENC_RAWINT0, IntState);

        g_strSocpStat.u32IntEncDstTfr |= IntState;
        spin_unlock_irqrestore(&lock, lock_flag);
        bHandle = BSP_TRUE;

        for (i = 0; i < SOCP_MAX_ENCDST_CHN; i++)
        {
            if (IntState & ((u32)1 << i))
            {
                if(g_ulThrowout == 0x5a5a5a5a) /*仅在压测中使用*/
                {
                    spin_lock_irqsave(&lock, lock_flag);
                    SOCP_REG_READ(SOCP_REG_ENCDEST_BUFWPTR(i), write);
                    SOCP_REG_WRITE(SOCP_REG_ENCDEST_BUFRPTR(i), write);
                    SOCP_REG_SETBITS(SOCP_REG_ENC_RAWINT0, i, 1, 1);
                    SOCP_REG_SETBITS(SOCP_REG_ENC_MASK0, i, 1, 0);
                    /* overflow int */
                    SOCP_REG_SETBITS(SOCP_REG_ENC_RAWINT2, i + 16, 1, 1);
                    SOCP_REG_SETBITS(SOCP_REG_ENC_MASK2, i + 16, 1, 0);
                    spin_unlock_irqrestore(&lock, lock_flag);

                    bHandle = BSP_FALSE;
                }
                if(i == 1)
                {
                    g_stEncDstStat[g_ulEncDstStatCount].ulIntStartSlice = bsp_get_slice_value();
                    countFlag = BSP_TRUE;
                }
                g_stSocpDebugInfo.sSocpDebugEncDst.u32SocpEncDstIsrTrfIntCnt[i]++;
            }
        }
    }
    // 上溢中断与阈值中断共用一个寄存器
    else if (IntFlag & SOCP_APP_ENC_OUTOVFINT_MASK)
    {
        SOCP_REG_READ(SOCP_REG_ENC_INTSTAT2, IntState);
        // 编码目的buffer阈值中断处理
        if(0 != (IntState & SOCP_ENC_DST_BUFF_THRESHOLD_OVF_MASK))
        {
            spin_lock_irqsave(&lock, lock_flag);
            SOCP_REG_READ(SOCP_REG_ENC_MASK2, mask);
            SOCP_REG_WRITE(SOCP_REG_ENC_MASK2, (IntState | mask));
            SOCP_REG_WRITE(SOCP_REG_ENC_RAWINT2, IntState);
            g_strSocpStat.u32IntEncDstThresholdOvf |= IntState;
            spin_unlock_irqrestore(&lock, lock_flag);

            bHandle = BSP_TRUE;

            for (i = 0; i < SOCP_MAX_ENCDST_CHN; i++)
            {
                if (IntState & ((u32)1 << (i + SOCP_ENC_DST_BUFF_THRESHOLD_OVF_BEGIN)))
                {
                    g_stSocpDebugInfo.sSocpDebugEncDst.u32SocpEncDstIsrThresholdOvfIntCnt[i]++;
                }
            }

        }
        // 编码目的buffer上溢中断
        if (0 != (IntState & SOCP_ENC_DST_BUFF_OVF_MASK))
        {
            spin_lock_irqsave(&lock, lock_flag);
            SOCP_REG_READ(SOCP_REG_ENC_MASK2, mask);
            SOCP_REG_WRITE(SOCP_REG_ENC_MASK2, (IntState | mask));
            SOCP_REG_WRITE(SOCP_REG_ENC_RAWINT2, IntState);
            g_strSocpStat.u32IntEncDstOvf |= IntState;
            spin_unlock_irqrestore(&lock, lock_flag);

            bHandle = BSP_TRUE;

            for (i = 0; i < SOCP_MAX_ENCDST_CHN; i++)
            {
                if (IntState & ((u32)1 << i))
                {
                    g_stSocpDebugInfo.sSocpDebugEncDst.u32SocpEncDstIsrOvfIntCnt[i]++;
                }
            }
        }
    }
    else if(g_strSocpStat.compress_isr)
    {
        g_strSocpStat.compress_isr();
        return ;
    }

    /* 编码目的buffer模式切换完成 */
    else if (IntFlag & SOCP_CORE0_ENC_MODESWT_MASK)
    {
        spin_lock_irqsave(&lock, lock_flag);

        SOCP_REG_READ(SOCP_REG_ENC_INTSTAT0, IntState);
        SOCP_REG_READ(SOCP_REG_ENC_MASK0, mask);
        SOCP_REG_SETBITS(SOCP_REG_ENC_MASK0, 16, 7, (((IntState | mask)>>16)&0x7f));

        /* 清原始中断状态 */
        SOCP_REG_SETBITS(SOCP_REG_ENC_RAWINT0, 16, 7, ((IntState>>16)&0x7f));

        mask = 0;
        for (i = 0; i < SOCP_MAX_ENCDST_CHN; i++)
        {
            	SOCP_REG_READ(SOCP_REG_ENCDEST_SBCFG(i), ModeState);
            	if(ModeState & 0x02)
            	{
               	 	mask |= (1<<i);
            	}
        }

        /* 屏蔽处于循环模式通道的传输中断和阈值溢出中断 */
        SOCP_REG_SETBITS(SOCP_REG_ENC_MASK0, 0, 7, mask);
        SOCP_REG_SETBITS(SOCP_REG_ENC_MASK2, 0, 7, mask);
        SOCP_REG_SETBITS(SOCP_REG_ENC_MASK2, 16, 7, mask);

        spin_unlock_irqrestore(&lock, lock_flag);
    }
    else
    {
        bHandle = BSP_FALSE;
    }

    if(bHandle)
    {
        if(countFlag == BSP_TRUE){
            g_stEncDstStat[g_ulEncDstStatCount].ulIntEndSlice= bsp_get_slice_value();
        }
        up(&g_strSocpStat.u32EncDstSemID);
    }

    return ;
}
/*lint -restore +e550*/

/*****************************************************************************
* 函 数 名   : socp_handler_decsrc
*
* 功能描述  : 解码源通道中断处理函数
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 无
*****************************************************************************/
void socp_handler_decsrc(void)
{
    u32 IntFlag  = 0;
    u32 IntState = 0;
    int bHandle = BSP_FALSE;
    u32 i = 0;

    /*编码输入错误*/
    SOCP_REG_READ(SOCP_REG_GBL_INTSTAT, IntFlag);
    if (IntFlag & SOCP_DEC_INERRINT_MASK)
    {
        SOCP_REG_READ(SOCP_REG_DEC_INTSTAT1, IntState);
        SOCP_REG_WRITE(SOCP_REG_DEC_RAWINT1, IntState);

        g_strSocpStat.u32IntDecSrcErr |= IntState;
        bHandle = BSP_TRUE;

        for (i = 0; i < SOCP_MAX_DECSRC_CHN; i++)
        {
            if (IntState & 0x1)
            {
                g_stSocpDebugInfo.sSocpDebugDecSrc.u32SocpDecSrcIsrErrIntCnt[i]++;
            }
        }
    }

    if(bHandle)
    {
        up(&g_strSocpStat.u32DecSrcSemID);
    }

    return;
}


/*****************************************************************************
* 函 数 名   : socp_handler_decdst
*
* 功能描述  : 解码目的通道中断处理函数
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 无
*****************************************************************************/

void socp_handler_decdst(void)
{
    u32 IntFlag  = 0;
    u32 IntState = 0;
    int bHandle = BSP_FALSE;
    u32 TfMask  ;
    u32 TfState ;
    u32 OvMask   ;
    u32 OvState   ;
    u32 TfMaskReg ;
    u32 i = 0;

    TfMask    = SOCP_CORE0_DEC_TFRINT_MASK;
    TfState   = SOCP_REG_DEC_CORE0ISTAT0;
    TfMaskReg = SOCP_REG_DEC_CORE0MASK0;
    OvMask  = SOCP_CORE0_DEC_OUTOVFINT_MASK;
    OvState = SOCP_REG_DEC_CORE0ISTAT2;

    /*解码传输中断*/
    SOCP_REG_READ(SOCP_REG_GBL_INTSTAT, IntFlag);
    if (IntFlag & TfMask)
    {
        u32 mask;
        SOCP_REG_READ(TfState, IntState);
        IntState = IntState & 0xffff;
        SOCP_REG_READ(TfMaskReg, mask);
        SOCP_REG_WRITE(TfMaskReg, (IntState | mask));  //added by yangzhi 2011.7.27
        SOCP_REG_WRITE(SOCP_REG_DEC_RAWINT0, IntState);

        g_strSocpStat.u32IntDecDstTfr |= IntState;
        bHandle = BSP_TRUE;

        for (i = 0; i < SOCP_MAX_DECDST_CHN; i++)
        {
            if (IntState & ((u32)1 << i))
            {
                g_stSocpDebugInfo.sSocpDebugDecDst.u32SocpDecDstIsrTrfIntCnt[i]++;
            }
        }
    }

    /*解码目的buffer 上溢*/
    SOCP_REG_READ(SOCP_REG_GBL_INTSTAT, IntFlag);
    if (IntFlag & OvMask)
    {
        SOCP_REG_READ(OvState, IntState);
        IntState = IntState & 0xffff;
        SOCP_REG_WRITE(SOCP_REG_DEC_RAWINT2, IntState);

        g_strSocpStat.u32IntDecDstOvf |= IntState;
        bHandle = BSP_TRUE;

        for (i = 0; i < SOCP_MAX_DECDST_CHN; i++)
        {
            if (IntState & ((u32)1 << i))
            {
                g_stSocpDebugInfo.sSocpDebugDecDst.u32SocpDecDstIsrOvfIntCnt[i]++;
            }
        }
    }

    if(bHandle)
    {
        up(&g_strSocpStat.u32DecDstSemID);
    }

    return;
}


/*****************************************************************************
* 函 数 名   : socp_app_int_handler
*
* 功能描述  : APP 核中断处理函数
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 无
*****************************************************************************/
u32 socp_app_int_handler(void)
{
    g_stSocpDebugInfo.sSocpDebugGBl.u32SocpAppEtrIntCnt++;

    socp_handler_encsrc();

    socp_handler_encdst();
    socp_handler_decsrc();
    socp_handler_decdst();

    g_stSocpDebugInfo.sSocpDebugGBl.u32SocpAppSucIntCnt++;

    return 1;
}

struct platform_device *modem_socp_pdev;

static int socp_driver_probe(struct platform_device *pdev)
{
    dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));//lint !e598 !e648
    modem_socp_pdev = pdev;

    return BSP_OK;
}

static const struct of_device_id socp_dev_of_match[] = {
       {
	   	.compatible = "hisilicon,socp_balong_app",
		.data = NULL,
	 },
       {},
};

static struct platform_driver socp_driver = {
        .driver = {
                   .name = "modem_socp",

                   .owner = THIS_MODULE,
                   .of_match_table = socp_dev_of_match,
        },
        .probe = socp_driver_probe,
};

/*****************************************************************************
* 函 数 名  : socp_init
*
* 功能描述  : 模块初始化函数
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 初始化成功的标识码
*****************************************************************************/
s32 socp_init(void)
{
    s32 ret;
    u32 irq;

    struct device_node* dev = NULL;

    if(BSP_TRUE == g_strSocpStat.bInitFlag)
    {
        return BSP_OK;
    }

	socp_crit("[init]start\n");

    spin_lock_init(&lock);

    ret = platform_driver_register(&socp_driver);
    if(ret)
    {
        socp_error("driver_register fail,ret=0x%x\n", ret);
        return ret;
    }

    /*Add dts begin*/
    dev = of_find_compatible_node(NULL,NULL,"hisilicon,socp_balong_app");
    if(NULL == dev)
    {
        socp_error("[init] Socp dev find failed\n");
        return BSP_ERROR;
    }
    /*Add dts end*/

    g_strSocpStat.baseAddr = (unsigned long)of_iomap(dev,0);
    if(0 == g_strSocpStat.baseAddr)
    {
        socp_error("[init] base addr is NULL!\n");
        return BSP_ERROR; /* [false alarm]:屏蔽Fortify错误 */
    }

    /* bsp_memmap.h里面还没有对BBP寄存器空间做映射 */
    g_strSocpStat.armBaseAddr = (unsigned long)BBP_REG_ARM_BASEADDR;
    memset_s(&g_stSocpDebugInfo, sizeof(g_stSocpDebugInfo), 0x0 ,sizeof(g_stSocpDebugInfo));

    SOCP_REG_READ(SOCP_REG_SOCP_VERSION, socp_version);

    socp_global_ctrl_init();

    ret = socp_clk_enable();
    if(ret)
    {
        return ret;
    }

    socp_global_reset();

    bsp_socp_ind_delay_init();

    /* 创建编解码任务 */
    ret = socp_init_task();
    if (BSP_OK != ret)
    {
        socp_error("[init] create task failed(0x%x)\n", ret);
        return (s32)ret;
    }

    /* 挂中断 */
    irq = irq_of_parse_and_map(dev,0);
    ret = request_irq(irq, (irq_handler_t)socp_app_int_handler, 0, "SOCP_APP_IRQ",  BSP_NULL);
    if (BSP_OK != ret)
    {
        socp_error("[init] connect app core int failed(0x%x)\n", ret);
        return BSP_ERROR;
    }


    /* 设置初始化状态 */
    g_strSocpStat.bInitFlag = BSP_TRUE;

    socp_encsrc_headerr_irq_enable();

    socp_crit("[init]ok\n");

    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : bsp_socp_encdst_dsm_init
*
* 功能描述  : socp编码目的端中断状态初始化
* 输入参数  : EncDestChanID: 编码目的端通道号
*             bEnable: 初始化的中断状态
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
void bsp_socp_encdst_dsm_init(u32 EncDestChanID, u32 bEnable)
{
    u32 RealChanID;
    u32 u32ChanType;

    RealChanID   = SOCP_REAL_CHAN_ID(EncDestChanID);
    u32ChanType = SOCP_REAL_CHAN_TYPE(EncDestChanID);

    if(socp_check_chan_type(u32ChanType, SOCP_CODER_DEST_CHAN) != BSP_OK)
    {
        return;
    }

    if(SOCP_DEST_DSM_DISABLE == bEnable)
    {
        SOCP_REG_SETBITS(SOCP_REG_ENC_RAWINT0, RealChanID, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENC_MASK0, RealChanID, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENC_RAWINT2, RealChanID, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENC_MASK2, RealChanID, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENC_RAWINT2, RealChanID+16, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENC_MASK2, RealChanID+16, 1, 1);

        g_SocpEnableState[RealChanID] = SOCP_DEST_DSM_DISABLE;
    }
    else if(SOCP_DEST_DSM_ENABLE == bEnable)
    {
        SOCP_REG_SETBITS(SOCP_REG_ENC_RAWINT0, RealChanID, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENC_MASK0, RealChanID, 1, 0);
        SOCP_REG_SETBITS(SOCP_REG_ENC_RAWINT2, RealChanID, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENC_MASK2, RealChanID, 1, 0);
        SOCP_REG_SETBITS(SOCP_REG_ENC_RAWINT2, RealChanID+16, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENC_MASK2, RealChanID+16, 1, 0);

        g_SocpEnableState[RealChanID] = SOCP_DEST_DSM_ENABLE;
    }
    else
    {
		;
    }
}

/*****************************************************************************
* 函 数 名  : bsp_socp_data_send_continue
*
* 功能描述  : socp编码目的端数据上报使能，在readdone中调用
              根据diag连接状态判断是否上报

* 注    意  : 该函数调用时，需要调用者保证同步

* 输入参数  : EncDestChanID: 编码目的端通道号
*             bEnable: 中断使能
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
void bsp_socp_data_send_continue(u32 RealChanID)
{
    if(SOCP_DEST_DSM_ENABLE == g_SocpEnableState[RealChanID])
    {
        SOCP_REG_SETBITS(SOCP_REG_ENC_RAWINT0, RealChanID, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENC_MASK0, RealChanID, 1, 0);
        SOCP_REG_SETBITS(SOCP_REG_ENC_RAWINT2, RealChanID, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENC_MASK2, RealChanID, 1, 0);
        SOCP_REG_SETBITS(SOCP_REG_ENC_RAWINT2, RealChanID+16, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENC_MASK2, RealChanID+16, 1, 0);
    }
}
/*****************************************************************************
* 函 数 名  : bsp_socp_data_send_manager
*
* 功能描述  : socp编码目的端上报数据
* 输入参数  : EncDestChanID: 编码目的端通道号
*             bEnable: 中断使能
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
void bsp_socp_data_send_manager(u32 EncDestChanID, u32 bEnable)
{
    unsigned long lock_flag;
    u32 RealChanID;
    u32 u32ChanType;

    RealChanID   = SOCP_REAL_CHAN_ID(EncDestChanID);
    u32ChanType = SOCP_REAL_CHAN_TYPE(EncDestChanID);

    if(socp_check_chan_type(u32ChanType, SOCP_CODER_DEST_CHAN) != BSP_OK)
    {
        return;
    }

    if((SOCP_DEST_DSM_DISABLE == bEnable) && (g_SocpEnableState[RealChanID] == SOCP_DEST_DSM_ENABLE))
    {
        spin_lock_irqsave(&lock, lock_flag);
        SOCP_REG_SETBITS(SOCP_REG_ENC_RAWINT0, RealChanID, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENC_MASK0, RealChanID, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENC_RAWINT2, RealChanID, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENC_MASK2, RealChanID, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENC_RAWINT2, RealChanID+16, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENC_MASK2, RealChanID+16, 1, 1);

        g_SocpEnableState[RealChanID] = SOCP_DEST_DSM_DISABLE;

        spin_unlock_irqrestore(&lock, lock_flag);
    }
    else if((SOCP_DEST_DSM_ENABLE == bEnable) && (g_SocpEnableState[RealChanID] == SOCP_DEST_DSM_DISABLE))
    {
        spin_lock_irqsave(&lock, lock_flag);
        SOCP_REG_SETBITS(SOCP_REG_ENC_RAWINT0, RealChanID, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENC_MASK0, RealChanID, 1, 0);
        SOCP_REG_SETBITS(SOCP_REG_ENC_RAWINT2, RealChanID, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENC_MASK2, RealChanID, 1, 0);
        SOCP_REG_SETBITS(SOCP_REG_ENC_RAWINT2, RealChanID+16, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENC_MASK2, RealChanID+16, 1, 0);

         g_SocpEnableState[RealChanID] = SOCP_DEST_DSM_ENABLE;

        spin_unlock_irqrestore(&lock, lock_flag);
    }
    else
    {
        ;
    }
}

/***************************************************************************************
* 函 数 名  : socp_set_reg_wr_addr
*
* 功能描述  : 配置通道基地址寄存器和读写指针寄存器，函数内区分SOCP32位和64位寻址适配
*
* 输入参数  : ChanId: 通道号，包括通道类型和通道ID
*             pAttr: 通道配置参数
              start: 通道buffer起始地址
              end: 通道buffer结束地址
*
* 输出参数  : 无
*
* 返 回 值  : 无
****************************************************************************************/
void socp_set_reg_wr_addr(u32 ChanId, void *pAttr, unsigned long start, unsigned long end)
{
    u32 RealChanId = 0;
    u32 ChanType = 0;
    unsigned long rdstart = 0;
    unsigned long rdend = 0;
    SOCP_ENCSRC_CHAN_S *pEncSrcChan = NULL;
    SOCP_ENCDST_CHAN_S *pEncDstChan = NULL;
    SOCP_DECSRC_CHAN_S *pDecSrcChan = NULL;
    SOCP_DECDST_CHAN_S *pDecDstChan = NULL;

    RealChanId = SOCP_REAL_CHAN_ID(ChanId);
    ChanType  = SOCP_REAL_CHAN_TYPE(ChanId);

    if(ChanType == SOCP_CODER_SRC_CHAN)   // 编码源通道
    {
        SOCP_REG_WRITE(SOCP_REG_ENCSRC_BUFADDR_L(RealChanId), (u32)start);
        SOCP_REG_WRITE(SOCP_REG_ENCSRC_BUFADDR_H(RealChanId), (u32)((u64)start>>32));
        SOCP_REG_WRITE(SOCP_REG_ENCSRC_BUFWPTR(RealChanId),0);
        SOCP_REG_WRITE(SOCP_REG_ENCSRC_BUFRPTR(RealChanId), 0);
        if(SOCP_ENCSRC_CHNMODE_LIST == ((SOCP_CODER_SRC_CHAN_S *)pAttr)->eMode)
        {
            rdstart = (unsigned long)((SOCP_CODER_SRC_CHAN_S *)pAttr)->sCoderSetSrcBuf.pucRDStart;
            rdend   = (unsigned long)((SOCP_CODER_SRC_CHAN_S *)pAttr)->sCoderSetSrcBuf.pucRDEnd;
    		SOCP_REG_WRITE(SOCP_REG_ENCSRC_RDQADDR_L(RealChanId), (u32)rdstart);
    		SOCP_REG_WRITE(SOCP_REG_ENCSRC_RDQADDR_H(RealChanId), (u32)((u64)rdstart>>32));
            SOCP_REG_WRITE(SOCP_REG_ENCSRC_RDQRPTR(RealChanId),0);
            SOCP_REG_WRITE(SOCP_REG_ENCSRC_RDQWPTR(RealChanId), 0);
        }

        pEncSrcChan = &g_strSocpStat.sEncSrcChan[RealChanId];
        pEncSrcChan->eChnMode               = ((SOCP_CODER_SRC_CHAN_S *)pAttr)->eMode;
        pEncSrcChan->ePriority              = ((SOCP_CODER_SRC_CHAN_S *)pAttr)->ePriority;
        pEncSrcChan->eDataType              = ((SOCP_CODER_SRC_CHAN_S *)pAttr)->eDataType;
        pEncSrcChan->eDataTypeEn            = ((SOCP_CODER_SRC_CHAN_S *)pAttr)->eDataTypeEn;
        pEncSrcChan->eDebugEn               = ((SOCP_CODER_SRC_CHAN_S *)pAttr)->eDebugEn;
        pEncSrcChan->u32DestChanID          = ((SOCP_CODER_SRC_CHAN_S *)pAttr)->u32DestChanID;
        pEncSrcChan->u32BypassEn            = ((SOCP_CODER_SRC_CHAN_S *)pAttr)->u32BypassEn;
        pEncSrcChan->sEncSrcBuf.Start    = start;
        pEncSrcChan->sEncSrcBuf.End      = end;
        pEncSrcChan->sEncSrcBuf.u32Write    = 0;
        pEncSrcChan->sEncSrcBuf.u32Read     = 0;
        pEncSrcChan->sEncSrcBuf.u32Length   = end - start + 1;//lint !e834
        pEncSrcChan->sEncSrcBuf.u32IdleSize = 0;

        if(SOCP_ENCSRC_CHNMODE_LIST == ((SOCP_CODER_SRC_CHAN_S *)pAttr)->eMode)
        {
            pEncSrcChan->sRdBuf.Start    = rdstart;
            pEncSrcChan->sRdBuf.End      = rdend;
            pEncSrcChan->sRdBuf.u32Write    = 0;
            pEncSrcChan->sRdBuf.u32Read     = 0;
    	    pEncSrcChan->sRdBuf.u32Length   = rdend - rdstart + 1;//lint !e834
            pEncSrcChan->u32RdThreshold     = ((SOCP_CODER_SRC_CHAN_S *)pAttr)->sCoderSetSrcBuf.u32RDThreshold;
        }

    }

    else if(ChanType == SOCP_CODER_DEST_CHAN)   // 编码目的通道
    {
        SOCP_REG_WRITE(SOCP_REG_ENCDEST_BUFADDR_L(RealChanId), (u32)start);
        SOCP_REG_WRITE(SOCP_REG_ENCDEST_BUFADDR_H(RealChanId), (u32)((u64)start>>32));
        SOCP_REG_WRITE(SOCP_REG_ENCDEST_BUFRPTR(RealChanId),0);
        SOCP_REG_WRITE(SOCP_REG_ENCDEST_BUFWPTR(RealChanId), 0);

        pEncDstChan = &g_strSocpStat.sEncDstChan[RealChanId];
        pEncDstChan->sEncDstBuf.Start    = start;
        pEncDstChan->sEncDstBuf.End      = end;
        pEncDstChan->sEncDstBuf.u32Read     = 0;
        pEncDstChan->sEncDstBuf.u32Write    = 0;
        pEncDstChan->sEncDstBuf.u32Length   = end - start + 1;//lint !e834
        pEncDstChan->bufThreshold  =   ((SOCP_CODER_DEST_CHAN_S *)pAttr)->sCoderSetDstBuf.u32Threshold;
        pEncDstChan->u32Thrh       =   ((SOCP_CODER_DEST_CHAN_S *)pAttr)->u32EncDstThrh;
        /* 表明该通道已经配置 */
        pEncDstChan->u32SetStat = SOCP_CHN_SET;
    }

    else if(ChanType == SOCP_DECODER_SRC_CHAN)   // 解码源通道
    {
        SOCP_REG_WRITE(SOCP_REG_DECSRC_BUFADDR_L(RealChanId),(u32)start);
        SOCP_REG_WRITE(SOCP_REG_DECSRC_BUFADDR_H(RealChanId), (u32)((u64)start>>32));
        SOCP_REG_WRITE(SOCP_REG_DECSRC_BUFRPTR(RealChanId),0);
        SOCP_REG_WRITE(SOCP_REG_DECSRC_BUFWPTR(RealChanId), 0);

        pDecSrcChan = &g_strSocpStat.sDecSrcChan[RealChanId];
        pDecSrcChan->u32ChanID = RealChanId;
        pDecSrcChan->eDataTypeEn= ((SOCP_DECODER_SRC_CHAN_STRU *)pAttr)->eDataTypeEn;
        pDecSrcChan->sDecSrcBuf.Start = start;
        pDecSrcChan->sDecSrcBuf.End = end;
        pDecSrcChan->sDecSrcBuf.u32Length = end - start + 1;//lint !e834
        pDecSrcChan->sDecSrcBuf.u32Read     = 0;
        pDecSrcChan->sDecSrcBuf.u32Write    = 0;
        pDecSrcChan->u32SetStat = SOCP_CHN_SET;
    }

    else if(ChanType == SOCP_DECODER_DEST_CHAN)   // 解码目的通道
    {
        SOCP_REG_WRITE(SOCP_REG_DECDEST_BUFADDR_L(RealChanId), (u32)start);
        SOCP_REG_WRITE(SOCP_REG_DECDEST_BUFADDR_H(RealChanId), (u32)((u64)start>>32));
        SOCP_REG_WRITE(SOCP_REG_DECDEST_BUFWPTR(RealChanId),0);
        SOCP_REG_WRITE(SOCP_REG_DECDEST_BUFRPTR(RealChanId), 0);

        pDecDstChan = &g_strSocpStat.sDecDstChan[RealChanId];
        pDecDstChan->eDataType = ((SOCP_DECODER_DEST_CHAN_STRU *)pAttr)->eDataType;
        pDecDstChan->sDecDstBuf.Start    = start;
        pDecDstChan->sDecDstBuf.End      = end;
        pDecDstChan->sDecDstBuf.u32Length   = end - start + 1;//lint !e834
        pDecDstChan->sDecDstBuf.u32Read     = 0;
        pDecDstChan->sDecDstBuf.u32Write    = 0;
    }

}

/*****************************************************************************
* 函 数 名  : bsp_socp_coder_set_src_chan
*
* 功能描述  : 编码源通道配置函数
*
* 输入参数  : pSrcAttr     编码源通道配置参数
*             ulSrcChanID  编码源通道ID
*
* 输出参数  : 无
*
* 返 回 值  : 申请及配置成功与否的标识码
*****************************************************************************/
s32 bsp_socp_coder_set_src_chan(SOCP_CODER_SRC_ENUM_U32 enSrcChanID, SOCP_CODER_SRC_CHAN_S *pSrcAttr)
{
    unsigned long start = 0;
    unsigned long end = 0;
    unsigned long base_addr_rdstart = 0;
    unsigned long base_addr_rdend = 0;
    u32 buflength   = 0;
    u32 Rdbuflength = 0;
    u32 i;
    u32 srcChanId;
    u32 u32SrcChanType;
    u32 u32DstChanID;
    u32 u32DstChanType;
    u32 ResetFlag;
    s32 ret;

    g_stSocpDebugInfo.sSocpDebugGBl.u32SocpAllocEncSrcCnt++;

    /* 判断是否已经初始化 */
    if((ret=socp_check_init()) != BSP_OK)
    {
        return ret;
    }

    /* 判断参数有效性 */
    if(NULL == pSrcAttr)
    {
        socp_error("the parameter is NULL\n");
        return BSP_ERR_SOCP_NULL;
    }

    if((ret=socp_check_chan_priority(pSrcAttr->ePriority)) != BSP_OK)
    {
        return ret;
    }
    if((ret=socp_check_data_type(pSrcAttr->eDataType)) != BSP_OK)
    {
        return ret;
    }

    if((ret=socp_check_data_type_en(pSrcAttr->eDataTypeEn)) != BSP_OK)
    {
        return ret;
    }
    if((ret=socp_check_enc_debug_en(pSrcAttr->eDebugEn)) != BSP_OK)
    {
        return ret;
    }
    srcChanId       = SOCP_REAL_CHAN_ID(enSrcChanID);
    u32SrcChanType  = SOCP_REAL_CHAN_TYPE(enSrcChanID);

    if((ret=socp_check_chan_type(u32SrcChanType, SOCP_CODER_SRC_CHAN)) != BSP_OK)
    {
        return ret;
    }

    if((ret=socp_check_encsrc_chan_id(srcChanId)) != BSP_OK)
    {
        return ret;
    }
    u32DstChanID   = SOCP_REAL_CHAN_ID(pSrcAttr->u32DestChanID);
    u32DstChanType = SOCP_REAL_CHAN_TYPE(pSrcAttr->u32DestChanID);
    if((ret=socp_check_chan_type(u32DstChanType, SOCP_CODER_DEST_CHAN)) != BSP_OK)
    {
        return ret;
    }
    if((ret=socp_check_chan_id(u32DstChanID, SOCP_MAX_ENCDST_CHN)) != BSP_OK)
    {
        return ret;
    }

    if ((SOCP_ENCSRC_CHNMODE_CTSPACKET != pSrcAttr->eMode)
        && (SOCP_ENCSRC_CHNMODE_LIST != pSrcAttr->eMode))
    {
        socp_error("chnnel mode(%d) is invalid\n", pSrcAttr->eMode);
        return BSP_ERR_SOCP_INVALID_PARA;
    }

    /* 使用配置参数进行配置 */
    /* 判断起始地址是否8字节对齐 */
    start   = (unsigned long)pSrcAttr->sCoderSetSrcBuf.pucInputStart;
    end     = (unsigned long)pSrcAttr->sCoderSetSrcBuf.pucInputEnd;

    if(0 == start)
    {
        socp_error("start addr is NULL\n");
        return BSP_ERR_SOCP_NULL;
    }
    if((ret=socp_check_8bytes_align(start)) != BSP_OK)
    {
        return ret;
    }
    if(0 == end)
    {
        socp_error("end addr is NULL\n");
        return BSP_ERR_SOCP_NULL;
    }
    if((ret=socp_check_buf_addr(start, end)) != BSP_OK)
    {
        return ret;
    }
    buflength = (u32)(end - start + 1);

    if((ret=socp_check_8bytes_align(buflength)) != BSP_OK)
    {
        return ret;
    }

    if(socp_version < SOCP_201_VERSION)
    {
        if(buflength > SOCP_ENC_SRC_BUFLGTH_MAX)
        {
            socp_error("buffer length is too large\n");
            return BSP_ERR_SOCP_INVALID_PARA;
        }
    }

    /* 如果是用链表缓冲区，则配置RDbuffer的起始地址和长度 */
    if(SOCP_ENCSRC_CHNMODE_LIST == pSrcAttr->eMode)
    {
        /* 判断RDBuffer的起始地址是否8字节对齐 */
        base_addr_rdstart = (unsigned long)pSrcAttr->sCoderSetSrcBuf.pucRDStart;
        base_addr_rdend   = (unsigned long)pSrcAttr->sCoderSetSrcBuf.pucRDEnd;

        if(0 == base_addr_rdstart)
        {
            socp_error("RD start addr is NULL\n");
            return BSP_ERR_SOCP_NULL;
        }
        if((ret=socp_check_8bytes_align(base_addr_rdstart)) != BSP_OK)
        {
            return ret;
        }
        if(0 == base_addr_rdend)
        {
            socp_error("RD end addr is NULL\n");
            return BSP_ERR_SOCP_NULL;
        }
        if((ret=socp_check_buf_addr(base_addr_rdstart, base_addr_rdend)) != BSP_OK)
        {
            return ret;
        }
        Rdbuflength = (u32)(base_addr_rdend - base_addr_rdstart + 1);

        if((ret=socp_check_8bytes_align(Rdbuflength)) != BSP_OK)
        {
            return ret;
        }
        if(Rdbuflength > SOCP_ENC_SRC_RDLGTH_MAX)
        {
            socp_error("RD buffer length is too large\n");
            return BSP_ERR_SOCP_INVALID_PARA;
        }
    }

    /* 复位通道 */
    SOCP_REG_SETBITS(SOCP_REG_ENCRST, srcChanId, 1, 1);

    /* 等待通道自清 */
    for (i = 0; i < SOCP_RESET_TIME; i++)
    {
        ResetFlag = SOCP_REG_GETBITS(SOCP_REG_ENCRST, srcChanId, 1);
        if(0 == ResetFlag)
        {
            break;
        }
    }

    /* 配置编码源通道参数 */
	/*lint -save -e647*/
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG1(srcChanId), 1, 2, pSrcAttr->eMode);
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG1(srcChanId), 4, 4, pSrcAttr->u32DestChanID);
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG1(srcChanId), 8, 2, pSrcAttr->ePriority);
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG1(srcChanId), 10, 1, pSrcAttr->u32BypassEn);
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG1(srcChanId), 11, 1, pSrcAttr->eDataTypeEn);
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG1(srcChanId), 31, 1, pSrcAttr->eDebugEn);
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG1(srcChanId), 16, 8, pSrcAttr->eDataType);
	/*lint -restore +e647*/

    /*lint -save -e845*/
    /*lint -save -e647*/
    if(socp_version >= SOCP_201_VERSION)
    {
        SOCP_REG_WRITE(SOCP_REG_ENCSRC_BUFCFG0(srcChanId),buflength);
    }
    else
    {
        SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG0(srcChanId), 0, 27, buflength);
        SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG0(srcChanId), 27, 5, 0);
    }
    if(SOCP_ENCSRC_CHNMODE_LIST == pSrcAttr->eMode)
    {
        SOCP_REG_SETBITS(SOCP_REG_ENCSRC_RDQCFG(srcChanId), 0, 16, Rdbuflength);
        SOCP_REG_SETBITS(SOCP_REG_ENCSRC_RDQCFG(srcChanId), 16, 16, 0);
    }
    /*lint -restore +e647*/
    /*lint -restore +e845*/

    socp_set_reg_wr_addr(enSrcChanID, (void*)pSrcAttr, start, end);

    /* 标记通道状态 */
    g_strSocpStat.sEncSrcChan[srcChanId].u32AllocStat = SOCP_CHN_ALLOCATED;
    g_stSocpDebugInfo.sSocpDebugGBl.u32SocpAllocEncSrcSucCnt++;
    return BSP_OK;

}

/*****************************************************************************
* 函 数 名  : bsp_socp_decoder_set_dest_chan
*
* 功能描述  : 解码目的通道申请及配置函数
*
* 输入参数  : pAttr           解码目的通道配置参数
*             pDestChanID     初始化解码目的通道ID，解码通道源与目的ID有对应关系
                              SrcID = DestChanID%4
*
* 输出参数  :
*
* 返 回 值  : 申请及配置成功与否的标识码
*****************************************************************************/
s32 bsp_socp_decoder_set_dest_chan(SOCP_DECODER_DST_ENUM_U32 enDestChanID,
                                                SOCP_DECODER_DEST_CHAN_STRU *pAttr)
{
    unsigned long start;
    unsigned long end;
    u32 bufThreshold;
    u32 buflength;
    u32 u32ChanID;
    u32 u32SrcChanID;
    u32 u32ChanType;
    // SOCP_DECDST_CHAN_S *pChan;
    s32 ret;

    g_stSocpDebugInfo.sSocpDebugGBl.u32SocpAllocDecDstCnt++;

    /* 判断是否已经初始化 */
    if((ret=socp_check_init()) != BSP_OK)
    {
        return ret;
    }

    /* 判断参数有效性 */
    if(NULL == pAttr)
    {
        socp_error("the parameter is NULL\n");
        return BSP_ERR_SOCP_NULL;
    }
    if((ret=socp_check_data_type(pAttr->eDataType)) != BSP_OK)
    {
        return ret;
    }
    u32ChanID    = SOCP_REAL_CHAN_ID(enDestChanID);
    u32ChanType  = SOCP_REAL_CHAN_TYPE(enDestChanID);
    if((ret=socp_check_chan_type(u32ChanType, SOCP_DECODER_DEST_CHAN)) != BSP_OK)
    {
        return ret;
    }
    if((ret=socp_check_chan_id(u32ChanID, SOCP_MAX_DECDST_CHN)) != BSP_OK)
    {
        return ret;
    }

    u32SrcChanID = SOCP_REAL_CHAN_ID(pAttr->u32SrcChanID);
    u32ChanType = SOCP_REAL_CHAN_TYPE(pAttr->u32SrcChanID);
    if((ret=socp_check_chan_type(u32ChanType, SOCP_DECODER_SRC_CHAN)) != BSP_OK)
    {
        return ret;
    }
    if((ret=socp_check_chan_id(u32SrcChanID, SOCP_MAX_DECSRC_CHN)) != BSP_OK)
    {
        return ret;
    }

    /* 判断通道ID对应关系 */
    if(u32SrcChanID != u32ChanID%4)
    {
        socp_error("dest ID(%d) is not matching src ID(%d)!\n", u32ChanID, u32SrcChanID);
        return BSP_ERR_SOCP_INVALID_PARA;
    }

    /* 判断给定的地址和长度是否为八字节倍数*/
    start           = (unsigned long)pAttr->sDecoderDstSetBuf.pucOutputStart;
    end             = (unsigned long)pAttr->sDecoderDstSetBuf.pucOutputEnd;
    bufThreshold    = pAttr->sDecoderDstSetBuf.u32Threshold;

    if(0 == start)
    {
        socp_error("start addr is NULL\n");
        return BSP_ERR_SOCP_NULL;
    }
    if((ret=socp_check_8bytes_align(start)) != BSP_OK)
    {
        return ret;
    }
    if(0 == end)
    {
        socp_error("end addr is NULL\n");
        return BSP_ERR_SOCP_NULL;
    }
    if((ret=socp_check_buf_addr(start, end)) != BSP_OK)
    {
        return ret;
    }
    buflength = (u32)(end - start + 1);

    if(0 == bufThreshold)
    {
        socp_error("the bufThreshold is 0\n");
        return BSP_ERR_SOCP_NULL;
    }
    if (bufThreshold > SOCP_DEC_DST_TH_MAX)
    {
        socp_error("buffer threshold is too large!\n");
        return BSP_ERR_SOCP_INVALID_PARA;
    }

    if((ret=socp_check_8bytes_align(buflength)) != BSP_OK)
    {
        return ret;
    }
    if (buflength > SOCP_DEC_DST_BUFLGTH_MAX)
    {
        socp_error("buffer length is too large!\n");
        return BSP_ERR_SOCP_INVALID_PARA;
    }
	/*lint -save -e647*/
    SOCP_REG_SETBITS(SOCP_REG_DECDEST_BUFCFG(u32ChanID), 24, 8, pAttr->eDataType);
    SOCP_REG_SETBITS(SOCP_REG_DECDEST_BUFCFG(u32ChanID), 0, 16, buflength);
    SOCP_REG_SETBITS(SOCP_REG_DECDEST_BUFCFG(u32ChanID), 16, 8, bufThreshold);

    socp_set_reg_wr_addr(enDestChanID, (void*)pAttr, start, end);

    /* 先清中断，再打开中断*/
    SOCP_REG_SETBITS(SOCP_REG_DEC_RAWINT0, u32ChanID, 1, 1);
    SOCP_REG_SETBITS(SOCP_REG_DEC_CORE0MASK0, u32ChanID, 1, 0);
    SOCP_REG_SETBITS(SOCP_REG_DEC_RAWINT2, u32ChanID, 1, 1);
    SOCP_REG_SETBITS(SOCP_REG_DEC_CORE0MASK2, u32ChanID, 1, 0);
	/*lint -restore +e647*/

    /* 标记通道分配状态 */
    g_strSocpStat.sDecDstChan[u32ChanID].u32AllocStat = SOCP_CHN_ALLOCATED;
    g_stSocpDebugInfo.sSocpDebugGBl.u32SocpAllocDecDstSucCnt++;

    return BSP_OK;
}

static s32 socp_encdst_param_check(SOCP_CODER_DEST_CHAN_S *pDestAttr)
{
    unsigned long start;
    unsigned long end;
    u32 bufThreshold;
	u32 buflength;
	s32 ret;

    /* 判断是否已经初始化 */
    if((ret=socp_check_init()) != BSP_OK)
    {
        return ret;
    }

    /* 判断参数有效性 */
    if(NULL == pDestAttr)
    {
        socp_error("the parameter is NULL\n");
        return BSP_ERR_SOCP_NULL;
    }

    start = (unsigned long)pDestAttr->sCoderSetDstBuf.pucOutputStart;
    end   = (unsigned long)pDestAttr->sCoderSetDstBuf.pucOutputEnd;
    bufThreshold = pDestAttr->sCoderSetDstBuf.u32Threshold;

    if(0 == start)
    {
        socp_error("start addr is NULL\n");
        return BSP_ERR_SOCP_NULL;
    }
    if((ret=socp_check_8bytes_align(start)) != BSP_OK)
    {
        return ret;
    }
    if(0 == end)
    {
        socp_error("end addr is NULL\n");
        return BSP_ERR_SOCP_NULL;
    }
    if((ret=socp_check_buf_addr(start, end)) != BSP_OK)
    {
        return ret;
    }
    buflength = (u32)(end - start + 1);

    if(0 == bufThreshold)
    {
        socp_error("the bufThreshold is 0\n");
        return BSP_ERR_SOCP_NULL;
    }
    if((ret=socp_check_8bytes_align(buflength)) != BSP_OK)
    {
        return ret;
    }

	return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : bsp_socp_coder_set_dest_chan_attr
*
* 功能描述  : 编码目的通道配置函数
*
* 输入参数  : u32DestChanID      编码目的通道ID
              pDestAttr          编码目的通道配置参数
*
* 输出参数  : 无
* 返 回 值  : 配置成功与否的标识码
*****************************************************************************/
s32 bsp_socp_coder_set_dest_chan_attr(u32 u32DestChanID, SOCP_CODER_DEST_CHAN_S *pDestAttr)
{
    unsigned long start;
    unsigned long end;
    u32 bufThreshold;
    u32 buflength;
    u32 u32ChanID;
    u32 u32ChanType;
    u32 u32Thrh;
    u32 u32TimeoutMode;
    s32 ret;

    g_stSocpDebugInfo.sSocpDebugGBl.u32SocpSetEncDstCnt++;

	ret = socp_encdst_param_check(pDestAttr);
	if(ret)
	{
		return ret;
	}

    u32ChanID   = SOCP_REAL_CHAN_ID(u32DestChanID);
    u32ChanType = SOCP_REAL_CHAN_TYPE(u32DestChanID);
    if((ret=socp_check_chan_type(u32ChanType, SOCP_CODER_DEST_CHAN)) != BSP_OK)
    {
        return ret;
    }
    if((ret=socp_check_chan_id(u32ChanID, SOCP_MAX_ENCDST_CHN)) != BSP_OK)
    {
        return ret;
    }

    start = (unsigned long)pDestAttr->sCoderSetDstBuf.pucOutputStart;
    end   = (unsigned long)pDestAttr->sCoderSetDstBuf.pucOutputEnd;
    bufThreshold = pDestAttr->sCoderSetDstBuf.u32Threshold;
    u32Thrh = pDestAttr->u32EncDstThrh;
    u32TimeoutMode = pDestAttr->u32EncDstTimeoutMode;
    buflength = (u32)(end - start + 1);

    /* 如果经过配置则不能再次配置,通道复位之后只配置一次 */
    /* 使用配置参数进行配置 */
	/*lint -save -e647*/
    if (SOCP_CHN_SET == g_strSocpStat.sEncDstChan[u32ChanID].u32SetStat)
    {
        socp_error("channel 0x%x can't be set twice!\n", u32ChanID);
        return BSP_ERR_SOCP_SET_FAIL;
    }
    SOCP_REG_WRITE(SOCP_REG_ENCDEST_BUFCFG(u32ChanID),buflength);
    SOCP_REG_WRITE(SOCP_REG_ENCDEST_BUFTHRESHOLD(u32ChanID),(bufThreshold*1024));

    SOCP_REG_SETBITS(SOCP_REG_ENCDEST_BUFTHRH(u32ChanID), 0, 31, u32Thrh);

    if(socp_version < SOCP_206_VERSION)
    {
       u32TimeoutMode = SOCP_TIMEOUT_TRF;
    }

    if(SOCP_TIMEOUT_TRF_LONG == u32TimeoutMode)
    {
    	SOCP_REG_SETBITS(SOCP_REG_GBLRST, 4, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENCDEST_SBCFG(u32ChanID), 3, 1, 1);
        bsp_socp_set_timeout(SOCP_TIMEOUT_TRF_LONG, SOCP_TIMEOUT_TRF_LONG_MIN); /*立即上报:长超时阈值=10ms*/
    }
    else if((SOCP_TIMEOUT_TRF_SHORT == u32TimeoutMode))
    {
    	SOCP_REG_SETBITS(SOCP_REG_GBLRST, 4, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENCDEST_SBCFG(u32ChanID), 3, 1, 0);
        bsp_socp_set_timeout(SOCP_TIMEOUT_TRF_SHORT, SOCP_TIMEOUT_TRF_SHORT_VAL); /*短超时阈值=10ms*/
    }
	else
	{
		SOCP_REG_SETBITS(SOCP_REG_GBLRST, 4, 1, 0);
	}


    socp_set_reg_wr_addr(u32DestChanID, (void*)pDestAttr, start, end);

    bsp_socp_encdst_dsm_init(u32DestChanID, SOCP_DEST_DSM_DISABLE);

	/*lint -restore +e647*/

    g_stSocpDebugInfo.sSocpDebugGBl.u32SocpSetEncDstSucCnt++;

    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : bsp_socp_decoder_set_src_chan_attr
*
* 功能描述  : 解码源通道配置函数
*
* 输入参数  : u32SrcChanID    解码源通道ID
*             pInputAttr      解码源通道配置参数
*
* 输出参数  :
*
* 返 回 值  : 配置成功与否的标识码
*****************************************************************************/
s32 bsp_socp_decoder_set_src_chan_attr(u32 u32SrcChanID, SOCP_DECODER_SRC_CHAN_STRU *pInputAttr)
{
    unsigned long start;
    unsigned long end;
    u32 buflength = 0;
    u32 u32ChanID;
    u32 u32ChanType;
    u32 i;
    u32 u32ResetFlag;
    // SOCP_DECSRC_CHAN_S *pDecSrcChan;
    s32 ret;

    g_stSocpDebugInfo.sSocpDebugGBl.u32SocpSetDecSrcCnt++;

    /* 判断是否已经初始化 */
    if((ret=socp_check_init()) != BSP_OK)
    {
        return ret;
    }

    /* 判断参数有效性 */
    if(NULL == pInputAttr)
    {
        socp_error("the parameter is NULL\n");
        return BSP_ERR_SOCP_NULL;
    }
    u32ChanID   = SOCP_REAL_CHAN_ID(u32SrcChanID);
    u32ChanType = SOCP_REAL_CHAN_TYPE(u32SrcChanID);
    if((ret=socp_check_chan_type(u32ChanType, SOCP_DECODER_SRC_CHAN)) != BSP_OK)
    {
        return ret;
    }
    if((ret=socp_check_chan_id(u32ChanID, SOCP_MAX_DECSRC_CHN)) != BSP_OK)
    {
        return ret;
    }
    if((ret=socp_check_data_type_en(pInputAttr->eDataTypeEn)) != BSP_OK)
    {
        return ret;
    }

    start = (unsigned long)pInputAttr->sDecoderSetSrcBuf.pucInputStart;
    end = (unsigned long)pInputAttr->sDecoderSetSrcBuf.pucInputEnd;

    if(0 == start)
    {
        socp_error("start addr is NULL\n");
        return BSP_ERR_SOCP_NULL;
    }
    if((ret=socp_check_8bytes_align(start)) != BSP_OK)
    {
        return ret;
    }
    if(0 == end)
    {
        socp_error("end addr is NULL\n");
        return BSP_ERR_SOCP_NULL;
    }
    if((ret=socp_check_buf_addr(start, end)) != BSP_OK)
    {
        return ret;
    }
    buflength = (u32)(end - start + 1);

    if((ret=socp_check_8bytes_align(buflength)) != BSP_OK)
    {
        return ret;
    }
    if (buflength > SOCP_DEC_SRC_BUFLGTH_MAX)
    {
        socp_error("buffer length is too large!\n");
        return BSP_ERR_SOCP_INVALID_PARA;
    }

    if (SOCP_CHN_SET == g_strSocpStat.sDecSrcChan[u32ChanID].u32SetStat)
    {
        socp_error("channel 0x%x has been configed!\n", u32ChanID);
        return BSP_ERR_SOCP_DECSRC_SET;
    }

    /* 首先复位通道 */
    SOCP_REG_SETBITS(SOCP_REG_DECRST, u32ChanID, 1, 1);

    /* 等待通道复位状态自清 */
    for (i = 0; i < SOCP_RESET_TIME; i++)
    {
        u32ResetFlag = SOCP_REG_GETBITS(SOCP_REG_DECRST, u32ChanID, 1);
        if (0 == u32ResetFlag)
        {
            break;
        }
    }
	/*lint -save -e647*/
    SOCP_REG_SETBITS(SOCP_REG_DECSRC_BUFCFG0(u32ChanID), 0, 16, buflength);
    SOCP_REG_SETBITS(SOCP_REG_DECSRC_BUFCFG0(u32ChanID), 31, 1, pInputAttr->eDataTypeEn);
	/*lint -restore +e647*/

    socp_set_reg_wr_addr(u32SrcChanID, (void*)pInputAttr, start, end);

    g_stSocpDebugInfo.sSocpDebugGBl.u32SocpSetDeSrcSucCnt++;

    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : bsp_socp_decoder_get_err_cnt
*
* 功能描述  : 解码通道中获取错误计数函数
*
* 输入参数  : u32ChanID       解码通道ID

* 输出参数  : pErrCnt         解码通道错误计数结构体指针
*
* 返 回 值  : 获取成功与否的标识码
*****************************************************************************/
s32 bsp_socp_decoder_get_err_cnt(u32 u32DstChanID, SOCP_DECODER_ERROR_CNT_STRU *pErrCnt)
{
    u32 u32ChanID;
    u32 u32ChanType;
    s32 ret;

    /* 判断是否已经初始化 */
    if((ret=socp_check_init()) != BSP_OK)
    {
        return ret;
    }

    /* 判断通道ID是否有效 */
    u32ChanID   = SOCP_REAL_CHAN_ID(u32DstChanID);
    u32ChanType = SOCP_REAL_CHAN_TYPE(u32DstChanID);
    if((ret=socp_check_chan_type(u32ChanType, SOCP_DECODER_SRC_CHAN)) != BSP_OK)
    {
        return ret;
    }
    if((ret=socp_check_chan_id(u32ChanID, SOCP_MAX_DECSRC_CHN)) != BSP_OK)
    {
        return ret;
    }
    if((ret=socp_check_decsrc_set(u32ChanID)) != BSP_OK)
    {
        return ret;
    }

    /* 判断参数有效性 */
    if(NULL == pErrCnt)
    {
        socp_error("the parameter is NULL\n");
        return BSP_ERR_SOCP_NULL;
    }

    /* 判断通道是否打开，并设置为debug模式*/
    if (g_strSocpStat.sDecSrcChan[u32ChanID].u32ChanEn)
    {
    	/*lint -save -e647*/
        pErrCnt->u32PktlengthCnt = SOCP_REG_GETBITS(SOCP_REG_DEC_BUFSTAT0(u32ChanID), 16, 16);
        pErrCnt->u32CrcCnt = SOCP_REG_GETBITS(SOCP_REG_DEC_BUFSTAT0(u32ChanID), 0, 16);
        pErrCnt->u32DataTypeCnt   = SOCP_REG_GETBITS(SOCP_REG_DEC_BUFSTAT1(u32ChanID), 16, 16);
        pErrCnt->u32HdlcHeaderCnt = SOCP_REG_GETBITS(SOCP_REG_DEC_BUFSTAT1(u32ChanID), 0, 16);
		/*lint -restore +e647*/
    }
    else
    {
        socp_crit("debug mode is closed!\n");
        return BSP_ERR_SOCP_SET_FAIL;
    }

    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : bsp_socp_set_timeout
*
* 功能描述  : 超时阈值设置函数
*
* 输入参数  :   eTmOutEn          设置对象选择及使能
                u32Timeout        超时阈值
*
* 输出参数  :
*
* 返 回 值  : 设置成功与否的标识码
*****************************************************************************/
s32 bsp_socp_set_timeout(SOCP_TIMEOUT_EN_ENUM_UIN32 eTmOutEn, u32 u32Timeout)
{
    u32 u32newtime;
    u32 temp;


    u32newtime = ((socp_version == SOCP_203_VERSION) || (socp_version == SOCP_204_VERSION)) ? SOCP_CLK_RATIO(u32Timeout) : u32Timeout;

    switch (eTmOutEn)
    {
        case SOCP_TIMEOUT_BUFOVF_DISABLE:
        {
            SOCP_REG_SETBITS(SOCP_REG_BUFTIMEOUT, 31, 1, 0);
            break;
        }
        case SOCP_TIMEOUT_BUFOVF_ENABLE:
        {
            SOCP_REG_SETBITS(SOCP_REG_BUFTIMEOUT, 31, 1, 1);
            /* 增加换算的方法 */
            SOCP_REG_SETBITS(SOCP_REG_BUFTIMEOUT, 0, 16, u32newtime);
            break;
        }

        case SOCP_TIMEOUT_TRF:
        {
            if(SOCP_REG_GETBITS(SOCP_REG_GBLRST, 4, 1))
            {
                SOCP_REG_SETBITS(SOCP_REG_GBLRST, 4, 1, 0);
            }

			SOCP_REG_WRITE(SOCP_REG_INTTIMEOUT, u32newtime);
            break;
        }


        case SOCP_TIMEOUT_TRF_LONG:
			/* fall through */
        case SOCP_TIMEOUT_TRF_SHORT:
        {
            if(0 == SOCP_REG_GETBITS(SOCP_REG_GBLRST, 4, 1))
            {
                SOCP_REG_SETBITS(SOCP_REG_GBLRST, 4, 1, 1);
            }

            if(socp_version >= SOCP_206_VERSION)
            {
                if(SOCP_TIMEOUT_TRF_LONG == eTmOutEn)
                {
                    if(u32newtime > 0xffffff)
                    {
                        socp_error("the value is too large!\n");
                        return BSP_ERR_SOCP_INVALID_PARA;
                    }
                    SOCP_REG_READ(SOCP_REG_INTTIMEOUT, temp);
                    u32newtime = (u32newtime<<8) | (temp&0xff);
                    SOCP_REG_WRITE(SOCP_REG_INTTIMEOUT, u32newtime);
                }
                else
                {
                    if(u32newtime > 0xff)
                    {
                        socp_error("the value is too large!\n");
                        return BSP_ERR_SOCP_INVALID_PARA;
                    }
                    SOCP_REG_READ(SOCP_REG_INTTIMEOUT, temp);
                    u32newtime = (temp&0xffffff00) | u32newtime;
                    SOCP_REG_WRITE(SOCP_REG_INTTIMEOUT, u32newtime);
                }
            }
            else
            {
			    socp_error("This version does not support the function.\n");
            }

            break;
        }


        default:
        {
            socp_error("invalid timeout choice type!\n");
            return BSP_ERR_SOCP_SET_FAIL;
        }
    }

    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : bsp_socp_set_dec_pkt_lgth
*
* 功能描述  : 超时阈值设置函数
*
* 输入参数  :   pPktlgth          解码包长度设置参数结构体
*
* 输出参数  :
*
* 返 回 值  : 设置成功与否的标识码
*****************************************************************************/
/* cov_verified_start */
s32 bsp_socp_set_dec_pkt_lgth(SOCP_DEC_PKTLGTH_STRU *pPktlgth)
{
    u32 u32PktMaxLgth;
    u32 u32PktMinLgth;
    s32 ret;

    /* 判断是否已经初始化 */
    if((ret=socp_check_init()) != BSP_OK)
    {
        return ret;
    }
    /* 判断参数有效性 */
    if(NULL == pPktlgth)
    {
        socp_error("the parameter is NULL\n");
        return BSP_ERR_SOCP_NULL;
    }
    u32PktMaxLgth = pPktlgth->u32PktMax;
    u32PktMinLgth = pPktlgth->u32PktMin;

    if(0 == u32PktMaxLgth)
    {
        socp_error("the u32PktMaxLgth is 0.\n");
        return BSP_ERR_SOCP_NULL;
    }
    if (u32PktMaxLgth*1024 > SOCP_DEC_MAXPKT_MAX)
    {
        socp_error("u32PktMaxLgth 0x%x is too large!\n", u32PktMaxLgth);
        return BSP_ERR_SOCP_INVALID_PARA;
    }

    if (u32PktMinLgth > SOCP_DEC_MINPKT_MAX)
    {
        socp_error("u32PktMinLgth 0x%x is too large!\n", u32PktMinLgth);
        return BSP_ERR_SOCP_INVALID_PARA;
    }

    /* 配置解码通路包长度配置寄存器*/
    SOCP_REG_SETBITS(SOCP_REG_DEC_PKTLEN, 0, 12, u32PktMaxLgth);
    SOCP_REG_SETBITS(SOCP_REG_DEC_PKTLEN, 12, 5, u32PktMinLgth);

    return BSP_OK;
}
/* cov_verified_stop */

/*****************************************************************************
* 函 数 名  : bsp_socp_set_debug
*
* 功能描述  : 设置解码源通道debug模式函数
*
* 输入参数  : u32DecChanID  解码源通道ID
              u32DebugEn    debug模式使能标识
*
* 输出参数  :
*
* 返 回 值  : 设置成功与否的标识码
*****************************************************************************/
s32 bsp_socp_set_debug(u32 u32DecChanID, u32 u32DebugEn)
{
    u32 u32ChanID;
    u32 u32ChanType;
    s32 ret;

    /* 判断是否已经初始化 */
    if((ret=socp_check_init()) != BSP_OK)
    {
        return ret;
    }

    /* 判断通道ID是否有效 */
    u32ChanID = SOCP_REAL_CHAN_ID(u32DecChanID);
    u32ChanType = SOCP_REAL_CHAN_TYPE(u32DecChanID);
    if((ret=socp_check_chan_type(u32ChanType, SOCP_DECODER_SRC_CHAN)) != BSP_OK)
    {
        return ret;
    }
    if((ret=socp_check_chan_id(u32ChanID, SOCP_MAX_DECSRC_CHN)) != BSP_OK)
    {
        return ret;
    }
    if((ret=socp_check_decsrc_set(u32ChanID)) != BSP_OK)
    {
        return ret;
    }

    /* 判断该通道打开模式，没有打开的话，可以设置 */
    if(g_strSocpStat.sDecSrcChan[u32ChanID].u32ChanEn)
    {
        socp_error("decoder channel is open, can't set debug bit\n");
        return BSP_ERR_SOCP_SET_FAIL;
    }
    else
    {
		/*lint -save -e647*/
		SOCP_REG_SETBITS(SOCP_REG_DECSRC_BUFCFG0(u32ChanID), 29, 1, u32DebugEn);
		/*lint -restore +e647*/
    }

    return BSP_OK;
}


/*****************************************************************************
* 函 数 名  : bsp_socp_free_channel
*
* 功能描述  : 通道释放函数
*
* 输入参数  : u32ChanID       编解码通道指针
*
* 输出参数  : 无
*
* 返 回 值  : 释放成功与否的标识码
*****************************************************************************/
s32 bsp_socp_free_channel(u32 u32ChanID)
{
    u32 u32RealChanID;
    u32 u32ChanType;
    s32 ret;

    /* 判断是否已经初始化 */
    if((ret=socp_check_init()) != BSP_OK)
    {
        return ret;
    }
    /* 判断通道ID是否有效 */
    u32RealChanID = SOCP_REAL_CHAN_ID(u32ChanID);
    u32ChanType = SOCP_REAL_CHAN_TYPE(u32ChanID);

    if (SOCP_CODER_SRC_CHAN == u32ChanType)
    {
        SOCP_ENCSRC_CHAN_S *pChan;

        if((ret=socp_check_encsrc_chan_id(u32RealChanID)) != BSP_OK)
        {
            return ret;
        }
        if((ret=socp_check_encsrc_alloc(u32RealChanID)) != BSP_OK)
        {
            return ret;
        }

        pChan = &g_strSocpStat.sEncSrcChan[u32RealChanID];
        if (SOCP_CHN_ENABLE == pChan->u32ChanEn)
        {
            socp_error("chan 0x%x is running!\n", u32ChanID);
            return BSP_ERR_SOCP_CHAN_RUNNING;
        }

        (void)socp_reset_enc_chan(u32RealChanID);

        pChan->u32AllocStat = SOCP_CHN_UNALLOCATED;

        g_stSocpDebugInfo.sSocpDebugEncSrc.u32SocpFreeEncSrcCnt[u32RealChanID]++;
    }
    else if (SOCP_DECODER_DEST_CHAN == u32ChanType)
    {
        if((ret=socp_check_chan_id(u32RealChanID, SOCP_MAX_DECDST_CHN)) != BSP_OK)
        {
            return ret;
        }
        if((ret=socp_check_decdst_alloc(u32RealChanID)) != BSP_OK)
        {
            return ret;
        }

        /* 设置数据类型无效 */
		/*lint -save -e647*/
        SOCP_REG_SETBITS(SOCP_REG_DECDEST_BUFCFG(u32RealChanID), 24, 8, 0xff);
		/*lint -restore +e647*/

        g_strSocpStat.sDecDstChan[u32RealChanID].u32AllocStat = SOCP_CHN_UNALLOCATED;

        g_stSocpDebugInfo.sSocpDebugDecDst.u32SocpFreeDecDstCnt[u32RealChanID]++;
    }
    else
    {
        socp_error("invalid chan type 0x%x!\n", u32ChanType);
        return BSP_ERR_SOCP_INVALID_CHAN;
    }

    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : bsp_socp_chan_soft_reset
*
* 功能描述  : 通道软复位函数
*
* 输入参数  : u32ChanID       编解码通道ID
*
* 输出参数  : 无
*
* 返 回 值  : 释放成功与否的标识码
*****************************************************************************/
s32 bsp_socp_chan_soft_reset(u32 u32ChanID)
{
    u32 u32RealChanID;
    u32 u32ChanType;
    s32 ret;

    /* 判断是否已经初始化 */
    if((ret=socp_check_init()) != BSP_OK)
    {
        return ret;
    }

    /* 判断通道ID是否有效 */
    u32RealChanID = SOCP_REAL_CHAN_ID(u32ChanID);
    u32ChanType = SOCP_REAL_CHAN_TYPE(u32ChanID);
    /* 编码源通道复位暂只处理动态分配及LTE DSP/BBP通道 */
    /* 其余通道的复位操作可能涉及特殊的处理，需后续添加接口 */
    if (SOCP_CODER_SRC_CHAN == u32ChanType)
    {
        if((ret=socp_check_encsrc_chan_id(u32RealChanID)) != BSP_OK)
        {
            return ret;
        }
        if((ret=socp_check_encsrc_alloc(u32RealChanID)) != BSP_OK)
        {
            return ret;
        }
        (void)socp_reset_enc_chan(u32ChanID);
        g_stSocpDebugInfo.sSocpDebugEncSrc.u32SocpSoftResetEncSrcCnt[u32RealChanID]++;
    }
    else if (SOCP_DECODER_SRC_CHAN == u32ChanType)
    {
        if((ret=socp_check_chan_id(u32RealChanID, SOCP_MAX_DECSRC_CHN)) != BSP_OK)
        {
            return ret;
        }
        if((ret=socp_check_decsrc_set(u32RealChanID)) != BSP_OK)
        {
            return ret;
        }

        (void)socp_reset_dec_chan(u32RealChanID);

        g_stSocpDebugInfo.sSocpDebugDecSrc.u32SocpSoftResetDecSrcCnt[u32RealChanID]++;
    }
    else
    {
        socp_error("invalid chan type: 0x%x!\n", u32ChanType);
        return BSP_ERR_SOCP_INVALID_CHAN;
    }

    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : bsp_socp_start
*
* 功能描述  : 编码或者解码启动函数
*
* 输入参数  : u32SrcChanID      通道ID
* 输出参数  :
*
* 返 回 值  : 启动成功与否的标识码
*****************************************************************************/
s32 bsp_socp_start(u32 u32SrcChanID)
{
    u32 u32RealChanID;
    u32 u32ChanType;
    u32 u32Logic = (u32)1;
    u32 u32DstId;
    u32 i;
    u32 IntIDMask = 0;
    s32 ret;


    /* 判断是否已经初始化 */
    if((ret=socp_check_init()) != BSP_OK)
    {
        return ret;
    }
    /* 判断通道ID是否有效 */
    u32RealChanID = SOCP_REAL_CHAN_ID(u32SrcChanID);
    u32ChanType = SOCP_REAL_CHAN_TYPE(u32SrcChanID);

    /* 编码通道 */
    if (SOCP_CODER_SRC_CHAN == u32ChanType)
    {
        if (u32RealChanID < SOCP_MAX_ENCSRC_CHN)
        {
            if((ret=socp_check_encsrc_chan_id(u32RealChanID)) != BSP_OK)
            {
                return ret;
            }
            if((ret=socp_check_encsrc_alloc(u32RealChanID)) != BSP_OK)
            {
                return ret;
            }
        }
        else
        {
            socp_error("enc src 0x%x is invalid!\n", u32SrcChanID);
            return BSP_ERR_SOCP_INVALID_CHAN;
        }
	/*lint -save -e647*/
        u32DstId = SOCP_REG_GETBITS(SOCP_REG_ENCSRC_BUFCFG1(u32RealChanID), 4, 4);

        if (SOCP_CHN_SET != g_strSocpStat.sEncDstChan[u32DstId].u32SetStat)
        {
            socp_error("enc dst chan is invalid!\n");
            return BSP_ERR_SOCP_DEST_CHAN;
        }

        /* 先清中断，再打开中断*/
        SOCP_REG_SETBITS(SOCP_REG_ENC_RAWINT1, u32RealChanID, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_APP_MASK1, u32RealChanID, 1, 0);
        if(socp_version >= SOCP_206_VERSION)
        {
            SOCP_REG_SETBITS(SOCP_REG_ENCDEST_SBCFG(u32DstId),0,1,1);   // 启动编码目的通道
        }

        if (SOCP_ENCSRC_CHNMODE_LIST == g_strSocpStat.sEncSrcChan[u32RealChanID].eChnMode)
        {
            SOCP_REG_SETBITS(SOCP_REG_ENC_RAWINT3, u32RealChanID, 1, 1);
            /* 保持RD中断屏蔽 */
            //SOCP_REG_SETBITS(SOCP_REG_APP_MASK3, u32RealChanID, 1, 0);
        }

        /* 设置打开状态*/
        SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG1(u32RealChanID), 0, 1, 1);
        if(u32RealChanID < SOCP_MAX_ENCSRC_CHN)
        {
            g_strSocpStat.sEncSrcChan[u32RealChanID].u32ChanEn = SOCP_CHN_ENABLE;
            g_stSocpDebugInfo.sSocpDebugEncSrc.u32SocpStartEncSrcCnt[u32RealChanID]++;
        }
    }
    else if (SOCP_DECODER_SRC_CHAN == u32ChanType)
    {
        if((ret=socp_check_chan_id(u32RealChanID, SOCP_MAX_DECSRC_CHN)) != BSP_OK)
        {
            return ret;
        }
        if((ret=socp_check_decsrc_set(u32RealChanID)) != BSP_OK)
        {
            return ret;
        }

        /* 打开rd完成中断*/
        if (SOCP_DECSRC_CHNMODE_LIST == g_strSocpStat.sDecSrcChan[u32RealChanID].eChnMode)
        {
            SOCP_REG_SETBITS(SOCP_REG_DEC_RAWINT1, u32RealChanID, 1, 1);
            SOCP_REG_SETBITS(SOCP_REG_DEC_MASK1, u32RealChanID, 1, 0);
        }

        for (i = 1; i < SOCP_DEC_SRCINT_NUM; i++)
        {
            IntIDMask  = SOCP_REG_GETBITS(SOCP_REG_DEC_RAWINT1, i * 4, 4);
            IntIDMask |= 1 << u32RealChanID;
            SOCP_REG_SETBITS(SOCP_REG_DEC_RAWINT1, i * 4, 4, IntIDMask);

            IntIDMask  = SOCP_REG_GETBITS(SOCP_REG_DEC_MASK1, i * 4, 4);
            IntIDMask &= ~(u32Logic << u32RealChanID);
            SOCP_REG_SETBITS(SOCP_REG_DEC_MASK1, i * 4, 4, IntIDMask);
        }

        /* 设置打开状态*/
        SOCP_REG_SETBITS(SOCP_REG_DECSRC_BUFCFG0(u32RealChanID), 30, 1, 1);
        g_strSocpStat.sDecSrcChan[u32RealChanID].u32ChanEn = SOCP_CHN_ENABLE;

        g_stSocpDebugInfo.sSocpDebugDecSrc.u32SocpStartDecSrcCnt[u32RealChanID]++;
    }
    else
    {
        socp_error("invalid chan type: 0x%x!\n", u32ChanType);
        return BSP_ERR_SOCP_INVALID_CHAN;
    }
	/*lint -restore +e647*/
    return BSP_OK;
}



/*****************************************************************************
* 函 数 名  : bsp_socp_stop
*
* 功能描述  : 编码或者解码停止函数
*
* 输入参数  : u32SrcChanID      通道ID
*
* 输出参数  :
*
* 返 回 值  : 停止成功与否的标识码
*****************************************************************************/

s32 bsp_socp_stop(u32 u32SrcChanID)
{
    u32 u32RealChanID;
    u32 u32ChanType;
    u32 IntIDMask = 0;
    u32 i;
    s32 ret;


    /* 判断是否已经初始化 */
    if((ret=socp_check_init()) != BSP_OK)
    {
        return ret;
    }

    /* 判断通道ID是否有效 */
    u32RealChanID = SOCP_REAL_CHAN_ID(u32SrcChanID);
    u32ChanType = SOCP_REAL_CHAN_TYPE(u32SrcChanID);

    /* 编码通道 */
    if (SOCP_CODER_SRC_CHAN == u32ChanType)
    {
        if (u32RealChanID < SOCP_MAX_ENCSRC_CHN)
        {
            if((ret=socp_check_encsrc_chan_id(u32RealChanID)) != BSP_OK)
            {
                return ret;
            }
            if((ret=socp_check_encsrc_alloc(u32RealChanID)) != BSP_OK)
            {
                return ret;
            }
        }
        else
        {
            socp_error("enc src 0x%x is invalid!\n", u32SrcChanID);
            return BSP_ERR_SOCP_INVALID_CHAN;
        }
	/*lint -save -e647*/
        SOCP_REG_SETBITS(SOCP_REG_APP_MASK1, u32RealChanID, 1, 1);
        if (SOCP_ENCSRC_CHNMODE_LIST == g_strSocpStat.sEncSrcChan[u32RealChanID].eChnMode)
        {
            SOCP_REG_SETBITS(SOCP_REG_APP_MASK3, u32RealChanID, 1, 1);
        }

        /* 设置通道关闭状态*/
        SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG1(u32RealChanID), 0, 1, 0);
        if (u32RealChanID < SOCP_MAX_ENCSRC_CHN)
        {
            g_strSocpStat.sEncSrcChan[u32RealChanID].u32ChanEn = SOCP_CHN_DISABLE;
            g_stSocpDebugInfo.sSocpDebugEncSrc.u32SocpStopEncSrcCnt[u32RealChanID]++;
        }
    }
    else if (SOCP_DECODER_SRC_CHAN == u32ChanType)
    {
        if((ret=socp_check_chan_id(u32RealChanID, SOCP_MAX_DECSRC_CHN)) != BSP_OK)
        {
            return ret;
        }
        if((ret=socp_check_decsrc_set(u32RealChanID)) != BSP_OK)
        {
            return ret;
        }

        /* 关闭中断*/
        if (SOCP_DECSRC_CHNMODE_LIST == g_strSocpStat.sDecSrcChan[u32RealChanID].eChnMode)
        {
            SOCP_REG_SETBITS(SOCP_REG_DEC_CORE0MASK0, u32RealChanID, 1, 1);
        }

        for (i = 1; i < SOCP_DEC_SRCINT_NUM; i++)
        {
            IntIDMask  = SOCP_REG_GETBITS(SOCP_REG_DEC_MASK1, i * 4, 4);
            IntIDMask |= 1 << u32RealChanID;
            SOCP_REG_SETBITS(SOCP_REG_DEC_CORE0MASK0, i * 4, 4, IntIDMask);
        }

        /* 设置通道关闭状态*/
        SOCP_REG_SETBITS(SOCP_REG_DECSRC_BUFCFG0(u32RealChanID), 30, 1,0);
        g_strSocpStat.sDecSrcChan[u32RealChanID].u32ChanEn = SOCP_CHN_DISABLE;

        g_stSocpDebugInfo.sSocpDebugDecSrc.u32SocpStopDecSrcCnt[u32RealChanID]++;
    }
    else
    {
        socp_error("invalid chan type: 0x%x!\n", u32ChanType);
        return BSP_ERR_SOCP_INVALID_CHAN;
    }
	/*lint -restore +e647*/
    return BSP_OK;
}


/*****************************************************************************
* 函 数 名  : bsp_socp_register_event_cb
*
* 功能描述  : 异常事件回调函数注册函数
*
* 输入参数  : u32ChanID      通道ID
*             EventCB        异常事件的回调函数
* 输出参数  :
*
* 返 回 值  : 注册成功与否的标识码
*****************************************************************************/
s32 bsp_socp_register_event_cb(u32 u32ChanID, socp_event_cb EventCB)
{
    u32  u32RealChanID;
    u32  u32ChanType;
    s32 ret;

    /* 判断是否已经初始化 */
    if((ret=socp_check_init()) != BSP_OK)
    {
        return ret;
    }

    /* 获取通道类型和实际的通道ID */
    u32RealChanID = SOCP_REAL_CHAN_ID(u32ChanID);
    u32ChanType = SOCP_REAL_CHAN_TYPE(u32ChanID);

    switch (u32ChanType)
    {
        case SOCP_CODER_SRC_CHAN:      /* 编码源通道 */
        {
            if (u32RealChanID < SOCP_MAX_ENCSRC_CHN)
            {
                if((ret=socp_check_encsrc_chan_id(u32RealChanID)) != BSP_OK)
                {
                    return ret;
                }
                if((ret=socp_check_encsrc_alloc(u32RealChanID)) != BSP_OK)
                {
                    return ret;
                }
                g_strSocpStat.sEncSrcChan[u32RealChanID].event_cb = EventCB;

                g_stSocpDebugInfo.sSocpDebugEncSrc.u32SocpRegEventEncSrcCnt[u32RealChanID]++;
            }
            break;
        }
        case SOCP_CODER_DEST_CHAN:       /* 编码目的通道*/
        {
            if((ret=socp_check_chan_id(u32RealChanID, SOCP_MAX_ENCDST_CHN)) != BSP_OK)
            {
                return ret;
            }
            if((ret=socp_check_encdst_set(u32RealChanID)) != BSP_OK)
            {
                return ret;
            }

            g_strSocpStat.sEncDstChan[u32RealChanID].event_cb = EventCB;

            g_stSocpDebugInfo.sSocpDebugEncDst.u32SocpRegEventEncDstCnt[u32RealChanID]++;
            break;
        }
        case SOCP_DECODER_SRC_CHAN:       /* 解码源通道*/
        {
            if((ret=socp_check_chan_id(u32RealChanID, SOCP_MAX_DECSRC_CHN)) != BSP_OK)
            {
                return ret;
            }
            if((ret=socp_check_decsrc_set(u32RealChanID)) != BSP_OK)
            {
                return ret;
            }

            g_strSocpStat.sDecSrcChan[u32RealChanID].event_cb = EventCB;

            g_stSocpDebugInfo.sSocpDebugDecSrc.u32SocpRegEventDecSrcCnt[u32RealChanID]++;
            break;
        }
        case SOCP_DECODER_DEST_CHAN:       /* 解码目的通道*/
        {
            if((ret=socp_check_chan_id(u32RealChanID, SOCP_MAX_DECDST_CHN)) != BSP_OK)
            {
                return ret;
            }
            if((ret=socp_check_decdst_alloc(u32RealChanID)) != BSP_OK)
            {
                return ret;
            }

            g_strSocpStat.sDecDstChan[u32RealChanID].event_cb = EventCB;

            g_stSocpDebugInfo.sSocpDebugDecDst.u32SocpRegEventDecDstCnt[u32RealChanID]++;
            break;
        }
        default:
        {
            socp_error("invalid chan type: 0x%x!\n", u32ChanType);
            return BSP_ERR_SOCP_INVALID_CHAN;
        }
    }

    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : bsp_socp_get_write_buff
*
* 功能描述  : 上层获取写数据buffer函数
*
* 输入参数  : u32SrcChanID    源通道ID
* 输出参数  : pBuff           获取的buffer
*
* 返 回 值  : 获取成功与否的标识码
*****************************************************************************/
s32 bsp_socp_get_write_buff(u32 u32SrcChanID, SOCP_BUFFER_RW_STRU *pBuff)
{
    u32 u32ChanID;
    u32 u32ChanType;
    u32 uPAddr;
    s32 ret;

    /* 判断是否已经初始化 */
    if((ret=socp_check_init()) != BSP_OK)
    {
        return ret;
    }

    /* 判断参数有效性 */
    if(NULL == pBuff)
    {
        socp_error("the parameter is NULL\n");
        return BSP_ERR_SOCP_NULL;
    }
    /* 获得实际通道id */
    u32ChanID   = SOCP_REAL_CHAN_ID(u32SrcChanID);
    u32ChanType = SOCP_REAL_CHAN_TYPE(u32SrcChanID);

    /* 编码通道 */
    if (SOCP_CODER_SRC_CHAN == u32ChanType)
    {
        g_stSocpDebugInfo.sSocpDebugEncSrc.u32SocpGetWBufEncSrcEtrCnt[u32ChanID]++;

        /* 检验通道id */
        if((ret=socp_check_encsrc_chan_id(u32ChanID)) != BSP_OK)
        {
            return ret;
        }
        if((ret=socp_check_encsrc_alloc(u32ChanID)) != BSP_OK)
        {
            return ret;
        }

        /* 根据读写指针获取buffer */
        SOCP_REG_READ(SOCP_REG_ENCSRC_BUFRPTR(u32ChanID), uPAddr);
        g_strSocpStat.sEncSrcChan[u32ChanID].sEncSrcBuf.u32Read = uPAddr;
        SOCP_REG_READ(SOCP_REG_ENCSRC_BUFWPTR(u32ChanID), uPAddr);
        g_strSocpStat.sEncSrcChan[u32ChanID].sEncSrcBuf.u32Write= uPAddr;
        socp_get_idle_buffer(&g_strSocpStat.sEncSrcChan[u32ChanID].sEncSrcBuf, pBuff);
        g_stSocpDebugInfo.sSocpDebugEncSrc.u32SocpGetWBufEncSrcSucCnt[u32ChanID]++;
    }
    else if (SOCP_DECODER_SRC_CHAN == u32ChanType) /* 解码通道 */
    {
        g_stSocpDebugInfo.sSocpDebugDecSrc.u32SocpGetWBufDecSrcEtrCnt[u32ChanID]++;

        /* 检验通道id */
        if((ret=socp_check_chan_id(u32ChanID, SOCP_MAX_DECSRC_CHN)) != BSP_OK)
        {
            return ret;
        }
        if((ret=socp_check_decsrc_set(u32ChanID)) != BSP_OK)
        {
            return ret;
        }
        SOCP_REG_READ(SOCP_REG_DECSRC_BUFRPTR(u32ChanID), uPAddr);
        g_strSocpStat.sDecSrcChan[u32ChanID].sDecSrcBuf.u32Read = uPAddr;
        SOCP_REG_READ(SOCP_REG_DECSRC_BUFWPTR(u32ChanID), uPAddr);
        g_strSocpStat.sDecSrcChan[u32ChanID].sDecSrcBuf.u32Write= uPAddr;
        socp_get_idle_buffer(&g_strSocpStat.sDecSrcChan[u32ChanID].sDecSrcBuf, pBuff);
        g_stSocpDebugInfo.sSocpDebugDecSrc.u32SocpGetWBufDecSrcSucCnt[u32ChanID]++;
    }
    else
    {
        socp_error("invalid chan type: 0x%x!\n", u32ChanType);
        return BSP_ERR_SOCP_INVALID_CHAN;
    }

    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : bsp_socp_write_done
*
* 功能描述  : 写数据完成函数
*
* 输入参数  : u32SrcChanID    源通道ID
              u32WrtSize      写入数据的长度
*
* 输出参数  :
*
* 返 回 值  : 操作完成与否的标识码
*****************************************************************************/
s32 bsp_socp_write_done(u32 u32SrcChanID, u32 u32WrtSize)
{
    u32 u32ChanID;
    u32 u32ChanType;
    u32 u32WrtPad;
    SOCP_BUFFER_RW_STRU RwBuff;
    u32  uPAddr;
    s32 ret;

    /* 判断是否已经初始化 */
    if((ret=socp_check_init()) != BSP_OK)
    {
        return ret;
    }

    /* 判断参数有效性 */
    if(0 == u32WrtSize)
    {
        socp_error("the u32WrtSize is 0\n");
        return BSP_ERR_SOCP_NULL;
    }

    /* 获得实际通道id */
    u32ChanID   = SOCP_REAL_CHAN_ID(u32SrcChanID);
    u32ChanType = SOCP_REAL_CHAN_TYPE(u32SrcChanID);

    /* 编码通道 */
    if (SOCP_CODER_SRC_CHAN == u32ChanType)
    {
        SOCP_ENCSRC_CHAN_S *pChan;

        g_stSocpDebugInfo.sSocpDebugEncSrc.u32socp_write_doneEncSrcEtrCnt[u32ChanID]++;

        /* 检验通道id */
        if((ret=socp_check_encsrc_chan_id(u32ChanID)) != BSP_OK)
        {
            return ret;
        }
        if((ret=socp_check_encsrc_alloc(u32ChanID)) != BSP_OK)
        {
            return ret;
        }

        pChan = &g_strSocpStat.sEncSrcChan[u32ChanID];
        u32WrtPad = u32WrtSize % 8;
        if (0 != u32WrtPad)
        {
            u32WrtSize += (8 - u32WrtPad);
        }

        SOCP_REG_READ(SOCP_REG_ENCSRC_BUFWPTR(u32ChanID), uPAddr);
        pChan->sEncSrcBuf.u32Write= uPAddr;
        SOCP_REG_READ(SOCP_REG_ENCSRC_BUFRPTR(u32ChanID), uPAddr);
        pChan->sEncSrcBuf.u32Read =uPAddr;

        socp_get_idle_buffer(&pChan->sEncSrcBuf, &RwBuff);

        if (RwBuff.u32Size + RwBuff.u32RbSize < u32WrtSize)
        {
            socp_error("RwBuff is not enough, u32WrtSize=0x%x\n", u32WrtSize);
            g_stSocpDebugInfo.sSocpDebugEncSrc.u32socp_write_doneEncSrcFailCnt[u32ChanID]++;
            return BSP_ERR_SOCP_INVALID_PARA;
        }

        /* 设置读写指针 */
        socp_write_done(&pChan->sEncSrcBuf, u32WrtSize);

        /* 写入写指针到写指针寄存器*/
        uPAddr = pChan->sEncSrcBuf.u32Write; /* [false alarm]:屏蔽Fortify错误 */
        SOCP_REG_WRITE(SOCP_REG_ENCSRC_BUFWPTR(u32ChanID), uPAddr);
        g_stSocpDebugInfo.sSocpDebugEncSrc.u32socp_write_doneEncSrcSucCnt[u32ChanID]++;
    }
    else  if(SOCP_DECODER_SRC_CHAN == u32ChanType) /* 解码通道 */
    {
        SOCP_DECSRC_CHAN_S  *pChan;

        g_stSocpDebugInfo.sSocpDebugDecSrc.u32socp_write_doneDecSrcEtrCnt[u32ChanID]++;

        /* 检验通道id */
        if((ret=socp_check_chan_id(u32ChanID, SOCP_MAX_DECSRC_CHN)) != BSP_OK)
        {
            return ret;
        }
        if((ret=socp_check_decsrc_set(u32ChanID)) != BSP_OK)
        {
            return ret;
        }
        pChan = &g_strSocpStat.sDecSrcChan[u32ChanID];
        SOCP_REG_READ(SOCP_REG_DECSRC_BUFWPTR(u32ChanID), uPAddr);
        pChan->sDecSrcBuf.u32Write= uPAddr;
        SOCP_REG_READ(SOCP_REG_DECSRC_BUFRPTR(u32ChanID), uPAddr);
        pChan->sDecSrcBuf.u32Read =uPAddr;
        socp_get_idle_buffer(&pChan->sDecSrcBuf, &RwBuff);

        if (RwBuff.u32Size + RwBuff.u32RbSize < u32WrtSize)
        {
            socp_error("RwBuff is not enough, u32WrtSize=0x%x\n", u32WrtSize);
            g_stSocpDebugInfo.sSocpDebugDecSrc.u32socp_write_doneDecSrcFailCnt[u32ChanID]++;

            return BSP_ERR_SOCP_INVALID_PARA;
        }

        /* 设置读写指针 */
        socp_write_done(&pChan->sDecSrcBuf, u32WrtSize);

        /* 写入写指针到写指针寄存器*/
        uPAddr = pChan->sDecSrcBuf.u32Write; /* [false alarm]:屏蔽Fortify错误 */
        SOCP_REG_WRITE(SOCP_REG_DECSRC_BUFWPTR(u32ChanID), uPAddr);
        g_stSocpDebugInfo.sSocpDebugDecSrc.u32socp_write_doneDecSrcSucCnt[u32ChanID]++;
    }
    else
    {
        socp_error("invalid chan type: 0x%x!\n", u32ChanType);
        return BSP_ERR_SOCP_INVALID_CHAN;
    }

    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : bsp_socp_register_rd_cb
*
* 功能描述  : RDbuffer回调函数注册函数
*
* 输入参数  : u32SrcChanID    源通道ID
              RdCB            RDbuffer完成回调函数
*
* 输出参数  :
*
* 返 回 值  : 注册成功与否的标识码
*****************************************************************************/
s32 bsp_socp_register_rd_cb(u32 u32SrcChanID, socp_rd_cb RdCB)
{
    u32 u32RealChanID;
    u32 u32ChanType;
    s32 ret;

    /* 判断是否已经初始化 */
    if((ret=socp_check_init()) != BSP_OK)
    {
        return ret;
    }

    /* 获取通道类型和实际的通道ID */
    u32RealChanID = SOCP_REAL_CHAN_ID(u32SrcChanID);
    u32ChanType = SOCP_REAL_CHAN_TYPE(u32SrcChanID);

    /* 编码通道 */
    if (SOCP_CODER_SRC_CHAN == u32ChanType)
    {
        if((ret=socp_check_encsrc_chan_id(u32RealChanID)) != BSP_OK)
        {
            return ret;
        }
        if((ret=socp_check_encsrc_alloc(u32RealChanID)) != BSP_OK)
        {
            return ret;
        }

        if (SOCP_ENCSRC_CHNMODE_LIST == g_strSocpStat.sEncSrcChan[u32RealChanID].eChnMode)
        {
            /* 设置对应通道的回调函数*/
            g_strSocpStat.sEncSrcChan[u32RealChanID].rd_cb = RdCB;
        }
        else
        {
            socp_error("invalid chan mode!\n");
            return BSP_ERR_SOCP_CHAN_MODE;
        }

        g_stSocpDebugInfo.sSocpDebugEncSrc.u32SocpRegRdCBEncSrcCnt[u32RealChanID]++;
    }
    else
    {
        socp_error("invalid chan type: 0x%x!\n", u32ChanType);
        return BSP_ERR_SOCP_INVALID_CHAN;
    }

    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : bsp_socp_get_rd_buffer
*
* 功能描述  : 获取RDbuffer函数
*
* 输入参数  : u32SrcChanID    源通道ID
*
* 输出参数  : pBuff           获取的RDbuffer
*
* 返 回 值  : 获取成功与否的标识码
*****************************************************************************/
/* cov_verified_start */
s32 bsp_socp_get_rd_buffer(u32 u32SrcChanID, SOCP_BUFFER_RW_STRU *pBuff)
{
    u32 u32ChanID;
    u32 u32ChanType;
    u32 uPAddr;
    s32 ret;

    /* 判断是否已经初始化 */
    if((ret=socp_check_init()) != BSP_OK)
    {
        return ret;
    }

    /* 判断参数有效性 */
    if(NULL == pBuff)
    {
        socp_error("the parameter is NULL\n");
        return BSP_ERR_SOCP_NULL;
    }

    /* 获得实际通道id */
    u32ChanID   = SOCP_REAL_CHAN_ID(u32SrcChanID);
    u32ChanType = SOCP_REAL_CHAN_TYPE(u32SrcChanID);

    /* 编码通道 */
    if (SOCP_CODER_SRC_CHAN == u32ChanType)
    {
        g_stSocpDebugInfo.sSocpDebugEncSrc.u32SocpGetRdBufEncSrcEtrCnt[u32ChanID]++;

        /* 检验通道id */
        if((ret=socp_check_encsrc_chan_id(u32ChanID)) != BSP_OK)
        {
            return ret;
        }
        if((ret=socp_check_encsrc_alloc(u32ChanID)) != BSP_OK)
        {
            return ret;
        }

        /* 根据读写指针获取buffer */
        SOCP_REG_READ(SOCP_REG_ENCSRC_RDQRPTR(u32ChanID), uPAddr);
        g_strSocpStat.sEncSrcChan[u32ChanID].sRdBuf.u32Read = uPAddr;
        SOCP_REG_READ(SOCP_REG_ENCSRC_RDQWPTR(u32ChanID), uPAddr);
        g_strSocpStat.sEncSrcChan[u32ChanID].sRdBuf.u32Write= uPAddr;
        socp_get_data_buffer(&g_strSocpStat.sEncSrcChan[u32ChanID].sRdBuf, pBuff);
        g_stSocpDebugInfo.sSocpDebugEncSrc.u32SocpGetRdBufEncSrcSucCnt[u32ChanID]++;
    }
    else
    {
        socp_error("invalid chan type(0x%x)\n", u32ChanType);
        return BSP_ERR_SOCP_INVALID_CHAN;
    }

    return BSP_OK;
}


/*****************************************************************************
* 函 数 名  : bsp_socp_read_rd_done
*
* 功能描述  : 读取RDbuffer数据完成函数
*
* 输入参数  : u32SrcChanID   源通道ID
              u32RDSize      读取的RDbuffer数据长度
*
* 输出参数  :
*
* 返 回 值  : 读取成功与否的标识码
*****************************************************************************/
s32 bsp_socp_read_rd_done(u32 u32SrcChanID, u32 u32RDSize)
{
    u32 u32ChanID;
    u32 u32ChanType;
    SOCP_BUFFER_RW_STRU RwBuff;
    u32  uPAddr;
    s32 ret;

    /* 判断是否已经初始化 */
    if((ret=socp_check_init()) != BSP_OK)
    {
        return ret;
    }

    /* 判断参数有效性 */
    if(0 == u32RDSize)
    {
        socp_error("the u32RDSize is 0\n");
        return BSP_ERR_SOCP_NULL;
    }

    /* 获得实际通道id */
    u32ChanID   = SOCP_REAL_CHAN_ID(u32SrcChanID);
    u32ChanType = SOCP_REAL_CHAN_TYPE(u32SrcChanID);

    /* 编码通道 */
    if (SOCP_CODER_SRC_CHAN == u32ChanType)
    {
        SOCP_ENCSRC_CHAN_S *pChan;

        g_stSocpDebugInfo.sSocpDebugEncSrc.u32SocpReadRdDoneEncSrcEtrCnt[u32ChanID]++;

        /* 检验通道id */
        if((ret=socp_check_encsrc_chan_id(u32ChanID)) != BSP_OK)
        {
            return ret;
        }
        if((ret=socp_check_encsrc_alloc(u32ChanID)) != BSP_OK)
        {
            return ret;
        }

        pChan = &g_strSocpStat.sEncSrcChan[u32ChanID];
        g_strSocpStat.sEncSrcChan[u32ChanID].u32LastRdSize = 0;

        /* 设置读写指针 */
        SOCP_REG_READ(SOCP_REG_ENCSRC_RDQWPTR(u32ChanID), uPAddr);
        pChan->sRdBuf.u32Write = uPAddr;
        SOCP_REG_READ(SOCP_REG_ENCSRC_RDQRPTR(u32ChanID), uPAddr);
        pChan->sRdBuf.u32Read  = uPAddr;
        socp_get_data_buffer(&pChan->sRdBuf, &RwBuff);

        if (RwBuff.u32Size + RwBuff.u32RbSize < u32RDSize)
        {
            socp_error("RwBuff is not enough, u32RDSize=0x%x\n",u32RDSize);
            g_stSocpDebugInfo.sSocpDebugEncSrc.u32SocpReadRdDoneEncSrcFailCnt[u32ChanID]++;

            return BSP_ERR_SOCP_INVALID_PARA;
        }

        socp_read_done(&pChan->sRdBuf, u32RDSize);

        /* 写入读指针到读指针寄存器*/
        uPAddr= pChan->sRdBuf.u32Read; /* [false alarm]:屏蔽Fortify错误 */
        SOCP_REG_WRITE(SOCP_REG_ENCSRC_RDQRPTR(u32ChanID), uPAddr);
        g_stSocpDebugInfo.sSocpDebugEncSrc.u32SocpReadRdDoneEncSrcSucCnt[u32ChanID]++;
    }
    else
    {
        socp_error("invalid chan type(0x%x)", u32ChanType);
        return BSP_ERR_SOCP_INVALID_CHAN;
    }

    return BSP_OK;
}
/* cov_verified_stop */

//以下目的通道专用
/*****************************************************************************
* 函 数 名  : bsp_socp_register_read_cb
*
* 功能描述  : 读数据回调函数注册函数
*
* 输入参数  : u32DestChanID  目的通道邋ID
              ReadCB         读数据回调函数
*
* 输出参数  :
*
* 返 回 值  : 注册成功与否的标识码
*****************************************************************************/
s32 bsp_socp_register_read_cb(u32 u32DestChanID, socp_read_cb ReadCB)
{
    u32 u32RealChanID;
    u32 u32ChanType;
    s32 ret;

    /* 判断是否已经初始化 */
    if((ret=socp_check_init()) != BSP_OK)
    {
        return ret;
    }

    /* 获取通道类型和实际的通道ID */
    u32RealChanID = SOCP_REAL_CHAN_ID(u32DestChanID);
    u32ChanType = SOCP_REAL_CHAN_TYPE(u32DestChanID);

    if (SOCP_DECODER_DEST_CHAN == u32ChanType) /* 解码通道 */
    {
        if((ret=socp_check_chan_id(u32RealChanID, SOCP_MAX_DECDST_CHN)) != BSP_OK)
        {
            return ret;
        }
        if((ret=socp_check_decdst_alloc(u32RealChanID)) != BSP_OK)
        {
            return ret;
        }

        g_strSocpStat.sDecDstChan[u32RealChanID].read_cb = ReadCB;

        g_stSocpDebugInfo.sSocpDebugDecDst.u32SocpRegReadCBDecDstCnt[u32RealChanID]++;
    }
    else if (SOCP_CODER_DEST_CHAN == u32ChanType)/* 编码通道 */
    {
        if((ret=socp_check_chan_id(u32RealChanID, SOCP_MAX_ENCDST_CHN)) != BSP_OK)
        {
            return ret;
        }
        if((ret=socp_check_encdst_set(u32RealChanID)) != BSP_OK)
        {
            return ret;
        }

        /* 设置对应通道的回调函数*/
        g_strSocpStat.sEncDstChan[u32RealChanID].read_cb = ReadCB;

        g_stSocpDebugInfo.sSocpDebugEncDst.u32SocpRegReadCBEncDstCnt[u32RealChanID]++;
    }
    else
    {
        socp_error("invalid chan type: 0x%x!\n", u32ChanType);
        return BSP_ERR_SOCP_INVALID_CHAN;
    }

    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : bsp_socp_get_read_buff
*
* 功能描述  : 获取读数据buffer函数
*
* 输入参数  : u32DestChanID  目的通道buffer

* 输出参数  : pBuffer        获取的读数据buffer
*
* 返 回 值  : 获取成功与否的标识码
*****************************************************************************/
s32 bsp_socp_get_read_buff(u32 u32DestChanID, SOCP_BUFFER_RW_STRU *pBuffer)
{
    u32 u32ChanID;
    u32 u32ChanType;
    u32  uPAddr;
    s32 ret;

    /* 判断是否已经初始化 */
    if((ret=socp_check_init()) != BSP_OK)
    {
        return ret;
    }

    /* 判断参数有效性 */
    if(NULL == pBuffer)
    {
        socp_error("the parameter is NULL\n");
        return BSP_ERR_SOCP_NULL;
    }

    /* 获得实际通道id */
    u32ChanID   = SOCP_REAL_CHAN_ID(u32DestChanID);
    u32ChanType = SOCP_REAL_CHAN_TYPE(u32DestChanID);
    pBuffer->u32Size   = 0;
    pBuffer->u32RbSize = 0;

    if (SOCP_DECODER_DEST_CHAN == u32ChanType) /* 解码通道 */
    {
        g_stSocpDebugInfo.sSocpDebugDecDst.u32SocpGetReadBufDecDstEtrCnt[u32ChanID]++;

        /* 检验通道id */
        if((ret=socp_check_chan_id(u32ChanID, SOCP_MAX_DECDST_CHN)) != BSP_OK)
        {
            return ret;
        }
        if((ret=socp_check_decdst_alloc(u32ChanID)) != BSP_OK)
        {
            return ret;
        }
        /* 根据读写指针获取buffer */
        SOCP_REG_READ(SOCP_REG_DECDEST_BUFRPTR(u32ChanID), uPAddr);
        g_strSocpStat.sDecDstChan[u32ChanID].sDecDstBuf.u32Read = uPAddr;
        SOCP_REG_READ(SOCP_REG_DECDEST_BUFWPTR(u32ChanID), uPAddr);
        g_strSocpStat.sDecDstChan[u32ChanID].sDecDstBuf.u32Write= uPAddr;
        socp_get_data_buffer(&g_strSocpStat.sDecDstChan[u32ChanID].sDecDstBuf, pBuffer);
        g_stSocpDebugInfo.sSocpDebugDecDst.u32SocpGetReadBufDecDstSucCnt[u32ChanID]++;
    }
    else if (SOCP_CODER_DEST_CHAN == u32ChanType)
    {

        g_stSocpDebugInfo.sSocpDebugEncDst.u32SocpGetReadBufEncDstEtrCnt[u32ChanID]++;
        /*deflate使能获取deflate buffer*/
        if((SOCP_COMPRESS == g_strSocpStat.sEncDstChan[u32ChanID].struCompress.bcompress )
            &&(g_strSocpStat.sEncDstChan[u32ChanID].struCompress.ops.getbuffer))
        {
            return g_strSocpStat.sEncDstChan[u32ChanID].struCompress.ops.getbuffer(pBuffer);

        }

        /* 检验通道id */
        if((ret=socp_check_chan_id(u32ChanID, SOCP_MAX_ENCDST_CHN)) != BSP_OK)
        {
            return ret;
        }
        if((ret=socp_check_encdst_set(u32ChanID)) != BSP_OK)
        {
            return ret;
        }

        /* 根据读写指针获取buffer */
        SOCP_REG_READ(SOCP_REG_ENCDEST_BUFRPTR(u32ChanID), uPAddr);
        g_strSocpStat.sEncDstChan[u32ChanID].sEncDstBuf.u32Read = uPAddr;
        SOCP_REG_READ(SOCP_REG_ENCDEST_BUFWPTR(u32ChanID), uPAddr);
        g_strSocpStat.sEncDstChan[u32ChanID].sEncDstBuf.u32Write= uPAddr;
        socp_get_data_buffer(&g_strSocpStat.sEncDstChan[u32ChanID].sEncDstBuf, pBuffer);
        g_stSocpDebugInfo.sSocpDebugEncDst.u32SocpGetReadBufEncDstSucCnt[u32ChanID]++;


    }
    else
    {
        socp_error("invalid chan type: 0x%x!\n", u32ChanType);
        return BSP_ERR_SOCP_INVALID_CHAN;
    }

    return BSP_OK;
}


s32 socp_decode_read_data_done(u32 u32ChanID, u32 u32ReadSize)
{
    u32 TfMaskReg = 0;
    s32 ret;
    u32  uPAddr;
    SOCP_BUFFER_RW_STRU RwBuff;
    SOCP_DECDST_CHAN_S *pChan;
    unsigned long lock_flag;

    g_stSocpDebugInfo.sSocpDebugDecDst.u32socp_read_doneDecDstEtrCnt[u32ChanID]++;

    /* 检验通道id */
    if((ret=socp_check_chan_id(u32ChanID, SOCP_MAX_DECDST_CHN)) != BSP_OK)
    {
        return ret;
    }
    if((ret=socp_check_decdst_alloc(u32ChanID)) != BSP_OK)
    {
        return ret;
    }
    //added by yangzhi 2011.7.25
    TfMaskReg = SOCP_REG_DEC_CORE0MASK0;
    pChan = &g_strSocpStat.sDecDstChan[u32ChanID];
    SOCP_REG_READ(SOCP_REG_DECDEST_BUFWPTR(u32ChanID), uPAddr);
    pChan->sDecDstBuf.u32Write= uPAddr;
    SOCP_REG_READ(SOCP_REG_DECDEST_BUFRPTR(u32ChanID), uPAddr);
    pChan->sDecDstBuf.u32Read = uPAddr;
    socp_get_data_buffer(&pChan->sDecDstBuf, &RwBuff);

    if(RwBuff.u32Size + RwBuff.u32RbSize < u32ReadSize)
    {
        socp_error("RwBuff is not enough, u32ReadSize=0x%x!\n",u32ReadSize);
		
        spin_lock_irqsave(&lock, lock_flag);
        SOCP_REG_SETBITS(SOCP_REG_DEC_RAWINT0, u32ChanID, 1, 1);
        SOCP_REG_SETBITS(TfMaskReg, u32ChanID, 1, 0);
        spin_unlock_irqrestore(&lock, lock_flag);
        g_stSocpDebugInfo.sSocpDebugDecDst.u32socp_read_doneDecDstFailCnt[u32ChanID]++;

        return BSP_ERR_SOCP_INVALID_PARA;
    }

    /* 设置读写指针 */
    socp_read_done(&pChan->sDecDstBuf, u32ReadSize);

    /* 写入写指针到写指针寄存器*/
    uPAddr = pChan->sDecDstBuf.u32Read; /* [false alarm]:屏蔽Fortify错误 */
    SOCP_REG_WRITE(SOCP_REG_DECDEST_BUFRPTR(u32ChanID), uPAddr);

    spin_lock_irqsave(&lock, lock_flag);
    SOCP_REG_SETBITS(SOCP_REG_DEC_RAWINT0, u32ChanID, 1, 1);
    SOCP_REG_SETBITS(TfMaskReg, u32ChanID, 1, 0);
    spin_unlock_irqrestore(&lock, lock_flag);

    if (0 == u32ReadSize)
    {
        g_stSocpDebugInfo.sSocpDebugDecDst.u32socp_read_doneZeroDecDstCnt[u32ChanID]++;
    }
    else
    {
        g_stSocpDebugInfo.sSocpDebugDecDst.u32socp_read_doneValidDecDstCnt[u32ChanID]++;
    }

    g_stSocpDebugInfo.sSocpDebugDecDst.u32socp_read_doneDecDstSucCnt[u32ChanID]++;

    return BSP_OK;
}

u32 bsp_socp_mode(u32 u32ChanID)
{
    u32 u32modestate;

    /*lint -save -e647*/
    if(socp_version < SOCP_206_VERSION)
    {
        u32modestate = SOCP_REG_GETBITS(SOCP_REG_ENCDEST_SBCFG(u32ChanID),1,1);
    }
    else
    {
        u32modestate = SOCP_REG_GETBITS(SOCP_REG_ENCDEST_SBCFG(u32ChanID),2,1);
    }
    /*lint -restore +e647*/

    return u32modestate;
}

u32 bsp_socp_mode_ex(u32 u32ChanID)
{
    u32 u32modestate= 0;
    /*lint -save -e647*/
    if(socp_version < SOCP_206_VERSION)
    {
        u32modestate = SOCP_REG_GETBITS(SOCP_REG_ENCDEST_SBCFG(u32ChanID),0,2);
    }
    else
    {
        u32modestate = SOCP_REG_GETBITS(SOCP_REG_ENCDEST_SBCFG(u32ChanID),1,2);
    }
    /*lint -restore +e647*/
    return u32modestate;
}

s32 socp_encode_read_data_done(u32 u32ChanID, u32 u32ReadSize)
{
    s32 ret;
    u32  uPAddr;
    u32  uPAddr2;
    u32  uPAddr3;
    SOCP_BUFFER_RW_STRU RwBuff;
    unsigned long lock_flag;
    u32 curmodestate;
    SOCP_ENCDST_CHAN_S *pChan;

    g_stSocpDebugInfo.sSocpDebugEncDst.u32socp_read_doneEncDstEtrCnt[u32ChanID]++;

    /* 检验通道id */
    if((ret = socp_check_chan_id(u32ChanID, SOCP_MAX_ENCDST_CHN)) != BSP_OK)
    {
        return ret;
    }
    if((ret = socp_check_encdst_set(u32ChanID)) != BSP_OK)
    {
        return ret;
    }
    if(u32ChanID == 1)
    {
        g_stEncDstStat[g_ulEncDstStatCount].ulReadDoneStartSlice = bsp_get_slice_value();
    }
    /*判断deflate使能，deflate readdone*/
    if(( SOCP_COMPRESS == g_strSocpStat.sEncDstChan[u32ChanID].struCompress.bcompress)
        &&(g_strSocpStat.sEncDstChan[u32ChanID].struCompress.ops.readdone))
    {
        return g_strSocpStat.sEncDstChan[u32ChanID].struCompress.ops.readdone(u32ReadSize);

    }

    if (0 == u32ReadSize)
    {
        g_stSocpDebugInfo.sSocpDebugEncDst.u32socp_read_doneZeroEncDstCnt[u32ChanID]++;
    }
    else
    {
        g_stSocpDebugInfo.sSocpDebugEncDst.u32socp_read_doneValidEncDstCnt[u32ChanID]++;
    }

    spin_lock_irqsave(&lock, lock_flag);

    curmodestate = bsp_socp_mode_ex(u32ChanID);
    if(0 != curmodestate)   /* ctrl & state 不是阻塞模式*/
    {
        spin_unlock_irqrestore(&lock, lock_flag);
        socp_error("no block mode, curmodestate=0x%x\n",curmodestate);
        return BSP_OK;
    }

    pChan = &g_strSocpStat.sEncDstChan[u32ChanID];
    SOCP_REG_READ(SOCP_REG_ENCDEST_BUFWPTR(u32ChanID), uPAddr);
    pChan->sEncDstBuf.u32Write= uPAddr;
    SOCP_REG_READ(SOCP_REG_ENCDEST_BUFRPTR(u32ChanID), uPAddr);
    pChan->sEncDstBuf.u32Read = uPAddr;
    socp_get_data_buffer(&pChan->sEncDstBuf, &RwBuff);

    if(RwBuff.u32Size + RwBuff.u32RbSize < u32ReadSize)
    {
        SOCP_REG_SETBITS(SOCP_REG_ENC_RAWINT0, u32ChanID, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENC_MASK0, u32ChanID, 1, 0);
        /* overflow int */
        SOCP_REG_SETBITS(SOCP_REG_ENC_RAWINT2, u32ChanID + 16, 1, 1);
        SOCP_REG_SETBITS(SOCP_REG_ENC_MASK2, u32ChanID + 16, 1, 0);

        spin_unlock_irqrestore(&lock, lock_flag);
        g_stSocpDebugInfo.sSocpDebugEncDst.u32socp_read_doneEncDstFailCnt[u32ChanID]++;

        socp_error("RwBuff is not enough, u32ReadSize=0x%x!\n",u32ReadSize);
        return BSP_ERR_SOCP_INVALID_PARA;
    }

    /* 设置读写指针 */
    socp_read_done(&pChan->sEncDstBuf, u32ReadSize);

    /* 写入读指针到读指针寄存器*/
    uPAddr2 = pChan->sEncDstBuf.u32Read; /* [false alarm]:屏蔽Fortify错误 */
    /*lint -save -e732*/
    SOCP_REG_WRITE(SOCP_REG_ENCDEST_BUFRPTR(u32ChanID), uPAddr2);
    SOCP_REG_READ(SOCP_REG_ENCDEST_BUFRPTR(u32ChanID), uPAddr3);
    /*lint -restore +e732*/
    SOCP_DEBUG_TRACE(SOCP_DEBUG_READ_DONE, pChan->sEncDstBuf.u32Write, uPAddr, uPAddr2, uPAddr3);

    bsp_socp_data_send_continue(u32ChanID);

    spin_unlock_irqrestore(&lock, lock_flag);
    g_stSocpDebugInfo.sSocpDebugEncDst.u32socp_read_doneEncDstSucCnt[u32ChanID]++;

    if(u32ChanID == 1)
    {
        g_stEncDstStat[g_ulEncDstStatCount].ulReadDoneEndSlice = bsp_get_slice_value();
        g_ulEncDstStatCount = (g_ulEncDstStatCount+1)%SOCP_MAX_ENC_DST_COUNT;
    }

    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : bsp_socp_read_data_done
*
* 功能描述  : 读数据完成函数
*
* 输入参数  : u32DestChanID    目的通道ID
*             u32ReadSize      读取数据大小
* 输出参数  : 无
*
* 返 回 值  : 读数据成功与否的标识码
*****************************************************************************/
s32 bsp_socp_read_data_done(u32 u32DestChanID, u32 u32ReadSize)
{
    u32 u32ChanID;
    u32 u32ChanType;
    s32 ret;

    /* 判断是否已经初始化 */
    if((ret=socp_check_init()) != BSP_OK)
    {
        return ret;
    }

    /*根据MSP要求去掉该检测，保证可以更新0字节，2011-04-29*/
    //SOCP_CHECK_PARA(u32ReadSize);

    /* 获得实际通道id */
    u32ChanID   = SOCP_REAL_CHAN_ID(u32DestChanID);
    u32ChanType = SOCP_REAL_CHAN_TYPE(u32DestChanID);

    if (SOCP_DECODER_DEST_CHAN == u32ChanType) /* 解码通道 */
    {
        ret = socp_decode_read_data_done(u32ChanID, u32ReadSize);
    }
    else if (SOCP_CODER_DEST_CHAN == u32ChanType)/* 编码通道 */
    {
        ret = socp_encode_read_data_done(u32ChanID, u32ReadSize);
    }
    else
    {
        socp_error("invalid chan type: 0x%x!\n", u32ChanType);
        return BSP_ERR_SOCP_INVALID_CHAN;
    }

    return ret;

}

/*****************************************************************************
* 函 数 名  : bsp_socp_set_bbp_enable
*
* 功能描述  : 使能/禁止BPP LOG和数采
*
* 输入参数  : bEnable       使能标识
*
* 输出参数  : 无
*
* 返 回 值  : 读数据成功与否的标识码
*****************************************************************************/
/* cov_verified_start */
s32 bsp_socp_set_bbp_enable(int bEnable)
{
    if(bEnable)
    {
        BBP_REG_SETBITS(BBP_REG_CH_EN, 0, 1, 1);
    }
    else
    {
        BBP_REG_SETBITS(BBP_REG_CH_EN, 0, 1, 0);
    }
    return BSP_OK;
}


/*****************************************************************************
* 函 数 名  : bsp_socp_set_bbp_ds_mode
*
* 功能描述  : 设置BPP数采模式
*
* 输入参数  : eDsMode    数采模式
*
* 输出参数  : 无
*
* 返 回 值  :
*****************************************************************************/
s32 bsp_socp_set_bbp_ds_mode(SOCP_BBP_DS_MODE_ENUM_UIN32 eDsMode)
{
    if(eDsMode >= SOCP_BBP_DS_MODE_BUTT)
    {
        socp_error("invalid DS mode!\n");
        return BSP_ERR_SOCP_INVALID_PARA;
    }

    BBP_REG_SETBITS(BBP_REG_DS_CFG, 31, 1, eDsMode);
    return BSP_OK;
}
/* cov_verified_stop */

void bsp_socp_set_enc_dst_threshold(bool mode,u32 u32DestChanID)
{
    u32 bufLength;
    u32 threshold;

    u32DestChanID = SOCP_REAL_CHAN_ID(u32DestChanID);

    SOCP_REG_READ(SOCP_REG_ENCDEST_BUFCFG(u32DestChanID),bufLength);
    if(mode == true)/*true为需要打开延时上报的场景*/
    {
        threshold = (bufLength >> 2)*3;
    }
    else
    {
        threshold = 0x1000;
    }
    SOCP_REG_WRITE(SOCP_REG_ENCDEST_BUFTHRESHOLD(u32DestChanID),threshold);

    socp_crit("set encdst threshold success! 0x%x\n", threshold);

    return;
}

/*****************************************************************************
* 函 数 名  : bsp_socp_mode_change_chip_bugfix
*
* 功能描述  : 软件修复芯片在模式切换的时候需要读走一部分数据的bug
*
* 输入参数  : 通道号
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
void bsp_socp_mode_change_chip_bugfix(u32 chanid)
{
    s32 ret;
    u32 len;
    u32 bufdepth;
    u32 WrVal;
    u32 RdVal;
    u32 u32ChanID = SOCP_REAL_CHAN_ID(chanid);

    SOCP_REG_READ(SOCP_REG_ENCDEST_BUFWPTR(u32ChanID), WrVal);
    SOCP_REG_READ(SOCP_REG_ENCDEST_BUFRPTR(u32ChanID), RdVal);
    SOCP_REG_READ(SOCP_REG_ENCDEST_BUFCFG(u32ChanID), bufdepth);

    if(WrVal >= RdVal)
    {
        len = WrVal - RdVal;
    }
    else
    {
        len = WrVal + bufdepth - RdVal;
    }

    if(bufdepth - len < 64*1024)
    {
        ret = bsp_socp_read_data_done(chanid, len < 64*1024 ? len:64*1024);
        if (ret != BSP_OK)
        {
            socp_error("socp_update read ptr fail(0x%x)\n", ret);
        }
    }

}

/*****************************************************************************
* 函 数 名  : bsp_socp_encdst_set_cycle
*
* 功能描述  : SOCP循环模式设置
*
* 输入参数  : 通道号、模式
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
/*lint -save -e647*/
s32 bsp_socp_encdst_set_cycle(u32 chanid, u32 cycle)
{
    u32 u32modestate;
    unsigned long lock_flag;
    u32 u32ChanID = SOCP_REAL_CHAN_ID(chanid);
    u32 waittime = 10000;

    /* 关闭自动时钟门控，否则上报模式配置不生效 */
    SOCP_REG_SETBITS(SOCP_REG_CLKCTRL,0,1,0);

    if(socp_version < SOCP_206_VERSION)
    {
        u32modestate = SOCP_REG_GETBITS(SOCP_REG_ENCDEST_SBCFG(u32ChanID),1,1);
    }
    else
    {
        u32modestate = SOCP_REG_GETBITS(SOCP_REG_ENCDEST_SBCFG(u32ChanID),2,1);
    }

    if ((0==cycle||1==cycle) && u32modestate)
    {
        u32 i;

        spin_lock_irqsave(&lock, lock_flag);
        if(socp_version < SOCP_206_VERSION)
        {
            SOCP_REG_SETBITS(SOCP_REG_ENCDEST_SBCFG(u32ChanID),0,1,0);
        }
        else
        {
            SOCP_REG_SETBITS(SOCP_REG_ENCDEST_SBCFG(u32ChanID),1,1,0);
        }
        spin_unlock_irqrestore(&lock, lock_flag);

        for(i=0; i<waittime; i++)
        {
            if(socp_version < SOCP_206_VERSION)
            {
                u32modestate = SOCP_REG_GETBITS(SOCP_REG_ENCDEST_SBCFG(u32ChanID),1,1);
            }
            else
            {
                u32modestate = SOCP_REG_GETBITS(SOCP_REG_ENCDEST_SBCFG(u32ChanID),2,1);
            }
            if(0 == u32modestate)
            {
                break;
            }
        }

        if(waittime == i)
        {
            g_stSocpDebugInfo.sSocpDebugEncDst.u32SocpEncDstModeChangeFailCnt[u32ChanID]++;
            socp_error("set encdst cycle off timeout!\n");
            /* 关闭自动时钟门控，否则上报模式配置不生效 */
            SOCP_REG_SETBITS(SOCP_REG_CLKCTRL,0,1,1);
            return BSP_ERROR;
        }

        /*Drop data of socp dst buffer*/
        bsp_socp_mode_change_chip_bugfix(chanid);

        bsp_socp_data_send_manager(chanid, SOCP_DEST_DSM_ENABLE);

    }
    else if((2==cycle) && (!u32modestate))
    {
        u32 i;

        bsp_socp_data_send_manager(chanid, SOCP_DEST_DSM_DISABLE);

        spin_lock_irqsave(&lock, lock_flag);
        if(socp_version < SOCP_206_VERSION)
        {
            SOCP_REG_SETBITS(SOCP_REG_ENCDEST_SBCFG(u32ChanID),0,1,1);
        }
        else
        {
            SOCP_REG_SETBITS(SOCP_REG_ENCDEST_SBCFG(u32ChanID),1,1,1);
        }
        spin_unlock_irqrestore(&lock, lock_flag);


        for(i=0; i<waittime; i++)
        {
            if(socp_version < SOCP_206_VERSION)
            {
                u32modestate = SOCP_REG_GETBITS(SOCP_REG_ENCDEST_SBCFG(u32ChanID),1,1);
            }
            else
            {
                u32modestate = SOCP_REG_GETBITS(SOCP_REG_ENCDEST_SBCFG(u32ChanID),2,1);
            }
            if(1 == u32modestate)
            {
                break;
            }
        }

        if(waittime == i)
        {
            g_stSocpDebugInfo.sSocpDebugEncDst.u32SocpEncDstModeChangeFailCnt[u32ChanID]++;
            socp_error("set encdst cycle on timeout!\n");
            SOCP_REG_SETBITS(SOCP_REG_CLKCTRL,0,1,1);
            return BSP_ERROR;
        }
    }

    /* 关闭自动时钟门控，否则上报模式配置不生效 */
    SOCP_REG_SETBITS(SOCP_REG_CLKCTRL,0,1,1);
    return BSP_OK;
}
/*lint -restore +e647*/

/*****************************************************************************
* 函 数 名   : socp_enc_dst_stat
*
* 功能描述  : 获取socp打印信息
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 无
*****************************************************************************/
void show_socp_enc_dst_stat(void)
{
    int i ;
    int count = (g_ulEncDstStatCount+1)%SOCP_MAX_ENC_DST_COUNT;

    for(i=0;i<SOCP_MAX_ENC_DST_COUNT;i++)
    {
        socp_crit("\r stat %d count:\r\n",i);
        socp_crit("Int   Start  : 0x%x,  End  : 0x%x ,Slice :0x%x\n",g_stEncDstStat[count].ulIntStartSlice,g_stEncDstStat[count].ulIntEndSlice,g_stEncDstStat[count].ulIntEndSlice-g_stEncDstStat[count].ulIntStartSlice);
        socp_crit("Task  Start  : 0x%x,  End  : 0x%x ,Slice :0x%x\n",g_stEncDstStat[count].ulTaskStartSlice,g_stEncDstStat[count].ulTaskEndSlice,g_stEncDstStat[count].ulTaskEndSlice-g_stEncDstStat[count].ulTaskStartSlice);
        socp_crit("Read  Start  : 0x%x,  End  : 0x%x ,Slice :0x%x\n",g_stEncDstStat[count].ulReadDoneStartSlice,g_stEncDstStat[count].ulReadDoneEndSlice,g_stEncDstStat[count].ulReadDoneEndSlice-g_stEncDstStat[count].ulReadDoneStartSlice);
        socp_crit("Int  ==> Task 0x%x\n",g_stEncDstStat[count].ulTaskStartSlice-g_stEncDstStat[count].ulIntStartSlice);
        socp_crit("Task ==> Read 0x%x\n",g_stEncDstStat[count].ulReadDoneStartSlice-g_stEncDstStat[count].ulTaskEndSlice);
        count = (count+1)%SOCP_MAX_ENC_DST_COUNT;
    }
}

#define MALLOC_MAX_SIZE     0x100000
#define MALLOC_MAX_INDEX    8           /*page_size 为4K*/
#define SOCP_PAGE_SIZE      0x1000

//__inline
s32 socp_get_index(u32 u32Size,u32 *index)
{
    u32 i = 0;
    if(u32Size > MALLOC_MAX_SIZE)
    {
        return BSP_ERROR;
    }
    for(i=0;i<=MALLOC_MAX_INDEX;i++)
    {
        if(u32Size <= (u32)(SOCP_PAGE_SIZE * (1<<i)))
        {
            *index = i;
            break;
        }
    }
    return BSP_OK;
}

void* socp_malloc(u32 u32Size)
{
    u8 *pItem= NULL;
    u32 index = 0;

    if(BSP_OK != socp_get_index(u32Size,&index))
    {
        return BSP_NULL;
    }

    index = 4;
    /* 分配内存 */
    pItem = (u8*)__get_free_pages(GFP_KERNEL,index);
    if(!pItem)
    {
        socp_error("malloc failed\n");
        return BSP_NULL;
    }

    return (void*)pItem;
}

s32 socp_free(void* pMem)
{
    u32 *pItem;

    pItem = pMem;

    free_pages((unsigned long)pItem,4);
    return BSP_OK;
}

/* 低功耗相关 begin */
/* 低功耗部分暂不修改 */
/* cov_verified_start */
void BSP_SOCP_DrxRestoreRegAppOnly(void)
{
    u32 i= 0;

    for(i=0;i<SOCP_MAX_ENCDST_CHN;i++)
    {
        SOCP_REG_READ(SOCP_REG_ENCDEST_BUFRPTR(i),g_strSocpStat.sEncDstChan[i].sEncDstBuf.u32Read);
        SOCP_REG_READ(SOCP_REG_ENCDEST_BUFWPTR(i),g_strSocpStat.sEncDstChan[i].sEncDstBuf.u32Write);
    }

    for(i=0;i<SOCP_MAX_DECDST_CHN;i++)
    {
        SOCP_REG_READ(SOCP_REG_DECDEST_BUFRPTR(i),g_strSocpStat.sDecDstChan[i].sDecDstBuf.u32Read);
        SOCP_REG_READ(SOCP_REG_DECDEST_BUFWPTR(i),g_strSocpStat.sDecDstChan[i].sDecDstBuf.u32Write);
    }

    for(i=0;i<SOCP_MAX_ENCSRC_CHN;i++)
    {
        SOCP_REG_READ(SOCP_REG_ENCSRC_BUFRPTR(i),  g_strSocpStat.sEncSrcChan[i].sEncSrcBuf.u32Read);
        SOCP_REG_READ(SOCP_REG_ENCSRC_BUFWPTR(i),g_strSocpStat.sEncSrcChan[i].sEncSrcBuf.u32Write);

        SOCP_REG_READ(SOCP_REG_ENCSRC_RDQRPTR(i),  g_strSocpStat.sEncSrcChan[i].sRdBuf.u32Read);
        SOCP_REG_READ(SOCP_REG_ENCSRC_RDQWPTR(i),g_strSocpStat.sEncSrcChan[i].sRdBuf.u32Write);
    }

    for(i=0;i<SOCP_MAX_DECSRC_CHN;i++)
    {
        SOCP_REG_READ(SOCP_REG_DECSRC_BUFRPTR(i),  g_strSocpStat.sDecSrcChan[i].sDecSrcBuf.u32Read);
        SOCP_REG_READ(SOCP_REG_DECSRC_BUFWPTR(i),g_strSocpStat.sDecSrcChan[i].sDecSrcBuf.u32Write);
    }
}
/* cov_verified_stop */

/*****************************************************************************
* 函 数 名  : bsp_socp_get_state
*
* 功能描述  : 获取SOCP状态
*
* 返 回 值  : SOCP_IDLE    空闲
*             SOCP_BUSY    忙碌
*****************************************************************************/
SOCP_STATE_ENUM_UINT32 bsp_socp_get_state(void)
{
    u32 u32EncChanState;
    u32 u32DecChanState;

    SOCP_REG_READ(SOCP_REG_ENCSTAT, u32EncChanState);
    SOCP_REG_READ(SOCP_REG_DECSTAT, u32DecChanState);
    if(u32EncChanState != 0 || u32DecChanState != 0)
    {
        return SOCP_BUSY;
    }

    return SOCP_IDLE;
}

/*****************************************************************************
* 函 数 名  : socp_is_encdst_chan_empty
*
* 功能描述  : SOCP编码目的通道是否有数据
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : u32 0:无数据 非0:对应通道置位
*****************************************************************************/
u32 socp_is_encdst_chan_empty(void)
{
    u32 chanSet = 0;
    u32 i;
    u32 u32ReadPtr;
    u32 u32WritePtr;

    /* 判断目的通道读写指针是否相等 */
    for(i = 0; i < SOCP_MAX_ENCDST_CHN; i++)
    {
        SOCP_REG_READ(SOCP_REG_ENCDEST_BUFWPTR(i), u32WritePtr);
        SOCP_REG_READ(SOCP_REG_ENCDEST_BUFRPTR(i), u32ReadPtr);
        if(u32WritePtr != u32ReadPtr)
        {
            chanSet = chanSet | (1 << i);
        }
    }

    return chanSet;
}
module_init(socp_init);





/*****************************************************************************
* 函 数 名  : socp_set_clk_autodiv_enable
* 功能描述  : 调用clk接口clk_disable_unprepare将bypass置0，即开自动降频
* 输入参数  : 无
* 输出参数  : 无
* 返 回 值  : 无
* 注    意  :
              clk_prepare_enable 接口与 clk_disable_unprepare 接口必须成对使用
              clk的自动降频默认处于打开状态，所以
              必须先进行 clk_prepare_enable 才能进行 clk_disable_unprepare 操作
*****************************************************************************/
void bsp_socp_set_clk_autodiv_enable(void)
{
}


/*****************************************************************************
* 函 数 名  : socp_set_clk_autodiv_disable
* 功能描述  : 调用clk接口clk_prepare_enable将bypass置1，即关自动降频
* 输入参数  : 无
* 输出参数  : 无
* 返 回 值  : 无
* 注    意  :
              clk_prepare_enable 接口与 clk_disable_unprepare 接口必须成对使用
              clk的自动降频默认处于打开状态，所以
              必须先进行 clk_prepare_enable 才能进行 clk_disable_unprepare 操作
*****************************************************************************/
void bsp_socp_set_clk_autodiv_disable(void)
{
}

/*****************************************************************************
* 函 数 名  : bsp_socp_set_decode_timeout_register
*
* 功能描述  :编解码中断超时配置寄存器选择。
                            1 - 编码通道采用INT_TIMEOUT(0x024)；解码通道采用DEC_INT_TIMEOUT(0x20);
                            0 - 编解码通道都采用INT_TIMEOUT(0x024)

*
* 返 回 值  :  空
*
*
*****************************************************************************/

u32 bsp_get_socp_ind_dst_int_slice(void)
{
    return g_stEncDstStat[g_ulEncDstStatCount].ulIntStartSlice;
}


s32 bsp_clear_socp_buff(u32 u32SrcChanID)
{
    return BSP_OK;
}

EXPORT_SYMBOL(socp_reset_dec_chan);
EXPORT_SYMBOL(socp_soft_free_encdst_chan);
EXPORT_SYMBOL(socp_soft_free_decsrc_chan);
EXPORT_SYMBOL(bsp_socp_clean_encsrc_chan);
EXPORT_SYMBOL(socp_init);
EXPORT_SYMBOL(bsp_socp_encdst_dsm_init);
EXPORT_SYMBOL(bsp_socp_data_send_manager);
EXPORT_SYMBOL(bsp_socp_coder_set_src_chan);
EXPORT_SYMBOL(bsp_socp_decoder_set_dest_chan);
EXPORT_SYMBOL(bsp_socp_coder_set_dest_chan_attr);
EXPORT_SYMBOL(bsp_socp_decoder_get_err_cnt);
EXPORT_SYMBOL(bsp_socp_decoder_set_src_chan_attr);
EXPORT_SYMBOL(bsp_socp_set_timeout);
EXPORT_SYMBOL(bsp_socp_set_dec_pkt_lgth);
EXPORT_SYMBOL(bsp_socp_set_debug);
EXPORT_SYMBOL(bsp_socp_free_channel);
EXPORT_SYMBOL(bsp_socp_chan_soft_reset);
EXPORT_SYMBOL(bsp_socp_start);
EXPORT_SYMBOL(bsp_socp_stop);
EXPORT_SYMBOL(bsp_socp_register_event_cb);
EXPORT_SYMBOL(bsp_socp_get_write_buff);
EXPORT_SYMBOL(bsp_socp_write_done);
EXPORT_SYMBOL(bsp_socp_register_rd_cb);
EXPORT_SYMBOL(bsp_socp_get_rd_buffer);
EXPORT_SYMBOL(bsp_socp_read_rd_done);
EXPORT_SYMBOL(bsp_socp_register_read_cb);
EXPORT_SYMBOL(bsp_socp_get_read_buff);
EXPORT_SYMBOL(bsp_socp_read_data_done);
EXPORT_SYMBOL(bsp_socp_set_bbp_enable);
EXPORT_SYMBOL(bsp_socp_set_bbp_ds_mode);
EXPORT_SYMBOL(socp_get_index);
EXPORT_SYMBOL(socp_malloc);
EXPORT_SYMBOL(socp_free);
EXPORT_SYMBOL(BSP_SOCP_DrxRestoreRegAppOnly);
EXPORT_SYMBOL(bsp_socp_get_state);
EXPORT_SYMBOL(socp_is_encdst_chan_empty);



