/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 */

#include "socp.h"

#include "securec.h"
#include "mdrv_socp.h"
#include "bsp_socp.h"
#include "socp_debug.h"


/*lint --e{124}*/

socp_enc_src_info_s g_socp_enc_src_info;
static void socp_coder_set_src_info(u32 chan_id, socp_enc_src_chan_s *src_chan, socp_src_chan_cfg_s *src_attr);

void socp_enc_src_read_cb(u32 chan_id)
{
    if (g_socp_enc_src_info.enc_src_chan[chan_id].rd_cb != NULL) {
        g_socp_enc_src_info.enc_src_chan[chan_id].rd_cb(chan_id);
    }
}


static void socp_coder_set_src_info(u32 chan_id, socp_enc_src_chan_s *src_chan, socp_src_chan_cfg_s *src_attr)
{
    u32 buf_len =
        (uintptr_t)src_attr->coder_src_setbuf.input_end - (uintptr_t)src_attr->coder_src_setbuf.input_start + 1;
    u32 rd_buf_len =
        (uintptr_t)src_attr->coder_src_setbuf.rd_input_end - (uintptr_t)src_attr->coder_src_setbuf.rd_input_start + 1;

    src_chan->chan_mode = src_attr->mode;
    src_chan->priority = src_attr->priority;
    src_chan->data_type = src_attr->data_type;
    src_chan->data_type_en = src_attr->data_type_en;
    src_chan->debug_en = src_attr->debug_en;
    src_chan->dst_chan_id = src_attr->dest_chan_id;
    src_chan->bypass_en = src_attr->bypass_en;
    src_chan->trans_id_en = src_attr->trans_id_en;
    src_chan->ptr_img_en = src_attr->rptr_img_en;
    src_chan->read_ptr_img_phy_addr = src_attr->read_ptr_img_phy_addr;
    src_chan->read_ptr_img_vir_addr = src_attr->read_ptr_img_vir_addr;
    src_chan->enc_src_buff.start = (uintptr_t)src_attr->coder_src_setbuf.input_start;
    src_chan->enc_src_buff.end = (uintptr_t)src_attr->coder_src_setbuf.input_end;
    src_chan->enc_src_buff.write = 0;
    src_chan->enc_src_buff.read = 0;
    src_chan->enc_src_buff.length = buf_len;
    src_chan->enc_src_buff.idle_size = 0;

    if (src_attr->mode == SOCP_ENCSRC_CHNMODE_LIST) {
        src_chan->rd_buff.start = (uintptr_t)src_attr->coder_src_setbuf.rd_input_start;
        src_chan->rd_buff.end = (uintptr_t)src_attr->coder_src_setbuf.rd_input_end;
        src_chan->rd_buff.write = 0;
        src_chan->rd_buff.read = 0;
        src_chan->rd_buff.length = rd_buf_len;
        src_chan->rd_threshold = src_attr->coder_src_setbuf.rd_threshold;
    }

    src_chan->alloc_state = SOCP_CHN_ALLOCATED;
}


/*
 * 函 数 名: bsp_socp_coder_set_src_chan
 * 功能描述: 编码源通道配置函数
 * 输入参数: src_attr 编码源通道配置参数
 * src_chan_id  编码源通道ID
 * 输出参数: 无
 * 返 回 值: 申请及配置成功与否的标识码
 */
s32 bsp_socp_coder_set_src_chan(unsigned int src_chan_id, socp_src_chan_cfg_s *src_attr)
{
    socp_enc_src_chan_s *src_chan = &g_socp_enc_src_info.enc_src_chan[src_chan_id];

    socp_hal_src_chan_init(src_chan_id, src_attr);
    if (src_attr->mode == SOCP_ENCSRC_CHNMODE_LIST) {
        socp_rd_int_mask_proc(src_chan_id, SOCP_RD_MASK_CLEAR);
    }

    socp_hal_set_rate_ctrl_group(src_chan_id, src_attr->chan_group);

    socp_coder_set_src_info(src_chan_id, src_chan, src_attr);
    return BSP_OK;
}

/*
 * 函 数 名: bsp_socp_free_channel
 * 功能描述: 通道释放函数
 * 输入参数: chan_id 编解码通道指针
 * 输出参数: 无
 * 返 回 值: 释放成功与否的标识码
 */
