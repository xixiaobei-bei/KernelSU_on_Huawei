/*
 * stwlc68_chip.h
 *
 * stwlc68 registers, chip info, etc.
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _STWLC68_CHIP_H_
#define _STWLC68_CHIP_H_

#define STWLC68_ADDR_LEN                      2
#define STWLC68_4ADDR_LEN                     4
#define STWLC68_4ADDR_F_LEN                   5 /* len with flag */
#define STWLC68_4ADDR_FLAG                    0xFA
#define STWLC68_GPIO_PWR_GOOD_VAL             1
#define STWLC68_IRQ_INT_VALID_VAL             0

/* chip_info: 0x0000 ~ 0x000C */
#define STWLC68_CHIP_INFO_ADDR                0x0000
#define STWLC68_CHIP_INFO_LEN                 14
/* chip id register */
#define STWLC68_CHIP_ID_ADDR                  0x0000
#define STWLC68_CHIP_ID_LEN                   2
#define STWLC68_CHIP_ID                       68 /* 0x0044 */
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
#define STWLC68_ALL_RX_INTR_ENABLED           0xFFFF
/* intr_clr register */
#define STWLC68_INTR_CLR_ADDR                 0x0012
#define STWLC68_INTR_CLR_LEN                  2
#define STWLC68_INTR_CLR_CNT                  2
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
#define STWLC68_ABNORMAL_INTR                 0xFFFF
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
/* tx_intr_en register */
#define STWLC68_TX_INTR_EN_ADDR               0x0026
#define STWLC68_TX_INTR_EN_LEN                2
#define STWLC68_TX_OVTP_INTR_EN               BIT(0)
#define STWLC68_TX_OCP_INTR_EN                BIT(1)
#define STWLC68_TX_OVP_INTR_EN                BIT(2)
#define STWLC68_TX_SYS_ERR_INTR_EN            BIT(3)
#define STWLC68_TX_SEND_PKT_SUCC_EN           BIT(8)
#define STWLC68_TX_SEND_PKT_TIMEOUT_EN        BIT(9)
#define STWLC68_TX_EPT_PKT_RCVD_EN            BIT(10)
#define STWLC68_TX_START_PING_EN              BIT(11)
#define STWLC68_TX_SS_PKT_RCVD_EN             BIT(12)
#define STWLC68_TX_ID_PKT_RCVD_EN             BIT(13)
#define STWLC68_TX_CFG_PKT_RCVD_EN            BIT(14)
#define STWLC68_TX_PP_PKT_RCVD_EN             BIT(15)
#define STWLC68_ALL_TX_INTR_ENABLED           0xFFFF
/* tx_intr_clr register */
#define STWLC68_TX_INTR_CLR_ADDR              0x0028
#define STWLC68_TX_INTR_CLR_LEN               2
#define STWLC68_TX_INTR_CLR_CNT               2
#define STWLC68_TX_OVTP_INTR_CLR              BIT(0)
#define STWLC68_TX_OCP_INTR_CLR               BIT(1)
#define STWLC68_TX_OVP_INTR_CLR               BIT(2)
#define STWLC68_TX_SYS_ERR_INTR_CLR           BIT(3)
#define STWLC68_TX_SEND_PKT_SUCC_CLR          BIT(8)
#define STWLC68_TX_SEND_PKT_TIMEOUT_CLR       BIT(9)
#define STWLC68_TX_EPT_PKT_RCVD_CLR           BIT(10)
#define STWLC68_TX_START_PING_CLR             BIT(11)
#define STWLC68_TX_SS_PKT_RCVD_CLR            BIT(12)
#define STWLC68_TX_ID_PKT_RCVD_CLR            BIT(13)
#define STWLC68_TX_CFG_PKT_RCVD_CLR           BIT(14)
#define STWLC68_TX_PP_PKT_RCVD_CLR            BIT(15)
#define STWLC68_ALL_TX_INTR_CLR               0xFFFF
/* tx_intr_latch register */
#define STWLC68_TX_INTR_LATCH_ADDR            0x002A
#define STWLC68_TX_INTR_LATCH_LEN             2
#define STWLC68_TX_OVTP_INTR_LATCH            BIT(0)
#define STWLC68_TX_OCP_INTR_LATCH             BIT(1)
#define STWLC68_TX_OVP_INTR_LATCH             BIT(2)
#define STWLC68_TX_SYS_ERR_INTR_LATCH         BIT(3)
#define STWLC68_TX_EXT_MON_INTR_LATCH         BIT(6)
#define STWLC68_TX_RP_DM_TIMEOUT_LATCH        BIT(7)
#define STWLC68_TX_SEND_PKT_SUCC_LATCH        BIT(8)
#define STWLC68_TX_SEND_PKT_TIMEOUT_LATCH     BIT(9)
#define STWLC68_TX_EPT_PKT_RCVD_LATCH         BIT(10)
#define STWLC68_TX_START_PING_LATCH           BIT(11)
#define STWLC68_TX_SS_PKG_RCVD_LATCH          BIT(12)
#define STWLC68_TX_ID_PCKET_RCVD_LATCH        BIT(13)
#define STWLC68_TX_CFG_PKT_RCVD_LATCH         BIT(14)
#define STWLC68_TX_PP_PKT_RCVD_LATCH          BIT(15)
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
#define STWLC68_CMD_VAL                       1
#define STWLC68_CMD_SEND_MSG                  BIT(0)
#define STWLC68_CMD_SEND_MSG_SHIFT            0
#define STWLC68_CMD_SEND_MSG_WAIT_RPLY        BIT(1)
#define STWLC68_CMD_SEND_MSG_WAIT_RPLY_SHIFT  1
#define STWLC68_CMD_SEND_EPT                  BIT(2)
#define STWLC68_CMD_SEND_EPT_SHIFT            2
#define STWLC68_CMD_SW2TX                     BIT(6)
#define STWLC68_CMD_SW2TX_SHIFT               6
#define STWLC68_CMD_SW2RX                     BIT(7)
#define STWLC68_CMD_SW2RX_SHIFT               7
#define STWLC68_CMD_VOUT_ON                   BIT(8)
#define STWLC68_CMD_VOUT_ON_SHIFT             8
#define STWLC68_CMD_SEND_FC                   BIT(10)
#define STWLC68_CMD_SEND_FC_SHIFT             10
#define STWLC68_CMD_TX_EN                     BIT(11)
#define STWLC68_CMD_TX_EN_SHIFT               11
#define STWLC68_CMD_TX_DIS                    BIT(12)
#define STWLC68_CMD_TX_DIS_SHIFT              12
#define STWLC68_CMD_TX_FOD_EN                 BIT(13)
#define STWLC68_CMD_TX_FOD_EN_SHIFT           13
/* vout register */
#define STWLC68_VOUT_ADDR                     0x0030
#define STWLC68_VOUT_LEN                      2
/* tx vin register */
#define STWLC68_TX_VIN_ADDR                   0x0030
#define STWLC68_TX_VIN_LEN                    2
/* vrect register */
#define STWLC68_VRECT_ADDR                    0x0032
#define STWLC68_VRECT_LEN                     2
/* tx vrect register */
#define STWLC68_TX_VRECT_ADDR                 0x0032
#define STWLC68_TX_VRECT_LEN                  2
/* iout register */
#define STWLC68_IOUT_ADDR                     0x0034
#define STWLC68_IOUT_LEN                      2
/* tx iin register */
#define STWLC68_TX_IIN_ADDR                   0x0034
#define STWLC68_TX_IIN_LEN                    2
/* chip_temp register, in degC */
#define STWLC68_CHIP_TEMP_ADDR                0x0036
#define STWLC68_CHIP_TEMP_LEN                 2
/* op_freq register, in kHZ */
#define STWLC68_OP_FREQ_ADDR                  0x0038
#define STWLC68_OP_FREQ_LEN                   2
/* tx_oper_freq register, in kHZ */
#define STWLC68_TX_OP_FREQ_ADDR               0x0038
#define STWLC68_TX_OP_FREQ_LEN                2
/* rx_signal_strength register */
#define STWLC68_RX_SS_ADDR                    0x003A
#define STWLC68_RX_SS_LEN                     1
#define STWLC68_RX_SS_MIN                     0
#define STWLC68_RX_SS_MAX                     150
/* rx_vout_set register */
#define STWLC68_RX_VOUT_SET_ADDR              0x003C
#define STWLC68_RX_VOUT_SET_LEN               2
#define STWLC68_RX_VOUT_SET_STEP              25 /* mV */
#define STWLC68_RX_VOUT_MAX                   20000
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
/* ask mod config: bit7 exe, bit6-0: pos, 1: neg, bit5: mode_clmp, bit4: mod comm */
#define STWLC68_ASK_CFG_ADDR                  0x0042
#define STWLC68_ASK_CFG_LEN                   1
#define STWLC68_MOD_C_NEGATIVE                0xD0
#define STWLC68_BOTH_CAP_POSITIVE             0xB0
/* send_msg_data register */
#define STWLC68_SEND_MSG_HEADER_ADDR          0x0050
#define STWLC68_SEND_MSG_CMD_ADDR             0x0051
#define STWLC68_SEND_MSG_DATA_ADDR            0x0052
#define STWLC68_SEND_MSG_DATA_LEN             4
/* send_msg: bit(0): header, bit(1): cmd, bit(2,3,4,5): data */
#define STWLC68_SEND_MSG_PKT_LEN              6
/* rcvd_msg_data register */
#define STWLC68_RCVD_MSG_HEADER_ADDR          0x0059
#define STWLC68_RCVD_MSG_CMD_ADDR             0x005A
#define STWLC68_RCVD_MSG_DATA_ADDR            0x005B
/* rcvd_msg: bit(0): header, bit(1): cmd, bit(2,3,4,5): data */
#define STWLC68_RCVD_MSG_PKT_LEN              6
#define STWLC68_RCVD_MSG_DATA_LEN             4
#define STWLC68_RCVD_PKT_BUFF_LEN             8
#define STWLC68_RCVD_PKT_STR_LEN              64
/* ept_msg register */
#define STWLC68_EPT_MSG_ADDR                  0x0062
#define STWLC68_EPT_MSG_LEN                   1
#define STWLC68_RX_EPT_UNKOWN                 0x00
#define STWLC68_EPT_ERR_VRECT                 0xA0
#define STWLC68_EPT_ERR_VOUT                  0xA1
/* tx rp demodulation timeout register */
#define STWLC68_TX_RP_TIMEOUT_ADDR            0x0064
/* tx fod coef register */
#define STWLC68_TX_PLOSS_TH_ADDR              0x0065
#define STWLC68_TX_PLOSS_TH_UNIT              32 /* mW */
#define STWLC68_TX_PLOSS_CNT_ADDR             0x0066
/* tx ping interval */
#define STWLC68_TX_PING_INTERVAL              0x006A
#define STWLC68_TX_PING_INTERVAL_STEP         10
#define STWLC68_TX_PING_INTERVAL_MIN          200 /* ms */
#define STWLC68_TX_PING_INTERVAL_MAX          1000 /* ms */
#define STWLC68_TX_PING_INTERVAL_INIT         500 /* ms */
/* tx otp register */
#define STWLC68_TX_OTP_ADDR                   0x006D
#define STWLC68_TX_OTP_THRES                  80
/* tx ocp register */
#define STWLC68_TX_OCP_ADDR                   0x006E
#define STWLC68_TX_OCP_VAL                    2000 /* mA */
#define STWLC68_TX_OCP_STEP                   10 /* mA */
/* tx ovp register */
#define STWLC68_TX_OVP_ADDR                   0x006F
#define STWLC68_TX_OVP_VAL                    12000 /* mV */
#define STWLC68_TX_OVP_STEP                   100 /* mV */
/* tx_cep register */
#define STWLC68_TX_CEP_ADDR                   0x0075
#define STWLC68_TX_CEP_LEN                    1
/* tx set oper_freq */
#define STWLC68_TX_SET_OPER_FREQ_ADDR         0x0078
#define STWLC68_TX_OPER_FRE_UNIT              1000
/* tx max fop */
#define STWLC68_TX_MAX_FOP_ADDR               0x007A
#define STWLC68_TX_MAX_FOP_VAL                145 /* kHz */
/* tx min fop */
#define STWCL68_TX_MIN_FOP_ADDR               0x007B
#define STWLC68_TX_MIN_FOP_VAL                113 /* kHz */
/* tx ping freq */
#define STWLC68_TX_PING_FREQ_ADDR             0x007C
#define STWLC68_TX_PING_FREQ_UNIT             1000
#define STWLC68_TX_PING_FREQ_INIT             135 /* kHz */
#define STWLC68_TX_PING_FREQ_MIN              100 /* kHz */
#define STWLC68_TX_PING_FREQ_MAX              150 /* kHz */
/* tx ping interval */
#define STWLC68_TX_ILIMIT                     0x007F
#define STWLC68_TX_ILIMIT_STEP                16
#define STWLC68_TX_ILIMIT_MIN                 500
#define STWLC68_TX_ILIMIT_MAX                 1500
/* tx mode vset register */
#define STWLC68_PS_TX_GPIO_ADDR               0x006C
#define STWLC68_PS_TX_GPIO_OPEN               0x40
#define STWLC68_PS_TX_GPIO_PU                 0x80
#define STWLC68_PS_TX_GPIO_PD                 0xC0
#define STWLC68_PS_TX_VOLT_5V5                5500
#define STWLC68_PS_TX_VOLT_6V                 6000
#define STWLC68_PS_TX_VOLT_6V8                6800
#define STWLC68_PS_TX_VOLT_6V9                6900
#define STWLC68_PS_TX_VOLT_10V                10000
/* fod coef register */
#define STWLC68_RX_FOD_ADDR                   0x0080
#define STWLC68_RX_FOD_LEN                    12
#define STWLC68_RX_FOD_STR_LEN                64
#define STWLC68_RX_FOD_TMP_STR_LEN            4
/* rx fod offset register */
#define STWLC68_RX_OFFSET_ADDR                0x008C
#define STWLC68_RX_OFFSET_DEFAULT_VALUE       0
/* tx uvp register */
#define STWLC68_TX_UVP_ADDR                   0x008E
#define STWLC68_TX_UVP_STEP                   100
/* rx_ctrl_err register */
#define STWLC68_RX_CTRL_ERR_ADDR              0x0090
#define STWLC68_RX_CTRL_ERR_LEN               2
/* rx_rcvd_pwr register,  in mW */
#define STWLC68_RX_RCVD_PWR_ADDR              0x0092
#define STWLC68_RX_RCVD_PWR_LEN               2
/* tx_pwr_tfrd_to_rx register */
#define STWLC68_TX_TFRD_PWR_ADDR              0x0090
#define STWLC68_TX_TFRD_PWR_LEN               2
/* tx_pwr_rcvd_by_rx register */
#define STWLC68_TX_RCVD_PWR_ADDR              0x0092
#define STWLC68_TX_RCVD_PWR_LEN               2
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
/* rx 24bit RPP set register */
#define STWLC68_RX_RPP_SET_ADDR               0x00c3
#define STWLC68_RX_RPP_VAL_UNIT               2 /* in 0.5Watt units */
#define STWLC68_RX_RPP_VAL_MASK               0x3F
/* ldo_cfg: ldo_drop0-3 && ldo_cur_thres1-3 */
#define STWLC68_LDO_CFG_ADDR                  0x00A4
#define STWLC68_LDO_CFG_LEN                   7
#define STWLC68_LDO_VDROP_STEP                16 /* mV, vrect-vout */
#define STWLC68_LDO_CUR_TH_STEP               8 /* mA, iout */
/* die_id register */
#define STWLC68_DIE_ID_ADDR                   0x00B0
#define STWLC68_DIE_ID_LEN                    16
#define STWLC68_DIE_ID_STR_LEN                64
#define STWLC68_DIE_ID_TMP_STR_LEN            3
/* fc_volt register */
#define STWLC68_FC_VOLT_ADDR                  0x00C0
#define STWLC68_FC_VOLT_LEN                   2
#define STWLC68_BPCP_SLEEP_TIME               50
#define STWLC68_BPCP_TIMEOUT                  200
#define STWLC68_FC_VOUT_RETRY_CNT             3
#define STWLC68_FC_VOUT_SLEEP_TIME            50
#define STWLC68_FC_VOUT_TIMEOUT               1500
#define STWLC68_FC_VOUT_DEFAULT               5000
#define STWLC68_FC_VOUT_ERR_LTH               500 /* lower th */
#define STWLC68_FC_VOUT_ERR_UTH               1000 /* upper th */
/* fc_vrect delta */
#define STWLC68_FC_VRECT_DELTA_ADDR           0x00C2
#define STWLC68_FC_VRECT_DELTA                2000 /* mV */
#define STWLC68_FC_VRECT_DELTA_STEP           32
/* wdt_timeout register, in ms */
#define STWLC68_WDT_TIMEOUT_ADDR              0x00C6
#define STWLC68_WDT_TIMEOUT_LEN               2
#define STWLC68_WDT_MEOUT                     1000
/* wdt_feed register */
#define STWLC68_WDT_FEED_ADDR                 0x00C8
#define STWLC68_WDT_FEED_LEN                  1
/* tx ept type register */
#define STWLC68_TX_EPT_REASON_RCVD_ADDR       0x00D0
#define STWLC68_TX_EPT_REASON_PKG_LEN         2
#define STWLC68_TX_EPT_SRC_SS                 BIT(1)
#define STWLC68_TX_EPT_SRC_ID                 BIT(2)
#define STWLC68_TX_EPT_SRC_XID                BIT(3)
#define STWLC68_TX_EPT_SRC_CFG_CNT_ERR        BIT(4)
#define STWLC68_TX_EPT_SRC_CFG_ERR            BIT(5)
#define STWLC68_TX_EPT_SRC_RX_EPT             BIT(6)
#define STWLC68_TX_EPT_SRC_CEP_TIMEOUT        BIT(8)
#define STWLC68_TX_EPT_SRC_RPP_TIMEOUT        BIT(9)
#define STWLC68_TX_EPT_SRC_FOD                BIT(13)
#define STWLC68_TX_EPT_SRC_OTP                BIT(14)
/* sram start_addr register */
#define STWLC68_SRAM_START_ADDR_ADDR          0x0140
#define STWLC68_SRAM_START_ADDR_LEN           4
/* sram exe register */
#define STWLC68_EXE_SRAM_ADDR                 0x0145
#define STWLC68_EXE_SRAM_LEN                  1
#define STWLC68_EXE_SRAM_CMD                  0x10

