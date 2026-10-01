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

#ifndef __RNIC_IFACE_CFG_H__
#define __RNIC_IFACE_CFG_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"
#include "RnicCtx.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/

#if (FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
#define RNIC_GET_FC_HEAD_BY_MODEMID(usModemId)  \
            ((usModemId == MODEM_ID_0) ? EIPF_MODEM0_ULFC : \
                ((usModemId == MODEM_ID_1) ? EIPF_MODEM1_ULFC : EIPF_MODEM2_ULFC))
#endif


/*****************************************************************************
  3 枚举定义
*****************************************************************************/


enum RNIC_BIT_OPT_ENUM
{
    RNIC_BIT_OPT_FALSE       = 0x00,
    RNIC_BIT_OPT_TRUE        = 0x01,
    RNIC_BIT_OPT_BUTT
};
typedef VOS_UINT8 RNIC_BIT_OPT_ENUM_UINT8;

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/


typedef struct
{
    VOS_UINT8                           aucRmnetName[RNIC_RMNET_NAME_MAX_LEN];  /* Rmnet网卡名 */
    AT_RNIC_USB_TETHER_CONN_ENUM_UINT8  enTetherConnStat;                       /* USB Tethering连接状态 */
    VOS_UINT8                           aucRsv[7];
} RNIC_PS_IFACE_TETHER_INFO_STRU;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
VOS_VOID RNIC_IFACE_SetFeatureCfg(VOS_VOID);
VOS_VOID RNIC_IFACE_SetNapiCfg(RNIC_DEV_ID_ENUM_UINT8 enRmNetId);
VOS_VOID RNIC_IFACE_AdjustNapiWeight(VOS_UINT8 ucRmNetId);
VOS_VOID RNIC_IFACE_PsIfaceUp(VOS_UINT8 ucRmNetId, VOS_UINT8 ucIpFamilyMask);
VOS_VOID RNIC_IFACE_PsIfaceDown(VOS_UINT8 ucRmNetId, VOS_UINT8 ucIpFamilyMask);
VOS_VOID RNIC_IFACE_ResetPsIface(VOS_UINT8 ucRmNetId);
VOS_VOID RNIC_IFACE_TetherInfo(
    RNIC_PS_IFACE_TETHER_INFO_STRU     *pstTetherInfo
);
VOS_VOID RNIC_IFACE_SetDsFlowStats(VOS_UINT8 ucRmNetId);
VOS_VOID RNIC_IFACE_ClearDsFlowFlux(VOS_UINT8 ucRmNetId);
#if (defined(CONFIG_BALONG_SPE))
VOS_VOID RNIC_IFACE_SetSpeCfg(VOS_VOID);
#endif /* CONFIG_BALONG_SPE */
#if (defined(CONFIG_BALONG_ESPE))
VOS_VOID RNIC_IFACE_SetEspePortConfig(VOS_VOID);
#endif /* CONFIG_BALONG_ESPE */
VOS_VOID RNIC_IFACE_ClearDsFlowStats(VOS_UINT8 ucRmNetId);
VOS_VOID RNIC_IFACE_RegTxDropCB(VOS_UINT8 ucRmNetId);
VOS_VOID RNIC_IFACE_RegDevNotifyCB(VOS_VOID);
VOS_VOID RNIC_IFACE_RegOnDemandOpsCB(VOS_VOID);
VOS_VOID RNIC_IFACE_NetdevNotify(VOS_VOID);



#if (VOS_OS_VER != VOS_WIN32)
#pragma pack()
#else
#pragma pack(pop)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __RNIC_IFACE_CFG_H__ */