s32 bsp_socp_free_channel(u32 chan_id)
{
    socp_enc_src_chan_s *src_chan = &g_socp_enc_src_info.enc_src_chan[chan_id];

    socp_hal_set_src_chan_start_stop(chan_id, 0); /* 关闭当前通道 */

    socp_hal_set_src_buffer_addr(chan_id, 0, 0);
    socp_hal_set_src_write_ptr(chan_id, 0);
    socp_hal_set_src_read_ptr(chan_id, 0);
    socp_hal_set_src_buffer_length(chan_id, 0);

    if (src_chan->chan_mode == SOCP_ENCSRC_CHNMODE_LIST) {
        socp_hal_set_src_rd_buffer_addr(chan_id, 0, 0);
        socp_hal_set_src_rd_write_ptr(chan_id, 0);
        socp_hal_set_src_rd_read_ptr(chan_id, 0);
        socp_hal_set_src_rd_buffer_length(chan_id, 0);
    }

    if (src_chan->ptr_img_en) { /* 读指针镜像使能，需要将读指针镜像寄存器清0 */
        socp_hal_set_rptr_img_addr(chan_id, 0, 0);
    }

    socp_hal_clear_src_chan_cfg(chan_id);

    (void)memset_s(src_chan, sizeof(socp_enc_src_chan_s), 0, sizeof(socp_enc_src_chan_s));

    return BSP_OK;
}

/*
 * 函 数 名: bsp_socp_start
 * 功能描述: 编码或者解码启动函数
 * 输入参数: src_chan_id 通道ID
 * 返 回 值: 启动成功与否的标识码
 */
s32 bsp_socp_start(u32 src_chan_id)
{
    socp_enc_src_chan_s *src_chan = &g_socp_enc_src_info.enc_src_chan[src_chan_id];

    if (src_chan->chan_en == SOCP_CHN_ENABLE) {
        return BSP_OK;
    }

    socp_clear_enc_src_int_state(src_chan_id, src_chan->chan_mode);

    socp_hal_clear_rate_ctrl_cnt(src_chan_id);

    socp_hal_set_src_chan_start_stop(src_chan_id, 1); /* 源端通道使能 */

    src_chan->chan_en = SOCP_CHN_ENABLE;
    return BSP_OK;
}

/*
 * 函 数 名: bsp_socp_stop
 * 功能描述: 编码或者解码停止函数
 * 输入参数: src_chan_id 通道ID
 * 返 回 值: 停止成功与否的标识码
 */
s32 bsp_socp_stop(u32 chan_id)
{
    socp_enc_src_chan_s *src_chan = &g_socp_enc_src_info.enc_src_chan[chan_id];

    /* 清除原始rd中断 */
    if (src_chan->chan_mode == SOCP_ENCSRC_CHNMODE_LIST) {
        socp_rd_int_mask_proc(chan_id, SOCP_RD_MASK_SET);
    }

    /* 去使能源通道 */
    socp_hal_set_src_chan_start_stop(chan_id, 0); /* 源端通道使能 */

    src_chan->chan_en = SOCP_CHN_DISABLE;
    return BSP_OK;
}

/*
 * 函 数 名: bsp_socp_get_write_buff
 * 功能描述: 上层获取写数据buffer函数
 * 输入参数: src_chan_id 源通道ID
 * 输出参数: p_buff 获取的buffer
 * 返 回 值: 获取成功与否的标识码
 */
s32 bsp_socp_get_write_buff(u32 chan_id, socp_buffer_rw_s *rw_buff)
{
    socp_enc_src_chan_s *src_chan = &g_socp_enc_src_info.enc_src_chan[chan_id];

    if (src_chan->ptr_img_en) { /* 读指针镜像使能，需要读取读指针镜像地址中的读指针 */
        src_chan->enc_src_buff.read = *((u32 *)(uintptr_t)src_chan->read_ptr_img_vir_addr);
    } else { /* 读指针镜像不使能，直接从寄存器中读取读指针 */
        src_chan->enc_src_buff.read = socp_hal_get_src_read_ptr(chan_id);
    }

    if (src_chan->enc_src_buff.write < src_chan->enc_src_buff.read) {
        /* 读指针大于写指针，直接计算 */
        rw_buff->buffer = (char *)(uintptr_t)(src_chan->enc_src_buff.start + src_chan->enc_src_buff.write);
        rw_buff->size = src_chan->enc_src_buff.read - src_chan->enc_src_buff.write - 1;
        rw_buff->rb_buffer = NULL;
        rw_buff->rb_size = 0;
    } else {
        /* 写指针大于读指针，需要考虑回卷 */
        rw_buff->buffer = (char *)(uintptr_t)(src_chan->enc_src_buff.start + src_chan->enc_src_buff.write);
        rw_buff->size =
            (u32)(src_chan->enc_src_buff.end - (src_chan->enc_src_buff.start + src_chan->enc_src_buff.write) + 1);
        rw_buff->rb_buffer = (char *)(uintptr_t)(src_chan->enc_src_buff.start);
        rw_buff->rb_size = src_chan->enc_src_buff.read;
    }

    return BSP_OK;
}

