/*
 * adapter_protocol.h
 *
 * adapter protocol driver
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

#ifndef _ADAPTER_PROTOCOL_H_
#define _ADAPTER_PROTOCOL_H_

enum adapter_protocol_type {
	ADAPTER_PROTOCOL_BEGIN = 0,
	ADAPTER_PROTOCOL_SCP = ADAPTER_PROTOCOL_BEGIN, /* huawei scp */
	ADAPTER_PROTOCOL_FCP, /* huawei fcp */
	ADAPTER_PROTOCOL_PD,
	ADAPTER_PROTOCOL_UVDM,
	ADAPTER_PROTOCOL_END,
};

enum adapter_support_mode {
	ADAPTER_SUPPORT_UNDEFINED = 0x0,
	ADAPTER_SUPPORT_SCP_B_LVC = 0x1,
	ADAPTER_SUPPORT_SCP_B_SC = 0x2,
	ADAPTER_SUPPORT_FCP = 0x4,
};

enum adapter_support_type {
	ADAPTER_TYPE_UNKNOWN = 0x0,
	ADAPTER_TYPE_9V2A,         /* 9v2a 5v2a */
	ADAPTER_TYPE_5V4P5A,       /* 5v4.5a 4.5v5a 5v2a */
	ADAPTER_TYPE_10V4A,        /* 10v4a 11v3a 5v4.5a 9v2a 5v2a */
	ADAPTER_TYPE_10V2A,        /* 10v2a 9v2a 5v2a */
	ADAPTER_TYPE_20V3P25A_MAX, /* 20v3.25a 20v2a 15v2.66a 12v3a 9v2a 5v2a */
	ADAPTER_TYPE_20V3P25A,     /* 20v3.25a 15v3a 12v2a 9v2a 5v2a */
	ADAPTER_TYPE_10V4A_BANK,   /* 10v4a */
	ADAPTER_TYPE_10V4A_CAR,    /* 10v4a */
	ADAPTER_TYPE_10V2P25A,     /* 10v2.25a */
	ADAPTER_TYPE_10V2P25A_CAR, /* 10v2.25a */
	ADAPTER_TYPE_11V6A,        /* 11v6a */
	/* Qiantang River double port adaptor typeC+typeA */
	ADAPTER_TYPE_QTR_C_20V3A, /* 20v3a 10v4a 5.5v4a */
	ADAPTER_TYPE_QTR_C_10V4A, /* 10v4a 5.5v4a */
	ADAPTER_TYPE_QTR_A_10V4A, /* 10v4a 5.5v4a */
	ADAPTER_TYPE_QTR_A_10V2P25A, /* 10v2.25a 5.5v4a */
	/* Huangpu River three port adaptor typeC1+typeA1+typeA2 */
	ADAPTER_TYPE_HPR_C_11V6A,
	ADAPTER_TYPE_HPR_C_10V4A,
	ADAPTER_TYPE_HPR_A_11V6A,
	ADAPTER_TYPE_HPR_A_10V4A,
	ADAPTER_TYPE_HPR_A_10V2P25A,
	ADAPTER_TYPE_JLR_20V6P7A,
	ADAPTER_TYPE_5V4P5A_BANK,  /* 5v4.5a */
	ADAPTER_TYPE_10V2P25A_BANK, /* 10v2.25a */
	ADAPTER_TYPE_11V6A_BANK,
	ADAPTER_TYPE_11V6A_CAR,
	ADAPTER_TYPE_HHR_20V4P5A,
	ADAPTER_TYPE_YLR_20V5A_CAR,
};

enum adapter_detect_error_code {
	ADAPTER_DETECT_OTHER = -1,
	ADAPTER_DETECT_SUCC = 0,
	ADAPTER_DETECT_FAIL = 1,
};

enum adapter_antifake_detect_code {
	ADAPTER_ANTIFAKE_FAIL = -1,
	ADAPTER_ANTIFAKE_INIT = 0,
	ADAPTER_ANTIFAKE_SUCC = 1,
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
	ADAPTER_CHIP_UNKNOWN = -1,
	ADAPTER_CHIP_RICHTEK = 0,
	ADAPTER_CHIP_IWATT,
	ADAPTER_CHIP_WELTREND,
	ADAPTER_CHIP_ID0X32,
};

enum adapter_protocol_device_id {
	PROTOCOL_DEVICE_ID_BEGIN = 0,
	PROTOCOL_DEVICE_ID_FSA9685 = PROTOCOL_DEVICE_ID_BEGIN,
	PROTOCOL_DEVICE_ID_RT8979,
	PROTOCOL_DEVICE_ID_SCHARGER_V300,
	PROTOCOL_DEVICE_ID_SCHARGER_V600,
	PROTOCOL_DEVICE_ID_FUSB3601,
	PROTOCOL_DEVICE_ID_BQ2560X,
	PROTOCOL_DEVICE_ID_RT9466,
	PROTOCOL_DEVICE_ID_RT1711H,
	PROTOCOL_DEVICE_ID_FUSB30X,
	PROTOCOL_DEVICE_ID_SM5450,
	PROTOCOL_DEVICE_ID_SC8545,
	PROTOCOL_DEVICE_ID_SC200X,
	PROTOCOL_DEVICE_ID_END,
};

struct adapter_protocol_device_data {
	unsigned int id;
	const char *name;
};

struct adapter_update_mode_data {
	unsigned int adp_type;
	unsigned int mode;
};

struct adapter_support_mode_data {
	int mode;
	const char *mode_name;
};

struct adapter_init_data {
	int scp_mode_enable;
	int vset_boundary;
	int iset_boundary;
	int init_voltage;
	int watchdog_timer;
	int dp_delitch_time;
};

struct adapter_device_info {
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
	int adp_type;
	int compiledver;
	int specver;
	int volt_cap;
	int max_pwr;
};

struct adapter_protocol_ops {
	const char *type_name;
	int (*soft_reset_master)(void);
	int (*soft_reset_slave)(void);
	int (*hard_reset_master)(void);
	int (*detect_adapter_support_mode)(int *mode);
	int (*get_support_mode)(int *mode);
	int (*get_device_info)(struct adapter_device_info *info);
	int (*get_chip_vendor_id)(int *id);
	int (*get_chip_serial_num)(int *id);
	int (*set_output_enable)(int enable);
	int (*set_output_mode)(int enable);
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
	int (*get_power_drop_current)(int *cur);
	int (*get_power_curve_num)(int *num);
	int (*get_power_curve)(int *val, unsigned int size);
	int (*get_inside_temp)(int *temp);
	int (*get_port_temp)(int *temp);
	int (*get_port_leakage_current_flag)(int *flag);
	int (*auth_encrypt_start)(int key, unsigned char *hash, int size);
	int (*auth_encrypt_release)(void);
	int (*set_usbpd_enable)(int enable);
	int (*set_default_state)(void);
	int (*set_default_param)(void);
	int (*set_init_data)(struct adapter_init_data *data);
	int (*set_slave_power_mode)(int mode);
	int (*set_rx_reduce_voltage)(void);
	int (*get_protocol_register_state)(void);
	int (*get_adp_type)(int *type);
	int (*get_slave_status)(void);
	int (*get_master_status)(void);
	int (*stop_charging_config)(void);
	bool (*is_accp_charger_type)(void);
	bool (*is_undetach_cable)(void);
};

struct adapter_protocol_dev {
	struct adapter_device_info info;
	unsigned int total_ops;
	struct adapter_protocol_ops *p_ops[ADAPTER_PROTOCOL_END];
};

#ifdef CONFIG_ADAPTER_PROTOCOL
struct adapter_protocol_dev *adapter_get_protocol_dev(void);
int adapter_protocol_ops_register(struct adapter_protocol_ops *ops);
int adapter_soft_reset_master(int prot);
int adapter_soft_reset_slave(int prot);
int adapter_hard_reset_master(int prot);
int adapter_detect_adapter_support_mode(int prot, int *mode);
int adapter_get_support_mode(int prot, int *mode);
void adapter_update_adapter_support_mode(int prot, unsigned int *mode);
int adapter_get_device_info(int prot);
int adapter_show_device_info(int prot);
int adapter_get_chip_vendor_id(int prot, int *id);
int adapter_get_chip_serial_num(int prot, int *num);
int adapter_set_output_enable(int prot, int enable);
int adapter_set_output_mode(int prot, int enable);
int adapter_set_reset(int prot, int enable);
int adapter_set_output_voltage(int prot, int volt);
int adapter_get_output_voltage(int prot, int *volt);
int adapter_set_output_current(int prot, int cur);
int adapter_get_output_current(int prot, int *cur);
int adapter_get_output_current_set(int prot, int *cur);
int adapter_get_min_voltage(int prot, int *volt);
int adapter_get_max_voltage(int prot, int *volt);
int adapter_get_min_current(int prot, int *cur);
int adapter_get_max_current(int prot, int *cur);
int adapter_get_power_drop_current(int prot, int *cur);
int adapter_get_power_curve_num(int prot, int *num);
int adapter_get_power_curve(int prot, int *val, unsigned int size);
int adapter_get_inside_temp(int prot, int *temp);
int adapter_get_port_temp(int prot, int *temp);
int adapter_get_port_leakage_current_flag(int prot, int *flag);
int adapter_auth_encrypt_start(int prot,
	int key, unsigned char *hash, int size);
