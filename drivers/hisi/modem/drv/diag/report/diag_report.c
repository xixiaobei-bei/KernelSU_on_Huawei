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
#include "diag_report.h"
#include <product_config.h>
#include <mdrv.h>
#include <mdrv_errno.h>
#include <soc_socp_adapter.h>
#include <securec.h>
#include <bsp_diag.h>
#include <bsp_slice.h>
#include <bsp_dump.h>
#include <bsp_hds_ind.h>
#include "diag_system_debug.h"
#include "diag_service.h"


#define THIS_MODU mod_diag
#ifndef DIAG_SYSTEM_5G
u32 g_ind_trans_id = 0;
u32 g_cnf_trans_id = 0;
#else
u16 g_ind_trans_id = 0;
u16 g_cnf_trans_id = 0;
#endif
diag_log_pkt_num_s g_diag_log_num[DIAG_LOG_BUTT];

u32 diag_send_data(diag_msg_report_head_s *data, diag_log_type_e log_type)
{
    diag_srv_log_head_s *srv_header = (diag_srv_log_head_s *)data->header;
    unsigned long lock;
    u8 timestamp[8] = {0};
    u32 timestamp_len = sizeof(srv_header->frame_header.srv_head.timestamp);
    u32 *num = NULL;
    u32 ret;

    spin_lock_irqsave(&g_diag_log_num[log_type].lock, lock);
    switch (log_type) {
        case DIAG_LOG_PRINT:
            num = &(((diag_srv_print_head_s *)data->header)->print_head.msg_num);
            break;
        case DIAG_LOG_AIR:
            num = &(((diag_srv_air_head_s *)data->header)->air_header.msg_num);
            break;
        case DIAG_LOG_VoLTE:
            num = &(((diag_srv_volte_head_s *)data->header)->volte_header.msg_num);
            break;
        case DIAG_LOG_TRACE:
            num = &(((diag_srv_layer_head_s *)data->header)->trace_header.msg_num);
            break;
        case DIAG_LOG_TRANS:
            num = &(((diag_srv_trans_head_s *)data->header)->trans_header.msg_num);
            break;
        case DIAG_LOG_USER:
            num = &(((diag_srv_user_plane_head_s *)data->header)->user_header.msg_num);
            break;
        case DIAG_LOG_EVENT:
            num = &(((diag_srv_event_head_s *)data->header)->event_header.msg_num);
            break;
        case DIAG_LOG_DT:
            num = &(((diag_srv_dt_head_s *)data->header)->dt_header.msg_num);
            break;
        default:
            break;
    }
    if (num != NULL)
        *num = g_diag_log_num[log_type].num++;

    (void)mdrv_timer_get_accuracy_timestamp((u32 *)&(timestamp[4]), (u32 *)&(timestamp[0]));
    ret = memcpy_s(srv_header->frame_header.srv_head.timestamp, sizeof(srv_header->frame_header.srv_head.timestamp),
        timestamp, timestamp_len);
    if (ret) {
        diag_error("memory copy error 0x%x\n", ret);
    }
    ret = diag_srv_pack_ind_data(data);
    spin_unlock_irqrestore(&g_diag_log_num[log_type].lock, lock);

    return ret;
}

char *diag_get_file_name_from_path(char *file_name)
{
    char *path_pos1 = NULL;
    char *path_pos2 = NULL;
    char *path_pos = NULL;

    /* 操作系统可能使用'\'来查找路径 */
    path_pos1 = (char *)strrchr(file_name, '\\');
    if (path_pos1 == NULL) {
        path_pos1 = file_name;
    }

    /* 操作系统可能使用'/'来查找路径 */
    path_pos2 = (char *)strrchr(file_name, '/');
    if (path_pos2 == NULL) {
        path_pos2 = file_name;
    }

    path_pos = (path_pos1 > path_pos2) ? path_pos1 : path_pos2;

    /* 如果没找到'\'或'/'则使用原来的字符串，否则使用截断后的字符串 */
    if (file_name != path_pos) {
        path_pos++;
    }

    return path_pos;
}

