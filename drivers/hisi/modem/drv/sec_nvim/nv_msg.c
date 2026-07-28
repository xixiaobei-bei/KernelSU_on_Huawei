/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#include <securec.h>
#include "nv_msg.h"
#include "nv_comm.h"
#include "nv_debug.h"

#define THIS_MODU mod_nv

/*lint -save -e550*//* Warning 550: (Warning -- Symbol '__dummy' (line 267) not accessed)*/

struct nv_global_msg_info_stru g_msg_ctrl;
static const u32 nv_mid_droit[NV_MID_PRI_LEVEL_NUM] = {20,1,1,1,1,1,0};
nv_cmd_req* nv_get_free_req(void) {
    nv_cmd_req *pNvCmdReq;

    pNvCmdReq = nv_malloc(sizeof(nv_cmd_req));
    if (pNvCmdReq == NULL) {
        nv_printf("malloc fail\n");
        return NULL;
    }
    (void)memset_s((void*)pNvCmdReq, sizeof(nv_cmd_req), 0, sizeof(nv_cmd_req));

    return pNvCmdReq;
}
/*****************************************************************************
  函 数 名  : bsp_nvm_acore_msg_cb
  功能描述  : 处理acore的写Nv和写文件的请求回调函数,注册到nv_cmd_req中的nv_msg_callback,
  nv_ap_task任务处理完成后会调用到该函数
  如果发出的请求消息需要等待回复，则注册本回调，否则不注册本回调；
  输出参数  : 无
  返 回 值  : 0 处理成功 非0处理失败
  输入参数  : ret处理结果 sn 消息处理对应的sn号
  调用函数  :
  被调函数  :
 *****************************************************************************/
u32 bsp_nvm_acore_msg_cb(u32 result, u32 sn)
{
    struct list_head *me;
    nv_cmd_reply *pNvCmdReply = NULL;
    unsigned long flags;
    msg_queue_t  *pMsgQueue;

    pMsgQueue = &g_msg_ctrl.req_list;

    /* nv icc process is probably access reply msg
     * add semaphore lock to prevent freeing reply msg
     */
    spin_lock_irqsave(&pMsgQueue->lock, flags);
    list_for_each(me, &g_msg_ctrl.req_list.list)
    {

        /*lint -save -e826*//*Info 826: (Info -- Suspicious pointer-to-pointer conversion (area too small))*/
        pNvCmdReply = list_entry(me, nv_cmd_reply, stList);/* [false alarm] */
        /*lint -restore*/
        if(pNvCmdReply->sn == sn)
        {
            pNvCmdReply->ret = result;
            osl_sem_up(&pNvCmdReply->sem_req);
            spin_unlock_irqrestore(&pMsgQueue->lock, flags);
            g_msg_ctrl.sync_wr_done_count++;
            return NV_OK;
        }
    }
    spin_unlock_irqrestore(&pMsgQueue->lock, flags);

    nv_printf("Invalid nv cmd SN %d in reply list", sn);
    return NV_OK;
}


/*****************************************************************************
  函 数 名  : bsp_nvm_ccore_msg_cb
  功能描述  : 处理ccore的写Nv和写文件的请求回调函数,在icc回调中注册到nv_cmd_req的nv_msg_callback,
  nv_ap_task任务处理完成后会调用到该函数
  msg_type为NV_ICC_REQ_INSTANT_FLUSH NV_ICC_REQ_FLUSH 注册本回调
  msg_type为NV_ICC_REQ_DELAY_FLUSH 不注册本回调
  输出参数  : 无
  返 回 值  : 0 处理成功 非0处理失败
  输入参数  : ret处理结果 sn 消息处理对应的sn号
  调用函数  :
  被调函数  :
 *****************************************************************************/
