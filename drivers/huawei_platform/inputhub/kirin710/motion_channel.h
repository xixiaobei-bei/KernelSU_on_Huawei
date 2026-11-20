/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: motion channel header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __MOTION_CHANNEL_H__
#define __MOTION_CHANNEL_H__

#include <linux/types.h>

void enable_motions_when_recovery_iom3(void);
void disable_motions_when_sysreboot(void);

#endif
