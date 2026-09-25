/*
 * wireless_protocol_qi.h
 *
 * qi protocol driver
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

#ifndef _WIRELESS_PROTOCOL_QI_H_
#define _WIRELESS_PROTOCOL_QI_H_

#ifndef BIT
#define BIT(x)                                (1 << (x))
#endif

#define QI_PROTOCOL_BYTE_BITS                 8
#define QI_PROTOCOL_BYTE_MASK                 0xff

/* qi protocol command set */
#define QI_CMD_GET_SIGNAL_STRENGTH            0x01
#define QI_CMD_SET_TX_FOP                     0x03 /* set tx operating freq */
#define QI_CMD_GET_TX_FOP                     0x04 /* get tx operating freq */
#define QI_CMD_GET_TX_VERSION                 0x05 /* read tx fw version */
#define QI_CMD_GET_TX_IIN                     0x06 /* read tx vin */
#define QI_CMD_GET_TX_VIN                     0x07 /* read tx vin */
#define QI_CMD_SET_TX_VIN                     0x0a
#define QI_CMD_GET_TX_ADAPTER_TYPE            0x0b /* get adapter type */
#define QI_CMD_SEND_RX_READY                  0x0f /* rx ready */

#define QI_CMD_CONFIRM_START_CE               0x10
#define QI_CMD_CONFIRM_STOP_CE                0x11
#define QI_CMD_SEND_SN                        0x12 /* sn3~sn0 */
#define QI_CMD_SN7_4                          0x13
#define QI_CMD_SN11_8                         0x14
#define QI_CMD_SN15_12                        0x15
#define QI_CMD_SEND_BATT_TEMP                 0x16 /* send temp */
#define QI_CMD_SEND_BATT_CAPACITY             0x17 /* send battery voltage */
#define QI_CMD_SET_CURRENT_LIMIT              0x18 /* confirm 300ma */
#define QI_CMD_CONFIRM_1000MA                 0x19
#define QI_CMD_CLOSE_VBUS                     0x1a
#define QI_CMD_OPEN_VBUS                      0x1b
#define QI_CMD_VBUS_TO_VBATT                  0x1c
#define QI_CMD_SEND_RX_VOUT                   0x1d
#define QI_CMD_SEND_RX_IOUT                   0x1e
#define QI_CMD_RX_BOOST_SUCC                  0x1f /* rx boost success */

#define QI_CMD_CERT_SUCC                      0x20 /* rx auth success */
#define QI_CMD_CERT_FAIL                      0x21 /* rx auth fail */
#define QI_CMD_CHARGER_IC_IIN                 0x22
#define QI_CMD_SEND_CHRG_MODE                 0x23

#define QI_CMD_TX_START_COUNT_CE              0x32
#define QI_CMD_TX_STOP_COUNT_CE               0x33
#define QI_CMD_GET_TX_TEMP                    0x34
#define QI_CMD_GET_TX_COIL_NO                 0x35
#define QI_CMD_START_CERT                     0x36 /* send radom3~0 */
#define QI_CMD_SEND_RAMDOM7_4                 0x37
#define QI_CMD_GET_HASH                       0x38 /* get hash3~0 */
#define QI_CMD_GET_HASH7_4                    0x39
#define QI_CMD_GET_GET_HANDSHAKE_TIME         0x3a
#define QI_CMD_GET_TX_ID                      0x3b /* handshake */
#define QI_CMD_GET_CABLE_TYPE                 0x3c
#define QI_CMD_SET_ATE_TEST_MODE              0x3d
#define QI_CMD_USER_ID                        0x3f

