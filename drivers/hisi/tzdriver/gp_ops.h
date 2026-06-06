/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 * Description:  Function declaration for alloc global operation and pass params to TEE.
 * Author: qiqingchao  q00XXXXXX
 * Create: 2016-06-21
 */
#ifndef _GP_OPS_H_
#define _GP_OPS_H_

int tc_user_param_valid(tc_ns_client_context *client_context, int n);
int tc_client_call(tc_ns_client_context *client_context,
	tc_ns_dev_file *dev_file, tc_ns_session *session, uint8_t flags);
#endif
