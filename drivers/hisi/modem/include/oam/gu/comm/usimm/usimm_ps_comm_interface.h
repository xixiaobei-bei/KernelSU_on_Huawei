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
 * 功能描述: 该头文件定义了---usim模块对外的接口定义
 * 生成日期: 2008年9月23日
 */

#ifndef __USIMM_PS_COMM_INTERFACE_H__
#define __USIMM_PS_COMM_INTERFACE_H__

#include "vos.h"
#include "product_config.h"
#include "mdrv.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define PS_USIM_SERVICE_NOT_AVAILIABLE          0
#define PS_USIM_SERVICE_AVAILIABLE              1

#define USIM_VSIM_AES_KEY_LEN                   32

#define USIMM_NULL_ID                           0xFFFF

/* 定义卡密码长度 */
#define USIMM_PINNUMBER_LEN                     8

#define USIMM_T0_APDU_MAX_LEN                   256

/* USIMM模块支持的最大路径长度,UINT16为单位 */
#define USIMM_MAX_PATH_LEN                      6

#define USIMM_REFRESH_FILE_MAX_PATH_LEN         ((USIMM_MAX_PATH_LEN + 2) * 4)

#define USIMM_REFRESH_FILE_MAX_PATH_LEN_SUB_ONE (USIMM_REFRESH_FILE_MAX_PATH_LEN - 1)

/* USIMM模块支持的一次REFRESH命令更新文件个数，根据STK数据长度最大256，计算最多的文件路径为65个 */
#define USIMM_MAX_REFRESH_FILE_NUM              65

/* APDU回复数据的最大长度 */
#define USIMM_APDU_RSP_MAX_LEN                  260

#define USIMM_APDU_HEADLEN                      5

#define USIMM_EF_IMSI_LEN                       9
#define USIMM_EF_CIMSI_LEN                      10
#define USIMM_EF_IMSI_COMM_LEN                  10
#define USIMM_EF_AD_MIN_LEN                     3

#define USIMM_EF_GID_LEN                        1      /* 目前锁网锁卡校验场景只会用到第一个字节 */
#define USIMM_EF_GID1_LEN                       16
#define USIMM_EF_GID2_LEN                       16

#define MFLAB                                   0x3F
#define DFUNDERMFLAB                            0x7F
#define DFUNDERDFLAB                            0x5F
#define MFID                                    0x3F00
#define EFIDUNDERMF                             0x2F00
#define EFIDUNDERMFDFDF                         0x4F00
#define EFIDUNDERMFDF                           0x6F00
#define DFIDUNDERMF                             0x7F00
#define DFIDUNDERMFDF                           0x5F00

#define EFBDN                                   0x6F4D
#define EFFDN                                   0x6F3B
#define EFMSISDN                                0x6F40

/* AID长度最小值和最大值 */
#define USIMM_AID_LEN_MIN                       5
#define USIMM_AID_LEN_MAX                       16

#define USIMM_TPDU_HEAD_LEN                     5
#define USIMM_TPDU_DATA_LEN_MAX                 256
#define USIMM_PRIVATECGLA_DATA_LEN_MAX          (USIMM_TPDU_DATA_LEN_MAX*3)

#define USIMM_BCMCSRSP_LEN_MAX                  16

#define USIMM_ERROR_INFO_MAX                    20

#define USIMM_ICCID_FILE_LEN                    10

#define USIMM_MCC_LEN                           3

#define USIMM_EFAD_FILE_MNC_INDEX               3

#define USIMM_FILE_OPEN_MODE_R                  "rb"            /* open binary file for reading */
#define USIMM_FILE_OPEN_MODE_W                  "wb"            /* open binary file for writing */
#define USIMM_FILE_OPEN_MODE_RW                 "rb+"           /* open binary file for reading and writing */
#define USIMM_FILE_OPEN_MODE_NEW_RW             "wb+"           /* creat binary file for reading and writing */
#define USIMM_FILE_OPEN_MODE_APEND              "a"
#define USIMM_FILE_OPEN_MODE_ADD_W              "ab"            /* creat binary file for add writing */

#define USIMM_FILE_SEEK_SET                     0             /* the beginning of the file */
#define USIMM_FILE_SEEK_CUR                     1             /* the current value of the file position indicator */
#define USIMM_FILE_SEEK_END                     2             /* the end of the file */

#define USIMM_RUNCAVE_RAND_LEN                  4
#define USIMM_RUNCAVE_DIGLEN                    3
#define USIMM_ESN_MEID_LEN                      7
#define USIMM_CDMA_RANDSSD_LEN                  7
#define USIMM_CDMA_AUTHBS_LEN                   3
#define USIMM_AUTH_RUNCAVE_LEN                  17
#define USIMM_CDMA_GEN_KEY_LEN                  8     /* Generate Key/VPM命令返回的key长度 */

/* 定义鉴权数据的长度 */
#define USIMM_AUTH_IKSPACELEN                   17    /* Len+Data */
#define USIMM_AUTH_CKSPACELEN                   17    /* Len+Data */
#define USIMM_AUTH_KCSPACELEN                   9     /* Len+Data */
#define USIMM_AUTH_AUTHSPACELEN                 15    /* Len+Data */
#define USIMM_AUTH_RESSPACELEN                  17    /* Len+Data */
#define USIMM_AUTH_ATUHRSPACELEN                3     /* Data */
#define USIMM_AUTH_MSG_PADDING_LEN              20    /* 消息结构体中预留字节，为PCLint修改 */
#define USIMM_AUTH_BSCSPACELEN                  4

#define USIMM_PATHSTR_MAX_LEN                   60

#define USIMM_PATHSTR_MAX_LEN_SUB_ONE           (USIMM_PATHSTR_MAX_LEN - 1) /* For VOS_StrNLen Modify */

#define USIMM_APDU_DATA_MAXLEN                  255

#define USIMM_SETMUTILFILE_MAX                  14

#define USIMM_ADF_STR                        "3F007FFF"

/* EF under MF */
#define USIMM_MF_STR                         "3F00"
#define USIMM_DIR_STR                        "3F002F00"
#define USIMM_ICCID_STR                      "3F002FE2"
#define USIMM_PL_STR                         "3F002F05"
#define USIMM_ARR_STR                        "3F002F06"

#define USIMM_USIM_STR                       "3F007FFF"
#define USIMM_USIM_EFLI_STR                  "3F007FFF6F05"
#define USIMM_USIM_EFARR_STR                 "3F007FFF6F06"
#define USIMM_USIM_EFIMSI_STR                "3F007FFF6F07"
#define USIMM_USIM_EFKEYS_STR                "3F007FFF6F08"
#define USIMM_USIM_EFKEYSPS_STR              "3F007FFF6F09"
#define USIMM_USIM_EFCSP_STR                 "3F007FFF6F15"
#define USIMM_USIM_EFDCK_STR                 "3F007FFF6F2C"
#define USIMM_USIM_EFHPPLMN_STR              "3F007FFF6F31"
#define USIMM_USIM_EFCNL_STR                 "3F007FFF6F32"
#define USIMM_USIM_EFACMMAX_STR              "3F007FFF6F37"
#define USIMM_USIM_EFUST_STR                 "3F007FFF6F38"
#define USIMM_USIM_EFACM_STR                 "3F007FFF6F39"
#define USIMM_USIM_EFFDN_STR                 "3F007FFF6F3B"
#define USIMM_USIM_EFSMS_STR                 "3F007FFF6F3C"
#define USIMM_USIM_EFGID1_STR                "3F007FFF6F3E"
#define USIMM_USIM_EFGID2_STR                "3F007FFF6F3F"
#define USIMM_USIM_EFMSISDN_STR              "3F007FFF6F40"
#define USIMM_USIM_EFPUCT_STR                "3F007FFF6F41"
#define USIMM_USIM_EFSMSP_STR                "3F007FFF6F42"
#define USIMM_USIM_EFSMSS_STR                "3F007FFF6F43"
#define USIMM_USIM_EFCBMI_STR                "3F007FFF6F45"
#define USIMM_USIM_EFSPN_STR                 "3F007FFF6F46"
#define USIMM_USIM_EFSMSR_STR                "3F007FFF6F47"
#define USIMM_USIM_EFCBMID_STR               "3F007FFF6F48"
#define USIMM_USIM_EFSDN_STR                 "3F007FFF6F49"
#define USIMM_USIM_EFEXT2_STR                "3F007FFF6F4B"
#define USIMM_USIM_EFEXT3_STR                "3F007FFF6F4C"
#define USIMM_USIM_EFBDN_STR                 "3F007FFF6F4D"
#define USIMM_USIM_EFEXT5_STR                "3F007FFF6F4E"
#define USIMM_USIM_EFCCP2_STR                "3F007FFF6F4F"
#define USIMM_USIM_EFCBMIR_STR               "3F007FFF6F50"
#define USIMM_USIM_EFEXT4_STR                "3F007FFF6F55"
#define USIMM_USIM_EFEST_STR                 "3F007FFF6F56"
#define USIMM_USIM_EFACL_STR                 "3F007FFF6F57"
#define USIMM_USIM_EFCMI_STR                 "3F007FFF6F58"
#define USIMM_USIM_EFSTART_HFN_STR           "3F007FFF6F5B"
#define USIMM_USIM_EFTHRESHOL_STR            "3F007FFF6F5C"
#define USIMM_USIM_EFPLMNWACT_STR            "3F007FFF6F60"
#define USIMM_USIM_EFOPLMNWACT_STR           "3F007FFF6F61"
#define USIMM_USIM_EFHPLMNwACT_STR           "3F007FFF6F62"
#define USIMM_USIM_EFPSLOCI_STR              "3F007FFF6F73"
#define USIMM_USIM_EFACC_STR                 "3F007FFF6F78"
#define USIMM_USIM_EFFPLMN_STR               "3F007FFF6F7B"
#define USIMM_USIM_EFLOCI_STR                "3F007FFF6F7E"
#define USIMM_USIM_EFICI_STR                 "3F007FFF6F80"
#define USIMM_USIM_EFOCI_STR                 "3F007FFF6F81"
#define USIMM_USIM_EFICT_STR                 "3F007FFF6F82"
#define USIMM_USIM_EFOCT_STR                 "3F007FFF6F83"
#define USIMM_USIM_EFAD_STR                  "3F007FFF6FAD"
#define USIMM_USIM_EFVGCS_STR                "3F007FFF6FB1"
#define USIMM_USIM_EFVGCSS_STR               "3F007FFF6FB2"
#define USIMM_USIM_EFVBS_STR                 "3F007FFF6FB3"
#define USIMM_USIM_EFVBSS_STR                "3F007FFF6FB4"
#define USIMM_USIM_EFEMLPP_STR               "3F007FFF6FB5"
#define USIMM_USIM_EFAAEM_STR                "3F007FFF6FB6"
#define USIMM_USIM_EFECC_STR                 "3F007FFF6FB7"
#define USIMM_USIM_EFHIDDENKEY_STR           "3F007FFF6FC3"
#define USIMM_USIM_EFNETPAR_STR              "3F007FFF6FC4"
#define USIMM_USIM_EFPNN_STR                 "3F007FFF6FC5"
#define USIMM_USIM_EFOPL_STR                 "3F007FFF6FC6"
#define USIMM_USIM_EFMBDN_STR                "3F007FFF6FC7"
#define USIMM_USIM_EFEXT6_STR                "3F007FFF6FC8"
#define USIMM_USIM_EFMBI_STR                 "3F007FFF6FC9"
#define USIMM_USIM_EFMWIS_STR                "3F007FFF6FCA"
#define USIMM_USIM_EFCFIS_STR                "3F007FFF6FCB"
#define USIMM_USIM_EFEXT7_STR                "3F007FFF6FCC"
#define USIMM_USIM_EFSPDI_STR                "3F007FFF6FCD"
#define USIMM_USIM_EFMMSN_STR                "3F007FFF6FCE"
#define USIMM_USIM_EFEXT8_STR                "3F007FFF6FCF"
#define USIMM_USIM_EFMMSICP_STR              "3F007FFF6FD0"
#define USIMM_USIM_EFMMSUP_STR               "3F007FFF6FD1"
#define USIMM_USIM_EFMMSUCP_STR              "3F007FFF6FD2"
#define USIMM_USIM_EFNIA_STR                 "3F007FFF6FD3"
#define USIMM_USIM_EFVGCSCA_STR              "3F007FFF6FD4"
#define USIMM_USIM_EFVBSCA_STR               "3F007FFF6FD5"
#define USIMM_USIM_EFGBAP_STR                "3F007FFF6FD6"
#define USIMM_USIM_EFMSK_STR                 "3F007FFF6FD7"
#define USIMM_USIM_EFMUK_STR                 "3F007FFF6FD8"
#define USIMM_USIM_EFEHPLMN_STR              "3F007FFF6FD9"
#define USIMM_USIM_EFGBANL_STR               "3F007FFF6FDA"
#define USIMM_USIM_EFEHPLMNPI_STR            "3F007FFF6FDB"
#define USIMM_USIM_EFLRPLMNSI_STR            "3F007FFF6FDC"
#define USIMM_USIM_EFNAFKCA_STR              "3F007FFF6FDD"
#define USIMM_USIM_EFSPNI_STR                "3F007FFF6FDE"
#define USIMM_USIM_EFPNNI_STR                "3F007FFF6FDF"
#define USIMM_USIM_EFNCPIP_STR               "3F007FFF6FE2"
#define USIMM_USIM_EFEPSLOCI_STR             "3F007FFF6FE3"
#define USIMM_USIM_EFEPSNSC_STR              "3F007FFF6FE4"
#define USIMM_USIM_EFUFC_STR                 "3F007FFF6FE6"
#define USIMM_USIM_EFUICCIARI_STR            "3F007FFF6FE7"
#define USIMM_USIM_EFNASCONFIG_STR           "3F007FFF6FE8"
#define USIMM_USIM_EFPWS_STR                 "3F007FFF6FEC"
#define USIMM_USIM_EFFDNURI_STR              "3F007FFF6FED"
#define USIMM_USIM_EFSDNURI_STR              "3F007FFF6FEF"
#define USIMM_USIM_EFEPDGID_STR              "3F007FFF6FF3"
#define USIMM_USIM_EFEPDGSELECTION_STR       "3F007FFF6FF4"
#define USIMM_USIM_EFEPDGIDEM_STR            "3F007FFF6FF5"
#define USIMM_USIM_EFEPDGSELECTIONEM_STR     "3F007FFF6FF6"

#define USIMM_USIM_DFPHONEBOOK_STR           "3F007FFF5F3A"
#define USIMM_USIM_EFPSC_STR                 "3F007FFF5F3A4F22"
#define USIMM_USIM_EFCC_STR                  "3F007FFF5F3A4F23"
#define USIMM_USIM_EFPUID_STR                "3F007FFF5F3A4F24"
#define USIMM_USIM_EFPBR_STR                 "3F007FFF5F3A4F30"

#define USIMM_USIM_DFGSM_ACCESS_STR          "3F007FFF5F3B"
#define USIMM_USIM_EFKC_STR                  "3F007FFF5F3B4F20"
#define USIMM_USIM_EFKCGPRS_STR              "3F007FFF5F3B4F52"
#define USIMM_USIM_EFCPBCCH_STR              "3F007FFF5F3B4F63"
#define USIMM_USIM_EFINVSCAN_STR             "3F007FFF5F3B4F64"

#define USIMM_USIM_DFMEXE_STR                "3F007FFF5F3C"
#define USIMM_USIM_EFMexE_ST_STR             "3F007FFF5F3C4F40"
#define USIMM_USIM_EFORPK_STR                "3F007FFF5F3C4F41"
#define USIMM_USIM_EFARPK_STR                "3F007FFF5F3C4F42"
#define USIMM_USIM_EFTPRK_STR                "3F007FFF5F3C4F43"

#define USIMM_USIM_DFSOLSA_STR               "3F007FFF5F70"
#define USIMM_USIM_EFSAI_STR                 "3F007FFF5F704F30"
#define USIMM_USIM_EFSLL_STR                 "3F007FFF5F704F31"

#define USIMM_USIM_DFWLAN_STR                "3F007FFF5F40"
#define USIMM_USIM_EFPSEUDO_STR              "3F007FFF5F404F41"
#define USIMM_USIM_EFUPLMNWLAN_STR           "3F007FFF5F404F42"
#define USIMM_USIM_EF0PLMNWLAN_STR           "3F007FFF5F404F43"
#define USIMM_USIM_EFUWSIDL_STR              "3F007FFF5F404F44"
#define USIMM_USIM_EFOWSIDL_STR              "3F007FFF5F404F45"
#define USIMM_USIM_EFWRI_STR                 "3F007FFF5F404F46"
#define USIMM_USIM_EFHWSIDL_STR              "3F007FFF5F404F47"
#define USIMM_USIM_EFWEHPLMNPI_STR           "3F007FFF5F404F48"
#define USIMM_USIM_EFWHPI_STR                "3F007FFF5F404F49"
#define USIMM_USIM_EFWLRPLMN_STR             "3F007FFF5F404F4A"
#define USIMM_USIM_EFHPLMNDAI_STR            "3F007FFF5F404F4B"

#define USIMM_USIM_DFHNB_STR                 "3F007FFF5F50"
#define USIMM_USIM_EFACSGL_STR               "3F007FFF5F504F81"
#define USIMM_USIM_EFCSGT_STR                "3F007FFF5F504F82"
#define USIMM_USIM_EFHNBN_STR                "3F007FFF5F504F83"
#define USIMM_USIM_EFOCSGL_STR               "3F007FFF5F504F84"
#define USIMM_USIM_EFOCSGT_STR               "3F007FFF5F504F85"
#define USIMM_USIM_EFOHNBN_STR               "3F007FFF5F504F86"

#define USIMM_USIM_DFPROSE_STR               "3F007FFF5F90"

#define USIMM_USIM_DF5GS_STR                 "3F007FFF5FC0"
#define USIMM_USIM_EF5GS3GPPLOCI_STR         "3F007FFF5FC04F01"
#define USIMM_USIM_EF5GS3GPPNSC_STR          "3F007FFF5FC04F03"
#define USIMM_USIM_EF5GAUTHKEYS_STR          "3F007FFF5FC04F05"
#define USIMM_USIM_EFUAC_AIC_STR             "3F007FFF5FC04F06"
#define USIMM_USIM_EFSUCI_CALC_INFO_STR      "3F007FFF5FC04F07"
#define USIMM_USIM_EFOPL_5GS_STR             "3F007FFF5FC04F08"

#define USIMM_USIM_EFROUTING_INDICATOR_STR   "3F007FFF5FC04F0A"
#define USIMM_USIM_EFPST_STR                 "3F007FFF5F904F10"

#define USIMM_USIM_DFACDC_STR                "3F007FFF5FA0"
#define USIMM_USIM_EFACDCLIST_STR            "3F007FFF5FA04F01"

#define USIMM_TELE_STR                       "3F007F10"
#define USIMM_TELE_EFADN_STR                 "3F007F106F3A"
#define USIMM_TELE_EFFDN_STR                 "3F007F106F3B"
#define USIMM_TELE_EFSMS_STR                 "3F007F106F3C"
#define USIMM_TELE_EFCCP_STR                 "3F007F106F3D"
#define USIMM_TELE_EFMSISDN_STR              "3F007F106F40"
#define USIMM_TELE_EFSMSP_STR                "3F007F106F42"
#define USIMM_TELE_EFSMSS_STR                "3F007F106F43"
#define USIMM_TELE_EFLND_STR                 "3F007F106F44"
#define USIMM_TELE_EFSMSR_STR                "3F007F106F47"
#define USIMM_TELE_EFSDN_STR                 "3F007F106F49"
#define USIMM_TELE_EFEXT1_STR                "3F007F106F4A"
#define USIMM_TELE_EFEXT2_STR                "3F007F106F4B"
#define USIMM_TELE_EFEXT3_STR                "3F007F106F4C"
#define USIMM_TELE_EFBDN_STR                 "3F007F106F4D"
#define USIMM_TELE_EFEXT4_STR                "3F007F106F4E"
#define USIMM_TELE_EFECCP_STR                "3F007F106F4F"
#define USIMM_TELE_EFRMA_STR                 "3F007F106F53"
#define USIMM_TELE_EFSUME_STR                "3F007F106F54"
#define USIMM_TELE_EFICE_DN_STR              "3F007F106FE0"
#define USIMM_TELE_EFICE_FF_STR              "3F007F106FE1"
#define USIMM_TELE_EFPSISMSC_STR             "3F007F106FE5"

#define USIMM_TELE_DFGRAP_STR                "3F007F105F50"
#define USIMM_TELE_DFGRAP_EFIMG_STR          "3F007F105F504F20"
#define USIMM_TELE_DFGRAP_EFICEGRAPH_STR     "3F007F105F504F21"
#define USIMM_TELE_DFGRAP_EFLAUNCHSCWS_STR   "3F007F105F504F01"

#define USIMM_TELE_DFPB_STR                  "3F007F105F3A"
#define USIMM_TELE_DFPB_EFPSC_STR            "3F007F105F3A4F22"
#define USIMM_TELE_DFPB_EFCC_STR             "3F007F105F3A4F23"
#define USIMM_TELE_DFPB_EFPUID_STR           "3F007F105F3A4F24"
#define USIMM_TELE_DFPB_EFPBR_STR            "3F007F105F3A4F30"

#define USIMM_TELE_DFMM_STR                  "3F007F105F3B"
#define USIMM_TELE_DFMM_EFMML_STR            "3F007F105F3B4F47"
#define USIMM_TELE_DFMUM_EFMMDF_STR          "3F007F105F3B4F48"

#define USIMM_TELE_DFMSS_STR                 "3F007F105F3C"
#define USIMM_TELE_EFMLPL_STR                "3F007F105F3C4F20"
#define USIMM_TELE_EFMSPL_STR                "3F007F105F3C4F21"
#define USIMM_TELE_EFMMSSMODE_STR            "3F007F105F3C4F22"

#define USIMM_TELE_DFMCPTT_STR               "3F007F105F3D"
#define USIMM_TELE_EFMST_STR                 "3F007F105F3D4F01"

