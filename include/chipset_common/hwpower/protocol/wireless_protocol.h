/*
 * wireless_protocol.h
 *
 * wireless protocol driver
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

#ifndef _WIRELESS_PROTOCOL_H_
#define _WIRELESS_PROTOCOL_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>

#define WIRELESS_LOG_BUF_SIZE     128
#define WIRELESS_TX_FWVER_LEN     32
#define WIRELESS_TX_BD_STR_LEN    32 /* 32bit bigdata */

enum wireless_protocol_type {
	WIRELESS_PROTOCOL_BEGIN = 0,
	WIRELESS_PROTOCOL_QI = WIRELESS_PROTOCOL_BEGIN, /* qi */
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
	WIRELESS_DEVICE_ID_STWLC88,
	WIRELESS_DEVICE_ID_CPS7181,
	WIRELESS_DEVICE_ID_CPS4057,
	WIRELESS_DEVICE_ID_MT5735,
	WIRELESS_DEVICE_ID_CPS4029,
	WIRELESS_DEVICE_ID_END,
};

enum wireless_protocol_adapter_voltage {
	WIRELESS_ADAPTER_0V = 0,
	WIRELESS_ADAPTER_5V = 5000, /* mv */
	WIRELESS_ADAPTER_9V = 9000, /* mv */
	WIRELESS_ADAPTER_12V = 12000, /* mv */
	WIRELESS_ADAPTER_15V = 15000, /* mv */
};

enum wireless_protocol_tx_type {
	WIRELESS_TX_TYPE_UNKNOWN = 0,
	WIRELESS_TX_TYPE_CP85,
	WIRELESS_TX_TYPE_CP60,
	WIRELESS_TX_TYPE_CP61,
	WIRELESS_TX_TYPE_CP62_LX,
	WIRELESS_TX_TYPE_CP62_XW,
	WIRELESS_TX_TYPE_CP62R,
	WIRELESS_TX_TYPE_CP39S,
	WIRELESS_TX_TYPE_CP39S_HK,
	WIRELESS_TX_TYPE_CK30_LX,
	WIRELESS_TX_TYPE_CK30_LP,
	WIRELESS_TX_TYPE_ERROR = 0xff,
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
	bool support_get_ept;
	bool support_fop_range;
};

struct wireless_protocol_tx_fop_range {
	int base_min;
	int base_max;
	int ext1_min;
	int ext1_max;
	int ext2_min;
	int ext2_max;
};

struct wireless_protocol_tx_alarm {
	u8 src;
	u32 plim;
	u32 vlim;
	u8 reserved;
};

enum wireless_protocol_tx_alarm_src {
	TX_ALARM_SRC_TEMP = 0,
	TX_ALARM_SRC_PLIM,
	TX_ALARM_SRC_FAN,
	TX_ALARM_SRC_FOD,
	TX_ALARM_SRC_TIME,
	TX_ALARM_SRC_QVAL,
};

enum wireless_evt {
	RX_NO_EVENT = 0,
	RX_STATR_CHARGING
};

