/*
 * mt5735_chip.h
 *
 * mt5735 registers, chip info, etc.
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

#ifndef _MT5735_CHIP_H_
#define _MT5735_CHIP_H_

#define MT5735_ADDR_LEN                      2
#define MT5735_HW_ADDR_LEN                   4
#define MT5735_HW_ADDR_F_LEN                 5 /* len with flag */
#define MT5735_HW_ADDR_FLAG                  0xFA
#define MT5735_GPIO_PWR_GOOD_VAL             1

/* chip_info: 0x0000 ~ 0x000C */
#define MT5735_CHIP_INFO_ADDR                0x0000
#define MT5735_CHIP_INFO_LEN                 14
/* chip id register */
#define MT5735_CHIP_ID_ADDR                  0x0000
#define MT5735_HW_CHIP_ID_ADDR               0x5A50
#define MT5735_CHIP_ID_LEN                   2
#define MT5735_CHIP_ID                       0x5735
#define MT5735_HW_CHIP_ID                    0x5730
#define MT5735_CHIP_ID_AB                    0xFFFF /* abnormal chip id */
/* op mode register */
#define MT5735_OP_MODE_ADDR                  0x0008
#define MT5735_OP_MODE_LEN                   4
#define MT5735_OP_MODE_NA                    0x00
#define MT5735_OP_MODE_SA                    0x03 /* stand_alone */
#define MT5735_OP_MODE_RX                    BIT(0)
#define MT5735_OP_MODE_TX                    BIT(1)
#define MT5735_SYS_MODE_MASK                 (BIT(0) | BIT(1))
#define MT5735_OP_MODE_START_TX              BIT(4)
/* send_msg: bit[0]:header, bit[1]:cmd, bit[2:5]:data */
#define MT5735_SEND_MSG_DATA_LEN             4
#define MT5735_SEND_MSG_PKT_LEN              6
/* rcvd_msg: bit[0]:header, bit[1]:cmd, bit[2:5]:data */
#define MT5735_RCVD_MSG_DATA_LEN             4
#define MT5735_RCVD_MSG_PKT_LEN              6
#define MT5735_RCVD_PKT_BUFF_LEN             8
#define MT5735_RCVD_PKT_STR_LEN              64

/*
 * rx mode
 */
