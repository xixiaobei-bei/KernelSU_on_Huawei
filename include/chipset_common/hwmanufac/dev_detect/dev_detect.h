/*
 * hw_dev_dec.h
 *
 * define enumerated variables for useing in device self-checking schemes
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#ifndef KERNEL_INCLUDE_DEVICE_DETECT_H
#define KERNEL_INCLUDE_DEVICE_DETECT_H

/* add new devices before DEV_I2C_MAX */
enum dev_check_type {               /* id */
	DEV_I2C_TOUCH_PANEL = 0,    /* 00 */
	DEV_I2C_COMPASS,            /* 01 */
	DEV_I2C_G_SENSOR,           /* 02 */
	DEV_I2C_CAMERA_MAIN,        /* 03 */
	DEV_I2C_CAMERA_SLAVE,       /* 04 */
	DEV_I2C_KEYPAD,             /* 05 */
	DEV_I2C_APS,                /* 06 */
	DEV_I2C_GYROSCOPE,          /* 07 */
	DEV_I2C_NFC,                /* 08 */
	DEV_I2C_DC_DC,              /* 09 */
	DEV_I2C_SPEAKER,            /* 10 */
	DEV_I2C_OFN,                /* 11 */
	DEV_I2C_TPS,                /* 12 */
	DEV_I2C_L_SENSOR,           /* 13 */
	DEV_I2C_CHARGER,            /* 14 */
	DEV_I2C_BATTERY,            /* 15 */
	DEV_I2C_NCT,                /* 16 */
	DEV_I2C_MHL,                /* 17 */
	DEV_I2C_AUDIENCE,           /* 18 */
	DEV_I2C_IRDA,               /* 19 */
	DEV_I2C_CS,                 /* 20 */
	DEV_I2C_USB_SWITCH,         /* 21 */
	DEV_I2C_PMU_DCDC,           /* 22 */
	DEV_SPI_FPGA,               /* 23 */
	DEV_I2C_CPU_CHIP,           /* 24 */
	DEV_I2C_AIRPRESS,           /* 25 */
	DEV_I2C_HANDPRESS,          /* 26 */
	DEV_I2C_FFLASH,             /* 27 */
	DEV_I2C_VIBRATOR_LRA,       /* 28 */
	DEV_I2C_TYPEC,              /* 29 */
	DEV_I2C_ANC_MAX14744,       /* 30 */
	DEV_I2C_LASER,              /* 31 */
	DEV_I2C_CAMERA_PMIC,        /* 32 */
	DEV_I2C_LOADSWITCH,         /* 33 */
	DEV_I2C_BUCKBOOST,          /* 34 */
	DEV_I2C_COUL,               /* 35 */
	DEV_I2C_DTV,                /* 36 */
	DEV_I2C_AP_COLOR_SENSOR,    /* 37 */
	DEV_I2C_SWITCHCAP,          /* 38 */
	DEV_I2C_STEP_HALL,          /* 39 */
	DEV_I2C_BUCK,               /* 40 */
	DEV_I2C_ANTENNA_BOARD,      /* 41 */
	DEV_I2C_BATTERY_STATE,      /* 42 */
	DEV_I2C_PMIC,               /* 43 */
	DEV_I2C_SC_AUX,             /* 44 */
	DEV_I2C_MAX,                /* MAX */
};

struct dev_detect_type {
	const unsigned int device_id;
	const char *device_name;
};

