/*
 * stwlc88_chip.h
 *
 * stwlc88 registers, chip info, etc.
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

#ifndef _STWLC88_CHIP_H_
#define _STWLC88_CHIP_H_

#define ST88_ADDR_LEN                      2
#define ST88_HW_ADDR_LEN                   4
#define ST88_HW_ADDR_F_LEN                 5 /* len with flag */
#define ST88_HW_ADDR_FLAG                  0xFA
#define ST88_GPIO_PWR_GOOD_VAL             1

/* chip_info: 0x0000 ~ 0x000C */
#define ST88_CHIP_INFO_ADDR                0x0000
#define ST88_CHIP_INFO_LEN                 14
/* chip id register */
#define ST88_CHIP_ID_ADDR                  0x0000
#define ST88_CHIP_ID_LEN                   2
#define ST88_CHIP_ID                       88
#define ST88_CHIP_ID_AB                    0xFFFF /* abnormal chip id */
/* chip rev register */
#define ST88_CHIP_REV_ADDR                 0x0002
#define ST88_CHIP_REV_LEN                  1
/* cust_id register */
#define ST88_CUST_ID_ADDR                  0x0003
#define ST88_CUST_ID_LEN                   1
/* rom_id register */
#define ST88_ROM_ID_ADDR                   0x0004
#define ST88_ROM_ID_LEN                    2
/* ftp_patch_id register */
#define ST88_FTP_PATCH_ID_ADDR             0x0006
#define ST88_FTP_PATCH_ID_LEN              2
/* ram_patch_id register */
#define ST88_RAM_PATCH_ID_ADDR             0x0008
#define ST88_RAM_PATCH_ID_LEN              2
/* cfg_id register */
#define ST88_CFG_ID_ADDR                   0x000A
#define ST88_CFG_ID_LEN                    2
/* pe_id register */
#define ST88_PE_ID_ADDR                    0x000C
#define ST88_PE_ID_LEN                     2
/* op mode register */
#define ST88_OP_MODE_ADDR                  0x000E
#define ST88_OP_MODE_LEN                   1
#define ST88_OP_MODE_NA                    0x00
#define ST88_OP_MODE_SA                    0x01 /* stand_alone */
#define ST88_OP_MODE_RX                    0x02
#define ST88_OP_MODE_TX                    0x03
/* device_id register */
#define ST88_DIE_ID_ADDR                   0x0010
#define ST88_DIE_ID_LEN                    16
#define ST88_DIE_ID_STR_LEN                64
#define ST88_DIE_ID_TMP_STR_LEN            3
/* sys_cmd register */
#define ST88_SYS_CMD_ADDR                  0x0020
#define ST88_SYS_CMD_LEN                   2
#define ST88_SYS_CMD_VAL                   1
#define ST88_SYS_CMD_SW2TX                 BIT(0)
#define ST88_SYS_CMD_SW2TX_SHIFT           0
#define ST88_SYS_CMD_SW2RX                 BIT(1)
#define ST88_SYS_CMD_SW2RX_SHIFT           1
#define ST88_SYS_CMD_FTP_WR                BIT(2)
#define ST88_SYS_CMD_FTP_WR_SHIFT          2
#define ST88_SYS_CMD_FTP_RD                BIT(3)
#define ST88_SYS_CMD_FTP_RD_SHIFT          3
#define ST88_SYS_CMD_FTP_ERASE             BIT(4)
#define ST88_SYS_CMD_FTP_ERASE_SHIFT       4
#define ST88_SYS_CMD_FTP_FULL_ERASE        BIT(5)
#define ST88_SYS_CMD_FTP_FULL_ERASE_SHIFT  5
#define ST88_SYS_CMD_FW_RESET              BIT(6)
#define ST88_SYS_CMD_FW_RESET_SHIFT        6
#define ST88_SYS_CMD_MI2C_WR               BIT(8)
#define ST88_SYS_CMD_MI2C_WR_SHIFT         8
#define ST88_SYS_CMD_MI2C_WR_RD            BIT(9)
#define ST88_SYS_CMD_MI2C_WR_RD_SHIFT      9
#define ST88_SYS_CMD_MI2C_RD               BIT(10)
#define ST88_SYS_CMD_MI2C_RD_SHIFT         10
#define ST88_SYS_CMD_MEM_RD                BIT(11)
#define ST88_SYS_CMD_MEM_RD_SHIFT          11
#define ST88_SYS_CMD_MEM_WR                BIT(12)
#define ST88_SYS_CMD_MEM_WR_SHIFT          12
#define ST88_SYS_CMD_EXEC_ADDR             BIT(13)
#define ST88_SYS_CMD_EXEC_ADDR_SHIFT       13
/* ftp_wr_pwd register */
#define ST88_FTP_WR_PWD_ADDR               0x0022
#define ST88_FTP_WR_PWD_LEN                1
#define ST88_FTP_WR_UNLOCK_PWD             0xC5 /* others, lock */
#define ST88_FTP_WR_LOCK_PWD               0x66
/* aux_len register */
#define ST88_AUX_LEN_ADDR                  0x0023
#define ST88_AUX_LEN_LEN                   1
/* aux_addr register */
#define ST88_AUX_ADDR_ADDR                 0x0024
#define ST88_AUX_ADDR_LEN                  4
/* ram_clean_start_addr register */
#define ST88_RAM_CLEAN_STRAT_ADDR_ADDR     0x0028
#define ST88_RAM_CLEAN_STRAT_ADDR_LEN      2
/* sys_err_latch register */
#define ST88_SYS_ERR_ADDR                  0x002c
#define ST88_SYS_ERR_LEN                   4
#define ST88_SYS_ERR_M0_HARD_FAULT         BIT(0)
#define ST88_SYS_ERR_HW_WDT                BIT(1)
#define ST88_SYS_ERR_FTP_ERR               BIT(2)
#define ST88_SYS_ERR_MI2C_ERR              BIT(3)
#define ST88_SYS_ERR_FTP_PE_HDR_ERR        0x0100 /* bit[8:9] = 1 */
#define ST88_SYS_ERR_FTP_PE_CRC_ERR        0x0200 /* bit[8:9] = 2 */
#define ST88_SYS_ERR_FTP_CFG_HDR_ERR       0x0400 /* bit[10:11] = 1 */
#define ST88_SYS_ERR_FTP_CFG_CRC_ERR       0x0800 /* bit[10:11] = 2 */
#define ST88_SYS_ERR_FTP_PATCH_HDR_ERR     0x1000 /* bit[8:9] = 1 */
#define ST88_SYS_ERR_FTP_PATCH_CRC_ERR     0x2000 /* bit[8:9] = 2 */
#define ST88_SYS_ERR_FTP_PI_HDR_ERR        0x4000 /* bit[8:9] = 1 */
#define ST88_SYS_ERR_FTP_PI_CRC_ERR        0x8000 /* bit[8:9] = 2 */
/* aux_data register */
#define ST88_AUX_DATA_ADDR                 0x0180
#define ST88_AUX_DATA_LEN                  384
/* send_msg_data register */
#define ST88_SEND_MSG_HEADER_ADDR          0x0180
#define ST88_SEND_MSG_CMD_ADDR             0x0181
#define ST88_SEND_MSG_DATA_ADDR            0x0182
/* send_msg: bit[0]: header, bit[1]: cmd, bit[2:5]: data */
#define ST88_SEND_MSG_DATA_LEN             4
#define ST88_SEND_MSG_PKT_LEN              6
/* rcvd_msg_data register */
#define ST88_RCVD_MSG_HEADER_ADDR          0x0190
#define ST88_RCVD_MSG_CMD_ADDR             0x0191
#define ST88_RCVD_MSG_DATA_ADDR            0x0192
/* rcvd_msg: bit[0]: header, bit[1]: cmd, bit[2:5]: data */
#define ST88_RCVD_MSG_DATA_LEN             4
#define ST88_RCVD_MSG_PKT_LEN              6
#define ST88_RCVD_PKT_BUFF_LEN             8
#define ST88_RCVD_PKT_STR_LEN              64

