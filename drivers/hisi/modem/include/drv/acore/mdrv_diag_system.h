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
#include "mdrv_socp.h"
#include "mdrv_diag_system_common.h"

#ifndef __MDRV_DIAG_SYSTEM_H__
#define __MDRV_DIAG_SYSTEM_H__
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef DIAG_SYSTEM_5G
#define SCM_CODER_SRC_LOM_CNF          (SOCP_CODER_SRC_ACPU_CNF)
#define SCM_CODER_SRC_LOM_IND          (SOCP_CODER_SRC_ACPU_IND)
#else
#define SCM_CODER_SRC_LOM_CNF          (SOCP_CODER_SRC_LOM_CNF1)
#define SCM_CODER_SRC_LOM_IND          (SOCP_CODER_SRC_LOM_IND1)
#endif

#define DIAG_CODER_SRC_CNF_PADDR       (DIAG_CODER_SRC_ACORE_CNF_PADDR)
#define DIAG_CODER_SRC_CNF_LENGTH      (DIAG_CODER_SRC_ACORE_CNF_LENGTH)

#define DIAG_CODER_SRC_IND_PADDR       (DIAG_CODER_SRC_ACORE_IND_PADDR)
#define DIAG_CODER_SRC_IND_LENGTH      (DIAG_CODER_SRC_ACORE_IND_LENGTH)

#define OM_PORT_HANDLE_NUM      (7) /* OM_PORT_HANDLE_NUM = OM_PORT_HANDLE_BUTT */

typedef enum {
    CPM_AT_COMM,
    CPM_OM_IND_COMM,        /* OM可维可测端口 */
    CPM_OM_CFG_COMM,        /* OM配置端口 */
    CPM_DIAG_COMM,
    CPM_PCVOICE_COMM,
    CPM_COMM_BUTT
} mdrv_cpm_logic_port_type_e;

typedef enum {
    CONN_STATE_CONNECT = 1,
    CONN_STATE_DISCONNECT = 2,
    CONN_STATE_BUTT
} diag_conn_state_e;

typedef enum {
    EN_DIAG_DST_LOST_BRANCH,            /* 当前时间段分支失败次数 */
    EN_DIAG_DST_LOST_CPMWR,             /* 当前时间段CPM写失败次数 */
    EN_DIAG_DST_LOST_CPMCB,             /* 当前时间段CPM写回调失败次数 */
    EN_DIAG_DST_LOST_MAX
} diag_lost_dst_ind_e;

typedef struct {
    unsigned int usb_write_num1;
    unsigned int usb_write_num2;
    unsigned int usb_write_cb_num;
    unsigned int usb_write_max_time;
    unsigned int usb_in_num;
    unsigned int usb_in_time;
    unsigned int usb_out_num;
    unsigned int usb_out_time;
    unsigned int usb_state_err_num;
    unsigned int usb_state_err_time;
    unsigned int usb_write_err_time;
    unsigned int usb_write_err_num;
    unsigned int usb_write_err_val;
    unsigned int usb_write_err_len;
    unsigned int usb_open_num;
    unsigned int usb_open_slice;
    unsigned int usb_open_ok_num;
    unsigned int usb_open_ok_slice;
    unsigned int usb_open_ok_num2;
    unsigned int usb_open_ok_slice2;
    unsigned int usb_close_num;
    unsigned int usb_close_slice;
    unsigned int usb_close_ok_num;
    unsigned int usb_close_ok_slice;
    unsigned int usb_ind_sync_fail_num;
    unsigned int usb_ind_sync_fail_slice;
    unsigned int usb_cnf_sync_fail_num;
    unsigned int usb_cnf_sync_fail_slice;
    unsigned int usb_udi_handle_err;
    unsigned int usb_get_rd_data_err;
    unsigned int usb_udi_rcv_null_err;
    unsigned int usb_udi_read_buf_free_err;
    unsigned int usb_rcv_pkt_num;
    unsigned int usb_rcv_pkt_len;
} mdrv_ppm_port_info_s;

