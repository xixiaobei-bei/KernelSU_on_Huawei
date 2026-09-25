/*
 * tc_client_driver.h
 *
 * function declaration for proc open,close session and invoke
 *
 * Copyright (c) 2012-2018 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef _TC_NS_CLIENT_DRIVER_H_
#define _TC_NS_CLIENT_DRIVER_H_

#include <linux/ion.h>
#include <linux/hisi/hisi_ion.h>
#include <crypto/hash.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include "teek_ns_client.h"

extern struct ion_client *g_drm_ion_client;
extern struct tc_ns_dev_list g_tc_ns_dev_list;
extern struct crypto_shash *g_tee_shash_tfm;
extern int g_tee_init_crypt_state;
extern struct mutex g_tee_crypto_hash_lock;
extern struct list_head g_service_list;

int tc_ns_load_image(tc_ns_dev_file *dev_file, char *file_buffer, unsigned int file_size);
#endif

