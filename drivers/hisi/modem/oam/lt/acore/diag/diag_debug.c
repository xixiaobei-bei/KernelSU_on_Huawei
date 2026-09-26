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
#include <mdrv.h>
#include "soc_socp_adapter.h"
#include "nv_stru_pam.h"
#include "nv_stru_sys.h"
#include "nv_stru_drv.h"
#include "acore_nv_stru_drv.h"
#include "diag_debug.h"
#include "diag_api.h"
#include "diag_cfg.h"
#include "diag_connect.h"
#include "diag_msgmsp.h"
#include "diag_common.h"
#include "diag_acore_common.h"


#define THIS_FILE_ID MSP_FILE_ID_DIAG_DEBUG_C

VOS_UINT32 g_DiagDebugCfg = DIAG_CFG_SWT_CLOSE;

/*
 * Function Name   : CBT : Count Branch Timestamp (计数、分支、时间戳定位功能)
 * Description     : 用于统计次数和所走分支的问题定位
 */
DIAG_CBT_INFO_TBL_STRU g_astCBTInfoTbl[EN_DIAG_DEBUG_INFO_MAX] = {{0}};

DIAG_CBT_INFO_TBL_STRU *diag_DebugGetInfo(VOS_VOID)
{
    return g_astCBTInfoTbl;
}

/*
 * Function Name: diag_CBT
 * Description:
 */
VOS_VOID diag_CBT(DIAG_CBT_ID_ENUM ulType, VOS_UINT32 ulRserved1, VOS_UINT32 ulRserved2, VOS_UINT32 ulRserved3)
{
    g_astCBTInfoTbl[ulType].ulCalledNum += 1;
    g_astCBTInfoTbl[ulType].ulRserved1 = ulRserved1;
    g_astCBTInfoTbl[ulType].ulRserved2 = ulRserved2;
    g_astCBTInfoTbl[ulType].ulRserved3 = ulRserved3;
    g_astCBTInfoTbl[ulType].ulRtcTime = VOS_GetTick();
}

/*
 * Function Name   : LNR : Last N Ring buffer store (最后N条信息循环存储功能)
 * Description     : 保存最近的N条信息
 */
DIAG_LNR_INFO_TBL_STRU g_astLNRInfoTbl[EN_DIAG_LNR_INFO_MAX] = {{0}};


VOS_VOID diag_LNR(DIAG_LNR_ID_ENUM ulType, VOS_UINT32 ulRserved1, VOS_UINT32 ulRserved2)
{
    g_astLNRInfoTbl[ulType].ulRserved1[g_astLNRInfoTbl[ulType].ulCur] = ulRserved1;
    g_astLNRInfoTbl[ulType].ulRserved2[g_astLNRInfoTbl[ulType].ulCur] = ulRserved2;
    g_astLNRInfoTbl[ulType].ulCur = (g_astLNRInfoTbl[ulType].ulCur + 1) % DIAG_LNR_NUMBER;
}

VOS_VOID DIAG_ShowLNR(DIAG_LNR_ID_ENUM ulType, VOS_UINT32 n)
{
    VOS_UINT32 i;
    VOS_UINT32 cur;

    cur = (g_astLNRInfoTbl[ulType].ulCur + DIAG_LNR_NUMBER - n) % DIAG_LNR_NUMBER;

    for (i = 0; i < n; i++) {
        diag_crit("0x%x, 0x%x.\n", g_astLNRInfoTbl[ulType].ulRserved1[cur], g_astLNRInfoTbl[ulType].ulRserved2[cur]);
        cur = (cur + 1) % DIAG_LNR_NUMBER;
    }
}
/*
 * Function Name: DIAG_ShowLogCfg
 * Description: 根据输入的任务PID查询log配置开关信息
 */
VOS_VOID DIAG_ShowLogCfg(VOS_UINT32 ulModuleId)
{
    VOS_UINT32 level;

    /* 检查DIAG是否初始化且HSO是否连接上 */
    if (!DIAG_IS_CONN_ON) {
        diag_crit("do not connect tool\n");
    }

    if (DIAG_CFG_MODULE_IS_INVALID((VOS_INT32)ulModuleId)) {
        diag_crit("invalid PID. PID range(%d -- %d)\n", VOS_PID_DOPRAEND, VOS_PID_BUTT);
        return;
    }

    diag_crit("open global switch 0x%x(0xffffffff = invalid).\n", g_PrintTotalCfg);

    /* level中存储的值(0|ERROR|WARNING|NORMAL|INFO|0|0|0) bit 6-3 分别表示ERROR-INFO */
    level = g_PrintModuleCfg[ulModuleId - VOS_PID_DOPRAEND];
    if (level & 0x08) {
        diag_crit("PID %d print level: info.\n", ulModuleId);
    } else if (level & 0x10) {
        diag_crit("PID %d print level: normal.\n", ulModuleId);
    } else if (level & 0x20) {
        diag_crit("PID %d print level: warning.\n", ulModuleId);
    } else if (level & 0x40) {
        diag_crit("PID %d print level: error.\n", ulModuleId);
    } else {
        diag_crit("PID %d print level: off.\n", ulModuleId);
    }

    diag_crit("PRINT failed cnt: %d.\n", g_astCBTInfoTbl[EN_DIAG_CBT_API_PRINTFV_ERR].ulCalledNum);
    diag_crit("PRINT success cnt: %d.\n", g_astCBTInfoTbl[EN_DIAG_CBT_API_PRINTFV_OK].ulCalledNum);
}

/*
 * Function Name: DIAG_ShowEventCfg
 * Description: 查询EVENT配置开关信息
 */