s32 diag_report_drv_log(u32 module_id, u32 pid, const char *fmt)
{
    diag_print_ind_head_s *report_info = NULL;
    diag_srv_log_head_s log_header;
    diag_msg_report_head_s diag_head;
    s32 text_len;
    u32 msg_len;
    s32 ret;
    char text[DIAG_DRV_PRINT_MAX_LEN + 1] = DIAG_HDS_NAME; /* 所有打印文本内容，可能包括文件和行号,以'\0'结尾 */

    /* 由于底软打印中会增加[], HIDS识别时会认为底软中的[] 为行号，因此此处默认填写[] */
    ret = strcat_s(text, DIAG_DRV_PRINT_MAX_LEN, fmt);
    if (ret != EOK) {
        diag_error("strcat_s return error ret:0x%x\n", ret);
        return ERR_MSP_INALID_LEN_ERROR;
    }
    text_len = strnlen(text, DIAG_DRV_PRINT_MAX_LEN);

    report_info = &log_header.print_head;

    /* 组装DIAG命令参数 */
    report_info->module = pid;
    /* 1:error, 2:warning, 3:normal, 4:info */
    /* (0|ERROR|WARNING|NORMAL|INFO|0|0|0) */
    report_info->level = (0x80000000 >> DIAG_GET_PRINTF_LEVEL(module_id));

    /* 字符串的长度加上信息的长度 */
    msg_len = (u32)text_len + sizeof(diag_print_ind_head_s);

    /* 填充数据头 */
    diag_srv_fill_head((diag_srv_head_s *)&log_header);
    DIAG_SRV_SET_MODEM_ID(&log_header.frame_header, DIAG_GET_MODEM_ID(module_id));
    DIAG_SRV_SET_TRANS_ID(&log_header.frame_header, g_ind_trans_id++);
    DIAG_SRV_SET_COMMAND_ID(&log_header.frame_header, DIAG_FRAME_MSG_TYPE_BSP, DIAG_GET_MODE_ID(module_id),
        DIAG_FRAME_MSG_PRINT, 0);
    DIAG_SRV_SET_MSG_LEN(&log_header.frame_header, msg_len);

    diag_head.header_size = sizeof(log_header);
    diag_head.header = &log_header;

    diag_head.data_size = (u32)text_len;
    diag_head.data = (void *)text;

    return (s32)diag_send_data(&diag_head, DIAG_LOG_PRINT);
}

