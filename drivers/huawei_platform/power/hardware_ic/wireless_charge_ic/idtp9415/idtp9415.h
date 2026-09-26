/*
 * idtp9415.h
 *
 * idtp9415 macro, addr etc.
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

#ifndef _IDTP9415_H_
#define _IDTP9415_H_

#define IDTP9415_ADDR_LEN                     2
#define IDTP9415_GPIO_PWR_GOOD_VAL            1

/* rx to tx message */
#define IDTP9415_RX2TX_HEADER_ADDR            0x0050
#define IDTP9415_RX2TX_CMD_ADDR               0x0051
#define IDTP9415_RX2TX_DATA_ADDR              0x0052
#define IDTP9415_RX2TX_PKT_LEN                6
#define IDTP9415_RX2TX_DATA_LEN               4
#define IDTP9415_RX2TX_BUFF_LEN               8
#define IDTP9415_RX2TX_STR_LEN                64
/* tx to rx message */
#define IDTP9415_TX2RX_HEADER_ADDR            0x0058
#define IDTP9415_TX2RX_CMD_ADDR               0x0059
#define IDTP9415_TX2RX_DATA_ADDR              0x005a
#define IDTP9415_TX2RX_PKT_LEN                6
#define IDTP9415_TX2RX_DATA_LEN               4

/* chip id */
#define IDTP9415_CHIP_ID_ADDR                 0x0000
#define IDTP9415_CHIP_ID_LEN                  2
#define IDTP9415_CHIP_ID_H_ADDR               0x5870 /* 0x94 */
#define IDTP9415_CHIP_ID_L_ADDR               0x5874 /* 0x15 */
#define IDTP9415_CHIP_ID                      0x9415
/* hardware version */
#define IDTP9415_HWREV_ADDR                   0x0002
#define IDTP9415_HWREV_LEN                    1
/* customer id */
#define IDTP9415_CUST_ID_ADDR                 0x0003
#define IDTP9415_CUST_ID_LEN                  1
#define IDTP9415_HW_CUST_ID                   0x88
/* minor revision of firmware in MTP */
#define IDTP9415_MINOR_FW_ADDR                0x001c
#define IDTP9415_MINOR_FW_LEN                 2
/* major revision of firmware in MTP */
#define IDTP9415_MAJOR_FW_ADDR                0x001e
#define IDTP9415_MAJOR_FW_LEN                 2

/*
 * idtp9415 rx mode
 */

