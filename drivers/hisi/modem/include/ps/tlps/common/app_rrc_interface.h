/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
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

#ifndef __APP_RRC_INTERFACE_H__
#define __APP_RRC_INTERFACE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#include  "lps_common.h"



#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif

/*****************************************************************************
  2 macro
*****************************************************************************/
#define RRC_APP_CELL_MAX_COUNT           (32)
#define RRC_APP_MAX_NUM_OF_MCC           (3)


/*typedef DT_CmdUint8 APP_CmdUint8;*/
/*typedef DT_ResultUint32 APP_ResultUint32;*/

/* RRC发起随机接入到MAC收到rar */
#define PS_OM_EST_PTL_SND_MACRA_TO_RCV_RAR_TIME      (11)           /* 11ms T4-T1 */
/* 从mac收到rar到发送消息3时间 */
#define PS_OM_EST_PTL_RCV_RAR_TO_SND_MSG3_TIME       (3)            /* 2.5ms T5-T4*/
/* 发送消息3到收到setup消息 */
#define PS_OM_EST_PTL_SND_MSG3_TO_RCV_SETUP_TIME     (73)           /* 28.5ms+2*Ts1c  T6-T5*/
/* 处理setup消息协议时间 */
#define PS_OM_EST_PTL_SETUP_TO_CMPL_TIME             (3)             /* 3ms T7-T6*/
/* 发起随机接入到发送setup cmpl协议时间 */
#define PS_OM_EST_PTL_RRC_SND_MACRA_TO_CMPL_TIME     (92)            /* 47.5ms+2*Ts1c--(50.5~92.5ms) T7-T1*/
/* 建链用户面时延协议值 */
#define PS_OM_EST_PTL_USER_PLANE_TIME                (106)            /* 61ms+2*Ts1c+Ts1u--66~106ms T13-T1*/

/* 小区搜索协议时间 */
#define PS_OM_REEST_PTL_SEARCH_CELL_TIME             (100)            /* <100ms */
/* 系统消息更新协议时间 */
#define PS_OM_REEST_PTL_RCV_SI_TIME                  (1280)           /* <1280ms */
/* 检测到链路失败到在新小区上发起随机接入协议时间 */
#define PS_OM_REEST_PTL_RCV_REEST_IND_TO_MACRA       (1500)           /* <1500ms */

/*  物理层切换开始到结束协议时间 */
#define PS_OM_HO_PTL_HO_REQ_TO_CNF                   (1)              /* <1ms T6-T5*/

/* 随机接入请求到收到Cnf的协议时间 */
#define PS_OM_HO_PTL_RRC_MACRA_REQ_TO_MACRA_CNF      (11)             /* 2.5 + 1+ 7.5ms T8-T7*/

/* 发送切换完成消息到收到确认消息协议时间 */
#define PS_OM_HO_PTL_SND_CMPL_TO_CMPL_CNF            (13)             /* 13ms T10-T9*/

/* 收到切换重配消息到发起随机接入协议时间 */
#define PS_OM_HO_PTL_RCV_RBCFG_TO_MACRA_REQ          (50)             /* 50ms T7-T4*/

/* 收到切换重配置消息到回复数传用户面时延协议时间 */
#define PS_OM_HO_PTL_USER_PLANE_TIME                 (100)            /* 100ms T11-T4*/

/* 受限列表中保存最大的条目数 */
#define RRC_APP_MAX_LIMITED_ITEM_COUNT         (32)
/* 新增测量消息中上报小区最大个数*/
#define LRRC_LCSEL_INTRA_CELL_REPORT_NUM 4
#define LRRC_LCSEL_INTER_CELL_REPORT_NUM 4
#define LRRC_LCSEL_UTRAN_CELL_REPORT_NUM 4
#define LRRC_LCSEL_INTER_FREQ_REPORT_NUM 9

#define LRRC_SCELL_MAX_NUM 4
#define LRRC_INTRA_CELL_MAX_NUM 16
#define LRRC_INTER_CELL_MAX_NUM 16
#define LRRC_UTRAN_CELL_MAX_NUM 16
#define LRRC_GERAN_CELL_MAX_NUM 16


#define LRRC_ECELL_QRY_CELL_REPORT_NUM 9
#define LRRC_ECELL_QRY_NBRCELL_NUM 8
#define LRRC_ECELL_QRY_SELL_REPORT_NUM 4
#define RRC_ECELL_QRY_INVALID_UINT8        0XFF
#define RRC_ECELL_QRY_INVALID_UINT16       0XFFFF
#define RRC_ECELL_QRY_INVALID_UINT32       0XFFFFFFFF

/* UE支持的UTRAN频点的最大测量数目 */
#define LRRC_TRRC_PHY_MAX_SUPPORT_CARRIER_NUM               9

#define LRRC_GURRC_GERAN_ARFCN_MAX_NUM                      32


/* 存放BAND信息最大的数组 */
#define LRRC_APP_MAX_BAND_IND_ARRAY_NUM                     (8)

#define LRRC_APP_MAX_MEAS_CELL_NUM                          (18)
#define LRRC_APP_MAX_GERAN_MEAS_CELL_NUM                          (32)


#define LRRC_APP_LWCLASH_MAX_SCELL_NUM                      (4)



/*****************************************************************************
  3 Massage Declare
*****************************************************************************/
/*****************************************************************************
 枚举名    : APP_RRC_MsgId
 协议表格  :
 ASN.1描述 :
 枚举说明  : RRC <-> APP 接口消息ID
*****************************************************************************/
enum APP_RRC_MsgId {
    /* RRC发给APP的原语 */
    ID_APP_RRC_TRANSPARENT_CMD_REQ      = (PS_MSG_ID_APP_TO_RRC_BASE + 0x00),   /* _H2ASN_MsgChoice APP_RRC_TRANSPARENT_CMD_REQ_STRU */
    ID_APP_RRC_CELL_MEAS_RPT_REQ        = (PS_MSG_ID_APP_TO_RRC_BASE + 0x01),   /* _H2ASN_MsgChoice APP_RRC_CellMeasRptReq */
    ID_APP_RRC_TIME_DELAY_RPT_REQ       = (PS_MSG_ID_APP_TO_RRC_BASE + 0x02),   /* _H2ASN_MsgChoice APP_RRC_TimeDelayRptReq */
    ID_APP_RRC_INQ_CAMP_CELL_INFO_REQ   = (PS_MSG_ID_APP_TO_RRC_BASE + 0x03),   /* _H2ASN_MsgChoice APP_RRC_INQ_CAMP_CELL_INFO_REQ_STRU */
    ID_APP_RRC_LOCK_WORK_INFO_REQ       = (PS_MSG_ID_APP_TO_RRC_BASE + 0x04),   /* _H2ASN_MsgChoice APP_RRC_LOCK_WORK_INFO_REQ_STRU */
    /* MSP_L4_L4A->ERRC */
    /* _MSGPARSE_Interception APP_RRC_CsqReq I0->IO, I1->I1 */
    ID_APP_RRC_CSQ_REQ                  = (PS_MSG_ID_APP_TO_RRC_BASE + 0x05),
    /* ERR->ERR */
    ID_APP_RRC_PTL_STATE_QUERY_REQ      = (PS_MSG_ID_APP_TO_RRC_BASE + 0x06),   /* _H2ASN_MsgChoice APP_RRC_PTL_STATE_QUERY_REQ_STRU */
    ID_APP_RRC_CELL_INFO_QUERY_REQ      = (PS_MSG_ID_APP_TO_RRC_BASE + 0x07),   /* _H2ASN_MsgChoice APP_RRC_CELL_INFO_QUERY_REQ_STRU */
    /* MSP_L4_L4A->ERRC */
    /* _MSGPARSE_Interception APP_RRC_LwclashReq I0->IO, I1->I1 */
    ID_APP_RRC_LWCLASH_REQ              = (PS_MSG_ID_APP_TO_RRC_BASE + 0x08),
    /* ERR->ERR */
    ID_APP_RRC_SERVING_CELL_INFO_QUERY_REQ   = (PS_MSG_ID_APP_TO_RRC_BASE + 0x09),   /* _H2ASN_MsgChoice APP_RRC_CELL_INFO_QUERY_REQ_STRU */
    ID_APP_RRC_CSEL_INFO_QUERY_REQ           = (PS_MSG_ID_APP_TO_RRC_BASE + 0x0a),   /* _H2ASN_MsgChoice APP_RRC_CELL_INFO_QUERY_REQ_STRU */
    ID_APP_RRC_UE_CAP_INFO_QUERY_REQ         = (PS_MSG_ID_APP_TO_RRC_BASE + 0x0b),   /* _H2ASN_MsgChoice APP_RRC_CELL_INFO_QUERY_REQ_STRU */
    ID_APP_RRC_AC_BARRING_INFO_QUERY_REQ     = (PS_MSG_ID_APP_TO_RRC_BASE + 0x0c),   /* _H2ASN_MsgChoice APP_RRC_CELL_INFO_QUERY_REQ_STRU */
    ID_APP_RRC_DRX_INFO_QUERY_REQ            = (PS_MSG_ID_APP_TO_RRC_BASE + 0x0d),   /* _H2ASN_MsgChoice APP_RRC_CELL_INFO_QUERY_REQ_STRU */


    ID_APP_RRC_NMR_REQ                  = (PS_MSG_ID_APP_TO_RRC_BASE + 0x0e),   /* _H2ASN_MsgChoice APP_RRC_NMR_REQ_STRU */
    /* MSP_L4_L4A->ERRC */
    /* _MSGPARSE_Interception APP_RRC_CellidReq I0->IO, I1->I1 */
    ID_APP_RRC_CELLID_REQ               = (PS_MSG_ID_APP_TO_RRC_BASE + 0x0f),

    /* _MSGPARSE_Interception APP_LRRC_ReselOffsetCfgNtf */
    ID_APP_LRRC_RESEL_OFFSET_CFG_NTF         = (PS_MSG_ID_APP_TO_RRC_BASE + 0x10),
    /* _MSGPARSE_Interception APP_LRRC_ConToIdleNtf */
    ID_APP_LRRC_CON_TO_IDLE_NTF          = (PS_MSG_ID_APP_TO_RRC_BASE + 0x11),
    /* _MSGPARSE_Interception APP_LRRC_FastDormCfgNtf */
    ID_APP_LRRC_FAST_DORM_CFG_NTF          = (PS_MSG_ID_APP_TO_RRC_BASE + 0x12),
    /* _MSGPARSE_Interception APP_LRRC_GetNcellInfoReq */
    ID_APP_LRRC_GET_NCELL_INFO_REQ      = (PS_MSG_ID_APP_TO_RRC_BASE + 0x13),
    /* ERR->ERR */
    ID_APP_LRRC_INQ_TCFG_TXPOWER_REQ    = (PS_MSG_ID_APP_TO_RRC_BASE + 0x14),   /* _H2ASN_MsgChoice APP_RRC_INQ_TCFG_TXPOWER_REQ_STRU */


    ID_APP_RRC_FORCE_HOANDCSEL_REQ        = (PS_MSG_ID_APP_TO_RRC_BASE + 0x15),/*_H2ASN_MsgChoice APP_RRC_FORCE_HOANDCSEL_REQ_STRU*/
    ID_APP_RRC_BARCELL_ACCESS_REQ        = (PS_MSG_ID_APP_TO_RRC_BASE + 0x16),/*_H2ASN_MsgChoice APP_RRC_BARCELL_ACCESS_REQ_STRU*/
    /* MSP_L4_L4A->ERRC*/
    /* _MSGPARSE_Interception APP_LRRC_SetUeRelVersionReq I0->IO, I1->I1 */
    ID_APP_LRRC_SET_UE_REL_VERSION_REQ      = (PS_MSG_ID_APP_TO_RRC_BASE + 0x17),
    /* ERR->ERR */
    /* begin:add for 路测融合 */
    ID_DT_LRRC_MEAS_REPORT_REQ         = (PS_MSG_ID_APP_TO_RRC_BASE + 0x18),
    ID_DT_LRRC_SYNC_REPORT_REQ         = (PS_MSG_ID_APP_TO_RRC_BASE + 0x19),
    /* end:add for 路测融合 */


    ID_APP_LRRC_GET_UE_CAP_INFO_REQ    = (PS_MSG_ID_APP_TO_RRC_BASE + 0x1a),   /* _H2ASN_MsgChoice APP_LRRC_GET_UE_CAP_INFO_REQ_STRU */
    /* MSP_L4_L4A->ERRC */
    /* _MSGPARSE_Interception APP_RRC_SCELL_INFO_REQ_STRU I0->IO, I1->I1 */
    ID_APP_RRC_SCELL_INFO_REQ    = (PS_MSG_ID_APP_TO_RRC_BASE + 0x1b),


    /* ERR->ERR */
    ID_APP_RRC_QUERY_ECELL_INFO_REQ    = (PS_MSG_ID_APP_TO_RRC_BASE + 0x1c),  /* _H2ASN_MsgChoice APP_RRC_QUERY_ECELL_INFO_REQ_STRU */
    ID_APP_RRC_QUERY_SCELL_INFO_REQ   = (PS_MSG_ID_APP_TO_RRC_BASE + 0x1d),   /* _H2ASN_MsgChoice APP_RRC_QUERY_SCELL_INFO_REQ_STRU */


    ID_APP_LRRC_SET_TLPS_PRINT2LAYER_REQ    = (PS_MSG_ID_RRC_TO_APP_BASE + 0x18),