VOS_VOID DIAG_ShowEventCfg(VOS_UINT32 ulpid)
{
    VOS_UINT8 *event_cfg = diag_get_event_cfg();
    /* 检查DIAG是否初始化且HSO是否连接上 */
    if (!DIAG_IS_CONN_ON) {
        diag_crit("do not connect tool\n");
    } else if (!DIAG_IS_EVENT_ON) {
        diag_crit("event global switch closed\n");
    } else {
        if (DIAG_CFG_MODULE_IS_INVALID(ulpid)) {
            diag_crit("Invalid PID: A(%d -- %d), C(%d -- %d)!\n", VOS_PID_CPU_ID_1_DOPRAEND, VOS_CPU_ID_1_PID_BUTT,
                VOS_PID_CPU_ID_0_DOPRAEND, VOS_CPU_ID_0_PID_BUTT);
        } else {
            if (event_cfg[ulpid - VOS_PID_DOPRAEND]) {
                diag_crit("ulpid %d event switch opened\n", ulpid);
            } else {
                diag_crit("ulpid %d event switch closed\n", ulpid);
            }
        }
    }

    diag_crit("Event report failed cnt: %d.\n", g_astCBTInfoTbl[EN_DIAG_CBT_API_EVENT_ERR].ulCalledNum);
    diag_crit("Event report success cnt: %d.\n", g_astCBTInfoTbl[EN_DIAG_CBT_API_EVENT_OK].ulCalledNum);
}

/*
 * Function Name: DIAG_ShowAirCfg
 * Description: 查询空口配置开关信息
 */
VOS_VOID DIAG_ShowAirCfg(VOS_VOID)
{
    /* 检查DIAG是否初始化且HSO是否连接上 */
    if (!DIAG_IS_CONN_ON) {
        diag_crit("do not connect tool\n");
    } else if (!DIAG_IS_LT_AIR_ON) {
        diag_crit("air global switch closed\n");
    } else {
        diag_crit("air global switch opend\n");
    }
    diag_crit("AIR report failed cnt: %d.\n", g_astCBTInfoTbl[EN_DIAG_CBT_API_AIR_ERR].ulCalledNum);
    diag_crit("AIR report success cnt: %d.\n", g_astCBTInfoTbl[EN_DIAG_CBT_API_AIR_OK].ulCalledNum);
}

static VOS_VOID DIAG_ShowLayerSrcCfg(VOS_UINT32 ulModuleId)
{
    VOS_UINT32 ulOffset;
    VOS_UINT32 ulState = 0;

    if (DIAG_CFG_LAYER_MODULE_IS_ACORE(ulModuleId)) {
        ulOffset = DIAG_CFG_LAYER_MODULE_ACORE_OFFSET(ulModuleId);
        if (g_ALayerSrcModuleCfg[ulOffset]) {
            ulState = 1;
        }
    } else if (DIAG_CFG_LAYER_MODULE_IS_CCORE(ulModuleId)) {
        ulOffset = DIAG_CFG_LAYER_MODULE_CCORE_OFFSET(ulModuleId);
        if (g_CLayerSrcModuleCfg[ulOffset]) {
            ulState = 1;
        }
    } else if (DIAG_CFG_LAYER_MODULE_IS_NRM(ulModuleId)) {
        ulOffset = DIAG_CFG_LAYER_MODULE_NRM_OFFSET(ulModuleId);
        if (g_NrmLayerSrcModuleCfg[ulOffset]) {
            ulState = 1;
        }
    } else {
        diag_crit("Invalid PID: A(%d -- %d), C(%d -- %d), NRM(%d -- %d)!\n", VOS_PID_CPU_ID_1_DOPRAEND,
            VOS_CPU_ID_1_PID_BUTT, VOS_PID_CPU_ID_0_DOPRAEND, VOS_CPU_ID_0_PID_BUTT, VOS_PID_CPU_ID_2_DOPRAEND,
            VOS_CPU_ID_2_PID_BUTT);

        return;
    }

    diag_crit("SRC module %d switch status is %s .\n", ulModuleId, ulState ? "Opend" : "closed");

    return;
}
static VOS_VOID DIAG_ShowLayerDstCfg(VOS_UINT32 ulModuleId)
{
    VOS_UINT32 ulOffset;
    VOS_UINT32 ulState = 0;

    if (DIAG_CFG_LAYER_MODULE_IS_ACORE(ulModuleId)) {
        ulOffset = DIAG_CFG_LAYER_MODULE_ACORE_OFFSET(ulModuleId);
        if (g_ALayerDstModuleCfg[ulOffset]) {
            ulState = 1;
        }
    } else if (DIAG_CFG_LAYER_MODULE_IS_CCORE(ulModuleId)) {
        ulOffset = DIAG_CFG_LAYER_MODULE_CCORE_OFFSET(ulModuleId);
        if (g_CLayerDstModuleCfg[ulOffset]) {
            ulState = 1;
        }
    } else if (DIAG_CFG_LAYER_MODULE_IS_NRM(ulModuleId)) {
        ulOffset = DIAG_CFG_LAYER_MODULE_NRM_OFFSET(ulModuleId);
        if (g_NrmLayerDstModuleCfg[ulOffset]) {
            ulState = 1;
        }
    } else {
        diag_crit("Invalid PID: A(%d -- %d), C(%d -- %d), NRM(%d -- %d)!\n", VOS_PID_CPU_ID_1_DOPRAEND,
            VOS_CPU_ID_1_PID_BUTT, VOS_PID_CPU_ID_0_DOPRAEND, VOS_CPU_ID_0_PID_BUTT, VOS_PID_CPU_ID_2_DOPRAEND,
            VOS_CPU_ID_2_PID_BUTT);

        return;
    }

    diag_crit("DST module %d switch status is %s .\n", ulModuleId, ulState ? "opened" : "closed");
}
/*
 * Function Name: DIAG_ShowLayerCfg
 * Description: 根据输入的任务PID查询层间消息配置开关信息
 */