/* rx power */
#define IDTP9415_PRX_ADDR                     0x0024
#define IDTP9415_PRX_LEN                      2
/* intr_clr */
#define IDTP9415_RX_INT_CLR_ADDR              0x0028
#define IDTP9415_RX_INT_CLR_LEN               4
#define IDTP9415_RX_INT_CLR_OCP               BIT(0)
#define IDTP9415_RX_INT_CLR_OVP               BIT(1)
#define IDTP9415_RX_INT_CLR_OTP               BIT(2)
#define IDTP9415_RX_INT_CLR_RX_READY          BIT(3)
#define IDTP9415_RX_INT_CLR_TXDATA_RCVD       BIT(4)
#define IDTP9415_RX_INT_CLR_MODE_CHANGED      BIT(5)
#define IDTP9415_RX_INT_CLR_LDO_ON            BIT(6)
#define IDTP9415_RX_INT_CLR_LDO_OFF           BIT(7)
#define IDTP9415_RX_INT_CLR_PWR_ON            BIT(8)
#define IDTP9415_RX_INT_CLR_FC_SUCC           BIT(9)
#define IDTP9415_RX_INT_CLR_SEND_PKT_TIMEOUT  BIT(10)
#define IDTP9415_RX_INT_CLR_SEND_PKT_SUCC     BIT(11)
#define IDTP9415_RX_INT_CLR_ID_AUTH_FAIL      BIT(12)
#define IDTP9415_RX_INT_CLR_ID_AUTH_SUCC      BIT(13)
#define IDTP9415_RX_INT_CLR_SLEEP_MODE        BIT(14)
#define IDTP9415_RX_INT_CLR_FC_FAIL           BIT(15)
#define IDTP9415_RX_INT_CLR_DEV_AUTH_FAIL     BIT(16)
#define IDTP9415_RX_INT_CLR_DEV_AUTH_SUCC     BIT(17)
#define IDTP9415_RX_INT_CLR_SEND_RPP          BIT(18)
#define IDTP9415_RX_INT_CLR_SEND_CEP          BIT(19)
#define IDTP9415_ALL_INTR_CLR                 0xFFFFFFFF
/* status */
#define IDTP9415_RX_STATUS_ADDR               0x002C
#define IDTP9415_RX_STATUS_LEN                4
#define IDTP9415_RX_STATUS_OCP                BIT(0)
#define IDTP9415_RX_STATUS_OVP                BIT(1)
#define IDTP9415_RX_STATUS_OTP                BIT(2)
#define IDTP9415_RX_STATUS_RX_READY           BIT(3)
#define IDTP9415_RX_STATUS_TXDATA_RCVD        BIT(4)
#define IDTP9415_RX_STATUS_MODE_CHANGED       BIT(5)
#define IDTP9415_RX_STATUS_LDO_ON             BIT(6)
#define IDTP9415_RX_STATUS_LDO_OFF            BIT(7)
#define IDTP9415_RX_STATUS_PWR_ON             BIT(8)
#define IDTP9415_RX_STATUS_FC_SUCC            BIT(9)
#define IDTP9415_RX_STATUS_SEND_PKT_TIMEOUT   BIT(10)
#define IDTP9415_RX_STATUS_SEND_PKT_SUCC      BIT(11)
#define IDTP9415_RX_STATUS_ID_AUTH_FAIL       BIT(12)
#define IDTP9415_RX_STATUS_ID_AUTH_SUCC       BIT(13)
#define IDTP9415_RX_STATUS_SLEEP_MODE         BIT(14)
#define IDTP9415_RX_STATUS_FC_FAIL            BIT(15)
#define IDTP9415_RX_STATUS_DEV_AUTH_FAIL      BIT(16)
#define IDTP9415_RX_STATUS_DEV_AUTH_SUCC      BIT(17)
#define IDTP9415_RX_STATUS_SEND_RPP           BIT(18)
#define IDTP9415_RX_STATUS_SEND_CEP           BIT(19)
/* interrupt */
#define IDTP9415_RX_INT_ADDR                  0x0030
#define IDTP9415_RX_INT_LEN                   4
#define IDTP9415_RX_INT_OCP                   BIT(0)
#define IDTP9415_RX_INT_OVP                   BIT(1)
#define IDTP9415_RX_INT_OTP                   BIT(2)
#define IDTP9415_RX_INT_READY                 BIT(3)
#define IDTP9415_RX_INT_TXDATA_RCVD           BIT(4)
#define IDTP9415_RX_INT_MODE_CHANGE           BIT(5)
#define IDTP9415_RX_INT_LDO_ON                BIT(6)
#define IDTP9415_RX_INT_LDO_OFF               BIT(7)
#define IDTP9415_RX_INT_POWER_ON              BIT(8)
#define IDTP9415_RX_INT_FC_SUCC               BIT(9)
#define IDTP9415_RX_INT_SEND_PKT_TIMEOUT      BIT(10)
#define IDTP9415_RX_INT_SEND_PKT_SUCC         BIT(11)
#define IDTP9415_RX_INT_ID_AUTH_FAIL          BIT(12)
#define IDTP9415_RX_INT_ID_AUTH_SUCC          BIT(13)
#define IDTP9415_RX_INT_SLEEP_MODE            BIT(14)
#define IDTP9415_RX_INT_FC_FAIL               BIT(15)
#define IDTP9415_RX_INT_DEV_AUTH_FAIL         BIT(16)
#define IDTP9415_RX_INT_DEV_AUTH_SUCC         BIT(17)
#define IDTP9415_RX_INT_SEND_RPP              BIT(18)
#define IDTP9415_RX_INT_SEND_CEP              BIT(19)
/* interrupt clear */
#define IDTP9415_RX_INT_EN_ADDR               0x0034
#define IDTP9415_RX_INT_EN_LEN                4
#define IDTP9415_RX_INT_EN_OCP                BIT(0)
#define IDTP9415_RX_INT_EN_OVP                BIT(1)
#define IDTP9415_RX_INT_EN_OTP                BIT(2)
#define IDTP9415_RX_INT_EN_READY              BIT(3)
#define IDTP9415_RX_INT_EN_TXDATA_RCVD        BIT(4)
#define IDTP9415_RX_INT_EN_MODE_CHANGE        BIT(5)
#define IDTP9415_RX_INT_EN_LDO_ON             BIT(6)
#define IDTP9415_RX_INT_EN_LDO_OFF            BIT(7)
#define IDTP9415_RX_INT_EN_POWER_ON           BIT(8)
#define IDTP9415_RX_INT_EN_FC_SUCC            BIT(9)
#define IDTP9415_RX_INT_EN_SEND_PKT_TIMEOUT   BIT(10)
#define IDTP9415_RX_INT_EN_SEND_PKT_SUCC      BIT(11)
#define IDTP9415_RX_INT_EN_ID_AUTH_FAIL       BIT(12)
#define IDTP9415_RX_INT_EN_ID_AUTH_SUCC       BIT(13)
#define IDTP9415_RX_INT_EN_SLEEP_MODE         BIT(14)
#define IDTP9415_RX_INT_EN_FC_FAIL            BIT(15)
#define IDTP9415_RX_INT_EN_DEV_AUTH_FAIL      BIT(16)
#define IDTP9415_RX_INT_EN_DEV_AUTH_SUCC      BIT(17)
#define IDTP9415_RX_INT_EN_SEND_RPP           BIT(18)
#define IDTP9415_RX_INT_EN_SEND_CEP           BIT(19)
/* battery charge status */
#define IDTP9415_RX_CHG_STATE_ADDR            0x003A
#define IDTP9415_RX_CHG_STATE_LEN             1
/* end of power transfer */
#define IDTP9415_RX_EPT_ADDR                  0x003B
#define IDTP9415_RX_EPT_LEN                   1
/* vout ADC value */
#define IDTP9415_RX_VOUT_ADC_ADDR             0x003C
#define IDTP9415_RX_VOUT_ADC_LEN              2
/* rx_vout_set */
#define IDTP9415_RX_VOUT_SET_ADDR             0x003E
#define IDTP9415_RX_VOUT_SET_LEN              2
#define IDTP9415_RX_VOUT_MAX                  22000
#define IDTP9415_RX_VOUT_MIN                  2800
/* vrect ADC value */
#define IDTP9415_RX_VRECT_ADC_ADDR            0x0040
#define IDTP9415_RX_VRECT_ADC_LEN             2
/* iout register */
#define IDTP9415_RX_IOUT_ADDR                 0x0044
#define IDTP9415_RX_CALIBRATE_IOUT_ADDR       0x008A
#define IDTP9415_RX_IOUT_LEN                  2
/* chip_temp, in degC */
#define IDTP9415_RX_TEMP_ADDR                 0x0046
#define IDTP9415_RX_TEMP_LEN                  2
/* op_freq, in kHZ */
#define IDTP9415_RX_FOP_ADDR                  0x0048
#define IDTP9415_RX_FOP_LEN                   2
/* ilimit set */
#define IDTP9415_RX_ILIM_SADDR                0x004A
#define IDTP9415_RX_ILIM_LEN                  1
/* rx signal strength */
#define IDTP9415_RX_SS_ADDR                   0x004B
#define IDTP9415_RX_SS_LEN                    1
#define IDTP9415_RX_SS_MIN                    0
#define IDTP9415_RX_SS_MAX                    255
/* op mode register */
#define IDTP9415_OP_MODE_ADDR                 0x004D
#define IDTP9415_OP_MODE_LEN                  1
#define IDTP9415_RX_MODE                      BIT(0)
#define IDTP9415_TX_MODE                      BIT(2)
#define IDTP9415_EXT_MODE                     BIT(3)
/* cmd */
#define IDTP9415_RX_CMD_ADDR                  0x004E
#define IDTP9415_RX_CMD_LEN                   2
#define IDTP9415_RX_CMD_VAL                   1
#define IDTP9415_RX_CMD_SEND_MSG              BIT(0)
#define IDTP9415_RX_CMD_SEND_MSG_SHIFT        0
#define IDTP9415_RX_CMD_TG_LDO                BIT(1)
#define IDTP9415_RX_CMD_TG_LDO_SHIFT          1
#define IDTP9415_RX_CMD_SEND_DEV_AUTH         BIT(2)
#define IDTP9415_RX_CMD_SEND_DEV_AUTH_SHIFT   2
#define IDTP9415_RX_CMD_SEND_EPT              BIT(3)
#define IDTP9415_RX_CMD_SEND_EPT_SHIFT        3
#define IDTP9415_RX_CMD_SEND_CSP              BIT(4)
#define IDTP9415_RX_CMD_SEND_CSP_SHIFT        4
#define IDTP9415_RX_CMD_CLR_INT_MASK          BIT(5)
#define IDTP9415_RX_CMD_CLR_INT_SHIFT         5
#define IDTP9415_RX_CMD_SEND_FC               BIT(7)
#define IDTP9415_RX_CMD_SEND_FC_SHIFT         7
#define IDTP9415_RX_CMD_SOFT_START            BIT(8)
#define IDTP9415_RX_CMD_SOFT_START_SHIFT      8
#define IDTP9415_RX_CMD_EXT_ISNS              BIT(9)
#define IDTP9415_RX_CMD_EXT_ISNS_SHIFT        9
#define IDTP9415_RX_CMD_CHANGE_RPP_CTR        BIT(10)
#define IDTP9415_RX_CMD_CHANGE_RPP_CTR_SHIFT  10
#define IDTP9415_RX_CMD_EN_FC_WD              BIT(11)
#define IDTP9415_RX_CMD_EN_FC_WD_SHIFT        11
#define IDTP9415_RX_CMD_DIS_FC_WD             BIT(12)
#define IDTP9415_RX_CMD_DIS_FC_WD_SHIFT       12
#define IDTP9415_RX_CMD_FEED_FC_WD            BIT(13)
#define IDTP9415_RX_CMD_FEED_FC_WD_SHIFT      13
#define IDTP9415_RX_CMD_EPP_RENEG             BIT(15)
#define IDTP9415_RX_CMD_EPP_RENEG_SHIFT       15
/* over voltage clamping voltage */
#define IDTP9415_OVP_TH_DDR                   0x0062
#define IDTP9415_OVP_TH_LEN                   1
/* rx fod */
#define IDTP9415_RX_FOD_ADDR                  0x0068
#define IDTP9415_RX_FOD_LEN                   8 /* 16 bytes */
#define IDTP9415_RX_FOD_TMP_STR_LEN           8
/* fc_volt */
#define IDTP9415_RX_FC_ADDR                   0x0078
#define IDTP9415_RX_FC_LEN                    2
#define IDTP9415_BPCP_SLEEP_TIME              50
#define IDTP9415_BPCP_TIMEOUT                 200
#define IDTP9415_RX_FC_RETRY_CNT              3
#define IDTP9415_RX_FC_SLEEP_TIME             50
#define IDTP9415_RX_FC_TIMEOUT                1500
#define IDTP9415_RX_FC_DEF_VOLT               5000
#define IDTP9415_RX_FC_ERR_LTH                500 /* lower th */
#define IDTP9415_RX_FC_ERR_UTH                1000 /* upper th */
/* vout */
#define IDTP9415_RX_VOUT_ADDR                 0x007C
#define IDTP9415_RX_VOUT_LEN                  2
/* vrect */
#define IDTP9415_RX_VRECT_ADDR                0x007E
#define IDTP9415_RX_VRECT_LEN                 2
/* rpp value */
#define IDTP9415_RX_RPP_VAL_ADDR              0x0080
#define IDTP9415_RX_RPP_VAL_LEN               2
/* cep value */
#define IDTP9415_RX_CEP_VAL_ADDR              0x0082
#define IDTP9415_RX_CEP_VAL_LEN               2
/* fast_charge timer */
#define IDTP9415_RX_FC_WD_TIMEOUT_ADDR        0x0086
#define IDTP9415_RX_FC_WD_TIMEOUT             1000
/* ap power on status */
#define IDTP9415_RX_AP_ON_ADDR                0x0090
#define IDTP9415_RX_AP_ON_LEN                 1
#define IDTP9415_RX_AP_IN_READ                0
#define IDTP9415_RX_AP_ON                     1
/* rpp control */
#define IDTP9415_RX_RPP_CTR_ADDR              0x0091
#define IDTP9415_RX_RPP_CTR_LEN               1
#define IDTP9415_RX_RPP_VAL_UNIT              2 /* in 0.5Watt units */
#define IDTP9415_RX_RPP_VAL_MASK              0x3F /* bit0-5 */
/* set com mode */
#define IDTP9415_RX_COM_CFG_ADDR              0x0092
#define IDTP9415_RX_COM_CFG_LEN               1
#define IDTP9415_COM_EN                       0x03 /* bit0-1 */
#define IDTP9415_CLAMP_EN                     0x0C /* bit2-3 */
/* vrect coef */
#define IDTP9415_RX_VRECT_COEF_ADDR           0x4A2
#define IDTP9415_RX_VRECT_COEF_LEN            6
#define IDTP9415_RX_PWR_KNEE_ADDR             0x4A2
#define IDTP9415_RX_VRCORR_FACTOR_ADDR        0x4A3
#define IDTP9415_RX_VRMAX_COR_ADDR            0x4A4 /* vdiff=gap*21/4095 */
#define IDTP9415_RX_VRMIN_COR_ADDR            0x4A6

