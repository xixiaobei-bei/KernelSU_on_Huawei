/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Create: 2012/10/30
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 and
 *  only version 2 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1) Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2) Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3) Neither the name of Huawei nor the names of its contributors may
 *     be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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

#include "ndis_at_proc.h"
#include "vos.h"
#include "securec.h"
#include "ads_dev_i.h"
#include "at_ndis_interface.h"
#include "ip_comm.h"
#include "ip_nd_server.h"
#include "ndis_interface.h"
#include "ipv4_dhcp_server.h"
#include "ndserver_at_proc.h"
#include "ndis_drv.h"
#include "ndis_om.h"
#include "ndis_entity.h"
#include "ndis_msg_proc.h"
#include "ndis_arp_proc.h"
#include "ndis_dl_proc.h"


#define THIS_MODU ps_ndis
/*lint -e767*/
#define THIS_FILE_ID PS_FILE_ID_NDISARPROC_C
/*lint +e767*/

/*
 * 2 Declare the Global Variable
 */
const VOS_UINT8 g_invalidAddr[IPV4_ADDR_LEN] = {0};

/*
 * 功能描述: IPV4 PDN信息配置
 * 修改历史:
 *  1.日    期: 2011年3月15日
 *    修改内容: 新生成函数
 */
STATIC VOS_VOID Ndis_Ipv4PdnInfoCfg(const AT_NDIS_Ipv4PdnInfo *nasNdisInfo, NDIS_Entity *ndisEntity)
{
    NDIS_Ipv4Info         *ipV4Info    = &(ndisEntity->ipV4Info);
    errno_t                rlt;

    ipV4Info->arpInitFlg = PS_FALSE;
    ipV4Info->arpRepFlg  = PS_FALSE;

    rlt = memcpy_s(ipV4Info->ueIpInfo.ipAddr8bit, IPV4_ADDR_LEN, nasNdisInfo->PdnAddrInfo.ipv4AddrU8,
                   IPV4_ADDR_LEN);
    NDIS_CHK_SEC_RETURN_NO_VAL((rlt != EOK));

    rlt = memcpy_s(ipV4Info->gwIpInfo.ipAddr8bit, IPV4_ADDR_LEN, nasNdisInfo->gateWayAddrInfo.ipv4AddrU8,
                   IPV4_ADDR_LEN);
    NDIS_CHK_SEC_RETURN_NO_VAL((rlt != EOK));

    rlt = memcpy_s(ipV4Info->nmIpInfo.ipAddr8bit, IPV4_ADDR_LEN, nasNdisInfo->subnetMask.ipv4AddrU8,
                   IPV4_ADDR_LEN);
    NDIS_CHK_SEC_RETURN_NO_VAL((rlt != EOK));

    /* 使能则配置DNS */
    if (nasNdisInfo->opDnsPrim == PS_TRUE) {
        rlt = memcpy_s(ipV4Info->primDnsAddr.ipAddr8bit, IPV4_ADDR_LEN,
                       nasNdisInfo->dnsPrimAddrInfo.ipv4AddrU8, IPV4_ADDR_LEN);
        NDIS_CHK_SEC_RETURN_NO_VAL((rlt != EOK));
    } else {
        ipV4Info->primDnsAddr.ipAddr32bit = 0;
    }

    /* 使能则配置辅DNS */
    if (nasNdisInfo->opDnsSec == PS_TRUE) {
        rlt = memcpy_s(ipV4Info->secDnsAddr.ipAddr8bit, IPV4_ADDR_LEN,
                       nasNdisInfo->dnsSecAddrInfo.ipv4AddrU8, IPV4_ADDR_LEN);
        NDIS_CHK_SEC_RETURN_NO_VAL((rlt != EOK));
    } else {
        ipV4Info->secDnsAddr.ipAddr32bit = 0;
    }

    /* 使能则配置主WINS */
    if (nasNdisInfo->opWinsPrim == PS_TRUE) {
        rlt = memcpy_s(ipV4Info->primWinsAddr.ipAddr8bit, IPV4_ADDR_LEN,
                       nasNdisInfo->winsPrimAddrInfo.ipv4AddrU8, IPV4_ADDR_LEN);
        NDIS_CHK_SEC_RETURN_NO_VAL((rlt != EOK));
    } else {
        ipV4Info->primWinsAddr.ipAddr32bit = 0;
    }

    /* 使能则配置辅WINS */
    if (nasNdisInfo->opWinsSec == PS_TRUE) {
        rlt = memcpy_s(ipV4Info->secWinsAddr.ipAddr8bit, IPV4_ADDR_LEN,
                       nasNdisInfo->winsSecAddrInfo.ipv4AddrU8, IPV4_ADDR_LEN);
        NDIS_CHK_SEC_RETURN_NO_VAL((rlt != EOK));
    } else {
        ipV4Info->secWinsAddr.ipAddr32bit = 0;
    }

    /* PCSCF暂时不操作,待需求描述 */
    ipV4Info->ipAssignStatus = IPV4_DHCP_ADDR_STATUS_FREE;

    return;
}

