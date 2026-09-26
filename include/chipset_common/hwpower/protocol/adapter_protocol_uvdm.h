/*
 * adapter_protocol_uvdm.h
 *
 * uvdm protocol driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _ADAPTER_PROTOCOL_UVDM_H_
#define _ADAPTER_PROTOCOL_UVDM_H_

#include <linux/device.h>
#include <linux/completion.h>
#include <linux/notifier.h>

/* section: protocol data */
#define UVDM_VID                    0x12d1
#define UVDM_VDM_TYPE               0
#define UVDM_VERSION                0
#define UVDM_VDOS_LEN               7

/* section: protocol time */
#define UVDM_RETRY_TIMES            3
#define UVDM_WAIT_RESPONSE_TIME     1000

/* section: unit */
#define UVDM_VOLTAGE_UNIT           10

/* section: auth encrypt */
#define UVDM_KEY_INDEX_OFFSET       0
#define UVDM_RANDOM_S_OFFSET        1
#define UVDM_RANDOM_E_OFFESET       7
#define UVDM_RANDOM_HASH_SHIFT      8
#define UVDM_ENCTYPT_HASH_LEN       8
#define UVDM_ENCRYPT_RANDOM_LEN     8

/* section: mask */
#define uvdm_bit(i)                 ((1 << (i)) - 1)
#define UVDM_MASK_VDM_TYPE          uvdm_bit(1)
#define UVDM_MASK_POWER_TYPE        uvdm_bit(2)
#define UVDM_MASK_RESPONSE_STATE    uvdm_bit(3)
#define UVDM_MASK_OTG_EVENT         uvdm_bit(3)
#define UVDM_MASK_FUNCTION          uvdm_bit(5)
#define UVDM_MASK_CMD               uvdm_bit(7)
#define UVDM_MASK_KEY               uvdm_bit(8)
#define UVDM_MASK_REPORT_ABNORMAL   uvdm_bit(8)
#define UVDM_MASK_VOLTAGE           uvdm_bit(12)
#define UVDM_MASK_VID               uvdm_bit(16)

/*
 * uvdm message header data as below:
 * bit0: command direction
 * bit1~7: command
 * bit8~12: function
 * bit13~14: uvdm version
 * bit15: vdm type
 * bit16~31: vendor id
 */
enum uvdm_protocol_header_data_shift {
	UVDM_HDR_SHIFT_CMD_DIRECTTION = 0,
	UVDM_HDR_SHIFT_CMD = 1,
	UVDM_HDR_SHIFT_FUNCTION = 8,
	UVDM_HDR_SHIFT_VERSION = 13,
	UVDM_HDR_SHIFT_VDM_TYPE = 15,
	UVDM_HDR_SHIFT_VID = 16,
};

enum uvdm_protocol_command_direction {
	UVDM_CMD_DIRECTION_INITIAL,
	UVDM_CMD_DIRECTION_ANSWER,
};

enum uvdm_protocol_command_dc_ctrl {
	UVDM_CMD_REPORT_POWER_TYPE = 1,
	UVDM_CMD_SEND_RANDOM = 2,
	UVDM_CMD_GET_HASH = 3,
	UVDM_CMD_SWITCH_POWER_MODE = 4,
	UVDM_CMD_SET_ADAPTER_ENABLE = 5,
	UVDM_CMD_SET_VOLTAGE = 6,
	UVDM_CMD_GET_VOLTAGE = 7,
	UVDM_CMD_GET_TEMP = 8,
	UVDM_CMD_HARD_RESET = 9,
	UVDM_CMD_REPORT_ABNORMAL = 10,
	UVDM_CMD_SET_RX_REDUCE_VOLTAGE = 11,
};

enum uvdm_protocol_command_pd_ctrl {
	UVDM_CMD_REPORT_OTG_EVENT = 1,
};

enum uvdm_protocol_function {
	UVDM_FUNCTION_BEGIN,
	UVDM_FUNCTION_TA_CTRL,
	UVDM_FUNCTION_DOCK_CTRL,
	UVDM_FUNCTION_DC_CTRL,
	UVDM_FUCNTION_RX_CTRL,
	UVDM_FUNCTION_PD_CTRL,
	UVDM_FUNCTION_END,
};

enum uvdm_protocol_vdos_count {
	UVDM_VDOS_COUNT_BEGIN,
	UVDM_VDOS_COUNT_ONE,
	UVDM_VDOS_COUNT_TWO,
	UVDM_VDOS_COUNT_THREE,
	UVDM_VDOS_COUNT_FOUR,
	UVDM_VDOS_COUNT_FIVE,
	UVDM_VDOS_COUNT_SIX,
	UVDM_VDOS_COUNT_SEVEN,
	UVDM_VDOS_COUNT_END,
};

enum uvdm_protocol_vdo_byte {
	UVDM_VDO_BYTE_ZERO,
	UVDM_VDO_BYTE_ONE,
	UVDM_VDO_BYTE_TWO,
	UVDM_VDO_BYTE_THREE,
	UVDM_VDO_BYTE_FOUR,
	UVDM_VDO_BYTE_FIVE,
	UVDM_VDO_BYTE_SIX,
	UVDM_VDO_BYTE_SEVEN,
};

enum uvdm_protocol_power_mode {
	UVDM_PWR_MODE_DEFAULT,
	UVDM_PWR_MODE_BUCK2SC,
	UVDM_PWR_MODE_SC2BUCK5W,
	UVDM_PWR_MODE_SC2BUCK10W,
};

enum uvdm_protocol_response_status {
	UVDM_RESPONSE_ACK,
	UVDM_RESPONSE_NAK,
	UVDM_RESPONSE_BUSY,
	UVDM_RESPONSE_TX_INTERRUPT,
	UVDM_RESPONSE_POWER_READY,
};

struct uvdm_protocol_header_data {
	u32 cmd_direction;
	u32 cmd;
	u32 function;
	u32 uvdm_version;
	u32 vdm_type;
	u32 vid;
};

struct uvdm_protocol_device_info {
	int power_type;
	int volt;
	int abnormal_flag;
	int otg_event;
};

struct uvdm_protocol_ops {
	const char *chip_name;
	void *dev_data;
	void (*send_data)(u32 *, u8, bool, void *);
};

struct uvdm_protocol_dev {
	struct device *dev;
	struct notifier_block nb;
	struct completion rsp_comp;
	struct completion report_type_comp;
	int dev_id;
	struct uvdm_protocol_ops *p_ops;
	u8 encrypt_random_value[UVDM_ENCRYPT_RANDOM_LEN];
	u8 encrypt_hash_value[UVDM_ENCTYPT_HASH_LEN];
	struct uvdm_protocol_device_info info;
};

#ifdef CONFIG_ADAPTER_PROTOCOL_UVDM
int uvdm_protocol_ops_register(struct uvdm_protocol_ops *ops);
#else
static inline int uvdm_protocol_ops_register(struct uvdm_protocol_ops *ops)
{
	return 0;
}
#endif /* CONFIG_ADAPTER_PROTOCOL_UVDM */

#endif /* _ADAPTER_PROTOCOL_UVDM_H_ */
