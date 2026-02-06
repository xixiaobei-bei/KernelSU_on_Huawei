/*
 * nu1619.h
 *
 * nu1619 macro, addr etc.
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#ifndef _NU1619_H_
#define _NU1619_H_

#define NU1619_CHIP_ID                       0x1619
#define NU1619_ADDR_LEN                      2
#define NU1619_CMD_REG_DATA_LEN              2
#define NU1619_CHIP_CTRL_ADDR                0x000d
#define NU1619_GPIO_PWR_GOOD_VAL             1
#define NU1619_PWR_CTRL_AC                   0
#define NU1619_PWR_CTRL_EXTRA                1

/* chip_info */
#define NU1619_REG_CMD_CHIP_ID               0xb1
#define NU1619_REG_CMD_MTP_FW_VER            0x95
/* op mode register */
#define NU1619_OP_MODE_ADDR                  0x000b
#define NU1619_STANDALONE_MODE               0x01
#define NU1619_RX_MODE                       0x02
#define NU1619_TX_MODE                       0x03
/* nu_cmd: read reg */
#define NU1619_CMD_REG_BYTE_LEN              1
#define NU1619_CMD_REG_WORD_LEN              2
#define NU1619_CMD_READ_RETRY_CNT            3
#define NU1619_CMD_READ_REGS_SIZE            2
#define NU1619_CMD_READ_MASK                 0x80
/* nu_cmd: write reg */
#define NU1619_CMD_WRITE_REG                 0x000d
#define NU1619_CMD_WRITE_DATA1_REG           0x0000
#define NU1619_CMD_WRITE_DATA2_REG           0x0001
#define NU1619_CMD_WRITE_DATA3_REG           0x0002
#define NU1619_CMD_WRITE_DATA4_REG           0x0003
#define NU1619_CMD_WRITE_DATA5_REG           0x0004
#define NU1619_CMD_WRITE_DATA6_REG           0x0005
#define NU1619_CMD_WRITE_DATA7_REG           0x0006
#define NU1619_CMD_WRITE_DATA8_REG           0x0007
#define NU1619_CMD_WRITE_RETRY_CNT           10
#define NU1619_CMD_WRITE_MASK                0x20
/* nu_cmd: feed wd */
#define NU1619_REG_CMD_WD                    0x0c
#define NU1619_WD_SET_VAL                    0x0f02
#define NU1619_WD_KICK_VAL                   0x0f03
/* rx to tx message */
#define NU1619_REG_CMD_RX2TX_HEADER          0x87
#define NU1619_REG_CMD_RX2TX_CMD             0x88
#define NU1619_REG_CMD_RX2TX_DATA            0x89
#define NU1619_RX2TX_PKT_LEN                 6
#define NU1619_RX2TX_DATA_LEN                4
#define NU1619_RX2TX_BUFF_LEN                8
#define NU1619_RX2TX_STR_LEN                 64
#define NU1619_REG_CMD_SEND_MSG              0x01
#define NU1619_CMD_ACK                       0xff
/* tx to rx message */
#define NU1619_REG_CMD_TX2RX_HEADER          0x87
#define NU1619_REG_CMD_TX2RX_CMD             0x88
#define NU1619_REG_CMD_TX2RX_DATA            0x89
#define NU1619_TX2RX_PKT_LEN                 6
#define NU1619_TX2RX_DATA_LEN                4

/*
 * nu1619 rx mode
 */

/* intr_latch register */
#define NU1619_RX_INT_ADDR                   0x0023
#define NU1619_RX_INT_OCP                    BIT(0)
#define NU1619_RX_INT_OVP                    BIT(1)
#define NU1619_RX_INT_OTP                    BIT(2)
#define NU1619_RX_INT_TXDATA_RCVD            BIT(4)
#define NU1619_RX_INT_READY                  BIT(6)
#define NU1619_RX_INT_LDO_OFF                BIT(7)
#define NU1619_RX_INT_POWER_ON               BIT(10)
#define NU1619_RX_INT_LDO_ON                 BIT(11)
#define NU1619_RX_INT_SEND_PKT_TIMEOUT       BIT(12)
#define NU1619_RX_INT_SEND_PKT_SUCC          BIT(13)
/* intr_clr register */
#define NU1619_REG_CMD_RX_INT_CLR            0x08
#define NU1619_RX_INT_CLR_OCP                BIT(0)
#define NU1619_RX_INT_CLR_OVP                BIT(1)
#define NU1619_RX_INT_CLR_OTP                BIT(2)
#define NU1619_RX_INT_CLR_TXDATA_RCVD        BIT(4)
#define NU1619_RX_INT_CLR_RX_READY           BIT(6)
#define NU1619_RX_INT_CLR_LDO_OFF            BIT(7)
#define NU1619_RX_INT_CLR_PWR_ON             BIT(10)
#define NU1619_RX_INT_CLR_LDO_ON             BIT(11)
#define NU1619_RX_INT_CLR_SEND_PKT_TIMEOUT   BIT(12)
#define NU1619_RX_INT_CLR_SEND_PKT_SUCC      BIT(13)
#define NU1619_ALL_INTR_CLR                  0xffff
/* intr_en register */
#define NU1619_REG_CMD_RX_INT_EN             0x12
#define NU1619_RX_INT_EN_OCP                 BIT(0)
#define NU1619_RX_INT_EN_OVP                 BIT(1)
#define NU1619_RX_INT_EN_OTP                 BIT(2)
#define NU1619_RX_INT_EN_TXDATA_RCVD         BIT(4)
#define NU1619_RX_INT_EN_RX_READY            BIT(6)
#define NU1619_RX_INT_EN_LDO_OFF             BIT(7)
#define NU1619_RX_INT_EN_PWR_ON              BIT(10)
#define NU1619_RX_INT_EN_LDO_ON              BIT(11)
#define NU1619_RX_INT_EN_SEND_PKT_TIMEOUT    BIT(12)
#define NU1619_RX_INT_EN_SEND_PKT_SUCC       BIT(13)
#define NU1619_ALL_INTR_EN                   0xffff
/* nu_cmd: rx */
#define NU1619_REG_CMD_SET_RX_VOUT           0x02
#define NU1619_RX_VOUT_MAX                   22000
#define NU1619_RX_VOUT_MIN                   2800

#define NU1619_REG_CMD_RX_FOD_WR1            0x06
#define NU1619_REG_CMD_RX_FOD_WR2            0x07
#define NU1619_RX_FOD_LEN                    12
#define NU1619_RX_FOD_TMP_STR_LEN            4
#define NU1619_RX_FOD_SET_LEN                6

#define NU1619_REG_CMD_SEND_FC               0x09
#define NU1619_RX_FC_RETRY_CNT               3
#define NU1619_BPCP_SLEEP_TIME               50
#define NU1619_BPCP_TIMEOUT                  200
#define NU1619_RX_FC_TIMEOUT                 1500
#define NU1619_RX_FC_SLEEP_TIME              50
#define NU1619_RX_FC_DEF_VOLT                5000
#define NU1619_RX_FC_ERR_LTH                 500 /* lower th */
#define NU1619_RX_FC_ERR_UTH                 1000 /* upper th */

#define NU1619_REG_CMD_RPP                   0x0a
#define NU1619_RX_RPP_VAL_UNIT               2 /* in 0.5Watt units */
#define NU1619_RX_RPP_VAL_MASK               0x3f /* bit0-5 */

#define NU1619_REG_CMD_CHIP_RESET            0x0f
#define NU1619_REG_CMD_RX_LDO_CFG            0x10
#define NU1619_LDO_CFG_LEN                   7
#define NU1619_REG_CMD_RX_CEP_VAL            0x94

#define NU1619_REG_CMD_RX_SS                 0x99
#define NU1619_RX_SS_MIN                     0
#define NU1619_RX_SS_MAX                     255

#define NU1619_REG_CMD_SEND_EPT              0x05
#define NU1619_REG_CMD_RX_VRECT              0x9d
#define NU1619_REG_CMD_RX_VOUT               0x9f
#define NU1619_REG_CMD_RX_IOUT               0xa3
#define NU1619_REG_CMD_RX_TEMP               0xa5

