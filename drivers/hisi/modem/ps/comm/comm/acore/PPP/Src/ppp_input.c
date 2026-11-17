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


#include "ppp_input.h"
#include "ppp_public.h"
#include "hdlc_interface.h"
#include "product_config.h"
#include "TTFComm.h"
#include "gucttf_tag.h"
#include "securec.h"
#include "pppc_pppa_interface.h"

#if (FEATURE_OFF == FEATURE_HARDWARE_HDLC_FUNC)
#include "hdlc_software.h"
#include "ppp_convert.h"
#else
#include "hdlc_hardware.h"
#endif

#if (FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
#include "ads_dev_i.h"
#else
#include "AdsDeviceInterface.h"
#endif



/******************************************************************************
   1 头文件包含
******************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_PPP_INPUT_C
#define    THIS_MODU           mod_ppp

#define PPPA_DATA_QUEUE_MAX_CNT     1500

typedef struct
{
    PPP_ZC_QUEUE_STRU           stDataQ;                    /* PPP数据队列，上下行数据都在其中 */
    VOS_UINT32                  dataQMaxCnt;                /* PPP数据队列最大深度 */
    PPP_DATA_Q_STAT_ST          stStat;                     /* PPP数据队列的统计信息 */
    HTIMER                      delayProtTimer;             /* 延时处理定时器去 */
    volatile VOS_UINT32         ulNotifyMsg;                /* 通知PPP处理数据 */
    VOS_SPINLOCK                spinLock;                   /* ulNotifyMsg在多个任务中会被修改，因此需要使用自旋锁 */
    VOS_UINT32                  pppTaskId;                  /* PPP任务ID，通知PPP任务有数处理时使用 */
    VOS_UINT32                  rawDataByPass;              /* RAWDATA拨号时数据是否是透传模式 */
}PPP_DATA_Q_CTRL_ST;


/*PPP的数据队列结构体,上下行数据都在同一个队列中*/
PPP_DATA_Q_CTRL_ST     g_PppDataQCtrl;

#define PPPA_GET_DATAQ_CTRL()   &g_PppDataQCtrl
#define PPPA_GET_STAT_ADDR()    &(g_PppDataQCtrl.stStat)
#define PPPA_DATA_DELAY_PROC_TIMER_LEN  10

VOS_BOOL    g_pppUsed           = VOS_FALSE;
VOS_UINT32  g_pppDelayTimerLen  = PPPA_DATA_DELAY_PROC_TIMER_LEN;

VOS_VOID PPP_ClearDataQ(VOS_VOID)
{
    PPP_DATA_Q_CTRL_ST *dataQCtrl = PPPA_GET_DATAQ_CTRL();
    PPP_ZC_STRU        *pstMem = (PPP_ZC_STRU *)PPP_ZC_DEQUEUE_HEAD(&(dataQCtrl->stDataQ));

    while (pstMem != VOS_NULL_PTR) {
        PPP_MemFree(pstMem);
        pstMem  = (PPP_ZC_STRU *)PPP_ZC_DEQUEUE_HEAD(&(dataQCtrl->stDataQ));
    }
}

VOS_UINT32 PPP_DataQInit(VOS_VOID)
{
    PPP_DATA_Q_CTRL_ST *dataQCtrl = PPPA_GET_DATAQ_CTRL();
    VOS_UINT32          rslt = VOS_OK;

    (VOS_VOID)memset_s(dataQCtrl, sizeof(PPP_DATA_Q_CTRL_ST), 0, sizeof(PPP_DATA_Q_CTRL_ST));

    PPP_ZC_QUEUE_INIT(&(dataQCtrl->stDataQ));
    dataQCtrl->rawDataByPass = VOS_FALSE;
    dataQCtrl->ulNotifyMsg = VOS_TRUE;
    dataQCtrl->dataQMaxCnt = PPPA_DATA_QUEUE_MAX_CNT;

    PPPA_SetDataStatAddr(&(dataQCtrl->stStat));
    
    VOS_SpinLockInit(&(dataQCtrl->spinLock));

#if (FEATURE_ON == FEATURE_HARDWARE_HDLC_FUNC)
    rslt = PPP_HDLC_HARD_Init();
#endif
    return rslt;
}

