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
#ifndef  MN_CALL_API_H
#define  MN_CALL_API_H

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "vos.h"
#include "TafTypeDef.h"
#include "TafAppSsa.h"
#include "AtMnInterface.h"
#include "product_config.h"
#include "TafCallComm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 常量定义
*****************************************************************************/
/* 可选项是否存在的标志 */
#define MN_CALL_OPTION_EXIST            (1)
#define MN_CALL_OPTION_NOT_EXIST        (0)

#define MN_CALL_MAX_EMC_NUM             (80)
#define MN_CALL_ECC_NUM_VALID_SIM_PRESENT (1)
#define MN_CALL_ECC_NUM_VALID_SIM_ABSENT  (0)
#define MN_CALL_ECC_NUM_INVALID_CATEGORY  (0)

#define MN_CALL_RPT_CFG_MAX_SIZE        (8)
#define     MN_CALL_NET_RAT_TYPE_WCDMA      (0)           /* 当前驻留为W */
#define     MN_CALL_NET_RAT_TYPE_GSM        (1)           /* 当前驻留为G */
#define     MN_CALL_NET_RAT_TYPE_LTE        (2)           /* 当前驻留模为L */

#define TAF_CALL_DTMF_MIN_ONLENGTH              (65)                            /* DTMF音最小时长 */
#define TAF_CALL_DTMF_DEFAULT_ONLENGTH          (60000)                         /* 默认DTMF音时长60s */

#define TAF_CALL_DTMF_DEFAULT_OFFLENGTH          (75)                           /* stop dtmf ack和start dtmf req之间时间间隔大于70毫秒小于600毫秒,默认75毫秒 */

#define TAF_CALL_MAX_BC_NUM                             (2)                     /* 最大BC个数 */
#define TAF_CALL_MIN_BC_NUM                             (1)                     /* 最小BC个数 */

#if (FEATURE_ON == FEATURE_ECALL)
#define  TAF_ECALL_REDIAL_PERIOD_TIMER_LENGTH           (120000)                /* 120s */
#define  TAF_ECALL_REDIAL_INTERVAL_TIMER_LENGTH         (5000)                  /* 5s */
#define  TAF_CALL_T2_TIMER_LENGTH                       (3600000)               /* 时长为1小时 */
/* T9定时器时长可配置，默认配置为1小时，可配置时长范围: 1 hour<= T9 <= 12 hours */
#define  TAF_CALL_ONE_HOUR_TIMER_LENGTH                 (3600000)
#define  TAF_CALL_T9_MIN_TIMER_LENGTH                   (1)                     /* T9最小时长为1小时 */
#define  TAF_CALL_T9_MAX_TIMER_LENGTH                   (12)                    /* T9最大时长为12消息 */
#endif

#define TAF_CALL_MAX_NUM_DIGIT_LEN                          (64)
#define TAF_CALL_MAX_FLASH_DIGIT_LEN                        (32)
#define TAF_CALL_MAX_BURST_DTMF_NUM                         (255)




#define TAF_CALL_MAX_DISPALY_CHARI_OCTET_NUM                (64)
#define TAF_CALL_MAX_CALLED_NUMBER_CHARI_OCTET_NUM          (64)
#define TAF_CALL_MAX_CALLING_NUMBER_CHARI_OCTET_NUM         (64)
#define TAF_CALL_MAX_CONNECTED_NUMBER_CHARI_OCTET_NUM       (64)
#define TAF_CALL_MAX_REDIRECTING_NUMBER_CHARI_OCTET_NUM     (64)
#define TAF_CALL_MAX_EXT_DISPLAY_DATA_NUM                   (3)
#define TAF_CALL_MAX_EXTENDED_DISPALY_CHARI_OCTET_NUM       (64)

#define MN_CALL_EMER_CATEGORG_VAL_MAX_SUSPORT_ECALL         ( ((VOS_UINT8) MN_CALL_EMER_CATEGORG_POLICE) \
                                                            | ((VOS_UINT8) MN_CALL_EMER_CATEGORG_AMBULANCE) \
                                                            | ((VOS_UINT8) MN_CALL_EMER_CATEGORG_FIRE_BRIGADE) \
                                                            | ((VOS_UINT8) MN_CALL_EMER_CATEGORG_MARINE_GUARD) \
                                                            | ((VOS_UINT8) MN_CALL_EMER_CATEGORG_MOUNTAIN_RESCUE) \
                                                            | ((VOS_UINT8) MN_CALL_EMER_CATEGORG_MAN_INIT_ECALL) \
                                                            | ((VOS_UINT8) MN_CALL_EMER_CATEGORG_AUTO_INIT_ECALL) )


#define MN_CALL_EMER_CATEGORG_VAL_MAX_NOT_SUSPORT_ECALL     ( ((VOS_UINT8) MN_CALL_EMER_CATEGORG_POLICE) \
                                                            | ((VOS_UINT8) MN_CALL_EMER_CATEGORG_AMBULANCE) \
                                                            | ((VOS_UINT8) MN_CALL_EMER_CATEGORG_FIRE_BRIGADE) \
                                                            | ((VOS_UINT8) MN_CALL_EMER_CATEGORG_MARINE_GUARD) \
                                                            | ((VOS_UINT8) MN_CALL_EMER_CATEGORG_MOUNTAIN_RESCUE) )

#define TAF_CALL_APP_EC_RANDOM_NUM                          (32)
#define TAF_CALL_APP_EC_RANDOM_DATA_LENGTH                  (33)
#define TAF_CALL_ECC_KMC_PUBLIC_KEY_MAX_HEX_LENGTH          (48)
#define TAF_CALL_ECC_KMC_PUBLIC_KEY_MAX_ASCII_LENGTH        (96)
#define TAF_CALL_APP_ENCRYPTED_VOICE_DATA_MAX_LENGTH        (1024)
#define TAF_CALL_APP_EC_RANDOM_MAX_GROUP                    (8)


#define TAF_CALL_EMC_CATEGORY_MAX_VALID_BIT_CNT             (7)

#define   MN_CALL_FILL_MSG_HEADER(pSendMsg, RecvPid)    \
        (pSendMsg)->ulSenderCpuId   = VOS_LOCAL_CPUID; \
        (pSendMsg)->ulSenderPid     = WUEPS_PID_TAF; \
        (pSendMsg)->ulReceiverCpuId = VOS_LOCAL_CPUID; \
        (pSendMsg)->ulReceiverPid   = RecvPid;

#define TAF_CALL_CNAP_NAME_STR_MAX_LENGTH                   (161)               /* CNAP主叫名称字段最大长度 */

#define TAF_APDS_CALL_PULL_DIALOG_ID                        (255)

/*****************************************************************************
  3类型定义
*****************************************************************************/


/*****************************************************************************
 枚举名    : NAS_CC_STATE_INFO_STRU
 结构说明  : CC内部状态结构
*****************************************************************************/
enum NAS_CC_CALL_STATE_ENUM                                                     /* Call state value (octet 2)               */
{                                                                               /* Bits 6 5 4 3 2 1                         */
    NAS_CC_CALL_STATE_U0                                    =  0,               /*      0 0 0 0 0 0 UO - null               */
    NAS_CC_CALL_STATE_U0_1                                  =  2,               /*      0 0 0 0 1 0 U0.1- MM con pending    */
    NAS_CC_CALL_STATE_U0_2                                  = 34,               /*      1 0 0 0 1 0 U0.2- CC prompt present */
    NAS_CC_CALL_STATE_U0_3                                  = 35,               /*      1 0 0 0 1 1 U0.3- Wait for network  */
    NAS_CC_CALL_STATE_U0_4                                  = 36,               /*      1 0 0 1 0 0 U0.4- CC-Est present    */
    NAS_CC_CALL_STATE_U0_5                                  = 37,               /*      1 0 0 1 0 1 U0.5- CC-Est confirmed  */
    NAS_CC_CALL_STATE_U0_6                                  = 38,               /*      1 0 0 1 1 0 U0.6- Recall present    */
    NAS_CC_CALL_STATE_U1                                    =  1,               /*      0 0 0 0 0 1 U1 - call initiated     */
    NAS_CC_CALL_STATE_U3                                    =  3,               /*      0 0 0 0 1 1 U3 - MO call proc       */
    NAS_CC_CALL_STATE_U4                                    =  4,               /*      0 0 0 1 0 0 U4 - call delivered     */
    NAS_CC_CALL_STATE_U6                                    =  6,               /*      0 0 0 1 1 0 U6 - call present       */
    NAS_CC_CALL_STATE_U7                                    =  7,               /*      0 0 0 1 1 1 U7 - call received      */
    NAS_CC_CALL_STATE_U8                                    =  8,               /*      0 0 1 0 0 0 U8 - connect request    */
    NAS_CC_CALL_STATE_U9                                    =  9,               /*      0 0 1 0 0 1 U9 - MT call Cnf        */
    NAS_CC_CALL_STATE_U10                                   = 10,               /*      0 0 1 0 1 0 U10- active             */
    NAS_CC_CALL_STATE_U11                                   = 11,               /*      0 0 1 0 1 1 U11- disconnect REQ     */
    NAS_CC_CALL_STATE_U12                                   = 12,               /*      0 0 1 1 0 0 U12- disconnect IND     */
    NAS_CC_CALL_STATE_U19                                   = 19,               /*      0 1 0 0 1 1 U19- release REQ        */
    NAS_CC_CALL_STATE_U26                                   = 26,               /*      0 1 1 0 1 0 U26- MO modify          */
    NAS_CC_CALL_STATE_U27                                   = 27,               /*      0 1 1 0 1 1 U27- MT modify          */
    NAS_CC_CALL_STATE_BUTT
};
typedef VOS_UINT8   NAS_CC_CALL_STATE_ENUM_U8;


enum NAS_NW_CC_STATE_ENUM                                                     /* Call state value (octet 2)               */
{                                                                               /* Bits 6 5 4 3 2 1                         */
    NAS_NW_CC_CALL_STATE_N0                                    =  0,            /*      0 0 0 0 0 0 NO - null               */
    NAS_NW_CC_CALL_STATE_N0_1                                  =  2,            /*      0 0 0 0 1 0 N0.1- MM con pending    */
    NAS_NW_CC_CALL_STATE_N0_2                                  = 34,            /*      1 0 0 0 1 0 N0.2- CC con pending */
    NAS_NW_CC_CALL_STATE_N0_3                                  = 35,            /*      1 0 0 0 1 1 N0.3- network answer pending  */
    NAS_NW_CC_CALL_STATE_N0_4                                  = 36,            /*      1 0 0 1 0 0 N0.4- CC-Est present    */
    NAS_NW_CC_CALL_STATE_N0_5                                  = 37,            /*      1 0 0 1 0 1 N0.5- CC-Est confirmed  */
    NAS_NW_CC_CALL_STATE_N0_6                                  = 38,            /*      1 0 0 1 1 0 N0.6- Recall present    */
    NAS_NW_CC_CALL_STATE_N1                                    =  1,            /*      0 0 0 0 0 1 N1 - call initiated     */
    NAS_NW_CC_CALL_STATE_N3                                    =  3,            /*      0 0 0 0 1 1 N3 - MO call proc       */
    NAS_NW_CC_CALL_STATE_N4                                    =  4,            /*      0 0 0 1 0 0 N4 - call delivered     */
    NAS_NW_CC_CALL_STATE_N6                                    =  6,            /*      0 0 0 1 1 0 N6 - call present       */
    NAS_NW_CC_CALL_STATE_N7                                    =  7,            /*      0 0 0 1 1 1 N7 - call received      */
    NAS_NW_CC_CALL_STATE_N8                                    =  8,            /*      0 0 1 0 0 0 N8 - connect request    */
    NAS_NW_CC_CALL_STATE_N9                                    =  9,            /*      0 0 1 0 0 1 N9 - MT call Cnf        */
    NAS_NW_CC_CALL_STATE_N10                                   = 10,            /*      0 0 1 0 1 0 N10- active             */
    NAS_NW_CC_CALL_STATE_N12                                   = 12,            /*      0 0 1 1 0 0 N12- disconnect IND     */
    NAS_NW_CC_CALL_STATE_N19                                   = 19,            /*      0 1 0 0 1 1 N19- release REQ        */
    NAS_NW_CC_CALL_STATE_N26                                   = 26,            /*      0 1 1 0 1 0 N26- MO modify          */
    NAS_NW_CC_CALL_STATE_N27                                   = 27,            /*      0 1 1 0 1 1 N27- MT modify          */
    NAS_NW_CC_CALL_STATE_N28                                   = 28,            /*      0 1 1 0 1 1 N28- connect IND        */
    NAS_NW_CC_CALL_STATE_BUTT
};
typedef VOS_UINT8   NAS_NW_CC_STATE_ENUM_U8;