VOS_VOID DIAG_ShowLayerCfg(VOS_UINT32 ulModuleId, VOS_UINT32 ulSrcDst)
{
    /* 检查DIAG是否初始化且HSO是否连接上 */
    if (!DIAG_IS_CONN_ON) {
        diag_crit("do not connect tool\n");
    }

    if (ulSrcDst == DIAG_CMD_LAYER_MOD_SRC) {
        DIAG_ShowLayerSrcCfg(ulModuleId);
    } else {
        DIAG_ShowLayerDstCfg(ulModuleId);
    }

    diag_crit("Layer report failed cnt: %d.\n", g_astCBTInfoTbl[EN_DIAG_CBT_API_TRACE_ERR].ulCalledNum);
    diag_crit("Layer report success cnt: %d.\n", g_astCBTInfoTbl[EN_DIAG_CBT_API_TRACE_OK].ulCalledNum);

    diag_crit("vos hook layer report failed cnt: %d.\n", g_astCBTInfoTbl[EN_DIAG_API_MSG_LAYER_ERR].ulCalledNum);
    diag_crit("vos hook layer report success cnt: %d.\n", g_astCBTInfoTbl[EN_DIAG_API_MSG_LAYER_OK].ulCalledNum);
    diag_crit("vos hook layer report filtered cnt: %d: srcid 0x%x, dstid 0x%x, msgid 0x%x.\n",
        g_astCBTInfoTbl[EN_DIAG_API_MSG_LAYER_MATCH].ulCalledNum,
        g_astCBTInfoTbl[EN_DIAG_API_MSG_LAYER_MATCH].ulRserved1,
        g_astCBTInfoTbl[EN_DIAG_API_MSG_LAYER_MATCH].ulRserved2,
        g_astCBTInfoTbl[EN_DIAG_API_MSG_LAYER_MATCH].ulRserved3);
}

/*
 * Function Name: DIAG_ShowUsrCfg
 * Description: 查询用户面配置开关信息
 */
VOS_VOID DIAG_ShowUsrCfg(VOS_VOID)
{
    diag_crit("User Plane report failed cnt: %d.\n", g_astCBTInfoTbl[EN_DIAG_CBT_API_USER_ERR].ulCalledNum);
    diag_crit("User Plane report success cnt: %d.\n", g_astCBTInfoTbl[EN_DIAG_CBT_API_USER_OK].ulCalledNum);
}

/*
 * Function Name: DIAG_ShowTrans
 * Description: 查询最后n个透传上报相关信息
 */
VOS_VOID DIAG_ShowTrans(VOS_UINT32 n)
{
    diag_crit("Trance report failed cnt: %d.\n", g_astCBTInfoTbl[EN_DIAG_CBT_API_TRANS_ERR].ulCalledNum);
    diag_crit("Trance report success cnt: %d.\n", g_astCBTInfoTbl[EN_DIAG_CBT_API_TRANS_OK].ulCalledNum);

    diag_crit("The last %d trance messages:\n", n);

    DIAG_ShowLNR(EN_DIAG_LNR_TRANS_IND, n);
}

/*
 * Function Name: DIAG_ShowTrans
 * Description: 查询最后n个透传上报相关信息
 */
VOS_VOID DIAG_ShowPsTransCmd(VOS_UINT32 n)
{
    diag_crit("The last %d trance messages:\n", n);
    DIAG_ShowLNR(EN_DIAG_LNR_PS_TRANS, n);
}

HTIMER g_DebugTimer;
DIAG_MNTN_API_OK_STRU g_ind_src_mntn_info = {};
/*
 * Function Name: diag_ReportMntn
 * Description: 通过控制通道定时上报可维可测信息
 */
VOS_VOID diag_ReportSrcMntn(VOS_VOID)
{
    DIAG_DEBUG_SRC_MNTN_STRU stDiagInfo = {};
    diag_debug_info_s *pstDebugInfo = NULL;
    VOS_UINT32 ulRet;
    static VOS_UINT32 last_slice = 0;
    VOS_UINT32 current_slice;
    errno_t err;

    stDiagInfo.pstMntnHead.ulModule = MDRV_DIAG_GEN_MODULE_EX(DIAG_MODEM_0, DIAG_MODE_LTE, DIAG_MSG_TYPE_BSP);
    stDiagInfo.pstMntnHead.ulPid = DIAG_AGENT_PID;
    stDiagInfo.pstMntnHead.ulMsgId = DIAG_DEBUG_AP_SRC_MNTN_CMDID;
    stDiagInfo.pstMntnHead.ulReserve = 0;
    stDiagInfo.pstMntnHead.ulLength = sizeof(stDiagInfo.pstMntnInfo);

    stDiagInfo.pstMntnInfo.ulChannelId = SOCP_CODER_SRC_PS_IND;
    err = memcpy_s(stDiagInfo.pstMntnInfo.chanName, sizeof(stDiagInfo.pstMntnInfo.chanName), DIAG_SRC_NAME,
        sizeof(DIAG_SRC_NAME));
    if (err != EOK) {
        diag_error("memcpy fail:%x\n", err);
    }
    current_slice = mdrv_timer_get_normal_timestamp();
    stDiagInfo.pstMntnInfo.ulDeltaTime = DIAG_SLICE_DELTA(last_slice, current_slice);

    pstDebugInfo = mdrv_diag_get_mntn_info(DIAGLOG_SRC_MNTN);

    stDiagInfo.pstMntnInfo.ulPackageLen = pstDebugInfo->package_len;
    stDiagInfo.pstMntnInfo.ulPackageNum = pstDebugInfo->package_num;

    stDiagInfo.pstMntnInfo.ulAbandonLen = pstDebugInfo->abandon_len;
    stDiagInfo.pstMntnInfo.ulAbandonNum = pstDebugInfo->abandon_num;

    stDiagInfo.pstMntnInfo.ulThrputEnc =
        ((((stDiagInfo.pstMntnInfo.ulPackageLen - stDiagInfo.pstMntnInfo.ulAbandonLen) / 1024) * 32768) /
        stDiagInfo.pstMntnInfo.ulDeltaTime) *
        1024;

    stDiagInfo.pstMntnInfo.ulOverFlow50Num = pstDebugInfo->overflow_50_num;
    stDiagInfo.pstMntnInfo.ulOverFlow80Num = pstDebugInfo->overflow_80_num;

    /* 各类消息上报次数 */
    stDiagInfo.pstMntnInfo.ulTraceNum =
        g_astCBTInfoTbl[EN_DIAG_CBT_API_TRACE_OK].ulCalledNum - g_ind_src_mntn_info.ulTraceNum;
    stDiagInfo.pstMntnInfo.ulLayerNum =
        g_astCBTInfoTbl[EN_DIAG_API_MSG_LAYER_OK].ulCalledNum - g_ind_src_mntn_info.ulLayerNum;
    stDiagInfo.pstMntnInfo.ulEventNum =
        g_astCBTInfoTbl[EN_DIAG_CBT_API_EVENT_OK].ulCalledNum - g_ind_src_mntn_info.ulEventNum;
    stDiagInfo.pstMntnInfo.ulLogNum =
        g_astCBTInfoTbl[EN_DIAG_CBT_API_PRINTFV_OK].ulCalledNum - g_ind_src_mntn_info.ulLogNum;
    stDiagInfo.pstMntnInfo.ulAirNum =
        g_astCBTInfoTbl[EN_DIAG_CBT_API_AIR_OK].ulCalledNum - g_ind_src_mntn_info.ulAirNum;
    stDiagInfo.pstMntnInfo.ulTransNum =
        g_astCBTInfoTbl[EN_DIAG_CBT_API_TRANS_OK].ulCalledNum - g_ind_src_mntn_info.ulTransNum;
    stDiagInfo.pstMntnInfo.ulVolteNum =
        g_astCBTInfoTbl[EN_DIAG_CBT_API_VOLTE_OK].ulCalledNum - g_ind_src_mntn_info.ulVolteNum;
    stDiagInfo.pstMntnInfo.ulUserNum =
        g_astCBTInfoTbl[EN_DIAG_CBT_API_USER_OK].ulCalledNum - g_ind_src_mntn_info.ulUserNum;
    stDiagInfo.pstMntnInfo.ulLayerMatchNum =
        g_astCBTInfoTbl[EN_DIAG_API_MSG_LAYER_MATCH].ulCalledNum - g_ind_src_mntn_info.ulLayerMatchNum;

    g_ind_src_mntn_info.ulTraceNum = stDiagInfo.pstMntnInfo.ulTraceNum;
    g_ind_src_mntn_info.ulLayerNum = stDiagInfo.pstMntnInfo.ulLayerNum;
    g_ind_src_mntn_info.ulEventNum = stDiagInfo.pstMntnInfo.ulEventNum;
    g_ind_src_mntn_info.ulLogNum = stDiagInfo.pstMntnInfo.ulLogNum;
    g_ind_src_mntn_info.ulAirNum = stDiagInfo.pstMntnInfo.ulAirNum;
    g_ind_src_mntn_info.ulTransNum = stDiagInfo.pstMntnInfo.ulTransNum;
    g_ind_src_mntn_info.ulVolteNum = stDiagInfo.pstMntnInfo.ulVolteNum;
    g_ind_src_mntn_info.ulUserNum = stDiagInfo.pstMntnInfo.ulUserNum;
    g_ind_src_mntn_info.ulLayerMatchNum = stDiagInfo.pstMntnInfo.ulLayerMatchNum;

    stDiagInfo.pstMntnHead.pData = (void *)(&stDiagInfo.pstMntnInfo);
    ulRet = mdrv_diag_report_trans((diag_trans_ind_s *)&stDiagInfo);
    if (!ulRet) {
        last_slice = current_slice;
        /* 发送成功，清除本地记录 */
        mdrv_diag_reset_mntn_info(DIAGLOG_SRC_MNTN);
    }
    return;
}

