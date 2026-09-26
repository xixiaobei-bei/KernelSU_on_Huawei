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

#include "at_stub_mm_set_cmd_proc.h"

#include "at_set_para_cmd.h"
#include "securec.h"
#include "at_mdrv_interface.h"
#include "taf_type_def.h"
#include "at_cmd_proc.h"
#include "at_input_proc.h"
#include "at_check_func.h"
#include "at_parse_cmd.h"
#include "ppp_interface.h"
#include "at_msg_print.h"
#include "si_app_stk.h"
#include "si_app_pih.h"
#include "at_phy_interface.h"
#include "at_data_proc.h"
#include "taf_drv_agent.h"
#include "mn_comm_api.h"
#include "at_cmd_msg_proc.h"
#include "at_rabm_interface.h"
#include "at_snd_msg.h"
#include "at_device_cmd.h"
#include "at_rnic_interface.h"
#include "at_taf_agent_interface.h"
#include "taf_agent_interface.h"
#include "taf_app_xsms_interface.h"

#include "taf_std_lib.h"
#include "mnmsgcbencdec.h"

#if (FEATURE_LTE == FEATURE_ON)
#include "mdrv_nvim.h"
#include "at_common.h"
#include "gen_msg.h"
#include "at_lte_common.h"
#endif
#if (FEATURE_LTEV == FEATURE_ON)
#include "vnas_at_interface.h"
#endif

#include "nv_stru_gucnas.h"
#include "nv_stru_sys.h"
#include "nv_stru_gas.h"
#include "nv_stru_pam.h"
#include "nv_stru_msp_interface.h"
#include "nv_stru_msp.h"

#include "mn_call_api.h"
#include "at_ctx.h"
#include "app_vc_api.h"
#include "at_imsa_interface.h"
#include "at_init.h"
#include "at_file_handle.h"

#include "gunas_errno.h"

#include "css_at_interface.h"
#include "taf_app_mma.h"
#include "taf_mtc_api.h"

#include "taf_ccm_api.h"

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
#if (VOS_OS_VER != VOS_LINUX)
#include "Linuxstub.h"
#endif
#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
#include "ads_dev_i.h"
#else
#include "ads_device_interface.h"
#endif
#else
#include "cds_interface.h"
#endif

#if (VOS_OS_VER == VOS_WIN32)
#include "NasStub.h"
#include "UsimmApi.h"
#endif

#include "imm_interface.h"
#include "throt_at_interface.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_STUB_MM_SET_CMD_PROC_C

#define AT_RPLMNSTUB_L_RPLMN 0
#define AT_IMSI_PLMN_LIST_ARRAY_MAX_LEN 16
#define AT_DAM_PLMN_LIST_ARRAY_MAX_LEN 16
#define AT_DAMCFGSTUB_DAM_ACTIVE_FLG 0
#define AT_DAMCFGSTUB_ENABLE_LTE 1
#define AT_EONS_PRIO_TYPE_ARRAY_INDEX_0 0
#define AT_EONS_PRIO_TYPE_ARRAY_INDEX_1 1
#define AT_EONS_PRIO_TYPE_ARRAY_INDEX_2 2
#define AT_EONS_PRIO_TYPE_ARRAY_INDEX_3 3
#define AT_RPLMNSTUB_LAI_VALID_LEN 4
#define AT_RPLMNSTUB_RAC_VALID_LEN 2
#define AT_RPLMNSTUB_LAI 2
#define AT_RPLMNSTUB_RAC 3
#define AT_CSGSTUB_UECSG 0
#define AT_CSGSTUB_EFAD 1
#define AT_CSGSTUB_CSGLIST 2
#define AT_CSUNAVAILPLMNSTUB_PLMNID 1
#define AT_NPLMN_PLMN_ID 1
#define AT_EONSSTUB_NCNUM 0
#define AT_EONSSTUB_TYPE1 1
#define AT_EONSSTUB_TYPE2 2
#define AT_EONSSTUB_TYPE3 3
#define AT_EONSSTUB_TYPE4 4
#define AT_EONSSTUB_RESTART_FLAG 5
#define AT_DAMCFGSTUB_IMSI_PLMN_NUM 3
#define AT_NPLMN_SET_NPLMN_NUM 0
#define AT_NPLMN_MIN_LENGTH 5
#define AT_NPLMN_MAX_LENGTH 6
#define AT_NPLMN_SIMRAT 2
#define AT_RPLMNSTUB_PLMN_ID 1
#define AT_EONS_PRIO_TYPE_ARRAY_MAX_LEN 4
#define AT_DAMCFGSTUB_DAM_DISABLE_LTE 2
#define AT_DAMCFGSTUB_IMSI_PLMN_MAX_VALUE 16
#define AT_HALF_BYTE_BIT_LEN 4