#define IDTP9415_SHUTDOWN_SLEEP_TIME          200
#define IDTP9415_RCV_MSG_SLEEP_TIME           100
#define IDTP9415_RCV_MSG_SLEEP_CNT            10
#define IDTP9415_WAIT_FOR_ACK_SLEEP_TIME      100
#define IDTP9415_WAIT_FOR_ACK_RETRY_CNT       5
#define IDTP9415_SNED_MSG_RETRY_CNT           2
#define IDTP9415_RX_TMP_BUFF_LEN              32

/*
 * idtp9415 tx mode
 */

/* ap power on status */
#define IDTP9415_TX_AP_ON_ADDR                 0x0026
#define IDTP9415_TX_AP_ON_LEN                  1
#define IDTP9415_TX_AP_ON                      1
/* tx intr_clr */
#define IDTP9415_TX_INT_CLR_ADDR               0x0028
#define IDTP9415_TX_INT_CLR_LEN                4
#define IDTP9415_TX_INT_CLR_EPT_TYPE           BIT(0)
#define IDTP9415_TX_INT_CLR_START_DPING        BIT(1)
#define IDTP9415_TX_INT_CLR_GET_SS             BIT(2)
#define IDTP9415_TX_INT_CLR_GET_ID             BIT(3)
#define IDTP9415_TX_INT_CLR_GET_CFG            BIT(4)
#define IDTP9415_TX_INT_CLR_GET_PPP            BIT(5)
#define IDTP9415_TX_INT_CLR_GET_DPING          BIT(6)
/* tx interrupt status register */
#define IDTP9415_TX_STATUS_ADDR                0x002C
#define IDTP9415_TX_STATUS_LEN                 4
#define IDTP9415_TX_STATUS_EPT_TYPE            BIT(0)
#define IDTP9415_TX_STATUS_START_DPING         BIT(1)
#define IDTP9415_TX_STATUS_GET_SS              BIT(2)
#define IDTP9415_TX_STATUS_GET_ID              BIT(3)
#define IDTP9415_TX_STATUS_GET_CFG             BIT(4)
#define IDTP9415_TX_STATUS_GET_PPP             BIT(5)
#define IDTP9415_TX_STATUS_GET_DPING           BIT(6)
#define IDTP9415_TX_STATUS_TX_INIT             BIT(7)
/* tx interrupt register */
#define IDTP9415_TX_INT_ADDR                   0x0030
#define IDTP9415_TX_INT_LEN                    4
#define IDTP9415_TX_INT_EPT_TYPE               BIT(0)
#define IDTP9415_TX_INT_START_DPING            BIT(1)
#define IDTP9415_TX_INT_GET_SS                 BIT(2)
#define IDTP9415_TX_INT_GET_ID                 BIT(3)
#define IDTP9415_TX_INT_GET_CFG                BIT(4)
#define IDTP9415_TX_INT_GET_PPP                BIT(5)
#define IDTP9415_TX_INT_GET_DPING              BIT(6)
#define IDTP9415_TX_INT_TX_INIT                BIT(7)
#define IDTP9415_TX_INT_SET_VTX                BIT(8)
/* tx interrupt enable */
#define IDTP9415_TX_INT_EN_ADDR                0x0034
#define IDTP9415_TX_INT_EN_LEN                 4
#define IDTP9415_TX_INT_EN_EPT_TYPE            BIT(0)
#define IDTP9415_TX_INT_EN_START_DPING         BIT(1)
#define IDTP9415_TX_INT_EN_GET_SS              BIT(2)
#define IDTP9415_TX_INT_EN_GET_ID              BIT(3)
#define IDTP9415_TX_INT_EN_GET_CFG             BIT(4)
#define IDTP9415_TX_INT_EN_GET_PPP             BIT(5)
#define IDTP9415_TX_INT_EN_GET_DPING           BIT(6)
#define IDTP9415_TX_INT_EN_TX_INIT             BIT(7)
#define IDTP9415_TX_INT_EN_SET_VTX             BIT(8)
/* tx fop register */
#define IDTP9415_TX_FOP_ADDR                   0x0062
/* tx max fop */
#define IDTP9415_TX_MAX_FOP_ADDR               0x0064
#define IDTP9415_TX_MAX_FOP_VAL                148 /* kHz */
/* tx min fop */
#define IDTP9415_TX_MIN_FOP_ADDR               0x0066
#define IDTP9415_TX_MIN_FOP_VAL                111 /* kHz */
/* tx ping frequency register */
#define IDTP9415_TX_PING_FREQUENCY_ADDR        0x0068
#define IDTP9415_TX_PING_HCLK                  60000
#define IDTP9415_TX_PING_FREQUENCY_MIN         100 /* kHz */
#define IDTP9415_TX_PING_FREQUENCY_MAX         150 /* kHz */
#define IDTP9415_TX_PING_FREQUENCY_INIT        130
/* tx ocp register */
#define IDTP9415_TX_OCP_ADDR                   0x006A
#define IDTP9415_TX_OCP2_ADDR                  0x0086
#define IDTP9415_TX_OCP_VAL                    2000
/* tx ovp register */
#define IDTP9415_TX_OVP_ADDR                   0x006C
#define IDTP9415_TX_OVP_VAL                    12000
/* tx iin register */
#define IDTP9415_TX_IIN_ADDR                   0x006E
/* tx vin register */
#define IDTP9415_TX_VIN_ADDR                   0x0070
/* tx vrect register */
#define IDTP9415_TX_VRECT_ADDR                 0x0072
/* tx pocp register */
#define IDTP9415_TX_POCP_FAST_ADDR             0x0082
#define IDTP9415_TX_POCP_SLOW_ADDR             0x0084
#define IDTP9415_TX_POCP_VAL                   1600
/* tx ept type register */
#define IDTP9415_TX_EPT_TYPE_ADDR              0X0074
#define IDTP9415_TX_EPT_CMD                    BIT(0)
#define IDTP9415_TX_EPT_SS                     BIT(1)
#define IDTP9415_TX_EPT_ID                     BIT(2)
#define IDTP9415_TX_EPT_XID                    BIT(3)
#define IDTP9415_TX_EPT_CFG_COUNT_ERR          BIT(4)
#define IDTP9415_TX_EPT_PCH                    BIT(5)
#define IDTP9415_TX_EPT_FIRSTCEP               BIT(6)
#define IDTP9415_TX_EPT_TIMEOUT                BIT(7)
#define IDTP9415_TX_EPT_CEP_TIMEOUT            BIT(8)
#define IDTP9415_TX_EPT_RPP_TIMEOUT            BIT(9)
#define IDTP9415_TX_EPT_OCP                    BIT(10)
#define IDTP9415_TX_EPT_OVP                    BIT(11)
#define IDTP9415_TX_EPT_LVP                    BIT(12)
#define IDTP9415_TX_EPT_FOD                    BIT(13)
#define IDTP9415_TX_EPT_OTP                    BIT(14)
#define IDTP9415_TX_EPT_POCP                   BIT(15)
/* tx cmd register */
#define IDTP9415_TX_CMD_ADDR                   0x0076
#define IDTP9415_TX_CMD_VAL                    1
#define IDTP9415_TX_CMD_EN_MASK                BIT(0)
#define IDTP9415_TX_CMD_EN_SHIFT               0
#define IDTP9415_TX_CMD_CLRINT_MASK            BIT(1)
#define IDTP9415_TX_CMD_CLRINT_SHIFT           1
#define IDTP9415_TX_CMD_DIS_MASK               BIT(2)
#define IDTP9415_TX_CMD_DIS_SHIFT              2
#define IDTP9415_TX_CMD_SEND_FSK_MASK          BIT(3)
#define IDTP9415_TX_CMD_SEND_FSK_SHIFT         3
#define IDTP9415_TX_CMD_WD_MASK                BIT(4)
#define IDTP9415_TX_CMD_WD_SHIFT               4
#define IDTP9415_TX_CMD_FOD_EN_MASK            BIT(5)
#define IDTP9415_TX_CMD_FOD_EN_SHIFT           5
#define IDTP9415_TX_CMD_BOOST_CFG_MASK         BIT(6)
#define IDTP9415_TX_CMD_BOOST_CFG_SHIFT        6
/* tx ping interval register */
#define IDTP9415_TX_PING_INTERVAL_ADDR         0x0079
#define IDTP9415_TX_PING_INTERVAL_MIN          200 /* ms */
#define IDTP9415_TX_PING_INTERVAL_MAX          1000 /* ms */
#define IDTP9415_TX_PING_INTERVAL_INIT         500 /* ms */
#define IDTP9415_TX_PING_INTERVAL_STEP         10
/* tx chip temp register */
#define IDTP9415_CHIP_TEMP_ADDR                0x007A
/* tx mode vset register */
#define IDTP9415_PS_TX_GPIO_ADDR               0x008A
#define IDTP9415_PS_TX_GPIO_OPEN               0x00
#define IDTP9415_PS_TX_GPIO_PU                 0x01
#define IDTP9415_PS_TX_GPIO_PD                 0x02
#define IDTP9415_PS_TX_VOLT_5V5                5500
#define IDTP9415_PS_TX_VOLT_6V8                6800
#define IDTP9415_PS_TX_VOLT_10V                10000
/* tx fod */
#define IDTP9415_TX_FOD_ADDR                   0x0092
#define IDTP9415_TX_FOD_LEN                    9
/* tx_power value register, in mw */
#define IDTP9415_TX_PTX_ADDR                   0x00A8
#define IDTP9415_TX_PTX_LEN                    2
/* rx_power value register, in mw */
#define IDTP9415_TX_PRX_ADDR                   0x00AA
#define IDTP9415_TX_PRX_LEN                    2
/* tx_ploss value register, in mw */
#define IDTP9415_TX_PLOSS_ADDR                 0x00AC
#define IDTP9415_TX_PLOSS_LEN                  2
/* tx_cep_value register */
#define IDTP9415_TX_CEP_ADDR                   0x0498
#define IDTP9415_TX_CEP_LEN                    1

