/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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

#include "taf_type_def.h"
#include "taf_msg_chk_api.h"
#if ((VOS_OSA_CPU == OSA_CPU_ACPU) || (defined(__PC_UT__)))
#include "ads_intra_msg.h"
#include "rnic_intra_msg.h"
#include "rnic_msg_proc.h"
#endif
#include "cds_ads_interface.h"
#include "dsm_rnic_pif.h"
#include "dsm_ads_pif.h"
#include "ads_interface.h"
#include "imsa_ads_interface.h"
#include "at_rnic_interface.h"
#include "rnic_cds_interface.h"
#include "imsa_rnic_interface.h"


#define THIS_FILE_ID PS_FILE_ID_ADS_MSG_CHK_C

#if ((FEATURE_ON == FEATURE_IMS) && (FEATURE_ON == FEATURE_ACPU_PHONE_MODE))
STATIC TAF_ChkMsgLenNameMapTbl* RNIC_GetChkImsaMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 RNIC_GetChkImsaMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* RNIC_GetChkCdsMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 RNIC_GetChkCdsMsgLenTblSize(VOS_VOID);
#endif
STATIC TAF_ChkMsgLenNameMapTbl* RNIC_GetChkDsmMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 RNIC_GetChkDsmMsgLenTblSize(VOS_VOID);
STATIC VOS_UINT32 ADS_UL_ChkCdsAdsDataIndMsgLen(const MSG_Header *msgHeader);
STATIC VOS_UINT32 ADS_UL_ChkCdsAdsDataIndV2MsgLen(const MSG_Header *msgHeader);
STATIC TAF_ChkMsgLenNameMapTbl* ADS_UL_GetChkTafMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 ADS_UL_GetChkTafMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* ADS_UL_GetChkCdsMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 ADS_UL_GetChkCdsMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* ADS_UL_GetChkDsmMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 ADS_UL_GetChkDsmMsgLenTblSize(VOS_VOID);
#if ((FEATURE_IMS == FEATURE_ON) && (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON))
STATIC TAF_ChkMsgLenNameMapTbl* ADS_UL_GetChkImsaMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 ADS_UL_GetChkImsaMsgLenTblSize(VOS_VOID);
#endif
STATIC TAF_ChkMsgLenNameMapTbl* ADS_DL_GetChkTafMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 ADS_DL_GetChkTafMsgLenTblSize(VOS_VOID);
#if ((VOS_OSA_CPU == OSA_CPU_ACPU) || (defined(__PC_UT__)))
STATIC TAF_ChkMsgLenNameMapTbl* ADS_UL_GetChkAdsUlMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 ADS_UL_GetChkAdsUlMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* ADS_DL_GetChkAdsDlMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 ADS_DL_GetChkAdsDlMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* RNIC_GetChkRnicMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 RNIC_GetChkRnicMsgLenTblSize(VOS_VOID);
#endif
STATIC TAF_ChkMsgLenNameMapTbl* RNIC_GetChkAtMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 RNIC_GetChkAtMsgLenTblSize(VOS_VOID);

#if ((FEATURE_ON == FEATURE_IMS) && (FEATURE_ON == FEATURE_ACPU_PHONE_MODE))
static TAF_ChkMsgLenNameMapTbl g_rnicChkImsaMsgLenTbl[] = {
    { ID_IMSA_RNIC_PDN_ACT_IND, sizeof(IMSA_RNIC_PdnActInd), VOS_NULL_PTR },
    { ID_IMSA_RNIC_PDN_DEACT_IND, sizeof(IMSA_RNIC_PdnDeactInd), VOS_NULL_PTR },
    { ID_IMSA_RNIC_PDN_MODIFY_IND, sizeof(IMSA_RNIC_PdnModifyInd ), VOS_NULL_PTR },
    { ID_IMSA_RNIC_RESERVED_PORTS_CONFIG_IND, sizeof(IMSA_RNIC_ReservedPortsConfigInd), VOS_NULL_PTR },
    { ID_IMSA_RNIC_SOCKET_EXCEPTION_IND, sizeof(IMSA_RNIC_SocketExceptionInd), VOS_NULL_PTR },
    { ID_IMSA_RNIC_SIP_PORTS_RANGE_IND, sizeof(IMSA_RNIC_SipPortRangeInd), VOS_NULL_PTR },
};

