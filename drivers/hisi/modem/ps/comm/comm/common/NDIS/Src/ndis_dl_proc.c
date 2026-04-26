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

#include "ndis_dl_proc.h"
#include "vos.h"
#include "ads_device_interface.h"
#include "imm_interface.h"
#include "ip_nd_server.h"
#include "ndserver_entity.h"
#include "ndis_drv.h"
#include "ndis_interface.h"
#include "ndis_entity.h"
#include "ndis_debug.h"
#include "ndis_om.h"


#define THIS_MODU ps_ndis
/*lint -e767*/
#define THIS_FILE_ID PS_FILE_ID_NDISDLPROC_C
/*lint +e767*/

/*
 * 2 Declare the Global Variable
 */

/* 声明 */

/*
 * 功能描述: NDIS_ADS下行数据接收回调函数
 * 修改历史:
 *  1.日    期: 2011年12月9日
 *    修改内容: 新生成函数
 *  2.日    期: 2013年1月16日
 *    修改内容: DSDA特性开发，入参修改为扩展承载ID
 */
VOS_INT Ndis_DlAdsDataRcv(VOS_UINT8 exRabId, IMM_Zc *data, ADS_PktTypeUint8 pktType, VOS_UINT32 exParam)
{
    if (data == VOS_NULL_PTR) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DlAdsDataRcv, recv NULL PTR!");
        return PS_FAIL;
    }

    if ((Ndis_ChkRabIdValid(exRabId) != PS_SUCC) || (pktType >= ADS_PKT_TYPE_BUTT)) {
        /*lint -e522*/
        IMM_ZcFree(data);
        /*lint +e522*/
        NDIS_ERROR_LOG2(NDIS_TASK_PID, "Ndis_DlAdsDataRcv, recv RabId or PktType fail!", exRabId, pktType);
        NDIS_STAT_DL_DISCARD_ADSPKT(1);
        return PS_FAIL;
    }

    /* 增加从ADS接收到的数据包个数统计 */
    NDIS_STAT_DL_RECV_ADSPKT_SUCC(1);

    Ndis_LomTraceRcvDlData();
    if (Ndis_DlSendNcm(exRabId, pktType, data) != PS_SUCC) {
        /*lint -e522*/
        IMM_ZcFree(data);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DlAdsDataRcv, Ndis_DlSendNcm fail!");
        return PS_FAIL;
    }

    return PS_SUCC;
}

/*
 * 功能描述: NDIS_ADS下行数据接收回调函数
 * 修改历史:
 *  1.日    期: 2018年08月16日
 *    修改内容: 新生成函数
 */
VOS_INT Ndis_DlAdsDataRcvV2(unsigned long userData, IMM_Zc *data)
{
    ADS_PktTypeUint8    pktType;

    if (data == VOS_NULL_PTR) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DlAdsDataRcvV2, recv NULL PTR!");
        return PS_FAIL;
    }

    if (Ndis_ChkRabIdValid((VOS_UINT8)userData) != PS_SUCC) {
        /*lint -e522*/
        IMM_ZcFree(data);
        /*lint +e522*/
        NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_DlAdsDataRcvV2, recv RabId or PktType fail!", userData);
        NDIS_STAT_DL_DISCARD_ADSPKT(1);
        return PS_FAIL;
    }

    /* 增加从ADS接收到的数据包个数统计 */
    NDIS_STAT_DL_RECV_ADSPKT_SUCC(1);

    Ndis_LomTraceRcvDlData();
    if (IMM_ZcGetProtocol(data) == IP_PAYLOAD) {
        pktType = ADS_PKT_TYPE_IPV4;
    } else {
        pktType = ADS_PKT_TYPE_IPV6;
    }
    if (Ndis_DlSendNcm((VOS_UINT8)userData, pktType, data) != PS_SUCC) {
        /*lint -e522*/
        IMM_ZcFree(data);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DlAdsDataRcvV2, Ndis_DlSendNcm fail!");
        return PS_FAIL;
    }

    return PS_SUCC;
}


/*
 * Function Name: NDIS_SendDlPkt
 * Description: 发送下行缓存的IPV4数据包
 */
