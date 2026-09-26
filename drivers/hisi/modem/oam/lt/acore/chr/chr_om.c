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
 *
 */

/*
 * 1 头文件包含
 */

#include <securec.h>
#include "product_config.h"
#include <mdrv_diag_system.h>
#include <mdrv_nvim.h>
#include <mdrv_chr.h>
#include "at_app_vcom_interface.h"
#include "nv_stru_sys.h"
#include "acore_nv_stru_msp.h"
#include "msp_diag_comm.h"
#include "msp_diag.h"
#include <eagleye.h>
#include "chr_om.h"
#include "chr_cfg.h"


#define THIS_FILE_ID PS_FILE_ID_OM_ERRORLOG_C

#undef THIS_MODU
#define THIS_MODU "mdm_chr"
unsigned int chr_get_drv_data(void *data, unsigned int msg_len);
#if (FEATURE_ON == FEATURE_CHR_OM)

unsigned int g_chr_acpu_dbg_flag = VOS_FALSE;

chr_app_msg_record_s g_chr_app_msg_record; /* OM收到AP需要在全局变量中记录内容 */

struct semaphore g_chr_rx_buff_sem;

mdrv_ppm_vcom_debug_info_s g_chr_vcom_debug_info = { 0 };

chr_errlog_debug_info g_chr_recv_ue_send_ap = { 0 };

HTIMER g_chr_acpu_full_tmr = NULL; /* 查询上报起定时器 */

HTIMER g_chr_acpu_clt_info_tmr = NULL; /* 信息收集起定时器 */

/* 新增下发上报请求时的时间戳，用于故障上报结束时的消息 */
unsigned long long g_chr_trig_time_64bit = 0;

NV_ID_CHR_CONFIG_CTRL_INFO_STRU g_chr_fault_map_gutl = { 0 };
NV_ID_CHR_C_CONFIG_CTRL_INFO_STRU g_chr_fault_map_cdma = { 0 };

/* 1表示超时后下次下发采集请求前，MTA上报的消息将不被OM处理 */
unsigned int g_chr_acpu_clt_info_no_proc = 0;
unsigned char g_chr_rpt_flag[OM_CHR_RPT_FLAG_LEN] = {0};

#define chr_log_report(fmt, ...)                                                                                      \
    mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(0, 0, 1), MSP_PID_CHR, "chr_om", __LINE__, "%s:" fmt, __FUNCTION__, \
        ##__VA_ARGS__)
#define OM_ERRLOG_MAX_CFG_LEN 1024
/*
 * 功能描述: 打印当前OM通道的状态
 */
void chr_open_log(unsigned int flag)
{
    g_chr_acpu_dbg_flag = flag;

    return;
}

/*
 * 功能描述: 各组件给Om上报消息完毕
 */
void chr_acpu_rcv_msg_finish(void)
{
    chr_app_report_status_s app_report_status = { { 0, }, 0, 0, 0, 0, 0, 0 };

    if (memset_s(&app_report_status, sizeof(app_report_status), 0, (unsigned int)(sizeof(chr_app_report_status_s)))) {
        chr_print("memset failed!\n");
    }

    app_report_status.chr_om_header.msgtype = OM_ERR_LOG_MSG_ERR_REPORT_END;
    app_report_status.chr_om_header.msgsn = g_chr_vcom_debug_info.vcom_send_sn++;
    app_report_status.chr_om_header.msglen = (sizeof(chr_app_report_status_s) - sizeof(chr_alarm_msg_head_s));
    app_report_status.modemid = g_chr_app_msg_record.modem_id;
    app_report_status.faultid = g_chr_app_msg_record.fault_id;
    app_report_status.report_status = g_chr_app_msg_record.report_need;
    (void)mdrv_timer_get_accuracy_timestamp(&app_report_status.high_slice, &app_report_status.low_slice);

    chr_acpu_send_vcom_data((unsigned char *)&app_report_status, (unsigned int)sizeof(chr_app_report_status_s));
    if (memset_s(g_chr_rpt_flag, sizeof(g_chr_rpt_flag), OM_MSG_RECEIVE_FLAG, sizeof(g_chr_rpt_flag))) {
        chr_print("memset failed!\n");
    }
    /* 防止消息再次下发不成功 */
    g_chr_app_msg_record.report_need = OM_AP_SEND_MSG_FINISH;
    g_chr_app_msg_record.state = ERRLOG_IDLE;
    chr_print_dbg("success!\r\n ");

    return;
}

/*
 * 功能描述: OM给AP回复处理结果
 */
void chr_acpu_send_app_result(unsigned int result, unsigned short modem_id)
{
    chr_app_result_cnf_s app_result_cnf = { { 0, }, 0, 0, 0, 0 };

    if (memset_s(&app_result_cnf, sizeof(app_result_cnf), 0, (unsigned int)sizeof(chr_app_result_cnf_s))) {
        chr_print("memset failed!\n");
    }

    app_result_cnf.chr_om_header.msgtype = OM_ERR_LOG_MSG_RESULT;
    app_result_cnf.chr_om_header.msgsn = g_chr_vcom_debug_info.vcom_send_sn++;
    app_result_cnf.chr_om_header.msglen = (sizeof(chr_app_result_cnf_s) - sizeof(chr_alarm_msg_head_s));
    app_result_cnf.modemid = modem_id;
    app_result_cnf.status = result;

    chr_acpu_send_vcom_data((unsigned char *)&app_result_cnf, (unsigned int)sizeof(chr_app_result_cnf_s));
    chr_print_dbg("Result response success. status = 0x%x!\r\n ", result);
    return;
}
/*
 * 功能描述: 设置标志
 */
void chr_acpu_set_rpt_flag(VOS_UINT id)
{
    unsigned int i, j;

    i = id / (sizeof(unsigned int) * OM_UCHAR_BIT_NUMS);
    j = id % (sizeof(unsigned int) * OM_UCHAR_BIT_NUMS);

    g_chr_app_msg_record.rpt_flag[i] |= 1 << j;

    return;
}

/*
 * 功能描述: 收到的模块标志位置0
 */
void chr_acpu_send_skip_pid(unsigned int idx)
{
    if (g_chr_app_msg_record.report_need > 0) {
        chr_acpu_set_rpt_flag(idx);
        g_chr_app_msg_record.report_need--;
    }
}

/*
 * 功能描述: 获取PID表
 */
int chr_acpu_rcv_app_msg_check(chr_app_om_req_s *pstAppOmReqErrLog)
{
    /* 如果没有上报完成 */
    if ((g_chr_app_msg_record.report_need != OM_AP_SEND_MSG_FINISH) || (g_chr_app_msg_record.state != ERRLOG_IDLE)) {
        chr_print("error log report not end. reportsend = %d!\r\n ", g_chr_app_msg_record.report_need);

        return OM_APP_REPORT_NOT_FINISH_ERR;
    }

    return VOS_OK;
}
/*
 * 功能描述: 清除PID标志位
 */
void chr_acpu_clear_send_flag_all(void)
{
    if (memset_s(g_chr_app_msg_record.send_flag, OM_SEND_RPT_FLAG_LEN * sizeof(unsigned int),
                 0, OM_SEND_RPT_FLAG_LEN * sizeof(unsigned int))) {
        chr_print("memset failed!\n");
    }
    if (memset_s(g_chr_app_msg_record.rpt_flag, OM_SEND_RPT_FLAG_LEN * sizeof(unsigned int),
                 0, OM_SEND_RPT_FLAG_LEN * sizeof(unsigned int))) {
        chr_print("memset failed!\n");
    }
    return;
}
/*
 * 功能描述: 发送pid标志处理函数
 */
void chr_acpu_set_send_flag(VOS_UINT id)
{
    VOS_UINT i, j;

    i = id / (sizeof(unsigned int) * OM_UCHAR_BIT_NUMS);
    j = id % (sizeof(unsigned int) * OM_UCHAR_BIT_NUMS);

    g_chr_app_msg_record.send_flag[i] |= 1 << j;

    return;
}

/*
 * 功能描述: 对收到的pid进行标志清空
 */
VOS_BOOL chr_acpu_chk_rpt_flag(VOS_UINT id)
{
    unsigned int i, j;

    i = id / (sizeof(unsigned int) * OM_UCHAR_BIT_NUMS);
    j = id % (sizeof(unsigned int) * OM_UCHAR_BIT_NUMS);

    if (g_chr_app_msg_record.rpt_flag[i] & (1 << j)) {
        return VOS_FALSE;
    } else {
        return VOS_TRUE;
    }
}

/*
 * 功能描述: 解析控制NV列表
 */
int chr_acpu_get_fault_map(unsigned int modem_id, unsigned int fault_id, VOS_UINT *fault_num, FAULTID_MAP **fault_map)
{
    VOS_UINT result = NV_OK;

    if (fault_id < CHR_FAULTID_GUTL_MAX) {
        if (0 == g_chr_fault_map_gutl.ulFaultIdNum) {
            result = mdrv_nv_readex(modem_id, NV_ID_ErrLogGutlCtrlInfo, (void *)&g_chr_fault_map_gutl,
                sizeof(NV_ID_CHR_CONFIG_CTRL_INFO_STRU));
        }

        *fault_num = g_chr_fault_map_gutl.ulFaultIdNum;
        *fault_map = g_chr_fault_map_gutl.aucFaulIdMap;

        if (result != NV_OK) {
            return VOS_ERR;
        } else {
            return VOS_OK;
        }
    } else if (fault_id < CHR_FAULTID_CDMA_MAX) {
        if (0 == g_chr_fault_map_cdma.ulFaultIdNum) {
            result = mdrv_nv_readex(modem_id, NV_ID_ErrLogCdmaCtrlInfo, (void *)&g_chr_fault_map_cdma,
                sizeof(NV_ID_CHR_C_CONFIG_CTRL_INFO_STRU));
        }

        *fault_num = g_chr_fault_map_cdma.ulFaultIdNum;
        *fault_map = g_chr_fault_map_cdma.aucFaulIdMap;

        if (result != NV_OK) {
            return VOS_ERR;
        } else {
            return VOS_OK;
        }
    } else {
        return VOS_ERR;
    }
}
/*
 * 功能描述: 获取faultid对应的alamid列表
 */
int chr_acpu_get_fault_cfg(unsigned int fault_id, unsigned int modem_id, unsigned int *ulNvId,
    NV_ID_CHR_FAULTID_CONFIG_STRU *fault_cfg)
{
    VOS_UINT result;
    VOS_UINT i = 0;
    VOS_UINT fault_id_num;
    FAULTID_MAP *fault_map = VOS_NULL_PTR;
    VOS_UINT nvid = (unsigned int)(-1);

    result = chr_acpu_get_fault_map(modem_id, fault_id, &fault_id_num, &fault_map);
    if (result != VOS_OK) {
        chr_print("get faultmap error. faultid =  %d!\n", fault_id);
        return OM_APP_GET_NVID_ERR;
    }

    if (CHR_FAULTID_NUM_MAX < fault_id_num) {
        chr_print("faultid num is too large. faultidnum =  %d, faultid = %d!\n", fault_id_num, fault_id);
        return OM_APP_GET_NVID_ERR;
    }

    for (i = 0; i < fault_id_num; i++) {
        if (fault_id == fault_map[i].usFaultId) {
            nvid = fault_map[i].ulNvId;
        }
    }
    if ((unsigned int)(-1) == nvid) {
        chr_print("no nvid matched. faultidnum = %d. faultid = %d!\n", fault_id_num, fault_id);
        return OM_APP_READ_NV_ERR;
    }
    *ulNvId = nvid;

    result = mdrv_nv_readex(modem_id, nvid, (void *)fault_cfg, sizeof(NV_ID_CHR_FAULTID_CONFIG_STRU));
    if (result != VOS_OK) {
        chr_print("alarmid nv read failed. nvid = %d, modemid = %d!\n", nvid, modem_id);
        return OM_APP_READ_NV_ERR;
    }

    if (fault_id != fault_cfg->ulFaultId) {
        chr_print("pFaultCfg->ulFaultId wrong!nvid = %d, modemid = %d, faultid = %d, cfgfaultid = %d!\n", nvid,
            modem_id, fault_id, fault_cfg->ulFaultId);

        return OM_APP_READ_NV_ERR;
    }

    return VOS_OK;
}
/*
 * 功能描述: OM给各组件发送消息PID
 */
int chr_acpu_send_req(unsigned int *send_cnt)
{
    NV_ID_CHR_FAULTID_CONFIG_STRU *fault_cfg = &g_chr_app_msg_record.fault_cfg;
    chr_log_report_req_s *report_req = VOS_NULL_PTR;
    unsigned int i;
    unsigned int recv_pid;
    unsigned int alarm_id;
    unsigned int send_pid_cnt = 0;

    for (i = 0; i < fault_cfg->ulAlarmNum; i++) {
        recv_pid = fault_cfg->aucAlarmMap[i].ulPid;
        alarm_id = fault_cfg->aucAlarmMap[i].ulAlarmId;

        /* if pid is invalid, skip it! */
        if (VOS_PID_AVAILABLE != VOS_CheckPidValidity(recv_pid)) {
            chr_acpu_send_skip_pid(i);
            chr_print_dbg("Index[%d]: Pid 0x%x Alarmid 0x%x is invalid, Skip it!\n", i, recv_pid, alarm_id);
            continue;
        }

        /* 给对应的PID发送消息 */
        send_pid_cnt++;
        report_req =
            (chr_log_report_req_s *)VOS_AllocMsg(MSP_PID_CHR, (sizeof(chr_log_report_req_s) - VOS_MSG_HEAD_LENGTH));
        if (report_req == VOS_NULL_PTR) {
            chr_print("Alloc Req Msg failed!\n");
            return OM_APP_MEM_ALLOC_MSG_ERR;
        }

        report_req->ulReceiverPid = recv_pid;
        report_req->msg_name = ID_OM_ERR_LOG_REPORT_REQ;
        report_req->modemid = g_chr_app_msg_record.modem_id;
        report_req->faultid = g_chr_app_msg_record.fault_id;
        report_req->alarmid = alarm_id;
        report_req->msgsn = g_chr_app_msg_record.msg_sn;

        if (VOS_OK != VOS_SendMsg(MSP_PID_CHR, report_req)) {
            chr_print("send pid 0x%x, Alarmid 0x%x  !\n", recv_pid, alarm_id);
        }
        chr_acpu_set_send_flag(i);
        chr_print_dbg("send pid 0x%x, Alarmid 0x%x  !\n", recv_pid, alarm_id);
    }

    *send_cnt = send_pid_cnt;

    return VOS_OK;
}

/*
 * 功能描述: 起5s的定时器
 */
int chr_acpu_start_timer(void)
{
    /* 起5s定时器 */
    g_chr_acpu_full_tmr = VOS_NULL_PTR;
    if (VOS_OK != VOS_StartRelTimer(&g_chr_acpu_full_tmr, MSP_PID_CHR, OM_ERRLOG_TIMER_LENTH, OM_ERRORLOG_TIMER_NAME,
        OM_ERRORLOG_TIMER_PARA, VOS_RELTIMER_NOLOOP, VOS_TIMER_PRECISION_5)) {
        chr_print("StartDrxTimer fail\n");

        return OM_APP_ERRLOG_START_TIMER_ERR;
    }
    chr_print_dbg("start timer success!\n");

    return VOS_OK;
}

int chr_send_req_report(void)
{
    int result;
    unsigned int send_pid_cnt = 0;
    /* 根据告警相关性，向对应PID发送消息 */
    result = chr_acpu_send_req(&send_pid_cnt);
    if (result != VOS_OK) {
        g_chr_app_msg_record.state = ERRLOG_IDLE;
        return result;
    }

    /* 假如没有告警相关性PID，直接给AP回复消息上报完毕 */
    if (send_pid_cnt == 0) {
        chr_print("no PID send.\n");
        chr_acpu_rcv_msg_finish();
        return VOS_OK;
    }
    /* 起timer定时器 */
    result = chr_acpu_start_timer();
    if (result != VOS_OK) {
        chr_print("start timer failed\n");
        return result;
    }

    return VOS_OK;
}
/*
 * 功能描述: 向PAM获取PID表，向各组件下发Errorlog 上报请求
 */
int chr_acpu_report_msg(unsigned char *data, unsigned int msg_len)
{
    chr_app_om_req_s *app_om_req = VOS_NULL_PTR;
    int result;
    unsigned int *nvid = &g_chr_app_msg_record.fault_nv;
    NV_ID_CHR_FAULTID_CONFIG_STRU *fault_cfg = &g_chr_app_msg_record.fault_cfg;

    if (data == VOS_NULL_PTR) {
        chr_print("input data is NULL !\n");
        return OM_APP_PARAM_INAVALID;
    }

    if (msg_len < sizeof(chr_app_om_req_s)) {
        chr_print("input length too short !\n");
        return OM_APP_MSG_LENGTH_ERR;
    }
    /* 收到AP消息检查 */
    app_om_req = (chr_app_om_req_s *)data;

    /* 输入消息长度的检查 */
    if (msg_len != (app_om_req->chr_om_header.msglen + sizeof(chr_alarm_msg_head_s))) {
        chr_print("input length not match !\n");
        return OM_APP_MSG_LENGTH_ERR;
    }

    /* 是否完成上一次上报，OM是否处于BUSY状态 */
    result = chr_acpu_rcv_app_msg_check(app_om_req);
    if (result != VOS_OK) {
        chr_print("check req failed. status = 0x%x,fauiltid = %d, modemid = %d!\n", result, app_om_req->faultid,
            app_om_req->modemid);
        return result;
    } else {
        g_chr_app_msg_record.state = ERRLOG_BUSY;
        g_chr_app_msg_record.msg_sn++;
        g_chr_app_msg_record.fault_id = app_om_req->faultid;
        g_chr_app_msg_record.modem_id = app_om_req->modemid;
        g_chr_app_msg_record.alarm_id_num = 0;
        fault_cfg->ulAlarmNum = 0;
    }

    chr_print_dbg("modem:0x%x,faultid:0x%x!\n", app_om_req->modemid, app_om_req->faultid);

    result = chr_acpu_get_fault_cfg(app_om_req->faultid, app_om_req->modemid, nvid, fault_cfg);
    if (result != VOS_OK) {
        chr_print("get fault cfg failed.status = %d, faultid = %d, modemid = %d!\n", result, app_om_req->faultid,
            app_om_req->modemid);
        g_chr_app_msg_record.state = ERRLOG_IDLE;
        return result;
    } else {
        g_chr_app_msg_record.alarm_id_num = fault_cfg->ulAlarmNum;
        g_chr_app_msg_record.report_need = fault_cfg->ulAlarmNum;
        chr_acpu_clear_send_flag_all();
        chr_print_dbg("faultnv = %d ulAlarmIdNum = %d!\n", *nvid, fault_cfg->ulAlarmNum);
    }

    return chr_send_req_report();
}

/*
 * 功能描述:  可维可测，收到的消息内容发往log工具一份
 */
void chr_acpu_msg_hook(unsigned char *data, unsigned int msg_len, unsigned int data_type)
{
    mdrv_diag_trans_ind_s diag_trans = { 0 };

    diag_trans.ulModule = MDRV_DIAG_GEN_MODULE_EX(0, DIAG_MODE_LTE, DIAG_MSG_TYPE_MSP);
    diag_trans.ulPid = MSP_PID_CHR;
    diag_trans.ulMsgId = OM_APP_ERRLOG_HOOK_IND | data_type;
    diag_trans.ulLength = msg_len;
    diag_trans.pData = data;

    mdrv_diag_trans_report_ex(&diag_trans);

    return;
}

/*
 * 功能描述:  调NAS接口，把数据发走
 */
void chr_acpu_send_vcom_data(unsigned char *data, unsigned int msg_len)
{
    /* 调用注册函数，给Vcom发数据 */
    g_chr_vcom_debug_info.vcom_send_num++;
    g_chr_vcom_debug_info.vcom_send_len += msg_len;

    OM_ACPU_CHR_DEBUG_TRACE((unsigned char *)data, msg_len, OM_ACPU_ERRLOG_SEND);

    /* 增加Trans勾包 */
    chr_acpu_msg_hook(data, msg_len, OM_ERRLOG_SEND_MSG);

    if (VOS_OK != APP_VCOM_SEND(APP_VCOM_DEV_INDEX_ERRLOG, data, msg_len)) {
        g_chr_vcom_debug_info.vcom_send_err_num++;
        g_chr_vcom_debug_info.vcom_send_err_len += msg_len;

        (void)chr_print("Send vcom Msg failed.\n");

        return;
    }
    return;
}

/*
 * 功能描述:  收到各组件消息，检查是否需要上报
 */
unsigned int chr_acpu_rcv_msg_header_check(chr_recv_report_s *recv_data_info, unsigned int *send_pid_cnt)
{
    NV_ID_CHR_FAULTID_CONFIG_STRU *fault_cfg = &g_chr_app_msg_record.fault_cfg;
    unsigned int i;

    if (g_chr_app_msg_record.state == ERRLOG_IDLE) {
        chr_print("state is idle, drop the message!\n");
        return VOS_ERR;
    }

    /* 记录对应组件已上报消息 */
    for (i = 0; i < g_chr_app_msg_record.alarm_id_num; i++) {
        if ((fault_cfg->aucAlarmMap[i].ulPid == recv_data_info->chr_rcv_header.ulSenderPid) &&
            (fault_cfg->aucAlarmMap[i].ulAlarmId == recv_data_info->chr_rcv_header.alarmid) &&
            (g_chr_app_msg_record.msg_sn == recv_data_info->msg_sn) && (chr_acpu_chk_rpt_flag(i))) {
            (*send_pid_cnt)++;
            chr_acpu_set_rpt_flag(i);
            g_chr_app_msg_record.report_need--;

            g_chr_recv_ue_send_ap.send_num++;
            g_chr_recv_ue_send_ap.send_len += recv_data_info->rpt_len;

            break;
        }
    }

    return VOS_OK;
}

/*
 * 功能描述:  收到各组件消息，检查是否需要上报
 */
unsigned int chr_acpu_rcv_msg_check(chr_recv_report_s *recv_data_info)
{
    unsigned int send_pid_cnt = 0;

    switch (recv_data_info->chr_type_union.msg_type) {
        case OM_ERR_LOG_MSG_ERR_REPORT:
            /* Error Log 上报 */
            if (VOS_OK != chr_acpu_rcv_msg_header_check(recv_data_info, &send_pid_cnt)) {
                return VOS_ERR;
            }
            break;

        case OM_ERR_LOG_MSG_FAULT_REPORT:
            /* 平台检测故障主动上报 */
            send_pid_cnt++;
            break;

        case OM_ERR_LOG_MSG_ALARM_REPORT:
            /* 平台检测告警主动上报 */
            send_pid_cnt++;
            break;

        default:
            /* 异常 */
            (void)chr_print("Msg type wrong!\r\n ");

            return VOS_ERR;
    }

    /* 不是预期上报,丢弃消息 */
    if (send_pid_cnt == OM_AP_NO_MSG_SEND) {
        (void)chr_print("No msg Send!\r\n ");

        return VOS_ERR;
    }

    return VOS_OK;
}

/*
 * 功能描述: 测试桩函数:模拟AP请求上报Error log
 */
int chr_acpu_report_msg_test(unsigned short modem_id, unsigned short fault_id)
{
    chr_app_om_req_s app_ctrl_status;

    int ret;
    if (memset_s(&app_ctrl_status, sizeof(app_ctrl_status), 0, (unsigned int)(sizeof(chr_app_om_req_s)))) {
        chr_print("memset failed!\n");
    }

    app_ctrl_status.chr_om_header.msgtype = OM_ERR_LOG_MSG_SET_ERR_REPORT;
    app_ctrl_status.chr_om_header.msgsn = OM_CHR_STUB_SN;
    app_ctrl_status.chr_om_header.msglen = sizeof(unsigned int) + sizeof(unsigned int);
    app_ctrl_status.moduleid = 0x01;
    app_ctrl_status.modemid = modem_id;
    app_ctrl_status.faultid = fault_id;

    ret =
        chr_acpu_read_vcom_data(APP_VCOM_DEV_INDEX_ERRLOG, (unsigned char *)&app_ctrl_status, sizeof(chr_app_om_req_s));
    if (ret == VOS_ERR) {
        chr_print("read vcom data failed!\n");
    }
    return ret;
}
/*
 * 功能描述: 测试桩函数:模拟AP下发黑名单配置
 */
int chr_acpu_cfg_block_list_test(unsigned int msg_module_id, unsigned short alarm_id, unsigned short alarm_type)
{
    int ret;
    VOS_UINT alloc_size;

    chr_app_block_list_s *app_block_list = VOS_NULL_PTR;
    chr_list_info_s *block_list_info = VOS_NULL_PTR;
    unsigned char *block_list_cfg = VOS_NULL_PTR;
    alloc_size = sizeof(chr_app_block_list_s) + sizeof(chr_list_info_s);

    block_list_cfg = (unsigned char *)VOS_MemAlloc(MSP_PID_CHR, DYNAMIC_MEM_PT, alloc_size);
    if (block_list_cfg == VOS_NULL_PTR) {
        chr_print("failed to malloc\n");
        return VOS_ERR;
    }
    if (memset_s(block_list_cfg, alloc_size, 0, alloc_size)) {
        chr_print("memset failed!\n");
    }

    app_block_list = (chr_app_block_list_s *)block_list_cfg;

    app_block_list->chr_om_header.msgtype = OM_ERR_LOG_MSG_BLOCK_LIST;
    app_block_list->chr_om_header.msgsn = 0;
    app_block_list->chr_om_header.msglen = sizeof(chr_list_info_s);

    block_list_info = (chr_list_info_s *)(app_block_list->block_list);

    block_list_info->moduleid = msg_module_id;
    block_list_info->alarmid = alarm_id;
    block_list_info->alarm_type = alarm_type;

    ret = chr_acpu_read_vcom_data(APP_VCOM_DEV_INDEX_ERRLOG, block_list_cfg, alloc_size);
    if (ret == VOS_ERR) {
        chr_print("read vcom data failed!\n");
        (void)VOS_MemFree(MSP_PID_CHR, block_list_cfg);
        return VOS_ERR;
    }
    (void)VOS_MemFree(MSP_PID_CHR, block_list_cfg);

    return ret;
}
/*
 * 功能描述: 测试桩函数:模拟AP下发高优先级
 */
int chr_acpu_cfg_priority_test(unsigned int msg_module_id, unsigned short alarm_id, unsigned char sn,
    unsigned short alarm_type)
{
    int ret;
    VOS_UINT alloc_size;

    chr_app_priority_cfg_s *app_priority_list = VOS_NULL_PTR;
    chr_priority_info_s *priority_info = VOS_NULL_PTR;
    unsigned char *app_priority_cfg = VOS_NULL_PTR;

    alloc_size = sizeof(chr_app_priority_cfg_s) + sizeof(chr_priority_info_s);
    app_priority_cfg = (unsigned char *)VOS_MemAlloc(MSP_PID_CHR, DYNAMIC_MEM_PT, alloc_size);
    if (app_priority_cfg == VOS_NULL_PTR) {
        chr_print("failed to malloc\n");
        return VOS_ERR;
    }
    if (memset_s(app_priority_cfg, alloc_size, 0, alloc_size)) {
        chr_print("memset failed!\n");
    }

    app_priority_list = (chr_app_priority_cfg_s *)app_priority_cfg;

    app_priority_list->chr_om_header.msgtype = OM_ERR_LOG_MSG_PRIORITY_CFG;
    app_priority_list->chr_om_header.msgsn = 0;
    app_priority_list->chr_om_header.msglen = alloc_size - sizeof(chr_alarm_msg_head_s);
    app_priority_list->count = 1;
    app_priority_list->packt_sn = sn;

    priority_info = (chr_priority_info_s *)(app_priority_list->prior_map);

    priority_info->moduleid = msg_module_id;
    priority_info->alarmid = alarm_id;
    priority_info->alarm_type = alarm_type;

    ret = chr_acpu_read_vcom_data(APP_VCOM_DEV_INDEX_ERRLOG, (unsigned char *)app_priority_cfg, alloc_size);
    if (ret == VOS_ERR) {
        chr_print("read vcom data failed!\n");
        (void)VOS_MemFree(MSP_PID_CHR, app_priority_cfg);
        return VOS_ERR;
    }
    (void)VOS_MemFree(MSP_PID_CHR, app_priority_cfg);

    return ret;
}
/*
 * 功能描述: 测试桩函数:模拟AP下发上报周期配置
 */
int chr_acpu_cfg_period_test(unsigned int msg_module_id, unsigned int count, unsigned char sn)
{
    int ret;
    VOS_UINT alloc_size;

    chr_app_period_cfg_s *app_period_list = VOS_NULL_PTR;
    chr_period_cfg_s *period_info = VOS_NULL_PTR;
    unsigned char *app_period_cfg = VOS_NULL_PTR;

    alloc_size = sizeof(chr_app_period_cfg_s) + sizeof(chr_period_cfg_s);
    app_period_cfg = (unsigned char *)VOS_MemAlloc(MSP_PID_CHR, DYNAMIC_MEM_PT, alloc_size);
    if (app_period_cfg == VOS_NULL_PTR) {
        chr_print("failed to malloc\n");
        return VOS_ERR;
    }
    if (memset_s(app_period_cfg, alloc_size, 0, alloc_size)) {
        chr_print("memset failed!\n");
    }

    app_period_list = (chr_app_period_cfg_s *)app_period_cfg;

    app_period_list->chr_om_header.msgtype = OM_ERR_LOG_MSG_PERIOD_CFG;
    app_period_list->chr_om_header.msgsn = 0;
    app_period_list->chr_om_header.msglen = alloc_size - sizeof(chr_alarm_msg_head_s);
    app_period_list->count = 1;
    app_period_list->msg_sn = sn;
    app_period_list->period = 1;

    period_info = (chr_period_cfg_s *)(app_period_list->period_map);

    period_info->moduleid = msg_module_id;
    period_info->count = count;

    ret = chr_acpu_read_vcom_data(APP_VCOM_DEV_INDEX_ERRLOG, (unsigned char *)app_period_cfg, alloc_size);
    if (ret == VOS_ERR) {
        chr_print("read vcom data failed!\n");
        (void)VOS_MemFree(MSP_PID_CHR, app_period_cfg);
        return VOS_ERR;
    }
    (void)VOS_MemFree(MSP_PID_CHR, app_period_cfg);

    return ret;
}

/* 设置采集信息回复消息不需要处理标志 */
void chr_acpu_clt_info_no_proc_set(void)
{
    g_chr_acpu_clt_info_no_proc = OM_CLTINFO_CNF_NOT_NEED_PROCESS;
}

/* 复位采集信息回复消息不需要处理标志 */
void chr_acpu_clt_info_no_proc_reset(void)
{
    g_chr_acpu_clt_info_no_proc = OM_CLTINFO_CNF_NEED_PROCESS;
}
/* 获取采集信息回复消息不需要处理标志 */
unsigned int chr_acpu_clt_cnf_get_no_proc_flag(void)
{
    return g_chr_acpu_clt_info_no_proc;
}

/* 通过modemID获对应的MTA的pid ，当前只将请求发给MTA */
unsigned int chr_acpu_clt_get_mta_pid(unsigned int modem_id)
{
    if (modem_id == 0)
        return I0_UEPS_PID_MTA;
    else if (modem_id == OM_CHR_MODEM_ID_1)
        return I1_UEPS_PID_MTA;
    else if (modem_id == OM_CHR_MODEM_ID_2)
        return I2_UEPS_PID_MTA;
    else
        return OM_CLTINFO_INVALID_PID;
}

/* OM收到采集信息请求消息处理 */
int chr_acpu_clt_req_msg_proc(unsigned char *data, unsigned int msg_len)
{
    chr_app_info_clt_req_s *app_clt_info = NULL;
    chr_info_clt_report_req_s *clt_report_req = NULL;
    VOS_UINT alloc_size;

    if (data == VOS_NULL_PTR) {
        return VOS_ERR;
    }
    if (msg_len < sizeof(chr_app_info_clt_req_s) - sizeof(app_clt_info->msgdata)) {
        return VOS_ERR;
    }
    app_clt_info = (chr_app_info_clt_req_s *)data;
    if (msg_len - (sizeof(chr_app_info_clt_req_s) - sizeof(app_clt_info->msgdata)) < app_clt_info->msglen) {
        return VOS_ERR;
    }

    alloc_size =
        sizeof(chr_info_clt_report_req_s) - VOS_MSG_HEAD_LENGTH - sizeof(app_clt_info->msgdata) + app_clt_info->msglen;

    clt_report_req = (chr_info_clt_report_req_s *)VOS_AllocMsg(MSP_PID_CHR, alloc_size);
    if (clt_report_req == VOS_NULL_PTR) {
        chr_print("pVOSCltInfo is null!\n");
        return VOS_ERR;
    }

    clt_report_req->ulReceiverPid = chr_acpu_clt_get_mta_pid(app_clt_info->modemid);

    if (clt_report_req->ulReceiverPid == OM_CLTINFO_INVALID_PID) {
        chr_print("chr_acpu_clt_get_mta_pid failed\n");
        (void)VOS_FreeMsg(MSP_PID_CHR, clt_report_req);
        return OM_APP_SEND_MODEM_ID_ERR;
    }
    clt_report_req->msgname = ID_OM_INFO_CLT_REPORT_REQ;
    clt_report_req->modemid = app_clt_info->modemid;
    clt_report_req->scene_type = app_clt_info->infoid;
    clt_report_req->msg_len = app_clt_info->msglen;
    if (memcpy_s(clt_report_req->data, clt_report_req->msg_len, app_clt_info->msgdata, clt_report_req->msg_len)) {
        chr_print("memcpy failed!\n");
    }

    (void)VOS_SendMsg(MSP_PID_CHR, clt_report_req);
    chr_acpu_clt_info_no_proc_reset();
    g_chr_app_msg_record.modem_id = app_clt_info->modemid;

    /* 起5s定时器 */
    g_chr_acpu_clt_info_tmr = VOS_NULL_PTR;
    if (VOS_OK != VOS_StartRelTimer(&g_chr_acpu_clt_info_tmr, MSP_PID_CHR, OM_CLTINFO_TIMER_LENTH,
        OM_CLTINFO_TIMER_NAME, OM_CLTINFO_TIMER_PARA, VOS_RELTIMER_NOLOOP, VOS_TIMER_PRECISION_5)) {
        chr_print("VOS_StartRelTimer failed.\n");
        return OM_APP_ERRLOG_START_TIMER_ERR;
    }

    return VOS_OK;
}

/* 采集信息回复消息结束消息，msgType 0x11 */
void chr_acpu_recv_clt_finish(void)
{
    chr_app_report_status_s app_report_status = {};

    app_report_status.chr_om_header.msgtype = OM_ERR_LOG_MSG_INFO_CLT_END;
    app_report_status.chr_om_header.msgsn = g_chr_vcom_debug_info.vcom_send_sn++;
    app_report_status.chr_om_header.msglen = (sizeof(chr_app_report_status_s) - sizeof(chr_alarm_msg_head_s));
    app_report_status.modemid = g_chr_app_msg_record.modem_id;
    app_report_status.report_status = 0;
    app_report_status.faultid = g_chr_app_msg_record.fault_id;
    app_report_status.low_slice = (unsigned int)(g_chr_trig_time_64bit & 0xffffffff);
    app_report_status.high_slice =
        (unsigned int)((g_chr_trig_time_64bit >> (sizeof(int) * OM_UCHAR_BIT_NUMS)) & 0xffffffff);

    chr_acpu_send_vcom_data((unsigned char *)&app_report_status, sizeof(chr_app_report_status_s));

    return;
}

/* 采集信息下发请求消息打桩函数，用于测试 */
int chr_acpu_report_clt_info(unsigned short modem_id, unsigned short info_id, unsigned int mcc, unsigned int mnc)
{
    chr_app_info_clt_req_s *clt_info_req = NULL;
    chr_info_clk_plmn_id_s *clt_info_plmn = NULL;

    int ret;
    void *clt_info_alloc = NULL;
    unsigned int alloc_size = sizeof(chr_app_info_clt_req_s) + sizeof(unsigned int); /* mcc/mnc 共8字节 */

    clt_info_alloc = VOS_MemAlloc(MSP_PID_CHR, DYNAMIC_MEM_PT, alloc_size);
    if (clt_info_alloc == NULL) {
        chr_print("failed to malloc\n");
        return VOS_ERR;
    }

    if (memset_s(clt_info_alloc, alloc_size, 0, alloc_size)) {
        chr_print("memset failed!\n");
    }

    clt_info_req = (chr_app_info_clt_req_s *)clt_info_alloc;
    clt_info_req->chr_om_header.msgtype = 0x10;
    clt_info_req->chr_om_header.msgsn = OM_CHR_STUB_SN;
    clt_info_req->chr_om_header.msglen = sizeof(unsigned int) + sizeof(unsigned int) +
                                         sizeof(unsigned int) + sizeof(unsigned int);

    clt_info_req->modemid = modem_id;
    clt_info_req->infoid = info_id;
    clt_info_req->msglen = sizeof(unsigned int) + sizeof(unsigned int);

    clt_info_plmn = (chr_info_clk_plmn_id_s *)(clt_info_req->msgdata);
    clt_info_plmn->mcc = mcc;
    clt_info_plmn->mnc = mnc;
    ret = chr_acpu_read_vcom_data(APP_VCOM_DEV_INDEX_ERRLOG, (unsigned char *)clt_info_alloc, alloc_size);
    if (ret == VOS_ERR) {
        (void)VOS_MemFree(MSP_PID_CHR, clt_info_alloc);
        chr_print("read vcom data failed!\n");
        return VOS_ERR;
    }
    (void)VOS_MemFree(MSP_PID_CHR, clt_info_alloc);
    return ret;
}


/*
 * 功能描述: 打印超时之后上报的PID
 */
void chr_acpu_show_timeout_pid(void)
{
    unsigned int i;

    /* 记录对应组件已上报消息 */
    for (i = 0; i < g_chr_app_msg_record.alarm_id_num; i++) {
        if (chr_acpu_chk_rpt_flag(i)) {
            (void)chr_print_dbg("Pid = 0x%x, AlarmId = 0x%x!\n", g_chr_app_msg_record.fault_cfg.aucAlarmMap[i].ulPid,
                g_chr_app_msg_record.fault_cfg.aucAlarmMap[i].ulAlarmId);
        }
    }
    return;
}

/*
 * 功能描述: 超时之后的处理函数
 */
void chr_acpu_timeout_proc(void)
{
    chr_acpu_show_timeout_pid();
    chr_acpu_rcv_msg_finish();
    (void)chr_print_dbg("OM not receive all msg, timer is on!\n");

    return;
}
/*
 * 功能描述: 把从ap接收的查询命令数据发送到chr acore任务
 */
int chr_acpu_req_proc(unsigned char *data, unsigned int msg_len)
{
    chr_app_req_s *app_req_msg;

    /* 给对应的PID发送消息 */
    app_req_msg =
        (chr_app_req_s *)VOS_AllocMsg(MSP_PID_CHR, (((sizeof(chr_app_req_s)) - VOS_MSG_HEAD_LENGTH) + msg_len));
    if (app_req_msg == VOS_NULL_PTR) {
        (void)chr_print("Alloc msg failed!\n");
        return OM_APP_MEM_ALLOC_MSG_ERR;
    }
    app_req_msg->ulReceiverPid = MSP_PID_CHR;
    app_req_msg->msg_name = CHR_ID_ERR_LOG_REQ;
    app_req_msg->msg_len = msg_len;

    if (memcpy_s(app_req_msg->data, msg_len, data, msg_len)) {
        chr_print("memcpy failed!\n");
    }
    (void)VOS_SendMsg(MSP_PID_CHR, app_req_msg);

    (void)chr_print_dbg("success!\n ");
    return VOS_OK;
}
/*
 * 功能描述: 把从ap接收的黑名单配置数据发送到chr acore任务
 */
int chr_acpu_block_list_req_proc(unsigned char *data, unsigned int msg_len)
{
    chr_app_req_s *app_req_msg;

    /* 给对应的PID发送消息 */
    app_req_msg =
        (chr_app_req_s *)VOS_AllocMsg(MSP_PID_CHR, (((sizeof(chr_app_req_s)) - VOS_MSG_HEAD_LENGTH) + msg_len));
    if (app_req_msg == VOS_NULL_PTR) {
        chr_print("alloc mem fail!\n");
        return OM_APP_MEM_ALLOC_MSG_ERR;
    }
    app_req_msg->ulReceiverPid = MSP_PID_CHR;
    app_req_msg->msg_name = CHR_ID_BLOCKLIST_REQ;
    app_req_msg->msg_len = msg_len;

    if (memcpy_s(app_req_msg->data, msg_len, data, msg_len)) {
        chr_print("memcpy failed!\n");
    }
    (void)VOS_SendMsg(MSP_PID_CHR, app_req_msg);

    (void)chr_print_dbg("success!\n ");
    return VOS_OK;
}
/*
 * 功能描述: 把从ap接收的高优先级配置数据发送到chr acore任务
 */
int chr_acpu_priority_req_proc(unsigned char *data, unsigned int msg_len)
{
    chr_app_req_s *app_req_msg;

    /* 给对应的PID发送消息 */
    app_req_msg =
        (chr_app_req_s *)VOS_AllocMsg(MSP_PID_CHR, (((sizeof(chr_app_req_s)) - VOS_MSG_HEAD_LENGTH) + msg_len));
    if (app_req_msg == VOS_NULL_PTR) {
        return OM_APP_MEM_ALLOC_MSG_ERR;
    }
    app_req_msg->ulReceiverPid = MSP_PID_CHR;
    app_req_msg->msg_name = CHR_ID_PRIORITY_REQ;
    app_req_msg->msg_len = msg_len;

    if (memcpy_s(app_req_msg->data, msg_len, data, msg_len)) {
        chr_print("memcpy failed!\n");
    }
    (void)VOS_SendMsg(MSP_PID_CHR, app_req_msg);

    (void)chr_print_dbg("success!\n ");
    return VOS_OK;
}
/*
 * 功能描述: 把从ap接收的上报周期配置数据发送到chr acore任务
 */
int chr_acpu_period_req_proc(unsigned char *data, unsigned int msg_len)
{
    chr_app_req_s *app_req_msg;

    /* 给对应的PID发送消息 */
    app_req_msg =
        (chr_app_req_s *)VOS_AllocMsg(MSP_PID_CHR, (((sizeof(chr_app_req_s)) - VOS_MSG_HEAD_LENGTH) + msg_len));
    if (app_req_msg == VOS_NULL_PTR) {
        return OM_APP_MEM_ALLOC_MSG_ERR;
    }
    app_req_msg->ulReceiverPid = MSP_PID_CHR;
    app_req_msg->msg_name = CHR_ID_PERIOD_REQ;
    app_req_msg->msg_len = msg_len;

    if (memcpy_s(app_req_msg->data, msg_len, data, msg_len)) {
        chr_print("memcpy failed!\n");
    }
    (void)VOS_SendMsg(MSP_PID_CHR, app_req_msg);

    (void)chr_print_dbg("success!\n ");
    return VOS_OK;
}


int chr_msg_dispatch(unsigned char *data, unsigned int msg_len)
{
    chr_alarm_msg_head_s *alarm_msg_head = (chr_alarm_msg_head_s *)data;
    if (alarm_msg_head->msgtype == OM_ERR_LOG_MSG_SET_ERR_REPORT) {
        up(&g_chr_rx_buff_sem);
        return chr_acpu_req_proc(data, msg_len);
    } else if (alarm_msg_head->msgtype == OM_ERR_LOG_MSG_INFO_CLT_REQ) {
        up(&g_chr_rx_buff_sem);
        return chr_acpu_clt_req_msg_proc(data, msg_len);
    } else if (alarm_msg_head->msgtype == OM_ERR_LOG_MSG_BLOCK_LIST) {
        up(&g_chr_rx_buff_sem);
        return chr_acpu_block_list_req_proc(data, msg_len);
    } else if (alarm_msg_head->msgtype == OM_ERR_LOG_MSG_PRIORITY_CFG) {
        up(&g_chr_rx_buff_sem);
        return chr_acpu_priority_req_proc(data, msg_len);
    } else if (alarm_msg_head->msgtype == OM_ERR_LOG_MSG_PERIOD_CFG) {
        up(&g_chr_rx_buff_sem);
        return chr_acpu_period_req_proc(data, msg_len);
    } else {
        g_chr_vcom_debug_info.vcom_rcv_err_num++;
        g_chr_vcom_debug_info.vcom_rcv_err_len += msg_len;

        (void)chr_print("Recv msg type is wrong\n");

        chr_acpu_send_app_result(OM_APP_MSG_TYPE_ERR, MODEM_ID_BUTT);
        up(&g_chr_rx_buff_sem);
        return VOS_ERR;
    }
}

/*
 * 功能描述: NAS把收到数据调用OM 接口读走
 */
