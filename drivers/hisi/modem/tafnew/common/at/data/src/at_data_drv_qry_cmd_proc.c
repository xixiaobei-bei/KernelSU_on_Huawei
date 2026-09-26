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

#include "at_data_drv_qry_cmd_proc.h"

#include "securec.h"
#include "at_mdrv_interface.h"
#include "at_cmd_proc.h"
#include "at_check_func.h"
#include "at_parse_cmd.h"
#include "at_phy_interface.h"
#include "at_data_proc.h"
#include "at_cmd_msg_proc.h"
#include "at_snd_msg.h"
#include "at_input_proc.h"
#include "at_event_report.h"
#include "at_device_cmd.h"
#include "at_taf_agent_interface.h"
#include "app_vc_api.h"
#include "taf_oam_interface.h"
#include "taf_iface_api.h"
#include "at_lte_eventreport.h"

#if (FEATURE_LTE == FEATURE_ON)
#include "mdrv_nvim.h"
#include "at_common.h"
#include "at_lte_common.h"
#endif

#include "at_msg_print.h"
#include "ps_common_def.h"
#include "ps_lib.h"
#include "product_config.h"

#if (VOS_OS_VER == VOS_WIN32)
#include "NasStub.h"
#endif




/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_DATA_DRV_QRY_CMD_PROC_C

TAF_UINT32 At_QryDialModePara(TAF_UINT8 indexNum)
{
    VOS_UINT8  dialmode = 0;
    VOS_UINT8  cdcSpec = 0;
    VOS_UINT32 rst;

    /* 调用底软接口，根据当前的设备形态来获取当前支持的拨号方式 */
    /* ucRst:       VOS_OK/VOS_ERR */
    /* ucDialmode:  0 - 使用Modem拨号; 1 - 使用NDIS拨号; 2 - Modem和NDIS共存 */
    /*
     * ucCdcSpec:   0 - Modem/NDIS都符合CDC规范; 1 - Modem符合CDC规范;
     * 2 - NDIS符合CDC规范;         3 - Modem/NDIS都符合CDC规范
     */

    rst = (VOS_UINT32)DRV_GET_DIAG_MODE_VALUE(&dialmode, &cdcSpec);

    if (rst != VOS_OK) {
        AT_ERR_LOG("At_QryDialModePara:ERROR:DRV_GET_DIAG_MODE_VALUE Return Err!");
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s:%d,%d", g_parseContext[indexNum].cmdElement->cmdName, dialmode, cdcSpec);
    return AT_OK;
}