#define USIMM_CSIM_STR                       "3F007FFF"
#define USIMM_CSIM_EFCOUNT_STR               "3F007FFF6F21"
#define USIMM_CSIM_EFIMSIM_STR               "3F007FFF6F22"
#define USIMM_CSIM_EFIMSIT_STR               "3F007FFF6F23"
#define USIMM_CSIM_EFTMSI_STR                "3F007FFF6F24"
#define USIMM_CSIM_EFAH_STR                  "3F007FFF6F25"
#define USIMM_CSIM_EFAOP_STR                 "3F007FFF6F26"
#define USIMM_CSIM_EFALOC_STR                "3F007FFF6F27"
#define USIMM_CSIM_EFCDMAHOME_STR            "3F007FFF6F28"
#define USIMM_CSIM_EFZNREGI_STR              "3F007FFF6F29"
#define USIMM_CSIM_EFSNREGI_STR              "3F007FFF6F2A"
#define USIMM_CSIM_EFDISTREGI_STR            "3F007FFF6F2B"
#define USIMM_CSIM_EFACCOLC_STR              "3F007FFF6F2C"
#define USIMM_CSIM_EFTERM_STR                "3F007FFF6F2D"
#define USIMM_CSIM_EFSSCI_STR                "3F007FFF6F2E"
#define USIMM_CSIM_EFACP_STR                 "3F007FFF6F2F"
#define USIMM_CSIM_EFPRL_STR                 "3F007FFF6F30"
#define USIMM_CSIM_EFRUIMID_STR              "3F007FFF6F31"
#define USIMM_CSIM_EFCSIMST_STR              "3F007FFF6F32"
#define USIMM_CSIM_EFSPC_STR                 "3F007FFF6F33"
#define USIMM_CSIM_EFOTAPASPC_STR            "3F007FFF6F34"
#define USIMM_CSIM_EFNAMLOCK_STR             "3F007FFF6F35"
#define USIMM_CSIM_EFOTA_STR                 "3F007FFF6F36"
#define USIMM_CSIM_EFSP_STR                  "3F007FFF6F37"
#define USIMM_CSIM_EFESNMEID_STR             "3F007FFF6F38"
#define USIMM_CSIM_EFLI_STR                  "3F007FFF6F3A"
#define USIMM_CSIM_EFFDN_STR                 "3F007FFF6F3B"
#define USIMM_CSIM_EFSMS_STR                 "3F007FFF6F3C"
#define USIMM_CSIM_EFSMSP_STR                "3F007FFF6F3D"
#define USIMM_CSIM_EFSMSS_STR                "3F007FFF6F3E"
#define USIMM_CSIM_EFSSFC_STR                "3F007FFF6F3F"
#define USIMM_CSIM_EFSPN_STR                 "3F007FFF6F41"
#define USIMM_CSIM_EFUSGIND_STR              "3F007FFF6F42"
#define USIMM_CSIM_EFAD_STR                  "3F007FFF6F43"
#define USIMM_CSIM_EFMDN_STR                 "3F007FFF6F44"
#define USIMM_CSIM_EFMAXPRL_STR              "3F007FFF6F45"
#define USIMM_CSIM_EFSPCS_STR                "3F007FFF6F46"
#define USIMM_CSIM_EFECC_STR                 "3F007FFF6F47"
#define USIMM_CSIM_EFME3GPDOPC_STR           "3F007FFF6F48"
#define USIMM_CSIM_EF3GPDOPM_STR             "3F007FFF6F49"
#define USIMM_CSIM_EFSIPCAP_STR              "3F007FFF6F4A"
#define USIMM_CSIM_EFMIPCAP_STR              "3F007FFF6F4B"
#define USIMM_CSIM_EFSIPUPP_STR              "3F007FFF6F4C"
#define USIMM_CSIM_EFMIPUPP_STR              "3F007FFF6F4D"
#define USIMM_CSIM_EFSIPSP_STR               "3F007FFF6F4E"
#define USIMM_CSIM_EFMIPSP_STR               "3F007FFF6F4F"
#define USIMM_CSIM_EFSIPPAPSS_STR            "3F007FFF6F50"
#define USIMM_CSIM_EFPUZL_STR                "3F007FFF6F53"
#define USIMM_CSIM_EFMAXPUZL_STR             "3F007FFF6F54"
#define USIMM_CSIM_EFMECRP_STR               "3F007FFF6F55"
#define USIMM_CSIM_EFHRPDCAP_STR             "3F007FFF6F56"
#define USIMM_CSIM_EFHRPDUPP_STR             "3F007FFF6F57"
#define USIMM_CSIM_EFCSSPR_STR               "3F007FFF6F58"
#define USIMM_CSIM_EFATC_STR                 "3F007FFF6F59"
#define USIMM_CSIM_EFEPRL_STR                "3F007FFF6F5A"
#define USIMM_CSIM_EFBCSMSCFG_STR            "3F007FFF6F5B"
#define USIMM_CSIM_EFBCSMSPREF_STR           "3F007FFF6F5C"
#define USIMM_CSIM_EFBCSMSTABLE_STR          "3F007FFF6F5D"
#define USIMM_CSIM_EFBCSMSP_STR              "3F007FFF6F5E"
#define USIMM_CSIM_EFBAKPARA_STR             "3F007FFF6F63"
#define USIMM_CSIM_EFUPBAKPARA_STR           "3F007FFF6F64"
#define USIMM_CSIM_EFMMSN_STR                "3F007FFF6F65"
#define USIMM_CSIM_EFEXT8_STR                "3F007FFF6F66"
#define USIMM_CSIM_EFMMSICP_STR              "3F007FFF6F67"
#define USIMM_CSIM_EFMMSUP_STR               "3F007FFF6F68"
#define USIMM_CSIM_EFMMSUCP_STR              "3F007FFF6F69"
#define USIMM_CSIM_EFAUTHCAP_STR             "3F007FFF6F6A"
#define USIMM_CSIM_EF3GCIK_STR               "3F007FFF6F6B"
#define USIMM_CSIM_EFDCK_STR                 "3F007FFF6F6C"
#define USIMM_CSIM_EFGID1_STR                "3F007FFF6F6D"
#define USIMM_CSIM_EFGID2_STR                "3F007FFF6F6E"
#define USIMM_CSIM_EFCDMACNL_STR             "3F007FFF6F6F"
#define USIMM_CSIM_EFHOMETAG_STR             "3F007FFF6F70"
#define USIMM_CSIM_EFGROUPTAG_STR            "3F007FFF6F71"
#define USIMM_CSIM_EFSPECTAG_STR             "3F007FFF6F72"
#define USIMM_CSIM_EFCALLPROMPT_STR          "3F007FFF6F73"
#define USIMM_CSIM_EFSFEUIMID_STR            "3F007FFF6F74"
#define USIMM_CSIM_EFEST_STR                 "3F007FFF6F75"
#define USIMM_CSIM_EFHIDDENKEY_STR           "3F007FFF6F76"
#define USIMM_CSIM_EFLCSVER_STR              "3F007FFF6F77"
#define USIMM_CSIM_EFLCSCP_STR               "3F007FFF6F78"
#define USIMM_CSIM_EFSDN_STR                 "3F007FFF6F79"
#define USIMM_CSIM_EFEXT2_STR                "3F007FFF6F7A"
#define USIMM_CSIM_EFEXT3_STR                "3F007FFF6F7B"
#define USIMM_CSIM_EFICI_STR                 "3F007FFF6F7C"
#define USIMM_CSIM_EFOCI_STR                 "3F007FFF6F7D"
#define USIMM_CSIM_EFEXT5_STR                "3F007FFF6F7E"
#define USIMM_CSIM_EFCCP2_STR                "3F007FFF6F7F"
#define USIMM_CSIM_EFAPPLAB_STR              "3F007FFF6F80"
#define USIMM_CSIM_EFMODEL_STR               "3F007FFF6F81"
#define USIMM_CSIM_EFRC_STR                  "3F007FFF6F82"
#define USIMM_CSIM_EFSMSCAP_STR              "3F007FFF6F83"
#define USIMM_CSIM_EFMIPFLAGS_STR            "3F007FFF6F84"
#define USIMM_CSIM_EF3GPDUPPEXT_STR          "3F007FFF6F85"
#define USIMM_CSIM_EFIPV6CAP_STR             "3F007FFF6F87"
#define USIMM_CSIM_EFTCPCFG_STR              "3F007FFF6F88"
#define USIMM_CSIM_EFDGC_STR                 "3F007FFF6F89"
#define USIMM_CSIM_EFWAPBWCP_STR             "3F007FFF6F8A"
#define USIMM_CSIM_EFWAPBWBM_STR             "3F007FFF6F8B"
#define USIMM_CSIM_EFMMSCFG_STR              "3F007FFF6F8C"
#define USIMM_CSIM_EFJDL_STR                 "3F007FFF6F8D"

#define USIMM_CSIM_DFPHONEBOOK_STR           "3F007FFF5F3A"
#define USIMM_CSIM_EFPSC_STR                 "3F007FFF5F3A4F22"
#define USIMM_CSIM_EFCC_STR                  "3F007FFF5F3A4F23"
#define USIMM_CSIM_EFPUID_STR                "3F007FFF5F3A4F24"
#define USIMM_CSIM_EFPBR_STR                 "3F007FFF5F3A4F30"

/* EF under MF/DFGSM */
#define USIMM_GSM_STR                        "3F007F20"
#define USIMM_GSM_EFLP_STR                   "3F007F206F05"
#define USIMM_GSM_EFIMSI_STR                 "3F007F206F07"
#define USIMM_GSM_EFKC_STR                   "3F007F206F20"
#define USIMM_GSM_EFDCK_STR                  "3F007F206F2C"
#define USIMM_GSM_EFPLMNSEL_STR              "3F007F206F30"
#define USIMM_GSM_EFHPLMN_STR                "3F007F206F31"
#define USIMM_GSM_EFCNL_STR                  "3F007F206F32"
#define USIMM_GSM_EFACMMAX_STR               "3F007F206F37"
#define USIMM_GSM_EFSST_STR                  "3F007F206F38"
#define USIMM_GSM_EFACM_STR                  "3F007F206F39"
#define USIMM_GSM_EFGID1_STR                 "3F007F206F3E"
#define USIMM_GSM_EFGID2_STR                 "3F007F206F3F"
#define USIMM_GSM_EFPUCT_STR                 "3F007F206F41"
#define USIMM_GSM_EFCBMI_STR                 "3F007F206F45"
#define USIMM_GSM_EFSPN_STR                  "3F007F206F46"
#define USIMM_GSM_EFCBMID_STR                "3F007F206F48"
#define USIMM_GSM_EFBCCH_STR                 "3F007F206F74"
#define USIMM_GSM_EFACC_STR                  "3F007F206F78"
#define USIMM_GSM_EFFPLMN_STR                "3F007F206F7B"
#define USIMM_GSM_EFLOCI_STR                 "3F007F206F7E"
#define USIMM_GSM_EFAD_STR                   "3F007F206FAD"
#define USIMM_GSM_EFPHASE_STR                "3F007F206FAE"
#define USIMM_GSM_EFVGCS_STR                 "3F007F206FB1"
#define USIMM_GSM_EFVGCSS_STR                "3F007F206FB2"
#define USIMM_GSM_EFVBS_STR                  "3F007F206FB3"
#define USIMM_GSM_EFVBSS_STR                 "3F007F206FB4"
#define USIMM_GSM_EFEMLPP_STR                "3F007F206FB5"
#define USIMM_GSM_EFAAEM_STR                 "3F007F206FB6"
#define USIMM_GSM_EFECC_STR                  "3F007F206FB7"
#define USIMM_GSM_EFCBMIR_STR                "3F007F206F50"
#define USIMM_GSM_EFNIA_STR                  "3F007F206F51"
#define USIMM_GSM_EFKCGPRS_STR               "3F007F206F52"
#define USIMM_GSM_EFLOCIGPRS_STR             "3F007F206F53"
#define USIMM_GSM_EFSUME_STR                 "3F007F206F54"
#define USIMM_GSM_EFCMI_STR                  "3F007F206F58"
#define USIMM_GSM_EFPLMNWACT_STR             "3F007F206F60"
#define USIMM_GSM_EFOPLMNWACT_STR            "3F007F206F61"
#define USIMM_GSM_EFHPLMNACT_STR             "3F007F206F62"
#define USIMM_GSM_EFCPBCCH_STR               "3F007F206F63"
#define USIMM_GSM_EFINVSCAN_STR              "3F007F206F64"
#define USIMM_GSM_EFPNN_STR                  "3F007F206FC5"
#define USIMM_GSM_EFOPL_STR                  "3F007F206FC6"
#define USIMM_GSM_EFMBDN_STR                 "3F007F206FC7"
#define USIMM_GSM_EFEXT6_STR                 "3F007F206FC8"
#define USIMM_GSM_EFMBI_STR                  "3F007F206FC9"
#define USIMM_GSM_EFMWIS_STR                 "3F007F206FCA"
#define USIMM_GSM_EFCFIS_STR                 "3F007F206FCB"
#define USIMM_GSM_EFEXT7_STR                 "3F007F206FCC"
#define USIMM_GSM_EFSPDI_STR                 "3F007F206FCD"
#define USIMM_GSM_EFMMSN_STR                 "3F007F206FCE"
#define USIMM_GSM_EFEXT8_STR                 "3F007F206FCF"
#define USIMM_GSM_EFMMSICP_STR               "3F007F206FD0"
#define USIMM_GSM_EFMMSUP_STR                "3F007F206FD1"
#define USIMM_GSM_EFMMSUCP_STR               "3F007F206FD2"

/* EF for CPHS, under DFGSM */
#define USIMM_GSM_EFVMW_STR                  "3F007F206F11"
#define USIMM_GSM_EFSVCSTBL_STR              "3F007F206F12"
#define USIMM_GSM_EFCFF_STR                  "3F007F206F13"
#define USIMM_GSM_EFONS_STR                  "3F007F206F14"
#define USIMM_GSM_EFCSP_STR                  "3F007F206F15"
#define USIMM_GSM_EFCPHSI_STR                "3F007F206F16"
#define USIMM_GSM_EFMN_STR                   "3F007F206F17"
#define USIMM_GSM_EFONSS_STR                 "3F007F206F18"
#define USIMM_GSM_EFINFONUM_STR              "3F007F206F19"

/* EF under MF/DFGSM/DFSOLSA */
#define USIMM_GSM_DFSOLSA_STR                "3F007F205F70"
#define USIMM_GSM_DFSOLSA_EFSAI_STR          "3F007F205F704F30"
#define USIMM_GSM_DFSOLSA_EFSLL_STR          "3F007F205F704F31"

/* EF under MF/DFGSM/DFMExE */
#define USIMM_GSM_DFMEXE_STR                 "3F007F205F3C"
#define USIMM_GSM_DFMEXE_EFST_STR            "3F007F205F3C4F40"
#define USIMM_GSM_DFMEXE_EFORPK_STR          "3F007F205F3C4F41"
#define USIMM_GSM_DFMEXE_EFARPK_STR          "3F007F205F3C4F42"
#define USIMM_GSM_DFMEXE_EFTPRPK_STR         "3F007F205F3C4F43"

/* EF under DFCDMA */
#define USIMM_CDMA_STR                       "3F007F25"
#define USIMM_CDMA_EFCOUNT_STR               "3F007F256F21"
#define USIMM_CDMA_EFIMSIM_STR               "3F007F256F22"
#define USIMM_CDMA_EFIMSIT_STR               "3F007F256F23"
#define USIMM_CDMA_EFTMSI_STR                "3F007F256F24"
#define USIMM_CDMA_EFAH_STR                  "3F007F256F25"
#define USIMM_CDMA_EFAOP_STR                 "3F007F256F26"
#define USIMM_CDMA_EFALOC_STR                "3F007F256F27"
#define USIMM_CDMA_EFCDMAHOME_STR            "3F007F256F28"
#define USIMM_CDMA_EFZNREGI_STR              "3F007F256F29"
#define USIMM_CDMA_EFSNREGI_STR              "3F007F256F2A"
#define USIMM_CDMA_EFDISTREGI_STR            "3F007F256F2B"
#define USIMM_CDMA_EFACCOLC_STR              "3F007F256F2C"
#define USIMM_CDMA_EFTERM_STR                "3F007F256F2D"
#define USIMM_CDMA_EFSSCI_STR                "3F007F256F2E"
#define USIMM_CDMA_EFACP_STR                 "3F007F256F2F"
#define USIMM_CDMA_EFPRL_STR                 "3F007F256F30"
#define USIMM_CDMA_EFRUIMID_STR              "3F007F256F31"
#define USIMM_CDMA_EFCST_STR                 "3F007F256F32"
#define USIMM_CDMA_EFSPC_STR                 "3F007F256F33"
#define USIMM_CDMA_EFOTAPASPC_STR            "3F007F256F34"
#define USIMM_CDMA_EFNAMLOCK_STR             "3F007F256F35"
#define USIMM_CDMA_EFOTA_STR                 "3F007F256F36"
#define USIMM_CDMA_EFSP_STR                  "3F007F256F37"
#define USIMM_CDMA_EFESNMEIDME_STR           "3F007F256F38"
#define USIMM_CDMA_EFREVISION_STR            "3F007F256F39"
#define USIMM_CDMA_EFRUIMPL_STR              "3F007F256F3A"
#define USIMM_CDMA_EFSMS_STR                 "3F007F256F3C"
#define USIMM_CDMA_EFSMSP_STR                "3F007F256F3D"
#define USIMM_CDMA_EFSMSS_STR                "3F007F256F3E"
#define USIMM_CDMA_EFSSFC_STR                "3F007F256F3F"
#define USIMM_CDMA_EFSPN_STR                 "3F007F256F41"
#define USIMM_CDMA_EFUSGIND_STR              "3F007F256F42"
#define USIMM_CDMA_EFAD_STR                  "3F007F256F43"
#define USIMM_CDMA_EFMDN_STR                 "3F007F256F44"
#define USIMM_CDMA_EFMAXPRL_STR              "3F007F256F45"
#define USIMM_CDMA_EFSPCS_STR                "3F007F256F46"
#define USIMM_CDMA_EFECC_STR                 "3F007F256F47"
#define USIMM_CDMA_EFME3GPDOPC_STR           "3F007F256F48"
#define USIMM_CDMA_EF3GPDOPM_STR             "3F007F256F49"
#define USIMM_CDMA_EFSIPCAP_STR              "3F007F256F4A"
#define USIMM_CDMA_EFMIPCAP_STR              "3F007F256F4B"
#define USIMM_CDMA_EFSIPUPP_STR              "3F007F256F4C"
#define USIMM_CDMA_EFMIPUPP_STR              "3F007F256F4D"
#define USIMM_CDMA_EFSIPSP_STR               "3F007F256F4E"
#define USIMM_CDMA_EFMIPSP_STR               "3F007F256F4F"
#define USIMM_CDMA_EFSIPPAPSS_STR            "3F007F256F50"
#define USIMM_CDMA_EFPUZL_STR                "3F007F256F53"
#define USIMM_CDMA_EFMAXPUZL_STR             "3F007F256F54"
#define USIMM_CDMA_EFMECRP_STR               "3F007F256F55"
#define USIMM_CDMA_EFHRPDCAP_STR             "3F007F256F56"
#define USIMM_CDMA_EFHRPDUPP_STR             "3F007F256F57"
#define USIMM_CDMA_EFCSSPR_STR               "3F007F256F58"
#define USIMM_CDMA_EFACT_STR                 "3F007F256F59"
#define USIMM_CDMA_EFEPRL_STR                "3F007F256F5A"
#define USIMM_CDMA_EFBCSMSOFG_STR            "3F007F256F5B"
#define USIMM_CDMA_EFBCSMSPREF_STR           "3F007F256F5C"
#define USIMM_CDMA_EFBCSMSTBL_STR            "3F007F256F5D"
#define USIMM_CDMA_EFBCSMSP_STR              "3F007F256F5E"
#define USIMM_CDMA_EFIMPI_STR                "3F007F256F5F"
#define USIMM_CDMA_EFDOMAIN_STR              "3F007F256F60"
#define USIMM_CDMA_EFIMPU_STR                "3F007F256F61"
#define USIMM_CDMA_EFPCSCF_STR               "3F007F256F62"
#define USIMM_CDMA_EFBAKPARA_STR             "3F007F256F63"
#define USIMM_CDMA_EFUPBAKPARA_STR           "3F007F256F64"
#define USIMM_CDMA_EFMMSN_STR                "3F007F256F65"
#define USIMM_CDMA_EFEXT8_STR                "3F007F256F66"
#define USIMM_CDMA_EFMMSICP_STR              "3F007F256F67"
#define USIMM_CDMA_EFMMSUP_STR               "3F007F256F68"
#define USIMM_CDMA_EFMMSUCP_STR              "3F007F256F69"
#define USIMM_CDMA_EFAUTHCAP_STR             "3F007F256F6A"
#define USIMM_CDMA_EF3GCIK_STR               "3F007F256F6B"
#define USIMM_CDMA_EFDCK_STR                 "3F007F256F6C"
#define USIMM_CDMA_EFGID1_STR                "3F007F256F6D"
#define USIMM_CDMA_EFGID2_STR                "3F007F256F6E"
#define USIMM_CDMA_EFCDMACNL_STR             "3F007F256F6F"
#define USIMM_CDMA_EFHOMETAG_STR             "3F007F256F70"
#define USIMM_CDMA_EFGROUPTAG_STR            "3F007F256F71"
#define USIMM_CDMA_EFSPECTAG_STR             "3F007F256F72"
#define USIMM_CDMA_EFCALLPROM_STR            "3F007F256F73"
#define USIMM_CDMA_EFSFEUIMID_STR            "3F007F256F74"
#define USIMM_CDMA_EFSMSCAP_STR              "3F007F256F76"
#define USIMM_CDMA_EFIPV6CAP_STR             "3F007F256F77"
#define USIMM_CDMA_EFMIPFLAG_STR             "3F007F256F78"
#define USIMM_CDMA_EFTCPCFG_STR              "3F007F256F79"
#define USIMM_CDMA_EFDGC_STR                 "3F007F256F7A"
#define USIMM_CDMA_EFWAPBRCP_STR             "3F007F256F7B"
#define USIMM_CDMA_EFWAPBRBM_STR             "3F007F256F7C"
#define USIMM_CDMA_EF3GPDUPPEX_STR           "3F007F256F7D"
#define USIMM_CDMA_EFMMSCFG_STR              "3F007F256F7E"
#define USIMM_CDMA_EFJDL_STR                 "3F007F256F7F"
#define USIMM_CDMA_EFMODEL_STR               "3F007F256F90"
#define USIMM_CDMA_EFRC_STR                  "3F007F256F91"
#define USIMM_CDMA_EFAPPLAB_STR              "3F007F256F92"

#define USIMM_ISIM_STR                       "3F007FFF"
#define USIMM_ISIM_EFIST_STR                 "3F007FFF6F07"
#define USIMM_ISIM_EFIMPI_STR                "3F007FFF6F02"
#define USIMM_ISIM_EFDOMAIN_STR              "3F007FFF6F03"
#define USIMM_ISIM_EFIMPU_STR                "3F007FFF6F04"
#define USIMM_ISIM_EFAD_STR                  "3F007FFF6FAD"
#define USIMM_ISIM_EFARR_STR                 "3F007FFF6F06"
#define USIMM_ISIM_EFPCSCF_STR               "3F007FFF6F09"
#define USIMM_ISIM_EFGBAP_STR                "3F007FFF6FD5"
#define USIMM_ISIM_EFGBANL_STR               "3F007FFF6FD7"
#define USIMM_ISIM_EFNAFKCA_STR              "3F007FFF6FDD"
#define USIMM_ISIM_EFUICCIARI_STR            "3F007FFF6FE7"
#define USIMM_ISIM_EFSMS_STR                 "3F007FFF6F3C"
#define USIMM_ISIM_EFSMSS_STR                "3F007FFF6F43"
#define USIMM_ISIM_EFSMSR_STR                "3F007FFF6F47"
#define USIMM_ISIM_EFSMSP_STR                "3F007FFF6F42"

#define USIMM_SLOT_SIZE 3

enum USIMM_CmdTypeReq {
    USIMM_ACTIVECARD_REQ            = 0,
    USIMM_INITCONTINUE_REQ          = 1,
    USIMM_PROTECTRESET_REQ          = 2,
    USIMM_DEACTIVECARD_REQ          = 3,
    USIMM_AUTHENTICATION_REQ        = 4,
    USIMM_PINHANDLE_REQ             = 5,
    USIMM_UPDATEFILE_REQ            = 6,
    USIMM_READFILE_REQ              = 7,
    USIMM_QUERYFILE_REQ             = 8,
    USIMM_STATUSCMD_REQ             = 9,
    USIMM_SEARCHFILE_REQ            = 10,
    USIMM_FBDNPROCESS_REQ           = 11,
    USIMM_OPENCHANNEL_REQ           = 12,
    USIMM_CLOSECHANNEL_REQ          = 13,
    USIMM_SENDTPDUDATA_REQ          = 14,
    USIMM_STKTERMINALRSP_REQ        = 15,
    USIMM_STKENVELOPE_REQ           = 16,
    USIMM_STKREFRESH_REQ            = 17,
    USIMM_PBINIT_STATUS_IND         = 18,
    USIMM_RACCESS_REQ               = 19,
    USIMM_SETMUTILFILE_REQ          = 20,
    USIMM_RESERVE_IND               = 21,
    USIMM_CARDSTATUS_CB_IND         = 22,
    USIMM_BSCHALLENGE_REQ           = 23,
    USIMM_GENERATE_KEYVPM_REQ       = 24,
    USIMM_MAMAGESSD_REQ             = 25,
    USIMM_CDMASPECAUTH_REQ          = 26,
    USIMM_QUERYESNMEIDRSLT_REQ      = 27,
    USIMM_VSIMFILEDATAWRITE_REQ     = 28,
    USIMM_CHANGE_CARDMODE_REQ       = 29,
    USIMM_FDNQUERY_REQ              = 30,
    USIMM_QUERYVOLTAGE_REQ          = 31,
    USIMM_UPDATEFILE_WITHIMSI_REQ   = 32,
    USIMM_PRIVATECGLAHANDLE_REQ     = 33,
    USIMM_CARDTYPEEX_REQ            = 34,
    USIMM_SILENTPINHANDLE_REQ       = 35,
    USIMM_NVREFRESH_REQ             = 36,
    USIMM_SCI_CFG_SET_REQ           = 37,
    USIMM_SCICHG_IND                = 38,
    USIMM_ESIMSWITCH_SET_REQ        = 39,
    USIMM_ESIMSWITCH_QRY_REQ        = 40,
    USIMM_GETIDENTITY_REQ           = 41,
    USIMM_PASSTHROUGH_SET_REQ       = 42,
    USIMM_PASSTHROUGH_QUERY_REQ     = 43,
#if (FEATURE_MBB_CUST == FEATURE_ON)
#if (FEATURE_PHONE_USIM == FEATURE_OFF)
    USIMM_NOCARD_SET_REQ            = 44,
    USIMM_ACTIVE_OPEN_SPEED_REQ     = 45,
#endif
#endif
#if (FEATURE_ASSOCIATED_CARD_LOCK == FEATURE_ON)
    USIMM_CARDEXIST_QUERY_REQ       = 46,
#endif

