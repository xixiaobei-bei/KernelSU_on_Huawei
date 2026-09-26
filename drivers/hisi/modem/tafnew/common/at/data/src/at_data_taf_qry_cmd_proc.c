/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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

#include "at_data_taf_qry_cmd_proc.h"
#include "at_data_comm.h"
#include "securec.h"

#include "at_mdrv_interface.h"
#include "ppp_interface.h"
#include "taf_drv_agent.h"
#include "taf_tafm_remote.h"
#include "ttf_mem_at_interface.h"
#include "at_cmd_proc.h"
#include "at_check_func.h"
#include "at_parse_cmd.h"
#include "at_phy_interface.h"
#include "at_data_proc.h"
#include "at_cmd_msg_proc.h"
#include "at_snd_msg.h"
#include "at_rnic_interface.h"
#include "at_input_proc.h"
#include "at_event_report.h"
#include "at_device_cmd.h"
#include "at_taf_agent_interface.h"
#include "app_vc_api.h"
#include "taf_oam_interface.h"
#include "taf_iface_api.h"
#include "at_lte_eventreport.h"
#include "at_ctx_packet.h"

#if (FEATURE_LTE == FEATURE_ON)
#include "mdrv_nvim.h"
#include "at_common.h"
#include "at_lte_common.h"
#endif

#include "nv_stru_sys.h"
#include "nv_stru_gas.h"
#include "nv_stru_msp_interface.h"
#include "nv_stru_msp.h"

#include "css_at_interface.h"

#include "at_msg_print.h"

#include "ps_common_def.h"
#include "ps_lib.h"
#include "product_config.h"
#include "taf_ccm_api.h"
#include "mn_comm_api.h"
#if (FEATURE_LTEV == FEATURE_ON)
#include "vnas_at_interface.h"
#endif

#if (VOS_OS_VER == VOS_WIN32)
#include "NasStub.h"
#endif
#include "throt_at_interface.h"



/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_DATA_TAF_QRY_CMD_PROC_C

#define AT_PS_CALL_TYPE_STICK 0

VOS_UINT32 AT_QryCgeqreqPara(VOS_UINT8 indexNum)
{
    /* 执行命令操作 */
    if (TAF_PS_GetUmtsQosInfo(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0) != VOS_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGEQREQ_READ;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_TestCgeqreqPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length;

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: \"IP\",%s%s", g_parseContext[indexNum].cmdElement->cmdName,
        CGEQREQ_TEST_CMD_PARA_STRING, g_atCrLf);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: \"IPV6\",%s%s", g_parseContext[indexNum].cmdElement->cmdName,
        CGEQREQ_TEST_CMD_PARA_STRING, g_atCrLf);


    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: \"IPV4V6\",%s", g_parseContext[indexNum].cmdElement->cmdName,
        CGEQREQ_TEST_CMD_PARA_STRING);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 At_QryCgeqminPara(VOS_UINT8 indexNum)
{
    /* 执行命令操作 */
    if (TAF_PS_GetUmtsQosMinInfo(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0) != VOS_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGEQMIN_READ;

    return AT_WAIT_ASYNC_RETURN;
}

TAF_UINT32 AT_QryCgtftPara(TAF_UINT8 indexNum)
{
    /* 执行命令操作 */
    if (TAF_PS_GetTftInfo(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0) != VOS_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGTFT_READ;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_TestCgtft(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;

    if (AT_IsSupportReleaseRst(AT_ACCESS_STRATUM_REL11)) {
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress,
            "%s: \"IP\",(1-16),(0-255),,(0-255),,,(00000000-FFFFFFFF),,(00000-FFFFF),(0-3),,(0-255)\r\n",
            g_parseContext[indexNum].cmdElement->cmdName);
#else
        length +=
            (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress,
                "%s: \"IP\",(1-16),(0-255),,(0-255),,,(00000000-FFFFFFFF),,(00000-FFFFF),(0-3),\r\n",
                g_parseContext[indexNum].cmdElement->cmdName);
#endif
    } else {
        length +=
            (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress,
                "%s: \"IP\",(1-16),(0-255),,(0-255),,,(00000000-FFFFFFFF),,(00000-FFFFF),(0-3)\r\n",
                g_parseContext[indexNum].cmdElement->cmdName);
    }

    if (AT_GetIpv6Capability() != AT_IPV6_CAPABILITY_IPV4_ONLY) {
        if (AT_IsSupportReleaseRst(AT_ACCESS_STRATUM_REL11)) {
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length,
                "%s: \"IPV6\",(1-16),(0-255),,(0-255),,,(00000000-FFFFFFFF),,(00000-FFFFF),(0-3),,(0-255)\r\n",
                g_parseContext[indexNum].cmdElement->cmdName);
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length,
                "%s: \"IPV4V6\",(1-16),(0-255),,(0-255),,,(00000000-FFFFFFFF),,(00000-FFFFF),(0-3),,(0-255)",
                g_parseContext[indexNum].cmdElement->cmdName);
#else
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length,
                "%s: \"IPV6\",(1-16),(0-255),,(0-255),,,(00000000-FFFFFFFF),,(00000-FFFFF),(0-3),\r\n",
                g_parseContext[indexNum].cmdElement->cmdName);
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length,
                "%s: \"IPV4V6\",(1-16),(0-255),,(0-255),,,(00000000-FFFFFFFF),,(00000-FFFFF),(0-3),",
                g_parseContext[indexNum].cmdElement->cmdName);
#endif
        } else {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length,
                "%s: \"IPV6\",(1-16),(0-255),,(0-255),,,(00000000-FFFFFFFF),,(00000-FFFFF),(0-3)\r\n",
                g_parseContext[indexNum].cmdElement->cmdName);
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length,
                "%s: \"IPV4V6\",(1-16),(0-255),,(0-255),,,(00000000-FFFFFFFF),,(00000-FFFFF),(0-3)",
                g_parseContext[indexNum].cmdElement->cmdName);
        }
    }

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

TAF_UINT32 AT_QryCgautoPara(TAF_UINT8 indexNum)
{
    /* 执行命令操作 */
    if (TAF_PS_GetAnsModeInfo(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0) != VOS_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGAUTO_READ;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_TestCgautoPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: (0,1)", g_parseContext[indexNum].cmdElement->cmdName);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 At_TestCgpaddr(VOS_UINT8 indexNum)
{
    /* 执行命令操作 */
    if (TAF_PS_GetPdpContextInfo(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0) != VOS_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGPADDR_TEST;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryCgdcontPara(VOS_UINT8 indexNum)
{
    /* 执行命令操作 */
    if (TAF_PS_GetPrimPdpContextInfo(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0) !=
        VOS_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGDCONT_READ;

    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)

LOCAL VOS_UINT16 At_TestCgdcontNrMode(VOS_UINT8 indexNum)
{
    VOS_UINT16 length;
    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress,
        "%s: (0-31),\"IP\",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1),,,,,,(0-2),,(0,1),(0,1),(0,1),(0,1)\r\n",
        g_parseContext[indexNum].cmdElement->cmdName);

    if (AT_GetIpv6Capability() != AT_IPV6_CAPABILITY_IPV4_ONLY) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length,
            "%s: (0-31),\"IPV6\",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1),,,,,,(0-2),,(0,1),(0,1),(0,1),(0,1)\r\n",
            g_parseContext[indexNum].cmdElement->cmdName);
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length,
            "%s: (0-31),\"IPV4V6\",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1),,,,,,(0-2),,(0,1),(0,1),(0,1),(0,1)\r\n",
            g_parseContext[indexNum].cmdElement->cmdName);
    }

    if (AT_GetEthernetCap() == VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length,
            "%s: (0-31),\"PPP\",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1),,,,,,(0-2),,(0,1),(0,1),(0,1),(0,1)\r\n",
            g_parseContext[indexNum].cmdElement->cmdName);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length,
            "%s: (0-31),\"Ethernet\",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1),,,,,,(0-2),,(0,1),(0,1),(0,1),(0,1)",
            g_parseContext[indexNum].cmdElement->cmdName);
    }
    else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length,
            "%s: (0-31),\"PPP\",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1),,,,,,(0-2),,(0,1),(0,1),(0,1),(0,1)",
            g_parseContext[indexNum].cmdElement->cmdName);
    }
    return length;
}
#endif

VOS_UINT32 At_TestCgdcont(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;

#if (FEATURE_LTE == FEATURE_ON)
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    length = At_TestCgdcontNrMode(indexNum);
#else
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0-31),\"IP\",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1)\r\n",
        g_parseContext[indexNum].cmdElement->cmdName);

    if (AT_GetIpv6Capability() != AT_IPV6_CAPABILITY_IPV4_ONLY) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length,
            "%s: (0-31),\"IPV6\",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1)\r\n",
            g_parseContext[indexNum].cmdElement->cmdName);
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length,
            "%s: (0-31),\"IPV4V6\",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1)\r\n",
            g_parseContext[indexNum].cmdElement->cmdName);
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: (0-31),\"PPP\",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1)",
        g_parseContext[indexNum].cmdElement->cmdName);
#endif
#else
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (1-11),\"IP\",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1)\r\n",
        g_parseContext[indexNum].cmdElement->cmdName);

    if (AT_GetIpv6Capability() != AT_IPV6_CAPABILITY_IPV4_ONLY) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length,
            "%s: (1-11),\"IPV6\",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1)\r\n",
            g_parseContext[indexNum].cmdElement->cmdName);
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length,
            "%s: (1-11),\"IPV4V6\",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1)\r\n",
            g_parseContext[indexNum].cmdElement->cmdName);
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: (1-11),\"PPP\",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1)",
        g_parseContext[indexNum].cmdElement->cmdName);

#endif

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_QryCgdscontPara(VOS_UINT8 indexNum)
{
    /* 执行命令操作 */
    if (TAF_PS_GetSecPdpContextInfo(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0) !=
        VOS_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGDSCONT_READ;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_TestCgdscont(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;

#if (FEATURE_LTE == FEATURE_ON)
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (1-31),(0-31),(0-2),(0-3),(0-1)",
        g_parseContext[indexNum].cmdElement->cmdName);
#else
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (1-11),(0-11),(0-2),(0-3),(0-1)",
        g_parseContext[indexNum].cmdElement->cmdName);

#endif

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

TAF_UINT32 AT_QryCgactPara(TAF_UINT8 indexNum)
{
    /* 执行命令操作 */
    if (TAF_PS_GetPdpContextState(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0) != VOS_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGACT_READ;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_TestCgact(VOS_UINT8 indexNum)
{
    VOS_UINT16 length;

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0,1)", g_parseContext[indexNum].cmdElement->cmdName);
    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 At_TestCgcmod(VOS_UINT8 indexNum)
{

    /* CGEQNEG的测试命令和CGCMODE的返回值相同，使用相同的函数 */
    return At_TestCgeqnegPara(indexNum);
}

VOS_UINT32 At_TestCgansPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: (0,1),(\"NULL\")", g_parseContext[indexNum].cmdElement->cmdName);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 At_TestCgansExtPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: (0,1)", g_parseContext[indexNum].cmdElement->cmdName);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 At_TestCgdataPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: (\"NULL\")", g_parseContext[indexNum].cmdElement->cmdName);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

TAF_UINT32 AT_QryCgdnsPara(TAF_UINT8 indexNum)
{
    /* 执行命令操作 */
    if (TAF_PS_GetPdpDnsInfo(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0) != VOS_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGDNS_READ;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_TestCgdnsPara(VOS_UINT8 indexNum)
{

    /* CGDNS的测试命令和CGCMODE的返回值相同，使用相同的函数 */
    return At_TestCgeqnegPara(indexNum);
}

TAF_UINT32 At_QryGlastErrPara(TAF_UINT8 indexNum)
{
    TAF_UINT16 length;
    TAF_UINT32 atErrType;

    atErrType = AT_GetErrType();
    /* 将错误码上报给后台 */
    length = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s:", g_parseContext[indexNum].cmdElement->cmdName);
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%d", atErrType);
    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_SndQryFastDorm(VOS_UINT16 clientId, VOS_UINT8 opId)
{
    AT_RABM_QryFastdormParaReq *sndMsg = VOS_NULL_PTR;
    VOS_UINT32                  rslt;

    /* 申请内存  */
    /*lint -save -e516 */
    sndMsg = (AT_RABM_QryFastdormParaReq *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT,
                                                        sizeof(AT_RABM_QryFastdormParaReq) - VOS_MSG_HEAD_LENGTH);
    /*lint -restore */
    if (sndMsg == VOS_NULL_PTR) {
        /* 内存申请失败 */
        AT_ERR_LOG("AT_SndSetFastDorm:ERROR: Memory Alloc Error for pstMsg");
        return VOS_ERR;
    }

    /* 填写相关参数 */
    TAF_CfgMsgHdr((MsgBlock *)sndMsg, WUEPS_PID_AT, AT_GetDestPid(clientId, I0_WUEPS_PID_RABM),
                  sizeof(AT_RABM_QryFastdormParaReq) - VOS_MSG_HEAD_LENGTH);

    sndMsg->msgHeader.msgName = ID_AT_RABM_QRY_FASTDORM_PARA_REQ;
    sndMsg->clientId          = clientId;
    sndMsg->opId              = opId;

    /* 调用VOS发送原语 */
    rslt = TAF_TraceAndSendMsg(WUEPS_PID_AT, sndMsg);
    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_SndSetFastDorm:ERROR:TAF_TraceAndSendMsg ");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_QryFastDormPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rslt;

    /* 调用MN_FillAndSndAppReqMsg()，等待RABM的回复 */
    rslt = AT_SndQryFastDorm(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);
    if (rslt == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_FASTDORM_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 At_TestCgcontrdp(VOS_UINT8 indexNum)
{
    return At_TestCgeqnegPara(indexNum);
}

VOS_UINT32 At_TestCgscontrdp(VOS_UINT8 indexNum)
{
    return At_TestCgeqnegPara(indexNum);
}

VOS_UINT32 At_TestCgtftrdp(VOS_UINT8 indexNum)
{
    return At_TestCgeqnegPara(indexNum);
}

VOS_UINT32 AT_QryIpv6CapPara(VOS_UINT8 indexNum)
{
    VOS_UINT8 ipv6Capability;

    switch (AT_GetIpv6Capability()) {
        case AT_IPV6_CAPABILITY_IPV4_ONLY:
            ipv6Capability = AT_IPV6_CAPABILITY_IPV4_ONLY;
            break;

        case AT_IPV6_CAPABILITY_IPV6_ONLY:
            ipv6Capability = AT_IPV6_CAPABILITY_IPV6_ONLY;
            break;

        case AT_IPV6_CAPABILITY_IPV4V6_OVER_ONE_PDP:
            ipv6Capability = AT_IPV6_CAPABILITY_IPV4_ONLY + AT_IPV6_CAPABILITY_IPV6_ONLY +
                             AT_IPV6_CAPABILITY_IPV4V6_OVER_ONE_PDP;
            break;

        case AT_IPV6_CAPABILITY_IPV4V6_OVER_TWO_PDP:
            ipv6Capability = AT_IPV6_CAPABILITY_IPV4_ONLY + AT_IPV6_CAPABILITY_IPV6_ONLY +
                             AT_IPV6_CAPABILITY_IPV4V6_OVER_TWO_PDP;
            break;

        default:
            ipv6Capability = AT_IPV6_CAPABILITY_IPV4_ONLY;
            break;
    }

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, ipv6Capability);
    return AT_OK;
}

VOS_UINT32 AT_TestIpv6capPara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: <0-0xFF>", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

VOS_UINT32 At_QryNdisdupPara(VOS_UINT8 indexNum)
{
    /* 其他的PDP TYPE 没有提出需求，目前只返回OK */
    return AT_OK;
}

VOS_UINT32 At_TestNdisDup(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;

#if (FEATURE_LTE == FEATURE_ON)
    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (1-20),(0-2)", g_parseContext[indexNum].cmdElement->cmdName);
#else
    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (1-16),(0-2)", g_parseContext[indexNum].cmdElement->cmdName);
#endif

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_QryChdataPara_AppUser(TAF_UINT8 indexNum)
{
    AT_PS_DataChanlCfg     *dataChanCfg = VOS_NULL_PTR;
    AT_CH_DataChannelUint32 dataChannelId;
    VOS_UINT32              rslt;
    VOS_UINT16              length = 0;
    VOS_UINT8               loop;
    VOS_UINT8               num = 0;

    /* 输出结果 */
    for (loop = 1; loop <= TAF_MAX_CID; loop++) {
        dataChanCfg = AT_PS_GetDataChanlCfg(indexNum, loop);
        if ((dataChanCfg->used == VOS_TRUE) && (dataChanCfg->rmNetId != AT_PS_INVALID_RMNET_ID)) {
            if (num != 0) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
            }

            /* ^CHDATA:  */
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "^CHDATA: ");

            /* <cid> */
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%d", loop);

            /* <datachannel> */
            rslt = AT_PS_GetChDataValueFromRnicRmNetId((RNIC_DEV_ID_ENUM_UINT8)(dataChanCfg->rmNetId), &dataChannelId);

            if (rslt != VOS_OK) {
                return AT_ERROR;
            }

            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", dataChannelId);

            num++;
        }
    }

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_QryChdataPara(TAF_UINT8 indexNum)
{
    VOS_UINT8 userId;

    /* 命令状态类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    userId = AT_GetUserIndexByClientIndex(indexNum);

    /* APP通道的处理 */
    if (AT_CheckAppUser(userId) == VOS_TRUE) {
        return AT_QryChdataPara_AppUser(userId);
    }

    return AT_ERROR;
}

VOS_UINT32 AT_TestChdataPara(VOS_UINT8 indexNum)
{
    /* 通道检查 */
    if (AT_IsApPort(indexNum) == VOS_FALSE) {
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen =
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %s", g_parseContext[indexNum].cmdElement->cmdName,
            CHDATA_TEST_CMD_PARA_STRING);

    return AT_OK;
}

VOS_UINT32 At_QryIfaceDynamicParaComCheck(const VOS_UINT8 indexNum, TAF_Ctrl *ctrl)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 构造控制结构体 */
    if (AT_PS_BuildIfaceCtrl(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, ctrl) == VOS_ERR) {
        return AT_ERROR;
    }

    return VOS_OK;
}

VOS_UINT32 At_QryApConnStPara(VOS_UINT8 indexNum)
{
    TAF_Ctrl   ctrl;
    VOS_UINT32 bitCid;
    VOS_UINT32 rst;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    rst = At_QryIfaceDynamicParaComCheck(indexNum, &ctrl);

    if (rst != VOS_OK) {
        return rst;
    }

    /* BitCid 构造 */
    /* 支持1-11的cid */
    bitCid = TAF_IFACE_ALL_BIT_NV_CID;

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&ctrl, bitCid) != VOS_OK) {
        AT_ERR_LOG("At_SetApConnStPara: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_APCONNST_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_TestApConnStPara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (1-11)", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

TAF_UINT32 AT_QryAuthdataPara(TAF_UINT8 indexNum)
{
    if (TAF_PS_GetAuthDataInfo(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0) != VOS_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_AUTHDATA_READ;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_TestAuhtdata(VOS_UINT8 indexNum)
{
    VOS_UINT16 length;

#if (FEATURE_LTE == FEATURE_ON)
    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0-31),(0-2),,", g_parseContext[indexNum].cmdElement->cmdName);
#else
    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (1-11),(0-2),,", g_parseContext[indexNum].cmdElement->cmdName);
#endif

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_QryCgpiafPara(VOS_UINT8 indexNum)
{
    AT_CommPsCtx *commPsCtx = VOS_NULL_PTR;

    commPsCtx = AT_GetCommPsCtxAddr();

    /* 命令操作类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 输出查询结果 */
    /* +CGPIAF: <IPv6_AddressFormat>,<IPv6_SubnetNotation>,<IPv6_LeadingZeros>,<IPv6_CompressZeros> */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
        commPsCtx->opIpv6AddrFormat, commPsCtx->opIpv6SubnetNotation, commPsCtx->opIpv6LeadingZeros,
        commPsCtx->opIpv6CompressZeros);

    return AT_OK;
}

VOS_UINT32 AT_QryCgerepPara(VOS_UINT8 indexNum)
{
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;

    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
        netCtx->cgerepCfg.mode, netCtx->cgerepCfg.bfr);

    return AT_OK;
}

VOS_UINT32 At_QryDconnStatPara(VOS_UINT8 indexNum)
{
    TAF_Ctrl   ctrl;
    VOS_UINT32 bitCid;
    VOS_UINT32 rst;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    rst = At_QryIfaceDynamicParaComCheck(indexNum, &ctrl);

    if (rst != VOS_OK) {
        return rst;
    }

    /* 支持1-11的cid */
    bitCid = TAF_IFACE_ALL_BIT_NV_CID;

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&ctrl, bitCid) != VOS_OK) {
        AT_ERR_LOG("At_QryDconnStatPara: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DCONNSTAT_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_TestDconnStatPara(VOS_UINT8 indexNum)
{
    TAF_Ctrl   ctrl;
    VOS_UINT32 bitCid;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_TEST_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 构造控制结构体 */
    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    if (AT_PS_BuildIfaceCtrl(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &ctrl) == VOS_ERR) {
        return AT_ERROR;
    }

    /* 支持1-11的cid */
    bitCid = TAF_IFACE_ALL_BIT_NV_CID;

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&ctrl, bitCid) != VOS_OK) {
        AT_ERR_LOG("AT_TestDconnStatPara: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DCONNSTAT_TEST;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QrySinglePdnSwitch(VOS_UINT8 indexNum)
{
    /* 执行命令操作 */
    if (TAF_PS_GetSinglePdnSwitch(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0) !=
        VOS_OK) {
        AT_WARN_LOG("AT_QrySinglePdnSwitch: TAF_PS_GetSinglePdnSwitch fail.");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SINGLEPDNSWITCH_QRY;
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_QryDhcpPara(VOS_UINT8 indexNum)
{
    TAF_Ctrl   ctrl;
    VOS_UINT32 bitCid;
    VOS_UINT32 rst;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    rst = At_QryIfaceDynamicParaComCheck(indexNum, &ctrl);

    if (rst != VOS_OK) {
        return rst;
    }

    /* BitCid 构造 */
    /* 支持1-11的cid */
    bitCid = TAF_IFACE_ALL_BIT_NV_CID;

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&ctrl, bitCid) != VOS_OK) {
        AT_ERR_LOG("At_QryDhcpPara: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DHCP_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_TestDhcpPara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (1-11)", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

VOS_UINT32 AT_QryNdisStatPara(VOS_UINT8 indexNum)
{
    TAF_Ctrl   ctrl;
    VOS_UINT32 bitCid;
    VOS_UINT32 rst;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    rst = At_QryIfaceDynamicParaComCheck(indexNum, &ctrl);

    if (rst != VOS_OK) {
        return rst;
    }

    /* 支持1-11的cid */
    bitCid = TAF_IFACE_ALL_BIT_NV_CID;

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&ctrl, bitCid) != VOS_OK) {
        AT_ERR_LOG("AT_QryNdisStatPara: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NDISSTATQRY_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryDhcpv6Para(VOS_UINT8 indexNum)
{
    TAF_Ctrl   ctrl;
    VOS_UINT32 bitCid;
    VOS_UINT32 rst;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    rst = At_QryIfaceDynamicParaComCheck(indexNum, &ctrl);

    if (rst != VOS_OK) {
        return rst;
    }

    /* BitCid 构造 */
    /* 支持1-11的cid */
    bitCid = TAF_IFACE_ALL_BIT_NV_CID;

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&ctrl, bitCid) != VOS_OK) {
        AT_ERR_LOG("AT_QryDhcpv6Para: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DHCPV6_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_TestDhcpv6Para(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (1-11)", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

VOS_UINT32 AT_QryCPsErrPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length;

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
        AT_PS_GetPsCallErrCause(indexNum));

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_QryApRaInfoPara(VOS_UINT8 indexNum)
{
    TAF_Ctrl   ctrl;
    VOS_UINT32 bitCid;
    VOS_UINT32 rst;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    rst = At_QryIfaceDynamicParaComCheck(indexNum, &ctrl);

    if (rst != VOS_OK) {
        return rst;
    }

    /* BitCid 构造 */
    /* 支持1-11的cid */
    bitCid = TAF_IFACE_ALL_BIT_NV_CID;

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&ctrl, bitCid) != VOS_OK) {
        AT_ERR_LOG("AT_QryApRaInfoPara: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_APRAINFO_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_TestApRaInfoPara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (1-11)", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

VOS_UINT32 At_TestUsbTetherInfo(VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_TEST_CMD) {
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (\"0:disconn\",\"1:connect\"),(\"rmnet0 - rmnet6\")",
        g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

VOS_UINT32 AT_TestIPv6TempAddrPara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (1-11)", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

VOS_UINT32 At_QryApDialModePara(VOS_UINT8 indexNum)
{
    AT_RNIC_DialModeReq *msg = VOS_NULL_PTR;

    /* 申请ID_RNIC_AT_DIAL_MODE_REQ消息 */
    /*lint -save -e516 */
    msg = (AT_RNIC_DialModeReq *)TAF_AllocMsgWithHeaderLen(WUEPS_PID_AT, sizeof(AT_RNIC_DialModeReq));
    /*lint -restore */
    if (msg == VOS_NULL_PTR) {
        AT_WARN_LOG("At_QryApDialModePara: alloc msg fail!");
        return VOS_ERR;
    }

    /* 初始化消息 */
    (VOS_VOID)memset_s((VOS_CHAR *)msg + VOS_MSG_HEAD_LENGTH,
                       sizeof(AT_RNIC_DialModeReq) - VOS_MSG_HEAD_LENGTH, 0x00,
                       sizeof(AT_RNIC_DialModeReq) - VOS_MSG_HEAD_LENGTH);

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, ACPU_PID_RNIC, sizeof(AT_RNIC_DialModeReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgId    = ID_AT_RNIC_DIAL_MODE_REQ;
    msg->clientId = g_atClientTab[indexNum].clientId;

    /* 发ID_RNIC_AT_DIAL_MODE_REQ消息给RNIC获取当前的流速 */
    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_APDIALMODE_READ;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        AT_WARN_LOG("At_QryApDialModePara: Send msg failed!");
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryDislogPara(VOS_UINT8 indexNum)
{
    AT_DiagOpenFlagUint32 diagPortState;

#if (FEATURE_LTE == FEATURE_ON)
    VOS_UINT32 retDiag;
    VOS_UINT32 ret3GDiag;
    VOS_UINT32 retGps;

    ret3GDiag = AT_ExistSpecificPort(AT_DEV_DIAG);
    retDiag   = AT_ExistSpecificPort(AT_DEV_4G_DIAG);
    retGps    = AT_ExistSpecificPort(AT_DEV_4G_GPS);

    if ((ret3GDiag == VOS_TRUE) && (retDiag == VOS_TRUE) && (retGps == VOS_TRUE)) {
        diagPortState = AT_DIAG_OPEN_FLAG_OPEN;
    } else {
        diagPortState = AT_DIAG_OPEN_FLAG_CLOSE;
    }
#else
    /* 从NV中读取 DISLOG 操作的密码及DIAG口的开关状态 */

    /* DIAG 口的开关状态, 0 打开; 1 关闭 */
    if (AT_ExistSpecificPort(AT_DEV_DIAG) != VOS_TRUE) {
        diagPortState = AT_DIAG_OPEN_FLAG_CLOSE;
    } else {
        diagPortState = AT_DIAG_OPEN_FLAG_OPEN;
    }

#endif

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s:%d", g_parseContext[indexNum].cmdElement->cmdName, diagPortState);
    return AT_OK;
}

#if (FEATURE_SECURITY_SHELL == FEATURE_ON)

VOS_UINT32 AT_QryShellPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 shellStatus = AT_SHELL_CLOSE;

    if (TAF_ACORE_NV_READ(MODEM_ID_0, en_NV_Item_AT_SHELL_OPEN_FLAG, &shellStatus, sizeof(shellStatus)) != NV_OK) {
        AT_WARN_LOG("AT_QryShellPara:WARNING:TAF_ACORE_NV_READ SHELL_OPEN_FLAG faild!");

        return AT_ERROR;
    }

    /* shell的状态只能为0,1,2 */
    if (shellStatus > AT_SHELL_OPEN) {
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s:%d", g_parseContext[indexNum].cmdElement->cmdName, shellStatus);
    return AT_OK;
}
#endif

TAF_UINT32 At_QryDwinsPara(TAF_UINT8 indexNum)
{
    WINS_Config wins;

    (VOS_VOID)memset_s(&wins, sizeof(wins), 0x00, sizeof(wins));

    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_WINS_CONFIG, &wins, sizeof(WINS_Config)) != NV_OK) {
        return AT_ERROR;
    }

    if (wins.ucStatus == 0) {
        /* 若en_NV_Item_WINS未设置，则默认WINS设置为使能 */
        wins.ucWins = WINS_CONFIG_ENABLE;
    }

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, wins.ucWins);
    return AT_OK;
}

VOS_UINT32 AT_QryApDsFlowRptCfgPara(VOS_UINT8 indexNum)
{
    /* 执行命令操作 */
    if (TAF_PS_GetApDsFlowRptCfg(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0) != VOS_OK) {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_APDSFLOWRPTCFG_QRY;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryDsFlowNvWriteCfgPara(VOS_UINT8 indexNum)
{
    /* 执行命令操作 */
    if (TAF_PS_GetDsFlowNvWriteCfg(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0) !=
        VOS_OK) {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DSFLOWNVWRCFG_QRY;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryPsCallRptCmdPara(VOS_UINT8 indexNum)
{

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d, %d", g_parseContext[indexNum].cmdElement->cmdName,
        AT_PS_CALL_TYPE_STICK, AT_GetCommPsCtxAddr()->stickCmd);
    return AT_OK;
}

VOS_UINT32 AT_QryApLanAddrPara(VOS_UINT8 indexNum)
{
    TAF_Ctrl   ctrl;
    VOS_UINT32 bitCid;
    VOS_UINT32 rst;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    rst = At_QryIfaceDynamicParaComCheck(indexNum, &ctrl);

    if (rst != VOS_OK) {
        return rst;
    }

    /* BitCid 构造 */
    /* 支持1-11的cid */
    bitCid = TAF_IFACE_ALL_BIT_NV_CID;

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&ctrl, bitCid) != VOS_OK) {
        AT_ERR_LOG("AT_QryApLanAddrPara: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_APLANADDR_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_TestApLanAddrPara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (1-11)", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

#if (FEATURE_HUAWEI_VP == FEATURE_ON)
VOS_UINT32 AT_SndQryVoicePrefer(VOS_UINT16 clientId, VOS_UINT8 opId)
{
    AT_RABM_QryVoicepreferParaReq *sndMsg = VOS_NULL_PTR;
    VOS_UINT32                     rslt;

    /* 申请内存  */
    /*lint -save -e516 */
    sndMsg = (AT_RABM_QryVoicepreferParaReq *)TAF_AllocMsgWithHeaderLen(WUEPS_PID_AT,
                                                           sizeof(AT_RABM_QryVoicepreferParaReq) - VOS_MSG_HEAD_LENGTH);
    /*lint -restore */
    if (sndMsg == VOS_NULL_PTR) {
        /* 内存申请失败 */
        AT_ERR_LOG("AT_SndQryVoicePrefer:ERROR: Memory Alloc Error for pstMsg");
        return VOS_ERR;
    }

    /* 填写相关参数 */
    TAF_CfgMsgHdr((MsgBlock *)sndMsg, WUEPS_PID_AT, AT_GetDestPid(clientId, I0_WUEPS_PID_RABM),
                  sizeof(AT_RABM_QryVoicepreferParaReq) - VOS_MSG_HEAD_LENGTH);

    sndMsg->msgHeader.msgName = ID_AT_RABM_QRY_VOICEPREFER_PARA_REQ;
    sndMsg->clientId          = clientId;
    sndMsg->opId              = opId;

    /* 调用VOS发送原语 */
    rslt = TAF_TraceAndSendMsg(WUEPS_PID_AT, sndMsg);
    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_SndQryVoicePrefer:ERROR:TAF_TraceAndSendMsg ");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_QryVoicePreferPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rslt;

    /* 调用MN_FillAndSndAppReqMsg()，等待RABM的回复 */
    rslt = AT_SndQryVoicePrefer(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);
    if (rslt == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_VOICEPREFER_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}
#endif


#if (FEATURE_MBB_CUST == FEATURE_ON)
/*
 * 功能说明: ^LTEPROFILE测试命令处理
 * 输出参数: index: AT通道号
 * 返回结果: AT_WAIT_ASYNC_RETURN: 异步等待状态
 *           AT_ERROR: 执行失败
 */
VOS_UINT32 AT_TestCustProfilePara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = TAF_PS_GetCustomAttachProfileCount(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId),
        g_atClientTab[indexNum].opId);
    if (result == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_LTEPROFILE_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

/*
 * 功能说明: ^LTEAPNATTACH查询命令处理
 * 输入参数: index: AT通道号
 * 返回结果: AT_WAIT_ASYNC_RETURN: 异步等待
 *           AT_ERROR: 执行失败
 */
VOS_UINT32 AT_QryCustProfileAttachSwitch(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = TAF_PS_GetAttachProfileSwitchStatus(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId),
        g_atClientTab[indexNum].opId);
    if (result == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_LTEAPNATTACH_SWITCH;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_QryC5gqosPara(VOS_UINT8 indexNum)
{
    if (TAF_PS_Get5gQosInfo(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0) != VOS_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_C5GQOS_READ;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_TestC5gqos(VOS_UINT8 indexNum)
{
    VOS_UINT16 length;

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress,
        "%s: (1-31),(0-12,16-17,65-67,69-76,79-80,128-254),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295)",
        g_parseContext[indexNum].cmdElement->cmdName);
    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 At_TestC5gQosRdp(VOS_UINT8 indexNum)
{
    return At_TestCgeqnegPara(indexNum);
}

VOS_UINT32 At_TestCgcontexPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress,
        "%s: (1-31),(1-3),(1-5)",
        g_parseContext[indexNum].cmdElement->cmdName);
    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}
#endif


