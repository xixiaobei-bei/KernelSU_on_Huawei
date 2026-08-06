/*
 * power_manager.h
 *
 * This is Power mananger.
 *
 * Copyright (c) 2020-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __POWER_MANAGER_H__
#define __POWER_MANAGER_H__

#include <linux/seqlock.h>

#define PREIOD_LENGTH 5
/*
 * If one frame is sent within 10 ms on average,
 * frequent power-on and power-off may cause CPU power consumption and encoding performance problems.
 */
#define PREIOD_TIME_LIMIT (PREIOD_LENGTH * 10000)
#define HARDWARE_USAGE_LIMIT 80
#define PM_INFO_BUFFER_SIZE 512

enum {
	POWER_ON_TIME = 0,
	BUSY_TIME,
	TIME_BUTT
};

struct pm_base_time {
	uint64_t count;
	uint64_t start_tick;
};

struct pm_stats_time {
	uint64_t count;
	uint64_t start_tick;
	uint64_t prev_stats_time;
	uint64_t stats_time;
};

struct pm_period_time {
	uint64_t start_tick;      // Start time of the statistical period.
	uint64_t power_on_time;
	uint64_t busy_time;
	uint64_t period_time;     // Statistical period duration
};

struct power_manager {
	bool low_power_enable;
	seqlock_t seq_lock;
	struct pm_base_time base_time;
	struct pm_stats_time stats_time[TIME_BUTT];
	struct pm_period_time period_time;
};

struct pm_period_info {
	uint64_t busy_time;
	uint64_t power_on_time;
	uint64_t total_time;
};

struct pm_total_info {
	uint64_t busy_time;
	uint64_t power_on_time;
	uint64_t total_time;
	uint64_t power_off_count;
	uint64_t frame_count;
};

void pm_init_account(struct power_manager *pm);
void pm_deinit_account(struct power_manager *pm);
void pm_update_account(struct power_manager *pm);
void pm_hardware_power_on_enter(struct power_manager *pm);
void pm_hardware_power_on_exit(struct power_manager *pm);
void pm_hardware_busy_enter(struct power_manager *pm);
void pm_hardware_busy_exit(struct power_manager *pm);
bool pm_if_need_power_off(struct power_manager *pm);
int pm_show_period_account(struct power_manager *pm, char *buf, int32_t size);
int pm_show_total_account(struct power_manager *pm, char *buf, int32_t size);

#endif
