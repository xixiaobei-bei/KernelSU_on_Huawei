/*
 * sm5450_protocol.h
 *
 * sm5450 protocol header file
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

#ifndef _SM5450_PROTOCOL_H_
#define _SM5450_PROTOCOL_H_

#include "sm5450.h"

/* scp */
#define SM5450_SCP_ACK_RETRY_CYCLE                     4
#define SM5450_SCP_ACK_RETRY_CYCLE_1                   10
#define SM5450_SCP_RESTART_TIME                        4
#define SM5450_SCP_DATA_LEN                            8
#define SM5450_SCP_RETRY_TIME                          2
#define SM5450_SCP_POLL_TIME                           100 /* 100ms */
#define SM5450_SCP_DETECT_MAX_COUT                     20 /* scp detect max count */
#define SM5450_SCP_NO_ERR                              0
#define SM5450_SCP_IS_ERR                              1

#define SM5450_SCP_CMD_SBRRD                           0x0c
#define SM5450_SCP_CMD_SBRWR                           0x0b
#define SM5450_SCP_CMD_MBRRD                           0x1c
#define SM5450_SCP_CMD_MBRWR                           0x1b

/* fcp adapter vol value */
#define SM5450_FCP_ADAPTER_MAX_VOL                     12000
#define SM5450_FCP_ADAPTER_MIN_VOL                     5000
#define SM5450_FCP_ADAPTER_RST_VOL                     5000
#define SM5450_FCP_ADAPTER_VOL_CHECK_ERROR             500
#define SM5450_FCP_ADAPTER_VOL_CHECK_POLLTIME          100
#define SM5450_FCP_ADAPTER_VOL_CHECK_TIMEOUT           10

/* SCP_CTL reg=0x1B */
#define SM5450_SCP_CTL_REG                             0x1B
#define SM5450_SCP_CTL_EN_SCP_MASK                     BIT(7)
#define SM5450_SCP_CTL_EN_SCP_SHIFT                    7
#define SM5450_SCP_CTL_SCP_DET_EN_MASK                 BIT(6)
#define SM5450_SCP_CTL_SCP_DET_EN_SHIFT                6
#define SM5450_SCP_CTL_EN_STIMER_MASK                  BIT(4)
#define SM5450_SCP_CTL_EN_STIMER_SHIFT                 4
#define SM5450_SCP_CTL_MSTR_RST_MASK                   BIT(3)
#define SM5450_SCP_CTL_MSTR_RST_SHIFT                  3
#define SM5450_SCP_CTL_SNDCMD_MASK                     BIT(1)
#define SM5450_SCP_CTL_SNDCMD_SHIFT                    1

#define SM5450_SCP_CTL_SNDCMD_START                    1
#define SM5450_SCP_CTL_SNDCMD_RESET                    0
#define SM5450_SCP_CTL_REG_INIT                        SM5450_SCP_CTL_EN_STIMER_MASK
#define SM5450_SCP_CTL_WDT_RESET                       0

/* SCP_ISR1 reg=0x1C */
#define SM5450_SCP_ISR1_REG                            0x1C
#define SM5450_SCP_ISR1_SCP_DM_DET_FLAG_MASK           BIT(7)
#define SM5450_SCP_ISR1_SCP_DM_DET_FLAG_EN_SCP_SHIFT   7
#define SM5450_SCP_ISR1_SCP_DET_FAIL_FLAG_MASK         BIT(6)
#define SM5450_SCP_ISR1_SCP_DET_FAIL_FLAG_SHIFT        6
#define SM5450_SCP_ISR1_SCP_PLGIN_MASK                 BIT(5)
#define SM5450_SCP_ISR1_SCP_PLGIN_SHIFT                5
#define SM5450_SCP_ISR1_ERR_ACK_L_MASK                 BIT(4)
#define SM5450_SCP_ISR1_ERR_ACK_L_SHIFT                4
#define SM5450_SCP_ISR1_ACK_CRCRX_MASK                 BIT(3)
#define SM5450_SCP_ISR1_ACK_CRCRX_SHIFT                3
#define SM5450_SCP_ISR1_ACK_PARRX_MASK                 BIT(2)
#define SM5450_SCP_ISR1_ACK_PARRX_SHIFT                2
#define SM5450_SCP_ISR1_ENABLE_HAND_NO_RESPOND_MASK    BIT(1)
#define SM5450_SCP_ISR1_ENABLE_HAND_NO_RESPOND_SHIFT   1
#define SM5450_SCP_ISR1_TRANS_HAND_NO_RESPOND_MASK     BIT(0)
#define SM5450_SCP_ISR1_TRANS_HAND_NO_RESPOND_SHIFT    0

/* SCP_ISR2 reg=0x1D */
#define SM5450_SCP_ISR2_REG                            0x1D
#define SM5450_SCP_ISR2_MSTR_RST_CPL_FLAG_MASK         BIT(7)
#define SM5450_SCP_ISR2_MSTR_RST_CPL_FLAG_SHIFT        7
#define SM5450_SCP_ISR2_CMD_CPL_MASK                   BIT(6)
#define SM5450_SCP_ISR2_CMD_CPL_SHIFT                  6
#define SM5450_SCP_ISR2_ACK_MASK                       BIT(3)
#define SM5450_SCP_ISR2_ACK_SHIFT                      3
#define SM5450_SCP_ISR2_NACK_MASK                      BIT(2)
#define SM5450_SCP_ISR2_NACK_SHIFT                     2
#define SM5450_SCP_ISR2_SLV_R_CPL_MASK                 BIT(1)
#define SM5450_SCP_ISR2_SLV_R_CPL_SHIFT                1
#define SM5450_SCP_ISR2_STMR_MASK                      BIT(0)
#define SM5450_SCP_ISR2_STMR_SHIFT                     0

/* SCP_MASK1 reg=0x1E */
#define SM5450_SCP_MASK1_REG                           0x1E
#define SM5450_SCP_MASK1_SCP_DM_DET_FLAG_MASK          BIT(7)
#define SM5450_SCP_MASK1_SCP_DM_DET_FLAG_EN_SCP_SHIFT  7
#define SM5450_SCP_MASK1_SCP_DET_FAIL_FLAG_MASK        BIT(6)
#define SM5450_SCP_MASK1_SCP_DET_FAIL_FLAG_SHIFT       6
#define SM5450_SCP_MASK1_SCP_PLGIN_MASK                BIT(5)
#define SM5450_SCP_MASK1_SCP_PLGIN_SHIFT               5
#define SM5450_SCP_MASK1_ERR_ACK_L_MASK                BIT(4)
#define SM5450_SCP_MASK1_ERR_ACK_L_SHIFT               4
#define SM5450_SCP_MASK1_ACK_CRCRX_MASK                BIT(3)
#define SM5450_SCP_MASK1_ACK_CRCRX_SHIFT               3
#define SM5450_SCP_MASK1_ACK_PARRX_MASK                BIT(2)
#define SM5450_SCP_MASK1_ACK_PARRX_SHIFT               2
#define SM5450_SCP_MASK1_ENABLE_HAND_NO_RESPOND_MASK   BIT(1)
#define SM5450_SCP_MASK1_ENABLE_HAND_NO_RESPOND_SHIFT  1
#define SM5450_SCP_MASK1_TRANS_HAND_NO_RESPOND_MASK    BIT(0)
#define SM5450_SCP_MASK1_TRANS_HAND_NO_RESPOND_SHIFT   0

