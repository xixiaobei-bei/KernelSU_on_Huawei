/*
 * direct_charge_debug.h
 *
 * debug for for direct charge
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

#ifndef _DIRECT_CHARGE_DEBUG_H_
#define _DIRECT_CHARGE_DEBUG_H_

#ifdef CONFIG_DIRECT_CHARGER
void sc_dbg_register(void *dev_data);
void lvc_dbg_register(void *dev_data);
#else
static inline void sc_dbg_register(void *dev_data)
{
}

static inline void lvc_dbg_register(void *dev_data)
{
}
#endif /* CONFIG_DIRECT_CHARGER */

#endif /* _DIRECT_CHARGE_DEBUG_H_ */
