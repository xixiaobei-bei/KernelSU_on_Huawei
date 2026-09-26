/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#include "soft_decode.h"
#include <linux/module.h>
#include <mdrv.h>
#include <eagleye.h>
#include <osl_sem.h>
#include <osl_thread.h>
#include <osl_malloc.h>
#include <securec.h>
#include <bsp_rfile.h>
#include <bsp_dump.h>
#include "bsp_diag.h"
#include <bsp_slice.h>
#include "hdlc.h"
#include "diag_port_manager.h"


#define THIS_MODU mod_soft_dec
spinlock_t g_soft_decode_data_rcv_spinlock;         /* 自旋锁，用来作OM数据接收的临界资源保护 */
om_hdlc_s g_soft_decode_hdlc_entity;                /* HDLC控制结构 */
soft_decode_rcv_ctrl_s g_soft_decode_rcv_ctrl_info; /* soft_decode数据接收任务控制结构 */
soft_decode_mntn_info_s g_soft_decode_mntn_info;    /* soft_decode 维测结构 */
soft_decode_dst_cb g_soft_decode_dst_cb;

/*
 * 函 数 名: soft_decode_cfg_data_recv
 * 功能描述: OM配置信息接收函数
 * 输入参数: p_buffer:数据内容
 * len:数据长度
 */
u32 soft_decode_cfg_data_recv(u8 *buffer, u32 len)
{
    u32 ret;
    unsigned long lock_level;

    spin_lock_irqsave(&g_soft_decode_data_rcv_spinlock, lock_level);
    ret = soft_decode_data_recv(buffer, len);
    spin_unlock_irqrestore(&g_soft_decode_data_rcv_spinlock, lock_level);

    return ret;
}

u32 soft_decode_data_recv(const u8 *buffer, u32 len)
{
    rw_buffer_s rw_buffer;
    u32 ret;

    diag_get_idle_buffer(g_soft_decode_rcv_ctrl_info.ring_buffer, &rw_buffer);
    if ((rw_buffer.size + rw_buffer.rb_size) < len) {
        g_soft_decode_mntn_info.rb_info.buff_no_enough++;
        return ERR_MSP_FAILURE;
    }

    ret = diag_ring_buffer_put(g_soft_decode_rcv_ctrl_info.ring_buffer, rw_buffer, buffer, (s32)len);
    COVERITY_TAINTED_SET((VOID *)&(g_soft_decode_rcv_ctrl_info.ring_buffer.buf));
    if (ret) {
        g_soft_decode_mntn_info.rb_info.rbuff_put_err++;
        return ret;
    }

    osl_sem_up(&(g_soft_decode_rcv_ctrl_info.sem_id));
    return BSP_OK;
}

/*
 * 函 数 名: soft_decode_send_to_hdlc_dec
 * 功能描述: SCM软解码数据接收函数
 * 输入参数: p_hdlc_ctrl: HDLC控制结构
 * p_data:  要发送的数据内容
 * len: 数据长度
 */
u32 soft_decode_send_to_hdlc_dec(om_hdlc_s *hdlc_ctrl, const u8 *data, u32 len)
{
    u32 i;
    u32 result;
    u8 tmp_char;

    result = ERR_MSP_FAILURE;

    for (i = 0; i < len; i++) {
        tmp_char = (u8)data[i];

        result = diag_hdlc_decap(hdlc_ctrl, tmp_char);
        if (result == HDLC_SUCC) {
            g_soft_decode_mntn_info.hdlc_decap_data.data_len += hdlc_ctrl->info_len;
            g_soft_decode_mntn_info.hdlc_decap_data.num++;

            soft_decode_data_dispatch(hdlc_ctrl);
        } else if (result == HDLC_NOT_HDLC_FRAME) {
            /* 不是完整分帧,继续HDLC解封装 */
        } else {
            g_soft_decode_mntn_info.frame_decap_data++;
        }
    }

    return BSP_OK;
}

void soft_decode_data_dispatch(om_hdlc_s *hdlc_ctrl)
{
    if (g_soft_decode_dst_cb != NULL) {
        g_soft_decode_dst_cb(hdlc_ctrl->decap_buff + sizeof(u8), hdlc_ctrl->info_len - sizeof(u8), 0, 0);
    }

    return;
}

/*
 * 函 数 名: soft_decode_hdlc_init
 * 功能描述: SCM软解码HDLC解封装初始化函数
 * 输入参数: p_hdlc:指向HDLC控制结构的指针
 */
