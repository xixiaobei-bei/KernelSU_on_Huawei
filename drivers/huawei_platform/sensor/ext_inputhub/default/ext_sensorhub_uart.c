/*
 * ext_sensorhub_uart.c
 *
 * code for external sensorhub uart driver
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
#include <linux/delay.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <linux/wait.h>
#include <linux/of_gpio.h>

#include "securec.h"
#include "ext_sensorhub_route.h"
#include "ext_sensorhub_uart.h"

#define PUBLIC_BUF_MAX (3 * 1024)
#define N_HW_M3 45
#define FLOW_CTRL_ENABLE 1
#define TTY_NAME "ttyAMA5"
#define MAX_UART_READ_LEN 10
#define COMMU_UART_FIRST_WRITE 128
#define UART_WORK_SPEED 3800000
#define UART_DIRECT_SPEED 115200
#define UART_READ_TIMEOUT 1
#define UART_READ_TIMEOUT_RETRY 12
#define FRAME_HEADER_LEN_UART 6
#define DATA_LEN_INDEX_HIGH 2
#define DATA_LEN_INDEX_LOW 3
#define BITS_PER_BYTE 8
#define UART_ALIGN_BYTES_COUNT 8
#define MIN_TTYB_SIZE 256
#define UART_WAIT_DMA_READY_US 50
#define UART_WAIT_DMA_READY_MS 10
#define UART_WAIT_DMA_READY_TIMES 100
#define UART_WAIT_DMA_READ_READY_TIMES 600
#define UART_IOCTL_CMD 0x5500
#define UART_RX_ADDR 0xE896CF28
#define GPIO_DI_PD 0x2
#define GPIO_DI_NP 0x0

struct uart_receive_data {
	u8 data[MAX_UART_READ_BUF_LEN];
	int aim_count;
	int count;
	bool data_ready;
	bool is_reading;
	int pkg_count;
};

static struct tty_struct *g_tty;
static wait_queue_head_t g_read_wait;
static wait_queue_head_t g_next_wait;
static struct mutex buf_list_lock;
static struct uart_receive_data g_receive_data;
static int g_uart_read_mode;
static int g_ap_rx_rdy_gpio;
static int g_wakeup_ap_gpio;
static bool g_open_status;
static bool g_uart_resume = true;

static int m3_uart_tty_open(void)
{
	struct tty_struct *tty = NULL;
	int ret;
	dev_t dev_no;

	pr_info("%s begin to open uart tty", __func__);
	ret = tty_dev_name_to_number(TTY_NAME, &dev_no);
	if (ret != 0) {
		pr_err("%s can't found tty:%s ret=%d", __func__, TTY_NAME, ret);
		return -ENODEV;
	}

	/* open tty */
	tty = tty_kopen(dev_no);
	if (!tty) {
		pr_err("%s open tty %s failed", __func__, TTY_NAME);
		return -ENODEV;
	}

	if (tty->ops->open) {
		ret = tty->ops->open(tty, NULL);
	} else {
		pr_err("%s tty->ops->open is NULL", __func__);
		ret = -ENODEV;
	}

	if (ret) {
		tty_unlock(tty);
		return -ENODEV;
	}

	g_tty = tty;
	g_open_status = true;
	pr_info("%s tty->ops->open is success", __func__);
	return 0;
}

static void m3_uart_tty_close(void)
{
	struct tty_struct *tty = g_tty;

	pr_info("%s begin", __func__);

	/* close tty */
	if (!tty) {
		pr_err("%s tty is null, ignore", __func__);
		return;
	}

	tty_lock(tty);
	if (tty->ops->close)
		tty->ops->close(tty, NULL);
	else
		pr_warn("%s tty->ops->close is null", __func__);

	tty_unlock(tty);

	tty_kclose(tty);
}

static void m3_ktty_set_termios(struct tty_struct *tty,
				int baud_rate, u8 enable_flowctl)
{
	struct ktermios ktermios;

	ktermios = tty->termios;

	/* close soft flowctrl */
	ktermios.c_iflag &= ~IXON;

	/* set uart cts/rts flowctrl */
	ktermios.c_cflag &= ~CRTSCTS;
	if (enable_flowctl == FLOW_CTRL_ENABLE)
		ktermios.c_cflag |= CRTSCTS;

	/* set csize */
	ktermios.c_cflag &= ~(CSIZE);
	ktermios.c_cflag |= CS8;

	/* set uart baudrate */
	ktermios.c_cflag &= ~CBAUD;
	ktermios.c_cflag |= BOTHER;
	tty_termios_encode_baud_rate(&ktermios, baud_rate, baud_rate);
	tty_set_termios(tty, &ktermios);

	pr_info("%s set baud_rate=%d, except=%d\n",
		__func__, (int)tty_termios_baud_rate(&tty->termios),
		(int)baud_rate);
}

