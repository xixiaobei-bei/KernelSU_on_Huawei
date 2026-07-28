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

#include <linux/clk.h>
#include <of.h>
#include <osl_thread.h>
#include "deflate.h"
#include "socp_ind_delay.h"

struct deflate_ctrl_info g_strDeflateCtrl;
struct deflate_debug_info g_strDeflateDebug;
struct deflate_abort_info g_strDeflateAbort;
u32 g_strDeflateEnableState=0;

extern DRV_DEFLATE_CFG_STRU  g_deflate_nv_ctrl;
#define DEFDST_TASK_PRO  81

/*****************************************************************************
* 函 数 名  : deflate_get_data_buffer
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
void deflate_get_data_buffer(DEFLATE_RING_BUF_STRU *pRingBuffer, DEFLATE_BUFFER_RW_STRU *pRWBuffer)
{
    if(pRingBuffer->u32Read <= pRingBuffer->u32Write)
    {
        /* 写指针大于读指针，直接计算 */
        pRWBuffer->pBuffer = (char *)(pRingBuffer->Start + (u32)pRingBuffer->u32Read);
        pRWBuffer->u32Size = (u32)(pRingBuffer->u32Write - pRingBuffer->u32Read);
        pRWBuffer->pRbBuffer = (char *)BSP_NULL;
        pRWBuffer->u32RbSize = 0;
    }
    else
    {
        /* 读指针大于写指针，需要考虑回卷 */
        pRWBuffer->pBuffer = (char *)(pRingBuffer->Start + (u32)pRingBuffer->u32Read);
        pRWBuffer->u32Size = ((u32)((u64)pRingBuffer->End - ((u64)pRingBuffer->Start + pRingBuffer->u32Read)) + 1);
        pRWBuffer->pRbBuffer = (char *)pRingBuffer->Start;
        pRWBuffer->u32RbSize = pRingBuffer->u32Write;
    }
}
/*****************************************************************************
* 函 数 名  : deflate_read_done
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
void deflate_read_done(DEFLATE_RING_BUF_STRU *pRingBuffer, u32 u32Size)
{
	pRingBuffer->u32Read += u32Size;
	if(pRingBuffer->u32Read > (u32)(pRingBuffer->End - pRingBuffer->Start))
	{
    	pRingBuffer->u32Read -= pRingBuffer->u32Length;
	}
}
/*****************************************************************************
* 函 数 名  : deflate_debug
*
* 功能描述  :读取寄存器
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
void deflate_debug(void)
{
    /*lint -save -e732*/
    DEFLATE_REG_READ(SOCP_REG_DEFLATE_GLOBALCTRL,g_strDeflateAbort.u32DeflateGlobal);
    DEFLATE_REG_READ(SOCP_REG_DEFLATE_IBUFTIMEOUTCFG,g_strDeflateAbort.u32IbufTimeoutConfig);
    DEFLATE_REG_READ(SOCP_REG_DEFLATE_RAWINT,g_strDeflateAbort.u32RawInt);
    DEFLATE_REG_READ(SOCP_REG_DEFLATE_INT,g_strDeflateAbort.u32IntState);
    DEFLATE_REG_READ(SOCP_REG_DEFLATE_INTMASK,g_strDeflateAbort.u32IntMask);
    DEFLATE_REG_READ(SOCP_REG_DEFLATE_TFRTIMEOUTCFG,g_strDeflateAbort.u32ThfTimeoutConfig);
    DEFLATE_REG_READ(SOCP_REG_DEFLATE_STATE,g_strDeflateAbort.u32DeflateState);
    DEFLATE_REG_READ(SOCP_REG_DEFLATE_ABORTSTATERECORD,g_strDeflateAbort.u32AbortStateRecord);
    DEFLATE_REG_READ(SOCP_REG_DEFLATEDEBUG_CH,g_strDeflateAbort.u32DebugChan);
    DEFLATE_REG_READ(SOCP_REG_DEFLATEDEST_BUFREMAINTHCFG,g_strDeflateAbort.u32ObufThrh);
    DEFLATE_REG_READ(SOCP_REG_DEFLATEDEST_BUFRPTR,g_strDeflateAbort.u32ReadAddr);
    DEFLATE_REG_READ(SOCP_REG_DEFLATEDEST_BUFWPTR,g_strDeflateAbort.u32WriteAddr);
    DEFLATE_REG_READ(SOCP_REG_DEFLATEDEST_BUFADDR,g_strDeflateAbort.u32StartAddr);
    DEFLATE_REG_READ(SOCP_REG_DEFLATEDEST_BUFDEPTH,g_strDeflateAbort.u32BufSize);
    DEFLATE_REG_READ(SOCP_REG_DEFLATEDEST_BUFTHRH,g_strDeflateAbort.u32IntThrh);
    DEFLATE_REG_READ(SOCP_REG_DEFLATEDEST_BUFOVFTIMEOUT,g_strDeflateAbort.u32OvfTimeoutEn);
    DEFLATE_REG_READ(SOCP_REG_SOCP_MAX_PKG_BYTE_CFG,g_strDeflateAbort.u32PkgConfig);
    DEFLATE_REG_READ(SOCP_REG_DEFLATE_OBUF_DEBUG,g_strDeflateAbort.u32ObufDebug);
    /*lint -restore +e732*/
}
/*****************************************************************************
* 函 数 名   : deflate_set
*
* 功能描述  :deflate配置接口，当配置socp目的端时候调用
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 配置成功与否的标识码
*****************************************************************************/
u32 deflate_set(u32 u32DestChanID, DEFLATE_CHAN_CONFIG_S *pDeflateAttr)
{
    unsigned long start;
    unsigned long end;
    u32 bufThreshold;
    u32 buflength;
    u32 u32ChanID;
    u32 u32Thrh;

    struct deflate_ctrl_info * pChan;
    /* 判断是否已经初始化 */
    if (!g_strDeflateCtrl.initFlag)
    {
        socp_error("the module has not been initialized!\n");
        return DEFLATE_ERR_NOT_INIT;
    }
    g_strDeflateDebug.u32DeflateDstSetCnt++;
    if (NULL == pDeflateAttr)
    {
        socp_error("the parameter is NULL!\n");
        return DEFLATE_ERR_NULL;
    }

    /* 判断参数有效性 */
    u32ChanID  = DEFLATE_REAL_CHAN_ID(u32DestChanID);
    if(u32ChanID > DEFLATE_MAX_ENCDST_CHN)
    {
        socp_error("the chan id(0x%x) is invalid!\n", u32ChanID);
        return DEFLATE_ERR_INVALID_PARA;
    }
    start = (uintptr_t)pDeflateAttr->sCoderSetDstBuf.pucOutputStart;
    end   = (uintptr_t)pDeflateAttr->sCoderSetDstBuf.pucOutputEnd;
    bufThreshold = pDeflateAttr->sCoderSetDstBuf.u32Threshold*1024;//socp寄存器是kbyte为单位,deflate是byte为单位
    u32Thrh = 16*1024;//阈值溢出

    if ((0 == start)||(0 == end) || (0 == bufThreshold)||(0 == u32Thrh))
    {
        socp_error("the parameter is NULL!\n");
        return DEFLATE_ERR_NULL;
    }

    if (0 != (start%8))
    {
        socp_error("the parameter is not 8 bytes aligned!\n");
        return DEFLATE_ERR_NOT_8BYTESALIGN;
    }
    if (start >= end)
    {
        socp_error("the buff is invalid!\n");
        return DEFLATE_ERR_INVALID_PARA;
    }

    buflength =(u32) ((end - start) + 1);

    if (0 != (buflength%8))
    {
        socp_error("the parameter is not 8 bytes aligned!\n");
        return DEFLATE_ERR_NOT_8BYTESALIGN;
    }

    /* 如果经过配置则不能再次配置 */
    pChan = &g_strDeflateCtrl;

    if (!pChan->u32SetStat)
    {
        /* 写入起始地址到目的buffer起始地址寄存器*/
        DEFLATE_REG_WRITE(SOCP_REG_DEFLATE_DST_BUFADDR_L, (u32)start);
    	DEFLATE_REG_WRITE(SOCP_REG_DEFLATE_DST_BUFADDR_H, (u32)((u64)start>>32));
    	DEFLATE_REG_WRITE(SOCP_REG_DEFLATEDEST_BUFRPTR,0);
    	DEFLATE_REG_WRITE(SOCP_REG_DEFLATEDEST_BUFRPTR,0);

        DEFLATE_REG_WRITE(SOCP_REG_DEFLATEDEST_BUFDEPTH,buflength);
        DEFLATE_REG_WRITE(SOCP_REG_DEFLATEDEST_BUFREMAINTHCFG,u32Thrh);//阈值溢出
        DEFLATE_REG_WRITE(SOCP_REG_DEFLATEDEST_BUFTHRH,bufThreshold);
        DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_GLOBALCTRL, 22, 4,u32ChanID);

        /* 在g_strDeflateStat中保存参数*/
        pChan->u32ChanID                   = u32DestChanID;
        pChan->u32Thrh                     = u32Thrh;
        pChan->sDeflateDstChan.Start       = start;
        pChan->sDeflateDstChan.End         = end;
        pChan->sDeflateDstChan.u32Write    = (u32)start;
        pChan->sDeflateDstChan.u32Read     = (u32)start;
        pChan->sDeflateDstChan.u32Length   = buflength;

        /* 表明该通道已经配置 */
        pChan->u32SetStat = DEFLATE_CHN_SET;
   }
   else
   {
        socp_crit("Deflate set finished!\n");
   }
    return DEFLATE_OK;
}
/*****************************************************************************
* 函 数 名   : deflate_clear
*
* 功能描述  :deflate清除配置
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 成功标识符
*****************************************************************************/
u32 deflate_clear(u32 u32DestChanID)
{
    u32  u32ChanID;
    u32ChanID  = DEFLATE_REAL_CHAN_ID(u32DestChanID);
    if(u32ChanID > DEFLATE_MAX_ENCDST_CHN)
    {
        socp_error("the chan id is invalid!\n");
        return DEFLATE_ERR_INVALID_PARA;
    }
    /*deflate中断标志初始化*/
    g_strDeflateCtrl.u32IntDeflateDstOvf           =0;
    g_strDeflateCtrl.u32IntDeflateDstTfr           =0;
    g_strDeflateCtrl.u32IntDeflateDstThresholdOvf  =0;
    g_strDeflateCtrl.u32IntDeflateWorkAbort        =0;

    /*deflate目的通道属性初始化*/
    g_strDeflateCtrl.u32SetStat   =0;
    g_strDeflateCtrl.u32Thrh      =0;

    return DEFLATE_OK;
}
/*****************************************************************************
* 函 数 名   : deflate_enable
*
* 功能描述  :deflate使能接口,供SOCP非压缩转压缩调用
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 成功标识符
*****************************************************************************/
u32 deflate_enable(u32 u32DestChanID)
{
    u32  u32ChanID;

    u32ChanID  = DEFLATE_REAL_CHAN_ID(u32DestChanID);
    if(u32ChanID > DEFLATE_MAX_ENCDST_CHN)
    {
        socp_error("the chan id is invalid!\n");
        return DEFLATE_ERR_INVALID_PARA;
    }
    /* 判断是否已经初始化 */
    if (!g_strDeflateCtrl.initFlag)
    {
        socp_error("the module has not been initialized!\n");
        return DEFLATE_ERR_NOT_INIT;
    }
    if(DEFLATE_CHN_SET == g_strDeflateCtrl.u32SetStat)
    {

        /* 清中断，开中断 */
        bsp_deflate_data_send_manager(COMPRESS_ENABLE_STATE);
        /*测试清中断，屏蔽阈值中断和上溢中断*/
        //DEFLATE_REG_WRITE(SOCP_REG_DEFLATE_INTMASK,0x192);
        /*测试清中断，屏蔽阈值中断和阈值溢出中断*/
        //DEFLATE_REG_WRITE(SOCP_REG_DEFLATE_INTMASK,0x152);

        /*使能deflate*/
        DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_GLOBALCTRL, 21, 1, 1);
    }
    else
    {
        socp_error("Deflate Enable failed!\n");
        return DEFLATE_ERR_SET_INVALID;
    }
    return DEFLATE_OK;
}
/*****************************************************************************
* 函 数 名   : deflate_unable
*
* 功能描述  :deflate不使能接口,供SOCP压缩转非压缩场景调用
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 成功与否标识码
*****************************************************************************/
u32 deflate_disable(u32 u32DestChanID)
{
    u32 DeflateIdleState;
    u32  u32ChanID;

    u32ChanID  = DEFLATE_REAL_CHAN_ID(u32DestChanID);
    if(u32ChanID > DEFLATE_MAX_ENCDST_CHN)
    {
        socp_error("the chan id is invalid!\n");
        return DEFLATE_ERR_INVALID_PARA;
    }
    /* 判断是否已经初始化 */
    if (!g_strDeflateCtrl.initFlag)
    {
        socp_error("the module has not been initialized!\n");
        return DEFLATE_ERR_NOT_INIT;
    }

    if(DEFLATE_CHN_SET == g_strDeflateCtrl.u32SetStat)
    {
        /*立即压缩*/
        DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_GLOBALCTRL, 20, 1, 1);


         /*lint -save -e732*/
          /*读deflate全局状态寄存器，获取deflate工作状态*/
        DEFLATE_REG_READ(SOCP_REG_DEFLATE_GLOBALCTRL,DeflateIdleState);
        while(!(DeflateIdleState & DEFLATE_WORK_STATE))
        {
           DEFLATE_REG_READ(SOCP_REG_DEFLATE_GLOBALCTRL,DeflateIdleState);
           msleep(1);
        }
        /*lint -save -e845*/
        DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_GLOBALCTRL, 21, 1, 0);
         /*lint -restore +e845*/

    	DEFLATE_REG_WRITE(SOCP_REG_DEFLATEDEST_BUFRPTR ,0);
    	DEFLATE_REG_WRITE(SOCP_REG_DEFLATEDEST_BUFWPTR, 0);

        /*清压缩中断状态，屏蔽中断状态*/
        bsp_deflate_data_send_manager(COMPRESS_DISABLE_STATE);
    }
    else
    {
        socp_error("Deflate Unable failed!\n") ;
        return DEFLATE_ERR_SET_INVALID;
    }
    return DEFLATE_OK;
 }