    /* APP发给RRC的原语 */
    ID_RRC_APP_TRANSPARENT_CMD_CNF      = (PS_MSG_ID_RRC_TO_APP_BASE + 0x00),   /* _H2ASN_MsgChoice RRC_APP_TRANSPARENT_CMD_CNF_STRU */
    ID_RRC_APP_TRANSPARENT_CMD_IND      = (PS_MSG_ID_RRC_TO_APP_BASE + 0x01),   /* _H2ASN_MsgChoice NULL */
    ID_RRC_APP_CELL_MEAS_RPT_CNF        = (PS_MSG_ID_RRC_TO_APP_BASE + 0x02),   /* _H2ASN_MsgChoice RRC_APP_CellMeasRptCnf */
    ID_RRC_APP_CELL_MEAS_RPT_IND        = (PS_MSG_ID_RRC_TO_APP_BASE + 0x03),   /* _H2ASN_MsgChoice RRC_APP_CellMeasReportInd */
    ID_RRC_APP_TIME_DELAY_RPT_CNF       = (PS_MSG_ID_RRC_TO_APP_BASE + 0x04),   /* _H2ASN_MsgChoice RRC_APP_TimeDelayRptCnf */
    ID_RRC_APP_TIME_DELAY_RPT_IND       = (PS_MSG_ID_RRC_TO_APP_BASE + 0x05),   /* _H2ASN_MsgChoice RRC_APP_TIME_DELAY_RPT_IND_STRU */
    ID_RRC_APP_INQ_CAMP_CELL_INFO_CNF   = (PS_MSG_ID_RRC_TO_APP_BASE + 0x06),   /* _H2ASN_MsgChoice RRC_APP_INQ_CAMP_CELL_INFO_CNF_STRU */
    ID_RRC_APP_INQ_CAMP_CELL_INFO_IND   = (PS_MSG_ID_RRC_TO_APP_BASE + 0x07),   /* _H2ASN_MsgChoice RRC_APP_INQ_CAMP_CELL_INFO_IND_STRU */
    ID_RRC_APP_LOCK_WORK_INFO_CNF       = (PS_MSG_ID_RRC_TO_APP_BASE + 0x08),   /* _H2ASN_MsgChoice RRC_APP_LOCK_WORK_INFO_CNF_STRU */
    ID_RRC_APP_RPT_DBG_INFO_IND         = (PS_MSG_ID_RRC_TO_APP_BASE + 0x09),   /* _H2ASN_MsgChoice RRC_APP_DEBUG_INFO_STRU */
    ID_RRC_APP_CSQ_CNF                  = (PS_MSG_ID_RRC_TO_APP_BASE + 0x0a),   /* _H2ASN_MsgChoice RRC_APP_CSQ_CNF_STRU */
    ID_RRC_APP_CSQ_IND                  = (PS_MSG_ID_RRC_TO_APP_BASE + 0x0b),   /* _H2ASN_MsgChoice RRC_APP_CSQ_IND_STRU */
    ID_RRC_APP_PTL_STATE_QUERY_CNF      = (PS_MSG_ID_RRC_TO_APP_BASE + 0x0c),     /* _H2ASN_MsgChoice RRC_APP_PTL_STATE_QUERY_CNF_STRU */
    ID_RRC_APP_CELL_INFO_QUERY_CNF      = (PS_MSG_ID_RRC_TO_APP_BASE + 0x0e),   /* _H2ASN_MsgChoice RRC_APP_CELL_INFO_QUERY_CNF_STRU */
    ID_RRC_APP_LWCLASH_CNF              = (PS_MSG_ID_RRC_TO_APP_BASE + 0x0f),   /* _H2ASN_MsgChoice RRC_APP_PTL_LWCLASH_IND_STRU */
    ID_RRC_APP_LWCLASH_IND              = (PS_MSG_ID_RRC_TO_APP_BASE + 0x10),   /* _H2ASN_MsgChoice RRC_APP_PTL_LWCLASH_CNF_STRU */
    ID_RRC_APP_SERVING_CELL_INFO_QUERY_CNF    = (PS_MSG_ID_RRC_TO_APP_BASE + 0x11),   /* _H2ASN_MsgChoice RRC_APP_CELL_INFO_QUERY_CNF_STRU */
    ID_RRC_APP_SERVING_CELL_INFO_IND    = (PS_MSG_ID_RRC_TO_APP_BASE + 0x12),   /* _H2ASN_MsgChoice RRC_APP_CELL_INFO_QUERY_CNF_STRU */
    ID_RRC_APP_CSEL_INFO_QUERY_CNF      = (PS_MSG_ID_RRC_TO_APP_BASE + 0x13),   /* _H2ASN_MsgChoice RRC_APP_CELL_INFO_QUERY_CNF_STRU */

    ID_RRC_APP_UE_CAP_INFO_QUERY_CNF      = (PS_MSG_ID_RRC_TO_APP_BASE + 0x15),   /* _H2ASN_MsgChoice RRC_APP_CELL_INFO_QUERY_CNF_STRU */
    ID_RRC_APP_AC_BARRING_INFO_QUERY_CNF  = (PS_MSG_ID_RRC_TO_APP_BASE + 0x17),   /* _H2ASN_MsgChoice RRC_APP_CELL_INFO_QUERY_CNF_STRU */

    ID_RRC_APP_DRX_INFO_QUERY_CNF         = (PS_MSG_ID_RRC_TO_APP_BASE + 0x1b),   /* _H2ASN_MsgChoice RRC_APP_CELL_INFO_QUERY_CNF_STRU */

    ID_RRC_APP_NMR_CNF                    = (PS_MSG_ID_RRC_TO_APP_BASE + 0x1d),   /* _H2ASN_MsgChoice RRC_APP_NMR_CNF_STRU */
    ID_RRC_APP_CELLID_CNF                 = (PS_MSG_ID_RRC_TO_APP_BASE + 0x1e),   /* _H2ASN_MsgChoice RRC_APP_CELLID_CNF_STRU */

    ID_RRC_APP_GET_NCELL_INFO_CNF       = (PS_MSG_ID_RRC_TO_APP_BASE + 0x1f), /*_H2ASN_MsgChoice LRRC_APP_GetNcellInfoCnf*/
    ID_RRC_APP_MEASRPT_EVENTA1_TRIG             = (PS_MSG_ID_RRC_TO_APP_BASE + 0x25),               /* _H2ASN_MsgChoice LRRC_LCMM_MeasRptTrig */
    ID_RRC_APP_MEASRPT_EVENTA2_TRIG             = (PS_MSG_ID_RRC_TO_APP_BASE + 0x26),               /* _H2ASN_MsgChoice LRRC_LCMM_MeasRptTrig */
    ID_RRC_APP_MEASRPT_EVENTA3_TRIG             = (PS_MSG_ID_RRC_TO_APP_BASE + 0x27),               /* _H2ASN_MsgChoice LRRC_LCMM_MeasRptTrig */
    ID_RRC_APP_MEASRPT_EVENTA4_TRIG             = (PS_MSG_ID_RRC_TO_APP_BASE + 0x28),               /* _H2ASN_MsgChoice LRRC_LCMM_MeasRptTrig */
    ID_RRC_APP_MEASRPT_EVENTA5_TRIG             = (PS_MSG_ID_RRC_TO_APP_BASE + 0x29),               /* _H2ASN_MsgChoice LRRC_LCMM_MeasRptTrig */
    ID_RRC_APP_MEASRPT_EVENTA6_TRIG             = (PS_MSG_ID_RRC_TO_APP_BASE + 0x2a),               /* _H2ASN_MsgChoice LRRC_LCMM_MeasRptTrig */
    ID_RRC_APP_MEASRPT_EVENTB1_TRIG             = (PS_MSG_ID_RRC_TO_APP_BASE + 0x2b),               /* _H2ASN_MsgChoice LRRC_LCMM_MeasRptTrig */
    ID_RRC_APP_MEASRPT_EVENTB2_TRIG             = (PS_MSG_ID_RRC_TO_APP_BASE + 0x2c),               /* _H2ASN_MsgChoice LRRC_LCMM_MeasRptTrig */
    ID_RRC_APP_MEASRPT_EVENTC1_TRIG             = (PS_MSG_ID_RRC_TO_APP_BASE + 0x2d),               /* _H2ASN_MsgChoice LRRC_LCMM_MeasRptTrig */
    ID_RRC_APP_MEASRPT_EVENTC2_TRIG             = (PS_MSG_ID_RRC_TO_APP_BASE + 0x2e),               /* _H2ASN_MsgChoice LRRC_LCMM_MeasRptTrig */
    ID_RRC_APP_MEASRPT_EVENTB1_NR_TRIG          = (PS_MSG_ID_RRC_TO_APP_BASE + 0x2f),               /* _H2ASN_MsgChoice LRRC_LCMM_MeasRptTrig */
    ID_RRC_APP_MEASRPT_EVENTB2_NR_TRIG          = (PS_MSG_ID_RRC_TO_APP_BASE + 0x30),               /* _H2ASN_MsgChoice LRRC_LCMM_MeasRptTrig */
    ID_RRC_APP_MEASRPT_PERIOD_TRIG              = (PS_MSG_ID_RRC_TO_APP_BASE + 0x31),               /* _H2ASN_MsgChoice LRRC_LCMM_MeasRptTrig */
    ID_RRC_APP_EXCHANGE_MEASID_LINK_MEASOBJ     = (PS_MSG_ID_RRC_TO_APP_BASE + 0x32), /* _H2ASN_MsgChoice LRRC_LCMM_ExchangeMeasIdLinkObj */
    ID_RRC_APP_RESEL_MEASEVALUATE               = (PS_MSG_ID_RRC_TO_APP_BASE + 0x33), /* _H2ASN_MsgChoice LRRC_CSEL_ReselMeasEvaluate */

    ID_LRRC_APP_SET_UE_REL_VERSION_CNF      = (PS_MSG_ID_RRC_TO_APP_BASE + 0x40),/* _H2ASN_MsgChoice LRRC_APP_SET_UE_REL_VERSION_CNF_STRU  */

    ID_APP_RRC_FORCE_HOANDCSEL_CNF        = (PS_MSG_ID_RRC_TO_APP_BASE + 0x41),/* _H2ASN_MsgChoice RRC_APP_FORCE_HOANDCSEL_CNF_STRU */
    ID_APP_RRC_BARCELL_ACCESS_CNF        = (PS_MSG_ID_RRC_TO_APP_BASE + 0x42),/* _H2ASN_MsgChoice RRC_APP_BARCELL_ACCESS_CNF_STRU */

    ID_LRRC_APP_SET_TLPS_PRINT2LAYER_CNF = (PS_MSG_ID_RRC_TO_APP_BASE + 0x47),


    ID_LRRC_DT_MEAS_REPORT_CNF   = (PS_MSG_ID_RRC_TO_APP_BASE + 0x48),
    ID_LRRC_DT_SYNC_REPORT_CNF   = (PS_MSG_ID_RRC_TO_APP_BASE + 0x49),


    ID_LRRC_APP_GET_UE_CAP_INFO_CNF       = (PS_MSG_ID_RRC_TO_APP_BASE + 0x4a), /* _H2ASN_MsgChoice LRRC_APP_GET_UE_CAP_INFO_CNF_STRU */
    ID_LRRC_APP_FGI_INFO_IND              = (PS_MSG_ID_RRC_TO_APP_BASE + 0x4b), /* _H2ASN_MsgChoice LRRC_APP_FGI_INFO_IND_STRU */


    ID_RRC_APP_DBG_SYSINFO_IND           = (PS_MSG_ID_RRC_TO_APP_BASE + 0x4c),  /* _H2ASN_MsgChoice  LRRC_APP_DBG_SYSINFO_IND_STRU */



    ID_APP_RRC_DEBUG_STUB_SET_3HOUR_REQ       = (PS_MSG_ID_APP_TO_RRC_BASE + 0x20),   /* _H2ASN_MsgChoice APP_RRC_DEBUG_STUB_SET_3HOUR_REQ_STRU */
    ID_APP_RRC_DEBUG_STUB_HO_FAIL_REQ         = (PS_MSG_ID_APP_TO_RRC_BASE + 0x21),   /* _H2ASN_MsgChoice APP_RRC_DEBUG_STUB_HO_FAIL_REQ_STRU */
    ID_APP_RRC_DEBUG_STUB_REL_ALL_REQ         = (PS_MSG_ID_APP_TO_RRC_BASE + 0x22),   /* _H2ASN_MsgChoice APP_RRC_DEBUG_STUB_REL_ALL_REQ_STRU */
    ID_APP_RRC_DEBUG_STUB_SET_EVAL_MESS_REQ   = (PS_MSG_ID_APP_TO_RRC_BASE + 0x25),   /* _H2ASN_MsgChoice APP_RRC_DEBUG_STUB_Set_Evaluation_Message_Req */
    ID_APP_RRC_DEBUG_STUB_L2W_RESEL_LIMIT_REQ   = (PS_MSG_ID_APP_TO_RRC_BASE + 0x26),
    ID_RRC_APP_DEBUG_STUB_CMD_CNF             = (PS_MSG_ID_RRC_TO_APP_BASE + 0x4d),   /* _H2ASN_MsgChoice RRC_APP_DEBUG_STUB_CMD_CNF_STRU */



    ID_RRC_APP_SCELL_INFO_CNF              = (PS_MSG_ID_RRC_TO_APP_BASE + 0x4e), /* _H2ASN_MsgChoice RRC_APP_SCELL_INFO_CNF_STRU */


    ID_RRC_CLOUD_CA_INFO_IND               = (PS_MSG_ID_RRC_TO_APP_BASE + 0x4f),


    ID_RRC_APP_QUERY_ECELL_INFO_CNF         = (PS_MSG_ID_RRC_TO_APP_BASE + 0x51), /* _H2ASN_MsgChoice RRC_APP_QUERY_ECELL_INFO_CNF_STRU */
    ID_RRC_APP_QUERY_SCELL_INFO_CNF         = (PS_MSG_ID_RRC_TO_APP_BASE + 0x52), /* _H2ASN_MsgChoice RRC_APP_QUERY_SCELL_INFO_CNF_STRU */



    ID_RRC_PLMN_FEATURE_INFO_IND               = (PS_MSG_ID_RRC_TO_APP_BASE + 0x53),