VOS_VOID NDIS_SendDlPkt(VOS_UINT32 addrIndex)
{
    IMM_Zc                 *immZc  = VOS_NULL_PTR;
    VOS_UINT8               rabId;
    ADS_PktTypeUint8        pktType;
    VOS_UINT16              app;
    VOS_INT32               lockKey;

    if (addrIndex >= NAS_NDIS_MAX_ITEM) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "NDIS_SendDlPkt, addrIndex invalid!");
        return;
    }
    if (NDIS_GET_DL_PKTQUE(addrIndex) == VOS_NULL_PTR) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "NDIS_SendDlPkt, que null!");
        return;
    }

    while (LUP_IsQueEmpty(NDIS_GET_DL_PKTQUE(addrIndex)) != PS_TRUE) {
        lockKey = VOS_SplIMP();
        if (LUP_DeQue(NDIS_GET_DL_PKTQUE(addrIndex), (VOS_VOID **)(&immZc)) != PS_SUCC) {
            VOS_Splx(lockKey);
            NDIS_ERROR_LOG(NDIS_TASK_PID, "NDIS_SendDlPkt, LUP_DeQue return fail!");
            return;
        }
        VOS_Splx(lockKey);

        /* 从 ImmZc中取出RabId和enPdpType */
        /*lint -e522*/
        app     = IMM_ZcGetUserApp(immZc);
        rabId   = (VOS_UINT8)(app & 0xFF); /* 保存的值实际是 扩展RabId */
        pktType = (VOS_UINT8)(app >> 8); /* 8:bit */
        if (Ndis_DlSendNcm(rabId, pktType, immZc) != PS_SUCC) {
            IMM_ZcFree(immZc);
            NDIS_ERROR_LOG(NDIS_TASK_PID, "NDIS_SendDlPkt, Ndis_DlSendNcm fail!");
        }
        /*lint +e522*/
        NDIS_STAT_SEND_QUE_PKT_NUM(1);
    }

    return;
}

/*
 * Description: 当未获得PC MAC地址时，缓存下行IP包
 * History: VOS_UINT8
 */
VOS_VOID NDIS_MacAddrInvalidProc(IMM_Zc *immZc, VOS_UINT8 addrIndex)
{
    IMM_Zc      *queHead = VOS_NULL_PTR;

    NDIS_STAT_MAC_INVALID_PKT_NUM(1);
    if (addrIndex >= NAS_NDIS_MAX_ITEM) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "NDIS_MacAddrInvalidProc, addrIndex invalid!");
        return;
    }

    if (NDIS_GET_DL_PKTQUE(addrIndex) == VOS_NULL_PTR) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "NDIS_MacAddrInvalidProc, que null!");
        return;
    }

    /* **********************下行IPV4数据包缓存******************************** */
    if (LUP_IsQueFull(NDIS_GET_DL_PKTQUE(addrIndex)) == PS_TRUE) {
        if (LUP_DeQue(NDIS_GET_DL_PKTQUE(addrIndex), (VOS_VOID **)(&queHead)) != PS_SUCC) {
            /*lint -e522*/
            IMM_ZcFree(immZc);
            /*lint +e522*/
            NDIS_ERROR_LOG(NDIS_TASK_PID, "NdSer_MacAddrInvalidProc, LUP_DeQue return NULL");
            return;
        }
        /*lint -e522*/
        IMM_ZcFree(queHead); /* 释放最早的IP包 */
        /*lint +e522*/
    }

    if (LUP_EnQue(NDIS_GET_DL_PKTQUE(addrIndex), immZc) != PS_SUCC) { /* 插入最新的IP包 */
        /*lint -e522*/
        IMM_ZcFree(immZc);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "NdSer_MacAddrInvalidProc, LUP_EnQue return NULL");
        return;
    }

    NDIS_STAT_ENQUE_PKT_NUM(1);

    return;
}

/*
 * 功能描述: 下行方向的IPV4 NCM数据的发送
 */
STATIC VOS_UINT32 Ndis_DlUsbIpv4Proc(VOS_UINT8 **addData, VOS_UINT8 exRabId, IMM_Zc *immZc)
{
    VOS_UINT8         index;
    NDIS_Entity       *ndisEntity = VOS_NULL_PTR;
    VOS_UINT16        app;
    VOS_UINT16        tmpApp;

    /* 使用ExRabId获取NDIS实体 */
    ndisEntity = NDIS_GetEntityByRabId(exRabId);
    if (ndisEntity == VOS_NULL_PTR) {
        NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_DlUsbIpv4Proc, NDIS_GetEntityByRabId Error!", exRabId);
        return PS_FAIL;
    }

    if (ndisEntity->ipV4Info.arpInitFlg == PS_FALSE) {
        /* 将ucExRabId和数据包类型放入ImmZc的私有数据域中 */
        tmpApp = (ADS_PKT_TYPE_IPV4 & 0xFF);
        app    = ((VOS_UINT16)(tmpApp << 8)) | (exRabId); /* 8:bit */
        IMM_ZcSetUserApp(immZc, app);
        index = NDIS_GetEntityIndex(exRabId);
        NDIS_MacAddrInvalidProc(immZc, index);
        return PS_SUCC;
    }

    /* 填充MAC帧头，调用ImmZc接口将MAC帧头填入ImmZc中 */
    *addData = ndisEntity->ipV4Info.macFrmHdr;
    return PS_RSLT_CODE_BUTT;
}

/*
 * 功能描述: 下行方向的IPV6 NCM数据的发送
 */
