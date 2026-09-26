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

#include "socp.h"
#include <securec.h>
#include <osl_thread.h>
#include "bsp_version.h"
#include "bsp_print.h"
#include "bsp_slice.h"
#include "bsp_softtimer.h"
#include "bsp_socp.h"
#include "socp_debug.h"


socp_enc_dst_info_s g_socp_enc_dst_info[SOCP_MAX_ENCDST_CHN];
spinlock_t g_socp_dst_lock;

void socp_enc_dst_spinlock_init(void)
{
    spin_lock_init(&g_socp_dst_lock);
}

/*
 * 函 数 名: bsp_socp_coder_set_dst_chan
 * 功能描述: 编码目的通道配置函数
 * 输入参数: dst_chan_id：编码目的通道ID
 * dst_attr：编码目的通道配置参数
 * 输出参数: 无
 * 返 回 值: 配置成功与否的标识码
 */
s32 bsp_socp_coder_set_dst_chan(u32 dst_chan_id, socp_dst_chan_cfg_s *dst_attr)
{
    u32 chan_id = SOCP_REAL_CHAN_ID(dst_chan_id);
    socp_enc_dst_info_s *dst_chan = &g_socp_enc_dst_info[chan_id];

    if (dst_chan->set_state == SOCP_CHN_SET) {
        socp_error("chan 0x%x can't be set twice!\n", chan_id);
        return BSP_ERR_SOCP_SET_FAIL;
    }

    dst_chan->enc_dst_buff.start = (uintptr_t)dst_attr->coder_dst_setbuf.output_start;
    dst_chan->enc_dst_buff.end = (uintptr_t)dst_attr->coder_dst_setbuf.output_end;
    dst_chan->enc_dst_buff.read = 0;
    dst_chan->enc_dst_buff.write = 0;
    dst_chan->enc_dst_buff.length = dst_chan->enc_dst_buff.end - dst_chan->enc_dst_buff.start + 1; //lint !e834

    dst_chan->buf_thrhold = dst_attr->coder_dst_setbuf.threshold;
    dst_chan->thrhold_high = dst_attr->coder_dst_setbuf.threshold;
    dst_chan->thrhold_low = dst_attr->coder_dst_setbuf.threshold;
    dst_chan->threshold = dst_attr->encdst_thrh;

    socp_hal_set_dst_buffer_addr(chan_id, (u32)dst_chan->enc_dst_buff.start,
        (u32)((u64)dst_chan->enc_dst_buff.start >> 32));
    socp_hal_set_dst_write_ptr(chan_id, 0);
    socp_hal_set_dst_read_ptr(chan_id, 0);
    socp_hal_set_dst_buffer_length(chan_id, dst_chan->enc_dst_buff.length);
    socp_hal_set_dst_arbitrate_length(chan_id, dst_chan->threshold);
    socp_hal_set_dst_threshold_length(chan_id, dst_chan->buf_thrhold);

    if (dst_attr->encdst_timeout_mode == SOCP_TIMEOUT_TFR_LONG) {
        socp_set_enc_dst_timeout(chan_id, dst_attr->encdst_timeout_mode, SOCP_TIMEOUT_TFR_LONG_MIN);
    } else {
        socp_set_enc_dst_timeout(chan_id, dst_attr->encdst_timeout_mode, SOCP_TIMEOUT_TFR_SHORT_VAL);
    }

    bsp_socp_encdst_dsm_init(dst_chan_id, SOCP_DEST_DSM_DISABLE);

    return BSP_OK;
}

void socp_set_enc_dst_timeout(u32 chan_id, u32 timeout_mode, u32 timeout)
{
    if (timeout_mode == SOCP_TIMEOUT_TFR_LONG) {
        socp_hal_set_single_chan_timeout_mode(chan_id, 1);
    } else {
        socp_hal_set_single_chan_timeout_mode(chan_id, 0);
    }

    socp_hal_set_timeout_threshold(timeout_mode, timeout);
}

/*
 * 函 数 名: socp_get_data_buffer
 * 功能描述: 获取空闲缓冲区的数据
 * 输入参数: ring_buffer     待查询的环形buffer
 * p_rw_buff 输出的环形buffer
 * 输出参数: 无
 * 返 回 值: 无
 */
void socp_get_data_buffer(socp_ring_buff_s *ring_buffer, socp_buffer_rw_s *rw_buffer)
{
    if (ring_buffer->read <= ring_buffer->write) {
        /* 写指针大于读指针，直接计算 */
        rw_buffer->buffer = (char *)(uintptr_t)((unsigned long)ring_buffer->start + (u32)ring_buffer->read);
        rw_buffer->size = (u32)(ring_buffer->write - ring_buffer->read);
        rw_buffer->rb_buffer = (char *)BSP_NULL;
        rw_buffer->rb_size = 0;
    } else {
        /* 读指针大于写指针，需要考虑回卷 */
        rw_buffer->buffer = (char *)(uintptr_t)((unsigned long)ring_buffer->start + (u32)ring_buffer->read);
        rw_buffer->size =
            (u32)((unsigned long)ring_buffer->end - ((unsigned long)ring_buffer->start + ring_buffer->read) + 1);
        rw_buffer->rb_buffer = (char *)(uintptr_t)ring_buffer->start;
        rw_buffer->rb_size = ring_buffer->write;
    }

    return;
}


