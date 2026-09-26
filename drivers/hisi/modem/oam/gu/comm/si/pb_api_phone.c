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

/*
 * 版权所有（c）华为技术有限公司 2017-2019
 * 功能描述: 该C文件给出了---接口模块实现
 * 生成日期: 2017-8-28
 */
#include "taf_type_def.h"
#include "si_pb.h"
#include "product_config.h"

#if ((OSA_CPU_ACPU == VOS_OSA_CPU) || (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON))
#include "si_pih.h"
#endif

#if (VOS_OS_VER == VOS_WIN32)
#include "ut_mem.h"
#endif


#define THIS_FILE_ID PS_FILE_ID_PBAPI_C
#define THIS_MODU mod_pam_pb

#if (FEATURE_PHONE_USIM == FEATURE_ON)

#if ((OSA_CPU_ACPU == VOS_OSA_CPU) || (defined(DMT)) || (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON))
/*
 * 功能描述: FDN激活
 * 修改历史:
 * 1. 日    期: 2008年10月18日
 *    修改内容: Create
 */
VOS_UINT32 SI_PB_GetReceiverPid(MN_CLIENT_ID_T clientId, VOS_UINT32 *receiverPid)
{
#if (MULTI_MODEM_NUMBER > 1)
    ModemIdUint16 modemId;
    SI_PIH_CardSlotUint32 slotId;

    /* 调用接口获取Modem ID */
    if (AT_GetModemIdFromClient(clientId, &modemId) != VOS_OK) {
        return VOS_ERR;
    }

    slotId = SI_GetSlotIdByModemId(modemId);
    if (slotId >= SI_PIH_CARD_SLOT_BUTT) {
        return VOS_ERR;
    }

    if (slotId == SI_PIH_CARD_SLOT_0) {
        *receiverPid = I0_MAPS_PB_PID;
    } else if (slotId == SI_PIH_CARD_SLOT_1) {
        *receiverPid = I1_MAPS_PB_PID;
    } else {
        *receiverPid = I2_MAPS_PB_PID;
    }
#else
    *receiverPid = MAPS_PB_PID;
#endif

    return VOS_OK;
}

/*
 * 功能描述: 读取电话本记录
 * 修改历史:
 * 1. 日    期: 2008年10月14日
 *    修改内容: Create
 */
MODULE_EXPORTED SI_UINT32 SI_PB_Read(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, SI_PB_StorateTypeUint32 storage, SI_UINT16 index1,
    SI_UINT16 index2)
{
    return TAF_FAILURE;
}

/*
 * 功能描述: 读取复合电话本记录
 * 修改历史:
 * 1. 日    期: 2009年06月05日
 *    修改内容: Create
 */
MODULE_EXPORTED SI_UINT32 SI_PB_SRead(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, SI_PB_StorateTypeUint32 storage,
    SI_UINT16 index1, SI_UINT16 index2)
{
    return TAF_FAILURE;
}

/*
 * 功能描述: 电话本号码查询
 * 修改历史:
 * 1. 日    期: 2008年10月14日
 *    修改内容: Create
 */
MODULE_EXPORTED SI_UINT32 SI_PB_Query(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    return TAF_FAILURE;
}

/*
 * 功能描述: 设置当前号码本使用的存储器类型
 * 修改历史:
 * 1. 日    期: 2008年10月14日
 *    修改内容: Create
 */
MODULE_EXPORTED SI_UINT32 SI_PB_Set(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, SI_PB_StorateTypeUint32 storage)
{
    SI_PB_SetReq *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (storage != SI_PB_STORAGE_FD) {
        PB_ERROR_LOG("SI_PB_Set:Double Modem only support the FDN");

        return TAF_FAILURE;
    }

    if (SI_PB_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PB_ERROR_LOG("SI_PB_Set:Get ulReceiverPid Error.");

        return TAF_FAILURE;
    }

    msg = (SI_PB_SetReq *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_SetReq) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PB_ERROR_LOG("SI_PB_Set:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(msg, receiverPid);
    msg->msgName = SI_PB_SET_REQ;
    msg->client = clientId;
    msg->opId = opId;
    msg->storage = storage;

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PB_ERROR_LOG("SI_PB_Set:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: 电话本中追加一条记录
 * 修改历史:
 * 1. 日    期: 2008年10月14日
 *    修改内容: Create
 */
MODULE_EXPORTED SI_UINT32 SI_PB_Add(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, SI_PB_StorateTypeUint32 storage,
    SI_PB_Record *record)
{
    SI_PB_AddRep *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PB_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PB_ERROR_LOG("SI_PB_Add:Get ulReceiverPid Error.");

        return TAF_FAILURE;
    }

    if (record == VOS_NULL_PTR) {
        PB_ERROR_LOG("SI_PB_Add:pRecord is a NULL pointer");

        return TAF_FAILURE;
    }

    /*lint -e433 */
    msg = (SI_PB_AddRep *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_AddRep) - VOS_MSG_HEAD_LENGTH);
    /*lint +e433 */
    if (msg == VOS_NULL_PTR) {
        PB_ERROR_LOG("SI_PB_Add:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(msg, receiverPid);
    msg->msgName = SI_PB_ADD_REQ;
    msg->client = clientId;
    msg->opId = opId;

    msg->storage = SI_PB_STORAGE_FD; /* 只能够操作FDN号码 */

    record->index = 1;

    (VOS_VOID)memcpy_s(&msg->record, sizeof(SI_PB_Record), record, sizeof(SI_PB_Record));

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PB_ERROR_LOG("SI_PB_Add:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}
/*
 * 功能描述: 电话本中追加一条记录
 * 修改历史:
 * 1. 日    期: 2009年06月05日
 *    修改内容: Create
 */
MODULE_EXPORTED SI_UINT32 SI_PB_SAdd(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, SI_PB_StorateTypeUint32 storage,
    SI_PB_Record *record)
{
    return TAF_FAILURE;
}

/*
 * 功能描述: 电话本中追加一条记录
 * 修改历史:
 * 1. 日    期: 2008年10月14日
 *    修改内容: Create
 */
MODULE_EXPORTED SI_UINT32 SI_PB_Modify(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, SI_PB_StorateTypeUint32 storage,
    SI_PB_Record *record)
{
    SI_PB_ModifyRep *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PB_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PB_ERROR_LOG("SI_PB_Modify:Get ulReceiverPid Error.");

        return TAF_FAILURE;
    }

    if (record == VOS_NULL_PTR) {
        PB_ERROR_LOG("SI_PB_Modify:pRecord is a NULL pointer");

        return TAF_FAILURE;
    }

    /*lint -e433 */
    msg = (SI_PB_ModifyRep *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_ModifyRep) - VOS_MSG_HEAD_LENGTH);
    /*lint +e433 */
    if (msg == VOS_NULL_PTR) {
        PB_ERROR_LOG("SI_PB_Modify:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(msg, receiverPid);
    msg->msgName = SI_PB_MODIFY_REQ;
    msg->client = clientId;
    msg->opId = opId;

    msg->storage = SI_PB_STORAGE_FD; /* 只能够操作FDN号码 */

    (VOS_VOID)memcpy_s(&msg->record, sizeof(SI_PB_Record), record, sizeof(SI_PB_Record));

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PB_ERROR_LOG("SI_PB_Modify:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: 电话本中追加一条记录
 * 修改历史:
 * 1. 日    期: 2009年06月05日
 *    修改内容: Create
 */
MODULE_EXPORTED SI_UINT32 SI_PB_SModify(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, SI_PB_StorateTypeUint32 storage,
    SI_PB_Record *record)
{
    return TAF_FAILURE;
}

/*
 * 功能描述: 电话本中删除一条记录
 * 修改历史:
 * 1. 日    期: 2008年10月14日
 *    修改内容: Create
 */
MODULE_EXPORTED SI_UINT32 SI_PB_Delete(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, SI_PB_StorateTypeUint32 storage,
    SI_UINT16 pbIndex)
{
    SI_PB_DeleteReq *msg = VOS_NULL_PTR;
    VOS_UINT32 receiverPid;

    if (SI_PB_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        PB_ERROR_LOG("SI_PB_Delete:Get ulReceiverPid Error.");

        return TAF_FAILURE;
    }

    msg = (SI_PB_DeleteReq *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_DeleteReq) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        PB_ERROR_LOG("SI_PB_Delete:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    VOS_SET_RECEIVER_ID(msg, receiverPid);
    msg->msgName = SI_PB_DELETE_REQ;
    msg->client = clientId;
    msg->opId = opId;
    msg->index = pbIndex;

    msg->storage = SI_PB_STORAGE_FD; /* 只能够操作FDN号码 */

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_SendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        PB_ERROR_LOG("SI_PB_Delete:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*
 * 功能描述: 电话本中查找一个记录
 * 修改历史:
 * 1. 日    期: 2009年3月12日
 *    修改内容: Create
 */
MODULE_EXPORTED SI_UINT32 SI_PB_Search(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, SI_PB_StorateTypeUint32 storage,
    SI_UINT8 length, SI_UINT8 *content)
{
    return TAF_FAILURE;
}

/*
 * 功能描述: 获取电话本当前存储介质
 * 修改历史:
 * 1. 日    期: 2011年05月17日
 *    修改内容: Creat，at+cpbf混合查找功能
 */
MODULE_EXPORTED VOS_UINT32 SI_PB_GetStorateType(VOS_VOID)
{
    return SI_PB_STORAGE_UNSPECIFIED; /* 返回当前未指定 */
}

/*
 * 功能描述: 获取复合电话本标志，由此可判断出当前是否支持复合电话本
 * 修改历史:
 * 1. 日    期: 2011年05月17日
 *    修改内容: Creat，at+cpbf混合查找功能
 */
MODULE_EXPORTED VOS_UINT32 SI_PB_GetSPBFlag(VOS_VOID)
{
    return VOS_FALSE; /* 返回状态关闭 */
}

#endif

#endif /* (FEATURE_PHONE_USIM == FEATURE_ON) */

