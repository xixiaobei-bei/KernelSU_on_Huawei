#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/notifier.h>
#include <linux/topology.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/version.h>
#include <linux/cpu.h>
#include <linux/cpumask.h>
#include <linux/cpufreq.h>
#include <linux/devfreq.h>
#include <linux/cpu_pm.h>
#include <linux/hisi/hisi_drg.h>
#include <linux/platform_device.h>
#include <linux/kthread.h>
#include <linux/topology.h>
#include <linux/of.h>

#define CREATE_TRACE_POINTS
#include <trace/events/hisi_drg.h>

#define CPUFREQ_AMP	1000UL
#define DEVFREQ_AMP	1UL
#define KHZ		1000UL
#define INVALID_STATE	-1

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0))
#define kthread_init_work	init_kthread_work
#define kthread_queue_work	queue_kthread_work
#define kthread_flush_work	flush_kthread_work
#endif

struct drg_freq_device {
	/* copy of cpufreq device's related cpus */
	cpumask_t cpus;
	/* driver pointer of devfreq device */
	struct devfreq *df;
	/* device node for binding drg_device */
	struct device_node *np;
	/* work used to unbind drg_device */
	struct kthread_work destory_work;
	/* list of all binded master drg_device */
	struct list_head master_list;
	/* list of all binded slave drg_device */
	struct list_head slave_list;
	/* list node for g_drg_cpufreq_list or g_drg_devfreq_list */
	struct list_head node;
	/* amplifer used to convert device input freq to Hz */
	unsigned int amp;
	unsigned long cur_freq;
};

struct drg_rule;

struct drg_device {
	/* binded drg_freq_device */
	struct drg_freq_device *freq_dev;
	/* device node parsed from phandle for binding drg freq device */
	struct device_node *np;
	/* the drg rule this device belong to */
	struct drg_rule *ruler;
	/* range divider freq, described as KHz */
	unsigned int *divider;
	/* list node for drg freq device's master list or slave list*/
	struct list_head node;
	/* minimum freq, set by drg rule */
	unsigned long min_freq;
	/* maximum freq, set by drg rule */
	unsigned long max_freq;
	/* thermal clip state for slave, cur state for master */
	int state;
};

struct drg_rule {
	struct kobject kobj;
	/* work used to notify slave device the freq range has changed */
	struct kthread_work exec_work;
	/* list node for g_drg_rule_list */
	struct list_head node;
	/* all master device belong to this rule */
	struct drg_device *master_dev;
	/* all slave device belong to this rule */
	struct drg_device *slave_dev;
	/* rule name*/
	const char *name;
	/* divider number */
	int divider_num;
	/* master device number */
	int master_num;
	/* slave device number */
	int slave_num;
	/* current freq range index */
	int state_idx;
	/* limit slave's minimum freq */
	bool dn_limit;
	/* limit slave's maximum freq */
	bool up_limit;
	/* be aware of slave's thermal clamp */
	bool thermal_enable;
	/* if rule have multi master, use maximum state of these master */
	bool master_vote_max;
	bool enable;
};

static LIST_HEAD(g_drg_rule_list);
static LIST_HEAD(g_drg_cpufreq_list);
static LIST_HEAD(g_drg_devfreq_list);
static DEFINE_PER_CPU(unsigned int, drg_margin);
static DEFINE_RWLOCK(drg_list_lock);
static DEFINE_KTHREAD_WORKER(g_drg_worker);
static struct task_struct *g_drg_work_thread;
static cpumask_t idle_cpumask;
static bool g_drg_initialized;

extern int update_devfreq(struct devfreq *devfreq);

static struct drg_freq_device *drg_get_cpu_dev(unsigned int cpu)
{
	struct drg_freq_device *freq_dev;

	read_lock(&drg_list_lock);
	/* find the freq device of cpu */
	list_for_each_entry(freq_dev, &g_drg_cpufreq_list, node) {
		if (cpumask_test_cpu(cpu, &freq_dev->cpus)) {
			read_unlock(&drg_list_lock);
			return freq_dev;
		}
	}

	read_unlock(&drg_list_lock);
	return NULL;
}

static struct drg_freq_device *drg_get_devfreq_dev(struct devfreq *df)
{
	struct drg_freq_device *freq_dev;

	read_lock(&drg_list_lock);
	/* find the freq device of devfreq */
	list_for_each_entry(freq_dev, &g_drg_devfreq_list, node) {
		if (freq_dev->df == df) {
			read_unlock(&drg_list_lock);
			return freq_dev;
		}
	}

	read_unlock(&drg_list_lock);
	return NULL;
}

static int drg_freq2state(struct drg_rule *ruler,
			  unsigned int *divider, unsigned long freq)
{
	int i, state = 0;

	/* convert freq to freq range index according to divider */
	for (i = 0; i < ruler->divider_num; i++) {
		if (freq >= divider[i])
			state++;
	}

	return state;
}