u32 bsp_nvm_ccore_msg_cb(u32 result, u32 sn)
{
    nv_icc_msg_t icc_cnf = {0};
    u32 ret;
    u32 chanid;

    icc_cnf.msg_type        = NV_ICC_CNF;
    icc_cnf.send_slice      = bsp_get_slice_value();
    icc_cnf.sn              = sn;
    icc_cnf.ret             = result;
    chanid = ICC_CHN_NV << 16 | NV_RECV_FUNC_AC;
    ret = nv_icc_send(chanid, (u8*)&icc_cnf, (u32)sizeof(icc_cnf));
    if(ret)
    {
        nv_printf("send icc to ccore fail, sn:0x%x  errno:0x%x", sn, ret);
    }
    //lint -save -e455
    wake_unlock(&g_nv_ctrl.wake_lock);
    g_msg_ctrl.icc_cb_reply++;
    return ret;
    //lint -restore
}

/*****************************************************************************
  函 数 名  : nv_get_task_workqueue_node
  功能描述  : 获取nv_task_ap中的消息节点信息 ，并删除原有节点
  输出参数  : 无
  返 回 值  : u32 0:成功 非零:失败
  输入参数  : node: 存放获取到得消息
  调用函数  :
  被调函数  :
 *****************************************************************************/
nv_cmd_req* nv_get_cmd_req(void)
{
    unsigned long flags;
    nv_cmd_req *pNvCmdReq;
    msg_queue_t  *pMsgQueue;

    pNvCmdReq = NULL;
    pMsgQueue = &g_msg_ctrl.high_task_list;

    /* remove from current nv high/low cmd request list */
    spin_lock_irqsave(&pMsgQueue->lock, flags);
    if (!list_empty(&pMsgQueue->list)) {
        /*lint -save -e826*//*Info 826: (Info -- Suspicious pointer-to-pointer conversion (area too small))*/
        pNvCmdReq = list_first_entry(&pMsgQueue->list, nv_cmd_req, stList);
        /*lint -restore*/
        list_del(&pNvCmdReq->stList);
    }
    spin_unlock_irqrestore(&pMsgQueue->lock, flags);

    if (pNvCmdReq)
        return pNvCmdReq;

    pMsgQueue = &g_msg_ctrl.low_task_list;
    spin_lock_irqsave(&pMsgQueue->lock, flags);
    if (!list_empty(&pMsgQueue->list)) {

        /*lint -save -e826*//*Info 826: (Info -- Suspicious pointer-to-pointer conversion (area too small))*/
        pNvCmdReq = list_first_entry(&pMsgQueue->list, nv_cmd_req, stList);
        /*lint -restore*/
        list_del(&pNvCmdReq->stList);
    }
    spin_unlock_irqrestore(&pMsgQueue->lock, flags);

    return pNvCmdReq;
}

int nv_put_cmd_req(nv_cmd_req* pNvCmdReq)
{
    if(pNvCmdReq == NULL)
    {
        nv_printf("illegal function parameter \n");
        return BSP_ERR_NV_FREE_BUF_FAIL;
    }

    nv_free(pNvCmdReq);
    return 0;
}

/*****************************************************************************
  函 数 名  : nv_send_rmsg_sync
  功能描述  : 发送同步处理消息，并等待消息处理完成后返回结果
  输入参数  : msgtype:消息类型
           itemid :  nvid
modem_id: nv的modem id
输出参数  : 无
返 回 值  : 无
 *****************************************************************************/

