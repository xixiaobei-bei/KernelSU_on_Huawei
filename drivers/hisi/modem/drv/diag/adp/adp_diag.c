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

#include <osl_malloc.h>
#include <mdrv.h>
#include <scm_ind_src.h>
#include <scm_ind_dst.h>
#include <scm_init.h>
#include <scm_common.h>
#include <ppm_common.h>
#include <ppm_socket.h>
#include <ppm_usb.h>
#include <ppm_port_switch.h>
#include <diag_port_manager.h>
#include <diag_system_debug.h>
#include <diag_report.h>
#include <diag_service.h>


#ifdef CONFIG_DIAG_SYSTEM
void mdrv_diag_ptr(unsigned int type, unsigned int para_mark, unsigned int para0, unsigned int para1)
{
    diag_ptr(type, para_mark, para0, para1);
}

void mdrv_ppm_reg_disconnect_cb(ppm_disconnect_port_cb cb)
{
    ppm_disconnect_cb_reg(cb);
}
unsigned int mdrv_get_thrput_info(diag_thrput_id_e type)
{
    return diag_get_throughput_info(type);
}

void mdrv_scm_reg_ind_coder_dst_send_fuc(void)
{
    scm_reg_ind_coder_dst_send_func();
}
unsigned int mdrv_ppm_log_port_switch(unsigned int phy_port, unsigned int effect)
{
    return ppm_port_switch(phy_port);
}
unsigned int mdrv_diag_set_log_port(unsigned int phy_port, unsigned int effect)
{
    return ppm_port_switch(phy_port);
}

unsigned int mdrv_ppm_query_log_port(unsigned int *log_port)
{
    return ppm_query_port(log_port);
}
unsigned int mdrv_diag_get_log_port(unsigned int *log_port)
{
    return ppm_query_port(log_port);
}

void mdrv_ppm_query_usb_info(void *usb_info, unsigned int len)
{
    ppm_query_usb_info(usb_info, len);
}
void mdrv_ppm_clear_usb_time_info(void)
{
    ppm_clear_usb_debug_info();
}
unsigned int mdrv_cpm_com_send(unsigned int logic_port, unsigned char *virt_addr, unsigned char *phy_addr,
    unsigned int len)
{
    return cpm_com_send(logic_port, virt_addr, phy_addr, len);
}
void mdrv_cpm_logic_rcv_reg(unsigned int logic_port, cpm_rcv_cb rcv_func)
{
    ppm_socket_recv_cb_reg(logic_port, rcv_func);
}

void mdrv_scm_set_power_on_log(void)
{
    scm_set_power_on_log();
}

unsigned int mdrv_diag_shared_mem_write(unsigned int type, unsigned int len, const char *data)
{
    return diag_shared_mem_write(type, len, data);
}
unsigned int mdrv_diag_shared_mem_read(unsigned int type)
{
    return diag_shared_mem_read(type);
}

unsigned int mdrv_diag_debug_file_header(const void *file)
{
    return diag_debug_file_header(file);
}

void mdrv_diag_debug_file_tail(const void *file, char *file_path)
{
    return diag_debug_file_tail(file, file_path);
}

unsigned int mdrv_diag_report_log(unsigned int module_id, unsigned int pid, char *file_name, unsigned int line_num,
    const char *fmt, va_list arg)
{
    return (unsigned int)bsp_diag_report_log(module_id, pid, file_name, line_num, fmt, arg);
}

unsigned int mdrv_diag_report_trans(diag_trans_ind_s *trans_msg)
{
    return diag_report_trans(trans_msg);
}

unsigned int mdrv_diag_report_event(diag_event_ind_s *event_msg)
{
    return diag_report_event(event_msg);
}
unsigned int mdrv_diag_report_air(diag_air_ind_s *air_msg)
{
    return diag_report_air(air_msg);
}

unsigned int mdrv_diag_report_trace(void *trace_msg, unsigned int modem_id)
{
    return diag_report_trace(trace_msg, modem_id);
}

void mdrv_diag_report_reset(void)
{
    return diag_report_reset();
}

void mdrv_diag_reset_mntn_info(diag_mntn_e type)
{
    if (type == DIAGLOG_SRC_MNTN) {
        return diag_reset_src_mntn_info();
    } else if (type == DIAGLOG_DST_MNTN) {
        return diag_reset_dst_mntn_info();
    }
}

