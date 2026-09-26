/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: define adapter for MTK
 * Author: yuanshuai
 * Create: 2020-10-26
 */

#ifndef ADAPTER_QCOM_H
#define ADAPTER_QCOM_H

#include <hwbootfail/core/adapter.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BFI_DEV_PATH "/dev/block/by-name/rrecord"
#define LONG_PRESS_DEV_PATH "/dev/block/by-name/bootfail_info"

/* 12MB -128 KB */
#define BFI_PART_SIZE 0x800000

/* ----local macroes ---- */
#define BL_LOG_NAME "fastboot_log"
#define KERNEL_LOG_NAME "last_kmsg"
#define BL_LOG_MAX_LEN 0x40000

#define BF_BOPD_RESERVE 32

int bf_rmem_init(void);
void *get_bf_mem_addr(void);
unsigned long get_bf_mem_size(void);
int qcom_adapter_init(struct adapter *padp);
void qcom_save_long_press_logs(void);

#ifdef __cplusplus
}
#endif
#endif

