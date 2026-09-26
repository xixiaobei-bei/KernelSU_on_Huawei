/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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
 * 1 Include HeadFile
 */
#include <securec.h>
#include "diag_connect.h"
#include <product_config.h>
#include <mdrv.h>
#include <msp.h>
#include <vos.h>
#include "diag_common.h"
#include "diag_cfg.h"
#include "diag_msgmsp.h"
#include "diag_msgphy.h"
#include "diag_msgps.h"
#include "diag_api.h"
#include "diag_debug.h"
#include "msp_errno.h"
#include "soc_socp_adapter.h"
#include "diag_time_stamp.h"
#include "diag_api.h"

#define THIS_FILE_ID MSP_FILE_ID_DIAG_CONNECT_C

#ifdef DIAG_SEC_TOOLS
VOS_UINT32 g_ulAuthState = DIAG_AUTH_TYPE_DEFAULT;
HTIMER g_AuthTimer = VOS_NULL;
#endif

#ifdef DIAG_SYSTEM_5G

#ifdef DIAG_SEC_TOOLS
DIAG_CMD_HOST_CONNECT_CNF_STRU g_stConnInfo = {};
#endif

/*
 * connect过程****************************************
 * 设备查询
 * step1. 打开USB串口
 * step2. GetModemInfo(0x10065000)
 * step3. disconnect(0x10065001)
 * step4. 关闭USB串口(USB会回调SCM告知USB端口断开)
 * 设备连接:
 * step1. 打开USB串口
 * step2. connect(0x10065000)
 * step3. GetPidTable()(0x10014003)
 * step4. GetModemInfo(0x10065000)
 * step5. disconnect(0x10065001)
 * step6. 关闭USb串口(USB会回调SCM告知USB端口断开)
 */
struct CONNECT_CTRL_STRU g_stConnectCtrl = {};

DIAG_CONNECT_PROC_STRU g_astConnectTbl[] = {
#ifdef DIAG_SYSTEM_5G
    { DIAG_CONN_ID_ACPU_DEFALUT, "DEFAULT", VOS_NULL, 0, 0, 0 },
    { DIAG_CONN_ID_ACPU_PS, "PS", VOS_NULL, 0, 0, 0 },
    { DIAG_CONN_ID_ACPU_BSP, "BSP", VOS_NULL, 0, 0, 0 },
    { DIAG_CONN_ID_ACPU_APP, "APP LOG", VOS_NULL, 0, 0, 0 },
    { DIAG_CONN_ID_ACPU_TEE, "TEE", VOS_NULL, 0, 0, 0 },
    { DIAG_CONN_ID_ACPU_LPM3, "LPM3", VOS_NULL, 0, 0, 0 },
    { DIAG_CONN_ID_ACPU_LRM, "LRM", VOS_NULL, 0, 0, 0 },
    { DIAG_CONN_ID_ACPU_NRM, "NRM", VOS_NULL, 0, 0, 0 }
#else
    { DIAG_CONN_ID_ACPU_PS, "PS", VOS_NULL, 0, 0, 0 },
    { DIAG_CONN_ID_ACPU_BSP, "BSP", VOS_NULL, 0, 0, 0 },
    { DIAG_CONN_ID_ACPU_APP, "APP LOG", VOS_NULL, 0, 0, 0 },
    { DIAG_CONN_ID_ACPU_LRM, "LRM", VOS_NULL, 0, 0, 0 },
#endif
};