VOS_VOID PPPA_SaveTaskId(VOS_UINT32 taskId)
{
    PPP_DATA_Q_CTRL_ST *dataQCtrl = PPPA_GET_DATAQ_CTRL();

    dataQCtrl->pppTaskId = taskId;
}


VOS_UINT32 PPP_GetRawDataByPassMode(VOS_VOID)
{
    PPP_DATA_Q_CTRL_ST *dataQCtrl = PPPA_GET_DATAQ_CTRL();
    
    return dataQCtrl->rawDataByPass;
}

VOS_VOID PPP_SetRawDataByPassMode(VOS_UINT32 ulRawDataByPassMode)
{
    PPP_DATA_Q_CTRL_ST *dataQCtrl = PPPA_GET_DATAQ_CTRL();
    
    dataQCtrl->rawDataByPass = ulRawDataByPassMode;
}

VOS_UINT32 PPP_INPUT_GetDataCnt(VOS_VOID)
{
    PPP_DATA_Q_CTRL_ST *dataQCtrl = PPPA_GET_DATAQ_CTRL();

    return PPP_ZC_GET_QUEUE_LEN(&(dataQCtrl->stDataQ));
}


VOS_VOID PPP_NotiFyProcData(PPP_DATA_Q_CTRL_ST *dataQCtrl)
{
    VOS_ULONG           ulFlags     = 0UL;
    
    VOS_SpinLockIntLock(&(dataQCtrl->spinLock), ulFlags);

    if (dataQCtrl->ulNotifyMsg != VOS_TRUE) {
        VOS_SpinUnlockIntUnlock(&(dataQCtrl->spinLock) , ulFlags);
        return;
    }
    dataQCtrl->ulNotifyMsg = VOS_FALSE;
    VOS_SpinUnlockIntUnlock(&(dataQCtrl->spinLock) , ulFlags);
    dataQCtrl->stStat.ulSndMsgCnt++;
    (VOS_VOID)VOS_EventWrite(dataQCtrl->pppTaskId, PPP_RCV_DATA_EVENT);
}


VOS_UINT32 PPP_EnqueueData(PPP_ZC_STRU *pstImmZc, PPP_DATA_TYPE_ENUM_UINT8 enDataType, PPP_ID usPppId)
{
    PPP_DATA_Q_CTRL_ST *dataQCtrl   = PPPA_GET_DATAQ_CTRL();
    PPP_ZC_QUEUE_STRU  *pstDataQ    = &(dataQCtrl->stDataQ);

    if (PPP_ZC_GET_QUEUE_LEN(pstDataQ) > dataQCtrl->dataQMaxCnt)
    {
        PPP_MemFree(pstImmZc);
        dataQCtrl->stStat.ulDropCnt++;
        PPP_NotiFyProcData(dataQCtrl);
        return PS_FAIL;
    }

    /*将数据结点插入队列尾部*/
    PPP_ZC_ENQUEUE_TAIL(pstDataQ, pstImmZc);

    if (PPP_ZC_GET_QUEUE_LEN(pstDataQ) > dataQCtrl->stStat.ulQMaxCnt)
    {
        dataQCtrl->stStat.ulQMaxCnt = PPP_ZC_GET_QUEUE_LEN(pstDataQ);
    }

    PPP_NotiFyProcData(dataQCtrl);

    return PS_SUCC;
} /* PPP_EnqueueData */