/*
 * idtp9415 mtp
 */
#define IDTP9415_MTP_STRT_ADDR                 0x0000
#define IDTP9415_MTP_MAX_LEN                   0x6000 /* 24k */
#define IDTP9415_MTP_DATA_ADDR                 0x0400
#define IDTP9415_BTLOADR_DATA_ADDR             0x0800
#define IDTP9415_M0_ADDR                       0x3040
#define IDTP9415_M0_RESET                      0x80

#define IDTP9415_MTP_BUSY                      0x01
#define IDTP9415_MTP_WR_OK                     0x02
#define IDTP9415_MTP_WR_ERR                    0x04
#define IDTP9415_MTP_CHKSUM_ERR                0x08

#define IDTP9415_PAGE_SIZE                     128 /* 16bytes unit */

#define IDTP9415_MTP_STATUS_UNKNOWN            0
#define IDTP9415_MTP_STATUS_GOOD               1
#define IDTP9415_MTP_STATUS_BAD                2

#define IDTP9415_RX_VOUT_LTH                   4000
#define IDTP9415_EXT_ISNS_ADDR                 0x0088
#define IDTP9415_CHECK_VOUT                    7500
#define IDTP9415_CHECK_IOUT                    300
#define IDTP9415_IOUT_MAX                      2100
#define IDTP9415_IOUT_EXT                      1500

