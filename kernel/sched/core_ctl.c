/*
 * core_ctrl.c
 *
 * Count the average number of threads and load of each cluster,
 * isolate unnecessary cpu
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#define pr_fmt(fmt)	"core_ctl: " fmt

#include <linux/core_ctl.h>
#include <linux/init.h>
#include <linux/notifier.h>
#include <linux/cpu.h>
#include <linux/cpumask.h>
#include <linux/topology.h>
#include <linux/cpufreq.h>
#include <linux/hisi_rtg.h>
#include <linux/kthread.h>
#include <linux/percpu.h>
#include <linux/math64.h>
#include <linux/version.h>
#include "sched.h"
#include "walt.h"
#include <linux/sched/rt.h>
#include <linux/capability.h>
#include <linux/version.h>
#include <uapi/linux/sched/types.h>

#include <trace/events/sched.h>

#include "frame/frame.h"
#include <securec.h>

/*
 * the type definition of cpu is not unified in kernel,
 * which will cause pclint violation. In order to satisfy
 * pclint while respecting kernel native code, we have to
 * redefine some cpu related macro here
 */
#if NR_CPUS != 1
#undef for_each_cpu
#define for_each_cpu(cpu, mask)				\
	for ((cpu) = -1;				\
	     (cpu) = (int)cpumask_next((cpu), (mask)),	\
	     (cpu) < (int)nr_cpu_ids;)
#endif

#define break_if_fail_else_add_ret(ret, count) \
	do {				       \
		if ((ret) < 0)		       \
			break;		       \
		else			       \
			(count) += (ret);      \
	} while (0)

#define TRANS_BASE	10
#define NOT_PREFERRED_NUM	2

struct cluster_data {
	bool inited;
	bool pending;
	bool enable;
	bool spread_affinity;
	unsigned int min_cpus;
	unsigned int max_cpus;
	unsigned int offline_delay_ms;
	unsigned int busy_thres;
	unsigned int idle_thres;
	unsigned int active_cpus;
	unsigned int num_cpus;
	unsigned int nr_isolated_cpus;
	unsigned int need_cpus;
	unsigned int task_thres;
	unsigned int open_thres;
	unsigned int close_thres;
	unsigned int first_cpu;
	unsigned int boost;
	unsigned int capacity;
	unsigned int nrrun;
	unsigned int last_nrrun;
	unsigned int max_nrrun;
	unsigned int last_max_nrrun;
	cpumask_t cpu_mask;
	s64 boost_ts;
	s64 need_ts;
	spinlock_t pending_lock;
	struct task_struct *core_ctl_thread;
	struct list_head lru;
	struct list_head cluster_node;
	struct kobject kobj;
};

struct cpu_data {
	bool is_busy;
	bool not_preferred;
	bool isolated_by_us;
	int cpu;
	unsigned int load;
	u64 isolate_ts;
	u64 isolate_cnt;
	u64 isolated_time;
	struct cluster_data *cluster;
	struct list_head sib;
};

static DEFINE_PER_CPU(struct cpu_data, cpu_state);
static DEFINE_SPINLOCK(state_lock);
static LIST_HEAD(cluster_list);
static bool g_initialized;
static unsigned int g_core_ctl_check_interval_ms = UPDATE_INIT_20MS;
static struct kobject *core_ctl_global_kobject;

static void apply_need(struct cluster_data *state, bool bypass_time_limit);
static void wake_up_core_ctl_thread(struct cluster_data *state);
static unsigned int get_active_cpu_count(const struct cluster_data *cluster);
static void update_isolated_time(struct cpu_data *cpu);
static void __core_ctl_set_boost(struct cluster_data *cluster,
				 unsigned int timeout);

/* ========================= sysfs interface =========================== */
static ssize_t store_min_cpus(struct cluster_data *state,
			      const char *buf, size_t count)
{
	unsigned int val;
	unsigned long flags;

	if (kstrtouint(buf, TRANS_BASE, &val) != 0)
		return -EINVAL;

	spin_lock_irqsave(&state_lock, flags);
	state->min_cpus = min(val, state->max_cpus);
	spin_unlock_irqrestore(&state_lock, flags);

	apply_need(state, false);

	return count;
}

static ssize_t show_min_cpus(const struct cluster_data *state, char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			  "%u\n", state->min_cpus);
}

static ssize_t store_max_cpus(struct cluster_data *state,
			      const char *buf, size_t count)
{
	unsigned int val;
	unsigned long flags;

	if (kstrtouint(buf, TRANS_BASE, &val) != 0)
		return -EINVAL;

	spin_lock_irqsave(&state_lock, flags);
	val = min(val, state->num_cpus);
	state->max_cpus = val;
	state->min_cpus = min(state->min_cpus, state->max_cpus);
	spin_unlock_irqrestore(&state_lock, flags);

	apply_need(state, true);

	return count;
}

static ssize_t show_max_cpus(const struct cluster_data *state, char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			  "%u\n", state->max_cpus);
}

static ssize_t store_offline_delay_ms(struct cluster_data *state,
				      const char *buf, size_t count)
{
	unsigned int val;
	unsigned long flags;

	if (kstrtouint(buf, TRANS_BASE, &val) != 0)
		return -EINVAL;

	spin_lock_irqsave(&state_lock, flags);
	state->offline_delay_ms = val;
	spin_unlock_irqrestore(&state_lock, flags);

	apply_need(state, false);

	return count;
}

static ssize_t show_offline_delay_ms(const struct cluster_data *state,
				     char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			  "%u\n", state->offline_delay_ms);
}