    ID_APP_RRC_DEBUG_STUB_ADD_BAD_CELL_REQ     = (PS_MSG_ID_APP_TO_RRC_BASE + 0x23),
/* begin: add for B5000 路测 */
#if (FEATURE_ON == FEATURE_LTE_DT)
    ID_LRRC_LPHY_DT_MEAS_IN_CONNECTED_IND           = (0x0988),
    ID_LRRC_LPHY_DT_INACTIVE_MEAS_IN_CONNECTED_IND  = (0x09b5),
    ID_LRRC_LPHY_DT_MEAS_IN_IDLE_IND                = (0x0986),
    ID_LRRC_LPHY_DT_INACTIVE_MEAS_IN_IDLE_IND       = (0x09b4),
    ID_LRRC_LPHY_DT_MEAS_INFO_IND                   = (0x09a6),
    ID_LRRC_LPHY_DT_RL_SETUP_REQ                    = (0x08c2),
    ID_RRC_APP_LTE_CSEL_INFO_IND                    = (0x0d14),
    ID_RRC_APP_UTRA_CSEL_INFO_IND                   = (0x0d19),
    ID_RRC_APP_GERAN_CSEL_INFO_IND                  = (0x0d1a),
    ID_RRC_APP_UE_CAP_INFO_IND                      = (0x0d16),
    ID_RRC_APP_AC_BARRING_INFO_IND                  = (0x0d18),
    ID_RRC_APP_PTL_STATE_CHANGE_IND                 = (0x0d0d),
    ID_RRC_APP_DRX_INFO_IND                         = (0x0d1c),
    ID_LRRC_DT_SYNC_INFO_IND                        = (0x0904),
    ID_LRRC_DT_OUT_OF_SYNC_INFO_IND                 = (0x090f),
    ID_RRC_APP_SERVING_CELL_CA_INFO_IND             = (0x8D12),
#else
    ID_LRRC_DT_MEAS_INFO_IND                        = (0x988),/* 根据对外接口，路测测量Ind消息id为0x988，等于id_lrrc_lphy_conn_meas_ind */
    ID_LRRC_DT_SYNC_INFO_IND                        = (0x904),/* 根据对外接口，路测测量Ind消息id为0x904，等于id_lrrc_lphy_sync_info_ind */
    ID_LRRC_DT_OUT_OF_SYNC_INFO_IND                 = (0x90f),/* 根据对外接口，路测测量Ind消息id为0x90f，等于id_lrrc_lphy_out_of_sync_info_ind */
    ID_RRC_APP_SERVING_CELL_CA_INFO_IND             = (PS_MSG_ID_RRC_TO_DT_BASE + 0x12),
    ID_RRC_APP_LTE_CSEL_INFO_IND                    = (PS_MSG_ID_RRC_TO_APP_BASE + 0x14),   /* _H2ASN_MsgChoice RRC_APP_CELL_INFO_QUERY_CNF_STRU */
    ID_RRC_APP_UTRA_CSEL_INFO_IND                   = (PS_MSG_ID_RRC_TO_APP_BASE + 0x19),   /* _H2ASN_MsgChoice RRC_APP_CELL_INFO_QUERY_CNF_STRU */
    ID_RRC_APP_GERAN_CSEL_INFO_IND                  = (PS_MSG_ID_RRC_TO_APP_BASE + 0x1a),   /* _H2ASN_MsgChoice RRC_APP_CELL_INFO_QUERY_CNF_STRU */
    ID_RRC_APP_UE_CAP_INFO_IND                      = (PS_MSG_ID_RRC_TO_APP_BASE + 0x16),   /* _H2ASN_MsgChoice RRC_APP_CELL_INFO_QUERY_CNF_STRU */
    ID_RRC_APP_AC_BARRING_INFO_IND                  = (PS_MSG_ID_RRC_TO_APP_BASE + 0x18),   /* _H2ASN_MsgChoice RRC_APP_CELL_INFO_QUERY_CNF_STRU */
    ID_RRC_APP_PTL_STATE_CHANGE_IND                 = (PS_MSG_ID_RRC_TO_APP_BASE + 0x0d),   /* _H2ASN_MsgChoice RRC_APP_PTL_STATE_CHANGE_IND_STRU */
    ID_RRC_APP_DRX_INFO_IND                         = (PS_MSG_ID_RRC_TO_APP_BASE + 0x1c),   /* _H2ASN_MsgChoice RRC_APP_CELL_INFO_QUERY_CNF_STRU */
#endif
/* end: add for B5000 路测 */
    /* begin: add for B5000 路测 */
    ID_APP_RRC_RL_SETUP_REQ                    = (PS_MSG_ID_APP_TO_RRC_BASE + 0x24),
    ID_RRC_APP_RL_SETUP_CNF                    = (PS_MSG_ID_RRC_TO_APP_BASE + 0x54),
    /* end: add for B5000 路测 */
    ID_APP_RRC_DEBUG_STUB_SET_LOSTCOVERAGE_REQ = (PS_MSG_ID_APP_TO_RRC_BASE + 0x27),        /* 0x27为偏移 */
    ID_APP_RRC_MSG_ID_BUTT
};
typedef VOS_UINT32    APP_RRC_MsgIdUint32;

/*****************************************************************************
  4 Enum
*****************************************************************************/
/*****************************************************************************
 结构名    : DT_Cmd
 结构说明  : DT命令枚举
*****************************************************************************/
enum DT_Cmd {
    DT_CMD_STOP,        /*停止*/
    DT_CMD_START,       /*启动*/
    DT_CMD_BUTT
};
typedef VOS_UINT8 DT_CmdUint8;

/*****************************************************************************
 结构名    : DT_Result
 结构说明  : DT操作结果枚举
*****************************************************************************/
enum DT_Result {
    DT_RESULT_SUCC = 0,
    DT_RESULT_FAIL,
    DT_RESULT_NOT_SUPPORT_MEASURE,                /*当前不支持的测量*/

    /**********各自模块分别在下面增加需要的结果值********/
    /*RRC模式的结果值定义Begin*/
    DT_RESULT_RRC_BEGIN = 0x1000,

    /*RRC模式的结果值定义End*/

    /*NAS模式的结果值定义Begin*/
    DT_RESULT_NAS_BEGIN                 = 0x2000,
    DT_RESULT_NAS_PLMN_LOCK             = 0x2001,
    DT_RESULT_NAS_PLMN_UNLOCK           = 0x2002,

    /*NAS模式的结果值定义End*/

    /*L2模式的结果值定义Begin*/
   DT_RESULT_L2_BEGIN = 0x3000,

    /*L2模式的结果值定义End*/

   /*RRU模式的结果值定义Begin*/
   DT_RESULT_RRU_BEGIN = 0x4000,

   /*RRU模式的结果值定义End*/

    DT_RESULT_BUTT
};
typedef VOS_UINT32 DT_ResultUint32;

typedef DT_CmdUint8       APP_CmdUint8;
typedef DT_ResultUint32   APP_ResultUint32;
/*****************************************************************************
 枚举名    : APP_RRC_CellRptPeriod
 枚举说明  :
*****************************************************************************/
enum APP_RRC_CellRptPeriod {
    APP_RRC_CELL_REPORT_PERIOD_300MS = 0,
    APP_RRC_CELL_REPORT_PERIOD_600MS,
    APP_RRC_CELL_REPORT_PERIOD_900MS,
    APP_RRC_CELL_REPORT_PERIOD_1200MS,
    APP_RRC_CELL_REPORT_PERIOD_BUTT
};
typedef VOS_UINT8 APP_RRC_CellRptPeriodUint8;

/*****************************************************************************
 枚举名    : APP_LatencyType
 枚举说明  :
*****************************************************************************/
enum APP_LatencyType {
    APP_LATENCY_EST =0,               /*连接建立时延*/
    APP_LATENCY_HO,                   /*切换时延*/
    APP_LATENCY_REEST,                /*重建时延*/
    APP_LATENCY_BUTT
};
typedef VOS_UINT8 APP_LatencyTypeUint8;

/*****************************************************************************
 枚举名    : APP_RRC_LockWorkInfoType
 枚举说明  : 锁定的类型
*****************************************************************************/
enum APP_RRC_LockWorkInfoType {
    RRC_APP_LOCK_FREQPOINT,              /* 频点锁定 */
    RRC_APP_LOCK_FREQANDCELL,            /* 频点和小区联合锁定 */
    RRC_APP_LOCK_FREQBAND,               /* 频带锁定 */
    RRC_APP_LOCK_BUTT
};
typedef VOS_UINT8 APP_RRC_LockWorkInfoTypeUing8;
/*****************************************************************************
 枚举名    : APP_DelayType
 枚举说明  :
*****************************************************************************/
enum APP_DelayType {
    APP_DELAY_CONTROL_PLANE =0,     /*控制面时延*/
    APP_DELAY_USER_PLANE,           /*用户面时延*/
    APP_DELAY_HO,                   /*切换时延*/
    APP_DELAY_BUTT
};
typedef VOS_UINT8 APP_DelayTypeUint8;
/*****************************************************************************
 枚举名    : APP_CAMPED_FLAG_ENUM
 枚举说明  :
*****************************************************************************/
enum APP_CampedFlag {
    APP_CAMPED,                 /* 已驻留 */
    APP_NOT_CAMPED,             /* 未驻留 */
    APP_CAMPED_BUTT
};
typedef VOS_UINT8 APP_CampedFlagUint8;
/*****************************************************************************
 枚举名    : APP_DelayType
 枚举说明  :
*****************************************************************************/
enum APP_StateFlag {
    APP_STATE_NOT_IN_RANGE,                 /* 不在冲突范围内,状态2 */
    APP_STATE_IN_RANGE,                     /* 在冲突范围内,状态1*/
    APP_STATE_BUTT
};
typedef VOS_UINT8 APP_StateFlagUint8;


/*****************************************************************************
 枚举名    : LRRC_CaCcNumInfo
 枚举说明  :
*****************************************************************************/
enum LRRC_CaCcNumInfo {
    LRRC_CA_CC_NUM_2,                             /* 代表最大支持CA CC数是2CC */
    LRRC_CA_CC_NUM_3,                             /* 代表最大支持CA CC数是3CC */
    LRRC_CA_CC_NUM_4,                             /* 代表最大支持CA CC数是4CC */
    LRRC_CA_CC_NUM_BUTT
};
typedef VOS_UINT8 LRRC_CaCcNumUint8;


/*****************************************************************************
   5 STRUCT
*****************************************************************************/
/*****************************************************************************
 结构名    : APP_OM_Msg
 结构说明  : APP(后台工具)与OM交互的消息体
*****************************************************************************/
typedef struct {
     VOS_MSG_HEADER                     /*VOS头
 */
     VOS_UINT32          msgId;
     APP_MSG_HEADER                     /*APP头
 */
     VOS_UINT8           para[4];    /* 参数内容 */
}APP_OM_Msg;

/*****************************************************************************
 结构名    : APP_PLMN_ID_STRU
 结构说明  :
    MCC, Mobile country code (aucPlmnId[0], aucPlmnId[1] bits 1 to 4)
    MNC, Mobile network code (aucPlmnId[2], aucPlmnId[1] bits 5 to 8).

    The coding of this field is the responsibility of each administration but BCD
    coding shall be used. The MNC shall consist of 2 or 3 digits. For PCS 1900 for NA,
    Federal regulation mandates that a 3-digit MNC shall be used. However a network
    operator may decide to use only two digits in the MNC over the radio interface.
    In this case, bits 5 to 8 of octet 4 shall be coded as "1111". Mobile equipment
    shall accept MNC coded in such a way.

    ---------------------------------------------------------------------------
                 ||(BIT8)|(BIT7)|(BIT6)|(BIT5)|(BIT4)|(BIT3)|(BIT2)|(BIT1)
    ---------------------------------------------------------------------------
    aucPlmnId[0] ||    MCC digit 2            |           MCC digit 1
    ---------------------------------------------------------------------------
    aucPlmnId[1] ||    MNC digit 3            |           MCC digit 3
    ---------------------------------------------------------------------------
    aucPlmnId[2] ||    MNC digit 2            |           MNC digit 1
    ---------------------------------------------------------------------------

    AT命令：
    at+cops=1,2,"mcc digit 1, mcc digit 2, mcc digit 3, mnc digit 1, mnc digit 2, mnc

digit 3",2 :

    e.g.
    at+cops=1,2,"789456",2 :
    --------------------------------------------------------------------------------
    (mcc digit 1)|(mcc digit 2)|(mcc digit 3)|(mnc digit 1)|(mnc digit 2)|(mnc digit 3)
    --------------------------------------------------------------------------------
       7         |     8       |      9      |     4       |      5      |     6
    --------------------------------------------------------------------------------

    在aucPlmnId[3]中的存放格式:
    ---------------------------------------------------------------------------
                 ||(BIT8)|(BIT7)|(BIT6)|(BIT5)|(BIT4)|(BIT3)|(BIT2)|(BIT1)
    ---------------------------------------------------------------------------
    aucPlmnId[0] ||    MCC digit 2 = 8        |           MCC digit 1 = 7
    ---------------------------------------------------------------------------
    aucPlmnId[1] ||    MNC digit 3 = 6        |           MCC digit 3 = 9
    ---------------------------------------------------------------------------
    aucPlmnId[2] ||    MNC digit 2 = 5        |           MNC digit 1 = 4
    ---------------------------------------------------------------------------

*****************************************************************************/
typedef struct {
    VOS_UINT8                           plmnId[3];
    VOS_UINT8                           rsv;
} APP_PlmnId;

/*****************************************************************************
 结构名    : APP_RRC_CellMeasRptReq
 结构说明  :
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgID;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    APP_CmdUint8                        command;
    APP_RRC_CellRptPeriodUint8          period;
    VOS_UINT8                           reserved[2]; /* 保留字段，长度为2,便于4字节对齐 */
}APP_RRC_CellMeasRptReq;

/*****************************************************************************
 结构名    : RRC_APP_CellMeasRptCnf
 结构说明  :
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    APP_ResultUint32              result;
}RRC_APP_CellMeasRptCnf;

/*****************************************************************************
 结构名    :RRC_APP_CellMeasRslt
 结构说明  :小区测量结果
*****************************************************************************/
typedef struct {
    VOS_UINT16 cellId;
    VOS_INT16  rsrp;
    VOS_INT16  rsrq;
    VOS_INT16  rssi;
}RRC_APP_CellMeasRslt;
/*****************************************************************************
 结构名    :LRRC_APP_DebugStubLostCoverage
 协议表格  :
 ASN.1描述 :
 结构说明  :LRRC_APP_DebugStubLostCoverage全局结构
*****************************************************************************/
typedef struct {
    VOS_UINT8   lostCoverageSwitch;    /* 打桩丢网定时器开关, 0表示关闭，1表示打开 */
    VOS_UINT8   rsv;                   /* 保留位1 */
    VOS_UINT16  timeLen;               /* 丢网定时器长度, 只在开关打开时生效*/
} LRRC_APP_DebugStubLostCoverage;

/*****************************************************************************
 结构名    :RRC_APP_CellMeasReportInd
 结构说明  :RRC通过此原语周期上报小区的能量测量结果。
            当前存在同频测量时，RRC上报小区的测量结果；不存在，不上报数据；
            如果当前不存在同频测量，但是网络侧之后发送了同频测量，RRC根据之前的OM设置进行上报小区测量结果。
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    VOS_UINT8                           cellCnt;                              /* 小区数量 */
    VOS_UINT8                           reserved[3]; /* 保留字段，长度为3,便于4字节对齐 */
    RRC_APP_CellMeasRslt         cellMeasInd[RRC_APP_CELL_MAX_COUNT];
}RRC_APP_CellMeasReportInd;

