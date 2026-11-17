/*
 * power_vote.h
 *
 * vote for power module
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _POWER_VOTE_H_
#define _POWER_VOTE_H_

#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/sysfs.h>
#include <linux/string.h>
#include <linux/platform_device.h>

#define POWER_VOTE_RD_BUF_SIZE        128
#define POWER_VOTE_WR_BUF_SIZE        32
#define POWER_VOTE_MAX_CLIENTS        32

enum power_vote_type {
	POWER_VOTE_TYPE_BEGIN = 0,
	POWER_VOTE_SET_MIN = POWER_VOTE_TYPE_BEGIN,
	POWER_VOTE_SET_MAX,
	POWER_VOTE_SET_ANY,
	POWER_VOTE_TYPE_END,
};

struct power_vote_client_data {
	char *name;
	bool enabled;
	int value;
};

struct power_vote_object {
	const char *name;
	struct list_head list;
	struct mutex lock;
	int type;
	bool voted_on;
	int eff_client_id;
	int eff_result;
	int override_result;
	struct power_vote_client_data clients[POWER_VOTE_MAX_CLIENTS];
	void *data;
	int (*cb)(struct power_vote_object *, void *, int, const char *);
};

typedef int (*power_vote_cb)(struct power_vote_object *, void *, int, const char *);

int power_vote_create_object(const char *name,
	int vote_type, power_vote_cb cb, void *data);
void power_vote_destroy_object(const char *name);
int power_vote_set(const char *name,
	const char *client_name, bool enabled, int value);
int power_vote_set_override(const char *name,
	const char *client_name, bool enabled, int value);
bool power_vote_is_client_enabled_locked(const char *name,
	const char *client_name, bool lock_flag);
int power_vote_get_client_value_locked(const char *name,
	const char *client_name, bool lock_flag);
int power_vote_get_effective_result_locked(const char *name, bool lock_flag);

#endif /* _POWER_VOTE_H_ */
