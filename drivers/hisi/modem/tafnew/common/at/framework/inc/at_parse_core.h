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

#ifndef __ATPARSECORE_H__
#define __ATPARSECORE_H__

#include "vos.h"
#include "hi_list.h"
#include "at_data_proc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define AT_CMD_MIN_LEN 3

extern VOS_UINT8            AT_GetParaIndex(VOS_VOID);
extern AT_ParseParaType    *AT_GetParaList(VOS_UINT8 index);
extern AT_ParseContext     *AT_GetParseContext(AT_ClientIdUint16 index);
extern VOS_UINT8            AT_BlockCmdCheck(VOS_VOID);
extern VOS_VOID             AT_PendClientProc(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len);
extern VOS_VOID             AT_HoldBlockCmd(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len);
extern VOS_UINT32           AT_ParseCmdIsComb(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len);
extern VOS_UINT32           AT_ParseCmdIsPend(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len);
extern VOS_VOID             AT_ResetParseVariable(VOS_VOID);
extern VOS_UINT32           At_MatchSmsCmdName(VOS_UINT8 indexNum, VOS_CHAR *pszCmdName);
#if (FEATURE_AT_CMD_TRUST_LIST == FEATURE_ON)
#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
extern VOS_UINT32 AT_MatchUserCustomCmdName(VOS_UINT8 indexNum, VOS_CHAR *cmdName);
#endif
extern VOS_UINT32 AT_MatchTrustListCmdName(VOS_CHAR *cmdName, VOS_UINT16 cmdNameLen);
#endif
extern VOS_UINT32           ParseParam(AT_ParCmdElement *cmdElement);
extern AT_RreturnCodeUint32 fwCmdTestProc(VOS_UINT8 indexNum, AT_ParCmdElement *cmdElement);
extern VOS_UINT32 LimitedCmdProc(VOS_UINT8 clientId, VOS_UINT8 *data, VOS_UINT16 len, AT_ParCmdElement *cmdElement);
extern VOS_VOID   RepeatCmdProc(AT_ParseContext *clientContext);
extern VOS_VOID   SaveRepeatCmd(AT_ParseContext *clientContext, VOS_UINT8 *data, VOS_UINT16 len);
extern VOS_UINT32 ScanDelChar(VOS_UINT8 *data, VOS_UINT16 *len, VOS_UINT8 atS5);
extern VOS_UINT32 ScanCtlChar(VOS_UINT8 *data, VOS_UINT16 *len);
extern VOS_UINT32 ScanBlankChar(VOS_UINT8 *data, VOS_UINT16 *len);
extern VOS_UINT32 FormatCmdStr(VOS_UINT8 *data, VOS_UINT16 *len, VOS_UINT8 atS3);
extern VOS_UINT32 CmdStringFormat(VOS_UINT8 clientId, VOS_UINT8 *data, VOS_UINT16 *len);
extern VOS_VOID   atCmdMsgProc(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT32 dataLength, VOS_UINT16 len);
extern VOS_UINT32 AT_DiscardInvalidChar(VOS_UINT8 *data, VOS_UINT32 dataLength, VOS_UINT16 *len);

extern VOS_VOID atnlproxy_wakeup_tsk(VOS_VOID);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of at_parse_core.h */
