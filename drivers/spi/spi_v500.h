/*
 * spi_v500.h. A driver for the SPI bus master.
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __SPI_V500_H_
#define __SPI_V500_H_

/* error number */
#define E_HWSPI_LOCK        1
#define E_SPI_NO_CLK        2
#define E_SPI_NO_CLK_EN     3
#define E_SPI_CLK_SET_RATE  4
#define E_SPI_PARAMS        5

/* register bit */
#define BIT_CSEN            0U
#define BIT_CPOL            2U
#define BIT_CPHA            3U
#define BIT_DRPR            4U
#define BIT_DRPO            12U
#define BIT_FRAME           20U
#define BIT_HSPD            25U
#define BIT_TXDMAE          26U
#define BIT_RXDMAE          27U

#define GET_HARDWARE_TIMEOUT  10000
#define SPI_4G_PHYS_ADDR      0xFFFFFFFF
#define ENUM_SPI_HWSPIN_LOCK  27

/*
 * This macro is used to define some register default values.
 * reg is masked with mask, the OR:ed with an (again masked)
 * val shifted sb steps to the left.
 */
#define SSP_WRITE_BITS(reg, val, mask, sb) \
	((reg) = ((reg) & ~(mask)) | (((val) << (sb)) & (mask)))

/*
 * This macro is also used to define some default values.
 * It will just shift val by sb steps to the left and mask
 * the result with mask.
 */
#define GEN_MASK_BITS(val, mask, sb) \
	(((val) << (sb)) & (mask))

#define DO_NOT_QUEUE_DMA         0
#define QUEUE_DMA                1
#define RX_TRANSFER              1
#define TX_TRANSFER              2
#define FIFO_DEPTH               256

/*
 * Macros to access SSP Registers with their offsets
 */
#define SSP_CSCR(r) ((r) + 0x000) /* cs register */
#define SSP_COMMONCR(r) ((r) + 0x004) /* spi common register */
#define SSP_ENR(r) ((r) + 0x008) /* spi enable register */
#define SSP_FIFOCR(r) ((r) + 0x00C) /* fifo level control register */
#define SSP_IMC(r) ((r) + 0x010) /* interrupt mask register */
#define SSP_DIN(r) ((r) + 0x014) /* data in register */
#define SSP_DOUT(r) ((r) + 0x018) /* data out register */
#define SSP_SR(r) ((r) + 0x01C)  /* status register */
#define SSP_RIS(r) ((r) + 0x020) /* raw interrupt register */
#define SSP_MIS(r) ((r) + 0x024) /* after mask interrupt status register */
#define SSP_ICR(r) ((r) + 0x028) /* interrupt clear register */

/*
 * SPI CS Control register -- SSP_CSCR
 */
#define SSP_CSCR_MASK_CSEN (0x1U << 0)

/*
 * SPI Common Control register--SSP_COMMONCR
 */
#define SSP_COMMONCR_LOOPBACK (0x1U << 1)
#define SSP_COMMONCR_MASK_CPOL (0x1U << 2)
#define SSP_COMMONCR_MASK_CPHA (0x1U << 3)
#define SSP_COMMONCR_MASK_DRPR (0xFFU << 4)
#define SSP_COMMONCR_MASK_DRPO (0xFFU << 12)  /* clk divider Prescale */
#define SSP_COMMONCR_MASK_FRAMESIZE (0x0FU << 20) /* Effective data bit width */
#define SSP_COMMONCR_MASK_HSPD (0x1U << 25)   /* Transmit mode high/nomal */
#define SSP_COMMONCR_MASK_TXDMAE (0x1U << 26) /* Transmit DMA Enable bit */
#define SSP_COMMONCR_MASK_RXDMAE (0x1U << 27) /* Receive DMA Enable bit */
#define SSP_COMMONCR_MASK_TIMEOUTCNT (0x3U << 28) /* Receive FIFO timeout count */

/*
 * SPI Enable Register -- SSP_ENR
 */
#define SSP_ENR_MASK_EN (0x1U << 0) /* spi enable */

/*
 * SSP Status Register - SSP_SR
 */
#define SSP_SR_MASK_TFE   (0x1U << 0) /* Transmit FIFO empty */
#define SSP_SR_MASK_TNF   (0x1U << 1) /* Transmit FIFO not fUl */
#define SSP_SR_MASK_RNE   (0x1U << 2) /* Receive FIFO not empty */
#define SSP_SR_MASK_RFF   (0x1U << 3) /* Receive FIFO fUl */
#define SSP_SR_MASK_BSY   (0x1U << 4) /* Busy Flag */

