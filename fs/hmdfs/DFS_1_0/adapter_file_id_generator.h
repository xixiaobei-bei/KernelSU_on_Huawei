/* SPDX-License-Identifier: GPL-2.0 */
/*
 * adapter_file_id_generator.h
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: koujilong@huawei.com
 * Create: 2020-04-14
 *
 */

#ifndef HMDFS_ADAPTER_FILE_ID_GENERATOR_H
#define HMDFS_ADAPTER_FILE_ID_GENERATOR_H

#include "hmdfs.h"
#include <linux/types.h>

int hmdfs_adapter_generate_file_id(struct hmdfs_sb_info *sbi, const char *dir,
				   const char *name, uint32_t *out_id);
int hmdfs_adapter_remove_file_id(uint32_t id);
int hmdfs_adapter_remove_file_id_async(struct hmdfs_sb_info *sbi, uint32_t id);

void hmdfs_adapter_convert_file_id_to_path(uint32_t id, char *buf);
int hmdfs_adapter_add_rename_task(struct hmdfs_sb_info *sbi,
				  struct path *lower_parent_path,
				  const char *newname, uint32_t id,
				  umode_t mode);

int hmdfs_persist_file_id(struct dentry *dentry, uint32_t id);
uint32_t hmdfs_read_file_id(struct inode *inode);

int hmdfs_adapter_file_id_generator_init(void);
void hmdfs_adapter_file_id_generator_cleanup(void);

#endif
