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




#include "TafTypeDef.h"
#include "si_pb.h"
#include "product_config.h"
#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#include "UsimmApi.h"
#endif

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
#include "si_pih.h"
#endif

/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define      THIS_FILE_ID     PS_FILE_ID_PBAPI_C

#if (FEATURE_ON == FEATURE_PHONE_SC)

#if ((OSA_CPU_ACPU == VOS_OSA_CPU) || (defined(DMT)))
#if (FEATURE_ON == FEATURE_SCI_SWITCH_OPTIMIZE)

VOS_UINT32 SI_PB_GetReceiverPid(MN_CLIENT_ID_T  ClientId, VOS_UINT32 *pulReceiverPid)
{
#if (MULTI_MODEM_NUMBER > 1)
    MODEM_ID_ENUM_UINT16                enModemID;
    SI_PIH_CARD_SLOT_ENUM_UINT32        enSlotId;

    /* 调用接口获取Modem ID */
    if(VOS_OK != AT_GetModemIdFromClient(ClientId,&enModemID))
    {
        return VOS_ERR;
    }

    enSlotId = SI_GetSlotIdByModemId(enModemID);

    if (SI_PIH_CARD_SLOT_BUTT <= enSlotId)
    {
        return VOS_ERR;
    }

    if(SI_PIH_CARD_SLOT_0 == enSlotId)
    {
        *pulReceiverPid = I0_MAPS_PB_PID;
    }
    else if (SI_PIH_CARD_SLOT_1 == enSlotId)
    {
        *pulReceiverPid = I1_MAPS_PB_PID;
    }
    else
    {
        *pulReceiverPid = I2_MAPS_PB_PID;
    }
#else
    *pulReceiverPid = I0_MAPS_PIH_PID;
#endif

    return VOS_OK;
}

#else

VOS_UINT32 SI_PB_GetReceiverPid(MN_CLIENT_ID_T  ClientId, VOS_UINT32 *pulReceiverPid)
{
#if ( MULTI_MODEM_NUMBER > 1 )
    MODEM_ID_ENUM_UINT16    enModemID;

    /* 调用接口获取Modem ID */
    if(VOS_OK != AT_GetModemIdFromClient(ClientId,&enModemID))
    {
        return VOS_ERR;
    }

    if(MODEM_ID_1 == enModemID)
    {
        *pulReceiverPid = I1_MAPS_PB_PID;
    }
#if (MULTI_MODEM_NUMBER == 3)
    else if (MODEM_ID_2 == enModemID)
    {
        *pulReceiverPid = I2_MAPS_PB_PID;
    }
#endif /* MULTI_MODEM_NUMBER == 3 */
    else
    {
        *pulReceiverPid = I0_MAPS_PB_PID;
    }
#else
    *pulReceiverPid = MAPS_PB_PID;
#endif

    return VOS_OK;
}
#endif


SI_UINT32 SI_PB_Read(  MN_CLIENT_ID_T           ClientId,
                            MN_OPERATION_ID_T        OpId,
                            SI_PB_STORATE_TYPE       Storage,
                            SI_UINT16                Index1,
                            SI_UINT16                Index2)
{
    return TAF_FAILURE;
}


SI_UINT32 SI_PB_SRead(  MN_CLIENT_ID_T           ClientId,
                            MN_OPERATION_ID_T        OpId,
                            SI_PB_STORATE_TYPE       Storage,
                            SI_UINT16                Index1,
                            SI_UINT16                Index2)
{
    return TAF_FAILURE;
}


SI_UINT32 SI_PB_Query(     MN_CLIENT_ID_T           ClientId,
                                MN_OPERATION_ID_T        OpId)
{
    return TAF_FAILURE;
}