/*
 * SSP TX/RX FIFO Register - SSP_FIFOCR
 */
#define SSP_FIFOCR_MASK_TXDMA (0x07U << 0)
#define SSP_FIFOCR_MASK_TXINT (0x3U << 3)
#define SSP_FIFOCR_MASK_RXDMA (0x7U << 6)
#define SSP_FIFOCR_MASK_RXINT (0x3U << 9)

/*
 * SSP Interrupt Mask Set/Clear Register - SSP_IMC
 */
#define SSP_IMC_MASK_RORIM (0x1U << 0) /* Receive Overrun Interrupt mask */
#define SSP_IMC_MASK_RTIM  (0x1U << 1) /* Receive timeout Interrupt mask */
#define SSP_IMC_MASK_RXIM  (0x1U << 2) /* Receive FIFO Interrupt mask */
#define SSP_IMC_MASK_TXIM  (0x1U << 3) /* Transmit FIFO Interrupt mask */

/*
 * SSP Raw Interrupt Status Register - SSP_RIS
 */
/* Receive Overrun Raw Interrupt status */
#define SSP_RIS_MASK_RORRIS		(0x1U << 0)
/* Receive Timeout Raw Interrupt status */
#define SSP_RIS_MASK_RTRIS		(0x1U << 1)
/* Receive FIFO Raw Interrupt status */
#define SSP_RIS_MASK_RXRIS		(0x1U << 2)
/* Transmit FIFO Raw Interrupt status */
#define SSP_RIS_MASK_TXRIS		(0x1U << 3)

/*
 * SSP Masked Interrupt Status Register - SSP_MIS
 */
/* Receive Overrun Masked Interrupt status */
#define SSP_MIS_MASK_RORMIS		(0x1U << 0)
/* Receive Timeout Masked Interrupt status */
#define SSP_MIS_MASK_RTMIS		(0x1U << 1)
/* Receive FIFO Masked Interrupt status */
#define SSP_MIS_MASK_RXMIS		(0x1U << 2)
/* Transmit FIFO Masked Interrupt status */
#define SSP_MIS_MASK_TXMIS		(0x1U << 3)

/*
 * SSP Interrupt Clear Register - SSP_ICR
 */
/* Receive Overrun Raw Clear Interrupt bit */
#define SSP_ICR_MASK_RORIC		(0x1U << 0)
/* Receive Timeout Clear Interrupt bit */
#define SSP_ICR_MASK_RTIC		(0x1U << 1)

/*
 * Message State
 * we use the spi_message.state (void *) pointer to
 * hold a single state value, that's why all this
 * (void *) casting is done here.
 */
#define STATE_START			((void *) 0)
#define STATE_RUNNING		((void *) 1)
#define STATE_DONE			((void *) 2)
#define STATE_ERROR			((void *) -1)

/*
 * DMA Burst Config
 */
#define MAX_BURST_1		1
#define MAX_BURST_4		4
#define MAX_BURST_8		8
#define MAX_BURST_16		16
#define MAX_BURST_32		32

/*
 * SSP State - Whether Enabled or Disabled
 */
#define SSP_DISABLED			0
#define SSP_ENABLED			1

/*
 * SSP DMA State - Whether DMA Enabled or Disabled
 */
#define SSP_DMA_DISABLED		0
#define SSP_DMA_ENABLED			1

/*
 * SSP Clock Defaults
 */
#define SSP_DEFAULT_CLKRATE 0x2
#define SSP_DEFAULT_PRESCALE 0x40

/*
 * SSP Clock Parameter ranges
 */
#define CPSDVR_MIN 0x02
#define CPSDVR_MAX 0xFE
#define SCR_MIN 0x00
#define SCR_MAX 0xFF

/*
 * SSP Interrupt related Macros
 */
#define DEFAULT_SSP_REG_IMSC  0xFU
#define DISABLE_ALL_INTERRUPTS DEFAULT_SSP_REG_IMSC
#define ENABLE_ALL_INTERRUPTS 0x0U

#define CLEAR_ALL_INTERRUPTS  0x3

#define SPI_POLLING_TIMEOUT 1000

#define HARDWARE_LOCK_DEFAULT 1

