/*
 * wireless_protocol.h
 *
 * wireless protocol driver
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

#ifndef _WIRELESS_PROTOCOL_H_
#define _WIRELESS_PROTOCOL_H_

#define WIRELESS_LOG_BUF_SIZE     128
#define WIRELESS_TX_FWVER_LEN     32

#define WIRELESS_TX_BD_STR_LEN    32 /* 32bit bigdata */

enum wireless_protocol_type {
	WIRELESS_PROTOCOL_BEGIN = 0,
	WIRELESS_PROTOCOL_QI = WIRELESS_PROTOCOL_BEGIN, /* qi/wpc */
	WIRELESS_PROTOCOL_A4WP, /* a4wp */
	WIRELESS_PROTOCOL_END,
};

enum wireless_protocol_read_flag {
	WIRELESS_NOT_RD_FLAG = 0,
	WIRELESS_HAS_RD_FLAG = 2,
};

enum wireless_protocol_byte_num {
	WIRELESS_BYTE_ONE = 1,
	WIRELESS_BYTE_TWO = 2,
	WIRELESS_BYTE_FOUR = 4,
	WIRELESS_BYTE_EIGHT = 8,
	WIRELESS_BYTE_SIXTEEN = 16,
};

enum wireless_protocol_retry_num {
	WIRELESS_RETRY_ONE = 1,
	WIRELESS_RETRY_TWO = 2,
	WIRELESS_RETRY_THREE = 3,
	WIRELESS_RETRY_FOUR = 4,
	WIRELESS_RETRY_FIVE = 5,
	WIRELESS_RETRY_SIX = 6,
};

enum wireless_protocol_chip_vendor {
	WIRELESS_CHIP_IDT,
	WIRELESS_CHIP_ST,
};

enum wireless_protocol_device_id {
	WIRELESS_DEVICE_ID_BEGIN = 0,
	WIRELESS_DEVICE_ID_IDTP9221 = WIRELESS_DEVICE_ID_BEGIN,
	WIRELESS_DEVICE_ID_STWLC68,
	WIRELESS_DEVICE_ID_IDTP9415,
	WIRELESS_DEVICE_ID_NU1619,
	WIRELESS_DEVICE_ID_END,
};

enum wireless_protocol_adapter_voltage {
	WIRELESS_ADAPTER_0V = 0,
	WIRELESS_ADAPTER_5V = 5000, /* mv */
	WIRELESS_ADAPTER_9V = 9000, /* mv */
	WIRELESS_ADAPTER_12V = 12000, /* mv */
};

enum wireless_protocol_tx_type {
	WIRELESS_TX_TYPE_UNKNOWN = 0,
	WIRELESS_TX_TYPE_CP58,
	WIRELESS_TX_TYPE_CP60,
	WIRELESS_TX_TYPE_CP61,
	WIRELESS_TX_TYPE_CP39S,
};

struct wireless_protocol_device_data {
	unsigned int id;
	const char *name;
};

struct wireless_protocol_tx_cap {
	u8 type;
	u8 ext_type;
	int vout_max;
	int iout_max;
	bool can_boost;
	bool cable_ok;
	bool no_need_cert;
	bool support_scp;
	bool support_12v;
	bool support_extra_cap;
	bool support_fan;
	bool support_tec;
	bool support_fod_status;
};

struct wireless_device_info {
	char tx_fwver[WIRELESS_TX_FWVER_LEN];
	char tx_bd_info[WIRELESS_TX_BD_STR_LEN];
	int tx_id;
	int tx_type;
	int tx_adp_type;
};