VOS_VOID diag_ReportDstMntn(VOS_VOID)
{
    DIAG_DEBUG_DST_LOST_STRU stDiagInfo = {};
    socp_buffer_rw_s stSocpBuff = { NULL };
    diag_mntn_dst_info_s *pstDstInfo = VOS_NULL;
    VOS_UINT32 ulRet, current_slice;
    static VOS_UINT32 last_slice = 0;
    errno_t err;

    pstDstInfo = (diag_mntn_dst_info_s *)mdrv_diag_get_mntn_info(DIAGLOG_DST_MNTN);

    mdrv_socp_mntn_enc_dst_int_info(&(pstDstInfo->socp_trf_times), &(pstDstInfo->socp_thr_ovf_times));
    mdrv_ppm_query_usb_info(&(pstDstInfo->ppm_usb_info), sizeof(mdrv_ppm_usb_debug_info_s));
    err = memcpy_s(&stDiagInfo.pstMntnInfo, sizeof(stDiagInfo.pstMntnInfo), pstDstInfo, sizeof(*pstDstInfo));
    if (err != EOK) {
        diag_error("memcpy fail:%x\n", err);
    }

    stDiagInfo.pstMntnInfo.chan_id = SOCP_CODER_DST_OM_IND;
    err = memcpy_s(stDiagInfo.pstMntnInfo.chan_name, sizeof(stDiagInfo.pstMntnInfo.chan_name), "ind_dst",
        sizeof("ind_dst"));
    if (err != EOK) {
        diag_error("memcpy fail:%x\n", err);
    }

    (void)mdrv_socp_get_read_buffer(SOCP_CODER_DST_OM_IND, &stSocpBuff);
    stDiagInfo.pstMntnInfo.use_size = stSocpBuff.rb_size + stSocpBuff.size;

    current_slice = mdrv_timer_get_normal_timestamp();
    stDiagInfo.pstMntnInfo.delta_time = DIAG_SLICE_DELTA(last_slice, current_slice);

    if (((stDiagInfo.pstMntnInfo.delta_time * 0x3E8) / 0x8000) == 0) { /* 实际场景中不会出现，此判断是避免UBSAN检查报错 */
        stDiagInfo.pstMntnInfo.thrput_phy = 0;
        stDiagInfo.pstMntnInfo.thrput_cb = 0;
    } else {
        stDiagInfo.pstMntnInfo.thrput_phy = ((mdrv_get_thrput_info(EN_DIAG_THRPUT_DATA_CHN_PHY) / 0x400) * 0x3E8) /
            ((stDiagInfo.pstMntnInfo.delta_time * 0x3E8) / 0x8000);
        stDiagInfo.pstMntnInfo.thrput_cb = ((mdrv_get_thrput_info(EN_DIAG_THRPUT_DATA_CHN_CB) / 0x400) * 0x3E8) /
            ((stDiagInfo.pstMntnInfo.delta_time * 0x3E8) / 0x8000);
    }

    stDiagInfo.pstMntnHead.ulModule = MDRV_DIAG_GEN_MODULE_EX(DIAG_MODEM_0, DIAG_MODE_LTE, DIAG_MSG_TYPE_BSP);
    stDiagInfo.pstMntnHead.ulPid = DIAG_AGENT_PID;
    stDiagInfo.pstMntnHead.ulMsgId = DIAG_DEBUG_DST_MNTN_CMDID;
    stDiagInfo.pstMntnHead.ulLength = sizeof(stDiagInfo.pstMntnInfo);

    stDiagInfo.pstMntnHead.pData = (void *)(&stDiagInfo.pstMntnInfo);

    ulRet = mdrv_diag_report_trans((diag_trans_ind_s *)&stDiagInfo);
    if (!ulRet) {
        /* 发送成功，清除本地记录 */
        mdrv_diag_reset_mntn_info(DIAGLOG_DST_MNTN);
        last_slice = current_slice;
        mdrv_clear_socp_encdst_int_info();
        mdrv_ppm_clear_usb_time_info();
    }
    return;
}