#define NU1619_REG_CMD_FREQ                  0xa7
#define NU1619_RX_FREQ_COUNT_VAL             23680
#define NU1619_REG_CMD_RX_VOUT_REG           0xa9
#define NU1619_REG_CMD_TX_VOUT_REG           0xb9
#define NU1619_REG_CMD_RX_FOD_RD             0xbf
#define NU1619_TRX_CHIP_TEMP                 50

/*
 * nu1619 tx mode
 */

/* intr_latch register */
#define NU1619_TX_INT_ADDR                   0x0023
#define NU1619_TX_INT_EPT_TYPE               BIT(0)
#define NU1619_TX_INT_START_DPING            BIT(1)
#define NU1619_TX_INT_GET_SS                 BIT(2)
#define NU1619_TX_INT_GET_ID                 BIT(3)
#define NU1619_TX_INT_GET_CFG                BIT(4)
#define NU1619_TX_INT_GET_PPP                BIT(5)
#define NU1619_TX_INT_GET_DPING              BIT(6)
#define NU1619_TX_INT_TX_INIT                BIT(7)
#define NU1619_TX_INT_SET_VTX                BIT(8)
/* intr_clr register */
#define NU1619_REG_CMD_TX_INT_CLR            0x04
#define NU1619_TX_INT_CLR_EPT_TYPE           BIT(0)
#define NU1619_TX_INT_CLR_START_DPING        BIT(1)
#define NU1619_TX_INT_CLR_GET_SS             BIT(2)
#define NU1619_TX_INT_CLR_GET_ID             BIT(3)
#define NU1619_TX_INT_CLR_GET_CFG            BIT(4)
#define NU1619_TX_INT_CLR_GET_PPP            BIT(5)
#define NU1619_TX_INT_CLR_GET_DPING          BIT(6)
#define NU1619_TX_INT_CLR_TX_INIT            BIT(7)
/* intr_en register */
#define NU1619_REG_CMD_TX_INT_EN             0x07
#define NU1619_TX_INT_EN_EPT_TYPE            BIT(0)
#define NU1619_TX_INT_EN_START_DPING         BIT(1)
#define NU1619_TX_INT_EN_GET_SS              BIT(2)
#define NU1619_TX_INT_EN_GET_ID              BIT(3)
#define NU1619_TX_INT_EN_GET_CFG             BIT(4)
#define NU1619_TX_INT_EN_GET_PPP             BIT(5)
#define NU1619_TX_INT_EN_GET_DPING           BIT(6)
#define NU1619_TX_INT_EN_TX_INIT             BIT(7)
/* tx ept type register */
#define NU1619_REG_CMD_TX_EPT                0xaf
#define NU1619_REG_CMD_TX_EPT_CLR            0x0b
#define NU1619_TX_EPT_CMD                    BIT(0)
#define NU1619_TX_EPT_SS                     BIT(1)
#define NU1619_TX_EPT_ID                     BIT(2)
#define NU1619_TX_EPT_XID                    BIT(3)
#define NU1619_TX_EPT_CFG_COUNT_ERR          BIT(4)
#define NU1619_TX_EPT_PCH                    BIT(5)
#define NU1619_TX_EPT_FIRSTCEP               BIT(6)
#define NU1619_TX_EPT_TIMEOUT                BIT(7)
#define NU1619_TX_EPT_CEP_TIMEOUT            BIT(8)
#define NU1619_TX_EPT_RPP_TIMEOUT            BIT(9)
#define NU1619_TX_EPT_OCP                    BIT(10)
#define NU1619_TX_EPT_OVP                    BIT(11)
#define NU1619_TX_EPT_LVP                    BIT(12)
#define NU1619_TX_EPT_FOD                    BIT(13)
#define NU1619_TX_EPT_OTP                    BIT(14)
#define NU1619_TX_EPT_POCP                   BIT(15)
/* nu_cmd: tx */
#define NU1619_REG_CMD_SET_TX_FOD            0x02
#define NU1619_TX_FOD_LEN                    6
#define NU1619_REG_CMD_BST_CFG               0x06
#define NU1619_PS_TX_VOLT_5V5                5500
#define NU1619_PS_TX_VOLT_6V8                6800
#define NU1619_PS_TX_VOLT_10V                10000
#define NU1619_PS_TX_GPIO_PU                 0x01
#define NU1619_PS_TX_GPIO_OPEN               0x00
#define NU1619_PS_TX_GPIO_PD                 0x02

