/*
  * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
  * Description: for proc open,close session and invoke.
  * Author: qiqingchao  q00XXXXXX
  * Create: 2016-06-21
  */


#ifndef _TC_NS_CLIENT_DRIVER_H_
#define _TC_NS_CLIENT_DRIVER_H_

#include <linux/ion.h>
#include <linux/hisi/hisi_ion.h>
#include "teek_ns_client.h"
#include <crypto/hash.h>
#include <linux/list.h>
#include <linux/mutex.h>

extern struct ion_client *g_drm_ion_client;
extern struct tc_ns_dev_list g_tc_ns_dev_list;
extern struct crypto_shash *g_tee_shash_tfm;
extern int g_tee_init_crypt_state;
extern struct mutex g_tee_crypto_hash_lock;
extern struct list_head g_service_list;

int tc_ns_load_image(tc_ns_dev_file *dev_file, char *file_buffer, unsigned int file_size);
#endif

