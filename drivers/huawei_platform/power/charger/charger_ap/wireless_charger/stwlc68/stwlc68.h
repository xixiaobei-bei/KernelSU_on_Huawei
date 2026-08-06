/*
 * stwlc68.h
 *
 * stwlc68 macro, addr etc.
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

#ifndef _STWLC68_H_
#define _STWLC68_H_

#define STWLC68_ADDR_LEN                      2
#define STWLC68_4ADDR_LEN                     4
#define STWLC68_4ADDR_F_LEN                   5 /* len with flag */
#define STWLC68_4ADDR_FLAG                    0xFA
#define STWLC68_CHIP_ID                       68 /* 0x0044 */

/* chip_info: 0x0000 ~ 0x000C */
#define STWLC68_CHIP_INFO_ADDR                0x0000
#define STWLC68_CHIP_INFO_LEN                 14
#define STWLC68_CHIP_INFO_STR_LEN             128
/* chip id register */
#define STWLC68_CHIP_ID_ADDR                  0x0000
#define STWLC68_CHIP_ID_LEN                   2
/* chip rev register */
#define STWLC68_CHIP_REV_ADDR                 0x0002
#define STWLC68_CHIP_REV_LEN                  1
/* cust id register */
#define STWLC68_CUST_ID_ADDR                  0x0003
#define STWLC68_CUST_ID_LEN                   1
/* fw id register */
#define STWLC68_FW_ID_ADDR                    0x0004
#define STWLC68_FW_ID_LEN                     2
/* svn rev register */
#define STWLC68_SVN_REV_ADDR                  0x0006
#define STWLC68_SVN_REV_LEN                   2
/* config id register */
#define STWLC68_CFG_ID_ADDR                   0x0008
#define STWLC68_CFG_ID_LEN                    2
/* pe id register */
#define STWLC68_PE_ID_ADDR                    0x000A
#define STWLC68_PE_ID_LEN                     2
/* patch id register */
#define STWLC68_PATCH_ID_ADDR                 0x000C
#define STWLC68_PATCH_ID_LEN                  2
/* op mode register */
#define STWLC68_OP_MODE_ADDR                  0x000E
#define STWLC68_OP_MODE_LEN                   1
#define STWLC68_STANDALONE_MODE               0x01 /* debug */
#define STWLC68_RX_MODE                       0x02
#define STWLC68_TX_MODE                       0x03
/* op sub mode register */
#define STWLC68_OP_SUB_MODE_ADDR              0x000F
#define STWLC68_OP_SUB_MODE_LEN               1
/* intr_en register */
#define STWLC68_INTR_EN_ADDR                  0x0010
#define STWLC68_INTR_EN_LEN                   2
#define STWLC68_OVTP_INTR_EN                  BIT(0)
#define STWLC68_OCP_INTR_EN                   BIT(1)
#define STWLC68_OVP_INTR_EN                   BIT(2)
#define STWLC68_SYS_ERR_INTR_EN               BIT(3)
#define STWLC68_RX_RCVD_MSG_INTR_EN           BIT(5)
#define STWLC68_OUTPUT_ON_INTR_EN             BIT(6)
#define STWLC68_OUTPUT_OFF_INTR_EN            BIT(7)
#define STWLC68_SEND_PKT_SUCC_INTR_EN         BIT(8)
#define STWLC68_SEND_PKT_TIMEOUT_INTR_EN      BIT(9)
/* intr_clr register */
#define STWLC68_INTR_CLR_ADDR                 0x0012
#define STWLC68_INTR_CLR_LEN                  2
#define STWLC68_OVTP_INTR_CLR                 BIT(0)
#define STWLC68_OCP_INTR_CLR                  BIT(1)
#define STWLC68_OVP_INTR_CLR                  BIT(2)
#define STWLC68_SYS_ERR_INTR_CLR              BIT(3)
#define STWLC68_RX_RCVD_MSG_INTR_CLR          BIT(5)
#define STWLC68_OUTPUT_ON_INTR_CLR            BIT(6)
#define STWLC68_OUTPUT_OFF_INTR_CLR           BIT(7)
#define STWLC68_SEND_PKT_SUCC_INTR_CLR        BIT(8)
#define STWLC68_SEND_PKT_TIMEOUT_INTR_CLR     BIT(9)
#define STWLC68_ALL_INTR_CLR                  0xFFFF
/* intr_latch register */
#define STWLC68_INTR_LATCH_ADDR               0x0014
#define STWLC68_INTR_LATCH_LEN                2
#define STWLC68_OVTP_INTR_LATCH               BIT(0)
#define STWLC68_OCP_INTR_LATCH                BIT(1)
#define STWLC68_OVP_INTR_LATCH                BIT(2)
#define STWLC68_SYS_ERR_INTR_LATCH            BIT(3)
#define STWLC68_RX_RCVD_MSG_INTR_LATCH        BIT(5)
#define STWLC68_OUTPUT_ON_INTR_LATCH          BIT(6)
#define STWLC68_OUTPUT_OFF_INTR_LATCH         BIT(7)
#define STWLC68_SEND_PKT_SUCC_INTR_LATCH      BIT(8)
#define STWLC68_SEND_PKT_TIMEOUT_INTR_LATCH   BIT(9)
#define STWLC68_RX_STATUS_POWER_ON            BIT(10)
#define STWLC68_RX_STATUS_READY               BIT(11)
#define STWLC68_RX_STATUS_TX2RX_ACK           BIT(5)
/* stat register */
#define STWLC68_STAT_ADDR                     0x0016
#define STWLC68_STAT_LEN                      2
#define STWLC68_OVTP_STAT                     BIT(0)
#define STWLC68_OCP_STAT                      BIT(1)
#define STWLC68_OVP_STAT                      BIT(2)
#define STWLC68_SYS_ERR_STAT                  BIT(3)
#define STWLC68_RX_RCVD_MSG_STAT              BIT(5)
#define STWLC68_OUTPUT_ON_STAT                BIT(6)
#define STWLC68_OUTPUT_OFF_STAT               BIT(7)
#define STWLC68_SEND_PKT_SUCC_STAT            BIT(8)
#define STWLC68_SEND_PKT_TIMEOUT_STAT         BIT(9)
/* ovtp_src_latch register */
#define STWLC68_OVTP_SRC_LATCH_ADDR           0x0018
#define STWLC68_OVTP_SRC_LATCH_LEN            1
/* ovp_src_latch register */
#define STWLC68_OVP_SRC_LATCH_ADDR            0x0019
#define STWLC68_OVP_SRC_LATCH_LEN             1
/* ocp_src_latch register */
#define STWLC68_OCP_SRC_LATCH_ADDR            0x001A
#define STWLC68_OCP_SRC_LATCH_LEN             1
/* sys_err_latch register */
#define STWLC68_SYS_ERR_LATCH_ADDR            0x001C
#define STWLC68_SYS_ERR_LATCH_LEN             4
/* cmd register */
#define STWLC68_CMD_ADDR                      0x0020
#define STWLC68_CMD_LEN                       2
#define STWLC68_CMD_SEND_MSG                  BIT(0)
#define STWLC68_CMD_SEND_MSG_WAIT_RPLY        BIT(1)
#define STWLC68_CMD_SEND_EPT                  BIT(2)
#define STWLC68_CMD_SEND_FC                   BIT(10)
/* vout register */
#define STWLC68_VOUT_ADDR                     0x0030
#define STWLC68_VOUT_LEN                      2
/* vrect register */
#define STWLC68_VRECT_ADDR                    0x0032
#define STWLC68_VRECT_LEN                     2
/* iout register */
#define STWLC68_IOUT_ADDR                     0x0034
#define STWLC68_IOUT_LEN                      2
/* chip_temp register, in degC */
#define STWLC68_CHIP_TEMP_ADDR                0x0036
#define STWLC68_CHIP_TEMP_LEN                 2
/* op_freq register, in kHZ */
#define STWLC68_OP_FREQ_ADDR                  0x0038
#define STWLC68_OP_FREQ_LEN                   2
/* rx_signal_strength register */
#define STWLC68_RX_SS_ADDR                    0x003A
#define STWLC68_RX_SS_LEN                     1
#define STWLC68_RX_SS_MIN                     0
#define STWLC68_RX_SS_MAX                     255
/* rx_vout_set register */
#define STWLC68_RX_VOUT_SET_ADDR              0x003C
#define STWLC68_RX_VOUT_SET_LEN               2
#define STWLC68_RX_VOUT_SET_STEP              25 /* mV */
#define STWLC68_RX_VOUT_MAX                   22000
#define STWLC68_RX_VOUT_MIN                   0
/* vout register */
#define STWLC68_RX_GET_VOUT_ADDR              0x003c
#define STWLC68_RX_VOUT_VALUE_MAX             12600 /* mV */
#define STWLC68_RX_VOUT_REG_MAX               4095
#define STWLC68_RX_SET_VOUT_ADDR              0x003e
/* rx_vrect_adj register */
#define STWLC68_RX_VRECT_ADJ_ADDR             0x003E
#define STWLC68_RX_VRECT_ADJ_LEN              1
/* rx_ilim_set register */
#define STWLC68_RX_ILIM_SET_ADDR              0x003F
#define STWLC68_RX_ILIM_SET_LEN               1
/* rx_clamp_volt register */
#define STWLC68_RX_VCLAMP_ADDR                0x0040
#define STWLC68_RX_VCLAMP_LEN                 1
/* send_msg_data register */
#define STWLC68_RX2TX_HEADER_ADDR             0x0050
#define STWLC68_RX2TX_CMD_ADDR                0x0051
#define STWLC68_RX2TX_DATA_ADDR               0x0052
#define STWLC68_RX2TX_DATA_LEN                4
/* rx2tx: bit(0): header;  bit(1) : command;  bit(2,3,4,5) : data */
#define STWLC68_RX2TX_PKT_LEN                 6
/* rcvd_msg_data register */
#define STWLC68_RCVD_HEADER_ADDR              0x0059
#define STWLC68_RCVD_CMD_ADDR                 0x005A
#define STWLC68_RCVD_DATA_ADDR                0x005B
#define STWLC68_RCVD_MSG_DATA_LEN             5
#define STWLC68_RCVD_DATA_LEN                 4
/* ept_msg register */
#define STWLC68_EPT_MSG_ADDR                  0x0062
#define STWLC68_EPT_MSG_LEN                   1
#define STWLC68_RX_EPT_UNKOWN                 0x00
#define STWLC68_EPT_ERR_VRECT                 0xA0
#define STWLC68_EPT_ERR_VOUT                  0xA1
/* tx_pwr_transferred_to_rx register */
#define STWLC68_TX_PWR_TRANS_TO_RX_ADDR       0x0070
#define STWLC68_TX_PWR_TRANS_TO_RX_LEN        2
/* tx_pwr_received_by_rx register */
#define STWLC68_TX_PWR_RCV_BY_RX_ADDR         0x0072
#define STWLC68_TX_PWR_RCV_BY_RX_LEN          2
/* tx_pwr_efficiency register */
#define STWLC68_TX_PWR_EFF_ADDR               0x0074
#define STWLC68_TX_PWR_EFF_LEN                1
/* tx_recent_cep register */
#define STWLC68_TX_RECENT_CEP_ADDR            0x0075
#define STWLC68_TX_RECENT_CEP_LEN             1
/* tx_oper_freq register */
#define STWLC68_TX_OPER_FREQ_ADDR             0x0076
#define STWLC68_TX_OPER_FREQ_LEN              1
/* rx_ctrl_err register */
#define STWLC68_RX_CTRL_ERR_ADDR              0x0090
#define STWLC68_RX_CTRL_ERR_LEN               2
/* rx_rcvd_pwr register,  in mW */
#define STWLC68_RX_RCVD_PWR_ADDR              0x0092
#define STWLC68_RX_RCVD_PWR_LEN               2
/* rx_fdc_dec_dbg register */
#define STWLC68_RX_FDC_DEC_DBG_ADDR           0x0094
#define STWLC68_RX_FDC_DEC_DBG_LEN            1
/* rx_fsk_smpl_cnt register */
#define STWLC68_RX_FSK_SMPL_CNT_ADDR          0x0096
#define STWLC68_RX_FSK_SMPL_CNT_LEN           2
/* rx_fsk_smpl_0 register */
#define STWLC68_RX_FSK_SMPL_0_ADDR            0x0098
#define STWLC68_RX_FSK_SMPL_0_LEN             2
/* rx_fsk_smpl_1 register */
#define STWLC68_RX_FSK_SMPL_1_ADDR            0x009A
#define STWLC68_RX_FSK_SMPL_1_LEN             2
/* rx_fsk_smpl_2 register */
#define STWLC68_RX_FSK_SMPL_2_ADDR            0x009C
#define STWLC68_RX_FSK_SMPL_2_LEN             2
/* rx_ptc_gua_pwr register */
#define STWLC68_RX_PTC_GUA_PWR_ADDR           0x00A0
#define STWLC68_RX_PTC_GUA_PWR_LEN            1
/* rx_ptc_max_pwr register */
#define STWLC68_RX_PTC_MAX_PWR_ADDR           0x00A1
#define STWLC68_RX_PTC_MAX_PWR_LEN            1
/* rx_ptc_rp_hdr register */
#define STWLC68_RX_PTC_RP_HDR_ADDR            0x00A2
#define STWLC68_RX_PTC_RP_HDR_LEN             1
/* rx_ptc_fsk_cfg register */
#define STWLC68_RX_PTC_FSK_CFG_ADDR           0x00A3
#define STWLC68_RX_PTC_FSK_CFG_LEN            1
/* fc_volt register */
#define STWLC68_FC_VOLT_ADDR                  0x00C0
#define STWLC68_FC_VOLT_LEN                   2
#define STWLC68_FC_VOUT_SLEEP_TIME            50
#define STWLC68_FC_VOUT_TIMEOUT               5000
#define STWLC68_FC_VOUT_ERR_LTH               500 /* lower th */
#define STWLC68_FC_VOUT_ERR_UTH               1000 /* upper th */
/* wdt_timeout register, in ms */
#define STWLC68_WDT_TIMEOUT_ADDR              0x00C6
#define STWLC68_WDT_TIMEOUT_LEN               2
#define STWLC68_WDT_MEOUT                     1000
/* wdt_feed register */
#define STWLC68_WDT_FEED_ADDR                 0x00C8
#define STWLC68_WDT_FEED_LEN                  1
/* mi2c frq register */
#define STWLC68_MI2C_FRQ_ADDR                 0x00DE
#define STWLC68_MI2C_FRQ_VAL                  0x1E0F


