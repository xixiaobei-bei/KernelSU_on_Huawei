/*
 * Huawei Touchscreen Driver
 *
 * Copyright (C) 2017-2020 Huawei Device Co.Ltd
 * License terms: GNU General Public License (GPL) version 2
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

#ifndef __HUAWEI_THP_ATTR_H_
#define __HUAWEI_THP_ATTR_H_

int is_tp_detected(void);
int thp_set_prox_switch_status(bool enable);
bool thp_get_prox_switch_status(void);

#endif
