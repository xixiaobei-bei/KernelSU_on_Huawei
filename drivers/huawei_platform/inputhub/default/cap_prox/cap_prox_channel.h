/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: cap prox channel header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __CAP_PROX_CHANNEL_H__
#define __CAP_PROX_CHANNEL_H__

#include "cap_prox_detect.h"

int send_cap_prox1_calibrate_data_to_mcu(void);
int send_cap_prox_calibrate_data_to_mcu(void);
void reset_cap_prox_calibrate_data(void);
void reset_cap_prox1_calibrate_data(void);

#endif