/* i2c ctrl register */
#define STWLC68_MI2C_CMD_ADDR                 0x00E0 /* cmd address */
#define STWLC68_MI2C_CMD_WR                   BIT(0)
#define STWLC68_MI2C_CMD_RD                   BIT(1)
#define STWLC68_MI2C_CMD_HIGH                 BIT(2)
#define STWLC68_MI2C_CMD_LOW                  BIT(3)
#define STWLC68_MI2C_SLAVE_ADDR               0x00E2 /* slave address */
#define STWLC68_MI2C_SLAVE_ADDR_VAL           0x1B
#define STWLC68_MI2C_REG_ADDR                 0x00E3 /* reg address */
#define STWLC68_MI2C_REG_LEN                  0x00E5 /* 0:1byte;1:2byte */
#define STWLC68_MI2C_REG_1BYTE                0
#define STWLC68_MI2C_REG_2BYTE                1
#define STWLC68_MI2C_DATA_LEN                 0x00E6 /* data len */
#define STWLC68_MI2C_DATA                     0x00E7 /* data address */
#define STWLC68_MI2C_WRD_DELAY_TIME           50 /* ms */
/* charger-pump register */
#define CP_I2C_INIT_ADDR                      0x61
#define CP_I2C_INIT_RST                       0x00
#define CP_I2C_INIT_CLR                       0x80
#define CP_PMID_VLIMIT_ADDR                   0x00
#define CP_PMID_VLIMIT_VAL                    0xff
#define CP_ILIMIT_ADDR                        0x01
#define CP_ILIMIT_VAL                         0xd2
#define CP_FRQ_ADDR                           0x02
#define CP_FRQ_VAL                            0x44
#define CP_PMID_OV_ADDR                       0x03
#define CP_PMID_OV_VAL                        0x81

