/*
 *
 * mailbox driver maintain utils.
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
#include <linux/fs.h>
#include <linux/hisi/rdr_hisi_ap_hook.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/timekeeping.h>
#include <linux/sort.h>
#include <securec.h>

#include "ipc_rproc_id_mgr.h"
#include "ipc_mailbox.h"
#include "ipc_mailbox_event.h"
#include "ipc_mailbox_mntn.h"

/*lint -e580 */

#define MAX_DBG_LOG_TYPE 32
#define MAX_DBG_FUNC_NAME_LEN 64
#define MAX_RPROC_ID HISI_RPROC_MAX_MBX_ID
#define MAX_RPROC_ID_POOL_CNT ((HISI_RPROC_MAX_MBX_ID / 8) + 1)

#define IPC_MNTN_OPEN 0xE551
#define MAILBOX_DUMP_TIME 10000000000UL
#define CONTINUOUS_FAIL_CNT_MAX 50
#define BIT_CNT_OF_BYTE 8

#define MAX_EVENT_POOL_CNT ((MAX_IPC_EVENT_NUM / 8) + 1)

#define def_mntn_event(_event, _event_handle) \
	{ _event, _event_handle }

#define CONTINUOUS_FAIL_JUDGE \
	(likely(g_mbox_mntn_data.continuous_fail_cnt < CONTINUOUS_FAIL_CNT_MAX))

#define MAX_SEND_RECORD_CNT  5
#define MAX_ASYNC_SEND_RECORD_CNT  5
#define NS_TO_US   1000

#undef pr_fmt
#define pr_fmt(fmt) "[ap_ipc]:" fmt

#define mntn_pr_err(fmt, args...)   pr_err(fmt "\n", ##args)
#define mntn_pr_info(fmt, args...)  pr_info(fmt "\n", ##args)

enum ipc_send_exception_type {
	MBX_SYNC_SUCCESS,
	MBX_SYNC_TASKLET_JAM_EX,
	MBX_SYNC_ISR_JAM_EX,
	MBX_SYNC_ACK_LOST_EX,
	MBX_ASYNC_TASK_FIFO_FULL_EX,
	MAX_MBX_EXCEPTION_TYPE
};

struct ipc_mbox_rt_rec {
	unsigned int rec_idx;
	unsigned int async_rec_idx;
	char snd_rec_cnt;
	u64 sync_send_dump_time;
	u64 async_send_dump_time;
	u64 sync_send_start_time;
	u64 sync_send_slice_begin;

	unsigned int sync_send_spend_time[MAX_SEND_RECORD_CNT];
	u64 async_send_timestamps[MAX_ASYNC_SEND_RECORD_CNT];
	enum ipc_send_exception_type exception[MAX_SEND_RECORD_CNT];

	unsigned int ex_cnt[MAX_MBX_EXCEPTION_TYPE];
};

static char *g_ipc_send_exception_name[MAX_MBX_EXCEPTION_TYPE] = {
	[MBX_SYNC_SUCCESS] = "MBX_SYNC_SUCCESS",
	[MBX_SYNC_TASKLET_JAM_EX] = "MBX_SYNC_TASKLET_JAM_EX",
	[MBX_SYNC_ISR_JAM_EX] = "MBX_SYNC_ISR_JAM_EX",
	[MBX_SYNC_ACK_LOST_EX] = "MBX_SYNC_ACK_LOST_EX",
	[MBX_ASYNC_TASK_FIFO_FULL_EX] = "MBX_ASYNC_TASK_FIFO_FULL_EX"
};

struct ipc_mbox_mntn_data {
	struct list_head *all_mdevs;
	unsigned char rproc_trace_flag[MAX_RPROC_ID_POOL_CNT];
	unsigned char event_trace_flag[MAX_EVENT_POOL_CNT];
	unsigned int ipc_dump_flag;
	/*
	 * use the continuous_fail_cnt to
	 * control the Continuous ipc timeout times
	 * which may overflow the kmesg log
	 */
	int continuous_fail_cnt;