struct wireless_protocol_ops {
	const char *type_name;
	int (*send_rx_vout)(int);
	int (*send_rx_iout)(int);
	int (*send_rx_boost_succ)(void);
	int (*send_rx_ready)(void);
	int (*send_tx_capability)(u8 *, int);
	int (*send_tx_fw_version)(u8 *, int);
	int (*send_cert_confirm)(bool);
	int (*send_charge_state)(u8);
	int (*send_fod_status)(int);
	int (*send_charge_mode)(u8);
	int (*set_fan_speed_limit)(u8);
	int (*set_rpp_format)(u8);
	int (*get_ept_type)(u16 *);
	int (*get_rpp_format)(u8 *);
	int (*get_tx_fw_version)(char *, int);
	void (*get_tx_bd_info)(char *, int);
	int (*get_tx_id)(int *);
	int (*get_tx_type)(int *);
	int (*get_tx_adapter_type)(int *);
	int (*get_tx_capability)(struct wireless_protocol_tx_cap *);
	int (*auth_encrypt_start)(int, u8 *, int, u8 *, int);
	int (*fix_tx_fop)(int);
	int (*unfix_tx_fop)(void);
	void (*reset_dev_info)(void);
	/* for wireless accessory */
	int (*acc_set_tx_dev_state)(u8);
	int (*acc_set_tx_dev_info_cnt)(u8);
	int (*acc_get_tx_dev_state)(u8 *);
	int (*acc_get_tx_dev_no)(u8 *);
	int (*acc_get_tx_dev_mac)(u8 *, int);
	int (*acc_get_tx_dev_model_id)(u8 *, int);
	int (*acc_get_tx_dev_submodel_id)(u8 *);
	int (*acc_get_tx_dev_version)(u8 *);
	int (*acc_get_tx_dev_business)(u8 *);
	int (*acc_get_tx_dev_info_cnt)(u8 *);
};

struct wireless_protocol_dev {
	struct wireless_device_info info;
	unsigned int total_ops;
	struct wireless_protocol_ops *p_ops[WIRELESS_PROTOCOL_END];
};

#ifdef CONFIG_WIRELESS_PROTOCOL
struct wireless_protocol_dev *wireless_get_protocol_dev(void);
int wireless_protocol_ops_register(struct wireless_protocol_ops *ops);
int wireless_send_rx_vout(int prot, int rx_vout);
int wireless_send_rx_iout(int prot, int rx_iout);
int wireless_send_rx_boost_succ(int prot);
int wireless_send_rx_ready(int prot);
int wireless_send_tx_capability(int prot, u8 *cap, int len);
int wireless_send_tx_fw_version(int prot, u8 *fw, int len);
int wireless_send_cert_confirm(int prot, bool succ_flag);
int wireless_send_charge_state(int prot, u8 state);
int wireless_send_fod_status(int prot, int status);
int wireless_send_charge_mode(int prot, u8 mode);
int wireless_set_fan_speed_limit(int prot, u8 limit);
int wireless_set_rpp_format(int prot, u8 pmax);
int wireless_get_ept_type(int prot, u16 *type);
int wireless_get_rpp_format(int prot, u8 *format);
char *wireless_get_tx_fw_version(int prot);
void wireless_get_tx_bigdata_info(int prot);
int wireless_get_tx_id(int prot);
int wireless_get_tx_type(int prot);
int wireless_get_tx_adapter_type(int prot);
int wireless_get_tx_capability(int prot, struct wireless_protocol_tx_cap *cap);
int wireless_auth_encrypt_start(int prot,
	int key, u8 *random, int r_size, u8 *hash, int h_size);
int wireless_fix_tx_fop(int prot, int fop);
int wireless_unfix_tx_fop(int prot);
void wireless_reset_dev_info(int prot);
/* for wireless accessory */
int wireless_acc_set_tx_dev_state(int prot, u8 state);
int wireless_acc_set_tx_dev_info_cnt(int prot, u8 cnt);
int wireless_acc_get_tx_dev_state(int prot, u8 *state);
int wireless_acc_get_tx_dev_no(int prot, u8 *no);
int wireless_acc_get_tx_dev_mac(int prot, u8 *mac, int len);
int wireless_acc_get_tx_dev_model_id(int prot, u8 *id, int len);
int wireless_acc_get_tx_dev_submodel_id(int prot, u8 *id);
int wireless_acc_get_tx_dev_version(int prot, u8 *ver);
int wireless_acc_get_tx_dev_business(int prot, u8 *bus);
int wireless_acc_get_tx_dev_info_cnt(int prot, u8 *cnt);
#else
static inline struct wireless_protocol_dev *wireless_get_protocol_dev(void)
{
	return NULL;
}