#define STWLC68_SW2TX_RETRY_SLEEP_TIME        25 /* ms */
#define STWLC68_SW2TX_RETRY_TIME              1500 /* ms */

/* fw addr */
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
#define STWLC68_OTP_FREQ_CTRL_VAL             0x0E
#define STWLC68_CHIP_RESET_ADDR               0x2001C00c
#define STWLC68_CHIP_RESET                    0x01
#define STWLC68_OTP_VPP_ADDR                  0x00068158
#define STWLC68_OTP_VPP_6V_VAL                0x06
#define STWLC68_OTP_SKIP_SOAK_ADDR            0x00068038
#define STWLC68_OTP_SKIP_SOAK_VAL             0x3D

#define STWLC68_LDO5V_EN_ADDR                 0x2001C19B
#define STWLC68_LDO5V_EN                      1
#define STWLC68_LDO5V_DIS                     0

#define STWLC68_CUT_ID_V10                    0x01 /* 1.0 */
#define STWLC68_CUT_ID_V11                    0x02 /* 1.1 */
#define STWLC68_FW_OP_MODE_SA                 0x01
#define STWLC68_OTP_OPS_DELAY                 100 /* ms */
#define STWLC68_ILOAD_DRIVE_ADDR              0x2001C176
#define STWLC68_ILOAD_DISABLE_VALUE           0x01
#define STWLC68_ILOAD_DATA_LEN                0x01

