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

#include "mdrv.h"
#include "ps_tag.h"
#include "RnicCtx.h"
#include "RnicEntity.h"
#include "PsCommonDef.h"
#include "RnicMsgProc.h"
#include "RnicIfaceCfg.h"
#include "RnicIfaceOndemand.h"
#include "RnicLog.h"
#include "RnicMntn.h"
#include "rnic_ondemand_i.h"
#include "rnic_dev_i.h"
#include "AcpuReset.h"
#include "AtRnicInterface.h"



/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_RNIC_MAIN_C


/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/* RNIC CTX,用于保存RNIC的全局变量 */
RNIC_CTX_STRU                           g_stRnicCtx;


VOS_INT RNIC_CCpuResetCallback(
    DRV_RESET_CB_MOMENT_E               enParam,
    VOS_INT                             iUserData
)
{
    RNIC_CCPU_RESET_IND_STRU           *pstMsg = VOS_NULL_PTR;

    /* 复位前 */
    if (MDRV_RESET_CB_BEFORE == enParam)
    {
        RNIC_PR_LOGI("before reset enter, %u", VOS_GetSlice());

        /* 构造消息 */
        pstMsg = (RNIC_CCPU_RESET_IND_STRU*)PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_RNIC,
                                                                    sizeof(RNIC_CCPU_RESET_IND_STRU));
        if (VOS_NULL_PTR == pstMsg)
        {
            RNIC_PR_LOGI("before reset alloc msg fail, %u", VOS_GetSlice());
            return VOS_ERROR;
        }

        /* 填写消息头 */
        pstMsg->ulReceiverPid               = ACPU_PID_RNIC;
        pstMsg->enMsgId                     = ID_RNIC_CCPU_RESET_START_IND;

        /* 发消息 */
        if (VOS_OK != PS_SEND_MSG(ACPU_PID_RNIC, pstMsg))
        {
            RNIC_PR_LOGI("before reset send msg fail, %u", VOS_GetSlice());
            return VOS_ERROR;
        }

        /* 等待回复信号量初始为锁状态，等待消息处理完后信号量解锁。 */
        if (VOS_OK != VOS_SmP(RNIC_GET_RESET_SEM(), RNIC_RESET_TIMEOUT_LEN))
        {
            RNIC_PR_LOGI("before reset Vos_SmP fail, %u", VOS_GetSlice());

            return VOS_ERROR;
        }

        return VOS_OK;
    }
    /* 复位后 */
    else if (MDRV_RESET_CB_AFTER == enParam)
    {
        RNIC_PR_LOGI("After reset , %u", VOS_GetSlice());

        return VOS_OK;
    }
    else
    {
        return VOS_ERROR;
    }
}


VOS_VOID RNIC_InitCtx(VOS_VOID)
{
    RNIC_CTX_STRU                      *pstRnicCtx;
    TAF_NV_RNIC_NAPI_CFG_STRU           stNapiCfg;
    TAF_NV_RNIC_NAPI_LB_CFG_STRU        stNapiLbCfg;
    VOS_UINT32                          ulIndex;

    TAF_MEM_SET_S(&g_stRnicCtx, sizeof(RNIC_CTX_STRU), 0x00, sizeof(RNIC_CTX_STRU));
    TAF_MEM_SET_S(&stNapiCfg, sizeof(TAF_NV_RNIC_NAPI_CFG_STRU), 0x00, sizeof(TAF_NV_RNIC_NAPI_CFG_STRU));
    TAF_MEM_SET_S(&stNapiLbCfg, sizeof(TAF_NV_RNIC_NAPI_LB_CFG_STRU), 0x00, sizeof(TAF_NV_RNIC_NAPI_LB_CFG_STRU));

    pstRnicCtx = RNIC_GET_RNIC_CTX_ADR();

    /* 读取NV中的NAPI配置 */
    RNIC_ReadNapiCfg(&stNapiCfg, &stNapiLbCfg);

    for (ulIndex = 0 ; ulIndex < RNIC_NET_ID_MAX_NUM ; ulIndex++)
    {
        pstRnicCtx->astIfaceCtx[ulIndex].enRmNetId = (VOS_UINT8)ulIndex;

        /* 初始化RNIC PDN信息 */
        RNIC_InitPdnInfo(&(pstRnicCtx->astIfaceCtx[ulIndex].stPsIfaceInfo));

        /* 初始化设备NAPI配置 */
        RNIC_InitNapiCfg((VOS_UINT8)ulIndex, &stNapiCfg, &stNapiLbCfg);

    }

    /* 初始化RNIC定时器上下文 */
    RNIC_InitAllTimers();

#if (FEATURE_OFF == FEATURE_DELAY_MODEM_INIT)
    /* 初始化拨号模式信息 */
    RNIC_InitOnDemandDialInfo();
#endif

    /* 初始化复位信号量 */
    RNIC_InitResetSem();

    /* 初始化Tethering信息 */
    RNIC_InitTetherInfo();

    return;
}