/*
 * 函 数 名: socp_read_done
 * 功能描述: 更新缓冲区的读指针
 * 输入参数:  ring_buffer       待更新的环形buffer
 * size          更新的数据长度
 * 输出参数: 无
 * 返 回 值:  无
 */
void socp_read_done(socp_ring_buff_s *ring_buffer, u32 size)
{
    ring_buffer->read += size;
    if (ring_buffer->read > (u32)(ring_buffer->end - ring_buffer->start)) {
        ring_buffer->read -= ring_buffer->length;
    }
}

/*
 * 函 数 名: socp_soft_free_encdst_chan
 * 功能描述: 软释放编码目的通道
 * 输入参数: enc_dst_chan_id       编码通道号
 * 输出参数: 无
 * 返 回 值: 释放成功与否的标识码
 */
s32 bsp_socp_free_enc_dst_chan(u32 dst_chan_id)
{
    u32 chan_id = SOCP_REAL_CHAN_ID(dst_chan_id);
    socp_enc_dst_info_s *dst_chan = &g_socp_enc_dst_info[chan_id];

    socp_hal_set_dst_chan_start_stop(chan_id, 0);

    /* 写入起始地址到目的buffer起始地址寄存器 */
    socp_hal_set_dst_buffer_addr(chan_id, 0, 0);
    socp_hal_set_dst_write_ptr(chan_id, 0);
    socp_hal_set_dst_read_ptr(chan_id, 0);

    socp_hal_set_dst_threshold_length(chan_id, 0);
    socp_hal_set_dst_arbitrate_length(chan_id, 0);

    (void)memset_s(dst_chan, sizeof(socp_enc_dst_info_s), 0, sizeof(socp_enc_dst_info_s));

    return BSP_OK;
}

/*
 * 函 数 名: bsp_socp_register_read_cb
 * 功能描述: 读数据回调函数注册函数
 * 输入参数: dst_chan_id  目的通道邋ID
 * read_cb 读数据回调函数
 * 输出参数:
 * 返 回 值: 注册成功与否的标识码
 */
s32 bsp_socp_register_read_cb(u32 dst_chan_id, socp_read_cb read_cb)
{
    u32 chan_id = SOCP_REAL_CHAN_ID(dst_chan_id);
    if (g_socp_enc_dst_info[chan_id].read_cb == NULL) {
        g_socp_enc_dst_info[chan_id].read_cb = read_cb;
    }

    return BSP_OK;
}

void socp_enc_dst_read_cb(u32 chan_id)
{
    if (g_socp_enc_dst_info[chan_id].read_cb != NULL) {
        g_socp_enc_dst_info[chan_id].read_cb(chan_id);
    }

    return;
}

/*
 * 函 数 名: bsp_socp_get_read_buff
 * 功能描述: 获取读数据buffer函数
 * 输入参数: dst_chan_id  目的通道buffer
 * 输出参数: p_buffer        获取的读数据buffer
 * 返 回 值: 获取成功与否的标识码
 */
s32 bsp_socp_get_read_buff(u32 dst_chan_id, socp_buffer_rw_s *rw_buffer)
{
    u32 chan_id = SOCP_REAL_CHAN_ID(dst_chan_id);
    socp_enc_dst_info_s *dst_chan = NULL;
    socp_ring_buff_s *dst_buffer = &g_socp_enc_dst_info[chan_id].enc_dst_buff;

    dst_chan = &g_socp_enc_dst_info[chan_id];

    /* 如果deflate使能，获取deflate read buffer */
    if ((dst_chan->cps_info.bcompress == DEFLATE_IND_COMPRESS) && (dst_chan->cps_info.ops.getbuffer)) {
        return dst_chan->cps_info.ops.getbuffer(rw_buffer);
    }

    /* 根据读写指针获取buffer */
    dst_buffer->read = socp_hal_get_dst_read_ptr(chan_id);
    dst_buffer->write = socp_hal_get_dst_write_ptr(chan_id);

    socp_get_data_buffer(dst_buffer, rw_buffer);
    return BSP_OK;
}

/*
 * 函 数 名: bsp_socp_read_data_done
 * 功能描述: 读数据完成函数
 * 输入参数: dst_chan_id 目的通道ID
 * read_size 读取数据大小
 * 输出参数: 无
 * 返 回 值: 读数据成功与否的标识码
 */
