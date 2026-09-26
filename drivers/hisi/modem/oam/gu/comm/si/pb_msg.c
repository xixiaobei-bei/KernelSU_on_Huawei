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
 * 版权所有（c）华为技术有限公司 2008-2019
 * 功能描述: 该C文件给出了---完成消息处理模块实现
 * 生成日期: 2008-5-15
 */

#include "si_pb.h"

#if (FEATURE_PHONE_USIM == FEATURE_OFF)
#include "om_private.h"
#if (VOS_OSA_CPU == OSA_CPU_CCPU)
#include "usimm_fdn.h"
#include "usimm_base_info.h"
#include "usimm_card_app_info.h"
#endif
#endif
#include "at_oam_interface.h"
#include "taf_oam_interface.h"

#if (VOS_OS_VER == VOS_WIN32)
#include "ut_mem.h"
#endif

#define THIS_FILE_ID PS_FILE_ID_PB_MSG_C
#define THIS_MODU mod_pam_pb

#if (FEATURE_PHONE_USIM == FEATURE_OFF)

SI_PB_ReqUint g_pbReqUnit;

/* 双核都需要的全局变量，A 上面的通过消息和 C 的同步 */
SI_PB_Control g_pbCtrlInfo;
SI_PB_NVCtrlInfo g_pbConfigInfo;

SI_PB_Content    g_pbContent[SI_PB_MAX_NUMBER];
SI_EXT_Content   g_extContent[SI_PB_MAX_NUMBER];
SI_ANR_Content   g_anrContent[SI_PB_ANRMAX];
SI_EML_Content   g_emlContent;
SI_IAP_Content   g_iapContent;
SI_PB_InitState g_pbInitState;
VOS_UINT8             g_pbCStatus = VOS_TRUE;

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
SI_PB_SearchCtrl g_pbSearchCtrlInfo;
VOS_UINT32             g_pbFileCnt;
VOS_UINT32             g_pbRecordCnt;
VOS_UINT32             g_pbInitFileNum;
VOS_UINT32             g_pbInitExtFileNum;
VOS_UINT32             g_pbExtRecord;
#endif

/* 此处不做修改，使用C核sim卡状态g_pbInitState.cardService */
#if (OSA_CPU_ACPU == VOS_OSA_CPU)
extern USIMM_CardAppServicUint32 g_acpuCardStatus;
#endif

VOS_UINT32 SI_PB_ErrorProc(VOS_VOID)
{
    USIMM_CardAppServicUint32  cardStatus = USIMM_CARDAPP_SERVIC_BUTT;

/* 此处不做修改，使用C核sim卡状态g_pbInitState.cardService */
#if (OSA_CPU_ACPU == VOS_OSA_CPU)
    cardStatus = (VOS_UINT8)g_acpuCardStatus;
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    cardStatus = g_pbInitState.cardService;
#endif

    if (PB_LOCKED == g_pbReqUnit.pbLock) {
        PB_WARNING_LOG("SI_PB_ErrorProc: The PB is Locked");

        return TAF_ERR_UNSPECIFIED_ERROR;
    } else if ((USIMM_CARDAPP_SERVIC_UNAVAILABLE == cardStatus) || (USIMM_CARDAPP_SERVIC_ABSENT == cardStatus)) {
        PB_WARNING_LOG("SI_PB_ErrorProc: The Card Absent");

        return TAF_ERR_SIM_FAIL;
    } else if (USIMM_CARDAPP_SERVIC_SIM_PUK == cardStatus) {
        PB_WARNING_LOG("SI_PB_ErrorProc: PUK NEED");

        return TAF_ERR_NEED_PUK1;
    } else if (USIMM_CARDAPP_SERVIC_SIM_PIN == cardStatus) {
        PB_WARNING_LOG("SI_PB_ErrorProc: PIN NEED");

        return TAF_ERR_NEED_PIN1;
    } else {
        return TAF_ERR_NO_ERROR;
    }
}

#if ((OSA_CPU_ACPU == VOS_OSA_CPU) || (defined(DMT)) || (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON))
/*
 * 功能描述: 回调处理函数
 * 修改历史:
 * 1. 日    期: 2008年10月20日
 *    修改内容: Create
 */
VOS_UINT32 SI_PBCallback(SI_PB_EventInfo *event)
{
    if (event->pbError != TAF_ERR_NO_ERROR) {
        g_pbReqUnit.pbLock = PB_UNLOCK; /* 解锁 */
    } else if ((event->pbEventType != SI_PB_EVENT_READ_CNF) && (event->pbEventType != SI_PB_EVENT_SREAD_CNF) &&
               (event->pbEventType != SI_PB_EVENT_SEARCH_CNF)) {
        g_pbReqUnit.pbLock = PB_UNLOCK; /* 解锁 */
    } else if (event->pbLastTag == VOS_TRUE) {
        g_pbReqUnit.pbLock = PB_UNLOCK; /* 解锁 */
    } else {
    }

    At_PbCallBackFunc(event);

    return VOS_OK;
}

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
/*
 * 功能描述: 处理读取Ecc号码请求
 * 修改历史:
 * 1. 日    期: 2007年10月15日
 *    修改内容: Creat
 */
VOS_VOID SI_PB_ReadXeccProc(VOS_UINT16 indexNum, VOS_UINT16 indexStar, SI_PB_EventInfo *cnfData)
{
    VOS_UINT32 result;
    VOS_UINT32 validFlag;
    VOS_UINT16 i;
    VOS_UINT8 *content = VOS_NULL_PTR;
    errno_t    ret;

    if (g_pbContent[PB_XECC_CONTENT].content == VOS_NULL_PTR) {
        PB_ERROR_LOG("SI_PB_ReadXeccProc Error: content is NULL");

        return;
    }

    content = g_pbContent[PB_XECC_CONTENT].content +
                 ((indexStar - 1) * g_pbContent[PB_XECC_CONTENT].recordLen);
    validFlag = VOS_FALSE;

    for (i = 0; i < indexNum; i++) {
        cnfData->pbEvent.pbReadCnf.pbRecord.index = i + indexStar;

        result = SI_PB_CheckEccValidity(content[0]);

        if (VOS_ERR == result) {
            cnfData->pbEvent.pbReadCnf.pbRecord.validFlag = SI_PB_CONTENT_INVALID;
        } else {
            cnfData->pbEvent.pbReadCnf.pbRecord.validFlag = SI_PB_CONTENT_VALID;
            validFlag                                      = VOS_TRUE;

            SI_PB_BcdToAscii(3, content, sizeof(cnfData->pbEvent.pbReadCnf.pbRecord.number),
                             cnfData->pbEvent.pbReadCnf.pbRecord.number,
                             &cnfData->pbEvent.pbReadCnf.pbRecord.numberLength); /* Ecc号码在姓名前面 */

            cnfData->pbEvent.pbReadCnf.pbRecord.numberType = PB_NUMBER_TYPE_NORMAL;

            if (g_pbContent[PB_XECC_CONTENT].nameLen != 0) {
                SI_PB_DecodePBName(g_pbContent[PB_XECC_CONTENT].nameLen, &content[3],
                                   &cnfData->pbEvent.pbReadCnf.pbRecord.alphaTagType,
                                   &cnfData->pbEvent.pbReadCnf.pbRecord.alphaTagLength);

                (VOS_VOID)memset_s(cnfData->pbEvent.pbReadCnf.pbRecord.alphaTag,
                                   sizeof(cnfData->pbEvent.pbReadCnf.pbRecord.alphaTag), 0xFF,
                                   sizeof(cnfData->pbEvent.pbReadCnf.pbRecord.alphaTag));

                cnfData->pbEvent.pbReadCnf.pbRecord.alphaTagLength =
                    PAM_GetMin(cnfData->pbEvent.pbReadCnf.pbRecord.alphaTagLength, SI_PB_ALPHATAG_MAX_LEN);

                ret = memcpy_s(cnfData->pbEvent.pbReadCnf.pbRecord.alphaTag, SI_PB_ALPHATAG_MAX_LEN, &content[3],
                               cnfData->pbEvent.pbReadCnf.pbRecord.alphaTagLength);
                PAM_PRINT_SECFUN_RESULT(ret);
            } else {
                cnfData->pbEvent.pbReadCnf.pbRecord.alphaTagLength = 0;
            }
        }

        if (i == (indexNum - 1)) {
            cnfData->pbLastTag = VOS_TRUE;
            cnfData->pbError   = ((VOS_TRUE == validFlag) ? TAF_ERR_NO_ERROR : TAF_ERR_PB_NOT_FOUND);
        }

        (VOS_VOID)SI_PBCallback(cnfData);

        content += g_pbContent[PB_XECC_CONTENT].recordLen; /* 内容指针按照记录长度偏移 */
    }

    return;
}
#endif

/*
 * 功能描述: 处理读取Ecc号码请求
 * 修改历史:
 * 1. 日    期: 2007年10月15日
 *    修改内容: Creat
 */
VOS_VOID SI_PB_ReadEccProc(VOS_UINT16 indexNum, VOS_UINT16 indexStar, SI_PB_EventInfo *cnfData)
{
    VOS_UINT32 result;
    VOS_UINT32 validFlag;
    VOS_UINT16 i;
    VOS_UINT8 *content = VOS_NULL_PTR;
    errno_t    ret;

    content = g_pbContent[PB_ECC_CONTENT].content +
                 ((indexStar - 1) * g_pbContent[PB_ECC_CONTENT].recordLen);
    validFlag = VOS_FALSE;

    for (i = 0; i < indexNum; i++) {
        cnfData->pbEvent.pbReadCnf.pbRecord.index = i + indexStar;

        result = SI_PB_CheckEccValidity(content[0]);

        if (VOS_ERR == result) {
            cnfData->pbEvent.pbReadCnf.pbRecord.validFlag = SI_PB_CONTENT_INVALID;
        } else {
            cnfData->pbEvent.pbReadCnf.pbRecord.validFlag = SI_PB_CONTENT_VALID;
            validFlag                                      = VOS_TRUE;

            SI_PB_BcdToAscii(3, content, sizeof(cnfData->pbEvent.pbReadCnf.pbRecord.number),
                             cnfData->pbEvent.pbReadCnf.pbRecord.number,
                             &cnfData->pbEvent.pbReadCnf.pbRecord.numberLength); /* Ecc号码在姓名前面 */

            cnfData->pbEvent.pbReadCnf.pbRecord.numberType = PB_NUMBER_TYPE_NORMAL;

            if (g_pbContent[PB_ECC_CONTENT].nameLen != 0) {
                SI_PB_DecodePBName(g_pbContent[PB_ECC_CONTENT].nameLen, &content[3],
                                   &cnfData->pbEvent.pbReadCnf.pbRecord.alphaTagType,
                                   &cnfData->pbEvent.pbReadCnf.pbRecord.alphaTagLength);

                (VOS_VOID)memset_s(cnfData->pbEvent.pbReadCnf.pbRecord.alphaTag,
                                   sizeof(cnfData->pbEvent.pbReadCnf.pbRecord.alphaTag), 0xFF,
                                   sizeof(cnfData->pbEvent.pbReadCnf.pbRecord.alphaTag));

                cnfData->pbEvent.pbReadCnf.pbRecord.alphaTagLength =
                    PAM_GetMin(cnfData->pbEvent.pbReadCnf.pbRecord.alphaTagLength, SI_PB_ALPHATAG_MAX_LEN);

                ret = memcpy_s(cnfData->pbEvent.pbReadCnf.pbRecord.alphaTag, SI_PB_ALPHATAG_MAX_LEN, &content[3],
                               cnfData->pbEvent.pbReadCnf.pbRecord.alphaTagLength);

                PAM_PRINT_SECFUN_RESULT(ret);
            } else {
                cnfData->pbEvent.pbReadCnf.pbRecord.alphaTagLength = 0;
            }
        }

        if (i == (indexNum - 1)) {
            cnfData->pbLastTag = VOS_TRUE;
            cnfData->pbError   = ((VOS_TRUE == validFlag) ? TAF_ERR_NO_ERROR : TAF_ERR_PB_NOT_FOUND);
        }

        (VOS_VOID)SI_PBCallback(cnfData);

        content += g_pbContent[PB_ECC_CONTENT].recordLen; /* 内容指针按照记录长度偏移 */
    }

    return;
}

/*
 * 功能描述: 处理读取电话本的请求
 * 修改历史:
 * 1. 日    期: 2007年10月15日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PB_ReadProc(PBMsgBlock *msg)
{
    SI_PB_ReadReq *reqMsg = VOS_NULL_PTR;
    SI_PB_EventInfo cnfData;
    VOS_UINT8             pBoffset;
    VOS_UINT32            result;
    VOS_UINT16            indexNum;
    VOS_UINT16            i;
    VOS_UINT8            *content  = VOS_NULL_PTR;
    VOS_UINT32            validFlag = VOS_FALSE;

    if (msg->ulLength < (sizeof(SI_PB_ReadReq) - VOS_MSG_HEAD_LENGTH)) {
        PB_ERROR1_LOG("SI_PB_ReadProc: incorrect msg length", msg->ulLength);
        PB_ERROR1_LOG("SI_PB_ReadProc: incorrect send id", msg->ulSenderPid);

        return VOS_ERR;
    }

    (VOS_VOID)memset_s((VOS_UINT8 *)&cnfData, sizeof(SI_PB_EventInfo), (VOS_CHAR)0,
                       sizeof(SI_PB_EventInfo));

    reqMsg = (SI_PB_ReadReq *)msg;

    cnfData.clientId = reqMsg->client;

    cnfData.opId = reqMsg->opId;

    cnfData.pbEventType = SI_PB_EVENT_READ_CNF;

    cnfData.storage = reqMsg->storage;

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    if ((PB_ECC == reqMsg->storage) || (PB_XECC == reqMsg->storage)) {
        cnfData.pbError = TAF_ERR_NO_ERROR;
    }
#else
    if (PB_ECC == reqMsg->storage) {
        cnfData.pbError = TAF_ERR_NO_ERROR;
    }
#endif
    else {
        cnfData.pbError = SI_PB_ErrorProc();
    }

    if (TAF_ERR_NO_ERROR != cnfData.pbError) {
        PB_WARNING_LOG("SI_PB_ReadProc: Proc Error");

        return SI_PBCallback(&cnfData);
    }

    g_pbReqUnit.pbLock = PB_LOCKED;

    result = SI_PB_LocateRecord(reqMsg->storage, reqMsg->index1, reqMsg->index2, &pBoffset);
    if (VOS_OK != result) { /* 当前电话本内容未找到 */
        PB_WARNING_LOG("SI_PB_ReadProc: SI_PB_LocateRecord Return Error");

        cnfData.pbError = result;

        (VOS_VOID)SI_PBCallback(&cnfData);

        return VOS_ERR;
    }

    if (0 == reqMsg->index1) {
        indexNum = g_pbContent[pBoffset].totalNum;

        reqMsg->index1 = 1; /* 从第一条开始读取 */

        reqMsg->index2 = g_pbContent[pBoffset].totalNum;
    } else {
        indexNum = (reqMsg->index2 - reqMsg->index1) + 1; /* 计算两个索引之间的记录数 */
    }

    cnfData.pbEvent.pbReadCnf.recordNum = 1;

    /* 紧急呼叫号码在华山和Balong上面都有缓冲 */

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    if ((PB_XECC == reqMsg->storage)) {
        SI_PB_ReadXeccProc(indexNum, reqMsg->index1, &cnfData);

        g_pbReqUnit.pbLock = PB_UNLOCK;

        return VOS_OK;
    }
#endif

    if (PB_ECC == reqMsg->storage) {
        SI_PB_ReadEccProc(indexNum, reqMsg->index1, &cnfData);

        g_pbReqUnit.pbLock = PB_UNLOCK;

        return VOS_OK;
    }

    content = g_pbContent[pBoffset].content + ((reqMsg->index1 - 1) * g_pbContent[pBoffset].recordLen);

    cnfData.pbLastTag = VOS_FALSE;

    for (i = 0; i < indexNum; i++) { /* 根据读取的电话本索引范围循环 */
        (VOS_VOID)memset_s((VOS_UINT8 *)&cnfData.pbEvent.pbReadCnf, sizeof(SI_PB_EventReadCnf), 0,
                           sizeof(SI_PB_EventReadCnf));

        SI_PB_TransPBFromate(&g_pbContent[pBoffset], (VOS_UINT16)(reqMsg->index1 + i), content,
                             &cnfData.pbEvent.pbReadCnf.pbRecord);

        content += g_pbContent[pBoffset].recordLen; /* 内容指针按照记录长度偏移 */

        /* 根据index判断SI_PB_CONTENT_VALID == cnfData.pbEvent.pbReadCnf.pbRecord.validFlag) */
        if (SI_PB_CONTENT_VALID == SI_PB_GetBitFromBuf(g_pbContent[pBoffset].index,
                                                       g_pbContent[pBoffset].indexSize, reqMsg->index1 + i)) {
            validFlag = VOS_TRUE;
        }

        if (i == (indexNum - 1)) {
            cnfData.pbLastTag = VOS_TRUE;
            cnfData.pbError   = ((VOS_TRUE == validFlag) ? TAF_ERR_NO_ERROR : TAF_ERR_PB_NOT_FOUND);
        }

        (VOS_VOID)SI_PBCallback(&cnfData); /* 返回转换结果 */
    }

    return VOS_OK;
}

/*
 * 功能描述: 电话本查找处理函数
 * 修改历史:
 * 1. 日    期: 2009年03月12日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PB_SearchHandle(const SI_PB_SearchReq *msg, SI_PB_EventInfo *cnfData, VOS_UINT8 offset)
{
    VOS_UINT16 matchRecordNum = 0;
    VOS_UINT16 i;

    cnfData->pbError   = TAF_ERR_NO_ERROR;
    cnfData->pbLastTag = VOS_FALSE;
    g_pbReqUnit.pbLock = PB_LOCKED;

    for (i = 0; i < g_pbContent[offset].totalNum; i++) {
        if (VOS_ERR ==
            SI_PB_CheckContentValidity(&g_pbContent[offset],
                                       g_pbContent[offset].content + (i * g_pbContent[offset].recordLen),
                                       g_pbContent[offset].recordLen)) {
            continue;
        } else if (g_pbContent[offset].recordLen >= msg->length) {
            if ((VOS_OK == VOS_MemCmp(msg->content,
                                      g_pbContent[offset].content + (i * g_pbContent[offset].recordLen),
                                      msg->length)) || (0 == msg->length)) {
                SI_PB_TransPBFromate(&g_pbContent[offset], (VOS_UINT16)(i + 1),
                                     g_pbContent[offset].content + (i * g_pbContent[offset].recordLen),
                                     &cnfData->pbEvent.pbSearchCnf.pbRecord);

                matchRecordNum++;
                (VOS_VOID)SI_PBCallback(cnfData);
            }
        } else {
            continue;
        }
    }

    if (0 == matchRecordNum) {
        return TAF_ERR_PB_NOT_FOUND;
    }

    cnfData->pbLastTag                              = VOS_TRUE;
    cnfData->pbEvent.pbSearchCnf.pbRecord.validFlag = SI_PB_CONTENT_INVALID;

    (VOS_VOID)SI_PBCallback(cnfData);

    return TAF_ERR_NO_ERROR;
}

/*
 * 功能描述: index方式电话本查找处理函数
 * 修改历史:
 * 1. 日    期: 2009年04月15日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PB_SearchReqFunc(const SI_PB_SearchReq *msg, SI_PB_EventInfo *cnfData, VOS_UINT8 offset)
{
    VOS_UINT8  sendReqFlag = PB_REQ_NOT_SEND;
    VOS_UINT8  record;
    VOS_UINT32 result;
    VOS_UINT16 fileId;
    errno_t    ret;

    if (msg->length <= sizeof(g_pbReqUnit.xdnContent)) {
        ret = memcpy_s(g_pbReqUnit.xdnContent, sizeof(g_pbReqUnit.xdnContent), msg->content,
                       msg->length);

        PAM_PRINT_SECFUN_RESULT(ret);
    } else {
        PB_WARNING_LOG("SI_PB_SearchReqFunc: Text string too long");

        return TAF_ERR_PARA_ERROR;
    }

    g_pbReqUnit.curIndex = 1;

    g_pbReqUnit.searchLen = msg->length;

    g_pbReqUnit.index2 = g_pbContent[offset].totalNum;

    g_pbReqUnit.pbEventType = SI_PB_EVENT_SEARCH_CNF;

    g_pbReqUnit.equalFlag = VOS_FALSE;

    g_pbReqUnit.pbStoateType = msg->storage;

    while (g_pbReqUnit.curIndex <= g_pbReqUnit.index2) {
        if (SI_PB_CONTENT_VALID == SI_PB_GetBitFromBuf(g_pbContent[offset].index,
                                                       g_pbContent[offset].indexSize, g_pbReqUnit.curIndex)) {
            sendReqFlag = PB_REQ_SEND;
            break;
        }

        g_pbReqUnit.curIndex++;
    }

    if (PB_REQ_SEND == sendReqFlag) {
        if (SI_PB_STORAGE_SM == msg->storage) { /* 如果是ADN需要转换为记录号 */
            result = SI_PB_CountADNRecordNum(g_pbReqUnit.curIndex, &fileId, &record);
        } else { /* 访问其它电话本 */
            result = SI_PB_GetXDNFileID(msg->storage, &fileId);

            record = (VOS_UINT8)g_pbReqUnit.curIndex;
        }

        if (VOS_ERR == result) { /* 转换结果失败 */
            PB_WARNING_LOG("SI_PB_SearchReqFunc: Get the XDN File ID and Record number is Error");

            cnfData->pbError = TAF_ERR_PARA_ERROR;

            return TAF_ERR_PARA_ERROR;
        }
    } else {
        cnfData->pbLastTag = VOS_TRUE;

        PB_WARNING_LOG("SI_PB_SearchReqFunc: Content not found!");

        return TAF_ERR_PB_NOT_FOUND;
    }

    return TAF_ERR_NO_ERROR;
}

/*
 * 功能描述: 电话本查找处理函数
 * 修改历史:
 * 1. 日    期: 2009年03月12日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PB_SearchProc(PBMsgBlock *msg)
{
    SI_PB_EventInfo cnfData;
    VOS_UINT32            result;
    VOS_UINT8             offset = 0;

    if (msg->ulLength < (sizeof(SI_PB_SearchReq) - VOS_MSG_HEAD_LENGTH)) {
        PB_ERROR1_LOG("SI_PB_SearchProc: incorrect msg length", msg->ulLength);
        PB_ERROR1_LOG("SI_PB_SearchProc: incorrect send id", msg->ulSenderPid);

        return VOS_ERR;
    }
    cnfData.clientId    = ((SI_PB_SearchReq *)msg)->client;
    cnfData.opId        = ((SI_PB_SearchReq *)msg)->opId;
    cnfData.pbEventType = SI_PB_EVENT_SEARCH_CNF;
    cnfData.pbLastTag   = VOS_TRUE;

    if (TAF_ERR_NO_ERROR != (result = SI_PB_ErrorProc())) { /* 如果定位失败或者缓冲内容不存在 */
        cnfData.pbError   = result;
        cnfData.pbLastTag = VOS_TRUE;
        return SI_PBCallback(&cnfData); /* 调用回调函数 */
    }

    if (VOS_OK != SI_PB_FindPBOffset(((SI_PB_SearchReq *)msg)->storage, &offset)) {
        cnfData.pbError = TAF_ERR_UNSPECIFIED_ERROR;

        return SI_PBCallback(&cnfData); /* 返回转换结果 */
    }

    if (VOS_NULL_PTR == g_pbContent[offset].content) {
        if (TAF_ERR_NO_ERROR != (result = SI_PB_SearchReqFunc((const SI_PB_SearchReq *)msg, &cnfData, offset))) {
            cnfData.pbError = result;

            (VOS_VOID)SI_PBCallback(&cnfData); /* 返回转换结果 */
        }

        return VOS_OK;
    }

    if (TAF_ERR_NO_ERROR != (result = SI_PB_SearchHandle((const SI_PB_SearchReq *)msg, &cnfData, offset))) {
        cnfData.pbError = result;

        (VOS_VOID)SI_PBCallback(&cnfData); /* 返回转换结果 */
    }

    return VOS_OK;
}

/*
 * 功能描述: Email电话本读取
 * 修改历史:
 * 1. 日    期: 2009年06月05日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PB_SReadEMLProc(VOS_UINT16 index, VOS_UINT16 offset, SI_PB_Record *record)
{
    VOS_UINT32 eMLContentOffset;
    VOS_UINT8 *emlContent = VOS_NULL_PTR;
    VOS_UINT8  eMLRecordNum;
    VOS_UINT8  type2EMLFileCnt;
    VOS_UINT8  i;

    /*
     * email,区分Type1和2，先看是否有USED, 如果存在读内存
     * Type1 直接根据index读
     * Type2 根据index及IAP缓存转换出实际记录号，如果记录号无效则退出
     */
    if (0 == g_pbCtrlInfo.emlFileNum) {
        PB_WARNING_LOG("SI_PB_SReadEMLProc: No Valid email Record");
        return VOS_OK;
    }

    if (PB_FILE_TYPE1 == g_pbCtrlInfo.emlInfo[0].emlType) { /* Email文件类型， */
        /* 对于TYPE1类型的文件，转入的参数index + usOffset不能超过EMAIL的总记录数 */
        if ((index + offset) > g_emlContent.totalNum) {
            PB_WARNING_LOG("SI_PB_SReadEMLProc: email Record index Large than Total number.");
            return VOS_OK;
        }

        emlContent = g_emlContent.content + ((index + (offset - 1)) * g_emlContent.recordLen);
    } else {
        if (VOS_NULL_PTR == g_iapContent.iapContent) {
            PB_ERROR_LOG("SI_PB_SReadEMLProc: IAP storage NULL");
            return TAF_ERR_PB_STORAGE_OP_FAIL;
        }

        /* Type2 Email文件，需在gstIAPContent.pIAPContent中找到ADN 记录和EML记录对应关系 */
        /*lint -e679*/
        eMLRecordNum = g_iapContent.iapContent[(((index + offset) - 1) * g_iapContent.recordLen) +
                                                   (g_pbCtrlInfo.emlInfo[0].emlTagNum - 1)];
        /*lint +e679*/

        if (VOS_OK != SI_PB_GetFileCntFromIndex(index + offset, &type2EMLFileCnt)) {
            PB_WARNING_LOG("SI_PB_SReadEMLProc: SI_PB_GetFileCntFromIndex Return Error.");
            return VOS_OK;
        }

        if ((g_pbCtrlInfo.emlInfo[type2EMLFileCnt - 1].recordNum < eMLRecordNum) ||
            (0xFF == eMLRecordNum) || (0 == eMLRecordNum)) {
            PB_INFO_LOG("SI_PB_SReadEMLProc: No email Record");
            return VOS_OK;
        }

        eMLContentOffset = (eMLRecordNum - 1) * g_pbCtrlInfo.emlInfo[type2EMLFileCnt - 1].recordLen;

        /* 由于EMAIL文件每个记录数可能不一致，需要逐个累加来计算偏移量 */
        for (i = 0; i < (type2EMLFileCnt - 1); i++) {
            eMLContentOffset += g_pbCtrlInfo.emlInfo[i].recordLen * g_pbCtrlInfo.emlInfo[i].recordNum;
        }

        emlContent = g_emlContent.content + eMLContentOffset;
    }

    SI_PB_TransEMLFromate(g_emlContent.dataLen, emlContent, record);

    return VOS_OK;
}