/*
 * 功能描述: PDN IPV4地址信息检查
 * 修改历史:
 *  1.日    期: 2011年12月11日
 *    修改内容: 新生成函数
 */
STATIC VOS_UINT32 Ndis_CheckIpv4PdnInfo(const AT_NDIS_Ipv4PdnInfo *ipv4PdnInfo)
{
    if (ipv4PdnInfo->opPdnAddr == PS_FALSE) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_CheckIpv4PdnInfo,  pstIpv4PdnInfo->bitOpPdnAddr is false!");
        return PS_FAIL;
    }

    /* PDN地址和网关地址如果为全0，则也失败 */
    if (VOS_MemCmp(ipv4PdnInfo->PdnAddrInfo.ipv4AddrU8, g_invalidAddr, IPV4_ADDR_LEN) == 0) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_CheckIpv4PdnInfo,  stPDNAddrInfo all zero!");
        return PS_FAIL;
    }

    if (VOS_MemCmp(ipv4PdnInfo->gateWayAddrInfo.ipv4AddrU8, g_invalidAddr, IPV4_ADDR_LEN) == 0) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_CheckIpv4PdnInfo,  stGateWayAddrInfo all zero!");
        return PS_FAIL;
    }

    return PS_SUCC;
}

/*
 * 功能描述: 检查参数配置参数是否合法
 * 修改历史:
 *  1.日    期: 2011年12月23日
 *    修改内容: 新生成函数
 */
STATIC VOS_UINT32 Ndis_PdnV4PdnCfg(const AT_NDIS_PdnInfoCfgReq *nasNdisInfo, NDIS_Entity *ndisEntity)
{
    /* IPV4地址检查 */
    if (nasNdisInfo->opIpv4PdnInfo == PS_FALSE) { /* 原语指示IPV4信息无效 */
        NDIS_INFO_LOG(NDIS_TASK_PID, "Ndis_PdnV4PdnCfg,  bitOpIpv4PdnInfo is false!");
        return PS_FAIL;
    }

    if (Ndis_CheckIpv4PdnInfo(&(nasNdisInfo->ipv4PdnInfo)) != PS_SUCC) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_PdnV4PdnCfg,  Ndis_CheckIpv4PdnInfo fail!");
        return PS_FAIL;
    }

    Ndis_Ipv4PdnInfoCfg(&(nasNdisInfo->ipv4PdnInfo), ndisEntity);

    /* 更新NDIS实体承载属性 */
    ndisEntity->rabType |= NDIS_ENTITY_IPV4;

    return PS_SUCC;
}

/*
 * 功能描述: IPV6 PDN信息配置
 * 修改历史:
 *  1.日    期: 2011年12月23日
 *    修改内容: 新生成函数
 */
STATIC VOS_UINT32 Ndis_PdnV6PdnCfg(const AT_NDIS_PdnInfoCfgReq *nasNdisInfo, NDIS_Entity *ndisEntity)
{
    VOS_UINT8 exRabId;

    /* IPV6地址检查 */
    if (nasNdisInfo->opIpv6PdnInfo == PS_FALSE) { /* 原语指示IPV6信息无效 */
        NDIS_INFO_LOG(NDIS_TASK_PID, "Ndis_PdnV6PdnCfg,  bitOpIpv6PdnInfo is false!");
        return PS_FAIL;
    }

    exRabId = NDIS_FORM_EXBID(nasNdisInfo->modemId, nasNdisInfo->rabId);

    if (NdSer_CheckIpv6PdnInfo(&(nasNdisInfo->ipv6PdnInfo)) != PS_SUCC) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_PdnV6PdnCfg,  NdSer_CheckIpv6PdnInfo fail!");
        return PS_FAIL;
    }

    /* 调ND SERVER API  配置IPV6地址信息给ND SERVER */
    NdSer_Ipv6PdnInfoCfg(exRabId, &(nasNdisInfo->ipv6PdnInfo));

    /* 更新NDIS实体属性 */
    ndisEntity->rabType |= NDIS_ENTITY_IPV6;

    return PS_SUCC;
}

