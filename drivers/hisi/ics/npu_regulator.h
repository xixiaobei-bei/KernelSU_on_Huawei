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

#ifndef __NPU_REGULATOR_H__
#define __NPU_REGULATOR_H__

#include <linux/platform_device.h>

/* call in probe, read config by dtsi */
int npu_regulator_init(struct device *dev);

/* call in remove */
void npu_regulator_exit(void);

/* regulator on */
int npu_regulator_on(unsigned int coreID);

/* regulator off */
void npu_regulator_off(unsigned int coreID);

#endif