/*
 * rx mode
 */

/* rx_rp_val register, last RP value sent */
#define ST88_RX_RP_VAL_ADDR                0x005C
#define ST88_RX_RP_VAL_LEN                 2
/* rx_ce_val register, last CE value sent */
#define ST88_RX_CE_VAL_ADDR                0x005E
#define ST88_RX_CE_VAL_LEN                 1
/* rx_ask_mod_cfg register */
#define ST88_RX_ASK_CFG_ADDR               0x0060
#define ST88_RX_ASK_CFG_LEN                1
#define ST88_ALL_CAP_POSITIVE              0xE1
#define ST88_CAP_C_NEGATIVE                0x91
#define ST88_CAP_BC_NEGATIVE               0xD1
/* rx_irq_en register */
#define ST88_RX_IRQ_EN_ADDR                0x0080
#define ST88_RX_IRQ_EN_LEN                 4
/* rx_irq_clr register */
#define ST88_RX_IRQ_CLR_ADDR               0x0084
#define ST88_RX_IRQ_CLR_LEN                4
#define ST88_RX_IRQ_CLR_ALL                0xFFFFFFFF
/* rx_irq_latch register */
#define ST88_RX_IRQ_ADDR                   0x0088
#define ST88_RX_IRQ_LEN                    4
#define ST88_RX_IRQ_OTP                    BIT(0)
#define ST88_RX_IRQ_OCP                    BIT(1)
#define ST88_RX_IRQ_OVP                    BIT(2)
#define ST88_RX_IRQ_SYS_ERR                BIT(3)
#define ST88_RX_IRQ_DATA_RCVD              BIT(5)
#define ST88_RX_IRQ_OUTPUT_ON              BIT(6)
#define ST88_RX_IRQ_OUTPUT_OFF             BIT(7)
#define ST88_RX_IRQ_SEND_PKT_SUCC          BIT(8)
#define ST88_RX_IRQ_SEND_PKT_TIMEOUT       BIT(9)
#define ST88_RX_IRQ_POWER_ON               BIT(10)
#define ST88_RX_IRQ_READY                  BIT(11)
/* rx_status register */
#define ST88_RX_STATUS_ADDR                0x008C
#define ST88_RX_STATUS_LEN                 4
/* rx_cmd register */
#define ST88_RX_CMD_ADDR                   0x0090
#define ST88_RX_CMD_LEN                    2
#define ST88_RX_CMD_VAL                    1
#define ST88_RX_CMD_VOUT_ON                BIT(0)
#define ST88_RX_CMD_VOUT_ON_SHIFT          0
#define ST88_RX_CMD_VOUT_OFF               BIT(1)
#define ST88_RX_CMD_VOUT_OFF_SHIFT         1
#define ST88_RX_CMD_SEND_MSG               BIT(2)
#define ST88_RX_CMD_SEND_MSG_SHIFT         2
#define ST88_RX_CMD_SEND_MSG_RPLY          BIT(3)
#define ST88_RX_CMD_SEND_MSG_RPLY_SHIFT    3
#define ST88_RX_CMD_SEND_EPT               BIT(4)
#define ST88_RX_CMD_SEND_EPT_SHIFT         4
#define ST88_RX_CMD_SEND_DTS               BIT(5)
#define ST88_RX_CMD_SEND_DTS_SHIFT         5
#define ST88_RX_CMD_SEND_FC                BIT(6)
#define ST88_RX_CMD_SEND_FC_SHIFT          6
/* rx_vrect register */
#define ST88_RX_VRECT_ADDR                 0x0092
#define ST88_RX_VRECT_LEN                  2
/* rx_vout register */
#define ST88_RX_VOUT_ADDR                  0x0094
#define ST88_RX_VOUT_LEN                   2
/* rx_iout register */
#define ST88_RX_IOUT_ADDR                  0x0096
#define ST88_RX_IOUT_LEN                   2
/* rx_chip_temp register, in degC */
#define ST88_RX_CHIP_TEMP_ADDR             0x0098
#define ST88_RX_CHIP_TEMP_LEN              2
/* rx_op_freq register, in kHZ */
#define ST88_RX_OP_FREQ_ADDR               0x009A
#define ST88_RX_OP_FREQ_LEN                2
/* rx_ntc register */
#define ST88_RX_NTC_ADDR                   0x009C
#define ST88_RX_NTC_LEN                    2
/* rx_adc_in1 register */
#define ST88_RX_ADC_IN1_ADDR               0x009E
#define ST88_RX_ADC_IN1_LEN                2
/* rx_adc_in2 register */
#define ST88_RX_ADC_IN2_ADDR               0x00A0
#define ST88_RX_ADC_IN2_LEN                2
/* rx_adc_in3 register */
#define ST88_RX_ADC_IN3_ADDR               0x00A2
#define ST88_RX_ADC_IN3_LEN                2
/* rx_ctrl_err register, in mV */
#define ST88_RX_CTRL_ERR_ADDR              0x00A4
#define ST88_RX_CTRL_ERR_LEN               2
/* rx_rcvd_pwr register, in mW */
#define ST88_RX_RCVD_PWR_ADDR              0x00A6
#define ST88_RX_RCVD_PWR_LEN               2
/* rx_signal_strength register */
#define ST88_RX_SS_ADDR                    0x00A8
#define ST88_RX_SS_LEN                     1
#define ST88_RX_SS_MIN                     0
#define ST88_RX_SS_MAX                     255
/* rx_vout_set register */
#define ST88_RX_VOUT_SET_ADDR              0x00B2
#define ST88_RX_VOUT_SET_LEN               2
#define ST88_RX_VOUT_SET_STEP              25 /* mV */
#define ST88_RX_VOUT_MAX                   22000
#define ST88_RX_VOUT_MIN                   25
/* rx_vrect_adj register */
#define ST88_RX_VRECT_ADJ_ADDR             0x00B4
#define ST88_RX_VRECT_ADJ_LEN              1
/* rx_ilim_set register */
#define ST88_RX_ILIM_SET_ADDR              0x00B5
#define ST88_RX_ILIM_SET_LEN               1
#define ST88_RX_ILIM_SET_STEP              100 /* mV */
#define ST88_RX_ILIM_MAX                   1500
#define ST88_RX_ILIM_MIN                   100
/* rx_fod_coef register, 0x00c7: rx_ser in 4mohm */
#define ST88_RX_FOD_ADDR                   0x00B6
#define ST88_RX_FOD_LEN                    18
#define ST88_RX_FOD_TMP_STR_LEN            4
/* rx_ldo_cfg: ldo_drop0-3 && ldo_cur_thres1-3 */
#define ST88_RX_LDO_CFG_ADDR               0x00C8
#define ST88_RX_LDO_CFG_LEN                7
#define ST88_RX_LDO_VDROP_STEP             16 /* mV, vrect-vout */
#define ST88_RX_LDO_CUR_TH_STEP            8 /* mA, iout */
/* rx_ept_msg register, ept reason to be included when sending ept packet */
#define ST88_RX_EPT_MSG_ADDR               0x00CF
#define ST88_RX_EPT_MSG_LEN                1
/* rx_fc_volt register */
#define ST88_RX_FC_VOLT_ADDR               0x00D0
#define ST88_RX_FC_VOLT_LEN                2
#define ST88_RX_BPCP_SLEEP_TIME            50
#define ST88_RX_BPCP_TIMEOUT               200
#define ST88_RX_FC_VOUT_RETRY_CNT          3
#define ST88_RX_FC_VOUT_SLEEP_TIME         50
#define ST88_RX_FC_VOUT_TIMEOUT            1500
#define ST88_RX_FC_VOUT_DEFAULT            5000
#define ST88_RX_FC_VOUT_ERR_LTH            500 /* lower threshold */
#define ST88_RX_FC_VOUT_ERR_UTH            1000 /* upper threshold */
/* rx_wdt_timeout register, in ms */
#define ST88_RX_WDT_TIMEOUT_ADDR           0x00D2
#define ST88_RX_WDT_TIMEOUT_LEN            2
#define ST88_RX_WDT_TIMEOUT                1000
/* rx_wdt_feed register */
#define ST88_RX_WDT_FEED_ADDR              0x00D4
#define ST88_RX_WDT_FEED_LEN               1
/* rx_24bit_rp_set register */
#define ST88_RX_RPP_SET_ADDR               0x00D5
#define ST88_RX_RPP_SET_LEN                1
#define ST88_RX_RPP_VAL_UNIT               2 /* in 0.5Watt units */
#define ST88_RX_RPP_VAL_MASK               0x3F
/* rx_fc_vrect_diff register */
#define ST88_RX_FC_VRECT_DIFF_ADDR         0x00D6
#define ST88_RX_FC_VRECT_DIFF_LEN          2
#define ST88_RX_FC_VRECT_DIFF              2000 /* mV */
#define ST88_RX_FC_VRECT_DIFF_STEP         32
/* rx_dts_send register */
#define ST88_RX_DTS_SEND_ADDR              0x00D8
#define ST88_RX_DTS_SEND_LEN               4
/* rx_dts_rcvd register */
#define ST88_RX_DTS_RCVD_ADDR              0x00DC
#define ST88_RX_DTS_RCVD_LEN               4

