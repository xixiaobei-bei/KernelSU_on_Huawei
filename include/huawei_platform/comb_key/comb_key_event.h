/*
 * comb_key.h
 *
 * comb_key driver
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
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

#ifndef _COMB_KEY_EVENT_H
#define _COMB_KEY_EVENT_H
#include  <linux/notifier.h>

enum comb_key_press_staus {
	COMB_KEY_MIN_STATUS,
	COMB_KEY_PRESS_DOWN,
	COMB_KEY_PRESS_RELEASE,
	COMB_KEY_MAX_STATUS
};

int comb_key_register_notifier(struct notifier_block *nb);
int comb_key_unregister_notifier(struct notifier_block *nb);
int comb_key_call_notifiers(unsigned long val,void *v);

#endif