	int is_rec_sync_time;
	int is_rec_async_time;
	struct ipc_mbox_rt_rec rt_rec[MAX_RPROC_ID];
};

static struct ipc_mbox_mntn_data g_mbox_mntn_data;

static inline char *ipc_get_send_exception_name(unsigned int ex)
{
	return (ex >= MAX_MBX_EXCEPTION_TYPE) ? "UNKOWN_EXCEPTION" :
		g_ipc_send_exception_name[ex];
}

static inline int ipc_is_ipc_dump_open(void)
{
	return g_mbox_mntn_data.ipc_dump_flag == IPC_MNTN_OPEN;
}

static struct ipc_mbox_rt_rec *ipc_get_mbox_rt_rec(int rproc_id)
{
	struct ipc_mbox_mntn_data *dbg_cfg = &g_mbox_mntn_data;

	if (rproc_id < 0 || rproc_id >=  MAX_RPROC_ID)
		return NULL;

	return &dbg_cfg->rt_rec[rproc_id];
}

/*
 * Enable ipc timeout dump mntn
 * IPC_MNTN_OPEN 0xE551, enable ipc timeout dump mntn
 */
int ipc_open_ipc_timestamp_dump(void)
{
	g_mbox_mntn_data.ipc_dump_flag = IPC_MNTN_OPEN;
	mntn_pr_err("close ipc dump flag success");
	return MBX_MNTN_SUCCESS;
}
EXPORT_SYMBOL(ipc_open_ipc_timestamp_dump);

/*
 * Disable the ipc timeout dump mntn
 */
int ipc_close_ipc_timestamp_dump(void)
{
	g_mbox_mntn_data.ipc_dump_flag = 0;
	mntn_pr_err("open ipc dump flag success");
	return MBX_MNTN_SUCCESS;
}
EXPORT_SYMBOL(ipc_close_ipc_timestamp_dump);

void ipc_mntn_reset_continuous_fail_cnt(void)
{
	g_mbox_mntn_data.continuous_fail_cnt = 0;
}
EXPORT_SYMBOL(ipc_mntn_reset_continuous_fail_cnt);

static void ipc_build_event_msg(char *str_buf,
	unsigned int buf_size, const char *fmt_msg, va_list args)
{
	int ret;

	if (!fmt_msg || !fmt_msg[0]) {
		str_buf[0] = 0;
		return;
	}

	/*lint -e592 */
	ret = vsnprintf(str_buf, buf_size - 1, fmt_msg, args);
	if (ret < 0) {
		mntn_pr_err("build event msg failed");
		return;
	}
	/*lint +e592 */
}
void ipc_mntn_register_mdevs(struct list_head *list)
{
}
EXPORT_SYMBOL(ipc_mntn_register_mdevs);

/***************************************************************************/
static void ipc_mbox_sync_begin_eh(struct hisi_mbox_device *mbox,
	unsigned int event, const char *fmt_msg, va_list args)
{
	struct ipc_mbox_rt_rec *rec = NULL;

	(void)event;
	(void)fmt_msg;
	(void)args;

	rec = ipc_get_mbox_rt_rec(mbox->ops->get_rproc_id(mbox));
	if (!rec)
		return;

	rec->sync_send_start_time = ktime_get_ns();

	if (mbox->ops->get_slice &&
		mbox->ops->get_slice(mbox, &rec->sync_send_slice_begin))
		return;
}

static inline void ipc_pr_continuous_fail_msg(const char *msg)
{
	if (CONTINUOUS_FAIL_JUDGE)
		mntn_pr_err("%s", msg);
}

static void ipc_record_sync_fail_time(struct hisi_mbox_device *mbox)
{
	struct ipc_mbox_rt_rec *rec = NULL;

	if (!ipc_is_ipc_dump_open())
		return;

	rec = ipc_get_mbox_rt_rec(mbox->ops->get_rproc_id(mbox));
	if (!rec)
		return;

	if (ktime_get_ns() - rec->sync_send_dump_time > MAILBOX_DUMP_TIME) {
		rdr_system_error(MODID_AP_S_MAILBOX, 0, 0);
		rec->sync_send_dump_time = ktime_get_ns();
	}
}

