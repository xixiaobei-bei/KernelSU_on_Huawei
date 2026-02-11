/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 * Description: Cmdmonitor function, monitor every cmd which is sent to TEE.
 * Author: qiqingchao  q00XXXXXX
 * Create: 2016-06-21
 */
#include "cmdmonitor.h"
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <securec.h>
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/task.h>
#endif
#ifdef CONFIG_TEELOG
#include <huawei_platform/log/imonitor.h>
#define IMONITOR_TA_CRASH_EVENT_ID 901002003
#define IMONITOR_MEMSTAT_EVENT_ID 940007001
#define IMONITOR_TAMEMSTAT_EVENT_ID 940007002
#include "tlogger.h"
#endif
#include "tc_ns_log.h"
#include "smc.h"
#include "tui.h"
#include "mailbox_mempool.h"

const char g_cmd_monitor_white_table[][TASK_COMM_LEN] = {
	{"FIAgentThread"},
	{"AIAgentThread"},
};
const uint32_t g_white_table_thread_num = sizeof(g_cmd_monitor_white_table) /
	TASK_COMM_LEN;

static struct timespec memstat_check_time = {0};
static bool after_loader = false;
static int cmd_need_archivelog = 0;
static LIST_HEAD(g_cmd_monitor_list);
static int cmd_monitor_list_size = 0;
/* report 2 hours */
static const long long memstat_report_freq = 2 * 60 * 60 * 1000;
#define MAX_CMD_MONITOR_LIST 200
static DEFINE_MUTEX(g_cmd_monitor_lock);
struct cmd_monitor {
	struct list_head list;
	struct timespec sendtime;
	int count;
	bool returned;
	bool is_reported;
	int pid;
	int tid;
	char pname[TASK_COMM_LEN];
	char tname[TASK_COMM_LEN];
	unsigned int lastcmdid;
	long long timetotal;
};
static struct delayed_work cmd_monitor_work;
static struct delayed_work cmd_monitor_work_archive;
static int g_tee_detect_ta_crash = 0;
enum {
	TYPE_CRASH_TA = 1,
	TYPE_CRASH_TEE = 2,
};

void tzdebug_archivelog(void)
{
	schedule_delayed_work(&cmd_monitor_work_archive, usecs_to_jiffies(0));
}
void cmd_monitor_ta_crash(int32_t type)
{
	g_tee_detect_ta_crash = ((type == TYPE_CRASH_TEE) ? TYPE_CRASH_TEE :
		TYPE_CRASH_TA);
	tzdebug_archivelog();
}
static int get_pid_name(pid_t pid, char *comm, size_t size)
{
	struct task_struct *task = NULL;
	int sret = 0;

	if ((size <= TASK_COMM_LEN - 1) || (comm == NULL))
		return -1;
	rcu_read_lock();
	task = find_task_by_vpid(pid);
	if (task != NULL)
		get_task_struct(task);
	rcu_read_unlock();
	if (task != NULL) {
		sret = strncpy_s(comm, size, task->comm, strlen(task->comm));
		if (sret != 0)
			tloge("strncpy_s faild: errno = %d.\n", sret);
		put_task_struct(task);
		return sret;
	}
	return -1;

}

static bool is_thread_in_white_table(const char *tname)
{
	uint32_t i = 0;

	if (tname == NULL)
		return false;

	for (i = 0; i < g_white_table_thread_num; i++) {
		if (!strcmp(tname, g_cmd_monitor_white_table[i]))
			return true;
	}
	return false;
}

bool is_thread_reported(unsigned int tid)
{
	bool ret = false;
	struct cmd_monitor *monitor = NULL;

	mutex_lock(&g_cmd_monitor_lock);
	list_for_each_entry(monitor, &g_cmd_monitor_list, list) {
		if (monitor->tid == tid && (!is_tui_in_use(monitor->tid))) {
			ret = monitor->is_reported;
			break;
		}
	}
	mutex_unlock(&g_cmd_monitor_lock);
	return ret;
}

