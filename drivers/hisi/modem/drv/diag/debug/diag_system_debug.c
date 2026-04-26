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

/*
 * 1 Include HeadFile
 */
#include "diag_system_debug.h"
#include <mdrv.h>
#include <mdrv_diag_system.h>
#include <securec.h>
#include <osl_thread.h>
#include <osl_types.h>
#include <osl_malloc.h>
#include <osl_sem.h>
#include <bsp_slice.h>
#include <bsp_rfile.h>
#include <securec.h>
#include "ppm_common.h"
#include "scm_ind_src.h"
#include "scm_common.h"
#include "scm_ind_dst.h"


/* debug提示信息的长度 */
#define DIAG_DEBUG_INFO_LEN 32

#define DIAG_LOG_PATH MODEM_LOG_ROOT "/drv/DIAG/"
#define DIRPAH_LEN 64

diag_debug_info_s g_diag_debug_info = { 0 };
diag_ptr_info_s g_diag_ptr_info = { 0 };
u32 g_diag_log_level = 0;
char *g_diag_ptr_name[EN_DIAG_PTR_CFG_END + 1] = {
    "begin",
    "ppm_rcv",
    "cpm_rcv",
    "scm_soft",
    "scm_self",
    "scm_rcv",
    "scm_rcv1",
    "scm_disp",
    "mspsvic1",
    "mspsvic2",
    "diagsvc1",
    "diagsvc2",
    "diagmsg1",
    "diagmsg2",
    "msgmsp1",
    "msptrans",
    "msp_ps1",
    "msp_ps2",
    "connect",
    "disconn",
    "msg_rpt",
    "svicpack",
    "snd_code",
    "scm_code",
    "code_dst",
    "scm_send",
    "cpm_send",
    "ppm_send",
    "bsp_msg",
    "trans_bbp",
    "bbp_msg",
    "bbp_msg_cnf",
    "bbp_get_chan_size",
    "chan_size_cnf",
    "sample_gen",
    "sample_gen_cnf",
    "bbp_get_addr",
    "get_addr_cnf",
    "bbp_get_version",
    "get_version_cnf",
    "BBP_ABLE_CHAN",
    "ABLE_CHAN_CNF",
    "APP_TRANS_PHY",
    "DSP_MSG",
    "DSP_MSG_CNF",
    "TRANS_HIFI",
    "HIFI_MSG",
    "HIFI_MSG_CNF",
    "SOCP_VOTE",
    "APP_TRANS_MSG",
    "FAIL_CMD_CNF",
    "PRINT_CFG",
    "PRINT_CFG_OK",
    "PRINT_CFG_FAIL",
    "LAYER_CFG",
    "LAYER_CFG_SUCESS",
    "LAYER_CFG_FAIL",
    "AIR_CFG",
    "AIR_CFG_SUCESS",
    "AIR_CFG_FAIL",
    "EVENT_CFG",
    "EVENT_CFG_OK",
    "EVENT_CFG_FAIL",
    "MSG_CFG",
    "MSG_CFG_SUCESS",
    "MSG_CFG_FAIL",
    "GTR_CFG",
    "GTR_CFG_OK",
    "GTR_CFG_FAIL",
    "GUGTR_CFG",
    "GET_TIME",
    "GET_TIME_STAMP_CNF",
    "GET_MODEM_NUM",
    "MODEM_NUM_CNF",
    "GET_PID_TABLE",
    "PID_TABLE_CNF",
    "TRANS_CNF",
    "FS_QUERY",
    "FS_SCAN",
    "FS_MAKE_DIR",
    "FS_OPEN",
    "FS_IMPORT",
    "FS_EXOPORT",
    "FS_DELETE",
    "FS_SPACE",
    "NV_RD",
    "NV_RD_OK",
    "NV_RD_FAIL",
    "GET_NV_LIST",
    "NV_LIST_OK",
    "GET_NV_LIST_FAIL",
    "GET_RESUME_LIST",
    "RESUME_LIST_OK",
    "RESUME_LIST_FAIL",
    "NV_WR",
    "NV_WR_SUCESS",
    "NV_WR_FAIL",
    "AUTH_NV_CFG",
    "NV_AUTH_PROC",
    "NV_AUTH_FAIL",
    "TRANS",
    "TRANS_CNF",
    "HIGHTS",
    "HIGHTS_CNF",
    "USERPLANE",
    "USERPLANE_CNF",
};