/*****************************************************************************
* 函 数 名   : deflate_register_read_cb
*
* 功能描述  : deflate 注册读回调接口
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   :成功与否标识码
*****************************************************************************/
 u32 deflate_register_read_cb(deflate_read_cb ReadCB)
 {
    /* 判断是否已经初始化 */
    if (!g_strDeflateCtrl.initFlag)
    {
        socp_error("the module has not been initialized!\n");
        return DEFLATE_ERR_NOT_INIT;
    }

    /*deflate是否已经配置*/
    if(DEFLATE_CHN_SET == g_strDeflateCtrl.u32SetStat)
    {
        /* 设置对应通道的回调函数*/
        g_strDeflateCtrl.read_cb = ReadCB;
        g_strDeflateDebug.u32DeflateRegReadCBCnt++;
    }
    else
    {
         socp_error("Deflate readCB  failed!\n");
         return DEFLATE_ERR_SET_INVALID;
    }
    return DEFLATE_OK;

}
/*****************************************************************************
* 函 数 名   : deflate_register_event_cb
*
* 功能描述  :deflate 注册event回调接口
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 成功与否标识码
*****************************************************************************/
 u32 deflate_register_event_cb(deflate_event_cb EventCB)
 {

    /* 判断是否已经初始化 */
    if (!g_strDeflateCtrl.initFlag)
    {
        socp_error("the module has not been initialized!\n");
        return DEFLATE_ERR_NOT_INIT;
    }

   /*deflate是否已经配置*/
    if(DEFLATE_CHN_SET == g_strDeflateCtrl.u32SetStat)
    {
        /* 设置对应通道的回调函数*/
        g_strDeflateCtrl.event_cb = EventCB;
        g_strDeflateDebug.u32DeflateRegEventCBCnt++;
    }
    else
    {
         socp_error("Deflate RegiseEventCB  failed!\n");
         return DEFLATE_ERR_SET_INVALID;
    }
    return DEFLATE_OK;

 }
