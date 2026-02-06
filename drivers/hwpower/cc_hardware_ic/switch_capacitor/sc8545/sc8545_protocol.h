/*
 * sc8545_protocol.h
 *
 * sc8545 protocol header file
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

#ifndef _SC8545_PROTOCOL_H_
#define _SC8545_PROTOCOL_H_

#include "sc8545.h"

/* scp */
#define SC8545_SCP_ACK_RETRY_TIME              10
#define SC8545_SCP_RESTART_TIME                4
#define SC8545_SCP_RETRY_TIME                  3
#define SC8545_SCP_POLL_TIME                   100 /* ms */
#define SC8545_SCP_DETECT_MAX_CNT              20
#define SC8545_SCP_PING_DETECT_MAX_CNT         15
#define SC8545_SCP_NO_ERR                      0
#define SC8545_SCP_IS_ERR                      1
#define SC8545_SCP_WRITE_OP                    0
#define SC8545_SCP_READ_OP                     1
#define SC8545_SCP_MULTI_READ_LEN              2
#define SC8545_SCP_ACK_AND_CRC_LEN             2
#define SC8545_SCP_SBRWR_NUM                   1
#define SC8545_SCP_CMD_SBRRD                   0x0c
#define SC8545_SCP_CMD_SBRWR                   0x0b
#define SC8545_SCP_CMD_MBRRD                   0x1c
#define SC8545_SCP_CMD_MBRWR                   0x1b
#define SC8545_SCP_ACK                         0x08
#define SC8545_SCP_NACK                        0x03

/* fcp adapter vol value */
#define SC8545_FCP_ADAPTER_MAX_VOL             12000
#define SC8545_FCP_ADAPTER_MIN_VOL             5000
#define SC8545_FCP_ADAPTER_RST_VOL             5000
#define SC8545_FCP_ADAPTER_ERR_VOL             500
#define SC8545_FCP_ADAPTER_VOL_CHECK_TIME      10

/* DPDM_CTRL1 reg=0x21 */
#define SC8545_DPDM_CTRL1_REG                  0x21
#define SC8545_DM_500K_PD_EN_MASK              BIT(7)
#define SC8545_DM_500K_PD_EN_SHIFT             7
#define SC8545_DP_500K_PD_EN_MASK              BIT(6)
#define SC8545_DP_500K_PD_EN_SHIFT             6
#define SC8545_DM_20K_PD_EN_MASK               BIT(5)
#define SC8545_DM_20K_PD_EN_SHIFT              5
#define SC8545_DP_20K_PD_EN_MASK               BIT(4)
#define SC8545_DP_20K_PD_EN_SHIFT              4
#define SC8545_DM_SINK_EN_MASK                 BIT(3)
#define SC8545_DM_SINK_EN_SHIFT                3
#define SC8545_DP_SINK_EN_MASK                 BIT(2)
#define SC8545_DP_SINK_EN_SHIFT                2
#define SC8545_DP_SRC_10UA_MASK                BIT(1)
#define SC8545_DP_SRC_10UA_SHIFT               1
#define SC8545_DPDM_EN_MASK                    BIT(0)
#define SC8545_DPDM_EN_SHIFT                   0

/* DPDM_CTRL2 reg=0x22 */
#define SC8545_DPDM_CTRL2_REG                  0x22
#define SC8545_DPDM_OVP_DIS_MASK               BIT(6)
#define SC8545_DPDM_OVP_DIS_SHIFT              6
#define SC8545_DM_BUF_MASK                     (BIT(5) | BIT(4))
#define SC8545_DM_BUF_SHIFT                    4
#define SC8545_DP_BUF_MASK                     (BIT(3) | BIT(2))
#define SC8545_DP_BUF_SHIFT                    2
#define SC8545_DM_SINK_EN_MASK                 BIT(3)
#define SC8545_DM_SINK_EN_SHIFT                3
#define SC8545_DP_SINK_EN_MASK                 BIT(2)
#define SC8545_DP_SINK_EN_SHIFT                2
#define SC8545_DM_BUFF_EN_MASK                 BIT(1)
#define SC8545_DM_BUFF_EN_SHIFT                1
#define SC8545_DP_BUFF_EN_MASK                 BIT(0)
#define SC8545_DP_BUFF_EN_SHIFT                0

/* DPDM_STAT reg=0x23 */
#define SC8545_DPDM_STAT_REG                   0x23
#define SC8545_VDM_RD_MASK                     (BIT(5) | BIT(4) | BIT(3))
#define SC8545_VDM_RD_SHIFT                    3
#define SC8545_VDP_RD_MASK                     (BIT(2) | BIT(1) | BIT(0))
#define SC8545_VDP_RD_SHIFT                    0

/* DPDM_FLAG_MASK reg=0x24 */
#define SC8545_DPDM_FLAG_MASK_REG              0x24
#define SC8545_DM_LOW_MSK_MASK                 BIT(6)
#define SC8545_DM_LOW_MSK_SHIFT                6
#define SC8545_DM_LOW_FLAG_MASK                BIT(5)
#define SC8545_DM_LOW_FLAG_SHIFT               5
#define SC8545_DP_LOW_MSK_MASK                 BIT(4)
#define SC8545_DP_LOW_MSK_SHIFT                4
#define SC8545_DP_LOW_FLAG_MASK                BIT(3)
#define SC8545_DP_LOW_FLAG_SHIFT               3
#define SC8545_DPDM_OVP_MSK_MASK               BIT(2)
#define SC8545_DPDM_OVP_MASK_SHIFT             2
#define SC8545_DPDM_OVP_FLAG_MASK              BIT(1)
#define SC8545_DPDM_OVP_FLAG_SHIFT             1
#define SC8545_DPDM_OVP_STAT_MASK              BIT(0)
#define SC8545_DPDM_OVP_STAT_SHIFT             0

