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

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "CsdDownLink.h"
#include "CsdDebug.h"



/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID                 PS_FILE_ID_CSD_DOWN_LINK_C

#if( FEATURE_ON == FEATURE_CSD )
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
extern VOS_UINT32 AT_SendCsdZcDataToModem(
    VOS_UINT8                           ucIndex,
    IMM_ZC_STRU                        *pstDataBuf
);


/*****************************************************************************
  3 函数实现
*****************************************************************************/


VOS_VOID CSD_DL_ProcIsr(VOS_VOID)
{
    VOS_SEM                             hDLDataSem = VOS_NULL_PTR;

    hDLDataSem  = CSD_GetDownLinkDataSem();

    /*释放下行取数据信号量*/
    VOS_SmV(hDLDataSem);
}


VOS_VOID CSD_DL_SendData(VOS_VOID)
{
    IMM_ZC_STRU                        *pstCsdDLMem;
    CST_CSD_DATA_IND_STRU               stDLData;
    VOS_UINT32                          ulDICCNodeCnt;
    VOS_UINT32                          ulRslt;
    VOS_CHAR                           *ImmZcData;


    for ( ; ; )
    {
        /*获取DICC通道发送来数据的个数*/
        ulDICCNodeCnt = DICC_GetChannelCurrDataCnt(ACPU_PID_CSD,
                                                   DICC_CHAN_ID_DL_CSD_DATA_CHAN,
                                                   DICC_CPU_ID_ACPU);

        if ( DICC_INVALID_VALUE == ulDICCNodeCnt )
        {
            CSD_ERROR_LOG1(ACPU_PID_CSD,
                           "CSD_DL_SendData :: DICC_GetChannelCurrDataCnt is Wrong",
                           ulDICCNodeCnt);
            break;
        }

        if (0 != ulDICCNodeCnt )
        {
           /*从DICC通道中移出数据*/
            ulRslt      = DICC_RemoveChannelData(ACPU_PID_CSD,
                                                 DICC_CHAN_ID_DL_CSD_DATA_CHAN,
                                                 (VOS_UINT8 *)(&stDLData),
                                                 DICC_CPU_ID_ACPU);

            if (DICC_OK != ulRslt)
            {
                CSD_ERROR_LOG1(ACPU_PID_CSD,
                              "CSD_DL_SendData:: enMsgId Illage",
                              ulRslt);
                break;
            }

            if (VOS_NULL_PTR == stDLData.pGarbage)
            {
                CSD_ERROR_LOG(ACPU_PID_CSD,
                              "CSD_DL_SendData:: stDLData.pGarbage Is Null");
                break;
            }

            CSD_DBG_DL_RECV_PKT_NUM(1);

            /*申请sk_buffer内存*/
            pstCsdDLMem = IMM_ZcStaticAlloc(stDLData.usLen);

            if (VOS_NULL_PTR == pstCsdDLMem)
            {
                CSD_ERROR_LOG1(ACPU_PID_CSD,
                              "CSD_DL_SendData:: pstCsdDLMem Is Null IMM_ZcStaticAlloc Fail",
                              pstCsdDLMem);

                /*通知cCpu释放TTFmem*/
                IMM_RemoteFreeTtfMem(stDLData.pGarbage);

                /*此处用continue可能导致死循环*/
                break;

            }

            /*此步骤不能少用来偏移数据尾指针*/
            ImmZcData = (VOS_CHAR *)IMM_ZcPut(pstCsdDLMem, stDLData.usLen);

            TAF_MEM_CPY_S(ImmZcData, stDLData.usLen, (VOS_UINT8 *)TTF_PHY_TO_VIRT((VOS_VOID *)(stDLData.pucData)), stDLData.usLen);

            /*发送数据到驱动,第一个参数为pppid目前不使用，由于失败AT会释放内存，
            所以此处不需要另行释放a核内存*/
            ulRslt      = AT_SendCsdZcDataToModem(CSD_UL_GetAtClientIndex(), pstCsdDLMem);

            if (VOS_OK != ulRslt)
            {
                CSD_DBG_DL_SEND_FAIL_NUM(1);

                CSD_ERROR_LOG(ACPU_PID_CSD,
                              "CSD_DL_SendData:: AT_SendZcDataToModem  Fail");

                /*通知cCpu释放TTFmem*/
                IMM_RemoteFreeTtfMem(stDLData.pGarbage);

                /*此处用continue可能导致死循环*/
                break;
            }
            CSD_DBG_DL_SEND_PKT_NUM(1);

            /*通知cCpu释放TTFmem*/
            IMM_RemoteFreeTtfMem(stDLData.pGarbage);

        }
        else
        {

            /*通道内数据发送完*/
            CSD_NORMAL_LOG1(ACPU_PID_CSD,
                            "CSD_DL_SendData Queue is Null",
                            ulDICCNodeCnt);

            break;
        }

        CSD_NORMAL_LOG1(ACPU_PID_CSD,
                        "CSD_DL_SendData Done",
                        ulDICCNodeCnt);

    }

}