static int ext_sensorhub_open_tty_drv(void)
{
	int ret;
	struct tty_struct *tty = NULL;

	ret = m3_uart_tty_open();
	if (ret < 0)
		return -ENODEV;

	tty = g_tty;
	m3_ktty_set_termios(tty, UART_WORK_SPEED, false);
	tty_unlock(tty);

	pr_info("%s start tty set ldisc 4000 num=%d\n", __func__, N_HW_M3);

	/* set line ldisc */
	ret = tty_set_ldisc(tty, N_HW_M3); /* export after 4.13 */
	if (ret != 0) {
		pr_err("%s failed to set ldisc on tty, ret:%d", __func__, ret);
		return ret;
	}

	return 0;
}

static void ext_sensorhub_release_tty_drv(void)
{
	m3_uart_tty_close();
}

static void uart_tty_wakeup(struct tty_struct *tty)
{
	pr_info("%s begin", __func__);
}

static void uart_tty_flush_buffer(struct tty_struct *tty)
{
	/* do nothing */
	pr_info("%s begin", __func__);
}

static int uart_tty_open(struct tty_struct *tty)
{
	pr_info("%s enter\n", __func__);

	/* don't do an wakeup for now */
	clear_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);

	/* set mem already allocated */
	tty->receive_room = PUBLIC_BUF_MAX;
	/* Flush any pending characters in the driver and discipline. */
	tty_ldisc_flush(tty);
	tty_driver_flush_buffer(tty);

	return 0;
}

static void uart_tty_close(struct tty_struct *tty)
{
	pr_info("%s: entered\n", __func__);

	/* Flush any pending characters in the driver and discipline. */
	tty_ldisc_flush(tty);
	tty_driver_flush_buffer(tty);

	pr_info("tty close done!\n");
}

static void uart_tty_receive_direct(const u8 *data, int count)
{
	u8 *buffer = NULL;
	int ret;
	struct ext_sensorhub_buf_list *resp_list = NULL;

	buffer = kmalloc(MAX_UART_READ_BUF_LEN, GFP_KERNEL);
	if (!buffer)
		return;
	ret = memcpy_s(buffer, MAX_UART_READ_BUF_LEN, data, count);
	if (ret != EOK) {
		kfree(buffer);
		return;
	}

	pr_info("%s g_uart_read_mode direct mode, count:%d", __func__, count);
	/* resp_list free in ext_sensorhub_route_append after used */
	resp_list = kmalloc(sizeof(*resp_list), GFP_KERNEL);
	if (!resp_list) {
		kfree(buffer);
		return;
	}

	resp_list->service_id = SERVICE_ID_COMMU;
	resp_list->command_id = SERVICE_UPGRADE_1132;
	resp_list->read_cnt = count;
	resp_list->buffer = buffer;
	/* buffer also kfree after used */
	ext_sensorhub_route_append(resp_list);
}

static int calc_uart_pkg_len(int data_len)
{
	int total_len = data_len + FRAME_HEADER_LEN_UART;
	bool left = total_len % UART_ALIGN_BYTES_COUNT == 0 ? 0 : 1;
	int real_len = (total_len / UART_ALIGN_BYTES_COUNT + left) *
	UART_ALIGN_BYTES_COUNT;

	return real_len;
}

static int dma_ready(void)
{
	int ret;

	if (!g_tty || !g_tty->ops || !g_tty->ops->ioctl)
		return -EINVAL;

	ret = g_tty->ops->ioctl(g_tty, UART_IOCTL_CMD, 0);
	/* equals 1 means success, -ENOIOCTLCMD means ioctl has not adapt */
	if (ret == 1 || ret == -ENOIOCTLCMD) {
		pr_info("%s get dma ready ret:%d", __func__, ret);
		return 0;
	}
	if (ret == 0)
		ret = -EFAULT;
	return ret;
}