int adapter_auth_encrypt_release(int prot);
int adapter_set_usbpd_enable(int prot, int enable);
int adapter_set_default_state(int prot);
int adapter_set_default_param(int prot);
int adapter_set_init_data(int prot, struct adapter_init_data *data);
int adapter_set_slave_power_mode(int prot, int mode);
int adapter_set_rx_reduce_voltage(int prot);
int adapter_get_protocol_register_state(int prot);
int adapter_get_adp_type(int prot, int *type);
int adapter_get_slave_status(int prot);
int adapter_get_master_status(int prot);
int adapter_stop_charging_config(int prot);
bool adapter_is_accp_charger_type(int prot);
bool adapter_is_undetach_cable(int prot);
#else
static inline struct adapter_protocol_dev *adapter_get_protocol_dev(void)
{
	return NULL;
}

static inline int adapter_protocol_ops_register(
	struct adapter_protocol_ops *ops)
{
	return -1;
}

static inline int adapter_soft_reset_master(int prot)
{
	return -1;
}

static inline int adapter_soft_reset_slave(int prot)
{
	return -1;
}

static inline int adapter_hard_reset_master(int prot)
{
	return -1;
}

static inline int adapter_detect_adapter_support_mode(int prot, int *mode)
{
	return -1;
}

static inline int adapter_get_support_mode(int prot, int *mode)
{
	return -1;
}

static inline void adapter_update_adapter_support_mode(int prot,
	unsigned int *mode)
{
}

static inline int adapter_get_device_info(int prot)
{
	return -1;
}

static inline int adapter_show_device_info(int prot)
{
	return -1;
}

static inline int adapter_get_chip_vendor_id(int prot, int *id)
{
	return -1;
}

static inline int adapter_get_chip_serial_num(int prot, int *num)
{
	return -1;
}

static inline int adapter_set_output_enable(int prot, int enable)
{
	return -1;
}

static inline int adapter_set_output_mode(int prot, int enable)
{
	return -1;
}

static inline int adapter_set_reset(int prot, int enable)
{
	return -1;
}

static inline int adapter_set_output_voltage(int prot, int volt)
{
	return -1;
}

static inline int adapter_get_output_voltage(int prot, int *volt)
{
	return -1;
}

static inline int adapter_set_output_current(int prot, int cur)
{
	return -1;
}

static inline int adapter_get_output_current(int prot, int *cur)
{
	return -1;
}

static inline int adapter_get_output_current_set(int prot, int *cur)
{
	return -1;
}

static inline int adapter_get_min_voltage(int prot, int *volt)
{
	return -1;
}

static inline int adapter_get_max_voltage(int prot, int *volt)
{
	return -1;
}

static inline int adapter_get_min_current(int prot, int *cur)
{
	return -1;
}

static inline int adapter_get_max_current(int prot, int *cur)
{
	return -1;
}

static inline int adapter_get_power_drop_current(int prot, int *cur)
{
	return -1;
}

static inline int adapter_get_power_curve_num(int prot, int *num)
{
	return -1;
}

static inline int adapter_get_power_curve(int prot, int *val, unsigned int size)
{
	return -1;
}

static inline int adapter_get_inside_temp(int prot, int *temp)
{
	return -1;
}

static inline int adapter_get_port_temp(int prot, int *temp)
{
	return -1;
}

static inline int adapter_get_port_leakage_current_flag(int prot, int *flag)
{
	return -1;
}

static inline int adapter_auth_encrypt_start(int prot,
	int key, unsigned char *hash, int size)
{
	return -1;
}

static inline int adapter_auth_encrypt_release(int prot)
{
	return -1;
}

static inline int adapter_set_usbpd_enable(int prot, int enable)
{
	return -1;
}

static inline int adapter_set_default_state(int prot)
{
	return -1;
}

static inline int adapter_set_default_param(int prot)
{
	return -1;
}

static inline int adapter_set_init_data(int prot,
	struct adapter_init_data *data)
{
	return -1;
}

static inline int adapter_set_slave_power_mode(int prot, int mode)
{
	return -1;
}

static inline int adapter_set_rx_reduce_voltage(int prot)
{
	return -1;
}

static inline int adapter_get_protocol_register_state(int prot)
{
	return -1;
}

static inline int adapter_get_adp_type(int prot, int *type)
{
	return -1;
}

static inline int adapter_get_slave_status(int prot)
{
	return -1;
}

static inline int adapter_get_master_status(int prot)
{
	return -1;
}

static inline int adapter_stop_charging_config(int prot)
{
	return -1;
}

static inline bool adapter_is_accp_charger_type(int prot)
{
	return false;
}

static inline bool adapter_is_undetach_cable(int prot)
{
	return false;
}
#endif /* CONFIG_ADAPTER_PROTOCOL */

#endif /* _ADAPTER_PROTOCOL_H_ */