VOS_UINT32 PPP_PullPacketEvent(VOS_UINT16 usPppId, IMM_ZC_STRU *pstImmZc)
{
    PPP_DATA_Q_STAT_ST *dataStat = PPPA_GET_STAT_ADDR();
    if (pstImmZc == VOS_NULL_PTR)
    {
        PPP_MNTN_LOG(PS_PRINT_WARNING,
                      "PPP_PullPacketEvent, WARNING, pstImmZc is NULL!\r\n" );

        return PS_FAIL;
    }

    dataStat->ulUplinkCnt++;

    /*填充pstData的usApp字段:高8位放usPppId,低8位放PPP报文类型*/
    PPP_ZC_SET_DATA_APP(pstImmZc, (VOS_UINT16)(usPppId << 8) | (VOS_UINT16)PPP_PULL_PACKET_TYPE);

    if ( PS_SUCC != PPP_EnqueueData(pstImmZc, PPP_PULL_PACKET_TYPE, (PPP_ID)usPppId)) {
        dataStat->ulUplinkDropCnt++;
    }

    return PS_SUCC;
} /* PPP_PullPacketEvent */

VOS_UINT32 PPP_PullRawDataEvent(VOS_UINT16 usPppId, IMM_ZC_STRU *pstImmZc)
{
    PPP_ZC_STRU        *pstMem = VOS_NULL_PTR;
    PPP_DATA_Q_STAT_ST *dataStat = PPPA_GET_STAT_ADDR();

    dataStat->ulUplinkCnt++;

    if(VOS_NULL_PTR == pstImmZc)
    {
        PPP_MNTN_LOG(PS_PRINT_WARNING,
                      "PPP_PullRawDataEvent, WARNING, Alloc TTF mem fail!\r\n" );

        return PS_FAIL;
    }
    
    if (PPP_GetRawDataByPassMode() == VOS_TRUE){
#if (FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
        VOS_UINT16                          usDataLen;

        /* 递交给ADS的数据需要预留MAC头，因此透传模式下需要重新申请内存 */
        usDataLen = PPP_ZC_GET_DATA_LEN(pstImmZc);
        if ((0 == usDataLen) || (usDataLen > PPP_ZC_MAX_DATA_LEN))
        {
            dataStat->ulUplinkDropCnt++;
            PPP_MNTN_LOG1(PS_PRINT_WARNING, "DataLen err", usDataLen);
            PPP_MemFree(pstImmZc);
            return PS_FAIL;
        }
        
        pstMem = PPP_MemCopyAlloc(PPP_ZC_GET_DATA_PTR(pstImmZc), usDataLen, PPP_ZC_UL_RESERVE_LEN);
        PPP_MemFree(pstImmZc);
        if (VOS_NULL_PTR == pstMem)
        {
            PPP_MNTN_LOG(PS_PRINT_WARNING, "Alloc Mem Fail");
            dataStat->ulUplinkDropCnt++;
            return PS_FAIL;
        }
#else
        pstMem = pstImmZc;
#endif
        /* PPP模式透传模式下递交给ADS的是PPP报文，因此协议类型填0 */
        if (PS_SUCC != PPP_SendUlDataToAds(usPppId, pstMem, 0))
        {
            return PS_FAIL;
        }
    } else {
        /*填充pstData的usApp字段:高8位放usPppId,低8位放PPP报文类型*/
        PPP_ZC_SET_DATA_APP(pstImmZc, (VOS_UINT16)(usPppId << 8) | (VOS_UINT16)PPP_PULL_RAW_DATA_TYPE);

        if ( PS_SUCC != PPP_EnqueueData(pstImmZc, PPP_PULL_RAW_DATA_TYPE, (PPP_ID)usPppId) ) {
            dataStat->ulUplinkDropCnt++;
        }
    }

    return PS_SUCC;
} /* PPP_PullRawEvent */

