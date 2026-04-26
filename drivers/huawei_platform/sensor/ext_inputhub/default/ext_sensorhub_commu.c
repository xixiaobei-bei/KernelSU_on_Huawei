/*
 * ext_sensorhub_commu.c
 *
 * code for communication with external sensorhub
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

#include "ext_sensorhub_commu.h"

#include <linux/errno.h>
#include <linux/stddef.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/pm_wakeup.h>
#include <soc_gpio_interface.h>
#include <linux/syscore_ops.h>
#include <linux/preempt.h>
#include <linux/syscalls.h>
#include <uapi/linux/sched/types.h>

#include "securec.h"
#include "ext_sensorhub_sdio.h"
#include "ext_sensorhub_spi.h"
#include "ext_sensorhub_uart.h"
#include "ext_sensorhub_frame.h"

#define COMMU_READ_DELAY_TIME_US	100
#define COMMU_READ_MAX_DELAY_TIMES	200
#define COMMU_WRITE_DELAY_TIME_US	30
#define COMMU_WRITE_MAX_DELAY_TIMES	700
#define COMMU_RETRY_INTERVAL_MS		1
#define COMMU_MAX_RETRY_TIMES		10
#define COMMU_ALIGN_BYTES_COUNT		4
#define AO_GPIO_GROUP_STEP		8
#define IO_BUFFER_LEN			40
/* multiple package */
#define COMMU_WAIT_NEXT_WRITE_DELAY_TIMES 200
#define COMMU_WAIT_NEXT_WRITE_DELAY_TIME_US 5
#define COMMU_UART_FIRST_WRITE 128
#define COMMU_UART_SECOND_WRITE_TIMOUE_MS 20
#define QUEUE_WORK_RETRY_COUNT 3
#define COMMU_SCHED_PRIORITY 2
#define DMA_READY_TIMEOUT_ERRNO 123
#define DMA_READY_WAKE_LOCK_TIME_MS 200
#define RESET_SENSORHUB_INTERVAL_MS 6

struct spi_write_context {
	struct task_struct *kworker_task;
	struct kthread_work write_work;
	struct kthread_worker kworker;
	wait_queue_head_t spi_write_wait;

	u8 *buf;
	u32 len;
	bool spi_write_done;
	int spi_write_flag;
	int spi_ret;
};

struct uart_second_pkg {
	u8 *buffer;
	int len;
	int write_flag;
	wait_queue_head_t write_wait;
	bool write_done;
	int count;
};

struct force_upgrade {
	unsigned int recv_irq;
	struct work_struct recv_work;
	int mcu_rst_gpio;
};

static int ap_status_gpio;
/* enum coumm driver */
static struct commu_data *commu_data_tbl[COMMU_DRIVER_COUNT];
static struct spi_write_context g_spi_context;
static struct uart_second_pkg g_second_pkg;
static int g_uart_write_mode;
static u32 g_spi_read_len = DEFAULT_SPI_READ_LEN;

static inline bool highpri_work(struct work_struct *work)
{
	return queue_work_on(WORK_CPU_UNBOUND, system_highpri_wq, work);
}
static struct force_upgrade g_force_upgrade;

static int commu_request_common_gpio(
	int *gpio, const char *compat, const char *name)
{
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, compat);
	if (!np) {
		pr_err("gpio_request %s: %s node not found\n", __func__, *name);
		return -ENODEV;
	}

	*gpio = of_get_named_gpio(np, name, 0);
	if (*gpio < 0) {
		pr_err("gpio_request error %s:%d.\n", name, *gpio);
		return -ENODEV;
	}

	if (gpio_request(*gpio, name) < 0) {
		pr_err("gpio_request Failed to request gpio %d for %s\n",
		       *gpio, name);
		return -ENODEV;
	}

	return 0;
}

static int commu_request_direction_gpio(
	int *gpio, const char *compat, const char *name)
{
	int ret;

	ret = commu_request_common_gpio(gpio, compat, name);
	if (ret < 0)
		return ret;

	if (gpio_direction_input(*gpio) < 0) {
		pr_err("gpio_request Failed to set dir %d for %s\n",
		       *gpio, compat);
		return -ENODEV;
	}

	return ret;
}

static int commu_request_gpio(struct commu_data *commu_data)
{
	int ret;

	/* Get MCU Wakeup AP GPIO */
	ret = commu_request_direction_gpio(&commu_data->wakeup_ap_gpio,
					   "mcu,wakeupap",
					   commu_data->wakeup_ap);
	if (ret < 0) {
		pr_err("%s: mcu,wakeupap node not found\n", __func__);
		return ret;
	}
	commu_data->wake_irq = gpio_to_irq(commu_data->wakeup_ap_gpio);
	pr_info("request_irq_gpio commu_data.wake_irq = %u",
		commu_data->wake_irq);

	/* Get AP Wakeup MCU Gpio */
	ret = commu_request_common_gpio(&commu_data->wakeup_mcu_gpio,
					"mcu,wakeupmcu",
					commu_data->wakeup_mcu);
	if (ret < 0) {
		pr_err(" %s: mcu,wakeupmcu node not found\n", __func__);
		return ret;
	}
	pr_info("ap_wakeup_mcu gpio:%d done\n",
		commu_data->wakeup_mcu_gpio);

	/* Get AP Read Sync Gpio */
	ret = commu_request_common_gpio(&commu_data->ap_read_ready_gpio,
					"mcu,readsync",
					commu_data->ap_read_ready);
	if (ret < 0) {
		pr_err("%s: mcu,readsync node not found\n", __func__);
		return ret;
	}
	pr_info("ap_read_ready_gpio gpio:%d done\n",
		commu_data->ap_read_ready_gpio);

	/* Get AP Write Sync Gpio */
	ret = commu_request_direction_gpio(&commu_data->mcu_read_ready_gpio,
					   "mcu,writesync",
					   commu_data->mcu_read_ready);
	if (ret < 0) {
		pr_err("%s: mcu,writesync node not found\n", __func__);
		return ret;
	}
	pr_info("mcu_read_ready_gpio gpio:%d done\n",
		commu_data->mcu_read_ready_gpio);
	/* for rx irq */
	commu_data->rx_ready_irq = gpio_to_irq(commu_data->mcu_read_ready_gpio);
	pr_info("request_irq_gpio commu_data.rx_ready_irq = %u",
		commu_data->rx_ready_irq);

	return 0;
}

