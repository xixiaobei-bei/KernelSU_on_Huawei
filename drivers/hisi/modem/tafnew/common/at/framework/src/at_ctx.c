/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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

#include "at_ctx.h"

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
#if (VOS_OS_VER == VOS_LINUX)
#include <linux/pm_wakeup.h>
#else
#include "Linuxstub.h"
#endif
#endif

#include "securec.h"
#include "at_data_proc.h"
#include "mdrv_rfile.h"
#include "at_mbb_cmd.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_CTX_C

/* **************************AT的全局变量表 Begin***************************** */
/* AT与编译两次的PID对应表  */
#if (MULTI_MODEM_NUMBER >= 2)
const AT_ModemPidTab g_atModemPidTab[] = {
    { I0_UEPS_PID_GRM, I1_UEPS_PID_GRM, I2_UEPS_PID_GRM, 0 },
    { I0_UEPS_PID_DL, I1_UEPS_PID_DL, I2_UEPS_PID_DL, 0 },
    { I0_UEPS_PID_LL, I1_UEPS_PID_LL, I2_UEPS_PID_LL, 0 },
    { I0_UEPS_PID_SN, I1_UEPS_PID_SN, I2_UEPS_PID_SN, 0 },
    { I0_UEPS_PID_GAS, I1_UEPS_PID_GAS, I2_UEPS_PID_GAS, 0 },
    { I0_WUEPS_PID_MM, I1_WUEPS_PID_MM, I2_WUEPS_PID_MM, 0 },
    { I0_WUEPS_PID_MMC, I1_WUEPS_PID_MMC, I2_WUEPS_PID_MMC, 0 },
    { I0_WUEPS_PID_GMM, I1_WUEPS_PID_GMM, I2_WUEPS_PID_GMM, 0 },
    { I0_WUEPS_PID_MMA, I1_WUEPS_PID_MMA, I2_WUEPS_PID_MMA, 0 },
    { I0_WUEPS_PID_CC, I1_WUEPS_PID_CC, I2_WUEPS_PID_CC, 0 },
    { I0_WUEPS_PID_SS, I1_WUEPS_PID_SS, I2_WUEPS_PID_SS, 0 },
    { I0_WUEPS_PID_TC, I1_WUEPS_PID_TC, I2_WUEPS_PID_TC, 0 },
    { I0_WUEPS_PID_SMS, I1_WUEPS_PID_SMS, I2_WUEPS_PID_SMS, 0 },
    { I0_WUEPS_PID_RABM, I1_WUEPS_PID_RABM, I2_WUEPS_PID_RABM, 0 },
    { I0_WUEPS_PID_SM, I1_WUEPS_PID_SM, I2_WUEPS_PID_SM, 0 },
    { I0_WUEPS_PID_ADMIN, I1_WUEPS_PID_ADMIN, I2_WUEPS_PID_ADMIN, 0 },
    { I0_WUEPS_PID_TAF, I1_WUEPS_PID_TAF, I2_WUEPS_PID_TAF, 0 },
    { I0_UEPS_PID_DSM, I1_UEPS_PID_DSM, I2_UEPS_PID_DSM, 0 },
    { I0_UEPS_PID_CCM, I1_UEPS_PID_CCM, I2_UEPS_PID_CCM, 0 },
    { I0_WUEPS_PID_VC, I1_WUEPS_PID_VC, I2_WUEPS_PID_VC, 0 },
    { I0_WUEPS_PID_DRV_AGENT, I1_WUEPS_PID_DRV_AGENT, I2_WUEPS_PID_DRV_AGENT, 0 },
    { I0_UEPS_PID_MTA, I1_UEPS_PID_MTA, I2_UEPS_PID_MTA, 0 },
    { I0_DSP_PID_GPHY, I1_DSP_PID_GPHY, I2_DSP_PID_GPHY, 0 },
    { I0_DSP_PID_SLEEP, I1_DSP_PID_SLEEP, I2_DSP_PID_SLEEP, 0 },
    { I0_DSP_PID_APM, I1_DSP_PID_APM, I2_DSP_PID_APM, 0 },
    { I0_WUEPS_PID_SLEEP, I1_WUEPS_PID_SLEEP, I2_WUEPS_PID_SLEEP, 0 },
    { I0_WUEPS_PID_WRR, I1_WUEPS_PID_WRR, 0, 0 },
    { I0_WUEPS_PID_WCOM, I1_WUEPS_PID_WCOM, 0, 0 },
    { I0_DSP_PID_WPHY, I1_DSP_PID_WPHY, 0, 0 },
#if (FEATURE_MODEM1_SUPPORT_LTE == FEATURE_ON)
    { I0_MSP_L4_L4A_PID, I1_MSP_L4_L4A_PID, I0_MSP_L4_L4A_PID, 0 },
    { I0_MSP_LDSP_PID, I1_MSP_LDSP_PID, I0_MSP_LDSP_PID, 0 },
    { I0_MSP_SYS_FTM_PID, I1_MSP_SYS_FTM_PID, I0_MSP_SYS_FTM_PID, 0 },
    { I0_PS_PID_IMSA, I1_PS_PID_IMSA, I0_PS_PID_IMSA, 0 },
    { I0_PS_PID_NAS_COMM, I1_PS_PID_NAS_COMM, I2_PS_PID_NAS_COMM, 0 }
#endif
};
#endif

VOS_UINT32 g_ctzuFlag = 0;

/* AT模块公共上下文 */
AT_CommCtx g_atCommCtx;

/* AT模块与Modem相关的上下文 */
AT_ModemCtx g_atModemCtx[MODEM_ID_BUTT];

/* AT模块与Client相关的上下文 */
AT_ClientCtx g_atClientCtx[AT_MAX_CLIENT_NUM];

/* AT模块复位相关的上下文 */
AT_ResetCtx g_atResetCtx;

/* **************************AT的全局变量表 End***************************** */

/* ********************************后续需要调整的************************************ */
/* 是否具有权限标志(控制DIAG/SHELL口的权限) */
AT_E5_RightFlagUint32 g_ate5RightFlag;

/*
 * 由于 g_stATDislogPwd 中的 DIAG口的状态要放入备份NV列表; 而密码不用备份
 * 故将 g_stATDislogPwd 中的密码废弃, 仅使用其中的 DIAG 口状态;
 * 重新定义NV项用来保存密码
 */
VOS_INT8 g_atOpwordPwd[AT_OPWORD_PWD_LEN + 1];

/* 用于当前是否有权限操作AT端口 */
AT_RightOpenFlagInfo g_atRightOpenFlg;

/* 使能禁止SD卡时需要的密码 */
VOS_INT8 g_ate5DissdPwd[AT_DISSD_PWD_LEN + 1];

/* 命令不支持提示字串 */
VOS_UINT8 g_atCmdNotSupportStr[AT_NOTSUPPORT_STR_LEN + 4];

AT_ABORT_CmdCtx g_atAbortCmdCtx; /* 用于保存打断的信息 */

/* 纪录拨号错误码是否使能 */
PPP_DIAL_ERR_CODE_ENUM g_pppDialErrCodeRpt;

AT_DIAL_ConnectDisplayRate g_dialConnectDisplayRate = {0};

/* UE下行能力: 包含协议版本和能力值 */
AT_DOWNLINK_RateCategory g_atDlRateCategory;

VOS_UINT8 g_atS3 = 13; /* <CR> */
VOS_UINT8 g_atS4 = 10; /* <LF> */
VOS_UINT8 g_atS5 = 8;  /* <DEL> */
/* Number of seconds to wait before blind dialling:default value = 2 */
VOS_UINT8 g_atS6 = 2;
VOS_UINT8 g_atS7 = 50; /* Number of seconds in which connection must be established or call will be disconnected,
                          default value = 50(refer to Q) */

AT_CMEE_TYPE g_atCmeeType; /* E5错误码默认错误编号 */

TAF_UINT32 g_stkFunctionFlag = TAF_FALSE;

/* ********************************CC Begin************************************ */
/* ********************************CC End************************************ */

/* ********************************SMS Begin************************************ */
MN_MSG_Class0TailorUint8 g_class0Tailor = MN_MSG_CLASS0_DEF;
/* ********************************SMS End************************************ */

/* ********************************NET Begin************************************ */
VOS_UINT16                  g_reportCregActParaFlg = VOS_FALSE;
CREG_CGREG_CI_RPT_BYTE_ENUM g_ciRptByte            = CREG_CGREG_CI_RPT_TWO_BYTE;

/* ********************************NET End************************************ */
#if (FEATURE_LTE == FEATURE_ON)
TAF_NVIM_LteRsrpCfg g_rsrpCfg;
TAF_NVIM_LteRscpCfg g_rscpCfg;
TAF_NVIM_LteEcioCfg g_ecioCfg;

AT_AccessStratumRelease g_releaseInfo;
#endif

AT_SS_CustomizePara g_atSsCustomizePara = {0};

AT_TRACE_MsgidTab g_atTraceMsgIdTab[AT_CLIENT_ID_BUTT];

#if (FEATURE_MBB_CUST == FEATURE_ON)
#if (FEATURE_PHONE_USIM == FEATURE_OFF)
VOS_UINT32 g_stkReportAppFlag[MODEM_ID_BUTT];
#endif
#endif

/*lint -e648 -e598 -e845 */

const AT_ClientCfgMapTab g_atClientCfgMapTbl[] = {
    AT_CLIENT_CFG_ELEMENT(PCUI),   AT_CLIENT_CFG_ELEMENT(CTRL),  AT_CLIENT_CFG_ELEMENT(PCUI2),
    AT_CLIENT_CFG_ELEMENT(MODEM),  AT_CLIENT_CFG_ELEMENT(NDIS),  AT_CLIENT_CFG_ELEMENT(UART),
#if (FEATURE_AT_HSUART == FEATURE_ON)
    AT_CLIENT_CFG_ELEMENT(HSUART),
#endif
    AT_CLIENT_CFG_ELEMENT(SOCK),

    AT_CLIENT_CFG_ELEMENT(APP),    AT_CLIENT_CFG_ELEMENT(APP1),
#if (FEATURE_VCOM_EXT == FEATURE_ON)
    AT_CLIENT_CFG_ELEMENT(APP2),   AT_CLIENT_CFG_ELEMENT(APP3),  AT_CLIENT_CFG_ELEMENT(APP4),
    AT_CLIENT_CFG_ELEMENT(APP5),   AT_CLIENT_CFG_ELEMENT(APP6),  AT_CLIENT_CFG_ELEMENT(APP7),
    AT_CLIENT_CFG_ELEMENT(APP8),   AT_CLIENT_CFG_ELEMENT(APP9),  AT_CLIENT_CFG_ELEMENT(APP10),
    AT_CLIENT_CFG_ELEMENT(APP11),  AT_CLIENT_CFG_ELEMENT(APP12), AT_CLIENT_CFG_ELEMENT(APP13),
    AT_CLIENT_CFG_ELEMENT(APP14),  AT_CLIENT_CFG_ELEMENT(APP15), AT_CLIENT_CFG_ELEMENT(APP16),
    AT_CLIENT_CFG_ELEMENT(APP17),  AT_CLIENT_CFG_ELEMENT(APP18), AT_CLIENT_CFG_ELEMENT(APP19),
    AT_CLIENT_CFG_ELEMENT(APP20),  AT_CLIENT_CFG_ELEMENT(APP21), AT_CLIENT_CFG_ELEMENT(APP22),
    AT_CLIENT_CFG_ELEMENT(APP23),  AT_CLIENT_CFG_ELEMENT(APP24), AT_CLIENT_CFG_ELEMENT(APP25),
    AT_CLIENT_CFG_ELEMENT(APP26),

    AT_CLIENT_CFG_ELEMENT(APP27),  AT_CLIENT_CFG_ELEMENT(APP28), AT_CLIENT_CFG_ELEMENT(APP29),
    AT_CLIENT_CFG_ELEMENT(APP30),  AT_CLIENT_CFG_ELEMENT(APP31), AT_CLIENT_CFG_ELEMENT(APP32),
    AT_CLIENT_CFG_ELEMENT(APP33),  AT_CLIENT_CFG_ELEMENT(APP34), AT_CLIENT_CFG_ELEMENT(APP35),
    AT_CLIENT_CFG_ELEMENT(APP36),  AT_CLIENT_CFG_ELEMENT(APP37), AT_CLIENT_CFG_ELEMENT(APP38),
    AT_CLIENT_CFG_ELEMENT(APP39),  AT_CLIENT_CFG_ELEMENT(APP40), AT_CLIENT_CFG_ELEMENT(APP41),
    AT_CLIENT_CFG_ELEMENT(APP42),  AT_CLIENT_CFG_ELEMENT(APP43), AT_CLIENT_CFG_ELEMENT(APP44),
    AT_CLIENT_CFG_ELEMENT(APP45),  AT_CLIENT_CFG_ELEMENT(APP46), AT_CLIENT_CFG_ELEMENT(APP47),
    AT_CLIENT_CFG_ELEMENT(APP48),  AT_CLIENT_CFG_ELEMENT(APP49), AT_CLIENT_CFG_ELEMENT(APP50),
    AT_CLIENT_CFG_ELEMENT(APP51),  AT_CLIENT_CFG_ELEMENT(APP52),

#endif

#if (FEATURE_IOT_CMUX == FEATURE_ON)
    AT_CLIENT_CFG_ELEMENT(CMUXAT), AT_CLIENT_CFG_ELEMENT(CMUXMDM), AT_CLIENT_CFG_ELEMENT(CMUXEXT),
    AT_CLIENT_CFG_ELEMENT(CMUXGPS),
#endif
};
/*lint +e648 +e598 +e845 */

const VOS_UINT8 g_atClientCfgMapTabLen = AT_ARRAY_SIZE(g_atClientCfgMapTbl);

#if ((FEATURE_UE_MODE_CDMA == FEATURE_ON) && (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT == FEATURE_ON) && \
     (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE == FEATURE_ON))

VOS_UINT8  g_currEncVoiceDataWriteFileNum = 0;
VOS_UINT32 g_atCurrEncVoiceDataCount      = 0;
VOS_UINT8  g_atCurrEncVoiceTestFileNum    = 0;
TAF_CHAR   g_atCurrDocName[AT_TEST_ECC_FILE_NAME_MAX_LEN];

/*lint -e786*/
const TAF_CHAR *g_currEncVoiceDataWriteFilePath[] = { "",
                                                MODEM_LOG_ROOT "/ECC_TEST/Encrypted_call_data1.txt",
                                                MODEM_LOG_ROOT "/ECC_TEST/Encrypted_call_data2.txt",
                                                MODEM_LOG_ROOT "/ECC_TEST/Encrypted_call_data3.txt",
                                                MODEM_LOG_ROOT "/ECC_TEST/Encrypted_call_data4.txt",
                                                MODEM_LOG_ROOT "/ECC_TEST/Encrypted_call_data5.txt" };
/*lint +e786*/

#endif
#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
struct wakeup_source g_atWakeLock;
#endif

