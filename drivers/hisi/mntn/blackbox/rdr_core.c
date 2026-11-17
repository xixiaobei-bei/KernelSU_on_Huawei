/*
 * rdr_core.c
 *
 * blackbox. (kernel run data recorder.)
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/hardirq.h>
#include <linux/syscalls.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/reboot.h>
#include <linux/export.h>
#include <linux/version.h>


#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/util.h>
#include <linux/hisi/hisi_bootup_keypoint.h>
#include <pr_log.h>
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/debug.h>
#include <uapi/linux/sched/types.h>
#endif
#define PR_LOG_TAG BLACKBOX_TAG

#include <securec.h>
#include "rdr_inner.h"
#include "rdr_field.h"
#include "rdr_print.h"
#include "rdr_debug.h"
#include "../bl31/hisi_bl31_exception.h"
#ifdef CONFIG_HISI_BB_SHUTDOWN_REASON
#include "rdr_poweroff_log.h"
#endif
#ifdef CONFIG_HISI_KERNELDUMP
#include "../../memory_dump/kernel_dump.h"
#endif

static struct semaphore rdr_sem;
static LIST_HEAD(g_rdr_syserr_list);
static DEFINE_SPINLOCK(g_rdr_syserr_list_lock);
static struct wakeup_source blackbox_wl;
static struct rdr_exception_info_s *g_exce_info = NULL;

static void rdr_register_system_error(u32 modid, u32 arg1, u32 arg2)
{
	struct rdr_syserr_param_s *p = NULL;
	struct rdr_exception_info_s *p_exce_info = NULL;
	struct list_head *cur = NULL;
	struct list_head *next = NULL;
	struct rdr_syserr_param_s *e_cur = NULL;
	int exist = 0;

	BB_PRINT_START();

	p_exce_info = rdr_get_exception_info(modid);
	if (p_exce_info == NULL) {
		BB_PRINT_ERR("get exception failed, modid 0x%x\n", modid);
		return;
	}

	p = kzalloc(sizeof(*p), GFP_ATOMIC);
	if (p == NULL) {
		BB_PRINT_ERR("kzalloc rdr_syserr_param_s faild\n");
		return;
	}

	p->modid = modid;
	p->arg1 = arg1;
	p->arg2 = arg2;

	spin_lock(&g_rdr_syserr_list_lock);

	(void)rdr_exception_trace_record(p_exce_info->e_reset_core_mask,
		p_exce_info->e_from_core, p_exce_info->e_exce_type, p_exce_info->e_exce_subtype);

	if (p_exce_info->e_reentrant == (u32)RDR_REENTRANT_DISALLOW) {
		list_for_each_safe(cur, next, &g_rdr_syserr_list) {
			e_cur = list_entry(cur, struct rdr_syserr_param_s, syserr_list);
			if (e_cur->modid == p->modid) {
				exist = 1;
				BB_PRINT_ERR("exception:[0x%x] disallow reentrant.  return\n", modid);
				break;
			}
		}
	}

	if (exist == 0)
		list_add_tail(&p->syserr_list, &g_rdr_syserr_list);
	else if (exist == 1)
		kfree(p);
	spin_unlock(&g_rdr_syserr_list_lock);
	BB_PRINT_END();
}

void rdr_system_error(u32 modid, u32 arg1, u32 arg2)
{
	char *modid_str = NULL;
#ifdef CONFIG_HUAWEI_PRINTK_CTRL
	printk_level_setup(LOGLEVEL_DEBUG);
#endif
	BB_PRINT_START();
	if (in_atomic() || irqs_disabled() || in_irq())
		BB_PRINT_ERR("%s: in atomic or irqs disabled or in irq\n", __func__);
	modid_str = blackbox_get_modid_str(modid);
	BB_PRINT_PN("%s: blackbox receive exception modid is [0x%x][%s]!\n",
		     __func__, modid, modid_str);
	show_stack(current, NULL);
	if (!rdr_init_done()) {
		BB_PRINT_ERR("rdr init faild!\n");
		BB_PRINT_END();
#ifdef CONFIG_HUAWEI_PRINTK_CTRL
		printk_level_setup(sysctl_printk_level);
#endif
		return;
	}
	rdr_register_system_error(modid, arg1, arg2);
	up(&rdr_sem);
	BB_PRINT_END();
#ifdef CONFIG_HUAWEI_PRINTK_CTRL
	printk_level_setup(sysctl_printk_level);
#endif
}
EXPORT_SYMBOL(rdr_system_error);

struct rdr_exception_info_s *rdr_get_exce_info(void)
{
	return g_exce_info;
}


void rdr_syserr_process_for_ap(u32 modid, u64 arg1, u64 arg2)
{
	struct rdr_exception_info_s *p_exce_info = NULL;
	char date[DATATIME_MAXLEN];
	struct rdr_syserr_param_s p;

	BB_PRINT_START();

#ifdef CONFIG_HISI_KERNELDUMP
	(void)ko_dump();
#endif
#ifdef CONFIG_HUAWEI_PRINTK_CTRL
	printk_level_setup(LOGLEVEL_DEBUG);
#endif

	pr_emerg("%s", linux_banner);

	p.modid = modid;
	p.arg1 = arg1;
	p.arg2 = arg2;
	preempt_disable();
	show_stack(current, NULL);

	if (!rdr_init_done()) {
		BB_PRINT_ERR("rdr init faild!\n");
		BB_PRINT_END();
		return;
	}

	rdr_field_baseinfo_reinit();
	rdr_save_args(modid, arg1, arg2);
	p_exce_info = rdr_get_exception_info(modid);
	if (p_exce_info == NULL) {
		preempt_enable();
		BB_PRINT_ERR("get exception info faild.  return\n");
		return;
	}

	(void)rdr_exception_trace_record(p_exce_info->e_reset_core_mask,
		p_exce_info->e_from_core, p_exce_info->e_exce_type, p_exce_info->e_exce_subtype);

	/* dfx_syserr_loop_test must be called between rdr_exception_trace_record&record_exce_type */
	hisi_syserr_loop_test();

	record_exce_type(p_exce_info);
	(void)memset_s(date, DATATIME_MAXLEN, 0, DATATIME_MAXLEN);
	(void)snprintf(date, DATATIME_MAXLEN, "%s-%08lld", rdr_get_timestamp(), rdr_get_tick());

	rdr_fill_edata(p_exce_info, date);
	(void)rdr_notify_module_dump(modid, p_exce_info, NULL);
	rdr_record_reboot_times2mem();
	rdr_notify_module_reset(modid, p_exce_info);

	preempt_enable();
