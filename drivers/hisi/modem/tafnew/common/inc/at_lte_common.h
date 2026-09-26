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

#ifndef __AT_LTE_COMMON_H__
#define __AT_LTE_COMMON_H__

#include "gen_msg.h"
#include "AtParse.h"
#include "app_rrc_interface.h"
#include "at_cmd_proc.h"
#include "at_lte_ms_proc.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 4)

#define AT_LTE_EVENT_NULL_SIZE 0

#define AT_RSSI_LOW (-113)
#define AT_RSSI_HIGH (-51)
#define AT_CSQ_RSSI_LOW 0
#define AT_CSQ_RSSI_HIGH 31
#define AT_RSSI_UNKNOWN 99

#define AT_BER_UNKNOWN 99

enum {
    EN_SERVICE_CELL_ID,
    EN_SYN_FREQ_CELL_ID,
    EN_ASYN_FREQ_CELL_ID,
    EN_ASYN_UMTS_CELL_ID,
    EN_ASYN_GSM_CELL_ID,
    EN_CELL_ID_BUTT
};

typedef enum {
    AT_CMD_LTE_BEGIN = AT_CMD_COMM_BUTT,
    AT_CMD_LWCLASH,
    AT_CMD_RADVER,
    AT_CMD_LCELLINFO,
    AT_CMD_LTCOMMCMD,
    AT_CMD_LCACELL,
    AT_CMD_CASCELLINFO,
    AT_CMD_LTE_BUTT
} AT_LTE_CmdIndex;

typedef struct {
    VOS_UINT32 srcError; /* USIM,NAS... */

    VOS_UINT32 atError; /* AT错误码 */

} AT_ERROR_CodeTable;

/*lint -e958 -e959 ;cause:64bit*/
typedef struct {
    VOS_UINT32         msgId;
    PFN_AT_FW_MSG_PROC cnfMsgProc;
} AT_FTM_CnfMsgProc;
/*lint +e958 +e959 ;cause:64bit*/


typedef struct {
    VOS_UINT32         msgId;
    PFN_AT_FW_IND_PROC indMsgProc;
} AT_FTM_IndMsgProc;

/* +CSQ */

typedef struct {
    VOS_UINT8 rssiValue;
    VOS_UINT8 channalQual;
} TAF_RSSI;

extern AT_SEND_DataBuffer g_atSendData;
extern VOS_UINT8         *g_atSndCodeAddr;

extern VOS_VOID CmdErrProc(VOS_UINT8 clientId, VOS_UINT32 errCode, VOS_UINT16 bufLen, VOS_UINT8 *buf);

/* AT模块给FTM 模块发送消息 */

VOS_UINT32 atSendFtmDataMsg(VOS_UINT32 taskId, VOS_UINT32 msgId, VOS_UINT32 clientId, const VOS_UINT8 *data,
                                   VOS_UINT32 uLen);

extern VOS_UINT32 atSendL4aDataMsg(MN_CLIENT_ID_T clientId, VOS_UINT32 taskId, VOS_UINT32 msgId, const VOS_UINT8 *data,
                                   VOS_UINT32 uLen);

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif /* __AT_LTE_COMMON_H__ */