/*
 * 函 数 名  : bsp_socp_write_done
 * 功能描述  : 写数据完成函数
 * 输入参数  : src_chan_id    源通道ID
 * write_size 写入数据的长度
 * 返 回 值  : 操作完成与否的标识码
 */
s32 bsp_socp_write_done(u32 src_chan_id, u32 write_size)
{
    socp_ring_buff_s *src_buffer = &g_socp_enc_src_info.enc_src_chan[src_chan_id].enc_src_buff;
    u32 cur_size;

    cur_size = src_buffer->end - (src_buffer->start + src_buffer->write) + 1;
    if (cur_size > write_size) {
        src_buffer->write += write_size;
    } else {
        src_buffer->write = write_size - cur_size;
    }

    /* 设置写指针 */
    socp_hal_set_src_write_ptr(src_chan_id, src_buffer->write);

    return BSP_OK;
}

/*
 * 函 数 名  : bsp_socp_register_rd_cb
 * 功能描述  : RDbuffer回调函数注册函数
 * 输入参数  : src_chan_id    ：源通道ID
 * rd_cb：RDbuffer完成回调函数
 * 输出参数  : 无
 * 返 回 值  : 注册成功与否的标识码
 */
s32 bsp_socp_register_rd_cb(u32 src_chan_id, socp_rd_cb rd_cb)
{
    socp_enc_src_chan_s *src_chan = &g_socp_enc_src_info.enc_src_chan[src_chan_id];

    if (src_chan->chan_mode == SOCP_ENCSRC_CHNMODE_LIST) {
        src_chan->rd_cb = rd_cb;
    }

    return BSP_OK;
}

/*
 * 函 数 名: bsp_socp_get_rd_buffer
 * 功能描述: 获取RDbuffer函数
 * 输入参数: src_chan_id    源通道ID
 * 输出参数: p_buff           获取的RDbuffer
 * 返 回 值: 获取成功与否的标识码
 */
s32 bsp_socp_get_rd_buffer(u32 src_chan_id, socp_buffer_rw_s *p_buff)
{
    socp_enc_src_chan_s *src_chan = &g_socp_enc_src_info.enc_src_chan[src_chan_id];

    /* 判断参数有效性 */
    if (p_buff == NULL) {
        socp_error("the parameter is NULL\n");
        return BSP_ERROR;
    }

    src_chan->rd_buff.write = socp_hal_get_src_rd_write_ptr(src_chan_id);

    if (src_chan->rd_buff.read <= src_chan->rd_buff.write) {
        p_buff->buffer = (char *)(uintptr_t)(src_chan->rd_buff.start + src_chan->rd_buff.read);
        p_buff->size = (u32)(src_chan->rd_buff.write - src_chan->rd_buff.read);
        p_buff->rb_buffer = NULL;
        p_buff->rb_size = 0;
    } else {
        p_buff->buffer = (char *)(uintptr_t)(src_chan->rd_buff.start + src_chan->rd_buff.read);
        p_buff->size = (uintptr_t)(src_chan->rd_buff.end - (src_chan->rd_buff.start + src_chan->rd_buff.read) + 1);
        p_buff->rb_buffer = (char *)(uintptr_t)(src_chan->rd_buff.start);
        p_buff->rb_size = src_chan->rd_buff.write;
    }

    return BSP_OK;
}

