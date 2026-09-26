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
 */

#include <securec.h>
#include <bsp_dump.h>
#include "nv_msg.h"
#include "nv_comm.h"
#include "nv_debug.h"

#define THIS_MODU mod_nv

#define NV_TASK_DELAY 50
static u32 nv_icc_send(u32 chanid, u8 *pdata, u32 len)
{
    s32 ret;
    u32 fun_id = chanid & 0xffff; /* get fun id */
    u32 core_type;
    u32 i;

    if (fun_id == NV_RECV_FUNC_AC) {
        core_type = ICC_CPU_MODEM;
    } else {
        return BSP_ERR_NV_INVALID_PARAM;
    }
    nv_debug_trace(pdata, len);
    for (i = 0; i < NV_ICC_SEND_COUNT; i++) {
        ret = bsp_icc_send(core_type, chanid, pdata, len);
        if (ret == ICC_INVALID_NO_FIFO_SPACE) {
            nv_taskdelay(NV_TASK_DELAY);
            continue;
        } else if (ret != (s32)len) {
            nv_printf("[%s]:ret :0x%x,len 0x%x\n", __FUNCTION__, ret, len);
            return BSP_ERR_NV_ICC_CHAN_ERR;
        } else {
            nv_debug_record(NV_DEBUG_SEND_ICC | (((nv_icc_msg_t *)(uintptr_t)pdata)->msg_type << 0x10));
            return NV_OK;
        }
    }
    system_error(DRV_ERRNO_NV_ICC_FIFO_FULL, core_type, chanid, (char *)pdata, len);
    return NV_ERROR;
}

u32 bsp_nvm_ccore_msg_cb(u32 result, u32 sn)
{
    nv_icc_msg_t icc_cnf = {0};
    u32 ret;
    u32 chanid;
    struct nv_global_ctrl_info_stru *nv_ctrl = nv_get_ctrl_info();

    icc_cnf.msg_type = NV_ICC_CNF;
    icc_cnf.send_slice = bsp_get_slice_value();
    icc_cnf.sn = sn;
    icc_cnf.ret = result;
    chanid = (ICC_CHN_NV << 0x10) | NV_RECV_FUNC_AC;
    ret = nv_icc_send(chanid, (u8 *)&icc_cnf, (u32)sizeof(icc_cnf));
    if (ret) {
        nv_printf("send icc to ccore fail, sn:0x%x  errno:0x%x", sn, ret);
    }
    //lint -save -e455
    __pm_relax(&(nv_ctrl->wake_lock));
    g_msg_ctrl.icc_cb_reply++;
    return ret;
    //lint -restore
}

static u32 nv_handle_icc_rmsg(u32 chanid, u32 len)
{
    u32 ret;
    nv_icc_msg_t icc_req;
    struct nv_global_ctrl_info_stru *nv_ctrl = nv_get_ctrl_info();

    g_msg_ctrl.icc_cb_count++;
    if (len != sizeof(nv_icc_msg_t)) {
        nv_printf("Illegal nv icc msg size %d, should be %d \n", len, (u32)sizeof(nv_icc_msg_t));
        return BSP_ERR_NV_MALLOC_FAIL;
    }

    ret = (u32)bsp_icc_read(chanid, (u8 *)&icc_req, len);
    if (len != ret) {
        nv_printf("read icc fail, errno:0x%x\n", ret);
        return BSP_ERR_NV_ICC_CHAN_ERR;
    }

    //lint -save -e456 -e454
    switch (icc_req.msg_type) {
        case NV_ICC_REQ_FLUSH:
        case NV_ICC_REQ_FLUSH_RWNV:
        case NV_ICC_REQ_LOAD_BACKUP:
        case NV_ICC_REQ_LOAD_CARRIER_RESUM:
        case NV_ICC_REQ_LOAD_CARRIER_CUST:
        case NV_ICC_REQ_UPDATE_DEFAULT:
        case NV_ICC_REQ_UPDATE_BACKUP:
        case NV_ICC_REQ_FACTORY_RESET:
        case NV_ICC_REQ_UPGRADE_FLUSH:
            __pm_stay_awake(&(nv_ctrl->wake_lock));
            ret = nv_send_rmsg(icc_req.msg_type, icc_req.sn);
            break;

        case NV_ICC_REQ_FLUSH_RDWR_ASYNC:
            ret = nv_send_rmsg(icc_req.msg_type, icc_req.sn);
            break;

        default:
            nv_printf("invalid nv icc msg type %d \n", icc_req.msg_type);
            ret = BSP_ERR_NV_ICC_CHAN_ERR;
    }

    nv_debug_record(NV_DEBUG_RECEIVE_ICC | (icc_req.msg_type << 0x10));
    return ret;
    //lint -restore
}

static s32 nv_icc_msg_proc(u32 chanid, u32 len, void *pdata)
{
    return (s32)nv_handle_icc_rmsg(chanid, len);
}

void nv_modify_print_sw(u32 arg)
{
    struct nv_global_ctrl_info_stru *nv_ctrl = nv_get_ctrl_info();
    nv_ctrl->debug_sw = arg;
}

s32 nv_modify_pm_sw(s32 arg)
{
    struct nv_global_ctrl_info_stru *nv_ctrl = nv_get_ctrl_info();
    nv_ctrl->pmSw = (bool)arg;
    return 0;
}

u32 nv_cpmsg_chan_init(void)
{
    u32 chanid;

    chanid = (ICC_CHN_NV << 0x10) | NV_RECV_FUNC_AC;

    /* reg icc debug proc */
    (void)bsp_icc_debug_register(chanid, nv_modify_pm_sw, (s32)true);

    return (u32)bsp_icc_event_register(chanid, nv_icc_msg_proc, NULL, NULL, NULL);
}
