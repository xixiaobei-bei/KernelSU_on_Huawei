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

#ifndef _BSP_SOCP_H
#define _BSP_SOCP_H

#include "osl_common.h"
#include "mdrv_socp.h"
#include "acore_nv_id_drv.h"
#include "acore_nv_stru_drv.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_ERR_SOCP_BASE BSP_DEF_ERR(8, 0)
#define BSP_ERR_SOCP_NULL (BSP_ERR_SOCP_BASE + 0x1)
#define BSP_ERR_SOCP_NOT_INIT (BSP_ERR_SOCP_BASE + 0x2)
#define BSP_ERR_SOCP_MEM_ALLOC (BSP_ERR_SOCP_BASE + 0x3)
#define BSP_ERR_SOCP_SEM_CREATE (BSP_ERR_SOCP_BASE + 0x4)
#define BSP_ERR_SOCP_TSK_CREATE (BSP_ERR_SOCP_BASE + 0x5)
#define BSP_ERR_SOCP_INVALID_CHAN (BSP_ERR_SOCP_BASE + 0x6)
#define BSP_ERR_SOCP_INVALID_PARA (BSP_ERR_SOCP_BASE + 0x7)
#define BSP_ERR_SOCP_NO_CHAN (BSP_ERR_SOCP_BASE + 0x8)
#define BSP_ERR_SOCP_SET_FAIL (BSP_ERR_SOCP_BASE + 0x9)
#define BSP_ERR_SOCP_TIMEOUT (BSP_ERR_SOCP_BASE + 0xa)
#define BSP_ERR_SOCP_NOT_8BYTESALIGN (BSP_ERR_SOCP_BASE + 0xb)
#define BSP_ERR_SOCP_CHAN_RUNNING (BSP_ERR_SOCP_BASE + 0xc)
#define BSP_ERR_SOCP_CHAN_MODE (BSP_ERR_SOCP_BASE + 0xd)
#define BSP_ERR_SOCP_DEST_CHAN (BSP_ERR_SOCP_BASE + 0xe)
#define BSP_ERR_SOCP_DECSRC_SET (BSP_ERR_SOCP_BASE + 0xf)

#define SOCP_MAX_MEM_SIZE (200 * 1024 * 1024)
#define SOCP_MIN_MEM_SIZE (1 * 1024 * 1024)
#define SOCP_MAX_TIMEOUT 1200 /* MS */
#define SOCP_MIN_TIMEOUT 10   /* MS */
#define SOCP_RESERVED_TRUE 1
#define SOCP_RESERVED_FALSE 0

typedef u32 (*socp_compress_isr)(void);
typedef u32 (*socp_compress_event_cb)(socp_event_cb event_cb);
typedef u32 (*socp_compress_read_cb)(socp_read_cb read_cb);
typedef u32 (*socp_compress_enable)(u32 chanid);
typedef u32 (*socp_compress_disable)(u32 chanid);
typedef u32 (*socp_compress_set)(u32 chanid, socp_dst_chan_cfg_s *attr);
typedef u32 (*socp_compress_getbuffer)(socp_buffer_rw_s *pRingBuffer);
typedef u32 (*socp_compress_readdone)(u32 size);
typedef u32 (*socp_compress_clear)(u32 chanid);

typedef enum {
    SOCP_TIMEOUT_TFR_START,
    SOCP_TIMEOUT_TFR_LONG,
    SOCP_TIMEOUT_TFR_SHORT,
    SOCP_TIMEOUT_TFR_BUTT
}socp_timeout_en_e;

typedef enum {
    SOCP_DST_CHAN_NOT_CFG = 0,      /* 内存无配置，默认值 */
    SOCP_DST_CHAN_DELAY,            /* 使用预留内存 */
    SOCP_DST_CHAN_DTS,              /* 根据DTS配置申请，用于无预留buffer的场景 */
    SOCP_DST_CHAN_MIN               /* 申请的保护性内存，64KBytes */
}socp_dst_chan_cfg_type_e;

typedef struct {
    socp_compress_isr isr;
    socp_compress_event_cb register_event_cb;
    socp_compress_read_cb register_read_cb;
    socp_compress_enable enable;
    socp_compress_disable disable;
    socp_compress_set set;
    socp_compress_getbuffer getbuffer;
    socp_compress_readdone readdone;
    socp_compress_clear clear;
} socp_compress_ops_s;