u32 g_diag_throughput[EN_DIAG_THRPUT_MAX] = {};
diag_mntn_dst_info_s g_ind_dst_mntn_info = {};
u32 g_send_usb_start_slice = 0;
u32 g_send_pedev_start_slice = 0;
diag_lnr_info_tbl_s g_lnr_info_table[EN_DIAG_DRV_LNR_INFO_MAX] = {{0}};
/*
 * Function Name: diag_ptr
 * Description: DIAG打点信息记录接口
 * History:
 * 1.    2015-06-21 Draft Enact
 */
void diag_ptr(u32 type, u32 para_mark, u32 para0, u32 para1)
{
    g_diag_ptr_info.ptr_element[g_diag_ptr_info.cur_info].step = (u16)type;
    g_diag_ptr_info.ptr_element[g_diag_ptr_info.cur_info].para_mark = (u16)para_mark;
    g_diag_ptr_info.ptr_element[g_diag_ptr_info.cur_info].timestamp = bsp_get_slice_value();
    if (para_mark) {
        g_diag_ptr_info.ptr_element[g_diag_ptr_info.cur_info].para[0] = para0;
        g_diag_ptr_info.ptr_element[g_diag_ptr_info.cur_info].para[1] = para1;
    }
    g_diag_ptr_info.cur_info = (g_diag_ptr_info.cur_info + 1) % DIAG_PTR_NUMBER;
}

void diag_debug_show_ptr(u32 num)
{
    u32 i, cur;
    diag_ptr_info_s *tmp_ptr;
    u32 ptr_size;
    u32 event;

    ptr_size = sizeof(g_diag_ptr_info);

    tmp_ptr = (diag_ptr_info_s *)osl_malloc(ptr_size);
    if (tmp_ptr == NULL) {
        return;
    }

    if (memcpy_s(tmp_ptr, ptr_size, &g_diag_ptr_info, sizeof(g_diag_ptr_info))) {
        osl_free(tmp_ptr);
        return;
    }

    cur = (tmp_ptr->cur_info - num + DIAG_PTR_NUMBER) % DIAG_PTR_NUMBER;

    diag_crit("current ptr:0x%x\n", tmp_ptr->cur_info);
    for (i = 0; i < num; i++) {
        if (tmp_ptr->ptr_element[cur].timestamp != 0) {
            if ((i % 20) == 0) {
                osl_task_delay(10);
            }
            event = tmp_ptr->ptr_element[cur].step;
            diag_crit("%02d %-20s %08d ms \n", event, g_diag_ptr_name[event],
                (tmp_ptr->ptr_element[cur].timestamp / 33));
        }

        cur = (cur + 1) % DIAG_PTR_NUMBER;
    }
    diag_crit("i = %d, over!\n", i);

    osl_free(tmp_ptr);

    return;
}
EXPORT_SYMBOL(diag_debug_show_ptr);

#if !(defined(BSP_CONFIG_PHONE_TYPE)) && defined(DIAG_SYSTEM_5G) && defined(CONFIG_USB_SUPPORT)
extern int bsp_get_usb_speed(void);
u32 diag_get_usb_type(void)
{
    int usb_type;

    usb_type = bsp_get_usb_speed();
    switch (usb_type) {
        case 20:
            return (u32)TYPE_USB_SPEED_HIGH;

        case 30:
            return (u32)TYPE_USB_SPEED_SUPER;

        case 31:
            return (u32)TYPE_USB_SPEED_SUPER_PLUS;

        default:
            return (u32)NOT_SUPPORT_QUERY;
    }
}
#else
u32 diag_get_usb_type(void)
{
    return (u32)NOT_SUPPORT_QUERY;
}
#endif

