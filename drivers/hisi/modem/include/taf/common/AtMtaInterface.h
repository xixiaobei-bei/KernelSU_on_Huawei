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

#ifndef __ATMTAINTERFACE_H__
#define __ATMTAINTERFACE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "PsTypeDef.h"
#include "AtMnInterface.h"
#include "MtaCommInterface.h"
#include "nv_stru_sys.h"
#include "nv_stru_gucnas.h"

#include "AtParse.h"

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define MTA_CPOS_XML_MAX_LEN            (1024)              /* MTA接收AT下发的XML码流最大长度 */
#define MTA_CPOSR_XML_MAX_LEN           (1024)              /* MTA向AT上报的XML码流最大长度为1024 */
#define MTA_SIMLOCK_PASSWORD_LEN        (16)                /* 锁网锁卡解锁密码的长度 */
#define MTA_MAX_EPDU_NAME_LEN           (32)                /* EPDU码流上报中Name的长度 */
#define MTA_MAX_EPDU_BODY_LEN           (500)               /* EPDU码流大小 */

#define MTA_CLIENTID_BROADCAST          (0xFFFF)
#define MTA_INVALID_TAB_INDEX           (0x00)


#define AT_MTA_WRR_AUTOTEST_MAX_PARA_NUM          (10) /* autotest命令下发参数最大个数 */
#define MTA_AT_WRR_AUTOTEST_MAX_RSULT_NUM         (20) /* autotest命令上报结果最大个数 */
#define MTA_AT_WRR_ONE_MEANRPT_MAX_CELL_NUM       (8)  /* 一次测量报告中小区最大个数 */
#define MTA_AT_WRR_MAX_MEANRPT_NUM                (10) /* 上报最近测量报告的最大个数 */

#define MTA_AT_WRR_MAX_NCELL_NUM                  (8)

#define AT_MTA_GAS_AUTOTEST_MAX_PARA_NUM          (10)
#define MTA_AT_GAS_AUTOTEST_MAX_RSULT_NUM         (20) /* autotest命令上报结果最大个数 */

#define AT_MTA_RPT_CFG_MAX_SIZE                   (8)

#define AT_MTA_HANDLEDECT_MIN_TYPE                (0)
#define AT_MTA_HANDLEDECT_MAX_TYPE                (4)

#if (FEATURE_ON == FEATURE_SC_SEC_UPDATE)
#define AT_RSA_CIPHERTEXT_LEN           (256)
#else
#define AT_RSA_CIPHERTEXT_LEN           (128)
#endif

#define  AT_MTA_INTEREST_FREQ_MAX_NUM         (0x05)
#define  AT_MTA_MBMS_AVL_SERVICE_MAX_NUM      (0x10)
#define  AT_MTA_ISMCOEX_BANDWIDTH_NUM         (0x06)

#define AT_MTA_MEID_DATA_LEN                         (7)

#define MTA_AT_EFRUIMID_OCTET_LEN_EIGHT                   (8)
#define MTA_AT_EFRUIMID_OCTET_LEN_FIVE                    (5)

#define MTA_AT_UIMID_OCTET_LEN                            (4)

#define AT_MTA_GPS_XO_COEF_NUM                       (4)

#if (FEATURE_ON == FEATURE_PROBE_FREQLOCK)
#define MTA_FREQLOCK_MAX_MODE                     (4)
#endif

#define MTC_MTA_MAX_BESTFREQ_GROUPNUM        (8)

#define MTA_RCM_MAX_DIE_ID_LEN              (32)

#define MTA_PMU_MAX_DIE_SN_LEN              (20)

#define AT_MAX_RS_INFO_NUM                  (32)    /* LRRC最大RS信息个数 */
#define AT_RS_INFO_MULTI                    (100)   /* LRRC将RSRP、RSRQ的值乘100，再报给MTA */

#define AT_MTA_PSEUCELL_PLMN_LEN_FIVE       (5)
#define AT_MTA_PSEUCELL_PLMN_LEN_SIX        (6)
#define AT_MTA_PSEUCELL_LAC_LEN             (4)
#define AT_MTA_PSEUCELL_CELLID_LEN          (8)
#define AT_MTA_PLMN_MCC_LEN                 (3)
#define AT_MTA_PLMN_MNC_LEN_TWO             (2)
#define AT_MTA_PLMN_MNC_LEN_THREE           (3)

#define PESN_NV_DATA_LEN                            (4)
#define MEID_NV_DATA_LEN_NEW                        (7)

#define AT_MTA_EPDU_NAME_LENGTH_MAX         (32)
#define AT_MTA_EPDU_CONTENT_LENGTH_MAX      (250)
#define AT_MTA_EPDU_CONTENT_STRING_LEN_MAX  (500)

#define AT_MTA_MAX_BAND_NUM                 (8)     /* 最多支持8个Band */
#define AT_MTA_BAND_INFO_OFFSET             (3)     /* band信息偏移参数值 */
#define AT_MTA_MAX_SUB_BAND_NUM             (4)     /* 每个Band的频率可以分为4个子BAND */
#define AT_MTA_BAND_INFO_LEN                (72)    /* 每个Band信息的长度信息 */
#define MTA_AT_CA_MAX_CELL_NUM              (8)
#define MTA_AT_UTC_MAX_LENGTH               (21)

#define MTA_AT_BOOSTER_IND_MAX_NUM          (248)      /* Booster特性最多支持LRRC给MTA上报248字节码流 */
#define MTA_AT_BOOSTER_QRY_CNF_MAX_NUM      (40)      /* Booster特性最多支持查询结果字节码流 */

#define MTA_AT_EPS_NETWORK_BYTE             (2)

/*****************************************************************************
  3 枚举定义
*****************************************************************************/

enum AT_MTA_MSG_TYPE_ENUM
{
    /* 消息名称 */                      /* 消息ID */        /* 备注, 生成ASN */
    /* AT发给MTA的消息 */
    ID_AT_MTA_CPOS_SET_REQ              = 0x0000,           /* _H2ASN_MsgChoice AT_MTA_CPOS_REQ_STRU        */
    ID_AT_MTA_CGPSCLOCK_SET_REQ         = 0x0001,           /* _H2ASN_MsgChoice AT_MTA_CGPSCLOCK_REQ_STRU   */
    ID_AT_MTA_SIMLOCKUNLOCK_SET_REQ     = 0x0003,           /* _H2ASN_MsgChoice AT_MTA_SIMLOCKUNLOCK_REQ_STRU */

    ID_AT_MTA_QRY_NMR_REQ               = 0x0004,           /* _H2ASN_MsgChoice AT_MTA_QRY_NMR_REQ_STRU */

    ID_AT_MTA_RESEL_OFFSET_CFG_SET_NTF  = 0x0005,           /* _H2ASN_MsgChoice AT_MTA_RESEL_OFFSET_CFG_SET_NTF_STRU */
    ID_AT_MTA_WRR_AUTOTEST_QRY_REQ      = 0x0006,           /* _H2ASN_MsgChoice AT_MTA_WRR_AUTOTEST_QRY_PARA_STRU */
    ID_AT_MTA_WRR_CELLINFO_QRY_REQ      = 0x0007,           /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */
    ID_AT_MTA_WRR_MEANRPT_QRY_REQ       = 0x0008,           /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */
    ID_AT_MTA_WRR_FREQLOCK_SET_REQ      = 0x0009,           /* _H2ASN_MsgChoice MTA_AT_WRR_FREQLOCK_CTRL_STRU     */
    ID_AT_MTA_WRR_RRC_VERSION_SET_REQ   = 0x000A,           /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU*/
    ID_AT_MTA_WRR_CELLSRH_SET_REQ       = 0x000B,           /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU*/
    ID_AT_MTA_WRR_FREQLOCK_QRY_REQ      = 0x000C,           /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */
    ID_AT_MTA_WRR_RRC_VERSION_QRY_REQ   = 0x000D,           /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */
    ID_AT_MTA_WRR_CELLSRH_QRY_REQ       = 0x000E,           /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */

    ID_AT_MTA_BODY_SAR_SET_REQ          = 0x000F,           /* _H2ASN_MsgChoice AT_MTA_BODY_SAR_SET_REQ_STRU */
    ID_AT_MTA_CURC_SET_NOTIFY           = 0x0010,           /* _H2ASN_MsgChoice AT_MTA_CURC_SET_NOTIFY_STRU */
    ID_AT_MTA_CURC_QRY_REQ              = 0x0011,           /* _H2ASN_MsgChoice AT_MTA_CURC_QRY_REQ_STRU */
    ID_AT_MTA_UNSOLICITED_RPT_SET_REQ   = 0x0012,           /* _H2ASN_MsgChoice AT_MTA_UNSOLICITED_RPT_SET_REQ_STRU */
    ID_AT_MTA_UNSOLICITED_RPT_QRY_REQ   = 0x0013,           /* _H2ASN_MsgChoice AT_MTA_UNSOLICITED_RPT_QRY_REQ_STRU */

    ID_AT_MTA_IMEI_VERIFY_QRY_REQ       = 0x0014,           /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */
    ID_AT_MTA_CGSN_QRY_REQ              = 0x0015,           /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */
    ID_AT_MTA_NCELL_MONITOR_SET_REQ     = 0x0016,           /* _H2ASN_MsgChoice AT_MTA_NCELL_MONITOR_SET_REQ_STRU */
    ID_AT_MTA_NCELL_MONITOR_QRY_REQ     = 0x0017,           /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */

    ID_AT_MTA_REFCLKFREQ_SET_REQ        = 0x0018,           /* _H2ASN_MsgChoice AT_MTA_REFCLKFREQ_SET_REQ_STRU */
    ID_AT_MTA_REFCLKFREQ_QRY_REQ        = 0x0019,           /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */

    ID_AT_MTA_HANDLEDECT_SET_REQ        = 0x001A,           /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */
    ID_AT_MTA_HANDLEDECT_QRY_REQ        = 0x001B,           /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */

    ID_AT_MTA_ECID_SET_REQ              = 0x001C,           /* _H2ASN_MsgChoice AT_MTA_ECID_SET_REQ_STRU */

    ID_AT_MTA_MIPICLK_QRY_REQ           = 0x001D,           /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */

    ID_AT_MTA_SET_DPDTTEST_FLAG_REQ     = 0x001E,           /* _H2ASN_MsgChoice AT_MTA_SET_DPDTTEST_FLAG_REQ_STRU */
    ID_AT_MTA_SET_DPDT_VALUE_REQ        = 0x001F,           /* _H2ASN_MsgChoice AT_MTA_SET_DPDT_VALUE_REQ_STRU */
    ID_AT_MTA_QRY_DPDT_VALUE_REQ        = 0x0020,           /* _H2ASN_MsgChoice AT_MTA_QRY_DPDT_VALUE_REQ_STRU */

    ID_AT_MTA_RRC_PROTECT_PS_REQ        = 0x0021,
    ID_AT_MTA_PHY_INIT_REQ              = 0x0022,

    ID_AT_MTA_SET_JAM_DETECT_REQ        = 0x0023,           /* _H2ASN_MsgChoice AT_MTA_SET_JAM_DETECT_REQ_STRU */

    ID_AT_MTA_SET_FREQ_LOCK_REQ         = 0x0024,           /* _H2ASN_MsgChoice AT_MTA_SET_FREQ_LOCK_REQ_STRU */

    ID_AT_MTA_SET_GSM_FREQLOCK_REQ      = 0x0025,          /* _H2ASN_MsgChoice AT_MTA_SET_GSM_FREQLOCK_REQ_STRU */
    ID_AT_MTA_SET_FEMCTRL_REQ           = 0x0026,

    ID_AT_MTA_NVWRSECCTRL_SET_REQ       = 0x0027,

    ID_AT_MTA_MBMS_SERVICE_OPTION_SET_REQ   = 0x0028,      /* _H2ASN_MsgChoice AT_MTA_MBMS_SERVICE_OPTION_SET_REQ_STRU */
    ID_AT_MTA_MBMS_PREFERENCE_SET_REQ       = 0x0029,      /* _H2ASN_MsgChoice AT_MTA_MBMS_PREFERENCE_SET_REQ_STRU */
    ID_AT_MTA_SIB16_NETWORK_TIME_QRY_REQ    = 0x002A,      /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */
    ID_AT_MTA_BSSI_SIGNAL_LEVEL_QRY_REQ     = 0x002B,      /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */
    ID_AT_MTA_NETWORK_INFO_QRY_REQ          = 0x002C,      /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */
    ID_AT_MTA_EMBMS_STATUS_QRY_REQ          = 0x002D,      /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */
    ID_AT_MTA_MBMS_UNSOLICITED_CFG_SET_REQ  = 0x002E,      /* _H2ASN_MsgChoice AT_MTA_MBMS_UNSOLICITED_CFG_SET_REQ_STRU */
    ID_AT_MTA_LTE_LOW_POWER_SET_REQ         = 0x002F,      /* _H2ASN_MsgChoice AT_MTA_LOW_POWER_CONSUMPTION_SET_REQ_STRU */
    ID_AT_MTA_INTEREST_LIST_SET_REQ         = 0x0030,      /* _H2ASN_MsgChoice AT_MTA_MBMS_INTERESTLIST_SET_REQ_STRU */
    ID_AT_MTA_MBMS_SERVICE_STATE_SET_REQ    = 0x0031,      /* _H2ASN_MsgChoice AT_MTA_MBMS_SERVICE_STATE_SET_REQ_STRU */
    ID_AT_MTA_MBMS_AVL_SERVICE_LIST_QRY_REQ = 0x0032,      /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */
    ID_AT_MTA_LTE_WIFI_COEX_SET_REQ         = 0x0033,      /* _H2ASN_MsgChoice AT_MTA_LTE_WIFI_COEX_SET_REQ_STRU */
    ID_AT_MTA_LTE_WIFI_COEX_QRY_REQ         = 0x0034,      /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */

    ID_AT_MTA_SET_FR_REQ                = 0x0035,

    ID_AT_MTA_MEID_SET_REQ              = 0x0036,     /* _H2ASN_MsgChoice AT_MTA_MEID_SET_REQ_STRU */
    ID_AT_MTA_MEID_QRY_REQ              = 0x0037,

#if (FEATURE_ON == FEATURE_PROBE_FREQLOCK)
    ID_AT_MTA_SET_M2M_FREQLOCK_REQ      = 0x0038,           /* _H2ASN_MsgChoice AT_MTA_SET_M2M_FREQLOCK_REQ_STRU */
    ID_AT_MTA_QRY_M2M_FREQLOCK_REQ      = 0x0039,           /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */
#endif

    ID_AT_MTA_TRANSMODE_QRY_REQ         = 0x0044,

    ID_AT_MTA_UE_CENTER_SET_REQ         = 0x0045,     /* _H2ASN_MsgChoice AT_MTA_SET_UE_CENTER_REQ_STRU */
    ID_AT_MTA_UE_CENTER_QRY_REQ         = 0x0046,

    ID_AT_MTA_SET_NETMON_SCELL_REQ      = 0x0047,
    ID_AT_MTA_SET_NETMON_NCELL_REQ      = 0x0048,

    ID_AT_MTA_RFICSSIRD_QRY_REQ         = 0x0049,

    ID_AT_MTA_ANQUERY_QRY_REQ           = 0x004A,
    ID_AT_MTA_CSNR_QRY_REQ              = 0x004B,
    ID_AT_MTA_CSQLVL_QRY_REQ            = 0x004C,

    ID_AT_MTA_EVDO_SYS_EVENT_SET_REQ    = 0x004d,     /* _H2ASN_MsgChoice AT_MTA_EVDO_SYS_EVENT_SET_REQ_STRU */
    ID_AT_MTA_EVDO_SIG_MASK_SET_REQ     = 0x004e,     /* _H2ASN_MsgChoice AT_MTA_EVDO_SIG_MASK_SET_REQ_STRU */

    ID_AT_MTA_XCPOSR_SET_REQ            = 0x004F,                               /* _H2ASN_MsgChoice AT_MTA_SET_XCPOSR_REQ_STRU */
    ID_AT_MTA_XCPOSR_QRY_REQ            = 0x0050,                               /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */
    ID_AT_MTA_XCPOSRRPT_SET_REQ         = 0x0051,                               /* _H2ASN_MsgChoice AT_MTA_SET_XCPOSRRPT_REQ_STRU */
    ID_AT_MTA_XCPOSRRPT_QRY_REQ         = 0x0052,                               /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */
    ID_AT_MTA_CLEAR_HISTORY_FREQ_REQ    = 0x0053,       /* _H2ASN_MsgChoice AT_MTA_CLEAR_HISTORY_FREQ_REQ_STRU */

    ID_AT_MTA_SET_SENSOR_REQ            = 0x0054,                               /* _H2ASN_MsgChoice AT_MTA_SET_SENSOR_REQ_STRU*/
    ID_AT_MTA_SET_SCREEN_REQ            = 0x0055,                               /* _H2ASN_MsgChoice AT_MTA_SET_SCREEN_REQ_STRU*/
    ID_AT_MTA_FRAT_IGNITION_QRY_REQ     = 0x0056,                               /* _H2ASN_MsgChoice MTA_AT_FRAT_IGNITION_QRY_CNF_STRU */
    ID_AT_MTA_FRAT_IGNITION_SET_REQ     = 0x0057,                               /* _H2ASN_MsgChoice MTA_AT_FRAT_IGNITION_SET_CNF_STRU */

    ID_AT_MTA_MODEM_TIME_SET_REQ        = 0x0058,

    ID_AT_MTA_RX_TEST_MODE_SET_REQ      = 0x0059,

    ID_AT_MTA_SLAVE_SET_REQ             = 0x005A,

    ID_AT_MTA_BESTFREQ_SET_REQ          = 0x005B,           /* AT_MTA_BESTFREQ_SET_REQ_STRU */
    ID_AT_MTA_BESTFREQ_QRY_REQ          = 0x005C,

    ID_AT_MTA_RFIC_DIE_ID_QRY_REQ       = 0x005D,

    ID_AT_MTA_CRRCONN_SET_REQ           = 0x005E,
    ID_AT_MTA_CRRCONN_QRY_REQ           = 0x005F,
    ID_AT_MTA_VTRLQUALRPT_SET_REQ       = 0x0060,

    ID_AT_MTA_PMU_DIE_SN_QRY_REQ        = 0x0061,

    ID_AT_MTA_MODEM_CAP_UPDATE_REQ      = 0x0062,

    ID_AT_MTA_TAS_CFG_REQ               = 0x0063,
    ID_AT_MTA_TAS_QRY_REQ               = 0x0064,

    ID_AT_MTA_MIPI_WREX_REQ              = 0x0065,
    ID_AT_MTA_MIPI_RDEX_REQ              = 0x0066,

    ID_AT_MTA_RS_INFO_QRY_REQ           = 0x0067,

    ID_AT_MTA_PHY_COM_CFG_SET_REQ       = 0x0068,

    ID_AT_MTA_ERRCCAP_CFG_SET_REQ       = 0x0069,
    ID_AT_MTA_ERRCCAP_QRY_SET_REQ       = 0x006A,

    ID_AT_MTA_QRY_GSM_FREQLOCK_REQ      = 0x006B,
    ID_AT_MTA_DEL_CELLENTITY_REQ        = 0x006C,
    ID_AT_MTA_PSEUCELL_INFO_SET_REQ     = 0x006D,

