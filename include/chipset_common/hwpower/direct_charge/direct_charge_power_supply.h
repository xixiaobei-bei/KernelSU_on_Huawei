/*
 * direct_charge_power_supply.h
 *
 * power supply for direct charge
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

#ifndef _DIRECT_CHARGE_POWER_SUPPLY_H_
#define _DIRECT_CHARGE_POWER_SUPPLY_H_

#ifdef CONFIG_DIRECT_CHARGER
int dc_set_bst_ctrl(int enable);
#else
static inline int dc_set_bst_ctrl(int enable)
{
	return 0;
}
#endif /* CONFIG_DIRECT_CHARGER */

#endif /* _DIRECT_CHARGE_POWER_SUPPLY_H_ */