/* do after gpio init */
static int hisi_get_gpio_base(struct commu_data *commu_data)
{
	struct device_node *dn = NULL;
	unsigned int group;
	char *io_buffer = NULL;
	int err;
	int ret = 0;

	pr_info("%s wake up gpio: %d\n", __func__, commu_data->wakeup_ap_gpio);
	group = commu_data->wakeup_ap_gpio / AO_GPIO_GROUP_STEP;
	io_buffer = kzalloc(IO_BUFFER_LEN, GFP_KERNEL);
	if (!io_buffer)
		return -ENOMEM;
	memset_s(io_buffer, IO_BUFFER_LEN, 0, IO_BUFFER_LEN);
	err = snprintf_s(io_buffer, IO_BUFFER_LEN, IO_BUFFER_LEN - 1,
			 "arm,primecell%u", group);
	if (err < 0) {
		pr_err("[%s]snprintf_s fail[%d]\n", __func__, ret);
		ret = err;
		goto err_free;
	}

	dn = of_find_compatible_node(NULL, NULL, io_buffer);
	if (!dn) {
		pr_err("%s: hisilicon,primecell%d No compatible node found\n",
		       __func__, group);
		ret = -ENODEV;
		goto err_free;
	}
	commu_data->ao_gpio_base = of_iomap(dn, 0);
	if (!commu_data->ao_gpio_base) {
		ret = -EINVAL;
		of_node_put(dn);
		goto err_free;
	}
	of_node_put(dn);
	pr_info("%s get ao gpio base success\n", __func__);

err_free:
	kfree(io_buffer);
	return ret;
}

static void commu_release_gpio(struct commu_data *commu_data)
{
	gpio_free(commu_data->wakeup_ap_gpio);
	gpio_free(commu_data->wakeup_mcu_gpio);
	gpio_free(commu_data->ap_read_ready_gpio);
	gpio_free(commu_data->mcu_read_ready_gpio);
}

static irqreturn_t spi_commu_wake_irq(int irq, void *arg)
{
	struct commu_data *commu_data = (struct commu_data *)arg;
	bool kret = false;
	int i;

	/* check mcu wakeup ap is high, mcu reboot, clear init flag */
	if (gpio_get_value(commu_data->mcu_read_ready_gpio) == 1 &&
	    gpio_get_value(commu_data->wakeup_ap_gpio) == 1) {
		pr_err("%s mcu is rebooting, clear init flag", __func__);
		notify_mcu_reboot(commu_data->driver);
		return IRQ_HANDLED;
	}

	for (i = 0; i < QUEUE_WORK_RETRY_COUNT; ++i) {
		kret = kthread_queue_work(&commu_data->read_kworker,
					  &commu_data->read_work);
		if (kret)
			break;
	}

	return IRQ_HANDLED;
}

static irqreturn_t sdio_commu_wake_irq(int irq, void *arg)
{
	struct commu_data *commu_data = (struct commu_data *)arg;

	/* check mcu wakeup ap is high, mcu reboot, clear init flag */
	if (gpio_get_value(commu_data->mcu_read_ready_gpio) == 1 &&
	    gpio_get_value(commu_data->wakeup_ap_gpio) == 1) {
		pr_err("%s mcu is rebooting, clear init flag", __func__);
		commu_data->initialed = false;
		notify_mcu_reboot(commu_data->driver);
		return IRQ_HANDLED;
	}

	highpri_work(&commu_data->wakeup_read_work);
	return IRQ_HANDLED;
}

static irqreturn_t uart_commu_wake_irq(int irq, void *arg)
{
	struct commu_data *commu_data = (struct commu_data *)arg;

	/* check mcu wakeup ap is high, 1132 reboot, clear init flag */
	if (gpio_get_value(commu_data->mcu_read_ready_gpio) == 1 &&
	    gpio_get_value(commu_data->wakeup_ap_gpio) == 1) {
		pr_err("%s 1132 is rebooting, clear init flag", __func__);
		notify_mcu_reboot(commu_data->driver);
		return IRQ_HANDLED;
	}

	highpri_work(&commu_data->wakeup_read_work);
	return IRQ_HANDLED;
}

static irqreturn_t spi_commu_rx_irq(int irq, void *arg)
{
	g_spi_context.spi_write_flag++;
	return IRQ_HANDLED;
}

static irqreturn_t sdio_commu_rx_irq(int irq, void *arg)
{
	struct commu_data *commu_data = (struct commu_data *)arg;

	if (gpio_get_value(commu_data->mcu_read_ready_gpio) == 1)
		schedule_work(&commu_data->init_work);
	return IRQ_HANDLED;
}

static int commu_request_irq(struct commu_data *commu_data)
{
	int retval;

	if (!commu_data) {
		pr_err("%s commu_data is null\n", __func__);
		return -EINVAL;
	}

	retval = request_irq(commu_data->wake_irq,
			     commu_data->commu_ops.op_commu_wake_irq,
			     IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND,
			     "commu wake irq", commu_data);
	if (retval < 0) {
		pr_err("couldn't acquire MCU HOST WAKE UP IRQ reval = %d\n",
		       retval);
		return retval;
	}

	if (commu_data->driver == SPI_DRIVER) {
		retval = request_irq(commu_data->rx_ready_irq,
				     commu_data->commu_ops.op_commu_rx_irq,
				     IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND,
				     "commu wake irq", commu_data);
	} else if (commu_data->driver == SDIO_DRIVER) {
		retval = request_irq(commu_data->rx_ready_irq,
				     commu_data->commu_ops.op_commu_rx_irq,
				     IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND,
				     "commu wake irq", commu_data);
	} else {
		retval = request_irq(commu_data->rx_ready_irq,
				     commu_data->commu_ops.op_commu_rx_irq,
				     IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND,
				     "commu wake irq", commu_data);
	}
	if (retval < 0) {
		pr_err("couldn't acquire MCU HOST INIT IRQ reval = %d\n",
		       retval);
		return retval;
	}

	return 0;
}