    ID_AT_MTA_POWER_DET_QRY_REQ         = 0x006E,           /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */

    ID_AT_MTA_MIPIREAD_SET_REQ          = 0x006F,

    ID_AT_MTA_NO_CARD_MODE_SET_REQ      = 0x0070,
    ID_AT_MTA_NO_CARD_MODE_QRY_REQ      = 0x0071,

    ID_AT_MTA_CHRALARMRLAT_CFG_SET_REQ  = 0x0072,           /* _H2ASN_MsgChoice AT_MTA_CHR_ALARM_RLAT_CFG_SET_REQ_STRU */

    ID_AT_MTA_ECC_CFG_SET_REQ           = 0x0073,

    ID_AT_MTA_EPDU_SET_REQ              = 0x0074,

    ID_AT_MTA_LTE_CA_CELL_RPT_CFG_SET_REQ = 0x0075,
    ID_AT_MTA_LTE_CA_CELL_RPT_CFG_QRY_REQ = 0x0076,
    ID_AT_MTA_LTE_CA_CFG_SET_REQ          = 0x0077,
    ID_AT_MTA_LTE_CA_CELLEX_QRY_REQ       = 0x0078,

    ID_AT_MTA_LTE_FINE_TIME_SET_REQ     = 0x0079,
    ID_AT_MTA_GAS_AUTOTEST_QRY_REQ        = 0x0080,               /* _H2ASN_MsgChoice AT_MTA_GAS_AUTOTEST_QRY_PARA_STRU    */

    ID_AT_MTA_COMM_BOOSTER_SET_REQ     = 0x0081,           /* _H2ASN_MsgChoice AT_MTA_COMM_BOOSTER_SET_REQ_STRU */
    ID_AT_MTA_COMM_BOOSTER_QRY_REQ     = 0x0082,           /* _H2ASN_MsgChoice AT_MTA_COMM_BOOSTER_QUERY_REQ_STRU */


    ID_AT_MTA_NVLOAD_SET_REQ           = 0x0083,

    ID_AT_MTA_SMS_DOMAIN_SET_REQ         = 0x0084,
    ID_AT_MTA_SMS_DOMAIN_QRY_REQ         = 0x0085,
    ID_AT_MTA_EPS_NETWORK_QRY_REQ       = 0x0086,                               /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */

    ID_AT_MTA_SET_SAMPLE_REQ             = 0x0087,

    ID_AT_MTA_GPS_LOCSET_SET_REQ        = 0x0088,
    
    ID_AT_MTA_CCLK_QRY_REQ              = 0x0089,


    ID_AT_MTA_GAME_MODE_SET_REQ         = 0x008A,           /* _H2ASN_MsgChoice AT_MTA_COMM_GAME_MODE_SET_STRU */
    ID_AT_MTA_PSEUDBTS_SET_REQ          = 0x008B,

    /* MTA发给AT的消息 */
    ID_MTA_AT_CPOS_SET_CNF              = 0x1000,           /* _H2ASN_MsgChoice MTA_AT_CPOS_CNF_STRU        */
    ID_MTA_AT_CGPSCLOCK_SET_CNF         = 0x1001,           /* _H2ASN_MsgChoice MTA_AT_CGPSCLOCK_CNF_STRU   */
    ID_MTA_AT_CPOSR_IND                 = 0x1002,           /* _H2ASN_MsgChoice MTA_AT_CPOSR_IND_STRU       */
    ID_MTA_AT_XCPOSRRPT_IND             = 0x1003,           /* _H2ASN_MsgChoice MTA_AT_XCPOSRRPT_IND_STRU   */
    ID_MTA_AT_APSEC_SET_CNF             = 0x1004,           /* _H2ASN_MsgChoice MTA_AT_APSEC_CNF_STRU       */
    ID_MTA_AT_SIMLOCKUNLOCK_SET_CNF     = 0x1005,           /* _H2ASN_MsgChoice MTA_AT_SIMLOCKUNLOCK_CNF_STRU */

    ID_MTA_AT_QRY_NMR_CNF               = 0x1006,           /* _H2ASN_MsgChoice MTA_AT_QRY_NMR_CNF_STRU */

    ID_MTA_AT_WRR_AUTOTEST_QRY_CNF      = 0x1007,           /* _H2ASN_MsgChoice MTA_AT_WRR_AUTOTEST_QRY_CNF_STRU       */
    ID_MTA_AT_WRR_CELLINFO_QRY_CNF      = 0x1008,           /* _H2ASN_MsgChoice MTA_AT_WRR_CELLINFO_QRY_CNF_STRU       */
    ID_MTA_AT_WRR_MEANRPT_QRY_CNF       = 0x1009,           /* _H2ASN_MsgChoice MTA_AT_WRR_MEANRPT_QRY_CNF_STRU        */
    ID_MTA_AT_WRR_FREQLOCK_SET_CNF      = 0x100A,           /* _H2ASN_MsgChoice MTA_AT_WRR_FREQLOCK_SET_CNF_STRU       */
    ID_MTA_AT_WRR_RRC_VERSION_SET_CNF   = 0x100B,           /* _H2ASN_MsgChoice MTA_AT_WRR_RRC_VERSION_SET_CNF_STRU    */
    ID_MTA_AT_WRR_CELLSRH_SET_CNF       = 0x100C,           /* _H2ASN_MsgChoice MTA_AT_WRR_CELLSRH_SET_CNF_STRU        */
    ID_MTA_AT_WRR_FREQLOCK_QRY_CNF      = 0x100D,           /* _H2ASN_MsgChoice MTA_AT_WRR_FREQLOCK_QRY_CNF_STRU       */
    ID_MTA_AT_WRR_RRC_VERSION_QRY_CNF   = 0x100E,           /* _H2ASN_MsgChoice MTA_AT_WRR_RRC_VERSION_QRY_CNF_STRU    */
    ID_MTA_AT_WRR_CELLSRH_QRY_CNF       = 0x100F,           /* _H2ASN_MsgChoice MTA_AT_WRR_CELLSRH_QRY_CNF_STRU        */

    ID_MTA_AT_BODY_SAR_SET_CNF          = 0x1010,           /* _H2ASN_MsgChoice MTA_AT_RESULT_CNF_STRU */

    ID_MTA_AT_CURC_QRY_CNF              = 0x1011,           /* _H2ASN_MsgChoice MTA_AT_CURC_QRY_CNF_STRU */
    ID_MTA_AT_UNSOLICITED_RPT_SET_CNF   = 0x1012,           /* _H2ASN_MsgChoice MTA_AT_UNSOLICITED_RPT_SET_CNF_STRU */
    ID_MTA_AT_UNSOLICITED_RPT_QRY_CNF   = 0x1013,           /* _H2ASN_MsgChoice MTA_AT_UNSOLICITED_RPT_QRY_CNF_STRU */

    ID_MTA_AT_IMEI_VERIFY_QRY_CNF       = 0x1014,            /* _H2ASN_MsgChoice AT_MTA_RESERVE_STRU */
    ID_MTA_AT_CGSN_QRY_CNF              = 0x1015,            /* _H2ASN_MsgChoice MTA_AT_CGSN_QRY_CNF_STRU */
    ID_MTA_AT_NCELL_MONITOR_SET_CNF     = 0x1016,           /* _H2ASN_MsgChoice MTA_AT_RESULT_CNF_STRU        */
    ID_MTA_AT_NCELL_MONITOR_QRY_CNF     = 0x1017,           /* _H2ASN_MsgChoice MTA_AT_NCELL_MONITOR_QRY_CNF_STRU        */
    ID_MTA_AT_NCELL_MONITOR_IND         = 0x1018,           /* _H2ASN_MsgChoice MTA_AT_NCELL_MONITOR_IND_STRU        */

    ID_MTA_AT_REFCLKFREQ_SET_CNF        = 0x1019,           /* _H2ASN_MsgChoice MTA_AT_RESULT_CNF_STRU */
    ID_MTA_AT_REFCLKFREQ_QRY_CNF        = 0x101A,           /* _H2ASN_MsgChoice MTA_AT_REFCLKFREQ_QRY_CNF_STRU */
    ID_MTA_AT_REFCLKFREQ_IND            = 0x101B,           /* _H2ASN_MsgChoice MTA_AT_REFCLKFREQ_IND_STRU */

    ID_MTA_AT_HANDLEDECT_SET_CNF        = 0x101C,           /* _H2ASN_MsgChoice MTA_AT_HANDLEDECT_SET_CNF_STRU */
    ID_MTA_AT_HANDLEDECT_QRY_CNF        = 0x101D,           /* _H2ASN_MsgChoice MTA_AT_HANDLEDECT_QRY_CNF_STRU */

    ID_MTA_AT_PS_TRANSFER_IND           = 0x101E,           /* _H2ASN_MsgChoice MTA_AT_PS_TRANSFER_IND_STRU */

    ID_MTA_AT_ECID_SET_CNF              = 0x101F,           /* _H2ASN_MsgChoice MTA_AT_ECID_SET_CNF_STRU */

    ID_MTA_AT_MIPICLK_QRY_CNF           = 0x1020,           /* _H2ASN_MsgChoice MTA_AT_RF_LCD_MIPICLK_CNF_STRU */
    ID_MTA_AT_MIPICLK_INFO_IND          = 0x1021,           /* _H2ASN_MsgChoice MTA_AT_RF_LCD_MIPICLK_IND_STRU */

    ID_MTA_AT_SET_DPDTTEST_FLAG_CNF     = 0x1022,           /* _H2ASN_MsgChoice MTA_AT_SET_DPDTTEST_FLAG_CNF_STRU */
    ID_MTA_AT_SET_DPDT_VALUE_CNF        = 0x1023,           /* _H2ASN_MsgChoice MTA_AT_SET_DPDT_VALUE_CNF_STRU */
    ID_MTA_AT_QRY_DPDT_VALUE_CNF        = 0x1024,           /* _H2ASN_MsgChoice MTA_AT_QRY_DPDT_VALUE_CNF_STRU */

    ID_MTA_AT_RRC_PROTECT_PS_CNF        = 0x1025,
    ID_MTA_AT_PHY_INIT_CNF              = 0x1026,

    ID_MTA_AT_SET_JAM_DETECT_CNF        = 0x1027,           /* _H2ASN_MsgChoice MTA_AT_SET_JAM_DETECT_CNF_STRU */
    ID_MTA_AT_JAM_DETECT_IND            = 0x1028,           /* _H2ASN_MsgChoice MTA_AT_JAM_DETECT_IND_STRU */

    ID_MTA_AT_SET_FREQ_LOCK_CNF         = 0x1029,           /* _H2ASN_MsgChoice MTA_AT_SET_FREQ_LOCK_CNF_STRU */

    ID_MTA_AT_SET_GSM_FREQLOCK_CNF      = 0x102A,           /* _H2ASN_MsgChoice MTA_AT_SET_GSM_FREQLOCK_CNF_STRU */

    ID_AT_MTA_SET_FEMCTRL_CNF           = 0x102B,

    ID_MTA_AT_XPASS_INFO_IND            = 0x102C,

    ID_MTA_AT_NVWRSECCTRL_SET_CNF       = 0x102D,

    ID_MTA_AT_MBMS_SERVICE_OPTION_SET_CNF   = 0x102E,       /* _H2ASN_MsgChoice MTA_AT_RESULT_CNF_STRU */
    ID_MTA_AT_MBMS_PREFERENCE_SET_CNF       = 0x102F,       /* _H2ASN_MsgChoice MTA_AT_RESULT_CNF_STRU */
    ID_MTA_AT_SIB16_NETWORK_TIME_QRY_CNF    = 0x1030,       /* _H2ASN_MsgChoice MTA_AT_MBMS_SIB16_NETWORK_TIME_QRY_CNF_STRU */
    ID_MTA_AT_BSSI_SIGNAL_LEVEL_QRY_CNF     = 0x1031,       /* _H2ASN_MsgChoice MTA_AT_MBMS_BSSI_SIGNAL_LEVEL_QRY_CNF_STRU */
    ID_MTA_AT_NETWORK_INFO_QRY_CNF          = 0x1032,       /* _H2ASN_MsgChoice MTA_AT_MBMS_NETWORK_INFO_QRY_CNF_STRU */
    ID_MTA_AT_EMBMS_STATUS_QRY_CNF          = 0x1033,       /* _H2ASN_MsgChoice MTA_AT_EMBMS_STATUS_QRY_CNF_STRU */
    ID_MTA_AT_MBMS_UNSOLICITED_CFG_SET_CNF  = 0x1034,       /* _H2ASN_MsgChoice MTA_AT_RESULT_CNF_STRU */
    ID_MTA_AT_MBMS_SERVICE_EVENT_IND        = 0x1035,       /* _H2ASN_MsgChoice MTA_AT_MBMS_SERVICE_EVENT_IND_STRU */
    ID_MTA_AT_LTE_LOW_POWER_SET_CNF         = 0x1036,       /* _H2ASN_MsgChoice MTA_AT_RESULT_CNF_STRU */
    ID_MTA_AT_INTEREST_LIST_SET_CNF         = 0x1037,       /* _H2ASN_MsgChoice MTA_AT_RESULT_CNF_STRU */
    ID_MTA_AT_MBMS_SERVICE_STATE_SET_CNF    = 0x1038,       /* _H2ASN_MsgChoice MTA_AT_RESULT_CNF_STRU */
    ID_MTA_AT_MBMS_AVL_SERVICE_LIST_QRY_CNF = 0x1039,       /* _H2ASN_MsgChoice MTA_AT_MBMS_AVL_SERVICE_LIST_QRY_CNF_STRU */
    ID_MTA_AT_LTE_WIFI_COEX_SET_CNF         = 0x103A,       /* _H2ASN_MsgChoice MTA_AT_RESULT_CNF_STRU */
    ID_MTA_AT_LTE_WIFI_COEX_QRY_CNF         = 0x103B,       /* _H2ASN_MsgChoice MTA_AT_LTE_WIFI_COEX_QRY_CNF_STRU */

    ID_MTA_AT_SET_FR_CNF                = 0x103C,

    ID_MTA_AT_MEID_SET_CNF              = 0x103d,     /* _H2ASN_MsgChoice MTA_AT_RESULT_CNF_STRU */

    ID_MTA_AT_RFICSSIRD_QRY_CNF         = 0x103e,

    ID_MTA_AT_MEID_QRY_CNF              = 0x103f,     /* _H2ASN_MsgChoice MTA_AT_MEID_QRY_CNF_STRU */

#if (FEATURE_ON == FEATURE_PROBE_FREQLOCK)
    ID_MTA_AT_SET_M2M_FREQLOCK_CNF      = 0x1040,           /* _H2ASN_MsgChoice AT_MTA_SET_M2M_FREQLOCK_CNF_STRU */
    ID_MTA_AT_QRY_M2M_FREQLOCK_CNF      = 0x1041,           /* _H2ASN_MsgChoice MTA_AT_QRY_FREQLOCK_CNF_STRU */
#endif

    ID_MTA_AT_TRANSMODE_QRY_CNF         = 0x1052,

    ID_MTA_AT_UE_CENTER_SET_CNF         = 0x1053,
    ID_MTA_AT_UE_CENTER_QRY_CNF         = 0x1054,     /* _H2ASN_MsgChoice MTA_AT_QRY_UE_CENTER_CNF_STRU */

    ID_MTA_AT_SET_NETMON_SCELL_CNF      = 0x1055,     /* _H2ASN_MsgChoice MTA_AT_NETMON_CELL_INFO_STRU */
    ID_MTA_AT_SET_NETMON_NCELL_CNF      = 0x1056,     /* _H2ASN_MsgChoice MTA_AT_NETMON_CELL_INFO_STRU */

    ID_AT_MTA_QRY_AFC_CLK_FREQ_XOCOEF_REQ = 0x1057, /* _H2ASN_MsgChoice AT_MTA_QRY_AFC_CLK_FREQ_XOCOEF_REQ_STRU */
    ID_MTA_AT_QRY_AFC_CLK_FREQ_XOCOEF_CNF = 0x1058, /* _H2ASN_MsgChoice MTA_AT_QRY_AFC_CLK_FREQ_XOCOEF_CNF_STRU */
    ID_MTA_AT_ANQUERY_QRY_CNF           = 0x1059,       /* _H2ASN_MsgChoice MTA_AT_ANTENNA_INFO_QRY_CNF_STRU */
    ID_MTA_AT_CSNR_QRY_CNF              = 0x105A,       /* _H2ASN_MsgChoice MTA_AT_CSNR_QRY_CNF_STRU */
    ID_MTA_AT_CSQLVL_QRY_CNF            = 0x105B,       /* _H2ASN_MsgChoice MTA_AT_CSQLVL_QRY_CNF_STRU */

    ID_MTA_AT_EVDO_SYS_EVENT_CNF        = 0x105c,     /* _H2ASN_MsgChoice MTA_AT_RESULT_CNF_STRU */
    ID_MTA_AT_EVDO_SIG_MASK_CNF         = 0x105d,     /* _H2ASN_MsgChoice MTA_AT_RESULT_CNF_STRU */
    ID_MTA_AT_EVDO_REVA_RLINK_INFO_IND  = 0x105e,
    ID_MTA_AT_EVDO_SIG_EXEVENT_IND      = 0x105f,

    ID_MTA_AT_XCPOSR_SET_CNF            = 0x1060,                               /* _H2ASN_MsgChoice MTA_AT_RESULT_CNF_STRU */
    ID_MTA_AT_XCPOSR_QRY_CNF            = 0x1061,                               /* _H2ASN_MsgChoice MTA_AT_QRY_XCPOSR_CNF_STRU */
    ID_MTA_AT_XCPOSRRPT_SET_CNF         = 0x1062,                               /* _H2ASN_MsgChoice MTA_AT_RESULT_CNF_STRU */
    ID_MTA_AT_XCPOSRRPT_QRY_CNF         = 0x1063,                               /* _H2ASN_MsgChoice MTA_AT_QRY_XCPOSRRPT_CNF_STRU */
    ID_MTA_AT_CLEAR_HISTORY_FREQ_CNF    = 0x1064,   /* _H2ASN_MsgChoice MTA_AT_RESULT_CNF_STRU */

    ID_MTA_AT_SET_SENSOR_CNF           = 0x1065,     /* _H2ASN_MsgChoice MTA_AT_SET_SENSOR_CNF_STRU */
    ID_MTA_AT_SET_SCREEN_CNF           = 0x1066,     /* _H2ASN_MsgChoice MTA_AT_SET_SCREEN_CNF_STRU */
    ID_MTA_AT_FRAT_IGNITION_QRY_CNF     = 0x1067,                               /* _H2ASN_MsgChoice MTA_AT_FRAT_IGNITION_QRY_CNF_STRU */
    ID_MTA_AT_FRAT_IGNITION_SET_CNF     = 0x1068,                               /* _H2ASN_MsgChoice MTA_AT_FRAT_IGNITION_SET_CNF_STRU */

    ID_MTA_AT_SET_MODEM_TIME_CNF              = 0x1069,

    ID_MTA_AT_RX_TEST_MODE_SET_CNF      = 0x106A,

    ID_MTA_AT_AFC_CLK_UNLOCK_CAUSE_IND  = 0x106B,

    ID_MTA_AT_SIB16_TIME_UPDATE_IND     = 0x106C,