static void ipc_record_sync_exception_type(struct hisi_mbox_device *mbox,
	enum ipc_send_exception_type e_type)
{
	struct ipc_mbox_rt_rec *rec = NULL;

	rec = ipc_get_mbox_rt_rec(mbox->ops->get_rproc_id(mbox));
	if (!rec)
		return;

	rec->exception[rec->rec_idx] = e_type;
	rec->ex_cnt[e_type]++;
}

static void ipc_mbox_sync_fail(struct hisi_mbox_device *mbox,
	 const char *msg, enum ipc_send_exception_type e_type)
{
	ipc_record_sync_exception_type(mbox, e_type);
	ipc_pr_continuous_fail_msg(msg);
	ipc_record_sync_fail_time(mbox);
}

static void ipc_mbox_sync_tasklet_jsm_eh(struct hisi_mbox_device *mbox,
	unsigned int event, const char *fmt_msg, va_list args)
{
	(void)event;
	(void)fmt_msg;
	(void)args;

	ipc_mbox_sync_fail(
		mbox, "reason:TASKLET jam", MBX_SYNC_TASKLET_JAM_EX);
}

static void ipc_mbox_sync_isr_jam_eh(struct hisi_mbox_device *mbox,
	unsigned int event, const char *fmt_msg, va_list args)
{
	(void)event;
	(void)fmt_msg;
	(void)args;

	ipc_mbox_sync_fail(mbox, "reason:ISR jam", MBX_SYNC_ISR_JAM_EX);
}

static void ipc_mbox_sync_ack_lost_eh(struct hisi_mbox_device *mbox,
	unsigned int event, const char *fmt_msg, va_list args)
{
	(void)event;
	(void)fmt_msg;
	(void)args;

	ipc_mbox_sync_fail(mbox, "reason:ACK lost", MBX_SYNC_ACK_LOST_EX);
}

static void ipc_mbox_show_timeout_dump(struct hisi_mbox_device *mbox,
	const char *fmt_msg, va_list args)
{
	char event_msg[MAX_EVENT_MSG_LEN] = {0};

	ipc_build_event_msg(event_msg, MAX_EVENT_MSG_LEN, fmt_msg, args);
	mntn_pr_err("%s", event_msg);

	if (mbox->ops->status)
		mbox->ops->status(mbox);
}

static void ipc_mbox_show_slice_time(struct hisi_mbox_device *mbox)
{
	struct ipc_mbox_rt_rec *rec = NULL;
	u64 slice_end = 0;
	u64 system_diff, diff;

	if (mbox->ops->get_slice && mbox->ops->get_slice(mbox, &slice_end))
		return;

	rec = ipc_get_mbox_rt_rec(mbox->ops->get_rproc_id(mbox));
	if (!rec)
		return;

	diff = ktime_get_ns() - rec->sync_send_start_time;
	system_diff = slice_end - rec->sync_send_slice_begin;
	mntn_pr_err("system_diff_time : %llu, mntn_diff_time : %llu\n",
		system_diff, diff);
}

static void ipc_mbox_sync_timeout_eh(struct hisi_mbox_device *mbox,
	unsigned int event, const char *fmt_msg, va_list args)
{
	(void)event;

	g_mbox_mntn_data.continuous_fail_cnt++;

	if (!CONTINUOUS_FAIL_JUDGE)
		return;

	ipc_mbox_show_timeout_dump(mbox, fmt_msg, args);
	ipc_mbox_show_slice_time(mbox);
}

static void ipc_mbox_async_in_queue_fail_eh(struct hisi_mbox_device *mbox,
	unsigned int event, const char *fmt_msg, va_list args)
{
	struct ipc_mbox_rt_rec *rec = NULL;

	(void)fmt_msg;
	(void)args;

	ipc_record_sync_exception_type(mbox, MBX_ASYNC_TASK_FIFO_FULL_EX);

	if (!ipc_is_ipc_dump_open())
		return;

	rec = ipc_get_mbox_rt_rec(mbox->ops->get_rproc_id(mbox));
	if (!rec)
		return;

	if (ktime_get_ns() - rec->async_send_dump_time >
		MAILBOX_DUMP_TIME) {
		rdr_system_error(MODID_AP_S_MAILBOX, 0, 0);
		rec->async_send_dump_time = ktime_get_ns();
	}
}

static void ipc_mbox_sync_proc_record_eh(struct hisi_mbox_device *mbox,
	unsigned int event, const char *fmt_msg, va_list args)
{
	(void)event;

	ipc_pr_continuous_fail_msg(fmt_msg);
}

static void ipc_mbox_async_send_time_eh(struct hisi_mbox_device *mbox,
	unsigned int event, const char *fmt_msg, va_list args)
{
	static struct ipc_mbox_mntn_data *dbg_cfg = &g_mbox_mntn_data;
	struct ipc_mbox_rt_rec *rec = NULL;

	(void)fmt_msg;
	(void)args;

	if (!dbg_cfg->is_rec_async_time)
		return;

	rec = ipc_get_mbox_rt_rec(mbox->ops->get_rproc_id(mbox));
	if (!rec)
		return;

	rec->async_send_timestamps[rec->async_rec_idx] = ktime_get_ns();
	rec->async_rec_idx = (rec->async_rec_idx + 1) % MAX_ASYNC_SEND_RECORD_CNT;
}

struct ipc_mbox_mntn_event_register {
	unsigned int event;
	void (*event_handle)(struct hisi_mbox_device *mbox,
		unsigned int event, const char *fmt_msg, va_list args);
};

static struct ipc_mbox_mntn_event_register g_mbox_mntn_event_handles[] = {
	def_mntn_event(EVENT_IPC_SYNC_SEND_BEGIN, ipc_mbox_sync_begin_eh),

	def_mntn_event(EVENT_IPC_SYNC_TASKLET_JAM,
		ipc_mbox_sync_tasklet_jsm_eh),
	def_mntn_event(EVENT_IPC_SYNC_ISR_JAM,
		ipc_mbox_sync_isr_jam_eh),
	def_mntn_event(EVENT_IPC_SYNC_ACK_LOST,
		ipc_mbox_sync_ack_lost_eh),

	def_mntn_event(EVENT_IPC_SYNC_SEND_TIMEOUT, ipc_mbox_sync_timeout_eh),
	def_mntn_event(EVENT_IPC_ASYNC_IN_QUEUE_FAIL,
		ipc_mbox_async_in_queue_fail_eh),
	def_mntn_event(EVENT_IPC_SYNC_PROC_RECORD,
		ipc_mbox_sync_proc_record_eh),

	def_mntn_event(EVENT_IPC_ASYNC_TASK_SEND,
		ipc_mbox_async_send_time_eh),
};

static int __init ipc_mbox_mntn_init(void)
{
	unsigned int i;
	int ret;
	struct ipc_mbox_mntn_event_register *event_reg = NULL;

	for (i = 0; i < ARRAY_SIZE(g_mbox_mntn_event_handles); i++) {
		event_reg = &g_mbox_mntn_event_handles[i];
		ret = ipc_regist_mailbox_event(
			event_reg->event, event_reg->event_handle);
		if (ret) {
			mntn_pr_err("regist mbox_event:%u failed",
				event_reg->event);
			return ret;
		}
	}

	return MBX_MNTN_SUCCESS;
}
module_init(ipc_mbox_mntn_init);

static void __exit ipc_mbox_mntn_exit(void)
{
	unsigned int i;
	struct ipc_mbox_mntn_event_register *event_reg = NULL;

	for (i = 0; i < ARRAY_SIZE(g_mbox_mntn_event_handles); i++) {
		event_reg = &g_mbox_mntn_event_handles[i];
		ipc_unregist_mailbox_event(
			event_reg->event, event_reg->event_handle);
	}
}
module_exit(ipc_mbox_mntn_exit);
/*lint +e580 */