/*****************************************************************************
 结构名    : APP_RRC_TimeDelayRptReq
 结构说明  : 要求上报控制面时延。控制面时延：信令RB建立的开销时间，即发起连接建立请求到RB1建立成功的时间。
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    APP_CmdUint8                  command;
    APP_DelayTypeUint8           delayType;
    VOS_UINT8                           reserved[2]; /* 保留字段，长度为2,便于4字节对齐 */
}APP_RRC_TimeDelayRptReq;

/*****************************************************************************
 结构名    :RRC_APP_TimeDelayRptCnf
 结构说明  :对原语APP_RRC_TIME_DELAY_RPT_REQ_STRU的回复
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    APP_DelayTypeUint8           delayType;
    VOS_UINT8                           reserved[3]; /* 保留字段，长度为3,便于4字节对齐 */
    APP_ResultUint32              result;
}RRC_APP_TimeDelayRptCnf;

/*****************************************************************************
 结构名    :APP_RRC_INQ_CAMP_CELL_INFO_REQ_STRU
 结构说明  :获取当前驻留小区的ID、频点等信息的查询请求，或者停止上报驻留小区信息的请求。
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    APP_CmdUint8                  command;
    VOS_UINT8                           reserved[3]; /* 保留字段，长度为3,便于4字节对齐 */
}APP_RRC_InqCampCellInfoReq;

/*****************************************************************************
 结构名    :RRC_APP_INQ_CAMP_CELL_INFO_CNF_STRU
 结构说明  :对原语APP_RRC_INQ_CAMP_CELL_INFO_REQ_STRU的回复
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    APP_ResultUint32              result;
}RRC_APP_InqCampCellInfoCnf;

/*****************************************************************************
 结构名    :RRC_APP_INQ_CAMP_CELL_INFO_IND_STRU
 结构说明  :上报当前驻留小区信息
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    PS_BOOL_ENUM_UINT8                  valueFlag;
    VOS_UINT8                           reserved[3]; /* 保留字段，长度为3,便于4字节对齐 */
    VOS_UINT16                          cellId;
    VOS_UINT16                          freqInfo;
}RRC_APP_InqCampCellInfoInd;

/*****************************************************************************
 结构名    :APP_RRC_FREQ_BAND_STRU
 结构说明  :频带范围
*****************************************************************************/
typedef struct {
    VOS_UINT16                          lowBand;                              /* 频点下限 */
    VOS_UINT16                          highBand;                             /* 频点上限 */
}APP_RRC_FreqBand;

/*****************************************************************************
 结构名    :APP_RRC_CELL_INFO_STRU
 结构说明  :小区信息
*****************************************************************************/
typedef struct {
    VOS_UINT16                          cellId;                               /* 小区ID */
    VOS_UINT16                          freqPoint;                            /* 频点信息 */
}APP_RRC_CellInfo;

/*****************************************************************************
 结构名    :APP_RRC_LOCK_INFO_STRU
 结构说明  :锁定的相关信息
*****************************************************************************/
typedef struct {
    APP_RRC_LockWorkInfoTypeUing8  lockType;                         /* 锁定类型 */
    VOS_UINT8                               bandInd;
    union {
        VOS_UINT16                      freqPoint;                            /* 锁定的频点 */
        APP_RRC_CellInfo          freqAndCell;                          /* 锁定的小区ID和频点 */
        APP_RRC_FreqBand          freqBand;                             /* 锁定频带的上下限 */
    }u;
}APP_RRC_LockInfo;

/*****************************************************************************
 结构名    :APP_RRC_LOCK_WORK_INFO_REQ_STRU
 结构说明  :路测下发的锁定请求的结构体
*****************************************************************************/
/*lint -save -e959*/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32 msgID;                                     /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    APP_CmdUint8                        command;                              /* 锁定还是解锁 */
    VOS_UINT8                           reserved[3]; /* 保留字段，长度为3,便于4字节对齐 */
    APP_RRC_LockInfo              lockInfo;
}APP_RRC_LockWorkInfoReq;
/*lint -restore*/

/*****************************************************************************
 结构名    :RRC_APP_LOCK_WORK_INFO_CNF_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :对原语APP_RRC_LOCK_WORK_INFO_REQ进行回复
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    APP_ResultUint32                    result;
}RRC_APP_LockWorkInfoCnf;


/*****************************************************************************
 结构名    :APP_LRRC_GET_UE_CAP_INFO_REQ_STRU
 结构说明  :HIDS下发的获取关键UE能力信息的结构体
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgID;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    VOS_UINT8                           reserved[128]; /* 保留字段，长度为128,便于4字节对齐 */
}APP_LRRC_GetUeCapInfoReq;

/*****************************************************************************
 结构名    : LRRC_APP_UE_CAP_INFO_STRU
 结构说明  : LRRC与APP之间上报的UE能力关键信元
*****************************************************************************/
typedef struct {
    /* ulFGI01To32: APP解析的bit位如下: short DRX:FGI4, C-DRX:FGI5, L-W,PSHO:FGI8,
       L-G,PSHO:FGI9, CSFB w/measurements(REL9):FGI22, TTI Bundling:FGI28, SPS:FGI29 */
    VOS_UINT32                          fGI01To32;                            /* fgi bit位从最高位至最低位对应FGI1至FGI32 */
    VOS_UINT32                          fGI33To64;                            /* fgi bit位从最高位至最低位对应FGI33至FGI64 */
    VOS_UINT32                          fGI101To132;                          /* fgi bit位从最高位至最低位对应FGI101至FGI132 */
    VOS_UINT32                          fGIReserved;                          /* fgi bit保留位，保留32bit以便于协议扩展 */

    /* 从最高bit开始计数，为profile0x0001,以此类推，如果值为1，表示支持，0表示
       不支持;分别对应profile0x0001,profile0x0002,profile0x0003,profile0x0004,
       profile0x0006,profile0x0101,profile0x0103,profile0x0104 */
    VOS_UINT16                          rOHC;

    VOS_UINT8                           category;
    LRRC_CaCcNumUint8                   caCCNum;                              /* 最大支持的CA CC数 */


    VOS_UINT32                          bandNum;                                   /* usBandNum 取值范围[1,256] */
    VOS_UINT32                          bandInd[LRRC_APP_MAX_BAND_IND_ARRAY_NUM]; /* aulBandInd,每个BIT，1:代表支持，0代表不支持，
                                                                                        ulBandInd[0]中第一个BIT代表BAND1,以此类推; */
    VOS_UINT32                          qam256[LRRC_APP_MAX_BAND_IND_ARRAY_NUM]; /* aul256Qam,每个BIT，1:代表支持，0代表不支持，
                                                                                        aul256Qam[0]中第一个BIT，以此类推; */
    PS_BOOL_ENUM_UINT8                  ulMimo4Layer;                           /* 是否支持上行4层 */
    PS_BOOL_ENUM_UINT8                  dlMimo4Layer;                           /* 是否支持下行4层 */
    VOS_UINT8                           reserved[2]; /* 保留字段，长度为2,便于4字节对齐 */
}LRRC_APP_UeCapInfo;

/*****************************************************************************
 结构名    :LRRC_APP_GET_UE_CAP_INFO_CNF_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :对原语APP_LRRC_GET_UE_CAP_INFO_REQ_STRU进行回复
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    APP_ResultUint32                    result;
    LRRC_APP_UeCapInfo                  ueCapInfo;
    /* 32字节，用来指示后续是否有Ind指示，目前主要用于扩展CA组合，因为CA组合占用空间很大 */
    VOS_UINT8                           reserved[32];
}LRRC_APP_GetUeCapInfoCnf;


/*****************************************************************************
 结构名    : APP_RRC_TRANSPARENT_CMD_REQ
 协议表格  :
 ASN.1描述 :
 结构说明  :透明命令头
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32 msgId;                                     /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    VOS_UINT16 transCmdNum;
    VOS_UINT16 paraSize;
    VOS_UINT8  para[4];
}APP_RRC_TransparentCmdReq;


/*****************************************************************************
 结构名    : RRC_APP_TRANSPARENT_CMD_CNF_STRU
 结构说明  : PS->OMT的透明命令执行结果数据结构(
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32          msgId;                            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    VOS_UINT8           transCmdCnf[4];                 /*透明命令结果码流，可变*/
}RRC_APP_TransparentCmdCnf;

/*****************************************************************************
 结构名    :PS_APP_HO_LATENCY_DETAIL_STRU
 结构说明  :切换时延关键点的上报
*****************************************************************************/
typedef struct {
    VOS_UINT32                          t4ulRrcRcvHoRecfg;
    VOS_UINT32                          rrcRcvNasRabmRsp;
    VOS_UINT32                          rrcRcvCmmHoCnf;
    VOS_UINT32                          rrcRcvStopPdcpCnf;
    VOS_UINT32                          t5ulRrcSndDspHoReq;
    VOS_UINT32                          t6ulRrcRcvDspHoCnf;
    VOS_UINT32                          t7ulRrcSndMacRaReq;
    VOS_UINT32                          t12ulMacSndPhyAccessReq;
    VOS_UINT32                          t13ulMacRcvRar;
    VOS_UINT32                          t8ulRrcRcvMacRaCnf;
    VOS_UINT32                          rrcSndSmcSecuCfg;
    VOS_UINT32                          t9ulRrcSndRecfgCmp;
    VOS_UINT32                          t10ulRrcRcvAmDataCnf;
    VOS_UINT32                          rrcRcvCqiSrsCnf;
    VOS_UINT32                          rrcRcvPdcpContineCnf;
    VOS_UINT32                          t11ulRrcRcvRabmStatusRsp;
}PS_APP_HoLatencyDetail;
/*****************************************************************************
 结构名    :PS_APP_HO_COMP_DETAIL_STRU
 结构说明  :切换时延与协议对比
*****************************************************************************/
typedef struct {
    VOS_UINT32                          realTestT6T5;
    VOS_UINT32                          ptlT6T5;          /* 1ms */
    VOS_UINT32                          realTestT8T7;
    VOS_UINT32                          ptlT8T7;          /* 2.5 + 1+ 7.5ms */
    VOS_UINT32                          realTestT10T9;
    VOS_UINT32                          ptlT10T9;         /* 13ms */
    VOS_UINT32                          realTestT7T4;
    VOS_UINT32                          ptlT7T4;          /* 50ms */
    VOS_UINT32                          realTestT11T4;
    VOS_UINT32                          ptlT11T4;         /* 100ms */
}PS_APP_HoCompDetail;
/*****************************************************************************
 结构名    :PS_APP_EST_LATENCY_DETAIL_STRU
 结构说明  :建链时延关键点的上报
*****************************************************************************/
typedef struct {
    VOS_UINT32                          t1ulRrcSndMacRaReq;
    VOS_UINT32                          t2ulMacSndPhyAccessReq;
    VOS_UINT32                          t4ulMacRcvRar;
    VOS_UINT32                          rrcRcvMacRaCnf;
    VOS_UINT32                          t5ulMacSndMsg3;
    VOS_UINT32                          t6ulRrcRcvSetUp;
    VOS_UINT32                          t7ulRrcSndEstCmp;
    VOS_UINT32                          rrcRcvSecCmd;
    VOS_UINT32                          rrcSndSecCmdCmp;
    VOS_UINT32                          rrcRcvCapEnq;
    VOS_UINT32                          rrcSndCapInfo;
    VOS_UINT32                          rrcRcvRecfg;
    VOS_UINT32                          t13ulRrcSndRecfgCmp;
}PS_APP_EstLatencyDetail;
/*****************************************************************************
 结构名    :PS_APP_EST_COMP_STRU
 结构说明  :建链时延与协议对比
*****************************************************************************/
typedef struct {
    VOS_UINT32                          realTestT4T1;
    VOS_UINT32                          ptlT4T1;              /* 5s + 6ms */
    VOS_UINT32                          realTestT5T4;
    VOS_UINT32                          ptlT5T4;              /* 2.5ms */
    VOS_UINT32                          testRealT6T5;
    VOS_UINT32                          ptlT6T5;              /* 28.5ms+2*Ts1c */
    VOS_UINT32                          realTestT7T6;
    VOS_UINT32                          ptlT7T6;              /* 3ms */
    VOS_UINT32                          reaTestlT7T1;
    VOS_UINT32                          ptlT7T1;              /* 47.5ms+2*Ts1c */
    VOS_UINT32                          reaTestlT13T1;
    VOS_UINT32                          ptlT13T1;             /* 61ms+2*Ts1c+Ts1u */
}PS_APP_EstCompDetail;
/*****************************************************************************
 结构名    :PS_APP_REEST_LATENCY_DETAIL_STRU
 结构说明  :重建时延关键点的上报
*****************************************************************************/
typedef struct {
    VOS_UINT32                          t2ulRrcRcvReestInd;
    VOS_UINT32                          t3ulRrcSndCellSearchReq;
    VOS_UINT32                          t4ulRrcRcvCellSearchInd;
    VOS_UINT32                          t5ulRrcRcvSi;
    VOS_UINT32                          t6ulRrcSndCampReq;
    VOS_UINT32                          t7ulRrcRcvCampCnf;
    VOS_UINT32                          t10ulRrcSndMacRaReq;
    VOS_UINT32                          macSndPhyAccessReq;
    VOS_UINT32                          macRcvRar;
    VOS_UINT32                          rrcRcvMacRaCnf;
    VOS_UINT32                          macSndMsg3;
    VOS_UINT32                          rrcRcvReest;
    VOS_UINT32                          rrcSndReestCmpl;
}PS_APP_ReestLatencyDetail;

/*****************************************************************************
 结构名    :PS_APP_REEST_COMP_DETAIL_STRU
 结构说明  :重建时延与协议对比
*****************************************************************************/
typedef struct {
    VOS_UINT32                          realTestT4T3;
    VOS_UINT32                          ptlT4T3;              /* 100ms */
    VOS_UINT32                          realTestT5T4;
    VOS_UINT32                          ptlT5T4;              /* 1280ms */
    VOS_UINT32                          realTestT10T2;
    VOS_UINT32                          ptlT10T2;             /* 1500ms */

}PS_APP_ReestCompDetail;
/*****************************************************************************
 结构名    :RRC_APP_TIME_LATENCY_RPT_IND_STRU
 结构说明  :时延时间的上报
*****************************************************************************/
typedef struct {
    VOS_UINT32                                  t7T4ulHoCPlaneRealLatency;
    VOS_UINT32                                  hoCPlanePtlLatency;
    VOS_UINT32                                  t11T4ulHoUPlaneRealLatency;
    VOS_UINT32                                  hoUPlanePtlLatency;
    PS_APP_HoLatencyDetail                      hoLatencyDetail;
    PS_APP_HoCompDetail                         hoCompDetail;
}PS_APP_HoLatency;

