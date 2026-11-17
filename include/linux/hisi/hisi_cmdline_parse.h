/*
 *
 * cmdline parse head file
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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
#ifndef __CMDLINE_PARSE_H
#define __CMDLINE_PARSE_H

int get_low_battery_flag(void);
void set_low_battery_flag(int flag);

unsigned int get_boot_into_recovery_flag(void);
unsigned int get_cmd_sepc_info(void);

#endif