s32 diag_report_ps_log(u32 module_id, u32 pid, char *file_name, u32 line_num, const char *fmt, va_list arg)
{
    diag_print_ind_head_s *report_info = NULL;
    diag_srv_log_head_s log_header;
    diag_msg_report_head_s diag_head;
    char *offset_name = NULL;
    s32 text_len = 0;
    u32 msg_len;
    s32 ret;
    char text[DIAG_PS_PRINT_MAX_LEN + 1]; /* 所有打印文本内容，可能包括文件和行号,以'\0'结尾 */

    if (file_name == NULL) {
        offset_name = " ";
    } else {
        /* 文件名截短 */
        offset_name = diag_get_file_name_from_path(file_name);
    }

    /* 给HSO的打印字符串形式如下:pszFileName[line_num]data。HSO根据中括号[]去截取相应的信息 */
    ret = snprintf_s(text, DIAG_PS_PRINT_MAX_LEN, (DIAG_PS_PRINT_MAX_LEN - 1), "%s[%d]", offset_name, line_num);
    if (ret < 0) {
        diag_error("snprintf_s fail,ulParamLen:0x%x\n", text_len);
        return ERR_MSP_FAILURE;
    } else if (ret > DIAG_PS_PRINT_MAX_LEN) {
        /* 内存越界，主动复位 */
        system_error(DRV_ERRNO_DIAG_OVERFLOW, __LINE__, (u32)text_len, 0, 0);
        return ERR_MSP_FAILURE;
    }
    text_len = ret;

    ret = vscnprintf((text + text_len), (u32)(DIAG_PS_PRINT_MAX_LEN - text_len), (const char *)fmt, arg);
    if (ret < 0) {
        diag_error("vsnprintf fail, ret:0x%x\n", ret);
        return ERR_MSP_FAILURE;
    }
    text_len += ret;

    text[DIAG_PS_PRINT_MAX_LEN - 1] = '\0';
    msg_len = strnlen(text, DIAG_PS_PRINT_MAX_LEN) + 1;

    report_info = &log_header.print_head;

    /* 组装DIAG命令参数 */
    report_info->module = pid;
    /* 1:error, 2:warning, 3:normal, 4:info */
    /* (0|ERROR|WARNING|NORMAL|INFO|0|0|0) */
    report_info->level = (0x80000000 >> DIAG_GET_PRINTF_LEVEL(module_id));

    /* 字符串的长度加上信息的长度 */
    msg_len += sizeof(diag_print_ind_head_s);

    /* 填充数据头 */
    diag_srv_fill_head((diag_srv_head_s *)&log_header);
    DIAG_SRV_SET_MODEM_ID(&log_header.frame_header, DIAG_GET_MODEM_ID(module_id));
    DIAG_SRV_SET_TRANS_ID(&log_header.frame_header, g_ind_trans_id++);
    DIAG_SRV_SET_COMMAND_ID(&log_header.frame_header, DIAG_FRAME_MSG_TYPE_PS, DIAG_GET_MODE_ID(module_id),
        DIAG_FRAME_MSG_PRINT, 0);
    DIAG_SRV_SET_MSG_LEN(&log_header.frame_header, msg_len);

    diag_head.header_size = sizeof(log_header);
    diag_head.header = &log_header;

    diag_head.data_size = (u32)text_len;
    diag_head.data = (void *)&text;

    return (s32)diag_send_data(&diag_head, DIAG_LOG_PRINT);
}

/*
 * 函 数 名: bsp_diag_report_log
 * 功能描述: 打印上报接口
 * 输入参数: module_id( 31-24:modem_id,23-16:modeid,15-12:level )
 * file_name(上报时会把文件路径删除，只保留文件名)
 * line_num(行号)
 * fmt(可变参数)
 * 注意事项: 由于此接口会被协议栈频繁调用，为提高处理效率，本接口内部会使用1K的局部变量保存上报的字符串信息，
 * 从而此接口对协议栈有两点限制，一是调用此接口的任务栈中的内存要至少为此接口预留1K空间；
 * 二是调用此接口输出的log不要超过1K（超出部分会自动丢弃）
 */
s32 bsp_diag_report_log(u32 module_id, u32 pid, char *file_name, u32 line_num, const char *fmt, va_list arg)
{
    if (pid == DIAG_DRV_HDS_PID) {
        return diag_report_drv_log(module_id, pid, fmt);
    } else {
        return diag_report_ps_log(module_id, pid, file_name, line_num, fmt, arg);
    }
}

s32 bsp_diag_trans_report(void *trans_msg)
{
    return bsp_transreport((struct hds_trans_ind *)trans_msg);
}

/*
 * 函 数 名: diag_report_trans
 * 功能描述: 结构化数据上报扩展接口(比DIAG_TransReport多传入了DIAG_MESSAGE_TYPE)
 * 输入参数: mdrv_diag_trans_ind_s->module( 31-24:modem_id,23-16:modeid,15-12:level,11-8:groupid )
 * mdrv_diag_trans_ind_s->msg_id(透传命令ID)
 * mdrv_diag_trans_ind_s->length(透传信息的长度)
 * mdrv_diag_trans_ind_s->data(透传信息)
 */
