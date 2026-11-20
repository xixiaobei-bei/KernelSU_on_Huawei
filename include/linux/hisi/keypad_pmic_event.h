/*
  * hisi volume key notifier head file
  *
  * Copyright (C) Huawei Technologies Co., Ltd. 2016. All rights reserved.
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

#ifndef _HISI_VOLUMEKEY_EVENT_H
#define _HISI_VOLUMEKEY_EVENT_H
#include  <linux/notifier.h>


typedef enum {
	KEYPAD_PMIC_DOWN_PRESS = 0,
	KEYPAD_PMIC_DOWN_RELEASE,
	KEYPAD_PMIC_UP_PRESS,
	KEYPAD_PMIC_UP_RELEASE,
	PVOLUMEKEY_MAX
} keypad_pmic_event_t;

#ifdef  CONFIG_KEYBOARD_HISI_PMIC_GPIO_KEY
int keypad_pmic_register_notifier(struct notifier_block *nb);
int keypad_pmic_unregister_notifier(struct notifier_block *nb);
int keypad_pmic_call_notifiers(unsigned long val,void *v);
#endif

#endif