static const struct dev_detect_type dev_detect_table[] = {
	{ DEV_I2C_TOUCH_PANEL, "touch_panel" },      /* id : 00 order number 01 */
	{ DEV_I2C_COMPASS, "compass" },              /* id : 01 order number 02 */
	{ DEV_I2C_G_SENSOR, "g_sensor" },            /* id : 02 order number 03 */
	{ DEV_I2C_CAMERA_MAIN, "camera_main" },      /* id : 03 order number 04 */
	{ DEV_I2C_CAMERA_SLAVE, "camera_slave" },    /* id : 04 order number 05 */
	{ DEV_I2C_KEYPAD, "keypad" },                /* id : 05 order number 06 */
	{ DEV_I2C_APS, "aps" },                      /* id : 06 order number 07 */
	{ DEV_I2C_GYROSCOPE, "gyroscope" },          /* id : 07 order number 08 */
	{ DEV_I2C_NFC, "nfc" },                      /* id : 08 order number 09 */
	{ DEV_I2C_DC_DC, "dc_dc" },                  /* id : 09 order number 10 */
	{ DEV_I2C_SPEAKER, "speaker" },              /* id : 10 order number 11 */
	{ DEV_I2C_OFN, "ofn" },                      /* id : 11 order number 12 */
	{ DEV_I2C_TPS, "tps" },                      /* id : 12 order number 13 */
	{ DEV_I2C_L_SENSOR, "l_sensor" },            /* id : 13 order number 14 */
	{ DEV_I2C_CHARGER, "charge" },               /* id : 14 order number 15 */
	{ DEV_I2C_BATTERY, "battery" },              /* id : 15 order number 16 */
	{ DEV_I2C_NCT, "nct" },                      /* id : 16 order number 17 */
	{ DEV_I2C_MHL, "mhl" },                      /* id : 17 order number 18 */
	{ DEV_I2C_AUDIENCE, "audience" },            /* id : 18 order number 19 */
	{ DEV_I2C_IRDA, "irda" },                    /* id : 19 order number 20 */
	{ DEV_I2C_CS, "hand_sensor" },               /* id : 20 order number 21 */
	{ DEV_I2C_USB_SWITCH, "usb_switch" },        /* id : 21 order number 22 */
	{ DEV_I2C_PMU_DCDC, "pmu_dcdc" },            /* id : 22 order number 23 */
	{ DEV_SPI_FPGA, "antenna_ctl" },             /* id : 23 order number 24 */
	{ DEV_I2C_CPU_CHIP, "cpu_chip" },            /* id : 24 order number 25 */
	{ DEV_I2C_AIRPRESS, "airpress" },            /* id : 25 order number 26 */
	{ DEV_I2C_HANDPRESS, "handpress" },          /* id : 26 order number 27 */
	{ DEV_I2C_FFLASH, "fflash" },                /* id : 27 order number 28 */
	{ DEV_I2C_VIBRATOR_LRA, "vibrator_lra" },    /* id : 28 order number 29 */
	{ DEV_I2C_TYPEC, "typec" },                  /* id : 29 order number 30 */
	{ DEV_I2C_ANC_MAX14744, "anc_max14744" },    /* id : 30 order number 31 */
	{ DEV_I2C_LASER, "laser" },                  /* id : 31 order number 32 */
	{ DEV_I2C_CAMERA_PMIC, "camera_pmic" },      /* id : 32 order number 33 */
	{ DEV_I2C_LOADSWITCH, "charge_loadswitch" }, /* id : 33 order number 34 */
	{ DEV_I2C_BUCKBOOST, "buck_boost" },         /* id : 34 order number 35 */
	{ DEV_I2C_COUL, "coul" },                    /* id : 35 order number 36 */
	{ DEV_I2C_DTV, "dtv" },                      /* id : 36 order number 37 */
	{ DEV_I2C_AP_COLOR_SENSOR, "ap_color" },     /* id : 37 order number 38 */
	{ DEV_I2C_SWITCHCAP, "charge_switchcap" },   /* id : 38 order number 39 */
	{ DEV_I2C_STEP_HALL, "step_hall" },          /* id : 39 order number 40 */
	{ DEV_I2C_BUCK, "buck" },                    /* id : 40 order number 41 */
	{ DEV_I2C_ANTENNA_BOARD, "antenna_board" },  /* id : 41 order number 42 */
	{ DEV_I2C_BATTERY_STATE, "battery_state" },  /* id : 42 order number 43 */
	{ DEV_I2C_PMIC, "pmic" },                    /* id : 43 order number 44 */
	{ DEV_I2C_SC_AUX, "charge_sc_aux" },         /* id : 44 order number 45 */
};

/* External interface for setting the successful flag of device self-check */
int set_hw_dev_flag(int dev_id);

#endif
