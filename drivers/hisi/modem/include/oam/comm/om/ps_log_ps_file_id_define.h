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

#ifndef PS_LOG_PS_FILE_ID_DEFINE_H
#define PS_LOG_PS_FILE_ID_DEFINE_H

#include "ps_log_file_id_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/* PS源文件ID号 */
typedef enum
{
                PS_FILE_ID_ACF_C = LOG_MIN_FILE_ID_PS,
/* 001  */      PS_FILE_ID_API_STUB_C,
/* 002  */      PS_FILE_ID_APP_API_C,
/* 003  */      PS_FILE_ID_APP_API_REG_C,
/* 004  */      PS_FILE_ID_APPINTERFACEAPI_C,

/* 006  */      PS_FILE_ID_ASYNC_C,

/* 023  */      PS_FILE_ID_AUTH_C,
/* 024  */      PS_FILE_ID_BBPTIMER_C,
/* 025  */      PS_FILE_ID_BITENCODE_C,
/* 026  */      PS_FILE_ID_CARDLOCKCIPHER_C,


/* 065  */      PS_FILE_ID_CFDCMPR_C,
/* 066  */      PS_FILE_ID_CFDCMPRV42_C,
/* 067  */      PS_FILE_ID_CFPCMPR_C,
/* 068  */      PS_FILE_ID_CFPCMPR1144_C,
/* 069  */      PS_FILE_ID_CFPCMPR2507_C,
/* 070  */      PS_FILE_ID_CFPCMPRCOMM_C,
/* 071  */      PS_FILE_ID_CM_RCVTAFADAPTER_C,
/* 072  */      PS_FILE_ID_CM_SNDTAFADAPTER_C,
/* 112  */      PS_FILE_ID_ENTHERSTUB_C,
/* 113  */      PS_FILE_ID_ETHERMAIN_C,
/* 201  */      PS_FILE_ID_GBBPTIMER_C,

/* 221  */      PS_FILE_ID_GNA_END_C,
/* 222  */      PS_FILE_ID_GNA_MGR_C,
/* 223  */      PS_FILE_ID_GNA_NPT_C,
/* 224  */      PS_FILE_ID_GNA_STUBSFORWIN_C,
/* 225  */      PS_FILE_ID_GNA_TRANSFER_C,
/* 226  */      PS_FILE_ID_GNA_ULDATA_C,
/* 238  */      PS_FILE_ID_GSLEEPFLOW_C,
/* 239  */      PS_FILE_ID_GSLEEPSLEEP_C,

/* 289  */      PS_FILE_ID_MUX_CORE_C,
/* 290  */      PS_FILE_ID_MUX_DOPRA_C,
/* 291  */      PS_FILE_ID_MUX_WIN_C,
/* 292  */      PS_FILE_ID_NAS_STUB_C,
/* 293  */      PS_FILE_ID_NETHOOK_C,
/* 340  */      PS_FILE_ID_PS_HELP_C,
/* 341  */      PS_FILE_ID_PS_INIT_C,
/* 342  */      PS_FILE_ID_PS_LIB_C,
/* 343  */      PS_FILE_ID_PS_NASHELP_C,
/* 345  */      PS_FILE_ID_PS_RLCSTUB_C,
/* 346  */      PS_FILE_ID_PS_STUB_C,
/* 347  */      PS_FILE_ID_PS_TAFHELP_C,

/* 360  */      PS_FILE_ID_RECURTEST_C,
/* 361  */      PS_FILE_ID_RFA_C,
/* 362  */      PS_FILE_ID_RTC_TIMER_C,
/* 363  */      PS_FILE_ID_SLEEP_FLOW_C,
/* 364  */      PS_FILE_ID_SLEEP_INIT_C,
/* 365  */      PS_FILE_ID_SLEEP_ISR_C,
/* 366  */      PS_FILE_ID_SLEEP_DRV_C,

/* 379  */      PS_FILE_ID_SMT_COM_C,
/* 380  */      PS_FILE_ID_SMT_RCVMMC_C,
/* 381  */      PS_FILE_ID_SMT_RCVSMR_C,
/* 382  */      PS_FILE_ID_SMT_RCVTAF_C,
/* 383  */      PS_FILE_ID_SMT_RCVTIMER_C,
/* 384  */      PS_FILE_ID_SMT_RCVUSIM_C,
/* 385  */      PS_FILE_ID_SMT_SNDSMR_C,
/* 386  */      PS_FILE_ID_SMT_SNDTAF_C,
/* 387  */      PS_FILE_ID_SMT_SNDUSIM_C,
/* 388  */      PS_FILE_ID_SMT_TASKMNG_C,
/* 394  */      PS_FILE_ID_SRCPARSEPRINTF_C,


/* 411  */      PS_FILE_ID_STUB_COMM_C,
/* 412  */      PS_FILE_ID_STUB_MAIN_C,

/* 436  */      PS_FILE_ID_THROUGHPUT_C,
/* 437  */      PS_FILE_ID_TS04_C,
/* 701  */      PS_FILE_ID_HPA_STUB_COMM_C,
/* 702  */      PS_FILE_ID_HPA_STUB_RCV_MSG_C,
/* 703  */      PS_FILE_ID_MONITOR_C,
/* 704  */      PS_FILE_ID_UPDATE_FROM_TF_C,
/* 705  */      PS_FILE_ID_NDIS_CTRL_C,
/* 706  */      PS_FILE_ID_NDIS_DHCPS_C,
/* 707  */      PS_FILE_ID_NDIS_DL_DATA_C,
/* 708  */      PS_FILE_ID_NDIS_COMM_C,
/* 709  */      PS_FILE_ID_NDIS_UL_DATA_C,
/* 710  */      PS_FILE_ID_NDIS_MNTN_C,
/* 711  */      PS_FILE_ID_NDIS_STUB_C,
/* 712  */      PS_FILE_ID_NDIS_USB_ST_STUB_C,



/* 721 */      PS_FILE_ID_VHWUEA1_C,
/* 722 */      PS_FILE_ID_VHWMAIN_C,
/* 723 */      PS_FILE_ID_VHWHDLC_C,
/* 724 */      PS_FILE_ID_VHWMEM_C,
/* 725 */      PS_FILE_ID_AT_AP_DL_DATA_C,
/* 726 */      PS_FILE_ID_AP_COMM_C,
/* 734 */      PS_FILE_ID_VHW_DATAMOVER_C,
/* V3 R7_7C1 End */
/*  735  */      PS_FILE_ID_GSLEEP_FLOW_C,
/*  736  */      PS_FILE_ID_AGING_TEST_C,
/*  737  */      PS_FILE_ID_OM_OPERATOR_C,
/*  738  */      PS_FILE_ID_DIAG_INIT_C,
/*  739  */      PS_FILE_ID_DIAG_UTILS_C,
/*  742 */       PS_FILE_ID_E5_API_C,
/*  743 */       PS_FILE_ID_E5_CTRI_C,
/*  744  */      DPM_DFS_C,
/*  748  */      PS_FILE_ID_OM_PC_VOICE_C,
/*  749  */      PS_FILE_ID_CSTCSTM_C,
/*  750  */      PS_FILE_ID_CSTL2R_C,
/*  751  */      PS_FILE_ID_CSTNTM_C,
/*  752  */      PS_FILE_ID_CSTRA0_C,
/*  753  */      PS_FILE_ID_CSTRA1M_C,
/*  754  */      PS_FILE_ID_CSTRINGBUF_C,
/*  755  */      PS_FILE_ID_CSTRLPDATA_C,
/*  756  */      PS_FILE_ID_CSTTRANSENTITY_C,
/*  757  */      PS_FILE_ID_CSTRLPE_C,
/*  758  */      PS_FILE_ID_FAXBUF_C,
/*  759  */      PS_FILE_ID_FAXGFA_C,
/*  760  */      PS_FILE_ID_FAXMGR_C,
/*  761  */      PS_FILE_ID_FAXT31_C,

/*  764  */      PS_FILE_ID_MNMSG_CB_SENDAS_C,
/*  765  */      PS_FILE_ID_MNMSG_CB_PROC_C,
/*  766  */      PS_FILE_ID_MNMSG_CB_ENCDEC_C,

/*  780  */      PS_FILE_ID_OM_PRINTF_C,
/*  781  */      PS_FILE_ID_OM_PCCOMM_C,

/*V3 ERRLOG Start */
/*  782  */      PS_FILE_ID_PS_LOG_C,
/*  783  */      PS_FILE_ID_CPU_FLOW_CTRL_C,
/*  784  */      PS_FILE_ID_R_ITF_FLOW_CTRL_C,

/*  793  */      PS_FILE_ID_PPP_PUBLIC_C,
/*  794  */      PS_FILE_ID_DEVMGR_C,
/*  795  */      PS_FILE_ID_DATADISPATCH_C,
/*  796  */      PS_FILE_ID_GUIPFPROC_C,
/*  798  */      PS_FILE_ID_GUAT_NDIS_C,
/*  799  */      PS_FILE_ID_GUAT_MAIN_C,
/*  800  */      PS_FILE_ID_GUAT_PPP_C,
/*  801  */      PS_FILE_ID_GUAT_CMDTAB_C,
/*  836  */      PS_FILE_ID_XML_C,
/*  837  */      PS_FILE_ID_OAM_STUB_C,
/*  838  */      PS_FILE_ID_OMRINGBUFFER_C,
/*  793  */       PS_FILE_ID_L_ARPFUNC_C,
                  PS_FILE_ID_L_HPADLPROC_C,
                  PS_FILE_ID_L_HPADLENTRY_C,
                  PS_FILE_ID_L_HPAULENTRY_C,
                  PS_FILE_ID_L_HPACOMM_C,

/*  798  */       PS_FILE_ID_L_HPAULPROC_C,
                  PS_FILE_ID_MAIN_C,
                  PS_FILE_ID_LINTERMSGTEST_C,





                  PS_FILE_ID_DEFINE_BUTT        /* 下面大的枚举的偏移 */

}PS_FILE_ID_DEFINE_ENUM1;