static void uart_read_err_out()
{
	mutex_lock(&buf_list_lock);
	g_receive_data.data_ready = true;
	g_receive_data.count = -EFAULT;
	wake_up_interruptible(&g_read_wait);
	mutex_unlock(&buf_list_lock);
}

static void uart_tty_receive_first(const u8 *data, int count)
{
	unsigned short data_len;
	int ret;
	int i;

	ret = memcpy_s(g_receive_data.data + g_receive_data.count,
		       MAX_UART_READ_BUF_LEN - g_receive_data.count,
		       data, count);
	if (ret != EOK) {
		pr_err("%s memcpy error: %d", __func__, ret);
		return;
	}

	data_len = (g_receive_data.data[DATA_LEN_INDEX_HIGH] << BITS_PER_BYTE) +
	g_receive_data.data[DATA_LEN_INDEX_LOW];

	if (data_len <= COMMU_UART_FIRST_WRITE - FRAME_HEADER_LEN_UART) {
		pr_info("%s uart get in read short first pkg", __func__);
		if (!g_receive_data.is_reading)
			return;

		pr_info("%s uart in read short first pkg reading", __func__);
		ret = wait_event_interruptible_timeout(
			g_next_wait, dma_ready() == 0,
			usecs_to_jiffies(UART_WAIT_DMA_READY_MS));
		if (ret == 0) {
			pr_err("%s wait dma timeout", __func__);
			uart_read_err_out();
			return;
		}
		if (ret < 0) {
			pr_err("%s wait dma error: %d", __func__, ret);
			uart_read_err_out();
			return;
		}
		if (!g_receive_data.is_reading ||
		    gpio_get_value(g_wakeup_ap_gpio) == 0) {
			pr_err("%s wakeup ap is 0 or timeout", __func__);
			uart_read_err_out();
			return;
		}

		gpio_direction_output(g_ap_rx_rdy_gpio, 0);
		g_receive_data.pkg_count = UART_SHORT_PKG_COUNT;
		g_receive_data.aim_count =
		COMMU_UART_FIRST_WRITE + UART_ACK_PKG_LEN;
		g_receive_data.count += count;
		gpio_direction_output(g_ap_rx_rdy_gpio, 1);

		return;
	}
	pr_info("%s uart get in read long first pkg", __func__);

	if (g_receive_data.is_reading) {
		pr_info("%s uart get in read long first pkg reading", __func__);

		ret = wait_event_interruptible_timeout(
			g_next_wait, dma_ready() == 0,
			usecs_to_jiffies(UART_WAIT_DMA_READY_MS));
		if (ret == 0) {
			pr_err("%s wait dma timeout", __func__);
			uart_read_err_out();
			return;
		}
		if (ret < 0) {
			pr_err("%s wait dma error: %d", __func__, ret);
			uart_read_err_out();
			return;
		}
		if (!g_receive_data.is_reading ||
		    gpio_get_value(g_wakeup_ap_gpio) == 0) {
			pr_err("%s wakeup ap is 0 or timeout", __func__);
			uart_read_err_out();
			return;
		}

		gpio_direction_output(g_ap_rx_rdy_gpio, 0);
		g_receive_data.pkg_count = UART_LONG_PKG_COUNT;
		g_receive_data.aim_count = calc_uart_pkg_len(data_len);
		g_receive_data.count += count;
		gpio_direction_output(g_ap_rx_rdy_gpio, 1);
	}
}

