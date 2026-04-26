/*
 * power_genl.h
 *
 * netlink message for power module
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _POWER_GENL_H_
#define _POWER_GENL_H_

#include <linux/list.h>
#include <net/genetlink.h>

#define POWER_GENL_NODE_NAME_LEN            16

/* define macro for generic netlink */
#define POWER_GENL_SEQ                      0
#define POWER_GENL_PORTID                   0
#define POWER_GENL_FLAG                     0
#define POWER_GENL_HDR_LEN                  0
#define POWER_GENL_MEM_MARGIN               200
#define POWER_GENL_NAME                     "POWER_GENL"

#define POWER_GENL_PROBE_UNREADY            0
#define POWER_GENL_PROBE_START              1

#define POWER_GENL_MIN_OPS                  0
#define POWER_GENL_MAX_OPS                  255

enum power_genl_attr_type {
	POWER_GENL_ATTR_BEGIN = 0,
	POWER_GENL_ATTR_UNUSED = POWER_GENL_ATTR_BEGIN,
	POWER_GENL_ATTR0,
	POWER_GENL_ATTR1,
	POWER_GENL_ATTR_END,
};

#define POWER_GENL_TOTAL_ATTR               (POWER_GENL_ATTR_END - 1)
#define POWER_GENL_BATT_INFO_DEV_ATTR       POWER_GENL_ATTR0
#define POWER_GENL_BATT_INFO_DAT_ATTR       POWER_GENL_ATTR1
#define POWER_GENL_RAW_DATA_ATTR            POWER_GENL_ATTR1
#define POWER_GENL_MAX_ATTR_INDEX           (POWER_GENL_ATTR_END - 1)

enum power_genl_cmd_num {
	/* 00-49 is for battery */
	POWER_GENL_CMD_BATT_INFO = 0,
	POWER_GENL_CMD_BATT_BIND_RD = 1,
	POWER_GENL_CMD_BATT_BIND_WR = 2,
	POWER_GENL_CMD_BATT_DMD = 10,
	POWER_GENL_CMD_BOARD_INFO = 40,
	POWER_GENL_CMD_BATT_FINAL_RESULT = 49,
	/* 50 is for direct charging */
	POWER_GENL_CMD_DC_AUTH_HASH = 50,
	/* 51 is for wireless rx charging */
	POWER_GENL_CMD_WLRX_AUTH_HASH = 51,
	/* 52 is for wireless tx charging */
	POWER_GENL_CMD_WLTX_AUTH_HASH = 52,
	/* 53 is for uvdm charging */
	POWER_GENL_CMD_UVDM_AUTH_HASH = 53,
	/* max */
	POWER_GENL_CMD_TOTAL_NUM = 256,
};

/*
 * define error code with power genl
 * genl is simplified identifier with general netlink
 */
enum power_genl_error_code {
	POWER_GENL_SUCCESS = 0,
	POWER_GENL_EUNCHG,
	POWER_GENL_ETARGET,
	POWER_GENL_ENULL,
	POWER_GENL_ENEEDLESS,
	POWER_GENL_EUNREGED,
	POWER_GENL_EMESGLEN,
	POWER_GENL_EALLOC,
	POWER_GENL_EPUTMESG,
	POWER_GENL_EUNICAST,
	POWER_GENL_EADDATTR,
	POWER_GENL_EPORTID,
	POWER_GENL_EPROBE,
	POWER_GENL_ELATE,
	POWER_GENL_EREGED,
	POWER_GENL_ECMD,
	POWER_GENL_ENAME,
};

enum power_genl_target_port {
	POWER_GENL_TP_BEGIN = 0,
	POWER_GENL_TP_POWERCT = POWER_GENL_TP_BEGIN,
	POWER_GENL_TP_AF,
	POWER_GENL_TP_PROT,
	POWER_GENL_TP_END,
};

enum power_genl_sysfs_type {
	POWER_GENL_SYSFS_BEGIN = 0,
	POWER_GENL_SYSFS_POWERCT = POWER_GENL_SYSFS_BEGIN,
	POWER_GENL_SYSFS_AF,
	POWER_GENL_SYSFS_PROT,
	POWER_GENL_SYSFS_END,
};

struct power_genl_attr {
	const unsigned char *data;
	unsigned int len;
	int type;
};

struct power_genl_easy_ops {
	unsigned char cmd;
	int (*doit)(unsigned char, void *, int);
};

struct power_genl_normal_ops {
	unsigned char cmd;
	int (*doit)(struct sk_buff *, struct genl_info *);
};

struct power_genl_node {
	struct list_head node;
	const unsigned int target;
	const char name[POWER_GENL_NODE_NAME_LEN];
	const struct power_genl_easy_ops *easy_ops;
	const unsigned char n_easy_ops; /* number of easy ops */
	const struct power_genl_normal_ops *normal_ops;
	const unsigned char n_normal_ops; /* number of normal ops */
	int (*srv_on_cb)(void);
};

struct power_genl_dev {
	struct device *dev;
	struct genl_ops *ops_head;
	unsigned int total_ops;
	int probe_status;
	unsigned int port_id[POWER_GENL_TP_END];
};

int power_genl_send(unsigned int target,
	unsigned char cmd, unsigned char version,
	struct power_genl_attr *attrs, unsigned char attr_num);
int power_genl_easy_send(unsigned int target,
	unsigned char cmd, unsigned char version,
	void *data, unsigned int len);

int power_genl_node_register(struct power_genl_node *genl_node);
int power_genl_easy_node_register(struct power_genl_node *genl_node);
int power_genl_normal_node_register(struct power_genl_node *genl_node);

#endif /* _POWER_GENL_H_ */
