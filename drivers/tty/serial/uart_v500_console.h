/*
 * Console Driver for UART serial ports
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
#ifndef __UART_V500_CONSOLE_H_
#define __UART_V500_CONSOLE_H_
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_core.h>
#include <linux/device.h>
#include <linux/clk.h>
#include <linux/slab.h>
#include <linux/serial.h>
#include <linux/dmaengine.h>
#include <linux/scatterlist.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/types.h>
#include <linux/of_device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/sizes.h>
#include <linux/io.h>
#include <linux/kfifo.h>
#include <linux/kthread.h>
#include <linux/of_address.h>
#include <linux/version.h>
#include <linux/cpumask.h>
#include <linux/acpi.h>
#include <securec.h>
#include <pr_log.h>
#include <linux/debugfs.h>
#include "uart_v500_reg.h"

#define PR_LOG_TAG                      "uart_v500"
#define UART_DEVICENAME                   "ttyHW"
#define UART_ISR_PASS_LIMIT               256
#define UART_SERIAL_MAJOR                 204
#define MINOR_START                       64
#define MAX_UART_IDX                      7
#define UART_ASYNC_PRINT_WORK_TIMEOUT     1600000
#define MAX_DEBUG_FS_NAME_LEN             16
#define MAX_UART_CONSOLE_NAME_LEN         7
#define CONSOLE_FIFO_SIZE_DEFAULT         262144
#define CONSOLE_FIFO_CPUON_DEFAULT        3
#define UART_RESET_WAIT_TIME              10
#define UAP_REG_CFG_NUM                   4
#define TXFF_TIMEOUT                      100000
#define UART_CONFIG_BIT_MASK              16

int __init uart_v500_console_setup(struct console *co, char *options);
struct uart_v500_port *get_uart_ports_uap(int index);
struct uart_v500_dma_data {
	bool (*dma_filter)(struct dma_chan *chan, void *filter_param);
	void *dma_rx_param;
	void *dma_tx_param;
	void (*init)(void);
	void (*exit)(void);
};

#ifdef CONFIG_SERIAL_UART_V500_CONSOLE
static int console_uart_name_is_ttyhw;
int get_console_index(void);
int get_console_name(char *name, int name_buf_len);

#define UART_ASYNC_PRINT_LOCAL_BUF_SIZE 96
#define UART_ASYNC_PRINT_BUF_SIZE_DEF 0x20000 /* 256k */
#define UART_ASYNC_PRINT_CPUON_NOTSET (-1)

void uart_v500_console_write(
	struct console *co, const char *s, unsigned int count);

struct uart_async_print_unit {
	int max_cpus;
	unsigned int async_valid; /* enable or not */
	int async_print_cpuid; /* -1: not set */
	struct kfifo async_print_fifo;
	spinlock_t async_print_lock_in;
	unsigned long async_print_got;
	unsigned long async_print_out;
	unsigned long async_print_sent;
	unsigned long async_print_queued;
	unsigned long async_print_buf_max; /* fifo required size max */
	unsigned long async_print_buf_times;
	unsigned long async_print_over_cnt;
	unsigned long async_print_over_size;
	unsigned long async_print_tasklet_run;
	struct task_struct *thread;
	wait_queue_head_t waitqueue;
	atomic_t async_print_sig;
};
#endif

struct uart_v500_sgbuf {
	struct scatterlist sg;
	char *buf;
};

struct uart_v500_dmarx_data {
	struct dma_chan *chan;
	struct completion complete;
	bool use_buf_b;
	struct uart_v500_sgbuf sgbuf_a;
	struct uart_v500_sgbuf sgbuf_b;
	dma_cookie_t dma_cookie;
	bool running;
};

struct uart_v500_dmatx_data {
	struct dma_chan *chan;
	struct scatterlist sg;
	char *buffer;
	bool queued;
};

struct vendor_data {
	const unsigned int *reg_offset;
	unsigned int fifo_cfg;
	unsigned int stat_busy;
	unsigned int stat_cts;
	bool access_32b;
};

static struct uart_driver g_uart_v500;
static struct console uart_v500_console = {
	.name    = UART_DEVICENAME,
	.write   = uart_v500_console_write,
	.device  = uart_console_device,
	.setup   = uart_v500_console_setup,
	.flags   = CON_PRINTBUFFER,
	.index   = -1,
	.data    = &g_uart_v500,
};

static struct uart_driver g_uart_v500 = {
	.owner = THIS_MODULE,
	.driver_name = "uart_v500_serial",
	.dev_name = UART_DEVICENAME,
	.major = UART_SERIAL_MAJOR,
	.minor = MINOR_START,
	.nr = MAX_UART_IDX,
	.cons = &uart_v500_console,
};

struct uart_v500_port {
	struct uart_port port;
	const unsigned int *reg_offset;
	struct clk *clk;
	/* Two optional pin states - default & sleep */
	struct pinctrl *pinctrl;
	struct pinctrl_state *pinctrls_default;
	struct pinctrl_state *pinctrls_idle;
	const struct vendor_data *vendor;
	unsigned int dma_conctrl; /* dma control reg */
	unsigned int int_msk; /* interrupt mask */
	unsigned int old_status;
	unsigned int bind_interrupt_flag;
	unsigned int fifosize; /* vendor-specific */
	unsigned int old_config;
	bool auto_rts;
	char uart_type[12]; /* uart_type name less than 12 */
#ifdef CONFIG_DMA_ENGINE
	/* DMA stuff */
	bool using_tx_dma;
	struct uart_v500_dmarx_data dmarx;
	bool using_rx_dma;
	struct uart_v500_dmatx_data dmatx;
	bool is_dma_probed;
#endif
#ifdef CONFIG_SERIAL_UART_V500_CONSOLE
	bool reset_val;
	void __iomem *reset_base;
	unsigned int reset_bit;
	unsigned int reset_enable;
	unsigned int reset_disable;
	unsigned int reset_status;
	struct uart_async_print_unit async_print_unit;
#endif
	unsigned int rx_use_global_sg_buf;
	bool rx_global_sg_buf_alloced;
	bool uart_rx_dma_disabled;
};

unsigned int uart_v500_read(const struct uart_v500_port *uap, unsigned int reg);

void uart_v500_write(unsigned int val, const struct uart_v500_port *uap,
	unsigned int reg);

void uart_v500_probe_console_enable(struct platform_device *dev,
	struct uart_v500_port *uap, const char *console_name);

#ifdef CONFIG_SERIAL_UART_V500_CONSOLE
int uart_v500_async_print_work_uninit(struct uart_v500_port *uap);

void uart_v500_console_write_tx(struct console *co, const char *s,
	unsigned int count);

void uart_v500_console_write(struct console *co, const char *s,
	unsigned int count);

void uart_v500_console_putchar(struct uart_port *port, int ch);
#endif

int uart_v500_probe_reset_func_enable(struct platform_device *dev,
	struct uart_v500_port *uap);
void uart_chip_reset_endisable(struct uart_v500_port *dev, unsigned int enable);

int uart_probe_dt_alias(int index, struct device *dev);
int uart_v500_find_free_port(void);

#endif
