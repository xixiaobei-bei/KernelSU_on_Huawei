#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/timer.h>
#include <linux/kernel.h>

#include "tc_ns_log.h"
#include "securec.h"
#include "teek_ns_client.h"
#include "smc.h"
#include <huawei_platform/log/imonitor.h>
#define IMONITOR_TA_CRASH_EVENT_ID      (901002003)
#include "tlogger.h"

extern void wakeup_tc_siq(void);

static int cmd_need_archivelog = 0;
static LIST_HEAD(cmd_monitor_list);
static int cmd_monitor_list_size = 0;
#define MAX_CMD_MONITOR_LIST 200
static DEFINE_MUTEX(cmd_monitor_lock);
struct cmd_monitor {
	struct list_head list;
	struct timespec sendtime;
	int count ;
	bool returned;
	bool isReported;
	unsigned int pid;
	unsigned int tid;
	char pname[TASK_COMM_LEN];
	long timetotal;
};
static struct delayed_work cmd_monitor_work;
static struct delayed_work cmd_monitor_work_archive;
void tzdebug_archivelog(void)
{
	schedule_delayed_work(&cmd_monitor_work_archive, usecs_to_jiffies(0));
}

static int get_pid_name(pid_t pid,char* comm,size_t size)
{
	struct task_struct *task;
	int sret;
	if (size <= TASK_COMM_LEN - 1 || !comm) {
		return -1;
	}
	rcu_read_lock();
	task = find_task_by_vpid(pid);
	if (task != NULL) {
		get_task_struct(task);
	}
	rcu_read_unlock();
	if (task != NULL) {
		sret = strncpy_s(comm, size, task->comm, strlen(task->comm));
		if (sret != 0) {
			tloge("memcpy_s faild\n");
		}
		put_task_struct(task);
		return sret;
	}
	return -1;

}

static void cmd_monitor_tick(void)
{
	long timedif;
	struct cmd_monitor *monitor = NULL;
	struct cmd_monitor *tmp = NULL;
	struct timespec nowtime = current_kernel_time();
	mutex_lock(&cmd_monitor_lock);
	list_for_each_entry_safe(monitor, tmp, &cmd_monitor_list, list) {
		if (monitor->returned ==true) {
			tloge("[cmd_monitor_tick] pid=%d,pname=%s,tid=%d,count=%d timetotal=%ld us returned\n", monitor->pid, monitor->pname, monitor->tid, monitor->count, monitor->timetotal);
			list_del(&monitor->list);
			kfree(monitor);
			cmd_monitor_list_size--;
			continue;
		}
		/* not return, we need to check  */
		timedif = 1000*nowtime.tv_sec-1000*monitor->sendtime.tv_sec+nowtime.tv_nsec/1000000-monitor->sendtime.tv_nsec/1000000;

		/* if cmd timeout 5 s, we log the teeos log,and report*/
		if (timedif > 5*1000 && !monitor->isReported) {
			monitor->isReported = true;
			tloge("[cmd_monitor_tick] pid=%d,pname=%s,tid=%d,timedif=%ld ms and report\n", monitor->pid, monitor->pname, monitor->tid, timedif);
			/* print tee stask*/
			cmd_need_archivelog = 1;
			wakeup_tc_siq();
		} else if (timedif > 1*1000) {
			tloge("[cmd_monitor_tick] pid=%d,pname=%s,tid=%d,timedif=%ld ms\n", monitor->pid, monitor->pname, monitor->tid, timedif);
        }
        }
        tlogi("[cmd_monitor_tick] cmd_monitor_list_size=%d\n",cmd_monitor_list_size);
        if (cmd_monitor_list_size > 0) {
		/* if have cmd in monitor list, we need tick*/
		schedule_delayed_work(&cmd_monitor_work, usecs_to_jiffies(1000000));
        }
        mutex_unlock(&cmd_monitor_lock);
}
static void cmd_monitor_tickfn(struct work_struct *work)
{
        (void)(work);
        cmd_monitor_tick();
}
static void cmd_monitor_archivefn(struct work_struct *work)
{
        (void)(work);
	if( tlogger_store_lastmsg() < 0)
		tloge("[cmd_monitor_tick]tlogger_store_lastmsg failed\n");
}


static struct cmd_monitor* init_monitor_locked(void)
{
        struct cmd_monitor* newitem;
        int pidnameresult;
        newitem = kzalloc(sizeof(struct cmd_monitor),GFP_KERNEL);/*lint !429 !593*/
        if (newitem == NULL) {
                tloge("[cmd_monitor_tick]kmalloc faild\n");
                return NULL;
        }
        newitem->sendtime = current_kernel_time();
        newitem->count = 1;
        newitem->returned = false;
        newitem->isReported = false;
        newitem->pid = current->tgid;
        newitem->tid = current->pid;
        pidnameresult = get_pid_name(newitem->pid, newitem->pname, sizeof(newitem->pname));
        if (pidnameresult != 0) {
                newitem->pname[0] = '\0';
        }
        INIT_LIST_HEAD(&newitem->list);
        list_add_tail(&newitem->list, &cmd_monitor_list);
        cmd_monitor_list_size++;
        return newitem;
}

void cmd_monitor_log(TC_NS_SMC_CMD *cmd)
{
	int foundFlag =0;
	unsigned int pid;
	unsigned int tid;
	struct cmd_monitor *monitor = NULL;
	struct cmd_monitor* newitem;
	if (cmd == NULL)
	{
		return;
	}
	pid = current->tgid;
	tid = current->pid;
	mutex_lock(&cmd_monitor_lock);
	do {
		list_for_each_entry(monitor, &cmd_monitor_list, list) {
			if(monitor->pid == pid && monitor->tid == tid){
				foundFlag = 1;
				/* restart*/
				monitor->sendtime = current_kernel_time();
				monitor->count++;
				monitor->returned = false;
				monitor->isReported = false;
				break;
			}
		}
		if (foundFlag == 0) {
			if (cmd_monitor_list_size > MAX_CMD_MONITOR_LIST-1) {
				tloge("[cmd_monitor_tick]MAX_CMD_MONITOR_LIST\n");
				break;
			}
			newitem = init_monitor_locked();
			if (newitem == NULL) {
				tloge("[cmd_monitor_tick]init_monitor failed\n");
				break;
			}
			/* the first cmd will cause timer*/
			if (cmd_monitor_list_size == 1) {
				schedule_delayed_work(&cmd_monitor_work, usecs_to_jiffies(1000000));
			}
		}
        }while(0);
        mutex_unlock(&cmd_monitor_lock);
}

void cmd_monitor_logend(TC_NS_SMC_CMD *cmd)
{
	unsigned int pid;
	unsigned int tid;
	struct cmd_monitor *monitor = NULL;
	if (cmd == NULL)
	{
		return;
	}
	pid = current->tgid;
	tid = current->pid;
	mutex_lock(&cmd_monitor_lock);
	list_for_each_entry(monitor, &cmd_monitor_list, list) {
		if(monitor->pid == pid && monitor->tid == tid && monitor->returned ==false){
			struct timespec nowtime = current_kernel_time();
			long timedif = 1000000*nowtime.tv_sec-1000000*monitor->sendtime.tv_sec+nowtime.tv_nsec/1000-monitor->sendtime.tv_nsec/1000;
			monitor->timetotal += timedif;
			monitor->returned = true;
			/* we need set all monitor.returned = true ,dont break;*/
		}
	}
	mutex_unlock(&cmd_monitor_lock);
}
void do_cmd_need_archivelog(void)
{
	if(cmd_need_archivelog == 1) {
		cmd_need_archivelog =0;
		schedule_delayed_work(&cmd_monitor_work_archive, usecs_to_jiffies(1000000));
	}
}
void init_cmd_monitor(void)
{
	INIT_DEFERRABLE_WORK(&cmd_monitor_work, cmd_monitor_tickfn);
	INIT_DEFERRABLE_WORK(&cmd_monitor_work_archive, cmd_monitor_archivefn);
}