/*
 * Function Name: diag_debug_ptr
 * Description: DIAG处理流程的打点信息
 */
void diag_debug_ptr(void)
{
    void *file = NULL;
    u32 ret;
    u32 value;
    s8 *dir_path = DIAG_LOG_PATH;
    s8 *file_path = DIAG_LOG_PATH "diag_ptr.bin";

    /* 如果DIAG目录不存在则先创建目录 */
    if (mdrv_file_access(dir_path, RFILE_RDONLY) != BSP_OK) {
        if (mdrv_file_mkdir(dir_path) != BSP_OK) {
            diag_error("mkdir %s failed.\n", DIAG_LOG_PATH);
            return;
        }
    }

    file = mdrv_file_open(file_path, "wb+");
    if (file == NULL) {
        diag_error(" bsp_open failed.\n");
        return;
    }

    ret = diag_debug_file_header(file);
    if (ret != BSP_OK) {
        diag_error("diag_debug_file_header failed .\n");
        ret = mdrv_file_close(file);
        if (ret) {
            diag_error("mdrv_file_close fail,0x%x\n", ret);
        }
        return;
    }

    /* 打点信息长度 */
    value = DIAG_DEBUG_SIZE_FLAG | sizeof(g_diag_ptr_info);
    ret = mdrv_file_write(file, 1, sizeof(value), (s8 *)&value);
    if (ret != sizeof(value)) {
        diag_error("write sizeof g_diag_ptr_info failed.\n");
    }

    /* 再写入打点信息 */
    ret = mdrv_file_write(file, 1, sizeof(g_diag_ptr_info), (s8 *)&g_diag_ptr_info);
    if (ret != sizeof(g_diag_ptr_info)) {
        diag_error("write g_diag_ptr_info failed.\n");
    }

    diag_debug_file_tail(file, file_path);

    ret = mdrv_file_close(file);
    if (ret) {
        diag_error("mdrv_file_close fail,0x%x\n", ret);
    }
    return;
}
EXPORT_SYMBOL(diag_debug_ptr);

/*
 * Function Name: diag_debug_file_header
 * Description: 给debug文件写上文件头
 */
u32 diag_debug_file_header(const void *file)
{
    u32 ret;
    u32 value;

    ret = (u32)mdrv_file_seek(file, 0, DRV_SEEK_SET);
    if (ret != BSP_OK) {
        diag_error("file_seek failed .\n");
        return (u32)BSP_ERROR;
    }

    value = DIAG_DEBUG_START;

    /* file start flag */
    ret = (u32)mdrv_file_write(&value, 1, sizeof(value), file);
    if (ret != sizeof(value)) {
        diag_error("write start flag failed.\n");
        return (u32)BSP_ERROR;
    }

    value = DIAG_DEBUG_VERSION;

    /* debug version */
    ret = (u32)mdrv_file_write(&value, 1, sizeof(value), file);
    if (ret != sizeof(value)) {
        diag_error("write debug version failed.\n");
        return (u32)BSP_ERROR;
    }

    value = 0;

    /* file size */
    ret = (u32)mdrv_file_write(&value, 1, sizeof(value), file);
    if (ret != sizeof(value)) {
        diag_error("write file size failed.\n");
        return (u32)BSP_ERROR;
    }

    value = bsp_get_slice_value();

    /* 当前的slice */
    ret = (u32)mdrv_file_write(&value, 1, sizeof(value), file);
    if (ret != sizeof(value)) {
        diag_error("write timestamp failed.\n");
        return (u32)BSP_ERROR;
    }

    return ERR_MSP_SUCCESS;
}

/*
 * Function Name: diag_debug_file_tail
 * Description: 给debug文件写上文件尾
 */
void diag_debug_file_tail(const void *file, char *path)
{
    u32 ret;
    u32 value;

    /* file end flag */
    value = DIAG_DEBUG_END;
    ret = (u32)mdrv_file_write(&value, 1, sizeof(value), file);
    if (ret != sizeof(value)) {
        diag_error("write start flag failed %s.\n", path);
    }
}

