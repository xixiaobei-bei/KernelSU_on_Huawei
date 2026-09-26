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
#include "at_device_drv_qry_cmd_proc.h"
#include "securec.h"

#include "at_snd_msg.h"
#include "at_cmd_proc.h"
#include "taf_app_mma.h"
#include "at_mdrv_interface.h"


#define THIS_FILE_ID PS_FILE_ID_AT_DEVICE_DRV_QRY_CMD_PROC_C

VOS_UINT32 AT_QryBatVolPara(VOS_UINT8 indexNum)
{
    /* 获取 电池电压值 */
    if (AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                               DRV_AGENT_TBATVOLT_QRY_REQ, VOS_NULL_PTR, 0, I0_WUEPS_PID_DRV_AGENT) == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_TBATVOLT_QRY; /* 设置当前操作模式 */
        return AT_WAIT_ASYNC_RETURN;                                 /* 等待异步事件返回 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_QryVersion(VOS_UINT8 indexNum)
{
    AT_PR_LOGI("Rcv Msg");

    if (AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                               DRV_AGENT_VERSION_QRY_REQ, VOS_NULL_PTR, 0, I0_WUEPS_PID_DRV_AGENT) == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_VERSION_QRY;

        AT_PR_LOGI("Snd Msg");
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_QryGPIOPL(VOS_UINT8 indexNum)
{
    if (AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, DRV_AGENT_GPIOPL_QRY_REQ,
                               VOS_NULL_PTR, 0, I0_WUEPS_PID_DRV_AGENT) == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_GPIOPL_QRY; /* 设置当前操作模式 */
        return AT_WAIT_ASYNC_RETURN;                               /* 等待异步事件返回 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_QryCsdfltPara(VOS_UINT8 indexNum)
{
    /* 发消息到C核获取SIMLOCK 状态信息 */
    if (Taf_ParaQuery(g_atClientTab[indexNum].clientId, 0, TAF_PH_SIMLOCK_VALUE_PARA, TAF_NULL_PTR) != TAF_SUCCESS) {
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CSDFLT_READ;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryTbatPara(VOS_UINT8 indexNum)
{
    /*
     * 调用底软接口获取电池安装方式:
     * <mount type> 电池安装方式
     * 0 无电池
     * 1 可更换电池
     * 2 内置一体化电池
     */
    /* 命令状态类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_DEVICE_OTHER_ERROR;
    }

    if (AT_IsMiscSupport(BSP_MODULE_TYPE_CHARGE) == BSP_MODULE_UNSUPPORT) {
        return AT_ERROR;
    }

    if (AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, DRV_AGENT_TBAT_QRY_REQ,
                               VOS_NULL_PTR, 0, I0_WUEPS_PID_DRV_AGENT) == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_TBAT_QRY; /* 设置当前操作模式 */
        /* 等待异步事件返回 */
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryChipTempPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;

    /* 发消息到C核获取PA、SIM卡和电池的温度信息 */
    ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 DRV_AGENT_CHIPTEMP_QRY_REQ, VOS_NULL_PTR, 0, I0_WUEPS_PID_DRV_AGENT);
    if (ret != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryChipTempPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CHIPTEMP_READ;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryNvBackupStatusPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;

    /* 发消息到C核获取当前NV备份状态信息 */
    ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 DRV_AGENT_NVBACKUPSTAT_QRY_REQ, VOS_NULL_PTR, 0, I0_WUEPS_PID_DRV_AGENT);
    if (ret != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryNvBackupStatusPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NVBACKUPSTAT_READ;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryNandBadBlockPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;

    /* 发消息到C核获取NAND FLASH的所有坏块索引列表信息 */
    ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 DRV_AGENT_NANDBBC_QRY_REQ, VOS_NULL_PTR, 0, I0_WUEPS_PID_DRV_AGENT);
    if (ret != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryNandBadBlockPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NANDBBC_READ;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryNandDevInfoPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;

    /* 发消息到C核获取NAND FLASH的型号信息 */
    ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 DRV_AGENT_NANDVER_QRY_REQ, VOS_NULL_PTR, 0, I0_WUEPS_PID_DRV_AGENT);
    if (ret != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryNandDevInfoPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NANDVER_READ;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_TestRsfrPara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "^RSFR:(VERSION,SIMLOCK)");
    return AT_OK;
}

VOS_UINT32 AT_QryPmuDieSNPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 发送跨核消息到C核, 查询RFIC IDE ID */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_MTA_PMU_DIE_SN_QRY_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (rst != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryPmuDieSNPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_PMU_DIE_SN_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