static int commu_read_check(struct commu_data *commu_data)
{
	enum commu_driver driver;
	int ret;

	if (!commu_data || !commu_data->initialed) {
		pr_err("%s driver:%d commu data not init\n", __func__, driver);
		return -EINVAL;
	}

	if (commu_data->commu_direct_mode)
		return -EBUSY;

	driver = commu_data->driver;
	if (gpio_get_value(commu_data->wakeup_ap_gpio) != 1) {
		pr_warn("%s driver:%d wake up ap has pull down, do nothing",
			__func__, driver);
		return -EINVAL;
	}

	ret = down_timeout(&commu_data->sr_wake_sema, 6 * HZ);
	if (ret == -ETIME) {
		pr_err("%s driver:%d down sr wake sema timeout",
		       __func__, driver);
		return -EINVAL;
	}
	up(&commu_data->sr_wake_sema);
	/* check mcu rx ready is high, mcu reboot */
	if (gpio_get_value(commu_data->mcu_read_ready_gpio) == 1 &&
	    gpio_get_value(commu_data->wakeup_ap_gpio) == 1) {
		pr_err("%s driver:%d wakeup_ap_gpio is high, mcu rebooting",
		       __func__, driver);
		return -EINVAL;
	}

	return 0;
}

static void commu_wakeup_read(struct commu_data *commu_data)
{
	unsigned int delay_times = 0;
	int ret;
	unsigned char *read_buf = NULL;
	u32 read_len = 0;
	enum commu_driver driver;
	bool need_decode = false;

	if (commu_read_check(commu_data) != 0)
		return;
	driver = commu_data->driver;
	mutex_lock(&commu_data->commu_op_lock);
	if (!commu_data->initialed)
		goto err;
	__pm_stay_awake(&commu_data->wake_lock);
	if (gpio_get_value(commu_data->wakeup_ap_gpio) != 1) {
		pr_err("%s driver:%d wakeup_ap_gpio is not high",
		       __func__, driver);
		goto err;
	}
	/* for driver do some setting */
	if (commu_data->commu_ops.op_read_ready) {
		ret = commu_data->commu_ops.op_read_ready(true);
		if (ret < 0) {
			ret = -DMA_READY_TIMEOUT_ERRNO;
			goto err;
		}
	}
	gpio_direction_output(commu_data->ap_read_ready_gpio, 1);
	/* wait for, uart driver send and then pull down, do not need check */
	if (commu_data->driver != UART_DRIVER) {
		while (gpio_get_value(commu_data->wakeup_ap_gpio) == 1) {
			if (!commu_data->initialed)
				goto err;
			if (delay_times++ > COMMU_READ_MAX_DELAY_TIMES) {
				pr_err("%s driver: %d delay times is out of range",
				       __func__, driver);
				goto err;
			}
			udelay(COMMU_READ_DELAY_TIME_US);
		}
	}
	ret = commu_data->commu_ops.op_read_count(&read_len);
	if (ret != 0 || read_len == 0) {
		pr_err("%s driver:%d get read count:%d, error %d",
		       __func__, driver, read_len, ret);
		goto err;
	}
	/* buffer has one more byte for spi driver */
	read_buf = kmalloc(read_len + 1, GFP_KERNEL);
	if (!read_buf)
		goto err;
	/* Execute Read Operation, initial already */
	ret = commu_data->commu_ops.op_commu_read(read_buf, read_len);
	if (ret < 0) {
		pr_err("%s driver:%d read from driver error, ret:%d\n",
		       __func__, driver, ret);
		goto err;
	}
	need_decode = true;
err:
	if (commu_data->commu_ops.op_read_ready)
		commu_data->commu_ops.op_read_ready(false);
	gpio_direction_output(commu_data->ap_read_ready_gpio, 0);
	mutex_unlock(&commu_data->commu_op_lock);
	if (need_decode) {
		ret = decode_frame_resp(read_buf, read_len, commu_data->driver);
		if (ret < 0)
			pr_err("%s driver:%d decode resp error: %d",
			       __func__, driver, ret);
	}
	kfree(read_buf);
	__pm_relax(&commu_data->wake_lock);
	if (ret == -DMA_READY_TIMEOUT_ERRNO) {
		__pm_wakeup_event(&commu_data->wake_lock,
				  DMA_READY_WAKE_LOCK_TIME_MS);
		pr_warn("%s driver:%d wake lock for:%d ms",
			__func__, driver, DMA_READY_WAKE_LOCK_TIME_MS);
	}
	read_buf = NULL;
}

static void commu_read_worker(struct kthread_work *work)
{
	struct commu_data *commu_data = container_of(work, struct commu_data,
		read_work);

	commu_wakeup_read(commu_data);
}

static void commu_wakeup_read_work(struct work_struct *work)
{
	struct commu_data *commu_data = container_of(work, struct commu_data,
		wakeup_read_work);

	commu_wakeup_read(commu_data);
}

ssize_t commu_write(struct commu_data *commu_data,
		    unsigned char *buf, size_t len)
{
	ssize_t status;
	unsigned int write_retry_times = 0;
	unsigned int wait_retry_times = 0;
	unsigned int delay_times = 0;
	size_t send_len;
	enum commu_driver driver;

	if (!commu_data)
		return -EINVAL;
	driver = commu_data->driver;
	if (commu_data->commu_direct_mode) {
		pr_err("%s driver:%d is on direct mode\n", __func__, driver);
		return -EBUSY;
	}
	/* send len may > data len,but must < FRAME_BUF_LEN(buf contains '0') */
	send_len = commu_data->commu_ops.op_write_count(len);
	if (send_len <= 0)
		return -EBUSY;
writeretry:
	mutex_lock(&commu_data->commu_op_lock);
	if (!commu_data->initialed) {
		status = -EREMOTEIO;
		goto err;
	}

	if (gpio_get_value(commu_data->mcu_read_ready_gpio) == 1) {
		write_retry_times++;
		mutex_unlock(&commu_data->commu_op_lock);
		msleep(COMMU_RETRY_INTERVAL_MS);
		if (write_retry_times <= COMMU_MAX_RETRY_TIMES) {
			goto writeretry;
		} else {
			pr_err("%s driver:%d last package is writing\n",
			       __func__, driver);
			return -ETIME;
		}
	}

	if (gpio_get_value(commu_data->wakeup_ap_gpio) == 1) {
		gpio_direction_output(commu_data->wakeup_mcu_gpio, 0);
		mutex_unlock(&commu_data->commu_op_lock);
		msleep(COMMU_RETRY_INTERVAL_MS);
		wait_retry_times++;
		if (wait_retry_times <= COMMU_MAX_RETRY_TIMES) {
			goto writeretry;
		} else {
			pr_err("%s driver:%d bus compete times is out of range\n",
			       __func__, driver);
			return -ETIME;
		}
	}

	if (commu_data->commu_ops.op_write_ready)
		commu_data->commu_ops.op_write_ready(buf, len);
	if (!commu_data->resumed) {
		pr_err("%s driver:%d is not resumed", __func__, driver);
		status = -EACCES;
		goto err;
	}
	gpio_direction_output(commu_data->wakeup_mcu_gpio, 1);
	if (commu_data->driver != UART_DRIVER) {
		while (gpio_get_value(commu_data->mcu_read_ready_gpio) != 1) {
			if (!commu_data->initialed) {
				status = -EREMOTEIO;
				goto err;
			}
			delay_times++;
			if (delay_times > COMMU_WRITE_MAX_DELAY_TIMES) {
				status = -ETIME;
				pr_err("%s driver:%d delay times is out of range\n",
				       __func__, driver);
				goto err;
			}
			udelay(COMMU_WRITE_DELAY_TIME_US);
		}
	}

	/* write to driver */
	status = commu_data->commu_ops.op_commu_write(buf, send_len);

err:
	if (commu_data->commu_ops.op_write_ready)
		commu_data->commu_ops.op_write_ready(NULL, 0);
	gpio_direction_output(commu_data->wakeup_mcu_gpio, 0);
	mutex_unlock(&commu_data->commu_op_lock);
	return status;
}