void *mdrv_diag_get_mntn_info(diag_mntn_e type)
{
    if (type == DIAGLOG_SRC_MNTN) {
        return (void *)diag_get_src_mntn_info();
    } else {
        return diag_get_dst_mntn_info();
    }
}

unsigned int mdrv_diag_report_msg_trans(diag_trans_ind_s *trans_msg, unsigned int cmd_id)
{
    return diag_report_msg_trans(trans_msg, cmd_id);
}

unsigned int mdrv_diag_report_cnf(diag_cnf_info_s *diag_info, void *cnf_msg, unsigned int len)
{
    return diag_report_cnf(diag_info, cnf_msg, len);
}

unsigned int mdrv_diag_report_reset_msg(diag_trans_ind_s *trans_msg)
{
    return diag_report_reset_msg(trans_msg);
}

void mdrv_diag_service_proc_reg(diag_srv_proc_cb srv_fn)
{
    return diag_srv_proc_func_reg(srv_fn);
}

unsigned int mdrv_diag_get_usb_type(void)
{
    return diag_get_usb_type();
}

void *mdrv_diag_dymalloc(u32 size)
{
    return osl_malloc(size);
}

void mdrv_diag_dymem_free(const void *p)
{
    osl_free(p);
}
#else
void mdrv_diag_ptr(unsigned int type, unsigned int para_mark, unsigned int para0, unsigned int para1)
{
    return;
}
void mdrv_ppm_reg_disconnect_cb(ppm_disconnect_port_cb cb)
{
    return;
}
unsigned int mdrv_get_thrput_info(diag_thrput_id_e type)
{
    return 0;
}

void mdrv_scm_reg_ind_coder_dst_send_fuc(void)
{
    return;
}
unsigned int mdrv_ppm_log_port_switch(unsigned int phy_port, unsigned int effect)
{
    return 0;
}
unsigned int mdrv_ppm_query_log_port(unsigned int *log_port)
{
    return 0;
}
unsigned int mdrv_cpm_com_send(unsigned int logic_port, unsigned char *virt_addr, unsigned char *phy_addr,
    unsigned int len)
{
    return 0;
}
void mdrv_cpm_logic_rcv_reg(unsigned int logic_port, cpm_rcv_cb rcv_func)
{
    return;
}
void mdrv_diag_reset_dst_mntn_info(void)
{
    return;
}

unsigned int mdrv_diag_shared_mem_write(unsigned int type, unsigned int len, const char *data)
{
    return 0;
}

unsigned int mdrv_diag_shared_mem_read(unsigned int type)
{
    return 0;
}
unsigned int mdrv_diag_debug_file_header(const void *file)
{
    return 0;
}
void mdrv_diag_debug_file_tail(const void *file, char *file_path)
{
    return;
}

unsigned int mdrv_diag_report_log(unsigned int module_id, unsigned int pid, char *file_name, unsigned int line_num,
    const char *fmt, va_list arg)
{
    return 0;
}

unsigned int mdrv_diag_report_trans(diag_trans_ind_s *trans_msg)
{
    return 0;
}

unsigned int mdrv_diag_report_event(diag_event_ind_s *event_msg)
{
    return 0;
}
unsigned int mdrv_diag_report_air(diag_air_ind_s *air_msg)
{
    return 0;
}

unsigned int mdrv_diag_report_trace(void *trace_msg, unsigned int modem_id)
{
    return 0;
}

void mdrv_diag_report_reset(void)
{
    return;
}

void mdrv_diag_reset_mntn_info(diag_mntn_e type)
{
    return;
}

void *mdrv_diag_get_mntn_info(diag_mntn_e type)
{
    return NULL;
}

unsigned int mdrv_diag_report_msg_trans(diag_trans_ind_s *trans_id, unsigned int cmd_id)
{
    return 0;
}

unsigned int mdrv_diag_report_cnf(diag_cnf_info_s *diag_info, void *cnf_msg, unsigned int len)
{
    return 0;
}

void mdrv_diag_service_proc_reg(diag_srv_proc_cb srv_fn)
{
    return;
}

void mdrv_ppm_query_usb_info(void *usb_info, unsigned int len)
{
    return;
}

void mdrv_ppm_clear_usb_time_info(void)
{
    return;
}

unsigned int mdrv_diag_get_usb_type(void)
{
    return 0;
}

void *mdrv_diag_dymalloc(u32 size)
{
    return NULL;
}

void mdrv_diag_dymem_free(const void *p)
{
    return;
}


#endif
