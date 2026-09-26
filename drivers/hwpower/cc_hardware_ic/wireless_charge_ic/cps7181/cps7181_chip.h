/*
 * cps7181_chip.h
 *
 * cps7181 registers, chip info, etc.
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

#ifndef _CPS7181_CHIP_H_
#define _CPS7181_CHIP_H_

#define CPS7181_SW_I2C_ADDR                   0x30 /* SW: software */
#define CPS7181_HW_I2C_ADDR                   0x31 /* HW: hardware */
#define CPS7181_ADDR_LEN                      2
#define CPS7181_GPIO_PWR_GOOD_VAL             1

/* i2c test */
#define CPS7181_I2C_TEST_ADDR                 0x1D80
#define CPS7181_I2C_TEST_LEN                  4
#define CPS7181_I2C_TEST_VAL                  0x12345678 /* any data but 0 */
/* chip id register */
#define CPS7181_CHIP_ID_ADDR                  0x1D00
#define CPS7181_CHIP_ID_LEN                   4
#define CPS7181_CHIP_ID                       0x7181
/* mtp_version register */
#define CPS7181_MTP_VER_ADDR                  0x1D10
#define CPS7181_MTP_VER_LEN                   2
/* op mode register */
#define CPS7181_OP_MODE_ADDR                  0x1D14
#define CPS7181_OP_MODE_LEN                   1
#define CPS7181_OP_MODE_NA                    0x00
#define CPS7181_OP_MODE_BP                    0x01 /* back_powered */
#define CPS7181_OP_MODE_TX                    0x02
#define CPS7181_OP_MODE_RX                    0x03
/* crc val register */
#define CPS7181_CRC_ADDR                      0x1D16
#define CPS7181_CRC_LEN                       2
/* send_msg_data register */
#define CPS7181_SEND_MSG_HEADER_ADDR          0x1D82
#define CPS7181_SEND_MSG_CMD_ADDR             0x1D83
#define CPS7181_SEND_MSG_DATA_ADDR            0x1D84
/* send_msg: bit[0]: header, bit[1]: cmd, bit[2:5]: data */
#define CPS7181_SEND_MSG_DATA_LEN             4
#define CPS7181_SEND_MSG_PKT_LEN              6
/* rcvd_msg_data register */
#define CPS7181_RCVD_MSG_HEADER_ADDR          0x1DC2
#define CPS7181_RCVD_MSG_CMD_ADDR             0x1DC3
#define CPS7181_RCVD_MSG_DATA_ADDR            0x1DC4
/* rcvd_msg: bit[0]: header, bit[1]: cmd, bit[2:5]: data */
#define CPS7181_RCVD_MSG_DATA_LEN             4
#define CPS7181_RCVD_MSG_PKT_LEN              6
#define CPS7181_RCVD_PKT_BUFF_LEN             8
#define CPS7181_RCVD_PKT_STR_LEN              64

/*
 * rx mode
 */