VOS_VOID RNIC_InitPdnInfo(RNIC_PS_IFACE_INFO_STRU *pstPdnInfo)
{
    RNIC_InitIpv4PdnInfo(pstPdnInfo);
    RNIC_InitIpv6PdnInfo(pstPdnInfo);

    pstPdnInfo->enModemId = MODEM_ID_BUTT;
    pstPdnInfo->enRatType = RNIC_PS_RAT_TYPE_BUTT;

    IMM_ZcQueueHeadInit(&pstPdnInfo->stImsQue);

    return;
}


VOS_VOID RNIC_InitNapiCfg(
    RNIC_DEV_ID_ENUM_UINT8              enRmNetId,
    TAF_NV_RNIC_NAPI_CFG_STRU          *pstNapiCfg,
    TAF_NV_RNIC_NAPI_LB_CFG_STRU       *pstNapiLbCfg
)
{
#if (FEATURE_ON == FEATURE_RNIC_NAPI_GRO)
    /* 3GPP域7张网卡按照NV配置，其他网卡关闭NAPI */
    if (enRmNetId <= RNIC_3GPP_NET_ID_MAX_NUM)
    {
        RNIC_SET_NAPI_FEATURE(enRmNetId, pstNapiCfg->ucNapiEnable);
        RNIC_SET_GRO_FEATURE(enRmNetId, pstNapiCfg->ucGroEnable);
        RNIC_SET_NAPI_WEIGHT_ADJ_MODE(enRmNetId, pstNapiCfg->enNapiWeightAdjMode);
        RNIC_SET_NAPI_WEIGHT(enRmNetId, pstNapiCfg->ucNapiPollWeight);
        RNIC_SET_NAPI_POLL_QUE_MAX_LEN(enRmNetId, pstNapiCfg->usNapiPollQueMaxLen);

        RNIC_SET_DL_PKT_NUM_PER_SEC_LEVEL1(enRmNetId, pstNapiCfg->stNapiWeightDynamicAdjCfg.stDlPktNumPerSecLevel.ulDlPktNumPerSecLevel1);
        RNIC_SET_DL_PKT_NUM_PER_SEC_LEVEL2(enRmNetId, pstNapiCfg->stNapiWeightDynamicAdjCfg.stDlPktNumPerSecLevel.ulDlPktNumPerSecLevel2);
        RNIC_SET_DL_PKT_NUM_PER_SEC_LEVEL3(enRmNetId, pstNapiCfg->stNapiWeightDynamicAdjCfg.stDlPktNumPerSecLevel.ulDlPktNumPerSecLevel3);
        RNIC_SET_DL_PKT_NUM_PER_SEC_LEVEL4(enRmNetId, pstNapiCfg->stNapiWeightDynamicAdjCfg.stDlPktNumPerSecLevel.ulDlPktNumPerSecLevel4);

        RNIC_SET_NAPI_WEIGHT_LEVEL1(enRmNetId, pstNapiCfg->stNapiWeightDynamicAdjCfg.stNapiWeightLevel.ucNapiWeightLevel1);
        RNIC_SET_NAPI_WEIGHT_LEVEL2(enRmNetId, pstNapiCfg->stNapiWeightDynamicAdjCfg.stNapiWeightLevel.ucNapiWeightLevel2);
        RNIC_SET_NAPI_WEIGHT_LEVEL3(enRmNetId, pstNapiCfg->stNapiWeightDynamicAdjCfg.stNapiWeightLevel.ucNapiWeightLevel3);
        RNIC_SET_NAPI_WEIGHT_LEVEL4(enRmNetId, pstNapiCfg->stNapiWeightDynamicAdjCfg.stNapiWeightLevel.ucNapiWeightLevel4);

        /* NAPI负载均衡配置, 如果NAPI特性关闭，则直接关闭NAPI负载均衡 */
        if (RNIC_FEATURE_OFF == RNIC_GET_NAPI_FEATURE(enRmNetId))
        {
            RNIC_SET_NAPI_LB_FEATURE(enRmNetId, RNIC_FEATURE_OFF);
        }
        else
        {
            RNIC_SET_NAPI_LB_FEATURE(enRmNetId, pstNapiLbCfg->ucNapiLbEnable);
            RNIC_SET_NAPI_LB_CPUMASK(enRmNetId, pstNapiLbCfg->usNapiLbCpumask);
            /* 默认为档位0 */
            RNIC_SET_NAPI_LB_CUR_LEVEL(enRmNetId, 0);

            TAF_MEM_CPY_S(g_stRnicCtx.astIfaceCtx[enRmNetId].stFeatureCfg.stNapiLbCfg.astNapiLbLevelCfg,
                       sizeof(RNIC_NAPI_LB_LEVEL_CFG_STRU) * RNIC_NVIM_NAPI_LB_MAX_LEVEL,
                       pstNapiLbCfg->astNapiLbLevelCfg,
                       sizeof(RNIC_NAPI_LB_LEVEL_CFG_STRU) * RNIC_NVIM_NAPI_LB_MAX_LEVEL);
        }

    }
    else
    {
        RNIC_SET_NAPI_FEATURE(enRmNetId, RNIC_FEATURE_OFF);
        RNIC_SET_GRO_FEATURE(enRmNetId, RNIC_FEATURE_OFF);
        RNIC_SET_NAPI_LB_FEATURE(enRmNetId, RNIC_FEATURE_OFF);
    }
#else
    RNIC_SET_NAPI_FEATURE(enRmNetId, RNIC_FEATURE_OFF);
    RNIC_SET_GRO_FEATURE(enRmNetId, RNIC_FEATURE_OFF);
    RNIC_SET_NAPI_LB_FEATURE(enRmNetId, RNIC_FEATURE_OFF);
#endif

    return;
}