VOS_VOID CSD_DL_ClearData(VOS_VOID)
{
    VOS_UINT32                          ulDICCNodeCnt;
    VOS_UINT32                          ulRslt;
    CST_CSD_DATA_IND_STRU               stDLData;


    /*获取DICC通道发送来数据的个数*/
    ulDICCNodeCnt   = DICC_GetChannelCurrDataCnt(ACPU_PID_CSD,
                                                 DICC_CHAN_ID_DL_CSD_DATA_CHAN,
                                                 DICC_CPU_ID_ACPU);
    if ( DICC_INVALID_VALUE == ulDICCNodeCnt )
    {
        CSD_ERROR_LOG1(ACPU_PID_CSD,
                       "CSD_DL_ClearData :: DICC_GetChannelCurrDataCnt is Wrong",
                       ulDICCNodeCnt);
        return;
    }

    while ((0 != ulDICCNodeCnt))
    {

        /*从DICC通道中移出数据*/
        ulRslt      = DICC_RemoveChannelData(ACPU_PID_CSD,
                                             DICC_CHAN_ID_DL_CSD_DATA_CHAN,
                                             (VOS_UINT8 *)(&stDLData),
                                             DICC_CPU_ID_ACPU);

        if (DICC_OK != ulRslt)
        {
            CSD_ERROR_LOG1(ACPU_PID_CSD,
                          "CSD_DL_ClearData:: DICC_RemoveChannelData Fail",
                          ulRslt);
            break;
        }

        ulDICCNodeCnt-- ;

        /*通知cCpu释放TTFmem*/
        IMM_RemoteFreeTtfMem(stDLData.pGarbage);
    }

    CSD_NORMAL_LOG(ACPU_PID_CSD,"CSD_DL_ClearData Done");
}

VOS_VOID CSD_DL_ProcDataTask(VOS_VOID)
{
    VOS_SEM                             hDownLinkSem;

    hDownLinkSem = CSD_GetDownLinkDataSem();

    for ( ; ; )
    {

        /* 获取下行取数据信号量 */
        if (VOS_OK != VOS_SmP(hDownLinkSem, 0 ))
        {
            CSD_NORMAL_LOG(ACPU_PID_CSD,
                "CSD_DL_ProcDataTask:: VOS_SmP pulDownLinkSem then continue !");


            continue;
        }

        /*获取当前是否挂断电话*/
        if (AT_CSD_CALL_STATE_ON != CSD_GetCallState())
        {
            /*释放数据*/
            CSD_DL_ClearData();

            CSD_ERROR_LOG(ACPU_PID_CSD,
                          "CSD_DL_ProcDataTask:: AT_CSD_CALL_STATE_OFF Receive ISR");


            continue;

        }

        CSD_DL_SendData();
    }
}

#endif /*FEATURE_CSD*/






