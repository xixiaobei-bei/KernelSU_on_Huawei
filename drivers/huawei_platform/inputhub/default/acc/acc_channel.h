/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: acc channel header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ACC_CHANNEL_H__
#define __ACC_CHANNEL_H__

int write_gsensor1_offset_to_nv(const char *temp, int length);
int write_gsensor_offset_to_nv(const char *temp, int length);
int send_gsensor1_calibrate_data_to_mcu(void);
int send_gsensor_calibrate_data_to_mcu(void);
void reset_acc_calibrate_data(void);
void reset_acc1_calibrate_data(void);

#endif