VOS_BOOL PPPA_GetUsedFalg(VOS_VOID)
{
    return g_pppUsed;
}
VOS_INT PPP_DlPacketProc(PPP_ID usPppId, IMM_ZC_STRU *pstImmZc)
{
    PPP_DATA_Q_STAT_ST *dataStat = PPPA_GET_STAT_ADDR();

    if(pstImmZc == VOS_NULL_PTR) {
        PPP_MNTN_LOG(PS_PRINT_WARNING, "PPP_PushPacketEvent, WARNING, pstImmZc is NULL!\r\n" );
        return PS_FAIL;
    }

    /*如果该链接还没建立起来*/
    if (PPPA_GetUsedFalg() != VOS_TRUE) {
        /*该变量需要被初始化为0*/
        dataStat->ulDownlinkDropCnt++;
        PPP_MNTN_LOG1(PS_PRINT_NORMAL, "ppp id err", dataStat->ulDownlinkDropCnt);
        PPP_MemFree(pstImmZc);
        return PS_FAIL;
    }

    /*填充pstData的usApp字段:高8位放usPppId,低8位放PPP报文类型*/
    PPP_ZC_SET_DATA_APP(pstImmZc, (VOS_UINT16)(usPppId << 8) | (VOS_UINT16)PPP_PUSH_PACKET_TYPE);

    if ( PS_SUCC != PPP_EnqueueData(pstImmZc, PPP_PUSH_PACKET_TYPE, usPppId) ) {
        dataStat->ulDownlinkDropCnt++;
    }

    return PS_SUCC;
}

VOS_VOID PPPA_SendDlDataToAT(PPP_ID pppId, IMM_ZC_STRU *data)
{
    PPP_DATA_Q_STAT_ST *dataStat = PPPA_GET_STAT_ADDR();

    AT_SendZcDataToModem(pppId, data);
    dataStat->ulDownlinkSndDataCnt++;
}

VOS_INT PPP_DlRawDataProc(PPP_ID usPppId, IMM_ZC_STRU *pstImmZc)
{
    PPP_DATA_Q_STAT_ST *dataStat = PPPA_GET_STAT_ADDR();
    
    if(pstImmZc == VOS_NULL_PTR) {
        PPP_MNTN_LOG(PS_PRINT_WARNING,
                      "PPP_PushRawDataEvent, WARNING, pstImmZc is NULL!\r\n" );

        return PS_FAIL;
    }

    if(PPPA_GetUsedFalg() != VOS_TRUE) {
        dataStat->ulDownlinkDropCnt++;
        PPP_MemFree(pstImmZc);
        PPP_MNTN_LOG(PS_PRINT_WARNING,
                     "PPP_PushRawData, WARNING, Invalid PPP id, packet from GGSN droped\r\n");

        return PS_FAIL;
    }

    if (PPP_GetRawDataByPassMode() == VOS_TRUE) {
        PPPA_SendDlDataToAT(usPppId, pstImmZc);
    } else {
        /*填充pstData的usApp字段:高8位放usPppId,低8位放PPP报文类型*/
        PPP_ZC_SET_DATA_APP(pstImmZc, (VOS_UINT16)(usPppId << 8) | (VOS_UINT16)PPP_PUSH_RAW_DATA_TYPE);

        if (PS_SUCC != PPP_EnqueueData(pstImmZc, PPP_PUSH_RAW_DATA_TYPE, usPppId)) {
            dataStat->ulDownlinkDropCnt++;
            return PS_FAIL;
        }
    }

    return PS_SUCC;
}

#if (FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
VOS_INT PPP_PushPacketEvent(VOS_ULONG ulUserData, IMM_ZC_STRU *pstImmZc)
{
    PPP_ID              usPppId  = (PPP_ID)ulUserData;
    PPP_DATA_Q_STAT_ST *dataStat = PPPA_GET_STAT_ADDR();

    dataStat->ulDownlinkCnt++;
    return PPP_DlPacketProc(usPppId, pstImmZc);
}

