/*
 * adsp misc interface
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
#ifndef _ADSP_MISC_H_
#define _ADSP_MISC_H_

#define MAX_PAYLOAD_SIZE 450
#define TINY_PAYLOAD_SIZE 100
#define MAX_GET_PARAMS_NUM 100
#define MAX_PARAM_SIZE 14000
#define PROTECT_MAGIC_NUM 8888

struct adsp_header {
	unsigned int protect_header;
	unsigned short cmd;
	unsigned short type;
};

struct adsp_set_param {
	struct adsp_header header;
	unsigned short payload_len;
	unsigned char payload[MAX_PAYLOAD_SIZE];
	unsigned short reserve[10];
};

struct adsp_get_param {
	struct adsp_header header;
	unsigned short payload_len;
	unsigned char payload[TINY_PAYLOAD_SIZE];
	// result
	unsigned short result_len;
	unsigned char result[MAX_GET_PARAMS_NUM];
};

struct adsp_set_smartpa_param {
	struct adsp_header header;
	unsigned short param_size;
	unsigned char param[MAX_PARAM_SIZE];
};

enum {
	ADSP_SHARED_MEM_UPDATE = 0x1,
	ADSP_SHARED_MEM_NOTIFY_ONLY = 0x2,
};

struct adsp_set_shared_mem {
	unsigned int cmd;
	unsigned int param_size;
	unsigned char param[0];
};

#define ADSP_IOCTL_MAGIC 'M'
#define IPI_SET_BUF _IOR(ADSP_IOCTL_MAGIC, 0x01, struct adsp_set_param)
#define IPI_GET_BUF _IOR(ADSP_IOCTL_MAGIC, 0x02, struct adsp_get_param)
#define SET_PA_PARAM _IOR(ADSP_IOCTL_MAGIC, 0x03, struct adsp_set_smartpa_param)
#define SET_ADSP_SHARED_MEM _IOR(ADSP_IOCTL_MAGIC, 0x04, struct adsp_set_shared_mem)

#endif