    ID_MTA_AT_SLAVE_SET_CNF             = 0x106D,

    ID_MTA_AT_BESTFREQ_SET_CNF          = 0x106E,
    ID_MTA_AT_BEST_FREQ_INFO_IND        = 0x106F,                             /* _H2ASN_MsgChoice MTA_AT_BEST_FREQ_CASE_STRU */
    ID_MTA_AT_BESTFREQ_QRY_CNF          = 0x1070,

    ID_MTA_AT_RFIC_DIE_ID_QRY_CNF       = 0x1071,

    ID_MTA_AT_CRRCONN_SET_CNF           = 0x1072,
    ID_MTA_AT_CRRCONN_QRY_CNF           = 0x1073,
    ID_MTA_AT_CRRCONN_STATUS_IND        = 0x1074,
    ID_MTA_AT_VTRLQUALRPT_SET_CNF       = 0x1075,
    ID_MTA_AT_RL_QUALITY_INFO_IND       = 0x1076,
    ID_MTA_AT_VIDEO_DIAG_INFO_RPT       = 0x1077,

    ID_MTA_AT_PMU_DIE_SN_QRY_CNF        = 0x1078,

    ID_MTA_AT_MODEM_CAP_UPDATE_CNF      = 0x1079,

    ID_MTA_AT_LTE_CATEGORY_INFO_IND     = 0x107A,                               /* _H2ASN_MsgChoice MTA_AT_LTE_CATEGORY_INFO_IND_STRU */

    ID_MTA_AT_TAS_TEST_CFG_CNF          = 0x1080,
    ID_MTA_AT_TAS_TEST_QRY_CNF          = 0x1081,

    ID_MTA_AT_ACCESS_STRATUM_REL_IND    = 0x1082,

    ID_MTA_AT_MIPI_WREX_CNF             = 0x1083,
    ID_MTA_AT_MIPI_RDEX_CNF             = 0x1084,

    ID_MTA_AT_RS_INFO_QRY_CNF           = 0x1085,

    ID_MTA_AT_PHY_COM_CFG_SET_CNF       = 0x1086,

    ID_MTA_AT_ERRCCAP_CFG_SET_CNF       = 0x1087,
    ID_MTA_AT_ERRCCAP_QRY_SET_CNF       = 0x1088,

    ID_MTA_AT_QRY_GSM_FREQLOCK_CNF      = 0x1089,                               /* _H2ASN_MsgChoice MTA_AT_QRY_GSM_FREQLOCK_CNF_STRU */
    ID_MTA_AT_DEL_CELLENTITY_CNF        = 0x108A,

    ID_MTA_AT_POWER_DET_QRY_CNF         = 0x108B,


    ID_MTA_AT_PSEUCELL_INFO_SET_CNF     = 0x108C,

    ID_MTA_AT_MIPIREAD_SET_CNF          = 0x108D,

    ID_MTA_AT_NO_CARD_MODE_SET_CNF      = 0x108E,
    ID_MTA_AT_NO_CARD_MODE_QRY_CNF      = 0x108F,

    ID_MTA_AT_CHRALARMRLAT_CFG_SET_CNF  = 0x1090,

    ID_MTA_AT_ECC_CFG_SET_CNF           = 0x1091,
    ID_MTA_AT_ECC_STATUS_IND            = 0x1092,

    ID_MTA_AT_EPDU_SET_CNF              = 0x1093,
    ID_MTA_AT_EPDUR_DATA_IND            = 0x1094,

    ID_MTA_AT_LTE_CA_CFG_SET_CNF        = 0x1095,
    ID_MTA_AT_LTE_CA_CELLEX_QRY_CNF     = 0x1096,
    ID_MTA_AT_LTE_CA_CELLEX_INFO_NTF    = 0x1097,

    ID_MTA_AT_LTE_CA_CELL_RPT_CFG_SET_CNF = 0x1098,
    ID_MTA_AT_LTE_CA_CELL_RPT_CFG_QRY_CNF = 0x1099,

    ID_MTA_AT_LTE_FINE_TIME_SET_CNF     = 0x109A,
    ID_MTA_AT_SIB_FINE_TIME_NTF         = 0x109B,
    ID_MTA_AT_LPP_FINE_TIME_NTF         = 0x109C,

    ID_MTA_AT_PHY_COMM_ACK_IND          = 0x109D,

    ID_MTA_AT_GAS_AUTOTEST_QRY_CNF      = 0x109E,                   /* _H2ASN_MsgChoice MTA_AT_GAS_AUTOTEST_QRY_CNF_STRU */

    ID_MTA_AT_COMM_BOOSTER_SET_CNF     = 0x109F,  /* _H2ASN_MsgChoice MTA_AT_COMM_BOOSTER_SET_CNF_STRU */
    ID_MTA_AT_COMM_BOOSTER_IND         = 0x10A0,  /* _H2ASN_MsgChoice MTA_AT_COMM_BOOSTER_IND_STRU */
    ID_MTA_AT_COMM_BOOSTER_QUERY_CNF   = 0x10A1,  /* _H2ASN_MsgChoice MTA_AT_COMM_BOOSTER_QUERY_CNF_STRU */

    ID_MTA_AT_NVLOAD_SET_CNF           = 0x10A2,

    ID_MTA_AT_SMS_DOMAIN_SET_CNF       = 0x10A3,          /* _H2ASN_MsgChoice MTA_AT_SMS_DOMAIN_SET_CNF_STRU */
    ID_MTA_AT_SMS_DOMAIN_QRY_CNF       = 0x10A4,          /* _H2ASN_MsgChoice MTA_AT_SMS_DOMAIN_QRY_CNF_STRU */

    ID_MTA_AT_EPS_NETWORK_QRY_CNF       = 0x10A5,                               /* _H2ASN_MsgChoice MTA_AT_EPS_NETWORK_QRY_CNF_STRU */
    ID_MTA_AT_EPS_NETWORK_INFO_IND      = 0x10A6,                               /* _H2ASN_MsgChoice MTA_AT_EPS_NETWORK_INFO_IND_STRU */

    ID_MTA_AT_SET_SAMPLE_CNF           = 0x10A7,

    ID_MTA_AT_GPS_LOCSET_SET_CNF       = 0x10A8,
    
    ID_MTA_AT_CCLK_QRY_CNF         = 0x10A9,
   
    ID_MTA_AT_TEMP_PROTECT_IND         = 0x10AA,

    ID_AT_MTA_GAME_MODE_SET_CNF         = 0x10AB,           /* _H2ASN_MsgChoice MTA_AT_GAME_MODE_SET_CFN_STRU */

    ID_MTA_AT_PSEUDBTS_SET_CNF          = 0x10AC,
    ID_MTA_AT_PSEUD_BTS_IDENT_IND       = 0x1102,
    /* 最后一条消息 */
    ID_AT_MTA_MSG_TYPE_BUTT

};
typedef VOS_UINT32 AT_MTA_MSG_TYPE_ENUM_UINT32;


enum MTA_AT_RESULT_ENUM
{
    /* 与AT模块对应的标准错误码 */
    MTA_AT_RESULT_NO_ERROR                      = 0x000000,                     /* 消息处理正常 */
    MTA_AT_RESULT_ERROR,                                                        /* 消息处理出错 */
    MTA_AT_RESULT_INCORRECT_PARAMETERS,
    MTA_AT_RESULT_OPTION_TIMEOUT,
    MTA_AT_RESULT_FUNC_DISABLE,                                                 /* 消息处理功能关闭 */
    /* 预留对应AT标准命令错误码 */

    /* 装备命令特有错误码 */
    MTA_AT_RESULT_DEVICE_ERROR_BASE             = 0x100000,
    MTA_AT_RESULT_DEVICE_SEC_IDENTIFY_FAIL,                                     /* 产线鉴权失败 */
    MTA_AT_RESULT_DEVICE_SEC_SIGNATURE_FAIL,                                    /* 签名校验失败 */
    MTA_AT_RESULT_DEVICE_SEC_DK_INCORRECT,                                      /* 端口密码错误 */
    MTA_AT_RESULT_DEVICE_SEC_UNLOCK_KEY_INCORRECT,                              /* 解锁密码错误 */
    MTA_AT_RESULT_DEVICE_SEC_PH_PHYNUM_LEN_ERROR,                               /* 物理号长度错误 */
    MTA_AT_RESULT_DEVICE_SEC_PH_PHYNUM_VALUE_ERROR,                             /* 物理号码错误 */
    MTA_AT_RESULT_DEVICE_SEC_PH_PHYNUM_TYPE_ERROR,                              /* 物理号类型错误 */
    MTA_AT_RESULT_DEVICE_SEC_RSA_ENCRYPT_FAIL,                                  /* RSA加密失败 */
    MTA_AT_RESULT_DEVICE_SEC_RSA_DECRYPT_FAIL,                                  /* RSA解密失败 */
    MTA_AT_RESULT_DEVICE_SEC_GET_RAND_NUMBER_FAIL,                              /* 获取随机数失败(crypto_rand) */
    MTA_AT_RESULT_DEVICE_SEC_WRITE_HUK_FAIL,                                    /* HUK写入错误 */
    MTA_AT_RESULT_DEVICE_SEC_FLASH_ERROR,                                       /* Flash错误 */
    MTA_AT_RESULT_DEVICE_SEC_NV_ERROR,                                          /* NV读写错误 */
    MTA_AT_RESULT_DEVICE_SEC_OTHER_ERROR,                                       /* 其它错误 */


    /* 私有命令特有错误码 */
    MTA_AT_RESULT_PRICMD_ERROR_BASE             = 0x200000,

    MTA_AT_RESULT_BUTT
};
typedef VOS_UINT32 MTA_AT_RESULT_ENUM_UINT32;


enum MTA_AT_CGPSCLOCK_ENUM
{
    MTA_AT_CGPSCLOCK_STOP           = 0,
    MTA_AT_CGPSCLOCK_START,
    MTA_AT_CGPSCLOCK_BUTT
};
typedef VOS_UINT32 MTA_AT_CGPSCLOCK_ENUM_UINT32;



enum MTA_AT_CPOS_OPERATE_TYPE_ENUM
{
    MTA_AT_CPOS_SEND                = 0,
    MTA_AT_CPOS_CANCEL,
    MTA_AT_CPOS_BUTT
};
typedef VOS_UINT32 MTA_AT_CPOS_OPERATE_TYPE_ENUM_UINT32;


enum AT_MTA_PERS_CATEGORY_ENUM
{
    AT_MTA_PERS_CATEGORY_NETWORK                    = 0x00,                     /* Category: 锁网 */
    AT_MTA_PERS_CATEGORY_NETWORK_SUBSET             = 0x01,                     /* Category: 锁子网 */
    AT_MTA_PERS_CATEGORY_SERVICE_PROVIDER           = 0x02,                     /* Category: 锁SP */
    AT_MTA_PERS_CATEGORY_CORPORATE                  = 0x03,                     /* Category: 锁团体 */
    AT_MTA_PERS_CATEGORY_SIM_USIM                   = 0x04,                     /* Category: 锁(U)SIM卡 */

    AT_MTA_PERS_CATEGORY_BUTT
};
typedef VOS_UINT8 AT_MTA_PERS_CATEGORY_ENUM_UINT8;


enum AT_MTA_RPT_GENERAL_CTRL_TYPE_ENUM
{
    AT_MTA_RPT_GENERAL_CONTROL_NO_REPORT      = 0x00,                           /* 禁止所有的主动上报 */
    AT_MTA_RPT_GENERAL_CONTROL_REPORT         = 0x01,                           /* 打开所有的主动上报 */
    AT_MTA_RPT_GENERAL_CONTROL_CUSTOM         = 0x02,                           /* 按BIT位控制对应命令的主动上报 */

    AT_MTA_RPT_GENERAL_CONTROL_BUTT
};
typedef VOS_UINT8 AT_MTA_RPT_GENERAL_CTRL_TYPE_ENUM_UINT8;


enum AT_MTA_RPT_SET_TYPE_ENUM
{
    AT_MTA_SET_MODE_RPT_TYPE            = 0,                                 /* Category: 设置模式变化是否主动上报 */
    AT_MTA_SET_SRVST_RPT_TYPE,
    AT_MTA_SET_RSSI_RPT_TYPE,
    AT_MTA_SET_TIME_RPT_TYPE,
    AT_MTA_SET_CTZR_RPT_TYPE,
    AT_MTA_SET_SIMST_RPT_TYPE,
    AT_MTA_SET_CREG_RPT_TYPE,
    AT_MTA_SET_CGREG_RPT_TYPE,
    AT_MTA_SET_CEREG_RPT_TYPE,
    AT_MTA_SET_CSID_RPT_TYPE,
    AT_MTA_SET_CLOCINFO_RPT_TYPE,
    AT_MTA_SET_MTREATTACH_RPT_TYPE,
    AT_MTA_SET_CENFS_RPT_TYPE,
    AT_MTA_SET_RPT_TYPE_BUTT
};
typedef VOS_UINT32 AT_MTA_RPT_SET_TYPE_ENUM_UINT32;


enum AT_MTA_RPT_QRY_TYPE_ENUM
{
    AT_MTA_QRY_TIME_RPT_TYPE = 0,
    AT_MTA_QRY_CTZR_RPT_TYPE,
    AT_MTA_QRY_CSSN_RPT_TYPE,
    AT_MTA_QRY_CUSD_RPT_TYPE,
    AT_MTA_QRY_RPT_TYPE_BUTT
};
typedef VOS_UINT32 AT_MTA_RPT_QRY_TYPE_ENUM_UINT32;



enum AT_MTA_CMD_RPT_FLG_ENUM
{
    AT_MTA_CMD_RPT_FLG_OFF              = 0,                        /* AT命令不主动上报 */
    AT_MTA_CMD_RPT_FLG_ON,                                          /* AT命令主动上报 */
    AT_MTA_CMD_RPT_FLG_BUTT
};
typedef VOS_UINT8 AT_MTA_CMD_RPT_FLG_ENUM_UINT8;

enum AT_MTA_CLEAR_FREQ_FLG_ENUM
{
    AT_MTA_CLEAR_FREQ_FLG_NOT_CSG_HISTORY_FREQ              = 0,                /* 清除非CSG历史频点 */
    AT_MTA_CLEAR_FREQ_FLG_CSG_HISTORY_FREQ,                                     /* 清除CSG历史频点 */
    AT_MTA_CLEAR_FREQ_FLG_ALL_FREQ,                                         /* 清除所有历史频点 */
    AT_MTA_CLEAR_FREQ_FLG_BUTT
};
typedef VOS_UINT8 AT_MTA_CLEAR_FREQ_FLG_ENUM_UINT8;



enum AT_MTA_CMD_RATMODE_ENUM
{
    AT_MTA_CMD_RATMODE_GSM              = 0,
    AT_MTA_CMD_RATMODE_WCDMA,
    AT_MTA_CMD_RATMODE_LTE,
    AT_MTA_CMD_RATMODE_TD,
    AT_MTA_CMD_RATMODE_CDMA,
    AT_MTA_CMD_RATMODE_BUTT
};
typedef VOS_UINT8 AT_MTA_CMD_RATMODE_ENUM_UINT8;


enum MTA_AT_JAM_RESULT_ENUM
{
    MTA_AT_JAM_RESULT_JAM_DISAPPEARED        = 0x00,                            /* 状态正常无干扰 */
    MTA_AT_JAM_RESULT_JAM_DISCOVERED,                                           /* 干扰存在 */

    MTA_AT_JAM_RESULT_BUTT
};
typedef VOS_UINT32 MTA_AT_JAM_RESULT_ENUM_UINT32;


enum AT_MTA_FREQLOCK_RATMODE_ENUM
{
    AT_MTA_FREQLOCK_RATMODE_GSM         = 0x01,
    AT_MTA_FREQLOCK_RATMODE_WCDMA,
    AT_MTA_FREQLOCK_RATMODE_LTE,
    AT_MTA_FREQLOCK_RATMODE_CDMA1X,
    AT_MTA_FREQLOCK_RATMODE_TD,
    AT_MTA_FREQLOCK_RATMODE_WIMAX,
    AT_MTA_FREQLOCK_RATMODE_EVDO,

    AT_MTA_FREQLOCK_RATMODE_BUTT
};
typedef VOS_UINT8 AT_MTA_FREQLOCK_RATMODE_ENUM_UINT8;


enum AT_MTA_GSM_BAND_ENUM
{
    AT_MTA_GSM_BAND_850                 = 0x00,
    AT_MTA_GSM_BAND_900,
    AT_MTA_GSM_BAND_1800,
    AT_MTA_GSM_BAND_1900,

    AT_MTA_GSM_BAND_BUTT
};
typedef VOS_UINT16 AT_MTA_GSM_BAND_ENUM_UINT16;


enum AT_MTA_CFG_ENUM
{
    AT_MTA_CFG_DISABLE                  = 0,
    AT_MTA_CFG_ENABLE                   = 1,
    AT_MTA_CFG_BUTT
};
typedef VOS_UINT8 AT_MTA_CFG_ENUM_UINT8;


enum AT_MTA_MBMS_SERVICE_STATE_SET_ENUM
{
    AT_MTA_MBMS_SERVICE_STATE_SET_ACTIVE       = 0,
    AT_MTA_MBMS_SERVICE_STATE_SET_DEACTIVE     = 1,
    AT_MTA_MBMS_SERVICE_STATE_SET_DEACTIVE_ALL = 2,
    AT_MTA_MBMS_SERVICE_STATE_SET_BUTT
};
typedef VOS_UINT8 AT_MTA_MBMS_SERVICE_STATE_SET_ENUM_UINT8;


enum AT_MTA_MBMS_CAST_MODE_ENUM
{
    AT_MTA_MBMS_CAST_MODE_UNICAST       = 0,
    AT_MTA_MBMS_CAST_MODE_MULTICAST     = 1,
    AT_MTA_MBMS_CAST_MODE_BUTT
};
typedef VOS_UINT8 AT_MTA_MBMS_CAST_MODE_ENUM_UINT8;


enum MTA_AT_EMBMS_FUNTIONALITY_STATUS_ENUM
{
    MTA_AT_EMBMS_FUNTIONALITY_OFF       = 0,
    MTA_AT_EMBMS_FUNTIONALITY_ON        = 1,
    MTA_AT_EMBMS_FUNTIONALITY_STATUS_BUTT
};
typedef VOS_UINT8 MTA_AT_EMBMS_FUNTIONALITY_STATUS_ENUM_UINT8;


enum MTA_AT_MBMS_SERVICE_EVENT_ENUM
{
    MTA_AT_MBMS_SERVICE_EVENT_SERVICE_CHANGE       = 0,
    MTA_AT_MBMS_SERVICE_EVENT_NO_SERVICE           = 1,
    MTA_AT_MBMS_SERVICE_EVENT_ONLY_UNICAST         = 2,
    MTA_AT_MBMS_SERVICE_EVENT_SERVICE_AVAILABLE    = 3,
    MTA_AT_MBMS_SERVICE_EVENT_BUTT
};
typedef VOS_UINT8 MTA_AT_MBMS_SERVICE_EVENT_ENUM_UINT8;