#if (FEATURE_MT_CALL_SMS_WAKELOCK == FEATURE_ON)
struct wakeup_source g_callWakeLock;
struct wakeup_source g_smsWakeLock;
#endif
#endif

#if (VOS_WIN32 == VOS_OS_VER)

const AT_USIMM_FileNumToId g_atSimFileNumToIdTab[] = {
    { 0x6F07, USIMM_GSM_EFIMSI_ID },     { 0x6F46, USIMM_GSM_EFSPN_ID },       { 0x6F31, USIMM_GSM_EFHPLMN_ID },
    { 0x6F53, USIMM_GSM_EFLOCIGPRS_ID }, { 0x6f61, USIMM_GSM_EFOPLMNWACT_ID }, { 0x6f62, USIMM_GSM_EFHPLMNACT_ID },
    { 0x6F7E, USIMM_GSM_EFLOCI_ID }
};

const VOS_UINT32 g_atSimFileNumToIdTabLen = AT_ARRAY_SIZE(g_atSimFileNumToIdTab);

const AT_USIMM_FileNumToId g_atUsimFileNumToIdTab[] = {
    { 0x3F00, USIMM_MF_ID },
    { 0x2F00, USIMM_DIR_ID },
    { 0x2FE2, USIMM_ICCID_ID },
    { 0x2F05, USIMM_PL_ID },
    { 0x2F06, USIMM_ARR_ID },
    { 0x7FFF, USIMM_USIM_ID },
    { 0x6F05, USIMM_USIM_EFLI_ID },
    { 0x6F06, USIMM_USIM_EFARR_ID },
    { 0x6F07, USIMM_USIM_EFIMSI_ID },
    { 0x6F08, USIMM_USIM_EFKEYS_ID },
    { 0x6F09, USIMM_USIM_EFKEYSPS_ID },
    { 0x6F15, USIMM_USIM_EFCSP_ID },
    { 0x6F2C, USIMM_USIM_EFDCK_ID },
    { 0x6F31, USIMM_USIM_EFHPPLMN_ID },
    { 0x6F32, USIMM_USIM_EFCNL_ID },
    { 0x6F37, USIMM_USIM_EFACMMAX_ID },
    { 0x6F38, USIMM_USIM_EFUST_ID },
    { 0x6F39, USIMM_USIM_EFACM_ID },
    { 0x6F3B, USIMM_USIM_EFFDN_ID },
    { 0x6F3C, USIMM_USIM_EFSMS_ID },
    { 0x6F3E, USIMM_USIM_EFGID1_ID },
    { 0x6F3F, USIMM_USIM_EFGID2_ID },
    { 0x6F40, USIMM_USIM_EFMSISDN_ID },
    { 0x6F41, USIMM_USIM_EFPUCT_ID },
    { 0x6F42, USIMM_USIM_EFSMSP_ID },
    { 0x6F43, USIMM_USIM_EFSMSS_ID },
    { 0x6F45, USIMM_USIM_EFCBMI_ID },
    { 0x6F46, USIMM_USIM_EFSPN_ID },
    { 0x6F47, USIMM_USIM_EFSMSR_ID },
    { 0x6F48, USIMM_USIM_EFCBMID_ID },
    { 0x6F49, USIMM_USIM_EFSDN_ID },
    { 0x6F4B, USIMM_USIM_EFEXT2_ID },
    { 0x6F4C, USIMM_USIM_EFEXT3_ID },
    { 0x6F4D, USIMM_USIM_EFBDN_ID },
    { 0x6F4E, USIMM_USIM_EFEXT5_ID },
    { 0x6F4F, USIMM_USIM_EFCCP2_ID },
    { 0x6F50, USIMM_USIM_EFCBMIR_ID },
    { 0x6F55, USIMM_USIM_EFEXT4_ID },
    { 0x6F56, USIMM_USIM_EFEST_ID },
    { 0x6F57, USIMM_USIM_EFACL_ID },
    { 0x6F58, USIMM_USIM_EFCMI_ID },
    { 0x6F5B, USIMM_USIM_EFSTART_HFN_ID },
    { 0x6F5C, USIMM_USIM_EFTHRESHOL_ID },
    { 0x6F60, USIMM_USIM_EFPLMNWACT_ID },
    { 0x6F61, USIMM_USIM_EFOPLMNWACT_ID },
    { 0x6F62, USIMM_USIM_EFHPLMNwACT_ID },
    { 0x6F73, USIMM_USIM_EFPSLOCI_ID },
    { 0x6F78, USIMM_USIM_EFACC_ID },
    { 0x6F7B, USIMM_USIM_EFFPLMN_ID },
    { 0x6F7E, USIMM_USIM_EFLOCI_ID },
    { 0x6F80, USIMM_USIM_EFICI_ID },
    { 0x6F81, USIMM_USIM_EFOCI_ID },
    { 0x6F82, USIMM_USIM_EFICT_ID },
    { 0x6F83, USIMM_USIM_EFOCT_ID },
    { 0x6FAD, USIMM_USIM_EFAD_ID },
    { 0x6FB1, USIMM_USIM_EFVGCS_ID },
    { 0x6FB2, USIMM_USIM_EFVGCSS_ID },
    { 0x6FB3, USIMM_USIM_EFVBS_ID },
    { 0x6FB4, USIMM_USIM_EFVBSS_ID },
    { 0x6FB5, USIMM_USIM_EFEMLPP_ID },
    { 0x6FB6, USIMM_USIM_EFAAEM_ID },
    { 0x6FB7, USIMM_USIM_EFECC_ID },
    { 0x6FC3, USIMM_USIM_EFHIDDENKEY_ID },
    { 0x6FC4, USIMM_USIM_EFNETPAR_ID },
    { 0x6FC5, USIMM_USIM_EFPNN_ID },
    { 0x6FC6, USIMM_USIM_EFOPL_ID },
    { 0x6FC7, USIMM_USIM_EFMBDN_ID },
    { 0x6FC8, USIMM_USIM_EFEXT6_ID },
    { 0x6FC9, USIMM_USIM_EFMBI_ID },
    { 0x6FCA, USIMM_USIM_EFMWIS_ID },
    { 0x6FCB, USIMM_USIM_EFCFIS_ID },
    { 0x6FCC, USIMM_USIM_EFEXT7_ID },
    { 0x6FCD, USIMM_USIM_EFSPDI_ID },
    { 0x6FCE, USIMM_USIM_EFMMSN_ID },
    { 0x6FCF, USIMM_USIM_EFEXT8_ID },
    { 0x6FD0, USIMM_USIM_EFMMSICP_ID },
    { 0x6FD1, USIMM_USIM_EFMMSUP_ID },
    { 0x6FD2, USIMM_USIM_EFMMSUCP_ID },
    { 0x6FD3, USIMM_USIM_EFNIA_ID },
    { 0x6FD4, USIMM_USIM_EFVGCSCA_ID },
    { 0x6FD5, USIMM_USIM_EFVBSCA_ID },
    { 0x6FD6, USIMM_USIM_EFGBAP_ID },
    { 0x6FD7, USIMM_USIM_EFMSK_ID },
    { 0x6FD8, USIMM_USIM_EFMUK_ID },
    { 0x6FD9, USIMM_USIM_EFEHPLMN_ID },
    { 0x6FDA, USIMM_USIM_EFGBANL_ID },
    { 0x6FDB, USIMM_USIM_EFEHPLMNPI_ID },
    { 0x6FDC, USIMM_USIM_EFLRPLMNSI_ID },
    { 0x6FDD, USIMM_USIM_EFNAFKCA_ID },
    { 0x6FDE, USIMM_USIM_EFSPNI_ID },
    { 0x6FDF, USIMM_USIM_EFPNNI_ID },
    { 0x6FE2, USIMM_USIM_EFNCPIP_ID },
    { 0x6FE3, USIMM_USIM_EFEPSLOCI_ID },
    { 0x6FE4, USIMM_USIM_EFEPSNSC_ID },
    { 0x6FE6, USIMM_USIM_EFUFC_ID },
    { 0x6FE7, USIMM_USIM_EFUICCIARI_ID },
    { 0x6FE8, USIMM_USIM_EFNASCONFIG_ID },
    { 0x6FEC, USIMM_USIM_EFPWS_ID },
    { 0x5F3A, USIMM_USIM_DFPHONEBOOK_ID },
    { 0x4F22, USIMM_USIM_EFPSC_ID },
    { 0x4F23, USIMM_USIM_EFCC_ID },
    { 0x4F24, USIMM_USIM_EFPUID_ID },
    { 0x4F30, USIMM_USIM_EFPBR_ID },
    { 0x5F3B, USIMM_USIM_DFGSM_ACCESS_ID },
    { 0x4F20, USIMM_USIM_EFKC_ID },
    { 0x4F52, USIMM_USIM_EFKCGPRS_ID },
    { 0x4F63, USIMM_USIM_EFCPBCCH_ID },
    { 0x4F64, USIMM_USIM_EFINVSCAN_ID },
    { 0x5F3C, USIMM_USIM_DFMEXE_ID },
    { 0x4F40, USIMM_USIM_EFMexE_ST_ID },
    { 0x4F41, USIMM_USIM_EFORPK_ID },
    { 0x4F42, USIMM_USIM_EFARPK_ID },
    { 0x4F43, USIMM_USIM_EFTPRK_ID },
    { 0x5F70, USIMM_USIM_DFSOLSA_ID },
    { 0x4F30, USIMM_USIM_EFSAI_ID },
    { 0x4F31, USIMM_USIM_EFSLL_ID },
    { 0x5F40, USIMM_USIM_DFWLAN_ID },
    { 0x4F41, USIMM_USIM_EFPSEUDO_ID },
    { 0x4F42, USIMM_USIM_EFUPLMNWLAN_ID },
    { 0x4F43, USIMM_USIM_EF0PLMNWLAN_ID },
    { 0x4F44, USIMM_USIM_EFUWSIDL_ID },
    { 0x4F45, USIMM_USIM_EFOWSIDL_ID },
    { 0x4F46, USIMM_USIM_EFWRI_ID },
    { 0x4F47, USIMM_USIM_EFHWSIDL_ID },
    { 0x4F48, USIMM_USIM_EFWEHPLMNPI_ID },
    { 0x4F49, USIMM_USIM_EFWHPI_ID },
    { 0x4F4A, USIMM_USIM_EFWLRPLMN_ID },
    { 0x4F4B, USIMM_USIM_EFHPLMNDAI_ID },
    { 0x5F50, USIMM_USIM_DFHNB_ID },
    { 0x4F81, USIMM_USIM_EFACSGL_ID },
    { 0x4F82, USIMM_USIM_EFCSGT_ID },
    { 0x4F83, USIMM_USIM_EFHNBN_ID },
    { 0x4F84, USIMM_USIM_EFOCSGL_ID },
    { 0x4F85, USIMM_USIM_EFOCSGT_ID },
    { 0x4F86, USIMM_USIM_EFOHNBN_ID },
    { 0X6F30, USIMM_GSM_EFPLMNSEL_ID },
    { 0x6F53, USIMM_GSM_EFLOCIGPRS_ID },
    { 0x6F20, USIMM_GSM_EFKC_ID },
    { 0x6F52, USIMM_GSM_EFKCGPRS_ID },

    { 0x4F01, USIMM_USIM_EF5GS3GPPLOCI_ID },
    { 0x4F03, USIMM_USIM_EF5GS3GPPNSC_ID },
    { 0x4F05, USIMM_USIM_EF5GAUTHKEYS_ID },
    { 0x4F06, USIMM_USIM_EFUAC_AIC_ID },
    { 0x4F07, USIMM_USIM_EFSUCI_CALC_INFO_ID },
    { 0x4F08, USIMM_USIM_EFOPL_5GS_ID },
    { 0x4F0A, USIMM_USIM_EFROUTING_INDICATOR_ID },
};

const VOS_UINT32 g_atUsimFileNumToIdTabLen = AT_ARRAY_SIZE(g_atUsimFileNumToIdTab);

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
const AT_USIMM_FileNumToId g_atCsimFileNumToIdTab[] = {
    { 0x6F22, USIMM_CSIM_EFIMSIM_ID },
    { 0x6F47, USIMM_CSIM_EFECC_ID },
};

const VOS_UINT32 g_atCsimFileNumToIdTabLen = AT_ARRAY_SIZE(g_atCsimFileNumToIdTab);

#endif

#endif

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
AT_DEVICE_CmdCtrl g_atDevCmdCtrl = {0};
#else
AT_MT_Info g_mtInfoCtx = {0};
#endif
/* 速率气泡显示的速率表数组,由于是遵照CONNECT <Rate>的格式，故预留一个空格 */
const TAF_CHAR *g_pppDialRateDisplay[] = {
    " 9600",     " 85600",    " 107000",  " 236800",   " 296000",   " 384000",   " 3600000",  " 7200000",
    " 10200000", " 14400000", " 1800000", " 21600000", " 28800000", " 43200000", " 57600000", " 86400000"
};

/* 速率气泡显示的速率表数组,NV定制值 */
const VOS_CHAR *g_dialRateDisplayNv[] = {
    " 53600", " 236800", " 384000", " 1800000", " 3600000", " 7200000",
};

#if (FEATURE_LTE == FEATURE_ON)
/* 速率气泡显示的速率表数组,NV定制值 */
const AT_DisplayRatePair g_lteRateDisplay[AT_UE_LTE_CATEGORY_NUM_MAX] = {
    { " 1000000", " 1000000" },        /* category0上下行理论最大速率 */
    { " 10000000", " 5000000" },       /* category1上下行理论最大速率 */
    { " 50000000", " 25000000" },      /* category2上下行理论最大速率 */
    { " 100000000", " 50000000" },     /* category3上下行理论最大速率 */
    { " 150000000", " 50000000" },     /* category4上下行理论最大速率 */
    { " 300000000", " 75000000" },     /* category5上下行理论最大速率 */
    { " 300000000", " 50000000" },     /* category6上下行理论最大速率 */
    { " 300000000", " 100000000" },    /* category7上下行理论最大速率 */
    { " 3000000000", " 1500000000" },  /* category8上下行理论最大速率 */
    { " 450000000", " 50000000" },     /* category9上下行理论最大速率 */
    { " 450000000", " 100000000" },    /* category10上下行理论最大速率 */
    { " 600000000", " 50000000" },     /* category11上下行理论最大速率 */
    { " 600000000", " 100000000" },    /* category12上下行理论最大速率 */
    { " 390000000", " 150000000" },    /* category13上下行理论最大速率 */
    { " 3900000000", " 9500000000" },  /* category14上下行理论最大速率 */
    { " 750000000", " 220000000" },    /* category15上下行理论最大速率 */
    { " 980000000", " 100000000" },    /* category16上下行理论最大速率 */
    { " 25000000000", " 2100000000" }, /* category17上下行理论最大速率 */
    { " 1200000000", " 210000000" },   /* category18上下行理论最大速率 */
    { " 1600000000", " 13000000000" }, /* category19上下行理论最大速率 */
    { " 1900000000", " 310000000" },   /* category20上下行理论最大速率 */
    { " 1300000000", " 300000000" },   /* category21上下行理论最大速率 */
};
#endif