static void commu_init_work(struct work_struct *work)
{
	int ret;
	struct commu_data *commu_data = container_of(work, struct commu_data,
		init_work);

	pr_debug("%s begin to do communication init work check\n", __func__);
	if (!commu_data) {
		pr_err("%s commu data NULL\n", __func__);
		return;
	}

	/* ap is writing, not a initial work */
	if (commu_data->initialed ||
	    gpio_get_value(commu_data->wakeup_mcu_gpio) == 1)
		return;

	pr_info("%s is initial work detect sdio card\n", __func__);
	__pm_stay_awake(&commu_data->wake_lock);
	ret = down_timeout(&commu_data->sr_wake_sema, 12 * HZ);
	if (ret == -ETIME) {
		pr_err("%s down sr wake sema timeout", __func__);
		__pm_relax(&commu_data->wake_lock);
		return;
	}
	up(&commu_data->sr_wake_sema);
	ext_sensorhub_detect_sdio_card();
	commu_data->initialed = true;
	__pm_relax(&commu_data->wake_lock);
}

static int sdio_commu_init(struct commu_data *commu_data)
{
	int status;

	INIT_WORK(&commu_data->init_work, commu_init_work);
	status = ext_sensorhub_sdio_func_probe();

	return status;
}

static void sdio_commu_exit(struct commu_data *commu_data)
{
	ext_sensorhub_sdio_exit();
	cancel_work_sync(&commu_data->init_work);
}

static size_t sdio_write_count(size_t len)
{
	size_t send_len;
	size_t align_factor = COMMU_ALIGN_BYTES_COUNT;

	// 512 bytes
	if (len >= HISDIO_BLOCK_SIZE)
		align_factor = HISDIO_BLOCK_SIZE;

	if (len % align_factor == 0)
		send_len = len;
	else
		send_len = (len / align_factor + 1) * align_factor;

	return send_len;
}

static void force_upgrade_recv_work(struct work_struct *work)
{
	/* upload irq to commu msg */
	notify_mcu_force_upgrade_irq();
}

static void force_upgrade_init(struct commu_data *commu_data)
{
	int ret;

	INIT_WORK(&g_force_upgrade.recv_work, force_upgrade_recv_work);
	g_force_upgrade.recv_irq = gpio_to_irq(commu_data->ap_read_ready_gpio);
	ret = commu_request_common_gpio(&g_force_upgrade.mcu_rst_gpio,
					"mcu,forceupgrade", "mcu_reset");
	if (ret < 0)
		pr_warn("%s request mcu reset gpio failed", __func__);
}

static int spi_commu_init(struct commu_data *commu_data)
{
	int ret;

	ret = ext_sensorhub_spi_init();
	commu_data->initialed = true;

	force_upgrade_init(commu_data);
	return ret;
}

static void spi_commu_exit(struct commu_data *commu_data)
{
	ext_sensorhub_spi_exit();
}

/* adapt for read len change */
void set_spi_read_len(u32 len)
{
	g_spi_read_len = len;
	pr_info("%s spi read len: %d", __func__, len);
}

static int spi_read_count(u32 *read_len)
{
	*read_len = g_spi_read_len - 1;
	return 0;
}

static int spi_commu_read(u8 *buf, u32 len)
{
	int ret;
	int i;

	/* read buffer already has one more byte */
	ret = ext_sensorhub_spi_read(buf, len + 1);
	if (ret < 0)
		return ret;

	/* remove data beginning 0x00 */
	for (i = 0; i < len; ++i)
		buf[i] = buf[i + 1];

	return ret;
}

static int commu_spi_wait_rdy(int index)
{
	int status = 0;
	int delay_times = 0;
	struct commu_data *commu_data = commu_data_tbl[SPI_DRIVER];

	while (gpio_get_value(commu_data->mcu_read_ready_gpio) != 1 ||
	       g_spi_context.spi_write_flag <= index) {
		delay_times++;
		if (delay_times > COMMU_WAIT_NEXT_WRITE_DELAY_TIMES) {
			status = -ETIME;
			pr_err("%s wait for rdy up delay times is out of range",
			       __func__);
			goto err;
		}
		udelay(COMMU_WAIT_NEXT_WRITE_DELAY_TIME_US);
	}

err:
	return status;
}

static void spi_write_worker(struct kthread_work *work)
{
	int ret;
	int i;
	int each_len = SPI_SEND_DATA_LEN;
	int pkg_count = g_spi_context.len / each_len +
	(g_spi_context.len % each_len == 0 ? 0 : 1);
	int send_len;
	int copy_len;
	u8 *buffer = NULL;

	/* write begin with one more byte 0x80 */
	buffer = kmalloc(each_len + 1, GFP_KERNEL);
	if (!buffer) {
		ret = -ENOMEM;
		goto err;
	}
	if (g_spi_context.spi_write_done) {
		ret = -EACCES;
		goto err;
	}
	g_spi_context.spi_write_flag = 0;
	for (i = 0; i < pkg_count; ++i) {
		copy_len = each_len;
		send_len = each_len + 1;
		if (i == pkg_count - 1) {
			copy_len = g_spi_context.len -
			each_len * (pkg_count - 1);
			send_len = SPI_SEND_LEN -
			(each_len + 1) * (pkg_count - 1);
		}

		ret = memcpy_s(&buffer[1], each_len,
			       g_spi_context.buf + i * SPI_SEND_DATA_LEN,
			       copy_len);
		if (ret != EOK) {
			ret = -EFAULT;
			goto err;
		}
		buffer[0] = 0x80;
		ret = ext_sensorhub_spi_write(buffer, send_len);
		if (ret < 0)
			goto err;
		/* wait for another rdy pull down and up, last pkg don't wait */
		if (i != pkg_count - 1) {
			ret = commu_spi_wait_rdy(i);
			if (ret != 0)
				goto err;
		}
	}
err:
	g_spi_context.spi_write_flag = 0;
	g_spi_context.spi_ret = ret;
	g_spi_context.spi_write_done = true;
	wake_up_interruptible(&g_spi_context.spi_write_wait);
	kfree(buffer);
}

