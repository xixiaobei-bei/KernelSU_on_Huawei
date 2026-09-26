/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: gyro channel header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __GYRO_CHANNEL_H__
#define __GYRO_CHANNEL_H__

int send_gyro1_calibrate_data_to_mcu(void);
int send_gyro_calibrate_data_to_mcu(void);
int send_gyro_temperature_offset_to_mcu(void);
int write_gyro1_sensor_offset_to_nv(const char *temp, int length);
int write_gyro_sensor_offset_to_nv(const char *temp, int length);
int write_gyro_temperature_offset_to_nv(const char *temp, int length);
void reset_gyro_calibrate_data(void);
void reset_gyro1_calibrate_data(void);

#endif
