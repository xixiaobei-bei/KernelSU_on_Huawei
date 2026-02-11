/*
 * drv2605.h
 *
 * drv2605 driver
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
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


#ifndef _DRV2605_H
#define _DRV2605_H

#include <linux/module.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/leds.h>
#include <linux/switch.h>

#define DEVICE_NAME                 "drv2605"
#define DRIVER_VERSION              "130"
#define CDEVIE_NAME                 "haptics"
#define DEFAULT_NAME                "vibrator"


/* Commands */
#define HAPTIC_CMDID_PLAY_SINGLE_EFFECT     0x01
#define HAPTIC_CMDID_PLAY_EFFECT_SEQUENCE   0x02
#define HAPTIC_CMDID_PLAY_TIMED_EFFECT      0x03
#define HAPTIC_CMDID_GET_DEV_ID             0x04
#define HAPTIC_CMDID_RUN_DIAG               0x05
#define HAPTIC_CMDID_STOP                   0xFF

/* Command size */
#define HAPTIC_CMDSZ_SINGLE_EFFECT     2
#define HAPTIC_CMDSZ_EFFECT_SEQUENCE   9
#define HAPTIC_CMDSZ_TIMED_EFFECT      3
#define HAPTIC_CMDSZ_STOP              1

#define GO_REG 0x0C
#define GO     0x01
#define STOP   0x00

/* Status */
#define STATUS_REG          0x00
#define STATUS_DEFAULT      0x00

#define DIAG_RESULT_MASK    (1 << 3)
#define AUTO_CAL_PASSED     (0 << 3)
#define AUTO_CAL_FAILED     (1 << 3)
#define DIAG_GOOD           (0 << 3)
#define DIAG_BAD            (1 << 3)

#define DEV_ID_MASK (7 << 5)
#define DRV2605 (5 << 5)
#define DRV2604 (4 << 5)

/* Mode */
#define MODE_REG                    0x01
#define MODE_DEFAULT                0x40
#define MODE_REG_VALUE              0x07

#define MODE_STANDBY                0x40
#define MODE_DEVICE_READY           0x00

#define DRV260X_MODE_MASK           0x07
#define MODE_INTERNAL_TRIGGER       0
#define MODE_EXTERNAL_TRIGGER_EDGE  1
#define MODE_EXTERNAL_TRIGGER_LEVEL 2
#define MODE_PWM_OR_ANALOG_INPUT    3
#define MODE_RESERVED               4
#define MODE_REAL_TIME_PLAYBACK     5
#define MODE_DIAGNOSTICS            6
#define AUTO_CALIBRATION            7

#define MODE_STANDBY_MASK           0x40
#define MODE_READY                  1
#define MODE_SOFT_STANDBY           0
#define MODE_RESET                  0x80

/* Real Time Playback */
#define REAL_TIME_PLAYBACK_REG      0x02

/* Library Selection */
#define LIBRARY_SELECTION_REG       0x03
#define LIBRARY_SELECTION_DEFAULT   0x00

#define LIBRARY_A 0x01
#define LIBRARY_B 0x02
#define LIBRARY_C 0x03
#define LIBRARY_D 0x04
#define LIBRARY_E 0x05
#define LIBRARY_F 0x06

#define LIBRARY_SELECTION_MASK              0x07
#define LIBRARY_SELECTION_LIBRARY_RAM       0
#define LIBRARY_SELECTION_LIBRARY_OVERDRIVE 1
#define LIBRARY_SELECTION_LIBRARY_40_60     2
#define LIBRARY_SELECTION_LIBRARY_60_80     3
#define LIBRARY_SELECTION_LIBRARY_100_140   4
#define LIBRARY_SELECTION_LIBRARY_140_PLUS  5

#define LIBRARY_SELECTION_HIZ_MASK          0x10
#define LIBRARY_SELECTION_HIZ_EN            1
#define LIBRARY_SELECTION_HIZ_DIS           0