static TAF_ChkMsgLenNameMapTbl g_rnicChkCdsMsgLenTbl[] = {
    { ID_CDS_RNIC_IMS_DATA_IND, sizeof(CDS_RNIC_ImsDataInd), VOS_NULL_PTR },
};
#endif

static TAF_ChkMsgLenNameMapTbl g_rnicChkDsmMsgLenTbl[] = {
    { ID_DSM_RNIC_IFACE_CFG_IND, sizeof(DSM_RNIC_IfaceCfgInd), VOS_NULL_PTR },
    { ID_DSM_RNIC_IFACE_REL_IND, sizeof(DSM_RNIC_IfaceRelInd), VOS_NULL_PTR },
};

static TAF_ChkMsgLenNameMapTbl g_adsUlChkTafMsgLenTbl[] = {
    { ID_APS_ADS_PDP_STATUS_IND, sizeof(ADS_PdpStatusInd), VOS_NULL_PTR },
};

static TAF_ChkMsgLenNameMapTbl g_adsUlChkCdsMsgLenTbl[] = {
    { ID_CDS_ADS_DATA_IND, 0, ADS_UL_ChkCdsAdsDataIndMsgLen },
    { ID_CDS_ADS_DATA_IND_V2, 0, ADS_UL_ChkCdsAdsDataIndV2MsgLen },
};

static TAF_ChkMsgLenNameMapTbl g_adsUlChkDsmMsgLenTbl[] = {
    { ID_DSM_ADS_IFACE_CONFIG_IND, sizeof(DSM_ADS_IfaceConfigInd), VOS_NULL_PTR },
#if (FEATURE_SHARE_APN == FEATURE_ON)
    { ID_DSM_ADS_ESPE_TRANSFER_CONFIG_IND, sizeof(DSM_ADS_EspeTransferConfigInd), VOS_NULL_PTR },
#endif
};

#if ((FEATURE_IMS == FEATURE_ON) && (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON))
static TAF_ChkMsgLenNameMapTbl g_adsUlChkImsaMsgLenTbl[] = {
    { ID_IMSA_ADS_IFACE_CONFIG_IND, sizeof(IMSA_ADS_IfaceConfigInd), VOS_NULL_PTR },
};
#endif

/* ACPU_PID_ADS_DL接收WUEPS_PID_TAF消息长度检查MAP */
static TAF_ChkMsgLenNameMapTbl g_adsDlChkTafMsgLenTbl[] = {
    { ID_APS_ADS_PDP_STATUS_IND, sizeof(ADS_PdpStatusInd), VOS_NULL_PTR },
};

#if ((VOS_OSA_CPU == OSA_CPU_ACPU) || (defined(__PC_UT__)))
/* ACPU_PID_ADS_DL接收ACPU_PID_ADS_DL消息长度检查MAP */
static TAF_ChkMsgLenNameMapTbl g_adsDlChkAdsDlMsgLenTbl[] = {
    { ID_ADS_CCPU_RESET_START_IND, sizeof(ADS_CcpuResetInd), VOS_NULL_PTR },
    { ID_ADS_CCPU_RESET_END_IND, sizeof(ADS_CcpuResetInd), VOS_NULL_PTR },
    { ID_ADS_PACKET_ERROR_IND, sizeof(ADS_PacketErrorInd), VOS_NULL_PTR }
};

static TAF_ChkMsgLenNameMapTbl g_adsUlChkAdsUlMsgLenTbl[] = {
    { ID_ADS_CCPU_RESET_START_IND, sizeof(ADS_CcpuResetInd), VOS_NULL_PTR },
    { ID_ADS_CCPU_RESET_END_IND, sizeof(ADS_CcpuResetInd), VOS_NULL_PTR },
    { ID_ADS_DPL_CONFIG_IND, sizeof(ADS_DplConfigInd), VOS_NULL_PTR }
};

