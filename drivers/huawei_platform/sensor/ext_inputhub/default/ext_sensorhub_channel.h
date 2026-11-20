/*
 * ext_sensorhub_channel.h
 *
 * head file for external sensorhub channel driver
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

#ifndef KEXT_SNESORHUB_CHANNEL_H
#define KEXT_SNESORHUB_CHANNEL_H

#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/compat.h>
#define EXT_SENSORHUBIO 0xB3

#define EXT_SENSORHUB_IOCTL_SEND_CMD	_IOW(EXT_SENSORHUBIO, 0x01, short)
#define EXT_SENSORHUB_IOCTL_READ_DATA	_IOR(EXT_SENSORHUBIO, 0x02, short)
#define EXT_SENSORHUB_IOCTL_CONFIG_CMD	_IOW(EXT_SENSORHUBIO, 0x03, short)
#define EXT_SENSORHUB_IOCTL_QUERY_MAX_LEN _IOR(EXT_SENSORHUBIO, 0x04, short)

struct ioctl_send_data {
	/* channel id */
	unsigned short port;
	unsigned char service_id;
	unsigned char command_id;
	bool need_resp;
	unsigned char *send_buffer;
	int send_buffer_len;
	unsigned char *receive_buffer;
	int receive_buffer_len;
};

struct ioctl_read_data {
	/* channel id */
	unsigned short port;
	unsigned char service_id;
	unsigned char command_id;
	unsigned char *receive_buffer;
	int receive_buffer_len;
};

struct ioctl_config_cmd {
	/* channel id */
	unsigned short port;
	unsigned short cmd_cnt;
	unsigned char *service_ids;
	unsigned char *command_ids;
};

struct ioctl_send_data32 {
	/* channel id */
	unsigned short port;
	unsigned char service_id;
	unsigned char command_id;
	bool need_resp;
	compat_uptr_t send_buffer; /* unsigned char */
	int send_buffer_len;
	compat_uptr_t receive_buffer; /* unsigned char */
	int receive_buffer_len;
};

struct ioctl_read_data32 {
	/* channel id */
	unsigned short port;
	unsigned char service_id;
	unsigned char command_id;
	compat_uptr_t receive_buffer; /* unsigned char */
	int receive_buffer_len;
};

struct ioctl_config_cmd32 {
	/* channel id */
	unsigned short port;
	unsigned short cmd_cnt;
	compat_uptr_t service_ids; /* unsigned char */
	compat_uptr_t command_ids; /* unsigned char */
};

#endif /* EXT_SNESORHUB_CHANNEL_H */