u32 diag_report_trans(diag_trans_ind_s *trans_msg)
{
    diag_srv_trans_head_s trans_header;
    diag_trans_ind_head_s *trans_info;
    diag_msg_report_head_s diag_head;

    trans_info = &trans_header.trans_header;
    trans_info->module = trans_msg->pid;
    trans_info->msg_id = trans_msg->msg_id;

    /* 填充数据头 */
    diag_srv_fill_head((diag_srv_head_s *)&trans_header);
    DIAG_SRV_SET_MODEM_ID(&trans_header.frame_header, DIAG_GET_MODEM_ID(trans_msg->module));
    DIAG_SRV_SET_TRANS_ID(&trans_header.frame_header, g_ind_trans_id++);
    DIAG_SRV_SET_COMMAND_ID(&trans_header.frame_header, DIAG_GET_GROUP_ID(trans_msg->module),
        DIAG_GET_MODE_ID(trans_msg->module), DIAG_FRAME_MSG_STRUCT, ((trans_msg->msg_id) & 0x7ffff));
    DIAG_SRV_SET_MSG_LEN(&trans_header.frame_header, sizeof(trans_header.trans_header) + trans_msg->length);

    diag_head.header_size = sizeof(trans_header);
    diag_head.header = &trans_header;
    diag_head.data_size = DIAG_SRV_GET_REAL_LEN(trans_msg->length);
    diag_head.data = trans_msg->data;

    return diag_send_data(&diag_head, DIAG_LOG_TRANS);
}

/*
 * 函 数 名: diag_report_event
 * 功能描述: 事件上报接口，给PS使用(替换原来的DIAG_ReportEventLog)
 * 输入参数: mdrv_diag_event_ind_s->module( 31-24:modem_id,23-16:modeid,15-12:level,11-0:pid )
 * mdrv_diag_event_ind_s->event_id(event ID)
 * mdrv_diag_event_ind_s->length(event的长度)
 * mdrv_diag_event_ind_s->data(event信息)
 */
u32 diag_report_event(diag_event_ind_s *event_msg)
{
    diag_srv_event_head_s event_header;
    diag_event_ind_head_s *event_info;
    diag_msg_report_head_s diag_head;

    event_info = &event_header.event_header;
    event_info->event_id = event_msg->event_id;
    event_info->module = event_msg->pid;

    /* 填充数据头 */
    diag_srv_fill_head((diag_srv_head_s *)&event_header);
    DIAG_SRV_SET_MODEM_ID(&event_header.frame_header, DIAG_GET_MODEM_ID(event_msg->module));
    DIAG_SRV_SET_TRANS_ID(&event_header.frame_header, g_ind_trans_id++);
    DIAG_SRV_SET_COMMAND_ID(&event_header.frame_header, DIAG_FRAME_MSG_TYPE_PS, DIAG_GET_MODE_ID(event_msg->module),
        DIAG_FRAME_MSG_EVENT, 0);
    DIAG_SRV_SET_MSG_LEN(&event_header.frame_header, sizeof(event_header.event_header) + event_msg->length);

    diag_head.header_size = sizeof(event_header);
    diag_head.header = &event_header;
    diag_head.data_size = DIAG_SRV_GET_REAL_LEN(event_msg->length);
    diag_head.data = event_msg->data;

    return diag_send_data(&diag_head, DIAG_LOG_EVENT);
}

/*
 * 函 数 名: diag_report_air
 * 功能描述: 空口消息上报接口，给PS使用(替换原来的DIAG_ReportAirMessageLog)
 * 输入参数: mdrv_diag_air_ind_s->module( 31-24:modem_id,23-16:modeid,15-12:level,11-0:pid )
 * mdrv_diag_air_ind_s->msg_id(空口消息ID)
 * mdrv_diag_air_ind_s->direction(空口消息的方向)
 * mdrv_diag_air_ind_s->length(空口消息的长度)
 * mdrv_diag_air_ind_s->data(空口消息信息)
 */