    USIMM_CMDTYPE_REQ_BUTT
};
typedef VOS_UINT32 USIMM_CmdTypeReqUint32;

enum USIMM_CmdTypeCnf {
    USIMM_ACTIVECARD_CNF        = 0,
    USIMM_INITCONTINUE_CNF      = 1,
    USIMM_PROTECTRESET_CNF      = 2,
    USIMM_DEACTIVECARD_CNF      = 3,
    USIMM_AUTHENTICATION_CNF    = 4,
    USIMM_PINHANDLE_CNF         = 5,
    USIMM_UPDATEFILE_CNF        = 6,
    USIMM_READFILE_CNF          = 7,
    USIMM_QUERYFILE_CNF         = 8,
    USIMM_STATUSCMD_CNF         = 9,
    USIMM_SEARCHFILE_CNF        = 10,
    USIMM_FBDNPROCESS_CNF       = 11,
    USIMM_OPENCHANNEL_CNF       = 12,
    USIMM_CLOSECHANNEL_CNF      = 13,
    USIMM_SENDTPDUDATA_CNF      = 14,
    USIMM_BSCHALLENGE_CNF       = 15,
    USIMM_GENERATE_KEYVPM_CNF   = 16,
    USIMM_MANAGESSD_CNF         = 17,
    USIMM_STKTERMINALRSP_CNF    = 18,
    USIMM_STKREFRESH_CNF        = 19,
    USIMM_STKENVELOPE_CNF       = 20,
    USIMM_RACCESS_CNF           = 21,
    USIMM_SETMUTILFILE_CNF      = 22,
    USIMM_CDMASPECAUTH_CNF      = 23,
    USIMM_QUERYESNMEIDRSLT_CNF  = 24,
    USIMM_CHANGE_CARDMODE_CNF   = 25,
    USIMM_FDNQUERY_CNF          = 26,
    USIMM_QUERYVOLTAGE_CNF      = 27,
    USIMM_PRIVATECGLAHANDLE_CNF = 28,
    USIMM_CARDTYPEEX_CNF        = 29,
    USIMM_SILENTPINHANDLE_CNF   = 30,
    USIMM_NVREFRESH_CNF         = 31,
    USIMM_SCI_CFG_SET_CNF       = 32,
    USIMM_ESIMSWITCH_SET_CNF    = 33,
    USIMM_ESIMSWITCH_QRY_CNF    = 34,
    USIMM_GETIDENTITY_CNF       = 35,
    USIMM_PASSTHROUGH_SET_CNF   = 36,
    USIMM_PASSTHROUGH_QUERY_CNF = 37,
#if (FEATURE_MBB_CUST == FEATURE_ON)
#if (FEATURE_PHONE_USIM == FEATURE_OFF)
    USIMM_NOCARD_SET_CNF        = 38,
#endif
#endif
#if (FEATURE_ASSOCIATED_CARD_LOCK == FEATURE_ON)
    USIMM_CARDEXIST_QUERY_CNF   = 39,
#endif

    USIMM_CARDSTATUS_IND        = 100,
    USIMM_STKREFRESH_IND        = 101,
    USIMM_STKDATA_IND           = 102,
    USIMM_ECCNUMBER_IND         = 103,
    USIMM_VSIM_RDH_IND          = 104,
    USIMM_HOTINOUT_IND          = 105,
    USIMM_XECCNUMBER_IND        = 106,
    USIMM_CARDSCIERR_IND        = 107,
    USIMM_CARDERROR_IND         = 108,
    USIMM_ICCICCONTENT_IND      = 109,
    USIMM_HOTPLUGSTATUS_IND     = 110,
    USIMM_SWCHECKSTATUS_IND     = 111,
    USIMM_FDNSTATUS_IND         = 112,
    USIMM_PRIVATECGLAHANDLE_IND = 113,
    USIMM_CARDFETCH_IND         = 114,

    USIMM_ATR_RPT_IND           = 115,
    USIMM_PB_STATUS_IND         = 116,
    USIMM_SCICHG_CARDSTATUS_IND = 117,
    USIMM_SUPPORT_5GCARD_IND    = 118,
    USIMM_STKENVELOPE_RETRY     = 119,
    USIMM_REFRESH_PLMN_IND      = 120,
    USIMM_UPDATE_FILE_AT_IND    = 121,
    USIMM_UICC_TO_ICC_CHR_IND   = 122,
    USIMM_CARD_BUSY_CHR_IND     = 123,
    USIMM_FCP_ERROR_CHR_IND     = 124,
    USIMM_AUTH_FAIL_CHR_IND     = 125,
    USIMM_STATIS_EVENT_IND      = 126,

    ID_USIMM_MTA_NV_REFRESH_RSP = 0x2002, /* 防止USIMM->MTA新增消息和ID_MTA_NV_REFRESH_RSP取值重复 */
    USIMM_CMDTYPE_BUTT
};
typedef VOS_UINT32 USIMM_CmdTypeCnfUint32;

enum USIMM_SWCheck {
    USIMM_SW_OK                 = VOS_OK,   /* 命令执行正常 */
    USIMM_SW_ERR,                   /* 1,   命令执行错误 */
    USIMM_SW_OK_WITH_RESOPNSE,      /* 2,   命令执行正常，有数据返回 */
    USIMM_SW_OK_WITH_SAT,           /* 3,   命令执行正常，有SAT数据返回 */
    USIMM_SW_CMD_REMAINTIME,        /* 4,   当前命令的剩余次数 */
    USIMM_SW_CARD_BUSY,             /* 5,   当前卡忙 */
    USIMM_SW_AUTN_ERROR,            /* 6,   鉴权错误 */
    USIMM_SW_MEMORY_ERROR,          /* 7,   卡内存错误 */
    USIMM_SW_NOFILE_SELECT,         /* 8,   当前无任何文件选中 */
    USIMM_SW_OUTOF_RANGE,           /* 9,   超出范围 */
    USIMM_SW_NOFILE_FOUND,          /* 10,  文件找不到 */
    USIMM_SW_FILETYPE_ERROR,        /* 11,  文件类型矛盾 */
    USIMM_SW_SECURITY_ERROR,        /* 12,  安全状态不满足 */
    USIMM_SW_PIN_BLOCK,             /* 13,  密码被锁 */
    USIMM_SW_PINSTATUS_ERROR,       /* 14,  密码状态矛盾 */
    USIMM_SW_PINNUMBER_ERROR,       /* 15,  密码索引错误 */
    USIMM_SW_FILESTATUS_ERROR,      /* 16,  文件状态矛盾 */
    USIMM_SW_INCREACE_ERROR,        /* 17,  INCREACE操作错误 */
    USIMM_SW_CLA_ERROR,             /* 18,  CLA错误 */
    USIMM_SW_CMD_ERROR,             /* 19,  参数1、2、3、命令数据错误 */
    USIMM_SW_P3_ERROR,              /* 20,  推荐P3长度 */
    USIMM_SW_NORECORD_FOUND,        /* 21,  无记录 */
    USIMM_SW_DOWNLOAD_ERROR,        /* 22,  下载数据错误 */
    USIMM_SW_CHANNEL_NOSUPPORT,     /* 23,  不支持逻辑通道 */
    USIMM_SW_WARNING,               /* 24,  命令执行告警,需要获取信息 */
    USIMM_SW_OTHER_WARNING,         /* 25,  命令执行告警 */
    USIMM_SW_OTHER_ERROR,           /* 26,  命令执行错误 */
    USIMM_SW_SENDCMD_ERROR,         /* 27,  发送命令错误 */
    USIMM_SW_NO_INFO,               /* 28,  No information given, state of non volatile memory unchanged */
    USIMM_SW_DATA_CORRUPTED,        /* 29,  Part of returned data may be corrupted */
    USIMM_SW_END_OF_FILE,           /* 30,  End of file/record reached before reading Le bytes */
    USIMM_SW_INVALID_FILE,          /* 31,  Selected file invalidated */
    USIMM_SW_TERMINATE_FILE,        /* 32,  Selected file in termination state */
    USIMM_SW_MORE_DATA1,            /* 33,  More data available */
    USIMM_SW_MORE_DATA2,            /* 34,  More data available and proactive command pending */
    USIMM_SW_RESPONSE_DATA,         /* 35,  Authentication response data available */
    USIMM_SW_MORE_DATA3,            /* 36,  More data expected */
    USIMM_SW_MORE_DATA4,            /* 37,  More data expected and proactive command pending */
    USIMM_SW_TECH_ERROR,            /* 38,  Technical problem, no precise diagnosis */
    USIMM_SW_COMMAND_ERROR,         /* 39,  Command not allowed - secure channel - security not satisfied */
    USIMM_SW_EXPIRED_ERROR,         /* 40,  Security session or association expired */
    USIMM_SW_UNHANDLE_ERROR,        /* 41,  状态字与协议定义的INS字段不符 */
    USIMM_SW_CDMA_AUTN_ERROR,       /* 42,  CDMA鉴权过程错误 */
    USIMM_SW_MSGCHECK_ERROR,        /* 43,  参数检查错误 */
    USIMM_SW_INVALID_BAKID,         /* 44,  BCMCS特殊错误 */
    USIMM_SW_INVALID_BCMCSFID,      /* 45,  BCMCS特殊错误 */
    USIMM_SW_DFPATH_ERROR,          /* 46,  Status路径不匹配 */
    USIMM_SW_CLA_NOT_SUPPORT,       /* 47,  CLA错误,需往ICC切 */
    USIMM_SW_REF_DATA_INVALID,      /* 48,  Referenced Data Invalidated */

    USIMM_SW_BUTT
};
typedef VOS_UINT32 USIMM_SWCheckUint32;

enum USIMM_CardApp {
    USIMM_GUTL_APP          = 0,
    USIMM_IMS_APP           = 1,
    USIMM_CDMA_APP          = 2,
    USIMM_CARDAPP_BUTT      = 3,
    USIMM_UNLIMIT_APP1      = USIMM_CARDAPP_BUTT,
    USIMM_UNLIMIT_APP2      = 4,
    USIMM_UNLIMIT_APP3      = 5,
    USIMM_UNLIMIT_APP4      = 6,
    USIMM_UNLIMIT_APP5      = 7,
    USIMM_UNLIMIT_APP6      = 8,
    USIMM_UNLIMIT_APP7      = 9,
    USIMM_UNLIMIT_APP8      = 10,
    USIMM_UNLIMIT_APP9      = 11,
    USIMM_UNLIMIT_APP10     = 12,
    USIMM_UNLIMIT_APP11     = 13,
    USIMM_UNLIMIT_APP12     = 14,
    USIMM_UNLIMIT_APP13     = 15,
    USIMM_UNLIMIT_APP14     = 16,
    USIMM_UNLIMIT_APP15     = 17,
    USIMM_UNLIMIT_APP16     = 18,
    USIMM_UNLIMIT_APP17     = 19,
    USIMM_UNLIMIT_APP18     = 20,
    USIMM_UNLIMIT_APP19     = 21,
    USIMM_UNLIMIT_AUTO      = 22,
    USIMM_UNLIMIT_APP_BUTT
};
typedef VOS_UINT32 USIMM_CardAppUint32;

enum USIMM_PhyCardType {
    USIMM_PHYCARD_TYPE_ICC              = 0x00,
    USIMM_PHYCARD_TYPE_UICC             = 0x01,
    USIMM_PHYCARD_TYPE_NOCARD           = 0xFF,
    USIMM_PHYCARD_TYPE_BUTT
};

typedef VOS_UINT32 USIMM_PhyCardTypeUint32;

enum USIMM_ApiResult {    /* 对外接口返回的错误码 */
    USIMM_API_SUCCESS = VOS_OK, /* 函数执行成功 */
    USIMM_API_FAILED,           /* 函数执行失败，原因不明 */
    USIMM_API_WRONG_PARA,       /* 因为参数错误，函数执行失败 */
    USIMM_API_NOTAVAILABLE,     /* 因为没有可用应用，函数执行失败 */
    USIMM_API_RESULT_BUTT
};

enum USIMM_PinCmdType {
    USIMM_PINCMD_NULL           = 0x00000000,
    USIMM_PINCMD_VERIFY         = 0x00000020,   /* 校验PIN码 */
    USIMM_PINCMD_CHANGE         = 0x00000024,   /* 更换PIN码的密码 */
    USIMM_PINCMD_DISABLE        = 0x00000026,   /* 禁止使用PIN码 */
    USIMM_PINCMD_ENABLE         = 0x00000028,   /* 使用PIN码 */
    USIMM_PINCMD_UNBLOCK        = 0x0000002C,   /* 解除阻止PIN码 */
    USIMM_PINCMD_BUTT
};
typedef VOS_UINT32      USIMM_PinCmdTypeUint32;

enum USIMM_PinType {
    USIMM_PIN                   = 0x00000000,      /* PIN1 */
    USIMM_PUK                   = 0x00000001,      /* PUK1 */
    USIMM_PIN2                  = 0x00000006,      /* PIN2 */
    USIMM_PUK2                  = 0x00000007,      /* PUK2 */
    USIMM_PHSIM_PIN             = 0x00000008,      /* PH-SIM PIN */
    USIMM_PHNET_PIN             = 0x00000009,      /* PH-NET PIN */
    USIMM_SIM_NON               = 0x000000FF,      /* 不确定操作PIN的类型时填入此值 */
    USIMM_PIN_TYPE_BUTT
};
typedef VOS_UINT32      USIMM_PinTypeUint32;

enum  USIMM_PinEnableStatus {
    USIMM_PIN_DISABLED      = 0,      /* 未使能 */
    USIMM_PIN_ENABLED       = 1,       /* 已经使能 */
    USIMM_PIN_ENABLE_BUTT
};
typedef VOS_UINT32      USIMM_PinEnableStatusUint32;

enum USIMM_PinVeriyStatus {
    USIMM_PIN_NONEED        = 0, /* 不需要验证 */
    USIMM_PIN_NEED          = 1, /* 需要验证 */
    USIMM_PUK_NEED          = 2, /* 需要验证puk */
    USIMM_PUK_BLOCK         = 3, /* 需要验证ADM */
    USIMM_PIN_VERIY_BUTT
};
typedef VOS_UINT32      USIMM_PinVeriyStatusUint32;

enum USIMM_RefreshType {
    USIMM_INITIAL_FULL_FILE_CHANGE_NOTIFY = 0,
    USIMM_FILE_CHANGE_NOTIFY             = 1,
    USIMM_INITIAL_FILE_CHANGE_NOTIFY     = 2,
    USIMM_INITIALIZATION                 = 3,
    USIMM_RESET                          = 4,
    USIMM_APPLICATION_RESET_3G_ONLY      = 5,
    USIMM_SESSION_RESET_3G_ONLY          = 6,
    USIMM_STEERING_OF_ROAMING            = 7,
    USIMM_REFRESH_BUTT
};
typedef VOS_UINT32      USIMM_RefreshTypeUint32;