#define STWLC68_CP_OPEN                       1
#define STWLC68_SC_OPEN                       2
#define STWLC68_CP_CLOSE                      3


/* program otp */
#define STWLC68_FW_ADDR_LEN                   1
#define STWLC68_RST_ADDR                      0x2001C00C
#define STWLC68_RST_SYS                       BIT(0)
#define STWLC68_RST_M0                        BIT(1)
#define STWLC68_RST_OTP                       BIT(4)
#define STWLC68_CUT_ID_ADDR                   0x2001C002
#define STWLC68_CLEAN_ADDR_REV0               0x0005052C
#define STWLC68_CLEAN_ADDR_REV1               0x00050540
#define STWLC68_OTP_ENABLE_ADDR               0x2001C04F
#define STWLC68_OTP_ENABLE                    0x01
#define STWLC68_OTP_PROGRAM_ADDR              0x2001C050
#define STWLC68_OTP_UNLOCK_CODE               0x7B
#define STWLC68_OTP_LOCK_CODE                 0x00
#define STWLC68_FW_I2C_ADDR                   0x2001C014
#define STWLC68_FW_I2C_DISABLE                0x01
#define STWLC68_RD_ROM_OTP_PULSE_ADDR         0x000680EC
#define STWLC68_RD_ROM_OTP_PULSE_VAL          0x13
#define STWLC68_CLEAN_OTP_PULSE_ADDR          0x000680F0
#define STWLC68_CLEAN_OTP_PULSE_VAL           0x5B
#define STWLC68_RD_VERIFY1_OTP_ADDR           0x000680F4
#define STWLC68_RD_VERIFY1_OTP_VAL            0x2E
#define STWLC68_RD_VERIFY2_OTP_ADDR           0x000680F8
#define STWLC68_RD_VERIFY2_OTP_VAL            0x2E
#define STWLC68_RD_USRCMP_OTP_ADDR            0x000680FC
#define STWLC68_RD_USRCMP_OTP_VAL             0x13
#define STWLC68_RD_MISSION_OTP_ADDR           0x00068100
#define STWLC68_RD_MISSION_OTP_VAL            0x13
#define STWLC68_RD_PGM_LO_OTP_ADDR            0x00068104
#define STWLC68_RD_PGM_LO_OTP_VAL             0x29
#define STWLC68_RD_PGM_HI_OTP_ADDR            0x00068108
#define STWLC68_RD_PGM_HI_OTP_VAL             0x23
#define STWLC68_RD_SOAK_OTP_LO_ADDR           0x0006810C
#define STWLC68_RD_SOAK_OTP_LO_VAL            0xA1
#define STWLC68_RD_SOAK_OTP_HI_ADDR           0x00068110
#define STWLC68_RD_SOAK_OTP_HI_VAL            0x8C
#define STWLC68_OTP_FREQ_CTRL_ADDR            0x00068004
#define STWLC68_OTP_FREQ_CTRL_VAL             0x0F

