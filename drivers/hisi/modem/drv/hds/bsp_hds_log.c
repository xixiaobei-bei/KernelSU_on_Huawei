/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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
#include <linux/string.h>
#include "bsp_hds_service.h"
#include "bsp_hds_log.h"
#include "bsp_diag.h"
#include "bsp_hds_cmd.h"
#include "bsp_hds_ind.h"
#include "bsp_hds_service.h"

#undef THIS_MODU
#define THIS_MODU mod_hds

u32 g_printlog_conn = false;
u32 g_printlog_enable = false;
u32 g_translog_conn = false;
u32 g_printlog_level = 0;

/*
 * 功能描述: 与工具连接状态设置
 * 输入参数: 无
 * 返 回 值: 成功与否标识码
 * 日    期: 2016年8月10日
 */
int bsp_hds_translog_conn(void)
{
    g_translog_conn = true;
    return BSP_OK;
}

int bsp_hds_translog_disconn(void)
{
    g_translog_conn = false;
    return BSP_OK;
}

/*
 * 功能描述: 开关状态、打印级别设置
 * 输入参数: 无
 * 返 回 值: 成功与否标识码
 * 日    期: 2016年8月10日
 */
int bsp_printlog_cfg(u32 enable, u32 level)
{
    if (level > PRINT_LEVEL_FATAL) {
        hds_printf("printlog level=%d is error\n", level);
        return BSP_ERROR;
    }
    g_printlog_level = level;

    if (enable) {
        g_printlog_enable = true;
    } else {
        g_printlog_enable = false;
    }

    hds_printf("printlog switch:%d,level:%d\n", g_printlog_enable, g_printlog_level);
    return BSP_OK;
}

/*
 * 功能描述: 与工具连接状态设置
 * 输入参数: 无
 * 返 回 值: 成功与否标识码
 * 日    期: 2016年8月10日
 */
int bsp_hds_printlog_conn(void)
{
    g_printlog_conn = true;
    return BSP_OK;
}

int bsp_hds_printlog_disconn(void)
{
    g_printlog_conn = false;
    return BSP_OK;
}

/*
 * 功能描述: 处理DIAG FW发过来的BSP LOG配置命令的处理入口
 * 输入参数: u8* pstReq, u8 *cnf, u32 *cnf_len
 * 日    期: 2016年8月10日
 */
int bsp_LogProcEntry(u8 *pstReq)
{
    diag_frame_head_s *pData = NULL;
    diag_data_head_s *pReqData = NULL;
    struct hds_print_cfg_req *plogcfg = NULL;
    diag_comm_cnf_s stLogSetCnf = { 0 };

    hds_cnf_stru stCommCnf = { 0 };
    /*lint -save -e826 */
    pData = (diag_frame_head_s *)(void *)(pstReq);
    pReqData = (diag_data_head_s *)(void *)(pData->data);
    plogcfg = (struct hds_print_cfg_req *)(void *)(pReqData->data);
    /*lint -restore +e826 */
    /* set confirm data */
    stLogSetCnf.result = (u32)bsp_printlog_cfg(plogcfg->enable, plogcfg->level);
    stLogSetCnf.auid = pReqData->auid;
    stLogSetCnf.sn = pReqData->sn;

    if (stLogSetCnf.result == BSP_OK) {
        return BSP_OK;
    } else {
        bsp_hds_cnf_common_fill(&stCommCnf, pData);
        bsp_hds_confirm(&stCommCnf, (void *)&stLogSetCnf, (unsigned int)sizeof(diag_comm_cnf_s));
        return (int)stLogSetCnf.result;
    }
}

/*
 * 功能描述: 向bsp注册cmdid和回调函数
 * 日    期: 2016年8月10日
 */
void bsp_hds_log_init(void)
{
    bsp_hds_cmd_register(DIAG_CMD_LOG_SET, (bsp_hds_func)bsp_LogProcEntry);
}