/*
 * Function Name: diag_ThroughputIn
 * Description: 吞吐率记录
 */
u32 diag_get_throughput_info(diag_thrput_id_e type)
{
    return g_diag_throughput[type];
}

/* 复位维测信息记录 */
void diag_reset_dst_mntn_info(void)
{
    memset_s(&g_ind_dst_mntn_info, sizeof(g_ind_dst_mntn_info), 0, sizeof(g_ind_dst_mntn_info));
    memset_s(&g_diag_throughput, sizeof(g_diag_throughput), 0, sizeof(g_diag_throughput));
}

s32 diag_system_debug_event_cb(unsigned int chan_id, unsigned int event, unsigned int param)
{
    u32 ret;
    u32 busy_size;
    u32 dst_size;
    socp_buffer_rw_s stSocpBuff;

    if ((chan_id == SOCP_CODER_DST_OM_IND) &&
        ((event == SOCP_EVENT_OUTBUFFER_OVERFLOW) || (event == SOCP_EVENT_OUTBUFFER_THRESHOLD_OVERFLOW))) {
        g_ind_dst_mntn_info.delta_overflow_cnt++;

        dst_size = scm_ind_get_dst_buff_size();
        ret = bsp_socp_get_read_buff(SOCP_CODER_DST_OM_IND, &stSocpBuff);
        if (ret) {
            return ret;
        }

        busy_size = stSocpBuff.rb_size + stSocpBuff.size;
        /* 目的通道buff 80%上溢次数统计 */
        if (busy_size * 100 >= dst_size * 80) {
            g_ind_dst_mntn_info.delta_part_overflow_cnt++;
        }
    }
    return BSP_OK;
}

diag_mntn_dst_info_s *diag_get_dst_mntn_info(void)
{
    return &g_ind_dst_mntn_info;
}

void diag_ps_buffer_overflow_record(u32 pack_len)
{
    if (pack_len < DIAG_SRC_50_OVERFLOW_THR) {
        g_diag_debug_info.overflow_50_num++;
    }
    if (pack_len < DIAG_SRC_80_OVERFLOW_THR) {
        g_diag_debug_info.overflow_80_num++;
    }
}

u32 diag_debug_log_level(u32 level)
{
    g_diag_log_level = level; /* diag log level: 0=error, !0=debug */
    return g_diag_log_level;
}

void diag_ps_ind_lost_record(u32 len)
{
    g_diag_debug_info.abandon_num++;
    g_diag_debug_info.abandon_len += len;
}

void diag_ps_package_record(u32 pack_len)
{
    g_diag_debug_info.package_len += pack_len;
    g_diag_debug_info.package_num++;
}

void diag_reset_src_mntn_info(void)
{
    memset_s(&g_diag_debug_info, sizeof(g_diag_debug_info), 0, sizeof(g_diag_debug_info));
}

diag_debug_info_s *diag_get_src_mntn_info(void)
{
    return &g_diag_debug_info;
}

/*
 * Function Name: diag_LNR
 * Description: 最后NV条信息的记录接口
 */
void diag_lnr(diag_lnr_id_e type, u32 param1, u32 param2)
{
    g_lnr_info_table[type].param1[g_lnr_info_table[type].cur_info] = param1;
    g_lnr_info_table[type].param2[g_lnr_info_table[type].cur_info] = param2;
    g_lnr_info_table[type].cur_info = (g_lnr_info_table[type].cur_info + 1) % DIAG_DRV_LNR_NUMBER;
}

void diag_show_lnr(diag_lnr_id_e type, u32 n)
{
    u32 i;
    u32 cur;

    cur = (g_lnr_info_table[type].cur_info + DIAG_DRV_LNR_NUMBER - n) % DIAG_DRV_LNR_NUMBER;

    for (i = 0; i < n; i++) {
        diag_crit("0x%x, 0x%x.\n", g_lnr_info_table[type].param1[cur], g_lnr_info_table[type].param2[cur]);
        cur = (cur + 1) % DIAG_DRV_LNR_NUMBER;
    }
}