static ssize_t store_task_thres(struct cluster_data *state,
				const char *buf, size_t count)
{
	unsigned int val;
	unsigned long flags;

	if (kstrtouint(buf, TRANS_BASE, &val) != 0)
		return -EINVAL;

	if (val < state->num_cpus)
		return -EINVAL;

	spin_lock_irqsave(&state_lock, flags);
	state->task_thres = val;
	spin_unlock_irqrestore(&state_lock, flags);

	apply_need(state, false);

	return count;
}

static ssize_t show_task_thres(const struct cluster_data *state, char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			  "%u\n", state->task_thres);
}

static ssize_t store_busy_thres(struct cluster_data *state,
				const char *buf, size_t count)
{
	unsigned int val;
	unsigned long flags;

	if (kstrtouint(buf, TRANS_BASE, &val) != 0)
		return -EINVAL;

	spin_lock_irqsave(&state_lock, flags);
	state->busy_thres = val;
	spin_unlock_irqrestore(&state_lock, flags);

	apply_need(state, false);

	return count;
}

static ssize_t show_busy_thres(const struct cluster_data *state, char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			  "%u\n", state->busy_thres);
}

static ssize_t store_idle_thres(struct cluster_data *state,
				const char *buf, size_t count)
{
	unsigned int val;
	unsigned long flags;

	if (kstrtouint(buf, TRANS_BASE, &val) != 0)
		return -EINVAL;

	spin_lock_irqsave(&state_lock, flags);
	state->idle_thres = val;
	spin_unlock_irqrestore(&state_lock, flags);

	apply_need(state, false);

	return count;
}

static ssize_t show_idle_thres(const struct cluster_data *state, char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			  "%u\n", state->idle_thres);
}

static ssize_t store_open_thres(struct cluster_data *state,
				const char *buf, size_t count)
{
	unsigned int val;
	unsigned long flags;

	if (kstrtouint(buf, TRANS_BASE, &val) != 0)
		return -EINVAL;

	spin_lock_irqsave(&state_lock, flags);
	state->open_thres = min(val, state->num_cpus * NR_AVG_AMP);
	spin_unlock_irqrestore(&state_lock, flags);

	apply_need(state, false);

	return count;
}

static ssize_t show_open_thres(const struct cluster_data *state,
			       char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			  "%u\n", state->open_thres);
}

static ssize_t store_close_thres(struct cluster_data *state,
				 const char *buf, size_t count)
{
	unsigned int val;
	unsigned long flags;

	if (kstrtouint(buf, TRANS_BASE, &val) != 0)
		return -EINVAL;

	spin_lock_irqsave(&state_lock, flags);
	state->close_thres = min(val, state->num_cpus * NR_AVG_AMP);
	spin_unlock_irqrestore(&state_lock, flags);

	apply_need(state, false);

	return count;
}

static ssize_t show_close_thres(const struct cluster_data *state,
				char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			  "%u\n", state->close_thres);
}

#ifdef CONFIG_HISI_DEBUG_FS
static ssize_t show_need_cpus(const struct cluster_data *state, char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			  "%u\n", state->need_cpus);
}

static ssize_t show_active_cpus(const struct cluster_data *state, char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			  "%u\n", state->active_cpus);
}

static ssize_t show_global_state(const struct cluster_data *state, char *buf)
{
	struct cpu_data *cpud = NULL;
	struct cluster_data *cluster = NULL;
	ssize_t count = 0;
	int ret;
	int cpu;
	unsigned long flags;

	spin_lock_irqsave(&state_lock, flags);
	for_each_possible_cpu(cpu) {
		cpud = &per_cpu(cpu_state, cpu);
		cluster = cpud->cluster;
		if (IS_ERR_OR_NULL(cluster))
			continue;

		if (!cluster->inited)
			continue;

		update_isolated_time(cpud);

		ret = snprintf_s(buf + count, PAGE_SIZE - count,
				    PAGE_SIZE - count - 1, "CPU%u\n", cpu);
		break_if_fail_else_add_ret(ret, count);
		ret = snprintf_s(buf + count, PAGE_SIZE - count,
				 PAGE_SIZE - count - 1,
				 "\tOnline: %u\n", cpu_online(cpu));
		break_if_fail_else_add_ret(ret, count);
		ret = snprintf_s(buf + count, PAGE_SIZE - count,
				 PAGE_SIZE - count - 1,
				 "\tIsolated: %u\n", cpu_isolated(cpu));
		break_if_fail_else_add_ret(ret, count);
		ret = snprintf_s(buf + count, PAGE_SIZE - count,
				 PAGE_SIZE - count - 1,
				 "\tIsolate cnt: %llu\n", cpud->isolate_cnt);
		break_if_fail_else_add_ret(ret, count);
		ret = snprintf_s(buf + count, PAGE_SIZE - count,
				 PAGE_SIZE - count - 1,
				 "\tIsolated time: %llu\n",
				 cpud->isolated_time);
		break_if_fail_else_add_ret(ret, count);
		ret = snprintf_s(buf + count, PAGE_SIZE - count,
				 PAGE_SIZE - count - 1,
				 "\tFirst CPU: %u\n", cluster->first_cpu);
		break_if_fail_else_add_ret(ret, count);
		ret = snprintf_s(buf + count, PAGE_SIZE - count,
				 PAGE_SIZE - count - 1,
				 "\tLoad%%: %u\n", cpud->load);
		break_if_fail_else_add_ret(ret, count);
		ret = snprintf_s(buf + count, PAGE_SIZE - count,
				 PAGE_SIZE - count - 1,
				 "\tIs busy: %u\n", cpud->is_busy);
		break_if_fail_else_add_ret(ret, count);
		ret = snprintf_s(buf + count, PAGE_SIZE - count,
				 PAGE_SIZE - count - 1,
				 "\tNot preferred: %u\n",
				 cpud->not_preferred);
		break_if_fail_else_add_ret(ret, count);

		if (cpu != cluster->first_cpu)
			continue;

		ret = snprintf_s(buf + count, PAGE_SIZE - count,
				 PAGE_SIZE - count - 1,
				 "\tNr running: %u\n", cluster->nrrun);
		break_if_fail_else_add_ret(ret, count);
		ret = snprintf_s(buf + count, PAGE_SIZE - count,
				 PAGE_SIZE - count - 1,
				 "\tMax nr running: %u\n",
				 cluster->max_nrrun);
		break_if_fail_else_add_ret(ret, count);
		ret = snprintf_s(buf + count, PAGE_SIZE - count,
				 PAGE_SIZE - count - 1,
				 "\tActive CPUs: %u\n",
				 get_active_cpu_count(cluster));
		break_if_fail_else_add_ret(ret, count);
		ret = snprintf_s(buf + count, PAGE_SIZE - count,
				 PAGE_SIZE - count - 1,
				 "\tNeed CPUs: %u\n", cluster->need_cpus);
		break_if_fail_else_add_ret(ret, count);
		ret = snprintf_s(buf + count, PAGE_SIZE - count,
				 PAGE_SIZE - count - 1,
				 "\tNr isolated CPUs: %u\n",
				 cluster->nr_isolated_cpus);
		break_if_fail_else_add_ret(ret, count);
		ret = snprintf_s(buf + count, PAGE_SIZE - count,
				 PAGE_SIZE - count - 1,
				 "\tBoost: %u\n", cluster->boost);
		break_if_fail_else_add_ret(ret, count);
	}
	spin_unlock_irqrestore(&state_lock, flags);

	return count;
}
#endif

static ssize_t store_not_preferred(struct cluster_data *state,
				   const char *buf, size_t count)
{
	struct cpu_data *cpud = NULL;
	unsigned int cpu;
	unsigned int val;
	unsigned long flags;
	int ret;

	ret = sscanf_s(buf, "%u %u\n", &cpu, &val);
	if (ret != NOT_PREFERRED_NUM || cpu >= (unsigned int)nr_cpu_ids)
		return -EINVAL;

	spin_lock_irqsave(&state_lock, flags);
	if (cpumask_test_cpu((int)cpu, &state->cpu_mask)) {
		cpud = &per_cpu(cpu_state, cpu);
		cpud->not_preferred = !!val;
	}
	spin_unlock_irqrestore(&state_lock, flags);

	return count;
}

static ssize_t show_not_preferred(const struct cluster_data *state, char *buf)
{
	struct cpu_data *cpud = NULL;
	ssize_t count = 0;
	unsigned long flags;
	int ret;
	int cpu;

	spin_lock_irqsave(&state_lock, flags);
	for_each_cpu(cpu, &state->cpu_mask) {
		cpud = &per_cpu(cpu_state, cpu);
		ret = snprintf_s(buf + count, PAGE_SIZE - count,
				 PAGE_SIZE - count - 1,
				 "CPU#%d: %u\n", cpu, cpud->not_preferred);
		break_if_fail_else_add_ret(ret, count);
	}
	spin_unlock_irqrestore(&state_lock, flags);

	return count;
}

static ssize_t store_update_interval_ms(struct cluster_data *state,
					const char *buf, size_t count)
{
	unsigned int val;
	unsigned long flags;

	if (kstrtouint(buf, TRANS_BASE, &val) != 0)
		return -EINVAL;

	spin_lock_irqsave(&state_lock, flags);
	g_core_ctl_check_interval_ms = clamp(val, UPDATE_INTERVAL_MIN,
					    UPDATE_INTERVAL_MAX);
	spin_unlock_irqrestore(&state_lock, flags);

	request_running_avg_update_ms(g_core_ctl_check_interval_ms);

	return count;
}

static ssize_t show_update_interval_ms(const struct cluster_data *state,
				       char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			  "%u\n", g_core_ctl_check_interval_ms);
}

static ssize_t store_boost(struct cluster_data *state,
			   const char *buf, size_t count)
{
	unsigned int val;

	if (kstrtouint(buf, TRANS_BASE, &val) != 0)
		return -EINVAL;

	__core_ctl_set_boost(state, val);

	return count;
}

static ssize_t show_boost(const struct cluster_data *state,
			  char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			  "%u\n", state->boost);
}

static ssize_t store_enable(struct cluster_data *state,
			    const char *buf, size_t count)
{
	bool enable = false;

	if (kstrtobool(buf, &enable) != 0)
		return -EINVAL;

	if (state->enable != enable) {
		state->enable = enable;
		apply_need(state, false);
	}

	return count;
}

static ssize_t show_enable(const struct cluster_data *state, char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%u\n", state->enable);
}

static ssize_t store_spread_affinity(struct cluster_data *state,
				     const char *buf, size_t count)
{
	unsigned int val;

	if (kstrtouint(buf, TRANS_BASE, &val) != 0)
		return -EINVAL;

	state->spread_affinity = !!val;

	return count;
}

static ssize_t show_spread_affinity(const struct cluster_data *state, char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			  "%u\n", state->spread_affinity);
}

struct core_ctl_attr {
	struct attribute attr;
	ssize_t (*show)(const struct cluster_data *, char *);
	ssize_t (*store)(struct cluster_data *, const char *, size_t count);
};

#define core_ctl_attr_ro(_name)		\
	static struct core_ctl_attr _name =	\
	__ATTR(_name, 0440, show_##_name, NULL)

#define core_ctl_attr_rw(_name)			\
	static struct core_ctl_attr _name =		\
	__ATTR(_name, 0640, show_##_name, store_##_name)

core_ctl_attr_rw(min_cpus);
core_ctl_attr_rw(max_cpus);
core_ctl_attr_rw(offline_delay_ms);
core_ctl_attr_rw(busy_thres);
core_ctl_attr_rw(idle_thres);
core_ctl_attr_rw(task_thres);
core_ctl_attr_rw(open_thres);
core_ctl_attr_rw(close_thres);
#ifdef CONFIG_HISI_DEBUG_FS
core_ctl_attr_ro(need_cpus);
core_ctl_attr_ro(active_cpus);
core_ctl_attr_ro(global_state);
#endif
core_ctl_attr_rw(not_preferred);
core_ctl_attr_rw(update_interval_ms);
core_ctl_attr_rw(boost);
core_ctl_attr_rw(enable);
core_ctl_attr_rw(spread_affinity);

static struct attribute *default_attrs[] = {
	&min_cpus.attr,
	&max_cpus.attr,
	&offline_delay_ms.attr,
	&busy_thres.attr,
	&idle_thres.attr,
	&task_thres.attr,
	&open_thres.attr,
	&close_thres.attr,
#ifdef CONFIG_HISI_DEBUG_FS
	&need_cpus.attr,
	&active_cpus.attr,
	&global_state.attr,
#endif
	&not_preferred.attr,
	&update_interval_ms.attr,
	&boost.attr,
	&enable.attr,
	&spread_affinity.attr,
	NULL
};

#define to_cluster_data(k) container_of(k, struct cluster_data, kobj)
#define to_attr(a) container_of(a, struct core_ctl_attr, attr)
static ssize_t show(struct kobject *kobj, struct attribute *attr, char *buf)
{
	struct cluster_data *data = to_cluster_data(kobj);
	struct core_ctl_attr *cattr = to_attr(attr);
	ssize_t ret = -EIO;

	if (cattr->show != NULL)
		ret = cattr->show(data, buf);

	return ret;
}

static ssize_t store(struct kobject *kobj, struct attribute *attr,
		     const char *buf, size_t count)
{
	struct cluster_data *data = to_cluster_data(kobj);
	struct core_ctl_attr *cattr = to_attr(attr);
	ssize_t ret = -EIO;

	if (cattr->store != NULL)
		ret = cattr->store(data, buf, count);

	return ret;
}

static const struct sysfs_ops sysfs_ops = {
	.show	= show,
	.store	= store,
};

static struct kobj_type ktype_core_ctl = {
	.sysfs_ops	= &sysfs_ops,
	.default_attrs	= default_attrs,
};

/* ==================== runqueue based core count =================== */
int update_misfit_task(void);

static int update_running_avg(bool sched_avg_updated)
{
	struct sched_nr_stat stat;
	unsigned int overflow_nrrun, spare_nrrun;
	unsigned long flags;
	struct cluster_data *cluster = NULL;
	struct cluster_data *temp = NULL;
	int ret = 0;

	spin_lock_irqsave(&state_lock, flags);

	if (!sched_avg_updated) {
		ret = -EPERM;
		goto check_misfit;
	}

	list_for_each_entry(cluster, &cluster_list, cluster_node) {
		overflow_nrrun = 0;
		cluster->last_nrrun = cluster->nrrun;
		cluster->last_max_nrrun = cluster->max_nrrun;

		/* do not take the nrrun into account when the cluster is isolated */
		if (cluster->active_cpus == 0)
			memset(&stat, 0, sizeof(struct sched_nr_stat));
		else
			sched_get_cpus_running_avg(&cluster->cpu_mask, &stat);

		trace_core_ctl_get_nr_running_avg(&cluster->cpu_mask,
						  stat.avg, stat.big_avg,
						  stat.iowait_avg, stat.nr_max);

		cluster->nrrun = stat.avg;
		cluster->max_nrrun = stat.nr_max;
		/*
		 * Big cluster only need to take care of big tasks, but if
		 * there are not enough big cores, big tasks need to be run
		 * on little as well. Thus for little's runqueue stat, it
		 * has to use overall runqueue average, or derive what big
		 * tasks would have to be run on little. The latter approach
		 * is not easy to get given core control reacts much slower
		 * than scheduler, and can't predict scheduler's behavior.
		 */
		if (cluster->cluster_node.prev != &cluster_list) {
			temp = list_prev_entry(cluster, cluster_node);

			/* this cluster is overload,
			 * need share load to a bigger cluster
			 */
			if (stat.avg > cluster->max_cpus * NR_AVG_AMP) {
				overflow_nrrun =
					stat.avg - (cluster->max_cpus *
						    NR_AVG_AMP);
				stat.big_avg = max(overflow_nrrun,
						   stat.big_avg);
			}

			spare_nrrun = temp->max_cpus * NR_AVG_AMP;
			/* prevent unisolate a cluster for little pulse load */
			if (temp->nrrun < spare_nrrun &&
			    (temp->active_cpus > 0 ||
			     stat.big_avg > temp->open_thres)) {
				spare_nrrun -= temp->nrrun;
				spare_nrrun = min(spare_nrrun, stat.big_avg);
				temp->nrrun += spare_nrrun;
				cluster->nrrun -= min(cluster->nrrun,
						      spare_nrrun);
			}

			if (temp->nrrun < temp->close_thres) {
				cluster->nrrun += temp->nrrun;
				temp->nrrun = 0;
			}

			temp->nrrun = DIV_ROUND_UP(temp->nrrun, NR_AVG_AMP);
		}
	}

	if (!list_empty(&cluster_list)) {
		temp = list_last_entry(&cluster_list,
				       struct cluster_data, cluster_node);
		temp->nrrun = DIV_ROUND_UP(temp->nrrun, NR_AVG_AMP);
	}

check_misfit:
	if (!update_misfit_task())
		ret = 0;

	spin_unlock_irqrestore(&state_lock, flags);
	return ret;
}

int update_misfit_task(void)
{
	struct cluster_data *cluster = NULL;
	struct cluster_data *temp = NULL;
	unsigned int misfits, spare, rtg_nr_running;
	int cluster_id, cpu;
	int ret = -ENOSYS;

	list_for_each_entry(cluster, &cluster_list, cluster_node) {
		misfits = 0;
		for_each_cpu(cpu, &cluster->cpu_mask) {
			if (cpu_rq(cpu)->misfit_task_load)
				misfits++;
		}

		if (cluster->cluster_node.prev != &cluster_list) {
			temp = list_prev_entry(cluster, cluster_node);
			if (temp->max_cpus <= temp->nrrun)
				continue;

			spare = temp->max_cpus - temp->nrrun;
			misfits = min(spare, misfits);
			if (misfits) {
				temp->nrrun += misfits;
				cluster->nrrun = (cluster->nrrun > (misfits + 1)) ?
						 (cluster->nrrun - misfits) : 1;
				ret = 0;
			}

			cluster_id = topology_physical_package_id(
					cluster->first_cpu);
			rtg_nr_running = get_cluster_grp_running(cluster_id);
			if (rtg_nr_running > cluster->task_thres) {
				rtg_nr_running -= cluster->task_thres;
				temp->nrrun = max(temp->nrrun, rtg_nr_running);
			}
		}
	}

	cluster = list_first_entry(&cluster_list, struct cluster_data,
				   cluster_node);
	if (!update_frame_isolation() && cluster->nrrun == 0) {
		cluster->nrrun = 1;
		ret = 0;
	}

	return ret;
}

/* adjust needed CPUs based on current runqueue information */
static unsigned int apply_task_need(const struct cluster_data *cluster,
				    unsigned int new_need)
{
	/* only unisolate more cores if there are tasks to run */
	if (cluster->nrrun > new_need)
		new_need = cluster->nrrun;
	else if (cluster->nrrun == new_need && new_need)
		return new_need + 1;

	if (cluster->max_nrrun > cluster->task_thres)
		new_need++;

	return new_need;
}

/* ======================= load based core count  ====================== */

static unsigned int apply_limits(const struct cluster_data *cluster,
				 unsigned int new_cpus)
{
	return clamp(new_cpus, cluster->min_cpus, cluster->max_cpus);
}

static unsigned int get_active_cpu_count(const struct cluster_data *cluster)
{
	return cluster->num_cpus - sched_isolate_count(&cluster->cpu_mask,
						       true);
}

static bool is_active(const struct cpu_data *state)
{
	return cpu_online(state->cpu) && !cpu_isolated(state->cpu);
}

static bool adjustment_possible(const struct cluster_data *cluster,
				unsigned int need)
{
	return (need < cluster->active_cpus || (need > cluster->active_cpus &&
						cluster->nr_isolated_cpus));
}

static bool eval_need(struct cluster_data *cluster, bool bypass_time_limit)
{
	unsigned long flags;
	struct cpu_data *cpud = NULL;
	unsigned int busy_cpus = 0;
	unsigned int last_need;
	int ret;
	bool need_flag = false;
	unsigned int new_need;
	s64 now;
	s64 elapsed = 0;

	if (unlikely(!cluster->inited))
		return 0;

	spin_lock_irqsave(&state_lock, flags);

	cluster->active_cpus = get_active_cpu_count(cluster);
	now = ktime_to_ms(ktime_get());

	if (!cluster->enable) {
		new_need = cluster->num_cpus;
	} else if (cluster->boost) {
		new_need = cluster->max_cpus;
		elapsed = now - cluster->boost_ts;
		elapsed = clamp(elapsed, 0LL, (s64)cluster->boost);
		cluster->boost -= elapsed;
		cluster->boost_ts = now;
		elapsed = 0;
	} else {
		list_for_each_entry(cpud, &cluster->lru, sib) {
			if (walt_cpu_high_irqload(cpud->cpu) ||
			    cpud->load >= cluster->busy_thres)
				cpud->is_busy = true;
			else if (cpud->load < cluster->idle_thres)
				cpud->is_busy = false;
			if (cpud->is_busy)
				busy_cpus++;
		}
		new_need = apply_task_need(cluster, busy_cpus);
	}

	new_need = apply_limits(cluster, new_need);
	need_flag = adjustment_possible(cluster, new_need);
	last_need = cluster->need_cpus;

	if (new_need >= cluster->active_cpus || bypass_time_limit) {
		ret = 1;
	} else {
		if (new_need == last_need) {
			cluster->need_ts = now;
			spin_unlock_irqrestore(&state_lock, flags);
			return 0;
		}

		elapsed = now - cluster->need_ts;
		ret = elapsed >= cluster->offline_delay_ms;
	}

	if (ret) {
		cluster->need_ts = now;
		cluster->need_cpus = new_need;
	}

	trace_core_ctl_eval_need(&cluster->cpu_mask,
				 cluster->nrrun, busy_cpus,
				 cluster->active_cpus, last_need,
				 new_need, elapsed, ret && need_flag);

	spin_unlock_irqrestore(&state_lock, flags);

	return ret && need_flag;
}

static void apply_need(struct cluster_data *cluster, bool bypass_time_limit)
{
	if (eval_need(cluster, bypass_time_limit))
		wake_up_core_ctl_thread(cluster);
}

static void core_ctl_update_busy(int cpu, unsigned int load, bool check_load)
{
	struct cpu_data *cpud = &per_cpu(cpu_state, cpu);
	struct cluster_data *cluster = cpud->cluster;
	bool old_is_busy = cpud->is_busy;
	unsigned int old_load;

	if (IS_ERR_OR_NULL(cluster))
		return;

	if (!cluster->inited)
		return;

	old_load = cpud->load;
	cpud->load = load;

	if (!check_load && cpumask_next(cpu, &cluster->cpu_mask) < nr_cpu_ids)
		return;

	if (check_load && old_load == load)
		return;

	apply_need(cluster, false);

	trace_core_ctl_update_busy(cpu, load, old_is_busy, cpud->is_busy);
}

/* ========================= core count enforcement ==================== */

static void wake_up_core_ctl_thread(struct cluster_data *cluster)
{
	unsigned long flags;

	spin_lock_irqsave(&cluster->pending_lock, flags);
	cluster->pending = true;
	spin_unlock_irqrestore(&cluster->pending_lock, flags);

	wake_up_process(cluster->core_ctl_thread);
}

static void __core_ctl_set_boost(struct cluster_data *cluster,
				 unsigned int timeout)
{
	unsigned long flags;

	if (unlikely(!g_initialized))
		return;

	spin_lock_irqsave(&state_lock, flags);

	if (cluster == NULL)
		cluster = list_first_entry(&cluster_list,
					   struct cluster_data, cluster_node);

	if (timeout > cluster->boost || timeout == 0) {
		cluster->boost = timeout;
		cluster->boost_ts = ktime_to_ms(ktime_get());
	}

	spin_unlock_irqrestore(&state_lock, flags);

	apply_need(cluster, false);

	trace_core_ctl_set_boost(cluster->boost);
}

void core_ctl_set_boost(unsigned int timeout)
{
	__core_ctl_set_boost(NULL, timeout);
}
EXPORT_SYMBOL(core_ctl_set_boost);

void core_ctl_spread_affinity(cpumask_t *allowed_mask)
{
	struct cluster_data *cluster = NULL;

	if (unlikely(!g_initialized || !allowed_mask))
		return;

	if (cpumask_empty(allowed_mask))
		return;

	cluster = list_first_entry(&cluster_list,
				   struct cluster_data, cluster_node);

	if (cluster->enable &&
	    cluster->spread_affinity &&
	    cpumask_subset(allowed_mask, &cluster->cpu_mask) &&
	    cluster->cluster_node.next != &cluster_list) {
		if (capable(CAP_SYS_ADMIN))
			return;

		cluster = list_next_entry(cluster, cluster_node);
		cpumask_or(allowed_mask, allowed_mask, &cluster->cpu_mask);
	}
}

void core_ctl_check(bool sched_avg_updated)
{
	struct cluster_data *cluster = NULL;

	if (unlikely(!g_initialized))
		return;

	if (!update_running_avg(sched_avg_updated)) {
		list_for_each_entry(cluster, &cluster_list, cluster_node) {
			apply_need(cluster, false);
		}
	}
}

static void move_cpu_lru(struct cpu_data *state, bool forward)
{
#ifdef CONFIG_MOVE_CPU_LRU
	list_del(&state->sib);
	if (forward)
		list_add_tail(&state->sib, &state->cluster->lru);
	else
		list_add(&state->sib, &state->cluster->lru);
#endif
}

static void __try_to_isolate(struct cluster_data *cluster,
			     unsigned int need, bool isolate_busy)
{
	struct cpu_data *cpud = NULL;
	struct cpu_data *tmp = NULL;
	unsigned long flags;
	unsigned int num_cpus = cluster->num_cpus;
	unsigned int nr_isolated = 0;
	int ret;

	/*
	 * Protect against entry being removed (and added at tail) by other
	 * thread (hotplug).
	 */
	spin_lock_irqsave(&state_lock, flags);

	list_for_each_entry_safe(cpud, tmp, &cluster->lru, sib) {
		if (num_cpus == 0)
			break;

		num_cpus--;
		if (!is_active(cpud))
			continue;

		if (isolate_busy) {
			if (cluster->active_cpus <= cluster->max_cpus)
				break;
		} else {
			if (cluster->active_cpus == need)
				break;

			if (cpud->is_busy)
				continue;
		}

		pr_debug("Trying to isolate CPU%d\n", cpud->cpu);

		spin_unlock_irqrestore(&state_lock, flags);
		ret = sched_isolate_cpu_unlocked(cpud->cpu);
		spin_lock_irqsave(&state_lock, flags);

		if (!ret) {
			cpud->isolated_by_us = true;
			cpud->isolate_cnt++;
			cpud->isolate_ts = ktime_to_ms(ktime_get());
			move_cpu_lru(cpud, true);
			nr_isolated++;
		} else {
			pr_debug("Unable to isolate CPU%d\n", cpud->cpu);
		}
		cluster->active_cpus = get_active_cpu_count(cluster);
	}

	cluster->nr_isolated_cpus += nr_isolated;

	spin_unlock_irqrestore(&state_lock, flags);
}

static void try_to_isolate(struct cluster_data *cluster, unsigned int need)
{
	__try_to_isolate(cluster, need, false);
	/*
	 * If the number of active CPUs is within the limits, then
	 * don't force isolation of any busy CPUs.
	 */
	if (cluster->active_cpus <= cluster->max_cpus)
		return;

	__try_to_isolate(cluster, need, true);
}

static void update_isolated_time(struct cpu_data *cpu)
{
	u64 ts;

	if (!cpu->isolated_by_us)
		return;

	ts = ktime_to_ms(ktime_get());

	cpu->isolated_time += ts - cpu->isolate_ts;

	cpu->isolate_ts = ts;
}

static void __try_to_unisolate(struct cluster_data *cluster,
			       unsigned int need, bool force)
{
	struct cpu_data *cpud = NULL;
	struct cpu_data *tmp = NULL;
	unsigned long flags;
	unsigned int num_cpus = cluster->num_cpus;
	unsigned int nr_unisolated = 0;
	int ret;

	/*
	 * Protect against entry being removed (and added at tail) by other
	 * thread (hotplug).
	 */
	spin_lock_irqsave(&state_lock, flags);

	list_for_each_entry_safe_reverse(cpud, tmp, &cluster->lru, sib) {
		if (num_cpus == 0)
			break;

		num_cpus--;
		if (!cpud->isolated_by_us)
			continue;
		if ((cpu_online(cpud->cpu) && !cpu_isolated(cpud->cpu)) ||
		    (!force && cpud->not_preferred))
			continue;
		if (cluster->active_cpus == need)
			break;

		pr_debug("Trying to unisolate CPU%d\n", cpud->cpu);

		spin_unlock_irqrestore(&state_lock, flags);
		ret = sched_unisolate_cpu_unlocked(cpud->cpu, false);
		spin_lock_irqsave(&state_lock, flags);

		if (!ret) {
			update_isolated_time(cpud);
			cpud->isolated_by_us = false;
			move_cpu_lru(cpud, false);
			nr_unisolated++;
		} else {
			pr_debug("Unable to unisolate CPU%d\n", cpud->cpu);
		}
		cluster->active_cpus = get_active_cpu_count(cluster);
	}

	cluster->nr_isolated_cpus -= nr_unisolated;

	spin_unlock_irqrestore(&state_lock, flags);
}

static void try_to_unisolate(struct cluster_data *cluster, unsigned int need)
{
	__try_to_unisolate(cluster, need, false);

	if (cluster->active_cpus == need)
		return;

	__try_to_unisolate(cluster, need, true);
}

static void __ref do_core_ctl(struct cluster_data *cluster)
{
	unsigned int need;

	need = apply_limits(cluster, cluster->need_cpus);

	if (adjustment_possible(cluster, need)) {
		pr_debug("Trying to adjust group %u from %u to %u\n",
			 cluster->first_cpu, cluster->active_cpus, need);

		cpu_maps_update_begin();

		if (cluster->active_cpus > need)
			try_to_isolate(cluster, need);
		else if (cluster->active_cpus < need)
			try_to_unisolate(cluster, need);

		cpu_maps_update_done();
	}
}

static int __ref try_core_ctl(void *data)
{
	struct cluster_data *cluster = data;
	unsigned long flags;

	while (1) {
		set_current_state(TASK_INTERRUPTIBLE);
		spin_lock_irqsave(&cluster->pending_lock, flags);
		if (!cluster->pending) {
			spin_unlock_irqrestore(&cluster->pending_lock, flags);
			schedule();
			if (kthread_should_stop())
				break;
			spin_lock_irqsave(&cluster->pending_lock, flags);
		}
		set_current_state(TASK_RUNNING);
		cluster->pending = false;
		spin_unlock_irqrestore(&cluster->pending_lock, flags);

		do_core_ctl(cluster);
	}

	return 0;
}

static int __ref cpuhp_core_ctl_online(unsigned int cpu)
{
	struct cpu_data *state = &per_cpu(cpu_state, cpu);
	struct cluster_data *cluster = state->cluster;
	unsigned long flags;

	if (IS_ERR_OR_NULL(cluster))
		return -ENODEV;

	if (unlikely(!cluster->inited))
		return 0;

	/*
	 * Moving to the end of the list should only happen in
	 * CPU_ONLINE and not on CPU_UP_PREPARE to prevent an
	 * infinite list traversal when thermal (or other entities)
	 * reject trying to online CPUs.
	 */
	spin_lock_irqsave(&state_lock, flags);
	move_cpu_lru(state, false);
	spin_unlock_irqrestore(&state_lock, flags);

	apply_need(cluster, false);

	return 0;
}

static int __ref cpuhp_core_ctl_offline(unsigned int cpu)
{
	struct cpu_data *state = &per_cpu(cpu_state, cpu);
	struct cluster_data *cluster = state->cluster;
	unsigned long flags;

	if (IS_ERR_OR_NULL(cluster))
		return -ENODEV;

	if (unlikely(!cluster->inited))
		return 0;

	/*
	 * We don't want to have a CPU both offline and isolated.
	 * So unisolate a CPU that went down if it was isolated by us.
	 */
	spin_lock_irqsave(&state_lock, flags);
	if (state->isolated_by_us) {
		update_isolated_time(state);
		state->isolated_by_us = false;
		cluster->nr_isolated_cpus--;
	} else {
		/* Move a CPU to the end of the LRU when it goes offline. */
		move_cpu_lru(state, true);
	}

	state->load = 0;
	spin_unlock_irqrestore(&state_lock, flags);

	apply_need(cluster, false);

	return 0;
}

/* ============================ init code ============================== */

static cpumask_var_t g_core_ctl_disable_cpumask;
static bool g_core_ctl_disable_cpumask_present;

static int __init core_ctl_disable_setup(char *str)
{
	if (str == NULL) {
		pr_err("core_ctl: no cmdline str\n");
		return -EINVAL;
	}

	if (*str == 0) {
		pr_err("core_ctl: no valid cmdline\n");
		return -EINVAL;
	}

	alloc_bootmem_cpumask_var(&g_core_ctl_disable_cpumask);

	if (cpulist_parse(str, g_core_ctl_disable_cpumask) < 0) {
		pr_err("core_ctl: parse cpumask err\n");
		free_bootmem_cpumask_var(g_core_ctl_disable_cpumask);
		return -EINVAL;
	}

	g_core_ctl_disable_cpumask_present = true;
	pr_info("disable_cpumask=%*pKbl\n",
		cpumask_pr_args(g_core_ctl_disable_cpumask));

	return 0;
}
early_param("core_ctl_disable_cpumask", core_ctl_disable_setup);

static bool should_skip(const struct cpumask *mask)
{
	if (!g_core_ctl_disable_cpumask_present)
		return false;

	/*
	 * We operate on a cluster basis. Disable the core_ctl for
	 * a cluster, if all of it's cpus are specified in
	 * core_ctl_disable_cpumask
	 */
	return cpumask_subset(mask, g_core_ctl_disable_cpumask);
}

static struct cluster_data *find_cluster_by_first_cpu(unsigned int first_cpu)
{
	struct cluster_data *temp = NULL;

	list_for_each_entry(temp, &cluster_list, cluster_node) {
		if (temp->first_cpu == first_cpu)
			return temp;
	}

	return NULL;
}

static void insert_cluster_by_cap(struct cluster_data *cluster)
{
	struct cluster_data *temp = NULL;

	/* bigger capacity first */
	list_for_each_entry(temp, &cluster_list, cluster_node) {
		if (temp->capacity < cluster->capacity) {
			list_add_tail(&cluster->cluster_node,
				      &temp->cluster_node);
			return;
		}
	}

	list_add_tail(&cluster->cluster_node, &cluster_list);
}

static int cluster_init(const struct cpumask *mask)
{
	struct device *dev = NULL;
	unsigned int first_cpu = cpumask_first(mask);
	struct cluster_data *cluster = NULL;
	struct cpu_data *state = NULL;
	int cpu, ret;
	struct sched_param param = { .sched_priority = MAX_RT_PRIO - 1 };

	/* disabled by cmdline */
	if (should_skip(mask))
		return 0;

	/* cluster data has been inited */
	if (find_cluster_by_first_cpu(first_cpu))
		return 0;

	dev = get_cpu_device(first_cpu);
	if (IS_ERR_OR_NULL(dev)) {
		pr_err("core_ctl: fail to get cpu device\n");
		return -ENODEV;
	}

	pr_info("Creating CPU group %d\n", first_cpu);

	cluster = devm_kzalloc(dev, sizeof(*cluster), GFP_KERNEL);
	if (IS_ERR_OR_NULL(cluster)) {
		pr_err("core_ctl: alloc cluster err\n");
		return -ENOMEM;
	}

	cpumask_copy(&cluster->cpu_mask, mask);
	cluster->num_cpus = cpumask_weight(mask);
	cluster->first_cpu = first_cpu;
	cluster->min_cpus = cluster->num_cpus;
	cluster->max_cpus = cluster->num_cpus;
	cluster->need_cpus = cluster->num_cpus;
	cluster->offline_delay_ms = 100;
	cluster->task_thres = UINT_MAX;
	cluster->nrrun = cluster->num_cpus;
	cluster->enable = true;
	cluster->spread_affinity = true;
	cluster->capacity = capacity_orig_of(first_cpu);
	INIT_LIST_HEAD(&cluster->lru);
	spin_lock_init(&cluster->pending_lock);

	for_each_cpu(cpu, mask) {
		state = &per_cpu(cpu_state, cpu);
		state->cluster = cluster;
		state->cpu = cpu;
		list_add(&state->sib, &cluster->lru);
	}
	cluster->active_cpus = get_active_cpu_count(cluster);

	cluster->core_ctl_thread = kthread_run(try_core_ctl, (void *)cluster,
					       "core_ctl/%u", first_cpu);
	if (IS_ERR(cluster->core_ctl_thread)) {
		pr_err("core_ctl: thread create err\n");
		return PTR_ERR(cluster->core_ctl_thread);
	}

	sched_setscheduler_nocheck(cluster->core_ctl_thread, SCHED_FIFO,
				   &param);

	ret = kobject_init_and_add(&cluster->kobj, &ktype_core_ctl,
				   core_ctl_global_kobject, "cluster%u",
				   topology_physical_package_id(first_cpu));
	if (ret) {
		pr_err("core_ctl: failed to init cluster->kobj: %d\n", ret);
		kobject_put(&cluster->kobj);
		return -ENOMEM;
	}

	if (sysfs_create_link(&dev->kobj, &cluster->kobj, "core_ctl"))
		pr_err("core_trl: symlink creation failed\n");

	cluster->inited = true;

	insert_cluster_by_cap(cluster);

	return 0;
}

static int cpufreq_gov_cb(struct notifier_block *nb, unsigned long val,
			  void *data)
{
	struct cpufreq_govinfo *info = data;

	switch (val) {
	case CPUFREQ_LOAD_CHANGE:
		core_ctl_update_busy(info->cpu, info->load,
				     info->sampling_rate_us != 0);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static struct notifier_block cpufreq_gov_nb = {
	.notifier_call = cpufreq_gov_cb,
};

static int __init core_ctl_init(void)
{
	int cpu, ret;

	if (should_skip(cpu_possible_mask))
		return 0;

	core_ctl_global_kobject = kobject_create_and_add("core_ctl",
						&cpu_subsys.dev_root->kobj);
	if (!core_ctl_global_kobject)
		return -ENOMEM;

	cpuhp_setup_state_nocalls(CPUHP_AP_ONLINE_DYN, "core_ctl:online",
				  cpuhp_core_ctl_online, NULL);

	cpuhp_setup_state_nocalls(CPUHP_CORE_CTRL_DEAD, "core_ctl:dead",
				  NULL, cpuhp_core_ctl_offline);

	cpufreq_register_notifier(&cpufreq_gov_nb, CPUFREQ_GOVINFO_NOTIFIER);

	cpu_maps_update_begin();
	for_each_online_cpu(cpu) {
		ret = cluster_init(topology_core_cpumask(cpu));
		if (ret)
			pr_err("create core ctl group%d failed: %d\n",
			       cpu, ret);
	}
	cpu_maps_update_done();
	g_initialized = true;
	return 0;
}

late_initcall(core_ctl_init);