/* SCP_MASK2 reg=0x1F */
#define SM5450_SCP_MASK2_REG                           0x1F
#define SM5450_SCP_MASK2_MSTR_RST_CPL_FLAG_MASK        BIT(7)
#define SM5450_SCP_MASK2_MSTR_RST_CPL_FLAG_SHIFT       7
#define SM5450_SCP_MASK2_CMD_CPL_MASK                  BIT(6)
#define SM5450_SCP_MASK2_CMD_CPL_SHIFT                 6
#define SM5450_SCP_MASK2_ACK_MASK                      BIT(3)
#define SM5450_SCP_MASK2_ACK_SHIFT                     3
#define SM5450_SCP_MASK2_NACK_MASK                     BIT(2)
#define SM5450_SCP_MASK2_NACK_SHIFT                    2
#define SM5450_SCP_MASK2_SLV_R_CPL_MASK                BIT(1)
#define SM5450_SCP_MASK2_SLV_R_CPL_SHIFT               1
#define SM5450_SCP_MASK2_STMR_MASK                     BIT(0)
#define SM5450_SCP_MASK2_STMR_SHIFT                    0

/* SCP_STATUS reg=0x20 */
#define SM5450_SCP_STATUS_REG                          0x20
#define SM5450_SCP_STATUS_ENABLE_HAND_SUCCESS_MASK     BIT(1)
#define SM5450_SCP_STATUS_ENABLE_HAND_SUCCESS_SHIFT    1
#define SM5450_SCP_STATUS_SCP_RDATA_READY_MASK         BIT(0)
#define SM5450_SCP_STATUS_SCP_RDATA_READY_SHIFT        0

/* SCP_STIMER reg=0x21 */
#define SM5450_SCP_STIMER_REG                          0x21
#define SM5450_SCP_STIMER_STIMER_MASK                  (BIT(2) | BIT(1) | BIT(0))
#define SM5450_SCP_STIMER_STIMER_SHIFT                 0
#define SM5450_SCP_STIMER_WDT_RESET                    0x02

/* RT_BUFFER_0 reg=0x22 */
#define SM5450_RT_BUFFER_0_REG                         0x22
#define SM5450_RT_BUFFER_0_MASK                        0xFF
#define SM5450_RT_BUFFER_0_SHIFT                       0
#define SM5450_RT_BUFFER_0_WDT_RESET                   0

/* RT_BUFFER_1 reg=0x23 */
#define SM5450_RT_BUFFER_1_REG                         0x23
#define SM5450_RT_BUFFER_1_MASK                        0xFF
#define SM5450_RT_BUFFER_1_SHIFT                       0
#define SM5450_RT_BUFFER_1_WDT_RESET                   0

/* RT_BUFFER_2 reg=0x24 */
#define SM5450_RT_BUFFER_2_REG                         0x24
#define SM5450_RT_BUFFER_2_MASK                        0xFF
#define SM5450_RT_BUFFER_2_SHIFT                       0
#define SM5450_RT_BUFFER_2_WDT_RESET                   0

/* RT_BUFFER_3 reg=0x25 */
#define SM5450_RT_BUFFER_3_REG                         0x25
#define SM5450_RT_BUFFER_3_MASK                        0xFF
#define SM5450_RT_BUFFER_3_SHIFT                       0
#define SM5450_RT_BUFFER_3_WDT_RESET                   0

/* RT_BUFFER_4 reg=0x26 */
#define SM5450_RT_BUFFER_4_REG                         0x26
#define SM5450_RT_BUFFER_4_MASK                        0xFF
#define SM5450_RT_BUFFER_4_SHIFT                       0
#define SM5450_RT_BUFFER_4_WDT_RESET                   0

/* RT_BUFFER_5 reg=0x27 */
#define SM5450_RT_BUFFER_5_REG                         0x27
#define SM5450_RT_BUFFER_5_MASK                        0xFF
#define SM5450_RT_BUFFER_5_SHIFT                       0
#define SM5450_RT_BUFFER_5_WDT_RESET                   0

/* RT_BUFFER_6 reg=0x28 */
#define SM5450_RT_BUFFER_6_REG                         0x28
#define SM5450_RT_BUFFER_6_MASK                        0xFF
#define SM5450_RT_BUFFER_6_SHIFT                       0
#define SM5450_RT_BUFFER_6_WDT_RESET                   0

