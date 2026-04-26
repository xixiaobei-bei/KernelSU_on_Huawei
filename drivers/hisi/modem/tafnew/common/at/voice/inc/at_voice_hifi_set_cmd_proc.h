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
#ifndef __ATVOICEHIFISETCMDPROC_H__
#define __ATVOICEHIFISETCMDPROC_H__

#include "at_ctx.h"
#include "AtParse.h"
#include "at_mn_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#if (FEATURE_MBB_CUST == FEATURE_ON)
#define SIO_CLOCK_DELAY_TIME 500
#define B_CALL_ACTIVE 1
#define B_CALL_INACTIVE 0
#endif
#define AT_CMD_CLVL_TELEMATIC_MAX_PARA 12
#define AT_CMD_CLVL_NON_TELEMATIC_MAX_PARA 5

#if (FEATURE_ECALL == FEATURE_ON)
VOS_UINT32 AT_SetEclpushPara(VOS_UINT8 indexNum);
#endif
VOS_UINT32 At_CheckClvlPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetClvlPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetVMSETPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCmutPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetModemLoopPara(VOS_UINT8 indexNum);
#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_UINT32 AT_SetSpeakerMutePara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetCmicPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetEchoSuppressionPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetPcmFrPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetEchoSwitchPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCpcmPara(VOS_UINT8 indexNum);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