/*
 * 功能描述: ETHERNET PDN信息配置
 */
STATIC VOS_UINT32 Ndis_PdnEthPdnCfg(const AT_NDIS_PdnInfoCfgReq *nasNdisInfo, NDIS_Entity *ndisEntity)
{
    if (nasNdisInfo->opEthPdnInfo == PS_FALSE) { /* 原语指示ETH信息无效 */
        NDIS_INFO_LOG(NDIS_TASK_PID, "Ndis_PdnEthPdnCfg,  bitOpEthPdnInfo is false!");
        return PS_FAIL;
    }

    /* 更新NDIS实体承载属性 */
    ndisEntity->rabType |= NDIS_ENTITY_ETH;

    return PS_SUCC;
}

/*
 * 功能描述: NDIS向AT返回的配置确认结果处理
 * 修改历史:
 *  1.日    期: 2012年4月25日
 *    修改内容: 新生成函数
 */
STATIC VOS_UINT8 Ndis_AtCnfResultProc(const AT_NDIS_PdnInfoCfgReq *nasNdisInfo, VOS_UINT32 v4Ret,
                                      VOS_UINT32 v6Ret)
{
    VOS_UINT8 result;

    /* 根据配置结果向AT返回配置CNF原语 */
    if ((nasNdisInfo->opIpv4PdnInfo == PS_TRUE) && (nasNdisInfo->opIpv6PdnInfo == PS_TRUE)) {
        if ((v4Ret == PS_SUCC) && (v6Ret == PS_SUCC)) { /* IPV4和IPV6配置都成功 */
            result = AT_NDIS_PDNCFG_CNF_SUCC;
        } else if (v4Ret == PS_SUCC) { /* 只有IPV4配置成功 */
            result = AT_NDIS_PDNCFG_CNF_IPV4ONLY_SUCC;
        } else { /* 只有IPV6配置成功 */
            result = AT_NDIS_PDNCFG_CNF_IPV6ONLY_SUCC;
        }
    } else if (nasNdisInfo->opIpv4PdnInfo == PS_TRUE) { /* 只配置了IPV4 */
        result = AT_NDIS_PDNCFG_CNF_IPV4ONLY_SUCC;
    } else { /* 只配置了IPV6 */
        result = AT_NDIS_PDNCFG_CNF_IPV6ONLY_SUCC;
    }

    return result;
}
STATIC VOS_UINT32 Ndis_PdnInfoCfgSpeProc(NDIS_Entity *ndisEntity)
{
        ndisEntity->spePort = NDIS_INVALID_SPEPORT;
    return IP_SUCC;
}

STATIC VOS_UINT32 Ndis_PdnJugeV4V6ConfigIndicate(const AT_NDIS_PdnInfoCfgReq *nasNdisInfo, NDIS_Entity *ndisEntity,
                                                 VOS_UINT32 *v4Ret, VOS_UINT32 *v6Ret, AT_NDIS_PdnInfoCfgCnf cfgCnf)
{
    *v4Ret = Ndis_PdnV4PdnCfg(nasNdisInfo, ndisEntity);
    *v6Ret = Ndis_PdnV6PdnCfg(nasNdisInfo, ndisEntity);

    if ((*v6Ret == PS_FAIL) && (*v4Ret == PS_FAIL)) { /* 如果IPV4和IPV6配置指示信息都无效，也认为配置失败 */
        /* 向AT回复PDN配置失败 */
        (VOS_VOID)Ndis_SndMsgToAt((VOS_UINT8 *)&cfgCnf, sizeof(AT_NDIS_PdnInfoCfgCnf),
                                  ID_AT_NDIS_PDNINFO_CFG_CNF);
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_PdnJugeV4V6ConfigIndicate,  Ipv4 and Ipv6 Cfg all fail!");
        return PS_FAIL;
    }
    return PS_SUCC;
}
STATIC VOS_UINT32 NdisCfgParaCheck(const AT_NDIS_PdnInfoCfgReq *nasNdisInfo, VOS_UINT8 exRabId,
                                   AT_NDIS_PdnInfoCfgCnf cfgCnf)
{
    /* ExRabId参数范围有效性检查。若检查失败，则直接向AT回复配置失败 */
    if (Ndis_ChkRabIdValid(exRabId) != PS_SUCC) {
        (VOS_VOID)Ndis_SndMsgToAt((VOS_UINT8 *)&cfgCnf, sizeof(AT_NDIS_PdnInfoCfgCnf), ID_AT_NDIS_PDNINFO_CFG_CNF);
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_PdnInfoCfgProc,  Ndis_ChkRabIdValid fail!");
        return PS_FAIL;
    }
    return PS_SUCC;
}

