/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: This file describe GPU interface/callback
 * Create: 2019-2-24
 */
#ifndef GPU_HOOK_H
#define GPU_HOOK_H

#include <linux/types.h>

/* GPU AI frequency schedule interface */
struct kbase_fence_info {
	pid_t game_pid;
	u64 signaled_seqno;
};

#ifdef CONFIG_GPU_AI_FENCE_INFO
int perf_ctrl_get_gpu_fence(void __user *uarg);
int perf_ctrl_get_gpu_buffer_size(void __user *uarg);
#else
static inline int perf_ctrl_get_gpu_fence(void __user *uarg)
{
	return -EINVAL;
}
static inline int perf_ctrl_get_gpu_buffer_size(void __user *uarg)
{
	return -EINVAL;
}
#endif
#if defined(CONFIG_GPU_AI_FENCE_INFO) && defined(CONFIG_MALI_LAST_BUFFER)
int perf_ctrl_enable_gpu_lb(const void __user *uarg);
#else
static inline int perf_ctrl_enable_gpu_lb(const void __user *uarg __maybe_unused)
{
	return -EINVAL;
}
#endif

#ifdef CONFIG_GPU_VIRTUAL_DEVFREQ
int gpu_virtual_devfreq_get_freq(unsigned long *freq);
int gpu_virtual_devfreq_get_devfreq_min(unsigned long *devfreq_min);
int gpu_virtual_devfreq_set_core_mask(unsigned long virtual_freq,
	bool virtual_freq_flag);
int gpu_virtual_devfreq_set_mode(bool flag);
int gpu_virtual_devfreq_get_enable(bool *flag);
int gpu_virtual_devfreq_set_enable(bool flag);
#endif

#ifdef CONFIG_GPU_THROTTLE_DEVFREQ
void gpu_devfreq_set_gpu_throttle(unsigned long thro_low_freq,
	unsigned long thro_high_freq, unsigned long *freq);
#endif

#endif /* GPU_HOOK_H */