/* rx_irq_en register */
#define CPS7181_RX_IRQ_EN_ADDR                0x1D40
#define CPS7181_RX_IRQ_EN_LEN                 2
/* rx_irq_latch register */
#define CPS7181_RX_IRQ_ADDR                   0x1D42
#define CPS7181_RX_IRQ_LEN                    2
#define CPS7181_RX_IRQ_POWER_ON               BIT(0)
#define CPS7181_RX_IRQ_LDO_OFF                BIT(1)
#define CPS7181_RX_IRQ_LDO_ON                 BIT(2)
#define CPS7181_RX_IRQ_READY                  BIT(3)
#define CPS7181_RX_IRQ_FSK_ACK                BIT(4)
#define CPS7181_RX_IRQ_FSK_TIMEOUT            BIT(5)
#define CPS7181_RX_IRQ_FSK_PKT                BIT(6)
#define CPS7181_RX_IRQ_VRECT_OVP              BIT(7)
#define CPS7181_RX_IRQ_OTP                    BIT(8)
#define CPS7181_RX_IRQ_OCP                    BIT(9)
#define CPS7181_RX_IRQ_HOCP                   BIT(10)
#define CPS7181_RX_IRQ_OSP                    BIT(11)
#define CPS7181_RX_IRQ_MLDO_OVP               BIT(12)
/* rx_irq_clr register */
#define CPS7181_RX_IRQ_CLR_ADDR               0x1D44
#define CPS7181_RX_IRQ_CLR_LEN                2
#define CPS7181_RX_IRQ_CLR_ALL                0xFFFF
/* rx_cmd register */
#define CPS7181_RX_CMD_ADDR                   0x1D46
#define CPS7181_RX_CMD_LEN                    1
#define CPS7181_RX_CMD_VAL                    1
#define CPS7181_RX_CMD_SEND_MSG               BIT(0)
#define CPS7181_RX_CMD_SEND_MSG_SHIFT         0
#define CPS7181_RX_CMD_SEND_MSG_RPLY          BIT(0)
#define CPS7181_RX_CMD_SEND_MSG_RPLY_SHIFT    0
#define CPS7181_RX_CMD_SEND_FC                BIT(1)
#define CPS7181_RX_CMD_SEND_FC_SHIFT          1
#define CPS7181_RX_CMD_SEND_EPT               BIT(2)
#define CPS7181_RX_CMD_SEND_EPT_SHIFT         2
#define CPS7181_RX_CMD_SYS_RST                BIT(3)
#define CPS7181_RX_CMD_SYS_RST_SHIFT          3
/* rx_ept_msg register */
#define CPS7181_RX_EPT_MSG_ADDR               0x1F00
#define CPS7181_RX_EPT_MSG_LEN                1
/* rx_vout_set register */
#define CPS7181_RX_VOUT_SET_ADDR              0x1F40
#define CPS7181_RX_VOUT_SET_LEN               2
#define CPS7181_RX_VOUT_MAX                   20000
#define CPS7181_RX_VOUT_MIN                   3500
/* rx_func_en regiter */
#define CPS7181_RX_FUNC_EN_ADDR               0x1F4D
#define CPS7181_RX_FUNC_EN_LEN                1
#define CPS7181_RX_FUNC_EN                    1
#define CPS7181_RX_FUNC_DIS                   0
#define CPS7181_RX_HV_WDT_EN_MASK             BIT(2)
#define CPS7181_RX_HV_WDT_EN_SHIFT            2
#define CPS7181_RX_CMALL_EN_MASK              (BIT(3) | BIT(4))
#define CPS7181_RX_CMALL_EN_SHIFT             3
#define CPS7181_RX_CMALL_EN_VAL               0x3
#define CPS7181_RX_CMAB_EN_VAL                0x2
#define CPS7181_RX_CM_POLARITY_MASK           BIT(5)
#define CPS7181_RX_CM_POLARITY_SHIFT          5
#define CPS7181_RX_CM_POSITIVE                0
#define CPS7181_RX_CM_NEGTIVE                 1
#define CPS7181_RX_RP24BIT_EN_MASK            BIT(6)
#define CPS7181_RX_RP24BIT_EN_SHIFT           6
#define CPS7181_RX_EXT_VCC_EN_MASK            BIT(7)
#define CPS7181_RX_EXT_VCC_EN_SHIFT           7
/* rx_fc_volt register */
#define CPS7181_RX_FC_VOLT_ADDR               0x1F50
#define CPS7181_RX_FC_VOLT_LEN                2
#define CPS7181_RX_BPCP_SLEEP_TIME            50
#define CPS7181_RX_BPCP_TIMEOUT               200
#define CPS7181_RX_FC_VOUT_RETRY_CNT          3
#define CPS7181_RX_FC_VOUT_SLEEP_TIME         50
#define CPS7181_RX_FC_VOUT_TIMEOUT            1500
#define CPS7181_RX_FC_VOUT_DEFAULT            5000
#define CPS7181_RX_FC_VOUT_ERR_LTH            500 /* lower threshold */
#define CPS7181_RX_FC_VOUT_ERR_UTH            1000 /* upper threshold */
/* rx_max_pwr for RP val calculation */
#define CPS7181_RX_RP_PMAX_ADDR               0x1F55
#define CPS7181_RX_RP_PMAX_LEN                1
#define CPS7181_RX_RP_VAL_UNIT                2
/* rx_wdt_timeout register, in ms */
#define CPS7181_RX_WDT_TIMEOUT_ADDR           0x1F56
#define CPS7181_RX_WDT_TIMEOUT_LEN            2
#define CPS7181_RX_WDT_TIMEOUT                1000
/* rx_ldo_cfg: ldo_drop0-1 && ldo_cur_thres0-1 */
#define CPS7181_RX_LDO_CFG_ADDR               0x1F58
#define CPS7181_RX_LDO_CFG_LEN                8
/* rx_rp_type */
#define CPS7181_RX_RP_TYPE_ADDR               0x1F60
#define CPS7181_RX_RP_TYPE_LEN                1
#define CPS7181_RX_RP_NO_REPLY                0
#define CPS7181_RX_RP_WITH_REPLY              1
/* rx_signal_strength register */
#define CPS7181_RX_SS_ADDR                    0x1F84
#define CPS7181_RX_SS_LEN                     1
#define CPS7181_RX_SS_MIN                     0
#define CPS7181_RX_SS_MAX                     255
/* rx_ce_val register */
#define CPS7181_RX_CE_VAL_ADDR                0x1F86
#define CPS7181_RX_CE_VAL_LEN                 1
/* rx_rp_val register */
#define CPS7181_RX_RP_VAL_ADDR                0x1F88
#define CPS7181_RX_RP_VAL_LEN                 2
/* rx_op_freq register, in kHZ */
#define CPS7181_RX_OP_FREQ_ADDR               0x1F8A
#define CPS7181_RX_OP_FREQ_LEN                2
/* rx_vrect register */
#define CPS7181_RX_VRECT_ADDR                 0x1F94
#define CPS7181_RX_VRECT_LEN                  2
/* rx_iout register */
#define CPS7181_RX_IOUT_ADDR                  0x1F96
#define CPS7181_RX_IOUT_LEN                   2
/* rx_vout register */
#define CPS7181_RX_VOUT_ADDR                  0x1F98
#define CPS7181_RX_VOUT_LEN                   2
/* rx_chip_temp register, in degC */
#define CPS7181_RX_CHIP_TEMP_ADDR             0x1F9A
#define CPS7181_RX_CHIP_TEMP_LEN              2
/* rx_fod_coef register */
#define CPS7181_RX_FOD_ADDR                   0x1F05
#define CPS7181_RX_FOD_LEN                    24
#define CPS7181_RX_FOD_STR_LEN                96
#define CPS7181_RX_FOD_TMP_STR_LEN            4

