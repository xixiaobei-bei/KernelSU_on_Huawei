/* SPDX-License-Identifier: GPL-2.0 */
/*
 * device_node.h
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Description: send and recv msg for hmdfs
 * Author: liuxuesong3@huawei.com
 * Create: 2020-04-14
 *
 */

#ifndef HMDFS_DEVICE_NODE_H
#define HMDFS_DEVICE_NODE_H

#include "hmdfs.h"
#include "transport.h"

enum CTRL_NODE_CMD {
	CMD_INIT = 0,
	CMD_UPDATE_SOCKET,
	CMD_OFF_LINE,
	CMD_SET_ACCOUNT,
	CMD_OFF_LINE_ALL,
	CMD_CNT,
};

struct init_param {
	int32_t cmd;
	uint64_t local_iid;
	uint8_t current_account[HMDFS_ACCOUNT_HASH_MAX_LEN];
} __packed;

struct update_socket_param {
	int32_t cmd;
	int32_t newfd;
	uint64_t local_iid;
	uint8_t status;
	uint8_t masterkey[HMDFS_KEY_SIZE];
	uint8_t cid[HMDFS_CID_SIZE];
} __packed;

struct offline_param {
	int32_t cmd;
	uint8_t remote_cid[HMDFS_CID_SIZE];
} __packed;

struct offline_all_param {
	int32_t cmd;
} __packed;

struct set_account_param {
	int32_t cmd;
	uint8_t current_account[HMDFS_ACCOUNT_HASH_MAX_LEN];
} __packed;

enum NOTIFY {
	NOTIFY_HS_DONE = 0,
	NOTIFY_OFFLINE,
	NOTIFY_OFFLINE_IID,
	NOTIFY_GET_SESSION,
	NOTIFY_NONE,
	NOTIFY_CNT,
};

struct notify_param {
	int32_t notify;
	int32_t fd;
	uint64_t remote_iid;
	uint8_t remote_cid[HMDFS_CID_SIZE];
} __packed;

struct sbi_attribute {
	struct attribute attr;
	ssize_t (*show)(struct kobject *kobj, struct sbi_attribute *attr,
			char *buf);
	ssize_t (*store)(struct kobject *kobj, struct sbi_attribute *attr,
			 const char *buf, size_t len);
};

struct peer_attribute {
	struct attribute attr;
	ssize_t (*show)(struct kobject *kobj, struct peer_attribute *attr,
			char *buf);
	ssize_t (*store)(struct kobject *kobj, struct peer_attribute *attr,
			 const char *buf, size_t len);
};

struct sbi_cmd_attribute {
	struct attribute attr;
	int command;
};

void notify(struct hmdfs_peer *node, struct notify_param *param);
int hmdfs_register_sysfs(const char *name, struct hmdfs_sb_info *sbi);
void hmdfs_unregister_sysfs(struct hmdfs_sb_info *sbi);
void hmdfs_release_sysfs(struct hmdfs_sb_info *sbi);
int hmdfs_register_peer_sysfs(struct hmdfs_sb_info *sbi,
			      struct hmdfs_peer *peer);
void hmdfs_release_peer_sysfs(struct hmdfs_peer *peer);
int hmdfs_sysfs_init(void);
void hmdfs_sysfs_exit(void);

static inline struct sbi_attribute *to_sbi_attr(struct attribute *x)
{
	return container_of(x, struct sbi_attribute, attr);
}

static inline struct hmdfs_sb_info *to_sbi(struct kobject *x)
{
	return container_of(x, struct hmdfs_sb_info, kobj);
}

static inline struct peer_attribute *to_peer_attr(struct attribute *x)
{
	return container_of(x, struct peer_attribute, attr);
}

static inline struct hmdfs_peer *to_peer(struct kobject *x)
{
	return container_of(x, struct hmdfs_peer, kobj);
}
#endif
