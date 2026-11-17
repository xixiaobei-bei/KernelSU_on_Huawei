/*
 * hiview_selinux.h
 *
 * Hisilicon K3 SOC DFX source file
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#ifndef HIVIEW_SELINUX_H
#define HIVIEW_SELINUX_H

#include <linux/lsm_audit.h>

#define CONFIG_HUAWEI_HIVIEW_SELINUX

int hw_hiview_selinux_avc_audit(struct common_audit_data *cad);

#endif