VOS_INT PPP_PushRawDataEvent(VOS_ULONG ulUserData, IMM_ZC_STRU *pstImmZc)
{
    PPP_ID              usPppId  = (PPP_ID)ulUserData;
    PPP_DATA_Q_STAT_ST *dataStat = PPPA_GET_STAT_ADDR();

    dataStat->ulDownlinkCnt++;
    return PPP_DlRawDataProc(usPppId, pstImmZc);
}
VOS_UINT32 PPP_SendUlDataToAds(VOS_UINT16 usPppId,  PPP_ZC_STRU *pstImmZc, VOS_UINT16 usProto)
{
    VOS_UINT8               ucIfaceId = 0;
    PPP_DATA_Q_STAT_ST     *dataStat    = PPPA_GET_STAT_ADDR();

    /* 通过usPppId，寻找到ucIfaceId */
    if (VOS_OK != At_PppId2IfaceId(usPppId, &ucIfaceId))
    {
        dataStat->ulUplinkDropCnt++;
        PPP_MemFree(pstImmZc);
        PPP_MNTN_LOG2(PS_PRINT_NORMAL, "Can not get IfaceId", usPppId, ucIfaceId);

        return PS_FAIL;
    }

    IMM_ZcSetProtocol(pstImmZc, usProto);

    /* 数据发送给ADS，如果失败由ADS释放内存 */
    if ( VOS_OK != ads_iface_tx(ucIfaceId, pstImmZc) )
    {
        dataStat->ulUplinkDropCnt++;

        return PS_FAIL;
    }

    dataStat->ulUplinkSndDataCnt++;

    return PS_SUCC;
}

#else

VOS_INT PPP_PushPacketEvent(VOS_UINT8 ucRabId, IMM_ZC_STRU *pstImmZc, ADS_PKT_TYPE_ENUM_UINT8 enPktType, VOS_UINT32 ulExParam)
{
    PPP_ID              usPppId;
    PPP_DATA_Q_STAT_ST *dataStat = PPPA_GET_STAT_ADDR();

    dataStat->ulDownlinkCnt++;

    if ( !PPP_RAB_TO_PPPID(&usPppId, ucRabId) )
    {
        dataStat->ulDownlinkDropCnt++;
        PPP_MemFree(pstImmZc);
        PPP_MNTN_LOG1(PS_PRINT_NORMAL,
                      "PPP, PPP_PushPacketEvent, NORMAL, Can not get PPP Id, RabId <1>", ucRabId);

        return PS_FAIL;
    }

    return PPP_DlPacketProc(usPppId, pstImmZc);
}


VOS_INT PPP_PushRawDataEvent(VOS_UINT8 ucRabId, IMM_ZC_STRU *pstImmZc, ADS_PKT_TYPE_ENUM_UINT8 enPktType, VOS_UINT32 ulExParam)
{
    PPP_ID              usPppId;
    PPP_DATA_Q_STAT_ST *dataStat = PPPA_GET_STAT_ADDR();

    dataStat->ulDownlinkCnt++;

    /* 通过RabId，寻找到PPP ID和相应的实体 */
    if ( !PPP_RAB_TO_PPPID(&usPppId, ucRabId) )
    {
        PPP_MemFree(pstImmZc);
        PPP_MNTN_LOG1(PS_PRINT_WARNING,
                      "PPP, PPP_PushRawDataEvent, WARNING, Can not get PPP Id, RabId <1>", ucRabId);

        return PS_FAIL;
    }

    return PPP_DlRawDataProc(usPppId, pstImmZc);
}
VOS_UINT32 PPP_SendUlDataToAds(VOS_UINT16 usPppId,  PPP_ZC_STRU *pstImmZc, VOS_UINT16 usProto)
{
    VOS_UINT8               ucRabId  = 0;
    PPP_DATA_Q_STAT_ST     *dataStat = PPPA_GET_STAT_ADDR();

    /* 通过usPppId，寻找到usRabId */
    if ( !PPP_PPPID_TO_RAB(usPppId, &ucRabId) )
    {
        dataStat->ulUplinkDropCnt++;
        PPP_MemFree(pstImmZc);
        PPP_MNTN_LOG2(PS_PRINT_NORMAL,
                    "PPP, PPP_PushPacketEvent, WARNING, Can not get PPP Id %d, RabId %d",
                    usPppId, ucRabId);

        return PS_FAIL;
    }

    /* 数据发送给ADS，如果失败由ADS释放内存 */
    if ( VOS_OK != ADS_UL_SendPacket(pstImmZc, ucRabId) )
    {
        dataStat->ulUplinkDropCnt++;

        return PS_FAIL;
    }

    dataStat->ulUplinkSndDataCnt++;

    return PS_SUCC;
}