//u32 nv_send_rmsg(u32 msg_type, u32 req_sn, u32 itemid, u32 modem_id) {
u32 nv_send_rmsg(u32 msg_type, u32 req_sn)
{
    nv_cmd_req * pNvCmd;
    msg_queue_t  *pMsgQueue;
    unsigned long flags;
    u8 type, priority;
    nv_cb_func func;

    switch (msg_type) {
        case NV_ICC_REQ_FLUSH:
            type = NV_TASK_MSG_WRITE2FILE;
            priority = HIGH_PRIORITY_MSG_QUEUE;
            func = bsp_nvm_ccore_msg_cb;
            pMsgQueue = &g_msg_ctrl.high_task_list;
            break;

        case NV_ICC_REQ_FLUSH_RDWR_ASYNC:
            type = NV_TASK_MSG_WRITE2FILE;
            priority = HIGH_PRIORITY_MSG_QUEUE;
            func = NULL;
            pMsgQueue = &g_msg_ctrl.low_task_list;
            break;

        case NV_ICC_REQ_LOAD_CARRIER_CUST:
            type = NV_TASK_MSG_LOAD_CARRIER;
            priority = HIGH_PRIORITY_MSG_QUEUE;
            func = bsp_nvm_ccore_msg_cb;
            pMsgQueue = &g_msg_ctrl.high_task_list;
            break;

        case NV_ICC_REQ_LOAD_CARRIER_RESUM:
            type = NV_TASK_MSG_LOAD_CARRIER_COMM;
            priority = HIGH_PRIORITY_MSG_QUEUE;
            func = bsp_nvm_ccore_msg_cb;
            pMsgQueue = &g_msg_ctrl.high_task_list;
            break;

        case NV_ICC_REQ_LOAD_BACKUP:
            type = NV_TASK_MSG_LOAD_BACKUP;
            priority = HIGH_PRIORITY_MSG_QUEUE;
            func = bsp_nvm_ccore_msg_cb;
            pMsgQueue = &g_msg_ctrl.high_task_list;
            break;

        case NV_ICC_REQ_UPDATE_DEFAULT:
            type = NV_TASK_MSG_UPDATE_DEFAULT;
            priority = HIGH_PRIORITY_MSG_QUEUE;
            func = bsp_nvm_ccore_msg_cb;
            pMsgQueue = &g_msg_ctrl.high_task_list;
            break;

        case NV_ICC_REQ_UPDATE_BACKUP:
            type = NV_TASK_MSG_UPDATE_BACKUP;
            priority = HIGH_PRIORITY_MSG_QUEUE;
            func = bsp_nvm_ccore_msg_cb;
            pMsgQueue = &g_msg_ctrl.high_task_list;
            break;

       case NV_ICC_REQ_FACTORY_RESET:
            type = NV_TASK_MSG_FACTORY_RESET;
            priority = HIGH_PRIORITY_MSG_QUEUE;
            func = bsp_nvm_ccore_msg_cb;
            pMsgQueue = &g_msg_ctrl.high_task_list;
            break;

        default:
            nv_printf("invalid nv icc msg type %d \n", msg_type);
            return BSP_ERR_NV_ICC_CHAN_ERR;

    };

    /*构造消息*/
    pNvCmd = nv_get_free_req();
    if(!pNvCmd)
    {
        nv_printf("malloc fail\n");
        return BSP_ERR_NV_MALLOC_FAIL;
    }

    pNvCmd->msg_type   = type;
    pNvCmd->priority   = priority;
    pNvCmd->sn         = req_sn;
    //pNvCmd->nv_item_info.itemid    = (u16)itemid;
    //pNvCmd->nv_item_info.modem_id  = (u16)modem_id;
    pNvCmd->nv_msg_callback        = func;

    /*加入消息队列*/
    spin_lock_irqsave(&pMsgQueue->lock, flags);
    list_add_tail(&pNvCmd->stList, &pMsgQueue->list);
    spin_unlock_irqrestore(&pMsgQueue->lock, flags);

    //nv_debug_record(NV_DEBUG_REQ_REMOTE_WRITE | (itemid<<16));

    osl_sem_up(&g_nv_ctrl.task_sem);
    g_msg_ctrl.remote_wr_count++;
    /* coverity[leaked_storage] */
    return NV_OK;
}