/* command */
#define STWLC68_CMD_START_SAMPLE              0x1d
#define STWLC68_CMD_STOP_SAMPLE               0x1e
#define STWLC68_CMD_ACK                       0xff
#define STWLC68_CMD_ACK_HEAD                  0x1e

#define STWLC68_TX_ID_LEN                     3
#define STWLC68_LIMIT_CURRENT_TIME            500
#define STWLC68_BATT_CAPACITY_LEN             2
#define STWLC68_BATT_TEMP_LEN                 1
#define STWLC68_BATT_TEMP_MAX                 127
#define STWLC68_SHUTDOWN_SLEEP_TIME           200
#define STWLC68_RCV_MSG_SLEEP_TIME            100
#define STWLC68_RCV_MSG_SLEEP_CNT             10
#define STWLC68_WAIT_FOR_ACK_SLEEP_TIME       100
#define STWLC68_WAIT_FOR_ACK_RETRY_CNT        5
#define STWLC68_SNED_MSG_RETRY_CNT            2
#define STWLC68_GET_TX_PARA_RETRY_CNT         2
#define STWLC68_RX_TMP_BUFF_LEN               32

/* fw otp_info */
#define STWLC68_OTP_START_ADDR                0x60000
#define STWLC68_OTP_CLEAN_ADDR                0x600C0
#define STWLC68_OTP_MAX_SIZE                  (16 * 1024)
#define STWLC68_RAM_MAX_SIZE                  (8 * 1024)
#define STWLC68_OTP_CMP_SIZE                  16
#define STWLC68_OTP_DUMMY_LEN                 8