/*
 * tx mode
 */

/* tx_irq_en register */
#define ST88_TX_IRQ_EN_ADDR                0x0100
#define ST88_TX_IRQ_EN_LEN                 4
#define ST88_TX_IRQ_EN_VAL                 0xFEFFFEC7
/* tx_irq_clr register */
#define ST88_TX_IRQ_CLR_ADDR               0x0104
#define ST88_TX_IRQ_CLR_LEN                4
#define ST88_TX_IRQ_CLR_ALL                0xFFFFFFFF
/* tx_irq_latch register */
#define ST88_TX_IRQ_ADDR                   0x0108
#define ST88_TX_IRQ_LEN                    4
#define ST88_TX_IRQ_OTP                    BIT(0)
#define ST88_TX_IRQ_OCP                    BIT(1)
#define ST88_TX_IRQ_OVP                    BIT(2)
#define ST88_TX_IRQ_SYS_ERR                BIT(3)
#define ST88_TX_IRQ_RPP_RCVD               BIT(4)
#define ST88_TX_IRQ_CEP_RCVD               BIT(5)
#define ST88_TX_IRQ_SEND_PKT_SUCC          BIT(6)
#define ST88_TX_IRQ_DPING_RCVD             BIT(7)
#define ST88_TX_IRQ_CEP_TIMEOUT            BIT(8)
#define ST88_TX_IRQ_RPP_TIMEOUT            BIT(9)
#define ST88_TX_IRQ_EPT_PKT_RCVD           BIT(10)
#define ST88_TX_IRQ_START_PING             BIT(11)
#define ST88_TX_IRQ_SS_PKG_RCVD            BIT(12)
#define ST88_TX_IRQ_ID_PKT_RCVD            BIT(13)
#define ST88_TX_IRQ_CFG_PKT_RCVD           BIT(14)
#define ST88_TX_IRQ_PP_PKT_RCVD            BIT(15)
#define ST88_TX_IRQ_FOD_DET                BIT(19)
#define ST88_TX_IRQ_ACK_RCVD               BIT(25)
/* tx_irq_status register */
#define ST88_TX_IRQ_STATUS_ADDR            0x010C
#define ST88_TX_IRQ_STATUS_LEN             4
/* tx_cmd register */
#define ST88_TX_CMD_ADDR                   0x0110
#define ST88_TX_CMD_LEN                    2
#define ST88_TX_CMD_VAL                    1
#define ST88_TX_CMD_EN_TX                  BIT(0)
#define ST88_TX_CMD_EN_TX_SHIFT            0
#define ST88_TX_CMD_DIS_TX                 BIT(1)
#define ST88_TX_CMD_DIS_TX_SHIFT           1
#define ST88_TX_CMD_SEND_MSG               BIT(2)
#define ST88_TX_CMD_SEND_MSG_SHIFT         2
#define ST88_TX_CMD_AUTO_SEND_MSG          BIT(5)
#define ST88_TX_CMD_AUTO_SEND_MSG_SHIFT    5
/* tx_ept_type register */
#define ST88_TX_EPT_SRC_ADDR               0x0112
#define ST88_TX_EPT_SRC_LEN                3
#define ST88_TX_EPT_SRC_OVP                BIT(0) /* stop */
#define ST88_TX_EPT_SRC_OCP                BIT(1) /* stop */
#define ST88_TX_EPT_SRC_OTP                BIT(2) /* stop */
#define ST88_TX_EPT_SRC_FOD                BIT(3) /* stop */
#define ST88_TX_EPT_SRC_CMD                BIT(4) /* AP stop */
#define ST88_TX_EPT_SRC_RX                 BIT(5) /* re ping receive ept */
#define ST88_TX_EPT_SRC_CEP_TIMEOUT        BIT(6) /* re ping */
#define ST88_TX_EPT_SRC_RPP_TIMEOUT        BIT(7) /* re ping */
#define ST88_TX_EPT_SRC_RX_RST             BIT(8) /* re ping receive ept */
#define ST88_TX_EPT_SRC_SYS_ERR            BIT(9) /* stop */
#define ST88_TX_EPT_SRC_PING_TIMEOUT       BIT(10) /* re ping */
#define ST88_TX_EPT_SRC_SS                 BIT(11) /* re ping */
#define ST88_TX_EPT_SRC_ID                 BIT(12) /* re ping */
#define ST88_TX_EPT_SRC_CFG                BIT(13) /* re ping */
#define ST88_TX_EPT_SRC_CFG_CNT            BIT(14) /* re ping */
#define ST88_TX_EPT_SRC_PCH                BIT(15) /* re ping */
#define ST88_TX_EPT_SRC_XID                BIT(16) /* re ping */
#define ST88_TX_EPT_SRC_NEGO               BIT(17) /* re ping */
#define ST88_TX_EPT_SRC_NEGO_TIMEOUT       BIT(18) /* re ping */
/* tx_cep val register */
#define ST88_TX_CEP_ADDR                   0x0115
#define ST88_TX_CEP_LEN                    1
/* tx_vrect register, in mV */
#define ST88_TX_VRECT_ADDR                 0x0116
#define ST88_TX_VRECT_LEN                  2
/* tx_vin register, in mV */
#define ST88_TX_VIN_ADDR                   0x0118
#define ST88_TX_VIN_LEN                    2
/* tx_iin register, in mA */
#define ST88_TX_IIN_ADDR                   0x011A
#define ST88_TX_IIN_LEN                    2
/* tx_chip_temp register, in degC */
#define ST88_TX_CHIP_TEMP_ADDR             0x011C
#define ST88_TX_CHIP_TEMP_LEN              2
/* tx_oper_freq register, in 4Hz */
#define ST88_TX_OP_FREQ_ADDR               0x011E
#define ST88_TX_OP_FREQ_LEN                2
#define ST88_TX_OP_FREQ_STEP               4
/* tx_ntc register */
#define ST88_TX_NTC_ADDR                   0x0120
#define ST88_TX_NTC_LEN                    2
/* tx_adc_in1 register */
#define ST88_TX_ADC_IN1_ADDR               0x0122
#define ST88_TX_ADC_IN1_LEN                2
/* tx_adc_in2 register */
#define ST88_TX_ADC_IN2_ADDR               0x0124
#define ST88_TX_ADC_IN2_LEN                2
/* tx_adc_in3 register */
#define ST88_TX_ADC_IN3_ADDR               0x0126
#define ST88_TX_ADC_IN3_LEN                2
/* tx_pwr_tfrd_to_rx register */
#define ST88_TX_TFRD_PWR_ADDR              0x0128
#define ST88_TX_TFRD_PWR_LEN               2
/* tx_pwr_rcvd_by_rx register */
#define ST88_TX_RCVD_PWR_ADDR              0x012A
#define ST88_TX_RCVD_PWR_LEN               2
/* tx_ptc_ref_pwr register */
#define ST88_TX_PTC_REF_PWR_ADDR           0x012C
#define ST88_TX_PTC_REF_PWR_LEN            1
/* tx_customer register */
#define ST88_TX_CUST_CTRL_ADDR             0x0134
#define ST88_TX_CUST_CTRL_LEN              1
#define ST88_TX_PS_GPIO_MASK               (BIT(0) | BIT(1))
#define ST88_TX_PS_GPIO_SHIFT              0
#define ST88_TX_PS_GPIO_OPEN               0x1
#define ST88_TX_PS_GPIO_PU                 0x2
#define ST88_TX_PS_GPIO_PD                 0x3
#define ST88_TX_PS_VOLT_5V5                5500
#define ST88_TX_PS_VOLT_6V8                6800
#define ST88_TX_PS_VOLT_10V                10000
#define ST88_TX_PT_BRIDGE_MASK             (BIT(2) | BIT(3))
#define ST88_TX_PT_BRIDGE_SHIFT            2
#define ST88_TX_PT_BRIDGE_NO_CHANGE        0 /* same as ping */
#define ST88_TX_PT_HALF_BRIDGE             1 /* manual half bridge mode */
#define ST88_TX_PT_FULL_BRIDGE             2 /* manual full bridge mode */
#define ST88_TX_PT_AUTO_SW_BRIDGE          3 /* auto switch */
#define ST88_TX_PING_BRIDGE_MASK           BIT(4)
#define ST88_TX_PING_BRIDGE_SHIFT          4
#define ST88_TX_PING_FULL_BRIDGE           0 /* ping in full bridge mode */
#define ST88_TX_PING_HALF_BRIDGE           1 /* ping in half bridge mode */
#define ST88_TX_MANUAL_BRIDGE_FC_MASK      BIT(5)
#define ST88_TX_MANUAL_BRIDGE_FC_SHIFT     5
#define ST88_TX_MANUAL_BRIDGE_FC_EN        1
/* tx_duty register */
#define ST88_TX_DUTY_ADDR                  0x0136
#define ST88_TX_DUTY_LEN                   1
/* tx_pmax register */
#define ST88_TX_PMAX_ADDR                  0x0137
#define ST88_TX_PMAX_LEN                   1
#define ST88_TX_RPP_VAL_UNIT               2 /* in 0.5Watt units */
/* tx_fod_th_id register */
#define ST88_TX_FOD_TH_ID_ADDR             0x013A
#define ST88_TX_FOD_TH_ID_LEN              1
/* tx_ovp_thres register, in mV */
#define ST88_TX_OVP_TH_ADDR                0x0140
#define ST88_TX_OVP_TH_LEN                 1
#define ST88_TX_OVP_TH                     20000
#define ST88_TX_OVP_TH_STEP                500
/* tx_ocp_thres register, in mA */
#define ST88_TX_OCP_TH_ADDR                0x0141
#define ST88_TX_OCP_TH_LEN                 1
#define ST88_TX_OCP_TH                     1800
#define ST88_TX_OCP_TH_STEP                100
/* tx_otp register, in degC */
#define ST88_TX_OTP_TH_ADDR                0x0142
#define ST88_TX_OTP_TH_LEN                 1
#define ST88_TX_OTP_TH                     100
/* tx_ilimit register */
#define ST88_TX_ILIM_ADDR                  0x0143
#define ST88_TX_ILIM_LEN                   1
#define ST88_TX_ILIM_STEP                  16
#define ST88_TX_ILIM_MIN                   500
#define ST88_TX_ILIM_MAX                   2000
/* tx_max_fop, in kHz */
#define ST88_TX_MAX_FOP_ADDR               0x0144
#define ST88_TX_MAX_FOP_LEN                1
#define ST88_TX_MAX_FOP                    145
/* tx_min_fop, in kHz */
#define ST88_TX_MIN_FOP_ADDR               0x0145
#define ST88_TX_MIN_FOP_LEN                1
#define ST88_TX_MIN_FOP                    113
/* tx_ping_freq, in kHz */
#define ST88_TX_PING_FREQ_ADDR             0x0146
#define ST88_TX_PING_FREQ_LEN              1
#define ST88_TX_PING_FREQ                  135
#define ST88_TX_PING_FREQ_MIN              100
#define ST88_TX_PING_FREQ_MAX              180
/* tx_ping_interval, in ms */
#define ST88_TX_PING_INTERVAL_ADDR         0x0147
#define ST88_TX_PING_INTERVAL_LEN          1
#define ST88_TX_PING_INTERVAL_STEP         50
#define ST88_TX_PING_INTERVAL_MIN          0
#define ST88_TX_PING_INTERVAL_MAX          1000
#define ST88_TX_PING_INTERVAL              500
/* tx_max_duty cycle, in % */
#define ST88_TX_MAX_DC_ADDR                0x0148
#define ST88_TX_MAX_DC_LEN                 1
#define ST88_TX_MAX_DC                     50
/* tx_min_duty cycle, in % */
#define ST88_TX_MIN_DC_ADDR                0x0149
#define ST88_TX_MIN_DC_LEN                 1
#define ST88_TX_MIN_DC                     36
/* tx_fod_ploss_thres register, in mW */
#define ST88_TX_PLOSS_TH_ADDR              0x014A
#define ST88_TX_PLOSS_TH_LEN               1
#define ST88_TX_PLOSS_TH_STEP              32
#define ST88_TX_PLOSS_TH_VAL               6500 /* mW */
/* tx_fod_ploss_cnt register */
#define ST88_TX_PLOSS_CNT_ADDR             0x014B
#define ST88_TX_PLOSS_CNT_LEN              1
#define ST88_TX_PLOSS_CNT_VAL              2
/* tx_listen_to_cep_max_cnt register, (0.8*cnt)ms */
#define ST88_TX_CEP_MAX_CNT_ADDR           0x014C
#define ST88_TX_CEP_MAX_CNT_LEN            1
#define ST88_TX_CEP_MAX_CNT_VAL            5
/* tx_ping_duty_cycle register */
#define ST88_TX_PING_DC_ADDR               0x014E
#define ST88_TX_PING_DC_LEN                1
#define ST88_TX_PING_DC_VAL                50
/*
 * tx_fod, in mv
 * 0x0162: tx_vin < 7.5V
 * 0x0164: tx_vin >= 7.5V, half bridge mode
 * 0x0166: tx_vin >= 7.5V, full bridge mode, without FC
 * 0x0168: tx_vin >= 7.5V, full bridge mode, after FC
 * 0x016A: tx_vin >= 12.5V, tx_iin < 1.2 (1.1) A
 * 0x016C: tx_vin >= 12.5V, tx_iin >= 1.2A
 */
