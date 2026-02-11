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


#include <mdrv.h>
#include <mdrv_diag_system.h>
#include "diag_msgbsp.h"
#include "diag_debug.h"
#include "msp_diag_comm.h"
#include "diag_fs_proc.h"
#include "nv_stru_sys.h"
#include "nv_stru_gucnas.h"
#include "acore_nv_stru_gucnas.h"
#include "soc_socp_adapter.h"
#include "diag_connect.h"

#define    THIS_FILE_ID        MSP_FILE_ID_DIAG_MSGBSP_C

static VOS_BOOL g_ulPrivilegeRslt = FALSE;
VOS_UINT32 g_DiagHdsCmdNumC = 0;
VOS_UINT32 g_DiagHdsCmdListC[DIAG_HDS_CMD_MAX] = {0};
VOS_UINT32 g_DiagHdsCmdNumA = 0;
VOS_UINT32 g_DiagHdsCmdListA[DIAG_HDS_CMD_MAX] = {0};
DIAG_FRAME_INFO_STRU g_DiagNvAuthHead = {{0}};
DIAG_BSP_CTRL   g_DiagBspCtrl =
{
    1,
    SOCP_CODER_SRC_BSP_ACORE,
};


/*lint -save -e433*/
VOS_UINT32 diag_NvRdProc(VOS_UINT8* pstReq)
{
    VOS_UINT32 ret = ERR_MSP_SUCCESS;
    DIAG_CMD_NV_QRY_REQ_STRU* pstNVQryReq = NULL;
    DIAG_CMD_NV_QRY_CNF_STRU* pstNVQryCnf = NULL;
    VOS_UINT32 ulNVLen = 0, i = 0;
    VOS_UINT32 ulTotalSize = 0, ulOffset = 0;
    VOS_UINT8 *pData;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};
    DIAG_FRAME_INFO_STRU *pstDiagHead = NULL;

    pstDiagHead = (DIAG_FRAME_INFO_STRU*)(pstReq);

    mdrv_diag_PTR(EN_DIAG_PTR_NV_RD_PROC, 1, pstDiagHead->ulCmdId, 0);

    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_BSP;

    if(pstDiagHead->ulMsgLen < sizeof(MSP_DIAG_DATA_REQ_STRU) + sizeof(DIAG_CMD_NV_QRY_REQ_STRU))
    {
        diag_error("Data error! ulDataLen: 0x%x \n", pstDiagHead->ulMsgLen);
        ret = ERR_MSP_INVALID_PARAMETER;
        goto DIAG_ERROR;
    }

    pstNVQryReq = (DIAG_CMD_NV_QRY_REQ_STRU*)(pstDiagHead->aucData + sizeof(MSP_DIAG_DATA_REQ_STRU));
    if((0 == pstNVQryReq->ulCount)
        || (pstDiagHead->ulMsgLen < sizeof(MSP_DIAG_DATA_REQ_STRU)
            + sizeof(DIAG_CMD_NV_QRY_REQ_STRU) + pstNVQryReq->ulCount*sizeof(VOS_UINT32)))
    {
        diag_error("Data error!, count: 0x%x ulDataLen: 0x%x \n",pstNVQryReq->ulCount, pstDiagHead->ulMsgLen);
        ret = ERR_MSP_INVALID_PARAMETER;
        goto DIAG_ERROR;
    }

    for(i = 0; i < pstNVQryReq->ulCount; i++)
    {
        /*根据请求ID获取NV项长度*/
        ret = mdrv_nv_get_length(pstNVQryReq->ulNVId[i], &ulNVLen);
        if(ERR_MSP_SUCCESS != ret)
        {
            diag_printf("[%s]:ulNVId=0x%x,ulNVLen=%d!\n",__FUNCTION__, pstNVQryReq->ulNVId[i], ulNVLen);
            goto DIAG_ERROR;
        }

        ulTotalSize += ulNVLen + sizeof(VOS_UINT32) + sizeof(VOS_UINT32); /* NV内容的长度加上(NVID和len各占用四字节) */
    }

    /* DIAG_CMD_NV_QRY_CNF_STRU的实际长度 */
    ulTotalSize += (sizeof(DIAG_CMD_NV_QRY_CNF_STRU) - sizeof(VOS_UINT32) - sizeof(VOS_UINT32));

    /*lint -save -e423 *//*432为内存泄露，经检视此处无内存泄露*/
    pstNVQryCnf = VOS_MemAlloc(MSP_PID_DIAG_APP_AGENT, DYNAMIC_MEM_PT, ulTotalSize);
    /*lint -restore  */
    if(NULL  == pstNVQryCnf)
    {
        diag_printf("[%s]:malloc error!\n",__FUNCTION__);
        return ERR_MSP_DIAG_ALLOC_MALLOC_FAIL;
    }

    pstNVQryCnf->ulModemid = pstNVQryReq->ulModemid;
    pstNVQryCnf->ulCount   = pstNVQryReq->ulCount;

    pData = (VOS_UINT8*)(&pstNVQryCnf->ulNVId);
    ulOffset = 0;

    for(i = 0; i < pstNVQryReq->ulCount; i++)
    {
        /*根据请求ID获取NV项长度*/
        ret = mdrv_nv_get_length(pstNVQryReq->ulNVId[i], &ulNVLen);
        if(ERR_MSP_SUCCESS != ret)
        {
            diag_printf("[%s]:ulNVId=0x%x,ulNVLen=%d!\n",__FUNCTION__, pstNVQryReq->ulNVId[i], ulNVLen);
            ret = ERR_MSP_DIAG_GET_NVLEN_ERR;
            goto DIAG_ERROR;
        }

        *(VOS_UINT32*)(pData + ulOffset) = pstNVQryReq->ulNVId[i]; /* [false alarm]:屏蔽Fortify */
        ulOffset += sizeof(VOS_UINT32);

        *(VOS_UINT32*)(pData + ulOffset) = ulNVLen; /* [false alarm]:屏蔽Fortify */
        ulOffset += sizeof(VOS_UINT32);

        /*lint -save -e662 */
        ret = mdrv_nv_readex(pstNVQryReq->ulModemid, pstNVQryReq->ulNVId[i], (pData + ulOffset), ulNVLen);
        /*lint -restore  */
        if(ret != ERR_MSP_SUCCESS)
        {
            diag_printf("[%s]:NV READ ERR 0x%x,ulNVId=0x%x\n",__FUNCTION__, ret, pstNVQryReq->ulNVId[i]);
            //ret = ERR_MSP_NV_ERROR_READ;
            goto DIAG_ERROR;
        }

        ulOffset += ulNVLen;
    }

    DIAG_MSG_COMMON_PROC(stDiagInfo, (*pstNVQryCnf), pstDiagHead);

    pstNVQryCnf->ulRc    = ret;

    ret = DIAG_MsgReport(&stDiagInfo, pstNVQryCnf, ulTotalSize);
    mdrv_diag_PTR(EN_DIAG_PTR_NV_RD_SUCESS_CNF, 1, pstDiagHead->ulCmdId, ret);

    VOS_MemFree(MSP_PID_DIAG_APP_AGENT, pstNVQryCnf);

    return ret;

DIAG_ERROR:
    if(NULL == pstNVQryCnf)
    {
        return ERR_MSP_FAILURE;
    }
    DIAG_MSG_COMMON_PROC(stDiagInfo, (*pstNVQryCnf), pstDiagHead);

    pstNVQryCnf->ulRc    = ret;
    pstNVQryCnf->ulCount = 0;

    ret = DIAG_MsgReport(&stDiagInfo, pstNVQryCnf, sizeof(DIAG_CMD_NV_QRY_CNF_STRU));
    mdrv_diag_PTR(EN_DIAG_PTR_NV_RD_FAIL_CNF, 1, pstDiagHead->ulCmdId, ret);

    VOS_MemFree(MSP_PID_DIAG_APP_AGENT, pstNVQryCnf);

    return ret;
}
 /*lint -restore */


VOS_UINT32 diag_GetNvListProc(VOS_UINT8* pstReq)
{
    VOS_UINT32 ret, ulNvNum, ulTotalLen;
    DIAG_CMD_NV_LIST_CNF_STRU* pstNVCnf = NULL;
    DIAG_CMD_NV_LIST_CNF_STRU stNVCnf = {0};
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};
    DIAG_FRAME_INFO_STRU *pstDiagHead;

    pstDiagHead = (DIAG_FRAME_INFO_STRU*)(pstReq);

    mdrv_diag_PTR(EN_DIAG_PTR_GET_NV_LIST_PROC, 1, pstDiagHead->ulCmdId, 0);

    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_BSP;

    ulNvNum = mdrv_nv_get_nvid_num();
    if (0 == ulNvNum)
    {
        diag_printf("[%s]: ulNvNum=%d!\n",__FUNCTION__, ulNvNum);
        ret = ERR_MSP_DIAG_GET_NV_LIST_ERR;
        goto DIAG_ERROR;
    }

    /*APP_HEAD + Result + NV nums + NV ID/LEN lists*/
    ulTotalLen = sizeof(DIAG_CMD_NV_LIST_CNF_STRU) + (ulNvNum*sizeof(NV_LIST_INFO_STRU));

    pstNVCnf = (DIAG_CMD_NV_LIST_CNF_STRU*)VOS_MemAlloc(MSP_PID_DIAG_APP_AGENT, DYNAMIC_MEM_PT, ulTotalLen);

    if (VOS_NULL_PTR == pstNVCnf)
    {
        diag_printf("[%s]:malloc error!\n",__FUNCTION__);
        ret = ERR_MSP_DIAG_ALLOC_MALLOC_FAIL;
        goto DIAG_ERROR;
    }

    /*获取每个NV项的ID和长度*/
    ret = mdrv_nv_get_nvid_list(pstNVCnf->astNvList);
    if (NV_OK != ret)
    {
        VOS_MemFree(MSP_PID_DIAG_APP_AGENT, pstNVCnf);
        goto DIAG_ERROR;
    }

    DIAG_MSG_COMMON_PROC(stDiagInfo, (*pstNVCnf), pstDiagHead);

    pstNVCnf->ulCount    = ulNvNum;
    pstNVCnf->ulRc       = ERR_MSP_SUCCESS;

    ret = DIAG_MsgReport(&stDiagInfo, pstNVCnf, ulTotalLen);
    mdrv_diag_PTR(EN_DIAG_PTR_GET_NV_LIST_SUCESS_CNF, 1, pstDiagHead->ulCmdId, ret);

    VOS_MemFree(MSP_PID_DIAG_APP_AGENT, pstNVCnf);

    return ret;

DIAG_ERROR:

    DIAG_MSG_COMMON_PROC(stDiagInfo, stNVCnf, pstDiagHead);

    stNVCnf.ulCount      = 0;
    stNVCnf.ulRc         = ret;

    ret = DIAG_MsgReport(&stDiagInfo, &stNVCnf, sizeof(stNVCnf));
    mdrv_diag_PTR(EN_DIAG_PTR_GET_NV_LIST_FAIL_CNF, 1, pstDiagHead->ulCmdId, ret);

    return ret;
}


VOS_VOID diag_InitAuthVariable(VOS_VOID)
{
    IMEI_STRU stIMEI;
    VOS_UINT8 aucDefaultIMEI[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    /*假如IMEI为默认值，则不需要鉴权*/
    if (NV_OK ==  mdrv_nv_read(en_NV_Item_IMEI, (VOS_VOID*)&stIMEI, sizeof(stIMEI)))
    {
        if (0 == VOS_MemCmp((VOS_CHAR*)aucDefaultIMEI, &stIMEI, sizeof(stIMEI)))
        {
            g_ulPrivilegeRslt = TRUE;
        }
    }

    return;
}


VOS_UINT32 diag_IsAuthNv(VOS_UINT32 ulNvId)
{
    VOS_UINT32 i;
    VOS_UINT32 aulNvAuthIdNum;
    VOS_UINT32 * aulNvAuthIdList;

    mdrv_nv_get_nvauth_list(&aulNvAuthIdList, &aulNvAuthIdNum);
    if((aulNvAuthIdList == NULL) || (aulNvAuthIdNum == 0))
    {
        return VOS_NO;
    }

    for (i = 0; i < aulNvAuthIdNum; i++)
    {
        if (ulNvId == aulNvAuthIdList[i])
        {
            if (TRUE == g_ulPrivilegeRslt)
            {
                return VOS_YES;
            }
            return VOS_NO;
        }
    }

    return VOS_YES;
}


VOS_UINT32 diag_NvWrProc(VOS_UINT8* pstReq)
{
    VOS_UINT32 ret;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};
    DIAG_CMD_NV_WR_REQ_STRU* pstNVWRReq = NULL;
    DIAG_FRAME_INFO_STRU *pstDiagHead = NULL;
    DIAG_CMD_NV_WR_CNF_STRU stNVWRCnf = {0};
    VOS_UINT32 i = 0;
    VOS_UINT32 ulOffset = 0;
    VOS_UINT32 ulNvid, ulLen;
    DIAG_NV_WRITE_TABLE_STRU *pstNvWrTable = NULL;

    pstDiagHead = (DIAG_FRAME_INFO_STRU*)(pstReq);
    mdrv_diag_PTR(EN_DIAG_PTR_NV_WR_PROC, 1, pstDiagHead->ulCmdId, 0);

    if(pstDiagHead->ulMsgLen < sizeof(MSP_DIAG_DATA_REQ_STRU) + sizeof(DIAG_CMD_NV_WR_REQ_STRU))
    {
        diag_error("datalen too small ,datalen=0x%x\n", pstDiagHead->ulMsgLen);
        return ERR_MSP_INVALID_PARAMETER;
    }

    pstNVWRReq = (DIAG_CMD_NV_WR_REQ_STRU*)(pstDiagHead->aucData + sizeof(MSP_DIAG_DATA_REQ_STRU));

    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_BSP;

    ulOffset = sizeof(MSP_DIAG_DATA_REQ_STRU) +  sizeof(DIAG_CMD_NV_WR_REQ_STRU) - sizeof(DIAG_NV_WRITE_TABLE_STRU);
    pstNvWrTable = (DIAG_NV_WRITE_TABLE_STRU *)(pstDiagHead->aucData + ulOffset);

    for(i = 0; i < pstNVWRReq->ulCount; i++)
    {
        if(pstDiagHead->ulMsgLen < ulOffset)
        {
            diag_error("rev datalen is small,len:0x%x cur offset:0x%x\n", pstDiagHead->ulMsgLen, ulOffset);
            ret = ERR_MSP_INALID_LEN_ERROR;
            goto DIAG_ERROR2;
        }

        ulNvid    = pstNvWrTable->ulNVId;
        ulLen     = pstNvWrTable->ulDataSize;

        if (VOS_YES != diag_IsAuthNv(ulNvid))
        {
            ret = ERR_MSP_DIAG_NOT_AUTH_NV_ERR;
            printk(KERN_ERR "[%s]:not auth nv ,ulNVId=0x%x\n",__FUNCTION__, ulNvid);
            goto DIAG_ERROR2;
        }

        diag_crit("NV Write ulNVId=0x%x\n", ulNvid);

        /*写入NV项*/
        /*lint -save -e662 */
        ret = mdrv_nv_writeex(pstNVWRReq->ulModemid, ulNvid, pstNvWrTable->aucData, ulLen);
        /*lint -restore  */
        if(ret != ERR_MSP_SUCCESS)
        {
            printk(KERN_ERR "[%s]:NV Write ERR 0x%x,ulNVId=0x%x\n",__FUNCTION__, ret, ulNvid);
            ret = ERR_MSP_DIAG_WRITE_NV_ERR;
            goto DIAG_ERROR2;
        }
        /*将Nv项刷写到flash中*/
        ret = mdrv_nv_flush();
        if(ret)
        {
            printk(KERN_ERR "[%s]:flush nv ERR 0x%x,ulNVId=0x%x\n",__FUNCTION__, ret, ulNvid);
            ret = ERR_MSP_DIAG_FLUSH_NV_ERR;
            goto DIAG_ERROR2;
        }
        ulOffset += sizeof(DIAG_NV_WRITE_TABLE_STRU) + ulLen;
        pstNvWrTable = (DIAG_NV_WRITE_TABLE_STRU *)(pstDiagHead->aucData + ulOffset);
    }

    DIAG_MSG_COMMON_PROC(stDiagInfo, stNVWRCnf, pstDiagHead);

    stNVWRCnf.ulModemid  = pstNVWRReq->ulModemid;
    stNVWRCnf.ulRc       = ret;

    ret = DIAG_MsgReport(&stDiagInfo, &stNVWRCnf, sizeof(stNVWRCnf));
    mdrv_diag_PTR(EN_DIAG_PTR_NV_WR_SUCESS_CNF, 1, pstDiagHead->ulCmdId, ret);

    return ret;

DIAG_ERROR2:

    DIAG_MSG_COMMON_PROC(stDiagInfo, stNVWRCnf, pstDiagHead);

    stNVWRCnf.ulModemid  = pstNVWRReq->ulModemid;
    stNVWRCnf.ulRc       = ret;

    ret = DIAG_MsgReport(&stDiagInfo, &stNVWRCnf, sizeof(stNVWRCnf));
    mdrv_diag_PTR(EN_DIAG_PTR_NV_WR_FAIL_CNF, 1, pstDiagHead->ulCmdId, ret);

    return ret;
}
/*lint -save -e826*/
VOS_VOID diag_AuthNvCfg(MsgBlock* pMsgBlock)
{
    DIAG_CMD_NV_AUTH_CNF_STRU  stAuthCnf = {0};
    MSP_DIAG_CNF_INFO_STRU     stDiagInfo = {0};
    MTA_DIAG_RSA_VERIFY_CNF_STRU *pstMsg = (MTA_DIAG_RSA_VERIFY_CNF_STRU *)pMsgBlock;

    mdrv_diag_PTR(EN_DIAG_PTR_AUTH_NV_CFG, 0, 0, 0);

    if( pMsgBlock->ulLength + VOS_MSG_HEAD_LENGTH < sizeof(MTA_DIAG_RSA_VERIFY_CNF_STRU))
    {
        diag_error("len error!:0x%x\n", pMsgBlock->ulLength);
        return;
    }

    if (TRUE == pstMsg->ulVerifyRslt)
    {
        g_ulPrivilegeRslt = pstMsg->ulVerifyRslt;
        stAuthCnf.ulRc = VOS_OK;
    }
    else if (FALSE == pstMsg->ulVerifyRslt)
    {
        g_ulPrivilegeRslt = pstMsg->ulVerifyRslt;
        stAuthCnf.ulRc = VOS_ERR;
    }
    else
    {
        diag_printf("[%s]:nv_auth result exception!\n",__FUNCTION__);
        stAuthCnf.ulRc = VOS_ERR;
    }

    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_BSP;

    DIAG_MSG_COMMON_PROC(stDiagInfo, stAuthCnf, ((DIAG_FRAME_INFO_STRU *)(&g_DiagNvAuthHead)));

    if (VOS_OK != DIAG_MsgReport(&stDiagInfo, &stAuthCnf, (VOS_INT)sizeof(stAuthCnf)))
    {
        diag_printf("%s:diag nv auth msg report fail!\n", __FUNCTION__);
    }

}

