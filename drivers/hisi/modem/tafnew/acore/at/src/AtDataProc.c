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

/*****************************************************************************
   1 头文件包含
*****************************************************************************/
#include "AtDataProc.h"
#include "ATCmdProc.h"
#include "AtCsdInterface.h"
#include "AtTafAgentInterface.h"
#include "AtCtxPacket.h"



/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_AT_DATAPROC_C

/*****************************************************************************
   2 全局变量定义
*****************************************************************************/

/* HiLink模式: 正常模式或网关模式 */
AT_HILINK_MODE_ENUM_U8                  g_enHiLinkMode = AT_HILINK_NORMAL_MODE;

/* 保存指定的FC ID对应的FC Pri */
AT_FCID_MAP_STRU                        g_stFcIdMaptoFcPri[FC_ID_BUTT];

/* ^DCONN上报函数表 */
CONST AT_PS_REPORT_CONN_RESULT_STRU           g_astAtRptConnectedResultTab[] =
{
    /* 消息ID */                        /* 消息处理函数 */
    {AT_APP_USER,                       AT_PS_ReportDCONN},
    {AT_NDIS_USER,                      AT_PS_ReportNdisStatConn}
};

/* ^DEND上报函数表 */
CONST AT_PS_REPORT_END_RESULT_STRU            g_astAtRptEndedResultTab[] =
{
    /* 消息ID */                        /* 消息处理函数 */
    {AT_APP_USER,                       AT_PS_ReportDEND},
    {AT_NDIS_USER,                      AT_PS_ReportNdisStatEnd}
};

CONST AT_PS_SND_PDP_ACT_IND_STRU              g_astAtSndPdpActIndTab[] =
{
    /* 消息ID */                        /* 消息处理函数 */
    {AT_APP_USER,                       AT_PS_SndRnicPdpActInd},
    {AT_NDIS_USER,                      AT_PS_ProcNdisPdpActInd}
};

CONST AT_PS_SND_PDP_DEACT_IND_STRU            g_astAtSndPdpDeActIndTab[] =
{
    /* 消息ID */                        /* 消息处理函数 */
    {AT_APP_USER,                       AT_PS_SndRnicPdpDeactInd},
    {AT_NDIS_USER,                      AT_PS_ProcNdisPdpDeActInd}
};

CONST AT_CHDATA_RNIC_RMNET_ID_STRU            g_astAtChdataRnicRmNetIdTab[] =
{
    {AT_CH_DATA_CHANNEL_ID_1, RNIC_DEV_ID_RMNET0, PS_IFACE_ID_RMNET0, {0, 0}},
    {AT_CH_DATA_CHANNEL_ID_2, RNIC_DEV_ID_RMNET1, PS_IFACE_ID_RMNET1, {0, 0}},
    {AT_CH_DATA_CHANNEL_ID_3, RNIC_DEV_ID_RMNET2, PS_IFACE_ID_RMNET2, {0, 0}},
    {AT_CH_DATA_CHANNEL_ID_4, RNIC_DEV_ID_RMNET3, PS_IFACE_ID_RMNET3, {0, 0}},
    {AT_CH_DATA_CHANNEL_ID_5, RNIC_DEV_ID_RMNET4, PS_IFACE_ID_RMNET4, {0, 0}}
    ,
    {AT_CH_DATA_CHANNEL_ID_6, RNIC_DEV_ID_RMNET5, PS_IFACE_ID_RMNET5, {0, 0}},
    {AT_CH_DATA_CHANNEL_ID_7, RNIC_DEV_ID_RMNET6, PS_IFACE_ID_RMNET6, {0, 0}}
};

CONST AT_PS_APP_CALL_RNIC_IFACE_ID_STRU       g_astAtPsAppCallRnicIFaceIdTab[] =
{
    {AT_PS_APP_CALL_ID_BEGIN + 0, RNIC_DEV_ID_RMNET0, PS_IFACE_ID_RMNET0, 0},
    {AT_PS_APP_CALL_ID_BEGIN + 1, RNIC_DEV_ID_RMNET1, PS_IFACE_ID_RMNET1, 0},
    {AT_PS_APP_CALL_ID_BEGIN + 2, RNIC_DEV_ID_RMNET2, PS_IFACE_ID_RMNET2, 0},
};

CONST AT_PS_NDIS_CALL_IFACE_ID_STRU           g_astAtPsNdisCallIFaceIdTab[] =
{
    {AT_PS_NDIS_CALL_ID_BEGIN, PS_IFACE_ID_NDIS0, 0, 0},
};

AT_PS_FC_IFACE_ID_STRU                        g_astAtPsFcIFaceIdTab[] =
{
    /* APP的IFACE ID相关 */
    {FC_ID_NIC_1,   PS_IFACE_ID_RMNET0, 0, 0},
    {FC_ID_NIC_2,   PS_IFACE_ID_RMNET1, 0, 0},
    {FC_ID_NIC_3,   PS_IFACE_ID_RMNET2, 0, 0},
    {FC_ID_NIC_4,   PS_IFACE_ID_RMNET3, 0, 0},
    {FC_ID_NIC_5,   PS_IFACE_ID_RMNET4, 0, 0},
    {FC_ID_NIC_6,   PS_IFACE_ID_RMNET5, 0, 0},
    {FC_ID_NIC_7,   PS_IFACE_ID_RMNET6, 0, 0},
    /* NDIS的IFACE ID相关 */
    {FC_ID_NIC_1,   PS_IFACE_ID_NDIS0,  0, 0},
};

CONST AT_PS_WLAN_PDN_ACT_ERR_CODE_MAP_STRU           g_astTafPsWlanPdnActErrCodeMapTbl[] =
{
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_PARA_ERROR,              WIFI_IMSA_PDN_ACT_RESULT_PARAM_ERR                            },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_LINKLOST,                WIFI_IMSA_PDN_ACT_RESULT_LINKLOST                             },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_TIMEOUT,                 WIFI_IMSA_PDN_ACT_RESULT_TIMEOUT                              },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_SM_ERROR,                WIFI_IMSA_PDN_ACT_RESULT_MAPCON_STATE_ERR                     },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_OTHER_ERROR,             WIFI_IMSA_PDN_ACT_RESULT_OTHER_FAILURE                        },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_SIM_AUTH_NO_RESP,        WIFI_IMSA_PDN_ACT_RESULT_SIM_AUTH_NO_RESP                     },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_AUTH_NETWORK_ERROR,      WIFI_IMSA_PDN_ACT_RESULT_AUTH_NETWORK_ERROR                   },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_PARSE_NETWORK_PAYLOAD_ERROR,     WIFI_IMSA_PDN_ACT_RESULT_PARSE_NETWORK_PAYLOAD_ERROR  },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_ADD_SA_SP_ERROR,         WIFI_IMSA_PDN_ACT_RESULT_ADD_SA_AP_ERROR                      },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_CLIENT_PAYLOAD_ERROR,    WIFI_IMSA_PDN_ACT_RESULT_CLIENT_PAYLOAD_ERROR                 },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_MSG_NETWORK_OVERTIME_ERROR,      WIFI_IMSA_PDN_ACT_RESULT_MSG_NETWORK_OVERTIME_ERROR   },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_ENC_ALG_ERROR,           WIFI_IMSA_PDN_ACT_RESULT_ENC_ALG_ERROR                        },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_APN_ID_ERROR,            WIFI_IMSA_PDN_ACT_RESULT_APN_ID_ERROR                         },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_NETWORK_AUTH_SIM_ERROR,  WIFI_IMSA_PDN_ACT_RESULT_NETWORK_AUTH_SIM_ERROR               },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_STATUS_ERROR,            WIFI_IMSA_PDN_ACT_RESULT_STATUS_ERROR                         },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_SA_INIT_ERROR,           WIFI_IMSA_PDN_ACT_RESULT_SA_INIT_ERROR                        },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_NON_3GPP_ACCESS_TO_EPC_NOT_ALLOWED,  WIFI_IMSA_PDN_ACT_RESULT_NON_3GPP_ACCESS_TO_EPC_NOT_ALLOWED   },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_USER_UNKNOWN,            WIFI_IMSA_PDN_ACT_RESULT_USER_UNKNOWN                         },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_PLMN_NOT_ALLOWED,        WIFI_IMSA_PDN_ACT_RESULT_PLMN_NOT_ALLOWED                     },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_AUTHORIZATION_REJECTED,  WIFI_IMSA_PDN_ACT_RESULT_AUTHORIZATION_REJECTED               },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_RAT_TYPE_NOT_ALLOWED,    WIFI_IMSA_PDN_ACT_RESULT_RAT_TYPE_NOT_ALLOWED                 },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_NETWORK_FAILURE,         WIFI_IMSA_PDN_ACT_RESULT_NETWORK_FAILURE                      },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_NO_APN_SUBSCRIPTION,     WIFI_IMSA_PDN_ACT_RESULT_NO_APN_SUBSCRIPTION                  },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_PDN_CONNECTION_REJECTION,WIFI_IMSA_PDN_ACT_RESULT_PDN_CONNECTION_REJECTION             },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_CONNECTION_REACHED,      WIFI_IMSA_PDN_ACT_RESULT_CONNECTION_REACHED                   },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_PROTECLE_ERROR,          WIFI_IMSA_PDN_ACT_RESULT_PROTECLE_ERROR                       },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_INTERNAL_ADDRESS_ERROR,  WIFI_IMSA_PDN_ACT_RESULT_INTERNAL_ADDRESS_ERROR               },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_IPV4_ADDRESS_CONFLICT,   WIFI_IMSA_PDN_ACT_RESULT_IPV4_ADDRESS_CONFLICT                },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_CERT_ERROR,              WIFI_IMSA_PDN_ACTIVE_RESULT_CERT_ERROR                        },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_NOT_SUPPORT_CONCURRENCE, WIFI_IMSA_PDN_ACTIVE_RESULT_BEARS_CONCURRENT_NOT_SUPPORT      },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_SHUTDOWN,                WIFI_IMSA_PDN_ACTIVE_RESULT_SHUTDOWN                          },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_DONE_ERROR,              WIFI_IMSA_PDN_ACTIVE_RESULT_DONE_ERROR                        },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_REDIRECT_FAILURE,        WIFI_IMSA_PDN_ACTIVE_RESULT_REDIRECT_FAILURE                  },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_ROAM_FORBID,             WIFI_IMSA_PDN_ACT_RESULT_ROAM_FORBID                          },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_HIFI_STATUS_OVERTIME,    WIFI_IMSA_PDN_ACT_RESULT_HIFI_STATUS_OVERTIME                 },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_IKED_ABNORMAL,           WIFI_IMSA_PDN_ACT_RESULT_IKED_ABNORMAL                        },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_UNKNOWN_NETWORK_ERROR,   WIFI_IMSA_PDN_ACT_RESULT_UNKNOWN_NETWORK_ERROR                },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_SWITCH_CACHE_MAX_REACHED,   WIFI_IMSA_PDN_ACT_RESULT_SWITCH_CACHE_MAX_REACHED          },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_APN_MAX_REACHED,         WIFI_IMSA_PDN_ACT_RESULT_APN_MAX_REACHED                      },
    { TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_REQUEST_REPEATED,        WIFI_IMSA_PDN_ACT_RESULT_REQUEST_REPEATED                     },
};

CONST AT_PS_WLAN_PDN_DEACT_ERR_CODE_MAP_STRU           g_astTafPsWlanPdnDeActErrCodeMapTbl[] =
{
    { TAF_PS_CAUSE_WLAN_PDN_DEACT_RESULT_KEEP_LIVE,             WIFI_IMSA_PDN_DEACT_CAUSE_KEEP_LIVE                           },
    { TAF_PS_CAUSE_WLAN_PDN_DEACT_RESULT_NETWORK_DELETE,        WIFI_IMSA_PDN_DEACT_CAUSE_NETWORK_DELETE                      },
    { TAF_PS_CAUSE_WLAN_PDN_DEACT_RESULT_IKED_RESET,            WIFI_IMSA_PDN_DEACT_CAUSE_IKED_RESET                          },
    { TAF_PS_CAUSE_WLAN_PDN_DEACT_RESULT_REKEY_ERROR,           WIFI_IMSA_PDN_DEACT_CAUSE_REKEY_ERROR                         },
    { TAF_PS_CAUSE_WLAN_PDN_DEACT_RESULT_MAPCONN_TEAR,          WIFI_IMSA_PDN_DEACT_CAUSE_MAPCON_TEAR                         },
    { TAF_PS_CAUSE_WLAN_PDN_DEACT_RESULT_BEARS_CONNCURRENT_NOT_SUPPOER, WIFI_IMSA_PDN_DEACT_CAUSE_BEARS_CONCURRENT_NOT_SUPPORT},
};

/*****************************************************************************
   3 函数、变量声明
*****************************************************************************/

extern const TAF_CHAR                       *g_PppDialRateDisplay[];

extern const AT_DISPLAY_RATE_PAIR_STRU      g_ucLTERateDisplay[AT_UE_LTE_CATEGORY_NUM_MAX];

extern const VOS_CHAR                       *g_ucDialRateDisplayNv[];

/*****************************************************************************
   4 函数实现
*****************************************************************************/

VOS_UINT32 AT_Ipv4AddrAtoi(
    VOS_CHAR                           *pcString,
    VOS_UINT8                          *pucNumber
)
{
    VOS_UINT32                          i = 0;
    VOS_UINT32                          ulStrLen = 0;
    VOS_UINT32                          ulNumLen = 0;
    VOS_UINT32                          ulDotNum = 0;
    VOS_UINT32                          ulValTmp = 0;
    VOS_UINT8                           aucAddr[4] = {0};


    if ( (VOS_NULL_PTR == pcString) || (VOS_NULL_PTR == pucNumber) )
    {
        return VOS_ERR;
    }

    ulStrLen = VOS_StrLen(pcString);

    if ( ulStrLen > VOS_StrLen("255.255.255.255") )
    {
        AT_NORM_LOG("AT_Ipv4AddrAtoi: PCSCF IPV4 address length out of range");
        return VOS_ERR;
    }

    for ( i = 0; i < ulStrLen; i++ )
    {
        if ( (pcString[i] >= '0') && (pcString[i] <= '9') )
        {
            ulValTmp = (ulValTmp * 10) + (pcString[i] - '0');

            ulNumLen++;
            continue;
        }
        else if ( pcString[i] == '.' )
        {
            if ( (0 == ulNumLen) || (ulNumLen > 3) )
            {
                AT_NORM_LOG("AT_Ipv4AddrAtoi: the number between dots is out of range");
                return VOS_ERR;
            }

            if ( ulValTmp > 255 )
            {
                AT_NORM_LOG("AT_Ipv4AddrAtoi: the number is larger than 255");
                return VOS_ERR;
            }

            aucAddr[ulDotNum] = (VOS_UINT8)ulValTmp;

            ulValTmp = 0;
            ulNumLen = 0;

            /* 统计'.'的个数 */
            ulDotNum++;

            continue;
        }
        else
        {
            AT_NORM_LOG("AT_Ipv4AddrAtoi: character not number nor dot, return ERROR");
            /* 其他值直接返回失败 */
            return VOS_ERR;
        }
    }

    /* 如果不是3个'.'则返回失败 */
    if ( 3 != ulDotNum )
    {
        AT_NORM_LOG("AT_Ipv4AddrAtoi: dot number is not 3");
        return VOS_ERR;
    }

    /* 检查最后地址域的取值 */
    if (ulValTmp > 255)
    {
        AT_NORM_LOG("AT_Ipv4AddrAtoi: last number is larger than 255");
        return VOS_ERR;
    }

    aucAddr[ulDotNum] = (VOS_UINT8)ulValTmp;

    TAF_MEM_CPY_S(pucNumber, TAF_IPV4_ADDR_LEN, aucAddr, 4);

    return VOS_OK;
}


VOS_UINT32 AT_Ipv4AddrItoa(
    VOS_CHAR                           *pcString,
    VOS_UINT8                          *pucNumber
)
{
    VOS_UINT32                          ret = VOS_OK;

    if ( (VOS_NULL_PTR == pcString) || (VOS_NULL_PTR == pucNumber) )
    {
        ret = VOS_ERR;
    }
    else
    {
        TAF_MEM_SET_S(pcString, TAF_MAX_IPV4_ADDR_STR_LEN, 0x00, TAF_MAX_IPV4_ADDR_STR_LEN);
        VOS_sprintf_s(pcString, TAF_MAX_IPV4_ADDR_STR_LEN, "%d.%d.%d.%d", pucNumber[0], pucNumber[1], pucNumber[2], pucNumber[3]);
    }

    return ret;
}


VOS_UINT32 AT_Ipv4Addr2Str(
    VOS_CHAR                           *pcString,
    VOS_UINT8                          *pucNumber
)
{
    if ( (VOS_NULL_PTR == pcString) || (VOS_NULL_PTR == pucNumber) )
    {
        return VOS_ERR;
    }

    TAF_MEM_SET_S(pcString, TAF_MAX_IPV4_ADDR_STR_LEN, 0x00, TAF_MAX_IPV4_ADDR_STR_LEN);

    /* 如果输入地址为空，则返回空字符串 */
    if ((0 == pucNumber[0])
      &&(0 == pucNumber[1])
      &&(0 == pucNumber[2])
      &&(0 == pucNumber[3]))
    {
        return VOS_OK;
    }

    VOS_sprintf_s(pcString, TAF_MAX_IPV4_ADDR_STR_LEN, "%d.%d.%d.%d", pucNumber[0], pucNumber[1], pucNumber[2], pucNumber[3]);

    return VOS_OK;
}


VOS_VOID AT_PcscfIpv4Addr2Str(
    VOS_CHAR                           *pcString,
    VOS_UINT8                          *pucNumber
)
{
    TAF_MEM_SET_S(pcString, TAF_MAX_IPV4_ADDR_STR_LEN, 0, TAF_MAX_IPV4_ADDR_STR_LEN);

    VOS_sprintf_s(pcString, TAF_MAX_IPV4_ADDR_STR_LEN, "%d.%d.%d.%d", pucNumber[0], pucNumber[1], pucNumber[2], pucNumber[3]);
}


VOS_UINT32 AT_Ipv6AddrAtoi(
    VOS_CHAR                           *pcString,
    VOS_UINT8                          *pucNumber
)
{
    VOS_UINT32                          i = 0;
    VOS_UINT32                          ulStrLen = 0;
    VOS_UINT32                          ulNumLen = 0;
    VOS_UINT32                          ulDotNum = 0;
    VOS_UINT32                          ulValTmp = 0;
    VOS_UINT8                           aucAddr[TAF_IPV6_ADDR_LEN] = {0};


    if ( (VOS_NULL_PTR == pcString) || (VOS_NULL_PTR == pucNumber) )
    {
        return VOS_ERR;
    }

    ulStrLen = VOS_StrLen(pcString);

    if ( ulStrLen > VOS_StrLen("255.255.255.255.255.255.255.255.255.255.255.255.255.255.255.255") )
    {
        return VOS_ERR;
    }

    for ( i = 0; i < ulStrLen; i++ )
    {
        if ( (pcString[i] >= '0') && (pcString[i] <= '9') )
        {
            ulValTmp = (ulValTmp * 10) + (pcString[i] - '0');

            ulNumLen++;
            continue;
        }
        else if ( pcString[i] == '.' )
        {
            if ( (0 == ulNumLen) || (ulNumLen > 3) )
            {
                return VOS_ERR;
            }

            if ( ulValTmp > 255 )
            {
                return VOS_ERR;
            }

            aucAddr[ulDotNum] = (VOS_UINT8)ulValTmp;

            ulValTmp = 0;
            ulNumLen = 0;

            /* 统计'.'的个数 */
            ulDotNum++;

            continue;
        }
        else
        {
            /* 其他值直接返回失败 */
            return VOS_ERR;
        }
    }

    /* 如果不是3个'.'则返回失败 */
    if ((TAF_IPV6_ADDR_LEN - 1) != ulDotNum )
    {
        return VOS_ERR;
    }

    /* 检查最后地址域的取值 */
    if (ulValTmp > 255)
    {
        return VOS_ERR;
    }

    aucAddr[ulDotNum] = (VOS_UINT8)ulValTmp;

    TAF_MEM_CPY_S(pucNumber, TAF_IPV6_ADDR_LEN, aucAddr, TAF_IPV6_ADDR_LEN);

    return VOS_OK;
}


VOS_VOID  AT_Ipv6LenStrToAddrProcCompressed(
    VOS_UINT8                          *pucStr,
    VOS_UINT8                           ucColonCount,
    VOS_UINT8                           ucDotCount,
    VOS_UINT8                           ucStrlen,
    VOS_UINT8                           ucIdxPos
)
{
    VOS_UINT8                           i;

    /* 把字符串从压缩位置开始顺次向后移动 */
    for (i = ucStrlen; i >= ucIdxPos; i--)
    {
        if (AT_MAX_IPV6_STR_DOT_NUM != ucDotCount)
        {
            pucStr[i + AT_MAX_IPV6_STR_COLON_NUM - ucColonCount] =  pucStr[i];
        }
        else
        {
            pucStr[i + AT_MAX_IPV4V6_STR_COLON_NUM - ucColonCount] =  pucStr[i];
        }
    }

    /* 补足压缩的冒号 */
    if (AT_MAX_IPV6_STR_DOT_NUM != ucDotCount)
    {
        for (i = ucIdxPos; i < (ucIdxPos + AT_MAX_IPV6_STR_COLON_NUM - ucColonCount); i++)
        {
            pucStr[i + 1] =  ':';
        }
    }
    else
    {
        for (i = ucIdxPos; i < (ucIdxPos + AT_MAX_IPV4V6_STR_COLON_NUM - ucColonCount); i++)
        {
            pucStr[i + 1] =  ':';
        }
    }

    return;
}


VOS_UINT32 AT_Ipv6LenStrToAddrAccess(
    VOS_UINT8                          *pucStr,
    VOS_UINT8                          *pucColonCount,
    VOS_UINT8                          *pucDotCount,
    VOS_UINT8                          *pucStrlen,
    VOS_UINT8                          *pucIdxPos
)
{
    VOS_UINT32                          i;

    for (i = 0; ((i < TAF_MAX_IPV6_ADDR_COLON_STR_LEN) && ('\0' != pucStr[i])); i++)
    {
        if ( (pucStr[i] < '0' || pucStr[i] > '9')
          && (pucStr[i] < 'A' || pucStr[i] > 'F')
          && ('.' != pucStr[i])
          && (':' != pucStr[i]) )
        {
            return VOS_ERR;
        }
        /* 取得补充冒号索引位置 */
        if ((i > 0)
         && (':' == pucStr[i - 1])
         && (':' == pucStr[i]))
        {
            *pucIdxPos = (VOS_UINT8)i;
        }

        /* 统计冒号个数 */
        if (':' == pucStr[i])
        {
            (*pucColonCount)++;
        }

        /*统计点号个数*/
        if ('.' == pucStr[i])
        {
            (*pucDotCount)++;
        }
    }

    *pucStrlen = (VOS_UINT8)i;

    return VOS_OK;
}


VOS_UINT32 AT_PcscfIpv6StrToAddr(
    VOS_UINT8                          *pucStr,
    VOS_UINT8                          *pucIpAddr,
    VOS_UINT8                           ucColonCount
)
{
    VOS_UINT8                           i;
    VOS_UINT8                           j;
    VOS_UINT16                          usValue;            /*Ipv6十六进制转换用*/
    VOS_UINT8                           ucValue;            /*Ipv4十六进制转换用*/
    VOS_UINT32                          ulNumLen;

    usValue                             = 0;
    ucValue                             = 0;
    j                                   = 0;
    ulNumLen                            = 0;

    if ((VOS_NULL_PTR == pucStr)
     || (VOS_NULL_PTR == pucIpAddr))
    {
        AT_NORM_LOG("AT_PcscfIpv6StrToAddr: input ptr is NULL, return ERROR");
        return VOS_ERR;
    }

    for (i = 0; ((i < TAF_MAX_IPV6_ADDR_COLON_STR_LEN) && ('\0' != pucStr[i])); i++)
    {
        /* 匹配字符 */
        if (':' != pucStr[i])
        {
            usValue <<= 4;

            if ((pucStr[i] >= '0') &&(pucStr[i] <= '9'))
            {
                /* 十进制格式转换 */
                usValue += (pucStr[i] - '0');
                ucValue  = (VOS_UINT8)((ucValue * 10) + (pucStr[i] - 0x30));
            }
            else
            {
                /* 十六进制格式转换 */
                 usValue += ((pucStr[i] - 'A') + 10);
            }

            ulNumLen++;
        }
        /* 匹配到冒号 */
        else
        {
            /* 冒号之间的字符超过4个则认为格式错误 */
            if (4 < ulNumLen)
            {
                AT_ERR_LOG("AT_PcscfIpv6StrToAddr: the number of char betwwen colons is more than 4, return ERROR");
                return VOS_ERR;
            }

            /* IPV6十六进制取高八位数据 */
            pucIpAddr[j] = (VOS_UINT8)((usValue >> 8) & 0x00FF);
            j++;
            /* IPV6十六进制取低八位数据 */
            pucIpAddr[j] = (VOS_UINT8)(usValue & 0x00FF);
            j++;
            usValue      = 0;
            ulNumLen     = 0;
        }
    }

    /* 匹配最后一次转换 */
    if (AT_MAX_IPV6_STR_COLON_NUM == ucColonCount)
    {
        pucIpAddr[j] = (VOS_UINT8)((usValue >> 8) & 0x00FF);
        j++;
        pucIpAddr[j] = (VOS_UINT8)(usValue & 0x00FF);
        j++;
    }

    return VOS_OK;
}


VOS_UINT32 AT_CheckPcscfIpv6Addr(
    VOS_CHAR                           *pucIpv6Str,
    VOS_UINT32                         *pulPortExistFlg
)
{
    VOS_CHAR                           *pucIpv6Start = VOS_NULL_PTR;
    VOS_CHAR                           *pucIpv6End   = VOS_NULL_PTR;

    if ((VOS_NULL_PTR == pucIpv6Str)
     || (VOS_NULL_PTR == pulPortExistFlg))
    {
        AT_NORM_LOG("AT_CheckPcscfIpv6Addr: input ptr is NULL, return ERROR");
        return VOS_ERR;
    }

    pucIpv6End   = VOS_StrStr(pucIpv6Str, "]");
    pucIpv6Start = VOS_StrStr(pucIpv6Str, "[");

    if ((VOS_NULL_PTR == pucIpv6End)
     && (VOS_NULL_PTR == pucIpv6Start))
    {
        AT_NORM_LOG("AT_CheckPcscfIpv6Addr: NO [ ] symbol in IPV6 addr, port not exists");
        *pulPortExistFlg = VOS_FALSE;
        return VOS_OK;
    }

    if ((VOS_NULL_PTR != pucIpv6End)
     && (VOS_NULL_PTR != pucIpv6Start)
     && (pucIpv6End > pucIpv6Start))
    {
        if ((pucIpv6End - pucIpv6Start) > TAF_MAX_IPV6_ADDR_COLON_STR_LEN)
        {
            AT_ERR_LOG("AT_CheckPcscfIpv6Addr: length of IPV6 addr in [] is larger than 39, return ERROR");
            return VOS_ERR;
        }

        AT_NORM_LOG("AT_CheckPcscfIpv6Addr: Have both [ ] symbol in IPV6 addr");
        *pulPortExistFlg = VOS_TRUE;

        return VOS_OK;
    }

    AT_ERR_LOG("AT_CheckPcscfIpv6Addr: IPV6 addr format incorrect");
    return VOS_ERR;
}



VOS_UINT32  AT_ParsePortFromPcscfIpv6Addr(
    VOS_UINT8                          *pucStr,
    VOS_UINT8                          *pucIpv6Addr,
    VOS_UINT32                         *pulPortExistFlg,
    VOS_UINT32                         *pulPortNum
)
{
    VOS_CHAR                           *pucIpv6Start = VOS_NULL_PTR;
    VOS_CHAR                           *pucIpv6End   = VOS_NULL_PTR;
    VOS_CHAR                           *pucIpv6Str   = VOS_NULL_PTR;
    VOS_CHAR                           *pcStrPort    = VOS_NULL_PTR;

    if ((VOS_NULL_PTR == pucStr)
     || (VOS_NULL_PTR == pucIpv6Addr)
     || (VOS_NULL_PTR == pulPortExistFlg)
     || (VOS_NULL_PTR == pulPortNum))
    {
        AT_ERR_LOG("AT_ParsePortFromPcscfIpv6Addr: input ptr is NULL, return ERROR");
        return VOS_ERR;
    }

    if (VOS_OK != AT_CheckPcscfIpv6Addr((VOS_CHAR*)pucStr, pulPortExistFlg))
    {
        AT_ERR_LOG("AT_ParsePortFromPcscfIpv6Addr: AT_CheckPcscfIpv6Addr FAIL, return ERROR");
        return VOS_ERR;
    }

    /* [ipv6]:port格式兼容处理，获取ipv6端口号 */
    pucIpv6Str   = (VOS_CHAR*)pucStr;
    pucIpv6End   = VOS_StrStr(pucIpv6Str, "]");
    pucIpv6Start = VOS_StrStr(pucIpv6Str, "[");

    if (VOS_TRUE == *pulPortExistFlg)
    {
        if ( (VOS_NULL_PTR == pucIpv6End)
          || (VOS_NULL_PTR == pucIpv6Start))
        {
            AT_ERR_LOG("AT_ParsePortFromPcscfIpv6Addr: NO [ ] symbol in IPV6 addr, return ERROR");
            return VOS_ERR;
        }

        /* 保存中括号内的IPV6地址信息，移除端口号 */
        TAF_MEM_CPY_S(pucIpv6Addr, TAF_MAX_IPV6_ADDR_COLON_STR_LEN, pucIpv6Start + 1, (VOS_UINT32)((pucIpv6End - pucIpv6Start) - 1));

        /* 记录分隔地址和端口的冒号地址 */
        pcStrPort = VOS_StrStr(pucIpv6End, ":");

        /* [ipv6]:port格式没有冒号，返回ERROR*/
        if (VOS_NULL_PTR == pcStrPort)
        {
            *pulPortExistFlg = VOS_FALSE;
            AT_ERR_LOG("AT_ParsePortFromPcscfIpv6Addr: IPV6 Port colon missed, return ERROR ");
            return VOS_ERR;
        }

        /* 冒号前面不是"]"，返回ERROR */
        if (pcStrPort != pucIpv6End + 1)
        {
            *pulPortExistFlg = VOS_FALSE;
            AT_ERR_LOG("AT_ParsePortFromPcscfIpv6Addr: IPV6 Port colon location incorrect, return ERROR ");
            return VOS_ERR;
        }

        /* [ipv6]:port格式端口解析错误，返回ERROR*/
        if (VOS_OK != AT_PortAtoI(pcStrPort+1, pulPortNum))
        {
            *pulPortExistFlg = VOS_FALSE;
            AT_ERR_LOG("AT_ParsePortFromPcscfIpv6Addr: IPV6 Port decode ERROR");
            return VOS_ERR;
        }

        *pulPortExistFlg = VOS_TRUE;
    }
    else
    {
        /* 没有端口号，传入的字符串只包含IPV6地址 */
        TAF_MEM_CPY_S(pucIpv6Addr, TAF_MAX_IPV6_ADDR_COLON_STR_LEN, pucStr, TAF_MAX_IPV6_ADDR_COLON_STR_LEN);
    }

    return VOS_OK;
}


VOS_UINT32  AT_ParseAddrFromPcscfIpv6Addr(
    VOS_UINT8                          *pucStr,
    VOS_UINT8                          *pucIpAddr
)
{
    VOS_UINT8                           ucColonCount;       /* 字符串中冒号个数 */
    VOS_UINT8                           ucDotCount;         /* 字符串中点号个数 */
    VOS_UINT8                           ucStrlen;           /* 字符串长度 */
    VOS_UINT8                           ucIdxPos;           /* 需要补充冒号的位置 */
    VOS_UINT32                          ulResult;

    ucColonCount = 0;
    ucDotCount   = 0;
    ucStrlen     = 0;
    ucIdxPos     = 0xFF;

    if ((VOS_NULL_PTR == pucStr)
     || (VOS_NULL_PTR == pucIpAddr))
    {
        AT_NORM_LOG("AT_ParseAddrFromPcscfIpv6Addr: input ptr is NULL, return ERROR");
        return VOS_ERR;
    }

    /* 遍历IPV6地址字符串 */
    if (VOS_OK != AT_Ipv6LenStrToAddrAccess(pucStr, &ucColonCount, &ucDotCount, &ucStrlen, &ucIdxPos))
    {
        AT_ERR_LOG("AT_ParseAddrFromPcscfIpv6Addr: AT_Ipv6LenStrToAddrAccess FAIL, return ERROR");
        return VOS_ERR;
    }

    /* 不支持IPV4IPV6混合型的格式 */
    if (0 != ucDotCount)
    {
        AT_ERR_LOG("AT_ParseAddrFromPcscfIpv6Addr: There have dot symbol in address format, return ERROR");
        return VOS_ERR;
    }

    /* 字符串为空返回失败 */
    if (0 == ucStrlen)
    {
        AT_ERR_LOG("AT_ParseAddrFromPcscfIpv6Addr: IP address length is 0, return ERROR");
        return VOS_ERR;
    }

    /* 冒号个数大于7则返回失败 */
    if (ucColonCount > AT_MAX_IPV6_STR_COLON_NUM)
    {
        AT_ERR_LOG("AT_ParseAddrFromPcscfIpv6Addr: IPV6 address Colon number is larger than 7, return ERROR");
        return VOS_ERR;
    }

    if (AT_MAX_IPV6_STR_COLON_NUM == ucColonCount)
    {
        /* 非压缩格式处理 */
        ulResult = AT_PcscfIpv6StrToAddr(pucStr, pucIpAddr, ucColonCount);
    }
    else
    {
        if (0xFF != ucIdxPos)
        {
            /* 压缩格式处理 */
            AT_Ipv6LenStrToAddrProcCompressed(pucStr, ucColonCount, ucDotCount, ucStrlen, ucIdxPos);
            /* 映射IPV6地址格式 */
            ulResult = AT_PcscfIpv6StrToAddr(pucStr, pucIpAddr, AT_MAX_IPV6_STR_COLON_NUM);
        }
        /* 压缩IPV6地址中找不到两个相邻的冒号，格式错误 */
        else
        {
            AT_ERR_LOG("AT_ParseAddrFromPcscfIpv6Addr: Can not find two consecutive colons in compressed IPV6 address , return ERROR");
            return VOS_ERR;
        }
    }

    return ulResult;
}


VOS_UINT32  AT_Ipv6PcscfDataToAddr(
    VOS_UINT8                          *pucStr,
    VOS_UINT8                          *pucIpAddr,
    VOS_UINT32                         *pulPortExistFlg,
    VOS_UINT32                         *pulPortNum

)
{
    VOS_UINT8                           pucStrTmp[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];

    TAF_MEM_SET_S(pucStrTmp, TAF_MAX_IPV6_ADDR_COLON_STR_LEN, 0, TAF_MAX_IPV6_ADDR_COLON_STR_LEN);

    if ((VOS_NULL_PTR == pucStr)
     || (VOS_NULL_PTR == pucIpAddr)
     || (VOS_NULL_PTR == pulPortExistFlg)
     || (VOS_NULL_PTR == pulPortNum))
    {
        AT_NORM_LOG("AT_Ipv6PcscfDataToAddr: input ptr is NULL, return ERROR");
        return VOS_ERR;
    }

    if (VOS_OK != AT_ParsePortFromPcscfIpv6Addr(pucStr, pucStrTmp, pulPortExistFlg, pulPortNum))
    {
        AT_ERR_LOG("AT_Ipv6PcscfDataToAddr: AT_ParsePortFromPcscfIpv6Addr FAIL, return ERROR");
        return VOS_ERR;
    }

    /* 将IPV6地址格式转换为大写 */
    VOS_StrToUpper((VOS_CHAR*)pucStrTmp);

    if (VOS_OK != AT_ParseAddrFromPcscfIpv6Addr(pucStrTmp, pucIpAddr))
    {
        AT_ERR_LOG("AT_Ipv6PcscfDataToAddr: AT_ParseAddrFromPcscfIpv6Addr FAIL, return ERROR");
        return VOS_ERR;
    }

    return VOS_OK;
}



VOS_UINT32 AT_Ipv6AddrToStr(
    VOS_UINT8                           aucAddrStr[],
    VOS_UINT8                           aucIpAddr[],
    AT_IPV6_STR_TYPE_ENUM_UINT8         enIpStrType
)
{
    VOS_UINT8                          *pucBuffer;
    VOS_UINT16                          ausAddrValue[AT_IPV6_STR_MAX_TOKENS];
    VOS_UINT16                          usAddrNum;
    VOS_UINT8                           ucDelimiter;
    VOS_UINT8                           ucTokensNum;
    VOS_UINT8                           ucRadix;
    VOS_UINT32                          i;

    pucBuffer                           = aucAddrStr;

    TAF_MEM_SET_S(ausAddrValue, sizeof(ausAddrValue), 0x00, sizeof(ausAddrValue));

    /* 根据IP字符串格式表达类型, 配置对应的转换参数 */
    switch (enIpStrType)
    {
        case AT_IPV6_STR_TYPE_HEX:
            ucDelimiter = AT_IPV6_STR_DELIMITER;
            ucTokensNum = 8;
            ucRadix     = 16;
            break;

        case AT_IPV6_STR_TYPE_DEC:
            ucDelimiter = AT_IPV4_STR_DELIMITER;
            ucTokensNum = 16;
            ucRadix     = 10;
            break;

        default:
            return VOS_ERR;
    }

    /* 根据IP字符串格式表达类型, 获取分段的IP地址数值 */
    for (i = 0; i < ucTokensNum; i++)
    {
        usAddrNum = *aucIpAddr++;

        if (AT_IPV6_STR_TYPE_HEX == enIpStrType)
        {
            usAddrNum <<= 8;
            usAddrNum  |= *aucIpAddr++;
        }

        ausAddrValue[i] = usAddrNum;
    }

    /* 遍历IP地址分段, 创建有分隔符标记的IP地址字符串 */
    for (i=0; i < ucTokensNum; i++)
    {
        pucBuffer    = (VOS_UINT8*)AT_Itoa(ausAddrValue[i],
                                               (VOS_CHAR*)pucBuffer,
                                               ucRadix,
                                               (TAF_MAX_IPV6_ADDR_DOT_STR_LEN - (VOS_UINT32)(aucAddrStr - pucBuffer)));
        *pucBuffer++ = ucDelimiter;
    }

    /* 取出最后一个分隔符, 补充字符串结束符 */
    if (aucAddrStr != pucBuffer)
    {
        *(--pucBuffer) = '\0';
    }

    return VOS_OK;
}


VOS_UINT32 AT_Ipv6Addr2DecString(
    VOS_CHAR                           *pcIpv6FormatStr,
    VOS_UINT8                           aucIpv6Addr[]
)
{
    VOS_UINT32                          ulLength;
    VOS_UINT32                          ulLoop;
    VOS_INT                             iRslt;

    ulLength = 0;

    /* 循环打印10进制点分IPv6地址 */
    for (ulLoop = 0; ulLoop < AT_IPV6_ADDR_DEC_TOKEN_NUM; ulLoop++)
    {
        /* 打印点分隔符 */
        if (0 != ulLoop)
        {
            *(pcIpv6FormatStr + ulLength) = AT_IP_STR_DOT_DELIMITER;
            ulLength ++;
        }

        iRslt = VOS_sprintf_s(pcIpv6FormatStr + ulLength,
                              AT_IPV6_ADDR_DEC_FORMAT_STR_LEN - ulLength,
                              "%d",
                              aucIpv6Addr[ulLoop]);

        if (iRslt <= 0)
        {
            AT_WARN_LOG("AT_Ipv6Addr2DecString: Print IPv6 Addr Failed!");
            return 0;
        }

        ulLength += iRslt;
    }

    return ulLength;
}


VOS_VOID AT_ConvertIpv6AddrToHexAddrAndGetMaxZeroCnt(
    VOS_UINT8                           aucIpv6Addr[],
    VOS_UINT16                          ausIpv6HexAddr[],
    VOS_UINT32                         *pulZeroStartIndex,
    VOS_UINT32                         *pulZeroMaxCnt
)
{
    VOS_UINT32                          ulLoop;
    VOS_UINT32                          ulZeroTmpIndex;
    VOS_UINT32                          ulZeroTmpCnt;

    ulZeroTmpIndex      = 0;
    ulZeroTmpCnt        = 0;

    /* 循环转换IPv6格式地址，并统计最长连续零段 */
    for (ulLoop = 0; ulLoop < AT_IPV6_ADDR_HEX_TOKEN_NUM; ulLoop++)
    {
        /* 合并字节 */
        ausIpv6HexAddr[ulLoop] = *(aucIpv6Addr + ulLoop + ulLoop);
        ausIpv6HexAddr[ulLoop] <<= 8;
        ausIpv6HexAddr[ulLoop] |= *(aucIpv6Addr + ulLoop + ulLoop + 1);

        if (0 == ausIpv6HexAddr[ulLoop])
        {
            /* 如果16进制字段为0，则记录临时cnt和Index */
            if (0 == ulZeroTmpCnt)
            {
                ulZeroTmpIndex = ulLoop;
            }

            ulZeroTmpCnt++;
        }
        else
        {
            /* 如果16进制字段不为0，则判断是否更新最长连续零记录 */
            if (ulZeroTmpCnt > *pulZeroMaxCnt)
            {
                *pulZeroStartIndex    = ulZeroTmpIndex;
                *pulZeroMaxCnt        = ulZeroTmpCnt;
            }

            ulZeroTmpCnt    = 0;
            ulZeroTmpIndex  = 0;
        }
    }


    /* 判断是否更新最长连续零记录 */
    if (ulZeroTmpCnt > *pulZeroMaxCnt)
    {
        *pulZeroStartIndex    = ulZeroTmpIndex;
        *pulZeroMaxCnt        = ulZeroTmpCnt;
    }

    return;
}


VOS_UINT32 AT_Ipv6Addr2HexString(
    VOS_CHAR                           *pcIpv6FormatStr,
    VOS_UINT8                           aucIpv6Addr[]
)
{
    VOS_UINT32                          ulLength;
    AT_COMM_PS_CTX_STRU                *pstCommPsCtx        = VOS_NULL_PTR;
    VOS_CHAR                           *pcFormatStr         = VOS_NULL_PTR;
    VOS_UINT32                          ulLoop;
    VOS_UINT16                          ausIpv6HexAddr[AT_IPV6_ADDR_HEX_TOKEN_NUM];
    VOS_UINT32                          ulZeroStartIndex;
    VOS_UINT32                          ulZeroMaxCnt;
    VOS_INT                             iRslt;

    /* 局部变量初始化 */
    ulLength            = 0;
    pstCommPsCtx        = AT_GetCommPsCtxAddr();
    ulZeroStartIndex    = 0;
    ulZeroMaxCnt        = 0;
    TAF_MEM_SET_S(ausIpv6HexAddr, sizeof(ausIpv6HexAddr), 0, sizeof(ausIpv6HexAddr));

    /* 根据是否保留前导零确认打印格式 */
    pcFormatStr = (VOS_FALSE == pstCommPsCtx->bitOpIpv6LeadingZeros) ? "%04X" : "%X";

    /* 转换IPv6地址为16Bit HEX类型，并统计最大连续零个数 */
    AT_ConvertIpv6AddrToHexAddrAndGetMaxZeroCnt(aucIpv6Addr, ausIpv6HexAddr, &ulZeroStartIndex, &ulZeroMaxCnt);

    /* 循环打印16进制点分IPv6地址 */
    for (ulLoop = 0; ulLoop < AT_IPV6_ADDR_HEX_TOKEN_NUM; ulLoop++)
    {
        /* 开启零压缩功能，且存在最大连续零，则压缩打印 */
        if ( (VOS_FALSE != pstCommPsCtx->bitOpIpv6CompressZeros)
          && (ulZeroMaxCnt > 0) )
        {
            /* 第一个0，打印冒号 */
            if (ulZeroStartIndex == ulLoop)
            {
                *(pcIpv6FormatStr + ulLength) = AT_IP_STR_COLON_DELIMITER;
                ulLength ++;
                continue;
            }

            /* 其余0不打印 */
            if ( (ulLoop > ulZeroStartIndex)
              && (ulLoop < (ulZeroStartIndex + ulZeroMaxCnt)) )
            {
                /* 最后一位为0，需要多打印一个冒号 */
                if (ulLoop == (AT_IPV6_ADDR_HEX_TOKEN_NUM - 1))
                {
                    *(pcIpv6FormatStr + ulLength) = AT_IP_STR_COLON_DELIMITER;
                    ulLength ++;
                }

                continue;
            }
        }

        /* 打印冒号分隔符 */
        if (0 != ulLoop)
        {
            *(pcIpv6FormatStr + ulLength) = AT_IP_STR_COLON_DELIMITER;
            ulLength ++;
        }

        iRslt = VOS_sprintf_s(pcIpv6FormatStr + ulLength,
                              AT_IPV6_ADDR_COLON_FORMAT_STR_LEN - ulLength,
                              pcFormatStr,
                              ausIpv6HexAddr[ulLoop]);

        if (iRslt <= 0)
        {
            AT_WARN_LOG("AT_Ipv6Addr2HexString: Print IPv6 Addr Failed!");
            return 0;
        }

        ulLength += iRslt;
    }

    return ulLength;
}


VOS_UINT32 AT_Ipv6AddrMask2FormatString(
    VOS_CHAR                           *pcIpv6FormatStr,
    VOS_UINT8                           aucIpv6Addr[],
    VOS_UINT8                           aucIpv6Mask[]
)
{
    VOS_UINT32                          ulLength;
    AT_COMM_PS_CTX_STRU                *pstCommPsCtx        = VOS_NULL_PTR;
    VOS_INT                             iRslt;

    /* 局部变量初始化 */
    ulLength        = 0;
    pstCommPsCtx    = AT_GetCommPsCtxAddr();
    iRslt           = 0;

    /* 必须要有IPv6地址，不单独打印IPv6子网掩码 */
    if (VOS_NULL_PTR == aucIpv6Addr)
    {
        AT_WARN_LOG("AT_Ipv6AddrMask2FormatString: No IPv6 Address!");
        *pcIpv6FormatStr = '\0';
        return 0;
    }

    if (VOS_FALSE == pstCommPsCtx->bitOpIpv6AddrFormat)
    {
        /* 10进制点分格式打印IPv6地址 */
        ulLength = AT_Ipv6Addr2DecString(pcIpv6FormatStr, aucIpv6Addr);

        /* 打印子网掩码 */
        if (VOS_NULL_PTR != aucIpv6Mask)
        {
            /* 使用点分隔IP地址与子网掩码 */
            *(pcIpv6FormatStr + ulLength) = AT_IP_STR_DOT_DELIMITER;
            ulLength ++;

            /* 10进制点分格式打印IPv6子网掩码 */
            ulLength += AT_Ipv6Addr2DecString(pcIpv6FormatStr + ulLength, aucIpv6Mask);
        }
    }
    else
    {
        /* 16进制冒号分隔格式打印IPv6地址 */
        ulLength = AT_Ipv6Addr2HexString(pcIpv6FormatStr, aucIpv6Addr);

        /* 打印子网掩码 */
        if (VOS_NULL_PTR != aucIpv6Mask)
        {
            /* 判断子网掩码格式 */
            if (VOS_FALSE == pstCommPsCtx->bitOpIpv6SubnetNotation)
            {
                /* 完整子网掩码地址与IPv6地址通过空格分隔 */
                /* 使用点分隔IP地址与子网掩码 */
                *(pcIpv6FormatStr + ulLength) = ' ';
                ulLength ++;

                /* 10进制点分格式打印IPv6子网掩码 */
                ulLength += AT_Ipv6Addr2HexString(pcIpv6FormatStr + ulLength, aucIpv6Mask);
            }
            else
            {
                /* 斜线分隔子网前缀无类域与IPv6地址 */
                /* 使用斜线分隔IP地址与子网掩码 */
                *(pcIpv6FormatStr + ulLength) = '/';
                ulLength ++;

                iRslt = VOS_sprintf_s(pcIpv6FormatStr + ulLength,
                                      AT_IPV6_ADDR_MASK_FORMAT_STR_LEN - ulLength,
                                      "%d",
                                      AT_CalcIpv6PrefixLength(aucIpv6Mask));
                if (iRslt <= 0)
                {
                    AT_WARN_LOG("AT_Ipv6AddrMask2FormatString: Print IPv6 Subnet Failed!");
                    *pcIpv6FormatStr = '\0';
                    return 0;
                }

                ulLength += iRslt;
            }
        }
    }

    /* 最后补充字符串结束符 */
    *(pcIpv6FormatStr + ulLength) = '\0';

    return ulLength;
}


VOS_UINT8 AT_CalcIpv6PrefixLength(VOS_UINT8 *pucLocalIpv6Mask)
{
    VOS_UINT32                          i                   = 0;
    VOS_UINT8                           j                   = 0;
    VOS_UINT8                           ucMaskLength        = 0;

    if ( VOS_NULL_PTR == pucLocalIpv6Mask )
    {
        return ucMaskLength;
    }

    for(i = 0; i < AT_IPV6_STR_MAX_TOKENS; i++)
    {
        if (0xFF == *(pucLocalIpv6Mask + i))
        {
            ucMaskLength = ucMaskLength + 8;
        }
        else
        {
            for (j = 0; j < 8; j++)
            {
                if (0 != ((*(pucLocalIpv6Mask + i)) & ((VOS_UINT32)1 << (7 - j))))
                {
                    ucMaskLength ++;
                }
                else
                {
                    break;
                }
            }
            break;
        }
    }

    return ucMaskLength;
}


VOS_VOID AT_GetIpv6MaskByPrefixLength(
    VOS_UINT8                       ucLocalIpv6Prefix,
    VOS_UINT8                      *pucLocalIpv6Mask
)
{
    VOS_UINT8                           ucNum                   = 0;
    VOS_UINT8                           i                       = 0;

    ucNum = ucLocalIpv6Prefix/8;

    if ( VOS_NULL_PTR == pucLocalIpv6Mask )
    {
        return;
    }

    for (i = 0; i < ucNum; i ++)
    {
        *(pucLocalIpv6Mask + i) = 0xFF;
    }

    ucNum = ucLocalIpv6Prefix % 8;

    if (0 != ucNum)
    {
        *(pucLocalIpv6Mask + i) = 0xFF & ((VOS_UINT32)0xFF << (8 - ucNum));

    }

    return;
}


VOS_UINT32 AT_FindIpv6AddrZeroFieldsToBeCompressed(
    VOS_UINT8                          *pucZeroFieldStart,
    VOS_UINT8                          *pucZeroFieldCount,
    VOS_UINT16                          ausAddrValue[],
    VOS_UINT8                           ucTokensNum
)
{
    VOS_UINT8                           ucStart;
    VOS_UINT8                           ucCount;
    VOS_UINT8                           i;

    ucStart                             = 0xFF;
    ucCount                             = 0;

    for (i = 0; i < ucTokensNum - 1; i++)
    {
        if ( (0x0000 == ausAddrValue[i])
          && (0x0000 == ausAddrValue[i + 1]) )
        {
            /* 记录数值连续为0的IP地址段起始位置 */
            if (0xFF == ucStart)
            {
                ucStart = i;
            }

            /* 更新数值连续为0的IP地址段个数 */
            ucCount++;
        }
        else
        {
            /* 更新待压缩的IP地址段位置, 以及IP地址段个数 */
            if (0xFF != ucStart)
            {
                if (ucCount > *pucZeroFieldCount)
                {
                    *pucZeroFieldStart = ucStart;
                    *pucZeroFieldCount = ucCount;
                }

                ucStart = 0xFF;
                ucCount = 0;
            }
        }
    }

    /* 数值连续为0的IP地址段在结尾时, 需要更新一次待压缩的IP地址段位置,
       以及IP地址段个数 */
    if (0xFF != ucStart)
    {
        if (ucCount > *pucZeroFieldCount)
        {
            *pucZeroFieldStart = ucStart;
            *pucZeroFieldCount = ucCount;
        }
    }

    return VOS_OK;
}


VOS_UINT32 AT_ConvertIpv6AddrToCompressedStr(
    VOS_UINT8                           aucAddrStr[],
    VOS_UINT8                           aucIpAddr[],
    VOS_UINT8                           ucTokensNum
)
{
    VOS_UINT8                          *pucBuffer;
    VOS_UINT16                          ausAddrValue[8]; /*TAF_IPV6_STR_RFC2373_TOKENS]; */
    VOS_UINT16                          usAddrNum;
    VOS_UINT8                           ucDelimiter;
    VOS_UINT8                           ucRadix;
    VOS_UINT8                           ucZeroFieldStart;
    VOS_UINT8                           ucZeroFieldCount;
    VOS_UINT8                           i;

    TAF_MEM_SET_S(ausAddrValue, sizeof(ausAddrValue), 0x00, sizeof(ausAddrValue));

    pucBuffer                           = aucAddrStr;
    ucDelimiter                         = TAF_IPV6_STR_DELIMITER;
    ucRadix                             = 16;
    ucZeroFieldStart                    = 0xFF;
    ucZeroFieldCount                    = 0;

    /* 根据IP字符串格式表达类型, 获取分段的IP地址数值 */
    for (i = 0; i < ucTokensNum; i++)
    {
        usAddrNum = *aucIpAddr++;

        usAddrNum <<= 8;
        usAddrNum  |= *aucIpAddr++;

        ausAddrValue[i] = usAddrNum;
    }

    /* 找出需要使用"::"表示的IP地址段的起始位置  */
    AT_FindIpv6AddrZeroFieldsToBeCompressed(&ucZeroFieldStart,
                                            &ucZeroFieldCount,
                                            ausAddrValue,
                                            ucTokensNum);

    /* 遍历IP地址分段, 创建有分隔符标记的IP地址字符串 */
    for (i=0; i < ucTokensNum; i++)
    {
        if (ucZeroFieldStart == i)
        {
            *pucBuffer++ = ucDelimiter;

            i += ucZeroFieldCount;

            /* 如果已到IP地址分段的最后一段, 需要补充分隔符 */
            if ((ucTokensNum - 1) == i)
            {
                *pucBuffer++ = ucDelimiter;
            }
        }
        else
        {
            /* 如果是IP地址分段的第一段, 不需要补充分隔符 */
            if (0 != i)
            {
                *pucBuffer++ = ucDelimiter;
            }
            pucBuffer    = (VOS_UINT8*)AT_Itoa(ausAddrValue[i],
                                               (VOS_CHAR*)pucBuffer,
                                               ucRadix,
                                               (TAF_MAX_IPV6_ADDR_COLON_STR_LEN - (VOS_UINT32)(aucAddrStr - pucBuffer)));
        }
    }

    /* 补充字符串结束符 */
    if (aucAddrStr != pucBuffer)
    {
        *pucBuffer = '\0';
    }

    return VOS_OK;
}


VOS_UINT32  AT_LenStr2IpAddr(
    VOS_UINT8                           *pucStr,
    VOS_UINT8                           *pucIpAddr
)
{
    VOS_UINT8 i;
    VOS_UINT8 j;
    VOS_UINT8 ucValue;

    i                = 0;
    j                = 0;
    ucValue          = 0;

    if (0 == pucStr[0])
    {
       /*ADDR长度为0，直接长度赋值返回*/
        pucIpAddr[0] = 0;
        pucIpAddr[1] = 0;
        pucIpAddr[2] = 0;
        pucIpAddr[3] = 0;
        return AT_SUCCESS;
    }

    if (pucStr[0] == '.')
    {
        /*如果第1个有效字符是'.'，IP地址是非法的*/
        return AT_FAILURE;
    }

    for (i = 0; (i <= AT_AP_MAX_IPV4_ADDR_LEN) && (0 != pucStr[i]) ; i++)
    {
        /*从第1个有效字符开始检查*/
        if (((pucStr[i] < 0x30) || (pucStr[i] > 0x39)) && (pucStr[i] != '.'))
        {
            /*超出'0'-'9'的字符非法*/
            return AT_FAILURE;
        }
        if (pucStr[i] != '.')
        {
            /*如果是有效字符，转化为数字*/
            if (((ucValue * 10) + (pucStr[i] - 0x30)) <= 255)
            {
                /*字符串转化为有效IP段位值*/
                ucValue = (TAF_UINT8)((ucValue * 10) + (pucStr[i] - 0x30));
            }
            else
            {
                /*超过255出错*/
                return AT_FAILURE;
            }
        }
        else
        {   /*如果字符是'.'，前一位段值已经计算出来*/
            /*如果字符是'.'，前一位段值已经计算出来*/
            if (j <= 3)
            {
               /*本版本只支持IPV4地址*/
                pucIpAddr[j] = ucValue;
                ucValue      = 0;

                /*开始下一个有效字符段的长度累计*/
                j++;
            }
            else
            {
                /*超出4个IP位段，非法*/
                return AT_FAILURE;
            }
        }
    }

    if (j == 3)
    {
        pucIpAddr[j] = ucValue;
        return AT_SUCCESS;
    }
    else
    {
        return AT_FAILURE;
    }
}


VOS_UINT32  AT_PortAtoI(
    VOS_CHAR                           *pcString,
    VOS_UINT32                         *pulValue
)
{
    VOS_CHAR                           *pucTmp;
    VOS_UINT32                          ulRet;

    if ((VOS_NULL_PTR == pcString)
     || (VOS_NULL_PTR == pulValue))
    {
        AT_ERR_LOG("AT_PortAtoI: pcString or pulValue is NULL, return ERROR");
        return VOS_ERR;
    }

    pucTmp  = pcString;
    ulRet   = 0;

    *pulValue = 0;

    for (pucTmp = pcString; *pucTmp != '\0'; pucTmp++)
    {
        /* 非数字, 则返回失败*/
        if (('0' > *pucTmp) || ('9' < *pucTmp))
        {
            AT_ERR_LOG("AT_PortAtoI: Not all number type in pcString , return ERROR");
            return VOS_ERR;
        }

        ulRet = (ulRet * 10) + (*pucTmp - '0');

        if (IMS_PCSCF_ADDR_PORT_MAX < ulRet)
        {
            AT_ERR_LOG("AT_PortAtoI: Port number is larger than 65535, return ERROR");
            return VOS_ERR;
        }
    }

    if ((0 < ulRet)
     && (IMS_PCSCF_ADDR_PORT_MAX >= ulRet))
    {
        *pulValue = (VOS_UINT32)ulRet;

        return VOS_OK;
    }

    AT_ERR_LOG("AT_PortAtoI: return ERROR");
    return VOS_ERR;
}


VOS_UINT64  AT_AtoI(
    VOS_UINT8                         *pString
)
{
    VOS_UINT8   *pucTmp;
    VOS_UINT64   ullRet;

    pucTmp  = pString;
    ullRet  = 0;

    for (pucTmp = pString ; *pucTmp != '\0' ; pucTmp++)
    {
        /* 非数字,则不处理*/
        if ((*pucTmp < '0') || (*pucTmp > '9'))
        {
            continue;
        }

        ullRet = (ullRet * 10) + (*pucTmp - '0');
    }

    return ullRet;
}


VOS_CHAR* AT_Itoa(
    VOS_UINT16                          usValue,
    VOS_CHAR                           *pcStr,
    VOS_UINT16                          usRadix,
    VOS_UINT32                          ulLength
)
{
    if (usRadix == 16)
    {
        pcStr += VOS_sprintf_s(pcStr, ulLength, "%x", usValue);
    }
    else if(usRadix == 10)
    {
        pcStr += VOS_sprintf_s(pcStr, ulLength, "%d", usValue);
    }
    else
    {
    }
    return pcStr;
}


VOS_INT32  AT_AtoInt(
    VOS_UINT8                          *pString,
    VOS_INT32                          *pOut
)
{
    VOS_UINT8                          *pucTmp;
    VOS_INT32                           lFlag;     /* negative flag */

    pucTmp  = pString;
    lFlag   = 0;

    if ('-' == *pucTmp)
    {
        lFlag = VOS_TRUE;
        pucTmp ++;
    }

    for (; *pucTmp != '\0' ; pucTmp++)
    {
        /* 非数字, 直接返回错误 */
        if ((*pucTmp < '0') || (*pucTmp > '9'))
        {
            return VOS_ERR;
        }

        *pOut = (*pOut * 10) + (*pucTmp - '0');
    }

    if (VOS_TRUE == lFlag)
    {
        *pOut = (0 - (*pOut));
    }

    return VOS_OK;
}


VOS_VOID AT_GetDhcpPara(
    AT_DHCP_PARA_STRU                  *pstConfig,
    AT_IPV4_DHCP_PARAM_STRU            *pstIpv4Dhcp
)
{
    /* 将DHCP参数转换为网络序 */
    pstConfig->stDhcpCfg.ulIPAddr     = VOS_HTONL(pstIpv4Dhcp->ulIpv4Addr);
    pstConfig->stDhcpCfg.ulSubNetMask = VOS_HTONL(pstIpv4Dhcp->ulIpv4NetMask);
    pstConfig->stDhcpCfg.ulGateWay    = VOS_HTONL(pstIpv4Dhcp->ulIpv4GateWay);
    pstConfig->stDhcpCfg.ulPrimDNS    = VOS_HTONL(pstIpv4Dhcp->ulIpv4PrimDNS);
    pstConfig->stDhcpCfg.ulSndDNS     = VOS_HTONL(pstIpv4Dhcp->ulIpv4SecDNS);

    return;
}


VOS_UINT16 AT_CalcIpHdrCRC16(
    VOS_UINT8                          *pucData,
    VOS_UINT16                          usSize
)
{
    VOS_UINT16                         *pusBuffer;
    VOS_UINT32                          ulCheckSum;

    pusBuffer    = (VOS_UINT16 *)pucData;
    ulCheckSum = 0;

    while (usSize > 1)
    {
        ulCheckSum += *pusBuffer++;
        usSize     -= sizeof(VOS_UINT16);
    }

    if (usSize)
    {
        ulCheckSum += *(VOS_UINT8 *)pusBuffer;
    }

    ulCheckSum  = (ulCheckSum >> 16) + (ulCheckSum & 0xffff);
    ulCheckSum += (ulCheckSum >> 16);

    return (VOS_UINT16)(~ulCheckSum);
}


VOS_UINT32 AT_BuildUdpHdr(
    AT_UDP_PACKET_FORMAT_STRU          *pstUdpPkt,
    VOS_UINT16                          usLen,
    VOS_UINT32                          ulSrcAddr,
    VOS_UINT32                          ulDstAddr,
    VOS_UINT16                          usSrcPort,
    VOS_UINT16                          usDstPort
)
{
    static VOS_UINT16                   usIdentification = 0;

    /* 检查指针合法性 */
    if (VOS_NULL_PTR == pstUdpPkt)
    {
        return VOS_ERR;
    }

    /* 填写IP头 */
    pstUdpPkt->stIpHdr.ucIpVer          = AT_IP_VERSION;
    pstUdpPkt->stIpHdr.ucIpHdrLen       = 5;
    pstUdpPkt->stIpHdr.ucServiceType    = 0x00;
    pstUdpPkt->stIpHdr.usTotalLen       = VOS_HTONS(usLen + AT_IP_HDR_LEN + AT_UDP_HDR_LEN);
    pstUdpPkt->stIpHdr.usIdentification = VOS_HTONS(usIdentification);
    pstUdpPkt->stIpHdr.usOffset         = 0;
    pstUdpPkt->stIpHdr.ucTTL            = AT_IP_DEF_TTL;
    pstUdpPkt->stIpHdr.ucProtocol       = AT_IPPROTO_UDP;
    pstUdpPkt->stIpHdr.ulSrcAddr        = VOS_HTONL(ulSrcAddr);
    pstUdpPkt->stIpHdr.ulDstAddr        = VOS_HTONL(ulDstAddr);
    pstUdpPkt->stIpHdr.usCheckSum       = AT_CalcIpHdrCRC16((VOS_UINT8 *)&pstUdpPkt->stIpHdr, AT_IP_HDR_LEN);

    /* 填写UDP头 */
    pstUdpPkt->stUdpHdr.usSrcPort       = VOS_HTONS(usSrcPort);
    pstUdpPkt->stUdpHdr.usDstPort       = VOS_HTONS(usDstPort);
    pstUdpPkt->stUdpHdr.usLen           = VOS_HTONS(usLen + AT_UDP_HDR_LEN);
    pstUdpPkt->stUdpHdr.usChecksum      = 0;

    usIdentification++;

    return VOS_OK;
}


VOS_UINT32 AT_GetDisplayRate(
    VOS_UINT16                          usClientId,
    AT_DISPLAY_RATE_STRU               *pstSpeed
)
{
    PPP_RATE_DISPLAY_ENUM_UINT32        enRateDisplay;
    AT_DISPLAY_RATE_STRU                stDialRateTmp;
    TAF_AGENT_SYS_MODE_STRU             stSysMode;
    VOS_UINT32                          ulRet;
    VOS_UINT32                          ulDlSpeedLen      = 0;
    VOS_UINT32                          ulUlSpeedLen      = 0;
    VOS_UINT32                          ulNvDialRateIndex;
    VOS_UINT8                           ucSubSysMode;
    VOS_UINT8                           ucDlCategoryIndex = 0;
    VOS_UINT8                           ucUlCategoryIndex = 0;

    /* 变量初始化 */
    TAF_MEM_SET_S(&stSysMode, sizeof(stSysMode), 0x00, sizeof(stSysMode));
    TAF_MEM_SET_S(&stDialRateTmp, sizeof(AT_DISPLAY_RATE_STRU), 0x00, sizeof(AT_DISPLAY_RATE_STRU));

    /* 从C核获取ucRatType和ucSysSubMode */
    ulRet = TAF_AGENT_GetSysMode(usClientId, &stSysMode);

    if (VOS_OK != ulRet)
    {
        stSysMode.enRatType     = TAF_PH_INFO_GSM_RAT;
        stSysMode.enSysSubMode  = TAF_SYS_SUBMODE_BUTT;
    }

    ucSubSysMode = stSysMode.enSysSubMode;
    switch(ucSubSysMode)
    {
        case TAF_SYS_SUBMODE_GSM:
        case TAF_SYS_SUBMODE_GPRS:
            ulNvDialRateIndex =    g_stDialConnectDisplayRate.ucGprsConnectRate;
            break;

        case TAF_SYS_SUBMODE_EDGE:
            ulNvDialRateIndex =    g_stDialConnectDisplayRate.ucEdgeConnectRate;
            break;

        case TAF_SYS_SUBMODE_WCDMA:
            ulNvDialRateIndex =    g_stDialConnectDisplayRate.ucWcdmaConnectRate;
            break;

        case TAF_SYS_SUBMODE_HSDPA:
        case TAF_SYS_SUBMODE_HSDPA_HSUPA:
            ulNvDialRateIndex =    g_stDialConnectDisplayRate.ucDpaConnectRate;
            break;

        default:
            ulNvDialRateIndex = 0;
            break;
    }

    if ((0 == ulNvDialRateIndex) || (ulNvDialRateIndex > 6))
    {
        if ( (TAF_PH_INFO_WCDMA_RAT == stSysMode.enRatType)
          || (TAF_PH_INFO_TD_SCDMA_RAT == stSysMode.enRatType) )
        {
            enRateDisplay = AT_GetRateDisplayIndexForWcdma(&g_stAtDlRateCategory);

            ulDlSpeedLen = VOS_StrLen((TAF_CHAR *)g_PppDialRateDisplay[enRateDisplay]);
            ulUlSpeedLen = VOS_StrLen((TAF_CHAR *)g_PppDialRateDisplay[enRateDisplay]);
            TAF_MEM_CPY_S(stDialRateTmp.ucDlSpeed, (VOS_SIZE_T)sizeof(stDialRateTmp.ucDlSpeed), g_PppDialRateDisplay[enRateDisplay], ulDlSpeedLen);
            TAF_MEM_CPY_S(stDialRateTmp.ucUlSpeed, (VOS_SIZE_T)sizeof(stDialRateTmp.ucUlSpeed), g_PppDialRateDisplay[enRateDisplay], ulUlSpeedLen);
        }
        else if (TAF_PH_INFO_LTE_RAT == stSysMode.enRatType)
        {
            ucDlCategoryIndex = AT_GetLteUeDlCategoryIndex();
            ucUlCategoryIndex = AT_GetLteUeUlCategoryIndex();

            ulDlSpeedLen = VOS_StrLen((TAF_CHAR *)(g_ucLTERateDisplay[ucDlCategoryIndex].acStrDlSpeed));
            TAF_MEM_CPY_S(stDialRateTmp.ucDlSpeed, (VOS_SIZE_T)sizeof(stDialRateTmp.ucDlSpeed), g_ucLTERateDisplay[ucDlCategoryIndex].acStrDlSpeed, ulDlSpeedLen);
            ulUlSpeedLen = VOS_StrLen((TAF_CHAR *)(g_ucLTERateDisplay[ucUlCategoryIndex].acStrUlSpeed));
            TAF_MEM_CPY_S(stDialRateTmp.ucUlSpeed, (VOS_SIZE_T)sizeof(stDialRateTmp.ucUlSpeed), g_ucLTERateDisplay[ucUlCategoryIndex].acStrUlSpeed, ulUlSpeedLen);
        }
        else
        {
            enRateDisplay = AT_GetRateDisplayIndexForGsm(&stSysMode);

            ulDlSpeedLen = VOS_StrLen((TAF_CHAR *)g_PppDialRateDisplay[enRateDisplay]);
            ulUlSpeedLen = VOS_StrLen((TAF_CHAR *)g_PppDialRateDisplay[enRateDisplay]);
            TAF_MEM_CPY_S(stDialRateTmp.ucDlSpeed, (VOS_SIZE_T)sizeof(stDialRateTmp.ucDlSpeed), g_PppDialRateDisplay[enRateDisplay], ulDlSpeedLen);
            TAF_MEM_CPY_S(stDialRateTmp.ucUlSpeed, (VOS_SIZE_T)sizeof(stDialRateTmp.ucUlSpeed), g_PppDialRateDisplay[enRateDisplay], ulUlSpeedLen);
        }
    }
    else
    {
        ulDlSpeedLen = VOS_StrLen((TAF_CHAR *)g_ucDialRateDisplayNv[ulNvDialRateIndex - 1]);
        ulUlSpeedLen = VOS_StrLen((TAF_CHAR *)g_ucDialRateDisplayNv[ulNvDialRateIndex - 1]);
        TAF_MEM_CPY_S(stDialRateTmp.ucDlSpeed, (VOS_SIZE_T)sizeof(stDialRateTmp.ucDlSpeed), g_ucDialRateDisplayNv[ulNvDialRateIndex - 1], ulDlSpeedLen);
        TAF_MEM_CPY_S(stDialRateTmp.ucUlSpeed, (VOS_SIZE_T)sizeof(stDialRateTmp.ucUlSpeed), g_ucDialRateDisplayNv[ulNvDialRateIndex - 1], ulUlSpeedLen);
    }

    /*在CONNECT后附上速率信息*/
    TAF_MEM_CPY_S(pstSpeed->ucDlSpeed, AT_AP_SPEED_STRLEN + 1, stDialRateTmp.ucDlSpeed, (VOS_UINT16)ulDlSpeedLen);
    TAF_MEM_CPY_S(pstSpeed->ucUlSpeed, AT_AP_SPEED_STRLEN + 1, stDialRateTmp.ucUlSpeed, (VOS_UINT16)ulUlSpeedLen);
    pstSpeed->ucDlSpeed[ulDlSpeedLen] = '\0';
    pstSpeed->ucUlSpeed[ulUlSpeedLen] = '\0';

    return VOS_OK;
}


VOS_UINT8* AT_PutNetworkAddr32(
  VOS_UINT8                            *pucAddr,
  VOS_UINT32                            ulAddr
)
{
    *pucAddr = (ulAddr >> 24);
    pucAddr++;

    *pucAddr = ((ulAddr >> 16) & 0xFF);
    pucAddr++;

    *pucAddr = ((ulAddr >> 8) & 0xFF);
    pucAddr++;

    *pucAddr = (ulAddr & 0xFF);
    pucAddr++;

    return pucAddr;
}


VOS_UINT32 AT_Get3gppSmCauseByPsCause(
    TAF_PS_CAUSE_ENUM_UINT32            enCause
)
{
    VOS_UINT32                          ul3gppSmCause;

    if ( (enCause >= TAF_PS_CAUSE_SM_NW_SECTION_BEGIN)
      && (enCause <= TAF_PS_CAUSE_SM_NW_SECTION_END) )
    {
        ul3gppSmCause = enCause - TAF_PS_CAUSE_SM_NW_SECTION_BEGIN;
    }
    /* E5、闪电卡在PDP DEACTIVE时上报网侧的36号原因值 */
    else if (TAF_PS_CAUSE_SUCCESS == enCause)
    {
        ul3gppSmCause = TAF_PS_CAUSE_SM_NW_REGULAR_DEACTIVATION - TAF_PS_CAUSE_SM_NW_SECTION_BEGIN;
    }
    else
    {
        ul3gppSmCause = AT_NDISSTAT_ERR_UNKNOWN;
    }

    return ul3gppSmCause;
}


VOS_VOID AT_NDIS_ConnStatusChgProc(NCM_IOCTL_CONNECT_STUS_E enStatus)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;
    VOS_UINT32                          ulCallId;

    if (NCM_IOCTL_STUS_BREAK == enStatus)
    {
        for (ulCallId = AT_PS_NDIS_CALL_ID_BEGIN; ulCallId <= AT_PS_NDIS_CALL_ID_END; ulCallId++)
        {
            pstCallEntity = AT_PS_GetCallEntity(AT_NDIS_GET_CLIENT_ID(), (VOS_UINT8)ulCallId);
            (VOS_VOID)AT_PS_HangupCall(pstCallEntity->stUserInfo.enUserIndex, (VOS_UINT8)ulCallId, TAF_PS_CALL_END_CAUSE_NORMAL);
        }
    }

    return;
}


VOS_UINT32 AT_ResetFlowCtl(
    VOS_UINT32                          ulParam1,
    VOS_UINT32                          ulParam2
)
{
    VOS_UINT8                           ucRabIdIndex;
    VOS_UINT32                          ulRabIdMask;
    VOS_UINT32                          ulRet;
    FC_ID_ENUM_UINT8                    enFcId;
    MODEM_ID_ENUM_UINT16                enModemId;

    enFcId      = (FC_ID_ENUM_UINT8)ulParam2;
    enModemId   = g_stFcIdMaptoFcPri[enFcId].enModemId;
    ulRabIdMask = g_stFcIdMaptoFcPri[enFcId].ulRabIdMask;

    if (VOS_TRUE == g_stFcIdMaptoFcPri[enFcId].ulUsed)
    {
        for (ucRabIdIndex = AT_PS_MIN_RABID; ucRabIdIndex <= AT_PS_MAX_RABID; ucRabIdIndex++)
        {
            if (1 == ((ulRabIdMask >> ucRabIdIndex) & 0x1))
            {
                FC_ChannelMapDelete(ucRabIdIndex, enModemId);
            }
        }

        ulRet = FC_DeRegPoint(enFcId, enModemId);
        if (VOS_OK != ulRet)
        {
            AT_ERR_LOG("AT_ResetFlowCtl: ERROR: DeReg point failed.");
            return VOS_ERR;
        }

        g_stFcIdMaptoFcPri[enFcId].ulUsed       = VOS_FALSE;
        g_stFcIdMaptoFcPri[enFcId].enFcPri      = FC_PRI_BUTT;
        g_stFcIdMaptoFcPri[enFcId].ulRabIdMask  = 0;
        g_stFcIdMaptoFcPri[enFcId].enModemId    = MODEM_ID_BUTT;

    }

    return VOS_OK;
}


FC_PRI_ENUM_UINT8 AT_GetFCPriFromQos(
    TAF_UMTS_QOS_STRU                  *pstUmtsQos
)
{

    FC_PRI_ENUM_UINT8                   enFCPri;
    VOS_UINT8                           ucTrafficClass;

    /* 初始化 */
    enFCPri         = FC_PRI_FOR_PDN_NONGBR;

    ucTrafficClass              = pstUmtsQos->ucTrafficClass;

    /* 根据QOS trafficClass参数获取QCI */
    if (AT_QOS_TRAFFIC_CLASS_CONVERSATIONAL == ucTrafficClass)
    {
        enFCPri = FC_PRI_FOR_PDN_GBR;
    }
    else if (AT_QOS_TRAFFIC_CLASS_STREAMING == ucTrafficClass)
    {
        enFCPri = FC_PRI_FOR_PDN_GBR;
    }
    else if (AT_QOS_TRAFFIC_CLASS_INTERACTIVE == ucTrafficClass)
    {
        enFCPri = FC_PRI_FOR_PDN_NONGBR;
    }
    else
    {
        enFCPri = FC_PRI_FOR_PDN_LOWEST;
    }

    return enFCPri;
}


VOS_VOID  AT_ModemPsRspPdpDeactivatedEvtProc(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent
)
{
    AT_UART_LINE_CTRL_STRU             *pstLineCtrl = VOS_NULL_PTR;
    VOS_UINT32                          ulATHCmdFlg;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength;

    pstLineCtrl = AT_GetUartLineCtrlInfo();
    usLength    = 0;
    ulResult    = AT_FAILURE;
    ulATHCmdFlg = (AT_CMD_H_PS_SET == AT_PS_GET_CURR_CMD_OPT(ucIndex)) ?
                  VOS_TRUE : VOS_FALSE;

    /* 去注册Modem端口的流控点 */
    AT_DeRegModemPsDataFCPoint(ucIndex, pstEvent->ucRabId);

    if (AT_PPP_DATA_MODE == gastAtClientTab[ucIndex].DataMode)
    {
        /* 释放PPP实体 & HDLC去使能 */
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_REL_PPP_REQ);
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_HDLC_DISABLE);

        if (AT_DATA_MODE == gastAtClientTab[ucIndex].Mode)
        {
            /* 根据问题单AT2D13296，在被动去激活情景下，向PPP发送了
               PPP_AT_CTRL_REL_PPP_REQ后，不立即切入命令态，而是等
               待PPP回应AT_PPP_PROTOCOL_REL_IND_MSG之后再切入命令态 */
            /* 开保护定时器，用于等待PPP回应AT_PPP_PROTOCOL_REL_IND_MSG */
            AT_STOP_TIMER_CMD_READY(ucIndex);

            if (AT_SUCCESS != At_StartTimer(AT_PPP_PROTOCOL_REL_TIME, ucIndex))
            {
                AT_ERR_LOG("At_UsbModemStatusPreProc:ERROR:Start Timer fail");
            }

            /* 设置当前操作类型 */
            gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_WAIT_PPP_PROTOCOL_REL_SET;

            return;
        }
    }
    else if (AT_IP_DATA_MODE == gastAtClientTab[ucIndex].DataMode)
    {
        /* 释放PPP实体 & HDLC去使能 */
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_REL_PPP_RAW_REQ);
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_HDLC_DISABLE);
    }
    else
    {
        /* 其它数传模式，不用处理 */
        AT_WARN_LOG("TAF_PS_EVT_PDP_DEACTIVATED OTHER MODE");
    }

    /* 返回命令模式 */
    At_SetMode(ucIndex, AT_CMD_MODE,AT_NORMAL_MODE);

    if (VOS_TRUE == ulATHCmdFlg)
    {
        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_NO_CARRIER;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);

    /* ATH断开PPP连接时, 如果DCD信号模式为CONNECT ON, 需要拉低DCD信号 */
    if ( (AT_OK == ulResult)
      && (AT_UART_DCD_MODE_CONNECT_ON == pstLineCtrl->enDcdMode) )
    {
        AT_CtrlDCD(ucIndex, AT_IO_LEVEL_LOW);
    }

    return;
}


VOS_VOID  AT_ModemPsRspPdpActEvtRejProc(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU  *pstEvent
)
{
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength;

    ulResult                            = AT_FAILURE;
    usLength                            = 0;

    if ( (AT_CMD_D_PPP_CALL_SET == gastAtClientTab[ucIndex].CmdCurrentOpt)
      || (AT_CMD_PPP_ORG_SET == gastAtClientTab[ucIndex].CmdCurrentOpt) )
    {
        ulResult = AT_NO_CARRIER;
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_REL_PPP_REQ);

        /* 向PPP发送HDLC去使能操作 */
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_HDLC_DISABLE);

        /* 返回命令模式 */
        At_SetMode(ucIndex,AT_CMD_MODE,AT_NORMAL_MODE);

    }
    else if (AT_CMD_D_IP_CALL_SET == gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        ulResult = AT_ERROR;
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_REL_PPP_RAW_REQ);

        /* 向PPP发送HDLC去使能操作 */
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_HDLC_DISABLE);


    }
    else
    {
        ulResult = AT_ERROR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);

    return;
}


VOS_VOID AT_FillPppIndConfigInfoPara(
    AT_PPP_IND_CONFIG_INFO_STRU        *pstPppIndConfigInfo,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    /* 填写IP地址 */
    TAF_MEM_CPY_S(pstPppIndConfigInfo->aucIpAddr,
               sizeof(pstPppIndConfigInfo->aucIpAddr),
               pstEvent->stPdpAddr.aucIpv4Addr,
               TAF_IPV4_ADDR_LEN);

    /* 填写DNS地址 */
    if (VOS_TRUE == pstEvent->stDns.bitOpPrimDnsAddr)
    {
        pstPppIndConfigInfo->stPcoIpv4Item.bitOpPriDns = VOS_TRUE;

        TAF_MEM_CPY_S(pstPppIndConfigInfo->stPcoIpv4Item.aucPriDns,
               sizeof(pstPppIndConfigInfo->stPcoIpv4Item.aucPriDns),
               pstEvent->stDns.aucPrimDnsAddr,
               TAF_IPV4_ADDR_LEN);
    }

    if (VOS_TRUE == pstEvent->stDns.bitOpSecDnsAddr)
    {
        pstPppIndConfigInfo->stPcoIpv4Item.bitOpSecDns = VOS_TRUE;

        TAF_MEM_CPY_S(pstPppIndConfigInfo->stPcoIpv4Item.aucSecDns,
                       sizeof(pstPppIndConfigInfo->stPcoIpv4Item.aucSecDns),
                       pstEvent->stDns.aucSecDnsAddr,
                       TAF_IPV4_ADDR_LEN);

    }

    /* 填写NBNS地址 */
    if (VOS_TRUE == pstEvent->stNbns.bitOpPrimNbnsAddr)
    {
        pstPppIndConfigInfo->stPcoIpv4Item.bitOpPriNbns = VOS_TRUE;

        TAF_MEM_CPY_S(pstPppIndConfigInfo->stPcoIpv4Item.aucPriNbns,
                   sizeof(pstPppIndConfigInfo->stPcoIpv4Item.aucPriNbns),
                   pstEvent->stNbns.aucPrimNbnsAddr,
                   TAF_IPV4_ADDR_LEN);
    }

    if (VOS_TRUE == pstEvent->stNbns.bitOpSecNbnsAddr)
    {
        pstPppIndConfigInfo->stPcoIpv4Item.bitOpSecNbns = VOS_TRUE;

        TAF_MEM_CPY_S(pstPppIndConfigInfo->stPcoIpv4Item.aucSecNbns,
                   sizeof(pstPppIndConfigInfo->stPcoIpv4Item.aucSecNbns),
                   pstEvent->stNbns.aucSecNbnsAddr,
                   TAF_IPV4_ADDR_LEN);
    }

    /* 填写GATE WAY地址 */
    if (VOS_TRUE == pstEvent->stGateWay.bitOpGateWayAddr)
    {
        pstPppIndConfigInfo->stPcoIpv4Item.bitOpGateWay = VOS_TRUE;

        TAF_MEM_CPY_S(pstPppIndConfigInfo->stPcoIpv4Item.aucGateWay,
                   sizeof(pstPppIndConfigInfo->stPcoIpv4Item.aucGateWay),
                   pstEvent->stGateWay.aucGateWayAddr,
                   TAF_IPV4_ADDR_LEN);
    }


}


VOS_UINT32 AT_RegModemPsDataFCPoint(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent,
    FC_ID_ENUM_UINT8                    enFcId
)
{
    FC_REG_POINT_STRU                   stRegFcPoint;
    VOS_UINT32                          ulRet;
    FC_PRI_ENUM_UINT8                   enFcPri;
    /* Modified by l60609 for DSDA Phase II, 2012-12-21, Begin */
    MODEM_ID_ENUM_UINT16                enModemId;
    AT_UART_CTX_STRU                   *pstUartCtx = VOS_NULL_PTR;

    pstUartCtx = AT_GetUartCtxAddr();

    /* UART端口流控关闭时不注册流控点 */
    if ( (VOS_TRUE == AT_CheckHsUartUser(ucIndex))
      && (AT_UART_FC_DTE_BY_DCE_NONE == pstUartCtx->stFlowCtrl.enDteByDce) )
    {
        return VOS_ERR;
    }

    enModemId = MODEM_ID_0;

    TAF_MEM_SET_S(&stRegFcPoint, sizeof(stRegFcPoint), 0x00, sizeof(FC_REG_POINT_STRU));

    ulRet = AT_GetModemIdFromClient((AT_CLIENT_TAB_INDEX_UINT8)pstEvent->stCtrl.usClientId, &enModemId);

    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_RegModemPsDataFCPoint: Get modem id fail.");
        return VOS_ERR;
    }

    /* 配置通道与RABID映射关系 */
    FC_ChannelMapCreate(enFcId, pstEvent->ucRabId, enModemId);

    stRegFcPoint.enFcId             = enFcId;

    /* 根据网卡上最高优先级RAB QoS优先级来折算,优先级改变时，需要改变优先级 */
    /*  FC_PRI_3        有最低优先级的承载
        FC_PRI_4        有NONGBR承载
        FC_PRI_5        有GBR承载 */
    if (TAF_USED == pstEvent->bitOpUmtsQos)
    {
        enFcPri = AT_GetFCPriFromQos(&pstEvent->stUmtsQos);
    }
    else
    {
        enFcPri = FC_PRI_FOR_PDN_NONGBR;
    }
    stRegFcPoint.enFcPri            = enFcPri;
    stRegFcPoint.enPolicyId         = FC_POLICY_ID_MEM;
    stRegFcPoint.pClrFunc           = AT_MODEM_StopFlowCtrl;
    stRegFcPoint.pSetFunc            = AT_MODEM_StartFlowCtrl;

    stRegFcPoint.ulParam1           = (VOS_UINT32)g_alAtUdiHandle[ucIndex];
    stRegFcPoint.enModemId          = enModemId;
    /* Modified by l60609 for DSDA Phase II, 2012-12-21, End */
    stRegFcPoint.ulParam2           = enFcId;
    stRegFcPoint.pRstFunc           = AT_ResetFlowCtl;

    /* 注册流控点,需要分别注册MEM,CPU,CDS和GPRS。 */
    ulRet = FC_RegPoint(&stRegFcPoint);
    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_RegModemFCPoint: ERROR: FC RegPoint MEM Failed.");
        return VOS_ERR;
    }

    stRegFcPoint.enPolicyId         = FC_POLICY_ID_CPU_A;
    ulRet = FC_RegPoint(&stRegFcPoint);
    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_RegModemFCPoint: ERROR: FC RegPoint A CPU Failed.");
        return VOS_ERR;
    }

    stRegFcPoint.enPolicyId         = FC_POLICY_ID_CDS;
    ulRet = FC_RegPoint(&stRegFcPoint);
    if (VOS_OK != ulRet)
    {
        return VOS_ERR;
    }

    stRegFcPoint.enPolicyId         = FC_POLICY_ID_GPRS;
    ulRet = FC_RegPoint(&stRegFcPoint);
    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_RegModemFCPoint: ERROR: FC RegPoint GPRS Failed.");
        return VOS_ERR;
    }

    stRegFcPoint.enPolicyId         = FC_POLICY_ID_CDMA;
    ulRet = FC_RegPoint(&stRegFcPoint);
    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_RegModemFCPoint: ERROR: reg CDMA point Failed.");
        return VOS_ERR;
    }

    /* 设置FCID与FC Pri的映射关系 */
    g_stFcIdMaptoFcPri[FC_ID_MODEM].ulUsed      = VOS_TRUE;
    g_stFcIdMaptoFcPri[FC_ID_MODEM].enFcPri     = enFcPri;
    /* 有一张网卡上多个RABID的情况，所以需要将多个RABID记录下来 */
    g_stFcIdMaptoFcPri[FC_ID_MODEM].ulRabIdMask |= ((VOS_UINT32)1 << (pstEvent->ucRabId));
    g_stFcIdMaptoFcPri[FC_ID_MODEM].enModemId   = enModemId;

    /* 勾流控消息 */
    AT_MNTN_TraceRegFcPoint(ucIndex, AT_FC_POINT_TYPE_MODEM_PS);

    return VOS_OK;
}


VOS_UINT32 AT_DeRegModemPsDataFCPoint(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           ucRabId
)
{
    VOS_UINT32                          ulRet;
    /* Modified by l60609 for DSDA Phase II, 2012-12-28, Begin */
    MODEM_ID_ENUM_UINT16                enModemId;
    AT_UART_CTX_STRU                   *pstUartCtx = VOS_NULL_PTR;

    pstUartCtx = AT_GetUartCtxAddr();

    /* UART端口流控关闭时不注册流控点 */
    if ( (VOS_TRUE == AT_CheckHsUartUser(ucIndex))
        && (AT_UART_FC_DTE_BY_DCE_NONE == pstUartCtx->stFlowCtrl.enDteByDce) )
    {
        return VOS_ERR;
    }

    enModemId = MODEM_ID_0;

    ulRet = AT_GetModemIdFromClient(ucIndex, &enModemId);
    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_DeRegModemPsDataFCPoint: Get modem id fail.");
        return VOS_ERR;
    }
    /* Modified by l60609 for DSDA Phase II, 2012-12-21, Begin */
    /* 删除流控模块映射关系 */
    FC_ChannelMapDelete(ucRabId, enModemId);

    ulRet = FC_DeRegPoint(FC_ID_MODEM, enModemId);
    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_DeRegModemPsDataFCPoint: ERROR: FC DeRegPoint Failed.");
        return VOS_ERR;
    }
    /* Modified by l60609 for DSDA Phase II, 2012-12-21, End */

    /* 清除FCID与FC Pri的映射关系 */
    g_stFcIdMaptoFcPri[FC_ID_MODEM].ulUsed      = VOS_FALSE;
    g_stFcIdMaptoFcPri[FC_ID_MODEM].enFcPri     = FC_PRI_BUTT;
    /* 有一张网卡上多个RABID的情况，所以需要将对应的RABID掩码清除掉 */
    g_stFcIdMaptoFcPri[FC_ID_MODEM].ulRabIdMask &= ~((VOS_UINT32)1 << ucRabId);
    g_stFcIdMaptoFcPri[FC_ID_MODEM].enModemId   = MODEM_ID_BUTT;

    /* 勾流控消息 */
    AT_MNTN_TraceDeregFcPoint(ucIndex, AT_FC_POINT_TYPE_MODEM_PS);

    return VOS_OK;
}


VOS_VOID AT_ModemPsRspPdpActEvtCnfProc(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    AT_PPP_IND_CONFIG_INFO_STRU         stPppIndConfigInfo;

    /* 注册Modem端口的流控点 */
    AT_RegModemPsDataFCPoint(ucIndex, pstEvent, FC_ID_MODEM);

    /* 初始化 */
    TAF_MEM_SET_S(&stPppIndConfigInfo, sizeof(stPppIndConfigInfo),
                  0x00, sizeof(AT_PPP_IND_CONFIG_INFO_STRU));

    /* 清除命令操作类型 */
    AT_STOP_TIMER_CMD_READY(ucIndex);


    if (TAF_PDP_PPP == pstEvent->stPdpAddr.enPdpType)
    {
        At_SetMode(ucIndex, AT_DATA_MODE, AT_IP_DATA_MODE);

        /* 注册下行数据发送函数 */
        Ppp_RegDlDataCallback(gastAtClientTab[ucIndex].usPppId);

        At_FormatResultData(ucIndex, AT_CONNECT);
    }
    else if (TAF_PDP_IPV4 == pstEvent->stPdpAddr.enPdpType)
    {
        /* 填写IP地址, DNS, NBNS */
        AT_FillPppIndConfigInfoPara(&stPppIndConfigInfo, pstEvent);

        /* 将AUTH和IPCP帧交给PPP处理: */
        Ppp_RcvConfigInfoInd(gastAtClientTab[ucIndex].usPppId, &stPppIndConfigInfo);
    }
    else
    {
        /* 其他类型不做处理 */
    }

    return;
}


VOS_VOID  AT_ModemPsRspPdpDeactEvtCnfProc(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent
)
{
    AT_UART_LINE_CTRL_STRU             *pstLineCtrl = VOS_NULL_PTR;
    VOS_UINT32                          ulModemUsrFlg;
    VOS_UINT32                          ulATHCmdFlg;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength;

    pstLineCtrl     = AT_GetUartLineCtrlInfo();
    ulModemUsrFlg   = AT_CheckModemUser(ucIndex);
    usLength        = 0;
    ulResult        = AT_FAILURE;
    ulATHCmdFlg     = (AT_CMD_H_PS_SET == gastAtClientTab[ucIndex].CmdCurrentOpt) ?
                      VOS_TRUE : VOS_FALSE;

    if ( (AT_MODEM_USER != gastAtClientTab[ucIndex].UserType)
      && (AT_HSUART_USER != gastAtClientTab[ucIndex].UserType) )
    {
        return;
    }

    if (AT_PPP_DATA_MODE == gastAtClientTab[ucIndex].DataMode)
    {
        /* 释放PPP实体 & HDLC去使能 */
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_REL_PPP_REQ);
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_HDLC_DISABLE);
    }
    else if (AT_IP_DATA_MODE == gastAtClientTab[ucIndex].DataMode)
    {
        /* 释放PPP实体 & HDLC去使能 */
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_REL_PPP_RAW_REQ);

        /* 向PPP发送HDLC去使能操作 */
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_HDLC_DISABLE);
    }
    else
    {
        /* 其它数传模式，不用处理 */
        AT_WARN_LOG("TAF_PS_EVT_PDP_DEACTIVE_CNF OTHER MODE");
    }

    /* 去注册Modem端口的流控点 */
    AT_DeRegModemPsDataFCPoint(ucIndex, pstEvent->ucRabId);

    /* 返回命令模式 */
    At_SetMode(ucIndex, AT_CMD_MODE, AT_NORMAL_MODE);

    /* 为了规避Linux后台二次拨号失败问题，采用如下规避方案:
       PDP正常断开流程结束后，接收到TAF_PS_EVT_PDP_DEACTIVE_CNF事件，判断操作
       系统是否为Linux，若否，则按照原先流程处理，若是，则不再发送"NO CARRIER" */
    if ((VOS_OK == DRV_GET_LINUXSYSTYPE()) && (VOS_TRUE == ulModemUsrFlg))
    {
        /* 原先流程中，在At_FormatResultData函数内部，发送了"NO CARRIER"之后，
           需要将DCD拉低，在此规避方案中，不发送"NO CARRIER"，但DCD信号的拉低
           操作仍旧需要保持
        */
        AT_STOP_TIMER_CMD_READY(ucIndex);
        AT_CtrlDCD(ucIndex, AT_IO_LEVEL_LOW);
        return;
    }

    if (VOS_TRUE == ulATHCmdFlg)
    {
        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_NO_CARRIER;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);

    /* ATH断开PPP连接时, 如果DCD信号模式为CONNECT ON, 需要拉低DCD信号 */
    if ( (AT_OK == ulResult)
      && (AT_UART_DCD_MODE_CONNECT_ON == pstLineCtrl->enDcdMode) )
    {
        AT_CtrlDCD(ucIndex, AT_IO_LEVEL_LOW);
    }

    return;
}


VOS_VOID AT_MODEM_ProcCallEndCnfEvent(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_END_CNF_STRU           *pstEvent
)
{
    AT_UART_LINE_CTRL_STRU             *pstLineCtrl = VOS_NULL_PTR;
    AT_DCE_MSC_STRU                     stDecMsc;
    VOS_UINT32                          ulModemUsrFlg;
    VOS_UINT32                          ulATHCmdFlg;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength;

    pstLineCtrl     = AT_GetUartLineCtrlInfo();
    ulModemUsrFlg   = AT_CheckModemUser(ucIndex);
    ulResult        = AT_FAILURE;
    usLength        = 0;
    ulATHCmdFlg     = (AT_CMD_H_PS_SET == AT_PS_GET_CURR_CMD_OPT(ucIndex)) ?
                      VOS_TRUE : VOS_FALSE;

    TAF_MEM_SET_S(&stDecMsc, sizeof(stDecMsc), 0x00, sizeof(AT_DCE_MSC_STRU));

    /* 检查当前用户的操作类型 */
    if ( (AT_CMD_PS_DATA_CALL_END_SET != AT_PS_GET_CURR_CMD_OPT(ucIndex))
      && (AT_CMD_H_PS_SET != AT_PS_GET_CURR_CMD_OPT(ucIndex)) )
    {
        return;
    }

    /* PS域呼叫正在处理断开请求, 直接返回 */
    if (TAF_ERR_NO_ERROR == pstEvent->enCause)
    {
        return;
    }

    /* 释放PPP实体 & HDLC去使能 */
    if (AT_PPP_DATA_MODE == AT_PS_GET_CURR_DATA_MODE(ucIndex))
    {
        PPP_RcvAtCtrlOperEvent(AT_PS_GET_PPPID(ucIndex), PPP_AT_CTRL_REL_PPP_REQ);
        PPP_RcvAtCtrlOperEvent(AT_PS_GET_PPPID(ucIndex), PPP_AT_CTRL_HDLC_DISABLE);
    }
    else if (AT_IP_DATA_MODE == AT_PS_GET_CURR_DATA_MODE(ucIndex))
    {
        PPP_RcvAtCtrlOperEvent(AT_PS_GET_PPPID(ucIndex), PPP_AT_CTRL_REL_PPP_RAW_REQ);
        PPP_RcvAtCtrlOperEvent(AT_PS_GET_PPPID(ucIndex), PPP_AT_CTRL_HDLC_DISABLE);
    }
    else
    {
        ;
    }

    /* 返回命令模式 */
    At_SetMode(ucIndex, AT_CMD_MODE, AT_NORMAL_MODE);

    /* 为了规避Linux后台二次拨号失败问题，采用如下规避方案:
       PDP正常断开流程结束后，接收到TAF_PS_EVT_PDP_DEACTIVE_CNF事件，判断操作
       系统是否为Linux，若否，则按照原先流程处理，若是，则不再发送"NO CARRIER" */
    if ((VOS_OK == DRV_GET_LINUXSYSTYPE()) && (VOS_TRUE == ulModemUsrFlg))
    {
        /* 原先流程中，在At_FormatResultData函数内部，发送了"NO CARRIER"之后，
           需要将DCD拉低，在此规避方案中，不发送"NO CARRIER"，但DCD信号的拉低
           操作仍旧需要保持
        */
        AT_STOP_TIMER_CMD_READY(ucIndex);
        AT_CtrlDCD(ucIndex, AT_IO_LEVEL_LOW);
        return;
    }

    if (VOS_TRUE == ulATHCmdFlg)
    {
        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_NO_CARRIER;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, ulResult);

    /* ATH断开PPP连接时, 如果DCD信号模式为CONNECT ON, 需要拉低DCD信号 */
    if ( (AT_OK == ulResult)
      && (AT_UART_DCD_MODE_CONNECT_ON == pstLineCtrl->enDcdMode) )
    {
        AT_CtrlDCD(ucIndex, AT_IO_LEVEL_LOW);
    }

    return;
}


VOS_VOID  AT_AnswerPdpActInd(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    VOS_UINT16                          usPppId;
    VOS_UINT32                          ulRslt;

    usPppId     = 0;

    if (TAF_PDP_PPP == pstEvent->stPdpAddr.enPdpType)
    {
        if (VOS_OK != Ppp_CreateRawDataPppReq(&usPppId))
        {
            ulRslt = AT_ERROR;
        }
        else
        {
            /* 记录PPP id和Index的对应关系 */
            gastAtPppIndexTab[usPppId]          = ucIndex;

            /* 保存PPP id */
            gastAtClientTab[ucIndex].usPppId    = usPppId;


            /* 注册Modem端口的流控点 */
            AT_RegModemPsDataFCPoint(ucIndex, pstEvent, FC_ID_MODEM);

            /* 切换数传通道 */
            At_SetMode(ucIndex, AT_DATA_MODE, AT_IP_DATA_MODE);

            /* 注册下行数据发送函数 */
            Ppp_RegDlDataCallback(usPppId);

            ulRslt = AT_CONNECT;
        }

        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, ulRslt);
    }
    else
    {
        /* 方案待定....*/
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_CONNECT);
    }

    return;
}


VOS_UINT32 At_RcvTeConfigInfoReq(
    VOS_UINT16                          usPppId,
    AT_PPP_REQ_CONFIG_INFO_STRU        *pstPppReqConfigInfo
)
{
    if (AT_MAX_CLIENT_NUM <= usPppId)
    {
        AT_WARN_LOG("At_RcvTeConfigInfoReq usPppId Wrong");
        return AT_FAILURE;
    }

    if (TAF_NULL_PTR == pstPppReqConfigInfo)
    {
        AT_WARN_LOG("At_RcvTeConfigInfoReq pPppReqConfigInfo NULL");
        return AT_FAILURE;
    }

    if (AT_CMD_D_PPP_CALL_SET != gastAtClientTab[gastAtPppIndexTab[usPppId]].CmdCurrentOpt)
    {
        AT_WARN_LOG("At_RcvTeConfigInfoReq NOT AT_CMD_D_PPP_CALL_SET");
        return AT_FAILURE;
    }

    if ( VOS_OK == TAF_PS_PppDialOrig(WUEPS_PID_AT,
                                      AT_PS_BuildExClientId(gastAtClientTab[gastAtPppIndexTab[usPppId]].usClientId),
                                      0,
                                      gastAtClientTab[gastAtPppIndexTab[usPppId]].ucCid,
                                      (TAF_PPP_REQ_CONFIG_INFO_STRU *)pstPppReqConfigInfo) )
    {
        /* 开定时器 */
        if (AT_SUCCESS != At_StartTimer(AT_ACT_PDP_TIME, gastAtPppIndexTab[usPppId]))
        {
            AT_ERR_LOG("At_RcvTeConfigInfoReq:ERROR:Start Timer");
            return AT_FAILURE;
        }

        /* 设置当前操作类型 */
        gastAtClientTab[gastAtPppIndexTab[usPppId]].CmdCurrentOpt = AT_CMD_PPP_ORG_SET;

        return AT_SUCCESS;
    }
    else
    {
        return AT_FAILURE;
    }
}


VOS_UINT32 At_RcvPppReleaseInd(
    VOS_UINT16                          usPppId
)
{
    TAF_UINT8                           aucEventInfo[4];
    AT_PPP_RELEASE_IND_MSG_STRU        *pMsg;
    VOS_UINT32                          ulLength;

    if (AT_MAX_CLIENT_NUM <= usPppId)
    {
        AT_WARN_LOG("At_RcvPppReleaseInd usPppId Wrong");
        return AT_FAILURE;
    }

    /* EVENT- At_RcvPppReleaseInd:usPppId / gastAtPppIndexTab[usPppId] */
    aucEventInfo[0] = (TAF_UINT8)(usPppId >> 8);
    aucEventInfo[1] = (TAF_UINT8)usPppId;
    aucEventInfo[2] = gastAtPppIndexTab[usPppId];
    AT_EventReport(WUEPS_PID_AT, NAS_OM_EVENT_DTE_RELEASE_PPP,
                   aucEventInfo, (VOS_UINT32)sizeof(aucEventInfo));

    /* 向AT模块发送AT_PPP_RELEASE_IND_MSG */
    ulLength       = sizeof(AT_PPP_RELEASE_IND_MSG_STRU) - VOS_MSG_HEAD_LENGTH;
    /*lint -save -e516 */
    pMsg = (AT_PPP_RELEASE_IND_MSG_STRU *)PS_ALLOC_MSG(PS_PID_APP_PPP, ulLength);/*lint !e830*/
    /*lint -restore */
    if (VOS_NULL_PTR == pMsg)
    {
        /* 打印出错信息---申请消息包失败 */
        AT_ERR_LOG( "At_RcvPppReleaseInd:ERROR:Allocates a message packet for AT_PPP_RELEASE_IND_MSG_STRU msg FAIL!" );
        return AT_FAILURE;
    }
    /* 填写消息头 */
    pMsg->MsgHeader.ulSenderCpuId   = VOS_LOCAL_CPUID;
    pMsg->MsgHeader.ulSenderPid     = PS_PID_APP_PPP;
    pMsg->MsgHeader.ulReceiverCpuId = VOS_LOCAL_CPUID;
    pMsg->MsgHeader.ulReceiverPid   = WUEPS_PID_AT;
    pMsg->MsgHeader.ulLength        = ulLength;

    pMsg->MsgHeader.ulMsgName       = AT_PPP_RELEASE_IND_MSG;
    /* 填写消息体 */
    pMsg->ucIndex                   = gastAtPppIndexTab[usPppId];

    /* 发送该消息 */
    if (VOS_OK != PS_SEND_MSG(PS_PID_APP_PPP, pMsg))
    {
        /* 打印警告信息---发送消息失败 */
        AT_WARN_LOG( "At_RcvPppReleaseInd:WARNING:SEND AT_PPP_RELEASE_IND_MSG msg FAIL!" );
        return AT_FAILURE;
    }
    else
    {
        /* 打印流程信息---发送了消息 */
        AT_WARN_LOG( "At_RcvPppReleaseInd:NORMAL:SEND AT_PPP_RELEASE_IND_MSG Msg" );
        return AT_SUCCESS;
    }
}


VOS_VOID At_PppReleaseIndProc(
    VOS_UINT8                           ucIndex
)
{
    if (AT_MAX_CLIENT_NUM <= ucIndex)
    {
        AT_WARN_LOG("At_PppReleaseIndProc:ERROR:ucIndex is abnormal!");
        return;
    }

    if (AT_CMD_WAIT_PPP_PROTOCOL_REL_SET == gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        /* 将AT通道切换为命令模式 */
        At_SetMode(ucIndex, AT_CMD_MODE, AT_NORMAL_MODE);

        /*停止定时器*/
        AT_STOP_TIMER_CMD_READY(ucIndex);

        /*回复NO CARRIER*/
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_NO_CARRIER);

        return;
    }

    if (AT_CMD_PS_DATA_CALL_END_SET == gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        return;
    }

    if ( VOS_OK == TAF_PS_CallEnd(WUEPS_PID_AT,
                                  AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                  0,
                                  gastAtClientTab[ucIndex].ucCid) )
    {
        /* 开定时器 */
        if (AT_SUCCESS != At_StartTimer(AT_DETACT_PDP_TIME, ucIndex))
        {
            AT_ERR_LOG("At_PppReleaseIndProc:ERROR:Start Timer");
            return;
        }

        /* 设置当前操作类型 */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_PS_DATA_CALL_END_SET;
    }

    return;
}


TAF_UINT32 At_PsRab2PppId(
    TAF_UINT8                           ucExRabId,
    TAF_UINT16                         *pusPppId
)
{
    TAF_UINT16                          usPppId;
    TAF_UINT8                           ucIndex;

    if (VOS_NULL_PTR == pusPppId)
    {
        AT_WARN_LOG("At_PsRab2PppId, pusPppId NULL");
        return TAF_FAILURE;
    }

    /* 通过PPP ID方向查找，效率会比较高 */
    for (usPppId = 1; usPppId <= PPP_MAX_ID_NUM; usPppId++)
    {
        ucIndex = gastAtPppIndexTab[usPppId];

        if (AT_MAX_CLIENT_NUM <= ucIndex)
        {
            continue;
        }

        if ( AT_CLIENT_USED != gastAtClientTab[ucIndex].ucUsed )
        {
            continue;
        }

        if (AT_DATA_MODE != gastAtClientTab[ucIndex].Mode)
        {
            continue;
        }

        if ( (AT_PPP_DATA_MODE != gastAtClientTab[ucIndex].DataMode)
          && (AT_IP_DATA_MODE != gastAtClientTab[ucIndex].DataMode) )
        {
            continue;
        }

        if (gastAtClientTab[ucIndex].ucExPsRabId == ucExRabId)
        {
            *pusPppId = usPppId;    /* 返回结果 */
            return TAF_SUCCESS;
        }

    }

    AT_LOG1("AT, At_PsRab2PppId, WARNING, Get PppId from Rab <1> Fail", ucExRabId);

    return TAF_FAILURE;
} /* At_PsRab2PppId */



TAF_UINT32 At_PppId2PsRab(
    TAF_UINT16                          usPppId,
    TAF_UINT8                          *pucExRabId
)
{
    TAF_UINT8                           ucIndex;

    if ((usPppId < 1) || (usPppId > PPP_MAX_ID_NUM))
    {
        TAF_LOG1(WUEPS_PID_AT, 0, PS_LOG_LEVEL_WARNING,
            "AT, At_PppId2PsRab, WARNING, PppId <1> Wrong", usPppId);
        return TAF_FAILURE;
    }

    if (TAF_NULL_PTR == pucExRabId)
    {
        AT_WARN_LOG("AT, At_PppId2PsRab, WARNING, pucRabId NULL");
        return TAF_FAILURE;
    }

    ucIndex = gastAtPppIndexTab[usPppId];

    if (AT_MAX_CLIENT_NUM <= ucIndex)
    {
        AT_LOG1("AT, At_PppId2PsRab, WARNING, index <1> Wrong", ucIndex);
        return TAF_FAILURE;
    }

    if (AT_DATA_MODE != gastAtClientTab[ucIndex].Mode)
    {
        AT_LOG1("AT, At_PppId2PsRab, WARNING, Mode <1> Wrong", gastAtClientTab[ucIndex].Mode);
        return TAF_FAILURE;
    }

    if ( (AT_PPP_DATA_MODE != gastAtClientTab[ucIndex].DataMode)
      && (AT_IP_DATA_MODE  != gastAtClientTab[ucIndex].DataMode) )
    {
        AT_LOG1("AT, At_PppId2PsRab, WARNING, DataMode <1> Wrong", gastAtClientTab[ucIndex].DataMode);
        return TAF_FAILURE;
    }

    *pucExRabId = gastAtClientTab[ucIndex].ucExPsRabId;

    return TAF_SUCCESS;
} /* At_PppId2PsRab */




VOS_UINT32 AT_GetLanAddr32(
  VOS_UINT8                            *pucAddr
)
{
    VOS_UINT32                          ulAddr;

    ulAddr = *pucAddr++;
    ulAddr <<= 8;
    ulAddr |= *pucAddr++;
    ulAddr <<= 8;
    ulAddr |= *pucAddr++;
    ulAddr <<= 8;
    ulAddr |= *pucAddr;

    return ulAddr;
}


VOS_VOID AT_GetNwIpv4Addr(
    VOS_UINT32                          ulAddr,
    VOS_UINT8                          *pucAddr
)
{
    *pucAddr++ = (ulAddr & 0xFF000000) >> 24;
    *pucAddr++ = (ulAddr & 0x00FF0000) >> 16;
    *pucAddr++ = (ulAddr & 0x0000FF00) >> 8;
    *pucAddr++ = (ulAddr & 0x000000FF);

    return;
}


VOS_UINT32 AT_DHCPCheckCfg(
    AT_DHCP_SETUP_PARAM_ST             *ptrDHCPCfg
)
{
    if ( (0 == ptrDHCPCfg->ulIPAddr)
      || (0 == (0xffffffffU - ptrDHCPCfg->ulIPAddr)) )
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_DHCPGetIPMask(
    VOS_UINT32                          ulIpAddrHL
)
{
    VOS_UINT32 ulMask;

    if (AT_IS_CLASS_A_IPV4_ADDR(ulIpAddrHL))
    {
        ulMask = AT_IPV4_CLASS_A_SUBNET_MASK;
    }
    else if (AT_IS_CLASS_B_IPV4_ADDR(ulIpAddrHL))
    {
        ulMask = AT_IPV4_CLASS_B_SUBNET_MASK;
    }
    else
    {
        ulMask = AT_IPV4_CLASS_C_SUBNET_MASK;
    }

    return ulMask;
}

VOS_UINT32 AT_DHCPGetGateWay(
    VOS_UINT32                          ulIpAddrHL,
    VOS_UINT32                          ulMaskHL
)
{
    VOS_UINT32 ulGateWay;

    ulGateWay = 0;

    ulGateWay = (ulIpAddrHL & ulMaskHL) + 1;

    while (ulGateWay == ulIpAddrHL)
    {
        ulGateWay ++;
    };

    return ulGateWay;
}


VOS_UINT32  AT_DHCPServerSetUp(
    AT_DHCP_SETUP_PARAM_ST              *ptrDHCPParam,
    AT_DHCP_CONFIG_STRU                 *ptrDHCPConfig
)
{
    if (VOS_ERR == AT_DHCPCheckCfg(ptrDHCPParam))
    {
        AT_ERR_LOG("AT_DHCPServerSetUp, ERROR, DHCP Config IP Address is Error!" );
        return VOS_ERR;
    }

      /*计算掩码、网关*/
    ptrDHCPConfig->ulIPAddr     = ptrDHCPParam->ulIPAddr;
    ptrDHCPConfig->ulSubNetMask =
      AT_DHCPGetIPMask(ptrDHCPParam->ulIPAddr);
    ptrDHCPConfig->ulGateWay    =
      AT_DHCPGetGateWay(ptrDHCPParam->ulIPAddr,ptrDHCPConfig->ulSubNetMask);
    ptrDHCPConfig->ulPrimDNS    = ptrDHCPParam->ulPrimDNS;
    ptrDHCPConfig->ulSndDNS     = ptrDHCPParam->ulSndDNS;

    return VOS_OK;
}


VOS_UINT32 AT_GetFcPriFromMap(
    FC_ID_ENUM_UINT8                   enFcId,
    AT_FCID_MAP_STRU                  *pstFcIdMap
)
{
    if (enFcId >= FC_ID_BUTT)
    {
        return VOS_ERR;
    }

    if ((FC_ID_MODEM == enFcId)
     || (FC_ID_NIC_1 == enFcId)
     || (FC_ID_NIC_2 == enFcId)
     || (FC_ID_NIC_3 == enFcId)
     || (FC_ID_NIC_4 == enFcId)
     || (FC_ID_NIC_5 == enFcId)
     || (FC_ID_NIC_6 == enFcId)
     || (FC_ID_NIC_7 == enFcId)
     || (FC_ID_DIPC_1 == enFcId)
     || (FC_ID_DIPC_2 == enFcId)
     || (FC_ID_DIPC_3 == enFcId))
    {
        pstFcIdMap->ulUsed  = g_stFcIdMaptoFcPri[enFcId].ulUsed;
        pstFcIdMap->enFcPri = g_stFcIdMaptoFcPri[enFcId].enFcPri;

        return VOS_OK;
    }

    return VOS_ERR;
}


VOS_VOID AT_ChangeFCPoint(
    TAF_CTRL_STRU                       *pstCtrl,
    FC_PRI_ENUM_UINT8                    enFCPri,
    FC_ID_ENUM_UINT8                     enFcId
)
{
    VOS_UINT32                          ulRet;
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = MODEM_ID_0;

    ulRet = AT_GetModemIdFromClient((AT_CLIENT_TAB_INDEX_UINT8)pstCtrl->usClientId, &enModemId);

    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_ChangeFCPoint: Get modem id fail.");
        return;
    }

    if (FC_ID_BUTT != enFcId)
    {
        ulRet = FC_ChangePoint(enFcId, FC_POLICY_ID_MEM, enFCPri, enModemId);
        if (VOS_OK != ulRet)
        {
            AT_ERR_LOG("AT_ChangeFCPoint: ERROR: Change FC Point Failed.");
        }

        ulRet = FC_ChangePoint(enFcId, FC_POLICY_ID_CPU_A , enFCPri, enModemId);
        if (VOS_OK != ulRet)
        {
            AT_ERR_LOG("AT_ChangeFCPoint: ERROR: Change FC Point Failed.");
        }

        ulRet = FC_ChangePoint(enFcId, FC_POLICY_ID_CDS, enFCPri, enModemId);
        if (VOS_OK != ulRet)
        {
            AT_ERR_LOG("AT_ChangeFCPoint: ERROR: Change FC Point Failed.");
        }

        ulRet = FC_ChangePoint(enFcId, FC_POLICY_ID_GPRS, enFCPri, enModemId);
        if (VOS_OK != ulRet)
        {
            AT_ERR_LOG("AT_ChangeFCPoint: ERROR: Change FC Point Failed.");
        }
    }

    return;
}


VOS_VOID AT_NotifyFcWhenPdpModify(
    TAF_PS_CALL_PDP_MODIFY_CNF_STRU    *pstEvent,
    FC_ID_ENUM_UINT8                    enFcId
)
{
    FC_PRI_ENUM_UINT8                   enFCPriCurrent;
    AT_FCID_MAP_STRU                    stFCPriOrg;

    if (VOS_OK == AT_GetFcPriFromMap(enFcId, &stFCPriOrg))
    {
        if (TAF_USED == pstEvent->bitOpUmtsQos)
        {
            enFCPriCurrent = AT_GetFCPriFromQos(&pstEvent->stUmtsQos);
        }
        else
        {
            enFCPriCurrent = FC_PRI_FOR_PDN_NONGBR;
        }

        if ( (VOS_TRUE == stFCPriOrg.ulUsed)
           && (enFCPriCurrent > stFCPriOrg.enFcPri))
        {
            /* 根据返回QOS来改变流控点的优先级*/
            AT_ChangeFCPoint(&pstEvent->stCtrl,enFCPriCurrent,enFcId);
        }
    }

    return;
}


VOS_VOID AT_PS_SetPsCallErrCause(
    VOS_UINT16                          usClientId,
    TAF_PS_CAUSE_ENUM_UINT32            enPsErrCause
)
{
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(usClientId);

    pstPsModemCtx->enPsErrCause = enPsErrCause;

    return;
}


TAF_PS_CAUSE_ENUM_UINT32 AT_PS_GetPsCallErrCause(
    VOS_UINT16                          usClientId
)
{
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(usClientId);

    return pstPsModemCtx->enPsErrCause;
}


VOS_VOID AT_PS_AddIpAddrRabIdMap(
    VOS_UINT16                          usClientId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;
    VOS_UINT32                          ulIpAddr;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(usClientId);
    ulIpAddr      = 0;

    if (!AT_PS_IS_RABID_VALID(pstEvent->ucRabId))
    {
        return;
    }

    if (pstEvent->bitOpPdpAddr)
    {
        ulIpAddr = AT_GetLanAddr32(pstEvent->stPdpAddr.aucIpv4Addr);
        pstPsModemCtx->aulIpAddrRabIdMap[pstEvent->ucRabId - AT_PS_RABID_OFFSET] = ulIpAddr;
    }

    return;
}


VOS_VOID AT_PS_DeleteIpAddrRabIdMap(
    VOS_UINT16                          usClientId,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent
)
{
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(usClientId);

    if (!AT_PS_IS_RABID_VALID(pstEvent->ucRabId))
    {
        return;
    }

    pstPsModemCtx->aulIpAddrRabIdMap[pstEvent->ucRabId - AT_PS_RABID_OFFSET] = 0;

    return;
}


VOS_UINT32 AT_PS_GetIpAddrByRabId(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucRabId
)
{
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;
    VOS_UINT32                          ulIpAddr;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(usClientId);
    ulIpAddr      = 0;

    if (AT_PS_IS_RABID_VALID(ucRabId))
    {
        ulIpAddr = pstPsModemCtx->aulIpAddrRabIdMap[ucRabId - AT_PS_RABID_OFFSET];
    }

    return ulIpAddr;
}


VOS_UINT32 AT_PS_GetChDataValueFromRnicRmNetId(
    RNIC_DEV_ID_ENUM_UINT8              enRnicRmNetId,
    AT_CH_DATA_CHANNEL_ENUM_UINT32     *penDataChannelId
)
{
    VOS_UINT32                          i;
    CONST AT_CHDATA_RNIC_RMNET_ID_STRU *pstChdataRnicRmNetIdTab;

    pstChdataRnicRmNetIdTab = AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_PTR();

    for (i = 0; i < AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_SIZE() ; i++)
    {
        if (enRnicRmNetId == pstChdataRnicRmNetIdTab[i].enRnicRmNetId)
        {
            *penDataChannelId = pstChdataRnicRmNetIdTab[i].enChdataValue;
            break;
        }
    }

    if (i >= AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_SIZE())
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


CONST AT_CHDATA_RNIC_RMNET_ID_STRU *AT_PS_GetChDataCfgByChannelId(
    VOS_UINT8                           ucIndex,
    AT_CH_DATA_CHANNEL_ENUM_UINT32      enDataChannelId
)
{
    CONST AT_CHDATA_RNIC_RMNET_ID_STRU *pstChdataRnicRmNetIdTab = VOS_NULL_PTR;
    CONST AT_CHDATA_RNIC_RMNET_ID_STRU *pstChDataConfig         = VOS_NULL_PTR;
    VOS_UINT32                          ulRslt;
    VOS_UINT32                          i;
    MODEM_ID_ENUM_UINT16                enModemId;

    pstChdataRnicRmNetIdTab = AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_PTR();

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);
    if (VOS_OK != ulRslt)
    {
        return VOS_NULL_PTR;
    }

    /**
     * MODEM0 RNIC ID 为 0~2， CHDATA的值为 1~3
     * MODEM1 RNIC ID 为 3~4， CHDATA的值为 4~5
     * MODEM2 RNIC ID 为 6~7， CHDATA的值为 6~7
     */

    if (MODEM_ID_0 == enModemId)
    {
        if ((enDataChannelId < AT_CH_DATA_CHANNEL_ID_1)
         || (enDataChannelId > AT_CH_DATA_CHANNEL_ID_3))
        {
            return VOS_NULL_PTR;
        }
    }
    else if (MODEM_ID_1 == enModemId)
    {
        if ((enDataChannelId < AT_CH_DATA_CHANNEL_ID_4)
         || (enDataChannelId > AT_CH_DATA_CHANNEL_ID_5))
        {
            return VOS_NULL_PTR;
        }
    }
    else if (MODEM_ID_2 == enModemId)
    {
        if ((enDataChannelId < AT_CH_DATA_CHANNEL_ID_6)
         || (enDataChannelId > AT_CH_DATA_CHANNEL_ID_7))
        {
            return VOS_NULL_PTR;
        }
    }
    else
    {
        return VOS_NULL_PTR;
    }

    /*  以上判断已能保证enDataChannelId的有效性，所以RM NET ID一定能在表中找到 */
    for (i = 0; i < AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_SIZE() ; i++)
    {
        if (enDataChannelId == pstChdataRnicRmNetIdTab[i].enChdataValue)
        {
            pstChDataConfig = &pstChdataRnicRmNetIdTab[i];
            break;
        }
    }

    return pstChDataConfig;
}


VOS_UINT32 AT_PS_GetAppCallRnicIFaceIdFromCallId(
    VOS_UINT8                                   ucCallId,
    RNIC_DEV_ID_ENUM_UINT8                     *penRmNetId,
    PS_IFACE_ID_ENUM_UINT8                     *penIfaceId
)
{
    VOS_UINT32                                  i;
    CONST AT_PS_APP_CALL_RNIC_IFACE_ID_STRU    *pstRnicIFaceIdTab;

    pstRnicIFaceIdTab = AT_PS_GET_APP_CALL_RNIC_IFACE_ID_TBL_PTR();

    for (i = 0; i < AT_PS_GET_APP_CALL_RNIC_IFACE_ID_TBL_SIZE(); i++)
    {
        if (ucCallId == pstRnicIFaceIdTab[i].ucCallIndex)
        {
            *penRmNetId = pstRnicIFaceIdTab[i].enRnicRmNetId;
            *penIfaceId = pstRnicIFaceIdTab[i].enIfaceId;
            return VOS_OK;
        }
    }

    AT_WARN_LOG("AT_PS_GetAppCallRnicIFaceIdFromCallId: not find iface id");
    return VOS_ERR;
}


VOS_UINT32 AT_PS_GetNdisCallIFaceIdFromCallId(
    VOS_UINT8                                   ucCallId,
    RNIC_DEV_ID_ENUM_UINT8                     *penRmNetId,
    PS_IFACE_ID_ENUM_UINT8                     *penIfaceId
)
{
    VOS_UINT32                                  i;
    CONST AT_PS_NDIS_CALL_IFACE_ID_STRU        *pstNdisIFaceIdTab;

    pstNdisIFaceIdTab = AT_PS_GET_NDIS_CALL_IFACE_ID_TBL_PTR();

    for (i = 0; i < AT_PS_GET_NDIS_CALL_IFACE_ID_TBL_SIZE(); i++)
    {
        if (ucCallId == pstNdisIFaceIdTab[i].ucCallIndex)
        {
            *penRmNetId = RNIC_DEV_ID_BUTT;
            *penIfaceId = pstNdisIFaceIdTab[i].enIfaceId;
            return VOS_OK;
        }
    }

    AT_WARN_LOG("AT_PS_GetNdisCallIFaceIdFromCallId: not find iface id");
    return VOS_ERR;
}


FC_ID_ENUM_UINT8 AT_PS_GetFcIdByIFaceId(
    VOS_UINT32                          ulIFaceId
)
{
    VOS_UINT32                                  i;
    AT_PS_FC_IFACE_ID_STRU                     *pstFcIFaceIdTab;

    pstFcIFaceIdTab = AT_PS_GET_FC_IFACE_ID_TBL_PTR();

    for (i = 0; i < AT_PS_GET_FC_IFACE_ID_TBL_SIZE(); i++)
    {
        if (ulIFaceId == pstFcIFaceIdTab[i].enIfaceId)
        {
            return pstFcIFaceIdTab[i].enFcId;
        }
    }

    AT_WARN_LOG("AT_PS_GetFcIdFromRnidRmNetId: WARNING: data channel id is abnormal.");
    return FC_ID_BUTT;
}


AT_PS_CALL_ENTITY_STRU* AT_PS_GetCallEntity(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId
)
{
    AT_MODEM_PS_CTX_STRU                  *pstPsModemCtx = VOS_NULL_PTR;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(usClientId);

    return &(pstPsModemCtx->astCallEntity[ucCallId]);
}


AT_PS_USER_INFO_STRU* AT_PS_GetUserInfo(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId
)
{
    AT_MODEM_PS_CTX_STRU                  *pstPsModemCtx = VOS_NULL_PTR;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(usClientId);

    return &(pstPsModemCtx->astCallEntity[ucCallId].stUserInfo);
}


AT_PS_DATA_CHANL_CFG_STRU* AT_PS_GetDataChanlCfg(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCid
)
{
    AT_MODEM_PS_CTX_STRU                  *pstPsModemCtx        = VOS_NULL_PTR;

    pstPsModemCtx       = AT_GetModemPsCtxAddrFromClientId(usClientId);

    return &(pstPsModemCtx->astChannelCfg[ucCid]);
}


VOS_UINT32 AT_PS_GetRabIdByRabIdMask(
    VOS_UINT32                          ulRabIdMask,
    VOS_UINT8                          *pucRabId
)
{
    VOS_UINT32                          ulRabIdEx;

    for (ulRabIdEx = AT_PS_MIN_RABID; ulRabIdEx <= AT_PS_MAX_RABID; ulRabIdEx++)
    {
        /* 当前Bit位为1，则此位为对应的RabId */
        if (0 != (ulRabIdMask & ((VOS_UINT32)0x00000001 << ulRabIdEx)))
        {
            *pucRabId = (VOS_UINT8)ulRabIdEx;
            return VOS_OK;
        }
    }

    *pucRabId = AT_PS_INVALID_RABID;

    return VOS_ERR;
}


VOS_VOID AT_PS_UpdateCallIFaceId(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    VOS_UINT8                           ucCallId
)
{
    VOS_UINT8                                  *pucSystemAppConfig  = VOS_NULL_PTR;
    AT_PS_DATA_CHANL_CFG_STRU                  *pstChanCfg          = VOS_NULL_PTR;
    RNIC_DEV_ID_ENUM_UINT8                      enRmNetId;
    PS_IFACE_ID_ENUM_UINT8                      enIfaceId;

    pucSystemAppConfig  = AT_GetSystemAppConfigAddr();
    enRmNetId           = RNIC_DEV_ID_BUTT;
    enIfaceId           = PS_IFACE_ID_BUTT;

    /* 保存网卡ID*/
    if (AT_PS_WAN_TYPE_APP == pstCallEntity->enPsCallType)
    {
        pstChanCfg = AT_PS_GetDataChanlCfg(pstCallEntity->stUserInfo.enPortIndex,
                                           pstCallEntity->stUserInfo.ucUsrCid);

        /* 手机形态动态分配 */
        if (SYSTEM_APP_WEBUI != *pucSystemAppConfig)
        {

            pstCallEntity->ucIfaceId      = (VOS_UINT8)pstChanCfg->ulIfaceId;
            pstCallEntity->ucRmNetId      = (VOS_UINT8)pstChanCfg->ulRmNetId;
            return;
        }

        /* WEBUI APP形态有动态分配的使用动态分配的，否则使用静态的 */
        if ( (VOS_TRUE == pstChanCfg->ulUsed)
          && (AT_PS_INVALID_RMNET_ID != pstChanCfg->ulRmNetId) )
        {
            pstCallEntity->ucIfaceId      = (VOS_UINT8)pstChanCfg->ulIfaceId;
            pstCallEntity->ucRmNetId      = (VOS_UINT8)pstChanCfg->ulRmNetId;
            return;
        }

        if (VOS_OK == AT_PS_GetAppCallRnicIFaceIdFromCallId(ucCallId, &enRmNetId, &enIfaceId))
        {
            pstCallEntity->ucIfaceId      = (VOS_UINT8)enIfaceId;
            pstCallEntity->ucRmNetId      = (VOS_UINT8)enRmNetId;
            return;
        }

        return;
    }

    if (AT_PS_WAN_TYPE_NDIS == pstCallEntity->enPsCallType)
    {
        /* NDIS使用静态的 */
        if (VOS_OK == AT_PS_GetNdisCallIFaceIdFromCallId(ucCallId, &enRmNetId, &enIfaceId))
        {
            pstCallEntity->ucIfaceId      = (VOS_UINT8)enIfaceId;
            pstCallEntity->ucRmNetId      = (VOS_UINT8)enRmNetId;
        }
    }

    return;
}

VOS_UINT8 AT_PS_TransCidToCallId(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCid
)
{
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;

    /* cid 无效值 */
    if (ucCid > TAF_MAX_CID)
    {
        return AT_PS_CALL_INVALID_CALLID;
    }

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(usClientId);

    return pstPsModemCtx->aucCidToIndexTbl[ucCid];
}


TAF_PS_CALL_END_CAUSE_ENUM_UINT8 AT_PS_TransCallEndCause(
    VOS_UINT8                           ucConnectType
)
{
    TAF_PS_CALL_END_CAUSE_ENUM_UINT8    enCause;

    enCause = TAF_PS_CALL_END_CAUSE_NORMAL;

    if (TAF_PS_CALL_TYPE_DOWN_FORCE == ucConnectType)
    {
        enCause = TAF_PS_CALL_END_CAUSE_FORCE;
    }

    return enCause;
}


VOS_UINT32 AT_PS_IsCallIdValid(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId
)
{
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;

    if (ucCallId >= AT_PS_MAX_CALL_NUM)
    {
        return VOS_FALSE;
    }

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(usClientId);

    if ((AT_PS_CALL_INVALID_CID == pstPsModemCtx->astCallEntity[ucCallId].stIpv4Info.ucIpv4Cid)
     && (AT_PS_CALL_INVALID_CID == pstPsModemCtx->astCallEntity[ucCallId].stIpv6Info.ucIpv6Cid))
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}


VOS_UINT32 AT_PS_IsNeedSndCallEnded(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    TAF_PS_APN_DATA_SYS_ENUM_UINT8      enDataSys
)
{
    /* 该CALL没有发起激活，默认可以上报 */
    if (TAF_PS_APN_DATA_SYS_NONE == enDataSys)
    {
        AT_NORM_LOG("AT_PS_IsNeedSndCallEnded: this call is not dailup!");
        return VOS_TRUE;
    }

    /* 该CALL的APN没有配置DATA SYSTEM策略信息，默认可以上报 */
    if (TAF_PS_APN_CURRENT_DOMAIN_BUTT == AT_PS_GetPsCallCurrentDataSys(pstCallEntity))
    {
        AT_NORM_LOG("AT_PS_IsNeedSndCallEnded: this call apn is not data sys config!");
        return VOS_TRUE;
    }

    /* 该CALL的APN配置DATA SYSTEM策略信息，且是当前域的返回结果，则需要上报 */
    if (enDataSys == AT_PS_GetPsCallCurrentDataSys(pstCallEntity))
    {
        AT_NORM_LOG("AT_PS_IsNeedSndCallEnded: this call apn is data sys config, and need report!");
        return VOS_TRUE;
    }

    /* 该CALL的APN配置DATA SYSTEM策略信息，但是不是当前域的返回结果，则不需要上报 */
    AT_NORM_LOG("AT_PS_IsNeedSndCallEnded: this call apn is data sys config, and not need report!");
    return VOS_FALSE;
}


VOS_UINT32 AT_PS_IsPdpTypeStateAllIdle(
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    AT_PDP_STATE_ENUM_U8                enIpv4State,
    AT_PDP_STATE_ENUM_U8                enIpv6State
)
{
    if ( ( (TAF_PDP_IPV4 == enPdpType)
        || (TAF_PDP_IPV4V6 == enPdpType))
      && (AT_PDP_STATE_IDLE != enIpv4State))
    {
        AT_NORM_LOG("AT_PS_IsPdpTypeStateAllIdle: IPV4 state is not idle!");
        return VOS_FALSE;
    }

    if ( ( (TAF_PDP_IPV6 == enPdpType)
        || (TAF_PDP_IPV4V6 == enPdpType))
      && (AT_PDP_STATE_IDLE != enIpv6State))
    {
        AT_NORM_LOG("AT_PS_IsPdpTypeStateAllIdle: IPV6 state is not idle!");
        return VOS_FALSE;
    }

    AT_NORM_LOG("AT_PS_IsPdpTypeStateAllIdle: IP state is all idle!");
    return VOS_TRUE;
}


VOS_UINT32 AT_PS_IsNeedSetInvalidCid(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    TAF_PS_APN_DATA_SYS_ENUM_UINT8      enDataSys
)
{
    VOS_UINT32                          ulRst;

    ulRst = VOS_TRUE;

    /* 该CALL没有发起激活，默认可以设置为无效CID */
    if (TAF_PS_APN_DATA_SYS_NONE == enDataSys)
    {
        AT_NORM_LOG("AT_PS_IsNeedSetInvalidCid: this call is not dailup!");
        return VOS_TRUE;
    }

    /* 该CALL的APN没有定制，默认可以设置为无效CID */
    if (VOS_FALSE == pstCallEntity->stApnDataSysInfo.ucDataSysInfoFlg)
    {
        AT_NORM_LOG("AT_PS_IsNeedSetInvalidCid: this call apn is not data sys config, and need set invalid cid!");
        return VOS_TRUE;
    }

    if (TAF_PS_APN_DATA_SYS_CELLULAR == enDataSys)
    {
        ulRst = AT_PS_IsPdpTypeStateAllIdle(enPdpType, pstCallEntity->stIpv4Info.enWlanIpv4State, pstCallEntity->stIpv6Info.enWlanIpv6State);
    }
    else if (TAF_PS_APN_DATA_SYS_WLAN == enDataSys)
    {
       ulRst = AT_PS_IsPdpTypeStateAllIdle(enPdpType, pstCallEntity->stIpv4Info.enIpv4State, pstCallEntity->stIpv6Info.enIpv6State);
    }
    else
    {
        /* nothing to do */
        AT_ERR_LOG("AT_PS_IsNeedSetInvalidCid: enDataSys is error!");
    }

    return ulRst;
}


VOS_UINT32 AT_PS_IsNeedClearCurrCall(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    TAF_PS_APN_DATA_SYS_ENUM_UINT8      enDataSys
)
{
    /* 该CALL没有发起激活，默认可以清除 */
    if (TAF_PS_APN_DATA_SYS_NONE == enDataSys)
    {
        AT_NORM_LOG("AT_PS_IsNeedClearCurrCall: this call is not dailup!");
        return VOS_TRUE;
    }

    /* 该CALL的APN没有定制，默认可以清除 */
    if (VOS_FALSE == pstCallEntity->stApnDataSysInfo.ucDataSysInfoFlg)
    {
        AT_NORM_LOG("AT_PS_IsNeedClearCurrCall: this call apn is not data sys config, and need clear!");
        return VOS_TRUE;
    }

    /* 该CALL WLAN上流程还没有结束，不可以清除 */
    if ( (TAF_PS_APN_DATA_SYS_CELLULAR == enDataSys)
      && ( (AT_PDP_STATE_IDLE != pstCallEntity->stIpv4Info.enWlanIpv4State)
        || (AT_PDP_STATE_IDLE != pstCallEntity->stIpv6Info.enWlanIpv6State)))
    {
        AT_NORM_LOG("AT_PS_IsNeedClearCurrCall: this call has wlan state, and not need clear!");
        return VOS_FALSE;
    }

    /* 该CALL CELLULAR上流程还没有结束，不可以清除 */
    if ( (TAF_PS_APN_DATA_SYS_WLAN == enDataSys)
      && ( (AT_PDP_STATE_IDLE != pstCallEntity->stIpv4Info.enIpv4State)
        || (AT_PDP_STATE_IDLE != pstCallEntity->stIpv6Info.enIpv6State)))
    {
        AT_NORM_LOG("AT_PS_IsNeedClearCurrCall: this call has cellular state, and not need clear!");
        return VOS_FALSE;
    }

    AT_NORM_LOG("AT_PS_IsNeedClearCurrCall: this call apn is data sys config, and need clear!");
    return VOS_TRUE;
}


VOS_UINT32  AT_PS_IsUsrDialTypeDualStack(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
)
{
    if (VOS_FALSE == AT_PS_GetPsCallHandOverFlg(pstCallEntity))
    {
        if (TAF_PDP_IPV4V6 == pstCallEntity->stUsrDialParam.enPdpType)
        {
            return VOS_TRUE;
        }
    }
    else
    {
        if (TAF_PDP_IPV4V6 == pstCallEntity->stDialPdpType.enHoPdpType)
        {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}


VOS_VOID AT_PS_AssignCallIdToCid(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCid,
    VOS_UINT8                           ucCallId
)
{
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(usClientId);

    pstPsModemCtx->aucCidToIndexTbl[ucCid] = ucCallId;

    return;
}


VOS_VOID AT_PS_FreeCallIdToCid(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCid,
    VOS_UINT8                           ucCallId,
    TAF_PS_APN_DATA_SYS_ENUM_UINT8      enDataSys
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(usClientId);
    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);

    if (VOS_FALSE == AT_PS_IsNeedClearCurrCall(pstCallEntity, enDataSys))
    {
        AT_NORM_LOG("AT_PS_FreeCallIdToCid: not need FreeCallIdToCid!");
        return;
    }

    pstPsModemCtx->aucCidToIndexTbl[ucCid] = AT_PS_CALL_INVALID_CALLID;

    return;
}


VOS_VOID AT_PS_SetCid2CurrCall(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    VOS_UINT8                           ucCid,
    TAF_PS_APN_DATA_SYS_ENUM_UINT8      enDataSys
)
{
    /* 呼叫实体索引(CallId)由调用者保证其有效性 */

    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);

    if ( (AT_PS_CALL_INVALID_CID == ucCid)
      && (VOS_FALSE == AT_PS_IsNeedSetInvalidCid(pstCallEntity, enPdpType, enDataSys)))
    {
        AT_NORM_LOG("AT_PS_SetCid2CurrCall: not need Set Invalid Cid!");
        return;
    }

    switch (enPdpType)
    {
        case TAF_PDP_IPV4:
            pstCallEntity->stIpv4Info.ucIpv4Cid = ucCid;
            break;

        case TAF_PDP_IPV6:
            pstCallEntity->stIpv6Info.ucIpv6Cid = ucCid;
            break;

        case TAF_PDP_IPV4V6:
            pstCallEntity->stIpv4Info.ucIpv4Cid = ucCid;
            pstCallEntity->stIpv6Info.ucIpv6Cid = ucCid;
            break;

        default:
            AT_ERR_LOG("AT_PS_SetCid2CurrCall: PDP type is invalid!");
            break;
    }

    return;
}


VOS_UINT8 AT_PS_GetCidByCallType(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
)
{
    /* 呼叫实体索引(CallId)由调用者保证其有效性 */
    VOS_UINT8                           ucCid;

    switch (enPdpType)
    {
        case TAF_PDP_IPV4:
            ucCid = pstCallEntity->stIpv4Info.ucIpv4Cid;
            break;

        case TAF_PDP_IPV6:
            ucCid = pstCallEntity->stIpv6Info.ucIpv6Cid;
            break;

        default:
            ucCid = AT_PS_CALL_INVALID_CID;
            AT_ERR_LOG("AT_PS_SetCid2CurrCall: PDP type is invalid!");
            break;
    }

    return ucCid;
}


VOS_VOID AT_PS_SetCallStateByType(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    AT_PDP_STATE_ENUM_U8                enPdpState
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);

    switch (enPdpType)
    {
        case TAF_PDP_IPV4:
            pstCallEntity->stIpv4Info.enIpv4State = enPdpState;
            break;

        case TAF_PDP_IPV6:
            pstCallEntity->stIpv6Info.enIpv6State = enPdpState;
            break;

        case TAF_PDP_IPV4V6:
            pstCallEntity->stIpv4Info.enIpv4State = enPdpState;
            pstCallEntity->stIpv6Info.enIpv6State = enPdpState;
            break;

        default:
            AT_ERR_LOG("AT_PS_SetCallStateByType: PDP type is invalid!");
            break;
    }

    return;
}


AT_PDP_STATE_ENUM_U8 AT_PS_GetCallStateByType(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
)
{
    AT_PDP_STATE_ENUM_U8                enPdpState;

    switch (enPdpType)
    {
        case TAF_PDP_IPV4:
            enPdpState = pstCallEntity->stIpv4Info.enIpv4State;
            break;

        case TAF_PDP_IPV6:
            enPdpState = pstCallEntity->stIpv6Info.enIpv6State;
            break;

        default:
            enPdpState = AT_PDP_STATE_IDLE;
            AT_ERR_LOG("AT_PS_GetCallStateByType: PDP type is invalid!");
            break;
    }

    return enPdpState;
}


AT_PDP_STATE_ENUM_U8 AT_PS_GetCallStateByCid(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    VOS_UINT8                           ucCid
)
{
    if (pstCallEntity->stIpv4Info.ucIpv4Cid == ucCid)
    {
        return pstCallEntity->stIpv4Info.enIpv4State;
    }

    if (pstCallEntity->stIpv6Info.ucIpv6Cid == ucCid)
    {
        return pstCallEntity->stIpv6Info.enIpv6State;
    }

    return AT_PDP_STATE_IDLE;
}


TAF_PDP_TYPE_ENUM_UINT8 AT_PS_GenPdpTypeByCid(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    VOS_UINT8                           ucCid
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);

    if ((AT_PDP_STATE_ACTED == pstCallEntity->stIpv4Info.enIpv4State)
     && (AT_PDP_STATE_ACTED == pstCallEntity->stIpv6Info.enIpv6State)
     && (ucCid == pstCallEntity->stIpv4Info.ucIpv4Cid)
     && (ucCid == pstCallEntity->stIpv6Info.ucIpv6Cid))
    {
        return TAF_PDP_IPV4V6;
    }

    if ((AT_PDP_STATE_ACTED == pstCallEntity->stIpv4Info.enIpv4State)
     && (ucCid == pstCallEntity->stIpv4Info.ucIpv4Cid))
    {
        return TAF_PDP_IPV4;
    }

    if ((AT_PDP_STATE_ACTED == pstCallEntity->stIpv6Info.enIpv6State)
     && (ucCid == pstCallEntity->stIpv6Info.ucIpv6Cid))
    {
        return TAF_PDP_IPV6;
    }

    return TAF_PDP_TYPE_BUTT;
}


VOS_VOID AT_PS_ReportNdisStat(
    AT_PS_USER_INFO_STRU               *pstUsrInfo,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    AT_PDP_STATUS_ENUM_UINT32           enStat,
    TAF_PS_CAUSE_ENUM_UINT32            enCause
)
{
    VOS_UINT32                          ul3gppSmCause;
    VOS_UINT16                          usLength;

    usLength      = 0;

    ul3gppSmCause = AT_Get3gppSmCauseByPsCause(enCause);

    if (AT_PDP_STATUS_DEACT == enStat)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s%s:0,%d,,",
                                           gaucAtCrLf,
                                           gastAtStringTab[AT_STRING_NDISSTAT].pucText,
                                           ul3gppSmCause);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s%s:1,,,",
                                           gaucAtCrLf,
                                           gastAtStringTab[AT_STRING_NDISSTAT].pucText);
    }

    switch (enPdpType)
    {
        case TAF_PDP_IPV4:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "\"IPV4\"%s",
                                               gaucAtCrLf);
            break;

        case TAF_PDP_IPV6:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "\"IPV6\"%s",
                                               gaucAtCrLf);
            break;

        case TAF_PDP_IPV4V6:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "\"IPV4\"%s",
                                               gaucAtCrLf);
            if (AT_PDP_STATUS_DEACT == enStat)
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%s%s:0,%d,,\"IPV6\"%s",
                                                   gaucAtCrLf,
                                                   gastAtStringTab[AT_STRING_NDISSTAT].pucText,
                                                   ul3gppSmCause,
                                                   gaucAtCrLf);
            }
            else
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%s%s:1,,,\"IPV6\"%s",
                                                   gaucAtCrLf,
                                                   gastAtStringTab[AT_STRING_NDISSTAT].pucText,
                                                   gaucAtCrLf);
            }
            break;

        default:
            usLength = 0;
            AT_ERR_LOG("AT_PS_ReportNdisStat: PDP type is invalid in ^NDISSTAT.");
            return;
    }

    At_SendResultData(pstUsrInfo->enPortIndex, pgucAtSndCodeAddr, usLength);

    return;
}


VOS_VOID AT_PS_ReportNdisStatEx(
    AT_PS_USER_INFO_STRU               *pstUsrInfo,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    AT_PDP_STATUS_ENUM_UINT32           enStat,
    TAF_PS_CAUSE_ENUM_UINT32            enCause
)
{
    VOS_UINT32                          ul3gppSmCause;
    VOS_UINT16                          usLength;

    usLength      = 0;

    ul3gppSmCause = AT_Get3gppSmCauseByPsCause(enCause);

    if (AT_PDP_STATUS_DEACT == enStat)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s%s:%d,0,%d,,",
                                           gaucAtCrLf,
                                           gastAtStringTab[AT_STRING_NDISSTATEX].pucText,
                                           pstUsrInfo->ucUsrCid,
                                           ul3gppSmCause);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s%s:%d,1,,,",
                                           gaucAtCrLf,
                                           gastAtStringTab[AT_STRING_NDISSTATEX].pucText,
                                           pstUsrInfo->ucUsrCid);
    }

    switch (enPdpType)
    {
        case TAF_PDP_IPV4:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "\"IPV4\"%s",
                                               gaucAtCrLf);
            break;

        case TAF_PDP_IPV6:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "\"IPV6\"%s",
                                               gaucAtCrLf);
            break;

        case TAF_PDP_IPV4V6:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "\"IPV4\"%s",
                                               gaucAtCrLf);

            if (AT_PDP_STATUS_DEACT == enStat)
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%s%s:%d,0,%d,,\"IPV6\"%s",
                                                   gaucAtCrLf,
                                                   gastAtStringTab[AT_STRING_NDISSTATEX].pucText,
                                                   pstUsrInfo->ucUsrCid,
                                                   ul3gppSmCause,
                                                   gaucAtCrLf);
            }
            else
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%s%s:%d,1,,,\"IPV6\"%s",
                                                   gaucAtCrLf,
                                                   gastAtStringTab[AT_STRING_NDISSTATEX].pucText,
                                                   pstUsrInfo->ucUsrCid,
                                                   gaucAtCrLf);
            }
            break;

        default:
            usLength = 0;
            AT_ERR_LOG("AT_PS_ReportNdisStat: PDP type is invalid in ^NDISSTAT.");
            return;
    }

    At_SendResultData(pstUsrInfo->enPortIndex, pgucAtSndCodeAddr, usLength);

    return;
}



VOS_VOID AT_PS_ReportDCONN(
    AT_PS_USER_INFO_STRU               *pstUsrInfo,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
)
{
    VOS_UINT8                          *pucSystemAppConfig;
    VOS_UINT16                          usLength;

    usLength = 0;
    pucSystemAppConfig                  = AT_GetSystemAppConfigAddr();

    /* 处理E5、闪电卡、E355等形态的拨号 */
    /* PCUI口下发上报^NDISSTAT，APP口下发上报^NDISSTATEX */
    if (SYSTEM_APP_WEBUI == *pucSystemAppConfig)
    {
        if (VOS_TRUE == AT_CheckAppUser(pstUsrInfo->enPortIndex))
        {
            AT_PS_ReportNdisStatEx(pstUsrInfo,
                                   enPdpType,
                                   AT_PDP_STATUS_ACT,
                                   TAF_PS_CAUSE_SUCCESS);

            return;
        }

        AT_PS_ReportNdisStatConn(pstUsrInfo, enPdpType);
        return;
    }

    switch (enPdpType)
    {
        case TAF_PDP_IPV4:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^DCONN:%d,\"IPV4\"%s",
                                               gaucAtCrLf,
                                               pstUsrInfo->ucUsrCid,
                                               gaucAtCrLf);
            break;

        case TAF_PDP_IPV6:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^DCONN:%d,\"IPV6\"%s",
                                               gaucAtCrLf,
                                               pstUsrInfo->ucUsrCid,
                                               gaucAtCrLf);
            break;

        case TAF_PDP_IPV4V6:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^DCONN:%d,\"IPV4\"%s",
                                               gaucAtCrLf,
                                               pstUsrInfo->ucUsrCid,
                                               gaucAtCrLf);
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^DCONN:%d,\"IPV6\"%s",
                                               gaucAtCrLf,
                                               pstUsrInfo->ucUsrCid,
                                               gaucAtCrLf);
            break;

        default:
            AT_ERR_LOG("AT_PS_ReportDCONN: PDP type is invalid in ^DCONN.");
            return;
    }

    At_SendResultData(pstUsrInfo->enPortIndex, pgucAtSndCodeAddr, usLength);

    return;
}


VOS_VOID AT_PS_ReportNdisStatConn(
    AT_PS_USER_INFO_STRU               *pstUsrInfo,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
)
{
    AT_PS_ReportNdisStat(pstUsrInfo,
                         enPdpType,
                         AT_PDP_STATUS_ACT,
                         TAF_PS_CAUSE_SUCCESS);

    return;
}


VOS_VOID AT_PS_ReportDEND(
    AT_PS_USER_INFO_STRU               *pstUsrInfo,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    TAF_PS_CAUSE_ENUM_UINT32            enCause
)
{
    VOS_UINT8                          *pucSystemAppConfig;
    VOS_UINT16                          usLength;

    usLength = 0;
    pucSystemAppConfig                  = AT_GetSystemAppConfigAddr();

    /* 处理E5、闪电卡、E355等形态的拨号 */
    /* PCUI口下发上报^NDISSTAT，APP口下发上报^NDISSTATEX */
    if (SYSTEM_APP_WEBUI == *pucSystemAppConfig)
    {
        if (VOS_TRUE == AT_CheckAppUser(pstUsrInfo->enPortIndex))
        {
            AT_PS_ReportNdisStatEx(pstUsrInfo,
                                   enPdpType,
                                   AT_PDP_STATUS_DEACT,
                                   enCause);

            return;
        }

        AT_PS_ReportNdisStatEnd(pstUsrInfo, enPdpType, enCause);
        return;
    }

    switch (enPdpType)
    {
        case TAF_PDP_IPV4:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^DEND:%d,%d,\"IPV4\"%s",
                                               gaucAtCrLf,
                                               pstUsrInfo->ucUsrCid,
                                               enCause,
                                               gaucAtCrLf);

            break;
        case TAF_PDP_IPV6:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^DEND:%d,%d,\"IPV6\"%s",
                                               gaucAtCrLf,
                                               pstUsrInfo->ucUsrCid,
                                               enCause,
                                               gaucAtCrLf);
            break;

        case TAF_PDP_IPV4V6:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^DEND:%d,%d,\"IPV4\"%s",
                                               gaucAtCrLf,
                                               pstUsrInfo->ucUsrCid,
                                               enCause,
                                               gaucAtCrLf);
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^DEND:%d,%d,\"IPV6\"%s",
                                               gaucAtCrLf,
                                               pstUsrInfo->ucUsrCid,
                                               enCause,
                                               gaucAtCrLf);
            break;

        default:
            AT_ERR_LOG("AT_PS_ReportDEND: PDP type is invalid in ^DEND.");
            return;
    }

    At_SendResultData(pstUsrInfo->enPortIndex, pgucAtSndCodeAddr, usLength);

    return;
}


VOS_VOID AT_PS_ReportNdisStatEnd(
    AT_PS_USER_INFO_STRU               *pstUsrInfo,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    TAF_PS_CAUSE_ENUM_UINT32            enCause
)
{
    AT_PS_ReportNdisStat(pstUsrInfo,
                         enPdpType,
                         AT_PDP_STATUS_DEACT,
                         enCause);

    return;
}


AT_PS_RPT_CONN_RSLT_FUNC AT_PS_GetRptConnResultFunc(AT_USER_TYPE ucUsrType)
{
    CONST AT_PS_REPORT_CONN_RESULT_STRU    *pstRptConnRsltFuncTblPtr = VOS_NULL_PTR;
    AT_PS_RPT_CONN_RSLT_FUNC                pRptConnRsltFunc         = VOS_NULL_PTR;
    VOS_UINT32                              ulCnt;

    pstRptConnRsltFuncTblPtr = AT_PS_GET_RPT_CONN_RSLT_FUNC_TBL_PTR();

    /* 用户类型匹配 */
    for (ulCnt = 0; ulCnt < AT_PS_GET_RPT_CONN_RSLT_FUNC_TBL_SIZE(); ulCnt++)
    {
        if (ucUsrType == pstRptConnRsltFuncTblPtr[ulCnt].ucUsrType)
        {
            pRptConnRsltFunc = pstRptConnRsltFuncTblPtr[ulCnt].pRptConnRsltFunc;
            break;
        }
    }

    return pRptConnRsltFunc;
}


AT_PS_RPT_END_RSLT_FUNC AT_PS_GetRptEndResultFunc(AT_USER_TYPE ucUsrType)
{
    CONST AT_PS_REPORT_END_RESULT_STRU *pstRptEndRsltFuncTblPtr = VOS_NULL_PTR;
    AT_PS_RPT_END_RSLT_FUNC             pRptEndRsltFunc         = VOS_NULL_PTR;
    VOS_UINT32                          ulCnt;

    pstRptEndRsltFuncTblPtr = AT_PS_GET_RPT_END_RSLT_FUNC_TBL_PTR();

    /* 用户类型匹配 */
    for (ulCnt = 0; ulCnt < AT_PS_GET_RPT_END_RSLT_FUNC_TBL_SIZE(); ulCnt++)
    {
        if (ucUsrType == pstRptEndRsltFuncTblPtr[ulCnt].ucUsrType)
        {
            pRptEndRsltFunc = pstRptEndRsltFuncTblPtr[ulCnt].pRptEndRsltFunc;
            break;
        }
    }

    return pRptEndRsltFunc;
}


VOS_VOID AT_PS_PdpAddrProc(
    VOS_UINT8                           ucCallId,
    AT_DHCP_CONFIG_STRU                *pstDhcpConfig,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity   = VOS_NULL_PTR;
    AT_DIAL_PARAM_STRU                 *pstDialPara     = VOS_NULL_PTR;
    VOS_UINT32                          ulIpAddr;
    VOS_UINT8                           ucIndex;

    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);
    pstDialPara   = &pstCallEntity->stUsrDialParam;
    ulIpAddr      = 0;

    pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ucRabId                 = pstEvent->ucRabId;
    pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ucPduSessionId          = pstEvent->ucPduSessionId;
    pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4Addr              = pstDhcpConfig->ulIPAddr;
    pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4GateWay           = pstDhcpConfig->ulGateWay;
    pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4NetMask           = pstDhcpConfig->ulSubNetMask;

    /* 如果用户设置了主DNS，就使用用户设置的DNS，网络返回的DNS不使用 */
    if (VOS_TRUE == pstDialPara->ulPrimIPv4DNSValidFlag)
    {
        if (AT_FAILURE == AT_LenStr2IpAddr(pstDialPara->aucPrimIPv4DNSAddr,
                                           (VOS_UINT8*)&ulIpAddr))
        {
            ulIpAddr = 0;
        }

        pstCallEntity->stIpv4Info.stIpv4DhcpInfo.bitOpIpv4PriDns     = VOS_TRUE;
        pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4PrimDNS       = VOS_NTOHL(ulIpAddr);
    }
    else
    {
        if (0 != pstDhcpConfig->ulPrimDNS)
        {
            pstCallEntity->stIpv4Info.stIpv4DhcpInfo.bitOpIpv4PriDns = VOS_TRUE;
            pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4PrimDNS   = pstDhcpConfig->ulPrimDNS;
        }
        else
        {
            pstCallEntity->stIpv4Info.stIpv4DhcpInfo.bitOpIpv4PriDns = VOS_FALSE;
        }
    }

    /* 如果用户设置了副DNS，就使用用户设置的DNS，网络返回的DNS不使用 */
    if (VOS_TRUE == pstDialPara->ulSndIPv4DNSValidFlag)
    {
        if (AT_FAILURE == AT_LenStr2IpAddr(pstDialPara->aucSndIPv4DNSAddr,
                                           (VOS_UINT8*)&ulIpAddr))
        {
            ulIpAddr = 0;
        }

        pstCallEntity->stIpv4Info.stIpv4DhcpInfo.bitOpIpv4SecDns     = VOS_TRUE;
        pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4SecDNS        = VOS_NTOHL(ulIpAddr);
    }
    else
    {
        if (0 != pstDhcpConfig->ulSndDNS)
        {
            pstCallEntity->stIpv4Info.stIpv4DhcpInfo.bitOpIpv4SecDns = VOS_TRUE;
            pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4SecDNS    = pstDhcpConfig->ulSndDNS;
        }
        else
        {
            pstCallEntity->stIpv4Info.stIpv4DhcpInfo.bitOpIpv4SecDns = VOS_FALSE;
        }
    }

    pstCallEntity->stIpv4Info.stIpv4DhcpInfo.bitOpIpv4PriWINNS       = VOS_FALSE;
    pstCallEntity->stIpv4Info.stIpv4DhcpInfo.bitOpIpv4SecWINNS       = VOS_FALSE;

    /* 获取主PCSCF地址 */
    pstCallEntity->stIpv4Info.stIpv4DhcpInfo.stIpv4PcscfList.ucIpv4PcscfAddrNum = AT_MIN(TAF_PCSCF_ADDR_MAX_NUM,
                                                                              pstEvent->stIpv4PcscfList.ucIpv4PcscfAddrNum);
    for (ucIndex = 0; ucIndex < pstEvent->stIpv4PcscfList.ucIpv4PcscfAddrNum; ucIndex++)
    {
        pstCallEntity->stIpv4Info.stIpv4DhcpInfo.stIpv4PcscfList.aulIpv4PcscfAddrList[ucIndex] = AT_GetLanAddr32(pstEvent->stIpv4PcscfList.astIpv4PcscfAddrList[ucIndex].aucPcscfAddr);
    }

    return;
}


VOS_VOID AT_PS_ProcConnInd(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    AT_DHCP_SETUP_PARAM_ST              stParam;
    AT_DHCP_CONFIG_STRU                 stConfig;

    TAF_MEM_SET_S(&stParam, sizeof(stParam), 0x00, sizeof(AT_DHCP_SETUP_PARAM_ST));
    TAF_MEM_SET_S(&stConfig, sizeof(stConfig), 0x00, sizeof(AT_DHCP_CONFIG_STRU));

    /* 如果PS事件有DNS地址，则记录DNS地址*/
    if (pstEvent->stDns.bitOpPrimDnsAddr)
    {
        stParam.ulPrimDNS = AT_GetLanAddr32(pstEvent->stDns.aucPrimDnsAddr);
    }

    if (pstEvent->stDns.bitOpSecDnsAddr)
    {
        stParam.ulSndDNS = AT_GetLanAddr32(pstEvent->stDns.aucSecDnsAddr);
    }

    /* 记录IP地址*/
    stParam.ulIPAddr = AT_GetLanAddr32(pstEvent->stPdpAddr.aucIpv4Addr);

    if (VOS_ERR == AT_DHCPServerSetUp(&stParam, &stConfig))
    {
        AT_ERR_LOG("AT_PS_ProcConnInd : ERROR AT_DHCPServerSetUp Error!" );
    }
    else
    {
        /* 处理IPv4地址参数 */
        AT_PS_PdpAddrProc(ucCallId, &stConfig, pstEvent);
    }

    return;
}


VOS_VOID AT_PS_SndCallConnectedResult(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
)
{
    AT_PS_USER_INFO_STRU               *pstUsrInfo          = VOS_NULL_PTR;
    AT_PS_RPT_CONN_RSLT_FUNC            pRptConnRsltFunc    = VOS_NULL_PTR;

    /* 获取当前呼叫用户信息 */
    pstUsrInfo       = AT_PS_GetUserInfo(usClientId, ucCallId);

    /* 在事件处理表中查找处理函数 */
    pRptConnRsltFunc = AT_PS_GetRptConnResultFunc(pstUsrInfo->ucUsrType);

    /* 如果处理函数存在则调用 */
    if (VOS_NULL_PTR != pRptConnRsltFunc)
    {
        pRptConnRsltFunc(pstUsrInfo,enPdpType);
    }
    else
    {
        AT_ERR_LOG("AT_PS_SndCallConnectedResult:ERROR: User type is invalid!");
    }

    return;
}


VOS_UINT32 AT_PS_CallEndedReportAlready(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    TAF_PDP_TYPE_ENUM_UINT8            *penPdpType
)
{
    if (TAF_PDP_IPV4 == *penPdpType)
    {
        if (VOS_TRUE == pstCallEntity->stIpv4Info.ucIpv4DendRptFlg)
        {
            AT_NORM_LOG("AT_PS_CallEndedReportAlready: Has already report Ipv4 Dend!");
            return VOS_TRUE;
        }

        pstCallEntity->stIpv4Info.ucIpv4DendRptFlg = VOS_TRUE;
    }
    else if (TAF_PDP_IPV6 == *penPdpType)
    {
        if (VOS_TRUE == pstCallEntity->stIpv6Info.ucIpv6DendRptFlg)
        {
            AT_NORM_LOG("AT_PS_CallEndedReportAlready: Has already report Ipv6 Dend!");
            return VOS_TRUE;
        }

        pstCallEntity->stIpv6Info.ucIpv6DendRptFlg = VOS_TRUE;
    }
    else if (TAF_PDP_IPV4V6 == *penPdpType)
    {
        if ( (VOS_TRUE == pstCallEntity->stIpv4Info.ucIpv4DendRptFlg)
          && (VOS_TRUE == pstCallEntity->stIpv6Info.ucIpv6DendRptFlg))
        {
            AT_NORM_LOG("AT_PS_CallEndedReportAlready: Has already report IPV4 Ipv6 DCONN!");
            return VOS_TRUE;
        }

        if ( (VOS_TRUE == pstCallEntity->stIpv4Info.ucIpv4DendRptFlg)
          || (VOS_TRUE == pstCallEntity->stIpv6Info.ucIpv6DendRptFlg))
        {
            *penPdpType = (VOS_TRUE == pstCallEntity->stIpv4Info.ucIpv4DendRptFlg) ? TAF_PDP_IPV6 : TAF_PDP_IPV4;
        }

        pstCallEntity->stIpv4Info.ucIpv4DendRptFlg = VOS_TRUE;
        pstCallEntity->stIpv6Info.ucIpv6DendRptFlg = VOS_TRUE;
    }
    else
    {
        /* nothing to do */
        AT_ERR_LOG("AT_PS_CallEndedReportAlready: Pdp Type is error!");
    }

    return VOS_FALSE;
}


VOS_VOID AT_PS_ChgCallEndedCause(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    TAF_PS_CAUSE_ENUM_UINT32           *penCause
)
{
    TAF_PS_CAUSE_ENUM_UINT32            enHoFailCause;

    enHoFailCause   = TAF_PS_CAUSE_SUCCESS;

    enHoFailCause = AT_PS_GetCallHandOverFailCause(pstCallEntity);

    if (TAF_PS_CAUSE_SUCCESS != enHoFailCause)
    {
        *penCause = enHoFailCause;
    }

    return;
}


VOS_VOID AT_PS_SndCallEndedResult(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    TAF_PS_CAUSE_ENUM_UINT32            enCause,
    TAF_PS_APN_DATA_SYS_ENUM_UINT8      enDataSys
)
{
    AT_PS_USER_INFO_STRU               *pstUsrInfo      = VOS_NULL_PTR;
    AT_PS_RPT_END_RSLT_FUNC             pRptEndRsltFunc = VOS_NULL_PTR;
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity   = VOS_NULL_PTR;

    /* 获取当前呼叫用户信息 */
    pstUsrInfo      = AT_PS_GetUserInfo(usClientId, ucCallId);
    pstCallEntity   = AT_PS_GetCallEntity(usClientId, ucCallId);

    /* 在事件处理表中查找处理函数 */
    pRptEndRsltFunc = AT_PS_GetRptEndResultFunc(pstUsrInfo->ucUsrType);

    if (VOS_FALSE == AT_PS_IsNeedSndCallEnded(pstCallEntity, enDataSys))
    {
        AT_NORM_LOG("AT_PS_SndCallEndedResult: not need report DEND!");
        return;
    }

    if (VOS_TRUE == AT_PS_CallEndedReportAlready(pstCallEntity, &enPdpType))
    {
        AT_NORM_LOG("AT_PS_SndCallEndedResult: has already report dend!");
        return;
    }

    AT_PS_ChgCallEndedCause(pstCallEntity, &enCause);

    /* 如果处理函数存在则调用 */
    if (VOS_NULL_PTR != pRptEndRsltFunc)
    {
        pRptEndRsltFunc(pstUsrInfo,
                        enPdpType,
                        enCause);
    }
    else
    {
        AT_ERR_LOG("AT_PS_SndCallEndedResult:ERROR: User type is invalid!");
    }

    return;
}


VOS_VOID AT_PS_ReportCustomPcoInfo(
    TAF_PS_CUSTOM_PCO_INFO_STRU        *pstPcoCustInfo,
    TAF_PS_PDN_OPERATE_TYPE_ENUM_UINT8  enOperateType,
    VOS_UINT8                           ucCid,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    AT_CLIENT_TAB_INDEX_UINT8           enPortIndex
)
{
    VOS_UINT8                          *pstrIpType      = VOS_NULL_PTR;
    VOS_UINT8                           aucStrIpv4[]    = "IPV4";
    VOS_UINT8                           aucStrIpv6[]    = "IPV6";
    VOS_UINT8                           aucStrIpv4v6[]  = "IPV4V6";
    VOS_UINT32                          i = 0;
    VOS_UINT16                          usLength = 0;

    if (TAF_PDP_IPV4 == enPdpType)
    {
        pstrIpType = aucStrIpv4;
    }
    else if (TAF_PDP_IPV6 == enPdpType)
    {
        pstrIpType = aucStrIpv6;
    }
    else if (TAF_PDP_IPV4V6 == enPdpType)
    {
        pstrIpType = aucStrIpv4v6;
    }
    else
    {
        AT_ERR_LOG1("AT_PS_ReportCustomPcoInfo:ERROR: invalid pdp type ", enPdpType);
        return;
    }

    if (pstPcoCustInfo->ulContainerNum > TAF_PS_MAX_CUSTOM_PCO_CONTAINER_NUM)
    {
        AT_ERR_LOG1("AT_PS_ReportCustomPcoInfo:ERROR: ulContainerNum abnormal ", pstPcoCustInfo->ulContainerNum);
        return;
    }

    for ( i = 0; i < pstPcoCustInfo->ulContainerNum; i++ )
    {
        usLength = 0;

        /* 上报形式为:^CUSTPCOINFO:CID,OPERATETYPE,IPTYPE,CONTAINERID,CONTAINER CONTENT */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr, (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s%s %d,%d,\"%s\",%x,\"",
                                           gaucAtCrLf,
                                           gastAtStringTab[AT_STRING_CUST_PCO_INFO].pucText,
                                           ucCid,
                                           enOperateType,
                                           pstrIpType,
                                           pstPcoCustInfo->astContainerList[i].usContainerId);


        usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_UINT8 *)pgucAtSndCodeAddr + usLength,
                                                        pstPcoCustInfo->astContainerList[i].aucContents,
                                                        (VOS_UINT16)pstPcoCustInfo->astContainerList[i].ucContentLen);

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "\"%s",
                                           gaucAtCrLf);

        /* 调用At_SendResultData发送命令结果 */
        At_SendResultData(enPortIndex, pgucAtSndCodeAddr, usLength);
    }

    return;
}


VOS_VOID AT_PS_GenCallDialParam(
    AT_DIAL_PARAM_STRU                 *pstCallDialParam,
    AT_DIAL_PARAM_STRU                 *pstUsrDialParam,
    VOS_UINT8                           ucCid,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
)
{
    /* 获取用户拨号参数 */
    *pstCallDialParam           = *pstUsrDialParam;

    /* 刷新CID和PDP类型 */
    pstCallDialParam->ucCid     = ucCid;
    pstCallDialParam->enPdpType = enPdpType;

    return;
}


VOS_UINT32 AT_PS_AppSetFlowCtrl(
    VOS_UINT32                          ulParam1,
    VOS_UINT32                          ulParam2
)
{
    VOS_UINT32                          ulRslt;

    ulRslt = RNIC_StartFlowCtrl((VOS_UINT8)ulParam1);

    if (VOS_OK != ulRslt)
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_PS_AppClearFlowCtrl(
    VOS_UINT32                          ulParam1,
    VOS_UINT32                          ulParam2
)
{
    VOS_UINT32                          ulRslt;

    ulRslt = RNIC_StopFlowCtrl((VOS_UINT8)ulParam1);

    if (VOS_OK != ulRslt)
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_VOID AT_PS_AppRegFCPoint(
    FC_ID_ENUM_UINT8                    enFcId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent,
    VOS_UINT8                           ucRmNetId
)
{
    FC_REG_POINT_STRU                   stRegFcPoint;
    FC_PRI_ENUM_UINT8                   enDefaultFcPri;
    VOS_UINT32                          ulRslt;
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = MODEM_ID_0;

    TAF_MEM_SET_S(&stRegFcPoint, sizeof(stRegFcPoint), 0x00, sizeof(FC_REG_POINT_STRU));

    ulRslt = AT_GetModemIdFromClient((AT_CLIENT_TAB_INDEX_UINT8)pstEvent->stCtrl.usClientId, &enModemId);

    if (VOS_OK != ulRslt)
    {
        AT_ERR_LOG("AT_PS_AppRegFCPoint: Get modem id fail.");
        return;
    }

    /* 配置通道与RABID映射关系 */
    FC_ChannelMapCreate(enFcId, pstEvent->ucRabId, enModemId);

    /* 根据网卡上最高优先级RAB QoS优先级来折算,优先级改变时，需要改变优先级 */
    /*  FC_PRI_3        有最低优先级的承载
        FC_PRI_4        有NONGBR承载
        FC_PRI_5        有GBR承载 */
    enDefaultFcPri          = FC_PRI_FOR_PDN_LOWEST;
    stRegFcPoint.enFcId     = enFcId;
    stRegFcPoint.enFcPri    = enDefaultFcPri;

    stRegFcPoint.enModemId  = enModemId;
    stRegFcPoint.pClrFunc   = AT_PS_AppClearFlowCtrl;
    stRegFcPoint.pSetFunc   = AT_PS_AppSetFlowCtrl;

    /* Paramter1设置为RmNetId, Paramter2设置为FCID */
    stRegFcPoint.ulParam1   = ucRmNetId;
    stRegFcPoint.ulParam2   = enFcId;
    stRegFcPoint.pRstFunc   = AT_ResetFlowCtl;

    /* 注册流控点, 需要分别注册MEM和CDS */
    stRegFcPoint.enPolicyId = FC_POLICY_ID_MEM;
    ulRslt = FC_RegPoint(&stRegFcPoint);
    if (VOS_OK != ulRslt)
    {
        AT_ERR_LOG("AT_PS_AppRegFCPoint: ERROR: reg mem point Failed.");
        return;
    }


    stRegFcPoint.enPolicyId = FC_POLICY_ID_CDS;
    ulRslt = FC_RegPoint(&stRegFcPoint);
    if (VOS_OK != ulRslt)
    {
        AT_ERR_LOG("AT_PS_AppRegFCPoint: ERROR: reg CDS point Failed.");
        return;
    }

    /* 设置FCID与FC Pri的映射关系 */
    g_stFcIdMaptoFcPri[enFcId].ulUsed       = VOS_TRUE;
    g_stFcIdMaptoFcPri[enFcId].enFcPri      = enDefaultFcPri;
    /* 有一张网卡上多个RABID的情况，所以需要将多个RABID记录下来 */
    g_stFcIdMaptoFcPri[enFcId].ulRabIdMask  |= ((VOS_UINT32)1 << (pstEvent->ucRabId));
    g_stFcIdMaptoFcPri[enFcId].enModemId    = enModemId;

    /* 勾流控消息 */
    AT_MNTN_TraceRegFcPoint((VOS_UINT8)pstEvent->stCtrl.usClientId, AT_FC_POINT_TYPE_RMNET);

    return;
}


VOS_VOID  AT_PS_ProcAppRegFCPoint(
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent,
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
)
{
    VOS_UINT32                          ulResult;
    AT_FCID_MAP_STRU                    stFCPriOrg;
    FC_ID_ENUM_UINT8                    enDefaultFcId;

    /* 获取网卡ID对应的FC ID */
    enDefaultFcId = AT_PS_GetFcIdByIFaceId(pstCallEntity->ucIfaceId);

    ulResult = AT_GetFcPriFromMap(enDefaultFcId ,&stFCPriOrg);
    if (VOS_OK == ulResult)
    {
        /* 如果FC ID未注册，那么注册该流控点。目前只支持一个网卡.*/
        if (VOS_TRUE != stFCPriOrg.ulUsed)
        {
            /* 注册APP拨号使用的流控点(默认使用网卡1) */
            AT_PS_AppRegFCPoint(enDefaultFcId, pstEvent, pstCallEntity->ucRmNetId);
        }
        else
        {
            /* APP拨号只使用最低的流控QOS优先级FC_PRI_FOR_PDN_LOWEST */
            AT_NORM_LOG("AT_PS_ProcAppRegFCPoint: No need to change the default QOS priority.");
        }
    }

    return;
}


VOS_UINT32 AT_PS_EnableNdisFlowCtl(
    VOS_UINT32                          ulParam1,
    VOS_UINT32                          ulParam2
)
{
    /* 通过udi_ioctl函数使能流控 */
    VOS_UINT32  ulEnbflg = NCM_IOCTL_FLOW_CTRL_ENABLE;

    if (0 != mdrv_udi_ioctl (g_ulAtUdiNdisHdl, NCM_IOCTL_FLOW_CTRL_NOTIF, (VOS_VOID*)(&ulEnbflg)))
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_PS_DisableNdisFlowCtl(
    VOS_UINT32                          ulParam1,
    VOS_UINT32                          ulParam2
)
{
    /* 通过udi_ioctl函数去使能流控 */
    VOS_UINT32  ulEnbflg = NCM_IOCTL_FLOW_CTRL_DISABLE;

    if (0 != mdrv_udi_ioctl (g_ulAtUdiNdisHdl, NCM_IOCTL_FLOW_CTRL_NOTIF, (VOS_VOID*)(&ulEnbflg)))
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_PS_RegNdisFCPoint(
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent,
    FC_ID_ENUM_UINT8                    enFcId,
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    FC_REG_POINT_STRU                   stRegFcPoint;
    VOS_UINT32                          ulRet;
    FC_PRI_ENUM_UINT8                   enFCPri;

    TAF_MEM_SET_S(&stRegFcPoint, sizeof(stRegFcPoint), 0x00, sizeof(FC_REG_POINT_STRU));

    /* 配置通道与RABID映射关系 */
    FC_ChannelMapCreate(enFcId, pstEvent->ucRabId, enModemId);

    stRegFcPoint.enFcId             = enFcId;

    /* 根据网卡上最高优先级RAB QoS优先级来折算,优先级改变时，需要改变优先级 */
    /*  FC_PRI_3        有最低优先级的承载
        FC_PRI_4        有NONGBR承载
        FC_PRI_5        有GBR承载 */

    if (TAF_USED == pstEvent->bitOpUmtsQos)
    {
        enFCPri = AT_GetFCPriFromQos(&pstEvent->stUmtsQos);
    }
    else
    {
        enFCPri = FC_PRI_FOR_PDN_NONGBR;
    }
    stRegFcPoint.enFcPri            = enFCPri;
    stRegFcPoint.enPolicyId         = FC_POLICY_ID_MEM;
    stRegFcPoint.enModemId          = enModemId;
    stRegFcPoint.pClrFunc           = AT_PS_DisableNdisFlowCtl;
    stRegFcPoint.pSetFunc           = AT_PS_EnableNdisFlowCtl;
    stRegFcPoint.pRstFunc           = AT_ResetFlowCtl;
    stRegFcPoint.ulParam2           = enFcId;
    stRegFcPoint.ulParam1           = (VOS_UINT32)pstEvent->stCtrl.usClientId;

    /* 注册流控点,需要分别注册MEM,CPU,CDS和GPRS。 */
    ulRet = FC_RegPoint(&stRegFcPoint);
    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_PS_RegNdisFCPoint: ERROR: reg mem point Failed.");
        return VOS_ERR;
    }

    stRegFcPoint.enPolicyId         = FC_POLICY_ID_CPU_A;
    ulRet = FC_RegPoint(&stRegFcPoint);
    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_PS_RegNdisFCPoint: ERROR: reg a cpu point Failed.");
        return VOS_ERR;
    }

    stRegFcPoint.enPolicyId         = FC_POLICY_ID_CDS;
    ulRet = FC_RegPoint(&stRegFcPoint);
    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_PS_RegNdisFCPoint: ERROR: reg cds point Failed.");
        return VOS_ERR;
    }

    stRegFcPoint.enPolicyId         = FC_POLICY_ID_GPRS;
    ulRet = FC_RegPoint(&stRegFcPoint);
    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_PS_RegNdisFCPoint: ERROR: reg gprs point Failed.");
        return VOS_ERR;
    }

    /* 设置FCID与FC Pri的映射关系 */
    g_stFcIdMaptoFcPri[enFcId].ulUsed      = VOS_TRUE;
    g_stFcIdMaptoFcPri[enFcId].enFcPri     = enFCPri;
    /* 有一张网卡上多个RABID的情况，所以需要将多个RABID记录下来 */
    g_stFcIdMaptoFcPri[enFcId].ulRabIdMask |= ((VOS_UINT32)1 << (pstEvent->ucRabId));
    g_stFcIdMaptoFcPri[enFcId].enModemId   = enModemId;

    /* 勾流控消息 */
    AT_MNTN_TraceRegFcPoint((VOS_UINT8)pstEvent->stCtrl.usClientId, AT_FC_POINT_TYPE_NDIS);

    return VOS_OK;
}


VOS_UINT32 AT_PS_DeRegNdisFCPoint(
    FC_ID_ENUM_UINT8                     enFcId,
    MODEM_ID_ENUM_UINT16                 enModemId
)
{
    VOS_UINT32                          ulRet;
    VOS_UINT8                           ucRabId;

    ucRabId = AT_PS_INVALID_RABID;

    if (VOS_ERR == AT_PS_GetRabIdByRabIdMask(g_stFcIdMaptoFcPri[enFcId].ulRabIdMask, &ucRabId))
    {
        AT_ERR_LOG1("AT_PS_DeRegNdisFCPoint: Get RabId err, RabIdMask :", g_stFcIdMaptoFcPri[enFcId].ulRabIdMask);
        return VOS_ERR;
    }

    /* 在调用FC_DeRegPoint前,先调用FC_ChannelMapDelete */
    FC_ChannelMapDelete(ucRabId, enModemId);

    ulRet = FC_DeRegPoint(enFcId, enModemId);
    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_PS_DeRegNdisFCPoint: ERROR: de reg point Failed.");
        return VOS_ERR;
    }

    /* 清除FCID与FC Pri的映射关系 */
    g_stFcIdMaptoFcPri[enFcId].ulUsed      = VOS_FALSE;
    g_stFcIdMaptoFcPri[enFcId].enFcPri     = FC_PRI_BUTT;
    /* 有一张网卡上多个RABID的情况，所以需要将对应的RABID掩码清除掉 */
    g_stFcIdMaptoFcPri[enFcId].ulRabIdMask &= ~((VOS_UINT32)1 << ucRabId);
    g_stFcIdMaptoFcPri[enFcId].enModemId   = MODEM_ID_BUTT;

    /* 勾流控消息 */
    AT_MNTN_TraceDeregFcPoint(AT_CLIENT_TAB_NDIS_INDEX, AT_FC_POINT_TYPE_NDIS);

    return VOS_OK;
}


VOS_VOID  AT_PS_ProcNdisRegFCPoint(
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent,
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
)
{
    VOS_UINT32                          ulRet;
    AT_FCID_MAP_STRU                    stFCPriOrg;
    FC_PRI_ENUM_UINT8                   enFCPriCurrent;
    MODEM_ID_ENUM_UINT16                enModemId;
    FC_ID_ENUM_UINT8                    enDefaultFcId;

    enModemId                           = MODEM_ID_0;

    ulRet = AT_GetModemIdFromClient((AT_CLIENT_TAB_INDEX_UINT8)(pstEvent->stCtrl.usClientId), &enModemId);

    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_PS_ProcNdisRegFCPoint:Get Modem Id fail");
        return;
    }

    /* 获取网卡ID对应的FC ID */
    enDefaultFcId = AT_PS_GetFcIdByIFaceId(pstCallEntity->ucIfaceId);

    ulRet = AT_GetFcPriFromMap(enDefaultFcId ,&stFCPriOrg);
    if (VOS_OK == ulRet)
    {
        /* 如果FC ID未注册，那么注册该流控点。目前只支持一个网卡.*/
        if (VOS_TRUE != stFCPriOrg.ulUsed)
        {
            /* 注册NDIS端口的流控点 */
            AT_PS_RegNdisFCPoint(pstEvent, enDefaultFcId, enModemId);
        }
        else
        {
            AT_NORM_LOG("AT_PS_ProcNdisRegFCPoint: has already reg");

            if (TAF_USED == pstEvent->bitOpUmtsQos)
            {
                enFCPriCurrent = AT_GetFCPriFromQos(&pstEvent->stUmtsQos);
            }
            else
            {
                enFCPriCurrent = FC_PRI_FOR_PDN_NONGBR;
            }

            /* 如果当前FC优先级比之前承载的FC优先级高，那么调整优先级。*/
            if(enFCPriCurrent > stFCPriOrg.enFcPri)
            {
                AT_ChangeFCPoint(&pstEvent->stCtrl,enFCPriCurrent, enDefaultFcId);
            }
        }
    }

    return;
}


VOS_VOID  AT_PS_ProcNdisDeRegFCPoint(
    FC_ID_ENUM_UINT8                     enFcId,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent
)
{
    VOS_UINT32                          ulRet;
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId                           = MODEM_ID_0;

    ulRet = AT_GetModemIdFromClient((AT_CLIENT_TAB_INDEX_UINT8)(pstEvent->stCtrl.usClientId), &enModemId);

    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_PS_ProcNdisDeRegFCPoint:Get Modem Id fail");
        return;
    }

    /* 去注册NDIS端口的流控点 */
    AT_PS_DeRegNdisFCPoint(enFcId, enModemId);

    return;
}


VOS_UINT32 AT_PS_AppDeRegFCPoint(
    FC_ID_ENUM_UINT8                     enFcId,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent
)
{
    VOS_UINT32                          ulRslt;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT8                           ucRabId;

    enModemId = MODEM_ID_0;
    ucRabId   = AT_PS_INVALID_RABID;

    ulRslt = AT_GetModemIdFromClient((AT_CLIENT_TAB_INDEX_UINT8)pstEvent->stCtrl.usClientId, &enModemId);

    if (VOS_OK != ulRslt)
    {
        AT_ERR_LOG("AT_PS_AppDeRegFCPoint: Get modem id fail.");
        return VOS_ERR;
    }

    if (VOS_ERR == AT_PS_GetRabIdByRabIdMask(g_stFcIdMaptoFcPri[enFcId].ulRabIdMask, &ucRabId))
    {
        AT_ERR_LOG1("AT_PS_AppDeRegFCPoint: Get RabId err, RabIdMask :", g_stFcIdMaptoFcPri[enFcId].ulRabIdMask);
        return VOS_ERR;
    }

    /* 在调用FC_DeRegPoint前,先调用FC_ChannelMapDelete */
    FC_ChannelMapDelete(ucRabId, enModemId);

    ulRslt = FC_DeRegPoint(enFcId, enModemId);
    if (VOS_OK != ulRslt)
    {
        AT_ERR_LOG("AT_PS_AppDeRegFCPoint: ERROR: de reg point Failed.");
        return VOS_ERR;
    }

    /* 清除FCID与FC Pri的映射关系 */
    g_stFcIdMaptoFcPri[enFcId].ulUsed       = VOS_FALSE;
    g_stFcIdMaptoFcPri[enFcId].enFcPri      = FC_PRI_BUTT;
    /* 有一张网卡上多个RABID的情况，所以需要将对应的RABID掩码清除掉 */
    g_stFcIdMaptoFcPri[enFcId].ulRabIdMask  &= ~((VOS_UINT32)1 << ucRabId);
    g_stFcIdMaptoFcPri[enFcId].enModemId    = MODEM_ID_BUTT;

    /* 勾流控消息 */
    AT_MNTN_TraceDeregFcPoint((VOS_UINT8)pstEvent->stCtrl.usClientId, AT_FC_POINT_TYPE_RMNET);

    return VOS_OK;
}


VOS_VOID AT_PS_RegAppFCPoint(
    VOS_UINT8                           ucCid,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    VOS_UINT32                          ulRslt;
    AT_FCID_MAP_STRU                    stFCPriOrg;
    FC_ID_ENUM_UINT8                    enDefaultFcId;
    VOS_UINT32                          ulRmNetId;
    VOS_UINT32                          ulIfaceId;
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(pstEvent->stCtrl.usClientId);

    /* 寻找配套的通道ID */
    if ((VOS_TRUE == pstPsModemCtx->astChannelCfg[ucCid].ulUsed)
     && (pstPsModemCtx->astChannelCfg[ucCid].ulRmNetId < RNIC_DEV_ID_BUTT))
    {
        ulRmNetId = pstPsModemCtx->astChannelCfg[ucCid].ulRmNetId;
        ulIfaceId = pstPsModemCtx->astChannelCfg[ucCid].ulIfaceId;
    }
    else
    {
        AT_ERR_LOG("AT_PS_RegAppFCPoint: data channel id is abnormal.\n");
        return;
    }

    /* 上述分支已能保证网卡的有效性 */
    enDefaultFcId = AT_PS_GetFcIdByIFaceId(ulIfaceId);

    ulRslt = AT_GetFcPriFromMap(enDefaultFcId ,&stFCPriOrg);
    if (VOS_OK == ulRslt)
    {
        /* 如果FC ID未注册，那么注册该流控点。*/
        if (VOS_TRUE != stFCPriOrg.ulUsed)
        {
            /* 注册APP拨号使用的流控点 */
            AT_PS_AppRegFCPoint(enDefaultFcId, pstEvent, (VOS_UINT8)ulRmNetId);
        }
        else
        {
            /* APP拨号只使用最低的流控QOS优先级FC_PRI_FOR_PDN_LOWEST */
            AT_NORM_LOG("AT_PS_RegAppFCPoint: No need to change the default QOS priority.");
        }
    }
    return;
}


VOS_VOID AT_PS_ProcAppDeRegFCPoint(
    VOS_UINT8                           ucCid,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent
)
{
    FC_ID_ENUM_UINT8                    enDefaultFcId;
    VOS_UINT32                          ulIfaceId;
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(pstEvent->stCtrl.usClientId);

    /* 寻找配套的通道ID */
    if ((VOS_TRUE == pstPsModemCtx->astChannelCfg[ucCid].ulUsed)
     && (pstPsModemCtx->astChannelCfg[ucCid].ulRmNetId < RNIC_DEV_ID_BUTT))
    {
        ulIfaceId = (VOS_UINT8)pstPsModemCtx->astChannelCfg[ucCid].ulIfaceId;
    }
    else
    {
        AT_ERR_LOG("AT_PS_ProcAppDeRegFCPoint: data channel id is abnormal.\n");
        return;
    }

    /* 上述分支已能保证网卡的有效性 */
    enDefaultFcId = AT_PS_GetFcIdByIFaceId(ulIfaceId);

    /* 去注册APP拨号使用的流控点 */
    AT_PS_AppDeRegFCPoint(enDefaultFcId, pstEvent);

    return;
}


VOS_VOID AT_PS_RegFCPoint(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity   = VOS_NULL_PTR;
    AT_PS_USER_INFO_STRU               *pstUserInfo     = VOS_NULL_PTR;
    VOS_UINT8                          *pucSystemAppConfig;

    pucSystemAppConfig  = AT_GetSystemAppConfigAddr();
    pstCallEntity       = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);
    pstUserInfo         = &(pstCallEntity->stUserInfo);


    /* APP PS CALL处理 */
    if (AT_PS_WAN_TYPE_APP == pstCallEntity->enPsCallType)
    {
        if (SYSTEM_APP_WEBUI == *pucSystemAppConfig)
        {
            /* E5形态 */
            AT_PS_ProcAppRegFCPoint(pstEvent, pstCallEntity);
            return;
        }

        if (AT_APP_USER == pstUserInfo->ucUsrType)
        {
            /* 手机形态 */
            AT_PS_RegAppFCPoint(pstUserInfo->ucUsrCid, pstEvent);
            return;
        }

        AT_ERR_LOG("AT_PS_RegFCPoint:ERROR: User type is invalid!");
        return;
    }

    /* NDIS PS CALL处理 */
    if (AT_PS_WAN_TYPE_NDIS == pstCallEntity->enPsCallType)
    {
        /* STICK形态 */
        AT_PS_ProcNdisRegFCPoint(pstEvent, pstCallEntity);
        return;
    }

    AT_ERR_LOG("AT_PS_RegFCPoint:ERROR: PS CALL TYPE is invalid!");
    return;
}


VOS_VOID AT_PS_DeRegFCPoint(
    VOS_UINT8                               ucCallId,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU    *pstEvent
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity   = VOS_NULL_PTR;
    AT_PS_USER_INFO_STRU               *pstUserInfo     = VOS_NULL_PTR;
    VOS_UINT8                          *pucSystemAppConfig;

    pucSystemAppConfig  = AT_GetSystemAppConfigAddr();
    pstCallEntity       = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);
    pstUserInfo         = &(pstCallEntity->stUserInfo);

    if (AT_PS_WAN_TYPE_APP == pstCallEntity->enPsCallType)
    {
        if (SYSTEM_APP_WEBUI == *pucSystemAppConfig)
        {
            /* 去注册APP拨号使用的流控点(默认使用网卡1) */
            AT_PS_AppDeRegFCPoint(AT_PS_GetFcIdByIFaceId(pstCallEntity->ucIfaceId), pstEvent);
            return;
        }

        if (AT_APP_USER == pstUserInfo->ucUsrType)
        {
            /* 手机形态 */
            AT_PS_ProcAppDeRegFCPoint(pstUserInfo->ucUsrCid, pstEvent);
            return;
        }

        AT_ERR_LOG("AT_PS_DeRegFCPoint:ERROR: User type is invalid!");
        return;
    }

    if (AT_PS_WAN_TYPE_NDIS == pstCallEntity->enPsCallType)
    {
        /* NDIS用户类型的PDP状态改变后的处理流程 */
        AT_PS_ProcNdisDeRegFCPoint(AT_PS_GetFcIdByIFaceId(pstCallEntity->ucIfaceId), pstEvent);
        return;
    }

    AT_ERR_LOG("AT_PS_DeRegFCPoint:ERROR: PS CALL TYPE is invalid!");
    return;
}


VOS_VOID AT_PS_SndRnicIpv4PdpActInd(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    AT_RNIC_PDN_INFO_CFG_IND_STRU      *pstMsg        = VOS_NULL_PTR;
    VOS_UINT32                          ulRslt;
    MODEM_ID_ENUM_UINT16                enModemId;

    /* 通过client获得modemid */
    ulRslt = AT_GetModemIdFromClient(pstEvent->stCtrl.usClientId, &enModemId);
    if (VOS_OK != ulRslt)
    {
        AT_ERR_LOG("AT_PS_SndRnicIpv4PdpActInd: Get modem id fail.");
        return;
    }

    /* 申请AT_RNIC_PDN_INFO_CFG_IND_STRU消息 */
    /*lint -save -e516 */
    pstMsg = (AT_RNIC_PDN_INFO_CFG_IND_STRU *)AT_ALLOC_MSG_WITH_HDR(
                            sizeof(AT_RNIC_PDN_INFO_CFG_IND_STRU));
    /*lint -restore */
    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("AT_PS_SndRnicIpv4PdpActInd: alloc msg fail!");
        return;
    }

    /* 初始化消息 */
    TAF_MEM_SET_S(AT_GET_MSG_ENTITY(pstMsg), AT_GET_MSG_LENGTH(pstMsg),
                  0x00, AT_GET_MSG_LENGTH(pstMsg));

    /* 填写消息头 */
    AT_CFG_RNIC_MSG_HDR(pstMsg, ID_AT_RNIC_PDN_INFO_CFG_IND);

    /* 填写消息内容 */
    pstMsg->bitOpIpv4PdnInfo = VOS_TRUE;
    pstMsg->bitOpIpv6PdnInfo = VOS_FALSE;
    pstMsg->ucRmNetId        = pstCallEntity->ucRmNetId;
    pstMsg->ucIfaceId        = pstCallEntity->ucIfaceId;

    pstMsg->ucRabId          = pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ucRabId;
    pstMsg->enModemId        = enModemId;

    pstMsg->ulIpv4Addr       = pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4Addr;

    (VOS_VOID)PS_SEND_MSG(WUEPS_PID_AT, pstMsg);

    return;
}


VOS_VOID AT_PS_SndRnicIpv6PdpActInd(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    AT_RNIC_PDN_INFO_CFG_IND_STRU      *pstMsg        = VOS_NULL_PTR;
    VOS_UINT32                          ulRslt;
    MODEM_ID_ENUM_UINT16                enModemId;

    /* 通过client获得modemid */
    ulRslt = AT_GetModemIdFromClient(pstEvent->stCtrl.usClientId, &enModemId);
    if (VOS_OK != ulRslt)
    {
        AT_ERR_LOG("AT_PS_SndRnicIpv6PdpActInd: Get modem id fail.");
        return;
    }

    /* 申请AT_RNIC_PDN_INFO_CFG_IND_STRU消息 */
    /*lint -save -e516 */
    pstMsg = (AT_RNIC_PDN_INFO_CFG_IND_STRU *)AT_ALLOC_MSG_WITH_HDR(
                            sizeof(AT_RNIC_PDN_INFO_CFG_IND_STRU));
    /*lint -restore */
    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("AT_PS_SndRnicIpv6PdpActInd: alloc msg fail!");
        return;
    }

    /* 初始化消息 */
    TAF_MEM_SET_S(AT_GET_MSG_ENTITY(pstMsg), AT_GET_MSG_LENGTH(pstMsg),
                  0x00, AT_GET_MSG_LENGTH(pstMsg));

    /* 填写消息头 */
    AT_CFG_RNIC_MSG_HDR(pstMsg, ID_AT_RNIC_PDN_INFO_CFG_IND);

    /* 填写消息内容 */
    pstMsg->bitOpIpv4PdnInfo = VOS_FALSE;
    pstMsg->bitOpIpv6PdnInfo = VOS_TRUE;
    pstMsg->ucRmNetId        = pstCallEntity->ucRmNetId;
    pstMsg->ucIfaceId        = pstCallEntity->ucIfaceId;

    pstMsg->ucRabId          = pstCallEntity->stIpv6Info.stIpv6DhcpInfo.ucRabId;
    pstMsg->enModemId        = enModemId;

    TAF_MEM_CPY_S(pstMsg->aucIpv6Addr,
               sizeof(pstMsg->aucIpv6Addr),
               pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6Addr,
               TAF_IPV6_ADDR_LEN);

    (VOS_VOID)PS_SEND_MSG(WUEPS_PID_AT, pstMsg);

    return;
}


VOS_VOID AT_PS_SndRnicPdpActInd(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
)
{
    /* 向DIPC上报PDP IPv4已经激活事件 */
    if (TAF_PDP_IPV4 == (enPdpType & TAF_PDP_IPV4))
    {
        AT_PS_SndRnicIpv4PdpActInd(pstCallEntity, pstEvent);
    }

    /* 向DIPC上报PDP IPv6已经激活事件 */
    if (TAF_PDP_IPV6 == (enPdpType & TAF_PDP_IPV6))
    {
        AT_PS_SndRnicIpv6PdpActInd(pstCallEntity, pstEvent);
    }

    return;
}


VOS_VOID AT_PS_SndRnicPdpDeactInd(
    AT_PS_CALL_ENTITY_STRU              *pstCallEntity,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent,
    TAF_PDP_TYPE_ENUM_UINT8              enPdpType
)
{
    AT_RNIC_PDN_INFO_REL_IND_STRU      *pstMsg;

    /* 申请AT_RNIC_PDN_INFO_REL_IND_STRU消息 */
    /*lint -save -e516 */
    pstMsg = (AT_RNIC_PDN_INFO_REL_IND_STRU *)AT_ALLOC_MSG_WITH_HDR(
                            sizeof(AT_RNIC_PDN_INFO_REL_IND_STRU));
    /*lint -restore */
    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("AT_NotifyRnicPdpDeactInd: alloc msg fail!");
        return;
    }

    /* 初始化消息 */
    TAF_MEM_SET_S(AT_GET_MSG_ENTITY(pstMsg), AT_GET_MSG_LENGTH(pstMsg),
                  0x00, AT_GET_MSG_LENGTH(pstMsg));

    /* 填写消息头 */
    AT_CFG_RNIC_MSG_HDR(pstMsg, ID_AT_RNIC_PDN_INFO_REL_IND);

    /* 填写消息内容 */
    pstMsg->bitOpIpv4PdnInfo = ((TAF_PDP_IPV4 == (enPdpType & TAF_PDP_IPV4)) ? VOS_TRUE : VOS_FALSE);
    pstMsg->bitOpIpv6PdnInfo = ((TAF_PDP_IPV6 == (enPdpType & TAF_PDP_IPV6)) ? VOS_TRUE : VOS_FALSE);
    pstMsg->ucRmNetId        = pstCallEntity->ucRmNetId;
    pstMsg->ucIfaceId        = pstCallEntity->ucIfaceId;

    (VOS_VOID)PS_SEND_MSG(WUEPS_PID_AT, pstMsg);

    return;
}


VOS_VOID AT_PS_ActiveUsbNet(VOS_VOID)
{
    VOS_UINT32                          ulLinkstus;
    VOS_INT32                           lRtn;
    NCM_IOCTL_CONNECTION_SPEED_S        stNcmConnectSpeed;
    AT_DISPLAY_RATE_STRU                stSpeed;

    TAF_MEM_SET_S(&stSpeed, sizeof(stSpeed), 0x00, (VOS_SIZE_T)(sizeof(AT_DISPLAY_RATE_STRU)));

    if (VOS_OK != AT_GetDisplayRate(AT_CLIENT_ID_NDIS, &stSpeed))
    {
        AT_ERR_LOG("AT_PS_ActiveUsbNet : ERROR : AT_GetDisplayRate Error!");
    }
    /* 如果速率超出U32的范围，取最大值0xffffffff */
    stNcmConnectSpeed.u32DownBitRate   = (AT_AtoI(stSpeed.ucDlSpeed) >= 0xffffffff) ? 0xffffffff : (VOS_UINT32)AT_AtoI(stSpeed.ucDlSpeed);
    stNcmConnectSpeed.u32UpBitRate     = (AT_AtoI(stSpeed.ucUlSpeed) >= 0xffffffff) ? 0xffffffff : (VOS_UINT32)AT_AtoI(stSpeed.ucUlSpeed);

    lRtn        = mdrv_udi_ioctl(g_ulAtUdiNdisHdl, NCM_IOCTL_CONNECTION_SPEED_CHANGE_NOTIF, (VOS_VOID *)(&stNcmConnectSpeed));
    if (0 != lRtn)
    {
        AT_ERR_LOG("AT_PS_ActiveUsbNet, Ctrl Speed Fail!" );
        return;
    }

    ulLinkstus  = NCM_IOCTL_CONNECTION_LINKUP;
    lRtn        = mdrv_udi_ioctl (g_ulAtUdiNdisHdl, NCM_IOCTL_NETWORK_CONNECTION_NOTIF, &ulLinkstus);
    if (0 != lRtn)
    {
        AT_ERR_LOG("AT_PS_ActiveUsbNet, Active usb net Fail!" );
        return;
    }

    return;
}


VOS_VOID AT_PS_DeActiveUsbNet(VOS_VOID)
{
    VOS_UINT32  ulLinkstus;
    VOS_INT32   lRtn;

    /*去激活，已和BSP确认，如果本来是去激活，再去激活并没有影响*/
    ulLinkstus = NCM_IOCTL_CONNECTION_LINKDOWN;

    lRtn  = mdrv_udi_ioctl (g_ulAtUdiNdisHdl, NCM_IOCTL_NETWORK_CONNECTION_NOTIF, (VOS_VOID*)(&ulLinkstus));
    if(0 != lRtn)
    {
        AT_ERR_LOG("AT_PS_DeActiveUsbNet, Deactive usb net Fail!" );
        return;
    }

    return;
}


VOS_VOID AT_PS_SendNdisIPv6PdnInfoCfgReq(
    MODEM_ID_ENUM_UINT16                 enModemId,
    AT_PS_CALL_ENTITY_STRU              *pstCallEntity,
    TAF_PS_IPV6_INFO_IND_STRU           *pIPv6RaNotify
)
{
    AT_COMM_PS_CTX_STRU                *pstPsCntxt      = VOS_NULL_PTR;
    AT_NDIS_PDNINFO_CFG_REQ_STRU       *pstNdisCfgReq   = VOS_NULL_PTR;

    /* 初始化 */
    pstPsCntxt          = AT_GetCommPsCtxAddr();

    /* 申请AT_NDIS_PDNINFO_CFG_REQ_STRU消息 */
    /*lint -save -e516 */
    pstNdisCfgReq = (AT_NDIS_PDNINFO_CFG_REQ_STRU *)AT_ALLOC_MSG_WITH_HDR(
                            sizeof(AT_NDIS_PDNINFO_CFG_REQ_STRU));
    /*lint -restore */
    if (VOS_NULL_PTR == pstNdisCfgReq)
    {
        AT_WARN_LOG("AT_PS_SendNdisIPv6PdnInfoCfgReq: alloc msg fail!");
        return;
    }

    /* 初始化消息 */
    TAF_MEM_SET_S(AT_GET_MSG_ENTITY(pstNdisCfgReq), AT_GET_MSG_LENGTH(pstNdisCfgReq),
                  0x00, AT_GET_MSG_LENGTH(pstNdisCfgReq));

    /* 填写消息头 */
    AT_CFG_NDIS_MSG_HDR(pstNdisCfgReq, ID_AT_NDIS_PDNINFO_CFG_REQ);

    /* 填写消息体 */
    pstNdisCfgReq->ulHandle               = g_ulAtUdiNdisHdl;
    pstNdisCfgReq->bitOpIpv6PdnInfo       = VOS_TRUE;
    pstNdisCfgReq->enModemId              = enModemId;
    pstNdisCfgReq->ucRabId                = pIPv6RaNotify->ucRabId;

    /* 填充主副DNS */
    pstNdisCfgReq->stIpv6PdnInfo.stDnsSer.ucSerNum    = 0;
    if (VOS_TRUE == pstCallEntity->stIpv6Info.stIpv6DhcpInfo.bitOpIpv6PriDns)
    {
        TAF_MEM_CPY_S((VOS_VOID *)pstNdisCfgReq->stIpv6PdnInfo.stDnsSer.aucPriServer,
                    sizeof(pstNdisCfgReq->stIpv6PdnInfo.stDnsSer.aucPriServer),
                    pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6PrimDNS,
                    TAF_IPV6_ADDR_LEN);
        pstNdisCfgReq->stIpv6PdnInfo.stDnsSer.ucSerNum += 1;
    }

    if (VOS_TRUE == pstCallEntity->stIpv6Info.stIpv6DhcpInfo.bitOpIpv6SecDns)
    {
        TAF_MEM_CPY_S((VOS_VOID *)pstNdisCfgReq->stIpv6PdnInfo.stDnsSer.aucSecServer,
                    sizeof(pstNdisCfgReq->stIpv6PdnInfo.stDnsSer.aucSecServer),
                    pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6SecDNS,
                    TAF_IPV6_ADDR_LEN);
        pstNdisCfgReq->stIpv6PdnInfo.stDnsSer.ucSerNum += 1;
    }

    /* 填充MTU */
    if (VOS_TRUE == pIPv6RaNotify->stIpv6RaInfo.bitOpMtu)
    {
        pstNdisCfgReq->stIpv6PdnInfo.ulBitOpMtu   = VOS_TRUE;
        pstNdisCfgReq->stIpv6PdnInfo.ulMtu        = pIPv6RaNotify->stIpv6RaInfo.ulMtu;
    }

    pstNdisCfgReq->stIpv6PdnInfo.ulBitCurHopLimit = pIPv6RaNotify->stIpv6RaInfo.ulBitCurHopLimit;
    pstNdisCfgReq->stIpv6PdnInfo.ulBitM           = pIPv6RaNotify->stIpv6RaInfo.ulBitM;
    pstNdisCfgReq->stIpv6PdnInfo.ulBitO           = pIPv6RaNotify->stIpv6RaInfo.ulBitO;
    pstNdisCfgReq->stIpv6PdnInfo.ulPrefixNum      = pIPv6RaNotify->stIpv6RaInfo.ulPrefixNum;
    TAF_MEM_CPY_S((VOS_VOID *)pstNdisCfgReq->stIpv6PdnInfo.astPrefixList,
                sizeof(pstNdisCfgReq->stIpv6PdnInfo.astPrefixList),
                (VOS_VOID *)pIPv6RaNotify->stIpv6RaInfo.astPrefixList,
                sizeof(TAF_PDP_IPV6_PREFIX_STRU)*TAF_MAX_PREFIX_NUM_IN_RA);

    /* 填写INTERFACE，取IPV6地址的后8字节来填写INTERFACE */
    TAF_MEM_CPY_S((VOS_VOID*)pstNdisCfgReq->stIpv6PdnInfo.aucInterfaceId,
                sizeof(pstNdisCfgReq->stIpv6PdnInfo.aucInterfaceId),
               (VOS_VOID*)pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6Addr,
                sizeof(VOS_UINT8)*AT_NDIS_IPV6_IFID_LENGTH);

    /* 填充主副PCSCF地址  */
    pstNdisCfgReq->stIpv6PdnInfo.stPcscfSer.ucSerNum      = 0;
    if (pstCallEntity->stIpv6Info.stIpv6DhcpInfo.stIpv6PcscfList.ucIpv6PcscfAddrNum > 0)
    {
        pstNdisCfgReq->stIpv6PdnInfo.stPcscfSer.ucSerNum++;

        TAF_MEM_CPY_S((VOS_VOID *)pstNdisCfgReq->stIpv6PdnInfo.stPcscfSer.aucPriServer,
                      sizeof(pstNdisCfgReq->stIpv6PdnInfo.stPcscfSer.aucPriServer),
                      pstCallEntity->stIpv6Info.stIpv6DhcpInfo.stIpv6PcscfList.astIpv6PcscfAddrList[0].aucPcscfAddr,
                      sizeof(pstCallEntity->stIpv6Info.stIpv6DhcpInfo.stIpv6PcscfList.astIpv6PcscfAddrList[0].aucPcscfAddr));
    }

    if (pstCallEntity->stIpv6Info.stIpv6DhcpInfo.stIpv6PcscfList.ucIpv6PcscfAddrNum > 1)
    {
        pstNdisCfgReq->stIpv6PdnInfo.stPcscfSer.ucSerNum++;

        TAF_MEM_CPY_S((VOS_VOID *)pstNdisCfgReq->stIpv6PdnInfo.stPcscfSer.aucSecServer,
                      sizeof(pstNdisCfgReq->stIpv6PdnInfo.stPcscfSer.aucSecServer),
                      pstCallEntity->stIpv6Info.stIpv6DhcpInfo.stIpv6PcscfList.astIpv6PcscfAddrList[1].aucPcscfAddr,
                      sizeof(pstCallEntity->stIpv6Info.stIpv6DhcpInfo.stIpv6PcscfList.astIpv6PcscfAddrList[1].aucPcscfAddr));
    }

    pstNdisCfgReq->lSpePort   = pstPsCntxt->lSpePort;
    pstNdisCfgReq->ulIpfFlag  = pstPsCntxt->ulIpfPortFlg;

    /* 发送消息 */
    (VOS_VOID)PS_SEND_MSG(WUEPS_PID_AT, pstNdisCfgReq);

    return;
}


VOS_VOID AT_PS_SendNdisIPv4PdnInfoCfgReq(
    AT_CLIENT_TAB_INDEX_UINT8           usClientId,
    AT_IPV4_DHCP_PARAM_STRU            *pstIPv4DhcpParam
)
{
    AT_COMM_PS_CTX_STRU                *pstPsCntxt      = VOS_NULL_PTR;
    AT_NDIS_PDNINFO_CFG_REQ_STRU       *pstNdisCfgReq   = VOS_NULL_PTR;
    VOS_UINT32                          ulRelt;
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId   = MODEM_ID_0;
    pstPsCntxt  = AT_GetCommPsCtxAddr();
    ulRelt      = AT_GetModemIdFromClient(usClientId, &enModemId);

    if (VOS_OK != ulRelt)
    {
        AT_ERR_LOG("AT_PS_SendNdisIPv4PdnInfoCfgReq:Get Modem Id fail");
        return;
    }

    /* 申请AT_NDIS_PDNINFO_CFG_REQ_STRU消息 */
    /*lint -save -e516 */
    pstNdisCfgReq = (AT_NDIS_PDNINFO_CFG_REQ_STRU *)AT_ALLOC_MSG_WITH_HDR(
                            sizeof(AT_NDIS_PDNINFO_CFG_REQ_STRU));
    /*lint -restore */
    if (VOS_NULL_PTR == pstNdisCfgReq)
    {
        AT_WARN_LOG("AT_PS_SendNdisIPv4PdnInfoCfgReq: alloc msg fail!");
        return;
    }

    /* 初始化消息 */
    TAF_MEM_SET_S(AT_GET_MSG_ENTITY(pstNdisCfgReq), AT_GET_MSG_LENGTH(pstNdisCfgReq),
                  0x00, AT_GET_MSG_LENGTH(pstNdisCfgReq));

    /* 填写消息头 */
    AT_CFG_NDIS_MSG_HDR(pstNdisCfgReq, ID_AT_NDIS_PDNINFO_CFG_REQ);

    /* 填写消息体 */
    pstNdisCfgReq->bitOpIpv4PdnInfo       = VOS_TRUE;
    pstNdisCfgReq->enModemId              = enModemId;
    pstNdisCfgReq->ulHandle               = g_ulAtUdiNdisHdl;

    /* 构造消息 */
    if (0 != pstIPv4DhcpParam->ucRabId)
    {
        pstNdisCfgReq->ucRabId = pstIPv4DhcpParam->ucRabId;
    }

    /* 填写IPv4地址 */
    if (0 != pstIPv4DhcpParam->ulIpv4Addr)
    {
        pstNdisCfgReq->stIpv4PdnInfo.bitOpPdnAddr     = VOS_TRUE;
        AT_PutNetworkAddr32(pstNdisCfgReq->stIpv4PdnInfo.stPDNAddrInfo.aucIpV4Addr,
                            pstIPv4DhcpParam->ulIpv4Addr);
    }

    /* 填写掩码地址 */
    if (0 != pstIPv4DhcpParam->ulIpv4NetMask)
    {
        AT_PutNetworkAddr32(pstNdisCfgReq->stIpv4PdnInfo.stSubnetMask.aucIpV4Addr,
                            pstIPv4DhcpParam->ulIpv4NetMask);
    }

    /* 填写网关地址 */
    if (0 != pstIPv4DhcpParam->ulIpv4GateWay)
    {
        AT_PutNetworkAddr32(pstNdisCfgReq->stIpv4PdnInfo.stGateWayAddrInfo.aucIpV4Addr,
                            pstIPv4DhcpParam->ulIpv4GateWay);
    }

    /* 填写主DNS地址 */
    if (0 != pstIPv4DhcpParam->ulIpv4PrimDNS)
    {
        pstNdisCfgReq->stIpv4PdnInfo.bitOpDnsPrim     = VOS_TRUE;
        AT_PutNetworkAddr32(pstNdisCfgReq->stIpv4PdnInfo.stDnsPrimAddrInfo.aucIpV4Addr,
                            pstIPv4DhcpParam->ulIpv4PrimDNS);

    }

    /* 填写辅DNS地址 */
    if (0 != pstIPv4DhcpParam->ulIpv4SecDNS)
    {
        pstNdisCfgReq->stIpv4PdnInfo.bitOpDnsSec      = VOS_TRUE;
        AT_PutNetworkAddr32(pstNdisCfgReq->stIpv4PdnInfo.stDnsSecAddrInfo.aucIpV4Addr,
                          pstIPv4DhcpParam->ulIpv4SecDNS);

    }

    /* 填写主WINS地址 */
    if (0 != pstIPv4DhcpParam->ulIpv4PrimWINNS)
    {
        pstNdisCfgReq->stIpv4PdnInfo.bitOpWinsPrim    = VOS_TRUE;
        AT_PutNetworkAddr32(pstNdisCfgReq->stIpv4PdnInfo.stWinsPrimAddrInfo.aucIpV4Addr,
                            pstIPv4DhcpParam->ulIpv4PrimWINNS);
    }

    /* 填写辅WINS地址 */
    if (0 != pstIPv4DhcpParam->ulIpv4SecWINNS)
    {
        pstNdisCfgReq->stIpv4PdnInfo.bitOpWinsSec     = VOS_TRUE;
        AT_PutNetworkAddr32(pstNdisCfgReq->stIpv4PdnInfo.stWinsSecAddrInfo.aucIpV4Addr,
                            pstIPv4DhcpParam->ulIpv4SecWINNS);
    }

    if (pstIPv4DhcpParam->stIpv4PcscfList.ucIpv4PcscfAddrNum > 0)
    {
        AT_PutNetworkAddr32(pstNdisCfgReq->stIpv4PdnInfo.stPcscfPrimAddrInfo.aucIpV4Addr,
                            pstIPv4DhcpParam->stIpv4PcscfList.aulIpv4PcscfAddrList[0]);

        pstNdisCfgReq->stIpv4PdnInfo.bitOpPcscfPrim = VOS_TRUE;
    }

    if (pstIPv4DhcpParam->stIpv4PcscfList.ucIpv4PcscfAddrNum > 1)
    {
        AT_PutNetworkAddr32(pstNdisCfgReq->stIpv4PdnInfo.stPcscfSecAddrInfo.aucIpV4Addr,
                            pstIPv4DhcpParam->stIpv4PcscfList.aulIpv4PcscfAddrList[1]);

        pstNdisCfgReq->stIpv4PdnInfo.bitOpPcscfSec    = VOS_TRUE;
    }

    pstNdisCfgReq->lSpePort   = pstPsCntxt->lSpePort;
    pstNdisCfgReq->ulIpfFlag  = pstPsCntxt->ulIpfPortFlg;

    /* 发送消息 */
    (VOS_VOID)PS_SEND_MSG(WUEPS_PID_AT, pstNdisCfgReq);

    return;
}


VOS_VOID AT_PS_NdisConfigIpv6Dns(AT_PS_CALL_ENTITY_STRU *pstCallEntity)
{
    NCM_IPV6DNS_S                       stIPv6Dns;
    VOS_INT32                           lRslt;

    /*lint -save -e516 */
    stIPv6Dns.pu8Ipv6DnsInfo = (unsigned char*)PS_MEM_ALLOC(
                                    WUEPS_PID_AT,
                                    BSP_NCM_IPV6_DNS_LEN);
    /*lint -restore */
    if (VOS_NULL_PTR == stIPv6Dns.pu8Ipv6DnsInfo)
    {
        AT_ERR_LOG("AT_PS_NdisConfigIpv6Dns:Invalid stIPv6Dns.pu8Ipv6DnsInfo");
        return;
    }

    TAF_MEM_SET_S(stIPv6Dns.pu8Ipv6DnsInfo, BSP_NCM_IPV6_DNS_LEN, 0x00, BSP_NCM_IPV6_DNS_LEN);

    /* 上报给底软的DNS长度固定为32(Primary DNS LEN + Secondary DNS LEN) */
    stIPv6Dns.u32Length = BSP_NCM_IPV6_DNS_LEN;

    /*如果有DNS，需要调用DRV的接口上报DNS给PC*/
    if (VOS_TRUE == pstCallEntity->stIpv6Info.stIpv6DhcpInfo.bitOpIpv6PriDns)
    {
        TAF_MEM_CPY_S(stIPv6Dns.pu8Ipv6DnsInfo,
                   BSP_NCM_IPV6_DNS_LEN,
                   pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6PrimDNS,
                   AT_MAX_IPV6_DNS_LEN);
    }

    if (VOS_TRUE == pstCallEntity->stIpv6Info.stIpv6DhcpInfo.bitOpIpv6SecDns)
    {
        TAF_MEM_CPY_S(stIPv6Dns.pu8Ipv6DnsInfo + AT_MAX_IPV6_DNS_LEN,
                   BSP_NCM_IPV6_DNS_LEN - AT_MAX_IPV6_DNS_LEN,
                   pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6SecDNS,
                   AT_MAX_IPV6_DNS_LEN);
    }

    /* 设置低软主副DNS信息 */
    lRslt = mdrv_udi_ioctl(g_ulAtUdiNdisHdl, NCM_IOCTL_SET_IPV6_DNS, &stIPv6Dns);
    if (0 != lRslt)
    {
        AT_ERR_LOG("AT_PS_NdisConfigIpv6Dns, DRV_UDI_IOCTL Fail!" );
    }

    /* 释放申请的内存 */
    /*lint -save -e516 */
    PS_MEM_FREE(WUEPS_PID_AT, stIPv6Dns.pu8Ipv6DnsInfo);
    /*lint -restore */
}


VOS_VOID AT_PS_SendNdisRelReq(
    VOS_UINT16                                  usClientId,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU        *pstEvent
)
{
    AT_NDIS_PDNINFO_REL_REQ_STRU       *pstNdisRelReq   = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRet;

    enModemId                           = MODEM_ID_0;

    ulRet = AT_GetModemIdFromClient(usClientId, &enModemId);

    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_PS_SendNdisRelReq:Get Modem Id fail");
        return;
    }

    /* 申请AT_NDIS_PDNINFO_REL_REQ_STRU消息 */
    /*lint -save -e516 */
    pstNdisRelReq = (AT_NDIS_PDNINFO_REL_REQ_STRU *)AT_ALLOC_MSG_WITH_HDR(
                            sizeof(AT_NDIS_PDNINFO_REL_REQ_STRU));
    /*lint -restore */
    if (VOS_NULL_PTR == pstNdisRelReq)
    {
        AT_WARN_LOG("AT_PS_SendNdisRelReq: alloc msg fail!");
        return;
    }

    /* 初始化消息 */
    TAF_MEM_SET_S(AT_GET_MSG_ENTITY(pstNdisRelReq), AT_GET_MSG_LENGTH(pstNdisRelReq),
                  0x00, AT_GET_MSG_LENGTH(pstNdisRelReq));

    /* 填写消息头 */
    AT_CFG_NDIS_MSG_HDR(pstNdisRelReq, ID_AT_NDIS_PDNINFO_REL_REQ);

    /* 填写消息体 */
    pstNdisRelReq->enModemId = enModemId;
    pstNdisRelReq->ucRabId   = pstEvent->ucRabId;

    /* 发送消息 */
    (VOS_VOID)PS_SEND_MSG(WUEPS_PID_AT, pstNdisRelReq);

    return;
}



VOS_VOID AT_PS_ProcNdisPdpActInd(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
)
{
    /*激活网卡*/
    AT_PS_ActiveUsbNet();

    /* 向DIPC上报PDP IPv4已经激活事件 */
    if (TAF_PDP_IPV4 == (enPdpType & TAF_PDP_IPV4))
    {
        /* 把IPV4的PDN信息发送给NDIS模块 */
        AT_PS_SendNdisIPv4PdnInfoCfgReq(pstCallEntity->stUserInfo.enPortIndex,
                                        &(pstCallEntity->stIpv4Info.stIpv4DhcpInfo));
    }

    /* 向DIPC上报PDP IPv6已经激活事件 */
    if (TAF_PDP_IPV6 == (enPdpType & TAF_PDP_IPV6))
    {
        /* 通过底软上报IPv6 DNS */
        AT_PS_NdisConfigIpv6Dns(pstCallEntity);
    }

    return;
}


VOS_VOID AT_PS_ProcNdisPdpDeActInd(
    AT_PS_CALL_ENTITY_STRU              *pstCallEntity,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent,
    TAF_PDP_TYPE_ENUM_UINT8              enPdpType
)
{
    AT_PS_SendNdisRelReq(pstCallEntity->stUserInfo.enPortIndex, pstEvent);

    /* 通知网卡去激活 */
    AT_PS_DeActiveUsbNet();

    return;
}


VOS_VOID AT_PS_ActivateIFace(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity             = VOS_NULL_PTR;
    CONST AT_PS_SND_PDP_ACT_IND_STRU   *pstSndPdpActIndFuncTblPtr = VOS_NULL_PTR;
    AT_PS_SND_PDP_ACT_IND_FUNC          pSndPdpActIndFunc         = VOS_NULL_PTR;
    VOS_UINT32                          ulCnt;

    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);
    pstSndPdpActIndFuncTblPtr = AT_PS_GET_SND_PDP_ACT_IND_FUNC_TBL_PTR();

    /* 在事件处理表中查找处理函数 */
    for (ulCnt = 0; ulCnt < AT_PS_GET_SND_PDP_ACT_IND_FUNC_TBL_SIZE(); ulCnt++)
    {
        if (pstCallEntity->stUserInfo.ucUsrType == pstSndPdpActIndFuncTblPtr[ulCnt].ucUsrType)
        {
            /* 类型匹配 */
            pSndPdpActIndFunc = pstSndPdpActIndFuncTblPtr[ulCnt].pSndPdpActInd;
            break;
        }
    }

    /* 如果处理函数存在则调用 */
    if (VOS_NULL_PTR != pSndPdpActIndFunc)
    {
        pSndPdpActIndFunc(pstCallEntity, pstEvent, enPdpType);
    }
    else
    {
        AT_ERR_LOG("AT_PS_ActivateIFace:ERROR: User type is invalid!");
    }

    return;
}


VOS_VOID AT_PS_DeactivateIFace(
    VOS_UINT8                               ucCallId,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU    *pstEvent,
    TAF_PDP_TYPE_ENUM_UINT8                 enPdpType
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity               = VOS_NULL_PTR;
    CONST AT_PS_SND_PDP_DEACT_IND_STRU *pstSndPdpDeActIndFuncTblPtr = VOS_NULL_PTR;
    AT_PS_SND_PDP_DEACT_IND_FUNC        pSndPdpDeActIndFunc         = VOS_NULL_PTR;
    VOS_UINT32                          ulCnt;

    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);
    pstSndPdpDeActIndFuncTblPtr = AT_PS_GET_SND_PDP_DEACT_IND_FUNC_TBL_PTR();

    /* 在事件处理表中查找处理函数 */
    for (ulCnt = 0; ulCnt < AT_PS_GET_SND_PDP_DEACT_IND_FUNC_TBL_SIZE(); ulCnt++)
    {
        if (pstCallEntity->stUserInfo.ucUsrType == pstSndPdpDeActIndFuncTblPtr[ulCnt].ucUsrType)
        {
            /* 类型匹配 */
            pSndPdpDeActIndFunc = pstSndPdpDeActIndFuncTblPtr[ulCnt].pSndPdpDeActInd;
            break;
        }
    }

    /* 如果处理函数存在则调用 */
    if (VOS_NULL_PTR != pSndPdpDeActIndFunc)
    {
        pSndPdpDeActIndFunc(pstCallEntity, pstEvent, enPdpType);
    }
    else
    {
        AT_ERR_LOG("AT_PS_DeactivateIFace:ERROR: User type is invalid!");
    }

    return;
}


VOS_UINT32 AT_PS_IsHoEndCellularConnect(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    TAF_PDP_TYPE_ENUM_UINT8             enConnectPdpType
)
{
    /* 单IPV4情况判断HANDOVER是否结束 */
    if ( (TAF_PDP_IPV4 == pstCallEntity->stDialPdpType.enHoPdpType)
      && (TAF_PDP_IPV4 == enConnectPdpType))
    {
        AT_NORM_LOG("AT_PS_IsHoEndCellularConnect: IPV4 is handover end!");
        return VOS_TRUE;
    }

    /* 单IPV6情况判断HANDOVER是否结束 */
    if ( (TAF_PDP_IPV6 == pstCallEntity->stDialPdpType.enHoPdpType)
      && (TAF_PDP_IPV6 == enConnectPdpType))
    {
        AT_NORM_LOG("AT_PS_IsHoEndCellularConnect: IPV6 is handover end!");
        return VOS_TRUE;
    }

    /* 双栈情况判断HANDOVER是否结束 */
    if (TAF_PDP_IPV4V6 == pstCallEntity->stDialPdpType.enHoPdpType)
    {
        if ( (TAF_PDP_IPV4 == enConnectPdpType)
          && (AT_PDP_STATE_ACTED == pstCallEntity->stIpv6Info.enIpv6State)
          && (VOS_TRUE == pstCallEntity->stIpv6Info.stIpv6RaInfo.bitOpPrefixAddr))
        {
            AT_NORM_LOG("AT_PS_IsHoEndCellularConnect: IPV4V6 is handover end 1!");
            return VOS_TRUE;
        }

        if ( (TAF_PDP_IPV6 == enConnectPdpType)
          && (AT_PDP_STATE_ACTED == pstCallEntity->stIpv4Info.enIpv4State))
        {
            AT_NORM_LOG("AT_PS_IsHoEndCellularConnect: IPV4V6 is handover end 2!");
            return VOS_TRUE;
        }
    }

    AT_NORM_LOG("AT_PS_IsHoEndCellularConnect: handover is not end!");
    return VOS_FALSE;
}


VOS_VOID AT_PS_ProcHoEndCellularConnect(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    VOS_UINT8                           ucCallId
)
{
    TAF_PS_APN_DATA_SYS_POLICY_INFO_STRU   *pstApnDataSysPolicyInfo = VOS_NULL_PTR;
    VOS_UINT32                              ulTmrName;
    VOS_UINT32                              ulTmrParam;

    /* 通过域选索引得到域选配置 */
    pstApnDataSysPolicyInfo = AT_PS_GetApnDataSysPolicyInfo(pstCallEntity->stUserInfo.enPortIndex,
                                                            pstCallEntity->stApnDataSysInfo.ucDataSysPolicyIndex);

    AT_PS_SetCallHandOverFlg(pstCallEntity->stUserInfo.enPortIndex, ucCallId, VOS_FALSE);

    /* 设置handover呼叫类型 */
    AT_PS_SetHoCallType(pstCallEntity->stUserInfo.enPortIndex, ucCallId, TAF_PDP_TYPE_BUTT);

    /* 在WLAN下发起本地去激活 */
    (VOS_VOID)AT_PS_HangupWlanCall(pstCallEntity->stUserInfo.enUserIndex, ucCallId, VOS_TRUE, VOS_TRUE);

    /* 启动防乒乓定时器 */
    AT_SET_PROTECT_PDN_IN_DATA_SYS_TMR_NAME(ulTmrName);
    AT_SET_PROTECT_PDN_IN_DATA_SYS_TMR_PARAM(ulTmrParam, pstCallEntity->stUserInfo.enPortIndex, ucCallId);

    AT_StartRelTimer(&(pstCallEntity->stPsCallTimerInfo.hProtectInDataSysTmrHdl),
                     pstApnDataSysPolicyInfo->stDataSysPolicy.usCelluarOutProtectTimeLen * 1000,
                     ulTmrName,
                     ulTmrParam,
                     VOS_RELTIMER_NOLOOP);


    return;
}



VOS_UINT32 AT_PS_CmpIpv6PrefixAddr(
    VOS_UINT8                          *pucLocIpv6PrefixAddr,
    VOS_UINT8                          *pucNewIpv6PrefixAddr
)
{
    /* 比较ipv6前缀地址*/
    if (0 != VOS_StrNiCmp((VOS_CHAR *)pucLocIpv6PrefixAddr,
                          (VOS_CHAR *)pucNewIpv6PrefixAddr,
                          (TAF_IPV6_PREFIX_LEN)))
    {
        AT_NORM_LOG("AT_PS_CmpIpv6PrefixAddr: Ipv6 Prefix Addr is diff");
        return VOS_FALSE;
    }

    AT_NORM_LOG("AT_PS_CmpIpv6PrefixAddr: Ipv6 Prefix Addr is same");
    return VOS_TRUE;
}


VOS_UINT64 AT_PS_GenerateRandomIPv6IID(VOS_VOID)
{
    VOS_UINT32                         *pulAddr = VOS_NULL_PTR;
    VOS_UINT64                          ullIID;
    VOS_UINT32                          ulSlice;

    pulAddr = (VOS_UINT32 *)&ullIID;
    ulSlice = VOS_GetSlice();

    /* 在进行PC测试时，不使用随机值 */
    VOS_SetSeed(ulSlice);
    pulAddr[0] = VOS_Rand(0xFFFFFFFFU);
    VOS_SetSeed(pulAddr[0]);
    pulAddr[1] = VOS_Rand(0xFFFFFFFFU);

    *(VOS_UINT8 *)(&pulAddr[0]) &= ~0x02;

    return ullIID;
}


VOS_UINT32 AT_PS_GetIPv6IIDMgrListSize(
    AT_PS_IPV6_IID_MGR_INFO_STRU       *pstMgrInfo
)
{
    HI_LIST_S                          *pstMe  = VOS_NULL_PTR;
    VOS_UINT32                          ulSize = 0;

    msp_list_for_each(pstMe, &(pstMgrInfo->stListHead))
    {
        ulSize++;
    }

    return ulSize;
}


AT_PS_IPV6_IID_ENTRY_STRU* AT_PS_LookupIPv6IIDMgrEntry(
    AT_PS_IPV6_IID_MGR_INFO_STRU       *pstMgrInfo,
    VOS_UINT8                          *pucApn
)
{
    AT_PS_IPV6_IID_ENTRY_STRU          *pstEntry = VOS_NULL_PTR;
    HI_LIST_S                          *pstMe    = VOS_NULL_PTR;
    VOS_UINT32                          ulFound  = VOS_FALSE;

    /* 遍历链表, 匹配APN */
    msp_list_for_each(pstMe, &(pstMgrInfo->stListHead))
    {
        pstEntry = msp_list_entry(pstMe, AT_PS_IPV6_IID_ENTRY_STRU, stList);
        if (!VOS_StrNiCmp((VOS_CHAR *)pstEntry->aucApn,
                          (VOS_CHAR *)pucApn,
                          (TAF_MAX_APN_LEN + 1)))
        {
            ulFound = VOS_TRUE;
            break;
        }
    }

    return ((VOS_TRUE == ulFound) ? pstEntry : VOS_NULL_PTR);
}


AT_PS_IPV6_IID_ENTRY_STRU* AT_PS_EvictIPv6IIDMgrEntry(
    AT_PS_IPV6_IID_MGR_INFO_STRU       *pstMgrInfo
)
{
    HI_LIST_S                          *pstListHead = VOS_NULL_PTR;
    AT_PS_IPV6_IID_ENTRY_STRU          *pstEntry    = VOS_NULL_PTR;

    pstListHead = &(pstMgrInfo->stListHead);
    if (VOS_TRUE == msp_list_empty(pstListHead))
    {
        AT_ERR_LOG("AT_PS_EvictIPv6IIDMgrEntry: list is empty.");
        return VOS_NULL_PTR;
    }

    pstEntry = msp_list_last_entry(pstListHead, AT_PS_IPV6_IID_ENTRY_STRU, stList);
    msp_list_del(&(pstEntry->stList));

    return pstEntry;
}


AT_PS_IPV6_IID_ENTRY_STRU* AT_PS_AllocIPv6IIDMgrEntry(
    AT_PS_IPV6_IID_MGR_INFO_STRU       *pstMgrInfo
)
{
    AT_PS_IPV6_IID_ENTRY_STRU          *pstEntry = VOS_NULL_PTR;

    pstEntry = (AT_PS_IPV6_IID_ENTRY_STRU *)PS_MEM_ALLOC(WUEPS_PID_AT,
                    sizeof(AT_PS_IPV6_IID_ENTRY_STRU));//lint !e516

    return pstEntry;
}


AT_PS_IPV6_IID_ENTRY_STRU* AT_PS_CreateIPv6IIDMgrEntry(
    AT_PS_IPV6_IID_MGR_INFO_STRU       *pstMgrInfo
)
{
    AT_PS_IPV6_IID_ENTRY_STRU          *pstEntry = VOS_NULL_PTR;
    VOS_UINT32                          ulSize;

    ulSize = AT_PS_GetIPv6IIDMgrListSize(pstMgrInfo);
    if (ulSize >= AT_PS_IPV6_IID_MGR_TBL_SIZE)
    {
        /* 链表节点达到最大个数, 从尾部滑出一个节点 */
        AT_NORM_LOG("AT_PS_CreateIPv6IIDMgrEntry: list is full, evict an entry.");
        pstEntry = AT_PS_EvictIPv6IIDMgrEntry(pstMgrInfo);
    }
    else
    {
        /* 分配新节点, 如果分配失败, 从尾部滑出一个节点 */
        pstEntry = AT_PS_AllocIPv6IIDMgrEntry(pstMgrInfo);
        if (VOS_NULL_PTR == pstEntry)
        {
            AT_ERR_LOG("AT_PS_CreateIPv6IIDMgrEntry: alloc an entry failed, try to evict an entry.");
            pstEntry = AT_PS_EvictIPv6IIDMgrEntry(pstMgrInfo);
        }
    }

    if (VOS_NULL_PTR != pstEntry)
    {
        /* 初始化节点 */
        AT_NORM_LOG("AT_PS_CreateIPv6IIDMgrEntry: entry is created.");
        TAF_MEM_SET_S(pstEntry, sizeof(AT_PS_IPV6_IID_ENTRY_STRU),
                      0x00, sizeof(AT_PS_IPV6_IID_ENTRY_STRU));
    }

    return pstEntry;
}


VOS_UINT64 AT_PS_GetIPv6IID(
    MODEM_ID_ENUM_UINT16                enModemId,
    VOS_UINT8                          *pucApn,
    VOS_UINT8                          *pucPrefix
)
{
    AT_PS_IPV6_IID_MGR_INFO_STRU       *pstMgrInfo = VOS_NULL_PTR;
    AT_PS_IPV6_IID_ENTRY_STRU          *pstEntry   = VOS_NULL_PTR;
    VOS_UINT64                          ullIID     = 0;

    /* 获取IPv6接口ID记录管理 */
    pstMgrInfo = AT_GetPsIPv6IIDMgrInfoByModemId(enModemId);
    if (VOS_NULL_PTR == pstMgrInfo)
    {
        AT_ERR_LOG1("AT_PS_GetIPv6IID: modem id is invalid.", enModemId);
        return 0;
    }

    /* 不支持空APN */
    if (0 == VOS_StrLen((VOS_CHAR *)pucApn))
    {
        AT_ERR_LOG("AT_PS_GetIPv6IID: apn length is 0.");
        return 0;
    }

    /* 前缀有效性检查 */
    if (0 == *((VOS_UINT64 *)pucPrefix))
    {
        AT_ERR_LOG("AT_PS_GetIPv6IID: prefix is invalid.");
        return 0;
    }

    /* 根据APN查找IPv6接口ID记录 */
    pstEntry = AT_PS_LookupIPv6IIDMgrEntry(pstMgrInfo, pucApn);
    if (VOS_NULL_PTR == pstEntry)
    {
        /* 未找到记录, 创新一个新的纪录 */
        AT_NORM_LOG("AT_PS_GetIPv6IID: entry is not found.");

        pstEntry = AT_PS_CreateIPv6IIDMgrEntry(pstMgrInfo);
        if (VOS_NULL_PTR != pstEntry)
        {
            /* 创建新纪录成功, 更新记录信息 */
            TAF_MEM_CPY_S(pstEntry->aucApn, sizeof(pstEntry->aucApn),
                          pucApn, VOS_StrLen((VOS_CHAR *)pucApn));
            TAF_MEM_CPY_S(pstEntry->aucPrefix, sizeof(pstEntry->aucPrefix),
                          pucPrefix, TAF_IPV6_PREFIX_LEN);
            pstEntry->ullIID = AT_PS_GenerateRandomIPv6IID();
            msp_list_add(&(pstEntry->stList), &(pstMgrInfo->stListHead));

            ullIID = pstEntry->ullIID;
        }
    }
    else
    {
        /*
         * 找到记录, 匹配IPv6地址前缀
         * 如果匹配, 则使用上一次的接口ID, 否则随机生成并更新记录
         */
        AT_NORM_LOG("AT_PS_GetIPv6IID: entry is found.");

        if (TAF_MEM_CMP(pstEntry->aucPrefix, pucPrefix, TAF_IPV6_PREFIX_LEN))
        {
            TAF_MEM_CPY_S(pstEntry->aucPrefix, sizeof(pstEntry->aucPrefix),
                          pucPrefix, TAF_IPV6_PREFIX_LEN);
            pstEntry->ullIID = AT_PS_GenerateRandomIPv6IID();
        }

        msp_list_del(&(pstEntry->stList));
        msp_list_add(&(pstEntry->stList), &(pstMgrInfo->stListHead));

        ullIID = pstEntry->ullIID;
    }

    return ullIID;
}


VOS_UINT32 AT_PS_GetPdpTypeChgMgrListSize(
    AT_PS_PDP_TYPE_CHG_MGR_INFO_STRU       *pstMgrInfo
)
{
    HI_LIST_S                          *pstMe  = VOS_NULL_PTR;
    VOS_UINT32                          ulSize = 0;

    msp_list_for_each(pstMe, &(pstMgrInfo->stListHead))
    {
        ulSize++;
    }

    return ulSize;
}


AT_PS_PDP_TYPE_CHG_ENTRY_STRU* AT_PS_LookupPdpTypeChgMgrEntry(
    AT_PS_PDP_TYPE_CHG_MGR_INFO_STRU   *pstMgrInfo,
    VOS_UINT8                          *pucApn
)
{
    AT_PS_PDP_TYPE_CHG_ENTRY_STRU      *pstEntry = VOS_NULL_PTR;
    HI_LIST_S                          *pstMe    = VOS_NULL_PTR;
    VOS_UINT32                          ulFound  = VOS_FALSE;

    if (VOS_NULL_PTR == pstMgrInfo)
    {
        AT_ERR_LOG("AT_PS_LookupPdpTypeChgMgrEntry :pstMgrInfo is NULL");
        return VOS_NULL_PTR;
    }

    /* 遍历链表, 匹配APN */
    msp_list_for_each(pstMe, &(pstMgrInfo->stListHead))
    {
        pstEntry = msp_list_entry(pstMe, AT_PS_PDP_TYPE_CHG_ENTRY_STRU, stList);
        if (!VOS_StrNiCmp((VOS_CHAR *)pstEntry->aucApn,
                          (VOS_CHAR *)pucApn,
                          (TAF_MAX_APN_LEN + 1)))
        {
            ulFound = VOS_TRUE;
            break;
        }
    }

    return ((VOS_TRUE == ulFound) ? pstEntry : VOS_NULL_PTR);
}


AT_PS_PDP_TYPE_CHG_ENTRY_STRU* AT_PS_EvictPdpTypeChgMgrEntry(
    AT_PS_PDP_TYPE_CHG_MGR_INFO_STRU       *pstMgrInfo
)
{
    HI_LIST_S                          *pstListHead = VOS_NULL_PTR;
    AT_PS_PDP_TYPE_CHG_ENTRY_STRU      *pstEntry    = VOS_NULL_PTR;

    pstListHead = &(pstMgrInfo->stListHead);
    if (VOS_TRUE == msp_list_empty(pstListHead))
    {
        AT_ERR_LOG("AT_PS_EvictPdpTypeChgMgrEntry: list is empty.");
        return VOS_NULL_PTR;
    }

    pstEntry = msp_list_last_entry(pstListHead, AT_PS_PDP_TYPE_CHG_ENTRY_STRU, stList);
    msp_list_del(&(pstEntry->stList));

    return pstEntry;
}


AT_PS_PDP_TYPE_CHG_ENTRY_STRU* AT_PS_AllocPdpTypeChgMgrEntry(
    AT_PS_PDP_TYPE_CHG_MGR_INFO_STRU       *pstMgrInfo
)
{
    AT_PS_PDP_TYPE_CHG_ENTRY_STRU          *pstEntry = VOS_NULL_PTR;

    pstEntry = (AT_PS_PDP_TYPE_CHG_ENTRY_STRU *)PS_MEM_ALLOC(WUEPS_PID_AT,
                    sizeof(AT_PS_PDP_TYPE_CHG_ENTRY_STRU)); /*lint !e516*/

    return pstEntry;
}


VOS_VOID AT_PS_CreatePdpTypeChgMgrEntry(
    AT_PS_PDP_TYPE_CHG_MGR_INFO_STRU       *pstMgrInfo,
    VOS_UINT8                              *pucApn,
    TAF_PDP_TYPE_ENUM_UINT8                 enPdpType
)
{
    AT_PS_PDP_TYPE_CHG_ENTRY_STRU      *pstEntry = VOS_NULL_PTR;
    VOS_UINT32                          ulSize;

    ulSize = AT_PS_GetPdpTypeChgMgrListSize(pstMgrInfo);
    if (ulSize >= AT_PS_PDP_TYPE_CHG_MGR_TBL_SIZE)
    {
        /* 链表节点达到最大个数, 从尾部滑出一个节点 */
        AT_NORM_LOG("AT_PS_CreatePdpTypeChgMgrEntry: list is full, evict an entry.");
        pstEntry = AT_PS_EvictPdpTypeChgMgrEntry(pstMgrInfo);
    }
    else
    {
        /* 分配新节点, 如果分配失败, 从尾部滑出一个节点 */
        pstEntry = AT_PS_AllocPdpTypeChgMgrEntry(pstMgrInfo);
        if (VOS_NULL_PTR == pstEntry)
        {
            AT_ERR_LOG("AT_PS_CreatePdpTypeChgMgrEntry: alloc an entry failed, try to evict an entry.");
        }
    }

    if (VOS_NULL_PTR != pstEntry)
    {
        /* 初始化节点 */
        AT_NORM_LOG("AT_PS_CreatePdpTypeChgMgrEntry: entry is created.");
        TAF_MEM_SET_S(pstEntry, sizeof(AT_PS_PDP_TYPE_CHG_ENTRY_STRU),
                      0x00, sizeof(AT_PS_PDP_TYPE_CHG_ENTRY_STRU));

        /* 创建新纪录成功, 更新记录信息 */
        TAF_MEM_CPY_S(pstEntry->aucApn, sizeof(pstEntry->aucApn),
                      pucApn, VOS_StrLen((VOS_CHAR *)pucApn));

        pstEntry->enPdpType = enPdpType;

        msp_list_add(&(pstEntry->stList), &(pstMgrInfo->stListHead));

        return;
    }

    AT_ERR_LOG("AT_PS_CreatePdpTypeChgMgrEntry: pstEntry is null!");

    return;
}


VOS_VOID AT_PS_ProcPdpTypeChgMgrList(
    VOS_UINT16                              usClientId,
    VOS_UINT8                               ucAPNLen,
    VOS_UINT8                              *pucApn,
    TAF_PS_CAUSE_ENUM_UINT32                enCause
)
{
    AT_COMM_PS_CTX_STRU                *pstCommPsCtx    = VOS_NULL_PTR;
    AT_PS_PDP_TYPE_CHG_MGR_INFO_STRU   *pstMgrInfo      = VOS_NULL_PTR;
    AT_PS_PDP_TYPE_CHG_ENTRY_STRU      *pstEntry        = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          i;
    VOS_UINT8                           ucFindNvCauseFlg;

    ucFindNvCauseFlg        = VOS_FALSE;
    enModemId               = MODEM_ID_0;
    i                       = 0;

    if (VOS_OK != AT_GetModemIdFromClient(usClientId, &enModemId))
    {
        AT_ERR_LOG1("AT_PS_ProcDialPdpTypeChg: modem id is invalid.", enModemId);
        return;
    }

    pstCommPsCtx    = AT_GetCommPsCtxAddr();
    pstMgrInfo      = AT_GetPsPdpTypeChgMgrInfoByModemId(enModemId);

    if (VOS_NULL_PTR == pstMgrInfo)
    {
        AT_ERR_LOG("AT_PS_ProcPdpTypeChgMgrList :pstMgrInfo is NULL");
        return;
    }

    if ( (VOS_TRUE == pstCommPsCtx->astPdpTypeChgPolicyCfg[enModemId].ucPdpTypeChgEnableFlg)
      && (0 != ucAPNLen))
    {
        for (i = 0; i < pstCommPsCtx->astPdpTypeChgPolicyCfg[enModemId].ucRejCauseNum; i++)
        {
            if (enCause == pstCommPsCtx->astPdpTypeChgPolicyCfg[enModemId].astPdpTypeChgInfo[i].ucCauseValue + TAF_PS_CAUSE_SM_NW_SECTION_BEGIN)
            {
                /* 找到对应定制的项 */
                ucFindNvCauseFlg = VOS_TRUE;
                break;
            }
        }
    }

    if (VOS_TRUE == ucFindNvCauseFlg)
    {
        pstEntry = AT_PS_LookupPdpTypeChgMgrEntry(pstMgrInfo, pucApn);

        if (VOS_NULL_PTR != pstEntry)
        {
            /* 找到对应节点，需要更新PDP TYPE */
            pstEntry->enPdpType     = pstCommPsCtx->astPdpTypeChgPolicyCfg[enModemId].astPdpTypeChgInfo[i].enPdpType;

            msp_list_del(&(pstEntry->stList));
            msp_list_add(&(pstEntry->stList), &(pstMgrInfo->stListHead));

            AT_NORM_LOG1("AT_PS_ProcDialPdpTypeChg: find node and update pdp type is ", pstEntry->enPdpType);
        }
        else
        {
            /* 没找到对应节点，需要新增节点 */
            AT_PS_CreatePdpTypeChgMgrEntry(pstMgrInfo,
                                           pucApn,
                                           pstCommPsCtx->astPdpTypeChgPolicyCfg[enModemId].astPdpTypeChgInfo[i].enPdpType);
        }
    }

    return;
}


AT_PS_PDP_TYPE_CHG_ENTRY_STRU* AT_PS_GetPdpTypeChgMgrNode(
    VOS_UINT16                              usClientId,
    VOS_UINT8                               ucCallId
)
{
    AT_COMM_PS_CTX_STRU                *pstCommPsCtx    = VOS_NULL_PTR;
    AT_PS_PDP_TYPE_CHG_MGR_INFO_STRU   *pstMgrInfo      = VOS_NULL_PTR;
    AT_PS_PDP_TYPE_CHG_ENTRY_STRU      *pstEntry        = VOS_NULL_PTR;
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity   = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId               = MODEM_ID_0;
    pstCommPsCtx            = AT_GetCommPsCtxAddr();

    if (VOS_OK != AT_GetModemIdFromClient(usClientId, &enModemId))
    {
        AT_ERR_LOG1("AT_PS_GetPdpTypeChgMgrNode: modem id is invalid.", enModemId);
        return pstEntry;
    }

    if (VOS_FALSE == pstCommPsCtx->astPdpTypeChgPolicyCfg[enModemId].ucPdpTypeChgEnableFlg)
    {
        return pstEntry;
    }

    pstMgrInfo      = AT_GetPsPdpTypeChgMgrInfoByModemId(enModemId);
    pstCallEntity   = AT_PS_GetCallEntity(usClientId, ucCallId);

    pstEntry = AT_PS_LookupPdpTypeChgMgrEntry(pstMgrInfo, pstCallEntity->stUsrDialParam.aucAPN);

    return pstEntry;
}



VOS_UINT32 AT_PS_IsIpv6CapabilityValid(VOS_UINT8 ucCapability)
{
    if ((AT_IPV6_CAPABILITY_IPV4_ONLY == ucCapability)
     || (AT_IPV6_CAPABILITY_IPV6_ONLY == ucCapability)
     || (AT_IPV6_CAPABILITY_IPV4V6_OVER_ONE_PDP == ucCapability))
    {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}


VOS_UINT32 AT_PS_IsIpv6Support(VOS_VOID)
{
    VOS_UINT8                           ucIpv6Capability;

    ucIpv6Capability = AT_GetCommPsCtxAddr()->ucIpv6Capability;

    if ((AT_IPV6_CAPABILITY_IPV6_ONLY == ucIpv6Capability)
     || (AT_IPV6_CAPABILITY_IPV4V6_OVER_ONE_PDP == ucIpv6Capability))
    {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

VOS_UINT32 AT_PS_GenIpv6LanAddrWithRadomIID(
    VOS_UINT8                          *pucPrefix,
    VOS_UINT32                          ulPrefixByteLen,
    VOS_UINT8                          *pucIpv6LanAddr
)
{
    VOS_UINT32                         *paulAddr = VOS_NULL_PTR;
    VOS_UINT32                          ulTick;

    paulAddr = (VOS_UINT32 *)pucIpv6LanAddr;

    if (ulPrefixByteLen > AT_IPV6_ADDR_PREFIX_BYTE_LEN)
    {
        return VOS_ERR;
    }

    /* 拷贝IPv6地址前缀 */
    TAF_MEM_CPY_S(pucIpv6LanAddr, TAF_IPV6_ADDR_LEN, pucPrefix, ulPrefixByteLen);

    ulTick = VOS_GetTick();

    /* 在进行PC测试时，不使用随机值 */
    VOS_SetSeed(ulTick);
    paulAddr[2] = VOS_Rand(0xFFFFFFFFU);

    VOS_SetSeed(ulTick + 10);
    paulAddr[3] = VOS_Rand(0xFFFFFFFFU);


    *(VOS_UINT8 *)(&paulAddr[2]) &= ~0x02;

    return VOS_OK;
}


VOS_VOID  AT_PS_SaveIPv6Dns(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;

    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);

    pstCallEntity->stIpv6Info.stIpv6DhcpInfo.bitOpIpv6PriDns    = VOS_FALSE;
    pstCallEntity->stIpv6Info.stIpv6DhcpInfo.bitOpIpv6SecDns    = VOS_FALSE;

    /* 保存主副DNS，收到RA参数时需要，激活网卡时需要通知DRV */
    if (VOS_TRUE == pstEvent->stIpv6Dns.bitOpPrimDnsAddr)
    {
        TAF_MEM_CPY_S((VOS_VOID*)pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6PrimDNS,
                sizeof(pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6PrimDNS),
                pstEvent->stIpv6Dns.aucPrimDnsAddr,
                AT_MAX_IPV6_DNS_LEN);
        pstCallEntity->stIpv6Info.stIpv6DhcpInfo.bitOpIpv6PriDns = VOS_TRUE;
    }

    if (VOS_TRUE == pstEvent->stIpv6Dns.bitOpSecDnsAddr)
    {
        TAF_MEM_CPY_S((VOS_VOID*)pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6SecDNS,
                   sizeof(pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6SecDNS),
                   pstEvent->stIpv6Dns.aucSecDnsAddr,
                   AT_MAX_IPV6_DNS_LEN);
        pstCallEntity->stIpv6Info.stIpv6DhcpInfo.bitOpIpv6SecDns = VOS_TRUE;
    }

}


VOS_VOID  AT_PS_SaveIPv6Pcscf(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);

    pstCallEntity->stIpv6Info.stIpv6DhcpInfo.stIpv6PcscfList.ucIpv6PcscfAddrNum = AT_MIN(TAF_PCSCF_ADDR_MAX_NUM,
                                                                              pstEvent->stIpv6PcscfList.ucIpv6PcscfAddrNum);

    for (ucIndex = 0; ucIndex < pstCallEntity->stIpv6Info.stIpv6DhcpInfo.stIpv6PcscfList.ucIpv6PcscfAddrNum; ucIndex++)
    {
        TAF_MEM_CPY_S(pstCallEntity->stIpv6Info.stIpv6DhcpInfo.stIpv6PcscfList.astIpv6PcscfAddrList[ucIndex].aucPcscfAddr,
                      TAF_IPV6_ADDR_LEN,
                      pstEvent->stIpv6PcscfList.astIpv6PcscfAddrList[ucIndex].aucPcscfAddr,
                      TAF_IPV6_ADDR_LEN);
    }

    return;
}


VOS_VOID AT_PS_ProcConnectedIpv6Addr(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;

    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);

    pstCallEntity->stIpv6Info.stIpv6DhcpInfo.ucRabId         = pstEvent->ucRabId;
    pstCallEntity->stIpv6Info.stIpv6DhcpInfo.ucPduSessionId  = pstEvent->ucPduSessionId;

    /* 处理IPv6的IP地址，形式为网络序 */
    /* 域选业务不使用临时地址 */
    if (VOS_FALSE == pstCallEntity->stApnDataSysInfo.ucDataSysInfoFlg)
    {
        TAF_MEM_CPY_S(pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6TmpAddr,
                      (VOS_UINT32)(sizeof(pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6TmpAddr)),
                      pstEvent->stPdpAddr.aucIpv6Addr,
                      TAF_IPV6_ADDR_LEN);
    }

    if (VOS_FALSE == AT_PS_GetPsCallHandOverFlg(pstCallEntity))
    {
        TAF_MEM_CPY_S(pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6Addr,
                      (VOS_UINT32)(sizeof(pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6Addr)),
                      pstEvent->stPdpAddr.aucIpv6Addr,
                      TAF_IPV6_ADDR_LEN);
    }

    /* 处理IPV6的主副DNS地址，形式为网络序 */
    AT_PS_SaveIPv6Dns(ucCallId, pstEvent);

    /* 处理IPV6的主副PCSCF地址，形式为网络序 */
    AT_PS_SaveIPv6Pcscf(ucCallId, pstEvent);

    return;
}


TAF_PS_CALL_END_CAUSE_ENUM_UINT8 AT_PS_ChgIpv6RaFailCauseToCallEndCause(
    TAF_PS_IPV6_INFO_RESULT_ENUM_UINT8          enIpv6Rst
)
{
    TAF_PS_CALL_END_CAUSE_ENUM_UINT8        enCallEndCause;

    enCallEndCause  = TAF_PS_CALL_END_CAUSE_BUTT;

    switch (enIpv6Rst)
    {
        case TAF_PS_IPV6_INFO_RESULT_ALLOC_FAIL:
            enCallEndCause = TAF_PS_CALL_END_CAUSE_IPV6_ADDR_ALLOC_FAIL;
            break;

        case TAF_PS_IPV6_INFO_RESULT_REFRESH_FAIL:
            enCallEndCause = TAF_PS_CALL_END_CAUSE_IPV6_ADDR_TIME_OUT;
            break;

        default:
            enCallEndCause = TAF_PS_CALL_END_CAUSE_NORMAL;
            break;
    }

    return enCallEndCause;
}


VOS_VOID AT_PS_ProcIpv6HoRaFail(
    TAF_PS_IPV6_INFO_IND_STRU          *pstRaInfoNotifyInd,
    VOS_UINT8                           ucCallId
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;

    pstCallEntity   = AT_PS_GetCallEntity(pstRaInfoNotifyInd->stCtrl.usClientId, ucCallId);

    /* 记录IP地址改变的错误码 */
    AT_PS_SetPsCallErrCause(pstRaInfoNotifyInd->stCtrl.usClientId, TAF_PS_CAUSE_IP_ADDRESS_CHG_IN_HANDOVER);

    AT_PS_SetCallHandOverFailCause(pstCallEntity->stUserInfo.enPortIndex, ucCallId, TAF_PS_CAUSE_IP_ADDRESS_CHG_IN_HANDOVER);

    AT_PS_SetCallHandOverFlg(pstCallEntity->stUserInfo.enPortIndex, ucCallId, VOS_FALSE);

    /* 设置handover呼叫类型 */
    AT_PS_SetHoCallType(pstCallEntity->stUserInfo.enPortIndex, ucCallId, TAF_PDP_TYPE_BUTT);

    /* 发起WLAN本地去激活 */
    (VOS_VOID)AT_PS_HangupWlanCall(pstCallEntity->stUserInfo.enUserIndex, ucCallId, VOS_TRUE, VOS_TRUE);

    /* IP地址有变化，需要执行CELLULAR上去激活 */
    (VOS_VOID)AT_PS_HangupCall(pstCallEntity->stUserInfo.enUserIndex, ucCallId, TAF_PS_CALL_END_CAUSE_NORMAL);

    return;
}


VOS_VOID AT_PS_ProcIpv6RaFail(
    TAF_PS_IPV6_INFO_IND_STRU          *pstRaInfoNotifyInd,
    VOS_UINT8                           ucCallId
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;

    pstCallEntity   = AT_PS_GetCallEntity(pstRaInfoNotifyInd->stCtrl.usClientId, ucCallId);

    if (VOS_FALSE == AT_PS_GetPsCallHandOverFlg(pstCallEntity))
    {
        if ( (AT_PS_CALL_INVALID_CID == pstCallEntity->stIpv4Info.ucIpv4Cid)
          && (AT_PDP_STATE_IDLE == pstCallEntity->stIpv4Info.enIpv4State))
        {
            /* IPV4地址不存在，需要去激活该承载 */
            if (VOS_OK == TAF_PS_CallEndEx(WUEPS_PID_AT,
                                           AT_PS_BuildPsCallExClientId(pstRaInfoNotifyInd->stCtrl.usClientId, ucCallId),
                                           0,
                                           pstCallEntity->stIpv6Info.ucIpv6Cid,
                                           AT_PS_ChgIpv6RaFailCauseToCallEndCause(pstRaInfoNotifyInd->enIpv6Rst)))
            {
                AT_PS_SetCallStateByType(pstRaInfoNotifyInd->stCtrl.usClientId, ucCallId, TAF_PDP_IPV6, AT_PDP_STATE_DEACTING);
            }
            else
            {
                AT_ERR_LOG("AT_PS_ProcIpv6RaFail: Hangup call failed!");
            }
        }
        else
        {
            /* 存在IPV4地址，直接上报AP IPV6 DEND信息 */
            /* 其实IPV6的承载还是存在的，只是IPV6地址不可用，之后真正去激活IPV6的承载时，不用再上报IPV6的DEND信息 */
            AT_PS_SndCallEndedResult(pstRaInfoNotifyInd->stCtrl.usClientId,
                                     ucCallId,
                                     TAF_PDP_IPV6,
                                     TAF_PS_CAUSE_IPV6_ADDRESS_DISABLE,
                                     TAF_PS_APN_DATA_SYS_CELLULAR);
        }
    }
    else
    {
        AT_PS_ProcIpv6HoRaFail(pstRaInfoNotifyInd, ucCallId);
    }

    return;
}


VOS_VOID AT_PS_FillIpv6RaInfo(
    TAF_PS_IPV6_INFO_IND_STRU          *pstRaInfoNotifyInd,
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
)
{
    AT_IPV6_RA_INFO_STRU               *pstIpv6RaInfo = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT64                          ullIID;

    enModemId = MODEM_ID_0;
    if (VOS_OK != AT_GetModemIdFromClient(pstRaInfoNotifyInd->stCtrl.usClientId, &enModemId))
    {
        AT_ERR_LOG("AT_PS_FillIpv6RaInfo: get modem id failed!");
        return;
    }

    pstIpv6RaInfo = &pstCallEntity->stIpv6Info.stIpv6RaInfo;

    /* 记录IPv6前缀 */
    /* 没有获取到前缀或者前缀有变化的时候才去重新获取接口ID，但是现在前缀有效期是永久生效，一旦之后不是永久生效后需要和AP对接 */
    if ( (VOS_FALSE == pstIpv6RaInfo->bitOpPrefixAddr)
      || (TAF_MEM_CMP(pstIpv6RaInfo->aucPrefixAddr, pstRaInfoNotifyInd->stIpv6RaInfo.astPrefixList[0].aucPrefix, TAF_IPV6_PREFIX_LEN)))
    {
        pstIpv6RaInfo->bitOpPrefixAddr    = VOS_TRUE;
        pstIpv6RaInfo->ulPrefixBitLen     = pstRaInfoNotifyInd->stIpv6RaInfo.astPrefixList[0].ulBitPrefixLen;
        TAF_MEM_CPY_S(pstIpv6RaInfo->aucPrefixAddr,
                      (VOS_UINT32)(sizeof(pstIpv6RaInfo->aucPrefixAddr)),
                      pstRaInfoNotifyInd->stIpv6RaInfo.astPrefixList[0].aucPrefix,
                      TAF_IPV6_ADDR_LEN);

        AT_NORM_LOG("AT_PS_FillIpv6RaInfo: Get Ipv6 IID.");

        ullIID = AT_PS_GetIPv6IID(enModemId,
                                  pstCallEntity->stUsrDialParam.aucAPN,
                                  pstIpv6RaInfo->aucPrefixAddr);
        if (0 == ullIID)
        {
            ullIID = AT_PS_GenerateRandomIPv6IID();
        }

        /* 记录IPv6全局地址 */
        pstIpv6RaInfo->bitOpLanAddr = VOS_TRUE;
        TAF_MEM_CPY_S(pstIpv6RaInfo->aucLanAddr,
                      TAF_IPV6_PREFIX_LEN,
                      pstIpv6RaInfo->aucPrefixAddr,
                      TAF_IPV6_PREFIX_LEN);

        TAF_MEM_CPY_S(pstIpv6RaInfo->aucLanAddr + TAF_IPV6_PREFIX_LEN,
                      TAF_IPV6_IID_LEN,
                      &ullIID,
                      (VOS_UINT32)(sizeof(ullIID)));

        /* 更新DHCPV6信息中的IPv6临时地址 域选业务不使用临时地址*/
        if (VOS_FALSE == pstCallEntity->stApnDataSysInfo.ucDataSysInfoFlg)
        {
            TAF_MEM_CPY_S(pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6TmpAddr,
                          TAF_IPV6_PREFIX_LEN,
                          pstIpv6RaInfo->aucPrefixAddr,
                          TAF_IPV6_PREFIX_LEN);
        }

        /* 更新DHCPV6信息中的IPv6全局地址 */
        TAF_MEM_CPY_S(pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6Addr,
                      (VOS_UINT32)(sizeof(pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6Addr)),
                      pstIpv6RaInfo->aucLanAddr,
                      (VOS_UINT32)(sizeof(pstIpv6RaInfo->aucLanAddr)));
    }

    /* 记录Preferred Lifetime */
    pstIpv6RaInfo->bitOpPreferredLifetime = VOS_TRUE;
    pstIpv6RaInfo->ulPreferredLifetime    = pstRaInfoNotifyInd->stIpv6RaInfo.astPrefixList[0].ulPreferredLifeTime;

    /* 记录Valid Lifetime */
    pstIpv6RaInfo->bitOpValidLifetime     = VOS_TRUE;
    pstIpv6RaInfo->ulValidLifetime        = pstRaInfoNotifyInd->stIpv6RaInfo.astPrefixList[0].ulValidLifeTime;

    /* 记录IPv6 MTU */
    if (VOS_TRUE == pstRaInfoNotifyInd->stIpv6RaInfo.bitOpMtu)
    {
        pstIpv6RaInfo->bitOpMtuSize       = VOS_TRUE;
        pstIpv6RaInfo->ulMtuSize          = pstRaInfoNotifyInd->stIpv6RaInfo.ulMtu;
    }

    return;
}


VOS_VOID AT_PS_ProcHoIpv6RaInfo(
    TAF_PS_IPV6_INFO_IND_STRU          *pstRaInfoNotifyInd,
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    VOS_UINT8                           ucCallId
)
{
    /* IPV6地址前缀不相等 */
    if ( (0 == pstRaInfoNotifyInd->stIpv6RaInfo.ulPrefixNum)
      || (VOS_FALSE == AT_PS_CmpIpv6PrefixAddr(pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6Addr, pstRaInfoNotifyInd->stIpv6RaInfo.astPrefixList[0].aucPrefix)))
    {
        /* 记录IP地址改变的错误码 */
        AT_PS_SetPsCallErrCause(pstRaInfoNotifyInd->stCtrl.usClientId, TAF_PS_CAUSE_IP_ADDRESS_CHG_IN_HANDOVER);

        AT_PS_SetCallHandOverFailCause(pstRaInfoNotifyInd->stCtrl.usClientId, ucCallId, TAF_PS_CAUSE_IP_ADDRESS_CHG_IN_HANDOVER);

        AT_PS_SetCallHandOverFlg(pstCallEntity->stUserInfo.enPortIndex, ucCallId, VOS_FALSE);

        /* 设置handover呼叫类型 */
        AT_PS_SetHoCallType(pstCallEntity->stUserInfo.enPortIndex, ucCallId, TAF_PDP_TYPE_BUTT);

        /* 发起WLAN本地去激活 */
        (VOS_VOID)AT_PS_HangupWlanCall(pstCallEntity->stUserInfo.enUserIndex, ucCallId, VOS_TRUE, VOS_TRUE);

        /* IP地址有变化，需要执行CELLULAR上去激活 */
        (VOS_VOID)AT_PS_HangupCall(pstCallEntity->stUserInfo.enUserIndex, ucCallId, TAF_PS_CALL_END_CAUSE_NORMAL);

        return;
    }

    AT_PS_FillIpv6RaInfo(pstRaInfoNotifyInd, pstCallEntity);

    if (VOS_TRUE == AT_PS_IsHoEndCellularConnect(pstCallEntity, TAF_PDP_IPV6))
    {
        AT_PS_ProcHoEndCellularConnect(pstCallEntity, ucCallId);
    }

    return;
}


VOS_UINT32 AT_PS_MatchIpv4v6ConnFailFallbackCause(
    TAF_PS_CAUSE_ENUM_UINT32            enCause
)
{
    AT_PS_IPV6_BACKPROC_EXT_CAUSE_STRU *pstBackProcExtCause = VOS_NULL_PTR;
    AT_COMM_PS_CTX_STRU                *pstCommPsCtx        = VOS_NULL_PTR;

    VOS_UINT32                          ulRslt;
    VOS_UINT32                          ulCnt;

    pstCommPsCtx    = AT_GetCommPsCtxAddr();
    ulRslt          = VOS_FALSE;

    /* 检查是否匹配拒绝原因值为#28 */
    if (TAF_PS_CAUSE_SM_NW_UNKNOWN_PDP_ADDR_OR_TYPE == enCause)
    {
        if (VOS_FALSE == pstCommPsCtx->stRedialForNwCauseCfg.ucRedialForNoPdpTypeCausePolicy)
        {
            return VOS_FALSE;
        }

        return VOS_TRUE;
    }

    /* 检查是否匹配用户定制原因值 */
    pstBackProcExtCause = &(AT_GetCommPsCtxAddr()->stIpv6BackProcExtCauseTbl);

    for (ulCnt = 0; ulCnt < pstBackProcExtCause->ulCauseNum; ulCnt++)
    {
        if (pstBackProcExtCause->aenPsCause[ulCnt] == enCause)
        {
            ulRslt = VOS_TRUE;
            break;
        }
    }

    return ulRslt;
}


VOS_UINT32 AT_PS_SetupSingleStackConn(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity   = VOS_NULL_PTR;
    AT_DIAL_PARAM_STRU                 *pstUsrDialParam = VOS_NULL_PTR;
    AT_DIAL_PARAM_STRU                  stCallDialParam;
    AT_PDP_STATE_ENUM_U8                enCallState;
    VOS_UINT8                           ucCid;
    VOS_UINT32                          ulRslt;

    TAF_MEM_SET_S(&stCallDialParam, sizeof(stCallDialParam), 0x00, sizeof(AT_DIAL_PARAM_STRU));

    ulRslt          = VOS_ERR;
    ucCid           = 0;
    pstCallEntity   = AT_PS_GetCallEntity(usClientId, ucCallId);
    pstUsrDialParam = &(pstCallEntity->stUsrDialParam);
    enCallState     = AT_PS_GetCallStateByType(pstCallEntity, enPdpType);

    if (AT_PDP_STATE_IDLE == enCallState)
    {
        /* 查找一个未激活的CID进行激活 */
        if (VOS_OK == TAF_AGENT_FindCidForDial(usClientId, &ucCid))
        {
            /* 填写拨号参数 */
            AT_PS_GenCallDialParam(&stCallDialParam,
                                   pstUsrDialParam,
                                   ucCid,
                                   enPdpType);

            /* 发起激活请求 */
            if (VOS_OK == AT_PS_SetupCall(usClientId, ucCallId, &stCallDialParam))
            {
                ulRslt = VOS_OK;
            }
        }
    }
    else
    {
        ulRslt = VOS_OK;
    }

    return ulRslt;
}


VOS_UINT32 AT_PS_HangupSingleStackConn(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity;
    VOS_UINT32                          ulRslt;
    AT_PDP_STATE_ENUM_U8                enCallState;
    VOS_UINT8                           ucCid;

    ulRslt          = VOS_ERR;
    pstCallEntity   = AT_PS_GetCallEntity(usClientId, ucCallId);
    ucCid           = AT_PS_GetCidByCallType(pstCallEntity, enPdpType);
    enCallState     = AT_PS_GetCallStateByType(pstCallEntity, enPdpType);

    switch (enCallState)
    {
        case AT_PDP_STATE_ACTED:
        case AT_PDP_STATE_ACTING:
            if (VOS_OK == TAF_PS_CallEnd(WUEPS_PID_AT,
                                         AT_PS_BuildPsCallExClientId(usClientId, ucCallId),
                                         0,
                                         ucCid))
            {
                ulRslt = VOS_OK;

                AT_PS_SetCallStateByType(usClientId, ucCallId, enPdpType, AT_PDP_STATE_DEACTING);
            }
            else
            {
                AT_ERR_LOG("AT_PS_HangupSingleStackConn: Hangup call failed!");
            }
            break;

        case AT_PDP_STATE_DEACTING:
            ulRslt = VOS_OK;
            break;

        case AT_PDP_STATE_IDLE:
        default:
            break;
    }

    return ulRslt;
}


VOS_VOID AT_PS_ProcDualStackCallConn(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    AT_COMM_PS_CTX_STRU                *pstCommPsCtx = VOS_NULL_PTR;

    pstCommPsCtx = AT_GetCommPsCtxAddr();

    /* 不带原因值或#52原因值，需要发起另一种PDP激活 */
    if ( (VOS_FALSE == pstEvent->bitOpCause)
      || ( (VOS_TRUE == pstEvent->bitOpCause)
        && (TAF_PS_CAUSE_SM_NW_SINGLE_ADDR_BEARERS_ONLY_ALLOWED == pstEvent->enCause) ) )
    {
        switch (pstEvent->stPdpAddr.enPdpType)
        {
            case TAF_PDP_IPV4:
                if ((VOS_FALSE == pstEvent->bitOpCause)
                 && (VOS_FALSE == pstCommPsCtx->stRedialForNwCauseCfg.ucRedialForNoCausePolicy))
                {
                    AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                                             ucCallId,
                                             TAF_PDP_IPV6,
                                             TAF_PS_CAUSE_SM_NW_PDP_TYPE_IPV4_ONLY_ALLOWED,
                                             TAF_PS_APN_DATA_SYS_CELLULAR);
                    break;
                }

                if (VOS_OK != AT_PS_SetupSingleStackConn(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV6))
                {
                    /* 记录呼叫错误码 */
                    AT_PS_SetPsCallErrCause(pstEvent->stCtrl.usClientId, TAF_PS_CAUSE_UNKNOWN);

                    /* 上报IPv6拨号失败 */
                    AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                                             ucCallId,
                                             TAF_PDP_IPV6,
                                             AT_PS_GetPsCallErrCause(pstEvent->stCtrl.usClientId),
                                             TAF_PS_APN_DATA_SYS_CELLULAR);
                }
                break;

            case TAF_PDP_IPV6:
                if ((VOS_FALSE == pstEvent->bitOpCause)
                 && (VOS_FALSE == pstCommPsCtx->stRedialForNwCauseCfg.ucRedialForNoCausePolicy))
                {
                    AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                                             ucCallId,
                                             TAF_PDP_IPV4,
                                             TAF_PS_CAUSE_SM_NW_PDP_TYPE_IPV6_ONLY_ALLOWED,
                                             TAF_PS_APN_DATA_SYS_CELLULAR);
                    break;
                }

                if (VOS_OK != AT_PS_SetupSingleStackConn(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4))
                {
                    /* 记录呼叫错误码 */
                    AT_PS_SetPsCallErrCause(pstEvent->stCtrl.usClientId, TAF_PS_CAUSE_UNKNOWN);

                    /* 上报IPv4拨号失败 */
                    AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                                             ucCallId,
                                             TAF_PDP_IPV4,
                                             AT_PS_GetPsCallErrCause(pstEvent->stCtrl.usClientId),
                                             TAF_PS_APN_DATA_SYS_CELLULAR);
                }
                break;

            default:
                AT_WARN_LOG("AT_PS_ProcDualStackCallConn: PDP type is invalid!");
                break;
        }
    }
    else
    {
        /* 根据原因值, 通知APP拨号结果 */
        switch (pstEvent->enCause)
        {
            /* 原因值#50 (IPv4 ONLY), 通知APP模块IPv6激活失败 */
            case TAF_PS_CAUSE_SM_NW_PDP_TYPE_IPV4_ONLY_ALLOWED:
            case TAF_PS_CAUSE_PDP_TYPE_IPV4_ONLY_ALLOWED:
                AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                                         ucCallId,
                                         TAF_PDP_IPV6,
                                         pstEvent->enCause,
                                         TAF_PS_APN_DATA_SYS_CELLULAR);

                break;

            /* 原因值#51 (IPv6 ONLY), 通知APP模块IPv4激活失败 */
            case TAF_PS_CAUSE_SM_NW_PDP_TYPE_IPV6_ONLY_ALLOWED:
            case TAF_PS_CAUSE_PDP_TYPE_IPV6_ONLY_ALLOWED:
                AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                                         ucCallId,
                                         TAF_PDP_IPV4,
                                         pstEvent->enCause,
                                         TAF_PS_APN_DATA_SYS_CELLULAR);

                break;

            /* 其他原因值, 不处理 */
            default:
                AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                                         ucCallId,
                                         (TAF_PDP_IPV4 == pstEvent->stPdpAddr.enPdpType) ? TAF_PDP_IPV6 : TAF_PDP_IPV4,
                                         pstEvent->enCause,
                                         TAF_PS_APN_DATA_SYS_CELLULAR);
                break;
        }
    }

    return;
}


VOS_VOID AT_PS_ProcIpv4ConnSuccFallback(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    AT_PS_PDP_TYPE_CHG_ENTRY_STRU      *pstEntry        = VOS_NULL_PTR;

    switch (AT_PS_GetCurrCallType(pstEvent->stCtrl.usClientId, ucCallId))
    {
        /* 这种情况不属于PDP回退，是前面PDP激活被拒绝后，分别发起IPV4、
           IPV6的PDP激活。其中，TAF_PDP_IPV6是异常情况，用户发起IPV6的PDP
           激活，而网络回复IPV4的激活，此时也需要重新发起IPV6的PDP激活 */
        case TAF_PDP_IPV4:
            pstEntry = AT_PS_GetPdpTypeChgMgrNode(pstEvent->stCtrl.usClientId, ucCallId);
            if (VOS_NULL_PTR != pstEntry)
            {
                if (TAF_PDP_IPV4 == pstEntry->enPdpType)
                {
                    /* 将该PDP切换到IDLE态 */
                    AT_PS_SetCallStateByType(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV6, AT_PDP_STATE_IDLE);

                    /* 上报IPv6拨号失败 */
                    AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                                             ucCallId,
                                             TAF_PDP_IPV6,
                                             TAF_PS_CAUSE_SM_NW_PDP_TYPE_IPV4_ONLY_ALLOWED,
                                             TAF_PS_APN_DATA_SYS_CELLULAR);

                    break;
                }
            }

            if (VOS_OK != AT_PS_SetupSingleStackConn(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV6))
            {
                /* 记录呼叫错误码 */
                AT_PS_SetPsCallErrCause(pstEvent->stCtrl.usClientId, TAF_PS_CAUSE_UNKNOWN);

                /* 将该PDP切换到IDLE态 */
                AT_PS_SetCallStateByType(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV6, AT_PDP_STATE_IDLE);

                /* 上报IPv6拨号失败 */
                AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                                         ucCallId,
                                         TAF_PDP_IPV6,
                                         AT_PS_GetPsCallErrCause(pstEvent->stCtrl.usClientId),
                                         TAF_PS_APN_DATA_SYS_CELLULAR);
            }
            break;

        /* 这里是PDP激活回退功能实现 */
        case TAF_PDP_IPV4V6:

            /* 设置IPv6对应的CID为无效 */
            AT_PS_SetCid2CurrCall(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV6, AT_PS_CALL_INVALID_CID, TAF_PS_APN_DATA_SYS_CELLULAR);

            /* 将该PDP切换到IDLE态 */
            AT_PS_SetCallStateByType(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV6, AT_PDP_STATE_IDLE);

            AT_PS_ProcDualStackCallConn(ucCallId, pstEvent);

            break;

        default:
            AT_ERR_LOG("AT_PS_ProcIpv4ConnSuccFallback: PDP type is invalid!");
            break;
    }

    return;
}


VOS_VOID AT_PS_ProcIpv6ConnSuccFallback(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    AT_PS_PDP_TYPE_CHG_ENTRY_STRU      *pstEntry        = VOS_NULL_PTR;

    switch (AT_PS_GetCurrCallType(pstEvent->stCtrl.usClientId, ucCallId))
    {
        case TAF_PDP_IPV6:
            /* 这种情况不属于PDP回退，是前面PDP激活被拒绝后，分别发起IPV4、
               IPV6的PDP激活, IPV6激活后不再发起IPV4的PDP激活 */
            pstEntry = AT_PS_GetPdpTypeChgMgrNode(pstEvent->stCtrl.usClientId, ucCallId);
            if (VOS_NULL_PTR != pstEntry)
            {
                if (TAF_PDP_IPV6 == pstEntry->enPdpType)
                {
                    /* 将该PDP切换到IDLE态 */
                    AT_PS_SetCallStateByType(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4, AT_PDP_STATE_IDLE);

                    /* 上报IPv4拨号失败 */
                    AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                                             ucCallId,
                                             TAF_PDP_IPV4,
                                             TAF_PS_CAUSE_SM_NW_PDP_TYPE_IPV6_ONLY_ALLOWED,
                                             TAF_PS_APN_DATA_SYS_CELLULAR);

                }
            }
            break;

        /* 这里是PDP激活回退功能实现 */
        case TAF_PDP_IPV4V6:

            /* 设置IPv4对应的CID为无效 */
            AT_PS_SetCid2CurrCall(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4, AT_PS_CALL_INVALID_CID, TAF_PS_APN_DATA_SYS_CELLULAR);

            /* 将该PDP切换到IDLE态 */
            AT_PS_SetCallStateByType(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4, AT_PDP_STATE_IDLE);

            AT_PS_ProcDualStackCallConn(ucCallId, pstEvent);

            break;

        default:
            AT_ERR_LOG("AT_PS_ProcIpv6ConnSuccFallback: PDP type is invalid!");
            break;
    }

    return;
}


VOS_UINT32 AT_PS_ProcIpv4ConnFailFallback(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU  *pstEvent
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity       = VOS_NULL_PTR;
    AT_DIAL_PARAM_STRU                  stCallDialParam;
    VOS_UINT32                          ulRslt;

    TAF_MEM_SET_S(&stCallDialParam, sizeof(stCallDialParam), 0x00, sizeof(AT_DIAL_PARAM_STRU));

    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);
    ulRslt        = VOS_OK;

    if (AT_PDP_STATE_IDLE == pstCallEntity->stIpv6Info.enIpv6State)
    {
        /* 填写拨号参数 */
        AT_PS_GenCallDialParam(&stCallDialParam,
                               &pstCallEntity->stUsrDialParam,
                               pstCallEntity->stUserInfo.ucUsrCid,
                               TAF_PDP_IPV6);

        /* 发起IPv6类型的PDP激活请求 */
        if (VOS_OK != AT_PS_SetupCall(pstEvent->stCtrl.usClientId, ucCallId, &stCallDialParam))
        {
            ulRslt = VOS_ERR;
        }
    }

    return ulRslt;
}


VOS_UINT32 AT_PS_ProcIpv4v6ConnFailFallback(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU  *pstEvent
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity       = VOS_NULL_PTR;
    AT_DIAL_PARAM_STRU                  stCallDialParam;
    VOS_UINT32                          ulRslt;

    TAF_MEM_SET_S(&stCallDialParam, sizeof(stCallDialParam), 0x00, sizeof(AT_DIAL_PARAM_STRU));

    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);
    ulRslt        = VOS_ERR;

    /* 填写拨号参数 */
    AT_PS_GenCallDialParam(&stCallDialParam,
                           &pstCallEntity->stUsrDialParam,
                           pstCallEntity->stUserInfo.ucUsrCid,
                           TAF_PDP_IPV4);

    /* 发起IPv4类型的PDP激活请求 */
    if (VOS_OK == AT_PS_SetupCall(pstEvent->stCtrl.usClientId, ucCallId, &stCallDialParam))
    {
        ulRslt = VOS_OK;
    }

    return ulRslt;
}


VOS_UINT32 AT_PS_CmpIpv4Addr(
    VOS_UINT32                          ulLocalIpv4Addr,
    VOS_UINT32                          ulNewIpv4Addr
)
{
    /* 比较ipv4地址*/
    if (ulLocalIpv4Addr != ulNewIpv4Addr)
    {
        AT_NORM_LOG("AT_PS_CmpIpv4Addr: Ipv4 Addr is diff");
        return VOS_FALSE;
    }

    AT_NORM_LOG("AT_PS_CmpIpv4Addr: Ipv4 Addr is same");
    return VOS_TRUE;
}


VOS_UINT32 AT_PS_CmpIpv4v6AddrHoWlanConn(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    WLAN_AT_PDN_ACTIVATE_CNF_STRU      *pstWlanPdnActivateCnf
)
{
    VOS_UINT32                          ulIpv4Rslt;
    VOS_UINT32                          ulIpv6Rslt;
    TAF_PDP_TYPE_ENUM_UINT8             enWlanPdpType;

    ulIpv4Rslt      = VOS_TRUE;
    ulIpv6Rslt      = VOS_TRUE;
    enWlanPdpType   = AT_PS_ConvertPdpType2Cellular(pstWlanPdnActivateCnf->enPdnType);

    /* CELLULAR2WLAN，双栈变为单栈的场景，现在直接认为是地址不匹配，切换失败处理 */
    if (enWlanPdpType != pstCallEntity->stDialPdpType.enHoPdpType)
    {
        AT_WARN_LOG("AT_PS_CmpIpv4v6AddrHoWlanConn: pdp type is diff");
        return VOS_FALSE;
    }

    /* 比较ipv4地址 */
    if ( (TAF_PDP_IPV4 == enWlanPdpType)
      || (TAF_PDP_IPV4V6 == enWlanPdpType) )
    {
        ulIpv4Rslt = AT_PS_CmpIpv4Addr(pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4Addr, AT_GetLanAddr32((VOS_UINT8 *)(pstWlanPdnActivateCnf->stPdpAddr.aucIpV4Addr)));
    }

    /* 比较ipv6地址 */
    if ( (TAF_PDP_IPV6 == enWlanPdpType)
      || (TAF_PDP_IPV4V6 == enWlanPdpType) )
    {
        ulIpv6Rslt = AT_PS_CmpIpv6PrefixAddr(pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6Addr,
                                             (VOS_UINT8 *)(pstWlanPdnActivateCnf->stPdpAddr.aucIpV6Addr));
    }

    if ( (VOS_FALSE == ulIpv4Rslt)
      || (VOS_FALSE == ulIpv6Rslt) )
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}


VOS_VOID AT_PS_ProcIpv4HoCallConnected(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    AT_PS_CALL_ENTITY_STRU                 *pstCallEntity          = VOS_NULL_PTR;

    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);

    /* IPV4地址无变化 */
    if (VOS_TRUE == AT_PS_CmpIpv4Addr(pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4Addr, AT_GetLanAddr32(pstEvent->stPdpAddr.aucIpv4Addr)))
    {
        /* 将本IPv4类型PDP状态切换到激活状态 */
        AT_PS_SetCallStateByType(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4, AT_PDP_STATE_ACTED);

        /* 处理IPv4类型的DHCP */
        AT_PS_ProcConnInd(ucCallId, pstEvent);

        /* 向网卡发送PDP激活事件 */
        AT_PS_ActivateIFace(ucCallId, pstEvent, TAF_PDP_IPV4);

        /* 向FC注册流控点 */
        AT_PS_RegFCPoint(ucCallId, pstEvent);

        AT_PS_ReportCustomPcoInfo(&pstEvent->stCustomPcoInfo,
                            TAF_PS_PDN_OPERATE_TYPE_ACTIVE,
                            pstCallEntity->stUserInfo.ucUsrCid,
                            pstEvent->stPdpAddr.enPdpType,
                            pstCallEntity->stUserInfo.enPortIndex);

        if (VOS_TRUE == pstEvent->bitOpIpv4Mtu)
        {
            pstCallEntity->stIpv4Info.usIpv4Mtu = pstEvent->usIpv4Mtu;
        }

        if (VOS_TRUE == AT_PS_IsHoEndCellularConnect(pstCallEntity, TAF_PDP_IPV4))
        {
            AT_PS_ProcHoEndCellularConnect(pstCallEntity, ucCallId);
            return;
        }
        else
        {
            /* handover发起的是IPV4V6拨号 */
            AT_PS_ProcIpv4ConnSuccFallback(ucCallId, pstEvent);

            if ( (AT_PDP_STATE_ACTING == pstCallEntity->stIpv6Info.enIpv6State)
              || ( (AT_PDP_STATE_ACTED == pstCallEntity->stIpv6Info.enIpv6State)
                && (VOS_FALSE == pstCallEntity->stIpv6Info.stIpv6RaInfo.bitOpPrefixAddr)))
            {
                /* handover流程继续进行，等待RA信息后再处理 */
                AT_NORM_LOG("AT_PS_ProcIpv4HoCallConnected: handover is not end.");
                return;
            }
        }
    }

    /* 记录IP地址改变的错误码 */
    AT_PS_SetPsCallErrCause(pstEvent->stCtrl.usClientId, TAF_PS_CAUSE_IP_ADDRESS_CHG_IN_HANDOVER);

    AT_PS_SetCallHandOverFailCause(pstEvent->stCtrl.usClientId, ucCallId, TAF_PS_CAUSE_IP_ADDRESS_CHG_IN_HANDOVER);

    AT_PS_SetCallHandOverFlg(pstCallEntity->stUserInfo.enPortIndex, ucCallId, VOS_FALSE);

    /* 设置handover呼叫类型 */
    AT_PS_SetHoCallType(pstCallEntity->stUserInfo.enPortIndex, ucCallId, TAF_PDP_TYPE_BUTT);

    /* 发起WLAN本地去激活 */
    (VOS_VOID)AT_PS_HangupWlanCall(pstCallEntity->stUserInfo.enUserIndex, ucCallId, VOS_TRUE, VOS_TRUE);

    /* IP地址有变化，需要执行CELLULAR上去激活 */
    (VOS_VOID)AT_PS_HangupCall(pstCallEntity->stUserInfo.enUserIndex, ucCallId, TAF_PS_CALL_END_CAUSE_NORMAL);

    return;
}


VOS_VOID AT_PS_ProcIpv4CallConnected(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity;

    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);

    if (VOS_FALSE == AT_PS_GetPsCallHandOverFlg(pstCallEntity))
    {
        /* 将本IPv4类型PDP状态切换到激活状态 */
        AT_PS_SetCallStateByType(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4, AT_PDP_STATE_ACTED);

        /* 处理IPv4类型的DHCP */
        AT_PS_ProcConnInd(ucCallId, pstEvent);

        /* 上报IPv4连接状态 */
        AT_PS_SndCallConnectedResult(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4);

        /* 向网卡发送PDP激活事件 */
        AT_PS_ActivateIFace(ucCallId, pstEvent, TAF_PDP_IPV4);

        /* 向FC注册流控点 */
        AT_PS_RegFCPoint(ucCallId, pstEvent);

        /* 将指定CID的PDP的激活状态设置为激活态 */
        AT_SetAtChdataCidActStatus(pstEvent->stCtrl.usClientId, ucCallId, pstCallEntity->stUserInfo.ucUsrCid);

        AT_PS_ReportCustomPcoInfo(&pstEvent->stCustomPcoInfo,
                            TAF_PS_PDN_OPERATE_TYPE_ACTIVE,
                            pstCallEntity->stUserInfo.ucUsrCid,
                            pstEvent->stPdpAddr.enPdpType,
                            pstCallEntity->stUserInfo.enPortIndex);


        if (VOS_TRUE == pstEvent->bitOpIpv4Mtu)
        {
            pstCallEntity->stIpv4Info.usIpv4Mtu = pstEvent->usIpv4Mtu;
        }

        /* 如果用户发起的是IPV4V6，需要激活另一个PDP */
        if (VOS_TRUE == AT_PS_IsUsrDialTypeDualStack(pstCallEntity))
        {
            AT_PS_ProcIpv4ConnSuccFallback(ucCallId, pstEvent);
        }
        if ( (AT_PDP_STATE_IDLE == pstCallEntity->stIpv6Info.enIpv6State)
          || ( (AT_PDP_STATE_ACTED == pstCallEntity->stIpv6Info.enIpv6State)
            && (VOS_TRUE == pstCallEntity->stIpv6Info.stIpv6RaInfo.bitOpPrefixAddr)))
        {
            /* 注册数据系统域改变通知 */
            AT_PS_RegDataSysChgNtf(pstCallEntity);
        }
    }
    else
    {
        AT_PS_ProcIpv4HoCallConnected(ucCallId, pstEvent);
    }

    return;
}


VOS_VOID AT_PS_ProcHoCellularCallReject(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);

    if (VOS_TRUE == AT_PS_GetPsCallHandOverFlg(pstCallEntity))
    {
        AT_PS_SetCallHandOverFlg(usClientId, ucCallId, VOS_FALSE);

        AT_PS_SetCallHandOverFailCause(usClientId, ucCallId, AT_PS_GetPsCallErrCause(usClientId));

        /* 设置handover呼叫类型 */
        AT_PS_SetHoCallType(usClientId, ucCallId, TAF_PDP_TYPE_BUTT);

        /* 发起WLAN本地去激活 */
        (VOS_VOID)AT_PS_HangupWlanCall(usClientId, ucCallId, VOS_TRUE, VOS_TRUE);

        /* 发起CELLULAR上去激活 */
        (VOS_VOID)AT_PS_HangupCall(AT_PS_GetUserInfo(usClientId, ucCallId)->enUserIndex, ucCallId, TAF_PS_CALL_END_CAUSE_NORMAL);
    }

    return;
}


VOS_UINT8 AT_PS_ProcHoCallThrotRejectCause(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity   = VOS_NULL_PTR;

    pstCallEntity   = AT_PS_GetCallEntity(usClientId, ucCallId);

    if ( (VOS_TRUE == AT_PS_GetPsCallHandOverFlg(pstCallEntity))
      && (TAF_PDP_TYPE_BUTT != pstCallEntity->stDialPdpType.enWlanPdpType))
    {
        /* Cellular下被拒原因值为调节算法拒绝，则保持WLAN下状态，等待THROT模块的订阅通知后再发起WLAN2CELLULAR的流程 */
        if ( (TAF_PS_CAUSE_SM_THROT_ALG_NOT_ALLOWED == AT_PS_GetPsCallErrCause(usClientId))
          || (TAF_PS_CAUSE_SM_THROT_T3396_IS_RUNNING == AT_PS_GetPsCallErrCause(usClientId))
          || (TAF_PS_CAUSE_SM_THROT_BACKOFF_IS_RUNNING == AT_PS_GetPsCallErrCause(usClientId)))
        {
            AT_NORM_LOG("AT_PS_ProcHoCallThrotRejectCause: handover is end because of throt in cellular!");

            /* 设置Handover结束 */
            AT_PS_SetCallHandOverFlg(usClientId, ucCallId, VOS_FALSE);

            /* 设置当前域为WLAN */
            AT_PS_SetPsCallCurrentDataSys(pstCallEntity, TAF_PS_APN_DATA_SYS_WLAN);

            /* 将相应类型的PDP状态切换到IDLE */
            AT_PS_SetCallStateByType(usClientId, ucCallId, pstCallEntity->stDialPdpType.enCurrPdpType, AT_PDP_STATE_IDLE);

            /* 还原呼叫CID */
            AT_PS_SetCid2CurrCall(usClientId, ucCallId, pstCallEntity->stDialPdpType.enHoPdpType, pstCallEntity->stUsrDialParam.ucCid, TAF_PS_APN_DATA_SYS_WLAN);

            /* 设置handover呼叫类型 */
            AT_PS_SetHoCallType(usClientId, ucCallId, TAF_PDP_TYPE_BUTT);

            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}



VOS_VOID AT_PS_ProcIpv4CallReject(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU  *pstEvent
)
{
    /* 用户发起IPv4v6类型的PDP激活, 而且被网络拒绝, 原因为28, 协议栈需要
       分别发起IPv4/IPv6类型的PDP激活, 协议栈首先发起IPv4, 再发起IPv6,
       如果IPv4类型的PDP激活再次被网络拒绝, 协议栈还需要尝试IPV6类型的
       PDP激活为了防止PDP激活嵌套, 如果IPv6类型的PDP激活失败, 将不再尝试
       IPv4类型的PDP激活 */

    AT_PDP_STATE_ENUM_U8                enPreCallState;
    TAF_PDP_TYPE_ENUM_UINT8             enPrePdpTypeChgValue;
    AT_PS_PDP_TYPE_CHG_ENTRY_STRU      *pstEntry        = VOS_NULL_PTR;
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity   = VOS_NULL_PTR;


    pstCallEntity           = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);
    enPreCallState          = AT_PS_GetCallStateByType(pstCallEntity, TAF_PDP_IPV4);
    enPrePdpTypeChgValue    = TAF_PDP_TYPE_BUTT;

    if (VOS_TRUE == AT_PS_ProcHoCallThrotRejectCause(pstEvent->stCtrl.usClientId, ucCallId))
    {
        AT_NORM_LOG("AT_PS_ProcIpv4CallReject: handover is end because of throt in cellular!");
        return;
    }

    /* 设置IPv4对应的CID为无效 */
    AT_PS_SetCid2CurrCall(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4, AT_PS_CALL_INVALID_CID, TAF_PS_APN_DATA_SYS_CELLULAR);

    /* 将IPv4类型的PDP状态切换到IDLE */
    AT_PS_SetCallStateByType(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4, AT_PDP_STATE_IDLE);

    /* 上报IPv4拨号失败 */
    AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                             ucCallId,
                             TAF_PDP_IPV4,
                             pstEvent->enCause,
                             TAF_PS_APN_DATA_SYS_CELLULAR);

    /* 释放CALLID和CID的映射关系 */
    AT_PS_FreeCallIdToCid(pstEvent->stCtrl.usClientId, pstEvent->ucCid, ucCallId, TAF_PS_APN_DATA_SYS_CELLULAR);

    if ( (AT_PS_IsUsrDialTypeDualStack(pstCallEntity))
      && (AT_PDP_STATE_ACTING == enPreCallState) )
    {
        pstEntry                = AT_PS_GetPdpTypeChgMgrNode(pstEvent->stCtrl.usClientId, ucCallId);

        if (VOS_NULL_PTR != pstEntry)
        {
            enPrePdpTypeChgValue    = pstEntry->enPdpType;
        }

        AT_PS_ProcPdpTypeChgMgrList(pstEvent->stCtrl.usClientId,
                                    pstCallEntity->stUsrDialParam.ucAPNLen,
                                    pstCallEntity->stUsrDialParam.aucAPN,
                                    pstEvent->enCause);

        if (TAF_PDP_IPV4 == enPrePdpTypeChgValue)
        {
            /* 上报IPv6拨号失败 */
            AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                                     ucCallId,
                                     TAF_PDP_IPV6,
                                     TAF_PS_CAUSE_SM_NW_PDP_TYPE_IPV4_ONLY_ALLOWED,
                                     TAF_PS_APN_DATA_SYS_CELLULAR);
        }
        else
        {
            if (VOS_OK == AT_PS_ProcIpv4ConnFailFallback(ucCallId, pstEvent))
            {
                AT_ERR_LOG("AT_PS_ProcIpv4CallReject: 14004AT_PS_ProcIpv4ConnFailFallback.");
                return;
            }
            else
            {
                /* 记录呼叫错误码 */
                AT_PS_SetPsCallErrCause(pstEvent->stCtrl.usClientId, TAF_PS_CAUSE_UNKNOWN);

                /* 上报IPv6拨号失败 */
                AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                                         ucCallId,
                                         TAF_PDP_IPV6,
                                         AT_PS_GetPsCallErrCause(pstEvent->stCtrl.usClientId),
                                         TAF_PS_APN_DATA_SYS_CELLULAR);
            }
        }
    }

    AT_ERR_LOG("AT_PS_ProcIpv4CallReject: AT_PS_FreeCallEntity.");

    /* 释放呼叫实体 */
    AT_PS_FreeCallEntity(pstEvent->stCtrl.usClientId, ucCallId, TAF_PS_APN_DATA_SYS_CELLULAR);

    AT_PS_ProcHoCellularCallReject(pstEvent->stCtrl.usClientId, ucCallId);

    return;
}


VOS_VOID AT_PS_ProcIpv4CallEnded(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity;

    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);

    AT_PS_ReportCustomPcoInfo(&pstEvent->stCustomPcoInfo,
                            TAF_PS_PDN_OPERATE_TYPE_DEACTIVE,
                            pstCallEntity->stUserInfo.ucUsrCid,
                            pstEvent->enPdpType,
                            pstCallEntity->stUserInfo.enPortIndex);

    /* 设置IPv4对应的CID为无效 */
    AT_PS_SetCid2CurrCall(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4, AT_PS_CALL_INVALID_CID, TAF_PS_APN_DATA_SYS_CELLULAR);

    /* 将IPv4类型的PDP状态切换到IDLE */
    AT_PS_SetCallStateByType(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4, AT_PDP_STATE_IDLE);

    /* 上报IPv4去激活 */
    AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                             ucCallId,
                             TAF_PDP_IPV4,
                             pstEvent->enCause,
                             TAF_PS_APN_DATA_SYS_CELLULAR);

    /* 向网卡发送PDP去激活事件 */
    AT_PS_DeactivateIFace(ucCallId, pstEvent, TAF_PDP_IPV4);

    /* 向FC去注册流控点 */
    AT_PS_DeRegFCPoint(ucCallId, pstEvent);

    /* 最后清除CID与数传通道的映射关系 */
    AT_CleanAtChdataCfg(pstEvent->stCtrl.usClientId, pstCallEntity->stUserInfo.ucUsrCid, ucCallId, TAF_PS_APN_DATA_SYS_CELLULAR);

    /* 释放CALLID和CID的映射关系 */
    AT_PS_FreeCallIdToCid(pstEvent->stCtrl.usClientId, pstEvent->ucCid, ucCallId, TAF_PS_APN_DATA_SYS_CELLULAR);

    /* 如果IPv6类型的PDP还处于激活状态, 需要将其去激活 */
    if (AT_PS_IsUsrDialTypeDualStack(pstCallEntity))
    {
        if (VOS_OK == AT_PS_HangupSingleStackConn(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV6))
        {
             AT_ERR_LOG("AT_PS_ProcIpv4CallEnded: AT_PS_HangupSingleStackConn.");
            return;
        }

        if (TAF_PDP_IPV4 == AT_PS_GetCurrCallType(pstEvent->stCtrl.usClientId, ucCallId))
        {
            /* 上报IPv6拨号断开 */
            AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                                     ucCallId,
                                     TAF_PDP_IPV6,
                                     TAF_PS_CAUSE_SUCCESS,
                                     TAF_PS_APN_DATA_SYS_CELLULAR);
        }
    }

    AT_ERR_LOG("AT_PS_ProcIpv4CallEnded: AT_PS_FreeCallEntity.");

    /* 清除PDP实体 */
    AT_PS_FreeCallEntity(pstEvent->stCtrl.usClientId, ucCallId, TAF_PS_APN_DATA_SYS_CELLULAR);

    return;
}


VOS_VOID AT_PS_ProcIpv6HoCallConnected(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity;

    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);

    /* 将本IPv6类型状态切换到激活状态 */
    AT_PS_SetCallStateByType(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV6, AT_PDP_STATE_ACTED);

    /* 处理IPV6地址 */
    AT_PS_ProcConnectedIpv6Addr(ucCallId, pstEvent);

    /* 向网卡发送PDP激活事件 */
    AT_PS_ActivateIFace(ucCallId, pstEvent, TAF_PDP_IPV6);

    /* 向FC注册流控点 */
    AT_PS_RegFCPoint(ucCallId, pstEvent);

    AT_PS_ReportCustomPcoInfo(&pstEvent->stCustomPcoInfo,
                        TAF_PS_PDN_OPERATE_TYPE_ACTIVE,
                        pstCallEntity->stUserInfo.ucUsrCid,
                        pstEvent->stPdpAddr.enPdpType,
                        pstCallEntity->stUserInfo.enPortIndex);

    if ( (TAF_PDP_IPV6 == pstCallEntity->stDialPdpType.enHoPdpType)
      || ( (TAF_PDP_IPV4V6 == pstCallEntity->stDialPdpType.enHoPdpType)
        && (AT_PDP_STATE_ACTED == pstCallEntity->stIpv4Info.enIpv4State)))
    {
        /* 需要等待IPV6地址前缀后再处理 */
        AT_NORM_LOG("AT_PS_ProcIpv6HoCallConnected: handover is not end, wait ipv6 address.");

        return;
    }
    else
    {
        /* handover发起的是IPV4V6拨号 */
        AT_PS_ProcIpv6ConnSuccFallback(ucCallId, pstEvent);

        if (AT_PDP_STATE_ACTING == pstCallEntity->stIpv4Info.enIpv4State)
        {
            /* handover流程继续进行 */
            AT_NORM_LOG("AT_PS_ProcIpv6HoCallConnected: handover is not end, wait ipv4 result.");

            return;
        }
    }

    /* 记录IP地址改变的错误码 */
    AT_PS_SetPsCallErrCause(pstEvent->stCtrl.usClientId, TAF_PS_CAUSE_IP_ADDRESS_CHG_IN_HANDOVER);

    AT_PS_SetCallHandOverFailCause(pstEvent->stCtrl.usClientId, ucCallId, TAF_PS_CAUSE_IP_ADDRESS_CHG_IN_HANDOVER);

    AT_PS_SetCallHandOverFlg(pstCallEntity->stUserInfo.enPortIndex, ucCallId, VOS_FALSE);

    /* 设置handover呼叫类型 */
    AT_PS_SetHoCallType(pstCallEntity->stUserInfo.enPortIndex, ucCallId, TAF_PDP_TYPE_BUTT);

    /* 发起WLAN本地去激活 */
    (VOS_VOID)AT_PS_HangupWlanCall(pstCallEntity->stUserInfo.enUserIndex, ucCallId, VOS_TRUE, VOS_TRUE);

    /* IP地址有变化，需要执行CELLULAR上去激活 */
    (VOS_VOID)AT_PS_HangupCall(pstCallEntity->stUserInfo.enUserIndex, ucCallId, TAF_PS_CALL_END_CAUSE_NORMAL);

    return;
}


VOS_VOID AT_PS_ProcIpv6CallConnected(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity;

    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);

    if (VOS_FALSE == AT_PS_GetPsCallHandOverFlg(pstCallEntity))
    {
        /* 将本IPv6类型状态切换到激活状态 */
        AT_PS_SetCallStateByType(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV6, AT_PDP_STATE_ACTED);

        /* 处理IPV6地址 */
        AT_PS_ProcConnectedIpv6Addr(ucCallId, pstEvent);

        /* 向网卡发送PDP激活事件 */
        AT_PS_ActivateIFace(ucCallId, pstEvent, TAF_PDP_IPV6);

        /* 向FC注册流控点 */
        AT_PS_RegFCPoint(ucCallId, pstEvent);

        /* 将指定CID的PDP的激活状态设置为激活态 */
        AT_SetAtChdataCidActStatus(pstEvent->stCtrl.usClientId, ucCallId, pstCallEntity->stUserInfo.ucUsrCid);

        AT_PS_ReportCustomPcoInfo(&pstEvent->stCustomPcoInfo,
                            TAF_PS_PDN_OPERATE_TYPE_ACTIVE,
                            pstCallEntity->stUserInfo.ucUsrCid,
                            pstEvent->stPdpAddr.enPdpType,
                            pstCallEntity->stUserInfo.enPortIndex);

        /* 如果用户发起的是IPV4V6，需要激活另一个PDP */
        if (AT_PS_IsUsrDialTypeDualStack(pstCallEntity))
        {
            AT_PS_ProcIpv6ConnSuccFallback(ucCallId, pstEvent);
        }
    }
    else
    {
        AT_PS_ProcIpv6HoCallConnected(ucCallId, pstEvent);
    }

    return;
}


VOS_VOID AT_PS_ProcIpv4v6HoCallConnected(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity;

    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);

    /* 比较IPV4地址无变化 */
    if (VOS_TRUE == AT_PS_CmpIpv4Addr(pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4Addr, AT_GetLanAddr32(pstEvent->stPdpAddr.aucIpv4Addr)))
    {
        /* IPv4v6类型状态切换到激活状态 */
        AT_PS_SetCallStateByType(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4V6, AT_PDP_STATE_ACTED);

        /* 处理IPv4类型的DHCP */
        AT_PS_ProcConnInd(ucCallId, pstEvent);

        /* 处理IPV6地址 */
        AT_PS_ProcConnectedIpv6Addr(ucCallId, pstEvent);

        /* 向网卡发送PDP激活事件 */
        AT_PS_ActivateIFace(ucCallId, pstEvent, TAF_PDP_IPV4V6);

        /* 向FC注册流控点 */
        AT_PS_RegFCPoint(ucCallId, pstEvent);

        if (VOS_TRUE == pstEvent->bitOpIpv4Mtu)
        {
            pstCallEntity->stIpv4Info.usIpv4Mtu = pstEvent->usIpv4Mtu;
        }

        AT_PS_ReportCustomPcoInfo(&pstEvent->stCustomPcoInfo,
                            TAF_PS_PDN_OPERATE_TYPE_ACTIVE,
                            pstCallEntity->stUserInfo.ucUsrCid,
                            pstEvent->stPdpAddr.enPdpType,
                            pstCallEntity->stUserInfo.enPortIndex);

        /* 需要等待IPV6地址前缀后再处理 */
        AT_NORM_LOG("AT_PS_ProcIpv4v6HoCallConnected: handover is not end, wait ipv6 address.");
    }
    else
    {
        /* 记录IP地址改变的错误码 */
        AT_PS_SetPsCallErrCause(pstEvent->stCtrl.usClientId, TAF_PS_CAUSE_IP_ADDRESS_CHG_IN_HANDOVER);

        AT_PS_SetCallHandOverFailCause(pstEvent->stCtrl.usClientId, ucCallId, TAF_PS_CAUSE_IP_ADDRESS_CHG_IN_HANDOVER);

        AT_PS_SetCallHandOverFlg(pstCallEntity->stUserInfo.enPortIndex, ucCallId, VOS_FALSE);

        /* 设置handover呼叫类型 */
        AT_PS_SetHoCallType(pstCallEntity->stUserInfo.enPortIndex, ucCallId, TAF_PDP_TYPE_BUTT);

        /* 发起WLAN本地去激活 */
        (VOS_VOID)AT_PS_HangupWlanCall(pstCallEntity->stUserInfo.enUserIndex, ucCallId, VOS_TRUE, VOS_TRUE);

        /* IP地址有变化，需要执行CELLULAR上去激活 */
        (VOS_VOID)AT_PS_HangupCall(pstCallEntity->stUserInfo.enUserIndex, ucCallId, TAF_PS_CALL_END_CAUSE_NORMAL);
    }

    return;
}


VOS_VOID AT_PS_ProcIpv4v6CallConnected(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity;

    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);

    if (VOS_FALSE == AT_PS_GetPsCallHandOverFlg(pstCallEntity))
    {
        /* IPv4v6类型状态切换到激活状态 */
        AT_PS_SetCallStateByType(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4V6, AT_PDP_STATE_ACTED);

        /* 处理IPV4地址 */
        AT_PS_ProcConnInd(ucCallId, pstEvent);

        /* 处理IPV6地址 */
        AT_PS_ProcConnectedIpv6Addr(ucCallId, pstEvent);

        /* 上报IPv4激活指示, IPv6类型的激活指示需要在获取RA信息后上报 */
        AT_PS_SndCallConnectedResult(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4);

        /* 向网卡发送PDP激活事件 */
        AT_PS_ActivateIFace(ucCallId, pstEvent, TAF_PDP_IPV4V6);

        /* 向FC注册流控点 */
        AT_PS_RegFCPoint(ucCallId, pstEvent);

        /* 将指定CID的PDP的激活状态设置为激活态 */
        AT_SetAtChdataCidActStatus(pstEvent->stCtrl.usClientId, ucCallId, pstCallEntity->stUserInfo.ucUsrCid);

        if (VOS_TRUE == pstEvent->bitOpIpv4Mtu)
        {
            pstCallEntity->stIpv4Info.usIpv4Mtu = pstEvent->usIpv4Mtu;
        }

        AT_PS_ReportCustomPcoInfo(&pstEvent->stCustomPcoInfo,
                            TAF_PS_PDN_OPERATE_TYPE_ACTIVE,
                            pstCallEntity->stUserInfo.ucUsrCid,
                            pstEvent->stPdpAddr.enPdpType,
                            pstCallEntity->stUserInfo.enPortIndex);
    }
    else
    {
        AT_PS_ProcIpv4v6HoCallConnected(ucCallId, pstEvent);
    }

    return;
}


VOS_VOID AT_PS_ProcIpv6CallReject(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU  *pstEvent
)
{
    AT_PS_PDP_TYPE_CHG_ENTRY_STRU      *pstEntry        = VOS_NULL_PTR;
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity   = VOS_NULL_PTR;
    TAF_PDP_TYPE_ENUM_UINT8             enPrePdpTypeChgValue;

    enPrePdpTypeChgValue    = TAF_PDP_TYPE_BUTT;
    pstCallEntity           = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);

    if (TAF_PDP_IPV6 == pstCallEntity->stDialPdpType.enHoPdpType)
    {
        if (VOS_TRUE == AT_PS_ProcHoCallThrotRejectCause(pstEvent->stCtrl.usClientId, ucCallId))
        {
            AT_NORM_LOG("AT_PS_ProcIpv6CallReject: handover is end because of throt in cellular!");
            return;
        }
    }

    if (AT_PS_IsUsrDialTypeDualStack(pstCallEntity))
    {
        pstEntry = AT_PS_GetPdpTypeChgMgrNode(pstEvent->stCtrl.usClientId, ucCallId);

        if (VOS_NULL_PTR != pstEntry)
        {
            enPrePdpTypeChgValue    = pstEntry->enPdpType;
        }


        AT_PS_ProcPdpTypeChgMgrList(pstEvent->stCtrl.usClientId,
                                    pstCallEntity->stUsrDialParam.ucAPNLen,
                                    pstCallEntity->stUsrDialParam.aucAPN,
                                    pstEvent->enCause);

        if (TAF_PDP_IPV6 == enPrePdpTypeChgValue)
        {
            /* 上报IPv4拨号失败 */
            AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                                     ucCallId,
                                     TAF_PDP_IPV4,
                                     TAF_PS_CAUSE_SM_NW_PDP_TYPE_IPV6_ONLY_ALLOWED,
                                     TAF_PS_APN_DATA_SYS_CELLULAR);
        }
    }

    /* 如果IPv6类型, 就不需要再尝试IPv4, 因为前面已经发起过IPv4类型的PDP
       激活, 如果再发起IPv4类型的PDP激活的话, 可能会导致PDP激活嵌套 */

    /* 设置IPv6对应的CID为无效 */
    AT_PS_SetCid2CurrCall(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV6, AT_PS_CALL_INVALID_CID, TAF_PS_APN_DATA_SYS_CELLULAR);

    /* 将IPv6类型的PDP状态切换到IDLE */
    AT_PS_SetCallStateByType(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV6, AT_PDP_STATE_IDLE);

    /* 上报IPv6去激活 */
    AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                             ucCallId,
                             TAF_PDP_IPV6,
                             pstEvent->enCause,
                             TAF_PS_APN_DATA_SYS_CELLULAR);

    /* 释放CALLID和CID的映射关系 */
    AT_PS_FreeCallIdToCid(pstEvent->stCtrl.usClientId, pstEvent->ucCid, ucCallId, TAF_PS_APN_DATA_SYS_CELLULAR);

    /* 若同一个实体中的IPV4状态为IDLE则需要清除实体 */
    if (AT_PDP_STATE_IDLE == AT_PS_GetCallStateByType(pstCallEntity, TAF_PDP_IPV4))
    {
        AT_ERR_LOG("AT_PS_ProcIpv6CallReject: AT_PS_FreeCallEntity.");

        /* 释放呼叫实体 */
        AT_PS_FreeCallEntity(pstEvent->stCtrl.usClientId, ucCallId, TAF_PS_APN_DATA_SYS_CELLULAR);
    }

    AT_PS_ProcHoCellularCallReject(pstEvent->stCtrl.usClientId, ucCallId);

    return;
}


VOS_VOID AT_PS_ProcIpv4v6CallReject(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU  *pstEvent
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity   = VOS_NULL_PTR;

    pstCallEntity   = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);

    if (VOS_TRUE == AT_PS_ProcHoCallThrotRejectCause(pstEvent->stCtrl.usClientId, ucCallId))
    {
        AT_NORM_LOG("AT_PS_ProcIpv4v6CallReject: handover is end because of throt in cellular!");
        return;
    }

    /* 设置IPv4v6对应的CID为无效 */
    AT_PS_SetCid2CurrCall(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4V6, AT_PS_CALL_INVALID_CID, TAF_PS_APN_DATA_SYS_CELLULAR);

    /* 将IPv4v6类型的PDP状态切换到IDLE */
    AT_PS_SetCallStateByType(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4V6, AT_PDP_STATE_IDLE);

    /* 释放CALLID和CID的映射关系 */
    AT_PS_FreeCallIdToCid(pstEvent->stCtrl.usClientId, pstEvent->ucCid, ucCallId, TAF_PS_APN_DATA_SYS_CELLULAR);

    AT_PS_ProcPdpTypeChgMgrList(pstEvent->stCtrl.usClientId,
                                pstCallEntity->stUsrDialParam.ucAPNLen,
                                pstCallEntity->stUsrDialParam.aucAPN,
                                pstEvent->enCause);

    /* IPv4v6连接失败回退处理 */
    if (VOS_TRUE == AT_PS_MatchIpv4v6ConnFailFallbackCause(pstEvent->enCause))
    {
        if (VOS_OK == AT_PS_ProcIpv4v6ConnFailFallback(ucCallId, pstEvent))
        {
            return;
        }
        else
        {
            /* 记录呼叫错误码 */
            AT_PS_SetPsCallErrCause(pstEvent->stCtrl.usClientId, TAF_PS_CAUSE_UNKNOWN);
        }
    }

    /* 上报IPv4拨号失败 */
    AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                             ucCallId,
                             TAF_PDP_IPV4,
                             AT_PS_GetPsCallErrCause(pstEvent->stCtrl.usClientId),
                             TAF_PS_APN_DATA_SYS_CELLULAR);
    /* 上报IPv6拨号失败 */
    AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                             ucCallId,
                             TAF_PDP_IPV6,
                             AT_PS_GetPsCallErrCause(pstEvent->stCtrl.usClientId),
                             TAF_PS_APN_DATA_SYS_CELLULAR);

    AT_ERR_LOG("AT_PS_ProcIpv4v6CallReject: AT_PS_FreeCallEntity.");

    /* 释放呼叫实体 */
    AT_PS_FreeCallEntity(pstEvent->stCtrl.usClientId, ucCallId, TAF_PS_APN_DATA_SYS_CELLULAR);

    AT_PS_ProcHoCellularCallReject(pstEvent->stCtrl.usClientId, ucCallId);

    return;
}


VOS_VOID AT_PS_ProcIpv6CallEnded(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity;

    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);

    AT_PS_ReportCustomPcoInfo(&pstEvent->stCustomPcoInfo,
                            TAF_PS_PDN_OPERATE_TYPE_DEACTIVE,
                            pstCallEntity->stUserInfo.ucUsrCid,
                            pstEvent->enPdpType,
                            pstCallEntity->stUserInfo.enPortIndex);

    /* 设置IPv6对应的CID为无效 */
    AT_PS_SetCid2CurrCall(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV6, AT_PS_CALL_INVALID_CID, TAF_PS_APN_DATA_SYS_CELLULAR);

    /* 将IPv6类型的PDP状态切换到IDLE */
    AT_PS_SetCallStateByType(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV6, AT_PDP_STATE_IDLE);

    /* 上报IPv6去激活 */
    AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                             ucCallId,
                             TAF_PDP_IPV6,
                             pstEvent->enCause,
                             TAF_PS_APN_DATA_SYS_CELLULAR);

    /* 向网卡发送PDP去激活事件 */
    AT_PS_DeactivateIFace(ucCallId, pstEvent, TAF_PDP_IPV6);

    /* 向FC去注册流控点 */
    AT_PS_DeRegFCPoint(ucCallId, pstEvent);

    /* 最后清除CID与数传通道的映射关系 */
    AT_CleanAtChdataCfg(pstEvent->stCtrl.usClientId, pstCallEntity->stUserInfo.ucUsrCid, ucCallId, TAF_PS_APN_DATA_SYS_CELLULAR);

    /* 释放CALLID和CID的映射关系 */
    AT_PS_FreeCallIdToCid(pstEvent->stCtrl.usClientId, pstEvent->ucCid, ucCallId, TAF_PS_APN_DATA_SYS_CELLULAR);

    /* 如果IPv4类型的PDP还处于激活状态, 需要将其去激活 */
    if (VOS_OK == AT_PS_HangupSingleStackConn(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4))
    {
        AT_ERR_LOG("AT_PS_ProcIpv6CallEnded: 14378AT_PS_HangupSingleStackConn.");
        return;
    }

    /* 释放呼叫实体 */
    AT_PS_FreeCallEntity(pstEvent->stCtrl.usClientId, ucCallId, TAF_PS_APN_DATA_SYS_CELLULAR);

    AT_ERR_LOG("AT_PS_ProcIpv6CallEnded: AT_PS_FreeCallEntity.");

    return;
}


VOS_VOID AT_PS_ProcIpv4v6CallEnded(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity;

    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);

    AT_PS_ReportCustomPcoInfo(&pstEvent->stCustomPcoInfo,
                            TAF_PS_PDN_OPERATE_TYPE_DEACTIVE,
                            pstCallEntity->stUserInfo.ucUsrCid,
                            pstEvent->enPdpType,
                            pstCallEntity->stUserInfo.enPortIndex);

    /* 设置IPv4v6对应的CID为无效 */
    AT_PS_SetCid2CurrCall(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4V6, AT_PS_CALL_INVALID_CID, TAF_PS_APN_DATA_SYS_CELLULAR);

    /* 将IPv4v6类型的PDP状态切换到IDLE */
    AT_PS_SetCallStateByType(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4V6, AT_PDP_STATE_IDLE);

    /* 上报IPv4和IPv6连接断开指示 */
    AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                             ucCallId,
                             TAF_PDP_IPV4V6,
                             pstEvent->enCause,
                             TAF_PS_APN_DATA_SYS_CELLULAR);

    /* 向网卡发送PDP去激活事件 */
    AT_PS_DeactivateIFace(ucCallId, pstEvent, TAF_PDP_IPV4V6);

    /* 向FC去注册流控点 */
    AT_PS_DeRegFCPoint(ucCallId, pstEvent);

    /* 最后清除CID与数传通道的映射关系 */
    AT_CleanAtChdataCfg(pstEvent->stCtrl.usClientId, pstCallEntity->stUserInfo.ucUsrCid, ucCallId, TAF_PS_APN_DATA_SYS_CELLULAR);

    /* 释放CALLID和CID的映射关系 */
    AT_PS_FreeCallIdToCid(pstEvent->stCtrl.usClientId, pstEvent->ucCid, ucCallId, TAF_PS_APN_DATA_SYS_CELLULAR);

    AT_ERR_LOG("AT_PS_ProcIpv4v6CallEnded: AT_PS_FreeCallEntity.");

    /* 清除PDP实体 */
    AT_PS_FreeCallEntity(pstEvent->stCtrl.usClientId, ucCallId, TAF_PS_APN_DATA_SYS_CELLULAR);

    return;
}


VOS_VOID AT_PS_ProcCallConnectedEvent(
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    VOS_UINT8                           ucCallId;

    ucCallId = AT_PS_TransCidToCallId(pstEvent->stCtrl.usClientId, pstEvent->ucCid);

    if (VOS_FALSE == AT_PS_IsCallIdValid(pstEvent->stCtrl.usClientId, ucCallId))
    {
        AT_ERR_LOG("AT_PS_ProcCallConnectedEvent:ERROR: CallId is invalid!");
        return;
    }

    /* 根据PDP类型分别处理*/
    switch (pstEvent->stPdpAddr.enPdpType)
    {
        case TAF_PDP_IPV4:
            /* 处理IPv4的PDP激活成功事件 */
            AT_PS_ProcIpv4CallConnected(ucCallId, pstEvent);
            break;

        case TAF_PDP_IPV6:
            /* 处理IPv6的PDP激活成功事件 */
            AT_PS_ProcIpv6CallConnected(ucCallId, pstEvent);
            break;

        case TAF_PDP_IPV4V6:
            /* 处理IPv4v6的PDP激活成功事件 */
            AT_PS_ProcIpv4v6CallConnected(ucCallId, pstEvent);
            break;

        default:
            AT_WARN_LOG("AT_PS_ProcCallConnectedEvent:WARNING: PDP type is invaild!");
            break;
    }

    return;
}


VOS_VOID AT_PS_ProcCallRejectEvent(
    TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU   *pstEvent
)
{
    VOS_UINT8                           ucCallId;

    ucCallId = AT_PS_TransCidToCallId(pstEvent->stCtrl.usClientId, pstEvent->ucCid);

    if (VOS_FALSE == AT_PS_IsCallIdValid(pstEvent->stCtrl.usClientId, ucCallId))
    {
        AT_ERR_LOG("AT_PS_ProcCallRejectEvent:ERROR: CallId is invalid!");
        return;
    }

    /* 根据PDP类型分别处理*/
    switch (AT_PS_GetCurrCallType(pstEvent->stCtrl.usClientId, ucCallId))
    {
        case TAF_PDP_IPV4:
            /* 处理IPv4的PDP激活被拒事件 */
            AT_PS_ProcIpv4CallReject(ucCallId, pstEvent);
            break;

        case TAF_PDP_IPV6:
            /* 处理IPv6的PDP激活被拒事件 */
            AT_PS_ProcIpv6CallReject(ucCallId, pstEvent);
            break;

        case TAF_PDP_IPV4V6:
            /* 处理IPv4v6的PDP激活被拒事件 */
            AT_PS_ProcIpv4v6CallReject(ucCallId, pstEvent);
            break;

        default:
            AT_WARN_LOG("AT_PS_ProcCallRejectEvent:WARNING: PDP type is invaild!");
            break;
    }

    return;
}


VOS_VOID AT_PS_ProcCallEndedEvent(
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent
)
{
    VOS_UINT8                           ucCallId;

    ucCallId = AT_PS_TransCidToCallId(pstEvent->stCtrl.usClientId, pstEvent->ucCid);

    if (VOS_FALSE == AT_PS_IsCallIdValid(pstEvent->stCtrl.usClientId, ucCallId))
    {
        AT_ERR_LOG("AT_PS_ProcCallEndedEvent:ERROR: CallId is invalid!");
        return;
    }

    /* 根据PDP类型分别处理*/
    switch ( pstEvent->enPdpType )
    {
        case TAF_PDP_IPV4:
            /* 处理IPv4的PDP去激活事件 */
            AT_PS_ProcIpv4CallEnded(ucCallId, pstEvent);
            break;

        case TAF_PDP_IPV6:
            /* 处理IPv6的PDP去激活事件 */
            AT_PS_ProcIpv6CallEnded(ucCallId, pstEvent);
            break;

        case TAF_PDP_IPV4V6:
            /* 处理IPv4v6的PDP去激活事件 */
            AT_PS_ProcIpv4v6CallEnded(ucCallId, pstEvent);
            break;

        default:
            AT_WARN_LOG("AT_PS_ProcCallRejectEvent:WARNING: PDP type is invaild!");
            break;
    }

    return;
}


VOS_VOID AT_PS_ProcCallOrigCnfEvent(TAF_PS_CALL_ORIG_CNF_STRU *pstCallOrigCnf)
{
    /* (1) 将CID翻译成呼叫实体索引
     * (2) 检查呼叫实体索引有效性
     * (3) 检查呼叫错误码&当前呼叫状态
     *     如果成功
     *     ->无处理
     *     如果失败
     *     ->上报^DEND
     *     ->清除CALLID和CID的映射
     *     ->释放呼叫实体
     */
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity;
    AT_PDP_STATE_ENUM_U8                enCallState;
    VOS_UINT8                           ucCallId;

    /* 获取CID关联的呼叫实体索引(CallId) */
    ucCallId      = AT_PS_TransCidToCallId(pstCallOrigCnf->stCtrl.usClientId, pstCallOrigCnf->ucCid);

    if (VOS_FALSE == AT_PS_IsCallIdValid(pstCallOrigCnf->stCtrl.usClientId, ucCallId))
    {
        AT_ERR_LOG("AT_PS_ProcCallOrigCnfEvent: CallId is invalid!");
        return;
    }

    pstCallEntity = AT_PS_GetCallEntity(pstCallOrigCnf->stCtrl.usClientId, ucCallId);

    enCallState = AT_PS_GetCallStateByCid(pstCallEntity, pstCallOrigCnf->ucCid);

    if (TAF_PS_CAUSE_SUCCESS != pstCallOrigCnf->enCause)
    {
        switch (enCallState)
        {
            case AT_PDP_STATE_ACTING:
            case AT_PDP_STATE_DEACTING:

                /* 设置对应的CID为无效 */
                AT_PS_SetCid2CurrCall(pstCallOrigCnf->stCtrl.usClientId,
                                      ucCallId,
                                      AT_PS_GetCurrCallType(pstCallOrigCnf->stCtrl.usClientId, ucCallId),
                                      AT_PS_CALL_INVALID_CID,
                                      TAF_PS_APN_DATA_SYS_CELLULAR);

                /* 设置对应的PDP状态切换到IDLE */
                AT_PS_SetCallStateByType(pstCallOrigCnf->stCtrl.usClientId,
                                         ucCallId,
                                         AT_PS_GetCurrCallType(pstCallOrigCnf->stCtrl.usClientId, ucCallId),
                                         AT_PDP_STATE_IDLE);

                /* 上报拨号失败 */
                AT_PS_SndCallEndedResult(pstCallOrigCnf->stCtrl.usClientId,
                                         ucCallId,
                                         AT_PS_GetCurrCallType(pstCallOrigCnf->stCtrl.usClientId, ucCallId),
                                         pstCallOrigCnf->enCause,
                                         TAF_PS_APN_DATA_SYS_CELLULAR);

                /* 清除CALLID和CID的映射 */
                AT_PS_FreeCallIdToCid(pstCallOrigCnf->stCtrl.usClientId, pstCallOrigCnf->ucCid, ucCallId, TAF_PS_APN_DATA_SYS_CELLULAR);

                break;

            default:
                AT_WARN_LOG1("AT_PS_ProcCallOrigCnfEvent: State is invalid! <state>", enCallState);
                break;
        }

        if (VOS_FALSE == AT_PS_GetPsCallHandOverFlg(pstCallEntity))
        {
            if (VOS_TRUE == AT_PS_IsLinkDown(pstCallEntity))
            {
                AT_ERR_LOG("AT_PS_ProcCallOrigCnfEvent: AT_PS_FreeCallEntity.");

                /* 释放呼叫实体 */
                AT_PS_FreeCallEntity(pstCallOrigCnf->stCtrl.usClientId, ucCallId, TAF_PS_APN_DATA_SYS_CELLULAR);
            }
        }
        else
        {
            /* 切换被拒，需要在WLAN上本地去激活，如果CELLULAR上有承载，需要去激活 */
            AT_PS_ProcHoCellularCallReject(pstCallOrigCnf->stCtrl.usClientId, ucCallId);

            if (VOS_FALSE == AT_PS_IsLinkDown(pstCallEntity))
            {
                (VOS_VOID)AT_PS_HangupCall(AT_PS_GetUserInfo(pstCallOrigCnf->stCtrl.usClientId, ucCallId)->enUserIndex,
                                                               ucCallId,
                                                               TAF_PS_CALL_END_CAUSE_NORMAL);
            }
        }
    }
    return;
}


VOS_VOID AT_PS_ProcCallEndCnfEvent(TAF_PS_CALL_END_CNF_STRU *pstCallEndCnf)
{
    /* (1) 将CID翻译成呼叫实体索引
     * (2) 检查呼叫实体索引有效性
     * (3) 检查呼叫错误码
     *     如果成功
     *     ->无处理
     *     如果失败
     *     ->异常(打印)
     */

    /* 获取CID关联的呼叫实体索引(CallId)
     * (A) CallId无效
     *     -> 返回
     * (B) CallId有效
     *     -> 继续
     */
    VOS_UINT8                           ucCallId;

    ucCallId = AT_PS_TransCidToCallId(pstCallEndCnf->stCtrl.usClientId, pstCallEndCnf->ucCid);

    if (VOS_FALSE == AT_PS_IsCallIdValid(pstCallEndCnf->stCtrl.usClientId, ucCallId))
    {
        AT_ERR_LOG("AT_PS_ProcCallOrigCnfEvent: CallId is invalid!");
        return;
    }

    if (TAF_ERR_NO_ERROR != pstCallEndCnf->enCause)
    {
        AT_ERR_LOG1("AT_PS_ProcCallOrigCnfEvent: End call failed! <cause>", pstCallEndCnf->enCause);
    }

    return;
}


VOS_UINT32 AT_PS_ProcCallModifyEvent(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_MODIFY_CNF_STRU    *pstEvent
)
{
    VOS_UINT8                           ucCallId;
    VOS_UINT8                           ucUserCid;
    AT_MODEM_PS_CTX_STRU               *pstModemPsCtx = VOS_NULL_PTR;
    VOS_UINT32                          ulIfaceId;
    FC_ID_ENUM_UINT8                    enDefaultFcId;
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType;

    ulIfaceId = 0;

    ucCallId = AT_PS_TransCidToCallId(ucIndex, pstEvent->ucCid);
    if (AT_PS_IsCallIdValid(ucIndex, ucCallId))
    {
        /* AT^NDISDUP拨号，用户发起CGCMODE或网侧发起MODIFY IND */
        pstModemPsCtx = AT_GetModemPsCtxAddrFromClientId(pstEvent->stCtrl.usClientId);

        /* 获取对应的用户CID */
        ucUserCid = pstModemPsCtx->astCallEntity[ucCallId].stUserInfo.ucUsrCid;

        /* 获取用户CID对应的网卡ID */
        ulIfaceId = pstModemPsCtx->astCallEntity[ucCallId].ucIfaceId;

        /* 获取网卡ID对应的FC ID */
        enDefaultFcId = AT_PS_GetFcIdByIFaceId(ulIfaceId);

        if (enDefaultFcId >= FC_ID_BUTT)
        {
            return VOS_ERR;
        }

        AT_NotifyFcWhenPdpModify(pstEvent, enDefaultFcId);

        enPdpType = AT_PS_GenPdpTypeByCid(pstEvent->stCtrl.usClientId, ucCallId, pstEvent->ucCid);
        AT_PS_ReportCustomPcoInfo(&pstEvent->stCustomPcoInfo,
                            TAF_PS_PDN_OPERATE_TYPE_MODIFY,
                            ucUserCid,
                            enPdpType,
                            pstModemPsCtx->astCallEntity[ucCallId].stUserInfo.enPortIndex);
    }

    return VOS_OK;
}


VOS_VOID AT_PS_StopAllPsCallTimer(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
)
{
    VOS_UINT32                          ulTmrName;

    /* WLAN下激活PDP定时器需要停止 */
    AT_SET_WLAN_ACT_PDN_CNF_TMR_NAME(ulTmrName);
    if (VOS_NULL_PTR != pstCallEntity->stPsCallTimerInfo.hWaitWlanActCnfTmrHdl)
    {
        AT_StopRelTimer(ulTmrName, &(pstCallEntity->stPsCallTimerInfo.hWaitWlanActCnfTmrHdl));
    }

    /* WLAN下去激活PDP定时器需要停止 */
    AT_SET_WLAN_DEACT_PDN_CNF_TMR_NAME(ulTmrName);
    if (VOS_NULL_PTR != pstCallEntity->stPsCallTimerInfo.hWaitWlanDeActCnfTmrHdl)
    {
        AT_StopRelTimer(ulTmrName, &(pstCallEntity->stPsCallTimerInfo.hWaitWlanDeActCnfTmrHdl));
    }

    /* 防乒乓定时器需要停止 */
    AT_SET_PROTECT_PDN_IN_DATA_SYS_TMR_NAME(ulTmrName);
    if (VOS_NULL_PTR != pstCallEntity->stPsCallTimerInfo.hProtectInDataSysTmrHdl)
    {
        AT_StopRelTimer(ulTmrName, &(pstCallEntity->stPsCallTimerInfo.hProtectInDataSysTmrHdl));
    }

    return;
}


VOS_UINT32 AT_PS_AllocCallEntity(
    VOS_UINT16                          usClientId,
    AT_PS_CALL_TYPE_ENUM_U8             enPsCallType,
    VOS_UINT8                          *pucCallId
)
{
    AT_MNTN_PS_CALL_ENTITY_STRU         stMntnPsCallEntity;
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity       = VOS_NULL_PTR;
    VOS_UINT32                          ulCallId;
    VOS_UINT8                           ucBeginId;
    VOS_UINT8                           ucEndId;

    if (AT_PS_WAN_TYPE_APP == enPsCallType)
    {
        ucBeginId   = AT_PS_APP_CALL_ID_BEGIN;
        ucEndId     = AT_PS_APP_CALL_ID_END;
    }
    else
    {
        ucBeginId   = AT_PS_NDIS_CALL_ID_BEGIN;
        ucEndId     = AT_PS_NDIS_CALL_ID_END;
    }

    for (ulCallId = ucBeginId; ulCallId <= ucEndId; ulCallId++)
    {
        pstCallEntity = AT_PS_GetCallEntity(usClientId, (VOS_UINT8)ulCallId);

        AT_NORM_LOG2("AT_PS_AllocCallEntity: usClientId and ucCallId", (VOS_INT32)usClientId, ulCallId);

        if (VOS_FALSE == pstCallEntity->ulUsedFlg)
        {
            break;
        }
    }

    if (ulCallId > ucEndId)
    {
        for (ulCallId = ucBeginId; ulCallId <= ucEndId; ulCallId++)
        {
            pstCallEntity = AT_PS_GetCallEntity(usClientId, (VOS_UINT8)ulCallId);

            TAF_MEM_SET_S(&stMntnPsCallEntity, sizeof(stMntnPsCallEntity), 0x00, sizeof(AT_MNTN_PS_CALL_ENTITY_STRU));

            /* 填写消息头 */
            AT_MNTN_CFG_MSG_HDR(&stMntnPsCallEntity, ID_AT_MNTN_PS_CALL_ENTITY_RPT, (sizeof(AT_MNTN_PS_CALL_ENTITY_STRU) - VOS_MSG_HEAD_LENGTH));

            /* 填写消息内容 */
            stMntnPsCallEntity.ulUsedFlg                = pstCallEntity->ulUsedFlg;
            stMntnPsCallEntity.ucCurrPdpType            = pstCallEntity->stDialPdpType.enCurrPdpType;
            stMntnPsCallEntity.ucPortIndex              = pstCallEntity->stUserInfo.enPortIndex;
            stMntnPsCallEntity.ucUserIndex              = pstCallEntity->stUserInfo.enUserIndex;
            stMntnPsCallEntity.ucUsrType                = pstCallEntity->stUserInfo.ucUsrType;
            stMntnPsCallEntity.ucUsrCid                 = pstCallEntity->stUserInfo.ucUsrCid;
            stMntnPsCallEntity.ucUserPdpType            = pstCallEntity->stUsrDialParam.enPdpType;
            stMntnPsCallEntity.ucPdpTypeValidFlag       = pstCallEntity->stUsrDialParam.ucPdpTypeValidFlag;
            stMntnPsCallEntity.ucAPNLen                 = pstCallEntity->stUsrDialParam.ucAPNLen;
            stMntnPsCallEntity.usUsernameLen            = pstCallEntity->stUsrDialParam.usUsernameLen;
            stMntnPsCallEntity.usPasswordLen            = pstCallEntity->stUsrDialParam.usPasswordLen;
            stMntnPsCallEntity.usAuthType               = pstCallEntity->stUsrDialParam.usAuthType;
            stMntnPsCallEntity.ulIPv4ValidFlag          = pstCallEntity->stUsrDialParam.ulIPv4ValidFlag;
            stMntnPsCallEntity.ucIpv4Cid                = pstCallEntity->stIpv4Info.ucIpv4Cid;
            stMntnPsCallEntity.ucIpv4State              = pstCallEntity->stIpv4Info.enIpv4State;
            stMntnPsCallEntity.ucIpv4DendRptFlg         = pstCallEntity->stIpv4Info.ucIpv4DendRptFlg;
            stMntnPsCallEntity.ucIpv6Cid                = pstCallEntity->stIpv6Info.ucIpv6Cid;
            stMntnPsCallEntity.ucIpv6State              = pstCallEntity->stIpv6Info.enIpv6State;
            stMntnPsCallEntity.ucIpv6DendRptFlg         = pstCallEntity->stIpv6Info.ucIpv6DendRptFlg;
            /* 发送消息 */
            AT_MNTN_TraceEvent(&stMntnPsCallEntity);
        }


        return VOS_ERR;
    }

    /* 加上非空的判断 */
    if (VOS_NULL_PTR != pstCallEntity)
    {
        pstCallEntity->ulUsedFlg    = VOS_TRUE;
        pstCallEntity->enPsCallType = enPsCallType;
    }

    *pucCallId = (VOS_UINT8)ulCallId;

    return VOS_OK;
}


VOS_VOID AT_PS_FreeCallEntity(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    TAF_PS_APN_DATA_SYS_ENUM_UINT8      enDataSys
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);

    AT_LOG1("AT_PS_FreeCallEntity_normal: usClientId.", (VOS_INT32)usClientId);
    AT_LOG1("AT_PS_FreeCallEntity_normal: ucCallId.", (VOS_INT32)ucCallId);

    if (enDataSys == AT_PS_GetPsCallCurrentDataSys(pstCallEntity))
    {
        /* 去注册数据系统域改变通知,一定要放在清理其他信息之前 */
        AT_PS_DeRegDataSysChgNtf(usClientId, ucCallId);
    }

    if (VOS_FALSE == AT_PS_IsNeedClearCurrCall(pstCallEntity, enDataSys))
    {
        AT_NORM_LOG("AT_PS_FreeCallEntity: not need FreeCallEntity!");
        return;
    }

    /* PS呼叫实体里面的所有定时器需要停止 */
    AT_PS_StopAllPsCallTimer(pstCallEntity);

    /* 清除CID与通道的关系 */
    AT_CleanAtChdataCfg(usClientId, pstCallEntity->stUserInfo.ucUsrCid, ucCallId, enDataSys);

    TAF_MEM_SET_S(&pstCallEntity->stUserInfo,
                  (VOS_UINT32)(sizeof(pstCallEntity->stUserInfo)),
                  0x00,
                  (VOS_UINT32)(sizeof(AT_PS_USER_INFO_STRU)));

    pstCallEntity->ulUsedFlg                    = VOS_FALSE;
    pstCallEntity->ucRmNetId                    = RNIC_DEV_ID_BUTT;
    pstCallEntity->ucIfaceId                    = PS_IFACE_ID_BUTT;
    pstCallEntity->enPsCallType                 = AT_PS_WAN_TYPE_BUTT;
    pstCallEntity->stIpv4Info.ucIpv4Cid         = AT_PS_CALL_INVALID_CID;
    pstCallEntity->stIpv4Info.enIpv4State       = AT_PDP_STATE_IDLE;
    pstCallEntity->stDialPdpType.enWlanPdpType  = TAF_PDP_TYPE_BUTT;
    pstCallEntity->stDialPdpType.enHoPdpType    = TAF_PDP_TYPE_BUTT;
    pstCallEntity->stIpv4Info.enWlanIpv4State   = AT_PDP_STATE_IDLE;
    pstCallEntity->stIpv4Info.ucIpv4DendRptFlg  = VOS_FALSE;
    pstCallEntity->stIpv4Info.usIpv4Mtu         = 0;

    TAF_MEM_SET_S(&pstCallEntity->stIpv4Info.stIpv4DhcpInfo,
                  (VOS_UINT32)(sizeof(pstCallEntity->stIpv4Info.stIpv4DhcpInfo)),
                  0x00,
                  (VOS_UINT32)(sizeof(AT_IPV4_DHCP_PARAM_STRU)));

    pstCallEntity->stIpv6Info.ucIpv6Cid         = AT_PS_CALL_INVALID_CID;
    pstCallEntity->stIpv6Info.enIpv6State       = AT_PDP_STATE_IDLE;
    pstCallEntity->stIpv6Info.enWlanIpv6State   = AT_PDP_STATE_IDLE;
    pstCallEntity->stIpv6Info.ucIpv6DendRptFlg  = VOS_FALSE;

    TAF_MEM_SET_S(&pstCallEntity->stIpv6Info.stIpv6RaInfo,
                  (VOS_UINT32)(sizeof(pstCallEntity->stIpv6Info.stIpv6RaInfo)),
                  0x00,
                  (VOS_UINT32)(sizeof(AT_IPV6_RA_INFO_STRU)));

    TAF_MEM_SET_S(&pstCallEntity->stIpv6Info.stIpv6DhcpInfo,
                  (VOS_UINT32)(sizeof(pstCallEntity->stIpv6Info.stIpv6DhcpInfo)),
                  0x00,
                  (VOS_UINT32)(sizeof(AT_IPV6_DHCP_PARAM_STRU)));

    TAF_MEM_SET_S(&pstCallEntity->stApnDataSysInfo,
                  (VOS_UINT32)(sizeof(pstCallEntity->stApnDataSysInfo)),
                  0x00,
                  (VOS_UINT32)(sizeof(AT_PS_APN_DATA_SYS_INFO_STRU)));

    TAF_MEM_SET_S(&pstCallEntity->stPsCallTimerInfo,
                  (VOS_UINT32)(sizeof(pstCallEntity->stPsCallTimerInfo)),
                  0x00,
                  (VOS_UINT32)(sizeof(AT_PS_CALL_TIMER_INFO_STRU)));

    return;
}


VOS_UINT32 AT_PS_IsWlanLinkGoingUp(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
)
{
    /* 呼叫实体索引(CallId)由调用者保证其有效性 */

    /* (1) 任意呼叫状态处于已建立/正在建立: 返回TRUE
     * (2) 其他场景: 返回FALSE
     */
    VOS_UINT32                          ulLinkUpFlg;
    ulLinkUpFlg   = VOS_FALSE;

    if ((AT_PDP_STATE_ACTED == pstCallEntity->stIpv4Info.enWlanIpv4State)
     || (AT_PDP_STATE_ACTING == pstCallEntity->stIpv4Info.enWlanIpv4State))
    {
        ulLinkUpFlg = VOS_TRUE;
    }

    if ((AT_PDP_STATE_ACTED == pstCallEntity->stIpv6Info.enWlanIpv6State)
     || (AT_PDP_STATE_ACTING == pstCallEntity->stIpv6Info.enWlanIpv6State))
    {
        ulLinkUpFlg = VOS_TRUE;
    }

    return ulLinkUpFlg;
}


VOS_UINT32 AT_PS_IsLinkGoingUp(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
)
{
    /* 呼叫实体索引(CallId)由调用者保证其有效性 */

    /* (1) 任意呼叫状态处于已建立/正在建立: 返回TRUE
     * (2) 其他场景: 返回FALSE
     */
    VOS_UINT32                          ulLinkUpFlg;

    ulLinkUpFlg     = VOS_FALSE;

    if ((AT_PDP_STATE_ACTED  == pstCallEntity->stIpv4Info.enIpv4State)
     || (AT_PDP_STATE_ACTING == pstCallEntity->stIpv4Info.enIpv4State))
    {
        ulLinkUpFlg = VOS_TRUE;
    }

    if ((AT_PDP_STATE_ACTED  == pstCallEntity->stIpv6Info.enIpv6State)
     || (AT_PDP_STATE_ACTING == pstCallEntity->stIpv6Info.enIpv6State))
    {
        ulLinkUpFlg = VOS_TRUE;
    }

    return ulLinkUpFlg;
}


VOS_UINT32 AT_PS_IsWlanLinkGoingDown(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
)
{
    /* 呼叫实体索引(CallId)由调用者保证其有效性 */

    /* (1) 任意拨号连接处于DEACTING, 返回TRUE
     * (2) 其他场景, 返回FALSE
     */

    VOS_UINT32                          ulLinkDownFlg;

    ulLinkDownFlg = VOS_FALSE;

    if (AT_PDP_STATE_DEACTING == pstCallEntity->stIpv4Info.enWlanIpv4State)
    {
        ulLinkDownFlg = VOS_TRUE;
    }

    if (AT_PDP_STATE_DEACTING == pstCallEntity->stIpv6Info.enWlanIpv6State)
    {
        ulLinkDownFlg = VOS_TRUE;
    }

    return ulLinkDownFlg;
}


VOS_UINT32 AT_PS_IsLinkGoingDown(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
)
{
    /* 呼叫实体索引(CallId)由调用者保证其有效性 */

    /* (1) 任意拨号连接处于DEACTING, 返回TRUE
     * (2) 其他场景, 返回FALSE
     */
    VOS_UINT32                          ulLinkDownFlg;

    ulLinkDownFlg = VOS_FALSE;

    if (AT_PDP_STATE_DEACTING == pstCallEntity->stIpv4Info.enIpv4State)
    {
        ulLinkDownFlg = VOS_TRUE;
    }

    if (AT_PDP_STATE_DEACTING == pstCallEntity->stIpv6Info.enIpv6State)
    {
        ulLinkDownFlg = VOS_TRUE;
    }

    return ulLinkDownFlg;
}


VOS_UINT32 AT_PS_IsLinkDown(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
)
{
    /* 呼叫实体索引(CallId)由调用者保证其有效性 */

    /* (1) IPv4和IPv6连接都处于IDLE, 返回TRUE
     * (2) 其他场景, 返回FALSE
     */
    VOS_UINT32                          ulLinkDownFlg;

    ulLinkDownFlg = VOS_TRUE;

    if (AT_PDP_STATE_IDLE != pstCallEntity->stIpv4Info.enIpv4State)
    {
        ulLinkDownFlg = VOS_FALSE;
    }

    if (AT_PDP_STATE_IDLE != pstCallEntity->stIpv6Info.enIpv6State)
    {
        ulLinkDownFlg = VOS_FALSE;
    }

    return ulLinkDownFlg;
}


VOS_VOID AT_PS_ReportCurrCallConnState(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
)
{
    /* 呼叫实体索引(CallId)由调用者保证其有效性 */

    /* (1) 根据呼叫实体中的用户类型查表, 获取连接建立状态上报函数指针
     * (2) 获取呼叫连接状态, 处于连接状态则上报
     */

    AT_PS_RPT_CONN_RSLT_FUNC            pRptConnRsltFunc = VOS_NULL_PTR;

    pRptConnRsltFunc = AT_PS_GetRptConnResultFunc(pstCallEntity->stUserInfo.ucUsrType);

    if (VOS_NULL_PTR != pRptConnRsltFunc)
    {
        if ( (AT_PDP_STATE_ACTED == pstCallEntity->stIpv4Info.enIpv4State)
          || (AT_PDP_STATE_ACTED == pstCallEntity->stIpv4Info.enWlanIpv4State) )
        {
            pRptConnRsltFunc(&(pstCallEntity->stUserInfo), TAF_PDP_IPV4);
        }

        if ( ( (AT_PDP_STATE_ACTED == pstCallEntity->stIpv6Info.enIpv6State)
            && (VOS_TRUE == pstCallEntity->stIpv6Info.stIpv6RaInfo.bitOpPrefixAddr))
          || (AT_PDP_STATE_ACTED == pstCallEntity->stIpv6Info.enWlanIpv6State) )
        {
            pRptConnRsltFunc(&(pstCallEntity->stUserInfo), TAF_PDP_IPV6);
        }
    }
    else
    {
        AT_WARN_LOG("AT_PS_ReportCurrCallConnState: pRptConnRsltFunc is NULL.");
    }

    return;
}


VOS_VOID AT_PS_ReportCurrCallEndState(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
)
{
    /* 呼叫实体索引(CallId)由调用者保证其有效性 */

    /* (1) 根据呼叫实体中的用户类型查表, 获取连接建立状态上报函数指针
     * (2) 获取呼叫连接状态, 处于断开状态则上报
     */

    AT_PS_RPT_END_RSLT_FUNC             pRptEndRsltFunc = VOS_NULL_PTR;

    pRptEndRsltFunc = AT_PS_GetRptEndResultFunc(pstCallEntity->stUserInfo.ucUsrType);

    if (VOS_NULL_PTR != pRptEndRsltFunc)
    {
        if ( (AT_PDP_STATE_IDLE == pstCallEntity->stIpv4Info.enIpv4State)
          && (AT_PDP_STATE_IDLE == pstCallEntity->stIpv4Info.enWlanIpv4State))
        {
            pRptEndRsltFunc(&(pstCallEntity->stUserInfo),
                            TAF_PDP_IPV4,
                            TAF_PS_CAUSE_SUCCESS);
        }

        if (VOS_TRUE == AT_PS_IsIpv6Support())
        {
            if ( (AT_PDP_STATE_IDLE == pstCallEntity->stIpv6Info.enIpv6State)
              && (AT_PDP_STATE_IDLE == pstCallEntity->stIpv6Info.enWlanIpv6State))
            {
                pRptEndRsltFunc(&(pstCallEntity->stUserInfo),
                                TAF_PDP_IPV6,
                                TAF_PS_CAUSE_SUCCESS);
            }
        }
    }
    else
    {
        AT_WARN_LOG("AT_PS_ReportCurrCallEndState: pRptConnRsltFunc is NULL.");
    }

    return;
}


VOS_VOID AT_PS_ReportAllCallEndState(
    VOS_UINT8                           ucIndex,
    AT_PS_CALL_TYPE_ENUM_U8             enPsCallType
)
{
    /* (1) 根据用户类型查表, 获取连接断开状态上报函数指针
     * (2) 上报所有连接断开状态
     */

    AT_PS_RPT_END_RSLT_FUNC             pRptEndRsltFunc = VOS_NULL_PTR;
    AT_PS_USER_INFO_STRU                stUsrInfo;

    TAF_MEM_SET_S(&stUsrInfo, sizeof(stUsrInfo), 0x00, sizeof(AT_PS_USER_INFO_STRU));

    AT_PS_ParseUsrInfo(ucIndex, enPsCallType, &stUsrInfo);

    pRptEndRsltFunc = AT_PS_GetRptEndResultFunc(stUsrInfo.ucUsrType);

    if (VOS_NULL_PTR != pRptEndRsltFunc)
    {
        pRptEndRsltFunc(&stUsrInfo,
                        TAF_PDP_IPV4,
                        TAF_PS_CAUSE_SUCCESS);

        if (VOS_TRUE == AT_PS_IsIpv6Support())
        {
            pRptEndRsltFunc(&stUsrInfo,
                            TAF_PDP_IPV6,
                            TAF_PS_CAUSE_SUCCESS);
        }
    }
    else
    {
        AT_WARN_LOG("AT_PS_ReportAllCallEndState: pRptConnRsltFunc is NULL.");
    }

    return;
}


AT_PS_CALL_TYPE_ENUM_U8 AT_PS_GetPsCallType(VOS_UINT8 ucIndex)
{
    AT_PS_CALL_TYPE_ENUM_U8             enPsCallType;
    VOS_UINT8                          *pucSystemAppConfig;

    enPsCallType        = AT_PS_WAN_TYPE_BUTT;
    pucSystemAppConfig  = AT_GetSystemAppConfigAddr();

    switch(gastAtClientTab[ucIndex].UserType)
    {
        case AT_NDIS_USER:
            enPsCallType = AT_PS_WAN_TYPE_NDIS;
            break;

        case AT_APP_USER:
            enPsCallType = AT_PS_WAN_TYPE_APP;
            break;

        case AT_USBCOM_USER:
            if ( (VOS_TRUE == AT_GetPcuiPsCallFlag())
              || (SYSTEM_APP_WEBUI == *pucSystemAppConfig))
            {
                enPsCallType = AT_PS_WAN_TYPE_APP;
                break;
            }

            enPsCallType = AT_PS_WAN_TYPE_NDIS;
            break;

        case AT_CTR_USER:
            if (VOS_TRUE == AT_GetCtrlPsCallFlag())
            {
                enPsCallType = AT_PS_WAN_TYPE_APP;
                break;
            }

            if (SYSTEM_APP_WEBUI != *pucSystemAppConfig)
            {
                enPsCallType = AT_PS_WAN_TYPE_NDIS;
                break;
            }
            break;

        case AT_PCUI2_USER:
            if (VOS_TRUE == AT_GetPcui2PsCallFlag())
            {
                enPsCallType = AT_PS_WAN_TYPE_APP;
                break;
            }

            if (SYSTEM_APP_WEBUI != *pucSystemAppConfig)
            {
                enPsCallType = AT_PS_WAN_TYPE_NDIS;
                break;
            }
            break;

        default:
            AT_WARN_LOG("AT_PS_GetPsCallType: UserType Is Invalid!");
            enPsCallType = AT_PS_WAN_TYPE_BUTT;
            break;
    }

    return enPsCallType;
}


VOS_UINT32 AT_PS_CheckDialParamCnt(
    AT_PS_CALL_TYPE_ENUM_U8             enPsCallType
)
{
    VOS_UINT8                       *pucSystemAppConfig   = VOS_NULL_PTR;

    pucSystemAppConfig  = AT_GetSystemAppConfigAddr();

    /* 检查参数个数 */
    if (AT_PS_WAN_TYPE_APP == enPsCallType)
    {
        if (SYSTEM_APP_WEBUI != *pucSystemAppConfig)
        {
            if (gucAtParaIndex > 8)
            {
                AT_NORM_LOG1("AT_PS_CheckDialParamCnt: Phone Dial Parameter number is .\n", gucAtParaIndex);
                return AT_TOO_MANY_PARA;
            }
        }
        else
        {
            if (gucAtParaIndex > 7)
            {
                AT_NORM_LOG1("AT_PS_CheckDialParamCnt: WEBUI APP Dial Parameter number is .\n", gucAtParaIndex);
                return AT_TOO_MANY_PARA;
            }
        }

        return AT_SUCCESS;
    }

    if (AT_PS_WAN_TYPE_NDIS == enPsCallType)
    {
        /* 按照MBB产品线要求扩展到7个 */
        if (gucAtParaIndex > 7)
        {
            AT_NORM_LOG1("AT_PS_CheckDialParamCnt: NDIS Dial Parameter number is .\n", gucAtParaIndex);
            return AT_TOO_MANY_PARA;
        }
        else
        {
            return AT_SUCCESS;
        }
    }

    AT_WARN_LOG("AT_PS_ValidateDialParam: Ps Call Type is invalid.\n");

    return AT_ERROR;
}


VOS_UINT32 AT_PS_CheckDialParamApn(VOS_VOID)
{
    /* 检查 APN */
    if (0 != gastAtParaList[2].usParaLen)
    {
        /* APN长度检查 */
        if (gastAtParaList[2].usParaLen > TAF_MAX_APN_LEN)
        {
            AT_NORM_LOG("AT_PS_CheckDialParamApn: APN is too long.");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* APN格式检查 */
        if (VOS_OK != AT_CheckApnFormat(gastAtParaList[2].aucPara,
                                        gastAtParaList[2].usParaLen))
        {
            AT_NORM_LOG("AT_PS_CheckDialParamApn: Format of APN is wrong.");
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    return AT_SUCCESS;
}


VOS_UINT32 AT_PS_CheckDialParamIpAddr(
    AT_PS_CALL_TYPE_ENUM_U8             enPsCallType
)
{
    VOS_UINT8                           aucIpv4Addr[TAF_IPV4_ADDR_LEN];

    TAF_MEM_SET_S(aucIpv4Addr, sizeof(aucIpv4Addr), 0x00, TAF_IPV4_ADDR_LEN);

    /* ip addr检查 */
    if (gastAtParaList[6].usParaLen > (TAF_MAX_IPV4_ADDR_STR_LEN - 1))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (gastAtParaList[6].usParaLen > 0)
    {
        if (VOS_OK != AT_Ipv4AddrAtoi((VOS_CHAR *)gastAtParaList[6].aucPara, aucIpv4Addr))
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    return AT_SUCCESS;
}


VOS_UINT32 AT_PS_CheckDialParamDataChanl(
    VOS_UINT8                           ucIndex,
    AT_PS_CALL_TYPE_ENUM_U8             enPsCallType
)
{
    AT_PS_DATA_CHANL_CFG_STRU          *pstChanCfg          = VOS_NULL_PTR;
    VOS_UINT8                          *pucSystemAppConfig  = VOS_NULL_PTR;

    pucSystemAppConfig  = AT_GetSystemAppConfigAddr();
    pstChanCfg          = AT_PS_GetDataChanlCfg(ucIndex, (VOS_UINT8)gastAtParaList[0].ulParaValue);

    if ( (AT_PS_WAN_TYPE_APP == enPsCallType)
      && (SYSTEM_APP_WEBUI != *pucSystemAppConfig))
    {
        /* 检查通道映射 */
        if ( (VOS_FALSE == pstChanCfg->ulUsed)
          || (AT_PS_INVALID_RMNET_ID == pstChanCfg->ulRmNetId) )
        {
            AT_NORM_LOG2("AT_PS_CheckDialParamDataChanl: Used is .RmNetId is .\n", pstChanCfg->ulUsed, pstChanCfg->ulRmNetId);
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    return AT_SUCCESS;
}


VOS_UINT32 AT_PS_ValidateDialParam(
    VOS_UINT8                           ucIndex,
    AT_PS_CALL_TYPE_ENUM_U8             enPsCallType
)
{
    VOS_UINT32                          ulRst;

    /* 检查命令类型 */
    if (AT_CMD_OPT_SET_CMD_NO_PARA == g_stATParseCmd.ucCmdOptType)
    {
        AT_NORM_LOG("AT_PS_ValidateDialParam: No parameter input.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 检查参数个数 */
    ulRst = AT_PS_CheckDialParamCnt(enPsCallType);
    if (AT_SUCCESS != ulRst)
    {
        return ulRst;
    }

    /* 检查 CID */
    if (0 == gastAtParaList[0].usParaLen)
    {
        AT_NORM_LOG("AT_PS_ValidateDialParam: Missing CID.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 拨号 CONN: 该参数不能省略, 1表示建立连接, 0表示断开断开连接 */
    if (0 == gastAtParaList[1].usParaLen)
    {
        AT_NORM_LOG("AT_PS_ValidateDialParam: Missing connect state.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 检查 APN */
    ulRst = AT_PS_CheckDialParamApn();
    if (AT_SUCCESS != ulRst)
    {
        return ulRst;
    }

    /* 检查 Username */
    if (gastAtParaList[3].usParaLen > TAF_MAX_AUTHDATA_USERNAME_LEN)
    {
        AT_NORM_LOG1("AT_PS_ValidateDialParam: Username length is.\n", gastAtParaList[3].usParaLen);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 检查 Password */
    if (gastAtParaList[4].usParaLen > TAF_MAX_AUTHDATA_PASSWORD_LEN)
    {
        AT_NORM_LOG1("AT_PS_ValidateDialParam: Password length is.\n", gastAtParaList[4].usParaLen);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ip addr检查 */
    ulRst = AT_PS_CheckDialParamIpAddr(enPsCallType);
    if (AT_SUCCESS != ulRst)
    {
        AT_NORM_LOG("AT_PS_ValidateDialParam: ip addr is invalid.");
        return ulRst;
    }

    if (gastAtParaList[7].usParaLen > 0)
    {
        if (VOS_TRUE != AT_PS_CheckDialRatType(ucIndex, (VOS_UINT8)gastAtParaList[7].ulParaValue))
        {
            AT_NORM_LOG1("AT_PS_ValidateDialParam: DialRatType is.\n", gastAtParaList[7].ulParaValue);
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    /* 检查通道映射 */
    ulRst = AT_PS_CheckDialParamDataChanl(ucIndex, enPsCallType);
    if (AT_SUCCESS != ulRst)
    {
        return ulRst;
    }

    return AT_SUCCESS;
}


VOS_VOID AT_PS_ParseUsrInfo(
    VOS_UINT8                           ucIndex,
    AT_PS_CALL_TYPE_ENUM_U8             enPsCallType,
    AT_PS_USER_INFO_STRU               *pstUserInfo
)
{
    VOS_UINT8                          *pucSystemAppConfig   = VOS_NULL_PTR;
    AT_CLIENT_TAB_INDEX_UINT8           enUserIndex;

    enUserIndex         = ucIndex;
    pucSystemAppConfig  = AT_GetSystemAppConfigAddr();

    switch(enPsCallType)
    {
        case AT_PS_WAN_TYPE_APP:
            if (SYSTEM_APP_WEBUI != *pucSystemAppConfig)
            {
                /* PCUI口且设置PCUI口模拟NDISDUP拨号 */
                if ( (AT_USBCOM_USER == gastAtClientTab[ucIndex].UserType)
                  && (VOS_TRUE == AT_GetPcuiPsCallFlag()))
                {
                    enUserIndex = AT_GetPcuiUsertId();
                }

                /* CTRL口且设置CTRL口模拟NDISDUP拨号 */
                if ( (AT_CTR_USER == gastAtClientTab[ucIndex].UserType)
                  && (VOS_TRUE == AT_GetCtrlPsCallFlag()))
                {
                    enUserIndex = AT_GetCtrlUserId();
                }

                /* PCUI2口且设置CTRL口模拟NDISDUP拨号 */
                if ( (AT_PCUI2_USER == gastAtClientTab[ucIndex].UserType)
                  && (VOS_TRUE == AT_GetPcui2PsCallFlag()))
                {
                    enUserIndex = AT_GetPcui2UserId();
                }

                break;
            }

            if (VOS_TRUE != AT_CheckAppUser(ucIndex))
            {
                enUserIndex = AT_CLIENT_TAB_APP_INDEX;
            }

            break;

        case AT_PS_WAN_TYPE_NDIS:
            if (VOS_TRUE != AT_CheckNdisUser(ucIndex))
            {
                enUserIndex = AT_CLIENT_TAB_NDIS_INDEX;
            }

            break;

        default:
            AT_WARN_LOG("AT_PS_ParseUsrInfo: PS CALL TYPE is Invalid.");
            break;
    }

    pstUserInfo->enPortIndex = ucIndex;
    pstUserInfo->enUserIndex = enUserIndex;
    pstUserInfo->ucUsrType   = (AT_USER_TYPE)gastAtClientTab[enUserIndex].UserType;
    pstUserInfo->ucUsrCid    = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    return;
}


VOS_VOID AT_PS_ParseUsrDialAuthType(
    VOS_UINT8                           ucIndex,
    AT_PS_CALL_TYPE_ENUM_U8             enPsCallType,
    AT_DIAL_PARAM_STRU                 *pstUsrDialParam
)
{
    VOS_UINT8                       *pucSystemAppConfig   = VOS_NULL_PTR;

    pucSystemAppConfig  = AT_GetSystemAppConfigAddr();

    /* AUTH TYPE */
    if ( (VOS_TRUE == At_CheckCurrRatModeIsCL(ucIndex))
      && (SYSTEM_APP_WEBUI != *pucSystemAppConfig)
      && (AT_PS_WAN_TYPE_APP == enPsCallType))
    {
        pstUsrDialParam->usAuthType = AT_ClGetPdpAuthType(gastAtParaList[5].ulParaValue,
                                                               gastAtParaList[5].usParaLen);
    }
    else
    {
        if (gastAtParaList[5].usParaLen > 0)
        {
            pstUsrDialParam->usAuthType = AT_CtrlGetPDPAuthType(gastAtParaList[5].ulParaValue,
                                                                    gastAtParaList[5].usParaLen);
        }
        else
        {
            pstUsrDialParam->usAuthType = TAF_PDP_AUTH_TYPE_NONE;

            /* 如果用户名和密码长度均不为0, 且鉴权类型未设置, 则默认使用CHAP类型 */
            if ( (0 != gastAtParaList[3].usParaLen)
              && (0 != gastAtParaList[4].usParaLen) )
            {
                pstUsrDialParam->usAuthType = TAF_PDP_AUTH_TYPE_CHAP;
            }
        }
    }

    return;
}


VOS_UINT32 AT_PS_ParseUsrDialParam(
    VOS_UINT8                           ucIndex,
    AT_PS_CALL_TYPE_ENUM_U8             enPsCallType,
    AT_DIAL_PARAM_STRU                 *pstUsrDialParam
)
{
    /* 由调用者保证入参和出参有效性 */
    VOS_UINT8                          *pucSystemAppConfig  = VOS_NULL_PTR;
    TAF_PDP_PRIM_CONTEXT_STRU           stPdpCtxInfo;
    VOS_UINT32                          ulRslt;

    TAF_MEM_SET_S(&stPdpCtxInfo, sizeof(stPdpCtxInfo), 0x00, sizeof(TAF_PDP_PRIM_CONTEXT_STRU));
    pucSystemAppConfig  = AT_GetSystemAppConfigAddr();

    /* CID */
    pstUsrDialParam->ucCid         = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    /* APN */
    pstUsrDialParam->ucAPNLen      = (VOS_UINT8)gastAtParaList[2].usParaLen;
    TAF_MEM_CPY_S(pstUsrDialParam->aucAPN,
               sizeof(pstUsrDialParam->aucAPN),
               gastAtParaList[2].aucPara,
               gastAtParaList[2].usParaLen);

    /* Username */
    pstUsrDialParam->usUsernameLen = (VOS_UINT16)gastAtParaList[3].usParaLen;
    TAF_MEM_CPY_S(pstUsrDialParam->aucUsername,
               sizeof(pstUsrDialParam->aucUsername),
               gastAtParaList[3].aucPara,
               gastAtParaList[3].usParaLen);

    /* Password */
    pstUsrDialParam->usPasswordLen = (VOS_UINT16)gastAtParaList[4].usParaLen;
    TAF_MEM_CPY_S(pstUsrDialParam->aucPassword,
               sizeof(pstUsrDialParam->aucPassword),
               gastAtParaList[4].aucPara,
               gastAtParaList[4].usParaLen);

    /* AUTH TYPE */
    AT_PS_ParseUsrDialAuthType(ucIndex, enPsCallType, pstUsrDialParam);

    /* ADDR */
    /* 只有E5拨号和NDIS拨号才需要检查 */
    if ( ( ( (AT_PS_WAN_TYPE_APP == enPsCallType) && (SYSTEM_APP_WEBUI == *pucSystemAppConfig))
        || (AT_PS_WAN_TYPE_NDIS == enPsCallType))
      && (gastAtParaList[6].usParaLen > 0))
    {
        pstUsrDialParam->ulIPv4ValidFlag = VOS_TRUE;
        TAF_MEM_CPY_S((VOS_CHAR *)pstUsrDialParam->aucIPv4Addr,
                   sizeof(pstUsrDialParam->aucIPv4Addr),
                   (VOS_CHAR *)gastAtParaList[6].aucPara,
                   gastAtParaList[6].usParaLen);
    }

    /* PDN TYPE */
    ulRslt = TAF_AGENT_GetPdpCidPara(&stPdpCtxInfo, ucIndex, pstUsrDialParam->ucCid);

    if ( (VOS_OK == ulRslt)
      && (AT_PS_IS_PDP_TYPE_SUPPORT(stPdpCtxInfo.stPdpAddr.enPdpType)) )
    {
        pstUsrDialParam->enPdpType = stPdpCtxInfo.stPdpAddr.enPdpType;
    }
    else
    {
        pstUsrDialParam->enPdpType = TAF_PDP_IPV4;
    }


    if (gastAtParaList[7].usParaLen > 0)
    {
        pstUsrDialParam->ucBitRatType  = (VOS_UINT8)gastAtParaList[7].ulParaValue;
    }


    if (VOS_OK != AT_CheckIpv6Capability(pstUsrDialParam->enPdpType))
    {
        AT_INFO_LOG("AT_PS_ParseUsrDialParam: PDP type is not supported.");

        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_VOID AT_PS_SetCurrCallType(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);

    pstCallEntity->stDialPdpType.enCurrPdpType = enPdpType;

    return;
}


TAF_PDP_TYPE_ENUM_UINT8 AT_PS_GetCurrCallType(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);

    return pstCallEntity->stDialPdpType.enCurrPdpType;
}


VOS_VOID AT_PS_TransferQosPara(
    VOS_UINT8                           ucCid,
    TAF_PS_PDP_QOS_QUERY_PARA_STRU     *pstQosQueryPara,
    TAF_PS_PDP_QOS_SET_PARA_STRU       *pstQosSetPara
)
{
    if (VOS_TRUE == pstQosQueryPara->ucQosFlag)
    {
        pstQosSetPara->stQos.ucDefined = VOS_TRUE;
        pstQosSetPara->stQos.ucCid = ucCid;

        pstQosSetPara->stQos.bitOpTrafficClass = VOS_TRUE;
        pstQosSetPara->stQos.ucTrafficClass    = pstQosQueryPara->stQos.ucTrafficClass;

        pstQosSetPara->stQos.bitOpDeliverOrder = VOS_TRUE;
        pstQosSetPara->stQos.ucDeliverOrder    = pstQosQueryPara->stQos.ucDeliverOrder;

        pstQosSetPara->stQos.bitOpDeliverErrSdu = VOS_TRUE;
        pstQosSetPara->stQos.ucDeliverErrSdu    = pstQosQueryPara->stQos.ucDeliverErrSdu;

        pstQosSetPara->stQos.bitOpMaxSduSize = VOS_TRUE;
        pstQosSetPara->stQos.usMaxSduSize    = pstQosQueryPara->stQos.usMaxSduSize;

        pstQosSetPara->stQos.bitOpMaxBitUl = VOS_TRUE;
        pstQosSetPara->stQos.ulMaxBitUl    = pstQosQueryPara->stQos.ulMaxBitUl;

        pstQosSetPara->stQos.bitOpMaxBitDl = VOS_TRUE;
        pstQosSetPara->stQos.ulMaxBitDl    = pstQosQueryPara->stQos.ulMaxBitDl;

        pstQosSetPara->stQos.bitOpResidualBer = VOS_TRUE;
        pstQosSetPara->stQos.ucResidualBer    = pstQosQueryPara->stQos.ucResidualBer;

        pstQosSetPara->stQos.bitOpSduErrRatio = VOS_TRUE;
        pstQosSetPara->stQos.ucSduErrRatio    = pstQosQueryPara->stQos.ucSduErrRatio;

        pstQosSetPara->stQos.bitOpTransDelay = VOS_TRUE;
        pstQosSetPara->stQos.usTransDelay    = pstQosQueryPara->stQos.usTransDelay;

        pstQosSetPara->stQos.bitOpTraffHandlePrior = VOS_TRUE;
        pstQosSetPara->stQos.ucTraffHandlePrior    = pstQosQueryPara->stQos.ucTraffHandlePrior;

        pstQosSetPara->stQos.bitOpGtdBitUl = VOS_TRUE;
        pstQosSetPara->stQos.ulGtdBitUl    = pstQosQueryPara->stQos.ulGuarantBitUl;

        pstQosSetPara->stQos.bitOpGtdBitDl  = VOS_TRUE;
        pstQosSetPara->stQos.ulGtdBitDl     = pstQosQueryPara->stQos.ulGuarantBitDl;
    }
    else
    {
        pstQosSetPara->stQos.ucDefined = VOS_FALSE;
        pstQosSetPara->stQos.ucCid = ucCid;
    }

    if (VOS_TRUE == pstQosQueryPara->ucMinQosFlag)
    {
        pstQosSetPara->stMinQos.ucDefined = VOS_TRUE;
        pstQosSetPara->stMinQos.ucCid = ucCid;

        pstQosSetPara->stMinQos.bitOpTrafficClass = VOS_TRUE;
        pstQosSetPara->stMinQos.ucTrafficClass    = pstQosQueryPara->stMinQos.ucTrafficClass;

        pstQosSetPara->stMinQos.bitOpDeliverOrder = VOS_TRUE;
        pstQosSetPara->stMinQos.ucDeliverOrder    = pstQosQueryPara->stMinQos.ucDeliverOrder;

        pstQosSetPara->stMinQos.bitOpDeliverErrSdu = VOS_TRUE;
        pstQosSetPara->stMinQos.ucDeliverErrSdu    = pstQosQueryPara->stMinQos.ucDeliverErrSdu;

        pstQosSetPara->stMinQos.bitOpMaxSduSize = VOS_TRUE;
        pstQosSetPara->stMinQos.usMaxSduSize    = pstQosQueryPara->stMinQos.usMaxSduSize;

        pstQosSetPara->stMinQos.bitOpMaxBitUl = VOS_TRUE;
        pstQosSetPara->stMinQos.ulMaxBitUl    = pstQosQueryPara->stMinQos.ulMaxBitUl;

        pstQosSetPara->stMinQos.bitOpMaxBitDl = VOS_TRUE;
        pstQosSetPara->stMinQos.ulMaxBitDl    = pstQosQueryPara->stMinQos.ulMaxBitDl;

        pstQosSetPara->stMinQos.bitOpResidualBer = VOS_TRUE;
        pstQosSetPara->stMinQos.ucResidualBer    = pstQosQueryPara->stMinQos.ucResidualBer;

        pstQosSetPara->stMinQos.bitOpSduErrRatio = VOS_TRUE;
        pstQosSetPara->stMinQos.ucSduErrRatio    = pstQosQueryPara->stMinQos.ucSduErrRatio;

        pstQosSetPara->stMinQos.bitOpTransDelay = VOS_TRUE;
        pstQosSetPara->stMinQos.usTransDelay    = pstQosQueryPara->stMinQos.usTransDelay;

        pstQosSetPara->stMinQos.bitOpTraffHandlePrior = VOS_TRUE;
        pstQosSetPara->stMinQos.ucTraffHandlePrior    = pstQosQueryPara->stMinQos.ucTraffHandlePrior;

        pstQosSetPara->stMinQos.bitOpGtdBitUl = VOS_TRUE;
        pstQosSetPara->stMinQos.ulGtdBitUl    = pstQosQueryPara->stMinQos.ulGuarantBitUl;

        pstQosSetPara->stMinQos.bitOpGtdBitDl  = VOS_TRUE;
        pstQosSetPara->stMinQos.ulGtdBitDl     = pstQosQueryPara->stMinQos.ulGuarantBitDl;

    }
    else
    {
        pstQosSetPara->stMinQos.ucDefined = VOS_FALSE;
        pstQosSetPara->stMinQos.ucCid = ucCid;
    }

    return;
}


VOS_UINT32 AT_PS_SetQosPara(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucUserCid,
    VOS_UINT8                           ucActCid
)
{
    TAF_PS_PDP_QOS_QUERY_PARA_STRU      stQosQueryPara;
    TAF_PS_PDP_QOS_SET_PARA_STRU        stQosSetPara;

    TAF_MEM_SET_S(&stQosQueryPara, sizeof(stQosQueryPara), 0x00, sizeof(TAF_PS_PDP_QOS_QUERY_PARA_STRU));
    TAF_MEM_SET_S(&stQosSetPara, sizeof(stQosSetPara), 0x00, sizeof(TAF_PS_PDP_QOS_SET_PARA_STRU));

    /* 判断需要设置的CID是否用户下发的CID相同 */
    if (ucActCid == ucUserCid)
    {
        return VOS_OK;
    }

    /* 获取用户设置的QOS参数 */
    if (VOS_OK != TAF_AGENT_GetPdpCidQosPara(usClientId,
                                             ucUserCid,
                                             &stQosQueryPara))
    {
        AT_ERR_LOG("AT_PS_SetQosPara: ERROR: Get QOS Fail.");
        return VOS_ERR;
    }

    /* QOS参数转换 */
    AT_PS_TransferQosPara(ucActCid, &stQosQueryPara, &stQosSetPara);

    /* 获取用户设置的QOS参数 */
    if (VOS_OK != TAF_AGENT_SetPdpCidQosPara(usClientId,
                                             &stQosSetPara))
    {
        AT_ERR_LOG("AT_PS_SetQosPara: ERROR: Set QOS Fail.");
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_VOID AT_PS_GetPsDialParamFromAtDialParam(
    TAF_PS_DIAL_PARA_STRU              *pstPsDialParam,
    AT_DIAL_PARAM_STRU                 *pstDialParam
)
{
    TAF_MEM_SET_S((VOS_VOID *)pstPsDialParam, sizeof(TAF_PS_DIAL_PARA_STRU), 0x00, sizeof(TAF_PS_DIAL_PARA_STRU));

    pstPsDialParam->enPdpType       = pstDialParam->enPdpType;
    pstPsDialParam->ucCid           = pstDialParam->ucCid;

    /* 获取APN */
    if (0 != pstDialParam->ucAPNLen)
    {
        pstPsDialParam->bitOpApn        = VOS_TRUE;
        TAF_MEM_CPY_S((VOS_CHAR*)pstPsDialParam->aucApn,
                   sizeof(pstPsDialParam->aucApn),
                   (VOS_CHAR*)pstDialParam->aucAPN,
                   pstDialParam->ucAPNLen);
    }

    /* 填入验证信息 */
    pstPsDialParam->bitOpAuthType   = VOS_TRUE;
    pstPsDialParam->enAuthType      = (PPP_AUTH_TYPE_ENUM_UINT8)pstDialParam->usAuthType;

    if (0 != pstDialParam->usPasswordLen)
    {
        pstPsDialParam->bitOpPassWord = VOS_TRUE;
        TAF_MEM_CPY_S((VOS_VOID *)pstPsDialParam->aucPassWord,
                   sizeof(pstPsDialParam->aucPassWord),
                   (VOS_VOID *)pstDialParam->aucPassword,
                   pstDialParam->usPasswordLen);
    }
    else
    {
        pstPsDialParam->bitOpPassWord = VOS_FALSE;
    }

    if (0 != pstDialParam->usUsernameLen)
    {
        pstPsDialParam->bitOpUserName = VOS_TRUE;
        TAF_MEM_CPY_S((VOS_VOID *)pstPsDialParam->aucUserName,
                   sizeof(pstPsDialParam->aucUserName),
                   (VOS_VOID *)pstDialParam->aucUsername,
                   pstDialParam->usUsernameLen);
    }
    else
    {
        pstPsDialParam->bitOpUserName = VOS_FALSE;
    }

    /* 设置<PDP_addr> */
    if (VOS_TRUE == pstDialParam->ulIPv4ValidFlag)
    {
        /* 把IP地址字符串转换成数字, IP地址格式已经统一在前面检查 */
        if ( VOS_OK == AT_Ipv4AddrAtoi((VOS_CHAR*)pstDialParam->aucIPv4Addr,
                                       (VOS_UINT8*)pstPsDialParam->stPdpAddr.aucIpv4Addr) )
        {
            pstPsDialParam->bitOpIpAddr         = VOS_TRUE;
            pstPsDialParam->stPdpAddr.enPdpType = TAF_PDP_IPV4;
        }
    }

    pstPsDialParam->ucBitRatType   = pstDialParam->ucBitRatType;

}


VOS_UINT32 AT_PS_SetupCall(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    AT_DIAL_PARAM_STRU                 *pstCallDialParam
)
{
    /* 呼叫实体索引(CallId)由调用者保证其有效性 */

    /* (1) 设置CID上下文参数
     *     -> 失败: 返回ERROR
     *     -> 成功: 继续
     * (2) 发起PS域呼叫
     *     -> 失败: 返回ERROR
     *     -> 成功: 继续
     * (3) 添加CallId映射
     * (4) 设置呼叫PDP类型
     * (5) 设置呼叫CID
     * (6) 设置呼叫状态
     */
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;
    TAF_PS_DIAL_PARA_STRU               stPsDialParamInfo;
    VOS_UINT32                          ulRslt;

    TAF_MEM_SET_S(&stPsDialParamInfo, sizeof(stPsDialParamInfo),
                  0x00, sizeof(TAF_PS_DIAL_PARA_STRU));

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);

    /* 设置QOS参数 */
    if (VOS_OK != AT_PS_SetQosPara(usClientId, AT_PS_GetUserInfo(usClientId, ucCallId)->ucUsrCid, pstCallDialParam->ucCid))
    {
        return VOS_ERR;
    }

    /* 填写PS呼叫参数 */
    AT_PS_GetPsDialParamFromAtDialParam(&stPsDialParamInfo, pstCallDialParam);

    stPsDialParamInfo.ucRmnetId = pstCallEntity->ucIfaceId;

    if (VOS_TRUE == AT_PS_GetPsCallHandOverFlg(pstCallEntity))
    {
        stPsDialParamInfo.bitOpReqType      = VOS_TRUE;
        stPsDialParamInfo.enPdpRequestType  = TAF_PDP_REQUEST_TYPE_HANDOVER;

        if ( (TAF_PDP_IPV6 == stPsDialParamInfo.enPdpType)
          || (TAF_PDP_IPV4V6 == stPsDialParamInfo.enPdpType))
        {
            TAF_MEM_CPY_S(stPsDialParamInfo.stPdpAddr.aucIpv6Addr,
                          TAF_IPV6_ADDR_LEN,
                          pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6Addr,
                          TAF_IPV6_ADDR_LEN);
        }
    }

    /* 发起PS域呼叫 */
    ulRslt = TAF_PS_CallOrig(WUEPS_PID_AT,
                              AT_PS_BuildPsCallExClientId(usClientId, ucCallId),
                             0, &stPsDialParamInfo);

    if (VOS_OK != ulRslt)
    {
        AT_ERR_LOG("AT_PS_SetupCall: Setup call failed.");
        return VOS_ERR;
    }

    /* 添加CALLID映射 */
    AT_PS_AssignCallIdToCid(usClientId, stPsDialParamInfo.ucCid, ucCallId);

    /* 设置呼叫PDP类型 */
    AT_PS_SetCurrCallType(usClientId, ucCallId, stPsDialParamInfo.enPdpType);

    /* 设置呼叫CID */
    AT_PS_SetCid2CurrCall(usClientId, ucCallId, stPsDialParamInfo.enPdpType, stPsDialParamInfo.ucCid, TAF_PS_APN_DATA_SYS_CELLULAR);

    /* 设置呼叫状态 */
    AT_PS_SetCallStateByType(usClientId, ucCallId, stPsDialParamInfo.enPdpType, AT_PDP_STATE_ACTING);

    /* 设置当前域为CELLULAR */
    AT_PS_SetPsCallCurrentDataSys(pstCallEntity, TAF_PS_APN_DATA_SYS_CELLULAR);

    return VOS_OK;
}


VOS_UINT32 AT_PS_SetupWlanCall(
    AT_DIAL_PARAM_STRU                 *pstCallDialParam,
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;
    VOS_UINT32                          ulRslt;
    VOS_UINT32                          ulTmrName;
    VOS_UINT32                          ulTmrParam;

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);

    /* Wlan下发起PS域呼叫 */
    ulRslt = AT_PS_SndWlanMsgPdnActivateReq((VOS_UINT8)usClientId,
                                            pstCallDialParam->enPdpType,
                                            (VOS_INT8)AT_PS_GetPsCallHandOverFlg(pstCallEntity),
                                            ucCallId);

    if (VOS_OK != ulRslt)
    {
        AT_ERR_LOG("AT_PS_SetupWlanCall: Setup call in wlan failed.");
        return VOS_ERR;
    }

    AT_SET_WLAN_ACT_PDN_CNF_TMR_NAME(ulTmrName);
    AT_SET_WLAN_ACT_PDN_CNF_TMR_PARAM(ulTmrParam, usClientId, ucCallId);

    /* 启动定时器 */
    AT_StartRelTimer(&(pstCallEntity->stPsCallTimerInfo.hWaitWlanActCnfTmrHdl),
                     AT_WAIT_WLAN_ACT_PDN_CNF_TIMER_LEN,
                     ulTmrName,
                     ulTmrParam,
                     VOS_RELTIMER_NOLOOP);

    if (VOS_FALSE == AT_PS_GetPsCallHandOverFlg(pstCallEntity))
    {
        /* 添加CALLID映射 */
        AT_PS_AssignCallIdToCid(usClientId, pstCallDialParam->ucCid, ucCallId);

        /* 设置呼叫CID */
        AT_PS_SetCid2CurrCall(usClientId, ucCallId, pstCallDialParam->enPdpType, pstCallDialParam->ucCid, TAF_PS_APN_DATA_SYS_WLAN);
    }

    /* 设置Wlan呼叫PDP类型 */
    AT_PS_SetWlanCurrCallType(usClientId, ucCallId, pstCallDialParam->enPdpType);

    /* 设置Wlan呼叫状态 */
    AT_PS_SetWlanCallStateByType(usClientId, ucCallId, pstCallDialParam->enPdpType, AT_PDP_STATE_ACTING);

    /* 设置当前域为WLAN */
    AT_PS_SetPsCallCurrentDataSys(pstCallEntity, TAF_PS_APN_DATA_SYS_WLAN);

    return VOS_OK;
}


VOS_UINT32 AT_PS_HangupWlanCall(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    VOS_INT8                            cIsLocal,
    VOS_INT8                            cIsHandover
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;
    VOS_UINT32                          ulTmrName;
    VOS_UINT32                          ulTmrParam;

    pstCallEntity   = AT_PS_GetCallEntity(usClientId, ucCallId);

    if ( (AT_PDP_STATE_ACTED == pstCallEntity->stIpv4Info.enWlanIpv4State)
      || (AT_PDP_STATE_ACTING == pstCallEntity->stIpv4Info.enWlanIpv4State)
      || (AT_PDP_STATE_ACTED == pstCallEntity->stIpv6Info.enWlanIpv6State)
      || (AT_PDP_STATE_ACTING == pstCallEntity->stIpv6Info.enWlanIpv6State))
    {
        if (VOS_OK == AT_PS_SndWlanMsgPdnDeActivateReq((VOS_UINT8)usClientId, cIsLocal, cIsHandover, ucCallId))
        {
            AT_PS_SetWlanCallStateByType(usClientId, ucCallId, pstCallEntity->stDialPdpType.enWlanPdpType, AT_PDP_STATE_DEACTING);

            /* 停止激活CNF定时器 */
            AT_SET_WLAN_ACT_PDN_CNF_TMR_NAME(ulTmrName);

            if (VOS_NULL_PTR != pstCallEntity->stPsCallTimerInfo.hWaitWlanActCnfTmrHdl)
            {
                AT_StopRelTimer(ulTmrName, &(pstCallEntity->stPsCallTimerInfo.hWaitWlanActCnfTmrHdl));
            }

            AT_SET_WLAN_DEACT_PDN_CNF_TMR_NAME(ulTmrName);
            AT_SET_WLAN_DEACT_PDN_CNF_TMR_PARAM(ulTmrParam, usClientId, ucCallId);

            /* 启动去激活CNF定时器 */
            AT_StartRelTimer(&(pstCallEntity->stPsCallTimerInfo.hWaitWlanDeActCnfTmrHdl),
                             AT_WAIT_WLAN_DEACT_PDN_CNF_TIMER_LEN,
                             ulTmrName,
                             ulTmrParam,
                             VOS_RELTIMER_NOLOOP);
        }
        else
        {
            AT_ERR_LOG("AT_PS_HangupWlanCall: End call failed.");
            return VOS_ERR;
        }
    }

    return VOS_OK;
}


VOS_UINT32 AT_PS_HangupCall(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_END_CAUSE_ENUM_UINT8    enCause
)
{
    /* 呼叫实体索引(CallId)由调用者保证其有效性 */

    /* (1) 检查IPv4类型呼叫是否已建立/正在建立
     *     -> 是: 断开该呼叫, 设置呼叫状态
     *     -> 否: 继续
     * (2) 检查IPv6类型呼叫是否已建立/正在建立
     *     -> 是: 断开该呼叫, 设置呼叫状态
     *     -> 否: 继续
     * (3) 结束
     */
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;

    pstCallEntity   = AT_PS_GetCallEntity(usClientId, ucCallId);

    if ( (AT_PDP_STATE_ACTED == pstCallEntity->stIpv4Info.enIpv4State)
      || (AT_PDP_STATE_ACTING == pstCallEntity->stIpv4Info.enIpv4State) )
    {
        if (VOS_OK == TAF_PS_CallEndEx(WUEPS_PID_AT,
                                       AT_PS_BuildPsCallExClientId(usClientId, ucCallId),
                                       0,
                                       pstCallEntity->stIpv4Info.ucIpv4Cid,
                                       enCause))
        {
            AT_PS_SetCallStateByType(usClientId, ucCallId, TAF_PDP_IPV4, AT_PDP_STATE_DEACTING);

            if ( (pstCallEntity->stIpv4Info.ucIpv4Cid == pstCallEntity->stIpv6Info.ucIpv6Cid)
              && ( (AT_PDP_STATE_ACTED == pstCallEntity->stIpv6Info.enIpv6State)
                || (AT_PDP_STATE_ACTING == pstCallEntity->stIpv6Info.enIpv6State) ))
            {
                AT_PS_SetCallStateByType(usClientId, ucCallId, TAF_PDP_IPV6, AT_PDP_STATE_DEACTING);
            }
        }
        else
        {
            AT_ERR_LOG("AT_PS_HangupCall: STEP1->End call failed.");
            return VOS_ERR;
        }
    }

    if ( ( (pstCallEntity->stIpv4Info.ucIpv4Cid != pstCallEntity->stIpv6Info.ucIpv6Cid)
        || (AT_PDP_STATE_IDLE == pstCallEntity->stIpv4Info.enIpv4State))
      && ( (AT_PDP_STATE_ACTED == pstCallEntity->stIpv6Info.enIpv6State)
        || (AT_PDP_STATE_ACTING == pstCallEntity->stIpv6Info.enIpv6State) ) )
    {
        if (VOS_OK == TAF_PS_CallEndEx(WUEPS_PID_AT,
                                       AT_PS_BuildPsCallExClientId(usClientId, ucCallId),
                                       0,
                                       pstCallEntity->stIpv6Info.ucIpv6Cid,
                                       enCause))
        {
            AT_PS_SetCallStateByType(usClientId, ucCallId, TAF_PDP_IPV6, AT_PDP_STATE_DEACTING);
        }
        else
        {
            AT_ERR_LOG("AT_PS_HangupCall: STEP2->End call step2 failed.");
            return VOS_ERR;
        }
    }

    return VOS_OK;
}


VOS_UINT32 AT_PS_ProcConflictDialUpWithCurrCall(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           ucCallId
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity;

    /* 呼叫实体索引(CallId)由调用者保证其有效性 */

    pstCallEntity = AT_PS_GetCallEntity(ucIndex, ucCallId);

    /* 如果连接正在建立, 返回OK, 同时上报已建立的连接状态 */
    if ( (VOS_TRUE == AT_PS_IsLinkGoingUp(pstCallEntity))
      || (VOS_TRUE == AT_PS_IsWlanLinkGoingUp(pstCallEntity)))
    {
        AT_INFO_LOG("AT_PS_ProcConflictDialUpWithCurrCall: Call is going up.");

        At_FormatResultData(ucIndex, AT_OK);

        AT_PS_ReportCurrCallConnState(pstCallEntity);

        return VOS_OK;
    }

    /* 如果连接正在断开, 返回ERROR */
    if (VOS_TRUE == AT_PS_IsLinkGoingDown(pstCallEntity))
    {
        AT_INFO_LOG("AT_PS_ProcConflictDialUpWithCurrCall: Call is going down.");

        AT_PS_SetPsCallErrCause(ucIndex, TAF_PS_CAUSE_UNKNOWN);

        At_FormatResultData(ucIndex, AT_ERROR);

        return VOS_OK;
    }

    if (VOS_TRUE == AT_PS_IsWlanLinkGoingDown(pstCallEntity))
    {
        AT_INFO_LOG("AT_PS_ProcConflictDialUpWithCurrCall: wlan Call is going down.");

        AT_PS_SetPsCallErrCause(ucIndex, TAF_PS_CAUSE_UNKNOWN);

        At_FormatResultData(ucIndex, AT_ERROR);

        return VOS_OK;
    }

    return VOS_ERR;
}


VOS_VOID AT_PS_ProcDialPdpTypeChg(
    VOS_UINT16                          usClientId,
    AT_DIAL_PARAM_STRU                 *pstUsrDialParam
)
{
    AT_COMM_PS_CTX_STRU                *pstCommPsCtx    = VOS_NULL_PTR;
    AT_PS_PDP_TYPE_CHG_MGR_INFO_STRU   *pstMgrInfo      = VOS_NULL_PTR;
    AT_PS_PDP_TYPE_CHG_ENTRY_STRU      *pstEntry        = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = MODEM_ID_0;

    if (VOS_OK != AT_GetModemIdFromClient(usClientId, &enModemId))
    {
        AT_ERR_LOG1("AT_PS_ProcDialPdpTypeChg: modem id is invalid.", enModemId);
        return;
    }

    pstCommPsCtx    = AT_GetCommPsCtxAddr();
    pstMgrInfo      = AT_GetPsPdpTypeChgMgrInfoByModemId(enModemId);

    /* 定制NV打开的情况下且针对双栈类型时才需要替换PDP TYPE */
    if ( (VOS_TRUE == pstCommPsCtx->astPdpTypeChgPolicyCfg[enModemId].ucPdpTypeChgEnableFlg)
      && (TAF_PDP_IPV4V6 == pstUsrDialParam->enPdpType))
    {
        pstEntry = AT_PS_LookupPdpTypeChgMgrEntry(pstMgrInfo, pstUsrDialParam->aucAPN);

        if (VOS_NULL_PTR != pstEntry)
        {
            pstUsrDialParam->enPdpType = pstEntry->enPdpType;
        }
    }

    return;
}


VOS_UINT32 AT_PS_DialUpDataSys(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    AT_PS_DATA_SYS_ENUM_UINT32          enDataSysType,
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           ucCallId
)
{
    AT_DIAL_PARAM_STRU                  stUsrDialParam;
    VOS_UINT32                          ulRslt;

    TAF_MEM_SET_S(&stUsrDialParam, (VOS_UINT32)(sizeof(stUsrDialParam)), 0x00, (VOS_UINT32)(sizeof(AT_DIAL_PARAM_STRU)));

    TAF_MEM_CPY_S(&stUsrDialParam,
                  (VOS_UINT32)(sizeof(stUsrDialParam)),
                  &(pstCallEntity->stUsrDialParam),
                  (VOS_UINT32)(sizeof(pstCallEntity->stUsrDialParam)));

    ulRslt                 = VOS_ERR;

    if (AT_PS_DATA_SYS_CELLULAR == enDataSysType)
    {
        /* 在Cellular下发起呼叫 */
        /* 处理PDP TYPE CHG */
        AT_PS_ProcDialPdpTypeChg(pstCallEntity->stUserInfo.enUserIndex, &stUsrDialParam);

        /* 建立呼叫 */
        ulRslt = AT_PS_SetupCall(pstCallEntity->stUserInfo.enUserIndex, ucCallId, &stUsrDialParam);
    }
    else if (AT_PS_DATA_SYS_WLAN == enDataSysType)
    {
        /* 在Wlan下发起呼叫 */
        /* APN被调节 */
        if (VOS_FALSE == AT_PS_IsWlanThrotAllowed(pstCallEntity))
        {
            /* 被调节错误码单独记录 */
            AT_PS_SetPsCallErrCause(ucIndex, TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_THROT_NOT_ALLOWED);

            return VOS_ERR;
        }

        /* 建立呼叫 */
        ulRslt = AT_PS_SetupWlanCall(&stUsrDialParam, ucIndex, ucCallId);
    }
    else
    {
        AT_ERR_LOG("AT_PS_DialUpDataSys: DIALUP DATA SYS NOT ALLOWED.");
    }

    if (VOS_OK != ulRslt)
    {
        /* 不允许发起 */
        AT_ERR_LOG("AT_PS_DialUpDataSys: not allow dial up.");

        /* 记录呼叫错误码 */
        AT_PS_SetPsCallErrCause(ucIndex, TAF_PS_CAUSE_UNKNOWN);

        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_PS_ProcDialUp(
    VOS_UINT8                           ucIndex,
    AT_PS_CALL_TYPE_ENUM_U8             enPsCallType
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity       = VOS_NULL_PTR;
    VOS_UINT8                          *pucSystemAppConfig  = VOS_NULL_PTR;
    AT_PS_USER_INFO_STRU                stUserInfo;
    AT_DIAL_PARAM_STRU                  stUsrDialParam;
    VOS_UINT32                          ulRslt;
    AT_PS_DATA_SYS_ENUM_UINT32          enDataSystem;
    VOS_UINT8                           ucCallId;

    TAF_MEM_SET_S(&stUserInfo, sizeof(stUserInfo), 0x00, sizeof(AT_PS_USER_INFO_STRU));
    TAF_MEM_SET_S(&stUsrDialParam, sizeof(stUsrDialParam), 0x00, sizeof(AT_DIAL_PARAM_STRU));
    enDataSystem        = AT_PS_DATA_SYS_CELLULAR;
    pucSystemAppConfig  = AT_GetSystemAppConfigAddr();

    /* 获取用户信息 */
    AT_PS_ParseUsrInfo(ucIndex, enPsCallType, &stUserInfo);

    /* 获取拨号参数 */
    ulRslt = AT_PS_ParseUsrDialParam(ucIndex, enPsCallType, &stUsrDialParam);

    if (VOS_OK != ulRslt)
    {
        AT_ERR_LOG("AT_PS_ProcDialUp: Get dial parameter failed.");

        AT_PS_SetPsCallErrCause(ucIndex, TAF_PS_CAUSE_UNKNOWN);

        return AT_ERROR;
    }

    /* 获取CID关联的呼叫实体索引 */
    ucCallId = AT_PS_TransCidToCallId(ucIndex, stUsrDialParam.ucCid);

    if (AT_PS_IsCallIdValid(ucIndex, ucCallId))
    {
        pstCallEntity = AT_PS_GetCallEntity(ucIndex, ucCallId);

        if (enPsCallType != pstCallEntity->enPsCallType)
        {
            AT_PS_SetPsCallErrCause(ucIndex, TAF_PS_CAUSE_UNKNOWN);
            AT_ERR_LOG("AT_PS_ProcCallDialUp: Entity Call Type is Different.");
            return AT_ERROR;
        }

        ulRslt = AT_PS_ProcConflictDialUpWithCurrCall(ucIndex, ucCallId);

        if (VOS_OK != ulRslt)
        {
            AT_PS_SetPsCallErrCause(ucIndex, TAF_PS_CAUSE_UNKNOWN);
            AT_PS_FreeCallEntity(ucIndex, ucCallId, TAF_PS_APN_DATA_SYS_NONE);
            AT_ERR_LOG("AT_PS_ProcCallDialUp: Call entity status error.");
            return AT_ERROR;
        }

        /* 在函数里面已经进行上报操作，这里直接返回即可 */
        return AT_SUCCESS;
    }

    /* 创建呼叫实体 */
    ulRslt = AT_PS_AllocCallEntity(ucIndex, enPsCallType, &ucCallId);

    if (VOS_OK != ulRslt)
    {
        AT_NORM_LOG("AT_PS_ProcDialUp: Alloc call entity failed.");

        AT_PS_SetPsCallErrCause(ucIndex, TAF_PS_CAUSE_UNKNOWN);

        return AT_ERROR;
    }

    pstCallEntity = AT_PS_GetCallEntity(ucIndex, ucCallId);

    /* 保存用户信息 */
    pstCallEntity->stUserInfo     = stUserInfo;

    /* 保存拨号参数 */
    pstCallEntity->stUsrDialParam = stUsrDialParam;

    /* 保存网卡ID*/
    AT_PS_UpdateCallIFaceId(pstCallEntity, ucCallId);

    if ( (SYSTEM_APP_WEBUI != *pucSystemAppConfig)
      && (AT_PS_WAN_TYPE_APP == enPsCallType))
    {
        /* 处理域选配置 */
        AT_PS_SelectApnDataSysConfig(ucIndex, pstCallEntity);

        enDataSystem = AT_PS_GetPreDataSystem(pstCallEntity, ucIndex);
    }

    /* 根据域选结果 选择呼叫发起域 */
    ulRslt          = AT_PS_DialUpDataSys(pstCallEntity, enDataSystem, ucIndex, ucCallId);

    if (VOS_OK != ulRslt)
    {
        /* 释放呼叫实体 */
        AT_PS_FreeCallEntity(ucIndex, ucCallId, TAF_PS_APN_DATA_SYS_NONE);

        return AT_ERROR;
    }

    return AT_OK;
}


VOS_UINT32 AT_PS_ProcDialDownOneDataSys(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_END_CAUSE_ENUM_UINT8    enCause
)
{
    AT_PS_USER_INFO_STRU               *pstUserInfo = VOS_NULL_PTR;
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;

    pstUserInfo = AT_PS_GetUserInfo(ucIndex, ucCallId);
    pstCallEntity = AT_PS_GetCallEntity(ucIndex, ucCallId);

    if ( (VOS_TRUE == pstCallEntity->stApnDataSysInfo.ucDataSysInfoFlg)
      && (TAF_PS_APN_DATA_SYS_WLAN == pstCallEntity->stApnDataSysInfo.enCurrentDataSys))
    {
        /* 需要在WLAN上发起去激活请求 */
        if (VOS_TRUE == AT_PS_IsWlanLinkGoingDown(pstCallEntity))
        {
            At_FormatResultData(ucIndex, AT_OK);

            AT_PS_ReportCurrCallEndState(pstCallEntity);

            return AT_SUCCESS;
        }

        if (VOS_OK != AT_PS_HangupWlanCall(pstUserInfo->enUserIndex, ucCallId, VOS_FALSE, VOS_FALSE))
        {
            AT_ERR_LOG("AT_PS_ProcDialDown: Hangup wlan call failed.");
            return AT_ERROR;
        }

        return AT_OK;
    }

    /* 检查连接状态是否为正在断开
     * (A) 如果连接正在断开
     *　　 -> 上报OK
     *     -> 上报已断开连接^DEND(or NDISSTAT)
     *     -> 返回SUCCESS
     * (B) 其他状态
     *     -> 继续处理
     */
    if (VOS_TRUE == AT_PS_IsLinkGoingDown(pstCallEntity))
    {
        At_FormatResultData(ucIndex, AT_OK);

        AT_PS_ReportCurrCallEndState(pstCallEntity);

        return AT_SUCCESS;
    }

    /* 挂断呼叫 */
    if (VOS_OK != AT_PS_HangupCall(pstUserInfo->enUserIndex, ucCallId, enCause))
    {
        AT_ERR_LOG("AT_PS_ProcDialDown: Hangup call failed.");
        return AT_ERROR;
    }

    return AT_OK;
}


VOS_UINT32 AT_PS_ProcDialDownMultiDataSys(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_END_CAUSE_ENUM_UINT8    enCause
)
{
    AT_PS_USER_INFO_STRU               *pstUserInfo = VOS_NULL_PTR;
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;

    pstUserInfo = AT_PS_GetUserInfo(ucIndex, ucCallId);
    pstCallEntity = AT_PS_GetCallEntity(ucIndex, ucCallId);

    if (TAF_PS_APN_DATA_SYS_CELLULAR == pstCallEntity->stApnDataSysInfo.enCurrentDataSys)
    {
        /* 需要在CELLULAR上发起去激活请求，在WLAN上发起本地去激活请求 */
        if (VOS_OK != AT_PS_HangupCall(pstUserInfo->enUserIndex, ucCallId, enCause))
        {
            AT_ERR_LOG("AT_PS_ProcDialDownMultiDataSys: Hangup call failed.");
            return AT_ERROR;
        }

        (VOS_VOID)AT_PS_HangupWlanCall(pstUserInfo->enUserIndex, ucCallId, VOS_TRUE, VOS_TRUE);

        AT_PS_SetCallHandOverFlg(pstUserInfo->enUserIndex, ucCallId, VOS_FALSE);

        /* 设置handover呼叫类型 */
        AT_PS_SetHoCallType(pstUserInfo->enUserIndex, ucCallId, TAF_PDP_TYPE_BUTT);

        return AT_OK;
    }
    else if (TAF_PS_APN_DATA_SYS_WLAN == pstCallEntity->stApnDataSysInfo.enCurrentDataSys)
    {
        /* 需要在WLAN上发起去激活请求，在CELLULAR上发起本地去激活请求 */
        if (VOS_OK != AT_PS_HangupWlanCall(pstUserInfo->enUserIndex, ucCallId, VOS_FALSE, VOS_FALSE))
        {
            AT_ERR_LOG("AT_PS_ProcDialDownMultiDataSys: Hangup wlan call failed.");
            return AT_ERROR;
        }

        (VOS_VOID)AT_PS_HangupCall(pstCallEntity->stUserInfo.enUserIndex, ucCallId, TAF_PS_CALL_END_CAUSE_LOCAL);

        AT_PS_SetCallHandOverFlg(pstUserInfo->enUserIndex, ucCallId, VOS_FALSE);

        /* 设置handover呼叫类型 */
        AT_PS_SetHoCallType(pstUserInfo->enUserIndex, ucCallId, TAF_PDP_TYPE_BUTT);

        return AT_OK;
    }
    else
    {
        AT_ERR_LOG("AT_PS_ProcDialDownMultiDataSys: enCurrentDataSys is failed.");
        return AT_ERROR;
    }
}


VOS_UINT32 AT_PS_ProcDialDown(
    VOS_UINT8                           ucIndex,
    AT_PS_CALL_TYPE_ENUM_U8             enPsCallType
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;
    VOS_UINT8                           ucCallId;
    TAF_PS_CALL_END_CAUSE_ENUM_UINT8    enCause;

    ucCallId = AT_PS_TransCidToCallId(ucIndex, (VOS_UINT8)gastAtParaList[0].ulParaValue);

    if (VOS_FALSE == AT_PS_IsCallIdValid(ucIndex, ucCallId))
    {
        At_FormatResultData(ucIndex, AT_OK);

        AT_PS_ReportAllCallEndState(ucIndex, enPsCallType);

        return AT_SUCCESS;
    }

    pstCallEntity = AT_PS_GetCallEntity(ucIndex, ucCallId);

    enCause = AT_PS_TransCallEndCause((VOS_UINT8)gastAtParaList[1].ulParaValue);

    if (VOS_FALSE == AT_PS_GetPsCallHandOverFlg(pstCallEntity))
    {
        /* 只有在一个域上需要去激活 */
        return AT_PS_ProcDialDownOneDataSys(ucIndex, ucCallId, enCause);
    }
    else
    {
        /* CELLULAR和WLAN上都需要去激活 */
        return AT_PS_ProcDialDownMultiDataSys(ucIndex, ucCallId, enCause);
    }
}


VOS_UINT32 AT_PS_ProcDialCmd(
    VOS_UINT8                           ucIndex,
    AT_PS_CALL_TYPE_ENUM_U8             enPsCallType
)
{
    VOS_UINT8                          *pucSystemAppConfig;

    pucSystemAppConfig  = AT_GetSystemAppConfigAddr();

    if (TAF_PS_CALL_TYPE_UP == gastAtParaList[1].ulParaValue)
    {
        if ( (AT_PS_WAN_TYPE_APP == enPsCallType)
          && (SYSTEM_APP_WEBUI == *pucSystemAppConfig)
          && (AT_HILINK_GATEWAY_MODE == g_enHiLinkMode))
        {
            /* 记录PS域呼叫错误码 */
            AT_PS_SetPsCallErrCause(ucIndex, TAF_PS_CAUSE_UNKNOWN);

            return AT_ERROR;
        }

        return AT_PS_ProcDialUp(ucIndex, enPsCallType);
    }
    else
    {
        return AT_PS_ProcDialDown(ucIndex, enPsCallType);
    }
}


VOS_UINT16 AT_PS_BuildExClientId(VOS_UINT16 usClientId)
{
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = MODEM_ID_0;

    if (VOS_OK != AT_GetModemIdFromClient(usClientId, &enModemId))
    {
        enModemId =  MODEM_ID_BUTT;
    }

    return TAF_PS_BUILD_EXCLIENTID(enModemId, usClientId);
}


VOS_UINT16 AT_PS_BuildNdisExClientId(
    VOS_UINT16                          usPortIndex,
    VOS_UINT16                          usClientId
)
{
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = MODEM_ID_0;

    if (VOS_OK != AT_GetModemIdFromClient(usPortIndex, &enModemId))
    {
        enModemId =  MODEM_ID_BUTT;
    }

    return TAF_PS_BUILD_EXCLIENTID(enModemId, usClientId);
}


VOS_UINT16 AT_PS_BuildPsCallExClientId(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;
    VOS_UINT8                          *pucSystemAppConfig;

    pucSystemAppConfig  = AT_GetSystemAppConfigAddr();
    pstCallEntity       = AT_PS_GetCallEntity(usClientId, ucCallId);

    if (AT_PS_WAN_TYPE_APP == pstCallEntity->enPsCallType)
    {
        if (SYSTEM_APP_WEBUI != *pucSystemAppConfig)
        {
            return AT_PS_BuildExClientId(usClientId);
        }

        return AT_PS_BuildExClientId(AT_APP_GET_CLIENT_ID());
    }
    else
    {
        return AT_PS_BuildNdisExClientId(pstCallEntity->stUserInfo.enPortIndex, AT_NDIS_GET_CLIENT_ID());
    }
}


VOS_UINT32 AT_PS_CheckDialRatType(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           ucBitRatType
)
{
    if (VOS_TRUE == At_CheckCurrRatModeIsCL(ucIndex))
    {
        switch (ucBitRatType)
        {
            case AT_PS_DIAL_RAT_TYPE_NO_ASTRICT:
            case AT_PS_DIAL_RAT_TYPE_1X_OR_HRPD:
            case AT_PS_DIAL_RAT_TYPE_LTE_OR_EHRPD:
                return VOS_TRUE;

            default:
                AT_NORM_LOG1("AT_PS_CheckDialRatType: Rat Type Error.\n", ucBitRatType);
                return VOS_FALSE;
        }
    }
    else
    {
        AT_NORM_LOG("AT_PS_CheckDialRatType: Not CL mode.\n");
        return VOS_FALSE;
    }
}


VOS_VOID AT_PS_ProcRabidChangedEvent(
    TAF_PS_CALL_PDP_RABID_CHANGE_IND_STRU  *pstEvent
)
{
    VOS_UINT8                               ucCallId;
    AT_PS_CALL_ENTITY_STRU                 *pstCallEntity   = VOS_NULL_PTR;
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU      *pstActEvent     = VOS_NULL_PTR;
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU     stDeactEvent;

    ucCallId = AT_PS_TransCidToCallId(pstEvent->stCtrl.usClientId, pstEvent->ucCid);

    if (VOS_FALSE == AT_PS_IsCallIdValid(pstEvent->stCtrl.usClientId, ucCallId))
    {
        AT_ERR_LOG("AT_PS_ProcRabidChangedEvent:ERROR: CallId is invalid!");
        return;
    }

    /*--------------------------1.通知网卡去激活，去注册流控点----------------------------*/

    TAF_MEM_SET_S(&stDeactEvent,
                  sizeof(stDeactEvent),
                  0,
                  sizeof(TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU));

    TAF_MEM_CPY_S(&stDeactEvent.stCtrl,
                  sizeof(TAF_CTRL_STRU),
                  &pstEvent->stCtrl,
                  sizeof(TAF_CTRL_STRU));

    stDeactEvent.ucRabId                = pstEvent->ucOldRabId;
    stDeactEvent.enPdpType              = pstEvent->enPdpType;

    /* 向网卡发送PDP去激活事件 */
    AT_PS_DeactivateIFace(ucCallId, &stDeactEvent, pstEvent->enPdpType);

    /* 向FC去注册流控点 */
    AT_PS_DeRegFCPoint(ucCallId, &stDeactEvent);


    /*--------------------------2.通知网卡激活，注册流控点----------------------------*/
    /*lint -save -e516 */
    pstActEvent = (TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU *)PS_MEM_ALLOC(WUEPS_PID_TAF,
                      sizeof(TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU));
    /*lint -restore */
    /* 申请消息失败 */
    if (VOS_NULL_PTR == pstActEvent)
    {
        /* 系统异常打印, 直接返回 */
        AT_ERR_LOG("AT_PS_ProcRabidChangedEvent: PS_MEM_ALLOC Error!");
        return;
    }

    TAF_MEM_SET_S(pstActEvent,
                  sizeof(TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU),
                  0,
                  sizeof(TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU));

    TAF_MEM_CPY_S(&pstActEvent->stCtrl,
                  sizeof(TAF_CTRL_STRU),
                  &pstEvent->stCtrl,
                  sizeof(TAF_CTRL_STRU));

    pstActEvent->ucRabId                = pstEvent->ucNewRabId;

    /*--------------------------3.刷新AT本地信息----------------------------*/
    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);

    if (TAF_PDP_IPV4 == (pstEvent->enPdpType & TAF_PDP_IPV4))
    {
        pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ucRabId   = pstEvent->ucNewRabId;
    }

    if (TAF_PDP_IPV6 == (pstEvent->enPdpType & TAF_PDP_IPV6))
    {
        pstCallEntity->stIpv6Info.stIpv6DhcpInfo.ucRabId   = pstEvent->ucNewRabId;
    }

    /* 向网卡发送PDP激活事件 */
    AT_PS_ActivateIFace(ucCallId, pstActEvent, pstEvent->enPdpType);

    /* 向FC注册流控点 */
    AT_PS_RegFCPoint(ucCallId, pstActEvent);
    /*lint -save -e516 */
    PS_MEM_FREE(WUEPS_PID_TAF, pstActEvent);
    /*lint -restore */

    return;
}


TAF_PS_CAUSE_ENUM_UINT32 AT_PS_MapWlanActCnfCause(
    WIFI_IMSA_PDN_ACT_RESULT_ENUM_INT8      ucErrCause
)
{
    CONST AT_PS_WLAN_PDN_ACT_ERR_CODE_MAP_STRU *pstErrMapTblPtr = VOS_NULL_PTR;
    VOS_UINT32                                  ulErrMapTblSize;
    VOS_UINT32                                  ulCnt;

    pstErrMapTblPtr     = g_astTafPsWlanPdnActErrCodeMapTbl;
    ulErrMapTblSize     = sizeof(g_astTafPsWlanPdnActErrCodeMapTbl) / sizeof(AT_PS_WLAN_PDN_ACT_ERR_CODE_MAP_STRU);

    for (ulCnt = 0; ulCnt < ulErrMapTblSize; ulCnt++)
    {
        if (pstErrMapTblPtr[ulCnt].ulWlanCause == ucErrCause)
        {
            return pstErrMapTblPtr[ulCnt].enPsCause;
        }
    }

    return TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_UNKNOWN;
}


TAF_PS_CAUSE_ENUM_UINT32 AT_PS_MapWlanDeActIndCause(
    WIFI_IMSA_PDN_DEACT_CAUSE_ENUM_INT8     ucErrCause
)
{
    CONST AT_PS_WLAN_PDN_DEACT_ERR_CODE_MAP_STRU   *pstErrMapTblPtr = VOS_NULL_PTR;
    VOS_UINT32                                      ulErrMapTblSize;
    VOS_UINT32                                      ulCnt;

    pstErrMapTblPtr     = g_astTafPsWlanPdnDeActErrCodeMapTbl;
    ulErrMapTblSize     = sizeof(g_astTafPsWlanPdnDeActErrCodeMapTbl) / sizeof(AT_PS_WLAN_PDN_DEACT_ERR_CODE_MAP_STRU);

    for (ulCnt = 0; ulCnt < ulErrMapTblSize; ulCnt++)
    {
        if (pstErrMapTblPtr[ulCnt].ulWlanCause == ucErrCause)
        {
            return pstErrMapTblPtr[ulCnt].enPsCause;
        }
    }

    return TAF_PS_CAUSE_WLAN_PDN_DEACT_RESULT_UNKNOWN;
}


VOS_VOID AT_PS_SetWlanCallStateByType(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    AT_PDP_STATE_ENUM_U8                enPdpState
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);

    switch (enPdpType)
    {
        case TAF_PDP_IPV4:
            pstCallEntity->stIpv4Info.enWlanIpv4State = enPdpState;
            break;

        case TAF_PDP_IPV6:
            pstCallEntity->stIpv6Info.enWlanIpv6State = enPdpState;
            break;

        case TAF_PDP_IPV4V6:
            pstCallEntity->stIpv4Info.enWlanIpv4State = enPdpState;
            pstCallEntity->stIpv6Info.enWlanIpv6State = enPdpState;
            break;

        default:
            AT_ERR_LOG("AT_PS_SetWlanCallStateByType: PDP type is invalid!");
            break;
    }

    return;
}

VOS_VOID AT_PS_SetWlanCurrCallType(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);

    pstCallEntity->stDialPdpType.enWlanPdpType = enPdpType;

    return;
}

VOS_VOID AT_PS_SetHoCallType(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);

    pstCallEntity->stDialPdpType.enHoPdpType = enPdpType;

    return;
}


VOS_VOID AT_PS_SetPsCallCurrentDataSys(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    TAF_PS_APN_DATA_SYS_ENUM_UINT8      enCurrentDataSys
)
{
    if (VOS_TRUE == pstCallEntity->stApnDataSysInfo.ucDataSysInfoFlg)
    {
        pstCallEntity->stApnDataSysInfo.enCurrentDataSys = enCurrentDataSys;
    }

    return;
}


TAF_PS_APN_DATA_SYS_ENUM_UINT8 AT_PS_GetPsCallCurrentDataSys(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
)
{
    if (VOS_TRUE == pstCallEntity->stApnDataSysInfo.ucDataSysInfoFlg)
    {
        return pstCallEntity->stApnDataSysInfo.enCurrentDataSys;
    }

    return TAF_PS_APN_CURRENT_DOMAIN_BUTT;
}


VOS_VOID AT_PS_SetCallHandOverFlg(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    VOS_UINT8                           ucIsHandOver
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);

    if (VOS_TRUE == pstCallEntity->stApnDataSysInfo.ucDataSysInfoFlg)
    {
        AT_NORM_LOG1("AT_PS_SetCallHandOverFlg: handover is ", ucIsHandOver);
        pstCallEntity->stApnDataSysInfo.ucIsHandOver = ucIsHandOver;
    }

    return;
}


VOS_UINT8 AT_PS_GetPsCallHandOverFlg(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
)
{
    if (VOS_TRUE == pstCallEntity->stApnDataSysInfo.ucDataSysInfoFlg)
    {
        AT_NORM_LOG1("AT_PS_GetPsCallHandOverFlg: handover is ", pstCallEntity->stApnDataSysInfo.ucIsHandOver);
        return pstCallEntity->stApnDataSysInfo.ucIsHandOver;
    }

    AT_NORM_LOG("AT_PS_GetPsCallHandOverFlg: not data sys apn, handover is false");
    return VOS_FALSE;
}


VOS_VOID AT_PS_SetCallHandOverFailCause(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    TAF_PS_CAUSE_ENUM_UINT32            enHoFailCause
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);

    if (VOS_TRUE == pstCallEntity->stApnDataSysInfo.ucDataSysInfoFlg)
    {
        AT_NORM_LOG1("AT_PS_SetCallHandOverFailCause: enHoFailCause is ", enHoFailCause);
        pstCallEntity->stApnDataSysInfo.enHoFailCause = enHoFailCause;
    }

    return;
}


TAF_PS_CAUSE_ENUM_UINT32 AT_PS_GetCallHandOverFailCause(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
)
{
    if (VOS_TRUE == pstCallEntity->stApnDataSysInfo.ucDataSysInfoFlg)
    {
        AT_NORM_LOG1("AT_PS_GetCallHandOverFailCause: enHoFailCause is ", pstCallEntity->stApnDataSysInfo.enHoFailCause);
        return pstCallEntity->stApnDataSysInfo.enHoFailCause;
    }

    return TAF_PS_CAUSE_SUCCESS;
}


TAF_PDP_TYPE_ENUM_UINT8 AT_PS_GetHoPdpIpType(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
)
{
    if (TAF_PS_APN_DATA_SYS_CELLULAR == pstCallEntity->stApnDataSysInfo.enCurrentDataSys)
    {
        /* CELLULAR2WLAN */
        if ( (AT_PDP_STATE_ACTED == pstCallEntity->stIpv4Info.enIpv4State)
          && (AT_PDP_STATE_ACTED == pstCallEntity->stIpv6Info.enIpv6State))
        {
            return TAF_PDP_IPV4V6;
        }
        else if (AT_PDP_STATE_ACTED == pstCallEntity->stIpv4Info.enIpv4State)
        {
            return TAF_PDP_IPV4;
        }
        else
        {
            return TAF_PDP_IPV6;
        }
    }
    else if (TAF_PS_APN_DATA_SYS_WLAN == pstCallEntity->stApnDataSysInfo.enCurrentDataSys)
    {
        /* WLAN2CELLULAR */
        if ( (AT_PDP_STATE_ACTED == pstCallEntity->stIpv4Info.enWlanIpv4State)
          && (AT_PDP_STATE_ACTED == pstCallEntity->stIpv6Info.enWlanIpv6State))
        {
            return TAF_PDP_IPV4V6;
        }
        else if (AT_PDP_STATE_ACTED == pstCallEntity->stIpv4Info.enWlanIpv4State)
        {
            return TAF_PDP_IPV4;
        }
        else
        {
            return TAF_PDP_IPV6;
        }
    }
    else
    {
        AT_ERR_LOG("AT_PS_GetHoPdpIpType: enCurrentDataSys is error.");

        return pstCallEntity->stUsrDialParam.enPdpType;
    }
}


VOS_UINT32 AT_PS_GetDestDataSysWhenSupMultiDataSys(
    TAF_PS_DATA_SYSTEM_STATUS_STRU          stDataSystem,
    AT_PS_CALL_ENTITY_STRU                 *pstCallEntity,
    TAF_PS_APN_DATA_SYS_ENUM_UINT8         *penDestDataSys
)
{
    TAF_PS_APN_DATA_SYS_POLICY_INFO_STRU    *pstApnDataSysPolicyInfo = VOS_NULL_PTR;

    /* 通过域选索引得到域选配置 */
    pstApnDataSysPolicyInfo = AT_PS_GetApnDataSysPolicyInfo(pstCallEntity->stUserInfo.enPortIndex,
                                                            pstCallEntity->stApnDataSysInfo.ucDataSysPolicyIndex);

    if ( (VOS_FALSE == pstCallEntity->stApnDataSysInfo.ucDataSysInfoFlg)
      || (TAF_PS_APN_DATA_SYS_POLICY_CELLULAR_PREFER != pstApnDataSysPolicyInfo->stDataSysPolicy.enDataSysPolicy))
    {
        AT_WARN_LOG("AT_PS_GetDestDataSysWhenSupMultiDataSys: not support multi data sys");
        return VOS_FALSE;
    }

    if ( (VOS_FALSE == stDataSystem.bCellularPowerState)
      && (VOS_FALSE == stDataSystem.bWlanPowerState))
    {
        AT_WARN_LOG("AT_PS_GetDestDataSysWhenSupMultiDataSys: DATA SYS IS  NONE");
        return VOS_FALSE;
    }

    if (TAF_PS_APN_DATA_SYS_POLICY_CELLULAR_PREFER == pstApnDataSysPolicyInfo->stDataSysPolicy.enDataSysPolicy)
    {
        /* CELLULAR域优先 */
        if ( (VOS_TRUE == stDataSystem.bCellularPowerState)
          && (TAF_PS_CELLULAR_SERVICE_STATUS_NORMAL_SERVICE == stDataSystem.enPsServiceStatus))
        {
            AT_NORM_LOG("AT_PS_GetDestDataSysWhenSupMultiDataSys: Dest Data Sys is CELLULAR.");
            *penDestDataSys = TAF_PS_APN_DATA_SYS_CELLULAR;
            return VOS_TRUE;
        }

        if ( (VOS_TRUE == stDataSystem.bWlanPowerState)
          && (TAF_PS_WLAN_SERVICE_STATUS_NORMAL_SERVICE == stDataSystem.enWiFiServiceStatus))
        {
            AT_NORM_LOG("AT_PS_GetDestDataSysWhenSupMultiDataSys: Dest Data Sys is WLAN .");
            *penDestDataSys = TAF_PS_APN_DATA_SYS_WLAN;
            return VOS_TRUE;
        }
    }

    AT_WARN_LOG("AT_PS_GetDestDataSysWhenSupMultiDataSys: data sys unknown!");
    return VOS_FALSE;
}

TAF_PS_APN_DATA_SYS_POLICY_INFO_STRU* AT_PS_GetApnDataSysPolicyInfo(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucIndex
)
{
    AT_COMM_PS_CTX_STRU                *pstCommPsCtx          = VOS_NULL_PTR;
    VOS_UINT32                          ulRslt;
    MODEM_ID_ENUM_UINT16                usModemId;

    usModemId = MODEM_ID_0;

    ulRslt = AT_GetModemIdFromClient((VOS_UINT8)usClientId, &usModemId);

    if (VOS_ERR == ulRslt)
    {
        AT_ERR_LOG("AT_PS_GetApnDataSysPolicyInfo: Get ModemId Fail!");
    }

    pstCommPsCtx    = AT_GetCommPsCtxAddr();

    return &(pstCommPsCtx->astApnDataSysCfgList[usModemId].astApnDataSysInfo[ucIndex]);
}

VOS_VOID AT_PS_SelectApnDataSysConfig(
    VOS_UINT8                           ucIndex,
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
)
{
    TAF_PS_APN_DATA_SYS_POLICY_CONFIG_STRU     *pstApnDataSysCfgList  = VOS_NULL_PTR;
    VOS_UINT32                                  i;
    MODEM_ID_ENUM_UINT16                        usModemId;

    usModemId = MODEM_ID_0;

    if (VOS_OK != AT_GetModemIdFromClient(ucIndex, &usModemId))
    {
        AT_ERR_LOG("AT_PS_SelectApnDataSysConfig: Get modem id fail.");

        return;
    }

    pstApnDataSysCfgList    = &(AT_GetCommPsCtxAddr()->astApnDataSysCfgList[usModemId]);

    /* 遍历域选配置APN */
    for (i = 0; i < pstApnDataSysCfgList->ucProfileNum; i++)
    {
        /* 用户设置APN与域选APN匹配 保存配置到呼叫实体 */
        if (0 == VOS_StrNiCmp((VOS_CHAR *)(pstApnDataSysCfgList->astApnDataSysInfo[i].aucApn),
                                (VOS_CHAR *)(pstCallEntity->stUsrDialParam.aucAPN),
                                TAF_MAX_APN_LEN + 1))
        {
            AT_NORM_LOG1("AT_PS_SelectApnDataSysConfig: this apn is data sys config. data system policy index is ", i);
            pstCallEntity->stApnDataSysInfo.ucDataSysInfoFlg     = VOS_TRUE;
            pstCallEntity->stApnDataSysInfo.ucDataSysPolicyIndex = (VOS_UINT8)i;
            break;
        }
    }

    return;
}


AT_PS_DATA_SYS_ENUM_UINT32 AT_PS_GetPreDataSystem(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    VOS_UINT8                           ucIndex
)
{
    TAF_PS_APN_DATA_SYS_POLICY_INFO_STRU   *pstApnDataSysPolicyInfo = VOS_NULL_PTR;
    VOS_UINT32                              ulRslt;
    VOS_UINT8                               ucBitDataSystem;

    /* 通过域选索引得到域选配置 */
    pstApnDataSysPolicyInfo = AT_PS_GetApnDataSysPolicyInfo(ucIndex, pstCallEntity->stApnDataSysInfo.ucDataSysPolicyIndex);

    ulRslt                  = VOS_ERR;
    ucBitDataSystem         = 0;

    /* 域选配置不使能 */
    if (VOS_FALSE == pstCallEntity->stApnDataSysInfo.ucDataSysInfoFlg)
    {
        return AT_PS_DATA_SYS_CELLULAR;
    }

    /* 获取系统状态失败 不允许发起 */
    ulRslt = TAF_AGENT_GetDataSystem(ucIndex, &ucBitDataSystem);

    if (VOS_OK != ulRslt)
    {
        AT_NORM_LOG("AT_PS_GetPreDataSystem: Can't Get Data System ");
        return AT_PS_DATA_SYS_NONE;
    }

    if (0 == ucBitDataSystem)
    {
        AT_NORM_LOG("AT_PS_GetPreDataSystem: ucBitDataSystem is NONE .");
        return AT_PS_DATA_SYS_NONE;
    }

    /* 在Cellular下发起 */
    if (VOS_TRUE == AT_PS_DialUpInCellularAllowed(ucBitDataSystem, pstApnDataSysPolicyInfo->stDataSysPolicy.enDataSysPolicy))
    {
        AT_NORM_LOG("AT_PS_GetPreDataSystem: Cellular is satisfy .");
        return AT_PS_DATA_SYS_CELLULAR;
    }

    /* 在Wlan下发起 */
    if (VOS_TRUE == AT_PS_DialUpInWlanAllowed(ucBitDataSystem, pstApnDataSysPolicyInfo->stDataSysPolicy.enDataSysPolicy))
    {
        AT_NORM_LOG("AT_PS_GetPreDataSystem: Wlan is satisfy .");
        return AT_PS_DATA_SYS_WLAN;
    }

    /* 不允许发起 */
    AT_NORM_LOG("AT_PS_GetPreDataSystem: Data System is not satisfy .");
    return AT_PS_DATA_SYS_NONE;
}


VOS_UINT32 AT_PS_IsPsCallInSteadyState(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
)
{
    /* 正在切换过程中 */
    if ( ( (AT_PDP_STATE_IDLE != pstCallEntity->stIpv4Info.enIpv4State) || (AT_PDP_STATE_IDLE != pstCallEntity->stIpv6Info.enIpv6State))
      && ( (AT_PDP_STATE_IDLE != pstCallEntity->stIpv4Info.enWlanIpv4State) || (AT_PDP_STATE_IDLE != pstCallEntity->stIpv6Info.enWlanIpv6State)))
    {
        AT_NORM_LOG("AT_PS_IsPsCallInSteadyState: data sys switching .");
        return VOS_FALSE;
    }

    /* 原域(CELLULAR)上正在发起去激活操作 */
    if (TAF_PS_APN_DATA_SYS_CELLULAR == pstCallEntity->stApnDataSysInfo.enCurrentDataSys)
    {
        if ( (AT_PDP_STATE_DEACTING == pstCallEntity->stIpv4Info.enIpv4State)
          || (AT_PDP_STATE_DEACTING == pstCallEntity->stIpv6Info.enIpv6State))
        {
            AT_NORM_LOG("AT_PS_IsPsCallInSteadyState: cellular is deacting .");
            return VOS_FALSE;
        }
    }

    /* 原域(WLAN)上正在发起去激活操作 */
    if (TAF_PS_APN_DATA_SYS_WLAN == pstCallEntity->stApnDataSysInfo.enCurrentDataSys)
    {
        if ( (AT_PDP_STATE_DEACTING == pstCallEntity->stIpv4Info.enWlanIpv4State)
          || (AT_PDP_STATE_DEACTING == pstCallEntity->stIpv6Info.enWlanIpv6State))
        {
            AT_NORM_LOG("AT_PS_IsPsCallInSteadyState: wlan is deacting .");
            return VOS_FALSE;
        }
    }

    return VOS_TRUE;
}


VOS_UINT32 AT_PS_IsNeedPsCallDataSysSwitch(
    VOS_UINT16                          usClientId,
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    TAF_PS_APN_DATA_SYS_ENUM_UINT8     *penDestDataSys
)
{
    AT_COMM_PS_CTX_STRU                    *pstCommPsCtx            = VOS_NULL_PTR;
    TAF_PS_APN_DATA_SYS_POLICY_INFO_STRU   *pstApnDataSysPolicyInfo = VOS_NULL_PTR;
    VOS_UINT32                              ulRst;

    MODEM_ID_ENUM_UINT16                    usModemId;

    pstCommPsCtx    = AT_GetCommPsCtxAddr();

    if (VOS_OK != AT_GetModemIdFromClient(usClientId, &usModemId))
    {
        AT_ERR_LOG("AT_PS_IsNeedPsCallDataSysSwitch: Get modem id fail.");
        return VOS_FALSE;
    }

    /* 通过域选索引得到域选配置 */
    pstApnDataSysPolicyInfo = AT_PS_GetApnDataSysPolicyInfo(usClientId, pstCallEntity->stApnDataSysInfo.ucDataSysPolicyIndex);

    /* 域选策略不是CELLULAR和WLAN，则不允许切换 */
    if (TAF_PS_APN_DATA_SYS_POLICY_CELLULAR_PREFER != pstApnDataSysPolicyInfo->stDataSysPolicy.enDataSysPolicy)
    {
        AT_NORM_LOG("AT_PS_IsNeedPsCallDataSysSwitch: not TAF_PS_APN_DATA_SYS_POLICY_CELLULAR_PREFER.");
        return VOS_FALSE;
    }

    ulRst = AT_PS_GetDestDataSysWhenSupMultiDataSys(pstCommPsCtx->astDataSystemStatus[usModemId], pstCallEntity, penDestDataSys);

    /* 得到目的域失败 */
    if (VOS_FALSE == ulRst)
    {
        AT_NORM_LOG("AT_PS_IsNeedPsCallDataSysSwitch: AT_PS_GetDestDataSysWhenSupMultiDataSys is false.");
        return VOS_FALSE;
    }

    /* 当前域已经满足 */
    if ((*penDestDataSys) == pstCallEntity->stApnDataSysInfo.enCurrentDataSys)
    {
        AT_NORM_LOG("AT_PS_IsNeedPsCallDataSysSwitch: CurrentDataSys is ok.");
        return VOS_FALSE;
    }

    /* 防乒乓定时器正在运行，则不允许切换 */
    if (VOS_NULL_PTR != pstCallEntity->stPsCallTimerInfo.hProtectInDataSysTmrHdl)
    {
        AT_NORM_LOG("AT_PS_IsNeedPsCallDataSysSwitch: hProtectInDataSysTmrHdl is not null.");
        return VOS_FALSE;
    }

    /* WLAN2CELLULAR，漫游情况下不支持切换，则不允许切换 */
    if ( (TAF_PS_APN_DATA_SYS_CELLULAR == (*penDestDataSys))
      && (VOS_TRUE == pstCommPsCtx->astDataSystemStatus[usModemId].bCellularRoamingStatus)
      && (VOS_FALSE == pstApnDataSysPolicyInfo->stDataSysPolicy.ucRoamHoSupportFlg))
    {
        AT_NORM_LOG("AT_PS_IsNeedPsCallDataSysSwitch: not support Roam data Sys Switch.");
        return VOS_FALSE;
    }

    /* WLAN2CELLULAR，接入技术不支持切换，则不允许切换 */
    if ( (TAF_PS_APN_DATA_SYS_CELLULAR == (*penDestDataSys))
      && (0 == (pstApnDataSysPolicyInfo->stDataSysPolicy.ucDataSysSwitchRatPolicy & (0x01 << pstCommPsCtx->astDataSystemStatus[usModemId].enRatType))))
    {
        AT_NORM_LOG("AT_PS_IsNeedPsCallDataSysSwitch: not support Rat data Sys Switch.");
        return VOS_FALSE;
    }

    /* 非稳态，则不允许切换 */
    if (VOS_FALSE == AT_PS_IsPsCallInSteadyState(pstCallEntity))
    {
        AT_NORM_LOG("AT_PS_IsNeedPsCallDataSysSwitch: doing data Sys switch.");
        return VOS_FALSE;
    }

    /* CELLULAR2WLAN，调节模块不允许发起，则不允许切换，等待订阅通知后发起 */
    if ( (TAF_PS_APN_DATA_SYS_WLAN == (*penDestDataSys))
      && (VOS_FALSE == AT_PS_IsWlanThrotAllowed(pstCallEntity)))
    {
        AT_NORM_LOG("AT_PS_IsNeedPsCallDataSysSwitch: do not dailup in wlan because of throt.");
        return VOS_FALSE;
    }

    return VOS_TRUE;
}


VOS_UINT32 AT_PS_DialUpInWlanAllowed(
    VOS_UINT8                               ucBitDataSystem,
    TAF_PS_APN_DATA_SYS_POLICY_ENUM_UINT8   enDataSysPolicy
)
{
    VOS_UINT8                               ucWlanAvailableFlg;

    ucWlanAvailableFlg      = VOS_FALSE;

    ucWlanAvailableFlg      = (0x01 & (ucBitDataSystem >> TAFAGENT_DATA_SYSTEM_WLAN));

    /* Wlan不可用 */
    if (VOS_FALSE == ucWlanAvailableFlg)
    {
        return VOS_FALSE;
    }

    /* 域选策略不允许在Wlan下发起 */
    if ( (TAF_PS_APN_DATA_SYS_POLICY_ONLY_WLAN != enDataSysPolicy)
      && (TAF_PS_APN_DATA_SYS_POLICY_CELLULAR_PREFER != enDataSysPolicy))
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_PS_DialUpInCellularAllowed(
    VOS_UINT8                               ucBitDataSystem,
    TAF_PS_APN_DATA_SYS_POLICY_ENUM_UINT8   enDataSysPolicy
)
{
    VOS_UINT8                               ucCellularAvailableFlg;

    ucCellularAvailableFlg  = VOS_FALSE;

    ucCellularAvailableFlg  = (0x01 & (ucBitDataSystem >> TAFAGENT_DATA_SYSTEM_CELLULAR));

    /* CELLULAR不可用 */
    if (VOS_FALSE == ucCellularAvailableFlg)
    {
        return VOS_FALSE;
    }

    /* 域选策略不允许在CELLULAR下发起 */
    if ( (TAF_PS_APN_DATA_SYS_POLICY_CELLULAR_PREFER != enDataSysPolicy)
      && (TAF_PS_APN_DATA_SYS_POLICY_ONLY_CELLULAR != enDataSysPolicy))
    {
        return VOS_FALSE;
    }

    /* 当CELLULAR WLAN均可用，域选配置为WLAN PREFER时也不允许在CELLULAR下发起 */
    return VOS_TRUE;
}


VOS_UINT32 AT_PS_IsWlanThrotAllowed(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
)
{
    VOS_UINT32                          ulSetupAllowedFlg;

    ulSetupAllowedFlg = VOS_FALSE;

    if (VOS_OK != TAF_AGENT_GetWlanApnThrotAllowed(pstCallEntity->stUserInfo.enUserIndex,
                                                   pstCallEntity->stUsrDialParam.ucAPNLen,
                                                   pstCallEntity->stUsrDialParam.aucAPN,
                                                   &ulSetupAllowedFlg))
    {
        /* 查询调节参数失败 */
        AT_ERR_LOG("AT_PS_IsWlanThrotAllowed: Get wlan throt fail.");
        return VOS_FALSE;
    }

    if (VOS_TRUE != ulSetupAllowedFlg)
    {
       /* 不允许发起 */
       AT_ERR_LOG("AT_PS_IsWlanThrotAllowed: wlan is throttling.");
       return VOS_FALSE;
    }

    /* 允许发起 */
    return VOS_TRUE;
}


TAF_PDP_TYPE_ENUM_UINT8 AT_PS_ConvertPdpType2Cellular(IMSA_WIFI_IP_TYPE_ENUM_INT8 enWlanPdpType)
{
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType;

    switch (enWlanPdpType)
    {
        case IMSA_WIFI_IP_IPV4:
            enPdpType = TAF_PDP_IPV4;
            break;

        case IMSA_WIFI_IP_IPV6:
            enPdpType = TAF_PDP_IPV6;
            break;

        case IMSA_WIFI_IP_IPV4V6:
            enPdpType = TAF_PDP_IPV4V6;
            break;

        default:
            enPdpType = TAF_PDP_TYPE_BUTT;
            break;
    }

    return enPdpType;
}

IMSA_WIFI_IP_TYPE_ENUM_INT8 AT_PS_ConvertPdpType2Wlan( TAF_PDP_TYPE_ENUM_UINT8 eCellularPdpType)
{
    IMSA_WIFI_IP_TYPE_ENUM_INT8             enPdpType;

    switch (eCellularPdpType)
    {
        case TAF_PDP_IPV4 :
            enPdpType = IMSA_WIFI_IP_IPV4;
            break;

        case TAF_PDP_IPV6 :
            enPdpType = IMSA_WIFI_IP_IPV6;
            break;

        case TAF_PDP_IPV4V6:
            enPdpType = IMSA_WIFI_IP_IPV4V6;
            break;

        default:
            enPdpType = IMSA_WIFI_IP_TYPE_BUTT;
            break;
    }

    return enPdpType;
}


VOS_VOID AT_PS_ReportImsCtrlMsgu(
    VOS_UINT8                                       ucIndex,
    AT_IMS_CTRL_MSG_RECEIVE_MODULE_ENUM_UINT8       enModule,
    VOS_UINT32                                      ulMsgLen,
    VOS_UINT8                                      *pucDst
)
{
    /* 定义局部变量 */
    VOS_UINT16                          usLength;

    usLength = 0;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s^IMSCTRLMSGU: %d,%d,\"",
                                       gaucAtCrLf,
                                       enModule,
                                       ulMsgLen);

    usLength += (VOS_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_UINT8 *)pgucAtSndCodeAddr + usLength,
                                                    pucDst,
                                                    (VOS_UINT16)ulMsgLen);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "\"%s",
                                       gaucAtCrLf);

    /* 调用At_SendResultData发送命令结果 */
    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return;
}


VOS_VOID AT_PS_RegDataSysChgNtf(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
)
{
    TAF_PS_APN_DATA_SYS_POLICY_INFO_STRU    *pstApnDataSysPolicyInfo = VOS_NULL_PTR;

    /* 通过域选索引得到域选配置 */
    pstApnDataSysPolicyInfo = AT_PS_GetApnDataSysPolicyInfo(pstCallEntity->stUserInfo.enPortIndex, pstCallEntity->stApnDataSysInfo.ucDataSysPolicyIndex);

    /* 现在只有存在切换业务的APN才注册钩子 */
    if ( (VOS_TRUE == pstCallEntity->stApnDataSysInfo.ucDataSysInfoFlg)
      && (TAF_PS_APN_DATA_SYS_POLICY_CELLULAR_PREFER == pstApnDataSysPolicyInfo->stDataSysPolicy.enDataSysPolicy))
    {
        (VOS_VOID)TAF_AGENT_RegDataSysChgNtf(pstCallEntity->stUserInfo.enPortIndex, pstCallEntity->stUserInfo.ucUsrCid);
    }

    return;
}


VOS_VOID AT_PS_DeRegDataSysChgNtf(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId
)
{
    AT_PS_CALL_ENTITY_STRU                 *pstCallEntity           = VOS_NULL_PTR;
    TAF_PS_APN_DATA_SYS_POLICY_INFO_STRU   *pstApnDataSysPolicyInfo = VOS_NULL_PTR;

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);

    /* 通过域选索引得到域选配置 */
    pstApnDataSysPolicyInfo = AT_PS_GetApnDataSysPolicyInfo(usClientId, pstCallEntity->stApnDataSysInfo.ucDataSysPolicyIndex);

    /* 现在只有存在切换业务的APN才注册钩子，所以去注册也只有存在切换业务的APN */
    if ( (VOS_TRUE == pstCallEntity->stApnDataSysInfo.ucDataSysInfoFlg)
      && (TAF_PS_APN_DATA_SYS_POLICY_CELLULAR_PREFER == pstApnDataSysPolicyInfo->stDataSysPolicy.enDataSysPolicy))
    {
        (VOS_VOID)TAF_AGENT_DeRegDataSysChgNtf(usClientId, pstCallEntity->stUserInfo.ucUsrCid);
    }

    return;
}


VOS_VOID AT_PS_WlanPdpAddrProc(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    AT_DHCP_CONFIG_STRU                *pstDhcpConfig
)
{
    AT_DIAL_PARAM_STRU                 *pstDialPara     = VOS_NULL_PTR;
    VOS_UINT32                          ulIpAddr;

    pstDialPara   = &pstCallEntity->stUsrDialParam;
    ulIpAddr      = 0;

    pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4Addr              = pstDhcpConfig->ulIPAddr;
    pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4GateWay           = pstDhcpConfig->ulGateWay;
    pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4NetMask           = pstDhcpConfig->ulSubNetMask;

    /* 如果用户设置了主DNS，就使用用户设置的DNS，网络返回的DNS不使用 */
    if (VOS_TRUE == pstDialPara->ulPrimIPv4DNSValidFlag)
    {
        if (AT_FAILURE == AT_LenStr2IpAddr(pstDialPara->aucPrimIPv4DNSAddr,
                                           (VOS_UINT8*)&ulIpAddr))
        {
            ulIpAddr = 0;
        }

        pstCallEntity->stIpv4Info.stIpv4DhcpInfo.bitOpIpv4PriDns     = VOS_TRUE;
        pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4PrimDNS       = VOS_NTOHL(ulIpAddr);
    }
    else
    {
        if (0 != pstDhcpConfig->ulPrimDNS)
        {
            pstCallEntity->stIpv4Info.stIpv4DhcpInfo.bitOpIpv4PriDns = VOS_TRUE;
            pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4PrimDNS   = pstDhcpConfig->ulPrimDNS;
        }
        else
        {
            pstCallEntity->stIpv4Info.stIpv4DhcpInfo.bitOpIpv4PriDns = VOS_FALSE;
        }
    }

    /* 如果用户设置了副DNS，就使用用户设置的DNS，网络返回的DNS不使用 */
    if (VOS_TRUE == pstDialPara->ulSndIPv4DNSValidFlag)
    {
        if (AT_FAILURE == AT_LenStr2IpAddr(pstDialPara->aucSndIPv4DNSAddr,
                                           (VOS_UINT8*)&ulIpAddr))
        {
            ulIpAddr = 0;
        }

        pstCallEntity->stIpv4Info.stIpv4DhcpInfo.bitOpIpv4SecDns     = VOS_TRUE;
        pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4SecDNS        = VOS_NTOHL(ulIpAddr);
    }
    else
    {
        if (0 != pstDhcpConfig->ulSndDNS)
        {
            pstCallEntity->stIpv4Info.stIpv4DhcpInfo.bitOpIpv4SecDns = VOS_TRUE;
            pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4SecDNS    = pstDhcpConfig->ulSndDNS;
        }
        else
        {
            pstCallEntity->stIpv4Info.stIpv4DhcpInfo.bitOpIpv4SecDns = VOS_FALSE;
        }
    }

    pstCallEntity->stIpv4Info.stIpv4DhcpInfo.bitOpIpv4PriWINNS       = VOS_FALSE;
    pstCallEntity->stIpv4Info.stIpv4DhcpInfo.bitOpIpv4SecWINNS       = VOS_FALSE;

    /* 获取主PCSCF地址 */
    /* WLAN下的PCSCF地址先不考虑，故不保存 */
    pstCallEntity->stIpv4Info.stIpv4DhcpInfo.stIpv4PcscfList.ucIpv4PcscfAddrNum = 0;

    /* WLAN下usIpv4Mtu清除 */
    pstCallEntity->stIpv4Info.usIpv4Mtu   = 0;

    return;
}


VOS_VOID AT_PS_ProcWlanConnectedIpv4Addr(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    WLAN_AT_PDN_ACTIVATE_CNF_STRU      *pstWlanPdnActivateCnf
)
{
    AT_DHCP_SETUP_PARAM_ST              stParam;
    AT_DHCP_CONFIG_STRU                 stConfig;

    TAF_MEM_SET_S(&stParam, (VOS_UINT32)(sizeof(stParam)), 0x00, (VOS_UINT32)(sizeof(AT_DHCP_SETUP_PARAM_ST)));
    TAF_MEM_SET_S(&stConfig, (VOS_UINT32)(sizeof(stConfig)), 0x00, (VOS_UINT32)(sizeof(AT_DHCP_CONFIG_STRU)));

    /* 如果PS事件有DNS地址，则记录DNS地址*/
    if (pstWlanPdnActivateCnf->stDns.cOpPriDns)
    {
        stParam.ulPrimDNS = AT_GetLanAddr32(pstWlanPdnActivateCnf->stDns.aucPriDns);
    }

    if (pstWlanPdnActivateCnf->stDns.cOpSecDns)
    {
        stParam.ulSndDNS = AT_GetLanAddr32(pstWlanPdnActivateCnf->stDns.aucSecDns);
    }

    /* 记录IP地址*/
    stParam.ulIPAddr = AT_GetLanAddr32((VOS_UINT8 *)(pstWlanPdnActivateCnf->stPdpAddr.aucIpV4Addr));

    if (VOS_ERR == AT_DHCPServerSetUp(&stParam, &stConfig))
    {
        AT_ERR_LOG("AT_PS_ProcWlanConnectedIpv4Addr : AT_DHCPServerSetUp Error!" );
        return;
    }
    else
    {
        /* 处理IPv4地址参数 */
        AT_PS_WlanPdpAddrProc(pstCallEntity, &stConfig);
    }

    return;
}


VOS_VOID AT_PS_ProcWlanConnectedIpv6Addr(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    WLAN_AT_PDN_ACTIVATE_CNF_STRU      *pstWlanPdnActivateCnf
)
{
    /* 处理IPv6的IP地址，形式为网络序 */
    TAF_MEM_CPY_S(pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6Addr,
                  (VOS_UINT32)(sizeof(pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6Addr)),
                  pstWlanPdnActivateCnf->stPdpAddr.aucIpV6Addr,
                  TAF_IPV6_ADDR_LEN);

    /* 处理IPV6的主副DNS地址，形式为网络序 */
    pstCallEntity->stIpv6Info.stIpv6DhcpInfo.bitOpIpv6PriDns    = VOS_FALSE;
    pstCallEntity->stIpv6Info.stIpv6DhcpInfo.bitOpIpv6SecDns    = VOS_FALSE;

    if (VOS_TRUE == pstWlanPdnActivateCnf->stIPv6Dns.cOpPriDns)
    {
        TAF_MEM_CPY_S((VOS_VOID*)pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6PrimDNS,
                      (VOS_UINT32)(sizeof(pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6PrimDNS)),
                      pstWlanPdnActivateCnf->stIPv6Dns.aucPriDns,
                      IMSA_WIFI_IPV6_ADDR_LEN);
        pstCallEntity->stIpv6Info.stIpv6DhcpInfo.bitOpIpv6PriDns = VOS_TRUE;
    }

    if (VOS_TRUE == pstWlanPdnActivateCnf->stIPv6Dns.cOpSecDns)
    {
        TAF_MEM_CPY_S((VOS_VOID*)pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6SecDNS,
                      (VOS_UINT32)(sizeof(pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6SecDNS)),
                      pstWlanPdnActivateCnf->stIPv6Dns.aucSecDns,
                      IMSA_WIFI_IPV6_ADDR_LEN);
        pstCallEntity->stIpv6Info.stIpv6DhcpInfo.bitOpIpv6SecDns = VOS_TRUE;
    }

    /* 处理IPV6的主副PCSCF地址，形式为网络序 */
    /* WLAN下的PCSCF地址先不考虑，故不保存 */
    pstCallEntity->stIpv6Info.stIpv6DhcpInfo.stIpv6PcscfList.ucIpv6PcscfAddrNum = 0;

    /* WLAN下没有RA信息 */
    TAF_MEM_SET_S(&pstCallEntity->stIpv6Info.stIpv6RaInfo,
                  (VOS_UINT32)(sizeof(pstCallEntity->stIpv6Info.stIpv6RaInfo)),
                  0x00,
                  (VOS_UINT32)(sizeof(AT_IPV6_RA_INFO_STRU)));

    return;
}


VOS_VOID AT_PS_ProcWlanConnectedIpAddr(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    WLAN_AT_PDN_ACTIVATE_CNF_STRU      *pstWlanPdnActivateCnf
)
{
    /* 处理IP类型的DHCP */
    if ( (TAF_PDP_IPV4 == AT_PS_ConvertPdpType2Cellular(pstWlanPdnActivateCnf->enPdnType))
      || (TAF_PDP_IPV4V6 == AT_PS_ConvertPdpType2Cellular(pstWlanPdnActivateCnf->enPdnType)))
    {
        AT_PS_ProcWlanConnectedIpv4Addr(pstCallEntity, pstWlanPdnActivateCnf);
    }

    if ( (TAF_PDP_IPV6 == AT_PS_ConvertPdpType2Cellular(pstWlanPdnActivateCnf->enPdnType))
      || (TAF_PDP_IPV4V6 == AT_PS_ConvertPdpType2Cellular(pstWlanPdnActivateCnf->enPdnType)))
    {
        AT_PS_ProcWlanConnectedIpv6Addr(pstCallEntity, pstWlanPdnActivateCnf);
    }

    return;
}


VOS_VOID AT_PS_FillHoWlanPdnActivateIpv4Info(
    AT_WLAN_PDN_ACTIVATE_REQ_STRU           *pstPdnActReq,
    AT_PS_CALL_ENTITY_STRU                  *pstCallEntity

)
{
    AT_GetNwIpv4Addr(pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4Addr, (VOS_UINT8 *)(pstPdnActReq->stPdpAddr.aucIpV4Addr));

    if (VOS_TRUE == pstCallEntity->stIpv4Info.stIpv4DhcpInfo.bitOpIpv4PriDns)
    {
        pstPdnActReq->stDns.cOpPriDns = VOS_TRUE;
        AT_GetNwIpv4Addr(pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4PrimDNS, pstPdnActReq->stDns.aucPriDns);
    }

    if (VOS_TRUE == pstCallEntity->stIpv4Info.stIpv4DhcpInfo.bitOpIpv4SecDns)
    {
        pstPdnActReq->stDns.cOpSecDns = VOS_TRUE;
        AT_GetNwIpv4Addr(pstCallEntity->stIpv4Info.stIpv4DhcpInfo.ulIpv4SecDNS, pstPdnActReq->stDns.aucSecDns);
    }

    return;
}


VOS_VOID AT_PS_FillHoWlanPdnActivateIpv6Info(
    AT_WLAN_PDN_ACTIVATE_REQ_STRU           *pstPdnActReq,
    AT_PS_CALL_ENTITY_STRU                  *pstCallEntity

)
{
    TAF_MEM_CPY_S(pstPdnActReq->stPdpAddr.aucIpV6Addr,
                  TAF_IPV6_ADDR_LEN,
                  pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6Addr,
                  TAF_IPV6_ADDR_LEN);

    if (VOS_TRUE == pstCallEntity->stIpv6Info.stIpv6DhcpInfo.bitOpIpv6PriDns)
    {
        pstPdnActReq->stIPv6Dns.cOpPriDns = VOS_TRUE;

        TAF_MEM_CPY_S(pstPdnActReq->stIPv6Dns.aucPriDns,
                  TAF_IPV6_ADDR_LEN,
                  pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6PrimDNS,
                  TAF_IPV6_ADDR_LEN);
    }

    if (VOS_TRUE == pstCallEntity->stIpv6Info.stIpv6DhcpInfo.bitOpIpv6SecDns)
    {
        pstPdnActReq->stIPv6Dns.cOpSecDns = VOS_TRUE;

        TAF_MEM_CPY_S(pstPdnActReq->stIPv6Dns.aucSecDns,
                  TAF_IPV6_ADDR_LEN,
                  pstCallEntity->stIpv6Info.stIpv6DhcpInfo.aucIpv6SecDNS,
                  TAF_IPV6_ADDR_LEN);
    }

    return;
}


VOS_UINT32 AT_PS_SndWlanMsgPdnActivateReq(
    VOS_UINT8                           ucIndex,
    TAF_PDP_TYPE_ENUM_UINT8             enIpType,
    VOS_INT8                            cIsHandover,
    VOS_UINT8                           ucCallId
)
{
    AT_WLAN_PDN_ACTIVATE_REQ_STRU      *pstPdnActReq  = VOS_NULL_PTR;
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;
    VOS_UINT8                           ucBroadCastIndex;

    pstPdnActReq    = (AT_WLAN_PDN_ACTIVATE_REQ_STRU *)PS_MEM_ALLOC(WUEPS_PID_AT, sizeof(AT_WLAN_PDN_ACTIVATE_REQ_STRU)); //lint !e516

    if (VOS_NULL_PTR == pstPdnActReq)
    {
        AT_ERR_LOG("AT_PS_SndWlanMsgPdnActivateReq: Memory malloc failed!");
        return VOS_ERR;
    }

    pstCallEntity   = AT_PS_GetCallEntity(ucIndex, ucCallId);
    TAF_MEM_SET_S(pstPdnActReq, (VOS_UINT32)(sizeof(AT_WLAN_PDN_ACTIVATE_REQ_STRU)), 0x00, (VOS_UINT32)(sizeof(AT_WLAN_PDN_ACTIVATE_REQ_STRU)));

    pstPdnActReq->lMsgId        = ID_IMSA_WIFI_IMS_PDN_ACTIVATE_REQ;
    pstPdnActReq->enIpType      = AT_PS_ConvertPdpType2Wlan(enIpType);
    pstPdnActReq->cIsHandover   = cIsHandover;
    pstPdnActReq->cServiceType  = AT_PS_CALL_GET_SERVICE_TYPE_FROM_CID(pstCallEntity->stUsrDialParam.ucCid);

    pstPdnActReq->stApn.ucLength    = TAF_MIN(pstCallEntity->stUsrDialParam.ucAPNLen, TAF_MAX_APN_LEN);
    TAF_MEM_CPY_S(pstPdnActReq->stApn.aucValue,
                  (VOS_UINT32)(sizeof(pstPdnActReq->stApn.aucValue)),
                  pstCallEntity->stUsrDialParam.aucAPN,
                  pstPdnActReq->stApn.ucLength);

    if (VOS_TRUE == cIsHandover)
    {
        pstPdnActReq->stPdpAddr.enIpType = AT_PS_ConvertPdpType2Wlan(enIpType);

        if ( (TAF_PDP_IPV4 == enIpType)
          || (TAF_PDP_IPV4V6 == enIpType))
        {
            AT_PS_FillHoWlanPdnActivateIpv4Info(pstPdnActReq, pstCallEntity);
        }

        if ( (TAF_PDP_IPV6 == enIpType)
          || (TAF_PDP_IPV4V6 == enIpType))
        {
            AT_PS_FillHoWlanPdnActivateIpv6Info(pstPdnActReq, pstCallEntity);
        }
    }

    if (AT_SUCCESS == At_ClientIdToUserBroadCastId(ucIndex, &ucBroadCastIndex))
    {
        AT_PS_ReportImsCtrlMsgu(ucBroadCastIndex,
                                AT_IMS_CTRL_MSG_RECEIVE_MODULE_NON_IMSA,
                                (VOS_UINT32)(sizeof(AT_WLAN_PDN_ACTIVATE_REQ_STRU)),
                                (VOS_UINT8 *)pstPdnActReq);
    }

    PS_MEM_FREE(WUEPS_PID_AT, pstPdnActReq); //lint !e516

    return VOS_OK;
}


VOS_UINT32 AT_PS_SndWlanMsgPdnDeActivateReq(
    VOS_UINT8                           ucIndex,
    VOS_INT8                            cIsLocal,
    VOS_INT8                            cIsHandover,
    VOS_UINT8                           ucCallId
)
{
    AT_WLAN_PDN_DEACTIVATE_REQ_STRU    *pstPdnDeActReq     = VOS_NULL_PTR;
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity      = VOS_NULL_PTR;
    VOS_UINT8                           ucBroadCastIndex;

    pstPdnDeActReq  = (AT_WLAN_PDN_DEACTIVATE_REQ_STRU *)PS_MEM_ALLOC(WUEPS_PID_AT, sizeof(AT_WLAN_PDN_DEACTIVATE_REQ_STRU)); //lint !e516
    pstCallEntity   = AT_PS_GetCallEntity(ucIndex, ucCallId);

    if (VOS_NULL_PTR == pstPdnDeActReq)
    {
        AT_ERR_LOG("AT_PS_SndWlanMsgPdnDeActivateReq: Memory malloc failed!");
        return VOS_ERR;
    }

    TAF_MEM_SET_S(pstPdnDeActReq, (VOS_UINT32)(sizeof(AT_WLAN_PDN_DEACTIVATE_REQ_STRU)), 0x00, (VOS_UINT32)(sizeof(AT_WLAN_PDN_DEACTIVATE_REQ_STRU)));

    pstPdnDeActReq->lMsgId        = ID_IMSA_WIFI_IMS_PDN_DEACTIVATE_REQ;
    pstPdnDeActReq->cIsLocal      = cIsLocal;
    pstPdnDeActReq->cIsHandover   = cIsHandover;
    pstPdnDeActReq->cServiceType  = AT_PS_CALL_GET_SERVICE_TYPE_FROM_CID(pstCallEntity->stUsrDialParam.ucCid);

    pstPdnDeActReq->stApn.ucLength    = TAF_MIN(pstCallEntity->stUsrDialParam.ucAPNLen, TAF_MAX_APN_LEN);
    TAF_MEM_SET_S(pstPdnDeActReq->stApn.aucValue,
                  (VOS_UINT32)(sizeof(pstPdnDeActReq->stApn.aucValue)),
                  0x00,
                  (VOS_UINT32)(sizeof(pstPdnDeActReq->stApn.aucValue)));

    TAF_MEM_CPY_S(pstPdnDeActReq->stApn.aucValue,
                  (VOS_UINT32)(sizeof(pstPdnDeActReq->stApn.aucValue)),
                  pstCallEntity->stUsrDialParam.aucAPN,
                  pstPdnDeActReq->stApn.ucLength);

    if (AT_SUCCESS == At_ClientIdToUserBroadCastId(pstCallEntity->stUserInfo.enPortIndex, &ucBroadCastIndex))
    {
        AT_PS_ReportImsCtrlMsgu(ucBroadCastIndex,
                                AT_IMS_CTRL_MSG_RECEIVE_MODULE_NON_IMSA,
                                (VOS_UINT32)(sizeof(AT_WLAN_PDN_DEACTIVATE_REQ_STRU)),
                                (VOS_UINT8 *)pstPdnDeActReq);
    }

    PS_MEM_FREE(WUEPS_PID_AT, pstPdnDeActReq); //lint !e516

    return VOS_OK;
}


VOS_VOID AT_PS_CleanPsCallEntityWhenProcWlanMsg(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    TAF_PS_CAUSE_ENUM_UINT32            enCause
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity;
    TAF_PDP_TYPE_ENUM_UINT8             enWlanPdpType;

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);
    enWlanPdpType = pstCallEntity->stDialPdpType.enWlanPdpType;

    /* 将IP类型的PDP状态切换到IDLE */
    AT_PS_SetWlanCallStateByType(usClientId, ucCallId, enWlanPdpType, AT_PDP_STATE_IDLE);

    /* 设置Wlan呼叫PDP类型 */
    AT_PS_SetWlanCurrCallType(usClientId, ucCallId, TAF_PDP_TYPE_BUTT);

    /* 设置IP对应的CID为无效 */
    AT_PS_SetCid2CurrCall(usClientId, ucCallId, enWlanPdpType, AT_PS_CALL_INVALID_CID, TAF_PS_APN_DATA_SYS_WLAN);

    /* 最后清除CID与数传通道的映射关系 */
    AT_CleanAtChdataCfg(usClientId, pstCallEntity->stUserInfo.ucUsrCid, ucCallId, TAF_PS_APN_DATA_SYS_WLAN);

    /* 释放CALLID和CID的映射关系 */
    AT_PS_FreeCallIdToCid(usClientId, pstCallEntity->stUsrDialParam.ucCid, ucCallId, TAF_PS_APN_DATA_SYS_WLAN);

    /* 上报IP连接断开指示 */
    AT_PS_SndCallEndedResult(usClientId,
                             ucCallId,
                             enWlanPdpType,
                             enCause,
                             TAF_PS_APN_DATA_SYS_WLAN);

    /* 释放呼叫实体 */
    AT_PS_FreeCallEntity(usClientId, ucCallId, TAF_PS_APN_DATA_SYS_WLAN);

    return;
}


VOS_VOID AT_PS_ProcWlanDiffPdpTypeConn(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    WLAN_AT_PDN_ACTIVATE_CNF_STRU      *pstWlanPdnActivateCnf
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity;
    TAF_PDP_TYPE_ENUM_UINT8             enWlanPdpType;

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);
    enWlanPdpType = AT_PS_ConvertPdpType2Cellular(pstWlanPdnActivateCnf->enPdnType);

    AT_NORM_LOG2("AT_PS_ProcWlanDiffPdpTypeConn: enWlanPdpType and pstCallEntity->stDialPdpType.enWlanPdpType is ", enWlanPdpType, pstCallEntity->stDialPdpType.enWlanPdpType);

    /* 现在只存在请求是IPV4V6，回复成功可能是单栈的情况，不存在请求为IPV4，回复IPV6成功或者请求IPV6，回复IPV4成功的情况 */
    if ( (enWlanPdpType != pstCallEntity->stDialPdpType.enWlanPdpType)
      && (TAF_PDP_IPV4V6 == pstCallEntity->stDialPdpType.enWlanPdpType))
    {
        if (TAF_PDP_IPV4 == enWlanPdpType)
        {
            /* 设置IP对应的CID为无效 */
            AT_PS_SetCid2CurrCall(usClientId, ucCallId, TAF_PDP_IPV6, AT_PS_CALL_INVALID_CID, TAF_PS_APN_DATA_SYS_WLAN);

            /* 将IPV6类型的PDP状态切换到IDLE */
            AT_PS_SetWlanCallStateByType(usClientId, ucCallId, TAF_PDP_IPV6, AT_PDP_STATE_IDLE);

            /* 记录PS域呼叫错误码 */
            AT_PS_SetPsCallErrCause(usClientId, TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_IPV6_NOT_ALLOWED);

            /* 上报IPV6拨号失败 */
            AT_PS_SndCallEndedResult(usClientId,
                                     ucCallId,
                                     TAF_PDP_IPV6,
                                     TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_IPV6_NOT_ALLOWED,
                                     TAF_PS_APN_DATA_SYS_WLAN);
        }

        if (TAF_PDP_IPV6 == enWlanPdpType)
        {
            /* 设置IP对应的CID为无效 */
            AT_PS_SetCid2CurrCall(usClientId, ucCallId, TAF_PDP_IPV4, AT_PS_CALL_INVALID_CID, TAF_PS_APN_DATA_SYS_WLAN);

            /* 将IPV4类型的PDP状态切换到IDLE */
            AT_PS_SetWlanCallStateByType(usClientId, ucCallId, TAF_PDP_IPV4, AT_PDP_STATE_IDLE);

            /* 记录PS域呼叫错误码 */
            AT_PS_SetPsCallErrCause(usClientId, TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_IPV4_NOT_ALLOWED);

            /* 上报IPV4拨号失败 */
            AT_PS_SndCallEndedResult(usClientId,
                                     ucCallId,
                                     TAF_PDP_IPV4,
                                     TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_IPV4_NOT_ALLOWED,
                                     TAF_PS_APN_DATA_SYS_WLAN);
        }

        /* 更新WLAN下的PDN IP类型 */
        AT_PS_SetWlanCurrCallType(usClientId, ucCallId, enWlanPdpType);
    }

    return;
}


VOS_VOID AT_PS_ProcWlanCallConnected(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    WLAN_AT_PDN_ACTIVATE_CNF_STRU      *pstWlanPdnActivateCnf
)
{
    AT_PS_CALL_ENTITY_STRU                 *pstCallEntity           = VOS_NULL_PTR;
    TAF_PS_APN_DATA_SYS_POLICY_INFO_STRU   *pstApnDataSysPolicyInfo = VOS_NULL_PTR;
    VOS_UINT32                              ulTmrName;
    VOS_UINT32                              ulTmrParam;

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);

    /* 通过域选索引得到域选配置 */
    pstApnDataSysPolicyInfo = AT_PS_GetApnDataSysPolicyInfo(usClientId, pstCallEntity->stApnDataSysInfo.ucDataSysPolicyIndex);

    if (VOS_FALSE == AT_PS_GetPsCallHandOverFlg(pstCallEntity))
    {
        /* 将本IP类型PDP状态切换到激活状态 */
        AT_PS_SetWlanCallStateByType(usClientId, ucCallId, AT_PS_ConvertPdpType2Cellular(pstWlanPdnActivateCnf->enPdnType), AT_PDP_STATE_ACTED);

        /* 处理IP类型的DHCP */
        AT_PS_ProcWlanConnectedIpAddr(pstCallEntity, pstWlanPdnActivateCnf);

        /* 上报IP连接状态 */
        AT_PS_SndCallConnectedResult(usClientId, ucCallId, AT_PS_ConvertPdpType2Cellular(pstWlanPdnActivateCnf->enPdnType));

        /* 现在只存在请求是IPV4V6，回复成功可能是单栈的情况，不存在请求为IPV4，回复IPV6成功或者请求IPV6，回复IPV4成功的情况 */
        AT_PS_ProcWlanDiffPdpTypeConn(usClientId, ucCallId, pstWlanPdnActivateCnf);

        /* 将指定CID的PDP的激活状态设置为激活态 */
        AT_SetAtChdataCidActStatus(usClientId, ucCallId, pstCallEntity->stUserInfo.ucUsrCid);

        /* 注册数据系统域改变通知 */
        AT_PS_RegDataSysChgNtf(pstCallEntity);
    }
    else
    {
        /* 检查IP地址无变化 */
        if (VOS_TRUE == AT_PS_CmpIpv4v6AddrHoWlanConn(pstCallEntity, pstWlanPdnActivateCnf))
        {
            AT_PS_SetCallHandOverFlg(usClientId, ucCallId, VOS_FALSE);

            /* 设置handover呼叫类型 */
            AT_PS_SetHoCallType(usClientId, ucCallId, TAF_PDP_TYPE_BUTT);

            /* 处理IP类型的DHCP */
            AT_PS_ProcWlanConnectedIpAddr(pstCallEntity, pstWlanPdnActivateCnf);

            /* 将本IP类型PDP状态切换到激活状态 */
            AT_PS_SetWlanCallStateByType(usClientId, ucCallId, AT_PS_ConvertPdpType2Cellular(pstWlanPdnActivateCnf->enPdnType), AT_PDP_STATE_ACTED);

            /* 在CELLULAR下发起本地去激活 */
            (VOS_VOID)AT_PS_HangupCall(AT_PS_GetUserInfo(usClientId, ucCallId)->enUserIndex, ucCallId, TAF_PS_CALL_END_CAUSE_LOCAL);

            /* 启动防乒乓定时器 */
            AT_SET_PROTECT_PDN_IN_DATA_SYS_TMR_NAME(ulTmrName);
            AT_SET_PROTECT_PDN_IN_DATA_SYS_TMR_PARAM(ulTmrParam, usClientId, ucCallId);

            AT_StartRelTimer(&(pstCallEntity->stPsCallTimerInfo.hProtectInDataSysTmrHdl),
                             pstApnDataSysPolicyInfo->stDataSysPolicy.usWlanOutProtectTimeLen * 1000,
                             ulTmrName,
                             ulTmrParam,
                             VOS_RELTIMER_NOLOOP);
        }
        else
        {
            /* 记录IP地址改变的错误码 */
            AT_PS_SetPsCallErrCause(usClientId, TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_IP_ADDRESS_CHG_IN_HANDOVER);

            AT_PS_SetCallHandOverFlg(usClientId, ucCallId, VOS_FALSE);

            /* 设置handover呼叫类型 */
            AT_PS_SetHoCallType(usClientId, ucCallId, TAF_PDP_TYPE_BUTT);

            /* 发起CELLULAR本地去激活 */
            (VOS_VOID)AT_PS_HangupCall(AT_PS_GetUserInfo(usClientId, ucCallId)->enUserIndex, ucCallId, TAF_PS_CALL_END_CAUSE_LOCAL);

            /* 现在只存在请求是IPV4V6，回复成功可能是单栈的情况，不存在请求为IPV4，回复IPV6成功或者请求IPV6，回复IPV4成功的情况 */
            /* 该操作必须放在CELLULAR去激活操作后，WLAN去激活操作前 */
            AT_PS_ProcWlanDiffPdpTypeConn(usClientId, ucCallId, pstWlanPdnActivateCnf);

            AT_PS_SetCallHandOverFailCause(usClientId, ucCallId, TAF_PS_CAUSE_IP_ADDRESS_CHG_IN_HANDOVER);

            /* IP地址有变化，需要执行WLAN上去激活 */
            (VOS_VOID)AT_PS_HangupWlanCall(usClientId, ucCallId, VOS_FALSE, VOS_FALSE);
        }
    }

    return;
}


VOS_VOID AT_PS_ProcWlanCallReject(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    WLAN_AT_PDN_ACTIVATE_CNF_STRU      *pstWlanPdnActivateCnf
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;

    /* 记录PS域呼叫错误码 */
    AT_PS_SetPsCallErrCause(usClientId, AT_PS_MapWlanActCnfCause(pstWlanPdnActivateCnf->enCause));

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);

    if (VOS_TRUE == AT_PS_GetPsCallHandOverFlg(pstCallEntity))
    {
        AT_PS_SetCallHandOverFailCause(usClientId, ucCallId, AT_PS_GetPsCallErrCause(usClientId));

        AT_PS_SetCallHandOverFlg(usClientId, ucCallId, VOS_FALSE);

        /* 设置handover呼叫类型 */
        AT_PS_SetHoCallType(usClientId, ucCallId, TAF_PDP_TYPE_BUTT);

        /* 发起CELLULAR本地去激活 */
        (VOS_VOID)AT_PS_HangupCall(AT_PS_GetUserInfo(usClientId, ucCallId)->enUserIndex, ucCallId, TAF_PS_CALL_END_CAUSE_LOCAL);
    }

    /* CELLULAR下发起本地去激活后再清理全部变量 */
    AT_PS_CleanPsCallEntityWhenProcWlanMsg(usClientId, ucCallId, AT_PS_GetPsCallErrCause(usClientId));

    return;
}


VOS_VOID AT_PS_ProcWlanMsgPdnActivateCnf (
    VOS_UINT16                          usClientId,
    WLAN_AT_PDN_ACTIVATE_CNF_STRU      *pstWlanPdnActivateCnf
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity;
    VOS_UINT32                          ulTmrName;
    VOS_UINT8                           ucCallId;


    if (!AT_PS_IS_SERVICE_TYPE_VALID(pstWlanPdnActivateCnf->cServiceType))
    {
        AT_ERR_LOG("AT_PS_ProcWlanMsgPdnActivateCnf: cServiceType is invalid!");
        return;
    }

    ucCallId = AT_PS_TransCidToCallId(usClientId, AT_PS_CALL_GET_CID_FROM_SERVICE_TYPE(pstWlanPdnActivateCnf->cServiceType));

    if (VOS_FALSE == AT_PS_IsCallIdValid(usClientId, ucCallId))
    {
        AT_ERR_LOG("AT_PS_ProcWlanMsgPdnActivateCnf: CallId is invalid!");
        return;
    }

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);
    AT_SET_WLAN_ACT_PDN_CNF_TMR_NAME(ulTmrName);

    if (VOS_NULL_PTR != pstCallEntity->stPsCallTimerInfo.hWaitWlanActCnfTmrHdl)
    {
        AT_StopRelTimer(ulTmrName, &(pstCallEntity->stPsCallTimerInfo.hWaitWlanActCnfTmrHdl));
    }

    AT_NORM_LOG2("AT_PS_ProcWlanMsgPdnActivateCnf: enCause and lNetworkReason is ", pstWlanPdnActivateCnf->enCause, pstWlanPdnActivateCnf->lNetworkReason);

    if (WIFI_IMSA_PDN_ACT_RESULT_SUCCESS == pstWlanPdnActivateCnf->enCause)
    {
        /* 处理WLAN的PDP激活成功事件 */
        AT_PS_ProcWlanCallConnected(usClientId, ucCallId, pstWlanPdnActivateCnf);
    }
    else
    {
        /* 处理WLAN的PDP激活被拒事件 */
        AT_PS_ProcWlanCallReject(usClientId, ucCallId, pstWlanPdnActivateCnf);
    }

    TAF_AGENT_FeedbackWlanApnThrotResult(usClientId,
                                         pstCallEntity->stUsrDialParam.ucAPNLen,
                                         pstCallEntity->stUsrDialParam.aucAPN,
                                         (VOS_UINT32)(pstWlanPdnActivateCnf->enCause), /*lint !e571*/
                                         pstWlanPdnActivateCnf->lNetworkReason);

    return;
}


VOS_VOID AT_PS_ProcWlanMsgPdnDeactiveCnf (
    VOS_UINT16                          usClientId,
    WLAN_AT_PDN_DEACTIVATE_CNF_STRU    *pstWlanPdnDeActivateCnf
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity;
    VOS_UINT32                          ulTmrName;
    VOS_UINT8                           ucCallId;

    if (!AT_PS_IS_SERVICE_TYPE_VALID(pstWlanPdnDeActivateCnf->cServiceType))
    {
        AT_ERR_LOG("AT_PS_ProcWlanMsgPdnDeactiveCnf: cServiceType is invalid!");
        return;
    }

    ucCallId = AT_PS_TransCidToCallId(usClientId, AT_PS_CALL_GET_CID_FROM_SERVICE_TYPE(pstWlanPdnDeActivateCnf->cServiceType));

    if (VOS_FALSE == AT_PS_IsCallIdValid(usClientId, ucCallId))
    {
        AT_ERR_LOG("AT_PS_ProcWlanMsgPdnDeactiveCnf: CallId is invalid!");
        return;
    }

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);
    AT_SET_WLAN_DEACT_PDN_CNF_TMR_NAME(ulTmrName);

    if (VOS_NULL_PTR != pstCallEntity->stPsCallTimerInfo.hWaitWlanDeActCnfTmrHdl)
    {
        AT_StopRelTimer(ulTmrName, &(pstCallEntity->stPsCallTimerInfo.hWaitWlanDeActCnfTmrHdl));
    }

    AT_PS_CleanPsCallEntityWhenProcWlanMsg(usClientId, ucCallId, TAF_PS_CAUSE_SUCCESS);

    return;
}


VOS_VOID AT_PS_ProcWlanMsgPdnDeactiveInd (
    VOS_UINT16                          usClientId,
    WLAN_AT_PDN_DEACTIVATE_IND_STRU    *pstWlanPdnDeActivateInd
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity;
    VOS_UINT32                          ulTmrName;
    VOS_UINT8                           ucCallId;

    if (!AT_PS_IS_SERVICE_TYPE_VALID(pstWlanPdnDeActivateInd->cServiceType))
    {
        AT_ERR_LOG("AT_PS_ProcWlanMsgPdnDeactiveInd: cServiceType is invalid!");
        return;
    }

    ucCallId = AT_PS_TransCidToCallId(usClientId, AT_PS_CALL_GET_CID_FROM_SERVICE_TYPE(pstWlanPdnDeActivateInd->cServiceType));

    if (VOS_FALSE == AT_PS_IsCallIdValid(usClientId, ucCallId))
    {
        AT_ERR_LOG("AT_PS_ProcWlanMsgPdnDeactiveInd: CallId is invalid!");
        return;
    }

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);

    /* 停止激活CNF定时器 */
    AT_SET_WLAN_ACT_PDN_CNF_TMR_NAME(ulTmrName);

    if (VOS_NULL_PTR != pstCallEntity->stPsCallTimerInfo.hWaitWlanActCnfTmrHdl)
    {
        AT_StopRelTimer(ulTmrName, &(pstCallEntity->stPsCallTimerInfo.hWaitWlanActCnfTmrHdl));
    }

    /* 停止去激活CNF定时器 */
    AT_SET_WLAN_DEACT_PDN_CNF_TMR_NAME(ulTmrName);

    if (VOS_NULL_PTR != pstCallEntity->stPsCallTimerInfo.hWaitWlanDeActCnfTmrHdl)
    {
        AT_StopRelTimer(ulTmrName, &(pstCallEntity->stPsCallTimerInfo.hWaitWlanDeActCnfTmrHdl));
    }

    if (VOS_FALSE == AT_PS_GetPsCallHandOverFlg(pstCallEntity))
    {
        /* 记录PS域呼叫错误码 */
        AT_PS_SetPsCallErrCause(usClientId, AT_PS_MapWlanDeActIndCause(pstWlanPdnDeActivateInd->enCause));
    }

    AT_PS_CleanPsCallEntityWhenProcWlanMsg(usClientId, ucCallId, AT_PS_GetPsCallErrCause(usClientId));

    return;
}


VOS_UINT32 AT_PS_ProcMapconMsg (
    VOS_UINT16                          usClientId,
    AT_MAPCON_CTRL_MSG_STRU            *pstAtMapConMsgPara
)
{
    VOS_UINT32                         *pulMsgType;
    VOS_UINT32                          ulResult;

    pulMsgType  = (VOS_UINT32 *)(pstAtMapConMsgPara->ucMsgContext);
    ulResult    = VOS_OK;

    AT_NORM_LOG1("AT_PS_ProcMapconMsg: Msg Type is ", (*pulMsgType));

    switch (*pulMsgType)
    {
        case ID_WIFI_IMSA_IMS_PDN_ACTIVATE_CNF:
            AT_PS_ProcWlanMsgPdnActivateCnf(usClientId, (WLAN_AT_PDN_ACTIVATE_CNF_STRU*)&pstAtMapConMsgPara->ucMsgContext[0]);
            break;

        case ID_WIFI_IMSA_IMS_PDN_DEACTIVATE_CNF:
            AT_PS_ProcWlanMsgPdnDeactiveCnf(usClientId, (WLAN_AT_PDN_DEACTIVATE_CNF_STRU*)&pstAtMapConMsgPara->ucMsgContext[0]);
            break;

        case ID_WIFI_IMSA_IMS_PDN_DEACTIVATE_IND:
            AT_PS_ProcWlanMsgPdnDeactiveInd(usClientId, (WLAN_AT_PDN_DEACTIVATE_IND_STRU*)&pstAtMapConMsgPara->ucMsgContext[0]);
            break;

        default:
            AT_ERR_LOG("AT_PS_ProcMapconMsg: Msg Type is error!");
            ulResult = VOS_ERR;
            break;
    }

    return ulResult;
}


VOS_VOID AT_PS_HoWlanCall(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    VOS_UINT8                           ucCallId
)
{
    AT_DIAL_PARAM_STRU                  stUsrDialParam;
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType;

    TAF_MEM_CPY_S(&stUsrDialParam, (VOS_UINT32)(sizeof(stUsrDialParam)), &(pstCallEntity->stUsrDialParam), (VOS_UINT32)(sizeof(pstCallEntity->stUsrDialParam)));
    enPdpType = AT_PS_GetHoPdpIpType(pstCallEntity);

    /* 发起在wlan下建立承载(handover) */
    AT_PS_SetCallHandOverFlg(pstCallEntity->stUserInfo.enPortIndex, ucCallId, VOS_TRUE);

    /* 设置handover呼叫类型 */
    AT_PS_SetHoCallType(pstCallEntity->stUserInfo.enPortIndex, ucCallId, enPdpType);

    stUsrDialParam.enPdpType = enPdpType;

    AT_NORM_LOG1("AT_PS_HoWlanCall: act pdp in wlan Pdp Type is ", enPdpType);

    if (VOS_OK != AT_PS_SetupWlanCall(&stUsrDialParam, pstCallEntity->stUserInfo.enPortIndex, ucCallId))
    {
        /* 切换失败, 需要在CELLULAR下执行本地去激活操作 */
        AT_PS_SetCallHandOverFlg(pstCallEntity->stUserInfo.enPortIndex, ucCallId, VOS_FALSE);

        AT_PS_SetCallHandOverFailCause(pstCallEntity->stUserInfo.enPortIndex, ucCallId, TAF_PS_CAUSE_UNKNOWN);

        /* 设置handover呼叫类型 */
        AT_PS_SetHoCallType(pstCallEntity->stUserInfo.enPortIndex, ucCallId, TAF_PDP_TYPE_BUTT);

        AT_PS_SetPsCallErrCause(pstCallEntity->stUserInfo.enPortIndex, TAF_PS_CAUSE_UNKNOWN);

        (VOS_VOID)AT_PS_HangupCall(pstCallEntity->stUserInfo.enUserIndex, ucCallId, TAF_PS_CALL_END_CAUSE_LOCAL);
    }

    return;
}


VOS_VOID AT_PS_HoCellularCall(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    VOS_UINT8                           ucCallId
)
{
    AT_DIAL_PARAM_STRU                  stUsrDialParam;
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType;

    TAF_MEM_CPY_S(&stUsrDialParam,
                  (VOS_UINT32)(sizeof(stUsrDialParam)),
                  &pstCallEntity->stUsrDialParam,
                  (VOS_UINT32)(sizeof(pstCallEntity->stUsrDialParam)));

    enPdpType = AT_PS_GetHoPdpIpType(pstCallEntity);

    /* 发起在cellular下建立承载(handover) */
    AT_PS_SetCallHandOverFlg(pstCallEntity->stUserInfo.enPortIndex, ucCallId, VOS_TRUE);

    /* 设置handover呼叫类型 */
    AT_PS_SetHoCallType(pstCallEntity->stUserInfo.enPortIndex, ucCallId, enPdpType);

    stUsrDialParam.enPdpType = enPdpType;

    AT_NORM_LOG1("AT_PS_HoCellularCall: act pdp in cellular Pdp Type is ", enPdpType);

    if (VOS_OK != AT_PS_SetupCall(pstCallEntity->stUserInfo.enUserIndex, ucCallId, &stUsrDialParam))
    {
        /* 切换失败, 需要在WLAN下执行本地去激活操作 */
        AT_PS_SetCallHandOverFlg(pstCallEntity->stUserInfo.enPortIndex, ucCallId, VOS_FALSE);

        AT_PS_SetCallHandOverFailCause(pstCallEntity->stUserInfo.enPortIndex, ucCallId, TAF_PS_CAUSE_UNKNOWN);

        /* 设置handover呼叫类型 */
        AT_PS_SetHoCallType(pstCallEntity->stUserInfo.enPortIndex, ucCallId, TAF_PDP_TYPE_BUTT);

        AT_PS_SetPsCallErrCause(pstCallEntity->stUserInfo.enPortIndex, TAF_PS_CAUSE_UNKNOWN);

        (VOS_VOID)AT_PS_HangupWlanCall(pstCallEntity->stUserInfo.enUserIndex, ucCallId, VOS_TRUE, VOS_TRUE);
    }

    return;
}


VOS_VOID AT_PS_HoCall(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    VOS_UINT8                           ucCallId,
    TAF_PS_APN_DATA_SYS_ENUM_UINT8      enDestDataSys
)
{
    switch(enDestDataSys)
    {
        case TAF_PS_APN_DATA_SYS_WLAN:
            AT_PS_HoWlanCall(pstCallEntity, ucCallId);
            break;

        case TAF_PS_APN_DATA_SYS_CELLULAR:
            AT_PS_HoCellularCall(pstCallEntity, ucCallId);
            break;

        default:
            /* nothing to do */
            AT_ERR_LOG1("AT_PS_HoCall: enDestDataSys is error, is !", enDestDataSys);
            break;
    }

    return;
}


VOS_VOID AT_PS_ProcWlanPowerOff (
    VOS_UINT16                          usClientId
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity;
    WLAN_AT_PDN_DEACTIVATE_IND_STRU     stWlanPdnDeActivateInd;
    VOS_UINT32                          i;

    for (i = 0; i < AT_PS_MAX_CALL_NUM; i++)
    {
        pstCallEntity = AT_PS_GetCallEntity(usClientId, (VOS_UINT8)i);

        TAF_MEM_SET_S(&stWlanPdnDeActivateInd,
                      (VOS_UINT32)(sizeof(stWlanPdnDeActivateInd)),
                      0x00,
                      (VOS_UINT32)(sizeof(WLAN_AT_PDN_DEACTIVATE_IND_STRU)));

        if ( (VOS_TRUE == pstCallEntity->ulUsedFlg)
          && ( (AT_PDP_STATE_IDLE != pstCallEntity->stIpv4Info.enWlanIpv4State)
            || (AT_PDP_STATE_IDLE != pstCallEntity->stIpv6Info.enWlanIpv6State)))
        {
            /* 按wlan上报DeactiveInd处理 */
            stWlanPdnDeActivateInd.cServiceType     = AT_PS_CALL_GET_SERVICE_TYPE_FROM_CID(pstCallEntity->stUsrDialParam.ucCid);
            stWlanPdnDeActivateInd.enCause          = WIFI_IMSA_PDN_DEACT_CAUSE_MAPCON_TEAR;
            stWlanPdnDeActivateInd.stApn.ucLength   = TAF_MIN(pstCallEntity->stUsrDialParam.ucAPNLen, TAF_MAX_APN_LEN);

            TAF_MEM_CPY_S(stWlanPdnDeActivateInd.stApn.aucValue,
                          (VOS_UINT32)(sizeof(stWlanPdnDeActivateInd.stApn.aucValue)),
                          pstCallEntity->stUsrDialParam.aucAPN,
                          (VOS_UINT32)(stWlanPdnDeActivateInd.stApn.ucLength));

            AT_PS_ProcWlanMsgPdnDeactiveInd(usClientId, &stWlanPdnDeActivateInd);
        }
    }

    return;
}


VOS_VOID AT_PS_PreProcPsCallDataSysChgNtf(
    VOS_UINT16                   usClientId,
    VOS_UINT32                   ulBitDataSysSwitchCid
)
{
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;
    VOS_UINT32                          i;
    TAF_PS_APN_DATA_SYS_ENUM_UINT8      enDestDataSys;

    pstPsModemCtx   = AT_GetModemPsCtxAddrFromClientId(usClientId);
    enDestDataSys   = TAF_PS_APN_DATA_SYS_NONE;

    for (i = 0; i < AT_PS_MAX_CALL_NUM; i++)
    {
        if ( (0 != ((0x01 << pstPsModemCtx->astCallEntity[i].stUsrDialParam.ucCid) & ulBitDataSysSwitchCid))
          && (VOS_TRUE == pstPsModemCtx->astCallEntity[i].ulUsedFlg)
          && (VOS_TRUE == pstPsModemCtx->astCallEntity[i].stApnDataSysInfo.ucDataSysInfoFlg))
        {
            if (VOS_TRUE == AT_PS_IsNeedPsCallDataSysSwitch(usClientId, &pstPsModemCtx->astCallEntity[i], &enDestDataSys))
            {
                /* 满足切换条件，执行切换 */
                AT_PS_HoCall(&pstPsModemCtx->astCallEntity[i], (VOS_UINT8)i, enDestDataSys);
            }
            else
            {
                /* 不满足切换条件，无任何处理 */
                /* nothing to do */
            }
        }
    }

    return;
}

VOS_VOID AT_PS_RcvTiWlanActPdnCnfExpired(REL_TIMER_MSG *pstTmrMsg)
{
    WLAN_AT_PDN_ACTIVATE_CNF_STRU       stWlanPdnActivateCnf;
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity   = VOS_NULL_PTR;
    VOS_UINT8                           ucCallId;
    VOS_UINT8                           ucIndex;

    ucCallId       = AT_GET_WLAN_ACT_PDN_CNF_CALLID_FROM_TMR_PARAM(pstTmrMsg->ulPara);
    ucIndex        = AT_GET_WLAN_ACT_PDN_CNF_CLIENTID_FROM_TMR_PARAM(pstTmrMsg->ulPara);

    if (VOS_FALSE == AT_PS_IsCallIdValid(ucIndex, ucCallId))
    {
        AT_ERR_LOG("AT_PS_RcvTiWlanActPdnCnfExpired: CallId is invalid!");
        return;
    }

    pstCallEntity   = AT_PS_GetCallEntity(ucIndex, ucCallId);

    TAF_MEM_SET_S(&stWlanPdnActivateCnf, (VOS_UINT32)(sizeof(stWlanPdnActivateCnf)), 0x00, (VOS_UINT32)(sizeof(WLAN_AT_PDN_ACTIVATE_CNF_STRU)));

    stWlanPdnActivateCnf.enCause    = WIFI_IMSA_PDN_ACT_RESULT_TIMEOUT;
    stWlanPdnActivateCnf.enPdnType  = AT_PS_ConvertPdpType2Wlan(pstCallEntity->stDialPdpType.enWlanPdpType);

    AT_PS_ProcWlanCallReject(ucIndex, ucCallId, &stWlanPdnActivateCnf);

    TAF_AGENT_FeedbackWlanApnThrotResult(ucIndex,
                                         pstCallEntity->stUsrDialParam.ucAPNLen,
                                         pstCallEntity->stUsrDialParam.aucAPN,
                                         WIFI_IMSA_PDN_ACT_RESULT_TIMEOUT,
                                         0);

    return;
}


VOS_VOID AT_PS_RcvTiWlanDeActPdnCnfExpired(REL_TIMER_MSG *pstTmrMsg)
{
    WLAN_AT_PDN_DEACTIVATE_CNF_STRU     stWlanPdnDeActivateCnf;
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity   = VOS_NULL_PTR;
    VOS_UINT8                           ucCallId;
    VOS_UINT8                           ucIndex;

    ucCallId       = AT_GET_WLAN_DEACT_PDN_CNF_CALLID_FROM_TMR_PARAM(pstTmrMsg->ulPara);
    ucIndex        = AT_GET_WLAN_DEACT_PDN_CNF_CLIENTID_FROM_TMR_PARAM(pstTmrMsg->ulPara);

    if (VOS_FALSE == AT_PS_IsCallIdValid(ucIndex, ucCallId))
    {
        AT_ERR_LOG("AT_PS_RcvTiWlanDeActPdnCnfExpired: CallId is invalid!");
        return;
    }

    pstCallEntity   = AT_PS_GetCallEntity(ucIndex, ucCallId);

    TAF_MEM_SET_S(&stWlanPdnDeActivateCnf, (VOS_UINT32)(sizeof(stWlanPdnDeActivateCnf)), 0x00, (VOS_UINT32)(sizeof(WLAN_AT_PDN_DEACTIVATE_CNF_STRU)));

    stWlanPdnDeActivateCnf.cServiceType    = AT_PS_CALL_GET_SERVICE_TYPE_FROM_CID(pstCallEntity->stUsrDialParam.ucCid);

    AT_PS_ProcWlanMsgPdnDeactiveCnf(ucIndex, &stWlanPdnDeActivateCnf);

    return;
}


VOS_VOID AT_PS_RcvTiProtectPdnInDataSysExpired(REL_TIMER_MSG *pstTmrMsg)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity   = VOS_NULL_PTR;
    VOS_UINT32                          ulBitDataSysSwitchCid;
    VOS_UINT8                           ucCallId;
    VOS_UINT8                           ucIndex;

    ucCallId       = AT_GET_PROTECT_PDN_IN_DATA_SYS_CALLID_FROM_TMR_PARAM(pstTmrMsg->ulPara);
    ucIndex        = AT_GET_PROTECT_PDN_IN_DATA_SYS_CLIENTID_FROM_TMR_PARAM(pstTmrMsg->ulPara);

    if (VOS_FALSE == AT_PS_IsCallIdValid(ucIndex, ucCallId))
    {
        AT_ERR_LOG("AT_PS_RcvTiProtectPdnInDataSysExpired: CallId is invalid!");
        return;
    }

    pstCallEntity   = AT_PS_GetCallEntity(ucIndex, ucCallId);

    /* 执行切换逻辑 */
    ulBitDataSysSwitchCid = 0x01 << pstCallEntity->stUsrDialParam.ucCid;
    AT_PS_PreProcPsCallDataSysChgNtf(ucIndex, ulBitDataSysSwitchCid);

    return;
}