/* Table 10.5.100/3GPP TS 24.008: Auxiliary states information element        */
enum NAS_CC_HOLD_AUX_STATE_ENUM                                                 /* Hold aux state (octet 3)          */
{                                                                               /* Bits 4 3                          */
    NAS_CC_HOLD_AUX_S_IDLE                                  = 0,                /*      0 0 idle                     */
    NAS_CC_HOLD_AUX_S_HOLD_REQ                              = 1,                /*      0 1 hold request             */
    NAS_CC_HOLD_AUX_S_CALL_HELD                             = 2,                /*      1 0 call held                */
    NAS_CC_HOLD_AUX_S_RETRIEVE_REQ                          = 3,                /*      1 1 retrieve request         */
    NAS_CC_HOLD_AUX_S_BUTT
};                                                                              /* defined in 24.083 [27].           */
typedef VOS_UINT8   NAS_CC_HOLD_AUX_STATE_ENUM_U8;

/* Table 10.5.101/3GPP TS 24.008: Auxiliary states information element        */
enum NAS_CC_MPTY_AUX_STATE_ENUM                                                 /* Multi party aux state (octet 3)   */
{                                                                               /* Bits 2 1                          */
    NAS_CC_MPTY_AUX_S_IDLE                                  = 0,                /*      0 0 idle                     */
    NAS_CC_MPTY_AUX_S_MPTY_REQ                              = 1,                /*      0 1 MPTY request             */
    NAS_CC_MPTY_AUX_S_CALL_IN_MPTY                          = 2,                /*      1 0 call in MPTY             */
    NAS_CC_MPTY_AUX_S_SPLIT_REQ                             = 3,                /*      1 1 split request            */
    NAS_CC_MPTY_AUX_S_BUTT
};                                                                              /* defined in 24.084 [28].           */
typedef VOS_UINT8   NAS_CC_MPTY_AUX_STATE_ENUM_U8;



enum NAS_CC_CALL_TYPE
{
    NAS_CC_CALL_TYPE_MO_NORMAL_CALL,                    /* 主叫正常呼 */
    NAS_CC_CALL_TYPE_EMERGENCY_CALL,                    /* 主叫紧急呼 */
    NAS_CC_CALL_TYPE_MT_NORMAL_CALL,                    /* 被叫普通呼 */
    NAS_CC_CALL_TYPE_BUTT
};
typedef VOS_UINT8  NAS_CC_CALL_TYPE_ENUM_U8;






typedef struct
{
    VOS_UINT8                           ucCallId;           /* 呼叫ID */
    VOS_UINT8                           ucTi;               /* 事务ID */
    NAS_CC_CALL_STATE_ENUM_U8           enCallState;        /* 呼叫协议状态 */
    NAS_CC_HOLD_AUX_STATE_ENUM_U8       enHoldAuxState;     /* 呼叫保持辅助状态 */
    NAS_CC_MPTY_AUX_STATE_ENUM_U8       enMptyAuxState;     /* 多方呼叫辅助状态 */
    VOS_UINT8                           aucReserve1[3];
} NAS_CC_STATE_INFO_STRU;

/*****************************************************************************
 枚举名    : AT_CC_STATE_QRY_CNF_STRU
 结构说明  : CC回复AT状态查询回复接口
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;                              /* 消息名 */
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_UINT8                           ucCallNum;                              /* 呼叫个数 */
    VOS_UINT8                           aucCallExist[MN_CALL_MAX_NUM];
    NAS_CC_STATE_INFO_STRU              astCcStateInfoList[MN_CALL_MAX_NUM];                          /* 消息内容 */
} AT_CC_STATE_QRY_CNF_MSG_STRU;




enum TAF_CALL_SUB_STATE_ENUM
{
    TAF_CALL_SUB_STATE_NULL,
    TAF_CALL_SUB_STATE_INCOMING_WAIT_CONNECT_ACK,                                    /* ata接听后上报ok，等网络connect ack子状态*/
    TAF_CALL_SUB_STATE_DISCONNECT_WAIT_EVT_REL,
    TAF_CALL_SUB_STATE_BUTT
};
typedef VOS_UINT8 TAF_CALL_SUB_STATE_ENUM_UINT8;



/* Call Control Protocol Cause Value */
enum MN_CALL_CC_CAUSE_ENUM
{
    MN_CALL_INVALID_CAUSE                                   = 0,                /*   0. invalid cause value*/
    MN_CALL_UNASSIGNED_CAUSE                                = 1,                /*   1. Unassigned (unallocated) number            */
    MN_CALL_NO_ROUTE_TO_DEST                                = 3,                /*   3. No route to destination                    */
    MN_CALL_CHANNEL_UNACCEPTABLE                            = 6,                /*   6. Channel unacceptable                       */
    MN_CALL_OPERATOR_DETERMINED_BARRING                     = 8,                /*   8. Operator determined barring                */
    MN_CALL_NORMAL_CALL_CLEARING                            = 16,               /*  16. Normal call clearing                       */
    MN_CALL_USER_BUSY                                       = 17,               /*  17. User busy                                  */
    MN_CALL_NO_USER_RESPONDING                              = 18,               /*  18. No user responding                         */
    MN_CALL_USER_ALERTING_NO_ANSWER                         = 19,               /*  19. User alerting, no answer                   */
    MN_CALL_CALL_REJECTED                                   = 21,               /*  21. Call rejected                              */
    MN_CALL_NUMBER_CHANGED                                  = 22,               /*  22. Number changed                             */
    MN_CALL_CALL_REJECTED_DUE_TO_FEATURE_AT_THE_DESTINATION = 24,               /*  24. Call rejected due to feature at the destination */
    MN_CALL_PRE_EMPTION                                     = 25,               /*  25. Pre-emption                                */
    MN_CALL_NON_SELECTED_USER_CLEARING                      = 26,               /*  26. Non selected user clearing                 */
    MN_CALL_DESTINATION_OUT_OF_ORDER                        = 27,               /*  27. Destination out of order                   */
    MN_CALL_INVALID_NUMBER_FORMAT                           = 28,               /*  28. Invalid number format                      */
    MN_CALL_FACILITY_REJECTED                               = 29,               /*  29. Facility rejected                          */
    MN_CALL_RESPONSE_TO_STATUS_ENQUIRY                      = 30,               /*  30. Response to STATUS ENQUIRY                 */
    MN_CALL_NORMAL_UNSPECIFIED                              = 31,               /*  31. Normal, unspecified                        */
    MN_CALL_NO_CIRCUIT_CHANNEL_AVAILABLE                    = 34,               /*  34. No circuit/channel available               */
    MN_CALL_NETWORK_OUT_OF_ORDER                            = 38,               /*  38. Network out of order                       */
    MN_CALL_TEMPORARY_FAILURE                               = 41,               /*  41. Temporary failure                          */
    MN_CALL_SWITCHING_EQUIPMENT_CONGESTION                  = 42,               /*  42. Switching equipment congestion             */
    MN_CALL_ACCESS_INFORMATION_DISCARDED                    = 43,               /*  43. Access information discarded               */
    MN_CALL_REQUESTED_CIRCUIT_CHANNEL_NOT_AVAILABLE         = 44,               /*  44. requested circuit/channel not available    */
    MN_CALL_RESOURCES_UNAVAILABLE_UNSPECIFIED               = 47,               /*  47. Resources unavailable, unspecified         */
    MN_CALL_QUALITY_OF_SERVICE_UNAVAILABLE                  = 49,               /*  49. Quality of service unavailable             */
    MN_CALL_REQUESTED_FACILITY_NOT_SUBSCRIBED               = 50,               /*  50. Requested facility not subscribed          */
    MN_CALL_INCOMING_CALL_BARRED_WITHIN_CUG                 = 55,               /*  55. Incoming calls barred within the CUG       */
    MN_CALL_BEARER_CAPABILITY_NOT_AUTHORISED                = 57,               /*  57. Bearer capability not authorized           */
    MN_CALL_BEARER_CAPABILITY_NOT_PRESENTLY_AVAILABLE       = 58,               /*  58. Bearer capability not presently available  */
    MN_CALL_SERVICE_OR_OPTION_NOT_AVAILABLE                 = 63,               /*  63. Service or option not available, unspec    */
    MN_CALL_BEARER_SERVICE_NOT_IMPLEMENTED                  = 65,               /*  65. Bearer service not implemented             */
    MN_CALL_ACM_GEQ_ACMMAX                                  = 68,               /*  68. ACM equal to or greater than ACMmax        */
    MN_CALL_REQUESTED_FACILITY_NOT_IMPLEMENTED              = 69,               /*  69. Requested facility not implemented         */
    MN_CALL_ONLY_RESTRICTED_DIGITAL_INFO_BC_AVAILABLE       = 70,               /*  70. Only restricted digital info BC available  */
    MN_CALL_SERVICE_OR_OPTION_NOT_IMPLEMENTED               = 79,               /*  79. Service or option not implemented, unspec  */
    MN_CALL_INVALID_TRANSACTION_ID_VALUE                    = 81,               /*  81. Invalid transaction identifier value       */
    MN_CALL_USER_NOT_MEMBER_OF_CUG                          = 87,               /*  87. User not member of CUG                     */
    MN_CALL_INCOMPATIBLE_DESTINATION                        = 88,               /*  88. Incompatible destination Incompatible para */
    MN_CALL_INVALID_TRANSIT_NETWORK_SELECTION               = 91,               /*  91. Invalid transit network selection          */
    MN_CALL_SEMANTICALLY_INCORRECT_MESSAGE                  = 95,               /*  95. Semantically incorrect message             */
    MN_CALL_INVALID_MANDATORY_INFORMATION                   = 96,               /*  96. Invalid mandatory information              */
    MN_CALL_MESSAGE_TYPE_NON_EXISTENT                       = 97,               /*  97. Msg type non-existent or not implemented   */
    MN_CALL_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROT_STATE     = 98,               /*  98. Msg type not compatible with protocol state*/
    MN_CALL_IE_NON_EXISTENT_OR_NOT_IMPLEMENTED              = 99,               /*  99. IE non-existent or not implemented         */
    MN_CALL_CONDITIONAL_IE_ERROR                            = 100,              /* 100. Conditional IE error                       */
    MN_CALL_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE      = 101,              /* 101. Message not compatible with protocol state */
    MN_CALL_RECOVERY_ON_TIMER_EXPIRY                        = 102,              /* 102. Recovery on timer expiry Timer number      */
    MN_CALL_PROTOCOL_ERROR_UNSPECIFIED                      = 111,              /* 111. Protocol error, unspecified                */
    MN_CALL_INTERWORKING_UNSPECIFIED                        = 127,               /* 127. Interworking, unspecified                  */
    MN_CALL_CC_CAUSE_BUTT
};
typedef VOS_UINT8   MN_CALL_CC_CAUSE_ENUM_U8;