#define DEFAULT_SSP_REG_CSCR (GEN_MASK_BITS(SSP_CS_CTRL_DISENABLE, SSP_CSCR_MASK_CSEN, 0))
#define DEFAULT_SSP_REG_COMMONCR      \
	(GEN_MASK_BITS(LOOPBACK_DISABLED, SSP_COMMONCR_LOOPBACK, 1) | \
	GEN_MASK_BITS(SSP_CLK_POL_IDLE_LOW, SSP_COMMONCR_MASK_CPOL, 2) | \
	GEN_MASK_BITS(SSP_CLK_SECOND_EDGE, SSP_COMMONCR_MASK_CPHA, 3) | \
	GEN_MASK_BITS(SSP_DEFAULT_PRESCALE, SSP_COMMONCR_MASK_DRPR, 4) | \
	GEN_MASK_BITS(SSP_DEFAULT_CLKRATE, SSP_COMMONCR_MASK_DRPO, 12) | \
	GEN_MASK_BITS(SSP_DATA_BITS_12, SSP_COMMONCR_MASK_FRAMESIZE, 20) | \
	GEN_MASK_BITS(SSP_NOMAL, SSP_COMMONCR_MASK_HSPD, 25) | \
	GEN_MASK_BITS(SSP_DMA_DISABLED, SSP_COMMONCR_MASK_TXDMAE, 26) | \
	GEN_MASK_BITS(SSP_DMA_DISABLED, SSP_COMMONCR_MASK_RXDMAE, 27))
#define DEFAULT_SSP_REG_ENR                       \
	(GEN_MASK_BITS(SSP_DISABLED, SSP_ENR_MASK_EN, 0))


#define DEFAULT_SSP_REG_FIFOCR                    \
	(GEN_MASK_BITS(SSP_TX_64_OR_MORE_EMPTY_LOC, SSP_FIFOCR_MASK_TXDMA, 0) |   \
	GEN_MASK_BITS(SSP_TX_64_OR_MORE_EMPTY_LOC, SSP_FIFOCR_MASK_TXINT, 3) | \
	GEN_MASK_BITS(SSP_RX_16_OR_MORE_ELEM, SSP_FIFOCR_MASK_RXDMA, 6) |     \
	GEN_MASK_BITS(SSP_RX_16_OR_MORE_ELEM, SSP_FIFOCR_MASK_RXINT, 9))


/*
 * The type of reading going on on this chip
 */
enum ssp_reading {
	READING_NULL,
	READING_U8,
	READING_U16,
	READING_U32
};

/*
 * The type of writing going on on this chip
 */
enum ssp_writing {
	WRITING_NULL,
	WRITING_U8,
	WRITING_U16,
	WRITING_U32
};

enum ssp_transfer_state {
	STATE_CHIP_SELECT,
	STATE_CFG_DMA_DONE,
	STATE_DMA_CALLBACK,
	STATE_CHIP_DESELECT,
	STATE_FIFO_OVERRUN
};

/*
 * whether SSP is in loopback mode or not
 */
enum ssp_loopback {
	LOOPBACK_DISABLED,
	LOOPBACK_ENABLED
};

/*
 * whether spi is in high speed mode or nomal speed mode
 */
enum ssp_speed_mode { SSP_NOMAL, SSP_HIGH };

/*
 * whether spi cs ctrl is enable
 */
enum ssp_cs_ctrl {
	SSP_CS_CTRL_DISENABLE,
	SSP_CS_CTRL_ENABLE
};

/*
 * enum SPI Clock Polarity - clock polarity (Motorola SPI interface only)
 * @SSP_CLK_POL_IDLE_LOW: Low inactive level
 * @SSP_CLK_POL_IDLE_HIGH: High inactive level
 */
enum ssp_spi_clk_pol { SSP_CLK_POL_IDLE_LOW, SSP_CLK_POL_IDLE_HIGH };

/*
 * enum SPI Clock Phase - clock phase (Motorola SPI interface only)
 * @SSP_CLK_FIRST_EDGE: Receive data on first edge transition (actual direction depends on polarity)
 * @SSP_CLK_SECOND_EDGE: Receive data on second edge transition (actual direction depends on polarity)
 */
enum ssp_spi_clk_phase {
	SSP_CLK_FIRST_EDGE,
	SSP_CLK_SECOND_EDGE
};

/*
 * CHIP select/deselect commands
 */
enum ssp_chip_select { SSP_CHIP_SELECT, SSP_CHIP_DESELECT };

/*
 * enum ssp_mode - spi mode of operation (Communication modes)
 */
enum ssp_mode {
	INTERRUPT_TRANSFER,
	POLLING_TRANSFER,
	DMA_TRANSFER
};