/*****************************************************************************
 结构名    :PS_APP_REEST_LATENCY_STRU
 结构说明  :时延时间的上报
*****************************************************************************/
typedef struct {
    VOS_UINT32                                  t10T2ulReestRealLatency;
    VOS_UINT32                                  reestPtlLatency;
    PS_APP_ReestLatencyDetail                   reestLatencyDetail;
    PS_APP_ReestCompDetail                      reestCompDetail;
}PS_APP_ReestLatency;

/*****************************************************************************
 结构名    :PS_APP_EST_LATENCY_STRU
 结构说明  :时延时间的上报
*****************************************************************************/
typedef struct {
    VOS_UINT32                                  t13T1ulEstUPlaneRealLatency;
    VOS_UINT32                                  estUPlanePtlLatency;
    VOS_UINT32                                  t7T1ulEstCPlaneRealLatency;
    VOS_UINT32                                  estCPlanePtlLatency;
    PS_APP_EstLatencyDetail                     estLatencyDetail;
    PS_APP_EstCompDetail                        estCompDetail;
}PS_APP_EstLatency;

/*****************************************************************************
 结构名    :RRC_APP_TIME_LATENCY_RPT_IND_STRU
 结构说明  :时延时间的上报
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;

    APP_LatencyTypeUint8         latencyType;
    VOS_UINT8                           reserved[3]; /* 保留字段，长度为3,便于4字节对齐 */
    union {
        PS_APP_HoLatency             hoElapse;
        PS_APP_ReestLatency          reestElapse;
        PS_APP_EstLatency            estElapse;
    }u;
} RRC_APP_TimeDelayRptInd;

/*****************************************************************************
 结构名    :APP_RRC_CSQ_REQ_STRU
 结构说明  :CSQ查询请求
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    VOS_UINT16                          swt;              /* 0: 停止周期上报 1: 查询RSSI，不启动IND上报  2: 启动周期上报RSSI */
    VOS_UINT16                          period;
    VOS_UINT16                          ndb;              /* 范围 0-5 dbm*/
    VOS_UINT16                          ms;               /* 范围 1-20 s*/
}APP_RRC_CsqReq;

/*****************************************************************************
 结构名    :RRC_APP_CSQ_CNF_STRU
 结构说明  :CSQ查询请求响应
*****************************************************************************/
typedef struct {
    VOS_UINT16                          ri;                /*RI值*/
    VOS_UINT16                          cqi[2];           /* CQI两个码字 */
    VOS_UINT8                           res[2]; /* 保留字段，长度为2,便于4字节对齐 */

}APP_RRC_CqiInfo;

/*****************************************************************************
 结构名    :RRC_APP_CSQ_CNF_STRU
 结构说明  :CSQ查询请求响应
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    APP_ResultUint32              result;
    VOS_INT16                           rsrp;              /* RSRP测量值范围：(-141,-44) */
    VOS_INT16                           rsrq;              /* RSRQ测量值范围：(-40, -6) */
    VOS_INT16                           rssi;              /* RSSI测量值 */
    VOS_UINT16                          ber;              /* 误码率 */
    VOS_INT32                           sinr;              /* SINR  RS_SNR */
    APP_RRC_CqiInfo                   cqi;              /* CQI两个码字 */
}RRC_APP_CsqCnf;

/*****************************************************************************
 结构名    :RRC_APP_CSQ_IND_STRU
 结构说明  :RRC上报给APP的周期CSQ查询指示
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_INT16                           rsrp;              /* RSRP测量值范围：(-141,-44) */
    VOS_INT16                           rsrq;              /* RSRQ测量值范围：(-40, -6) */
    VOS_INT16                           rssi;              /* RSSI测量值 */
    VOS_UINT16                          ber;              /* 误码率 */
    VOS_INT32                           sinr;              /* SINR  RS_SNR */
    APP_RRC_CqiInfo               cqi;              /* CQI两个码字 */
}RRC_APP_CsqInd;

/*****************************************************************************
 结构名    :APP_RRC_PTL_STATE_QUERY_REQ_STRU
 结构说明  :协议状态查询请求
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    APP_CmdUint8                  command;
    APP_RRC_CellRptPeriodUint8  period;
    VOS_UINT8                           reserved[2]; /* 保留字段，长度为2,便于4字节对齐 */

}APP_RRC_PtlStateQueryReq;

/*****************************************************************************
 结构名    : RRC_APP_PTL_STATE_QUERY_CNF_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : RRC上报的协议状态
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;             /* MSP直接将此ID做为CmdID发给Prob */
    APP_ResultUint32              result;
}RRC_APP_PtlStateQueryCnf;


/*****************************************************************************
 结构名    : RRC_APP_PTL_STATE_CHANGE_IND_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : RRC上报的协议状态
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    VOS_UINT32                          currentState;     /* RRC协议状态, 0:表示IDLE态 1:表示CONNECTED 2:表示协议状态无效 */
}RRC_APP_PtlStateChangeInd;

/*****************************************************************************
 结构名    : RRC_APP_PTL_STATE_CHANGE_IND_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : RRC上报的协议状态
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    VOS_UINT32                          currentState;     /* RRC协议状态, 0:表示IDLE态 1:表示CONNECTED 2:表示协议状态无效 */
}RRC_APP_DtPtlStateInd;



/*****************************************************************************
 结构名    :APP_RRC_PTL_STATE_QUERY_REQ_STRU
 结构说明  :小区基本信息查询请求
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
}APP_RRC_CellInfoQueryReq;

/*****************************************************************************
 结构名    : RRC_APP_PTL_STATE_QUERY_CNF_STRU
 结构说明  : RRC回复的小区基本信息
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    VOS_UINT16                          freq;             /* 0xFFFF为无效,单位：100KHz */
    VOS_UINT16                          pci;              /* 0xFFFF为无效,范围：(0,503) */
    VOS_UINT8                           dlBandWidth;      /* 0xff为无效,范围:(0,5): (0 : 1.4M , 1 : 3M , 2 : 5M ,3 : 10M , 4 : 15M, 5 : 20M) */
    VOS_UINT8                           reserved[3]; /* 保留字段，长度为3,便于4字节对齐 */
}RRC_APP_CellInfoQueryCnf;


typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgID;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    APP_CmdUint8                  command;
    APP_RRC_CellRptPeriodUint8  period;
    VOS_UINT8                           reserved[2]; /* 保留字段，长度为2,便于4字节对齐 */
}APP_RRC_InqCmdReq;

typedef APP_RRC_InqCmdReq APP_RRC_InqServingCellInfoReq;

typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgID;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    APP_ResultUint32              result;
}APP_RRC_InqCmdCnfHeader;
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgID;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
}APP_RRC_InqCmdIndHeader;


typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    APP_ResultUint32              result;
}RRC_APP_ForceHoandcselCnf;
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32 msgID;                                     /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    DT_CmdUint8                   cmd;              /* FORCE HO/CSEL FLAG */
    VOS_UINT8                           rsv[3]; /* 保留字段，长度为3,便于4字节对齐 */
    VOS_UINT32                          pci;              /* PHY Cell ID */
}APP_RRC_ForceHoandcselReq;
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    APP_ResultUint32              result;
}RRC_APP_BarcellAccessCnf;
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32 msgID;                                     /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    DT_CmdUint8                   cmd;              /* Bar Cell Access FLAG */
    VOS_UINT8                           rsv[3]; /* 保留字段，长度为3,便于4字节对齐 */
}APP_RRC_BarcellAccessReq;



/*****************************************************************************
 结构名    : APP_RRC_MSG_DATA
 协议表格  :
 ASN.1描述 :
 结构说明  : APP_RRC_MSG_DATA数据结构,以下为RRC专用
*****************************************************************************/
typedef struct {
    APP_RRC_MsgIdUint32          msgID;        /*_H2ASN_MsgChoice_Export APP_RRC_MsgIdUint32*/
    VOS_UINT8                           msg[4];
    /***************************************************************************
        _H2ASN_MsgChoice_When_Comment          APP_RRC_MsgIdUint32
    ****************************************************************************/
}APP_RRC_MsgData;

/*_H2ASN_Length UINT32*/

/*****************************************************************************
 结构名    : App_Rrc_InterfaceMsg
 协议表格  :
 ASN.1描述 :
 结构说明  : AppRrcInterface_MSG数据结构,以下为RRC专用
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER
    APP_RRC_MsgData                    msgData;
}App_Rrc_InterfaceMsg;
/*********************************************************
 枚举名    : LRRC_LPHY_LTE_BAND_WIDTH_ENUM
 协议表格  :
 ASN.1描述 :
 枚举说明  :
**********************************************************/
enum RRC_APP_BandWidth {
    RRC_APP_LTE_BAND_WIDTH_1D4M = 0,
    RRC_APP_LTE_BAND_WIDTH_3M,
    RRC_APP_LTE_BAND_WIDTH_5M,
    RRC_APP_LTE_BAND_WIDTH_10M,
    RRC_APP_LTE_BAND_WIDTH_15M,
    RRC_APP_LTE_BAND_WIDTH_20M,
    RRC_APP_LTE_BAND_WIDTH_BUTT
};
typedef VOS_UINT16 RRC_APP_BandWidthUint16;

/*****************************************************************************
 结构名    : APP_RRC_LWCLASH_REQ_STRU
 结构说明  :协议状态查询请求
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                      /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;        /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
} APP_RRC_LwclashReq;

/*********************************************************
 枚举名    : APP_RRC_ANTENNA_MAX_LAYERS_MIMO_ENUM
 协议表格  :
 ASN.1描述 :
 枚举说明  :
**********************************************************/
enum APP_RRC_AntennaMaxLayersMimo {
    APP_RRC_ANTENNA_TWO_LAYERS      = 0,      /* UE MIMO层数2层 */
    APP_RRC_ANTENNA_FOUR_LAYERS,              /* UE MIMO层数4层 */
    APP_RRC_ANTENNA_EIGHT_LAYERS,             /* UE MIMO层数8层 */
};
typedef VOS_UINT8    APP_RRC_AntennaMaxLayersMimoUint8;

/*****************************************************************************
 结构名    : RRC_APP_LWCLASH_PARA_STRU
结构说明  : RRC上报的消息
*****************************************************************************/
typedef struct {

    VOS_UINT32                              ulFreq;          /*上行中心频率 单位:100Khz*/
    VOS_UINT32                              dlFreq;          /*下行中心频率 单位:100Khz*/

    RRC_APP_BandWidthUint16          ulBandwidth;     /*上行带宽 */
    RRC_APP_BandWidthUint16          dlBandwidth;     /*上行带宽 */
    APP_CampedFlagUint8              camped;          /*是否驻留 */
    APP_StateFlagUint8               state;           /*是否为冲突状态 */
    VOS_UINT8                               band;            /*频带指示 */
    APP_RRC_AntennaMaxLayersMimoUint8   dlMimo;
} RRC_APP_LwclashPara;


/*****************************************************************************
 结构名    : RRC_APP_SCELL_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :
*****************************************************************************/
typedef struct {
    VOS_UINT32                              ulFreq;           /*上行中心频率 单位:100Khz*/
    VOS_UINT32                              dlFreq;           /*下行中心频率 单位:100Khz*/
    RRC_APP_BandWidthUint16          ulBandwidth;      /*上行带宽 */
    RRC_APP_BandWidthUint16          dlBandwidth;      /*下行带宽 */
    APP_RRC_AntennaMaxLayersMimoUint8   dlMimo;
    VOS_UINT8                               resv[3]; /* 保留字段，长度为3,便于4字节对齐 */
}RRC_APP_ScellInfo;



/*****************************************************************************
 结构名    : RRC_APP_LWCLASH_CNF_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                      /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;        /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;         /* MSP直接将此ID做为CmdID发给Prob */
    RRC_APP_LwclashPara           lWClashPara;
    VOS_UINT32                          scellNum;
    RRC_APP_ScellInfo             scellInfo[LRRC_APP_LWCLASH_MAX_SCELL_NUM];
    PS_BOOL_ENUM_UINT8                  dl256QamFlag;
    VOS_UINT8                           resv[3]; /* 保留字段，长度为3,便于4字节对齐 */
} RRC_APP_LwclashCnf;
/*****************************************************************************
 结构名    : RRC_APP_LWCLASH_IND_STRU
结构说明  : RRC上报的消息
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                      /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;        /*_H2ASN_Skip*/
    RRC_APP_LwclashPara           lWClashPara;
    VOS_UINT32                          scellNum;
    RRC_APP_ScellInfo             scellInfo[LRRC_APP_LWCLASH_MAX_SCELL_NUM];
    PS_BOOL_ENUM_UINT8                  dl256QamFlag;
    VOS_UINT8                           resv[3]; /* 保留字段，长度为3,便于4字节对齐 */
} RRC_APP_LwclashInd;

/*****************************************************************************
 结构名    :APP_RRC_NMR_REQ_STRU
 结构说明  :NMR查询请求
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
}APP_RRC_NmrReq;

/*****************************************************************************
 结构名    :RRC_APP_NMR_CGI_STRU
 结构说明  :NMR结构体定义
*****************************************************************************/
typedef struct {
    VOS_UINT16                          mncNum;           /*指示 MNC 个数*/
    VOS_UINT16                          mcc;
    VOS_UINT16                          mnc;
    VOS_UINT8                           res[2]; /* 保留字段，长度为2,便于4字节对齐 */
} RRC_APP_PlmnId;

/*****************************************************************************
 结构名    :RRC_APP_NMR_CGI_STRU
 结构说明  :NMR结构体定义
*****************************************************************************/
typedef struct {
    RRC_APP_PlmnId                plmnId;
    VOS_UINT32                          cellId;            /* 范围：(0,503) */
} RRC_APP_CellGlobalId;

/*****************************************************************************
 结构名    :RRC_APP_NMR_CGI_STRU
 结构说明  :NMR结构体定义
*****************************************************************************/
typedef struct {
    RRC_APP_CellGlobalId         cellGloId;        /* CellGlobalId */
    VOS_UINT16                          tac;
    VOS_UINT16                          pci;              /* 0xFFFF为无效,范围：(0,503) */
    VOS_INT16                           rsrp;              /* RSRP测量值范围：(-141,-44) */
    VOS_INT16                           rsrq;              /* RSRQ测量值范围：(-40, -6) */
    VOS_UINT16                          ta;               /* TA值*/
    VOS_UINT8                           res[2]; /* 保留字段，长度为2,便于4字节对齐 */
} RRC_APP_LteCellInfo;

