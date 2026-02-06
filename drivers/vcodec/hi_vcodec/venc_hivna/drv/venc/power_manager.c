/*
 * power_manager.c
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

#include "power_manager.h"
#include "drv_venc_osal.h"

void pm_init_account(struct power_manager *pm)
{
	(void)memset_s(pm, sizeof(struct power_manager), 0, sizeof(struct power_manager));
	seqlock_init(&pm->seq_lock);
	pm->base_time.start_tick = osal_get_sys_time_in_us();
	pm->low_power_enable = true;
}

void pm_deinit_account(struct power_manager *pm)
{
	(void)memset_s(pm, sizeof(struct power_manager), 0, sizeof(struct power_manager));
}

static uint64_t update_account(struct pm_stats_time *stats_time, uint64_t now)
{
	uint64_t stats_time_in_period;

	if (stats_time->start_tick == 0) {
		stats_time_in_period = stats_time->stats_time - stats_time->prev_stats_time;
		stats_time->prev_stats_time = stats_time->stats_time;
		return stats_time_in_period;
	}

	stats_time->stats_time += now - stats_time->start_tick;
	stats_time_in_period = stats_time->stats_time - stats_time->prev_stats_time;
	stats_time->start_tick = now;
	stats_time->prev_stats_time = stats_time->stats_time;

	return stats_time_in_period;
}

void pm_update_account(struct power_manager *pm)
{
	uint32_t flags;
	uint64_t now;
	struct pm_stats_time *power_on_time = pm->stats_time + POWER_ON_TIME;
	struct pm_stats_time *busy_time = pm->stats_time + BUSY_TIME;
	struct pm_base_time *base_time = &pm->base_time;
	struct pm_period_time *period_time = &pm->period_time;

	if (!pm->low_power_enable)
		return;

	write_seqlock_irqsave(&pm->seq_lock, flags);
	now = osal_get_sys_time_in_us();
	if (base_time->count == 0) {
		period_time->start_tick = now;
		power_on_time->start_tick = power_on_time->start_tick ? now : 0;
		busy_time->start_tick = busy_time->start_tick ? now : 0;
		power_on_time->prev_stats_time = power_on_time->stats_time = 0;
		busy_time->prev_stats_time = busy_time->stats_time = 0;
		goto exit;
	}

	if (base_time->count % PREIOD_LENGTH)
		goto exit;

	period_time->power_on_time = update_account(power_on_time, now);
	period_time->busy_time = update_account(busy_time, now);
	period_time->period_time = time_period(period_time->start_tick, now);
	period_time->start_tick = now;

exit:
	base_time->count++;
	write_sequnlock_irqrestore(&pm->seq_lock, flags);
}

static void hardware_stats_enter(struct pm_stats_time *stats_time)
{
	stats_time->start_tick = osal_get_sys_time_in_us();
}

static void hardware_stats_exit(struct pm_stats_time *stats_time)
{
	uint64_t now = osal_get_sys_time_in_us();

	stats_time->stats_time += time_period(stats_time->start_tick, now);
	stats_time->count++;
	stats_time->start_tick = 0;
}

void pm_hardware_power_on_enter(struct power_manager *pm)
{
	uint32_t flags;

	if (!pm->low_power_enable)
		return;

	write_seqlock_irqsave(&pm->seq_lock, flags);
	hardware_stats_enter(pm->stats_time + POWER_ON_TIME);
	write_sequnlock_irqrestore(&pm->seq_lock, flags);
}

void pm_hardware_power_on_exit(struct power_manager *pm)
{
	uint32_t flags;

	if (!pm->low_power_enable)
		return;

	write_seqlock_irqsave(&pm->seq_lock, flags);
	hardware_stats_exit(pm->stats_time + POWER_ON_TIME);
	write_sequnlock_irqrestore(&pm->seq_lock, flags);
}

void pm_hardware_busy_enter(struct power_manager *pm)
{
	uint32_t flags;

	if (!pm->low_power_enable)
		return;

	write_seqlock_irqsave(&pm->seq_lock, flags);
	hardware_stats_enter(pm->stats_time + BUSY_TIME);
	write_sequnlock_irqrestore(&pm->seq_lock, flags);
}

void pm_hardware_busy_exit(struct power_manager *pm)
{
	uint32_t flags;

	if (!pm->low_power_enable)
		return;

	write_seqlock_irqsave(&pm->seq_lock, flags);
	hardware_stats_exit(pm->stats_time + BUSY_TIME);
	write_sequnlock_irqrestore(&pm->seq_lock, flags);
}

bool pm_if_need_power_off(struct power_manager *pm)
{
	uint32_t seq;
	uint32_t next_seq = 0;
	uint32_t flags;
	uint64_t total_period_time;
	uint64_t busy_period_time;
	struct pm_period_time *period_time = &pm->period_time;

	if (!pm->low_power_enable)
		return false;

	do {
		seq = next_seq;
		flags = read_seqbegin_or_lock_irqsave(&pm->seq_lock, &seq);
		total_period_time = period_time->period_time;
		busy_period_time = period_time->busy_time;
		/* If lockless access failed, take the lock. */
		next_seq = 1;
	} while (need_seqretry(&pm->seq_lock, seq));
	done_seqretry_irqrestore(&pm->seq_lock, seq, flags);

	if (total_period_time == 0)
		return false;

	return (total_period_time > PREIOD_TIME_LIMIT)
		&& (busy_period_time * 100 / total_period_time < HARDWARE_USAGE_LIMIT);
}

