/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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
/*
 * 1 Include HeadFile
 */
#include <mdrv.h>
#include "diag_dt.h"
#include "diag_common.h"
#include "diag_msg_def.h"


#define THIS_FILE_ID MSP_FILE_ID_DIAG_DT_C

VOS_UINT32 diag_DT_HeartBeat(VOS_UINT8 *pstReq)
{
    msp_diag_frame_info_s *pstDiagHead = NULL;
    DIAG_MSG_TRANS_STRU *pstInfo = NULL;
    VOS_UINT32 data = 0x1234567;
    VOS_UINT32 ulLen;

    pstDiagHead = (msp_diag_frame_info_s *)(pstReq);

    if (pstDiagHead->ulMsgLen < sizeof(msp_diag_data_head_s) + sizeof(DIAG_CMD_DT_HB_REQ_STRU)) {
        diag_error("len error:0x%x\n", pstDiagHead->ulMsgLen);
        return ERR_MSP_FAILURE;
    }

    DIAG_MSG_ACORE_CFG_PROC(ulLen, pstDiagHead, pstInfo, data);
    DIAG_MSG_SEND_CFG_TO_NRM(ulLen, pstDiagHead, pstInfo, data);
DIAG_ERROR:
    return ERR_MSP_SUCCESS;
}