#if (FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
#if (VOS_OS_VER != VOS_WIN32)
/*
 * 功能描述: 桩函数
 * 修改历史:
 *  1.日    期: 2018年08月21日
 *    修改内容: 新生成函数
 */
VOS_UINT32 ADS_DL_RegDlDataCallback(VOS_UINT8 rabId, RCV_DL_DATA_FUNC pFunc, VOS_UINT32 exParam)
{
    return PS_SUCC;
}
#endif
#else
#if (VOS_OS_VER != VOS_WIN32)
/*
 * 功能描述: 桩函数
 * 修改历史:
 *  1.日    期: 2018年08月21日
 *    修改内容: 新生成函数
 */
int ads_iface_register_rx_handler(VOS_UINT8 ifaceId, struct ads_iface_rx_handler_s *rxHandler)
{
    return PS_SUCC;
}
#endif
#endif

/*
 * 功能描述: PDN地址信息参数配置
 */
STATIC VOS_VOID Ndis_PdnInfoCfgProc(const AT_NDIS_PdnInfoCfgReq *nasNdisInfo)
{
    VOS_UINT8                   exRabId = NDIS_FORM_EXBID(nasNdisInfo->modemId, nasNdisInfo->rabId);
    NDIS_Entity                *ndisEntity = VOS_NULL_PTR;
    AT_NDIS_PdnInfoCfgCnf       cfgCnf     = {0};
    VOS_UINT32                  v4Ret;
    VOS_UINT32                  v6Ret;

    cfgCnf.result  = AT_NDIS_PDNCFG_CNF_FAIL;
    cfgCnf.rabId   = nasNdisInfo->rabId;
    cfgCnf.modemId = nasNdisInfo->modemId;
    if (NdisCfgParaCheck(nasNdisInfo, exRabId, cfgCnf) != PS_SUCC) {
        return;
    }

    /* 如果根据ExRabId查找不到NDIS实体，则分配一个空闲的NDIS实体 */
    ndisEntity = NDIS_GetEntityByRabId(exRabId);
    if (ndisEntity == VOS_NULL_PTR) {
        /* 如果分配不到空闲的NDIS实体，则返回 */
        ndisEntity = NDIS_AllocEntity();
        if (ndisEntity == VOS_NULL_PTR) {
            /* 向AT回复PDN配置失败 */
            (VOS_VOID)Ndis_SndMsgToAt((VOS_UINT8 *)&cfgCnf, sizeof(AT_NDIS_PdnInfoCfgCnf), ID_AT_NDIS_PDNINFO_CFG_CNF);
            NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_PdnInfoCfgProc,  NDIS_AllocEntity failed!");
            return;
        }

        /* 该承载之前没有对应的NDIS实体，故填无效值 */
        NDIS_SET_INITIALSETUP_NDISENTITY_INFO(ndisEntity);
    }

    if (Ndis_PdnJugeV4V6ConfigIndicate(nasNdisInfo, ndisEntity, &v4Ret, &v6Ret, cfgCnf) == PS_FAIL) {
        return;
    }
    NDIS_SET_EXIST_NDIS_INFO(ndisEntity, exRabId, nasNdisInfo->handle, nasNdisInfo->iSpePort, nasNdisInfo->ipfFlag);

    cfgCnf.result  = Ndis_AtCnfResultProc(nasNdisInfo, v4Ret, v6Ret);
    cfgCnf.rabType = ndisEntity->rabType;

    /* 启动周期发送ARP的定时器 */
    if (Ndis_StartARPTimer(ndisEntity) != PS_SUCC) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_ConfigArpInfo StartTmr Failed!");
        return;
    }

    if (Ndis_PdnInfoCfgSpeProc(ndisEntity) == IP_FAIL) {
        return;
    }

    /* 向ADS注册下行回调:只注册一次 */
    if (ADS_DL_RegDlDataCallback(exRabId, Ndis_DlAdsDataRcv, 0) != VOS_OK) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_PdnInfoCfgProc, ADS_DL_RegDlDataCallback fail!");
        return;
    }

    /*lint -e718*/
    if (NDIS_UDI_IOCTL(ndisEntity->handle, NCM_IOCTL_REG_UPLINK_RX_FUNC, AppNdis_UsbReadCb) != VOS_OK) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_PdnInfoCfgProc, regist AppNdis_UsbReadCb fail!");
        return;
    }
    /*lint +e718*/

    if (Ndis_SndMsgToAt((VOS_UINT8 *)&cfgCnf, sizeof(AT_NDIS_PdnInfoCfgCnf), ID_AT_NDIS_PDNINFO_CFG_CNF) == PS_FAIL) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_PdnInfoCfgProc, send msg to at fail!");
    }

    return;
}