static TAF_ChkMsgLenNameMapTbl g_rnicChkRnicMsgLenTbl[] = {
    { ID_RNIC_CCPU_RESET_START_IND, sizeof(RNIC_CCPU_RESET_IND_STRU), VOS_NULL_PTR },
    { ID_RNIC_CCPU_RESET_END_IND, 0, VOS_NULL_PTR },
    { ID_RNIC_NETDEV_READY_IND, sizeof(RNIC_DEV_READY_STRU), VOS_NULL_PTR },
#if ((FEATURE_ON == FEATURE_IMS) && (FEATURE_ON == FEATURE_ACPU_PHONE_MODE))
    { ID_RNIC_IMS_DATA_PROC_IND, sizeof(RNIC_IMS_DataProcInd), VOS_NULL_PTR },
#endif
};
#endif

static TAF_ChkMsgLenNameMapTbl g_rnicChkAtMsgLenTbl[] = {
    { ID_AT_RNIC_DIAL_MODE_REQ, sizeof(AT_RNIC_DialModeReq), VOS_NULL_PTR },
    { ID_AT_RNIC_USB_TETHER_INFO_IND, sizeof(AT_RNIC_UsbTetherInfoInd), VOS_NULL_PTR },
    { ID_AT_RNIC_RMNET_CFG_REQ, sizeof(AT_RNIC_RmnetCfgReq), VOS_NULL_PTR }
};

/*
 * ACPU_PID_ADS_DL接收消息长度检查MAP
 * 例外，来自UEPS_PID_CDS的消息处理函数ADS_DL_RcvCdsMsg为空，不检查
 */
static TAF_ChkMsgLenSndPidMapTbl g_adsDlChkMsgLenTbl[] = {
#if ((VOS_OSA_CPU == OSA_CPU_ACPU) || (defined(__PC_UT__)))
    { ACPU_PID_ADS_DL, ADS_DL_GetChkAdsDlMsgLenTblAddr, ADS_DL_GetChkAdsDlMsgLenTblSize },
#endif
    { I0_WUEPS_PID_TAF, ADS_DL_GetChkTafMsgLenTblAddr, ADS_DL_GetChkTafMsgLenTblSize },
    { I1_WUEPS_PID_TAF, ADS_DL_GetChkTafMsgLenTblAddr, ADS_DL_GetChkTafMsgLenTblSize },
    { I2_WUEPS_PID_TAF, ADS_DL_GetChkTafMsgLenTblAddr, ADS_DL_GetChkTafMsgLenTblSize },
};

/* ACPU_FID_ADS_UL下的PID接收消息有效长度检查MAP */
static TAF_ChkMsgLenRcvPidMapTbl g_adsDlFidChkMsgLenTbl[] = {
    TAF_MSG_CHECK_TBL_ITEM(ACPU_PID_ADS_DL, g_adsDlChkMsgLenTbl),
};

/* ACPU_PID_RNIC接收消息长度检查MAP */
static TAF_ChkMsgLenSndPidMapTbl g_acpuRnicChkMsgLenTbl[] = {
    { WUEPS_PID_AT, RNIC_GetChkAtMsgLenTblAddr, RNIC_GetChkAtMsgLenTblSize },
#if ((VOS_OSA_CPU == OSA_CPU_ACPU) || (defined(__PC_UT__)))
    { ACPU_PID_RNIC, RNIC_GetChkRnicMsgLenTblAddr, RNIC_GetChkRnicMsgLenTblSize },
#endif
#if ((FEATURE_ON == FEATURE_IMS) && (FEATURE_ON == FEATURE_ACPU_PHONE_MODE))
    { I0_PS_PID_IMSA, RNIC_GetChkImsaMsgLenTblAddr, RNIC_GetChkImsaMsgLenTblSize },
    { I1_PS_PID_IMSA, RNIC_GetChkImsaMsgLenTblAddr, RNIC_GetChkImsaMsgLenTblSize },
    { UEPS_PID_CDS, RNIC_GetChkCdsMsgLenTblAddr, RNIC_GetChkCdsMsgLenTblSize },
#endif
    { I0_UEPS_PID_DSM, RNIC_GetChkDsmMsgLenTblAddr, RNIC_GetChkDsmMsgLenTblSize },
    { I1_UEPS_PID_DSM, RNIC_GetChkDsmMsgLenTblAddr, RNIC_GetChkDsmMsgLenTblSize },
    { I2_UEPS_PID_DSM, RNIC_GetChkDsmMsgLenTblAddr, RNIC_GetChkDsmMsgLenTblSize },
};