TAF_UINT8 g_stkCmdQualify = 0x0;

/* 纪录查询错误码的类型 */
TAF_UINT32 g_errType = 1;

VOS_UINT32 g_wifiFreq;
VOS_UINT32 g_wifiRate;
VOS_UINT32 g_wifiMode;
VOS_INT32  g_wifiPower;

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 g_wifiRf = 0xffffffffU;
#endif

VOS_UINT g_ucastWifiRxPkts;

#if (FEATURE_AT_CMD_TRUST_LIST == FEATURE_ON)
/* 设置和查询命令Trust名单映射表 */
static const AT_CmdTrustListMapTbl g_atUserCmdTrustListTbl[] = {
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    { "^CSGIDSRCH",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^APCONNST",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^ACTPDPSTUB",         TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SCICHG",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^COPS",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CQST",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^PORTCONCURRENT",     TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SENSOR",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^FASTDORM",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^LOGPORT",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CSND",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CSIM",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^ANQUERY",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^OPWORD",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CPWORD",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^DISLOG",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SHELL",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SPWORD",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NVWRPART",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^APPWRONREG",         TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^APHPLMN",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^MMPLMNINFO",         TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CURSM",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SENDDATA",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
#endif
    /* 通用操作命令 */
    { "+CSCS",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CMEE",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "V",                   TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "V0",                  TRUST_LIST_CMD_CLAC_IS_INVISIBLE },
    { "V1",                  TRUST_LIST_CMD_CLAC_IS_INVISIBLE },
    { "+CIMI",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGMI",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+GMI",                TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGMM",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+GMM",                TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGMR",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+GMR",                TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGSN",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+GSN",                TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^HWVER",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^HVER",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^FHVER",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^PFVER",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "S0",                  TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "S3",                  TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "S4",                  TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "S5",                  TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "E",                   TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "E0",                  TRUST_LIST_CMD_CLAC_IS_INVISIBLE },
    { "E1",                  TRUST_LIST_CMD_CLAC_IS_INVISIBLE },
    { "&F",                  TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "&F0",                 TRUST_LIST_CMD_CLAC_IS_INVISIBLE },
    { "+CRC",                TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "I",                   TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "X",                   TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "X0",                  TRUST_LIST_CMD_CLAC_IS_INVISIBLE },
    { "X1",                  TRUST_LIST_CMD_CLAC_IS_INVISIBLE },
    { "X2",                  TRUST_LIST_CMD_CLAC_IS_INVISIBLE },
    { "X3",                  TRUST_LIST_CMD_CLAC_IS_INVISIBLE },
    { "X4",                  TRUST_LIST_CMD_CLAC_IS_INVISIBLE },
    { "+GCAP",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^HS",                 TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CLAC",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CURC",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^U2DIAG",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^AT2OM",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CPAS",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^LOCINFO",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^EOPLMN",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CEUS",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+WS46",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    /* 电话管理命令 */
    { "+CGCLASS",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CFUN",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CSQ",                TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+COPS",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CREG",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGREG",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGATT",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SYSCFG",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^ICCID",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CGCATT",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CFPLMN",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SYSINFO",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SRVST",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^MODE",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CPLS",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CPOL",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^RSSI",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CSNR",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^WFREQLOCK",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^FREQLOCK",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^M2MFREQLOCK",        TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^TIME",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^TIMEQRY",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CTZR",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^PLMN",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NVM",                TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^XLEMA",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^ACINFO",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+COPN",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^REJINFO",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NETSCAN",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^GFREQLOCK",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SYSCFGEX",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SYSINFOEX",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CEREG",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^TTYMODE",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^BATTERYINFO",        TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+C5GREG",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+C5GPNSSAI",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+C5GNSSAI",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+C5GNSSAIRDP",        TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CSUEPOLICY",         TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    /* 电路域业务命令 */
    { "+CMOD",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CBST",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CLVL",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CLCC",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CHUP",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CSTA",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+VTS",                TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "A",                   TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "H",                   TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "H0",                  TRUST_LIST_CMD_CLAC_IS_INVISIBLE },
    { "D",                   TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^DTMF",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^VMSET",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CDUR",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CMUT",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^APDS",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CCSERR",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^LTECS",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CLCC",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^IMSSWITCH",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CEVDP",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^IMSDOMAINCFG",       TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^IMSREGDOMAIN",       TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NICKNAME",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CCWAI",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^REJCALL",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CSCHANNELINFO",      TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CVHU",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^APLANADDR",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CEMODE",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    /* 补充业务命令 */
    { "+CLIP",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CLIR",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+COLP",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CCUG",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CSSN",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CHLD",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CCWA",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CCFC",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CTFR",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CUSD",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CUUS1",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^USSDMODE",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CHLD",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CMOLR",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CMOLRE",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CMTLR",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CMTLRA",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CIND",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CSDF",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CCLK",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^XCPOSRRPT",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CPULOAD",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    /* 短信业务命令 */
    { "+CSMS",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CMGF",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CSCA",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGSMS",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CSMP",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CSDH",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CNMI",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CNMA",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CMGR",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CMGL",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CMGD",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CMGW",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CMGS",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CMSS",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CMGC",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CPMS",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CMSR",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CMGI",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CMMT",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CMMS",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CSCB",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CSASM",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SMSDOMAIN",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SMSNASCAP",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    /* 分组域业务命令 */
    { "+CGEQREQ",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGEQMIN",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGTFT",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGAUTO",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGPADDR",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGEQNEG",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGDCONT",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGDSCONT",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGACT",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGANS",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGCMOD",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGDATA",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CGDNS",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^GLASTERR",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^DIALMODE",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CIPHERQRY",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^DNSQUERY",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGCONTRDP",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGSCONTRDP",         TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGTFTRDP",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^AUTHDATA",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^IPV6CAP",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NDISDUP",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^DSFLOWCLR",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^DSFLOWQRY",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^DSFLOWRPT",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CHDATA",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGPIAF",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CEER",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGEREP",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^DCONNSTAT",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+C5GQOS",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+C5GQOSRDP",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^ROAMPDPTYPE",        TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    /* 安全管理命令 */
    { "+CLCK",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CPWD",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CPIN",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CPIN",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CPIN2",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^IMEIVERIFY",         TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    /* (U)SIM相关命令 */
    { "+CPBS",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CPBR",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CPBF",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CPBW",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CPBR",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CPBW",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SCPBR",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SCPBW",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CNUM",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CRSM",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SIMST",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SIMSQ",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^PNN",                TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SPN",                TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^OPL",                TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CPNN",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CARDMODE",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CCHC",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CCHO",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CCHP",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^BWT",                TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SINGLEMODEMDUALSLOT",TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CARDHISTORYATR",     TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^HVSST",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^DIESN",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    /* 测试类私有命令 */
    { "^CELLINFO",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NRFREQLOCK",         TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    /* NDIS特性命令 */
    { "^DHCP",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CRPN",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NDISSTATQRY",        TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^DHCPV6",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    /* 与AP对接命令 */
    { "^USERAGENTCFG",       TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^RCSSWITCH",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^IMSSRVSTATRPT",      TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^IMSSRVSTATUS",       TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^APRPTPORTSEL",       TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CPOS",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CPOSR",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CPSERR",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CLTEROAMALLOW",      TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SWVER",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^USERSRVSTATE",       TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+ECID",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^HPLMN",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^MONSC",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^MONNC",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^TIMESET",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^IMSSMSCFG",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^IMSREGERRRPT",       TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CIMSERR",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NVREFRESH",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^MONSSC",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^OVERHEATINGCFG",     TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^OVERHEATINGQRY",     TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NRBANDBLOCKLIST",    TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SMSANTIATTACKCAP",   TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SMSANTIATTACK",      TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NRSSBID",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^XCPOSR",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^ERRCCAPCFG",         TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^ERRCCAPQRY",         TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NRRCCAPCFG",         TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NRRCCAPQRY",         TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^LTEPWRCTRL",         TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NRPWRCTRL",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^LL2COMQRY",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NL2COMCFG",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NL2COMQRY",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^LENDC",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^ACTIVENCFG",         TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^TXPOWER",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NTXPOWER",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^PHYCOMCFG",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^LL2COMCFG",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NRCELLBAND",         TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    /* LTE相关命令 */
    { "+CIREG",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CIREP",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^ROAMIMSSERVICE",     TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGEQOS",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "+CGEQOSRDP",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CHISFREQ",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^LTELOWPOWER",        TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^DMUSER",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^LWCLASH",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^LCACELLEX",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    /* GUC装备命令 */
    { "+CMUX",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NVRD",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NVWR",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NVRDLEN",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NVRDEX",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NVWREX",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^BSN",                TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^PLATFORM",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^TMODE",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^FCHAN",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^FTXON",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "$QCDMG",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^TMMI",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SFEATURE",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^PRODNAME",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^FWAVE",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^EQVER",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^INFORBU",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^VERSION",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^FACINFO",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^MDATE",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^FRSSI",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^FRXON",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SN",                 TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CSVER",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^QOS",                TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SDOMAIN",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^GTIMER",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^RSIM",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^PHYNUM",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CALLSRV",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^TSELRF",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^QRYNVRESUME",        TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CHIPTEMP",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^IDENTIFYSTART",      TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^IDENTIFYEND",        TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^PHONEPHYNUM",        TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SIMLOCKDATAWRITE",   TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SIMLOCKDATAREAD",    TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^HUK",                TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^APRAINFO",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SIMLOCKUNLOCK",      TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CERSSI",             TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CNMR",               TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^WLTHRESHOLDCFG",     TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^FPLLSTATUS",         TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "O",                   TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "O0",                  TRUST_LIST_CMD_CLAC_IS_INVISIBLE },
    { "&C0",                 TRUST_LIST_CMD_CLAC_IS_INVISIBLE },
    { "&C1",                 TRUST_LIST_CMD_CLAC_IS_INVISIBLE },
    { "&D0",                 TRUST_LIST_CMD_CLAC_IS_INVISIBLE },
    { "&D1",                 TRUST_LIST_CMD_CLAC_IS_INVISIBLE },
    { "&D2",                 TRUST_LIST_CMD_CLAC_IS_INVISIBLE },
    { "&S0",                 TRUST_LIST_CMD_CLAC_IS_INVISIBLE },
    { "&S1",                 TRUST_LIST_CMD_CLAC_IS_INVISIBLE },
    { "+IPR",                TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^INFORRS",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^AUTHVER",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SECURESTATE",        TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^KCE",                TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SOCID",              TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^NVBACKUP",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^PMUDIESN",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SIMLOCKDATAREADEX",  TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SIMLOCKDATAWRITEEX", TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^DCXOTEMPCOMP",       TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^GNSSNTY",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^FTEMPRPT",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^FAGCGAIN",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^LOWPWRMODE",         TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^CCPUNIDDISABLE",     TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^SECDBGSTATE",        TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^AUTHKEYRD",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^FRFICMEM",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^FSERDESRT",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^I2STEST",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^UARTTEST",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^GODLOAD",            TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^RFICIDEX",           TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "^GETMODEMSCID",       TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    /* 基础命令（禁止放入DMT宏中） */
    { "&C",                  TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "&D",                  TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    { "&S",                  TRUST_LIST_CMD_CLAC_IS_VISIBLE },
    /* LTE&TDS装备命令 */
    { "^LTCOMMCMD",          TRUST_LIST_CMD_CLAC_IS_VISIBLE },
};
#endif

/* 判断是否为广播Client Id */
VOS_BOOL AT_IsBroadcastClientIndex(VOS_UINT8 indexNum)
{
    if (indexNum == AT_BROADCAST_CLIENT_INDEX_MODEM_0 || indexNum == AT_BROADCAST_CLIENT_INDEX_MODEM_1 ||
        indexNum == AT_BROADCAST_CLIENT_INDEX_MODEM_2) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

VOS_VOID AT_InitUsimStatus(ModemIdUint16 modemId)
{
    AT_USIM_InfoCtx *usimInfoCtx = VOS_NULL_PTR;

    usimInfoCtx = AT_GetUsimInfoCtxFromModemId(modemId);

    usimInfoCtx->cardStatus = USIMM_CARDAPP_SERVIC_BUTT;
    usimInfoCtx->cardType   = TAF_MMA_USIMM_CARD_TYPE_BUTT;

    usimInfoCtx->imsiLen = 0;
    (VOS_VOID)memset_s(usimInfoCtx->imsi, sizeof(usimInfoCtx->imsi), 0x00, sizeof(usimInfoCtx->imsi));

    usimInfoCtx->simsqStatus = TAF_MMA_SIMSQ_BUTT;

#if (FEATURE_MBB_CUST == FEATURE_ON)
#if (FEATURE_PHONE_USIM == FEATURE_OFF)
    usimInfoCtx->noCardMode = AT_NOCARD_MODE_OFF;
#endif
#endif
}

VOS_VOID AT_InitPlatformRatList(ModemIdUint16 modemId)
{
    AT_ModemSptRat *sptRat = VOS_NULL_PTR;
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    VOS_UINT8 *isCLMode = VOS_NULL_PTR;
#endif
    sptRat = AT_GetSptRatFromModemId(modemId);

    /* 默认情况下单板只支持GSM */
    sptRat->platformSptGsm      = VOS_TRUE;
    sptRat->platformSptWcdma    = VOS_FALSE;
    sptRat->platformSptLte      = VOS_FALSE;
    sptRat->platformSptUtralTdd = VOS_FALSE;

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    sptRat->platformSptNr = VOS_FALSE;
#endif

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    isCLMode  = AT_GetModemCLModeCtxAddrFromModemId(modemId);
    *isCLMode = VOS_FALSE;
#endif
}

VOS_VOID AT_InitCommPsCtx(VOS_VOID)
{
    AT_CommPsCtx *psCtx = VOS_NULL_PTR;
    VOS_UINT32    cnt;

    psCtx = AT_GetCommPsCtxAddr();

    (VOS_VOID)memset_s(psCtx, sizeof(AT_CommPsCtx), 0, sizeof(AT_CommPsCtx));

    psCtx->ipv6Capability = AT_IPV6_CAPABILITY_IPV4_ONLY;

    psCtx->ipv6AddrTestModeCfg = 0;

    psCtx->sharePdpFlag = VOS_FALSE;

    psCtx->spePort    = AT_INVALID_SPE_PORT;
    psCtx->ipfPortFlg = VOS_FALSE;

    psCtx->etherCap   = VOS_FALSE;

    /* 初始化CHDATA CFG */
    for (cnt = 0; cnt <= AT_CH_DATA_CHANNEL_BUTT; cnt++) {
        AT_CleanDataChannelCfg(&(psCtx->channelCfg[cnt]));
    }
}

VOS_VOID AT_InitCommPbCtx(VOS_VOID)
{
    AT_CommPbCtx *commPbCntxt = VOS_NULL_PTR;

    commPbCntxt = AT_GetCommPbCtxAddr();

    commPbCntxt->currIdx       = 0;
    commPbCntxt->lastIdx       = 0;
    commPbCntxt->singleReadFlg = VOS_FALSE;
}

AT_CmdProcCtx* AT_GetCmdProcCtxAddr(VOS_VOID)
{
    return &(g_atCommCtx.cmdProcCtx);
}

AT_AUTH_PUBKEYEX_CMD_PROC_CTX* AT_GetAuthPubkeyExCmdCtxAddr(VOS_VOID)
{
    return &(AT_GetCmdProcCtxAddr()->authPubkeyExCmdCtx);
}

AT_PhonePhynumCmdProcCtx* AT_GetPhonePhynumCmdCtxAddr(VOS_VOID)
{
    return &(AT_GetCmdProcCtxAddr()->phonePhynumCmdCtx);
}

AT_SIMLOCKDATAWRITEEX_CMD_PROC_CTX* AT_GetSimLockWriteExCmdCtxAddr(VOS_VOID)
{
    return &(AT_GetCmdProcCtxAddr()->simLockWriteExCmdCtx);
}

AT_VmsetCmdCtx* AT_GetCmdVmsetCtxAddr(VOS_VOID)
{
    return &(AT_GetCmdProcCtxAddr()->vmSetCmdCtx);
}

VOS_VOID AT_InitVmSetCtx(VOS_VOID)
{
    AT_VmsetCmdCtx *vmSetCmdCtx = VOS_NULL_PTR;

    vmSetCmdCtx = AT_GetCmdVmsetCtxAddr();

    vmSetCmdCtx->reportedModemNum = 0;
    vmSetCmdCtx->result           = AT_OK;
}

VOS_VOID AT_InitPhonePhynumSetCtx(VOS_VOID)
{
    AT_PhonePhynumCmdProcCtx *phynumSetCtx = VOS_NULL_PTR;

    phynumSetCtx = AT_GetPhonePhynumCmdCtxAddr();

    phynumSetCtx->typeValueLen   = 0;
    phynumSetCtx->phynumValueLen = 0;
    phynumSetCtx->hmacValueLen   = 0;
    phynumSetCtx->typeValue      = VOS_NULL_PTR;
    phynumSetCtx->phynumValue    = VOS_NULL_PTR;
    phynumSetCtx->hmacValue      = VOS_NULL_PTR;
}

VOS_VOID AT_InitSimlockWriteSetCtx(VOS_VOID)
{
    AT_SIMLOCKDATAWRITEEX_CMD_PROC_CTX *simlockWriteExCtx = VOS_NULL_PTR;

    simlockWriteExCtx = AT_GetSimLockWriteExCmdCtxAddr();

    simlockWriteExCtx->clientId                    = 0;
    simlockWriteExCtx->layer                       = 0;
    simlockWriteExCtx->curIdx                      = 0;
    simlockWriteExCtx->totalNum                    = 0;
    simlockWriteExCtx->simlockDataLen              = 0;
    simlockWriteExCtx->hmacLen                     = 0;
    simlockWriteExCtx->settingFlag                 = VOS_FALSE;
    simlockWriteExCtx->data                        = VOS_NULL_PTR;
    simlockWriteExCtx->hSimLockWriteExProtectTimer = VOS_NULL_PTR;

    (VOS_VOID)memset_s(simlockWriteExCtx->hmac, sizeof(simlockWriteExCtx->hmac), 0x00, sizeof(simlockWriteExCtx->hmac));
}

VOS_VOID AT_InitCmdProcCtx(VOS_VOID)
{
    AT_AUTH_PUBKEYEX_CMD_PROC_CTX *authPubkeyExCmdCtx = VOS_NULL_PTR;

    authPubkeyExCmdCtx = AT_GetAuthPubkeyExCmdCtxAddr();

    authPubkeyExCmdCtx->clientId                = 0;
    authPubkeyExCmdCtx->curIdx                  = 0;
    authPubkeyExCmdCtx->totalNum                = 0;
    authPubkeyExCmdCtx->paraLen                 = 0;
    authPubkeyExCmdCtx->settingFlag             = VOS_FALSE;
    authPubkeyExCmdCtx->data                    = VOS_NULL_PTR;
    authPubkeyExCmdCtx->hAuthPubkeyProtectTimer = VOS_NULL_PTR;

    AT_InitPhonePhynumSetCtx();
    AT_InitSimlockWriteSetCtx();

    AT_InitVmSetCtx();
}

VOS_VOID AT_InitMsgNumCtrlCtx(VOS_VOID)
{
    AT_CmdMsgNumCtrl *msgNumCtrlCtx = VOS_NULL_PTR;
    VOS_UINT32 i;
    msgNumCtrlCtx = AT_GetMsgNumCtrlCtxAddr();

    /* 锁初始化 */
    VOS_SpinLockInit(&(msgNumCtrlCtx->spinLock));

    for (i = 0; i < AT_MSG_QOSTYPE_BUT; i++) {
        msgNumCtrlCtx->msgCount[i] = 0;
    }
}

VOS_VOID AT_ClearAuthPubkeyCtx(VOS_VOID)
{
    AT_AUTH_PUBKEYEX_CMD_PROC_CTX *authPubkeyExCmdCtx = VOS_NULL_PTR;

    authPubkeyExCmdCtx = AT_GetAuthPubkeyExCmdCtxAddr();

    authPubkeyExCmdCtx->clientId    = 0;
    authPubkeyExCmdCtx->curIdx      = 0;
    authPubkeyExCmdCtx->totalNum    = 0;
    authPubkeyExCmdCtx->paraLen     = 0;
    authPubkeyExCmdCtx->settingFlag = VOS_FALSE;

    if (authPubkeyExCmdCtx->data != VOS_NULL_PTR) {
        PS_MEM_FREE(WUEPS_PID_AT, authPubkeyExCmdCtx->data);  //lint !e830
        authPubkeyExCmdCtx->data = VOS_NULL_PTR;
    }
}

VOS_VOID AT_RcvTiAuthPubkeyExpired(REL_TimerMsgBlock *tmrMsg)
{
    AT_ClearAuthPubkeyCtx();
}

VOS_VOID AT_ClearPhonePhynumCtx(VOS_VOID)
{
    AT_PhonePhynumCmdProcCtx *phonePhynumCtx = VOS_NULL_PTR;

    phonePhynumCtx = AT_GetPhonePhynumCmdCtxAddr();
    phonePhynumCtx->typeValueLen   = 0;
    phonePhynumCtx->phynumValueLen = 0;
    phonePhynumCtx->hmacValueLen   = 0;

    if (phonePhynumCtx->typeValue != VOS_NULL_PTR) {
        PS_MEM_FREE(WUEPS_PID_AT, phonePhynumCtx->typeValue);
    }

    if (phonePhynumCtx->phynumValue != VOS_NULL_PTR) {
        PS_MEM_FREE(WUEPS_PID_AT, phonePhynumCtx->phynumValue);
    }

    if (phonePhynumCtx->hmacValue != VOS_NULL_PTR) {
        PS_MEM_FREE(WUEPS_PID_AT, phonePhynumCtx->hmacValue);
    }
}

VOS_VOID AT_ClearSimLockWriteExCtx(VOS_VOID)
{
    AT_SIMLOCKDATAWRITEEX_CMD_PROC_CTX *simLockWriteExCtx = VOS_NULL_PTR;

    simLockWriteExCtx = AT_GetSimLockWriteExCmdCtxAddr();

    simLockWriteExCtx->clientId       = 0;
    simLockWriteExCtx->layer          = 0;
    simLockWriteExCtx->curIdx         = 0;
    simLockWriteExCtx->totalNum       = 0;
    simLockWriteExCtx->hmacLen        = 0;
    simLockWriteExCtx->simlockDataLen = 0;
    simLockWriteExCtx->settingFlag    = VOS_FALSE;

    (VOS_VOID)memset_s(simLockWriteExCtx->hmac, sizeof(simLockWriteExCtx->hmac), 0x00, sizeof(simLockWriteExCtx->hmac));

    if (simLockWriteExCtx->data != VOS_NULL_PTR) {
        PS_MEM_FREE(WUEPS_PID_AT, simLockWriteExCtx->data);
        simLockWriteExCtx->data = VOS_NULL_PTR;
    }
}

VOS_VOID AT_RcvTiSimlockWriteExExpired(REL_TimerMsgBlock *tmrMsg)
{
    AT_ClearSimLockWriteExCtx();
}

VOS_VOID AT_InitModemCcCtx(ModemIdUint16 modemId)
{
    AT_ModemCcCtx *ccCtx = VOS_NULL_PTR;
    VOS_UINT32     i;

    ccCtx = AT_GetModemCcCtxAddrFromModemId(modemId);

    ccCtx->curIsExistCallFlag = VOS_FALSE;
    ccCtx->csErrCause         = TAF_CS_CAUSE_SUCCESS;

    /* 语音自动应答参数初始化 */
    (VOS_VOID)memset_s(&(ccCtx->s0TimeInfo), sizeof(ccCtx->s0TimeInfo), 0x00, sizeof(ccCtx->s0TimeInfo));

    (VOS_VOID)memset_s(&(ccCtx->econfInfo), sizeof(ccCtx->econfInfo), 0x00, sizeof(ccCtx->econfInfo));

    for (i = 0; i < TAF_CALL_MAX_ECONF_CALLED_NUM; i++) {
        ccCtx->econfInfo.callInfo[i].callState = TAF_CALL_ECONF_STATE_BUTT;
        ccCtx->econfInfo.callInfo[i].cause     = TAF_CS_CAUSE_SUCCESS;
    }
}

VOS_VOID AT_InitModemSsCtx(ModemIdUint16 modemId)
{
    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;

    ssCtx = AT_GetModemSsCtxAddrFromModemId(modemId);

    ssCtx->ussdTransMode = AT_USSD_TRAN_MODE;
    ssCtx->cModType      = MN_CALL_MODE_SINGLE;
    ssCtx->salsType      = AT_SALS_DISABLE_TYPE;
    ssCtx->clipType      = AT_CLIP_DISABLE_TYPE;
    ssCtx->clirType      = AT_CLIR_AS_SUBSCRIPT;
    ssCtx->colpType      = AT_COLP_DISABLE_TYPE;
    ssCtx->crcType       = AT_CRC_DISABLE_TYPE;
    ssCtx->ccwaType      = AT_CCWA_DISABLE_TYPE;

    ssCtx->cbstDataCfg.speed    = MN_CALL_CSD_SPD_64K_MULTI;
    ssCtx->cbstDataCfg.name     = MN_CALL_CSD_NAME_SYNC_UDI;
    ssCtx->cbstDataCfg.connElem = MN_CALL_CSD_CE_T;

    (VOS_VOID)memset_s(&(ssCtx->ccugCfg), sizeof(ssCtx->ccugCfg), 0x00, sizeof(ssCtx->ccugCfg));
}

VOS_VOID AT_InitModemSmsCtx(ModemIdUint16 modemId)
{
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromModemId(modemId);

    smsCtx->cmgfMsgFormat  = AT_CMGF_MSG_FORMAT_PDU;
    smsCtx->csdhType       = AT_CSDH_NOT_SHOW_TYPE;
    smsCtx->paraCmsr       = VOS_FALSE;
    smsCtx->smsAutoReply   = VOS_FALSE;
    smsCtx->csmsMsgVersion = MN_MSG_CSMS_MSG_VERSION_PHASE2_PLUS;

    /* ME存储状态初始化 */
    smsCtx->msgMeStorageStatus = MN_MSG_ME_STORAGE_DISABLE;
    smsCtx->localStoreFlg      = VOS_TRUE;

    smsCtx->cnmiType.cnmiModeType    = AT_CNMI_MODE_BUFFER_TYPE;
    smsCtx->cnmiType.cnmiMtType      = AT_CNMI_MT_NO_SEND_TYPE;
    smsCtx->cnmiType.cnmiBmType      = AT_CNMI_BM_NO_SEND_TYPE;
    smsCtx->cnmiType.cnmiDsType      = AT_CNMI_DS_NO_SEND_TYPE;
    smsCtx->cnmiType.cnmiBfrType     = AT_CNMI_BFR_SEND_TYPE;
    smsCtx->cnmiType.cnmiTmpModeType = AT_CNMI_MODE_BUFFER_TYPE;
    smsCtx->cnmiType.cnmiTmpMtType   = AT_CNMI_MT_NO_SEND_TYPE;
    smsCtx->cnmiType.cnmiTmpBmType   = AT_CNMI_BM_NO_SEND_TYPE;
    smsCtx->cnmiType.cnmiTmpDsType   = AT_CNMI_DS_NO_SEND_TYPE;
    smsCtx->cnmiType.cnmiTmpBfrType  = AT_CNMI_BFR_SEND_TYPE;

    /* 发送域初始化 */

    /* 短信接收定制类型初始化 */
    smsCtx->smMeFullCustomize.actFlg      = VOS_FALSE;
    smsCtx->smMeFullCustomize.mtCustomize = MN_MSG_MT_CUSTOMIZE_NONE;

    /* 文本短信相关参数初始化 */
    /*
     * 27005 3 Text Mode 3.1 Parameter Definitions
     * Message Data Parameters
     * <fo> depending on the command or result code: first octet of 3GPP TS 23.040
     * [3] SMS-DELIVER, SMS-SUBMIT (default 17), SMS-STATUS-REPORT, or SMS-COMMAND
     * (default 2) in integer format
     * <vp> depending on SMS-SUBMIT <fo> setting: 3GPP TS 23.040 [3] TP-Validity
     * Period either in integer format (default 167), in time-string format (refer
     * <dt>), or if EVPF is supported, in enhanced format (hexadecimal coded string
     * with double quotes)
     */
    (VOS_VOID)memset_s(&(smsCtx->cscaCsmpInfo), sizeof(smsCtx->cscaCsmpInfo), 0x00, sizeof(smsCtx->cscaCsmpInfo));
    smsCtx->cscaCsmpInfo.parmInUsim.parmInd = 0xff;
    smsCtx->cscaCsmpInfo.vp.validPeriod     = MN_MSG_VALID_PERIOD_RELATIVE;
    smsCtx->cscaCsmpInfo.vp.u.otherTime     = AT_CSMP_SUBMIT_VP_DEFAULT_VALUE;

    smsCtx->cscaCsmpInfo.defaultSmspIndex = AT_CSCA_CSMP_STORAGE_INDEX;

    /* 短信及状态报告读，删除，写，发送或接收存储介质初始化 */
    smsCtx->cpmsInfo.rcvPath.staRptMemStore = MN_MSG_MEM_STORE_SIM;
    smsCtx->cpmsInfo.rcvPath.smMemStore     = MN_MSG_MEM_STORE_SIM;
    smsCtx->cpmsInfo.memReadorDelete        = MN_MSG_MEM_STORE_SIM;
    smsCtx->cpmsInfo.memSendorWrite         = MN_MSG_MEM_STORE_SIM;

    /* 短信及状态报告接收上报方式初始化 */
    smsCtx->cpmsInfo.rcvPath.rcvSmAct       = MN_MSG_RCVMSG_ACT_STORE;
    smsCtx->cpmsInfo.rcvPath.rcvStaRptAct   = MN_MSG_RCVMSG_ACT_STORE;
    smsCtx->cpmsInfo.rcvPath.smsServVersion = MN_MSG_CSMS_MSG_VERSION_PHASE2_PLUS;

    /* 短信或状态报告不存储直接上报PDU的缓存初始化 */
    (VOS_VOID)memset_s(&(smsCtx->smtBuffer), sizeof(smsCtx->smtBuffer), 0x00, sizeof(smsCtx->smtBuffer));

    /* 短信自动应答缓存数据指针初始化 */
    (VOS_VOID)memset_s(smsCtx->smsMtBuffer, sizeof(smsCtx->smsMtBuffer), 0x00, (sizeof(smsCtx->smsMtBuffer)));

    /* 广播短信的语言选择和不存储直接上报PDU的缓存初始化 */
#if ((FEATURE_GCBS == FEATURE_ON) || (FEATURE_WCBS == FEATURE_ON))
    (VOS_VOID)memset_s(&(smsCtx->cbsDcssInfo), sizeof(smsCtx->cbsDcssInfo), 0x00, sizeof(smsCtx->cbsDcssInfo));
#endif
}

VOS_VOID AT_InitModemNetCtx(ModemIdUint16 modemId)
{
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;

    netCtx = AT_GetModemNetCtxAddrFromModemId(modemId);

    netCtx->cerssiReportType = AT_CERSSI_REPORT_TYPE_5DB_CHANGE_REPORT;
    netCtx->cregType         = AT_CREG_RESULT_CODE_NOT_REPORT_TYPE;
    netCtx->cgregType        = AT_CGREG_RESULT_CODE_NOT_REPORT_TYPE;

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    netCtx->c5gregType = AT_C5GREG_RESULT_CODE_NOT_REPORT_TYPE;
#endif

    netCtx->prefPlmnType           = MN_PH_PREF_PLMN_UPLMN;
    netCtx->cpolFormatType         = AT_COPS_NUMERIC_TYPE;
    netCtx->roamFeature            = AT_ROAM_FEATURE_OFF;
    netCtx->roamCapa               = TAF_MMA_ROAM_NATIONAL_ON_INTERNATIONAL_ON;
#if (FEATURE_MBB_CUST == FEATURE_ON)
    netCtx->roamRule               = NVIM_ROAM_RULE_CMP_PLMN; /* 初始化默认使用规则0 */
#endif
    netCtx->spnType                = 0;
    netCtx->cerssiMinTimerInterval = 0;
    netCtx->calculateAntennaLevel  = AT_CMD_ANTENNA_LEVEL_0;
    (VOS_VOID)memset_s(netCtx->antennaLevel, sizeof(netCtx->antennaLevel), 0x00, sizeof(netCtx->antennaLevel));
    (VOS_VOID)memset_s(&(netCtx->timeInfo), sizeof(netCtx->timeInfo), 0x00, sizeof(netCtx->timeInfo));
    (VOS_VOID)memset_s(&(netCtx->cgerepCfg), sizeof(netCtx->cgerepCfg), 0x00, sizeof(netCtx->cgerepCfg));

#if (FEATURE_LTE == FEATURE_ON)
    netCtx->ceregType = AT_CEREG_RESULT_CODE_NOT_REPORT_TYPE;
#endif

    netCtx->csdfCfg.mode    = 1; /* 1 DD-MMM-YYYY */
    netCtx->csdfCfg.auxMode = 1;
#if (FEATURE_MBB_CUST == FEATURE_ON)
    (VOS_VOID)memset_s(&netCtx->cclkTimeInfo, sizeof(netCtx->cclkTimeInfo), 0x00, sizeof(netCtx->cclkTimeInfo));
    netCtx->cclkSecond = 0;
    netCtx->cclkTimeInfo.ieFlg = NAS_MM_INFO_IE_UTLTZ | NAS_MM_INFO_IE_LTZ;
    netCtx->cclkTimeInfo.localTimeZone = AT_INVALID_TZ_VALUE;
    netCtx->cclkTimeInfo.universalTimeandLocalTimeZone.year = 0; /* 0 默认年份偏移 */
    netCtx->cclkTimeInfo.universalTimeandLocalTimeZone.month = 1; /* 1 默认月份 */
    netCtx->cclkTimeInfo.universalTimeandLocalTimeZone.day = 6; /* 6 默认日期 */
    netCtx->cclkTimeInfo.universalTimeandLocalTimeZone.hour = 8; /* 8 默认时间 */
    netCtx->cclkTimeInfo.universalTimeandLocalTimeZone.timeZone = AT_INVALID_TZ_VALUE;
#endif
}

VOS_VOID AT_InitModemAgpsCtx(ModemIdUint16 modemId)
{
    AT_ModemAgpsCtx *agpsCtx = VOS_NULL_PTR;

    agpsCtx = AT_GetModemAgpsCtxAddrFromModemId(modemId);

    (VOS_VOID)memset_s(&(agpsCtx->xml), sizeof(agpsCtx->xml), 0x00, sizeof(agpsCtx->xml));

    agpsCtx->cposrReport  = AT_CPOSR_DISABLE;
    agpsCtx->xcposrReport = AT_XCPOSR_DISABLE;
    agpsCtx->cmolreType   = AT_CMOLRE_NUMERIC;
}

VOS_VOID AT_InitModemPsCtx(ModemIdUint16 modemId)
{
    AT_ModemPsCtx *psCtx = VOS_NULL_PTR;

    psCtx = AT_GetModemPsCtxAddrFromModemId(modemId);

    /* 初始化错误码 */
    psCtx->psErrCause = TAF_PS_CAUSE_SUCCESS;

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
    /* 初始化IP地址与enIfaceId的映射表 */
    (VOS_VOID)memset_s(psCtx->ipAddrIfaceIdMap, sizeof(psCtx->ipAddrIfaceIdMap), 0x00, sizeof(psCtx->ipAddrIfaceIdMap));
#else
    /* 初始化IP地址与RABID的映射表 */
    (VOS_VOID)memset_s(psCtx->ipAddrRabIdMap, sizeof(psCtx->ipAddrRabIdMap), 0x00, sizeof(psCtx->ipAddrRabIdMap));
#endif

#if (FEATURE_IMS == FEATURE_ON)
    (VOS_VOID)memset_s(&(psCtx->imsEmcRdp), sizeof(AT_IMS_EmcRdp), 0x00, sizeof(AT_IMS_EmcRdp));
#endif
}

VOS_VOID AT_InitModemPrivacyFilterCtx(ModemIdUint16 modemId)
{
    AT_ModemPrivacyFilterCtx *filterCtx = VOS_NULL_PTR;

    filterCtx = AT_GetModemPrivacyFilterCtxAddrFromModemId(modemId);

    (VOS_VOID)memset_s(filterCtx, (VOS_UINT32)sizeof(AT_ModemPrivacyFilterCtx), 0,
                       (VOS_UINT32)sizeof(AT_ModemPrivacyFilterCtx));

    filterCtx->filterEnableFlg = VOS_FALSE;
}
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_VOID AT_InitModemCdmaModemSwitchCtx(ModemIdUint16 modemId)
{
    AT_ModemCdmamodemswitchCtx *cdmaModemSwitchCtx = VOS_NULL_PTR;

    cdmaModemSwitchCtx = AT_GetModemCdmaModemSwitchCtxAddrFromModemId(modemId);

    (VOS_VOID)memset_s(cdmaModemSwitchCtx, (VOS_UINT32)sizeof(AT_ModemCdmamodemswitchCtx), 0x00,
                       (VOS_UINT32)sizeof(AT_ModemCdmamodemswitchCtx));

    cdmaModemSwitchCtx->enableFlg = VOS_FALSE;
}
#endif

VOS_VOID AT_InitModemImsCtx(ModemIdUint16 modemId)
{
    AT_ModemImsContext *imsCtx = VOS_NULL_PTR;

    imsCtx = AT_GetModemImsCtxAddrFromModemId(modemId);

    (VOS_VOID)memset_s(imsCtx, sizeof(AT_ModemImsContext), 0x00, sizeof(AT_ModemImsContext));

    imsCtx->batteryInfo.currBatteryInfo = AT_IMSA_BATTERY_STATUS_BUTT;
    imsCtx->batteryInfo.tempBatteryInfo = AT_IMSA_BATTERY_STATUS_BUTT;
}

VOS_VOID AT_InitNvloadCarrierCtx(ModemIdUint16 modemId)
{
    AT_ModemNvloadCtx *nvloadCtx = VOS_NULL_PTR;

    nvloadCtx = AT_GetModemNvloadCtxAddrFromModemId(modemId);

    (VOS_VOID)memset_s(nvloadCtx->carrierName, sizeof(nvloadCtx->carrierName), 0x00, sizeof(nvloadCtx->carrierName));
    nvloadCtx->specialEffectiveFlg = 0;
}

VOS_VOID AT_InitClientConfiguration(VOS_VOID)
{
    VOS_UINT8     i;
    AT_ClientCtx *clientCtx = VOS_NULL_PTR;

    for (i = 0; i < AT_CLIENT_ID_BUTT; i++) {
        clientCtx = AT_GetClientCtxAddr(i);

        clientCtx->clientConfiguration.reportFlg = VOS_TRUE;
        clientCtx->clientConfiguration.modemId   = MODEM_ID_0;
    }
}

VOS_VOID AT_InitResetCtx(VOS_VOID)
{
    AT_ResetCtx *resetCtx = VOS_NULL_PTR;

    (VOS_VOID)memset_s(&g_atStatsInfo, (VOS_UINT32)sizeof(g_atStatsInfo), 0x00, (VOS_UINT32)sizeof(g_atStatsInfo));

    resetCtx = AT_GetResetCtxAddr();

    resetCtx->hResetSem    = VOS_NULL_PTR;
    resetCtx->resetingFlag = VOS_FALSE;

    /* 分配二进制信号量 */
    if (VOS_SmBCreate("AT", 0, VOS_SEMA4_FIFO, &resetCtx->hResetSem) != VOS_OK) {
        PS_PRINTF_WARNING("Create AT acpu cnf sem failed!\n");
        AT_DBG_SET_SEM_INIT_FLAG(VOS_FALSE);
        AT_DBG_CREATE_BINARY_SEM_FAIL_NUM(1);

        return;
    } else {
        AT_DBG_SAVE_BINARY_SEM_ID(resetCtx->hResetSem);
    }

    AT_DBG_SET_SEM_INIT_FLAG(VOS_TRUE);
}

VOS_VOID AT_InitReleaseInfo(VOS_VOID)
{
    (VOS_VOID)memset_s(&g_releaseInfo, (VOS_UINT32)sizeof(g_releaseInfo), 0x00, (VOS_UINT32)sizeof(g_releaseInfo));

    g_releaseInfo.accessStratumRel = AT_ACCESS_STRATUM_REL9;
}

#if (FEATURE_AT_HSUART == FEATURE_ON)

VOS_VOID AT_InitUartCtx(VOS_VOID)
{
    AT_UartCtx *uartCtx = VOS_NULL_PTR;
    VOS_UINT32   callId;

    uartCtx = AT_GetUartCtxAddr();

    (VOS_VOID)memset_s(uartCtx, (VOS_UINT32)sizeof(AT_UartCtx), 0x00, (VOS_UINT32)sizeof(AT_UartCtx));

    /* 初始化UART波特率，帧格式默认值 */
    uartCtx->phyConfig.baudRate     = AT_UART_DEFAULT_BAUDRATE;
#if (FEATURE_UART_BAUDRATE_AUTO_ADAPTION == FEATURE_ON)
    uartCtx->phyConfig.baudRate     = AT_UART_BAUDRATE_0;
#endif
    uartCtx->phyConfig.frame.format = AT_UART_DEFAULT_FORMAT;
    uartCtx->phyConfig.frame.parity = AT_UART_DEFAULT_PARITY;

    /* 初始化FLOW CTRL默认值 */
    uartCtx->flowCtrl.dceByDte = AT_UART_DEFAULT_FC_DCE_BY_DTE;
    uartCtx->flowCtrl.dteByDce = AT_UART_DEFAULT_FC_DTE_BY_DCE;

    /* 初始化RI信号波形默认值 */
    uartCtx->riConfig.smsRiOnInterval    = AT_UART_DEFAULT_SMS_RI_ON_INTERVAL;
    uartCtx->riConfig.smsRiOffInterval   = AT_UART_DEFAULT_SMS_RI_OFF_INTERVAL;
    uartCtx->riConfig.voiceRiOnInterval  = AT_UART_DEFAULT_VOICE_RI_ON_INTERVAL;
    uartCtx->riConfig.voiceRiOffInterval = AT_UART_DEFAULT_VOICE_RI_OFF_INTERVAL;
    uartCtx->riConfig.voiceRiCycleTimes  = AT_UART_DEFAULT_VOICE_RI_CYCLE_TIMES;

    /* 初始化RI信号状态 */
    uartCtx->riStateInfo.runFlg = VOS_FALSE;
    uartCtx->riStateInfo.type   = AT_UART_RI_TYPE_BUTT;

    uartCtx->riStateInfo.hVoiceRiTmrHdl    = VOS_NULL_PTR;
    uartCtx->riStateInfo.voiceRiTmrStatus  = AT_TIMER_STATUS_STOP;
    uartCtx->riStateInfo.voiceRiCycleCount = 0;

    for (callId = 0; callId <= MN_CALL_MAX_NUM; callId++) {
        uartCtx->riStateInfo.voiceRiStatus[callId] = AT_UART_RI_STATUS_STOP;
    }

    uartCtx->riStateInfo.hSmsRiTmrHdl     = VOS_NULL_PTR;
    uartCtx->riStateInfo.smsRiOutputCount = 0;
    uartCtx->riStateInfo.smsRiTmrStatus   = AT_TIMER_STATUS_STOP;

    uartCtx->wmLowFunc    = VOS_NULL_PTR;
}
#endif

VOS_VOID AT_InitCommCtx(VOS_VOID)
{
    AT_CommCtx *commCtx = VOS_NULL_PTR;

    commCtx = AT_GetCommCtxAddr();

    /* 默认应该类型为MP */
    commCtx->systemAppConfigAddr = SYSTEM_APP_MP;

    (VOS_VOID)memset_s(&commCtx->customUsimmCfg, sizeof(NAS_NVIM_CustomUsimmCfg), 0, sizeof(NAS_NVIM_CustomUsimmCfg));

    /* 初始化PS域公共的上下文 */
    AT_InitCommPsCtx();

    AT_InitCommPbCtx();

    AT_InitCmdProcCtx();

    AT_InitReleaseInfo();
#if (FEATURE_ECALL == FEATURE_ON)
    AT_InitCommEcallCtx();
#endif /* FEATURE_ECALL == FEATURE_ON */

    AT_InitMtTrustListCtrl();
}

VOS_VOID AT_InitClientCtx(VOS_VOID)
{
    AT_InitClientConfiguration();
}

VOS_VOID AT_InitModemCtx(ModemIdUint16 modemId)
{
    AT_InitUsimStatus(modemId);

    AT_InitPlatformRatList(modemId);

    AT_InitModemCcCtx(modemId);

    AT_InitModemSsCtx(modemId);

    AT_InitModemSmsCtx(modemId);

    AT_InitModemNetCtx(modemId);

    AT_InitModemAgpsCtx(modemId);

    AT_InitModemPsCtx(modemId);

    AT_InitModemImsCtx(modemId);

    AT_InitModemPrivacyFilterCtx(modemId);
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    AT_InitModemCdmaModemSwitchCtx(modemId);
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    AT_InitModemUePolciyCtx(modemId);
#endif

    AT_InitNvloadCarrierCtx(modemId);
}

VOS_VOID AT_InitCtx(VOS_VOID)
{
    ModemIdUint16 modemId;

    AT_InitCommCtx();

    for (modemId = 0; modemId < MODEM_ID_BUTT; modemId++) {
        AT_InitModemCtx(modemId);
    }

    AT_InitClientCtx();
}

VOS_VOID AT_CleanDataChannelCfg(AT_PS_DataChanlCfg *chanCfg)
{
    chanCfg->used        = VOS_FALSE;
    chanCfg->rmNetId     = AT_PS_INVALID_RMNET_ID;
    chanCfg->ifaceId     = AT_PS_INVALID_IFACE_ID;
    chanCfg->ifaceActFlg = VOS_FALSE;
    chanCfg->portIndex   = AT_CLIENT_ID_BUTT;
    chanCfg->cid         = TAF_INVALID_CID;
    chanCfg->modemId     = MODEM_ID_BUTT;
}

ModemIdUint16 AT_GetModemIDFromPid(VOS_UINT32 pid)
{
#if (MULTI_MODEM_NUMBER >= 2)
    VOS_UINT32 modemPidTabLen;
    VOS_UINT32 i;

    modemPidTabLen = (sizeof(g_atModemPidTab) / sizeof(AT_ModemPidTab));

    for (i = 0; i < modemPidTabLen; i++) {
        if (pid == g_atModemPidTab[i].modem0Pid) {
            return MODEM_ID_0;
        }

        if (pid == g_atModemPidTab[i].modem1Pid) {
            return MODEM_ID_1;
        }

#if (MULTI_MODEM_NUMBER == 3)
        if (pid == g_atModemPidTab[i].modem2Pid) {
            return MODEM_ID_2;
        }
#endif
    }

    return MODEM_ID_BUTT;
#else
    return MODEM_ID_0;
#endif
}

VOS_UINT8* AT_GetSystemAppConfigAddr(VOS_VOID)
{
    AT_CommCtx *commCtx = VOS_NULL_PTR;

    commCtx = AT_GetCommCtxAddr();

    return &(commCtx->systemAppConfigAddr);
}

AT_ResetCtx* AT_GetResetCtxAddr(VOS_VOID)
{
    return &(g_atResetCtx);
}

AT_CommCtx* AT_GetCommCtxAddr(VOS_VOID)
{
    return &(g_atCommCtx);
}

AT_CommPsCtx* AT_GetCommPsCtxAddr(VOS_VOID)
{
    return &(g_atCommCtx.psCtx);
}

/* 获取维护特性控制开关上下文地址 */
AT_MtTrustListCtrl* AT_GetCommMtTrustCtrlAddr(VOS_VOID)
{
    AT_CommCtx *commCtx = VOS_NULL_PTR;

    commCtx = AT_GetCommCtxAddr();
    return &(commCtx->mtTrustListCtrl);
}

AT_CmdTrustListCtrl* AT_GetCommCmdTrustListCtrlAddr(VOS_VOID)
{
    AT_CommCtx *commCtx = VOS_NULL_PTR;

    commCtx = AT_GetCommCtxAddr();
    return &(commCtx->cmdTrustListCtrl);
}

#if (FEATURE_IMS == FEATURE_ON)

AT_IMS_EmcRdp* AT_GetImsEmcRdpByClientId(VOS_UINT16 clientId)
{
    ModemIdUint16 modemId = MODEM_ID_0;

    if (AT_GetModemIdFromClient(clientId, &modemId) != VOS_OK) {
        AT_ERR_LOG("AT_GetImsEmcRdpByClientId: ClientId is invalid.");
        return VOS_NULL_PTR;
    }

    return &(AT_GetModemPsCtxAddrFromModemId(modemId)->imsEmcRdp);
}
#endif

AT_CommPbCtx* AT_GetCommPbCtxAddr(VOS_VOID)
{
    return &(g_atCommCtx.commPbCtx);
}

AT_CmdMsgNumCtrl* AT_GetMsgNumCtrlCtxAddr(VOS_VOID)
{
    return &(g_atCommCtx.msgNumCtrlCtx);
}

AT_USIM_InfoCtx* AT_GetUsimInfoCtxFromModemId(ModemIdUint16 modemId)
{
    return &(g_atModemCtx[modemId].atUsimInfoCtx);
}

AT_ModemSptRat* AT_GetSptRatFromModemId(ModemIdUint16 modemId)
{
    return &(g_atModemCtx[modemId].platformCapList.platformRatList);
}

AT_ModemCcCtx* AT_GetModemCcCtxAddrFromModemId(ModemIdUint16 modemId)
{
    return &(g_atModemCtx[modemId].ccCtx);
}

AT_ModemCcCtx* AT_GetModemCcCtxAddrFromClientId(VOS_UINT16 clientId)
{
    ModemIdUint16 modemId = MODEM_ID_0;
    VOS_UINT32    rslt;

    rslt = AT_GetModemIdFromClient((VOS_UINT8)clientId, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_GetModemCcCtxAddrFromClientId: Get modem id fail.");
    }

    return &(g_atModemCtx[modemId].ccCtx);
}

AT_ModemSsCtx* AT_GetModemSsCtxAddrFromModemId(ModemIdUint16 modemId)
{
    return &(g_atModemCtx[modemId].ssCtx);
}

AT_ModemSsCtx* AT_GetModemSsCtxAddrFromClientId(VOS_UINT16 clientId)
{
    ModemIdUint16 modemId = MODEM_ID_0;
    VOS_UINT32    rslt;

    rslt = AT_GetModemIdFromClient((VOS_UINT8)clientId, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_GetModemSsCtxAddrFromClientId: Get modem id fail.");
    }

    return &(g_atModemCtx[modemId].ssCtx);
}

AT_ModemSmsCtx* AT_GetModemSmsCtxAddrFromModemId(ModemIdUint16 modemId)
{
    return &(g_atModemCtx[modemId].smsCtx);
}

AT_ModemSmsCtx* AT_GetModemSmsCtxAddrFromClientId(VOS_UINT16 clientId)
{
    ModemIdUint16 modemId = MODEM_ID_0;
    VOS_UINT32    rslt;

    rslt = AT_GetModemIdFromClient((VOS_UINT8)clientId, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_GetModemSmsCtxAddrFromClientId: Get modem id fail");
    }

    return &(g_atModemCtx[modemId].smsCtx);
}

AT_ModemNetCtx* AT_GetModemNetCtxAddrFromModemId(ModemIdUint16 modemId)
{
    return &(g_atModemCtx[modemId].netCtx);
}

AT_ModemNetCtx* AT_GetModemNetCtxAddrFromClientId(VOS_UINT16 clientId)
{
    ModemIdUint16 modemId = MODEM_ID_0;
    VOS_UINT32    rslt;

    rslt = AT_GetModemIdFromClient((VOS_UINT8)clientId, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_GetModemNetCtxAddrFromClientId: Get modem id fail");
    }

    return &(g_atModemCtx[modemId].netCtx);
}

AT_ModemAgpsCtx* AT_GetModemAgpsCtxAddrFromModemId(ModemIdUint16 modemId)
{
    return &(g_atModemCtx[modemId].agpsCtx);
}

AT_ModemAgpsCtx* AT_GetModemAgpsCtxAddrFromClientId(VOS_UINT16 clientId)
{
    ModemIdUint16 modemId = MODEM_ID_0;
    VOS_UINT32    rslt;

    rslt = AT_GetModemIdFromClient((VOS_UINT8)clientId, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_GetModemAgpsCtxAddrFromClientId: Get modem id fail.");
    }

    return &(g_atModemCtx[modemId].agpsCtx);
}

AT_ModemPsCtx* AT_GetModemPsCtxAddrFromModemId(ModemIdUint16 modemId)
{
    return &(g_atModemCtx[modemId].psCtx);
}

AT_ModemPsCtx* AT_GetModemPsCtxAddrFromClientId(VOS_UINT16 clientId)
{
    ModemIdUint16 modemId = MODEM_ID_0;
    VOS_UINT32    rslt;

    rslt = AT_GetModemIdFromClient((VOS_UINT8)clientId, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_GetModemPsCtxAddrFromClientId: Get modem id fail.");
    }

    return &(g_atModemCtx[modemId].psCtx);
}

AT_ModemImsContext* AT_GetModemImsCtxAddrFromModemId(ModemIdUint16 modemId)
{
    return &(g_atModemCtx[modemId].atImsCtx);
}

AT_ModemImsContext* AT_GetModemImsCtxAddrFromClientId(VOS_UINT16 clientId)
{
    ModemIdUint16 modemId = MODEM_ID_0;
    VOS_UINT32    rslt;

    rslt = AT_GetModemIdFromClient((VOS_UINT8)clientId, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_GetModemImsCtxAddrFromClientId: Get modem id fail.");
    }

    return &(g_atModemCtx[modemId].atImsCtx);
}

/*
 * 功能描述: 获取MBB 定制信息上下文
 */
AT_ModemMbbCustmizeCtx* AT_GetModemMbbCustCtxAddrFromModemId(ModemIdUint16 modemId)
{
    return &(g_atModemCtx[modemId].mbbCustCtx);
}

AT_ModemMbbCustmizeCtx* AT_GetModemMbbCustCtxAddrFromClientId(VOS_UINT16 clientId)
{
    ModemIdUint16 modemId = MODEM_ID_0;
    VOS_UINT32    rslt;

    rslt = AT_GetModemIdFromClient((VOS_UINT8)clientId, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_GetModemMbbCustCtxAddrFromClientId: Get modem id fail.");
    }

    return &(g_atModemCtx[modemId].mbbCustCtx);
}
#if (FEATURE_MBB_CUST == FEATURE_ON)
/*
 * 功能描述: 获取MBB 定制SYSCFGEX配置组合信息
 */
AT_ModemMbbSysCfgExRatGroupCtrl* AT_GetModemMbbSysCfgExRatGroupCtrl(ModemIdUint16 modemId)
{
    return &(AT_GetModemMbbCustCtxAddrFromModemId(modemId)->syscfgExRatGroupCtrl);
}

/*
 * 功能描述: 获取MBB 定制AUTO Mode rat信息
 */
AT_ModemMbbAutoModeRatInfo* AT_GetModemMbbModeRatInfo(ModemIdUint16 modemId)
{
    return &(AT_GetModemMbbCustCtxAddrFromModemId(modemId)->autoModeRatInfo);
}

/*
 * 功能描述: 获取MBB用户配置的ICCID传输模式
 */
AT_ModemIccidTransMode* AT_GetModemMbbIccidTransMode(ModemIdUint16 modemId)
{
    return &(AT_GetModemMbbCustCtxAddrFromModemId(modemId)->iccidTransMode);
}
#endif
AT_ModemNvloadCtx* AT_GetModemNvloadCtxAddrFromModemId(ModemIdUint16 modemId)
{
    return &(g_atModemCtx[modemId].atNvloadCtx);
}

VOS_UINT32 AT_GetPsIPv6IIDTestModeConfig(VOS_VOID)
{
    return AT_GetCommPsCtxAddr()->ipv6AddrTestModeCfg;
}

AT_ClientCtx* AT_GetClientCtxAddr(AT_ClientIdUint16 clientId)
{
    return &(g_atClientCtx[clientId]);
}

MODULE_EXPORTED VOS_UINT32 AT_GetModemIdFromClient(VOS_UINT16 clientId, ModemIdUint16 *modemId)
{
    AT_ClientCtx *atClientCtx = VOS_NULL_PTR;

    /* 判断是否是MODEM0广播的client index */
    if ((clientId == AT_BROADCAST_CLIENT_INDEX_MODEM_0) || (clientId == AT_BROADCAST_CLIENT_ID_MODEM_0)) {
        *modemId = MODEM_ID_0;
    }
    /* 判断是否是MODEM1广播的client index */
    else if ((clientId == AT_BROADCAST_CLIENT_INDEX_MODEM_1) || (clientId == AT_BROADCAST_CLIENT_ID_MODEM_1)) {
        *modemId = MODEM_ID_1;
    }
    /* 判断是否是MODEM2广播的client index */
    else if ((clientId == AT_BROADCAST_CLIENT_INDEX_MODEM_2) || (clientId == AT_BROADCAST_CLIENT_ID_MODEM_2)) {
        *modemId = MODEM_ID_2;
    }
    /* 非广播client index */
    else {
        /* client index 无效，直接返回 */
        if (clientId >= AT_CLIENT_ID_BUTT) {
            return VOS_ERR;
        }

        atClientCtx = AT_GetClientCtxAddr(clientId);

        *modemId = atClientCtx->clientConfiguration.modemId;
    }

    /* 在单卡的时候NV里读出来的MODEMID为MODEM1时，会发生内存越界，此处做个异常保护 */
    if (*modemId >= MODEM_ID_BUTT) {
        AT_ERR_LOG("AT_GetModemIdFromClient: modem id is invalid");

        *modemId = MODEM_ID_0;
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_GetDestPid(MN_CLIENT_ID_T clientId, VOS_UINT32 rcvPid)
{
#if (MULTI_MODEM_NUMBER > 1)
    VOS_UINT32    rslt;
    ModemIdUint16 modemId;
    VOS_UINT32    i;

    modemId = MODEM_ID_0;

    /* 获取client id对应的Modem Id */
    rslt = AT_GetModemIdFromClient(clientId, &modemId);

    /* modem 1上ulRcvPid对应的pid */
    if ((rslt == VOS_OK) && (modemId != MODEM_ID_0)) {
        for (i = 0; i < (sizeof(g_atModemPidTab) / sizeof(AT_ModemPidTab)); i++) {
            if (rcvPid != g_atModemPidTab[i].modem0Pid) {
                continue;
            }

            if (modemId == MODEM_ID_1) {
                return g_atModemPidTab[i].modem1Pid;
            }

            if (modemId == MODEM_ID_2) {
                return g_atModemPidTab[i].modem2Pid;
            }
        }

        /* 如果出现找不到对应的PID应该时出错了 */
        if (i >= (sizeof(g_atModemPidTab) / sizeof(AT_ModemPidTab))) {
            PS_PRINTF_WARNING("<AT_GetDestPid> usClientId is %d, ulRcvPid is %d no modem1 pid. \n", clientId, rcvPid);
        }
    }
#endif

    return rcvPid;
}

VOS_UINT8 AT_IsModemSupportRat(ModemIdUint16 modemId, TAF_MMA_RatTypeUint8 rat)
{
    AT_ModemSptRat *sptRatList = VOS_NULL_PTR;

    sptRatList = AT_GetSptRatFromModemId(modemId);
    if (rat == TAF_MMA_RAT_LTE) {
        return sptRatList->platformSptLte;
    }
    if (rat == TAF_MMA_RAT_WCDMA) {
        if ((sptRatList->platformSptWcdma == VOS_TRUE) || (sptRatList->platformSptUtralTdd == VOS_TRUE)) {
            return VOS_TRUE;
        } else {
            return VOS_FALSE;
        }
    }
    if (rat == TAF_MMA_RAT_GSM) {
        return sptRatList->platformSptGsm;
    }

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    if (rat == TAF_MMA_RAT_NR) {
        return sptRatList->platformSptNr;
    }
#endif

    return VOS_FALSE;
}

VOS_UINT8 AT_IsModemSupportUtralTDDRat(ModemIdUint16 modemId)
{
    AT_ModemSptRat *sptRatList = VOS_NULL_PTR;

    sptRatList = AT_GetSptRatFromModemId(modemId);

    return sptRatList->platformSptUtralTdd;
}

TAF_CS_CauseUint32 AT_GetCsCallErrCause(VOS_UINT16 clientId)
{
    AT_ModemCcCtx *ccCtx = VOS_NULL_PTR;

    ccCtx = AT_GetModemCcCtxAddrFromClientId(clientId);

    return ccCtx->csErrCause;
}

VOS_VOID AT_UpdateCallErrInfo(VOS_UINT16 clientId, TAF_CS_CauseUint32 csErrCause, TAF_CALL_ErrorInfoText *errInfoText)
{
    AT_ModemCcCtx *ccCtx = VOS_NULL_PTR;
    errno_t        memResult;

    ccCtx = AT_GetModemCcCtxAddrFromClientId(clientId);

    ccCtx->csErrCause = csErrCause;

    (VOS_VOID)memset_s(&(ccCtx->errInfoText), (VOS_SIZE_T)sizeof(ccCtx->errInfoText), 0x00,
                       (VOS_SIZE_T)sizeof(ccCtx->errInfoText));

    if (errInfoText != VOS_NULL_PTR) {
        if (errInfoText->textLen > 0) {
            ccCtx->errInfoText.textLen = TAF_MIN(errInfoText->textLen, TAF_CALL_ERROR_INFO_TEXT_STRING_SZ);

            memResult = memcpy_s(ccCtx->errInfoText.errInfoText, (VOS_SIZE_T)sizeof(ccCtx->errInfoText.errInfoText),
                                 errInfoText->errInfoText, ccCtx->errInfoText.textLen);
            TAF_MEM_CHK_RTN_VAL(memResult, (VOS_SIZE_T)sizeof(ccCtx->errInfoText.errInfoText),
                                ccCtx->errInfoText.textLen);
        }
    }
}

TAF_CALL_ErrorInfoText* AT_GetCallErrInfoText(VOS_UINT16 clientId)
{
    return &(AT_GetModemCcCtxAddrFromClientId(clientId)->errInfoText);
}

AT_ABORT_CmdPara* AT_GetAbortCmdPara(VOS_VOID)
{
    return &(g_atAbortCmdCtx.atAbortCmdPara);
}

VOS_UINT8* AT_GetAbortRspStr(VOS_VOID)
{
    return (g_atAbortCmdCtx.atAbortCmdPara.abortAtRspStr);
}

VOS_UINT32 AT_GetSsCustomizePara(AT_SS_CUSTOMIZE_TYPE_UINT8 ssCustomizeType)
{
    VOS_UINT8 mask;

    if (g_atSsCustomizePara.status != VOS_TRUE) {
        return VOS_FALSE;
    }

    mask = (VOS_UINT8)((VOS_UINT32)AT_SS_CUSTOMIZE_SERVICE_MASK << ssCustomizeType);
    if ((g_atSsCustomizePara.ssCmdCustomize & mask) != 0) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

VOS_SEM AT_GetResetSem(VOS_VOID)
{
    return g_atResetCtx.hResetSem;
}

VOS_UINT32 AT_GetResetFlag(VOS_VOID)
{
    return g_atResetCtx.resetingFlag;
}

VOS_VOID AT_SetResetFlag(VOS_UINT32 flag)
{
    g_atResetCtx.resetingFlag = flag;
}

AT_ModemPrivacyFilterCtx* AT_GetModemPrivacyFilterCtxAddrFromModemId(ModemIdUint16 modemId)
{
    return &(g_atModemCtx[modemId].filterCtx);
}
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

AT_ModemCdmamodemswitchCtx* AT_GetModemCdmaModemSwitchCtxAddrFromModemId(ModemIdUint16 modemId)
{
    return &(g_atModemCtx[modemId].cdmaModemSwitchCtx);
}
#endif

AT_ModemMtInfoCtx* AT_GetModemMtInfoCtxAddrFromModemId(ModemIdUint16 modemId)
{
    return &(g_atModemCtx[modemId].mtInfoCtx);
}

VOS_VOID AT_InitTraceMsgTab(VOS_VOID)
{
    (VOS_VOID)memset_s(g_atTraceMsgIdTab, sizeof(g_atTraceMsgIdTab), 0xFF, sizeof(g_atTraceMsgIdTab));
}

AT_InterMsgIdUint32 AT_GetResultMsgID(VOS_UINT8 indexNum)
{
    AT_InterMsgIdUint32 resultMsgID;

    if (indexNum == AT_BROADCAST_CLIENT_INDEX_MODEM_0) {
        resultMsgID = ID_AT_MNTN_RESULT_BROADCAST_MODEM_0;
    } else if (indexNum == AT_BROADCAST_CLIENT_INDEX_MODEM_1) {
        resultMsgID = ID_AT_MNTN_RESULT_BROADCAST_MODEM_1;
    } else if (indexNum == AT_BROADCAST_CLIENT_INDEX_MODEM_2) {
        resultMsgID = ID_AT_MNTN_RESULT_BROADCAST_MODEM_2;
    } else {
        resultMsgID = g_atTraceMsgIdTab[indexNum].resultMsgId;
    }

    return resultMsgID;
}

AT_InterMsgIdUint32 AT_GetCmdMsgID(VOS_UINT8 indexNum)
{
    return g_atTraceMsgIdTab[indexNum].cmdMsgId;
}

VOS_VOID AT_ConfigTraceMsg(VOS_UINT8 indexNum, AT_InterMsgIdUint32 cmdMsgId, AT_InterMsgIdUint32 resultMsgId)
{
    g_atTraceMsgIdTab[indexNum].cmdMsgId    = cmdMsgId;
    g_atTraceMsgIdTab[indexNum].resultMsgId = resultMsgId;
}

VOS_VOID At_SetAtCmdAbortTickInfo(VOS_UINT8 indexNum, VOS_UINT32 tick)
{
    g_atAbortCmdCtx.cmdAbortTick.atSetTick[indexNum] = tick;
}

AT_CMD_ABORT_TICK_INFO* At_GetAtCmdAbortTickInfo(VOS_VOID)
{
    return &(g_atAbortCmdCtx.cmdAbortTick);
}

VOS_UINT8 At_GetAtCmdAnyAbortFlg(VOS_VOID)
{
    return (g_atAbortCmdCtx.atAbortCmdPara.anyAbortFlg);
}

VOS_VOID At_SetAtCmdAnyAbortFlg(VOS_UINT8 flg)
{
    g_atAbortCmdCtx.atAbortCmdPara.anyAbortFlg = flg;
}

AT_UartCtx* AT_GetUartCtxAddr(VOS_VOID)
{
    return &(g_atCommCtx.uartCtx);
}

AT_UartPhyCfg* AT_GetUartPhyCfgInfo(VOS_VOID)
{
    return &(AT_GetUartCtxAddr()->phyConfig);
}

AT_UartFlowCtrl* AT_GetUartFlowCtrlInfo(VOS_VOID)
{
    return &(AT_GetUartCtxAddr()->flowCtrl);
}

AT_UartRiCfg* AT_GetUartRiCfgInfo(VOS_VOID)
{
    return &(AT_GetUartCtxAddr()->riConfig);
}

AT_UartRiStateInfo* AT_GetUartRiStateInfo(VOS_VOID)
{
    return &(AT_GetUartCtxAddr()->riStateInfo);
}

AT_PortBuffCfgInfo* AT_GetPortBuffCfgInfo(VOS_VOID)
{
    return &(AT_GetCommCtxAddr()->portBuffCfg);
}

AT_PortBuffCfgUint8 AT_GetPortBuffCfg(VOS_VOID)
{
    AT_CommCtx *commCtx = VOS_NULL_PTR;

    commCtx = AT_GetCommCtxAddr();

    return commCtx->portBuffCfg.smsBuffCfg;
}

VOS_VOID AT_InitPortBuffCfg(VOS_VOID)
{
    AT_PortBuffCfgInfo *usedClientIdTab = VOS_NULL_PTR;

    usedClientIdTab = AT_GetPortBuffCfgInfo();

    (VOS_VOID)memset_s(usedClientIdTab, sizeof(AT_PortBuffCfgInfo), 0xFF, sizeof(AT_PortBuffCfgInfo));

    usedClientIdTab->num = 0;
}

VOS_VOID AT_AddUsedClientId2Tab(VOS_UINT16 clientId)
{
    AT_PortBuffCfgInfo *portBuffCfg = VOS_NULL_PTR;
    VOS_UINT32          indexNum;

    if (clientId >= AT_MAX_CLIENT_NUM) {
        return;
    }

    portBuffCfg = AT_GetPortBuffCfgInfo();

    /*  排除内部通道 */
    if ((clientId >= AT_MIN_APP_CLIENT_ID) && (clientId <= AT_MAX_APP_CLIENT_ID)) {
        return;
    }

#if (VOS_WIN32 == VOS_OS_VER)
    if (clientId == AT_CLIENT_ID_SOCK) {
        return;
    }
#endif

    if (portBuffCfg->num >= AT_MAX_CLIENT_NUM) {
        portBuffCfg->num = AT_MAX_CLIENT_NUM - 1;
    }

    /* 循环查找是否已经记录过 */
    for (indexNum = 0; indexNum < portBuffCfg->num; indexNum++) {
        if (clientId == portBuffCfg->usedClientId[indexNum]) {
            return;
        }
    }

    /* 如果没有记录过，测记录到最后 */
    if (indexNum == portBuffCfg->num) {
        portBuffCfg->usedClientId[indexNum] = (VOS_UINT32)clientId;
        portBuffCfg->num++;
    }
}

VOS_VOID AT_RmUsedClientIdFromTab(VOS_UINT16 clientId)
{
    AT_PortBuffCfgInfo *portBuffCfg = VOS_NULL_PTR;
    VOS_UINT32          indexNum;

    portBuffCfg = AT_GetPortBuffCfgInfo();

    if (portBuffCfg->num > AT_MAX_CLIENT_NUM) {
        portBuffCfg->num = AT_MAX_CLIENT_NUM;
    }

    /* 循环查找是否已经记录过 */
    for (indexNum = 0; indexNum < portBuffCfg->num; indexNum++) {
        if (clientId == portBuffCfg->usedClientId[indexNum]) {
            break;
        }
    }

    /* 如果没有记录过则直接退出 */
    if (indexNum == portBuffCfg->num) {
        return;
    }

    /* 如果找到就删除对应的client */
    if (indexNum == (VOS_UINT32)portBuffCfg->num - 1) {
        portBuffCfg->num--;
        portBuffCfg->usedClientId[indexNum] = VOS_NULL_DWORD;

    } else {
        portBuffCfg->usedClientId[indexNum]             = portBuffCfg->usedClientId[portBuffCfg->num - 1];
        portBuffCfg->usedClientId[portBuffCfg->num - 1] = VOS_NULL_DWORD;
        portBuffCfg->num--;
    }
}

AT_ClientConfiguration* AT_GetClientConfig(AT_ClientIdUint16 clientId)
{
    return &(AT_GetClientCtxAddr(clientId)->clientConfiguration);
}

const AT_ClientCfgMapTab* AT_GetClientCfgMapTbl(VOS_UINT8 indexNum)
{
    return &(g_atClientCfgMapTbl[indexNum]);
}

VOS_UINT8 AT_GetPrivacyFilterEnableFlg(VOS_VOID)
{
    return VOS_TRUE;
}

VOS_UINT8 AT_IsSupportReleaseRst(
    AT_AccessStratumRelUint8 releaseType)
{
#if (FEATURE_LTE == FEATURE_ON)
    if (releaseType <= g_releaseInfo.accessStratumRel) {
        return VOS_TRUE;
    }
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /* 目前AT_IsSupportReleaseRst调用时，入参均是Release 11，对于NR宏开启的场景，都需要返回VOS_TRUE */
    return VOS_TRUE;
#else
    return VOS_FALSE;
#endif
}

VOS_VOID AT_ConvertCellIdToHexStrFormat(VOS_UINT32 cellIdentityLowBit, VOS_UINT32 cellIdentityHighBit,
                                        VOS_CHAR *pcCellIdStr)
{
    VOS_UINT32 length = 0;
    VOS_INT32  bufLen;

    if (cellIdentityHighBit == 0) {
        /* CellId高4字节无效 */
        bufLen = snprintf_s((VOS_CHAR *)pcCellIdStr, AT_CELLID_STRING_MAX_LEN, AT_CELLID_STRING_MAX_LEN - 1, "%X",
                            cellIdentityLowBit);

    } else {
        /* CellId高4字节有效 */
        bufLen = snprintf_s((VOS_CHAR *)pcCellIdStr, AT_CELLID_STRING_MAX_LEN, AT_CELLID_STRING_MAX_LEN - 1, "%X%08X",
                            cellIdentityHighBit, cellIdentityLowBit);
    }

    if (bufLen < 0) {
        AT_ERR_LOG("AT_ConvertCellIdToHexStrFormat: bufLen is error!");
        return;
    }
    length              = (VOS_UINT32)bufLen;
    pcCellIdStr[length] = 0;
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_UINT8* AT_GetModemCLModeCtxAddrFromModemId(ModemIdUint16 modemId)
{
    return &(g_atModemCtx[modemId].platformCapList.isClMode);
}
#endif

VOS_UINT8 AT_GetCgpsCLockEnableFlgByModemId(ModemIdUint16 modemId)
{
    return g_atModemCtx[modemId].agpsCtx.atCgpsClockEnableFlag;
}

VOS_VOID AT_SetCgpsCLockEnableFlgByModemId(ModemIdUint16 modemId, VOS_UINT8 enableFLg)
{
    g_atModemCtx[modemId].agpsCtx.atCgpsClockEnableFlag = enableFLg;
}

VOS_UINT8 At_GetGsmConnectRate(VOS_VOID)
{
    return g_dialConnectDisplayRate.gsmConnectRate;
}

VOS_UINT8 At_GetGprsConnectRate(VOS_VOID)
{
    return g_dialConnectDisplayRate.gprsConnectRate;
}

VOS_UINT8 At_GetEdgeConnectRate(VOS_VOID)
{
    return g_dialConnectDisplayRate.edgeConnectRate;
}

VOS_UINT8 At_GetWcdmaConnectRate(VOS_VOID)
{
    return g_dialConnectDisplayRate.wcdmaConnectRate;
}

VOS_UINT8 At_GetDpaConnectRate(VOS_VOID)
{
    return g_dialConnectDisplayRate.dpaConnectRate;
}

TAF_MMA_SimsqStateUint32 At_GetSimsqStatus(ModemIdUint16 modemId)
{
    return AT_GetUsimInfoCtxFromModemId(modemId)->simsqStatus;
}

VOS_VOID At_SetSimsqStatus(ModemIdUint16 modemId, TAF_MMA_SimsqStateUint32 simsqStatus)
{
    AT_USIM_InfoCtx *usimmInfoCtx = VOS_NULL_PTR;

    usimmInfoCtx = AT_GetUsimInfoCtxFromModemId(modemId);

    usimmInfoCtx->simsqStatus = simsqStatus;
}

VOS_UINT8 At_GetSimsqEnable(VOS_VOID)
{
    return AT_GetCommCtxAddr()->customUsimmCfg.simsqEnable;
}

VOS_VOID At_SetSimsqEnable(VOS_UINT8 simsqEnable)
{
    AT_CommCtx *commCtx = VOS_NULL_PTR;

    commCtx = AT_GetCommCtxAddr();

    commCtx->customUsimmCfg.simsqEnable = simsqEnable;
}
#if (FEATURE_ECALL == FEATURE_ON)

VOS_VOID AT_InitCommEcallCtx(VOS_VOID)
{
    AT_CommCtx *commCtx = VOS_NULL_PTR;

    commCtx = AT_GetCommCtxAddr();

    (VOS_VOID)memset_s(&(commCtx->ecallAtCtx), sizeof(AT_CmdEcallCtx), 0x00, sizeof(AT_CmdEcallCtx));
    commCtx->ecallAtCtx.ecallMode = 0; /* auto mode */
}

AT_ECALL_AlackInfo* AT_EcallAlAckInfoAddr(VOS_VOID)
{
    AT_CommCtx *commCtx = VOS_NULL_PTR;

    commCtx = AT_GetCommCtxAddr();
    return &(commCtx->ecallAtCtx.ecallAlackInfo);
}

VOS_VOID AT_SetEclModeValue(VOS_UINT8 eclmode)
{
    AT_CommCtx *commCtx = VOS_NULL_PTR;

    commCtx                       = AT_GetCommCtxAddr();
    commCtx->ecallAtCtx.ecallMode = eclmode;
}

VOS_UINT8 AT_GetEclModeValue(VOS_VOID)
{
    AT_CommCtx *commCtx = VOS_NULL_PTR;

    commCtx = AT_GetCommCtxAddr();
    return commCtx->ecallAtCtx.ecallMode;
}
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)

AT_ModemUePolicyCtx* AT_GetModemUePolicyCtxAddrFromModemId(MODEM_ID_ENUM_UINT16 modemId)
{
    return &(g_atModemCtx[modemId].uePolicyCtx);
}

AT_ModemUePolicyCtx* AT_GetModemUePolicyCtxAddrFromClientId(VOS_UINT16 clientId)
{
    MODEM_ID_ENUM_UINT16 modemId = MODEM_ID_0;
    VOS_UINT32           result;

    result = AT_GetModemIdFromClient((VOS_UINT8)clientId, &modemId);

    if (result != VOS_OK) {
        AT_ERR_LOG("AT_GetModemUePolicyCtxAddrFromClientId: Get modem id fail.");
    }

    return &(g_atModemCtx[modemId].uePolicyCtx);
}

VOS_VOID AT_InitModemUePolciyCtx(MODEM_ID_ENUM_UINT16 modemId)
{
    AT_ModemUePolicyCtx *uePolicyCtx = VOS_NULL_PTR;

    uePolicyCtx = AT_GetModemUePolicyCtxAddrFromModemId(modemId);

    (VOS_VOID)memset_s(uePolicyCtx, sizeof(AT_ModemUePolicyCtx), 0x00, sizeof(AT_ModemUePolicyCtx));
}

#endif

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)

AT_DEVICE_CmdCtrl* AT_GetDevCmdCtrl(VOS_VOID)
{
    return &g_atDevCmdCtrl;
}
#else

AT_MT_Info* AT_GetMtInfoCtx(VOS_VOID)
{
    return &g_mtInfoCtx;
}
#endif

const TAF_CHAR* AT_GetPppDialRateDisplay(VOS_UINT32 atRateIndex)
{
    return g_pppDialRateDisplay[atRateIndex];
}

const VOS_CHAR* AT_GetDialRateDisplayNv(VOS_UINT32 atRateIndex)
{
    return g_dialRateDisplayNv[atRateIndex];
}

#if (FEATURE_LTE == FEATURE_ON)

const AT_DisplayRatePair AT_GetLteRateDisplay(VOS_UINT32 atRateIndex)
{
    return g_lteRateDisplay[atRateIndex];
}
#endif

#if (FEATURE_LTE == FEATURE_ON)

VOS_VOID AT_SetWifiRF(VOS_UINT32 atWifiRF)
{
    g_wifiRf = atWifiRF;
}

VOS_UINT32 AT_GetWifiRF(VOS_VOID)
{
    return g_wifiRf;
}
#endif

VOS_VOID AT_SetStkCmdQualify(TAF_UINT8 atStkCmdQualify)
{
    g_stkCmdQualify = atStkCmdQualify;
}

TAF_UINT8 AT_GetStkCmdQualify(VOS_VOID)
{
    return g_stkCmdQualify;
}

VOS_VOID AT_SetErrType(TAF_UINT32 atErrType)
{
    g_errType = atErrType;
}

TAF_UINT32 AT_GetErrType(VOS_VOID)
{
    return g_errType;
}

VOS_VOID AT_SetWifiFreq(VOS_UINT32 atWifiFreq)
{
    g_wifiFreq = atWifiFreq;
}

VOS_UINT32 AT_GetWifiFreq(VOS_VOID)
{
    return g_wifiFreq;
}

VOS_VOID AT_SetWifiMode(VOS_UINT32 atWifiMode)
{
    g_wifiMode = atWifiMode;
}

VOS_UINT32 AT_GetWifiMode(VOS_VOID)
{
    return g_wifiMode;
}

VOS_VOID AT_SetWifiRate(VOS_UINT32 atWifiRate)
{
    g_wifiRate = atWifiRate;
}

VOS_UINT32 AT_GetWifiRate(VOS_VOID)
{
    return g_wifiRate;
}

VOS_VOID AT_SetWifiPower(VOS_INT32 atWifiPower)
{
    g_wifiPower = atWifiPower;
}

VOS_INT32 AT_GetWifiPower(VOS_VOID)
{
    return g_wifiPower;
}

VOS_UINT* AT_GetUcastWifiRxPkts(VOS_VOID)
{
    return &g_ucastWifiRxPkts;
}

#if (FEATURE_AT_CMD_TRUST_LIST == FEATURE_ON)

const AT_CmdTrustListMapTbl* AT_GetUserCmdTrustListTbl(VOS_VOID)
{
    return g_atUserCmdTrustListTbl;
}

VOS_UINT32 AT_GetUserCmdTrustListTblCmdNum(VOS_VOID)
{
    VOS_UINT32 trustListCmdNum;

    trustListCmdNum = sizeof(g_atUserCmdTrustListTbl) / sizeof(AT_CmdTrustListMapTbl);

    return trustListCmdNum;
}

static VOS_UINT32 AT_IsSpecialTrustListCmdAti(const VOS_CHAR *keyWord, VOS_UINT32 keyWordLen)
{
    if (keyWord[AT_COMMAND_BIT_0] == 'I') {
        if ((keyWord[AT_COMMAND_BIT_1] >= '0') && (keyWord[AT_COMMAND_BIT_1] <= '9')) {
            return AT_SUCCESS;
        }
    }

    return AT_ERROR;
}

static VOS_UINT32 AT_IsSpecialTrustListCmdAtd(const VOS_CHAR *keyWord, VOS_UINT32 keyWordLen)
{
    if (keyWord[AT_COMMAND_BIT_0] == 'D') {
        if ((keyWord[AT_COMMAND_BIT_1] == '>') || (keyWord[AT_COMMAND_BIT_1] == '*') ||
            ((keyWord[AT_COMMAND_BIT_1] >= '0') && (keyWord[AT_COMMAND_BIT_1] <= '9'))) {
            return AT_SUCCESS;
        }
    }

    return AT_ERROR;
}

VOS_UINT32 AT_GetCmdKeyWord(const VOS_CHAR *atCmd, VOS_UINT16 atCmdLen, VOS_CHAR *keyWord, VOS_UINT32 keyWordMaxLen)
{
    VOS_UINT32 loop;
    VOS_UINT32 keyWordLen = 0;

    for (loop = 0; loop < atCmdLen; loop++) {
        if ((atCmd[loop] == '=') || (atCmd[loop] == '?') || (atCmd[loop] == '\r') || (atCmd[loop] == ';')) {
            break;
        }
        keyWord[keyWordLen] = atCmd[loop];
        keyWordLen++;
        if (keyWordLen >= (keyWordMaxLen - 1)) {
            break;
        }
    }

    keyWord[keyWordLen] = '\0';
    /* 全部格式化为大写字符 */
    if(At_UpString(keyWord, keyWordLen) == AT_FAILURE) {
        AT_ERR_LOG("AT_GetCmdKeyWord At_UpString return fail");
        return AT_CMD_KEY_WORD_LEN_ERR;
    }

    return keyWordLen;
}

VOS_UINT32 AT_IsCmdInTrustList(VOS_CHAR *keyWord, VOS_UINT32 keyWordLen)
{
    const AT_CmdTrustListMapTbl *atCmdTrustListTbl = VOS_NULL_PTR;
    VOS_UINT32                   ret;
    VOS_UINT32                   loop;
    VOS_UINT32                   trustListAtCmdLen;
    VOS_UINT32                   trustListCmdNum;

    AT_INFO_LOG("AT_IsCmdInTrustList enter");

    /* 设置和查询命令的Trust名单检查 */
    atCmdTrustListTbl = AT_GetUserCmdTrustListTbl();
    trustListCmdNum = AT_GetUserCmdTrustListTblCmdNum();
    for (loop = 0; loop < trustListCmdNum; loop++) {
        trustListAtCmdLen = VOS_StrLen(atCmdTrustListTbl[loop].atCmd);
        if (trustListAtCmdLen != keyWordLen) {
            continue;
        }
        ret = VOS_StrNiCmp(keyWord, atCmdTrustListTbl[loop].atCmd, trustListAtCmdLen);
        if (ret == 0) {
            AT_INFO_LOG("AT_IsCmdInTrustList success");
            return AT_SUCCESS;
        }
    }

    ret = AT_IsSpecialTrustListCmdAti(keyWord, keyWordLen);
    if (ret == AT_SUCCESS) {
        AT_INFO_LOG("AT_IsSpecialTrustListCmd ATI success");
        return AT_SUCCESS;
    }

    ret = AT_IsSpecialTrustListCmdAtd(keyWord, keyWordLen);
    if (ret == AT_SUCCESS) {
        AT_INFO_LOG("AT_IsSpecialTrustListCmd ATD success");
        return AT_SUCCESS;
    }

    AT_ERR_LOG("AT_IsCmdInTrustList fail");

    return AT_ERROR;
}
#endif

#if (FEATURE_MBB_CUST == FEATURE_ON)
#if (FEATURE_PHONE_USIM == FEATURE_OFF)
VOS_UINT32 AT_GetStkReportAppConfigFlag(VOS_UINT16 clientId)
{
    ModemIdUint16 modemId = MODEM_ID_0;
    VOS_UINT32 rslt;

    rslt = AT_GetModemIdFromClient((VOS_UINT8)clientId, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_GetStkReportAppConfigFlag: Get modem id fail.");
    }

    return g_stkReportAppFlag[modemId];
}

VOS_VOID AT_SetStkReportAppConfigFlag(ModemIdUint16 modemId, VOS_UINT32 flag)
{
    if (modemId >= MODEM_ID_BUTT) {
        AT_ERR_LOG("AT_SetStkReportAppConfigFlag: modem id error.");
        return;
    }

    g_stkReportAppFlag[modemId] = flag;
}
VOS_VOID AT_SetSystemAppConfig(VOS_UINT16 clientId, VOS_UINT8 *systemAppConfig)
{
    if(AT_GetStkReportAppConfigFlag(clientId) == VOS_TRUE) {
        *systemAppConfig = SYSTEM_APP_ANDROID;
    } else {
        *systemAppConfig = SYSTEM_APP_WEBUI;
    }
}
#endif
VOS_VOID AtUpdateCclkInfo(TAF_MMA_TimeChangeInd *mmInfo)
{
    errno_t memResult;
    VOS_INT8 timeZone = 0;
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;
    NAS_MM_InfoInd *timeInfo = VOS_NULL_PTR;

    if (mmInfo == VOS_NULL_PTR) {
        return;
    }

    netCtx = AT_GetModemNetCtxAddrFromModemId(MODEM_ID_0);
    timeInfo = &(netCtx->cclkTimeInfo);
    netCtx->cclkSecond = netCtx->nwSecond;

    if ((mmInfo->ieFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ) {
        timeZone = mmInfo->universalTimeandLocalTimeZone.timeZone;
    }

    /* 保存网络下发的时区信息 */
    if (timeZone != timeInfo->localTimeZone) {
        timeInfo->localTimeZone = timeZone;
    }

    timeInfo->ieFlg = mmInfo->ieFlg;
    memResult = memcpy_s(&(timeInfo->universalTimeandLocalTimeZone),
        sizeof(timeInfo->universalTimeandLocalTimeZone),
        &mmInfo->universalTimeandLocalTimeZone,
        sizeof(mmInfo->universalTimeandLocalTimeZone));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(timeInfo->universalTimeandLocalTimeZone),
        sizeof(mmInfo->universalTimeandLocalTimeZone));
}
#endif

#if (FEATURE_AT_CMD_TRUST_LIST == FEATURE_ON)

VOS_UINT32 AT_IsNeedMatchTrustList(VOS_VOID)
{
    AT_CmdTrustListCtrl *cmdTrustListCtrl = VOS_NULL_PTR;

    /* 获取AT命令可信名单功能配置 */
    cmdTrustListCtrl = AT_GetCommCmdTrustListCtrlAddr();

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    AT_NORM_LOG1("AT_IsNeedMatchTrustList:", cmdTrustListCtrl->atCmdTrustListEnableFlg);
    if (cmdTrustListCtrl->atCmdTrustListEnableFlg == VOS_TRUE) {
        return VOS_TRUE;
    } else {
        return VOS_FALSE;
    }
#endif

    return VOS_TRUE;
}
#endif