#define QI_CMD_GET_ERROR_TEMP                 0x40
#define QI_CMD_GET_TX_CAP                     0x41 /* get tx capability */
#define QI_CMD_GET_TX_MAX_VIN_IIN             0x42
#define QI_CMD_SEND_CHRG_STATE                0x43 /* charger state */
#define QI_CMD_FIX_TX_FOP                     0x44 /* set tx fix freq */
#define QI_CMD_UNFIX_TX_FOP                   0x45 /* set tx not fix freq */
#define QI_CMD_GET_TX_COIL_COUNT              0x46
#define QI_CMD_GET_TX_COIL_NUMBER             0x47
#define QI_CMD_SEND_FOD_STATUS                0x48 /* fod status packet */
#define QI_CMD_GET_GET_TX_EXT_CAP             0x49 /* get tx extra capability */
#define QI_CMD_TX_ALARM                       0x4a
#define QI_CMD_PLOSS_ALARM_THRESHOLD          0x4b
#define QI_CMD_GET_EPT_TYPE                   0x4d

/* for wireless accessory */
#define QI_CMD_SEND_BT_MAC1                   0x52 /* rx to tx */
#define QI_CMD_SEND_BT_MAC2                   0x53 /* rx to tx */
#define QI_CMD_SEND_BT_MODEL_ID               0x54 /* rx to tx */

#define QI_CMD_SET_FAN_SPEED_LIMIT            0x69
#define QI_CMD_GET_RPP_FORMAT                 0x6b
#define QI_CMD_SET_RX_MAX_POWER               0x6c

#define QI_CMD_ACK_HEAD                       0x1e
#define QI_CMD_ACK_BST_ERR                    0xf0
#define QI_CMD_ACK                            0xff

/* qi protocol handshake */
#define QI_HANDSHAKE_ID_HIGH                  0x88
#define QI_HANDSHAKE_ID_LOW                   0x66
#define QI_HANDSHAKE_ID_HW                    0x8866

/* qi protocol ack */
#define QI_ACK_CMD_OFFSET                     0

/* cmd 0x05 */
#define QI_ACK_TX_FWVER_LEN                   5
#define QI_ACK_TX_FWVER_S                     1
#define QI_ACK_TX_FWVER_E                     4

/* cmd 0x0b */
#define QI_ACK_TX_ADP_TYPE_LEN                2
#define QI_ACK_TX_ADP_TYPE_OFFSET             1

/* cmd 0x1d */
#define QI_PARA_RX_VOUT_LEN                   2

/* cmd 0x1e */
#define QI_PARA_RX_IOUT_LEN                   2

/* cmd 0x23 */
#define QI_PARA_CHARGE_MODE_LEN               1

/* cmd 0x36 & 0x37 */
#define QI_PARA_RANDOM_LEN                    8
#define QI_PARA_KEY_INDEX_OFFSET              0
#define QI_PARA_RANDOM_S_OFFSET               1
#define QI_PARA_RANDOM_E_OFFSET               7
#define QI_PARA_RANDOM_GROUP_LEN              4

/* cmd 0x38 & 0x39 */
#define QI_ACK_HASH_LEN                       10
#define QI_ACK_HASH_S_OFFSET                  1
#define QI_ACK_HASH_E_OFFSET                  4
#define QI_ACK_HASH_GROUP_LEN                 5

/* cmd 0x3b */
#define QI_PARA_TX_ID_LEN                     2
#define QI_ACK_TX_ID_LEN                      3
#define QI_ACK_TX_ID_S_OFFSET                 1
#define QI_ACK_TX_ID_E_OFFSET                 2

/* cmd 0x41 */
#define QI_PARA_TX_CAP_LEN                    5
#define QI_PARA_TX_MAIN_CAP_S                 1
#define QI_PARA_TX_MAIN_CAP_E                 4
#define QI_PARA_TX_CAP_VOUT_STEP              100
#define QI_PARA_TX_CAP_IOUT_STEP              100
#define QI_PARA_TX_CAP_CABLE_OK_MASK          BIT(0)
#define QI_PARA_TX_CAP_CAN_BOOST_MASK         BIT(1)
#define QI_PARA_TX_CAP_EXT_TYPE_MASK          (BIT(2) | BIT(3))
#define QI_PARA_TX_CAP_CERT_MASK              BIT(4)
#define QI_PARA_TX_CAP_SUPPORT_SCP_MASK       BIT(5)
#define QI_PARA_TX_CAP_SUPPORT_12V_MASK       BIT(6)
#define QI_PARA_TX_CAP_SUPPORT_EXTRA_MASK     BIT(7)