#define ST88_TX_FOD_ADDR                   0x0162
#define ST88_TX_FOD_LEN                    12

/*
 * hardware register
 */

/* ftp data register */
#define ST88_FTP_PATCH_ADDR                0x00060000
#define ST88_FTP_SOFT_PROGRAM_VAL1         0x00000005
#define ST88_FTP_SOFT_PROGRAM_VAL2         0x00000006
#define ST88_FTP_SOFT_PROGRAM_LEN          4
#define ST88_CHUNK_FTP_READ_SIZE           128
#define ST88_FTP_SECTOR_BYTES_SIZE         128
#define ST88_FTP_PATCH_SSEC_ID             0 /* ftp patch start sector index */
#define ST88_FTP_CFG_SSEC_ID               93 /* ftp cfg start sector index */
#define ST88_FTP_ERVRF_MODE                1
#define ST88_FTP_PRVRF_MODE                2
/* ftp test mode config register */
#define ST88_FDMA_WIN_MODE_ADDR            0x00068008
#define ST88_FDMA_WIN_MODE_LEN             4
#define ST88_ERVRF_MODE_VAL                0x00000022
#define ST88_PRVRF_MODE_VAL                0x20000022
/* ftp test mode dump register */
#define ST88_TEST_MODE_DUMP_ADDR           0x0006800C
#define ST88_TEST_MODE_DUMP_LEN            4
#define ST88_TEST_MODE_DUMP_VAL            0x00020087
/* standby word register */
#define ST88_FTP_STANDBY_WORD_ADDR         0x0006FFFC
#define ST88_FTP_STANDBY_WORD_VAL          0x00000001
#define ST88_FTP_STANDBY_WORD_LEN          4
/* chip reset register */
#define ST88_RST_ADDR                      0x2001C00C
#define ST88_RST_SYS                       BIT(0)
#define ST88_RST_M0                        BIT(1)
/* system clock register */
#define ST88_SYS_CLK_DIV_ADDR              0x2001C00E
#define ST88_SYS_CLK_DIV_VAL               0x01
/* built-in load setting register */
#define ST88_ILOAD_ADDR                    0x2001C176
#define ST88_ILOAD_DIS_VAL                 0x01
/* rx_ldo5v_ps_ctrl regiter */
#define ST88_RX_LDO5V_EN_ADDR              0x2001C19B
#define ST88_RX_LDO5V_EN                   1
#define ST88_RX_LDO5V_DIS                  0
/* ftp control register */
#define ST88_FTP_CTRL_ADDR                 0x2001C1D4
#define ST88_FTP_CTRL_VAL                  0xF1
#define ST88_FTP_CTRL_VAL1                 0x6B
#define ST88_FTP_TRIM_LOCK_NUM_ADDR        0x2001C1D6
#define ST88_FTP_TRIM_LOCK_VAL             0x60
#define ST88_FTP_USER_LOCK_NUM_ADDR        0x2001C1D7
#define ST88_FTP_USER_LOCK_VAL             0x23
#define ST88_FTP_STAT_ADDR                 0x2001C1D8
#define ST88_FTP_STAT_SOFT_PROGRAM         BIT(2)
#define ST88_FTP_VDDCP_CTRL_ADDR           0x2001C1DA
#define ST88_FTP_VDDCP_CTRL_VAL            0x01
/* test mode config register */
#define ST88_TEST_MODE_ADDR                0x2001C1F8
#define ST88_TEST_MODE_LEN                 4
#define ST88_TEST_MODE_VAL                 0xB1000007

#endif /* _STWLC88_CHIP_H_ */