/* SCP_CTRL reg=0x25 */
#define SC8545_SCP_CTRL_REG                    0x25
#define SC8545_SCP_EN_MASK                     BIT(7)
#define SC8545_SCP_EN_SHIFT                    7
#define SC8545_SCP_SOFT_RST_MASK               BIT(6)
#define SC8545_SCP_SOFT_RST_SHIFT              6
#define SC8545_TX_CRC_DIS_MASK                 BIT(5)
#define SC8545_TX_CRC_DIS_SHIFT                5
#define SC8545_DM_3P3_EN_MASK                  BIT(4)
#define SC8545_DM_3P3_EN_SHIFT                 4
#define SC8545_CLR_TX_FIFO_MASK                BIT(3)
#define SC8545_CLR_TX_FIFO_SHIFT               3
#define SC8545_CLR_RX_FIFO_MASK                BIT(2)
#define SC8545_CLR_RX_FIFO_SHIFT               2
#define SC8545_SND_RST_TRANS_MASK              BIT(1)
#define SC8545_SND_RST_TRANS_SHIFT             1
#define SC8545_SND_START_TRANS_MASK            BIT(0)
#define SC8545_SND_START_TRANS_SHIFT           0
#define SC8545_SCP_ENABLE                      1
#define SC8545_SCP_DISABLE                     0
#define SC8545_SCP_TRANS_RESET                 1

/* SCP_WDATA reg=0x26 */
#define SC8545_SCP_TX_DATA_REG                 0x26
#define SC8545_SCP_TX_DATA_RST                 0

/* SCP_RDATA reg=0x27 */
#define SC8545_SCP_RX_DATA_REG                 0x27
#define SC8545_SCP_RX_DATA_RST                 0

/* SCP_FIFO_STAT reg=0x28 */
#define SC8545_SCP_FIFO_STAT_REG               0x28
#define SC8545_DM_BUSY_STAT_MASK               BIT(7)
#define SC8545_DM_BUSY_STAT_SHIFT              7
#define SC8545_TX_FIFO_CNT_STAT_MASK           (BIT(5) | BIT(4) | BIT(3))
#define SC8545_TX_FIFO_CNT_STAT_SHIFT          3
#define SC8545_RX_FIFO_CNT_STAT_MASK           (BIT(2) | BIT(1) | BIT(0))
#define SC8545_RX_FIFO_CNT_STAT_SHIFT          0
#define SC8545_SCP_FIFO_DEEPTH                 5

/* SCP_STAT reg=0x29 */
#define SC8545_SCP_STAT_REG                    0x29
#define SC8545_FIRST_PING_DET_STAT             0x78
#define SC8545_NO_FIRST_SLAVE_PING_STAT_MASK   BIT(7)
#define SC8545_NO_FIRST_SLAVE_PING_STAT_SHIFT  7
#define SC8545_NO_MED_SLAVE_PING_STAT_MASK     BIT(6)
#define SC8545_NO_MED_SLAVE_PING_STAT_SHIFT    6
#define SC8545_NO_LAST_SLAVE_PING_STAT_MASK    BIT(5)
#define SC8545_NO_LAST_SLAVE_PING_STAT_SHIFT   5
#define SC8545_NO_RX_PKT_STAS_MASK             BIT(4)
#define SC8545_NO_RX_PKT_STAS_SHIFT            4
#define SC8545_NO_TX_PKT_STAT_MASK             BIT(3)
#define SC8545_NO_TX_PKT_STAT_SHIFT            3
#define SC8545_WDT_EXPIRED_STAT_MASK           BIT(2)
#define SC8545_WDT_EXPIRED_STAT_SHIFT          2
#define SC8545_RX_CRC_ERR_STAT_MASK            BIT(1)
#define SC8545_RX_CRC_ERR_STAT_SHIFT           1
#define SC8545_RX_PAR_ERR_STAT_MASK            BIT(0)
#define SC8545_RX_PAR_ERR_STAT_SHIFT           0

/* SCP_FLAG_MASK reg=0x2A */
#define SC8545_SCP_FLAG_MASK_REG               0x2A
#define SC8545_SCP_FLAG_MASK_REG_INIT          0x20
#define SC8545_TX_FIFO_EMPTY_MSK_MASK          BIT(5)
#define SC8545_TX_FIFO_EMPTY_MSK_SHIFT         5
#define SC8545_TX_FIFO_EMPTY_FLAG_MASK         BIT(4)
#define SC8545_TX_FIFO_EMPTY_FLAG_SHIFT        4
#define SC8545_RX_FIFO_FULL_MSK_MASK           BIT(3)
#define SC8545_RX_FIFO_FULL_MSK_SHIFT          3
#define SC8545_RX_FIFO_FULL_FLAG_MASK          BIT(2)
#define SC8545_RX_FIFO_FULL_FLAG_SHIFT         2
#define SC8545_TRANS_DONE_MSK_MASK             BIT(1)
#define SC8545_TRANS_DONE_MSK_SHIFT            1
#define SC8545_TRANS_DONE_FLAG_MASK            BIT(0)
#define SC8545_TRANS_DONE_FLAG_SHIFT           0

int sc8545_protocol_ops_register(struct sc8545_device_info *di);

#endif /* _SC8545_PROTOCOL_H_ */
