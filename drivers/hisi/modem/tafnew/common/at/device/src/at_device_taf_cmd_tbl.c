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
#include "at_device_taf_cmd_tbl.h"

#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"
#include "at_device_cmd.h"
#include "at_device_comm.h"

#include "at_device_taf_set_cmd_proc.h"
#include "at_device_taf_qry_cmd_proc.h"



#define THIS_FILE_ID PS_FILE_ID_AT_DEVICE_TAF_CMD_TBL_C

static const AT_ParCmdElement g_atDeviceTafCmdTbl[] = {
#if ((FEATURE_UE_MODE_NR == FEATURE_ON) && defined(FEATURE_PHONE_ENG_AT_CMD) && \
     (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: UART通断测试控制及测试结果查询
     * [说明]: 本命令用于UART通断测试控制及测试结果查询。
     * [语法]:
     *     [命令]: ^UARTTEST
     *     [结果]: 执行设置成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行查询成功时：
     *             <CR><LF>^UARTTEST: <result><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     * [参数]:
     *     <result>: 测试结果
     *             0：成功（查询一次之后结果码会置成未测试）
     *             1：数据校验失败（查询一次之后结果码会置成未测试）
     *             2：测试超时（查询一次之后结果码会置成未测试）
     *             3：ICC没开启（查询一次之后结果码会置成未测试）
     *             254：测试结果还没有返回
     *             255：未测试
     * [示例]:
     *     · UART通断测试控制
     *       AT^UARTTEST
     *       OK
     *     · 查询UART通断测试结果
     *       AT^UARTTEST?
     *       ^UARTTEST: 0
     *       OK
     */
    { AT_CMD_UARTTEST,
      AT_SetUartTest, AT_SET_PARA_TIME, AT_QryUartTest, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^UARTTEST", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: I2S通断测试控制及测试结果查询
     * [说明]: 本命令用于I2S通断测试控制及测试结果查询。
     * [语法]:
     *     [命令]: ^I2STEST
     *     [结果]: 执行设置成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行查询成功时：
     *             <CR><LF>^I2STEST: <result><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     * [参数]:
     *     <result>: 测试结果
     *             0：成功（查询一次之后结果码会置成未测试）
     *             1：失败（查询一次之后结果码会置成未测试）
     *             2：测试超时（查询一次之后结果码会置成未测试）
     *             254：测试结果还没有返回
     *             255：未测试
     * [示例]:
     *     · I2S通断测试控制
     *       AT^I2STEST
     *       OK
     *     · 查询I2S通断测试结果
     *       AT^I2STEST?
     *       ^I2STEST: 0
     *       OK
     */
    { AT_CMD_I2STEST,
      AT_SetI2sTest, AT_SET_PARA_TIME, AT_QryI2sTest, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^I2STEST", VOS_NULL_PTR },
#endif

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 查询指定NV项的值
     * [说明]: 读NV项。
     * [语法]:
     *     [命令]: ^NVRD=<Nvid>
     *     [结果]: <CR><LF>^NVRD: <length>,<data><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     * [参数]:
     *     <Nvid>: U16整数型，表示NV项的ID，只支持10进制及16进制（0x开头）。
     *     <length>: 0～512整数型，表示<data>中占用字节的长度。
     *     <data>: 十六进制型，表示写入NV的数据内容，在读操作下无此参数，数据类型内小端顺序，字节以空格分割，字节数最多为128个。
     * [示例]:
     *       mnc长度存储在NV中，Nvid为0x2757。
     *       NV项的结构体定义为：
     *       typedef struct
     *       {
     *           VOS_INT16                           shwDcxoC2fix;
     *       }UCOM_NV_DCXO_C2_FIX_STRU;
     *     · 读mnc长度的AT命令为：
     *       AT^NVRD=10071
     *       ^NVRD: 2,03 00
     *       OK
     */
    { AT_CMD_NVRD,
      AT_SetNVReadPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^NVRD", (VOS_UINT8 *)"(0-65535)" },


#if (FEATURE_LTE == FEATURE_ON)
    /* BEGIN: LTE 快速校准装备AT命令 */
    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 读取NV长度
     * [说明]: 读取NV项的字节数长度。
     * [语法]:
     *     [命令]: ^NVRDLEN=<Nvid>
     *     [结果]: <CR><LF>^NVRDLEN: <Nvid>:< length ><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             有MT相关错误时：
     *             <CR><LF>+CME ERROR:< err_code ><CR><LF>
     * [参数]:
     *     <Nvid>: U16整数型，表示NV项的ID，只支持10进制及16进制（0x开头）。
     *     <length>: 正整数型，表示返回的数据字节数。
     *     <err_code>: 错误码取值：
     *             0：NV不存在 ；
     *             1：获取NV字节数长度失败；
     *             2：其他错误。
     */
    { AT_CMD_NVRDLEN,
      atSetNVRDLenPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^NVRDLEN", (VOS_UINT8 *)"(0-65535)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 读取大NV数据
     * [说明]: 读取NV项的扩展命令，支持指定NV数据的偏移位置，可用于大NV（大小超过2048字节）的读取场景。
     * [语法]:
     *     [命令]: ^NVRDEX=<Nvid>,<offset>,<length>
     *     [结果]: <CR><LF>^NVRDEX: < Nvid >,< offset >,< length >,< data ><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             有MT相关错误时：
     *             <CR><LF>+CME ERROR:< err_code ><CR><LF>
     * [参数]:
     *     <Nvid>: U16整数型，表示NV项的ID，只支持10进制及16进制（0x开头）。
     *     <offfset>: 大于等于0的整数，表示本次读取的data在Nvid数据中的偏移位置。
     *     <length>: 0～512整数型，表示返回的数据字节数，返回的长度可以小于下发的长度。
     *             length建议不超过512，由于不同的物理端口（PCUI，MODEM，UART，VCOM）buff大小不同，使用不同类型的端口读取NV时有可能由于该端口buff较小导致无法上报。PCUI口可以读取时根据NV ID取值，可以读取的大小在1655~1658字节。
     *     <data>: 返回的数据，以十六进制字符串输出，字节之间以空格分隔，数据类型内为小端顺序。
     *     <err_code>: 错误码取值：
     *             0：NV不存在；
     *             1：偏移值非法；
     *             2：数据长度不对；
     *             3：读取NV失败；
     *             4：其他错误。
     */
    { AT_CMD_NVRDEX,
      atSetNVRDExPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^NVRDEX", (VOS_UINT8 *)"(0-65535),(0-2048),(0-2048)" },

#endif


    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置和查询单板条码号
     * [说明]: 用来设置和查询单板条码号，即产品的非成品的单板条码号。若产品不支持可直接返回ERROR。。
     * [语法]:
     *     [命令]: ^BSN=<BSN>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^BSN?
     *     [结果]: <CR><LF>^BSN: <BSN><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <BSN>: 产品的单板条码号，格式为XXXXXXXXXXXXXXXX（字母数字型，定长16位）。例如：当数据卡的SN号为0391831057000001时，条码号为：0391831057000001。
     * [示例]:
     *     · 设置单板条码号
     *       AT^BSN=0391831057000001
     *       OK
     */
    { AT_CMD_BSN,
      At_SetBsn, AT_NOT_SET_TIME, At_QryBsn, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^BSN", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 获取方案平台信息
     * [说明]: 查询产品使用的平台信息。
     * [语法]:
     *     [命令]: ^PLATFORM?
     *     [结果]: <CR><LF>^PLATFORM: <providor>,<sub_platform_info> <CR><LF><CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     * [参数]:
     *     <providor>: 方案供应商信息。使用数字表示，长度为一个字节，取值范围为0～255。
     *             1：华为；
     *     <sub_platform _info>: 子平台信息，取值范围为0～255。
     *             每个方案厂家都从0开始编号，不同方案厂家编号可以重复，长度为一个字节。如果方案子平台分基带和射频平台，以基带平台为编号依据。为了便于统一管理，编号需要和装备人员申请。
     *     <err>: 错误码。
     *             0：无法读取平台信息；
     *             1：其它错误。
     * [示例]:
     *     · 查询产品使用的平台信息
     *       AT^PLATFORM?
     *       ^PLATFORM: 1,71
     *       OK
     */
    { AT_CMD_PLATFORM,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryPlatForm, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^PLATFORM", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置产品整机条码号
     * [说明]: 用来查询或者设置产品的SN号，即产品的成品板条码号。
     * [语法]:
     *     [命令]: ^SN
     *     [结果]: <CR><LF>^SN: <SN><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^SN?
     *     [结果]: <CR><LF>ERROR<CR><LF>
     *     [命令]: ^SN=?
     *     [结果]: <CR><LF>OK<CR><LF>
     *     [命令]: ^SN=<SN>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     * [参数]:
     *     <SN>: 产品的成品板条码号。
     *             格式：03XXXXXXXXXXXXXX（字母数字型，定长16位）。
     *     <err>: 错误码。
     *             0：条码长度错误；
     *             1：其它错误。
     * [表]: 例如：当MT的SN号为0391831057000001时，如下表格：
     *       0, 3, 9, 1, 8, 3, 1, 0, 5, 7, 0, 0, 0, 0, 0, 1,
     * [表]: NV_FACTORY_INFO_I项为SN号预留了20byte空间，当SN号不足20byte时，后面的字节填充0xFF，具体存储格式如下表格：
     *       30, 33, 39, 31, 38, 33, 31, 30, 35, 37, 30, 30, 30, 30, 30, 31, FF, FF, FF, FF,
     * [示例]:
     *     · 产品整机条码设置成功
     *       AT^SN=0391831057000001
     *       OK
     *     · 查询产品的成品条码号
     *       AT^SN
     *       ^SN: 0391831057000001
     *       OK
     *     · 测试^SN
     *       AT^SN=?
     *       OK
     */
    { AT_CMD_SN,
      At_SetSnPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_DEVICE_OTHER_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^SN", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置操作模式
     * [说明]: 用于设置MT的测试模式。
     * [语法]:
     *     [命令]: ^TMODE=<test_mode>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^TMODE?
     *     [结果]: <CR><LF>^TMODE: <status><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^TMODE=?
     *     [结果]: <CR><LF>^TMODE: <test_mode><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <test_mode>: 测试模式。
     *             0：信令模式，单板运行在信令模式下，可以切换到模式1和模式2；
     *             1：非信令模式，可以进行RF方面的非信令控制，不可直接切换到模式2。单板切换到非信令模式后默认射频电路都关闭；如果产品支持LTE-V，非信令模式需要同时设置AT^VMODE=0,0
     *             2：加载模式，用于软件升级，不可直接切换到模式1；
     *             3：重启模式，单板进行软重启；
     *             4：关机（需要返回之后才执行关机操作，仅用于E5）；
     *             11：支持GSM、WCDMA、LTE三种接入技术的非信令模式；
     *             12：支持GSM、WCDMA、LTE三种接入技术的信令模式。
     * [示例]:
     *     · 单板接收到自动关机下电指示，先回复OK，然后执行关机下电操作
     *       AT^TMODE=4
     *       OK
     *     · 设置非信令模式
     *       AT^TMODE=1
     *       OK
     *     ·
     *       AT^VMODE=0,0（如果产品支持LTE-V，需要下发该AT，保证全部进入非信令模式）
     *       OK
     */
    { AT_CMD_TMODE,
      At_SetTModePara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_TestTmodePara, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^TMODE", (VOS_UINT8 *)"(0,1,2,3,4,11,12,13,14,15,16,19)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置MMI测试结果
     * [说明]: 该命令仅适用于需要进行MMI测试的终端产品。
     *         设置指令用于设置测试结果。手动测试时，单板软件自动保存测试结果。
     *         查询指令用于检查手动测试完成情况。
     * [语法]:
     *     [命令]: ^TMMI=<result>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^TMMI?
     *     [结果]: <CR><LF>^TMMI: <result><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <result>: 测试结果。
     *             0：失败；
     *             1：成功。
     */
    { AT_CMD_TMMI,
      AT_SetTmmiPara, AT_NOT_SET_TIME, AT_QryTmmiPara, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^TMMI", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置屏测试
     * [说明]: 该命令仅适用于带屏的终端产品，用于测试各种屏的显示功能。
     * [语法]:
     *     [命令]: ^TSCREEN=<pattern>[,<index>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <pattern>: 屏的测试图形样式，取值范围为0~255。
     *     <index>: 用于多屏产品测试，指定当前测试屏的序号。取值范围0~255。
     */
    { AT_CMD_TSCREEN,
      AT_SetTestScreenPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^TSCREEN", (VOS_UINT8 *)"(0-255),(0-255)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置产品名称
     * [说明]: 设置、读取产品名称。
     * [语法]:
     *     [命令]: ^PRODNAME=<dutname>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^PRODNAME?
     *     [结果]: <CR><LF>^PRODNAME: <dut name><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <dut name>: 产品名称或型号（不带双引号），最长29字符，超过部分被截断。
     * [示例]:
     *     · 设置产品名称成功
     *       AT^PRODNAME=balong
     *       OK
     *     · 查询产品名称成功
     *       AT^PRODNAME?
     *       ^PRODNAME: balong
     *       OK
     */
    { AT_CMD_PRODNAME,
      AT_SetProdNamePara, AT_NOT_SET_TIME, AT_QryProdNamePara, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^PRODNAME", (VOS_UINT8 *)"(@ProductName)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 查询装备归一化AT命令版本号
     * [说明]: 命令返回装备归一化AT命令的版本号，用于不同版本间的兼容设计。若产品不支持直接返回ERROR。
     * [语法]:
     *     [命令]: ^EQVER?
     *     [结果]: <CR><LF>^EQVER: <version><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <version>: 装备归一化AT命令版本号。版本号以三位表示，每位为0～9的数字字符。版本号取决于单板实现的AT命令参考的文档版本号。
     *             114：第一个版本。
     *             依次递增类推，固定返回115。
     * [示例]:
     *     · 查询装备归一化AT命令版本号命令
     *       AT^EQVER?
     *       ^EQVER: 115
     *       OK
     */
    { AT_CMD_EQVER,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryEqverPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^EQVER", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置数据锁
     * [说明]: 对密码保护的数据进行解锁。若产品不支持可直接返回ERROR。命令单次有效，单板重启后需要再次解锁。
     * [语法]:
     *     [命令]: ^DATALOCK=<unlock_code>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^DATALOCK?
     *     [结果]: <CR><LF>^DATALOCK: <status><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <unlock_code>: 解锁码，用于解除数据写保护。
     *     <status>: 数据保护解锁状态。
     *             0：解锁状态，可以操作保护数据；
     *             1：锁状态为非解除状态，无法操作保护数据。
     * [示例]:
     *     · 查询解锁码状态
     *       AT^DATALOCK?
     *       ^DATALOCK: 1
     *       OK
     */
    { AT_CMD_DATALOCK,
      At_SetDataLock, AT_SET_PARA_TIME, At_QryDataLock, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^DATALOCK", (VOS_UINT8 *)"(@nlockCode)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 查询和解锁SIMLOCK
     * [说明]: 解锁SIMLOCK功能和查询SIMLOCK功能的使能情况。解锁时密码连续输入错误三次，重启单板。解锁失败次数到达最大尝试解锁次数后进入永久锁卡状态。若产品不支持可直接返回ERROR。
     * [语法]:
     *     [命令]: ^SIMLOCK=<opr>[,<param>]
     *     [结果]: 当<opr>=0或1且执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             当<opr>=2且命令成功执行时：
     *             <CR><LF>^SIMLOCK: <status><CR><LF><CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     *     [命令]: ^SIMLOCK?
     *     [结果]: <CR><LF>^SIMLOCK: <plmn-num> [<CR><LF>^SIMLOCK: <index>,<mnc-digital-num>,<begin>,<end><CR><LF>]
     *             [....]
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <opr>: 操作类型。
     *             0：解锁SIMLOCK。
     *             <param>：解锁密码或永久解锁密码，8位字符串，由0~9数字组合而成。仅当<opr>=0时才需要输入密码。
     *             1：设置SIMLOCK号段。
     *             <param>：SIMLOCK号段，最多可以支持20个号段。每个号段数据格式为<mnc-digital-num>,<begin>, <end>；多个号段之间用逗号隔开如[,<mnc-digital-num>,<begin>,<end>]。
     *             2：查询SIMLOCK功能的使能情况。
     *     <status>: 锁卡状态。
     *             0：未启用SIMLOCK功能；
     *             1：启用了SIMLOCK功能。
     *     <plmn-num>: 锁卡号段数量。
     *     <index>: 锁卡的索引，为0～20的数字。如果为0，直接显示OK。
     *     <mnc-digital-num>: MNC位数，取值范围为数字2或者数字3。
     *     <begin>: 起始内容，为0～9数字组成的字符串。
     *     <end>: 结束内容，为0～9数字组成的字符串。
     *     <err>: 错误码。
     *             0：写数据时，数据保护未解锁，无法操作保护数据；
     *             1：启用了SIMLOCK功能；
     *             2：输入的号段多于20段；
     *             3：<mnc-digital-num>输入超出范围。
     * [示例]:
     *     · SIMLOCK解锁成功
     *       AT^SIMLOCK=0,12345678
     *       OK
     */
    { AT_CMD_SIMLOCK,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QrySimLockPlmnInfo, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SIMLOCK", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 最大锁卡次数
     * [说明]: 该命令用来设置和查询SIMLOCK锁卡次数。
     * [语法]:
     *     [命令]: ^MAXLCKTMS=<number>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^MAXLCKTMS?
     *     [结果]: <CR><LF>^MAXLCKTMS: <number><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^MAXLCKTMS=?
     *     [结果]: <CR><LF>^MAXLCKTMS: (list of supported <number>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <number>: 整型值，最大锁卡次数，为非负整数, (0-429496728)。
     *     <err>: 错误码。
     *             0：数据保护未解除，操作失败；
     *             1：其它设置错误。
     * [示例]:
     *     · 设置SIMLOCK锁卡次数为1次
     *       AT^MAXLCKTMS=1
     *       OK
     */
    { AT_CMD_MAXLCK_TIMES,
      At_SetMaxLockTimes, AT_SET_PARA_TIME, At_QryMaxLockTimes, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^MAXLCKTMS", (VOS_UINT8 *)"(0-429496728)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置呼叫服务
     * [说明]: 设置呼叫服务信息。若产品不支持直接返回ERROR。
     * [语法]:
     *     [命令]: ^CALLSRV=<service>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CALLSRV?
     *     [结果]: <CR><LF>^CALLSRV: <service><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^CALLSRV=?
     *     [结果]: <CR><LF>^CALLSRV: (list of supported < service >s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <service>: 服务信息，长度为一个字节。
     *     <err>: 错误码。
     *             0：数据保护未解除，操作失败；
     *             1：其它设置错误。
     * [示例]:
     *     · 设置呼叫服务成功
     *       AT^CALLSRV=1
     *       OK
     *     · 测试命令
     *       AT^CALLSRV=?
     *       ^CALLSRV: (0,1)
     *       OK
     */
    { AT_CMD_CALLSRV,
      At_SetCallSrvPara, AT_NOT_SET_TIME, At_QryCallSrvPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CALLSRV", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置生产日期
     * [说明]: 设置、读取生产日期。
     * [语法]:
     *     [命令]: ^MDATE=<date_info>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^MDATE?
     *     [结果]: <CR><LF>^MDATE: <date_info><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^MDATE=?
     *     [结果]: <CR><LF>^MDATE: (@time)<PL><CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <date_info>: 生产日期，格式如2009-12-31 10:10:10。
     * [示例]:
     *     · 读取生产日期
     *       AT^MDATE?
     *       ^MDATE: 2009-12-31 10:10:10
     *       OK
     *     · 执行测试命令
     *       AT^MDATE=?
     *       ^MDATE: (@time)
     *       OK
     */
    { AT_CMD_MDATE,
      AT_SetMDatePara, AT_NOT_SET_TIME, AT_QryMDatePara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^MDATE", (VOS_UINT8 *)"(@time)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置制造信息
     * [说明]: 设置、读取制造信息指令。
     *         制造信息指装备写入的与生产相关的信息，如制造时间、工位识别码以及其它对装备内部使用的信息。
     * [语法]:
     *     [命令]: ^FACINFO=<index>,<value_info>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^FACINFO?
     *     [结果]: <CR><LF>^FACINFO: 0,<info0><CR><LF>
     *             <CR><LF>^FACINFO: 1,<info1><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <index>: 制造信息索引。
     *     <value_info>: 每段定制信息的内容，取值范围为<info0>或<info1>。
     *             <info0>：字段0的定制内容，长度为128字节，每个字节用十进制表示；
     *             <info1>：字段1的定制内容，长度为128字节，每个字节用十进制表示。
     * [示例]:
     *     · 读取制造信息
     *       AT^FACINFO?
     *       ^FACINFO: 0,
     *       ^FACINFO: 1,
     *       OK
     */
    { AT_CMD_FACINFO,
      AT_SetFacInfoPara, AT_NOT_SET_TIME, AT_QryFacInfoPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^FACINFO", (VOS_UINT8 *)"(0,1),(@valueInfo)" },


    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置查询物理号
     * [说明]: 该命令用来写入或者查询物理号。若产品不支持可直接返回ERROR。
     * [语法]:
     *     [命令]: ^PHYNUM=<type>,<number>[,<times>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^PHYNUM?
     *     [结果]: <CR><LF>^PHYNUM: <type>,<number><CR><LF>
     *             （显示单板支持的物理号）
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <type>: 物理号的类型。
     *             IMEI（单板IMEI）
     *             MACWLAN（MAC地址）
     *             SVN(单板SVN)
     *             ESN(暂不支持)
     *             MEID（暂不支持）
     *             UMID（暂不支持）
     *     <number>: 物理号，根据具体情况来设计，格式满足3GPP相关规定。不对MAC进行有效性检查，认为一直有效。
     *     <times>: 操作次数。
     *             0：第一次写；
     *             1：重写，重写物理号的AT需要在数据锁指令^DATALOCK后执行。
     *     <err>: 错误码。
     *             0：物理号长度错误；
     *             1：物理号不合法；
     *             2：物理号类型错误。
     * [示例]:
     *     · 设置物理号
     *       AT^PHYNUM=MACWLAN,7AFEE22111E4
     *       OK
     *     · 查询命令
     *       AT^PHYNUM?
     *       ^PHYNUM: IMEI,123456789012345
     *       ^PHYNUM: MACWLAN,7AFEE22111E4
     *       ^PHYNUM: SVN,12
     *       OK
     */
    { AT_CMD_PHYNUM,
      AT_SetPhyNumPara, AT_NOT_SET_TIME, AT_QryPhyNumPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^PHYNUM", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 查询定制版本号
     * [说明]: 命令返回数据卡定制版本号，定制版本号指对应单板支持的定制阶段。若产品不支持可直接返回ERROR。
     * [语法]:
     *     [命令]: ^CSVER?
     *     [结果]: <CR><LF>^CSVER: <csver><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <csver>: 定制版本号，版本号以四位表示，每位为0～9的数字字符。
     *             1000：定制第一个版本；
     *             1001：定制第二个版本；
     *             依次递增类推。
     * [示例]:
     *     · 定制版本号查询
     *       AT^CSVER?
     *       ^CSVER: 1001
     *       OK
     */
    { AT_CMD_CSVER,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryCsVer, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CSVER", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置QOS
     * [说明]: 设置QOS。若产品不支持直接返回ERROR。
     * [语法]:
     *     [命令]: ^QOS=<traffic_class>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^QOS?
     *     [结果]: <CR><LF>^QOS: <traffic_class><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <traffic_class>: 整型值，QOS类型，取值范围(0-4)。
     *     <err>: 错误码。
     *             0：数据保护未解除。
     *             1：其它错误。
     * [示例]:
     *     · QOS设置成功
     *       AT^QOS=1
     *       OK
     */
    { AT_CMD_QOS,
      At_SetQosPara, AT_NOT_SET_TIME, At_QryQosPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^QOS", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 查询外置SIM/USIM/UIM卡接触状态
     * [说明]: 查询外置SIM/USIM/UIM卡接触性是否良好。若产品采用内置卡直接返回没有外置卡。
     * [语法]:
     *     [命令]: ^RSIM?
     *     [结果]: <CR><LF>^RSIM: <state><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <state>: 接触状态。
     *             0：没有任何卡接触上；
     *             1：已经接上SIM/USIM/UIM卡；
     *             2：SIM/USIM/UIM卡繁忙需要等待。
     * [示例]:
     *     · 查询外置SIM卡接触状态
     *       AT^RSIM?
     *       ^RSIM: 0
     *       OK
     */
    { AT_CMD_RSIM,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryRsimPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^RSIM", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置WiFi WEP
     * [说明]: 设置、读取、查询WiFi WEP（WIFI的KEY）内容、组数。
     *         读取时，按行返回所有内容。
     *         该命令查询和设置均受DATALOCK数据锁保护。未输入密码时，查询WiFi WEP内容返回空（WEP已定制组数为0）。
     * [语法]:
     *     [命令]: ^WIWEP=<index>,<content>,<group>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^WIWEP?
     *     [结果]: <CR><LF>^WIWEP: <number><CR><LF>
     *             <CR><LF>^WIWEP: <index>,<content> ,<group><CR><LF>
     *             [….]
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^WIWEP=?
     *     [结果]: <CR><LF>^WIWEP: <total><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <index>: WiFi KEY的index值，取值范围0~3。
     *     <content>: <index>对应的WiFi KEY内容。
     *     <total>: 工位支持的WIFI KEY个数，Balong为16个。
     *     <number>: 工位支持的WIFI KEY个数。
     *     <group>: 同^SSID <group>参数，Balong支持0~3。
     * [示例]:
     *     · 设置第一组的第一个WIFI的KEY为12345
     *       AT^WIWEP=0,12345,0
     *       OK
     *     · 查询单板存储的WIFI的KEY
     *       AT^WIWEP?
     *       ^WIWEP: 1
     *       ^WIWEP: 0,12345,0
     *       OK
     *     · 测试命令
     *       AT^WIWEP=?
     *       ^WIWEP: 16
     *       OK
     */
    { AT_CMD_WIWEP,
      AT_SetWiwepPara, AT_SET_PARA_TIME, AT_QryWiwepPara, AT_QRY_PARA_TIME, AT_TestWiwepPara, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^WIWEP", (VOS_UINT8 *)"(0-3),(@wifikey),(0-3)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置/查询AT命令长度
     * [说明]: PC侧可以用此命令查询单板最大可以直接接收AT命令字符个数，以及单板通过AT一次最大可以响应的字符个数，该字符个数均不包含AT这两个字符。该命令中描述的发送和接收均相对于单板而言的。
     * [语法]:
     *     [命令]: ^CMDLEN=<max_rx_len>,<max_tx_len>
     *     [结果]: <CR><LF>^CMDLEN: <max_rx_len>,<max_tx_len><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CMDLEN?
     *     [结果]: <CR><LF>^CMDLEN: <max_rx_len>,<max_tx_len><CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^CMDLEN=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <max_rx_len>: 查询命令中，该参数指单板一次最大可以接收的字符个数。
     *             设置命令中，该参数指PC一次期望发送的字符个数。
     *             在设置命令的响应中，该参数是单板根据PC期望值和自身支持能力中的较小者作为最终的协商结果，在最终的数据文件传输中，PC一次给单板发送字符个数不大于该值。
     *     <max_tx_len>: 查询命令中，该参数指单板一次最大可以发送给PC的字符个数。
     *             设置命令中，该参数指PC期望单板一次发送的字符个数。
     *             在设置命令的响应中，该参数是单板根据PC期望值和自身支持能力中的较小者作为最终的协商结果，在最终的数据文件传输中，单板一次给PC发送的字符个数不大于该值。
     * [示例]:
     *     · 设置PC侧期望发送的AT命令长度为1000字符，期望接收的AT命令响应字符串长度为2000
     *       AT^CMDLEN=1000,2000
     *       OK
     *     · 查询单板侧期望接收的AT命令长度和期望发送的AT命令响应字符串长度
     *       AT^CMDLEN?
     *       ^CMDLEN: 1598,5000
     *       OK
     *     · 测试命令
     *       AT^CMDLEN=?
     *       OK
     */
    { AT_CMD_CMDLEN,
      AT_SetCmdlenPara, AT_SET_PARA_TIME, AT_QryCmdlenPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^CMDLEN", (VOS_UINT8 *)"(0-65535),(0-65535)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 查询UT工位NV恢复状态
     * [说明]: 该命令只用于产线CS工位验证UT工位是否升级成功。
     * [语法]:
     *     [命令]: ^QRYNVRESUME?
     *     [结果]: <CR><LF>^QRYNVRESUME: <status><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^QRYNVRESUME=?
     *     [结果]: <CR><LF>^QRYNVRESUME: (list of supported<status>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <status>: 当前状态。
     *             0：NV恢复成功；
     *             1：NV恢复失败。
     * [示例]:
     *     · 查询UT工位NV恢复成功
     *       AT^QRYNVRESUME?
     *       ^QRYNVRESUME: 0
     *       OK
     *     · 测试命令
     *       AT^QRYNVRESUME=?
     *       ^QRYNVRESUME: (0,1)
     *       OK
     */
    { AT_CMD_QRYNVRESUME,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryNvResumePara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^QRYNVRESUME", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 写SimLock锁网锁卡数据
     * [说明]: 该命令用于AP-Modem形态下写SimLock锁网锁卡数据，一次仅允许进行一种锁网锁卡类型数据的输入，如果三种锁网锁卡类型都要设置需要进行三次设置操作。该命令需要产线鉴权通过才能执行，设置的锁网锁卡信息重启后才能正常使用。
     *         命令使用场景：产线工具下发。
     *         命令使用限制：需要通过产线鉴权，只允许从与AP对接的专用AT通道下发。
     * [语法]:
     *     [命令]: ^SIMLOCKDATAWRITE=<simlock_data_write>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^SIMLOCKDATAWRITE=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <simlock_data_write>: 后台发送写入SimLock数据请求，为SimLock数据结构的二进制码流，包括锁卡类型、激活状态、PIN 码最大解锁次数、号段组数和锁网号段数组及其校验密文，CK、UK使用私钥进行RSA加密后的密文。
     *             传输时，通过Half-Byte编码成1096字节的字符串。
     * [示例]:
     *     · 写simlock锁网锁卡数据：
     *       AT^SIMLOCKDATAWRITE=0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456701234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345670123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456701234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345670123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456701234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567012345678901234
     *       OK
     *     · 测试命令
     *       AT^SIMLOCKDATAWRITE=?
     *       OK
     */
    { AT_CMD_SIMLOCKDATAWRITE,
      VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestHsicCmdPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SIMLOCKDATAWRITE", (VOS_UINT8 *)"(@SimlockData)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 写数据文件命令
     * [说明]: PC侧可以用此命令向单板写数据文件，命令支持任意文件（包括XML文件、文本文件、Binary文件）。
     * [语法]:
     *     [命令]: ^RSFW=<itemname>,<sub_itemname>,
     *             <ref>,<total>,<index>,<item>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             有MT相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^RSFW=?
     *     [结果]: <CR><LF>^RSFW: (list of supported <itemname>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <itemname>: 字符串，数据文件标识
     *             对于统计项名称字符串，取值见《移动宽带 业务平台在线统计 参数定义.doc》，如“RF60”。
     *     <sub_itemname>: 字符串，数据文件子标识
     *             此参数内容不在此文定义，由TE（PC应用程序）自行扩展，比如可以只读取文件的部分指定内容。如果不需要此参数，则为空字符串””。
     *     <ref>: 参考号
     *             类似长短信的“Concatenated short message reference number”，详见[1]的9.2.3.24.1节。取值范围0~255。这个参数的目的，在于增加校验机制，避免拼接时混乱。
     *             每次上报item，创建一个新的参考号；如果一个item被拆分，则共享一个参考号。
     *             SIMLOCK参考号为 123
     *     <total>: 拆分包总数
     *             类似长短信的“Maximum number of short messages in the concatenated short message”，详见[1]的9.2.3.24.1节。取值范围0~255。
     *             如果没有拆分，则为1。
     *     <index>: 拆分包当前索引
     *             类似长短信的“Sequence number of the current short message”，详见[1]的9.2.3.24.1节。取值范围0~255，从1开始计数。
     *             如果没有拆分，则为1。
     *     <item>: 实际上报数据文件，经过Base64编码后的字符串
     */
    { AT_CMD_RSFW,
      AT_SetRsfwPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestRsfwPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^RSFW", (VOS_UINT8 *)"(name),(subname),(0-255),(0-255),(0-255),(item)" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 查询ICCID
     * [说明]: 查询(U)SIM卡的ICCID（Icc Identification）值。
     * [语法]:
     *     [命令]: ^ICCID
     *     [结果]: <CR><LF>OK<CR><LF>
     *     [命令]: ^ICCID?
     *     [结果]: <CR><LF>^ICCID: <iccid><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^ICCID=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     * [示例]:
     *     · 查询ICCID值
     *       AT^ICCID?
     *       ^ICCID: 98680036904030021872
     *       OK
     *     · 测试命令
     *       AT^ICCID=?
     *       OK
     */
    { AT_CMD_CICCID,
      At_SetIccidPara, AT_SET_PARA_TIME, At_QryIccidPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_E5_IS_LOCKED,
      (TAF_UINT8 *)"^ICCID", TAF_NULL_PTR },


#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置发射机的DAC值
     * [说明]: 在调试状态下设置某一频段发射机的DAC值，若产品不支持可直接返回ERROR。。
     *         此命令在非信令模式（AT^TMODE=1）下使用，其它模式下返回错误码0。
     *         此命令需要在设置非信令信道（^FCHAN）后执行。
     * [语法]:
     *     [命令]: ^FDAC=<tx_control_num>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^FDAC?
     *     [结果]: <CR><LF>^FDAC: <tx_control_num><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     *     [命令]: ^FDAC=?
     *     [结果]: <CR><LF>^FDAC: (list of supported <tx_control_num>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <tx_control_num>: DAC的值。
     *     <err>: 错误码。
     *             0：未设置当前信道；
     *             1：无法读取当前设置的DAC值；
     *             2：设置DAC操作失败；
     *             3：读取DAC操作失败。
     */
    { AT_CMD_FDAC,
      AT_SetFDac, AT_SET_PARA_TIME, AT_QryFDac, AT_NOT_SET_TIME, At_TestFdacPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^FDAC", (VOS_UINT8 *)"(0-65536)" },
#endif

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置发射机PA等级
     * [说明]: 在调试状态下设置某一频段发射机的PA等级。若产品不支持可直接返回ERROR。
     *         此命令在非信令模式（AT^TMODE=1）下使用，其它模式下返回错误码0。
     *         此命令需要在设置非信令信道（^FCHAN）后执行。
     *         如果参数不带，则默认为0。
     *         注：此命令由FEATURE_PHONE_ENG_AT_CMD宏控制。
     * [语法]:
     *     [命令]: ^FPA=<level>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     *     [命令]: ^FPA?
     *     [结果]: <CR><LF>^FPA: <level><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^FPA=?
     *     [结果]: <CR><LF>^FPA: (list of supported < level >s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <level>: PA等级，长度为一个字节。
     *     <err>: 错误码。
     *             0：单板模式错误；
     *             1：没有设置相关信道；
     *             2：设置PA级失败；
     *             3：其它错误。
     * [示例]:
     *     · 设置PA等级成功
     *       AT^FPA=1
     *       OK
     *     · 执行测试命令
     *       AT^PFA=?
     *       ^PFA: (0-3)
     *       OK
     */
    { AT_CMD_FPA,
      At_SetFpaPara, AT_SET_PARA_TIME, At_QryFpaPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^FPA", (VOS_UINT8 *)FPA_CMD_PARA_STRING },

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
    /* 设置接收机等级，后面会逐渐用^FAGCGAIN代替^FLNA */
    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置接收机LNA等级
     * [说明]: 在调试状态下设置某一频段接收机的LNA等级。若产品不支持可直接返回ERROR。
     *         此命令在非信令模式（AT^TMODE=1）下使用，其它模式下返回错误码0。此命令需要在设置非信令信道（^FCHAN）后执行。
     *         注：此命令由FEATURE_PHONE_ENG_AT_CMD宏控制。
     * [语法]:
     *     [命令]: ^FLNA=<level>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^FLNA?
     *     [结果]: <CR><LF>^FLNA: <level><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <level>: 设置LNA的等级，长度为一个字节。
     *     <err>: 错误码。
     *             0：单板模式错误；
     *             1：没有设置相关信道；
     *             2：设置LNA级失败；
     *             3：其它错误。
     * [示例]:
     *     · 设置LNA等级成功
     *       AT^FLNA=1
     *       OK
     */
    { AT_CMD_FLNA,
      AT_SetFAgcgainPara, AT_SET_PARA_TIME, AT_QryFAgcgainPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_FLNA_OTHER_ERR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^FLNA", (VOS_UINT8 *)"(0-255)" },
#endif

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置接收机AGCGAIN等级
     * [说明]: 在调试状态下设置某一频段接收机的AGC等级。若产品不支持可直接返回ERROR。
     *         此命令在非信令模式（AT^TMODE=1）下使用，其它模式下返回错误码0。此命令需要在设置非信令信道（^FCHAN）后执行。
     *         如果参数不带，则默认为0。
     * [语法]:
     *     [命令]: ^FAGCGAIN=<level>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^FAGCGAIN?
     *     [结果]: <CR><LF>^FAGCGAIN: <level><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <level>: 设置FAGCGain的等级，长度为一个字节，取值范围0～255。0表示最高增益级。
     *     <err>: 错误码。
     *             0：单板模式错误；
     *             1：没有加载DSP（^FCHAN）；
     *             50：参数错误；
     * [示例]:
     *     · 设置AGCGAIN等级成功
     *       AT^FAGCGAIN=1
     *       OK
     */
    { AT_CMD_FAGCGAIN,
      AT_SetFAgcgainPara, AT_SET_PARA_TIME, AT_QryFAgcgainPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^FAGCGAIN", (VOS_UINT8 *)"(0-255)" },

#endif

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置Web UI登陆密码
     * [说明]: 设置、校验、解除Web UI登陆密码。
     * [语法]:
     *     [命令]: ^WUPWD=<opr>,<password>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <opr>: 操作类型，长度为一个字节。
     *             0：设置web UI的登陆密码；
     *             1：校验web UI的登陆密码。
     *     <password>: 对应的password的内容。
     */
    { AT_CMD_WUPWD,
      AT_SetWebPwdPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^WUPWD", (VOS_UINT8 *)"(0,1),(@WUPWD)" },


#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
#if (FEATURE_LTE == FEATURE_ON)
    { AT_CMD_SFM,
      At_SetSfm, AT_SET_PARA_TIME, At_QrySfm, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SFM", (VOS_UINT8 *)"(0,1)" },
#endif
#endif
};

/* 注册TAF装备AT命令表 */
VOS_UINT32 AT_RegisterDeviceTafCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atDeviceTafCmdTbl, sizeof(g_atDeviceTafCmdTbl) / sizeof(g_atDeviceTafCmdTbl[0]));
}