/* rx_send_msg_data register */
#define MT5735_SEND_MSG_HEADER_ADDR          0x0070
#define MT5735_SEND_MSG_CMD_ADDR             0x0071
#define MT5735_SEND_MSG_DATA_ADDR            0x0072
/* rx_rcvd_msg_data register */
#define MT5735_RCVD_MSG_HEADER_ADDR          0x0086
#define MT5735_RCVD_MSG_CMD_ADDR             0x0087
#define MT5735_RCVD_MSG_DATA_ADDR            0x0088
/* rx_rp_val register, last RP value sent */
#define MT5735_RX_RP_VAL_ADDR                0x005C
#define MT5735_RX_RP_VAL_LEN                 2
/* rx_ce_val register, last CE value sent */
#define MT5735_RX_CE_VAL_ADDR                0x00F1
#define MT5735_RX_CE_VAL_LEN                 1
/* rx_ask_mod_cfg register */
#define MT5735_RX_ASK_CFG_ADDR               0x0092
#define MT5735_RX_ASK_CFG_LEN                1
#define MT5735_ALL_CAP_POSITIVE              0x07
#define MT5735_BOTH_CAP_NEGATIVE             0x0B
/* rx_irq_en register */
#define MT5735_RX_IRQ_EN_ADDR                0x0080
#define MT5735_RX_IRQ_EN_LEN                 4
/* rx_irq_clr register */
#define MT5735_RX_IRQ_CLR_ADDR               0x0018
#define MT5735_RX_IRQ_CLR_LEN                4
#define MT5735_RX_IRQ_CLR_ALL                0xFFFFFFFF
/* rx_irq_latch register */
#define MT5735_RX_IRQ_ADDR                   0x0014
#define MT5735_RX_IRQ_LEN                    4
#define MT5735_RX_IRQ_OTP                    BIT(1)
#define MT5735_RX_IRQ_OCP                    BIT(13)
#define MT5735_RX_IRQ_OVP                    BIT(14)
#define MT5735_RX_IRQ_SYS_ERR                BIT(3)
#define MT5735_RX_IRQ_DATA_RCVD              BIT(19)
#define MT5735_RX_IRQ_OUTPUT_ON              BIT(10)
#define MT5735_RX_IRQ_OUTPUT_OFF             BIT(11)
#define MT5735_RX_IRQ_SEND_PKT_SUCC          BIT(18)
#define MT5735_RX_IRQ_SEND_PKT_TIMEOUT       BIT(17)
#define MT5735_RX_IRQ_POWER_ON               BIT(8)
#define MT5735_RX_IRQ_READY                  BIT(9)
/* rx_status register */
#define MT5735_RX_STATUS_ADDR                0x008C
#define MT5735_RX_STATUS_LEN                 4
/* rx_cmd register */
#define MT5735_RX_CMD_ADDR                   0x000C
#define MT5735_RX_CMD_LEN                    4
#define MT5735_RX_CMD_VAL                    1
#define MT5735_RX_CMD_VOUT_ON                BIT(0)
#define MT5735_RX_CMD_VOUT_ON_SHIFT          0
#define MT5735_RX_CMD_VOUT_OFF               BIT(1)
#define MT5735_RX_CMD_VOUT_OFF_SHIFT         1
#define MT5735_RX_CMD_SEND_MSG               BIT(2)
#define MT5735_RX_CMD_SEND_MSG_SHIFT         2
#define MT5735_RX_CMD_SEND_MSG_RPLY          BIT(8)
#define MT5735_RX_CMD_SEND_MSG_RPLY_SHIFT    8
#define MT5735_RX_CMD_SEND_EPT               BIT(10)
#define MT5735_RX_CMD_SEND_EPT_SHIFT         10
#define MT5735_RX_CMD_SEND_DTS               BIT(5)
#define MT5735_RX_CMD_SEND_DTS_SHIFT         5
#define MT5735_RX_CMD_SEND_FC                BIT(11)
#define MT5735_RX_CMD_SEND_FC_SHIFT          11
#define MT5735_RX_CMD_CLEAR_INT              BIT(2)
#define MT5735_RX_CMD_CLEAR_INT_SHIFT        2
#define MT5735_RX_CMD_SET_RX_VOUT            BIT(14)
#define MT5735_RX_CMD_SET_RX_VOUT_SHIFT      14
#define MT5735_RX_CMD_SET_RX_VRECT_OVP       BIT(12)
#define MT5735_RX_CMD_SET_RX_VRECT_OVP_SHIFT 12
#define MT5735_RX_CMD_SET_RX_OCP             BIT(13)
#define MT5735_RX_CMD_SET_RX_OCP_SHIFT       13
#define MT5735_RX_CMD_SET_RX_VOUT_OVP        BIT(15)
#define MT5735_RX_CMD_SET_RX_VOUT_OVP_SHIFT  15
#define MT5735_RX_CMD_SET_RX_OTP             BIT(16)
#define MT5735_RX_CMD_SET_RX_OTP_SHIFT       16
#define MT5735_RX_CMD_RX_LDO5V_DIS           BIT(17)
#define MT5735_RX_CMD_RX_LDO5V_DIS_SHIFT     17
#define MT5735_RX_CMD_RX_LDO5V_EN            BIT(18)
#define MT5735_RX_CMD_RX_LDO5V_EN_SHIFT      18
#define MT5735_RX_CMD_SET_24BIT_RPP          BIT(23)
#define MT5735_RX_CMD_SET_24BIT_RPP_SHIFT    23

