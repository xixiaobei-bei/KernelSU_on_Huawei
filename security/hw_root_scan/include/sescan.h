/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 * Description: the sescan.h for selinux status checking
 * Author: Yongzheng Wu <Wu.Yongzheng@huawei.com>
 *         likun <quentin.lee@huawei.com>
 *         likan <likan82@huawei.com>
 * Create: 2016-06-18
 */

#ifndef _SESCAN_H_
#define _SESCAN_H_

#include <asm/sections.h>
#include <linux/crypto.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/scatterlist.h>
#include <linux/security.h>
#include <linux/string.h>
#include <linux/version.h>
#include <crypto/hash.h>
#include <crypto/hash_info.h>
#include "./include/hw_rscan_utils.h"
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
#include "security.h"
#endif

/* selinux_enforcing is kernel variable */
#ifdef CONFIG_SECURITY_SELINUX_DEVELOP
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
#define SELINUX_ENFORCING enforcing_enabled(&selinux_state) ? 1 : 0
#else
extern int selinux_enforcing;
#define SELINUX_ENFORCING selinux_enforcing
#endif
#else
#define SELINUX_ENFORCING 1
#endif

int get_selinux_enforcing(void);
int sescan_hookhash(uint8_t *hash, size_t hash_len);

#endif