#define QI_PARA_TX_TYPE_FAC_BASE              0x20 /* tx for factory */
#define QI_PARA_TX_TYPE_FAC_MAX               0x3F
#define QI_PARA_TX_TYPE_CAR_BASE              0x40 /* tx for car */
#define QI_PARA_TX_TYPE_CAR_MAX               0x5F
#define QI_PARA_TX_EXT_TYPE_CAR               0x4 /* bit[3:2]=01b */
#define QI_PARA_TX_TYPE_PWR_BANK_BASE         0x60 /* tx for power bank */
#define QI_PARA_TX_TYPE_PWR_BANK_MAX          0x7F
#define QI_PARA_TX_EXT_TYPE_PWR_BANK          0x8 /* bit[3:2]=10b */

/* cmd 0x43 */
#define QI_PARA_CHARGE_STATE_LEN              1
#define QI_CHRG_STATE_FULL                    BIT(0)
#define QI_CHRG_STATE_DONE                    BIT(1)

/* cmd 0x44 */
#define QI_PARA_TX_FOP_LEN                    2
#define QI_FIXED_FOP_MAX                      148
#define QI_FIXED_FOP_MIN                      120

/* cmd 0x48 */
#define QI_PARA_FOD_STATUS_LEN                2

/* cmd 0x49 */
#define QI_PARA_TX_EXT_CAP_LEN                5
#define QI_PARA_TX_EXT_CAP_S                  1
#define QI_PARA_TX_EXT_CAP_E                  4
#define QI_PARA_TX_EXT_CAP_SUPPORT_FAN_MASK   BIT(2)
#define QI_PARA_TX_EXT_CAP_SUPPORT_TEC_MASK   BIT(3)
#define QI_PARA_TX_EXT_CAP_SUPPORT_QVAL_MASK  BIT(4)

/* cmd 0x4a */
#define QI_TX_ALARM_SRC                       0
#define QI_TX_ALARM_PLIM                      1

/* cmd 0x4d */
#define QI_ACK_EPT_TYPE_LEN                   3
#define QI_ACK_EPT_TYPE_S_OFFSET              1
#define QI_ACK_EPT_TYPE_E_OFFSET              2

/* cmd 0x69 */
#define QI_PARA_FAN_SPEED_LIMIT_LEN           1

/* cmd 0x6b */
#define QI_ACK_RPP_FORMAT_LEN                 2
#define QI_ACK_RPP_FORMAT_OFFSET              1
#define QI_ACK_RPP_FORMAT_8BIT                0
#define QI_ACK_RPP_FORMAT_24BIT               1

/* cmd 0x6c */
#define QI_PARA_RX_PMAX_LEN                   1
#define QI_PARA_RX_PMAX_OFFSET                0
#define QI_PARA_RX_PMAX_UNIT                  5 /* 5w per bit, max 75w */
#define QI_PARA_RX_PMAX_SHIFT                 4 /* high 4 bits, 0b1111 xxxx */

/*
 * ask: rx to tx
 * byte[0]: header, byte[1]: command, byte[2~5]: data
 * byte[0]: cmd, byte[1~5]: data
 */
#define QI_ASK_PACKET_LEN                     6
#define QI_ASK_PACKET_HDR                     0
#define QI_ASK_PACKET_CMD                     1
#define QI_ASK_PACKET_DAT0                    1
#define QI_ASK_PACKET_DAT1                    2
#define QI_ASK_PACKET_DAT2                    3
#define QI_ASK_PACKET_DAT3                    4
#define QI_ASK_PACKET_DAT4                    5
#define QI_ASK_PACKET_DAT_LEN                 4

#define QI_ASK_PACKET_HDR_MSG_SIZE_1_BYTE     0x18
#define QI_ASK_PACKET_HDR_MSG_SIZE_2_BYTE     0x28
#define QI_ASK_PACKET_HDR_MSG_SIZE_3_BYTE     0x38
#define QI_ASK_PACKET_HDR_MSG_SIZE_4_BYTE     0x48
#define QI_ASK_PACKET_HDR_MSG_SIZE_5_BYTE     0x58