/* ACPU_PID_ADS_UL接收消息长度检查MAP */
static TAF_ChkMsgLenSndPidMapTbl g_adsUlChkMsgLenTbl[] = {
    { I0_WUEPS_PID_TAF, ADS_UL_GetChkTafMsgLenTblAddr, ADS_UL_GetChkTafMsgLenTblSize },
    { I1_WUEPS_PID_TAF, ADS_UL_GetChkTafMsgLenTblAddr, ADS_UL_GetChkTafMsgLenTblSize },
    { I2_WUEPS_PID_TAF, ADS_UL_GetChkTafMsgLenTblAddr, ADS_UL_GetChkTafMsgLenTblSize },
    { UEPS_PID_CDS, ADS_UL_GetChkCdsMsgLenTblAddr, ADS_UL_GetChkCdsMsgLenTblSize },
    { I0_UEPS_PID_DSM, ADS_UL_GetChkDsmMsgLenTblAddr, ADS_UL_GetChkDsmMsgLenTblSize },
    { I1_UEPS_PID_DSM, ADS_UL_GetChkDsmMsgLenTblAddr, ADS_UL_GetChkDsmMsgLenTblSize },
    { I2_UEPS_PID_DSM, ADS_UL_GetChkDsmMsgLenTblAddr, ADS_UL_GetChkDsmMsgLenTblSize },
#if ((FEATURE_IMS == FEATURE_ON) && (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON))
    { I0_PS_PID_IMSA, ADS_UL_GetChkImsaMsgLenTblAddr, ADS_UL_GetChkImsaMsgLenTblSize },
    { I1_PS_PID_IMSA, ADS_UL_GetChkImsaMsgLenTblAddr, ADS_UL_GetChkImsaMsgLenTblSize },
#endif
#if ((VOS_OSA_CPU == OSA_CPU_ACPU) || (defined(__PC_UT__)))
    { ACPU_PID_ADS_UL, ADS_UL_GetChkAdsUlMsgLenTblAddr, ADS_UL_GetChkAdsUlMsgLenTblSize }
#endif
};

/* ACPU_FID_ADS_UL下的PID接收消息有效长度检查MAP */
static TAF_ChkMsgLenRcvPidMapTbl g_acpuAdsUlFidChkMsgLenTbl[] = {
    TAF_MSG_CHECK_TBL_ITEM(ACPU_PID_RNIC, g_acpuRnicChkMsgLenTbl),
    TAF_MSG_CHECK_TBL_ITEM(ACPU_PID_ADS_UL, g_adsUlChkMsgLenTbl),
};

#if ((FEATURE_ON == FEATURE_IMS) && (FEATURE_ON == FEATURE_ACPU_PHONE_MODE))
STATIC TAF_ChkMsgLenNameMapTbl* RNIC_GetChkImsaMsgLenTblAddr(VOS_VOID)
{
    return g_rnicChkImsaMsgLenTbl;
}

STATIC VOS_UINT32 RNIC_GetChkImsaMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_rnicChkImsaMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* RNIC_GetChkCdsMsgLenTblAddr(VOS_VOID)
{
    return g_rnicChkCdsMsgLenTbl;
}

STATIC VOS_UINT32 RNIC_GetChkCdsMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_rnicChkCdsMsgLenTbl);
}
#endif

STATIC TAF_ChkMsgLenNameMapTbl* RNIC_GetChkDsmMsgLenTblAddr(VOS_VOID)
{
    return g_rnicChkDsmMsgLenTbl;
}

STATIC VOS_UINT32 RNIC_GetChkDsmMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_rnicChkDsmMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* ADS_UL_GetChkTafMsgLenTblAddr(VOS_VOID)
{
    return g_adsUlChkTafMsgLenTbl;
}

