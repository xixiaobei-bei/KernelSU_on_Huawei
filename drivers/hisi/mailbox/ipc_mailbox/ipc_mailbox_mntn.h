/*
 *
 * IPC mailbox driver maintain utils.
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef IPC_MAILBOX_MNTN_H
#define IPC_MAILBOX_MNTN_H

#include <linux/list.h>

enum ipc_mbox_mntn_err_code {
	MBX_MNTN_SUCCESS,

	ER_MNTN_INVALID_RPROC_ID,
	ER_MNTN_INVALID_EVENT,
	ER_MNTN_NOT_READY_MDEV_LIST,
	ER_MNTN_UNKOWN_ERR
};

int ipc_start_rec_send_time(void);
int ipc_stop_rec_send_time(void);
int ipc_start_rec_async_send_time(void);
int ipc_stop_rec_async_send_time(void);
int ipc_show_rproc_rt_rec(int rproc_id);
int ipc_show_rproc_async_send_rec(int rproc_id);
int ipc_trace_rproc_id(int rproc_id);
int ipc_untrace_rproc_id(int rproc_id);
int ipc_untrace_all_ipc_func(void);
int ipc_trace_event(unsigned int event);
int ipc_untrace_event(unsigned int event);
int ipc_show_all_mntn_event(void);
int ipc_mbox_show_all_mdev_info(void);
int ipc_open_ipc_timestamp_dump(void);
int ipc_close_ipc_timestamp_dump(void);

void ipc_mntn_register_mdevs(struct list_head *list);
void ipc_mntn_reset_continuous_fail_cnt(void);

#endif