struct idtp9415_chip_info {
	u16 chip_id;
	u16 major_fw_ver;
	u16 minor_fw_ver;
};

struct idtp9415_pgm_str {
	u16 status;
	u16 addr;
	u16 code_len;
	u16 chk_sum;
	u8 fw[IDTP9415_PAGE_SIZE];
	u8 padding[8]; /* 8-bytes padding to round to 16-byte boundary */
};

struct idtp9415_dev_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct delayed_work mtp_check_work;
	struct delayed_work internal_iout_check_work;
	struct delayed_work external_iout_check_work;
	struct mutex mutex_irq;
	unsigned int ic_type;
	u16 rx_fod_5v[IDTP9415_RX_FOD_LEN];
	u16 pu_rx_fod_5v[IDTP9415_RX_FOD_LEN];
	u16 rx_fod_9v[IDTP9415_RX_FOD_LEN];
	u16 rx_fod_9v_cp39s_hk[IDTP9415_RX_FOD_LEN];
	u16 rx_fod_12v[IDTP9415_RX_FOD_LEN];
	u16 rx_fod_15v[IDTP9415_RX_FOD_LEN];
	u16 rx_fod_15v_cp39s_hk[IDTP9415_RX_FOD_LEN];
	u8 tx_fod_coef[IDTP9415_TX_FOD_LEN];
	u8 rx_vrect_coef_5v[IDTP9415_RX_VRECT_COEF_LEN];
	u8 rx_vrect_coef_9v[IDTP9415_RX_VRECT_COEF_LEN];
	u8 rx_vrect_coef_12v[IDTP9415_RX_VRECT_COEF_LEN];
	u8 rx_vrect_coef_sc[IDTP9415_RX_VRECT_COEF_LEN];
	int mtp_status;
	bool irq_active;
	bool pu_shell_flag;
	bool need_chk_pu_shell;
	int max_iout;
	int rx_ss_good_lth;
	int gpio_en;
	int gpio_en_valid_val;
	int gpio_sleep_en;
	int gpio_int;
	int gpio_pwr_good;
	int irq_int;
	u32 irq_val;
	u16 ept_type;
	int irq_cnt;
	int iout_check_cnt;
	int iout_check_valid_cnt;
	int rx_imax;
	int iout_sum;
	int ibus_sum;
};

#endif /* _IDTP9415_H_ */