enum AT_MTA_COEX_BW_TYPE_ENUM
{
    AT_MTA_COEX_BAND_WIDTH_6RB          = 0,                                    /* 宽带1.4M */
    AT_MTA_COEX_BAND_WIDTH_15RB         = 1,                                    /* 宽带3M */
    AT_MTA_COEX_BAND_WIDTH_25RB         = 2,                                    /* 宽带5M */
    AT_MTA_COEX_BAND_WIDTH_50RB         = 3,                                    /* 宽带10M */
    AT_MTA_COEX_BAND_WIDTH_75RB         = 4,                                    /* 宽带15M */
    AT_MTA_COEX_BAND_WIDTH_100RB        = 5,                                    /* 宽带20M */
    AT_MTA_COEX_BAND_WIDTH_BUTT
};
typedef VOS_UINT16 AT_MTA_COEX_BW_TYPE_ENUM_UINT16;


enum AT_MTA_COEX_CFG_ENUM
{
    AT_MTA_COEX_CFG_DISABLE             = 0,
    AT_MTA_COEX_CFG_ENABLE              = 1,
    AT_MTA_COEX_CFG_BUTT
};
typedef VOS_UINT16 AT_MTA_COEX_CFG_ENUM_UINT16;


enum AT_MTA_LTE_LOW_POWER_ENUM
{
    AT_MTA_LTE_LOW_POWER_NORMAL         = 0,
    AT_MTA_LTE_LOW_POWER_LOW            = 1,
    AT_MTA_LTE_LOW_POWER_BUTT
};
typedef VOS_UINT8 AT_MTA_LTE_LOW_POWER_ENUM_UINT8;


enum AT_MTA_MBMS_PRIORITY_ENUM
{
    AT_MTA_MBMS_PRIORITY_UNICAST        = 0,
    AT_MTA_MBMS_PRIORITY_MBMS           = 1,
    AT_MTA_MBMS_PRIORITY_BUTT
};
typedef VOS_UINT8 AT_MTA_MBMS_PRIORITY_ENUM_UINT8;


enum AT_MTA_UE_CENTER_TYPE_ENUM
{
    AT_MTA_UE_CENTER_VOICE_CENTRIC                          = 0,
    AT_MTA_UE_CENTER_DATA_CENTRIC                           = 1,
    AT_MTA_UE_CENTER_BUTT
};
typedef VOS_UINT32 AT_MTA_UE_CENTER_TYPE_ENUM_UINT32;


enum AT_MTA_BODY_SAR_STATE_ENUM
{
    AT_MTA_BODY_SAR_OFF                 = 0,                                    /* Body SAR功能关闭 */
    AT_MTA_BODY_SAR_ON,                                                         /* Body SAR功能开启 */
    AT_MTA_BODY_SAR_STATE_BUTT
};
typedef VOS_UINT16 AT_MTA_BODY_SAR_STATE_ENUM_UINT16;


enum MTA_AT_REFCLOCK_STATUS_ENUM
{
    MTA_AT_REFCLOCK_UNLOCKED            = 0,                                    /* GPS参考时钟频率非锁定状态 */
    MTA_AT_REFCLOCK_LOCKED,                                                     /* GPS参考时钟频率锁定状态 */
    MTA_AT_REFCLOCK_STATUS_BUTT
};
typedef VOS_UINT16 MTA_AT_REFCLOCK_STATUS_ENUM_UINT16;


enum MTA_AT_GPHY_XPASS_MODE_ENUM
{
    MTA_AT_GPHY_XPASS_MODE_DISABLE      = 0,                                    /* 非XPASS模式，类似高铁 */
    MTA_AT_GPHY_XPASS_MODE_ENABLE,                                              /* XPASS模式，类似高铁 */
    MTA_AT_GPHY_XPASS_MODE_BUTT
};
typedef VOS_UINT16 MTA_AT_GPHY_XPASS_MODE_ENUM_UINT16;


enum MTA_AT_WPHY_HIGHWAY_MODE_ENUM
{
    MTA_AT_WPHY_HIGHWAY_MODE_DISABLE    = 0,                                    /* W非高铁模式 */
    MTA_AT_WPHY_HIGHWAY_MODE_ENABLE,                                            /* W高铁模式 */
    MTA_AT_WPHY_HIGHWAY_MODE_BUTT
};
typedef VOS_UINT16 MTA_AT_WPHY_HIGHWAY_MODE_ENUM_UINT16;


enum MTA_AT_NETMON_GSM_STATE_ENUM
{
   MTA_AT_NETMON_GSM_STATE_INIT   = 0,
   MTA_AT_NETMON_GSM_STATTE_WAIT_CELL_INFO  = 1,
   MTA_AT_NETMON_GSM_STATTE_WAIT_TA  = 2,
   MTA_AT_NETMON_GSM_STATTE_BUTT
};

typedef VOS_UINT32 MTA_AT_NETMON_GSM_STATE_ENUM_UINT32;



enum MTA_AT_NETMON_CELL_INFO_RAT_ENUM
{
    MTA_AT_NETMON_CELL_INFO_GSM         = 0,
    MTA_AT_NETMON_CELL_INFO_UTRAN_FDD   = 1,
    MTA_AT_NETMON_CELL_INFO_UTRAN_TDD   = 2,
    MTA_AT_NETMON_CELL_INFO_LTE         = 3,
    MTA_AT_NETMON_CELL_INFO_RAT_BUTT
};
typedef VOS_UINT32 MTA_AT_NETMON_CELL_INFO_RAT_ENUM_UINT32;




enum MTA_TAF_CTZR_TYPE
{
    MTA_TAF_CTZR_OFF            = 0x00,            /* CTZR去使能 */
    MTA_TAF_CTZR_CTZV           = 0x01,            /* CTZR:CTZV */
    MTA_TAF_CTZR_CTZE           = 0x02,            /* CTZR:CTZE */
    MTA_TAF_CTZR_BUTT
};
typedef VOS_UINT32 MTA_TAF_CTZR_TYPE_ENUM_UINT32;


enum AT_MTA_AFC_CLK_STATUS_ENUM
{
    AT_MTA_AFC_CLK_UNLOCKED           = 0,                                     /* AFC UNLOCKED */
    AT_MTA_AFC_CLK_LOCKED,                                                     /* AFC LOCKED */

    AT_MTA_AFC_CLK_STATUS_BUTT
};
typedef VOS_UINT32 AT_MTA_AFC_CLK_STATUS_ENUM_UINT32;


enum AT_MTA_XCPOSR_CFG_ENUM
{
    AT_MTA_XCPOSR_CFG_DISABLE        = 0,                                       /* 不支持清除缓存 */
    AT_MTA_XCPOSR_CFG_ENABLE,                                                   /* 支持清除缓存 */
    AT_MTA_XCPOSR_CFG_BUTT
};
typedef VOS_UINT8 AT_MTA_XCPOSR_CFG_ENUM_UNIT8;


enum AT_MTA_FRAT_IGNITION_ENUM
{
    AT_MTA_FRAT_IGNITION_STATT_OFF        = 0,                                  /* 不支持清除缓存 */
    AT_MTA_FRAT_IGNITION_STATT_ON,                                                   /* 支持清除缓存 */
    AT_MTA_FRAT_IGNITION_STATT_BUTT
};
typedef VOS_UINT8 AT_MTA_FRAT_IGNITION_ENUM_UNIT8;



enum MTA_AT_AFC_CLK_UNLOCK_CAUSE_ENUM
{
    MTA_AT_AFC_CLK_UNLOCK_CAUSE_SLEEP,

    MTA_AT_AFC_CLK_UNLOCK_CAUSE_BUTT
};
typedef VOS_UINT16  MTA_AT_AFC_CLK_UNLOCK_CAUSE_ENUM_UINT16;


enum MTA_AT_MODEM_ID_ENUM
{
    MTA_AT_MODEM_ID_0  = 0,
    MTA_AT_MODEM_ID_1,
    MTA_AT_MODEM_ID_2,

    MTA_AT_MODEM_ID_BUTT
};
typedef VOS_UINT8 MTA_AT_MODEM_ID_ENUM_UINT8;


enum MTA_AT_RAT_MODE_ENUM
{
    MTA_AT_RAT_MODE_GSM       = 0,
    MTA_AT_RAT_MODE_WCDMA,
    MTA_AT_RAT_MODE_LTE,
    MTA_AT_RAT_MODE_TDSCDMA,
    MTA_AT_RAT_MODE_CDMA_1X,
    MTA_AT_RAT_MODE_HRPD,

    MTA_AT_RAT_MODE_BUTT
};
typedef VOS_UINT8 MTA_AT_RAT_MODE_ENUM_UINT8;


enum MTA_AT_DAY_LIGHT_SAVING_IND_ENUM
{
    MTA_AT_NO_ADJUST                  = 0,
    MTA_AT_ONE_HOUR                   = 1,
    MTA_AT_TWO_HOUR                   = 2,
    MTA_AT_HOUR_BUTT
};
typedef VOS_UINT8   MTA_AT_DAY_LIGHT_SAVING_IND_ENUM_UINT8;


enum MTA_AT_TIME_TYPE_ENUM
{
    MTA_AT_LOCAL_TIME                 = 1,
    MTA_AT_GPS_TIME                   = 2,
    MTA_AT_LOCAL_GPS_TIME             = 3,
    MTA_AT_TIME_BUTT
};
typedef VOS_UINT8   MTA_AT_TIME_TYPE_ENUM_UINT8;


enum AT_MTA_MODEM_CAP_UPDATE_TYPE_ENUM
{
    AT_MTA_MODEM_CAP_UPDATE_TYPE_CDMA_MODEM_SWITCH   = 0x00U,       /* AT^CDMAMODEMSWITCH导致的平台能力更新 */
    AT_MTA_MODEM_CAP_UPDATE_TYPE_ACTIVE_MODEM        = 0x01U,       /* AT^ACTIVEMODEM导致的平台能力更新 */

    AT_MTA_MODEM_CAP_UPDATE_TYPE_BUTT
};
typedef VOS_UINT8 AT_MTA_MODEM_CAP_UPDATE_TYPE_ENUM_UINT8;


enum AT_MTA_RS_INFO_TYPE_ENUM
{
    AT_MTA_RSRP_TYPE                     = 0,
    AT_MTA_RSRQ_TYPE                     = 1,
    AT_MTA_RS_INFO_TYPE_BUTT
};
typedef VOS_UINT32 AT_MTA_RS_INFO_TYPE_ENUM_UINT32;


enum AT_MTA_PSEUCELL_NOTIFY_TYPE_ENUM
{
    AT_MTA_PSEUCELL_NOTIFY_TYPE_BY_MESSAGE = 0,
    AT_MTA_PSEUCELL_NOTIFY_TYPE_BUTT
};
typedef VOS_UINT8 AT_MTA_PSEUCELL_NOTIFY_TYPE_ENUM_UINT8;


enum AT_MTA_SYS_MODE_ENUM
{
    AT_MTA_SYS_MODE_GSM                 = 1,
    AT_MTA_SYS_MODE_1X                  = 2,
    AT_MTA_SYS_MODE_WCDMA               = 3,
    AT_MTA_SYS_MODE_TDSCDMA             = 4,
    AT_MTA_SYS_MODE_WIMAX               = 5,
    AT_MTA_SYS_MODE_LTE                 = 6,
    AT_MTA_SYS_MODE_HRPD                = 7,
    AT_MTA_SYS_MODE_BUTT
};
typedef VOS_UINT32 AT_MTA_SYS_MODE_ENUM_UINT32;


enum AT_MTA_CHR_ALARM_RLAT_OP_ENUM
{
    AT_MTA_CHR_ALARM_RLAT_OP_READ       = 0,
    AT_MTA_CHR_ALARM_RLAT_OP_WRITE      = 1,

    AT_MTA_CHR_ALARM_RLAT_OP_BUTT
};
typedef VOS_UINT32 AT_MTA_CHR_ALARM_RLAT_OP_ENUM_UINT32;


enum MTA_AT_BAND_WIDTH_ENUM
{
    MTA_AT_BAND_WIDTH_1DOT4M                    = 0,            /* 带宽为1.4MHz */
    MTA_AT_BAND_WIDTH_3M,                                       /* 带宽为3MHz */
    MTA_AT_BAND_WIDTH_5M,                                       /* 带宽为5MHz */
    MTA_AT_BAND_WIDTH_10M,                                      /* 带宽为10MHz */
    MTA_AT_BAND_WIDTH_15M,                                      /* 带宽为15MHz */
    MTA_AT_BAND_WIDTH_20M,                                      /* 带宽为20MHz */
    MTA_AT_BAND_WIDTH_BUTT
};
typedef VOS_UINT8 MTA_AT_BAND_WIDTH_ENUM_UINT8;


enum MTA_AT_FINE_TIME_RAT_ENUM
{
    MTA_AT_FINE_TIME_RAT_GSM         = 0,
    MTA_AT_FINE_TIME_RAT_WCDMA       = 1,
    MTA_AT_FINE_TIME_RAT_LTE         = 2,
    MTA_AT_FINE_TIME_RAT_TDS_CDMA    = 3,
    MTA_AT_FINE_TIME_RAT_CDMA        = 4,
    MTA_AT_FINE_TIME_RAT_BUTT
};
typedef VOS_UINT8 MTA_AT_FINE_TIME_RAT_ENUM_UINT8;

/*****************************************************************************
 结构名    : MTA_AT_PROTOCOL_STATE_ENUM
 协议表格  :
 ASN.1描述 :
 结构说明  : 当前的协议状态
*****************************************************************************/
enum MTA_AT_PROTOCOL_STATE_ENUM
{
    MTA_AT_PROTOCOL_STATE_IDLE      = 0,
    MTA_AT_PROTOCOL_STATE_CONNECTED = 1,

    MTA_AT_PROTOCOL_STATE_BUTT
};
typedef VOS_UINT8 MTA_AT_PROTOCOL_STATE_ENUM_UINT8;

/*****************************************************************************
 结构名    : AT_MTA_FINETIME_PARA_ENUM
 协议表格  :
 ASN.1描述 :
 结构说明  : FINETIME PARA
*****************************************************************************/
enum AT_MTA_FINETIME_PARA_ENUM
{
    AT_MTA_FINETIME_PARA_0      = 0,
    AT_MTA_FINETIME_PARA_1      = 1,
    AT_MTA_FINETIME_PARA_2      = 2,

    AT_MTA_FINETIME_PARA_BUTT
};
typedef VOS_UINT8 AT_MTA_FINETIME_PARA_ENUM_UINT8;


enum TAF_MTA_NV_CARRIER_OPT_RESULT_ENUM
{
    TAF_MTA_NV_CARRIER_OPT_NO_ERROR                  = 0x00000000,
    TAF_MTA_NV_CARRIER_OPT_INVALID_PARA              = 0x10100002, /* 无效的参数 */
    TAF_MTA_NV_CARRIER_OPT_NOT_FIND_FILE             = 0x10100006, /* 文件不存在 */
    TAF_MTA_NV_CARRIER_OPT_NOT_FIND_NV               = 0x10100016, /* 没有发现NV */
    TAF_MTA_NV_CARRIER_OPT_WRITE_NV_TIMEOUT          = 0x10100035, /* 写NV超时,(实际是写入文件系统超时) */
    TAF_MTA_NV_CARRIER_OPT_WRITE_NV_NOT_SUPPORT_ERR  = 0x10100037, /* 不支持错误 */
    TAF_MTA_NV_CARRIER_OPT_MODEM_ID_ERROR            = 0x10100039, /* modem id 参数错误 */

    TAF_MTA_NV_CARRIER_OPT_BUTT
};
typedef VOS_UINT32 TAF_MTA_NV_CARRIER_OPT_RESULT_ENUM_UINT32;





enum AT_MTA_SMS_DOMAIN_ENUM
{
    AT_MTA_SMS_DOMAIN_NOT_USE_SMS_OVER_IP                  = 0,                 /**< not to use SMS over ip */
    AT_MTA_SMS_DOMAIN_PREFER_TO_USE_SMS_OVER_IP            = 1,                 /**< prefer to use SMS over ip, CS/PS SMS as secondary */

    AT_MTA_SMS_DOMAIN_BUTT
};
typedef VOS_UINT8 AT_MTA_SMS_DOMAIN_ENUM_UINT8;

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
/*****************************************************************************
  5 消息头定义
*******************************s**********************************************/
/*****************************************************************************
  6 消息定义
*****************************************************************************/
/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;                                /* 消息名 */
    AT_APPCTRL_STRU                     stAppCtrl;
    VOS_UINT8                           aucContent[4];                          /* 消息内容 */
} AT_MTA_MSG_STRU;


typedef struct
{
    MTA_AT_CPOS_OPERATE_TYPE_ENUM_UINT32    enCposOpType;
    VOS_UINT32                              ulXmlLength;
    VOS_CHAR                                acXmlText[MTA_CPOS_XML_MAX_LEN];
} AT_MTA_CPOS_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;                               /* 命令执行结果 */
} MTA_AT_CPOS_CNF_STRU;


typedef struct
{
    /* AT向MTA上报的XML码流最大长度为1024，同时还需预留一个字节存放字符串结尾的空字符 */
    VOS_CHAR                            acXmlText[MTA_CPOSR_XML_MAX_LEN + 1];
    VOS_UINT8                           aucRsv[3];
} MTA_AT_CPOSR_IND_STRU;

typedef struct
{
    VOS_UINT16                                   usTransactionID;
    VOS_UINT8                                    ucMsgBodyType;
    VOS_UINT8                                    ucCommonIEValidFlg;
    VOS_UINT8                                    ucEndFlg;
    VOS_UINT8                                    ucID;
    VOS_UINT8                                    ucReserved;
    VOS_UINT8                                    ucNameLength;
    VOS_UINT8                                    aucName[MTA_MAX_EPDU_NAME_LEN]; /* 预留一位保存'\0' */
    VOS_UINT8                                    ucTotal;
    VOS_UINT8                                    ucIndex;
    VOS_UINT16                                   usDataLength;
    VOS_UINT8                                    aucData[MTA_MAX_EPDU_BODY_LEN];
} MTA_AT_EPDU_DATA_IND_STRU;

typedef struct
{
    VOS_UINT32                          ulClearFlg;
} MTA_AT_XCPOSRRPT_IND_STRU;


typedef struct
{
    MTA_AT_CGPSCLOCK_ENUM_UINT32        enGpsClockState;                        /* RF芯片GPS时钟状态 */
} AT_MTA_CGPSCLOCK_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;                               /* 命令执行结果 */

    MTA_AT_MODEM_ID_ENUM_UINT8          enModemId;
    MTA_AT_RAT_MODE_ENUM_UINT8          enRatMode;

    VOS_UINT8                           aucReserved[2];
} MTA_AT_CGPSCLOCK_CNF_STRU;


typedef struct
{
    AT_MTA_PERS_CATEGORY_ENUM_UINT8     enCategory;
    VOS_UINT8                           aucReserved[3];
    VOS_UINT8                           aucPassword[MTA_SIMLOCK_PASSWORD_LEN];
} AT_MTA_SIMLOCKUNLOCK_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;                               /* 命令执行结果 */
} MTA_AT_SIMLOCKUNLOCK_CNF_STRU;



typedef struct
{
    VOS_UINT8                           ucRatType;
    VOS_UINT8                           aucReserve[3];                          /* 预留后续使用 */
}AT_MTA_QRY_NMR_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32         enResult;
    VOS_UINT8                         ucTotalIndex;
    VOS_UINT8                         ucCurrIndex;
    VOS_UINT16                        usNMRLen;                                 /* NMR数据长度 */
    VOS_UINT8                         aucNMRData[4];                            /* NMR数据首地址 */
}MTA_AT_QRY_NMR_CNF_STRU;