static int find_proper_upbound(struct drg_freq_device *freq_dev,
			       unsigned long *freq)
{
	struct device *dev;
	int cpu = cpumask_first(&freq_dev->cpus);

	if (cpu < nr_cpu_ids)
		dev = get_cpu_device(cpu);
	else if (freq_dev->df)
		dev = freq_dev->df->dev.parent;
	else
		dev = NULL;

	if (!dev)
		return -ENODEV;

	rcu_read_lock();
	(void) dev_pm_opp_find_freq_floor(dev, freq);
	rcu_read_unlock();

	return 0;
}

static unsigned long drg_check_thermal_limit(struct drg_device *master,
					     unsigned long new_freq)
{
	struct drg_rule *ruler = master->ruler;
	struct drg_device *slave;
	unsigned long ret = new_freq;
	int i, state, thermal_state = INT_MAX;
	int thermal_slave = 0;

	if (!ruler->thermal_enable)
		return ret;

	/* find minimum thermal clamp freq range index */
	for (i = 0; i < ruler->slave_num; i++) {
		slave = &ruler->slave_dev[i];
		if (slave->state >= 0 &&
		    slave->state < thermal_state) {
			thermal_state = slave->state;
			thermal_slave = i;
		}
	}

	/* get the freq range index determined by master freq */
	state = drg_freq2state(ruler, master->divider, new_freq);

	/*
	 * if master's freq range index is higher than slave's thermal
	 * clamp freq range index, it means slave cannot sync freq range
	 * with master, if they have strict constraints, master's freq must
	 * be limited to ensure it will not break the rule.
	 */
	if (state > thermal_state) {
		/*
		 * find a proper freq for master according to
		 * thermal clamp freq range index
		 */
		ret = (master->divider[thermal_state] - 1) * KHZ;
		if (find_proper_upbound(master->freq_dev, &ret))
			ret /= KHZ;
		else
			ret = new_freq;

		trace_drg_thermal_limit(ruler->name, thermal_slave,
					thermal_state, state, new_freq, ret);
	}

	return ret;
}

static int drg_rule_get_master_state(struct drg_rule *ruler)
{
	int i, state = INVALID_STATE;

	if (!ruler->enable)
		return state;

	for (i = 0; i < ruler->master_num; i++) {
		if (ruler->master_dev[i].state < 0)
			continue;

		if (state < 0 ||
		    (ruler->master_vote_max &&
		     ruler->master_dev[i].state > state) ||
		    (!ruler->master_vote_max &&
		     ruler->master_dev[i].state < state))
			state = ruler->master_dev[i].state;
	}

	return state;
}

static void drg_get_slave_freq_range(struct drg_freq_device *freq_dev,
				     unsigned long *min_freq,
				     unsigned long *max_freq)
{
	struct drg_rule *ruler;
	struct drg_device *slave;
	unsigned long rule_min, rule_max;
	int state;

	*min_freq = 0;
	*max_freq = ULONG_MAX;

	list_for_each_entry(slave, &freq_dev->slave_list, node) {
		ruler = slave->ruler;
		state = drg_rule_get_master_state(ruler);

		if (!ruler->dn_limit || state <= 0)
			rule_min = 0;
		else
			rule_min = slave->divider[state - 1] * KHZ;

		if (!ruler->up_limit ||
		    state == ruler->divider_num ||
		    state < 0)
			rule_max = ULONG_MAX;
		else
			rule_max = (slave->divider[state] - 1) * KHZ;

		find_proper_upbound(freq_dev, &rule_max);

		slave->min_freq = rule_min;
		slave->max_freq = rule_max;
		trace_drg_update_freq_range(ruler->name, 0, state, rule_min, rule_max);

		/*
		 * the priority of the rule is determined by
		 * the order defined in dts, so the followed freq
		 * range cannot be large than pervious
		 */
		*max_freq = clamp(slave->max_freq, *min_freq, *max_freq);/*lint !e666 !e1058*/
		*min_freq = clamp(slave->min_freq, *min_freq, *max_freq);/*lint !e666 !e1058*/
	}
}

static void drg_rule_update_work(struct kthread_work *work)
{
	struct drg_rule *ruler = container_of(work,
					      struct drg_rule, exec_work);
	struct drg_device *slave;
	int i, cpu, state;

	state = drg_rule_get_master_state(ruler);

	ruler->state_idx = state;

	cpu_maps_update_begin();
	for (i = 0; i < ruler->slave_num; i++) {
		slave = &ruler->slave_dev[i];
		if (!slave->freq_dev)
			continue;

		cpu = cpumask_any_and(&slave->freq_dev->cpus, cpu_online_mask);
		if (cpu < nr_cpu_ids) {
			cpufreq_update_policy(cpu);
			continue;
		}

		if (slave->freq_dev->df) {
			mutex_lock(&slave->freq_dev->df->lock);
			update_devfreq(slave->freq_dev->df);
			mutex_unlock(&slave->freq_dev->df->lock);
		}
	}
	cpu_maps_update_done();
}