static int spi_suspend_clear_last_write(void)
{
	pr_info("%s clear last write", __func__);
	g_spi_context.spi_write_flag = 0;
	g_spi_context.spi_ret = -EACCES;
	g_spi_context.spi_write_done = true;
	gpio_direction_output(commu_data_tbl[SPI_DRIVER]->wakeup_mcu_gpio, 0);
	wake_up_interruptible(&g_spi_context.spi_write_wait);
}

static int spi_init_task(void)
{
	int ret;
	struct sched_param param = { .sched_priority = COMMU_SCHED_PRIORITY };

	init_waitqueue_head(&g_spi_context.spi_write_wait);
	kthread_init_worker(&g_spi_context.kworker);
	g_spi_context.kworker_task = kthread_run(kthread_worker_fn,
						 &g_spi_context.kworker,
						 "%s", "spi_write_worker");
	if (IS_ERR(g_spi_context.kworker_task)) {
		pr_err("failed to create spi write task");
		return -EINVAL;
	}
	kthread_init_work(&g_spi_context.write_work, spi_write_worker);

	ret = sched_setscheduler(g_spi_context.kworker_task,
				 SCHED_FIFO, &param);
	pr_info("%s get init sched_setscheduler ret:%d", __func__, ret);
}

static int spi_commu_write(u8 *buffer, u32 length)
{
	int ret;
	bool kret = false;
	int i;
	int retry_count;

	g_spi_context.buf = buffer;
	g_spi_context.len = length;
	g_spi_context.spi_write_done = false;
	for (i = 0; i < QUEUE_WORK_RETRY_COUNT; ++i) {
		kret = kthread_queue_work(&g_spi_context.kworker,
					  &g_spi_context.write_work);
		if (kret)
			break;
	}
	if (!kret) {
		g_spi_context.buf = NULL;
		g_spi_context.len = 0;
		g_spi_context.spi_write_done = true;
		__pm_relax(&commu_data_tbl[SPI_DRIVER]->wake_lock);
		pr_err("%s queue spi kwork err", __func__);
		return -EAGAIN;
	}

	/* try to get in certain time */
	retry_count = 0;
	ret = 0;
	while (!g_spi_context.spi_write_done) {
		retry_count++;
		udelay(COMMU_WRITE_DELAY_TIME_US);
		if (retry_count >= COMMU_WRITE_MAX_DELAY_TIMES) {
			ret = -ETIME;
			break;
		}
	}
	if (ret < 0) {
		pr_info("%s cannot get write result in certain time, ret:%d, done flag:%d",
			__func__, ret, g_spi_context.spi_write_done);
		ret = wait_event_interruptible(g_spi_context.spi_write_wait,
					       g_spi_context.spi_write_done);
	}
	if (ret < 0) {
		g_spi_context.buf = NULL;
		g_spi_context.len = 0;
		pr_warn("%s spi write end get ret:%d", __func__, ret);
	}

	return g_spi_context.spi_ret;
}

static irqreturn_t force_upgrade_recv(int irq, void *arg)
{
	pr_info("%s get irq:%d", __func__);
	if (!schedule_work(&g_force_upgrade.recv_work))
		pr_info("%s schedule work error", __func__);
}

/* here to switch ap&mcu rdy gpio mode */
int ext_sensorhub_set_force_upgrade_mode(int mode)
{
	int retval;
	struct commu_data *commu_data = __INCLUDE_LEVEL__;

	if (!commu_data_tbl[SPI_DRIVER])
		return -EINVAL;

	pr_info("%s set mode:%d", __func__, mode);
	commu_data = commu_data_tbl[SPI_DRIVER];
	/* 0 normal mode */
	if (mode == 0) {
		/* set mcu_read_ready_gpio to input and request irq */
		gpio_direction_output(commu_data->mcu_read_ready_gpio, 0);
		gpio_direction_input(commu_data->mcu_read_ready_gpio);
		retval = request_irq(commu_data->rx_ready_irq,
				     commu_data->commu_ops.op_commu_rx_irq,
				     IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND,
				     "commu wake irq", commu_data);
		if (retval < 0)
			pr_warn("%s request irq failed", __func__, retval);

		free_irq(g_force_upgrade.recv_irq, commu_data);
		gpio_direction_output(commu_data->ap_read_ready_gpio, 0);
		gpio_direction_output(g_force_upgrade.mcu_rst_gpio, 0);
		commu_data->commu_direct_mode = 0;
	} else {
		commu_data->commu_direct_mode = 1;
		/* switch to upgrade mode. 1. free irq, 2. set gpio to low */
		free_irq(commu_data->rx_ready_irq, commu_data);
		gpio_direction_output(commu_data->mcu_read_ready_gpio, 0);
		gpio_direction_output(commu_data->ap_read_ready_gpio, 0);

		/* get mcu initial interrupt */
		gpio_direction_input(commu_data->ap_read_ready_gpio);
		retval = request_irq(g_force_upgrade.recv_irq,
				     force_upgrade_recv,
				     IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND,
				     "force upgrade", commu_data);
		if (retval < 0)
			pr_warn("%s request force upgrade irq failed",
				__func__, retval);
	}

	return 0;
}

int get_spi_commu_mode(void)
{
	if (!commu_data_tbl[SPI_DRIVER])
		return -EINVAL;
	return commu_data_tbl[SPI_DRIVER]->commu_direct_mode;
}

int ext_sensorhub_mcu_boot(int value)
{
	if (!commu_data_tbl[SPI_DRIVER])
		return -EINVAL;

	pr_info("%s set value:%d, mode:%d", __func__, value,
		commu_data_tbl[SPI_DRIVER]->commu_direct_mode);
	if (commu_data_tbl[SPI_DRIVER]->commu_direct_mode == 0)
		return -EPERM;

	mutex_lock(&commu_data_tbl[SPI_DRIVER]->commu_op_lock);
	gpio_direction_output(
			commu_data_tbl[SPI_DRIVER]->mcu_read_ready_gpio, value);
	mutex_unlock(&commu_data_tbl[SPI_DRIVER]->commu_op_lock);

	return 0;
}

