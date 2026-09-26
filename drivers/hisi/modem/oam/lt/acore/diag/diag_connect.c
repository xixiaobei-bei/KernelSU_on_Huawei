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
  1 Include HeadFile
*****************************************************************************/
#include <mdrv.h>
#include <mdrv_diag_system.h>
#include "product_config.h"
#include "diag_common.h"
#include "diag_cfg.h"
#include "diag_msgmsp.h"
#include "diag_msgphy.h"
#include "diag_api.h"
#include "diag_debug.h"
#include "msp_errno.h"
#include "soc_socp_adapter.h"
#include "diag_time_stamp.h"
#include "diag_msgps.h"
#include "diag_connect.h"


#define    THIS_FILE_ID        MSP_FILE_ID_DIAG_CONNECT_C

extern HTIMER          g_DebugTimer;

VOS_VOID diag_GetModemInfo(DIAG_CONNECT_FRAME_INFO_STRU *pstDiagHead)
{
    VOS_UINT32 ulCnfRst;
    DIAG_CMD_HOST_CONNECT_CNF_STRU stCnf    = {0};
    const MODEM_VER_INFO_S* pstVerInfo;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo       = {0};
    DIAG_MSG_REPORT_HEAD_STRU stDiagHead    = {0};

    /*处理结果*/
    stCnf.ulAuid = ((MSP_DIAG_DATA_REQ_STRU*)(pstDiagHead->aucData))->ulAuid;
    stCnf.ulSn   = ((MSP_DIAG_DATA_REQ_STRU*)(pstDiagHead->aucData))->ulSn;

    (VOS_VOID)VOS_MemSet_s(&(stCnf.stBuildVersion), (VOS_UINT32)sizeof(DIAG_CMD_UE_BUILD_VER_STRU), 0, (VOS_UINT32)sizeof(DIAG_CMD_UE_BUILD_VER_STRU));

    /*获取版本信息*/
    pstVerInfo = mdrv_ver_get_info();
	if(pstVerInfo!=NULL)
	{
        stCnf.stBuildVersion.usVVerNo        = pstVerInfo->stswverinfo.ulVVerNO;
        stCnf.stBuildVersion.usRVerNo        = pstVerInfo->stswverinfo.ulRVerNO;
        stCnf.stBuildVersion.usCVerNo        = pstVerInfo->stswverinfo.ulCVerNO;
        stCnf.stBuildVersion.usBVerNo        = pstVerInfo->stswverinfo.ulBVerNO;
        stCnf.stBuildVersion.usSpcNo         = pstVerInfo->stswverinfo.ulSpcNO;
        stCnf.stBuildVersion.usHardwareVerNo = pstVerInfo->stswverinfo.ulCustomNOv;
        stCnf.stBuildVersion.ulProductNo     = pstVerInfo->stswverinfo.ulProductNo;

        /*获取数采基地址*/
        stCnf.ulChipBaseAddr = (VOS_UINT32)pstVerInfo->stproductinfo.echiptype;
	}

    /*获取IMEI号*/
    //(VOS_VOID)diag_GetImei(stCnf.szImei);

    /*获取软件版本号*/
    (VOS_VOID)VOS_MemSet_s(&stCnf.stUeSoftVersion, (VOS_UINT32)sizeof(DIAG_CMD_UE_SOFT_VERSION_STRU), 0, (VOS_UINT32)sizeof(DIAG_CMD_UE_SOFT_VERSION_STRU));

    /*路测信息获取*/
    (VOS_VOID) mdrv_nv_read(EN_NV_ID_AGENT_FLAG, &(stCnf.stAgentFlag), (VOS_UINT32)sizeof(NV_ITEM_AGENT_FLAG_STRU));

    stCnf.diag_cfg.UintValue = 0;

    /* 010: OM通道融合的版本 */
    /* 110: OM融合GU未融合的版本 */
    /* 100: OM完全融合的版本 */
    stCnf.diag_cfg.CtrlFlag.ulDrxControlFlag    = 0; /*和HIDS确认此处不再使用,打桩处理即可*/
    stCnf.diag_cfg.CtrlFlag.ulPortFlag          = 0;
    stCnf.diag_cfg.CtrlFlag.ulOmUnifyFlag       = 1;

	stCnf.ulLpdMode                             = 0x5a5a5a5a;
    stCnf.ulRc                                  = ERR_MSP_SUCCESS;


    (VOS_VOID)VOS_MemSet_s(stCnf.szProduct, (VOS_UINT32)sizeof(stCnf.szProduct),0,(VOS_UINT32)sizeof(stCnf.szProduct));
    (VOS_VOID)VOS_MemCpy_s(stCnf.szProduct, (VOS_UINT32)sizeof(stCnf.szProduct), 
        PRODUCT_FULL_VERSION_STR, VOS_StrNLen(PRODUCT_FULL_VERSION_STR, sizeof(stCnf.szProduct)-1));

    DIAG_MSG_COMMON_PROC(stDiagInfo, stCnf, pstDiagHead);

    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_MSP;

    stDiagHead.u.stID.pri4b     = (stDiagInfo.ulMsgType & 0xf);
    stDiagHead.u.stID.mode4b    = (stDiagInfo.ulMode & 0xf);
    stDiagHead.u.stID.sec5b     = (stDiagInfo.ulSubType & 0x1f);
    stDiagHead.u.stID.cmdid19b  = (stDiagInfo.ulMsgId & 0x7ffff);
    stDiagHead.ulSsid           = stDiagInfo.ulSSId;
    stDiagHead.ulModemId        = stDiagInfo.ulModemid;
    stDiagHead.ulDirection      = stDiagInfo.ulDirection;
    stDiagHead.ulMsgTransId     = stDiagInfo.ulTransId;
    stDiagHead.ulChanId         = SCM_CODER_SRC_LOM_CNF;
    stDiagHead.ulDataSize       = sizeof(stCnf);
    stDiagHead.pData            = &stCnf;

    ulCnfRst = diag_ServicePackData(&stDiagHead);

    if(ERR_MSP_SUCCESS != ulCnfRst)
    {
        diag_printf("diag_GetModemInfo failed.\n");
    }
    else
    {
        diag_printf("diag_GetModemInfo success.\n");
    }

    return;
}

VOS_UINT32 diag_ConnProc(VOS_UINT8* pstReq)
{
    VOS_UINT32 ret;
    VOS_UINT32 ulCnfRst = ERR_MSP_UNAVAILABLE;
    DIAG_MSG_MSP_CONN_STRU *pstConn;
    DIAG_CMD_HOST_CONNECT_CNF_STRU stCnf = {0};
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};
    const MODEM_VER_INFO_S* pstVerInfo = VOS_NULL;
    DIAG_CMD_REPLAY_SET_REQ_STRU stReplay={0};
    DIAG_FRAME_INFO_STRU *pstDiagHead;
    DIAG_CMD_GET_MDM_INFO_REQ_STRU *pstInfo = VOS_NULL;

    pstDiagHead = (DIAG_FRAME_INFO_STRU *)pstReq;

    mdrv_diag_PTR(EN_DIAG_PTR_MSGMSP_CONN_IN, 1, pstDiagHead->ulCmdId, 0);


    /* 新增获取modem信息的命令用于工具查询单板信息 */
    if(sizeof(DIAG_CMD_GET_MDM_INFO_REQ_STRU) == pstDiagHead->ulMsgLen)
    {
        pstInfo = (DIAG_CMD_GET_MDM_INFO_REQ_STRU *)pstDiagHead->aucData;
        if((VOS_NULL != pstInfo) && (DIAG_GET_MODEM_INFO == pstInfo->ulInfo))
        {
            diag_GetModemInfo((DIAG_CONNECT_FRAME_INFO_STRU *)pstDiagHead);
            return ERR_MSP_SUCCESS;
        }
    }

    if(pstDiagHead->ulMsgLen < sizeof(MSP_DIAG_DATA_REQ_STRU))
    {
        diag_error("rev tool msg len error:0x%x\n", pstDiagHead->ulMsgLen);
        return ERR_MSP_INALID_LEN_ERROR;
    }

    diag_printf("Receive tool connect cmd!\n");

    pstConn = (DIAG_MSG_MSP_CONN_STRU *)VOS_AllocMsg(MSP_PID_DIAG_APP_AGENT, (VOS_UINT32)(sizeof(DIAG_MSG_MSP_CONN_STRU)-VOS_MSG_HEAD_LENGTH));
    if(VOS_NULL == pstConn)
    {
        ulCnfRst = ERR_MSP_DIAG_ALLOC_MALLOC_FAIL;
        goto DIAG_ERROR;
    }

    /*设置连接状态开关值*/
    ulCnfRst = diag_CfgSetGlobalBitValue(&g_ulDiagCfgInfo,DIAG_CFG_CONN_BIT,DIAG_CFG_SWT_OPEN);
    if(ulCnfRst)
    {
        diag_printf("Open DIAG_CFG_CONN_BIT failed.\n");
        goto DIAG_ERROR;
    }

    /* 关闭SOCP模块的自动降频 */
    mdrv_socp_disalbe_dfs();

    (VOS_VOID)VOS_MemSet_s(&(pstConn->stConnInfo.stBuildVersion), (VOS_UINT32)sizeof(DIAG_CMD_UE_BUILD_VER_STRU), 0, (VOS_UINT32)sizeof(DIAG_CMD_UE_BUILD_VER_STRU));

    /*获取版本信息*/
    pstVerInfo = mdrv_ver_get_info();
    if(pstVerInfo!=NULL)
    {
        pstConn->stConnInfo.stBuildVersion.usVVerNo        = pstVerInfo->stswverinfo.ulVVerNO;
        pstConn->stConnInfo.stBuildVersion.usRVerNo        = pstVerInfo->stswverinfo.ulRVerNO;
        pstConn->stConnInfo.stBuildVersion.usCVerNo        = pstVerInfo->stswverinfo.ulCVerNO;
        pstConn->stConnInfo.stBuildVersion.usBVerNo        = pstVerInfo->stswverinfo.ulBVerNO;
        pstConn->stConnInfo.stBuildVersion.usSpcNo         = pstVerInfo->stswverinfo.ulSpcNO;
        pstConn->stConnInfo.stBuildVersion.usHardwareVerNo = pstVerInfo->stswverinfo.ulCustomNOv;
        pstConn->stConnInfo.stBuildVersion.ulProductNo     = pstVerInfo->stswverinfo.ulProductNo;

        /*获取数采基地址*/
        pstConn->stConnInfo.ulChipBaseAddr = (VOS_UINT32)pstVerInfo->stproductinfo.echiptype;
    }

    /*获取IMEI号*/
    //(VOS_VOID)diag_GetImei(pstConn->stConnInfo.szImei);

    /*获取软件版本号*/
    (VOS_VOID)VOS_MemSet_s(&pstConn->stConnInfo.stUeSoftVersion, (VOS_UINT32)sizeof(DIAG_CMD_UE_SOFT_VERSION_STRU), 0, (VOS_UINT32)sizeof(DIAG_CMD_UE_SOFT_VERSION_STRU));

    /*路测信息获取*/
    (VOS_VOID)mdrv_nv_read(EN_NV_ID_AGENT_FLAG,&(pstConn->stConnInfo.stAgentFlag), (VOS_UINT32)sizeof(NV_ITEM_AGENT_FLAG_STRU));

    pstConn->stConnInfo.diag_cfg.UintValue = 0;

    /* 010: OM通道融合的版本 */
    /* 110: OM融合GU未融合的版本 */
    /* 100: OM完全融合的版本 */
    pstConn->stConnInfo.diag_cfg.CtrlFlag.ulDrxControlFlag = 0; /*和HIDS确认此处不再使用,打桩处理即可*/
    pstConn->stConnInfo.diag_cfg.CtrlFlag.ulPortFlag = 0;
    pstConn->stConnInfo.diag_cfg.CtrlFlag.ulOmUnifyFlag = 1;

    pstConn->stConnInfo.ulLpdMode = 0x5a5a5a5a;

    (VOS_VOID)VOS_MemSet_s(pstConn->stConnInfo.szProduct, (VOS_UINT32)sizeof(pstConn->stConnInfo.szProduct),
        0, (VOS_UINT32)sizeof(pstConn->stConnInfo.szProduct));
    
    (VOS_VOID)VOS_MemCpy_s(pstConn->stConnInfo.szProduct, (VOS_UINT32)sizeof(pstConn->stConnInfo.szProduct), 
        PRODUCT_FULL_VERSION_STR, VOS_StrNLen(PRODUCT_FULL_VERSION_STR, sizeof(pstConn->stConnInfo.szProduct)-1));

    ulCnfRst = diag_SendMsg(MSP_PID_DIAG_APP_AGENT,PS_PID_MM,ID_MSG_DIAG_CMD_REPLAY_TO_PS,(VOS_UINT8*)&stReplay,\
                (VOS_UINT32)sizeof(DIAG_CMD_REPLAY_SET_REQ_STRU));
    if(ulCnfRst)
    {
        ulCnfRst = ERR_MSP_DIAG_SEND_MSG_FAIL;
        goto DIAG_ERROR;
    }

    /*处理结果*/
    pstConn->stConnInfo.ulAuid = ((MSP_DIAG_DATA_REQ_STRU*)(pstDiagHead->aucData))->ulAuid;
    pstConn->stConnInfo.ulSn   = ((MSP_DIAG_DATA_REQ_STRU*)(pstDiagHead->aucData))->ulSn;
    pstConn->stConnInfo.ulRc   = ERR_MSP_SUCCESS;

    pstConn->ulReceiverPid = MSP_PID_DIAG_AGENT;
    pstConn->ulSenderPid   = MSP_PID_DIAG_APP_AGENT;
    pstConn->ulCmdId       = pstDiagHead->ulCmdId;
    pstConn->ulMsgId       = DIAG_MSG_MSP_CONN_REQ;

    ulCnfRst = VOS_SendMsg(MSP_PID_DIAG_APP_AGENT, pstConn);
    if(ERR_MSP_SUCCESS == ulCnfRst)
    {
        /*复位维测信息记录*/
        diag_reset_src_mntn_info();
        mdrv_diag_reset_dst_mntn_info();

        /* 启动定时器上报可维可测信息给工具定位丢包问题 */
        ret = VOS_StartRelTimer(&g_DebugTimer, MSP_PID_DIAG_APP_AGENT, DIAG_DEBUG_TIMER_LEN, DIAG_DEBUG_TIMER_NAME, \
                                DIAG_DEBUG_TIMER_PARA, VOS_RELTIMER_NOLOOP, VOS_TIMER_NO_PRECISION);
        if(ret != ERR_MSP_SUCCESS)
        {
            diag_printf("VOS_StartRelTimer fail [%s]\n", __FUNCTION__);
        }

        mdrv_applog_conn();

        mdrv_hds_printlog_conn();

        mdrv_hds_translog_conn();

        if(!DIAG_IS_POLOG_ON)
        {
            mdrv_socp_send_data_manager(SOCP_CODER_DST_OM_IND, SOCP_DEST_DSM_ENABLE);
        }
        diag_printf("Diag send ConnInfo to Modem success.\n");

        return ulCnfRst;
    }

