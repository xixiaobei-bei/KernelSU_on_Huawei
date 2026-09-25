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
#include "TafCcmApi.h"
#include "MnCommApi.h"
#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#include "NasMultiInstanceApi.h"
#endif
#include "securec.h"


#define    THIS_FILE_ID        PS_FILE_ID_TAF_CCM_API_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#if (OSA_CPU_ACPU == VOS_OSA_CPU)
    extern VOS_UINT32 AT_GetDestPid(
        MN_CLIENT_ID_T                      usClientId,
        VOS_UINT32                          ulRcvPid
    );
#endif

/*****************************************************************************
  3 类型定义
*****************************************************************************/

/*****************************************************************************
  4 函数声明
*****************************************************************************/

/*****************************************************************************
  5 变量定义
*****************************************************************************/


/*****************************************************************************
  6 函数定义
*****************************************************************************/


MODULE_EXPORTED VOS_UINT32  TAF_CCM_CallCommonReq(
    TAF_CTRL_STRU                      *pstCtrl,
    const VOS_VOID                     *pPara,
    VOS_UINT32                          ulMsgType,
    VOS_UINT32                          ulParaLen,
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    TAF_CCM_APP_REQ_MSG_STRU           *pstTafCcmAppMsg = VOS_NULL_PTR;
    VOS_UINT8                          *pucMsg          = VOS_NULL_PTR;
    VOS_UINT32                          ulMsgLen;
    VOS_UINT32                          ulReceiverPid;
    VOS_UINT32                          ulSenderPid;
    errno_t                             lMemResult;
#if (OSA_CPU_ACPU == VOS_OSA_CPU)
    ulSenderPid   = AT_GetDestPid(pstCtrl->usClientId, WUEPS_PID_TAF);
    ulReceiverPid = AT_GetDestPid(pstCtrl->usClientId, UEPS_PID_CCM);
#else
    ulSenderPid   = NAS_MULTIINSTANCE_GetSpecModemPid(enModemId, WUEPS_PID_TAF);
    ulReceiverPid = NAS_MULTIINSTANCE_GetSpecModemPid(enModemId, UEPS_PID_CCM);
#endif

    ulMsgLen = 0;

    if ((pPara == VOS_NULL_PTR)
        && (ulParaLen != 0))
    {
        return VOS_ERR;
    }

    /* 获取消息长度 */
    ulMsgLen = ulParaLen + sizeof(TAF_CCM_APP_REQ_MSG_STRU);

    pucMsg = (VOS_UINT8 *)PS_ALLOC_MSG_WITH_HEADER_LEN(ulSenderPid, ulMsgLen);

    if (pucMsg == VOS_NULL_PTR)
    {
        return VOS_ERR;
    }

    pstTafCcmAppMsg                     = (TAF_CCM_APP_REQ_MSG_STRU *)pucMsg;

    TAF_CfgMsgHdr((MsgBlock*)pstTafCcmAppMsg, ulSenderPid, ulReceiverPid, (ulMsgLen - VOS_MSG_HEAD_LENGTH));

    pstTafCcmAppMsg->ulMsgName          = ulMsgType;

    /* 填充消息头 */
    (VOS_VOID)memcpy_s(&(pstTafCcmAppMsg->stCtrl),
                       sizeof(pstTafCcmAppMsg->stCtrl),
                       pstCtrl,
                       sizeof(TAF_CTRL_STRU));

    /* 支持不带任何参数的消息结构 */
    if ((pPara != VOS_NULL_PTR)
     && (ulParaLen > 0))
    {
        lMemResult = memcpy_s(pstTafCcmAppMsg->aucContent,
                              ulParaLen,
                              (VOS_UINT8 *)pPara,
                              ulParaLen);
        TAF_MEM_CHK_RTN_VAL(lMemResult, ulParaLen, ulParaLen);
    }

    if (PS_SEND_MSG(ulSenderPid, pucMsg) != VOS_OK)
    {
        return VOS_ERR;
    }

    return VOS_OK;
}





