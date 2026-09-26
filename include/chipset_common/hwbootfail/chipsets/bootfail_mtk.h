/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: define platform-dependent bootfail interfaces
 * Author: wangsiwen
 * Create: 2020-09-20
 */

#ifndef BOOTFAIL_MTK_H
#define BOOTFAIL_MTK_H

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

/* ---- c++ support ---- */
#ifdef __cplusplus
}
#endif
#endif