s32 bsp_socp_read_data_done(u32 dst_chan_id, u32 read_size)
{
    u32 chan_id = SOCP_REAL_CHAN_ID(dst_chan_id);
    socp_enc_dst_info_s *dst_chan = &g_socp_enc_dst_info[chan_id];
    socp_ring_buff_s *dst_buffer = &g_socp_enc_dst_info[chan_id].enc_dst_buff;
    socp_buffer_rw_s rw_buff;
    unsigned long lock_flag;

    socp_mntn_record_read_done_cnt(chan_id);

    if (dst_chan_id == SOCP_CODER_DST_OM_IND) {
        socp_mntn_record_read_done_start_slice();
    }

    /* 如果deflate使能，操作deflate */
    if ((dst_chan->cps_info.bcompress == DEFLATE_IND_COMPRESS) && (dst_chan->cps_info.ops.readdone)) {
        return dst_chan->cps_info.ops.readdone(read_size);
    }

    spin_lock_irqsave(&g_socp_dst_lock, lock_flag);

    if (socp_hal_get_dst_chan_mode(chan_id) != 0) {
        spin_unlock_irqrestore(&g_socp_dst_lock, lock_flag);
        socp_error("now socp dst chan is cycle mode\n");
        return BSP_OK;
    }

    dst_buffer->write = socp_hal_get_dst_write_ptr(chan_id);
    dst_buffer->read = socp_hal_get_dst_read_ptr(chan_id);
    socp_get_data_buffer(dst_buffer, &rw_buff);
    if (rw_buff.size + rw_buff.rb_size < read_size) {
        socp_set_dst_int_open(chan_id);
        spin_unlock_irqrestore(&g_socp_dst_lock, lock_flag);
        socp_mntn_record_read_done_fail_cnt(chan_id);
        socp_error("rw_buff is not enough, read_size=0x%x\n", read_size);
        return BSP_ERR_SOCP_INVALID_PARA;
    }
    /* 设置读写指针 */
    socp_read_done(dst_buffer, read_size);
    /* 写入读指针到读指针寄存器 */
    socp_hal_set_dst_read_ptr(chan_id, dst_buffer->read);
    bsp_socp_data_send_continue(chan_id);
    spin_unlock_irqrestore(&g_socp_dst_lock, lock_flag);
    socp_mntn_record_read_done_succ_cnt(chan_id);

    if (dst_chan_id == SOCP_CODER_DST_OM_IND) {
        socp_mntn_record_read_done_end_slice();
    }

    return BSP_OK;
}
s32 socp_dst_channel_enable(u32 dst_chan_id)
{
    u32 chan_id = SOCP_REAL_CHAN_ID(dst_chan_id);

    socp_hal_set_dst_chan_start_stop(chan_id, 1);
    return BSP_OK;
}
s32 socp_dst_channel_disable(u32 dst_chan_id)
{
    u32 chan_id = SOCP_REAL_CHAN_ID(dst_chan_id);

    socp_hal_set_dst_chan_start_stop(chan_id, 0);
    return BSP_OK;
}
s32 bsp_socp_dst_trans_id_disable(u32 dst_chan_id)
{
    u32 real_chan_id = SOCP_REAL_CHAN_ID(dst_chan_id);

    socp_hal_set_dst_chan_trans_id_en(real_chan_id, 0);
    return BSP_OK;
}

void bsp_socp_set_enc_dst_threshold(u32 mode, u32 dst_chan_id)
{
    u32 chan_id = SOCP_REAL_CHAN_ID(dst_chan_id);
    socp_enc_dst_info_s *dst_chan = &g_socp_enc_dst_info[chan_id];
    u32 buf_len = socp_hal_get_dst_buffer_length(chan_id);

    if (mode == SOCP_IND_MODE_DELAY) {
        dst_chan->thrhold_high = (buf_len >> 2) * 3;
        dst_chan->thrhold_low = dst_chan->thrhold_high >> 5;
    } else {
        dst_chan->thrhold_high = dst_chan->buf_thrhold;
        dst_chan->thrhold_low = dst_chan->buf_thrhold;
    }

    socp_hal_set_dst_threshold_length(chan_id, dst_chan->thrhold_high);
    socp_crit("set encdst thrh success! 0x%x\n", dst_chan->thrhold_high);

    return;
}

s32 bsp_clear_socp_dst_buffer(u32 dst_chan_id)
{
    u32 chan_id = SOCP_REAL_CHAN_ID(dst_chan_id);
    socp_ring_buff_s *dst_buffer = &g_socp_enc_dst_info[chan_id].enc_dst_buff;
    unsigned long lock_flag;
    u32 value;

    spin_lock_irqsave(&g_socp_dst_lock, lock_flag);
    value = socp_hal_get_dst_write_ptr(chan_id);
    socp_hal_set_dst_read_ptr(chan_id, value);
    dst_buffer->read = value;
    spin_unlock_irqrestore(&g_socp_dst_lock, lock_flag);

    return BSP_OK;
}

socp_enc_dst_info_s *socp_get_dst_chan_info(u32 chan_id)
{
    return &g_socp_enc_dst_info[chan_id];
}