/*
 * 功能描述: ANR电话本读取
 * 修改历史:
 * 1. 日    期: 2009年06月05日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PB_SReadANRProc(VOS_UINT16 index, VOS_UINT16 offset, SI_PB_Record *record)
{
    VOS_UINT32 anrContentOffset;
    VOS_UINT32 i;
    VOS_UINT8  j;
    VOS_UINT8 *anrContent = VOS_NULL_PTR;
    VOS_UINT8  type2ANRRecord;
    VOS_UINT8  type2ANRFileCnt;
    VOS_UINT32 type2ANROffset;

    /* ANR,先根据缓存个数判断是否存在USED，如果存在读内存，内存为空则出错，
    内存内容读上来后判断 */
    for (i = 0; i < g_pbCtrlInfo.anrStorageNum; i++) {
        if (VOS_NULL == g_anrContent[i].content) {
            PB_ERROR_LOG("SI_PB_SReadANRProc: storage NULL");
            return TAF_ERR_PB_STORAGE_OP_FAIL;
        }

        /* 如果为Type2类型ANR，需要通过IAP找到真实需要读取的 */
        if (PB_FILE_TYPE2 == g_pbCtrlInfo.anrInfo[0][i].anrType) {
            type2ANROffset = (((index + offset) - 1) * g_iapContent.recordLen) +
                             (g_pbCtrlInfo.anrInfo[0][i].anrTagNum - 1);

            if ((g_iapContent.iapContent == VOS_NULL_PTR) || (type2ANROffset >= g_iapContent.contentSize)) {
                PB_ERROR_LOG("SI_PB_SReadANRProc: storage NULL");
                return TAF_ERR_PB_STORAGE_OP_FAIL;
            }

            /*lint -e679*/
            type2ANRRecord = g_iapContent.iapContent[type2ANROffset];
            /*lint +e679*/

            if (VOS_OK != SI_PB_GetFileCntFromIndex(index + offset, &type2ANRFileCnt)) {
                PB_WARNING_LOG("SI_PB_SReadANRProc: SI_PB_GetFileCntFromIndex Return Error.");

                continue;
            }

            /* index对应的Type2 ANR无效 */
            if ((g_pbCtrlInfo.anrInfo[type2ANRFileCnt - 1][i].recordNum < type2ANRRecord) ||
                (0 == type2ANRRecord) || (0xFF == type2ANRRecord)) {
                PB_NORMAL_LOG("SI_PB_SReadANRProc: Empty Type2 ANR Or Invalid");

                continue;
            }

            anrContentOffset = (type2ANRRecord - 1) * g_anrContent[i].recordLen;

            for (j = 0; j < (type2ANRFileCnt - 1); j++) {
                anrContentOffset += g_pbCtrlInfo.anrInfo[j][i].recordNum *
                                      g_pbCtrlInfo.anrInfo[j][i].recordLen;
            }

            anrContent = g_anrContent[i].content + anrContentOffset;
        } else {
            /* 对于TYPE1类型的ANR文件，转入的参数index + usOffset不能超过ANR的总记录数 */
            if ((index + offset) > g_anrContent[i].totalNum) {
                continue;
            }

            anrContent = g_anrContent[i].content + (((index + offset) - 1) * g_anrContent[i].recordLen);
        }

        SI_PB_TransANRFromate((VOS_UINT8)i, anrContent, g_anrContent[i].recordLen, record);
    }

    return VOS_OK;
}

/*
 * 功能描述: 复合电话本读取
 * 修改历史:
 * 1. 日    期: 2009年06月05日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PB_SReadProc(PBMsgBlock *msg)
{
    SI_PB_ReadReq *reqMsg = VOS_NULL_PTR;
    SI_PB_EventInfo cnfData;
    VOS_UINT8             pBoffset;
    VOS_UINT32            result;
    VOS_UINT16            indexNum;
    VOS_UINT16            i;
    VOS_UINT8            *content     = VOS_NULL_PTR;
    VOS_UINT8            *tempContent = VOS_NULL_PTR;
    VOS_UINT32            validFlag    = VOS_FALSE;

    if (msg->ulLength < (sizeof(SI_PB_ReadReq) - VOS_MSG_HEAD_LENGTH)) {
        PB_ERROR1_LOG("SI_PB_SReadProc: incorrect msg length", msg->ulLength);
        PB_ERROR1_LOG("SI_PB_SReadProc: incorrect send id", msg->ulSenderPid);

        return VOS_ERR;
    }

    (VOS_VOID)memset_s((VOS_UINT8 *)&cnfData, sizeof(SI_PB_EventInfo), 0, sizeof(SI_PB_EventInfo));

    reqMsg = (SI_PB_ReadReq *)msg;

    cnfData.clientId = reqMsg->client;

    cnfData.opId = reqMsg->opId;

    cnfData.pbEventType = SI_PB_EVENT_SREAD_CNF;

    cnfData.storage = reqMsg->storage;

    if ((0 == g_pbConfigInfo.spbFlag) || (SI_PB_STORAGE_SM != reqMsg->storage)) {
        /* 改为Operation not allowed */
        cnfData.pbError = TAF_ERR_CMD_TYPE_ERROR;

        PB_WARNING_LOG("SI_PB_SReadProc: Proc is Not Allow");

        return SI_PBCallback(&cnfData);
    }

    cnfData.pbError = SI_PB_ErrorProc();

    if (TAF_ERR_NO_ERROR != cnfData.pbError) {
        PB_WARNING_LOG("SI_PB_SReadProc: Proc Error");

        return SI_PBCallback(&cnfData);
    }

    g_pbReqUnit.pbLock = PB_LOCKED;

    result = SI_PB_LocateRecord(reqMsg->storage, reqMsg->index1, reqMsg->index2, &pBoffset);
    if (VOS_OK != result) { /* 当前电话本内容未找到 */
        PB_WARNING_LOG("SI_PB_SReadProc: SI_PB_LocateRecord Return Error");

        cnfData.pbError = result;

        (VOS_VOID)SI_PBCallback(&cnfData);

        return VOS_ERR;
    }

    if (0 == reqMsg->index1) {
        indexNum = g_pbContent[pBoffset].totalNum;

        reqMsg->index1 = 1; /* 从第一条开始读取 */

        reqMsg->index2 = g_pbContent[pBoffset].totalNum;
    } else {
        indexNum = (reqMsg->index2 - reqMsg->index1) + 1; /* 计算两个索引之间的记录数 */
    }

    if (VOS_NULL == g_pbContent[pBoffset].content) { /* ADN一定存在缓存 */
        cnfData.pbError = TAF_ERR_PB_STORAGE_OP_FAIL;

        PB_WARNING_LOG("SI_PB_SReadProc: The ADN Memory Error");

        return SI_PBCallback(&cnfData);
    }

    content = g_pbContent[pBoffset].content + ((reqMsg->index1 - 1) * g_pbContent[pBoffset].recordLen);

    cnfData.pbLastTag = VOS_FALSE;

    for (i = 0; i < indexNum; i++) { /* 根据读取的电话本索引范围循环 */
        cnfData.pbError = VOS_OK;

        (VOS_VOID)memset_s((VOS_UINT8 *)&cnfData.pbEvent.pbReadCnf, sizeof(SI_PB_EventReadCnf), 0,
                           sizeof(SI_PB_EventReadCnf));

        cnfData.pbEvent.pbReadCnf.recordNum = 1;

        cnfData.pbEvent.pbReadCnf.pbRecord.index = (VOS_UINT16)(reqMsg->index1 + i);

        /* 此条记录无效，且不是最后一条 */
        if (SI_PB_CONTENT_INVALID == SI_PB_GetBitFromBuf(g_pbContent[pBoffset].index,
                                                         g_pbContent[pBoffset].indexSize,
                                                         reqMsg->index1 + i)) { /* 从index中获取有效信息 */
            if (i == (indexNum - 1)) { /* 最后一条记录无效 */
                cnfData.pbError   = ((VOS_TRUE == validFlag) ? TAF_ERR_NO_ERROR : TAF_ERR_PB_NOT_FOUND);
                cnfData.pbLastTag = VOS_TRUE;

                return SI_PBCallback(&cnfData);
            }

            continue;
        }

        validFlag = VOS_TRUE;

        tempContent = content + (i * g_pbContent[pBoffset].recordLen); /* 内容指针按照记录长度偏移 */

        SI_PB_TransPBFromate(&g_pbContent[pBoffset], (VOS_UINT16)(reqMsg->index1 + i), tempContent,
                             &cnfData.pbEvent.pbReadCnf.pbRecord);

        /* ANR,先根据缓存个数判断是否存在USED，如果存在读内存，内存为空则出错，
        内存内容读上来后判断 */
        result = SI_PB_SReadANRProc(reqMsg->index1, i, &cnfData.pbEvent.pbReadCnf.pbRecord);
        if (VOS_OK != result) {
            cnfData.pbError = result;

            PB_WARNING_LOG("SI_PB_SReadProc: SI_PB_SReadANRProc Return Error");

            return SI_PBCallback(&cnfData);
        }

        result = SI_PB_SReadEMLProc(reqMsg->index1, i, &cnfData.pbEvent.pbReadCnf.pbRecord);
        if (VOS_OK != result) {
            cnfData.pbError = result;

            PB_WARNING_LOG("SI_PB_SReadProc: SI_PB_SReadEMLProc Return Error");

            return SI_PBCallback(&cnfData);
        }

        /* 读取的已是最后一条记录，则置标志位为True */
        if (i == (indexNum - 1)) {
            cnfData.pbLastTag = VOS_TRUE;
        }

        (VOS_VOID)SI_PBCallback(&cnfData); /* 返回转换结果 */
    }

    return VOS_OK;
}

/*
 * 功能描述: 用于处理Ccpu电话本发送的全局变量初始化完毕消息
 * 修改历史:
 * 1.日    期: 2007年10月15日
 *   修改内容: Creat
 */
VOS_UINT32 SI_PBUpdateAGlobal(PBMsgBlock *msg)
{
    SI_PB_UpdateglobalInd *indMsg = VOS_NULL_PTR;
    errno_t ret;

#if (VOS_LINUX == VOS_OS_VER)
    VOS_UINT32   i;
    VOS_UINT_PTR tempAddr;
#endif

    if (msg->ulLength < (sizeof(SI_PB_UpdateglobalInd) - VOS_MSG_HEAD_LENGTH)) {
        (VOS_VOID)memset_s(&g_pbCtrlInfo, sizeof(g_pbCtrlInfo), 0, sizeof(g_pbCtrlInfo));
        (VOS_VOID)memset_s(&g_pbConfigInfo, sizeof(g_pbConfigInfo), 0, sizeof(g_pbConfigInfo));
        (VOS_VOID)memset_s(g_pbContent, sizeof(g_pbContent), 0, sizeof(g_pbContent));
        (VOS_VOID)memset_s(g_extContent, sizeof(g_extContent), 0, sizeof(g_extContent));
        (VOS_VOID)memset_s(g_anrContent, sizeof(g_anrContent), 0, sizeof(g_anrContent));
        (VOS_VOID)memset_s(&g_emlContent, sizeof(g_emlContent), 0, sizeof(g_emlContent));
        (VOS_VOID)memset_s(&g_iapContent, sizeof(g_iapContent), 0, sizeof(g_iapContent));
        (VOS_VOID)memset_s(&g_pbInitState, sizeof(g_pbInitState), 0, sizeof(g_pbInitState));

        PB_ERROR1_LOG("SI_PBUpdateAGlobal: incorrect msg", msg->ulLength);

        return VOS_ERR;
    }

    indMsg = (SI_PB_UpdateglobalInd *)msg;

    ret = memcpy_s(&g_pbCtrlInfo, sizeof(g_pbCtrlInfo), &indMsg->pbCtrlInfo, sizeof(g_pbCtrlInfo));
    PAM_PRINT_SECFUN_RESULT(ret);

    ret = memcpy_s(&g_pbConfigInfo, sizeof(g_pbConfigInfo), &indMsg->pbConfigInfo, sizeof(g_pbConfigInfo));
    PAM_PRINT_SECFUN_RESULT(ret);

    ret = memcpy_s(g_pbContent, sizeof(g_pbContent), indMsg->pbContent, sizeof(g_pbContent));
    PAM_PRINT_SECFUN_RESULT(ret);

    ret = memcpy_s(g_extContent, sizeof(g_extContent), indMsg->extContent, sizeof(g_extContent));
    PAM_PRINT_SECFUN_RESULT(ret);

    ret = memcpy_s(g_anrContent, sizeof(g_anrContent), indMsg->anrContent, sizeof(g_anrContent));
    PAM_PRINT_SECFUN_RESULT(ret);

    ret = memcpy_s(&g_emlContent, sizeof(g_emlContent), &indMsg->emlContent, sizeof(g_emlContent));
    PAM_PRINT_SECFUN_RESULT(ret);

    ret = memcpy_s(&g_iapContent, sizeof(g_iapContent), &indMsg->iapContent, sizeof(g_iapContent));
    PAM_PRINT_SECFUN_RESULT(ret);

    ret = memcpy_s(&g_pbInitState, sizeof(g_pbInitState), &indMsg->pbInitState, sizeof(g_pbInitState));
    PAM_PRINT_SECFUN_RESULT(ret);

#if (VOS_LINUX == VOS_OS_VER)
    for (i = 0; i < PB_CONTENT_BUTT; i++) {
        g_pbContent[i].content = VOS_NULL_PTR;

        tempAddr = indMsg->pbContentAddr[i].contentAddr;

        g_pbContent[i].content = (VOS_UINT8 *)tempAddr;

        g_pbContent[i].index = VOS_NULL_PTR;

        tempAddr = indMsg->pbContentAddr[i].indexAddr;

        g_pbContent[i].index = (VOS_UINT8 *)tempAddr;

        if (VOS_NULL_PTR != g_pbContent[i].content) {
            g_pbContent[i].content = (VOS_UINT8 *)mdrv_phy_to_virt(MEM_DDR_MODE, g_pbContent[i].content);
        }
        if (VOS_NULL_PTR != g_pbContent[i].index) {
            g_pbContent[i].index = (VOS_UINT8 *)mdrv_phy_to_virt(MEM_DDR_MODE, g_pbContent[i].index);
        }
    }

    for (i = 0; i < SI_PB_MAX_NUMBER; i++) {
        g_extContent[i].extContent = VOS_NULL_PTR;

        tempAddr = indMsg->extContentAddr[i];

        g_extContent[i].extContent = (VOS_UINT8 *)tempAddr;

        if (VOS_NULL_PTR != g_extContent[i].extContent) {
            g_extContent[i].extContent = (VOS_UINT8 *)mdrv_phy_to_virt(MEM_DDR_MODE, g_extContent[i].extContent);
        }
    }

    for (i = 0; i < SI_PB_ANRMAX; i++) {
        g_anrContent[i].content = VOS_NULL_PTR;

        tempAddr = indMsg->anrContentAddr[i];

        g_anrContent[i].content = (VOS_UINT8 *)tempAddr;

        if (VOS_NULL_PTR != g_anrContent[i].content) {
            g_anrContent[i].content = (VOS_UINT8 *)mdrv_phy_to_virt(MEM_DDR_MODE, g_anrContent[i].content);
        }
    }

    g_emlContent.content = VOS_NULL_PTR;

    tempAddr = indMsg->emlContentAddr;

    g_emlContent.content = (VOS_UINT8 *)tempAddr;

    if (VOS_NULL_PTR != g_emlContent.content) {
        g_emlContent.content = (VOS_UINT8 *)mdrv_phy_to_virt(MEM_DDR_MODE, g_emlContent.content);
    }

    g_iapContent.iapContent = VOS_NULL_PTR;

    tempAddr = indMsg->iapContentAddr;

    g_iapContent.iapContent = (VOS_UINT8 *)tempAddr;

    if (VOS_NULL_PTR != g_iapContent.iapContent) {
        g_iapContent.iapContent = (VOS_UINT8 *)mdrv_phy_to_virt(MEM_DDR_MODE, g_iapContent.iapContent);
    }
#endif

    return VOS_OK;
}

/*
 * 功能描述: 用于处理Ccpu发送的更新当前电话本设置消息
 * 修改历史:
 * 1. 日    期: 2007年10月15日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PBUpdateACurPB(PBMsgBlock *msg)
{
    SI_PB_SetpbInd *indMsg = VOS_NULL_PTR;

    if (msg->ulLength < (sizeof(SI_PB_SetpbInd) - VOS_MSG_HEAD_LENGTH)) {
        PB_ERROR1_LOG("SI_PBUpdateACurPB: incorrect msg length", msg->ulLength);
        PB_ERROR1_LOG("SI_PBUpdateACurPB: incorrect send id", msg->ulSenderPid);

        return VOS_ERR;
    }

    indMsg = (SI_PB_SetpbInd *)msg;

    g_pbCtrlInfo.pbCurType = indMsg->pbCurType;

    return VOS_OK;
}
#endif

#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (defined(DMT)))

VOS_UINT32 SI_PBSendAtEventCnf(SI_PB_EventInfo *event)
{
    MN_APP_PbAtCnf *msg = VOS_NULL_PTR; /* 发送给AT的消息 */
    errno_t         ret;

    if (event->pbError != TAF_ERR_NO_ERROR) {
        g_pbReqUnit.pbLock = PB_UNLOCK; /* 解锁 */
    } else if ((event->pbEventType != SI_PB_EVENT_READ_CNF) && (event->pbEventType != SI_PB_EVENT_SREAD_CNF) &&
               (event->pbEventType != SI_PB_EVENT_SEARCH_CNF)) {
        g_pbReqUnit.pbLock = PB_UNLOCK; /* 解锁 */
    } else if (event->pbLastTag == VOS_TRUE) {
        g_pbReqUnit.pbLock = PB_UNLOCK; /* 解锁 */
    } else {
    }

    msg = (MN_APP_PbAtCnf *)VOS_AllocMsg(MAPS_PB_PID, sizeof(MN_APP_PbAtCnf) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) { /* 每次申请失败会复位 */
        return VOS_ERR;
    }

    msg->ulReceiverPid = WUEPS_PID_AT;
    msg->msgId       = PB_AT_EVENT_CNF;

    ret = memcpy_s(&msg->pbAtEvent, sizeof(SI_PB_EventInfo), event, sizeof(SI_PB_EventInfo));
    PAM_PRINT_SECFUN_RESULT(ret);

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_OK != VOS_SendMsg(MAPS_PB_PID, msg)) {
        PB_ERROR_LOG("SI_PBSendAtEventCnf:Send AT Msg Error");

        return VOS_ERR;
    }

    return VOS_OK;
}
/*
 * 功能描述: PB模块发送电话本初始化状态指示消息
 * 修改历史:
 * 1.日    期: 2015年01月30日
 *   修改内容: Create
 */
VOS_VOID SI_PB_InitStatusInd(USIMM_PbInitStatusUint16 pbInitStatus)
{
    USIMM_PbinitStatusInd *msg = VOS_NULL_PTR;

    msg = (USIMM_PbinitStatusInd *)VOS_AllocMsg(MAPS_PB_PID,
                                                          sizeof(USIMM_PbinitStatusInd) - VOS_MSG_HEAD_LENGTH);
    if (VOS_NULL_PTR == msg) {
        PB_WARNING_LOG("SI_PB_InitStatusInd: VOS_AllocMsg is Failed");

        return;
    }

    msg->msgHeader.ulReceiverPid = WUEPS_PID_USIM;
    msg->msgHeader.msgName     = USIMM_PBINIT_STATUS_IND;
    msg->msgHeader.appType     = USIMM_GUTL_APP;
    msg->msgHeader.sendPara    = VOS_NULL;
    msg->pbInitStatus            = pbInitStatus;
    msg->rsv                     = VOS_NULL;

    PAM_OM_ReportLayerMsg(msg);
    (VOS_VOID)VOS_SendMsg(MAPS_PB_PID, msg);

    return;
}

/*
 * 功能描述:对姓名字段长度进行处理
 * 修改历史:
 * 1. 日    期: 2007年10月15日
 *    修改内容: Creat
 */
VOS_UINT8 SI_PB_AlphaTagTruncation(SI_PB_Record *record, VOS_UINT8 offset)
{
    VOS_UINT8 uctemp;
    VOS_UINT8 len;

    /* 如果姓名长度为0，则返回Copy长度为0 */
    if ((0 == record->alphaTagLength) || (0 == g_pbContent[offset].nameLen)) {
        PB_INFO_LOG("SI_PB_AlphaTagTruncation: The Name Len is 0");
        return 0;
    }

    if (SI_PB_ALPHATAG_TYPE_UCS2_80 == record->alphaTagType) {
        uctemp = g_pbContent[offset].nameLen - ((g_pbContent[offset].nameLen - 1) % 2);

        return (record->alphaTagLength > uctemp) ? uctemp : record->alphaTagLength;
    } else if (SI_PB_ALPHATAG_TYPE_GSM == record->alphaTagType) {
        if (record->alphaTagLength > g_pbContent[offset].nameLen) {
            len = g_pbContent[offset].nameLen;

            len = (SI_PB_ALPHATAG_TRANSFER_TAG == record->alphaTag[len - 1]) ? (len - 1) : (len);
        } else {
            len = record->alphaTagLength;
        }

        return len;
    } else { /* 剩下81、82编码两种情况 */
        /* 计算被截去的长度 */
        uctemp = (record->alphaTagLength > g_pbContent[offset].nameLen) ? g_pbContent[offset].nameLen
                 : record->alphaTagLength;

        /* 81 编码格式最小3字节；82编码格式最小4字节 */
        if (uctemp < ((0x81 == record->alphaTag[0]) ? 3 : 4)) {
            record->alphaTag[1] = 0;

            return 0;
        }

        record->alphaTag[1] = uctemp - ((0x81 == record->alphaTag[0]) ? 3 : 4);

        return uctemp;
    }
}

