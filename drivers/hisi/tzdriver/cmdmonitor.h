/*
 * cmd_monitor.h
 *
 * cmdmonitor function declaration
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
#include "tzdebug.h"
#include "teek_ns_client.h"

#ifndef _CMD_MONITOR_H_
#define _CMD_MONITOR_H_
void cmd_monitor_log(tc_ns_smc_cmd *cmd);
void cmd_monitor_reset_send_time(void);
void cmd_monitor_logend(tc_ns_smc_cmd *cmd);
void init_cmd_monitor(void);
void do_cmd_need_archivelog(void);
bool is_thread_reported(unsigned int tid);
void tzdebug_archivelog(void);
void cmd_monitor_ta_crash(int32_t type);
void report_imonitor(struct tee_mem *meminfo);
void memstat_report(void);

#endif