static void uart_tty_receive(struct tty_struct *tty, const unsigned char *data,
			     char *tty_flags, int count)
{
	int ret;
	int i;

	pr_info("%s receive data count:%d", __func__, count);
	if (count <= 0)
		return;
	if (g_uart_read_mode != 0) {
		uart_tty_receive_direct(data, count);
		return;
	}
	if (!g_receive_data.is_reading) {
		pr_warn("%s is not reading uart, drop it", __func__);
		return;
	}
	if (g_receive_data.count == 0)
		g_receive_data.aim_count = COMMU_UART_FIRST_WRITE;

	if (g_receive_data.count < COMMU_UART_FIRST_WRITE &&
	    g_receive_data.count + count >= COMMU_UART_FIRST_WRITE) {
		pr_warn("%s receive first package", __func__);
		uart_tty_receive_first(data, count);
		return;
	}

	ret = memcpy_s(g_receive_data.data + g_receive_data.count,
		       MAX_UART_READ_BUF_LEN - g_receive_data.count,
		       data, count);
	if (ret != EOK) {
		pr_warn("%s copy data fail", __func__);
		return;
	}

	mutex_lock(&buf_list_lock);
	g_receive_data.count = g_receive_data.count + count;
	pr_info("%s g_receive_data count:%d, aim count: %d",
		__func__, g_receive_data.count,  g_receive_data.aim_count);
	mutex_unlock(&buf_list_lock);
	if (g_receive_data.count < g_receive_data.aim_count)
		return;

	if (g_receive_data.pkg_count == UART_SHORT_PKG_COUNT) {
		pr_info("%s uart get in read short? second pkg", __func__);
		/* short pkg has done */
		mutex_lock(&buf_list_lock);
		g_receive_data.data_ready = true;
		wake_up_interruptible(&g_read_wait);
		mutex_unlock(&buf_list_lock);
	} else if (g_receive_data.pkg_count == UART_LONG_PKG_COUNT) {
		pr_info("%s uart get in read long second pkg", __func__);
		if (!g_receive_data.is_reading)
			return;

		pr_info("%s uart in read long second pkg reading", __func__);

		ret = wait_event_interruptible_timeout(
			g_next_wait, dma_ready() == 0,
			usecs_to_jiffies(UART_WAIT_DMA_READY_MS));
		if (ret == 0) {
			pr_err("%s wait dma timeout", __func__);
			uart_read_err_out();
			return;
		}
		if (ret < 0) {
			pr_err("%s wait dma error: %d", __func__, ret);
			uart_read_err_out();
			return;
		}
		if (!g_receive_data.is_reading ||
		    gpio_get_value(g_wakeup_ap_gpio) == 0) {
			pr_err("%s wakeup ap is 0 or timeout", __func__);
			uart_read_err_out();
			return;
		}

		gpio_direction_output(g_ap_rx_rdy_gpio, 0);
		g_receive_data.pkg_count = UART_SHORT_PKG_COUNT;
		g_receive_data.aim_count =
		g_receive_data.aim_count + UART_ACK_PKG_LEN;
		gpio_direction_output(g_ap_rx_rdy_gpio, 1);
	}
}

static struct tty_ldisc_ops g_m3_ldisc_ops = {
	.magic = TTY_LDISC_MAGIC,
	.name = "uart_m3",
	.open = uart_tty_open,
	.close = uart_tty_close,
	.receive_buf = uart_tty_receive,
	.write_wakeup = uart_tty_wakeup,
	.flush_buffer = uart_tty_flush_buffer,
	.owner = THIS_MODULE
};

int ext_sensorhub_uart_write(u8 *buf, u32 len)
{
	int ret;
	int count = len;

	if (!g_tty) {
		pr_err("%s tty is null", __func__);
		return -EFAULT;
	}

	if (!g_tty->ops || !g_tty->ops->write) {
		pr_err("%s tty write operation is null", __func__);
		return -EFAULT;
	}

	if (!g_open_status)
		pr_warn("%s uart has not open", __func__);

	while (count > 0) {
		ret = g_tty->ops->write(g_tty, buf + (len - count), count);
		if (ret <= 0)
			return ret;
		count -= ret;
	}

	return len;
}

static bool data_ready(void)
{
	return g_receive_data.data_ready;
}

int ext_sensorhub_uart_read_begin(bool is_ready)
{
	int delay_times = 0;
	int ret;

	mutex_lock(&buf_list_lock);
	g_receive_data.count = 0;
	g_receive_data.is_reading = is_ready;
	mutex_unlock(&buf_list_lock);
	if (is_ready) {
		ret = dma_ready();
		while (ret != 0) {
			if (delay_times++ > UART_WAIT_DMA_READ_READY_TIMES) {
				pr_err("%s wait dma ready delay times is out of range, ret:%d",
				       __func__, ret);
				return -ETIME;
			}
			udelay(UART_WAIT_DMA_READY_US);
			ret = dma_ready();
		}
		pr_info("%s delay_times: %d", __func__, ret);
	}

	return 0;
}

static void tty_buffer_free(struct tty_port *port, struct tty_buffer *b)
{
	struct tty_bufhead *buf = &port->buf;

	/* Dumb strategy for now - should keep some stats */
	WARN_ON(atomic_sub_return(b->size, &buf->mem_used) < 0);

	if (b->size > MIN_TTYB_SIZE)
		kfree(b);
	else if (b->size > 0)
		llist_add(&b->free, &buf->free);
}

