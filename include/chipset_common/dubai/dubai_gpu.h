#ifndef DUBAI_GPU_H
#define DUBAI_GPU_H

#include <linux/types.h>

struct dubai_gpu_freq_info {
	int64_t freq;
	int64_t run_time;
	int64_t idle_time;
} __packed;

typedef int (*dubai_gpu_enable_fn_t)(bool enable);
typedef int (*dubai_gpu_freq_stats_fn_t)(struct dubai_gpu_freq_info *data, int num);
typedef int (*dubai_gpu_freq_count_fn_t)(void);

struct dubai_gpu_stats_ops {
	dubai_gpu_enable_fn_t enable;
	dubai_gpu_freq_stats_fn_t get_stats;
	dubai_gpu_freq_count_fn_t get_num;
};

#endif // DUBAI_GPU_H
