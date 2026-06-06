/*
 * charger_protocol.h
 *
 * charger protocol driver
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

#ifndef _ADAPTER_PROTOCOL_H_
#define _ADAPTER_PROTOCOL_H_

#define ADAPTER_RD_BUF_SIZE  32
#define ADAPTER_WR_BUF_SIZE  32

enum adapter_sysfs_type {
	ADAPTER_SYSFS_BEGIN = 0,

	ADAPTER_SYSFS_SUPPORT_MODE = ADAPTER_SYSFS_BEGIN,
	ADAPTER_SYSFS_CHIP_ID,
	ADAPTER_SYSFS_VENDOR_ID,
	ADAPTER_SYSFS_MODULE_ID,
	ADAPTER_SYSFS_SERIAL_NO,
	ADAPTER_SYSFS_HWVER,
	ADAPTER_SYSFS_FWVER,
	ADAPTER_SYSFS_MIN_VOLT,
	ADAPTER_SYSFS_MAX_VOLT,
	ADAPTER_SYSFS_MIN_CUR,
	ADAPTER_SYSFS_MAX_CUR,

	ADAPTER_SYSFS_END,
};

enum adapter_protocol_type {
	ADAPTER_PROTOCOL_BEGIN = 0,

	ADAPTER_PROTOCOL_SCP = ADAPTER_PROTOCOL_BEGIN, /* huawei scp */
	ADAPTER_PROTOCOL_FCP, /* huawei fcp */

	ADAPTER_PROTOCOL_END,
};

enum adapter_support_mode {
	ADAPTER_SUPPORT_UNDEFINED = 0x0,
	ADAPTER_SUPPORT_SCP_B_LVC = 0x1,
	ADAPTER_SUPPORT_SCP_B_SC = 0x2,
	ADAPTER_SUPPORT_SCP_A = 0x3,
	ADAPTER_SUPPORT_FCP = 0x4,
};

enum adapter_type {
	ADAPTER_TYPE_UNKNOWN = 0x0,
	ADAPTER_TYPE_5V5A = 0x1,
	ADAPTER_TYPE_10V4A = 0x2,
	ADAPTER_TYPE_10V2A = 0x3,
	ADAPTER_TYPE_20V3P25A = 0x4,
};

enum adapter_detect_error_code {
	ADAPTER_DETECT_OTHER = -1,
	ADAPTER_DETECT_SUCC = 0,
	ADAPTER_DETECT_FAIL = 1,
};

enum adapter_output_status {
	ADAPTER_OUTPUT_NORMAL = 0,
	ADAPTER_OUTPUT_OTP = 1,
	ADAPTER_OUTPUT_OCP = 2,
	ADAPTER_OUTPUT_OVP = 3,
	ADAPTER_OUTPUT_UVP = 4,
};

enum adapter_protocol_rw_error_flag {
	NO_RW_ERROR_FLAG = 0,
	RW_ERROR_FLAG = 1,
};

enum adapter_protocol_read_flag {
	NOT_READ_FLAG = 0,
	HAS_READ_FLAG = 2,
};

enum adapter_protocol_byte_num {
	BYTE_ONE = 1,
	BYTE_TWO = 2,
	BYTE_FOUR = 4,
	BYTE_EIGHT = 8,
	BYTE_SIXTEEN = 16,
};

enum adapter_protocol_retry_num {
	RETRY_ONE = 1,
	RETRY_TWO = 2,
	RETRY_THREE = 3,
	RETRY_FOUR = 4,
	RETRY_FIVE = 5,
	RETRY_SIX = 6,
};

enum adapter_protocol_chip_vendor {
	ADAPTER_CHIP_RICHTEK,
	ADAPTER_CHIP_IWATT,
	ADAPTER_CHIP_WELTREND,
	ADAPTER_CHIP_ID0X32,
};

enum adapter_protocol_device_id {
	DEVICE_ID_BEGIN = 0,

	DEVICE_ID_FSA9685 = DEVICE_ID_BEGIN, /* for fsa9685 */
	DEVICE_ID_SCHARGER_V300, /* for scharger_v300 */
	DEVICE_ID_SCHARGER_V600, /* for scharger_v600 */
	DEVICE_ID_FUSB3601, /* for fusb3601 */

	DEVICE_ID_END,
};

struct adapter_support_mode_data {
	int mode;
	const char *mode_name;
};

struct adapter_init_data {
	int scp_mode_enable;
	int vset_boundary;
	int iset_boundary;
	int init_adaptor_voltage;
	int watchdog_timer;
	int dp_delitch_time;
};

struct adapter_info {
	int support_mode;
	int chip_id;
	int vendor_id;
	int module_id;
	int serial_no;
	int hwver;
	int fwver;
	int min_volt;
	int max_volt;
	int min_cur;
	int max_cur;
	int adp_b_type;
	int compiledver;
	int specver;
	int volt_cap;
	int max_pwr;
};

