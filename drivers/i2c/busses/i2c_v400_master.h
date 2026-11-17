/*
 * I2C adapter driver.
 *
 * Based on the TI DAVINCI I2C adapter driver.
 *
 * Copyright (c) 2006 Texas Instruments.
 * Copyright (c) 2007 MontaVista Software Inc.
 * Copyright (c) 2009 Provigent Ltd.
 *
 * ----------------------------------------------------------------------------
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * ----------------------------------------------------------------------------
 *
 */

#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/scatterlist.h>
#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/pm_qos.h>

#define HS_IC_DEFAULT_FUNCTIONALITY (I2C_FUNC_I2C | I2C_FUNC_SMBUS_BYTE | I2C_FUNC_SMBUS_BYTE_DATA |       \
	I2C_FUNC_SMBUS_WORD_DATA | I2C_FUNC_SMBUS_BLOCK_DATA | I2C_FUNC_SMBUS_I2C_BLOCK)

#define HS_I2C_SPEED_MASK              0x3
#define HS_I2C_SPEED_STD               0x0
#define HS_I2C_SPEED_FAST              0x1
#define HS_I2C_SPEED_HIGH              0x2
#define HS_I2C_RESTART_EN              0x8

#define I2C_SS_SPEED                   100
#define I2C_FS_SPEED                   400
#define I2C_HS_SPEED                   1000
#define I2C_SS_TLOW_MIN_NS             4700
#define I2C_FS_TLOW_MIN_NS             1300
#define I2C_HS_TLOW_MIN_NS             500
#define I2C_SS_THIGH_MIN_NS            4000
#define I2C_FS_THIGH_MIN_NS            600
#define I2C_HS_THIGH_MIN_NS            260
#define I2C_SS_SPEED_HPERIOD           10000
#define I2C_FS_SPEED_HPERIOD           2500
#define I2C_HS_SPEED_HPERIOD           1000

#define REG_SIZE                       4
#define REG_CNT                        22
#define TIMEOUT                        3
#define STATUS_IDLE                    0x0
#define STATUS_WRITE_IN_PROGRESS       0x1
#define STATUS_READ_IN_PROGRESS        0x2
#define I2C_RESET_REG_CNT              4
#define I2C_KHZ                        1000
#define I2C_TIMEOUT_DEFAULT_VAL        100
#define I2C_WAIT_STATUS_TIMES          10
#define I2C_PROPER_RETRIES             3
#define I2C_RECOVER_SLEEP_MS           100
#define GET_DEV_LOCK_TIMEOUT           500
#define SPEED_INTERVAL                 1000000
#define WAIT_FOR_COMPLETION            ((HZ * totallen) / 400 + HZ)

#define I2C_FIFO_TX_DEPTH              64
#define I2C_FIFO_RX_DEPTH              64

#define I2C_DELAY_OFF_NS               70
#define I2C_SCL_HIGHT_NS               100
#define I2C_SCL_LOW_NS                 200
#define I2C_SDA_HOLD_NS                300
#define I2C_SDA_FALLING_NS             300
#define I2C_SCL_FALLING_NS             300

#define ERROR_TXDATA_NOACK             0
#define ERROR_10B_RD_NORSTRT           1
#define ERROR_10ADDR2_NOACK            2
#define ERROR_10ADDR1_NOACK            3
#define ERROR_7B_ADDR_NOACK            4
#define ERROR_HS_ACKDET                5
#define ERROR_HS_NORSTRT               6
#define ERROR_SBYTE_NORSTRT            7
#define ERROR_SBYTE_ACKDET             8
#define ERROR_GCALL_READ               9
#define ERROR_GCALL_NOACK              10

#define HISI_SEC_SLAVE_MAX_NUM         2
#define HISI_DEFAULT_SEC_SLAVE         0x28
#define HISI_DEFAULT_SEC_SLAVE_NUM     1
#define I2C_DMA_MAX_BURST              16
#define I2C_TRANS_ERROR_GCALL_READ     BIT(ERROR_GCALL_READ)
#define I2C_TRANS_ERROR_7B_ADDR_NOACK  BIT(ERROR_7B_ADDR_NOACK)
#define I2C_TRANS_ERROR_10ADDR1_NOACK  BIT(ERROR_10ADDR1_NOACK)
#define I2C_TRANS_ERROR_10ADDR2_NOACK  BIT(ERROR_10ADDR2_NOACK)
#define I2C_TRANS_ERROR_TXDATA_NOACK   BIT(ERROR_TXDATA_NOACK)
#define I2C_TRANS_ERROR_GCALL_NOACK    BIT(ERROR_GCALL_NOACK)

