/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: camera buf
 * Author: yangkai
 * Create: 2020-01-11
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 */

#ifndef HISP300_MSG_ISPNN_H_INCLUDED
#define HISP300_MSG_ISPNN_H_INCLUDED

#define MAX_ONLINE_FRAME_NUM 4
#define MAX_MULTIS_FRAME_NUM 4
#define ISPNN_MODEL_ID_MAX 23 /* 23:ISPNN_MODEL_MAX */

typedef struct _buffer_map_t {
	unsigned int isp_arch_addr; /* R8(CS) or ARCH(ES) */
	unsigned int isp_core_addr; /* ISP pipeline */
	unsigned int npu_arch_addr; /* npu TS */
	long long npu_core_addr; /* AI Core */
	unsigned int buffer_size;
} buffer_map_t;

typedef struct _buffer_isp_t {
	unsigned int isp_core_addr; /* ISP pipeline */
	unsigned int buffer_size;
} buffer_isp_t;

typedef struct {
	unsigned int model_id;
	buffer_map_t offline_model_buf;
	buffer_map_t offline_kernel_buf;
	buffer_map_t online_model_buf[MAX_ONLINE_FRAME_NUM];
} ispnn_model_info_t;

typedef struct _buf_create_ispnn_model_t {
	unsigned int create_count;
	buffer_map_t syscache_buf;
	buffer_isp_t isp_offline_buf;
	ispnn_model_info_t ispnn_model_info[ISPNN_MODEL_ID_MAX];
} buf_create_ispnn_model_t;

typedef struct _msg_req_create_ispnn_model_t {
	unsigned int create_ispnn_model_buffer;
} msg_req_create_ispnn_model_t;

typedef struct _msg_req_extend_ispnn_buffer_t {
	buffer_map_t multiframe_input_buf[MAX_MULTIS_FRAME_NUM];
} msg_req_extend_ispnn_buffer_t;

typedef struct _msg_req_enable_ispnn_model_t {
	unsigned int cam_id;
	unsigned int model_type;
	unsigned int enable_count;
	unsigned int model_id[ISPNN_MODEL_ID_MAX];
	unsigned int flag;
	unsigned int sync_pq_flag;
} msg_req_enable_ispnn_model_t;

typedef struct _msg_req_disable_ispnn_model_t {
	unsigned int cam_id;
	unsigned int disable_count;
	unsigned int model_id[ISPNN_MODEL_ID_MAX];
	unsigned int flag;
	unsigned int sync_pq_flag;
} msg_req_disable_ispnn_model_t;

typedef struct _msg_req_destroy_ispnn_model_t {
	unsigned int destroy_count;
	unsigned int model_id[ISPNN_MODEL_ID_MAX];
	unsigned int reserved;
} msg_req_destroy_ispnn_model_t;

typedef struct _msg_ack_create_ispnn_model_t {
	unsigned int status;
} msg_ack_create_ispnn_model_t;

typedef struct _msg_ack_extend_ispnn_buffer_t {
	unsigned int model_id;
	unsigned int status;
} msg_ack_extend_ispnn_buffer_t;

typedef struct _msg_ack_enable_ispnn_model_t {
	unsigned int cam_id;
	unsigned int status;
} msg_ack_enable_ispnn_model_t;

typedef struct _msg_ack_disable_ispnn_model_t {
	unsigned int cam_id;
	unsigned int model_id;
	unsigned int status;
} msg_ack_disable_ispnn_model_t;

typedef struct _msg_ack_destroy_ispnn_model_t {
	unsigned int model_id;
	unsigned int status;
} msg_ack_destroy_ispnn_model_t;

#endif /* HISP300_MSG_ISPNN_H_INCLUDED */