/*
 * hw_dev_array.h
 *
 * Device detect array header file.
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef KERNEL_INCLUDE_DEV_ARRAY_H
#define KERNEL_INCLUDE_DEV_ARRAY_H
#include "hw_dev_dec.h"

struct hw_dec_struct {
	int devices_id;
	char *devices_name;
};

static struct hw_dec_struct hw_dec_device_array[] = {
	{ DEV_I2C_TOUCH_PANEL, "touch_panel" },      /* id : 0 order 1 */
	{ DEV_I2C_COMPASS, "compass" },              /* id : 1 order 2 */
	{ DEV_I2C_G_SENSOR, "g_sensor" },            /* id : 2 order 3 */
	{ DEV_I2C_CAMERA_MAIN, "camera_main" },      /* id : 3 order 4 */
	{ DEV_I2C_CAMERA_SLAVE, "camera_slave" },    /* id : 4 order 5 */
	{ DEV_I2C_KEYPAD, "keypad" },                /* id : 5 order 6 */
	{ DEV_I2C_APS, "aps" },                      /* id : 6 order 7 */
	{ DEV_I2C_GYROSCOPE, "gyroscope" },          /* id : 7 order 8 */
	{ DEV_I2C_NFC, "nfc" },                      /* id : 8 order 9 */
	{ DEV_I2C_DC_DC, "dc_dc" },                  /* id : 9 order 10 */
	{ DEV_I2C_SPEAKER, "speaker" },              /* id : 10 order 11 */
	{ DEV_I2C_OFN, "ofn" },                      /* id : 11 order 12 */
	{ DEV_I2C_TPS, "tps" },                      /* id : 12 order 13 */
	{ DEV_I2C_L_SENSOR, "l_sensor" },            /* id : 13 order 14 */
	{ DEV_I2C_CHARGER, "charge" },               /* id : 14 order 15 */
	{ DEV_I2C_BATTERY, "battery" },              /* id : 15 order 16 */
	{ DEV_I2C_NCT, "nct" },                      /* id : 16 order 17 */
	{ DEV_I2C_MHL, "mhl" },                      /* id : 17 order 18 */
	{ DEV_I2C_AUDIENCE, "audience" },            /* id : 18 order 19 */
	{ DEV_I2C_IRDA, "irda" },                    /* id : 19 order 20 */
	{ DEV_I2C_CS, "hand_sensor" },               /* id : 20 order 21 */
	{ DEV_I2C_USB_SWITCH, "usb_switch" },        /* id : 21 order 22 */
	{ DEV_I2C_PMU_DCDC, "pmu_dcdc" },            /* id : 22 order 23 */
	{ DEV_SPI_FPGA, "antenna_ctl" },             /* id : 23 order 24 */
	{ DEV_I2C_CPU_CHIP, "cpu_chip" },            /* id : 24 order 25 */
	{ DEV_I2C_AIRPRESS, "airpress" },            /* id : 25 order 26 */
	{ DEV_I2C_HANDPRESS, "handpress" },          /* id : 26 order 27 */
	{ DEV_I2C_FFLASH, "fflash" },                /* id : 27 order 28 */
	{ DEV_I2C_VIBRATOR_LRA, "vibrator_lra" },    /* id : 28 order 29 */
	{ DEV_I2C_TYPEC, "typec" },                  /* id : 29 order 30 */
	{ DEV_I2C_ANC_MAX14744, "anc_max14744" },    /* id : 30 order 31 */
	{ DEV_I2C_LASER, "laser" },                  /* id : 31 order 32 */
	{ DEV_I2C_CAMERA_PMIC, "camera_pmic" },      /* id : 32 order 33 */
	{ DEV_I2C_LOADSWITCH, "charge_loadswitch" }, /* id : 33 order 34 */
	{ DEV_I2C_BUCKBOOST, "buck_boost" },         /* id : 34 order 35 */
	{ DEV_I2C_COUL, "coul" },                    /* id : 35 order 36 */
	{ DEV_I2C_DTV, "dtv" },                      /* id : 36 order 37 */
	{ DEV_I2C_AP_COLOR_SENSOR, "ap_color" },     /* id : 37 order 38 */
	{ DEV_I2C_SWITCHCAP, "charge_switchcap" },   /* id : 38 order 39 */
	{ DEV_I2C_STEP_HALL,"step_hall" },           /* id : 39 order 40 */
	{ DEV_I2C_BUCK,"buck" },                     /* id : 40 order 41 */
	{ DEV_I2C_MAX, "NULL" },                     /* id : 41 order 42 */
	{ DEV_CONNECTIVITY_WIFI, "wifi" },           /* id : 42 order 43 */
	{ DEV_CONNECTIVITY_BT, "bt" },               /* id : 43 order 44 */
	{ DEV_CONNECTIVITY_FM, "fm" },               /* id : 44 order 45 */
	{ DEV_CONNECTIVITY_GPS, "gps" },             /* id : 45 order 46 */
	{ DEV_CONNECTIVITY_GPU, "gpu" },             /* id : 46 order 47 */
};

#endif
