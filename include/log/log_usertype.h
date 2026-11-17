/*
 * log_usertype.h
 *
 * interfaces to indicate user types in kernel
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#ifndef _LOG_USERTYPE_H
#define _LOG_USERTYPE_H

#define COMMERCIAL_USER		1
#define FANS_USER		2
#define BETA_USER		3
#define TEST_USER		4
#define OVERSEA_USER		5
#define OVERSEA_COMMERCIAL_USER	6

unsigned int get_log_usertype(void);

#define get_logusertype_flag	get_log_usertype

#endif