VOS_VOID diag_ReportMntn(VOS_VOID)
{
    /* 开机log */
    if (!DIAG_IS_POLOG_ON) {
        /* HIDS未连接 */
        if (!DIAG_IS_CONN_ON) {
            return;
        }
    }

    /* 源端维测信息上报 */
    diag_ReportSrcMntn();

    /* 目的端维测信息上报 */
    diag_ReportDstMntn();

    return;
}

VOS_VOID diag_StopMntnTimer(VOS_VOID)
{
    /* 删除定时器 */
    if (g_DiagDebugCfg == DIAG_CFG_SWT_CLOSE) {
        diag_info("mntn is not active\n");
        return;
    }

    g_DiagDebugCfg = DIAG_CFG_SWT_CLOSE;

    (VOS_VOID)VOS_StopRelTimer(&g_DebugTimer);
}

VOS_VOID diag_StartMntnTimer(VOS_UINT32 ulMntnReportTime)
{
    VOS_UINT32 ulCnfRst;

    if (g_DiagDebugCfg == DIAG_CFG_SWT_OPEN) {
        diag_StopMntnTimer();
    }

    g_DiagDebugCfg = DIAG_CFG_SWT_OPEN;

    ulCnfRst = VOS_StartRelTimer(&g_DebugTimer, MSP_PID_DIAG_APP_AGENT, ulMntnReportTime, DIAG_DEBUG_TIMER_NAME,
        DIAG_DEBUG_TIMER_PARA, VOS_RELTIMER_LOOP, VOS_TIMER_NO_PRECISION);
    if (ulCnfRst != ERR_MSP_SUCCESS) {
        diag_error("start dbug timer fail\n");
    }
}

/*
 * Function Name: diag_UserPlaneCfgProc
 * Description: deal witch user plane msg cfg
 * Input: pstReq 待处理数据
 * Output: None
 * Return: VOS_UINT32
 */
VOS_UINT32 diag_MntnCfgProc(VOS_UINT8 *pstReq)
{
    DIAG_CMD_LOG_CAT_DIAG_MNTN_CNF_STRU stDebugCnf = { 0 };
    DIAG_CMD_LOG_DIAG_MNTN_REQ_STRU *pstDebugReq = NULL;
    msp_diag_frame_info_s *pstDiagHead = NULL;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = { 0 };
    DIAG_MSG_TRANS_STRU *pstInfo = NULL;
    VOS_UINT32 ret;
    VOS_UINT32 ulLen;

    pstDiagHead = (msp_diag_frame_info_s *)(pstReq);

    if (pstDiagHead->ulMsgLen < sizeof(msp_diag_data_head_s) + sizeof(DIAG_CMD_LOG_DIAG_MNTN_REQ_STRU)) {
        diag_error("len error:0x%x\n", pstDiagHead->ulMsgLen);
        ret = ERR_MSP_INALID_LEN_ERROR;
        goto DIAG_ERROR;
    }

    pstDebugReq = (DIAG_CMD_LOG_DIAG_MNTN_REQ_STRU *)(pstReq + DIAG_MESSAGE_DATA_HEADER_LEN);

    if (pstDebugReq->usMntnSwitch == DIAG_CFG_SWT_CLOSE) {
        diag_info("switch to close\n");
        /* 删除定时器 */
        diag_StopMntnTimer();
    } else {
        if (pstDebugReq->usMntnTime == 0) {
            pstDebugReq->usMntnTime = 0x3e8; // 如果工具下发维测周期为0，MSP默认值配置为0x3e8(1000ms)
        }
        diag_info("switch to open, Mntn Cycle = %d ms\n", pstDebugReq->usMntnTime);
        diag_StartMntnTimer(pstDebugReq->usMntnTime);
    }

    DIAG_MSG_ACORE_CFG_PROC(ulLen, pstDiagHead, pstInfo, ret);
    DIAG_MSG_SEND_CFG_TO_NRM(ulLen, pstDiagHead, pstInfo, ret);

    ret = ERR_MSP_SUCCESS;
DIAG_ERROR:
    DIAG_MSG_COMMON_PROC(stDiagInfo, stDebugCnf, pstDiagHead);
    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_MSP;
    stDebugCnf.ulRc = ret;

    /* 组包给FW回复 */
    ret = DIAG_MsgReport(&stDiagInfo, &stDebugCnf, (VOS_UINT32)sizeof(stDebugCnf));
    return (VOS_UINT32)ret;
}

VOS_UINT32 diag_RateInfoCfgProc(VOS_UINT8 *pstReq)
{
    msp_diag_frame_info_s *pstDiagHead = NULL;
    DIAG_MSG_TRANS_STRU *pstInfo = NULL;
    VOS_UINT32 ret = ERR_MSP_SUCCESS;
    VOS_UINT32 ulLen;

    pstDiagHead = (msp_diag_frame_info_s *)(pstReq);

    if (pstDiagHead->ulMsgLen < sizeof(msp_diag_data_head_s) + sizeof(DIAG_CMD_LOG_DIAG_MNTN_REQ_STRU)) {
        diag_error("len error:0x%x\n", pstDiagHead->ulMsgLen);
        return ERR_MSP_INALID_LEN_ERROR;
    }

    DIAG_MSG_ACORE_CFG_PROC(ulLen, pstDiagHead, pstInfo, ret);
DIAG_ERROR:
    return (VOS_UINT32)ret;
}