/*解决VC6编译问题,大的枚举定义拆开*/
typedef enum
{

                  PS_FILE_ID_L_HPAGLOBAL_C = PS_FILE_ID_DEFINE_BUTT+1,

/*  937  */       PS_FILE_ID_L_HPAINIT_C,
                  PS_FILE_ID_L_HPAINTRSP_C,
                  PS_FILE_ID_L_HPAMSGDEBUG_C,
                  PS_FILE_ID_L_HPAMSGPROC_C,
                  PS_FILE_ID_L_HPARBUFFER_C,

/*  942  */       PS_FILE_ID_L_HPASIGNAL_C,
                  PS_FILE_ID_L_HPAUPSELFPROC_C,
                  PS_FILE_ID_L_PSRNGLIB_C,
                  PS_FILE_ID_L_CPU_UTILIZATION_C,
                  PS_FILE_ID_L_PSSOFTDEBUG_C,

/*  947  */       PS_FILE_ID_L_PSOM_C,
                  PS_FILE_ID_L_UPZEROCOPY_C,
                  PS_FILE_ID_L_PS_AUTH_DESALG_C,
                  PS_FILE_ID_L_PSAUTH_C,
                  PS_FILE_ID_L_PSMD5_C,

                  /*STUB文件*/
/*  952  */       PS_FILE_ID_L_SIML1SENDRRC_C,
                  PS_FILE_ID_L_RABMSTUB_C,
                  PS_FILE_ID_L_PSOM_ERRLOG_C,




                  /*L2环回测试文件*/

                  PS_FILE_ID_NASESMSMMSGPROC_C,
                  /*C50 code sync*/

/*  1064  */      PS_FILE_ID_L_PSSTUB_C,




            /*ERABM*/
            PS_FILE_ID_NASERABMCDSMSGPROC_C,


                  PS_FILE_ID_MMC_LMMINTERFACE_C,

/* R9 DC+MIMO合入 */
                  PS_FILE_ID_ACPU_OM_C,
                  PS_FILE_ID_ACPU_OMRL_C,
                  PS_FILE_ID_OM_AGENT_C,
                  PS_FILE_ID_NVIM_AFUNC_C,
                  PS_FILE_ID_ACPU_OMAGENT_C,
                  PS_FILE_ID_OM_SD_LOG_C,
                  PS_FILE_ID_CPM_C,
                  PS_FILE_ID_OM_NO_SIG_C,
                  PS_FILE_ID_PCSC_APP_PROC_C,
                  PS_FILE_ID_OM_APP_OUTSIDE_C,
                  PS_FILE_ID_IMC_C,
                  PS_FILE_ID_UPD_C,
                  PS_FILE_ID_DICC_C,
                  PS_FILE_ID_IMCLINUX_C,
                  PS_FILE_ID_RNIC_C,



                  PS_FILE_ID_ACPU_NFEX_C,
                  PS_FILE_ID_ACPU_NFEX_CTRL_C,
                  PS_FILE_ID_ACPU_IPS_MNTN_C,
                  PS_FILE_ID_ACPU_IPS_MNTN_MINI_C,
                  PS_FILE_ID_ACPU_IPS_TRAFFIC_STATISTIC_C,
                  PS_FILE_ID_CPULOAD_C,
                  PS_FILE_ID_FLOW_CTRL_C,
                  PS_FILE_ID_ACORE_FLOW_CTRL_C,
                  PS_FILE_ID_CCORE_FLOW_CTRL_C,
                  PS_FILE_ID_TTF_MEM_RB_C,
                  PS_FILE_ID_TTF_MEMCTRL_C,
                  /* Added by h0016399 for DSDA Project，2012-12-24,  end */
                  PS_FILE_ID_IMM_RB_C,
                  PS_FILE_ID_IMM_ZC_C,
                  PS_FILE_ID_IMM_MNTN_C,


                  PS_FILE_ID_LITTLE_IMAGE_LOG_C,


                  PS_FILE_ID_PSDIPC_C,
                  PS_FILE_ID_PSMUX_C,
                  PS_FILE_ID_SOCP_C,

                  /*Tds begin*/


                  PS_FILE_ID_T_F8F9_C,
                  PS_FILE_ID_T_FSM01COM_C,
                  PS_FILE_ID_T_FSM02INI_C,


                  PS_FILE_ID_T_FSMRRC_C,




                  PS_FILE_ID_T_LCR_PS4_C,
                  PS_FILE_ID_T_LCR_ST_SYS_C,







                  PS_FILE_ID_T_RTOS02_PS_C,





                  PS_FILE_ID_T_TESTFSM_C,




                  PS_FILE_ID_T_TGL100PUB_PS_C,

                  /*Tds end*/

                  PS_FILE_ID_NVIM_COMMFUNC_C,

                  PS_FILE_ID_ASN1APP_C,
                  PS_FILE_ID_ASN1MSGMEM_C,
                  PS_FILE_ID_RRCAPI_C,

                  PS_FILE_ID_OM_ERRORLOG_C,


                  PS_FILE_ID_IMS_NIC_C,
                  PS_FILE_ID_IPS_MNTN_CCORE_C,





                  PS_FILE_ID_CNAS_CCB_C,
                  PS_FILE_ID_CNAS_MAIN_C,
                  PS_FILE_ID_CNAS_TIMER_MGMT_C,
                  PS_FILE_ID_CNAS_MNTN_C,
                  PS_FILE_ID_CNAS_PRL_API_C,
                  PS_FILE_ID_CNAS_PRL_MNTN_C,
                  PS_FILE_ID_CNAS_PRL_PARSE_C,
                  PS_FILE_ID_CNAS_PRL_MEM_C,
                  PS_FILE_ID_CNAS_MNTN_DUMP_C,

                  PS_FILE_ID_CNAS_XSD_CTX_C,
                  PS_FILE_ID_CNAS_XSD_FSM_MAIN_C,
                  PS_FILE_ID_CNAS_XSD_FSM_MAIN_TBL_C,
                  PS_FILE_ID_CNAS_XSD_FSM_POWER_OFF_C,
                  PS_FILE_ID_CNAS_XSD_FSM_POWER_OFF_TBL_C,
                  PS_FILE_ID_CNAS_XSD_FSM_SWITCH_ON_C,
                  PS_FILE_ID_CNAS_XSD_FSM_SWITCH_ON_TBL_C,
                  PS_FILE_ID_CNAS_XSD_MAIN_C,
                  PS_FILE_ID_CNAS_XSD_PRE_PROC_ACT_C,
                  PS_FILE_ID_CNAS_XSD_PRE_PROC_TBL_C,
                  PS_FILE_ID_CNAS_XSD_PROC_NVIM_C,
                  PS_FILE_ID_CNAS_XSD_PROC_CARD_C,
                  PS_FILE_ID_CNAS_XSD_SND_CAS_C,
                  PS_FILE_ID_CNAS_XSD_SND_INTERNAL_MSG_C,
                  PS_FILE_ID_CNAS_XSD_SND_MSCC_C,
                  PS_FILE_ID_CNAS_XSD_SND_XCC_C,
                  PS_FILE_ID_CNAS_XSD_SND_XREG_C,
                  PS_FILE_ID_CNAS_XSD_COM_FUNC_C,
                  PS_FILE_ID_CNAS_XSD_FSM_SYS_ACQ_C,
                  PS_FILE_ID_CNAS_XSD_FSM_SYS_ACQ_TBL_C,
                  PS_FILE_ID_CNAS_XSD_FSM_REDIR_TBL_C,
                  PS_FILE_ID_CNAS_XSD_FSM_REDIR_C,
                  PS_FILE_ID_CNAS_XSD_SYS_ACQ_STRATEGY_C,
                  PS_FILE_ID_CNAS_XSD_MNTN_C,

                  PS_FILE_ID_CNAS_XSD_SND_TAF_C,
                  PS_FILE_ID_CNAS_XSD_LOG_PRIVACY_C,


                  PS_FILE_ID_CNAS_XREG_MAIN_C,
                  PS_FILE_ID_CNAS_XREG_CTX_C,
                  PS_FILE_ID_CNAS_XREG_FSM_MAIN_TBL_C,
                  PS_FILE_ID_CNAS_XREG_FSM_MAIN_C,
                  PS_FILE_ID_CNAS_XREG_SND_XSD_C,
                  PS_FILE_ID_CNAS_XREG_SND_CAS_C,
                  PS_FILE_ID_CNAS_XREG_SND_INTERNAL_MSG_C,
                  PS_FILE_ID_CNAS_XREG_PROCESS_C,
                  PS_FILE_ID_CNAS_XREG_PRE_PROC_TBL_C,
                  PS_FILE_ID_CNAS_XREG_PRE_PROC_C,
                  PS_FILE_ID_CNAS_XREG_REGING_PROC_C,
                  PS_FILE_ID_CNAS_XREG_REGING_PROC_TBL_C,
                  PS_FILE_ID_CNAS_XREG_LIST_PROC_C,
                  PS_FILE_ID_CNAS_XREG_SND_APS_C,
                  PS_FILE_ID_CNAS_XCC_MAIN_C,
                  PS_FILE_ID_CNAS_XCC_CTX_C,
                  PS_FILE_ID_CNAS_XCC_INSTANCE_MGMT_C,
                  PS_FILE_ID_CNAS_XCC_FSM_MAIN_C,
                  PS_FILE_ID_CNAS_XCC_FSM_MAIN_TBL_C,
                  PS_FILE_ID_CNAS_XCC_MAIN_CTRL_C,
                  PS_FILE_ID_CNAS_XCC_PRE_PROC_TBL_C,
                  PS_FILE_ID_CNAS_XCC_SND_XSD_C,
                  PS_FILE_ID_CNAS_XCC_SND_XCALL_C,
                  PS_FILE_ID_CNAS_XCC_SND_CAS_C,
                  PS_FILE_ID_CNAS_XCC_FSM_MO_CALLING_C,
                  PS_FILE_ID_CNAS_XCC_FSM_MO_CALLING_TBL_C,
                  PS_FILE_ID_CNAS_XCC_FSM_MT_CALLING_C,
                  PS_FILE_ID_CNAS_XCC_FSM_MT_CALLING_TBL_C,
                  PS_FILE_ID_CNAS_XCC_SND_INTERNAL_MSG_C,
                  PS_FILE_ID_CNAS_XCC_SND_APS_C,
                  PS_FILE_ID_CNAS_XCC_COM_FUNC_C,
                  PS_FILE_ID_CNAS_XCC_SND_MMA_C,
                  PS_FILE_ID_CNAS_XCC_SND_LMM_C,
                  PS_FILE_ID_CNAS_XCC_MNTN_C,
                  PS_FILE_ID_CNAS_XCC_LOG_PRIVACY_C,

                  PS_FILE_ID_CNAS_HSM_LOG_PRIVACY_C,

                  PS_FILE_ID_CNAS_HSD_LOG_PRIVACY_C,

                  PS_FILE_ID_CNAS_XCC_SND_CSMS_C,
                  PS_FILE_ID_CNAS_XCC_SND_XPDS_C,

                  PS_FILE_ID_CNAS_HLU_CTX_C,
                  PS_FILE_ID_CNAS_HLU_MAIN_C,
                  PS_FILE_ID_CNAS_HLU_COMM_C,
                  PS_FILE_ID_CNAS_HLU_SND_HSM_C,
                  PS_FILE_ID_CNAS_HLU_SND_HSD_C,
                  PS_FILE_ID_CNAS_HLU_SND_HRM_C,
                  PS_FILE_ID_CNAS_HLU_PROC_NVIM_C,

                  PS_FILE_ID_CNAS_HSD_COM_FUNC_C,
                  PS_FILE_ID_CNAS_HSD_CTX_C,
                  PS_FILE_ID_CNAS_HSD_FSM_MAIN_C,
                  PS_FILE_ID_CNAS_HSD_FSM_MAIN_TBL_C,
                  PS_FILE_ID_CNAS_HSD_FSM_POWER_OFF_C,
                  PS_FILE_ID_CNAS_HSD_FSM_POWER_OFF_TBL_C,
                  PS_FILE_ID_CNAS_HSD_FSM_SWITCH_ON_C,
                  PS_FILE_ID_CNAS_HSD_FSM_SWITCH_ON_TBL_C,
                  PS_FILE_ID_CNAS_HSD_FSM_SYS_ACQ_C,
                  PS_FILE_ID_CNAS_HSD_FSM_SYS_ACQ_TBL_C,
                  PS_FILE_ID_CNAS_HSD_MAIN_C,
                  PS_FILE_ID_CNAS_HSD_MNTN_C,
                  PS_FILE_ID_CNAS_HSD_PRE_PROC_ACT_C,
                  PS_FILE_ID_CNAS_HSD_PRE_PROC_TBL_C,
                  PS_FILE_ID_CNAS_HSD_PROC_CARD_C,
                  PS_FILE_ID_CNAS_HSD_PROC_NVIM_C,
                  PS_FILE_ID_CNAS_HSD_SND_CAS_C,
                  PS_FILE_ID_CNAS_HSD_SND_HLU_C,
                  PS_FILE_ID_CNAS_HSD_SND_HSM_C,
                  PS_FILE_ID_CNAS_HSD_SND_INTERNAL_MSG_C,
                  PS_FILE_ID_CNAS_HSD_SND_MSCC_C,
                  PS_FILE_ID_CNAS_HSD_SYS_ACQ_STRATEGY_C,
                  PS_FILE_ID_CNAS_HSD_SND_HRM_C,

                  PS_FILE_ID_CNAS_XREG_FSM_SWITCH_ON_C,
                  PS_FILE_ID_CNAS_XREG_FSM_SWITCH_ON_TBL_C,
                  PS_FILE_ID_CNAS_HSD_SND_RRM_C,
                  PS_FILE_ID_CNAS_HSD_AVOID_STRATEGY_C,

                  PS_FILE_ID_CNAS_HSD_FSM_POWER_SAVE_C,
                  PS_FILE_ID_CNAS_HSD_FSM_POWER_SAVE_TBL_C,
                  PS_FILE_ID_CNAS_HSD_FSM_INTER_SYS_C,
                  PS_FILE_ID_CNAS_HSD_FSM_INTER_SYS_TBL_C,
                  PS_FILE_ID_CNAS_HSD_SND_EHSM_C,

                  PS_FILE_ID_CNAS_HSD_FSM_BG_SRCH_C,
                  PS_FILE_ID_CNAS_HSD_FSM_BG_SRCH_TBL_C,

                  PS_FILE_ID_CNAS_HSD_FSM_OOC_NTF_C,
                  PS_FILE_ID_CNAS_HSD_FSM_OOC_NTF_TBL_C,

                  PS_FILE_ID_CNAS_HSM_MAIN_C,
                  PS_FILE_ID_CNAS_HSM_CTX_C,
                  PS_FILE_ID_CNAS_HSM_DECODE_C,
                  PS_FILE_ID_CNAS_HSM_ENCODE_C,
                  PS_FILE_ID_CNAS_HSM_FSM_CACHED_MSG_PRI_MNMT_C,
                  PS_FILE_ID_CNAS_HSM_FSM_MAIN_C,
                  PS_FILE_ID_CNAS_HSM_FSM_SESSION_ACT_C,
                  PS_FILE_ID_CNAS_HSM_FSM_SESSION_DEACT_C,
                  PS_FILE_ID_CNAS_HSM_FSM_TBL_C,
                  PS_FILE_ID_CNAS_HSM_FSM_UATI_REQUEST_C,
                  PS_FILE_ID_CNAS_HSM_PREPROC_ACT_C,
                  PS_FILE_ID_CNAS_HSM_PROC_NVIM_C,
                  PS_FILE_ID_CNAS_HSM_SND_AS_C,
                  PS_FILE_ID_CNAS_HSM_SND_TTF_C,
                  PS_FILE_ID_CNAS_HSM_SND_HSD_C,
                  PS_FILE_ID_CNAS_HSM_SND_APS_C,
                  PS_FILE_ID_CNAS_HSM_SND_HLU_C,
                  PS_FILE_ID_CNAS_HSM_PRE_PROC_TBL_C,
                  PS_FILE_ID_CNAS_HSM_COM_FUNC_C,
                  PS_FILE_ID_CNAS_HSM_SND_INTERNAL_MSG_C,
                  PS_FILE_ID_CNAS_HSM_SND_HRM_C,

                  PS_FILE_ID_CNAS_EHSM_CTX_C,
                  PS_FILE_ID_CNAS_EHSM_TIMER_MGMT_C,
                  PS_FILE_ID_CNAS_EHSM_FSM_MAIN_TBL_C,
                  PS_FILE_ID_CNAS_EHSM_FSM_MAIN_C,
                  PS_FILE_ID_CNAS_EHSM_MNTN_C,
                  PS_FILE_ID_CNAS_EHSM_MAIN_C,
                  PS_FILE_ID_CNAS_EHSM_PRE_PROC_ACT_C,
                  PS_FILE_ID_CNAS_EHSM_PRE_PROC_TBL_C,
                  PS_FILE_ID_CNAS_EHSM_SND_APS_C,
                  PS_FILE_ID_CNAS_EHSM_SND_ESM_C,
                  PS_FILE_ID_CNAS_EHSM_SND_HSD_C,
                  PS_FILE_ID_CNAS_EHSM_SND_HSM_C,
                  PS_FILE_ID_CNAS_EHSM_SND_INTERNAL_MSG_C,
                  PS_FILE_ID_CNAS_EHSM_SND_PPP_C,
                  PS_FILE_ID_CNAS_EHSM_FSM_ACTIVATING_C,
                  PS_FILE_ID_CNAS_EHSM_FSM_ACTIVATING_TBL_C,
                  PS_FILE_ID_CNAS_EHSM_FSM_DEACTIVATING_C,
                  PS_FILE_ID_CNAS_EHSM_FSM_DEACTIVATING_TBL_C,
                  PS_FILE_ID_CNAS_EHSM_PROC_NVIM_C,
                  PS_FILE_ID_CNAS_EHSM_COM_FUNC_C,
                  PS_FILE_ID_CNAS_EHSM_MSG_PRIO_COMPARE_C,
                  PS_FILE_ID_CNAS_EHSM_ENCODE_C,
                  PS_FILE_ID_CNAS_EHSM_DECODE_C,
                  PS_FILE_ID_CNAS_EHSM_SND_RRM_C,
                  PS_FILE_ID_CNAS_EHSM_SND_MSCC_C,

                  PS_FILE_ID_CNAS_EHSM_LOG_PRIVACY_C,

                  PS_FILE_ID_CNAS_EHSM_MSG_CHK_C,

                  PS_FILE_ID_CNAS_HSM_SND_MMA_C,


                  PS_FILE_ID_CNAS_HSM_MNTN_C,



                  PS_FILE_ID_CNAS_HSM_SND_RRM_C,
                  PS_FILE_ID_CNAS_HSM_KEEP_ALIVE_C,
                  PS_FILE_ID_CNAS_HSM_SND_EHSM_C,
                  PS_FILE_ID_CNAS_HSM_PROC_USIM_C,
                  PS_FILE_ID_CNAS_HSM_FSM_SWITCH_ON_C,
                  PS_FILE_ID_CNAS_HSM_FSM_CONN_MNMT_C,

                  PS_FILE_ID_CNAS_HRM_CTX_C,
                  PS_FILE_ID_CNAS_HRM_MAIN_C,
                  PS_FILE_ID_CNAS_HRM_COMM_FUNC_C,
                  PS_FILE_ID_CNAS_HRM_SND_CAS_C,
                  PS_FILE_ID_CNAS_HRM_SND_TTF_C,
                  PS_FILE_ID_CNAS_HRM_SND_HLU_C,
                  PS_FILE_ID_CNAS_HRM_SND_HSM_C,
                  PS_FILE_ID_CNAS_HRM_SND_HSD_C,
                  PS_FILE_ID_CNAS_HRM_PROC_CAS_C,
                  PS_FILE_ID_CNAS_HRM_PROC_TTF_C,
                  PS_FILE_ID_CNAS_HRM_PROC_HLU_C,
                  PS_FILE_ID_CNAS_HRM_PROC_HSM_C,
                  PS_FILE_ID_CNAS_HRM_PROC_HSD_C,
                  PS_FILE_ID_CNAS_HRM_LOG_PRIVACY_C,

                  PS_FILE_ID_ACPU_LOG_FILTER_C,
                  PS_FILE_ID_CCPU_LOG_FILTER_C,
                  PS_FILE_ID_GU_NAS_LOG_FILTER_C,
                  PS_FILE_ID_LOG_FILTER_COMM_C,

                  PS_FILE_ID_TAF_LOG_PRIVACY_MATCH_C,


                  PS_FILE_ID_NAS_DYNLOAD_API_C,
                  PS_FILE_ID_NAS_DYNLOAD_MNTN_C,
                  PS_FILE_ID_CNAS_XREG_MNTN_C,
                  PS_FILE_ID_NASLCSMAIN_C,
                  PS_FILE_ID_NASLCSEMMMSGPROC_C,
                  PS_FILE_ID_NASLCSTAFMSGPROC_C,
                  PS_FILE_ID_NASLCSTIMERMSGPROC_C,
                  PS_FILE_ID_NASLCSLPPMSGPROC_C,
                  PS_FILE_ID_NASLCSPUBLIC_C,
                  PS_FILE_ID_NASLCSMMMSGPROC_C,
                  PS_FILE_ID_NASLCSOM_C,

                  PS_FILE_ID_NASLCSENCODE_C,
                  PS_FILE_ID_NASLCSDECODE_C,
                  PS_FILE_ID_GULPS_SHAREMEM_C,
                  PS_FILE_ID_CNAS_EHSM_SND_NRSM_C,
                  PS_FILE_ID_BUTT

}PS_FILE_ID_DEFINE_ENUM;

typedef unsigned long  PS_FILE_ID_DEFINE_ENUM_UINT32;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif


