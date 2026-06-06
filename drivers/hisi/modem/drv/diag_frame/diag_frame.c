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

#include <linux/slab.h>
#include <securec.h>
#include "bsp_slice.h"
#include "bsp_diag_frame.h"


//#define HISI_HEADER_MAGIC               (0x48495349) /*HISI*/
void bsp_diag_fill_socp_head(diag_socp_head_stru * socp_packet, u32 len)
{
    socp_packet->u32HisiMagic   = DIAG_SOCP_HEAD_MAGIC;
    socp_packet->u32DataLen     = len;

    return;
}


void bsp_diag_frame_head_init(diag_frame_head_stru* diag_frame)
{
    u64   auctime = 0;

    diag_frame->stService.sid8b       = DIAG_FRAME_MSP_SID_DIAG_SERVICE;
    diag_frame->stService.mdmid3b     = 0;
    diag_frame->stService.rsv1b       = 0;
    diag_frame->stService.ssid4b      = DIAG_FRAME_SSID_APP_CPU;
    diag_frame->stService.sessionid8b = DIAG_FRAME_MSP_SERVICE_SESSION_ID;
    diag_frame->stService.mt2b        = DIAG_FRAME_MT_IND;
    diag_frame->stService.index4b     = 0;
    diag_frame->stService.eof1b       = 0;
    diag_frame->stService.ff1b        = 0;
    diag_frame->stService.MsgTransId  = 0;

    bsp_slice_getcurtime(&auctime);
    memcpy_s(diag_frame->stService.aucTimeStamp,sizeof(diag_frame->stService.aucTimeStamp), &auctime, sizeof(diag_frame->stService.aucTimeStamp));
    diag_frame->stID.cmdid19b         = 0;
    diag_frame->stID.sec5b            = 0;
    diag_frame->stID.mode4b           = 0;
    diag_frame->stID.pri4b            = 0;
    diag_frame->u32MsgLen             = 0;
    return;
}
