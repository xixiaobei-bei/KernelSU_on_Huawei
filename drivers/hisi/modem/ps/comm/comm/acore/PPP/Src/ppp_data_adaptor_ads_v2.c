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

#include "ppp_data_interface.h"
#include "v_typdef.h"
#include "ads_dev_i.h"

__STATIC_ PPP_DlPacketRxFun g_dlPacketReceiver = VOS_NULL_PTR;

VOS_INT PPP_PushPacketEvent(VOS_ULONG userData, IMM_Zc *immZc)
{
    PPP_Id pppId = (PPP_Id)userData;
    
    if (g_dlPacketReceiver != VOS_NULL_PTR) {
        return g_dlPacketReceiver(pppId, immZc);
    } else {
        PPP_MemFree(immZc);
        return VOS_ERR;
    }
}

VOS_UINT32 PPP_RegRxDlDataHandler(PPP_Id pppId, PPP_DlPacketRxFun fun)
{
    VOS_UINT8 ifaceId = 0;
    VOS_INT32 result;
    struct ads_iface_rx_handler_s rxHandler = {0};

    /* 通过usPppId，寻找到ucIfaceId */
    if (At_PppId2IfaceId(pppId, &ifaceId) != VOS_OK) {
        PPP_MNTN_LOG2(PS_PRINT_NORMAL, "Can not get Iface Id ", pppId, ifaceId);
        return VOS_ERR;
    }

    /* 这个时候PDP已经激活，注册上行数据接收接口 */
    rxHandler.user_data     = pppId;
    rxHandler.rx_func       = PPP_PushPacketEvent;
    rxHandler.rx_cmplt_func = VOS_NULL_PTR;
    result                  = ads_iface_register_rx_handler(ifaceId, &rxHandler);
    if (result != VOS_OK) {
        PPP_MNTN_LOG1(PS_PRINT_WARNING, "Register DL CB failed!", ifaceId);
        return VOS_ERR;
    }

    g_dlPacketReceiver = fun;
    return VOS_OK;
}

VOS_UINT32 PPP_TxUlData(VOS_UINT16 pppId, PPP_Zc *immZc, VOS_UINT16 proto)
{
    VOS_UINT8 ifaceId = 0;

    /* 通过pppId，寻找到ifaceId */
    if (At_PppId2IfaceId(pppId, &ifaceId) != VOS_OK) {
        PPP_MemFree(immZc);
        PPP_MNTN_LOG2(PS_PRINT_NORMAL, "Can not get IfaceId", pppId, ifaceId);

        return VOS_ERR;
    }

    /* 数据发送给ADS，如果失败由ADS释放内存 */
    if (ads_iface_tx(ifaceId, immZc) != VOS_OK) {
        return VOS_ERR;
    }
    return VOS_OK;
}

/* SPE要求数据包前预留MAC头内容 */
VOS_UINT32 PPP_ReserveHeaderInBypassMode(IMM_Zc **mem)
{
    VOS_UINT16 dataLen;
    IMM_Zc *newMem = VOS_NULL_PTR;
    IMM_Zc *oldMem = *mem;

    /* 递交给ADS的数据需要预留MAC头，因此透传模式下需要重新申请内存 */
    dataLen = PPP_ZC_GET_DATA_LEN(oldMem);
    if ((dataLen == 0) || (dataLen > PPP_ZC_MAX_DATA_LEN)) {
        PPP_MNTN_LOG1(PS_PRINT_WARNING, "DataLen err", dataLen);
        return VOS_ERR;
    }

    newMem = PPP_MemClone(PPP_ZC_GET_DATA_PTR(oldMem), dataLen, PPP_ZC_UL_RESERVE_LEN);
    if (newMem == VOS_NULL_PTR) {
        PPP_MNTN_LOG(PS_PRINT_WARNING, "Alloc Mem Fail");
        return VOS_ERR;
    }

    PPP_ZC_TRACE_MEM(newMem);

    *mem = newMem;
    PPP_MemFree(oldMem);
    return VOS_OK;
}

