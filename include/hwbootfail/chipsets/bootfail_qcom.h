/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: define platform-dependent bootfail interfaces
 * Author: yuanshuai
 * Create: 2020-10-26
 */

#ifndef BOOTFAIL_QCOM_H
#define BOOTFAIL_QCOM_H

#include <linux/vmalloc.h>
#include <linux/types.h>
#include <linux/semaphore.h>

/* ---- c++ support ---- */
#ifdef __cplusplus
extern "C" {
#endif

/* ---- export macroes ---- */

/*---- export function prototypes ----*/
void set_valid_long_press_flag(void);
void save_long_press_info(void);
void bootfail_pwk_press(void);
void bootfail_pwk_release(void);

/* ---- c++ support ---- */
#ifdef __cplusplus
}
#endif
#endif