STATIC VOS_UINT32 ADS_UL_GetChkTafMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_adsUlChkTafMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* ADS_UL_GetChkCdsMsgLenTblAddr(VOS_VOID)
{
    return g_adsUlChkCdsMsgLenTbl;
}

STATIC VOS_UINT32 ADS_UL_GetChkCdsMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_adsUlChkCdsMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* ADS_UL_GetChkDsmMsgLenTblAddr(VOS_VOID)
{
    return g_adsUlChkDsmMsgLenTbl;
}

STATIC VOS_UINT32 ADS_UL_GetChkDsmMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_adsUlChkDsmMsgLenTbl);
}

#if ((FEATURE_IMS == FEATURE_ON) && (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON))
STATIC TAF_ChkMsgLenNameMapTbl* ADS_UL_GetChkImsaMsgLenTblAddr(VOS_VOID)
{
    return g_adsUlChkImsaMsgLenTbl;
}

STATIC VOS_UINT32 ADS_UL_GetChkImsaMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_adsUlChkImsaMsgLenTbl);
}
#endif

VOS_UINT32 ADS_UL_ChkCdsAdsDataIndMsgLen(const MSG_Header *msgHeader)
{
    const CDS_ADS_DataInd *dataInd = VOS_NULL_PTR;
    VOS_UINT32             minMsgLen;

    minMsgLen = sizeof(CDS_ADS_DataInd) - TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msgHeader) + VOS_MSG_HEAD_LENGTH) < minMsgLen) {
        return VOS_FALSE;
    }

    dataInd = (const CDS_ADS_DataInd *)msgHeader;
    minMsgLen = sizeof(CDS_ADS_DataInd) + dataInd->len - sizeof(dataInd->data);
    if ((VOS_GET_MSG_LEN(msgHeader) + VOS_MSG_HEAD_LENGTH) < minMsgLen) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 ADS_UL_ChkCdsAdsDataIndV2MsgLen(const MSG_Header *msgHeader)
{
    CDS_ADS_DataIndV2 *dataInd = VOS_NULL_PTR;
    VOS_UINT32         minMsgLen;

    if ((VOS_GET_MSG_LEN(msgHeader) + VOS_MSG_HEAD_LENGTH) < sizeof(CDS_ADS_DataIndV2)) {
        return VOS_FALSE;
    }

    dataInd = (CDS_ADS_DataIndV2 *)msgHeader;

    minMsgLen = sizeof(CDS_ADS_DataIndV2) + dataInd->len;
    if ((VOS_GET_MSG_LEN(msgHeader) + VOS_MSG_HEAD_LENGTH) < minMsgLen) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

STATIC TAF_ChkMsgLenNameMapTbl* RNIC_GetChkAtMsgLenTblAddr(VOS_VOID)
{
    return g_rnicChkAtMsgLenTbl;
}

STATIC VOS_UINT32 RNIC_GetChkAtMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_rnicChkAtMsgLenTbl);
}

#if ((VOS_OSA_CPU == OSA_CPU_ACPU) || (defined(__PC_UT__)))
STATIC TAF_ChkMsgLenNameMapTbl* RNIC_GetChkRnicMsgLenTblAddr(VOS_VOID)
{
    return g_rnicChkRnicMsgLenTbl;
}

STATIC VOS_UINT32 RNIC_GetChkRnicMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_rnicChkRnicMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* ADS_UL_GetChkAdsUlMsgLenTblAddr(VOS_VOID)
{
    return g_adsUlChkAdsUlMsgLenTbl;
}

STATIC VOS_UINT32 ADS_UL_GetChkAdsUlMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_adsUlChkAdsUlMsgLenTbl);
}
#endif

#if ((VOS_OSA_CPU == OSA_CPU_ACPU) || (defined(__PC_UT__)))
STATIC TAF_ChkMsgLenNameMapTbl* ADS_DL_GetChkAdsDlMsgLenTblAddr(VOS_VOID)
{
    return g_adsDlChkAdsDlMsgLenTbl;
}

STATIC VOS_UINT32 ADS_DL_GetChkAdsDlMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_adsDlChkAdsDlMsgLenTbl);
}
#endif

STATIC TAF_ChkMsgLenNameMapTbl* ADS_DL_GetChkTafMsgLenTblAddr(VOS_VOID)
{
    return g_adsDlChkTafMsgLenTbl;
}

