/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 * Description: Cmdmonitor function declaration.
 * Author: qiqingchao  q00XXXXXX
 * Create: 2016-06-21
 */

#ifndef _CMD_MONITOR_H_
#define _CMD_MONITOR_H_

void cmd_monitor_log(tc_ns_smc_cmd *cmd);
void cmd_monitor_logend(tc_ns_smc_cmd *cmd);
void init_cmd_monitor(void);
void do_cmd_need_archivelog(void);
bool is_thread_reported(unsigned int tid);
void tzdebug_archivelog(void);
void cmd_monitor_ta_crash(int32_t type);

#endif