/*****************************************************************************
 结构名    :RRC_APP_NMR_CNF_STRU
 结构说明  :NMR查询请求响应
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    APP_ResultUint32              result;
    RRC_APP_LteCellInfo          lteCelInfo;
}RRC_APP_NmrCnf;

/*****************************************************************************
 结构名    :APP_RRC_CELLID_REQ_STRU
 结构说明  :小区基本信息查询请求
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
}APP_RRC_CellidReq;

/*****************************************************************************
 结构名    : RRC_APP_CELLID_CNF_STRU
 结构说明  : RRC回复的小区基本信息
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    APP_ResultUint32              result;
    APP_PlmnId                    plmnId;
    VOS_UINT32                          ci;               /* 0xFFFF为无效, */
    VOS_UINT16                          pci;              /* 0xFFFF为无效,范围：(0,503) */
    VOS_UINT16                          tac;             /* TAC */
}RRC_APP_CellidCnf;

/*****************************************************************************
  6 UNION
*****************************************************************************/


/*****************************************************************************
  7 Extern Global Variable
*****************************************************************************/
/*****************************************************************************
 结构名    : APP_LPS_MSG_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈和APP间的接口消息的结构体
*****************************************************************************/
typedef struct {
    VOS_UINT32                          msgId;      /*消息ID*/
    VOS_UINT32                          size;       /*消息体的长度*/
    VOS_UINT8                           value[4];  /*消息体有效内容的指针*/
}APP_LPS_Msg;


/*****************************************************************************
 结构名    : APP_LRRC_RESEL_OFFSET_CFG_NTF_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈和APP间的接口消息的结构体
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                         /*_H2ASN_Skip*/
    VOS_UINT32             msgId;        /*_H2ASN_Skip*/
    APP_MSG_HEADER

    VOS_UINT32             opId;
    VOS_UINT32             flag;
}APP_LRRC_ReselOffsetCfgNtf;

/*****************************************************************************
 结构名    : APP_LRRC_CON_TO_IDLE_NTF_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈和APP间的接口消息的结构体
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                         /*_H2ASN_Skip*/
    VOS_UINT32             msgId;        /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32              opId;
    VOS_INT32              reserv;
}APP_LRRC_ConToIdleNtf;


/*****************************************************************************
 枚举名    : LRRC_LPDCP_FAST_DORMANCY_CMD_ENUM
 协议表格  :
 ASN.1描述 :
 枚举说明  : 指示PDCP启动或停止FAST_DORMANCY
*****************************************************************************/
enum APP_LRRC_FastDorm {
    APP_LRRC_FAST_DORMANCY_STOP         = 0,                                  /* 停止FAST_DORMANCY */
    APP_LRRC_FAST_DORMANCY_START,                                             /* 启动FAST_DORMANCY */
    APP_LRRC_FAST_DORMANCY_BUTT
};
typedef VOS_UINT32 APP_LRRC_FastDormUint32;

/*****************************************************************************
 结构名    : APP_LRRC_FAST_DORM_CFG_NTF_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈和APP间的接口消息的结构体
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                         /*_H2ASN_Skip*/
    VOS_UINT32             msgId;        /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;

    APP_LRRC_FastDormUint32      flag;
    VOS_UINT32            timerLen;   /* unite: s */
}APP_LRRC_FastDormCfgNtf;



/*****************************************************************************
 结构名    : ID_APP_LRRC_SET_UE_REL_VERSION_REQ
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈和APP间的接口消息的结构体
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                         /*_H2ASN_Skip*/
    VOS_UINT32             msgId;        /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    VOS_UINT32              mode;     /* 2:LTE , 1:TDS */
    VOS_UINT32              version;  /* 9 - 14 */
}APP_LRRC_SetUeRelVersionReq;

/*****************************************************************************
 结构名    : ID_LRRC_APP_SET_UE_REL_VERSION_CNF
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈和APP间的接口消息的结构体
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                      /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;        /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;         /* MSP直接将此ID做为CmdID发给Prob */
    APP_ResultUint32              result;
} LRRC_APP_SetUeRelVersionCnf;


/*****************************************************************************
 结构名    :APP_RRC_SCELL_INFO_REQ_STRU
 结构说明  :AT下发的查询SCELL信息的结构体
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    VOS_UINT8                           reserved[4]; /* 保留字段，长度为4,便于4字节对齐 */
}APP_RRC_ScellInfoReq;

/*****************************************************************************
 结构名    : RRC_SCELL_INFO_STRU
 结构说明  :
*****************************************************************************/
typedef struct {
    VOS_UINT32  phyCellId;          /* scell的物理小区id */
    VOS_UINT32  scellUlFreq;        /* scell上行频率 */
    VOS_UINT32  scellDlFreq;        /* scell下行频率 */
    VOS_UINT32  scellUlFreqPoint;     /* scell上行频点*/
    VOS_UINT32  scellDlFreqPoint;     /* scell下行频点*/
    VOS_UINT8    scellUlBandWidth;   /* scell上行带宽*/
    VOS_UINT8    scellDlBandWidth;   /* scell下行带宽*/
    VOS_UINT8    freqBandIndicator;   /* scell 频段*/
    VOS_UINT8    rsv;   /* 保留 */
    VOS_INT16    rssi;
    VOS_INT16    rsrp;
    VOS_INT16    rsrq;
    VOS_UINT16  scellIndex;
}RRC_SCELL_Info;

/*****************************************************************************
 结构名    :RRC_APP_SCELL_INFO_CNF_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :对原语APP_RRC_SCELL_INFO_REQ_STRU进行回复
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                            msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                            opId;
    APP_ResultUint32      result;
    VOS_UINT32                            sCellCnt;
    RRC_SCELL_Info           sCellInfo[LRRC_SCELL_MAX_NUM];
}RRC_APP_ScellInfoCnf;

/*****************************************************************************
 结构名    : LRRC_DAM_BAR_LIST_ITEM_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :
*****************************************************************************/
typedef struct {
    VOS_UINT16                          freqInfo;
    VOS_UINT16                          cellId;
    VOS_UINT8                           bandInd;
    VOS_UINT8                           reserved[3]; /* 保留字段，长度为3,便于4字节对齐 */
    VOS_UINT32                          timeOutTimeInMs;
    VOS_UINT32                          remainTimeOutInMs;
}LRRC_DAM_BarListItem;

/*****************************************************************************
 结构名    : LRRC_APP_DAM_BAR_LIST_DBG_INFO_IND_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    VOS_UINT16                          itemCount;        /* Bar List中条目个数 */
    VOS_UINT16                          rev;              /* 保留 */
    LRRC_DAM_BarListItem         damBarListItem[RRC_APP_MAX_LIMITED_ITEM_COUNT];
}LRRC_APP_DamBarListDbgInfoInd;
/*****************************************************************************
 结构名    : LRRC_APP_IDLE_SERVE_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈和APP间的接口消息的结构体
*****************************************************************************/
typedef struct {
    VOS_UINT16                          currReselPrior;    /* 当前使用的重选优先级，移动性转换的时候 */
    VOS_UINT16                          bandInd;           /* 频带指示 */
    VOS_UINT16                          freqInfo;          /* 服务小区频点 */
    VOS_UINT16                          cellId;            /* 服务小区ID */
    VOS_INT16                           rsrp;               /* RSRP测量值 */
    VOS_INT16                           rsrq;               /* RSRQ测量值 */
    VOS_INT16                           rssi;               /* RSSI测量值 */
    VOS_INT16                           value;             /*计算得到的SP值*/
    VOS_INT16                           squal;              /*计算得到的SQ值*/
    VOS_INT16                           qrxLevMin;          /*准则RSRP评估参数*/
    VOS_INT16                           pcompensation;      /* 根据 P-Max 和 UE Max Tx power 推算出来的 PCompensation */
    VOS_INT16                           qqualMin;           /* S准则RSRQ评估参数 */
    VOS_INT16                           sIntraSearchP;      /* 启动同频测量的rsrp阈值参数 */
    VOS_INT16                           sIntraSearchQ;      /* 启动同频测量的rsrq阈值参数 */
    VOS_INT16                           sNonIntraSearchP;   /* 启动同优先级和低优先级异频测量的rsrp阈值参数 */
    VOS_INT16                           sNonIntraSearchQ;   /* 启动同优先级和低优先级异频测量的rsrq阈值参数 */
    VOS_INT16                           threshServingLowP;  /* 针对低优先级小区重选的服务小区质量门限 */
    VOS_INT16                           threshServingLowQ;  /* 重选评估时使用R9参数 */
} LRRC_APP_IdleServeInfo;


/*****************************************************************************
 结构名    : LRRC_APP_IDLE_INTRA_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈和APP间的接口消息的结构体
*****************************************************************************/
typedef struct {
    VOS_UINT16                          cellId;
    VOS_INT16                           sValue;            /*计算得到的SP值*/
    VOS_INT16                           squal;             /*计算得到的SQ值*/
    VOS_INT16                           rsrp;              /* RSRP测量值 */
    VOS_INT16                           rsrq;              /* RSRQ测量值 */
    VOS_INT16                           rssi;              /* RSSI测量值 */
}LRRC_APP_IdleCellMeasResult;

typedef struct {
    VOS_UINT16                          currReselPrior;    /* 当前使用的重选优先级，移动性转换的时候 */
    VOS_UINT16                          bandInd;           /* 频带指示 */
    VOS_UINT16                          freqInfo;          /*频点*/
    VOS_UINT16                          totalCellNum;      /*小区总数*/
    VOS_UINT16                          detectedCellNum;   /*检测到的小区数目*/
    VOS_INT16                           qrxLevMin;          /*计算SP值用到的最低接入电平*/
    VOS_INT16                           pcompensation;      /* 根据 P-Max 和 UE Max Tx power 推算出来的 PCompensation */
    VOS_INT16                           qqualMin;           /* S准则RSRQ评估参数 */
    VOS_INT16                           sIntraSearchP;      /* 启动同频测量的rsrp阈值参数 */
    VOS_INT16                           sIntraSearchQ;      /* 启动同频测量的rsrq阈值参数 */
    LRRC_APP_IdleCellMeasResult         intraCellMesRslt[LRRC_LCSEL_INTRA_CELL_REPORT_NUM];
}LRRC_APP_IdleIntraInfo;



/*****************************************************************************
 结构名    : LRRC_APP_IDLE_INTER_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈和APP间的接口消息的结构体
*****************************************************************************/
typedef struct {
    VOS_UINT16                              currReselPrior;    /* 当前使用的重选优先级，移动性转换的时候 */
    VOS_UINT16                              freqInfo;
    VOS_UINT16                              bandInd;           /* 频带指示 */
    VOS_INT16                               threshLowP;         /* 针对低优先级小区重选的服务小区质量门限 */
    VOS_INT16                               threshLowQ;         /* 重选评估时使用R9参数 */
    VOS_INT16                               threshHighP;        /* 针对低优先级小区重选的服务小区质量门限 */
    VOS_INT16                               threshHighQ;        /* 重选评估时使用R9参数 */
    VOS_UINT16                              totalCellNum;      /*小区总数*/
    VOS_UINT16                              detectedCellNum;   /*检测到的小区数目*/
    LRRC_APP_IdleCellMeasResult             interCellMesRslt[LRRC_LCSEL_INTER_CELL_REPORT_NUM];/* SCELL放在第一个的位置 */
}LRRC_APP_IdleSingleFreqMeasResult;

typedef struct {
    VOS_UINT16                                   interFreqNum;
    VOS_INT16                                    sNonIntraSearchP;   /* 启动同优先级和低优先级异频测量的rsrp阈值参数 */
    VOS_INT16                                    sNonIntraSearchQ;   /* 启动同优先级和低优先级异频测量的rsrq阈值参数 */
    VOS_INT16                                    servValue;          /*计算得到的SP值*/
    VOS_INT16                                    servqual;           /*计算得到的SQ值*/
    VOS_INT16                                    threshServingLowP;  /* 针对低优先级小区重选的服务小区质量门限 */
    VOS_INT16                                    threshServingLowQ;  /* 重选评估时使用R9参数 */
    LRRC_APP_IdleSingleFreqMeasResult            interFreqMesRslt[LRRC_LCSEL_INTER_FREQ_REPORT_NUM];
}LRRC_APP_IdleInterInfo;

/*****************************************************************************
 结构名    : LRRC_APP_IDLE_UTRAN_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈和APP间的接口消息的结构体
*****************************************************************************/
typedef struct {
    VOS_UINT16                              primaryCode;
    VOS_INT16                               rscp;               /* 精度1/8 */
    VOS_INT16                               ecn0;               /* 精度1/8 */
    VOS_INT16                               sValue;             /*计算得到的SP值*/
    VOS_INT16                               squal;              /*计算得到的SQ值*/
}LRRC_APP_UtranCellMeasResult;

typedef struct {
    VOS_UINT16                              currReselPrior;    /* 当前使用的重选优先级，移动性转换的时候 */
    VOS_UINT16                              arfcn;
    VOS_INT16                               rssi;               /* 精度1/8 */
    VOS_INT16                               qrxLevMin;          /*计算SP值用到的最低接入电平*/
    VOS_INT16                               pcompensation;      /* 根据 P-Max 和 UE Max Tx power 推算出来的 PCompensation */
    VOS_INT16                               qqualMin;           /* S准则RSRQ评估参数 */
    VOS_INT16                               threshLowP;         /* 针对低优先级小区重选的服务小区质量门限 */
    VOS_INT16                               threshLowQ;         /* 重选评估时使用R9参数 */
    VOS_INT16                               threshHighP;        /* 针对低优先级小区重选的服务小区质量门限 */
    VOS_INT16                               threshHighQ;        /* 重选评估时使用R9参数 */
    VOS_UINT16                              cellNum;
    LRRC_APP_UtranCellMeasResult    utranCellMesRslt[LRRC_LCSEL_UTRAN_CELL_REPORT_NUM];
}LRRC_APP_UtranSignleFreqMeasResult;

