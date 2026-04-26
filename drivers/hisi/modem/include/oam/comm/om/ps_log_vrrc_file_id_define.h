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

#ifndef PS_LOG_VRRC_FILE_ID_DEFINE_H
#define PS_LOG_VRRC_FILE_ID_DEFINE_H

#include "ps_log_file_id_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef enum
{
    PS_FILE_ID_VRRC_ENTRY_C             = VRRC_TEAM_FILE_ID + 1, /* 31745 */

    PS_FILE_ID_VRRC_LOG_C               = VRRC_TEAM_FILE_ID + 21, /* 31765 */
    PS_FILE_ID_VRRC_UTIL_C,

    PS_FILE_ID_VRRC_TIMER_C             = VRRC_TEAM_FILE_ID + 41, /* 31785 */
    PS_FILE_ID_VRRC_NV_C,
    PS_FILE_ID_VRRC_INTRA_MSG_QUEUE_C,
    PS_FILE_ID_VRRC_FSM_COMM_C,
    PS_FILE_ID_VRRC_FSM_PRINT_C,

    PS_FILE_ID_VRRC_AT_MSG_PROC_C       = VRRC_TEAM_FILE_ID + 61, /* 31805 */
    PS_FILE_ID_VRRC_MAIN_ERRC_FSM_C,
    PS_FILE_ID_VRRC_MAIN_ERRC_C,

    PS_FILE_ID_VRRC_UE_SYNC_MEAS_C     = VRRC_TEAM_FILE_ID + 81, /* 31825 */
    PS_FILE_ID_VRRC_SYNC_HO_C ,
    PS_FILE_ID_VRRC_SYNC_FSM_C,
    PS_FILE_ID_VRRC_SYNC_AT_MSG_PROC_C,
    PS_FILE_ID_VRRC_GNSS_SYNC_MEAS_C,
    PS_FILE_ID_VRRC_GNSS_DEBUG_C,

    PS_FILE_ID_VRRC_RES_POOL_C          = VRRC_TEAM_FILE_ID + 101, /* 31845 */

    PS_FILE_ID_VRRC_OM_ITF_C            = VRRC_TEAM_FILE_ID + 121, /* 31865 */
    PS_FILE_ID_VRRC_DT_ITF_C,
    PS_FILE_ID_VRRC_DUMP_C,

    PS_FILE_ID_VRRC_ITF_PHY_FUNC_C      = VRRC_TEAM_FILE_ID + 141, /* 31885 */
    PS_FILE_ID_VRRC_ITF_L2_FUNC_C,
    PS_FILE_ID_VRRC_ITF_WAIT_LIST_C,
    PS_FILE_ID_VRRC_ITF_AT_FUNC_C,

    PS_FILE_ID_VRRC_BUTT       /* 32768 */
} VRRC_FILE_ID_DEFINE_ENUM;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif


