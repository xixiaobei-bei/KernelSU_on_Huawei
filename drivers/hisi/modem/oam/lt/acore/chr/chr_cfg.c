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

#include "vos.h"
#include "v_msg.h"
#include <securec.h>
#include "chr_cfg.h"
#include "chr_om.h"
#include <mdrv_diag_system.h>
#include "msp_diag.h"

#define THIS_FILE_ID PS_FILE_ID_OM_ERRORLOG_C

#undef THIS_MODU
#define THIS_MODU "mdm_chr"

#if (FEATURE_ON == FEATURE_CHR_OM)
/* 保存黑名单的全局变量 */
chr_acpu_block_save_s g_chr_block_cfg;

/* 保存优先级列表的全局变量 */
chr_acpu_priority_save_s *g_chr_priority_cfg;  // 保存优先级0的配置表
chr_priority_list_s g_chr_priority_list;         // 临时记录AP下发的优先级多包数据

/* 保存上报周期的全局变量 */
chr_acpu_period_save_s *g_chr_period_cfg;  // 保存上报周期的配置表
chr_period_list_s g_chr_period_list;         // 临时记录AP下发的周期多包数据
mdrv_ppm_vcom_debug_info_s g_chr_cfg_vcom_debug_info = {0};
#define chr_log_cfg_report(fmt, ...)                                                                                     \
    mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(0, 0, 1), MSP_PID_CHR, "chr_cfg", __LINE__, "%s:" fmt, __FUNCTION__, \
                         ##__VA_ARGS__)

/*
 * 功能描述: APP下发给OM的黑名单处理函数
 */
unsigned int chr_acpu_block_list_send(unsigned int recv_pid)
{
    unsigned int block_list_len;
    chr_acpu_block_list_s *chr_acpu_block_list = NULL;

    if (recv_pid != MSP_PID_CHR_MDM && recv_pid != MSP_PID_CHR_NR) {
        chr_print("recv pid Req Msg failed!\n");
        return OM_APP_PARAM_INAVALID;
    }

    /* A核申请黑名单列表消息发送给c核 */
    block_list_len = g_chr_block_cfg.packet_len + sizeof(chr_acpu_block_list_s) - VOS_MSG_HEAD_LENGTH;
    chr_acpu_block_list = (chr_acpu_block_list_s *)VOS_AllocMsg(MSP_PID_CHR, block_list_len);
    if (VOS_NULL_PTR == chr_acpu_block_list) {
        chr_print("Alloc Req Msg failed!\n");
        return OM_APP_MEM_ALLOC_MSG_ERR;
    }

    chr_acpu_block_list->ulReceiverPid = recv_pid;
    chr_acpu_block_list->packet_len = g_chr_block_cfg.packet_len;
    chr_acpu_block_list->msg_name = OM_ACPU_REPORT_BLOCKLIST_NAME;

    if (memcpy_s(&chr_acpu_block_list->block_list, chr_acpu_block_list->packet_len,
                 g_chr_block_cfg.chr_block_list, g_chr_block_cfg.packet_len)) {
        chr_print("memcpy failed!\n");
    }

    /* a核发送黑名单给c核 */
    if (VOS_OK != VOS_SendMsg(MSP_PID_CHR, chr_acpu_block_list)) {
        chr_print("send block list Msg failed!\n");
        return OM_APP_SEND_CCPU_FAIL;
    }
    chr_print("success.\n");

    return VOS_OK;
}

unsigned int chr_send_block_list_to_mdm(void)
{
    unsigned int ret;
    ret = chr_acpu_block_list_send(MSP_PID_CHR_MDM);
    if (ret != OM_APP_MSG_OK) {
        chr_print("block List Send fail. ret = 0x%x!\n", ret);
        return ret;
    }

#ifdef FEATURE_CHR_NR
    ret = chr_acpu_block_list_send(MSP_PID_CHR_NR);
    if (ret != OM_APP_MSG_OK) {
        chr_print("block List Send to NR failed. ret = 0x%x!\n", ret);
        return ret;
    }
#endif
    return VOS_OK;
}

unsigned int chr_acpu_block_list_proc(unsigned char *data, unsigned int len)
{
    chr_app_block_list_s *chr_block_list = NULL;
    unsigned int msglen;
    unsigned int i;

    chr_list_info_s *list_info = NULL;

    if (len < sizeof(chr_app_block_list_s)) {
        chr_print("input length error !\n");
        return OM_APP_MSG_LENGTH_ERR;
    }
    /* 判断实际的消息长度和代码传入的是否一致 */
    chr_block_list = (chr_app_block_list_s *)data;

    msglen = chr_block_list->chr_om_header.msglen;
    chr_log_cfg_report("chr_acpu_block_list_proc MsgLen:0x%x,totallen:0x%x!\n", msglen, len);
    if (len != (msglen + sizeof(chr_alarm_msg_head_s))) {
        chr_print("input length error !\n");
        return OM_APP_MSG_LENGTH_ERR;
    }

    /* 检查黑名单的有效性 */
    for (i = 0; i < (msglen / sizeof(chr_list_info_s)); i++) {
        if (chr_block_list->block_list[i].moduleid > OM_ERR_LOG_MOUDLE_ID_BUTT ||
            chr_block_list->block_list[i].moduleid < OM_ERR_LOG_MOUDLE_ID_GUNAS) {
            chr_print("param invalid !\n");
            /* 返回错误结果 */
            return OM_APP_PARAM_INAVALID;
        }
        chr_log_cfg_report("blocklist moduleid:0x%x, alarmid:0x%x, AlarmType:%d!\n",
                         chr_block_list->block_list[i].moduleid, chr_block_list->block_list[i].alarmid,
                         chr_block_list->block_list[i].alarm_type);
    }

    /* a核黑名单保存 */
    list_info = (chr_list_info_s *)VOS_MemAlloc(MSP_PID_CHR, DYNAMIC_MEM_PT, msglen);
    if (list_info == NULL) {
        chr_print("Alloc Req Msg failed!\n");
        return OM_APP_MEM_ALLOC_ERR;
    }
    if (memcpy_s(list_info, msglen, chr_block_list->block_list, msglen)) {
        chr_print("memcpy failed!\n");
    }

    /* 黑名单是否已经保存过 */
    if (g_chr_block_cfg.chr_block_list != NULL) {
        (void)VOS_MemFree(MSP_PID_CHR, g_chr_block_cfg.chr_block_list);
    }
    g_chr_block_cfg.chr_block_list = list_info;
    g_chr_block_cfg.packet_len = msglen;

    return chr_send_block_list_to_mdm();
}

/*
 * 功能描述: 优先级0链表初始化函数
 */
void chr_acpu_priority_list_init(void)
{
    g_chr_priority_list.sn = PRIORITY_PACKET_START;
    g_chr_priority_list.packet_len = 0;
    blist_head_init(&g_chr_priority_list.priority_node.priority_list);

    return;
}
/*
 * 功能描述: 优先级0链表释放函数
 */
void chr_acpu_priority_list_free(void)
{
    LIST_S *me = NULL;
    LIST_S *n = NULL;
    chr_priority_node_s *save_node = VOS_NULL_PTR;

    blist_for_each_safe(me, n, &g_chr_priority_list.priority_node.priority_list)
    {
        save_node = blist_entry(me, chr_priority_node_s, priority_list);
        blist_del(&save_node->priority_list);
        VOS_MemFree(MSP_PID_CHR, save_node);
    }

    g_chr_priority_list.sn = PRIORITY_PACKET_START;
    g_chr_priority_list.packet_len = 0;

    return;
}
/*
 * 功能描述: 优先级0链表添加函数
 */
void chr_acpu_priority_list_add(chr_priority_node_s *priority_node)
{
    blist_add_tail(&priority_node->priority_list, &g_chr_priority_list.priority_node.priority_list);

    g_chr_priority_list.sn++;
    g_chr_priority_list.packet_len += priority_node->packet_len;

    return;
}
/*
 * 功能描述:优先级0下发清结束标志
 */
void chr_acpu_priority_list_end(void)
{
    g_chr_priority_list.sn = PRIORITY_PACKET_END;
    return;
}

/*
 * 功能描述: 检查优先级为0的合法性
 */
unsigned int chr_acpu_check_priority_list(unsigned char *data)
{
    chr_app_priority_cfg_s *app_priority_cfg;

    app_priority_cfg = (chr_app_priority_cfg_s *)data;
    /* 判断传入长度和传入个数是否匹配 */
    if (app_priority_cfg->count !=
        (app_priority_cfg->chr_om_header.msglen - sizeof(unsigned int)) / (sizeof(chr_priority_info_s))) {
        chr_print("input length not match count !\n");
        return PERIOD_CHK_FAIL;
    }

    /* 检查序列号是否完整 */
    if ((app_priority_cfg->packt_sn != g_chr_priority_list.sn) &&
        (app_priority_cfg->packt_sn != PRIORITY_PACKET_END) &&
        (app_priority_cfg->packt_sn != PRIORITY_PACKET_START)) {
        chr_print("SN not match!\n");
        return PERIOD_CHK_FAIL;
    }

    if (app_priority_cfg->packt_sn == PRIORITY_PACKET_START) {
        chr_print("SN restart!\n");
        return PERIOD_CHK_RESTART;
    }

    if (app_priority_cfg->packt_sn == PRIORITY_PACKET_END) {
        chr_print("SN complete!\n");
        return PERIOD_CHK_COMPLETE;
    }

    return PERIOD_CHK_CONTINUE;
}

/*
 * 功能描述: 保存优先级为0的名单
 */
unsigned int chr_acpu_priority_cfg_save(void)
{
    LIST_S *me = NULL;
    LIST_S *n = NULL;
    unsigned char *cur_prio = VOS_NULL_PTR;
    unsigned int len;
    unsigned int total_len;
    /*lint -save -e429*/
    chr_priority_node_s *save_node = VOS_NULL_PTR;
    chr_acpu_priority_save_s *prio_temp = VOS_NULL_PTR;
    total_len = g_chr_priority_list.packet_len + sizeof(chr_acpu_priority_save_s);

    prio_temp = (chr_acpu_priority_save_s *)VOS_MemAlloc(MSP_PID_CHR, DYNAMIC_MEM_PT, total_len);
    if (prio_temp == NULL) {
        return VOS_ERR;
    }
    if (memset_s(prio_temp, total_len, 0, total_len)) {
        chr_print("memset failed!\n");
    }

    cur_prio = (unsigned char *)prio_temp->priority_list;
    blist_for_each_safe(me, n, &g_chr_priority_list.priority_node.priority_list)
    {
        save_node = blist_entry(me, chr_priority_node_s, priority_list);
        len = save_node->packet_len;

        if (memcpy_s(cur_prio, len, save_node->priority_cfg, len)) {
            chr_print("memcpy failed!\n");
        }
        cur_prio += len;
        prio_temp->priority_len += len;

        if (prio_temp->priority_len > total_len) {
            VOS_MemFree(MSP_PID_CHR, prio_temp);
            return VOS_ERR;
        }
    }
    /*lint -restore +e429*/
    chr_acpu_priority_list_free();

    if (g_chr_priority_cfg != VOS_NULL_PTR) {
        VOS_MemFree(MSP_PID_CHR, g_chr_priority_cfg);
    }
    g_chr_priority_cfg = prio_temp;

    return VOS_OK;
}
/*
 * 功能描述: 优先级为0的名单发送给c核
 */
unsigned int chr_acpu_priority_cfg_send(unsigned int recvpid)
{
    chr_acpu_priority_cfg_s *acpu_prio_cfg = VOS_NULL_PTR;

    if (recvpid != MSP_PID_CHR_MDM && recvpid != MSP_PID_CHR_NR) {
        chr_print("recv pid Req Msg failed!\n");
        return OM_APP_PARAM_INAVALID;
    }

    /* A核申请优先级0列表消息发送给c核 */
    acpu_prio_cfg = (chr_acpu_priority_cfg_s *)VOS_AllocMsg(MSP_PID_CHR, (g_chr_priority_cfg->priority_len +
                                                                                   sizeof(chr_acpu_priority_cfg_s) -
                                                                                   VOS_MSG_HEAD_LENGTH));
    if (acpu_prio_cfg == VOS_NULL_PTR) {
        chr_print("Alloc Req Msg failed!\n");
        return OM_APP_MEM_ALLOC_MSG_ERR;
    }

    acpu_prio_cfg->ulReceiverPid = recvpid;
    acpu_prio_cfg->msg_name = OM_ACPU_REPORT_PRIORITY_NAME;
    acpu_prio_cfg->packet_len = g_chr_priority_cfg->priority_len;
    if (memcpy_s(&acpu_prio_cfg->priority_cfg, g_chr_priority_cfg->priority_len,
                 g_chr_priority_cfg->priority_list, g_chr_priority_cfg->priority_len)) {
        chr_print("memcpy failed!\n");
    }

    /* a核发送黑名单给c核 */
    if (VOS_OK != VOS_SendMsg(MSP_PID_CHR, acpu_prio_cfg)) {
        chr_print("send priority to ccpu failed!\n");
        return VOS_ERR;
    }

    chr_print("Send Priority Cfg success.\n");
    return VOS_OK;
}

unsigned int chr_send_priority_to_mdm(void)
{
    unsigned int ret;

    ret = chr_acpu_priority_cfg_save();
    if (ret != VOS_OK) {
        chr_acpu_priority_list_free();
        chr_print("PriorityCfg Save failed !\n");
        return OM_APP_SAVE_LIST_FAIL;
    }

    ret = chr_acpu_priority_cfg_send(MSP_PID_CHR_MDM);
    if (ret != VOS_OK) {
        chr_print("PriorityCfg Send failed !\n");
        return OM_APP_SEND_CCPU_FAIL;
    }

#ifdef FEATURE_CHR_NR
    ret = chr_acpu_priority_cfg_send(MSP_PID_CHR_NR);
    if (ret != VOS_OK) {
        chr_print("PriorityCfg Send to NR failed !\n");
        return OM_APP_SEND_CCPU_FAIL;
    }
#endif
    return VOS_OK;
}

/*
 * 功能描述: APP下发给OM的优先级配置处理函数,开机下发
 */
unsigned int chr_acpu_priority_cfg_proc(unsigned char *data, unsigned int msg_len)
{
    chr_app_priority_cfg_s *app_prio_cfg = VOS_NULL_PTR;
    chr_priority_node_s *priority_node = VOS_NULL_PTR;
    unsigned int len_tmp;
    unsigned int ret;
    unsigned int i;

    if (msg_len < sizeof(chr_app_priority_cfg_s)) {
        chr_print("input length too short !\n");
        return OM_APP_MSG_LENGTH_ERR;
    }
    /* 判断传入长度和实际的长度是否一致 */
    app_prio_cfg = (chr_app_priority_cfg_s *)data;
    chr_log_cfg_report("chr_acpu_priority_cfg_proc:MsgLen:0x%x,totallen:0x%x!\n", app_prio_cfg->chr_om_header.msglen,
                     msg_len);

    if (msg_len != (app_prio_cfg->chr_om_header.msglen + sizeof(chr_alarm_msg_head_s))) {
        chr_print("input length not match !\n");
        return OM_APP_MSG_LENGTH_ERR;
    }
    /* 打印优先级 */
    for (i = 0; i < app_prio_cfg->count; i++) {
        chr_log_cfg_report("Priority moduleid:0x%x, alarmid:0x%x, AlarmType:0x%x,priority:0x%x!\n",
                         app_prio_cfg->prior_map[i].moduleid, app_prio_cfg->prior_map[i].alarmid,
                         app_prio_cfg->prior_map[i].alarm_type, app_prio_cfg->prior_map[i].priority);
    }
    /* 检查优先级列表 */
    ret = chr_acpu_check_priority_list(data);
    if (ret == PERIOD_CHK_FAIL) {
        chr_print("param check failed!\n");
        chr_acpu_priority_list_free();
        return OM_APP_MEM_ALLOC_ERR;
    } else if (ret == PERIOD_CHK_RESTART) {
        chr_print("Priority restart!\n");
        chr_acpu_priority_list_free();
    }

    /* 创建节点，申请内存 */
    len_tmp = app_prio_cfg->chr_om_header.msglen - sizeof(unsigned int);
    priority_node = (chr_priority_node_s *)VOS_MemAlloc(MSP_PID_CHR, DYNAMIC_MEM_PT,
                                                        len_tmp + sizeof(chr_priority_node_s));
    if (priority_node == VOS_NULL_PTR) {
        chr_print("list mem alloc failed !\n");
        return OM_APP_MEM_ALLOC_ERR;
    }
    if (memcpy_s(priority_node->priority_cfg, len_tmp, app_prio_cfg->prior_map, len_tmp)) {
        chr_print("memcpy failed!\n");
    }
    priority_node->packet_len = len_tmp;
    chr_acpu_priority_list_add(priority_node);

    if (ret == PERIOD_CHK_COMPLETE) {
        chr_acpu_priority_list_end();
        return chr_send_priority_to_mdm();
    }
    return VOS_OK;
}

/*
 * 功能描述: 初始化上报周期的链表
 */
void chr_acpu_period_list_init(void)
{
    g_chr_period_list.sn = PRIORITY_PACKET_START;
    g_chr_period_list.packet_len = 0;
    blist_head_init(&g_chr_period_list.period_node.period_list);

    return;
}
/*
 * 功能描述: 上报周期链表的释放
 */
void chr_acpu_period_list_free(void)
{
    LIST_S *me = NULL;
    LIST_S *n = NULL;
    chr_period_node_s *save_node = VOS_NULL_PTR;

    blist_for_each_safe(me, n, &g_chr_period_list.period_node.period_list)
    {
        save_node = blist_entry(me, chr_period_node_s, period_list);
        blist_del(&save_node->period_list);
        VOS_MemFree(MSP_PID_CHR, save_node);
    }

    g_chr_period_list.sn = PERIOD_PACKET_START;
    g_chr_period_list.packet_len = 0;

    return;
}
/*
 * 功能描述: 上报周期链表的添加
 */
void chr_acpu_period_list_add(chr_period_node_s *period_node)
{
    blist_add_tail(&period_node->period_list, &g_chr_period_list.period_node.period_list);

    g_chr_period_list.sn++;
    g_chr_period_list.packet_len += period_node->packet_len;

    return;
}
/*
 * 功能描述: 下发上报周期名单结束标志
 */
void chr_acpu_period_list_end(void)
{
    g_chr_period_list.sn = PERIOD_PACKET_END;
    return;
}

/*
 * 功能描述: 检查上报周期名单的合法性
 */
unsigned int chr_acpu_check_period_list(unsigned char *data)
{
    chr_app_period_cfg_s *period_cfg;

    period_cfg = (chr_app_period_cfg_s *)data;

    /* 判断传入长度和传入个数是否匹配 */
    if (period_cfg->count != (period_cfg->chr_om_header.msglen - sizeof(unsigned int)) / (sizeof(chr_period_cfg_s))) {
        chr_print("input length not match count !\n");
        return PERIOD_CHK_FAIL;
    }

    /* 检查序列号是否完整 */
    if ((period_cfg->msg_sn != g_chr_period_list.sn) && (period_cfg->msg_sn != PERIOD_PACKET_END) &&
        (period_cfg->msg_sn != PERIOD_PACKET_START)) {
        chr_print("SN not match %d!\n", PERIOD_PACKET_END);
        return PERIOD_CHK_FAIL;
    }

    if (period_cfg->msg_sn == PERIOD_PACKET_START) {
        chr_print("SN restart!\n");
        return PERIOD_CHK_RESTART;
    }

    if (period_cfg->msg_sn == PRIORITY_PACKET_END) {
        chr_print("SN complete!\n");
        return PERIOD_CHK_COMPLETE;
    }

    return PERIOD_CHK_CONTINUE;
}

/*
 * 功能描述: 上报周期名单的保存
 */
unsigned int chr_acpu_period_cfg_save(void)
{
    LIST_S *me = NULL;
    LIST_S *n = NULL;
    unsigned char *cur_period = VOS_NULL_PTR;
    unsigned int len;
    unsigned int total_len;
    chr_period_node_s *save_node = VOS_NULL_PTR;
    /*lint -save -e429*/
    chr_acpu_period_save_s *period_tmp = VOS_NULL_PTR;

    total_len = g_chr_period_list.packet_len + sizeof(chr_acpu_period_save_s);
    period_tmp = (chr_acpu_period_save_s *)VOS_MemAlloc(MSP_PID_CHR, DYNAMIC_MEM_PT, total_len);
    if (period_tmp == NULL) {
        return VOS_ERR;
    }
    if (memset_s(period_tmp, total_len, 0, total_len)) {
        chr_print("memset failed!\n");
    }

    cur_period = (unsigned char *)period_tmp->period_cfg;
    blist_for_each_safe(me, n, &g_chr_period_list.period_node.period_list)
    {
        save_node = blist_entry(me, chr_period_node_s, period_list);
        len = save_node->packet_len;

        if (memcpy_s(cur_period, len, save_node->period_cfg, len)) {
            chr_print("memcpy failed!\n");
        }
        cur_period += len;
        period_tmp->packet_len += len;
        period_tmp->period = save_node->period;
        if (period_tmp->packet_len > total_len) {
            VOS_MemFree(MSP_PID_CHR, period_tmp);
            return VOS_ERR;
        }
    }

    chr_acpu_period_list_free();

    if (g_chr_period_cfg != VOS_NULL_PTR) {
        VOS_MemFree(MSP_PID_CHR, g_chr_period_cfg);
    }
    g_chr_period_cfg = period_tmp;
    /*lint -restore +e429*/
    return VOS_OK;
}
/*
 * 功能描述: 上报周期名单发送给c核
 */
unsigned int chr_acpu_period_cfg_send(unsigned int recvpid)
{
    chr_acpu_period_cfg_s *period_cfg = VOS_NULL_PTR;

    if (recvpid != MSP_PID_CHR_MDM && recvpid != MSP_PID_CHR_NR) {
        chr_print("recv pid Req Msg failed!\n");
        return OM_APP_PARAM_INAVALID;
    }

    /* A核消息发送给c核 */
    period_cfg = (chr_acpu_period_cfg_s *)VOS_AllocMsg(MSP_PID_CHR,
        g_chr_period_cfg->packet_len + sizeof(chr_acpu_period_cfg_s) - VOS_MSG_HEAD_LENGTH);
    if (period_cfg == VOS_NULL_PTR) {
        chr_print("Alloc Req Msg failed!\n");
        return OM_APP_MEM_ALLOC_MSG_ERR;
    }

    period_cfg->ulReceiverPid = MSP_PID_CHR_MDM;
    period_cfg->msg_name = OM_ACPU_REPORT_PERIOD_NAME;
    period_cfg->period = g_chr_period_cfg->period;
    period_cfg->packet_len = g_chr_period_cfg->packet_len;
    if (memcpy_s(&period_cfg->period_cfg, g_chr_period_cfg->packet_len, g_chr_period_cfg->period_cfg,
                 g_chr_period_cfg->packet_len)) {
        chr_print("memcpy failed!\n");
    }

    /* a核发送黑名单给c核 */
    if (VOS_OK != VOS_SendMsg(MSP_PID_CHR, period_cfg)) {
        chr_print("send Period to ccpu failed !\n");
        return VOS_ERR;
    }
    chr_print("Send Period Cfg success.\n");

    return VOS_OK;
}

unsigned int chr_send_period_to_mdm(void)
{
    unsigned int ret;

    ret = chr_acpu_period_cfg_save();
    if (ret != VOS_OK) {
        chr_acpu_period_list_free();
        chr_print("PeriodCfg Save failed !\n");
        return OM_APP_SAVE_LIST_FAIL;
    }

    ret = chr_acpu_period_cfg_send(MSP_PID_CHR_MDM);
    if (ret != VOS_OK) {
        chr_print("PeriodCfg Send failed !\n");
        return OM_APP_SEND_CCPU_FAIL;
    }
#ifdef FEATURE_CHR_NR
    ret = chr_acpu_period_cfg_send(MSP_PID_CHR_NR);
    if (ret != VOS_OK) {
        chr_print("PeriodCfg Send to NR failed !\n");
        return OM_APP_SEND_CCPU_FAIL;
    }
#endif
    return VOS_OK;
}

/*
 * 功能描述: APP下发给OM的周期处理函数
 */
unsigned int chr_acpu_period_cfg_proc(unsigned char *data, unsigned int msg_len)
{
    chr_app_period_cfg_s *app_period_cfg = VOS_NULL_PTR;

    chr_period_node_s *period_node = VOS_NULL_PTR;
    unsigned int len_tmp;
    unsigned int ret;
    unsigned int i;

    if (msg_len < sizeof(chr_app_period_cfg_s)) {
        chr_print("input length too short !\n");
        return OM_APP_MSG_LENGTH_ERR;
    }
    /* 判断传入长度和实际的长度是否一致 */
    app_period_cfg = (chr_app_period_cfg_s *)data;

    chr_log_cfg_report("period_cfg_proc:msgLen:0x%x,totallen:0x%x\n", app_period_cfg->chr_om_header.msglen, msg_len);
    if (msg_len != (app_period_cfg->chr_om_header.msglen + sizeof(chr_alarm_msg_head_s))) {
        chr_print("input length not match !\n");
        return OM_APP_MSG_LENGTH_ERR;
    }

    /* 打印周期 */
    for (i = 0; i < app_period_cfg->count; i++) {
        chr_log_cfg_report("period moduleid:0x%x, count:0x%x!\n", app_period_cfg->period_map[i].moduleid,
                         app_period_cfg->period_map[i].count);
    }
    /* 上报周期配置检查 */
    ret = chr_acpu_check_period_list(data);
    if (ret == PERIOD_CHK_FAIL) {
        chr_print("param check failed!\n");
        chr_acpu_period_list_free();
        return OM_APP_MEM_ALLOC_ERR;
    } else if (ret == PERIOD_CHK_RESTART) {
        chr_print("Period restart!\n");
        chr_acpu_period_list_free();
    }

    /* 创建节点，申请内存 */
    len_tmp = app_period_cfg->chr_om_header.msglen - sizeof(unsigned char) - sizeof(unsigned char) -
        sizeof(unsigned char) - sizeof(unsigned char);
    period_node = (chr_period_node_s *)VOS_MemAlloc(MSP_PID_CHR, DYNAMIC_MEM_PT, len_tmp + sizeof(chr_period_node_s));
    if (period_node == VOS_NULL_PTR) {
        chr_print("list mem alloc failed !\n");
        return OM_APP_MEM_ALLOC_ERR;
    }
    if (memcpy_s(period_node->period_cfg, len_tmp, app_period_cfg->period_map, len_tmp)) {
        chr_print("memcpy failed!\n");
    }
    period_node->packet_len = len_tmp;
    period_node->period = (unsigned int)app_period_cfg->period;
    /* 先回复一个结果给apk，释放vcom口 */
    chr_acpu_period_list_add(period_node);

    if (ret == PERIOD_CHK_COMPLETE) {
        chr_acpu_period_list_end();
        return chr_send_period_to_mdm();
    }

    return VOS_OK;
}

/*
 * 功能描述: OM给AP回复处理结果
 */
void chr_acpu_send_app_cfg_result(unsigned int result)
{
    chr_app_result_cfg_cnf_s app_result_cfg = { { 0, }, 0 };

    if (memset_s(&app_result_cfg, sizeof(app_result_cfg), 0, (unsigned int)sizeof(chr_app_result_cfg_cnf_s))) {
        chr_print("memset failed!\n");
    }

    app_result_cfg.chr_om_header.msgtype = OM_ERR_LOG_MSG_CFG_CNF;
    app_result_cfg.chr_om_header.msgsn = g_chr_cfg_vcom_debug_info.vcom_send_sn++;
    app_result_cfg.chr_om_header.msglen = (sizeof(chr_app_result_cfg_cnf_s) - sizeof(chr_alarm_msg_head_s));
    app_result_cfg.status = result;
    chr_acpu_send_vcom_data((unsigned char *)&app_result_cfg, (unsigned int)sizeof(chr_app_result_cfg_cnf_s));
    chr_print("cfg response success. status = 0x%x!\r\n ", result);

    return;
}
/*
 * Description: 诊断modem单独复位回调函数
 */
int chr_reset_ccore_cb(drv_reset_cb_moment_e param, int userdata)
{
    chr_app_req_s *reset_ccore_req = VOS_NULL_PTR;

    if (param == MDRV_RESET_CB_AFTER) {
        chr_print("modem reset after!\n");
        reset_ccore_req = (chr_app_req_s *)VOS_AllocMsg(MSP_PID_CHR,
                                                        sizeof(chr_app_req_s) - VOS_MSG_HEAD_LENGTH);
        if (reset_ccore_req == NULL) {
            chr_print("alloc mem fail!\n");
            return VOS_ERR;
        }
        reset_ccore_req->ulReceiverPid = MSP_PID_CHR;
        reset_ccore_req->msg_name = CHR_ID_RESET_CCORE;
        if (VOS_OK != VOS_SendMsg(MSP_PID_CHR, reset_ccore_req)) {
            chr_print("send msg fail !\n");
            return VOS_ERR;
        }
    }

    return VOS_OK;
}
/*
 * Description: 单独复位后a核处理函数
 */
unsigned int chr_acpu_reset_proc(void)
{
    if ((g_chr_block_cfg.packet_len != 0) && (g_chr_block_cfg.chr_block_list != NULL)) {
        chr_print("Send blockList enter.\n");
        (void)chr_acpu_block_list_send(MSP_PID_CHR_MDM);
#ifdef FEATURE_CHR_NR
        (void)chr_acpu_block_list_send(MSP_PID_CHR_NR);
#endif
    }

    if (g_chr_priority_cfg != NULL) {
        chr_print("Send Priority Cfg enter.\n");
        (void)chr_acpu_priority_cfg_send(MSP_PID_CHR_MDM);
#ifdef FEATURE_CHR_NR
        (void)chr_acpu_priority_cfg_send(MSP_PID_CHR_NR);
#endif
    }

    if (g_chr_period_cfg != NULL) {
        chr_print("Send Period Cfg enter.\n");
        (void)chr_acpu_period_cfg_send(MSP_PID_CHR_MDM);
#ifdef FEATURE_CHR_NR
        (void)chr_acpu_period_cfg_send(MSP_PID_CHR_NR);
#endif
    }

    return VOS_OK;
}
/*
 * 功能描述: CHR配置初始化
 */
unsigned int chr_cfg_init(void)
{
    unsigned int result;
    char *reset_name = "CHR"; /* C核单独复位的名字 */

    /* 初始化优先级/上报周期节点 */
    chr_acpu_priority_list_init();
    chr_acpu_period_list_init();

    result = (unsigned int)mdrv_sysboot_register_reset_notify(reset_name, (pdrv_reset_cbfun)chr_reset_ccore_cb,
                                                              0, CHR_SYSBOOT_PRIORITY_LEVEL);
    if (result) {
        chr_print("chr register ccore reset failed.\n");
        return VOS_OK;
    }
    return VOS_OK;
}

/*
 * 功能描述: CHR配置初始化
 */
void chr_acpu_block_list_show(void)
{
    unsigned int i;
    if (g_chr_block_cfg.chr_block_list == NULL) {
        chr_print("blocklist is null!\n");
        return;
    }
    for (i = 0; i < (g_chr_block_cfg.packet_len / sizeof(chr_list_info_s)); i++) {
        chr_print("blocklist moudleid:0x%x,alarmid:0x%x,type:0x%x !\n", g_chr_block_cfg.chr_block_list[i].moduleid,
                  g_chr_block_cfg.chr_block_list[i].alarmid, g_chr_block_cfg.chr_block_list[i].alarm_type);
    }
    return;
}
/*
 * 功能描述: CHR配置初始化
 */
void chr_acpu_priority0_show(void)
{
    unsigned int i;
    if (g_chr_priority_cfg == NULL) {
        chr_print("Priority0 is null!\n");
        return;
    }
    for (i = 0; i < (g_chr_priority_cfg->priority_len / sizeof(chr_priority_info_s)); i++) {
        chr_print("Priority0  moudleid:0x%x,alarmid:0x%x,priority:0x%x,type:0x%x !\n",
                  g_chr_priority_cfg->priority_list[i].moduleid, g_chr_priority_cfg->priority_list[i].alarmid,
                  g_chr_priority_cfg->priority_list[i].priority, g_chr_priority_cfg->priority_list[i].alarm_type);
    }
    return;
}

/*
 * 功能描述: CHR配置初始化
 */
void chr_acpu_period_show(void)
{
    unsigned int i;
    if (g_chr_period_cfg == NULL) {
        chr_print("PeriodCfg is null!\n");
        return;
    }
    for (i = 0; i < (g_chr_period_cfg->packet_len / sizeof(chr_period_cfg_s)); i++) {
        chr_print("period moudleid:0x%x,count:0x%x !\n", g_chr_period_cfg->period_cfg[i].moduleid + 0x20000,
                  g_chr_period_cfg->period_cfg[i].count);
    }
    return;
}

EXPORT_SYMBOL(chr_acpu_block_list_show);
EXPORT_SYMBOL(chr_acpu_priority0_show);
EXPORT_SYMBOL(chr_acpu_period_show);
#endif
