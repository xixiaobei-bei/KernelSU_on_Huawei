/*
 * tlogger.h
 *
 * TEE Logging Subsystem, read the tee os log from rdr memory
 *
 * Copyright (c) 2012-2018 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef _TLOGGER_H_
#define _TLOGGER_H_

void tz_log_write(void);
int tlogger_store_lastmsg(void);

#endif
