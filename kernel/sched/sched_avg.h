/*
 * sched_avg.h
 *
 * sched running average statistics
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef __SCHED_AVG_H
#define __SCHED_AVG_H

#define NR_AVG_AMP	128U
#define UPDATE_INIT_20MS	20
#define UPDATE_INTERVAL_MAX	1000U
#define UPDATE_INTERVAL_MIN	10U

struct sched_nr_stat {
	unsigned int avg;
	unsigned int iowait_avg;
	unsigned int big_avg;
	unsigned int nr_max;
};

struct rq;

#ifdef CONFIG_SCHED_RUNNING_AVG
bool sched_update_running_avg(void);

void sched_get_cpus_running_avg(const struct cpumask *cpus,
				struct sched_nr_stat *stat);

void sched_update_nr_prod(struct rq *updated_rq);
unsigned int sched_get_cpus_real_big_tasks(const struct cpumask *cpus);
void request_running_avg_update_ms(unsigned int update_ms);
#else
static inline bool sched_update_running_avg(void) { return false; }

static inline
void sched_get_cpus_running_avg(const struct cpumask *cpus,
				struct sched_nr_stat *stat) {}

static inline void sched_update_nr_prod(struct rq *updated_rq) {}
static inline
unsigned int sched_get_cpus_real_big_tasks(const struct cpumask *cpus) { return 0; }
#endif

#endif