#define STWLC68_OTP_RECOVER_WR_SIZE           4
#define STWLC68_OTP_PROGRAM_WR_SIZE           128
#define STWLC68_OTP_WR_BLOCK_SIZE             4096

#define STWLC68_RAM_FW_START_ADDR             0x00050000
#define STWLC68_RAM_FW_CODE_OFFSET            0x108C
#define STWLC68_RAM_CHECK_VAL1                0xAA /* A: 1010 */
#define STWLC68_RAM_CHECK_VAL2                0x55 /* 5: 0101 */
#define STWLC68_OTP_FW_START_ADDR             0x00051000
#define STWLC68_OTP_ERR_STATUS_ADDR           0x00068014
#define STWLC68_OTP_ERR_CORRUPT               0x40
#define STWLC68_PGM_IPS_MRR_HI_ADDR           0x0006814C
#define STWLC68_PGM_IPS_MRR_HI_VAL            0x05
#define STWLC68_RD_VERIFY1_MREF_HI_ADDR       0x000680C0
#define STWLC68_RD_VERIFY1_MREF_VAL           0xCD
#define STWLC68_RD_VERIFY_OTP_MREF_ADDR       0x000680C8
#define STWLC68_RD_VERIFY2_OTP_HI_VAL         0xE5
#define STWLC68_SYSTEM_OSC_ADDR               0x2001C169
#define STWLC68_OSC_TRIM_VAL                  0x3F
#define STWLC68_ROM_CHECK_ADDR                0x2001C006
#define STWLC68_SYSTEM_CLK_DIV_ADDR           0x2001C00E
#define STWLC68_CLK_DIV_VAL                   0x00