#define TAF_ECC_CALL_MAX_BCD_NUM_LEN         6
typedef struct
{
    MN_CALL_NUM_TYPE_ENUM_U8            enNumType;
    VOS_UINT8                           ucNumLen;
    VOS_UINT8                           aucBcdNum[TAF_ECC_CALL_MAX_BCD_NUM_LEN];
} TAF_ECC_CALL_BCD_NUM_STRU;


/* 设置CSSN的类型,激活或去激活 */
enum MN_CALL_SET_CSSN_TYPE_ENUM
{
    MN_CALL_SET_CSSN_DEACT,
    MN_CALL_SET_CSSN_ACT,
    MN_CALL_SET_CSSN_BUTT
};
typedef VOS_UINT32 MN_CALL_SET_CSSN_TYPE_ENUM_U32;

enum MN_CALL_CUUS1_FLG_ENUM
{
    MN_CALL_CUUS1_DISABLE,
    MN_CALL_CUUS1_ENABLE,
    MN_CALL_CUUS1_BUTT
};
typedef VOS_UINT32 MN_CALL_CUUS1_FLG_ENUM_U32;


/* 设置UUS1的类型,激活或去激活 */
enum MN_CALL_SET_UUS1_TYPE_ENUM
{
    MN_CALL_SET_UUS1_ACT,
    MN_CALL_SET_UUS1_DEACT,
    MN_CALL_SET_UUS1_BUTT
};
typedef VOS_UINT32 MN_CALL_SET_UUS1_TYPE_ENUM_U32;

/* 设置或上报UUS1的消息类型,包含下列几种消息,如果设置ANY,则默认为
设置SETUP,PROGRESS消息只能上报,不能设置 */
enum MN_CALL_UUS1_MSG_TYPE_ENUM
{
    MN_CALL_UUS1_MSG_ANY,
    MN_CALL_UUS1_MSG_SETUP,
    MN_CALL_UUS1_MSG_ALERT,
    MN_CALL_UUS1_MSG_CONNECT,
    MN_CALL_UUS1_MSG_DISCONNECT,
    MN_CALL_UUS1_MSG_RELEASE,
    MN_CALL_UUS1_MSG_RELEASE_COMPLETE,
    MN_CALL_UUS1_MSG_PROGRESS,
    MN_CALL_UUS1_MSG_BUTT
};
typedef VOS_UINT32 MN_CALL_UUS1_MSG_TYPE_ENUM_U32;

#define MN_CALL_MAX_UUS1_MSG_NUM        (7)
#define MN_CALL_MAX_CSSN_MSG_NUM        (2)

/*The user-user is a type 4 information element with a minimum length of 3 octets
and a maximum length of either 35 or 131 octets. In the SETUP message the
user-user information element has a maximum size of 35 octets in a
GSM PLMN. In the USER INFORMATION, ALERTING, CONNECT, DISCONNECT,
PROGRESS, RELEASE and RELEASE COMPLETE messages the user-user information
element has a maximum size of 131 octets in a GSM PLMN.*/

#define MN_CALL_MIN_UUIE_LEN                    (3)
#define MN_CALL_MAX_UUIE_LEN                    (131)
#define MN_CALL_MAX_SETUP_UUIE_CONTEND_LEN      (33)
#define MN_CALL_MAX_OTHER_UUIE_CONTEND_LEN      (129)
#define MN_CALL_MIN_UUIE_CONTEND_LEN            (1)
#define MN_CALL_UUIE_HEADER_LEN                 (2)

/* 此处的UUIE内容是协议中规定的UUIE内容,具体内容如下 */
/********************************************************
  |  8 |  7  | 6 |  5 |  4 |  3 |  2 |  1   |
  |    |     User-user IEI                  | octet 1
  |         Length of user-user contents    | octet 2                           |
  |      User-user protocol discriminator   | octet 3
  |     User-user information               | octet 4*
  |       ...............                   |
  |       ...............                   | octet N*
*********************************************************/
#define MN_CALL_UUS_IEI                         (0x7E)
#define MN_CALL_IEI_POS                         (0x00)
#define MN_CALL_LEN_POS                         (0x01)
#define MN_CALL_PD_POS                          (0x02)
#define MN_CALL_CONTENT_POS                     (0x03)

typedef struct
{
    MN_CALL_UUS1_MSG_TYPE_ENUM_U32      enMsgType;
    VOS_UINT8                           aucUuie[MN_CALL_MAX_UUIE_LEN];
    VOS_UINT8                           aucReserved1;
}MN_CALL_UUS1_INFO_STRU;


/* Call Event */
enum MN_CALL_EVENT_ENUM
{
    MN_CALL_EVT_ORIG                = 0x00,                                     /* originate a MO Call */
    MN_CALL_EVT_CALL_PROC           = 0x01,                                     /* Call is Proceeding */
    MN_CALL_EVT_ALERTING            = 0x02,                                     /* Alerting,MO Call */
    MN_CALL_EVT_CONNECT             = 0x03,                                     /* Call Connect */
    MN_CALL_EVT_RELEASED            = 0x04,                                     /* Call Released */
    MN_CALL_EVT_INCOMING            = 0x05,                                     /* Incoming Call */
    MN_CALL_EVT_SS_CMD_PROGRESS     = 0x06,                                     /* Call Related Command is in progress */
    MN_CALL_EVT_SS_CMD_RSLT         = 0x07,                                     /* Call Related Command is Completed */
    MN_CALL_EVT_SS_NOTIFY           = 0x08,                                     /* SS Notify */
    MN_CALL_EVT_START_DTMF_CNF      = 0x09,                                     /* Start DTMF tempooralily response */
    MN_CALL_EVT_STOP_DTMF_CNF       = 0x0b,                                     /* Stop DTMF tempooralily response */
    MN_CALL_EVT_CCBS_POSSIBLE       = 0x0d,                                     /* 可以激活CCBS*/
    MN_CALL_EVT_TIME_EXPIRED        = 0x0e,                                     /*超时*/
    MN_CALL_EVT_UUS1_INFO           = 0x0f,                                     /* UUS1信息上报 */
    MN_CALL_EVT_GET_CDUR_CNF        = 0x10,                                     /* 通话时长 */
    MN_CALL_EVT_CLCC_INFO           = 0x11,                                     /* 当前所有呼叫的信息 */
    MN_CALL_EVT_ALL_RELEASED        = 0x12,                                     /* Call All Released */
    MN_CALL_EVT_SET_UUS1_INFO_CNF   = 0x13,                                     /* 设置用户服务信令1信息 */
    MN_CALL_EVT_QRY_UUS1_INFO_CNF   = 0x14,                                     /* 查询用户服务信令1信息 */
    MN_CALL_EVT_SET_ALS_CNF         = 0x15,                                     /* 设置当前线路号 */
    MN_CALL_EVT_QRY_ALS_CNF         = 0x16,                                     /* 查询当前线路号 */

    MN_CALL_EVT_ECC_NUM_IND         = 0x17,

    MN_CALL_EVT_HOLD                = 0x18,                                     /* Call Hold 呼叫保持 */
    MN_CALL_EVT_RETRIEVE            = 0x19,                                     /* Call Retrieve 呼叫恢复 */

    MN_CALL_EVT_CLPR_SET_CNF        = 0x1a,


    MN_CALL_EVT_XLEMA_CNF           = 0x1d,

    MN_CALL_EVT_SET_CSSN_CNF        = 0x1e,

    MN_CALL_EVT_CALL_ORIG_CNF       = 0x1f,
    MN_CALL_EVT_SUPS_CMD_CNF        = 0x20,

    MN_CALL_EVT_START_DTMF_RSLT     = 0x22,
    MN_CALL_EVT_STOP_DTMF_RSLT      = 0x23,

    MN_CALL_EVT_QRY_ECALL_INFO_CNF  = 0x24,

    MN_CALL_EVT_CALL_MODIFY_CNF     = 0x25,
    MN_CALL_EVT_CALL_ANSWER_REMOTE_MODIFY_CNF   = 0x26,
    MN_CALL_EVT_CALL_MODIFY_STATUS_IND          = 0x27,

    TAF_CALL_EVT_SEND_FLASH_RSLT,                           /* _H2ASN_MsgChoice TAF_CALL_EVT_SEND_FLASH_RSLT_STRU */
    TAF_CALL_EVT_CALLED_NUM_INFO_IND,                       /* _H2ASN_MsgChoice TAF_CALL_EVT_CALLED_NUM_INFO_IND_STRU */
    TAF_CALL_EVT_CALLING_NUM_INFO_IND,                      /* _H2ASN_MsgChoice TAF_CALL_EVT_CALLING_NUM_INFO_IND_STRU */
    TAF_CALL_EVT_DISPLAY_INFO_IND,                          /* _H2ASN_MsgChoice TAF_CALL_EVT_DISPLAY_INFO_IND_STRU */
    TAF_CALL_EVT_EXT_DISPLAY_INFO_IND,                      /* _H2ASN_MsgChoice TAF_CALL_EVT_EXT_DISPLAY_INFO_IND_STRU */
    TAF_CALL_EVT_CONN_NUM_INFO_IND,                         /* _H2ASN_MsgChoice TAF_CALL_EVT_CONN_NUM_INFO_IND_STRU */
    TAF_CALL_EVT_REDIR_NUM_INFO_IND,                        /* _H2ASN_MsgChoice TAF_CALL_EVT_REDIR_NUM_INFO_IND_STRU */
    TAF_CALL_EVT_SIGNAL_INFO_IND,                           /* _H2ASN_MsgChoice TAF_CALL_EVT_SIGNAL_INFO_IND_STRU */
    TAF_CALL_EVT_LINE_CTRL_INFO_IND,                        /* _H2ASN_MsgChoice TAF_CALL_EVT_LINE_CTRL_INFO_IND_STRU */
    TAF_CALL_EVT_CALL_WAITING_IND,                          /* _H2ASN_MsgChoice TAF_CALL_EVT_CALL_WAITING_IND_STRU */
    TAF_CALL_EVT_ECONF_DIAL_CNF,
    TAF_CALL_EVT_ECONF_NOTIFY_IND,                               /* imsa通知SPM模块增强型多方通话参与者的状态发送变化  */
    TAF_CALL_EVT_CLCCECONF_INFO,

    TAF_CALL_EVT_SEND_BURST_DTMF_CNF,                            /* _H2ASN_MsgChoice TAF_CALL_EVT_SEND_BURST_DTMF_CNF_STRU */
    TAF_CALL_EVT_SEND_BURST_DTMF_RSLT,                           /* _H2ASN_MsgChoice TAF_CALL_EVT_SEND_BURST_DTMF_RSLT_STRU */

    TAF_CALL_EVT_CCWAC_INFO_IND,                            /* _H2ASN_MsgChoice TAF_CALL_EVT_CCWAC_INFO_IND_STRU */

    TAF_CALL_EVT_SEND_CONT_DTMF_CNF,                            /* _H2ASN_MsgChoice TAF_CALL_EVT_SEND_CONT_DTMF_CNF_STRU */
    TAF_CALL_EVT_SEND_CONT_DTMF_RSLT,                           /* _H2ASN_MsgChoice TAF_CALL_EVT_SEND_CONT_DTMF_RSLT_STRU */
    TAF_CALL_EVT_RCV_CONT_DTMF_IND,                             /* _H2ASN_MsgChoice TAF_CALL_EVT_RCV_CONT_DTMF_IND_STRU */
    TAF_CALL_EVT_RCV_BURST_DTMF_IND,                            /* _H2ASN_MsgChoice TAF_CALL_EVT_RCV_BURST_DTMF_IND_STRU */

