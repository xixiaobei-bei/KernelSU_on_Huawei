/*
 * hostcomm.h
 *
 * hostcomm driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#ifdef FSC_DEBUG

#ifndef _HOSTCOMM_H_
#define _HOSTCOMM_H_

/* Packet Structure */
#define PKTOUT_REQUEST                  0x00
#define PKTOUT_VERSION                  0x01
#define PKTIN_REQUEST                   0x00
#define PKTIN_STATUS                    0x01
#define PKTIN_ERRORCODE                 0x03

/* Command definitions */
#define CMD_GETDEVICEINFO               0x00
#define CMD_USBPD_BUFFER_READ           0xA0
#define CMD_USBPD_STATUS                0xA2
#define CMD_USBPD_CONTROL               0xA3
#define CMD_GET_SRC_CAPS                0xA4
#define CMD_GET_SINK_CAPS               0xA5
#define CMD_GET_SINK_REQ                0xA6
#define CMD_ENABLE_PD                   0xA7
#define CMD_DISABLE_PD                  0xA8
#define CMD_GET_ALT_MODES               0xA9
#define CMD_GET_MANUAL_RETRIES          0xAA
#define CMD_SET_STATE_UNATTACHED        0xAB
#define CMD_ENABLE_TYPEC_SM             0xAC
#define CMD_DISABLE_TYPEC_SM            0xAD
#define CMD_SEND_HARD_RESET             0xAE

#define CMD_DEVICE_LOCAL_REGISTER_READ  0xB0
#define CMD_SET_STATE                   0xB1
#define CMD_READ_STATE_LOG              0xB2
#define CMD_READ_PD_STATE_LOG           0xB3

#define CMD_READ_I2C                    0xC0
#define CMD_WRITE_I2C                   0xC1
#define CMD_GET_VBUS5V                  0xC4
#define CMD_SET_VBUS5V                  0xC5
#define CMD_GET_INTN                    0xC6

#define CMD_GET_TIMER_TICKS             0xF0
#define CMD_GET_SM_TICKS                0xF1
#define CMD_GET_GPIO_SM_TOGGLE          0xF2
#define CMD_SET_GPIO_SM_TOGGLE          0xF3

/* For GUI identification of firmware */
#define TEST_FIRMWARE                   0x01

/* Device Info */
/* MCU Identification */
enum mcu_type {
	MCU_UNKNOWN = 0,
	MCU_PIC18F14K50 = 1,
	MCU_PIC32MX795F512L = 2,
	MCU_PIC32MX250F128B = 3,
	MCU_GENERIC_LINUX = 4, /* Linux driver */
};

/* Device Type Identification */
enum dt_type {
	DT_UNKNOWN = -1,
	DT_USBI2C_STANDARD = 0,
	DT_USBI2CPD_TYPEC = 1,
};

/* Board Configuration */
enum bc_type {
	BC_UNKNOWN = -1,
	BC_STANDARD_I2C_CONFIG = 0,
	BC_FUSB300_EVAL = 0x100,
	BC_FUSB302_FPGA = 0x200,
	BC_FM14014 = 0x300,
};

#define MY_MCU          MCU_GENERIC_LINUX
#define MY_DEV_TYPE     DT_USBI2CPD_TYPEC
#define MY_BC           BC_STANDARD_I2C_CONFIG

void fusb_process_msg(u8 *in_msg_buffer, u8 *out_msg_buffer);

#endif /* _HOSTCOMM_H_ */
#endif /* FSC_DEBUG */