int chr_acpu_read_vcom_data(unsigned char dev_index, unsigned char *data, unsigned int msg_len)
{
    chr_alarm_msg_head_s *alarm_msg_head = NULL;
    COVERITY_TAINTED_SET(data);
    if (data == NULL) {
        (void)chr_print("data is null\n");
        return VOS_ERR;
    }

    if (dev_index != APP_VCOM_DEV_INDEX_ERRLOG) {
        (void)chr_print("PhyPort  is wrong. devindex = %d\n", dev_index);
        return VOS_ERR;
    }

    down(&g_chr_rx_buff_sem);

    if (msg_len > OM_ERRLOG_MAX_CFG_LEN) {
        chr_print("len too long\n");
        return VOS_ERR;
    }

    if (msg_len < sizeof(chr_alarm_msg_head_s)) {
        (void)chr_print(" len is too short. len = %d.\n", msg_len);

        up(&g_chr_rx_buff_sem);

        return VOS_ERR;
    }

    /* 根据消息头判断命令类型 */
    alarm_msg_head = (chr_alarm_msg_head_s *)data;
    if (msg_len != alarm_msg_head->msglen + sizeof(chr_alarm_msg_head_s)) {
        (void)chr_print(" len not match. len = %d.\n", msg_len);
        up(&g_chr_rx_buff_sem);

        return VOS_ERR;
    }

    chr_log_report("receive msg. Msgtype:0x%x,MsgLen:0x%x,len:0x%x!\n", alarm_msg_head->msgtype, alarm_msg_head->msglen,
        msg_len);

    /* 串口打印码流 */
    OM_ACPU_CHR_DEBUG_TRACE((unsigned char *)data, msg_len, OM_ACPU_ERRLOG_RCV);

    /* 增加Trans勾包 */
    chr_acpu_msg_hook(data, msg_len, OM_ERRLOG_RCV_MSG);

    g_chr_vcom_debug_info.vcom_rcv_num++;
    g_chr_vcom_debug_info.vcom_rcv_len += msg_len;
    g_chr_vcom_debug_info.vcom_rcv_sn = alarm_msg_head->msglen;

    return chr_msg_dispatch(data, msg_len);
}

/*
 * 功能描述: OM收到各业务模块上报消息处理
 */
void chr_acpu_msg_proc(MsgBlock *msg_block)
{
    chr_recv_report_s *recv_data_info;

    recv_data_info = (chr_recv_report_s *)msg_block;

    /* 判断是否是主动上报或者被动上报 */
    if ((recv_data_info->chr_rcv_header.msgname != ID_OM_ERR_LOG_REPORT_CNF) &&
        (recv_data_info->chr_rcv_header.msgname != ID_OM_FAULT_ERR_LOG_IND) &&
        (recv_data_info->chr_rcv_header.msgname != ID_OM_ALARM_ERR_LOG_IND)) {
        chr_log_report("not chr Log msg.\n");
        return;
    }

    /*  收到消息检查是否需要上报 */
    if (chr_acpu_rcv_msg_check(recv_data_info) != VOS_OK) {
        chr_log_report("rcv msg wrong!\r\n ");
        return;
    }

    recv_data_info->chr_type_union.chr_type.msg_type_short = recv_data_info->chr_type_union.msg_type & 0xffff;
    recv_data_info->chr_type_union.chr_type.fault_id = recv_data_info->chr_rcv_header.faultid;

    chr_acpu_send_vcom_data((unsigned char *)&recv_data_info->chr_type_union.msg_type,
        recv_data_info->rpt_len + sizeof(chr_alarm_msg_head_s));

    chr_log_report(
        "rcv msg Send Success,Msgname 0x%x, Type 0x%x, AlarmTyep 0x%x, Alarmid 0x%x,Pid 0x%x, Cnt 0x%x,modulid 0x%x\n",
        recv_data_info->chr_rcv_header.msgname, recv_data_info->chr_type_union.chr_type.msg_type_short,
        recv_data_info->chr_rcv_header.alarm_type, recv_data_info->chr_rcv_header.alarmid,
        recv_data_info->chr_rcv_header.ulSenderPid, g_chr_app_msg_record.report_need,
        recv_data_info->chr_rcv_header.moduleid);

    chr_print_dbg("rcv msg Alarmid 0x%x,Pid 0x%x, modulid 0x%x\n", recv_data_info->chr_rcv_header.alarmid,
        recv_data_info->chr_rcv_header.ulSenderPid, recv_data_info->chr_rcv_header.moduleid);

    if ((recv_data_info->chr_type_union.chr_type.msg_type_short == OM_ERR_LOG_MSG_ERR_REPORT) &&
        (g_chr_app_msg_record.report_need == OM_AP_SEND_MSG_FINISH)) {
        /* 停定时器 */
        (void)VOS_StopRelTimer(&g_chr_acpu_full_tmr);

        chr_acpu_rcv_msg_finish();
        chr_log_report("Report all msg, Stop timer\n");
    }

    return;
}

/*
 * 功能描述  : OM采集信息回复消息处理函数
 */
/* 采集信息回复消息处理函数 */
void chr_acpu_clt_cnf_msg_proc(MsgBlock *msg_block)
{
    chr_info_clt_report_cnf_s *clt_info_cnf = NULL;
    unsigned short send_size;

    clt_info_cnf = (chr_info_clt_report_cnf_s *)msg_block;
    if ((!clt_info_cnf) || (clt_info_cnf->msg_name != ID_OM_INFO_CLT_REPORT_CNF)) {
        return;
    }

    clt_info_cnf->msg_sn = g_chr_vcom_debug_info.vcom_send_sn++;
    send_size = clt_info_cnf->rpt_len + sizeof(chr_alarm_msg_head_s);

    if (chr_acpu_clt_cnf_get_no_proc_flag() == OM_CLTINFO_CNF_NEED_PROCESS) {
        chr_acpu_send_vcom_data((unsigned char *)&clt_info_cnf->msg_type, send_size);

        if (clt_info_cnf->msg_type == OM_ERR_LOG_MSG_INFO_CLT_CNF) {
            /* 停定时器 */
            VOS_StopRelTimer(&g_chr_acpu_clt_info_tmr);
            chr_acpu_recv_clt_finish();
        }
    }

    return;
}
/*
 * 功能描述: OM收到查询上报请求的处理函数
 */
void chr_acpu_recv_msg_proc(MsgBlock *msg_block)
{
    unsigned int result;
    chr_app_req_s *app_req_msg = (chr_app_req_s *)msg_block;

    result = chr_acpu_report_msg(app_req_msg->data, app_req_msg->msg_len);
    if (result != OM_APP_MSG_OK) {
        chr_acpu_send_app_result(result, MODEM_ID_BUTT);
        chr_print("errlog msg failed. err code = 0x%x!\n", result);
        return;
    }

    return;
}
/*
 * 功能描述: OM收到黑名单请求的处理函数
 */
void chr_acpu_block_list_recv_msg_proc(MsgBlock *msg_block)
{
    unsigned int ret;
    chr_app_req_s *app_req = (chr_app_req_s *)msg_block;

    ret = chr_acpu_block_list_proc(app_req->data, app_req->msg_len);
    if (ret != OM_APP_MSG_OK) {
        chr_acpu_send_app_cfg_result(ret);
        chr_print("blocklist msg failed. err code = 0x%x!\n", ret);
        return;
    }
    /* 先回复一个结果给apk，释放vcom口 */
    chr_acpu_send_app_cfg_result(OM_APP_MSG_OK);
    return;
}
/*
 * 功能描述: OM收到高优先级配置请求的处理函数
 */
void chr_acpu_priority_recv_msg_proc(MsgBlock *msg_block)
{
    unsigned int ret;
    chr_app_req_s *app_req = (chr_app_req_s *)msg_block;

    ret = chr_acpu_priority_cfg_proc(app_req->data, app_req->msg_len);
    if (ret != OM_APP_MSG_OK) {
        chr_acpu_send_app_cfg_result(ret);
        chr_print("prio msg failed. err code = 0x%x!\n", ret);
        return;
    }
    /* 先回复一个结果给apk，释放vcom口 */
    chr_acpu_send_app_cfg_result(OM_APP_MSG_OK);
    return;
}
/*
 * 功能描述: OM收到上报周期配置请求的处理函数
 */