    TAF_CALL_EVT_CCLPR_SET_CNF,                                /* _H2ASN_MsgChoice TAF_CALL_EVT_CCLPR_GET_CNF_STRU */

    TAF_CALL_EVT_CCWAI_SET_CNF,                                 /* _H2ASN_MsgChoice TAF_CALL_EVT_CCWAI_SET_CNF */

    TAF_CALL_EVT_CHANNEL_QRY_CNF,
    MN_CALL_EVT_BUTT
};
typedef  VOS_UINT32  MN_CALL_EVENT_ENUM_U32;


/* [10.5.4.8] Called party subaddress */
/*============================================================================*/
/* Table 10.5.119/3GPP TS 24.008: Called party subaddress */
enum MN_CALL_TYPE_OF_SUBADDR_ENUM                                                                   /* Type of subaddress (octet 3)      */
{                                                                               /* Bits 7 6 5                        */
    MN_CALL_SUBADDR_NSAP                = 0,                                    /*      0 0 0 NSAP                   */
    MN_CALL_USER_SPEC                   = 2                                     /*      0 1 0 User specified         */
};                                                  /* All other values are reserved     */
typedef VOS_UINT8   MN_CALL_TYPE_OF_SUBADDR_ENUM_UINT8;

/* 移枚举定义到tafAppCall.h文件中 */

enum TAF_CALL_DIGIT_MODE_ENUM
{
    TAF_CALL_DIGIT_MODE_DTMF            = 0x00,
    TAF_CALL_DIGIT_MODE_ASCII           = 0x01,
    TAF_CALL_DIGIT_MODE_BUTT            = 0x02
};
typedef VOS_UINT8 TAF_CALL_DIGIT_MODE_ENUM_UINT8;


enum TAF_CALL_NUMBER_PLAN_ENUM
{
    TAF_CALL_NUMBER_PLAN_UNKNOW         = 0x00,
    TAF_CALL_NUMBER_PLAN_ISDN           = 0x01,
    TAF_CALL_NUMBER_PLAN_DATA           = 0x03,
    TAF_CALL_NUMBER_PLAN_TELEX          = 0x04,
    TAF_CALL_NUMBER_PLAN_PRIVATE        = 0x09,
    TAF_CALL_NUMBER_PLAN_BUTT           = 0x0A
};
typedef VOS_UINT8 TAF_CALL_NUMBER_PLAN_ENUM_UINT8;


enum TAF_CALL_NUMBER_TYPE_ENUM
{
    TAF_CALL_NUMBER_TYPE_UNKNOW                             = 0x00,
    TAF_CALL_NUMBER_TYPE_INTERNATIONAL                      = 0x01,
    TAF_CALL_NUMBER_TYPE_NATIONAL                           = 0x02,
    TAF_CALL_NUMBER_TYPE_NETWORK_SPEC                       = 0x03,
    TAF_CALL_NUMBER_TYPE_SUBSCRIBER                         = 0x04,
    TAF_CALL_NUMBER_TYPE_RESERVED                           = 0x05,
    TAF_CALL_NUMBER_TYPE_ABBREVIATED                        = 0x06,
    TAF_CALL_NUMBER_TYPE_RESERVED_FOR_EXT                   = 0x07,
    TAF_CALL_NUMBER_TYPE_BUTT                               = 0x08
};
typedef VOS_UINT8 TAF_CALL_NUMBER_TYPE_ENUM_UINT8;




enum TAF_CALL_SEND_BURST_DTMF_CNF_RESULT_ENUM
{
    TAF_CALL_SEND_BURST_DTMF_CNF_RESULT_SUCCESS         = 0,
    TAF_CALL_SEND_BURST_DTMF_CNF_RESULT_ABNORMAL_STATE,
    TAF_CALL_SEND_BURST_DTMF_CNF_RESULT_BUFFER_FULL,
    TAF_CALL_SEND_BURST_DTMF_CNF_RESULT_ALLOC_MSG_FAIL,
    TAF_CALL_SEND_BURST_DTMF_CNF_RESULT_ALLOC_MEM_FAIL,

    TAF_CALL_SEND_BURST_DTMF_CNF_RESULT_BUTT
};
typedef VOS_UINT8   TAF_CALL_SEND_BURST_DTMF_CNF_RESULT_ENUM_U8;


enum TAF_CALL_CNI_VALIDITY_ENUM
{
    TAF_CALL_CNI_VALIDITY_VALID             = 0,                                /* CNI Valid */
    TAF_CALL_CNI_VALIDITY_WITHHELD,                                             /* CNI has been withheld by the originator */
    TAF_CALL_CNI_VALIDITY_NOT_AVAILABLE,                                        /* CNI is not available due to interworking problems or limitations of originating network */
    TAF_CALL_CNI_VALIDITY_BUTT
};
typedef VOS_UINT8 TAF_CALL_CNI_VALIDITY_ENUM_UINT8;


enum TAF_CALL_DCS_ENUM
{
    TAF_CALL_DCS_7BIT                   = 0,                                    /* 7 bit */
    TAF_CALL_DCS_8BIT,                                                          /* 8 bit */
    TAF_CALL_DCS_UCS2,                                                          /* UCS-2 */
    TAF_CALL_DCS_BUTT
};
typedef VOS_UINT8 TAF_CALL_DCS_ENUM_UINT8;



enum TAF_CS_CALL_DIR_ENUM
{
    TAF_CS_CALL_DIR_NON           = 0,        /* 从CS域存在呼叫到CS域不存在呼叫时，呼叫方向使用该标志 */
    TAF_CS_CALL_DIR_MO            = 1,        /* 从CS域不存在呼叫到CS域存在呼叫时，且第一通呼叫属于主叫时，使用该标志  */
    TAF_CS_CALL_DIR_MT            = 2,        /* 从CS域不存在呼叫到CS域存在呼叫时，且第一通呼叫属于被叫时，使用该标志  */
    TAF_CS_CALL_DIR_BUTT
};
typedef VOS_UINT8  TAF_CS_CALL_DIR_ENUM_UINT8;


typedef struct
{
    VOS_UINT8                           ucExistFlag;                            /* 网络是否携带CNAP信息标志 */
    TAF_CALL_CNI_VALIDITY_ENUM_UINT8    enCniValidity;                          /* <CNI_Validity>参数 */
    TAF_CALL_DCS_ENUM_UINT8             enDcs;                                  /* <name>参数的Data Coding Scheme */
    VOS_UINT8                           ucLength;
    VOS_UINT8                           aucNameStr[TAF_CALL_CNAP_NAME_STR_MAX_LENGTH];
    VOS_UINT8                           aucReserved[3];
} TAF_CALL_CNAP_STRU;



typedef struct
{
    MN_CLIENT_ID_T                              usClientId;
    MN_OPERATION_ID_T                           ucOpId;
    MN_CALL_ID_T                                ucCallId;
    TAF_CALL_SEND_BURST_DTMF_CNF_RESULT_ENUM_U8 enResult;
    VOS_UINT8                                   aucReserved[3];
}TAF_CALL_EVT_SEND_BURST_DTMF_CNF_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucResult;
}TAF_CALL_EVT_SEND_BURST_DTMF_RSLT_STRU;

/* Call Info Structure */
typedef struct
{
    MN_CLIENT_ID_T                      clientId;                               /* Client ID */
    MN_OPERATION_ID_T                   opId;                                   /* op ID */
    MN_CALL_ID_T                        callId;                                 /* Call ID */
    MN_CALL_STATE_ENUM_U8               enCallState;                            /* Call State */
    MN_CALL_MPTY_STATE_ENUM_U8          enMptyState;                            /* MPTY State */
    MN_CALL_TYPE_ENUM_U8                enCallType;                             /* Call Type */
    MN_CALL_MODE_ENUM_U8                enCallMode;                             /* Call Mode */
    MN_CALL_DIR_ENUM_U8                 enCallDir;                              /* Call Direction(MO/MT) */
    MN_CALL_PRESENTATION_IND_ENUM_UINT8 enCallingNumPI;
    MN_CALL_PRESENTATION_IND_ENUM_UINT8 enRedirectNumPI;
    TAF_CALL_SUB_STATE_ENUM_UINT8       enCallSubState;
    MN_CALL_CS_DATA_CFG_INFO_STRU       stDataCfgInfo;                          /* Data Configuration */
    MN_CALL_BCD_NUM_STRU                stCallNumber;                           /* Call Number */
    MN_CALL_SUBADDR_STRU                stSubCallNumber;                        /* Call SubAddress */
    MN_CALL_CALLED_NUM_STRU             stCalledNumber;                         /* Called Number */
    MN_CALL_SUBADDR_STRU                stCalledSubAddr;                        /* Called SubAddress */
    MN_CALL_BCD_NUM_STRU                stRedirectNumber;                       /* Redirect Number */
    MN_CALL_SUBADDR_STRU                stRedirectSubaddr;                      /* Redirect Call SubAddress */
    MN_CALL_BCD_NUM_STRU                stConnectNumber;                        /* Connect Number */
    MN_CALL_SUPS_CMD_ENUM_U8            enCallSupsCmd;                          /* Call Management Command */
    MN_CALL_SS_RESULT_ENUM_U8           enSsResult;                             /* the result of the last Call Management Command */
    MN_CALL_SS_ERR_CODE_ENUM_U8         enSsErrCode;                            /* SS Error Code */
    VOS_UINT8                           ucRabId;                                /* RAB ID */
    MN_CALL_NO_CLI_CAUSE_ENUM_U8        enNoCliCause;                           /* Cause of the NO CLI */
    MN_CALL_ALS_LINE_NO_ENUM_U8         enAlsLineNo;                            /* ALS Line number: 1 or 2,default is 1*/

    VOS_UINT8                           ucAtaReportOkAsyncFlag;                 /* VOS_TRUE:ata异步上报ok，发送connect后即上报ok，不等网络connect ack；
                                                                                   VOS_FALSE:ata同步上报，发送connect后等收到网络connect ack后再上报ok */

    TAF_CALL_VOICE_DOMAIN_ENUM_UINT8    enVoiceDomain;                          /* VOICE DOMAIN */
    MN_CALL_SS_NOTIFY_STRU              stSsNotify;                             /* SS Notify */
    TAF_CS_CAUSE_ENUM_UINT32            enCause;                                /* Cause Value of the Call Control */
    TAF_SS_CCBS_FEATURE_STRU            stCcbsFeature;                          /* CCBS feature info*/
    MN_CALL_UUS1_INFO_STRU              stUusInfo;
    VOS_UINT32                          ulCurCallTime;
    VOS_UINT32                          ulTotalCallTime;                        /*累计通话时间*/
    VOS_UINT32                          ulPreCallTime;                          /*上次通话时间*/
    MN_CALL_DISC_DIR                    stDiscDir;                              /* 挂断电话的方向 */
    VOS_UINT8                           aucCurcRptCfg[MN_CALL_RPT_CFG_MAX_SIZE];        /* CURC设置的主动上报标识 */
    VOS_UINT8                           aucUnsolicitedRptCfg[MN_CALL_RPT_CFG_MAX_SIZE]; /* 单个命令设置的主动上报标识 */
    VOS_UINT8                           ucEncryptFlag;                          /* 加密标记 */
    VOS_UINT8                           ucSrvccSuccFlg;                         /* 记录本次通话是否是通过SRVCC过来的 */
    MN_CALL_CF_CAUSE_ENUM_UINT8         enCallForwardCause;
    MN_CALL_HOLD_TONE_ENUM_UINT8        enHoldToneType;

    MN_CALL_DISPLAY_NAME_STRU           stDisplayName;

    TAF_CALL_EVT_SEND_BURST_DTMF_RSLT_STRU stSendBurstDTMFRslt;
    TAF_CALL_EVT_SEND_BURST_DTMF_CNF_STRU  stSendBurstDTMFCnf;

    MN_CALL_EMERGENCY_CAT_STRU          stEmergencyCat;                         /* Emergency Catory */

    TAF_CALL_CNAP_STRU                  stNameIndicator;                        /* Name Indicator */

    TAF_CALL_ERROR_INFO_TEXT_STRU       stErrInfoText;                          /* 异常信息文本 */
    VOS_UINT8                           ucStartHifiFlag;                        /* IMSA指示是否开启HIFI的标识 */
    TAF_CALL_SERVICE_TYPE_ENUM_UINT8    enServiceType;
    VOS_UINT8                           ucEconfFlag;
    VOS_UINT8                           aucReserved[1];

    TAF_CALL_ECBM_INFO_STRU             stEcbmInfo;
} MN_CALL_INFO_STRU;