/* EVENT上报调测接口 */
VOS_VOID DIAG_DebugEventReport(VOS_UINT32 ulpid)
{
    mdrv_diag_event_ind_s stEvent = { 0 };
    char aulData[0x10] = "event";

    stEvent.ulModule = MDRV_DIAG_GEN_MODULE(1, 0x2);
    stEvent.ulPid = ulpid;
    stEvent.ulEventId = 0x13579;
    stEvent.ulLength = sizeof(aulData);
    stEvent.pData = aulData;

    (VOS_VOID)mdrv_diag_event_report(&stEvent);
}

/* 层间消息上报调测接口 */
VOS_VOID DIAG_DebugLayerReport(VOS_UINT32 ulsndpid, VOS_UINT32 ulrcvpid, VOS_UINT32 ulMsg)
{
    DIAG_DATA_MSG_STRU *pDataMsg = NULL;
    char aulData[0x10] = "layer";
    errno_t err;

    diag_crit("pid %d send to %d, msgid 0x%x.\n", ulsndpid, ulsndpid, ulMsg);

    pDataMsg = (DIAG_DATA_MSG_STRU *)VOS_AllocMsg(ulsndpid,
        (sizeof(DIAG_DATA_MSG_STRU) + sizeof(aulData) - VOS_MSG_HEAD_LENGTH));
    if (pDataMsg != NULL) {
        pDataMsg->ulReceiverPid = ulrcvpid;
        pDataMsg->ulMsgId = ulMsg;
        pDataMsg->ulLen = sizeof(aulData);
        err = memcpy_s(pDataMsg->pContext, pDataMsg->ulLen, aulData, sizeof(aulData));
        if (err != EOK) {
            diag_error("memcpy fail:%x\n", err);
        }

        mdrv_diag_trace_report(pDataMsg);

        (VOS_VOID)VOS_FreeMsg(ulsndpid, pDataMsg);
    }
}

/* 层间消息上报调测接口 */
VOS_VOID DIAG_DebugVosLayerReport(VOS_UINT32 ulsndpid, VOS_UINT32 ulrcvpid, VOS_UINT32 ulMsg)
{
    DIAG_DATA_MSG_STRU *pDataMsg = NULL;
    char aulData[0x10] = "vos layer";
    errno_t err;

    diag_crit("pid %d send to %d, msgid 0x%x.\n", ulsndpid, ulsndpid, ulMsg);

    pDataMsg = (DIAG_DATA_MSG_STRU *)VOS_AllocMsg(ulsndpid,
        (sizeof(DIAG_DATA_MSG_STRU) + sizeof(aulData) - VOS_MSG_HEAD_LENGTH));
    if (pDataMsg != NULL) {
        pDataMsg->ulReceiverPid = ulrcvpid;
        pDataMsg->ulMsgId = ulMsg;
        pDataMsg->ulLen = sizeof(aulData);
        err = memcpy_s(pDataMsg->pContext, pDataMsg->ulLen, aulData, sizeof(aulData));
        if (err != EOK) {
            diag_error("memcpy fail:%x\n", err);
        }

        mdrv_diag_layer_msg_report(pDataMsg);

        (VOS_VOID)VOS_FreeMsg(ulsndpid, pDataMsg);
    }
}

/* log上报调测接口 */
VOS_VOID DIAG_DebugLogReport(VOS_UINT32 ulpid, VOS_UINT32 level)
{
    VOS_UINT32 ulMod = MDRV_DIAG_GEN_LOG_MODULE(1, 0x2, level);
    (VOS_VOID)mdrv_diag_log_report(ulMod, ulpid, "diag_debug.c", 0x2A8, "DIAG TEST.\n");
}

/* 透传上报调测接口 */
VOS_VOID DIAG_DebugTransReport(VOS_UINT32 ulpid)
{
    mdrv_diag_trans_ind_s std;
    char aulData[0x50] = "trans";

    std.ulModule = MDRV_DIAG_GEN_MODULE(1, 0x2);
    std.ulPid = ulpid;
    std.ulMsgId = 0x9753;
    std.ulLength = sizeof(aulData);
    std.pData = aulData;

    (VOS_VOID)mdrv_diag_trans_report(&std);
}

/* 层间开关调测接口 */
VOS_VOID DIAG_DebugLayerCfg(VOS_UINT32 ulModuleId, VOS_UINT8 ucFlag)
{
    VOS_UINT32 ulOffset;

    if (DIAG_CFG_LAYER_MODULE_IS_ACORE(ulModuleId)) {
        ulOffset = DIAG_CFG_LAYER_MODULE_ACORE_OFFSET(ulModuleId);

        g_ALayerSrcModuleCfg[ulOffset] = ucFlag;
        g_ALayerDstModuleCfg[ulOffset] = ucFlag;
    } else if (DIAG_CFG_LAYER_MODULE_IS_CCORE(ulModuleId)) {
        ulOffset = DIAG_CFG_LAYER_MODULE_CCORE_OFFSET(ulModuleId);

        g_CLayerSrcModuleCfg[ulOffset] = ucFlag;
        g_CLayerDstModuleCfg[ulOffset] = ucFlag;
    } else {
        diag_crit("Invalid PID: A(%d -- %d), C(%d -- %d)!\n", VOS_PID_CPU_ID_1_DOPRAEND, VOS_CPU_ID_1_PID_BUTT,
            VOS_PID_CPU_ID_0_DOPRAEND, VOS_CPU_ID_0_PID_BUTT);

        return;
    }
}

#define GEN_MODULE_ID(modem_id, modeid, level, pid) (((modem_id) << 24) | ((modeid) << 16) | ((level) << 12) | (pid))