enum wireless_device_type {
	WIRELESS_DEVICE_UNKNOWN = 0,
	WIRELESS_DEVICE_PHONE,
	WIRELESS_DEVICE_EAR,
	WIRELESS_DEVICE_PAD,
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
	int (*send_rx_evt)(unsigned int, u8);
	int (*send_rx_vout)(unsigned int, int);
	int (*send_rx_iout)(unsigned int, int);
	int (*send_rx_boost_succ)(unsigned int);
	int (*send_rx_ready)(unsigned int);
	int (*send_tx_capability)(unsigned int, u8 *, int);
	int (*send_tx_alarm)(unsigned int, u8 *, int);
	int (*send_tx_fw_version)(unsigned int, u8 *, int);
	int (*send_cert_confirm)(unsigned int, bool);
	int (*send_charge_state)(unsigned int, u8);
	int (*send_fod_status)(unsigned int, int);
	int (*send_charge_mode)(unsigned int, u8);
	int (*set_fan_speed_limit)(unsigned int, u8);
	int (*set_rpp_format)(unsigned int, u8);
	int (*get_ept_type)(unsigned int, u16 *);
	int (*get_rpp_format)(unsigned int, u8 *);
	int (*get_tx_fw_version)(unsigned int, char *, int);
	int (*get_tx_bigdata_info)(unsigned int, char *, int);
	int (*get_tx_id)(unsigned int, int *);
	int (*get_tx_type)(unsigned int, int *);
	int (*get_tx_adapter_type)(unsigned int, int *);
	int (*get_tx_capability)(unsigned int, struct wireless_protocol_tx_cap *);
	int (*get_tx_max_power)(unsigned int, int *);
	int (*get_tx_adapter_capability)(unsigned int, int *, int *);
	int (*get_tx_cable_type)(unsigned int, int *, int *);
	int (*get_tx_fop_range)(unsigned int, struct wireless_protocol_tx_fop_range *);
	int (*auth_encrypt_start)(unsigned int, int, u8 *, int, u8 *, int);
	int (*fix_tx_fop)(unsigned int, int);
	int (*unfix_tx_fop)(unsigned int);
	int (*reset_dev_info)(unsigned int);
	/* for wireless accessory */
	int (*acc_set_tx_dev_state)(unsigned int, u8);
	int (*acc_set_tx_dev_info_cnt)(unsigned int, u8);
	int (*acc_get_tx_dev_state)(unsigned int, u8 *);
	int (*acc_get_tx_dev_no)(unsigned int, u8 *);
	int (*acc_get_tx_dev_mac)(unsigned int, u8 *, int);
	int (*acc_get_tx_dev_model_id)(unsigned int, u8 *, int);
	int (*acc_get_tx_dev_submodel_id)(unsigned int, u8 *);
	int (*acc_get_tx_dev_version)(unsigned int, u8 *);
	int (*acc_get_tx_dev_business)(unsigned int, u8 *);
	int (*acc_get_tx_dev_info_cnt)(unsigned int, u8 *);
	/* for wireless tx auth */
	int (*get_rx_random)(unsigned int, u8 *, int);
	int (*set_tx_cipherkey)(unsigned int, u8 *, int);
};

struct wireless_protocol_dev {
	struct wireless_device_info info;
	unsigned int total_ops;
	struct wireless_protocol_ops *p_ops[WIRELESS_PROTOCOL_END];
};

#ifdef CONFIG_WIRELESS_PROTOCOL
struct wireless_protocol_dev *wireless_get_protocol_dev(void);
int wireless_protocol_ops_register(struct wireless_protocol_ops *ops);
int wireless_send_charge_event(unsigned int ic_type, int prot, u8 evt);
int wireless_send_rx_vout(unsigned int ic_type, int prot, int rx_vout);
int wireless_send_rx_iout(unsigned int ic_type, int prot, int rx_iout);
int wireless_send_rx_boost_succ(unsigned int ic_type, int prot);
int wireless_send_rx_ready(unsigned int ic_type, int prot);
int wireless_send_tx_capability(unsigned int ic_type, int prot, u8 *cap, int len);
int wireless_send_tx_alarm(unsigned int ic_type, int prot, u8 *alarm, int len);
int wireless_send_tx_fw_version(unsigned int ic_type, int prot, u8 *fw, int len);
int wireless_send_cert_confirm(unsigned int ic_type, int prot, bool succ_flag);
int wireless_send_charge_state(unsigned int ic_type, int prot, u8 state);
int wireless_send_fod_status(unsigned int ic_type, int prot, int status);
int wireless_send_charge_mode(unsigned int ic_type, int prot, u8 mode);
int wireless_set_fan_speed_limit(unsigned int ic_type, int prot, u8 limit);
int wireless_set_rpp_format(unsigned int ic_type, int prot, u8 pmax);
int wireless_get_ept_type(unsigned int ic_type, int prot, u16 *type);
int wireless_get_rpp_format(unsigned int ic_type, int prot, u8 *format);
char *wireless_get_tx_fw_version(unsigned int ic_type, int prot);
int wireless_get_tx_bigdata_info(unsigned int ic_type, int prot);
int wireless_get_tx_id(unsigned int ic_type, int prot);
int wireless_get_tx_type(unsigned int ic_type, int prot);
int wireless_get_tx_adapter_type(unsigned int ic_type, int prot);
int wireless_get_tx_capability(unsigned int ic_type, int prot,
	struct wireless_protocol_tx_cap *cap);