STATIC VOS_UINT32 Ndis_DlUsbIpv6Proc(VOS_UINT8 **addData, VOS_UINT8 exRabId, IMM_Zc *immZc)
{
    VOS_UINT8         index;
    VOS_UINT8         teAddrState;
    VOS_UINT16        app;
    VOS_UINT16        tmpApp;

    /* 填充MAC帧头，调用ImmZc接口将MAC帧头填入ImmZc中 */
    index = (VOS_UINT8)IP_NDSERVER_GET_ADDR_INFO_INDEX(exRabId);
    if (index >= IP_NDSERVER_ADDRINFO_MAX_NUM) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DlUsbIpv6Proc, IP_NDSERVER_GET_ADDR_INFO_INDEX fail!");
        return PS_FAIL;
    }
    teAddrState = IP_NDSERVER_TE_ADDR_BUTT;
    *addData = NdSer_GetMacFrm(index, &teAddrState);
    if (*addData == VOS_NULL_PTR) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DlUsbIpv6Proc, NdSer_GetMacFrm fail!");
        NDIS_STAT_DL_GET_IPV6MAC_FAIL(1);
        return PS_FAIL;
    }

    if (teAddrState != IP_NDSERVER_TE_ADDR_REACHABLE) {
        /* 将ucExRabId和数据包类型放入ImmZc的私有数据域中 */
        tmpApp = (ADS_PKT_TYPE_IPV6 & 0xFF);
        app    = ((VOS_UINT16)(tmpApp << 8)) | (exRabId); /* 8:bit */
        IMM_ZcSetUserApp(immZc, app);

        NdSer_MacAddrInvalidProc(immZc, index);
        return PS_SUCC;
    }

    return PS_RSLT_CODE_BUTT;
}



/*
 * 功能描述: 下行方向的NCM数据的发送
 */
STATIC VOS_UINT32 Ndis_DlUsbSendNcm(NDIS_Entity *ndisEntity, VOS_UINT8 exRabId,
                                    ADS_PktTypeUint8 pktType, IMM_Zc *immZc)
{
    VOS_UINT8        *addData = VOS_NULL_PTR;
    VOS_UINT32        size;
    VOS_UINT32        result;

    /* 填充MAC帧头，调用ImmZc接口将MAC帧头填入ImmZc中 */
    if ((pktType == ADS_PKT_TYPE_IPV4) && ((ndisEntity->rabType & NDIS_ENTITY_IPV4) == NDIS_ENTITY_IPV4)) {
        result = Ndis_DlUsbIpv4Proc(&addData, exRabId, immZc);
        if (result != PS_RSLT_CODE_BUTT) {
            return result;
        }
    } else if ((pktType == ADS_PKT_TYPE_IPV6) && ((ndisEntity->rabType & NDIS_ENTITY_IPV6) == NDIS_ENTITY_IPV6)) {
        result = Ndis_DlUsbIpv6Proc(&addData, exRabId, immZc);
        if (result != PS_RSLT_CODE_BUTT) {
            return result;
        }
    } else if ((pktType == ADS_PKT_TYPE_ETH) && ((ndisEntity->rabType & NDIS_ENTITY_ETH) == NDIS_ENTITY_ETH)) {
    } else { /* 数据包类型与承载支持类型不一致 */
        NDIS_ERROR_LOG2(NDIS_TASK_PID, "Ndis_DlSendNcm, Rab is different from PktType!",
                        ndisEntity->rabType, pktType);
        NDIS_STAT_DL_PKT_DIFF_RAB_NUM(1);
        return PS_FAIL;
    }

    if (pktType != ADS_PKT_TYPE_ETH) {
        if (IMM_ZcAddMacHead(immZc, addData) != VOS_OK) {
            NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DlSendNcm, IMM_ZcAddMacHead fail!");
            NDIS_STAT_DL_ADDMACFRM_FAIL(1);
            return PS_FAIL;
        }
    }

    size = IMM_ZcGetUsedLen(immZc);

    /*lint -e718*/
    if (NDIS_UDI_WRITE(ndisEntity->handle, immZc, size) != 0) {
        NDIS_STAT_DL_SEND_USBPKT_FAIL(1);
        return PS_FAIL;
    }
    /*lint +e718*/

    NDIS_STAT_DL_SEND_USBPKT_SUCC(1);
    return PS_SUCC;
}

/*
 * 功能描述: 下行方向的NCM数据的发送
 * 修改历史:
 *  1.日    期: 2011年2月11日
 *    修改内容: 新生成函数
 *     2.日    期: 2015年2月11日
 *    修改内容: SPE
 */
VOS_UINT32 Ndis_DlSendNcm(VOS_UINT8 exRabId, ADS_PktTypeUint8 pktType, IMM_Zc *immZc)
{
    VOS_UINT32        result;
    NDIS_Entity      *ndisEntity = VOS_NULL_PTR;

    /* 使用ExRabId获取NDIS实体 */
    ndisEntity = NDIS_GetEntityByRabId(exRabId);
    if (ndisEntity == IP_NULL_PTR) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DlSendNcm, NDIS_GetEntityByRabId fail!");
        return PS_FAIL;
    }

    if ((ndisEntity->rabType & NDIS_ENTITY_ETH) == NDIS_ENTITY_ETH) {
        result = Ndis_DlUsbSendNcm(ndisEntity, exRabId, ADS_PKT_TYPE_ETH, immZc);
    } else {
        result = Ndis_DlUsbSendNcm(ndisEntity, exRabId, pktType, immZc);
    }
    return result;
}




