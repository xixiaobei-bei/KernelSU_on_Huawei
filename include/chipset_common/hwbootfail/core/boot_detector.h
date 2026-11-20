/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: define the global interface for Boot Detector
 * Author: qidechun
 * Create: 2019-03-04
 */

#ifndef BOOT_DETECTOR_H
#define BOOT_DETECTOR_H

/*----includes ----*/
#include "boot_recovery.h"
#include "boot_interface.h"
#include "adapter.h"

/*----c++ support ----*/
#ifdef __cplusplus
extern "C" {
#endif

/*---- export prototypes ----*/
enum post_action {
	PA_DO_NOTHING,
	PA_REBOOT,
	PA_SHUTDOWN,
};

/*---- export macroes ----*/
#define BF_INJECT_MAGIC_NUM (0x568F6973)
#define BF_SW_MAGIC_NUM (0x73698f56)
#define BF_HW_MAGIC_NUM (0x48574246) /* HWBF */

/*---- export function prototypes ----*/
enum bootfail_errorcode get_boot_stage(enum boot_stage *pstage);
enum bootfail_errorcode set_boot_stage(enum boot_stage stage);
enum bootfail_errorcode boot_fail_error(struct bootfail_proc_param *pparam);
enum bootfail_errorcode boot_detector_init(struct adapter *padp);

/*---- c++ support ----*/
#ifdef __cplusplus
}
#endif
#endif