STATIC VOS_UINT32 ADS_DL_GetChkTafMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_adsDlChkTafMsgLenTbl);
}

VOS_VOID TAF_SortAdsUlFidChkMsgLenTbl(VOS_VOID)
{
#if ((VOS_OSA_CPU == OSA_CPU_ACPU) || (defined(__PC_UT__)))
    TAF_SortChkMsgLenTblByMsgName(g_rnicChkRnicMsgLenTbl, TAF_GET_ARRAY_NUM(g_rnicChkRnicMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_adsUlChkAdsUlMsgLenTbl, TAF_GET_ARRAY_NUM(g_adsUlChkAdsUlMsgLenTbl));
#endif
    TAF_SortChkMsgLenTblByMsgName(g_rnicChkAtMsgLenTbl, TAF_GET_ARRAY_NUM(g_rnicChkAtMsgLenTbl));
#if ((FEATURE_ON == FEATURE_IMS) && (FEATURE_ON == FEATURE_ACPU_PHONE_MODE))
    TAF_SortChkMsgLenTblByMsgName(g_rnicChkImsaMsgLenTbl, TAF_GET_ARRAY_NUM(g_rnicChkImsaMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_rnicChkCdsMsgLenTbl, TAF_GET_ARRAY_NUM(g_rnicChkCdsMsgLenTbl));
#endif
    TAF_SortChkMsgLenTblByMsgName(g_rnicChkDsmMsgLenTbl, TAF_GET_ARRAY_NUM(g_rnicChkDsmMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_adsUlChkTafMsgLenTbl, TAF_GET_ARRAY_NUM(g_adsUlChkTafMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_adsUlChkCdsMsgLenTbl, TAF_GET_ARRAY_NUM(g_adsUlChkCdsMsgLenTbl));
    TAF_SortChkMsgLenTblByMsgName(g_adsUlChkDsmMsgLenTbl, TAF_GET_ARRAY_NUM(g_adsUlChkDsmMsgLenTbl));
#if ((FEATURE_IMS == FEATURE_ON) && (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON))
    TAF_SortChkMsgLenTblByMsgName(g_adsUlChkImsaMsgLenTbl, TAF_GET_ARRAY_NUM(g_adsUlChkImsaMsgLenTbl));
#endif
}

VOS_VOID TAF_SortAdsDlFidChkMsgLenTbl(VOS_VOID)
{
#if ((VOS_OSA_CPU == OSA_CPU_ACPU) || (defined(__PC_UT__)))
    TAF_SortChkMsgLenTblByMsgName(g_adsDlChkAdsDlMsgLenTbl, TAF_GET_ARRAY_NUM(g_adsDlChkAdsDlMsgLenTbl));
#endif
    TAF_SortChkMsgLenTblByMsgName(g_adsDlChkTafMsgLenTbl, TAF_GET_ARRAY_NUM(g_adsDlChkTafMsgLenTbl));
}

VOS_UINT32 TAF_ChkAdsDlFidRcvMsgLen(const MsgBlock *msg)
{
    if (VOS_GET_SENDER_ID(msg) == VOS_PID_TIMER) {
        return TAF_ChkTimerMsgLen(msg);
    }

    /* 查找所有到达指定接收PID的长度检查表，不需要做长度检查的消息不记录 */
    return TAF_ChkMsgLen(msg, g_adsDlFidChkMsgLenTbl, TAF_GET_ARRAY_NUM(g_adsDlFidChkMsgLenTbl));
}

VOS_UINT32 TAF_ChkAdsUlFidRcvMsgLen(const MsgBlock *msg)
{
    if (VOS_GET_SENDER_ID(msg) == VOS_PID_TIMER) {
        return TAF_ChkTimerMsgLen(msg);
    }

    /* 查找所有到达指定接收PID的长度检查表，不需要做长度检查的消息不记录 */
    return TAF_ChkMsgLen(msg, g_acpuAdsUlFidChkMsgLenTbl, TAF_GET_ARRAY_NUM(g_acpuAdsUlFidChkMsgLenTbl));
}


