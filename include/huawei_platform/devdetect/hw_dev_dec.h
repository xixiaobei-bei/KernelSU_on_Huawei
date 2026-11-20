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
	DEV_I2C_TOUCH_PANEL = 0,    /* 0 used in kernel layer */
	DEV_I2C_COMPASS,            /* 1 used in kernel layer */
	DEV_I2C_G_SENSOR,           /* 2 used in kernel layer */
	DEV_I2C_CAMERA_MAIN,        /* 3 used in kernel layer */
	DEV_I2C_CAMERA_SLAVE,       /* 4 used in kernel layer */
	DEV_I2C_KEYPAD,             /* 5 used in kernel layer */
	DEV_I2C_APS,                /* 6 used in kernel layer */
	DEV_I2C_GYROSCOPE,          /* 7 used in kernel layer */
	DEV_I2C_NFC,                /* 8 used in kernel layer */
	DEV_I2C_DC_DC,              /* 9 used in kernel layer */
	DEV_I2C_SPEAKER,            /* 10 used in kernel layer */
	DEV_I2C_OFN,                /* 11 used in kernel layer */
	DEV_I2C_TPS,                /* 12 used in kernel layer */
	DEV_I2C_L_SENSOR,           /* 13 used in kernel layer */
	DEV_I2C_CHARGER,            /* 14 used in kernel layer */
	DEV_I2C_BATTERY,            /* 15 used in kernel layer */
	DEV_I2C_NCT,                /* 16 used in kernel layer */
	DEV_I2C_MHL,                /* 17 used in kernel layer */
	DEV_I2C_AUDIENCE,           /* 18 used in kernel layer */
	DEV_I2C_IRDA,               /* 19 used in kernel layer */
	DEV_I2C_CS,                 /* 20 used in kernel layer */
	DEV_I2C_USB_SWITCH,         /* 21 used in kernel layer */
	DEV_I2C_PMU_DCDC,           /* 22 used in kernel layer */
	DEV_SPI_FPGA,               /* 23 used in kernel layer */
	DEV_I2C_CPU_CHIP,           /* 24 used in kernel layer */
	DEV_I2C_AIRPRESS,           /* 25 used in kernel layer */
	DEV_I2C_HANDPRESS,          /* 26 used in kernel layer */
	DEV_I2C_FFLASH,             /* 27 used in kernel layer */
	DEV_I2C_VIBRATOR_LRA,       /* 28 used in kernel layer */
	DEV_I2C_TYPEC,              /* 29 used in kernel layer */
	DEV_I2C_ANC_MAX14744,       /* 30 used in kernel layer */
	DEV_I2C_LASER,              /* 31 used in kernel layer */
	DEV_I2C_CAMERA_PMIC,        /* 32 used in kernel layer */
	DEV_I2C_LOADSWITCH,         /* 33 used in kernel layer */
	DEV_I2C_BUCKBOOST,          /* 34 used in kernel layer */
	DEV_I2C_COUL,               /* 35 used in kernel layer */
	DEV_I2C_DTV,                /* 36 used in kernel layer */
	DEV_I2C_AP_COLOR_SENSOR,    /* 37 used in kernel layer */
	DEV_I2C_SWITCHCAP,          /* 38 used in kernel layer */
	DEV_I2C_STEP_HALL,          /* 39 used in kernel layer */
	DEV_I2C_BUCK,               /* 40 used in kernel layer */
	DEV_I2C_ANTENNA_BOARD,      /* 41 used in kernel layer */
	DEV_I2C_BATTERY_STATE,      /* 42 used in kernel layer */
	DEV_I2C_PMIC,               /* 43 used in kernel layer */
	DEV_I2C_SC_AUX,             /* 44 used in kernel layer */
	DEV_I2C_MAX,                /* MAX 45 */
};

struct dev_detect_type {
	const unsigned int device_id;
	const char *device_name;
};