static void drg_update_state(struct drg_rule *ruler)
{
	/*
	 * drg_rule_update_work will update other cpufreq/devfreq's freq,
	 * throw it to a global worker can avoid function reentrant and
	 * other potential race risk
	 */
	kthread_queue_work(&g_drg_worker, &ruler->exec_work);
}

static void drg_refresh_all_rule(void)
{
	struct drg_rule *ruler;

	if (!g_drg_initialized)
		return;

	list_for_each_entry(ruler, &g_drg_rule_list, node)
		drg_update_state(ruler);
}

static unsigned long drg_freq_check_limit(struct drg_freq_device *freq_dev,
				  unsigned long target_freq)
{
	struct drg_device *master;
	unsigned long ret, min_freq = 0, max_freq = ULONG_MAX;
	int old_state, cpu;

	if (!g_drg_initialized) {
		freq_dev->cur_freq = target_freq * freq_dev->amp / KHZ;
		return target_freq;
	}

	/*
	 * first check if this device is slave of some rules, its freq
	 * must fit into the freq range decided by master
	 */
	drg_get_slave_freq_range(freq_dev, &min_freq, &max_freq);

	ret = clamp(target_freq * freq_dev->amp, min_freq, max_freq) / KHZ;/*lint !e666*/
	cpu = freq_dev->df ? 0 : cpumask_first(&freq_dev->cpus);
	trace_drg_check_limit(freq_dev->np->name, cpu, target_freq,
			      min_freq, max_freq);

	/*
	 * second check if this device is master of some rules, its freq
	 * will not break the rule due to slave's thermal clamp
	 */
	list_for_each_entry(master, &freq_dev->master_list, node)
		ret = drg_check_thermal_limit(master, ret);

	if (cpumask_subset(&freq_dev->cpus, &idle_cpumask))
		goto end;

	/* get current freq range index and notice all related slaves */
	list_for_each_entry(master, &freq_dev->master_list, node) {
		old_state = master->state;
		master->state = drg_freq2state(master->ruler,
					       master->divider, ret);

		if (master->state != old_state)
			drg_update_state(master->ruler);
	}
end:
	freq_dev->cur_freq = ret;

	return ret * KHZ / freq_dev->amp;
}

/*
 * drg_cpufreq_check_limit() - Return freq clamped by drg
 * @policy: the cpufreq policy of the freq domain
 * @target_freq: the target freq determined by cpufreq governor
 *
 * check this policy's freq device, if it is slave of any drg rule, make sure
 * target_freq will never exceed drg freq range, if it is master of any drg
 * rule, notify other slave device of this rule if this master has changed
 * freq range.
 *
 * Return: frequency.
 *
 * Locking: This function must be called under policy->rwsem. cpufreq core
 * ensure this function will not be called when unregistering the freq device
 * of this policy.
 */
unsigned int drg_cpufreq_check_limit(struct cpufreq_policy *policy,
				     unsigned int target_freq)
{
	struct drg_freq_device *freq_dev;

	if (!policy)
		return target_freq;

	freq_dev = drg_get_cpu_dev(policy->cpu);
	if (!freq_dev)
		return target_freq;

	return drg_freq_check_limit(freq_dev, target_freq);
}

/*
 * drg_devfreq_check_limit() - Return freq clamped by drg
 * @df: the devfreq device
 * @target_freq: the target freq determined by devfreq governor
 *
 * check this devfreq's freq device, if it is slave of any drg rule, make sure
 * target_freq will never exceed drg freq range, if it is master of any drg
 * rule, notify other slave device of this rule if this master has changed
 * freq range.
 *
 * Return: frequency.
 *
 * Locking: This function must be called under df->lock. This lock will
 * protect against unregistering the freq device of this devfreq.
 */
unsigned long drg_devfreq_check_limit(struct devfreq *df,
				      unsigned long target_freq)
{
	struct drg_freq_device *freq_dev;

	if (!df)
		return target_freq;

	freq_dev = drg_get_devfreq_dev(df);
	if (!freq_dev)
		return target_freq;

	return drg_freq_check_limit(freq_dev, target_freq);
}

static void drg_update_clip_state(struct drg_freq_device *freq_dev,
				  unsigned long target_freq)
{
	struct drg_device *slave;

	if (!g_drg_initialized)
		return;

	list_for_each_entry(slave, &freq_dev->slave_list, node)
		slave->state = drg_freq2state(slave->ruler,
					      slave->divider,
					      target_freq);
}

/*
 * drg_cpufreq_cooling_update()
 * @cpu: cpu number
 * @clip_freq: the clip freq determined by cpu cooling device
 *
 * convert clip freq to freq range index for each related slave of any drg
 * rule.
 *
 * Locking: cpufreq dirver should ensure cooling device will be unregistered
 * before drg freq device being unregistered.
 */
void drg_cpufreq_cooling_update(unsigned int cpu, unsigned int clip_freq)
{
	struct drg_freq_device *freq_dev;

	/*
	 * the cooling device we used is not the one registered
	 * in cpufreq-dt, so this interface may be called when
	 * we hotplug cpu, in order to prevent freq_dev being
	 * accessed when cpufreq unregister this device in hotplug,
	 * hold hotplug lock in this interface
	 */
	cpu_maps_update_begin();
	freq_dev = drg_get_cpu_dev(cpu);
	if (freq_dev)
		drg_update_clip_state(freq_dev, clip_freq);

	cpu_maps_update_done();
}

/*
 * drg_devfreq_cooling_update()
 * @df: devfreq device
 * @clip_freq: the clip freq determined by cpu cooling device
 *
 * convert clip freq to freq range index for each related slave of any drg
 * rule.
 *
 * Locking: devfreq dirver should ensure cooling device will be unregistered
 * before drg freq device being unregistered.
 */
void drg_devfreq_cooling_update(struct devfreq *df, unsigned long clip_freq)
{
	struct drg_freq_device *freq_dev;

	freq_dev = drg_get_devfreq_dev(df);
	if (freq_dev)
		drg_update_clip_state(freq_dev, clip_freq / KHZ);
}

/*
 * drg_get_cpu_margin() - Return drg margin of the cpu
 * @cpu: cpu number
 *
 * return the drg margin of the cpu, if it is not set before, return max
 * drg margin.
 */
unsigned int drg_get_cpu_margin(unsigned int cpu)
{
	if (per_cpu(drg_margin, cpu))
		return per_cpu(drg_margin, cpu);

	return MAX_DRG_MARGIN;
}

static int drg_cpufreq_policy_notifier(struct notifier_block *nb,
				       unsigned long event, void *data)
{
	struct cpufreq_policy *policy = (struct cpufreq_policy *) data;
	struct drg_freq_device *freq_dev;
	unsigned int margin;
	int cpu;
	unsigned long max_freq, min_freq = 0;

	if (event != CPUFREQ_ADJUST || !g_drg_initialized)
		return NOTIFY_DONE;

	freq_dev = drg_get_cpu_dev(policy->cpu);
	if (!freq_dev)
		return NOTIFY_DONE;

	max_freq = ((unsigned long)policy->cpuinfo.max_freq) * freq_dev->amp;
	drg_get_slave_freq_range(freq_dev, &min_freq, &max_freq);

	max_freq /= freq_dev->amp;
	min_freq /= freq_dev->amp;

	if (max_freq == 0)
		return NOTIFY_DONE;

	margin = (((unsigned long)policy->cpuinfo.max_freq) * MAX_DRG_MARGIN) /
		 max_freq;

	for_each_cpu(cpu, &freq_dev->cpus)
		per_cpu(drg_margin, cpu) = margin;

	cpufreq_verify_within_limits(policy, min_freq, max_freq);

	trace_drg_cpu_policy_adjust(policy->cpu, margin, min_freq, max_freq);

	return NOTIFY_OK;
}

static struct notifier_block drg_cpufreq_policy_nb = {
	.notifier_call = drg_cpufreq_policy_notifier,
};

static int drg_cpu_pm_notifier(struct notifier_block *nb, unsigned long action,
				void *data)
{
	struct drg_freq_device *freq_dev;
	struct drg_device *master;
	int cpu = smp_processor_id();
	bool dev_found = false;
	bool trigger_cpufreq = false;

	if (!g_drg_initialized)
		return NOTIFY_OK;

	read_lock(&drg_list_lock);
	list_for_each_entry(freq_dev, &g_drg_cpufreq_list, node) {
		if (cpumask_test_cpu(cpu, &freq_dev->cpus)) {
			dev_found = true;
			break;
		}
	}

	if (!dev_found)
		goto unlock;

	switch (action) {
	case CPU_PM_ENTER:
		if (!hisi_cluster_cpu_all_pwrdn())
			break;

		cpumask_or(&idle_cpumask, &idle_cpumask, &freq_dev->cpus);

		list_for_each_entry(master, &freq_dev->master_list, node) {
			master->state = 0;
		}
		break;
	case CPU_PM_ENTER_FAILED:
	case CPU_PM_EXIT:
		if (!cpumask_subset(&freq_dev->cpus, &idle_cpumask))
			break;

		list_for_each_entry(master, &freq_dev->master_list, node) {
			trigger_cpufreq = true;
			master->state = drg_freq2state(master->ruler,
						       master->divider,
						       freq_dev->cur_freq);
		}

		cpumask_andnot(&idle_cpumask, &idle_cpumask, &freq_dev->cpus);
		break;
	}

unlock:
	read_unlock(&drg_list_lock);

	if (trigger_cpufreq)
		drg_refresh_all_rule();

	return NOTIFY_OK;
}

static struct notifier_block drg_cpu_pm_nb = {
	.notifier_call = drg_cpu_pm_notifier,
};


static void drg_freq_dev_release_work(struct kthread_work *work)
{
	struct drg_device *drg_dev, *tmp;
	struct drg_freq_device *freq_dev =
			container_of(work,
				     struct drg_freq_device, destory_work);

	list_for_each_entry_safe(drg_dev, tmp, &freq_dev->slave_list, node) {
		drg_dev->freq_dev = NULL;
		drg_dev->state = INVALID_STATE;
		list_del(&drg_dev->node);
	}

	list_for_each_entry_safe(drg_dev, tmp, &freq_dev->master_list, node) {
		drg_dev->freq_dev = NULL;
		drg_dev->state = INVALID_STATE;
		list_del(&drg_dev->node);
		drg_update_state(drg_dev->ruler);
	}
}

static void _of_match_drg_rule(struct drg_freq_device *freq_dev,
			       struct drg_rule *ruler)
{
	struct drg_device *tmp_dev;
	int i;

	for (i = 0; i < ruler->master_num; i++) {
		if (ruler->master_dev[i].np == freq_dev->np) {
			tmp_dev = &ruler->master_dev[i];
			list_add_tail(&tmp_dev->node,
				 &freq_dev->master_list);
			tmp_dev->freq_dev = freq_dev;
			tmp_dev->state = drg_freq2state(ruler,
							tmp_dev->divider,
							freq_dev->cur_freq);
			pr_debug("%s:match master%d %s\n",
				 ruler->name, i, freq_dev->np->full_name);
			return;
		}
	}

	for (i = 0; i < ruler->slave_num; i++) {
		if (ruler->slave_dev[i].np == freq_dev->np) {
			list_add_tail(&ruler->slave_dev[i].node,
				 &freq_dev->slave_list);
			ruler->slave_dev[i].freq_dev = freq_dev;
			pr_debug("%s:match slave%d %s\n",
				 ruler->name, i, freq_dev->np->full_name);
			return;
		}
	}
}

static void of_match_drg_rule(struct drg_freq_device *freq_dev)
{
	struct drg_rule *ruler;

	list_for_each_entry(ruler, &g_drg_rule_list, node)
		_of_match_drg_rule(freq_dev, ruler);
}

static void of_match_drg_freq_device(struct drg_rule *ruler)
{
	struct drg_freq_device *freq_dev;

	list_for_each_entry(freq_dev, &g_drg_cpufreq_list, node)
		_of_match_drg_rule(freq_dev, ruler);

	list_for_each_entry(freq_dev, &g_drg_devfreq_list, node)
		_of_match_drg_rule(freq_dev, ruler);
}

static void drg_destory_freq_dev(struct drg_freq_device *freq_dev)
{
	if (g_drg_initialized) {
		kthread_init_work(&freq_dev->destory_work,
				  drg_freq_dev_release_work);
		kthread_queue_work(&g_drg_worker, &freq_dev->destory_work);
		kthread_flush_work(&freq_dev->destory_work);
	}

	kfree(freq_dev);
}

/*
 * drg_cpufreq_register()
 * @policy: the cpufreq policy of the freq domain
 *
 * register drg freq device for the cpu policy, match it with related drg
 * device of all rules. drg_refresh_all_rule will be called to make sure no
 * violation of rule will happen after adding this device.
 *
 * Locking: this function will hold drg_list_lock during registration, cpu
 * cooling device should be registered after drg freq device registration.
 */
/*lint --e{593}*/
void drg_cpufreq_register(struct cpufreq_policy *policy)
{
	struct drg_freq_device *freq_dev;
	struct device *cpu_dev;

	if (!policy) {
		pr_err("%s:null cpu policy\n", __func__);
		return;
	}

	cpu_dev = get_cpu_device(cpumask_first(policy->related_cpus));
	if (!cpu_dev) {
		pr_err("%s:null cpu device\n", __func__);
		return;
	}

	write_lock(&drg_list_lock);
	list_for_each_entry(freq_dev, &g_drg_cpufreq_list, node) {
		if (cpumask_subset(policy->related_cpus, &freq_dev->cpus) ||
		    freq_dev->np == cpu_dev->of_node)
			goto unlock;
	}

	freq_dev = (struct drg_freq_device *)
			kzalloc(sizeof(*freq_dev), GFP_ATOMIC);
	if (!freq_dev) {
		pr_err("alloc cpu%d freq dev err\n", policy->cpu);
		goto unlock;
	}

	cpumask_copy(&freq_dev->cpus, policy->related_cpus);
	freq_dev->np = cpu_dev->of_node;
	freq_dev->amp = CPUFREQ_AMP;
	freq_dev->cur_freq = policy->cur * CPUFREQ_AMP / KHZ;
	INIT_LIST_HEAD(&freq_dev->master_list);
	INIT_LIST_HEAD(&freq_dev->slave_list);

	list_add_tail(&freq_dev->node, &g_drg_cpufreq_list);
	if (g_drg_initialized)
		of_match_drg_rule(freq_dev);
unlock:
	write_unlock(&drg_list_lock);
	drg_refresh_all_rule();
}

/*
 * drg_cpufreq_unregister()
 * @policy: the cpufreq policy of the freq domain
 *
 * unregister drg freq device for the cpu policy, release the connection with
 * related drg device of all rules, and if this device is master of the rule,
 * update this rule.
 *
 * Locking: this function will hold drg_list_lock during unregistration, cpu
 * cooling device should be unregistered before drg freq device unregistration.
 * the caller may hold policy->lock, because there may be other work in worker
 * thread wait for policy->lock, so we can't put destory_work to work thread
 * and wait for completion of the work, and drg_rule_update_work need hold
 * hotplug lock to protect against drg_cpufreq_unregister during hotplug.
 */
void drg_cpufreq_unregister(struct cpufreq_policy *policy)
{
	struct drg_freq_device *freq_dev;
	bool found = false;

	if (!policy) {
		pr_err("%s:null cpu policy\n", __func__);
		return;
	}

	write_lock(&drg_list_lock);
	list_for_each_entry(freq_dev, &g_drg_cpufreq_list, node) {
		if (cpumask_test_cpu(policy->cpu, &freq_dev->cpus)) {
			found = true;
			list_del(&freq_dev->node);
			break;
		}
	}
	write_unlock(&drg_list_lock);

	if (found) {
		drg_freq_dev_release_work(&freq_dev->destory_work);
		kfree(freq_dev);
	}
}

/*
 * drg_devfreq_register()
 * @df: devfreq device
 *
 * register drg freq device for the devfreq device, match it with related drg
 * device of all rules. drg_refresh_all_rule will be called to make sure no
 * violation of rule will happen after adding this device.
 *
 * Locking: this function will hold drg_list_lock during registration, devfreq
 * cooling device should be registered after drg freq device registration.
 */
/*lint --e{593}*/
void drg_devfreq_register(struct devfreq *df)
{
	struct drg_freq_device *freq_dev;
	struct device *dev;

	if (!df) {
		pr_err("%s:null devfreq\n", __func__);
		return;
	}

	dev = df->dev.parent;
	if (!dev) {
		pr_err("%s:null devfreq parent dev\n", __func__);
		return;
	}

	write_lock(&drg_list_lock);
	list_for_each_entry(freq_dev, &g_drg_devfreq_list, node) {
		if (freq_dev->df == df ||
		    freq_dev->np == dev->of_node)
			goto unlock;
	}

	freq_dev = (struct drg_freq_device *)
			kzalloc(sizeof(*freq_dev), GFP_ATOMIC);
	if (!freq_dev) {
		pr_err("alloc freq dev err\n");
		goto unlock;
	}

	freq_dev->df = df;
	freq_dev->np = dev->of_node;
	freq_dev->amp = DEVFREQ_AMP;
	freq_dev->cur_freq = df->previous_freq * DEVFREQ_AMP / KHZ;
	INIT_LIST_HEAD(&freq_dev->master_list);
	INIT_LIST_HEAD(&freq_dev->slave_list);

	list_add_tail(&freq_dev->node, &g_drg_devfreq_list);
	if (g_drg_initialized)
		of_match_drg_rule(freq_dev);
unlock:
	write_unlock(&drg_list_lock);
	drg_refresh_all_rule();
}

/*
 * drg_devfreq_unregister()
 * @df: devfreq device
 *
 * unregister drg freq device for devfreq device, release the connection with
 * related drg device of all rules, and if this device is master of the rule,
 * update this rule.
 *
 * Locking: this function will hold drg_list_lock during unregistration,
 * cooling device should be unregistered before drg freq device unregistration.
 * we will put destory_work to worker thread and wait for completion of the work
 * to protect against drg_rule_update_work.
 */
void drg_devfreq_unregister(struct devfreq *df)
{
	struct drg_freq_device *freq_dev;
	bool found = false;

	if (!df) {
		pr_err("%s:null devfreq\n", __func__);
		return;
	}

	/*
	 * prevent devfreq scaling freq when unregister freq device,
	 * cpufreq itself can prevent this from happening
	 */
	mutex_lock(&df->lock);
	write_lock(&drg_list_lock);

	list_for_each_entry(freq_dev, &g_drg_devfreq_list, node) {
		if (freq_dev->df == df) {
			found = true;
			list_del(&freq_dev->node);
			break;
		}
	}

	write_unlock(&drg_list_lock);
	mutex_unlock(&df->lock);

	if (found)
		drg_destory_freq_dev(freq_dev);
}

int drg_get_freq_range(struct drg_dev_freq *freq)
{
	unsigned long divider, min_freq = 0, max_freq = ULONG_MAX;
	struct drg_freq_device *freq_dev;
	struct drg_device *slave;
	int idx, cpu, cluster, ret = DRG_NONE_DEV;

	if (freq == NULL)
		return ret;

	read_lock(&drg_list_lock);

	if (freq->type & DEV_TYPE_CPU_CLUSTER) {
		cluster = freq->type & 0xff;
		list_for_each_entry(freq_dev, &g_drg_cpufreq_list, node) {
			cpu = cpumask_first(&freq_dev->cpus);
			if (cpu < nr_cpu_ids &&
			    topology_physical_package_id(cpu) == cluster) {
				ret = freq->type;
				break;
			}
		}

	} else {
		list_for_each_entry(freq_dev, &g_drg_devfreq_list, node) {
			if (devtype_name_compare(freq->type,
						  freq_dev->np->name) == true) {
				ret = freq->type;
				break;
			}
		}
	}

	if (ret != DRG_NONE_DEV) {
		list_for_each_entry(slave,
				    &freq_dev->slave_list, node) {
			if (slave->ruler->up_limit &&
			    slave->ruler->dn_limit) {
				idx = slave->ruler->divider_num - 1;
				divider = slave->divider[idx] * KHZ;
				max_freq = clamp(divider, min_freq, max_freq);/*lint !e666*/
			}
		}

		list_for_each_entry(slave,
				    &freq_dev->master_list, node) {
			if (slave->ruler->up_limit &&
			    slave->ruler->dn_limit) {
				idx = slave->ruler->divider_num - 1;
				divider = slave->divider[idx] * KHZ;
				max_freq = clamp(divider, min_freq, max_freq);/*lint !e666*/
			}
		}

		freq->max_freq = max_freq;
		freq->min_freq = min_freq;
	}

	read_unlock(&drg_list_lock);

	return ret;
}


struct drg_attr {
	struct attribute attr;
	ssize_t (*show)(const struct drg_rule *, char *);
	ssize_t (*store)(struct drg_rule *, const char *, size_t count);
};

#define drg_attr_ro(_name)		\
static struct drg_attr _name =	\
__ATTR(_name, 0440, show_##_name, NULL)

#define drg_attr_rw(_name)			\
static struct drg_attr _name =		\
__ATTR(_name, 0640, show_##_name, store_##_name)


static struct attribute *default_attrs[] = {
	NULL
};

#define to_drg_rule(k) container_of(k, struct drg_rule, kobj)
#define to_attr(a) container_of(a, struct drg_attr, attr)
static ssize_t show(struct kobject *kobj, struct attribute *attr, char *buf)
{
	struct drg_rule *data = to_drg_rule(kobj);
	struct drg_attr *cattr = to_attr(attr);
	ssize_t ret = -EIO;

	if (cattr->show)
		ret = cattr->show(data, buf);

	return ret;
}

static ssize_t store(struct kobject *kobj, struct attribute *attr,
		     const char *buf, size_t count)
{
	struct drg_rule *data = to_drg_rule(kobj);
	struct drg_attr *cattr = to_attr(attr);
	ssize_t ret = -EIO;

	if (cattr->store)
		ret = cattr->store(data, buf, count);

	return ret;
}

static const struct sysfs_ops sysfs_ops = {
	.show	= show,
	.store	= store,
};

static struct kobj_type ktype_drg = {
	.sysfs_ops	= &sysfs_ops,
	.default_attrs	= default_attrs,
};

static int drg_device_init(struct device *dev, struct drg_device **drg_dev,
			   int *dev_num, struct drg_rule *ruler,
			   struct device_node *np, const char *prop)
{
	struct of_phandle_args drg_dev_spec;
	struct drg_device *t_drg_dev;
	int divider_num = ruler->divider_num;
	int i;

	i = of_property_count_u32_elems(np, prop);
	if (i <= 0 || (i % (divider_num + 1))) {
		dev_err(dev, "count %s err:%d\n", prop, i);
		return -ENOENT;
	}

	*dev_num = i / (divider_num + 1);

	*drg_dev = (struct drg_device *)
			devm_kzalloc(dev, sizeof(**drg_dev) * (*dev_num),
				     GFP_KERNEL);
	if (IS_ERR_OR_NULL(*drg_dev)) {
		dev_err(dev, "alloc %s fail\n", prop);
		return -ENOMEM;
	}

	for (i = 0; i < *dev_num; i++) {
		t_drg_dev = &(*drg_dev)[i];
		t_drg_dev->divider = (unsigned int *)
					devm_kzalloc(dev,
						sizeof(*t_drg_dev->divider) *
						divider_num, GFP_KERNEL);
		if (IS_ERR_OR_NULL(t_drg_dev->divider)) {
			dev_err(dev, "alloc %s%d fail\n", prop, i);
			return -ENOMEM;
		}

		if (of_parse_phandle_with_fixed_args(np, prop, divider_num,
						     i, &drg_dev_spec)) {
			dev_err(dev, "parse %s%d err\n", prop, i);
			return -ENOENT;
		}

		t_drg_dev->np = drg_dev_spec.np;
		t_drg_dev->ruler = ruler;
		t_drg_dev->state = INVALID_STATE;
		t_drg_dev->max_freq = ULONG_MAX;
		memcpy(t_drg_dev->divider, drg_dev_spec.args, sizeof(*t_drg_dev->divider) * divider_num);/* unsafe_function_ignore: memcpy */
	}

	return 0;
}

/*lint --e{593}*/
static int drg_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = pdev->dev.of_node;
	struct device_node *child;
	struct drg_rule *ruler;
	int ret;

	for_each_child_of_node(np, child) {
		ruler = (struct drg_rule *)
				devm_kzalloc(&pdev->dev, sizeof(*ruler),
					     GFP_KERNEL);
		if (!ruler) {
			dev_err(dev, "alloc ruler fail\n");
			ret = -ENOMEM;
			goto err_probe;
		}

		if (of_property_read_u32(child, "hisi,divider-cells",
					 &ruler->divider_num)) {
			dev_err(dev, "read divider cells fail\n");
			ret = -ENOENT;
			goto err_probe;
		}

		ruler->up_limit =
			of_property_read_bool(child, "up_limit_enable");
		ruler->dn_limit =
			of_property_read_bool(child, "down_limit_enable");
		ruler->thermal_enable =
			of_property_read_bool(child, "thermal_enable");
		ruler->master_vote_max =
			of_property_read_bool(child, "master_vote_max");

		ret = drg_device_init(&pdev->dev, &ruler->master_dev,
				      &ruler->master_num, ruler,
				      child, "hisi,drg-master");
		if (ret)
			goto err_probe;

		ret = drg_device_init(&pdev->dev, &ruler->slave_dev,
				      &ruler->slave_num, ruler,
				      child, "hisi,drg-slave");
		if (ret)
			goto err_probe;

		ruler->name = child->name;
		ruler->enable = true;

		ret = kobject_init_and_add(&ruler->kobj, &ktype_drg,
					   &dev->kobj, ruler->name);
		if (ret) {
			dev_err(dev, "create kobj err %d\n", ret);
			goto err_probe;
		}

		kthread_init_work(&ruler->exec_work, drg_rule_update_work);

		list_add_tail(&ruler->node, &g_drg_rule_list);
	}

	ret = cpu_pm_register_notifier(&drg_cpu_pm_nb);
	if (ret) {
		dev_err(dev, "register cpu pm notifier fail\n");
		goto err_probe;
	}

	ret = cpufreq_register_notifier(&drg_cpufreq_policy_nb,
					CPUFREQ_POLICY_NOTIFIER);
	if (ret) {
		dev_err(dev, "register cpufreq notifier fail\n");
		goto err_probe;
	}

	g_drg_work_thread = kthread_run(kthread_worker_fn,
					(void *) &g_drg_worker, "hisi-drg");
	if (IS_ERR(g_drg_work_thread)) {
		dev_err(dev, "create drg thread fail\n");
		ret = PTR_ERR(g_drg_work_thread);
		goto err_probe;
	}

	write_lock(&drg_list_lock);

	list_for_each_entry(ruler, &g_drg_rule_list, node)
		of_match_drg_freq_device(ruler);

	g_drg_initialized = true;

	write_unlock(&drg_list_lock);

	drg_refresh_all_rule();

	return 0;

err_probe:
	list_for_each_entry(ruler, &g_drg_rule_list, node)
		kobject_del(&ruler->kobj);

	INIT_LIST_HEAD(&g_drg_rule_list);

	return ret;
}

static const struct of_device_id drg_of_match[] = {
	{
		.compatible = "hisilicon,hisi-drg",
	},
	{ /* end */ }
};

static struct platform_driver drg_driver = {
	.probe = drg_probe,
	.driver = {
		.name = "hisi-drg",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(drg_of_match),
	},
};

module_platform_driver(drg_driver);