/* rx_vrect register */
#define MT5735_RX_VRECT_ADDR                 0x0028
#define MT5735_RX_VRECT_LEN                  2
/* rx_vout register */
#define MT5735_RX_VOUT_ADDR                  0x0026
#define MT5735_RX_VOUT_LEN                   2
/* rx_iout register */
#define MT5735_RX_IOUT_ADDR                  0x0024
#define MT5735_RX_IOUT_LEN                   2
/* rx_chip_temp register, in degC */
#define MT5735_RX_CHIP_TEMP_ADDR             0x002A
#define MT5735_RX_CHIP_TEMP_LEN              2
/* rx_rpp register */
#define MT5735_RX_RPP_ADDR                   0x00E9
#define MT5735_RX_RPP_LEN                    2
/* rx_op_freq register, in kHZ */
#define MT5735_RX_OP_FREQ_ADDR               0x0020
#define MT5735_RX_OP_FREQ_LEN                2
/* rx_ntc register */
#define MT5735_RX_NTC_ADDR                   0x009C
#define MT5735_RX_NTC_LEN                    2
/* rx_adc_in1 register */
#define MT5735_RX_ADC_IN1_ADDR               0x009E
#define MT5735_RX_ADC_IN1_LEN                2
/* rx_adc_in2 register */
#define MT5735_RX_ADC_IN2_ADDR               0x00A0
#define MT5735_RX_ADC_IN2_LEN                2
/* rx_adc_in3 register */
#define MT5735_RX_ADC_IN3_ADDR               0x00A2
#define MT5735_RX_ADC_IN3_LEN                2
/* rx_ctrl_err register, in mV */
#define MT5735_RX_CTRL_ERR_ADDR              0x00A4
#define MT5735_RX_CTRL_ERR_LEN               2
/* rx_rcvd_pwr register, in mW */
#define MT5735_RX_RCVD_PWR_ADDR              0x00A6
#define MT5735_RX_RCVD_PWR_LEN               2
/* rx_signal_strength register */
#define MT5735_RX_SS_ADDR                    0x00E8
#define MT5735_RX_SS_LEN                     2
#define MT5735_RX_SS_MIN                     0
#define MT5735_RX_SS_MAX                     255
/* rx_vout_set register */
#define MT5735_RX_VOUT_SET_ADDR              0x0046
#define MT5735_RX_VOUT_SET_LEN               2
#define MT5735_RX_VOUT_SET_STEP              25 /* mV */
#define MT5735_RX_VOUT_MAX                   22000
#define MT5735_RX_VOUT_MIN                   25
/* rx_vrect_adj register */
#define MT5735_RX_VRECT_ADJ_ADDR             0x00B4
#define MT5735_RX_VRECT_ADJ_LEN              1
/* rx_ilim_set register */
#define MT5735_RX_ILIM_SET_ADDR              0x0042
#define MT5735_RX_ILIM_SET_LEN               2
#define MT5735_RX_ILIM_SET_STEP              100 /* mA */
#define MT5735_RX_ILIM_MAX                   1500
#define MT5735_RX_ILIM_MIN                   100
/* rx_vlim0_set register */
#define MT5735_RX_VRECT_VLIM0_SET_ADDR       0x002E
#define MT5735_RX_VRECT_VLIM0_SET_LEN        2
/* rx_vlim1_set register */
#define MT5735_RX_VRECT_VLIM1_SET_ADDR       0x0032
#define MT5735_RX_VRECT_VLIM1_SET_LEN        2
/* rx_vrect_vlim_set register */
#define MT5735_RX_VOUT_VLIM_SET_ADDR         0x0044
#define MT5735_RX_VOUT_VLIM_SET_LEN          2
/* rx_otp_set register */
#define MT5735_RX_OTP_SET_ADDR               0x00F2
#define MT5735_RX_OTP_SET_LEN                2
/* rx_fod_coef register, 0x00c7: rx_ser in 4mohm */
#define MT5735_RX_FOD_ADDR                   0x00A0
#define MT5735_RX_FOD_LEN                    16
#define MT5735_RX_FOD_TMP_STR_LEN            4
/* rx_ldo_cfg: ldo_drop0-3 && ldo_cur_thres1-3 */
#define MT5735_RX_LDO_CFG_ADDR               0x0036
#define MT5735_RX_LDO_CFG_LEN                8
#define MT5735_RX_LDO_VDROP_STEP             16 /* mV, vrect-vout */
#define MT5735_RX_LDO_CUR_TH_STEP            8 /* mA, iout */
/* rx_ept_msg register, ept reason to be included when sending ept packet */
#define MT5735_RX_EPT_MSG_ADDR               0x00EC
#define MT5735_RX_EPT_MSG_LEN                1
/* rx_fc_volt register */
#define MT5735_RX_FC_VOLT_ADDR               0x005E
#define MT5735_RX_FC_VOLT_LEN                2
#define MT5735_RX_BPCP_SLEEP_TIME            50
#define MT5735_RX_BPCP_TIMEOUT               200
#define MT5735_RX_FC_VOUT_RETRY_CNT          3
#define MT5735_RX_FC_VOUT_SLEEP_TIME         50
#define MT5735_RX_FC_VOUT_TIMEOUT            1500
#define MT5735_RX_FC_VOUT_DEFAULT            5000
#define MT5735_RX_FC_VOUT_ERR_LTH            500 /* lower threshold */
#define MT5735_RX_FC_VOUT_ERR_UTH            1000 /* upper threshold */
/* rx_wdt_timeout register, in ms */
#define MT5735_RX_WDT_TIMEOUT_ADDR           0x0062
#define MT5735_RX_WDT_TIMEOUT_LEN            2
#define MT5735_RX_WDT_TIMEOUT                1000
/* rx_wdt_feed register */
#define MT5735_RX_WDT_FEED_ADDR              0x0064
#define MT5735_RX_WDT_FEED_LEN               2
/* rx_24bit_rp_set register */
#define MT5735_RX_RPP_SET_ADDR               0x000C
#define MT5735_RX_RPP_SET_LEN                4
#define MT5735_RX_RPP_VAL_UNIT               2 /* in 0.5Watt units */
#define MT5735_RX_RPP_VAL_MASK               0x3F
/* rx max pwr for RP val calculation */
#define MT5735_RX_RP_PMAX_ADDR               0x0091
#define MT5735_RX_RP_PMAX_LEN                1
#define MT5735_RX_RP_VAL_UNIT                2
/* rx rp type */
#define MT5735_RX_RP_TYPE_ADDR               0x1F60
#define MT5735_RX_RP_TYPE_LEN                1
#define MT5735_RX_RP_NO_REPLY                0
#define MT5735_RX_RP_WITH_REPLY              1
/* rx_fc_vrect_diff register */
#define MT5735_RX_FC_VRECT_DIFF_ADDR         0x0048
#define MT5735_RX_FC_VRECT_DIFF_LEN          2
#define MT5735_RX_FC_VRECT_DIFF              2000 /* mV */
#define MT5735_RX_FC_VRECT_DIFF_STEP         32
/* rx_dts_send register */
#define MT5735_RX_DTS_SEND_ADDR              0x00D8
#define MT5735_RX_DTS_SEND_LEN               4
/* rx_dts_rcvd register */
#define MT5735_RX_DTS_RCVD_ADDR              0x00DC
#define MT5735_RX_DTS_RCVD_LEN               4

