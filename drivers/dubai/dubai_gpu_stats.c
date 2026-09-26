#include "dubai_gpu_stats.h"

#include <linux/slab.h>

#include <chipset_common/dubai/dubai_ioctl.h>

#include "utils/dubai_utils.h"

#define IOC_GPU_ENABLE DUBAI_GPU_DIR_IOC(W, 1, bool)
#define IOC_GPU_INFO_GET DUBAI_GPU_DIR_IOC(R, 2, struct dubai_gpu_stats)

static int dubai_set_gpu_enable(void __user *argp);
static int dubai_get_gpu_info(void __user *argp);

static int freq_num = 0;
static struct dubai_gpu_freq_info *freq_stats = NULL;
static struct dubai_gpu_stats_ops *stats_op = NULL;

long dubai_ioctl_gpu(unsigned int cmd, void __user *argp)
{
	int rc = 0;

	switch (cmd) {
	case IOC_GPU_ENABLE:
		rc = dubai_set_gpu_enable(argp);
		break;
	case IOC_GPU_INFO_GET:
		rc = dubai_get_gpu_info(argp);
		break;
	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

static void dubai_reset_gpu_stats(void)
{
	if (freq_stats) {
		kfree(freq_stats);
		freq_stats = NULL;
	}
	freq_num = 0;
	stats_op = NULL;
}

static int dubai_set_gpu_enable(void __user *argp)
{
	bool enable = false;

	if (!stats_op || !stats_op->enable || !stats_op->get_num) {
		dubai_info("Gpu stats not supported");
		return -EOPNOTSUPP;
	}

	if (get_enable_value(argp, &enable)) {
		dubai_err("Failed to get value from user");
		goto enable_failed;
	}

	if (stats_op->enable(enable)) {
		dubai_err("Failed to enable stats");
		goto enable_failed;
	}

	if (enable) {
		freq_num = stats_op->get_num();
		if (freq_num <= 0) {
			dubai_err("Failed to get freq num: %d", freq_num);
			goto init_failed;
		}
	} else {
		dubai_reset_gpu_stats();
	}

	return 0;

init_failed:
	(void)stats_op->enable(false);

enable_failed:
	dubai_gpu_unregister_ops();

	return -EFAULT;
}

static void dubai_add_gpu_delta_stats(
	struct dubai_gpu_stats *stats, int64_t freq, int64_t run_time, int64_t idle_time)
{
	if (unlikely((stats->num < 0) || (stats->num >= DUBAI_GPU_FREQ_MAX_SIZE))) {
		return;
	}

	if ((run_time <= 0) && (idle_time <= 0)) {
		return;
	}

	stats->array[stats->num].run_time = (run_time > 0) ? run_time : 0;
	stats->array[stats->num].idle_time = (idle_time > 0) ? idle_time : 0;
	stats->array[stats->num].freq = freq;
	stats->num++;
}

static int dubai_get_gpu_info(void __user *argp)
{
	int i;
	struct dubai_gpu_freq_info *new_stats = NULL;
	struct dubai_gpu_stats delta_stats;

	if (!stats_op || !stats_op->get_num || !stats_op->get_stats || (freq_num <= 0)) {
		dubai_err("Gpu stats not supported");
		return -EOPNOTSUPP;
	}

	new_stats = kzalloc(freq_num * sizeof(struct dubai_gpu_freq_info), GFP_KERNEL);
	if (!new_stats) {
		dubai_err("Failed to alloc");
		return -ENOMEM;
	}

	if (stats_op->get_stats(new_stats, freq_num)) {
		dubai_err("Failed to get stats");
		kfree(new_stats);
		return -EFAULT;
	}

	memset(&delta_stats, 0, sizeof(struct dubai_gpu_stats));
	for (i = 0; i < freq_num; i++) {
		if (new_stats[i].freq <= 0)
			continue;

		if (freq_stats && (freq_stats[i].freq == new_stats[i].freq)) {
			dubai_add_gpu_delta_stats(&delta_stats, new_stats[i].freq,
				new_stats[i].run_time - freq_stats[i].run_time,
				new_stats[i].idle_time - freq_stats[i].idle_time);
		} else {
			dubai_add_gpu_delta_stats(&delta_stats, new_stats[i].freq,
				new_stats[i].run_time, new_stats[i].idle_time);
		}
	}
	if (freq_stats) {
		kfree(freq_stats);
	}
	freq_stats = new_stats;

	if (delta_stats.num > 0) {
		if (unlikely(copy_to_user(argp, &delta_stats, sizeof(struct dubai_gpu_stats))))
			return -EFAULT;
	}

	return 0;
}

int dubai_gpu_register_ops(struct dubai_gpu_stats_ops *op)
{
	if (!stats_op)
		stats_op = op;
	return 0;
}

int dubai_gpu_unregister_ops(void)
{
	if (stats_op)
		stats_op = NULL;
	return 0;
}

void dubai_gpu_stats_init(void)
{
}

void dubai_gpu_stats_exit(void)
{
	dubai_reset_gpu_stats();
}