enum ssp_data_size {
	SSP_DATA_BITS_4 = 0x03,
	SSP_DATA_BITS_5,
	SSP_DATA_BITS_6,
	SSP_DATA_BITS_7,
	SSP_DATA_BITS_8,
	SSP_DATA_BITS_9,
	SSP_DATA_BITS_10,
	SSP_DATA_BITS_11,
	SSP_DATA_BITS_12,
	SSP_DATA_BITS_13,
	SSP_DATA_BITS_14,
	SSP_DATA_BITS_15,
	SSP_DATA_BITS_16,
	SSP_DATA_BITS_17,
	SSP_DATA_BITS_18,
	SSP_DATA_BITS_19,
	SSP_DATA_BITS_20,
	SSP_DATA_BITS_21,
	SSP_DATA_BITS_22,
	SSP_DATA_BITS_23,
	SSP_DATA_BITS_24,
	SSP_DATA_BITS_25,
	SSP_DATA_BITS_26,
	SSP_DATA_BITS_27,
	SSP_DATA_BITS_28,
	SSP_DATA_BITS_29,
	SSP_DATA_BITS_30,
	SSP_DATA_BITS_31,
	SSP_DATA_BITS_32
};

enum ssp_rx_level_trig {
	SSP_RX_1_OR_MORE_ELEM,
	SSP_RX_4_OR_MORE_ELEM,
	SSP_RX_8_OR_MORE_ELEM,
	SSP_RX_16_OR_MORE_ELEM,
	SSP_RX_32_OR_MORE_ELEM,
	SSP_RX_64_OR_MORE_ELEM,
	SSP_RX_128_OR_MORE_ELEM,
	SSP_RX_224_OR_MORE_ELEM
};

enum ssp_tx_level_trig {
	SSP_TX_1_OR_MORE_EMPTY_LOC,
	SSP_TX_4_OR_MORE_EMPTY_LOC,
	SSP_TX_8_OR_MORE_EMPTY_LOC,
	SSP_TX_16_OR_MORE_EMPTY_LOC,
	SSP_TX_32_OR_MORE_EMPTY_LOC,
	SSP_TX_64_OR_MORE_EMPTY_LOC,
	SSP_TX_128_OR_MORE_EMPTY_LOC
};

/*
 * enum ssp_clock_params - clock parameters, to set SSP clock at a
 * desired freq
 */
struct ssp_clock_params {
	u8 cpsdvsr; /* value from 2 to 254 (even only!) */
	u8 scr; /* value from 0 to 255 */
};

/*
 * struct hs_spi_dev - This is the private SSP driver data structure
 * @pdev: platform device model hookup
 * @phybase: the physical memory where the SSP device resides
 * @virtbase: the virtual memory where the SSP is mapped
 * @clk: outgoing clock "SPICLK" for the SPI bus
 * @master: SPI framework hookup
 * @master_info: controller-specific data from machine setup
 * @pump_transfers: Tasklet used in Interrupt Transfer mode
 * @cur_msg: Pointer to current spi_message being processed
 * @cur_transfer: Pointer to current spi_transfer
 * @cur_chip: pointer to current clients chip(assigned from controller_state)
 * @next_msg_cs_active: the next message in the queue has been examined
 *  and it was found that it uses the same chip select as the previous
 *  message, so we left it active after the previous transfer, and it's
 *  active already.
 * @tx: current position in TX buffer to be read
 * @tx_end: end position in TX buffer to be read
 * @rx: current position in RX buffer to be written
 * @rx_end: end position in RX buffer to be written
 * @read: the type of read currently going on
 * @write: the type of write currently going on
 * @exp_fifo_level: expected FIFO level
 * @dma_rx_channel: optional channel for RX DMA
 * @dma_tx_channel: optional channel for TX DMA
 * @sgt_rx: scattertable for the RX transfer
 * @sgt_tx: scattertable for the TX transfer
 * @dummypage: a dummy page used for driving data on the bus with DMA
 * @cur_cs: current chip select (gpio)
 * @chipselects: list of chipselects (gpios)
 */
