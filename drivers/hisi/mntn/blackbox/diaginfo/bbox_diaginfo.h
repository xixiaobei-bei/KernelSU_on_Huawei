/*
 * bbox_diaginfo.h
 *
 * bbox diaginfo main module for bbox diaginfo.
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#ifndef __BBOX_DIAGINFO_H__
#define __BBOX_DIAGINFO_H__
#include <linux/hisi/ipc_msg.h>
#include <bbox_diaginfo_id_def.h>
#include <linux/types.h>
#include <linux/list.h>

#define BBOX_DIAG_DATE_MAXLEN               14 /* 19700101080000 */
#define BBOX_DIAGINFO_ERROR_ID_NAME_LEN     24
#define MAX_DIAGINFO_LEN                  0xA0
#define SEC_TIME_INTERVAL           1000000000
#define LPMCU_DIAGINFO_STRING_MAX_LEN       58
#define MAX_USER_NODES_NUM                  15

/* diaginfo message transfered from lpmcu by ipc */
struct lpmcu_diaginfo {
	u64 ts; /* times of IPC received, 0: means empty msg */
	int diaginfo[MAX_MAIL_SIZE - 1];
};

/* define message information for diaginfo from lpmcu */
struct lpmcu_diaginfo_def {
	unsigned int err_id;
	unsigned int param_num;
	char pmsg[LPMCU_DIAGINFO_STRING_MAX_LEN];
};

/* define diaginfo message list, which whill be store into ddr buffer */
struct bbox_diaginfo_s {
	u64 ts; /* time */
	unsigned int errid;
	unsigned int len;
	char msg[MAX_DIAGINFO_LEN];
	char date[BBOX_DIAG_DATE_MAXLEN + 1];
};

struct bbox_diaginfo_param_s {
	struct list_head diaginfo_list;
	struct bbox_diaginfo_s diaginfo;
};

/* define module, loglevel, fault type for each error id */
struct diaginfo_map {
	enum bbox_diaginfo_errid errid;
	char error_id_name[BBOX_DIAGINFO_ERROR_ID_NAME_LEN];
	enum bbox_diaginfo_module module;
	enum bbox_diaginfo_level log_level;
	enum bbox_diaginfo_type fault_type;
	unsigned int count; /* define hoy many times the erorr_id has be recorded */
};

void set_bbox_diaginfo_need_check_count(unsigned int val);
struct diaginfo_map *get_diaginfo_map(enum bbox_diaginfo_errid errid);
unsigned int get_diaginfo_nodes_num(void);

#endif