#endif

VOS_VOID PPPA_SetNotifyFlag(VOS_UINT32 notifyFlag)
{
    VOS_ULONG           ulFlags     = 0UL;
    PPP_DATA_Q_CTRL_ST *dataQCtrl   = PPPA_GET_DATAQ_CTRL();
    
    VOS_SpinLockIntLock(&(dataQCtrl->spinLock), ulFlags);    /*lint !e571*/
    dataQCtrl->ulNotifyMsg = notifyFlag;
    VOS_SpinUnlockIntUnlock(&(dataQCtrl->spinLock), ulFlags);
}

VOS_VOID PPP_ProcDataNotify(VOS_VOID)
{
    PPP_ID                  usPppId;
    VOS_BOOL                delayProc = VOS_FALSE;
    PPP_DATA_Q_STAT_ST     *dataStat    = PPPA_GET_STAT_ADDR();
    PPP_DATA_Q_CTRL_ST     *dataQCtrl   = PPPA_GET_DATAQ_CTRL();
    
    PPP_ZC_STRU *pstMem = (PPP_ZC_STRU *)PPP_ZC_PEEK_QUEUE_HEAD(&dataQCtrl->stDataQ);

    dataStat->ulProcMsgCnt++;

    /* 队列为空的时候返回空指针 */
    if ( VOS_NULL_PTR == pstMem )
    {
        PPP_MNTN_LOG2(LOG_LEVEL_WARNING,
                      "PPP_ProcDataNotify, WARNING, queue is null!",
                      dataQCtrl->ulNotifyMsg, PPP_ZC_GET_QUEUE_LEN(&(dataQCtrl->stDataQ)));
        PPPA_SetNotifyFlag(VOS_TRUE);
        return;
    }

    /* 处理该结点(结点的释放动作已经在各处理函数内部完成，无需再释放结点) */
    usPppId         = (PPP_ZC_GET_DATA_APP(pstMem) & 0xFF00) >> 8;

#if (FEATURE_OFF == FEATURE_HARDWARE_HDLC_FUNC)
    delayProc = PPP_HDLC_SOFT_ProcData(usPppId, &g_PppDataQCtrl.stDataQ);
#else
    delayProc = PPP_HDLC_HARD_ProcData(usPppId, &g_PppDataQCtrl.stDataQ);
#endif

    /* 如果需要延时处理则启动定时器，此时不需设置通知标记 */
    if (delayProc == VOS_TRUE) {
        VOS_StartRelTimer(&(dataQCtrl->delayProtTimer), PS_PID_APP_PPP, g_pppDelayTimerLen,
                     PPPA_DATA_DELAY_PROC_TIMER, (VOS_UINT32)usPppId, VOS_RELTIMER_NOLOOP, VOS_TIMER_PRECISION_0);
        return;
    }
    
    PPPA_SetNotifyFlag(VOS_TRUE);
    if (PPP_ZC_GET_QUEUE_LEN(&(dataQCtrl->stDataQ)) > 0) {
        PPP_NotiFyProcData(dataQCtrl);
    }
}
/* 目前协商报文是通过VOS消息发送，每秒最多发送100个协商报文 */
#define PPPA_MAX_NEGO_PKT_CNT_PER_SEC   100

/* 目前为32K时钟，1s为32768个slice */
#define PPPA_SLICE_CNT_OF_ONE_SEC       32768

