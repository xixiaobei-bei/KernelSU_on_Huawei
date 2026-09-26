/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: this file provide sys  interface to set handle  battery state such as
 *          capacity, voltage, current, temperature
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _COUL_DRV_TEST_H
#define _COUL_DRV_TEST_H

#define INVALID_TEMP    (-99)
#define TEMP_MIN        (-40)
#define TEMP_MAX        80
#define CAPACITY_MAX    100
#define VOLTAGE_MIN     2000
#define VOLTAGE_MAX     5000
#define CURRENT_MIN     (-10000)
#define CURRENT_MAX     10000
#define BATT_FCC_MAX    10000
#define INVALID_INPUT_EVENT     10000
#define HEX     16
#define DEC     10
#define SRC_LEN 11
#define DEFAULT_TEST_START_FLAG         0
#define DEFAULT_INPUT_BATT_CAPACITY     50
#define DEFAULT_INPUT_BATT_CURRENT      500
#define DEFAULT_INPUT_BATT_EXIST        1
#define DEFAULT_INPUT_BATT_FCC          2000
#define DEFAULT_INPUT_BATT_FULL         0
#define DEFAULT_INPUT_BATT_TEMP         25
#define DEFAULT_INPUT_BATT_VOLTAGE      3800

struct coul_drv_test_info {
	int input_batt_exist;
	int input_batt_capacity;
	int input_batt_temp;
	int input_batt_full;
	int input_batt_volt;
	int input_batt_cur;
	int input_batt_fcc;
	int input_event;
	unsigned int test_start_flag;
};

struct coul_drv_test_info *get_coul_drv_test_info(void);

#endif