typedef struct
{
    VOS_UINT8                                   ucOffsetFlg;                    /* 0:灭屏；1:亮屏*/
    VOS_UINT8                                   aucReserve[3];                  /* 预留后续使用 */
}AT_MTA_RESEL_OFFSET_CFG_SET_NTF_STRU;


typedef struct
{
    VOS_UINT8                           ucCmd;
    VOS_UINT8                           ucParaNum;
    VOS_UINT16                          usReserve;
    VOS_UINT32                          aulPara[AT_MTA_WRR_AUTOTEST_MAX_PARA_NUM];
}AT_MTA_WRR_AUTOTEST_QRY_PARA_STRU;


typedef struct
{
    VOS_UINT8                           ucCmd;
    VOS_UINT8                           ucParaNum;
    VOS_UINT16                          usReserve;
    VOS_UINT32                          aulPara[AT_MTA_GAS_AUTOTEST_MAX_PARA_NUM];
}AT_MTA_GAS_AUTOTEST_QRY_PARA_STRU;


typedef struct
{
    VOS_UINT32                          ulRsltNum;
    VOS_UINT32                          aulRslt[MTA_AT_WRR_AUTOTEST_MAX_RSULT_NUM];
}MTA_AT_WRR_AUTOTEST_QRY_RSLT_STRU;


typedef struct
{
    VOS_UINT32                          ulResult;
    MTA_AT_WRR_AUTOTEST_QRY_RSLT_STRU   stWrrAutoTestRslt;
} MTA_AT_WRR_AUTOTEST_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulRsltNum;
    VOS_UINT32                          aulRslt[MTA_AT_GAS_AUTOTEST_MAX_RSULT_NUM];
}MTA_AT_GAS_AUTOTEST_QRY_RSLT_STRU;


typedef struct
{
    VOS_UINT32                          ulResult;
    MTA_AT_GAS_AUTOTEST_QRY_RSLT_STRU   stGrrAutoTestRslt;
} MTA_AT_GAS_AUTOTEST_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT16                      usCellFreq;
    VOS_UINT16                      usPrimaryScramCode;
    VOS_INT16                       sCpichRscp;
    VOS_INT16                       sCpichEcN0;

}MTA_AT_WRR_CELLINFO_STRU;


typedef struct
{
     VOS_UINT32                     ulCellNum;
     MTA_AT_WRR_CELLINFO_STRU       astWCellInfo[MTA_AT_WRR_MAX_NCELL_NUM];         /*最多支持W 8个邻区的查询*/

} MTA_AT_WRR_CELLINFO_RSLT_STRU;

typedef struct
{
    VOS_UINT32                          ulResult;
    MTA_AT_WRR_CELLINFO_RSLT_STRU       stWrrCellInfo;
} MTA_AT_WRR_CELLINFO_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT16                      usEventId;                                  /*对应的事件类型*/
    VOS_UINT16                      usCellNum;
    VOS_UINT16                      ausPrimaryScramCode[MTA_AT_WRR_ONE_MEANRPT_MAX_CELL_NUM];
}MTA_AT_WRR_MEANRPT_STRU;


typedef struct
{
    VOS_UINT32                      ulRptNum;
    MTA_AT_WRR_MEANRPT_STRU         astMeanRptInfo[MTA_AT_WRR_MAX_MEANRPT_NUM];
}MTA_AT_WRR_MEANRPT_RSLT_STRU;


typedef struct
{
    VOS_UINT32                          ulResult;
    MTA_AT_WRR_MEANRPT_RSLT_STRU        stMeanRptRslt;
} MTA_AT_WRR_MEANRPT_QRY_CNF_STRU;


typedef struct
{
    VOS_INT16                           sCpichRscp;
    VOS_INT16                           sCpichEcNo;
}MTA_AT_ANTENNA_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT8                           ucFreqLockEnable;
    VOS_UINT8                           aucReserved[1];
    VOS_UINT16                          usLockedFreq;
} MTA_AT_WRR_FREQLOCK_CTRL_STRU;


typedef struct
{
    VOS_UINT32                          ulResult;
} MTA_AT_WRR_FREQLOCK_SET_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulResult;
    MTA_AT_WRR_FREQLOCK_CTRL_STRU       stFreqLockInfo;
} MTA_AT_WRR_FREQLOCK_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulResult;
} MTA_AT_WRR_RRC_VERSION_SET_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucRrcVersion;
    VOS_UINT8                           aucReserved[3];
} MTA_AT_WRR_RRC_VERSION_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulResult;
} MTA_AT_WRR_CELLSRH_SET_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucCellSearchType;
    VOS_UINT8                           aucReserve[3];
} MTA_AT_WRR_CELLSRH_QRY_CNF_STRU;


typedef struct
{
    VOS_INT16                           sCpichRscp;
    VOS_INT16                           sCpichEcNo;
    VOS_UINT32                          ulCellId;
    VOS_UINT8                           ucRssi;                                 /* Rssi, GU下使用*/
    VOS_UINT8                           aucReserve[3];

}AT_MTA_ANQUERY_2G_3G_PARA_STRU;


typedef struct
{
    VOS_INT16                           sRsrp;                                  /* 范围：(-141,-44), 99为无效 */
    VOS_INT16                           sRsrq;                                  /* 范围：(-40, -6) , 99为无效 */
    VOS_INT16                           sRssi;                                  /* Rssi, LTE下使用*/
    VOS_UINT8                           aucReserve[2];
}AT_MTA_ANQUERY_4G_PARA_STRU;


typedef struct
{
    VOS_UINT8                               enServiceSysMode;                       /* 指示上报模式*/
    VOS_UINT8                               aucReserve[3];
    union
    {
        AT_MTA_ANQUERY_2G_3G_PARA_STRU      st2G3GCellSignInfo;                     /* Rssi, GU下使用*/
        AT_MTA_ANQUERY_4G_PARA_STRU         st4GCellSignInfo;                       /* Rssi, LTE下使用*/
    }u;
}AT_MTA_ANQUERY_PARA_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;      /* 操作结果 */
    AT_MTA_ANQUERY_PARA_STRU            stAntennaInfo;
}MTA_AT_ANTENNA_INFO_QRY_CNF_STRU;


typedef struct
{
    VOS_INT16                           sCpichRscp;
    VOS_INT16                           sCpichEcNo;
}AT_MTA_CSNR_PARA_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;      /* 操作结果 */
    AT_MTA_CSNR_PARA_STRU              stCsnrPara;
}MTA_AT_CSNR_QRY_CNF_STRU;

/*****************************************************************************
 结构名    : AT_MMA_CSQLVL_PARA_STRU
 结构说明  : CSQLVL查询操作的结果上报结构
             包括RSCP 等级和RSCP 的绝对值
*******************************************************************************/
typedef struct
{
    VOS_UINT16                          usRscp;                                 /* RSCP 等级 */
    VOS_UINT16                          usLevel;                                /* RSCP 的绝对值 */
}AT_MTA_CSQLVL_PARA_STRU;


typedef struct
{
    VOS_UINT8                           enRssilv;                         /* CSQLVLEX查询获取到的信号格数 */
    VOS_UINT8                           enBer;                            /* 比特误码率百分比，暂时不支持BER查询，填99*/
}AT_MTA_CSQLVLEXT_PARA_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;      /* 操作结果 */
    AT_MTA_CSQLVL_PARA_STRU             stCsqLvlPara;
    AT_MTA_CSQLVLEXT_PARA_STRU          stCsqLvlExtPara;
    VOS_UINT8                           aucReserved[2];
}MTA_AT_CSQLVL_QRY_CNF_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;                               /* 操作结果 */
    AT_MTA_FRAT_IGNITION_ENUM_UNIT8     enFratIgnitionState;                    /* IgnitionState */
    VOS_UINT8                           aucReserved[3];
}MTA_AT_FRAT_IGNITION_QRY_CNF_STRU;


typedef struct
{
    AT_MTA_FRAT_IGNITION_ENUM_UNIT8     enFratIgnitionState;                    /* IgnitionState */
    VOS_UINT8                           aucReserved[3];
} AT_MTA_FRAT_IGNITION_SET_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;                               /* 查询操作结果*/
} MTA_AT_FRAT_IGNITION_SET_CNF_STRU;



typedef struct
{
    AT_MTA_BODY_SAR_STATE_ENUM_UINT16   enState;       /* Body SAR状态 */
    VOS_UINT16                          usRsv;         /* 保留位 */
    MTA_BODY_SAR_PARA_STRU              stBodySARPara; /* Body SAR功率门限参数 */
}AT_MTA_BODY_SAR_SET_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;      /* 操作结果 */
}MTA_AT_RESULT_CNF_STRU;


typedef struct
{
    VOS_UINT8                           aucReserved[4]; /* 保留位 */
}AT_MTA_RESERVE_STRU;


typedef struct
{
    VOS_UINT8                           ucCssiRptFlg;                           /* 禁止或使能+CSSI的补充业务通知 0:不上报;1:上报 */
    VOS_UINT8                           ucCssuRptFlg;                           /* 禁止或使能+CSSU的补充业务通知 0:不上报;1:上报 */
} AT_MTA_CSSN_RPT_FLG_STRU;


typedef struct
{
    AT_MTA_RPT_GENERAL_CTRL_TYPE_ENUM_UINT8     enCurcRptType;                  /* 主动上报模式，0:关闭所有的主动上报；1:打开所有的主动上报；2:根据RptCfg参数决定对应的bit位是否主动上报 */
    VOS_UINT8                                   aucReserve[3];                  /* 保留字段 */
    VOS_UINT8                                   aucRptCfg[AT_MTA_RPT_CFG_MAX_SIZE];                   /* 64bit主动上报标识 */
} AT_MTA_CURC_SET_NOTIFY_STRU;


typedef struct
{
    VOS_UINT8                           aucReserve[4];
} AT_MTA_CURC_QRY_REQ_STRU;


typedef struct
{
    AT_MTA_RPT_SET_TYPE_ENUM_UINT32     enReqType;                              /* 请求类型 */

    union
    {
        VOS_UINT8                       ucModeRptFlg;                           /* mode是否主动上报标识 0:不上报，1:上报 */
        VOS_UINT8                       ucSrvstRptFlg;                          /* service status是否主动上报标识 */
        VOS_UINT8                       ucRssiRptFlg;                           /* rssi是否主动上报标识 */
        VOS_UINT8                       ucTimeRptFlg;                           /* time是否主动上报标识 */
        VOS_UINT8                       ucCtzrRptFlg;                           /* ctzr是否主动上报标识 */
        VOS_UINT8                       ucDsFlowRptFlg;                         /* 流量是否主动上报标识 */
        VOS_UINT8                       ucSimstRptFlg;                          /* sim卡状态是否主动上报标识 */
        VOS_UINT8                       ucCregRptFlg;                           /* cs域注册状态是否主动上报标识 */
        VOS_UINT8                       ucCgregRptFlg;                          /* ps域注册状态是否主动上报标识 */
        VOS_UINT8                       ucCeregRptFlg;                          /* L注册状态是否主动上报标识 */
        VOS_UINT8                       ucCsidRptFlg;                           /* CSID是否主动上报标识 */
        VOS_UINT8                       ucClocinfoRptFlg;                       /* CLOCINFO是否主动上报标识 */
        VOS_UINT8                       ucMtReattachRptFlg;                     /* MTREATTACH是否主动上报标识 */
        VOS_UINT8                       ucCenfsRptFlg;                          /* ^CENFS是否主动上报标识 0:不上报，1:上报 */
    }u;

    VOS_UINT8                           aucReserve[3];

 } AT_MTA_UNSOLICITED_RPT_SET_REQ_STRU;


typedef struct
{
    AT_MTA_RPT_QRY_TYPE_ENUM_UINT32     enReqType;
} AT_MTA_UNSOLICITED_RPT_QRY_REQ_STRU;


typedef struct
{
    VOS_UINT8                           ucGameMode;
    VOS_UINT8                           aucRsv[3];
} AT_MTA_COMM_GAME_MODE_SET_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           ucResult;
} MTA_AT_GAME_MODE_SET_CFN_STRU;



typedef struct
{
    VOS_UINT16                          usPidIndex;
    VOS_UINT8                           aucRsv[2];
    VOS_UINT32                          ulLen; /* 有效字符的个数 */
    VOS_UINT8                           aucDate[AT_CMD_COMM_BOOSTER_BS_MAX_LENGTH];
} AT_MTA_COMM_BOOSTER_SET_REQ_STRU;


typedef struct
{
    VOS_UINT16                          usPidIndex;
    VOS_UINT8                           aucRsv[2];
    VOS_UINT32                          ulLen; /* 有效字符的个数 */
    VOS_UINT8                           aucDate[AT_CMD_COMM_BOOSTER_BS_MAX_LENGTH];
} AT_MTA_COMM_BOOSTER_QUERY_REQ_STRU;


typedef struct
{
    VOS_UINT8                           ucInternalResultFlag;      /* 内部原因标识 */
    VOS_UINT8                           ucExternalResultFlag;      /* 外部原因标识 */
    VOS_UINT8                           aucRsv[2];                 /* 保留位 */
    VOS_UINT32                          ulResult;                  /* 结果 */
}MTA_AT_COMM_BOOSTER_SET_CNF_STRU;


typedef struct
{
    VOS_UINT8                           ucResult;
    VOS_UINT8                           aucRsv[3];
    VOS_UINT32                          ulLen;  /* 有效字符的个数 */
    VOS_UINT8                           aucDate[MTA_AT_BOOSTER_QRY_CNF_MAX_NUM];
}MTA_AT_COMM_BOOSTER_QUERY_CNF_STRU;



typedef struct
{
    TAF_MTA_NV_CARRIER_OPT_RESULT_ENUM_UINT32           enResult;
}MTA_AT_SET_NVLOAD_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulPidIndex;
    VOS_UINT32                          ulLen;
    VOS_UINT8                           aucBoosterIndData[MTA_AT_BOOSTER_IND_MAX_NUM];  /* Booster特性主动上报参数，TLV格式，满足四字节对齐 */
} MTA_AT_COMM_BOOSTER_IND_STRU;


typedef struct
{
    VOS_UINT32                           ulReqType;                               
}AT_MTA_NVLOAD_SET_REQ_STRU;


typedef struct
{
    VOS_UINT16                          usReqType;
    VOS_UINT16                          usTempRange;                 /*温区范围(极高温，高温，常温，低温，极低温)，"11111"代表所有温区*/
    VOS_INT16                           sPpmOffset;                  /*样本偏移量*/
    VOS_INT16                           sTimeOffset;                 /*时间偏移量*/
    
}AT_MTA_SET_SAMPLE_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
}MTA_AT_SET_SAMPLE_CNF_STRU;



typedef struct
{
    VOS_UINT8                           ucPseudRat;                            /* 伪基站制式 1.GSM  2.WCDMA  3.LTE */
    VOS_UINT8                           ucPseudBtsQryType;                     /* 查询参数 1.查询伪基站是否支持  2.查询伪基站拦截次数 */
    VOS_UINT8                           aucRsv[2];

}AT_MTA_SET_PSEUDBTS_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32               enResult;                               /* NO_ERROR:成功，其他:失败 */
    VOS_UINT8                               ucPseudBtsIdentType;                    /* 查询参数 1.查询伪基站是否支持  2.查询伪基站拦截次数 */
    VOS_UINT8                               aucRsv[3];

    union
    {
        VOS_UINT32                          ulPseudBtsIdentTimes;                   /* 识别次数 */
        VOS_UINT8                           ucPseudBtsIdentCap;                     /* 是否支持 */
    }u;
}MTA_AT_PSEUD_BTS_SET_CNF_STRU;




typedef struct
{
    VOS_UINT8                           ucSwitch;
    VOS_UINT8                           aucReserve[3];
} AT_MTA_NCELL_MONITOR_SET_REQ_STRU;


typedef struct
{
    AT_MTA_CLEAR_FREQ_FLG_ENUM_UINT8    enMode;
    VOS_UINT8                           aucReserved[3];
} AT_MTA_CLEAR_HISTORY_FREQ_REQ_STRU;



typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32                   enResult;
    AT_MTA_RPT_GENERAL_CTRL_TYPE_ENUM_UINT8     enCurcRptType;                          /* 主动上报模式，0:关闭所有的主动上报；1:打开所有的主动上报；2:根据ReportCfg参数决定对应的bit位是否主动上报 */
    VOS_UINT8                                   aucReserve[3];
    VOS_UINT8                                   aucRptCfg[AT_MTA_RPT_CFG_MAX_SIZE];  /* 主动上报标识 */
} MTA_AT_CURC_QRY_CNF_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
} MTA_AT_UNSOLICITED_RPT_SET_CNF_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
    AT_MTA_RPT_QRY_TYPE_ENUM_UINT32     enReqType;

    union
    {
        VOS_UINT8                       ucTimeRptFlg;                           /* time是否主动上报标识 */
        MTA_TAF_CTZR_TYPE_ENUM_UINT32   ucCtzrRptFlg;                           /* ctzr是否主动上报标识 */
        AT_MTA_CSSN_RPT_FLG_STRU        stCssnRptFlg;                           /* cssn是否主动上报标识结构体 */
        VOS_UINT8                       ucCusdRptFlg;                           /* cusd是否主动上报标识结构体 */
    }u;

} MTA_AT_UNSOLICITED_RPT_QRY_CNF_STRU;




typedef struct
{
    VOS_UINT8                           aucImei[NV_ITEM_IMEI_SIZE]; /* IMEI号码 */
}MTA_AT_CGSN_QRY_CNF_STRU;

typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
    VOS_UINT8                           ucSwitch;
    VOS_UINT8                           ucNcellState;
    VOS_UINT8                           aucReserved[2];
} MTA_AT_NCELL_MONITOR_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT8                           ucNcellState;
    VOS_UINT8                           aucReserved[3];
} MTA_AT_NCELL_MONITOR_IND_STRU;



typedef struct
{
    AT_MTA_CMD_RPT_FLG_ENUM_UINT8       enRptFlg;                               /* 命令上报开关标志 */
    VOS_UINT8                           aucReserved1[3];                        /* 保留位 */
} AT_MTA_REFCLKFREQ_SET_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
    VOS_UINT32                          ulFreq;                                 /* GPS参考时钟的频率值，单位Hz */
    VOS_UINT32                          ulPrecision;                            /* 当前GPS参考时钟的精度，单位ppb */
    MTA_AT_REFCLOCK_STATUS_ENUM_UINT16  enStatus;                               /* 时钟频率锁定状态 */
    VOS_UINT8                           aucReserved1[2];                        /* 保留位 */
} MTA_AT_REFCLKFREQ_QRY_CNF_STRU;



typedef struct
{
    VOS_UINT16                          usChannelNo;                            /* 接收端地址 */
    VOS_UINT16                          usRficReg;
}AT_MTA_RFICSSIRD_REQ_STRU;


typedef struct
{
    VOS_UINT32                          ulRegValue;                             /* 回复的寄存器的值 */
} MTA_AT_RFICSSIRD_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulFreq;                                 /* GPS参考时钟的频率值，单位Hz */
    VOS_UINT32                          ulPrecision;                            /* 当前GPS参考时钟的精度，单位ppb */
    MTA_AT_REFCLOCK_STATUS_ENUM_UINT16  enStatus;                               /* 时钟频率锁定状态 */
    VOS_UINT8                           aucReserved1[2];                        /* 保留位 */
} MTA_AT_REFCLKFREQ_IND_STRU;



typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;                               /* 操作结果 */
} MTA_AT_HANDLEDECT_SET_CNF_STRU;


typedef struct
{
    VOS_UINT16                          usHandle;                               /* 左右手类型 */
    VOS_UINT16                          aucReserved1[1];
    MTA_AT_RESULT_ENUM_UINT32           enResult;                               /* 操作结果 */
} MTA_AT_HANDLEDECT_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT8                           ucCause;                                /* PS域迁移原因值 */
    VOS_UINT8                           aucReserved1[3];                        /* 保留位 */
} MTA_AT_PS_TRANSFER_IND_STRU;


typedef struct
{
    VOS_UINT16                          usMipiClk;                              /* 主动上报MIPICLK值 */
    VOS_UINT8                           aucReserved1[2];                        /* 保留位 */
} MTA_AT_RF_LCD_MIPICLK_IND_STRU;


typedef struct
{
    VOS_UINT16                          usMipiClk;                              /* 主动上报MIPICLK值 */
    VOS_UINT16                          usResult;
} MTA_AT_RF_LCD_MIPICLK_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulVersion;                              /* 命令版本号 */
}AT_MTA_ECID_SET_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
    VOS_UINT8                           aucCellInfoStr[4];                      /* 增强型小区信息字符串 */
}MTA_AT_ECID_SET_CNF_STRU;


typedef struct
{
    PS_BOOL_ENUM_UINT8                  enPsProtectFlg;
    VOS_UINT8                           aucReserve[3];                          /* 预留后续使用 */
}AT_MTA_RRC_PROTECT_PS_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;                          /* 返回给AT的结果 */
}MTA_AT_RRC_PROTECT_PS_CNF_STRU;


typedef struct
{
    VOS_UINT8                           aucReserve[4];                          /* 预留后续使用 */
}AT_MTA_PHY_INIT_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;                          /* 返回给AT的结果 */
}MTA_AT_PHY_INIT_CNF_STRU;



typedef struct
{
    AT_MTA_CMD_RATMODE_ENUM_UINT8       enRatMode;
    VOS_UINT8                           ucFlag;
    VOS_UINT8                           aucReserved[2];
} AT_MTA_SET_DPDTTEST_FLAG_REQ_STRU;


typedef struct
{
    AT_MTA_CMD_RATMODE_ENUM_UINT8       enRatMode;
    VOS_UINT8                           aucReserved[3];
    VOS_UINT32                          ulDpdtValue;
} AT_MTA_SET_DPDT_VALUE_REQ_STRU;


typedef struct
{
    AT_MTA_CMD_RATMODE_ENUM_UINT8       enRatMode;
    VOS_UINT8                           aucReserved[3];
} AT_MTA_QRY_DPDT_VALUE_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
} MTA_AT_SET_DPDTTEST_FLAG_CNF_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
} MTA_AT_SET_DPDT_VALUE_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulDpdtValue;
    MTA_AT_RESULT_ENUM_UINT32           enResult;
} MTA_AT_QRY_DPDT_VALUE_CNF_STRU;


typedef struct
{
    VOS_UINT8                           ucFlag;
    VOS_UINT8                           ucMethod;
    VOS_UINT8                           ucThreshold;
    VOS_UINT8                           ucFreqNum;
} AT_MTA_SET_JAM_DETECT_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
} MTA_AT_SET_JAM_DETECT_CNF_STRU;


typedef struct
{
    MTA_AT_JAM_RESULT_ENUM_UINT32       enJamResult;
} MTA_AT_JAM_DETECT_IND_STRU;


typedef struct
{
    VOS_UINT8                           ucEnableFlg;                            /* 1:锁频功能打开，0:功能关闭 */
    AT_MTA_FREQLOCK_RATMODE_ENUM_UINT8  enRatMode;
    AT_MTA_GSM_BAND_ENUM_UINT16         enBand;
    VOS_UINT32                          ulLockedFreq;
} AT_MTA_SET_FREQ_LOCK_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
} MTA_AT_SET_FREQ_LOCK_CNF_STRU;


typedef struct
{
    PS_BOOL_ENUM_UINT8                  enableFlag;                             /* PS_TRUE:锁定，PS_FALSE:去锁定 */
    VOS_UINT8                           aucReserved[3];                         /* 保留位 */
    VOS_UINT16                          usFreq;                                 /* 频点值 */
    AT_MTA_GSM_BAND_ENUM_UINT16         enBand;                                 /* GSM频段 */
} AT_MTA_SET_GSM_FREQLOCK_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
} MTA_AT_SET_GSM_FREQLOCK_CNF_STRU;



typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           ulResult;
    PS_BOOL_ENUM_UINT8                  enLockFlg;                              /* PS_TRUE: 执行了锁频命令,PS_FALSE: 没有执行锁频命令 */
    VOS_UINT8                           aucReserved[3];                         /* 保留位 */
    VOS_UINT16                          usFreq;                                 /* 绝对频点号 */
    AT_MTA_GSM_BAND_ENUM_UINT16         enBand;                                 /* GSM频段 */
} MTA_AT_QRY_GSM_FREQLOCK_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulResult;
} MTA_AT_DEL_CELLENTITY_CNF_STRU;

#if (FEATURE_ON == FEATURE_PROBE_FREQLOCK)

enum AT_FREQLOCK_W_TYPE_ENUM
{
    AT_MTA_WCDMA_FREQLOCK_CMD_FREQ_ONLY         = 8,                            /* 对应CWAS锁频类型 */
    AT_MTA_WCDMA_FREQLOCK_CMD_COMBINED          = 9,                            /* 对应CWAS锁频+扰码类型 */
    AT_MTA_WCDMA_FREQLOCK_CMD_BUTT
};
typedef VOS_UINT8 AT_MTA_WCDMA_FREQLOCK_CMD_ENUM_UINT8;


enum AT_MTA_M2M_FREQLOCK_TYPE_ENUM
{
    AT_MTA_M2M_FREQLOCK_TYPE_ENUM_FREQ_ONLY     = 1,                            /* 锁频ONLY类型 */
    AT_MTA_M2M_FREQLOCK_TYPE_ENUM_FREQ_COMBINED = 2,                            /* 锁频+主扰码/扰码/物理小区ID */
    AT_MTA_M2M_FREQLOCK_TYPE_ENUM_BUTT
};
typedef VOS_UINT8 AT_MTA_M2M_FREQLOCK_TYPE_ENUM_UINT8;


typedef struct
{
    VOS_UINT16                                  usFreq;                         /* 频点值 */
    AT_MTA_GSM_BAND_ENUM_UINT16                 enBand;                         /* GSM频段 */
    VOS_UINT32                                  ulReserve;                      /* 8字节对齐-代码检视 */
} AT_MTA_M2M_GSM_FREQLOCK_PARA_STRU;


typedef struct
{
    AT_MTA_M2M_FREQLOCK_TYPE_ENUM_UINT8         ucFreqType;                     /* 锁频类型 */
    VOS_UINT8                                   aucReserved[3];                 /* 保留位 */
    VOS_UINT16                                  usFreq;                         /* 频点值 */
    VOS_UINT16                                  usPsc;                          /* WCDMA主扰码 */
}AT_MTA_M2M_WCDMA_FREQLOCK_PARA_STRU;


typedef struct
{
    AT_MTA_M2M_FREQLOCK_TYPE_ENUM_UINT8         ucFreqType;                     /* 锁频类型 */
    VOS_UINT8                                   aucReserved[3];                 /* 保留位 */
    VOS_UINT16                                  usFreq;                         /* 频点值 */
    VOS_UINT16                                  usSc;                           /* TD-SCDMA扰码 */
}AT_MTA_M2M_TDSCDMA_FREQLOCK_PARA_STRU;


typedef struct
{
    VOS_UINT32                                  ulFreq;                         /* 频点值 */
    VOS_UINT16                                  usPci;                          /* LTE物理小区ID */
    AT_MTA_M2M_FREQLOCK_TYPE_ENUM_UINT8         ucFreqType;                     /* 锁频类型 */
    VOS_UINT8                                   ucReserved;                     /* 保留位 */
}AT_MTA_M2M_LTE_FREQLOCK_PARA_STRU;


enum AT_FREQLOCK_FLAG_TYPE_ENUM
{
    AT_MTA_M2M_FREQLOCK_MODE_GSM            = 1,                                /* GSM制式 */
    AT_MTA_M2M_FREQLOCK_MODE_WCDMA          = 2,                                /* WCDMA制式 */
    AT_MTA_M2M_FREQLOCK_MODE_TDSCDMA        = 3,                                /* TD-SCDMA制式 */
    AT_MTA_M2M_FREQLOCK_MODE_LTE            = 4,                                /* LTE制式 */
    AT_MTA_M2M_FREQLOCK_MODE_BUTT
};
typedef VOS_UINT8 AT_MTA_M2M_FREQLOCK_MODE_ENUM_UINT8;


enum AT_MTA_M2M_FREQLOCK_FLAG_TYPE_ENUM
{
    AT_MTA_M2M_FREQLOCK_FLAG_TYPE_OFF       = 0,                                /* 锁频关闭 */
    AT_MTA_M2M_FREQLOCK_FLAG_TYPE_ON        = 1,                                /* 锁频开启 */
    AT_MTA_M2M_FREQLOCK_FLAG_TYPE_BUTT
};
typedef VOS_UINT8 AT_MTA_M2M_FREQLOCK_FLAG_ENUM_UINT8;


typedef struct
{
    AT_MTA_M2M_FREQLOCK_FLAG_ENUM_UINT8         enableFlag;                                 /* 使能标识 */
    AT_MTA_M2M_FREQLOCK_MODE_ENUM_UINT8         ucMode;                                     /* 锁频制式 */
    VOS_UINT8                                   aucReserved[6];                             /* 保留 */
    AT_MTA_M2M_GSM_FREQLOCK_PARA_STRU           stGFreqPara;                                /* G模锁频消息结构 */
    AT_MTA_M2M_WCDMA_FREQLOCK_PARA_STRU         stWFreqPara;                                /* W模锁频消息结构 */
    AT_MTA_M2M_TDSCDMA_FREQLOCK_PARA_STRU       stTDFreqPara;                               /* TD模锁频消息结构 */
    AT_MTA_M2M_LTE_FREQLOCK_PARA_STRU           stLFreqPara;                                /* L模锁频消息结构 */
} AT_MTA_SET_M2M_FREQLOCK_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           ulResult;
} MTA_AT_SET_M2M_FREQLOCK_CNF_STRU;


typedef struct
{
    VOS_UINT8                                   ucFreqState[MTA_FREQLOCK_MAX_MODE];     /* GUTL模式的锁频状态 */
    AT_MTA_M2M_GSM_FREQLOCK_PARA_STRU           stGFreqLockInfo;                        /* GSM锁频状态查询结果 */
    AT_MTA_M2M_WCDMA_FREQLOCK_PARA_STRU         stWFreqLockInfo;                        /* WCDMA锁频状态查询结果 */
    AT_MTA_M2M_TDSCDMA_FREQLOCK_PARA_STRU       stTFreqLockInfo;                        /* TD-SCDMA锁频状态查询结果 */
    AT_MTA_M2M_LTE_FREQLOCK_PARA_STRU           stLFreqLockInfo;                        /* LTE锁频状态查询结果 */
} MTA_AT_QRY_M2M_FREQLOCK_CNF_STRU;
#endif



typedef struct
{
    MTA_AT_GPHY_XPASS_MODE_ENUM_UINT16            enGphyXpassMode;
    VOS_UINT16                                    usResev1;
    MTA_AT_WPHY_HIGHWAY_MODE_ENUM_UINT16          enWphyXpassMode;
    VOS_UINT16                                    usResev2;
} MTA_AT_XPASS_INFO_IND_STRU;

typedef struct
{
    VOS_UINT32      ulCtrlType;
    VOS_UINT32      ulPara1;
    VOS_UINT32      ulPara2;
    VOS_UINT32      ulPara3;
} AT_MTA_SET_FEMCTRL_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
} MTA_AT_SET_FEMCTRL_CNF_STRU;


typedef struct
{
    VOS_UINT8                           ucSecType;                              /* 安全控制类型 */
    VOS_UINT8                           ucSecStrFlg;                            /* 是否携带安全校验密文 */
    VOS_UINT8                           aucReserved[2];
    VOS_UINT8                           aucSecString[AT_RSA_CIPHERTEXT_LEN];    /* 安全校验密文 */
} AT_MTA_NVWRSECCTRL_SET_REQ_STRU;


typedef struct
{
    PS_BOOL_ENUM_UINT8                          enActFrFlag;                    /* 激活FR标志，0:不激活  1:激活 */
    VOS_UINT8                                   aucRsv[3];                      /* 保留位 */
}AT_MTA_SET_FR_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32                   enRslt;                         /* FR设置及结果 */
}MTA_AT_SET_FR_CNF_STRU;


typedef struct
{
    VOS_UINT16                                    usYear;
    VOS_UINT8                                     ucMonth;
    VOS_UINT8                                     ucDay;
    VOS_UINT8                                     ucHour;
    VOS_UINT8                                     ucMinute;
    VOS_UINT8                                     ucSecond;
    VOS_UINT8                                     ucRsv;
}MTA_AT_TIME_STRU;


typedef struct
{
    MTA_AT_TIME_TYPE_ENUM_UINT8                   enTimeType;
    MTA_AT_DAY_LIGHT_SAVING_IND_ENUM_UINT8        enDayLightSavingInd;
    VOS_UINT8                                     aucRsv[2];
    MTA_AT_TIME_STRU                              stLocalTime;
    MTA_AT_TIME_STRU                              stGpsTime;
}MTA_AT_SIB16_TIME_UPDATE_STRU;


enum MTA_AT_ACCESS_STRATUM_REL_ENUM
{
    MTA_AT_ACCESS_STRATUM_REL8                  = 0,
    MTA_AT_ACCESS_STRATUM_REL9                  = 1,
    MTA_AT_ACCESS_STRATUM_REL10                 = 2,
    MTA_AT_ACCESS_STRATUM_REL11                 = 3,
    MTA_AT_ACCESS_STRATUM_REL12                 = 4,
    MTA_AT_ACCESS_STRATUM_REL13                 = 5,
    MTA_AT_ACCESS_STRATUM_REL14                 = 6,
    MTA_AT_ACCESS_STRATUM_REL_SPARE1            = 7
};
typedef VOS_UINT8 MTA_AT_ACCESS_STRATUM_REL_ENUM_UINT8;


typedef struct
{
    MTA_AT_ACCESS_STRATUM_REL_ENUM_UINT8        enAccessStratumRel; /* 指示AT接入层的协议版本 */
    VOS_UINT8                                   aucRsv[3];
} MTA_AT_ACCESS_STRATUM_REL_IND_STRU;



typedef struct
{
    VOS_UINT8                           ucValidflag;        /* 主卡主CC TM1时为TRUE（后续按周期上报）; 其他情况为 其他情况为FALSE(只上报一次，携带内容为全0). */
    VOS_UINT8                           ucRxAntNum;         /* UE接收天线数：2收、4收 */
    VOS_UINT8                           ucTmMode;            /*传输模式 。仅当 ucTmMode=0(对应传输模式1)时，透传的相位相关数据有效*/
    VOS_UINT8                           aucReserved;
    VOS_UINT32                          ulCorrQR1Data0011;  /* 透传以下6个寄存器数据。具体含义如下： */
    VOS_UINT32                          ulCorrQR1Data01IQ;
    VOS_UINT32                          ulCorrQR2Data0011;  /* 仅4收时有效； */
    VOS_UINT32                          ulCorrQR2Data01IQ;  /* 仅4收时有效； */
    VOS_UINT32                          ulCorrQR3Data0011;  /* 仅4收时有效； */
    VOS_UINT32                          ulCorrQR3Data01IQ;  /* 仅4收时有效； */
    VOS_INT16                           sRsrpR0;            /*天线 0 rsrp， 单位 dB*/
    VOS_INT16                           sRsrpR1;            /*天线 1 rsrp， 单位 dB*/
    VOS_INT16                           sRsrpR2;            /*天线 2 rsrp， 单位 dB，仅 4收时有效*/
    VOS_INT16                           sRsrpR3;            /*天线 3 rsrp， 单位 dB，仅 4收时有效*/
    VOS_INT16                           sRssiR0;            /*天线 0 rssi， 单位 dB*/
    VOS_INT16                           sRssiR1;            /*天线 1 rssi， 单位 dB*/
    VOS_INT16                           sRssiR2;            /*天线 2 rssi， 单位 dB，仅 4收时有效*/
    VOS_INT16                           sRssiR3;            /*天线 3 rssi， 单位 dB，仅 4收时有效*/
    VOS_INT16                           sRsv[8];
} MTA_AT_ECC_STATUS_IND_STRU;


typedef struct
{
    VOS_UINT32                          ulEccEnable;
    VOS_UINT32                          ulRptPeriod;
}AT_MTA_SET_ECC_CFG_REQ_STRU;


typedef MTA_AT_RESULT_CNF_STRU MTA_AT_SET_ECC_CFG_CNF_STRU;


typedef struct
{
    AT_MTA_CFG_ENUM_UINT8               enCfg;                                  /* 0:去使能，1:使能 */
    VOS_UINT8                           aucReserved[3];                         /* 保留位 */
} AT_MTA_MBMS_SERVICE_OPTION_SET_REQ_STRU;



typedef struct
{
    VOS_UINT32                          ulMcc;                                  /* MCC, 3 bytes */
    VOS_UINT32                          ulMnc;                                  /* MNC, 2 or 3 bytes */
} AT_MTA_PLMN_ID_STRU;


typedef struct
{
    VOS_UINT32                          ulMbmsSerId;                            /* Service ID */
    AT_MTA_PLMN_ID_STRU                 stPlmnId;                               /* PLMN ID */
} AT_MTA_MBMS_TMGI_STRU;


typedef struct
{
    AT_MTA_MBMS_SERVICE_STATE_SET_ENUM_UINT8    enStateSet;                     /* 状态设置 */
    VOS_UINT8                                   aucReserved[3];                 /* 保留位 */
    VOS_UINT32                                  ulAreaId;                       /* Area ID */
    AT_MTA_MBMS_TMGI_STRU                       stTMGI;                         /* TMGI */
} AT_MTA_MBMS_SERVICE_STATE_SET_REQ_STRU;


typedef struct
{
    AT_MTA_MBMS_CAST_MODE_ENUM_UINT8    enCastMode;                             /* 0:单播,1:组播 */
    VOS_UINT8                           aucReserved[3];                         /* 保留位 */
} AT_MTA_MBMS_PREFERENCE_SET_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
    VOS_UINT32                          aulUTC[2];                              /* (0..549755813887) */
} MTA_AT_MBMS_SIB16_NETWORK_TIME_QRY_CNF_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
    VOS_UINT8                           ucBSSILevel;                            /* BSSI信号强度,0xFF:表示BSSI信号强度无效 */
    VOS_UINT8                           aucReserved[3];                         /* 保留位 */
} MTA_AT_MBMS_BSSI_SIGNAL_LEVEL_QRY_CNF_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
    VOS_UINT32                          ulCellId;                               /* 小区ID */
} MTA_AT_MBMS_NETWORK_INFO_QRY_CNF_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32                   enResult;
    MTA_AT_EMBMS_FUNTIONALITY_STATUS_ENUM_UINT8 enStatus;                       /* 功能状态 */
    VOS_UINT8                                   aucReserved[3];                 /* 保留位 */
} MTA_AT_EMBMS_STATUS_QRY_CNF_STRU;