#if (VOS_WIN32 == VOS_OS_VER)
VOS_UINT32 AT_SetRplmnStub(VOS_UINT8 indexNum)
{
    TAF_PLMN_Id plmnId;
    VOS_UINT32  lai;
    VOS_UINT32  rac;

    /*
     * 参数不正确,
     * 0:表示需要设置L的RPLMN
     * 1:表示需要设置GU的RPLMN
     */
    if ((g_atParaList[AT_RPLMNSTUB_L_RPLMN].paraValue != 0) && (g_atParaList[AT_RPLMNSTUB_L_RPLMN].paraValue != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 需要设置PLMN ID的值 其长度只能为5 或者6 */
    if ((g_atParaList[AT_RPLMNSTUB_PLMN_ID].paraLen != AT_PLMN_STR_MIN_LEN) &&
        (g_atParaList[AT_RPLMNSTUB_PLMN_ID].paraLen != AT_PLMN_STR_MAX_LEN)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* Mcc */
    if (At_String2Hex(g_atParaList[AT_RPLMNSTUB_PLMN_ID].para, AT_MCC_LENGTH, &plmnId.mcc) == AT_FAILURE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    /* Mnc */
    if (At_String2Hex(&g_atParaList[AT_RPLMNSTUB_PLMN_ID].para[3],
                      g_atParaList[AT_RPLMNSTUB_PLMN_ID].paraLen - AT_MNC_LENGTH,
                      &plmnId.mnc) == AT_FAILURE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    plmnId.mcc |= 0xFFFFF000U;
    /* mnc一个数字只占半个字节，也就是4bit */
    plmnId.mnc |= (0xFFFFFFFFU << ((g_atParaList[AT_RPLMNSTUB_PLMN_ID].paraLen - AT_MNC_LENGTH) * 4));

    /* lai */
    lai = 0x0001;
    if (g_atParaList[AT_RPLMNSTUB_LAI].paraLen == AT_RPLMNSTUB_LAI_VALID_LEN) {
        if (At_String2Hex(g_atParaList[AT_RPLMNSTUB_LAI].para, AT_RPLMNSTUB_LAI_VALID_LEN, &lai) == AT_FAILURE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    /* rac  */
    rac = 0x01;
    if (g_atParaList[AT_RPLMNSTUB_RAC].paraLen == AT_RPLMNSTUB_RAC_VALID_LEN) {
        if (At_String2Hex(g_atParaList[AT_RPLMNSTUB_RAC].para, AT_RPLMNSTUB_RAC_VALID_LEN, &rac) == AT_FAILURE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    /* 需要设置L的RPLMN */
    if (g_atParaList[AT_RPLMNSTUB_L_RPLMN].paraValue == 0) {
        NAS_SetLteRplmnId(&plmnId, lai, rac);
    } else {
    }

    return AT_OK;
}

VOS_UINT32 AT_SetTinTypeStub(VOS_UINT8 indexNum)
{
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 设置TIN类型 */
    NAS_SetTinType(g_atParaList[0].paraValue);

    return AT_OK;
}

VOS_UINT32 AT_SetCsUnAvailPlmnStub(VOS_UINT8 indexNum)
{
    TAF_PLMN_Id plmnId;

    /*
     * 参数不正确,
     * 0:表示需要删除CS不可用网络
     * 1:表示需要新增CS不可用网络
     */
    if ((g_atParaList[0].paraValue != 0) && (g_atParaList[0].paraValue != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraValue != 0) {
        /* 需要设置PLMN ID的值 其参数长度只能为5 或者6  */
        if ((g_atParaList[AT_CSUNAVAILPLMNSTUB_PLMNID].paraLen != AT_PLMN_STR_MIN_LEN) &&
            (g_atParaList[AT_CSUNAVAILPLMNSTUB_PLMNID].paraLen != AT_PLMN_STR_MAX_LEN)) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* Mcc */
        if (At_String2Hex(g_atParaList[AT_CSUNAVAILPLMNSTUB_PLMNID].para, AT_MCC_LENGTH, &plmnId.mcc) == AT_FAILURE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
        /* Mnc */
        if (At_String2Hex(&g_atParaList[AT_CSUNAVAILPLMNSTUB_PLMNID].para[3],
                          g_atParaList[AT_CSUNAVAILPLMNSTUB_PLMNID].paraLen - AT_MNC_LENGTH, &plmnId.mnc) ==
                          AT_FAILURE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
        plmnId.mcc |= 0xFFFFF000U;
        /* mnc一个数字只占半个字节，也就是4bit */
        plmnId.mnc |= (0xFFFFFFFFU << ((g_atParaList[AT_CSUNAVAILPLMNSTUB_PLMNID].paraLen - AT_MNC_LENGTH) * 4));
    }

    /* 需要设置L的CS不可用网络 */
    if (NAS_SetCsUnAvailPlmnId(g_atParaList[0].paraValue, &plmnId) == VOS_OK) {
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetForbRoamTaStub(VOS_UINT8 indexNum)
{
    /*
     * 参数不正确,
     * 0:表示需要删除禁止漫游TA
     * 1:表示需要新增禁止漫游TA
     */
    if ((g_atParaList[0].paraValue != 0) && (g_atParaList[0].paraValue != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 需要设置L的禁止漫游TA */
    if (NAS_SetForbRoamTa(g_atParaList[0].paraValue) == VOS_OK) {
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetDisableRatPlmnStub(VOS_UINT8 indexNum)
{
    /*
     * 参数不正确,
     * 0:表示需要删除禁止接入技术网络列表
     */
    if (g_atParaList[0].paraValue != 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (NAS_SetDisableRatPlmn(g_atParaList[0].paraValue) == VOS_OK) {
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetCsgStub(VOS_UINT8 indexNum)
{
    /* 第一个参数为设置UE是否支持CSG功能，0:不支持csg功能；1:支持csg功能 */
    if (At_String2Hex(g_atParaList[AT_CSGSTUB_UECSG].para, g_atParaList[AT_CSGSTUB_UECSG].paraLen,
                      &g_atParaList[AT_CSGSTUB_UECSG].paraValue) == AT_FAILURE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[AT_CSGSTUB_UECSG].paraValue != 0) && (g_atParaList[AT_CSGSTUB_UECSG].paraValue != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第二个参数设置EFAD文件中csg display indicator的值，0:都上报；1:只上报operator csg list中csg id */
    if (At_String2Hex(g_atParaList[AT_CSGSTUB_EFAD].para, g_atParaList[AT_CSGSTUB_EFAD].paraLen,
                      &g_atParaList[AT_CSGSTUB_EFAD].paraValue) == AT_FAILURE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[AT_CSGSTUB_EFAD].paraValue != 0) && (g_atParaList[AT_CSGSTUB_EFAD].paraValue != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第3个参数csg list搜网是否仅上报operator csg list中的网络，0:都上报；1:只上报operator csg list中csg id */
    if (At_String2Hex(g_atParaList[AT_CSGSTUB_CSGLIST].para, g_atParaList[AT_CSGSTUB_CSGLIST].paraLen,
                      &g_atParaList[AT_CSGSTUB_CSGLIST].paraValue) == AT_FAILURE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[AT_CSGSTUB_CSGLIST].paraValue != 0) && (g_atParaList[AT_CSGSTUB_CSGLIST].paraValue != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    NAS_SetCsgCfg(g_atParaList[AT_CSGSTUB_UECSG].paraValue, g_atParaList[AT_CSGSTUB_EFAD].paraValue,
                  g_atParaList[AT_CSGSTUB_CSGLIST].paraValue);

    return AT_OK;
}

VOS_UINT32 AT_CheckDamActiveFlgPara(VOS_VOID)
{
    if (At_String2Hex(g_atParaList[AT_DAMCFGSTUB_DAM_ACTIVE_FLG].para,
        g_atParaList[AT_DAMCFGSTUB_DAM_ACTIVE_FLG].paraLen,
        &g_atParaList[AT_DAMCFGSTUB_DAM_ACTIVE_FLG].paraValue) == AT_FAILURE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[AT_DAMCFGSTUB_DAM_ACTIVE_FLG].paraValue != 0) &&
        (g_atParaList[AT_DAMCFGSTUB_DAM_ACTIVE_FLG].paraValue != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}

VOS_UINT32 AT_CheckEnableLteTrigPlmnSearchFlagPara(VOS_VOID)
{
    if (At_String2Hex(g_atParaList[AT_DAMCFGSTUB_ENABLE_LTE].para, g_atParaList[AT_DAMCFGSTUB_ENABLE_LTE].paraLen,
        &g_atParaList[AT_DAMCFGSTUB_ENABLE_LTE].paraValue) == AT_FAILURE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[AT_DAMCFGSTUB_ENABLE_LTE].paraValue != 0) &&
        (g_atParaList[AT_DAMCFGSTUB_ENABLE_LTE].paraValue != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}

VOS_UINT32 AT_CheckAddDamPlmnInDisabledPlmnListPara(VOS_VOID)
{
    if (At_String2Hex(g_atParaList[AT_DAMCFGSTUB_DAM_DISABLE_LTE].para,
        g_atParaList[AT_DAMCFGSTUB_DAM_DISABLE_LTE].paraLen,
        &g_atParaList[AT_DAMCFGSTUB_DAM_DISABLE_LTE].paraValue) == AT_FAILURE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[AT_DAMCFGSTUB_DAM_DISABLE_LTE].paraValue != 0) &&
        (g_atParaList[AT_DAMCFGSTUB_DAM_DISABLE_LTE].paraValue != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}

VOS_UINT32 AT_CheckImsiPlmnNumPara(VOS_VOID)
{
    if (At_String2Hex(g_atParaList[AT_DAMCFGSTUB_IMSI_PLMN_NUM].para, g_atParaList[AT_DAMCFGSTUB_IMSI_PLMN_NUM].paraLen,
        &g_atParaList[AT_DAMCFGSTUB_IMSI_PLMN_NUM].paraValue) == AT_FAILURE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[AT_DAMCFGSTUB_IMSI_PLMN_NUM].paraValue > AT_DAMCFGSTUB_IMSI_PLMN_MAX_VALUE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}

VOS_UINT32 AT_SetDamParaStub(VOS_UINT8 indexNum)
{
    TAF_PLMN_Id imsiPlmnList[AT_IMSI_PLMN_LIST_ARRAY_MAX_LEN];
    VOS_UINT8   imsiPlmnNum;
    TAF_PLMN_Id damPlmnList[AT_DAM_PLMN_LIST_ARRAY_MAX_LEN];
    VOS_UINT8   damPlmnNum;
    VOS_UINT8   enableLteTrigPlmnSearchFlag = VOS_FALSE;
    VOS_UINT8   addDamPlmnInDisabledPlmnList;
    VOS_UINT8   i;
    VOS_UINT8   paraIndex;
    VOS_UINT32  mcc;
    VOS_UINT32  mnc;
    VOS_UINT8   damActiveFlg = VOS_FALSE;

    (VOS_VOID)memset_s(&imsiPlmnList, sizeof(imsiPlmnList), 0x00, sizeof(imsiPlmnList));
    (VOS_VOID)memset_s(&damPlmnList, sizeof(damPlmnList), 0x00, sizeof(damPlmnList));

    /*
     * 第一个参数为enable lte是否需要搜网，0为不需要；1为需要；
     * 第二个参数为DAM网络disable lte后是否加入禁止接入技术网络列表中，0为不需要；1为需要；
     * 第三个参数为imsi plmn list个数，如果个数为2，第四个参数和第五个参数为 imsi plmn id。
     * 第六个参数为DAM plmn list 个数，如果个数为2，第七个参数和第8个参数为dam plmn id
     */

    if (AT_CheckDamActiveFlgPara() == AT_CME_INCORRECT_PARAMETERS) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 获取ucDamActiveFlg */
    damActiveFlg = g_atParaList[AT_DAMCFGSTUB_DAM_ACTIVE_FLG].paraValue;

    if (AT_CheckEnableLteTrigPlmnSearchFlagPara() == AT_CME_INCORRECT_PARAMETERS) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 获取ucEnableLteTrigPlmnSearchFlag */
    enableLteTrigPlmnSearchFlag = g_atParaList[AT_DAMCFGSTUB_ENABLE_LTE].paraValue;

    if (AT_CheckAddDamPlmnInDisabledPlmnListPara() == AT_CME_INCORRECT_PARAMETERS) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 获取ucAddDamPlmnInDisabledPlmnList */
    addDamPlmnInDisabledPlmnList = g_atParaList[AT_DAMCFGSTUB_DAM_DISABLE_LTE].paraValue;

    if (AT_CheckImsiPlmnNumPara() == AT_CME_INCORRECT_PARAMETERS) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 获取ucImsiPlmnNum */
    imsiPlmnNum = g_atParaList[AT_DAMCFGSTUB_IMSI_PLMN_NUM].paraValue;
    for (i = 1; i <= imsiPlmnNum; i++) {
        /* Mcc */
        if (At_String2Hex(g_atParaList[AT_DAMCFGSTUB_IMSI_PLMN_NUM + i].para, AT_MCC_LENGTH, &mcc) == AT_FAILURE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        imsiPlmnList[i - 1].mcc = mcc;

        /* Mnc */
        if (At_String2Hex(&g_atParaList[AT_DAMCFGSTUB_IMSI_PLMN_NUM + i].para[AT_COMMAND_PARA_INDEX_3],
                          g_atParaList[AT_DAMCFGSTUB_IMSI_PLMN_NUM + i].paraLen - AT_MCC_LENGTH, &mnc) == AT_FAILURE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        imsiPlmnList[i - 1].mnc = mnc;

        imsiPlmnList[i - 1].mcc |= 0xFFFFF000U;
        imsiPlmnList[i - 1].mnc |= (0xFFFFFFFFU <<
            ((g_atParaList[AT_DAMCFGSTUB_IMSI_PLMN_NUM + i].paraLen - AT_MCC_LENGTH) * AT_HALF_BYTE_BIT_LEN));
    }

    /* 第三个参数为imsi plmn list个数，加3为第六个参数为DAM plmn list个数，再加1为第七个参数dam plmn id */
    paraIndex = 3 + imsiPlmnNum + 1;
    if (At_String2Hex(g_atParaList[paraIndex].para, g_atParaList[paraIndex].paraLen,
                      &g_atParaList[paraIndex].paraValue) == AT_FAILURE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    damPlmnNum = g_atParaList[paraIndex].paraValue;

    for (i = 1; i <= damPlmnNum; i++) {
        /* Mcc */
        if (At_String2Hex(g_atParaList[paraIndex + i].para, AT_MCC_LENGTH, &damPlmnList[i - 1].mcc) == AT_FAILURE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* Mnc */
        if (At_String2Hex(&g_atParaList[paraIndex + i].para[AT_COMMAND_PARA_INDEX_3],
            g_atParaList[paraIndex + i].paraLen - AT_MCC_LENGTH, &damPlmnList[i - 1].mnc) == AT_FAILURE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        damPlmnList[i - 1].mcc |= 0xFFFFF000U;
        damPlmnList[i - 1].mnc |= (0xFFFFFFFFU <<
            ((g_atParaList[paraIndex + i].paraLen - AT_MCC_LENGTH) * AT_HALF_BYTE_BIT_LEN));
    }

    NAS_SetDamCfg(damActiveFlg, enableLteTrigPlmnSearchFlag, addDamPlmnInDisabledPlmnList, imsiPlmnNum, imsiPlmnList,
                  damPlmnNum, damPlmnList);
    return AT_OK;
}

VOS_UINT32 AT_SetEonsParaStub(VOS_UINT8 indexNum)
{
    VOS_UINT8 num;
    VOS_UINT8 eonsPrioType[AT_EONS_PRIO_TYPE_ARRAY_MAX_LEN];
    VOS_UINT8 refreshAllFileRestartFlag;

    (VOS_VOID)memset_s(&eonsPrioType, sizeof(eonsPrioType), 0x00, sizeof(eonsPrioType));

    num                                           = (VOS_UINT8)g_atParaList[AT_EONSSTUB_NCNUM].paraValue;
    eonsPrioType[AT_EONS_PRIO_TYPE_ARRAY_INDEX_0] = (VOS_UINT8)g_atParaList[AT_EONSSTUB_TYPE1].paraValue;
    eonsPrioType[AT_EONS_PRIO_TYPE_ARRAY_INDEX_1] = (VOS_UINT8)g_atParaList[AT_EONSSTUB_TYPE2].paraValue;
    eonsPrioType[AT_EONS_PRIO_TYPE_ARRAY_INDEX_2] = (VOS_UINT8)g_atParaList[AT_EONSSTUB_TYPE3].paraValue;
    eonsPrioType[AT_EONS_PRIO_TYPE_ARRAY_INDEX_3] = (VOS_UINT8)g_atParaList[AT_EONSSTUB_TYPE4].paraValue;
    refreshAllFileRestartFlag                     = (VOS_UINT8)g_atParaList[AT_EONSSTUB_RESTART_FLAG].paraValue;

    NAS_SetEonsPara(num, eonsPrioType, refreshAllFileRestartFlag);

    return AT_OK;
}

VOS_UINT32 AT_SetWildCardParaStub(VOS_UINT8 indexNum)
{
    NAS_SetWildCardPara(g_atParaList[0].para[0]);

    return AT_OK;
}

VOS_UINT32 AT_SetImsVoiceInterSysLauEnableStub(VOS_UINT8 indexNum)
{
    VOS_UINT8 forceLauFlag;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex > 1) {
        return AT_TOO_MANY_PARA;
    }

    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 获取漫游支持参数 */
    forceLauFlag = g_atParaList[0].paraValue;

    NAS_SetImsVoiceInterSysLauEnable(forceLauFlag);

    return AT_OK;
}

VOS_UINT32 AT_SetImsVoiceMMEnableStub(VOS_UINT8 indexNum)
{
    VOS_UINT8 imsVoiceMMEnableFlag;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex > 1) {
        return AT_TOO_MANY_PARA;
    }

    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 获取漫游支持参数 */
    imsVoiceMMEnableFlag = g_atParaList[0].paraValue;

    NAS_SetImsVoiceMMEnableFlg(imsVoiceMMEnableFlag);

    return AT_OK;
}

VOS_UINT32 AT_ResetNplmn(VOS_UINT8 indexNum)
{
    return AT_OK;
}

VOS_UINT32 AT_SetNplmn(VOS_UINT8 indexNum)
{
    return AT_OK;
}

VOS_UINT32 At_SetDelayBgStub(VOS_UINT8 indexNum)
{
    VOS_UINT32 delayLen = 0;
    VOS_UINT32 onOff = 0;

    At_String2Hex(g_atParaList[0].para, g_atParaList[0].paraLen, &onOff);
    At_String2Hex(g_atParaList[1].para, g_atParaList[1].paraLen, &delayLen);

    NAS_SetHighPrioPlmnRefreshTriggerBGSearchCfg((VOS_UINT8)onOff, delayLen);

    return AT_OK;
}

#endif