VOS_UINT32 diag_NvAuthProc(VOS_UINT8* pstReq)
{
    VOS_UINT32 ulRet;
    MSP_DIAG_CNF_INFO_STRU     stDiagInfo = {0};
    DIAG_CMD_NV_AUTH_REQ_STRU *pstAuthReq;
    DIAG_CMD_NV_AUTH_CNF_STRU  stAuthCnf = {0};
    DIAG_MTA_RSA_VERIFY_REQ_STRU *pstNvAuth;
    DIAG_FRAME_INFO_STRU *pstNvHead = (DIAG_FRAME_INFO_STRU *)pstReq;

    mdrv_diag_PTR(EN_DIAG_PTR_NV_AUTH_PROC, 1, pstNvHead->ulCmdId, 0);

    if(pstNvHead->ulMsgLen < sizeof(MSP_DIAG_DATA_REQ_STRU) + sizeof(pstAuthReq->ulLen))
    {
        diag_error("data too small datalen:0x%x\n", pstNvHead->ulMsgLen);
        return ERR_MSP_INALID_LEN_ERROR;
    }

    pstAuthReq = (DIAG_CMD_NV_AUTH_REQ_STRU*)(pstNvHead->aucData + sizeof(MSP_DIAG_DATA_REQ_STRU));

    /* 工具下发的不对 pstNvHead->ulMsgLen == 0xC，暂时注释掉，工具上库后打开
    if(pstNvHead->ulMsgLen < sizeof(MSP_DIAG_DATA_REQ_STRU) + sizeof(pstAuthReq->ulLen) + pstAuthReq->ulLen)
    {
        diag_error("data too small datalen:0x%x auth len:0x%x\n", pstNvHead->ulMsgLen, pstAuthReq->ulLen);
        return ERR_MSP_INALID_LEN_ERROR;
    }
    */
    pstNvAuth = (DIAG_MTA_RSA_VERIFY_REQ_STRU *)VOS_AllocMsg(MSP_PID_DIAG_APP_AGENT,(VOS_INT)(sizeof(DIAG_MTA_RSA_VERIFY_REQ_STRU) - VOS_MSG_HEAD_LENGTH));
    if(pstNvAuth == NULL)
    {
        diag_printf("%s: alloc msg fail\n", __FUNCTION__);
        return ERR_MSP_FAILURE;
    }

    pstNvAuth->ulReceiverPid  = I0_UEPS_PID_MTA;
    pstNvAuth->ulMsgId        = ID_DIAG_MTA_RSA_VERIFY_REQ;
    (VOS_VOID)VOS_MemSet_s(pstNvAuth->aucSecString, sizeof(pstNvAuth->aucSecString), 0, sizeof(pstNvAuth->aucSecString));
    (VOS_VOID)VOS_MemCpy_s(pstNvAuth->aucSecString, sizeof(pstNvAuth->aucSecString),pstAuthReq->aucAuth, pstAuthReq->ulLen);

    if(ERR_MSP_SUCCESS != VOS_SendMsg(MSP_PID_DIAG_APP_AGENT, pstNvAuth))
    {
        diag_printf("%s: diag send msg to mta fail\n", __FUNCTION__);

        stAuthCnf.ulRc = VOS_ERR;

        stDiagInfo.ulMsgType = DIAG_MSG_TYPE_BSP;

        DIAG_MSG_COMMON_PROC(stDiagInfo, stAuthCnf, pstNvHead);

        ulRet = DIAG_MsgReport(&stDiagInfo, &stAuthCnf, (VOS_INT)sizeof(stAuthCnf));
        mdrv_diag_PTR(EN_DIAG_PTR_NV_AUTH_FAIL_CNF, 1, pstNvHead->ulCmdId, ulRet);

        return ulRet;
    }

    else
    {
        VOS_MemCpy_s(&g_DiagNvAuthHead, sizeof(g_DiagNvAuthHead), pstNvHead, sizeof(*pstNvHead));
        return ERR_MSP_SUCCESS;
    }
}
/*lint -restore +e826*/
VOS_VOID diag_BspRecvCmdList(MsgBlock* pMsgBlock)
{
    DIAG_BSP_CMDLIST_STRU *pstMsg = (DIAG_BSP_CMDLIST_STRU *)pMsgBlock;

    if( pstMsg->ulLength  <  sizeof(pstMsg->ulMsgId) + sizeof(pstMsg->ulCmdNum) + pstMsg->ulCmdNum * sizeof(VOS_UINT32))
    {
        diag_printf("rec datalen is too small,len:0x%x\n", pstMsg->ulLength);
        return;
    }

    g_DiagHdsCmdNumC = pstMsg->ulCmdNum;
    VOS_MemCpy_s(g_DiagHdsCmdListC, sizeof(g_DiagHdsCmdListC),  pstMsg->ulCmdList, g_DiagHdsCmdNumC*sizeof(VOS_UINT32));

    diag_printf("%s Rcv ccore cmd list, num: 0x%x\n",__FUNCTION__, g_DiagHdsCmdNumC);

    return;
}

VOS_VOID diag_BspCmdListInit(VOS_VOID)
{
    g_DiagHdsCmdNumA = DIAG_HDS_CMD_MAX;
    mdrv_hds_get_cmdlist(g_DiagHdsCmdListA, &g_DiagHdsCmdNumA);
}

VOS_BOOL diag_BspIsAcoreCmd(VOS_UINT32 ulCmdId)
{
    unsigned int i;

    if(!ulCmdId)
        return VOS_FALSE;

    for(i=0; i<g_DiagHdsCmdNumA; i++)
    {
        if(g_DiagHdsCmdListA[i] == ulCmdId)
        {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}


VOS_BOOL diag_BspIsCcoreCmd(VOS_UINT32 ulCmdId)
{
    unsigned int i;

    if(!ulCmdId)
        return VOS_FALSE;

    for(i=0; i<g_DiagHdsCmdNumC; i++)
    {
        if(g_DiagHdsCmdListC[i] == ulCmdId)
        {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}

VOS_UINT32 diag_BspMsgProc(DIAG_FRAME_INFO_STRU *pData)
{
    VOS_UINT32 ulRet = ERR_MSP_INVALID_PARAMETER ;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};
    DIAG_BSP_COMM_CNF_STRU stCnf = {0};
    DIAG_FRAME_INFO_STRU *pstDiagHead = pData;
    DIAG_BSP_MSG_A_TRANS_C_STRU *pstInfo;
    VOS_UINT32 ulLen = 0;
    VOS_UINT32 acmd_flag = 0;
    VOS_UINT32 ccmd_flag = 0;

    mdrv_diag_PTR(EN_DIAG_PTR_BSP_MSG_PROC, 1, pData->ulCmdId, 0);

    if(DIAG_MSG_TYPE_BSP != pData->stID.pri4b)
    {
        diag_printf("%s Rcv Error Msg Id 0x%x\n",__FUNCTION__,pData->ulCmdId);
        return ulRet;
    }

    acmd_flag = diag_BspIsAcoreCmd(pData->ulCmdId);
    ccmd_flag = diag_BspIsCcoreCmd(pData->ulCmdId);

    if  ((VOS_TRUE == acmd_flag) && (VOS_FALSE == ccmd_flag))
    {
        ulRet = mdrv_hds_msg_proc((VOS_VOID *)pData);
        if(ulRet != 0)
        {
            diag_printf("acore msg_proc fail!\n",__FUNCTION__);
        }

        return ulRet;
    }
    else if ((VOS_FALSE == acmd_flag) && (VOS_TRUE == ccmd_flag))
    {
        /*通知ccore*/
        DIAG_MSG_BSP_ACORE_CFG_PROC(ulLen, pData, pstInfo, ulRet);
        return VOS_OK;
    }
    else if ((VOS_TRUE == acmd_flag) && (VOS_TRUE == ccmd_flag))
    {
        /*A核处理成功后通知ccore，A核处理不成功直接向工具回复*/
        ulRet = mdrv_hds_msg_proc((VOS_VOID*)pData);
        if(ulRet != 0)
        {
            diag_printf("comm_acore msg_proc fail!\n",__FUNCTION__);
            return ulRet;
        }

        DIAG_MSG_BSP_ACORE_CFG_PROC(ulLen, pData, pstInfo, ulRet);
        return VOS_OK;
    }
    else
    {
        //todo...
    }

    return ulRet;

DIAG_ERROR:

    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_BSP;

    DIAG_MSG_COMMON_PROC(stDiagInfo, stCnf, pstDiagHead);

    stCnf.ulRet = ERR_MSP_FAILURE;

    ulRet = DIAG_MsgReport(&stDiagInfo, &stCnf, sizeof(stCnf));

    return ulRet;

}

VOS_UINT32 diag_BspConnMgr(VOS_UINT8 *pData)
{
    DIAG_CONN_MSG_SEND_T *pstRevMsg;
    VOS_UINT32 ulRet;
    DIAG_CONNECT_RESULT stCnf;

    pstRevMsg = (DIAG_CONN_MSG_SEND_T *)pData;

    stCnf.ulChannelId = g_DiagBspCtrl.ulChannelId;
    if(ID_MSG_DIAG_CMD_CONNECT_REQ == pstRevMsg->ulMsgId)
    {
        ulRet = (VOS_UINT32)mdrv_hds_printlog_conn();
        if(ulRet)
        {
            stCnf.ulResult = ulRet;
            return diag_ConnCnf(DIAG_CONN_ID_ACPU_BSP, pstRevMsg->ulSn, g_DiagBspCtrl.ulChannelNum, &stCnf);
        }

        stCnf.ulResult = (VOS_UINT32)mdrv_hds_translog_conn();
        return diag_ConnCnf(DIAG_CONN_ID_ACPU_BSP, pstRevMsg->ulSn, g_DiagBspCtrl.ulChannelNum, &stCnf);
    }
    else
    {

        ulRet = (VOS_UINT32)mdrv_hds_printlog_disconn();
        if(ulRet)
        {
            stCnf.ulResult = ulRet;
            return diag_ConnCnf(DIAG_CONN_ID_ACPU_BSP, pstRevMsg->ulSn, g_DiagBspCtrl.ulChannelNum, &stCnf);
        }

        stCnf.ulResult = (VOS_UINT32)mdrv_hds_translog_disconn();
        return diag_ConnCnf(DIAG_CONN_ID_ACPU_BSP, pstRevMsg->ulSn, g_DiagBspCtrl.ulChannelNum, &stCnf);
    }
}
/*****************************************************************************
 Function Name   : diag_BspMsgInit
 Description     : MSP dsp部分初始化
 Input           : None
 Output          : None
 Return          : None
 History         :

*****************************************************************************/
VOS_VOID diag_BspMsgInit(VOS_VOID)
{
    VOS_UINT32 ulRet;

    /*注册message消息回调*/
    DIAG_MsgProcReg(DIAG_MSG_TYPE_BSP,diag_BspMsgProc);
    mdrv_hds_cnf_register((hds_cnf_func)DIAG_MsgReport);
    diag_nvInit();
    diag_fsInit();
    diag_BspCmdListInit();

    ulRet = diag_ConnMgrSendFuncReg(DIAG_CONN_ID_ACPU_BSP, g_DiagBspCtrl.ulChannelNum, &g_DiagBspCtrl.ulChannelId, diag_BspConnMgr);
    if(ulRet)
    {
        diag_printf("bsp reg connect msg fail, ret:0x%x\n", ulRet);
    }
}

VOS_VOID diag_nvInit(VOS_VOID)
{
    mdrv_hds_cmd_register(DIAG_CMD_NV_WR, (bsp_hds_func)diag_NvWrProc);
    mdrv_hds_cmd_register(DIAG_CMD_NV_RD, (bsp_hds_func)diag_NvRdProc);
    mdrv_hds_cmd_register(DIAG_CMD_GET_NV_LIST, (bsp_hds_func)diag_GetNvListProc);
    mdrv_hds_cmd_register(DIAG_CMD_NV_AUTH, (bsp_hds_func)diag_NvAuthProc);
}