u32 soft_decode_hdlc_init(om_hdlc_s *hdlc_ctrl)
{
    /* 申请用于HDLC解封装的缓存 */
    hdlc_ctrl->decap_buff = (u8 *)osl_malloc(SEND_BUFFER_SIZE);

    if (hdlc_ctrl->decap_buff == NULL) {
        soft_decode_error("decap_buff malloc fail!\n");
        return ERR_MSP_FAILURE;
    }

    hdlc_ctrl->decap_buff_size = SEND_BUFFER_SIZE;

    /* 初始化HDLC解封装控制上下文 */
    diag_hdlc_init(hdlc_ctrl);

    return BSP_OK;
}

/*
 * 函 数 名: soft_decode_recv_task
 * 功能描述: SCM软解码OM配置数据接收任务
 * 输入参数: ulPara1:参数1
 * ulPara2:参数2
 * ulPara3:参数3
 * ulPara4:参数4
 */
s32 soft_decode_recv_task(void *para)
{
    s32 ret;
    s32 len;
    unsigned long lock_level;
    rw_buffer_s rw_buffer;

    for (;;) {
        osl_sem_down(&(g_soft_decode_rcv_ctrl_info.sem_id));

        COVERITY_TAINTED_SET((VOID *)&(g_soft_decode_rcv_ctrl_info.ring_buffer.buf));

        diag_get_data_buffer(g_soft_decode_rcv_ctrl_info.ring_buffer, &rw_buffer);
        if ((rw_buffer.size + rw_buffer.rb_size) == 0) {
            continue;
        }
        len = rw_buffer.size + rw_buffer.rb_size;

        ret = diag_ring_buffer_get(g_soft_decode_rcv_ctrl_info.ring_buffer, rw_buffer,
            g_soft_decode_rcv_ctrl_info.buffer, len);
        if (ret) {
            spin_lock_irqsave(&g_soft_decode_data_rcv_spinlock, lock_level);
            diag_ring_buffer_flush(g_soft_decode_rcv_ctrl_info.ring_buffer);
            spin_unlock_irqrestore(&g_soft_decode_data_rcv_spinlock, lock_level);
            g_soft_decode_mntn_info.rb_info.rbuff_flush++;

            continue;
        }
        g_soft_decode_mntn_info.get_info.data_len += len;

        /* 调用HDLC解封装函数 */
        if (BSP_OK != soft_decode_send_to_hdlc_dec(&g_soft_decode_hdlc_entity, (u8 *)g_soft_decode_rcv_ctrl_info.buffer,
            (u32)len)) {
            soft_decode_error("soft_decode_send_to_hdlc_dec Fail\n");
        }
    }

    return 0;
}


/*
 * 函 数 名: SCM_SoftDecodeCfgRcvTaskInit
 * 功能描述: SCM软解码OM配置数据接收函数初始化
 */