SI_UINT32 SI_PB_Set(    MN_CLIENT_ID_T           ClientId,
                            MN_OPERATION_ID_T        OpId,
                            SI_PB_STORATE_TYPE      Storage)
{
    SI_PB_SET_REQ_STRU  *pMsg;
    VOS_UINT32          ulReceiverPid;

    if(SI_PB_STORAGE_FD != Storage)
    {
        PB_ERROR_LOG("SI_PB_Set:Double Modem only support the FDN");

        return TAF_FAILURE;
    }

    if (VOS_OK != SI_PB_GetReceiverPid(ClientId, &ulReceiverPid))
    {
        PB_ERROR_LOG("SI_PB_Set:Get ulReceiverPid Error.");

        return TAF_FAILURE;
    }

    pMsg = (SI_PB_SET_REQ_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_SET_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Set:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    pMsg->ulReceiverPid = ulReceiverPid;
    pMsg->ulMsgName     = SI_PB_SET_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;
    pMsg->ulStorage     = Storage;

    if(VOS_OK != VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Set:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}


SI_UINT32 SI_PB_Add(    MN_CLIENT_ID_T          ClientId,
                            MN_OPERATION_ID_T        OpId,
                             SI_PB_STORATE_TYPE      Storage,
                             SI_PB_RECORD_STRU       *pRecord)
{
    SI_PB_ADD_REP_STRU  *pMsg;
    VOS_UINT32          ulReceiverPid;

    if (VOS_OK != SI_PB_GetReceiverPid(ClientId, &ulReceiverPid))
    {
        PB_ERROR_LOG("SI_PB_Add:Get ulReceiverPid Error.");

        return TAF_FAILURE;
    }

    if(VOS_NULL_PTR == pRecord)
    {
        PB_ERROR_LOG("SI_PB_Add:pRecord is a NULL pointer");

        return TAF_FAILURE;
    }

    pMsg = (SI_PB_ADD_REP_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_ADD_REP_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Add:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    pMsg->ulReceiverPid = ulReceiverPid;
    pMsg->ulMsgName     = SI_PB_ADD_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;

    pMsg->ulStorage = SI_PB_STORAGE_FD;     /*只能够操作FDN号码*/

    pRecord->Index = 1;

    PAM_MEM_CPY_S(&pMsg->stRecord, sizeof(SI_PB_RECORD_STRU), pRecord, sizeof(SI_PB_RECORD_STRU));

    if(VOS_OK !=  VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Add:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

SI_UINT32 SI_PB_SAdd(    MN_CLIENT_ID_T          ClientId,
                            MN_OPERATION_ID_T        OpId,
                             SI_PB_STORATE_TYPE      Storage,
                             SI_PB_RECORD_STRU       *pRecord)
{
    return TAF_FAILURE;
}


SI_UINT32 SI_PB_Modify(    MN_CLIENT_ID_T          ClientId,
                                MN_OPERATION_ID_T       OpId,
                                SI_PB_STORATE_TYPE      Storage,
                                SI_PB_RECORD_STRU       *pRecord )
{
    SI_PB_MODIFY_REP_STRU  *pMsg;
    VOS_UINT32              ulReceiverPid;

    if (VOS_OK != SI_PB_GetReceiverPid(ClientId, &ulReceiverPid))
    {
        PB_ERROR_LOG("SI_PB_Modify:Get ulReceiverPid Error.");

        return TAF_FAILURE;
    }

    if(VOS_NULL_PTR == pRecord)
    {
        PB_ERROR_LOG("SI_PB_Modify:pRecord is a NULL pointer");

        return TAF_FAILURE;
    }

    pMsg = (SI_PB_MODIFY_REP_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_MODIFY_REP_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Modify:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    pMsg->ulReceiverPid = ulReceiverPid;
    pMsg->ulMsgName     = SI_PB_MODIFY_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;

    pMsg->ulStorage = SI_PB_STORAGE_FD;     /*只能够操作FDN号码*/

    PAM_MEM_CPY_S(&pMsg->Record, sizeof(SI_PB_RECORD_STRU), pRecord, sizeof(SI_PB_RECORD_STRU));

    if(VOS_OK !=  VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Modify:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}


SI_UINT32 SI_PB_SModify(    MN_CLIENT_ID_T          ClientId,
                                MN_OPERATION_ID_T       OpId,
                                SI_PB_STORATE_TYPE      Storage,
                                SI_PB_RECORD_STRU       *pRecord )
{
    return TAF_FAILURE;
}


SI_UINT32 SI_PB_Delete(     MN_CLIENT_ID_T             ClientId,
                                MN_OPERATION_ID_T           OpId,
                                SI_PB_STORATE_TYPE          Storage,
                                SI_UINT16                   Index)
{
    SI_PB_DELETE_REQ_STRU  *pMsg;
    VOS_UINT32              ulReceiverPid;

    if (VOS_OK != SI_PB_GetReceiverPid(ClientId, &ulReceiverPid))
    {
        PB_ERROR_LOG("SI_PB_Delete:Get ulReceiverPid Error.");

        return TAF_FAILURE;
    }

    pMsg = (SI_PB_DELETE_REQ_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_DELETE_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Delete:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    pMsg->ulReceiverPid = ulReceiverPid;
    pMsg->ulMsgName     = SI_PB_DELETE_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;
    pMsg->usIndex       = Index;

    pMsg->ulStorage = SI_PB_STORAGE_FD;     /*只能够操作FDN号码*/

    if(VOS_OK !=  VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Delete:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}


SI_UINT32 SI_PB_Search(    MN_CLIENT_ID_T             ClientId,
                                MN_OPERATION_ID_T           OpId,
                                SI_PB_STORATE_TYPE          Storage,
                                SI_UINT8                    ucLength,
                                SI_UINT8                    *pucContent)
{
    return TAF_FAILURE;
}


VOS_UINT32 SI_PB_GetStorateType(VOS_VOID)
{
    return SI_PB_STORAGE_UNSPECIFIED;   /*返回当前未指定*/
}


VOS_UINT32 SI_PB_GetSPBFlag(VOS_VOID)
{
    return VOS_FALSE;   /*返回状态关闭*/
}

#endif

#if (OSA_CPU_ACPU == VOS_OSA_CPU)

VOS_VOID SI_PB_InitContent(VOS_UINT8 ucLen, VOS_UINT8 ucValue,VOS_UINT8 *pucMem)
{
    VOS_UINT8 i;

    for(i=0; i<ucLen; i++)
    {
        pucMem[i] = ucValue;
    }

    return;
}
#endif

#if ((OSA_CPU_CCPU == VOS_OSA_CPU)||(defined(DMT)))

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)

VOS_UINT32 SI_PB_GetXeccNumber(
    SI_PIH_CARD_SLOT_ENUM_UINT32        enSlotId,
    SI_PB_ECC_DATA_STRU                *pstEccData
)
{
    VOS_UINT32                          ulResult;
    VOS_UINT32                          ulNum;
    VOS_UINT8                          *pucTemp;
    VOS_UINT8                           ucPBOffset=0;
    VOS_UINT32                          i;
    VOS_UINT32                          j;
    VOS_UINT32                          copyLen;

    /* 输入参数检测 */
    if (VOS_NULL_PTR == pstEccData)
    {
        PB_ERROR_LOG("SI_PB_GetXeccNumber Error: Para is incorrect.");

        return VOS_ERR;
    }

    PAM_MEM_SET_S(pstEccData, sizeof(SI_PB_ECC_DATA_STRU), 0, sizeof(SI_PB_ECC_DATA_STRU));

    ulResult = SI_PB_LocateRecord(enSlotId, PB_XECC, 1, 1, &ucPBOffset);

    /* 当前电话本不存在或者初始化未完成 */
    if (VOS_OK != ulResult)
    {
        PB_ERROR_LOG("SI_PB_GetXeccNumber Error: SI_PB_LocateRecord Return Failed");

        pstEccData->bEccExists = SI_PB_CONTENT_INVALID;
        pstEccData->ulReocrdNum= VOS_NULL;

        return VOS_ERR;
    }

    pstEccData->bEccExists = SI_PB_CONTENT_VALID;

    if (VOS_NULL_PTR == gastPBContent[enSlotId][ucPBOffset].pContent)
    {
        PB_ERROR_LOG("SI_PB_GetXeccNumber Error: pContent is NULL");

        return VOS_ERR;
    }

    ulNum = ((gastPBContent[enSlotId][ucPBOffset].usTotalNum>USIM_MAX_ECC_RECORDS)?USIM_MAX_ECC_RECORDS:gastPBContent[enSlotId][ucPBOffset].usTotalNum);

    pucTemp = gastPBContent[enSlotId][ucPBOffset].pContent;

    copyLen = PAM_GetMin(gastPBContent[enSlotId][ucPBOffset].ucNumberLen, USIM_ECC_LEN);

    for (i = 0, j = 0; i < ulNum; i++)   /* 根据数据结构最大长度循环 */
    {
        ulResult = SI_PB_CheckEccValidity(enSlotId, pucTemp);

        if (VOS_ERR == ulResult)     /* 当前记录内容无效 */
        {
            PB_INFO_LOG("SI_PB_GetXeccNumber Info: The Ecc Number is Empty");
        }
        else                                /* 转换当前记录内容 */
        {
            PB_INFO_LOG("SI_PB_GetXeccNumber Info: The Ecc Number is Not Empty");

            PAM_MEM_CPY_S(pstEccData->astEccRecord[j].aucEccCode, sizeof(pstEccData->astEccRecord[j].aucEccCode), pucTemp, copyLen);

            j++;
        }

        pucTemp += gastPBContent[enSlotId][ucPBOffset].ucRecordLen;
    }

    pstEccData->ulReocrdNum = j;

    return VOS_OK;
}
#endif

#endif

#endif /* (FEATURE_ON == FEATURE_PHONE_SC) */