DIAG_ERROR:

    if(pstConn)
    {
      VOS_FreeMsg(MSP_PID_DIAG_APP_AGENT, pstConn);
    }
    DIAG_MSG_COMMON_PROC(stDiagInfo, stCnf, pstDiagHead);

    stDiagInfo.ulMsgType    = DIAG_MSG_TYPE_MSP;

    stCnf.ulRc   = ulCnfRst;

    ulCnfRst = DIAG_MsgReport(&stDiagInfo, &stCnf, (VOS_UINT32)sizeof(stCnf));

    diag_printf("diag connect failed.\n");

    return ulCnfRst;
}



VOS_UINT32 diag_SetChanDisconn(MsgBlock* pMsgBlock)
{
    if(!DIAG_IS_CONN_ON)
    {
        return 0;
    }
    else
    {
        diag_ConnReset();
        diag_CfgResetAllSwt();

        /* 删除定时器 */
        (VOS_VOID)VOS_StopRelTimer(&g_DebugTimer);

        mdrv_hds_printlog_disconn();

        mdrv_hds_translog_disconn();

        /*将状态发送给C核*/
        diag_SendMsg(MSP_PID_DIAG_APP_AGENT,MSP_PID_DIAG_AGENT,ID_MSG_DIAG_HSO_DISCONN_IND, VOS_NULL, 0);

        mdrv_socp_send_data_manager(SOCP_CODER_DST_OM_IND, SOCP_DEST_DSM_DISABLE);
    }

    return 0;
}