int ext_sensorhub_mcu_reset(int value)
{
	if (!commu_data_tbl[SPI_DRIVER])
		return -EINVAL;

	if (commu_data_tbl[SPI_DRIVER]->commu_direct_mode == 0)
		return -EPERM;

	pr_info("%s set value:%d, mode:%d, mcu_rst_gpio:%d", __func__, value,
		commu_data_tbl[SPI_DRIVER]->commu_direct_mode,
		g_force_upgrade.mcu_rst_gpio);

	mutex_lock(&commu_data_tbl[SPI_DRIVER]->commu_op_lock);
	gpio_direction_output(g_force_upgrade.mcu_rst_gpio, value);
	mutex_unlock(&commu_data_tbl[SPI_DRIVER]->commu_op_lock);

	return 0;
}

int ext_sensorhub_reset(void)
{
	if (!commu_data_tbl[SPI_DRIVER])
		return -EINVAL;

	pr_info("%s reset sensorhub, mcu_rst_gpio:%d", __func__,
		g_force_upgrade.mcu_rst_gpio);

	mutex_lock(&commu_data_tbl[SPI_DRIVER]->commu_op_lock);
	gpio_direction_output(g_force_upgrade.mcu_rst_gpio, 1);
	mdelay(RESET_SENSORHUB_INTERVAL_MS);
	gpio_direction_output(g_force_upgrade.mcu_rst_gpio, 0);
	mutex_unlock(&commu_data_tbl[SPI_DRIVER]->commu_op_lock);

	return 0;
}

int ext_sensorhub_spi_direct_write(u8 *buf, u32 len, u32 tx_len, u8 *tx_buf)
{
	int ret;
	struct commu_data *commu_data = commu_data_tbl[SPI_DRIVER];

	if (!commu_data)
		return -EINVAL;

	mutex_lock(&commu_data->commu_op_lock);
	ret = ext_sensorhub_spi_update(buf, len, tx_len, tx_buf);
	mutex_unlock(&commu_data->commu_op_lock);

	return ret;
}

static size_t spi_write_count(size_t len)
{
	return SPI_FRAME_BUF_LEN;
}

void set_uart_commu_mode(int mode)
{
	g_uart_write_mode = mode;
	ext_sensorhub_set_uart_mode(mode);
}

int get_uart_commu_mode(void)
{
	return g_uart_write_mode;
}

static int uart_commu_init(struct commu_data *commu_data)
{
	int ret;

	g_uart_write_mode = 0;
	g_second_pkg.buffer = NULL;
	g_second_pkg.len = 0;
	g_second_pkg.write_done = false;
	g_second_pkg.count = UART_SHORT_PKG_COUNT;
	init_waitqueue_head(&g_second_pkg.write_wait);
	ret = ext_sensorhub_uart_init();
	ext_sensorhub_set_rx_gpio(commu_data->ap_read_ready_gpio);
	ext_sensorhub_set_wakeup_gpio(commu_data->wakeup_ap_gpio);
	commu_data->initialed = true;

	return ret;
}

static void uart_commu_exit(struct commu_data *commu_data)
{
	ext_sensorhub_uart_exit();
}

/* uart is async, can do send in irq handler */
static irqreturn_t uart_commu_rx_irq(int irq, void *arg)
{
	int second_len;
	int len = g_second_pkg.len;
	u8 *buf = g_second_pkg.buffer;

	g_second_pkg.write_flag++;
	pr_info("%s get int lwrite_flag：%d",
		__func__, g_second_pkg.write_flag);
	if (g_second_pkg.write_flag > 2 && g_second_pkg.buffer &&
	    g_second_pkg.count == UART_LONG_PKG_COUNT) {
		pr_info("%s get int long last 8", __func__);
		ext_sensorhub_uart_write(
			g_second_pkg.buffer,
			g_second_pkg.len);
		g_second_pkg.buffer = NULL;
		g_second_pkg.len = 0;
		g_second_pkg.write_done = true;
		wake_up_interruptible(&g_second_pkg.write_wait);
		return IRQ_HANDLED;
	}

	if (g_second_pkg.write_flag > 1 && g_second_pkg.buffer) {
		ext_sensorhub_uart_write(g_second_pkg.buffer, g_second_pkg.len);
		if (g_second_pkg.count == UART_SHORT_PKG_COUNT) {
			pr_info("%s get int short last 8", __func__);
			g_second_pkg.buffer = NULL;
			g_second_pkg.len = 0;
			g_second_pkg.write_done = true;
			wake_up_interruptible(&g_second_pkg.write_wait);
			return IRQ_HANDLED;
		} else if (g_second_pkg.count == UART_LONG_PKG_COUNT) {
			pr_info("%s get int long second", __func__);
			/* long pkg write ack */
			g_second_pkg.buffer =
			g_second_pkg.buffer + COMMU_UART_FIRST_WRITE;
			g_second_pkg.len = UART_ACK_PKG_LEN;

			return IRQ_HANDLED;
		}
	}

	if (g_second_pkg.write_flag > 0 && g_second_pkg.buffer) {
		ext_sensorhub_uart_write(buf, COMMU_UART_FIRST_WRITE);
		if (len <= COMMU_UART_FIRST_WRITE) {
			pr_info("%s get int short first", __func__);
			/* last 8 byte buffer */
			g_second_pkg.count = UART_SHORT_PKG_COUNT;
			g_second_pkg.buffer = buf + COMMU_UART_FIRST_WRITE;
			g_second_pkg.len = UART_ACK_PKG_LEN;

			return IRQ_HANDLED;
		} else {
			pr_info("%s get int long first", __func__);
			g_second_pkg.count = UART_LONG_PKG_COUNT;
			second_len = len - COMMU_UART_FIRST_WRITE;
			second_len = second_len % 8 == 0 ?
					second_len : (second_len / 8 + 1) * 8;

			g_second_pkg.len = second_len;
			g_second_pkg.buffer = buf + COMMU_UART_FIRST_WRITE;

			return IRQ_HANDLED;
		}
	}

	return IRQ_HANDLED;
}

static int uart_read_count(u32 *read_len)
{
	*read_len = MAX_UART_READ_BUF_LEN;
	return 0;
}