struct adapter_protocol_ops {
	const char *type_name;
	int (*soft_reset_master)(void);
	int (*soft_reset_slave)(void);
	int (*detect_adapter_support_mode)(int *mode);
	int (*get_support_mode)(int *mode);
	int (*get_device_info)(struct adapter_info *info);
	int (*get_chip_vendor_id)(int *id);
	int (*set_output_enable)(int enable);
	int (*set_reset)(int enable);
	int (*set_output_voltage)(int volt);
	int (*get_output_voltage)(int *volt);
	int (*set_output_current)(int cur);
	int (*get_output_current)(int *cur);
	int (*get_output_current_set)(int *cur);
	int (*get_min_voltage)(int *volt);
	int (*get_max_voltage)(int *volt);
	int (*get_min_current)(int *cur);
	int (*get_max_current)(int *cur);
	int (*get_inside_temp)(int *temp);
	int (*get_port_temp)(int *temp);
	int (*get_port_leakage_current_flag)(int *flag);
	int (*auth_encrypt_start)(int key, unsigned char *hash, int size);
	int (*auth_encrypt_release)(void);
	int (*set_usbpd_enable)(int enable);
	int (*set_default_state)(void);
	int (*set_default_param)(void);
	int (*set_init_data)(struct adapter_init_data *data);
	int (*get_protocol_register_state)(void);
	int (*get_adp_type)(int *type);
	int (*get_slave_status)(void);
	int (*get_master_status)(void);
	int (*stop_charging_config)(void);
};

struct adapter_dev {
	struct device *dev;
	unsigned int type;
	struct adapter_info info;
	unsigned int total_ops;
	struct adapter_protocol_ops *p_ops[ADAPTER_PROTOCOL_END];
};

#ifdef CONFIG_CHARGER_PROTOCOL
int adapter_protocol_ops_register(struct adapter_protocol_ops *ops);

int adapter_switch_protocol_type(int type);
int adapter_soft_reset_master(void);
int adapter_soft_reset_slave(void);
int adapter_detect_adapter_support_mode(int *mode);
int adapter_get_support_mode(int *mode);
int adapter_get_device_info(void);
int adapter_show_device_info(void);
int adapter_get_chip_vendor_id(int *id);
int adapter_set_output_enable(int enable);
int adapter_set_reset(int enable);
int adapter_set_output_voltage(int volt);
int adapter_get_output_voltage(int *volt);
int adapter_set_output_current(int cur);
int adapter_get_output_current(int *cur);
int adapter_get_output_current_set(int *cur);
int adapter_get_min_voltage(int *volt);
int adapter_get_max_voltage(int *volt);
int adapter_get_min_current(int *cur);
int adapter_get_max_current(int *cur);
int adapter_get_inside_temp(int *temp);
int adapter_get_port_temp(int *temp);
int adapter_get_port_leakage_current_flag(int *flag);
int adapter_auth_encrypt_start(int key, unsigned char *hash, int size);
int adapter_auth_encrypt_release(void);
int adapter_set_usbpd_enable(int enable);
int adapter_set_default_state(void);
int adapter_set_default_param(void);
int adapter_set_init_data(struct adapter_init_data *data);
int adapter_get_protocol_register_state(void);
int adapter_get_adp_type(int *type);
int adapter_get_slave_status(void);
int adapter_get_master_status(void);
int adapter_stop_charging_config(void);

#else

static inline int adapter_protocol_ops_register(
	struct adapter_protocol_ops *ops)
{
	return -1;
}

static inline int adapter_switch_protocol_type(int type)
{
	return -1;
}

static inline int adapter_soft_reset_master(void)
{
	return -1;
}

static inline int adapter_soft_reset_slave(void)
{
	return -1;
}

static inline int adapter_detect_adapter_support_mode(int *mode)
{
	return -1;
}

static inline int adapter_get_support_mode(int *mode)
{
	return -1;
}

static inline int adapter_get_device_info(void)
{
	return -1;
}

static inline int adapter_show_device_info(void)
{
	return -1;
}

static inline int adapter_get_chip_vendor_id(int *id)
{
	return -1;
}

static inline int adapter_set_output_enable(int enable)
{
	return -1;
}

static inline int adapter_set_reset(int enable)
{
	return -1;
}

static inline int adapter_set_output_voltage(int volt)
{
	return -1;
}

static inline int adapter_get_output_voltage(int *volt)
{
	return -1;
}

static inline int adapter_set_output_current(int cur)
{
	return -1;
}

static inline int adapter_get_output_current(int *cur)
{
	return -1;
}

static inline int adapter_get_output_current_set(int *cur)
{
	return -1;
}

static inline int adapter_get_min_voltage(int *volt)
{
	return -1;
}

static inline int adapter_get_max_voltage(int *volt)
{
	return -1;
}

static inline int adapter_get_min_current(int *cur)
{
	return -1;
}

static inline int adapter_get_max_current(int *cur)
{
	return -1;
}

static inline int adapter_get_inside_temp(int *temp)
{
	return -1;
}

static inline int adapter_get_port_temp(int *temp)
{
	return -1;
}

static inline int adapter_get_port_leakage_current_flag(int *flag)
{
	return -1;
}

static inline int adapter_auth_encrypt_start(int key, unsigned char *hash,
	int size)
{
	return -1;
}

static inline int adapter_auth_encrypt_release(void)
{
	return -1;
}

static inline int adapter_set_usbpd_enable(int enable)
{
	return -1;
}

static inline int adapter_set_default_state(void)
{
	return -1;
}
static inline int adapter_set_default_param(void)
{
	return -1;
}
static inline int adapter_set_init_data(struct adapter_init_data *data)
{
	return -1;
}

static inline int adapter_get_protocol_register_state(void)
{
	return -1;
}

static inline int adapter_get_adp_type(int *type)
{
	return -1;
}

static inline int adapter_get_slave_status(void)
{
	return -1;
}

static inline int adapter_get_master_status(void)
{
	return -1;
}

static inline int adapter_stop_charging_config(void)
{
	return -1;
}
#endif /* CONFIG_CHARGER_PROTOCOL */

#endif /* _ADAPTER_PROTOCOL_H_ */