int wireless_get_tx_max_power(unsigned int ic_type, int prot, int *power);
int wireless_get_tx_adapter_capability(unsigned int ic_type, int prot, int *vout, int *iout);
int wireless_get_tx_cable_type(unsigned int ic_type, int prot, int *type, int *iout);
int wireless_get_tx_fop_range(unsigned int ic_type, int prot,
	struct wireless_protocol_tx_fop_range *fop);
int wireless_auth_encrypt_start(unsigned int ic_type, int prot,
	int key, u8 *random, int r_size, u8 *hash, int h_size);
int wireless_fix_tx_fop(unsigned int ic_type, int prot, int fop);
int wireless_unfix_tx_fop(unsigned int ic_type, int prot);
int wireless_reset_dev_info(unsigned int ic_type, int prot);
/* for wireless accessory */
int wireless_acc_set_tx_dev_state(unsigned int ic_type, int prot, u8 state);
int wireless_acc_set_tx_dev_info_cnt(unsigned int ic_type, int prot, u8 cnt);
int wireless_acc_get_tx_dev_state(unsigned int ic_type, int prot, u8 *state);
int wireless_acc_get_tx_dev_no(unsigned int ic_type, int prot, u8 *no);
int wireless_acc_get_tx_dev_mac(unsigned int ic_type, int prot, u8 *mac, int len);
int wireless_acc_get_tx_dev_model_id(unsigned int ic_type, int prot, u8 *id, int len);
int wireless_acc_get_tx_dev_submodel_id(unsigned int ic_type, int prot, u8 *id);
int wireless_acc_get_tx_dev_version(unsigned int ic_type, int prot, u8 *ver);
int wireless_acc_get_tx_dev_business(unsigned int ic_type, int prot, u8 *bus);
int wireless_acc_get_tx_dev_info_cnt(unsigned int ic_type, int prot, u8 *cnt);
/* for wireless tx auth */
int wireless_get_rx_random(unsigned int ic_type, int prot, u8 *random, int len);
int wireless_set_tx_cipherkey(unsigned int ic_type, int prot, u8 *cipherkey, int len);
#else
static inline struct wireless_protocol_dev *wireless_get_protocol_dev(void)
{
	return NULL;
}

static inline int wireless_protocol_ops_register(struct wireless_protocol_ops *ops)
{
	return -ENOTSUPP;
}

static inline int wireless_send_charge_event(unsigned int ic_type, int prot, u8 evt)
{
	return -ENOTSUPP;
}

static inline int wireless_send_rx_vout(unsigned int ic_type, int prot, int rx_vout)
{
	return -ENOTSUPP;
}

static inline int wireless_send_rx_iout(unsigned int ic_type, int prot, int rx_iout)
{
	return -ENOTSUPP;
}

static inline int wireless_send_rx_boost_succ(unsigned int ic_type, int prot)
{
	return -ENOTSUPP;
}

static inline int wireless_send_rx_ready(unsigned int ic_type, int prot)
{
	return -ENOTSUPP;
}

static inline int wireless_send_tx_capability(unsigned int ic_type, int prot, u8 *cap, int len)
{
	return -ENOTSUPP;
}

static inline int wireless_send_tx_alarm(unsigned int ic_type, int prot, u8 *alarm, int len)
{
	return -ENOTSUPP;
}

static inline int wireless_send_tx_fw_version(unsigned int ic_type, int prot, u8 *fw, int len)
{
	return -ENOTSUPP;
}

static inline int wireless_send_cert_confirm(unsigned int ic_type, int prot, bool succ_flag)
{
	return -ENOTSUPP;
}

static inline int wireless_send_charge_state(unsigned int ic_type, int prot, u8 state)
{
	return -ENOTSUPP;
}

static inline int wireless_send_fod_status(unsigned int ic_type, int prot, int status)
{
	return -ENOTSUPP;
}

static inline int wireless_send_charge_mode(unsigned int ic_type, int prot, u8 mode)
{
	return -ENOTSUPP;
}

static inline int wireless_set_fan_speed_limit(unsigned int ic_type, int prot, u8 limit)
{
	return -ENOTSUPP;
}

static inline int wireless_set_rpp_format(unsigned int ic_type, int prot, u8 pmax)
{
	return -ENOTSUPP;
}

