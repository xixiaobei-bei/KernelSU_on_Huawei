/*
 * zrhung_wp_io.h
 *
 * This file is the header file for hung wp io
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#ifndef __LINUX_ZRHUNG_WP_IO_H__
#define __LINUX_ZRHUNG_WP_IO_H__

#ifdef CONFIG_HW_ZEROHUNG
void iowp_workqueue_init(void);
void iowp_report(pid_t pid, pid_t count, char *name);
#endif
#endif