typedef struct
{
    MN_CALL_TYPE_ENUM_U8                enCallType;                             /* Call Type */
    MN_CALL_CLIR_CFG_ENUM_U8            enClirCfg;                              /* CLIR Configuration */
    MN_CALL_MODE_ENUM_U8                enCallMode;                             /* Call Mode */
    VOS_UINT8                           ucCsCallRetryFlg;                       /* false: 紧急呼cs域尝试过或者cs域不可用 true: CS域没尝试过，且cs域可用*/
    MN_CALL_CALLED_NUM_STRU             stDialNumber;                           /* Call Number */
    MN_CALL_CUG_CFG_STRU                stCugCfg;                               /* CUG Configuration */
    MN_CALL_CS_DATA_CFG_STRU            stDataCfg;                              /* Data Configuration */
    MN_CALL_SUBADDR_STRU                stSubaddr;                              /* Subaddress */

    MN_CALL_EMERGENCY_CAT_STRU          stEmergencyCat;                         /* Emergency Catory */

    VOS_UINT8                           ucImsInvitePtptFlag;                    /* IMS 邀请第三方通话标记 */

    TAF_CALL_VOICE_DOMAIN_ENUM_UINT8    enVoiceDomain;                          /* VOICE DOMAIN */

    VOS_UINT8                           ucEncryptFlag;                          /* 加密标记 */

    VOS_UINT8                           ucDomainRedialFlag;                     /* 当前域呼叫失败后到其他域尝试标记 */
    VOS_UINT8                           ucHangupImsCallInCsRedialFlg;                   /* ucDomainRedialFlag为true，IMS普通呼叫收到380时，是否要挂断当前域的其他普通呼叫标志 */

    TAF_CALL_EMC_DOMAIN_ENUM_UINT8      enEmcDomain;

    TAF_CALL_SERVICE_TYPE_ENUM_UINT8    enServiceType;                          /* 业务类型 */

    VOS_UINT8                           ucRttFlg;                               /*是否发起RTT呼叫，0:非RTT呼叫 */
    VOS_UINT8                           ucCallPullFlg;                          /*1:拉取另一路呼叫，0:非拉取另一路呼叫 */
    VOS_UINT8                           ucCallPullDialogId;                     /* 0-254:拉取另一路呼叫,ucCallPullFlg=1; 255:非拉取另一路呼叫,ucCallPullFlg=0  */
    VOS_UINT8                           aucReserved[2];
} MN_CALL_ORIG_PARAM_STRU;


typedef struct
{
    TAF_CS_CAUSE_ENUM_UINT32            enEndCause;                             /* End Cause */
} MN_CALL_END_PARAM_STRU;


typedef struct
{
    VOS_UINT32                          ulReserved;
} MN_CALL_ANS_PARAM_STRU;


typedef struct
{
    MN_CALL_SUPS_CMD_ENUM_U8            enCallSupsCmd;                          /* Call Management Command */
    MN_CALL_ID_T                        callId;                                 /* Call ID */
    MN_CALL_TYPE_ENUM_U8                enCallType;                             /* 仅针对接听的那路呼叫才有意义 */
    TAF_CALL_REJ_CAUSE_ENUM_UINT8       enCallRejCause;                         /* 命令类型为MN_CALL_SUPS_CMD_REL_INCOMING_OR_WAITING时，此IE项有效 */
    MN_CALL_BCD_NUM_STRU                stRedirectNum;                          /* Redirect Number */
    MN_CALL_CALLED_NUM_STRU             stRemoveNum;                            /* 增强型会议中需要移除的与会者号码 */
    VOS_UINT8                           ucRttFlg;                               /* 0: 非RTT通话, 1: RTT通话 */
    VOS_UINT8                           aucReserved[3];                         /* 保留位 */
} MN_CALL_SUPS_PARAM_STRU;


typedef struct
{
    VOS_UINT32                          ulActNum;                               /* 需要设置的CSSN的个数 */
    MN_CALL_SET_CSSN_TYPE_ENUM_U32      aenSetType[MN_CALL_MAX_CSSN_MSG_NUM];   /* aenSetType[0]:CSSI是否需要激活，enSetType[1]:CSSU是否需要激活 */
}MN_CALL_SET_CSSN_REQ_STRU;



typedef struct
{
    MN_CALL_TYPE_ENUM_U8                enCurrCallType;                         /* 当前呼叫类型 */
    MN_CALL_TYPE_ENUM_U8                enExpectCallType;                       /* 期望呼叫类型 */
    VOS_UINT8                           aucReserved[2];
}MN_CALL_MODIFY_REQ_STRU;




typedef struct
{
    MN_CALL_CUUS1_FLG_ENUM_U32          enCuus1IFlg;                            /* CUUS1I标志 */
    MN_CALL_CUUS1_FLG_ENUM_U32          enCuus1UFlg;                            /* CUUS1U标志 */
    VOS_UINT32                          ulActNum;                             /*需要设置的UUS1的个数*/
    MN_CALL_SET_UUS1_TYPE_ENUM_U32      enSetType[MN_CALL_MAX_UUS1_MSG_NUM];  /*要设置的用户服务信令1类型*/
    MN_CALL_UUS1_INFO_STRU              stUus1Info[MN_CALL_MAX_UUS1_MSG_NUM]; /*设置的用户服务信令信息*/
}MN_CALL_UUS1_PARAM_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32  enEvent;    /*回复的事件类型*/
    MN_CLIENT_ID_T          ClientId;   /*端口号*/
    MN_OPERATION_ID_T       OpId;       /*操作ID*/
    VOS_UINT8               ucReserved; /* 保留字段 */
    VOS_UINT32              ulRet;      /*回复结果*/
}TAF_PH_SET_UUS1_INFO_CNF_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32  enEvent;                                /*回复的事件类型*/
    MN_CLIENT_ID_T          ClientId;                               /*端口号*/
    MN_OPERATION_ID_T       OpId;                                   /*操作ID*/
    VOS_UINT8               ucReserved;                             /* 保留字段 */
    VOS_UINT32              ulRet;                                  /*回复结果*/
    VOS_UINT32              ulActNum;                               /*用户服务信令1的个数*/
    MN_CALL_UUS1_INFO_STRU  stUus1Info[MN_CALL_MAX_UUS1_MSG_NUM];   /*用户服务信令1的信息*/
    MN_CALL_SET_UUS1_TYPE_ENUM_U32      aenSetType[MN_CALL_MAX_UUS1_MSG_NUM];  /*要设置的用户服务信令1类型 */
}TAF_PH_QRY_UUS1_INFO_CNF_STRU;


typedef struct
{
    MN_CALL_ALS_LINE_NO_ENUM_U8         enAlsLine;          /*要使用的线路号*/
    VOS_UINT8                           aucReserved[3];     /*保留字段*/
}MN_CALL_SET_ALS_PARAM_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32  enEvent;    /*回复的事件类型*/
    MN_CLIENT_ID_T          ClientId;   /*端口号*/
    MN_OPERATION_ID_T       OpId;       /*操作ID*/
    VOS_UINT8               ucReserved; /* 保留字段 */
    VOS_UINT32              ulRet;      /*回复结果*/
}MN_CALL_SET_ALS_CNF_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32          enEvent;                                    /*回复的事件类型*/
    MN_CLIENT_ID_T                  ClientId;                                   /*端口号*/
    MN_OPERATION_ID_T               OpId;                                       /*操作ID*/
    VOS_UINT8                       ucReserved1;                                /* 保留字段 */
    VOS_UINT32                      ulRet;                                      /*回复结果*/
    MN_CALL_ALS_LINE_NO_ENUM_U8     enAlsLine;                                  /*当前使用的线路号*/
    VOS_UINT8                       aucReserved2[3];                            /* 保留字段 */
}MN_CALL_QRY_ALS_CNF_STRU;



typedef struct
{
    VOS_UINT8                           ucIndex;                                /* 定制紧急呼号码索引 */
    VOS_UINT8                           ucTotal;                                /* 定制紧急呼号码总数 */
    VOS_UINT8                           ucCategory;                             /* 紧急呼号码类型 */
    VOS_UINT8                           ucSimPresent;                           /* 紧急呼号码是有卡时有效还是无卡时有效，1有卡时有效，0无卡时有效 */
    MN_CALL_BCD_NUM_STRU                stEccNum;                               /* 紧急呼号码 */
    VOS_UINT32                          ulMcc;                                  /* MCC,3 bytes */
} MN_CALL_ECC_LIST_INFO_STRU;

typedef TAF_CALL_ECONF_DIAL_INFO_STRU TAF_CALL_ECONF_DIAL_REQ_STRU;

typedef struct
{
    VOS_UINT32                          bitOpPeerVideoSupport       : 1;
    VOS_UINT32                          bitOpSpare                  : 31;

    MN_CALL_ID_T                        callId;                                 /* Call ID */
    MN_CALL_DIR_ENUM_U8                 enCallDir;                              /* Call Direction(MO/MT) */
    MN_CALL_STATE_ENUM_U8               enCallState;                            /* Call State */
    MN_CALL_MPTY_STATE_ENUM_U8          enMptyState;                            /* MPTY State */
    MN_CALL_TYPE_ENUM_U8                enCallType;                             /* Call Type */
    TAF_CALL_VOICE_DOMAIN_ENUM_UINT8    enVoiceDomain;                          /* VOICE DOMAIN */
    VOS_UINT8                           ucEConferenceFlag;                      /* Is Econf:true, Is not Econf:false */
    VOS_UINT8                           ucEncryptFlag;                          /* Is Encrypt:true, Is not Encrypt:false */
    MN_CALL_BCD_NUM_STRU                stCallNumber;                           /* Call Number */
    MN_CALL_CALLED_NUM_STRU             stCalledNumber;                         /* Called Number */
    MN_CALL_BCD_NUM_STRU                stConnectNumber;                        /* Connect Number */

    MN_CALL_DISPLAY_NAME_STRU           stDisplayName;

    TAF_CALL_PEER_VIDEO_SUPPORT_ENUM_UINT8  enPeerVideoSupport;
    TAF_CALL_IMS_DOMAIN_ENUM_UINT8      enImsDomain;
    VOS_UINT8                           ucReserved;
    TAF_CALL_RTT_IND_ENUM_UINT8         enRtt;                                  /* 0:非RTT通话；1:RTT通话; 2:远端为TTY通话; 3: 远端不支持RTT  */
    VOS_INT32                           lRttChannelId;                          /* 通话使用的channel ID，-1为无效值 */
    VOS_UINT32                          ulCps;                                  /* RTT通话协商的cps值  */
}MN_CALL_INFO_PARAM_STRU;