VOS_UINT32 DIAG_ApiTest(VOS_UINT8 *pstReq)
{
    DIAG_MSG_TRANS_STRU *pstInfo = VOS_NULL;
    mdrv_diag_air_ind_s airmsg;
    VOS_UINT32 data = DIAG_TEST_MSG_DATA;
    VOS_UINT32 ulModuleId, ulLen;
    msp_diag_frame_info_s *pstDiagHead = VOS_NULL;
    VOS_VOID *ptr = VOS_NULL;

    ptr = VOS_MemAlloc(MSP_PID_DIAG_APP_AGENT, DYNAMIC_MEM_PT, 0x9 * 0x400);
    if (ptr == VOS_NULL) {
        diag_error("alloc fail\n");
    }

    (VOS_VOID)DIAG_DebugLogReport(MSP_PID_DIAG_APP_AGENT, 1);

    (VOS_VOID)DIAG_DebugTransReport(MSP_PID_DIAG_APP_AGENT);
    (VOS_VOID)mdrv_diag_trans_report(VOS_NULL);

    (VOS_VOID)DIAG_DebugEventReport(MSP_PID_DIAG_APP_AGENT);
    (VOS_VOID)mdrv_diag_event_report(VOS_NULL);

    ulModuleId = GEN_MODULE_ID(0, 1, 1, MSP_PID_DIAG_APP_AGENT);
    airmsg.ulPid = MSP_PID_DIAG_APP_AGENT;
    airmsg.ulDirection = 0x3;
    airmsg.ulLength = sizeof(data);
    airmsg.ulModule = ulModuleId;
    airmsg.ulMsgId = DIAG_TEST_MSG_ID;
    airmsg.pData = (VOS_VOID *)&data;
    (VOS_VOID)mdrv_diag_air_report(&airmsg);

    if (ptr != VOS_NULL) {
        airmsg.ulPid = MSP_PID_DIAG_APP_AGENT;
        airmsg.ulDirection = 0x3;
        airmsg.ulLength = 9 * 1024;
        airmsg.ulModule = ulModuleId;
        airmsg.ulMsgId = DIAG_TEST_MSG_ID;
        airmsg.pData = (VOS_VOID *)&data;
        (VOS_VOID)mdrv_diag_air_report(&airmsg);
    }

    (VOS_VOID)mdrv_diag_air_report(VOS_NULL);

    (VOS_VOID)DIAG_DebugLayerReport(DOPRA_PID_TIMER, MSP_PID_DIAG_APP_AGENT, DIAG_TEST_MSG_ID);
    (VOS_VOID)mdrv_diag_trace_report(VOS_NULL);

    (VOS_VOID)DIAG_DebugVosLayerReport(DOPRA_PID_TIMER, MSP_PID_DIAG_APP_AGENT, DIAG_TEST_MSG_ID);
    (VOS_VOID)mdrv_diag_layer_msg_report(VOS_NULL);

    (VOS_VOID)mdrv_diag_error_log(VOS_NULL, 0, 0, 0, VOS_NULL, 0);

    VOS_MemFree(MSP_PID_DIAG_APP_AGENT, ptr);

    /* 修改后接口也支持在串口调用 */
    if (pstReq != VOS_NULL) {
        pstDiagHead = (msp_diag_frame_info_s *)(pstReq);

        DIAG_MSG_ACORE_CFG_PROC(ulLen, pstDiagHead, pstInfo, data);
        DIAG_MSG_SEND_CFG_TO_NRM(ulLen, pstDiagHead, pstInfo, data);
    }

    return VOS_OK;
DIAG_ERROR:
    return 0;
}

EXPORT_SYMBOL(DIAG_ApiTest);
/* *********************灌包测试 start*********************************** */
VOS_UINT32 g_diag_supress_times = 0x64;
VOS_UINT32 g_diag_supress_size = 0x3 * 0x400 + 0x200;
HTIMER g_ACoreFlowTimer = 0;

VOS_VOID DIAG_ACoreFlowPress(VOS_VOID)
{
    mdrv_diag_air_ind_s airmsg;
    VOS_UINT32 data = DIAG_TEST_FLOW_DATA;
    VOS_UINT32 ulModuleId = GEN_MODULE_ID(0, 1, 1, DIAG_AGENT_PID);
    VOS_UINT32 diag_supress_count = 0;

    while (diag_supress_count < g_diag_supress_times) {
        diag_supress_count++;

        airmsg.ulPid = DIAG_AGENT_PID;
        airmsg.ulDirection = 0x3;
        airmsg.ulLength = g_diag_supress_size;
        airmsg.ulModule = ulModuleId;
        airmsg.ulMsgId = DIAG_TEST_MSG_ID;
        airmsg.pData = (VOS_VOID *)&data;

        (VOS_VOID)mdrv_diag_air_report(&airmsg);
    }
}

VOS_VOID diag_StartFlowTimer(VOS_UINT32 ulFlowTime)
{
    VOS_UINT32 ulCnfRst;
    if (ulFlowTime == 0)
        ulFlowTime = 1;
    ulCnfRst = VOS_StartRelTimer(&g_ACoreFlowTimer, MSP_PID_DIAG_APP_AGENT, ulFlowTime, DIAG_FLOW_ACORE_TIMER_NAME,
        DIAG_ACORE_FLOW_TIMER_PARA, VOS_RELTIMER_LOOP, VOS_TIMER_PRECISION_0);
    if (ulCnfRst != ERR_MSP_SUCCESS) {
        diag_error("start flow timer fail\n");
    }
}

