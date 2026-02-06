/*
 * lcdkit_ext.h
 *
 * lcdkit ext head file for lcd driver
 *
 * Copyright (c) 2018-2020 Huawei Technologies Co., Ltd.
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

#ifndef __LCDKIT_EXT_H_
#define __LCDKIT_EXT_H_
/* TS Event */
enum lcdkit_pm_type
{
	LCDKIT_TS_BEFORE_SUSPEND = 0,
	LCDKIT_TS_SUSPEND_DEVICE,
	LCDKIT_TS_RESUME_DEVICE,
	LCDKIT_TS_AFTER_RESUME,
	LCDKIT_TS_IC_SHUT_DOWN,
	LCDKIT_TS_EARLY_SUSPEND,
	LCDKIT_TS_I2C_TO_AP,
	LCDKIT_TS_I2C_TO_MCU,
};

/* TS sync */
#define LCDKIT_NO_SYNC_TIMEOUT		0
#define LCDKIT_SHORT_SYNC_TIMEOUT		5

/* Function declare */
int lcdkit_register_notifier(struct notifier_block* nb);
int lcdkit_unregister_notifier(struct notifier_block* nb);
void lcdkit_notifier_call_chain(unsigned long event, void* data);
int lcdkit_proximity_poweroff(void);
int lcdkit_set_lcd_reset_low(void);
#endif