VOS_VOID RNIC_InitOnDemandDialInfo(VOS_VOID)
{
    RNIC_SET_PROC_DIAL_MODE(RNIC_DIAL_MODE_MANUAL);
    RNIC_SET_PROC_IDLE_TIME(RNIC_DIAL_DEMA_IDLE_TIME);
    RNIC_SET_PROC_EVENT_REPORT(RNIC_FORBID_EVENT_REPORT);

    wakeup_source_init(&g_stRnicCtx.stOnDemandDisconnWakeLock, "rnic_ondemand_dis_wake");

    return;
}


VOS_VOID RNIC_InitIpv4PdnInfo(RNIC_PS_IFACE_INFO_STRU *pstPdnInfo)
{
    pstPdnInfo->bitOpIpv4Act  = RNIC_BIT_OPT_FALSE;
    pstPdnInfo->ucIpv4RabId   = RNIC_RAB_ID_INVALID;
    pstPdnInfo->ucIpv4ExRabId = RNIC_RAB_ID_INVALID;
    pstPdnInfo->ulIpv4Addr    = 0;

    return;
}


VOS_VOID RNIC_InitIpv6PdnInfo(RNIC_PS_IFACE_INFO_STRU *pstPdnInfo)
{
    pstPdnInfo->bitOpIpv6Act  = RNIC_BIT_OPT_FALSE;
    pstPdnInfo->ucIpv6RabId   = RNIC_RAB_ID_INVALID;
    pstPdnInfo->ucIpv6ExRabId = RNIC_RAB_ID_INVALID;
    TAF_MEM_SET_S(pstPdnInfo->aucIpv6Addr,
               sizeof(pstPdnInfo->aucIpv6Addr),
               0x00,
               RNIC_MAX_IPV6_ADDR_LEN);

    return;
}


