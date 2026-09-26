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
#include "at_device_drv_cmd_tbl.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"

#include "at_device_drv_set_cmd_proc.h"
#include "at_device_drv_qry_cmd_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_DEVICE_DRV_CMD_TBL_C

static const AT_ParCmdElement g_atDeviceDrvCmdTbl[] = {

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 获取电池电压
     * [说明]: 该命令仅适用于带电池的终端产品（目前包括E5、PAD、手机）。获取电池的电压值，指电池向负载提供的输出电压值。
     * [语法]:
     *     [命令]: ^TBATVOLT?
     *     [结果]: <CR><LF>^TBATVOLT: <result><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <result>: 电池电压值，单位为mV，取值范围为0～65535。
     * [示例]:
     *     · 查询电池电压
     *       AT^TBATVOLT?
     *       ^TBATVOLT: 3700
     *       OK
     */
    { AT_CMD_BATVOL,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryBatVolPara, AT_QRY_PARA_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^TBATVOLT", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 信息备份
     * [说明]: 进行升级前的信息备份。若产品不支持可直接返回ERROR。
     * [语法]:
     *     [命令]: ^INFORBU
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [示例]:
     *     · 信息备份成功
     *       AT^INFORBU
     *       0
     *       OK
     *     · 信息备份失败
     *       AT^INFORBU
     *       ERROR
     */
    { AT_CMD_INFORBU,
      atSetNVFactoryBack, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^INFORBU", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 读取版本号
     * [说明]: 读取版本号，包括软件版本、硬件版本、产品型号。
     * [语法]:
     *     [命令]: ^VERSION?
     *     [结果]: <CR><LF>^VERSION: BDT:<build_time>
     *             <CR><LF>^VERSION: EXTS:<external_sw_version>
     *             <CR><LF>^VERSION: INTS:<internal_sw_version>
     *             <CR><LF>^VERSION: EXTD:<external_db_version>
     *             <CR><LF>^VERSION: INTD:<internal_db_version>
     *             <CR><LF>^VERSION: EXTH:<external_hw_version>
     *             <CR><LF>^VERSION: INTH:<internal_hw_version>
     *             <CR><LF>^VERSION: EXTU:<external _dut_name>
     *             <CR><LF>^VERSION: INTU:<internal _dut_name>
     *             <CR><LF>^VERSION: CFG:<configurate_version>
     *             <CR><LF>^VERSION: PRL:<prl_version><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <build_time>: 版本编译时间，格式如Oct 15 2009 16:27:00。
     *     <external_db_version>: 外部后台软件版本号。
     *     <external_hw_version>: 外部硬件版本号，格式如Ver.A。
     *     <internal_hw_version>: 内部硬件版本号。
     *     <external_dut_name>: 外部产品名称，格式如E122。
     *     <internal_dut_name>: 内部产品名称。
     *     <configurate_version>: 配置文件版本号，格式如1004。
     *     <prl_version>: PRL版本号，C制式专用。
     * [示例]:
     *     · 读取所有相关版本号
     *       AT^VERSION?
     *       ^VERSION: BDT:Jul  4 2011, 21:33:25
     *       ^VERSION: EXTS:21.131.00.00.00
     *       ^VERSION: INTS:
     *       ^VERSION: EXTD:PCSV22.001.04.00.03
     *       ^VERSION: INTD:
     *       ^VERSION: EXTH:CH2E353SM
     *       ^VERSION: INTH:
     *       ^VERSION: EXTU:E353
     *       ^VERSION: INTU:
     *       ^VERSION: CFG:1004
     *       ^VERSION: PRL:
     *       OK
     */
    { AT_CMD_VERSION,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryVersion, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^VERSION", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置连接器管脚电平
     * [说明]: 设置或者查询产品的连接器管脚（对应GPIO）电平。若产品不支持直接返回ERROR。
     * [语法]:
     *     [命令]: ^GPIOPL=<PL>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^GPIOPL?
     *     [结果]: <CR><LF>^GPIOPL: (@GPIOPL)<PL><CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <PL>: GPIO管脚电平，按位表示，0为低电平；1为高电平。
     *             AT未设置部分默认为0，数值使用16进制数表示，需要带上0x标识。数据最大长度为20byte。多个字节间低位在前，字节内部按照bit位高低排序，没有使用到的高位在命令中省略。
     * [示例]:
     *     · 设置GPIO第2和第4个管脚为高电平
     *       AT^GPIOPL=0x0A
     *       OK
     *     · 测试命令
     *       AT^GPIOPL=?
     *       ^GPIOPL: (@GPIOPL)
     *       OK
     */
    { AT_CMD_GPIOPL,
      At_SetGPIOPL, AT_SET_PARA_TIME, At_QryGPIOPL, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^GPIOPL", (VOS_UINT8 *)"(@GPIOPL)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 恢复定制默认值
     * [说明]: 设置定制项为默认值。若产品不支持可直接返回ERROR。
     * [语法]:
     *     [命令]: ^CSDFLT
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^CSDFLT?
     *     [结果]: <CR><LF>^CSDFLT: <value_info><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^CSDFLT=?
     *     [结果]: <CR><LF>^CSDFLT: (list of supported < value_info >s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <value_info>: 定制值信息，长度为一个字节。
     *             0：定制项为单板默认值；
     *             1：定制项为非单板默认值。
     * [示例]:
     *     · 默认值恢复
     *       AT^CSDFLT
     *       OK
     *     · 测试命令
     *       AT^CSDFLT=?
     *       ^CSDFLT: (0,1)
     *       OK
     */
    { AT_CMD_CSDFLT,
      At_SetCsdfltPara, AT_NOT_SET_TIME, At_QryCsdfltPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CSDFLT", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 测试电池
     * [说明]: 该命令仅适用于带电池的终端产品。获取电池的电压值，电池电量等信息。
     * [语法]:
     *     [命令]: ^TBAT=<type>,<opr>[,<value1>[,<value2>]]
     *     [结果]: <CR><LF>^TBAT: <type>[,<value>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^TBAT?
     *     [结果]: <CR><LF>^TBAT: <mount type><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^TBAT=?
     *     [结果]: <CR><LF>^TBAT: (list of supported <type>s),(list of supported <opr>s) ,(list of supported <value1>s) ,(list of supported <value2>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <type>: 测试项目类型。
     *             0：电池电压模拟值；
     *             1：电池电压数字值（即电压的HKADC值），对应<value>取值范围为0～65535。
     *     <opr>: 表示数据流操作方向，分为读取和设置两种，不是所有的测试项目都支持设置操作，需要根据具体项目确定。
     *             0：从单板读取；
     *             1：设置单板。
     *     <value>: 查询得到的电池电压数字值。
     *     <value1>: 设置电池电压数字值时有效，3.4V对应的HKADC值。
     *     <value2>: 设置电池电压数字值时有效，4.2V对应的HKADC值。
     *     <mount type>: 电池安装方式。
     *             0：无电池；
     *             1：可更换电池；
     *             2：内置一体化电池。
     * [示例]:
     *     · 设置电池电压数字值3.4V对应的HKADC值为340，4.2V对应的HKADC值为420
     *       AT^TBAT=1,1,340,420
     *       OK
     *     · 查询电池电压数字值，查询到的电池电压数字值为370
     *       AT^TBAT=1,0
     *       ^TBAT: 1,370
     *       OK
     *     · 查询电池安装方式，单板电池的安装方式为可更换电池
     *       AT^TBAT?
     *       ^TBAT: 1
     *       OK
     *     · 测试命令
     *       AT^TBAT=?
     *       ^TBAT: (0,1),(0,1),(0-65535),(0-65535)
     *       OK
     */
    { AT_CMD_TBAT,
      AT_SetTbatPara, AT_SET_PARA_TIME, AT_QryTbatPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^TBAT", (VOS_UINT8 *)"(0,1),(0,1),(0-65535),(0-65535)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 查询PA/SIM/电池温度
     * [说明]: AP上的工程菜单使用该命令向MODEM查询PA、SIM卡和电池的温度信息，其中PA包括G PA、W PA和L PA。
     *         该命令为平台化命令，该版本不支持L模，故参数<L PAtemp>返回无效值。
     *         AP-Modem形态中，因为Modem侧不控制电池，故参数<BATTERYtemp>返回无效值。
     *         AP-Modem形态中，目前硬件上没有做温度监测设计，故SIM卡温度返回无效值。
     *         G PA和W PA通过同一个热敏电阻检测温度，底软通过同一个寄存器读出温度，故同一时间读出的这两个器件温度相同。
     * [语法]:
     *     [命令]: ^CHIPTEMP?
     *     [结果]: <CR><LF>^CHIPTEMP: <G PAtemp>,<W PAtemp>, <L PAtemp>,<SIMtemp>,<BATTERYtemp><CR><LF> <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^CHIPTEMP=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <G PAtemp>: 整型值，GSM PA芯片当前的温度，取值范围-200~850，单位为0.1℃。
     *     <W PAtemp>: 整型值，WCDMA PA芯片当前的温度，取值范围-200~1250，单位为0.1℃。
     *     <L PAtemp>: 整型值，LTE PA芯片当前的温度，当前不支持，直接返回65535。
     *     <SIMtemp>: 整型值，SIM卡当前的温度，取值范围10~100，单位为1℃。
     *     <BATTERYtemp>: 整型值，电池当前的温度，取值范围-100~550，单位为0.1℃。
     * [示例]:
     *     · 查询PA、SIM卡和电池的温度信息
     *       AT^CHIPTEMP?
     *       ^CHIPTEMP: 390,390,65535,65535,65535
     *       OK
     */
    { AT_CMD_CHIPTEMP,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryChipTempPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CHIPTEMP", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 查询NV备份是否存在
     * [说明]: 该命令用来检查产品的NV备份是否存在。
     *         该命令为平台化命令，目前只在AP-Modem形态下有效。
     * [语法]:
     *     [命令]: ^NVBACKUPSTAT?
     *     [结果]: <CR><LF>^NVBACKUPSTAT: <backupstat><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^NVBACKUPSTAT=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <backupstat>: 整型值，NV备份情况，取值范围为0，1：
     *             0：NV备份不存在。
     *             1：NV备份存在。
     * [示例]:
     *     · 查询NV备份是否存在
     *       AT^NVBACKUPSTAT?
     *       ^NVBACKUPSTAT: 1
     *       OK
     */
    { AT_CMD_NVBACKUPSTAT,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryNvBackupStatusPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^NVBACKUPSTAT", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 查询NAND FLASH所有坏块索引
     * [说明]: AP上的工程菜单使用该命令向MODEM查询NAND FLASH所有坏块的索引列表。
     *         该命令为平台化命令，目前只在AP-Modem形态下有效；索引列表最多输出100个坏块索引。
     * [语法]:
     *     [命令]: ^NANDBBC?
     *     [结果]: <CR><LF>^NANDBBC: <totalNum>
     *             [[<CR><LF>^NANDBBC: <index>]……] <CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^NANDBBC=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <totalNum>: 整型值，NAND FLASH所有坏块的总个数。取值范围：0~65535。
     *     <index>: 整型值，NAND FLASH坏块的索引值。取值范围：0~65535。
     * [示例]:
     *     · 查询NAND FLASH的所有坏块的索引
     *       AT^NANDBBC?
     *       ^NANDBBC: 2
     *       ^NANDBBC: 10
     *       ^NANDBBC: 28
     *       OK
     */
    { AT_CMD_NANDBBC,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryNandBadBlockPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^NANDBBC", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 查询NAND FLASH型号
     * [说明]: AP上的工程菜单使用该命令向MODEM查询NAND FLASH的型号信息，包括如下信息：厂商ID、厂商名称、设备ID、设备规格字符串。
     *         该命令为平台化命令，目前只在AP-Modem形态下有效。
     * [语法]:
     *     [命令]: ^NANDVER?
     *     [结果]: <CR><LF>^NANDVER: <mfuid>,<mfuname>,<deviceid>, <devicespec><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^NANDVER=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mfuid>: 整型值，NAND FLASH的厂商ID。
     *     <mfuname>: 字符串类型，厂商名称。最长16个ASCII字符。
     *     <deviceid>: 整型值，设备ID。
     *     <devicespec>: 字符串类型，设备规格字符串。最长32个ASCII字符。
     * [示例]:
     *     · 查询NAND FLASH的型号信息（以Samsung为例）
     *       AT^NANDVER?
     *       ^NANDVER: 236,"Samsung",218,"NAND 256MiB 3,3V 8-bit"
     *       OK
     */
    { AT_CMD_NANDVER,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryNandDevInfoPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^NANDVER", VOS_NULL_PTR },

#if (FEATURE_LTE == FEATURE_ON)
    /* 生产NV恢复 */
    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 生产NV恢复操作
     * [说明]: 生产NV从备份区恢复。进行NV恢复前，首先需要使用DATALOCK命令进行数据解锁，否则返回ERROR。
     * [语法]:
     *     [命令]: ^INFORRS
     *     [结果]: <CR><LF>OK<CR><LF>
     *             有MT相关错误时：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <err_code>: 错误码
     *             0：表示写数据时，数据保护未解锁, 无法操作保护数据。
     */
    { AT_CMD_INFORRS,
      At_SetInfoRRS, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^INFORRS", VOS_NULL_PTR },
#endif

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 读数据文件命令
     * [说明]: PC侧可以用此命令读取单板的数据文件，命令支持任意文件（包括XML文件、文本文件、Binary文件）。
     * [语法]:
     *     [命令]: ^RSFR=<itemname>,<sub_itemname>
     *     [结果]: <CR><LF>^RSFR: <itemname>,<sub_itemname>,<ref>,<total>,\
     *             <index>,<item><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             有MT相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^RSFR=?
     *     [结果]: <CR><LF>^RSFR: (list of supported <itemname>s)<CR><LF>
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
    { AT_CMD_RSFR,
      AT_SetRsfrPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestRsfrPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^RSFR", (VOS_UINT8 *)"(name),(subname)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 信息备份
     * [说明]: 用于产线进行NV项备份。
     * [语法]:
     *     [命令]: ^NVBACKUP
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [示例]:
     *     · NV备份成功
     *       AT^NVBACKUP
     *       0
     *       OK
     *     · NV备份失败
     *       AT^NVBACKUP
     *       ERROR
     */
    { AT_CMD_NVBACKUP,
      atSetNVBackup, AT_UPGRADE_TIME_50S, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_E5_IS_LOCKED,
      (VOS_UINT8 *)"^NVBACKUP", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 读取PMU_DIE_ID
     * [说明]: 该命令用于读取PMU_DIE_ID。
     *         BalongV7R22C30新增。5G平台有多种类型PMU芯片，该命令可以获取对应类型的PMU_DIE_ID。
     * [语法]:
     *     [命令]: ^PMUDIESN=<value>
     *     [结果]: 执行成功时：
     *             <CR><LF>^PMUDIESN: 0x<DIEID><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况时返回：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *             说明：设置命令只在NR宏打开版本上支持。
     *     [命令]: ^PMUDIESN?
     *     [结果]: 执行成功时：
     *             <CR><LF>^PMUDIESN: 0x<DIEID><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况时返回：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     * [参数]:
     *     <value>: PMU芯片类型枚举值，取值范围0,1,2.
     *             说明：设置命令只在NR宏打开版本上支持。
     *     <DIEID>: 字符串类型，pmudieid码流，16进制输出 20字节160bit
     * [示例]:
     *     · 执行测试命令 底层获取到PMU_DIE_ID的情况
     *       AT^PMUDIESN?
     *       ^PMUDIESN: 0x0000000000000000000000000000000000000000
     *       OK
     */
    { AT_CMD_PMUDIESN,
      AT_SetPmuDieSNPara, AT_SET_PARA_TIME, AT_QryPmuDieSNPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^PMUDIESN", (VOS_UINT8 *)"(0,1,2)" },


    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: GNSS场景通知
     * [说明]: 为避免RF时钟周期打开对GNSS场景的影响，需要Hi110X芯片在GNSS场景下给modem发AT命令，modem侧收到AT命令后，调用RF时钟控制底软接口，通知底软现在处于GNSS场景，若此时RF时钟没有打开，则保持关闭，若上层打开RF时钟，则打开该时钟，若再收到关闭该时钟的指令，则保持打开，并保存当前上层的指令。当Hi110X芯片退出GNSS场景后，会发AT命令，该命令会传递到底软，底软后续时钟的开关只受上层软件控制。在Kirin980及后续平台上使用。
     * [语法]:
     *     [命令]: ^GNSSNTY=<value>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             错误情况时返回：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^GNSSNTY=?
     *     [结果]: <CR><LF>^GNSSNTY: (list of supported <value>s)
     *             <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <value>: 整型值十进制 范围（0-1）：
     *             0：通知底软现在推出GNSS场景，调用mdrv_pmic_rfclk_release()；
     *             1：通知底软现在处于GNSS场景，调用mdrv_pmic_rfclk_vote()。
     * [示例]:
     *     · 执行命令
     *       AT^GNSSNTY=0
     *       OK
     */
    { AT_CMD_GNSSNTY,
      At_SetGnssNtyPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^GNSSNTY", (VOS_UINT8 *)"(0,1)" },

    /* 闪电卡版本自动化测试控制应用手动拨号 */
    { AT_CMD_APPDIALMODE,
      At_SetAppDialModePara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^APPDIALMODE", (VOS_UINT8 *)"(0-1)" },
};

/* 注册DRV装备AT命令表 */
VOS_UINT32 AT_RegisterDeviceDrvCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atDeviceDrvCmdTbl, sizeof(g_atDeviceDrvCmdTbl) / sizeof(g_atDeviceDrvCmdTbl[0]));
}

