/* SPDX-License-Identifier: GPL-2.0 */
/*
 * fault_inject.c
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 * Author: yuyufen@huawei.com
 * Create: 2020-12-17
 *
 */

#ifndef HMDFS_FAULT_INJECT_H
#define HMDFS_FAULT_INJECT_H

#include <linux/fault-inject.h>
#include "protocol.h"

struct hmdfs_fault_inject {
#ifdef CONFIG_FAULT_INJECTION_DEBUG_FS
	struct fault_attr attr;
	struct dentry *parent;
	unsigned long op_mask;
	unsigned long fail_send_message;
	unsigned long fake_fid_ver;
	bool fail_req;
#endif
};

enum FAIL_MESSAGE_TYPE {
	T_MSG_FAIL = 1,
	T_MSG_DISCARD = 2,
};

enum CHANGE_FID_VER_TYPE {
	T_BOOT_COOKIE = 1,
	T_CON_COOKIE = 2,
};

#ifdef CONFIG_FAULT_INJECTION_DEBUG_FS
void __init hmdfs_create_debugfs_root(void);
void hmdfs_destroy_debugfs_root(void);

void hmdfs_fault_inject_init(struct hmdfs_fault_inject *fault_inject,
			     const char *name);
void hmdfs_fault_inject_fini(struct hmdfs_fault_inject *fault_inject);
bool hmdfs_should_fail_sendmsg(struct hmdfs_fault_inject *fault_inject,
			       struct hmdfs_peer *con,
			       struct hmdfs_send_data *msg, int *err);
bool hmdfs_should_fail_req(struct hmdfs_fault_inject *fault_inject,
			   struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			   int *err);
bool hmdfs_should_fake_fid_ver(struct hmdfs_fault_inject *fault_inject,
			       struct hmdfs_peer *con,
			       struct hmdfs_head_cmd *cmd,
			       enum CHANGE_FID_VER_TYPE fake_type);
#else
static inline void __init hmdfs_create_debugfs_root(void) {}
static inline void hmdfs_destroy_debugfs_root(void) {}

static inline void
hmdfs_fault_inject_init(struct hmdfs_fault_inject *fault_inject,
			const char *name)
{
}
static inline void
hmdfs_fault_inject_fini(struct hmdfs_fault_inject *fault_inject)
{
}
static inline bool
hmdfs_should_fail_sendmsg(struct hmdfs_fault_inject *fault_inject,
			  struct hmdfs_peer *con, struct hmdfs_send_data *msg,
			  int *err)
{
	return false;
}
static inline bool
hmdfs_should_fail_req(struct hmdfs_fault_inject *fault_inject,
		      struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
		      int *err)
{
	return false;
}
static inline bool
hmdfs_should_fake_fid_ver(struct hmdfs_fault_inject *fault_inject,
			  struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			  enum CHANGE_FID_VER_TYPE fake_type)
{
	return false;
}
#endif

#endif // HMDFS_FAULT_INJECT_H