int32_t pm_show_period_account(struct power_manager *pm, char *buf, int32_t size)
{
	uint32_t seq;
	uint32_t next_seq = 0;
	uint32_t flags;
	struct pm_period_info info = { 0 };
	struct pm_period_time *period_time = &pm->period_time;

	if (!pm->low_power_enable || size <= 0)
		return 0;

	do {
		seq = next_seq;
		flags = read_seqbegin_or_lock_irqsave(&pm->seq_lock, &seq);
		info.busy_time = period_time->busy_time;
		info.power_on_time = period_time->power_on_time;
		info.total_time = period_time->period_time;
		/* If lockless access failed, take the lock. */
		next_seq = 1;
	} while (need_seqretry(&pm->seq_lock, seq));
	done_seqretry_irqrestore(&pm->seq_lock, seq, flags);

	if (info.total_time == 0)
		return 0;

	return sprintf_s(buf, size,
		"busy period time %llu us, power on period time %llu us, total period time %llu us, usage rate %llu\n",
		info.busy_time, info.power_on_time, info.total_time, info.busy_time * 100 / info.total_time);
}

static uint64_t get_total_account(struct pm_stats_time *stats_time, uint64_t now)
{
	if (stats_time->start_tick == 0)
		return stats_time->stats_time;

	return now - stats_time->start_tick + stats_time->stats_time;
}

int pm_show_total_account(struct power_manager *pm, char *buf, int32_t size)
{
	uint32_t seq;
	uint32_t next_seq = 0;
	uint32_t flags;
	uint64_t now;
	struct pm_total_info info = { 0 };
	struct pm_base_time *base_time = &pm->base_time;

	if (!pm->low_power_enable || size <= 0)
		return 0;

	do {
		seq = next_seq;
		flags = read_seqbegin_or_lock_irqsave(&pm->seq_lock, &seq);
		now = osal_get_sys_time_in_us();
		info.busy_time = get_total_account(pm->stats_time + BUSY_TIME, now) / 1000;
		info.power_on_time = get_total_account(pm->stats_time + POWER_ON_TIME, now) / 1000;
		info.total_time = time_period(base_time->start_tick, now) / 1000;
		info.power_off_count = pm->stats_time[POWER_ON_TIME].count;
		info.frame_count = base_time->count;
		/* If lockless access failed, take the lock. */
		next_seq = 1;
	} while (need_seqretry(&pm->seq_lock, seq));
	done_seqretry_irqrestore(&pm->seq_lock, seq, flags);

	return sprintf_s(buf, size, "total busy time %llu ms, total power on time %llu ms, total time %llu ms, "
			"total power off count %llu, total frame count %llu\n",
			info.busy_time, info.power_on_time, info.total_time, info.power_off_count, info.frame_count);
}