VOS_VOID PPPA_SendPppcNegoData(PPP_ZC_STRU *negoData, VOS_UINT16 proto)
{
    PPPC_PPPA_NEGO_DATA_STRU       *negoMsg = VOS_NULL_PTR;
    VOS_UINT16                      msgLen = sizeof(PPPC_PPPA_NEGO_DATA_STRU) + PPP_ZC_GET_DATA_LEN(negoData);
    static VOS_UINT16               msgSendCnt = 0;
    static VOS_UINT32               beginTime = 0;
    VOS_UINT32                      currTime;

    if (msgSendCnt == 0) {
        /* 发送第一个报文时记录起始时间 */
        beginTime = mdrv_timer_get_normal_timestamp();
    } else {
        currTime = mdrv_timer_get_normal_timestamp();
        if (currTime - beginTime > PPPA_SLICE_CNT_OF_ONE_SEC) {
            /* 当前时间和起始时间间隔超过1s，重新开始计数 */
            msgSendCnt = 0;
            beginTime = currTime;
        } else {
            /* 1s内发送的协商报文数量大于100，不能继续发送协商报文 */
            if (msgSendCnt > PPPA_MAX_NEGO_PKT_CNT_PER_SEC) {
                PPP_MNTN_LOG(PS_PRINT_WARNING, "too many nego pkt");
                return;
            }
        }
    }
    msgSendCnt++;
    negoMsg = (PPPC_PPPA_NEGO_DATA_STRU*)PS_ALLOC_MSG_WITH_HEADER_LEN(PS_PID_APP_PPP, msgLen);
    if (negoMsg == VOS_NULL_PTR) {
        PPP_MNTN_LOG(PS_PRINT_WARNING, "alloc Msg Fail");
        return;
    }

    negoMsg->ulReceiverPid  = MSPS_PID_PPPC;
    negoMsg->msgId          = ID_PPPA_PPPC_NEGO_DATA_IND;
    negoMsg->proto          = proto;
    negoMsg->dataLen        = PPP_ZC_GET_DATA_LEN(negoData);
    negoMsg->pktType        = PPPC_PPPA_PKT_TYPE_BUTT;

    if (negoMsg->dataLen > 0) {
        PPPA_SF_CHK(memcpy_s(negoMsg->data, negoMsg->dataLen, PPP_ZC_GET_DATA_PTR(negoData), negoMsg->dataLen));
    }

    (VOS_VOID)PS_SEND_MSG(PS_PID_APP_PPP, negoMsg);
}


VOS_VOID PPP_FrameMntnInfo
(
    PPP_FRAME_MNTN_INFO_STRU *ptrPppMntnSt,
    VOS_UINT16                usDataLen
)
{
    ptrPppMntnSt->ulReceiverPid   = MSPS_PID_PPPC;
    ptrPppMntnSt->ulSenderPid     = PS_PID_APP_PPP;
    ptrPppMntnSt->ulLength        = (usDataLen + sizeof(PPP_FRAME_MNTN_INFO_STRU))- VOS_MSG_HEAD_LENGTH;
    ptrPppMntnSt->ulMsgname       = PPP_RECV_PROTO_PACKET_TYPE;
    ptrPppMntnSt->ulPppPhase      = 0;
    ptrPppMntnSt->ulIpcpState     = 0;
    ptrPppMntnSt->ulLcpState      = 0;
    ptrPppMntnSt->usPppId         = PPPA_PPP_ID;
    ptrPppMntnSt->usDataLen       = usDataLen;

    PPP_MNTN_TRACE_MSG(ptrPppMntnSt);
}

