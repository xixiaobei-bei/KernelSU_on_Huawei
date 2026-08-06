/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: mag channel header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __MAG_CHANNEL_H__
#define __MAG_CHANNEL_H__

enum CHARGE_TYPE {
	CHARGE_TYPE_BEGIN,
	CHARGE_TYPE_9V_2A = CHARGE_TYPE_BEGIN,
	CHARGE_TYPE_10V_2P25A,
	CHARGE_TYPE_5V_4P5A,
	CHARGE_TYPE_5V_2A,
	CHARGE_TYPE_UNKNOWN,
};



void send_mag_charger_to_mcu(void);
int send_current_to_mcu_mag(int current_value_now);
void mag_charge_notify_close(void);
void mag_charge_notify_open(void);
void reset_mag_calibrate_data(void);
int write_magsensor_calibrate_data_to_nv(const char *src);
int send_mag_calibrate_data_to_mcu(void);
#endif
