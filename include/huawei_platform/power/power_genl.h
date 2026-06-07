/*
 * power_genl.h
 *
 * power netlink message head file
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

/* define macro for generic netlink */
#define POWER_GENL_SEQ                      0
#define POWER_GENL_PORTID                   0
#define POWER_GENL_FLAG                     0
#define POWER_USER_HDR_LEN                  0
#define POWER_GENL_MEM_MARGIN               200
#define POWER_GENL_NAME                     "POWER_GENL"

#define POWER_GENL_PROBE_UNREADY            0
#define POWER_GENL_PROBE_START              1

#define POWER_GENL_MIN_OPS                  0
#define POWER_GENL_MAX_OPS                  255

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
};

struct power_genl_target {
	unsigned int port_id;
	unsigned int probed;
	const struct device_attribute dev_attr;
};

int power_genl_send_attrs(struct power_mesg_node *genl_node,
	unsigned char cmd, unsigned char version,
	struct batt_res *attrs, unsigned char attr_num);
int power_genl_easy_send(struct power_mesg_node *genl_node,
	unsigned char cmd, unsigned char version, void *data, unsigned int len);

int power_genl_node_register(struct power_mesg_node *genl_node);
int power_genl_easy_node_register(struct power_mesg_node *genl_node);
int power_genl_normal_node_register(struct power_mesg_node *genl_node);
int power_genl_init(void);

#endif /* _POWER_GENL_H_ */
