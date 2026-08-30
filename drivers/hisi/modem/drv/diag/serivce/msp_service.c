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




#include <product_config.h>
#include <mdrv.h>
#include <securec.h>
#include <osl_malloc.h>
#include <bsp_diag_frame.h>
#include <bsp_diag.h>
#include "msp_service.h"
#include "diag_service.h"
#include "diag_system_debug.h"

#define  THIS_MODU mod_diag

struct MSP_SERVICE_TABLE g_astMspService[DIAG_FRAME_MSP_SID_BUTT] = {
    {DIAG_FRAME_MSP_SID_DEFAULT,           NULL},
    {DIAG_FRAME_MSP_SID_AT_SERVICE,        NULL},
    {DIAG_FRAME_MSP_SID_DIAG_SERVICE,      NULL},
    {DIAG_FRAME_MSP_SID_DATA_SERVICE,      NULL},
    {DIAG_FRAME_MSP_SID_NV_SERVICE,        NULL},
    {DIAG_FRAME_MSP_SID_USIM_SERVICE,      NULL},
    {DIAG_FRAME_MSP_SID_DM_SERVICE,        NULL},
    {DIAG_FRAME_MSP_SID_CBT_SERVICE,       NULL}
};

u32 msp_ServiceProc(SOCP_DECODER_DST_ENUM_U32 enChanID,u8 *pucData, u32 ulSize,u8 *pucRBData, u32 ulRBSize)
{
    u32 ulRet = ERR_MSP_INVALID_PARAMETER;
    u32 ulTotalLen = 0;
    u8* pData;
    u32 ulSID = 0;
    diag_service_head_stru *pHeader;

    if(pucData == NULL)
    {
        diag_PTR(EN_DIAG_PTR_MSP_SERVICE_ERR1,  0, 0, 0);
        return ERR_MSP_INVALID_PARAMETER;
    }

    diag_PTR(EN_DIAG_PTR_MSP_SERVICE_1, 1, 0, 0);

    /*入参检查*/
    ulTotalLen = ulSize + ulRBSize;
    if(!ulTotalLen)
    {
        diag_PTR(EN_DIAG_PTR_MSP_SERVICE_ERR2, 1, 0, 0);
        return ERR_MSP_INVALID_PARAMETER;
    }

    pData = osl_malloc(ulTotalLen);
    if(pData == NULL)
    {
        diag_PTR(EN_DIAG_PTR_MSP_SERVICE_ERR3, 1, 0, 0);
        return ERR_MSP_MALLOC_FAILUE;
    }

    memcpy_s(pData, ulTotalLen, pucData, ulSize);

    /*回卷指针可能为空*/
    if((NULL != pucRBData)&&(0 != ulRBSize))
    {
        memcpy_s(pData + ulSize, ulTotalLen - ulSize, pucRBData, ulRBSize);
    }

    diag_PTR(EN_DIAG_PTR_MSP_SERVICE_2, 1, 0, 0);

    /*消息数据大小必须要大于service头长度*/
    if( ulTotalLen < sizeof(diag_service_head_stru))
    {
        osl_free(pData);
        diag_error("msg len is smaller than service header, msglen:0x%x\n", ulTotalLen);
        return ERR_MSP_INVALID_PARAMETER;
    }

    ulSID = SERVICE_HEAD_SID(pData);
    pHeader = (diag_service_head_stru *)pData;

    if(ulSID < DIAG_FRAME_MSP_SID_BUTT)
    {
        if(g_astMspService[ulSID].fnService)
        {
            /* coverity[tainted_data] */
            ulRet = g_astMspService[ulSID].fnService(pHeader, ulTotalLen);
        }
        else
        {
            diag_error("sid:0x%x is error\n", ulSID);
        }
    }
    else
    {
        diag_error("sid:0x%x is too large\n", ulSID);
    }

    osl_free(pData);

    return ulRet;
}

void msp_ServiceInit(void)
{
    u32 ret = ERR_MSP_SUCCESS;
    /* register to SCM */
    ret = mdrv_SCM_RegDecoderDestProc(SOCP_DECODER_DST_LOM,(SCM_DECODERDESTFUCN)msp_ServiceProc);
    if(ret != ERR_MSP_SUCCESS)
    {
    }
    return ;
}


void msp_ServiceProcReg(diag_frame_sid_type ulType, MSP_SERVICE_FUNC pServiceFn)
{
    /* coverity[cond_at_most] */
    if(ulType >= DIAG_FRAME_MSP_SID_BUTT)
    {
        return;
    }

    g_astMspService[ulType].fnService = pServiceFn;
}