/*
 * 功能描述: PDN释放
 */
STATIC VOS_VOID Ndis_PdnRel(const AT_NDIS_PdnInfoRelReq *nasNdisRel)
{
    VOS_UINT8                    exRabId;
    NDIS_Entity                 *ndisEntity     = VOS_NULL_PTR;
    NDIS_ArpPeriodTimer         *arpPeriodTimer = VOS_NULL_PTR;
    AT_NDIS_PdnInfoRelCnf        relCnf;

    NDIS_INFO_LOG(NDIS_TASK_PID, "Ndis_PdnRel entered!");
    relCnf.result  = AT_NDIS_FAIL;
    relCnf.rabId   = nasNdisRel->rabId;
    relCnf.modemId = nasNdisRel->modemId;

    exRabId = NDIS_FORM_EXBID(nasNdisRel->modemId, nasNdisRel->rabId);
    if (Ndis_ChkRabIdValid(exRabId) == PS_FAIL) {
        (VOS_VOID)Ndis_SndMsgToAt((VOS_UINT8 *)&relCnf, sizeof(AT_NDIS_PdnInfoRelCnf),
                                  ID_AT_NDIS_PDNINFO_REL_CNF);
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_PdnRel,  Ndis_ChkRabIdValid fail!");
        return;
    }

    ndisEntity = NDIS_GetEntityByRabId(exRabId);
    if (ndisEntity == VOS_NULL_PTR) {
        (VOS_VOID)Ndis_SndMsgToAt((VOS_UINT8 *)&relCnf, sizeof(AT_NDIS_PdnInfoRelCnf),
                                  ID_AT_NDIS_PDNINFO_REL_CNF);
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_PdnRel,  NDIS_GetEntityByRabId failed!");
        return;
    }

    arpPeriodTimer = &(ndisEntity->ipV4Info.arpPeriodTimer);

    relCnf.rabType = ndisEntity->rabType;

    /* 如果周期性ARP定时器还在运行，则停掉 */
    Ndis_StopARPTimer(arpPeriodTimer);

    /* 调用ND SERVER API 释放该RabId对应ND SERVER实体 */
    if ((ndisEntity->rabType & NDIS_ENTITY_IPV6) == NDIS_ENTITY_IPV6) {
        NdSer_Ipv6PdnRel(exRabId);
    }

    /* 更新该RabId对应NDIS实体为空 */
    ndisEntity->rabType    = NDIS_RAB_NULL;
    ndisEntity->rabId      = NDIS_INVALID_RABID;
    ndisEntity->handle     = NDIS_INVALID_HANDLE;
    ndisEntity->used       = PS_FALSE;
    ndisEntity->spePort     = NDIS_INVALID_SPEPORT;
    ndisEntity->speIpfFlag = PS_FALSE;

    /* NDIS向AT回复释放确认原语 */
    relCnf.result = AT_NDIS_SUCC;
    if (Ndis_SndMsgToAt((VOS_UINT8 *)&relCnf, sizeof(AT_NDIS_PdnInfoRelCnf), ID_AT_NDIS_PDNINFO_REL_CNF) == PS_FAIL) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_PdnRel, ndis send msg to at fail!");
    }

    return;
}

/*
 * 功能描述: IFACE UP CFG消息内容转成PND INFO CFG消息内容
 * 修改历史:
 *  1.日    期: 2018年08月16日
 *    修改内容: 新增
 */