static const struct dev_detect_type dev_detect_table[] = {
	{ DEV_I2C_TOUCH_PANEL, "touch_panel" },      /* id : 0 in number 1 */
	{ DEV_I2C_COMPASS, "compass" },              /* id : 1 in number 2 */
	{ DEV_I2C_G_SENSOR, "g_sensor" },            /* id : 2 in number 3 */
	{ DEV_I2C_CAMERA_MAIN, "camera_main" },      /* id : 3 in number 4 */
	{ DEV_I2C_CAMERA_SLAVE, "camera_slave" },    /* id : 4 in number 5 */
	{ DEV_I2C_KEYPAD, "keypad" },                /* id : 5 in number 6 */
	{ DEV_I2C_APS, "aps" },                      /* id : 6 in number 7 */
	{ DEV_I2C_GYROSCOPE, "gyroscope" },          /* id : 7 in number 8 */
	{ DEV_I2C_NFC, "nfc" },                      /* id : 8 in number 9 */
	{ DEV_I2C_DC_DC, "dc_dc" },                  /* id : 9 in number 10 */
	{ DEV_I2C_SPEAKER, "speaker" },              /* id : 10 in number 11 */
	{ DEV_I2C_OFN, "ofn" },                      /* id : 11 in number 12 */
	{ DEV_I2C_TPS, "tps" },                      /* id : 12 in number 13 */
	{ DEV_I2C_L_SENSOR, "l_sensor" },            /* id : 13 in number 14 */
	{ DEV_I2C_CHARGER, "charge" },               /* id : 14 in number 15 */
	{ DEV_I2C_BATTERY, "battery" },              /* id : 15 in number 16 */
	{ DEV_I2C_NCT, "nct" },                      /* id : 16 in number 17 */
	{ DEV_I2C_MHL, "mhl" },                      /* id : 17 in number 18 */
	{ DEV_I2C_AUDIENCE, "audience" },            /* id : 18 in number 19 */
	{ DEV_I2C_IRDA, "irda" },                    /* id : 19 in number 20 */
	{ DEV_I2C_CS, "hand_sensor" },               /* id : 20 in number 21 */
	{ DEV_I2C_USB_SWITCH, "usb_switch" },        /* id : 21 in number 22 */
	{ DEV_I2C_PMU_DCDC, "pmu_dcdc" },            /* id : 22 in number 23 */
	{ DEV_SPI_FPGA, "antenna_ctl" },             /* id : 23 in number 24 */
	{ DEV_I2C_CPU_CHIP, "cpu_chip" },            /* id : 24 in number 25 */
	{ DEV_I2C_AIRPRESS, "airpress" },            /* id : 25 in number 26 */
	{ DEV_I2C_HANDPRESS, "handpress" },          /* id : 26 in number 27 */
	{ DEV_I2C_FFLASH, "fflash" },                /* id : 27 in number 28 */
	{ DEV_I2C_VIBRATOR_LRA, "vibrator_lra" },    /* id : 28 in number 29 */
	{ DEV_I2C_TYPEC, "typec" },                  /* id : 29 in number 30 */
	{ DEV_I2C_ANC_MAX14744, "anc_max14744" },    /* id : 30 in number 31 */
	{ DEV_I2C_LASER, "laser" },                  /* id : 31 in number 32 */
	{ DEV_I2C_CAMERA_PMIC, "camera_pmic" },      /* id : 32 in number 33 */
	{ DEV_I2C_LOADSWITCH, "charge_loadswitch" }, /* id : 33 in number 34 */
	{ DEV_I2C_BUCKBOOST, "buck_boost" },         /* id : 34 in number 35 */
	{ DEV_I2C_COUL, "coul" },                    /* id : 35 in number 36 */
	{ DEV_I2C_DTV, "dtv" },                      /* id : 36 in number 37 */
	{ DEV_I2C_AP_COLOR_SENSOR, "ap_color" },     /* id : 37 in number 38 */
	{ DEV_I2C_SWITCHCAP, "charge_switchcap" },   /* id : 38 in number 39 */
	{ DEV_I2C_STEP_HALL, "step_hall" },          /* id : 39 in number 40 */
	{ DEV_I2C_BUCK, "buck" },                    /* id : 40 in number 41 */
	{ DEV_I2C_ANTENNA_BOARD, "antenna_board" },  /* id : 41 in number 42 */
	{ DEV_I2C_BATTERY_STATE, "battery_state" },  /* id : 42 in number 43 */
	{ DEV_I2C_PMIC, "pmic" },                    /* id : 43 in number 44 */
	{ DEV_I2C_SC_AUX, "charge_sc_aux" },         /* id : 44 in number 45 */
};

/* External interface for setting the successful flag of device self-check */
int set_hw_dev_flag(int dev_id);

#endif
