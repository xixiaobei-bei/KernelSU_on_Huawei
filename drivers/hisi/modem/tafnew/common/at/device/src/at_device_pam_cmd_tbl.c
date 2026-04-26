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
#include "at_device_pam_cmd_tbl.h"

#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"
#include "at_device_cmd.h"
#include "at_device_comm.h"

#include "at_device_pam_set_cmd_proc.h"
#include "at_device_pam_qry_cmd_proc.h"



#define THIS_FILE_ID PS_FILE_ID_AT_DEVICE_PAM_CMD_TBL_C

static const AT_ParCmdElement g_atDevicePamCmdTbl[] = {
#if (FEATURE_PHONE_SC == FEATURE_ON)
    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 查询手机或PAD型态SIMLOCK锁网锁卡信息
     * [说明]: 该命令用于AP-Modem形态下查询手机和PAD的SIMLOCK锁网锁卡信息，提供给产线工程菜单使用。
     *         命令使用场景：工程菜单下发。
     *         命令使用限制：不需要产线鉴权即可查询，只允许从与AP对接的专用AT通道下发。
     * [语法]:
     *     [命令]: ^PHONESIMLOCKINFO?
     *     [结果]: <CR><LF>
     *             ^PHONESIMLOCKINFO: <cat>,<indicator> [,(<code_begin>,<code_end>)...]<CR><LF>
     *             ^PHONESIMLOCKINFO: <cat>,<indicator> [,(<code_begin>,<code_end>)...]<CR><LF>
     *             ^PHONESIMLOCKINFO: <cat>,<indicator> [,(<code_begin>,<code_end>)...]<CR><LF>
     *             ^PHONESIMLOCKINFO: <cat>,<indicator>[,<total>,<flag>,(<code_begin>,<code_end>)...]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^PHONESIMLOCKINFO=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cat>: 字符串类型，锁网锁卡类型，目前仅支持下述锁网类型：
     *             NET：锁网络；
     *             NETSUB：锁子网；
     *             SP：锁SP；
     *             CP： 团体锁。
     *     <indicator>: 整数类型，锁网锁卡业务对应类型的激活状态：
     *             0：未激活；
     *             1：已激活。
     *     <total_group_num>: 实际总共有total组锁网锁卡号段
     *     <flag>: 指示受限或可信名单标识
     *             0: 可信名单
     *             1: 受限名单
     *     <code_begin>: 锁网锁卡业务的起始号段，最多支持20组。
     *     <code_end>: 锁网锁卡业务的结束号段，最多支持20组。
     * [示例]:
     *     · 查询手机SIMLOCK锁网锁卡信息
     *       AT^PHONESIMLOCKINFO?
     *       ^PHONESIMLOCKINFO: NET,1,2,0,(46001,46002),(46006,46006)
     *       ^PHONESIMLOCKINFO: NETSUB,1,15,0,(4600101,4600102),(4600205,4600208),(4600601, 4600601)
     *       ^PHONESIMLOCKINFO: SP,0
     *       ^PHONESIMLOCKINFO: CP,0
     *       OK
     *     · 测试命令
     *       AT^PHONESIMLOCKINFO=?
     *       OK
     */
    { AT_CMD_PHONESIMLOCKINFO,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryPhoneSimlockInfoPara, AT_QRY_PARA_TIME, AT_TestHsicCmdPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^PHONESIMLOCKINFO", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 读锁网锁卡安全数据
     * [说明]: AP-Modem形态下读锁网锁卡安全数据。工程菜单、装备工具会用到。
     *         命令使用场景：工程菜单、产线工具。
     *         命令使用限制：不需要产线鉴权即可查询，只允许从与AP对接的专用AT通道下发。
     * [语法]:
     *     [命令]: ^SIMLOCKDATAREAD?
     *     [结果]: <CR><LF>
     *             ^SIMLOCKDATAREAD: <cat>,<indicator>, <lock_status>,<max_times>,<remain_times><CR><LF>
     *             ^SIMLOCKDATAREAD: <cat>,<indicator>, <lock_status>,<max_times>,<remain_times><CR><LF>
     *             ^SIMLOCKDATAREAD: <cat>,<indicator>, <lock_status>,<max_times>,<remain_times><CR><LF>
     *             ^SIMLOCKDATAREAD: <cat>,<indicator>,
     *             <lock_status>,<max_times>,<remain_times><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^SIMLOCKDATAREAD=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cat>: 字符串类型，锁网锁卡类型，目前仅支持下述类型锁：
     *             NET：锁网络；
     *             NETSUB：锁子网；
     *             SP：锁SP；
     *             CP：团体锁。
     *     <indicator>: 整数类型，锁网锁卡业务对应类型的激活状态：
     *             0：未激活；
     *             1：已激活。
     *     <lock_status>: 字符串类型，锁网锁卡对应类型的解锁状态，<indicator>参数为激活状态时有效，未激活时为空：
     *             READY：解锁状态；
     *             PIN：PIN锁状态；
     *             PUK：PUK锁状态。
     *     <max_times>: 整数类型，允许的最大解锁尝试次数，PIN锁定状态有效，其他状态不输出。
     *     <remain_times>: 整数类型，剩余的次数，PIN锁定状态有效，其他状态不输出。
     * [示例]:
     *     · 读simlock锁网锁卡状态：
     *       AT^SIMLOCKDATAREAD?
     *       ^SIMLOCKDATAREAD: NET,1,PIN,10,10
     *       ^SIMLOCKDATAREAD: NETSUB,1,PUK,,
     *       ^SIMLOCKDATAREAD: SP,0,,,
     *       ^ SIMLOCKDATAREAD: CP,0,,,
     *       OK
     *     · 测试命令
     *       AT^SIMLOCKDATAREAD=?
     *       OK
     */
    { AT_CMD_SIMLOCKDATAREAD,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QrySimlockDataReadPara, AT_QRY_PARA_TIME, AT_TestHsicCmdPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SIMLOCKDATAREAD", VOS_NULL_PTR },
#endif

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置手机物理号
     * [说明]: 此命令受“产线鉴权机制”保护，仅在通过产线鉴权后设置操作才能生效。
     *         该命令用于AP-Modem形态下查询修改手机或PAD设备的物理号。
     *         命令使用限制：只允许从与AP对接的专用AT通道下发，且需要产线鉴权通过才能写入和读取，测试命令不需产线鉴权。
     *         查询命令不再使用。
     * [语法]:
     *     [命令]: ^PHONEPHYNUM=<type>,<rsa_number>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     *     [命令]: ^PHONEPHYNUM?
     *     [结果]: <CR><LF>
     *             ^PHONEPHYNUM: <type>,<rsa_number><CR><LF>
     *             ^PHONEPHYNUM: <type>,<rsa_number><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     *     [命令]: ^PHONEPHYNUM=?
     *     [结果]: <CR><LF>OK<CR><LF>
     *             查询命令不再使用。
     * [参数]:
     *     <type>: 字符串类型，物理号的类型。
     *             IMEI
     *             SN
     *     <rsa_number>: 物理号通过RSA加密后生成的128字节的密文。AT命令传输时，通过Half-Byte码编码后形成256字节的字符串。
     * [示例]:
     *     · 修改设备的IMEI号
     *       AT^PHONEPHYNUM=IMEI,0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456701234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567
     *       OK
     *     · 测试命令
     *       AT^PHONEPHYNUM=?
     *       OK
     */
    { AT_CMD_PHONEPHYNUM,
      VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestHsicCmdPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^PHONEPHYNUM", (VOS_UINT8 *)"(@type),(@Phynum),(@Hmac)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 发起产线鉴权
     * [说明]: 该命令是AP-Modem形态下产线鉴权命令，在产线鉴权时发起握手，提供给产线工具使用。
     *         设置命令用于获取UE的产线鉴权握手随机数加密数据。
     *         命令使用场景：产线工具发起产线鉴权。
     *         命令使用限制：只允许从与AP对接的专用AT通道下发。
     * [语法]:
     *     [命令]: ^IDENTIFYSTART
     *     [结果]: <CR><LF>^IDENTIFYSTART: <identify_start_response><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^IDENTIFYSTART=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <identify_start_response>: 产线鉴权握手密文，将RSA加密后的128个字节密文，通过Half-Byte码编码后形成256bytes的码流发送给后台工具。
     *             产线鉴权握手密文生成方式：
     *             UE生成一个32位随机数，使用产线鉴权公钥通过RSA算法加密生成128字节的密文，并转换成256个字符的Half-Byte码字符串作为参数<identify_start_response>回复给用户。
     * [示例]:
     *     · 发起产线鉴权
     *       AT^IDENTIFYSTART
     *       ^IDENTIFYSTART: 3A2ACB4FAD9571416D858870F4860B9D3EF741E7123E0D208290F66EA736FA539BA1FCFF2B7B103B8F7513CDEF425A8EA8C473201D4572791A65F5E235C75C96AC0C23B3ECB2990D6137685E500EC9FDCBD4F5349236E344D922AFB68F4CF5C6AF8D213B3C89BD6DD0F72E42EE04639DBF0079A3ABB0A3F6352044BB2FF3C5B3
     *       OK
     *     · 测试命令
     *       AT^IDENTIFYSTART=?
     *       OK
     */
    { AT_CMD_IDENTIFYSTART,
      AT_SetIdentifyStartPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestHsicCmdPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^IDENTIFYSTART", (VOS_UINT8 *)"(@Rsa)" },

#if (FEATURE_PHONE_SC == FEATURE_ON)
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: OTA鉴权发起命令
     * [说明]: 命令使用限制：只允许从与AP对接的专用AT通道下发。
     * [语法]:
     *     [命令]: ^IDENTIFYOTASTART
     *     [结果]: <CR><LF>^IDENTIFYOTASTART: <publicId1>,<publicId2>,<publicId3>,
     *             <identifyOTAStartRsp><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <publicId1>: SC OTA使用公钥id1，十六进制数 范围（0-0xFFFFFFFF）
     *     <publicId2>: SC OTA使用公钥id2，十六进制数 范围（0-0xFFFFFFFF）
     *     <publicId3>: SC OTA使用公钥id3，十六进制数 范围（0-0xFFFFFFFF）
     *     <identifyOTAStartRsp>: 鉴权握手密文，将RSA加密后的128个字节密文，通过Half-Byte码编码后形成256bytes的码流发送给后台工具。
     *             鉴权握手密文生成方式：
     *             UE生成一个32位随机数，使用产线鉴权公钥通过RSA算法加密生成128字节的密文，并转换成256个字符的Half-Byte码字符串作为参数<identify_start_response>回复给用户。
     * [示例]:
     *     · 发起鉴权
     *       AT^IDENTIFYOTASTART
     *       ^IDENTIFYOTASTART: 0x1A2B3C4D,0x1234ABCD,0xAABBCCDD,3A2ACB4FAD9571416D858870F4860B9D3EF741E7123E0D208290F66EA736FA539BA1FCFF2B7B103B8F7513CDEF425A8EA8C473201D4572791A65F5E235C75C96AC0C23B3ECB2990D6137685E500EC9FDCBD4F5349236E344D922AFB68F4CF5C6AF8D213B3C89BD6DD0F72E42EE04639DBF0079A3ABB0A3F6352044BB2FF3C5B3
     *       OK
     */
    { AT_CMD_IDENTIFYOTASTART,
      AT_SetIdentifyStartPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^IDENTIFYOTASTART", (VOS_UINT8 *)"(@Rsa)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: OTA鉴权结束命令
     * [说明]: 命令使用限制：下发^IDENTIFYOTASTART命令发起鉴权后使用，只允许从与AP对接的专用AT通道下发。
     * [语法]:
     *     [命令]: ^IDENTIFYOTAEND=<identify_end_request>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <identify_end_ request>: 鉴权握手密文，将RSA加密后的128字节密文，通过Half-Byte码编码后形成256个字符发送给后台工具。
     *             鉴权握手密文生成方式：
     *             通过私钥对^IDENTIFYOTASTART命令的返回值解密获得一个随机数，再用私钥对该随机数加密生成<identify_end_request>。
     * [示例]:
     *     · 下发完成鉴权设置命令
     *       AT^IDENTIFYOTAEND=948f2efb7b1c4e5d5d8788580091ad5fe39cfd461a7828027fa6b17979212c049bc759ded769f1aa18b3bfc40059b4ff6be7f240f968940fdd69a0f8be992bdc5557877505e5d7bac3c19bb1788f1b6b98c329dcddf3626a747bca87a2a03a526688d7a7f854cea3908fc9369fc0e12cd506420013cb702be0215d37d60a2c39
     *       OK
     */
    { AT_CMD_IDENTIFYOTAEND,
      VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^IDENTIFYOTAEND", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: OTA写SimLock锁网锁卡数据
     * [说明]: 命令使用限制：需要通过OTA下发的鉴权，只允许从与AP对接的专用AT通道下发。
     * [语法]:
     *     [命令]: ^SIMLOCKOTADATAWRITE=<layer>,<index>,<total>,<simlock_data>[,<hmac>]
     *     [结果]: <CR><LF>
     *             ^SIMLOCKOTADATAWRITE:<index><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <layer>: 需要写入那一层的锁卡信息。255/0/1/2/3
     *             255：锁网锁卡数据头
     *             0：NET
     *             1：NETSUB
     *             2：SP
     *             3：CP
     *     <index>: 第几包数据，取值范围1-255，要求从1开始计数，每次递增；
     *     <total>: 数据下发支持分包下发，需要分包的总包数取值范围为1-255，index值不能大于total；
     *     <simlock_data>: SimLock数据结构的二进制码流，可能包括锁卡类型、激活状态、……、PIN 码最大解锁次数、号段组数和锁网号段数组及其校验密文，CK、UK使用私钥进行RSA加密后的密文。
     *             传输时，通过Half-Byte编码成字符串。如果需要拆包，则是拆包后的SimLock数据。
     *             长度限制为1400个字符。
     *     <hmac>: 未拆包时的SimLock数据做完整性保护校验生成的摘要，Half- Byte编码下发。最后一包必须携带hmac。长度限制为64个字符。
     * [示例]:
     *     · 写simlock锁网锁卡数据（分两包下发）：
     *       AT^SIMLOCKOTADATAWRITE=0,1,2,012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345670123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456701234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345670123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456701234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345670123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456
     *       ^SIMLOCKOTADATAWRITE：1
     *       OK
     *       AT^SIMLOCKOTADATAWRITE=0,2,2,012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345670123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456701234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345670123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456701234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345670123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456,7890123456789012345678901234567012345678901234
     *       ^SIMLOCKOTADATAWRITE：2
     *       OK
     */
    { AT_CMD_SIMLOCKDATAWRITEEX,
      VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SIMLOCKOTADATAWRITE", (VOS_UINT8 *)SIMLOCKDATAWRITEEX_CMD_PARA_STRING },
#endif

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 完成产线鉴权
     * [说明]: 该命令用于AP-Modem形态下产线在鉴权时完成握手过程，提供给产线工具使用。
     *         设置命令由后台下发，参数为^IDENTIFYSTART命令返回的密文通过私钥解密出的随机数经过私钥加密后的密文。
     *         命令使用场景：产线工具完成产线鉴权。
     *         命令使用限制：下发^IDENTIFYSTART命令发起产线鉴权后使用，只允许从与AP对接的专用AT通道下发。
     * [语法]:
     *     [命令]: ^IDENTIFYEND=<identify_end_request>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^IDENTIFYEND=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <identify_end_ request>: 产线鉴权握手密文，将RSA加密后的128字节密文，通过Half-Byte码编码后形成256个字符发送给后台工具。
     *             产线鉴权握手密文生成方式：
     *             通过私钥对^IDENTIFYSTART命令的返回值解密获得一个随机数，再用私钥对该随机数加密生成<identify_end_request>。
     * [示例]:
     *     · 下发完成产线鉴权设置命令
     *       AT^IDENTIFYEND=948f2efb7b1c4e5d5d8788580091ad5fe39cfd461a7828027fa6b17979212c049bc759ded769f1aa18b3bfc40059b4ff6be7f240f968940fdd69a0f8be992bdc5557877505e5d7bac3c19bb1788f1b6b98c329dcddf3626a747bca87a2a03a526688d7a7f854cea3908fc9369fc0e12cd506420013cb702be0215d37d60a2c39
     *       OK
     *     · 测试命令
     *       AT^IDENTIFYEND=?
     *       OK
     */
    { AT_CMD_IDENTIFYEND,
      VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestHsicCmdPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^IDENTIFYEND", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: HUK数据写命令
     * [说明]: AP-Modem形态下产线使用^HUK写HUK数据到单板，无产线鉴权和加密的要求，以明文发送。
     *         命令使用场景：产线生产时使用。
     *         命令使用限制：一块单板仅允许一次写入，UE拒绝重复下发的写操作，只允许从与AP对接的专用AT通道下发。
     * [语法]:
     *     [命令]: ^HUK=<huk>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^HUK=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <huk>: UE设备的硬件唯一码（Hardware Unique key）；
     *             原始HUK信息是128BIT，AT命令输入的格式为Half-Byte码编码，需要2个字符表示一个8BIT数据，需要32个字符表示HUK的128bit数据；
     * [示例]:
     *     · 写入HUK到单板
     *       AT^HUK=01010202030304040101020203030404
     *       OK
     */
    { AT_CMD_HUK,
      AT_SetHukPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestHsicCmdPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^HUK", (VOS_UINT8 *)"(@huk)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置产线鉴权公钥
     * [说明]: 由于产品线安全红线要求，SHA算法由256升级为512，原有FACAUTHPUBKEY不满足安全红线升级后的设置需求，新增此命令替换原有命令，在Chicago及后续平台支持此命令。
     *         该命令用于AP-Modem形态下设置产线鉴权过程中，平台用于加密随机数的公钥。
     *         使用本命令前需要先通过AT命令^HUK设置设备的HUK码。
     *         命令使用场景：产线工具使用。
     *         命令使用限制：只允许从与AP对接的专用AT通道下发。
     * [语法]:
     *     [命令]: ^FACAUTHPUBKEYEX=<index>,<total>,<pubkey>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^FACAUTHPUBKEYEX=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <index>: 当前码流分段索引，有效范围1~255
     *     <total>: 码流分段总数，有效范围1~255,
     *     <pubkey>: 产线生成的密钥码流，由于AT命令处理能力极限为1600字节，因此此码流长度不能超过1570字节
     * [示例]:
     *     · 设置产线鉴权公钥(分两段下发)
     *       AT^FACAUTHPUBKEYEX=1,2,data1
     *       OK
     *       AT^FACAUTHPUBKEYEX=2,2,data2
     *       OK
     *     · 测试命令
     *       AT^FACAUTHPUBKEYEX=?
     *       OK
     */
    { AT_CMD_FACAUTHPUBKEYEX,
      VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestHsicCmdPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^FACAUTHPUBKEYEX", (VOS_UINT8 *)"(1-20),(1-20),(@Pubkey)" },

#if (FEATURE_PHONE_SC == FEATURE_ON)
    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 解锁锁网锁卡
     * [说明]: 该命令用于AP-Modem形态下解锁SimLock锁网锁卡功能。允许在无(U)SIM卡或插入合法(U)SIM卡的情况下对SimLock锁网锁卡功能进行解锁操作。该解锁操作不受当前锁状态的限制，若解锁的锁网锁卡类型未锁或已解锁则直接返回OK。
     *         该命令只在与AP对接的专用AT通道收到有效，其他AT通道接收到均认为失败。
     *         在插入非法(U)SIM卡的情况下使用此命令解锁需要重启UE后才能生效。
     * [语法]:
     *     [命令]: ^SIMLOCKUNLOCK=<cat>,<password>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^SIMLOCKUNLOCK=?
     *     [结果]: <CR><LF>^SIMLOCKUNLOCK: (list of supported <cat>s)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <cat>: 字符串类型，锁网锁卡类型，目前仅支持下述三种锁网类型：
     *             NET：锁网络；
     *             NETSUB：锁子网；
     *             SP：锁SP。
     *     <password>: 字符串类型，解锁对应锁网锁卡类型的PIN码或PUK码。密码为固定长度16的数字字符串，每个字符的取值范围为“0”~“9”。
     * [示例]:
     *     · 解锁NET网络
     *       AT^SIMLOCKUNLOCK="NET","0123456789012345"
     *       OK
     *     · 执行测试命令
     *       AT^SIMLOCKUNLOCK=?
     *       ^SIMLOCKUNLOCK: ("NET","NETSUB","SP")
     *       OK
     */
    { AT_CMD_SIMLOCKUNLOCK,
      AT_SetSimlockUnlockPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestSimlockUnlockPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SIMLOCKUNLOCK", (VOS_UINT8 *)"(\"NET\",\"NETSUB\",\"SP\"),(pwd)" },
#endif

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 切换AT通道到OM通道
     * [说明]: 本命令用于将AT通道切向OM通道，切换后该端口AT功能失效。
     * [语法]:
     *     [命令]: ^AT2OM
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>CME ERROR: <err><CR><LF>
     * [示例]:
     *     · 设置AT通道切向OM通道
     *       At^AT2OM
     *       OK
     */
    { AT_CMD_AT2OM,
      At_SetAt2OmPara, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_STICK,
      (TAF_UINT8 *)"^AT2OM", TAF_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 写SimLock锁网锁卡数据
     * [说明]: 该命令用于AP-Modem形态下写SimLock锁网锁卡数据，写锁卡数据时：有头数据先写头数据，再写每一层锁卡数据。按照锁卡头数据（如果有）、锁网、锁子网、锁SP、锁CP、锁卡数据顺序写入。一次仅允许进行一种锁网锁卡类型数据输入（如果一条AT命令下发不完可以分包下发）。
     *         该命令需要产线鉴权通过才能执行，设置的锁网锁卡信息重启后才能正常使用。
     *         命令使用场景：产线工具下发。
     *         命令使用限制：需要通过产线鉴权，只允许从与AP对接的专用AT通道下发。
     * [语法]:
     *     [命令]: ^SIMLOCKDATAWRITEEX=<layer>,<index>,<total>,<simlock_data>[,<hmac>]
     *     [结果]: <CR><LF>
     *             ^SIMLOCKDATAWRITEEX: <index><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^SIMLOCKDATAWRITE=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <layer>: 需要写入那一层的锁卡信息。255/0/1/2/3
     *             255：锁网锁卡数据头
     *             0：NET
     *             1：NETSUB
     *             2：SP
     *             3：CP
     *     <index>: 第几包数据，取值范围1-255，要求从1开始计数，每次递增；
     *     <total>: 数据下发支持分包下发，需要分包的总包数取值范围为1-255，index值不能大于taotal；
     *     <simlock_data>: SimLock数据结构的二进制码流，可能包括锁卡类型、激活状态、……、PIN 码最大解锁次数、号段组数和锁网号段数组及其校验密文，CK、UK使用私钥进行RSA加密后的密文。
     *             传输时，通过Half-Byte编码成字符串。如果需要拆包，则是拆包后的SimLock数据。
     *             长度限制为1400个字符。
     *     <hmac>: 未拆包时的SimLock数据做完整性保护校验生成的摘要，Half- Byte编码下发。最后一包必须携带hmac。长度限制为64个字符。
     * [示例]:
     *     · 写simlock锁网锁卡数据（分两包下发）：
     *       AT^SIMLOCKDATAWRITEEX=0,1,2,
     *       012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345670123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456701234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345670123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456701234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345670123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456
     *       ^SIMLOCKDATAWRITEEX: 1
     *       OK
     *       AT^SIMLOCKDATAWRITEEX=0,2,2,
     *       012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345670123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456701234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345670123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456701234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345670123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456,7890123456789012345678901234567012345678901234
     *       ^SIMLOCKDATAWRITEEX: 2
     *       OK
     *     · 异常场景（index不是从1开始，index大于total）：
     *       AT^SIMLOCKDATAWRITEEX=0,3,2,
     *       012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345670123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456701234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345670123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456701234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345670123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456
     *       +CME：ERROR
     *     · 测试命令
     *       AT^SIMLOCKDATAWRITEEX=?
     *       OK
     */
    { AT_CMD_SIMLOCKDATAWRITEEX,
      VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestHsicCmdPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SIMLOCKDATAWRITEEX", (VOS_UINT8 *)SIMLOCKDATAWRITEEX_CMD_PARA_STRING },

#if (FEATURE_PHONE_SC == FEATURE_ON)
    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 查询手机或PAD型态SIMLOCK锁网锁卡信息
     * [说明]: 该命令用于AP-Modem形态下查询手机和PAD的SIMLOCK锁网锁卡信息，提供给产线使用，用于回读校验写入手机的锁网锁卡信息，查询的信息和AT^SIMLOCKDATAWRITEEX的数据相比对。
     *         命令使用场景：产线工具。
     *         命令使用限制：不需要产线鉴权即可查询，只允许从与AP对接的专用AT通道下发。
     * [语法]:
     *     [命令]: ^SIMLOCKDATAREADEX=<layer>,<index>
     *     [结果]: <CR><LF>
     *             ^SIMLOCKDATAREADEX: <layer>,<index>,<total>,<simlock_data>
     *             <CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^SIMLOCKDATAREADEX=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <layer>: 需要写入那一层的锁卡信息。255/0/1/2/3
     *             255：锁网锁卡数据头
     *             0：NET
     *             1：NETSUB
     *             2：SP
     *             3：CP
     *     <index>: 当前上报处于第几包数据，从1开始。
     *     <total>: 总数据包数。每包携带最大字符数为1400.
     *     <simlock_data>: 锁卡数据码流。
     * [示例]:
     *     · 查询手机SIMLOCK锁网锁卡信息
     *       AT^SIMLOCKDATAREADEX=0,1
     *       ^SIMLOCKDATAREADEX: 0,1,1,
     *       012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345670123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456701234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345670123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456701234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345670123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456
     *       OK
     *     · 测试命令
     *       AT^SIMLOCKDATAREADEX=?
     *       OK
     */
    { AT_CMD_SIMLOCKDATAREADEX,
      AT_SimLockDataReadExPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestHsicCmdPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SIMLOCKDATAREADEX", (VOS_UINT8 *)SIMLOCKDATAREADEX_CMD_PARA_STRING },
#endif

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: eSIM切卡
     * [说明]: 该命令实现eUICC与UICC的电路切换。
     *         本命令只有当对应卡槽卡去激活情况或者无卡时才能下发。
     * [语法]:
     *     [命令]: ^ESIMSWITCH=<sci_num>,<card_type>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^ESIMSWITCH?
     *     [结果]: <CR><LF>^ESIMSWITCH: < card_type >,< card_type > CR><LF> <CR><LF>OK<CR><LF>
     *     [命令]: ^ESIMSWITCH=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <sci_num>: 卡槽号，目前取值0或者1（代表卡槽0或者卡槽1）。
     *     <card_type>: 需切换到的电路类型，目前取值0或者1（代表切换到UICC电路或者eUICC电路）。
     * [示例]:
     *     · 卡槽1从UICC电路切换到eUICC电路
     *       AT^ESIMSWITCH=1,1
     *       OK
     *     · 查询当前电路类型：卡槽0/1 都处于UICC电路
     *       AT^ESIMSWITCH?
     *       ^ESIMSWITCH: 0,0
     *       OK
     *     · 查询当前电路类型：卡槽0 UICC电路，卡槽1 eUICC电路
     *       AT^ESIMSWITCH?
     *       ^ESIMSWITCH: 0,1
     *       OK
     *     · 测试ESIMSWITCH
     *       AT^ESIMSWITCH=?
     *       OK
     */
    { AT_CMD_ESIMSWITCH,
      At_SetEsimSwitchPara, AT_SET_PARA_TIME, At_QryEsimSwitch, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^ESIMSWITCH", (VOS_UINT8 *)"(0,1),(0,1)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: Profile清空
     * [说明]: 在CW工位使用清除命令 AT^ESIMCLEAN 清除eSIM卡中数据。
     *         本命令只可以在CW工位CP鉴权后使用。
     * [语法]:
     *     [命令]: ^ESIMCLEAN
     *     [结果]: CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     * [示例]:
     *     · 清除eSIM内存操作
     *       AT^ESIMCLEAN
     *       OK
     */
    { AT_CMD_ESIMCLEAN,
      At_SetEsimCleanProfile, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^ESIMCLEAN", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 检测Profile是否为空
     * [说明]: 在CW工位使用 AT^ESIMCHECK? 命令（使用之前需要SIM卡完成一次重启操作）获取eSIM卡中的Profile信息。
     * [语法]:
     *     [命令]: ^ESIMCHECK?
     *     [结果]: <CR><LF>^ESIMCHECK: <result>CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <result>: 0表示没有Profile文件，1表示有Profile文件。
     * [示例]:
     *     · 检测Profile操作
     *       AT^ESIMCHECK?
     *       ^ESIMCHECK: 0
     *       OK
     */
    { AT_CMD_ESIMCHECK,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryEsimCheckProfile, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^ESIMCHECK", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 获取ESIM EID
     * [说明]: 产线使用，查询eSIM芯片EID信息。
     * [语法]:
     *     [命令]: ^ESIMEID?
     *     [结果]: <CR><LF>^ESIMEID: <eid>
     *             CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <eid>: eSIM芯片的EID信息，32个数字。
     * [示例]:
     *     · 查询EID操作
     *       AT^ESIMEID?
     *       ^ESIMEID: "89033024010400000100000000005353"
     *       OK
     */
    { AT_CMD_ESIMEID,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryEsimEid, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^ESIMEID", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 获取ESIM PKID
     * [说明]: 产线使用，查询eSIM芯片中PKID的值。
     * [语法]:
     *     [命令]: ^PKID?
     *     [结果]: <CR><LF>^PKID: <num>,<pkid_1>,…,<pkid_n>
     *             CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <num>: PKID的个数，最大支持10个
     *     <pkid_n>: 具体的PKID的值。
     * [示例]:
     *     · 查询PKID操作
     *       AT^PKID?
     *       ^PKID: 2,"041481370F5125D0B1D408D4C3B232E6D25E795BEBFB","041416B5D16048E3EA02BD4B606E5F77A4BF20808D83"
     *       OK
     */
    { AT_CMD_PKID,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryEsimPKID, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^PKID", VOS_NULL_PTR },

#if (FEATURE_SC_NETWORK_UPDATE == FEATURE_ON)
    { AT_CMD_SIMLOCKNWDATAWRITE,
      VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestHsicCmdPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SIMLOCKNWDATAWRITE", (VOS_UINT8 *)SIMLOCKDATAWRITEEX_CMD_PARA_STRING },
#endif

#if (FEATURE_PHONE_SC == FEATURE_ON)
    { AT_CMD_GETMODEMSCID,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryModemScIdPara, AT_QRY_PARA_TIME, AT_TestHsicCmdPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^GETMODEMSCID", VOS_NULL_PTR },
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: IC DIE ID读取命令
     * [说明]: 本命令用于读取多种芯片的DIE ID；
     * [语法]:
     *     [命令]: ^RFICIDEX?
     *     [结果]: 执行成功时：
     *             <CR><LF>^RFICIDEX: <index>,<type>,<die id><CR><LF>
     *             <CR><LF>^RFICIDEX: <index>,<type>,<die id><CR><LF>
     *             ……
     *             <CR><LF>OK<CR><LF>
     *             若底层返回失败时返回：
     *             <CR><LF>^RFICIDEX: <err_code><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <index>: 上报的序号：0，1，2，3……
     *     <type>: 芯片类型，这个参数由底层和AP或者工具约定
     *     <die id>: 字符串类型，die id码流，16进制输出，不同IC长度不同
     *     <err_code>: 底层错误码
     * [示例]:
     * [示例]:
     *       AT^RFICIDEX?
     *       ^RFICIDEX: 0,0,
     *       "0000000000000000000000000000000000000000000000000000000000000000"
     *       ^RFICIDEX: 1,1, "0000000000000000000000000000000000000000000000000000000000000000"
     *       ^RFICIDEX: 2,1, "0000000000000000000000000000000000000000000000000000000000000000"
     *       OK
     */
    { AT_CMD_RFICIDEX,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryRficDieIDExPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^RFICIDEX", VOS_NULL_PTR },
#endif

};

/* 注册PAM装备AT命令表 */
VOS_UINT32 AT_RegisterDevicePamCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atDevicePamCmdTbl, sizeof(g_atDevicePamCmdTbl) / sizeof(g_atDevicePamCmdTbl[0]));
}