VOS_UINT32 DIAG_FlowTest(VOS_UINT8 *pstReq)
{
    msp_diag_frame_info_s *pstDiagHead = NULL;
    DIAG_DEBUG_FLOW_TEST_STRU *pstDebugReq = NULL;
    DIAG_MSG_TRANS_STRU *pstInfo = NULL;
    VOS_UINT32 data = DIAG_TEST_MSG_DATA;
    VOS_UINT32 ulLen;

    pstDiagHead = (msp_diag_frame_info_s *)(pstReq);

    if (pstDiagHead->ulMsgLen < sizeof(msp_diag_data_head_s) + sizeof(DIAG_DEBUG_FLOW_TEST_STRU)) {
        diag_error("len error:0x%x\n", pstDiagHead->ulMsgLen);
        return ERR_MSP_INALID_LEN_ERROR;
    }

    pstDebugReq = (DIAG_DEBUG_FLOW_TEST_STRU *)(pstReq + DIAG_MESSAGE_DATA_HEADER_LEN);

    if (pstDebugReq->NRCCPU_Is_Enable || pstDebugReq->LRCCPU_Is_Enable) {
        DIAG_MSG_ACORE_CFG_PROC(ulLen, pstDiagHead, pstInfo, data);
        DIAG_MSG_SEND_CFG_TO_NRM(ulLen, pstDiagHead, pstInfo, data);
    }

    if (pstDebugReq->ACPU_Is_Enable != 0) {
        if (pstDebugReq->PackOneCycle != 0)
            g_diag_supress_times = pstDebugReq->PackOneCycle;
        diag_crit("DIAG FLOW_TEST Acore start\n");
        if (g_ACoreFlowTimer != 0)
            (VOS_VOID)VOS_StopRelTimer(&g_ACoreFlowTimer);
        diag_StartFlowTimer(pstDebugReq->ACPU_Inteval);
    } else {
        if (g_ACoreFlowTimer != 0)
            (VOS_VOID)VOS_StopRelTimer(&g_ACoreFlowTimer);
        diag_error("DIAG FLOW_TEST Acore suspend");
    }
DIAG_ERROR:
    return 0;
}
/* *********************灌包测试 end*********************************** */
#ifdef DIAG_SYSTEM_5G
VOS_UINT32 diag_debug_send_conn_msg(void)
{
    msp_diag_frame_info_s *pstDiagHead = VOS_NULL;
    DIAG_MSG_TRANS_STRU *pstInfo = VOS_NULL;
    static VOS_UINT32 ulSnCnt = 0;
    VOS_UINT32 ulRet;
    u8 auctime[0x8];
    VOS_UINT32 ulLen;
    u32 ulTimeStampLen;

    pstDiagHead = (msp_diag_frame_info_s *)VOS_MemAlloc(MSP_PID_DIAG_APP_AGENT, DYNAMIC_MEM_PT,
        sizeof(msp_diag_frame_info_s) + 0x8);
    if (pstDiagHead == VOS_NULL) {
        return ERR_MSP_MALLOC_FAILUE;
    }
    pstDiagHead->stService.sid4b = 0x2;
    pstDiagHead->stService.ver4b = 1;
    pstDiagHead->stService.mdmid3b = 0;
    pstDiagHead->stService.rsv5b = 0;
    pstDiagHead->stService.ssid8b = 1;

    pstDiagHead->stService.mt2b = 1;
    pstDiagHead->stService.index4b = 0;
    pstDiagHead->stService.eof1b = 0;

    /* 默认不分包* */
    pstDiagHead->stService.ff1b = 0;
    pstDiagHead->stService.ulMsgTransId = 0;
    pstDiagHead->stService.ulSocpEncDstId = 0;

    pstDiagHead->ulCmdId = ID_MSG_DIAG_DEBUG_EMU_CONNECT_REQ;
    pstDiagHead->ulMsgLen = 0x8;

    *(pstDiagHead->aucData) = 0; /* auId */
    *(pstDiagHead->aucData + 0x4) = ulSnCnt++;

    ulTimeStampLen = sizeof(pstDiagHead->stService.aucTimeStamp);

    (void)mdrv_timer_get_accuracy_timestamp((u32 *)&(auctime[0x4]), (u32 *)&(auctime[0]));
    ulRet = (VOS_UINT32)memcpy_s(pstDiagHead->stService.aucTimeStamp, sizeof(pstDiagHead->stService.aucTimeStamp),
        auctime, ulTimeStampLen);
    if (ulRet) {
        diag_error("memcpy_s fail,ulRet=0x%x\n", ulRet);
    }

    DIAG_MSG_ACORE_CFG_PROC(ulLen, pstDiagHead, pstInfo, ulRet);
    DIAG_MSG_SEND_CFG_TO_NRM(ulLen, pstDiagHead, pstInfo, ulRet);

    DIAG_NotifyConnState(DIAG_STATE_CONN);

DIAG_ERROR:
    VOS_MemFree(MSP_PID_DIAG_APP_AGENT, pstDiagHead);
    return ulRet;
}

/* EMU平台HIDS打桩连接，需要手动调用该接口 */
VOS_UINT32 mdrv_diag_emu_hids_connect(VOS_VOID)
{
    VOS_UINT32 ulRet;


    diag_crit("DIAG_VERSION_ENG is closed\n");

    return ulRet;
}
EXPORT_SYMBOL(mdrv_diag_emu_hids_connect);

static VOS_UINT32 diag_debug_send_disconn_msg(void)
{
    DIAG_CONN_MSG_SEND_T stConnMsg = { 0 };
    VOS_UINT32 ulRet;
    static VOS_UINT32 ulSnCnt = 0;

    stConnMsg.ulMsgId = ID_MSG_DIAG_CMD_DISCONNECT_REQ;
    stConnMsg.ulSn = ulSnCnt++;

    ulRet = diag_SendConnectMsg((VOS_UINT8 *)&stConnMsg);
    if (ulRet) {
        diag_info("diag_ConnMgrProc: send connect msg fail(0x%x)\n", ulRet);
    }

    DIAG_NotifyConnState(DIAG_STATE_DISCONN);

    return ulRet;
}

/* EMU平台HIDS打桩断开连接，需要手动调用该接口 */
VOS_UINT32 mdrv_diag_emu_hids_disconnect(VOS_VOID)
{
    VOS_UINT32 ulRet;

    ulRet = diag_debug_send_disconn_msg();
    if (ulRet) {
        diag_error("emu hids disconnect send msg fail,ret=0x%x\n", ulRet);
    }

#ifdef DIAG_SEC_TOOLS
    diag_set_auth_state(DIAG_AUTH_TYPE_DEFAULT);
#endif
    diag_CfgResetAllSwt();

    mdrv_socp_set_ind_mode(SOCP_IND_MODE_DIRECT);

    diag_crit("emu hids disconnect success\n");

    return ulRet;
}
EXPORT_SYMBOL(mdrv_diag_emu_hids_disconnect);
#endif