struct socp_enc_dst_log_cfg {
    void*           virt_addr;       /* SOCP编码目的通道数据虚拟BUFFER、在32位系统上是4字节，在64位系统上是8字节 */
    unsigned long   phy_addr;        /* SOCP编码目的通道数据物理BUFFER地址 */
    unsigned int    buff_size;       /* SOCP编码目的通道数据BUFFER大小 */
    unsigned int    over_time;       /* NVE中设置的超时时间 */
    unsigned int    log_on_flag;     /* 底软配置buffer功能启动标志(SOCP_DST_CHAN_CFG_TYPE_ENUM) */
    unsigned int    cur_time_out;    /* SOCP编码目的通道数据传输超时时间 */
};

typedef struct {
    void*           virt_addr;      /* SOCP编码目的通道数据虚拟BUFFER、在32位系统上是4字节，在64位系统上是8字节 */
    unsigned long   phy_addr;       /* SOCP编码目的通道数据物理BUFFER地址 */
    unsigned int    buff_size;      /* SOCP编码目的通道数据BUFFER大小 */
    unsigned int    buff_useable;    /* 预留的kernel buffer是否可用的标志 */
    unsigned int    timeout;        /* SOCP编码目的通道数据传输超时时间 */
    unsigned int    init_flag;      /* 预留内存获取流程完成标志 */
}socp_rsv_mem_s;

#ifdef ENABLE_BUILD_SOCP
s32 socp_init(void);
void bsp_socp_encdst_dsm_init(u32 enc_dst_chan_id, u32 b_enable);
void bsp_socp_data_send_manager(u32 enc_dst_chan_id, u32 b_enable);
s32 bsp_socp_coder_set_src_chan(unsigned int src_chan_id, socp_src_chan_cfg_s *src_attr);
s32 bsp_socp_coder_set_dst_chan(u32 dst_chan_id, socp_dst_chan_cfg_s *dst_attr);
s32 bsp_socp_free_channel(u32 chan_id);
u32 bsp_socp_clean_encsrc_chan(unsigned int src_chan_id);
s32 bsp_socp_start(u32 src_chan_id);
s32 bsp_socp_stop(u32 src_chan_id);
void bsp_socp_set_timeout(socp_timeout_en_e time_out_en, u32 time_out);
s32 bsp_socp_set_debug(u32 dst_chan_id, u32 debug_en);
s32 bsp_socp_get_write_buff(u32 src_chan_id, socp_buffer_rw_s *p_rw_buff);
s32 bsp_socp_write_done(u32 src_chan_id, u32 write_size);
s32 bsp_socp_register_rd_cb(u32 src_chan_id, socp_rd_cb rd_cb);
s32 bsp_socp_get_rd_buffer(u32 src_chan_id, socp_buffer_rw_s *p_rw_buff);
s32 bsp_socp_read_rd_done(u32 src_chan_id, u32 rd_size);
s32 bsp_socp_register_read_cb(u32 dst_chan_id, socp_read_cb read_cb);
s32 bsp_socp_get_read_buff(u32 dst_chan_id, socp_buffer_rw_s *buffer);
s32 bsp_socp_read_data_done(u32 dst_chan_id, u32 read_size);
s32 bsp_socp_set_bbp_enable(int b_enable);
s32 bsp_socp_set_bbp_ds_mode(socp_bbp_ds_mode_e ds_mode);
socp_state_e bsp_socp_get_state(void);
void bsp_socp_set_enc_dst_threshold(u32 mode, u32 dst_chan_id);
s32 bsp_socp_set_ind_mode(socp_ind_mode_e mode);
s32 bsp_socp_get_log_ind_mode(u32 *log_ind_mode);
void bsp_socp_encsrc_chan_open(u32 src_chan_id);
void bsp_socp_encsrc_chan_close(u32 src_chan_id);
s32 bsp_socp_check_state(u32 src_chan_id);
void bsp_socp_update_bbp_ptr(u32 src_chan_id);
u32 bsp_get_socp_ind_dst_int_slice(void);
s32 bsp_clear_socp_src_buffer(u32 src_chan_id);
s32 bsp_clear_socp_dst_buffer(u32 dst_chan_id);
s32 bsp_socp_soft_free_encdst_chan(u32 enc_dst_chan_id);
void bsp_socp_mntn_enc_dst_int_info(u32 *trf_info, u32 *thrh_ovf_info);
void bsp_clear_socp_encdst_int_info(void);
s32 socp_dst_channel_enable(u32 dst_chan_id);
s32 socp_dst_channel_disable(u32 dst_chan_id);
s32 bsp_socp_dst_trans_id_disable(u32 dst_chan_id);
u32 bsp_socp_get_log_cfg(socp_encdst_buf_log_cfg_s *cfg);
void bsp_socp_set_rate_threshold(short rate_limits);
s32 bsp_socp_free_enc_dst_chan(u32 dst_chan_id);
s32 bsp_socp_set_rate_ctrl(DRV_DIAG_RATE_STRU *p_rate_ctrl);
u8 *bsp_socp_get_pdev_addr(void);
void bsp_socp_set_stress_flag(u32 flag);
#else
static inline void bsp_socp_set_stress_flag(u32 flag)
{
    return;
}