#define STWLC68_CUT_ID_V10                    0x01 /* 1.0 */
#define STWLC68_CUT_ID_V11                    0x02 /* 1.1 */
#define STWLC68_OTP_MAX_SIZE                  (16 * 1024)
#define STWLC68_FW_OP_MODE_SA                 0x01
#define STWLC68_OTP_I2C_CHUNK_SIZE            4
#define STWLC68_OTP_OPS_DELAY                 50 /* ms */

/* command */
#define STWLC68_CMD_SET_TX_FOP                0x03 /* FOP operate frequency */
#define STWLC68_CMD_GET_TX_FOP                0x04
#define STWLC68_CMD_GET_TX_VERSION            0x05
#define STWLC68_CMD_GET_TX_IIN                0x06
#define STWLC68_CMD_GET_TX_VIN                0x07
#define STWLC68_CMD_GET_TX_ADAPTER_TYPE       0x0b
#define STWLC68_CMD_SEND_READY                0x0F
#define STWLC68_CMD_SEND_SN                   0x12
#define STWLC68_CMD_SEND_BATT_TEMP            0x16
#define STWLC68_CMD_SEND_BATT_CAPACITY        0x17
#define STWLC68_CMD_SET_CURRENT_LIMIT         0x18
#define STWLC68_CMD_START_SAMPLE              0x1d
#define STWLC68_CMD_STOP_SAMPLE               0x1e
#define STWLC68_CMD_RX_BOOST_SUCC             0x1f
#define STWLC68_CMD_CERT_SUCC                 0x20
#define STWLC68_CMD_CERT_FAIL                 0x21
#define STWLC68_CMD_START_CERT                0x36
#define STWLC68_CMD_GET_HASH                  0x38
#define STWLC68_CMD_GET_TX_ID                 0x3b
#define STWLC68_CMD_GET_TX_CAP                0x41
#define STWLC68_CMD_GET_TX_EXTRA_CAP          0x49
#define STWLC68_CMD_SEND_CHRG_STATE           0x43
#define STWLC68_CMD_FIX_TX_FOP                0x44
#define STWLC68_CMD_UNFIX_TX_FOP              0x45
#define STWLC68_CMD_SEND_QVAL                 0x48
#define STWLC68_CMD_ACK                       0xff
#define STWLC68_CMD_ACK_HEAD                  0x1e

