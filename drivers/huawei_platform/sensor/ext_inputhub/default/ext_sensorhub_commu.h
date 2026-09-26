/*
 * ext_sensorhub_commu.h
 *
 * head file for communication with external sensorhub
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

#ifndef EXT_SENSORHUB_COMMU_H
#define EXT_SENSORHUB_COMMU_H
#include <linux/types.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>

#define COMMU_DRIVER_COUNT 3

struct commu_data;

enum commu_driver {
	SDIO_DRIVER = 0x00,
	SPI_DRIVER = 0x01,
	UART_DRIVER = 0x02,
};

struct commu_ops {
	int (*op_commu_init)(struct commu_data *commu_data);
	void (*op_commu_exit)(struct commu_data *commu_data);
	int (*op_read_count)(u32 *read_len);
	int (*op_commu_read)(u8 *buf, u32 len);
	size_t (*op_write_count)(size_t len);
	int (*op_commu_write)(u8 *buf, u32 len);
	int (*op_read_ready)(bool is_ready);
	void (*op_write_ready)(u8 *buf, size_t len);

	irqreturn_t (*op_commu_wake_irq)(int irq, void *arg);
	irqreturn_t (*op_commu_rx_irq)(int irq, void *arg);
};

struct commu_data {
	/* The mutex lock for communication operation */
	struct mutex commu_op_lock;
	unsigned int wake_irq;
	unsigned int rx_ready_irq;
	int wakeup_ap_gpio;
	int wakeup_mcu_gpio;
	int ap_read_ready_gpio;
	int mcu_read_ready_gpio;
	unsigned char *wakeup_ap;
	unsigned char *wakeup_mcu;
	unsigned char *ap_read_ready;
	unsigned char *mcu_read_ready;

	struct task_struct *read_kworker_task;
	struct kthread_work read_work;
	struct kthread_worker read_kworker;

	struct work_struct wakeup_read_work;
	struct work_struct init_work;
	struct wakeup_source wake_lock;
	struct wakeup_source user_wake_lock;
	bool initialed;
	bool resumed;

	struct semaphore sr_wake_sema;
	void __iomem *ao_gpio_base;
	struct commu_ops commu_ops;
	enum commu_driver driver;
	int active_handshake;

	int commu_direct_mode;
};

struct commu_data *get_commu_data(enum commu_driver driver);

void ext_sensorhub_commu_init(void);

void ext_sensorhub_commu_exit(void);

int commu_init(struct commu_data *commu_data);

void commu_exit(struct commu_data *commu_data);

ssize_t commu_write(struct commu_data *commu_data,
		    unsigned char *buf, size_t len);

void commu_suspend(enum commu_driver driver);

void commu_resume(enum commu_driver driver);

/* set uart mode */
void set_uart_commu_mode(int mode);

/* get uart mode */
int get_uart_commu_mode(void);

int ext_sensorhub_spi_direct_write(u8 *buf, u32 len, u32 txLen, u8 *txBuf);

/* mcu force upgrade */
int ext_sensorhub_set_force_upgrade_mode(int mode);

int get_spi_commu_mode(void);

int ext_sensorhub_mcu_boot(int value);

int ext_sensorhub_mcu_reset(int value);

int ext_sensorhub_reset(void);

void set_spi_read_len(u32 len);

int ext_sensorhub_commu_uart_close(void);

int ext_sensorhub_commu_uart_open(void);

#endif /* EXT_SENSORHUB_COMMU_H */
