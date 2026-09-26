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

#ifndef __ADS_DEV_INTERFACE_H__
#define __ADS_DEV_INTERFACE_H__

#include "imm_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

/* 定义IPV6 MTU最大长度值 */
#define ADS_MTU_LEN_MAX 1500


enum ADS_PktType {
    ADS_PKT_TYPE_IPV4 = 0x00, /* IPV4 */
    ADS_PKT_TYPE_IPV6 = 0x01, /* IPV6 */
    ADS_PKT_TYPE_ETH  = 0x02, /* ETH */
    ADS_PKT_TYPE_BUTT
};
typedef VOS_UINT8 ADS_PktTypeUint8;


typedef struct {
    VOS_UINT32 opIpv4Addr : 1;
    VOS_UINT32 opIpv6Addr : 1;
    VOS_UINT32 opSpare : 30;

    VOS_UINT8 ipv4Addr[4];
    VOS_UINT8 ipv6Addr[16];
} ADS_FilterIpAddrInfo;

/*
 * 功能描述: ADS上行为上层模块提供的数据发送函数
 *           如果返回失败，内存释放由该接口执行，上层模块不需要有释放内存操作
 */

VOS_INT ADS_UL_SendPacket(IMM_Zc *immZc, VOS_UINT8 exRabId);

typedef VOS_INT (*RCV_DL_DATA_FUNC)(VOS_UINT8 ucExRabId, IMM_Zc *pData, ADS_PktTypeUint8 enPktType,
                                    VOS_UINT32 ulExParam);

/*
 * 功能描述: ADS下行数据处理模块为上层模块提供的注册下行数据接收函数接口
 */
VOS_UINT32 ADS_DL_RegDlDataCallback(VOS_UINT8 exRabId, RCV_DL_DATA_FUNC func, VOS_UINT32 exParam);

/*
 * 功能描述: 注册下行数据过滤回调扩展
 */
VOS_UINT32 ADS_DL_RegFilterDataCallback(VOS_UINT8 rabId, ADS_FilterIpAddrInfo *filterIpAddr, RCV_DL_DATA_FUNC func);

/*
 * 功能描述: 去注册下行数据过滤回调扩展
 */
VOS_UINT32 ADS_DL_DeregFilterDataCallback(VOS_UINT32 rabId);

/*
 * 功能描述: 上行发送数据扩展接口, 使用该接口发送的数据在ADS记录其信息, 用于
 *           下行数据过滤匹配, 该接口必须和ADS_DL_RegFilterDataCallback配合
 *           使用, 只有使用ADS_DL_RegFilterDataCallback注册过下行过滤回调后,
 *           下行数据才需要根据使用该接口记录的信息进行过滤
 *           如果返回失败，内存释放由该接口执行
 */
VOS_INT ADS_UL_SendPacketEx(IMM_Zc *immZc, ADS_PktTypeUint8 ipType, VOS_UINT8 exRabId);

#if (FEATURE_ON == FEATURE_RNIC_NAPI_GRO)
typedef VOS_VOID (*RCV_RD_LAST_DATA_FUNC)(VOS_UINT32 ulExParam);

/*
 * 功能描述: ADS下行数据处理模块为上层模块提供的注册下行RD最后一个数据处理回调函数
 */
VOS_UINT32 ADS_DL_RegNapiCallback(VOS_UINT8 exRabId, RCV_RD_LAST_DATA_FUNC lastDataFunc, VOS_UINT32 exParam);
#endif

#if (defined(CONFIG_BALONG_SPE))
/*
 * 功能描述: SPE中断触发事件, 唤醒ADS任务处理上行数据
 */
VOS_VOID ADS_IPF_SpeIntWakeupADS(VOS_VOID);

/*
 * 功能描述: 回收内存
 */
VOS_INT ADS_IPF_RecycleMem(IMM_Zc *immZc);

/*
 * 功能描述: 注册SPE端口
 */
VOS_VOID ADS_IPF_RegSpeWPort(VOS_INT32 port);

/*
 * 功能描述: 获取SPE WPORT端口的TD深度配置
 */
VOS_UINT32 ADS_IPF_GetSpeWPortTdDepth(VOS_VOID);
#endif

/*
 *功能描述：获取ipfMode端口
 */
VOS_UINT8 ADS_GetIpfMode(VOS_VOID);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