/* Waveform Sequencer */
#define WAVEFORM_SEQUENCER_REG      0x04
#define WAVEFORM_SEQUENCER_REG2     0x05
#define WAVEFORM_SEQUENCER_REG3     0x06
#define WAVEFORM_SEQUENCER_REG4     0x07
#define WAVEFORM_SEQUENCER_REG5     0x08
#define WAVEFORM_SEQUENCER_REG6     0x09
#define WAVEFORM_SEQUENCER_REG7     0x0A
#define WAVEFORM_SEQUENCER_REG8     0x0B
#define WAVEFORM_SEQUENCER_MAX      8
#define WAVEFORM_SEQUENCER_DEFAULT  0x00

/* OverDrive Time Offset */
#define OVERDRIVE_TIME_OFFSET_REG   0x0D

/* Sustain Time Offset, postive */
#define SUSTAIN_TIME_OFFSET_POS_REG 0x0E

/* Sustain Time Offset, negative */
#define SUSTAIN_TIME_OFFSET_NEG_REG 0x0F

/* Brake Time Offset */
#define BRAKE_TIME_OFFSET_REG       0x10

/* Audio to Haptics Control */
#define AUDIO_HAPTICS_CONTROL_REG   0x11
#define AUDIO_HAPTICS_RECT_10MS     (0 << 2)
#define AUDIO_HAPTICS_RECT_20MS     (1 << 2)
#define AUDIO_HAPTICS_RECT_30MS     (2 << 2)
#define AUDIO_HAPTICS_RECT_40MS     (3 << 2)
#define AUDIO_HAPTICS_FILTER_100HZ  0
#define AUDIO_HAPTICS_FILTER_125HZ  1
#define AUDIO_HAPTICS_FILTER_150HZ  2
#define AUDIO_HAPTICS_FILTER_200HZ  3

/* Audio to Haptics Minimum Input Leve */
#define AUDIO_HAPTICS_MIN_INPUT_REG   0x12

/* Audio to Haptics Maximum Input Level */
#define AUDIO_HAPTICS_MAX_INPUT_REG   0x13

/* Audio to Haptics Minimum Output Drive */
#define AUDIO_HAPTICS_MIN_OUTPUT_REG  0x14

/* Audio to Haptics Maximum Output Drive */
#define AUDIO_HAPTICS_MAX_OUTPUT_REG  0x15

/* Rated Voltage */
#define RATED_VOLTAGE_REG             0x16

/* Overdrive Clamp Voltage */
#define OVERDRIVE_CLAMP_VOLTAGE_REG   0x17

/* Auto Calibrationi Compensation Result */
#define AUTO_CALI_RESULT_REG          0x18

/* Auto Calibration Back-EMF Result */
#define AUTO_CALI_BACK_EMF_RESULT_REG 0x19

/* Feedback Control */
#define FEEDBACK_CONTROL_REG          0x1A

#define AUTO_CALI_RESULT_REG_VALUE    0x0D
#define EMF_RESULT_REG_VALUE          0x6D

#define FEEDBACK_CONTROL_BEMF_ERM_GAIN0 0
#define FEEDBACK_CONTROL_BEMF_ERM_GAIN1 1
#define FEEDBACK_CONTROL_BEMF_ERM_GAIN2 2
#define FEEDBACK_CONTROL_BEMF_ERM_GAIN3 3

#define FEEDBACK_CONTROL_BEMF_LRA_GAIN0 0
#define FEEDBACK_CONTROL_BEMF_LRA_GAIN1 1
#define FEEDBACK_CONTROL_BEMF_LRA_GAIN2 2
#define FEEDBACK_CONTROL_BEMF_LRA_GAIN3 3

#define LOOP_RESPONSE_SLOW              (0 << 2)
#define LOOP_RESPONSE_MEDIUM            (1 << 2)
#define LOOP_RESPONSE_FAST              (2 << 2)
#define LOOP_RESPONSE_VERY_FAST         (3 << 2)

#define FB_BRAKE_FACTOR_1X              (0 << 4)
#define FB_BRAKE_FACTOR_2X              (1 << 4)
#define FB_BRAKE_FACTOR_3X              (2 << 4)
#define FB_BRAKE_FACTOR_4X              (3 << 4)
#define FB_BRAKE_FACTOR_6X              (4 << 4)
#define FB_BRAKE_FACTOR_8X              (5 << 4)
#define FB_BRAKE_FACTOR_16X             (6 << 4)
#define FB_BRAKE_DISABLED               (7 << 4)
#define FEEDBACK_CONTROL_MODE_ERM       0
#define FEEDBACK_CONTROL_MODE_LRA       (1 << 7)