/*****************************************************************************
* 函 数 名   : deflate_read_data_done
*
* 功能描述  : deflate read done接口
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 成功与否标识码
*****************************************************************************/
u32 deflate_read_data_done(u32 ulDatalen)
{
 /*lint -save -e438*/
    DEFLATE_BUFFER_RW_STRU RwBuff;
    u32  uPAddr;
    u32  curmodestate;
    unsigned long lock_flag;

    /* 判断是否已经初始化 */
    if (!g_strDeflateCtrl.initFlag)
    {
        socp_error("the module has not been initialized!\n");
        return DEFLATE_ERR_NOT_INIT;
    }
    curmodestate = DEFLATE_REG_GETBITS(SOCP_REG_DEFLATE_GLOBALCTRL,27,1);
    if(1==curmodestate)
    {
        socp_crit("deflate ind delay mode is cycle 0x%x!\n",curmodestate);
        return BSP_OK;
    }

    if(DEFLATE_CHN_SET == g_strDeflateCtrl.u32SetStat)
    {
       g_strDeflateDebug.u32DeflateReaddoneEtrCnt++;
       if (0 == ulDatalen)
       {
           g_strDeflateDebug.u32DeflateReaddoneZeroCnt++;
       }
       else
       {
            g_strDeflateDebug.u32DeflateReaddoneValidCnt++;
       }
       DEFLATE_REG_READ(SOCP_REG_DEFLATEDEST_BUFRPTR, uPAddr);
       g_strDeflateCtrl.sDeflateDstChan.u32Read = uPAddr;
       DEFLATE_REG_READ(SOCP_REG_DEFLATEDEST_BUFWPTR, uPAddr);
       /*lint -restore +e732*/
       g_strDeflateCtrl.sDeflateDstChan.u32Write= uPAddr;
       deflate_get_data_buffer(&g_strDeflateCtrl.sDeflateDstChan, &RwBuff);
       if(RwBuff.u32Size + RwBuff.u32RbSize < ulDatalen)
       {
            socp_error("RwBuff is not enough, ulDatalen=0x%x!\n", ulDatalen);

             /*lint -save -e550 -e845*/
            spin_lock_irqsave(&g_strDeflateCtrl.intSpinLock, lock_flag);
            DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_RAWINT,  2, 1,1);
            DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_INTMASK ,  2, 1,0);

            DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_RAWINT,  6, 3,0x07);
            DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_INTMASK ,  6, 1,0);
            spin_unlock_irqrestore(&g_strDeflateCtrl.intSpinLock, lock_flag);

            g_strDeflateDebug.u32DeflateReaddoneFailCnt++;
            return DEFLATE_ERR_INVALID_PARA;
       }
        /*更新读指针*/
        deflate_read_done(&g_strDeflateCtrl.sDeflateDstChan,ulDatalen);
        /*写入读指针到读指针寄存器*/
        uPAddr = g_strDeflateCtrl.sDeflateDstChan.u32Read;
        DEFLATE_REG_WRITE(SOCP_REG_DEFLATEDEST_BUFRPTR, uPAddr);

        spin_lock_irqsave(&g_strDeflateCtrl.intSpinLock, lock_flag);
        bsp_deflate_data_send_continue();
        spin_unlock_irqrestore(&g_strDeflateCtrl.intSpinLock, lock_flag);
         /*lint -restore +e550 +e845*/
        g_strDeflateDebug.u32DeflateReaddoneSucCnt++;
    }
    else
    {
        socp_error("Readdatadone failed!\n");
        return DEFLATE_ERR_SET_INVALID;
    }

     return DEFLATE_OK;
     /*lint -restore +e438*/
}
/*****************************************************************************
* 函 数 名   : deflate_get_read_buffer
*
* 功能描述  : 获取deflate数据空间
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  :成功与否标识码
*****************************************************************************/
 u32 deflate_get_read_buffer(DEFLATE_BUFFER_RW_STRU *pBuff)
 {

    u32  uPAddr;
     /* 判断参数有效性 */
    if (NULL == pBuff)
    {
        socp_error("the parameter is NULL!\n");
        return DEFLATE_ERR_NULL;
    }
    /* 判断是否已经初始化 */
    if (!g_strDeflateCtrl.initFlag)
    {
        socp_error("the module has not been initialized!\n");
        return DEFLATE_ERR_NOT_INIT;
    }


    if(DEFLATE_CHN_SET == g_strDeflateCtrl.u32SetStat)
    {
        g_strDeflateDebug.u32DeflateGetReadBufEtrCnt++;
        /* 根据读写指针获取buffer */
        DEFLATE_REG_READ(SOCP_REG_DEFLATEDEST_BUFRPTR, uPAddr);
        g_strDeflateCtrl.sDeflateDstChan.u32Read = uPAddr;

        DEFLATE_REG_READ(SOCP_REG_DEFLATEDEST_BUFWPTR, uPAddr);
        g_strDeflateCtrl.sDeflateDstChan.u32Write= uPAddr;

        deflate_get_data_buffer(&g_strDeflateCtrl.sDeflateDstChan, pBuff);

        g_strDeflateDebug.u32DeflateGetReadBufSucCnt++;
    }
    else
    {
        socp_error("deflate set failed!\n");
        return DEFLATE_ERR_SET_INVALID;
    }

    return DEFLATE_OK;
 }