static int uart_commu_read(u8 *buf, u32 len)
{
	int ret;

	ret = ext_sensorhub_uart_read(buf, len);

	return ret;
}

static int uart_read_ready(bool is_ready)
{
	int ret;

	if (is_ready)
		/* begin read, already get op lock, here cannot close uart */
		/* here to open uart */
		ext_sensorhub_uart_open();

	ret = ext_sensorhub_uart_read_begin(is_ready);

	if (is_ready) {
		if (gpio_get_value(
			commu_data_tbl[UART_DRIVER]->wakeup_ap_gpio) != 1) {
			pr_err("%s uart driver wakeup_ap_gpio is not high",
			       __func__);
			return -ETIME;
		}
	}

	return ret;
}

static void uart_write_ready(unsigned char *buf, size_t len)
{
	if (buf && len > 0) {
		/* here to open uart already get op lock */
		ext_sensorhub_uart_open();
		g_second_pkg.write_flag = 0;
		g_second_pkg.buffer = buf;
		g_second_pkg.len = len;
		g_second_pkg.write_done = false;
	} else {
		g_second_pkg.write_flag = 0;
		g_second_pkg.buffer = NULL;
		g_second_pkg.len = 0;
		g_second_pkg.write_done = false;
	}
}

static int commu_uart_wait_complete(void)
{
	int ret;
	int status = 0;

	ret = wait_event_interruptible_timeout(
		g_second_pkg.write_wait, g_second_pkg.write_done,
		msecs_to_jiffies(COMMU_UART_SECOND_WRITE_TIMOUE_MS));
	if (ret == 0) {
		pr_err("%s wait for second complete timeout", __func__);
		return -ETIMEDOUT;
	}
	if (ret < 0) {
		pr_err("%s wait for second complete error: %d", __func__, ret);
		return ret;
	}

	return status;
}

static int uart_commu_write(u8 *buf, u32 len)
{
	int ret;

	if (g_uart_write_mode != 0) {
		pr_warn("%s uart is in direct mode", __func__, ret);
		return -EBUSY;
	}

	ret = commu_uart_wait_complete();
	if (ret != 0) {
		pr_err("%s uart write wait next err:%d", __func__, ret);
		goto err;
	}
err:

	g_second_pkg.buffer = NULL;
	g_second_pkg.len = 0;
	g_second_pkg.write_flag = 0;
	return ret;
}

static size_t uart_write_count(size_t len)
{
	int ret;

	ret = down_timeout(&commu_data_tbl[UART_DRIVER]->sr_wake_sema, 6 * HZ);
	if (ret == -ETIME) {
		pr_err("%s driver:%d down sr wake sema timeout",
		       __func__, UART_DRIVER);
		return 0;
	}
	up(&commu_data_tbl[UART_DRIVER]->sr_wake_sema);
	return len;
}

void commu_suspend(enum commu_driver driver)
{
	commu_data_tbl[driver]->resumed = false;
	if (down_interruptible(&commu_data_tbl[driver]->sr_wake_sema))
		pr_err("%s down sr wake sema error", __func__);
	notify_commu_wakeup(false, driver);
}

void commu_resume(enum commu_driver driver)
{
	commu_data_tbl[driver]->resumed = true;
	up(&commu_data_tbl[driver]->sr_wake_sema);
}

static bool check_resume_irq(struct commu_data *commu_data)
{
	unsigned int offset;
	bool ret = false;
	unsigned int data;

	offset = commu_data->wakeup_ap_gpio % AO_GPIO_GROUP_STEP;
	data = readl(commu_data->ao_gpio_base + SOC_GPIO_GPIOIE_ADDR(0));
	data = data & readl(commu_data->ao_gpio_base +
		SOC_GPIO_GPIOMIS_ADDR(0));
	ret = (data & BIT((u32)offset)) != 0;
	pr_info("%s pending result: %d\n", __func__, ret);
	return ret;
}

static void record_commu_wakeup(void)
{
	int i;

	if (ap_status_gpio != -1) {
		gpio_direction_output(ap_status_gpio, 1);
		pr_info("%s ap status notify resume", __func__);
	}

	for (i = 0; i < COMMU_DRIVER_COUNT; ++i) {
		if (commu_data_tbl[i] && check_resume_irq(commu_data_tbl[i])) {
			pr_info("will record for dubai");
			notify_commu_wakeup(true, i);
			return;
		}
	}
}

static int record_commu_suspend(void)
{
	if (ap_status_gpio != -1) {
		gpio_direction_output(ap_status_gpio, 0);
		pr_info("%s ap status notify suspend", __func__);
	}

	if (commu_data_tbl[SPI_DRIVER])
		spi_suspend_clear_last_write();

	return 0;
}

static struct syscore_ops dubai_pm_syscore_ops = {
	.resume = record_commu_wakeup,
	.suspend = record_commu_suspend,
};

static int read_work_init_task(struct commu_data *commu_data)
{
	int ret;
	struct sched_param param = { .sched_priority = COMMU_SCHED_PRIORITY };

	kthread_init_worker(&commu_data->read_kworker);
	commu_data->read_kworker_task = kthread_run(
		kthread_worker_fn, &commu_data->read_kworker,
		"%s %d", "read_worker",
		commu_data->driver);
	if (IS_ERR(commu_data->read_kworker_task)) {
		pr_err("failed to create read work task");
		return -EINVAL;
	}
	kthread_init_work(&commu_data->read_work, commu_read_worker);

	ret = sched_setscheduler(commu_data->read_kworker_task,
				 SCHED_FIFO, &param);
	pr_info("%s get init sched_setscheduler ret:%d", __func__, ret);
}

int commu_init(struct commu_data *commu_data)
{
	int status;

	sema_init(&commu_data->sr_wake_sema, 1);
	commu_data->initialed = false;
	commu_data->commu_direct_mode = 0;
	commu_data->resumed = true;
	pr_info("%s begin", __func__);
	mutex_init(&commu_data->commu_op_lock);

	status = commu_request_gpio(commu_data);
	if (status < 0)
		pr_err("%s request gpio error, status: %d", __func__, status);

	if (commu_data->driver == SPI_DRIVER)
		spi_init_task();
	INIT_WORK(&commu_data->wakeup_read_work, commu_wakeup_read_work);
	read_work_init_task(commu_data);
	status = commu_request_irq(commu_data);
	if (status < 0)
		pr_err("%s request irq error, status: %d", __func__, status);

	wakeup_source_init(&commu_data->wake_lock, "mcu_commu");
	wakeup_source_init(&commu_data->user_wake_lock, "mcu_user_commu");
	status = hisi_get_gpio_base(commu_data);
	if (status < 0)
		return -EFAULT;
	/* driver probe */
	status = commu_data->commu_ops.op_commu_init(commu_data);
	if (status < 0)
		pr_err("%s driver probe error, status: %d", __func__, status);

	pr_info("%s end , status:%d\n", __func__, status);

	return status;
}