/* Control1 */
#define CONTROL1_REG            0x1B

#define STARTUP_BOOST_ENABLED   (1 << 7)
#define STARTUP_BOOST_DISABLED  (0 << 7)

/* Control2 */
#define CONTROL2_REG            0x1C

#define IDISS_TIME_MASK         0x03
#define IDISS_TIME_VERY_SHORT   0
#define IDISS_TIME_SHORT        1
#define IDISS_TIME_MEDIUM       2
#define IDISS_TIME_LONG         3

#define BLANKING_TIME_MASK         0x0C
#define BLANKING_TIME_VERY_SHORT   (0 << 2)
#define BLANKING_TIME_SHORT        (1 << 2)
#define BLANKING_TIME_MEDIUM       (2 << 2)
#define BLANKING_TIME_VERY_LONG    (3 << 2)

#define AUTO_RES_GAIN_MASK         0x30
#define AUTO_RES_GAIN_VERY_LOW     (0 << 4)
#define AUTO_RES_GAIN_LOW          (1 << 4)
#define AUTO_RES_GAIN_MEDIUM       (2 << 4)
#define AUTO_RES_GAIN_HIGH         (3 << 4)

#define SOFT_BRAKE_MASK            0x40

#define BIDIR_INPUT_MASK           0x80
#define UNIDIRECT_INPUT            (0 << 7)
#define BIDIRECT_INPUT             (1 << 7)

/* Control3 */
#define CONTROL3_REG 0x1D

#define ERM_OPENLOOP_ENABLED (1 << 5)
#define NG_THRESH_1 (1 << 6)
#define NG_THRESH_2 (2 << 6)
#define NG_THRESH_3 (3 << 6)

/* Auto Calibration Memory Interface */
#define AUTOCAL_MEM_INTERFACE_REG           0x1E
#define AUTOCAL_TIME_150MS                  (0 << 4)
#define AUTOCAL_TIME_250MS                  (1 << 4)
#define AUTOCAL_TIME_500MS                  (2 << 4)
#define AUTOCAL_TIME_1000MS                 (3 << 4)
#define SILICON_REVISION_REG                0x3B
#define SILICON_REVISION_MASK               0x07
#define AUDIO_HAPTICS_MIN_INPUT_VOLTAGE     0x19
#define AUDIO_HAPTICS_MAX_INPUT_VOLTAGE     0x64
#define AUDIO_HAPTICS_MIN_OUTPUT_VOLTAGE    0x19
#define AUDIO_HAPTICS_MAX_OUTPUT_VOLTAGE    0xFF
#define DEFAULT_ERM_AUTOCAL_COMPENSATION    0x14
#define DEFAULT_ERM_AUTOCAL_BACKEMF         0x72
#define DEFAULT_LRA_AUTOCAL_COMPENSATION    0x06
#define DEFAULT_LRA_AUTOCAL_BACKEMF         0xDE
#define DEFAULT_DRIVE_TIME                  0x17
#define HIGH_VOL_ERR                        1

/* sw_dev state */
#define SW_STATE_IDLE                       0x00
#define SW_STATE_AUDIO2HAPTIC               0x01
#define SW_STATE_SEQUENCE_PLAYBACK          0x02
#define SW_STATE_RTP_PLAYBACK               0x04

/* vibrator calibrate */
#define VIB_CALIDATA_NV_NUM                 337
#define VIB_CALIDATA_NV_SIZE                3
#define NV_READ_TAG                         1
#define NV_WRITE_TAG                        0
#define MAX_BUF_LEGTH                       16
#define MAX_HAP_BUF_SIZE                    100
#define MIN_HAP_BUF_SIZE                    2
#define CALI_DATA_SIZE                      3

/* GPIO */
#define GPIO_LEVEL_LOW                      0
#define GPIO_LEVEL_HIGH                     1

/* DRV2605 built-in effect bank/library */
#define EFFECT_LIBRARY                      LIBRARY_F

#define MAX_TIME_DIFF                       50