enum USIMM_DefFileid {
    /* EF under MF */
    USIMM_MF_ID       = 0,
    USIMM_DIR_ID,
    USIMM_ICCID_ID,
    USIMM_PL_ID,
    USIMM_ARR_ID,
    USIMM_USIM_ID,
    USIMM_USIM_EFLI_ID,
    USIMM_USIM_EFARR_ID,
    USIMM_USIM_EFIMSI_ID,
    USIMM_USIM_EFKEYS_ID,
    USIMM_USIM_EFKEYSPS_ID,
    USIMM_USIM_EFCSP_ID,
    USIMM_USIM_EFDCK_ID,
    USIMM_USIM_EFHPPLMN_ID,
    USIMM_USIM_EFCNL_ID,
    USIMM_USIM_EFACMMAX_ID,
    USIMM_USIM_EFUST_ID,
    USIMM_USIM_EFACM_ID,
    USIMM_USIM_EFFDN_ID,
    USIMM_USIM_EFSMS_ID,
    USIMM_USIM_EFGID1_ID,
    USIMM_USIM_EFGID2_ID,
    USIMM_USIM_EFMSISDN_ID,
    USIMM_USIM_EFPUCT_ID,
    USIMM_USIM_EFSMSP_ID,
    USIMM_USIM_EFSMSS_ID,
    USIMM_USIM_EFCBMI_ID,
    USIMM_USIM_EFSPN_ID,
    USIMM_USIM_EFSMSR_ID,
    USIMM_USIM_EFCBMID_ID,
    USIMM_USIM_EFSDN_ID,
    USIMM_USIM_EFEXT2_ID,
    USIMM_USIM_EFEXT3_ID,
    USIMM_USIM_EFBDN_ID,
    USIMM_USIM_EFEXT5_ID,
    USIMM_USIM_EFCCP2_ID,
    USIMM_USIM_EFCBMIR_ID,
    USIMM_USIM_EFEXT4_ID,
    USIMM_USIM_EFEST_ID,
    USIMM_USIM_EFACL_ID,
    USIMM_USIM_EFCMI_ID,
    USIMM_USIM_EFSTART_HFN_ID,
    USIMM_USIM_EFTHRESHOL_ID,
    USIMM_USIM_EFPLMNWACT_ID,
    USIMM_USIM_EFOPLMNWACT_ID,
    USIMM_USIM_EFHPLMNwACT_ID,
    USIMM_USIM_EFPSLOCI_ID,
    USIMM_USIM_EFACC_ID,
    USIMM_USIM_EFFPLMN_ID,
    USIMM_USIM_EFLOCI_ID,
    USIMM_USIM_EFICI_ID,
    USIMM_USIM_EFOCI_ID,
    USIMM_USIM_EFICT_ID,
    USIMM_USIM_EFOCT_ID,
    USIMM_USIM_EFAD_ID,
    USIMM_USIM_EFVGCS_ID,
    USIMM_USIM_EFVGCSS_ID,
    USIMM_USIM_EFVBS_ID,
    USIMM_USIM_EFVBSS_ID,
    USIMM_USIM_EFEMLPP_ID,
    USIMM_USIM_EFAAEM_ID,
    USIMM_USIM_EFECC_ID,
    USIMM_USIM_EFHIDDENKEY_ID,
    USIMM_USIM_EFNETPAR_ID,
    USIMM_USIM_EFPNN_ID,
    USIMM_USIM_EFOPL_ID,
    USIMM_USIM_EFMBDN_ID,
    USIMM_USIM_EFEXT6_ID,
    USIMM_USIM_EFMBI_ID,
    USIMM_USIM_EFMWIS_ID,
    USIMM_USIM_EFCFIS_ID,
    USIMM_USIM_EFEXT7_ID,
    USIMM_USIM_EFSPDI_ID,
    USIMM_USIM_EFMMSN_ID,
    USIMM_USIM_EFEXT8_ID,
    USIMM_USIM_EFMMSICP_ID,
    USIMM_USIM_EFMMSUP_ID,
    USIMM_USIM_EFMMSUCP_ID,
    USIMM_USIM_EFNIA_ID,
    USIMM_USIM_EFVGCSCA_ID,
    USIMM_USIM_EFVBSCA_ID,
    USIMM_USIM_EFGBAP_ID,
    USIMM_USIM_EFMSK_ID,
    USIMM_USIM_EFMUK_ID,
    USIMM_USIM_EFEHPLMN_ID,
    USIMM_USIM_EFGBANL_ID,
    USIMM_USIM_EFEHPLMNPI_ID,
    USIMM_USIM_EFLRPLMNSI_ID,
    USIMM_USIM_EFNAFKCA_ID,
    USIMM_USIM_EFSPNI_ID,
    USIMM_USIM_EFPNNI_ID,
    USIMM_USIM_EFNCPIP_ID,
    USIMM_USIM_EFEPSLOCI_ID,
    USIMM_USIM_EFEPSNSC_ID,
    USIMM_USIM_EFUFC_ID,
    USIMM_USIM_EFUICCIARI_ID,
    USIMM_USIM_EFNASCONFIG_ID,
    USIMM_USIM_EFPWS_ID,
    USIMM_USIM_EFFDNURI_ID,
    USIMM_USIM_EFSDNURI_ID,
    USIMM_USIM_EFEPDGID_ID,
    USIMM_USIM_EFEPDGSELECTION_ID,
    USIMM_USIM_EFEPDGIDEM_ID,
    USIMM_USIM_EFEPDGSELECTIONEM_ID,
    USIMM_USIM_DFPHONEBOOK_ID,
    USIMM_USIM_EFPSC_ID,
    USIMM_USIM_EFCC_ID,
    USIMM_USIM_EFPUID_ID,
    USIMM_USIM_EFPBR_ID,
    USIMM_USIM_DFGSM_ACCESS_ID,
    USIMM_USIM_EFKC_ID,
    USIMM_USIM_EFKCGPRS_ID,
    USIMM_USIM_EFCPBCCH_ID,
    USIMM_USIM_EFINVSCAN_ID,
    USIMM_USIM_DFMEXE_ID,
    USIMM_USIM_EFMexE_ST_ID,
    USIMM_USIM_EFORPK_ID,
    USIMM_USIM_EFARPK_ID,
    USIMM_USIM_EFTPRK_ID,
    USIMM_USIM_DFSOLSA_ID,
    USIMM_USIM_EFSAI_ID,
    USIMM_USIM_EFSLL_ID,
    USIMM_USIM_DFWLAN_ID,
    USIMM_USIM_EFPSEUDO_ID,
    USIMM_USIM_EFUPLMNWLAN_ID,
    USIMM_USIM_EF0PLMNWLAN_ID,
    USIMM_USIM_EFUWSIDL_ID,
    USIMM_USIM_EFOWSIDL_ID,
    USIMM_USIM_EFWRI_ID,
    USIMM_USIM_EFHWSIDL_ID,
    USIMM_USIM_EFWEHPLMNPI_ID,
    USIMM_USIM_EFWHPI_ID,
    USIMM_USIM_EFWLRPLMN_ID,
    USIMM_USIM_EFHPLMNDAI_ID,
    USIMM_USIM_DFHNB_ID,
    USIMM_USIM_EFACSGL_ID,
    USIMM_USIM_EFCSGT_ID,
    USIMM_USIM_EFHNBN_ID,
    USIMM_USIM_EFOCSGL_ID,
    USIMM_USIM_EFOCSGT_ID,
    USIMM_USIM_EFOHNBN_ID,
    USIMM_USIM_DFPROSE_ID,
    USIMM_USIM_EFPST_ID,
    USIMM_USIM_DFACDC_ID,
    USIMM_USIM_EFACDCLIST_ID,
    USIMM_USIM_DF5GS_ID,
    USIMM_USIM_EF5GS3GPPLOCI_ID,
    USIMM_USIM_EF5GAUTHKEYS_ID,
    USIMM_USIM_EF5GS3GPPNSC_ID,
    USIMM_USIM_EFUAC_AIC_ID,
    USIMM_USIM_EFSUCI_CALC_INFO_ID,
    USIMM_USIM_EFOPL_5GS_ID,
    USIMM_USIM_EFROUTING_INDICATOR_ID,
    USIMM_TELE_ID,
    USIMM_TELE_EFADN_ID,
    USIMM_TELE_EFFDN_ID,
    USIMM_TELE_EFSMS_ID,
    USIMM_TELE_EFCCP_ID,
    USIMM_TELE_EFMSISDN_ID,
    USIMM_TELE_EFSMSP_ID,
    USIMM_TELE_EFSMSS_ID,
    USIMM_TELE_EFLND_ID,
    USIMM_TELE_EFSMSR_ID,
    USIMM_TELE_EFSDN_ID,
    USIMM_TELE_EFEXT1_ID,
    USIMM_TELE_EFEXT2_ID,
    USIMM_TELE_EFEXT3_ID,
    USIMM_TELE_EFBDN_ID,
    USIMM_TELE_EFEXT4_ID,
    USIMM_TELE_EFECCP_ID,
    USIMM_TELE_EFRMA_ID,
    USIMM_TELE_EFSUME_ID,
    USIMM_TELE_EFICE_DN_ID,
    USIMM_TELE_EFICE_FF_ID,
    USIMM_TELE_EFPSISMSC_ID,
    USIMM_TELE_DFGRAP_ID,
    USIMM_TELE_DFGRAP_EFIMG_ID,
    USIMM_TELE_DFGRAP_EFICEGRAPH_ID,
    USIMM_TELE_DFGRAP_EFLAUNCHSCWS_ID,
    USIMM_TELE_DFPB_ID,
    USIMM_TELE_DFPB_EFPSC_ID,
    USIMM_TELE_DFPB_EFCC_ID,
    USIMM_TELE_DFPB_EFPUID_ID,
    USIMM_TELE_DFPB_EFPBR_ID,
    USIMM_TELE_DFMM_ID,
    USIMM_TELE_DFMM_EFMML_ID,
    USIMM_TELE_DFMUM_EFMMDF_ID,
    USIMM_TELE_DFMSS_ID,
    USIMM_TELE_EFMLPL_ID,
    USIMM_TELE_EFMSPL_ID,
    USIMM_TELE_EFMMSSMODE_ID,
    USIMM_TELE_DFMCPTT_ID,
    USIMM_TELE_EFMST_ID,
    USIMM_CSIM_ID,
    USIMM_CSIM_EFCOUNT_ID,
    USIMM_CSIM_EFIMSIM_ID,
    USIMM_CSIM_EFIMSIT_ID,
    USIMM_CSIM_EFTMSI_ID,
    USIMM_CSIM_EFAH_ID,
    USIMM_CSIM_EFAOP_ID,
    USIMM_CSIM_EFALOC_ID,
    USIMM_CSIM_EFCDMAHOME_ID,
    USIMM_CSIM_EFZNREGI_ID,
    USIMM_CSIM_EFSNREGI_ID,
    USIMM_CSIM_EFDISTREGI_ID,
    USIMM_CSIM_EFACCOLC_ID,
    USIMM_CSIM_EFTERM_ID,
    USIMM_CSIM_EFSSCI_ID,
    USIMM_CSIM_EFACP_ID,
    USIMM_CSIM_EFPRL_ID,
    USIMM_CSIM_EFRUIMID_ID,
    USIMM_CSIM_EFCSIMST_ID,
    USIMM_CSIM_EFSPC_ID,
    USIMM_CSIM_EFOTAPASPC_ID,
    USIMM_CSIM_EFNAMLOCK_ID,
    USIMM_CSIM_EFOTA_ID,
    USIMM_CSIM_EFSP_ID,
    USIMM_CSIM_EFESNMEID_ID,
    USIMM_CSIM_EFLI_ID,
    USIMM_CSIM_EFFDN_ID,
    USIMM_CSIM_EFSMS_ID,
    USIMM_CSIM_EFSMSP_ID,
    USIMM_CSIM_EFSMSS_ID,
    USIMM_CSIM_EFSSFC_ID,
    USIMM_CSIM_EFSPN_ID,
    USIMM_CSIM_EFUSGIND_ID,
    USIMM_CSIM_EFAD_ID,
    USIMM_CSIM_EFMDN_ID,
    USIMM_CSIM_EFMAXPRL_ID,
    USIMM_CSIM_EFSPCS_ID,
    USIMM_CSIM_EFECC_ID,
    USIMM_CSIM_EFME3GPDOPC_ID,
    USIMM_CSIM_EF3GPDOPM_ID,
    USIMM_CSIM_EFSIPCAP_ID,
    USIMM_CSIM_EFMIPCAP_ID,
    USIMM_CSIM_EFSIPUPP_ID,
    USIMM_CSIM_EFMIPUPP_ID,
    USIMM_CSIM_EFSIPSP_ID,
    USIMM_CSIM_EFMIPSP_ID,
    USIMM_CSIM_EFSIPPAPSS_ID,
    USIMM_CSIM_EFPUZL_ID,
    USIMM_CSIM_EFMAXPUZL_ID,
    USIMM_CSIM_EFMECRP_ID,
    USIMM_CSIM_EFHRPDCAP_ID,
    USIMM_CSIM_EFHRPDUPP_ID,
    USIMM_CSIM_EFCSSPR_ID,
    USIMM_CSIM_EFATC_ID,
    USIMM_CSIM_EFEPRL_ID,
    USIMM_CSIM_EFBCSMSCFG_ID,
    USIMM_CSIM_EFBCSMSPREF_ID,
    USIMM_CSIM_EFBCSMSTABLE_ID,
    USIMM_CSIM_EFBCSMSP_ID,
    USIMM_CSIM_EFBAKPARA_ID,
    USIMM_CSIM_EFUPBAKPARA_ID,
    USIMM_CSIM_EFMMSN_ID,
    USIMM_CSIM_EFEXT8_ID,
    USIMM_CSIM_EFMMSICP_ID,
    USIMM_CSIM_EFMMSUP_ID,
    USIMM_CSIM_EFMMSUCP_ID,
    USIMM_CSIM_EFAUTHCAP_ID,
    USIMM_CSIM_EF3GCIK_ID,
    USIMM_CSIM_EFDCK_ID,
    USIMM_CSIM_EFGID1_ID,
    USIMM_CSIM_EFGID2_ID,
    USIMM_CSIM_EFCDMACNL_ID,
    USIMM_CSIM_EFHOMETAG_ID,
    USIMM_CSIM_EFGROUPTAG_ID,
    USIMM_CSIM_EFSPECTAG_ID,
    USIMM_CSIM_EFCALLPROMPT_ID,
    USIMM_CSIM_EFSFEUIMID_ID,
    USIMM_CSIM_EFEST_ID,
    USIMM_CSIM_EFHIDDENKEY_ID,
    USIMM_CSIM_EFLCSVER_ID,
    USIMM_CSIM_EFLCSCP_ID,
    USIMM_CSIM_EFSDN_ID,
    USIMM_CSIM_EFEXT2_ID,
    USIMM_CSIM_EFEXT3_ID,
    USIMM_CSIM_EFICI_ID,
    USIMM_CSIM_EFOCI_ID,
    USIMM_CSIM_EFEXT5_ID,
    USIMM_CSIM_EFCCP2_ID,
    USIMM_CSIM_EFAPPLAB_ID,
    USIMM_CSIM_EFMODEL_ID,
    USIMM_CSIM_EFRC_ID,
    USIMM_CSIM_EFSMSCAP_ID,
    USIMM_CSIM_EFMIPFLAGS_ID,
    USIMM_CSIM_EF3GPDUPPEXT_ID,
    USIMM_CSIM_EFIPV6CAP_ID,
    USIMM_CSIM_EFTCPCFG_ID,
    USIMM_CSIM_EFDGC_ID,
    USIMM_CSIM_EFWAPBWCP_ID,
    USIMM_CSIM_EFWAPBWBM_ID,
    USIMM_CSIM_EFMMSCFG_ID,
    USIMM_CSIM_EFJDL_ID,
    USIMM_CSIM_DFPHONEBOOK_ID,
    USIMM_CSIM_EFPSC_ID,
    USIMM_CSIM_EFCC_ID,
    USIMM_CSIM_EFPUID_ID,
    USIMM_CSIM_EFPBR_ID,
    /* EF under MF/DFGSM */
    USIMM_GSM_ID,
    USIMM_GSM_EFLP_ID,
    USIMM_GSM_EFIMSI_ID,
    USIMM_GSM_EFKC_ID,
    USIMM_GSM_EFDCK_ID,
    USIMM_GSM_EFPLMNSEL_ID,
    USIMM_GSM_EFHPLMN_ID,
    USIMM_GSM_EFCNL_ID,
    USIMM_GSM_EFACMMAX_ID,
    USIMM_GSM_EFSST_ID,
    USIMM_GSM_EFACM_ID,
    USIMM_GSM_EFGID1_ID,
    USIMM_GSM_EFGID2_ID,
    USIMM_GSM_EFPUCT_ID,
    USIMM_GSM_EFCBMI_ID,
    USIMM_GSM_EFSPN_ID,
    USIMM_GSM_EFCBMID_ID,
    USIMM_GSM_EFBCCH_ID,
    USIMM_GSM_EFACC_ID,
    USIMM_GSM_EFFPLMN_ID,
    USIMM_GSM_EFLOCI_ID,
    USIMM_GSM_EFAD_ID,
    USIMM_GSM_EFPHASE_ID,
    USIMM_GSM_EFVGCS_ID,
    USIMM_GSM_EFVGCSS_ID,
    USIMM_GSM_EFVBS_ID,
    USIMM_GSM_EFVBSS_ID,
    USIMM_GSM_EFEMLPP_ID,
    USIMM_GSM_EFAAEM_ID,
    USIMM_GSM_EFECC_ID,
    USIMM_GSM_EFCBMIR_ID,
    USIMM_GSM_EFNIA_ID,
    USIMM_GSM_EFKCGPRS_ID,
    USIMM_GSM_EFLOCIGPRS_ID,
    USIMM_GSM_EFSUME_ID,
    USIMM_GSM_EFCMI_ID,
    USIMM_GSM_EFPLMNWACT_ID,
    USIMM_GSM_EFOPLMNWACT_ID,
    USIMM_GSM_EFHPLMNACT_ID,
    USIMM_GSM_EFCPBCCH_ID,
    USIMM_GSM_EFINVSCAN_ID,
    USIMM_GSM_EFPNN_ID,
    USIMM_GSM_EFOPL_ID,
    USIMM_GSM_EFMBDN_ID,
    USIMM_GSM_EFEXT6_ID,
    USIMM_GSM_EFMBI_ID,
    USIMM_GSM_EFMWIS_ID,
    USIMM_GSM_EFCFIS_ID,
    USIMM_GSM_EFEXT7_ID,
    USIMM_GSM_EFSPDI_ID,
    USIMM_GSM_EFMMSN_ID,
    USIMM_GSM_EFEXT8_ID,
    USIMM_GSM_EFMMSICP_ID,
    USIMM_GSM_EFMMSUP_ID,
    USIMM_GSM_EFMMSUCP_ID,
    /* EF for CPHS, under DFGSM */
    USIMM_GSM_EFVMW_ID,
    USIMM_GSM_EFSVCSTBL_ID,
    USIMM_GSM_EFCFF_ID,
    USIMM_GSM_EFONS_ID,
    USIMM_GSM_EFCSP_ID,
    USIMM_GSM_EFCPHSI_ID,
    USIMM_GSM_EFMN_ID,
    USIMM_GSM_EFONSS_ID,
    USIMM_GSM_EFINFONUM_ID,
    /* EF under MF/DFGSM/DFSOLSA */
    USIMM_GSM_DFSOLSA_ID,
    USIMM_GSM_DFSOLSA_EFSAI_ID,
    USIMM_GSM_DFSOLSA_EFSLL_ID,
    /* EF under MF/DFGSM/DFMExE */
    USIMM_GSM_DFMEXE_ID,
    USIMM_GSM_DFMEXE_EFST_ID,
    USIMM_GSM_DFMEXE_EFORPK_ID,
    USIMM_GSM_DFMEXE_EFARPK_ID,
    USIMM_GSM_DFMEXE_EFTPRPK_ID,
    /* EF under DFCDMA */
    USIMM_CDMA_ID,
    USIMM_CDMA_EFCOUNT_ID,
    USIMM_CDMA_EFIMSIM_ID,
    USIMM_CDMA_EFIMSIT_ID,
    USIMM_CDMA_EFTMSI_ID,
    USIMM_CDMA_EFAH_ID,
    USIMM_CDMA_EFAOP_ID,
    USIMM_CDMA_EFALOC_ID,
    USIMM_CDMA_EFCDMAHOME_ID,
    USIMM_CDMA_EFZNREGI_ID,
    USIMM_CDMA_EFSNREGI_ID,
    USIMM_CDMA_EFDISTREGI_ID,
    USIMM_CDMA_EFACCOLC_ID,
    USIMM_CDMA_EFTERM_ID,
    USIMM_CDMA_EFSSCI_ID,
    USIMM_CDMA_EFACP_ID,
    USIMM_CDMA_EFPRL_ID,
    USIMM_CDMA_EFRUIMID_ID,
    USIMM_CDMA_EFCST_ID,
    USIMM_CDMA_EFSPC_ID,
    USIMM_CDMA_EFOTAPASPC_ID,
    USIMM_CDMA_EFNAMLOCK_ID,
    USIMM_CDMA_EFOTA_ID,
    USIMM_CDMA_EFSP_ID,
    USIMM_CDMA_EFESNMEIDME_ID,
    USIMM_CDMA_EFREVISION_ID,
    USIMM_CDMA_EFRUIMPL_ID,
    USIMM_CDMA_EFSMS_ID,
    USIMM_CDMA_EFSMSP_ID,
    USIMM_CDMA_EFSMSS_ID,
    USIMM_CDMA_EFSSFC_ID,
    USIMM_CDMA_EFSPN_ID,
    USIMM_CDMA_EFUSGIND_ID,
    USIMM_CDMA_EFAD_ID,
    USIMM_CDMA_EFMDN_ID,
    USIMM_CDMA_EFMAXPRL_ID,
    USIMM_CDMA_EFSPCS_ID,
    USIMM_CDMA_EFECC_ID,
    USIMM_CDMA_EFME3GPDOPC_ID,
    USIMM_CDMA_EF3GPDOPM_ID,
    USIMM_CDMA_EFSIPCAP_ID,
    USIMM_CDMA_EFMIPCAP_ID,
    USIMM_CDMA_EFSIPUPP_ID,
    USIMM_CDMA_EFMIPUPP_ID,
    USIMM_CDMA_EFSIPSP_ID,
    USIMM_CDMA_EFMIPSP_ID,
    USIMM_CDMA_EFSIPPAPSS_ID,
    USIMM_CDMA_EFPUZL_ID,
    USIMM_CDMA_EFMAXPUZL_ID,
    USIMM_CDMA_EFMECRP_ID,
    USIMM_CDMA_EFHRPDCAP_ID,
    USIMM_CDMA_EFHRPDUPP_ID,
    USIMM_CDMA_EFCSSPR_ID,
    USIMM_CDMA_EFACT_ID,
    USIMM_CDMA_EFEPRL_ID,
    USIMM_CDMA_EFBCSMSOFG_ID,
    USIMM_CDMA_EFBCSMSPREF_ID,
    USIMM_CDMA_EFBCSMSTBL_ID,
    USIMM_CDMA_EFBCSMSP_ID,
    USIMM_CDMA_EFIMPI_ID,
    USIMM_CDMA_EFDOMAIN_ID,
    USIMM_CDMA_EFIMPU_ID,
    USIMM_CDMA_EFPCSCF_ID,
    USIMM_CDMA_EFBAKPARA_ID,
    USIMM_CDMA_EFUPBAKPARA_ID,
    USIMM_CDMA_EFMMSN_ID,
    USIMM_CDMA_EFEXT8_ID,
    USIMM_CDMA_EFMMSICP_ID,
    USIMM_CDMA_EFMMSUP_ID,
    USIMM_CDMA_EFMMSUCP_ID,
    USIMM_CDMA_EFAUTHCAP_ID,
    USIMM_CDMA_EF3GCIK_ID,
    USIMM_CDMA_EFDCK_ID,
    USIMM_CDMA_EFGID1_ID,
    USIMM_CDMA_EFGID2_ID,
    USIMM_CDMA_EFCDMACNL_ID,
    USIMM_CDMA_EFHOMETAG_ID,
    USIMM_CDMA_EFGROUPTAG_ID,
    USIMM_CDMA_EFSPECTAG_ID,
    USIMM_CDMA_EFCALLPROM_ID,
    USIMM_CDMA_EFSFEUIMID_ID,
    USIMM_CDMA_EFSMSCAP_ID,
    USIMM_CDMA_EFIPV6CAP_ID,
    USIMM_CDMA_EFMIPFLAG_ID,
    USIMM_CDMA_EFTCPCFG_ID,
    USIMM_CDMA_EFDGC_ID,
    USIMM_CDMA_EFWAPBRCP_ID,
    USIMM_CDMA_EFWAPBRBM_ID,
    USIMM_CDMA_EF3GPDUPPEX_ID,
    USIMM_CDMA_EFMMSCFG_ID,
    USIMM_CDMA_EFJDL_ID,
    USIMM_CDMA_EFMODEL_ID,
    USIMM_CDMA_EFRC_ID,
    USIMM_CDMA_EFAPPLAB_ID,
    /* EF under ADFISIM */
    USIMM_ISIM_ID,
    USIMM_ISIM_EFIST_ID,
    USIMM_ISIM_EFIMPI_ID,
    USIMM_ISIM_EFDOMAIN_ID,
    USIMM_ISIM_EFIMPU_ID,
    USIMM_ISIM_EFAD_ID,
    USIMM_ISIM_EFARR_ID,
    USIMM_ISIM_EFPCSCF_ID,
    USIMM_ISIM_EFGBAP_ID,
    USIMM_ISIM_EFGBANL_ID,
    USIMM_ISIM_EFNAFKCA_ID,
    USIMM_ISIM_EFUICCIARI_ID,
    USIMM_ISIM_EFSMS_ID,
    USIMM_ISIM_EFSMSS_ID,
    USIMM_ISIM_EFSMSR_ID,
    USIMM_ISIM_EFSMSP_ID,
    USIMM_DEF_FILEID_BUTT
};
typedef VOS_UINT32 USIMM_DefFileidUint32;
typedef VOS_UINT16 USIMM_DefFileidUint16;

#define EFPLMNsel                               0x6F30 /* 暂时为NAS保留 */
#define EF_CUIM_PRL                             0x6F30   /* T */

#define EF_CUIM_SSCI                            0x6F2E   /* T */
#define EF_CUIM_TERM                            0x6F2D   /* T */
#define EF_CUIM_SMS                             0x6F3C   /* T */
#define EF_CUIM_EPRL                            0x6F5A   /* T */

typedef VOS_UINT16 USIMM_CuimFileidUint16;

enum UICC_ServicesType {
    UICC_SVR_NONE = 0,                       /* 00 */
    SIM_SVR_CHV1,                            /* 01 */       /* CHV1 disable function                                           */
    SIM_SVR_ADN,                             /* 02 */       /* Abbreviated Dialling Numbers (ADN)                              */
    SIM_SVR_FDN,                             /* 03 */       /* Fixed Dialling Numbers (FDN)                                    */
    SIM_SVR_SMS,                             /* 04 */       /* Short Message Storage (SMS)                                     */
    SIM_SVR_AOC,                             /* 05 */       /* Advice of Charge (AoC)                                          */
    SIM_SVR_CCP,                             /* 06 */       /* Capability Configuration Parameters (CCP)                       */
    SIM_SVR_PLMN_SELECTOR,                   /* 07 */       /* PLMN selector                                                   */
    SIM_SVR_RFU1,                            /* 08 */       /* RFU                                                             */
    SIM_SVR_MSISDN,                          /* 09 */       /* MSISDN                                                          */
    SIM_SVR_EXT1,                            /* 10 */       /* Extension1                                                      */
    SIM_SVR_EXT2,                            /* 11 */       /* Extension2                                                      */
    SIM_SVR_SMS_PARA,                        /* 12 */       /* SMS Parameters                                                  */
    SIM_SVR_LND,                             /* 13 */       /* Last Number Dialled (LND)                                       */
    SIM_SVR_CBMID,                           /* 14 */       /* Cell Broadcast Message Identifier                               */
    SIM_SVR_GIDL1,                           /* 15 */       /* Group Identifier Level 1                                        */
    SIM_SVR_GIDL2,                           /* 16 */       /* Group Identifier Level 2                                        */
    SIM_SVR_SERVICE_PRO_NAME,                /* 17 */       /* Service Provider Name                                           */
    SIM_SVR_SDN,                             /* 18 */       /* Service Dialling Numbers (SDN)                                  */
    SIM_SVR_EXT3,                            /* 19 */       /* Extension3                                                      */
    SIM_SVR_RFU2,                            /* 20 */       /* RFU                                                             */
    SIM_SVR_EFVGCS_EFVGCSS,                  /* 21 */       /* VGCS Group Identifier List (EFVGCS and EFVGCSS)                 */
    SIM_SVR_FVBS_EFVBSS,                     /* 22 */       /* VBS Group Identifier List (EFVBS and EFVBSS)                    */
    SIM_SVR_ENH_ML_SVC,                      /* 23 */       /* enhanced Multi Level Precedence and Pre emption Service         */
    SIM_SVR_AUTO_ANSWER_FOR_EMLPP,           /* 24 */       /* Automatic Answer for eMLPP                                      */
    SIM_SVR_DD_VIA_SMS_CB,                   /* 25 */       /* Data download via SMS CB                                        */
    SIM_SVR_DD_VIA_SMS_PP,                   /* 26 */       /* Data download via SMS PP                                        */
    SIM_SVR_MENU_SELECTION,                  /* 27 */       /* Menu selection                                                  */
    SIM_SVR_CALL_CONTROL,                    /* 28 */       /* Call control                                                    */
    SIM_SVR_PRO_SIM,                         /* 29 */       /* Proactive SIM                                                   */
    SIM_SVR_CBMIR,                           /* 30 */       /* Cell Broadcast Message Identifier Ranges                        */
    SIM_SVR_BDN,                             /* 31 */       /* Barred Dialling Numbers (BDN)                                   */
    SIM_SVR_EXT4,                            /* 32 */       /* Extension4                                                      */
    SIM_SVR_DPCK,                            /* 33 */       /* De personalization Control Keys                                 */
    SIM_SVR_CONL,                            /* 34 */       /* Co operative Network List                                       */
    SIM_SVR_SMS_REPORTS,                     /* 35 */       /* Short Message Status Reports                                    */
    SIM_SVR_NID,                             /* 36 */       /* Network's indication of alerting in the MS                      */
    SIM_SVR_MOSMC,                           /* 37 */       /* Mobile Originated Short Message control by SIM                  */
    SIM_SVR_GPRS,                            /* 38 */       /* GPRS                                                            */
    SIM_SVR_IMG,                             /* 39 */       /* Image (IMG)                                                     */
    SIM_SVR_SOLSA,                           /* 40 */       /* SoLSA (Support of Local Service Area)                           */
    SIM_SVR_USSD,                            /* 41 */       /* USSD string data object supported in Call Control               */
    SIM_SVR_RUN_AT_CMD,                      /* 42 */       /* RUN AT COMMAND command                                          */
    SIM_SVR_USER_CTRL_PLMN,                  /* 43 */       /* User controlled PLMN Selector with Access Technology            */
    SIM_SVR_OPERATOR_CTRL_PLMN,              /* 44 */       /* Operator controlled PLMN Selector with Access Technology        */
    SIM_SVR_HPLMN,                           /* 45 */       /* HPLMN Selector with Access Technology                           */
    SIM_SVR_CPBCCHI,                         /* 46 */       /* CPBCCH Information                                              */
    SIM_SVR_INV_SCAN,                        /* 47 */       /* Investigation Scan                                              */
    SIM_SVR_ECCP,                            /* 48 */       /* Extended Capability Configuration Parameters                    */
    SIM_SVR_MEXE,                            /* 49 */       /* MExE                                                            */
    SIM_SVR_RSV,                             /* 50 */       /* Reserved and shall be ignored                                   */
    SIM_SVR_PLMNNN,                          /* 51 */       /* PLMN Network Name                                               */
    SIM_SVR_OPLMNL,                          /* 52 */       /* Operator PLMN List                                              */
    SIM_SVR_MDN,                             /* 53 */       /* Mailbox Dialling Numbers                                        */
    SIM_SVR_MWIS,                            /* 54 */       /* Message Waiting Indication Status                               */
    SIM_SVR_CFIS,                            /* 55 */       /* Call Forwarding Indication Status                               */
    SIM_SVR_SPDI,                            /* 56 */       /* Service Provider Display Information                            */
    SIM_SVR_MMS,                             /* 57 */       /* Multimedia Messaging Service (MMS)                              */
    SIM_SVR_EXT8,                            /* 58 */       /* Extension 8                                                     */
    SIM_SVR_MMS_UCP,                         /* 59 */       /* MMS User Connectivity Parameters                                */
    SIM_SVR_BUTT,