void report_imonitor(struct tee_mem *meminfo)
{
#ifdef CONFIG_TEELOG
	int ret = 0;
	int ret2 = 0;
	uint32_t result = 0;
	uint32_t i;
	struct imonitor_eventobj *memstat = NULL;
	struct imonitor_eventobj *pamemobj = NULL;
	struct timespec nowtime = current_kernel_time();
	if (meminfo == NULL) {
		return;
	}
	if (meminfo->ta_num > MEMINFO_TA_MAX) {
		return;
	}
	memstat = imonitor_create_eventobj(IMONITOR_MEMSTAT_EVENT_ID);
	if (memstat == NULL) {
		tloge("create eventobj failed\n");
		return;
	}
	result |= (uint32_t)imonitor_set_param_integer_v2(memstat, "totalmem", meminfo->total_mem);
	result |= (uint32_t)imonitor_set_param_integer_v2(memstat, "mem", meminfo->pmem);
	result |= (uint32_t)imonitor_set_param_integer_v2(memstat, "freemem", meminfo->free_mem);
	result |= (uint32_t)imonitor_set_param_integer_v2(memstat, "freememmin", meminfo->free_mem_min);
	result |= (uint32_t)imonitor_set_param_integer_v2(memstat, "tanum", meminfo->ta_num);
	result |= (uint32_t)imonitor_set_time(memstat, nowtime.tv_sec);
	if (result != 0) {
		tloge("ret failed ret=%d\n", ret);
		imonitor_destroy_eventobj(memstat);
		return;
	}
	ret = imonitor_send_event(memstat);
	imonitor_destroy_eventobj(memstat);
	if (ret <= 0) {
		tloge("imonitor_send_event send packet failed\n");
		return;
	}

	for (i = 0; i < meminfo->ta_num; i++) {
		pamemobj = imonitor_create_eventobj(IMONITOR_TAMEMSTAT_EVENT_ID);
		if (pamemobj == NULL) {
			tloge("create obj failed\n");
			ret2 = -1;
			break;
		}
		result = 0;
		result |= (uint32_t)imonitor_set_param_string_v2(pamemobj, "NAME", meminfo->ta_mem_info[i].ta_name);
		result |= (uint32_t)imonitor_set_param_integer_v2(pamemobj, "MEM", meminfo->ta_mem_info[i].pmem);
		result |= (uint32_t)imonitor_set_param_integer_v2(pamemobj, "MEMMAX", meminfo->ta_mem_info[i].pmem_max);
		result |= (uint32_t)imonitor_set_param_integer_v2(pamemobj, "MEMLIMIT", meminfo->ta_mem_info[i].pmem_limit);
		result |= (uint32_t)imonitor_set_time(pamemobj, nowtime.tv_sec);
		if (result != 0) {
			tloge("ret2 failed ret=%d\n", ret);
			imonitor_destroy_eventobj(pamemobj);
			return;
		}
		ret2 = imonitor_send_event(pamemobj);
		imonitor_destroy_eventobj(pamemobj);
		if (ret2 <= 0) {
			tloge("imonitor_send_event 2 send packet failed\n");
			break;
		}
	}
#endif
}

static void memstat_report(void)
{
	int ret;
	struct tee_mem *meminfo = (struct tee_mem *)mailbox_alloc(
		sizeof(struct tee_mem), MB_FLAG_ZERO);
	if (meminfo == NULL) {
		tloge("mailbox alloc failed\n");
		return;
	}

	ret = get_tee_meminfo(meminfo);
	if (ret == 0) {
		tlogd("report imonitor\n");
		report_imonitor(meminfo);
	}
	mailbox_free(meminfo);
}

void cmd_monitor_reset_send_time(void)
{
	struct cmd_monitor *monitor = NULL;
	int pid = current->tgid;
	int tid = current->pid;

	mutex_lock(&g_cmd_monitor_lock);
	list_for_each_entry(monitor, &g_cmd_monitor_list, list) {
		if (monitor->pid == pid && monitor->tid == tid) {
			monitor->sendtime = current_kernel_time();
			break;
		}
	}
	mutex_unlock(&g_cmd_monitor_lock);
}