/*
 * Rated Voltage:
 * Calculated using the formula r = v * 255 / 5.6
 * where r is what will be written to the register
 * and v is the rated voltage of the actuator

 * Overdrive Clamp Voltage:
 * Calculated using the formula o = oc * 255 / 5.6
 * where o is what will be written to the register
 * and oc is the overdrive clamp voltage of the actuator
 */

#if (EFFECT_LIBRARY == LIBRARY_A)
#define ERM_RATED_VOLTAGE               0x3E
#define ERM_OVERDRIVE_CLAMP_VOLTAGE     0x90

#elif (EFFECT_LIBRARY == LIBRARY_B)
#define ERM_RATED_VOLTAGE               0x90
#define ERM_OVERDRIVE_CLAMP_VOLTAGE     0x90

#elif (EFFECT_LIBRARY == LIBRARY_C)
#define ERM_RATED_VOLTAGE               0x90
#define ERM_OVERDRIVE_CLAMP_VOLTAGE     0x90

#elif (EFFECT_LIBRARY == LIBRARY_D)
#define ERM_RATED_VOLTAGE               0x90
#define ERM_OVERDRIVE_CLAMP_VOLTAGE     0x90

#elif (EFFECT_LIBRARY == LIBRARY_E)
#define ERM_RATED_VOLTAGE               0x90
#define ERM_OVERDRIVE_CLAMP_VOLTAGE     0x90

#else
#define ERM_RATED_VOLTAGE               0x90
#define ERM_OVERDRIVE_CLAMP_VOLTAGE     0x90
#endif

#define LRA_RATED_VOLTAGE               0x34
#define LRA_OVERDRIVE_CLAMP_VOLTAGE     0x76
#define SKIP_LRA_AUTOCAL                1
#define GO_BIT_POLL_INTERVAL            15

#if EFFECT_LIBRARY == LIBRARY_A
#define REAL_TIME_PLAYBACK_STRENGTH     0x38
#elif EFFECT_LIBRARY == LIBRARY_F
#define REAL_TIME_PLAYBACK_STRENGTH     0x66
#else
#define REAL_TIME_PLAYBACK_STRENGTH     0x7F
#endif

/* time */
#define MAX_TIMEOUT                     10000
#define MIN_REDUCE_TIMEOUT              10
#define MAX_REDUCE_TIMEOUT              50

/* flag */
#define YES                             1
#define NO                              0

#define VB_NAME_LENGTH                  20
#define HAPTIC_STOP                     0

/* hptics */
#define HAPTICS_NUM                     8

/* voltage */
#define FACTORY_VOLTAGE_VALUE           0x48

/* haptic */
#define END_HAPTIC_TEST_FLAG            0x99

/* eng data */
#ifdef CONFIG_HUAWEI_DSM
#ifdef CONFIG_HUAWEI_DATA_ACQUISITION
#define VIB_CAL_NUM                     4
#define DEFAULT_VERSION                 1

#define VIB_CAL_STATUS_MSG              703021001
#define VIB_CAL_CALI18_MSG              703021002
#define VIB_CAL_CALI19_MSG              703021003
#define VIB_CAL_CALI1A_MSG              703021004

#define NOT_INVOLVE                     "NA"
#define VIB_STATION                     "MMI1"
#define VIB_TEST_NAME                   "VIBRATOR"
#endif
#endif

enum dbc_test_type {
	CLOSE_TEST = 0,
	OUT_ADD_TEST,
	OUT_DELETE_TEST
};

struct drv2605_data {
	struct led_classdev cclassdev;
	int value;
	int gpio_enable;
	int gpio_pwm;
	int max_timeout_ms;
	int reduce_timeout_ms;
	volatile int should_stop;
	struct i2c_client *client;
	struct mutex lock;
	struct work_struct work;
	struct work_struct work_play_eff;
	struct work_struct work_enable;
	uint8_t sequence[HAPTICS_NUM];
	struct class *class;
	struct device *device;
	dev_t version;
	struct cdev cdev;
	struct switch_dev sw_dev;
};

struct drv2605_pdata {
	int gpio_enable;
	int gpio_pwm;
	int max_timeout_ms;
	int reduce_timeout_ms;
	int support_amplitude_control;
	int check_power_state1;
	int check_power_state2;
	int check_power_time;
	char lra_rated_voltage;
	char lra_overdriver_voltage;
	char lra_rtp_strength;
	const char *name;
};
#endif