    USIM_SVR_LOCAL_PHONEBOOK = SIM_SVR_BUTT, /* 60 */       /* Local Phonebook, UADF/Phonebook/ADN  */
    USIM_SVR_FDN,                            /* 61 */       /* Forbidden Dialling Numbers                                      */
    USIM_SVR_EXT2,                           /* 62 */       /* Extension 2                                                     */
    USIM_SVR_SDN,                            /* 63 */       /* Service Dialling Numbers                                        */
    USIM_SVR_EXT3,                           /* 64 */       /* Extension 3                                                     */
    USIM_SVR_BDN,                            /* 65 */       /* Barred Dialing Numbers                                          */
    USIM_SVR_EXT4,                           /* 66 */       /* Extension 4                                                     */
    USIM_SVR_OCI_OCT,                        /* 67 */       /* Outgoing Call information/timer                                 */
    USIM_SVR_ICI_ICT,                        /* 68 */       /* Incoming call information/timer                                 */
    USIM_SVR_SMS,                            /* 69 */       /* Short Message Services                                          */
    USIM_SVR_SMSR,                           /* 70 */       /* Short Message Status Reports                                    */
    USIM_SVR_SMSP,                           /* 71 */       /* SMS Parameters                                                  */
    USIM_SVR_AOC,                            /* 72 */       /* Advice of Charge                                                */
    USIM_SVR_CCP,                            /* 73 */       /* Configuration Capability Params                                 */
    USIM_SVR_CBMI,                           /* 74 */       /* Cell Broadcast Message Identifier                               */
    USIM_SVR_CBMIR,                          /* 75 */       /* Cell Broadcast Message Identifiers Range                        */
    USIM_SVR_GID1,                           /* 76 */       /* Group Identifier 1                                              */
    USIM_SVR_GID2,                           /* 77 */       /* Group Identifier 2                                              */
    USIM_SVR_SPN,                            /* 78 */       /* Service Provider Name                                           */
    USIM_SVR_UPLMN_SEL_WACT,                 /* 79 */       /* User controlled PLMN Selector with Access Technology            */
    USIM_SVR_MSISDN,                         /* 80 */       /* MSISDN                                                          */
    USIM_SVR_IMAGE,                          /* 81 */       /* Image (IMG)                                                     */
    USIM_SVR_SOLSA,                          /* 82 */       /* Support of Local Service Area                                   */
    USIM_SVR_ENH_ML_SVC,                     /* 83 */       /* Enhanced Multi Level precedence and Premption Service           */
    USIM_SVR_AA_EMLPP,                       /* 84 */       /* Automatic Answer for eMLPP                                      */
    USIM_SVR_RFU1,                           /* 85 */       /* RFU                                                             */
    USIM_SVR_GSM_ACCESS_IN_USIM,             /* 86 */       /* USIM ADF Access to USIM files for GSM Roaming                   */
    USIM_SVR_DATA_DL_SMSPP,                  /* 87 */       /* Data download via SMS-PP                                        */
    USIM_SVR_DATA_DL_SMSCB,                  /* 88 */       /* Data download via SMS-CB                                        */
    USIM_SVR_CALL_CONTROL,                   /* 89 */       /* Call Control                                                    */
    USIM_SVR_MOSMS_CONTROL,                  /* 90 */       /* MO SMS control                                                  */
    USIM_SVR_RUN_AT_CMD,                     /* 91 */       /* RUN AT COMMAND command                                          */
    USIM_SVR_SET_ONE,                        /* 92 */       /* shall be set to1                                                */
    USIM_SVR_EST,                            /* 93 */       /* Enable Service Table in USIM                                    */
    USIM_SVR_ACL,                            /* 94 */       /* APN Control list                                                */
    USIM_SVR_DEPERSON_KEYS,                  /* 95 */       /* Depersonalization Keys                                          */
    USIM_SVR_COOP_NETWK_LIST,                /* 96 */       /* Co-operative Network List                                       */
    USIM_SVR_GSM_SECURITY_CONTEXT,           /* 97 */       /* GSM Security Context                                            */
    USIM_SVR_CPBCCH,                         /* 98 */       /* CP BCCH                                                         */
    USIM_SVR_INV_SCAN,                       /* 99 */       /* Investigation Scan                                              */
    USIM_SVR_MEXE,                           /* 100 */      /* MExE info                                                       */
    USIM_SVR_OPLMN_SEL_WACT,                 /* 101 */      /* Operator controlled PLMN Selector with Access Technology        */
    USIM_SVR_HPLMN_WACT,                     /* 102 */      /* HPLMN Selector with Access Technology                           */
    USIM_SVR_EXT5,                           /* 103 */      /* Extension 5                                                     */
    USIM_SVR_PLMN_NTWRK_NAME,                /* 104 */      /* PLMN Network Name                                               */
    USIM_SVR_OPLMN_LIST,                     /* 105 */      /* Operator PLMN list                                              */
    USIM_SVR_MAILBOX_DIAL,                   /* 106 */      /* Mailbox dialling numbers                                        */
    USIM_SVR_MSG_WAIT,                       /* 107 */      /* Message Wait indication                                         */
    USIM_SVR_CALL_FWD_IND,                   /* 108 */      /* Call Forward indication status                                  */
    USIM_SVR_RFU2,                           /* 109 */      /* RFU                                                             */
    USIM_SVR_SP_DISP_INFO,                   /* 110 */      /* Service Provider Display Information                            */
    USIM_SVR_MMS,                            /* 111 */      /* Multimedia Messaging Service                                    */
    USIM_SVR_EXT8,                           /* 112 */      /* Extension 8                                                     */
    USIM_SVR_GPRS_USIM,                      /* 113 */      /* Call control on GPRS by USIM                                    */
    USIM_SVR_MMS_USR_P,                      /* 114 */      /* MMS User Connectivity Parameters                                */
    USIM_SVR_NIA,                            /* 115 */      /* Network's indication of alerting in the MS                      */
    USIM_SVR_VGCS_GID_LIST,                  /* 116 */      /* VGCS Group Identifier List                                      */
    USIM_SVR_VBS_GID_LIST,                   /* 117 */      /* VBS Group Identifier List                                       */
    USIM_SVR_PSEUDONYM,                      /* 118 */      /* Pseudonmy                                                       */
    USIM_SVR_UPLMN_WLAN_ACC,                 /* 119 */      /* User Controlled PLMNselector for WLAN access                    */
    USIM_SVR_OPLMN_WLAN_ACC,                 /* 120 */      /* Opeator Controlled PLMN selector for WLAN access                */
    USIM_SVR_USER_CTRL_WSID,                 /* 121 */      /* User controlled WSID list                                       */
    USIM_SVR_OPER_CTRL_WSID,                 /* 122 */      /* Operator controlled WSID list                                   */
    USIM_SVR_VGCS_SECURITY,                  /* 123 */      /* VGCS Security                                                   */
    USIM_SVR_VBS_SECURITY,                   /* 124 */      /* VBS Security                                                    */
    USIM_SVR_WLAN_REAUTH_ID,                 /* 125 */      /* WLAN Reauthentication Identity                                  */
    USIM_SVR_MM_STORAGE,                     /* 126 */      /* Multimedia Messaging Storage                                    */
    USIM_SVR_GBA,                            /* 127 */      /* Generic Bootstrapping Architecture                              */
    USIM_SVR_MBMS_SECURITY,                  /* 128 */      /* MBMS security                                                   */
    USIM_SVR_DATA_DL_USSD,                   /* 129 */      /* Data Downlod via USSD and USSD application mode                 */
    USIM_SVR_EHPLMN,                         /* 130 */      /* EHPLMN                                                          */
    USIM_SVR_TP_AFTER_UICC_ACT,              /* 131 */      /* additional TERMINAL PROFILE after UICC activation               */
    USIM_SVR_EHPLMN_IND,                     /* 132 */      /* Equivalent HPLMN Presentation Indication                        */
    USIM_SVR_RPLMN_LACT,                     /* 133 */      /* RPLMN Last used Access Technology                               */
    USIM_SVR_OMA_BCAST_PROFILE,              /* 134 */      /* OMA BCAST Smart Card Profile                                     */
    USIM_SVR_GBAKEY_EM,                      /* 135 */      /* GBA-based Local Key Establishment Mechanism                      */
    USIM_SVR_TERMINAL_APP,                   /* 136 */      /* Terminal Applications                                            */
    USIM_SVR_SPN_ICON,                       /* 137 */      /* Service Provider Name Icon                                       */
    USIM_SVR_PNN_ICON,                       /* 138 */      /* PLMN Network Name Icon                                           */
    USIM_SVR_CONNECT_SIM_IP,                 /* 139 */      /* Connectivity Parameters for USIM IP connections                  */
    USIM_SVR_IWLEN_ID_LIST,                  /* 140 */      /* Home I-WLAN Specific Identifier List                             */
    USIM_SVR_IWLEN_EHPLMN_IND,               /* 141 */      /* I-WLAN Equivalent HPLMN Presentation Indication                  */
    USIM_SVR_IWLEN_HPLMN_IND,                /* 142 */      /* I-WLAN HPLMN Priority Indication                                 */
    USIM_SVR_IWLEN_RPLMN,                    /* 143 */      /* I-WLAN Last Registered PLMN                                      */
    USIM_SVR_EPS_INFO,                       /* 144 */      /* EPS Mobility Management Information                              */
    USIM_SVR_CSG_COR_IND,                    /* 145 */      /* Allowed CSG Lists and corresponding indications                  */
    USIM_SVR_CALL_CTRL_EPS,                  /* 146 */      /* Call control on EPS PDN connection by USIM                       */
    USIM_SVR_HPLMN_DIR_ACCESS,               /* 147 */      /* HPLMN Direct Access                                              */
    USIM_SVR_ECALL_DATA,                     /* 148 */      /* eCall Data                                                       */
    USIM_SVR_OP_CGS,                         /* 149 */      /* Operator CSG Lists and corresponding indications                 */
    USIM_SVR_SM_VOER_IP,                     /* 150 */      /* Support for SM-over-IP                                           */
    USIM_SVR_CSG_CTRL,                       /* 151 */      /* Support of CSG Display Control                                   */
    USIM_SVR_CTRL_IMS,                       /* 152 */      /* Communication Control for IMS by USIM                            */
    USIM_SVR_APP_EX,                         /* 153 */      /* Extended Terminal Applications                                   */
    USIM_SVR_ACCESS_IMS,                     /* 154 */      /* Support of UICC access to IMS                                    */
    USIM_SVR_NO_ACCESS,                      /* 155 */      /* Non-Access Stratum configuration by USIM                         */
    USIM_SVR_PWS_CONFIG,                     /* 156 */      /* PWS configuration by USIM                                        */
    USIM_SVR_RFU,                            /* 157 */      /* RFU                                                              */
    USIM_SVR_URI_UICC,                       /* 158 */      /* URI support by UICC                                              */
    USIM_SVR_EXTENDED_EARFCN,                /* 159 */      /* Extended EARFCN support                                          */
    USIM_SVR_PROSE,                          /* 160 */      /* ProSe                                                            */
    USIM_SVR_USAT,                           /* 161 */      /* USAT Application Pairing                                         */
    USIM_SVR_MEDIA_TYPE,                     /* 162 */      /* Media Type support                                               */
    USIM_SVR_IMS_CALL,                       /* 163 */      /* IMS call disconnection cause                                     */
    USIM_SVR_URI_MO_SHORT_MESSAGE,           /* 164 */      /* URI support for MO SHORT MESSAGE CONTROL                         */
    USIM_SVR_EPDG,                           /* 165 */      /* ePDG configuration information support                           */
    USIM_SVR_EPDG_CONFIGURED,                /* 166 */      /* ePDG configuration information configured                        */
    USIM_SVR_ACDC,                           /* 167 */      /* ACDC support                                                     */
    USIM_SVR_MCPTT,                          /* 168 */      /* MCPTT                                                            */
    USIM_SVR_EPDG_EMRG,                      /* 169 */      /* ePDG configuration information for Emergency Service support     */
    USIM_SVR_EPDG_EMRG_CONFIGURED,           /* 170 */      /* ePDG configuration information for Emergency Service configured  */
    USIM_SVR_ECALL_DATA_OVER_IMS,            /* 171 */      /* eCall Data over IMS                                              */
    USIM_SVR_URL_SUPORT_SMSPP_DWN,           /* 172 */      /* URI support for SMS-PP DOWNLOAD as defined in 3GPP TS 31.111 [12] */
    USIM_SVR_FROM_PREFERRED,                 /* 173 */      /* From Preferred */
    USIM_SVR_IMS_CFG_DATA,                   /* 174 */      /* IMS configuration data */
    USIM_SVR_TV_CFG,                         /* 175 */      /* TV configuration */
    USIM_SVR_3GPP_PS_DATA_OFF,               /* 176 */      /* 3GPP PS Data Off */
    USIM_SVR_3GPP_PS_DATA_OFF_LIST,          /* 177 */      /* 3GPP PS Data Off Service List */
    USIM_SVR_V2X,                            /* 178 */      /* V2X */
    USIM_SVR_XCAP_CFG_DATA,                  /* 179 */      /* XCAP Configuration Data */
    USIM_SVR_EARFCN_LIST_MTCNB_IOT,          /* 180 */      /* EARFCN list for MTC/NB-IOT UEs */
    USIM_SVR_5GS_MOBIL_MANAGEMENT,           /* 181 */      /* 5GS Mobility Management Information */
    USIM_SVR_5G_SEC_PARA,                    /* 182 */      /* 5G Security Parameters */
    USIM_SVR_SUB_IDENTIY_PRIVACY,            /* 183 */      /* Subscription identifier privacy support */
    USIM_SVR_SUCI_CALC_BY_USIM,              /* 184 */      /* SUCI calculation by the USIM */
    USIM_SVR_UAC_ACCESS_IDENTITIES,          /* 185 */      /* UAC Access Identities support */
    USIM_SVR_VPLMN_CTRL_PLANE_BASE,          /* 186 */      /* Control plane-based steering of UE in VPLMN */
    USIM_SVR_CALL_CTRL_PDU_SESSION,          /* 187 */      /* Call control on PDU Session by USIM */
    USIM_SVR_5GS_OPERATOR_PLMN_LIST,         /* 188 */      /* 5GS Operator PLMN List */
    USIM_SVR_SUPI_NTEWORK_IDENTIFIER,        /* 189 */      /* Support for SUPI of type network spaecific identifier */
    USIM_SVR_3GPP_PS_DATA_OFF_SEP_LISTS,     /* 190 */      /* 3GPP PS Data Off separate Home and Roaming lists */
    USIM_SVR_SUP_URSP_BY_USIM,               /* 191 */      /* Support for URSP by USIM */
    USIM_SVR_5G_SEC_PARA_EXTENDED,           /* 192 */      /* 5G Security Parameters extended */

    USIM_SVR_BUTT,

    ISIM_SVR_PCSCF_ADDR = USIM_SVR_BUTT,     /* 193 */      /* P-CSCF address */
    ISIM_SVR_GBA,                            /* 194 */      /* Generic Bootstrapping Architecture (GBA)                         */
    ISIM_SVR_HTTP_DIGEST,                    /* 195 */      /* HTTP Digest                                                      */
    ISIM_SVR_LOCAL_GBA,                      /* 196 */      /* GBA-based Local Key Establishment Mechanism                      */
    ISIM_SVR_IMS_PCSCF,                      /* 197 */      /* Support of P-CSCF discovery for IMS Local Break Out              */
    ISIM_SVR_SMS,                            /* 198 */      /* Short Message Storage (SMS)                                      */
    ISIM_SVR_SMSR,                           /* 199 */      /* Short Message Status Reports (SMSR)                              */
    ISIM_SVR_SOI,                            /* 200 */      /* Support for SM-over-IP including data download via SMS-PP as defined in TS 31.111 [31] */
    ISIM_SVR_COMM_CTRL,                      /* 201 */      /* Communication Control for IMS by ISIM                            */
    ISIM_SVR_UICC_IMS,                       /* 202 */      /* Support of UICC access to IMS                                  */
    ISIM_SVR_BUTT,

    UIM_SVR_CHV_DISABLE = ISIM_SVR_BUTT,      /* 203 */     /* CHV Disable fuction */
    UIM_SVR_ADN,                              /* 204 */     /* Abbreviated Dialing Numbers */
    UIM_SVR_FDN,                              /* 205 */     /* Fix Dialing Numbers */
    UIM_SVR_SMS,                              /* 206 */     /* Short Message Stroage */
    UIM_SVR_HRPD,                             /* 207 */     /* HRPD */
    UIM_SVR_EPB,                              /* 208 */     /* Enhanced Phone Book */
    UIM_SVR_MMD,                              /* 209 */     /* Multi Media Domain */
    UIM_SVR_SF_EUIMID,                        /* 210 */     /* SF_EUIMID-based EUIMID */
    UIM_SVR_MEID,                             /* 211 */     /* MEID Support */
    UIM_SVR_EXT1,                             /* 212 */     /* Extension1 */
    UIM_SVR_EXT2,                             /* 213 */     /* Extension2 */
    UIM_SVR_SMSP,                             /* 214 */     /* SMS Parameters */
    UIM_SVR_LND,                              /* 215 */     /* Last Number Dialed */
    UIM_SVR_SCP_BC_SMS,                       /* 216 */     /* Service Category Program for BC-SMS */
    UIM_SVR_3GPD_EXT,                         /* 217 */     /* Messaging and 3GPD Extensions */
    UIM_SVR_ROOT_CA,                          /* 218 */     /* Root Certificates */
    UIM_SVR_HOME_SPN,                         /* 219 */     /* CDMA Home Service Provider Name */
    UIM_SVR_SDN,                              /* 220 */     /* Service Dialing Numbers */
    UIM_SVR_EXT3,                             /* 221 */     /* Extension 3 */
    UIM_SVR_3GPD_SIP,                         /* 222 */     /* 3GPD SIP */
    UIM_SVR_WAP,                              /* 223 */     /* WAP Browser */
    UIM_SVR_JAVA,                             /* 224 */     /* Java */
    UIM_SVR_RSV_CDG1,                         /* 225 */     /* Reserved for CDG */
    UIM_SVR_RSV_CDG2,                         /* 226 */     /* Reserved for CDG */
    UIM_SVR_CBS_DOWNLOAD,                     /* 227 */     /* Data Download via SMS Broadcast */
    UIM_SVR_SMS_PP_DOWNOLAD,                  /* 228 */     /* Data Download via SMS-PP */
    UIM_SVR_MENU_SELECTION,                   /* 229 */     /* Menu Selection  */
    UIM_SVR_CALL_CONTROL,                     /* 230 */     /* Call Control */
    UIM_SVR_PROACTIVE_UIM,                    /* 231 */     /* Proactive R-UIM */
    UIM_SVR_AKA,                              /* 232 */     /* AKA */
    UIM_SVR_IPV6,                             /* 233 */     /* IPV6 */
    UIM_SVR_RFU1,                             /* 234 */     /* RFU */
    UIM_SVR_RFU2,                             /* 235 */     /* RFU */
    UIM_SVR_RFU3,                             /* 236 */     /* RFU */
    UIM_SVR_RFU4,                             /* 237 */     /* RFU */
    UIM_SVR_RFU5,                             /* 238 */     /* RFU */
    UIM_SVR_RFU6,                             /* 239 */     /* RFU */
    UIM_SVR_3GPD_MIP,                         /* 240 */     /* 3GPD MIP */
    UIM_SVR_BCMCS,                            /* 241 */     /* BCMCS */
    UIM_SVR_MMS,                              /* 242 */     /* Multimedia Messaging Service */
    UIM_SVR_EXT8,                             /* 243 */     /* Extension 8 */
    UIM_SVR_MMSCP,                            /* 244 */     /* MMS User Connectivity Parameters */
    UIM_SVR_APP_AUTH,                         /* 245 */     /* Application Authentication */
    UIM_SVR_GID1,                             /* 246 */     /* Group Identifier Level 1 */
    UIM_SVR_GID2,                             /* 247 */     /* Group Identifier Level 2 */
    UIM_SVR_CPCK,                             /* 248 */     /* De-Personalization Control Keys */
    UIM_SVR_CNL,                              /* 249 */     /* Cooperative Network List */
    UIM_SVR_BUTT,

    CSIM_SVR_LOCAL_PB = UIM_SVR_BUTT,         /* 250 */     /* Local Phone Book */
    CSIM_SVR_FDN,                             /* 251 */     /* Fixed Dialing Numbers */
    CSIM_SVR_EXT2,                            /* 252 */     /* Extension 2 */
    CSIM_SVR_SDN,                             /* 253 */     /* Service Dialing Numbers */
    CSIM_SVR_EXT3,                            /* 254 */     /* Extension 2 */
    CSIM_SVR_SMS,                             /* 255 */     /* Short Message Storage */
    CSIM_SVR_SMSP,                            /* 256 */     /* Short Message Parameters */
    CSIM_SVR_HRPD,                            /* 257 */     /* HRPD */
    CSIM_SVR_SCP_BCSMS,                       /* 258 */     /* Service Category Program for BC-SMS */
    CSIM_SVR_HOME_SPN,                        /* 259 */     /* CDMA Home Service Provider Name */
    CSIM_SVR_CBS_DOWNLOAD,                    /* 260 */     /* Data Download via SMS Broadcast */
    CSIM_SVR_SMSPP_DOWNLOAD,                  /* 261 */     /* Data Download via SMS PP */
    CSIM_SVR_CALL_CONTROL,                    /* 262 */     /* Call Control */
    CSIM_SVR_3GPD_SIP,                        /* 263 */     /* 3GPD_SIP */
    CSIM_SVR_3GPD_MIP,                        /* 264 */     /* 3GPD_MIP */
    CSIM_SVR_AKA,                             /* 265 */     /* AKA */
    CSIM_SVR_IP_LCS,                          /* 266 */     /* IP based Location Services */
    CSIM_SVR_BCMCS,                           /* 267 */     /* BCMCS */
    CSIM_SVR_MMS,                             /* 268 */     /* Multimedia Messaging Service */
    CSIM_SVR_EXT8,                            /* 269 */     /* Extension 8 */
    CSIM_SVR_MMS_UCP,                         /* 270 */     /* MMS User Connectivity Parameters */
    CSIM_SVR_APP_AUTH,                        /* 271 */     /* Application Authentication */
    CSIM_SVR_GID1,                            /* 272 */     /* Group Identifier Level 1 */
    CSIM_SVR_GID2,                            /* 273 */     /* Group Identifier Level 2 */
    CSIM_SVR_DPCK,                            /* 274 */     /* De-Personalization Control Keys */
    CSIM_SVR_CNL,                             /* 275 */     /* Cooperative Network List */
    CSIM_SVR_OCI,                             /* 276 */     /* Outgoing Call Information */
    CSIM_SVR_ICI,                             /* 277 */     /* Incoming Call Information */
    CSIM_SVR_EXT5,                            /* 278 */     /* Extension 5 */
    CSIM_SVR_MS,                              /* 279 */     /* Multimedia Stroage */
    CSIM_SVR_IMG,                             /* 280 */     /* Image EFIMG */
    CSIM_SVR_EST,                             /* 281 */     /* Enabled Services Table */
    CSIM_SVR_CCP,                             /* 282 */     /* Capability Configuration Parameters */
    CSIM_SVR_SF_EUIMID,                       /* 283 */     /* SF_EUIMID-based EUIMID */
    CSIM_SVR_3GPD_EXT,                        /* 284 */     /* Messaging and 3GPD Extensions */
    CSIM_SVR_ROOT_CA,                         /* 285 */     /* Root Certificates */
    CSIM_SVR_WAP_BROWSER,                     /* 286 */     /* WAP Browser */
    CSIM_SVR_JAVA,                            /* 287 */     /* Java */
    CSIM_SVR_RSV_CDG1,                        /* 288 */     /* Reserved for CDG */
    CSIM_SVR_RSV_CDG2,                        /* 289 */     /* Reserved for CDG */
    CSIM_SVR_IPV6,                            /* 290 */     /* IPV6 */
    CSIM_SVR_BUTT
};
typedef VOS_UINT32 UICC_ServicesTypeUint32;

enum USIMM_PbInitStatus {
    USIMM_PB_IDLE           = 0,
    USIMM_PB_INITIALING     = 1
};
typedef VOS_UINT16 USIMM_PbInitStatusUint16;

enum USIMM_RestRicCmd {
    USIMM_AUTHENTICATION                = 136,
    USIMM_SEARCH_RECORD                 = 162,
    USIMM_READ_BINARY                   = 176,
    USIMM_READ_RECORD                   = 178,
    USIMM_GET_RESPONSE                  = 192,
    USIMM_UPDATE_BINARY                 = 214,
    USIMM_UPDATE_RECORD                 = 220,
    USIMM_STATUS                        = 242,
    USIMM_CMD_BUTT
};
typedef VOS_UINT32 USIMM_RestricCmdUint32;