/*****************************************************************************
 函 数 名  : nv_handle_icc_rmsg
 功能描述  : 核间终端服务程序，处理C核发送的NV消息，通过消息队列转发给ICC任务
 输入参数  : void
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
u32 nv_handle_icc_rmsg(u32 chanid, u32 len)
{
    u32 ret;
    nv_icc_msg_single_t   icc_req;

    g_msg_ctrl.icc_cb_count++;
    if (len != sizeof(nv_icc_msg_single_t)) {
        nv_printf("Illegal nv icc msg size %d, should be %d \n", len, (u32)sizeof(nv_icc_msg_single_t));
        return BSP_ERR_NV_MALLOC_FAIL;
    }

    ret = (u32)bsp_icc_read(chanid, (u8*)&icc_req, len);
    if (len != ret) {
        nv_printf("read icc fail, errno:0x%x\n", ret);
        return BSP_ERR_NV_ICC_CHAN_ERR;
    }

    //lint -save -e456 -e454
    //nv_pm_trace(icc_req);
    switch (icc_req.msg_type) {
        case NV_ICC_REQ_FLUSH:
        case NV_ICC_REQ_LOAD_BACKUP:
        case NV_ICC_REQ_LOAD_CARRIER_RESUM:
        case NV_ICC_REQ_LOAD_CARRIER_CUST:
        case NV_ICC_REQ_UPDATE_DEFAULT:
        case NV_ICC_REQ_UPDATE_BACKUP:
        case NV_ICC_REQ_FACTORY_RESET:
            wake_lock(&g_nv_ctrl.wake_lock);
            ret = nv_send_rmsg(icc_req.msg_type, icc_req.sn);
            break;

        case NV_ICC_REQ_FLUSH_RDWR_ASYNC:
            ret = nv_send_rmsg(icc_req.msg_type, icc_req.sn);
            break;

        default:
            nv_printf("invalid nv icc msg type %d \n", icc_req.msg_type);
            ret = BSP_ERR_NV_ICC_CHAN_ERR;
    }

    nv_debug_record(NV_DEBUG_RECEIVE_ICC | (icc_req.msg_type<<16));
    return ret;
    //lint -restore
}


/*****************************************************************************
  函 数 名  : nv_msg_init
  功能描述  : 初始化nv消息链表
  输入参数  : 无
  输出参数  : 无
  返 回 值     : 0 返回正确，否则返回错误码无
 *****************************************************************************/

u32 nv_msg_init(void)
{
//    size_t len;

    (void)memset_s((void*)&g_msg_ctrl, sizeof(g_msg_ctrl), 0, sizeof(struct nv_global_msg_info_stru));
    g_msg_ctrl.req_sn = 0;

    spin_lock_init(&g_msg_ctrl.high_task_list.lock);
    INIT_LIST_HEAD(&g_msg_ctrl.high_task_list.list);

    spin_lock_init(&g_msg_ctrl.low_task_list.lock);
    INIT_LIST_HEAD(&g_msg_ctrl.low_task_list.list);

    spin_lock_init(&g_msg_ctrl.req_list.lock);
    INIT_LIST_HEAD(&g_msg_ctrl.req_list.list);

    return NV_OK;
}

void nv_msg_dump(void) {
    nv_printf("icc_cb_count %d \n",  g_msg_ctrl.icc_cb_count);
    nv_printf("icc_cb_reply %d \n",  g_msg_ctrl.icc_cb_reply);
    nv_printf("sync_wr_count %d \n",      g_msg_ctrl.sync_wr_count);
    nv_printf("sync_wr_done_count %d \n", g_msg_ctrl.sync_wr_done_count);
    nv_printf("async_wr_count %d \n",      g_msg_ctrl.async_wr_count);
    nv_printf("wr_timeout_count %d \n",      g_msg_ctrl.wr_timeout_count);
    nv_printf("remote_wr_count %d \n",      g_msg_ctrl.remote_wr_count);
    nv_printf("req_sn %d \n",        g_msg_ctrl.req_sn);
    nv_printf("task_proc_count %d \n", g_nv_ctrl.task_proc_count);
    nv_printf("wake_lock_active status %d \n", wake_lock_active(&g_nv_ctrl.wake_lock));
}

/*lint -restore*/

/* vim set ts=4 sw=4 */
