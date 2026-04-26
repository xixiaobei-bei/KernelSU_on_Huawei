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
#include "at_phone_mm_cmd_tbl.h"
#include "AtParse.h"
#include "at_parse_cmd.h"
#include "at_phone_mm_set_cmd_proc.h"
#include "at_phone_mm_qry_cmd_proc.h"
#include "at_test_para_cmd.h"
#include "at_device_cmd.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_PHONE_MM_CMD_TBL_C

static const AT_ParCmdElement g_atPhoneMmCmdTbl[] = {
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 查询系统信息
     * [说明]: 查询当前系统信息。如系统服务状态、domain、漫游状态、系统模式和SIM卡状态等。
     *         注：此命令由FEATURE_PHONE_ENG_AT_CMD宏控制。
     *         L模下该命令不支持。
     * [语法]:
     *     [命令]: ^SYSINFO
     *     [结果]: <CR><LF>^SYSINFO: <srv_status>,<srv_domain>, <roam_status>, <sys_mode>, <sim_state> [,<lock_state>, <sys_submode>]<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <srv_status>: 整数型，系统服务状态，取值0～4。
     *             0：无服务；
     *             1：有限制服务；
     *             2：服务有效；
     *             3：有限制的区域服务；
     *             4：省电和深睡状态。
     *             注意：
     *             在NV_SYSTEM_APP_CONFIG(121)配置当前上层应用为android时：
     *             1）如果当前处在深睡状态，且不能提供任何服务，则查询的服务状态为4；
     *             2）如果当前不处在深睡状态，且不能提供任何服务，则查询的服务状态为0；
     *             3）如果当前仅能提供限制服务，则查询的服务状态为1。
     *             在NV_SYSTEM_APP_CONFIG(121)配置当前上层应用不为android时：
     *             1）如果当前处在深睡状态，如果当前无服务或仅能提供紧急服务，则查询的服务状态为4。
     *     <srv_domain>: 整数型，服务域，取值0～4，255。
     *             0：无服务；
     *             1：仅CS服务；
     *             2：仅PS服务；
     *             3：PS+CS服务；
     *             4：CS、PS均未注册，并处于搜索状态；
     *             注意：<srv_status>取值为0（无服务）时，本参数取值不做规定，没有实际含义，在合法范围内即可。
     *     <roam_status>: 漫游状态。
     *             2：未知
     *             注意：
     *             1)成功注册到HPLMN上时，查询结果中的漫游状态显示为0；
     *             2)成功注册到漫游网络上时，查询结果中的漫游状态显示为1；
     *             3)其他情况下，查询结果中的漫游状态显示为2。
     *     <sys_mode>: 整数型，系统模式，取值0～8,15。
     *             0：无服务；
     *             1：保留；
     *             2：保留；
     *             3：GSM/GPRS模式；
     *             4：保留；
     *             5：WCDMA模式；
     *             6：GPS模式（不支持）；
     *             7：GSM/WCDMA；
     *             15：TD_SCDMA模式。
     *             注意：<srv_status>取值为0（无服务）时，本参数取值不做规定，没有实际含义，在合法范围内即可。
     *     <sim_state>: 整数型，卡状态，取值0～4，240，255。
     *             0：USIM卡状态无效；
     *             1：USIM卡状态有效；
     *             2：USIM在CS下无效；
     *             3：USIM在PS下无效；
     *             4：USIM在PS+CS下均无效；
     *             240：ROMSIM；
     *             255：USIM卡不存在。
     *     <lock_state>: 整数型，锁状态，取值0～1。
     *             0：SIM卡未被CardLock功能锁定；
     *             1：SIM卡被CardLock功能锁定。
     *             注意：此参数仅对E5形态和闪电卡有效。
     *     <sys_submode>: 整数型，系统子模式，取值0～18。
     *             0：无服务；
     *             1：GSM模式；
     *             2：GPRS模式；
     *             3：EDGE模式；
     *             4：WCDMA模式；
     *             5：HSDPA模式；
     *             6：HSUPA模式；
     *             7：HSDPA and HSUPA模式；（TD要求后台显示为HSPA）
     *             8：TD_SCDMA模式；
     *             9：HSPA+模式；
     *             14：UMB；
     *             16：3xRTT；
     *             17：HSPA+(64QAM)模式；
     *             18：HSPA+(MIMO)模式。
     *             注意：<srv_status>取值为0（无服务）时，本参数取值不做规定，没有实际含义，在合法范围内即可。
     * [示例]:
     *     · 系统信息查询
     *       AT^SYSINFO
     *       ^SYSINFO: 0,0,2,0,0,,0
     *       OK
     */
    { AT_CMD_SYSINFO,
      At_SetSystemInfo, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SYSINFO", TAF_NULL_PTR },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 设置系统配置参考
     * [说明]: 设置系统模式、G/W接入次序、频带、漫游支持和domain等特性。
     *         注：此命令由FEATURE_PHONE_ENG_AT_CMD宏控制。
     *         L模不支持该命令。
     * [语法]:
     *     [命令]: ^SYSCFG=<mode>,<acqorder>,<band>,<roam>,<srvdomain>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^SYSCFG?
     *     [结果]: <CR><LF>^SYSCFG: <mode>,<acqorder>,<band>,<roam>,<srvdomain><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^SYSCFG=?
     *     [结果]: <CR><LF>^SYSCFG: (list of supported <mode>s),
     *             (list of supported <acqorder>s),
     *             (list of supported( <band>)s),
     *             (list of supported <roam>s),
     *             (list of supported <srvdomain>s) <CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，接入模式。
     *             2：自动选择；
     *             3：CDMA模式；
     *             4：HDR模式；
     *             8：CDMA/HDR HYBRID 模式；
     *             13：GSM ONLY；
     *             14：WCDMA ONLY；
     *             16：不改变。
     *     <acqorder>: 整型值，接入优先级，取值0～3。
     *             0：自动；
     *             1：先GSM后WCDMA；
     *             2：先WCDMA后GSM；
     *             3：不改变。
     *             注意：<mode>为单模时，<acqorder>设置的值无效。即<mode>为13时，MS置<acqorder>为1；<mode>为14时，MS置<acqorder>为2。
     *             对于CDMA/HDR模式来说，没有接入优先级概念。
     *     <band>: 整数型，频带选择，参数为16进制串，取值为下列各参数或者是0x3FFF_FFFF和0x4000_0000以外的各参数叠加值。查询结果中该参数按照UE实际支持的频段信息填充。
     *             0x80：GSM DCS systems；
     *             0x100：Extended GSM 900；
     *             0x200：Primary GSM 900；
     *             0x8_0000：GSM850；
     *             0x10_0000：GSM Railway 900；
     *             0x20_0000：GSM PCS 1900；
     *             0x1_0000：WCDMA IMT-E 2600（BAND-VII）;
     *             0x40_0000：WCDMA IMT 2000（BAND-I）；
     *             0x80_0000：WCDMA US PCS 1900（BAND-II）；
     *             0x100_0000：WCDMA 1800（BAND-III）；
     *             0x200_0000：WCDMA US 1700（BAND-IV）；
     *             0x400_0000：WCDMA US 850（BAND-V）；
     *             0x800_0000：WCDMA 800（BAND-VI）;
     *             0x2_0000_0000_0000：WCDMA US 900（BAND-VIII）；
     *             0x4_0000_0000_0000：WCDMA US 1700（BAND-IX）；
     *             0x1000_0000_0000_0000：WCDMA 850（BAND-XIX）;
     *             0x2000_0000_0000_0000：WCDMA 1500（BAND-XI）;
     *             0x4000_0000：不改变；
     *             0x3FFF_FFFF：任何频带（默认支持的频带0x4F8_0380）。
     *     <roam>: 漫游特性未激活时：
     *             整型值，漫游支持，取值0～2。
     *             0：MBB CUST宏打开时，支持关闭漫游，允许设置为0；否则，不支持；
     *             1：可以漫游；
     *             2：不改变。
     *             漫游特性激活时：
     *             整型值，漫游支持，取值0～3。
     *             0：开启国内国际漫游；
     *             1：开启国内漫游，关闭国际漫游；
     *             2：关闭国内漫游，开启国际漫游；
     *             3：关闭国内国际漫游。
     *             注意：漫游特性通过NV项控制是否激活。漫游对于CDMA/HDR无效，忽略，但该参数会保存。
     *     <srvdomain>: 整型值，服务域，取值0～4。
     *             0：CS ONLY（开机仅注册CS域，在需要进行PS域数据服务的时候再进行PS附着，并且在结束PS服务的时候进行PS去附着）；
     *             1：PS ONLY（开机仅注册PS域，在需要进行CS域语音服务的时候再进行CS注册，并且在结束CS服务的时候进行CS去注册）；
     *             2：CS_PS；
     *             3：ANY（等同于0）；
     *             4：不改变。
     *             注：对于CDMA/HDR来说，服务域忽略，但改参数会保存。
     * [示例]:
     *     · 设置为双模且WCDMA模式优先，支持WCDMA BAND-I，GSM所有频段
     *       AT^SYSCFG=2,2,780380,2,4
     *       OK
     *     · 设置WCDMA ONLY，频带不改变，CS ONLY
     *       AT^SYSCFG=14,3,40000000,2,0
     *       OK
     *     · 查询系统配置
     *       AT^SYSCFG?
     *       ^SYSCFG: 14,2,780380,1,0
     *       OK
     *     · 测试SYSCFG
     *       AT^SYSCFG=?
     *       ^SYSCFG: (2,13,14,16),(0-3),(-Band-),(0-2),(0-4)
     *       OK
     */
    { AT_CMD_SYSCFG,
      At_SetSysCfgPara, AT_SYSCFG_SET_PARA_TIME, At_QrySysCfgPara, AT_QRY_PARA_TIME, AT_TestSyscfg, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SYSCFG", (VOS_UINT8 *)"(2,3,4,8,13,14,16),(0-3),(@Band),(0-3),(0-4)" },
#endif
#if (FEATURE_LTE == FEATURE_ON)
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 设置扩展系统配置
     * [说明]: 该命令设置系统模式、网络接入次序、频带、漫游支持和domain等特性。
     *         acqorder的设置最多支持6个制式的设置，即全制式的设置。
     *         该命令仅在单板支持专用时才能使用（即当前单板支持LTE能力或支持 NR 能力）。
     * [语法]:
     *     [命令]: ^SYSCFGEX=<acqorder>,<band>,<roam>,<srvdomain>,<lteband>,<reserve1>,<reserve2>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^SYSCFGEX?
     *     [结果]: <CR><LF>^SYSCFGEX: <acqorder>,<band>,<roam>,<srvdomain>,<lteband><CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: ^SYSCFGEX=?
     *     [结果]: ^SYSCFGEX: (list of supported <acqorder>s),(list of supported( <band>,<band_name>)s),(list of supported <roam>s),(list of supported <srvdomain>s),(list of supported( <lteband>,<lteband_name>)s),<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             注：list of supported <acqorder>s由之前的01，02,03,00,09及01,02,03的多种组合改为现在的01,02,03,04,07,08只上报所有单独制式.。
     *             01：GSM；
     *             02：WCDMA；
     *             03：LTE；
     *             08：NR。
     * [参数]:
     *     <acqorder>: 网络接入次序，字符串类型。其取值是”00”，”99”或是如下参数的组合。
     *             “00”：GUL+NR模式；
     *             “01”：GSM；
     *             “02”：WCDMA；
     *             “03”：LTE；
     *             “04”：CDMA 1X；
     *             “07”：CDMA EVDO；
     *             “08”：NR；
     *             “99”：无变化。
     *             注：TDSCDMA与WCDMA当成一个接入技术，当SYSCFGEX设置接入模式包含02时，表示TDSCDMA与WCDMA同时支持；反之都不支持。
     *             例如：“03”表示LTE Only；”030201”表示先LTE后WCDMA，最后GSM (LTE->WCDMA->GSM)；“0302”表示先LTE后WCDMA，不搜GSM。
     *             “99”不和其他值组合，仅表示不修改单板的网络接入次序。
     *             “00”不和其他值组合，仅表示修改单板的网络接入次序为GUL+NR模式，具体网络接入次序由单板确定。
     *     <band>: 频带选择，和模式的选择相关，具体取决于单板性能，其参数为16进制，取值为下列各参数或者是0x3FFFFFFF和0x40000000以外的各参数叠加值。查询结果中该参数按照UE实际支持的频段信息填充。
     *             80（CM_BAND_PREF_GSM_DCS_1800）：GSM DCS systems；
     *             100（CM_BAND_PREF_GSM_EGSM_900）：Extended GSM 900；
     *             200（CM_BAND_PREF_GSM_PGSM_900）：Primary GSM 900；
     *             100000（CM_BAND_PREF_GSM_RGSM_900）：Railways GSM 900;
     *             200000（CM_BAND_PREF_GSM_PCS_1900）：GSM PCS；
     *             10000（CM_BAND_PREF_WCDMA_VII_2600）：WCDMA 2600;
     *             400000（CM_BAND_PREF_WCDMA_I_IMT_2000）：WCDMA IMT 2000；
     *             800000（CM_BAND_PREF_WCDMA_II_PCS_1900）：WCDMA PCS；
     *             1000000（CM_BAND_PREF_WCDMA_III_1800）：WCDMA 1800；
     *             2000000（CM_BAND_PREF_WCDMA_IV_1700）：WCDMA 1700;
     *             4000000（CM_BAND_PREF_WCDMA_V_850）：WCDMA 850;
     *             8000000（CM_BAND_PREF_WCDMA_VI_800）：WCDMA 800;
     *             3FFFFFFF（CM_BAND_PREF_ANY）：任何频带；
     *             40000000（CM_BAND_PREF_NO_CHANGE）：频带不变化；
     *             0004000000000000 (CM_BAND_PREF_WCDMA_IX_1700)：WCDMA_IX_1700；
     *             0000000000400000 (CM_BAND_PREF_WCDMA_IMT)：WCDMA_IMT(2100)；
     *             0002000000000000：WCDMA 900；
     *             1000000000000000(CM_BAND_PREF_WCDMA_XIX_850)：WCDMA_XIX_850；
     *             2000000000000000(CM_BAND_PREF_WCDMA_XI_1500)：WCDMA_XI_1500；
     *             00680380：Automatic；
     *             00080000：GSM 850。
     *     <band_name>: 字符串类型，频带名称。
     *     <roam>: 漫游特性未激活时：
     *             整型值，取值0～2。
     *             0：MBB CUST宏打开时，支持关闭漫游，允许设置为0；否则，不支持；
     *             1：可以漫游；
     *             2：不改变。
     *             漫游特性激活时：
     *             整型值，漫游支持，取值0～3。
     *             0：开启国内国际漫游；
     *             1：开启国内漫游，关闭国际漫游；
     *             2：关闭国内漫游，开启国际漫游；
     *             3：关闭国内国际漫游。
     *             注意：漫游特性通过NV项控制是否激活。
     *     <srvdomain>: 域设置。
     *             0：CS_ONLY；
     *             1：PS_ONLY；
     *             2：CS_PS；
     *             3：ANY；
     *             4：无变化；
     *             5：NULL。
     *             注：
     *             1）服务域设置为NULL，表示CS、PS服务域都不设置；
     *             2）设置的模式里含有L或NR，服务域不允许设置为0或3。
     *     <lteband>: LTE频带选择，其参数为16进制，取值为下列各参数或者是0x7FFFFFFFFFFFFFFF以外的各参数叠加值。查询结果中该参数按照UE实际支持的频段信息填充。
     *             7FFFFFFFFFFFFFFF（CM_BAND_PREF_ANY）：任何频带；
     *             1 (CM_BAND_PREF_LTE_EUTRAN_BAND1)：LTE BC1；
     *             40 (CM_BAND_PREF_LTE_EUTRAN_BAND7)：LTE BC7；
     *             1000 (CM_BAND_PREF_LTE_EUTRAN_BAND13)：LTE BC13；
     *             10000 (CM_BAND_PREF_LTE_EUTRAN_BAND17)：LTE BC17；
     *             80 0000 0000 (CM_BAND_PREF_LTE_EUTRAN_BAND40)：LTE BC40。
     *             40000000（CM_BAND_PREF_NO_CHANGE）：频带不变化
     *             缺省：同设置40000000一样，频带不变化
     *             FF…FF（共64个）：同0x7FFFFFFFFFFFFFFF一样，表示任意频段设置
     *     <lteband_name>: 字符串类型，频带名称。
     *     <reserve1>: 保留字段1。
     *     <reserve2>: 保留字段2。
     * [示例]:
     *     · 设置系统配置
     *       AT^SYSCFGEX="00",3FFFFFFF,1,2,7FFFFFFFFFFFFFFF,0,0
     *       OK
     *       如上设置，则RAT接入顺序为NR->LTE->WCDMA->GSM；其中是否携带NR/LTE受NR/LTE宏控制。
     *     · 查询系统配置
     *       AT^SYSCFGEX?
     *       ^SYSCFGEX: "00",4C00000,0,2,8000011041
     *       OK
     *     · 测试系统配置
     *       AT^SYSCFGEX=?
     *        ^SYSCFGEX: "01","02","03","04,"07","08",(-band-),(0-2),(0-4),((7FFFFFFFFFFFFFFF, "All bands"))
     *        OK
     *     · CS业务存在时，设置系统配置
     *       AT^SYSCFGEX="00",3FFFFFFF,1,2,7FFFFFFFFFFFFFFF,0,0
     *       OK
     */
    { AT_CMD_SYSCFGEX,
      AT_SetSysCfgExPara, AT_SYSCFG_SET_PARA_TIME, AT_QrySysCfgExPara, AT_QRY_PARA_TIME, AT_TestSyscfgEx, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SYSCFGEX", (VOS_UINT8 *)"(Acqorder),(@Band),(0-3),(0-5),(@Lteband)" },
#endif

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 查询扩展系统信息
     * [说明]: 此命令查询当前系统信息。如系统服务状态、domain、是否漫游，系统模式，SIM卡状态等.
     * [语法]:
     *     [命令]: ^SYSINFOEX
     *     [结果]: <CR><LF>^SYSINFOEX: <srv_status>,<srv_domain>,<roam_status>,<sim_state>,<lock_state>,<sysmode>,<sysmode_name><submode>,<submode_name><CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <srv_status>: 系统服务状态。
     *             0：无服务；
     *             1：有限制服务；
     *             2：服务有效；
     *             3：有限制的区域服务；
     *             4：省电和深睡状态。
     *             注意：
     *             在NV_SYSTEM_APP_CONFIG(NVID=121)配置当前上层应用为android时：
     *             1）如果当前处在深睡状态，且不能提供任何服务，则查询的服务状态为4；
     *             2）如果当前不处在深睡状态，且不能提供任何服务，则查询的服务状态为0；
     *             3）如果当前仅能提供限制服务，则查询的服务状态为1。
     *             在NV_SYSTEM_APP_CONFIG(NVID=121)配置当前上层应用不为android时：
     *             1）如果当前处在深睡状态、当前无服务或仅能提供紧急服务，则查询的服务状态为4。
     *     <srv_domain>: 系统服务域。
     *             0：无服务；
     *             1：仅CS服务；
     *             2：仅PS服务；
     *             3：PS+CS服务；
     *             4：CS、PS均未注册，并处于搜索状态；
     *     <roam_status>: 漫游状态。
     *             0：非漫游状态；
     *             1：漫游状态；
     *             2：未知。
     *             注意：
     *             1)成功注册到HPLMN上时，查询结果中的漫游状态显示为0；
     *             2)成功注册到漫游网络上时，查询结果中的漫游状态显示为1；
     *             3)其他情况下，查询结果中的漫游状态显示为2。
     *     <sim_state>: SIM卡状态。
     *             0：USIM卡状态无效；
     *             1：USIM卡状态有效；
     *             2：USIM在CS下无效；
     *             3：USIM在PS下无效；
     *             4：USIM在PS+CS下均无效；
     *             240：ROMSIM版本；
     *             255：USIM卡不存在。
     *     <lock_state>: SIM卡的LOCK状态。
     *             0：SIM卡未被CardLock功能锁定；
     *             1：SIM卡被CardLock功能锁定。
     *             注意：此参数仅对E5形态和闪电卡有效。
     *     <sysmode>: 系统制式。
     *             0：NO SERVICE；
     *             1：GSM；
     *             3：WCDMA；
     *             4：TD-SCDMA；
     *             5：WIMAX；
     *             6：LTE；
     *             9：SVLTE/SRLTE；
     *             10：EUTRAN-5GC；
     *             11：NR-5GC；
     *             注意：当该参数的返回值不在本规范的合法范围（0~12），后台默认按照3（WCDMA）的流程来处理。
     *     <sysmode_name>: 系统制式对应的字符串表示形式。
     *             该参数以字符串的形式返回当前系统模式名称，其取值对应该命令参数<sysmode>取值所对应的字符串，例如<sysmode> = 3, <sysmode_name>="WCDMA"。
     *     <submode>: 系统子模式，未列出的值表示未使用。
     *             0：NO SERVICE；
     *             1：GSM；
     *             2：GPRS；
     *             3：EDGE；
     *             41：WCDMA；
     *             42：HSDPA；
     *             43：HSUPA；
     *             44：HSPA；
     *             45：HSPA+；
     *             46：DC-HSPA+；
     *             61：TD-SCDMA；
     *             62：HSDPA；
     *             63：HSUPA；
     *             64：HSPA；
     *             65：HSPA+；
     *             81：802.16e；
     *             101：LTE；
     *             110：EUTRAN-5GC；
     *             111：NR-5GC；
     *             当子模式下的返回参数值非法，后台默认按照主模式的返回值进行显示处理。
     *     <submode _name>: 系统子模式对应的字符串表示形式。
     *             该参数以字符串的形式返回当前网络子模式名称，其取值对应该命令第二个参数<submode>取值所对应的字符串，例如<submode> = 45, <submode _name> = "HSPA+"。
     * [示例]:
     *     · UE当前工作在WCDMA模式下的DC-HSPA+网络下。
     *       AT^SYSINFOEX
     *       ^SYSINFOEX: 2,3,1,1,1,3,"WCDMA",46,"DC-HSPA+"
     *       OK
     */
    { AT_CMD_SYSINFOEX,
      At_SetSystemInfoEx, AT_QRY_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SYSINFOEX", TAF_NULL_PTR },

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 设置MT接入模式
     * [说明]: 设置MT接入模式。
     *         注：此命令由FEATURE_PHONE_ENG_AT_CMD宏控制。
     *         L模不支持该命令。
     * [语法]:
     *     [命令]: ^CPAM=[<act>[,<pri>]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CPAM?
     *     [结果]: <CR><LF>^CPAM: <act>,<pri><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^CPAM=?
     *     [结果]: <CR><LF>^CPAM: (list of supported <act>s), (list of supported <pri>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <act>: 整型值，接入模式，取值0～3，默认值为3。
     *             0：WCDMA单模；
     *             1：WCDMA/GSM双模；
     *             2：GSM单模；
     *             3：不改变。
     *     <pri>: 整型值，接入优先级，取值0～2，默认值为2。
     *             0：GSM优先；
     *             1：WCDMA优先；
     *             2：不改变。
     * [示例]:
     *     · 设置为双模且WCDMA模式优先
     *       AT^CPAM=1,1
     *       OK
     *     · 设置为WCDMA单模
     *       AT^CPAM=0,1
     *       OK
     *     · 设置为GSM单模
     *       AT^CPAM=2,0
     *       OK
     *     · 查询接入模式
     *       AT^CPAM?
     *       ^CPAM: 1,1
     *       OK
     *     · 测试CPAM
     *       AT^CPAM=?
     *       ^CPAM: (0-3),(0-2)
     *       OK
     */
    { AT_CMD_CPAM,
      At_SetCpamPara, AT_SET_PARA_TIME, At_QryCpamPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CPAM", (VOS_UINT8 *)"(0-3),(0-2)" },
#endif
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: CS、PS域附着
     * [说明]: CS、PS域附着相关操作。
     *         注：只支持state为分离状态(即0)，mode为双域（即3）时，可以设置第三个reason参数，reason=1为防嗅探短信攻击触发。
     *         在L下使用此AT命令时，LMM启动内部保护定时器（目前LMM定义该定时器时长为15s），定时器超时后立即本地去注册；协议描述为：启动15s的T3421定时器，前四次定时器超时后，都可以重新发起去注册流程，同时重新启动T3421定时器，第5次定时器超时后，本地去注册。此处实现与协议不符。
     * [语法]:
     *     [命令]: ^CGCATT=<state>,<mode>[,<reason>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CGCATT?
     *     [结果]: <CR><LF>^CGCATT: <PS state>,<CS state><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^CGCATT=?
     *     [结果]: <CR><LF>^CGCATT: (list of supported <state>s), (list of supported <mode>s), (list of supported <reason>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <state>: 整型值，CS、PS域服务状态。
     *             0：分离状态；
     *             1：附着状态。
     *     <mode>: 整型值，操作模式。
     *             1：修改PS域的附着状态；
     *             2：修改CS域的附着状态；
     *             3：同时修改CS、PS域的附着状态。
     *     <PS state>: PS域服务状态，类型同<state>。
     *     <CS state>: CS域服务状态，类型同<state>。
     *     <reason>: 整型值，触发原因值。
     *             0：无原因；
     *             1：防嗅探短信攻击触发；
     * [示例]:
     *     · 发起CS、PS域去附着，原因为防嗅探短信攻击触发
     *       AT^CGCATT=0,3,1
     *       OK
     */
    { AT_CMD_CGCATT,
      At_SetCgcattPara, AT_CGATT_SET_PARA_TIME, At_QryCgcattPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^CGCATT", (VOS_UINT8 *)"(0,1),(1-3),(0,1)" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 设置服务状态变化
     * [说明]: 设置服务状态发生变化时，MT是否主动上报新的服务状态给TE。
     * [语法]:
     *     [命令]: ^SRVST=[<n>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^SRVST=?
     *     [结果]: <CR><LF>^SRVST: (list of supported <n>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整数型，设置是否主动上报服务状态，取值0～1。
     *             0：禁止主动上报服务状态(默认值)；
     *             1：启动主动上报服务状态。
     * [示例]:
     *     · 设置主动上报服务状态
     *       AT^SRVST=1
     *       OK
     */
    { AT_CMD_SRVST,
      At_SetSrvstPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SRVST", (TAF_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 禁止PLMN列表操作
     * [说明]: 禁止PLMN列表的查询、添加和删除操作。
     *         使用该命令将PLMN添加到禁止列表或从禁止列表删除时，必须软关机或上下电才能对EPLMN生效。
     * [语法]:
     *     [命令]: ^CFPLMN=<mode>[,<oper>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *             注：
     *             OK表示禁止列表修改成功；
     *             +CME ERROR: <err>表示禁止列表无需修改
     *     [命令]: ^CFPLMN?
     *     [结果]: <CR><LF>^CFPLMN: <num>[,( numeric <oper>)s] <CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^CFPLMN=?
     *     [结果]: <CR><LF>^CFPLMN: (list of supported <mode>s), (list of supported <oper>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，操作模式，取值0～1。
     *             0：删除禁止PLMN，不指定<oper>时表示删除禁止PLMN列表；
     *             1：添加禁止PLMN。
     *     <oper>: 字符串类型，运营商ID。
     *     <num>: 整型值，当前禁止PLMN的个数。
     * [示例]:
     *     · 删除非空禁止PLMN列表
     *       AT^CFPLMN=0
     *       OK
     *     · 添加PLMN："12345"，"12346"到禁止PLMN列表中
     *       AT^CFPLMN=1,"12345"
     *       OK
     *       AT^CFPLMN=1,"12346"
     *       OK
     *     · 从禁止PLMN列表中删除PLMN ID为"12345"的PLMN
     *       AT^CFPLMN=0,"12345"
     *       OK
     *     · 查询当前禁止PLMN列表
     *       AT^CFPLMN?
     *       ^CFPLMN: 1,"12346"
     *       OK
     *     · 测试^CFPLMN
     *       AT^CFPLMN=?
     *       ^CFPLMN: (0,1),(oper)
     *       OK
     */
    { AT_CMD_CFPLMN,
      At_SetFplmnPara, AT_SET_PARA_TIME, At_QryFPlmnPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^CFPLMN", (VOS_UINT8 *)"(0,1),(oper)" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 设置系统模式变化
     * [说明]: 设置当系统模式变化时，MT是否主动上报此指示给TE。
     * [语法]:
     *     [命令]: ^MODE=[<n>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^MODE=?
     *     [结果]: <CR><LF>^MODE: (list of supported <n>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整数型，设置是否主动上报系统模式，取值0～1。默认值为0。
     *             0：禁止主动上报系统模式；
     *             1：启动主动上报系统模式。
     * [示例]:
     *     · 设置主动上报系统模式
     *       AT^MODE=1
     *       OK
     */
    { AT_CMD_SYSMODE,
      At_SetModePara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^MODE", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 设置是否快速开机
     * [说明]: 设置是否为快速开机模式。
     * [语法]:
     *     [命令]: ^CQST=<value>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况:
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^CQST?
     *     [结果]: <CR><LF>^CQST: <value><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^CQST=?
     *     [结果]: <CR><LF>^CQST: (list of supported <value>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <value>: 0：非快速开机模式；
     *             1：快速开机模式。
     * [示例]:
     *     · 设置快速开机
     *       AT^CQST=1
     *       OK
     *     · 查询
     *       AT^CQST?
     *       ^CQST: 1
     *       OK
     */
    { AT_CMD_CQST,
      At_SetQuickStart, AT_SET_PARA_TIME, At_QryQuickStart, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CQST", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 驻留网络变化
     * [说明]: 查询命令：用户查询PLMN信息。
     *         主动上报：驻留网络变化后主动上报PLMN。
     * [语法]:
     *     [命令]: ^PLMN?
     *     [结果]: <CR><LF>^PLMN: [<mcc>],[<mnc>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: -
     *     [结果]: <CR><LF>^PLMN: <mcc>,<mnc><CR><LF>
     * [参数]:
     *     <mcc>: 国家码。
     *     <mnc>: 网络码。
     * [示例]:
     *     · PLMN查询
     *       AT^PLMN?
     *       ^PLMN: 460,01
     *       OK
     *       主动上报
     *       ^PLMN: 460,01
     */
    { AT_CMD_PLMN,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryPlmnPara, AT_QRY_PARA_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^PLMN", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 小区接入禁止信息上报
     * [说明]: 该命令用于主动上报小区接入禁止信息以及手动查询最近一次的小区接入禁止信息：<srv_domain> ,<cell_ac>,<reg_restrict>,<paging_restrict>。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^ACINFO: <srv_domain>,<cell_ac>,<reg_restrict>,<paging_restrict><CR><LF>
     *     [命令]: ^ACINFO?
     *     [结果]: <CR><LF>^ACINFO: <srv_domain>,<cell_ac>,<reg_restrict>,<paging_restrict>
     *             […]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <srv_domain>: 0：CS域；
     *             1：PS域。
     *     <cell_ac>: 0：无受限；
     *             1：限制正常业务受限；
     *             2：限制紧急业务受限；
     *             3：限制正常业务和紧急业务。
     *     <reg_restrict>: 0：不限制srv_domain指定服务域的注册；
     *             1：限制srv_domain指定服务域的注册。
     *     <paging_restrict>: 0：不限制srv_domain指定服务域的寻呼；
     *             1：限制srv_domain指定服务域的寻呼。
     * [示例]:
     *       CS紧急业务受限PS寻呼受限时，ACINFO上报如下
     *       ^ACINFO: 0,2,0,0
     *       ^ACINFO: 1,0,0,1
     *     · 查询最近一次小区的接入禁止信息情况
     *       AT^ACINFO?
     *       ^ACINFO: 0,3,1,1
     *       ^ACINFO: 1,0,0,0
     *       OK
     *     · LTE下查询ACINFO
     *       AT^ACINFO?
     *       ERROR
     */
    { AT_CMD_ACINFO,
      VOS_NULL_PTR, AT_SET_PARA_TIME, AT_QryAcInfoPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^ACINFO", VOS_NULL_PTR },


    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 查询MM INFO中运营商名称
     * [说明]: 该命令用于查询MM INFO中的运营商名称。当MM INFO无效时，返回空。
     * [语法]:
     *     [命令]: ^MMPLMNINFO?
     *     [结果]: <CR><LF>^MMPLMNINFO: <long_name>,<short_name>
     *             <CR><LF><CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <long_name>: 当前运营商的长名称，以UCS-2格式编码。最大长度40字节（80个ASCII字符）。
     *     <short_name>: 当前运营商的短名称，以UCS-2格式编码。最大长度36字节（72个ASCII字符）。
     * [示例]:
     *       查询MM INFO中的运营商名称
     *     · 运营商长名称为L700，短名称为S700，对应的UCS2编码分别为004C003700300030，0053003700300030。
     *       AT^MMPLMNINFO?
     *       ^MMPLMNINFO: 004C003700300030,0053003700300030
     *       OK
     */
    { AT_CMD_MMPLMNINFO,
      VOS_NULL_PTR, AT_SET_PARA_TIME, At_QryMmPlmnInfoPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^MMPLMNINFO", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 注册或业务请求或网络DETACH过程网络拒绝原因值主动上报
     * [说明]: 该命令用于AP-Modem 形态下Modem 主动上报注册或业务请求或网络DETACH过程网络下发的拒绝原因值或USIM 鉴权失败的原因值，在Modem 注册或业务请求或网络DETACH过程中，网络下发的拒绝原因值或USIM 鉴权失败的原因值通过该AT 命令进行上报。
     *         只上报协议中明确规定的拒绝原因值、Balong内部拓展的鉴权失败（#256 Authentication Failure）、联合注册中CS失败的Other Cause（#258，除#2、#16、#17、#22的其他原因值）及CS PS注册网络无响应（#301）及CS PS注册建链异常（#302、#303）。
     *         USIM鉴权失败的原因值也通过该命令上报，USIM的拒绝原因值从65537开始，包括(#65537、65538、65539、65540、65541)。
     * [语法]:
     *     [命令]: ^REJINFO?
     *     [结果]: [<CR><LF>^REJINFO: <PLMN ID>, <Service Domain>,<Reject Cause>,<Rat Type>,<Reject Type>,<Original Reject Cause >,<Lac>,<Rac>, <Cell Id><CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: 注册过程中收到网络拒绝原因值时
     *     [结果]: ^REJINFO: <PLMN ID>,<Service Domain>,<Reject Cause>,<Rat Type>,<Reject Type>,<Original Reject Cause >,<Lac>,<Rac>, <Cell Id>
     * [参数]:
     *     <PLMN ID>: 网络ID。
     *     <Service Domain>: 上报拒绝原因值的服务域。
     *             0：CS only；
     *             1：PS only；
     *             2：CS+PS。
     *     <Reject Cause>: 网络拒绝原因值，上报协议中明确规定的拒绝原因值和Balong内部拓展的鉴权失败（#256 Authentication Failure）及联合注册中CS失败的Other Cause（#258，除#2、#16、#17、#22的其他原因值）及CS PS注册网络无响应（#301）及CS PS注册建链异常（#302、#303）。
     *             此外USIM鉴权失败的原因值也通过该命令上报，USIM的拒绝原因值从65537开始，包括(#65537、65538、65539、65540、65541)。
     *     <Rat Type>: 被拒的网络类型。
     *             0：GERAN；
     *             1：UTRAN；
     *             2：E-UTRAN；
     *             5：NR-5GC；
     *             6：其他。
     *     <Reject Type>: 网络被拒类型：
     *             0：LAU被拒
     *             1：鉴权失败
     *             2：业务请求被拒
     *             3：网络detach被拒
     *             4：ATTACH被拒
     *             5：RAU被拒
     *             6：TAU被拒
     *     <Original Reject Cause>: 网络拒绝的原始原因值，如果没有网络拒绝的原因值，就用0。
     *     <Lac>: 在2G，3G，4G下为2byte的16进制数的字符串类型，位置码信息，例如：“00C3”表示10进制的195。
     *             在5G下为3byte的16进制数的字符串类型，位置码信息，六个字符，16进制表示。（例：“0000C3”＝10进制的195）。
     *     <Rac>: 路由区域码
     *     <Cell Id>: 8bytes的16进制数的字符串类型，小区信息。使用Half-Byte码（半字节）编码，例如000000000A444202，表示高4字节是0，低4字节是0x0A444202。
     *     <Esm Reject Cause>: 当LNAS注册 被拒绝#19时,会带上这个值
     * [示例]:
     *       主动上报4G：
     *       ^REJINFO: 46000,1,40,2,3,40,"26F8","FF","000000000A444202"
     *       主动上报5G：
     *       ^REJINFO: 46000,1,40,5,3,40,"0026F8","FF","0000000F0A444202"
     *     · 查询命令，查询到的是被4G拒绝：
     *       AT^REJINFO?
     *       ^REJINFO: 46000,1,40,2,3,40,"26F8","FF","000000000A444202"
     *       OK
     *     · 查询命令，查询到的是被5G拒绝：
     *       AT^REJINFO?
     *       ^REJINFO: 46000,1,40,5,3,40,"0026F8","FF","0000000F0A444202"
     *       OK
     */
    { AT_CMD_REJINFO,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryRejInfoPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^REJINFO", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 频点信息扫描
     * [说明]: 测试命令返回支持的小区个数、门限及接入技术的范围
     *         获取指定接入技术的小区频点信息列表
     *         获取频点信息过程中，可通过输入打断字符串（该字符串可通过NV en_NV_Item_AT_ABORT_CMD_PARA配置，若配置打断字符串为ANY的ASCII码，则表示任意字符串打断，否则打断字符串按NV中配置的字符处理）打断该过程，打断完成后输出当前已经扫描到的频点信息以及“OK”（输出字符串不受NV项配置）。
     *         GU为主模时下发L或NR模式的NETSCAN命令会返回ERROR；L为主模时下发GU模式的NETSCAN命令会返回ERROR；NR为主模时，只允许下发NR模式的NETSCAN命令。
     *         L模下受NV (NV_ID_SWITCH_PARA)控制。NV默认关闭。
     * [语法]:
     *     [命令]: ^NETSCAN=<n>,<pow>[,<mode>[,<band>]]
     *     [结果]: [<CR><LF>^NETSCAN: [<arfcn>],[<c1>],[<c2>],[<lac>],[<mcc>],[<mnc>],[<bsic>], [<rxlev>][,<cid>,[<band>],[psc],[pci][,<5GSCS>, <5GRSRP>, <5GRSRQ>,<5GRSSNR>]]<CR><LF> [<CR><LF>^NETSCAN: [<arfcn>],[<c1>],[<c2>],[<lac>],[<mcc>],[<mnc>],[<bsic>], [<rxlev>][,<cid>,[<band>],[psc],[pci][,<5GSCS>, <5GRSRP>,<5GRSRQ>,<5GRSSNR>]]<CR><LF>[...]]] <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^NETSCAN=?
     *     [结果]: <CR><LF>^NETSCAN: (list of supported <n>s),(list of supported <pow>s),(list of supported <mode>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 需要扫描出超过门限的小区个数，取值范围[1,20]。
     *     <pow>: 小区能量门限值，GU下取值范围[-110dBm,-47dBm]，LTE、NR下取值范围[-125dBm,-47dBm]。
     *     <mode>: 接入技术。
     *             0：GSM（缺省值）；
     *             1：UMTS（仅FDD）；
     *             2：LTE；
     *             3：LTE-NRDC（仅LTE主模下使用，扫描NR网络）；
     *             4：NR（仅NR主模下使用）。
     *     <band>: 同^SYSCFGEX命令中的<band>或<lteband>参数；<mode>为3或4时此参数的每个Bit对应NR的Band，不携带则扫描全部支持的NR模式Band。
     *     <arfcn>: 频点。
     *     <C1>: 用于GSM小区重选的C1值（暂不支持）。
     *     <C2>: 用于GSM小区重选的C2值（暂不支持）。
     *     <lac>: 位置区码，16进制数字表示。
     *     <mcc>: PLMN国家码，3位数字表示，例如中国PLMN的国家码表示为460。
     *     <mnc>: PLMN网络码，2位或3位数字表示。
     *     <bsic>: GSM小区基站码。
     *     <rxlev>: 接收到的信号强度，单位dBm。
     *     <cid>: 小区ID，16进制显示。
     *     <psc>: 主扰码，十进制显示（WCDMA制式下参数值有效）。
     *     <pci>: 物理小区ID，十进制显示（LTE和NR制式下参数值有效）。
     *     <5GSCS>: NR副载波间隔信息，5G SubcarrierSpacing information，整型值：
     *             0：15kHz；
     *             1：30kHz；
     *             2：60kHz；
     *             3：120kHz；
     *             4：240khz。
     *     <5GRSRP>: NR参考信号接收功率，5G Reference Signal Received Power，整型值，无效值99，单位：0.5dBm。
     *     <5GRSRQ>: NR参考信号接收质量，5G Reference Signal Received Quality，整型值，无效值99，单位：dB。
     *     <5GRSSNR>: 对应NR物理层的SINR，5G Receiving signal strength in dbm，整型值，无效值99，单位：dBm。
     * [示例]:
     *     · 不带接入技术和频段扫描
     *       AT^NETSCAN=2,-110
     *       ^NETSCAN: 15,,,4,460,09,5,-64,2,80,0,0
     *       ^NETSCAN: 31,,,6,460,09,2,-100,2,80,0,0
     *       OK
     *     · 带接入技术不带频段扫描
     *       AT^NETSCAN=2,-110,0
     *       ^NETSCAN: 15,,,4,460,09,5,-64,2,80,0,0
     *       ^NETSCAN: 31,,,6,460,09,2,-100,2,80,0,0
     *       OK
     *       AT^NETSCAN=20,-100,2
     *       ^NETSCAN: 100,,,2540,460,11,0,-80,2632709,1,0,375
     *       ^NETSCAN: 42590,,,d,440,10,0,-94,d5a,20000000000,0,139
     *       ^NETSCAN: 1350,,,64,460,50,0,-94,0,4,0,0
     *       ^NETSCAN: 40886,,,64,460,50,0,-95,13,10000000000,0,19
     *       ^NETSCAN: 300,,,64,460,50,0,-100,1,1,0,1
     *       OK
     *     · 带接入技术和频段扫描
     *       AT^NETSCAN=2,-110,0,300
     *       ^NETSCAN: 15,,,4,460,09,-64,2,300,0,0
     *       ^NETSCAN: 31,,,6,460,09,-1000,2,100300,0,0
     *       OK
     *     · LTE主模时扫描NR模式
     *       AT^NETSCAN=3,-125,3
     *       ^NETSCAN: 633984,,,1,460,10,0,0,1088000,20000000000000000000,0,285,1,-95, -21,52
     *       OK
     *     · 测试命令
     *       AT^NETSCAN=?
     *       ^NETSCAN: (1-20),(-125--47),(0-4)
     *       OK
     *     · 获取频点信息打断
     *       AT^NETSCAN=2,-110,0,300
     *       ABCD(打断，设置任意字符打断)
     *       ^NETSCAN: 15,,,4,460,09,5,-64,2,300,0,0
     *       ^NETSCAN: 31,,,6,460,09,2,-100,2,300,0,0
     *       OK
     */
    { AT_CMD_NETSCAN,
      AT_SetNetScanPara, AT_NETSCAN_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestNetScanPara, AT_NOT_SET_TIME,
      AT_AbortNetScanPara, AT_ABORT_NETSCAN_SET_PARA_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^NETSCAN", (VOS_UINT8 *)NETSCAN_CMD_PARA_STRING },

    /* 根据干扰预警需求新增AT命令^EMRSSICFG */
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: RSSI预警上报参数设置命令
     * [说明]: 该命令用于配置GUL制式下，^EMRSSIRPT上报的阈值，当服务小区RSSI值变化超过阈值时，上报^EMRSSIRPT，指示当前RSSI值。
     * [语法]:
     *     [命令]: ^EMRSSICFG=<rat>,<rssi_thresh>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^EMRSSICFG?
     *     [结果]: <CR><LF>^EMRSSICFG: <rat>,<rssi_thresh>[<CR><LF>^EMRSSICFG: <rat>,<rssi_thresh>[…]]<CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: ^EMRSSICFG=?
     *     [结果]: <CR><LF>^EMRSSICFG: (list of supported <rat>s), (list of supported <rssi_thresh>s)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <rat>: 整型值，网络制式。
     *             0：GSM
     *             1：WCDMA
     *             2：LTE
     *     <rssi_thresh>: 整数值，服务小区RSSI值变化超过该阈值时，上报^EMRSSIRPT指示当前RSSI值。取值范围：[0，5-70]，单位dbm。
     * [示例]:
     *     · 设置LTE服务小区RSSI上报参数
     *       AT^EMRSSICFG=2,10
     *       OK
     */
    { AT_CMD_EMRSSICFG,
      AT_SetEmRssiCfgPara, AT_SET_PARA_TIME, AT_QryEmRssiCfgPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^EMRSSICFG", (VOS_UINT8 *)"(0-2),(0,5-70)" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 查询信号质量参数
     * [说明]: 该命令用于返回收到的信号质量参数。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CESQ
     *     [结果]: 执行设置成功时：
     *             <CR><LF>+CESQ: <rxlev>,<ber>,<rscp>,<ecno>,<rsrq>,<rsrp><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             执行错误时:
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CESQ=?
     *     [结果]: <CR><LF>+CESQ: (list of supported <rxlev>s),(list of supported<ber>s),(list of supported <rscp>s),(list of supported<ecno>s),(list of supported <rsrq>s),(list of supported <rsrp>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <rxlev>: 整型值，信号强度水平。
     *             有效范围：（0-63,99） 其中99为无效值
     *     <ber>: 整型值，信道误码率。
     *             有效范围：（0-7,99）其中99为无效值
     *     <rscp>: 整型值，接收信号码功率。
     *             有效范围：（0-96,255）其中255为无效值
     *     <ecno>: 整型值，每个PN码片的能量与功率谱密度的比值。
     *             有效范围：（0-49,255）其中255为无效值
     *     <rsrq>: 整型值，参考信号的接受质量。
     *             有效范围：（0-34,255）其中255为无效值
     *     <rsrp>: 整型值，参考信号的功率。
     *             有效范围：（0-97,255）其中255为无效值
     * [示例]:
     *     · 查询CS、PS域错误码
     *       AT+CESQ
     *       +CESQ: 99,99,50,30,255,255
     *       OK
     *     · 测试命令
     *       AT+CESQ=?
     *       +CESQ: (0-63,99),(0-7,99),(0-96,255),(0-49,255),(0-34,255),(0-97,255)
     *       OK
     */
    { AT_CMD_CESQ,
      At_SetCesqPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_TestCesqPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CESQ", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 查询EONS中UCS2编码的PLMN的网络名称
     * [说明]: 本命令用于查询EONS中UCS2编码的PLMN的网络名称。
     * [语法]:
     *     [命令]: ^EONSUCS2?
     *     [结果]: 执行查询成功时：
     *             <CR><LF>^EONSUCS2: <long_name>,<short_name><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             在CSG情况下：
     *             <CR><LF>^EONSUCS2: <home NodeB Name>< CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <long_name>: 字符串类型的PLMN的网络名称，长名称。以UCS-2格式编码。
     *     <short_name>: 字符串类型的PLMN的网络名称，短名称。以UCS-2格式编码。
     *     <home NodeB Name>: home NodeB Name。最大长度48字节。
     */
    { AT_CMD_EONSUCS2,
      VOS_NULL_PTR, AT_SET_PARA_TIME, At_QryEonsUcs2Para, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^EONSUCS2", VOS_NULL_PTR },

    /* NAS应终端要求新增AT命令 */
    /*
     * [类别]: 协议AT-NDIS特性
     * [含义]: 查询运营商
     * [说明]: 该命令用于查询运营商的PLMN、短名和长名，即根据<name>的值查询运营商的信息。
     * [语法]:
     *     [命令]: ^CRPN[=<name_index>,<name>]
     *     [结果]: <CR><LF>^CRPN: <plmn>,<short name>,<full name>[<CR><LF>^CRPN: <plmn>,<short name>,<full name> [...]] <CR><LF><CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CRPN?
     *     [结果]: <CR><LF>OK<CR><LF>
     *     [命令]: ^CRPN=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <name_index>: 整型值，取值如下：
     *             0：第二个参数<name>为运营商长名；
     *             1：第二个参数<name>为运营商短名；
     *             2：第二个参数<name>为运营商PLMN。
     *     <name>: 根据第一个参数<name_index>的设置，输入运营商的PLMN、短名或长名。
     *     <plmn>: 字符串类型，运营商的PLMN，0～6byte。
     *     <short name>: 字符串类型，运营商短名。
     *     <full name>: 字符串类型，运营商长名。
     * [示例]:
     *     · 根据运营商长名查询运营商信息
     *       AT^CRPN=0,"O2-CZ"
     *       ^CRPN: "23002","O2 - CZ","O2 - CZ"
     *       OK
     *     · 根据运营商短名查询运营商信息
     *       AT^CRPN=1,"O2-CZ"
     *       ^CRPN: "23002","O2 - CZ","O2 - CZ"
     *       OK
     *     · 根据运营商PLMN查询运营商信息
     *       AT^CRPN=2,"23002"
     *       ^CRPN: "23002","O2 - CZ","O2 - CZ"
     *       OK
     *     · 查询命令
     *       AT^CRPN?
     *       OK
     *     · 测试命令
     *       AT^CRPN=?
     *       OK
     */
    { AT_CMD_CRPN,
      At_SetCrpnPara, AT_SET_PARA_TIME, At_QryCrpnPara, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^CRPN", (VOS_UINT8 *)"(0-2),(PLMN)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询当前是否有业务存在
     * [说明]: 该命令用于查询当前是否有CS业务和PS业务存在，目前暂不提供PS业务状态的查询，只保留该接口，第二个参数固定填成0，并无实际意义。
     * [语法]:
     *     [命令]: ^USERSRVSTATE?
     *     [结果]: <CR><LF>^USERSRVSTATE: <n,m><CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: ^USERSRVSTATE=?
     *     [结果]: <CR><LF>^USERSRVSTATE: (list of supported <n>s),(list of supported <m>s)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 当前是否有CS业务存在。
     *             0：无CS业务；
     *             1：有CS业务。
     *     <m>: 当前是否有PS业务存在，取值为0或1。
     *             注意：目前暂不提供PS业务状态的查询，只保留该接口，m取值固定为0，并无实际意义。
     * [示例]:
     *     · 查询是否有业务存在
     *       AT^USERSRVSTATE?
     *       ^USERSRVSTATE: 1,0
     *       OK
     */
    { AT_CMD_USERSRVSTATE,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryUserSrvStatePara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^USERSRVSTATE", (VOS_UINT8 *)"(0,1),(0,1)" },
};

/* 注册taf phone AT命令表 */
VOS_UINT32 AT_RegisterPhoneMmCmdTbl(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atPhoneMmCmdTbl, sizeof(g_atPhoneMmCmdTbl) / sizeof(g_atPhoneMmCmdTbl[0]));
}