DIAG_CONN_CHANN_TBL g_astChannelTbl[DIAG_SRC_CHANNLE_NUM] = {
    { 0, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },  { 1, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 2, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },  { 3, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 4, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },  { 5, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 6, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },  { 7, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 8, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },  { 9, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 10, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 11, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 12, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 13, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 14, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 15, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 16, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 17, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 18, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 19, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 20, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 21, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 22, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 23, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 24, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 25, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 26, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 27, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 28, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 29, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 30, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 31, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
#ifdef DIAG_SYSTEM_5G
    { 32, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 33, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 34, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 35, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 36, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 37, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 38, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 39, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 40, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 41, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 42, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 43, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 44, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 45, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 46, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 47, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 48, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 49, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 50, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 51, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 52, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 53, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 54, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 55, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 56, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 57, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 58, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 59, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 60, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 61, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
    { 62, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT }, { 63, DIAG_CONN_ID_ACPU_DEFALUT, DIAG_CHANN_DEFAULT },
#endif
};

/*
 * Function Name: diag_ConnectFailReport
 * Description: 连接失败回复处理函数
 * Input: VOS_UINT32 ulRet 回复给工具错误码
 * Output: None
 * Return: VOS_UINT32
 */
VOS_UINT32 diag_ConnectFailReport(VOS_UINT32 ulRet)
{
    DIAG_CMD_HOST_CONNECT_FAIL_CNF_STRU *pstFailConnInfo = VOS_NULL;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = { 0 };
    VOS_UINT32 ulIndex;
    VOS_UINT32 ulCnfRst;
    VOS_UINT32 ulCnfSize;

    ulCnfSize = sizeof(DIAG_CMD_HOST_CONNECT_FAIL_CNF_STRU) + sizeof(g_astChannelTbl);

    pstFailConnInfo = VOS_MemAlloc(MSP_PID_DIAG_APP_AGENT, DYNAMIC_MEM_PT, ulCnfSize);
    if (pstFailConnInfo == NULL) {
        diag_error("VOS_MemAlloc fail, ulCnfSize=0x%x\n", ulCnfSize);
        return ERR_MSP_DIAG_ALLOC_MALLOC_FAIL;
    }

    pstFailConnInfo->ulChannelNum = sizeof(g_astChannelTbl) / sizeof(g_astChannelTbl[0]);
    for (ulIndex = 0; ulIndex < pstFailConnInfo->ulChannelNum; ulIndex++) {
        pstFailConnInfo->stResult[ulIndex].ulChannelId = g_astChannelTbl[ulIndex].ulChannelId;
        pstFailConnInfo->stResult[ulIndex].ulResult = g_astChannelTbl[ulIndex].ulResult;
    }

    pstFailConnInfo->ulAuid = g_stConnectCtrl.stConnectCmd.ulAuid;
    pstFailConnInfo->ulSn = g_stConnectCtrl.stConnectCmd.ulSn;
    pstFailConnInfo->ulRc = ulRet;

    stDiagInfo.ulSSId = DIAG_SSID_CPU;
    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_MSP;
    stDiagInfo.ulMode = g_stConnectCtrl.connectCommandId.stID.mode4b;
    stDiagInfo.ulSubType = g_stConnectCtrl.connectCommandId.stID.sec5b;
    stDiagInfo.ulDirection = DIAG_MT_CNF;
    stDiagInfo.ulModemid = 0;
    stDiagInfo.ulMsgId = g_stConnectCtrl.connectCommandId.ulID;
    stDiagInfo.ulTransId = g_stConnectCtrl.stConnectCmd.ulMsgTransId;

    /* 清空本地记录 */
    diag_ResetConnectInfo();

    ulCnfRst = DIAG_MsgReport(&stDiagInfo, (VOS_VOID *)pstFailConnInfo, ulCnfSize);
    if (ulCnfRst) {
        VOS_MemFree(MSP_PID_DIAG_APP_AGENT, pstFailConnInfo);
        diag_error("report connect msg fail, ulCnfRst:0x%x\n", ulCnfRst);
        return ulCnfRst;
    } else {
        VOS_MemFree(MSP_PID_DIAG_APP_AGENT, pstFailConnInfo);
        return ERR_MSP_SUCCESS;
    }

    return ERR_MSP_SUCCESS;
}
/*
 * Function Name: diag_ResetConnectInfo
 * Description: 复位本地记录
 */
VOS_VOID diag_ResetConnectInfo(VOS_VOID)
{
    VOS_UINT32 ulIndex;

    for (ulIndex = 0; ulIndex < sizeof(g_astChannelTbl) / sizeof(g_astChannelTbl[0]); ulIndex++) {
        if (g_astChannelTbl[ulIndex].ulResult != DIAG_CHANN_DEFAULT) {
            g_astChannelTbl[ulIndex].ulResult = DIAG_CHANN_REGISTER;
        }
    }

    for (ulIndex = 0; ulIndex < sizeof(g_astConnectTbl) / sizeof(g_astConnectTbl[0]); ulIndex++) {
        g_astConnectTbl[ulIndex].ulHasCnf = 0;
    }

    g_stConnectCtrl.stConnectCmd.ulAuid = 0;
    g_stConnectCtrl.stConnectCmd.ulSn = 0;
    g_stConnectCtrl.stConnectCmd.ulMsgTransId = 0;
    g_stConnectCtrl.stConnectCmd.ulMsgId = 0;
}
/*
 * Function Name: diag_ConnectSucessReport
 * Description: 建连消息处理成功回复处理
 * Input: None
 * Output: None
 * Return: VOS_UINT32
 */
VOS_UINT32 diag_ConnectSucessReport(VOS_VOID)
{
    const modem_ver_info_s *pstVerInfo = VOS_NULL;
    DIAG_CMD_HOST_CONNECT_CNF_STRU stConnInfo = { 0 };
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = { 0 };
    VOS_UINT32 ulCnfRst;
    errno_t err;

    /* 获取版本信息 */
    pstVerInfo = mdrv_ver_get_info();
    if (pstVerInfo != NULL) {
        /* 获取数采基地址 */
        stConnInfo.ulChipBaseAddr = (VOS_UINT32)pstVerInfo->product_info.chip_type;
    }

    /* 获取IMEI号 */

    /* 获取软件版本号 */
    err = memset_s(&stConnInfo.stUeSoftVersion, (VOS_UINT32)sizeof(stConnInfo.stUeSoftVersion), 0,
        (VOS_UINT32)sizeof(stConnInfo.stUeSoftVersion));
    if (err != EOK) {
        diag_error("memset fail:%x\n", err);
    }

    /* 设置鉴权padding 模式 */
    *(VOS_UINT32 *)&(stConnInfo.stAgentFlag) = 1; /* rsa signature:SEC_RSA_PSS_PADDING */

    stConnInfo.diag_cfg.UintValue = 0;

    /* 010: OM通道融合的版本 */
    /* 110: OM融合GU未融合的版本 */
    /* 100: OM完全融合的版本 */
    stConnInfo.diag_cfg.CtrlFlag.ulDrxControlFlag = 0; /* 和HIDS确认此处不再使用,打桩处理即可 */
    stConnInfo.diag_cfg.CtrlFlag.ulPortFlag = 0;
    stConnInfo.diag_cfg.CtrlFlag.ulOmUnifyFlag = 0;
    stConnInfo.diag_cfg.CtrlFlag.ul5GFrameFlag = 1;
#ifdef DIAG_SEC_TOOLS
    stConnInfo.diag_cfg.CtrlFlag.ulAuthFlag = 1;
#else
    stConnInfo.diag_cfg.CtrlFlag.ulAuthFlag = 0;
#endif

    stConnInfo.ulLpdMode = 0x5a5a5a5a;

    err = memcpy_s(stConnInfo.szProduct, (VOS_UINT32)sizeof(stConnInfo.szProduct), PRODUCT_FULL_VERSION_STR,
        VOS_StrNLen(PRODUCT_FULL_VERSION_STR, sizeof(stConnInfo.szProduct) - 1));
    if (err != EOK) {
        diag_error("memcpy fail:%x\n", err);
    }
    (VOS_VOID)
        mdrv_timer_get_accuracy_timestamp((VOS_UINT32 *)&stConnInfo.szImei[4], (VOS_UINT32 *)&stConnInfo.szImei[0]);

    (VOS_VOID)mdrv_timer_get_accuracy_timestamp((VOS_UINT32 *)&stConnInfo.ucTimeStamp[4],
        (VOS_UINT32 *)&stConnInfo.ucTimeStamp[0]);

    stConnInfo.ulCpuFrequcy = mdrv_get_normal_timestamp_freq();
    stConnInfo.ulUsbType = (VOS_UINT32)mdrv_diag_get_usb_type();

    /* 处理结果 */
    stConnInfo.ulAuid = g_stConnectCtrl.stConnectCmd.ulAuid;
    stConnInfo.ulSn = g_stConnectCtrl.stConnectCmd.ulSn;
    stConnInfo.ulRc = ERR_MSP_SUCCESS;

    stDiagInfo.ulSSId = DIAG_SSID_CPU;
    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_MSP;
    stDiagInfo.ulMode = g_stConnectCtrl.connectCommandId.stID.mode4b;
    stDiagInfo.ulSubType = g_stConnectCtrl.connectCommandId.stID.sec5b;
    stDiagInfo.ulDirection = DIAG_MT_CNF;
    stDiagInfo.ulModemid = 0;
    stDiagInfo.ulMsgId = g_stConnectCtrl.stConnectCmd.ulMsgId;
    stDiagInfo.ulTransId = g_stConnectCtrl.stConnectCmd.ulMsgTransId;

    /* 清空本地记录 */
    diag_ResetConnectInfo();

    diag_crit("HIDS/HIMS connect success.\n");
#ifdef DIAG_SEC_TOOLS
    err = memcpy_s(&g_stConnInfo, sizeof(g_stConnInfo), &stConnInfo, sizeof(stConnInfo));
    if (err != EOK) {
        diag_error("memcpy fail:%x\n", err);
    }
#endif
    ulCnfRst = mdrv_diag_report_cnf((diag_cnf_info_s *)&stDiagInfo, &stConnInfo, (VOS_UINT32)sizeof(stConnInfo));
    if (ulCnfRst) {
        diag_error("report connect msg fail, ulCnfRst:0x%x\n", ulCnfRst);
        return ulCnfRst;
    } else {
#ifndef DIAG_SEC_TOOLS
        mdrv_ppm_notify_conn_state(CONN_STATE_CONNECT);
#endif
        return ERR_MSP_SUCCESS;
    }
}

/*
 * Function Name: diag_GetModemInfo
 * Description: 获取modem信息(用于工具未连接时，工具后台查询单板信息)
 * Input: VOS_VOID
 * Output: None
 * Return: VOS_UINT32
 */
VOS_VOID diag_GetModemInfo(DIAG_CONNECT_FRAME_INFO_STRU *pstDiagHead)
{
    VOS_UINT32 ulCnfRst;
    DIAG_CMD_HOST_CONNECT_CNF_STRU stCnf = { 0 };
    const modem_ver_info_s *pstVerInfo = NULL;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {};
    errno_t err;

    /* 获取版本信息 */
    pstVerInfo = mdrv_ver_get_info();
    if (pstVerInfo != NULL) {
        /* 获取数采基地址 */
        stCnf.ulChipBaseAddr = (VOS_UINT32)pstVerInfo->product_info.chip_type;
    }


    /* 获取软件版本号 */
    err = memset_s(&stCnf.stUeSoftVersion, (VOS_UINT32)sizeof(DIAG_CMD_UE_SOFT_VERSION_STRU), 0,
        (VOS_UINT32)sizeof(DIAG_CMD_UE_SOFT_VERSION_STRU));
    if (err != EOK) {
        diag_error("memset fail:%x\n", err);
    }

    /* 路测信息获取 */
    (VOS_VOID)mdrv_nv_read(EN_NV_ID_AGENT_FLAG, &(stCnf.stAgentFlag), (VOS_UINT32)sizeof(NV_ITEM_AGENT_FLAG_STRU));

    stCnf.diag_cfg.UintValue = 0;

    /* 010: OM通道融合的版本 */
    /* 110: OM融合GU未融合的版本 */
    /* 100: OM完全融合的版本 */
    stCnf.diag_cfg.CtrlFlag.ulDrxControlFlag = 0; /* 和HIDS确认此处不再使用,打桩处理即可 */
    stCnf.diag_cfg.CtrlFlag.ulPortFlag = 0;
    stCnf.diag_cfg.CtrlFlag.ulOmUnifyFlag = 0;
    stCnf.diag_cfg.CtrlFlag.ul5GFrameFlag = 1;

    /* HIDS兼容一段时间后删掉 2019-1-2 */
    (VOS_VOID)mdrv_timer_get_accuracy_timestamp((VOS_UINT32 *)&stCnf.szImei[4], (VOS_UINT32 *)&stCnf.szImei[0]);

    (VOS_VOID)
        mdrv_timer_get_accuracy_timestamp((VOS_UINT32 *)&stCnf.ucTimeStamp[4], (VOS_UINT32 *)&stCnf.ucTimeStamp[0]);
    stCnf.ulCpuFrequcy = mdrv_get_normal_timestamp_freq();
    stCnf.ulUsbType = (VOS_UINT32)mdrv_diag_get_usb_type();

    stCnf.ulLpdMode = 0x5a5a5a5a;
    stCnf.ulRc = ERR_MSP_SUCCESS;

    err = memset_s(stCnf.szProduct, (VOS_UINT32)sizeof(stCnf.szProduct), 0, (VOS_UINT32)sizeof(stCnf.szProduct));
    if (err != EOK) {
        diag_error("memset fail:%x\n", err);
    }
    err = memcpy_s(stCnf.szProduct, (VOS_UINT32)sizeof(stCnf.szProduct), PRODUCT_FULL_VERSION_STR,
        VOS_StrNLen(PRODUCT_FULL_VERSION_STR, sizeof(stCnf.szProduct) - 1));
    if (err != EOK) {
        diag_error("memcpy fail:%x\n", err);
    }

    stDiagInfo.ulSSId = DIAG_SSID_CPU;
    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_MSP;
    stDiagInfo.ulMode = pstDiagHead->stID.mode4b;
    stDiagInfo.ulSubType = pstDiagHead->stID.sec5b;
    stDiagInfo.ulDirection = DIAG_MT_CNF;
    stDiagInfo.ulModemid = 0;
    stDiagInfo.ulMsgId = pstDiagHead->ulCmdId;

    stCnf.ulAuid = ((msp_diag_data_head_s *)(pstDiagHead->aucData))->ulAuid;
    stCnf.ulSn = ((msp_diag_data_head_s *)(pstDiagHead->aucData))->ulSn;

    ulCnfRst = mdrv_diag_report_cnf((diag_cnf_info_s *)&stDiagInfo, (VOS_VOID *)&stCnf, sizeof(stCnf));
    if (ulCnfRst != ERR_MSP_SUCCESS) {
        diag_error("diag_GetModemInfo failed., ret:0x%x\n", ulCnfRst);
    } else {
        diag_crit("diag_GetModemInfo success.\n");
    }

    return;
}
/*
 * 函 数 名: diag_SendConnectMsg
 * 功能描述: 向其他子系统发送链接/断链消息
 * 输入参数: pstSendMsg 要发送的消息
 * 输出参数: None
 * 返 回 值: 发送结果
 */
VOS_UINT32 diag_SendConnectMsg(VOS_UINT8 *pstSendMsg)
{
    VOS_UINT32 ulIndex;
    VOS_UINT32 ret;

    /* 轮询所有子系统 */
    for (ulIndex = 0; ulIndex < sizeof(g_astConnectTbl) / sizeof(g_astConnectTbl[0]); ulIndex++) {
        if (g_astConnectTbl[ulIndex].pSendFunc) {
            ret = g_astConnectTbl[ulIndex].pSendFunc(pstSendMsg);
            if (ret) {
                diag_error("send msg to %s fail, ret:0x%x\n", g_astConnectTbl[ulIndex].cName, ret);
            } else {
                diag_info("send msg to %s success\n", g_astConnectTbl[ulIndex].cName);
            }
        }
    }
    return ERR_MSP_SUCCESS;
}

/*
 * 函 数 名: diag_ConnProc
 * 功能描述: 连接消息处理函数
 * 输入参数: None
 * 输出参数: None
 * 返 回 值: 处理结果
 */
VOS_UINT32 diag_ConnProc(VOS_UINT8 *pstReq)
{
    VOS_UINT32 ulCnfRst;
    DIAG_CONNECT_FRAME_INFO_STRU *pstDiagHead;
    DIAG_CMD_GET_MDM_INFO_REQ_STRU *pstInfo = VOS_NULL;

    pstDiagHead = (DIAG_CONNECT_FRAME_INFO_STRU *)pstReq;

    mdrv_diag_ptr(EN_DIAG_PTR_MSGMSP_CONN_IN, 1, pstDiagHead->ulCmdId, 0);

    diag_crit("receive tools connect start\n");

    if (diag_CheckModemStatus() != ERR_MSP_SUCCESS) {
        return ERR_MSP_UNAVAILABLE;
    }

    /* 获取modem信息的命令用于工具查询单板信息 */
    if (pstDiagHead->ulMsgLen >= sizeof(DIAG_CMD_GET_MDM_INFO_REQ_STRU)) {
        pstInfo = (DIAG_CMD_GET_MDM_INFO_REQ_STRU *)pstDiagHead->aucData;
        if ((pstInfo->ulInfo & DIAG_GET_MODEM_INFO_BIT) == DIAG_GET_MODEM_INFO_BIT) {
            diag_GetModemInfo(pstDiagHead);
            return ERR_MSP_SUCCESS;
        }
        if ((pstInfo->ulInfo & DIAG_VERIFY_SIGN_BIT) == DIAG_VERIFY_SIGN_BIT) {
#ifdef DIAG_SEC_TOOLS
            diag_ConnAuth(pstDiagHead);
#else
            diag_crit("no auth\n");
#endif
            return ERR_MSP_SUCCESS;
        }
        diag_error("not support rev tools valule:0x%x\n", pstInfo->ulInfo);
        return ERR_MSP_NOT_SUPPORT;
    }

#ifdef DIAG_SEC_TOOLS
    g_ulAuthState = DIAG_AUTH_TYPE_DEFAULT;
#endif
    ulCnfRst = diag_SendMsg(MSP_PID_DIAG_APP_AGENT, MSP_PID_DIAG_APP_AGENT, ID_MSG_DIAG_CMD_CONNECT_REQ, pstReq,
        sizeof(DIAG_CONNECT_FRAME_INFO_STRU) + sizeof(msp_diag_data_head_s));
    if (ulCnfRst) {
        return diag_ConnectFailReport(ERR_MSP_DIAG_SEND_MSG_FAIL);
    }
    return ERR_MSP_SUCCESS;
}
#ifdef DIAG_SEC_TOOLS
VOS_VOID diag_ConnAuth(DIAG_CONNECT_FRAME_INFO_STRU *pstDiagHead)
{
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {};
    DIAG_CMD_AUTH_CNF_STRU stCmdAuthCnf = {};
    DIAG_AUTH_STRU *pstAuthStru = VOS_NULL;
    VOS_UINT32 ulRet;
    errno_t err;

    stDiagInfo.ulSSId = DIAG_SSID_CPU;
    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_MSP;
    stDiagInfo.ulMode = DIAG_MODE_LTE;
    stDiagInfo.ulSubType = DIAG_MSG_CMD;
    stDiagInfo.ulDirection = DIAG_MT_CNF;
    stDiagInfo.ulModemid = 0;
    stDiagInfo.ulMsgId = DIAG_CMD_HOST_CONNECT;
    stDiagInfo.ulTransId = 0;

    stCmdAuthCnf.ulAuid = 0;
    stCmdAuthCnf.ulSn = 0;

    g_ulAuthState = DIAG_AUTH_TYPE_AUTHING;

    /* 鉴权消息DIAG 三级头+ 鉴权key */
    if ((pstDiagHead->ulMsgLen < sizeof(msp_diag_data_head_s)) || (pstDiagHead->ulMsgLen > DIAG_MEM_ALLOC_LEN_MAX)) {
        diag_error("ERROR: pstDiagHead->ulMsgLen=0x%x\n", pstDiagHead->ulMsgLen);
        g_ulAuthState = DIAG_AUTH_TYPE_DEFAULT;
        stCmdAuthCnf.ulRc = ERR_MSP_MALLOC_FAILUE;
        (VOS_VOID)DIAG_MsgReport(&stDiagInfo, (VOS_VOID *)&stCmdAuthCnf, sizeof(stCmdAuthCnf));
        return;
    }

    pstAuthStru = (DIAG_AUTH_STRU *)VOS_MemAlloc(MSP_PID_DIAG_APP_AGENT, DYNAMIC_MEM_PT,
        pstDiagHead->ulMsgLen + sizeof(g_stConnInfo));
    if (pstAuthStru == VOS_NULL) {
        diag_error("malloc fail\n");
        g_ulAuthState = DIAG_AUTH_TYPE_DEFAULT;
        stCmdAuthCnf.ulRc = ERR_MSP_INALID_LEN_ERROR;
        (VOS_VOID)DIAG_MsgReport(&stDiagInfo, (VOS_VOID *)&stCmdAuthCnf, sizeof(stCmdAuthCnf));
        return;
    }

    err = memcpy_s(&(pstAuthStru->stConnCnf), sizeof(pstAuthStru->stConnCnf), &g_stConnInfo, sizeof(g_stConnInfo));
    if (err != EOK) {
        diag_error("memcpy fail:%x\n", err);
    }
    err = memcpy_s(&(pstAuthStru->stAuthReq), pstDiagHead->ulMsgLen, pstDiagHead->aucData, pstDiagHead->ulMsgLen);
    if (err != EOK) {
        diag_error("memcpy fail:%x\n", err);
    }

    ulRet = diag_SendMsg(MSP_PID_DIAG_APP_AGENT, MSP_PID_DIAG_AGENT, DIAG_MSG_MSP_AUTH_REQ, (VOS_UINT8 *)pstAuthStru,
        pstDiagHead->ulMsgLen + sizeof(g_stConnInfo));
    if (ulRet) {
        g_ulAuthState = DIAG_AUTH_TYPE_DEFAULT;
        stCmdAuthCnf.ulRc = ulRet;
        (VOS_VOID)DIAG_MsgReport(&stDiagInfo, (VOS_VOID *)&stCmdAuthCnf, sizeof(stCmdAuthCnf));
    } else {
        ulRet = VOS_StartRelTimer(&g_AuthTimer, MSP_PID_DIAG_APP_AGENT, DIAG_AUTH_TIMER_LEN, DIAG_AUTH_TIMER_NAME,
            DIAG_AUTH_TIMER_PARA, VOS_RELTIMER_NOLOOP, VOS_TIMER_PRECISION_0);
        if (ulRet != ERR_MSP_SUCCESS) {
            diag_error("start auth timer fail\n");
        }
    }

    VOS_MemFree(MSP_PID_DIAG_APP_AGENT, pstAuthStru);
    return;
}

VOS_VOID diag_ConnAuthRst(MsgBlock *pMsgBlock)
{
    DIAG_DATA_MSG_STRU *pstMsg = (DIAG_DATA_MSG_STRU *)pMsgBlock;
    DIAG_AUTH_CNF_STRU *pstAuthCnf = NULL;
    DIAG_CMD_AUTH_CNF_STRU stCmdAuthCnf = {};
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {};

    /* 删除定时器 */
    if (g_AuthTimer) {
        VOS_StopRelTimer(&g_AuthTimer);
        g_AuthTimer = VOS_NULL;
    }

    if (g_ulAuthState != DIAG_AUTH_TYPE_AUTHING) {
        diag_error("there is no auth req\n");
        return;
    }

    if (pstMsg->ulLength < sizeof(DIAG_DATA_MSG_STRU) - VOS_MSG_HEAD_LENGTH + sizeof(DIAG_AUTH_CNF_STRU)) {
        diag_error("rec datalen is too small,len:0x%x\n", pstMsg->ulLength);
        return;
    }

    /* 获取鉴权结果 */
    pstAuthCnf = (DIAG_AUTH_CNF_STRU *)(pstMsg->pContext);
    if (pstAuthCnf->ulRet) {
        g_ulAuthState = DIAG_AUTH_TYPE_DEFAULT;
        diag_error("tools auth fail, ret:0x%x\n", pstAuthCnf->ulRet);
    } else {
        g_ulAuthState = DIAG_AUTH_TYPE_SUCCESS;
        diag_crit("tools auth success\n");
    }

    stCmdAuthCnf.ulAuid = pstAuthCnf->stMdmInfo.ulAuid;
    stCmdAuthCnf.ulSn = pstAuthCnf->stMdmInfo.ulSn;
    stCmdAuthCnf.ulRc = pstAuthCnf->ulRet;

    stDiagInfo.ulSSId = DIAG_SSID_CPU;
    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_MSP;
    stDiagInfo.ulMode = DIAG_MODE_LTE;
    stDiagInfo.ulSubType = DIAG_MSG_CMD;
    stDiagInfo.ulDirection = DIAG_MT_CNF;
    stDiagInfo.ulModemid = 0;
    stDiagInfo.ulMsgId = DIAG_CMD_HOST_CONNECT;
    stDiagInfo.ulTransId = 0;

    (VOS_VOID)DIAG_MsgReport(&stDiagInfo, (VOS_VOID *)&stCmdAuthCnf, sizeof(stCmdAuthCnf));
    return;
}

VOS_VOID diag_ConnAuthTimerProc(VOS_VOID)
{
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {};
    DIAG_CMD_AUTH_CNF_STRU stCmdAuthCnf = {};

    g_AuthTimer = VOS_NULL;

    if (g_ulAuthState != DIAG_AUTH_TYPE_AUTHING) {
        diag_error("auth timer is timeout,but there is no auth req, authState:0x%x\n", g_ulAuthState);
        return;
    }
    g_ulAuthState = DIAG_AUTH_TYPE_DEFAULT;

    stCmdAuthCnf.ulAuid = 0;
    stCmdAuthCnf.ulSn = 0;
    stCmdAuthCnf.ulRc = ERR_MSP_TIMEOUT;

    stDiagInfo.ulSSId = DIAG_SSID_CPU;
    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_MSP;
    stDiagInfo.ulMode = DIAG_MODE_LTE;
    stDiagInfo.ulSubType = DIAG_MSG_CMD;
    stDiagInfo.ulDirection = DIAG_MT_CNF;
    stDiagInfo.ulModemid = 0;
    stDiagInfo.ulMsgId = DIAG_CMD_HOST_CONNECT;
    stDiagInfo.ulTransId = 0;

    (VOS_VOID)DIAG_MsgReport(&stDiagInfo, (VOS_VOID *)&stCmdAuthCnf, sizeof(stCmdAuthCnf));
}

VOS_VOID diag_set_auth_state(VOS_UINT32 auth_state)
{
    g_ulAuthState = auth_state;
}
VOS_UINT32 diag_get_auth_state(VOS_VOID)
{
    return g_ulAuthState;
}
#endif
/*
 * 函 数 名: diag_DisConnProc
 * 功能描述: 断连消息处理函数
 * 输入参数: pstReq:断连消息
 * 输出参数: None
 * 返 回 值: None
 */
VOS_UINT32 diag_DisConnProc(VOS_UINT8 *pstReq)
{
    VOS_UINT32 ulCnfRst;

    if (g_stConnectCtrl.ulState == DIAG_CONN_STATE_DISCONNECTED) {
        return ERR_MSP_SUCCESS;
    }

    diag_crit("revice tool disconnenct start\n");

    if (diag_GetMiniLogCfg() == DIAG_CFG_SWT_CLOSE) { /* miniDIAG模式下，通道是否上报由SOCP模式控制 */
        mdrv_socp_send_data_manager(SOCP_CODER_DST_OM_IND, SOCP_DEST_DSM_DISABLE);
    }

    ulCnfRst = diag_SendMsg(MSP_PID_DIAG_APP_AGENT, MSP_PID_DIAG_APP_AGENT, ID_MSG_DIAG_CMD_DISCONNECT_REQ, pstReq,
        sizeof(DIAG_CONNECT_FRAME_INFO_STRU) + sizeof(msp_diag_data_head_s));
    if (ulCnfRst) {
        diag_error("send msg fail, ulRet:0x%x\n", ulCnfRst);
        return diag_ConnectFailReport(ERR_MSP_DIAG_SEND_MSG_FAIL);
    }

    return ERR_MSP_SUCCESS;
}

VOS_UINT32 diag_ConnMgrProc(VOS_UINT8 *pstReq)
{
    VOS_UINT32 ulCnfRst;
    DIAG_CONNECT_FRAME_INFO_STRU *pstDiagHead = (DIAG_CONNECT_FRAME_INFO_STRU *)pstReq;
    DIAG_CONN_MSG_SEND_T stSendMsg;

    diag_crit("Receive tool connect cmd!\n");

    diag_PushHighTs();

    /* 删除定时器 */
    diag_StopMntnTimer();

    g_stConnectCtrl.ulState = DIAG_CONN_STATE_CONNECTED;
    g_stConnectCtrl.stConnectCmd.ulAuid = ((msp_diag_data_head_s *)pstDiagHead->aucData)->ulAuid;
    g_stConnectCtrl.stConnectCmd.ulSn = ((msp_diag_data_head_s *)pstDiagHead->aucData)->ulSn;
    g_stConnectCtrl.stConnectCmd.ulMsgTransId = pstDiagHead->stService.ulMsgTransId;
    g_stConnectCtrl.stConnectCmd.ulMsgId = pstDiagHead->ulCmdId;
    g_stConnectCtrl.connectCommandId.ulID = pstDiagHead->ulCmdId;

    /* 启动定时器定时等待各个组件连接回复 */
    /* 向各组件发送连接消息 */
    stSendMsg.ulMsgId = ID_MSG_DIAG_CMD_CONNECT_REQ;
    stSendMsg.ulSn = ((msp_diag_data_head_s *)pstDiagHead->aucData)->ulSn;
    ulCnfRst = diag_SendConnectMsg((VOS_UINT8 *)&stSendMsg);
    if (ulCnfRst) {
        diag_info("diag_ConnMgrProc: send connect msg fail(0x%x)\n", ulCnfRst);
    } else {
        (VOS_VOID)diag_ConnectSucessReport();
    }

    DIAG_NotifyConnState(DIAG_STATE_CONN);

#ifndef DIAG_SEC_TOOLS
    if (!DIAG_IS_POLOG_ON) {
        mdrv_socp_send_data_manager(SOCP_CODER_DST_OM_IND, SOCP_DEST_DSM_ENABLE);
    }
#endif

    return ERR_MSP_SUCCESS;
}

/*
 * Function Name: diag_DisConnMgrProc
 * Description: 该函数用于处理ConnProcEntry传进来的HSO断开命令
 * Input: pstReq 待处理数据
 * Output: None
 * Return: VOS_UINT32
 */
VOS_UINT32 diag_DisConnMgrProc(VOS_UINT8 *pstReq)
{
    VOS_UINT32 ret;
    DIAG_CONNECT_FRAME_INFO_STRU *pstDiagHead = VOS_NULL;
    DIAG_CONN_MSG_SEND_T stSendMsg;

    diag_crit("Receive tool disconnect cmd!\n");

#ifdef DIAG_SEC_TOOLS
    g_ulAuthState = DIAG_AUTH_TYPE_DEFAULT;
#endif
    pstDiagHead = (DIAG_CONNECT_FRAME_INFO_STRU *)pstReq;

    mdrv_diag_ptr(EN_DIAG_PTR_MSGMSP_DISCONN_IN, 1, pstDiagHead->ulCmdId, 0);

    diag_StopHighTsTimer();
    /* 删除定时器 */
    diag_StopMntnTimer();

    g_stConnectCtrl.ulState = DIAG_CONN_STATE_DISCONNECTED;
    g_stConnectCtrl.stConnectCmd.ulAuid = ((msp_diag_data_head_s *)pstDiagHead->aucData)->ulAuid;
    g_stConnectCtrl.stConnectCmd.ulSn = ((msp_diag_data_head_s *)pstDiagHead->aucData)->ulSn;
    g_stConnectCtrl.stConnectCmd.ulMsgTransId = pstDiagHead->stService.ulMsgTransId;
    g_stConnectCtrl.stConnectCmd.ulMsgId = pstDiagHead->ulCmdId;
    g_stConnectCtrl.connectCommandId.ulID = DIAG_CMD_HOST_DISCONNECT;

    stSendMsg.ulMsgId = ID_MSG_DIAG_CMD_DISCONNECT_REQ;
    stSendMsg.ulSn = ((msp_diag_data_head_s *)pstDiagHead->aucData)->ulSn;
    ret = diag_SendConnectMsg((VOS_UINT8 *)&stSendMsg);
    if (ret) {
        diag_error("send disconnect msg fail, ret:0x%x\n", ret);
        return ERR_MSP_DIAG_SEND_MSG_FAIL;
    }

    DIAG_NotifyConnState(DIAG_STATE_DISCONN);

    return ERR_MSP_SUCCESS;
}

/*
 * Function Name: diag_FwSetChanSta
 * Description: 给C核发送连接状态，在USB拔除等情况使用到
 * Input: VOS_UINT32 flag
 * Output: None
 * Return: VOS_UINT32
 */
VOS_UINT32 diag_SetChanDisconn(MsgBlock *pMsgBlock)
{
    DIAG_CONN_MSG_SEND_T stSendMsg;
    VOS_UINT32 ulCnfRst;
    static VOS_UINT32 ulSn = 0x80000000;

    diag_StopHighTsTimer();
    /* 删除定时器 */
    diag_StopMntnTimer();

    if (diag_GetMiniLogCfg() == DIAG_CFG_SWT_CLOSE) { /* miniDIAG模式下，通道是否上报由SOCP模式控制 */
        mdrv_socp_send_data_manager(SOCP_CODER_DST_OM_IND, SOCP_DEST_DSM_DISABLE);
    }

    g_stConnectCtrl.stConnectCmd.ulMsgId = ID_MSG_DIAG_CMD_DISCONNECT_REQ;
    g_stConnectCtrl.stConnectCmd.ulSn = ulSn;

    stSendMsg.ulMsgId = ID_MSG_DIAG_CMD_DISCONNECT_REQ;
    stSendMsg.ulSn = ulSn;
    ulSn++;
    ulCnfRst = diag_SendConnectMsg((VOS_UINT8 *)&stSendMsg);
    if (ulCnfRst) {
        diag_error("send disconnect msg fail(0x%x)\n", ulCnfRst);
        return ERR_MSP_DIAG_SEND_MSG_FAIL;
    }

    DIAG_NotifyConnState(DIAG_STATE_DISCONN);

    return ERR_MSP_SUCCESS;
}
/*
 * Function Name: diag_ConnMgrSendFuncReg
 * Description: 提供给DIAG内部注册各个子系统的连接相关信息
 * Input: VOS_UINT32 ulConnId:connect id取自于DIAG_CONN_ID_TYPE_E
 * VOS_UINT32 ulChannelNum:该ID使用的源通道数量
 * VOS_UINT32 *ulChannel:该ID使用的通道ID列表
 * DIAG_SEND_PROC_FUNC pSendFuc:连接消息发送函数
 * Output: None
 * Return: VOS_UINT32
 */
VOS_UINT32 diag_ConnMgrSendFuncReg(VOS_UINT32 ulConnId, VOS_UINT32 ulChannelNum, VOS_UINT32 *ulChannel,
    DIAG_SEND_PROC_FUNC pSendFuc)
{
    VOS_UINT32 ulIndex;
    VOS_UINT32 ulChannelId;

    if ((ulConnId >= DIAG_CONN_ID_BUTT) || (ulChannelNum == 0) || (ulChannel == VOS_NULL) || (pSendFuc == VOS_NULL)) {
        diag_error("ulConnId is error, ulConnId:0x%x\n", ulConnId);
        return ERR_MSP_INVALID_PARAMETER;
    }

    if (!g_astConnectTbl[ulConnId].pSendFunc) {
        g_astConnectTbl[ulConnId].pSendFunc = pSendFuc;
        g_astConnectTbl[ulConnId].ulChannelCount = ulChannelNum;
    } else {
        diag_error("repeat register, ulConnId:0x%x\n", ulConnId);
        return ERR_MSP_DIAG_REPEAT_REGISTER;
    }

    for (ulIndex = 0; ulIndex < ulChannelNum; ulIndex++) {
        ulChannelId = ulChannel[ulIndex];
        if (g_astChannelTbl[ulChannelId].ulOwner == DIAG_CONN_ID_ACPU_DEFALUT) {
            g_astChannelTbl[ulChannelId].ulOwner = ulConnId;
            g_astChannelTbl[ulChannelId].ulResult = DIAG_CHANN_REGISTER;
        } else {
            diag_error("repeat register, ulConnId:0x%x ulChannelId:0x%x\n", ulConnId, ulChannelId);
            return ERR_MSP_DIAG_REPEAT_REGISTER;
        }
    }
    return ERR_MSP_SUCCESS;
}

/*
 * Function Name: diag_ConnCnf
 * Description: 连接消息处理完成后的回复函数
 * Input: VOS_UINT32  ulConnId:connect id取自于DIAG_CONN_ID_TYPE_E
 * VOS_UINT32 ulSn:本次回复对应的sn号
 * VOS_UINT32 ulCount:本次回复的源通道数量
 * DIAG_CONNECT_RESULT *pstResult:各个源通道处理结果
 * Output: None
 * Return: VOS_UINT32
 */
VOS_UINT32 diag_ConnCnf(VOS_UINT32 ulConnId, VOS_UINT32 ulSn, VOS_UINT32 ulCount, mdrv_diag_connect_s *pstResult)
{
    if (ulCount != g_astConnectTbl[ulConnId].ulChannelCount) {
        diag_error("%s cnf channel num:0x%x is diff from reg channel num 0x%x\n", g_astConnectTbl[ulConnId].cName,
            ulCount, g_astConnectTbl[ulConnId].ulChannelCount);
        return ERR_MSP_CHNNEL_NUM_ERROR;
    }

    return ERR_MSP_SUCCESS;
}

VOS_VOID diag_RxReady(VOS_VOID)
{
    /* 和开机log无关只要收到了ready命令就打开主动上报 */
    if (diag_GetMiniLogCfg() == DIAG_CFG_SWT_CLOSE) {
        mdrv_socp_send_data_manager(SOCP_CODER_DST_OM_IND, SOCP_DEST_DSM_ENABLE);
    }
}

#else

/*
 * Function Name: diag_GetModemInfo
 * Description: 获取modem信息(用于工具未连接时，工具后台查询单板信息)
 * Input: VOS_VOID
 * Output: None
 * Return: VOS_UINT32
 */
VOS_VOID diag_GetModemInfo(DIAG_CONNECT_FRAME_INFO_STRU *pstDiagHead)
{
    VOS_UINT32 ulCnfRst;
    DIAG_CMD_HOST_CONNECT_CNF_STRU stCnf = { 0 };
    const modem_ver_info_s *pstVerInfo = VOS_NULL;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo;
    errno_t err;

    /* 处理结果 */
    stCnf.ulAuid = ((msp_diag_data_head_s *)(pstDiagHead->aucData))->ulAuid;
    stCnf.ulSn = ((msp_diag_data_head_s *)(pstDiagHead->aucData))->ulSn;

    /* 获取版本信息 */
    pstVerInfo = mdrv_ver_get_info();
    if (pstVerInfo != NULL) {
        /* 获取数采基地址 */
        stCnf.ulChipBaseAddr = (VOS_UINT32)pstVerInfo->product_info.chip_type;
    }

    /* 获取IMEI号 */

    /* 获取软件版本号 */
    err = memset_s(&stCnf.stUeSoftVersion, (VOS_UINT32)sizeof(DIAG_CMD_UE_SOFT_VERSION_STRU), 0,
        (VOS_UINT32)sizeof(DIAG_CMD_UE_SOFT_VERSION_STRU));
    if (err != EOK) {
        diag_error("memset fail:%x\n", err);
    }

    /* 路测信息获取 */
    (VOS_VOID)mdrv_nv_read(EN_NV_ID_AGENT_FLAG, &(stCnf.stAgentFlag), (VOS_UINT32)sizeof(NV_ITEM_AGENT_FLAG_STRU));

    stCnf.diag_cfg.UintValue = 0;

    /* 010: OM通道融合的版本 */
    /* 110: OM融合GU未融合的版本 */
    /* 100: OM完全融合的版本 */
    stCnf.diag_cfg.CtrlFlag.ulDrxControlFlag = 0; /* 和HIDS确认此处不再使用,打桩处理即可 */
    stCnf.diag_cfg.CtrlFlag.ulPortFlag = 0;
    stCnf.diag_cfg.CtrlFlag.ulOmUnifyFlag = 1;

    stCnf.ulLpdMode = 0x5a5a5a5a;
    stCnf.ulRc = ERR_MSP_SUCCESS;

    err = memset_s(stCnf.szProduct, (VOS_UINT32)sizeof(stCnf.szProduct), 0, (VOS_UINT32)sizeof(stCnf.szProduct));
    if (err != EOK) {
        diag_error("memset fail:%x\n", err);
    }
    err = memcpy_s(stCnf.szProduct, (VOS_UINT32)sizeof(stCnf.szProduct), PRODUCT_FULL_VERSION_STR,
        VOS_StrNLen(PRODUCT_FULL_VERSION_STR, sizeof(stCnf.szProduct) - 1));
    if (err != EOK) {
        diag_error("memcpy fail:%x\n", err);
    }

    stDiagInfo.ulSSId = DIAG_SSID_CPU;
    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_MSP;
    stDiagInfo.ulMode = pstDiagHead->stID.mode4b;
    stDiagInfo.ulSubType = pstDiagHead->stID.sec5b;
    stDiagInfo.ulDirection = DIAG_MT_CNF;
    stDiagInfo.ulModemid = 0;
    stDiagInfo.ulMsgId = pstDiagHead->ulCmdId;

    stCnf.ulAuid = ((msp_diag_data_head_s *)(pstDiagHead->aucData))->ulAuid;
    stCnf.ulSn = ((msp_diag_data_head_s *)(pstDiagHead->aucData))->ulSn;

    ulCnfRst = mdrv_diag_report_cnf((diag_cnf_info_s *)&stDiagInfo, (VOS_VOID *)&stCnf, sizeof(stCnf));
    if (ulCnfRst != ERR_MSP_SUCCESS) {
        diag_error("diag_GetModemInfo failed.\n");
    } else {
        diag_crit("diag_GetModemInfo success.\n");
    }

    return;
}
/*
 * Function Name: diag_ConnProc
 * Description: 该函数用于处理ConnProcEntry传进来的HSO连接命令
 * Input: pstReq 待处理数据
 * Output: None
 * Return: VOS_UINT32
 */
VOS_UINT32 diag_ConnProc(VOS_UINT8 *pstReq)
{
    VOS_UINT32 ulCnfRst = ERR_MSP_UNAVAILABLE;
    DIAG_MSG_MSP_CONN_STRU *pstConn = VOS_NULL;
    DIAG_CMD_HOST_CONNECT_CNF_STRU stCnf = { 0 };
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = { 0 };
    const modem_ver_info_s *pstVerInfo = VOS_NULL;
    mdrv_diag_cmd_reply_set_req_s stReplay = { 0 };
    msp_diag_frame_info_s *pstDiagHead = VOS_NULL;
    DIAG_CMD_GET_MDM_INFO_REQ_STRU *pstInfo = VOS_NULL;
    errno_t err;

    pstDiagHead = (msp_diag_frame_info_s *)pstReq;

    mdrv_diag_ptr(EN_DIAG_PTR_MSGMSP_CONN_IN, 1, pstDiagHead->ulCmdId, 0);

    if (diag_CheckModemStatus() != ERR_MSP_SUCCESS) {
        return ERR_MSP_UNAVAILABLE;
    }

    /* 新增获取modem信息的命令用于工具查询单板信息 */
    if (pstDiagHead->ulMsgLen >= sizeof(DIAG_CMD_GET_MDM_INFO_REQ_STRU)) {
        pstInfo = (DIAG_CMD_GET_MDM_INFO_REQ_STRU *)pstDiagHead->aucData;
        if ((pstInfo->ulInfo & DIAG_GET_MODEM_INFO_BIT) == DIAG_GET_MODEM_INFO_BIT) {
            diag_GetModemInfo((DIAG_CONNECT_FRAME_INFO_STRU *)pstDiagHead);
            return ERR_MSP_SUCCESS;
        }
#ifdef DIAG_SEC_TOOLS
        if ((pstInfo->ulInfo & DIAG_VERIFY_SIGN_BIT) == DIAG_VERIFY_SIGN_BIT) {
            diag_ConnAuth((DIAG_CONNECT_FRAME_INFO_STRU *)pstDiagHead);
            return ERR_MSP_SUCCESS;
        }
#endif
    }

    if (pstDiagHead->ulMsgLen < sizeof(msp_diag_data_head_s)) {
        diag_error("rev tool msg len error:0x%x\n", pstDiagHead->ulMsgLen);
        return ERR_MSP_INALID_LEN_ERROR;
    }

    diag_crit("Receive tool connect cmd!\n");

#ifdef DIAG_SEC_TOOLS
    g_ulAuthState = DIAG_AUTH_TYPE_DEFAULT;
#endif
    pstConn = (DIAG_MSG_MSP_CONN_STRU *)VOS_AllocMsg(MSP_PID_DIAG_APP_AGENT,
        (VOS_UINT32)(sizeof(DIAG_MSG_MSP_CONN_STRU) - VOS_MSG_HEAD_LENGTH));
    if (pstConn == VOS_NULL) {
        ulCnfRst = ERR_MSP_DIAG_ALLOC_MALLOC_FAIL;
        goto DIAG_ERROR;
    }

    /* 设置连接状态开关值 */
    ulCnfRst = diag_CfgSetGlobalBitValue(&g_ulDiagCfgInfo, DIAG_CFG_CONN_BIT, DIAG_CFG_SWT_OPEN);
    if (ulCnfRst) {
        diag_error("Open DIAG_CFG_CONN_BIT failed.\n");
        goto DIAG_ERROR;
    }

    /* 获取版本信息 */
    pstVerInfo = mdrv_ver_get_info();
    if (pstVerInfo != NULL) {
        /* 获取数采基地址 */
        pstConn->stConnInfo.ulChipBaseAddr = (VOS_UINT32)pstVerInfo->product_info.chip_type;
    }

    /* 获取IMEI号 */

    /* 获取软件版本号 */
    err = memset_s(&pstConn->stConnInfo.stUeSoftVersion, (VOS_UINT32)sizeof(DIAG_CMD_UE_SOFT_VERSION_STRU), 0,
        (VOS_UINT32)sizeof(DIAG_CMD_UE_SOFT_VERSION_STRU));
    if (err != EOK) {
        diag_error("memset fail:%x\n", err);
    }

    /* 设置鉴权padding 模式 */
    *(VOS_UINT32 *)&(pstConn->stConnInfo.stAgentFlag) = 1; /* rsa signature:SEC_RSA_PSS_PADDING */

    pstConn->stConnInfo.diag_cfg.UintValue = 0;

    /* 010: OM通道融合的版本 */
    /* 110: OM融合GU未融合的版本 */
    /* 100: OM完全融合的版本 */
    pstConn->stConnInfo.diag_cfg.CtrlFlag.ulDrxControlFlag = 0; /* 和HIDS确认此处不再使用,打桩处理即可 */
    pstConn->stConnInfo.diag_cfg.CtrlFlag.ulPortFlag = 0;
    pstConn->stConnInfo.diag_cfg.CtrlFlag.ulOmUnifyFlag = 1;
#ifdef DIAG_SEC_TOOLS
    pstConn->stConnInfo.diag_cfg.CtrlFlag.ulAuthFlag = 1;
#else
    pstConn->stConnInfo.diag_cfg.CtrlFlag.ulAuthFlag = 0;
#endif
    pstConn->stConnInfo.ulLpdMode = 0x5a5a5a5a;

    err = memset_s(pstConn->stConnInfo.szProduct, (VOS_UINT32)sizeof(pstConn->stConnInfo.szProduct), 0,
        (VOS_UINT32)sizeof(pstConn->stConnInfo.szProduct));
    if (err != EOK) {
        diag_error("memset fail:%x\n", err);
    }

    err = memcpy_s(pstConn->stConnInfo.szProduct, (VOS_UINT32)sizeof(pstConn->stConnInfo.szProduct),
        PRODUCT_FULL_VERSION_STR, VOS_StrNLen(PRODUCT_FULL_VERSION_STR, sizeof(pstConn->stConnInfo.szProduct) - 1));
    if (err != EOK) {
        diag_error("memcpy fail:%x\n", err);
    }

    ulCnfRst = diag_SendMsg(MSP_PID_DIAG_APP_AGENT, PS_PID_MM, ID_MSG_DIAG_CMD_REPLAY_TO_PS, (VOS_UINT8 *)&stReplay,
        (VOS_UINT32)sizeof(mdrv_diag_cmd_reply_set_req_s));
    if (ulCnfRst) {
        ulCnfRst = ERR_MSP_DIAG_SEND_MSG_FAIL;
        goto DIAG_ERROR;
    }

    /* 处理结果 */
    pstConn->stConnInfo.ulAuid = ((msp_diag_data_head_s *)(pstDiagHead->aucData))->ulAuid;
    pstConn->stConnInfo.ulSn = ((msp_diag_data_head_s *)(pstDiagHead->aucData))->ulSn;
    pstConn->stConnInfo.ulRc = ERR_MSP_SUCCESS;

    pstConn->ulReceiverPid = MSP_PID_DIAG_AGENT;
    pstConn->ulSenderPid = MSP_PID_DIAG_APP_AGENT;
    pstConn->ulCmdId = pstDiagHead->ulCmdId;
    pstConn->ulMsgId = DIAG_MSG_MSP_CONN_REQ;

    ulCnfRst = VOS_SendMsg(MSP_PID_DIAG_APP_AGENT, pstConn);
    if (ulCnfRst == ERR_MSP_SUCCESS) {
        /* 复位维测信息记录 */
        mdrv_diag_reset_mntn_info(DIAGLOG_SRC_MNTN);
        mdrv_diag_reset_mntn_info(DIAGLOG_DST_MNTN);

        mdrv_applog_conn();

        mdrv_hds_printlog_conn();

        mdrv_hds_translog_conn();

        DIAG_NotifyConnState(DIAG_STATE_CONN);

#ifndef DIAG_SEC_TOOLS
        if (!DIAG_IS_POLOG_ON) {
            mdrv_socp_send_data_manager(SOCP_CODER_DST_OM_IND, SOCP_DEST_DSM_ENABLE);
        }
#endif
        diag_crit("Diag send ConnInfo to Modem success.\n");

        return ulCnfRst;
    }

DIAG_ERROR:

    if (pstConn != VOS_NULL) {
        if (VOS_FreeMsg(MSP_PID_DIAG_APP_AGENT, pstConn)) {
            diag_error("free mem fail\n");
        }
    }

    stCnf.ulRc = ulCnfRst;

    stDiagInfo.ulSSId = DIAG_SSID_CPU;
    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_MSP;
    stDiagInfo.ulMode = pstDiagHead->stID.mode4b;
    stDiagInfo.ulSubType = pstDiagHead->stID.sec5b;
    stDiagInfo.ulDirection = DIAG_MT_CNF;
    stDiagInfo.ulModemid = 0;
    stDiagInfo.ulMsgId = pstDiagHead->ulCmdId;

    stCnf.ulAuid = ((msp_diag_data_head_s *)(pstDiagHead->aucData))->ulAuid;
    stCnf.ulSn = ((msp_diag_data_head_s *)(pstDiagHead->aucData))->ulSn;

    ulCnfRst = mdrv_diag_report_cnf((diag_cnf_info_s *)&stDiagInfo, &stCnf, (VOS_UINT32)sizeof(stCnf));

    diag_error("diag connect failed.\n");

    return ulCnfRst;
}


VOS_UINT32 diag_SetChanDisconn(MsgBlock *pMsgBlock)
{
#ifdef DIAG_SEC_TOOLS
    g_ulAuthState = DIAG_AUTH_TYPE_DEFAULT;
#endif

    if (!DIAG_IS_CONN_ON) {
        return 0;
    } else {
        diag_ConnReset();
        diag_CfgResetAllSwt();

        /* 删除定时器 */
        diag_StopMntnTimer();

        mdrv_hds_printlog_disconn();

        mdrv_hds_translog_disconn();

        /* 将状态发送给C核 */
        (VOS_VOID)diag_SendMsg(MSP_PID_DIAG_APP_AGENT, MSP_PID_DIAG_AGENT, ID_MSG_DIAG_HSO_DISCONN_IND, VOS_NULL, 0);

        mdrv_socp_send_data_manager(SOCP_CODER_DST_OM_IND, SOCP_DEST_DSM_DISABLE);

        DIAG_NotifyConnState(DIAG_STATE_DISCONN);
    }

    return 0;
}

/*
 * Function Name: diag_DisConnProc
 * Description: 该函数用于处理ConnProcEntry传进来的HSO断开命令
 * Input: pstReq 待处理数据
 * Output: None
 * Return: VOS_UINT32
 */
VOS_UINT32 diag_DisConnProc(VOS_UINT8 *pstReq)
{
    VOS_UINT32 ret;
    DIAG_CMD_HOST_DISCONNECT_CNF_STRU stCnfDisConn = { 0 };
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = { 0 };
    msp_diag_frame_info_s *pstDiagHead = VOS_NULL;
    VOS_UINT32 ulLen;
    DIAG_MSG_TRANS_STRU *pstInfo = VOS_NULL;

    diag_crit("Receive tool disconnect cmd!\n");

#ifdef DIAG_SEC_TOOLS
    /* 清空鉴权状态 */
    g_ulAuthState = DIAG_AUTH_TYPE_DEFAULT;
#endif

    pstDiagHead = (msp_diag_frame_info_s *)pstReq;

    if (pstDiagHead->ulMsgLen < sizeof(msp_diag_data_head_s)) {
        diag_error("rev tool msg len error:0x%x\n", pstDiagHead->ulMsgLen);
        return ERR_MSP_INALID_LEN_ERROR;
    }

    mdrv_diag_ptr(EN_DIAG_PTR_MSGMSP_DISCONN_IN, 1, pstDiagHead->ulCmdId, 0);

    /* 重置所有开关状态为未打开 */
    diag_ConnReset();
    diag_CfgResetAllSwt();

    /* 删除定时器 */
    diag_StopMntnTimer();

    DIAG_MSG_ACORE_CFG_PROC(ulLen, pstDiagHead, pstInfo, ret);

    mdrv_hds_printlog_disconn();

    mdrv_hds_translog_disconn();

    mdrv_socp_send_data_manager(SOCP_CODER_DST_OM_IND, SOCP_DEST_DSM_DISABLE);

    DIAG_NotifyConnState(DIAG_STATE_DISCONN);

    return ret;

DIAG_ERROR:

    DIAG_MSG_COMMON_PROC(stDiagInfo, stCnfDisConn, pstDiagHead);

    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_MSP;

    stCnfDisConn.ulRc = ret;

    ret = DIAG_MsgReport(&stDiagInfo, &stCnfDisConn, (VOS_UINT32)sizeof(stCnfDisConn));

    diag_error("diag disconnect failed.\n");

    return ret;
}
#ifdef DIAG_SEC_TOOLS
VOS_VOID diag_ConnAuth(DIAG_CONNECT_FRAME_INFO_STRU *pstDiagHead)
{
    VOS_UINT32 ulRet;

    g_ulAuthState = DIAG_AUTH_TYPE_AUTHING;

    /* 鉴权消息DIAG 三级头+ 鉴权key */
    if ((pstDiagHead->ulMsgLen < sizeof(msp_diag_data_head_s)) || (pstDiagHead->ulMsgLen > DIAG_FRAME_SUM_LEN)) {
        diag_error("rev msglen is too small, len:0x%x\n", pstDiagHead->ulMsgLen);
        g_ulAuthState = DIAG_AUTH_TYPE_DEFAULT;
        (VOS_VOID)diag_FailedCmdCnf((msp_diag_frame_info_s *)pstDiagHead, ERR_MSP_INALID_LEN_ERROR);
        return;
    }

    ulRet = diag_SendMsg(MSP_PID_DIAG_APP_AGENT, MSP_PID_DIAG_AGENT, DIAG_MSG_MSP_AUTH_REQ, pstDiagHead->aucData,
        pstDiagHead->ulMsgLen);
    if (ulRet) {
        g_ulAuthState = DIAG_AUTH_TYPE_DEFAULT;
        diag_FailedCmdCnf((msp_diag_frame_info_s *)pstDiagHead, ulRet);
    }

    return;
}
VOS_VOID diag_ConnAuthRst(MsgBlock *pMsgBlock)
{
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {};
    DIAG_CMD_AUTH_CNF_STRU stCmdAuthCnf = {};
    DIAG_DATA_MSG_STRU *pstMsg = (DIAG_DATA_MSG_STRU *)pMsgBlock;
    DIAG_AUTH_CNF_STRU *pstAuthRst = NULL;

    /* 不是在鉴权中 */
    if (g_ulAuthState != DIAG_AUTH_TYPE_AUTHING) {
        diag_error("no auth req, g_ulAuthState:0x%x\n", g_ulAuthState);
        return;
    }

    if (pstMsg->ulLength < sizeof(DIAG_DATA_MSG_STRU) - VOS_MSG_HEAD_LENGTH + sizeof(DIAG_AUTH_CNF_STRU)) {
        diag_error("rev datalen is too small,len:0x%x\n", pstMsg->ulLength);
        return;
    }

    pstAuthRst = (DIAG_AUTH_CNF_STRU *)(pstMsg->pContext);

    /* success */
    if (pstAuthRst->ulRet == ERR_MSP_SUCCESS) {
        g_ulAuthState = DIAG_AUTH_TYPE_SUCCESS;
        mdrv_socp_send_data_manager(SOCP_CODER_DST_OM_IND, SOCP_DEST_DSM_ENABLE);
        diag_crit("tools auth success\n");
    } else {
        diag_error("tools auth fail, ret:0x%x\n", pstAuthRst->ulRet);
    }

    /* CNF to tools */
    stCmdAuthCnf.ulAuid = pstAuthRst->stMdmInfo.ulAuid;
    stCmdAuthCnf.ulSn = pstAuthRst->stMdmInfo.ulSn;
    stCmdAuthCnf.ulRc = pstAuthRst->ulRet;

    stDiagInfo.ulSSId = DIAG_SSID_CPU;
    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_MSP;
    stDiagInfo.ulMode = DIAG_MODE_LTE;
    stDiagInfo.ulSubType = DIAG_MSG_CMD;
    stDiagInfo.ulDirection = DIAG_MT_CNF;
    stDiagInfo.ulModemid = 0;
    stDiagInfo.ulMsgId = DIAG_CMD_HOST_CONNECT;
    stDiagInfo.ulTransId = 0;

    (VOS_VOID)DIAG_MsgReport(&stDiagInfo, (VOS_VOID *)&stCmdAuthCnf, sizeof(stCmdAuthCnf));
    return;
}
VOS_VOID diag_set_auth_state(VOS_UINT32 auth_state)
{
    g_ulAuthState = auth_state;
}
VOS_UINT32 diag_get_auth_state(VOS_VOID)
{
    return g_ulAuthState;
}
#endif
VOS_UINT32 diag_DisConnMgrProc(VOS_UINT8 *pstReq)
{
    return ERR_MSP_SUCCESS;
}
VOS_UINT32 diag_ConnMgrProc(VOS_UINT8 *pstReq)
{
    return ERR_MSP_SUCCESS;
}
VOS_UINT32 diag_ConnMgrSendFuncReg(VOS_UINT32 ulConnId, VOS_UINT32 ulChannelNum, VOS_UINT32 *ulChannel,
    DIAG_SEND_PROC_FUNC pSendFuc)
{
    return ERR_MSP_SUCCESS;
}
VOS_UINT32 diag_ConnCnf(VOS_UINT32 ulConnId, VOS_UINT32 ulSn, VOS_UINT32 ulCount, mdrv_diag_connect_s *pstResult)
{
    return ERR_MSP_SUCCESS;
}

#endif

/*
 * Function Name: diag_CheckModemStatus
 * Description: check modem status is ready or not
 * Input: None
 * Output: None
 * Return: VOS_UINT32
 */
VOS_UINT32 diag_CheckModemStatus(VOS_VOID)
{
#ifdef DIAG_SYSTEM_5G
    const modem_ver_info_s *ver_info = NULL;
#ifdef DIAG_MODEM_SINGLE_CORE
    VOS_UINT32 phy_state;
#endif
#endif
    VOS_UINT32 result;

    result = VOS_CheckPidValidity(MSP_PID_DIAG_AGENT);
    if (result != VOS_PID_AVAILABLE) {
        diag_error("LR PID Table is not ready,ulResult=0x%x\n", result);
        return ERR_MSP_UNAVAILABLE;
    }
#ifdef DIAG_SYSTEM_5G
    ver_info = mdrv_ver_get_info();
    if (ver_info->product_info.board_a_type == BOARD_TYPE_ASIC) {
        result = VOS_CheckPidValidity(MSP_PID_DIAG_NRM_AGENT);
        if (result != VOS_PID_AVAILABLE) {
            diag_error("NR PID Table is not ready,ulResult=0x%x\n", result);
            return ERR_MSP_UNAVAILABLE;
        }
#ifdef DIAG_MODEM_SINGLE_CORE /* modem单core平台等待phy加载镜像完成后连接 */
        phy_state = diag_GetPhyPowerOnState();
        if (phy_state == DIAG_PHY_POWER_ON_DOING) {
            return ERR_MSP_UNAVAILABLE;
        } else if (phy_state == DIAG_PHY_POWER_ON_FAIL) {
            diag_error("phy power on fail\n");
        } else {
            return ERR_MSP_SUCCESS;
        }
#endif
    }
#endif
    return ERR_MSP_SUCCESS;
}

mdrv_diag_conn_state_notify_fun g_ConnStateNotifyFun[DIAG_CONN_STATE_FUN_NUM] = {
    VOS_NULL, VOS_NULL, VOS_NULL, VOS_NULL, VOS_NULL
};

unsigned int mdrv_diag_conn_state_notify_fun_reg(mdrv_diag_conn_state_notify_fun pfun)
{
    VOS_UINT32 ulFunIndex;

    for (ulFunIndex = 0; ulFunIndex < sizeof(g_ConnStateNotifyFun) / sizeof(mdrv_diag_conn_state_notify_fun);
        ulFunIndex++) {
        if (g_ConnStateNotifyFun[ulFunIndex] == VOS_NULL) {
            g_ConnStateNotifyFun[ulFunIndex] = pfun;
            return ERR_MSP_SUCCESS;
        }
    }

    diag_error("no space to register new func\n");
    return ERR_MSP_NOT_FREEE_SPACE;
}

VOS_VOID DIAG_NotifyConnState(unsigned int state)
{
    VOS_UINT32 ulFunIndex;

    for (ulFunIndex = 0; ulFunIndex < sizeof(g_ConnStateNotifyFun) / sizeof(mdrv_diag_conn_state_notify_fun);
        ulFunIndex++) {
        if (g_ConnStateNotifyFun[ulFunIndex] != VOS_NULL) {
            g_ConnStateNotifyFun[ulFunIndex](state);
        }
    }
}

VOS_VOID DIAG_ShowConnStateNotifyFun(VOS_VOID)
{
    VOS_UINT32 ulFunIndex;

    for (ulFunIndex = 0; ulFunIndex < sizeof(g_ConnStateNotifyFun) / sizeof(mdrv_diag_conn_state_notify_fun);
        ulFunIndex++) {
        if (g_ConnStateNotifyFun[ulFunIndex] != VOS_NULL) {
            diag_crit("%d %s\n", ulFunIndex, g_ConnStateNotifyFun[ulFunIndex]);
        }
    }

    return;
}

#define DIAG_NV_IMEI_LEN 15

VOS_UINT32 diag_GetImei(VOS_CHAR szimei[])
{
    VOS_UINT32 ret;
    VOS_UINT32 uslen;
    VOS_UINT32 subscript;
    VOS_CHAR auctemp[DIAG_NV_IMEI_LEN + 1] = {0};

    uslen = DIAG_NV_IMEI_LEN + 1;

    ret = mdrv_nv_read(0, auctemp, uslen);
    if (ret != 0) {
        return ret;
    } else {
        for (subscript = 0; subscript < uslen; subscript++) {
            *(szimei + subscript) = *(auctemp + subscript) + 0x30; /* 字符转换 */
        }
        szimei[DIAG_NV_IMEI_LEN] = 0;
    }

    return 0;
}