void commu_exit(struct commu_data *commu_data)
{
	commu_data->commu_ops.op_commu_exit(commu_data);
	/* stop qpio irq */
	free_irq(commu_data->wake_irq, commu_data);

	if (commu_data->initialed)
		/* stop read work */
		cancel_work_sync(&commu_data->wakeup_read_work);

	/* gpio release */
	commu_release_gpio(commu_data);
}

/* for global init */
void ext_sensorhub_commu_init(void)
{
	int i;
	int ret;

	/* init NULL */
	for (i = 0; i < COMMU_DRIVER_COUNT; ++i)
		commu_data_tbl[i] = NULL;

	/* Get ap status GPIO */
	ret = commu_request_common_gpio(&ap_status_gpio,
					"ap,status", "ap_status");
	if (ret < 0) {
		pr_err("%s: ap,status node not found", __func__);
		ap_status_gpio = -1;
	} else {
		pr_info("%s: request ap,status gpio success", __func__);
		gpio_direction_output(ap_status_gpio, 1);
	}

	register_syscore_ops(&dubai_pm_syscore_ops);
}

/* for global exit */
void ext_sensorhub_commu_exit(void)
{
	unregister_syscore_ops(&dubai_pm_syscore_ops);
}

static void commu_data_uart_init(struct commu_data *commu_data)
{
	commu_data->commu_ops.op_commu_init = uart_commu_init;
	commu_data->commu_ops.op_commu_exit = uart_commu_exit;
	commu_data->commu_ops.op_read_count = uart_read_count;
	commu_data->commu_ops.op_commu_read = uart_commu_read;
	commu_data->commu_ops.op_write_count = uart_write_count;
	commu_data->commu_ops.op_commu_write = uart_commu_write;
	commu_data->commu_ops.op_commu_rx_irq = uart_commu_rx_irq;
	commu_data->commu_ops.op_commu_wake_irq = uart_commu_wake_irq;
	commu_data->commu_ops.op_read_ready = uart_read_ready;
	commu_data->commu_ops.op_write_ready = uart_write_ready;

	commu_data->driver = UART_DRIVER;
	commu_data->active_handshake = 1;
}

static void commu_data_spi_init(struct commu_data *commu_data)
{
	commu_data->commu_ops.op_commu_init = spi_commu_init;
	commu_data->commu_ops.op_commu_exit = spi_commu_exit;
	commu_data->commu_ops.op_read_count = spi_read_count;
	commu_data->commu_ops.op_commu_read = spi_commu_read;
	commu_data->commu_ops.op_write_count = spi_write_count;
	commu_data->commu_ops.op_commu_write = spi_commu_write;
	commu_data->commu_ops.op_commu_rx_irq = spi_commu_rx_irq;
	commu_data->commu_ops.op_commu_wake_irq = spi_commu_wake_irq;

	commu_data->driver = SPI_DRIVER;
	commu_data->active_handshake = 1;
}

static void commu_data_sdio_init(struct commu_data *commu_data)
{
	commu_data->commu_ops.op_commu_init = sdio_commu_init;
	commu_data->commu_ops.op_commu_exit = sdio_commu_exit;
	commu_data->commu_ops.op_read_count =
	ext_sensorhub_mmc_sdio_get_xfercount;
	commu_data->commu_ops.op_commu_read =
	ext_sensorhub_mmc_sdio_dev_read;
	commu_data->commu_ops.op_write_count = sdio_write_count;
	commu_data->commu_ops.op_commu_write =
	ext_sensorhub_mmc_sdio_dev_write;
	commu_data->commu_ops.op_commu_rx_irq = sdio_commu_rx_irq;
	commu_data->commu_ops.op_commu_wake_irq = sdio_commu_wake_irq;

	commu_data->driver = SDIO_DRIVER;
	commu_data->active_handshake = 0;
}

struct commu_data *get_commu_data(enum commu_driver driver)
{
	struct commu_data *commu_data = NULL;

	if (commu_data_tbl[driver])
		return commu_data_tbl[driver];

	/* free after used */
	commu_data = kmalloc(sizeof(*commu_data), GFP_KERNEL);
	if (!commu_data)
		return NULL;

	commu_data->commu_ops.op_read_ready = NULL;
	commu_data->commu_ops.op_write_ready = NULL;
	switch (driver) {
	case UART_DRIVER:
		commu_data_uart_init(commu_data);
		break;
	case SPI_DRIVER:
		commu_data_spi_init(commu_data);
		break;
	case SDIO_DRIVER:
	/* fallthrough */
	default:
		commu_data_sdio_init(commu_data);
	}
	if (driver == UART_DRIVER) {
		commu_data->wakeup_ap = "1132_wakeup_ap";
		commu_data->wakeup_mcu = "ap_wakeup_1132";
		commu_data->ap_read_ready = "ap_read_1132_sync";
		commu_data->mcu_read_ready = "ap_write_1132_sync";
	} else {
		commu_data->wakeup_ap = "mcu_wakeup_ap";
		commu_data->wakeup_mcu = "ap_wakeup_mcu";
		commu_data->ap_read_ready = "ap_read_sync";
		commu_data->mcu_read_ready = "ap_write_sync";
	}

	commu_data_tbl[driver] = commu_data;
	return commu_data;
}

int ext_sensorhub_commu_uart_close(void)
{
	int ret;
	struct commu_data *commu_data = commu_data_tbl[UART_DRIVER];

	if (!commu_data || !commu_data->initialed)
		return -EINVAL;

	mutex_lock(&commu_data->commu_op_lock);
	ret = ext_sensorhub_uart_close();
	mutex_unlock(&commu_data->commu_op_lock);

	return ret;
}

int ext_sensorhub_commu_uart_open(void)
{
	int ret;
	struct commu_data *commu_data = commu_data_tbl[UART_DRIVER];

	if (!commu_data || !commu_data->initialed)
		return -EINVAL;

	mutex_lock(&commu_data->commu_op_lock);
	ret = ext_sensorhub_uart_open();
	mutex_unlock(&commu_data->commu_op_lock);

	return ret;
}