enum USIMM_PollFcp {
    USIMM_POLL_NO_NEED_FCP              = 0,
    USIMM_POLL_NEED_FCP                 = 1,
    USIMM_POLL_FCP_BUTT
};
typedef VOS_UINT32 USIMM_PollFcpUint32;

enum USIMM_FbdnHandle {
    USIMM_FDN_DEACTIVE              = 0,
    USIMM_FDN_ACTIVE                = 1,
    USIMM_BDN_DEACTIVE              = 2,
    USIMM_BDN_ACTIVE                = 3,
    USIMM_FBDN_BUTT
};
typedef VOS_UINT32 USIMM_FbdnHandleUint32;

enum USIMM_FdnStatus {
    USIMM_FDNSTATUS_OFF                 = USIMM_FDN_DEACTIVE,
    USIMM_FDNSTATUS_ON                  = USIMM_FDN_ACTIVE,
    USIMM_FDNSTATUS_BUTT
};
typedef VOS_UINT32 USIMM_FdnStatusUint32;

enum USIMM_BdnStatus {
    USIMM_BDNSTATUS_OFF                 = 0,
    USIMM_BDNSTATUS_ON                  = 1,
    USIMM_BDNSTATUS_BUTT
};
typedef VOS_UINT32 USIMM_BdnStatusUint32;

/* PIN码操作失败结果 */
enum USIMM_PinError {
    USIMM_SIM_PIN_REQUIRED              = 151,
    USIMM_SIM_PUK_REQUIRED              = 152,
    USIMM_SIM_FAILURE                   = 153,
    USIMM_SIM_BUSY                      = 154,
    USIMM_SIM_WRONG                     = 155,
    USIMM_INCORRECT_PASSWORD            = 156,
    USIMM_SIM_PIN2_REQUIRED             = 157,
    USIMM_SIM_PUK2_REQUIRED             = 158,
    USIMM_OPERATION_NOT_ALLOW           = 167,
    USIMM_PIN_ERROR_BUTT
};
typedef VOS_UINT32 USIMM_PinErrorUint32;

enum USIMM_AuthType {
    USIMM_3G_AUTH                       = 0x00000000,
    USIMM_2G_AUTH                       = 0x00000001,
    USIMM_IMS_AUTH                      = 0x00000002,
    USIMM_HTTP_AUTH                     = 0x00000003,
    USIMM_GBA_AUTH                      = 0x00000004,
    USIMM_NAF_AUTH                      = 0x00000005,
    USIMM_RUNCAVE_AUTH                  = 0x00000006,
    USIMM_EAP_AUTH                      = 0x00000007,
    USIMM_WLAM_AUTH                     = 0x00000008,
    USIMM_AUTH_BUTT
};
typedef VOS_UINT32 USIMM_AuthTypeUint32;

/*
 * 结构说明: Api_UsimAuthCnf ucResult宏定义
 */
enum USIMM_AuthResult {
    USIMM_AUTH_UMTS_SUCCESS         = 0x00000000,
    USIMM_AUTH_GSM_SUCCESS          = 0x00000001,
    USIMM_AUTH_IMS_SUCCESS          = 0x00000002,        /* 鉴权成功 */
    USIMM_AUTH_CDMA_SUCCESS         = 0x00000003,
    USIMM_AUTH_MAC_FAILURE          = 0x00000004,        /* MAC失败 */
    USIMM_AUTH_SYNC_FAILURE         = 0x00000005,        /* 重同步失败 */
    USIMM_AUTH_UMTS_OTHER_FAILURE   = 0x00000006,
    USIMM_AUTH_GSM_OTHER_FAILURE    = 0x00000007,
    USIMM_AUTH_IMS_OTHER_FAILURE    = 0x00000008,        /* 其他失败 */
    USIMM_AUTH_CDMA_OTHER_FAILURE   = 0x00000009,
    USIMM_AUTH_RESULT_BUTT
};
typedef VOS_UINT32 USIMM_AuthResultUint32;

enum USIMM_RefreshFileType {
    USIMM_REFRESH_ALL_FILE,
    USIMM_REFRESH_FILE_LIST,
    USIMM_REFRESH_3G_SESSION_RESET,
    USIMM_REFRESH_FILE_BUTT
};
typedef VOS_UINT16 USIMM_RefreshFileTypeUint16;

enum USIMM_EFRWFlag {
    USIMM_EF_RW_NOT     = 0,
    USIMM_EF_RW_OK      = 1,
    USIMM_EF_BUTT
};
typedef VOS_UINT8 USIMM_EFRWFlagUint8;

enum USIMM_EFStatus {
    USIMM_EFSTATUS_DEACTIVE     = 0,
    USIMM_EFSTATUS_ACTIVE       = 1,
    USIMM_EFSTATUS_BUTT
};
typedef VOS_UINT8 USIMM_EFStatusUint8;

/*
 * 结构说明: AID类型枚举值
 * 1. 日    期: 2013年12月19日
 *    修改内容:
 */
enum USIMM_AidType {
    USIMM_AID_TYPE_USIM                 = USIMM_GUTL_APP,    /* USIM的AID */
    USIMM_AID_TYPE_ISIM                 = USIMM_IMS_APP,     /* ISIM的AID */
    USIMM_AID_TYPE_CSIM                 = USIMM_CDMA_APP,    /* CSIM的AID */
    USIMM_AID_TYPE_BUTT
};
typedef VOS_UINT32 USIMM_AidTypeUint32;

/*
 * 结构说明: TERMINAL RESPONSE透传参数
 * 1. 日    期: 2014年02月15日
 *    修改内容:
 */
enum USIMM_TrPara {
    USIMM_TR_TYPE_BALONG    = 0,
    USIMM_TR_TYPE_CBP       = 0x5A5A5A5A,
    USIMM_TR_TYPE_BUTT
};
typedef VOS_UINT32 USIMM_TrPara32;

enum USIMM_RunCaveRandType {
    USIMM_RUNCAVE_RAND          = 0x00,
    USIMM_RUNCAVE_RANDU         = 0x01,
    USIMM_RUNCAVE_RANDBUTT      = 0x02,
    USIMM_RUNCAVE_RANDTYPE_BUTT
};
typedef VOS_UINT8 USIMM_RunCaveRandTypeUint8;

enum USIMM_ManagessdType {
    USIMM_MANAGESSD_UPDATE      = 0x00000000,
    USIMM_MANAGESSD_CONFIRM     = 0x00000001,
    USIMM_MANAGESSD_BUTT
};
typedef VOS_UINT32 USIMM_ManagessdTypeUint32;

/*
 * 结构说明: 临时定义区分不同的tpdu执行来源
 */
enum USIMM_SendPara {
    USIMM_CSIM_SENDPARA             = 1,
    USIMM_CGLA_SENDPARA             = 2,
    USIMM_ISDB_SENDPARA             = 3,
    USIMM_CL_SENDPARA               = 4,
    USIMM_SENDPARA_BUTT
};
typedef VOS_UINT32 USIMM_SendParaUint32;

/*
 * Description: This Enum defines the type of Computer IP Authentication
 *    command, that included by CDMA special authentication command set.  It maps
 *    the P1 parameter of this command.
 *    See section 4.8.1 in <Invalid document reference (C.S0023 D_v2.0)> and
 * "cdma2000 Application on UICC for
 *    Spread Spectrum Systems".
 */

enum USIMM_ComputeipAuthType {
    USIMM_COMPUTEIP_CHAP    = 0x00000000,
    USIMM_COMPUTEIP_MNHA    = 0x00000001,
    USIMM_COMPUTEIP_MIP     = 0x00000002,
    USIMM_COMPUTEIP_MNAAA   = 0x00000003,
    USIMM_COMPUTEIP_HRPD    = 0x00000004,
    USIMM_COMPUTEIP_BUTT
};
typedef VOS_UINT32 USIMM_ComputeipAuthUint32;

/*
 * Description: This Enum defines the type of parameter, of MN HA
 * Authenticator command, that is one of CDMA special authentication commands.
 * It maps P2 parameter of this command.
 * See section 4.8.1.2 in <Invalid document reference (C.S0023 D_v2.0)>.
 */

enum USIMM_SpecAuthBlock {
    USIMM_SPECAUTH_BLOCK_FIRST  = 0x00,
    USIMM_SPECAUTH_BLOCK_NEXT   = 0x01,
    USIMM_SPECAUTH_BLOCK_SINGLE = 0x02,
    USIMM_SPECAUTH_BLOCK_LAST   = 0x03,
    USIMM_SPECAUTH_BLOCK_BUTT
};
typedef VOS_UINT8 USIMM_SpecAuthBlockUint8;

/*
 * Description: This Enum defines the type of BCMSC command parameter. It
 *  maps the BCMCS command P1 parameter.
 *  See section 4.9 in <Invalid document reference (C.S0023 D_v2.0)> and
 * "cdma2000 Application on UICC for
 *  Spread Spectrum Systems".
 */

enum USIMM_BcmcsAuth {
    USIMM_BCMCS_RETRIEVESK      = 0x00000000,
    USIMM_BCMCS_UPDATABAK       = 0x00000001,
    USIMM_BCMCS_DELETEBAK       = 0x00000002,
    USIMM_BCMCS_RETRIEVE_SRTPSK = 0x00000003,
    USIMM_BCMCS_GENAUTH_SIGN    = 0x00000004,
    USIMM_BCMCS_AUTHENTICATION  = 0x00000005,
    USIMM_BCMCS_BUTT
};
typedef VOS_UINT32 USIMM_BcmcsAuthUint32;

/*
 * Description: This Enum defines the type of Application Authentication,
 * that belong to CDMA special authentication command set. It maps the
 * Application Authentication command data's first byte, that is
 * authentication mechanism directive.
 * See section 4.10.1 Table 10 in <Invalid document reference
 * (C.S0023 D_v2.0)>.
 */
enum USIMM_CdmaAppAuthType {
    USIMM_CDMAAPP_CRAMMD5       = 0x00000000,
    USIMM_CDMAAPP_HTTPMD5       = 0x00000001,
    USIMM_CDMAAPP_HTTPMD5SESS   = 0x00000002,
    USIMM_CDMAAPP_HTTPV1MD5     = 0x00000003,
    USIMM_CDMAAPP_HTTPV1MD5SESS = 0x00000004,
    USIMM_CDMAAPP_SASL          = 0x00000005,
    USIMM_CDMAAPP_SASLOTP       = 0x00000006,
    USIMM_CDMAAPP_SASLGSS       = 0x00000007,
    USIMM_CDMAAPP_BUTT
};
typedef VOS_UINT32 USIMM_CdmaAppAuthUint32;

/*
 * Description: This Enum defines the type of command, that process CDMA
 * special authentication request. There are several command component this
 * command set. So that this request need this input parameter to index which
 * command could be used.
 */
enum USIMM_CdmaSpecAuthType {
    USIMM_CDMASPEC_COMPUTEIP    = 0x00000000,
    USIMM_CDMASPEC_BCMCS        = 0x00000001,
    USIMM_CDMASPEC_APPAUTH      = 0x00000002,
    USIMM_CDMASPEC_BUTT
};
typedef VOS_UINT32 USIMM_CdmaSpecAuthTypeUint32;

enum USIMM_VsimState {
    USIMM_VSIM_STATE_OFF        = 0x0,          /* VSIM关闭 */
    USIMM_VSIM_STATE_ON         = 0x0000A5A5,   /* VSIM打开 */
    USIMM_VSIM_STATE_BUTT
};
typedef VOS_UINT32 USIMM_VsimStateUint32;

enum USIMM_RealisimState {
    USIMM_REALISIM_STATE_OFF     = 0x0,         /* USIM模拟ISIM */
    USIMM_REALISIM_STATE_ON      = 0x00005A5A,  /* 真实ISIM */
    USIMM_REALISIM_STATE_BUTT
};
typedef VOS_UINT32 USIMM_RealisimStateUint32;

/*
 * 结构说明: 卡状态通知类型枚举
 */
enum USIMM_CardStatusIndType{
    USIMM_CARD_STATUS_INIT_IND               = 0,
    USIMM_CARD_STATUS_NV_REFRESH_IND         = 1,
    USIMM_CARD_STATUS_SWITCH_CARD_IND        = 2,
    USIMM_CARD_STATUS_IND_BUTT
};
typedef VOS_UINT8 USIMM_CardStatusIndUint8;

enum USIMM_CardType {
    USIMM_CARD_SIM          = 0x00,
    USIMM_CARD_USIM         = 0x01,
    USIMM_CARD_ISIM         = 0x02,
    USIMM_CARD_UIM          = 0x03,
    USIMM_CARD_CSIM         = 0x04,
    USIMM_CARD_ROM_SIM      = 0xF0,
    USIMM_CARD_UNKNOWN      = 0xF1,
    USIMM_CARD_NOCARD       = 0xFF,
    USIMM_CARD_TYPE_BUTT
};
typedef VOS_UINT32 USIMM_CardTypeUint32;

/*
 * 枚举说明: 服务类型枚举
 */
enum USIMM_CardAppServic {
    USIMM_CARDAPP_SERVIC_ABSENT            = 0x00000000,
    USIMM_CARDAPP_SERVIC_UNAVAILABLE       = 0x00000001,
    USIMM_CARDAPP_SERVIC_SIM_PIN           = 0x00000002,
    USIMM_CARDAPP_SERVIC_SIM_PUK           = 0x00000003,
    USIMM_CARDAPP_SERVIC_NET_LCOK          = 0x00000004,
    USIMM_CARDAPP_SERVICE_SUBNET_LOCK      = 0x00000005,
    USIMM_CARDAPP_SERVICE_GID1_LOCK        = 0x00000006,
    USIMM_CARDAPP_SERVICE_GID2_LOCK        = 0x00000007,
    USIMM_CARDAPP_SERVICE_IMSI_LOCK        = 0x00000008,
    USIMM_CARDAPP_SERVIC_AVAILABLE         = 0x00000009,
    USIMM_CARDAPP_SERVIC_BUTT
};
typedef VOS_UINT32 USIMM_CardAppServicUint32;

/*
 * 枚举说明: 卡类型枚举
 */
enum USIMM_CardMediumType {
    USIMM_CARD_MEDIUM_REAL              = 0x0,     /* 实卡 */
    USIMM_CARD_MEDIUM_VSIM,                        /* 虚拟卡 */
    USIMM_CARD_MEDIUM_ROM,                         /* 快速开机模式 */
    USIMM_CARD_MEDIUM_ESIM,                        /* eSIM卡 */

    USIMM_CARD_MEDIUM_BUTT,
};
typedef VOS_UINT32 USIMM_CardMediumTypeUint32;

/*
 * 枚举说明: 卡Fetch Type枚举
 */
enum USIMM_CardFetchType {
    USIMM_CARD_FETCH_PPDOWN             = 0x01,     /* ppdown */
    USIMM_CARD_FETCH_WRITE_EFLOCI       = 0x02,     /* efloci */
    USIMM_CARD_FETCH_WRITE_EFPSLOCI     = 0x03,     /* efpsloci */
    USIMM_CARD_FETCH_WRITE_EFEPSLOCI    = 0x04,     /* efepsloci */

    USIMM_CARD_FETCH_TYPE_BUTT
};
typedef VOS_UINT32 USIMM_CardFetchTypeUint32;

enum SI_PIH_CardSlot {
    SI_PIH_CARD_SLOT_0      = 0, /* SCI_ID_0 */
    SI_PIH_CARD_SLOT_1      = 1, /* SCI_ID_1 */
    SI_PIH_CARD_SLOT_2      = 2, /* SCI_ID_2 */
    SI_PIH_CARD_SLOT_BUTT
};
typedef VOS_UINT32  SI_PIH_CardSlotUint32;

/*
 * 枚举说明: SIMSQ状态枚举变量
 */
enum USIMM_SimsqState {
    USIMM_SIMSQ_NOT_INSERTED            = 0,
    USIMM_SIMSQ_INSERTED                = 1,
    USIMM_SIMSQ_BUTT
};
typedef VOS_UINT32 USIMM_SimsqStateUint32;

/*
 * 结构说明: ICC切模错误码枚举
 */
enum USIMM_ChangeModeError {
    USIMM_CHANGE_MODE_SUCC                  = 0,
    USIMM_CHANGE_MODE_APP_TYPE_ERROR        = 1, /* 期望切换APP类型错误 */
    USIMM_CHANGE_MODE_NOT_ICC               = 2, /* CCT国漫切换需只有ICC卡才需要USIM模块重启卡 */
    USIMM_CHANGE_MODE_NO_CWG                = 3, /* 非双模卡 */
    USIMM_CHANGE_MODE_UNAVAILABLE           = 4, /* 当前卡复位不可用 */
    USIMM_CHANGE_MODE_SET_PRF_APP_FAIL      = 5, /* 设置应用优先级顺序失败 */
    USIMM_CHANGE_MODE_SAME_APP              = 6, /* 需要切换的就是当前应用 */
    USIMM_CHANGE_MODE_FEATURE_NOT_SUPPORT   = 7, /* 特性不支持:切卡只有CDMA特性宏打开才支持 */

    USIMM_CHANGE_MODE_BUTT
};
typedef VOS_UINT32 USIMM_ChangeModeErrorUint32;

typedef struct {
    VOS_UINT8                   pin1RemainTime; /* PIN1码剩余次数      */
    VOS_UINT8                   pin2RemainTime; /* PIN2码剩余次数      */
    VOS_UINT8                   puk1RemainTime; /* PUK1码剩余次数     */
    VOS_UINT8                   puk2RemainTime; /* PUK2码剩余次数     */
    USIMM_PinEnableStatusUint32 pin1Enable;     /* PIN1激活状态        */
    USIMM_PinEnableStatusUint32 pin2Enable;     /* PIN2激活状态        */
} USIMM_PinInfo;

typedef struct {
    VOS_UINT8 commandNum;
    VOS_UINT8 commandType;
    VOS_UINT8 commandQua;
    VOS_UINT8 rsv;
} USIMM_StkCommandDetails;

typedef struct {
    VOS_UINT8 pin1RemainTime; /* PIN1码剩余次数 */
    VOS_UINT8 pin2RemainTime; /* PIN2码剩余次数 */
    VOS_UINT8 puk1RemainTime; /* PUK1码剩余次数 */
    VOS_UINT8 puk2RemainTime; /* PUK2码剩余次数 */
} USIMM_PinRemain;

typedef struct {
    VOS_UINT32 dataLen;
    VOS_UINT8 *data;
} USIMM_U8LvData;

typedef struct {
    VOS_UINT8 dataLen;
    VOS_UINT8 data[USIMM_APDU_DATA_MAXLEN];
} USIMM_U8Data;

typedef struct {
    VOS_UINT32  dataLen;
    VOS_UINT16 *data;
} USIMM_U16LvData;

typedef struct {
    VOS_UINT32 pathLen;
    VOS_CHAR   path[USIMM_PATHSTR_MAX_LEN];
} USIMM_FilePathInfo;

/* 存储符合电话本的相关信息 */
typedef struct {
    USIMM_FilePathInfo filePath;
    VOS_UINT16         dataLen;
    VOS_UINT16         rsv;
    VOS_UINT8          recordNum;
    VOS_UINT8          content[255];
} USIMM_MutilFileData;

/*
 * 结构说明: STK发给USIMM模块的RESET消息中RESET信息字段结构
 */
typedef struct {
    VOS_UINT16 fileListLen;            /* FILE列表长度 */
    VOS_UINT8  fileList[256];          /* FILE列表内容 */
    VOS_UINT16 aidLen;                 /* AID长度 */
    VOS_UINT8  aid[USIMM_AID_LEN_MAX]; /* AID内容 */
} USIMM_ResetInfo;

/*
 * 结构说明: 2G鉴权数据
 */
typedef struct {
    USIMM_U8Data rand;
} USIMM_2GAuth;

/*
 * 结构说明: 3G鉴权数据
 */
typedef struct {
    USIMM_U8Data rand;
    USIMM_U8Data autn;
} USIMM_3GAuth;

/*
 * 结构说明: GBA鉴权数据
 */
typedef struct {
    USIMM_U8Data rand;
    USIMM_U8Data autn;
} USIMM_GbaAuth;

/*
 * 结构说明: NAF鉴权数据
 */
typedef struct {
    USIMM_U8Data nafId;
    USIMM_U8Data impi;
} USIMM_NafAuth;

/*
 * 结构说明: HTTP鉴权数据
 */
typedef struct {
    USIMM_U8Data realm;
    USIMM_U8Data nonce;
    USIMM_U8Data cnonce;
} USIMM_HttpAuth;

/*
 * 结构说明: IMS鉴权数据
 */
typedef struct {
    USIMM_U8Data rand;
    USIMM_U8Data autn;
} USIMM_ImsAuth;

/*
 * 结构说明: RUN CAVE鉴权数据
 */
typedef struct {
    USIMM_RunCaveRandTypeUint8 randType;
    VOS_UINT8                  digLen;
    VOS_UINT8                  processCtrl;
    VOS_UINT8                  rand[USIMM_RUNCAVE_RAND_LEN];
    VOS_UINT8                  digits[USIMM_RUNCAVE_DIGLEN];
    VOS_UINT8                  esn[USIMM_ESN_MEID_LEN];
    VOS_UINT8                  rsv[3];
} USIMM_RunCaveAuth;

typedef struct {
    USIMM_U8Data rand;
    USIMM_U8Data autn;
} USIMM_EapAuth;

typedef struct {
    USIMM_U8Data rand;
    USIMM_U8Data autn;
} USIMM_WlanAuth;

typedef union {
    USIMM_3GAuth      usim3GAuth;
    USIMM_2GAuth      usim2GAuth;
    USIMM_ImsAuth     imsAuth;
    USIMM_HttpAuth    httpAuth;
    USIMM_GbaAuth     gbaAuth;
    USIMM_NafAuth     nafAuth;
    USIMM_RunCaveAuth runCaveAuth;
    USIMM_EapAuth     eapAuth;
    USIMM_WlanAuth    wlanAuth;
} USIMM_AuthDataBlock;

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
typedef struct {
    VOS_UINT8 authBS[USIMM_CDMA_AUTHBS_LEN];
    VOS_UINT8 rsv;
} USIMM_ConfirmSSDData;

typedef struct {
    VOS_UINT8 randSSD[USIMM_CDMA_RANDSSD_LEN];
    VOS_UINT8 processCtrl;
    VOS_UINT8 esn[USIMM_ESN_MEID_LEN];
    VOS_UINT8 rsv;
} USIMM_UpdateSSDData;

typedef union {
    USIMM_ConfirmSSDData confirmSSD;
    USIMM_UpdateSSDData  updateSSD;
} USIMM_ManageSSDDataBlock;

typedef struct {
    USIMM_ManagessdTypeUint32  cmdType;
    USIMM_ManageSSDDataBlock ssdData;
} USIMM_ManageSSDData;
#endif
/*
 * 结构说明: USIMM REFRESH主动上报的文件信息
 */
typedef struct {
    USIMM_DefFileidUint16 fileId;                                /* 转换后的文件ID */
    VOS_UINT16            pathLen;                               /* 文件路径长度 */
    VOS_UINT8             path[USIMM_REFRESH_FILE_MAX_PATH_LEN]; /* 文件全路径，字符串类型 */
    USIMM_CardAppUint32   appType;                               /* APP类型 */
} USIM_REFRESH_File;

typedef struct {
    VOS_UINT8 ik[USIMM_AUTH_IKSPACELEN];       /* Len+Data */
    VOS_UINT8 ck[USIMM_AUTH_CKSPACELEN];       /* Len+Data */
    VOS_UINT8 gsmKc[USIMM_AUTH_KCSPACELEN];    /* Len+Data */
    VOS_UINT8 auts[USIMM_AUTH_AUTHSPACELEN];   /* Len+Data */
    VOS_UINT8 authRes[USIMM_AUTH_RESSPACELEN]; /* Len+Data */
    VOS_UINT8 rsv[5];
} USIMM_TelecomAuthCnfData;