/* cmd: 0x41 */
#define STWLC68_TX_CAP_VOUT_STEP              100
#define STWLC68_TX_CAP_IOUT_STEP              100
#define STWLC68_TX_CAP_CABLE_OK_MASK          BIT(0)
#define STWLC68_TX_CAP_CAN_BOOST_MASK         BIT(1)
#define STWLC68_TX_CAP_NO_NEED_CERT_MASK      BIT(4)
#define STWLC68_TX_CAP_SUPPORT_SCP_MASK       BIT(5)
#define STWLC68_TX_CAP_SUPPORT_12V_MASK       BIT(6)
#define STWLC68_TX_CAP_SUPPORT_EXTRA_BIT_MASK BIT(7)

/* cmd: 0x49 */
#define STWLC68_TX_CAP_SUPPORT_FAN_MASK       BIT(2)
#define STWLC68_TX_CAP_SUPPORT_TEC_MASK       BIT(3)
#define STWLC68_TX_CAP_SUPPORT_QVAL_MASK      BIT(4)

#define STWLC68_TX_FOP_LEN                    2
#define STWLC68_FIXED_FOP_MAX                 148
#define STWLC68_FIXED_FOP_MIN                 120
#define STWLC68_TX_ADAPTER_TYPE_LEN           2
#define STWLC68_TX_ID_LEN                     3
#define STWLC68_LIMIT_CURRENT_TIME            500
#define STWLC68_BATT_CAPACITY_LEN             2
#define STWLC68_BATT_TEMP_LEN                 1
#define STWLC68_BATT_TEMP_MAX                 127
#define STWLC68_CHRG_STATE_LEN                1
#define STWLC68_QVAL_LEN                      2
#define STWLC68_SHUTDOWN_SLEEP_TIME           200
#define STWLC68_RCV_MSG_SLEEP_TIME            100
#define STWLC68_RCV_MSG_SLEEP_CNT             10
#define STWLC68_WAIT_FOR_ACK_SLEEP_TIME       100
#define STWLC68_WAIT_FOR_ACK_RETRY_CNT        5
#define STWLC68_SNED_MSG_RETRY_CNT            2
#define STWLC68_GET_TX_PARA_RETRY_CNT         2
#define STWLC68_TX_FW_VERSION_LEN             4
#define STWLC68_TX_FW_VERSION_STRING_LEN      32
#define STWLC68_RX_TMP_BUFF_LEN               32

struct stwlc68_chip_info {
	u16 chip_id;
	u8 chip_rev;
	u8 cust_id;
	u16 fw_id;
	u16 svn_rev;
	u16 cfg_id;
	u16 pe_id;
	u16 patch_id;
	u8 cut_id;
};

struct stwlc68_dev_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct work_struct cp_work; /* cp: charge-pump */
	struct mutex mutex_irq;
	int cp_state;
	int rx_ss_good_lth;
	int tx_fod_th_5v;
	int gpio_en;
	int gpio_en_valid_val;
	int gpio_sleep_en;
	int gpio_int;
	int irq_int;
	int irq_active;
	u16 irq_val;
	u16 ept_type;
	int irq_cnt;
	int sc_rx_vrmax_gap;
};

#endif /* _STWLC68_H_ */