struct hs_spi_controller {
	struct platform_device		*pdev;
	resource_size_t			phybase;
	void __iomem			*virtbase;
	struct clk			*clk;
	struct spi_master		*master;
	struct hs_spi_platform_data	*pdata;
	/* Message per-transfer pump */
	struct tasklet_struct		pump_transfers;
	struct spi_message		*cur_msg;
	struct spi_transfer		*cur_transfer;
	struct chip_data		*cur_chip;
	bool				next_msg_cs_active;
	bool				internal_cs_ctrl;
	void				*tx;
	void				*tx_end;
	void				*rx;
	void				*rx_end;
	int			         irq;
	enum ssp_reading		read;
	enum ssp_writing		write;
	u32				exp_fifo_level;
	enum ssp_rx_level_trig		rx_lev_trig;
	enum ssp_tx_level_trig		tx_lev_trig;
	/* DMA settings */
#ifdef CONFIG_DMA_ENGINE
	struct dma_chan			*dma_rx_channel;
	struct dma_chan			*dma_tx_channel;
	struct sg_table			sgt_rx;
	struct sg_table			sgt_tx;
	char				*dummypage;
	bool				dma_running;
#endif
	/* Two optional pin states - default & sleep */
	struct pinctrl                  *pinctrl;
	struct pinctrl_state            *pins_default;
	struct pinctrl_state            *pins_idle;
	struct pinctrl_state            *pins_sleep;
	void				*tx_buffer;
	void				*rx_buffer;
	int hardware_mutex;
	int dmacheck_addr;
	struct hwspinlock *spi_hwspin_lock;
	struct dev_pin_info pins;
	spinlock_t sync_spinlock;
	int sync_locked;
	enum ssp_transfer_state transfer_state;
	u32 transfer_len;
	spinlock_t lock;
	int cur_cs;
	int *chipselects;
};

struct dma_chan;
/*
 * struct hs_spi_device - device.platform_data for SPI controller devices.
 * @bus_id: identifier for this bus
 * @num_chipselect: chipselects are used to distinguish individual
 *     SPI slaves, and are numbered from zero to num_chipselects - 1.
 *     each slave has a chipselect signal, but it's common that not
 *     every chipselect is connected to a slave.
 * @enable_dma: if true enables DMA driven transfers.
 * @dma_rx_param: parameter to locate an RX DMA channel.
 * @dma_tx_param: parameter to locate a TX DMA channel.
 * @autosuspend_delay: delay in ms following transfer completion before the
 *     runtime power management system suspends the device. A setting of 0
 *     indicates no delay and the device will be suspended immediately.
 * @rt: indicates the controller should run the message pump with realtime
 *     priority to minimise the transfer latency on the bus.
 * @chipselects: list of <num_chipselects> chip select gpios
 */
struct hs_spi_platform_data {
	u16 bus_id;
	u8 num_chipselect;
	u8 enable_dma : 1;
	bool (*dma_filter)(struct dma_chan *chan, void *filter_param);
	void *dma_rx_param;
	void *dma_tx_param;
	int *chipselects;
};

/*
 * struct hs_spi_config_chip - spi_board_info.controller_data for SPI
 * slave devices, copied to spi_device.controller_data.
 *
 * @clk_freq: Tune freq parameters of SSP(when in master mode)
 * @com_mode: communication mode: polling, Interrupt or DMA
 * @rx_lev_trig: Rx FIFO watermark level (for IT & DMA mode)
 * @tx_lev_trig: Tx FIFO watermark level (for IT & DMA mode)
 * @cs_control: function pointer to board-specific function to
 * assert/deassert I/O port to control HW generation of devices chip-select.
 */
struct hs_spi_config_chip {
	struct ssp_clock_params clk_freq;
	enum ssp_mode com_mode;
	bool cs_ctrl_en;
	enum ssp_speed_mode speed_mode;
	enum ssp_rx_level_trig rx_lev_trig;
	enum ssp_tx_level_trig tx_lev_trig;
	void (*cs_control) (u32 control);
};

/*
 * struct chip_data - To maintain runtime state of SSP for each client chip
 * @cr0: Value of control register CR0 of SSP - on later ST variants this
 *       register is 32 bits wide rather than just 16
 * @cr1: Value of control register CR1 of SSP
 * @dmacr: Value of DMA control Register of SSP
 * @cpsr: Value of Clock prescale register
 * @n_bytes: how many bytes(power of 2) reqd for a given data width of client
 * @enable_dma: Whether to enable DMA or not
 * @read: function ptr to be used to read when doing xfer for this chip
 * @write: function ptr to be used to write when doing xfer for this chip
 * @cs_control: chip select callback provided by chip
 * @xfer_type: polling/interrupt/DMA
 *
 * Runtime state of the SSP controller, maintained per chip,
 * This would be set according to the current message that would be served
 */
struct chip_data {
	u32 csr; /* cs register */
	u32 cr; /* common register */
	u32 enr; /* enable spi register */
	u8 n_bytes;
	bool enable_dma;
	enum ssp_reading read;
	enum ssp_writing write;
	void (*cs_control) (u32 command);
	int xfer_type;
};

#endif