#define QI_PKT_LEN                            6
#define QI_PKT_HDR                            0
#define QI_PKT_CMD                            1
#define QI_PKT_DATA                           2 /* data: B2-B5 */

/* for wireless accessory */
#define QI_ACC_TX_DEV_MAC_LEN                 6
#define QI_ACC_TX_DEV_MODELID_LEN             3
#define QI_ACC_OFFSET0                        0
#define QI_ACC_OFFSET1                        1
#define QI_ACC_OFFSET2                        2
#define QI_ACC_OFFSET3                        3
#define QI_ACC_OFFSET4                        4
#define QI_ACC_OFFSET5                        5

/* for wireless tx_type */
#define QI_TX_TYPE_CP58                       0x8866
#define QI_TX_TYPE_CP60                       0x8867
#define QI_TX_TYPE_CP61                       0x8868
#define QI_TX_TYPE_CP39S                      0x8c68

enum qi_protocol_tx_cap_info {
	QI_TX_CAP_TYPE = 1,
	QI_TX_CAP_VOUT_MAX,
	QI_TX_CAP_IOUT_MAX,
	QI_TX_CAP_ATTR,
};

enum qi_protocol_tx_extra_cap_info {
	QI_TX_EXTRA_CAP_ATTR1 = 1,
	QI_TX_EXTRA_CAP_ATTR2,
	QI_TX_EXTRA_CAP_ATTR3,
	QI_TX_EXTRA_CAP_ATTR4,
};

struct qi_protocol_device_info {
	u8 tx_fwver[QI_ACK_TX_FWVER_LEN - 1];
	u8 tx_main_cap[QI_PARA_TX_CAP_LEN - 1];
	u8 tx_ext_cap[QI_PARA_TX_EXT_CAP_LEN - 1];
};

struct qi_protocol_acc_device_info {
	u8 dev_state;
	u8 dev_no;
	u8 dev_mac[QI_ACC_TX_DEV_MAC_LEN];
	u8 dev_model_id[QI_ACC_TX_DEV_MODELID_LEN];
	u8 dev_submodel_id;
	u8 dev_version;
	u8 dev_business;
	u8 dev_info_cnt;
};

struct qi_protocol_handle {
	int (*hdl_qi_ask_pkt)(void);
	int (*hdl_non_qi_ask_pkt)(void);
	void (*hdl_non_qi_fsk_pkt)(u8 *, int);
};

struct qi_protocol_ops {
	const char *chip_name;
	int (*send_msg)(u8 cmd, u8 *data, int len);
	int (*send_msg_with_ack)(u8 cmd, u8 *data, int len);
	int (*receive_msg)(u8 *data, int len);
	int (*send_fsk_msg)(u8 cmd, u8 *data, int len);
	int (*get_ask_packet)(u8 *data, int len);
	int (*get_chip_fw_version)(u8 *data, int len);
	int (*get_tx_id_pre)(void); /* process non protocol flow */
	int (*set_rpp_format_post)(u8 pmax); /* process non protocol flow */
};

struct qi_protocol_dev {
	struct device *dev;
	struct qi_protocol_device_info info;
	struct qi_protocol_acc_device_info acc_info;
	int dev_id;
	struct qi_protocol_ops *p_ops;
};

#ifdef CONFIG_WIRELESS_PROTOCOL_QI
int qi_protocol_ops_register(struct qi_protocol_ops *ops);
struct qi_protocol_handle *qi_protocol_get_handle(void);
#else
static inline int qi_protocol_ops_register(struct qi_protocol_ops *ops)
{
	return -1;
}

static inline struct qi_protocol_handle *qi_protocol_get_handle(void)
{
	return NULL;
}
#endif /* CONFIG_WIRELESS_PROTOCOL_QI */

#endif /* _WIRELESS_PROTOCOL_QI_H_ */