typedef struct
{
    AT_MTA_CFG_ENUM_UINT8                       enCfg;                          /* 0:关闭,1:打开 */
    VOS_UINT8                                   aucReserved[3];                 /* 保留位 */
} AT_MTA_MBMS_UNSOLICITED_CFG_SET_REQ_STRU;


typedef struct
{
    MTA_AT_MBMS_SERVICE_EVENT_ENUM_UINT8    enEvent;                            /* 服务事件 */
    VOS_UINT8                               aucReserved[3];                     /* 保留位 */
} MTA_AT_MBMS_SERVICE_EVENT_IND_STRU;


typedef struct
{
    AT_MTA_LTE_LOW_POWER_ENUM_UINT8         enLteLowPowerFlg;                   /* 0: Normal;1: Low Power Consumption */
    VOS_UINT8                               aucReserved[3];                     /* 保留位 */
} AT_MTA_LOW_POWER_CONSUMPTION_SET_REQ_STRU;


typedef struct
{
    VOS_UINT32                          aulFreqList[AT_MTA_INTEREST_FREQ_MAX_NUM];  /* 频点列表 */
    AT_MTA_MBMS_PRIORITY_ENUM_UINT8     enMbmsPriority;                             /* VOS_FALSE: 单播优先;VOS_TRUE: MBMS优先 */
    VOS_UINT8                           aucReserved[3];                             /* 保留位 */
} AT_MTA_MBMS_INTERESTLIST_SET_REQ_STRU;


typedef struct
{
    VOS_UINT32                          bitOpSessionId  : 1;
    VOS_UINT32                          bitSpare        : 31;
    VOS_UINT32                          ulAreaId;                               /* Area ID */
    AT_MTA_MBMS_TMGI_STRU               stTMGI;                                 /* TMGI */
    VOS_UINT32                          ulSessionId;                            /* Session ID */
} MTA_AT_MBMS_AVL_SERVICE_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
    VOS_UINT32                          ulAvlServiceNum;                                    /* 可用服务数,0:表示没有可用服务 */
    MTA_AT_MBMS_AVL_SERVICE_STRU        astAvlServices[AT_MTA_MBMS_AVL_SERVICE_MAX_NUM];    /* 可用服务列表 */
} MTA_AT_MBMS_AVL_SERVICE_LIST_QRY_CNF_STRU;


typedef struct
{
    AT_MTA_COEX_BW_TYPE_ENUM_UINT16             enCoexBWType;
    AT_MTA_COEX_CFG_ENUM_UINT16                 enCfg;
    VOS_UINT16                                  usTxBegin;
    VOS_UINT16                                  usTxEnd;
    VOS_INT16                                   sTxPower;
    VOS_UINT16                                  usRxBegin;
    VOS_UINT16                                  usRxEnd;
    VOS_UINT8                                   aucReserved[2];                 /* 保留位 */
} AT_MTA_COEX_PARA_STRU;


typedef struct
{
    VOS_UINT16                          usCoexParaNum;
    VOS_UINT16                          usCoexParaSize;
    AT_MTA_COEX_PARA_STRU               astCoexPara[AT_MTA_ISMCOEX_BANDWIDTH_NUM];
} AT_MTA_LTE_WIFI_COEX_SET_REQ_STRU;


typedef struct
{
    AT_MTA_COEX_PARA_STRU               astCoexPara[AT_MTA_ISMCOEX_BANDWIDTH_NUM];
} MTA_AT_LTE_WIFI_COEX_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT8                           aucMeid[AT_MTA_MEID_DATA_LEN];
    VOS_UINT8                           aucRsv[1];
}AT_MTA_MEID_SET_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;                               /* 查询操作结果*/
    VOS_UINT8                           aucEFRUIMID[MTA_AT_EFRUIMID_OCTET_LEN_EIGHT];
    VOS_UINT8                           aucMeID[MEID_NV_DATA_LEN_NEW + 1];  /*Data*/
    VOS_UINT8                           aucPEsn[PESN_NV_DATA_LEN];      /*Data*/
    VOS_UINT32                          ulMeIDReadRst;
    VOS_UINT32                          ulPEsnReadRst;
}MTA_AT_MEID_QRY_CNF_STRU;





typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;                               /* 查询操作结果*/
    VOS_UINT8                           ucTransMode;                            /* 传输模式*/
    VOS_UINT8                           aucReserved[3];                         /*保留位*/
} MTA_AT_TRANSMODE_QRY_CNF_STRU;


typedef struct
{
    AT_MTA_UE_CENTER_TYPE_ENUM_UINT32   enUeCenter;                             /*UE模式*/
} AT_MTA_SET_UE_CENTER_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;                               /* 查询操作结果*/
} MTA_AT_SET_UE_CENTER_CNF_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;                               /* 查询操作结果*/
    AT_MTA_UE_CENTER_TYPE_ENUM_UINT32   enUeCenter;                             /*UE模式*/
} MTA_AT_QRY_UE_CENTER_CNF_STRU;



typedef struct
{
    VOS_UINT32                          bitOpTa     :1;
    VOS_UINT32                          bitOpSpare  :31;
    VOS_UINT16                          usTa;
    VOS_UINT8                           aucReserved[2];
    VOS_UINT32                          ulMcc;              /* 移动国家码 */
    VOS_UINT32                          ulMnc;              /* 移动网络码 */
    VOS_UINT32                          ulCellID;           /* 小区ID */
    VOS_UINT32                          ulArfcn;            /* 绝对频点号 */
    VOS_UINT16                          usLac;              /* 位置区码 */
    VOS_INT16                           sRssi;              /* Receiving signal strength in dbm */
    MTA_RRC_GSM_BAND_ENUM_UINT16        enBand;             /* GSM频段(0-3) */
    VOS_UINT8                           ucBsic;             /* 小区基站码 */
    VOS_UINT8                           ucRxQuality;        /* IDLE态下或者PS数传态下无效,专用态下填充信道质量值，范围[0,7] ,无效值99*/
}MTA_AT_NETMON_GSM_SCELL_INFO_STRU;

 
typedef struct
{
    VOS_UINT32                                  bitOpCellID  : 1;
    VOS_UINT32                                  bitOpLAC     : 1;
    VOS_UINT32                                  bitOpSpare   : 30;
    VOS_UINT32                                  ulMcc;                  /*移动国家码*/
    VOS_UINT32                                  ulMnc;                  /*移动网络码*/
    VOS_UINT32                                  ulCellID;               /*小区ID*/
    VOS_UINT32                                  ulArfcn;                /*频点*/
    VOS_UINT16                                  usLAC;                  /*位置区码*/
    VOS_UINT8                                   aucReserved[2];
    MTA_NETMON_UTRAN_TYPE_ENUM_UINT32           enCellMeasTypeChoice;   /* NETMON频率信息类型:FDD,TDD */
    union
    {
        MTA_NETMON_UTRAN_SCELL_INFO_FDD_STRU    stCellMeasResultsFDD;    /* FDD */
        MTA_NETMON_UTRAN_SCELL_INFO_TDD_STRU    stCellMeasResultsTDD;    /* TDD */
    }u;
}MTA_AT_NETMON_UTRAN_SCELL_INFO_STRU;


typedef union
{
    MTA_AT_NETMON_GSM_SCELL_INFO_STRU       stGsmSCellInfo;    /* GSM服务小区信息 */
    MTA_AT_NETMON_UTRAN_SCELL_INFO_STRU     stUtranSCellInfo;  /* WCDMA服务小区信息 */
    MTA_NETMON_EUTRAN_SCELL_INFO_STRU       stLteSCellInfo;    /* LTE服务小区信息*/
}MTA_AT_NETMON_SCELL_INFO_UN;


typedef struct
{
    VOS_UINT32                          bitOpBsic     : 1;
    VOS_UINT32                          bitOpCellID   : 1;
    VOS_UINT32                          bitOpLAC      : 1;
    VOS_UINT32                          bitOpSpare    : 29;
    VOS_UINT32                          ulCellID;       /*小区ID*/
    VOS_UINT32                          ulAfrcn;        /*频点*/
    VOS_UINT16                          usLAC;          /*位置区码*/
    VOS_INT16                           sRSSI;          /* 频点的RSSI */
    MTA_RRC_GSM_BAND_ENUM_UINT16        enBand;         /* band 0-3 */
    VOS_UINT8                           ucBsic;         /*小区基站码*/
    VOS_UINT8                           ucReserved;
}MTA_AT_NETMON_GSM_NCELL_INFO_STRU;


typedef struct
{
    VOS_UINT32                          ulArfcn;          /*频点*/
    VOS_UINT16                          usPSC;            /*主扰码*/
    VOS_INT16                           sECN0;            /*ECN0*/
    VOS_INT16                           sRSCP;            /*RSCP*/
    VOS_UINT8                           aucReserved[2];
}MTA_AT_NETMON_UTRAN_NCELL_INFO_FDD_STRU;


typedef struct
{
    VOS_UINT32                          ulArfcn;          /*频点*/
    VOS_UINT16                          usSC;             /*扰码*/
    VOS_UINT16                          usSyncID;         /*下行导频码*/
    VOS_INT16                           sRSCP;            /*RSCP*/
    VOS_UINT8                           aucReserved[2];
}MTA_AT_NETMON_UTRAN_NCELL_INFO_TDD_STRU;


typedef struct
{
    VOS_UINT8                                               ucGsmNCellCnt;                               /* GSM 临区个数 */
    VOS_UINT8                                               ucUtranNCellCnt;                             /* WCDMA 临区个数 */
    VOS_UINT8                                               ucLteNCellCnt;                               /* LTE 临区个数 */
    VOS_UINT8                                               ucReserved;
    MTA_AT_NETMON_GSM_NCELL_INFO_STRU                       astGsmNCellInfo[NETMON_MAX_GSM_NCELL_NUM];   /* GSM 临区数据结构*/
    MTA_NETMON_UTRAN_TYPE_ENUM_UINT32                       enCellMeasTypeChoice;                        /* NETMON频率信息类型:FDD,TDD */
    union
    {
        MTA_AT_NETMON_UTRAN_NCELL_INFO_FDD_STRU             astFddNCellInfo[NETMON_MAX_UTRAN_NCELL_NUM]; /* FDD临区数据结构 */
        MTA_AT_NETMON_UTRAN_NCELL_INFO_TDD_STRU             astTddNCellInfo[NETMON_MAX_UTRAN_NCELL_NUM]; /* TDD临区数据结构 */
    }u;
    MTA_NETMON_LTE_NCELL_INFO_STRU                          astLteNCellInfo[NETMON_MAX_LTE_NCELL_NUM];   /* LTE 临区数据结构 */
}MTA_AT_NETMON_NCELL_INFO_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32                   enResult;               /* 查询结果是否成功 */
    MTA_NETMON_CELL_TYPE_ENUM_UINT32            enCellType;             /* 0:查询服务小区，1:查询临区 */
    MTA_AT_NETMON_NCELL_INFO_STRU               stNCellInfo;            /*临区信息*/
    MTA_AT_NETMON_CELL_INFO_RAT_ENUM_UINT32     enRatType;              /* 服务小区的接入技术类型 */
    MTA_AT_NETMON_SCELL_INFO_UN                 unSCellInfo;
} MTA_AT_NETMON_CELL_INFO_STRU;




typedef struct
{
    VOS_UINT32                          ulReserveValue;
} AT_MTA_QRY_AFC_CLK_FREQ_XOCOEF_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;

    AT_MTA_AFC_CLK_STATUS_ENUM_UINT32   enStatus;
    VOS_INT32                           lDeviation;                                 /* 频偏 */

    /* 温度范围 */
    VOS_INT16                           sCoeffStartTemp;
    VOS_INT16                           sCoeffEndTemp;

    /* 多项式系数 */
    VOS_UINT32                          aulCoeffMantissa[AT_MTA_GPS_XO_COEF_NUM];  /* a0,a1,a2,a3尾数 */
    VOS_UINT16                          ausCoeffExponent[AT_MTA_GPS_XO_COEF_NUM];  /* a0,a1,a2,a3指数 */

} MTA_AT_QRY_AFC_CLK_FREQ_XOCOEF_CNF_STRU;



typedef struct
{
    VOS_UINT32                          ulDoSysEvent;
}AT_MTA_EVDO_SYS_EVENT_SET_REQ_STRU;


typedef struct
{
    VOS_UINT32                          ulDoSigMask;
}AT_MTA_EVDO_SIG_MASK_SET_REQ_STRU;


typedef struct
{
    VOS_UINT32                          ulParaLen;
    VOS_UINT8                           aucContent[4];
} MTA_AT_EVDO_REVA_RLINK_INFO_IND_STRU;


typedef struct
{
    VOS_UINT32                          ulParaLen;
    VOS_UINT8                           aucContent[4];
} MTA_AT_EVDO_SIG_EXEVENT_IND_STRU;


typedef struct
{
    AT_MTA_XCPOSR_CFG_ENUM_UNIT8        enXcposrEnableCfg;                            /* GPS芯片是否支持清除辅助定位信息，上电开机默认为不支持 */
    VOS_UINT8                           aucReserved[3];
} AT_MTA_SET_XCPOSR_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;                               /* 命令执行结果 */
    AT_MTA_XCPOSR_CFG_ENUM_UNIT8        enXcposrEnableCfg;
    VOS_UINT8                           aucReserved[3];
} MTA_AT_QRY_XCPOSR_CNF_STRU;


typedef struct
{
    VOS_UINT8                           ucXcposrRptFlg;                         /* +XCPOSRRPT命令主动上报控制，上电开机默认为不允许主动上报 */
    VOS_UINT8                           aucReserved[3];
} AT_MTA_SET_XCPOSRRPT_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;                               /* 命令执行结果 */
    VOS_UINT8                           ucXcposrRptFlg;
    VOS_UINT8                           aucReserved[3];
} MTA_AT_QRY_XCPOSRRPT_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulSensorStat;                              /* Sensor Hub 状态 */
}AT_MTA_SET_SENSOR_REQ_STRU;

typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
} MTA_AT_SET_SENSOR_CNF_STRU;


typedef struct
{
    VOS_UINT8                           ucScreenStat;                              /* Screen 状态 */
    VOS_UINT8                           aucReserved[3];
}AT_MTA_SET_SCREEN_REQ_STRU;

typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
} MTA_AT_SET_SCREEN_CNF_STRU;


typedef struct
{
    AT_MTA_CFG_ENUM_UINT8               enCfg;                                  /* 0:关闭,1:打开 */
    VOS_UINT8                           aucReserved[3];                         /* 保留位 */
} AT_MTA_SET_RXTESTMODE_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;                               /* 操作结果 */
}MTA_AT_SET_RXTESTMODE_CNF_STRU;

typedef struct
{
    VOS_UINT16                          usCause;
    VOS_UINT16                          usReserved;        /* 保留位 */
}MTA_AT_AFC_CLK_UNLOCK_IND_STRU;


typedef struct
{
    VOS_UINT8                           ucGroupID;                             /* 器件号 */
    VOS_UINT8                           ucDeviceID;                            /* 设备号 */
    VOS_UINT8                           ucCaseID;                              /* 器件最优频率类号 */
    VOS_UINT8                           ucMode;                                /* 主动上报方式 */
} MTA_AT_DEVICE_FREQ_STRU;


typedef struct
{
    VOS_UINT8                           ucRptDeviceNum;                                    /* 要上报的激活器件数目 */
    VOS_UINT8                           aucReserved[3];                                    /* 保留位 */
    MTA_AT_DEVICE_FREQ_STRU             astDeviceFreqList[MTC_MTA_MAX_BESTFREQ_GROUPNUM];   /* 8组器件最优频率列表 */
} MTA_AT_BEST_FREQ_CASE_IND_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;                               /* 消息处理结果 */
    VOS_UINT8                           ucActiveDeviceNum;
    VOS_UINT8                           aucReserved[3];
    MTA_AT_DEVICE_FREQ_STRU             astDeviceInfoList[MTC_MTA_MAX_BESTFREQ_GROUPNUM];
}MTA_AT_BESTFREQ_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT32                           ucRatType;                              /*0-GSM 1-WCDMA*/
} AT_MTA_SLAVE_SET_REQ_STRU;

typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
    VOS_UINT16                          usRfic0DieIdValid;                       /*1-存在， 0-不存在*/
    VOS_UINT16                          usRfic1DieIdValid;
    VOS_UINT16                          usRfic2DieIdValid;
    VOS_UINT16                          usRfic3DieIdValid;
    VOS_UINT16                          ausRfic0DieId[MTA_RCM_MAX_DIE_ID_LEN];
    VOS_UINT16                          ausRfic1DieId[MTA_RCM_MAX_DIE_ID_LEN];
    VOS_UINT16                          ausRfic2DieId[MTA_RCM_MAX_DIE_ID_LEN];
    VOS_UINT16                          ausRfic3DieId[MTA_RCM_MAX_DIE_ID_LEN];
} MTA_AT_RFIC_DIE_ID_REQ_CNF_STRU;


typedef struct
{
    VOS_UINT16                          usCmdType;                              /* 命令类型 */
    VOS_UINT16                          usRatBitmap;                            /* 支持的rat模式，比bitmap */
    VOS_UINT32                          ulPara1;                                /* 配置参数1 */
    VOS_UINT32                          ulPara2;                                /* 配置参数2 */
    VOS_UINT32                          ulPara3;                                /* 配置参数3 */
} AT_MTA_PHY_COM_CFG_SET_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
} MTA_AT_PHY_COM_CFG_SET_CNF_STRU;


typedef struct
{
    AT_MTA_CFG_ENUM_UINT8               enEnable;                               /* 0关闭主动上报，1打开主动上报 */
    VOS_UINT8                           aucReserved[3];                         /* 保留位 */
}AT_MTA_SET_CRRCONN_REQ_STRU;


typedef struct
{
    VOS_UINT8                           ucStatus0;
    VOS_UINT8                           ucStatus1;
    VOS_UINT8                           ucStatus2;
    VOS_UINT8                           ucReserved;                             /* 保留位 */
}MTA_AT_CRRCONN_STATUS_IND_STRU;


typedef MTA_AT_RESULT_CNF_STRU MTA_AT_SET_CRRCONN_CNF_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
    AT_MTA_CFG_ENUM_UINT8               enEnable;
    VOS_UINT8                           ucStatus0;
    VOS_UINT8                           ucStatus1;
    VOS_UINT8                           ucStatus2;
} MTA_AT_QRY_CRRCONN_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulEnable;                               /* 0关闭主动上报，1打开主动上报 */
    VOS_UINT32                          ulThreshold;                            /* LPDCP信息上报阈值 */
}AT_MTA_SET_VTRLQUALRPT_REQ_STRU;


typedef struct
{
    VOS_INT16                           sRsrp;                                  /*RSRP测量结果*/
    VOS_INT16                           sRsrq;                                  /*RSRQ测量结果*/
    VOS_INT16                           sRssi;                                  /*RSSI测量结果*/
    VOS_UINT16                          usBler;                                 /*误码率*/
} MTA_AT_RL_QUALITY_INFO_IND_STRU;