typedef struct
{
    MN_CALL_EVENT_ENUM_U32              enEvent;                                /* 回复的事件类型 */
    MN_CLIENT_ID_T                      usClientId;                             /* 端口号 */
    MN_OPERATION_ID_T                   ucOpId;                                 /* 操作ID */
    MN_CALL_ID_T                        ucCallId;                               /* Call ID */
    TAF_CS_CAUSE_ENUM_UINT32            enCause;                                /* 错误码 */
}TAF_CALL_ECONF_DIAL_CNF_STRU;

typedef struct
{
    MN_CALL_EVENT_ENUM_U32                                  enEvent;            /* 事件名 */
    MN_CLIENT_ID_T                                          usClientId;         /* 用户ID */
    VOS_UINT8                                               ucNumOfMaxCalls;
    VOS_UINT8                                               ucNumOfCalls;       /* 多方通话与会者个数 */
    TAF_CALL_ECONF_INFO_PARAM_STRU                          astCallInfo[TAF_CALL_MAX_ECONF_CALLED_NUM];     /* 所用正在通话的信息 */
}TAF_CALL_ECONF_INFO_QRY_CNF_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32                          enEvent;                    /* 事件名 */
    MN_CLIENT_ID_T                                  usClientId;
    MN_OPERATION_ID_T                               ucOpId;                     /* 操作ID */
    VOS_UINT8                                       ucNumOfCalls;               /* 所有正在通话的个数 */
    TAF_CALL_ECONF_INFO_PARAM_STRU                  astCallInfo[TAF_CALL_MAX_ECONF_CALLED_NUM];     /* 所用正在通话的信息 */
} TAF_CALL_EVT_ECONF_NOTIFY_IND_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32              enEvent;                                /* 事件名 */
    MN_CLIENT_ID_T                      clientId;                               /* 用户ID */
    VOS_UINT8                           ucNumOfCalls;                           /* 所有正在通话的个数 */
    VOS_CHAR                            ucReserved;                             /* 保留字段 */
    MN_CALL_INFO_PARAM_STRU             astCallInfos[MN_CALL_MAX_NUM];           /* 所用正在通话的信息 */
} MN_CALL_INFO_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT8                           ucIndex;                                /* 定制紧急呼号码索引 */
    VOS_UINT8                           ucTotal;                                /* 定制紧急呼号码总数 */
    VOS_UINT8                           ucCategory;                             /* 紧急呼号码类型 */
    VOS_UINT8                           ucSimPresent;                           /* 紧急呼号码是有卡时有效还是无卡时有效，1有卡时有效，0无卡时有效 */
    VOS_UINT32                          ulMcc;                                  /* MCC,3 bytes */
    MN_CALL_BCD_NUM_STRU                stEccNum;                               /* 紧急呼号码 */
    VOS_UINT8                           ucAbnormalServiceFlg;                   /* 紧急呼号码有卡时是否仅在非正常服务时有效，0:任何服务时均有效，1:仅非正常服务时有效 */
    VOS_UINT8                           aucReserved[3];
}MN_CALL_APP_CUSTOM_ECC_NUM_REQ_STRU;


typedef struct
{
    VOS_UINT8                           ucCategory;                             /* 紧急呼号码类型 */
    VOS_UINT8                           ucValidSimPresent;                      /* 紧急呼号码是有卡时有效还是无卡时有效，1有卡时有效，0无卡时有效 */
    VOS_UINT8                           ucAbnormalServiceFlg;                   /* 紧急呼号码有卡时是否仅在非正常服务时有效，0:任何服务时均有效，1:仅非正常服务时有效 */
    VOS_UINT8                           ucEccNumLen;
    VOS_UINT8                           aucEccNum[MN_CALL_MAX_BCD_NUM_LEN];
    VOS_UINT32                          ulMcc;                                  /* MCC,3 bytes */
} MN_CALL_CUSTOM_ECC_NUM_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32              enEvent;                                /* 事件名 */
    MN_CLIENT_ID_T                      usClientId;
    VOS_UINT8                           aucReserve[2];
    VOS_UINT32                          ulEccNumCount;
    MN_CALL_CUSTOM_ECC_NUM_STRU         astCustomEccNumList[MN_CALL_MAX_EMC_NUM];
} MN_CALL_ECC_NUM_INFO_STRU;




enum MN_CALL_RADIO_MODE_ENUM
{
    MN_CALL_RADIO_MODE_GSM,
    MN_CALL_RADIO_MODE_WCDMA,
    MN_CALL_RADIO_MODE_IMS_EUTRAN,
    MN_CALL_RADIO_MODE_BUTT
};
typedef VOS_UINT8  MN_CALL_RADIO_MODE_ENUM_U8;

enum MN_CALL_CHANNEL_RATE_ENUM
{
    MN_CALL_CHANNEL_RATE_475,                   /* rate 4.75kbps/s */
    MN_CALL_CHANNEL_RATE_515,                   /* rate 5.15kbps/s */
    MN_CALL_CHANNEL_RATE_59,                    /* rate 5.9kbps/s */
    MN_CALL_CHANNEL_RATE_67,                    /* rate 6.7kbps/s */
    MN_CALL_CHANNEL_RATE_74,                    /* rate 7.4kbps/s */
    MN_CALL_CHANNEL_RATE_795,                   /* rate 7.95kbps/s */
    MN_CALL_CHANNEL_RATE_102,                   /* rate 10.2kbps/s */
    MN_CALL_CHANNEL_RATE_122,                   /* rate 12.2kbps/s */
    MN_CALL_CHANNEL_RATE_BUTT                   /* invalid value */
};
typedef VOS_UINT8  MN_CALL_CHANNEL_RATE_ENUM_U8;


enum TAF_CALL_TYPE_ENUM
{
    TAF_CALL_TYPE_NORMAL_CALL,
    TAF_CALL_TYPE_ECALL,
    TAF_CALL_TYPE_BUTT
};
typedef VOS_UINT16 TAF_CALL_TYPE_ENUM_UINT16;

typedef struct
{
    MN_CALL_RADIO_MODE_ENUM_U8          enMode;       /*GSM|WCDMA*/
    MN_CALL_CODEC_TYPE_ENUM_U8          enCodecType;  /* codec type */
    TAF_CALL_TYPE_ENUM_UINT16           enCallType;   /* eCall type */
} MN_CALL_CHANNEL_PARAM_STRU;

typedef struct
{
    VOS_BOOL                             bChannelEnable;    /*GSM|WCDMA*/
    MN_CALL_CHANNEL_PARAM_STRU           stChannelParam;        /* channel rate */
} MN_CALL_CHANNEL_INFO_STRU;

enum MN_CALL_CHANNEL_EVENT_ENUM
{
    MN_CALL_EVT_CHANNEL_OPEN,                                                   /* open codec channel */
    MN_CALL_EVT_CHANNEL_CLOSE,                                                  /* close codec channel */
    MN_CALL_EVT_CHANNEL_PARA_CHANGE,

    MN_CALL_EVT_CHANNEL_LOCAL_ALERTING,
    MN_CALL_EVT_TYPE_BUFF                                                       /* reserved value */
};
typedef  VOS_UINT32  MN_CALL_CHANNEL_EVENT_ENUM_U32;

/** ****************************************************************************
 * Name        : MN_CALL_APP_EC_KMC_DATA_STRU
 *
 * Description :
 *******************************************************************************/
typedef struct
{
    VOS_UINT8                                               ucEccKmcVer;
    VOS_UINT8                                               aucReserved[1];
    VOS_UINT16                                              usEccKmcLength;
    VOS_UINT8                                               aucEccKmcData[TAF_CALL_ECC_KMC_PUBLIC_KEY_MAX_HEX_LENGTH];
} MN_CALL_APP_EC_KMC_DATA_STRU;



typedef struct
{
    TAF_CALL_CCWAI_MODE_ENUM_UINT8                          enMode;
    TAF_CALL_CCWAI_SRV_CLASS_ENUM_UINT8                     enSrvClass;
    VOS_UINT8                                               aucReserve[2];
}TAF_CALL_CCWAI_SET_REQ_STRU;


typedef union
{
    MN_CALL_ORIG_PARAM_STRU             stOrig;
    MN_CALL_END_PARAM_STRU              stEnd;
    MN_CALL_ANS_PARAM_STRU              stAns;
    TAF_CALL_DTMF_PARAM_STRU            stDtmf;
    MN_CALL_SUPS_PARAM_STRU             stCallMgmtCmd;
    MN_CALL_UUS1_PARAM_STRU             stUus1Info;   /* 设置UUS1信息结构 */
    MN_CALL_SET_ALS_PARAM_STRU          stSetAls;     /* 设置ALS NO结构   */

    MN_CALL_APP_CUSTOM_ECC_NUM_REQ_STRU stEccNumReq;

    MN_CALL_SET_CSSN_REQ_STRU           stCssnReq;

    MN_CALL_MODIFY_REQ_STRU             stModifyReq;

    TAF_CALL_ECONF_DIAL_REQ_STRU        stEconfDial;

    TAF_CALL_CCWAI_SET_REQ_STRU         stCcwaiSet;

} MN_CALL_APP_REQ_PARM_UNION;

/* 应用请求消息的结构 */
typedef struct
{
    VOS_MSG_HEADER
    MN_CALL_APP_REQ_ENUM_UINT32         enReq;
    MN_CLIENT_ID_T                      clientId;
    MN_OPERATION_ID_T                   opId;
    MN_CALL_ID_T                        callId;
    MN_CALL_APP_REQ_PARM_UNION          unParm;
} MN_CALL_APP_REQ_MSG_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32              enEvent;                                /* 事件名 */
    MN_CLIENT_ID_T                      usClientId;
    TAF_CALL_VOICE_DOMAIN_ENUM_UINT8    enVoiceDomain;                          /* VOICE DOMAIN */
    VOS_UINT8                           aucReserve[1];
    VOS_UINT8                           ucCallNum;
    VOS_UINT8                           aucCallId[MN_CALL_MAX_NUM];
    VOS_UINT8                           aucCurcRptCfg[MN_CALL_RPT_CFG_MAX_SIZE];        /* CURC设置的主动上报标识 */
    VOS_UINT8                           aucUnsolicitedRptCfg[MN_CALL_RPT_CFG_MAX_SIZE]; /* 单个命令设置的主动上报标识 */
} MN_CALL_EVT_HOLD_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32              enEvent;                                /* 事件名 */
    MN_CLIENT_ID_T                      usClientId;
    TAF_CALL_VOICE_DOMAIN_ENUM_UINT8    enVoiceDomain;                          /* VOICE DOMAIN */
    VOS_UINT8                           aucReserve[1];
    VOS_UINT8                           ucCallNum;
    VOS_UINT8                           aucCallId[MN_CALL_MAX_NUM];
    VOS_UINT8                           aucCurcRptCfg[MN_CALL_RPT_CFG_MAX_SIZE];        /* CURC设置的主动上报标识 */
    VOS_UINT8                           aucUnsolicitedRptCfg[MN_CALL_RPT_CFG_MAX_SIZE]; /* 单个命令设置的主动上报标识 */
} MN_CALL_EVT_RETRIEVE_STRU;



typedef struct
{
    MN_CALL_EVENT_ENUM_U32              enEvent;                                /*回复的事件类型*/
    VOS_UINT32                          ulRet;
    AT_APPCTRL_STRU                     stAppCtrl;
    MN_CALL_CLPR_INFO_STRU              stClprInfo;
}MN_CALL_CLPR_GET_CNF_STRU;