typedef struct {
    unsigned int socp_cb_para_err_num;
    unsigned int usb_send_num;
    unsigned int usb_send_len;
    unsigned int usb_send_real_len;
    unsigned int usb_send_err_num;
    unsigned int usb_send_err_len;
    unsigned int usb_send_cb_abnormal_num;
    unsigned int usb_send_cb_abnormal_len;

    unsigned int ppm_get_virt_err;
    unsigned int ppm_get_virt_send_len;
    unsigned int ppm_discard_num;
    unsigned int ppm_discard_len;
    unsigned int socp_readdone_err_num;
    unsigned int socp_readdone_err_len;
} mdrv_ppm_port_debug_info_s;

typedef struct {
    mdrv_ppm_port_debug_info_s ind_debug_info;
    mdrv_ppm_port_debug_info_s cnf_debug_info;
    mdrv_ppm_port_info_s port_info[OM_PORT_HANDLE_NUM];
    unsigned int invalid_chan;
} mdrv_ppm_chan_debug_info_s;

typedef struct
{
    unsigned int vcom_send_sn;
    unsigned int vcom_send_num;
    unsigned int vcom_send_len;
    unsigned int vcom_send_err_num;
    unsigned int vcom_send_err_len;
    unsigned int max_time_len; /* 调用写接口花费的最大时长 */
    unsigned int vcom_rcv_sn;
    unsigned int vcom_rcv_num;
    unsigned int vcom_rcv_len;
    unsigned int vcom_rcv_err_num;
    unsigned int vcom_rcv_err_len;
} mdrv_ppm_vcom_debug_info_s;

typedef struct {
    unsigned int usb_send_time;
    unsigned int usb_cb_time;
    unsigned int usb_max_send_time;
    unsigned int usb_max_cb_time;
    unsigned int usb_cb_cnt;
} mdrv_ppm_usb_debug_info_s;

typedef struct {
    unsigned int      chan_id;
    char              chan_name[0x10];
    unsigned int      use_size;
    unsigned int      delta_time;
    unsigned int      delta_lost_times;
    unsigned int      delta_lost_len;
    unsigned int      cur_fail_num[EN_DIAG_DST_LOST_MAX];       /* 当前时间段内丢包的各类次数 */
    unsigned int      cur_fail_len[EN_DIAG_DST_LOST_MAX];       /* 当前时间段内丢包的总数据长度 */
    unsigned int      delta_overflow_cnt;                       /* 上报时间段内通道上溢次数 */
    unsigned int      delta_part_overflow_cnt;                  /* 上报时间段内通道80%上溢次数 */
    unsigned int      delta_socp_len;                           /* 上报时间间隔内从socp获取到的数据长度 */
    unsigned int      delta_usb_len;                            /* 上报时间间隔内发送给USB的总字节数 */
    unsigned int      delta_usb_ok_len;                         /* 上报时间间隔内发送给USB返回成功的总字节数 */
    unsigned int      delta_usb_free_len;                       /* 上报时间间隔内USB回调释放的总字节数 */
    unsigned int      delta_usb_fail_len;                       /* 上报时间间写入USB失败的总字节数 */
    unsigned int      delat_vcom_len;                           /* 上报时间间隔内发送给Vcom的总字节数 */
    unsigned int      delat_vcom_fail_len;                      /* 上报时间间隔内Vcom写入失败的总字节数 */
    unsigned int      delat_vcom_succ_len;                      /* 上报时间间隔内Vcom写入成功的总字节数 */
    unsigned int      delat_pcdev_len;                          /* 上报时间间隔内发送给PCDEV的总字节数 */
    unsigned int      delat_pcdev_ok_len;                       /* 上报时间间隔内发送给PCDEV返回成功的总字节数 */
    unsigned int      delat_pcdev_free_len;                     /* 上报时间间隔内PCDEV回调释放的总字节数 */
    unsigned int      delat_pcdev_fail_len;                     /* 上报时间间写入PCDEV失败的总字节数 */
    unsigned int      delat_socket_len;                         /* 上报时间间隔内发送给Socket的总字节数 */
    unsigned int      delat_socket_succ_len;                    /* 上报时间间隔内Socket写入成功的总字节数 */
    unsigned int      delat_socket_fail_len;                    /* 上报时间间隔内Socket写入成功的总字节数 */
    unsigned int      delat_socp_int_to_port_time;              /* 上报时间段内从socp上报中断开始到调用各端口发送的时间总和 */
    unsigned int      delta_usb_send_time;                      /* 上报时间段内发送给USB占用的时间总和 */
    unsigned int      delta_pcdev_send_time;                    /* 上报时间段内发送给USB/PCDEV占用的时间总和 */
    unsigned int      delta_vcom_send_time;                     /* 上报时间段内发送给Vcom占用的时间总和 */
    unsigned int      delta_socket_send_time;                   /* 上报时间段内发送给Socket占用的时间总和 */
    unsigned int      thrput_phy;                               /* 物理通道吞吐率 */
    unsigned int      thrput_cb;                                /* 回调吞吐率 */
    unsigned int      socp_trf_times;                           /* SOCP传输完成次数 */
    unsigned int      socp_thr_ovf_times;                       /* SOCP阈值上溢次数 */
    mdrv_ppm_usb_debug_info_s  ppm_usb_info;                    /* PPM USB维测信息 */
    unsigned int      tool_reserved[0xC];                        /* 给工具预留的64个字节，用于在工具上显示工具的维测信息 */
} diag_mntn_dst_info_s;

