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

#ifndef PS_LOG_NRNAS_FILE_ID_DEFINE_H
#define PS_LOG_NRNAS_FILE_ID_DEFINE_H

#include "ps_log_file_id_base.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef enum
{
/* NRMM BEGIN */
/* api */
/* 28672 */    PS_FILE_ID_NAS_NRMM_NVIM_READ_C                              = NRNAS_TEAM_FILE_ID,
/* 28673 */    PS_FILE_ID_NAS_NRMM_NVIM_WRITE_C                             = NRNAS_TEAM_FILE_ID + 1,
/* 28674 */    PS_FILE_ID_NAS_NRMM_ACCESS_CTRL_C                            = NRNAS_TEAM_FILE_ID + 2,
/* 28675 */    PS_FILE_ID_NAS_NRMM_CIPHER_API_C                             = NRNAS_TEAM_FILE_ID + 3,
/* 28676 */    PS_FILE_ID_NAS_NRMM_ESCAPE_C                                 = NRNAS_TEAM_FILE_ID + 4,
/* 28677 */    PS_FILE_ID_NAS_NRMM_MISC_C                                   = NRNAS_TEAM_FILE_ID + 5,
/* 28678 */    PS_FILE_ID_NAS_NRMM_MSG_QUEUE_C                              = NRNAS_TEAM_FILE_ID + 6,
/* 28679 */    PS_FILE_ID_NAS_NRMM_NSSAI_C                                  = NRNAS_TEAM_FILE_ID + 7,
/* 28680 */    PS_FILE_ID_NAS_NRMM_SEC_API_C                                = NRNAS_TEAM_FILE_ID + 8,
/* 28681 */    PS_FILE_ID_NAS_NRMM_SUCI_C                                   = NRNAS_TEAM_FILE_ID + 9,
/* 28682 */    PS_FILE_ID_NAS_NRMM_TAI_C                                    = NRNAS_TEAM_FILE_ID + 10,
/* 28683 */    PS_FILE_ID_NAS_NRMM_TIMER_C                                  = NRNAS_TEAM_FILE_ID + 11,
/* 28684 */    PS_FILE_ID_NAS_NRMM_USIM_C                                   = NRNAS_TEAM_FILE_ID + 12,

/* cm */
/* 28685 */    PS_FILE_ID_NAS_NRMM_CM_AIR_MSG_CONSTRUCT_C                   = NRNAS_TEAM_FILE_ID + 13,
/* 28686 */    PS_FILE_ID_NAS_NRMM_CM_COM_FUNC_C                            = NRNAS_TEAM_FILE_ID + 14,
/* 28687 */    PS_FILE_ID_NAS_NRMM_CM_CTX_C                                 = NRNAS_TEAM_FILE_ID + 15,
/* 28688 */    PS_FILE_ID_NAS_NRMM_CM_FSM_TAB_C                             = NRNAS_TEAM_FILE_ID + 16,
/* 28689 */    PS_FILE_ID_NAS_NRMM_CM_MAIN_C                                = NRNAS_TEAM_FILE_ID + 17,
/* 28690 */    PS_FILE_ID_NAS_NRMM_CM_PROC_AIR_MSG_C                        = NRNAS_TEAM_FILE_ID + 18,
/* 28691 */    PS_FILE_ID_NAS_NRMM_CM_PROC_CDS_MSG_C                        = NRNAS_TEAM_FILE_ID + 19,
/* 28692 */    PS_FILE_ID_NAS_NRMM_CM_PROC_IMSA_MSG_C                       = NRNAS_TEAM_FILE_ID + 20,
/* 28693 */    PS_FILE_ID_NAS_NRMM_CM_PROC_LPP_MSG_C                        = NRNAS_TEAM_FILE_ID + 21,
/* 28694 */    PS_FILE_ID_NAS_NRMM_CM_PROC_NRRC_MSG_C                       = NRNAS_TEAM_FILE_ID + 22,
/* 28695 */    PS_FILE_ID_NAS_NRMM_CM_PROC_NRSM_MSG_C                       = NRNAS_TEAM_FILE_ID + 23,
/* 28696 */    PS_FILE_ID_NAS_NRMM_CM_PROC_PCF_MSG_C                        = NRNAS_TEAM_FILE_ID + 24,
/* 28697 */    PS_FILE_ID_NAS_NRMM_CM_PROC_REG_MSG_C                        = NRNAS_TEAM_FILE_ID + 25,
/* 28698 */    PS_FILE_ID_NAS_NRMM_CM_PROC_REGM_MSG_C                       = NRNAS_TEAM_FILE_ID + 26,
/* 28699 */    PS_FILE_ID_NAS_NRMM_CM_PROC_SDAP_MSG_C                       = NRNAS_TEAM_FILE_ID + 27,
/* 28700 */    PS_FILE_ID_NAS_NRMM_CM_PROC_SEC_MSG_C                        = NRNAS_TEAM_FILE_ID + 28,
/* 28701 */    PS_FILE_ID_NAS_NRMM_CM_PROC_SMS_MSG_C                        = NRNAS_TEAM_FILE_ID + 29,
/* 28702 */    PS_FILE_ID_NAS_NRMM_CM_PROC_STK_MSG_C                        = NRNAS_TEAM_FILE_ID + 30,
/* 28703 */    PS_FILE_ID_NAS_NRMM_CM_PROC_TIMER_MSG_C                      = NRNAS_TEAM_FILE_ID + 31,

/* codec */
/* 28704 */    PS_FILE_ID_NAS_NRMM_CODEC_CFG_C                              = NRNAS_TEAM_FILE_ID + 32,
/* 28705 */    PS_FILE_ID_NAS_NRMM_DECODE_C                                 = NRNAS_TEAM_FILE_ID + 33,
/* 28706 */    PS_FILE_ID_NAS_NRMM_DECODE_CM_MSG_C                          = NRNAS_TEAM_FILE_ID + 34,
/* 28707 */    PS_FILE_ID_NAS_NRMM_DECODE_COM_IE_C                          = NRNAS_TEAM_FILE_ID + 35,
/* 28708 */    PS_FILE_ID_NAS_NRMM_DECODE_COMMOM_MSG_C                      = NRNAS_TEAM_FILE_ID + 36,
/* 28709 */    PS_FILE_ID_NAS_NRMM_DECODE_SPECIFIC_MSG_C                    = NRNAS_TEAM_FILE_ID + 37,
/* 28710 */    PS_FILE_ID_NAS_NRMM_ENCODE_C                                 = NRNAS_TEAM_FILE_ID + 38,

/* ctx */
/* 28711 */    PS_FILE_ID_NAS_NRMM_CTX_C                                    = NRNAS_TEAM_FILE_ID + 39,

/* dispatch */
/* 28712 */    PS_FILE_ID_NAS_NRMM_DISPATCH_AS_MSG_C                        = NRNAS_TEAM_FILE_ID + 40,

/* itf */
/* 28713 */    PS_FILE_ID_NAS_NRMM_SND_DMT_STUB_C                           = NRNAS_TEAM_FILE_ID + 41,
/* 28714 */    PS_FILE_ID_NAS_NRMM_SND_IMSA_C                               = NRNAS_TEAM_FILE_ID + 42,
/* 28715 */    PS_FILE_ID_NAS_NRMM_SND_LMM_C                                = NRNAS_TEAM_FILE_ID + 43,
/* 28716 */    PS_FILE_ID_NAS_NRMM_SND_LPP_C                                = NRNAS_TEAM_FILE_ID + 44,
/* 28717 */    PS_FILE_ID_NAS_NRMM_SND_LRRC_C                               = NRNAS_TEAM_FILE_ID + 45,
/* 28718 */    PS_FILE_ID_NAS_NRMM_SND_MMA_C                                = NRNAS_TEAM_FILE_ID + 46,
/* 28719 */    PS_FILE_ID_NAS_NRMM_ELTE_SND_MMC_C                           = NRNAS_TEAM_FILE_ID + 47,
/* 28720 */    PS_FILE_ID_NAS_NRMM_SND_MMC_C                                = NRNAS_TEAM_FILE_ID + 48,
/* 28721 */    PS_FILE_ID_NAS_NRMM_SND_MTA_C                                = NRNAS_TEAM_FILE_ID + 49,
/* 28722 */    PS_FILE_ID_NAS_NRMM_SND_MTC_C                                = NRNAS_TEAM_FILE_ID + 50,
/* 28723 */    PS_FILE_ID_NAS_NRMM_SND_NREAP_C                              = NRNAS_TEAM_FILE_ID + 51,
/* 28724 */    PS_FILE_ID_NAS_NRMM_SND_NRMM_C                               = NRNAS_TEAM_FILE_ID + 52,
/* 28725 */    PS_FILE_ID_NAS_NRMM_SND_NRRC_C                               = NRNAS_TEAM_FILE_ID + 53,
/* 28726 */    PS_FILE_ID_NAS_NRMM_SND_NRSM_C                               = NRNAS_TEAM_FILE_ID + 54,
/* 28727 */    PS_FILE_ID_NAS_NRMM_SND_PCF_C                                = NRNAS_TEAM_FILE_ID + 55,
/* 28728 */    PS_FILE_ID_NAS_NRMM_SND_REGM_C                               = NRNAS_TEAM_FILE_ID + 56,
/* 28729 */    PS_FILE_ID_NAS_NRMM_SND_RRM_C                                = NRNAS_TEAM_FILE_ID + 57,
/* 28730 */    PS_FILE_ID_NAS_NRMM_SND_SDAP_C                               = NRNAS_TEAM_FILE_ID + 58,
/* 28731 */    PS_FILE_ID_NAS_NRMM_SND_SMS_C                                = NRNAS_TEAM_FILE_ID + 59,
/* 28732 */    PS_FILE_ID_NAS_NRMM_SND_STK_C                                = NRNAS_TEAM_FILE_ID + 60,
/* 28733 */    PS_FILE_ID_NAS_NRMM_SND_TC_C                                 = NRNAS_TEAM_FILE_ID + 61,
/* 28734 */    PS_FILE_ID_NAS_NRMM_SND_USIM_C                               = NRNAS_TEAM_FILE_ID + 62,

/* iw */
/* 28735 */    PS_FILE_ID_NAS_NRMM_IW_CTX_C                                 = NRNAS_TEAM_FILE_ID + 63,
/* 28736 */    PS_FILE_ID_NAS_NRMM_IW_FSM_TAB_C                             = NRNAS_TEAM_FILE_ID + 64,
/* 28737 */    PS_FILE_ID_NAS_NRMM_IW_MAIN_C                                = NRNAS_TEAM_FILE_ID + 65,
/* 28738 */    PS_FILE_ID_NAS_NRMM_IW_PROC_MMC_MSG_C                        = NRNAS_TEAM_FILE_ID + 66,
/* 28739 */    PS_FILE_ID_NAS_NRMM_IW_PROC_NRRC_MSG_C                       = NRNAS_TEAM_FILE_ID + 67,
/* 28740 */    PS_FILE_ID_NAS_NRMM_IW_PROC_PWR_MSG_C                        = NRNAS_TEAM_FILE_ID + 68,

/* 28741 */    PS_FILE_ID_NAS_NRMM_MAIN_C                                   = NRNAS_TEAM_FILE_ID + 69,

/* mainctrl */
/* 28742 */    PS_FILE_ID_NAS_NRMM_MAIN_CTRL_C                              = NRNAS_TEAM_FILE_ID + 70,
/* 28743 */    PS_FILE_ID_NAS_NRMM_ELTE_MAIN_CTRL_C                         = NRNAS_TEAM_FILE_ID + 71,

/* mntn */
/* 28744 */    PS_FILE_ID_NAS_NRMM_CHR_C                                    = NRNAS_TEAM_FILE_ID + 72,
/* 28745 */    PS_FILE_ID_NAS_NRMM_DT_C                                     = NRNAS_TEAM_FILE_ID + 73,
/* 28746 */    PS_FILE_ID_NAS_NRMM_LOG_PRIVACY_C                            = NRNAS_TEAM_FILE_ID + 74,
/* 28747 */    PS_FILE_ID_NAS_NRMM_MNTN_C                                   = NRNAS_TEAM_FILE_ID + 75,

/* nws */
/* 28748 */    PS_FILE_ID_NAS_NRMM_NWS_CTX_C                                = NRNAS_TEAM_FILE_ID + 76,
/* 28749 */    PS_FILE_ID_NAS_NRMM_NWS_FSM_TAB_C                            = NRNAS_TEAM_FILE_ID + 77,
/* 28750 */    PS_FILE_ID_NAS_NRMM_NWS_MAIN_C                               = NRNAS_TEAM_FILE_ID + 78,
/* 28751 */    PS_FILE_ID_NAS_NRMM_NWS_PROC_LMM_MSG_C                       = NRNAS_TEAM_FILE_ID + 79,
/* 28752 */    PS_FILE_ID_NAS_NRMM_NWS_PROC_LRRC_MSG_C                      = NRNAS_TEAM_FILE_ID + 80,
/* 28753 */    PS_FILE_ID_NAS_NRMM_NWS_PROC_MMC_MSG_C                       = NRNAS_TEAM_FILE_ID + 81,
/* 28754 */    PS_FILE_ID_NAS_NRMM_NWS_PROC_NRRC_MSG_C                      = NRNAS_TEAM_FILE_ID + 82,
/* 28755 */    PS_FILE_ID_NAS_NRMM_NWS_PROC_REG_MSG_C                       = NRNAS_TEAM_FILE_ID + 83,

/* pwr */
/* 28756 */    PS_FILE_ID_NAS_NRMM_PWR_COM_FUNC_MSG_C                       = NRNAS_TEAM_FILE_ID + 84,
/* 28757 */    PS_FILE_ID_NAS_NRMM_PWR_CTX_C                                = NRNAS_TEAM_FILE_ID + 85,
/* 28758 */    PS_FILE_ID_NAS_NRMM_PWR_FSM_TAB_C                            = NRNAS_TEAM_FILE_ID + 86,
/* 28759 */    PS_FILE_ID_NAS_NRMM_PWR_MAIN_C                               = NRNAS_TEAM_FILE_ID + 87,
/* 28760 */    PS_FILE_ID_NAS_NRMM_PWR_PROC_LRRC_MSG_C                      = NRNAS_TEAM_FILE_ID + 88,
/* 28761 */    PS_FILE_ID_NAS_NRMM_PWR_PROC_MMC_MSG_C                       = NRNAS_TEAM_FILE_ID + 89,
/* 28762 */    PS_FILE_ID_NAS_NRMM_PWR_PROC_NRRC_MSG_C                      = NRNAS_TEAM_FILE_ID + 90,
/* 28763 */    PS_FILE_ID_NAS_NRMM_PWR_PROC_REG_MSG_C                       = NRNAS_TEAM_FILE_ID + 91,
/* 28764 */    PS_FILE_ID_NAS_NRMM_PWR_PROC_TIMER_MSG_C                     = NRNAS_TEAM_FILE_ID + 92,
/* 28765 */    PS_FILE_ID_NAS_NRMM_PWR_PROC_USIM_MSG_C                      = NRNAS_TEAM_FILE_ID + 93,

/* reg */
/* 28766 */    PS_FILE_ID_NAS_NRMM_REG_AIR_MSG_CONSTRUCT_C                  = NRNAS_TEAM_FILE_ID + 94,
/* 28767 */    PS_FILE_ID_NAS_NRMM_REG_COM_FUNC_C                           = NRNAS_TEAM_FILE_ID + 95,
/* 28768 */    PS_FILE_ID_NAS_NRMM_REG_CTX_C                                = NRNAS_TEAM_FILE_ID + 96,
/* 28769 */    PS_FILE_ID_NAS_NRMM_REG_FSM_TAB_C                            = NRNAS_TEAM_FILE_ID + 97,
/* 28770 */    PS_FILE_ID_NAS_NRMM_REG_MAIN_C                               = NRNAS_TEAM_FILE_ID + 98,
/* 28771 */    PS_FILE_ID_NAS_NRMM_REG_PROC_AIR_MSG_C                       = NRNAS_TEAM_FILE_ID + 99,
/* 28772 */    PS_FILE_ID_NAS_NRMM_REG_PROC_CM_MSG_C                        = NRNAS_TEAM_FILE_ID + 100,
/* 28773 */    PS_FILE_ID_NAS_NRMM_REG_PROC_IW_MSG_C                        = NRNAS_TEAM_FILE_ID + 101,
/* 28774 */    PS_FILE_ID_NAS_NRMM_REG_PROC_LMM_MSG_C                       = NRNAS_TEAM_FILE_ID + 102,
/* 28775 */    PS_FILE_ID_NAS_NRMM_REG_PROC_MMC_COM_MSG_C                   = NRNAS_TEAM_FILE_ID + 103,
/* 28776 */    PS_FILE_ID_NAS_NRMM_REG_PROC_MMC_ELTE_MSG_C                  = NRNAS_TEAM_FILE_ID + 104,
/* 28777 */    PS_FILE_ID_NAS_NRMM_REG_PROC_MMC_NR_MSG_C                    = NRNAS_TEAM_FILE_ID + 105,
/* 28778 */    PS_FILE_ID_NAS_NRMM_REG_PROC_MTA_MSG_C                       = NRNAS_TEAM_FILE_ID + 106,
/* 28779 */    PS_FILE_ID_NAS_NRMM_REG_PROC_NRRC_MSG_C                      = NRNAS_TEAM_FILE_ID + 107,
/* 28780 */    PS_FILE_ID_NAS_NRMM_REG_PROC_NW_CAUSE_C                      = NRNAS_TEAM_FILE_ID + 108,
/* 28781 */    PS_FILE_ID_NAS_NRMM_REG_PROC_NWS_MSG_C                       = NRNAS_TEAM_FILE_ID + 109,
/* 28782 */    PS_FILE_ID_NAS_NRMM_REG_PROC_PWR_MSG_C                       = NRNAS_TEAM_FILE_ID + 110,
/* 28783 */    PS_FILE_ID_NAS_NRMM_REG_PROC_REGM_MSG_C                      = NRNAS_TEAM_FILE_ID + 111,
/* 28784 */    PS_FILE_ID_NAS_NRMM_REG_PROC_RRM_MSG_C                       = NRNAS_TEAM_FILE_ID + 112,
/* 28785 */    PS_FILE_ID_NAS_NRMM_REG_PROC_SEC_MSG_C                       = NRNAS_TEAM_FILE_ID + 113,
/* 28786 */    PS_FILE_ID_NAS_NRMM_REG_PROC_THROLT_MSG_C                    = NRNAS_TEAM_FILE_ID + 114,
/* 28787 */    PS_FILE_ID_NAS_NRMM_REG_PROC_TIMER_MSG_C                     = NRNAS_TEAM_FILE_ID + 115,

/* sec */
/* 28788 */    PS_FILE_ID_NAS_NRMM_SEC_AIR_MSG_CONSTRUCT_C                  = NRNAS_TEAM_FILE_ID + 116,
/* 28789 */    PS_FILE_ID_NAS_NRMM_SEC_COM_FUNC_C                           = NRNAS_TEAM_FILE_ID + 117,
/* 28790 */    PS_FILE_ID_NAS_NRMM_SEC_CTX_C                                = NRNAS_TEAM_FILE_ID + 118,
/* 28791 */    PS_FILE_ID_NAS_NRMM_SEC_DISPATCH_AIR_MSG_C                   = NRNAS_TEAM_FILE_ID + 119,
/* 28792 */    PS_FILE_ID_NAS_NRMM_SEC_FSM_TAB_C                            = NRNAS_TEAM_FILE_ID + 120,
/* 28793 */    PS_FILE_ID_NAS_NRMM_SEC_MAIN_C                               = NRNAS_TEAM_FILE_ID + 121,
/* 28794 */    PS_FILE_ID_NAS_NRMM_SEC_PROC_AIR_MSG_C                       = NRNAS_TEAM_FILE_ID + 122,
/* 28795 */    PS_FILE_ID_NAS_NRMM_SEC_PROC_AS_MSG_C                        = NRNAS_TEAM_FILE_ID + 123,
/* 28796 */    PS_FILE_ID_NAS_NRMM_SEC_PROC_CM_MSG_C                        = NRNAS_TEAM_FILE_ID + 124,
/* 28797 */    PS_FILE_ID_NAS_NRMM_SEC_PROC_LMM_MSG_C                       = NRNAS_TEAM_FILE_ID + 125,
/* 28798 */    PS_FILE_ID_NAS_NRMM_SEC_PROC_LRRC_MSG_C                      = NRNAS_TEAM_FILE_ID + 126,
/* 28799 */    PS_FILE_ID_NAS_NRMM_SEC_PROC_MTA_MSG_C                       = NRNAS_TEAM_FILE_ID + 127,
/* 28800 */    PS_FILE_ID_NAS_NRMM_SEC_PROC_NREAP_MSG_C                     = NRNAS_TEAM_FILE_ID + 128,
/* 28801 */    PS_FILE_ID_NAS_NRMM_SEC_PROC_NRRC_MSG_C                      = NRNAS_TEAM_FILE_ID + 129,
/* 28802 */    PS_FILE_ID_NAS_NRMM_SEC_PROC_REG_MSG_C                       = NRNAS_TEAM_FILE_ID + 130,
/* 28803 */    PS_FILE_ID_NAS_NRMM_SEC_PROC_USIM_MSG_C                      = NRNAS_TEAM_FILE_ID + 131,
/* 28804 */    PS_FILE_ID_NAS_NRMM_SEC_PROC_TIMER_MSG_C                     = NRNAS_TEAM_FILE_ID + 132,
/* NRMM END */

/* NRSM BEGIN */
/* codec */
/* 28805 */    PS_FILE_ID_NAS_NRSM_CODEC_COM_FUNC_C                         = NRNAS_TEAM_FILE_ID + 133,
/* 28806 */    PS_FILE_ID_NAS_NRSM_DECODE_5GSM_STATUS_AIR_MSG_C             = NRNAS_TEAM_FILE_ID + 134,
/* 28807 */    PS_FILE_ID_NAS_NRSM_DECODE_BASIC_IE_C                        = NRNAS_TEAM_FILE_ID + 135,
/* 28808 */    PS_FILE_ID_NAS_NRSM_DECODE_EPCO_IE_C                         = NRNAS_TEAM_FILE_ID + 136,
/* 28809 */    PS_FILE_ID_NAS_NRSM_DECODE_IE_C                              = NRNAS_TEAM_FILE_ID + 137,
/* 28810 */    PS_FILE_ID_NAS_NRSM_DECODE_MAPPED_EPS_BEARER_IE_C            = NRNAS_TEAM_FILE_ID + 138,
/* 28811 */    PS_FILE_ID_NAS_NRSM_DECODE_PDU_AUTH_AIR_MSG_C                = NRNAS_TEAM_FILE_ID + 139,
/* 28812 */    PS_FILE_ID_NAS_NRSM_DECODE_PDU_EST_AIR_MSG_C                 = NRNAS_TEAM_FILE_ID + 140,
/* 28813 */    PS_FILE_ID_NAS_NRSM_DECODE_PDU_MOD_AIR_MSG_C                 = NRNAS_TEAM_FILE_ID + 141,
/* 28814 */    PS_FILE_ID_NAS_NRSM_DECODE_PDU_REL_AIR_MSG_C                 = NRNAS_TEAM_FILE_ID + 142,
/* 28815 */    PS_FILE_ID_NAS_NRSM_DECODE_PF_IE_C                           = NRNAS_TEAM_FILE_ID + 143,
/* 28816 */    PS_FILE_ID_NAS_NRSM_DECODE_QOS_FLOW_IE_C                     = NRNAS_TEAM_FILE_ID + 144,
/* 28817 */    PS_FILE_ID_NAS_NRSM_DECODE_QOS_RULE_IE_C                     = NRNAS_TEAM_FILE_ID + 145,
/* 28818 */    PS_FILE_ID_NAS_NRSM_ENCODE_5GSM_STATUS_AIR_MSG_C             = NRNAS_TEAM_FILE_ID + 146,
/* 28819 */    PS_FILE_ID_NAS_NRSM_ENCODE_BASIC_IE_C                        = NRNAS_TEAM_FILE_ID + 147,
/* 28820 */    PS_FILE_ID_NAS_NRSM_ENCODE_EPCO_IE_C                         = NRNAS_TEAM_FILE_ID + 148,
/* 28821 */    PS_FILE_ID_NAS_NRSM_ENCODE_PF_IE_C                           = NRNAS_TEAM_FILE_ID + 149,
/* 28822 */    PS_FILE_ID_NAS_NRSM_ENCODE_IE_C                              = NRNAS_TEAM_FILE_ID + 150,
/* 28823 */    PS_FILE_ID_NAS_NRSM_ENCODE_MAPPED_EPS_BEARER_IE_C            = NRNAS_TEAM_FILE_ID + 151,
/* 28824 */    PS_FILE_ID_NAS_NRSM_ENCODE_PDU_AUTH_AIR_MSG_C                = NRNAS_TEAM_FILE_ID + 152,
/* 28825 */    PS_FILE_ID_NAS_NRSM_ENCODE_PDU_EST_AIR_MSG_C                 = NRNAS_TEAM_FILE_ID + 153,
/* 28826 */    PS_FILE_ID_NAS_NRSM_ENCODE_PDU_MOD_AIR_MSG_C                 = NRNAS_TEAM_FILE_ID + 154,
/* 28827 */    PS_FILE_ID_NAS_NRSM_ENCODE_PDU_REL_AIR_MSG_C                 = NRNAS_TEAM_FILE_ID + 155,
/* 28828 */    PS_FILE_ID_NAS_NRSM_ENCODE_QOS_FLOW_IE_C                     = NRNAS_TEAM_FILE_ID + 156,
/* 28829 */    PS_FILE_ID_NAS_NRSM_ENCODE_QOS_RULE_IE_C                     = NRNAS_TEAM_FILE_ID + 157,

/* comm */
/* 28830 */    PS_FILE_ID_NAS_NRSM_AIR_MSG_RETRANSMIT_STRATEGY_C            = NRNAS_TEAM_FILE_ID + 158,
/* 28831 */    PS_FILE_ID_NAS_NRSM_COM_FUNC_C                               = NRNAS_TEAM_FILE_ID + 159,
/* 28832 */    PS_FILE_ID_NAS_NRSM_CTX_C                                    = NRNAS_TEAM_FILE_ID + 160,
/* 28833 */    PS_FILE_ID_NAS_NRSM_CUSTOM_CTX_C                             = NRNAS_TEAM_FILE_ID + 161,
/* 28834 */    PS_FILE_ID_NAS_NRSM_FOLLOW_ON_C                              = NRNAS_TEAM_FILE_ID + 162,
/* 28835 */    PS_FILE_ID_NAS_NRSM_FSM_CTX_C                                = NRNAS_TEAM_FILE_ID + 163,
/* 28836 */    PS_FILE_ID_NAS_NRSM_SESSION_CTX_C                            = NRNAS_TEAM_FILE_ID + 164,
/* 28837 */    PS_FILE_ID_NAS_NRSM_TIMER_MGMT_C                             = NRNAS_TEAM_FILE_ID + 165,

/* interface */
/* 28838 */    PS_FILE_ID_NAS_NRSM_PROC_NVIM_C                              = NRNAS_TEAM_FILE_ID + 166,
/* 28839 */    PS_FILE_ID_NAS_NRSM_SND_APS_C                                = NRNAS_TEAM_FILE_ID + 167,
/* 28840 */    PS_FILE_ID_NAS_NRSM_SND_EHSM_C                               = NRNAS_TEAM_FILE_ID + 168,
/* 28841 */    PS_FILE_ID_NAS_NRSM_SND_ESM_C                                = NRNAS_TEAM_FILE_ID + 169,
/* 28842 */    PS_FILE_ID_NAS_NRSM_SND_INTERNAL_MSG_C                       = NRNAS_TEAM_FILE_ID + 170,
/* 28843 */    PS_FILE_ID_NAS_NRSM_SND_MMA_C                                = NRNAS_TEAM_FILE_ID + 171,
/* 28844 */    PS_FILE_ID_NAS_NRSM_SND_MTA_C                                = NRNAS_TEAM_FILE_ID + 172,
/* 28845 */    PS_FILE_ID_NAS_NRSM_SND_NREAP_C                              = NRNAS_TEAM_FILE_ID + 173,
/* 28846 */    PS_FILE_ID_NAS_NRSM_SND_NRMM_C                               = NRNAS_TEAM_FILE_ID + 174,
/* 28847 */    PS_FILE_ID_NAS_NRSM_SND_OM_C                                 = NRNAS_TEAM_FILE_ID + 175,
/* 28848 */    PS_FILE_ID_NAS_NRSM_SND_RRM_C                                = NRNAS_TEAM_FILE_ID + 176,
/* 28849 */    PS_FILE_ID_NAS_NRSM_SND_SDAP_C                               = NRNAS_TEAM_FILE_ID + 177,
/* 28850 */    PS_FILE_ID_NAS_NRSM_SND_SM_C                                 = NRNAS_TEAM_FILE_ID + 178,
/* 28851 */    PS_FILE_ID_NAS_NRSM_SND_THROT_C                              = NRNAS_TEAM_FILE_ID + 179,

/* mntn */
/* 28852 */    PS_FILE_ID_NAS_NRSM_DIRECT_ERRLOG_C                          = NRNAS_TEAM_FILE_ID + 180,
/* 28853 */    PS_FILE_ID_NAS_NRSM_DUMP_C                                   = NRNAS_TEAM_FILE_ID + 181,
/* 28854 */    PS_FILE_ID_NAS_NRSM_ERRLOG_COM_FUNC_C                        = NRNAS_TEAM_FILE_ID + 182,
/* 28855 */    PS_FILE_ID_NAS_NRSM_MLOG_C                                   = NRNAS_TEAM_FILE_ID + 183,
/* 28856 */    PS_FILE_ID_NAS_NRSM_NON_DIRECT_ERRLOG_C                      = NRNAS_TEAM_FILE_ID + 184,
/* 28857 */    PS_FILE_ID_NAS_NRSM_LOG_PRIVACY_C                            = NRNAS_TEAM_FILE_ID + 185,

/* sessionmgnt */
/* 28858 */    PS_FILE_ID_NAS_NRSM_FSM_PDU_AUTH_C                           = NRNAS_TEAM_FILE_ID + 186,
/* 28859 */    PS_FILE_ID_NAS_NRSM_FSM_PDU_AUTH_TBL_C                       = NRNAS_TEAM_FILE_ID + 187,

/* 28860 */    PS_FILE_ID_NAS_NRSM_EPCO_COM_PROC_C                          = NRNAS_TEAM_FILE_ID + 188,
/* 28861 */    PS_FILE_ID_NAS_NRSM_GET_PDU_CTX_INDEX_LIST_C                 = NRNAS_TEAM_FILE_ID + 189,
/* 28862 */    PS_FILE_ID_NAS_NRSM_MAIN_C                                   = NRNAS_TEAM_FILE_ID + 190,
/* 28863 */    PS_FILE_ID_NAS_NRSM_MAPPED_EPS_BEARER_COM_PROC_C             = NRNAS_TEAM_FILE_ID + 191,
/* 28864 */    PS_FILE_ID_NAS_NRSM_PROC_FORBLIST_C                          = NRNAS_TEAM_FILE_ID + 192,
/* 28865 */    PS_FILE_ID_NAS_NRSM_QOS_FLOW_COM_PROC_C                      = NRNAS_TEAM_FILE_ID + 193,
/* 28866 */    PS_FILE_ID_NAS_NRSM_QOS_RULE_COM_PROC_C                      = NRNAS_TEAM_FILE_ID + 194,
/* 28867 */    PS_FILE_ID_NAS_NRSM_SESSION_COM_FUNC_C                       = NRNAS_TEAM_FILE_ID + 195,

/* 28868 */    PS_FILE_ID_NAS_NRSM_FSM_PDU_EST_C                            = NRNAS_TEAM_FILE_ID + 196,
/* 28869 */    PS_FILE_ID_NAS_NRSM_FSM_PDU_EST_COM_FUNC_C                   = NRNAS_TEAM_FILE_ID + 197,
/* 28870 */    PS_FILE_ID_NAS_NRSM_FSM_PDU_EST_PROC_AIR_MSG_C               = NRNAS_TEAM_FILE_ID + 198,
/* 28871 */    PS_FILE_ID_NAS_NRSM_FSM_PDU_EST_PROC_MAPPED_EPS_BEARER_C     = NRNAS_TEAM_FILE_ID + 199,
/* 28872 */    PS_FILE_ID_NAS_NRSM_FSM_PDU_EST_PROC_QOS_FLOW_C              = NRNAS_TEAM_FILE_ID + 200,
/* 28873 */    PS_FILE_ID_NAS_NRSM_FSM_PDU_EST_PROC_QOS_RULE_C              = NRNAS_TEAM_FILE_ID + 201,
/* 28874 */    PS_FILE_ID_NAS_NRSM_FSM_PDU_EST_TBL_C                        = NRNAS_TEAM_FILE_ID + 202,

/* 28875 */    PS_FILE_ID_NAS_NRSM_FSM_IRAT_C                               = NRNAS_TEAM_FILE_ID + 203,
/* 28876 */    PS_FILE_ID_NAS_NRSM_FSM_IRAT_TBL_C                           = NRNAS_TEAM_FILE_ID + 204,

/* 28877 */    PS_FILE_ID_NAS_NRSM_FSM_MAIN_C                               = NRNAS_TEAM_FILE_ID + 205,
/* 28878 */    PS_FILE_ID_NAS_NRSM_FSM_MAIN_TBL_C                           = NRNAS_TEAM_FILE_ID + 206,

/* 28879 */    PS_FILE_ID_NAS_NRSM_FSM_PDU_MOD_C                            = NRNAS_TEAM_FILE_ID + 207,
/* 28880 */    PS_FILE_ID_NAS_NRSM_FSM_PDU_MOD_COM_FUNC_C                   = NRNAS_TEAM_FILE_ID + 208,
/* 28881 */    PS_FILE_ID_NAS_NRSM_FSM_PDU_MOD_PROC_AIR_MSG_C               = NRNAS_TEAM_FILE_ID + 209,
/* 28882 */    PS_FILE_ID_NAS_NRSM_FSM_PDU_MOD_PROC_MAPPED_EPS_BEARER_C     = NRNAS_TEAM_FILE_ID + 210,
/* 28883 */    PS_FILE_ID_NAS_NRSM_FSM_PDU_MOD_PROC_QOS_FLOW_C              = NRNAS_TEAM_FILE_ID + 211,
/* 28884 */    PS_FILE_ID_NAS_NRSM_FSM_PDU_MOD_PROC_QOS_RULE_C              = NRNAS_TEAM_FILE_ID + 212,
/* 28885 */    PS_FILE_ID_NAS_NRSM_FSM_PDU_MOD_TBL_C                        = NRNAS_TEAM_FILE_ID + 213,

/* 28886 */    PS_FILE_ID_NAS_NRSM_MAIN_CTRL_C                              = NRNAS_TEAM_FILE_ID + 214,
/* 28887 */    PS_FILE_ID_NAS_NRSM_MAIN_CTRL_COM_FUNC_C                     = NRNAS_TEAM_FILE_ID + 215,
/* 28888 */    PS_FILE_ID_NAS_NRSM_MAIN_CTRL_PROC_APS_C                     = NRNAS_TEAM_FILE_ID + 216,
/* 28889 */    PS_FILE_ID_NAS_NRSM_MAIN_CTRL_PROC_CHR_C                     = NRNAS_TEAM_FILE_ID + 217,
/* 28890 */    PS_FILE_ID_NAS_NRSM_MAIN_CTRL_PROC_DT_C                      = NRNAS_TEAM_FILE_ID + 218,
/* 28891 */    PS_FILE_ID_NAS_NRSM_MAIN_CTRL_PROC_EHSM_C                    = NRNAS_TEAM_FILE_ID + 219,
/* 28892 */    PS_FILE_ID_NAS_NRSM_MAIN_CTRL_PROC_ESM_C                     = NRNAS_TEAM_FILE_ID + 220,
/* 28893 */    PS_FILE_ID_NAS_NRSM_MAIN_CTRL_PROC_MTA_C                     = NRNAS_TEAM_FILE_ID + 221,
/* 28894 */    PS_FILE_ID_NAS_NRSM_MAIN_CTRL_PROC_NREAP_C                   = NRNAS_TEAM_FILE_ID + 222,
/* 28895 */    PS_FILE_ID_NAS_NRSM_MAIN_CTRL_PROC_NRMM_C                    = NRNAS_TEAM_FILE_ID + 223,
/* 28896 */    PS_FILE_ID_NAS_NRSM_MAIN_CTRL_PROC_RRM_C                     = NRNAS_TEAM_FILE_ID + 224,
/* 28897 */    PS_FILE_ID_NAS_NRSM_MAIN_CTRL_PROC_SM_C                      = NRNAS_TEAM_FILE_ID + 225,
/* 28898 */    PS_FILE_ID_NAS_NRSM_MAIN_CTRL_PROC_THROT_C                   = NRNAS_TEAM_FILE_ID + 226,
/* 28899 */    PS_FILE_ID_NAS_NRSM_MAIN_CTRL_PROC_TIMER_C                   = NRNAS_TEAM_FILE_ID + 227,

/* 28900 */    PS_FILE_ID_NAS_NRSM_FSM_PDU_REL_C                            = NRNAS_TEAM_FILE_ID + 228,
/* 28901 */    PS_FILE_ID_NAS_NRSM_FSM_PDU_REL_COM_FUNC_C                   = NRNAS_TEAM_FILE_ID + 229,
/* 28902 */    PS_FILE_ID_NAS_NRSM_FSM_PDU_REL_PROC_AIR_MSG_C               = NRNAS_TEAM_FILE_ID + 230,
/* 28903 */    PS_FILE_ID_NAS_NRSM_FSM_PDU_REL_TBL_C                        = NRNAS_TEAM_FILE_ID + 231,

/* 28904 */    PS_FILE_ID_NAS_NRSM_TASK_C                                   = NRNAS_TEAM_FILE_ID + 232,
/* NSMM END */

/* PCF BEGIN */
/* 28905 */    PS_FILE_ID_NAS_PCF_COM_FUNC_C                                = NRNAS_TEAM_FILE_ID + 233,
/* 28906 */    PS_FILE_ID_NAS_PCF_CTX_C                                     = NRNAS_TEAM_FILE_ID + 234,
/* 28907 */    PS_FILE_ID_NAS_PCF_DECODE_C                                  = NRNAS_TEAM_FILE_ID + 235,
/* 28908 */    PS_FILE_ID_NAS_PCF_ENCODE_C                                  = NRNAS_TEAM_FILE_ID + 236,
/* 28909 */    PS_FILE_ID_NAS_PCF_MAIN_C                                    = NRNAS_TEAM_FILE_ID + 237,
/* 28910 */    PS_FILE_ID_NAS_PCF_MEM_C                                     = NRNAS_TEAM_FILE_ID + 238,
/* 28911 */    PS_FILE_ID_NAS_PCF_MNTN_C                                    = NRNAS_TEAM_FILE_ID + 239,
/* 28912 */    PS_FILE_ID_NAS_PCF_PROC_NVIM_C                               = NRNAS_TEAM_FILE_ID + 240,
/* 28913 */    PS_FILE_ID_NAS_PCF_SND_DSM_C                                 = NRNAS_TEAM_FILE_ID + 241,
/* 28914 */    PS_FILE_ID_NAS_PCF_SND_NRMM_C                                = NRNAS_TEAM_FILE_ID + 242,
/* 28915 */    PS_FILE_ID_NAS_PCF_URSP_MATCH_C                              = NRNAS_TEAM_FILE_ID + 243,
/* 28916 */    PS_FILE_ID_NAS_PCF_UE_POLICY_C                               = NRNAS_TEAM_FILE_ID + 244,
/* PCF END */

/* NREAP BEGIN */
/* 28917 */    PS_FILE_ID_NAS_NREAP_AES_C                                   = NRNAS_TEAM_FILE_ID + 245,
/* 28918 */    PS_FILE_ID_NAS_NREAP_AKA_C                                   = NRNAS_TEAM_FILE_ID + 246,
/* 28919 */    PS_FILE_ID_NAS_NREAP_AKA_PARSE_C                             = NRNAS_TEAM_FILE_ID + 247,
/* 28920 */    PS_FILE_ID_NAS_NREAP_AKA_PRIME_C                             = NRNAS_TEAM_FILE_ID + 248,
/* 28921 */    PS_FILE_ID_NAS_NREAP_COM_FUNC_C                              = NRNAS_TEAM_FILE_ID + 249,
/* 28922 */    PS_FILE_ID_NAS_NREAP_CTX_C                                   = NRNAS_TEAM_FILE_ID + 250,
/* 28923 */    PS_FILE_ID_NAS_NREAP_LOG_PRIVACY_C                           = NRNAS_TEAM_FILE_ID + 251,
/* 28924 */    PS_FILE_ID_NAS_NREAP_MAIN_C                                  = NRNAS_TEAM_FILE_ID + 252,
/* 28925 */    PS_FILE_ID_NAS_NREAP_MAIN_CTRL_C                             = NRNAS_TEAM_FILE_ID + 253,
/* 28926 */    PS_FILE_ID_NAS_NREAP_MD5_C                                   = NRNAS_TEAM_FILE_ID + 254,
/* 28927 */    PS_FILE_ID_NAS_NREAP_MNTN_C                                  = NRNAS_TEAM_FILE_ID + 255,
/* 28928 */    PS_FILE_ID_NAS_NREAP_PEER_C                                  = NRNAS_TEAM_FILE_ID + 256,
/* 28929 */    PS_FILE_ID_NAS_NREAP_PROC_NAS_C                              = NRNAS_TEAM_FILE_ID + 257,
/* 28930 */    PS_FILE_ID_NAS_NREAP_PROC_TIMER_C                            = NRNAS_TEAM_FILE_ID + 258,
/* 28931 */    PS_FILE_ID_NAS_NREAP_PROC_NVIM_C                             = NRNAS_TEAM_FILE_ID + 259,
/* 28932 */    PS_FILE_ID_NAS_NREAP_PROC_USIM_C                             = NRNAS_TEAM_FILE_ID + 260,
/* 28933 */    PS_FILE_ID_NAS_NREAP_SHA1_C                                  = NRNAS_TEAM_FILE_ID + 261,
/* 28934 */    PS_FILE_ID_NAS_NREAP_SHA256_C                                = NRNAS_TEAM_FILE_ID + 262,
/* 28935 */    PS_FILE_ID_NAS_NREAP_SND_NAS_C                               = NRNAS_TEAM_FILE_ID + 263,
/* 28936 */    PS_FILE_ID_NAS_NREAP_SND_USIMM_C                             = NRNAS_TEAM_FILE_ID + 264,
/* 28937 */    PS_FILE_ID_NAS_NREAP_TIMER_MGMT_C                            = NRNAS_TEAM_FILE_ID + 265,
/* NREAP END */

/* MSG CHK BEGIN */
/* 28938 */    PS_FILE_ID_NAS_NRMM_MSG_CHK_C                                = NRNAS_TEAM_FILE_ID + 266,
/* 28939 */    PS_FILE_ID_NAS_NRCM_MSG_CHK_C                                = NRNAS_TEAM_FILE_ID + 267,
/* 28940 */    PS_FILE_ID_NAS_NREAP_MSG_CHK_C                               = NRNAS_TEAM_FILE_ID + 268,
/* 28941 */    PS_FILE_ID_NRNAS_MSG_CHK_C                                   = NRNAS_TEAM_FILE_ID + 269,
/* MSG CHK BEGIN */

/* MNTN BEGIN */
/* 28942 */    PS_FILE_ID_NRNAS_MNTN_C                                      = NRNAS_TEAM_FILE_ID + 270,
/* 28943 */    PS_FILE_ID_NRNAS_MNTN_DUMP_C                                 = NRNAS_TEAM_FILE_ID + 271,
/* MNTN END */

/* ERRLOG BEGIN */
/* 28944 */    PS_FILE_ID_NRNAS_ERRLOG_CTX                                  = NRNAS_TEAM_FILE_ID + 272,
/* 28945 */    PS_FILE_ID_NRNAS_ERRLOG_REPORT                               = NRNAS_TEAM_FILE_ID + 273,
/* ERRLOG END */

/* OTHER BEGIN */
/* 28946 */    PS_FILE_ID_NAS_NR_ASN_COMMENT_CODE_TBL_C                     = NRNAS_TEAM_FILE_ID + 274,
/* 28947 */    PS_FILE_ID_NAS_NRCORE_LOG_PRIVACY_C                          = NRNAS_TEAM_FILE_ID + 275,
/* 28948 */    PS_FILE_ID_NRNAS_MD5_ADAPTER_C                               = NRNAS_TEAM_FILE_ID + 276,
/* 28949 */    PS_FILE_ID_NRNAS_PUBLIC_C                                    = NRNAS_TEAM_FILE_ID + 277,
/* OTHER END */
               PS_FILE_ID_NRNAS_BUTT
} NRNAS_FILE_ID_DEFINE_ENUM;
typedef unsigned long  NRNAS_FILE_ID_DEFINE_ENUM_UINT32;

#ifdef __cplusplus
}
#endif

#endif