/*****************************************************************************
* 函 数 名   : deflate_int_handler
*
* 功能描述  :deflate中断处理接口
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 无
*****************************************************************************/
u32 deflate_int_handler(void)
 {
    /*lint -save -e438*/
    u32 bHandle = false;
    u32  intState = 0;
    unsigned long lock_flag;
    u32 mask;
    if(DEFLATE_CHN_SET == g_strDeflateCtrl.u32SetStat)
    {
        DEFLATE_REG_READ(SOCP_REG_DEFLATE_INT, intState);
        /*阈值传输中断*/
        if(intState&DEFLATE_TFR_MASK)
        {
             spin_lock_irqsave(&g_strDeflateCtrl.intSpinLock, lock_flag);
             DEFLATE_REG_READ(SOCP_REG_DEFLATE_INTMASK,mask);
             DEFLATE_REG_WRITE(SOCP_REG_DEFLATE_INTMASK,intState|mask);
             DEFLATE_REG_WRITE(SOCP_REG_DEFLATE_RAWINT,intState);
             g_strDeflateCtrl.u32IntDeflateDstTfr |=  intState;
             spin_unlock_irqrestore(&g_strDeflateCtrl.intSpinLock, lock_flag);
             bHandle = TRUE;
        }
        /*buffer阈值溢出*/
        else if(intState&DEFLATE_THROVF_MASK)
        {
             spin_lock_irqsave(&g_strDeflateCtrl.intSpinLock, lock_flag);
             DEFLATE_REG_READ(SOCP_REG_DEFLATE_INTMASK,mask);
             DEFLATE_REG_WRITE(SOCP_REG_DEFLATE_INTMASK,intState|mask);
             DEFLATE_REG_WRITE(SOCP_REG_DEFLATE_RAWINT,intState);
             g_strDeflateCtrl.u32IntDeflateDstThresholdOvf |=  intState;
             spin_unlock_irqrestore(&g_strDeflateCtrl.intSpinLock, lock_flag);
             bHandle = TRUE;
        }
        /*buffer上溢中断*/
        else if(intState&DEFLATE_OVF_MASK)
        {
             spin_lock_irqsave(&g_strDeflateCtrl.intSpinLock, lock_flag);
             DEFLATE_REG_READ(SOCP_REG_DEFLATE_INTMASK,mask);
             DEFLATE_REG_WRITE(SOCP_REG_DEFLATE_INTMASK,intState|mask);
             DEFLATE_REG_WRITE(SOCP_REG_DEFLATE_RAWINT,intState);
             g_strDeflateCtrl.u32IntDeflateDstOvf |=  intState;
             spin_unlock_irqrestore(&g_strDeflateCtrl.intSpinLock, lock_flag);
             bHandle = TRUE;
        }
        /*异常中断*/
        else if(intState&DEFLATE_WORK_ABORT_MASK)
        {    spin_lock_irqsave(&g_strDeflateCtrl.intSpinLock, lock_flag);
             DEFLATE_REG_READ(SOCP_REG_DEFLATE_INTMASK,mask);
             DEFLATE_REG_WRITE(SOCP_REG_DEFLATE_INTMASK,intState|mask);
             DEFLATE_REG_WRITE(SOCP_REG_DEFLATE_RAWINT,intState);
             g_strDeflateCtrl.u32IntDeflateWorkAbort |=  intState;
             spin_unlock_irqrestore(&g_strDeflateCtrl.intSpinLock, lock_flag);
             bHandle = TRUE;
        }
       /*循环模式中断*/
        else if(intState&DEFLATE_CYCLE_MASK)
        {
             spin_lock_irqsave(&g_strDeflateCtrl.intSpinLock, lock_flag);
             DEFLATE_REG_READ(SOCP_REG_DEFLATE_INTMASK,mask);
             DEFLATE_REG_WRITE(SOCP_REG_DEFLATE_INTMASK,intState|mask);
             DEFLATE_REG_WRITE(SOCP_REG_DEFLATE_RAWINT,intState);
             g_strDeflateCtrl.u32IntDeflateCycle |=  intState;
             spin_unlock_irqrestore(&g_strDeflateCtrl.intSpinLock, lock_flag);
             bHandle = TRUE;
        }
        /*阻塞模式中断*/
        else if(intState&DEFLATE_NOCYCLE_MASK)
        {
             spin_lock_irqsave(&g_strDeflateCtrl.intSpinLock, lock_flag);
             DEFLATE_REG_READ(SOCP_REG_DEFLATE_INTMASK,mask);
             DEFLATE_REG_WRITE(SOCP_REG_DEFLATE_INTMASK,intState|mask);
             DEFLATE_REG_WRITE(SOCP_REG_DEFLATE_RAWINT,intState);
             g_strDeflateCtrl.u32IntDeflateNoCycle |=  intState;
             spin_unlock_irqrestore(&g_strDeflateCtrl.intSpinLock, lock_flag);
             bHandle = TRUE;
        }
        else
        {
             bHandle = FALSE;
        }

   }
   if(bHandle)
   {
        osl_sem_up(&g_strDeflateCtrl.task_sem);
   }
   /*lint -restore +e438*/
   return DEFLATE_OK;
 }
/*****************************************************************************
* 函 数 名   : deflate_cycle
*
* 功能描述  : deflate 循环模式处理
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 无
*****************************************************************************/
void deflate_cycle(void)
{
    unsigned long lock_flag;
    /* 检测通道是否配置*/
    if (DEFLATE_CHN_SET == g_strDeflateCtrl.u32SetStat)
    {


         /*屏蔽别的中断*/
         spin_lock_irqsave(&g_strDeflateCtrl.intSpinLock, lock_flag);
         DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_RAWINT,  2, 1,1);
         DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_INTMASK ,  2, 1,1);
         DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_RAWINT,  6, 3,0x07);
         DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_INTMASK ,  6, 3,0x07);
          /*打开阻塞中断*/
         DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_RAWINT,   10, 1,1);
         DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_INTMASK,  10, 1,0);
         spin_unlock_irqrestore(&g_strDeflateCtrl.intSpinLock, lock_flag);
         if (g_strDeflateCtrl.event_cb)
        {

            (void)g_strDeflateCtrl.event_cb(g_strDeflateCtrl.u32ChanID,DEFLATE_EVENT_CYCLE,0);

        }

    }
    return ;

}
void deflate_nocycle(void)
{
    unsigned long lock_flag;

    /* 检测通道是否配置*/
    if (DEFLATE_CHN_SET == g_strDeflateCtrl.u32SetStat)
    {

         /*打开别的中断*/
         spin_lock_irqsave(&g_strDeflateCtrl.intSpinLock, lock_flag);
         DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_RAWINT,  2, 1,1);
         DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_INTMASK ,  2, 1,0);
         DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_RAWINT,  6, 3,0x07);
         DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_INTMASK ,  6, 3,0);
          /*打开循环模式中断*/
         DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_RAWINT,  9, 1,1);
         DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_INTMASK,  9, 1,0);
         spin_lock_irqsave(&g_strDeflateCtrl.intSpinLock, lock_flag);
         if (g_strDeflateCtrl.event_cb)
         {

            (void)g_strDeflateCtrl.event_cb(g_strDeflateCtrl.u32ChanID,DEFLATE_EVENT_NOCYCLE,0);

         }

    }
    return ;

}
void deflate_thresholdovf(void)
{
    /* 检测通道是否配置*/
    if (DEFLATE_CHN_SET == g_strDeflateCtrl.u32SetStat)
    {
        if (g_strDeflateCtrl.event_cb)
        {
            g_strDeflateDebug.u32DeflateTskThresholdOvfCbOriCnt++;
            (void)g_strDeflateCtrl.event_cb(g_strDeflateCtrl.u32ChanID,DEFLATE_EVENT_THRESHOLD_OVERFLOW,0);
            g_strDeflateDebug.u32DeflateTskThresholdOvfCbCnt++;
         }
        if (g_strDeflateCtrl.read_cb)

        {
           g_strDeflateDebug.u32DeflateTskThresholdOvfCbOriCnt++;
           (void)g_strDeflateCtrl.read_cb(g_strDeflateCtrl.u32ChanID);
           g_strDeflateDebug.u32DeflateTskThresholdOvfCbCnt++;
         }
    }

    return ;
}
void deflate_ovf(void)
{
    /* 检测通道是否配置*/
    if (DEFLATE_CHN_SET == g_strDeflateCtrl.u32SetStat)
    {
        if (g_strDeflateCtrl.event_cb)
        {
            g_strDeflateDebug.u32DeflateTskOvfCbOriCnt++;
            (void)g_strDeflateCtrl.event_cb(g_strDeflateCtrl.u32ChanID,DEFLATE_EVENT_OVERFLOW,0);
            g_strDeflateDebug.u32DeflateTskOvfCbCnt++;
        }
        if (g_strDeflateCtrl.read_cb)
        {

            g_strDeflateDebug.u32DeflateTskOvfCbOriCnt++;
            (void)g_strDeflateCtrl.read_cb(g_strDeflateCtrl.u32ChanID);
            g_strDeflateDebug.u32DeflateTskTrfCbCnt++;
        }
    }
    return ;
}
/*****************************************************************************
* 函 数 名   : deflate_task
*
* 功能描述  : deflate 任务处理接口
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 无
*****************************************************************************/
void deflate_task(void)
 {

    u32 IntTfrState = 0;
    u32 IntOvfState = 0;
    u32 IntThresholdOvfState = 0;
    u32 IntCycleState =0;
    u32 IntNoCycleState =0;
    u32 IntWorkAortState =0;
    unsigned long lock_flag;

    while(1)
    {
        osl_sem_down(&g_strDeflateCtrl.task_sem);
         /*lint -save -e550*/
        spin_lock_irqsave(&g_strDeflateCtrl.intSpinLock, lock_flag);

        IntTfrState = g_strDeflateCtrl.u32IntDeflateDstTfr;
        g_strDeflateCtrl.u32IntDeflateDstTfr = 0;

        IntThresholdOvfState = g_strDeflateCtrl.u32IntDeflateDstThresholdOvf;
        g_strDeflateCtrl.u32IntDeflateDstThresholdOvf = 0;

        IntOvfState = g_strDeflateCtrl.u32IntDeflateDstOvf;
        g_strDeflateCtrl.u32IntDeflateDstOvf = 0;


        IntCycleState=g_strDeflateCtrl.u32IntDeflateCycle;
        g_strDeflateCtrl.u32IntDeflateCycle =0;

        IntNoCycleState=g_strDeflateCtrl.u32IntDeflateNoCycle;
        g_strDeflateCtrl.u32IntDeflateNoCycle =0;

        IntWorkAortState = g_strDeflateCtrl.u32IntDeflateWorkAbort;
        g_strDeflateCtrl.u32IntDeflateWorkAbort = 0;

        spin_unlock_irqrestore(&g_strDeflateCtrl.intSpinLock, lock_flag);
       /*lint -restore +e550*/
        /* 处理传输中断*/
        if (IntTfrState)
        {

             /* 检测通道是否配置*/
            if (DEFLATE_CHN_SET == g_strDeflateCtrl.u32SetStat)
            {
                if (g_strDeflateCtrl.read_cb)
                {
                    g_strDeflateDebug.u32DeflateTskTrfCbOriCnt++;
                    (void)g_strDeflateCtrl.read_cb(g_strDeflateCtrl.u32ChanID);
                    g_strDeflateDebug.u32DeflateTskTrfCbCnt++;
                }
            }
        }
        /* 处理目的 buffer 阈值溢出中断*/
        if (IntThresholdOvfState)
        {
            deflate_thresholdovf();

        }
        /*处理buffer上溢中断*/
        if (IntOvfState)
        {
            deflate_ovf();

        }
       if(IntCycleState)
       {
            deflate_cycle();

       }
       if(IntNoCycleState)
       {
            deflate_nocycle();
       }
       /*处理异常中断*/
       if (IntWorkAortState)
       {
           /* 检测通道是否配置*/
           if (DEFLATE_CHN_SET == g_strDeflateCtrl.u32SetStat)
           {
                if (g_strDeflateCtrl.event_cb)
                {
                     g_strDeflateDebug.u32DeflateTskIntWorkAortCbOriCnt++;
                     (void)g_strDeflateCtrl.event_cb(g_strDeflateCtrl.u32ChanID, DEFLATE_EVENT_WORK_ABORT,0);
                     deflate_debug();
                     g_strDeflateDebug.u32DeflateTskIntWorkAortCbCnt++;
                 }

          }
      }
  }
    //  return DEFLATE_OK;
}


void deflate_set_dst_threshold(bool mode)
{
    u32 bufLength;
    u32 threshold;

    DEFLATE_REG_READ(SOCP_REG_DEFLATEDEST_BUFDEPTH,bufLength);
    if(mode == true)/*true为需要打开延时上报的场景*/
    {
        threshold = (bufLength >> 2)*3;
    }
    else
    {
        threshold = 0x1000;
    }

    DEFLATE_REG_WRITE(SOCP_REG_DEFLATEDEST_BUFTHRH,threshold);

    socp_crit(" set encdst threshold success! 0x%x\n", threshold);

    return;
}


/*****************************************************************************
* 函 数 名  : deflate_set_cycle_mode
*
* 功能描述  : SOCP循环模式设置
*
* 输入参数  : 通道号、模式
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/

s32 deflate_set_cycle_mode(u32 cycle)
{
    u32 u32modestate;
    u32 waittime = 10000;
    unsigned long lock_flag;

    u32modestate =  DEFLATE_REG_GETBITS(SOCP_REG_DEFLATE_GLOBALCTRL,27,1);
    if ((0==cycle||1==cycle) && u32modestate)
    {
        u32 i;
        spin_lock_irqsave(&g_strDeflateCtrl.intSpinLock, lock_flag);
        DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_GLOBALCTRL,26,1,0);
        spin_unlock_irqrestore(&g_strDeflateCtrl.intSpinLock, lock_flag);
        for(i=0; i<waittime; i++)
        {
            u32modestate =  DEFLATE_REG_GETBITS(SOCP_REG_DEFLATE_GLOBALCTRL,27,1);
            if(0 == u32modestate)
            {
                break;
            }
        }

        if(waittime == i)
        {
            socp_error("set encdst cycle off timeout!\n");
            return DEFLATE_ERR_GET_CYCLE;
        }
        bsp_deflate_data_send_manager(COMPRESS_ENABLE_STATE);
        return DEFLATE_OK;
    }
    else if((2==cycle) && (!u32modestate))
    {
        u32 i;

        bsp_deflate_data_send_manager(COMPRESS_DISABLE_STATE);
        spin_lock_irqsave(&g_strDeflateCtrl.intSpinLock, lock_flag);
        DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_GLOBALCTRL,26,1,1);
        spin_unlock_irqrestore(&g_strDeflateCtrl.intSpinLock, lock_flag);
        for(i=0; i<waittime; i++)
        {
            u32modestate =  DEFLATE_REG_GETBITS(SOCP_REG_DEFLATE_GLOBALCTRL,27,1);
            if(1 == u32modestate)
            {
                break;
            }
        }

        if(waittime == i)
        {
            socp_error("set encdst cycle on timeout!\n");
            return DEFLATE_ERR_GET_CYCLE;
        }
    }

    return BSP_OK;


}
/*****************************************************************************
* 函 数 名  : deflate_set_time
*
* 功能描述  : DEFLATE时间设置
*
* 输入参数  : 通道号、模式
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/

s32 deflate_set_time(u32 mode)
{

    if((SOCP_IND_MODE_DIRECT != mode)&&(SOCP_IND_MODE_DELAY != mode)
            &&(SOCP_IND_MODE_CYCLE != mode))
    {

        socp_error("mode error !\n");
        return DEFLATE_ERR_IND_MODE;

    }
    if(SOCP_IND_MODE_DIRECT == mode)
    {
        DEFLATE_REG_WRITE(SOCP_REG_DEFLATE_TFRTIMEOUTCFG, DEFLATE_TIMEOUT_INDIRECT);
        return DEFLATE_OK;

    }
    else if(SOCP_IND_MODE_DELAY == mode)
    {
        DEFLATE_REG_WRITE(SOCP_REG_DEFLATE_TFRTIMEOUTCFG, DEFLATE_TIMEOUT_DEFLATY);
        return DEFLATE_OK;

    }
    else
    {
        socp_crit("cycle mode need not set time !\n");
        return DEFLATE_OK;
    }


}
/*****************************************************************************
* 函 数 名  : bsp_deflate_data_send_manager
*
* 功能描述  : deflate编码目的端上报数据
* 输入参数  : EncDestChanID: 编码目的端通道号
*             bEnable: 中断使能
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
void bsp_deflate_data_send_manager(u32 bEnable)
{
    unsigned long lock_flag;


    if((COMPRESS_DISABLE_STATE == bEnable) && (g_strDeflateEnableState == COMPRESS_ENABLE_STATE))
    {
        spin_lock_irqsave(&g_strDeflateCtrl.intSpinLock, lock_flag);
        DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_RAWINT,  2, 1,1);
        DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_INTMASK ,  2, 1,1);

        DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_RAWINT,  6, 3,0x07);
        DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_INTMASK ,  6, 3,0x07);

        g_strDeflateEnableState = COMPRESS_DISABLE_STATE;
        spin_unlock_irqrestore(&g_strDeflateCtrl.intSpinLock, lock_flag);
    }
    else if((COMPRESS_ENABLE_STATE == bEnable) && (g_strDeflateEnableState == COMPRESS_DISABLE_STATE))
    {
        spin_lock_irqsave(&g_strDeflateCtrl.intSpinLock, lock_flag);
        DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_RAWINT,  2, 1,1);
        DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_INTMASK ,  2, 1,0);

        DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_RAWINT,  6, 3,0x07);
        DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_INTMASK ,  6, 3,0);
        g_strDeflateEnableState = COMPRESS_ENABLE_STATE;

        spin_unlock_irqrestore(&g_strDeflateCtrl.intSpinLock, lock_flag);
    }
    else
    {
        socp_crit(" deflate has enabled.\n");
    }

}
/*****************************************************************************
* 函 数 名  : bsp_deflate_data_send_continue
*
* 功能描述  : deflate目的端数据上报使能


* 注    意  : 该函数调用时，需要调用者保证同步

* 输入参数  :        bEnable: 中断使能
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
void bsp_deflate_data_send_continue(void)
{
    if(COMPRESS_ENABLE_STATE == g_strDeflateEnableState)
    {
        DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_RAWINT,  2, 1,1);
        DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_INTMASK ,  2, 1,0);

        DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_RAWINT,  6, 3,0x07);
        DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_INTMASK ,  6, 3,0);
    }
    else
    {
        socp_crit("deflate g_SocpEnableState = %d\n", g_strDeflateEnableState);
    }
}

s32 deflate_init(void)
{
    s32 ret;
    struct clk  *cDeflate;
    struct device_node* dev ;
    struct socp_enc_dst_log_cfg *cDeflateConfig  ;
    socp_compress_ops_stru ops;
    unsigned long lock_flag;

    ret = bsp_nvm_read(NV_ID_DRV_DEFLATE, (u8 *)&g_deflate_nv_ctrl, sizeof(DRV_DEFLATE_CFG_STRU));
    if(ret)
    {
       g_deflate_nv_ctrl.deflate_enable =0;
       socp_error("deflate read nv fail!\n");
       return ret;
    }
    if(!g_deflate_nv_ctrl.deflate_enable)
    {
        socp_error("deflate_init:deflate is diabled!\n");
        return DEFLATE_OK;
    }

    if(BSP_TRUE == g_strDeflateCtrl.initFlag)
    {
        socp_error("deflate init already!\n");
        return DEFLATE_OK;
    }
    spin_lock_init(&g_strDeflateCtrl.intSpinLock);
    osl_sem_init(0, &g_strDeflateCtrl.task_sem);

    /*解析dts,获取deflate基地址，并映射*/
    dev = of_find_compatible_node(NULL,NULL,"hisilicon,deflate_balong_app");
    if(NULL == dev)
    {
        socp_error("deflate dev find failed!\n");
        return DEFLATE_ERROR;
    }
    g_strDeflateCtrl.baseAddr = (void*)of_iomap(dev,0);
    if(NULL == g_strDeflateCtrl.baseAddr)
    {
        socp_error("deflate base addr is error!\n");
        return DEFLATE_ERROR;
    }
    /*deflate中断标志初始化*/
    g_strDeflateCtrl.u32IntDeflateDstOvf    =0;
    g_strDeflateCtrl.u32IntDeflateDstTfr    =0;
    g_strDeflateCtrl.u32IntDeflateDstThresholdOvf  =0;
    g_strDeflateCtrl.u32IntDeflateWorkAbort  =0 ;

    /*deflate目的通道属性初始化*/
    g_strDeflateCtrl.u32SetStat   =0;
    g_strDeflateCtrl.u32Thrh      =0;

    /* 打开defalte时钟 */
	cDeflate = clk_get(NULL, "clk_socp_deflat");
    if(IS_ERR(cDeflate))
    {
        socp_error("deflate prepare clk fail!\n");
        return DEFLATE_ERROR;
    }
    clk_prepare(cDeflate);
	if(BSP_OK !=clk_enable(cDeflate))
	{
		socp_error("deflate clk enable failed!\n");
		return DEFLATE_ERROR;
	}
    /*创建deflate任务 */
    ret = osl_task_init("deflateProc",DEFDST_TASK_PRO, 4096, (OSL_TASK_FUNC) deflate_task,NULL, &g_strDeflateCtrl.taskid);
    if (BSP_OK != ret)
    {
        socp_error("create task failed!\n");
        return DEFLATE_ERROR;
    }
    /*deflate软复位*/
    DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_GLOBALCTRL, 1, 1, 1);

    /*如果配置延时上报，需要配置超时寄存器*/
    cDeflateConfig=bsp_socp_get_log_cfg();
    DEFLATE_REG_WRITE(SOCP_REG_DEFLATE_TFRTIMEOUTCFG, DEFLATE_TIMEOUT_INDIRECT);

     /*lint -save -e845*/
    /*设置buffer溢出不丢数*/
    DEFLATE_REG_SETBITS(SOCP_REG_DEFLATEDEST_BUFOVFTIMEOUT, 31, 1, 0);
    /*使能255包压缩场景*/
    DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_GLOBALCTRL, 10, 8, 0x5);
    DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_GLOBALCTRL, 18, 1, 0);
    /*bypass 模式*/
    DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_GLOBALCTRL,  0, 1, 0);
    /*ibuf超时配置*/
    DEFLATE_REG_WRITE(SOCP_REG_DEFLATE_IBUFTIMEOUTCFG,0xFFFFFFFF);
    /* 清中断，屏蔽中断 */
    spin_lock_irqsave(&g_strDeflateCtrl.intSpinLock, lock_flag);
    DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_RAWINT, 1, 2, 0x03);
    DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_INTMASK, 1, 2,0x03 );
    DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_RAWINT, 4, 1, 1);
    DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_INTMASK, 4, 1, 1);
    DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_RAWINT, 6, 3, 0x07);
    DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_INTMASK, 6, 3, 0x07);
    spin_unlock_irqrestore(&g_strDeflateCtrl.intSpinLock, lock_flag);
    ops.isr                 = deflate_int_handler;
    ops.register_Eventcb    = deflate_register_event_cb;
    ops.register_Readcb     = deflate_register_read_cb;
    ops.enable              = deflate_enable;
    ops.disable             = deflate_disable;
    ops.set                 = deflate_set;
    ops.getbuffer           = deflate_get_read_buffer;
    ops.readdone            = deflate_read_data_done;
    ops.clear               = deflate_clear;
    bsp_socp_register_compress(&ops);

    /* 设置初始化状态 */
    g_strDeflateCtrl.initFlag = BSP_TRUE;
    socp_crit("[init]Deflate init succeed\n");
    return DEFLATE_OK;
}


s32 deflate_stop(u32 u32DstChanID)
{
    u32 u32RealChanID;
    u32 u32ChanType;

    /* 判断是否已经初始化 */
    if (!g_strDeflateCtrl.initFlag)
    {
        socp_error("the module has not been initialized!\n");
        return DEFLATE_ERR_NOT_INIT;
    }

    /* 判断通道ID是否有效 */
    u32RealChanID = DEFLATE_REAL_CHAN_ID(u32DstChanID);
    u32ChanType = DEFLATE_REAL_CHAN_TYPE(u32DstChanID);

    /* 编码通道 */
    if (DEFLATE_CODER_DEST_CHAN  == u32ChanType)
    {
        if (u32RealChanID < DEFLATE_MAX_ENCDST_CHN )
        {

            if (DEFLATE_CHN_SET!= g_strDeflateCtrl.u32SetStat)
            {
                socp_error("encoder src not allocated!\n" );
                return DEFLATE_ERR_INVALID_CHAN;
            }
        }
        else
        {
            socp_error("enc dst 0x%x is valid!\n", u32DstChanID);
            return DEFLATE_ERROR ;
        }

        /*停止deflate*/
        DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_GLOBALCTRL, 21, 1, 0);
      /*lint -restore +e845*/

        /* 复位通道 */

        DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_GLOBALCTRL,1, 1, 1);
    }
    return DEFLATE_OK;
}
/*****************************************************************************
* 函 数 名   : deflate_help
*
* 功能描述  :deflate 打印信息
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值   : 无
*****************************************************************************/
 void deflate_help(void)
 {
   socp_crit("deflate_show_debug_gbl: 查看全局统计信息:通道申请、配置和中断总计数\n");
   socp_crit("deflate_show_dest_chan_cur: 查看压缩目的通道信息\n");
 }