static int receive_buf(struct tty_port *port,
		       struct tty_buffer *head, int count)
{
	unsigned char *p = char_buf_ptr(head, head->read);
	char	      *f = NULL;
	int n;

	if (~head->flags & TTYB_NORMAL)
		f = flag_buf_ptr(head, head->read);

	n = port->client_ops->receive_buf(port, p, f, count);
	if (n > 0)
		memset_s(p, n, 0, n);
	return n;
}

static void try_flush_to_ldisc(void)
{
	struct tty_port *port = g_tty->port;
	struct tty_bufhead *buf = &port->buf;
	struct tty_buffer *head = NULL;
	struct tty_buffer *next = NULL;
	int count;

	if (!port || !buf)
		return;

	mutex_lock(&buf->lock);

	while (1) {
		head = buf->head;

		/* Ldisc or user is trying to gain exclusive access */
		if (atomic_read(&buf->priority))
			break;
		/* for next */
		next = smp_load_acquire(&head->next);
		/* for current */
		count = smp_load_acquire(&head->commit) - head->read;
		if (!count) {
			if (!next)
				break;
			buf->head = next;
			tty_buffer_free(port, head);
			continue;
		}

		count = receive_buf(port, head, count);
		if (!count)
			break;
		head->read += count;
	}

	mutex_unlock(&buf->lock);
}

int ext_sensorhub_uart_read(u8 *buf, u32 len)
{
	int ret;
	int copy_len;
	int i;

	if (!g_open_status)
		pr_warn("%s uart has not open", __func__);

	for (i = 0; i < UART_READ_TIMEOUT_RETRY; ++i) {
		ret = wait_event_interruptible_timeout(
			g_read_wait, data_ready(),
			msecs_to_jiffies(UART_READ_TIMEOUT));
		if (ret == 0)
			try_flush_to_ldisc();
		else
			break;
	}
	g_receive_data.is_reading = false;
	if (ret == 0) {
		pr_err("%s read timeout", __func__);
		return -ETIMEDOUT;
	}
	if (ret < 0) {
		pr_err("%s wait resp error: %d", __func__, ret);
		return ret;
	}

	pr_info("%s begin read data count: %d", __func__, g_receive_data.count);
	mutex_lock(&buf_list_lock);
	if (g_receive_data.count <= 0) {
		g_receive_data.count = 0;
		g_receive_data.data_ready = false;
		mutex_unlock(&buf_list_lock);
		return -ETIMEDOUT;
	}
	if (len < g_receive_data.count)
		copy_len = len;
	else
		copy_len = g_receive_data.count;
	ret = memcpy_s(buf, len, g_receive_data.data, copy_len);
	if (ret != EOK) {
		g_receive_data.count = 0;
		g_receive_data.data_ready = false;
		mutex_unlock(&buf_list_lock);
		return -EFAULT;
	}
	g_receive_data.count = 0;
	g_receive_data.data_ready = false;
	mutex_unlock(&buf_list_lock);

	return ret;
}

static void tty_change_speed(struct tty_struct *tty, unsigned int speed)
{
	struct ktermios old_termios;

	down_write(&tty->termios_rwsem);
	old_termios = tty->termios;
	tty_encode_baud_rate(tty, speed, speed);
	if (tty->ops->set_termios)
		tty->ops->set_termios(tty, &old_termios);
	up_write(&tty->termios_rwsem);
}

void set_uart_speed(unsigned int speed)
{
	if (!g_tty) {
		pr_err("%s tty is null", __func__);
		return;
	}
	pr_info("%s set uart speed:%d", __func__, speed);
	tty_change_speed(g_tty, speed);
}

static unsigned long long reg_write_u32(unsigned int addr, unsigned int value)
{
	void __iomem *vir_addr = NULL;

	vir_addr = ioremap(addr, sizeof(unsigned long));
	if (!vir_addr) {
		pr_err("vir_addr is NULL\n");
		return 0;
	}

	writel(value, vir_addr);
	iounmap(vir_addr);
	return 0;
}

void set_rx_pd(void)
{
	reg_write_u32(UART_RX_ADDR, GPIO_DI_PD);
}

