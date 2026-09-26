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
#include "at_phone_as_cmd_tbl.h"
#include "AtParse.h"
#include "at_parse_cmd.h"
#include "at_phone_as_set_cmd_proc.h"
#include "at_phone_as_qry_cmd_proc.h"
#include "at_test_para_cmd.h"
#include "at_device_cmd.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_PHONE_AS_CMD_TBL_C

static const AT_ParCmdElement g_atPhoneAsCmdTbl[] = {
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 查询RSCP和ECIO
     * [说明]: 对信号强度RSCP/ECIO的查询。RSCP/ECIO信号值的获取依赖于RSSI信号的变化。
     *         RSCP/ECIO/RSSI存在如下的对应关系：RSCP=ECIO+RSSI；其中：RSCP/ECIO/RSSI都必须是实际的绝对值，不能是相对值。
     *         ECIO是指导频的码片平均能量和带内总功率密度比。
     * [语法]:
     *     [命令]: ^CSNR
     *     [结果]: <CR><LF>OK<CR><LF>
     *     [命令]: ^CSNR?
     *     [结果]: <CR><LF>^CSNR: <rscp>,<ecio><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^CSNR=?
     *     [结果]: <CR><LF>^CSNR: (list of supported <rscp>s), (list of supported <ecio>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <rscp>: 整型值，RSCP值，单位为dBm，取值为-145～-60。
     *     <ecio>: 整型值，ECIO值，单位为dBm，取值为-32～0。
     * [示例]:
     *     · 查询当前信号强度
     *       AT^CSNR?
     *       ^CSNR: -109,-9
     *       OK
     */
    { AT_CMD_CSNR,
      At_SetCSNR, AT_NOT_SET_TIME, At_QryCSNR, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CSNR", (VOS_UINT8 *)"(-145 - -60),(-32 - 0)" },

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    /* WAS应终端要求新增AT命令 */
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: W模锁频
     * [说明]: 查询，设置/取消W模锁频功能。
     *         注：此命令由FEATURE_PHONE_ENG_AT_CMD宏控制。
     * [语法]:
     *     [命令]: ^WFREQLOCK=<enable>[,<freq>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^WFREQLOCK?
     *     [结果]: <CR><LF>^FREQLOCK: <enable>[,<freq>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^WFREQLOCK=?
     *     [结果]: <CR><LF>^FREQLOCK: (list of supported <enable>) (list of supported <freq>)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <enable>: 整型值，W模锁频功能是否启动。
     *             0：锁频功能没有启用；
     *             1：启用了锁频功能。
     *     <freq>: 整数值，当前锁定的频率值，取值（请根据协议、产品规格确定有效范围）。
     *             <enable>=0时，<freq>可以不填。
     * [示例]:
     *     · 锁定频点
     *       AT^WFREQLOCK=1,10600
     *       OK
     */
    { AT_CMD_FREQLOCK,
      At_SetFreqLock, AT_SET_PARA_TIME, At_QryFreqLock, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^WFREQLOCK", (VOS_UINT8 *)"(0,1),(412-10838)" },
#endif

#if (FEATURE_PROBE_FREQLOCK == FEATURE_ON)
    /* 模块产品线GUTL锁频特性 */
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 锁频
     * [说明]: 路测使用，查询、设置、取消锁频功能。
     *         备注：同一时刻同一制式下仅支持一个频点的锁频，所以路测进行锁频操作时，如果同一制式下已经有一个频点是锁频状态，在锁下一个频点之前，需要先关闭前一个频点的锁频状态。
     *         AT^ M2MFREQLOCK命令在FEATURE_PROBE_FREQLOCK宏打开时使用，并且不能和其他锁频命令混用。
     * [语法]:
     *     [命令]: ^M2MFREQLOCK=<enable>[,<mode>,<freq>,[<band>],[<psc>],[<pci>]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^M2MFREQLOCK?
     *     [结果]: <CR><LF>^M2MFREQLOCK: <enable>,<mode>[,<freq>,[<band>],[<psc>],[<pci>]]
     *             <CR><LF>^M2MFREQLOCK: <enable>,<mode>[,<freq>,[<band>],[<psc>],[<pci>]]
     *             <CR><LF>^M2MFREQLOCK: <enable>,<mode>[,<freq>,[<band>],[<psc>],[<pci>]]
     *             <CR><LF>^M2MFREQLOCK: <enable>,<mode>[,<freq>,[<band>],[<psc>],[<pci>]]
     *             <CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <enable>: 整型值，标识锁频功能是否启用。
     *             0：锁频功能没有启用；
     *             1：启用了锁频功能。
     *     <mode>: 字符类型，锁频的制式。
     *             “01”          GSM
     *             “02”          WCDMA
     *             “03”          TD-SCDMA
     *             “04”          LTE
     *     <freq>: 整数值，当前锁定的频率值。
     *     <band>: 字符类型，锁频的频段，用来区分频点重复的频段。只在GSM制式下设置。
     *             “00”          850
     *             “01”          900
     *             “02”          1800
     *             “03”          1900
     *     <psc>: 字符类型，主扰码参数，WCDMA制式下该参数有效，该项为可选项，未配置该参数时，为WCDMA制式下锁频点；配置该参数后，为WCDMA制式下锁频点+扰码组合。
     *     <pci>: 物理小区ID，整形值，（请根据协议、产品规格确定取值范围），只在LTE制式下有效，用来区分不同LTE小区的信号。
     * [示例]:
     *     · 锁定39250频点
     *       AT^M2MFREQLOCK=1,"04",39250,,,
     *       OK
     *     · 锁定39250频点小区333
     *       AT^M2MFREQLOCK=1,"04",39250,,,333
     *       OK
     *     · 查询当前的频点锁定信息
     *       AT^M2MFREQLOCK?
     *       ^M2MFREQLOCK: 0,"01"
     *       ^M2MFREQLOCK: 0,"02"
     *       ^M2MFREQLOCK: 0,"03"
     *       ^M2MFREQLOCK: 1,"04",39250,,,333
     *       OK
     *     · 解除当前的锁定信息
     *       AT^M2MFREQLOCK=0
     *       OK
     */
    { AT_CMD_M2MFREQLOCK,
      At_SetM2MFreqLock, AT_SET_PARA_TIME, At_QryM2MFreqLock, AT_QRY_PARA_TIME, At_TestM2MFreqLock, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^M2MFREQLOCK", (VOS_UINT8 *)"(0,1),(Mode),(0-4294967295),(Band),(0-65535),(0-503)" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 锁频
     * [说明]: 查询、设置、取消GUTL锁频功能。
     *         备注：同一时刻同一制式下仅支持一个频点的锁频，所以进行锁频操作时，如果同一制式下已经有一个频点是锁频状态，在锁下一个频点之前，需要先关闭前一个频点的锁频状态。
     *         AT^FREQLOCK命令在FEATURE_PROBE_FREQLOCK宏打开时使用，并且不能和其他锁频命令混用。
     * [语法]:
     *     [命令]: ^FREQLOCK=<enable>[,<mode>,<freq>,[<band>],[<psc>],[<pci>]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^FREQLOCK?
     *     [结果]: <CR><LF>^FREQLOCK: <enable>,<mode>[,<freq>,[<band>],[<psc>],[<pci>]]
     *             <CR><LF>^FREQLOCK: <enable>,<mode>[,<freq>,[<band>],[<psc>],[<pci>]]
     *             <CR><LF>^FREQLOCK: <enable>,<mode>[,<freq>,[<band>],[<psc>],[<pci>]]
     *             <CR><LF>^FREQLOCK: <enable>,<mode>[,<freq>,[<band>],[<psc>],[<pci>]]
     *             <CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <enable>: 整型值，标识锁频功能是否启用。
     *             0：锁频功能没有启用；
     *             1：启用了锁频功能。
     *     <mode>: 字符类型，锁频的制式。
     *             “01”          GSM
     *             “02”          WCDMA
     *             “03”            TD-SCDMA
     *             “04”          LTE
     *     <freq>: 整数值，当前锁定的频率值。
     *     <band>: 字符类型，锁频的频段，用来区分频点重复的频段。只在GSM制式下设置。
     *             “00”          850
     *             “01”          900
     *             “02”          1800
     *             “03”          1900
     *     <psc>: 字符类型，主扰码参数，WCDMA制式下该参数有效，该项为可选项，未配置该参数时，为WCDMA制式下锁频点；配置该参数后，为WCDMA制式下锁频点+扰码组合。
     *     <pci>: 物理小区ID，整形值，（请根据协议、产品规格确定取值范围），只在LTE制式下有效，用来区分不同LTE小区的信号。
     * [示例]:
     *     · 锁定39250频点
     *       AT^FREQLOCK=1,"04",39250,,,
     *       OK
     *     · 锁定39250频点小区333
     *       AT^FREQLOCK=1,"04",39250,,,333
     *       OK
     *     · 查询当前的频点锁定信息
     *       AT^FREQLOCK?
     *       ^FREQLOCK: 0,"01"
     *       ^FREQLOCK: 0,"02"
     *       ^FREQLOCK: 0,"03"
     *       ^FREQLOCK: 1,"04",39250,,,333
     *       OK
     *     · 解除当前的锁定信息
     *       AT^FREQLOCK=0
     *       OK
     */
    { AT_CMD_M2MFREQLOCK,
      At_SetM2MFreqLock, AT_SET_PARA_TIME, At_QryM2MFreqLock, AT_QRY_PARA_TIME, At_TestM2MFreqLock, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^FREQLOCK", (VOS_UINT8 *)"(0,1),(Mode),(0-4294967295),(Band),(0-65535),(0-503)" },
#endif

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: G模锁频
     * [说明]: 设置/取消G模锁频功能。
     *         在如下三种情况下，下发这个AT命令，不能保证锁频成功。
     *         单板没开机；
     *         单板当前不支持G模；
     *         单板不支持当前需要锁定的频点；
     *         注：此命令由FEATURE_PHONE_ENG_AT_CMD宏控制。
     * [语法]:
     *     [命令]: ^GFREQLOCK=<enable>[,<freq>,<band>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^GFREQLOCK?
     *     [结果]: <CR><LF>^GFREQLOCK: <enable>[,<freq>,<band>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             Enable为0时，freq和band不上报
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^GFREQLOCK=?
     *     [结果]: <CR><LF>^GFREQLOCK: (list of supported<enable>)(list of support<freq>)(list of support<band>)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <enable>: 整型值，锁频功能是否启动
     *             0: 不启用锁频功能
     *             1: 启用锁频功能
     *     <freq>: 整数值，当前锁定的频点，取值范围0~1023
     *             当<enable>=0时，<freq>可以不填
     *     <band>: 整数值，当前锁定频率对应的band值:
     *             0: GSM 850频段
     *             1: GSM 900 频段
     *             2: GSM 1800频段
     *             3: GSM 1900频段
     *             当<enable>=0时，<band>可以不填
     * [示例]:
     *     · 锁定900频段的25号频点
     *       AT^GFREQLOCK=1,25,1
     *       OK
     *     · 查询锁频状态
     *       AT^GFREQLOCK?
     *       ^GFREQLCOK: 1,25,1
     *       OK
     */
    { AT_CMD_GSMFREQLOCK,
      AT_SetGFreqLock, AT_SET_PARA_TIME, AT_QryGFreqLock, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^GFREQLOCK", (VOS_UINT8 *)"(0,1),(0-1023),(0-3)" },
#endif

#if (FEATURE_DSDS == FEATURE_ON)
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 设置PS保护模式
     * [说明]: 注意：仅限V8R1使用。
     *         在DSDS特性宏开启时，可用此命令设置PS保护模式。在激活PS保护模式后，物理层会将PS业务类似于CS业务进行处理。
     * [语法]:
     *     [命令]: ^PSPROTECTMODE=<Type>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <Type>: 整数型，PS 保护模式类型，取值0～1。
     *             0：去激活PS保护模式
     *             1：激活PS保护模式
     * [示例]:
     *     · 激活PS保护模式
     *       AT^PSPROTECTMODE=1
     *       OK
     */
    { AT_CMD_PSPROTECTMODE,
      At_SetPsProtectModePara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^PSPROTECTMODE", (VOS_UINT8 *)"(0,1)" },
#endif

#if (FEATURE_LTE == FEATURE_ON)
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 查询运营商标识及小区ID
     * [说明]: 本命令用于查询4G小区信息。
     * [语法]:
     *     [命令]: ^CECELLID?
     *     [结果]: <CR><LF>^CECELLID: < PLMNID>,< CI >,< CELLID><TAC> <CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <PLMNID>: PLMN信息，五个或六个字符，其中前三位是MCC，后两位或三位是MNC。
     *     <CI>: 小区信息，八个字符，16进制表示。
     *     <CELLID>: 4bytes的16进制数的字符串类型，小区信息，类型同<LAC>
     *     <TAC>: 位置码信息，六个字符，16进制表示。
     * [示例]:
     *     · 查询当前4G小区信息
     *       AT^CECELLID?
     *       ^CECELLID: 47031,101120,458,1
     *       OK
     */
    { AT_CMD_CECELLID,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryCecellidPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE,
      (VOS_UINT8 *)"^CECELLID", VOS_NULL_PTR },
#endif

    /*
     * [类别]: 协议AT-测试类私有
     * [含义]: 查询WCDMA主小区邻区测量信息
     * [说明]: 该命令用来查询主小区邻区测量信息。
     *         CL模式下该命令不支持。CPMS
     *         本功能只在WCDMA下有效。
     * [语法]:
     *     [命令]: ^CELLINFO=<n>
     *     [结果]: <CR><LF>cellinfo<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^CELLINFO=?
     *     [结果]: <CR><LF>^CELLINFO: (list of supported <n>s) <CR><LF>
     * [参数]:
     *     <n>: 整型值，取值0～1。
     *             0：查询主小区测量信息。
     *             输出的内容依次为频点，扰码，RSCP，ECN0；当DCH状态，输出的是激活集中小区信息；非DCH状态，输出主小区信息；当获取不到测量值，输出no cellinfo rslt。
     *             1：查询邻区测量信息。
     *             输出的内容依次为频点，扰码，RSCP，ECN0；最多输出8个邻区的信息。
     * [示例]:
     *     · 查询主小区测量信息
     *       AT^CELLINFO=0
     *       10700,100,-65,-14
     *       OK
     */
    { AT_CMD_CELLINFO,
      At_SetCellInfoPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^CELLINFO", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-测试类私有
     * [含义]: 设置WCDMA小区搜索方式
     * [说明]: 该命令用来设置WCDMA小区搜索方式。
     *         本功能只在WCDMA下有效。
     * [语法]:
     *     [命令]: ^CELLSRCH=<n>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况:
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CELLSRCH?
     *     [结果]: <CR><LF>^CELLSRCH: <n><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况:<CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CELLSRCH=?
     *     [结果]: <CR><LF>^CELLSRCH: (list of supported <n> s)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，取值0～1。
     *             0：设置搜网模式为正常模式。
     *             1：设置搜网模式为不使用先验信息模式。
     * [示例]:
     *     · 设置WCDMA小区搜索方式
     *       AT^CELLSRCH=0
     *       OK
     */
    { AT_CMD_CELLSRCH,
      At_SetCellSearch, AT_SET_PARA_TIME, At_QryCellSearch, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE_E5,
      (VOS_UINT8 *)"^CELLSRCH", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: LTE传输模式查询命令
     * [说明]: 该命令用于查询LTE接入技术下的传输模式。
     * [语法]:
     *     [命令]: ^TRANSMODE?
     *     [结果]: <CR><LF>^TRANSMODE: <mode><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <mode>: LTE下的传输模式(Transmision Mode)，整型值。
     *             1：TM1；
     *             2：TM2；
     *             3：TM3；
     *             4：TM4；
     *             5：TM5；
     *             6：TM6；
     *             8：TM8；
     *             9：TM9；
     *             10：TM10。
     * [示例]:
     *     · WCDMA模式下，查询LTE传输模式：
     *       AT^TRANSMODE?
     *       ERROR
     *     · LTE模式下，查询LTE传输模式，当前传输模式为TM2：
     *       AT^TRANSMODE?
     *       ^TRANSMODE: 2
     *       OK
     */
    { AT_CMD_TRANSMODE,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryTransModePara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^TRANSMODE", VOS_NULL_PTR },  // CMD_TBL_PIN_IS_LOCKED?CMD_TBL_NO_LIMITED?
};

/* 注册phone as AT命令表 */
VOS_UINT32 AT_RegisterPhoneAsCmdTbl(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atPhoneAsCmdTbl, sizeof(g_atPhoneAsCmdTbl) / sizeof(g_atPhoneAsCmdTbl[0]));
}