#define I2C_TRANS_ERROR_NOACK (I2C_TRANS_ERROR_7B_ADDR_NOACK | I2C_TRANS_ERROR_10ADDR1_NOACK | \
	I2C_TRANS_ERROR_10ADDR2_NOACK | I2C_TRANS_ERROR_TXDATA_NOACK | I2C_TRANS_ERROR_GCALL_NOACK)

#define HISI_SLAVE_WRITE_REG           0xC5010002
#define HISI_I2C_READ_REG              0xc5010000
#define HISI_I2C_WRITE_REG             0xc5010001
#define HISI_I2C_XFER_LOCK             0xc5010002
#define HISI_I2C_XFER_UNLOCK           0xc5010003

/*
 * Registers offset
 */
#define FRAME_CTRL                     0x0
#define SLV_ADDR                       0x4
#define CMD_TXDATA                     0x8
#define RXDATA                         0xC
#define SS_SCL_HCNT                    0x10
#define SS_SCL_LCNT                    0x14
#define FS_SCL_HCNT                    0x18
#define FS_SCL_LCNT                    0x1C
#define HS_SCL_HCNT                    0x20
#define HS_SCL_LCNT                    0x24
#define FIFO_CTRL                      0x28
#define FIFO_STATE                     0x2C
#define SDA_HOLD                       0x30
#define DMA_CTRL                       0x34
#define FS_SPK_LEN                     0x38
#define HS_SPK_LEN                     0x3C
#define INT_MSTAT                      0x44
#define INT_CLR                        0x48
#define INT_MASK                       0x4C
#define TRANS_STATE                    0x50
#define TRANS_ERR                      0x54

/*
 * Registers config
 */
#define FIFO_CLEAR                     0x3
#define FIFO_TX_THD                    0xF00
#define FIFO_RX_THD                    0x3C
#define I2C_DEFULT_HOLD_TIME           0x1
#define TRANS_STATE_MASK               0xF
#define PIN_STATE_MASK                 0x60
#define I2C_PIN_STATUS                 0x60
#define HISI_I2C_CON_RECOVERY_CFG      0x0
#define HISI_I2C_TAR_RECOVERY_CFG      0x7F
#define HISI_I2C_RD_CFG                BIT(8)
#define HISI_I2C_RX_EMPTY              BIT(3)
#define HISI_I2C_TX_FULL               BIT(11)
#define I2C_RESTART_EN                 BIT(3)
#define I2C_STATUS_ACTIVITY            BIT(0)
#define I2C_CON_10BITADDR_MASTER       BIT(2)
#define I2C_FIFO_TX_EMPTY              BIT(9)
#define I2C_READ_CMD                   BIT(8)
#define I2C_IRQ_STOP                   BIT(9)
#define I2C_MAX_DMA_BUF_LEN            (60 * 1024)
#define I2C_DMA_ENABLE                 BIT(0)
#define I2C_DMA_DISABLE                0
#define I2C_INT_RX_FULL                BIT(3)
#define I2C_INT_TX_EMPTY               BIT(4)
#define I2C_INT_CLEAR                  0x1F
#define I2C_INT_TRCMP                  BIT(0)
#define I2C_INT_TRERR                  BIT(1)
#define I2C_INT_FIFOERR                BIT(2)
#define I2C_INT_RX_AFULL               BIT(3)
#define I2C_INT_TX_AEMPTY              BIT(4)
#define I2C_INTR_DEFAULT_MASK          (I2C_INT_RX_AFULL | I2C_INT_TRERR | I2C_INT_TRCMP)
#define I2C_INTR_MASTER_MASK           (I2C_INTR_DEFAULT_MASK | I2C_INT_TX_AEMPTY)