#ifdef CONFIG_HUAWEI_PRINTK_CTRL
	printk_level_setup(sysctl_printk_level);
#endif
	BB_PRINT_END();
}

/*
 * Description:    judge whether need save mntndump log, after call rdr_system_error for reset
 * Return:         true:need save; false:not need save
 */
bool need_save_mntndump_log(u32 reboot_type_s)
{
	if (reboot_type_s >= REBOOT_REASON_LABEL3 &&
		reboot_type_s < REBOOT_REASON_LABEL4)
		return true;

	return false;
}

static void rdr_module_dump(struct rdr_exception_info_s *p_exce_info, char *path, u32 mod_id)
{
	u32 mask;
	u32 cur_mask = 0;
	const int inter_ms = 100;
	int i = 0;
	int wait_dumplog_timeout = rdr_get_dumplog_timeout();

	mask = rdr_notify_module_dump(mod_id, p_exce_info, path);

	BB_PRINT_PN("rdr_notify_module_dump done. return mask=[0x%x]\n", mask);

	/*
	 * The value of mask is obtained by p_exce_info->e_notify_core_mask,
	 * when mask is 0, do not follow the RDR framework to export log
	 * Confirmed by: Liu Hailong
	 */
	if (mask != 0) {
		while (wait_dumplog_timeout > 0) {
			cur_mask = rdr_get_dump_result(mod_id);
			if (mask != cur_mask) {
				BB_PRINT_PN("%s: wait for dump\n", __func__);
				msleep(inter_ms);
				wait_dumplog_timeout -= inter_ms;
				i++;
			} else {
				BB_PRINT_PN
				    ("wait for dump done. use time:[%d], cur_maks[0x%x]\n",
				     i * inter_ms, cur_mask);
				break;
			}
			BB_PRINT_PN("wait for dump done. current status:[0x%x]\n", cur_mask);
		}
		if (wait_dumplog_timeout <= 0)
			BB_PRINT_PN
			    ("wait for dump status timeout... cur_mask[0x%x], target_mask[0x%x]\n",
			     cur_mask, mask);
	}

	rdr_field_dumplog_done();

	if (mask != 0) {
		if (check_himntn(HIMNTN_GOBAL_RESETLOG)) {
			if (mask != RDR_HIFI)
				rdr_save_cur_baseinfo(path);

			/*
			 * If the whole system needs to be reset for this exception,
			 * it means that the log has not been completed to save.
			 */
			if ((p_exce_info->e_reset_core_mask & RDR_AP) &&
				need_save_mntndump_log(p_exce_info->e_exce_type)) {
				/* Some logs need to be saved after reset and restart */
				bbox_save_done(path, BBOX_SAVE_STEP1);
			} else {
				/* All logs in this exception directory have been saved. */
				bbox_save_done(path, BBOX_SAVE_STEP_DONE);
			}

			if (!in_atomic() && !irqs_disabled() && !in_irq())
				/* Ensure all previous file system related operations can be completed */
				SYS_SYNC();
		}
	}

	rdr_field_procexec_done();
}

static void rdr_save_log(struct rdr_exception_info_s *p_exce_info,
				char *path, u32 mod_id, u32 path_len)
{
	int ret;
	bool need_save_log = false;
	bool is_save_done = true;
	char date[DATATIME_MAXLEN] = {'\0'};
	struct semaphore *tmp_sem = NULL;

	if (path_len < PATH_MAXLEN) {
		BB_PRINT_ERR("%s: path_len is too small\n", __func__);
		return;
	}

	need_save_log = rdr_check_log_rights();
	if (p_exce_info->e_notify_core_mask != 0) {
		if (need_save_log) {
			if (rdr_create_exception_path(p_exce_info, path, date, DATATIME_MAXLEN) != 0) {
				BB_PRINT_ERR("create exception path error\n");
				return;
			}
		} else {
			ret = snprintf_s(date, DATATIME_MAXLEN, DATATIME_MAXLEN - 1, "%s-%08lld",
				  rdr_get_timestamp(), rdr_get_tick());
			if (unlikely(ret < 0)) {
				BB_PRINT_ERR("[%s], snprintf_s date ret %d!\n", __func__, ret);
				return;
			}
			is_save_done = false;
		}
	} else {
		/* if no dump need(like modem-reboot), don't create exc-dir, but date is a must. */
		(void)memset_s(date, DATATIME_MAXLEN, 0, DATATIME_MAXLEN);
		ret = snprintf_s(date, DATATIME_MAXLEN, DATATIME_MAXLEN - 1, "%s-%08lld",
			  rdr_get_timestamp(), rdr_get_tick());
		if (unlikely(ret < 0)) {
			BB_PRINT_ERR("[%s], snprintf_s ret %d!\n", __func__, ret);
			return;
		}
	}
	rdr_fill_edata(p_exce_info, date);

	if (p_exce_info->e_exce_type != LPM3_S_EXCEPTION)
		(void)rdr_save_history_log(p_exce_info, date, DATATIME_MAXLEN,
			    is_save_done, get_boot_keypoint());
	if (need_save_log) {
		rdr_save_pstore_log(p_exce_info, path);
		rdr_module_dump(p_exce_info, path, mod_id);
		/* notify to save the clear text */
		tmp_sem = get_cleartext_sem();
		up(tmp_sem);
	}
}

static void rdr_syserr_process(struct rdr_syserr_param_s *p)
{
	int reboot_times;
	int max_reboot_times = rdr_get_reboot_times();
	u32 mod_id = p->modid;

	struct rdr_exception_info_s *p_exce_info = NULL;
	char path[PATH_MAXLEN];

	BB_PRINT_START();

	__pm_stay_awake(&blackbox_wl); /* make sure that the task can not be interrupted by suspend. */
	rdr_field_baseinfo_reinit();
	rdr_save_args(p->modid, p->arg1, p->arg2);
	p_exce_info = rdr_get_exception_info(mod_id);

	while (1) {
		if (rdr_get_suspend_state()) {
			BB_PRINT_PN("%s: wait for suspend\n", __func__);
			msleep(50);
		} else {
			break;
		}
	}

	if (p_exce_info == NULL) {
		(void)rdr_save_history_log_for_undef_exception(p);
		__pm_relax(&blackbox_wl);
		BB_PRINT_ERR("get exception info faild.  return\n");
		return;
	}

	/*
	 * If the whole system needs to be reset for this exception,
	 * you need to record the reset reason in the PMU register,
	 * otherwise only write it to history.log.
	 */
	if (p_exce_info->e_reset_core_mask & RDR_AP)
		record_exce_type(p_exce_info);

	BB_PRINT_PN("start saving data\n");
	rdr_set_saving_state(1);

	rdr_print_one_exc(p_exce_info);
	rdr_save_log(p_exce_info, path, mod_id, PATH_MAXLEN);

	rdr_set_saving_state(0);
	rdr_callback(p_exce_info, mod_id, path, PATH_MAXLEN);
	BB_PRINT_PN("saving data done\n");
	rdr_count_size();
	BB_PRINT_PN("rdr_count_size: done\n");

	if (p_exce_info->e_upload_flag == (u32)RDR_UPLOAD_YES)
		BB_PRINT_PN("rdr_upload log: done\n");

	BB_PRINT_PN("rdr_notify_module_reset: start\n");
	/* check if the last reset was triggered by AP  */
	if (p_exce_info->e_reset_core_mask & RDR_AP) {
		rdr_record_reboot_times2mem();
		reboot_times = rdr_record_reboot_times2file();
		BB_PRINT_PN("ap has reboot %d times\n", reboot_times);
		if (max_reboot_times < reboot_times)
			/* reset the file of reboot_times */
			rdr_reset_reboot_times();
	}
	rdr_notify_module_reset(mod_id, p_exce_info);
	BB_PRINT_PN("rdr_notify_module_reset: done\n");

	__pm_relax(&blackbox_wl);
	BB_PRINT_END();
}

bool rdr_syserr_list_empty(void)
{
	return list_empty(&g_rdr_syserr_list);
}

void rdr_syserr_list_print(void)
{
	struct list_head *cur = NULL;
	struct rdr_syserr_param_s *e_cur = NULL;
	struct list_head *next = NULL;
	struct rdr_exception_info_s *p_exce_info = NULL;

	BB_PRINT_PN("============%s start=============\n", __func__);
	BB_PRINT_PN("empty? [%s]\n", rdr_syserr_list_empty() ? "true" : "false");
	spin_lock(&g_rdr_syserr_list_lock);
	list_for_each_safe(cur, next, &g_rdr_syserr_list) {
		e_cur = list_entry(cur, struct rdr_syserr_param_s, syserr_list);
		p_exce_info = rdr_get_exception_info(e_cur->modid);
		if (p_exce_info == NULL) {
			BB_PRINT_ERR("exception info is NULL\n");
			continue;
		}
		rdr_print_one_exc(p_exce_info);
		p_exce_info = NULL;
	}
	spin_unlock(&g_rdr_syserr_list_lock);
	BB_PRINT_PN("============%s end=============\n", __func__);
}

static int rdr_exception_priority_process(void)
{
	struct list_head *cur = NULL;
	struct list_head *next = NULL;
	struct list_head *process = NULL;
	struct rdr_syserr_param_s *e_cur = NULL;
	struct rdr_syserr_param_s *e_process = NULL;
	struct rdr_exception_info_s *p_exce_info = NULL;
	u32 e_priority = RDR_PPRI_MAX;

	spin_lock(&g_rdr_syserr_list_lock);
	list_for_each_safe(cur, next, &g_rdr_syserr_list) {
		e_cur = list_entry(cur, struct rdr_syserr_param_s, syserr_list);
		p_exce_info = rdr_get_exception_info(e_cur->modid);
		if (p_exce_info == NULL) {
			BB_PRINT_ERR("rdr_get_exception_info fail\n");
			if (process == NULL) {
				process = cur;
				e_process = e_cur;
			}
			continue;
		}
		if (p_exce_info->e_process_priority >= RDR_PPRI_MAX)
			BB_PRINT_ERR("invalid prio[%u], current modid [0x%x]\n",
			    p_exce_info->e_process_priority, e_cur->modid);
		/* find the highest priority of all received exceptions in the list */
		if (p_exce_info->e_process_priority < e_priority) {
			BB_PRINT_PN("current prio[%u], current modid [0x%x]\n",
			    p_exce_info->e_process_priority, e_cur->modid);
			process = cur;
			e_process = e_cur;
			e_priority = p_exce_info->e_process_priority;
		}
	}
	if (process == NULL || e_process == NULL) {
		BB_PRINT_ERR("exception: NULL\n");
		spin_unlock(&g_rdr_syserr_list_lock);
		return -1;
	}

	list_del(process);
	spin_unlock(&g_rdr_syserr_list_lock);

#ifdef CONFIG_HUAWEI_PRINTK_CTRL
	printk_level_setup(LOGLEVEL_DEBUG);
#endif
	rdr_syserr_process(e_process);
#ifdef CONFIG_HUAWEI_PRINTK_CTRL
	printk_level_setup(sysctl_printk_level);
#endif

	kfree(e_process);
	process = NULL;
	e_process = NULL;

	return 0;
}

static int rdr_main_thread_body(void *arg)
{
	long jiffies_time;
	int ret;

	BB_PRINT_START();

	while (!kthread_should_stop()) {
		jiffies_time = msecs_to_jiffies(5000);
		if (down_timeout(&rdr_sem, jiffies_time)) {
			if (rdr_syserr_list_empty())
				continue;
		}
		BB_PRINT_DBG
		    ("%s: enter into a new while =============\n", __func__);
		BB_PRINT_DBG
		    ("============wait for fs ready start =============\n");
		while (rdr_wait_partition(PATH_MNTN_PARTITION, RDR_WAIT_PARTITION_TIME) != 0);
		BB_PRINT_DBG
		    ("============wait for fs ready e n d =============\n");
		while (!rdr_syserr_list_empty()) {
			ret = rdr_exception_priority_process();
			if (ret < 0)
				continue;
		}
	}
	BB_PRINT_END();
	return 0;
}

static bool init_done = false; /* default value is false */
bool rdr_init_done(void)
{
	struct semaphore *tmp_sem = NULL;

	if (init_done)
		return init_done;
	rdr_savebuf_sema_init();
	if (rdr_common_early_init() != 0) {
		BB_PRINT_ERR("rdr_common_early_init faild\n");
		return init_done;
	}
	if (rdr_common_init() != 0) {
		BB_PRINT_ERR("rdr_common_init faild\n");
		return init_done;
	}
	if (rdr_field_init() != 0) {
		BB_PRINT_ERR("rdr_field_init faild\n");
		return init_done;
	}

	if (rdr_exception_trace_init() != 0) {
		BB_PRINT_ERR("rdr_exception_trace_init faild\n");
		return init_done;
	}

#ifdef CONFIG_HISI_BB_SHUTDOWN_REASON
	if (rdr_poweroff_log_init() != 0)
		BB_PRINT_ERR("rdr_poweroff_log_init faild\n");
#endif

	sema_init(&rdr_sem, 0);

	tmp_sem = get_cleartext_sem();
	sema_init(tmp_sem, 0);

	init_done = true;

	return init_done;
}

static s32 __init rdr_init(void)
{
	struct task_struct *rdr_main = NULL;
	struct task_struct *rdr_bootcheck = NULL;
	struct task_struct *rdr_cleartext = NULL;
	struct sched_param param;
	int ret;

	BB_PRINT_START();
	if (!rdr_init_done()) {
		BB_PRINT_ERR("init environment faild\n");
		goto err;
	}

	if (get_pmu_reset_base_addr()) {
		BB_PRINT_ERR("[%s], get_pmu_reset_base_addr fail\n", __func__);
		goto err;
	}

	ret = rdr_register_cleartext_ops(RDR_EXCEPTION_TRACE, rdr_exception_trace_cleartext_print);
	if (unlikely(ret < 0)) {
		BB_PRINT_ERR(
		       "[%s], register rdr_exception_trace_cleartext_print fail, ret = [%d]\n",
		       __func__, ret);
		goto err;
	}

	wakeup_source_init(&blackbox_wl, "blackbox");
	rdr_main = kthread_run(rdr_main_thread_body, NULL, "bbox_main");
	if (rdr_main == NULL) {
		BB_PRINT_ERR("create thread rdr_main_thread faild\n");
		wakeup_source_trash(&blackbox_wl);
		goto err;
	}

	param.sched_priority = BBOX_RT_PRIORITY;
	if (sched_setscheduler(rdr_main, SCHED_FIFO, &param)) {
		BB_PRINT_ERR("sched_setscheduler rdr_bootcheck_thread faild\n");
		kthread_stop(rdr_main);
		wakeup_source_trash(&blackbox_wl);
		goto err;
	}
	rdr_bootcheck =
	    kthread_run(rdr_bootcheck_thread_body, NULL, "bbox_bootcheck");
	if (rdr_bootcheck == NULL) {
		BB_PRINT_ERR("create thread rdr_bootcheck_thread faild\n");
		kthread_stop(rdr_main);
		wakeup_source_trash(&blackbox_wl);
		goto err;
	}

	rdr_cleartext = kthread_run(rdr_cleartext_body, NULL, "bbox_cleartext");
	if (rdr_cleartext == NULL)
		BB_PRINT_ERR("create thread rdr_cleartext faild\n");


	/* notify bl31 to initialize it's mntn module */
	rdr_init_sucess_notify_bl31();

	BB_PRINT_END();
	return 0;
err:
	BB_PRINT_END();
	return -1;
}

static void __exit rdr_exit(void)
{
	rdr_dump_exit();
}

core_initcall(rdr_init);
module_exit(rdr_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("black box. kernel run data recorder");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
