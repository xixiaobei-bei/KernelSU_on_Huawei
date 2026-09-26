/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: vibrator channel header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef _VIBRATOR_CHANNEL_H_
#define _VIBRATOR_CHANNEL_H_

#include <linux/types.h>

int write_vibrator_calib_value_to_nv(const char *temp, uint16_t length);

#endif /* _VIBRATOR_CHANNEL_H_ */