u32 diag_report_air(diag_air_ind_s *air_msg)
{
    diag_msg_report_head_s diag_head;
    diag_srv_air_head_s air_head;
    diag_air_ind_head_s *report_head = NULL;

    report_head = &air_head.air_header;

    report_head->module = air_msg->pid;
    report_head->msg_id = air_msg->msg_id;
    report_head->direction = air_msg->direction;

    /* 填充数据头 */
    diag_srv_fill_head((diag_srv_head_s *)&air_head);
    DIAG_SRV_SET_MODEM_ID(&air_head.frame_header, DIAG_GET_MODEM_ID(air_msg->module));
    DIAG_SRV_SET_TRANS_ID(&air_head.frame_header, g_ind_trans_id++);
    DIAG_SRV_SET_COMMAND_ID(&air_head.frame_header, DIAG_FRAME_MSG_TYPE_PS, DIAG_GET_MODE_ID(air_msg->module),
        DIAG_FRAME_MSG_AIR, 0);
    DIAG_SRV_SET_MSG_LEN(&air_head.frame_header, sizeof(air_head.air_header) + air_msg->length);

    diag_head.header_size = sizeof(air_head);
    diag_head.header = &air_head;
    diag_head.data_size = DIAG_SRV_GET_REAL_LEN(air_msg->length);
    diag_head.data = air_msg->data;

    return diag_send_data(&diag_head, DIAG_LOG_AIR);
}

/*
 * 函 数 名: diag_report_trace
 * 功能描述: 层间消息上报接口，给PS使用(替换原来的DIAG_ReportLayerMessageLog)
 * 输入参数: msg_data(标准的VOS消息体，源模块、目的模块信息从消息体中获取)
 */
u32 diag_report_trace(void *trace_msg, u32 modem_id)
{
    diag_srv_layer_head_s trace_head;
    diag_layer_ind_head_s *trace_info;
    diag_msg_report_head_s diag_head;
    diag_osa_msg_head_s *diag_msg = (diag_osa_msg_head_s *)trace_msg;
    u32 msg_len;

    trace_info = &trace_head.trace_header;
    trace_info->module = diag_msg->sender_pid;
    trace_info->dest_module = diag_msg->receiver_pid;
    trace_info->msg_id = diag_msg->msg_id;

    /* 填充数据头 */
    diag_srv_fill_head((diag_srv_head_s *)&trace_head);
    DIAG_SRV_SET_MODEM_ID(&trace_head.frame_header, modem_id);
    DIAG_SRV_SET_TRANS_ID(&trace_head.frame_header, g_ind_trans_id++);
    DIAG_SRV_SET_COMMAND_ID(&trace_head.frame_header, DIAG_FRAME_MSG_TYPE_PS, DIAG_FRAME_MODE_COMM,
        DIAG_FRAME_MSG_LAYER, 0);
    msg_len = sizeof(diag_osa_msg_head_s) - sizeof(diag_msg->msg_id) + diag_msg->length;
    DIAG_SRV_SET_MSG_LEN(&trace_head.frame_header, sizeof(trace_head.trace_header) + msg_len);

    diag_head.header_size = sizeof(trace_head);
    diag_head.header = &trace_head;
    diag_head.data_size = DIAG_SRV_GET_REAL_LEN(msg_len);
    diag_head.data = diag_msg;

    return diag_send_data(&diag_head, DIAG_LOG_TRACE);
}

/*
 * 函 数 名: diag_report_msg_trans
 * 功能描述: 通用消息上报接口
 * 输入参数: void
 */
u32 diag_report_msg_trans(diag_trans_ind_s *trans_msg, u32 cmd_id)
{
    diag_srv_trans_head_s trans_head;
    diag_trans_ind_head_s *trans_info;
    diag_msg_report_head_s diag_head;

    trans_info = &trans_head.trans_header;
    trans_info->module = trans_msg->pid;

    /* 填充数据头 */
    diag_srv_fill_head((diag_srv_head_s *)&trans_head);
    DIAG_SRV_SET_MODEM_ID(&trans_head.frame_header, DIAG_GET_MODEM_ID(trans_msg->module));
    DIAG_SRV_SET_TRANS_ID(&trans_head.frame_header, g_ind_trans_id++);
    DIAG_SRV_SET_COMMAND_ID_EX(&trans_head.frame_header, cmd_id);
    DIAG_SRV_SET_MSG_LEN(&trans_head.frame_header, sizeof(trans_head.trans_header) + trans_msg->length);

    diag_head.header_size = sizeof(trans_head);
    diag_head.header = &trans_head;
    diag_head.data_size = DIAG_SRV_GET_REAL_LEN(trans_msg->length);
    diag_head.data = trans_msg->data;

    return diag_send_data(&diag_head, DIAG_LOG_TRANS);
}