typedef unsigned int (*ppm_disconnect_port_cb)(void);
typedef unsigned int (*cpm_rcv_cb)(unsigned char *data, unsigned int len);
typedef unsigned int (*diag_srv_proc_cb)(void *data);

void mdrv_ppm_reg_disconnect_cb(ppm_disconnect_port_cb cb);
void mdrv_scm_reg_ind_coder_dst_send_fuc(void);
unsigned int mdrv_cpm_com_send(unsigned int logic_port, unsigned char *virt_addr, unsigned char *phy_port, unsigned int len);
void mdrv_cpm_logic_rcv_reg(unsigned int logic_port, cpm_rcv_cb rcv_func);
unsigned int mdrv_ppm_log_port_switch(unsigned int  phy_port, unsigned int effect);
unsigned int mdrv_diag_set_log_port(unsigned int  phy_port, unsigned int effect);
unsigned int mdrv_ppm_query_log_port(unsigned int  *log_port);
unsigned int mdrv_diag_get_log_port(unsigned int  *log_port);
void mdrv_diag_get_dst_mntn_info(diag_mntn_dst_info_s * dst_mntn);
void mdrv_scm_set_power_on_log(void);
void mdrv_diag_service_proc_reg(diag_srv_proc_cb srv_fn);
int mdrv_socp_set_cfg_ind_mode(socp_ind_mode_e mode);
int mdrv_socp_get_cfg_ind_mode(unsigned int *mode);
int mdrv_socp_set_cps_ind_mode(deflate_ind_compress_e mode);
int mdrv_socp_get_cps_ind_mode(unsigned int *mode);
void mdrv_ppm_query_usb_info(void *usb_info, unsigned int len);
void mdrv_ppm_clear_usb_time_info(void);
unsigned int mdrv_diag_get_usb_type(void);
unsigned int mdrv_diag_report_reset_msg(diag_trans_ind_s *trans_msg);

/**
 * @brief diag动态内存申请接口
 * @par 描述:
 * diag内部使用，用于动态申请堆内存
 * @attention
 * <ul><li>该接口仅提供给diag内部使用，其他组件不允许使用</li></ul>
 * @param[in] unsigned int size：申请内存大小
 * @retval void *：申请到的内存地址，如果申请失败，返回NULL。
 */
void *mdrv_diag_dymalloc(unsigned int size);

/**
 * @brief diag动态内存释放接口
 * @par 描述:
 * diag内部使用，用于动态内存释放
 * @attention
 * <ul><li>该接口仅提供给diag内部使用，与mdrv_diag_dymalloc配对使用</li></ul>
 * @param[in] const void *p：待释放内存地址
 * @retval ：无。
 */
void mdrv_diag_dymem_free(const void *p);

#ifdef __cplusplus
}
#endif
#endif
