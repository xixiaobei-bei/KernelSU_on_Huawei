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

#ifndef PS_LOG_TRRC_FILE_ID_DEFINE_H
#define PS_LOG_TRRC_FILE_ID_DEFINE_H

#include "ps_log_file_id_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef enum
{
/* 9216 */        PS_FILE_ID_T_TRRCCCB_C = TRRC_TEAM_FILE_ID,
                  PS_FILE_ID_T_TRRCCOMM_C,
                  PS_FILE_ID_T_TRRCNASITF_C,
                  PS_FILE_ID_T_TRRCOM_C,
/* 9220 */        PS_FILE_ID_T_TRRCMEAS_C,
                  PS_FILE_ID_T_TRRCIRATCOMM_C,
                  PS_FILE_ID_T_RRCCMMDCHBSICMEAS_C,
                  PS_FILE_ID_T_RRCCMMDCHEUTRAMEAS_C,
                  PS_FILE_ID_T_ASN01_C,
/* 9225 */        PS_FILE_ID_T_ASN02_C,
                  PS_FILE_ID_T_ASN03_C,
                  PS_FILE_ID_T_ASN04_C,
                  PS_FILE_ID_T_ASN05_C,
                  PS_FILE_ID_T_ASN06_C,
/* 9230 */        PS_FILE_ID_T_ASN07_C,
                  PS_FILE_ID_T_ASN08_C,
                  PS_FILE_ID_T_ASN10_C,
                  PS_FILE_ID_T_ASN11_C,
                  PS_FILE_ID_T_ASN12_C,
/* 9235 */        PS_FILE_ID_T_ASN13_C,
                  PS_FILE_ID_T_ASN14_C,
                  PS_FILE_ID_T_ASN15_C,
                  PS_FILE_ID_T_ASN16_C,
                  PS_FILE_ID_T_ASN17_C,
/* 9240 */        PS_FILE_ID_T_ASN20MEM_C,
                  PS_FILE_ID_T_ASN21TRA_C,
                  PS_FILE_ID_T_ASN30_C,
                  PS_FILE_ID_T_ASN31_C,
                  PS_FILE_ID_T_ASN32_C,
/* 9245 */        PS_FILE_ID_T_ASN33_C,
                  PS_FILE_ID_T_ASN34_C,
                  PS_FILE_ID_T_ASN35_C,
                  PS_FILE_ID_T_ASN36_C,
                  PS_FILE_ID_T_ASN37_C,
/* 9250 */        PS_FILE_ID_T_RRC00COM_C,
                  PS_FILE_ID_T_RRC01NUL_C,
                  PS_FILE_ID_T_RRC02SEL_C,
                  PS_FILE_ID_T_RRC03IDL_C,
                  PS_FILE_ID_T_RRC04ACC_C,
/* 9255 */        PS_FILE_ID_T_RRC05WCA_C,
                  PS_FILE_ID_T_RRC06DCH_C,
                  PS_FILE_ID_T_RRC07REL_C,
                  PS_FILE_ID_T_RRC08RES_C,
                  PS_FILE_ID_T_RRC09FCH_C,
/* 9260 */        PS_FILE_ID_T_RRC10PCH_C,
                  PS_FILE_ID_T_RRC11CNF_C,
                  PS_FILE_ID_T_RRC12LIM_C,
                  PS_FILE_ID_T_RRC13SNW_C,
                  PS_FILE_ID_T_RRC14INACTIVE_C,
/* 9265 */        PS_FILE_ID_T_RRC15PREDEF_C,
                  PS_FILE_ID_T_RRC16WAIT_C,
                  PS_FILE_ID_T_RRC20PEND_C,
                  PS_FILE_ID_T_RRC21RATHO_C,
                  PS_FILE_ID_T_RRC30PUB_C,
/* 9270 */        PS_FILE_ID_T_RRC31PUB_C,
                  PS_FILE_ID_T_RRC32PUB_C,
                  PS_FILE_ID_T_RRC33PUB_C,
                  PS_FILE_ID_T_RRC34PUB_C,
                  PS_FILE_ID_T_RRC35PUB_C,
/* 9275 */        PS_FILE_ID_T_RRC36PUB_C,
                  PS_FILE_ID_T_RRC37PUB_C,
                  PS_FILE_ID_T_RRC40ASN_C,
                  PS_FILE_ID_T_RRC41PUB_C,
                  PS_FILE_ID_T_RRC42PUB_C,
/* 9280 */        PS_FILE_ID_T_RRC51PUB_C,
                  PS_FILE_ID_T_RRC61MAC_C,
                  PS_FILE_ID_T_RRC62RLC_C,
                  PS_FILE_ID_T_RRC63MM_C,
                  PS_FILE_ID_T_RRC64GMM_C,
/* 9285 */        PS_FILE_ID_T_RRC65RAB_C,
                  PS_FILE_ID_T_RRC66PDC_C,
                  PS_FILE_ID_T_RRC67SPP_C,
                  PS_FILE_ID_T_RRC68SMB_C,
                  PS_FILE_ID_T_RRC69RLP_C,
/* 9290 */        PS_FILE_ID_T_RRC70GRR_C,
                  PS_FILE_ID_T_RRC71RRC_C,
                  PS_FILE_ID_T_TRRCSIBIRATSYSINFOPROCESS_C,
                  PS_FILE_ID_T_TRRCCSELR9GLOBAL_C,
                  PS_FILE_ID_T_TRRCR9IEPROCESS_C,
/* 9295 */        PS_FILE_ID_T_TRRCEVALUATE_C,
                  PS_FILE_ID_T_TRRCLIMIT_C,
                  PS_FILE_ID_T_TRRCERRLOG_C,
                  PS_FILE_ID_T_TRRCDEBUG_C,
                  PS_FILE_ID_T_TESTACC_C,
/* 9300 */        PS_FILE_ID_T_TESTDM_C,
                  PS_FILE_ID_T_TESTIDL_C,
                  PS_FILE_ID_T_TESTRRC_C,
                  PS_FILE_ID_T_TRRCCOMMPRIVATE_C,
                  PS_FILE_ID_T_TRRCNASITFPRIVATE_C,
                  PS_FILE_ID_T_TRRCDEBUGCOMM_C,
                  PS_FILE_ID_T_TRRCDIRECTERRLOGCONVERT_C,
                 TRRC_FILE_ID_BUTT
 }TRRC_FILE_ID_DEFINE_ENUM;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif


