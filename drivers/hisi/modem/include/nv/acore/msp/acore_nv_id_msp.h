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



#ifndef __MSP_NV_ID_ACORE_H__
#define __MSP_NV_ID_ACORE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef enum
{
    EN_NV_ID_RSRP_CFG                   = 0xD004,
	EN_NV_ID_ECIO_CFG                   = 0xD005,
    EN_NV_ID_RSCP_CFG                   = 0xD006,
    EN_NV_ID_AUTHORITY_VER              = 0xD00A,
    EN_NV_ID_AUTHORITY_TYPE             = 0xD00B,
    EN_NV_ID_TMODE_RAT                  = 0xD0EF,
    
    NV_ID_MSP_RSRP_CFG                   = 0xD004,  //53252, NV_ID_RSRP_CFG_STRU
    NV_ID_MSP_ECIO_CFG                   = 0xD005,  //53253, NV_ID_ECIO_CFG_STRU
    NV_ID_MSP_RSCP_CFG                   = 0xD006,  //53254, NV_ID_RSCP_CFG_STRU
    NV_ID_MSP_AUTHORITY_VER              = 0xD009,  //53257, NV_MSP_AUTH_VER_STRU
    NV_ID_MSP_AUTHORITY_TYPE             = 0xD00a,  //53258, NV_MSP_AUTH_TYPE_STRU
    NV_ID_MSP_TMODE_RAT                  = 0xD0EF,  //53487, FTM_TMODE_RAT_NV_STRU

    EN_NV_ID_ACORE_END    = 0XFFFF
}NV_ID_MSP_ACORE_ENUM;

/*******************************NV ID END**************************************/
typedef enum
{
    NV_ID_ErrLogGutlCtrlInfo = 1001,
    NV_ID_ErrLogCdmaCtrlInfo = 1049,
    NV_ID_CHR_END    = 0XFFFF
}NV_ID_MSP_CHR_ENUM;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