typedef struct
{
    VOS_UINT32                          ulCurrBuffTime;                         /*最老数据包缓存时长*/
    VOS_UINT32                          ulCurrBuffPktNum;                       /*当前缓存数据包个数*/
    VOS_UINT32                          ulMacUlThrput;                          /*MAC上行速率,只包含上行新授权速率，不包含重传授权。单位:bytes/s*/
    VOS_UINT32                          ulMaxBuffTime;                          /*最大缓存时间，单位ms*/
}MTA_AT_VIDEO_DIAG_INFO_RPT_STRU;


typedef MTA_AT_RESULT_CNF_STRU MTA_AT_SET_VTRLQUALRPT_CNF_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
    VOS_UINT8                           aucDieSN[MTA_PMU_MAX_DIE_SN_LEN];
} MTA_AT_PMU_DIE_SN_REQ_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulExtendFlag;
    VOS_UINT32                          ulMipiId;
    VOS_UINT32                          ulSlaveId;
    VOS_UINT32                          ulRegAddr;
    VOS_UINT32                          ulValue;
    VOS_UINT32                          ulByteCnt;
} AT_MTA_MIPI_WREX_REQ_STRU;


typedef struct
{
    VOS_UINT32                          ulExtendFlag;
    VOS_UINT32                          ulMipiId;
    VOS_UINT32                          ulSlaveId;
    VOS_UINT32                          ulRegAddr;
    VOS_UINT32                          ulByteCnt;
    VOS_UINT32                          ulSpeedType;
} AT_MTA_MIPI_RDEX_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
} MTA_AT_MIPI_WREX_CNF_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
    VOS_UINT32                          ulValue;
} MTA_AT_MIPI_RDEX_CNF_STRU;


typedef struct
{
    AT_MTA_MODEM_CAP_UPDATE_TYPE_ENUM_UINT8                 enModemCapUpdateType;   /* MODEM平台能力更新类型 */
    VOS_UINT8                                               aucReserved[3];
} AT_MTA_MODEM_CAP_UPDATE_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32                               enRslt;                 /* MODEM平台能力更新结果 */
} MTA_AT_MODEM_CAP_UPDATE_CNF_STRU;


typedef struct
{
    VOS_UINT8                           ucDlCategery;                           /* 下行category */
    VOS_UINT8                           ucUlCategery;                           /* 上行category */
    VOS_UINT8                           aucReserved[2];                         /* 保留位 */
} MTA_AT_LTE_CATEGORY_INFO_IND_STRU;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/
/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

/*****************************************************************************
  H2ASN顶级消息结构定义
*****************************************************************************/
typedef struct
{
    AT_MTA_MSG_TYPE_ENUM_UINT32         ulMsgId;                                /*_H2ASN_MsgChoice_Export AT_MTA_MSG_TYPE_ENUM_UINT32*/
    AT_APPCTRL_STRU                     stAppCtrl;
    VOS_UINT8                           aucMsgBlock[4];
    /***************************************************************************
        _H2ASN_MsgChoice_When_Comment          AT_MTA_MSG_TYPE_ENUM_UINT32
    ****************************************************************************/
}AT_MTA_MSG_DATA;
/*_H2ASN_Length UINT32*/

typedef struct
{
    VOS_MSG_HEADER
    AT_MTA_MSG_DATA                     stMsgData;
}AtMtaInterface_MSG;


typedef struct
{
    VOS_INT32                           lSec;
    VOS_INT32                           lMin;
    VOS_INT32                           lHour;
    VOS_INT32                           lDay;
    VOS_INT32                           lMonth;
    VOS_INT32                           lYear;
    VOS_INT32                           lZone;
}AT_MTA_MODEM_TIME_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
} MTA_AT_SET_TIME_CNF_STRU;


typedef struct
{
    VOS_UINT8                           ucDeviceID;      /* 需要设置的干扰器件ID, 0表示全部，1-255分别对应特定器件 */
    VOS_UINT8                           ucMode;          /* 0表示关闭主动上报，1表示开启主动上报 */
    VOS_UINT8                           aucReserved[2];
}AT_MTA_BESTFREQ_SET_REQ_STRU;



enum MTA_AT_RCM_TAS_TEST_RATMODE_ENUM
{
    MTA_AT_RATMODE_GSM                  = 0,
    MTA_AT_RATMODE_WCDMA                = 1,
    MTA_AT_RATMODE_LTE                  = 2,
    MTA_AT_RATMODE_TDS                  = 3,
    MTA_AT_RATMODE_1X                   = 4,
    MTA_AT_RATMODE_HRPD                 = 5,
    MTA_AT_RATMODE_BUTT
};
typedef VOS_UINT32 MTA_AT_TAS_TEST_RATMODE_ENUM_UINT32;


typedef struct
{
    MTA_AT_TAS_TEST_RATMODE_ENUM_UINT32 enRatMode;                                      /* 1-WCDMA   2-TLE */
    VOS_UINT32                          ulPara0;
    VOS_UINT32                          ulPara1;
    VOS_UINT32                          ulPara2;
    VOS_UINT32                          ulPara3;
} AT_MTA_TAS_TEST_CFG_STRU;


typedef struct
{
    VOS_RATMODE_ENUM_UINT32             enRatMode;                                      /* 1-WCDMA   2-TLE */
} AT_MTA_TAS_TEST_QRY_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
} MTA_AT_TAS_TEST_CFG_CNF_STRU;


typedef struct
{
    VOS_UINT16                          uhwSrcAntTimeLength;                    /* 源(当前)天线时长，单位:ms */
    VOS_UINT16                          uhwDestAntTimeLength;                   /* 目标天线时长，单位:ms */
    VOS_INT16                           shwSrcAntTxPowerGain;                   /* 源(当前)天线发射功率提升值，单位:0.1dB */
    VOS_INT16                           shwDestAntTxPowerGain;                  /* 目标天线发射功率提升值，单位:0.1dB */
}MTA_AT_TX_STATEII_LEVEL_STRU;


#define MAX_STATEII_LEVEL_ITEM          4
typedef struct
{
    VOS_UINT16                          uhwLevelNum;
    VOS_UINT16                          uhwRsv;
    MTA_AT_TX_STATEII_LEVEL_STRU        astLevelItem[MAX_STATEII_LEVEL_ITEM];
}MTA_AT_TX_STATEII_LEVEL_TABLE_STRU;



typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32               enResult;
    MTA_AT_TAS_TEST_RATMODE_ENUM_UINT32     enRatMode;
    VOS_UINT32                              ulCurrLevel;                        /* 当前档位索引 */
    MTA_AT_TX_STATEII_LEVEL_STRU            stLevelInfo;                        /* 当前档位的具体内容 */
    MTA_AT_TX_STATEII_LEVEL_TABLE_STRU      stUsedTable;                        /* 档位表 */
}MTA_AT_TAS_TEST_QRY_CNF_STRU;


typedef struct
{
    AT_MTA_RS_INFO_TYPE_ENUM_UINT32     enRsInfoType;                           /* 0:查询RSRP，1:查询RSRQ */
}AT_MTA_RS_INFO_QRY_REQ_STRU;


typedef struct
{
    VOS_UINT32                          ulCellId;                               /* 小区ID */
    VOS_UINT32                          ulEarfcn;                               /* 小区频点 */
    VOS_INT32                           lRsrp;                                  /* 信号接收功率，LRRC乘100后的值，AT上报要带两位小数 */
}MTA_AT_RSRP_INFO_STRU;


typedef struct
{
    VOS_UINT32                          ulCellId;                               /* 小区ID */
    VOS_UINT32                          ulEarfcn;                               /* 小区频点 */
    VOS_INT32                           lRsrq;                                  /* 信号接收质量，LRRC乘100后的值，AT上报要带两位小数 */
}MTA_AT_RSRQ_INFO_STRU;


typedef struct
{
    AT_MTA_RS_INFO_TYPE_ENUM_UINT32     enRsInfoType;                           /* 0:查询RSRP，1:查询RSRQ */
    VOS_UINT32                          ulRsInfoNum;
    union
    {
        MTA_AT_RSRP_INFO_STRU           astRsrpInfo[AT_MAX_RS_INFO_NUM];        /* RSRP测量结果 */
        MTA_AT_RSRQ_INFO_STRU           astRsrqInfo[AT_MAX_RS_INFO_NUM];        /* RSRQ测量结果 */
    }u;
}MTA_AT_RS_INFO_RSLT_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
    MTA_AT_RS_INFO_RSLT_STRU            stRsInfoRslt;
}MTA_AT_RS_INFO_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT32                             ulErrcCapType;
    VOS_UINT32                             ulPara1;
    VOS_UINT32                             ulPara2;
    VOS_UINT32                             ulPara3;
} AT_MTA_ERRCCAP_CFG_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
}MTA_AT_ERRCCAP_CFG_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulErrcCapType;
} AT_MTA_ERRCCAP_QRY_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
    VOS_UINT32                          ulErrcCapType;
    VOS_UINT32                          ulPara1;
    VOS_UINT32                          ulPara2;
    VOS_UINT32                          ulPara3;
} MTA_AT_ERRCCAP_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT32                                              ulMcc;
    VOS_UINT32                                              ulMnc;
    VOS_UINT32                                              ulCellId;
    VOS_UINT32                                              ulLac;
    AT_MTA_SYS_MODE_ENUM_UINT32                             enSysmode;
    AT_MTA_PSEUCELL_NOTIFY_TYPE_ENUM_UINT8                  enPseucellNotifyType;   /* 0:为短信识别；其他:不支持 */
    VOS_UINT8                                               aucReserved[3];
}AT_MTA_PSEUCELL_INFO_SET_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
}MTA_AT_PSEUCELL_INFO_SET_CNF_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
    VOS_INT16                           sPowerDet;                              /* 功率检测结果，0.1dBm精度 */
    VOS_UINT8                           aucReserved1[2];                        /* 保留位 */
}MTA_AT_POWER_DET_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT16                          usReadType;
    VOS_UINT16                          usMipiId;
    VOS_UINT16                          usSlaveId;
    VOS_UINT16                          usRegAddr;
    VOS_UINT16                          usSpeedType;
    VOS_UINT16                          usReadBitMask;
    VOS_UINT16                          usReserved1;
    VOS_UINT16                          usReserved2;
} AT_MTA_MIPI_READ_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;      /* 操作结果 */
    VOS_UINT32                          ulValue;
}MTA_AT_MIPI_READ_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulEnableFlag;
} AT_MTA_NO_CARD_MODE_SET_REQ_STRU;

typedef MTA_AT_RESULT_CNF_STRU MTA_AT_NO_CARD_MODE_SET_CNF_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
    VOS_UINT32                          ulEnableFlag;
} MTA_AT_NO_CARD_MODE_QRY_CNF_STRU;



typedef struct
{
    AT_MTA_CHR_ALARM_RLAT_OP_ENUM_UINT32                    enAlarmOp;
    VOS_UINT32                                              ulAlarmId;
    VOS_UINT32                                              ulAlarmDetail;
}AT_MTA_CHR_ALARM_RLAT_CFG_SET_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32                               enResult;
    AT_MTA_CHR_ALARM_RLAT_OP_ENUM_UINT32                    enAlarmOp;
    VOS_UINT32                                              ulAlarmDetail;
}MTA_AT_CHR_ALARM_RLAT_CFG_SET_CNF_STRU;


typedef struct
{
    VOS_UINT16                          usTransactionId;
    VOS_UINT8                           ucMsgBodyType;
    VOS_UINT8                           ucLocSource;
    VOS_UINT8                           ucEndFlag;
    VOS_UINT8                           ucCommonIEValidFlg;
    VOS_UINT8                           ucId;
    VOS_UINT8                           ucLocCalcErr;
    VOS_UINT8                           ucNameLength;
    VOS_UINT8                           aucReserved[3];
    VOS_UINT8                           aucName[AT_MTA_EPDU_NAME_LENGTH_MAX];
    VOS_UINT8                           ucTotalNum;
    VOS_UINT8                           ucIndex;
    VOS_UINT16                          usEpduLength;
    VOS_UINT8                           aucEpduContent[AT_MTA_EPDU_CONTENT_LENGTH_MAX];
    VOS_UINT8                           aucReserved1[2];
} AT_MTA_EPDU_SET_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
} MTA_AT_EPDU_SET_CNF_STRU;


typedef struct
{
    VOS_UINT32                                  ulFreqBegin;                    /* 单位100khz */
    VOS_UINT32                                  ulFreqEnd;                      /* 单位100khz */
}AT_MTA_FREQ_RANGE_STRU;


typedef struct
{
    VOS_UINT16                                  usBandInd;                      /* Band:1-255 */
    VOS_UINT8                                   ucReserved;
    VOS_UINT8                                   ucFreqRangeNum;                 /* 0-4,0:表示去激活整个Band，频率范围是协议默认范围 */
    AT_MTA_FREQ_RANGE_STRU                      astFreqRange[AT_MTA_MAX_SUB_BAND_NUM];
}AT_MTA_BAND_INFO_STRU;


typedef struct
{
    VOS_UINT8                                   ucCaA2Flg;                     /* CA A2功能开关 */
    VOS_UINT8                                   ucCaA4Flg;                     /* CA A4功能开关 */
    VOS_UINT8                                   ucCaCqiFlg;                    /* CA cqi=0功能开关 */
    VOS_UINT8                                   aucReserved[3];
    VOS_UINT16                                  usBandNum;                     /* 0-8,0:表示去使能所有的CA SCell */
    AT_MTA_BAND_INFO_STRU                       astBandInfo[AT_MTA_MAX_BAND_NUM];
}AT_MTA_CA_INFO_STRU;


typedef struct
{
    VOS_UINT8                                   ucCaActFlag;                    /* 使能去使能Ca标志，0:去使能  1:使能 */
    VOS_UINT8                                   aucReserved[3];                 /* 保留位 */
    AT_MTA_CA_INFO_STRU                         stCaInfo;
}AT_MTA_CA_CFG_SET_REQ_STRU;


typedef struct
{
    VOS_UINT8                           ucCellIndex;                            /* 小区索引, 0: PCell, 1~7: SCell */
    MTA_AT_BAND_WIDTH_ENUM_UINT8        enBandWidth;                            /* SCell占用带宽 */
    VOS_UINT16                          usBandInd;                              /* SCell频段 */

    VOS_UINT8                           ucUlConfigured;                         /* CA上行是否被配置，0:未配置，1:已配置 */
    VOS_UINT8                           ucDlConfigured;                         /* CA下行是否被配置，0:未配置，1:已配置 */
    VOS_UINT8                           ucActived;                              /* SCell是否被激活，0:去激活，1:激活 */
    VOS_UINT8                           ucLaaScellFlg;                          /* 本SCell是否为LaaScell, 0:不是, 1:是 */

    VOS_UINT32                          ulEarfcn;                               /* SCell频点 */
}MTA_AT_CA_CELL_INFO_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;                               /* 操作结果 */
    VOS_UINT32                          ulTotalCellNum;                         /* 上报消息中有效Cell的个数 */
    MTA_AT_CA_CELL_INFO_STRU            astCellInfo[MTA_AT_CA_MAX_CELL_NUM];    /* 前ulTotalCellNum个元素有效 */
}MTA_AT_CA_CELL_INFO_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulTotalCellNum;                         /* 上报消息中有效Cell的个数 */
    MTA_AT_CA_CELL_INFO_STRU            astCellInfo[MTA_AT_CA_MAX_CELL_NUM];    /* 前ulTotalCellNum个元素有效 */
}MTA_AT_CA_CELL_INFO_NTF_STRU;


typedef struct
{
    VOS_UINT8                            ucFineTimeType;
    VOS_UINT8                            aucReserved[3];
} AT_MTA_FINE_TIME_SET_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
} MTA_AT_FINE_TIME_SET_CNF_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
    VOS_INT32                           lSinr;                                  /*当前信噪比 */
    VOS_UINT8                           aucUtcTime[MTA_AT_UTC_MAX_LENGTH];      /*(0..549755813887) */
    VOS_UINT8                           aucUtcTimeOffset[MTA_AT_UTC_MAX_LENGTH];/*(0..549755813887) */
    VOS_INT16                           sTa;                                    /*网侧配置的时间提前量 */
    VOS_INT16                           sLeapSecond;                            /*闰秒调整值 */
    VOS_UINT8                           ucLeapSecondValid;                      /*是否存在闰秒调整 */
    MTA_AT_FINE_TIME_RAT_ENUM_UINT8     enRat;
    MTA_AT_PROTOCOL_STATE_ENUM_UINT8    enState;                                /*当前LRRC协议状态 */
    VOS_UINT8                           aucReserved[3];
} MTA_AT_SIB_FINE_TIME_IND_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
    VOS_UINT16                          usSysFn;
    VOS_UINT8                           aucReserved [2];
} MTA_AT_LPP_FINE_TIME_IND_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
} MTA_AT_CA_CELL_SET_CNF_STRU;


typedef struct
{
   VOS_BOOL                             blEnableType;
} AT_MTA_CA_CELL_SET_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
    VOS_BOOL                            blEnableType;
} MTA_AT_CA_CELL_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulAckType;                              /* ACK类型 */
    VOS_UINT32                          ulAckValue1;                            /* ACK值1 */
    VOS_UINT32                          ulAckValue2;                            /* ACK值2 */
    VOS_UINT32                          ulAckValue3;                            /* ACK值3 */
}MTA_AT_PHY_COMM_ACK_IND_STRU;


typedef struct
{
   AT_MTA_SMS_DOMAIN_ENUM_UINT8         enSmsDomain;
   VOS_UINT8                            aucReserved[3];
} AT_MTA_SMS_DOMAIN_SET_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
} MTA_AT_SMS_DOMAIN_SET_CNF_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
    AT_MTA_SMS_DOMAIN_ENUM_UINT8        enSmsDomain;
    VOS_UINT8                           aucReserved[3];
} MTA_AT_SMS_DOMAIN_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulMcc;
    VOS_UINT32                          ulMnc;
    VOS_UINT8                           ucLength;
    VOS_UINT8                           aucEpsNetwork[MTA_AT_EPS_NETWORK_BYTE];
    VOS_UINT8                           ucReserved;
}MTA_AT_EPS_NETWORK_INFO_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
    MTA_AT_EPS_NETWORK_INFO_STRU        stEpsNetworkInfo;
}MTA_AT_EPS_NETWORK_QRY_CNF_STRU;


typedef struct
{
   VOS_UINT32                            ulLocationPermitFlag;
} AT_MTA_GPSLOCSET_SET_REQ_STRU;


typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
}MTA_AT_GPSLOCSET_SET_CNF_STRU;



typedef struct
{
    MTA_AT_RESULT_ENUM_UINT32           enResult;
    TIME_ZONE_TIME_STRU                 stTime;
}MTA_AT_CCLK_QRY_CNF_STRU;



typedef struct
{
    VOS_UINT32           ulPseudBtsType;
}MTA_AT_PSEUD_BTS_IDENT_IND_STRU;
/*****************************************************************************
  10 函数声明
*****************************************************************************/


#if ((VOS_OS_VER == VOS_WIN32) || (VOS_OS_VER == VOS_NUCLEUS))
#pragma pack()
#else
#pragma pack(0)
#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