/*
 * 函 数 名: bsp_socp_read_rd_done
 * 功能描述: 读取RDbuffer数据完成函数
 * 输入参数: src_chan_id   源通道ID
 * rd_size      读取的RDbuffer数据长度
 * 输出参数:
 * 返 回 值: 读取成功与否的标识码
 */
s32 bsp_socp_read_rd_done(u32 src_chan_id, u32 rd_size)
{
    socp_ring_buff_s *rd_buffer = &g_socp_enc_src_info.enc_src_chan[src_chan_id].rd_buff;
    socp_buffer_rw_s rw_buff;
    s32 ret;

    ret = bsp_socp_get_rd_buffer(src_chan_id, &rw_buff);
    if (ret) {
        return ret;
    }

    if (rw_buff.size + rw_buff.rb_size < rd_size) {
        socp_error("rw_buffer is not enough, rd_size=0x%x\n", rd_size);
        socp_rd_int_mask_proc(src_chan_id, SOCP_RD_MASK_CLEAR);
        return BSP_ERROR;
    }

    rd_buffer->read += rd_size;
    if (rd_buffer->read > rd_buffer->end - rd_buffer->start) {
        rd_buffer->read -= rd_buffer->length;
    }

    socp_hal_set_src_rd_read_ptr(src_chan_id, rd_buffer->read);
    return BSP_OK;
}

/*
 * 函 数 名: bsp_socp_get_state
 * 功能描述: 获取SOCP状态
 * 返 回 值: SOCP_IDLE    空闲
 * SOCP_BUSY    忙碌
 */
unsigned int bsp_socp_get_state(void)
{
    if (socp_hal_get_src_state()) {
        return SOCP_BUSY;
    }

    return SOCP_IDLE;
}

/*
 * 函 数 名:  bsp_socp_encsrc_chan_open
 * 功能描述: 打开SOCP编码源通道开关
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: void
 */
void bsp_socp_encsrc_chan_open(u32 src_chan_id)
{
    socp_hal_set_src_chan_start_stop(src_chan_id, 1);

    while (socp_hal_get_src_start_stop_state(src_chan_id) == 0) {
        ;
    }
}

/*
 * 函 数 名: bsp_socp_encsrc_chan_close
 * 功能描述: 关闭socp编码源通道开关，并且等待socp空闲后退出
 * 该接口用于防止socp访问下电区域
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: void
 */
void bsp_socp_encsrc_chan_close(u32 src_chan_id)
{
    socp_hal_set_src_chan_start_stop(src_chan_id, 0);

    while (socp_hal_get_single_src_state(src_chan_id) || socp_hal_get_src_start_stop_state(src_chan_id)) {
        ;
    }
}

s32 bsp_clear_socp_src_buffer(u32 src_chan_id)
{
    u32 value;
    socp_enc_src_chan_s *src_chan = &g_socp_enc_src_info.enc_src_chan[src_chan_id];

    while (socp_hal_get_single_src_state(src_chan_id)) {
        ;
    }

    value = socp_hal_get_src_read_ptr(src_chan_id);
    socp_hal_set_src_write_ptr(src_chan_id, value);
    src_chan->enc_src_buff.write = value;

    return BSP_OK;
}

void bsp_socp_set_rate_threshold(short rate_limits)
{
    socp_hal_set_rate_ctrl_threshold(rate_limits);
}
/*
 * 函 数 名: bsp_socp_set_rate_ctrl
 * 功能描述: 该此接口用于设置SOCP流控配置
 * 输入参数: rate_ctrl:流控配置
 * 输出参数: 无。
 * 返 回 值: 无
 */
s32 bsp_socp_set_rate_ctrl(DRV_DIAG_RATE_STRU *rate_ctrl)
{
    if (rate_ctrl->chan_enable == 0) {
        socp_crit("socp rate ctrl closed\n");
        socp_hal_set_rate_ctrl_cfg(0, 0);
        return BSP_OK;
    }
    /* 流控周期控制 */
    socp_hal_set_rate_ctrl_peroid(SOCP_RATE_PERIOD);

    /* 流控阈值配置 */
    bsp_socp_set_rate_threshold(rate_ctrl->rate_limits);

    /* 流控使能 */
    socp_hal_set_rate_ctrl_cfg(rate_ctrl->chan_rate_bits_l, rate_ctrl->chan_rate_bits_h);

    return BSP_OK;
}