/*
 * tx mode
 */
/* tx_send_msg_data register */
#define MT5735_TX_SEND_MSG_HEADER_ADDR       0x0036
#define MT5735_TX_SEND_MSG_CMD_ADDR          0x0037
#define MT5735_TX_SEND_MSG_DATA_ADDR         0x0038
/* tx_rcvd_msg_data register */
#define MT5735_TX_RCVD_MSG_HEADER_ADDR       0x0020
#define MT5735_TX_RCVD_MSG_CMD_ADDR          0x0021
#define MT5735_TX_RCVD_MSG_DATA_ADDR         0x0022
/* tx_irq_en register */
#define MT5735_TX_IRQ_EN_ADDR                0x0010
#define MT5735_TX_IRQ_EN_LEN                 4
#define MT5735_TX_IRQ_EN_VAL                 0x45E9F07
/* tx_irq_clr register */
#define MT5735_TX_IRQ_CLR_ADDR               0x0018
#define MT5735_TX_IRQ_CLR_LEN                4
#define MT5735_TX_IRQ_CLR_ALL                0xFFFFFFFF
/* tx_irq_latch register */
#define MT5735_TX_IRQ_ADDR                   0x0014
#define MT5735_TX_IRQ_LEN                    4
#define MT5735_TX_IRQ_CHIPRST                BIT(0)
#define MT5735_TX_IRQ_OTP                    BIT(1)
#define MT5735_TX_IRQ_SYS_ERR                BIT(3)
#define MT5735_TX_IRQ_RPP_RCVD               BIT(4)
#define MT5735_TX_IRQ_CEP_RCVD               BIT(5)
#define MT5735_TX_IRQ_SEND_PKT_SUCC          BIT(6)
#define MT5735_TX_IRQ_DPING_RCVD             BIT(16)
#define MT5735_TX_IRQ_SS_PKG_RCVD            BIT(8)
#define MT5735_TX_IRQ_ID_PKT_RCVD            BIT(9)
#define MT5735_TX_IRQ_CFG_PKT_RCVD           BIT(10)
#define MT5735_TX_IRQ_CEP_TIMEOUT            BIT(24)
#define MT5735_TX_IRQ_RPP_TIMEOUT            BIT(25)
#define MT5735_TX_IRQ_EPT_PKT_RCVD           BIT(17)
#define MT5735_TX_IRQ_START_PING             BIT(18)
#define MT5735_TX_IRQ_OCP                    BIT(20)
#define MT5735_TX_IRQ_OVP                    BIT(19)
#define MT5735_TX_IRQ_PP_PKT_RCVD            BIT(15)
#define MT5735_TX_IRQ_FOD_DET                BIT(23)
#define MT5735_TX_IRQ_FSK_ACK                BIT(26)
/* tx_irq_status register */
#define MT5735_TX_IRQ_STATUS_ADDR            0x010C
#define MT5735_TX_IRQ_STATUS_LEN             4
/* tx_cmd register */
#define MT5735_TX_CMD_ADDR                   0x000C
#define MT5735_TX_CMD_LEN                    4
#define MT5735_TX_CMD_VAL                    1
#define MT5735_TX_CMD_RST_SYS                BIT(1)
#define MT5735_TX_CMD_RST_SYS_SHIFT          1
#define MT5735_TX_CMD_CLEAR_INT              BIT(2)
#define MT5735_TX_CMD_CLEAR_INT_SHIFT        2
#define MT5735_TX_CMD_EN_TX                  BIT(3)
#define MT5735_TX_CMD_EN_TX_SHIFT            3
#define MT5735_TX_CMD_DIS_TX                 BIT(1)
#define MT5735_TX_CMD_DIS_TX_SHIFT           1
#define MT5735_TX_CMD_SEND_MSG               BIT(8)
#define MT5735_TX_CMD_SEND_MSG_SHIFT         8
#define MT5735_TX_CMD_OVP                    BIT(10)
#define MT5735_TX_CMD_OVP_SHIFT              10
#define MT5735_TX_CMD_OCP                    BIT(11)
#define MT5735_TX_CMD_OCP_SHIFT              11
#define MT5735_TX_CMD_OTP                    BIT(13)
#define MT5735_TX_CMD_OTP_SHIFT              13
#define MT5735_TX_CMD_FOD_CTRL               BIT(15)
#define MT5735_TX_CMD_FOD_CTRL_SHIFT         15
#define MT5735_TX_CMD_START_TX               BIT(16)
#define MT5735_TX_CMD_START_TX_SHIFT         16
#define MT5735_TX_CMD_STOP_TX                BIT(17)
#define MT5735_TX_CMD_STOP_TX_SHIFT          17
#define MT5735_TX_CMD_SET_24BIT_RPP          BIT(18)
#define MT5735_TX_CMD_SET_24BIT_RPP_SHIFT    18
#define MT5735_TX_CMD_SET_Q_CHECK            BIT(20)
#define MT5735_TX_CMD_SET_Q_CHECK_SHIFT      20
#define MT5735_TX_CMD_DIS_Q_CHECK            BIT(21)
#define MT5735_TX_CMD_DIS_Q_CHECK_SHIFT      21
/* tx_ept_type register */
#define MT5735_TX_EPT_SRC_ADDR               0x00EC
#define MT5735_TX_EPT_SRC_LEN                4
#define MT5735_TX_EPT_SRC_OVP                BIT(0) /* stop */
#define MT5735_TX_EPT_SRC_OCP                BIT(1) /* stop */
#define MT5735_TX_EPT_SRC_OTP                BIT(2) /* stop */
#define MT5735_TX_EPT_SRC_FOD                BIT(3) /* stop */
#define MT5735_TX_EPT_SRC_CMD                BIT(4) /* AP stop */
#define MT5735_TX_EPT_SRC_RX_EPT             BIT(5) /* re ping receive ept */
#define MT5735_TX_EPT_SRC_CEP_TIMEOUT        BIT(6) /* re ping */
#define MT5735_TX_EPT_SRC_RPP_TIMEOUT        BIT(7) /* re ping */
#define MT5735_TX_EPT_SRC_RX_RST             BIT(8) /* re ping receive ept */
#define MT5735_TX_EPT_SRC_SYS_ERR            BIT(9) /* stop */
#define MT5735_TX_EPT_SRC_PING_TIMEOUT       BIT(10) /* re ping */
#define MT5735_TX_EPT_SRC_SS                 BIT(11) /* re ping */
#define MT5735_TX_EPT_SRC_ID                 BIT(12) /* re ping */
#define MT5735_TX_EPT_SRC_CFG                BIT(13) /* re ping */
#define MT5735_TX_EPT_SRC_CFG_CNT            BIT(14) /* re ping */
#define MT5735_TX_EPT_SRC_PCH                BIT(15) /* re ping */
#define MT5735_TX_EPT_SRC_XID                BIT(16) /* re ping */
#define MT5735_TX_EPT_SRC_NEGO               BIT(17) /* re ping */
#define MT5735_TX_EPT_SRC_NEGO_TIMEOUT       BIT(18) /* re ping */
#define MT5735_TX_EPT_SRC_Q_FOD              BIT(23) /* stop */
/* tx_fod_ploss_cnt register */
#define MT5735_TX_PLOSS_CNT_ADDR             0x0060
#define MT5735_TX_PLOSS_CNT_LEN              1
#define MT5735_TX_PLOSS_CNT_VAL              2
/* tx_fod_ploss_thres register, in mW */
#define MT5735_TX_PLOSS_TH0_ADDR             0x0064 /* 5v full bridge */
#define MT5735_TX_PLOSS_TH0_VAL              5000
#define MT5735_TX_PLOSS_TH0_LEN              2
#define MT5735_TX_PLOSS_TH1_ADDR             0x0066 /* 10v half bridge */
#define MT5735_TX_PLOSS_TH1_VAL              5000
#define MT5735_TX_PLOSS_TH1_LEN              2
#define MT5735_TX_PLOSS_TH2_ADDR             0x0068 /* 10v full bridge low vol */
#define MT5735_TX_PLOSS_TH2_VAL              5000
#define MT5735_TX_PLOSS_TH2_LEN              2
#define MT5735_TX_PLOSS_TH3_ADDR             0x006A /* 10v full bridge high vol */
#define MT5735_TX_PLOSS_TH3_VAL              5000
#define MT5735_TX_PLOSS_TH3_LEN              2
#define MT5735_TX_HP_PLOSS_TH0_ADDR          0x006C /* 15v full bridge high vol, low current */
#define MT5735_TX_HP_PLOSS_TH0_VAL           5000
#define MT5735_TX_HP_PLOSS_TH0_LEN           2
#define MT5735_TX_HP_PLOSS_TH1_ADDR          0x006E /* 15v full bridge high vol, high current */
#define MT5735_TX_HP_PLOSS_TH1_VAL           5000
#define MT5735_TX_HP_PLOSS_TH1_LEN           2
/* tx_fod_th_id register */
#define MT5735_TX_FOD_TH_ID_ADDR             0x0070
#define MT5735_TX_FOD_TH_ID_LEN              1
/* tx_vrect register, in mV */
#define MT5735_TX_VRECT_ADDR                 0x007C
#define MT5735_TX_VRECT_LEN                  2
/* tx_vin register, in mV */
#define MT5735_TX_VIN_ADDR                   0x007A
#define MT5735_TX_VIN_LEN                    2
/* tx_iin register, in mA */
#define MT5735_TX_IIN_ADDR                   0x0078
#define MT5735_TX_IIN_LEN                    2
/* tx_chip_temp register, in degC */
#define MT5735_TX_CHIP_TEMP_ADDR             0x0080
#define MT5735_TX_CHIP_TEMP_LEN              2
/* tx_oper_freq register, in 4Hz */
#define MT5735_TX_OP_FREQ_ADDR               0x007E
#define MT5735_TX_OP_FREQ_LEN                2
#define MT5735_TX_OP_FREQ_STEP               10
/* tx_pwm_duty register */
#define MT5735_TX_PWM_DUTY_ADDR              0x008A
#define MT5735_TX_PWM_DUTY_LEN               1
/* tx_ploss value register, in mw */
#define MT5735_TX_PLOSS_ADDR                 0x008C
#define MT5735_TX_PLOSS_LEN                  2
/* tx_fod_status register */
#define MT5735_TX_Q_FOD_ADDR                 0x0094
#define MT5735_TX_Q_FOD_LEN                  2
#define MT5735_TX_F_FOD_ADDR                 0x0096
#define MT5735_TX_F_FOD_LEN                  2
/* tx_power value register, in mw */
#define MT5735_TX_PTX_ADDR                   0x00E0
#define MT5735_TX_PTX_LEN                    4
/* rx_power value register, in mw */
#define MT5735_TX_PRX_ADDR                   0x00E4
#define MT5735_TX_PRX_LEN                    4
/* tx ept_reason register */
#define MT5735_TX_EPT_REASON_ADDR            0x00F0
#define MT5735_TX_EPT_REASON_LEN             1
/* tx_cep_value register */
#define MT5735_TX_CEP_ADDR                   0x00F1
#define MT5735_TX_CEP_LEN                    1
/* tx_ntc register */
#define MT5735_TX_NTC_ADDR                   0x0120
#define MT5735_TX_NTC_LEN                    2
/* tx_adc_in1 register */
#define MT5735_TX_ADC_IN1_ADDR               0x0122
#define MT5735_TX_ADC_IN1_LEN                2
/* tx_adc_in2 register */
#define MT5735_TX_ADC_IN2_ADDR               0x0124
#define MT5735_TX_ADC_IN2_LEN                2
/* tx_adc_in3 register */
#define MT5735_TX_ADC_IN3_ADDR               0x0126
#define MT5735_TX_ADC_IN3_LEN                2
/* tx_pwr_tfrd_to_rx register */
#define MT5735_TX_TFRD_PWR_ADDR              0x0128
#define MT5735_TX_TFRD_PWR_LEN               2
/* tx_pwr_rcvd_by_rx register */
#define MT5735_TX_RCVD_PWR_ADDR              0x012A
#define MT5735_TX_RCVD_PWR_LEN               2
/* tx_ptc_ref_pwr register */
#define MT5735_TX_PTC_REF_PWR_ADDR           0x012C
#define MT5735_TX_PTC_REF_PWR_LEN            1
/* tx_customer register */
#define MT5735_TX_CUST_CTRL_ADDR             0x0089
#define MT5735_TX_CUST_CTRL_LEN              1
#define MT5735_TX_PS_GPIO_MASK               (BIT(0) | BIT(1))
#define MT5735_TX_PS_GPIO_SHIFT              0
#define MT5735_TX_PS_GPIO_OPEN               0x1
#define MT5735_TX_PS_GPIO_PU                 0x2
#define MT5735_TX_PS_GPIO_PD                 0x3
#define MT5735_TX_PS_VOLT_5V5                5500
#define MT5735_TX_PS_VOLT_6V8                6800
#define MT5735_TX_PS_VOLT_10V                10000
#define MT5735_TX_PT_BRIDGE_MASK             BIT(1)
#define MT5735_TX_PT_BRIDGE_SHIFT            1
#define MT5735_TX_PT_BRIDGE_NO_CHANGE        0 /* same as ping */
#define MT5735_TX_PT_HALF_BRIDGE             0 /* manual half bridge mode */
#define MT5735_TX_PT_FULL_BRIDGE             1 /* manual full bridge mode */
#define MT5735_TX_PT_AUTO_SW_BRIDGE          3 /* auto switch */
#define MT5735_TX_PING_BRIDGE_MASK           BIT(0)
#define MT5735_TX_PING_BRIDGE_SHIFT          0
#define MT5735_TX_PING_FULL_BRIDGE           1 /* ping in full bridge mode */
#define MT5735_TX_PING_HALF_BRIDGE           0 /* ping in half bridge mode */
#define MT5735_TX_FULL_BRIDGE_ITH            200 /* mA */
/* tx_ovp_thres register, in mV */
#define MT5735_TX_OVP_TH_ADDR                0x0058
#define MT5735_TX_OVP_TH_LEN                 2
#define MT5735_TX_OVP_TH                     20000
#define MT5735_TX_OVP_TH_STEP                1
/* tx_ocp_thres register, in mA */
#define MT5735_TX_OCP_TH_ADDR                0x0054
#define MT5735_TX_OCP_TH_LEN                 2
#define MT5735_TX_OCP_TH                     2000
#define MT5735_TX_OCP_TH_STEP                1
/* tx_otp register, in degC */
#define MT5735_TX_OTP_TH_ADDR                0x00F2
#define MT5735_TX_OTP_TH_LEN                 1
#define MT5735_TX_OTP_TH                     80
/* tx_ilimit register */
#define MT5735_TX_ILIM_ADDR                  0x0143
#define MT5735_TX_ILIM_LEN                   1
#define MT5735_TX_ILIM_STEP                  16
#define MT5735_TX_ILIM_MIN                   500
#define MT5735_TX_ILIM_MAX                   2000
/* tx_max_duty cycle, in % */
#define MT5735_TX_MAX_DC_ADDR                0x0148
#define MT5735_TX_MAX_DC_LEN                 1
#define MT5735_TX_MAX_DC                     50
/* tx_min_duty cycle, in % */
#define MT5735_TX_MIN_DC_ADDR                0x0149
#define MT5735_TX_MIN_DC_LEN                 1
#define MT5735_TX_MIN_DC                     30
/* tx_fod_ploss_thres register, in mW */
#define MT5735_TX_PLOSS_TH_ADDR              0x014A
#define MT5735_TX_PLOSS_TH_LEN               1
#define MT5735_TX_PLOSS_TH_STEP              32
#define MT5735_TX_PLOSS_TH_VAL               6500 /* mW */
/* tx_max_fop, in kHz */
#define MT5735_TX_MAX_FOP_ADDR               0x004C
#define MT5735_TX_MAX_FOP_LEN                2
#define MT5735_TX_MAX_FOP                    145
#define MT5735_TX_FOP_STEP                   10
/* tx_min_fop, in kHz */
#define MT5735_TX_MIN_FOP_ADDR               0x004E
#define MT5735_TX_MIN_FOP_LEN                2
#define MT5735_TX_MIN_FOP                    113
/* tx_ping_freq, in kHz */
#define MT5735_TX_PING_FREQ_ADDR             0x0050
#define MT5735_TX_PING_FREQ_LEN              2
#define MT5735_TX_PING_FREQ                  135
#define MT5735_TX_PING_FREQ_MIN              100
#define MT5735_TX_PING_FREQ_MAX              170
#define MT5735_TX_PING_STEP                  10
/* tx max pwr for rp val calculation */
#define MT5735_TX_RP_PMAX_ADDR               0x0071
#define MT5735_TX_RP_PMAX_LEN                1
#define MT5735_TX_RP_VAL_UNIT                2
/* tx_ping_interval, in ms */
#define MT5735_TX_PING_INTERVAL_ADDR         0x0084
#define MT5735_TX_PING_INTERVAL_LEN          2
#define MT5735_TX_PING_INTERVAL_STEP         1
#define MT5735_TX_PING_INTERVAL_MIN          0
#define MT5735_TX_PING_INTERVAL_MAX          1000
#define MT5735_TX_PING_INTERVAL              500
/* tx_pt_freq, in kHz */
#define MT5735_TX_PT_DELTA_FREQ_ADDR         0x0086
#define MT5735_TX_PT_FREQ_LEN                2
#define MT5735_TX_PT_FREQ                    130
#define MT5735_TX_PT_FREQ_MIN                100
#define MT5735_TX_PT_FREQ_MAX                150
#define MT5735_TX_PT_STEP                    10
#define MT5735_TX_PT_DELTA_FREQ              200
/* tx_ping_duty_cycle register */
#define MT5735_TX_PT_DC_ADDR                 0x0088
#define MT5735_TX_HALF_BRIDGE_DC             255
#define MT5735_TX_FULL_BRIDGE_DC             150

