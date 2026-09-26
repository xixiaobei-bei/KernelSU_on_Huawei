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
#include "bsp_hds_service.h"
#include "bsp_hds_cmd.h"
#include "bsp_diag.h"
#include <linux/list.h>
#include <linux/kernel.h>
#include "osl_malloc.h"
#include <securec.h>
/*lint -save -e826 */
#undef THIS_MODU
#define THIS_MODU mod_hds
struct bsp_hds_cmd_s g_hds_cmd_list_head;
hds_cnf_func g_hds_cnf_fn = NULL;
/*
 * 功能描述: bsp消息处理cmdid和回调函数注册接口
 * 输入参数: u32 cmdid, bsp_hds_func fn
 * 返 回 值: 0
 * 日    期: 2016年8月10日
 */
/*lint -save -e429 */
void bsp_hds_cmd_register(u32 cmdid, bsp_hds_func fn)
{
    struct bsp_hds_cmd_s *hds_list = NULL;
    struct list_head *me = NULL;
    struct bsp_hds_cmd_s *pstDiag = NULL;

    /* 判断cmdid是否重复注册 */
    list_for_each(me, &g_hds_cmd_list_head.list)
    {
        pstDiag = list_entry(me, struct bsp_hds_cmd_s, list);

        if (cmdid == pstDiag->ulCmdId) {
            hds_printf("cmdid=0x%x have registered!\n", cmdid);
            return;
        }
    }
    hds_list = osl_malloc((unsigned int)sizeof(struct bsp_hds_cmd_s));
    if (hds_list == NULL) {
        hds_printf("malloc hds_list memory failed!\n");
        return;
    }
    hds_list->ulCmdId = cmdid;
    hds_list->pFunc = fn;

    /* 将注册的cmdid和回调函数添加到链表中 */
    list_add(&hds_list->list, &g_hds_cmd_list_head.list);
}
/*lint -restore*/
/*
 * 功能描述: bsp消息处理
 * 输入参数: u32 cmdid, bsp_hds_func fn
 * 返 回 值: 0
 * 日    期: 2016年8月10日
 */
int bsp_hds_msg_proc(diag_frame_head_s *pData)
{
    s32 ret = 0;
    struct list_head *me = NULL;
    struct bsp_hds_cmd_s *pstDiag = NULL;

    if (pData->cmdid_stru.pri_4b != DIAG_FRAME_MSG_TYPE_BSP) {
        hds_printf("Rcv Error Msg_Id=0x%x\n", pData->cmd_id);
        return BSP_ERROR;
    }

    /* 遍历链表，根据注册的cmdid，跳到对应的回调中处理 */
    list_for_each(me, &g_hds_cmd_list_head.list)
    {
        pstDiag = list_entry(me, struct bsp_hds_cmd_s, list);

        if (pstDiag->ulCmdId == pData->cmd_id) {
            pstDiag->ulReserve++;
            ret = pstDiag->pFunc((u8 *)pData);
            return ret;
        }
    }

    /* 未注册此cmdid，返回对应的错误码 */
    hds_printf("cmdid=%d is not register\n", pData->cmd_id);

    return HDS_CMD_ERROR;
}

void bsp_hds_get_cmdlist(u32 *cmdlist, u32 *num)
{
    u32 i = 0;
    struct list_head *me = NULL;
    struct bsp_hds_cmd_s *pstDiag = NULL;

    if (memset_s(cmdlist, (unsigned long)(*num), 0, (unsigned long)(*num))) {
        hds_debug("er\n");
    }

    list_for_each(me, &g_hds_cmd_list_head.list)
    {
        if (i >= *num) {
            break;
        }

        pstDiag = list_entry(me, struct bsp_hds_cmd_s, list);
        cmdlist[i++] = pstDiag->ulCmdId;
    }

    *num = i;

    return;
}

void bsp_hds_cnf_register(hds_cnf_func fn)
{
    g_hds_cnf_fn = fn;
}

void bsp_hds_confirm(hds_cnf_stru *cnf, void *data, u32 len)
{
    if (g_hds_cnf_fn) {
        g_hds_cnf_fn((void *)cnf, data, len);
    } else {
        hds_printf("cnf fn isn't register, confirm failed!\n");
    }

    return;
}

void bsp_hds_cnf_common_fill(hds_cnf_stru *cnf, diag_frame_head_s *req)
{
    cnf->ulSSId = DIAG_FRAME_SSID_APP_CPU;
    cnf->ulMsgType = DIAG_FRAME_MSG_TYPE_BSP;
    cnf->ulMode = req->cmdid_stru.mode_4b;
    cnf->ulSubType = req->cmdid_stru.sec_5b;
    cnf->ulDirection = DIAG_FRAME_MT_CNF;
    cnf->ulModemid = 0;
    cnf->ulMsgId = req->cmd_id;
    cnf->ulTransId = req->srv_head.msg_trans_id;
}

/*
 * 功能描述: bsp消息回调debug信息
 * 日    期: 2016年8月10日
 */
void bsp_ShowDebugInfo(void)
{
    struct list_head *me = NULL;
    struct bsp_hds_cmd_s *pstDiag = NULL;

    list_for_each(me, &g_hds_cmd_list_head.list)
    {
        pstDiag = list_entry(me, struct bsp_hds_cmd_s, list);

        hds_printf("cmdid=0x%x\n", pstDiag->ulCmdId);
    }
}

void bsp_hds_service_init(void)
{
    INIT_LIST_HEAD(&g_hds_cmd_list_head.list);
    bsp_hds_log_init();
}
/*lint -restore +e826 */
