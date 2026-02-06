/*
 * rdr_recovery_exception.h
 *
 * blackbox save recovery log  moudle
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
#ifndef __RDR_RECOVERY_EXCEPTION_H__
#define __RDR_RECOVERY_EXCEPTION_H__
#define RECOVERY_LOG_PATH    "/splash2/recovery/hisi_logs/"
#define RECOVERY_WAIT_TIME    60
#define MAX_LOG_NUM    5
#define RECOVERY_HISTORY_LOG_MAX     0x1000 /* 4k */

void rdr_recovery_exce(void);
#endif