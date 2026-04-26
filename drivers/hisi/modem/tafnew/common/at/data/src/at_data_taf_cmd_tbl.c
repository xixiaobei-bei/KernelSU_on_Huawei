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

#include "at_data_taf_set_cmd_proc.h"
#include "at_data_taf_qry_cmd_proc.h"
#include "at_data_taf_cmd_tbl.h"
#include "at_cmd_proc.h"
#include "at_test_para_cmd.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_DATA_TAF_CMD_TBL_C

static const AT_ParCmdElement g_atDataTafCmdTbl[] = {
    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 设置DNS
     * [说明]: 设置或删除PDP上下文使用的DNS（Domain Name Server）信息，包括主、辅DNS。
     *         如果命令中只有<cid>一个参数，如AT^CGDNS=<cid>，表示删除此<cid>对应的PDP上下文的DNS信息。
     *         执行at^cgdns设置、删除动作时，cid的参数范围为1-11。
     *         使用at^cgdns?或at^cgdns=?时，返回的cid范围为1-31，如果没有PDP上下文，返回空（包含了缺省承载和专有承载）。
     *         注意：使用at^cgdns=?时，返回值为命令+CGACT已激活的PID。
     * [语法]:
     *     [命令]: ^CGDNS=<cid>,[<PriDns>,[<SecDns>]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CGDNS?
     *     [结果]: <CR><LF>^CGDNS: <cid>,[<PriDns>,[<SecDns>]]
     *             [<CR><LF>^CGDNS: <cid>,[<PriDns>,[<SecDns>]]
     *             [...]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^CGDNS=?
     *     [结果]: <CR><LF>^CGDNS: (list of <cid>s associated with active contexts) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cid>: 整型值，PDP上下文标识符，取值范围为1～11。
     *     <PriDns>: 字符串类型，IPV4类型的主DNS地址。
     *     <SecDns>: 字符串类型，IPV4类型的辅DNS地址。
     * [示例]:
     *     · 定义DNS参数
     *       AT^CGDNS=1,"192.168.0.10","192.168.0.200"
     *       OK
     *     · 查询命令
     *       AT^CGDNS?
     *       ^CGDNS: 1,"192.168.0.10","192.168.0.200"
     *       OK
     *     · 测试命令，返回激活的cid
     *       AT^CGDNS=?
     *       ^CGDNS: (1)
     *       OK
     */
    { AT_CMD_CGDNS,
      AT_SetCgdnsPara, AT_SET_PARA_TIME, AT_QryCgdnsPara, AT_QRY_PARA_TIME, At_TestCgdnsPara, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^CGDNS", (VOS_UINT8 *)"(1-11),(PriDns),(SecDns)" },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 查询错误码
     * [说明]: 查询单板某个功能项最近的错误码，目前仅提供查询拨号失败时的错误码。此命令必须与控制错误码上报的可配置NV项配合使用，否则返回ERROR。
     * [语法]:
     *     [命令]: ^GLASTERR=<type>
     *     [结果]: <CR><LF>^GLASTERR: <type>,<code><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^GLASTERR?
     *     [结果]: <CR><LF>^GLASTERR: <type><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^GLASTERR=?
     *     [结果]: <CR><LF>^GLASTERR: (list of supported <type>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <type>: 错误类型。
     *             0：保留；
     *             1：拨号类错误；
     *             2~255：保留。
     *     <code>: 错误码。
     *             0~65535：具体错误码。
     * [示例]:
     *     · PPP拨号失败后，使用AT^GLASTERR=1查询失败的原因.
     *       AT^GLASTERR=1
     *       ^GLASTERR: 1,8
     *       OK
     *     · 查询命令
     *       AT^GLASTERR?
     *       ^GLASTERR: 1
     *       OK
     *     · 测试命令
     *       AT^GLASTERR=?
     *       ^GLASTERR: (0-255)
     *       OK
     */
    { AT_CMD_GLASTERR,
      At_SetGlastErrPara, AT_SET_PARA_TIME, At_QryGlastErrPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^GLASTERR", (VOS_UINT8 *)"(0-255)" },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 查询与网侧协商后的主、辅DNS
     * [说明]: PDP激活后，AP通过该命令查询与网侧协商的主DNS、辅DNS信息，供AP数传时使用。若该CID指定PDP未激活，设置命令直接返回ERROR。
     *         目前AP仅支持IPv4类型。
     * [语法]:
     *     [命令]: ^DNSQUERY=<cid>
     *     [结果]: <CR><LF>^DNSQUERY: <pDns>,<sDns><CR><LF> <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^DNSQUERY=?
     *     [结果]: <CR><LF>^DNSQUERY: (list of supported<cid>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cid>: 整型值，PDP 上下文标识符。
     *             LTE版本取值范围1~31。
     *             其他版本取值范围1~11。
     *     <pDns>: 字符串类型，指定的<cid>对应的主 DNS 地址。
     *     <sDns>: 字符串类型，指定的<cid>对应的辅 DNS 地址。
     * [示例]:
     *     · 查询CID为1的主、辅DNS信息（当前CID 1已经激活）
     *       AT^DNSQUERY=1
     *       ^DNSQUERY: "192.168.0.10","192.168.0.200"
     *       OK
     */
    { AT_CMD_DNSQUERY,
      AT_SetDnsQueryPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^DNSQUERY", (VOS_UINT8 *)DNSQUERY_TEST_CMD_PARA_STRING },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 设置Fast Dormancy
     * [说明]: 使无线模块快速进入休眠或者省电模式的一项功能，AP侧判断满足一定条件后，使用该指令指示或者控制Modem对Fast Dormancy功能进行配置。
     * [语法]:
     *     [命令]: ^FASTDORM=<type>[,<timer_length>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^FASTDORM?
     *     [结果]: <CR><LF>^FASTDORM: <type>,<timer_length><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^FASTDORM=?
     *     [结果]: <CR><LF>^FASTDORM: (list of supported <type>s)[, (list of
     *             supported <timer_length>s)]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <type>: 整型值，Fast Dormancy功能配置类型。
     *             0：停止休眠；
     *             1：只允许Fast Dormancy；
     *             2：只允许ASCR（Autonomous Signalling Connection Release）；
     *             3：允许ASCR和Fast Dormancy。
     *     <timer_length>: 整型值，当前状态下无流量后多久可以进入Fast Dormancy。该项为可选项，没有配置时使用之前配置的时长或UE默认的值。单位为s，取值范围1~30，默认值为5。
     * [示例]:
     *     · 设置ASCR和Fast Dormancy类型，配置无流量的统计时长为5秒
     *       AT^FASTDORM=3,5
     *       OK
     *     · 查询当前Fast Dormancy配置
     *       AT^FASTDORM?
     *       ^FASTDORM: 3,5
     *       OK
     *     · 测试命令
     *       AT^FASTDORM=?
     *       ^FASTDORM: (0,1,2,3),(1-30)
     *       OK
     */
    { AT_CMD_FASTDORM,
      AT_SetFastDormPara, AT_SET_PARA_TIME, AT_QryFastDormPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^FASTDORM", (VOS_UINT8 *)"(0,1,2,3),(1-30)" },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 查询IPV6能力
     * [说明]: 本命令用于查询IPV6能力。
     *         后台启动时，调用此AT查询当前MT的IPV6能力。
     * [语法]:
     *     [命令]: ^IPV6CAP?
     *     [结果]: <CR><LF>^IPV6CAP: <cfg_value><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^IPV6CAP=?
     *     [结果]: <CR><LF>^IPV6CAP: (list of supported <cap_value>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             IPV6能力配置数据由NV项定制，为便于扩展，此处IPv6能力列表固定显示为<0-0xFF>
     * [参数]:
     *     <cap_value>: IPV6能力配置值，取值范围：0～0xFF。
     *             0x01：IPV4 ONLY；
     *             0x02：IPV6 ONLY；
     *             0x07：IPV4 ONLY，IPV6 ONLY，IPV4V6使用相同APN；
     *             0x0B：IPV4 ONLY，IPV6 ONLY，IPV4V6使用不同APN。
     *             其他值保留。
     * [示例]:
     *     · MT仅支持IPV4
     *       AT^IPV6CAP?
     *       ^IPV6CAP: 1
     *       OK
     *     · MT仅支持IPV6
     *       AT^IPV6CAP?
     *       ^IPV6CAP: 2
     *       OK
     *     · MT支持IPV4、IPV6和IPV4V6（符合3GPP R9版本）
     *       AT^IPV6CAP?
     *       ^IPV6CAP: 7
     *       OK
     *     · 测试命令
     *       AT^IPV6CAP=?
     *       ^IPV6CAP: <0-0xFF>
     *       OK
     */
    { AT_CMD_IPV6CAP,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryIpv6CapPara, AT_NOT_SET_TIME, AT_TestIpv6capPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^IPV6CAP", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-NDIS特性
     * [含义]: 数据业务拨号
     * [说明]: 本命令用于实现NDIS拨号。
     *         at^ndisdup=1,1：NDIS拨号；
     *         at^ndisdup=1,0：断开NDIS网络连接；
     *         支持E5、STICK拨号方式；
     *         HiB5000支持使用CID 1~20进行拨号，通用业务建议只使用1-11，12-20仅用于满足特殊测试场景(如GCF测试)。
     * [语法]:
     *     [命令]: ^NDISDUP=<cid>,<connect>[,<APN>[,<username>[,<passwd>[,<authpref>][,<ipaddr>][,<bitRatType>]]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况:
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^NDISDUP?
     *     [结果]: <CR><LF>OK<CR><LF>
     *     [命令]: ^NDISDUP=?
     *     [结果]: <CR><LF>^NDISDUP: (list of supported <cid>s) , (list of supported <connect>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cid>: 整型值，PDP上下文标识符；
     *             GU为1～16（目前只支持11，后续可扩展到16）；
     *             GUL为1～20。
     *     <connect>: 操作类型。
     *             0：断开连接；
     *             1：建立连接；
     *             2：强制去激活连接（针对LTE下只存在默认承载情况下）。
     *     <APN>: 接入点名称，字符串，取值范围0-99bytes。
     *     <username>: 用户名，字符串，取值范围0-99bytes。
     *     <passwd>: 密码，字符串，取值范围0-99bytes。
     *     <authpref>: 整型值，认证协议。取值如下：
     *             GUTL模式
     *             0：NONE（不鉴权）；
     *             1：PAP；
     *             2：CHAP；
     *             3：MsChapV2（目前暂不支持）。
     *             CL模式
     *             0：NONE（不鉴权）；
     *             1：PAP；
     *             2：CHAP；
     *             3：PAP or CHAP。
     *     <ipaddr>: IP地址。格式为：192.168.11.8。
     *     <bitRatType>: 此次数据拨号可以在哪些接入技术下尝试，掩码格式省略时：对拨号不做接入技术的限制；
     *             0：对拨号不做接入技术的限制；
     *             36：可以在LTE模式下尝试拨号
     *             其他请参考下表
     *             注意：此参数只能在CL模式下使用，GUL或L单模都不可。
     * [表]: <BitRatType>bit位表
     *       bit:   7,        6,        5,        4,        3,        2,    1,     0,
     *              RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, LTE,  WCDMA, GSM,
     * [示例]:
     *     · NDIS拨号
     *       AT^NDISDUP=1,1,"HUAWEI.COM"
     *       OK
     *     · 查询命令
     *       AT^NDISDUP?
     *       OK
     *     · 测试命令
     *       AT^NDISDUP=?
     *       ^NDISDUP： (1-20),(0-1)
     *       OK
     */
    { AT_CMD_NDISDUP,
      AT_SetNdisdupPara, AT_SET_PARA_TIME, At_QryNdisdupPara, AT_QRY_PARA_TIME, At_TestNdisDup, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^NDISDUP", (VOS_UINT8 *)NDISDUP_TEST_CMD_PARA_STRING },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: DS流量清零
     * [说明]: 将DS流量清零，包括DS累计连接时间、DS累计发送流量、DS累计接收流量、最后一次DS的连接时间、最后一次DS的发送流量和最后一次DS的接收流量（此6项数据的说明请见^DSFLOWQRY命令）。以上6项数据在出厂时初始均设置为0。
     * [语法]:
     *     [命令]: ^DSFLOWCLR
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     * [示例]:
     *     · 对所有NSAPI执行分组数据统计信息清零操作
     *       AT^DSFLOWCLR
     *       OK
     */
    { AT_CMD_DSFLOWCLR,
      AT_SetDsFlowClrPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^DSFLOWCLR", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 查询DS流量
     * [说明]: 查询最后一次DS的连接时间和流量，以及DS累计的连接时间和流量。
     *         如果当前处于online_data状态，最后一次DS指的是当前这次DS，否则指的是此前最后一次发生的DS。DS累计的连接时间和流量指，DS流量最后一次清零至当前这段时间内所有发生的DS的连接时间和流量的累加值。
     *         Stick和E5形态存在以下差异：
     *         E5查询得到的数据是单板整个生命周期中的最后一次DS的连接时间、发送流量、接收流量，以及DS累计的连接时间、发送流量、接收流量。
     *         STICK查询得到的数据是单板上电期间的最后一次DS的连接时间、发送流量、接收流量，以及DS累计的连接时间、发送流量、接收流量。
     *         当FEATURE_MBB_CUST宏打开，不带CID参数时，表示查询所有的CID流量；如果带CID参数时（CID范围为1-11），则查询指定CID流量。
     * [语法]:
     *     [命令]: ^DSFLOWQRY=[cid]
     *     [结果]: <CR><LF>^DSFLOWQRY: <last_ds_time>,<last_tx_flow>, <last_rx_flow>,<total_ds_time>,<total_tx_flow>, <total_rx_flow><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <last_ds_time>: 0x0000_0000～0xFFFF_FFFF，8位16进制数，表示最后一次DS的连接时间，单位为秒。
     *     <last_tx_flow>: 0x0000_0000_0000_0000～0xFFFF_FFFF_FFFF_FFFF，16位16进制数，表示最后一次DS的发送流量，单位为字节。
     *     <last_rx_flow>: 0x0000_0000_0000_0000～0xFFFF_FFFF_FFFF_FFFF，16位16进制数，表示最后一次DS的接收流量，单位为字节。
     *     <total_ds_time>: 0x0000_0000～0xFFFF_FFFF，8位16进制数，表示DS累计的连接时间，单位为秒。
     *     <total_tx_flow>: 0x0000_0000_0000_0000～0xFFFF_FFFF_FFFF_FFFF，16位16进制数，表示DS累计的发送流量，单位为字节。
     *     <total_rx_flow>: 0x0000_0000_0000_0000～0xFFFF_FFFF_FFFF_FFFF，16位16进制数，表示DS累计的接收流量，单位为字节。
     * [示例]:
     *     · 查询DS最后一次以及累计的连接时间和流量
     *       AT^DSFLOWQRY
     *       ^DSFLOWQRY: 0000002D,0000000000019A01,0000000000736A52,0000002D,0000000000019A01,0000000000736A52
     *       OK
     *     · 查询指定CID的DS最后一次以及累计的连接时间和流量
     *       AT^DSFLOWQRY=1
     *       ^DSFLOWQRY: 0000003A,0000000000015863,0000000000834B41,0000003A, 0000000000015863,0000000000834B41
     *       OK
     */
    { AT_CMD_DSFLOWQRY,
      AT_SetDsFlowQryPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^DSFLOWQRY", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 上报DS流量
     * [说明]: 当MT处于online_data状态时，该主动上报消息2s上报一次，低功耗下CPU睡眠态或低流量如ping包场景，上报时间间隔可能大于2s。上报的内容包括当前这次DS的连接时间、当前的发送速率、当前的接收速率、当前这次DS的发送流量、当前这次DS的接收流量、与网络侧协商后确定的PDP连接发送速率和与网络侧协商后确定的PDP连接接收速率。
     *         当FEATURE_MBB_CUST宏打开，默认上报NV 2590 中CID列表总流量。宏关闭时，上报所有连接的总流量。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^DSFLOWRPT: <curr_ds_time>,<tx_rate>,<rx_rate>,<curr_tx_flow>,<curr_rx_flow>, <qos_tx_rate>, <qos_rx_rate><CR><LF>
     *     [命令]: ^DSFLOWRPT=<oper>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^DSFLOWRPT=?
     *     [结果]: <CR><LF>^DSFLOWRPT: (list of supported <oper>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <curr_ds_time>: 0x0000_0000～0xFFFF_FFFF，8位16进制数，表示当前这次DS的连接时间，单位为秒。
     *     <tx_rate>: 0x0000_0000～0xFFFF_FFFF，8位16进制数，表示当前的发送速率，单位为字节每秒。
     *     <rx_rate>: 0x0000_0000～0xFFFF_FFFF，8位16进制数，表示当前的接收速率，单位为字节每秒。
     *     <curr_tx_flow>: 0x0000_0000_0000_0000～0xFFFF_FFFF_FFFF_FFFF，16位16进制数，表示当前这次DS的发送流量，单位为字节。
     *     <curr_rx_flow>: 0x0000_0000_0000_0000～0xFFFF_FFFF_FFFF_FFFF，16位16进制数，表示当前这次DS的接收流量，单位为字节。
     *     <qos_tx_rate>: 0x0000_0000～0xFFFF_FFFF，8位16进制数，与网络侧协商后确定的PDP连接发送速率，单位为字节每秒。
     *     <qos_rx_rate>: 0x0000_0000～0xFFFF_FFFF，8位16进制数，与网络侧协商后确定的PDP连接接收速率，单位为字节每秒。
     *     <oper>: 整型值：流量上报开关。
     *             0：禁用自动流量上报；
     *             1：使能自动流量上报。
     * [示例]:
     *     · 流量上报
     *       AT^DSFLOWRPT=1
     *       OK
     *     · 测试命令
     *       AT^DSFLOWRPT=?
     *       ^DSFLOWRPT: (0,1)
     *       OK
     */
    { AT_CMD_DSFLOWRPT,
      AT_SetDsFlowRptPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^DSFLOWRPT", (VOS_UINT8 *)"(0,1)" },

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
#if (FEATURE_UE_MODE_G == FEATURE_ON)
    /* 数传GCF测试命令 */
    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 发送GCF测试数据
     * [说明]: GCF测试，在不建立拨号连接的情况下发送数据。
     *         注：此命令由FEATURE_PHONE_ENG_AT_CMD宏控制。
     * [语法]:
     *     [命令]: ^TRIG=<mode>,<nsapi>,<length>,<times>,<millisecond>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^TRIG=?
     *     [结果]: <CR><LF>^TRIG: (list of supported <mode>s),(list of supported <nsapi>s),(list of supported <length>s),(list of supported <times>s),(list of supported <millisecond>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，数据传送LLC模式。
     *             0：非确认模式。
     *             1：确认模式。
     *     <nsapi>: 整型值，指定数据发送的NSAPI。
     *             取值范围为5～15。
     *     <length>: 整型值，每次数据发送长度，单位为字节，取值范围1～65535。
     *     <times>: 整型值，发送数据次数，取值范围为1～40。
     *     <millisecond>: 整型值，每次数据发送间隔时间，单位为毫秒，取值范围1～10000。
     * [示例]:
     *     · LLC非确认模式下发送500byte数据
     *       AT^TRIG=0,5,500,1,1000
     *       OK
     *     · LLC非确认模式下发送5000byte数据
     *       AT^TRIG=0,5,1000,5,1000
     *       OK
     */
    { AT_CMD_TRIG,
      At_SetTrigPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^TRIG", (VOS_UINT8 *)"(0,1),(5-15),(1-65535),(1-40),(1-10000)" },
#endif
#endif
    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 配置数传通道/网卡与CID的映射关系
     * [说明]: 该命令供AP/APP在发起PDP激活前配置数传通道与CID的映射关系。该映射关系在PDP去激活后自动注销。上电后默认没有映射关系，下电后映射关系也失效。
     *         如果命令中只有<cid>一个参数，如AT^CHDATA=<cid>，表示删除此<cid>对应的数传通道映射关系。
     *         指定<cid>已经激活后，不允许删除或修改此<cid>对应的数传通道映射关系，直接返回ERROR。
     *         不允许将不同的<cid>映射到同一个<datachannel>上。
     * [语法]:
     *     [命令]: ^CHDATA=<cid>,<datachannel>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CHDATA?
     *     [结果]: <CR><LF>^CHDATA: <cid>,<datachannel><CR><LF>
     *             [[<CR><LF>^CHDATA: <cid>,<datachannel><CR><LF>]……]
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^CHDATA=?
     *     [结果]: <CR><LF>^CHDATA: (list of supported <cid>s), (list of supported <datachannel>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cid>: 整型值，PDP 上下文标识符。
     *             GU版本：
     *             取值范围为1～11。
     *             GUL/GUTL版本：
     *             取值范围为1～31。
     *     <datachannel>: 整型值，数传通道或网卡ID。
     *             单卡模式：取值范围为1～8。
     *             双卡模式：取值范围为1~10。
     *             三卡模式：取值范围为1~12。
     * [示例]:
     *     · 设置命令，将数传通道/网卡2与CID 1映射
     *       AT^CHDATA=1,2
     *       OK
     *     · 查询命令，假设已经将数传通道/网卡2与CID 1映射
     *       AT^CHDATA?
     *       ^CHDATA: 1,2
     *       OK
     *     · 测试命令(GU模、双卡下测试，仅在APP使用的通道如HSIC MUX或VCOM口支持)
     *       AT^CHDATA=?
     *       ^CHDATA: (1-11),(1-5)
     *       OK
     */
    { AT_CMD_CHDATA,
      AT_SetChdataPara, AT_NOT_SET_TIME, AT_QryChdataPara, AT_NOT_SET_TIME, AT_TestChdataPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CHDATA", (VOS_UINT8 *)CHDATA_TEST_CMD_PARA_STRING },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 查询拨号状态
     * [说明]: 查询拨号过程中当前所处的状态。
     *         指定CID 查询拨号状态时，只限于使用^NDISDUP 命令激活的PDP，若指定CID 未激活，则返回ERROR。
     *         MBB不支持指定CID查询，指定CID查询是提供给手机产品使用的。
     * [语法]:
     *     [命令]: ^APCONNST=<cid>
     *     [结果]: <CR><LF>^APCONNST: <status>,<IPV4>,<status>,<IPV6>[,<status>,<Ethernet>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^APCONNST?
     *     [结果]: (list of <CR><LF>^APCONNST: <cid>,<status>,<IPV4>,<status>,<IPV6>[,<status>,<Ethernet>]<CR><LF>)<CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^APCONNST=?
     *     [结果]: <CR><LF>^APCONNST: (list of supported <cid>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cid>: cid信息
     *     <status>: 拨号状态。
     *             0：正在拨号状态；
     *             1：拨号完成，建立连接；
     *             2：断开拨号完成。
     *     <pdp_type>: 取值为字符串：
     *             "IPV4"
     *             "IPV6"
     *             "Ethernet"
     * [示例]:
     *       拨号成功，指定 CID 查询拨号状态
     *     · Ipv4
     *       AT^APCONNST=1
     *       ^APCONNST: 1,"IPV4"
     *       OK
     *     · Ipv6
     *       AT^APCONNST=1
     *       ^APCONNST: 1,"IPV6"
     *       OK
     *     · Ipv4v6
     *       AT^APCONNST=1
     *       ^APCONNST: 1,"IPV4",1,"IPV6"
     *       OK
     *     · 以太网类型
     *       AT^APCONNST=1
     *       ^APCONNST: 2,"IPV4",2,"IPV6",1,”Ethernet”
     *       OK
     *     · 拨号成功，查询拨号状态
     *       AT^APCONNST?
     *       ^APCONNST: 1,1,"IPV4",1,"IPV6"
     *       OK
     */
    { AT_CMD_APCONNST,
      At_SetApConnStPara, AT_SET_PARA_TIME, At_QryApConnStPara, AT_QRY_PARA_TIME, AT_TestApConnStPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^APCONNST", (VOS_UINT8 *)"(1-11)" },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 设置用户名和密码
     * [说明]: 本地保存一组以<cid>为索引的用户名密码等，每一条保存的设置环境包含一组与握手协议相关的参数。
     *         SET命令将握手协议的一组参数存入以<cid>为索引的数据存储组中。每个数据存储组初始都是未定义的，通过SET命令存入一组参数后，则成为已定义状态。<cid>的取值范围决定了能保存的已定义的数据存储组的数目。
     *         特殊的SET命令^AUTHDATA=<cid>清除<cid>指示的存储参数，此时数据存储组返回未定义状态。
     *         READ命令返回所有已定义的参数值，各条握手协议之间换行显示。
     *         TEST命令返回所有支持的取值，各条之间换行显示。
     * [语法]:
     *     [命令]: ^AUTHDATA=<cid>[,<Auth_type>[<PLMN>[,<passwd>[,<username>]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^AUTHDATA?
     *     [结果]: <CR><LF>^AUTHDATA: <cid>, <Auth_type>, <passwd>,<username>,<PLMN> [<CR><LF>^AUTHDATA: <cid>, <Auth_type>, <passwd>,<username>,<PLMN>[...]]<CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: ^AUTHDATA=?
     *     [结果]: <CR><LF>^AUTHDATA: (list of supported <cid>s), (list of supported <Auth_type>s),,<CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: ^AUTHDATA
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cid>: 整型值，PDP上下文标识符。
     *             LTE版本取值范围0~31，12~31暂不支持。
     *             其他版本取值范围为1～11。
     *     <Auth_type>: 整型值，握手协议，取值如下：
     *             0：不使用握手协议；
     *             1：PAP；
     *             2：CHAP。
     *     <PLMN>: 字符串类型，运营商的PLMN，0～6byte。
     *     <passwd>: 字符串类型，密码，0～99byte。
     *     <username>: 字符串类型，用户名，0～99byte。
     * [示例]:
     *     · 设置用户名和密码
     *       AT^AUTHDATA=1,1,"46000","password","username"
     *       OK
     *     · 查询命令
     *       AT^AUTHDATA?
     *       ^AUTHDATA: 1,1, "password","username","46000"
     *       OK
     *     · 测试命令
     *       AT^AUTHDATA=?
     *       ^AUTHDATA: (0-31),(0-2),,
     *       OK
     */
    { AT_CMD_AUTHDATA,
      At_SetAuthdataPara, AT_SET_PARA_TIME, AT_QryAuthdataPara, AT_QRY_PARA_TIME, At_TestAuhtdata, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^AUTHDATA", (VOS_UINT8 *)AUTHDATA_CMD_PARA_STRING },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 数据业务中最大传输单元
     * [说明]: 数据业务激活之后，查询数传过程中最大传输单元，即MTU值（Maximum Transmission Unit），此命令支持查询。
     * [语法]:
     *     [命令]: ^CGMTU=<CID>
     *     [结果]: <CR><LF>^CGMTU: <CID>,<IPV4_MTU>,<IPV6_MTU>[,<non-IP_MTU>]<CR><LF><CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CGMTU=?
     *     [结果]: <CR><LF>^CGMTU: (0-31)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <CID>: 整型值，PDP上下文标识符。取值范围0~31。
     *     <IPV4_MTU>: 整型值，IPV4数传过程中协商的最大传输单元，网侧没配则填0。
     *     <IPV6_MTU>: 整型值，IPV6数传过程中协商的最大传输单元，网侧没配则填0。
     *     <non-IP_MTU>: 整型值，Ethernet、non-IP或者Unstructured协议类型数传过程中协商的最大传输单元，网侧没配则填0。
     * [示例]:
     *     · 查询Cid 1的 MTU值
     *       AT^CGMTU=1
     *       ^CGMTU: 1,1500,1500
     *       OK
     *     · 测试命令
     *       AT^CGMTU=?
     *       ^CGMTU: (0-31)
     *       OK
     */
    { AT_CMD_CGMTU,
      AT_SetCgmtuPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^CGMTU", (VOS_UINT8 *)"(1-11)" },

    /* 该命令从MBB产品线那移植过来，减少MBB定制代码 */
    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 拨号连接状态查询命令
     * [说明]: 命令^DCONNSTAT?用于查询modem 侧的所有激活cid的数据连接状态，已连接的cid给出APN、IPv4、和IPv6和Ethernet类型 的连接状态和连接的类型，未连接的仅给出cid。^DCONNSTAT=?用于查询modem 侧所有激活cid中处于连接状态（包括连接中、已连接、断开中状态）的cid。
     * [语法]:
     *     [命令]: ^DCONNSTAT?
     *     [结果]: <CR><LF>^DCONNSTAT: <cid>[,<APN>,<ipv4_stat>,<ipv6_stat>,<dconn_type>[,<ether_stat>]][<CR><LF>^DCONNSTAT:<cid>[,<APN>,<ipv4_stat>,<ipv6_stat>,<dconn_type>[,<ether_stat>] ][…]<CR><LF><CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^DCONNSTAT=?
     *     [结果]: <CR><LF>^DCONNSTAT: (list of connected<cid>s)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <cid>: cid的值
     *     <APN>: 字符串值，表示网络接入点名字，当前CID处于连接状态才返回。
     *     <ipv4_stat>: IPv4类型的连接状态，当前CID处于连接状态才返回，取值定义如下：
     *             0：连接断开
     *             1：已连接
     *     <ipv6_stat>: IPv6类型的连接状态，当前CID处于连接状态才返回，取值定义如下：
     *             0：连接断开
     *             1：已连接
     *     <dconn_type>: 表示当前已连接的连接类型，当前CID处于连接状态时才返回：
     *             1：APP拨号
     *             2：NDIS拨号
     *             3：非以上类型的拨号
     *     <ether_stat>: Ethernet类型的连接状态，当前CID处于连接状态才返回，取值定义如下：
     *             0：连接断开
     *             1：已连接
     * [示例]:
     *     · 查询所有激活cid的数据连接状态
     *       AT^DCONNSTAT?
     *       ^DCONNSTAT: 1,”Huawei.com”,1,1,1
     *       ^DCONNSTAT: 2,”Huawei1.com”,0,0,1,1
     *       ^DCONNSTAT: 3
     *       ^DCONNSTAT: 4
     *       ^DCONNSTAT: 5
     *       ^DCONNSTAT: 6
     *       ^DCONNSTAT: 7
     *       ^DCONNSTAT: 8
     *       ^DCONNSTAT: 9
     *       ^DCONNSTAT: 10
     *       ^DCONNSTAT: 11
     *       OK
     */
    { AT_CMD_DCONNSTAT,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryDconnStatPara, AT_QRY_PARA_TIME, AT_TestDconnStatPara, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^DCONNSTAT", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 设置APN漫游IP类型
     * [说明]: 本命令用于设置不同cid对应APN在漫游场景的IP类型，需要开启2554漫游APN设置NV。如果命令中只有<cid>一个参数，如AT^ROAMPDPTYPE =<cid>，表示删除此<cid>对应的IP类型。
     * [语法]:
     *     [命令]: ^ROAMPDPTYPE=<cid>,<iptype>
     *     [结果]: 执行设置成功时：
     *             <CR><LF>OK<CR><LF>
     *             设置错误时：
     *             <CR><LF>+CME ERROR: <err ><CR><LF>
     * [参数]:
     *     <cid>: 整型值，PDP上下文索引，同+CGDCONT，目前只支持0。
     *     <iptype>: IP类型，只允许设置IP，IPV6，IPV4V6三种类型，“PPP”暂不支持
     * [示例]:
     *     · 设置漫游IP类型为IP
     *       AT^ROAMPDPTYPE=0,"IP"
     *       OK
     */
    { AT_CMD_ROAMPDPTYPE,
      AT_SetRoamPdpTypePara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^ROAMPDPTYPE", (VOS_UINT8 *)ROAMPDPTYPE_CMD_PARA_STRING },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: SINGLEPDN特性开关命令
     * [说明]: 此命令用于动态的开关SINGLEPDN特性，在SINGLEPDN开启时，将8451NV定制修改（前四位修改为1,0,1,11），默认使用CID1进行注册和业务，在SINGLEPDN关闭时，反向修改使能位（前四位修改为1,0,0,0），IP type类型不在命令修改范围内。
     * [语法]:
     *     [命令]: ^SINGLEPDNSWITCH=<switch>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err> <CR><LF>
     *     [命令]: ^SINGLEPDNSWITCH?
     *     [结果]: <CR><LF>^ SINGLEPDNSWITCH: <switch><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <switch>: 整型值，特性开关
     *             取值说明：取值范围为0到1
     *             0 关闭SINGLEPDN特性
     *             1 开启SINGLEPDN特性
     * [示例]:
     *     · 设置SINGLEPDN特性开关为打开状态
     *       AT^SINGLEPDNSWITCH=1
     *       OK
     *     · 查询SINGLEPDN特性开关状态
     *       AT^SINGLEPDNSWITCH?
     *       ^ SINGLEPDNSWITCH: 1
     *       OK
     */
    { AT_CMD_SINGLEPDNSWITCH,
      AT_SetSinglePdnSwitch, AT_SET_PARA_TIME, AT_QrySinglePdnSwitch, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SINGLEPDNSWITCH", (VOS_UINT8 *)"(0,1)" },

#if (FEATURE_MBB_CUST == FEATURE_ON)
    { AT_CMD_DSAMBR,
      AT_SetDsambrPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^DSAMBR", (VOS_UINT8*)"(1)" },
#endif

    /*
     * [类别]: 协议AT-NDIS特性
     * [含义]: 查询DHCP信息
     * [说明]: 该命令用于PC查询DHCP相关IP值，包括主机IP地址、默认网关、子网掩码和DHCP服务器。
     *         拨号连接上以后，PC下发该命令以获取相应的IP地址。如果未拨号或者拨号失败，查询DHCP将返回ERROR。
     *         指定CID查询DHCP相关信息时，只限于使用^NDISDUP命令激活的PDP。若指定CID未激活，则返回ERROR。
     * [语法]:
     *     [命令]: ^DHCP=<cid>
     *     [结果]: <CR><LF>^DHCP: <clip>,<netmask>,<gate>,<dhcp>,<pDNS>,
     *             <sDNS>,<max_rx_data>,<max_tx_data><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况:<CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^DHCP?
     *     [结果]: <CR><LF>^DHCP: <clip>,<netmask>,<gate>,<dhcp>,<pDNS>,<sDNS>,<max_rx_data>,<max_tx_data><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况<CR><LF>ERROR<CR><LF>
     *     [命令]: ^DHCP=?
     *     [结果]: <CR><LF>^DHCP: (list of supported <cid>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cid>: 当前PDP激活的CID。
     *     <clip>: 主机IP地址，取值范围0x0000_0000～0xFFFF_FFFF。
     *     <netmask>: 子网掩码，取值范围0x0000_00FF～0xFCFF_FFFF。
     *     <gate>: 默认网关，取值范围0x0000_0000～0xFFFF_FFFF。
     *     <dhcp>: DHCP server的地址，取值范围0x0000_0000～0xFFFF_FFFF。
     *     <pDNS>: 首选DNS的地址，取值范围0x0000_0000～0xFFFF_FFFF。
     *     <sDNS>: 备用DNS的地址，取值范围0x0000_0000～0xFFFF_FFFF。
     *     <max_rx_data>: 最大接收速率（bit/s）。
     *     <max_tx_data>: 最大发送速率（bit/s）。
     * [示例]:
     *     · 指定CID查询DHCP
     *       AT^DHCP=1
     *       ^DHCP: 16007e0a,fcffffff,15007e0a,15007e0a,6537480a,64ff480a,7200000,384000
     *       OK
     *     · 查询DHCP
     *       AT^DHCP?
     *       ^DHCP: 16007e0a,fcffffff,15007e0a,15007e0a,6537480a,64ff480a,7200000,384000
     *       OK
     *     · 测试命令
     *       AT^DHCP=?
     *       ^DHCP: (1-11)
     *       OK
     */
    { AT_CMD_DHCP,
      At_SetDhcpPara, AT_SET_PARA_TIME, At_QryDhcpPara, AT_QRY_PARA_TIME, AT_TestDhcpPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^DHCP", (VOS_UINT8 *)"(1-11)" },

    /*
     * [类别]: 协议AT-NDIS特性
     * [含义]: 查询连接状态
     * [说明]: 查询MT的ECM（NDIS/WWAN）连接状态。
     * [语法]:
     *     [命令]: ^NDISSTATQRY?
     *     [结果]: (list of <CR><LF>^NDISSTATQRY: <cid>,<stat>[,<err>[,<wx_state>[,<PDP_type>]]] [,<stat>,<err>,<wx_state>,<PDP_type>]<CR><LF>)
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^NDISSTATQRY=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cid>: cid信息
     *     <stat>: 整型值，连接状态。
     *             0：连接断开；
     *             1：已连接；
     *             2：连接中（仅在单板自动连接时上报，暂不支持）；
     *             3：断开中（仅在单板自动连接时上报，暂不支持）。
     *     <err>: 整型值，错误码，仅在拨号失败时上报该参数。
     *             0：unknow error/ unspecified error；
     *             其他值：遵循《3GPP TS 24.008 V5.5.0 (2002-09)》及后续版本中10.5.6.6SM Cause描述。
     *     <wx_state>: 整型值，WiMax数据卡子状态，暂不支持。
     *             1：DL Synchronization；
     *             2：Handover DL acquisition；
     *             3：UL Acquisition；
     *             4：Ranging；
     *             5：Handover ranging；
     *             6：Capabilities negotiation；
     *             7：Authorization；
     *             8：Registration。
     *     <PDP_type>: 取值为字符串。
     *             "IPV4"
     *             "IPV6"
     *             "Ethernet"
     * [示例]:
     *       根据MT支持的IPV6能力上报一组或两组连接状态。
     *     · 如果MT仅支持IPV4 Only时，IPV4连接状态为已连接状态。只上报一组连接状态。
     *       AT^NDISSTATQRY?
     *       ^NDISSTATQRY: 1,1,,,"IPV4"
     *       OK
     *     · 如果MT支持IPV4V6双栈时，IPV4连接状态为未连接状态而IPV6连接状态为已连接状态。上报两组连接状态，不管是拨号前还是拨号后。
     *       AT^NDISSTATQRY?
     *       ^NDISSTATQRY: 1,0,,,"IPV4",1,,,"IPV6"
     *       OK
     *     · 如果为Ethernet类型的拨号时，连接状态为已连接状态。
     *       AT^NDISSTATQRY?
     *       ^NDISSTATQRY: 1,1,,,"Ethernet"
     *       OK
     *     · 测试此命令
     *       AT^NDISSTATQRY=?
     *       OK
     */
#if (FEATURE_MBB_CUST == FEATURE_ON)
    /* MBB产品支持通过指定CID查询拨号状态 */
    { AT_CMD_NDISSTATQRY,
      AT_SetNdisStatQryPara, AT_SET_PARA_TIME, AT_QryNdisStatPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8*)"^NDISSTATQRY", (VOS_UINT8*)"(1-11)"},
#else
    { AT_CMD_NDISSTATQRY,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryNdisStatPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^NDISSTATQRY", VOS_NULL_PTR },
#endif

    /*
     * [类别]: 协议AT-NDIS特性
     * [含义]: 查询DHCPV6信息
     * [说明]: 该命令用于查询DHCPv6相关IPv6值，包括主机IPv6地址、默认网关、子网掩码和DHCPv6 服务器。
     *         IPv6拨号连接上以后，PC下发该命令以获取相应的IPv6地址。如果未拨号或者拨号失败，查询DHCPv6将返回ERROR。
     * [语法]:
     *     [命令]: ^DHCPV6=<cid>
     *     [结果]: <CR><LF>(list of ^DHCPV6: <clip_v6>,<netmask_v6>,<gate_v6>,<dhcp_v6>,<pDNS_v6>,<sDNS_v6>,<max_rx_data>,<max_tx_data><CR><LF>)<CR><LF>OK<CR><LF>
     *             错误情况:<CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^DHCPV6?
     *     [结果]: <CR><LF>^DHCPV6: <clip_v6>,<netmask_v6>,<gate_v6>,<dhcp_v6>,<pDNS_v6>,<sDNS_v6>,<max_rx_data>,<max_tx_data><CR><LF><CR><LF>OK<CR><LF>
     *             错误情况<CR><LF>ERROR<CR><LF>
     *     [命令]: ^DHCPV6=?
     *     [结果]: <CR><LF>^DHCPV6: (list of supported <cid>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cid>: CID信息
     *     <clip_v6>: 主机IPv6地址。
     *     <netmask_v6>: IPv6子网掩码。
     *     <gate_v6>: IPv6默认网关。
     *     <dhcp_v6>: DHCPv6 server的地址F。
     *     <pDNS_v6>: 首选DNSv6的地址。
     *     <sDNS_v6>: 备用DNSv6的地址。
     *     <max_rx_data_v6>: 最大接收速率（bit/s）。
     *     <max_tx_data_v6>: 最大发送速率（bit/s）。
     * [示例]:
     *     · 查询DHCPV6
     *       AT^DHCPV6?
     *       ^DHCPV6: ::,::,:: ,::, fe80::e1ec:e44a:a28f:aeb1,::,7200000,384000
     *       OK
     *     · 指定CID查询DHCPV6
     *       AT^DHCPV6=1
     *       ^DHCPV6: ::,::,:: ,::, fe80::e1ec:e44a:a28f:aeb1,::,7200000,384000
     *       OK
     *     · 测试命令
     *       AT^DHCPV6=?
     *       ^DHCPV6: (1-11)
     *       OK
     */
    { AT_CMD_DHCPV6,
      AT_SetDhcpv6Para, AT_SET_PARA_TIME, AT_QryDhcpv6Para, AT_QRY_PARA_TIME, AT_TestDhcpv6Para, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^DHCPV6", (VOS_UINT8 *)"(1-11)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询最后一次PS域呼叫错误码
     * [说明]: 该命令用于查询最后一次PS域呼叫错误原因，包含PS域呼叫失败和网侧异常断开PS域呼叫。
     * [语法]:
     *     [命令]: ^CPSERR?
     *     [结果]: <CR><LF>^CPSERR: <cause><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^CPSERR=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cause>: 整数值，错误码
     *             0~65535：见具体错误码
     * [示例]:
     *     · PS域呼叫失败，查询失败原因
     *       AT^CPSERR?
     *       ^CPSERR: 1
     *       OK
     *     · 测试命令
     *       AT^CPSERR=?
     *       OK
     */
    { AT_CMD_CPSERR_SET,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryCPsErrPara, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CPSERR", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 无数据流量场景下，快速拆除RRC连接
     * [说明]: 与闪电精灵对接时，在无数据流量场景下，AP需要MODEM支持快速拆除RRC连接。
     *         CL模式下该命令不支持，仅在W网络制式下使用，其他网络制式都回复error。
     * [语法]:
     *     [命令]: ^RELEASERRC
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^RELEASERRC?
     *     [结果]: <CR><LF>ERROR<CR><LF>
     *     [命令]: ^RELEASERRC=?
     *     [结果]: <CR><LF>ERROR<CR><LF>
     * [参数]:
     * [示例]:
     *     · 无数据流量场景时，拆除RRC连接
     *       AT^RELEASERRC
     *       OK
     */
    { AT_CMD_RELEASERRC,
      At_SetReleaseRrc, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^RELEASERRC", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 查询IPv6Route参数
     * [说明]: 该命令用于应用查询IPv6 RA消息中相关参数取值，包括MTU、前缀，前缀的preferred lifetime和Valid lifetime。
     *         IPv6拨号连接上以后，应用下发该命令以获取相应的RA消息参数，以配置RADVD。如果未拨号或者拨号失败，查询APRAINFO将返回ERROR。
     *         指定CID查询IPv6Route参数，只限于使用^NDISDUP命令激活的PDP，若指定CID未激活，则返回ERROR。
     * [语法]:
     *     [命令]: ^APRAINFO=<cid>
     *     [结果]: <CR><LF>^APRAINFO: <APIPV6MTU>,<APPREFIX>,<APPreferredLifetime>,<APValidLifetime><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^APRAINFO?
     *     [结果]: (list of <CR><LF>^APRAINFO: <cid>,<APIPV6MTU>,<APPREFIX>,<APPreferredLifetime>,<APValidLifetime><CR><LF>)< CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^APRAINFO=?
     *     [结果]: <CR><LF>^APRAINFO:  (list of supported <cid>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cid>: cid信息
     *     <APIPV6MTU>: 十进制数字，表明在RA消息中广播的IPv6的MTU的取值。
     *     <APPREFIX>: IPV6前缀，格式为引号括起来的字符串，“XXX/YY”。XXX为RFC5952规范格式的Ipv6地址，YY为前缀长度，如400::/64，字符串长度不定。
     *     <APPreferredLifetime>: 前缀的Preferred lifetime取值，十进制数字。
     *     <APValidLifetime>: 前缀的Valid lifetime取值，十进制数字。
     * [示例]:
     *     · 指定CID，查询^APRAINFO
     *       AT^APRAINFO=1
     *       ^APRAINFO: 1280,"400::/64",300,1000
     *       OK
     *     · 查询^APRAINFO
     *       AT^APRAINFO?
     *       ^APRAINFO: 1,1280,"400::/64",300,1000
     *       OK
     *     · 测试命令
     *       AT^APRAINFO=?
     *       ^APRAINFO: (1-11)
     *       OK
     */
    { AT_CMD_APRAINFO,
      AT_SetApRaInfoPara, AT_SET_PARA_TIME, AT_QryApRaInfoPara, AT_QRY_PARA_TIME, AT_TestApRaInfoPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^APRAINFO", (VOS_UINT8 *)"(1-11)" },

    { AT_CMD_USBTETHERINFO,
      At_SetUsbTetherInfo, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_TestUsbTetherInfo, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^USBTETHERINFO", (VOS_UINT8 *)"(0,1),(rnmet_name)" },

    { AT_CMD_RMNETCFG,
      AT_SetRmnetCfg, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^RMNETCFG", (VOS_UINT8 *)RMNETCFG_CMD_PARA_STRING },

    { AT_CMD_IPV6TEMPADDR,
      AT_SetIPv6TempAddrPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestIPv6TempAddrPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^IPV6TEMPADDR", (VOS_UINT8 *)"(1-11)" },

    { AT_CMD_APDIALMODE,
      VOS_NULL_PTR, AT_SET_PARA_TIME, At_QryApDialModePara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^APDIALMODE", (VOS_UINT8 *)"(0-3),(0-2),(30-2592000)" },

    { AT_CMD_OPWORD,
      AT_SetOpwordPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^OPWORD", VOS_NULL_PTR },

    { AT_CMD_CPWORD,
      AT_SetCpwordPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CPWORD", VOS_NULL_PTR },

    { AT_CMD_DISLOG,
      AT_SetDislogPara, AT_NOT_SET_TIME, AT_QryDislogPara, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_E5_IS_LOCKED,
      (VOS_UINT8 *)"^DISLOG", (VOS_UINT8 *)"(0,1,255)" },

#if (FEATURE_SECURITY_SHELL == FEATURE_ON)
    { AT_CMD_SHELL,
      AT_SetShellPara, AT_NOT_SET_TIME, AT_QryShellPara, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SHELL", (VOS_UINT8 *)"(0-2)" },
#endif

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
    { AT_CMD_CSND,
      At_SetCsndPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE,
      (VOS_UINT8 *)"^CSND", (VOS_UINT8 *)"(0-34),(1-2048)" },
#else
    { AT_CMD_CSND,
      At_SetCsndPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE,
      (VOS_UINT8 *)"^CSND", (VOS_UINT8 *)"(5-15),(1-2048)" },
#endif

    { AT_CMD_DWINS,
      AT_SetDwinsPara, AT_SET_PARA_TIME, At_QryDwinsPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE,
      (VOS_UINT8 *)"^DWINS", (VOS_UINT8 *)"(0,1)" },

    { AT_CMD_APDSFLOWRPTCFG,
      AT_SetApDsFlowRptCfgPara, AT_SET_PARA_TIME, AT_QryApDsFlowRptCfgPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^APDSFLOWRPTCFG", (VOS_UINT8 *)"(0,1),(0-4294967295)" },

    { AT_CMD_DSFLOWNVWRCFG,
      AT_SetDsFlowNvWriteCfgPara, AT_SET_PARA_TIME, AT_QryDsFlowNvWriteCfgPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^DSFLOWNVWRCFG", (VOS_UINT8 *)"(0,1),(0-255)" },

#if (FEATURE_HUAWEI_VP == FEATURE_ON)
    { AT_CMD_VOICEPREFER,
      AT_SetVoicePreferPara, AT_SET_PARA_TIME, AT_QryVoicePreferPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^VOICEPREFER", (VOS_UINT8 *)"(0-4294967295)" },
#endif

#if (FEATURE_MBB_CUST == FEATURE_ON)
    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: LTE ATTACH PROFILE LIST定制PROFILE设置和查询
     * [说明]: 该命令用于LTE ATTACH PROFILE LIST功能。
     *         测试命令用于查询参数范围，其中INDEX为实际有效index的总个数（NV有效+TABLE表有效）
     *         设置命令根据命令模式的不同分为读取PROFILE参数和写入PROFILE参数2个功能：
     *         写入命令仅支持PROFILE 0的修改和添加，参数只有两个时表示清空profile信息。
     *         读取命令可根据传入的INDEX返回预制的有效PROFILE信息，INDEX错误则返回ERROR。
     *         当FEATURE_MBB_CUST宏打开时该命令生效。
     * [语法]:
     *     [命令]: AT^LTEPROFILE=<mode>,<index>[,<PDP_type>,<IMSI_prefix>,<APN>,<username>,<passwd>,<Auth_type>,<profilename>]
     *     [结果]: 写入模式时：
     *             <CR><LF>OK<CR><LF>
     *             读取模式时：
     *             <CR><LF>^LTEPROFILE: <index>,<PDP_type>,<IMSI_prefix>,<APN>,<UserName>,<UserPwd>,<Auth_type>,<profilename><CR><LF>OK<CR><LF>
     *             有MT相关的错误：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *             有其他错误：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: AT^LTEPROFILE=?
     *     [结果]: <CR><LF>^LTEPROFILE: (range of supported <mode>S),(range of supported <index>s),<MaxImsiPrefixLen>,<MaxApnLen>,<MaxUserNameLen>,<MaxUserPwdLen>,(range of supported <AuthType>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值
     *             命令模式：
     *             0：读取模式，读取模式时，只能有两个参数。
     *             1：写入模式，为写入APN信息。
     *             仅支持写入<index>为0的PROFILE信息，以便于下次注册时优先匹配。写入时，如果只有两个参数，表示删除原来的内容。写入或删除的PROFILE在下次注册时生效。
     *     <index>: 整型值，定制APN的索引序号。
     *             <index>为0专用于查询写入的APN信息，其他的用于查询定制的APN。
     *     <PDP_type>: 字符串类型，定制的注册IP类型：
     *             “IP”        IPV4类型
     *             “IPV6”      IPV6类型
     *             “IPV4V6”     IPV4V6双栈类型
     *     <IMSI_prefix>: 字符串类型，需要匹配的SIM卡IMSI前缀，有效长度为0-10。
     *     <APN>: 字符串类型，注册时接入点名称，有效长度为1-32。
     *     <username>: 字符串类型，注册时接入点对应的鉴权用户名，有效长度为0-32。
     *     <passwd>: 字符串类型，注册时接入点对应的鉴权密码，有效长度为0-32。
     *     <Auth_type>: 整型值，鉴权类型：
     *             0：None类型(用户名密码都没有时才可设置)
     *             1：PAP鉴权类型
     *             2：CHAP鉴权类型
     *     <profilename>: 定制Profile的名称，写入保持有效字节长度为0-18。
     *             在设置命令中，使用超长的字符会被截断。使用的是UTF8编码后的HEX形式，如”abc”，在AT命令中查询结果为616263，长度为3，”中文”查询结果为E4B8ADE69687，实际字节长度为6。
     * [示例]:
     *     · 查询参数范围
     *       AT^LTEPROFILE=?
     *       ^LTEPROFILE: (0,1),(0-10),10,32,32,32,(0-3)
     *       OK
     *     · 写入用户设置的PROFILE信息
     *       AT^LTEPROFILE=1,0,"IPV4V6",46050,"v4v6","","",0,E4B8ADE69687
     *       OK
     *     · 读取定制的第5个PROFILE信息
     *       AT^LTEPROFILE=0,5
     *       ^LTEPROFILE: 5, "IPV6","46050","ipv6","","",0, E4B8ADE69687
     *       OK
     */
    { AT_CMD_LTEPROFILE,
    AT_SetCustProfilePara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestCustProfilePara, AT_TEST_PARA_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
    (VOS_UINT8 *)"^LTEPROFILE", (VOS_UINT8 *)"(0,1),(0-65535),(\"IP\",\"IPV6\",\"IPV4V6\"),(@ImsiPrefix),(APN),(UserName),(UserPwd),(0-3),(@profileName)" },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: LTE ATTACH PROFILE LIST 开关设置与查询命令
     * [说明]: 该命令用于LTE ATTACH PROFILE LIST功能，设置命令用于开启/关闭特性，查询命令用于查询特性开关和轮训状态,当FEATURE_MBB_CUST宏打开时该命令生效。
     * [语法]:
     *     [命令]: ^LTEAPNATTACH=<switch>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             有MT相关错误时：
     *             <CR><LF>+CME ERROR: <err> <CR><LF>
     *     [命令]: ^LTEAPNATTACH?
     *     [结果]: <CR><LF>^LTEAPNATTACH: <switch>,<retry_status><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             有MT相关错误时：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <switch>: 整型值，特性开关，默认初始值为0
     *             取值说明：取值范围为0到1。
     *             0：LTE ATTACH PROFILE LIST功能未使能。
     *             1：LTE ATTACH PROFILE LIST功能使能。
     *     <retry_status>: 整型值，取值范围为0到1。
     *             0：轮询已结束。
     *             1：正在轮询中。
     *             备注:<retry_status>状态仅在没有注册时有效，如果已经注册上网，该参数无效。AP仅在没有注册上LTE时下发查询命令进行状态查询。
     * [示例]:
     *     · 设置LTE ATTACH PROFILE LIST功能开启
     *       AT^LTEAPNATTACH=1
     *       OK
     *     · 设置LTE ATTACH PROFILE LIST功能关闭
     *       AT^LTEAPNATTACH=0
     *       OK
     */
    { AT_CMD_LTEAPNATTACH,
    AT_SetCustProfileAttachSwitch, AT_SET_PARA_TIME, AT_QryCustProfileAttachSwitch, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
    (VOS_UINT8 *)"^LTEAPNATTACH", (VOS_UINT8 *)"(0,1)" },
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 发送UE Policy码流
     * [说明]: 该命令用于AP侧在收到网络侧下发的UE POLICY 信息后，对网络侧的应答回复；以及用于AP把本地保存的UPSI、UE是否支持ANDSP、UE的OS ID等参数上传给网络。
     *         注意：
     *         在message_type为MANAGE UE POLICY COMMAND REJECT或者UE state indication时，<UE_policy_information_length>参数要与后续输入的字符串码流的长度相同。
     *         在输入^CSUEPOLICY 命令之后才可以输入UE_policy_information信息（类似于发短信的处理），在UE_policy_information信息结束后以ctrl-z结尾。
     *         当UE_policy_information信息超过1500字节，需要分段下发，每段码流使用CR（回车符）结尾，总长度不超过4000字节。
     * [语法]:
     *     [命令]: ^CSUEPOLICY=<pti>,<message_type>
     *             [,<UE_policy_information_length>[,<UE_policy_classmark>[,<UE_OS_id>]]],[<protocol_ver>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CSUEPOLICY=?
     *     [结果]: <CR><LF>^CSUEPOLICY: (list of supported < pti >s), (list of supported < message_type >s), (list of supported < UE_policy_information_length >s),(< PolicyClassMarkStr >),(< osIdStr >),(list of supported < protocol_ver >s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <pti>: 整型值，取值范围（0-254）
     *             0：用于 UE STATE INDICATION类型的消息
     *             1-254：用于MANAGE UE POLICY COMPLETE 和MANAGE UE POLICY COMMAND REJECT
     *     <message_type>: 整型值，消息类型
     *             0：MANAGE UE POLICY COMPLETE
     *             1：MANAGE UE POLICY COMMAND REJECT
     *             2：UE STATE INDICATION
     *     <UE_policy_information_length>: 整型值，UE Policy Information字符串长度，取值范围（0-4000）
     *             UE Policy Information码流，
     *             码流的内容参照协议章节：
     *             3GPP TS 24.501 [161] subclause D.6.3, D.6.4
     *     <UE_policy_classmark>: 字符串类型，最多8个字符
     *             码流的内容参照协议章节：
     *             3GPP TS 24.501 [161] subclause D.6.5
     *     <UE_OS_id>: 字符串类型，最多480个字符
     *             码流的内容参照协议章节：
     *             3GPP TS 24.501 [161] subclause D.6.6
     *     <protocol_ver>: 整形值，取值如下：1510、1520、1530
     * [示例]:
     *     · AP下发MANAGE UE POLICY COMPLETE信息
     *       AT^CSUEPOLICY=1,0
     *       OK
     *     · AP下发MANAGE UE POLICY COMMAND REJECT信息
     *       AT^CSUEPOLICY=1,1,22
     *       >000901261030000100016f
     *       >ctrl-z
     *       OK
     *     · AP下发UE STATE INDICATION信息
     *       AT^CSUEPOLICY=1,2,22,"00","0123456789abcdef",1520
     *       >000901261030000100016f
     *       >ctrl-z
     *       OK
     *     · AP下发UE STATE INDICATION信息，不携带任何信息
     *       AT^CSUEPOLICY=1,2
     *       OK
     */
    { AT_CMD_CSUEPOLICY,
      AT_SetCsUePolicy, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CSUEPOLICY", (VOS_UINT8 *)"(0-254),(0,1,2),(0-4000),(PolicyClassMarkStr),(osIdStr),(1510,1520,1530)" },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 设置数据业务信令在5G下可以尝试的最大重试次数
     * [说明]: 设置数据业务信令可以尝试的最大传递次数；目前仅用于5G模式下的激活业务流程中，
     *         后续可以考虑扩展到其它流程，或者其它模式；目前仅在5G下激活流程中支持，其它流程和制式不支持，后续根据需求打开。
     * [语法]:
     *     [命令]: ^CGCONTEX=<cid>[,<process>,<times>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CGCONTEX=?
     *     [结果]: <CR><LF>^CGCONTEX: (list of <cid>s associated with configured contexts) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cid>: 整型值，PDP上下文标识符。
     *             取值范围1~31。
     *     <process>: 整型值，
     *             1: 激活流程;
     *             2: modify流程;
     *             3: 去激活流程。
     *     目前仅在5G下激活流程中支持，其它流程和制式不支持，后续根据需求打开
     *     <times>: 整型值，传输次数，取值范围1~5。
     * [示例]:
     *     · 设置命令
     *       AT^CGCONTEX=1,1,5
     *       OK
     *     · 测试命令
     *       AT^CGCONTEX=?
     *       ^CGCONTEX: (1-31),(1-3),(1-5)
     *       OK
     */
    { AT_CMD_CGCONTEX,
      AT_SetCgcontexPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_TestCgcontexPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CGCONTEX", (VOS_UINT8 *)"(1-31),(1-3),(1-5)" },

#endif

#if (FEATURE_IMS == FEATURE_ON)
    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: IMS紧急承载通知
     * [说明]: 该命令用于AP的定位业务，SUPL给modem发送该AT命令，通知modem激活IMS紧急承载，然后在紧急承载上发送定位数据，SUPL业务结束，发送该AT命令通知modem去激活IMS紧急承载。
     * [语法]:
     *     [命令]: ^IMSEMCSTAT=<Option>
     *     [结果]: <CR><LF>^IMSEMCSTAT: <Option><Status><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <Option>: 整型值，IMS紧急承载操作。
     *             0：去激活IMS紧急承载；
     *             1：激活IMS紧急承载；
     *     <Status>: 整型值，IMS紧急承载状态。
     *             0：IMS紧急承载去激活；
     *             1：IMS紧急承载激活；
     *             2：IMS紧急承载HOLD；
     * [示例]:
     *     · IMS紧急承载激活
     *       AT^IMSEMCSTAT=1
     *       ^IMSEMCSTAT: 1
     *       OK
     *     · IMS紧急承载去激活成功
     *       AT^IMSEMCSTAT=0
     *       ^IMSEMCSTAT: 0
     *       OK
     *     · IMS紧急承载去激活失败
     *       AT^IMSEMCSTAT=0
     *       ^IMSEMCSTAT: 2
     *       OK
     */
    { AT_CMD_EMC_STATUS,
      AT_SetEmcStatus, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^IMSEMCSTAT", (VOS_UINT8 *)"(0,1)" },
#endif

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 设置数据业务连接状态主动上报命令
     * [说明]: 本命令用于设置数据业务连接状态主动上报命令，可以针对不同拨号形态进行单独控制，目前只支持STICK形态拨号。
     * [语法]:
     *     [命令]: ^PSCALLRPTCMD=<mode>,<cmdtype>
     *     [结果]: 执行设置成功时：
     *             <CR><LF>OK<CR><LF>
     *             设置错误时：
     *             <CR><LF>+CME ERROR: <err ><CR><LF>
     *     [命令]: ^PSCALLRPTCMD?
     *     [结果]: <CR><LF>^PSCALLRPTCMD: <mode>, <cmdtype>CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 拨号形态
     *             0：STICK模式
     *             目前只支持STICK模式。
     *     <cmdtype>: 上报命令控制
     *             0：上报命令为^NDISSTAT。
     *             1：上报命令为携带cid的^NDISSTAT
     *             2：上报命令为^NDISSTATEX
     * [示例]:
     *     · 设置STICK拨号上报命令为^NDISSTAT
     *       AT^PSCALLRPTCMD=0,0
     *       OK
     *     · 查询当前设置
     *       AT^PSCALLRPTCMD?
     *       ^PSCALLRPTCMD: 0,0
     *       OK
     */
    { AT_CMD_ROAMPDPTYPE,
      AT_SetPsCallRptCmdPara, AT_SET_PARA_TIME, AT_QryPsCallRptCmdPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^PSCALLRPTCMD", (VOS_UINT8 *)"(0),(0-2)" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 查询LAN全局地址
     * [说明]: E5在PDP激活后查询LAN全局地址。
     *         指定CID查询LAN全局地址，只限于使用^NDISDUP命令激活的PDP，若指定CID未激活，则返回ERROR。
     *         在E5模式下支持查询，指定CID查询LAN全局地址是在手机上支持的，E5模式下不支持指定CID查询LAN全局地址。
     * [语法]:
     *     [命令]: ^APLANADDR=<cid>
     *     [结果]: <CR><LF>^APLANADDR: <ADDR>,<PrefixLen><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^APLANADDR?
     *     [结果]: (list of <CR><LF>^APLANADDR: <CID>,<ADDR>,<PrefixLen><CR><LF>)<CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^APLANADDR=?
     *     [结果]: <CR><LF>^APLANADDR: (list of supported <cid>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <CID>: cid信息。
     *     <ADDR>: LAN全局地址，由64 bits的前缀和64 bist的MAC地址组成。
     *     <PrefixLen>: 前缀长度，单位bit。
     * [示例]:
     *     · 指定CID，查询LAN全局地址
     *       AT^APLANADDR=1
     *       ^APLANADDR: "300::2",64
     *       OK
     *     · 查询LAN全局地址
     *       AT^APLANADDR?
     *       ^APLANADDR: 1,"300::2",64
     *       OK
     */
    { AT_CMD_APLANADDR,
      AT_SetApLanAddrPara, AT_SET_PARA_TIME, AT_QryApLanAddrPara, AT_QRY_PARA_TIME, AT_TestApLanAddrPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^APLANADDR", (VOS_UINT8 *)"(1-11)" },
};

/* 注册TAF组件数据业务AT命令表 */
VOS_UINT32 AT_RegisterDataTafCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atDataTafCmdTbl, sizeof(g_atDataTafCmdTbl) / sizeof(g_atDataTafCmdTbl[0]));
}

