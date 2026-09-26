/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: mag channel header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __MAG_CHANNEL_H__
#define __MAG_CHANNEL_H__

int send_mag_calibrate_data_to_mcu(void);
int send_mag1_calibrate_data_to_mcu(void);
int write_mag1sensor_calibrate_data_to_nv(const char *src);
int write_magsensor_calibrate_data_to_nv(const char *src, int length);
void send_mag_charger_to_mcu(void);
int send_current_to_mcu_mag(int current_value_now);
void mag_charge_notify_close(void);
void mag_charge_notify_open(void);
void reset_mag_calibrate_data(void);

#endif