static inline int wireless_get_ept_type(unsigned int ic_type, int prot, u16 *type)
{
	return -ENOTSUPP;
}

static inline int wireless_get_rpp_format(unsigned int ic_type, int prot, u8 *format)
{
	return -ENOTSUPP;
}

static inline char *wireless_get_tx_fw_version(unsigned int ic_type, int prot)
{
	return NULL;
}

static inline int wireless_get_tx_bigdata_info(unsigned int ic_type, int prot)
{
	return -ENOTSUPP;
}

static inline int wireless_get_tx_id(unsigned int ic_type, int prot)
{
	return -ENOTSUPP;
}

static inline int wireless_get_tx_type(unsigned int ic_type, int prot)
{
	return -ENOTSUPP;
}

static inline int wireless_get_tx_adapter_type(unsigned int ic_type, int prot)
{
	return -ENOTSUPP;
}

static inline int wireless_get_tx_capability(unsigned int ic_type, int prot,
	struct wireless_protocol_tx_cap *cap)
{
	return -ENOTSUPP;
}

static inline int wireless_get_tx_max_power(unsigned int ic_type, int prot, int *power)
{
	return -ENOTSUPP;
}

static inline int wireless_get_tx_adapter_capability(unsigned int ic_type, int prot, int *vout,
	int *iout)
{
	return -ENOTSUPP;
}

static inline int wireless_get_tx_cable_type(unsigned int ic_type, int prot, int *type, int *iout)
{
	return -ENOTSUPP;
}

static inline int wireless_get_tx_fop_range(unsigned int ic_type, int prot,
	struct wireless_protocol_tx_fop_range *fop)
{
	return -ENOTSUPP;
}

static inline int wireless_auth_encrypt_start(unsigned int ic_type, int prot,
	int key, u8 *random, int r_size, u8 *hash, int h_size)
{
	return -ENOTSUPP;
}

static inline int wireless_fix_tx_fop(unsigned int ic_type, int prot, int fop)
{
	return -ENOTSUPP;
}

static inline int wireless_unfix_tx_fop(unsigned int ic_type, int prot)
{
	return -ENOTSUPP;
}

static inline int wireless_reset_dev_info(unsigned int ic_type, int prot)
{
	return -ENOTSUPP;
}

static inline int wireless_acc_set_tx_dev_state(unsigned int ic_type, int prot, u8 state)
{
	return -ENOTSUPP;
}

static inline int wireless_acc_set_tx_dev_info_cnt(unsigned int ic_type, int prot, u8 cnt)
{
	return -ENOTSUPP;
}

static inline int wireless_acc_get_tx_dev_state(unsigned int ic_type, int prot, u8 *state)
{
	return -ENOTSUPP;
}

static inline int wireless_acc_get_tx_dev_no(unsigned int ic_type, int prot, u8 *no)
{
	return -ENOTSUPP;
}

static inline int wireless_acc_get_tx_dev_mac(unsigned int ic_type, int prot, u8 *mac, int len)
{
	return -ENOTSUPP;
}

static inline int wireless_acc_get_tx_dev_model_id(unsigned int ic_type, int prot, u8 *id, int len)
{
	return -ENOTSUPP;
}

static inline int wireless_acc_get_tx_dev_submodel_id(unsigned int ic_type, int prot, u8 *id)
{
	return -ENOTSUPP;
}

static inline int wireless_acc_get_tx_dev_version(unsigned int ic_type, int prot, u8 *ver)
{
	return -ENOTSUPP;
}

static inline int wireless_acc_get_tx_dev_business(unsigned int ic_type, int prot, u8 *bus)
{
	return -ENOTSUPP;
}

static inline int wireless_acc_get_tx_dev_info_cnt(unsigned int ic_type, int prot, u8 *cnt)
{
	return -ENOTSUPP;
}

static inline int wireless_get_rx_random(unsigned int ic_type, int prot, u8 *random, int len)
{
	return -ENOTSUPP;
}

static inline int wireless_set_tx_cipherkey(unsigned int ic_type, int prot, u8 *cipherkey, int len)
{
	return -ENOTSUPP;
}
#endif /* CONFIG_WIRELESS_PROTOCOL */

#endif /* _WIRELESS_PROTOCOL_H_ */
