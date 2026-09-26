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
#include "at_custom_lrrc_set_cmd_proc.h"
#include "securec.h"

#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_lte_common.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_LRRC_SET_CMD_PROC_C

#define AT_VTRLQUALRPT_PARA_NUM 2
#define AT_LCACFG_MAX_PARA_NUM 11
#define AT_LCACFG_MIN_PARA_NUM 1
#define AT_LCACFG_ENABLE 0
#define AT_LCACFG_CFG_PARA 1
#define AT_LCACFG_BAND_NUM 2
#define AT_DISABLE_LCACFG_MIN_PARA_NUM 3
#define AT_CONFIG_ENABLE_PARA_NUM 1

VOS_UINT32 AT_SetVtrlqualrptPara(VOS_UINT8 indexNum)
{
    AT_MTA_SetVtrlqualrptReq setVtrlqualrpt;
    VOS_UINT32               rst;

    (VOS_VOID)memset_s(&setVtrlqualrpt, sizeof(setVtrlqualrpt), 0x00, sizeof(setVtrlqualrpt));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不正确 */
    if (g_atParaIndex != AT_VTRLQUALRPT_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[1].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数赋值 */
    setVtrlqualrpt.enable    = g_atParaList[0].paraValue;
    setVtrlqualrpt.threshold = g_atParaList[1].paraValue;

    /* 发送跨核消息到C核 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_MTA_VTRLQUALRPT_SET_REQ, (VOS_UINT8 *)&setVtrlqualrpt,
                                 (VOS_SIZE_T)sizeof(setVtrlqualrpt), I0_UEPS_PID_MTA);
    if (rst != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetVtrlqualrptPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_VTRLQUALRPT_SET;

    return AT_WAIT_ASYNC_RETURN;
}
#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_ProLCaCfgPara(AT_MTA_CaCfgSetReq *caCfgReq)
{
    errno_t    memResult;
    VOS_UINT32 paraIndex;
    VOS_UINT32 i;

    /* 如果配置为使能，只需要一个参数 */
    if (g_atParaList[AT_LCACFG_ENABLE].paraValue == AT_CONFIG_ENABLE_PARA_NUM) {
        caCfgReq->caActFlag = (VOS_UINT8)g_atParaList[AT_LCACFG_ENABLE].paraValue;
        return AT_OK;
    }
    /* 如果配置为去使能，至少需要三个参数参数 */
    if ((g_atParaIndex < AT_DISABLE_LCACFG_MIN_PARA_NUM) || (g_atParaList[AT_LCACFG_CFG_PARA].paraLen == 0) ||
        (g_atParaList[AT_LCACFG_BAND_NUM].paraLen == 0)) {
        AT_WARN_LOG("AT_ProLCaCfgPara: para num is error or para len is 0.");
        return AT_ERROR;
    }

    /* band num值和band数量对不上 */
    if ((g_atParaIndex - AT_MTA_BAND_INFO_OFFSET) != g_atParaList[AT_LCACFG_BAND_NUM].paraValue) {
        AT_WARN_LOG("AT_ProLCaCfgPara: para num is error.");
        return AT_ERROR;
    }

    caCfgReq->caActFlag       = (VOS_UINT8)g_atParaList[AT_LCACFG_ENABLE].paraValue;
    caCfgReq->caInfo.caA2Flg  = ((VOS_UINT8)g_atParaList[AT_LCACFG_CFG_PARA].paraValue) & (0x01);
    /* bit位为1位打开A4功能 */
    caCfgReq->caInfo.caA4Flg  = (((VOS_UINT8)g_atParaList[AT_LCACFG_CFG_PARA].paraValue) & (0x02)) >> 1;
    /* bit位为2位打开CQI=0功能 */
    caCfgReq->caInfo.caCqiFlg = (((VOS_UINT8)g_atParaList[AT_LCACFG_CFG_PARA].paraValue) & (0x04)) >> 2;
    caCfgReq->caInfo.bandNum  = (VOS_UINT16)g_atParaList[AT_LCACFG_BAND_NUM].paraValue;

    for (i = 0; i < (VOS_UINT32)TAF_MIN(caCfgReq->caInfo.bandNum, AT_MTA_MAX_BAND_NUM); i++) {
        paraIndex = AT_MTA_BAND_INFO_OFFSET + i;

        if (At_AsciiNum2HexString(g_atParaList[paraIndex].para, &(g_atParaList[paraIndex].paraLen)) == AT_FAILURE) {
            return AT_ERROR;
        }

        if (sizeof(AT_MTA_BandInfo) != g_atParaList[paraIndex].paraLen) {
            AT_WARN_LOG("AT_ProLCaCfgPara: para len is error.");
            return AT_ERROR;
        }

        memResult = memcpy_s(&(caCfgReq->caInfo.bandInfo[i]), (VOS_SIZE_T)sizeof(caCfgReq->caInfo.bandInfo[i]),
                             g_atParaList[paraIndex].para, g_atParaList[paraIndex].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, (VOS_SIZE_T)sizeof(caCfgReq->caInfo.bandInfo[i]),
                            g_atParaList[paraIndex].paraLen);
    }

    return AT_OK;
}

VOS_UINT32 AT_SetLCaCfgPara(VOS_UINT8 indexNum)
{
    AT_MTA_CaCfgSetReq caCfgReq;
    VOS_UINT32         rst;

    (VOS_VOID)memset_s(&caCfgReq, sizeof(caCfgReq), 0x00, sizeof(caCfgReq));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不正确 */
    if ((g_atParaIndex < AT_LCACFG_MIN_PARA_NUM) || (g_atParaIndex > AT_LCACFG_MAX_PARA_NUM)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数有效性检查 */
    if (AT_ProLCaCfgPara(&caCfgReq) == AT_ERROR) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 发送跨核消息到C核, 设置低功耗 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_MTA_LTE_CA_CFG_SET_REQ, (VOS_UINT8 *)&caCfgReq, sizeof(caCfgReq),
                                 I0_UEPS_PID_MTA);

    if (rst != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetLCaCfgPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_LTE_CA_CFG_SET;

    return AT_WAIT_ASYNC_RETURN;
}
#endif

#if (FEATURE_MBB_CUST == FEATURE_ON)
/*
 * 功能描述: ^LCELLINFO命令的打断处理
 *
 */
VOS_UINT32 AT_SetLCellInfoPara(VOS_UINT8 indexNum)
{
    L4A_READ_CellInfoReq cellInfo;
    VOS_UINT32 result;

    (VOS_VOID)memset_s(&cellInfo, sizeof(cellInfo), 0x00, sizeof(cellInfo));
    /* 参数过多 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    cellInfo.ctrl.clientId = g_atClientTab[indexNum].clientId;
    cellInfo.ctrl.opId = 0;
    cellInfo.ctrl.pid = WUEPS_PID_AT;
    cellInfo.cellFlag = g_atParaList[0].paraValue;

    result = atSendL4aDataMsg(g_atClientTab[indexNum].clientId, I0_MSP_L4_L4A_PID, ID_MSG_L4A_CELL_INFO_REQ,
        (VOS_VOID *)&cellInfo, sizeof(cellInfo));

    if (result == ERR_MSP_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_LCELLINFO_QUERY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
#endif
