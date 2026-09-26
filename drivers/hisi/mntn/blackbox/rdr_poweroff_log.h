/*
 * rdr_poweroff_log.h
 *
 * blackbox header file (blackbox: kernel run data recorder.)
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

#ifndef __BB_POWEROFF_H__
#define __BB_POWEROFF_H__

#define SHTDOWN_REASON_MAXLEN 64

#define POWEROFF_REASON_FILE_LIMIT (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

#define RDR_POWEROFF_TIME_OUT 5

int rdr_poweroff_log_init(void);

#endif /* End #define __POWEROFF_H__ */
