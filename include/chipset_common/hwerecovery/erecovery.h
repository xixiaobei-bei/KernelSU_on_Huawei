/*
 * erecovery.h
 *
 * This file is erecovery header file
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

#ifndef __ERECOVERY_H
#define __ERECOVERY_H

#include <linux/fs.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PROCESSNAME_LENGTH 128
#define MAX_FINGERPRINT_LENGTH 16
#define MAX_REASON_LENGTH 64
#define MAX_RESERVED_LENGTH 128
#define ERECOVERY_CMD_LEN_MAX 512
#define ERECOVERY_MSG_LEN_MAX (15 * 1024)
#define ERECOVERY_CMD_INVALID 0xFF

enum erec_event_type {
	EVENT_BEGIN = 0,
	EVENT_END,
	EVENT_REQUEST,
	EVENT_RESULT,
};

enum event_status {
	EVENT_SUCCESS = 0,
	EVENT_FAIL,
	EVENT_PENDING,
};

struct erecovery_eventobj {
	long erecovery_id;
	long fault_id;
	long pid;
	char process_name[MAX_PROCESSNAME_LENGTH];
	char fingerprint[MAX_FINGERPRINT_LENGTH];
	long time;
	long state;
	long result;
	char reason[MAX_REASON_LENGTH];
	char Reserved[MAX_RESERVED_LENGTH];
};

struct erecovery_write_event {
	uint32_t magic;
	struct erecovery_eventobj ere_obj;
};

long erecovery_report(struct erecovery_eventobj *eventdata);
long erecovery_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

#ifdef __cplusplus
}
#endif
#endif
