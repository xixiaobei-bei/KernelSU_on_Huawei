/*
 * hiusbc_debug.h -- Debugfs Header File for Hisilicon USB Controller.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __HIUSBC_DEBUGFS_H
#define __HIUSBC_DEBUGFS_H

#include "hiusbc_core.h"

void hiusbc_debugfs_init(struct hiusbc *hiusbc);
void hiusbc_debugfs_exit(struct hiusbc *hiusbc);
#endif /* __HIUSBC_DEBUGFS_H */
