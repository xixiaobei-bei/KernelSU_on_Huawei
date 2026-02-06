#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/time64.h>

#include <chipset_common/dubai/dubai_plat.h>

static int freq_num = 0;
static int freq_stats_num = 0;
static struct dubai_gpu_freq_info *freq_stats = NULL;
static DEFINE_MUTEX(stats_lock);
static atomic_t stats_enable = ATOMIC_INIT(0);

static void dubai_reset_gpu_freq_stats(void)
{
	if (freq_stats) {
		kfree(freq_stats);
		freq_stats = NULL;
	}
	freq_num = 0;
	freq_stats_num = 0;
	atomic_set(&stats_enable, 0);
}

static struct dubai_gpu_freq_info *dubai_gpu_lookup_add(int64_t freq)
{
	int i;

	for (i = 0; i < freq_num; i++) {
		if (freq == freq_stats[i].freq)
			return &freq_stats[i];
	}

	if (freq_stats_num < freq_num) {
		freq_stats[freq_stats_num].freq = freq;
		freq_stats_num++;
		return &freq_stats[freq_stats_num - 1];
	}
	dubai_err("Gpu freq num exceed: %d, abort freq: %ld", freq_num, freq);

	return NULL;
}

void dubai_set_gpu_freq_num(int num)
{
	if ((num > 0) && (freq_num == 0))
		freq_num = num;
}
EXPORT_SYMBOL(dubai_set_gpu_freq_num); //lint !e580

void dubai_update_gpu_info(int64_t freq, int64_t busy_time, int64_t total_time, int32_t cycle_ms)
{
	struct dubai_gpu_freq_info *info = NULL;
	int64_t run_time;

	if (!atomic_read(&stats_enable) || !freq || !total_time)
		return;

	if (!mutex_trylock(&stats_lock))
		return;

	if (unlikely(!freq_stats)) {
		freq_stats = kzalloc(freq_num * sizeof(struct dubai_gpu_freq_info), GFP_ATOMIC);
		if (!freq_stats) {
			dubai_err("Failed to alloc memory");
			return;
		}
	}

	info = dubai_gpu_lookup_add(freq);
	if (info) {
		info->freq = freq;
		run_time = busy_time * cycle_ms * USEC_PER_MSEC / total_time;
		info->run_time += run_time;
		info->idle_time += (cycle_ms * USEC_PER_MSEC - run_time);
	}
	mutex_unlock(&stats_lock);
}
EXPORT_SYMBOL(dubai_update_gpu_info); //lint !e580

static int dubai_set_gpu_enable(bool enable)
{
	if (enable) {
		if (freq_num <= 0) {
			dubai_err("Not support");
			return -EOPNOTSUPP;
		}
		atomic_set(&stats_enable, 1);
	} else {
		dubai_reset_gpu_freq_stats();
	}
	dubai_err("Gpu stats enable: %d", enable ? 1 : 0);

	return 0;
}

static int dubai_get_gpu_info(struct dubai_gpu_freq_info *data, int num)
{
	if (!atomic_read(&stats_enable))
		return -EPERM;

	if (!data || (num != freq_num)) {
		dubai_err("Invalid param: %d, %d", num, freq_num);
		return -EINVAL;
	}

	mutex_lock(&stats_lock);
	if (freq_stats)
		memcpy(data, freq_stats, freq_num * sizeof(struct dubai_gpu_freq_info));
	mutex_unlock(&stats_lock);

	return 0;
}

static int dubai_get_gpu_freq_num(void)
{
	return freq_num;
}

static struct dubai_gpu_stats_ops gpu_ops = {
	.enable = dubai_set_gpu_enable,
	.get_stats = dubai_get_gpu_info,
	.get_num = dubai_get_gpu_freq_num,
};

void dubai_gpu_freq_stats_init(void)
{
	dubai_register_module_ops(DUBAI_MODULE_GPU, &gpu_ops);
}

void dubai_gpu_freq_stats_exit(void)
{
	dubai_unregister_module_ops(DUBAI_MODULE_GPU);
	dubai_reset_gpu_freq_stats();
}