typedef struct {
    VOS_UINT16                                      arfcnNum;
    VOS_INT16                                       sNonIntraSearchP;   /* 启动同优先级和低优先级异频测量的rsrp阈值参数 */
    VOS_INT16                                       sNonIntraSearchQ;   /* 启动同优先级和低优先级异频测量的rsrq阈值参数 */
    VOS_INT16                                       servValue;          /*计算得到的SP值*/
    VOS_INT16                                       servqual;           /*计算得到的SQ值*/
    VOS_INT16                                       threshServingLowP;  /* 针对低优先级小区重选的服务小区质量门限 */
    VOS_INT16                                       threshServingLowQ;  /* 重选评估时�褂�9参数 */
    LRRC_APP_UtranSignleFreqMeasResult              utranFreqMesRslt[LRRC_TRRC_PHY_MAX_SUPPORT_CARRIER_NUM];
}LRRC_APP_IdleUtranInfo;

/*****************************************************************************
 结构名    : LRRC_APP_IDLE_UTRAN_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈和APP间的接口消息的结构体
*****************************************************************************/
typedef struct {
    VOS_UINT16                              currReselPrior;   /* 当前使用的重选优先级，移动性转换的时候 */
    VOS_UINT16                              arfcn;
    VOS_UINT16                              bandInd;          /* 频点指示 */
    VOS_INT16                               rssi;              /* 精度1/8 */
    VOS_UINT16                              ncc;
    VOS_UINT16                              bcc;
    VOS_INT16                               svalue;             /*计算得到的SP值*/
    VOS_INT16                               qrxLevMin;          /*计算SP值用到的最低接入电平*/
    VOS_INT16                               pcompensation;      /* 根据 P-Max 和 UE Max Tx power 推算出来的 PCompensation */
    VOS_INT16                               threshLowP;         /* 针对低优先级小区重选的服务小区质量门限 */
    VOS_INT16                               threshHighP;        /* 针对低优先级小区重选的服务小区质量门限 */
}LRRC_APP_GeranSignleFreqMeasResult;

typedef struct {
    VOS_UINT16                                    arfcnNum;
    VOS_INT16                                     sNonIntraSearchP;   /* 启动同优先级和低优先级异频测量的rsrp阈值参数 */
    VOS_INT16                                     sNonIntraSearchQ;   /* 启动同优先级和低优先级异频测量的rsrq阈值参数 */
    VOS_INT16                                     servValue;          /*计算得到的SP值*/
    VOS_INT16                                     servqual;           /*计算得到的SQ值*/
    VOS_INT16                                     threshServingLowP;  /* 针对低优先级小区重选的服务小区质量门限 */
    VOS_INT16                                     threshServingLowQ;  /* 重选评估时使用R9参数 */
    LRRC_APP_GeranSignleFreqMeasResult            geranFreqMesRslt[LRRC_GURRC_GERAN_ARFCN_MAX_NUM];
}LRRC_APP_IdleGeranInfo;


/*****************************************************************************
 结构名    : LRRC_APP_CELL_MEAS_RSLT_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈和APP间的接口消息的结构体
*****************************************************************************/
typedef struct {
    VOS_UINT16                          cellId;
    VOS_UINT16                          freqInfo;
    VOS_INT16                           rsrp;              /* RSRP测量值 */
    VOS_INT16                           rsrq;              /* RSRQ测量值 */
    VOS_INT16                           rssi;              /* RSSI测量值 */
} LRRC_APP_CellMeasRslt;

/*****************************************************************************
 结构名    : LRRC_APP_UTRAN_MEAS_RSLT_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈和APP间的接口消息的结构体
*****************************************************************************/
typedef struct {
    VOS_UINT16                          arfcn;
    VOS_UINT16                          primaryCode;
    VOS_INT16                           rscp;               /* 精度1/8 */
    VOS_INT16                           ecn0;               /* 精度1/8 */
    VOS_INT16                           rssi;               /* 精度1/8 */
    VOS_UINT16                          utranType;
} LRRC_APP_UtranCellRslt;
/*****************************************************************************
 枚举名     :LRRC_GRR_BANDINDICATOR_ENUM
 协议表格  :
 ASN.1描述   :
 枚举说明 : 2G小区频带指示
*****************************************************************************/
enum LRRC_APP_GeranBandind {
    DCS1800                          = 0,
    PCS1900                          = 1,

    LRRC_APP_GERAN_BANDIND_BUTT      = 0x7FFF
};
typedef VOS_UINT16    LRRC_APP_GeranBandindUint16;

/*****************************************************************************
 枚举名    : RRC_APP_PROTOCOL_STATE
 协议表格  :
 ASN.1描述 :
 枚举说明  : RRC协议状态类型
*****************************************************************************/
enum RRC_APP_ProtocolState {
    RRC_MEAS_PROTOCOL_STATE_IDLE            = 0 ,
    RRC_MEAS_PROTOCOL_STATE_CONNECTED,
    RRC_MEAS_PROTOCOL_STATE_BUTT            = 0x7FFF
};
typedef VOS_UINT16 RRC_APP_ProtocolStateUint16;

/*****************************************************************************
 枚举名     :LRRC_GRR_BANDINDICATOR_ENUM
 协议表格  :
 ASN.1描述   :
 枚举说明 : 2G小区频带指示
*****************************************************************************/
enum LRRC_GRR_BandInd {
    LRRC_GRR_BANDINDICATOR_DCS1800                          = 0,
    LRRC_GRR_BANDINDICATOR_PCS1900                             ,

    LRRC_GRR_BANDINDICATOR_BUTT
};
typedef VOS_UINT16    LRRC_GRR_BandIndUint16;

/*****************************************************************************
 结构名    : LRRC_APP_UTRAN_MEAS_RSLT_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈和APP间的接口消息的结构体
*****************************************************************************/
typedef struct {
    VOS_UINT16                          arfcn;
    LRRC_APP_GeranBandindUint16         bandInd;          /* 频点指示 */
    VOS_INT16                           rssi;              /* 精度1/8 */
} LRRC_APP_GeranCellRslt;

/*****************************************************************************
 结构名    : LRRC_APP_SERV_MEAS_RSLT_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈和APP间的接口消息的结构体
*****************************************************************************/
typedef struct {
    RRC_APP_ProtocolStateUint16 state;             /* 当前协议状态 */
    LRRC_APP_CellMeasRslt       servCellRslt;      /* 服务小区上报结果 */
} LRRC_APP_ServMeasRslt;

/*****************************************************************************
 结构名    : LRRC_APP_SCELL_MEAS_RSLT_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈和APP间的接口消息的结构体
*****************************************************************************/
typedef struct {
    LRRC_APP_CellMeasRslt       sCellRslt[LRRC_SCELL_MAX_NUM];          /* SCell上报结果 */
} LRRC_APP_ScellMeasRslt;

/*****************************************************************************
 结构名    : LRRC_APP_INTRA_MEAS_RSLT_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈和APP间的接口消息的结构体
*****************************************************************************/
typedef struct {
    LRRC_APP_CellMeasRslt       intraCellRslt[LRRC_INTRA_CELL_MAX_NUM]; /* 同频小区上报结果 */
} LRRC_APP_IntraMeasRslt;

/*****************************************************************************
 结构名    : LRRC_APP_INTER_MEAS_RSLT_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈和APP间的接口消息的结构体
*****************************************************************************/
typedef struct {
    LRRC_APP_CellMeasRslt       interCellRslt[LRRC_INTER_CELL_MAX_NUM]; /* 异频小区上报结果 */
} LRRC_APP_InterMeasRslt;
/*****************************************************************************
 结构名    : LRRC_APP_INTER_MEAS_RSLT_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈和APP间的接口消息的结构体
*****************************************************************************/
typedef struct {
    LRRC_APP_UtranCellRslt     utranCellRslt[LRRC_UTRAN_CELL_MAX_NUM]; /* 异频小区上报结果 */
} LRRC_APP_UtranMeasRslt;
/*****************************************************************************
 结构名    : LRRC_APP_INTER_MEAS_RSLT_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 协议栈和APP间的接口消息的结构体
*****************************************************************************/
typedef struct {
    LRRC_APP_GeranCellRslt       geranCellRslt[LRRC_GERAN_CELL_MAX_NUM]; /* 异频小区上报结果 */
} LRRC_APP_GeranMeasRslt;

/* add for Conn Meas Filter Optimize begin */
/*****************************************************************************
 结构名    : LRRC_CONN_MEAS_FILTER_INFO_IND_TYPE_ENUM
 结构说明  : LRRC上报滤波结果的类型
*****************************************************************************/
enum LRRC_CONN_MeasFilterInfoIndType {
    LRRC_CONN_MEAS_FILTER_EUTRA_INFO,    /* _H2ASN_MsgChoice LRRC_CONN_MEAS_FILTER_EUTRA_INFO_STRU */
    LRRC_CONN_MEAS_FILTER_GERA_INFO,    /* _H2ASN_MsgChoice LRRC_CONN_MEAS_FILTER_GERA_INFO_STRU */
    LRRC_CONN_MEAS_FILTER_UTRA_INFO,    /* _H2ASN_MsgChoice LRRC_CONN_MEAS_FILTER_UTRA_INFO_STRU */
    LRRC_CONN_MEAS_FILTER_CDMA_INFO,    /* _H2ASN_MsgChoice LRRC_CONN_MeasFilterCdmaInfo */
    LRRC_CONN_MEAS_FILTER_NR_INFO,      /* _H2ASN_MsgChoice LRRC_CONN_MeasFilterNrInfo */
    LRRC_CONN_MEAS_FILTER_INFO_IND_TYPE_BUTT
};

typedef VOS_UINT32 LRRC_CONN_MeasFilterInfoIndTypeUint32;
/*****************************************************************************
 结构名    : LRRC_APP_MeasCellRslt
 协议表格  :
 ASN.1描述 :
 结构说明  : LRRC EUTRA滤波结果消息的结构体
*****************************************************************************/
typedef struct {
    VOS_UINT16         cellId;
    VOS_INT16          rsrp;
    VOS_INT16          rsrq;
    VOS_INT16          rssi;
    VOS_INT16          sinr;
    VOS_UINT16         rsv;
} LRRC_APP_EutraCellMeasRslt;
/*****************************************************************************
 结构名    : LRRC_APP_SendFilterMeasRslt
 结构说明  : LRRC上报滤波结果小区的类型
*****************************************************************************/

enum LRRC_APP_SendFilterMeasRslt {
    LRRC_APP_PCELL_CELL_MEAS_RSLT = 0,
    LRRC_APP_PSCELL_CELL_MEAS_RSLT,
    LRRC_APP_OTHER_CELL_MEAS_RSLT,
    LRRC_APP_CELL_MEAS_RSLT_BUTT,
};
typedef VOS_UINT8 LRRC_APP_SendFilterMeasRsltUint8;

/*****************************************************************************
 结构名    : LRRC_CONN_MEAS_FILTER_EUTRA_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : LRRC EUTRA滤波结果消息的结构体
*****************************************************************************/
typedef struct {
    VOS_UINT32                          arfcn;
    VOS_UINT16                          freqInfo;
    VOS_UINT8                           bandInd;
    VOS_UINT8                           measCellNum;
    LRRC_APP_EutraCellMeasRslt          measCellList[LRRC_APP_MAX_MEAS_CELL_NUM];
}LRRC_CONN_MeasFilterEutraInfo;


/*****************************************************************************
 结构名    :LRRC_GRR_BSIC_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :CELL BAIC Info，协议36331 6.3.4
            usNcc(GSM Network Colour Code)  正常范围:(0..7), 8 表示无效值
            usBcc(GSM Base Station Colour Code)  正常范围:(0..7) , 8 表示无效值
*****************************************************************************/
typedef struct {
    VOS_UINT16                                              ncc;
    VOS_UINT16                                              bcc;
}LRRC_GRR_BsicInfo;
/*****************************************************************************
 结构名    : LRRC_APP_GeranCellMeasRslt
 协议表格  :
 ASN.1描述 :
 结构说明  : LRRC GERAN滤波结果消息的结构体
*****************************************************************************/
typedef struct {
    VOS_UINT16                             arfcn;            /* 频点 */
    LRRC_GRR_BandIndUint16                 bandInd;          /* 频点指示 */
    PS_BOOL_ENUM_UINT8                     bsicVaild;    /* BSIC有效标志 */
    VOS_UINT8                              resv;         /* 保留位 */
    VOS_INT16                              rssi;          /* 滤波后的Rssi */
    LRRC_GRR_BsicInfo                      bsic;         /* BSIC标志有效时的BSIC信息 */

} LRRC_APP_GeranCellMeasRslt;

/*****************************************************************************
 结构名    : LRRC_CONN_MEAS_FILTER_GERA_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : LRRC GERAN滤波结果消息的结构体
*****************************************************************************/
typedef struct {
    VOS_UINT16                                              arfcnNum;           /* 频点个数 */
    VOS_UINT8                                               reserved[2]; /* 保留字段，长度为2,便于4字节对齐 */
    LRRC_APP_GeranCellMeasRslt                              geranCellResult[LRRC_APP_MAX_GERAN_MEAS_CELL_NUM];
}LRRC_CONN_MeasFilterGeraInfo;

/*****************************************************************************
 结构名    : LRRC_CONN_MEAS_UTRA_TYPE_ENUM
 结构说明  : LRRC上报滤波结果的类型
*****************************************************************************/
enum LRRC_CONN_MeasUtraType {
    LRRC_CONN_MEAS_FILTER_UTRA_FDD,    /*_H2ASN_Skip*/
    LRRC_CONN_MEAS_FILTER_UTRA_TDD,    /*_H2ASN_Skip*/
    LRRC_CONN_MEAS_FILTER_UTRA_TYPE_BUTT
};
typedef VOS_UINT8 LRRC_CONN_MeasUtraTypeUint8;

/*****************************************************************************
 结构名    : LRRC_APP_UtraCellMeasRslt
 协议表格  :
 ASN.1描述 :
 结构说明  : LRRC UTRA滤波结果消息的结构体
*****************************************************************************/
typedef struct {
    VOS_UINT16                cellId;        /* 小区id */
    VOS_INT16                 rscp;          /* 滤波后的Rscp */
    VOS_INT16                 ecN0;          /* 滤波后的EcN0 */
    VOS_UINT16                rsv;
} LRRC_APP_UtraCellMeasRslt;

