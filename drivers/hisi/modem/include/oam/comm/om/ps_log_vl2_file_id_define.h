/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

#ifndef PS_LOG_VL2_FILE_ID_DEFINE_H
#define PS_LOG_VL2_FILE_ID_DEFINE_H

#include "ps_log_file_id_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef enum
{
    PS_FILE_ID_VMAC_OM_ITF_C       = VL2_TEAM_FILE_ID + 1, /* 29697 */
    PS_FILE_ID_VPDCP_OM_ITF_C      = VL2_TEAM_FILE_ID + 2, /* 29698 */
    PS_FILE_ID_VUP_TIMER_C         = VL2_TEAM_FILE_ID + 3, /* 29699 */
    PS_FILE_ID_VEQUIP_ITF_C        = VL2_TEAM_FILE_ID + 4, /* 29700 */
    PS_FILE_ID_VUP_APP_STUB_C      = VL2_TEAM_FILE_ID + 5, /* 29701 */
    PS_FILE_ID_VUP_OM_ITF_C        = VL2_TEAM_FILE_ID + 6, /* 29702 */
    PS_FILE_ID_VPDCP_DL_PROC_C     = VL2_TEAM_FILE_ID + 7, /* 29703 */
    PS_FILE_ID_VPDCP_ENTRY_C       = VL2_TEAM_FILE_ID + 8, /* 29704 */
    PS_FILE_ID_VPDCP_SECURITY_C    = VL2_TEAM_FILE_ID + 9, /* 29705 */
    PS_FILE_ID_VMAC_SEND_C         = VL2_TEAM_FILE_ID + 10, /* 29706 */
    PS_FILE_ID_VMAC_DL_COM_C       = VL2_TEAM_FILE_ID + 11, /* 29707 */
    PS_FILE_ID_VMAC_DEBUG_C        = VL2_TEAM_FILE_ID + 12, /* 29708 */
    PS_FILE_ID_VMAC_OM_RPT_C       = VL2_TEAM_FILE_ID + 13, /* 29709 */
    PS_FILE_ID_VMAC_RLC_UL_ENTRY_C = VL2_TEAM_FILE_ID + 14, /* 29710 */
    PS_FILE_ID_VMAC_UL_SCH_C       = VL2_TEAM_FILE_ID + 15, /* 29711 */
    PS_FILE_ID_VMAC_SL_PROC_C      = VL2_TEAM_FILE_ID + 16, /* 29712 */
    PS_FILE_ID_VMAC_UL_SLBCH_C     = VL2_TEAM_FILE_ID + 17, /* 29713 */
    PS_FILE_ID_VMAC_CBR_C          = VL2_TEAM_FILE_ID + 18, /* 29714 */
    PS_FILE_ID_VMAC_LCH_MNG_C      = VL2_TEAM_FILE_ID + 19, /* 29715 */
    PS_FILE_ID_VMAC_RLC_DL_ENTRY_C = VL2_TEAM_FILE_ID + 20, /* 29716 */
    PS_FILE_ID_VMAC_UL_COM_C       = VL2_TEAM_FILE_ID + 21, /* 29717 */
    PS_FILE_ID_VMAC_DL_SCH_C       = VL2_TEAM_FILE_ID + 22, /* 29718 */
    PS_FILE_ID_VMAC_ENTITY_C       = VL2_TEAM_FILE_ID + 23, /* 29719 */
    PS_FILE_ID_VRLC_PDCP_INTF_C    = VL2_TEAM_FILE_ID + 24, /* 29720 */
    PS_FILE_ID_VRLC_UM_RECV_C      = VL2_TEAM_FILE_ID + 25, /* 29721 */
    PS_FILE_ID_VRLC_COMM_C         = VL2_TEAM_FILE_ID + 26, /* 29722 */
    PS_FILE_ID_VRLC_UL_CONFIG_C    = VL2_TEAM_FILE_ID + 27, /* 29723 */
    PS_FILE_ID_VRLC_DL_CONFIG_C    = VL2_TEAM_FILE_ID + 28, /* 29724 */
    PS_FILE_ID_VRLC_MAC_INTF_C     = VL2_TEAM_FILE_ID + 29, /* 29725 */
    PS_FILE_ID_VMAC_PM_C           = VL2_TEAM_FILE_ID + 30, /* 29726 */
    PS_FILE_ID_VL2_BUTT
} VL2_FILE_ID_DEFINE_ENUM;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif


