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
#include "at_custom_drv_cmd_tbl.h"
#include "AtParse.h"

#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"

#include "at_custom_drv_set_cmd_proc.h"
#include "at_custom_drv_qry_cmd_proc.h"



#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_DRV_CMD_TBL_C


static const AT_ParCmdElement g_atCustomDrvCmdTbl[] = {
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 启动/关闭USB PHY
     * [说明]: 启动/关闭USB PHY。
     *         命令使用通道：只能通过与AP对接的AT通道或MUX通道下发。
     * [语法]:
     *     [命令]: +USBSWITCH=<mode>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +USBSWITCH?
     *     [结果]: <CR><LF>+USBSWITCH: <mode><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +USBSWITCH=?
     *     [结果]: <CR><LF>+USBSWITCH: (list of supported <mode>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整形值，表示开启或关闭USB PHY，取值1或2。
     *             1：USB PHY切换到Modem侧；
     *             2：USB PHY切换到AP侧。
     * [示例]:
     *     · AP侧要求Modem侧启动USB PHY
     *       AT+USBSWITCH=1
     *       OK
     */
    { AT_CMD_USBSWITCH,
      At_SetUsbSwitchPara, AT_NOT_SET_TIME, At_QryUsbSwitchPara, AT_NOT_SET_TIME, AT_TestUsbSwitchPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+USBSWITCH", (VOS_UINT8 *)"(1,2)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询天线连接状态
     * [说明]: 该AT命令用于与AP查询平台当前天线连接状态语法；
     * [语法]:
     *     [命令]: ^ANTSTATE
     *     [结果]: <CR><LF>^ANTSTATE: <Value><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <Value>: 整型值，天线连接状态。
     *             0：天线模式。
     *             1：传导模式。
     * [示例]:
     *     · AP查询当前天线模式
     *       AT^ANTSTATE
     *       ^ANTSTATE: 1
     *       OK
     */
    { AT_CMD_ANTSTATE,
      AT_QryAntState, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^ANTSTATE", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置/查询OM端口的默认输出设备接口
     * [说明]: 设置/查询OM默认输出设备接口，默认永久生效。
     * [语法]:
     *     [命令]: ^LOGPORT=<port_type>[,<config>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^LOGPORT?
     *     [结果]: <CR><LF>^LOGPORT: <port_type><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             有MT相关错误时：
     *             <CR><LF> ERROR<CR><LF>
     *     [命令]: ^LOGPORT=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <port_type>: 整数型，OM端口类型：
     *             0：USB
     *             1：VCOM
     *             2：WiFi
     *             3：SD卡
     *             4：本地文件系统
     *             5：HSIC
     *             其他：保留。
     *             目前仅支持USB和VCOM，其它暂不支持。
     *             设置命令配置除USB和VCOM外返回错误，设置后立即生效。
     *     <config>: 整数型，<port_type>设置后是否永久生效。
     *             0：临时生效，重启后恢复；
     *             1：永久生效。
     * [示例]:
     *     · 设置可维可测从USB输出并永久生效
     *       AT^LOGPORT=0,1
     *       OK
     *     · 查询当前OM通道类型
     *       AT^LOGPORT?
     *       ^LOGPORT: 0
     *       OK
     *     · 测试命令
     *       AT^LOGPORT=?
     *       OK
     */
    { AT_CMD_LOGPORT,
      At_SetLogPortPara, AT_NOT_SET_TIME, At_QryLogPortPara, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
#if (FEATURE_MBB_CUST == FEATURE_ON)
      (VOS_UINT8 *)"^LOGPORT", (VOS_UINT8 *)"(0-2),(0,1)" },
#else
      (VOS_UINT8 *)"^LOGPORT", (VOS_UINT8 *)"(0,1),(0,1)" },
#endif

#if (FEATURE_MBB_CUST == FEATURE_ON)
    { AT_CMD_GETDONGLEINFO,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryMbbDongleInfo, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (const VOS_UINT8 *)"^GETDONGLEINFO", VOS_NULL_PTR },

    { AT_CMD_CHECKAUTHORITY,
      At_CheckMbbAuthority, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (const VOS_UINT8 *)"^CHECKAUTHORITY", (const VOS_UINT8 *)"(@Rsa)" },

    { AT_CMD_CONFORMAUTHORITY,
      At_SetMbbConfirmAuthority, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (const VOS_UINT8 *)"^CONFORMAUTHORITY", VOS_NULL_PTR },

    { AT_CMD_NETDOGINTEGRITY,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryMbbNetDogIntegrity, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (const VOS_UINT8 *)"^NETDOGINT", VOS_NULL_PTR },

    { AT_CMD_WAKEUPCFG,
      AT_MbbSetWakeUpCfgPara, AT_SET_PARA_TIME, AT_MbbQryWakeupcfgPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (const VOS_UINT8 *)"^WAKEUPCFG", (VOS_UINT8 *)"(0-1),(0-3),(0-63)" },
#endif
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询终端芯片DIE序列号
     * [说明]: 查询终端芯片DIE序列号码，保存在装备信息系统中，作为终端整机信息，DIE 序列号实际长度156 BIT。
     * [语法]:
     *     [命令]: ^DIESN
     *     [结果]: <SN>
     *             +CME ERROR: <err>
     *     [命令]: ^DIESN=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <SN>: 芯片DIE序列号，16进制，20字节长。
     *             备注：因为实际DIE 序列号是156 BIT，在高位最高字节高4 BIT填充0x0。
     *             例：0x0f000000…000078， 红色所示“0”无效
     * [示例]:
     *     · 查询DIE 序列号
     *       AT^DIESN
     *       ^DIESN: 0x0f0d0000040000f5000000000000000000000008
     *       OK
     */
    { AT_CMD_DIESN,
      At_SetDieSNPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^DIESN", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置LOG延迟写入超时时间
     * [说明]: 此命令仅在AP+Modem产品形态下LOG延迟写入功能开启时设置超时时间才有意义。在满足条件的情况下设置立即生效，不需要重启单板。
     * [语法]:
     *     [命令]: ^LOGCFG[=[<TIMEOUT_TYPE>]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^LOGCFG=?
     *     [结果]: <CR><LF>OK<CR><LF>
     *     [命令]: ^LOGCFG?
     *     [结果]: <CR><LF> LOGCFG: <TIMEOUT_TYPE><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <TIMEOUT_TYPE>: 延迟写入超时时间类型。
     *             0：使用SOCP的默认超时时间，10ms；
     *             1：使用DTS中配置的延迟写入超时时间。
     * [示例]:
     *     · 使用SOCP的默认超时时间
     *       AT^LOGCFG=0
     *       OK
     */
    { AT_CMD_LOGCFG,
      AT_SetLogCfgPara, AT_SET_PARA_TIME, At_QryLogCfgPara, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^LOGCFG", (VOS_UINT8 *)"(0,1,2)" },

    { AT_CMD_LOGCPS,
      AT_SetLogCpsPara, AT_SET_PARA_TIME, At_QryLogCpsPara, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^LOGCPS", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 激活/恢复运营商定制NV配置
     * [说明]: 激活/恢复运营商定制NV配置 ACTIVENCFG。该命令只用于手机平台，其他平台不支持。
     * [语法]:
     *     [命令]: ^ACTIVENCFG=<enable>[,<carrierName>,<effectiveWay>]
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^ACTIVENCFG?
     *     [结果]: 执行成功时：
     *             <CR><LF>^ACTIVENCFG: < carrierName >,< effectiveWay><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误时：
     *             <CR><LF>ERROR ><CR><LF>
     *             或<CR><LF>+CME ERROR: <err_code><CR><LF>
     * [参数]:
     *     <enable>: 整型值，激活/恢复运营商定制NV配置
     *             0：Modem侧加载恢复comm.mbn，完成后，刷新NV文件系统。
     *             1：Modem侧加载carrier.bin，完成后，刷新NV文件系统。
     *             2：Modem侧先加载comm.mbn，完成后，加载carrier.bin，最后刷新NV文件系统。
     *             3：只完成写随卡NV，不执行FLUSH动作。
     *     <carrierName>: 字符串类型，随卡运营商名称，最大长度为48字节。
     *             说明:字符使用ASCII存储。
     *     <effectiveWay>: 整型值，运营商定制NV生效方式，取值范围0~2，默认为0。
     *             0：无需重启Modem或卡上下电；
     *             1：NV生效需要Modem重启；
     *             2：NV生效需要卡上下电。
     *             将保存到NV_ITEM_USIM_DEPENDENT_RESULT_INFO的effectiveWay中。
     * [示例]:
     *     · Modem侧加载恢复comm.mbn
     *       AT^ACTIVENCFG=0
     *       OK
     *     · Modem侧加载carrier.bin
     *       AT^ACTIVENCFG=1
     *       OK
     *     · Modem侧先加载comm.mbn，再加载carrier.bin
     *       AT^ACTIVENCFG=2
     *       OK
     *     · Modem侧加载carrier.bin，并保存随卡运营商名称和NV生效方式默认为0
     *       AT^ACTIVENCFG=1,"cmcc.com"
     *       OK
     *     · Modem侧加载carrier.bin，并保存随卡运营商名称和NV生效方式1
     *       AT^ACTIVENCFG=1,"cmcc.com",1
     *       OK
     *     · 查询命令随卡运营商名称和NV生效方式
     *       AT^ACTIVENCFG?
     *       ^ACTIVENCFG: "cmcc.com", 1
     *       OK
     */
    { AT_CMD_NVLOAD,
      AT_SetNvLoadPara, AT_SET_PARA_TIME, AT_QryNvLoadPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^ACTIVENCFG", (TAF_UINT8 *)"(0,1,2),(str),(0,1,2),(0,1)" },

    { AT_CMD_FLASHINFO,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryFlashInfoPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_E5_IS_LOCKED,
      (VOS_UINT8 *)"^FLASHINFO", VOS_NULL_PTR },

    { AT_CMD_NVCHK,
      AT_SetNVCHKPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^NVCHK", (TAF_UINT8 *)"(0-3)" }, /* mode */

    { AT_CMD_APPDMVER,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryAppdmverPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^APPDMVER", VOS_NULL_PTR },

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    { AT_CMD_MAXFREELOCKSIZE,
      TAF_NULL_PTR, AT_NOT_SET_TIME, At_QryMaxFreelockSizePara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE,
      (TAF_UINT8 *)"^MFREELOCKSIZE", TAF_NULL_PTR },

    { AT_CMD_SDREBOOT,
      AT_SetSdrebootCmd, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_E5_IS_LOCKED,
      (VOS_UINT8 *)"^SDREBOOT", (VOS_UINT8 *)"(pwd)" },

    { AT_CMD_YJCX,
      At_SetYjcxPara, AT_SET_PARA_TIME, At_QryYjcxPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE,
      (VOS_UINT8 *)"^YJCX", (VOS_UINT8 *)"(0,1)" },


#endif
};

/* 注册DRV定制AT命令表 */
VOS_UINT32 AT_RegisterCustomDrvCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atCustomDrvCmdTbl, sizeof(g_atCustomDrvCmdTbl) / sizeof(g_atCustomDrvCmdTbl[0]));
}

