/*
 * ext_sensorhub_uart.h
 *
 * head file for external sensorhub uart driver
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
#ifndef EXT_SENSORHUB_UART_H
#define EXT_SENSORHUB_UART_H

#define MAX_UART_READ_BUF_LEN 1100
#define UART_ACK_PKG_LEN 8
#define UART_SHORT_PKG_COUNT 2
#define UART_LONG_PKG_COUNT 3

int ext_sensorhub_uart_init(void);

int ext_sensorhub_uart_exit(void);

int ext_sensorhub_uart_read(u8 *buf, u32 len);

int ext_sensorhub_uart_read_begin(bool is_ready);

int ext_sensorhub_uart_write(u8 *buf, u32 len);

void ext_sensorhub_set_rx_gpio(int rx_gpio);

void ext_sensorhub_set_wakeup_gpio(int wakeup_gpio);

void ext_sensorhub_set_uart_mode(int mode);

void set_uart_speed(unsigned int speed);

int ext_sensorhub_uart_open(void);

int ext_sensorhub_uart_close(void);

void uart_status_lock(void);

void uart_status_unlock(void);

#endif /* EXT_SENSORHUB_UART_H */
