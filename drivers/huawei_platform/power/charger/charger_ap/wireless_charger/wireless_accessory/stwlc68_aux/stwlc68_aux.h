/*
 * stwlc68_aux.h
 *
 * stwlc68_aux macro, addr etc.
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

#ifndef _STWLC68_AUX_H_
#define _STWLC68_AUX_H_

#define STWLC68_AUX_ADDR_LEN                      2
#define STWLC68_AUX_4ADDR_LEN                     4
#define STWLC68_AUX_4ADDR_F_LEN                   5 /* len with flag */
#define STWLC68_AUX_4ADDR_FLAG                    0xFA
#define STWLC68_AUX_IRQ_INT_VALID_VAL             0

/* chip_info: 0x0000 ~ 0x000C */
#define STWLC68_AUX_CHIP_INFO_ADDR                0x0000
#define STWLC68_AUX_CHIP_INFO_LEN                 14
#define STWLC68_AUX_CHIP_INFO_STR_LEN             128
/* chip id register */
#define STWLC68_AUX_CHIP_ID_ADDR                  0x0000
#define STWLC68_AUX_CHIP_ID_LEN                   2
#define STWLC68_AUX_CHIP_ID                       68 /* 0x0044 */
/* config id register */
#define STWLC68_AUX_CFG_ID_ADDR                   0x0008
#define STWLC68_AUX_CFG_ID_LEN                    2
/* patch id register */
#define STWLC68_AUX_PATCH_ID_ADDR                 0x000C
#define STWLC68_AUX_PATCH_ID_LEN                  2
/* op mode register */
#define STWLC68_AUX_OP_MODE_ADDR                  0x000E
#define STWLC68_AUX_OP_MODE_LEN                   1
#define STWLC68_AUX_STANDALONE_MODE               0x01 /* debug */
#define STWLC68_AUX_RX_MODE                       0x02
#define STWLC68_AUX_TX_MODE                       0x03
#define STWLC68_AUX_ALL_INTR_CLR                  0xFFFF
#define STWLC68_AUX_ABNORMAL_INTR                 0xFFFF
/* tx_intr_clr register */
#define STWLC68_AUX_TX_INTR_CLR_ADDR              0x0028
#define STWLC68_AUX_TX_INTR_CLR_LEN               2
#define STWLC68_AUX_TX_INTR_CLR_CNT               2
/* tx_intr_latch register */
#define STWLC68_AUX_TX_INTR_LATCH_ADDR            0x002A
#define STWLC68_AUX_TX_INTR_LATCH_LEN             2
#define STWLC68_AUX_TX_OVTP_INTR_LATCH            BIT(0)
#define STWLC68_AUX_TX_OCP_INTR_LATCH             BIT(1)
#define STWLC68_AUX_TX_OVP_INTR_LATCH             BIT(2)
#define STWLC68_AUX_TX_SYS_ERR_INTR_LATCH         BIT(3)
#define STWLC68_AUX_TX_EXT_MON_INTR_LATCH         BIT(6)
#define STWLC68_AUX_TX_RP_DM_TIMEOUT_LATCH        BIT(7)
#define STWLC68_AUX_TX_SEND_PKT_SUCC_LATCH        BIT(8)
#define STWLC68_AUX_TX_SEND_PKT_TIMEOUT_LATCH     BIT(9)
#define STWLC68_AUX_TX_EPT_PKT_RCVD_LATCH         BIT(10)
#define STWLC68_AUX_TX_START_PING_LATCH           BIT(11)
#define STWLC68_AUX_TX_SS_PKG_RCVD_LATCH          BIT(12)
#define STWLC68_AUX_TX_ID_PCKET_RCVD_LATCH        BIT(13)
#define STWLC68_AUX_TX_CFG_PKT_RCVD_LATCH         BIT(14)
#define STWLC68_AUX_TX_PP_PKT_RCVD_LATCH          BIT(15)
/* cmd register */
#define STWLC68_AUX_CMD_ADDR                      0x0020
#define STWLC68_AUX_CMD_LEN                       2
#define STWLC68_AUX_CMD_VAL                       1
#define STWLC68_AUX_CMD_SEND_MSG                  BIT(0)
#define STWLC68_AUX_CMD_SEND_MSG_SHIFT            0
#define STWLC68_AUX_CMD_SEND_MSG_WAIT_RPLY        BIT(1)
#define STWLC68_AUX_CMD_SEND_MSG_WAIT_RPLY_SHIFT  1
#define STWLC68_AUX_CMD_SEND_EPT                  BIT(2)
#define STWLC68_AUX_CMD_SEND_EPT_SHIFT            2
#define STWLC68_AUX_CMD_SW2TX                     BIT(6)
#define STWLC68_AUX_CMD_SW2TX_SHIFT               6
#define STWLC68_AUX_CMD_SW2RX                     BIT(7)
#define STWLC68_AUX_CMD_SW2RX_SHIFT               7
#define STWLC68_AUX_CMD_VOUT_ON                   BIT(8)
#define STWLC68_AUX_CMD_VOUT_ON_SHIFT             8
#define STWLC68_AUX_CMD_SEND_FC                   BIT(10)
#define STWLC68_AUX_CMD_SEND_FC_SHIFT             10
#define STWLC68_AUX_CMD_TX_EN                     BIT(11)
#define STWLC68_AUX_CMD_TX_EN_SHIFT               11
#define STWLC68_AUX_CMD_TX_DIS                    BIT(12)
#define STWLC68_AUX_CMD_TX_DIS_SHIFT              12
#define STWLC68_AUX_CMD_TX_FOD_EN                 BIT(13)
#define STWLC68_AUX_CMD_TX_FOD_EN_SHIFT           13
/* vout register */
#define STWLC68_AUX_VOUT_ADDR                     0x0030
#define STWLC68_AUX_VOUT_LEN                      2
/* tx vin register */
#define STWLC68_AUX_TX_VIN_ADDR                   0x0030
#define STWLC68_AUX_TX_VIN_LEN                    2
/* vrect register */
#define STWLC68_AUX_VRECT_ADDR                    0x0032
#define STWLC68_AUX_VRECT_LEN                     2
/* tx vrect register */
#define STWLC68_AUX_TX_VRECT_ADDR                 0x0032
#define STWLC68_AUX_TX_VRECT_LEN                  2
/* iout register */
#define STWLC68_AUX_IOUT_ADDR                     0x0034
#define STWLC68_AUX_IOUT_LEN                      2
/* tx iin register */
#define STWLC68_AUX_TX_IIN_ADDR                   0x0034
#define STWLC68_AUX_TX_IIN_LEN                    2
/* chip_temp register, in degC */
#define STWLC68_AUX_CHIP_TEMP_ADDR                0x0036
#define STWLC68_AUX_CHIP_TEMP_LEN                 2
/* tx_oper_freq register, in kHZ */
#define STWLC68_AUX_TX_OP_FREQ_ADDR               0x0038
#define STWLC68_AUX_TX_OP_FREQ_LEN                2
/* send_msg_data register */
#define STWLC68_AUX_SEND_MSG_HEADER_ADDR          0x0050
#define STWLC68_AUX_SEND_MSG_CMD_ADDR             0x0051
#define STWLC68_AUX_SEND_MSG_DATA_ADDR            0x0052
#define STWLC68_AUX_SEND_MSG_DATA_LEN             4
/* send_msg: bit(0):header, bit(1):cmd, bit(2,3,4,5):data */
#define STWLC68_AUX_SEND_MSG_PKT_LEN              6
/* rcvd_msg_data register */
#define STWLC68_AUX_RCVD_MSG_HEADER_ADDR          0x0059
#define STWLC68_AUX_RCVD_MSG_CMD_ADDR             0x005A
#define STWLC68_AUX_RCVD_MSG_DATA_ADDR            0x005B
/* rcvd_msg: bit(0):header, bit(1):cmd, bit(2,3,4,5):data */
#define STWLC68_AUX_RCVD_MSG_PKT_LEN              6
#define STWLC68_AUX_RCVD_MSG_DATA_LEN             4
#define STWLC68_AUX_RCVD_PKT_BUFF_LEN             8
#define STWLC68_AUX_RCVD_PKT_STR_LEN              64
/* ept_msg register */
#define STWLC68_AUX_EPT_MSG_ADDR                  0x0062
#define STWLC68_AUX_EPT_MSG_LEN                   1
#define STWLC68_AUX_RX_EPT_UNKOWN                 0x00
#define STWLC68_AUX_EPT_ERR_VRECT                 0xA0
#define STWLC68_AUX_EPT_ERR_VOUT                  0xA1
/* tx rp demodulation timeout register */
#define STWLC68_AUX_TX_RP_TIMEOUT_ADDR            0x0064
/* tx fod coef register */
#define STWLC68_AUX_TX_PLOSS_TH_ADDR              0x0065
#define STWLC68_AUX_TX_PLOSS_TH_VAL               78
#define STWLC68_AUX_TX_PLOSS_TH_UNIT              32 /* mW */
#define STWLC68_AUX_TX_PLOSS_CNT_ADDR             0x0066
#define STWLC68_AUX_TX_PLOSS_CNT_VAL              4
/* tx ping interval */
#define STWLC68_AUX_TX_PING_INTERVAL              0x006A
#define STWLC68_AUX_TX_PING_INTERVAL_INIT         500 /* ms */
/* tx otp register */
#define STWLC68_AUX_TX_OTP_ADDR                   0x006D
#define STWLC68_AUX_TX_OTP_THRES                  80
/* tx ocp register */
#define STWLC68_AUX_TX_OCP_ADDR                   0x006E
#define STWLC68_AUX_TX_OCP_VAL                    2000 /* mA */
#define STWLC68_AUX_TX_OCP_STEP                   10 /* mA */
/* tx ovp register */
#define STWLC68_AUX_TX_OVP_ADDR                   0x006F
#define STWLC68_AUX_TX_OVP_VAL                    12000 /* mV */
#define STWLC68_AUX_TX_OVP_STEP                   100 /* mV */
/* tx_pwr_transferred_to_rx register */
#define STWLC68_AUX_TX_PWR_TRANS_TO_RX_ADDR       0x0070
#define STWLC68_AUX_TX_PWR_TRANS_TO_RX_LEN        2
/* tx_pwr_received_by_rx register */
#define STWLC68_AUX_TX_PWR_RCV_BY_RX_ADDR         0x0072
#define STWLC68_AUX_TX_PWR_RCV_BY_RX_LEN          2
/* tx_pwr_efficiency register */
#define STWLC68_AUX_TX_PWR_EFF_ADDR               0x0074
#define STWLC68_AUX_TX_PWR_EFF_LEN                1
/* tx_recent_cep register */
#define STWLC68_AUX_TX_RECENT_CEP_ADDR            0x0075
#define STWLC68_AUX_TX_RECENT_CEP_LEN             1
/* tx set oper_freq */
#define STWLC68_AUX_TX_SET_OPER_FREQ_ADDR         0x0078
#define STWLC68_AUX_TX_OPER_FRE_UNIT              1000
/* tx max fop */
#define STWLC68_AUX_TX_MAX_FOP_ADDR               0x007A
#define STWLC68_AUX_TX_MAX_FOP_VAL                142 /* kHz */
/* tx min fop */
#define STWCL68_TX_MIN_FOP_ADDR                   0x007B
#define STWLC68_AUX_TX_MIN_FOP_VAL                120 /* kHz */
/* tx ping freq */
#define STWLC68_AUX_TX_PING_FREQ_ADDR             0x007C
#define STWLC68_AUX_TX_PING_FREQ_UNIT             1000
#define STWLC68_AUX_TX_PING_FREQ_INIT             135 /* kHz */
#define STWLC68_AUX_TX_PING_FREQ_MIN              100 /* kHz */
#define STWLC68_AUX_TX_PING_FREQ_MAX              150 /* kHz */
/* tx ept type register */
#define STWLC68_AUX_TX_EPT_REASON_RCVD_ADDR       0x00D0
#define STWLC68_AUX_TX_EPT_REASON_PKG_LEN         2
#define STWLC68_AUX_TX_EPT_SRC_SS                 BIT(1)
#define STWLC68_AUX_TX_EPT_SRC_ID                 BIT(2)
#define STWLC68_AUX_TX_EPT_SRC_XID                BIT(3)
#define STWLC68_AUX_TX_EPT_SRC_CFG_CNT_ERR        BIT(4)
#define STWLC68_AUX_TX_EPT_SRC_CFG_ERR            BIT(5)
#define STWLC68_AUX_TX_EPT_SRC_CEP_TIMEOUT        BIT(8)
#define STWLC68_AUX_TX_EPT_SRC_RPP_TIMEOUT        BIT(9)
#define STWLC68_AUX_TX_EPT_SRC_FOD                BIT(13)
#define STWLC68_AUX_TX_EPT_SRC_OTP                BIT(14)
/* sram start_addr register */
#define STWLC68_AUX_SRAM_START_ADDR_ADDR          0x0140
#define STWLC68_AUX_SRAM_START_ADDR_LEN           4
/* sram exe register */
#define STWLC68_AUX_EXE_SRAM_ADDR                 0x0145
#define STWLC68_AUX_EXE_SRAM_LEN                  1
#define STWLC68_AUX_EXE_SRAM_CMD                  0x10

