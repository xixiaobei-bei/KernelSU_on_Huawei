/*
 * A driver for the camera fpga.
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
 */

#ifndef _ICE40_SPI_H_
#define _ICE40_SPI_H_

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
//#include <mach/gpio.h>
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
#include <media/huawei/depthisp_cfg.h>
#include <linux/of_gpio.h>
#include <linux/clk.h>

#define ICE40_SPI_SPEED_5M        (5000000)
#define ICE40_SPI_SPEED_10M       (10000000)
#define ICE40_SPI_SPEED_25M       (25000000)

#define SPI_TX_BUF_SIZE            (64)
#define SPI_RX_BUF_SIZE            (64)
#define SPI_BLOCK_BUF_SIZE         (4096)

#define ICE40_FPGA_FW_FILE             ("/odm/etc/top_bitmap.bin")

struct err_map
{
    u8    err_head;
    char *err_name;
    u32   err_num;
};

typedef enum
{
    LOAD_FW_NOT_START,
    LOAD_FW_DOING,
    LOAD_FW_DONE,
    LOAD_FW_FAIL,
    LOAD_FW_UNKNOWN
}load_fw_status;

struct ice40_spi_plat_data {
    int spi_cs_gpio;
    int irq_gpio;
    int reset_gpio;
    int enable_gpio;
    int chip_type;
    const char *fpga_clkname;
    /* spi master config */
    struct pl022_config_chip spidev0_chip_info;
    /* pin control config */
    struct pinctrl          *pinctrl;
    struct pinctrl_state    *pins_default;
    struct pinctrl_state    *pins_idle;
};

struct ice40_spi_priv_data {
    struct spi_device           *spi;
    struct mutex                busy_lock;
    struct ice40_spi_plat_data  plat_data;
    int            state;
    u8             last_err_code;
    struct clk *fpga_clkout;
    /*
     * NOTE: All buffers should be dma-safe buffers
     * tx_buf :used for 64-Bytes CMD-send or 8K-Bytes Block-send
     * rx_buf :used for 64-Bytes CMD-recv or 4K-Bytes Block-recv
     */
    u8       *tx_buf;
    u8       *rx_buf;
    u8       *ext_buf;
    /*
     * power status flag
     */
    int            power_number;        /*powerup and powerdown times*/
    int            enable_number;       /*enable and disable times*/
    int            power_up_times;      /*power up times*/
    load_fw_status load_fw_state;       /*load firmware status*/
    int            load_fw_fail_time;
    int            self_check_fail_time;
    /*
     * NOTE:
     * all these tx/rx/ext buffers above used with mutex lock, but
     * irq cannot use mutex lock, so a special buffer is supplied
     */
    u8                      *irq_buf;
    struct work_struct         dump_err_work;
    struct workqueue_struct    *work_queue;
    struct dsm_client       *client_ice40;
};

int ice40_spi_init(void);
int ice40_spi_exit(void);
int ice40_spi_enable(void);
int ice40_spi_disable(void);
int ice40_spi_load_fw(void);
void fpga_enable_gpio_power_up(u8 val);
int ice40_spi_init_fun(void);
int ice40_spi_close_fun(void);

#endif
