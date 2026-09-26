/*
 * hisi_hi6521_charger_power.h
 *
 * hisi_hi6521_charger_power
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#ifndef HISI_HI6521__CHARGER_POWER_H
#define HISI_HI6521__CHARGER_POWER_H

extern int scharger_power_status(int id);
extern int scharger_power_on(int id);
extern int scharger_power_off(int id);
extern int scharger_power_set_voltage(unsigned int vset_regs,unsigned int mask,int shift,int index);
extern int scharger_power_get_voltage_index(unsigned int vget_regs,unsigned int mask,int shift);
extern int scharger_power_set_current_limit(unsigned int vset_regs,unsigned int mask,int shift,int index);
extern int scharger_power_get_current_limit_index(unsigned int vget_regs,unsigned int mask,int shift);
extern int scharger_flash_led_timeout_disable(void);
extern int scharger_flash_torch_timeout_config(unsigned int timeoutSec);

#ifdef CONFIG_SCHARGER_V200

int scharger_flash_led_timeout_config(unsigned int timeoutMs);
int scharger_flash_led_timeout_enable(void);
int scharger_register_notifier(struct notifier_block *nb);
int scharger_unregister_notifier(struct notifier_block *nb);
int scharger_flash_bst_vo_config(int config_voltage);
#else
static inline int scharger_flash_led_timeout_config(unsigned int timeoutMs)
{
	return 0;
}
static inline int scharger_flash_led_timeout_enable(void)
{
	return 0;
}
static inline int scharger_register_notifier(struct notifier_block *nb)
{
	return 0;
}
static inline int scharger_unregister_notifier(struct notifier_block *nb)
{
	return 0;
}
static inline int scharger_flash_bst_vo_config(int config_voltage)
{
	return 0;
}
#endif
#endif