void chr_acpu_period_recv_msg_proc(MsgBlock *msg_block)
{
    unsigned int ret;
    chr_app_req_s *app_req = (chr_app_req_s *)msg_block;

    ret = chr_acpu_period_cfg_proc(app_req->data, app_req->msg_len);
    if (ret != OM_APP_MSG_OK) {
        chr_acpu_send_app_cfg_result(ret);
        chr_print("period msg failed. err code = 0x%x!\n", ret);
        return;
    }
    /* 先回复一个结果给apk，释放vcom口 */
    chr_acpu_send_app_cfg_result(OM_APP_MSG_OK);

    return;
}
/*
 * 功能描述: OM收到各业务模块上报消息处理
 */
void chr_acpu_chr_msg_proc(MsgBlock *msg_block)
{
    REL_TimerMsgBlock *msg_timer = NULL;
    chr_app_req_s *req_msg = NULL;
    COVERITY_TAINTED_SET((void *)(msg_block->value));
    /* 入参判断 */
    if (msg_block == NULL) {
        return;
    }

    /* 根据发送PID，执行不同处理 */
    switch (msg_block->ulSenderPid) {
        /* 超时消息，按照超时包格式，打包回复 */
        case DOPRA_PID_TIMER:
            msg_timer = (REL_TimerMsgBlock *)msg_block;

            if ((msg_timer->name == OM_ERRORLOG_TIMER_NAME) && (msg_timer->para == OM_ERRORLOG_TIMER_PARA)) {
                chr_acpu_timeout_proc();
            } else if ((msg_timer->name == OM_CLTINFO_TIMER_NAME) && (msg_timer->para == OM_CLTINFO_TIMER_PARA)) {
                chr_acpu_clt_info_no_proc_set();
                chr_acpu_recv_clt_finish();
            }
            break;
        case MSP_PID_CHR:
            req_msg = (chr_app_req_s *)msg_block;
            if (req_msg->msg_name == CHR_ID_ERR_LOG_REQ) {
                chr_acpu_recv_msg_proc(msg_block);
            } else if (req_msg->msg_name == CHR_ID_BLOCKLIST_REQ) {
                chr_acpu_block_list_recv_msg_proc(msg_block);
            } else if (req_msg->msg_name == CHR_ID_PRIORITY_REQ) {
                chr_acpu_priority_recv_msg_proc(msg_block);
            } else if (req_msg->msg_name == CHR_ID_PERIOD_REQ) {
                chr_acpu_period_recv_msg_proc(msg_block);
            } else if (req_msg->msg_name == CHR_ID_RESET_CCORE) {
                chr_acpu_reset_proc();
            }
            break;
        default: {
            chr_acpu_msg_proc(msg_block);
            chr_acpu_clt_cnf_msg_proc(msg_block);
        } break;
    }

    return;
}

/*
 * 功能描述: 用于ErrLog Vcom 口通道的初始化
 */
unsigned int chr_acpu_init(enum VOS_InitPhaseDefine ip)
{
    unsigned int ret = 0;

    if (ip == VOS_IP_LOAD_CONFIG) {
        /* 商用ERR LOG上报全局变量初始化 */
        g_chr_app_msg_record.report_need = 0; /* 记录Err Log需要上报组件 */

        g_chr_app_msg_record.modem_id = MODEM_ID_BUTT;
        g_chr_app_msg_record.state = ERRLOG_IDLE;
        g_chr_app_msg_record.msg_sn = 0;

        sema_init(&g_chr_rx_buff_sem, 1);

        /* 注册收Vcom Error log函数给NAS */
        ret = APP_VCOM_REG_DATA_CALLBACK(APP_VCOM_DEV_INDEX_ERRLOG, (SEND_UL_AT_FUNC)chr_acpu_read_vcom_data);
        if (ret != VOS_OK) {
            chr_print("RegDataCallback failed\n");
        }

        ret = chr_cfg_init();
        if (ret != VOS_OK) {
            chr_print("chr init failed\n");
        }

        ret = (unsigned int)mdrv_chr_drv_register_callback(chr_get_drv_data);
        if (ret != VOS_OK) {
            (void)chr_print("CHR register DRV ret %x\n", ret);
            return VOS_ERR;
        }
    }

    return ret;
}

/*
 * 功能描述: 用于打印 Error log 调测信息
 */
void chr_acpu_info_show(void)
{
    (void)chr_print("VCom Send sn is           %d\n", g_chr_vcom_debug_info.vcom_send_sn);
    (void)chr_print("VCom Send num is           %d\n", g_chr_vcom_debug_info.vcom_send_num);
    (void)chr_print("VCom Send Len is           %d\n", g_chr_vcom_debug_info.vcom_send_len);
    (void)chr_print("VCom Send Error num is         %d\n", g_chr_vcom_debug_info.vcom_send_err_num);
    (void)chr_print("VCom Send Error Len is         %d\n", g_chr_vcom_debug_info.vcom_send_err_len);
    (void)chr_print("VCom receive sn is           %d\n", g_chr_vcom_debug_info.vcom_rcv_sn);
    (void)chr_print("VCom receive num is           %d\n", g_chr_vcom_debug_info.vcom_rcv_num);
    (void)chr_print("VCom receive Len is           %d\n", g_chr_vcom_debug_info.vcom_rcv_len);
    (void)chr_print("VCom receive Error num is         %d\n", g_chr_vcom_debug_info.vcom_rcv_err_num);
    (void)chr_print("VCom receive Error Len is         %d\n", g_chr_vcom_debug_info.vcom_rcv_err_len);
    (void)chr_print("OM Send Err log msg num is         %d\n", g_chr_recv_ue_send_ap.send_num);
    (void)chr_print("OM Send Err log msg len is         %d\n", g_chr_recv_ue_send_ap.send_len);

    return;
}
EXPORT_SYMBOL(chr_acpu_report_msg_test);
EXPORT_SYMBOL(chr_acpu_report_clt_info);
EXPORT_SYMBOL(chr_acpu_info_show);
EXPORT_SYMBOL(chr_open_log);
EXPORT_SYMBOL(chr_acpu_cfg_block_list_test);
EXPORT_SYMBOL(chr_acpu_cfg_priority_test);
EXPORT_SYMBOL(chr_acpu_cfg_period_test);
#endif
/*
 * 功能描述: CHR fid初始化函数
 */
unsigned int msp_app_chr_fid_init(enum VOS_InitPhaseDefine ip)
{
#if (FEATURE_ON == FEATURE_CHR_OM)
    unsigned int ret_val = 0;
    switch (ip) {
        case VOS_IP_LOAD_CONFIG:
            ret_val = VOS_RegisterPIDInfo(MSP_PID_CHR, (InitFunType)chr_acpu_init, (MsgFunType)chr_acpu_chr_msg_proc);
            if (ret_val != VOS_OK) {
                return VOS_ERR;
            }

            ret_val = VOS_RegisterMsgTaskPrio(MSP_FID_CHR_ACPU, VOS_PRIORITY_M2);
            if (ret_val != VOS_OK) {
                return VOS_ERR;
            }
            break;
        default:
            break;
    }
#endif
    return VOS_OK;
}
unsigned int chr_get_drv_data(void *data, unsigned int msg_len)
{
#if (FEATURE_ON == FEATURE_CHR_OM)
    chr_drv_ind_s *recv_data_info;

    recv_data_info = (chr_drv_ind_s *)data;

    chr_acpu_send_vcom_data((unsigned char *)&recv_data_info->msg_type,
        recv_data_info->msglen + sizeof(chr_alarm_msg_head_s));
#endif
    return VOS_OK;
}

