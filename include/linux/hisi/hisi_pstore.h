/*
 * hisi_pstore.h
 *
 * hisi pstore
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#ifndef __BB_HISI_PERSIST_STORE__
#define __BB_HISI_PERSIST_STORE__
#include <linux/types.h>
#ifdef CONFIG_HISI_BB
void hisi_save_pstore_log(const char *name, const void *data, size_t size);
void hisi_free_persist_store(void);
void hisi_create_pstore_entry(void);
void hisi_remove_pstore_entry(void);
#else
static inline void hisi_save_pstore_log(const char *name, const void *data, size_t size) {};
static inline void hisi_free_persist_store(void) {};
static inline void hisi_create_pstore_entry(void) {};
static inline void hisi_remove_pstore_entry(void) {};
#endif

#endif // __BB_HISI_PERSIST_STORE__