STATIC VOS_VOID Ndis_ATIfaceUpCfgTransToPdnInfoCfg(const AT_NDIS_IfaceUpConfigInd *ndisIFaceInfo,
                                                   AT_NDIS_PdnInfoCfgReq *ndisPdnInfo)
{
    errno_t rlt;
    (VOS_VOID)memset_s((VOS_UINT8 *)ndisPdnInfo, sizeof(AT_NDIS_PdnInfoCfgReq), 0,
                       sizeof(AT_NDIS_PdnInfoCfgReq));

    ndisPdnInfo->msgId          = ndisIFaceInfo->msgId;
    ndisPdnInfo->opIpv4PdnInfo = ndisIFaceInfo->opIpv4PdnInfo;
    ndisPdnInfo->opIpv6PdnInfo = ndisIFaceInfo->opIpv6PdnInfo;
    ndisPdnInfo->opEthPdnInfo  = ndisIFaceInfo->opEthPdnInfo;
    ndisPdnInfo->modemId        = NDIS_GET_MODEMID_FROM_EXBID(ndisIFaceInfo->iFaceId); /* 将Iface ID转成ModemId+RabId */
    ndisPdnInfo->rabId = NDIS_GET_BID_FROM_EXBID(ndisIFaceInfo->iFaceId); /* 将Iface ID转成ModemId+RabId */
    ndisPdnInfo->handle = ndisIFaceInfo->handle;
    rlt = memcpy_s(&ndisPdnInfo->ipv4PdnInfo, sizeof(AT_NDIS_Ipv4PdnInfo),
                   &ndisIFaceInfo->ipv4PdnInfo, sizeof(AT_NDIS_Ipv4PdnInfo));
    NDIS_CHK_SEC_RETURN_NO_VAL((rlt != EOK));

    rlt = memcpy_s(&ndisPdnInfo->ipv6PdnInfo, sizeof(AT_NDIS_Ipv6PdnInfo),
                   &ndisIFaceInfo->ipv6PdnInfo, sizeof(AT_NDIS_Ipv6PdnInfo));
    NDIS_CHK_SEC_RETURN_NO_VAL((rlt != EOK));

    return;
}

STATIC VOS_UINT32 Ndis_IfaceJugeIfaceConfigIndicate(AT_NDIS_PdnInfoCfgReq *pdnInfo, NDIS_Entity *ndisEntity)
{
    VOS_UINT32                    v4Ret;
    VOS_UINT32                    v6Ret;
    VOS_UINT32                    ethRet;

    v4Ret  = Ndis_PdnV4PdnCfg(pdnInfo, ndisEntity);
    v6Ret  = Ndis_PdnV6PdnCfg(pdnInfo, ndisEntity);
    ethRet = Ndis_PdnEthPdnCfg(pdnInfo, ndisEntity);
    /* 如果IPV4和IPV6和ETH配置指示信息都无效，也认为配置失败 */
    if ((v6Ret == PS_FAIL) && (v4Ret == PS_FAIL) && (ethRet == PS_FAIL)) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_IfaceJugeIfaceConfigIndicate,  Ipv4/Ipv6/Eth Cfg all fail!");
        return PS_FAIL;
    }

    return PS_SUCC;
}

STATIC VOS_UINT32 Ndis_RegisterCbProc(NDIS_Entity *ndisEntity)
{
    struct ads_iface_rx_handler_s ifaceRxHandle;
    (VOS_VOID)memset_s(&ifaceRxHandle, sizeof(ifaceRxHandle), 0, sizeof(ifaceRxHandle));
    ifaceRxHandle.user_data     = ndisEntity->rabId;
    ifaceRxHandle.rx_func       = Ndis_DlAdsDataRcvV2;
    ifaceRxHandle.rx_cmplt_func = VOS_NULL_PTR;

    /* 向ADS注册下行回调 */
    if (ads_iface_register_rx_handler(ndisEntity->rabId, &ifaceRxHandle) != VOS_OK) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_RegisterCbProc, ADS_DL_RegDlDataCallback fail!");
        return PS_FAIL;
    }

    /*lint -e718*/
    if (NDIS_UDI_IOCTL(ndisEntity->handle, NCM_IOCTL_REG_UPLINK_RX_FUNC, AppNdis_UsbReadCb) != VOS_OK) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_RegisterCbProc, regist AppNdis_UsbReadCb fail!");
        return PS_FAIL;
    }
    /*lint +e718*/
    return PS_SUCC;
}

