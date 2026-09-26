/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
#ifndef _AT_DEVICE_PHY_RSLT_PROC_H_
#define _AT_DEVICE_PHY_RSLT_PROC_H_

#include "vos.h"
#include "at_phy_interface.h"
#include "taf_drv_agent.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

VOS_UINT32 AT_RcvDrvAgentQryProdtypeRsp(struct MsgCB *msg);
VOS_UINT16 AT_RficDieIDOut(VOS_UINT8 *msg, VOS_UINT32 rficNum, VOS_UINT16 length, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaRficDieIDQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT16 AT_RffeDieIDOut(VOS_UINT8 *msg, VOS_UINT32 rffeNum, VOS_UINT16 length, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaRffeDieIDQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 atQryLTCommCmdParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock);
VOS_UINT32 atSetLTCommCmdParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock);
VOS_UINT32 atSetFWAVEParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock);
#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 atSetFCHANParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock);
VOS_UINT32 atQryFCHANParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock);
VOS_UINT32 AT_RcvMtaLteCaCellExQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaCACellQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif
VOS_UINT32 atSetTselrfParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock);
VOS_UINT32 atSetFTXONParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock);
VOS_UINT32 atQryFTXONParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock);
VOS_VOID At_RfFpowdetTCnfProc(PHY_AT_PowerDetCnf *msg);
VOS_UINT32 At_SaveRxDivPara(VOS_UINT16 setDivBands, VOS_UINT8 rxDivCfg);

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
VOS_VOID   At_RfCfgCnfReturnSuccProc(VOS_UINT8 indexNum);
VOS_UINT32 At_RfCfgCnfReturnErrProc(VOS_UINT8 indexNum);
VOS_VOID At_HpaRfCfgCnfProc(HPA_AT_RfCfgCnf *msg);
VOS_VOID   At_CHpaRfCfgCnfProc(CHPA_AT_RfCfgCnf *msg);
VOS_UINT32 AT_SetFchanRspErr(DRV_AGENT_FchanSetErrorUint32 result);
VOS_UINT32 AT_RcvDrvAgentSetFchanRsp(struct MsgCB *msg);
VOS_UINT32 AT_RcvDrvAgentTseLrfSetRsp(struct MsgCB *msg);
VOS_UINT32 At_MipiRdCnfProc(HPA_AT_MipiRdCnf *msg);
VOS_UINT32 At_MipiWrCnfProc(HPA_AT_MipiWrCnf *msg);
VOS_UINT32 At_SsiWrCnfProc(HPA_AT_SsiWrCnf *msg);
VOS_UINT32 AT_RcvMtaPowerDetQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
#else
VOS_UINT32 AT_ProcSetWorkModeCnf(struct MsgCB *msg);
VOS_UINT32 AT_RcvBbicCalSetDpdtCnf(struct MsgCB *msg);
VOS_UINT32 At_RcvBbicCalDcxoCnf(struct MsgCB *msg);
VOS_UINT32 AT_RcvBbicCalQryFtemprptCnf(struct MsgCB *msg);
VOS_UINT32 AT_ProcRxCnf(struct MsgCB *msg);
VOS_UINT32 AT_RcvBbicCalMipiRedCnf(struct MsgCB *msg);
VOS_UINT32 AT_RcvBbicCalMipiWriteCnf(struct MsgCB *msg);
VOS_UINT32 AT_ProcTxCnf(struct MsgCB *msg);
VOS_UINT32 AT_ProcPowerDetCnf(struct MsgCB *msg);
VOS_UINT32 AT_ProcPowerDetCnf_ModulateWave(struct MsgCB *msg);
VOS_UINT32 AT_ProcGsmTxCnf_ModulateWave(struct MsgCB *msg);
VOS_UINT32 At_RcvBbicRssiInd(struct MsgCB *msg);
VOS_UINT32 At_RcvBbicPllStatusCnf(struct MsgCB *msg);
VOS_UINT32 At_ProcUeCbtRfIcMemTestCnf(struct MsgCB *msg);
VOS_UINT32 At_ProcFessFlpmNasSerdesAgingCnf(struct MsgCB *msg);
VOS_UINT32 AT_ProcFessFlpmNasSerdesSltCnf(struct MsgCB *msg);
VOS_UINT32 At_ProcFessFlpmNasSerdesNobizCnf(struct MsgCB *rcvMsg);
#endif

VOS_UINT32 atLcacellCnfProc(struct MsgCB *msgBlock);
VOS_UINT32 atSetRadverCnfProc(struct MsgCB *msgBlock);
VOS_UINT32 AT_RcvMtaSetMipiReadCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 atQryFRSSIParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock);
VOS_UINT32 atQryFPllStatusParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock);
VOS_VOID At_RfPllStatusCnfProc(PHY_AT_RfPllStatusCnf *msg);
VOS_UINT32 atSetFRXONParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock);
VOS_UINT32 atQryFRXONParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