/*
 * tx mode
 */

/* tx_irq_en register */
#define CPS7181_TX_IRQ_EN_ADDR                0x1D40
#define CPS7181_TX_IRQ_EN_LEN                 2
#define CPS7181_TX_IRQ_EN_RPP_TIMEOUT         BIT(6)
#define CPS7181_TX_IRQ_EN_RPP_SHIFT           6
/* tx_irq_latch register */
#define CPS7181_TX_IRQ_ADDR                   0x1D42
#define CPS7181_TX_IRQ_LEN                    2
#define CPS7181_TX_IRQ_START_PING             BIT(0)
#define CPS7181_TX_IRQ_SS_PKG_RCVD            BIT(1)
#define CPS7181_TX_IRQ_ID_PKT_RCVD            BIT(2)
#define CPS7181_TX_IRQ_CFG_PKT_RCVD           BIT(3)
#define CPS7181_TX_IRQ_PP_PKT_RCVD            BIT(4)
#define CPS7181_TX_IRQ_EPT_PKT_RCVD           BIT(5)
#define CPS7181_TX_IRQ_RPP_TIMEOUT            BIT(6)
#define CPS7181_TX_IRQ_CEP_TIMEOUT            BIT(7)
#define CPS7181_TX_IRQ_DPING_RCVD             BIT(8)
#define CPS7181_TX_IRQ_TX_INIT                BIT(9)
#define CPS7181_TX_IRQ_RPP_TYPE_ERR           BIT(11)
/* tx_irq_clr register */
#define CPS7181_TX_IRQ_CLR_ADDR               0x1D44
#define CPS7181_TX_IRQ_CLR_LEN                2
#define CPS7181_TX_IRQ_CLR_ALL                0xFFFF
/* tx_cmd register */
#define CPS7181_TX_CMD_ADDR                   0x1D47
#define CPS7181_TX_CMD_LEN                    1
#define CPS7181_TX_CMD_VAL                    1
#define CPS7181_TX_CMD_CRC_CHK                BIT(0)
#define CPS7181_TX_CMD_CRC_CHK_SHIFT          0
#define CPS7181_TX_CMD_EN_TX                  BIT(1)
#define CPS7181_TX_CMD_EN_TX_SHIFT            1
#define CPS7181_TX_CMD_DIS_TX                 BIT(2)
#define CPS7181_TX_CMD_DIS_TX_SHIFT           2
#define CPS7181_TX_CMD_SEND_MSG               BIT(3)
#define CPS7181_TX_CMD_SEND_MSG_SHIFT         3
#define CPS7181_TX_CMD_SYS_RST                BIT(4)
#define CPS7181_TX_CMD_SYS_RST_SHIFT          4
/* tx_ocp_thres register, in mA */
#define CPS7181_TX_OCP_TH_ADDR                0x1E42
#define CPS7181_TX_OCP_TH_LEN                 2
#define CPS7181_TX_OCP_TH                     2000
/* tx_uvp_thres register, in mA */
#define CPS7181_TX_UVP_TH_ADDR                0x1E44
#define CPS7181_TX_UVP_TH_LEN                 2
#define CPS7181_TX_UVP_TH                     4000
/* tx_ovp_thres register, in mV */
#define CPS7181_TX_OVP_TH_ADDR                0x1E46
#define CPS7181_TX_OVP_TH_LEN                 2
#define CPS7181_TX_OVP_TH                     14000
/* tx_min_fop, in kHz */
#define CPS7181_TX_MIN_FOP_ADDR               0x1E48
#define CPS7181_TX_MIN_FOP_LEN                1
#define CPS7181_TX_MIN_FOP                    113
/* tx_max_fop, in kHz */
#define CPS7181_TX_MAX_FOP_ADDR               0x1E49
#define CPS7181_TX_MAX_FOP_LEN                1
#define CPS7181_TX_MAX_FOP                    145
/* tx_ping_freq, in kHz */
#define CPS7181_TX_PING_FREQ_ADDR             0x1E4A
#define CPS7181_TX_PING_FREQ_LEN              1
#define CPS7181_TX_PING_FREQ                  135
#define CPS7181_TX_PING_FREQ_MIN              100
#define CPS7181_TX_PING_FREQ_MAX              150
/* tx_ping_ocp, in mA */
#define CPS7181_TX_PING_OCP_ADDR              0x1E4E
#define CPS7181_TX_PING_OCP_TH                1400
/* tx_ping_interval, in ms */
#define CPS7181_TX_PING_INTERVAL_ADDR         0x1E54
#define CPS7181_TX_PING_INTERVAL_LEN          2
#define CPS7181_TX_PING_INTERVAL_MIN          110
#define CPS7181_TX_PING_INTERVAL_MAX          1000
#define CPS7181_TX_PING_INTERVAL              500
/* tx_fod_ploss_cnt register */
#define CPS7181_TX_PLOSS_CNT_ADDR             0x1E57
#define CPS7181_TX_PLOSS_CNT_LEN              1
/* tx_fod_ploss_thres register, in mW */
#define CPS7181_TX_PLOSS_V0_TH                0x1E58
#define CPS7181_TX_PLOSS_V1_TH                0x1E5A
#define CPS7181_TX_PLOSS_V2_TH                0x1E5C
/* tx_customer register */
#define CPS7181_TX_PS_CTRL_ADDR               0x1E5F
#define CPS7181_TX_PS_CTRL_LEN                1
#define CPS7181_TX_PS_GPIO_PU                 0x0
#define CPS7181_TX_PS_GPIO_OPEN               0x1
#define CPS7181_TX_PS_GPIO_PD                 0x2
#define CPS7181_TX_PS_VOLT_5V5                5500
#define CPS7181_TX_PS_VOLT_6V8                6800
#define CPS7181_TX_PS_VOLT_10V                10000
#define CPS7181_TX_PS_VOLT_6V                 6000
#define CPS7181_TX_PS_VOLT_6V9                6900
/* tx_ilimit register */
#define CPS7181_TX_ILIM_ADDR                  0x1E60
#define CPS7181_TX_ILIM_LEN                   2
#define CPS7181_TX_ILIM_MIN                   500
#define CPS7181_TX_ILIM_MAX                   2000
/* tx_oper_freq register, in Hz */
#define CPS7181_TX_OP_FREQ_ADDR               0x1E84
#define CPS7181_TX_OP_FREQ_LEN                2
/* tx_vin register, in mV */
#define CPS7181_TX_VIN_ADDR                   0x1E86
#define CPS7181_TX_VIN_LEN                    2
/* tx_vrect register, in mV */
#define CPS7181_TX_VRECT_ADDR                 0x1E88
#define CPS7181_TX_VRECT_LEN                  2
/* tx_iin register, in mA */
#define CPS7181_TX_IIN_ADDR                   0x1E8A
#define CPS7181_TX_IIN_LEN                    2
/* tx_cep register */
#define CPS7181_TX_CEP_ADDR                   0x1E8D
#define CPS7181_TX_CEP_LEN                    1
/* tx_rpp register */
#define CPS7181_TX_RPP_VALUE                  0x1E8E
#define CPS7181_TX_RPP_LEN                    2
/* tx_ept_type register */
#define CPS7181_TX_EPT_SRC_ADDR               0x1E90
#define CPS7181_TX_EPT_SRC_CLEAR              0
#define CPS7181_TX_EPT_SRC_LEN                1
#define CPS7181_TX_EPT_SRC_WRONG_PKT          BIT(0)
#define CPS7181_TX_EPT_SRC_AC_DET             BIT(1)
#define CPS7181_TX_EPT_SRC_SSP                BIT(2)
#define CPS7181_TX_EPT_SRC_RX_EPT             BIT(3)
#define CPS7181_TX_EPT_SRC_CEP_TIMEOUT        BIT(4)
#define CPS7181_TX_EPT_SRC_RPP_TIMEOUT        BIT(5)
#define CPS7181_TX_EPT_SRC_OCP                BIT(6)
#define CPS7181_TX_EPT_SRC_OVP                BIT(7)
#define CPS7181_TX_EPT_SRC_UVP                BIT(8)
#define CPS7181_TX_EPT_SRC_FOD                BIT(9)
#define CPS7181_TX_EPT_SRC_OTP                BIT(10)
#define CPS7181_TX_EPT_SRC_POCP               BIT(11)
/* tx_pwm_duty register */
#define CPS7181_TX_PWM_DUTY_ADDR              0x1E92
#define CPS7181_TX_PWM_DUTY_LEN               1
/* tx_chip_temp register, in degC */
#define CPS7181_TX_CHIP_TEMP_ADDR             0x1E94
#define CPS7181_TX_CHIP_TEMP_LEN              2
/* rx_power value register, in mw */
#define CPS7181_TX_PRX_ADDR                   0x1E98
#define CPS7181_TX_PRX_LEN                    2
/* tx_power value register, in mw */
#define CPS7181_TX_PTX_ADDR                   0x1E9A
#define CPS7181_TX_PTX_LEN                    2
/* tx_ploss value register, in mw */
#define CPS7181_TX_PLOSS_ADDR                 0x1E9C
#define CPS7181_TX_PLOSS_LEN                  2
/* tx_ploss threshold register, in mw */
#define CPS7181_TX_PTX_TH_ADDR                0x1E9E
#define CPS7181_TX_PTX_TH_LEN                 2
/* tx_fod_th_id register */
#define CPS7181_TX_FOD_TH_ID_ADDR             0x1EA1
#define CPS7181_TX_FOD_TH_ID_LEN              1

