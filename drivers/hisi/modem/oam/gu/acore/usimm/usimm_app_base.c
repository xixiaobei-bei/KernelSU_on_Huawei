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

/*
 * 版权所有（c）华为技术有限公司 2001-2019
 * 功能描述: 该C文件给出了---Base模块实现
 * 生成日期: 2008年5月15日
 */

#include "vos.h"
#include "pcsc_proc.h"
#include "pih_external_api.h"

#if (FEATURE_PHONE_USIM == FEATURE_OFF)
#include "si_pb.h"
#endif


#define THIS_FILE_ID PS_FILE_ID_USIMM_APP_BASE_C

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON)
#define ACPU_USIMM_TASK_PRIORITY    VOS_PRIORITY_M5
#else
#define ACPU_USIMM_TASK_PRIORITY    VOS_PRIORITY_M2
#endif

#pragma pack(push, 4)

/*
 * 功能描述: ACPU OM FID' initializtion function
 * 修改历史:
 * 1. 日    期: 2011年7月1日
 *    修改内容: 创建
 */
VOS_UINT32 USIMM_APP_FID_Init(enum VOS_InitPhaseDefine ip)
{
    VOS_UINT32 rslt;


    switch (ip) {
        case VOS_IP_LOAD_CONFIG: {
#if ((FEATURE_PHONE_USIM == FEATURE_OFF) && (FEATURE_ACORE_MODULE_TO_CCORE != FEATURE_ON))
            rslt = VOS_RegisterPIDInfo(ACPU_PID_PB, (InitFunType)WuepsPBPidInit, (MsgFunType)I0_SI_PB_PidMsgProc);
            if (rslt != VOS_OK) {
                return VOS_ERR;
            }
#endif
            rslt = VOS_RegisterPIDInfo(ACPU_PID_PCSC, VOS_NULL_PTR, (MsgFunType)PCSC_AcpuMsgProc);
            if (rslt != VOS_OK) {
                return VOS_ERR;
            }

            rslt = VOS_RegisterMsgTaskPrio(ACPU_FID_USIMM, ACPU_USIMM_TASK_PRIORITY);
            if (rslt != VOS_OK) {
                return VOS_ERR;
            }

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON)
            if (VOS_RegisterFidTaskCoreBind(ACPU_FID_USIMM, VOS_CORE_MASK_CORE0) != VOS_OK) {
                return VOS_ERR;
            }
#endif

            SI_PIH_AcpuInit();

            break;
        }

        default:
            break;
    }
    return VOS_OK;
}

#pragma pack(pop)

