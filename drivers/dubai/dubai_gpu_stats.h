#ifndef DUBAI_GPU_STATS_H
#define DUBAI_GPU_STATS_H

#include <chipset_common/dubai/dubai_gpu.h>

#define DUBAI_GPU_FREQ_MAX_SIZE		20

struct dubai_gpu_stats {
	int32_t num;
	struct dubai_gpu_freq_info array[DUBAI_GPU_FREQ_MAX_SIZE];
} __packed;

long dubai_ioctl_gpu(unsigned int cmd, void __user *argp);
void dubai_gpu_stats_init(void);
void dubai_gpu_stats_exit(void);
int dubai_gpu_register_ops(struct dubai_gpu_stats_ops *op);
int dubai_gpu_unregister_ops(void);

#endif // DUBAI_GPU_STATS_H
