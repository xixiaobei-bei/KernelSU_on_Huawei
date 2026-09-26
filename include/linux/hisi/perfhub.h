/* Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
 * Description: typing the core
 */

#ifndef _PERFHUB_H
#define _PERFHUB_H

#ifdef CONFIG_FREQ_LIMIT_COUNTER
#include <linux/cpufreq.h>
void update_cpu_ipa_max_freq(struct cpufreq_policy *policy, unsigned int freq);
void update_cpu_freq_limit_counter(struct cpufreq_policy *policy, unsigned int target_freq);
void update_gpu_freq_limit_counter(unsigned int target_freq, unsigned int max_freq);
#endif
#endif /* _PERFHUB_H */