STATIC VOS_VOID Ndis_IfaceUpCfgProc(const AT_NDIS_IfaceUpConfigInd *ifacInfo)
{
    VOS_UINT8                     exRabId = ifacInfo->iFaceId;
    VOS_INT32                     handle;
    NDIS_Entity                  *ndisEntity = VOS_NULL_PTR;
    AT_NDIS_PdnInfoCfgReq         pdnInfo;

    /* ExRabId参数范围有效性检查。若检查失败，则直接向AT回复配置失败 */
    if (Ndis_ChkRabIdValid(exRabId) != PS_SUCC) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_IfaceUpCfgProc,  Ndis_ChkRabIdValid fail!");
        return;
    }

    /* 如果根据ExRabId查找不到NDIS实体，则分配一个空闲的NDIS实体 */
    ndisEntity = NDIS_GetEntityByRabId(exRabId);
    if (ndisEntity == VOS_NULL_PTR) {
        /* 如果分配不到空闲的NDIS实体，则返回 */
        ndisEntity = NDIS_AllocEntity();
        if (ndisEntity == VOS_NULL_PTR) {
            NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_IfaceUpCfgProc,  NDIS_AllocEntity failed!");
            return;
        }

        /* 该承载之前没有对应的NDIS实体，故填无效值 */
        NDIS_SET_INITIALSETUP_NDISENTITY_INFO(ndisEntity);
    }

    Ndis_ATIfaceUpCfgTransToPdnInfoCfg(ifacInfo, &pdnInfo);
    if (Ndis_IfaceJugeIfaceConfigIndicate(&pdnInfo, ndisEntity) == PS_FAIL) {
        return;
    }
    handle  = ifacInfo->handle;
    NDIS_SET_EXIST_NDIS_INFO(ndisEntity, exRabId, handle, 0, 0);

    /* 启动周期发送ARP的定时器 */
    if (Ndis_StartARPTimer(ndisEntity) != PS_SUCC) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_IfaceUpCfgProc StartTmr Failed!");
        return;
    }

    ndisEntity->spePort = NDIS_INVALID_SPEPORT;

    if (Ndis_RegisterCbProc(ndisEntity) != PS_SUCC) {
        return;
    }

#if (FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
#if (defined(CONFIG_BALONG_ESPE))
    mdrv_wan_set_ipfmac((struct ethhdr *)(VOS_VOID *)&g_speMacHeader);
#endif
#endif

    return;
}


/*
 * Description: 释放下行缓存队列中的PKT
 * History: VOS_VOID
 */
STATIC VOS_VOID NDIS_ClearDlPktQue(VOS_UINT32 addrIndex)
{
    IMM_Zc      *immZc = VOS_NULL_PTR;
    if (addrIndex >= NAS_NDIS_MAX_ITEM) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "NDIS_ClearDlPktQue, addrIndex invalid!");
        return;
    }
    if (NDIS_GET_DL_PKTQUE(addrIndex) == VOS_NULL_PTR) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "NDIS_ClearDlPktQue, que null!");
        return;
    }

    while (LUP_IsQueEmpty(NDIS_GET_DL_PKTQUE(addrIndex)) != PS_TRUE) {
        if (LUP_DeQue(NDIS_GET_DL_PKTQUE(addrIndex), (VOS_VOID **)(&immZc)) != PS_SUCC) {
            IPND_ERROR_LOG(NDIS_NDSERVER_PID, "NDIS_ClearDlPktQue, LUP_DeQue return fail!");
            return;
        }
        /*lint -e522*/
        IMM_ZcFree(immZc);
        /*lint +e522*/
    }

    return;
}

/*
 * 功能描述: IFACE DOWN消息处理
 * 修改历史:
 *  1.日    期: 2018年08月16日
 *    修改内容: 新生成函数
 */
