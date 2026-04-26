/*
 * session_manager.h
 *
 * function declaration for session management
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <linux/fs.h>
#include "tc_ns_client.h"
#include "teek_ns_client.h"

int tc_client_session_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg);
int tc_ns_open_session(struct tc_ns_dev_file *dev_file,
	struct tc_ns_client_context *context);
int tc_ns_close_session(struct tc_ns_dev_file *dev_file,
	const struct tc_ns_client_context *context);
int tc_ns_send_cmd(struct tc_ns_dev_file *dev_file,
	struct tc_ns_client_context *context);
int tc_ns_load_image(struct tc_ns_dev_file *dev,
	const char *file_buffer, unsigned int file_size);
int tc_ns_load_image_with_lock(struct tc_ns_dev_file *dev,
	const char *file_buffer, unsigned int file_size);
void close_unclosed_session_in_kthread(struct tc_ns_dev_file *dev);
struct tc_ns_session *tc_find_session_by_uuid(unsigned int dev_file_id,
	const struct tc_ns_smc_cmd *cmd);
struct tc_ns_service *tc_find_service_in_dev(const struct tc_ns_dev_file *dev,
	const unsigned char *uuid, int uuid_size);
struct tc_ns_session *tc_find_session_withowner(
	const struct list_head *session_list, unsigned int session_id,
	struct tc_ns_dev_file *dev_file);
int tc_ns_load_secfile(struct tc_ns_dev_file *dev_file,
	const void __user *argp);
void get_service_struct(struct tc_ns_service *service);
void put_service_struct(struct tc_ns_service *service);
void get_session_struct(struct tc_ns_session *session);
void put_session_struct(struct tc_ns_session *session);
void dump_services_status(const char *param);
void init_srvc_list(void);

#endif