typedef struct
{
    MN_CALL_EVENT_ENUM_U32              enEventId;          /* 事件类型   */

    AT_APPCTRL_STRU                     stAppCtrl;          /* 应用控制头 */
    TAF_CS_CAUSE_ENUM_UINT32            enCause;            /* 错误码     */
} MN_CALL_EVT_ERR_IND_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32              enEvent;        /*回复的事件类型*/
    TAF_ERROR_CODE_ENUM_UINT32          ulRet;          /*回复结果*/
    MN_CLIENT_ID_T                      ClientId;       /*端口号*/
    MN_OPERATION_ID_T                   OpId;           /*操作ID*/
    VOS_UINT8                           ucReserved;     /* 保留字段 */
} MN_CALL_SET_CSSN_CNF_STRU;

typedef struct
{
    MN_CALL_EVENT_ENUM_U32              enEvent;                                /* 事件名 */
    MN_CLIENT_ID_T                      usClientId;

    VOS_UINT8                           ucIsLocalAlertingFlag;                  /* 是否为本地播放回铃音标识1:本地播放回铃音；0:网络放音*/
    MN_CALL_CODEC_TYPE_ENUM_U8          enCodecType;                            /* codec type */

    TAF_CALL_VOICE_DOMAIN_ENUM_UINT8    enVoiceDomain;                          /* VOICE DOMAIN */
    VOS_UINT8                           aucRsv[3];
} MN_CALL_EVT_CHANNEL_INFO_STRU;






typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   opId;
    VOS_UINT8                           aucReserved1[1];
    TAF_CS_CAUSE_ENUM_UINT32            enCause;                                /* 错误码 */

    MN_CALL_ID_T                        callId;
    TAF_CALL_DTMF_STATE_ENUM_UINT8      enDtmfState;
    VOS_UINT8                           ucDtmfCnt;
    VOS_UINT8                           aucReserved2[1];

} TAF_CALL_EVT_DTMF_CNF_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   opId;
    VOS_UINT8                           ucReserved;
    VOS_UINT32                          ulResult;
} TAF_CALL_EVT_CCWAI_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT16                          usMsgName;                              /* 消息名 */
    VOS_UINT8                           aucReserve1[2];                         /* 保留 */
    VOS_UINT16                          usClientId;                             /* Client ID */
    VOS_UINT8                           ucOpId;                                 /* Operation ID */
    VOS_UINT8                           aucReserve2[1];                         /* 保留 */
}MN_CALL_QRY_ECALL_INFO_REQ_STRU;


typedef struct
{
    MN_CALL_ID_T                        ucCallId;                               /* 当前的call ID */
    MN_CALL_TYPE_ENUM_U8                enEcallType;                            /* eCall呼叫类型 */
    MN_CALL_STATE_ENUM_U8               enCallState;                            /* 呼叫状态 */
    VOS_UINT8                           aucReserved[1];                         /* 保留 */
    MN_CALL_CALLED_NUM_STRU             stDialNumber;                           /* 呼叫号码 */
}MN_CALL_ECALL_INFO_PARA_STRU;


typedef struct
{
    VOS_UINT8                           ucNumOfEcall;                           /* 当前eCall呼叫个数 */
    VOS_UINT8                           aucReserved[3];                         /* 保留 */
    MN_CALL_ECALL_INFO_PARA_STRU        astEcallInfos[MN_CALL_MAX_NUM];         /* eCall信息参数 */
}MN_CALL_ECALL_INFO_STRU;


typedef struct
{
    VOS_UINT16                          usClientId;
    VOS_UINT8                           ucOpId;
    VOS_UINT8                           aucReserved[1];
    MN_CALL_ECALL_INFO_STRU             stEcallInfo;
}MN_CALL_EVT_QRY_ECALL_INFO_CNF_STRU;



typedef struct
{
    VOS_UINT8                           ucDigitNum;
    /* ASCII 字符，合法的字符仅包括：'0' - '9', '*', '#', '+'。'+'只能出现在号码的最前面，
       号码的最大长度不能超过32（不包括"+"） */
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_FLASH_DIGIT_LEN + 1];
    VOS_UINT8                           aucReserved[2];
}TAF_CALL_FLASH_PARA_STRU;


typedef struct
{
    VOS_MSG_HEADER                                                  /*_H2ASN_Skip*/
    VOS_UINT16                          usMsgId;                    /*_H2ASN_Skip*//* 消息名 */
    VOS_UINT8                           aucReserved1[2];            /*_H2ASN_Skip*//* 保留 */
    MN_CLIENT_ID_T                      usClientId;                 /*_H2ASN_Skip*/
    MN_OPERATION_ID_T                   ucOpId;                     /*_H2ASN_Skip*/
    VOS_UINT8                           aucReserved2[1];            /*_H2ASN_Skip*/
    TAF_CALL_FLASH_PARA_STRU            stFlashPara;
}TAF_CALL_APP_SEND_FLASH_REQ_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucResult;                               /* VOS_OK:成功; VOS_ERR:失败 */
}TAF_CALL_EVT_SEND_FLASH_RSLT_STRU;


typedef struct
{
    MN_CALL_ID_T                        ucCallId;                                   /* 呼叫ID，如果携带该参数，必须是当前呼叫的呼叫ID */
    VOS_UINT8                           aucReserved[3];
    VOS_UINT8                           ucDigitNum;
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_BURST_DTMF_NUM];      /* DTMF Keys: 按键对应的字符  */
    VOS_UINT32                          ulOnLength;                                 /* DTMF音播放时长持续时间(单位ms) */
    VOS_UINT32                          ulOffLength;                                /* DTMF音发送的最小时间间隔(单位ms)  */
}TAF_CALL_BURST_DTMF_PARA_STRU;


typedef struct
{
    VOS_MSG_HEADER                                                                 /*_H2ASN_Skip*/
    VOS_UINT16                          usMsgId;                                   /*_H2ASN_Skip*/
    VOS_UINT8                           aucReserved1[2];                           /*_H2ASN_Skip*/
    MN_CLIENT_ID_T                      usClientId;                                /*_H2ASN_Skip*/
    MN_OPERATION_ID_T                   ucOpId;                                    /*_H2ASN_Skip*/
    VOS_UINT8                           aucReserved2[1];                           /*_H2ASN_Skip*/
    TAF_CALL_BURST_DTMF_PARA_STRU       stBurstDTMFPara;
} TAF_CALL_BURST_DTMF_REQ_MSG_STRU;



typedef struct
{
    VOS_MSG_HEADER                                                                 /*_H2ASN_Skip*/
    VOS_UINT16                          usMsgId;                                   /*_H2ASN_Skip*/
    VOS_UINT8                           aucReserved1[2];                           /*_H2ASN_Skip*/
    MN_CLIENT_ID_T                      usClientId;                                /*_H2ASN_Skip*/
    MN_OPERATION_ID_T                   ucOpId;                                    /*_H2ASN_Skip*/
    VOS_UINT8                           ucCallId;
} TAF_CALL_SND_CCLPR_REQ_MSG_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucCallId;
    TAF_CALL_NUMBER_TYPE_ENUM_UINT8     enNumType;
    TAF_CALL_NUMBER_PLAN_ENUM_UINT8     enNumPlan;
    VOS_UINT8                           ucDigitNum;
    VOS_UINT8                           aucRsved[1];
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_CALLED_NUMBER_CHARI_OCTET_NUM];
}TAF_CALL_EVT_CALLED_NUM_INFO_IND_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucCallId;
    TAF_CALL_NUMBER_TYPE_ENUM_UINT8     enNumType;
    TAF_CALL_NUMBER_PLAN_ENUM_UINT8     enNumPlan;
    VOS_UINT8                           ucPi;
    VOS_UINT8                           ucSi;
    VOS_UINT8                           ucDigitNum;
    VOS_UINT8                           aucRsved[3];
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_CALLING_NUMBER_CHARI_OCTET_NUM];
}TAF_CALL_EVT_CALLING_NUM_INFO_IND_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucCallId;
    VOS_UINT8                           ucDigitNum;
    VOS_UINT8                           aucRsved[3];
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_DISPALY_CHARI_OCTET_NUM];
}TAF_CALL_EVT_DISPLAY_INFO_IND_STRU;


typedef struct
{
    VOS_UINT8                           ucDispalyTag;
    VOS_UINT8                           ucDispalyLen;
    VOS_UINT8                           ucDigitNum;
    VOS_UINT8                           aucRsved[1];
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_EXTENDED_DISPALY_CHARI_OCTET_NUM];
}TAF_CALL_EVT_EXT_DISPLAY_INFO_DATA_STRU;


typedef struct
{
    MN_CLIENT_ID_T                                          usClientId;
    MN_OPERATION_ID_T                                       ucOpId;
    VOS_UINT8                                               ucCallId;
    VOS_UINT8                                               ucInfoRecsDataNum;
    VOS_UINT8                                               ucDisplayType;
    VOS_UINT8                                               ucExtDispInd;
    VOS_UINT8                                               ucReserved;
    TAF_CALL_EVT_EXT_DISPLAY_INFO_DATA_STRU                 aucInfoRecsData[TAF_CALL_MAX_EXT_DISPLAY_DATA_NUM];
}TAF_CALL_EVT_EXT_DISPLAY_INFO_IND_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucCallId;
    TAF_CALL_NUMBER_TYPE_ENUM_UINT8     enNumType;
    TAF_CALL_NUMBER_PLAN_ENUM_UINT8     enNumPlan;
    VOS_UINT8                           ucPi;
    VOS_UINT8                           ucSi;
    VOS_UINT8                           ucDigitNum;
    VOS_UINT8                           aucRsved[3];
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_CONNECTED_NUMBER_CHARI_OCTET_NUM];
}TAF_CALL_EVT_CONN_NUM_INFO_IND_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucCallId;
    VOS_UINT32                          bitOpPi:1;
    VOS_UINT32                          bitOpSi:1;
    VOS_UINT32                          bitOpRedirReason:1;
    VOS_UINT32                          bitOpSpare:29;
    TAF_CALL_NUMBER_TYPE_ENUM_UINT8     enNumType;
    TAF_CALL_NUMBER_PLAN_ENUM_UINT8     enNumPlan;
    VOS_UINT8                           ucPi;
    VOS_UINT8                           ucSi;
    VOS_UINT8                           ucRedirReason;
    VOS_UINT8                           ucDigitNum;
    VOS_UINT8                           aucRsved[2];
    VOS_UINT8                           aucDigitNum[TAF_CALL_MAX_REDIRECTING_NUMBER_CHARI_OCTET_NUM];
}TAF_CALL_EVT_REDIR_NUM_INFO_IND_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucCallId;
    VOS_UINT8                           ucSignalType;
    VOS_UINT8                           ucAlertPitch;
    VOS_UINT8                           ucSignal;
    VOS_UINT8                           aucRsved[1];
}TAF_CALL_EVT_SIGNAL_INFO_IND_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucCallId;
    VOS_UINT8                           ucPolarityIncluded;
    VOS_UINT8                           ucToggleModePresent;
    VOS_UINT8                           ucToggleMode;
    VOS_UINT8                           ucReversePolarityPresent;
    VOS_UINT8                           ucReversePolarity;
    VOS_UINT8                           ucPowerDenialTime;
    VOS_UINT8                           aucReserved[2];
}TAF_CALL_EVT_LINE_CTRL_INFO_IND_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucCallId;
    VOS_UINT8                           ucCallWaitingInd;
    VOS_UINT8                           aucRsved[3];
}TAF_CALL_EVT_CALL_WAITING_IND_STRU;



typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucCallId;
    TAF_CALL_NUMBER_TYPE_ENUM_UINT8     enNumType;
    TAF_CALL_NUMBER_PLAN_ENUM_UINT8     enNumPlan;
    VOS_UINT8                           ucPi;
    VOS_UINT8                           ucSi;
    VOS_UINT8                           ucDigitNum;
    VOS_UINT8                           ucSignalType;
    VOS_UINT8                           ucAlertPitch;
    VOS_UINT8                           ucSignal;
    VOS_UINT8                           ucSignalIsPresent;
    VOS_UINT8                           aucRsved[2];
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_CALLING_NUMBER_CHARI_OCTET_NUM + 1];
}TAF_CALL_EVT_CCWAC_INFO_IND_STRU;


typedef struct
{
    MN_CALL_CALLED_NUM_STRU             stDialNumber;
    VOS_UINT32                          ulSo;
    MN_CALL_TYPE_ENUM_U8                enCallType;
    VOS_UINT8                           aucReserved[3];
}TAF_CALL_CUSTOM_DIAL_PARA_STRU;


typedef struct
{
    VOS_MSG_HEADER                                                  /*_H2ASN_Skip*/
    VOS_UINT16                          usMsgId;                    /*_H2ASN_Skip*//* 消息名 */
    VOS_UINT8                           aucReserved1[2];            /*_H2ASN_Skip*//* 保留 */
    MN_CLIENT_ID_T                      usClientId;                 /*_H2ASN_Skip*/
    MN_OPERATION_ID_T                   ucOpId;                     /*_H2ASN_Skip*/
    VOS_UINT8                           aucReserved2[1];            /*_H2ASN_Skip*/
    TAF_CALL_CUSTOM_DIAL_PARA_STRU      stCustomDialPara;
}TAF_CALL_APP_SEND_CUSTOM_DIAL_REQ_STRU;



enum TAF_CALL_CONT_DTMF_SWITCH_ENUM
{
    TAF_CALL_CONT_DTMF_STOP         = 0,
    TAF_CALL_CONT_DTMF_START        = 1,
    TAF_CALL_CONT_DTMF_BUTT
};
typedef VOS_UINT8   TAF_CALL_CONT_DTMF_SWITCH_ENUM_U8;



typedef struct
{
    MN_CALL_ID_T                        ucCallId;                                   /* 呼叫ID, 当前呼叫的呼叫ID */
    TAF_CALL_CONT_DTMF_SWITCH_ENUM_U8   enSwitch;                                   /* enSwitch: TAF_CALL_CONT_DTMF_START,TAF_CALL_CONT_DTMF_STOP  */
    VOS_UINT8                           ucDigit;                                    /* DTMF Keys: 按键对应的字符，1-9、*、#  */
    VOS_UINT8                           ucReserved;
}TAF_CALL_CONT_DTMF_PARA_STRU;


typedef struct
{
    VOS_MSG_HEADER                                                                 /*_H2ASN_Skip*/
    VOS_UINT16                          usMsgId;                                   /*_H2ASN_Skip*/
    VOS_UINT8                           aucReserved1[2];                           /*_H2ASN_Skip*/
    MN_CLIENT_ID_T                      usClientId;                                /*_H2ASN_Skip*/
    MN_OPERATION_ID_T                   ucOpId;                                    /*_H2ASN_Skip*/
    VOS_UINT8                           ucReserved2;                               /*_H2ASN_Skip*/
    TAF_CALL_CONT_DTMF_PARA_STRU        stContDTMFPara;
} TAF_CALL_CONT_DTMF_REQ_MSG_STRU;


enum TAF_CALL_SEND_CONT_DTMF_CNF_RESULT_ENUM
{
    TAF_CALL_SEND_CONT_DTMF_CNF_RESULT_SUCCESS         = 0,
    TAF_CALL_SEND_CONT_DTMF_CNF_RESULT_ABNORMAL_STATE,
    TAF_CALL_SEND_CONT_DTMF_CNF_RESULT_BUFFER_FULL,
    TAF_CALL_SEND_CONT_DTMF_CNF_RESULT_ALLOC_MSG_FAIL,
    TAF_CALL_SEND_CONT_DTMF_CNF_RESULT_ALLOC_MEM_FAIL,

    TAF_CALL_SEND_CONT_DTMF_CNF_RESULT_BUTT
};
typedef VOS_UINT8   TAF_CALL_SEND_CONT_DTMF_CNF_RESULT_ENUM_U8;


typedef struct
{
    MN_CLIENT_ID_T                              usClientId;
    MN_OPERATION_ID_T                           ucOpId;
    MN_CALL_ID_T                                ucCallId;           /* 呼叫ID, 当前呼叫的呼叫ID */
    TAF_CALL_SEND_CONT_DTMF_CNF_RESULT_ENUM_U8  enResult;           /* 复用BURST DTMF CNF的结果 */
    VOS_UINT8                                   aucReserved[3];
}TAF_CALL_EVT_SEND_CONT_DTMF_CNF_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucResult;       /* 可能取值:VOS_OK, VOS_ERR*/
}TAF_CALL_EVT_SEND_CONT_DTMF_RSLT_STRU;



typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    MN_CALL_ID_T                        ucCallId;       /* 呼叫ID, 当前呼叫的呼叫ID */
    TAF_CALL_CONT_DTMF_SWITCH_ENUM_U8   enSwitch;       /* ucSwitch: TAF_CALL_CONT_DTMF_STOP,TAF_CALL_CONT_DTMF_START  */
    VOS_UINT8                           ucDigit;        /* 1-9,0,*,# */
    VOS_UINT8                           aucReserved[2];
}TAF_CALL_EVT_RCV_CONT_DTMF_IND_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    MN_CALL_ID_T                        ucCallId;                                   /* 呼叫ID, 当前呼叫的呼叫ID */
    VOS_UINT8                           ucDigitNum;                                 /**<  Digit number */
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_BURST_DTMF_NUM];      /**<  DTMF digits */
    VOS_UINT32                          ulOnLength;                                 /**<  refers to S.0005 3.7.3.3.2.9 */
    VOS_UINT32                          ulOffLength;                                /**<  refers to S.0005 3.7.3.3.2.9 */
}TAF_CALL_EVT_RCV_BURST_DTMF_IND_STRU;

typedef struct
{
    MN_CLIENT_ID_T                              usClientId;
    MN_OPERATION_ID_T                           ucOpId;
    MN_CALL_ID_T                                ucCallId;
    VOS_UINT32                                  ulRet;
    TAF_CALL_PRESENTATION_IND_ENUM_UINT8        enPI;
    VOS_UINT8                                   aucReserved[3];
}TAF_CALL_EVT_CCLPR_GET_CNF_STRU;



/*****************************************************************************
  4 宏定义
*****************************************************************************/


/*****************************************************************************
  5 全局变量声明
*****************************************************************************/


/*****************************************************************************
  6 接口函数声明
*****************************************************************************/


/* Orignate Call */
VOS_UINT32  MN_CALL_Orig(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    MN_CALL_ID_T                        *pCallId,
    const MN_CALL_ORIG_PARAM_STRU       *pstOrigParam
);

/* End Call */
VOS_UINT32  MN_CALL_End(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    MN_CALL_ID_T                        callId,
    const MN_CALL_END_PARAM_STRU        *pstEndParam
);

/* Send DTMF */
VOS_UINT32  TAF_CALL_SendDtmf(
    AT_MN_MSGTYPE_ENUM_UINT16           enMsgType,
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    const TAF_CALL_DTMF_PARAM_STRU     *pstDtmfParam
);

/* Call Management Command*/
VOS_UINT32  MN_CALL_Sups(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    const MN_CALL_SUPS_PARAM_STRU       *pstCallSupsParam
);

/* Get All Call Info */
VOS_UINT32  MN_CALL_GetCallInfos(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    MN_CALL_ID_T                        callId
);

VOS_UINT32 MN_CALL_SetAlsLineNo(
    TAF_UINT8                           ucIndex,
    MN_CALL_ALS_LINE_NO_ENUM_U8         enAlsLine
);

VOS_UINT32 MN_CALL_CheckUus1ParmValid(
    MN_CALL_SET_UUS1_TYPE_ENUM_U32      enSetType,
    MN_CALL_UUS1_INFO_STRU              *pstUus1Info
);

VOS_VOID  MN_CALL_SetUus1Info(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    MN_CALL_UUS1_PARAM_STRU            *pstUus1Info
);

VOS_VOID  MN_CALL_QryUus1Info(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId
);
VOS_VOID  MN_CALL_SetAls(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    MN_CALL_SET_ALS_PARAM_STRU         *pstAls
);
VOS_VOID  MN_CALL_QryAls(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId
);
VOS_UINT32  MN_CALL_SendAppRequest(
    MN_CALL_APP_REQ_ENUM_UINT32         enReq,
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    MN_CALL_ID_T                        callId,
    const MN_CALL_APP_REQ_PARM_UNION    *punParam
);

VOS_UINT32  MN_CALL_QryCdur(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    MN_CALL_ID_T                        callId
);

VOS_VOID MN_CALL_RcvTafEccNumReq(
    MN_CALL_APP_CUSTOM_ECC_NUM_REQ_STRU*    pstCustomEccNumReq
);


VOS_VOID MN_CALL_RcvAtClprGetReq(MN_CALL_APP_REQ_MSG_STRU            *pstCallMsg);


#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
extern VOS_UINT32  TAF_XCALL_SendBurstDtmf(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    TAF_CALL_BURST_DTMF_PARA_STRU      *pstSndBurstDTMFPara
);

extern VOS_UINT32  TAF_XCALL_SendFlashReq(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    TAF_CALL_FLASH_PARA_STRU           *pstFlashPara
);

VOS_UINT32  TAF_XCALL_SendCustomDialReq(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    TAF_CALL_CUSTOM_DIAL_PARA_STRU     *pstCustomDialPara
);

extern VOS_UINT32  TAF_XCALL_SendContinuousDtmf(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    TAF_CALL_CONT_DTMF_PARA_STRU       *pstSndContDTMFPara
);

VOS_UINT32  TAF_XCALL_SendCclpr(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    VOS_UINT8                           ucCallId
);

VOS_UINT32 TAF_XCALL_SendEncryptCall(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId,
    VOS_UINT32                                              enEccVoiceType,
    TAF_ECC_CALL_BCD_NUM_STRU                              *pstDialNumber
);
VOS_UINT32 TAF_XCALL_SendEccCtrl(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId,
    VOS_UINT32                                              enRemoteCtrlEvtType,
    VOS_UINT32                                              enResult
);
VOS_UINT32 TAF_XCALL_SetEccCap(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId,
    VOS_UINT32                                              ulEccSrvCap,
    VOS_UINT32                                              ulEccSrvStatus
);
VOS_UINT32 TAF_XCALL_SetEccTestMode(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId,
    VOS_UINT32                                              ulTestMode
);
VOS_UINT32 TAF_XCALL_SetEncryptCallKmc(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId,
    MN_CALL_APP_EC_KMC_DATA_STRU                          *pstEccKmcData
);
VOS_UINT32 TAF_XCALL_QryEncryptCallCap(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId
);
VOS_UINT32 TAF_XCALL_QryEncryptCallRandom(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId
);
VOS_UINT32 TAF_XCALL_QryEncryptCallKmc(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId
);
VOS_UINT32 TAF_XCALL_QryEncryptCallTestMode(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId
);
VOS_UINT32 TAF_XCALL_SetPrivacyModePreferred(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId,
    VOS_UINT32                                              ulPrivacyMode
);

VOS_UINT32 TAF_XCALL_QryPrivacyModePreferred(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId
);
#endif


VOS_UINT32 TAF_CALL_QryCnap(
    VOS_UINT32                              ulModuleId,
    MN_CLIENT_ID_T                          usClientId,
    MN_OPERATION_ID_T                       opId
);

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

#endif /* MN_CALL_API_H */

