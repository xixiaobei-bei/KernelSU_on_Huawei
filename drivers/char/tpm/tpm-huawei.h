/*
 * tpm driver for GP
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */
#include <linux/spi/spi.h>

#define GPIO_HIGH                       (1)
#define GPIO_LOW                        (0)

int tpm_reset(struct spi_device *dev);
int tpm_set_voltage(struct spi_device *dev);
int tpm_set_voltage_and_reset(struct spi_device *dev);