VOS_VOID RNIC_InitResetSem(VOS_VOID)
{
    g_stRnicCtx.hResetSem  = VOS_NULL_PTR;

    /* 分配二进制信号量 */
    if (VOS_OK != VOS_SmBCreate( "RNIC", 0, VOS_SEMA4_FIFO, &g_stRnicCtx.hResetSem))
    {
        PS_PRINTF_WARNING("Create rnic acpu cnf sem failed!\n");

        return;
    }

    PS_PRINTF_INFO("Create rnic acpu cnf sem success!\n");

    return;
}


VOS_VOID RNIC_CheckNapiCfgValid(
    TAF_NV_RNIC_NAPI_CFG_STRU          *pstNapiCfg,
    TAF_NV_RNIC_NAPI_LB_CFG_STRU       *pstNapiLbCfg
)
{
    if (pstNapiCfg->ucNapiEnable >= RNIC_FEATURE_BUTT)
    {
        pstNapiCfg->ucNapiEnable = RNIC_FEATURE_OFF;
    }

    if (pstNapiCfg->ucGroEnable >= RNIC_FEATURE_BUTT)
    {
        pstNapiCfg->ucGroEnable = RNIC_FEATURE_OFF;
    }

    if ((pstNapiCfg->ucNapiPollWeight > RNIC_NAPI_POLL_MAX_WEIGHT) ||
        (0 == pstNapiCfg->ucNapiPollWeight))
    {
        pstNapiCfg->ucNapiPollWeight   = RNIC_NAPI_POLL_DEFAULT_WEIGHT;
    }

    if ((pstNapiCfg->usNapiPollQueMaxLen > RNIC_POLL_QUEUE_DEFAULT_MAX_LEN) ||
        (0 == pstNapiCfg->usNapiPollQueMaxLen))
    {
        pstNapiCfg->usNapiPollQueMaxLen   = RNIC_POLL_QUEUE_DEFAULT_MAX_LEN;
    }

    if (pstNapiCfg->enNapiWeightAdjMode >= NAPI_WEIGHT_ADJ_MODE_BUTT)
    {
        pstNapiCfg->enNapiWeightAdjMode = NAPI_WEIGHT_ADJ_STATIC_MODE;
    }

    if (pstNapiLbCfg->ucNapiLbEnable >= RNIC_FEATURE_BUTT)
    {
        pstNapiLbCfg->ucNapiLbEnable = RNIC_FEATURE_OFF;
    }

    return;
}