VOS_VOID PPP_TtfMemFrameMntnInfo
(
    PPP_ZC_STRU *pstMem,
    VOS_UINT16   usProto
)
{
    VOS_UINT16                usFrameLen        = 0;
    PPP_FRAME_MNTN_INFO_STRU *ptrPppFrameMntnSt = VOS_NULL_PTR;
    VOS_UINT8                *pucBuff           = VOS_NULL_PTR;
    VOS_UINT32                ulRet             = PS_FAIL;

    /* PPP 帧长度*/
    usFrameLen = (VOS_UINT16)PPP_ZC_GET_DATA_LEN(pstMem);

    ptrPppFrameMntnSt = (PPP_FRAME_MNTN_INFO_STRU *)PS_MEM_ALLOC(PS_PID_APP_PPP,
                        usFrameLen + sizeof(PPP_FRAME_MNTN_INFO_STRU) + sizeof(usProto));
    if (VOS_NULL_PTR == ptrPppFrameMntnSt)
    {
        PPP_MNTN_LOG(PS_PRINT_WARNING,"PPP, PPP_TtfMemFrameMntnInfo, ERROR, Call VOS_MemAlloc fail!\n");
        return;
    }

    /* 填入PPP帧协议类型,网络字节序*/
    pucBuff = (VOS_UINT8 *)(ptrPppFrameMntnSt + 1);
    *((VOS_UINT16 *)(pucBuff)) = (VOS_HTONS(usProto)); 
    (pucBuff) += 2;         

    /* 填入帧内容,原始复制即可*/
    ulRet = PPP_MemGet(pstMem, 0, pucBuff, usFrameLen);
    if (PS_SUCC != ulRet)
    {
        PPP_MNTN_LOG(PS_PRINT_WARNING,"PPP, Ppp_frame_MntnInfo, ERROR, TTF_MemGet Fail!\n");
        PS_MEM_FREE(PS_PID_APP_PPP, ptrPppFrameMntnSt);
        return ;
    }

    /****************************************************************
       发送可维可测信息
       长度: 除可维可测信息头部外数据部分载荷
             涵盖帧长 + 2byte protocol字段
    *****************************************************************/
    PPP_FrameMntnInfo(ptrPppFrameMntnSt, usFrameLen + sizeof(usProto));

    /* 释放内存*/
    PS_MEM_FREE(PS_PID_APP_PPP, ptrPppFrameMntnSt);
}

VOS_VOID PPP_HDLC_ProcIpModeUlData(VOS_UINT16  pppId, PPP_ZC_STRU *pstMem, VOS_UINT16  usProto)
{
    if (usProto != PPPA_IP)
    {
        /* 把PPP协商包作为可维可测信息,IP包不做可维可测维护*/
        PPP_TtfMemFrameMntnInfo(pstMem, usProto);
        PPPA_SendPppcNegoData(pstMem, usProto); 
        PPP_MemFree(pstMem);
    } else {
        PPP_SendUlDataToAds(pppId, pstMem, PPPA_ETH_IPV4_PROTO);
    } 
}


VOS_VOID PPP_SetupHdlc(VOS_UINT16 pppId, VOS_BOOL ipMode)
{
#if (FEATURE_OFF == FEATURE_HARDWARE_HDLC_FUNC)
    PPP_HDLC_SetUp(pppId, ipMode);
#else
    PPP_Service_HdlcHardSetUp(pppId);
#endif
}

VOS_VOID PPP_ReleaseHdlc(VOS_UINT16 pppId)
{
#if (FEATURE_OFF == FEATURE_HARDWARE_HDLC_FUNC)
    PPP_CONV_Release(pppId);
#endif

}

VOS_VOID PPPA_SetUsedFlag(VOS_BOOL isUsed)
{
    g_pppUsed = isUsed;
}

VOS_VOID PPPA_DataEventProc(VOS_VOID)
{
#if (FEATURE_ON == FEATURE_HARDWARE_HDLC_FUNC)
    PPP_Service_HdlcHardOpenClk();
#endif

    PPP_ProcDataNotify();

#if (FEATURE_ON == FEATURE_HARDWARE_HDLC_FUNC)
    PPP_Service_HdlcHardCloseClk();
#endif
}

VOS_VOID PPPA_INPUT_ProcEchoReq(VOS_VOID)
{
    static VOS_UINT32   preDropCnt = 0;
    PPP_DATA_Q_STAT_ST *dataStat = PPPA_GET_STAT_ADDR();

    if (dataStat->ulDropCnt == preDropCnt) {
        return;
    }
    preDropCnt = dataStat->ulDropCnt;
    PPPA_SendPppcCommMsg(ID_PPPA_PPPC_IGNORE_ECHO);
}




