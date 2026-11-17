/* Module internals
 *
 * Copyright (C) 2018 Hisilicon, Inc. All Rights Reserved.
 *
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of
 * Hisilicon Inc. and its licensors, and are licensed to the recipient
 * under the terms of a separate license agreement.  They may be
 * adapted and modified by bona fide purchasers under the terms of the
 * separate license agreement for internal use, but no adapted or
 * modified version may be disclosed or distributed to third parties
 * in any manner, medium, or form, in whole or in part, without the
 * prior written consent of Hisilicon Inc.
 */

#ifndef __NPU_CLOCK_H__
#define __NPU_CLOCK_H__

#include <linux/platform_device.h>

typedef enum {
    DEV_UNSET_PROFILE = 0,
    DEV_LOW_PROFILE,
    DEV_NORMAL_PROFILE,
    DEV_HIGH_PROFILE,
} dev_perf;

/* read config by dtsi */
int npu_clock_init(struct device *dev);

void npu_clock_exit(void);

/* clock start, when a frame begin to calculate */
int npu_clock_start(unsigned int coreID);

/* clock off, when npu power-down */
void npu_clock_stop(unsigned int coreID);

/* get vcodecbus rate */
unsigned int npu_clock_get_vcodecbus_rate(void);
unsigned int npu_clock_get_npu_rate(unsigned int coreID);

/* set new clock profile, it will realize in the next frame */
long npu_clock_set_profile(unsigned int coreID, unsigned long profile);

/* set default clock profile */
void npu_clock_default_profile(void);

#endif