VOS_VOID RNIC_ReadNapiCfg(
    TAF_NV_RNIC_NAPI_CFG_STRU          *pstNapiCfg,
    TAF_NV_RNIC_NAPI_LB_CFG_STRU       *pstNapiLbCfg
)
{
#if (FEATURE_OFF == FEATURE_DATA_SERVICE_NEW_PLATFORM)
    TAF_NV_ADS_IPF_MODE_CFG_STRU        stIpfMode;

    TAF_MEM_SET_S(&stIpfMode, (VOS_SIZE_T)sizeof(stIpfMode), 0x00, (VOS_SIZE_T)sizeof(TAF_NV_ADS_IPF_MODE_CFG_STRU));
#endif

#if (FEATURE_ON == FEATURE_RNIC_NAPI_GRO)
    if (NV_OK != TAF_ACORE_NV_READ(MODEM_ID_0,
                                   en_NV_Item_Rnic_Napi_Cfg,
                                   pstNapiCfg,
                                   (VOS_UINT32)sizeof(TAF_NV_RNIC_NAPI_CFG_STRU)))
    {
        pstNapiCfg->ucNapiEnable        = RNIC_FEATURE_OFF;
        pstNapiCfg->enNapiWeightAdjMode = NAPI_WEIGHT_ADJ_STATIC_MODE;
        pstNapiCfg->ucNapiPollWeight    = RNIC_NAPI_POLL_DEFAULT_WEIGHT;
        pstNapiCfg->usNapiPollQueMaxLen = RNIC_POLL_QUEUE_DEFAULT_MAX_LEN;
        pstNapiCfg->ucGroEnable         = RNIC_FEATURE_OFF;
    }

    if (NV_OK != TAF_ACORE_NV_READ(MODEM_ID_0,
                                   en_NV_Item_Rnic_Napi_Lb_Cfg,
                                   pstNapiLbCfg,
                                   (VOS_UINT32)sizeof(TAF_NV_RNIC_NAPI_LB_CFG_STRU)))
    {
        pstNapiLbCfg->ucNapiLbEnable = RNIC_FEATURE_OFF;
    }
#endif

#if (FEATURE_OFF == FEATURE_DATA_SERVICE_NEW_PLATFORM)
    if (NV_OK == TAF_ACORE_NV_READ(MODEM_ID_0,
                                   en_NV_Item_ADS_IPF_MODE_CFG,
                                   &stIpfMode,
                                   (VOS_UINT32)sizeof(TAF_NV_ADS_IPF_MODE_CFG_STRU)))
    {
        /* 如果是进程上下文, 关闭NAPI和GRO */
        if (1 == stIpfMode.ucIpfMode)
        {
            pstNapiCfg->ucNapiEnable = RNIC_FEATURE_OFF;
            pstNapiCfg->ucGroEnable  = RNIC_FEATURE_OFF;
        }
    }
#endif

    RNIC_CheckNapiCfgValid(pstNapiCfg, pstNapiLbCfg);
    return;
}



VOS_VOID RNIC_InitTetherInfo(VOS_VOID)
{
    TAF_MEM_SET_S(g_stRnicCtx.stTetherInfo.aucRmnetName, RNIC_RMNET_NAME_MAX_LEN, 0x00, RNIC_RMNET_NAME_MAX_LEN);

    g_stRnicCtx.stTetherInfo.enTetherConnStat = AT_RNIC_USB_TETHER_DISCONNECT;
    g_stRnicCtx.stTetherInfo.ucOrigGroEnable  = RNIC_FEATURE_OFF;

    return;
}



VOS_UINT32 RNIC_PidInit (enum VOS_INIT_PHASE_DEFINE enPhase)
{
    switch(enPhase)
    {
        case VOS_IP_LOAD_CONFIG:

#if (FEATURE_OFF == FEATURE_DELAY_MODEM_INIT)
            /* 创建网卡 */
            rnic_create_netdev();

            /* 创建按需拨号相关proc节点 */
            rnic_create_ondemand_procfs();
#endif
            /* 初始化RNIC上下文信息 */
            RNIC_InitCtx();

#if (FEATURE_OFF == FEATURE_DELAY_MODEM_INIT)
            /* 注册按需拨号节点操作回调 */
            RNIC_IFACE_RegOnDemandOpsCB();
#endif
            /* 注册设备Ready notify回调 */
            RNIC_IFACE_RegDevNotifyCB();

           /* 注册回调函数，用于C核单独复位的处理 */
            mdrv_sysboot_register_reset_notify(NAS_RNIC_FUNC_PROC_NAME,
                                     RNIC_CCpuResetCallback,
                                     0,
                                     ACPU_RESET_PRIORITY_RNIC);
            break;

        case VOS_IP_FARMALLOC:
        case VOS_IP_INITIAL:
        case VOS_IP_ENROLLMENT:
        case VOS_IP_LOAD_DATA:
        case VOS_IP_FETCH_DATA:
        case VOS_IP_STARTUP:
        case VOS_IP_RIVAL:
        case VOS_IP_KICKOFF:
        case VOS_IP_STANDBY:
        case VOS_IP_BROADCAST_STATE:
        case VOS_IP_RESTART:
            break;
        default:
            break;
    }

    return VOS_OK;
}