/*
 * Function Name: diag_report_cnf
 * Description: DIAG message 层上报接口
 */
u32 diag_report_cnf(diag_cnf_info_s *diag_info, void *cnf_msg, u32 len)
{
    diag_srv_head_s cnf_head;
    diag_msg_report_head_s diag_head;

    diag_srv_fill_head((diag_srv_head_s *)&cnf_head);
    cnf_head.frame_header.srv_head.mt_2b = DIAG_FRAME_MT_CNF;
    DIAG_SRV_SET_MODEM_ID(&cnf_head.frame_header, diag_info->modem_id);
    DIAG_SRV_SET_TRANS_ID(&cnf_head.frame_header, g_cnf_trans_id++);
    DIAG_SRV_SET_COMMAND_ID(&cnf_head.frame_header, (diag_info->msg_type & 0xf), (diag_info->mode & 0xf),
        (diag_info->sub_type & 0x1f), (diag_info->msg_id & 0x7ffff));
    DIAG_SRV_SET_MSG_LEN(&cnf_head.frame_header, len);

    diag_head.header_size = sizeof(cnf_head);
    diag_head.header = &cnf_head;

    diag_head.data_size = DIAG_SRV_GET_REAL_LEN(len);
    diag_head.data = cnf_msg;

    diag_lnr(EN_DIAG_DRV_LNR_MESSAGE_RPT, cnf_head.frame_header.cmd_id, bsp_get_slice_value());

    return diag_srv_pack_cnf_data(&diag_head);
}

void diag_report_init(void)
{
    u32 i;
    for (i = 0; i < DIAG_LOG_BUTT; i++) {
        g_diag_log_num[i].num = 0;
        spin_lock_init(&g_diag_log_num[i].lock);
    }
}

/*
 * 函 数 名: diag_report_reset_msg
 * 功能描述:
 * 输入参数: 上报单独复位消息
 */
u32 diag_report_reset_msg(diag_trans_ind_s *trans_msg)
{
    diag_srv_trans_head_s trans_head;
    diag_trans_ind_head_s *trans_info;
    diag_msg_report_head_s diag_head;

    trans_info = &trans_head.trans_header;
    trans_info->module = trans_msg->pid;
    trans_info->msg_id = trans_msg->msg_id;

    /* 填充数据头 */
    diag_srv_fill_head((diag_srv_head_s *)&trans_head);
    DIAG_SRV_SET_MODEM_ID(&trans_head.frame_header, DIAG_GET_MODEM_ID(trans_msg->module));
    DIAG_SRV_SET_TRANS_ID(&trans_head.frame_header, g_ind_trans_id++);
    DIAG_SRV_SET_COMMAND_ID(&trans_head.frame_header, DIAG_GET_GROUP_ID(trans_msg->module),
        DIAG_GET_MODE_ID(trans_msg->module), DIAG_FRAME_MSG_CMD, ((trans_msg->msg_id) & 0x7ffff));
    DIAG_SRV_SET_MSG_LEN(&trans_head.frame_header, sizeof(trans_head.trans_header) + trans_msg->length);

    diag_head.header_size = sizeof(trans_head);
    diag_head.header = &trans_head;
    diag_head.data_size = DIAG_SRV_GET_REAL_LEN(trans_msg->length);
    diag_head.data = trans_msg->data;

    return diag_srv_pack_reset_data(&diag_head);
}

void diag_report_reset(void)
{
    unsigned long lock;
    u32 i;

    for (i = 0; i < DIAG_LOG_BUTT; i++) {
        spin_lock_irqsave(&g_diag_log_num[i].lock, lock);
        g_diag_log_num[i].num = 0;
        spin_unlock_irqrestore(&g_diag_log_num[i].lock, lock);
    }

    g_ind_trans_id = 0;
}