/*
 * firmware register
 */

/* mtp register */
#define MT5735_MTP_MINOR_ADDR                0x0004
#define MT5735_MTP_MAJOR_ADDR                0x0006
#define MT5735_MTP_CHIP_ID_ADDR              0x0008
#define MT5735_BTLOADR_ADDR                  0x1800
/* mtp program */
#define MT5735_MTP_PGM_CMD_ADDR              0x0000
#define MT5735_MTP_PGM_CMD                   0x10
#define MT5735_MTP_PGM_SIZE                  128
#define MT5735_MTP_PGM_ADDR_ADDR             0x0002
#define MT5735_MTP_PGM_LEN_ADDR              0x0004
#define MT5735_MTP_PGM_CHKSUM_ADDR           0x0006
#define MT5735_MTP_PGM_DATA_ADDR             0x0008
/* crc check */
#define MT5735_MTP_CRC_ADDR                  0x0000
#define MT5735_MTP_CRC_CMD                   0x0040
/* mtp_status_addr */
#define MT5735_MTP_STATUS_ADDR               0x0000
#define MT5735_MTP_PGM_RUNNING               0x0001
#define MT5735_MTP_PGM_OK                    0x0002
#define MT5735_MTP_PGM_CHKSUM_ERR            0x0004
#define MT5735_MTP_PGM_OTHER_ERR             0x0020
#define MT5735_MTP_CRC_RUNNING               0x0040
#define MT5735_MTP_CRC_ERR                   0x0080
#define MT5735_MTP_CRC_OK                    0x0100

#endif /* _MT5735_CHIP_H_ */