#define NU1619_REG_CMD_SET_TX_MIN_FOP        0x05
#define NU1619_REG_CMD_EN_TX_MODE            0x07
#define NU1619_ENTER_TX_MODE_VAL             0x20

#define NU1619_REG_CMD_SET_TX_MAX_FOP        0x08
#define NU1619_REG_CMD_SET_TX_PING_FREQ      0x09
#define NU1619_TX_PING_FREQUENCY_MIN         113 /* kHz */
#define NU1619_TX_PING_FREQUENCY_MAX         148 /* kHz */

#define NU1619_REG_CMD_SET_TX_PING_INTERVAL  0x0a
#define NU1619_TX_PING_INTERVAL_MIN          200 /* ms */
#define NU1619_TX_PING_INTERVAL_MAX          1000 /* ms */
#define NU1619_TX_PING_INTERVAL_INIT         500 /* 500ms unit: 10ms */
#define NU1619_TX_PING_INTERVAL_STEP         10

#define NU1619_REG_CMD_TX_OCP                0x0c
#define NU1619_TX_OCP_VAL                    2000
#define NU1619_REG_CMD_TX_OVP                0x0d
#define NU1619_TX_OVP_VAL                    12000
#define NU1619_REG_CMD_TX_FOD_EN             0x0e
#define NU1619_TX_FOD_EN_VAL                 1

#define NU1619_REG_CMD_TX_PING_INTERVAL      0x99
#define NU1619_REG_CMD_TX_VRECT              0x9d
#define NU1619_REG_CMD_TX_VIN                0x9f
#define NU1619_REG_CMD_TX_IIN                0xa3
#define NU1619_REG_CMD_TX_TEMP               0xa5
#define NU1619_REG_CMD_TX_FOP                0xa7
#define NU1619_TX_PING_HCLK                  100000
#define NU1619_TX_MIN_FOP_VAL                111
#define NU1619_TX_MAX_FOP_VAL                148

#define NU1619_REG_CMD_TX_MIN_FOP            0xa9
#define NU1619_REG_CMD_TX_MAX_FOP            0xab
#define NU1619_REG_CMD_TX_PINGFRE            0xad
#define NU1619_TX_PING_FREQUENCY_INIT        125

/*
 * nu1619 mtp
 */

/* mtp addr */
#define NU1619_MTP_WRITE_HEAD_ADDR           0x0010
#define NU1619_MTP_BOOT_HEAD_ADDR            0x0
#define NU1619_MTP_RX_HEAD_ADDR              0x1
#define NU1619_MTP_TX_HEAD_ADDR              0x13
#define NU1619_MTP_DOWNLOAD_ADDR             0x0012
#define NU1619_MTP_LOAD_BYTE_RETRY_CNT       100
#define NU1619_MTP_DOWNLOAD_LEN              4
#define NU1619_MTP_DATA_INDEX_3              3
#define NU1619_MTP_DATA_INDEX_2              2
#define NU1619_MTP_DATA_INDEX_1              1
#define NU1619_MTP_I2C_ADDR                  0x61
#define NU1619_NORMAL_I2C_ADDR               0x62
/* mtp dtm mode */
#define NU1619_DTM_REG0_ADDR                 0x2017
#define NU1619_DTM_REG0_VAL0                 0x69
#define NU1619_DTM_REG0_VAL1                 0x96
#define NU1619_DTM_REG0_VAL2                 0x66
#define NU1619_DTM_REG0_VAL3                 0x99
#define NU1619_DTM_REG0_VAL4                 0x55
#define NU1619_DTM_REG1_ADDR                 0x2018
#define NU1619_DTM_REG1_VAL                  0xff
#define NU1619_DTM_REG2_ADDR                 0x2019
#define NU1619_DTM_REG2_VAL                  0xff
#define NU1619_DTM_REG3_ADDR                 0x0001
#define NU1619_DTM_REG3_VAL                  0x5a
#define NU1619_DTM_REG4_ADDR                 0x0003
#define NU1619_DTM_REG4_VAL                  0xa5
/* mtp write mode */
#define NU1619_MTP_WM_REG0                   0x1000
#define NU1619_MTP_WM_REG0_VAL               0x10
#define NU1619_MTP_WM_REG1                   0x1130
#define NU1619_MTP_WM_REG1_VAL               0x3e
#define NU1619_MTP_WM_REG2                   0x0017
#define NU1619_MTP_WM_REG2_VAL               0x01
#define NU1619_MTP_WM_REG3                   0x1000
#define NU1619_MTP_WM_REG3_VAL               0x30
#define NU1619_MTP_WM_REG4                   0x001a
#define NU1619_MTP_WM_REG4_VAL               0x5a
#define NU1619_MTP_EXIT_VAL                  0x00
/* mtp version */
#define NU1619_MTP_STATUS_UNKNOWN            0
#define NU1619_MTP_STATUS_GOOD               1
#define NU1619_MTP_STATUS_BAD                2
#define NU1619_ENTER_FW_CHECK_MODE_VAL       0x21
#define NU1619_MTP_CHIP_ID_ADDR              0x0023
#define NU1619_MTP_FW_VER_ADDR               0x0020
#define NU1619_MTP_FW_CRC_ADDR               0x0008
#define NU1619_MTP_FW_VER_LEN                3
#define NU1619_MTP_FW_CHIP_CODE_LEN          1
#define NU1619_MTP_FW_CRC_LEN                4
#define NU1619_MTP_FW_VERSION                0x41411
#define NU1619_MTP_FW_VER_BOOT               0x04
#define NU1619_MTP_FW_VER_RX                 0x14
#define NU1619_MTP_FW_VER_TX                 0x11
#define NU1619_MTP_FW_CRC_VAL                0x1666666

