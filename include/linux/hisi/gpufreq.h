/*
 * gpufreq.h
 *
 * gpufreq share
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
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
#ifndef _GPUFREQ_H
#define _GPUFREQ_H


/*
 * struct devfreq_data :
 * Private data given from devfreq user device to governors.
 *
 * @vsync_hit: Indicate whether vsync signal is hit or miss.
 * @cl_boost: Indicate whether to boost the freq in OpenCL scene.
 */
struct devfreq_data {
	int vsync_hit;
	int cl_boost;
};

#define GPU_DEFAULT_GOVERNOR	"gpu_scene_aware"

#ifdef CONFIG_GPU_FHSS
int fhss_init(struct device *dev);
#else
static inline int fhss_init(struct device *dev){return 0;};
#endif

/* Hz */
void update_gputop_linked_freq(unsigned long freq);
/* MHz */
void ipa_gputop_freq_limit(unsigned long freq);

#endif /* _GPUFREQ_H */