/* RT_BUFFER_7 reg=0x29 */
#define SM5450_RT_BUFFER_7_REG                         0x29
#define SM5450_RT_BUFFER_7_MASK                        0xFF
#define SM5450_RT_BUFFER_7_SHIFT                       0
#define SM5450_RT_BUFFER_7_WDT_RESET                   0

/* RT_BUFFER_8 reg=0x2A */
#define SM5450_RT_BUFFER_8_REG                         0x2A
#define SM5450_RT_BUFFER_8_MASK                        0xFF
#define SM5450_RT_BUFFER_8_SHIFT                       0
#define SM5450_RT_BUFFER_8_WDT_RESET                   0

/* RT_BUFFER_9 reg=0x2B */
#define SM5450_RT_BUFFER_9_REG                         0x2B
#define SM5450_RT_BUFFER_9_MASK                        0xFF
#define SM5450_RT_BUFFER_9_SHIFT                       0
#define SM5450_RT_BUFFER_9_WDT_RESET                   0

/* RT_BUFFER_10 reg=0x2C */
#define SM5450_RT_BUFFER_10_REG                        0x2C
#define SM5450_RT_BUFFER_10_MASK                       0xFF
#define SM5450_RT_BUFFER_10_SHIFT                      0
#define SM5450_RT_BUFFER_10_WDT_RESET                  0

/* RT_BUFFER_11 reg=0x2D */
#define SM5450_RT_BUFFER_11_REG                        0x2D
#define SM5450_RT_BUFFER_11_MASK                       0xFF
#define SM5450_RT_BUFFER_11_SHIFT                      0

/* RT_BUFFER_12 reg=0x2E */
#define SM5450_RT_BUFFER_12_REG                        0x2E
#define SM5450_RT_BUFFER_12_MASK                       0xFF
#define SM5450_RT_BUFFER_12_SHIFT                      0

/* RT_BUFFER_13 reg=0x2F */
#define SM5450_RT_BUFFER_13_REG                        0x2F
#define SM5450_RT_BUFFER_13_MASK                       0xFF
#define SM5450_RT_BUFFER_13_SHIFT                      0

/* RT_BUFFER_14 reg=0x30 */
#define SM5450_RT_BUFFER_14_REG                        0x30
#define SM5450_RT_BUFFER_14_MASK                       0xFF
#define SM5450_RT_BUFFER_14_SHIFT                      0

/* RT_BUFFER_15 reg=0x31 */
#define SM5450_RT_BUFFER_15_REG                        0x31
#define SM5450_RT_BUFFER_15_MASK                       0xFF
#define SM5450_RT_BUFFER_15_SHIFT                      0

/* RT_BUFFER_16 reg=0x32 */
#define SM5450_RT_BUFFER_16_REG                        0x32
#define SM5450_RT_BUFFER_16_MASK                       0xFF
#define SM5450_RT_BUFFER_16_SHIFT                      0

/* RT_BUFFER_17 reg=0x33 */
#define SM5450_RT_BUFFER_17_REG                        0x33
#define SM5450_RT_BUFFER_17_MASK                       0xFF
#define SM5450_RT_BUFFER_17_SHIFT                      0

/* RT_BUFFER_18 reg=0x34 */
#define SM5450_RT_BUFFER_18_REG                        0x34
#define SM5450_RT_BUFFER_18_MASK                       0xFF
#define SM5450_RT_BUFFER_18_SHIFT                      0

/* RT_BUFFER_19 reg=0x35 */
#define SM5450_RT_BUFFER_19_REG                        0x35
#define SM5450_RT_BUFFER_19_MASK                       0xFF
#define SM5450_RT_BUFFER_19_SHIFT                      0

/* SCP_STATUS reg=0x96 */
#define SM5450_SCP_PING_REG                            0x96
#define SM5450_SCP_PING_WAIT_TIME_MASK                 BIT(4)
#define SM5450_SCP_PING_WAIT_TIME_SHIFT                4

int sm5450_scp_protocol_register(struct sm5450_device_info *di);
int sm5450_fcp_protocol_register(struct sm5450_device_info *di);

#endif /* _SM5450_PROTOCOL_H_ */