int soft_decode_init(void)
{
    u32 result;
    OSL_TASK_ID task_id;

    g_soft_decode_rcv_ctrl_info.ring_buffer = (om_ring_s *)osl_malloc(sizeof(om_ring_s));
    if (g_soft_decode_rcv_ctrl_info.ring_buffer == NULL) {
        soft_decode_error("ring_buffer malloc fail\n");
        return BSP_ERROR;
    }
    diag_ring_buffer_flush(g_soft_decode_rcv_ctrl_info.ring_buffer);

    g_soft_decode_rcv_ctrl_info.ring_buffer->buf = (char *)osl_malloc(RING_BUFFER_SIZE);
    if (g_soft_decode_rcv_ctrl_info.ring_buffer->buf == NULL) {
        osl_free(g_soft_decode_rcv_ctrl_info.ring_buffer);
        soft_decode_error("ring_buffer->buf malloc fail\n");
        return BSP_ERROR;
    }
    g_soft_decode_rcv_ctrl_info.ring_buffer->buf_size = RING_BUFFER_SIZE;

    osl_sem_init(0, &(g_soft_decode_rcv_ctrl_info.sem_id));

    /* 注册OM配置数据接收自处理任务 */
    result = (u32)osl_task_init("soft_dec", SOFT_DECODE_TASK_PRIORITY, SOFT_DECODE_TASK_STACK_SIZE,
        (OSL_TASK_FUNC)soft_decode_recv_task, NULL, &task_id);
    if (result) {
        soft_decode_error("soft_decode_recv_task init Fail.\n");
        goto SOFT_DECODE_ERROR;
    }

    (void)memset_s(&g_soft_decode_mntn_info, sizeof(g_soft_decode_mntn_info), 0, sizeof(g_soft_decode_mntn_info));

    if (BSP_OK != soft_decode_hdlc_init(&g_soft_decode_hdlc_entity)) {
        soft_decode_error("HDLC Init Fail.\n");
        g_soft_decode_mntn_info.hdlc_init_err++;
        goto SOFT_DECODE_ERROR;
    }

    g_soft_decode_rcv_ctrl_info.buffer = (char *)osl_malloc(SEND_BUFFER_SIZE);
    if (g_soft_decode_rcv_ctrl_info.buffer == NULL) {
        soft_decode_error("ctrl_info.buffer malloc fail\n");
        goto SOFT_DECODE_ERROR;
    }

    spin_lock_init(&g_soft_decode_data_rcv_spinlock);

    cpm_logic_rcv_reg(CPM_OM_CFG_COMM, soft_decode_cfg_data_recv);
    g_soft_decode_mntn_info.cpm_reg_logic_rcv_suc++;

    soft_decode_crit("[init]soft decode init ok\n");

    return BSP_OK;

SOFT_DECODE_ERROR:
    osl_free(g_soft_decode_rcv_ctrl_info.ring_buffer->buf);
    osl_free(g_soft_decode_rcv_ctrl_info.ring_buffer);
    return BSP_ERROR;
}
#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
module_init(soft_decode_init);
#endif

u32 bsp_soft_decode_dst_proc_reg(soft_decode_dst_cb func)
{
    g_soft_decode_dst_cb = func;
    return BSP_OK;
}

void soft_decode_info_show(void)
{
    soft_decode_crit("scm_soft_decode_info_show:\n");

    soft_decode_crit("\r\nSem Creat Error %d:\r\n", g_soft_decode_mntn_info.rb_info.sem_create_err);
    soft_decode_crit("\r\nSem Give Error %d:\r\n", g_soft_decode_mntn_info.rb_info.sem_give_err);
    soft_decode_crit("\r\nRing Buffer Creat Error %d:\r\n", g_soft_decode_mntn_info.rb_info.ringbuff_create_err);
    soft_decode_crit("\r\nTask Id Error %d:\r\n", g_soft_decode_mntn_info.rb_info.task_id_err);
    soft_decode_crit("\r\nRing Buffer not Enough %d:\r\n", g_soft_decode_mntn_info.rb_info.buff_no_enough);
    soft_decode_crit("\r\nRing Buffer Flush %d:\r\n", g_soft_decode_mntn_info.rb_info.rbuff_flush);
    soft_decode_crit("\r\nRing Buffer Put Error %d:\r\n", g_soft_decode_mntn_info.rb_info.rbuff_put_err);

    soft_decode_crit("\r\nRing Buffer Put Success Times %d:\r\n", g_soft_decode_mntn_info.put_info.num);
    soft_decode_crit("\r\nRing Buffer Put Success Bytes %d:\r\n", g_soft_decode_mntn_info.put_info.data_len);

    soft_decode_crit("\r\nRing Buffer Get Success Times %d:\r\n", g_soft_decode_mntn_info.get_info.num);
    soft_decode_crit("\r\nRing Buffer Get Success Bytes %d:\r\n", g_soft_decode_mntn_info.get_info.data_len);

    soft_decode_crit("\r\nHDLC Decode Success Times %d:\r\n", g_soft_decode_mntn_info.hdlc_decap_data.num);
    soft_decode_crit("\r\nHDLC Decode Success Bytes %d:\r\n", g_soft_decode_mntn_info.hdlc_decap_data.data_len);

    soft_decode_crit("\r\nHDLC Decode Error Times %d:\r\n", g_soft_decode_mntn_info.frame_decap_data);

    soft_decode_crit("\r\nHDLC Init Error Times %d:\r\n", g_soft_decode_mntn_info.hdlc_init_err);

    soft_decode_crit("\r\nHDLC Decode Data Type Error Times %d:\r\n", g_soft_decode_mntn_info.data_type_err);

    soft_decode_crit("\r\nCPM Reg Logic Rcv Func Success Times %d:\r\n", g_soft_decode_mntn_info.cpm_reg_logic_rcv_suc);
}
EXPORT_SYMBOL(soft_decode_info_show);