static inline u32 bsp_socp_get_stress_flag(void)
{
    return 0;
}

static inline void bsp_socp_encsrc_chan_open(u32 src_chan_id)
{
    return;
}

static inline void bsp_socp_encsrc_chan_close(u32 src_chan_id)
{
    return;
}

static inline s32 bsp_socp_check_state(u32 src_chan_id)
{
    return 0;
}

static inline s32 bsp_socp_get_write_buff(u32 src_chan_id, socp_buffer_rw_s *p_rw_buff)
{
    return 0;
}

static inline s32 bsp_socp_write_done(u32 src_chan_id, u32 write_size)
{
    return 0;
}

static inline s32 bsp_socp_coder_set_dst_chan(u32 dst_chan_id, socp_dst_chan_cfg_s *dst_attr)
{
    return 0;
}
static inline s32 bsp_socp_coder_set_src_chan(unsigned int src_chan_id, socp_src_chan_cfg_s *src_attr)
{
    return 0;
}

static inline s32 bsp_socp_start(u32 src_chan_id)
{
    return 0;
}
static inline u32 bsp_get_socp_ind_dst_int_slice(void)
{
    return 0;
}
static inline s32 bsp_socp_set_ind_mode(socp_ind_mode_e mode)
{
    return 0;
}
static inline s32 bsp_socp_get_read_buff(u32 dst_chan_id, socp_buffer_rw_s *p_buffer)
{
    return 0;
}
static inline u32 bsp_socp_get_log_cfg(socp_encdst_buf_log_cfg_s *cfg)
{
    return 0;
}
static inline s32 bsp_socp_read_data_done(u32 dst_chan_id, u32 read_size)
{
    return 0;
}
static inline s32 bsp_socp_register_read_cb(u32 dst_chan_id, socp_read_cb read_cb)
{
    return 0;
}
static inline void bsp_socp_encdst_dsm_init(u32 enc_dst_chan_id, u32 b_enable)
{
    return;
}
static inline s32 bsp_clear_socp_buff(u32 src_chan_id)
{
    return 0;
}
static inline s32 bsp_socp_soft_free_encdst_chan(u32 enc_dst_chan_id)
{
    return 0;
}
static inline void bsp_socp_mntn_enc_dst_int_info(u32 *trf_info, u32 *thrh_ovf_info)
{
    return;
}

static inline void bsp_clear_socp_encdst_int_info(void)
{
    return;
}

static inline s32 socp_dst_channel_enable(u32 dst_chan_id)
{
    return 0;
}

static inline s32 socp_dst_channel_disable(u32 dst_chan_id)
{
    return 0;
}

static inline s32 bsp_socp_set_rate_ctrl(DRV_DIAG_RATE_STRU *p_rate_ctrl)
{
    return 0;
}
static inline s32 bsp_socp_dst_trans_id_disable(u32 dst_chan_id)
{
    return 0;
}
static inline void bsp_socp_set_rate_threshold(short rate_limits)
{
    return;
}
static inline s32 bsp_socp_get_ind_chan_mode(void)
{
    return 0;
}
#endif

s32 bsp_socp_set_log_cfg_mode(socp_ind_mode_e mode);
s32 bsp_socp_get_log_cfg_state(u32 *Cfg_ind_mode);
s32 bsp_socp_get_ind_chan_mode(void);

#ifdef __cplusplus
}
#endif

#endif /* end of _BSP_SOCP_H */