/* boot options */
#define STWLC68_BOOT_SET_ADDR                 0x2001C218
#define STWLC68_BOOT_FROM_RAM                 0x01
#define STWLC68_BOOT_FROM_ROM                 0x00

#define STWLC68_RAM_FW_VER_ADDR               0x0006
#define STWLC68_GPIO_STATUS_ADDR              0x0012
#define STWLC68_GPIO_RESET_VAL                0x10
#define STWLC68_SYSTEM_ERR_ADDR               0x001D
#define STWLC68_OTP_WR_CMD_ADDR               0x0145
#define STWLC68_CLEAN_ADDR_UPDATE_VAL         0x40
#define STWLC68_OTP_ENABLE                    0x01
#define STWLC68_ROM_CHECK_VAL                 0x80
#define STWLC68_FW_CLEAN_ADDR_ADDR            0x0146
#define STWLC68_OTP_WRITE_LENGTH_ADDR         0x014C

#define STWLC68_RAM_ROM_STATUS_UNKNOWN        0x01
#define STWLC68_RAM_ROM_STATUS_OK             0x02
#define STWLC68_RAM_ROM_STATUS_BAD            0x03

/* fw sram_info */
#define ST_RAMPATCH_HEADER_SIZE               8
#define ST_RAMPATCH_MAX_SIZE                  4112 /* 4k */
#define ST_RAMPATCH_PAGE_SIZE                 128

#endif /* _STWLC68_CHIP_H_ */
