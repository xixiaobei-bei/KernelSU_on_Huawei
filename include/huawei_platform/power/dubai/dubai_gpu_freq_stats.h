#ifndef DUBAI_GPU_FREQ_STATS_H
#define DUBAI_GPU_FREQ_STATS_H

#include <linux/types.h>

extern void dubai_set_gpu_freq_num(int num);
extern void dubai_update_gpu_info(int64_t freq, int64_t busy_time, int64_t total_time, int32_t cycle_ms);

#endif // DUBAI_GPU_FREQ_STATS_H