typedef struct {
    VOS_UINT8 ik[USIMM_AUTH_IKSPACELEN];       /* Len+Data */
    VOS_UINT8 ck[USIMM_AUTH_CKSPACELEN];       /* Len+Data */
    VOS_UINT8 auts[USIMM_AUTH_AUTHSPACELEN];   /* Len+Data */
    VOS_UINT8 authRes[USIMM_AUTH_RESSPACELEN]; /* Len+Data */
    VOS_UINT8 response[255];                   /* Len+Data */
    VOS_UINT8 sessionKey[255];                 /* Len+Data */
    VOS_UINT8 ksExtNaf[6];                     /* Len+Data */
    VOS_UINT8 rsv[2];
} USIMM_ImsAuthCnfData;

typedef struct {
    VOS_UINT8 authr[USIMM_AUTH_ATUHRSPACELEN]; /* Data */
    VOS_UINT8 rsv;
} USIMM_RunCaveAuthCnf;

typedef union {
    USIMM_TelecomAuthCnfData teleCnf;
    USIMM_ImsAuthCnfData     imsCnf;
    USIMM_RunCaveAuthCnf     runCaveCnf;
} USIMM_AutnCnfDataBlock;

typedef struct {
    VOS_UINT8  tpduHead[USIMM_TPDU_HEAD_LEN];
    VOS_UINT8  rsv;
    VOS_UINT16 dataLen;
    VOS_UINT8  data[USIMM_TPDU_DATA_LEN_MAX];
} USIMM_TpduData;

typedef struct {
    VOS_UINT8  inputCmd[USIMM_TPDU_HEAD_LEN];
    VOS_UINT8  sw1;
    VOS_UINT8  sw2;
    VOS_UINT8  rsv;
    VOS_UINT32 dataLen;
    VOS_UINT8  rspData[USIMM_TPDU_DATA_LEN_MAX];
} USIMM_TpduRspData;

typedef struct {
    VOS_UINT8  inputCmd[USIMM_TPDU_HEAD_LEN];
    VOS_UINT8  sw1;
    VOS_UINT8  sw2;
    VOS_UINT8  lastDataFlag;
    VOS_UINT32 dataLen;
    VOS_UINT8  rspData[USIMM_PRIVATECGLA_DATA_LEN_MAX];
} USIMM_CglaRspData;

typedef struct {
    VOS_MSG_HEADER
    USIMM_CmdTypeReqUint32 msgName;
    USIMM_CardAppUint32    appType;
    VOS_UINT32             sendPara;
} USIMM_CmdHeaderReq;

#if (FEATURE_ASSOCIATED_CARD_LOCK == FEATURE_ON)
/*
 * 结构说明: 卡是否在位请求消息结构
 */
typedef struct {
    USIMM_CmdHeaderReq msgHeader;
}USIMM_CardExistQueryReq;
#endif

typedef struct {
    USIMM_CmdHeaderReq msgHeader;
} USIMM_ActiveCardReq;

typedef struct {
    USIMM_CmdHeaderReq msgHeader;
} USIMM_InitContinueReq;

/*
 * 结构说明: Store MEID/PESN消息处理结构
 */
typedef struct {
    USIMM_CmdHeaderReq msgHeader;
} USIMM_QueryEsnMeidRsltReq;

typedef struct {
    USIMM_CmdHeaderReq msgHeader;
} USIMM_QueryVoltageReq;

/*
 * Description: This structure defines the TLV data format. It could be used
 * for some command's data.
 */
typedef struct {
    VOS_UINT8 tag;
    VOS_UINT8 len;
    VOS_UINT8 data[USIMM_APDU_DATA_MAXLEN];
    VOS_UINT8 rsv[3];
} USIMM_TlvData;

/*
 * Description: This structure defines the input data format of CHAP command
 * type, See section 4.8.1.1 in <Invalid document reference (C.S0023 D_v2.0)>
 * and section 9.4.4.1 in 3GPP2 C.S0065 B v2.0: "cdma2000 Application on UICC
 * for Spread Spectrum Systems".
 */
typedef struct {
    VOS_UINT8    chapId;
    VOS_UINT8    naiEnterIndex;
    VOS_UINT8    rsv[2];
    USIMM_U8Data chapChallenge;
} USIMM_ComputeipChap;

/*
 * Description: This structure defines the input data format of MN HA
 * Authenticator command type. See section 4.8.1.2 in <Invalid document
 * reference (C.S0023 D_v2.0)> and section 9.4.4.1 in 3GPP2 C.S0065 B v2.0:
 * "cdma2000 Application on UICC for Spread Spectrum Systems".
 */
typedef struct {
    USIMM_SpecAuthBlockUint8 blockType;
    VOS_UINT8                naiEnterIndex;
    VOS_UINT8                rsv[2];
    USIMM_U8Data             chapChallenge;
} USIMM_ComputeipMnha;

/*
 * Description: This structure defines the input data format of MIP RRQ Hash
 * command type. See section 4.8.1.3 in <Invalid document reference
 * (C.S0023 D_v2.0)> and section 9.4.4.1 in 3GPP2 C.S0065 B v2.0: "cdma2000
 * Application on UICC for Spread Spectrum Systems".
 */
typedef struct {
    USIMM_SpecAuthBlockUint8 blockType;
    VOS_UINT8                rsv[3];
    USIMM_U8Data             miprrqData;
    USIMM_U8Data             mnaaaExHeader;
} USIMM_ComputeipMiprrqhash;

/*
 * Description: This structure defines the input data format of MN AAA
 * Authenticator command type. See section 4.8.1.4 in <Invalid document
 * reference (C.S0023 D_v2.0)> and section 9.4.4.1 in 3GPP2 C.S0065 B v2.0:
 * "cdma2000 Application on UICC for Spread Spectrum Systems".
 */
typedef struct {
    VOS_UINT8    naiEnterIndex;
    VOS_UINT8    rsv[3];
    USIMM_U8Data challenge;
} USIMM_ComputeipMnaaa;

/*
 * Description: This structure defines the input data format of MN AAA
 * Authenticator command type. See section 4.8.1.4 in <Invalid document
 * reference (C.S0023 D_v2.0)> and section 9.4.4.1 in 3GPP2 C.S0065 B v2.0:
 * "cdma2000 Application on UICC for Spread Spectrum Systems".
 */
typedef struct {
    VOS_UINT8    chapid;
    VOS_UINT8    rsv[3];
    USIMM_U8Data chapChallenge;
} USIMM_ComputeipHrpd;

/*
 * Description: This union defines the data container in
 * USIMM_ComputeipAuth.
 *    chapData: Input data format of CHAP command type, See section 4.8.1.1
 *    in <Invalid document reference (C.S0023 D_v2.0)> and section 9.4.4.1
 *    3GPP2 C.S0065 B v2.0: "cdma2000 Application on UICC for Spread Spectrum
 *    Systems";
 *    mnhaData: Input data format of MN HA Authenticator command type. See
 *    section 4.8.1.2 in <Invalid document reference (C.S0023 D_v2.0)> and
 *    section 9.4.4.1 3GPP2 C.S0065 B v2.0: "cdma2000 Application on UICC for
 *    Spread Spectrum Systems";
 *    mipData: Input data format of MIP RRQ Hash command type. See section
 *    4.8.1.3 in <Invalid document reference (C.S0023 D_v2.0)> and section
 *    9.4.4.1 3GPP2 C.S0065 B v2.0: "cdma2000 Application on UICC for Spread
 *    Spectrum Systems";
 *    mnaaaData: Input data format of MN AAA Authenticator command type. See
 *    section 4.8.1.4 in <Invalid document reference (C.S0023 D_v2.0)> and
 *    section 9.4.4.1 3GPP2 C.S0065 B v2.0: "cdma2000 Application on UICC for
 *    Spread Spectrum Systems";
 *    hrpdData: Input data format of HRPD Access Authentication command
 *    type. See section 4.8.1.5 in <Invalid document reference
 *    (C.S0023 D_v2.0)> and section 9.4.4.1 3GPP2 C.S0065 B v2.0: "cdma2000
 *    Application on UICC for Spread Spectrum Systems";
 */
typedef union {
    USIMM_ComputeipChap       chapData;
    USIMM_ComputeipMnha       mnhaData;
    USIMM_ComputeipMiprrqhash mipData;
    USIMM_ComputeipMnaaa      mnaaaData;
    USIMM_ComputeipHrpd       hrpdData;
} USIMM_ComputeipDataBlock;

/*
 * Description: This structure defines the input data format, that used for
 * Compute IP command, which belong to CDMA special authentication command set.
 * See section 4.8.1 in <Invalid document reference (C.S0023 D_v2.0)> and
 * section 9.4.4.1 in 3GPP2 C.S0065 B v2.0: "cdma2000 Application on UICC for
 * Spread Spectrum Systems".
 *    computeIPType: The command type that used to process the Compute IP
 *    request;
 *    computeIPData: The input data of Compute IP command;
 */
typedef struct {
    USIMM_ComputeipAuthUint32 computeIPType;
    USIMM_ComputeipDataBlock  computeIPData;
} USIMM_ComputeipAuth;

/*
 * Description: This structure defines the command RETRIEVE SK input data
 * format, See section 4.9.1 in <Invalid document reference (C.S0023 D_v2.0)>.
 * The tag could see Annex B in <Invalid document reference (C.S0023 D_v2.0)>,
 * and it defines by
 * USIMM_BCMCS_TAG_ENUM_UINT8 type.
 */
typedef struct {
    USIMM_TlvData bcmcsFlowId;
    USIMM_TlvData bakId;
    USIMM_TlvData skRand;
} USIMM_BcmcsRetrievesk;

/*
 * Description: This structure defines the command Update BAK input data
 * format, See section 4.9.2 in <Invalid document reference (C.S0023 D_v2.0)>.
 * The tag could see Annex B in <Invalid document reference (C.S0023 D_v2.0)>,
 * and it defines by
 * USIMM_BCMCS_TAG_ENUM_UINT8 type.
 */
typedef struct {
    USIMM_TlvData bcmcsFlowId;
    USIMM_TlvData bakId;
    USIMM_TlvData bakExprie;
    USIMM_TlvData tkRand;
    USIMM_U8Data  encryptedBak;
} USIMM_BcmcsUpdatabak;

/*
 * Description: This structure defines the command Delete BAK input data
 * format, See section 4.9.3 in <Invalid document reference (C.S0023 D_v2.0)>.
 * The tag could see Annex B in <Invalid document reference (C.S0023 D_v2.0)>,
 * and it defines by
 * USIMM_BCMCS_TAG_ENUM_UINT8 type.
 */
typedef struct {
    USIMM_TlvData bcmcsFlowId;
    USIMM_TlvData bakId;
} USIMM_BcmcsDeletebak;

/*
 * Description: This structure defines the command Delete BAK input data
 * format, See section 4.9.3 in <Invalid document reference (C.S0023 D_v2.0)>.
 * The tag could see Annex B in <Invalid document reference (C.S0023 D_v2.0)>,
 * and it defines by
 * USIMM_BCMCS_TAG_ENUM_UINT8 type.
 */
typedef struct {
    USIMM_TlvData bcmcsFlowId;
    USIMM_TlvData bakId;
    USIMM_TlvData skRand;
    USIMM_TlvData packetIndex;
} USIMM_BcmcsRetrieveSrtpsk;

/*
 * Description: This structure defines the command Generate Authorization
 * Signature input data format, See section 4.9.5 in <Invalid document
 * reference (C.S0023 D_v2.0)>.
 * The tag could see Annex B in <Invalid document reference (C.S0023 D_v2.0)>,
 * and it defines by
 * USIMM_BCMCS_TAG_ENUM_UINT8 type.
 */
typedef struct {
    USIMM_TlvData bcmcsFlowId;
    USIMM_TlvData bakId;
    USIMM_TlvData timestamp;
} USIMM_BcmcsGenauthSign;

/*
 * Description: This structure defines the command data that input by BCMCS
 * Authentication command.
 * See section 4.9.6 in <Invalid document reference (C.S0023 D_v2.0)> and
 * section 9.4.5 in 3GPP2 C.S0065 B v2.0: "cdma2000 Application on UICC for
 * Spread Spectrum Systems".
 *    rand: TLV Data structure;
 *    challenge: TLV Data structure;
 * The tag could see Annex B in <Invalid document reference (C.S0023 D_v2.0)>.
 */
typedef struct {
    USIMM_TlvData rand;
    USIMM_TlvData challenge;
} USIMM_BcmcsAuthentication;

/*
 * Description: This union define the data container in the
 * USIMM_BcmcsData.
 *    retrieveSK: Command RETRIEVE SK input data format, See section 4.9.1
 *    in <Invalid document reference (C.S0023 D_v2.0)>;
 *    updateBAK: Command Update BAK input data format, See section 4.9.2 in
 *    <Invalid document reference (C.S0023 D_v2.0)>;
 *    deleteBAK: Command Delete BAK input data format, See section 4.9.3 in
 *    <Invalid document reference (C.S0023 D_v2.0)>;
 *    stRESRTPSK: Command Retrieve SRTP SK input data format, See section
 *    4.9.4 in <Invalid document reference (C.S0023 D_v2.0)>;
 *    genAuthSign: Command Generate Authorization Signature input data
 *    format, See section 4.9.5 in <Invalid document reference
 *    (C.S0023 D_v2.0)>;
 *    authentication: Command BCMCS Authentication input data format, See
 *    section 4.9.6 in <Invalid document reference (C.S0023 D_v2.0)>
 */
typedef union {
    USIMM_BcmcsRetrievesk     retrieveSK;
    USIMM_BcmcsUpdatabak      updateBAK;
    USIMM_BcmcsDeletebak      deleteBAK;
    USIMM_BcmcsRetrieveSrtpsk reSRTPSK;
    USIMM_BcmcsGenauthSign    genAuthSign;
    USIMM_BcmcsAuthentication authentication;
} USIMM_BcmcsDataBlock;

/*
 * Description: This structure defines all of BCMCS command input data format.
 * bcmcsType: The command type;
 * ulServiceType: the value '01' is corresponding to "3GPP2 BCMCS", other
 * value is RFU;
 * uBCMCSData: BCMCS input data;
 */
typedef struct {
    USIMM_BcmcsAuthUint32 bcmcsType;
    VOS_UINT32            serviceType;
    USIMM_BcmcsDataBlock  bcmcsData;
} USIMM_BcmcsData;

/*
 * Description: This structure defines the input data that need by processing
 * the Application Authentication command. See section 4.10.1 in <Invalid
 * document reference (C.S0023 D_v2.0)> and section 9.4.6.1 in 3GPP2 C.S0065 B
 * v2.0: "cdma2000 Application on UICC for Spread Spectrum Systems".
 */
typedef struct {
    USIMM_CdmaAppAuthUint32 authMech;
    VOS_UINT32              appId;
    USIMM_U8Data            realm;
    USIMM_U8Data            serviceNonce;
    USIMM_U8Data            clientNonce;
} USIMM_CdmaAppAuth;

/*
 * Description: This union defines the data container in the
 * USIMM_CDMASPEC_AUTH_STRU.
 *    computeIP: Command Compute IP's input data;
 *    bcmcs: Command BCMCS's input data;
 *    stOTAAuth: Command Application Authentication's input data;
 */
typedef union {
    USIMM_ComputeipAuth computeIP;
    USIMM_BcmcsData     bcmcs;
    USIMM_CdmaAppAuth   appAuth;
} USIMM_CdmaSpecAuthBlock;

/*
 * 结构说明: SIM卡保护性复位请求消息结构
 */
typedef struct {
    USIMM_CmdHeaderReq msgHeader;
} USIMM_ProtectResetReq;

/*
 * 结构说明: SIM卡去激活请求消息结构
 */
typedef struct {
    USIMM_CmdHeaderReq msgHeader;
} USIMM_DeactiveCardReq;

/*
 * 结构说明: SIM卡鉴权请求消息结构
 */
typedef struct {
    USIMM_CmdHeaderReq   msgHeader;
    USIMM_AuthTypeUint32 authType;
    USIMM_AuthDataBlock  auth;
} USIMM_AuthenticationReq;

/*
 * 结构说明: SIM卡PIN码请求消息结构
 */
typedef struct {
    USIMM_CmdHeaderReq     msgHeader;
    USIMM_PinCmdTypeUint32 cmdType;
    USIMM_PinTypeUint32    pinType;
    VOS_UINT8              oldPin[USIMM_PINNUMBER_LEN];
    VOS_UINT8              newPin[USIMM_PINNUMBER_LEN];
} USIMM_PinHandleReq;

typedef struct {
    USIMM_CmdHeaderReq msgHeader;
    USIMM_FilePathInfo filePath;
    VOS_UINT16         rsv;
    VOS_UINT16         dataLen;
    VOS_UINT8          recordNum;
    VOS_UINT8          content[3];
} USIMM_UpdateFileReq;

/*
 * 结构说明: 带imsi检查的卡文件更新请求消息结构
 */
typedef struct {
    USIMM_CmdHeaderReq msgHeader;
    VOS_UINT8          imsi[USIMM_EF_IMSI_LEN];
    VOS_UINT8          rsv1[3];
    USIMM_FilePathInfo filePath;
    VOS_UINT16         rsv2;
    VOS_UINT16         dataLen;
    VOS_UINT8          recordNum;
    VOS_UINT8          content[3];
} USIMM_UpdateFileWithImsiReq;

typedef struct {
    USIMM_CmdHeaderReq msgHeader;
    USIMM_FilePathInfo filePath;
    VOS_BOOL           needReadCache;
    VOS_UINT16         rsv1;
    VOS_UINT8          recordNum;
    VOS_UINT8          rsv2;
} USIMM_ReadFileReq;

typedef struct {
    USIMM_CmdHeaderReq msgHeader;
    USIMM_FilePathInfo filePath;
} USIMM_QueryFileReq;

/*
 * 结构说明: SIM卡周期性轮询请求消息结构
 */
typedef struct {
    USIMM_CmdHeaderReq  msgHeader;
    USIMM_FilePathInfo  filePath;
    USIMM_PollFcpUint32 needFcp;
} USIMM_StatusCmdReq;

typedef struct {
    USIMM_CmdHeaderReq msgHeader;
    USIMM_FilePathInfo filePath;
    VOS_UINT32         dataLen;
    VOS_UINT8          data[4];
} USIMM_SearchFileReq;

/*
 * 结构说明: FDN激活去激活请求消息结构
 */
typedef struct {
    USIMM_CmdHeaderReq     msgHeader;
    USIMM_FbdnHandleUint32 handleType;
    VOS_UINT8              pin[USIMM_PINNUMBER_LEN];
} USIMM_FdnProcessReq;

/*
 * 结构说明: USIM模块CHANNEL INFO的数据结构
 */
typedef struct {
    VOS_UINT32 sessionId;                  /* 会话id */
    VOS_UINT32 chanNum;                    /* 通道号 */
    VOS_UINT32 aidLen;                     /* AID长度 */
    VOS_UINT8  adfName[USIMM_AID_LEN_MAX]; /* AID的内容 */
    VOS_UINT8  apdup2;
    VOS_UINT8  rsv[3];
} USIMM_ChannelInfo;

/*
 * 结构说明: 逻辑通道打开请求消息结构
 */
typedef struct {
    USIMM_CmdHeaderReq msgHeader;
    USIMM_ChannelInfo  channelInfo;
} USIMM_OpenChannelReq;

/*
 * 结构说明: 逻辑通道关闭请求消息结构
 */
typedef struct {
    USIMM_CmdHeaderReq msgHeader;
    VOS_UINT32         sessionId; /* 会话ID */
} USIMM_CloseChannelReq;

typedef struct {
    USIMM_CmdHeaderReq msgHeader;
    USIMM_FilePathInfo filePath;
    USIMM_TpduData     tpduData;
} USIMM_SendTpduDataReq;

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
/*
 * 结构说明: BASE STATION CHALLENGE请求消息结构
 */
typedef struct {
    USIMM_CmdHeaderReq msgHeader;
    VOS_UINT8          data[USIMM_AUTH_BSCSPACELEN];
} USIMM_BschallengeReq;

/*
 * 结构说明: Generate KEY VMP请求消息结构
 */
typedef struct {
    USIMM_CmdHeaderReq msgHeader;
    VOS_UINT8          firstOctet;
    VOS_UINT8          lastOctet;
    VOS_UINT8          rsv[2];
} USIMM_GenerateKeyvpmReq;

/*
 * 结构说明: Manage SSD请求消息结构
 */
typedef struct {
    USIMM_CmdHeaderReq  msgHeader;
    USIMM_ManageSSDData ssdData;
} USIMM_ManagessdReq;
#endif

/*
 * 结构说明: TERMINAL RESPONSE请求消息结构
 */
typedef struct {
    USIMM_CmdHeaderReq msgHeader;
    VOS_UINT8          dataLen;
    VOS_UINT8          content[3];
} USIMM_TerminalResponseReq;

/*
 * 结构说明: 电话本初始化状态指示消息结构
 */
typedef struct {
    USIMM_CmdHeaderReq       msgHeader;
    USIMM_PbInitStatusUint16 pbInitStatus;
    VOS_UINT16               rsv;
} USIMM_PbinitStatusInd;

/*
 * 结构说明: REFRESH请求消息结构
 */
typedef struct {
    USIMM_CmdHeaderReq      msgHeader;
    VOS_UINT32              commandNum;
    VOS_UINT32              commandType;
    USIMM_RefreshTypeUint32 refreshType;
    VOS_UINT32              aidLen;
    VOS_UINT8               aid[USIMM_AID_LEN_MAX];
    VOS_UINT16              len;
    VOS_UINT8               fileList[6];
} USIMM_StkrefreshReq;

/*
 * 结构说明: ENVELOPE请求消息结构
 */
typedef struct {
    USIMM_CmdHeaderReq msgHeader;
    VOS_PID            opReturnPid;
    VOS_UINT8          dataLen;
    VOS_UINT8          content[3];
} USIMM_STKEnvelopeReq;

/*
 * 结构说明: 受限制SIM卡访问请求消息结构
 */
typedef struct {
    USIMM_CmdHeaderReq     msgHeader;
    USIMM_RestricCmdUint32 cmdType;
    VOS_UINT16             efId;
    VOS_UINT16             pathLen;
    VOS_UINT8              p1;
    VOS_UINT8              p2;
    VOS_UINT8              p3;
    VOS_UINT8              rsv1;
    VOS_UINT16             path[USIMM_MAX_PATH_LEN];
    VOS_UINT8              content[USIMM_T0_APDU_MAX_LEN + 1];
    VOS_UINT8              rsv2[3];
} USIMM_RaccessReq;

typedef struct {
    USIMM_CmdHeaderReq  msgHeader;
    VOS_UINT32          fileCount;
    USIMM_MutilFileData fileData[USIMM_SETMUTILFILE_MAX]; /* 最多14个文件 */
} USIMM_SetMutilFileReq;

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
typedef struct {
    USIMM_CmdHeaderReq           msgHeader;
    USIMM_CdmaSpecAuthTypeUint32 authType;
    USIMM_CdmaSpecAuthBlock      authData;
} USIMM_CdmaSpecAuthReq;
#endif

#if (FEATURE_VSIM == FEATURE_ON)
typedef struct {
    USIMM_CmdHeaderReq      msgHeader;
    USIMM_PhyCardTypeUint32 cardType;
    VOS_UINT32              authType;
    VOS_UINT32              aesKeylen;
    VOS_UINT8               aeskey[32];
    VOS_UINT32              dataLen;    /* FILE DATA 长度 */
    VOS_UINT8               content[4]; /* FILE DATA 内容 */
} USIMM_VsimFileDataWriteReq;
#endif

/*
 * 1. 日    期: 2017年7月24日
 * 修改内容: 多实例修改
 */
typedef struct {
    USIMM_CmdHeaderReq    msgHeader; /* PIH消息头 */
    SI_PIH_CardSlotUint32 card0Slot;
    SI_PIH_CardSlotUint32 card1Slot;
    SI_PIH_CardSlotUint32 card2Slot;
} USIMM_SciCfgSetReq;

/*
 * 结构说明: 卡应用状态信息
 */
typedef struct {
    USIMM_CardAppServicUint32 cardAppService;
    VOS_UINT32                isTestCard;                   /* VOS_TRUE为测试卡，VOS_FALSE为非测试卡 */
    VOS_UINT16                isImsiValid;                  /* VOS_TRUE为有效，VOS_FALSE为无效 */
    VOS_UINT8                 imsi[USIMM_EF_IMSI_COMM_LEN]; /* UsimSim应用为IMSI，CsimUim为IMSI_M */
    VOS_UINT8                 gid1Len;                       /* 长度为0无效(服务状态位无效，读取失败填0)，其它值有效，最长16个字节 */
    VOS_UINT8                 gid1[USIMM_EF_GID1_LEN];      /* UsimSim应用为6F3E，CsimUim为6F6D */
    VOS_UINT8                 gid2Len;                       /* 长度为0无效(服务状态位无效，读取失败填0)，其它值有效，最长16个字节 */
    VOS_UINT8                 gid2[USIMM_EF_GID2_LEN];      /* UsimSim应用为6F3F，CsimUim为6F6E */
    VOS_UINT8                 efadMncLen;                    /* USIM应用中EFAD文件中指示IMSI中MNC长度，注：按照协议只有USIM应用对应EFAD文件内容第三字节指示MNC长度，其他应用没有 */
    VOS_UINT8                 rsv;
} USIMM_CardAppStatus;