/*
 * 功能描述:更新电话本的文件内容
 * 修改历史:
 * 1. 日    期: 2007年10月15日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PB_UpdataXDNFile(VOS_UINT8 pbOffset, VOS_UINT16 fileId, VOS_UINT16 extFileId, VOS_UINT8 recordNum,
                               SI_PB_Record *record)
{
    VOS_UINT8               copyLen;
    VOS_UINT8               extRecord;
    VOS_UINT32              result;
    VOS_UINT32              xDNContentOffset;
    VOS_UINT32              extInfoNum;
    SI_PB_SetfileInfo updateReq;
    errno_t                 ret;

    (VOS_VOID)memset_s(g_pbReqUnit.xdnContent, sizeof(g_pbReqUnit.xdnContent), 0xFF,
                       sizeof(g_pbReqUnit.xdnContent));
    (VOS_VOID)memset_s(g_pbReqUnit.extContent, sizeof(g_pbReqUnit.extContent), 0xFF,
                       sizeof(g_pbReqUnit.extContent));

    copyLen = SI_PB_AlphaTagTruncation(record, pbOffset);

    ret = memcpy_s(g_pbReqUnit.xdnContent, sizeof(g_pbReqUnit.xdnContent), record->alphaTag, copyLen);
    if (ret != EOK) {
        PB_WARNING_LOG("SI_PB_UpdataXDNFile: the input name length is too long.");

        return VOS_ERR;
    }

    /* 为避免aucXDNContent中内存被越界访问，需要对NameLen长度进行判断，从后面流程看出需要预留13字节 */
    if ((sizeof(g_pbReqUnit.xdnContent) - 13) <= g_pbContent[pbOffset].nameLen) {
        PB_WARNING_LOG("SI_PB_UpdataXDNFile: the name length is too long.");

        return VOS_ERR;
    }

    if (record->numberLength != 0) {
        g_pbReqUnit.xdnContent[g_pbContent[pbOffset].nameLen + 1] = record->numberType;
    }

    if ((record->numberLength > SI_PB_NUM_LEN) && (0xFFFF != extFileId)) {
        SI_PB_AsciiToBcd(record->number, SI_PB_NUM_LEN,
                         &g_pbReqUnit.xdnContent[g_pbContent[pbOffset].nameLen + 2],
                         sizeof(g_pbReqUnit.xdnContent) - (g_pbContent[pbOffset].nameLen + 2), &copyLen);

        g_pbReqUnit.xdnContent[g_pbContent[pbOffset].nameLen] = copyLen + 1;

        xDNContentOffset = (record->index - 1) * g_pbContent[pbOffset].recordLen;

        (pbOffset == PB_BDN_CONTENT) ?
            (xDNContentOffset += g_pbContent[pbOffset].recordLen - 2) : (xDNContentOffset += g_pbContent[pbOffset].recordLen - 1);

        if (g_pbContent[pbOffset].content == VOS_NULL_PTR) {
            PB_WARNING_LOG("SI_PB_UpdataXDNFile: content is NULL");

            return VOS_ERR;
        }

        /* 检查当前更新的记录是否已经占用了一个EXT文件记录 */
        extRecord  = g_pbContent[pbOffset].content[xDNContentOffset];
        extInfoNum = g_pbContent[pbOffset].extInfoNum;

        /* 如果XDN记录对应已经存在了EXT记录，则不需要去查找空闲的EXT记录 */
        if ((0xFF == extRecord) || (0 == extRecord)) {
            /* 找得到空闲的EXT文件记录时才更新EXT文件，找不到就不更新，超长号码也不报错 */
            result = SI_PB_FindUnusedExtRecord(g_extContent + extInfoNum, &extRecord, 1);
        } else {
            result = VOS_OK;
        }

        if ((extRecord <= g_extContent[extInfoNum].extTotalNum) && (VOS_OK == result)) {
            SI_PB_AsciiToBcd(record->number + SI_PB_NUM_LEN, (VOS_UINT8)(record->numberLength - SI_PB_NUM_LEN),
                             g_pbReqUnit.extContent + 2, sizeof(g_pbReqUnit.extContent) - 2, &copyLen);

            g_pbReqUnit.extContent[0] = SI_PB_ADDITIONAL_NUM;
            g_pbReqUnit.extContent[1] = copyLen;

            updateReq.efContent = g_pbReqUnit.extContent;
            updateReq.recordNum  = extRecord;
            updateReq.efLen      = SI_PB_EXT_LEN;
            updateReq.efId       = extFileId;

            result = SI_PB_SetFileReq(&updateReq);
        }

        /* 先更新EXT文件再更新ADN文件内容，如果更新EXT文件失败，对应的标志位设置为FF */
        if (VOS_OK != result) {
            g_pbReqUnit.xdnContent[g_pbContent[pbOffset].nameLen + 13] = 0xFF;
        } else {
            g_pbReqUnit.xdnContent[g_pbContent[pbOffset].nameLen + 13] = extRecord;
            g_pbReqUnit.extIndex                                              = extRecord;
        }

        updateReq.efContent = g_pbReqUnit.xdnContent;
        updateReq.recordNum  = recordNum;
        updateReq.efLen      = g_pbContent[pbOffset].recordLen;
        updateReq.efId       = fileId;

        /* 如果更新XDN文件成功，则认为更新EXT文件也是成功的 */
        return SI_PB_SetFileReq(&updateReq);
    }

    SI_PB_AsciiToBcd(record->number, record->numberLength,
                     &g_pbReqUnit.xdnContent[g_pbContent[pbOffset].nameLen + 2],
                     sizeof(g_pbReqUnit.xdnContent) - (g_pbContent[pbOffset].nameLen + 2), &copyLen);

    if (0 != record->numberLength) {
        g_pbReqUnit.xdnContent[g_pbContent[pbOffset].nameLen] = copyLen + 1;
    }

    updateReq.efContent = g_pbReqUnit.xdnContent;
    updateReq.recordNum  = recordNum;
    updateReq.efLen      = g_pbContent[pbOffset].recordLen;
    updateReq.efId       = fileId;

    return SI_PB_SetFileReq(&updateReq);
}

/*
 * 功能描述:更新ADN的文件内容
 * 修改历史:
 * 1. 日    期: 2009年06月08日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PB_UpdataADN(SI_PB_Record *record, SI_PB_Spbdata *spbReq, VOS_UINT16 *extUsedNum,
                           const SI_PB_SpbUpdate *pBFid)
{
    VOS_UINT8   copyLen;
    VOS_UINT8   numberLen;
    VOS_UINT8   extRecord;
    VOS_UINT16 *fileNum = VOS_NULL_PTR;
    VOS_UINT32  eXTContentOffset;
    VOS_UINT32  result = VOS_OK;
    errno_t     ret;

    fileNum = &spbReq->fileNum;

    (VOS_VOID)memset_s(g_pbReqUnit.xdnContent, sizeof(g_pbReqUnit.xdnContent), 0xFF,
                       sizeof(g_pbReqUnit.xdnContent));

    copyLen = SI_PB_AlphaTagTruncation(record, PB_ADN_CONTENT);

    copyLen = (VOS_UINT8)PAM_GetMin(copyLen, sizeof(g_pbReqUnit.xdnContent));

    /* 拷贝待更新的姓名内容到本地 */
    ret = memcpy_s(g_pbReqUnit.xdnContent, sizeof(g_pbReqUnit.xdnContent), record->alphaTag, copyLen);

    PAM_PRINT_SECFUN_RESULT(ret);

    /* 处理ADN号码 */
    if (0 != record->numberLength) {
        g_pbReqUnit.xdnContent[g_pbContent[PB_ADN_CONTENT].nameLen + 1] = record->numberType;

        numberLen = (record->numberLength > SI_PB_NUM_LEN) ? SI_PB_NUM_LEN : record->numberLength;

        /* 转换并拷贝待更新号码内容到本地 */
        SI_PB_AsciiToBcd(record->number, numberLen,
                         &g_pbReqUnit.xdnContent[g_pbContent[PB_ADN_CONTENT].nameLen + 2],
                         sizeof(g_pbReqUnit.xdnContent) - (g_pbContent[PB_ADN_CONTENT].nameLen + 2),
                         &copyLen);

        g_pbReqUnit.xdnContent[g_pbContent[PB_ADN_CONTENT].nameLen] = copyLen + 1;
    }

    eXTContentOffset = (record->index - 1) * g_pbContent[PB_ADN_CONTENT].recordLen;

    eXTContentOffset += g_pbContent[PB_ADN_CONTENT].recordLen - 1;

    /* 检查当前更新的记录是否已经占用了一个EXT文件记录 */
    extRecord = g_pbContent[PB_ADN_CONTENT].content[eXTContentOffset];
    extRecord = ((extRecord <= g_extContent[PB_ADN_CONTENT].extTotalNum) ? (extRecord) : 0xFF);

    (VOS_VOID)memset_s(g_pbReqUnit.extContent, sizeof(g_pbReqUnit.extContent), 0xFF,
                       sizeof(g_pbReqUnit.extContent));

    /* ADN超长号码处理 */
    if (record->numberLength > SI_PB_NUM_LEN) {
        if (VOS_NULL_PTR == g_extContent[PB_ADN_CONTENT].extContent) {
            /* 出错处理 */
            /* Operation not allowed */
            PB_WARNING_LOG("SI_PB_UpdataADN:Ext Content NULL");
            return TAF_ERR_PB_STORAGE_OP_FAIL;
        }

        /* 如果没有对应的EXT文件记录需要寻找空闲EXT记录 */
        if (0xFF == extRecord) {
            /* 找得到空闲的EXT文件记录时才更新EXT文件，找不到就不更新，超长号码也不报错 */
            result = SI_PB_FindUnusedExtRecord(&g_extContent[PB_ADN_CONTENT], &extRecord,
                                                 (VOS_UINT8)*extUsedNum);

            (*extUsedNum)++;
        }

        if (VOS_OK == result) {
            SI_PB_AsciiToBcd(record->number + SI_PB_NUM_LEN, (VOS_UINT8)(record->numberLength - SI_PB_NUM_LEN),
                             g_pbReqUnit.extContent + 2, sizeof(g_pbReqUnit.extContent) - 2, &copyLen);

            g_pbReqUnit.extIndex                                                   = extRecord;
            g_pbReqUnit.extContent[0]                                             = SI_PB_ADDITIONAL_NUM;
            g_pbReqUnit.extContent[1]                                             = copyLen;
            g_pbReqUnit.xdnContent[g_pbContent[PB_ADN_CONTENT].recordLen - 1] = extRecord;

            /* 记录待更新的EXT记录信息 */
            spbReq->fileId[*fileNum]    = pBFid->extFileId;
            spbReq->dataLen[*fileNum]   = SI_PB_EXT_LEN;
            spbReq->recordNum[*fileNum] = extRecord;
            spbReq->content[*fileNum]    = g_pbReqUnit.extContent;
            (*fileNum)++;
        } else {
            /* Buffer full */
            PB_WARNING_LOG("SI_PB_UpdataADN:ADN Ext number Full");
            return TAF_ERR_PB_STORAGE_FULL;
        }
    } else {
        /* 如果更新文件时号码长度小于20，可能需要将EXT号码删除 */
        if ((0xFF != extRecord) && (0xFFFF != pBFid->extFileId)) {
            g_pbReqUnit.extContent[0] = 0;
            g_pbReqUnit.extIndex       = extRecord;

            spbReq->fileId[*fileNum]    = pBFid->extFileId;
            spbReq->dataLen[*fileNum]   = SI_PB_EXT_LEN;
            spbReq->recordNum[*fileNum] = extRecord;
            spbReq->content[*fileNum]    = g_pbReqUnit.extContent;
            (*fileNum)++;
        }

        g_pbReqUnit.xdnContent[g_pbContent[PB_ADN_CONTENT].recordLen - 1] = 0xFF;
    }

    /* 记录待更新的ADN记录信息,在更新EXT文件之后 */
    spbReq->fileId[*fileNum]    = pBFid->adnFileId;
    spbReq->recordNum[*fileNum] = pBFid->recordNum;
    spbReq->dataLen[*fileNum]   = g_pbContent[PB_ADN_CONTENT].recordLen;
    spbReq->content[*fileNum]    = g_pbReqUnit.xdnContent;
    (*fileNum)++;

    return VOS_OK;
}

/*
 * 功能描述:更新ANR的文件内容
 * 修改历史:
 * 1. 日    期: 2009年09月24日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PB_UpdataType2ANR(const SI_PB_Record *record, SI_PB_Spbdata *spbReq,
                                const SI_PB_SpbUpdate *pBFid, VOS_UINT32 i, VOS_UINT8 *freeANRRecord)
{
    VOS_UINT32 type2ANROffset;
    errno_t    ret;

    if (0 == g_pbCtrlInfo.adnInfo[0].recordNum) {
        PB_WARNING_LOG("SI_PB_UpdataType2ANR:recordNum is 0");

        return TAF_ERR_PB_WRONG_PARA;
    }

    type2ANROffset = ((record->index - 1) * g_iapContent.recordLen) +
                     (g_pbCtrlInfo.anrInfo[0][i].anrTagNum - 1);

    if ((VOS_NULL_PTR == g_iapContent.iapContent) || (type2ANROffset >= g_iapContent.contentSize)) {
        PB_WARNING_LOG("SI_PB_UpdataType2ANR:IAP Content NULL");
        return TAF_ERR_PB_STORAGE_OP_FAIL;
    }

    if ((0xFF == g_iapContent.iapContent[type2ANROffset]) && (0 != record->additionNumber[i].numberLength)) {
        if (VOS_OK != SI_PB_GetFreeANRRecordNum(pBFid->anrFileId[i], freeANRRecord,
                                                &spbReq->fileId[spbReq->fileNum])) {
            PB_WARNING_LOG("SI_PB_UpdataType2ANR:No Free ANR Record");
            return TAF_ERR_PB_STORAGE_FULL;
        }

        /* 最多支持20个Type2类型文件 */
        if (sizeof(g_pbReqUnit.spbReq.iapContent) < g_iapContent.recordLen) {
            PB_ERROR_LOG("SI_PB_UpdataType2ANR:IAP recordLen too long");

            return TAF_ERR_PB_STORAGE_OP_FAIL;
        }

        ret = memcpy_s(g_pbReqUnit.spbReq.iapContent, sizeof(g_pbReqUnit.spbReq.iapContent),
                       &g_iapContent.iapContent[(record->index - 1) * g_iapContent.recordLen],
                       g_iapContent.recordLen);

        PAM_PRINT_SECFUN_RESULT(ret);

        g_pbReqUnit.spbReq.iapContent[g_pbCtrlInfo.anrInfo[0][i].anrTagNum - 1] = *freeANRRecord;

        /* 需要及时更新内存中IAP内容 */
        g_iapContent.iapContent[type2ANROffset] = *freeANRRecord;

        spbReq->recordNum[spbReq->fileNum] = pBFid->recordNum;
        spbReq->dataLen[spbReq->fileNum]   = g_iapContent.recordLen;
        spbReq->content[spbReq->fileNum]    = g_pbReqUnit.spbReq.iapContent;

        spbReq->fileNum++;
    }
    /* 没有需要更新的记录 */
    else if ((0xFF == g_iapContent.iapContent[type2ANROffset]) &&
             (0 == record->additionNumber[i].numberLength)) {
        PB_INFO_LOG("SI_PB_UpdataType2ANR: The Record is not found");
        return TAF_ERR_PB_NOT_FOUND;
    }
    /* ANR号码填为空，需要删除IAP中对应关系 */
    else if ((0xFF != g_iapContent.iapContent[type2ANROffset]) &&
             (0 == record->additionNumber[i].numberLength)) {
        *freeANRRecord = g_iapContent.iapContent[type2ANROffset];

        /* 最多支持20个Type2类型文件 */
        if (sizeof(g_pbReqUnit.spbReq.iapContent) < g_iapContent.recordLen) {
            PB_ERROR_LOG("SI_PB_DeleteHandleEmail:IAP recordLen too long");

            return TAF_ERR_PB_STORAGE_OP_FAIL;
        }

        if (VOS_OK != SI_PB_GetIAPFidFromANR(pBFid->anrFileId[i], &spbReq->fileId[spbReq->fileNum])) {
            PB_WARNING_LOG("SI_PB_UpdataANR:No IAP");
            return TAF_ERR_PB_STORAGE_OP_FAIL;
        }

        ret = memcpy_s(g_pbReqUnit.spbReq.iapContent, sizeof(g_pbReqUnit.spbReq.iapContent),
                       &g_iapContent.iapContent[(record->index - 1) * g_iapContent.recordLen],
                       g_iapContent.recordLen);

        PAM_PRINT_SECFUN_RESULT(ret);

        g_pbReqUnit.spbReq.iapContent[g_pbCtrlInfo.anrInfo[0][i].anrTagNum - 1] = 0xFF;

        /* 需要及时更新内存中IAP内容 */
        g_iapContent.iapContent[type2ANROffset] = 0xFF;

        spbReq->recordNum[spbReq->fileNum] = pBFid->recordNum;
        spbReq->dataLen[spbReq->fileNum]   = g_iapContent.recordLen;
        spbReq->content[spbReq->fileNum]    = g_pbReqUnit.spbReq.iapContent;

        spbReq->fileNum++;
    } else {
        *freeANRRecord = g_iapContent.iapContent[type2ANROffset];
    }

    return VOS_OK;
}

/*
 * 功能描述: 更新ANR-EXT的文件内容
 * 修改历史:
 * 1. 日    期: 2009年06月08日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PB_UpdateAnrExt(const AnrUpdateProcPara *para, const SI_PB_Record *record, SI_PB_Spbdata *spbReq,
                              VOS_UINT16 *extUsedNum, const SI_PB_SpbUpdate *pbFid)
{
    VOS_UINT32  i;
    VOS_UINT8   extRecord;
    VOS_UINT16 *fileNum = VOS_NULL_PTR;
    VOS_UINT32  result;
    VOS_UINT8   copyLen;

    i         = para->index;
    extRecord = para->extRecord;

    fileNum = &spbReq->fileNum;

    if (record->additionNumber[i].numberLength > SI_PB_NUM_LEN) {
        if (g_extContent[PB_ADN_CONTENT].extContent == VOS_NULL_PTR) {
            /* 出错处理 */
            /* Operation not allowed */
            PB_WARNING_LOG("SI_PB_UpdateAnrExt:ANR Ext Content NULL");

            return TAF_ERR_PB_STORAGE_OP_FAIL;
        }

        result = VOS_OK;

        /* 如果XDN记录对应已经存在了EXT记录，则不需要去查找空闲的EXT记录 */
        if (extRecord == 0xFF) {
            /* 找得到空闲的EXT文件记录时才更新EXT文件，找不到就不更新，超长号码也不报错 */
            result = SI_PB_FindUnusedExtRecord(&g_extContent[PB_ADN_CONTENT], &(extRecord), (VOS_UINT8)*extUsedNum);

            (*extUsedNum)++;
        }

        if (result == VOS_OK) {
            (VOS_VOID)memset_s(g_pbReqUnit.spbReq.anrExtContent[i], SI_PB_EXT_LEN, 0xFF, SI_PB_EXT_LEN);

            SI_PB_AsciiToBcd(record->additionNumber[i].number + SI_PB_NUM_LEN,
                             (VOS_UINT8)(record->additionNumber[i].numberLength - SI_PB_NUM_LEN),
                             &g_pbReqUnit.spbReq.anrExtContent[i][2],
                             sizeof(g_pbReqUnit.spbReq.anrExtContent[i]) - 2, &copyLen);

            g_pbReqUnit.spbReq.anrExtIndex[i]       = extRecord;
            g_pbReqUnit.spbReq.anrExtContent[i][0] = SI_PB_ADDITIONAL_NUM;
            g_pbReqUnit.spbReq.anrExtContent[i][1] = copyLen;
            g_pbReqUnit.spbReq
            .anrContent[i][g_anrContent[i].recordLen -
                              ((PB_FILE_TYPE2 == g_pbCtrlInfo.anrInfo[0][i].anrType) ? 3 : 1)] = extRecord;

            /* 记录待更新的EXT记录信息 */
            spbReq->fileId[*fileNum]    = pbFid->extFileId;
            spbReq->dataLen[*fileNum]   = SI_PB_EXT_LEN;
            spbReq->recordNum[*fileNum] = extRecord;
            spbReq->content[*fileNum]    = g_pbReqUnit.spbReq.anrExtContent[i];
            spbReq->fileNum++;
        } else {
            PB_WARNING_LOG("SI_PB_UpdateAnrExt:ANR Ext Buffer Full");

            return TAF_ERR_PB_STORAGE_FULL;
        }
    } else {
        /* 如果更新文件时号码长度小于20，可能需要将EXT号码删除 */
        if ((extRecord != 0xFF) && (pbFid->extFileId != 0xFFFF)) {
            (VOS_VOID)memset_s(g_pbReqUnit.spbReq.anrExtContent[i], SI_PB_EXT_LEN, 0xFF, SI_PB_EXT_LEN);

            g_pbReqUnit.spbReq.anrExtContent[i][0] = 0;
            g_pbReqUnit.spbReq.anrExtIndex[i]       = extRecord;

            spbReq->fileId[*fileNum]    = pbFid->extFileId;
            spbReq->dataLen[*fileNum]   = SI_PB_EXT_LEN;
            spbReq->recordNum[*fileNum] = extRecord;
            spbReq->content[*fileNum]    = g_pbReqUnit.spbReq.anrExtContent[i];
            (*fileNum)++;
        }

        g_pbReqUnit.spbReq.anrContent[i][g_anrContent[i].recordLen -
                                               ((PB_FILE_TYPE2 == g_pbCtrlInfo.anrInfo[0][i].anrType) ? 3 : 1)] =
                                                   0xFF;
    }

    return VOS_OK;
}

/*
 * 功能描述:更新ANR的文件内容
 * 修改历史:
 * 1. 日    期: 2009年06月08日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PB_UpdataANR(const SI_PB_Record *record, SI_PB_Spbdata *spbReq, VOS_UINT16 *extUsedNum,
                           const SI_PB_SpbUpdate *pBFid)
{
    VOS_UINT32        i;
    VOS_UINT32        eXTContentOffset;
    VOS_UINT32        aNROffset;
    VOS_UINT32        fileCnt;
    VOS_UINT32        result  = VOS_OK;
    VOS_UINT16       *fileId = VOS_NULL_PTR;
    VOS_UINT8         aDNSfi  = 0;
    VOS_UINT8         numberLen;
    VOS_UINT8         tempCopyLen;
    VOS_UINT8         extRecord;
    VOS_UINT8         freeANRRecord;
    VOS_UINT32        copyLen;
    errno_t           ret;
    AnrUpdateProcPara para;

    fileId = &spbReq->fileNum;

    /* ANR文件更新的数据根据NV项控制 */
    for (i = 0; i < g_pbConfigInfo.anrMaxNum; i++) {
        /* 此ANR在缓存中不存在 */
        if (0 == pBFid->anrFileId[i]) {
            continue;
        }

        /* 如果Type2类型ANR */
        if (PB_FILE_TYPE2 == g_pbCtrlInfo.anrInfo[0][i].anrType) {
            result = SI_PB_UpdataType2ANR(record, spbReq, pBFid, i, &freeANRRecord);
            if (VOS_OK != result) {
                /* 该条ANR不需要更新 */
                if (TAF_ERR_PB_NOT_FOUND == result) {
                    continue;
                }

                PB_ERROR_LOG("SI_PB_UpdataANR:UpdataType2ANR Error");
                return result;
            }

            if (VOS_OK != SI_PB_GetADNSfi(&aDNSfi, pBFid->adnFileId)) {
                /* 出错处理 */
                PB_WARNING_LOG("SI_PB_UpdataANR: The SI_PB_GetADNSfi is not found SFI");
                return TAF_ERR_PB_STORAGE_OP_FAIL;
            }

            /* 获取当前要更新的是第几个Type2 ANR文件 */
            fileCnt = (record->index - 1) / g_pbCtrlInfo.adnInfo[0].recordNum;

            /* 获取需要更新的Type2 ANR记录在缓存中的偏移 */
            aNROffset = (fileCnt *
                           (g_pbCtrlInfo.anrInfo[0][i].recordNum * g_pbCtrlInfo.anrInfo[0][i].recordLen)) +
                          ((freeANRRecord - 1) * g_pbCtrlInfo.anrInfo[0][i].recordLen);

            /* 获取需要更新的Type2 ANR记录中扩展号码的信息在缓存中的偏移 */
            eXTContentOffset = aNROffset + (g_anrContent[i].recordLen - 3);
        } else {
            /* 获取当前要更新的是第几个Type1 ANR文件 */
            freeANRRecord = pBFid->recordNum;

            /* 获取需要更新的Type1 ANR记录在缓存中的偏移 */
            aNROffset = (record->index - 1) * g_anrContent[i].recordLen;

            /* 获取需要更新的Type1 ANR记录中扩展号码的信息在缓存中的偏移 */
            eXTContentOffset = aNROffset + (g_anrContent[i].recordLen - 1);
        }

        copyLen = PAM_GetMin(g_anrContent[i].recordLen, sizeof(g_pbReqUnit.spbReq.anrContent[i]));

        ret = memcpy_s(g_pbReqUnit.spbReq.anrContent[i], sizeof(g_pbReqUnit.spbReq.anrContent[i]),
                       &g_anrContent[i].content[aNROffset], copyLen);
        PAM_PRINT_SECFUN_RESULT(ret);

        /* 非删除请求 */
        if ((0 != record->additionNumber[i].numberLength) &&
            (PB_FILE_TYPE2 == g_pbCtrlInfo.anrInfo[0][i].anrType)) {
            g_pbReqUnit.spbReq.anrContent[i][g_anrContent[i].recordLen - 2] = aDNSfi; /* ADN SFI */
            g_pbReqUnit.spbReq.anrContent[i][g_anrContent[i].recordLen - 1] = pBFid->recordNum;
        }

        /* 将ANR记录的2到13共12字节都设置为0xFF */
        ret = memset_s(g_pbReqUnit.spbReq.anrContent[i] + 1, sizeof(g_pbReqUnit.spbReq.anrContent[i]) - 1, 0xFF, 12);
        PAM_PRINT_SECFUN_RESULT(ret);

        /* 处理ANR号码 */
        if (0xFF == g_pbReqUnit.spbReq.anrContent[i][0]) {
            g_pbReqUnit.spbReq.anrContent[i][0] = 0;
        }

        g_pbReqUnit.spbReq.anrContent[i][2] = record->additionNumber[i].numberType;

        numberLen = (record->additionNumber[i].numberLength > SI_PB_NUM_LEN)
                      ? SI_PB_NUM_LEN
                      : record->additionNumber[i].numberLength;

        SI_PB_AsciiToBcd(record->additionNumber[i].number, numberLen, &g_pbReqUnit.spbReq.anrContent[i][3],
                         sizeof(g_pbReqUnit.spbReq.anrContent[i]) - 3, &tempCopyLen);

        g_pbReqUnit.spbReq.anrContent[i][1] = tempCopyLen + 1;

        /* 检查当前更新的记录是否已经占用了一个EXT文件记录，并且EXT的记录号不能越界 */

        extRecord = g_anrContent[i].content[eXTContentOffset];
        extRecord = ((extRecord <= g_extContent[PB_ADN_CONTENT].extTotalNum) ? (extRecord) : 0xFF);

        para.index     = i;
        para.extRecord = extRecord;

        /* 处理ANR的EXT文件 */
        result = SI_PB_UpdateAnrExt(&para, record, spbReq, extUsedNum, pBFid);
        if (result != VOS_OK) {
            return result;
        }

        /* 记录待更新的ANR记录信息 */
        spbReq->fileId[*fileId]    = pBFid->anrFileId[i];
        spbReq->recordNum[*fileId] = freeANRRecord;
        spbReq->dataLen[*fileId]   = g_anrContent[i].recordLen;
        spbReq->content[*fileId]    = g_pbReqUnit.spbReq.anrContent[i];
        (*fileId)++;
    }

    return VOS_OK;
}

/*
 * 功能描述:更新Type2的email的文件内容
 * 修改历史:
 * 1. 日    期: 2019年05月27日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PB_UpdateType2EML(const SI_PB_Record *record, SI_PB_Spbdata *spbReq, const SI_PB_SpbUpdate *pbFid)
{
    VOS_UINT32  type2EMLOffset;
    VOS_UINT16 *fileNum = VOS_NULL_PTR;
    VOS_UINT8   adnSfi;
    VOS_UINT8   freeEMLRecord;
    errno_t     ret;

    if (g_iapContent.iapContent == VOS_NULL_PTR) {
        PB_WARNING_LOG("SI_PB_UpdateType2EML:IAP Content NULL");
        return TAF_ERR_PB_STORAGE_OP_FAIL;
    }

    type2EMLOffset = ((record->index - 1) * g_iapContent.recordLen) + (g_pbCtrlInfo.emlInfo[0].emlTagNum - 1);

    /* 规避因IAP文件内容不合协议导致的异常 */
    if (g_iapContent.iapContent[type2EMLOffset] == 0) {
        g_iapContent.iapContent[type2EMLOffset] = 0xFF;
    }

    fileNum = &spbReq->fileNum;

    if ((g_iapContent.iapContent[type2EMLOffset] == 0xFF) && (record->email.emailLen != 0)) {
        if (VOS_OK != SI_PB_GetFreeEMLRecordNum(pbFid->emlFileId, &freeEMLRecord, &spbReq->fileId[*fileNum])) {
            PB_WARNING_LOG("SI_PB_UpdateType2EML:No Free email Record");
            return TAF_ERR_PB_STORAGE_OP_FAIL;
        }

        /* 将缓存中的IAP文件 */
        if (sizeof(g_pbReqUnit.spbReq.iapContent) < g_iapContent.recordLen) {
            PB_ERROR_LOG("SI_PB_UpdateType2EML:IAP recordLen too long");

            return TAF_ERR_PB_STORAGE_OP_FAIL;
        }

        ret = memcpy_s(g_pbReqUnit.spbReq.iapContent, sizeof(g_pbReqUnit.spbReq.iapContent),
                       &g_iapContent.iapContent[(record->index - 1) * g_iapContent.recordLen],
                       g_iapContent.recordLen);

        PAM_PRINT_SECFUN_RESULT(ret);

        g_pbReqUnit.spbReq.iapContent[g_pbCtrlInfo.emlInfo[0].emlTagNum - 1] = freeEMLRecord;

        /* 避免后面ANR也为Type2类型，导致拷贝缓存时内容不正确，及时更新IAP缓存， */
        g_iapContent.iapContent[type2EMLOffset] = freeEMLRecord;

        spbReq->recordNum[*fileNum] = pbFid->recordNum;
        spbReq->dataLen[*fileNum]   = g_iapContent.recordLen;
        spbReq->content[*fileNum]    = g_pbReqUnit.spbReq.iapContent;

        (*fileNum)++;
    } else if ((g_iapContent.iapContent[type2EMLOffset] == 0xFF) && (record->email.emailLen == 0)) {
        PB_INFO1_LOG("SI_PB_UpdateType2EML:The email is Null or email Len is %d", (long)record->email.emailLen);
        return VOS_OK;
    }
    /* Email填为空，需要删除IAP中对应关系 */
    else if ((g_iapContent.iapContent[type2EMLOffset] != 0xFF) && (record->email.emailLen == 0)) {
        freeEMLRecord = g_iapContent.iapContent[type2EMLOffset];

        /* 将缓存中的IAP文件 */
        if (sizeof(g_pbReqUnit.spbReq.iapContent) < g_iapContent.recordLen) {
            PB_ERROR_LOG("SI_PB_UpdateType2EML:IAP recordLen too long");
            return TAF_ERR_PB_STORAGE_OP_FAIL;
        }

        ret = memcpy_s(g_pbReqUnit.spbReq.iapContent, sizeof(g_pbReqUnit.spbReq.iapContent),
                       &g_iapContent.iapContent[(record->index - 1) * g_iapContent.recordLen],
                       g_iapContent.recordLen);

        PAM_PRINT_SECFUN_RESULT(ret);

        SI_PB_GetIAPFidFromEML(pbFid->emlFileId, &spbReq->fileId[*fileNum]);

        g_pbReqUnit.spbReq.iapContent[g_pbCtrlInfo.emlInfo[0].emlTagNum - 1] = 0xFF;

        /* 避免后面ANR也为Type2类型，导致拷贝缓存时内容不正确，及时更新IAP缓存， */
        g_iapContent.iapContent[type2EMLOffset] = 0xFF;

        spbReq->recordNum[*fileNum] = pbFid->recordNum;
        spbReq->dataLen[*fileNum]   = g_iapContent.recordLen;
        spbReq->content[*fileNum]    = g_pbReqUnit.spbReq.iapContent;

        (*fileNum)++;
    } else {
        freeEMLRecord = g_iapContent.iapContent[type2EMLOffset];
    }

    spbReq->recordNum[*fileNum] = freeEMLRecord;

    if (SI_PB_GetADNSfi(&adnSfi, pbFid->adnFileId) != VOS_OK) {
        /* 出错处理 */
        PB_ERROR_LOG("SI_PB_UpdateType2EML:IAP recordLen too long");
        return TAF_ERR_PB_STORAGE_OP_FAIL;
    }

    /* 非删除请求 */
    if (0 != record->email.emailLen) {
        g_pbReqUnit.spbReq.emlContent[g_emlContent.recordLen - 2] = adnSfi; /* ADN SFI */
        g_pbReqUnit.spbReq.emlContent[g_emlContent.recordLen - 1] = pbFid->recordNum;
    }

    return (VOS_UINT32)TAF_ERR_CODE_BUTT;
}

/*
 * 功能描述:更新Email的文件内容
 * 修改历史:
 * 1. 日    期: 2009年06月08日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PB_UpdataEML(SI_PB_Record *record, SI_PB_Spbdata *spbReq, const SI_PB_SpbUpdate *pBFid)
{
    VOS_UINT16 *fileId = VOS_NULL_PTR;
    VOS_UINT8   tempCopyLen;
    VOS_UINT32  copyLen;
    VOS_UINT32  result;
    errno_t     ret;

    /* Email文件更新 */
    if (0 == pBFid->emlFileId) {
        if (0 == record->email.emailLen) {
            PB_NORMAL_LOG("SI_PB_UpdataEML:Needn't to Update EML");
            return VOS_OK;
        }

        PB_WARNING_LOG("SI_PB_UpdataEML:No email storage");
        return TAF_ERR_PB_STORAGE_OP_FAIL;
    }

    fileId = &spbReq->fileNum;

    if (VOS_NULL_PTR == g_emlContent.content) {
        PB_WARNING_LOG("SI_PB_UpdataEML:email Content NULL");
        return TAF_ERR_PB_STORAGE_OP_FAIL;
    }

    copyLen = PAM_GetMin(g_emlContent.recordLen, SI_PB_EMAIL_MAX_LEN);

    ret = memset_s(g_pbReqUnit.spbReq.emlContent, SI_PB_EMAIL_MAX_LEN, 0xFF, copyLen);

    PAM_PRINT_SECFUN_RESULT(ret);

    if (PB_FILE_TYPE2 == g_pbCtrlInfo.emlInfo[0].emlType) { /* Type2 */
        result = SI_PB_UpdateType2EML(record, spbReq, pBFid);
        /* 结果为VOS_OK是需要进行返回操作 */
        if ((VOS_UINT32)TAF_ERR_CODE_BUTT != result) {
            return result;
        }
    } else {
        spbReq->recordNum[*fileId] = pBFid->recordNum;
    }

    tempCopyLen = ((VOS_UINT8)record->email.emailLen > g_emlContent.dataLen) ? g_emlContent.dataLen
                : (VOS_UINT8)record->email.emailLen;

    if (VOS_NULL != tempCopyLen) {
        if (SI_PB_ALPHATAG_TRANSFER_TAG == record->email.email[tempCopyLen - 1]) {
            record->email.email[tempCopyLen - 1] = 0xFF;
        }

        tempCopyLen = (VOS_UINT8)PAM_GetMin(tempCopyLen, sizeof(g_pbReqUnit.spbReq.emlContent));

        ret = memcpy_s(g_pbReqUnit.spbReq.emlContent, sizeof(g_pbReqUnit.spbReq.emlContent),
                       record->email.email, tempCopyLen);
        PAM_PRINT_SECFUN_RESULT(ret);
    }

    spbReq->dataLen[*fileId] = g_emlContent.recordLen;
    spbReq->fileId[*fileId]  = pBFid->emlFileId;
    spbReq->content[*fileId]  = g_pbReqUnit.spbReq.emlContent;

    (*fileId)++;

    return VOS_OK;
}

/*
 * 功能描述:更新电话本的文件内容
 * 修改历史:
 * 1. 日    期: 2009年06月08日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PB_UpdataSPBFile(const SI_PB_SpbUpdate *pBFid, SI_PB_Record *record)
{
    VOS_UINT32         result     = VOS_OK;
    VOS_UINT16         extUsedNum = 1;
    SI_PB_Spbdata spbReq; /* 最多九个文件 */

    (VOS_VOID)memset_s((VOS_VOID *)(&spbReq), sizeof(SI_PB_Spbdata), 0, sizeof(SI_PB_Spbdata));

    /* 更新Email文件,根据NV项的值为判断是否可以更新 */
    if (g_pbConfigInfo.emailFlag) {
        result = SI_PB_UpdataEML(record, &spbReq, pBFid);
        if (VOS_OK != result) {
            PB_WARNING_LOG("SI_PB_UpdataSPBFile:Update email Fail");
            return result;
        }
    }

    /* 更新ANR文件 */
    result = SI_PB_UpdataANR(record, &spbReq, &extUsedNum, pBFid);
    if (VOS_OK != result) {
        PB_WARNING_LOG("SI_PB_UpdataSPBFile:Update ANR Fail");
        return result;
    }

    /* 更新ADN文件 */
    result = SI_PB_UpdataADN(record, &spbReq, &extUsedNum, pBFid);
    if (VOS_OK != result) {
        PB_WARNING_LOG("SI_PB_UpdataSPBFile:Update ADN Fail");
        return result;
    }

    /* 调用USIMM接口更新文件 */
    if (VOS_OK != SI_PB_SetSPBFileReq(&spbReq)) {
        PB_WARNING_LOG("SI_PB_UpdataSPBFile: SI_PB_SetSPBFileReq Fail");
        return TAF_ERR_USIM_SIM_CARD_NOTEXIST; /* SIM Fail */
    }

    return VOS_OK;
}

/*
 * 功能描述: 更新电话本的文件内容
 * 修改历史:
 * 1.日    期: 2009年06月08日
 *   修改内容: Creat
 */
VOS_VOID SI_PB_DeleteHandleAnr(VOS_UINT16 fileId, VOS_UINT16 anrIndex, SI_PB_Spbdata *spbReq,
                               VOS_UINT8 record)
{
    VOS_UINT32 i;
    VOS_UINT8  extRecord;
    VOS_UINT8  aNRRecord;
    VOS_UINT16 anrFileId = 0;
    VOS_UINT32 recordOffset;
    VOS_UINT32 type2ANRFileCnt;
    VOS_UINT32 xDNContentOffset;
    VOS_UINT8  content[20] = {0}; /* 删除的内容添全F */
    VOS_UINT32 copyLen;
    errno_t    ret;
    VOS_UINT32 type2ANROffset;

    (VOS_VOID)memset_s(content, sizeof(content), 0xFF, sizeof(content)); /* 初始化局部变量 */

    for (i = 0; i < SI_PB_ANR_MAX; i++) {
        /* 根据ADN FID和ANR电话本编号获取需要删除的ANR电话本FID */
        (VOS_VOID)SI_PB_GetANRFidFromADN((VOS_UINT8)i, fileId, &anrFileId);

        if (0 == anrFileId) { /* 该ANR电话本不存在 */
            continue;
        }

        /* Type2 类型 ANR处理 */
        if (PB_FILE_TYPE2 == g_pbCtrlInfo.anrInfo[0][i].anrType) {
            type2ANROffset = ((anrIndex - 1) * g_iapContent.recordLen) +
                             (g_pbCtrlInfo.anrInfo[0][i].anrTagNum - 1);

            if ((g_iapContent.iapContent == VOS_NULL_PTR) || (type2ANROffset >= g_iapContent.contentSize)) {
                continue;
            }

            aNRRecord = g_iapContent.iapContent[type2ANROffset];

            /* 该条ANR无效，删除下一条 */
            if (0xFF == aNRRecord) {
                continue;
            }

            copyLen = PAM_GetMin(g_iapContent.recordLen, sizeof(g_pbReqUnit.spbReq.iapContent));

            ret = memcpy_s(g_pbReqUnit.spbReq.iapContent, sizeof(g_pbReqUnit.spbReq.iapContent),
                           &g_iapContent.iapContent[(anrIndex - 1) * g_iapContent.recordLen], copyLen);

            PAM_PRINT_SECFUN_RESULT(ret);

            /* 将缓存中的IAP文件 */
            g_pbReqUnit.spbReq.iapContent[g_pbCtrlInfo.anrInfo[0][i].anrTagNum - 1] = 0xFF;

            /* 及时更新缓存，防止Email也为Type2类型，从而导致拷贝的缓存内容不正确 */
            g_iapContent.iapContent[((anrIndex - 1) * g_iapContent.recordLen) +
                                                     (g_pbCtrlInfo.anrInfo[0][i].anrTagNum - 1)] = 0xFF;

            (VOS_VOID)SI_PB_GetIAPFidFromANR(anrFileId, &spbReq->fileId[spbReq->fileNum]);

            spbReq->recordNum[spbReq->fileNum] = record;
            spbReq->dataLen[spbReq->fileNum]   = g_iapContent.recordLen;
            spbReq->content[spbReq->fileNum]    = g_pbReqUnit.spbReq.iapContent;

            spbReq->fileNum++;

            /* 处理ANR的EXT号码 */
            if (0 == g_pbCtrlInfo.adnInfo[0].recordNum) {
                type2ANRFileCnt = 0;
            } else {
                type2ANRFileCnt = (anrIndex - 1) / g_pbCtrlInfo.adnInfo[0].recordNum;
            }

            recordOffset =
                (type2ANRFileCnt * (g_pbCtrlInfo.anrInfo[0][i].recordNum * g_anrContent[i].recordLen)) +
                ((aNRRecord - 1) * g_anrContent[i].recordLen);

            xDNContentOffset = g_anrContent[i].recordLen - 3;
        } else {
            aNRRecord = record;

            /* 处理ANR的EXT号码 */
            recordOffset = (anrIndex - 1) * g_anrContent[i].recordLen;

            xDNContentOffset = g_anrContent[i].recordLen - 1;
        }

        /* 判断ANR文件内容是否为空 */
        if (VOS_NULL_PTR == g_anrContent[i].content) {
            continue;
        }

        /* 检查当前更新的记录是否已经占用了一个EXT文件记录 */
        extRecord = g_anrContent[i].content[recordOffset + xDNContentOffset];

        /* 如果待删除的ANR号码存在扩展号码，可能需要将EXT号码删除，且EXT的记录号不能越界，超过EXT的最大值 */
        if ((extRecord <= g_extContent[PB_ADN_CONTENT].extTotalNum) && (0xFF != extRecord)) {
            ret = memcpy_s(g_pbReqUnit.spbReq.anrExtContent[i], sizeof(g_pbReqUnit.spbReq.anrExtContent[i]),
                           content, SI_PB_EXT_LEN);

            PAM_PRINT_SECFUN_RESULT(ret);

            g_pbReqUnit.spbReq.anrExtContent[i][0] = 0;

            g_pbReqUnit.spbReq.anrExtIndex[i] = extRecord;

            spbReq->fileId[spbReq->fileNum]    = g_extContent[PB_ADN_CONTENT].extFileId;
            spbReq->dataLen[spbReq->fileNum]   = SI_PB_EXT_LEN;
            spbReq->recordNum[spbReq->fileNum] = extRecord;
            spbReq->content[spbReq->fileNum]    = g_pbReqUnit.spbReq.anrExtContent[i];
            spbReq->fileNum++;
        }

        /* ANR记录有效，则加入待删除列表 */
        if (VOS_OK == SI_PB_CheckANRValidity(g_anrContent[i].content[recordOffset],
                                             g_anrContent[i].content[recordOffset + 1])) {
            copyLen = PAM_GetMin(g_anrContent[i].recordLen, sizeof(g_pbReqUnit.spbReq.anrContent[i]));

            ret = memcpy_s(g_pbReqUnit.spbReq.anrContent[i], sizeof(g_pbReqUnit.spbReq.anrContent[i]),
                           content, copyLen);

            PAM_PRINT_SECFUN_RESULT(ret);

            /* 记录待删除的ANR记录信息 */
            spbReq->fileId[spbReq->fileNum]    = anrFileId;
            spbReq->recordNum[spbReq->fileNum] = aNRRecord;
            spbReq->dataLen[spbReq->fileNum]   = g_anrContent[i].recordLen;
            spbReq->content[spbReq->fileNum]    = g_pbReqUnit.spbReq.anrContent[i];
            spbReq->fileNum++;
        }
    }
}

/*
 * 功能描述: 更新电话本的文件内容
 * 修改历史:
 * 1.日    期: 2009年06月08日
 *   修改内容: Creat
 */

VOS_VOID SI_PB_DeleteHandleEmail(VOS_UINT16 emlIndex, SI_PB_Spbdata *spbReq, VOS_UINT8 record,
                                 VOS_UINT16 fileId)
{
    VOS_UINT16 emlFileId = 0;
    VOS_UINT16 iAPFileid = 0;
    VOS_UINT32 type2EMLIndex;
    VOS_UINT8  emlRecord;
    VOS_UINT8 *emlContent                   = VOS_NULL_PTR;
    VOS_UINT8  content[SI_PB_EMAIL_MAX_LEN] = {0}; /* 删除的内容添全F */
    VOS_UINT32 freeSize                      = 0;
    VOS_UINT32 copyLen;
    VOS_UINT32 offset;
    errno_t    ret;

    (VOS_VOID)memset_s(content, sizeof(content), 0xFF, sizeof(content)); /* 初始化局部变量 */

    (VOS_VOID)SI_PB_GetEMLFIdFromADN(&emlFileId, fileId);

    if (0 == emlFileId) {
        PB_NORMAL_LOG("SI_PB_DeleteHandleEmail:No email");
        return;
    }

    if (PB_FILE_TYPE2 == g_pbCtrlInfo.emlInfo[0].emlType) {
        type2EMLIndex = (emlIndex - 1) * g_iapContent.recordLen;

        offset = type2EMLIndex + (g_pbCtrlInfo.emlInfo[0].emlTagNum - 1);

        if ((g_iapContent.iapContent == VOS_NULL_PTR) || (g_iapContent.contentSize <= offset)) {
            PB_ERROR_LOG("SI_PB_DeleteHandleEmail: IAP content is null");

            return;
        }

        emlRecord = g_iapContent.iapContent[offset];

        if ((0xFF == emlRecord) || (0x00 == emlRecord)) {
            return;
        }

        /* 放在Refresh MEM中做 */
        if (sizeof(g_pbReqUnit.spbReq.iapContent) < g_iapContent.recordLen) {
            PB_ERROR_LOG("SI_PB_DeleteHandleEmail:IAP recordLen too long");

            return;
        }

        ret = memcpy_s(g_pbReqUnit.spbReq.iapContent, sizeof(g_pbReqUnit.spbReq.iapContent),
                       &g_iapContent.iapContent[type2EMLIndex], g_iapContent.recordLen);

        PAM_PRINT_SECFUN_RESULT(ret);

        g_pbReqUnit.spbReq.iapContent[g_pbCtrlInfo.emlInfo[0].emlTagNum - 1] = 0xFF;

        /* 及时更新缓存，防止Email和ANR删除顺序变化后，拷贝的缓存内容不正确 */
        g_iapContent.iapContent[type2EMLIndex + (g_pbCtrlInfo.emlInfo[0].emlTagNum - 1)] = 0xFF;

        (VOS_VOID)SI_PB_GetIAPFidFromEML(emlFileId, &iAPFileid);

        spbReq->recordNum[spbReq->fileNum] = record;
        spbReq->fileId[spbReq->fileNum]    = iAPFileid;
        spbReq->dataLen[spbReq->fileNum]   = g_iapContent.recordLen;
        spbReq->content[spbReq->fileNum]    = g_pbReqUnit.spbReq.iapContent;

        spbReq->fileNum++;
    } else {
        emlRecord = record;
    }

    if (VOS_OK != SI_PB_GetEMLRecord(&emlContent, emlFileId, emlRecord, &freeSize)) {
        PB_NORMAL_LOG("SI_PB_DeleteHandleEmail:Search email Storge Fail");
        return;
    }

    if (0xFF != emlContent[0]) { /* 有效Email */
        copyLen = PAM_GetMin(g_emlContent.recordLen, SI_PB_EMAIL_MAX_LEN);

        ret = memcpy_s(g_pbReqUnit.spbReq.emlContent, sizeof(g_pbReqUnit.spbReq.emlContent), content,
                       copyLen);

        PAM_PRINT_SECFUN_RESULT(ret);

        /* 记录待删除的EML记录信息 */
        spbReq->fileId[spbReq->fileNum]    = emlFileId;
        spbReq->recordNum[spbReq->fileNum] = emlRecord;
        spbReq->dataLen[spbReq->fileNum]   = g_emlContent.recordLen;
        spbReq->content[spbReq->fileNum]    = g_pbReqUnit.spbReq.emlContent;
        spbReq->fileNum++;
    }

    return;
}

/*
 * 功能描述:删除电话本的文件内容
 * 修改历史:
 * 1. 日    期: 2009年06月08日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PB_DeleteHandle(VOS_UINT16 fileId, VOS_UINT8 record, VOS_UINT8 offset, VOS_UINT16 pbIndex)
{
    VOS_UINT16         extFileId;
    VOS_UINT8          extRecord;
    VOS_UINT32         extInfoNum;
    VOS_UINT8         *recordAddr = VOS_NULL_PTR;
    VOS_UINT8          recordLen;
    VOS_UINT32         recordOffset;
    VOS_UINT32         result        = VOS_OK;
    SI_PB_Spbdata      spbReq        = {{0}}; /* 最多九个文件 */
    VOS_UINT8          content[242] = {0};   /* 删除的内容添全F */
    errno_t            ret;

    (VOS_VOID)memset_s(content, sizeof(content), 0xFF, sizeof(content)); /* 初始化局部变量 */

    (VOS_VOID)memset_s((VOS_VOID *)(&spbReq), sizeof(SI_PB_Spbdata), 0, sizeof(SI_PB_Spbdata));

    if (g_pbContent[offset].content == VOS_NULL_PTR) {
        PB_WARNING_LOG("SI_PB_DeleteHandle: content is NULL");

        return TAF_ERR_ERROR;
    }

    /* XDN 及 EXT处理 */
    recordLen    = g_pbContent[offset].recordLen;
    if (recordLen > sizeof(g_pbReqUnit.xdnContent)) {
        PB_WARNING1_LOG("SI_PB_DeleteHandle: The Record Len is too long.", recordLen);

        return TAF_ERR_ERROR;
    }

    recordOffset = (pbIndex - 1) * recordLen;
    recordAddr      = g_pbContent[offset].content + recordOffset;

    /* 存在复合电话本情况下，可能ADN为无效值 */
    if ((1 == g_pbConfigInfo.spbFlag) && (PB_ADN_CONTENT == offset)) {
        /* ANR 及 EXT处理 */
        SI_PB_DeleteHandleAnr(fileId, pbIndex, &spbReq, record);

        /* email 处理 */
        SI_PB_DeleteHandleEmail(pbIndex, &spbReq, record, fileId);

        result = SI_PB_CheckContentValidity(&g_pbContent[offset], recordAddr, g_pbContent[offset].recordLen);
    }

    extRecord  = ((offset == PB_BDN_CONTENT) ? recordAddr[recordLen - 2] : recordAddr[recordLen - 1]);
    extInfoNum = g_pbContent[offset].extInfoNum;
    extFileId  = g_extContent[extInfoNum].extFileId;

    if ((extRecord <= g_extContent[extInfoNum].extTotalNum) && (0xFF != extRecord) && (0 != extRecord) &&
        (0xFFFF != extFileId)) {
        g_pbReqUnit.extIndex = extRecord;

        ret = memcpy_s(g_pbReqUnit.extContent, sizeof(g_pbReqUnit.extContent), content, SI_PB_EXT_LEN);
        PAM_PRINT_SECFUN_RESULT(ret);

        g_pbReqUnit.extContent[0] = 0;

        spbReq.fileId[spbReq.fileNum]    = extFileId;
        spbReq.dataLen[spbReq.fileNum]   = SI_PB_EXT_LEN;
        spbReq.recordNum[spbReq.fileNum] = extRecord;
        spbReq.content[spbReq.fileNum]    = g_pbReqUnit.extContent;
        spbReq.fileNum++;
    }

    /* 在记录有效时才放入待删除文件列表 */
    if (VOS_OK == result) {
        recordLen = (VOS_UINT8)PAM_GetMin(recordLen, sizeof(g_pbReqUnit.xdnContent));

        ret = memcpy_s(g_pbReqUnit.xdnContent, sizeof(g_pbReqUnit.xdnContent), content, recordLen);
        PAM_PRINT_SECFUN_RESULT(ret);

        spbReq.fileId[spbReq.fileNum]    = fileId;
        spbReq.dataLen[spbReq.fileNum]   = recordLen;
        spbReq.recordNum[spbReq.fileNum] = record;
        spbReq.content[spbReq.fileNum]    = g_pbReqUnit.xdnContent;
        spbReq.fileNum++;
    }

    /* 调用USIMM接口更新文件 */
    if (VOS_OK != SI_PB_SetSPBFileReq(&spbReq)) {
        return TAF_ERR_USIM_SIM_CARD_NOTEXIST; /* SIM Fail */
    }

    return VOS_OK;
}

VOS_UINT32 SI_PB_QueryProc(PBMsgBlock *msg)
{
    SI_PB_QueryReq *queryMsg = VOS_NULL_PTR;
    SI_PB_EventInfo cnfData;
    VOS_UINT32            result;
    VOS_UINT8             pbOffset;
    VOS_UINT16            usedNum;

    if (msg->ulLength < (sizeof(SI_PB_QueryReq) - VOS_MSG_HEAD_LENGTH)) {
        PB_ERROR1_LOG("SI_PB_QueryProc: incorrect msg length", msg->ulLength);
        PB_ERROR1_LOG("SI_PB_QueryProc: incorrect send id", msg->ulSenderPid);

        return VOS_ERR;
    }

    queryMsg = (SI_PB_QueryReq *)msg;

    (VOS_VOID)memset_s(&cnfData, sizeof(SI_PB_EventInfo), 0, sizeof(SI_PB_EventInfo)); /* 初始化当前的局部变量 */

    result = SI_PB_FindPBOffset(queryMsg->storage, &pbOffset); /* 定位当前电话本位置 */

    cnfData.clientId    = queryMsg->client; /* 根据消息内容填充回复数据 */
    cnfData.opId        = queryMsg->opId;
    cnfData.pbEventType = SI_PB_EVENT_QUERY_CNF;
    cnfData.storage     = queryMsg->storage;

    if (TAF_ERR_NO_ERROR != result) {
        PB_WARNING_LOG("SI_PB_QUERYProc: Find the PhoneBook Content is Error");

        cnfData.pbError = TAF_ERR_PB_NOT_FOUND;

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    cnfData.pbError = SI_PB_ErrorProc();

    if (TAF_ERR_NO_ERROR != cnfData.pbError) { /* 如果定位失败或者缓冲内容不存在 */
        PB_WARNING_LOG("SI_PB_QUERYProc: Proc Error");

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    if (PB_INIT_FINISHED != g_pbInitState.pbInitStep) {
        cnfData.pbError = TAF_ERR_SIM_BUSY;

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    if (PB_INITIALISED != g_pbContent[pbOffset].initialState) {
        PB_WARNING_LOG("SI_PB_QueryProc: Not Init.");

        cnfData.pbError = TAF_ERR_SIM_BUSY;

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    g_pbReqUnit.pbLock = PB_LOCKED;

    if (PB_ADN_CONTENT == pbOffset) {
        usedNum = (VOS_UINT16)g_pbCtrlInfo.totalUsed;
    } else {
        usedNum = g_pbContent[pbOffset].usedNum;
    }

    cnfData.pbEvent.pbQueryCnf.inUsedNum    = usedNum;
    cnfData.pbEvent.pbQueryCnf.totalNum     = g_pbContent[pbOffset].totalNum;
    cnfData.pbEvent.pbQueryCnf.textLen      = g_pbContent[pbOffset].nameLen;
    cnfData.pbEvent.pbQueryCnf.numLen       = g_pbContent[pbOffset].numberLen;
    cnfData.pbEvent.pbQueryCnf.emailTextLen = g_emlContent.dataLen;
    cnfData.pbEvent.pbQueryCnf.anrNumberLen = g_anrContent[0].numberLen;

    return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
}

/*
 * 功能描述: 发送当前的电话本设置给Acpu
 * 修改历史:
 * 1.日    期: 2011年10月15日
 *   修改内容: Creat
 */
VOS_VOID SI_PB_SendSetCurPB(VOS_VOID)
{
    SI_PB_SetpbInd *msg = VOS_NULL_PTR;

    msg = (SI_PB_SetpbInd *)VOS_AllocMsg(MAPS_PB_PID, sizeof(SI_PB_SetpbInd) - VOS_MSG_HEAD_LENGTH);
    if (VOS_NULL_PTR == msg) { /* 申请内存失败会重启 */
        return;
    }

    msg->ulReceiverPid = ACPU_PID_PB;
    msg->msgName     = SI_PB_UPDATE_CURPB;
    msg->pbCurType   = g_pbCtrlInfo.pbCurType; /* 当前全局变量内容已经更新 */

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_OK != VOS_SendMsg(MAPS_PB_PID, msg)) {
        PB_ERROR_LOG("SI_PB_SendSetCurPB: Send msg Error");
    }

    return;
}

VOS_UINT32 SI_PB_SetProc(PBMsgBlock *msg)
{
    SI_PB_SetReq   *setPbMsg = VOS_NULL_PTR;
    SI_PB_EventInfo cnfData;
    VOS_UINT32            result;
    VOS_UINT8             pbOffset;

    if (msg->ulLength < (sizeof(SI_PB_SetReq) - VOS_MSG_HEAD_LENGTH)) {
        PB_ERROR1_LOG("SI_PB_SetProc: incorrect msg length", msg->ulLength);
        PB_ERROR1_LOG("SI_PB_SetProc: incorrect send id", msg->ulSenderPid);

        return VOS_ERR;
    }

    setPbMsg = (SI_PB_SetReq *)msg;

    (VOS_VOID)memset_s(&cnfData, sizeof(SI_PB_EventInfo), 0, sizeof(SI_PB_EventInfo)); /* 初始化局部变量 */

    cnfData.clientId    = setPbMsg->client; /* 根据消息内容填充回复数据 */
    cnfData.opId        = setPbMsg->opId;
    cnfData.pbEventType = SI_PB_EVENT_SET_CNF;
    cnfData.storage     = setPbMsg->storage;

    result = SI_PB_FindPBOffset(cnfData.storage, &pbOffset);
    if (VOS_OK != result) {
        cnfData.pbError = TAF_ERR_PARA_ERROR;

        PB_WARNING_LOG("SI_PB_SetProc: Find the PhoneBook Content is Error");

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    cnfData.pbError = SI_PB_ErrorProc();

    if (TAF_ERR_NO_ERROR != cnfData.pbError) { /* 如果定位失败或者缓冲内容不存在 */
        PB_WARNING_LOG("SI_PB_SetProc: Proc Error");

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    if (PB_INIT_FINISHED != g_pbInitState.pbInitStep) {
        cnfData.pbError = TAF_ERR_SIM_BUSY;

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    if (PB_INITIALISED != g_pbContent[pbOffset].initialState) {
        PB_WARNING_LOG("SI_PB_SetProc: Not Init.");

        cnfData.pbError = TAF_ERR_SIM_BUSY;

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    g_pbReqUnit.pbLock = PB_LOCKED;

    cnfData.pbEvent.pbSetCnf.inUsedNum      = g_pbContent[pbOffset].usedNum;
    cnfData.pbEvent.pbSetCnf.totalNum       = g_pbContent[pbOffset].totalNum;
    cnfData.pbEvent.pbSetCnf.textLen        = g_pbContent[pbOffset].nameLen;
    cnfData.pbEvent.pbSetCnf.numLen         = g_pbContent[pbOffset].numberLen;
    cnfData.pbEvent.pbSetCnf.anrNumberLen   = g_anrContent[0].numberLen;
    cnfData.pbEvent.pbQueryCnf.emailTextLen = g_emlContent.dataLen;

    g_pbCtrlInfo.pbCurType = setPbMsg->storage; /* 设置当前全局变量内容 */

#if (FEATURE_ACORE_MODULE_TO_CCORE != FEATURE_ON)
    SI_PB_SendSetCurPB();
#endif

    return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
}

/*
 * 功能描述: 找到电话本空闲条目
 * 修改历史:
 * 1. 日    期: 2009年06月15日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PB_FindFreeIndex(VOS_UINT8 pbOffset, VOS_UINT16 *pbIndex)
{
    VOS_UINT32 i;

    for (i = 1; i <= g_pbContent[pbOffset].totalNum; i++) { /* 根据当前的总索引查询空记录 */
        if (SI_PB_CONTENT_INVALID ==
            SI_PB_GetBitFromBuf(g_pbContent[pbOffset].index, g_pbContent[pbOffset].indexSize, i)) {
            break;
        }
    }

    if (i > g_pbContent[pbOffset].totalNum) { /* 内容全满 */
        return TAF_ERR_PB_STORAGE_FULL;             /* 调用回调函数 */
    }

    *pbIndex = (VOS_UINT16)i;

    return VOS_OK;
}

VOS_UINT32 SI_PB_AddProc(PBMsgBlock *msg)
{
    SI_PB_AddRep   *pbAddMsg = VOS_NULL_PTR;
    SI_PB_EventInfo cnfData;
    VOS_UINT32            result;
    VOS_UINT16            fileId;
    VOS_UINT16            extFileId = 0xFFFF;
    VOS_UINT8             record;
    VOS_UINT8             pbOffset;
    VOS_UINT16            i;
    VOS_UINT32            extInfoNum;

    if (msg->ulLength < (sizeof(SI_PB_AddRep) - VOS_MSG_HEAD_LENGTH)) {
        PB_ERROR1_LOG("SI_PB_AddProc: incorrect msg length", msg->ulLength);
        PB_ERROR1_LOG("SI_PB_AddProc: incorrect send id", msg->ulSenderPid);

        return VOS_ERR;
    }

    pbAddMsg = (SI_PB_AddRep *)msg;

    (VOS_VOID)memset_s(&cnfData, sizeof(SI_PB_EventInfo), 0, sizeof(SI_PB_EventInfo)); /* 初始化局部变量 */

    cnfData.clientId    = pbAddMsg->client; /* 根据消息填充回复数据 */
    cnfData.opId        = pbAddMsg->opId;
    cnfData.pbEventType = SI_PB_EVENT_ADD_CNF;
    cnfData.storage     = pbAddMsg->storage;

    cnfData.pbError = SI_PB_ErrorProc();

    if ((TAF_ERR_NO_ERROR == cnfData.pbError) && (PB_INIT_FINISHED != g_pbInitState.pbInitStep)) {
        cnfData.pbError = TAF_ERR_SIM_BUSY;
    }

    /* 紧急呼叫号码不能更新 */
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    if ((PB_ECC == pbAddMsg->storage) || (PB_XECC == pbAddMsg->storage)) {
        cnfData.pbError = TAF_ERR_PB_STORAGE_OP_FAIL;
    }
#else
    if (PB_ECC == pbAddMsg->storage) {
        cnfData.pbError = TAF_ERR_PB_STORAGE_OP_FAIL;
    }
#endif

    if (TAF_ERR_NO_ERROR != cnfData.pbError) { /* 如果定位失败或者缓冲内容不存在 */
        PB_WARNING_LOG("SI_PB_ADDProc: Proc Error");

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    result = SI_PB_LocateRecord(pbAddMsg->storage, pbAddMsg->record.index, pbAddMsg->record.index, &pbOffset);
    if (VOS_OK != result) {
        PB_WARNING_LOG("SI_PB_ADDProc: Find the PhoneBook Content is Error");
        cnfData.pbError = result;

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    if (pbAddMsg->record.numberLength > g_pbContent[pbOffset].numberLen) {
        PB_WARNING_LOG("SI_PB_ADDProc: number Too Long");
        cnfData.pbError = TAF_ERR_PB_DIAL_STRING_TOO_LONG;

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    if (VOS_NULL_PTR == g_pbContent[pbOffset].content) { /* 定位缓冲失败 */
        PB_WARNING_LOG("SI_PB_ADDProc: Content is NULL");
        cnfData.pbError = TAF_ERR_UNSPECIFIED_ERROR;

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    g_pbReqUnit.pbLock = PB_LOCKED;

    /* 找到空闲记录 */
    result = SI_PB_FindFreeIndex(pbOffset, &i);
    if (VOS_OK != result) {
        cnfData.pbError = result;

        return SI_PBSendAtEventCnf(&cnfData);
    }

    if (SI_PB_STORAGE_SM == pbAddMsg->storage) { /* 如果是ADN需要转换为记录号 */
        result = SI_PB_CountADNRecordNum(i, &fileId, &record);
    } else { /* 访问其它电话本 */
        result = SI_PB_GetXDNFileID(pbAddMsg->storage, &fileId);

        record = (VOS_UINT8)i;
    }

    if (VOS_ERR == result) { /* 转换结果失败 */
        PB_WARNING_LOG("SI_PB_ADDProc: Get the XDN File ID and Record number is Error");

        cnfData.pbError = TAF_ERR_PARA_ERROR;

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    extInfoNum           = g_pbContent[pbOffset].extInfoNum;
    extFileId            = g_extContent[extInfoNum].extFileId;
    pbAddMsg->record.index = i;

    result = SI_PB_UpdataXDNFile(pbOffset, fileId, extFileId, record,
                                   &pbAddMsg->record); /* 调用更新文件API */
    if (VOS_OK != result) {
        PB_WARNING_LOG("SI_PB_ADDProc: SI_PB_UpdataXDNFile is Error");

        cnfData.pbError = TAF_ERR_USIM_SIM_CARD_NOTEXIST;

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    g_pbReqUnit.pbEventType = SI_PB_EVENT_ADD_CNF; /* 填充缓冲结构 */

    g_pbReqUnit.pbStoateType = pbAddMsg->storage;

    g_pbReqUnit.opId = pbAddMsg->opId;

    g_pbReqUnit.clientId = pbAddMsg->client;

    g_pbReqUnit.index1 = i;

    return VOS_OK;
}

/*
 * 功能描述: 根据AT输入确定需要更新的ANR电话本
 * 修改历史:
 * 1. 日    期: 2009年06月05日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PB_GetANRInfo(const SI_PB_AdditionNum *aNRNum, VOS_UINT16 aDNFid, VOS_UINT16 *anrFileId)
{
    VOS_UINT16 i;

    /* 第二电话本参数检查及FID获取 */
    for (i = 0; i < g_pbConfigInfo.anrMaxNum; i++) {
        if ((0 < aNRNum[i].numberLength) && (i >= g_pbCtrlInfo.anrStorageNum)) {
            PB_WARNING_LOG("SI_PB_GetANRInfo:Not Allowed");

            return TAF_ERR_PB_STORAGE_OP_FAIL; /* 调用回调函数 */
        }

        if (aNRNum[i].numberLength > g_anrContent[i].numberLen) {
            PB_WARNING_LOG("SI_PB_GetANRInfo:Para Wrong");

            return TAF_ERR_PB_DIAL_STRING_TOO_LONG;
        }

        anrFileId[i] = 0;

        (VOS_VOID)SI_PB_GetANRFidFromADN((VOS_UINT8)i, aDNFid, &anrFileId[i]); /* 根据ADN FID获取ANR FID */

        if ((0 == anrFileId[i]) && (0 != aNRNum[i].numberLength)) {
            PB_WARNING_LOG("SI_PB_GetANRInfo:No ANR");
            return TAF_ERR_PB_STORAGE_OP_FAIL;
        }
    }

    return VOS_OK;
}

/*
 * 功能描述: 对SAdd参数的检查
 * 修改历史:
 * 1. 日    期: 2019年05月30日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PB_CheckSAddPara(VOS_UINT32 storage)
{
    VOS_UINT32 errType;

    if ((g_pbConfigInfo.spbFlag == 0) || (storage != SI_PB_STORAGE_SM)) {
        /* 改为Operation not allowed */
        return TAF_ERR_CMD_TYPE_ERROR;
    }

    errType = SI_PB_ErrorProc();
    if ((errType == TAF_ERR_NO_ERROR) && (g_pbInitState.pbInitStep != PB_INIT_FINISHED)) {
        errType = TAF_ERR_SIM_BUSY;
    }

    return errType;
}

VOS_UINT32 SI_PB_SAddProc(PBMsgBlock *msg)
{
    SI_PB_AddRep   *reqMsg = VOS_NULL_PTR;
    SI_PB_EventInfo cnfData;
    VOS_UINT32            result;
    VOS_UINT8             record;
    VOS_UINT8             pbOffset;
    VOS_UINT16            pbIndex;
    VOS_UINT32            extInfoNum;
    SI_PB_SpbUpdate sPBFid;

    if (msg->ulLength < (sizeof(SI_PB_AddRep) - VOS_MSG_HEAD_LENGTH)) {
        PB_ERROR1_LOG("SI_PB_SAddProc: incorrect msg length", msg->ulLength);
        PB_ERROR1_LOG("SI_PB_SAddProc: incorrect send id", msg->ulSenderPid);

        return VOS_ERR;
    }

    (VOS_VOID)memset_s(&sPBFid, sizeof(sPBFid), 0, sizeof(sPBFid));

    reqMsg = (SI_PB_AddRep *)msg;

    (VOS_VOID)memset_s(&cnfData, sizeof(SI_PB_EventInfo), 0, sizeof(SI_PB_EventInfo)); /* 初始化局部变量 */

    cnfData.clientId    = reqMsg->client; /* 根据消息填充回复数据 */
    cnfData.opId        = reqMsg->opId;
    cnfData.pbEventType = SI_PB_EVENT_SADD_CNF;
    cnfData.storage     = reqMsg->storage;

    cnfData.pbError = SI_PB_CheckSAddPara(reqMsg->storage);

    if (TAF_ERR_NO_ERROR != cnfData.pbError) { /* 如果定位失败或者缓冲内容不存在 */
        PB_WARNING_LOG("SI_PB_SAddProc: SI_PB_CheckSAddPara Error");

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    result = SI_PB_LocateRecord(reqMsg->storage, reqMsg->record.index, reqMsg->record.index, &pbOffset);
    if (VOS_OK != result) {
        PB_WARNING_LOG("SI_PB_SAddProc: Find the PhoneBook Content is Error");
        cnfData.pbError = result;

        return SI_PBSendAtEventCnf(&cnfData);
    }

    if (reqMsg->record.numberLength > g_pbContent[pbOffset].numberLen) {
        PB_WARNING_LOG("SI_PB_SAddProc: number Too Long");
        cnfData.pbError = TAF_ERR_PB_DIAL_STRING_TOO_LONG;

        return SI_PBSendAtEventCnf(&cnfData);
    }

    if (VOS_NULL_PTR == g_pbContent[pbOffset].content) { /* 定位缓冲失败 */
        PB_WARNING_LOG("SI_PB_SAddProc: Content is NULL");
        cnfData.pbError = TAF_ERR_UNSPECIFIED_ERROR;

        return SI_PBSendAtEventCnf(&cnfData);
    }

    /* 找到空闲记录 */
    result = SI_PB_FindFreeIndex(pbOffset, &pbIndex);
    if (VOS_OK != result) {
        cnfData.pbError = result;

        return SI_PBSendAtEventCnf(&cnfData);
    }

    /* 记录需要更新的index号 */
    reqMsg->record.index = pbIndex;

    /* ADN电话本FID及记录号获取 */
    result = SI_PB_CountADNRecordNum(pbIndex, &sPBFid.adnFileId, &record);
    if (VOS_ERR == result) { /* 转换失败 */
        PB_WARNING_LOG("SI_PB_SAddProc: Get the XDN File ID and Record number is Error");

        cnfData.pbError = TAF_ERR_PARA_ERROR;

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    /* 第二电话本参数检查及ID获取 */
    result = SI_PB_GetANRInfo(reqMsg->record.additionNumber, sPBFid.adnFileId, sPBFid.anrFileId);
    if (VOS_OK != result) {
        PB_WARNING_LOG("SI_PB_SAddProc: Get the ANR Info Fail");

        cnfData.pbError = result;

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    /* email 文件FID获取,根据NV项的值为判断是否可以更新 */
    if (g_pbConfigInfo.emailFlag) {
        if (0 < reqMsg->record.email.emailLen) {
            (VOS_VOID)SI_PB_GetEMLFIdFromADN(&sPBFid.emlFileId, sPBFid.adnFileId);

            if (0 == sPBFid.emlFileId) {
                PB_WARNING_LOG("SI_PB_SModifyProc:No email");

                cnfData.pbError = TAF_ERR_PB_STORAGE_OP_FAIL;

                return SI_PBSendAtEventCnf(&cnfData);
            }
        }
    }

    /* EXT 文件FID获取 */
    extInfoNum         = g_pbContent[pbOffset].extInfoNum;
    sPBFid.extFileId = g_extContent[extInfoNum].extFileId;

    g_pbReqUnit.pbLock = PB_LOCKED;

    sPBFid.recordNum = record;

    result = SI_PB_UpdataSPBFile(&sPBFid, &reqMsg->record); /* 调用更新文件的API */
    if (VOS_OK != result) {
        PB_WARNING_LOG("SI_PB_SAddProc: SI_PB_UpdataXDNFile is Error");

        cnfData.pbError = result;

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    g_pbReqUnit.pbEventType = SI_PB_EVENT_ADD_CNF; /* 填充缓冲结构 */

    g_pbReqUnit.pbStoateType = reqMsg->storage;

    g_pbReqUnit.opId = reqMsg->opId;

    g_pbReqUnit.clientId = reqMsg->client;

    g_pbReqUnit.index1 = pbIndex;

    return VOS_OK;
}

VOS_UINT32 SI_PB_ModifyProc(PBMsgBlock *msg)
{
    SI_PB_AddRep   *repMsg = VOS_NULL_PTR;
    SI_PB_EventInfo cnfData;
    VOS_UINT32            result;
    VOS_UINT16            fileId;
    VOS_UINT16            extFileId = 0xFFFF;
    VOS_UINT8             record;
    VOS_UINT8             pbOffset;
    VOS_UINT32            extInfoNum;

    if (msg->ulLength < (sizeof(SI_PB_AddRep) - VOS_MSG_HEAD_LENGTH)) {
        PB_ERROR1_LOG("SI_PB_ModifyProc: incorrect msg length", msg->ulLength);
        PB_ERROR1_LOG("SI_PB_ModifyProc: incorrect send id", msg->ulSenderPid);

        return VOS_ERR;
    }

    repMsg = (SI_PB_AddRep *)msg;

    (VOS_VOID)memset_s(&cnfData, sizeof(SI_PB_EventInfo), 0, sizeof(SI_PB_EventInfo)); /* 初始化局部变量 */

    cnfData.clientId    = repMsg->client; /* 根据消息填充回复数据 */
    cnfData.opId        = repMsg->opId;
    cnfData.pbEventType = SI_PB_EVENT_MODIFY_CNF;
    cnfData.storage     = repMsg->storage;

    cnfData.pbError = SI_PB_ErrorProc();

    if ((TAF_ERR_NO_ERROR == cnfData.pbError) && (PB_INIT_FINISHED != g_pbInitState.pbInitStep)) {
        cnfData.pbError = TAF_ERR_SIM_BUSY;
    }

    /* 紧急呼叫号码不能更新 */
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    if ((PB_ECC == repMsg->storage) || (PB_XECC == repMsg->storage)) {
        cnfData.pbError = TAF_ERR_PB_STORAGE_OP_FAIL;
    }
#else
    if (PB_ECC == repMsg->storage) {
        cnfData.pbError = TAF_ERR_PB_STORAGE_OP_FAIL;
    }
#endif

    if (TAF_ERR_NO_ERROR != cnfData.pbError) { /* 如果定位失败或者缓冲内容不存在 */
        PB_WARNING_LOG("SI_PB_ModifyProc: Proc Error");

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    result = SI_PB_LocateRecord(repMsg->storage, repMsg->record.index, repMsg->record.index, &pbOffset);
    if (VOS_OK != result) { /* 定位电话本信息失败 */
        PB_WARNING_LOG("SI_PB_ModifyProc: Find the PhoneBook Content is Error");
        cnfData.pbError = result;

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    if (repMsg->record.numberLength > g_pbContent[pbOffset].numberLen) {
        PB_WARNING_LOG("SI_PB_ModifyProc: number Too Long");
        cnfData.pbError = TAF_ERR_PB_DIAL_STRING_TOO_LONG;

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    g_pbReqUnit.pbLock = PB_LOCKED;

    if (SI_PB_STORAGE_SM == repMsg->storage) { /* 访问的是ADN需要转换记录号 */
        result = SI_PB_CountADNRecordNum(repMsg->record.index, &fileId, &record);
    } else { /* 访问其它电话本 */
        result = SI_PB_GetXDNFileID(repMsg->storage, &fileId);

        record = (VOS_UINT8)repMsg->record.index;
    }

    if (VOS_ERR == result) { /* 转换失败 */
        PB_WARNING_LOG("SI_PB_ModifyProc: Get the XDN File ID and Record number is Error");

        cnfData.pbError = TAF_ERR_PARA_ERROR;

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    extInfoNum = g_pbContent[pbOffset].extInfoNum;
    extFileId  = g_extContent[extInfoNum].extFileId;

    result = SI_PB_UpdataXDNFile(pbOffset, fileId, extFileId, record, &repMsg->record); /* 调用更新文件的API */
    if (VOS_OK != result) {
        PB_WARNING_LOG("SI_PB_ModifyProc: SI_PB_UpdataXDNFile is Error");

        cnfData.pbError = TAF_ERR_USIM_SIM_CARD_NOTEXIST;

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    g_pbReqUnit.pbEventType = SI_PB_EVENT_MODIFY_CNF; /* 相关内容在全局变量中保存 */

    g_pbReqUnit.pbStoateType = repMsg->storage;

    g_pbReqUnit.opId = repMsg->opId;

    g_pbReqUnit.clientId = repMsg->client;

    g_pbReqUnit.index1 = repMsg->record.index;

    return VOS_OK;
}

VOS_UINT32 SI_PB_SModifyProc(PBMsgBlock *msg)
{
    SI_PB_AddRep   *repMsg = VOS_NULL_PTR;
    SI_PB_EventInfo cnfData;
    VOS_UINT32            result;
    VOS_UINT8             record;
    VOS_UINT8             pbOffset;
    VOS_UINT32            extInfoNum;
    SI_PB_SpbUpdate sPBFid;

    if (msg->ulLength < (sizeof(SI_PB_AddRep) - VOS_MSG_HEAD_LENGTH)) {
        PB_ERROR1_LOG("SI_PB_SModifyProc: incorrect msg length", msg->ulLength);
        PB_ERROR1_LOG("SI_PB_SModifyProc: incorrect send id", msg->ulSenderPid);

        return VOS_ERR;
    }

    (VOS_VOID)memset_s(&sPBFid, sizeof(sPBFid), 0, sizeof(sPBFid));

    repMsg = (SI_PB_AddRep *)msg;

    (VOS_VOID)memset_s(&cnfData, sizeof(SI_PB_EventInfo), 0, sizeof(SI_PB_EventInfo)); /* 初始化局部变量 */

    cnfData.clientId    = repMsg->client; /* 根据消息填充回复数据 */
    cnfData.opId        = repMsg->opId;
    cnfData.pbEventType = SI_PB_EVENT_SMODIFY_CNF;
    cnfData.storage     = repMsg->storage;

    if ((0 == g_pbConfigInfo.spbFlag) || (SI_PB_STORAGE_SM != repMsg->storage)) {
        /* 改为Operation not allowed */
        cnfData.pbError = TAF_ERR_CMD_TYPE_ERROR;

        return SI_PBSendAtEventCnf(&cnfData);
    }

    cnfData.pbError = SI_PB_ErrorProc();

    if ((TAF_ERR_NO_ERROR == cnfData.pbError) && (PB_INIT_FINISHED != g_pbInitState.pbInitStep)) {
        cnfData.pbError = TAF_ERR_SIM_BUSY;
    }

    if (TAF_ERR_NO_ERROR != cnfData.pbError) { /* 如果定位失败或者缓冲内容不存在 */
        PB_WARNING_LOG("SI_PB_SModifyProc: Proc Error");

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    result = SI_PB_LocateRecord(repMsg->storage, repMsg->record.index, repMsg->record.index, &pbOffset);
    if (VOS_OK != result) { /* 定位电话本信息失败 */
        PB_WARNING_LOG("SI_PB_SModifyProc: Find the PhoneBook Content is Error");
        cnfData.pbError = result;

        return SI_PBSendAtEventCnf(&cnfData);
    }

    /* ADN电话本号码长度检查 */
    if (repMsg->record.numberLength > g_pbContent[pbOffset].numberLen) {
        PB_WARNING_LOG("SI_PB_SModifyProc: number Too Long");
        cnfData.pbError = TAF_ERR_PB_DIAL_STRING_TOO_LONG;

        return SI_PBSendAtEventCnf(&cnfData);
    }

    if (VOS_NULL_PTR == g_pbContent[pbOffset].content) { /* 定位缓冲失败 */
        PB_WARNING_LOG("SI_PB_SModifyProc: Content is NULL");
        cnfData.pbError = TAF_ERR_UNSPECIFIED_ERROR;

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    /* ADN电话本FID及记录号获取 */
    result = SI_PB_CountADNRecordNum(repMsg->record.index, &sPBFid.adnFileId, &record);
    if (VOS_ERR == result) { /* 转换失败 */
        PB_WARNING_LOG("SI_PB_SModifyProc: Get the XDN File ID and Record number is Error");

        cnfData.pbError = TAF_ERR_PARA_ERROR;

        return SI_PBSendAtEventCnf(&cnfData);
    }

    /* 第二电话本参数检查及FID获取 */
    result = SI_PB_GetANRInfo(repMsg->record.additionNumber, sPBFid.adnFileId, sPBFid.anrFileId);
    if (VOS_OK != result) {
        PB_WARNING_LOG("SI_PB_SModifyProc: Get the ANR Info Fail");

        cnfData.pbError = result;

        return SI_PBSendAtEventCnf(&cnfData);
    }

    /* email 文件FID获取,不对返回值，用户更新的Email是否存在放在SI_PB_UpdataSPBFile中判断 */
    if (g_pbConfigInfo.emailFlag) {
        (VOS_VOID)SI_PB_GetEMLFIdFromADN(&sPBFid.emlFileId, sPBFid.adnFileId);
    } else {
        sPBFid.emlFileId = 0;
    }

    /* EXT文件FID获取 */
    extInfoNum         = g_pbContent[pbOffset].extInfoNum;
    sPBFid.extFileId = g_extContent[extInfoNum].extFileId;

    g_pbReqUnit.pbLock = PB_LOCKED;

    sPBFid.recordNum = record;

    result = SI_PB_UpdataSPBFile(&sPBFid, &repMsg->record); /* 调用更新文件的API */
    if (VOS_OK != result) {
        PB_WARNING_LOG("SI_PB_SModifyProc: SI_PB_UpdataSPBFile is Error");

        cnfData.pbError = result;

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    g_pbReqUnit.pbEventType = SI_PB_EVENT_MODIFY_CNF; /* 相关内容在全局变量中保存 */

    g_pbReqUnit.pbStoateType = repMsg->storage;

    g_pbReqUnit.opId = repMsg->opId;

    g_pbReqUnit.clientId = repMsg->client;

    g_pbReqUnit.index1 = repMsg->record.index;

    return VOS_OK;
}

VOS_UINT32 SI_PB_DeleteProc(PBMsgBlock *msg)
{
    SI_PB_DeleteReq *reqMsg = VOS_NULL_PTR;
    SI_PB_EventInfo  cnfData;
    VOS_UINT32             result;
    VOS_UINT16             fileId;
    VOS_UINT8              record;
    VOS_UINT8              pbOffset;
    VOS_UINT8              content[100]; /* 删除的内容添全F */

    if (msg->ulLength < (sizeof(SI_PB_DeleteReq) - VOS_MSG_HEAD_LENGTH)) {
        PB_ERROR1_LOG("SI_PB_DeleteProc: incorrect msg length", msg->ulLength);
        PB_ERROR1_LOG("SI_PB_DeleteProc: incorrect send id", msg->ulSenderPid);

        return VOS_ERR;
    }

    reqMsg = (SI_PB_DeleteReq *)msg;

    (VOS_VOID)memset_s(content, sizeof(content), 0xFF, sizeof(content)); /* 初始化局部变量 */

    (VOS_VOID)memset_s(&cnfData, sizeof(SI_PB_EventInfo), 0, sizeof(SI_PB_EventInfo)); /* 初始化局部变量 */

    cnfData.clientId    = reqMsg->client; /* 根据消息内容填充回复数据 */
    cnfData.opId        = reqMsg->opId;
    cnfData.pbEventType = SI_PB_EVENT_DELETE_CNF;
    cnfData.storage     = reqMsg->storage;

    cnfData.pbError = SI_PB_ErrorProc();

    if ((TAF_ERR_NO_ERROR == cnfData.pbError) && (PB_INIT_FINISHED != g_pbInitState.pbInitStep)) {
        cnfData.pbError = TAF_ERR_SIM_BUSY;
    }

    /* 紧急呼叫号码不能删除 */
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    if ((PB_ECC == reqMsg->storage) || (PB_XECC == reqMsg->storage)) {
        cnfData.pbError = TAF_ERR_PB_STORAGE_OP_FAIL;
    }
#else
    if (PB_ECC == reqMsg->storage) {
        cnfData.pbError = TAF_ERR_PB_STORAGE_OP_FAIL;
    }
#endif

    if (TAF_ERR_NO_ERROR != cnfData.pbError) { /* 如果定位失败或者缓冲内容不存在 */
        PB_WARNING_LOG("SI_PB_DeleteProc: Proc Error");

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    result = SI_PB_LocateRecord(reqMsg->storage, reqMsg->index, reqMsg->index, &pbOffset);
    if (VOS_OK != result) { /* 定位缓冲信息失败 */
        PB_WARNING_LOG("SI_PB_DeleteProc: Find the PhoneBook Content is Error");

        cnfData.pbError = result;

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    if (SI_PB_STORAGE_SM == reqMsg->storage) { /* 如果当前访问的是ADN需要转换记录号 */
        result = SI_PB_CountADNRecordNum(reqMsg->index, &fileId, &record);
    } else { /* 当前访问的是其它电话本 */
        result = SI_PB_GetXDNFileID(reqMsg->storage, &fileId);

        record = (VOS_UINT8)reqMsg->index;
    }

    if (VOS_ERR == result) { /* 转换错误 */
        PB_WARNING_LOG("SI_PB_DeleteProc: Get the XDN File ID and Record number is Error");

        cnfData.pbError = TAF_ERR_PARA_ERROR;

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    if (SI_PB_CONTENT_INVALID == SI_PB_GetBitFromBuf(g_pbContent[pbOffset].index,
                                                     g_pbContent[pbOffset].indexSize,
                                                     (VOS_UINT32)reqMsg->index)) {
        cnfData.pbError = TAF_ERR_NO_ERROR;

        return SI_PBSendAtEventCnf(&cnfData);
    }

    g_pbReqUnit.pbLock = PB_LOCKED;

    /* 调用函数删除，判断如果为ADN，需要删除复合电话本 */
    result = SI_PB_DeleteHandle(fileId, record, pbOffset, reqMsg->index);
    if (VOS_OK != result) {
        cnfData.pbError = result;

        return SI_PBSendAtEventCnf(&cnfData);
    }

    g_pbReqUnit.pbEventType = SI_PB_EVENT_DELETE_CNF; /* 相关内容保存 */

    g_pbReqUnit.pbStoateType = reqMsg->storage;

    g_pbReqUnit.opId = reqMsg->opId;

    g_pbReqUnit.clientId = reqMsg->client;

    g_pbReqUnit.index1 = reqMsg->index;

    return VOS_OK;
}

/*
 * 功能描述: Refresh请求的处理
 * 修改历史:
 * 1. 日    期: 2008年11月11日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PB_RefreshALLContent(VOS_VOID)
{
    VOS_UINT32 result = VOS_ERR;

    SI_PB_ClearPBContent(SI_CLEARALL);

    SI_PB_ClearSPBContent(SI_CLEARALL);

    SI_PB_InitGlobeVariable();

    SI_PB_InitStatusInd(USIMM_PB_IDLE);

    g_pbFileCnt = 0;

    if (VOS_OK != SI_PB_InitPBStatusJudge()) { /* 根据卡类型设定初始化步骤 */
        PB_ERROR_LOG("SI_PB_RefreshProc:Card Status Error");

        return VOS_ERR;
    }

    if (PB_INIT_EFPBR == g_pbInitState.pbInitStep) { /* USIM卡 */
        result = SI_PB_InitEFpbrReq();
    } else { /* SIM卡 */
        result = SI_PB_InitXDNSpaceReq();
    }

    return result;
}

/*
 * 功能描述: FDN号码检查消息回复
 * 修改历史:
 * 1.日    期: 2012年02月22日
 *   修改内容: Creat
 */
VOS_UINT32 SI_PB_FDNCheckProc(PBMsgBlock *msg)
{
    PB_FdnCheckCnf       *fdnCheckCnf = VOS_NULL_PTR;
    SI_PB_FdnCheckReq    *fdnCheckReq = VOS_NULL_PTR;
    VOS_UINT32                   result1      = VOS_OK;
    VOS_UINT32                   result2      = VOS_OK;
    VOS_UINT32                   usimFdnStatus;
    VOS_UINT32                   csimFdnStatus;
    SI_PIH_CardSlotUint32  slotId;

    if (msg->ulLength < (sizeof(SI_PB_FdnCheckReq) - VOS_MSG_HEAD_LENGTH)) {
        PB_ERROR1_LOG("SI_PB_FDNCheckProc: incorrect msg length", msg->ulLength);
        PB_ERROR1_LOG("SI_PB_FDNCheckProc: incorrect send id", msg->ulSenderPid);

        return VOS_ERR;
    }

    fdnCheckReq = (SI_PB_FdnCheckReq *)msg;

    fdnCheckCnf = (PB_FdnCheckCnf *)VOS_AllocMsg(MAPS_PB_PID,
                                                           sizeof(PB_FdnCheckCnf) - VOS_MSG_HEAD_LENGTH);
    if (VOS_NULL_PTR == fdnCheckCnf) {
        PB_ERROR_LOG("SI_PB_FDNCheckProc: alloc msg failed.");

        return VOS_ERR;
    }

    slotId = USIMM_GetSlotIdByModemId(MODEM_ID_0);

    /* 如果FDN未使能直接返回VOS_OK */
    USIMM_FdnQuery(slotId, &usimFdnStatus, &csimFdnStatus);

    fdnCheckCnf->ulReceiverPid  = fdnCheckReq->ulSenderPid;
    fdnCheckCnf->msgName      = SI_PB_EVENT_FDNCHECK_CNF;
    fdnCheckCnf->appType      = fdnCheckReq->appType;
    fdnCheckCnf->sendPara     = fdnCheckReq->sendPara;
    fdnCheckCnf->contextIndex = fdnCheckReq->contextIndex;

    if (USIMM_CDMA_APP == fdnCheckReq->appType) {
        if (USIMM_FDNSTATUS_OFF == csimFdnStatus) {
            PB_INFO_LOG("SI_PB_CheckFdn: CSIM FDN is disable.");
        } else {
            PB_WARNING_LOG("SI_PB_CheckFdn: CSIM FDN is Enable, Now return NO Check.");
        }
    }

    if (USIMM_GUTL_APP == fdnCheckReq->appType) {
        if (USIMM_FDNSTATUS_OFF == usimFdnStatus) {
            PB_INFO_LOG("SI_PB_CheckFdn: USIM FDN is disable.");
        } else {
            PB_INFO_LOG("SI_PB_CheckFdn: USIM FDN is Enable.");

            /* 检测第一个号码 */
            result1 = SI_PB_CheckFdn(fdnCheckReq->fdnNum.num1, fdnCheckReq->fdnNum.num1Len);

            /* 如果第二个号码长度大于0，检测第二个号码 */
            if (fdnCheckReq->fdnNum.num2Len > 0) {
                result2 = SI_PB_CheckFdn(fdnCheckReq->fdnNum.num2, fdnCheckReq->fdnNum.num2Len);
            }
        }
    }

    if ((VOS_OK != result1) && (VOS_OK != result2)) {
        fdnCheckCnf->result = PB_FDN_CHECK_BOTH_NUM_FAIL;
    } else if (VOS_OK != result1) {
        fdnCheckCnf->result = PB_FDN_CHECK_NUM1_FAIL;
    } else if (VOS_OK != result2) {
        fdnCheckCnf->result = PB_FDN_CHECK_NUM2_FAIL;
    } else {
        fdnCheckCnf->result = PB_FDN_CHECK_SUCC;
    }

    PAM_OM_ReportLayerMsg(fdnCheckCnf);
    return VOS_SendMsg(MAPS_PB_PID, fdnCheckCnf);
}

/*
 * 功能描述: ECALL号码获取请求消息
 * 修改历史:
 * 1.日    期: 2014年04月25日
 *   修改内容: Creat
 */
VOS_UINT32 SI_PB_EcallNumberQryProc(PBMsgBlock *msg)
{
    SI_PB_EcallQryReq *ecallQryReq = VOS_NULL_PTR;
    SI_PB_EcallQryCnf *ecallQryCnf = VOS_NULL_PTR;
    VOS_UINT32               rslt;
    VOS_UINT32               recordNum;
    VOS_UINT8                pbOffset;

    ecallQryReq = (SI_PB_EcallQryReq *)msg;

    if (msg->ulLength < (sizeof(SI_PB_EcallQryReq) - VOS_MSG_HEAD_LENGTH - sizeof(ecallQryReq->list))) {
        PB_ERROR1_LOG("SI_PB_EcallNumberQryProc: incorrect msg length", msg->ulLength);
        PB_ERROR1_LOG("SI_PB_EcallNumberQryProc: incorrect send id", msg->ulSenderPid);

        return VOS_ERR;
    }

    if (msg->ulLength < (sizeof(SI_PB_EcallQryReq) - VOS_MSG_HEAD_LENGTH - sizeof(ecallQryReq->list) + ecallQryReq->listLen)) {
        PB_ERROR1_LOG("SI_PB_EcallNumberQryProc: incorrect msg length", msg->ulLength);
        PB_ERROR1_LOG("SI_PB_EcallNumberQryProc: incorrect send id", msg->ulSenderPid);

        return VOS_ERR;
    }

    rslt = SI_PB_EcallNumberErrProc(ecallQryReq->storage, ecallQryReq->listLen, ecallQryReq->list,
                                      &pbOffset);

    /* 如果检查出错，返回只有结果。检查正确，按NAS要求的记录数返回FDN/SDN记录 */
    recordNum = (TAF_ERR_NO_ERROR == rslt) ? ecallQryReq->listLen : 0;

    /*lint -save -e961 */
    ecallQryCnf = (SI_PB_EcallQryCnf *)VOS_AllocMsg(MAPS_PB_PID,
                                                             sizeof(SI_PB_EcallQryCnf) - VOS_MSG_HEAD_LENGTH +
                                                             ((sizeof(SI_PB_EcallNum) * recordNum)));
    /*lint -restore */
    if (VOS_NULL_PTR == ecallQryCnf) {
        PB_ERROR_LOG("SI_PB_EcallNumberQryProc: alloc msg failed.");

        return VOS_ERR;
    }

    ecallQryCnf->ulReceiverPid = ecallQryReq->ulSenderPid;
    ecallQryCnf->msgName     = SI_PB_EVENT_ECALLQUERY_CNF;
    ecallQryCnf->storage     = ecallQryReq->storage;
    ecallQryCnf->recordNum   = VOS_NULL;
    ecallQryCnf->rslt        = rslt;
    ecallQryCnf->appType     = USIMM_GUTL_APP;

    if (TAF_ERR_NO_ERROR == rslt) {
        ecallQryCnf->recordNum = ecallQryReq->listLen;

        if (recordNum > 0) {
            (VOS_VOID)memset_s(ecallQryCnf->eCallNumber, sizeof(SI_PB_EcallNum) * recordNum, 0,
                               sizeof(SI_PB_EcallNum) * recordNum);
        }

        SI_PB_GetEcallNumber(g_pbContent + pbOffset, ecallQryCnf->eCallNumber, ecallQryReq->listLen,
                             ecallQryReq->list);
    }

    PAM_OM_ReportLayerMsg(ecallQryCnf);
    return VOS_SendMsg(MAPS_PB_PID, ecallQryCnf);
}

/*
 * 功能描述: 根据电话本的更新接
 * 修改历史:
 * 1. 日    期: 2009年06月05日
 *    修改内容: Creat
 */
VOS_VOID SI_PB_JudgeTotalUsedNum(VOS_UINT8 pbOffset, SI_PB_EventInfo *cnfData)
{
    if ((SI_PB_EVENT_ADD_CNF == g_pbReqUnit.pbEventType) || (SI_PB_EVENT_SADD_CNF == g_pbReqUnit.pbEventType)) {
        if (PB_ADN_CONTENT == pbOffset) {
            g_pbCtrlInfo.totalUsed++;
        }

        cnfData->pbEvent.pbAddCnf.index = g_pbReqUnit.index1;

        SI_PB_SetBitToBuf(g_pbContent[pbOffset].index, g_pbContent[pbOffset].indexSize, g_pbReqUnit.index1,
                          SI_PB_CONTENT_VALID);
    } else if (SI_PB_EVENT_DELETE_CNF == g_pbReqUnit.pbEventType) {
        if (PB_ADN_CONTENT == pbOffset) {
            g_pbCtrlInfo.totalUsed--;
        }

        SI_PB_SetBitToBuf(g_pbContent[pbOffset].index, g_pbContent[pbOffset].indexSize, g_pbReqUnit.index1,
                          SI_PB_CONTENT_INVALID);

        (VOS_VOID)memset_s(g_pbReqUnit.xdnContent, sizeof(g_pbReqUnit.xdnContent), 0xFF,
                           sizeof(g_pbReqUnit.xdnContent));
    } else if ((SI_PB_EVENT_MODIFY_CNF == g_pbReqUnit.pbEventType) ||
               (SI_PB_EVENT_SMODIFY_CNF == g_pbReqUnit.pbEventType)) {
        if (SI_PB_CONTENT_INVALID == SI_PB_GetBitFromBuf(g_pbContent[pbOffset].index,
                                                         g_pbContent[pbOffset].indexSize, g_pbReqUnit.index1)) {
            if (PB_ADN_CONTENT == pbOffset) {
                g_pbCtrlInfo.totalUsed++;
            }

            SI_PB_SetBitToBuf(g_pbContent[pbOffset].index, g_pbContent[pbOffset].indexSize,
                              g_pbReqUnit.index1, SI_PB_CONTENT_VALID);
        }
    } else {
        /* should not */
    }

    return;
}

/*
 * 功能描述: 跟新ADN电话本的使用情况
 * 修改历史:
 * 1. 日    期: 2009年06月05日
 *    修改内容: Creat
 */
VOS_VOID SI_PB_RefreshADNUsedNum(VOS_UINT8 *dstStorage, VOS_UINT32 destSize, VOS_UINT8 *srcReq,
                                 VOS_UINT32 srcSize, VOS_UINT8 offset)
{
    /* 该电话本缓存(需判断缓存是否为空)和PBreq比较 */
    /* 如果原来有效，现在无效，则有效值减减 */
    if (VOS_OK == SI_PB_CheckContentValidity(&g_pbContent[offset], dstStorage, destSize)) {
        if (VOS_OK != SI_PB_CheckContentValidity(&g_pbContent[offset], srcReq, srcSize)) {
            g_pbContent[offset].usedNum--;
        }
    }
    /* 如果原来无效，现在有效，则有效值加加 */
    else {
        if (VOS_OK == SI_PB_CheckContentValidity(&g_pbContent[offset], srcReq, srcSize)) {
            g_pbContent[offset].usedNum++;
        }
    }

    return;
}

/*
 * 功能描述: 跟新EXT电话本的使用情况
 * 修改历史:
 * 1. 日    期: 2009年06月05日
 *    修改内容: Creat
 */
VOS_VOID SI_PB_RefreshEXTUsedNum(VOS_UINT8 dstStorage0, VOS_UINT8 dstStorage1,
                                              VOS_UINT8 srcReq0, VOS_UINT8 srcReq1, VOS_UINT8 offset)
{
    if ((0x0 != dstStorage0) && (0xFF != dstStorage1)) { /* 有效 */
        if ((0x0 == srcReq0) || (0xFF == srcReq1)) {
            g_extContent[offset].extUsedNum--;
        }
    } else {
        if ((0x0 != srcReq0) && (0xFF != srcReq1)) {
            g_extContent[offset].extUsedNum++;
        }
    }

    return;
}

/*
 * 功能描述: 跟新ANR电话本的使用情况
 * 修改历史:
 * 1. 日    期: 2009年06月05日
 *    修改内容: Creat
 */
VOS_VOID SI_PB_RefreshANRUsedNum(VOS_UINT32 dstResult, VOS_UINT8 srcReq1, VOS_UINT8 srcReq2, VOS_UINT8 offset)
{
    if (VOS_OK == dstResult) { /* 有效 */
        if (VOS_OK != SI_PB_CheckANRValidity(srcReq1, srcReq2)) {
            g_anrContent[offset].usedNum--;
        }
    } else {
        if (VOS_OK == SI_PB_CheckANRValidity(srcReq1, srcReq2)) {
            g_anrContent[offset].usedNum++;
        }
    }

    return;
}

/*
 * 功能描述: 跟新EML电话本的使用情况
 * 修改历史:
 * 1. 日    期: 2009年06月05日
 *    修改内容: Creat
 */
VOS_VOID SI_PB_RefreshEMLUsedNum(VOS_UINT8 dstStorage, VOS_UINT8 srcReq)
{
    if (0xFF != dstStorage) { /* 有效 */
        if (0xFF == srcReq) {
            g_emlContent.usedNum--;
        }
    } else {
        if (0xFF != srcReq) {
            g_emlContent.usedNum++;
        }
    }

    return;
}

/*
 * 功能描述: 根据XDN的使用个数
 * 修改历史:
 * 1. 日    期: 2018年03月15日
 *    修改内容: Creat
 */
VOS_VOID SI_PB_XDN_ProcessUsedNum(VOS_UINT8 offset, const VOS_UINT8 *dstStorage, VOS_UINT32 dataSize)
{
    if (SI_PB_EVENT_ADD_CNF == g_pbReqUnit.pbEventType) {
        g_pbContent[offset].usedNum++;
    }

    if (SI_PB_EVENT_DELETE_CNF == g_pbReqUnit.pbEventType) {
        g_pbContent[offset].usedNum--;
        (VOS_VOID)memset_s(g_pbReqUnit.xdnContent, sizeof(g_pbReqUnit.xdnContent), 0xFF,
                           sizeof(g_pbReqUnit.xdnContent));
    }

    if (SI_PB_EVENT_MODIFY_CNF == g_pbReqUnit.pbEventType) {
        if (VOS_OK != SI_PB_CheckContentValidity(&g_pbContent[offset], dstStorage, dataSize)) {
            g_pbContent[offset].usedNum++;
        }
    }

    return;
}

/*
 * 功能描述: 判断是否XDN的文件ID
 * 修改历史:
 * 1. 日    期: 2018年03月15日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PB_Is_XdnFileId(VOS_UINT16 fileId)
{
    if ((EFFDN == fileId) || (EFBDN == fileId) || (EFMSISDN == fileId)) {
        return VOS_OK;
    }

    return VOS_ERR;
}

/*
 * 功能描述: 根据电话本的更新接
 * 修改历史:
 * 1. 日    期: 2009年06月05日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PB_RefreshMemory(VOS_UINT16 fileId, VOS_UINT16 recordLen, VOS_UINT8 offset, VOS_UINT8 recordNum)
{
    VOS_UINT8 *srcReq     = VOS_NULL_PTR;
    VOS_UINT8 *dstStorage = VOS_NULL_PTR;
    VOS_UINT16 memOffset;
    VOS_UINT8  suffix;
    VOS_UINT8  fileCnt;
    VOS_UINT32 freeSize   = 0;
    VOS_UINT32 destOffset = 0;
    VOS_UINT32 result;
    VOS_UINT32 copyLen;
    errno_t    ret;

    /* 解析出该文件属于哪个电话本 */
    if (VOS_OK == SI_PB_JudgeADNFid(fileId)) { /* ADN */
        memOffset   = (VOS_UINT16)((g_pbReqUnit.index1 - 1) * g_pbContent[offset].recordLen);
        srcReq     = g_pbReqUnit.xdnContent;

        if ((g_pbContent[offset].content == VOS_NULL_PTR) || (g_pbContent[offset].contentSize < memOffset)) {
            PB_WARNING_LOG("SI_PB_RefreshMemory: g_pbContent No Enough Space.");
            return VOS_ERR;
        }

        dstStorage = &g_pbContent[offset].content[memOffset];

        freeSize = g_pbContent[offset].contentSize - memOffset;

        SI_PB_RefreshADNUsedNum(dstStorage, freeSize, srcReq, sizeof(g_pbReqUnit.xdnContent), offset);
    } else if (VOS_OK == SI_PB_JudgeEXTFid(fileId, offset)) { /* EXT */
        /* 根据EXT的回复Record，取出在其在gstPBReqUnit中的对应位置 */
        if (VOS_OK != SI_PB_GetEXTContentFromReq(recordNum, &srcReq)) {
            PB_WARNING_LOG("SI_PB_RefreshMemory:SI_PB_GetEXTContentFromReq ERROR.");
            return VOS_ERR;
        }

        if (g_extContent[offset].extContent == VOS_NULL_PTR) {
            PB_WARNING_LOG("SI_PB_RefreshMemory:extContent is NULL");

            return VOS_ERR;
        }

        memOffset   = (recordNum - 1) * SI_PB_EXT_LEN;
        dstStorage = &g_extContent[offset].extContent[memOffset];

        if (g_extContent[offset].contentSize < memOffset) {
            PB_WARNING_LOG("SI_PB_RefreshMemory:g_extContent No Enough Space.");
            return VOS_ERR;
        }

        SI_PB_RefreshEXTUsedNum(dstStorage[0], dstStorage[1], srcReq[0], srcReq[1], offset);

        freeSize = g_extContent[offset].contentSize - memOffset;
    } else if (VOS_OK == SI_PB_GetANRSuffix(&fileCnt, &suffix, fileId)) { /* ANR */
        srcReq = g_pbReqUnit.spbReq.anrContent[suffix];

        memOffset =
            (VOS_UINT16)(((fileCnt * g_pbCtrlInfo.anrInfo[fileCnt][suffix].recordNum) + (recordNum - 1)) *
                         g_anrContent[suffix].recordLen);

        if ((g_anrContent[suffix].content == VOS_NULL_PTR) || (g_anrContent[suffix].contentSize < memOffset)) {
            PB_WARNING_LOG("SI_PB_RefreshMemory:g_anrContent No Enough Space.");
            return VOS_ERR;
        }

        dstStorage = &g_anrContent[suffix].content[memOffset];

        result = SI_PB_CheckANRValidity(dstStorage[0], dstStorage[1]);

        SI_PB_RefreshANRUsedNum(result, srcReq[0], srcReq[1], suffix);

        freeSize = g_anrContent[suffix].contentSize - memOffset;
    } else if (VOS_OK == SI_PB_GetEMLRecord(&dstStorage, fileId, recordNum, &freeSize)) { /* email */
        srcReq = g_pbReqUnit.spbReq.emlContent;

        SI_PB_RefreshEMLUsedNum(dstStorage[0], srcReq[0]);
    } else if (VOS_OK == SI_PB_JudgeIAPFid(fileId)) {
        srcReq = g_pbReqUnit.spbReq.iapContent;

        PB_NORMAL_LOG("SI_PB_RefreshMemory:IAP File.");

        destOffset = (g_pbReqUnit.index1 - 1) * g_iapContent.recordLen;

        if ((g_iapContent.iapContent == VOS_NULL_PTR) || (g_iapContent.contentSize < destOffset)) {
            PB_WARNING_LOG("SI_PB_RefreshMemory:g_iapContent No Enough Space.");
            return VOS_ERR;
        }

        dstStorage = &g_iapContent.iapContent[(g_pbReqUnit.index1 - 1) * g_iapContent.recordLen];

        freeSize = g_iapContent.contentSize - destOffset;
    } else if (VOS_OK == SI_PB_Is_XdnFileId(fileId)) {
        srcReq     = g_pbReqUnit.xdnContent;

        destOffset = (g_pbReqUnit.index1 - 1) * g_pbContent[offset].recordLen;

        if ((g_pbContent[offset].content == VOS_NULL_PTR) || (g_pbContent[offset].contentSize < destOffset)) {
            PB_WARNING_LOG("SI_PB_RefreshMemory:g_pbContent No Enough Space.");
            return VOS_ERR;
        }

        dstStorage = g_pbContent[offset].content + ((g_pbReqUnit.index1 - 1) * g_pbContent[offset].recordLen);

        freeSize = g_pbContent[offset].contentSize - destOffset;

        SI_PB_XDN_ProcessUsedNum(offset, dstStorage, freeSize);
    } else {
        PB_WARNING_LOG("SI_PB_RefreshMemory:Unknown PB Type.");
        return VOS_ERR;
    }

    copyLen = PAM_GetMin(freeSize, recordLen);

    if (copyLen != 0) {
        ret = memcpy_s(dstStorage, freeSize, srcReq, copyLen);
        PAM_PRINT_SECFUN_RESULT(ret);
    }

    return VOS_OK;
}

VOS_VOID SI_PB_UsimSetFileProc(const USIMM_UpdateFileCnf *msg, VOS_UINT8 pbOffset,
                               SI_PB_EventInfo *cnfData)
{
    VOS_UINT16 efId;

    SI_PB_GetEFIDFromPath(&msg->filePath, &efId);

    if (VOS_OK != msg->cmdResult.result) {
        cnfData->pbError = TAF_ERR_PB_STORAGE_OP_FAIL;
        PB_WARNING_LOG("SI_PB_UsimSetFileProc:Usimm Cnf ERROR.");
        return;
    }

    SI_PB_JudgeTotalUsedNum(pbOffset, cnfData);

    if (VOS_NULL_PTR != g_pbContent[pbOffset].content) {
        if (VOS_OK != SI_PB_RefreshMemory(efId, msg->efLen, pbOffset, msg->recordNum)) {
            cnfData->pbError = TAF_ERR_PB_STORAGE_OP_FAIL;
            PB_WARNING_LOG("SI_PB_UsimSetFileProc:SI_PB_RefreshMemory ERROR.");
            return;
        }
    }

    if (VOS_OK == SI_PB_CheckADNFileID(efId)) {
        SI_PB_IncreaceCCValue(1, msg->recordNum);
    }

    return;
}

/*
 * 功能描述: 处理对EXT文件操作的回复
 * 修改历史:
 * 1. 日    期: 2009年5月7日
 *    修改内容: Creat
 */
VOS_VOID SI_PB_UsimSetExtFileProc(const USIMM_UpdateFileCnf *msg, VOS_UINT8 pbOffset)
{
    VOS_UINT8 *ptemp = VOS_NULL_PTR;
    VOS_UINT32 extInfoNum;
    VOS_UINT32 freeSize;
    VOS_UINT32 destOffset;
    errno_t    ret;

    extInfoNum = g_pbContent[pbOffset].extInfoNum;

    if (VOS_OK == msg->cmdResult.result) {
        destOffset = (g_pbReqUnit.extIndex - 1) * SI_PB_EXT_LEN;

        if (g_extContent[pbOffset].contentSize < destOffset) {
            PB_WARNING_LOG("WARNING:SI_PB_UsimSetExtFileProc: No enough space");
            return;
        }

        freeSize = g_extContent[pbOffset].contentSize - destOffset;

        if (g_extContent[pbOffset].extContent == VOS_NULL_PTR) {
            PB_WARNING_LOG("WARNING:SI_PB_UsimSetExtFileProc: extContent is NULL");

            return;
        }

        ptemp = g_extContent[pbOffset].extContent + destOffset;

        if (SI_PB_EVENT_ADD_CNF == g_pbReqUnit.pbEventType) {
            g_extContent[extInfoNum].extUsedNum++;
        }

        if (SI_PB_EVENT_DELETE_CNF == g_pbReqUnit.pbEventType) {
            g_extContent[extInfoNum].extUsedNum--;
        }

        if ((SI_PB_EVENT_MODIFY_CNF == g_pbReqUnit.pbEventType) && (0xFF == *(ptemp + 1))) {
            g_extContent[extInfoNum].extUsedNum++;
        }

        ret = memcpy_s(ptemp, freeSize, g_pbReqUnit.extContent, SI_PB_EXT_LEN);

        PAM_PRINT_SECFUN_RESULT(ret);
    } else {
        PB_WARNING_LOG("WARNING:SI_PB_UsimSetExtFileProc ERROR.");
    }
}

/*
 * 功能描述: PB模块构造一条假的卡状态消息触发PB重新初始化
 * 修改历史:
 * 1.日    期: 2013年5月28日
 *   修改内容: Creat
 */
VOS_VOID SI_PB_RefreshCardIndMsgSnd(VOS_VOID)
{
    USIMM_CardStatusInd   *usimMsg = VOS_NULL_PTR;
    SI_PIH_CardSlotUint32  slotId;

    usimMsg = (USIMM_CardStatusInd *)VOS_AllocMsg(MAPS_PIH_PID,
                                                         sizeof(USIMM_CardStatusInd) - VOS_MSG_HEAD_LENGTH);
    if (VOS_NULL_PTR == usimMsg) {
        return;
    }

    (VOS_VOID)memset_s(usimMsg, sizeof(USIMM_CardStatusInd), 0, sizeof(USIMM_CardStatusInd));
    VOS_SET_SENDER_ID(usimMsg, MAPS_PIH_PID);
    VOS_SET_MSG_LEN(usimMsg, sizeof(USIMM_CardStatusInd) - VOS_MSG_HEAD_LENGTH);

    slotId = USIMM_GetSlotIdByModemId(MODEM_ID_0);

    usimMsg->msgName                      = USIMM_CARDSTATUS_IND; /* 设置消息名称 */
    usimMsg->phyCardType                  = USIMM_CCB_GetCardType(slotId);
    usimMsg->usimSimInfo.cardAppService = USIMM_CCB_GetAppService(slotId, USIMM_GUTL_APP);
    usimMsg->csimUimInfo.cardAppService = USIMM_CARDAPP_SERVIC_ABSENT;
    usimMsg->isimInfo.cardAppService    = USIMM_CARDAPP_SERVIC_ABSENT;
    usimMsg->addInfo.mediumType         = USIMM_GetCardMediumType(slotId);
    usimMsg->addInfo.statusType         = USIMM_CARD_STATUS_INIT_IND;
    usimMsg->ulReceiverPid                  = MAPS_PB_PID;

    PAM_OM_ReportLayerMsg(usimMsg);
    (VOS_VOID)VOS_SendMsg(MAPS_PIH_PID, usimMsg);

    return;
}

/*
 * 功能描述: PB模块判断是否是SIM卡电话本更新
 * 修改历史:
 * 1.日    期: 2015年2月2日
 *   修改内容: Creat
 */
VOS_BOOL SI_PB_SimRefreshFileCheck(VOS_UINT16 pathLen, const VOS_UINT8 *pcFilePath)
{
    USIMM_DefFileidUint32  fileId;

    if (VOS_OK != USIMM_ChangePathToDefFileID(USIMM_GUTL_APP, pathLen, (VOS_CHAR *)pcFilePath, &fileId)) {
        PB_WARNING_LOG("SI_PB_SimRefreshFileCheck: USIMM_ChangePathToDefFileID is ERROR.");

        return VOS_FALSE;
    }

    if ((fileId == USIMM_TELE_EFADN_ID) || (fileId == USIMM_TELE_EFFDN_ID) || (fileId == USIMM_TELE_EFBDN_ID) ||
        (fileId == USIMM_TELE_EFMSISDN_ID)) {
        return VOS_TRUE;
    } else {
        return VOS_FALSE;
    }
}

/*
 * 功能描述: PB模块判断是否是SIM卡电话本更新
 * 修改历史:
 * 1.日    期: 2015年2月2日
 *   修改内容: Creat
 */
VOS_BOOL SI_PB_UsimRefreshFileCheck(VOS_UINT16 pathLen, const VOS_UINT8 *pcFilePath, USIMM_CardAppUint32 appType)
{
    VOS_CHAR                     globalPath[] = { '3', 'F', '0', '0', '7', 'F', '1', '0', '5', 'F', '3', 'A' };
    VOS_CHAR                     locallPath[] = { '3', 'F', '0', '0', '7', 'F', 'F', 'F', '5', 'F', '3', 'A' };
    VOS_INT32                    rslt1          = VOS_ERR;
    VOS_INT32                    rslt2          = VOS_ERR;
    USIMM_DefFileidUint32  fileId;

    if (pathLen >= sizeof(globalPath)) {
        rslt1 = VOS_MemCmp(globalPath, pcFilePath, sizeof(globalPath));
    }

    if (pathLen >= sizeof(locallPath)) {
        rslt2 = VOS_MemCmp(locallPath, pcFilePath, sizeof(locallPath));
    }

    if ((VOS_OK == rslt1) || (VOS_OK == rslt2)) {
        return VOS_TRUE;
    }

    if (VOS_OK != USIMM_ChangePathToDefFileID(appType, pathLen, (VOS_CHAR *)pcFilePath, &fileId)) {
        PB_WARNING_LOG("SI_PB_SimRefreshFileCheck: USIMM_ChangePathToDefFileID is ERROR.");

        return VOS_FALSE;
    }

    if ((fileId == USIMM_TELE_EFFDN_ID) || (fileId == USIMM_TELE_EFBDN_ID) ||
        (fileId == USIMM_TELE_EFMSISDN_ID) || (fileId == USIMM_USIM_EFFDN_ID) ||
        (fileId == USIMM_USIM_EFBDN_ID) || (fileId == USIMM_USIM_EFMSISDN_ID)) {
        return VOS_TRUE;
    } else {
        return VOS_FALSE;
    }
}

/*
 * 功能描述: PB模块Refresh命令判断是否需要处理
 * 修改历史:
 * 1.日    期: 2015年2月2日
 *   修改内容: Creat
 */
VOS_BOOL SI_PB_RefreshFileCheck(const USIMM_STKRefreshInd *msg)
{
    VOS_UINT16 i;

    if (USIMM_CARD_SIM == g_pbInitState.cardType) {
        for (i = 0; i < msg->efNum; i++) {
            if (VOS_TRUE == SI_PB_SimRefreshFileCheck(msg->efId[i].pathLen, msg->efId[i].path)) {
                return VOS_TRUE;
            }
        }
    } else {
        for (i = 0; i < msg->efNum; i++) {
            if (VOS_TRUE == SI_PB_UsimRefreshFileCheck(msg->efId[i].pathLen, msg->efId[i].path,
                                                       msg->efId[i].appType)) {
                return VOS_TRUE;
            }
        }
    }

    return VOS_FALSE;
}

/*
 * 功能描述: PB模块Refresh命令处理函数
 * 修改历史:
 * 1.日    期: 2013年5月28日
 *   修改内容: Creat
 */
VOS_VOID SI_PB_PihRefreshIndProc(const USIMM_STKRefreshInd *msg)
{
    VOS_UINT32 pbRefreshFlag;

    if (USIMM_REFRESH_ALL_FILE == msg->refreshType) {
        pbRefreshFlag = VOS_TRUE;
    } else if (USIMM_REFRESH_3G_SESSION_RESET == msg->refreshType) {
        pbRefreshFlag = VOS_FALSE;
    } else if (USIMM_REFRESH_FILE_LIST == msg->refreshType) {
        if (VOS_FALSE == SI_PB_RefreshFileCheck(msg)) {
            pbRefreshFlag = VOS_FALSE;
        } else {
            pbRefreshFlag = VOS_TRUE;
        }
    } else {
        pbRefreshFlag = VOS_FALSE;
    }

    if (VOS_TRUE == pbRefreshFlag) {
        SI_PB_ReleaseAll();

        SI_PB_InitStatusInd(USIMM_PB_IDLE);

        SI_PB_RefreshCardIndMsgSnd();
    }

    return;
}
/*
 * 功能描述: 处理对复合电话本文件操作的回复
 * 修改历史:
 * 1. 日    期: 2009年06月08日
 *    修改内容: Creat
 */
VOS_VOID SI_PB_UsimSetSPBFileProc(const USIMM_SetMutilFileCnf *pbMsg, SI_PB_EventInfo *cnfData,
                                  VOS_UINT8 offset)
{
    VOS_UINT32 i;

    /* 如果USIMM回复失败，则直接退出，不对内存进行操作 */
    if (VOS_OK != pbMsg->cmdResult.result) {
        cnfData->pbError = TAF_ERR_PB_STORAGE_OP_FAIL;
        PB_WARNING_LOG("SI_PB_UsimSetSPBFileProc:Usimm Cnf ERROR.");
        return;
    }

    SI_PB_JudgeTotalUsedNum(offset, cnfData);

    /* 针对复合电话本中的每个电话本需要比较更改前后的内存具体内容确认使用情况，并更新内存 */
    for (i = 0; i < pbMsg->fileNum; i++) {
        if (VOS_OK !=
            SI_PB_RefreshMemory(pbMsg->efId[i], pbMsg->efLen[i], offset, pbMsg->recordNum[i])) {
            cnfData->pbError = TAF_ERR_PB_STORAGE_OP_FAIL;
            PB_WARNING_LOG("SI_PB_UsimSetSPBFileProc:SI_PB_RefreshMemory ERROR.");
            return;
        }

        if (VOS_OK == SI_PB_CheckADNFileID(pbMsg->efId[i])) {
            SI_PB_IncreaceCCValue(1, pbMsg->recordNum[i]);
        }
    }

    return;
}

VOS_VOID SI_PB_UsimGetFileProc(const USIMM_ReadFileCnf *msg, VOS_UINT8 pbOffset, SI_PB_EventInfo *cnfData)
{
    if (VOS_OK != msg->cmdResult.result) {
        cnfData->pbError = TAF_ERR_PB_STORAGE_OP_FAIL;

        PB_WARNING_LOG("SI_PB_UsimGetFileProc: USIMM Cnf Error");

        return;
    }

    SI_PB_TransPBFromate(&g_pbContent[pbOffset], g_pbReqUnit.curIndex, (const VOS_UINT8*)msg->efData,
                         &cnfData->pbEvent.pbReadCnf.pbRecord);

    cnfData->pbEvent.pbReadCnf.recordNum = (g_pbReqUnit.index2 - g_pbReqUnit.index1) + 1;

    if (g_pbReqUnit.curIndex == g_pbReqUnit.index2) {
        cnfData->pbLastTag = VOS_TRUE;
    } else {
        cnfData->pbLastTag = VOS_FALSE;

        g_pbReqUnit.curIndex++;
    }

    return;
}

VOS_VOID SI_PB_UsimSearchFileProc(const USIMM_ReadFileCnf *msg, VOS_UINT8 pbOffset,
                                  SI_PB_EventInfo *cnfData)
{
    if (VOS_OK != msg->cmdResult.result) {
        cnfData->pbError = TAF_ERR_PB_STORAGE_OP_FAIL;

        PB_WARNING_LOG("SI_PB_UsimSearchFileProc: USIMM Cnf Error");

        return;
    }

    cnfData->pbEvent.pbSearchCnf.recordNum = (g_pbReqUnit.index2 - g_pbReqUnit.index1) + 1;

    if (msg->efLen >= g_pbReqUnit.searchLen) {
        if ((VOS_OK == VOS_MemCmp(msg->efData, g_pbReqUnit.xdnContent, g_pbReqUnit.searchLen)) ||
            (0 == g_pbReqUnit.searchLen)) {
            SI_PB_TransPBFromate(&g_pbContent[pbOffset], g_pbReqUnit.curIndex, (const VOS_UINT8*)msg->efData,
                                 &cnfData->pbEvent.pbSearchCnf.pbRecord);

            g_pbReqUnit.equalFlag = VOS_TRUE;
        } else {
            cnfData->pbEvent.pbSearchCnf.pbRecord.validFlag = SI_PB_CONTENT_INVALID;
        }
    } else {
        cnfData->pbEvent.pbSearchCnf.pbRecord.validFlag = SI_PB_CONTENT_INVALID;
    }

    if (g_pbReqUnit.curIndex == g_pbReqUnit.index2) {
        cnfData->pbLastTag = VOS_TRUE;

        if (VOS_TRUE != g_pbReqUnit.equalFlag) {
            cnfData->pbError = TAF_ERR_PB_NOT_FOUND;
        }
    } else {
        cnfData->pbLastTag = VOS_FALSE;

        g_pbReqUnit.curIndex++;
    }

    return;
}

VOS_VOID SI_PB_UsimGetIndexFile(VOS_UINT8 pbOffset, SI_PB_EventInfo *cnfData)
{
    VOS_UINT32              result;
    VOS_UINT8               sendReqFlag = PB_REQ_NOT_SEND;
    VOS_UINT8               record;
    VOS_UINT16              fileId;
    SI_PB_GetfileInfo getFileInfo;

    /* 所有记录已经读完,或者出错则退出 */
    if ((VOS_OK != cnfData->pbError) || (VOS_TRUE == cnfData->pbLastTag)) {
        return;
    }

    while (g_pbReqUnit.curIndex <= g_pbReqUnit.index2) {
        if (SI_PB_CONTENT_VALID == SI_PB_GetBitFromBuf(g_pbContent[pbOffset].index,
                                                       g_pbContent[pbOffset].indexSize, g_pbReqUnit.curIndex)) {
            sendReqFlag = PB_REQ_SEND;
            break;
        }

        g_pbReqUnit.curIndex++;
    }

    if (PB_REQ_SEND == sendReqFlag) {
        if (SI_PB_STORAGE_SM == g_pbReqUnit.pbStoateType) { /* 如果是ADN需要转换为记录号 */
            result = SI_PB_CountADNRecordNum(g_pbReqUnit.curIndex, &fileId, &record);
        } else { /* 访问其它电话本 */
            result = SI_PB_GetXDNFileID(g_pbReqUnit.pbStoateType, &fileId);

            record = (VOS_UINT8)g_pbReqUnit.curIndex;
        }

        if (VOS_ERR == result) { /* 转换结果失败 */
            PB_WARNING_LOG("SI_PB_SearchReq: Get the XDN File ID and Record number is Error");

            cnfData->pbError = TAF_ERR_PARA_ERROR;

            return;
        }

        getFileInfo.recordNum = record;
        getFileInfo.efId      = fileId;

        result = SI_PB_GetFileReq(&getFileInfo);
        if (VOS_OK != result) {
            cnfData->pbError = TAF_ERR_USIM_SIM_CARD_NOTEXIST;
        }
    } else {
        cnfData->pbLastTag = VOS_TRUE;

        if ((SI_PB_EVENT_SEARCH_CNF == g_pbReqUnit.pbEventType) && (VOS_TRUE != g_pbReqUnit.equalFlag)) {
            cnfData->pbError = TAF_ERR_PB_NOT_FOUND;
        }
    }

    return;
}

/*
 * 功能描述: 发送全局变量内容给Acpu
 * 修改历史:
 * 1.日    期: 2011年10月15日
 *   修改内容: Creat
 */
VOS_VOID SI_PBSendGlobalToAcpu(VOS_VOID)
{
    SI_PB_UpdateglobalInd    *msg = VOS_NULL_PTR;
    VOS_UINT32                i;
    NAS_NVIM_SystemAppConfig  systemAppConfig = {0};
    VOS_UINT_PTR              tempAddr;
    errno_t                   ret;

    /* 规避ANDROID 64位指针的问题，不把指针发到A核去 */
    if (VOS_OK !=
        mdrv_nv_read(NV_ITEM_SYSTEM_APP_CONFIG, &systemAppConfig, sizeof(NAS_NVIM_SystemAppConfig))) {
        return;
    }

    if (SYSTEM_APP_ANDROID == systemAppConfig.sysAppConfigType) {
        return;
    }

    msg = (SI_PB_UpdateglobalInd *)VOS_AllocMsg(MAPS_PB_PID,
                                                         sizeof(SI_PB_UpdateglobalInd) - VOS_MSG_HEAD_LENGTH);
    if (VOS_NULL_PTR == msg) {
        return;
    }

    msg->ulReceiverPid = ACPU_PID_PB;
    msg->msgName     = SI_PB_UPDATE_AGOBAL;

    /* 拷贝全局 Acpu 需要的全局变量内容 */
    ret = memcpy_s(&msg->pbCtrlInfo, sizeof(g_pbCtrlInfo), &g_pbCtrlInfo, sizeof(g_pbCtrlInfo));
    PAM_PRINT_SECFUN_RESULT(ret);

    ret = memcpy_s(&msg->pbConfigInfo, sizeof(g_pbConfigInfo), &g_pbConfigInfo, sizeof(g_pbConfigInfo));
    PAM_PRINT_SECFUN_RESULT(ret);

    ret = memcpy_s(msg->pbContent, sizeof(g_pbContent), g_pbContent, sizeof(g_pbContent));
    PAM_PRINT_SECFUN_RESULT(ret);

    ret = memcpy_s(msg->extContent, sizeof(g_extContent), g_extContent, sizeof(g_extContent));
    PAM_PRINT_SECFUN_RESULT(ret);

    ret = memcpy_s(msg->anrContent, sizeof(g_anrContent), g_anrContent, sizeof(g_anrContent));
    PAM_PRINT_SECFUN_RESULT(ret);

    ret = memcpy_s(&msg->emlContent, sizeof(g_emlContent), &g_emlContent, sizeof(g_emlContent));
    PAM_PRINT_SECFUN_RESULT(ret);

    ret = memcpy_s(&msg->iapContent, sizeof(g_iapContent), &g_iapContent, sizeof(g_iapContent));
    PAM_PRINT_SECFUN_RESULT(ret);

    ret = memcpy_s(&msg->pbInitState, sizeof(g_pbInitState), &g_pbInitState, sizeof(g_pbInitState));
    PAM_PRINT_SECFUN_RESULT(ret);

    for (i = 0; i < SI_PB_MAX_NUMBER; i++) {
        tempAddr = (VOS_UINT_PTR)g_pbContent[i].index;
        msg->pbContentAddr[i].indexAddr = (VOS_UINT32)tempAddr;

        tempAddr = (VOS_UINT_PTR)g_pbContent[i].content;
        msg->pbContentAddr[i].contentAddr = (VOS_UINT32)tempAddr;

        tempAddr = (VOS_UINT_PTR)g_extContent[i].extContent;
        msg->extContentAddr[i] = (VOS_UINT32)tempAddr;
    }

    for (i = 0; i < SI_PB_ANRMAX; i++) {
        tempAddr = (VOS_UINT_PTR)g_anrContent[i].content;
        msg->anrContentAddr[i] = (VOS_UINT32)tempAddr;
    }

    tempAddr = (VOS_UINT_PTR)g_emlContent.content;
    msg->emlContentAddr = (VOS_UINT32)tempAddr;

    tempAddr = (VOS_UINT_PTR)g_iapContent.iapContent;
    msg->iapContentAddr = (VOS_UINT32)tempAddr;

    PAM_OM_ReportLayerMsg(msg);
    if (VOS_OK != VOS_SendMsg(MAPS_PB_PID, msg)) {
        PB_ERROR_LOG("SI_PBSendGobalToAcpu: Send msg Error");
    }

    return;
}

/*
 * 功能描述: ECALL号码初始化上报
 * 修改历史:
 * 1.日    期: 2014年04月25日
 *   修改内容: Creat
 */
VOS_VOID SI_PB_EcallInitIndFunc(VOS_VOID)
{
    SI_PB_EcallInitInd *ecallIndMsg = VOS_NULL_PTR;

    ecallIndMsg = (SI_PB_EcallInitInd *)VOS_AllocMsg(MAPS_PB_PID,
                                                              sizeof(SI_PB_EcallInitInd) - VOS_MSG_HEAD_LENGTH);
    if (VOS_NULL_PTR == ecallIndMsg) {
        PB_ERROR_LOG("SI_PB_EcallInitInd: Alloc Msg Fail");

        return;
    }

    ecallIndMsg->ulReceiverPid  = WUEPS_PID_TAF;
    ecallIndMsg->msgName      = SI_PB_EVENT_ECALLINIT_IND;
    ecallIndMsg->fdnRecordNum = g_pbContent[PB_FDN_CONTENT].totalNum;
    ecallIndMsg->sdnRecordNum = g_pbContent[PB_SDN_CONTENT].totalNum;

    PAM_OM_ReportLayerMsg(ecallIndMsg);
    (VOS_VOID)VOS_SendMsg(MAPS_PB_PID, ecallIndMsg);

    return;
}

/*
 * 功能描述: 释放电话本空间及全局变量清空
 * 修改历史:
 * 1. 日    期: 2012年06月09日
 *    修改内容: Creat
 */
VOS_VOID SI_PB_ReleaseAll(VOS_VOID)
{
    g_pbInitState.pbInitStep = PB_INIT_PB_STATUS;

    SI_PB_ClearPBContent(SI_CLEARALL);

    SI_PB_ClearSPBContent(SI_CLEARALL);

    SI_PB_InitGlobeVariable();

#if (FEATURE_ACORE_MODULE_TO_CCORE != FEATURE_ON)
    SI_PBSendGlobalToAcpu();
#endif
}

/*
 * 功能描述: 处理USIMM模块回复的请求，需要根据结果回复给AT
 * 修改历史:
 * 1. 日    期: 2007年10月15日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PBUsimCnfMsgProc(const PBMsgBlock *msg)
{
    VOS_UINT8             pbOffset;
    SI_PB_EventInfo cnfData;
    VOS_UINT32            result;
    VOS_UINT16            efId;

    (VOS_VOID)memset_s(&cnfData, sizeof(cnfData), 0, sizeof(cnfData));

    cnfData.clientId = g_pbReqUnit.clientId;

    cnfData.opId = g_pbReqUnit.opId;

    cnfData.pbError = TAF_ERR_NO_ERROR;

    cnfData.pbEventType = g_pbReqUnit.pbEventType;

    cnfData.storage = g_pbReqUnit.pbStoateType;

    result = SI_PB_FindPBOffset(g_pbReqUnit.pbStoateType, &pbOffset);
    if ((VOS_OK != result) && (USIMM_CARDSTATUS_IND != msg->msgName)) {
        cnfData.pbError = TAF_ERR_PB_STORAGE_OP_FAIL;

        return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
    }

    switch (msg->msgName) {
        case USIMM_UPDATEFILE_CNF:
            SI_PB_GetEFIDFromPath(&((USIMM_UpdateFileCnf *)msg)->filePath, &efId);

            if (VOS_OK == SI_PB_CheckSYNCHFileID(efId)) {
                return VOS_OK;
            }

            if (VOS_OK == SI_PB_CheckExtFileID(efId)) {
                SI_PB_UsimSetExtFileProc((const USIMM_UpdateFileCnf *)msg, pbOffset);

                return VOS_OK;
            }

            SI_PB_UsimSetFileProc((const USIMM_UpdateFileCnf *)msg, pbOffset, &cnfData);

            break;

        case USIMM_READFILE_CNF:
            if (SI_PB_EVENT_READ_CNF == cnfData.pbEventType) {
                SI_PB_UsimGetFileProc((const USIMM_ReadFileCnf *)msg, pbOffset, &cnfData);
            } else {
                SI_PB_UsimSearchFileProc((const USIMM_ReadFileCnf *)msg, pbOffset, &cnfData);
            }

            SI_PB_UsimGetIndexFile(pbOffset, &cnfData);

            break;

        case USIMM_SETMUTILFILE_CNF:
            SI_PB_UsimSetSPBFileProc((const USIMM_SetMutilFileCnf *)msg, &cnfData, pbOffset);
            break;

        default:
            PB_WARNING_LOG("SI_PBUsimCnfMsgProc:Msg Name is unknow");
            return VOS_ERR;
    }

    return SI_PBSendAtEventCnf(&cnfData); /* 调用回调函数 */
}

/*
 * 功能描述: 处理当前USIMM返回和上报的所有消息
 * 修改历史:
 * 1. 日    期: 2007年10月15日
 *    修改内容: Creat
 */
VOS_UINT32 SI_PBUsimMsgProc(const PBMsgBlock *msg)
{
    VOS_UINT32                 result;
    USIMM_CardStatusInd *stStatusMsg = VOS_NULL_PTR;

    /* 在Refresh命令过程中还会收到ECC号码的上报，需要处理 */
    if (USIMM_ECCNUMBER_IND == msg->msgName) {
        /* 如果之前有申请过内存，释放内存防止内存泄漏 */
        if (VOS_NULL_PTR != g_pbContent[PB_ECC_CONTENT].content) {
            (VOS_VOID)PB_FREE(g_pbContent[PB_ECC_CONTENT].content);

            g_pbContent[PB_ECC_CONTENT].content = VOS_NULL_PTR;
        }

        result = SI_PB_InitEccProc(msg);
    }
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    else if (USIMM_XECCNUMBER_IND == msg->msgName) {
        /* 如果之前有申请过内存，释放内存防止内存泄漏 */
        if (VOS_NULL_PTR != g_pbContent[PB_XECC_CONTENT].content) {
            (VOS_VOID)PB_FREE(g_pbContent[PB_XECC_CONTENT].content);

            g_pbContent[PB_XECC_CONTENT].content = VOS_NULL_PTR;
        }

        result = SI_PB_InitXeccProc(msg);
    }
#endif
    else if (USIMM_CARDSTATUS_IND == msg->msgName) {
        stStatusMsg = (USIMM_CardStatusInd *)msg;

        if ((USIMM_CARDAPP_SERVIC_ABSENT == stStatusMsg->usimSimInfo.cardAppService) &&
            (USIMM_CARDAPP_SERVIC_ABSENT == stStatusMsg->csimUimInfo.cardAppService)) {
            PB_WARNING_LOG("SI_PBUsimMsgProc:Card Absent");

            SI_PB_ReleaseAll();

            SI_PB_InitStatusInd(USIMM_PB_IDLE);
        }

        result = VOS_OK;
    } else if (USIMM_STKREFRESH_IND == msg->msgName) {
        /* 上报Pb状态给MMA */
        SI_PB_PbStatusInd(VOS_FALSE);

        SI_PB_PihRefreshIndProc((USIMM_STKRefreshInd *)msg);

        result = VOS_OK;
    } else {
        result = SI_PBUsimCnfMsgProc(msg);
    }

    return result;
}

PAM_STATIC const SI_PB_InitList g_pbInitProc[] = {
    {PB_INIT_PB_STATUS,     SI_PB_InitPBStatusJudge,    SI_PB_InitPBStatusProc},
    {PB_INIT_EFPBR,         SI_PB_InitEFpbrReq,         SI_PB_InitEFpbrProc},
    {PB_INIT_XDN_SPACE,     SI_PB_InitXDNSpaceReq,      SI_PB_InitXDNSpaceMsgProc},
    {PB_INIT_ANR_SPACE,     SI_PB_InitANRSpaceReq,      SI_PB_InitANRSpaceMsgProc},
    {PB_INIT_EML_SPACE,     SI_PB_InitEMLSpaceReq,      SI_PB_InitEMLSpaceMsgProc},
    {PB_INIT_IAP_SPACE,     SI_PB_InitIAPSpaceReq,      SI_PB_InitIAPSpaceMsgProc},
    {PB_INIT_EXT_SPACE,     SI_PB_InitEXTSpaceReq,      SI_PB_InitEXTSpaceMsgProc},
    {PB_INIT_IAP_CONTENT,   SI_PB_InitIAPContentReq,    SI_PB_InitIAPContentMsgProc},
    {PB_INIT_EML_CONTENT,   SI_PB_InitEMLContentReq,    SI_PB_InitEMLContentMsgProc},
    {PB_INIT_ANR_CONTENT,   SI_PB_InitANRContentReq,    SI_PB_InitANRContentMsgProc},
    {PB_INIT_EXTR_CONTENT,  SI_PB_InitEXTRContentReq,   SI_PB_InitEXTRContentMsgProc},
    {PB_INIT_XDN_CONTENT,   SI_PB_InitXDNContentReq,    SI_PB_InitXDNContentMsgProc},
    {PB_INIT_EXT_CONTENT,   SI_PB_InitEXTContentReq,    SI_PB_InitEXTContentMsgProc},
    {PB_INIT_PBC_CONTENT,   SI_PB_InitPBCContentReq,    SI_PB_InitPBCContentMsgProc},
    {PB_INIT_IAP_SEARCH,    SI_PB_InitIAPSearchReq,     SI_PB_InitIAPSearchMsgProc},
    {PB_INIT_IAP_CONTENT2,  SI_PB_InitIAPContentReq2,   SI_PB_InitIAPContentMsgProc2},
    {PB_INIT_EML_SEARCH,    SI_PB_InitEmailSearchReq,   SI_PB_InitEmailSearchMsgProc},
    {PB_INIT_EML_CONTENT2,  SI_PB_InitEmailContentReq,  SI_PB_InitEmailContentMsgProc},
    {PB_INIT_ANR_SEARCH,    SI_PB_InitANRSearchReq,     SI_PB_InitANRSearchMsgProc},
    {PB_INIT_ANR_CONTENT2,  SI_PB_InitANRContentReq2,   SI_PB_InitANRContentMsgProc2},
    {PB_INIT_XDN_SEARCH,    SI_PB_InitXDNSearchReq,     SI_PB_InitXDNSearchMsgProc},
    {PB_INIT_XDN_CONTENT2,  SI_PB_InitXDNContentReq2,   SI_PB_InitXDNContentMsgProc2},
    {PB_INIT_PBC_SEARCH,    SI_PB_InitPBCSearchReq,     SI_PB_InitPBCSearchMsgProc},
    {PB_INIT_PBC_CONTENT2,  SI_PB_InitPBCContentReq2,   SI_PB_InitPBCContentMsgProc2},
    {PB_INIT_SYNCH,         SI_PB_InitSYNCHReq,         SI_PB_InitSYNCHProc},
    {PB_INIT_INFO_APP,      SI_PB_InitInfoApp,          SI_PB_InitInfoAppDebug}
};

VOS_UINT32 SI_PBInitMsgProc(const PBMsgBlock *msg)
{
    VOS_UINT32 result = VOS_OK;

    switch (msg->msgName) {
        case USIMM_CARDSTATUS_IND: /* 这部分处理有待讨论，暂时这么写 */
        case USIMM_QUERYFILE_CNF:
        case USIMM_READFILE_CNF:
        case USIMM_UPDATEFILE_CNF:
        case USIMM_SEARCHFILE_CNF:
            if (USIMM_CARDSTATUS_IND == msg->msgName) {
                g_pbInitState.pbInitStep = PB_INIT_PB_STATUS;
            }

            g_pbInitState.reqStatus = PB_REQ_NOT_SEND;

            g_pbInitProc[g_pbInitState.pbInitStep].initProcFun(msg);

            SI_PB_InitPBStateProc(g_pbInitState.pbInitState);

            while (PB_REQ_SEND != g_pbInitState.reqStatus) {
                if (VOS_OK != g_pbInitProc[g_pbInitState.pbInitStep].reqFun()) {
                    PB_ERROR_LOG("SI_PBInitMsgProc:Send Init Request Error");

                    break;
                }
            }

            break;
        case USIMM_ECCNUMBER_IND:
            result = SI_PB_InitEccProc(msg);
            break;

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
        case USIMM_XECCNUMBER_IND:
            result = SI_PB_InitXeccProc(msg);

            if (VOS_OK != result) {
                PB_ERROR_LOG("SI_PBInitMsgProc:Process Message Error");
            }
            break;
#endif

        case USIMM_STKREFRESH_IND:
            SI_PB_PihRefreshIndProc((const USIMM_STKRefreshInd *)msg);
            break;

        default:
            break;
    }

    return result;
}
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
/*
 * 功能描述: 读取NV_ITEM_USIM_PB_CTRL_INFO并初始化g_pbConfigInfo
 * 修改历史:
 * 1. 日    期: 2020年11月17日
 *    修改内容: Creat
 */
PAM_STATIC VOS_VOID SI_PB_InitPbConfigInfoFromNv(VOS_VOID)
{
    /* 读取PB控制信息NV项 */
    if (mdrv_nv_read(NV_ITEM_USIM_PB_CTRL_INFO, (VOS_VOID *)&g_pbConfigInfo, sizeof(SI_PB_NVCtrlInfo)) != NV_OK) {
        /* 不支持复合电话本 */
        PB_ERROR_LOG("SI_PB_InitPbConfigInfoFromNv: Read NV Fail!");
        g_pbConfigInfo.spbFlag = VOS_FALSE;
        g_pbConfigInfo.emailFlag = VOS_FALSE;
        g_pbConfigInfo.anrMaxNum = 1;
        return;
    }

    /* 对NV数据进行判断并归一化 */
    g_pbConfigInfo.spbFlag = (g_pbConfigInfo.spbFlag >= 1) ? VOS_TRUE : VOS_FALSE;
    g_pbConfigInfo.emailFlag = (g_pbConfigInfo.emailFlag >= 1) ? VOS_TRUE : VOS_FALSE;

    if (g_pbConfigInfo.anrMaxNum > SI_PB_ANRMAX) {
        g_pbConfigInfo.anrMaxNum = 1;
    }

    return;
}

#endif

/*
 * 功能描述: 初始化PB相关的全局变量
 * 修改历史:
 * 1. 日    期: 2008年10月15日
 *    修改内容: Creat
 */
VOS_VOID SI_PB_InitGlobeVariable(VOS_VOID)
{
    (VOS_VOID)memset_s(&g_iapContent, sizeof(g_iapContent), 0, sizeof(g_iapContent));

    (VOS_VOID)memset_s(&g_pbCtrlInfo, sizeof(g_pbCtrlInfo), 0, sizeof(g_pbCtrlInfo));

    (VOS_VOID)memset_s(&g_pbInitState, sizeof(g_pbInitState), 0, sizeof(g_pbInitState));

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    if ((PB_INITIALISED != g_pbContent[PB_ECC_CONTENT].initialState) &&
        (PB_INITIALISED != g_pbContent[PB_XECC_CONTENT].initialState)) {
        (VOS_VOID)memset_s(&g_pbContent[PB_ECC_CONTENT], sizeof(g_pbContent), 0, sizeof(g_pbContent));
    } else {
        (VOS_VOID)memset_s(&g_pbContent[PB_ADN_CONTENT], sizeof(SI_PB_Content), 0, sizeof(SI_PB_Content));
        (VOS_VOID)memset_s(&g_pbContent[PB_FDN_CONTENT], sizeof(SI_PB_Content), 0, sizeof(SI_PB_Content));
        (VOS_VOID)memset_s(&g_pbContent[PB_BDN_CONTENT], sizeof(SI_PB_Content), 0, sizeof(SI_PB_Content));
        (VOS_VOID)memset_s(&g_pbContent[PB_MSISDN_CONTENT], sizeof(SI_PB_Content), 0,
                           sizeof(SI_PB_Content));
        (VOS_VOID)memset_s(&g_pbContent[PB_SDN_CONTENT], sizeof(SI_PB_Content), 0, sizeof(SI_PB_Content));
    }

    (VOS_VOID)memset_s(&g_pbSearchCtrlInfo, sizeof(SI_PB_SearchCtrl), 0xFF,
                       sizeof(SI_PB_SearchCtrl));

    SI_PB_InitPbConfigInfoFromNv();
#endif

    (VOS_VOID)memset_s(&g_extContent[0], SI_PB_MAX_NUMBER * sizeof(SI_EXT_Content), 0,
                       SI_PB_MAX_NUMBER * sizeof(SI_EXT_Content));

    (VOS_VOID)memset_s(&g_anrContent[0], SI_PB_ANRMAX * sizeof(SI_ANR_Content), 0,
                       SI_PB_ANRMAX * sizeof(SI_ANR_Content));

    (VOS_VOID)memset_s(&g_emlContent, sizeof(SI_EML_Content), 0, sizeof(SI_EML_Content));

    (VOS_VOID)memset_s(&g_pbReqUnit, sizeof(SI_PB_ReqUint), 0, sizeof(SI_PB_ReqUint));

    (VOS_VOID)memset_s(g_pbCtrlInfo.adnInfo, sizeof(g_pbCtrlInfo.adnInfo), 0xFF,
                       sizeof(g_pbCtrlInfo.adnInfo));

    g_pbCtrlInfo.pbCurType = SI_PB_STORAGE_SM;

    /* 设置电话本类型和其在缓冲中偏移的对应关系 */
    g_pbContent[PB_ECC_CONTENT].pbType = PB_ECC;

    g_pbContent[PB_ECC_CONTENT].activeStatus = SI_PB_ACTIVE;

    g_pbContent[PB_ADN_CONTENT].pbType = PB_ADN;

    g_pbContent[PB_ADN_CONTENT].activeStatus = SI_PB_ACTIVE;

    g_pbContent[PB_FDN_CONTENT].pbType = PB_FDN;

    g_pbContent[PB_FDN_CONTENT].activeStatus = SI_PB_ACTIVE;

    g_pbContent[PB_BDN_CONTENT].pbType = PB_BDN;

    g_pbContent[PB_BDN_CONTENT].activeStatus = SI_PB_ACTIVE;

    g_pbContent[PB_MSISDN_CONTENT].pbType = PB_MISDN;

    g_pbContent[PB_MSISDN_CONTENT].activeStatus = SI_PB_ACTIVE;

    g_pbContent[PB_SDN_CONTENT].pbType = PB_SDN;

    g_pbContent[PB_SDN_CONTENT].activeStatus = SI_PB_ACTIVE;

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    g_pbContent[PB_XECC_CONTENT].pbType = PB_XECC;

    g_pbContent[PB_XECC_CONTENT].activeStatus = SI_PB_ACTIVE;
#endif

    return;
}

/*
 * 功能描述: 初始化PB相关的全局变量
 * 修改历史:
 * 1. 日    期: 2008年10月15日
 *    修改内容: Creat
 */
VOS_UINT32 WuepsPBPidInit(enum VOS_InitPhaseDefine initPhrase)
{
    switch (initPhrase) {
        case VOS_IP_LOAD_CONFIG:
            SI_PB_InitGlobeVariable();
            break;

#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (VOS_WIN32 == VOS_OS_VER))
        case VOS_IP_RESTART:
            break;
#endif

        default:
            break;
    }
    return VOS_OK;
}

/*
 * 功能描述: 电话本模块的消息处理模块
 * 修改历史:
 * 1.日    期: 2007年10月15日
 *   修改内容: Creat
 */
#if defined(DMT) || (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON)
PAM_STATIC const SI_PB_ProcList g_pbReqProc[] = {
    {SI_PB_READ_REQ,        SI_PB_ReadProc},        /* 读取电话本 */
    {SI_PB_SEARCH_REQ,      SI_PB_SearchProc},      /* 搜索电话本 */
    {SI_PB_SREAD_REQ,       SI_PB_SReadProc},       /* 读取扩展电话本 */
    {SI_PB_SET_REQ,         SI_PB_SetProc},         /* 设置当前电话本 */
    {SI_PB_MODIFY_REQ,      SI_PB_ModifyProc},      /* 更改电话本 */
    {SI_PB_DELETE_REQ,      SI_PB_DeleteProc},      /* 删除电话本 */
    {SI_PB_QUERY_REQ,       SI_PB_QueryProc},       /* 查询当前电话本信息 */
    {SI_PB_ADD_REQ,         SI_PB_AddProc},         /* 添加电话本 */
    {SI_PB_SMODIFY_REQ,     SI_PB_SModifyProc},     /* 更新扩展电话本 */
    {SI_PB_SADD_REQ,        SI_PB_SAddProc},        /* 添加电话本 */
    {SI_PB_FDN_CHECK_REQ,   SI_PB_FDNCheckProc},    /* FDN号码检查 */
    {SI_PB_ECALL_QRY_REQ,   SI_PB_EcallNumberQryProc}, /* ECALL号码获取 */
};
#else
#if (OSA_CPU_ACPU == VOS_OSA_CPU)
PAM_STATIC const SI_PB_ProcList g_pbReqProc[] = {
    {SI_PB_READ_REQ,        SI_PB_ReadProc},        /* 读取电话本 */
    {SI_PB_SEARCH_REQ,      SI_PB_SearchProc},      /* 搜索电话本 */
    {SI_PB_SREAD_REQ,       SI_PB_SReadProc},       /* 读取扩展电话本 */
    {SI_PB_UPDATE_AGOBAL,   SI_PBUpdateAGlobal},    /* 同步全局变量 */
    {SI_PB_UPDATE_CURPB,    SI_PBUpdateACurPB},     /* 更新当前电话本设置 */
};
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
PAM_STATIC const SI_PB_ProcList g_pbReqProc[] = {
    {SI_PB_SET_REQ,         SI_PB_SetProc},         /* 设置当前电话本 */
    {SI_PB_MODIFY_REQ,      SI_PB_ModifyProc},      /* 更改电话本 */
    {SI_PB_DELETE_REQ,      SI_PB_DeleteProc},      /* 删除电话本 */
    {SI_PB_QUERY_REQ,       SI_PB_QueryProc},       /* 查询当前电话本信息 */
    {SI_PB_ADD_REQ,         SI_PB_AddProc},         /* 添加电话本 */
    {SI_PB_SMODIFY_REQ,     SI_PB_SModifyProc},     /* 更新扩展电话本 */
    {SI_PB_SADD_REQ,        SI_PB_SAddProc},        /* 添加电话本 */
    {SI_PB_FDN_CHECK_REQ,   SI_PB_FDNCheckProc},    /* FDN号码检查 */
    {SI_PB_ECALL_QRY_REQ,   SI_PB_EcallNumberQryProc}, /* ECALL号码获取 */
};
#endif
#endif

VOS_VOID I0_SI_PB_PidMsgProc(struct MsgCB *pbMsg)
{
    VOS_UINT32 result = VOS_ERR;
    VOS_UINT32 i;

    /* 只有Ccpu才处理从USIMM模块的消息 */
#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    USIMM_CardStatusInd *cardMsg = VOS_NULL_PTR;

    if (pbMsg == VOS_NULL_PTR) {
        PB_WARNING_LOG("I0_SI_PB_PidMsgProc: null msg");

        return;
    }

    if ((WUEPS_PID_USIM == pbMsg->ulSenderPid) || (MAPS_PIH_PID == pbMsg->ulSenderPid)) {
        if (USIMM_CARDSTATUS_IND == ((PBMsgBlock *)pbMsg)->msgName) {
            cardMsg = (USIMM_CardStatusInd *)pbMsg;

            if (USIMM_CARD_STATUS_INIT_IND != cardMsg->addInfo.statusType) {
                PB_NORMAL1_LOG("I0_SI_PB_PidMsgProc:No Need Init Pb ", cardMsg->addInfo.statusType);

                return;
            }
        }

        OM_RecordInfoStart(VOS_EXC_DUMP_MEM_NUM_2, pbMsg->ulSenderPid, MAPS_PB_PID,
                           ((PBMsgBlock *)pbMsg)->msgName);

        if (PB_INIT_FINISHED != g_pbInitState.pbInitStep) { /* 初始化的读文件消息等 */
            result = SI_PBInitMsgProc((const PBMsgBlock *)pbMsg);
        } else {
            result = SI_PBUsimMsgProc((const PBMsgBlock *)pbMsg);
        }

        OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_2);
    } else {
        for (i = 0; i < (sizeof(g_pbReqProc) / sizeof(SI_PB_ProcList)); i++) {
            if (g_pbReqProc[i].msgType == ((PBMsgBlock *)pbMsg)->msgName) {
                result = g_pbReqProc[i].procFun((PBMsgBlock *)pbMsg); /* 处理外部请求 */
                break;
            }
        }
    }
#else
    for (i = 0; i < (sizeof(g_pbReqProc) / sizeof(SI_PB_ProcList)); i++) {
        if (g_pbReqProc[i].msgType == ((PBMsgBlock *)pbMsg)->msgName) {
            result = g_pbReqProc[i].procFun((PBMsgBlock *)pbMsg); /* 处理外部请求 */
            break;
        }
    }
#endif

    if (VOS_OK != result) {
        PB_ERROR_LOG("SI_PB_PidMsgProc:Process Message Error");
    }

    return;
}

#endif /* (FEATURE_PHONE_USIM == FEATURE_OFF) */