static void cmd_monitor_tick(void)
{
	long long timedif;
	struct cmd_monitor *monitor = NULL;
	struct cmd_monitor *tmp = NULL;
	struct timespec nowtime = current_kernel_time();

	mutex_lock(&g_cmd_monitor_lock);
	list_for_each_entry_safe(monitor, tmp, &g_cmd_monitor_list, list) {
		if (monitor->returned == true) {
			tloge("[cmd_monitor_tick] pid=%d,pname=%s,tid=%d,tname=%s,\
lastcmdid=%u,count=%d timetotal=%lld us returned, remained command(s)=%d\n",
				monitor->pid, monitor->pname, monitor->tid,
				monitor->tname, monitor->lastcmdid, monitor->count,
				monitor->timetotal, cmd_monitor_list_size);
			list_del(&monitor->list);
			kfree(monitor);
			cmd_monitor_list_size--;
			continue;
		}
		/* not return, we need to check  */

		/*
		* get time value D (timedif=nowtime-sendtime), we do not care about overflow
		* 1 year means 1000 * (60*60*24*365) = 0x757B12C00
		* only 5bytes, will not overflow
		*/
		timedif = 1000 * (nowtime.tv_sec - monitor->sendtime.tv_sec) +
			(nowtime.tv_nsec - monitor->sendtime.tv_nsec) / 1000000;

		/* Temporally change timeout to 25s, we log the teeos log,and report */
		if ((timedif > 25 * 1000) && (!monitor->is_reported)) {
			monitor->is_reported = true;
			/* print tee stask */
			tloge("[cmd_monitor_tick] pid=%d,pname=%s,tid=%d,tname=%s,\
lastcmdid=%u,timedif=%lld ms and report\n",
				monitor->pid, monitor->pname, monitor->tid,
				monitor->tname, monitor->lastcmdid, timedif);
			/* threads out of white table need info dump */
			tloge("monitor: pid-%d", monitor->pid);
			if ((!(is_thread_in_white_table(monitor->tname))) &&
				(!is_tui_in_use(monitor->tid))) {
				cmd_need_archivelog = 1;
				wakeup_tc_siq();
			}
		} else if (timedif > 1 * 1000)
			tloge("[cmd_monitor_tick] pid=%d,pname=%s,tid=%d,timedif=%lld ms\n",
			      monitor->pid, monitor->pname, monitor->tid, timedif);
	}
	tlogi("[cmd_monitor_tick] cmd_monitor_list_size=%d\n",
		cmd_monitor_list_size);
	if (cmd_monitor_list_size > 0) {
		/* if have cmd in monitor list, we need tick */
		schedule_delayed_work(&cmd_monitor_work, usecs_to_jiffies(1000000));
	}
	mutex_unlock(&g_cmd_monitor_lock);
	/*
	* get time value D (timedif=nowtime-sendtime), we do not care about overflow
	* 1 year means 1000 * (60*60*24*365) = 0x757B12C00
	* only 5bytes, will not overflow
	*/
	timedif = 1000 * (nowtime.tv_sec - memstat_check_time.tv_sec) +
		(nowtime.tv_nsec - memstat_check_time.tv_nsec) / 1000000;
	if (timedif > memstat_report_freq && after_loader == true) {
		tloge("cmdmonitor auto report memstat\n");
		memstat_report();
		memstat_check_time = nowtime;
	}
	if (after_loader == false) {
		memstat_check_time = nowtime;
		after_loader = true;
	}
}
static void cmd_monitor_tickfn(struct work_struct *work)
{
	(void)(work);
	cmd_monitor_tick();
	/* check tlogcat if have new log  */
	tz_log_write();
}
static void cmd_monitor_archivefn(struct work_struct *work)
{
	(void)(work);
#ifdef CONFIG_TEELOG
	if (tlogger_store_lastmsg() < 0)
		tloge("[cmd_monitor_tick]tlogger_store_lastmsg failed\n");

	if (g_tee_detect_ta_crash == TYPE_CRASH_TA) {

		if (teeos_log_exception_archive(IMONITOR_TA_CRASH_EVENT_ID,
			"ta crash") < 0)
			tloge("log_exception_archive failed\n");

	}

	g_tee_detect_ta_crash = 0;
#endif
}


static struct cmd_monitor *init_monitor_locked(void)
{
	struct cmd_monitor *newitem = NULL;
	int pidnameresult = 0;
	int tidnameresult = 0;

	newitem = kzalloc(sizeof(*newitem), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)newitem)) {
		tloge("[cmd_monitor_tick]kzalloc faild\n");
		return NULL;
	}
	newitem->sendtime = current_kernel_time();
	newitem->count = 1;
	newitem->returned = false;
	newitem->is_reported = false;
	newitem->pid = current->tgid;
	newitem->tid = current->pid;
	pidnameresult = get_pid_name(newitem->pid, newitem->pname,
		sizeof(newitem->pname));
	if (pidnameresult != 0)
		newitem->pname[0] = '\0';
	tidnameresult = get_pid_name(newitem->tid, newitem->tname,
		sizeof(newitem->tname));
	if (tidnameresult != 0)
		newitem->pname[0] = '\0';
	INIT_LIST_HEAD(&newitem->list);
	list_add_tail(&newitem->list, &g_cmd_monitor_list);
	cmd_monitor_list_size++;
	return newitem;
}

void cmd_monitor_log(tc_ns_smc_cmd *cmd)
{
	int found_flag = 0;
	int pid = 0;
	int tid = 0;
	struct cmd_monitor *monitor = NULL;
	struct cmd_monitor *newitem = NULL;

	if (cmd == NULL)
		return;
	pid = current->tgid;
	tid = current->pid;
	mutex_lock(&g_cmd_monitor_lock);
	do {
		list_for_each_entry(monitor, &g_cmd_monitor_list, list) {
			if (monitor->pid == pid && monitor->tid == tid) {
				found_flag = 1;
				/* restart */
				monitor->sendtime = current_kernel_time();
				monitor->count++;
				monitor->returned = false;
				monitor->is_reported = false;
				monitor->lastcmdid = cmd->cmd_id;
				break;
			}
		}
		if (found_flag == 0) {
			if (cmd_monitor_list_size > MAX_CMD_MONITOR_LIST - 1) {
				tloge("[cmd_monitor_tick]MAX_CMD_MONITOR_LIST\n");
				break;
			}
			newitem = init_monitor_locked();
			if (newitem == NULL) {
				tloge("[cmd_monitor_tick]init_monitor failed\n");
				break;
			}
			newitem->lastcmdid = cmd->cmd_id;
			/* the first cmd will cause timer */
			if (cmd_monitor_list_size == 1)
				schedule_delayed_work(&cmd_monitor_work,
					usecs_to_jiffies(1000000));
		}
	} while (0);
	mutex_unlock(&g_cmd_monitor_lock);
}

void cmd_monitor_logend(tc_ns_smc_cmd *cmd)
{
	int pid = 0;
	int tid = 0;
	struct cmd_monitor *monitor = NULL;

	if (cmd == NULL)
		return;
	pid = current->tgid;
	tid = current->pid;
	mutex_lock(&g_cmd_monitor_lock);
	list_for_each_entry(monitor, &g_cmd_monitor_list, list) {
		if (monitor->pid == pid && monitor->tid == tid &&
			monitor->returned == false) {
			struct timespec nowtime = current_kernel_time();
			/*
			* get time value D (timedif=nowtime-sendtime), we do not care about overflow
			* 1 year means 1000000 * (60*60*24*365) = 0x1CAE8C13E000
			* only 6bytes, will not overflow
			*/
			long long timedif = 1000000 *
				(nowtime.tv_sec - monitor->sendtime.tv_sec) +
				(nowtime.tv_nsec - monitor->sendtime.tv_nsec) / 1000;
			monitor->timetotal += timedif;
			monitor->returned = true;
			/* we need set all monitor.returned = true ,dont break */
		}
	}
	mutex_unlock(&g_cmd_monitor_lock);
}
void do_cmd_need_archivelog(void)
{
	if (cmd_need_archivelog == 1) {
		cmd_need_archivelog = 0;
		schedule_delayed_work(&cmd_monitor_work_archive,
			usecs_to_jiffies(1000000));
	}
}
void init_cmd_monitor(void)
{
	INIT_DEFERRABLE_WORK((struct delayed_work *)
		(uintptr_t)&cmd_monitor_work, cmd_monitor_tickfn);
	INIT_DEFERRABLE_WORK((struct delayed_work *)
		(uintptr_t)&cmd_monitor_work_archive, cmd_monitor_archivefn);
}