void deflate_show_debug_gbl(void)
{
    socp_crit("\r DEFLATE 全局状态维护信息:\n");
    socp_crit("\r deflate基地址:                                            : 0x%pK\n",
           g_strDeflateCtrl.baseAddr);
    socp_crit("\r deflate配置编码目的通道的次数                             : 0x%x\n",
           (s32)g_strDeflateDebug.u32DeflateDstSetCnt);
    socp_crit("\r deflate配置编码目的通道成功的次数                         : 0x%x\n",
           (s32)g_strDeflateDebug.u32DeflateDstSetSucCnt);
    socp_crit("\r deflate注册deflate目的通道读数据回调函数次数              : 0x%x\n",
           (s32)g_strDeflateDebug.u32DeflateRegReadCBCnt);
    socp_crit("\r deflate注册deflate目的通道异常事件回调函数次数            : 0x%x\n",
           (s32)g_strDeflateDebug.u32DeflateRegEventCBCnt);
    socp_crit("\r deflate尝试获取deflate目的buffer次数                      : 0x%x\n",
           (s32)g_strDeflateDebug.u32DeflateGetReadBufEtrCnt);
    socp_crit("\r deflate获取deflate目的buffer成功次数                      : 0x%x\n",
           (s32)g_strDeflateDebug.u32DeflateGetReadBufSucCnt);
    socp_crit("\r deflate尝试读取deflate目的数据次数                        : 0x%x\n",
           (s32)g_strDeflateDebug.u32DeflateReaddoneEtrCnt);
    socp_crit("\r deflate尝试读取deflate目的数据长度等于0次数               : 0x%x\n",
           (s32)g_strDeflateDebug.u32DeflateReaddoneZeroCnt);
    socp_crit("\r deflate读取deflate目的数据长度不等于0次数                 : 0x%x\n",
           (s32)g_strDeflateDebug.u32DeflateReaddoneValidCnt);
    socp_crit("\r deflate读取deflate目的数据失败的次数                      : 0x%x\n",
           (s32)g_strDeflateDebug.u32DeflateReaddoneFailCnt);
    socp_crit("\r deflate读取deflate目的数据成功的次数                      : 0x%x\n",
           (s32)g_strDeflateDebug.u32DeflateReaddoneSucCnt);
    socp_crit("\r deflate处理传输中断任务的次数                             : 0x%x\n",
           (s32)g_strDeflateDebug.u32DeflateTskTrfCbOriCnt);
    socp_crit("\r deflate处理完传输中断任务的次数                           : 0x%x\n",
           (s32)g_strDeflateDebug.u32DeflateTskTrfCbCnt);
    socp_crit("\r deflate处理上溢中断的次数                                 : 0x%x\n",
           (s32)g_strDeflateDebug.u32DeflateTskOvfCbOriCnt);
    socp_crit("\r deflate处理完上溢中断的次数*                              : 0x%x\n",
           (s32)g_strDeflateDebug.u32DeflateTskOvfCbCnt);
    socp_crit("\r deflate处理阈值溢出的次数                                 : 0x%x\n",
           (s32)g_strDeflateDebug.u32DeflateTskThresholdOvfCbOriCnt);
    socp_crit("\r deflate处理完阈值溢出的次数                               : 0x%x\n",
           (s32)g_strDeflateDebug.u32DeflateTskThresholdOvfCbCnt);
    socp_crit("\r deflate处理异常的次数                                     : 0x%x\n",
           (s32)g_strDeflateDebug.u32DeflateTskIntWorkAortCbOriCnt);
    socp_crit("\r deflate处理完异常的次数                                   : 0x%x\n",
           (s32)g_strDeflateDebug.u32DeflateTskIntWorkAortCbCnt);
}
u32 deflate_show_dest_chan_cur(void)
{

    socp_crit("初始化状态:0x%x\n",g_strDeflateCtrl.initFlag);
    socp_crit("通道ID:0x%x\n",g_strDeflateCtrl.u32ChanID);
    socp_crit(" 通道配置状态:%d\n",g_strDeflateCtrl.u32SetStat);
    socp_crit("通道buffer 起始地址:0x%pK\n",(char *)g_strDeflateCtrl.sDeflateDstChan.Start);
    socp_crit("通道buffer 结束地址:0x%pK\n",(char *)g_strDeflateCtrl.sDeflateDstChan.End);
    socp_crit("通道buffer 读指针:0x%x\n",g_strDeflateCtrl.sDeflateDstChan.u32Read);
    socp_crit("通道buffer 写指针:0x%x\n",g_strDeflateCtrl.sDeflateDstChan.u32Write);
    socp_crit("通道buffer 长度:0x%x\n",g_strDeflateCtrl.sDeflateDstChan.u32Length);
    return BSP_OK;
}

static s32 deflate_suspend(struct device *dev)
{
    u32 DeflateIdleState;

    /*立即压缩*/
    DEFLATE_REG_SETBITS(SOCP_REG_DEFLATE_GLOBALCTRL, 20, 1, 1);

      /*读deflate全局状态寄存器，获取deflate工作状态*/
    DEFLATE_REG_READ(SOCP_REG_DEFLATE_GLOBALCTRL,DeflateIdleState);
    while(!(DeflateIdleState & DEFLATE_WORK_STATE))
    {

       DEFLATE_REG_READ(SOCP_REG_DEFLATE_GLOBALCTRL,DeflateIdleState);
    }

    socp_crit("open signle packet compress!\n");
    return 0;
}
static s32 deflate_resume(struct device *dev)
{
    return 0;
}
static const struct dev_pm_ops deflate_pm_ops ={
    .suspend = deflate_suspend,
    .resume  = deflate_resume,
};

#define DEFLATE_PM_OPS (&deflate_pm_ops)

static struct platform_driver modem_deflate_drv = {
    .driver     = {
        .name     = "modem_deflate",
        .owner    = (struct module *)(unsigned long)THIS_MODULE,
        .pm       = DEFLATE_PM_OPS,
    },
};

static struct platform_device modem_deflate_device = {
    .name = "modem_deflate",
    .id = 0,
    .dev = {
    .init_name = "modem_deflate",
    },
};

int deflate_dev_init(void)
{
    u32 ret;
    if(!g_deflate_nv_ctrl.deflate_enable)
    {
        socp_error("deflate is diabled!\n");
        return DEFLATE_OK;
    }
    ret = (u32)platform_device_register(&modem_deflate_device);
    if(ret)
    {
        socp_crit("modem_deflate_device fail !\n");
        return -1;
    }

    ret = (u32)platform_driver_register(&modem_deflate_drv);
    if(ret)
    {
        socp_crit("modem_deflate_drv fail !\n");
        platform_device_unregister(&modem_deflate_device);
        return -1;
    }
    return 0;
}


