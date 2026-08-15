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

#ifndef __ICS_POWER_H__
#define __ICS_POWER_H__

#include <linux/platform_device.h>
#include <linux/types.h>

int npu_power_init(struct device *dev);
void npu_power_exit(void);
int npu_power_on(unsigned int coreID);
void npu_power_off(unsigned int coreID);
bool npu_power_status(unsigned int coreID);

long npu_set_profile(unsigned int coreID, unsigned long profile);
int npu_update_profile(unsigned int coreID);
void npu_set_default_profile(void);
unsigned int npu_get_npu_rate(unsigned int coreID);
unsigned int npu_get_vcodecbus_rate(void);

#endif

