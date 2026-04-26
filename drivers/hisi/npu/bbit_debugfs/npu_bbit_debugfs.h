/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: for npu drv bbit debug
 */
#ifndef __NPU_BBIT_DEBUGFS_H
#define __NPU_BBIT_DEBUGFS_H

#include <linux/platform_device.h>
#include "npu_log.h"

#define BBIT_DIR_NAME "npu_bbit_debugfs"
#define BBIT_RESULT_VALID 0x5a5a5a5a
#define DEBUGFS_ROOT_DIR NULL

struct user_msg {
	uint32_t interface_id;
	void *buf;
};

int npu_bbit_debugfs_init(void);
void npu_bbit_set_result(int result);
int npu_bbit_get_result(int *result);

#endif
