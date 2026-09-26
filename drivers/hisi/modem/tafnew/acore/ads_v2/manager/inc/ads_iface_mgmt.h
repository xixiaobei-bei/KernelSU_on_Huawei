/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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

#ifndef __ADS_PS_IFACE_MGMT_H__
#define __ADS_PS_IFACE_MGMT_H__

#include "vos.h"
#include "mdrv_eipf.h"
#include "imm_interface.h"
#include "ads_public.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define ADS_GET_FC_HEAD_BY_MODEMID(usModemId) \
    ((usModemId == MODEM_ID_0) ? EIPF_MODEM0_ULFC : ((usModemId == MODEM_ID_1) ? EIPF_MODEM1_ULFC : EIPF_MODEM2_ULFC))

/*
 * 结构说明: ADS网卡接口配置
 */
typedef struct {
    VOS_UINT8  ifaceId;        /* 网卡号 */
    VOS_UINT8  sharedIfaceId;  /* 共享网卡号 */
    VOS_UINT8  ifaceOpType;    /* 网卡操作 */
    VOS_UINT8  transMode;      /* 传输模式 */
    VOS_UINT8  addrFamilyMask; /* ip类型 */
    VOS_UINT8  pduSessionId;   /* PDU Session ID */
    VOS_UINT16 modemId;        /* Modem ID */
} ADS_IFACE_Config;

VOS_VOID ADS_IfaceConfig(ADS_IFACE_Config *psifaceCfg);

VOS_VOID ADS_ProcSpecialData(IMM_Zc *immZc);

VOS_VOID ADS_RegIfaceCB(VOS_VOID);

#if (FEATURE_PC5_DATA_CHANNEL == FEATURE_ON)
VOS_VOID ADS_LtevIfaceUp(VOS_VOID);
VOS_VOID ADS_ReportLtevUlPktHook(struct ads_tx_ltev_pkt_rec_s *ads_ltev_ul_ipk_rec);
VOS_VOID ADS_ReportLtevDlPktHook(struct ads_rx_ltev_pkt_rec_s *ads_ltev_dl_ipk_rec);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of ads_iface_mgmt.h */