#define STWLC68_AUX_SW2TX_RETRY_SLEEP_TIME        25 /* ms */
#define STWLC68_AUX_SW2TX_RETRY_TIME              1500 /* ms */

/* fw addr */
#define STWLC68_AUX_FW_ADDR_LEN                   1
#define STWLC68_AUX_RST_ADDR                      0x2001C00C
#define STWLC68_AUX_RST_SYS                       BIT(0)
#define STWLC68_AUX_RST_M0                        BIT(1)
#define STWLC68_AUX_RST_OTP                       BIT(4)
#define STWLC68_AUX_CUT_ID_ADDR                   0x2001C002
#define STWLC68_AUX_CLEAN_ADDR_REV0               0x0005052C
#define STWLC68_AUX_CLEAN_ADDR_REV1               0x00050540
#define STWLC68_AUX_OTP_ENABLE_ADDR               0x2001C04F
#define STWLC68_AUX_OTP_ENABLE                    0x01
#define STWLC68_AUX_OTP_PROGRAM_ADDR              0x2001C050
#define STWLC68_AUX_OTP_UNLOCK_CODE               0x7B
#define STWLC68_AUX_OTP_LOCK_CODE                 0x00
#define STWLC68_AUX_FW_I2C_ADDR                   0x2001C014
#define STWLC68_AUX_FW_I2C_DISABLE                0x01
#define STWLC68_AUX_RD_ROM_OTP_PULSE_ADDR         0x000680EC
#define STWLC68_AUX_RD_ROM_OTP_PULSE_VAL          0x13
#define STWLC68_AUX_CLEAN_OTP_PULSE_ADDR          0x000680F0
#define STWLC68_AUX_CLEAN_OTP_PULSE_VAL           0x5B
#define STWLC68_AUX_RD_VERIFY1_OTP_ADDR           0x000680F4
#define STWLC68_AUX_RD_VERIFY1_OTP_VAL            0x2E
#define STWLC68_AUX_RD_VERIFY2_OTP_ADDR           0x000680F8
#define STWLC68_AUX_RD_VERIFY2_OTP_VAL            0x2E
#define STWLC68_AUX_RD_USRCMP_OTP_ADDR            0x000680FC
#define STWLC68_AUX_RD_USRCMP_OTP_VAL             0x13
#define STWLC68_AUX_RD_MISSION_OTP_ADDR           0x00068100
#define STWLC68_AUX_RD_MISSION_OTP_VAL            0x13
#define STWLC68_AUX_RD_PGM_LO_OTP_ADDR            0x00068104
#define STWLC68_AUX_RD_PGM_LO_OTP_VAL             0x29
#define STWLC68_AUX_RD_PGM_HI_OTP_ADDR            0x00068108
#define STWLC68_AUX_RD_PGM_HI_OTP_VAL             0x23
#define STWLC68_AUX_RD_SOAK_OTP_LO_ADDR           0x0006810C
#define STWLC68_AUX_RD_SOAK_OTP_LO_VAL            0xA1
#define STWLC68_AUX_RD_SOAK_OTP_HI_ADDR           0x00068110
#define STWLC68_AUX_RD_SOAK_OTP_HI_VAL            0x8C
#define STWLC68_AUX_OTP_FREQ_CTRL_ADDR            0x00068004
#define STWLC68_AUX_OTP_FREQ_CTRL_VAL             0x0E
#define STWLC68_AUX_CHIP_RESET_ADDR               0x2001C00c
#define STWLC68_AUX_CHIP_RESET                    0x01
#define STWLC68_AUX_OTP_VPP_ADDR                  0x00068158
#define STWLC68_AUX_OTP_VPP_6V_VAL                0x06
#define STWLC68_AUX_OTP_SKIP_SOAK_ADDR            0x00068038
#define STWLC68_AUX_OTP_SKIP_SOAK_VAL             0x3D

