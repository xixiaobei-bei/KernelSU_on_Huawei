/*
 * hiusbc_host.h -- Driver for Hisilicon USB Controller.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __HIUSBC_HOST_H
#define __HIUSBC_HOST_H

#include "hiusbc_core.h"

#define HIUSBC_HOST_RESOURCE_NUM 2
#define HIUSBC_HOST_XHCI_REG_END 0x2fff

int hiusbc_host_init(struct hiusbc *hiusbc);
void hiusbc_host_exit(struct hiusbc *hiusbc);

#endif /* __HIUSBC_HOST_H */