/*
 * firmware register
 */

/* hw cmd */
#define CPS7181_CMD_UNLOCK_I2C                0xF500
#define CPS7181_I2C_CODE                      0x19E5
#define CPS7181_CMD_HOLD_MCU                  0xF501
#define CPS7181_HOLD_MCU                      0x153F
#define CPS7181_CMD_SET_HI_ADDR               0xF503
#define CPS7181_CMD_INC_MODE                  0xF505
#define CPS7181_BYTE_INC                      0x0004
#define CPS7181_WORD_INC                      0x0006
/* sram addr */
#define CPS7181_SRAM_HI_ADDR                  0x2000
#define CPS7181_SRAM_BTL_BUFF                 0x0000 /* 2k */
#define CPS7181_SRAM_MTP_BUFF0                0x0800 /* 2k */
#define CPS7181_SRAM_MTP_BUFF1                0x1000 /* 2k */
#define CPS7181_SRAM_MTP_BUFF_SIZE            2048
#define CPS7181_SRAM_BTL_VER_ADDR             0x180C
/* sram cmd */
#define CPS7181_SRAM_CMD_LEN                  4
#define CPS7181_SRAM_STRAT_CMD_ADDR           0x20001800
#define CPS7181_STRAT_CARRY_BUF0              0x00000010
#define CPS7181_STRAT_CARRY_BUF1              0x00000020
#define CPS7181_START_CHK_BTL                 0x000000B0
#define CPS7181_START_CHK_MTP                 0x00000090
#define CPS7181_START_CHK_PGM                 0x00000080
#define CPS7181_SRAM_CHK_CMD_ADDR             0x20001804
#define CPS7181_CHK_SUCC                      0x55
#define CPS7181_CHK_FAIL                      0xAA /* 0x66: running */
/* programming addr */
#define CPS7181_PGM_CMD_LEN                   4
#define CPS7181_PGM_EN_TEST_ADDR              0x40012120
#define CPS7181_PGM_EN_TEST                   0x00001250
#define CPS7181_PGM_DIS_TEST                  0x00000000
#define CPS7181_PGM_EN_CARRY_ADDR             0x40012EE8
#define CPS7181_PGM_EN_CARRY                  0x0000D148
#define CPS7181_PGM_DIS_CARRY                 0x00000000
/* sys_set addr */
#define CPS7181_SYS_CMD_LEN                   4
#define CPS7181_SYS_SOFT_REST_ADDR            0x40040070
#define CPS7181_SYS_SOFT_REST                 0x61A00000
#define CPS7181_SYS_REMAP_EN_ADDR             0x400400A0
#define CPS7181_SYS_REMAP_EN                  0x000000FF
#define CPS7181_SYS_TRIM_DIS_ADDR             0x40040010
#define CPS7181_SYS_TRIM_DIS                  0x00008000

#endif /* _CPS7181_CHIP_H_ */
