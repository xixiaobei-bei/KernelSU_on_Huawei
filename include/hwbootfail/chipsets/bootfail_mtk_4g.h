/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: specific MTK-4G functions for BootDetector.
 * Author: Gong Chen <gongchen4@huawei.com>
 * Create: 2020-11-22
 */
#ifndef __CONFIG_BOOTFAIL_MTK_4G_H__
#define __CONFIG_BOOTFAIL_MTK_4G_H__

#include <linux/types.h>

#define BOOT_PHASE_PL             0x01
#define BOOT_PHASE_LK             0x02
#define BOOT_PHASE_KERNEL         0x03
#define BOOT_PHASE_ANDROID        0x04
#define BOOT_PHASE_PL_COLD_REBOOT 0X05
#define BOOT_PHASE_SUSPEND        0x06
#define BOOT_PHASE_RESUME         0X07
#define BOOT_PHASE_NATIVE         0x08
#define BOOT_PHASE_FRAMEWORK      0x09

#define ROFA_MEM_SIZE   32

struct boot_context {
	u32 boot_phase;
};

void init_boot_context(void);

void set_boot_phase(u32 phase);
u32 get_boot_phase(void);

void check_pwrkey_long_press(unsigned long pressed);
#endif
