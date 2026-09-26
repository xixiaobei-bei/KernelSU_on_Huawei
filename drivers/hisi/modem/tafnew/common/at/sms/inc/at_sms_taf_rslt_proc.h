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
#ifndef __ATSMSTAFRSLTPROC_H__
#define __ATSMSTAFRSLTPROC_H__
#include "at_ctx.h"
#include "AtParse.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)
#define AT_MSG_MAX_MSG_SEGMENT_NUM 2

/* 23040 9.2.3.1 bits no 0 and 1 of the first octet of all PDUs */
#define AT_GET_MSG_TP_MTI(ucMti, ucFo) ((ucMti) = ((ucFo) & AT_MSG_TP_MTI_MASK))

#define AT_GET_MSG_TP_RD(bFlag, fo) ((bFlag) = ((fo) & 0x04) ? TAF_TRUE : TAF_FALSE)
/* 23040 9.2.3.2 bit no 2 of the first octet of SMS DELIVER and SMS STATUS REPORT */
#define AT_GET_MSG_TP_MMS(bFlag, fo) ((bFlag) = ((fo) & 0x04) ? TAF_FALSE : TAF_TRUE)

/* 23040 9.2.3.26 bit no. 5 of the first octet of SMS STATUS REPORT */
#define AT_GET_MSG_TP_SRQ(bFlag, fo) ((bFlag) = ((fo) & 0x20) ? TAF_TRUE : TAF_FALSE)

/* 23040 9.2.3.4 bit no. 5 of the first octet of SMS DELIVER */
#define AT_GET_MSG_TP_SRI(bFlag, fo) ((bFlag) = ((fo) & 0x20) ? TAF_TRUE : TAF_FALSE)

/* 23040 9.2.3.5 bit no. 5 of the first octet of SMS SUBMIT and SMS COMMAND */
#define AT_GET_MSG_TP_SRR(bFlag, fo) ((bFlag) = ((fo) & 0x20) ? TAF_TRUE : TAF_FALSE)

#define AT_GET_MSG_TP_UDHI(bFlag, fo) ((bFlag) = ((fo) & 0x40) ? TAF_TRUE : TAF_FALSE)

#define AT_GET_MSG_TP_RP(bFlag, fo) ((bFlag) = ((fo) & 0x80) ? TAF_TRUE : TAF_FALSE)

extern VOS_UINT32 At_SmsProc(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len);
VOS_VOID AT_ProcSmsStream(VOS_UINT8 clientId, VOS_UINT8 *data, VOS_UINT16 length, AT_ParseContext *clientCxt);
VOS_UINT32 AT_RcvMtaSmsDomainQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaSmsNasCapQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_ProcMnCallBackCmdCnfCscaRead(TAF_UINT8 indexNum, TAF_UINT32 errorCode);
VOS_UINT32 AT_ProcMnCallBackCmdCnfCpmsCmd(TAF_UINT8 indexNum, TAF_UINT32 errorCode);
TAF_UINT32 At_CmdCmgdMsgProc(TAF_UINT8 indexNum, TAF_UINT32 errorCode);
VOS_VOID At_SendMsgFoAttr(VOS_UINT8 indexNum, MN_MSG_TsDataInfo *tsDataInfo);
extern TAF_UINT32 At_GetScaFromInputStr(const TAF_UINT8 *addr, MN_MSG_BcdAddr *bcdAddr, TAF_UINT32 *len);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