/*
 * 结构说明: USIMM 主动上报的卡状态附加信息，卡状态有效的时候才有意义
 */
typedef struct {
    USIMM_CardMediumTypeUint32 mediumType; /* 卡物理存储介质 */
    USIMM_RealisimStateUint32  isimState;
    VOS_UINT8                  hasCModule;
    VOS_UINT8                  hasWGModule;
    VOS_UINT8                  iccid[USIMM_ICCID_FILE_LEN];
    VOS_UINT8                  isICCIDValid;
    USIMM_CardStatusIndUint8   statusType;
    VOS_UINT8                  rev[2];
} USIMM_CardStatusAddinfo;

/* 卡状态消息和RIL有耦合，该结构变动需知会RIL */
typedef struct {
    VOS_MSG_HEADER
    USIMM_CmdTypeCnfUint32  msgName;
    USIMM_PhyCardTypeUint32 phyCardType; /* 物理卡状态 */
    USIMM_CardAppStatus     usimSimInfo; /* GUL SIM状态 */
    USIMM_CardAppStatus     csimUimInfo; /* CDMA SIM状态 */
    USIMM_CardAppStatus     isimInfo;    /* ISIM状态 */
    USIMM_CardStatusAddinfo addInfo;     /* 卡状态有效时候才能使用里面的信息 */
} USIMM_CardStatusInd;

#if (FEATURE_ASSOCIATED_CARD_LOCK == FEATURE_ON)
/* 给NAS回复卡是否存在复用卡状态消息 */
typedef USIMM_CardStatusInd USIMM_CardExistQueryCnf;
#endif
/*
 * 结构说明: USIMM 回复消息头结构
 */
typedef struct {
    VOS_MSG_HEADER
    USIMM_CmdTypeCnfUint32 msgName;
    VOS_UINT32             result;
    VOS_UINT32             errorCode;
    USIMM_CardAppUint32    apptype;
    VOS_UINT32             sendPara;
} USIMM_CmdHeaderCnf;

/*
 * 结构说明: USIMM 主动上报消息头结构
 */
typedef struct {
    VOS_MSG_HEADER
    USIMM_CmdTypeCnfUint32 msgName;
} USIMM_CmdHeaderInd;

/*
 * 结构说明: USIMM 鉴权回复消息结构
 */
typedef struct {
    USIMM_CmdHeaderCnf     cmdResult;
    USIMM_AuthTypeUint32   authType;
    USIMM_AuthResultUint32 result;
    USIMM_AutnCnfDataBlock cnfData;
} USIMM_AuthenticationCnf;

typedef struct {
    USIMM_CmdHeaderCnf     cmdResult;
    USIMM_PinCmdTypeUint32 cmdType;
    USIMM_PinTypeUint32    pinType;
    USIMM_PinInfo          pinInfo;
} USIMM_PinHandleCnf;

typedef struct {
    USIMM_CmdHeaderCnf     cmdResult;
    USIMM_PinCmdTypeUint32 cmdType;
    USIMM_PinTypeUint32    pinType;
    USIMM_PinInfo          pinInfo;
    VOS_UINT8              oldPin[USIMM_PINNUMBER_LEN];
} USIMM_SilentPinHandleCnf;

typedef struct {
    USIMM_CmdHeaderCnf cmdResult;
    USIMM_FilePathInfo filePath;
    VOS_UINT16         rsv1;
    VOS_UINT16         efLen;
    VOS_UINT8          recordNum;
    VOS_UINT8          sw1;
    VOS_UINT8          sw2;
    VOS_UINT8          rsv2;
} USIMM_UpdateFileCnf;

typedef struct {
    USIMM_CmdHeaderCnf cmdResult;
    USIMM_FilePathInfo filePath;
    VOS_UINT16         rsv;
    VOS_UINT16         efLen;
    VOS_UINT8          recordNum;
    VOS_UINT8          totalNum;
    VOS_UINT8          sw1;
    VOS_UINT8          sw2;
    VOS_UINT8          efData[4];
} USIMM_ReadFileCnf;

typedef struct {
    USIMM_CmdHeaderCnf  cmdResult;
    USIMM_FilePathInfo  filePath;
    VOS_UINT16          rsv;
    VOS_UINT16          fileLen;
    USIMM_EFStatusUint8 fileStatus; /* 文件的激活状态 */
    USIMM_EFRWFlagUint8 fileRWFlag; /* 文件可读和可更新标志 */
    VOS_UINT8           recordNum;
    VOS_UINT8           recordLen;
    VOS_UINT8           fileInfo[4];
} USIMM_QueryFileCnf;

typedef struct {
    USIMM_CmdHeaderCnf cmdResult;
    VOS_UINT8          sw1;
    VOS_UINT8          sw2;
    VOS_UINT16         rsv;
    VOS_UINT32         dataLen;
    VOS_UINT8          data[4];
} USIMM_StatusCmdCnf;

typedef struct {
    USIMM_CmdHeaderCnf cmdResult;
    USIMM_FilePathInfo filePath;
    VOS_UINT16         totalRecNum; /* 文件中记录数 */
    VOS_UINT8          sw1;
    VOS_UINT8          sw2;
    VOS_UINT16         dataLen;
    VOS_UINT8          data[2];
} USIMM_SearchFileCnf;

/*
 * 结构说明: Store MEID/PESN处理回复消息结构
 */
typedef struct {
    USIMM_CmdHeaderCnf cmdResult;
    VOS_UINT8          isStored; /* VOS_TRUE:Store MEID/PESN */
    VOS_UINT8          storeResult;
    VOS_UINT8          rsv[2];
} USIMM_QueryEsnMeidRsltCnf;

/*
 * 结构说明: FDN处理回复消息结构
 */
typedef struct {
    USIMM_CmdHeaderCnf    cmdResult;
    USIMM_FdnStatusUint32 fdnState;
} USIMM_FdnProcessCnf;

/*
 * 结构说明: 逻辑通道打开处理回复消息结构
 */
typedef struct {
    USIMM_CmdHeaderCnf cmdResult;
    VOS_UINT32         sessionId;
    VOS_UINT8          channelId;
    VOS_UINT8          sw1; /* 返回状态字1  */
    VOS_UINT8          sw2; /* 返回状态字2  */
    VOS_UINT8          rsv[3];
    VOS_UINT16         rspDataLen;                      /* 输出数据长度 */
    VOS_UINT8          rspData[USIMM_APDU_RSP_MAX_LEN]; /* 输出数据内容 */
} USIMM_OpenChannelCnf;

/*
 * 结构说明: 逻辑通道关闭处理回复消息结构
 */
typedef struct {
    USIMM_CmdHeaderCnf cmdResult;
} USIMM_CloseChannelCnf;

typedef struct {
    USIMM_CmdHeaderCnf cmdResult;
    USIMM_FilePathInfo filePath;
    USIMM_TpduRspData  rspData;
} USIMM_SendTpduDataCnf;

typedef struct {
    USIMM_CmdHeaderCnf cmdResult;
    USIMM_CglaRspData  rspData;
} USIMM_PrivateCglaHandleCnf;

/*
 * 结构说明: CDMA Base Station Challenge处理回复消息结构
 */
typedef struct {
    USIMM_CmdHeaderCnf cmdResult;
    VOS_UINT32         dataLen;
    VOS_UINT8          data[4];
} USIMM_BschallengeCnf;

/*
 * 结构说明: USIMM GENERATE KEY & VMP 处理回复消息结构
 */
typedef struct {
    USIMM_CmdHeaderCnf cmdResult;
    VOS_UINT8          key[USIMM_CDMA_GEN_KEY_LEN];
    VOS_UINT32         vpmLen;
    VOS_UINT8          vpm[4];
} USIMM_GenerateKeyVpmCnf;

/*
 * 结构说明: USIMM TERMINAL RESPONSE处理回复消息结构
 */
typedef struct {
    USIMM_CmdHeaderCnf cmdResult;
    VOS_UINT8          sw1;
    VOS_UINT8          sw2;
    VOS_UINT8          rsv[2];
} USIMM_STKTerminalRspCnf;

/*
 * 结构说明: USIMM简单命令处理回复消息结构
 */
typedef struct {
    USIMM_CmdHeaderCnf cmdResult;
} USIMM_SingleCmdCnf;

/*
 * 结构说明: USIMM REFRESH处理回复消息结构
 */
typedef struct {
    USIMM_CmdHeaderCnf      cmdResult;
    USIMM_RefreshTypeUint32 refreshType;
    VOS_UINT32              satLen; /* 后续主动命令的长度 */
} USIMM_STKRefreshCnf;

/*
 * 结构说明: USIMM ENVELOPE CNF消息结构
 */
typedef struct {
    USIMM_CmdHeaderCnf cmdResult;
    VOS_PID            opReturnPid;
    VOS_UINT8          sw1;
    VOS_UINT8          sw2;
    VOS_UINT8          dataType;
    VOS_UINT8          rsv;
    VOS_UINT32         dataLen;
    VOS_UINT8          data[USIMM_T0_APDU_MAX_LEN];
} USIMM_STKEnvelopeCnf;

/*
 * 结构说明: USIMM发送给的重试STKENVELOPE的消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    USIMM_CmdTypeReqUint32 msgName;
    USIMM_STKEnvelopeReq   envelopeReqMsgData;
} USIMM_STKEnvelopeRetry;

/*
 * 结构说明: 电话本多文件更新 CNF消息结构
 */
typedef struct {
    USIMM_CmdHeaderCnf cmdResult;
    VOS_UINT16         efId[USIMM_SETMUTILFILE_MAX];      /* 要更新的文件的ID */
    VOS_UINT16         efLen[USIMM_SETMUTILFILE_MAX];     /* EF长度 */
    VOS_UINT8          recordNum[USIMM_SETMUTILFILE_MAX]; /* linear fixed或cycle fixed类型EF记录号 */
    VOS_UINT8          fileNum;
    VOS_UINT8          rsv;
} USIMM_SetMutilFileCnf;

/*
 * 结构说明: 发给OM的透明消息
 */
typedef struct {
    VOS_MSG_HEADER          /* 该消息头填充TOOL_ID和PS的PID,保证消息能转发给相应的PS模块 */
    VOS_UINT16 transPrimId; /* 固定填写0x5001 */
    VOS_UINT16 reserve1;
    VOS_UINT8  funcType;    /* 数据查询04,空口消息上报:02 */
    VOS_UINT8  reserve2;
    VOS_UINT16 appLength;   /* 下一个字节到包尾的长度 */
    VOS_UINT32 sn;          /* 上报消息的序列号 */
    VOS_UINT32 timeStamp;   /* 消息的时间戳，填当前的SOC Tick值 */
    VOS_UINT16 primId;      /* 原语ID */
    VOS_UINT16 toolsId;     /* 工具ID */
    VOS_UINT8  result;      /* 传输消息的内容 */
    VOS_UINT8  efLen;       /* 前两个字节填写读取的长度 */
    VOS_UINT8  data[2];
} USIMM_OmTransMsg;

/*
 * 结构说明: USIMM STK主动上报消息结构
 */
typedef struct {
    USIMM_CmdHeaderInd indHdr;
    VOS_UINT16         len;     /* 主动命令长度 */
    VOS_UINT8          cmdType; /* 主动命令类型 */
    VOS_UINT8          rsv;
    VOS_UINT8          content[USIMM_T0_APDU_MAX_LEN];
} USIMM_STKDataInd;

/*
 * 结构说明: USIMM REFRESH主动上报消息结构
 */
typedef struct {
    USIMM_CmdHeaderInd          indHdr;
    USIMM_RefreshTypeUint32     usimRefreshType; /* 卡刷新类型 */
    USIMM_RefreshFileTypeUint16 refreshType;     /* 文件更新类型 */
    VOS_UINT16                  efNum;           /* 更新的文件的个数 */
    USIM_REFRESH_File           efId[1];         /* 更新的文件ID 信息，可变长数组 */
} USIMM_STKRefreshInd;

/*
 * 结构说明: USIMM 受限制访问CNF消息结构
 */
typedef struct {
    USIMM_CmdHeaderCnf cmdResult;
    VOS_UINT8          sw1;                             /* Status Word 1 */
    VOS_UINT8          sw2;                             /* Status Word 2 */
    VOS_UINT16         len;                             /* 返回数据长度,不包含SW1和SW2       */
    VOS_UINT8          content[USIMM_APDU_RSP_MAX_LEN]; /* 返回Data */
} USIMM_RaccessCnf;

/*
 * 结构说明: ECC号码上报消息结构
 */
typedef struct {
    USIMM_CmdHeaderInd indHdr;
    VOS_UINT16         fileSize;   /* content中数据长度 */
    VOS_UINT8          eccType;    /* 紧急呼叫号码是SIM卡还是USIM卡 */
    VOS_UINT8          recordNum;  /* USIM卡紧急呼叫号码记录数，SIM卡填0 */
    VOS_UINT8          content[4]; /* 紧急呼叫号码记录信息 */
} USIMM_EccNumberInd;

/*
 * 结构说明: 虚拟卡DH重协议上报指示消息结构
 */
typedef struct {
    USIMM_CmdHeaderInd indHdr;
} USIMM_VsimRdhInd;

/*
 * 结构说明: 卡拔插上报指示消息结构
 */
typedef struct {
    USIMM_CmdHeaderInd indHdr;
    VOS_UINT32 cardInOutStatus;
} USIMM_HotInOutInd;

/*
 * 结构说明: 应用SESSION信息
 */
typedef struct {
    VOS_UINT32 channelId;
    VOS_UINT32 sessionId;
} USIMM_AppSessionInfo;

/*
 * 结构说明: 查询应用SESSION处理回复消息结构
 */
typedef struct {
    USIMM_CmdHeaderCnf   cmdResult;
    USIMM_AppSessionInfo sessionInfo[USIMM_CARDAPP_BUTT];
} USIMM_CardSessionCnf;

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
/*
 * 结构说明: XECC号码上报消息结构
 */
typedef struct {
    USIMM_CmdHeaderInd indHdr;
    VOS_UINT16         fileSize;   /* content中数据长度 */
    VOS_UINT8          content[2]; /* 紧急呼叫号码记录信息 */
} USIMM_XeccNumberInd;

/*
 * 结构说明: This structure defines the reply data format after processing
 *    the Compute IP request.
 * The command type of Compute IP request, that input by
 *       request massage;
 * The data length that receive from card;
 * The data content that receive from card;
 */
typedef struct {
    USIMM_ComputeipAuthUint32 computeIPType;
    VOS_UINT32                dataLen;
    VOS_UINT8                 rspData[USIMM_APDU_RSP_MAX_LEN];
} USIMM_ComputeipCnf;

/*
 * 结构说明: This structure defines the BCMCS command response data
 *    format, that contain by return massage.
 * The BCMCS command type that input by request;
 * The response data tag, which could see Annex B in
 *       <Invalid document reference (C.S0023 D_v2.0)>;
 * The response data length;
 * The response data, and the max value is 16 bytes, according
 *       all of BCMCS command;
 */
typedef struct {
    USIMM_BcmcsAuthUint32 bcmcsType;
    VOS_UINT8             bcmcsRspTag;
    VOS_UINT8             bcmcsRspLen;
    VOS_UINT8             bcmcsRsp[USIMM_BCMCSRSP_LEN_MAX];
    VOS_UINT8             rsv[2];
} USIMM_BcmcsCnf;

/*
 * 结构说明: This structure defines the response data that by processing
 * the Application Authentication command. See section 4.10.1 in <Invalid
 * document reference (C.S0023 D_v2.0)> and section 9.4.6.1 in 3GPP2 C.S0065 B
 *  "cdma2000 Application on UICC for Spread Spectrum Systems".
 *  The command type which input by request massage;
 *  response data;
 *  Session key 1;
 *  Session key 2;
 */
typedef struct {
    USIMM_CdmaAppAuthUint32 appAuthType;
    USIMM_U8Data            response;
    USIMM_U8Data            sessionKey1;
    USIMM_U8Data            sessionKey2;
} USIMM_CdmaAppAuthCnf;

/*
 * 结构说明: This union defines the data container in
 *    PS_USIM_CDMASPECAUTH_CNF .
 * The response data of Compute IP command, defined by
 *       USIMM_ComputeipCnf;
 * The response data of BCMCS command, defined by
 *       USIMM_BcmcsCnf;
 * The response data of OTA, defined by
 *       USIMM_CDMAOTA_AUTH_CNF_STRU;
 */
typedef union {
    USIMM_ComputeipCnf   computeIPCnf;
    USIMM_BcmcsCnf       bcmcsCnf;
    USIMM_CdmaAppAuthCnf appAuthCnf;
} USIMM_CdmaSpecAuthData;

/*
 * 结构说明: USIMM 受限制访问CNF消息结构
 */
typedef struct {
    USIMM_CmdHeaderCnf           cmdResult;
    USIMM_CdmaSpecAuthTypeUint32 authType;
    USIMM_CdmaSpecAuthData       authCnf; /* 返回Data */
} USIMM_CdmaSpecAuthCnf;
#endif

typedef struct {
    USIMM_CmdHeaderInd     indHdr;
    USIMM_CmdTypeReqUint32 cmdType;
} USIMM_CardSciErrInd;

typedef struct {
    VOS_UINT32 errNum;
    VOS_UINT32 errInfo[USIMM_ERROR_INFO_MAX];
} USIMM_ErrorInfoData;

typedef struct {
    USIMM_CmdHeaderInd  indHdr;
    USIMM_ErrorInfoData errorInfo;
} USIMM_CardErrorInd;

typedef struct {
    USIMM_CmdHeaderInd        indHdr;
    USIMM_CardFetchTypeUint32 cardFetchType;
} USIMM_CardFetchInd;

/*
 * 结构说明: ICCID上报消息
 */
typedef struct {
    USIMM_CmdHeaderInd indHdr;
    VOS_UINT8          iccid[USIMM_ICCID_FILE_LEN];
    VOS_UINT8          rcv[2];
} USIMM_IccidContentInd;

/*
 * 结构说明: 请求ICC切模消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32          msgName;
    USIMM_CardAppUint32 newCardApp;
    VOS_UINT32          sendPara;
} USIMM_ChangeCardModeReq;

/*
 * 结构说明: ICC切模回复消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                  msgName;
    VOS_UINT32                  result;
    USIMM_ChangeModeErrorUint32 errCode;
} USIMM_ChangeCardModeCnf;

/*
 * 结构说明: FDN状态查询消息
 */
typedef struct {
    USIMM_CmdHeaderReq msgHeader;
} USIMM_FdnQueryReq;

/*
 * 结构说明: FDN状态回复消息
 */
typedef struct {
    USIMM_CmdHeaderCnf    cmdResult;
    USIMM_FdnStatusUint32 fdnState;
} USIMM_FdnQueryCnf;

/*
 * 结构说明: CardtypeEx状态回复消息
 */
typedef struct {
    USIMM_CmdHeaderCnf cmdResult;
    VOS_UINT32         cardMode;
    VOS_BOOL           hasCModule;
    VOS_BOOL           hasGModule;
} USIMM_QueryCardTypeCnf;

/*
 * 结构说明: FDN状态主动上报消息
 */
typedef struct {
    USIMM_CmdHeaderCnf    cmdResult;
    USIMM_FdnStatusUint32 fdnState;
} USIMM_FdnStatusInd;

typedef struct {
    USIMM_CmdHeaderCnf cmdResult;
    VOS_UINT32         voltage;
    VOS_UINT8          charaByte;
    VOS_UINT8          rsv[3];
} USIMM_QueryVoltageCnf;

/*
 * 1. 日    期: 2017年7月24日
 * 修改内容: 多实例修改
 */
typedef struct {
    VOS_UINT32 bitModemNum : 3;
    VOS_UINT32 bitReserved0 : 5;  /* bit[3-7]    : 保留。 */
    VOS_UINT32 bitModem0Slot : 3; /* bit[8-10]   : modem 0使用的SCI接口。
                                                      0：使用SCI0；
                                                      1：使用SCI1；
                                                      2：使用SCI2；
                                                    其余值:多卡，暂不支持 */
    VOS_UINT32 bitModem1Slot : 3; /* bit[11-13]  : modem 1使用的SCI接口。
                                                       0：使用SCI0；
                                                       1：使用SCI1；
                                                       2：使用SCI2；
                                                     其余值:多卡，暂不支持 */
    VOS_UINT32 bitModem2Slot : 3; /* bit[14-16]  : modem 2使用的SCI接口。
                                                       0：使用SCI0；
                                                       1：使用SCI1；
                                                       2：使用SCI2；
                                                     其余值:多卡，暂不支持 */
    VOS_UINT32 bitReserved1 : 15; /* bit[17-31]  : 保留 */
} USIMM_SciCfg;

/*
 * 结构说明: USIMM 回复Pb功能状态 消息头结构
 */
typedef struct {
    VOS_MSG_HEADER
    USIMM_CmdTypeCnfUint32 msgName;
    VOS_BOOL               pbReady; /* VOS_TRUE:pb ready  VOS_FALSE:pb not ready */
} USIMM_PbStatusCnf;

/*
 * 结构说明: USIMM 回复Simsq状态 消息头结构
 */
typedef struct {
    VOS_MSG_HEADER
    USIMM_CmdTypeCnfUint32 msgName;
    USIMM_SimsqStateUint32 simSqStatus;
} USIMM_SimsqStatusInd;

/*
 * 结构说明: USIMM 主动上报是否支持5G卡消息给STK
 */
typedef struct {
    USIMM_CmdHeaderCnf cmdResult;
    VOS_BOOL           isSupprot5GCardFIle;
} USIMM_Support5GCardFileInd;

/*
 * 结构说明: eUICC和UICC消息结构
 */
typedef struct {
    VOS_UINT8 slot;
    VOS_UINT8 cardType;
    VOS_UINT8 rsv[2];
} USIMM_EsimSwtich;

/*
 * 结构说明: eUICC和UICC切换请求消息结构
 */
typedef struct {
    USIMM_CmdHeaderReq msgHeader;
    USIMM_EsimSwtich   esimSwitch;
} USIMM_EsimSwtichReq;

/*
 * 结构说明: 卡槽对应电路状态
 */
typedef struct {
    VOS_INT32 slot0CardType;
    VOS_INT32 slot1CardType;
} USIMM_SlotCardType;

/*
 * 结构说明: 查询卡槽对应电路状态消息结构
 */
typedef struct {
    USIMM_CmdHeaderCnf msgHeader;
    USIMM_SlotCardType cardType;
} USIMM_EsimSwtichQryCnf;

/*
 * 结构说明: Get Identity请求消息结构
 */
typedef struct {
    USIMM_CmdHeaderReq msgHeader;
} USIMM_GetIdentityReq;

/*
 * 结构说明: Get Identity CNF消息结构
 */
/*lint -save -e43 */
typedef struct {
    USIMM_CmdHeaderCnf cmdResult;
    VOS_UINT32         length;
    VOS_UINT8          suci[0];
} USIMM_GetIdentityCnf;
/*lint -restore */

#if (FEATURE_MBB_CUST == FEATURE_ON)
#if (FEATURE_PHONE_USIM == FEATURE_OFF)
/* NOCARD开关标志 */
enum USIMM_NocardMode {
    USIMM_NOCARD_MODE_OFF = 0,
    USIMM_NOCARD_MODE_ON = 1,
    USIMM_NOCARD_MODE_BUFF
};

/* 向USIMM通知NOCARD消息结构 */
typedef struct {
    USIMM_CmdHeaderReq msgHeader;
    VOS_UINT32 noCardMode;
} USIMM_SetNoCardReq;

/* NOCARD设置结果响应消息结构 */
typedef struct {
    USIMM_CmdHeaderCnf msgHeader;
    VOS_UINT32 noCardMode;
} USIMM_SetNoCardCnf;
#endif
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cpluscplus */

#endif