VOS_UINT32 diag_DisConnProc(VOS_UINT8* pstReq)
{
    VOS_UINT32 ret;
    DIAG_CMD_HOST_DISCONNECT_CNF_STRU stCnfDisConn = {0};
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};
    DIAG_FRAME_INFO_STRU *pstDiagHead;
    VOS_UINT32 ulLen;
    DIAG_MSG_A_TRANS_C_STRU *pstInfo;

    diag_printf("Receive tool disconnect cmd!\n");

    pstDiagHead = (DIAG_FRAME_INFO_STRU *)pstReq;

    if(pstDiagHead->ulMsgLen < sizeof(MSP_DIAG_DATA_REQ_STRU))
    {
        diag_error("rev tool msg len error:0x%x\n", pstDiagHead->ulMsgLen);
        return ERR_MSP_INALID_LEN_ERROR;
    }

    mdrv_diag_PTR(EN_DIAG_PTR_MSGMSP_DISCONN_IN, 1, pstDiagHead->ulCmdId, 0);

    /*重置所有开关状态为未打开*/
    diag_ConnReset();
    diag_CfgResetAllSwt();

    /* 删除定时器 */
    (VOS_VOID)VOS_StopRelTimer(&g_DebugTimer);

    DIAG_MSG_ACORE_CFG_PROC(ulLen, pstDiagHead, pstInfo, ret);

    mdrv_hds_printlog_disconn();

    mdrv_hds_translog_disconn();

    mdrv_socp_send_data_manager(SOCP_CODER_DST_OM_IND, SOCP_DEST_DSM_DISABLE);

    return ret;

DIAG_ERROR:

    DIAG_MSG_COMMON_PROC(stDiagInfo, stCnfDisConn, pstDiagHead);

    stDiagInfo.ulMsgType    = DIAG_MSG_TYPE_MSP;

    stCnfDisConn.ulRc   = ret;

    ret = DIAG_MsgReport(&stDiagInfo, &stCnfDisConn, (VOS_UINT32)sizeof(stCnfDisConn));

    diag_printf("diag disconnect failed.\n");

    return ret;

}
VOS_UINT32 diag_ConnTimerProc(VOS_VOID)
{
    return ERR_MSP_SUCCESS;
}
VOS_UINT32 diag_DisConnMgrProc(VOS_UINT8* pstReq)
{
    return ERR_MSP_SUCCESS;
}
VOS_UINT32 diag_ConnMgrProc(VOS_UINT8* pstReq)
{
    return ERR_MSP_SUCCESS;
}
VOS_UINT32 diag_ConnMgrSendFuncReg (VOS_UINT32 ulConnId,
                                              VOS_UINT32 ulChannelNum,
                                              VOS_UINT32 *ulChannel,
                                              DIAG_SEND_PROC_FUNC pSendFuc)
{
    return ERR_MSP_SUCCESS;
}
VOS_UINT32 diag_ConnCnf (VOS_UINT32  ulConnId,  VOS_UINT32 ulSn, VOS_UINT32 ulCount, DIAG_CONNECT_RESULT *pstResult)
{
    return ERR_MSP_SUCCESS;
}