/*
 * struct hs_i2c_dev - private i2c_v400 data
 * @dev: driver model device node
 * @base: IO registers pointer
 * @cmd_complete: tx completion indicator
 * @clk: input reference clock
 * @slave: represent an I2C slave device
 * @cmd_err: run time hadware error code
 * @msgs: points to an array of messages currently being transferred
 * @msgs_num: the number of elements in msgs
 * @msg_write_idx: the element index of the current tx message in the msgs
 *	array
 * @tx_buf_len: the length of the current tx buffer
 * @tx_buf: the current tx buffer
 * @msg_read_idx: the element index of the current rx message in the msgs
 *	array
 * @rx_buf_len: the length of the current rx buffer
 * @rx_buf: the current rx buffer
 * @msg_err: error status of the current transfer
 * @status: i2c master status, one of STATUS_*
 * @abort_source: copy of the TX_ABRT_SOURCE register
 * @irq: interrupt number for the i2c master
 * @adapter: i2c subsystem adapter node
 * @slave_cfg: configuration for the slave device
 * @tx_fifo_depth: depth of the hardware tx fifo
 * @rx_fifo_depth: depth of the hardware rx fifo
 * @rx_outstanding: current master-rx elements in tx fifo
 * @clk_freq: bus clock frequency
 * @ss_hcnt: standard speed HCNT value
 * @ss_lcnt: standard speed LCNT value
 * @fs_hcnt: fast speed HCNT value
 * @fs_lcnt: fast speed LCNT value
 * @fp_hcnt: fast plus HCNT value
 * @fp_lcnt: fast plus LCNT value
 * @hs_hcnt: high speed HCNT value
 * @hs_lcnt: high speed LCNT value
 * @pm_qos: pm_qos_request used while holding a hardware lock on the bus
 * @acquire_lock: function to acquire a hardware lock on the bus
 * @release_lock: function to release a hardware lock on the bus
 * @pm_disabled: true if power-management should be disabled for this i2c-bus
 * @disable: function to disable the controller
 * @disable_int: function to disable all interrupts
 * @init: function to initialize the I2C hardware
 * @mode: operation mode - DW_IC_MASTER or DW_IC_SLAVE
 *
 * HCNT and LCNT parameters can be used if the platform knows more accurate
 * values than the one computed based only on the input clock frequency.
 * Leave them to be %0 if not used.
 */

struct hs_i2c_priv_data {
	u32 delay_off;
	u32 delay_bit;
	u32 reset_enable_off;
	u32 reset_disable_off;
	u32 reset_status_off;
	u32 reset_bit;
};

struct hs_i2c_dma_data {
	struct dma_chan *chan;
	struct scatterlist sg;
	u8 *buf;
};

struct hs_i2c_dev {
	struct device *dev;
	void __iomem *base;
	struct completion cmd_complete;
	struct clk *clk;
	u32 (*get_clk_rate_khz) (struct hs_i2c_dev *dev);
	int cmd_err;
	struct i2c_msg *msgs;
	int msgs_num;
	int msg_write_idx;
	u32 tx_buf_len;
	u8 *tx_buf;
	int msg_read_idx;
	u32 rx_buf_len;
	u8 *rx_buf;
	int msg_err;
	unsigned int status;
	u32 error_source;
	int irq;
	struct i2c_adapter adapter;
	u32 functionality;
	u32 master_cfg;
	unsigned int tx_fifo_depth;
	unsigned int rx_fifo_depth;
	int rx_outstanding;
	u32 clk_freq;
	u32 sda_hold_tx;
	u32 sda_hold_rx;
	u32 fs_spk_len;
	u32 hs_spk_len;
	spinlock_t spinlock;
	struct mutex lock;
	struct pinctrl *pinctrl;
	int pinctrl_flag;
	struct hs_i2c_priv_data priv;
	bool secure_mode;
	u32 reg_base;
	/* DMA stuff */
	bool using_tx_dma;
	bool using_rx_dma;
	struct hs_i2c_dma_data dmarx;
	struct hs_i2c_dma_data dmatx;
	int timeout_count;
	struct completion dma_complete;
	bool using_dma;
	resource_size_t mapbase;
	u32 delay_off;
	void __iomem *reset_reg_base;
	void (*reset_controller) (struct hs_i2c_dev *dev);
	void (*recover_bus)(struct i2c_adapter *);
};
