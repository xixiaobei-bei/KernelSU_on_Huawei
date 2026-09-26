/*
 * stm32_spi.h
 *
 * camera driver source file
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#ifndef _STM32_SPI_H_
#define _STM32_SPI_H_

#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/spi/spi.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/spi/spi.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/ctype.h>
#include <linux/regulator/consumer.h>
#include <linux/vmalloc.h>
#include <linux/random.h>
#include <linux/irq.h>
#include <linux/amba/pl022.h>
#include <linux/pinctrl/consumer.h>
#include <dsm/dsm_pub.h>
#include <linux/of_gpio.h>
#include <linux/clk.h>

#define STM32_SPI_SPEED_5M        5000000
#define STM32_SPI_SPEED_8M        8000000
#define SPI_TX_BUF_SIZE           64
#define SPI_RX_BUF_SIZE           64
#define SPI_BLOCK_BUF_SIZE        4096


struct err_map {
	u8    err_head;
	char *err_name;
	u32   err_num;
};

enum load_fw_status {
	LOAD_FW_NOT_START,
	LOAD_FW_DOING,
	LOAD_FW_DONE,
	LOAD_FW_FAIL,
	LOAD_FW_UNKNOWN
};

struct stm32_spi_plat_data {
	int spi_cs_gpio;
	int irq_gpio;
	int reset_gpio;
	int boot_gpio;
	int strobe_gpio;
	int chip_type;
	const char *fpga_clkname;
	/* spi master config */
	struct pl022_config_chip spidev0_chip_info;
	/* pin control config */
	struct pinctrl          *pinctrl;
	struct pinctrl_state    *pins_default;
	struct pinctrl_state    *pins_idle;
};

struct stm32_spi_priv_data {
	struct spi_device *spi;
	struct mutex busy_lock;
	struct stm32_spi_plat_data plat_data;
	int state;
	u8 last_err_code;
	/*
	 * NOTE: All buffers should be dma-safe buffers
	 * tx_buf :used for 64-Bytes CMD-send or 8K-Bytes Block-send
	 * rx_buf :used for 64-Bytes CMD-recv or 4K-Bytes Block-recv
	 */
	u8 *tx_buf;
	u8 *rx_buf;
	u8 *ext_buf;
	/*
	 * power status flag
	 */
	int power_number;        /* powerup and powerdown times */
	int enable_number;       /* enable and disable times */
	int power_up_times;      /* power up times */
	enum load_fw_status load_fw_state;       /* load firmware status */
	int load_fw_number;
	int load_fw_fail_time;
	int self_check_fail_time;
	/*
	 * NOTE:
	 * all these tx/rx/ext buffers above used with mutex lock, but
	 * irq cannot use mutex lock, so a special buffer is supplied
	 */
	u8 *irq_buf;
	struct work_struct dump_err_work;
	struct workqueue_struct *work_queue;
	struct dsm_client *client_stm32;
};

int stm32_spi_init(void);
int stm32_spi_exit(void);
int stm32_spi_enable(void);
int stm32_spi_disable(void);
int stm32_spi_load_fw(void);
int stm32_spi_init_fun(void);
int stm32_spi_close_fun(void);
int stm32_spi_checkdevice(void);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("stm32 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
#endif