/*****************************************************************************
 结构名    : LRRC_CONN_MEAS_FILTER_UTRA_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : LRRC FDD UTRA滤波结果消息的结构体
*****************************************************************************/
typedef struct {
    VOS_UINT16                              arfcn;
    LRRC_CONN_MeasUtraTypeUint8             utraType;             /* Utra的类型 */
    VOS_UINT8                               measCellNum;
    LRRC_APP_UtraCellMeasRslt               measCellList[LRRC_APP_MAX_MEAS_CELL_NUM];
}LRRC_CONN_MeasFilterUtraInfo;
/*****************************************************************************
 结构名    : LRRC_APP_CdmaCellMeasRslt
 协议表格  :
 ASN.1描述 :
 结构说明  : LRRC CDMA滤波结果消息的结构体
*****************************************************************************/
typedef struct {
    VOS_UINT16         cellId;
    VOS_UINT16         pilotPhase;
    VOS_INT16          sPilotStrength;
    VOS_UINT16         reserved; /* 4字节对齐保留字节 */
} LRRC_APP_CdmaCellMeasRslt;

/*****************************************************************************
 结构名    : LRRC_CONN_MeasFilterCdmaInfo
 协议表格  :
 ASN.1描述 :
 结构说明  : LRRC CDMA滤波结果消息的结构体
*****************************************************************************/
typedef struct {
    VOS_UINT16                        arfcn;
    VOS_UINT8                         bandClass;
    VOS_UINT8                         cdma2000Type;
    VOS_UINT32                        measCellNum;
    LRRC_APP_CdmaCellMeasRslt         measCellList[LRRC_APP_MAX_MEAS_CELL_NUM];
} LRRC_CONN_MeasFilterCdmaInfo;

/*****************************************************************************
 结构名    : LRRC_APP_MeasCellRslt
 协议表格  :
 ASN.1描述 :
 结构说明  : LRRC Nr滤波结果消息的结构体
*****************************************************************************/
typedef struct {
    VOS_UINT16         cellId;
    VOS_INT16          rsrp;
    VOS_INT16          rsrq;
    VOS_INT16          sinr;
} LRRC_APP_NrCellMeasRslt;

/*****************************************************************************
 结构名    : LRRC_CONN_MeasFilterNrInfo
 协议表格  :
 ASN.1描述 :
 结构说明  : LRRC NR滤波结果消息的结构体
*****************************************************************************/
typedef struct {
    VOS_UINT32                                arfcnNr;                                    /* NR的频点信息 */
    VOS_UINT32                                measCellNum;
    LRRC_APP_NrCellMeasRslt                   detectCellList[LRRC_APP_MAX_MEAS_CELL_NUM]; /* Detect小区 */
} LRRC_CONN_MeasFilterNrInfo;

/*****************************************************************************
 结构名    : LRRC_CONN_MEAS_FILTER_INFO_IND_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : LRRC滤波结果消息的结构体
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    LRRC_CONN_MeasFilterInfoIndTypeUint32       unionStructSelChoice;/*_H2ASN_Skip*/
    union  /* _H2ASN_Skip */
    {      /* _H2ASN_Skip */
        LRRC_CONN_MeasFilterEutraInfo       eutraConnMeasFilterInfo;   /* _H2ASN_Skip */
        LRRC_CONN_MeasFilterGeraInfo        geraConnMeasFilterInfo;   /* _H2ASN_Skip */
        LRRC_CONN_MeasFilterUtraInfo        utraConnMeasFilterInfo;   /* _H2ASN_Skip */
        LRRC_CONN_MeasFilterCdmaInfo        cdmaConnMeasFilterInfo;   /* _H2ASN_Skip */
        LRRC_CONN_MeasFilterNrInfo          nrConnMeasFilterInfo;     /* _H2ASN_Skip */
    }u;  /* _H2ASN_Skip */
}LRRC_CONN_MeasFilterInfoInd;
/* add for Conn Meas Filter Optimize end */

/*****************************************************************************
 结构名    : APP_OM_MSG_REDF_STRU
 结构说明  : APP(后台工具)与OM交互的消息体(和APP_OM_MSG_STRU相同)
*****************************************************************************/
typedef struct {
     VOS_MSG_HEADER                     /*VOS头*/
     VOS_UINT32          msgId;
     APP_MSG_HEADER                     /*APP头*/
     VOS_UINT8           para[4];    /* 参数内容 */
}APP_OM_MsgRedf;



/*****************************************************************************
 结构名    :APP_RRC_DEBUG_STUB_SET_3HOUR_REQ_STRU
 结构说明  :打桩3小时定时器
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                      msgId;                /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                      opId;
    VOS_UINT32                      timerLength;         /* 3小时定时器时长 */
}APP_RRC_DebugStubSet3hourReq;

/*****************************************************************************
 结构名    :APP_RRC_DebugStubSetLostCoverageReq
 结构说明  :和OM之间消息开关及时间设置：打桩设置丢网定时器时间
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32                      msgId;
    APP_MSG_HEADER
    VOS_UINT32                      opId;
    VOS_UINT8                       lostCoverrageSwitch;          /* 打桩丢网定时器开关, 0表示关闭，1表示打开 */
    VOS_UINT8                       rsv;                          /* 1为保留位 */
    VOS_UINT16                      timerLength;                  /* 丢网定时器长度，只在开关打开时生效 */
} APP_RRC_DebugStubSetLostCoverageReq;

/*****************************************************************************
 结构名    :APP_RRC_DEBUG_STUB_HO_FAIL_REQ_STRU
 结构说明  :打桩重建立
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                      msgId;                /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                      opId;
    VOS_UINT32                      hoFailFlag;         /* 1为打桩重建立，0为不打桩 */
}APP_RRC_DebugStubHoFailReq;
/*****************************************************************************
 结构名    :APP_RRC_DEBUG_STUB_REL_ALL_REQ_STRU
 结构说明  :打桩REL ALL
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                      msgId;                /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                      opId;
    VOS_UINT32                      relAllCmd;            /* 1为打桩REL_ALL，0为不打桩 */
}APP_RRC_DebugStubRelAllReq;
/*****************************************************************************
 结构名    :RRC_APP_DEBUG_STUB_CMD_CNF_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                          msgId;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                          opId;
    APP_ResultUint32              result;
}RRC_APP_DebugStubCmdCnf;



/*****************************************************************************
 结构名    :APP_RRC_DEBUG_STUB_ADD_BAD_CELL_REQ_STRU
 结构说明  :打桩坏小区列表
*****************************************************************************/
#ifdef MODEM_FUSION_VERSION
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                      msgId;
    APP_MSG_HEADER
    VOS_UINT32                      opId;
    VOS_UINT32                      addBadCellFlag; /* 1加坏小区，0为删除坏小区 */
    VOS_UINT16                      band;
    VOS_UINT16                      freq;
    VOS_UINT16                      phyCellId;
    VOS_UINT16                      errCnt;
    VOS_UINT32                      cause;
    VOS_UINT32                      cgi;
    VOS_UINT8                       reselLimitLen;
    VOS_UINT8                       hoLimitLen;
    VOS_UINT8                       searchLimitLen;
    VOS_UINT8                       estFailRejLimitLen;
    VOS_INT16                       rsrp;
    VOS_INT16                       rsrq;    /* 坏小区添加时的能量，和空口保持一致 */
    VOS_UINT16                      sinr;
    VOS_UINT16                      rsv5;
}APP_RRC_DebugStubAddBadCellReq;
#else
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                      msgId;                /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                      opId;
    VOS_UINT32                      addBadCellFlag;         /* 1加坏小区，0为删除坏小区 */
    VOS_UINT16                      band;
    VOS_UINT16                      freq;
    VOS_UINT16                      phyCellId;
    VOS_UINT16                      cause;
    VOS_UINT32                      exitFLow;
    VOS_UINT32                      rsv;
    VOS_UINT8                       rsv0;
    VOS_UINT8                       rsv1;
    VOS_UINT8                       rsv2;
    VOS_UINT8                       rsv3;
    VOS_INT16                       rsrp;
    VOS_INT16                       rsrq;    /* 坏小区添加时的能量 */
    VOS_UINT16                      rsv4;
    VOS_UINT16                      rsv5;
}APP_RRC_DebugStubAddBadCellReq;
#endif

/*****************************************************************************
 结构名    :APP_RRC_DEBUG_STUB_L2W_RESEL_RESTRAIN_REQ_STRU
 结构说明  :打桩重建立
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                                          /*_H2ASN_Skip*/
    VOS_UINT32                      msgId;                /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                      opId;
    VOS_UINT32                      limitFlag;         /* 1为打桩L2W重选抑制，0为不打桩 */
}APP_RRC_DebugStubL2WReselLimitReq;

/*****************************************************************************
 结构名    :LTE_CA_STATE_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :服务小区及同频异频邻区信息
****************************************************************************/
typedef struct {
    VOS_UINT32    mcc;         //无效值0xFFFFFFFF
    VOS_UINT32    mnc;         //MCC-MNC，无效值0xFFFFFFFF
    VOS_UINT16    arfcn;       //频点
    VOS_UINT16    band;        //频段
    VOS_UINT32    pci;         //PCI
    VOS_UINT32    cgi;         //CGI，无效值0xFFFFFFFF
    VOS_UINT16    tac;         //TAC，无效值0xFFFF
    VOS_UINT16    bW;          //BW，无效值0xFFFF
    VOS_INT16     rsrp;
    VOS_INT16     rssi;
    VOS_INT16     rsrq;
    VOS_UINT8     reserved[6]; /* 保留字段，长度为6,便于4字节对齐 */
} PS_LTE_CellInfo;

/*****************************************************************************
 结构名    :LTE_CA_STATE_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :增强小区信息，包含服务小区，邻区等信息，最多支持8个邻区
****************************************************************************/
typedef struct {
    VOS_UINT32    flag;           //为了前台解析兼容，当前默认为1
    VOS_UINT16    length;         //长度，此字节之后数据长度（不包含此字节）
    VOS_UINT8     ueState;        /*状态*,无效值为0xFF*/
    VOS_UINT8     reserved[5]; /* 保留字段，长度为5,便于4字节对齐 */
    VOS_UINT32    activeCount;    //服务小区个数
    VOS_UINT32    nbrCount;       //邻区个数
    PS_LTE_CellInfo     cellInfo[LRRC_ECELL_QRY_CELL_REPORT_NUM];
} PS_LTE_EcellInfo;

/*****************************************************************************
 结构名    :CA_STATE_INFO
 协议表格  :
 ASN.1描述 :
 结构说明  :Scell相关信息
*****************************************************************************/
typedef struct {
    VOS_UINT32    cellId;    //小区ID
    VOS_UINT32    arfcn;     //优先返回频点、（频率）
} CA_STATE_Info;

/*****************************************************************************
 结构名    :LTE_CA_STATE_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :CA相关信息
****************************************************************************/
typedef struct {
    VOS_UINT32        flag;        //默认为1
    VOS_UINT16        length;      //长度
    VOS_UINT8         reserved[2]; /* 保留字段，长度为2,便于4字节对齐 */
    VOS_UINT32        count;       //CA信息的个数
    CA_STATE_Info     cAInfo[LRRC_ECELL_QRY_CELL_REPORT_NUM];      //CA数据信息
} LTE_CA_StateInfo;

/*****************************************************************************
 结构名    :RRC_APP_SCELL_INFO_CNF_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :查询增强小区信息，包含服务小区，邻区等信息
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                         /*_H2ASN_Skip*/
    VOS_UINT32         msgID;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32         opId;
    VOS_UINT8          reserved[4]; /* 保留字段，长度为4,便于4字节对齐 */
}APP_RRC_QueryEcellInfoReq;

/*****************************************************************************
 结构名    :RRC_APP_SCELL_INFO_CNF_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :对原语APP_RRC_SCELL_INFO_REQ_STRU进行回复
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                         /*_H2ASN_Skip*/
    VOS_UINT32         msgID;            /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32         opId;
    VOS_UINT8          reserved[4]; /* 保留字段，长度为4,便于4字节对齐 */
}APP_RRC_QueryScellInfoReq;

/*****************************************************************************
 结构名    :RRC_APP_SCELL_INFO_CNF_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :对原语APP_RRC_ECELL_INFO_REQ_STRU进行回复
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                               /*_H2ASN_Skip*/
    VOS_UINT32                 msgId;          /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                 opId;
    PS_LTE_EcellInfo     eCellInfo;   /*增强型小区，服务小区及邻区*/
}RRC_APP_QueryEcellInfoCnf;

/*****************************************************************************
 结构名    :RRC_APP_SCELL_INFO_CNF_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :对原语APP_RRC_SCELL_INFO_REQ_STRU进行回复
*****************************************************************************/
typedef struct {
    VOS_MSG_HEADER                               /*_H2ASN_Skip*/
    VOS_UINT32                 msgId;          /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32                 opId;
    LTE_CA_StateInfo         cellInfo;      /*CA小区信息*/
}RRC_APP_QueryScellInfoCnf;



extern PS_BOOL_ENUM_UINT8  LRRC_COMM_LoadDspAlready( MODEM_ID_ENUM_UINT16 modemId);

extern VOS_UINT32 LHPA_InitDsp( VOS_VOID );
extern VOS_VOID LHPA_DbgSendSetWorkMode_toMaterMode(MODEM_ID_ENUM_UINT16 modemId);
extern VOS_VOID LHPA_DbgSendSetWorkMode_toSlaveMode(MODEM_ID_ENUM_UINT16 modemId);

extern VOS_VOID * LAPP_MemAlloc( MODEM_ID_ENUM_UINT16 modemId,VOS_UINT32 size );
extern VOS_UINT32  LApp_MemFree(MODEM_ID_ENUM_UINT16 modemId,VOS_VOID *addr );
extern VOS_UINT32  LAppSndMsgToLPs(MODEM_ID_ENUM_UINT16 modemId,APP_LPS_Msg  *appToPsMsg );
extern 	VOS_UINT32 LHPA_InitDsp_ForAT( VOS_VOID );
extern VOS_UINT32 LHPA_InitDspNvForLteTdsCBT(MODEM_ID_ENUM_UINT16 modemId);
extern VOS_UINT32 LHPA_LoadDspForLteCBT(MODEM_ID_ENUM_UINT16 modemId);
extern VOS_VOID   LHPA_DbgSetSlaveModeThenMasterMode(MODEM_ID_ENUM_UINT16 modemId);


/*****************************************************************************
  8 Fuction Extern
*****************************************************************************/


/*****************************************************************************
  9 OTHERS
*****************************************************************************/










#if (VOS_OS_VER != VOS_WIN32)
#pragma pack()
#else
#pragma pack(pop)
#endif




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of app_rrc_interface.h */
