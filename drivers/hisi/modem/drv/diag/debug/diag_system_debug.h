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

#ifndef __DIAG_SYSTEM_DEBUG_H__
#define __DIAG_SYSTEM_DEBUG_H__

/*
 * 1 Include Headfile
 */
#include <product_config.h>
#include <mdrv.h>
#include <mdrv_diag_system.h>
#include <osl_types.h>
#include <osl_sem.h>
#include <bsp_slice.h>
#include <bsp_diag.h>
#include <bsp_print.h>
#include <bsp_socp.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push)
#pragma pack(4)

#undef THIS_MODU
#define THIS_MODU mod_diag

#define DIAG_DEBUG_START 0xaaaa5555
#define DIAG_DEBUG_END 0x5555aaaa

/* debug版本 V1.00 */
#define DIAG_DEBUG_VERSION 0x00010000

/* debug提示信息的长度 */
#define DIAG_DEBUG_INFO_LEN 32

/* 两次数据采集的时间间隔 */
#define DIAG_DEBUG_DEALAY 5000

/* 复杂的数据结构，存储信息前先存储信息长度，高8位是0xa5，低24位是数据长度 */
#define DIAG_DEBUG_SIZE_FLAG 0xa5000000

#define diag_crit(fmt, ...) printk(KERN_ERR "[%s]:" fmt, BSP_MOD(THIS_MODU), ##__VA_ARGS__)
#define diag_error(fmt, ...) \
    printk(KERN_ERR "[%s]:<%s %d>" fmt, BSP_MOD(THIS_MODU), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define diag_debug(fmt, ...)

/* OverFlow (Data Flow Record 溢出次数统计) */
#define DIAG_SRC_50_OVERFLOW_THR (SCM_CODER_SRC_IND_BUFFER_SIZE >> 1) /* 50%上溢阈值 */
#define DIAG_SRC_80_OVERFLOW_THR (SCM_CODER_SRC_IND_BUFFER_SIZE >> 3) /* 87.5%上溢阈值 */

/* DFR (Data Flow Record，码流录制) */
#define DIAG_DFR_NAME_MAX 8
#define DIAG_DFR_BUFFER_MAX (8 * 1024)
#define DIAG_DFR_MAGIC_NUM 0x44494147 /* DIAG */

#define DIAG_DFR_START_NUM 0xabcd4321

#define DFR_ALIGN_WITH_4BYTE(len) (((len) + 3) & (~3))

typedef struct {
    u32 magic_num;                /* 魔术字 */
    osl_sem_id sem_id;            /* 存储空间访问的互斥信号量 */
    char name[DIAG_DFR_NAME_MAX]; /* 存储码流的名字 */
    u32 cur_info;                 /* 存储空间的当前地址 */
    u32 dfr_len;                  /* 存储空间的总长度 */
    u8 *data;                     /* 存储空间的首地址 */
} diag_dfr_info_s;

typedef struct {
    u32 start;
    u32 timestamp;
} diag_dfr_header_s;

/*
 * 初步估计每个诊断命令由10-20个点能覆盖整个处理流程，
 * 1000个点能缓存50-100个诊断命令的处理流程
 */
#define DIAG_PTR_NUMBER 2000

typedef struct {
    u16 step; /* 事件枚举 */
    u16 para_mark; /* 是否携带参数, 1携带参数 0不携带参数 */
    u32 timestamp; /* 当前时间 */
    u32 para[2]; /* 携带的参数，仅支持0个或者2个，不支持其他 */ /*lint !e43*/
} diag_ptr_element_s;

typedef struct {
    u32 cur_info;
    diag_ptr_element_s ptr_element[DIAG_PTR_NUMBER];
} diag_ptr_info_s;

/* 吞吐率统计 ********************************************************************** */
#define DIAG_THRPUT_DEBUG_NUM 100 /* 缓存100个点 */

#define DIAG_THRPUT_DEBUG_LEN (5 * 32768) /* 每次统计吞吐率最少间隔5秒以上 */

typedef struct {
    unsigned int timestamp;  /* 当前时间戳时间戳 */
    unsigned int throughput; /* 吞吐率(Bytes/s) */
} diag_thrput_node_s;

typedef struct {
    unsigned int timestamp;  /* 最近一次统计的时间戳 */
    unsigned int total_len;  /* 当前已累计的字节数 */
    unsigned int max_thrput; /* 吞吐率峰值 */
    unsigned int cur_ptr;    /* 当前指针 */
    diag_thrput_node_s node[DIAG_THRPUT_DEBUG_NUM];
} diag_thrput_info_s;

extern diag_mntn_dst_info_s g_ind_dst_mntn_info;
extern u32 g_diag_throughput[EN_DIAG_THRPUT_MAX];
extern u32 g_send_usb_start_slice;
extern u32 g_send_pedev_start_slice;

/* *****************LNR******************************* */
#define DIAG_DRV_LNR_NUMBER 100

typedef enum {
    DIAG_PTR_START = 0,
    DIAG_PTR_PPM_READ,
    DIAG_PTR_CPM_RECV,
    DIAG_PTR_SCM_SOFT_DECODE,
    DIAG_PTR_SCM_SELF_TASK,
    DIAG_PTR_SCM_RCV,
    DIAG_PTR_SOFT_DEC_OK,
    DIAG_PTR_SCM_DISPATCH,
    DIAG_PTR_MSP_SERVICE_IN,
    DIAG_PTR_MSP_SERVICE_OK,
    DIAG_PTR_SERVICE_IN,
    DIAG_PTR_SERVICE_OK,
    DIAG_PTR_SRV_PACK_IN = 0x15,
    DIAG_PTR_SRV_PACK_OK = 0x70,

    /* 失败流程的点 */
    DIAG_PTR_ERR_BEGIN = 0x100,
    DIAG_PTR_PPM_READ_ERR,
    DIAG_PTR_CPM_ERR,
    DIAG_PTR_SCM_ERR,
    DIAG_PTR_MSP_SERVICE_ERR,
    DIAG_PTR_SERVICE_WAIT,
    DIAG_PTR_DIAG_SERVICE_ERR,
    DIAG_PTR_SRV_PACK_ERR,
} diag_ptr_drv_id_e;

typedef enum {
    EN_DIAG_DRV_LNR_CMDID = 0,   /* 最后N条接收到的诊断命令 */
    EN_DIAG_DRV_LNR_LOG_LOST,    /* 最后N条log丢失的记录 */
    EN_DIAG_DRV_LNR_PS_TRANS,    /* 最后N条PS透传命令的记录 */
    EN_DIAG_DRV_LNR_CCORE_MSG,   /* 最后N条C核从A核收到的消息ID的记录 */
    EN_DIAG_DRV_LNR_MESSAGE_RPT, /* 最后N条通过message模块report上报的cmdid的记录 */
    EN_DIAG_DRV_LNR_TRANS_IND,   /* 最后N条透传上报的记录 */
    EN_DIAG_DRV_LNR_INFO_MAX
} diag_lnr_id_e;

typedef enum {
    NOT_SUPPORT_QUERY = 0,         /* DIAG不支持USB类型 */
    TYPE_USB_SPEED_HIGH = 1,       /* USB 2.0 */
    TYPE_USB_SPEED_SUPER = 2,      /* USB 3.0 */
    TYPE_USB_SPEED_SUPER_PLUS = 3, /* USB 3.1 */
} diag_usb_type_e;

typedef struct {
    unsigned int cur_info;
    unsigned int param1[DIAG_DRV_LNR_NUMBER];
    unsigned int param2[DIAG_DRV_LNR_NUMBER];
} diag_lnr_info_tbl_s;

static inline void diag_system_debug_ind_dst_lost(u32 type, u32 len)
{
    g_ind_dst_mntn_info.delta_lost_times++;
    g_ind_dst_mntn_info.delta_lost_len += len;
    g_ind_dst_mntn_info.cur_fail_num[type]++;
    g_ind_dst_mntn_info.cur_fail_len[type] += len;
}

static inline void diag_system_debug_rev_socp_data(u32 len)
{
    g_ind_dst_mntn_info.delta_socp_len += len;
}

static inline void diag_system_debug_usb_len(u32 len)
{
    g_ind_dst_mntn_info.delta_usb_len += len;
}
static inline void diag_system_debug_usb_ok_len(u32 len)
{
    g_ind_dst_mntn_info.delta_usb_ok_len += len;
}

static inline void diag_system_debug_usb_free_len(u32 len)
{
    g_ind_dst_mntn_info.delta_usb_free_len += len;
}
static inline void diag_system_debug_usb_fail_len(u32 len)
{
    g_ind_dst_mntn_info.delta_usb_fail_len += len;
}

static inline void diag_system_debug_pcdev_len(u32 len)
{
    g_ind_dst_mntn_info.delat_pcdev_len += len;
}
static inline void diag_system_debug_pcdev_ok_len(u32 len)
{
    g_ind_dst_mntn_info.delat_pcdev_ok_len += len;
}

static inline void diag_system_debug_pcdev_free_len(u32 len)
{
    g_ind_dst_mntn_info.delat_pcdev_free_len += len;
}
static inline void diag_system_debug_pcdev_fail_len(u32 len)
{
    g_ind_dst_mntn_info.delat_pcdev_fail_len += len;
}

static inline void diag_system_debug_vcom_len(u32 len)
{
    g_ind_dst_mntn_info.delat_vcom_len += len;
}
static inline void diag_system_debug_vcom_fail_len(u32 len)
{
    g_ind_dst_mntn_info.delat_vcom_fail_len += len;
}
static inline void diag_system_debug_vcom_sucess_len(u32 len)
{
    g_ind_dst_mntn_info.delat_vcom_succ_len += len;
}
static inline void diag_system_debug_socket_len(u32 len)
{
    g_ind_dst_mntn_info.delat_socket_len += len;
}
static inline void diag_system_debug_socket_sucess_len(u32 len)
{
    g_ind_dst_mntn_info.delat_socket_succ_len += len;
}
static inline void diag_system_debug_socket_fail_len(u32 len)
{
    g_ind_dst_mntn_info.delat_socket_fail_len += len;
}

static inline void diag_system_debug_send_data_end(void)
{
    u32 curent_slice;
    u32 start_slice;
    u32 delta;

    curent_slice = bsp_get_slice_value();
    start_slice = bsp_get_socp_ind_dst_int_slice();
    delta = get_timer_slice_delta(start_slice, curent_slice);

    g_ind_dst_mntn_info.delat_socp_int_to_port_time += delta;
}
static inline void diag_system_debug_send_usb_end(void)
{
    u32 curent_slice;
    u32 start_slice;
    u32 delta;

    curent_slice = bsp_get_slice_value();
    start_slice = g_send_usb_start_slice;

    delta = get_timer_slice_delta(start_slice, curent_slice);
    g_ind_dst_mntn_info.delta_usb_send_time += delta;
}
static inline void diag_system_debug_send_usb_start(void)
{
    g_send_usb_start_slice = bsp_get_slice_value();
}
static inline void diag_system_debug_send_pcdev_end(void)
{
    u32 curent_slice;
    u32 start_slice;
    u32 delta;

    curent_slice = bsp_get_slice_value();
    start_slice = g_send_pedev_start_slice;

    delta = get_timer_slice_delta(start_slice, curent_slice);
    g_ind_dst_mntn_info.delta_pcdev_send_time += delta;
}
static inline void diag_system_debug_send_pcdev_start(void)
{
    g_send_pedev_start_slice = bsp_get_slice_value();
}

static inline void diag_throughput_save(diag_thrput_id_e chan, u32 bytes)
{
    g_diag_throughput[chan] += bytes;
}

void diag_debug_ptr(void);
void diag_debug_show_ptr(u32 num);
void diag_ptr(u32 type, u32 para_mark, u32 para0, u32 para1);
void diag_throughput_save(diag_thrput_id_e chan, u32 bytes);
u32 diag_get_throughput_info(diag_thrput_id_e type);
s32 diag_system_debug_event_cb(unsigned int chan_id, unsigned int event, unsigned int param);
diag_mntn_dst_info_s *diag_get_dst_mntn_info(void);
void diag_reset_dst_mntn_info(void);
u32 diag_debug_log_level(u32 level);
void diag_lnr(diag_lnr_id_e type, u32 param1, u32 param2);
void diag_show_lnr(diag_lnr_id_e type, u32 n);
u32 diag_debug_file_header(const void *file);
void diag_debug_file_tail(const void *file, char *path);
void diag_reset_src_mntn_info(void);
diag_debug_info_s *diag_get_src_mntn_info(void);
void diag_ps_buffer_overflow_record(u32 pack_len);
void diag_ps_ind_lost_record(u32 len);
void diag_ps_package_record(u32 pack_len);
u32 diag_get_usb_type(void);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of diag_Debug.h */