void set_rx_np(void)
{
	reg_write_u32(UART_RX_ADDR, GPIO_DI_NP);
}

void ext_sensorhub_set_uart_mode(int mode)
{
	if (!g_tty) {
		pr_err("%s tty is null", __func__);
		return;
	}

	pr_info("%s set uart mode:%d, last mode:%d", __func__, mode,
		g_uart_read_mode);
	if (g_uart_read_mode == mode)
		return;
	if (mode == 0) {
		tty_change_speed(g_tty, UART_WORK_SPEED);
		set_rx_np();
	} else {
		tty_change_speed(g_tty, UART_DIRECT_SPEED);
		set_rx_pd();
	}
	g_uart_read_mode = mode;
}

void ext_sensorhub_set_rx_gpio(int rx_gpio)
{
	g_ap_rx_rdy_gpio = rx_gpio;
}

void ext_sensorhub_set_wakeup_gpio(int wakeup_gpio)
{
	g_wakeup_ap_gpio = wakeup_gpio;
}

int ext_sensorhub_uart_open(void)
{
	int ret;

	if (g_open_status)
		return 0;

	if (!g_tty || !g_tty->ops) {
		pr_err("%s open tty %s failed", __func__, TTY_NAME);
		return -ENODEV;
	}
	pr_info("%s begin", __func__);
	if (g_tty->ops->open) {
		ret = g_tty->ops->open(g_tty, NULL);
	} else {
		pr_err("%s tty->ops->open is NULL", __func__);
		ret = -ENODEV;
	}

	if (ret == 0) {
		g_open_status = true;
		g_uart_resume = true;
		pr_info("%s open tty uart success", __func__);
	}
	pr_info("%s end", __func__);
	return 0;
}

int ext_sensorhub_uart_close(void)
{
	struct tty_struct *tty = g_tty;

	if (!g_uart_resume) {
		pr_info("%s uart has not resume ,cannot close", __func__);
		return -EBUSY;
	}
	if (!g_open_status)
		return 0;

	pr_info("%s begin", __func__);

	/* close tty */
	if (!tty) {
		pr_err("%s tty is null, ignore", __func__);
		return  -ENODEV;
	}

	tty_lock(tty);
	if (tty->ops->close)
		tty->ops->close(tty, NULL);
	else
		pr_warn("%s tty->ops->close is null, cannot close", __func__);
	tty_unlock(tty);
	pr_info("%s end", __func__);
	g_open_status = false;
	return 0;
}

static int uart_sr_notify(struct notifier_block *notify_block,
			  unsigned long mode, void *unused)
{
	switch (mode) {
	case PM_POST_SUSPEND:
		pr_info("%s get in PM_POST_SUSPEND, resume", __func__);
		g_uart_resume = true;
		commu_resume(UART_DRIVER);
		break;
	case PM_SUSPEND_PREPARE:
		pr_info("%s get in PM_SUSPEND_PREPARE, suspend", __func__);
		g_uart_resume = false;
		commu_suspend(UART_DRIVER);
		break;
	default:
	    break;
	}
	return 0;
}

static struct notifier_block g_uart_sr_notifier = {
	.notifier_call = uart_sr_notify,
	.priority = INT_MIN,
};

int ext_sensorhub_uart_init(void)
{
	int ret;

	g_open_status = false;
	g_uart_read_mode = 0;
	g_receive_data.count = 0;
	g_receive_data.data_ready = false;
	g_receive_data.is_reading = false;
	g_receive_data.pkg_count = UART_SHORT_PKG_COUNT;
	mutex_init(&buf_list_lock);
	init_waitqueue_head(&g_read_wait);
	init_waitqueue_head(&g_next_wait);

	ret = tty_register_ldisc(N_HW_M3, &g_m3_ldisc_ops);
	pr_info("%s tty_register_ldisc uart init result: %d", __func__, ret);

	ret = ext_sensorhub_open_tty_drv();
	pr_info("%s ext_sensorhub_open_tty_drv uart init result: %d",
		__func__, ret);

	ret = register_pm_notifier(&g_uart_sr_notifier);
	if (ret < 0)
		pr_warn("%s :register uart pm_notifier failed, ret = %d!",
			__func__, ret);

	return ret;
}

int ext_sensorhub_uart_exit(void)
{
	ext_sensorhub_release_tty_drv();
	return tty_unregister_ldisc(N_HW_M3);
}