STATIC VOS_VOID Ndis_IfaceDownCfgProc(const AT_NDIS_IfaceDownConfigInd *ifacInfo)
{
    VOS_UINT8            exRabId;
    NDIS_Entity         *ndisEntity     = VOS_NULL_PTR;
    NDIS_ArpPeriodTimer *arpPeriodTimer = VOS_NULL_PTR;
    VOS_UINT32           addrIndex;

    NDIS_INFO_LOG(NDIS_TASK_PID, "Ndis_IfaceDownCfgProc entered!");

    exRabId = ifacInfo->iFaceId;
    if (Ndis_ChkRabIdValid(exRabId) != PS_SUCC) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "pstIfacInfo,  Ndis_ChkRabIdValid fail!");
        return;
    }

    ndisEntity = NDIS_GetEntityByRabId(exRabId);
    if (ndisEntity == VOS_NULL_PTR) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "pstIfacInfo,  NDIS_GetEntityByRabId failed!");
        return;
    }

    arpPeriodTimer = &(ndisEntity->ipV4Info.arpPeriodTimer);

    /* 如果周期性ARP定时器还在运行，则停掉 */
    Ndis_StopARPTimer(arpPeriodTimer);

    /* 调用ND SERVER API 释放该RabId对应ND SERVER实体 */
    if ((ndisEntity->rabType & NDIS_ENTITY_IPV6) == NDIS_ENTITY_IPV6) {
        NdSer_Ipv6PdnRel(exRabId);
    }

    /* 更新该RabId对应NDIS实体为空 */
    ndisEntity->rabType    = NDIS_RAB_NULL;
    ndisEntity->rabId      = NDIS_INVALID_RABID;
    ndisEntity->handle     = NDIS_INVALID_HANDLE;
    ndisEntity->used       = PS_FALSE;
    ndisEntity->spePort     = NDIS_INVALID_SPEPORT;
    ndisEntity->speIpfFlag = PS_FALSE;

    /* 释放下行PKT缓存队列 */
    addrIndex = NDIS_GetEntityIndex(exRabId);
    NDIS_ClearDlPktQue(addrIndex);

    return;
}

VOS_VOID Ndis_AtMsgProc(const MsgBlock *msgBlock)
{
    AT_NDIS_MsgIdUint32 msgId;
    NDIS_MsgTypeStruLen msgStruLen[] = {
        { ID_AT_NDIS_PDNINFO_CFG_REQ,  sizeof(AT_NDIS_PdnInfoCfgReq) },
        { ID_AT_NDIS_PDNINFO_REL_REQ, sizeof(AT_NDIS_PdnInfoRelReq) },
        { ID_AT_NDIS_IFACE_UP_CONFIG_IND, sizeof(AT_NDIS_IfaceUpConfigInd) },
        { ID_AT_NDIS_IFACE_DOWN_CONFIG_IND, sizeof(AT_NDIS_IfaceDownConfigInd) },
    };

    VOS_UINT32 msgIdNum = sizeof(msgStruLen) / sizeof(NDIS_MsgTypeStruLen);
    COVERITY_TAINTED_SET(msgBlock->value); /* 鹰眼插桩 */

    /* 长度异常保护 */
    if (sizeof(MSG_Header) - VOS_MSG_HEAD_LENGTH > msgBlock->ulLength) {
        NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_AtMsgProc: input msg length less than struc MSG_Header",
                        msgBlock->ulLength);
        return;
    }

    msgId = ((MSG_Header *)(VOS_VOID *)msgBlock)->msgName;
    if (NDIS_RecvMsgParaCheck(msgBlock->ulLength, msgStruLen, msgIdNum, msgId) == PS_FAIL) {
        NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_AtMsgProc: input msg length less than struc MSG_Header",
                        msgId);
        return;
    }
    switch (msgId) {
        case ID_AT_NDIS_PDNINFO_CFG_REQ: /* 根据消息的不同处理AT不同的请求 */
            Ndis_PdnInfoCfgProc((AT_NDIS_PdnInfoCfgReq *)(VOS_VOID *)msgBlock);
            break;

        case ID_AT_NDIS_PDNINFO_REL_REQ:
            Ndis_PdnRel((AT_NDIS_PdnInfoRelReq *)(VOS_VOID *)msgBlock);
            break;
        case ID_AT_NDIS_IFACE_UP_CONFIG_IND:
            Ndis_IfaceUpCfgProc((AT_NDIS_IfaceUpConfigInd *)(VOS_VOID *)msgBlock);
            break;

        case ID_AT_NDIS_IFACE_DOWN_CONFIG_IND:
            Ndis_IfaceDownCfgProc((AT_NDIS_IfaceDownConfigInd *)(VOS_VOID *)msgBlock);
            break;

        default:
            NDIS_WARNING_LOG(NDIS_TASK_PID, "Ndis_AtMsgProc:Error Msg!");
            break;
    }

    return;
}




