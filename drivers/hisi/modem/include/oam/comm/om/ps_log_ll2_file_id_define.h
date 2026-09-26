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

#ifndef PS_LOG_LL2_FILE_ID_DEFINE_H
#define PS_LOG_LL2_FILE_ID_DEFINE_H

#include "ps_log_file_id_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*LL2 file id begin*/
typedef enum
{
    /*mac*/
    PS_FILE_ID_L_MACRLCDLENTRY_C                = LL2_TEAM_FILE_ID + 0,
    PS_FILE_ID_L_MACRLCULENTRY_C                = LL2_TEAM_FILE_ID + 1,
    PS_FILE_ID_L_MACULCOM_C                     = LL2_TEAM_FILE_ID + 2,
    PS_FILE_ID_L_MACENTITY_C                    = LL2_TEAM_FILE_ID + 3,
    PS_FILE_ID_L_MACULSCH_C                     = LL2_TEAM_FILE_ID + 4,
    PS_FILE_ID_L_MACCTRL_C                      = LL2_TEAM_FILE_ID + 5,
    PS_FILE_ID_L_MACRANDOM_C                    = LL2_TEAM_FILE_ID + 6,
    PS_FILE_ID_L_MACDLSCH_C                     = LL2_TEAM_FILE_ID + 7,
    PS_FILE_ID_L_MACDLCOM_C                     = LL2_TEAM_FILE_ID + 8,
    PS_FILE_ID_L_MACTIME_C                      = LL2_TEAM_FILE_ID + 9,
    PS_FILE_ID_L_MACSEND_C                      = LL2_TEAM_FILE_ID + 10,
    PS_FILE_ID_L_MACPHYPROC_C                   = LL2_TEAM_FILE_ID + 11,
    PS_FILE_ID_L_MACSTUB_C                      = LL2_TEAM_FILE_ID + 12,
    /*rlc*/
    PS_FILE_ID_L_RLC_C                          = LL2_TEAM_FILE_ID + 13,
    PS_FILE_ID_L_RLCCOMM_C                      = LL2_TEAM_FILE_ID + 14,
    PS_FILE_ID_L_RLCTIMER_C                     = LL2_TEAM_FILE_ID + 15,
    PS_FILE_ID_L_RLCMACINTERF_C                 = LL2_TEAM_FILE_ID + 16,

    PS_FILE_ID_L_RLCPDCPINTERF_C                = LL2_TEAM_FILE_ID + 17,
    PS_FILE_ID_L_RLCUMTRANS_C                   = LL2_TEAM_FILE_ID + 18,
    PS_FILE_ID_L_RLCTMRECV_C                    = LL2_TEAM_FILE_ID + 19,
    PS_FILE_ID_L_RLCUMRECV_C                    = LL2_TEAM_FILE_ID + 20,
    PS_FILE_ID_L_RLCAMRECV_C                    = LL2_TEAM_FILE_ID + 21,

    PS_FILE_ID_L_RLCAMTRANS_C                   = LL2_TEAM_FILE_ID + 22,
    PS_FILE_ID_L_RLCDLCONFIG_C                  = LL2_TEAM_FILE_ID + 23,
    PS_FILE_ID_L_RLCULCONFIG_C                  = LL2_TEAM_FILE_ID + 24,
    PS_FILE_ID_L_RLCDEBUG_C                     = LL2_TEAM_FILE_ID + 25,

    /*pdcp*/
    PS_FILE_ID_L_PDCPDLPROC_C                   = LL2_TEAM_FILE_ID + 26,
    PS_FILE_ID_L_PDCPDLINTEGRITYVERI_C          = LL2_TEAM_FILE_ID + 27,
    PS_FILE_ID_L_PDCPSECURITY_C                 = LL2_TEAM_FILE_ID + 28,
    PS_FILE_ID_L_PDCPDLUNCOMPRESS_C             = LL2_TEAM_FILE_ID + 29,
    PS_FILE_ID_L_PDCPENTRY_C                    = LL2_TEAM_FILE_ID + 30,
    PS_FILE_ID_L_PDCPULPROC_C                   = LL2_TEAM_FILE_ID + 31,
    PS_FILE_ID_L_PDCPULHCODE_C                  = LL2_TEAM_FILE_ID + 32,
    PS_FILE_ID_L_PDCPULCIPHER_C                 = LL2_TEAM_FILE_ID + 33,
    PS_FILE_ID_L_PDCPULCOMPRESS_C               = LL2_TEAM_FILE_ID + 34,
    PS_FILE_ID_L_PDCPULINTEGRITYPROT_C          = LL2_TEAM_FILE_ID + 35,
    PS_FILE_ID_L_PDCP_COMM_C                    = LL2_TEAM_FILE_ID + 36,

    /*stub files*/
    PS_FILE_ID_L_PDCPDLDEBUG_C                  = LL2_TEAM_FILE_ID + 37,
    PS_FILE_ID_L_MACDEBUG_C                     = LL2_TEAM_FILE_ID + 38,

    /*L2 loopback test*/
    PS_FILE_ID_L_PDCP_LOOPBACK_C                = LL2_TEAM_FILE_ID + 39,
    PS_FILE_ID_L_RLCMBMS_C                      = LL2_TEAM_FILE_ID + 40,

    /*added by z103912 2012-2-2 for ROHC*/
    PS_FILE_ID_L_ROHC_CONSTRUCT_PKT             = LL2_TEAM_FILE_ID + 41,
    PS_FILE_ID_L_ROHC_COMP_PROF0_C              = LL2_TEAM_FILE_ID + 42,
    PS_FILE_ID_L_ROHC_COMP_PROF1_C              = LL2_TEAM_FILE_ID + 43,
    PS_FILE_ID_L_ROHC_COMP_PROF2_C              = LL2_TEAM_FILE_ID + 44,
    PS_FILE_ID_L_ROHC_COMP_PROF3_C              = LL2_TEAM_FILE_ID + 45,
    PS_FILE_ID_L_ROHC_COMP_PROF4_C              = LL2_TEAM_FILE_ID + 46,
    PS_FILE_ID_L_ROHC_COM_C                     = LL2_TEAM_FILE_ID + 47,
    PS_FILE_ID_L_ROHC_ADAPTER_C                 = LL2_TEAM_FILE_ID + 48,
    PS_FILE_ID_L_ROHC_MEMMNG_C                  = LL2_TEAM_FILE_ID + 49,
    PS_FILE_ID_L_ROHC_FEEDBACK_PROC_C           = LL2_TEAM_FILE_ID + 50,
    PS_FILE_ID_L_ROHC_REFORM_PKT_C              = LL2_TEAM_FILE_ID + 51,
    PS_FILE_ID_L_ROHC_DECOMP_PROF0_C            = LL2_TEAM_FILE_ID + 52,
    PS_FILE_ID_L_ROHC_DECOMP_PROF1_C            = LL2_TEAM_FILE_ID + 53,
    PS_FILE_ID_L_ROHC_DECOMP_PROF2_C            = LL2_TEAM_FILE_ID + 54,
    PS_FILE_ID_L_ROHC_DECOMP_PROF3_C            = LL2_TEAM_FILE_ID + 55,
    PS_FILE_ID_L_ROHC_DECOMP_PROF4_C            = LL2_TEAM_FILE_ID + 56,
    PS_FILE_ID_L_ROHC_DECOMP_MODE_TRANS_C       = LL2_TEAM_FILE_ID + 57,
    PS_FILE_ID_L_ROHC_DECOMP_IRIRDYN_PROC_C     = LL2_TEAM_FILE_ID + 58,
    PS_FILE_ID_L_ROHC_DECOMP_FEEDBAK_C          = LL2_TEAM_FILE_ID + 59,
    PS_FILE_ID_L_ROHC_DECOMP_C                  = LL2_TEAM_FILE_ID + 60,
    PS_FILE_ID_L_ROHC_CRC_C                     = LL2_TEAM_FILE_ID + 61,

    /* LTE用户面可维可测 */
    PS_FILE_ID_CDS_OMITF_C                      = LL2_TEAM_FILE_ID + 62,
    PS_FILE_ID_LPDCP_DLOM_ITF_C                 = LL2_TEAM_FILE_ID + 63,
    PS_FILE_ID_LRLC_OM_ITF_C                    = LL2_TEAM_FILE_ID + 64,
    PS_FILE_ID_LMAC_OM_ITF_C                    = LL2_TEAM_FILE_ID + 65,
    PS_FILE_ID_LUP_DT_ITF_C                     = LL2_TEAM_FILE_ID + 66,
    PS_FILE_ID_LUP_ERRLOG_ITF_C                 = LL2_TEAM_FILE_ID + 67,
    PS_FILE_ID_LUP_DCM_ITF_C                    = LL2_TEAM_FILE_ID + 68,

    /*L2 TTF QUEUE events appITF OMITF*/
    PS_FILE_ID_L_TTFQUEUE_C                     = LL2_TEAM_FILE_ID + 69,
    PS_FILE_ID_L2APPITF_C                       = LL2_TEAM_FILE_ID + 70,
    PS_FILE_ID_L2EVENTS_C                       = LL2_TEAM_FILE_ID + 71,
    PS_FILE_ID_L2OMITF_C                        = LL2_TEAM_FILE_ID + 72,

    /* LTE硬件接口 */
    PS_FILE_ID_LUP_HARDWIRE_ITF_C               = LL2_TEAM_FILE_ID + 73,

    /*CDS*/
    PS_FILE_ID_CDS_UL_PROC_C                    = LL2_TEAM_FILE_ID + 74,
    PS_FILE_ID_CDS_DL_PROC_C                    = LL2_TEAM_FILE_ID + 75,
    PS_FILE_ID_CDS_MSG_PROC_C                   = LL2_TEAM_FILE_ID + 76,
    PS_FILE_ID_CDS_IPF_CTRL_C                   = LL2_TEAM_FILE_ID + 77,
    PS_FILE_ID_CDS_ENTITY_C                     = LL2_TEAM_FILE_ID + 78,
    PS_FILE_ID_CDS_DEBUG_C                      = LL2_TEAM_FILE_ID + 79,
    PS_FILE_ID_CDS_IP_FRAGMENT_PROC_C           = LL2_TEAM_FILE_ID + 80,
    PS_FILE_ID_CDS_SOFT_FILTER_C                = LL2_TEAM_FILE_ID + 81,
    PS_FILE_ID_CDS_IMS_PROC_C                   = LL2_TEAM_FILE_ID + 82,
    PS_FILE_ID_CDS_DSM_PROC_C                   = LL2_TEAM_FILE_ID + 83,
    PS_FILE_ID_CDS_UL_CDMA_PROC_C               = LL2_TEAM_FILE_ID + 84,
    PS_FILE_ID_CDS_UL_GU_PROC_C                 = LL2_TEAM_FILE_ID + 85,
    PS_FILE_ID_CDS_UL_LTE_PROC_C                = LL2_TEAM_FILE_ID + 86,
    PS_FILE_ID_CDS_UL_NR_PROC_C                 = LL2_TEAM_FILE_ID + 87,
    PS_FILE_ID_CDS_CIPHER_UL_PROC_C             = LL2_TEAM_FILE_ID + 88,
    PS_FILE_ID_CDS_IMS_DL_FRAGMENT_PROC_C       = LL2_TEAM_FILE_ID + 89,
    PS_FILE_ID_CDS_IMS_UL_PROC_C                = LL2_TEAM_FILE_ID + 90,
    PS_FILE_ID_CDS_IMS_DL_PROC_C                = LL2_TEAM_FILE_ID + 91,
    PS_FILE_ID_CDS_DL_NL_PROC_C                 = LL2_TEAM_FILE_ID + 92,
    PS_FILE_ID_CDS_DL_GU_PROC_C                 = LL2_TEAM_FILE_ID + 93,
    PS_FILE_ID_CDS_DL_DISPATCH_C                = LL2_TEAM_FILE_ID + 94,
    PS_FILE_ID_CDS_COMM_MEM_C                   = LL2_TEAM_FILE_ID + 95,
    PS_FILE_ID_CDS_COMM_DFS_C                   = LL2_TEAM_FILE_ID + 96,
    PS_FILE_ID_CDS_COMM_TMR_C                   = LL2_TEAM_FILE_ID + 97,
    PS_FILE_ID_CDS_COMM_TASK_C                  = LL2_TEAM_FILE_ID + 98,
    PS_FILE_ID_CDS_SOFT_FILTER_CFG_C            = LL2_TEAM_FILE_ID + 99,
    PS_FILE_ID_CDS_SOFT_FILTER_ETH_C            = LL2_TEAM_FILE_ID + 100,
    PS_FILE_ID_CDS_SOFT_FILTER_IP_C             = LL2_TEAM_FILE_ID + 101,
    PS_FILE_ID_CDS_RNRD_CFG_C                   = LL2_TEAM_FILE_ID + 102,
    PS_FILE_ID_CDS_RNRD_FILTER_C                = LL2_TEAM_FILE_ID + 103,
    PS_FILE_ID_CDS_IPF_FILTER_CFG_C             = LL2_TEAM_FILE_ID + 104,
    PS_FILE_ID_CDS_IPF_INIT_C                   = LL2_TEAM_FILE_ID + 105,
    PS_FILE_ID_CDS_IPF_DL_PROC_C                = LL2_TEAM_FILE_ID + 106,
    PS_FILE_ID_CDS_IPF_FILTER_INIT_C            = LL2_TEAM_FILE_ID + 107,
    PS_FILE_ID_CDS_IPF_UL_PROC_C                = LL2_TEAM_FILE_ID + 108,
    PS_FILE_ID_CDS_IP_DECODE_C                  = LL2_TEAM_FILE_ID + 109,
    PS_FILE_ID_CDS_ETH_DECODE_C                 = LL2_TEAM_FILE_ID + 110,
    PS_FILE_ID_CDS_UL_ADAPTER_PROC_C            = LL2_TEAM_FILE_ID + 111,
    PS_FILE_ID_CDS_ENTITY_IFACE_C               = LL2_TEAM_FILE_ID + 112,
    PS_FILE_ID_CDS_ENTITY_PDUSESSION_C          = LL2_TEAM_FILE_ID + 113,
    PS_FILE_ID_CDS_ENTITY_COMM_C                = LL2_TEAM_FILE_ID + 114,
    PS_FILE_ID_CDS_COMM_ITF_C                   = LL2_TEAM_FILE_ID + 115,
    PS_FILE_ID_CDS_TAF_ITF_C                    = LL2_TEAM_FILE_ID + 116,
    PS_FILE_ID_CDS_NAS_ITF_C                    = LL2_TEAM_FILE_ID + 117,
    PS_FILE_ID_CDS_TC_ITF_C                     = LL2_TEAM_FILE_ID + 118,
    PS_FILE_ID_CDS_L2_ITF_C                     = LL2_TEAM_FILE_ID + 119,
    PS_FILE_ID_CDS_IMSA_ITF_C                   = LL2_TEAM_FILE_ID + 120,
    PS_FILE_ID_CDS_EVENT_ITF_C                  = LL2_TEAM_FILE_ID + 121,

    /*FC*/
    PS_FILE_ID_FC_FLOWCTRL_MANA_C               = LL2_TEAM_FILE_ID + 122,
    PS_FILE_ID_QOS_FC_OM_C                      = LL2_TEAM_FILE_ID + 123,
    PS_FILE_ID_QOS_FC_CHANNEL_FLOWCTRL_C        = LL2_TEAM_FILE_ID + 124,
    PS_FILE_ID_QOS_FC_RAB_STATUS_C              = LL2_TEAM_FILE_ID + 125,
    PS_FILE_ID_QOS_FC_STATUS_C                  = LL2_TEAM_FILE_ID + 126,

    /*NDCLIENT*/
    PS_FILE_ID_IPCOMM_C                         = LL2_TEAM_FILE_ID + 127,
    PS_FILE_ID_IPNDCLIENT_C                     = LL2_TEAM_FILE_ID + 128,
    PS_FILE_ID_NDCLIENTADSPROC_C                = LL2_TEAM_FILE_ID + 129,
    PS_FILE_ID_NDCLIENTAPSPROC_C                = LL2_TEAM_FILE_ID + 130,
    PS_FILE_ID_NDCLIENTCDSPROC_C                = LL2_TEAM_FILE_ID + 131,
    PS_FILE_ID_NDCLIENTCOMMONPKT_C              = LL2_TEAM_FILE_ID + 132,
    PS_FILE_ID_NDCLIENTCOMMONTIMER_C            = LL2_TEAM_FILE_ID + 133,
    PS_FILE_ID_NDCLIENTDEBUG_C                  = LL2_TEAM_FILE_ID + 134,
    PS_FILE_ID_NDCLIENTECHOREQPROC_C            = LL2_TEAM_FILE_ID + 135,
    PS_FILE_ID_NDCLIENTENTITY_C                 = LL2_TEAM_FILE_ID + 136,
    PS_FILE_ID_NDCLIENTOM_C                     = LL2_TEAM_FILE_ID + 137,
    PS_FILE_ID_NDCLIENTRAPROC_C                 = LL2_TEAM_FILE_ID + 138,
    PS_FILE_ID_IPNDSERVER_C                     = LL2_TEAM_FILE_ID + 139,
    PS_FILE_ID_NDSERVERATPROC_C                 = LL2_TEAM_FILE_ID + 140,
    PS_FILE_ID_NDSERVERDEBUG_C                  = LL2_TEAM_FILE_ID + 141,
    PS_FILE_ID_NDSERVERDHCPV6PKTPROC_C          = LL2_TEAM_FILE_ID + 142,
    PS_FILE_ID_NDSERVERENTITY_C                 = LL2_TEAM_FILE_ID + 143,
    PS_FILE_ID_NDSERVERNAPROC_C                 = LL2_TEAM_FILE_ID + 144,
    PS_FILE_ID_NDSERVERNSPROC_C                 = LL2_TEAM_FILE_ID + 145,
    PS_FILE_ID_NDSERVEROM_C                     = LL2_TEAM_FILE_ID + 146,
    PS_FILE_ID_NDSERVERPKTCOMM_C                = LL2_TEAM_FILE_ID + 147,
    PS_FILE_ID_NDSERVERRAPROC_C                 = LL2_TEAM_FILE_ID + 148,
    PS_FILE_ID_NDSERVERRSPROC_C                 = LL2_TEAM_FILE_ID + 149,
    PS_FILE_ID_NDSERVERTIMERPROC_C              = LL2_TEAM_FILE_ID + 150,
    PS_FILE_ID_NDISVCOMITF_C                    = LL2_TEAM_FILE_ID + 151,
    PS_FILE_ID_NDISCDSPROC_C                    = LL2_TEAM_FILE_ID + 152,
    PS_FILE_ID_NDISENTITY_C                     = LL2_TEAM_FILE_ID + 153,
    PS_FILE_ID_NDISULPROC_C                     = LL2_TEAM_FILE_ID + 154,
    PS_FILE_ID_NDISOM_C                         = LL2_TEAM_FILE_ID + 155,
    PS_FILE_ID_NDISDEBUG_C                      = LL2_TEAM_FILE_ID + 156,
    PS_FILE_ID_NDISDLPROC_C                     = LL2_TEAM_FILE_ID + 157,
    PS_FILE_ID_NDISMSGPROC_C                    = LL2_TEAM_FILE_ID + 158,
    PS_FILE_ID_NDISADSPROC_C                    = LL2_TEAM_FILE_ID + 159,
    PS_FILE_ID_NDISARPPROC_C                    = LL2_TEAM_FILE_ID + 160,
    PS_FILE_ID_NDISATPROC_C                     = LL2_TEAM_FILE_ID + 161,
    PS_FILE_ID_IPV4DHCPPKTFORM_C                = LL2_TEAM_FILE_ID + 162,
    PS_FILE_ID_IPV4DHCPPKTPARSE_C               = LL2_TEAM_FILE_ID + 163,
    PS_FILE_ID_IPV4DHCPSERVER_C                 = LL2_TEAM_FILE_ID + 164,

    /*added for L2 VOLTE analyze 2016-08-23 */
    PS_FILE_ID_LUP_AUTO_ANALYSIS_C              = LL2_TEAM_FILE_ID + 165,
    PS_FILE_ID_LUP_COM_DEPEND_C                 = LL2_TEAM_FILE_ID + 166,
    PS_FILE_ID_PSUP_COM_MEM_C                   = LL2_TEAM_FILE_ID + 167,
    PS_FILE_ID_LUPFC_C                          = LL2_TEAM_FILE_ID + 168,
    PS_FILE_ID_LRLC_UL_OM_C                     = LL2_TEAM_FILE_ID + 169,
    PS_FILE_ID_LRLC_DL_OM_C                     = LL2_TEAM_FILE_ID + 170,
    PS_FILE_ID_LMAC_UL_OM_C                     = LL2_TEAM_FILE_ID + 171,
    PS_FILE_ID_LMAC_DL_OM_C                     = LL2_TEAM_FILE_ID + 172,
    PS_FILE_ID_LMAC_RA_OM_C                     = LL2_TEAM_FILE_ID + 173,
    PS_FILE_ID_LRLC_UL_ARQ_C                    = LL2_TEAM_FILE_ID + 174,
    PS_FILE_ID_LRLC_UL_CONFIG_C                 = LL2_TEAM_FILE_ID + 175,
    PS_FILE_ID_LRLC_UL_ENTITY_C                 = LL2_TEAM_FILE_ID + 176,
    PS_FILE_ID_LRLC_UL_CTRL_TRANS_C             = LL2_TEAM_FILE_ID + 177,
    PS_FILE_ID_LRLC_UL_DATA_TRANS_C             = LL2_TEAM_FILE_ID + 178,
    PS_FILE_ID_L_RLCDLENTITY_C                  = LL2_TEAM_FILE_ID + 179,
    PS_FILE_ID_LRLC_DL_UM_DATATRANS_C           = LL2_TEAM_FILE_ID + 180,
    PS_FILE_ID_LRLC_DL_AM_DATATRANS_C           = LL2_TEAM_FILE_ID + 181,
    PS_FILE_ID_L_PDCPULENTITYMANAGE_C           = LL2_TEAM_FILE_ID + 182,
    PS_FILE_ID_L_PDCPULINTRAMSGPROC_C           = LL2_TEAM_FILE_ID + 183,
    PS_FILE_ID_L_PDCPRRCMSGPROC_C               = LL2_TEAM_FILE_ID + 184,
    PS_FILE_ID_L_PDCPRLCMSGPROC_C               = LL2_TEAM_FILE_ID + 185,
    PS_FILE_ID_L_PDCPRABMMSGPROC_C              = LL2_TEAM_FILE_ID + 186,
    PS_FILE_ID_L_PDCPIMSAMSGPROC_C              = LL2_TEAM_FILE_ID + 187,
    PS_FILE_ID_L_PDCPMACMSGPROC_C               = LL2_TEAM_FILE_ID + 188,
    PS_FILE_ID_L_PDCPBASTETMSGPROC_C            = LL2_TEAM_FILE_ID + 189,
    PS_FILE_ID_L_PDCPULSRBDATAPROC_C            = LL2_TEAM_FILE_ID + 190,
    PS_FILE_ID_L_PDCPULOPTIMIZE_C               = LL2_TEAM_FILE_ID + 191,
    PS_FILE_ID_L_PDCPULDRBSTOREDATA_C           = LL2_TEAM_FILE_ID + 192,
    PS_FILE_ID_L_PDCPULBSRCALCU_C               = LL2_TEAM_FILE_ID + 193,
    PS_FILE_ID_L_PDCPULDISCARDPROC_C            = LL2_TEAM_FILE_ID + 194,
    PS_FILE_ID_L_PDCPULCONTROLPDUPROC_C         = LL2_TEAM_FILE_ID + 195,
    PS_FILE_ID_L_PDCPDLOPTIMIZE_C               = LL2_TEAM_FILE_ID + 196,
    PS_FILE_ID_L_PDCPSLEEP_C                    = LL2_TEAM_FILE_ID + 197,
    PS_FILE_ID_L_PDCPDLENTRY_C                  = LL2_TEAM_FILE_ID + 198,
    PS_FILE_ID_L_PDCPDLREORDER_C                = LL2_TEAM_FILE_ID + 199,
    PS_FILE_ID_L_PDCPDLSRBDATAPROC_C            = LL2_TEAM_FILE_ID + 200,
    PS_FILE_ID_L_PDCPDLDRBRECVDATA_C            = LL2_TEAM_FILE_ID + 201,
    PS_FILE_ID_L_PDCPDLDRBSUBMITDATA_C          = LL2_TEAM_FILE_ID + 202,
    PS_FILE_ID_L_PDCPDLCONTROLPDUPROC_C         = LL2_TEAM_FILE_ID + 203,
    PS_FILE_ID_L_PDCPULDEBUG_C                  = LL2_TEAM_FILE_ID + 204,
    PS_FILE_ID_L_PDCPULOMITF_C                  = LL2_TEAM_FILE_ID + 205,
    PS_FILE_ID_L_PDCPMEM_C                      = LL2_TEAM_FILE_ID + 206,
    PS_FILE_ID_L_PDCPDLCHR_C                    = LL2_TEAM_FILE_ID + 207,
    PS_FILE_ID_L_PDCPULCHR_C                    = LL2_TEAM_FILE_ID + 208,
    PS_FILE_ID_L_PDCPDLINTRAMSGPROC_C           = LL2_TEAM_FILE_ID + 209,
    PS_FILE_ID_L_PDCPTIMER_C                    = LL2_TEAM_FILE_ID + 210,
    PS_FILE_ID_ROHC_COM_CRC                     = LL2_TEAM_FILE_ID + 211,
    PS_FILE_ID_ROHC_DL_COM_CONTEXT              = LL2_TEAM_FILE_ID + 212,
    PS_FILE_ID_ROHC_DL_COM_CRC                  = LL2_TEAM_FILE_ID + 213,
    PS_FILE_ID_ROHC_DL_COM_DECODE               = LL2_TEAM_FILE_ID + 214,
    PS_FILE_ID_ROHC_DL_COM_DECOMP0TYPE          = LL2_TEAM_FILE_ID + 215,
    PS_FILE_ID_ROHC_DL_COM_DECOMP1TYPE          = LL2_TEAM_FILE_ID + 216,
    PS_FILE_ID_ROHC_DL_COM_DECOMP2TYPE          = LL2_TEAM_FILE_ID + 217,
    PS_FILE_ID_ROHC_DL_COM_DECOMPIRDYNTYPE      = LL2_TEAM_FILE_ID + 218,
    PS_FILE_ID_ROHC_DL_COM_DECOMPIRTYPE         = LL2_TEAM_FILE_ID + 219,
    PS_FILE_ID_ROHC_DL_COM_ENTRY                = LL2_TEAM_FILE_ID + 220,
    PS_FILE_ID_ROHC_DL_COM_FEEDBACK             = LL2_TEAM_FILE_ID + 221,
    PS_FILE_ID_ROHC_DL_COM_MODESTATE_CHANGE     = LL2_TEAM_FILE_ID + 222,
    PS_FILE_ID_ROHC_DL_COM_REFORM               = LL2_TEAM_FILE_ID + 223,
    PS_FILE_ID_ROHC_DL_OM                       = LL2_TEAM_FILE_ID + 224,
    PS_FILE_ID_ROHC_DL_PROF0                    = LL2_TEAM_FILE_ID + 225,
    PS_FILE_ID_ROHC_DL_PROF1                    = LL2_TEAM_FILE_ID + 226,
    PS_FILE_ID_ROHC_DL_PROF2                    = LL2_TEAM_FILE_ID + 227,
    PS_FILE_ID_ROHC_DL_PROF3                    = LL2_TEAM_FILE_ID + 228,
    PS_FILE_ID_ROHC_DL_PROF4                    = LL2_TEAM_FILE_ID + 229,
    PS_FILE_ID_ROHC_UL_COM_CONSTRUCT0TYPE       = LL2_TEAM_FILE_ID + 230,
    PS_FILE_ID_ROHC_UL_COM_CONSTRUCT1TYPE       = LL2_TEAM_FILE_ID + 231,
    PS_FILE_ID_ROHC_UL_COM_CONSTRUCT2TYPE       = LL2_TEAM_FILE_ID + 232,
    PS_FILE_ID_ROHC_UL_COM_CONSTRUCTIRDYNTYPE   = LL2_TEAM_FILE_ID + 233,
    PS_FILE_ID_ROHC_UL_COM_CONSTRUCTIRTYPE      = LL2_TEAM_FILE_ID + 234,
    PS_FILE_ID_ROHC_UL_COM_CONTEXTOBTAIN        = LL2_TEAM_FILE_ID + 235,
    PS_FILE_ID_ROHC_UL_COM_CRC                  = LL2_TEAM_FILE_ID + 236,
    PS_FILE_ID_ROHC_UL_COM_CREATEFB             = LL2_TEAM_FILE_ID + 237,
    PS_FILE_ID_ROHC_UL_COM_ENCODE               = LL2_TEAM_FILE_ID + 238,
    PS_FILE_ID_ROHC_UL_COM_ENTRY                = LL2_TEAM_FILE_ID + 239,
    PS_FILE_ID_ROHC_UL_COM_MODESTATE_CHANGE     = LL2_TEAM_FILE_ID + 240,
    PS_FILE_ID_ROHC_UL_COM_OMODE_DATAPROC       = LL2_TEAM_FILE_ID + 241,
    PS_FILE_ID_ROHC_UL_COM_RMODE_DATAPROC       = LL2_TEAM_FILE_ID + 242,
    PS_FILE_ID_ROHC_UL_COM_UMODE_DATAPROC       = LL2_TEAM_FILE_ID + 243,
    PS_FILE_ID_ROHC_UL_COM_SELECT               = LL2_TEAM_FILE_ID + 244,
    PS_FILE_ID_ROHC_UL_COM_SUBHEAD              = LL2_TEAM_FILE_ID + 245,
    PS_FILE_ID_ROHC_UL_OM                       = LL2_TEAM_FILE_ID + 246,
    PS_FILE_ID_ROHC_UL_PROF0                    = LL2_TEAM_FILE_ID + 247,
    PS_FILE_ID_ROHC_UL_PROF1SELECT              = LL2_TEAM_FILE_ID + 248,
    PS_FILE_ID_ROHC_UL_PROF2SELECT              = LL2_TEAM_FILE_ID + 249,
    PS_FILE_ID_ROHC_UL_COM_CMPDYNDATA           = LL2_TEAM_FILE_ID + 250,
    PS_FILE_ID_ROHC_UL_COM_CONSTRUCTCOM         = LL2_TEAM_FILE_ID + 251,
    PS_FILE_ID_ROHC_UL_PROF6SELECT              = LL2_TEAM_FILE_ID + 252,
    PS_FILE_ID_ROHC_UL_PROF6_REPLICATE          = LL2_TEAM_FILE_ID + 253,
    PS_FILE_ID_ROHC_UL_COM_CONTEXTMATCH         = LL2_TEAM_FILE_ID + 254,
    PS_FILE_ID_ROHC_UL_COM_CONSTRUCTPROF6       = LL2_TEAM_FILE_ID + 255,
    PS_FILE_ID_L_RLCCHR_C                       = LL2_TEAM_FILE_ID + 256,
    PS_FILE_ID_ROHC_DL_COM_EXT3PROC             = LL2_TEAM_FILE_ID + 257,
    PS_FILE_ID_ROHC_DL_COM_GETBASEPARA          = LL2_TEAM_FILE_ID + 258,
    PS_FILE_ID_ROHC_UL_COM_CONSTRUCTEXT3        = LL2_TEAM_FILE_ID + 259,
    PS_FILE_ID_ROHC_UL_COM_CONTEXTMANAGE        = LL2_TEAM_FILE_ID + 260,
    PS_FILE_ID_ROHC_UL_COM_GETIPINFO            = LL2_TEAM_FILE_ID + 261,
    PS_FILE_ID_ROHC_COM_STAT                    = LL2_TEAM_FILE_ID + 262,
    PS_FILE_ID_ROHC_COM_PARA                    = LL2_TEAM_FILE_ID + 263,
}LL2_FILE_ID_DEFINE_ENUM;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif


