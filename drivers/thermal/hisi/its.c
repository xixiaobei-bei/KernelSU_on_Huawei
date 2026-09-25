#include <linux/debugfs.h>
#include <linux/of.h>
#include <linux/cpufreq.h>
#include <linux/cpumask.h>
#include <linux/module.h>
#include <linux/topology.h>
#include <soc_its_para.h>
#include <securec.h>

#ifdef CONFIG_HISI_ITS_DEBUG
#include <linux/ktime.h>
#include <linux/thermal.h>
#endif

#define ITS_SVC_REG_RD 0xc5009901UL
#define DEFAULT_FREQ_IDX 0xFF
#define MODE_LEN 16

extern int memset_s(void *dest, size_t destMax, int c, size_t count);
extern struct cpufreq_frequency_table *cpufreq_frequency_get_table(unsigned int cpu);

static DEFINE_MUTEX(its_lock);

struct its_power {
	bool initialized;
	int enabled;
	int polling_timeout;
	unsigned long long power[CORE_NUMBER];
	struct delayed_work work;
	struct class *its_class;
	struct device *its_device;
};

enum its_ops {
	GET_DYNAMIC_POWER_BY_CORE = 0,
	GET_DYNAMIC_POWER_BY_CLUSTER,
	SET_FREQ_CHANGE,
	GET_STATIC_POWER_BY_CORE,
	GET_TOTAL_POWER_BY_CORE,
	FUNCTION_MAX
};

static struct its_power *its_power_data;

void set_its_mode(int enable)
{
	if (its_power_data->initialized != true)
		return;

	mutex_lock(&its_lock);
	pr_err("set_its_mode set mode %d\n", enable);
	its_power_data->enabled = enable;
	if (enable != 0) {
		queue_delayed_work(system_freezable_power_efficient_wq, &its_power_data->work,
				msecs_to_jiffies(its_power_data->polling_timeout));
	} else {
		cancel_delayed_work(&its_power_data->work);
	}
	mutex_unlock(&its_lock);
}
EXPORT_SYMBOL(set_its_mode);

noinline unsigned long atfd_its_smc(u64 _function_id, u64 _arg0, u64 _arg1, u64 _arg2)
{
	register u64 function_id asm("x0") = _function_id;
	register u64 arg0 asm("x1") = _arg0;
	register u64 arg1 asm("x2") = _arg1;
	register u64 arg2 asm("x3") = _arg2;
	asm volatile (
			__asmeq("%0", "x0")
			__asmeq("%1", "x1")
			__asmeq("%2", "x2")
			__asmeq("%3", "x3")
			"smc	#0\n"
			: "+r" (function_id)
			: "r" (arg0), "r" (arg1), "r" (arg2));
	return (unsigned long)function_id;
}

noinline unsigned long atfd_its_ret3_smc(u64 _function_id, u64 *_arg0, u64 *_arg1, u64 *_arg2)
{
	register u64 function_id asm("x0") = _function_id;
	register u64 arg0 asm("x1") = *_arg0;
	register u64 arg1 asm("x2") = *_arg1;
	register u64 arg2 asm("x3") = *_arg2;
	asm volatile (
			__asmeq("%0", "x0")
			__asmeq("%1", "x1")
			__asmeq("%2", "x2")
			__asmeq("%3", "x3")
			"smc	#0\n"
			: "+r" (function_id)
			: "r" (arg0), "r" (arg1), "r" (arg2));
	*_arg0 = arg0;
	*_arg1 = arg1;
	*_arg2 = arg2;
	return (unsigned long)function_id;
}

u64 freq_to_index(u64 core_idx, u64 freq)
{
	struct cpufreq_frequency_table *freq_table = NULL, *pos = NULL;
	u64 freq_idx = DEFAULT_FREQ_IDX;
	u64 idx = 0;

	freq_table = cpufreq_frequency_get_table(core_idx);
	if (!freq_table || freq == 0) {
		return freq_idx;
	}

	cpufreq_for_each_valid_entry(pos, freq_table) {
		if (freq == pos->frequency) {
			freq_idx = idx;
			break;
		}
		idx++;
	}

	if (freq_idx == DEFAULT_FREQ_IDX) {
		pr_err("%s: core(%llu), table can't find freq %llu\n", __func__, core_idx, freq);
		return freq_idx;
	}

	return freq_idx;
}

u64 get_cluster_core_idx(u64 cluster_idx)
{
	u64 core_idx = 0;
	unsigned int i;

	for (i = 0; i < CORE_NUMBER; i++) {
		if (cluster_idx == its_core_para[i].cluster_idx) {
			core_idx = (u64)i;
			break;
		}
	}

	return core_idx;
}

unsigned long long hisi_sec_its_get_core_power(u64 core_idx, u64 freq)
{
	u64 freq_idx;

	if (its_power_data->initialized != true)
		return 0;

	freq_idx = freq_to_index(core_idx, freq);

	return atfd_its_smc(ITS_SVC_REG_RD, GET_DYNAMIC_POWER_BY_CORE,
				(freq_idx << ITS_NEW_FREQ_IDX_START) | core_idx, freq / 1000);
}
EXPORT_SYMBOL(hisi_sec_its_get_core_power);

unsigned long long hisi_sec_its_get_cluster_power(u64 cluster_idx, u64 freq)
{
	u64 freq_idx;
	u64 core_idx;

	if (its_power_data->initialized != true)
		return 0;

	core_idx = get_cluster_core_idx(cluster_idx);
	freq_idx = freq_to_index(core_idx, freq);

	return atfd_its_smc(ITS_SVC_REG_RD, GET_DYNAMIC_POWER_BY_CLUSTER,
				(freq_idx << ITS_NEW_FREQ_IDX_START) | cluster_idx, freq / 1000);
}
EXPORT_SYMBOL(hisi_sec_its_get_cluster_power);

unsigned long long hisi_sec_its_set_window_by_core(u64 core_idx, u64 old_freq, u64 freq)
{
	u64 freq_idx, old_freq_idx;

	if (its_power_data->initialized != true)
		return 0;

	old_freq_idx = freq_to_index(core_idx, old_freq);
	freq_idx = freq_to_index(core_idx, freq);

	if ((freq_idx == DEFAULT_FREQ_IDX) || (old_freq_idx == DEFAULT_FREQ_IDX))
		return 0;

	return atfd_its_smc(ITS_SVC_REG_RD, SET_FREQ_CHANGE,
			(old_freq_idx << ITS_OLD_FREQ_IDX_START) | (freq_idx << ITS_NEW_FREQ_IDX_START) | core_idx,
			((old_freq / 1000) << ITS_OLD_FREQ_START) | (freq / 1000));
}
EXPORT_SYMBOL(hisi_sec_its_set_window_by_core);

unsigned long long hisi_sec_its_get_static_power(u64 core_idx)
{
	if (its_power_data->initialized != true)
		return 0;

	return atfd_its_smc(ITS_SVC_REG_RD, GET_STATIC_POWER_BY_CORE, core_idx, 0);
}
EXPORT_SYMBOL(hisi_sec_its_get_static_power);

int hisi_sec_its_get_total_power(u64 core_idx, u64 freq, u64 *dynamic_power, u64 *static_power)
{
	int ret;
	u64 x1, x2, x3, freq_idx;

	if (its_power_data->initialized != true) {
		pr_err("%s: initialized not ready\n", __func__);
		return -1;
	}

	freq_idx = freq_to_index(core_idx, freq);

	x1 = GET_TOTAL_POWER_BY_CORE;
	x2 = (freq_idx << ITS_NEW_FREQ_IDX_START) | core_idx;
	x3 = freq / 1000;
	ret = atfd_its_ret3_smc(ITS_SVC_REG_RD, &x1, &x2, &x3);

	if (ret)
		return ret;

	*dynamic_power = x1;
	*static_power = x2;

	return 0;
}
EXPORT_SYMBOL(hisi_sec_its_get_total_power);

static int its_cpufreq_notifier(struct notifier_block *nb,
				    unsigned long event, void *data)
{
	struct cpufreq_freqs *freq = data;

	if (event != CPUFREQ_PRECHANGE)
		return NOTIFY_DONE;

	if (freq->new != freq->old)
		hisi_sec_its_set_window_by_core(freq->cpu, freq->old, freq->new);

	return NOTIFY_OK;
}
/* Notifier for cpufreq transition change */
static struct notifier_block its_cpufreq_notifier_block = {
		.notifier_call = its_cpufreq_notifier,
};

/* when dubai call IOCTL, set power result to zero */
int reset_power_result(void)
{
	int ret = 0;

	if (its_power_data->initialized != true)
		return ret;

	if (its_power_data != NULL) {
		ret = memset_s(&its_power_data->power, CORE_NUMBER * sizeof(unsigned long long),
						0, CORE_NUMBER * sizeof(unsigned long long));
		if (ret != 0) {
			pr_err("[%s] memset failed\n", __func__);
			return ret;
		}
	} else {
		return -EINVAL;
	}
	return ret;
}
EXPORT_SYMBOL(reset_power_result);

/* when dubai call IOCTL, get power result */
int get_its_power_result(its_cpu_power_t *result)
{
	int ret = 0;
	int i;

	if (its_power_data->initialized != true)
		return -EINVAL;

	for (i = 0; i < CORE_NUMBER; i++)
		result->power[i] = its_power_data->power[i];

	return ret;
}
EXPORT_SYMBOL(get_its_power_result);

static void its_dubai_getpower(struct work_struct *work)
{
	int idx, ret;
	unsigned long freq;
	u64 static_power, dynamic_power;
#ifdef CONFIG_HISI_ITS_DEBUG
	struct timespec ts;
	struct thermal_zone_device *temptz = NULL;
	int tmp, temp0 = 0, temp1 = 0, temp2 = 0;
	unsigned long long leakage[8], dynamic[8];
#endif

	if (its_power_data->enabled == 0)
		return;

	for (idx = 0; idx < nr_cpu_ids; idx++) {
		freq = cpufreq_quick_get(idx);
		/* total power = dynamic_power + static_power */
		if (freq > 0)
			ret = hisi_sec_its_get_total_power(idx, freq, &dynamic_power, &static_power);
		else
			ret = -1;

		if (ret) {
			its_power_data->power[idx] += 0;
#ifdef CONFIG_HISI_ITS_DEBUG
			pr_err("%s: core %d, its_get_total_power fail. ret = %d\n", __func__, idx, ret);
			leakage[idx] = 0;
			dynamic[idx] = 0;
#endif
		} else {
			if (ULONG_MAX - (dynamic_power + static_power) > its_power_data->power[idx])
				its_power_data->power[idx] += (dynamic_power + static_power);
			else
				its_power_data->power[idx] = (dynamic_power + static_power);
#ifdef CONFIG_HISI_ITS_DEBUG
			leakage[idx] = static_power;
			dynamic[idx] = dynamic_power;
#endif
		}
	}

#ifdef CONFIG_HISI_ITS_DEBUG
	temptz = thermal_zone_get_zone_by_name("cluster0");
	tmp = thermal_zone_get_temp(temptz, &temp0);
	temptz = thermal_zone_get_zone_by_name("cluster1");
	tmp = thermal_zone_get_temp(temptz, &temp1);
	temptz = thermal_zone_get_zone_by_name("cluster2");
	tmp = thermal_zone_get_temp(temptz, &temp2);
	getnstimeofday(&ts);
	pr_err("Time:%ld us UTC, temp:%d,%d,%d, power:%llu, %llu, %llu, %llu, %llu, %llu, %llu, %llu\n",
		(ts.tv_sec*1000000 + (ts.tv_nsec/1000)),
		temp0, temp1, temp2,
		its_power_data->power[0], its_power_data->power[1],
		its_power_data->power[2], its_power_data->power[3],
		its_power_data->power[4], its_power_data->power[5],
		its_power_data->power[6], its_power_data->power[7]);
	pr_err("detail power: %llu + %llu, %llu + %llu, %llu + %llu, %llu + %llu, "
			"%llu + %llu, %llu + %llu, %llu + %llu, %llu + %llu\n",
			dynamic[0], leakage[0], dynamic[1], leakage[1],
			dynamic[2], leakage[2], dynamic[3], leakage[3],
			dynamic[4], leakage[4], dynamic[5], leakage[5],
			dynamic[6], leakage[6], dynamic[7], leakage[7]);
#endif

	if (its_power_data->polling_timeout != 0)
		queue_delayed_work(system_freezable_power_efficient_wq, &its_power_data->work,
						msecs_to_jiffies(its_power_data->polling_timeout));
	else
		cancel_delayed_work(&its_power_data->work);
}

static ssize_t
its_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (dev == NULL || attr == NULL)
		return 0;

	return snprintf_s(buf, MODE_LEN,  MODE_LEN - 1, "%d\n", its_power_data->enabled);
}

static ssize_t
its_mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int value;

	if (dev == NULL || attr == NULL)
		return -EINVAL;

	if (kstrtoint(buf, 10, &value))
		return -EINVAL;

	set_its_mode(value);

	return (ssize_t)count;
}

static DEVICE_ATTR(its_mode, 0644, its_mode_show, its_mode_store);

#ifdef CONFIG_HISI_ITS_DEBUG
static ssize_t
its_polling_timeout_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (dev == NULL || attr == NULL)
		return 0;

	return snprintf_s(buf, MODE_LEN,  MODE_LEN - 1, "%d\n", its_power_data->polling_timeout);
}

static ssize_t
its_polling_timeout_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int value;

	if (dev == NULL || attr == NULL)
		return -EINVAL;

	if (kstrtoint(buf, 10, &value))
		return -EINVAL;

	mutex_lock(&its_lock);
	its_power_data->polling_timeout = value;

	if (its_power_data->polling_timeout != 0)
		queue_delayed_work(system_freezable_power_efficient_wq, &its_power_data->work,
						msecs_to_jiffies(its_power_data->polling_timeout));
	else
		cancel_delayed_work(&its_power_data->work);

	mutex_unlock(&its_lock);

	return (ssize_t)count;
}
static DEVICE_ATTR(polling_timeout, 0644, its_polling_timeout_show, its_polling_timeout_store);
#endif

static int hisi_its_init(void)
{
	int ret;
	struct device_node *np = NULL;

	its_power_data = kzalloc(sizeof(struct its_power), GFP_KERNEL);
	if (!its_power_data)
		return -ENOMEM;

	np = of_find_node_by_name(NULL, "its_config");
	if (np == NULL) {
		pr_err("its_config node not found\n");
		ret = -ENODEV;
		goto out_find_node;
	}

	ret = of_property_read_u32(np, "hisilicon,its_polling_timeout", (u32 *)&its_power_data->polling_timeout);
	if (ret) {
		pr_err("%s its_polling_timeout read err\n", __func__);
		goto node_put;
	}

	of_node_put(np);

	ret = cpufreq_register_notifier(&its_cpufreq_notifier_block,
			CPUFREQ_TRANSITION_NOTIFIER);

	if (ret) {
		pr_err("%s: register cpufreq notifier fail\n", __func__);
		goto register_err;
	}

	its_power_data->its_class = class_create(THIS_MODULE, "its");
	if (IS_ERR(its_power_data->its_class)) {
		pr_err("hisi its class create error\n");
		goto register_err;
	}

	its_power_data->its_device =
	    device_create(its_power_data->its_class, NULL, 0, NULL, "its");
	if (IS_ERR(its_power_data->its_device)) {
		pr_err("its device create error\n");
		ret = (int)PTR_ERR(its_power_data->its_device);
		goto class_destroy;
	}

	ret = device_create_file(its_power_data->its_device, &dev_attr_its_mode);
	if (ret) {
		pr_err("its mode create error\n");
		goto device_destroy;
	}

#ifdef CONFIG_HISI_ITS_DEBUG
	ret = device_create_file(its_power_data->its_device, &dev_attr_polling_timeout);
	if (ret) {
		pr_err("polling timeout create error\n");
		goto device_destroy;
	}
#endif

	its_power_data->initialized = true;
	its_power_data->enabled = 1;
	/* workqueue to calc power for dubai */
	if (its_power_data->polling_timeout != 0) {
		INIT_DELAYED_WORK(&its_power_data->work, its_dubai_getpower);
		queue_delayed_work(system_freezable_power_efficient_wq, &its_power_data->work,
						msecs_to_jiffies(its_power_data->polling_timeout));
	}

	return 0;

device_destroy:
	device_destroy(its_power_data->its_class, 0);
class_destroy:
	class_destroy(its_power_data->its_class);
	its_power_data->its_class = NULL;
register_err:
	cancel_delayed_work(&its_power_data->work);
node_put:
	of_node_put(np);
out_find_node:
	kfree(its_power_data);
	its_power_data = NULL;
	return ret;
}

static void hisi_its_exit(void)
{
	if (its_power_data) {
		cancel_delayed_work(&its_power_data->work);
		kfree(its_power_data);
		its_power_data = NULL;
	}
}

module_init(hisi_its_init);
module_exit(hisi_its_exit);