static inline int wireless_protocol_ops_register(
	struct wireless_protocol_ops *ops)
{
	return -1;
}

static inline int wireless_send_rx_vout(int prot, int rx_vout)
{
	return -1;
}

static inline int wireless_send_rx_iout(int prot, int rx_iout)
{
	return -1;
}

static inline int wireless_send_rx_boost_succ(int prot)
{
	return -1;
}

static inline int wireless_send_rx_ready(int prot)
{
	return -1;
}

static inline int wireless_send_tx_capability(int prot, u8 *cap, int len)
{
	return -1;
}

static inline int wireless_send_tx_fw_version(int prot, u8 *fw, int len)
{
	return -1;
}

static inline int wireless_send_cert_confirm(int prot, bool succ_flag)
{
	return -1;
}

static inline int wireless_send_charge_state(int prot, u8 state)
{
	return -1;
}

static inline int wireless_send_fod_status(int prot, int status)
{
	return -1;
}

static inline int wireless_send_charge_mode(int prot, u8 mode)
{
	return -1;
}

static inline int wireless_set_fan_speed_limit(int prot, u8 limit)
{
	return -1;
}

static inline int wireless_set_rpp_format(int prot, u8 pmax)
{
	return -1;
}

static inline int wireless_get_ept_type(int prot, u16 *type)
{
	return -1;
}

static inline int wireless_get_rpp_format(int prot, u8 *format)
{
	return -1;
}

static inline char *wireless_get_tx_fw_version(int prot)
{
	return NULL;
}

static inline void wireless_get_tx_bigdata_info(int prot)
{
	return;
}

static inline int wireless_get_tx_id(int prot)
{
	return -1;
}

static inline int wireless_get_tx_type(int prot)
{
	return -1;
}

static inline int wireless_get_tx_adapter_type(int prot)
{
	return -1;
}

static inline int wireless_get_tx_capability(int prot,
	struct wireless_protocol_tx_cap *cap)
{
	return -1;
}

static inline int wireless_auth_encrypt_start(int prot,
	int key, u8 *random, int r_size, u8 *hash, int h_size)
{
	return -1;
}

static inline int wireless_fix_tx_fop(int prot, int fop)
{
	return -1;
}

static inline int wireless_unfix_tx_fop(int prot)
{
	return -1;
}

static inline void wireless_reset_dev_info(int prot)
{
}

static inline int wireless_acc_set_tx_dev_state(int prot, u8 state)
{
	return -1;
}

static inline int wireless_acc_set_tx_dev_info_cnt(int prot, u8 cnt)
{
	return -1;
}

static inline int wireless_acc_get_tx_dev_state(int prot, u8 *state)
{
	return -1;
}

static inline int wireless_acc_get_tx_dev_no(int prot, u8 *no)
{
	return -1;
}

static inline int wireless_acc_get_tx_dev_mac(int prot, u8 *mac, int len)
{
	return -1;
}

static inline int wireless_acc_get_tx_dev_model_id(int prot, u8 *id, int len)
{
	return -1;
}

static inline int wireless_acc_get_tx_dev_submodel_id(int prot, u8 *id)
{
	return -1;
}

static inline int wireless_acc_get_tx_dev_version(int prot, u8 *ver)
{
	return -1;
}

static inline int wireless_acc_get_tx_dev_business(int prot, u8 *bus)
{
	return -1;
}

static inline int wireless_acc_get_tx_dev_info_cnt(int prot, u8 *cnt)
{
	return -1;
}
#endif /* CONFIG_WIRELESS_PROTOCOL */

#endif /* _WIRELESS_PROTOCOL_H_ */