#define STWLC68_AUX_LDO5V_EN_ADDR                 0x2001C19B
#define STWLC68_AUX_LDO5V_EN                      1
#define STWLC68_AUX_LDO5V_DIS                     0

#define STWLC68_AUX_CUT_ID_V10                    0x01 /* 1.0 */
#define STWLC68_AUX_CUT_ID_V11                    0x02 /* 1.1 */
#define STWLC68_AUX_FW_OP_MODE_SA                 0x01
#define STWLC68_AUX_OTP_OPS_DELAY                 100 /* ms */
#define STWLC68_AUX_ILOAD_DRIVE_ADDR              0x2001C176
#define STWLC68_AUX_ILOAD_DISABLE_VALUE           0x01
#define STWLC68_AUX_ILOAD_DATA_LEN                0x01

/* command */
#define STWLC68_AUX_CMD_START_SAMPLE              0x1d
#define STWLC68_AUX_CMD_STOP_SAMPLE               0x1e
#define STWLC68_AUX_CMD_ACK                       0xff
#define STWLC68_AUX_CMD_ACK_HEAD                  0x1e

#define STWLC68_AUX_TX_ID_LEN                     3
#define STWLC68_AUX_LIMIT_CURRENT_TIME            500
#define STWLC68_AUX_BATT_CAPACITY_LEN             2
#define STWLC68_AUX_BATT_TEMP_LEN                 1
#define STWLC68_AUX_BATT_TEMP_MAX                 127
#define STWLC68_AUX_SHUTDOWN_SLEEP_TIME           200
#define STWLC68_AUX_RCV_MSG_SLEEP_TIME            100
#define STWLC68_AUX_RCV_MSG_SLEEP_CNT             10
#define STWLC68_AUX_WAIT_FOR_ACK_SLEEP_TIME       100
#define STWLC68_AUX_WAIT_FOR_ACK_RETRY_CNT        5
#define STWLC68_AUX_SNED_MSG_RETRY_CNT            2
#define STWLC68_AUX_GET_TX_PARA_RETRY_CNT         2
#define STWLC68_AUX_RX_TMP_BUFF_LEN               32

struct stwlc68_aux_chip_info {
	u16 chip_id;
	u8 chip_rev;
	u8 cust_id;
	u16 sram_id;
	u16 svn_rev;
	u16 cfg_id;
	u16 pe_id;
	u16 patch_id;
	u8 cut_id;
};

struct stwlc68_aux_global_val {
	struct qi_protocol_handle *qi_hdl;
};

struct stwlc68_aux_dev_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct mutex mutex_irq;
	struct stwlc68_aux_global_val g_val;
	unsigned int ic_type;
	int gpio_en;
	int gpio_en_valid_val;
	int gpio_int;
	int irq_int;
	int irq_active;
	u16 irq_val;
	u16 ept_type;
	u16 tx_ping_interval;
	struct delayed_work sram_scan_work;
};

#endif /* _STWLC68_AUX_H_ */