#define NU1619_SHUTDOWN_SLEEP_TIME           200
#define NU1619_RCV_MSG_SLEEP_TIME            100
#define NU1619_RCV_MSG_SLEEP_CNT             10
#define NU1619_WAIT_FOR_ACK_RETRY_CNT        5
#define NU1619_WAIT_FOR_ACK_SLEEP_TIME       100
#define NU1619_SNED_MSG_RETRY_CNT            2

struct nu1619_read_reg {
	u16 cmd_addr;
	u16 data_addr;
	bool busy_flag;
};

enum nu1619_mtp_fw_type {
	NU1619_MTP_BOOT = 0,
	NU1619_MTP_RX,
	NU1619_MTP_TX,
};

enum nu1619_cmd_reg {
	NU1619_CMD_HEAD = 0,
	NU1619_CMD_DATA0,
	NU1619_CMD_DATA1,
	NU1619_CMD_DATA_TOTAL,
};

struct nu1619_chip_info {
	u16 chip_id;
	u32 mtp_fw_ver;
};

struct nu1619_dev_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct work_struct mtp_check_work;
	struct mutex mutex_irq;
	u8 rx_fod_5v[NU1619_RX_FOD_LEN];
	u8 rx_fod_9v[NU1619_RX_FOD_LEN];
	u8 rx_fod_9v_cp39s_hk[NU1619_RX_FOD_LEN];
	u8 rx_fod_12v[NU1619_RX_FOD_LEN];
	u8 rx_fod_15v[NU1619_RX_FOD_LEN];
	u8 rx_fod_15v_cp39s_hk[NU1619_RX_FOD_LEN];
	u8 tx_fod_coef[NU1619_TX_FOD_LEN];
	u8 rx_ldo_cfg_5v[NU1619_LDO_CFG_LEN];
	u8 rx_ldo_cfg_9v[NU1619_LDO_CFG_LEN];
	u8 rx_ldo_cfg_12v[NU1619_LDO_CFG_LEN];
	u8 rx_ldo_cfg_sc[NU1619_LDO_CFG_LEN];
	unsigned int ic_type;
	int mtp_status;
	u32 mtp_fw_ver;
	u32 mtp_check_sum;
	int max_iout;
	int rx_ss_good_lth;
	int gpio_en;
	int gpio_en_valid_val;
	int gpio_sleep_en;
	int gpio_int;
	int gpio_pwr_good;
	int gpio_pwr_ctrl;
	int irq_int;
	int irq_cnt;
	u16 irq_val;
	u16 ept_type;
	u16 chip_id;
	bool irq_active;
};

#endif /* _NU1619_H_ */
