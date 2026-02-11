/*
 * power_genl.c
 *
 * power netlink message source file
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

#include <linux/list.h>
#include <net/genetlink.h>
#include <net/netlink.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spinlock_types.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/power_mesg.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG power_genl
HWLOG_REGIST();

static LIST_HEAD(power_genl_node_head);

static int probe_status = POWER_GENL_PROBE_UNREADY;

static struct genl_family power_genl = {
	.hdrsize = POWER_USER_HDR_LEN,
	.name = POWER_GENL_NAME,
	.maxattr = POWER_GENL_MAX_ATTR_INDEX,
	.parallel_ops = 1,
	.n_ops = 0,
};

static ssize_t powerct_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_genl_target *target = NULL;

	target = container_of(attr, struct power_genl_target, dev_attr);

	return snprintf(buf, PAGE_SIZE, "%u", target->port_id);
}

static ssize_t powerct_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_genl_target *target = NULL;
	struct power_mesg_node *temp = NULL;

	target = container_of(attr, struct power_genl_target, dev_attr);

	if (count != sizeof(unsigned int)) {
		hwlog_err("invalid length=%zu\n", count);
		return -EINVAL;
	}

	memcpy(&target->port_id, buf, count);
	list_for_each_entry(temp, &power_genl_node_head, node) {
		if (temp->srv_on_cb && temp->srv_on_cb())
			hwlog_err("%s srv_on_cb failed\n", temp->name);
	}

	return count;
}

static struct power_genl_target nl_target[TARGET_PORT_END] = {
	[POWERCT_PORT] = {
		.port_id = 0,
		.probed = 0,
		.dev_attr = __ATTR_RW(powerct),
	},
};

int power_genl_send_attrs(struct power_mesg_node *genl_node,
	unsigned char cmd, unsigned char version,
	struct batt_res *attrs, unsigned char attr_num)
{
	struct power_mesg_node *temp = NULL;
	int ret_val;
	struct sk_buff *skb = NULL;
	void *msg_head = NULL;
	int i;
	unsigned int len;
	struct nlmsghdr *nlh = NULL;
	struct genlmsghdr *hdr = NULL;

	if (!attrs) {
		hwlog_err("cmd=%d resoure is null\n", cmd);
		return POWER_GENL_ENULL;
	}

	list_for_each_entry(temp, &power_genl_node_head, node) {
		if (temp == genl_node)
			break;
	}

	if (&temp->node == &power_genl_node_head) {
		hwlog_err("%s genl_node unregistered\n", temp->name);
		return POWER_GENL_EUNREGED;
	}

	if ((temp->target > TARGET_PORT_MAX) ||
		(nl_target[temp->target].port_id == 0)) {
		hwlog_err("target port id has not set\n");
		return POWER_GENL_EPORTID;
	}

	len = 0;
	for (i = 0; i < attr_num; i++)
		len = attrs[i].len;

	if (len > NLMSG_GOODSIZE - POWER_GENL_MEM_MARGIN)
		return POWER_GENL_EMESGLEN;

	/*
	 * allocate some memory,
	 * since the size is not yet known use NLMSG_GOODSIZE
	 */
	skb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!skb) {
		hwlog_err("new general message failed\n");
		return POWER_GENL_EALLOC;
	}

	/*
	 * genlmsg_put is not used because diffent version of genl_family
	 * may used at same time
	 */
	nlh = nlmsg_put(skb, POWER_GENL_PORTID, POWER_GENL_SEQ, power_genl.id,
		GENL_HDRLEN + power_genl.hdrsize, POWER_GENL_FLAG);
	if (!nlh) {
		hwlog_err("create message for genlmsg failed\n");
		ret_val = POWER_GENL_EPUTMESG;
		goto nosend;
	}

	/* fill genl head */
	hdr = nlmsg_data(nlh);
	hdr->cmd = cmd;
	hdr->version = version;
	hdr->reserved = 0;

	/* get message head */
	msg_head = (char *)hdr + GENL_HDRLEN;

	/* fill attributes */
	for (i = 0; i < attr_num; i++) {
		/* add a BATT_RAW_MESSAGE attribute (actual value to be sent) */
		if (nla_put(skb, attrs[i].type, attrs[i].len, attrs[i].data)) {
			hwlog_err("add attribute to genlmsg failed\n");
			ret_val = POWER_GENL_EADDATTR;
			goto nosend;
		}
	}

	/* finalize the message */
	genlmsg_end(skb, msg_head);

	/* send the message back */
	if (temp->target > TARGET_PORT_MAX ||
		genlmsg_unicast(&init_net, skb,
		nl_target[temp->target].port_id)) {
		hwlog_err("unicast genlmsg failed\n");
		return POWER_GENL_EUNICAST;
	}

	hwlog_info("%s cmd %d data was sent\n", temp->name, cmd);
	return POWER_GENL_SUCCESS;

nosend:
	kfree_skb(skb);
	return ret_val;
}

int power_genl_easy_send(struct power_mesg_node *genl_node,
	unsigned char cmd, unsigned char version, void *data, unsigned int len)
{
	struct batt_res attr;

	attr.data = data;
	attr.len = len;
	attr.type = POWER_GENL_RAW_DATA_ATTR;

	return power_genl_send_attrs(genl_node, cmd, version, &attr, 1);
}

int power_genl_check_port_id(unsigned int type, unsigned int pid)
{
	if (type > TARGET_PORT_MAX) {
		hwlog_err("invalid port_id=%d\n", type);
		return -1;
	}

	return !(nl_target[type].port_id == pid);
}

static int power_genl_node_mesg_cb(struct sk_buff *skb_in,
	struct genl_info *info)
{
	int i;
	int len;
	struct nlattr *raw_data_attr = NULL;
	void *data = NULL;
	struct power_mesg_node *temp = NULL;

	if (!info) {
		hwlog_err("info is null\n");
		return -1;
	}

	if (!info->attrs) {
		hwlog_err("info attrs is null\n");
		return -1;
	}

	raw_data_attr = info->attrs[POWER_GENL_RAW_DATA_ATTR];
	if (!raw_data_attr) {
		hwlog_err("raw_data_attr is null\n");
		return -1;
	}

	len = nla_len(raw_data_attr);
	data = nla_data(raw_data_attr);

	list_for_each_entry(temp, &power_genl_node_head, node) {
		for (i = 0; i < temp->n_ops; i++) {
			if (!temp->ops) {
				hwlog_err("ops is null\n");
				break;
			}

			if (info->genlhdr->cmd != temp->ops[i].cmd)
				continue;

			if (power_genl_check_port_id(POWERCT_PORT,
				info->snd_portid)) {
				hwlog_err("%s cmd %d filted by checking port id failed\n",
					temp->name, info->genlhdr->cmd);
				return POWER_GENL_EPORTID;
			}

			if (temp->ops[i].doit) {
				hwlog_info("%s cmd %d called\n",
					temp->name, info->genlhdr->cmd);
				temp->ops[i].doit(info->genlhdr->version,
					data, len);
			} else {
				hwlog_info("%s cmd %d doit is null\n",
					temp->name, info->genlhdr->cmd);
			}

			break;
		}
	}

	return 0;
}

int power_genl_node_register(struct power_mesg_node *genl_node)
{
	struct power_mesg_node *temp = NULL;

	if (probe_status == POWER_GENL_PROBE_START)
		return POWER_GENL_ELATE;

	if (strnlen(genl_node->name, POWER_NODE_NAME_MAX_LEN) >=
		POWER_NODE_NAME_MAX_LEN)
		hwlog_err("invalid length name=%s\n", genl_node->name);

	list_for_each_entry(temp, &power_genl_node_head, node) {
		if (!strncmp(genl_node->name, temp->name,
			POWER_NODE_NAME_MAX_LEN)) {
			hwlog_err("name=%s has registered\n", genl_node->name);
			return POWER_GENL_EREGED;
		}

		if (genl_node->target > TARGET_PORT_MAX) {
			hwlog_err("invalid port_id=%d\n", genl_node->target);
			return POWER_GENL_ETARGET;
		}
	}

	INIT_LIST_HEAD(&genl_node->node);
	list_add(&genl_node->node, &power_genl_node_head);

	hwlog_info("name=%s targer_port=%d register ok\n",
		genl_node->name, genl_node->target);

	return POWER_GENL_SUCCESS;
}

int power_genl_easy_node_register(struct power_mesg_node *genl_node)
{
	if (genl_node->cbs || genl_node->n_cbs) {
		hwlog_err("easy genl_node cbs illegal\n");
		return POWER_GENL_EUNCHG;
	}

	if (!genl_node->ops || genl_node->n_ops == 0) {
		hwlog_err("easy genl_node ops illegal\n");
		return POWER_GENL_EUNCHG;
	}

	return power_genl_node_register(genl_node);
}

int power_genl_normal_node_register(struct power_mesg_node *genl_node)
{
	if (genl_node->ops || genl_node->n_ops) {
		hwlog_err("normal genl_node ops illegal\n");
		return POWER_GENL_EUNCHG;
	}

	if (!genl_node->cbs || genl_node->n_cbs == 0) {
		hwlog_err("normal genl_node cbs illegal\n");
		return POWER_GENL_EUNCHG;
	}

	return power_genl_node_register(genl_node);
}

int power_genl_init(void)
{
	struct genl_ops *ops = NULL;
	struct genl_ops *ops_o = NULL;
	struct genl_ops *ops_temp = NULL;
	struct power_mesg_node *temp = NULL;
	int i;
	unsigned int total_ops;

	probe_status = POWER_GENL_PROBE_START;

	/* first: calculate the number of registered ops */
	total_ops = 0;
	list_for_each_entry(temp, &power_genl_node_head, node)
		total_ops += temp->n_ops + temp->n_cbs;

	if ((total_ops > POWER_GENL_MAX_OPS) ||
		(total_ops <= POWER_GENL_MIN_OPS)) {
		hwlog_err("invalid genl ops=%d,%d,%d\n",
			total_ops, POWER_GENL_MIN_OPS, POWER_GENL_MAX_OPS);
		return POWER_GENL_EPROBE;
	}

	/* save ops number */
	power_genl.n_ops = total_ops;

	/* second: malloc memory for all ops */
	ops = kcalloc(total_ops, sizeof(*ops), GFP_KERNEL);
	if (!ops)
		return POWER_GENL_EPROBE;

	ops_o = ops;
	list_for_each_entry(temp, &power_genl_node_head, node) {
		for (i = 0; i < temp->n_ops; i++) {
			for (ops_temp = ops_o; ops_temp < ops; ops_temp++) {
				if (temp->ops[i].cmd == ops_temp->cmd) {
					hwlog_err("cmd=%d ops in %s has registered\n",
						ops->cmd, temp->name);
					kfree(ops_o);
					return POWER_GENL_EPROBE;
				}
			}

			ops->cmd = temp->ops[i].cmd;
			ops->doit = power_genl_node_mesg_cb;
			ops++;
		}

		for (i = 0; i < temp->n_cbs; i++) {
			for (ops_temp = ops_o; ops_temp < ops; ops_temp++) {
				if (temp->cbs[i].cmd == ops_temp->cmd) {
					hwlog_err("cmd=%d cbs in %s has registered\n",
						ops->cmd, temp->name);
					kfree(ops_o);
					return POWER_GENL_EPROBE;
				}
			}

			ops->cmd = temp->cbs[i].cmd;
			ops->doit = temp->cbs[i].doit;
			ops++;
		}
	}

	/* save ops pointer */
	power_genl.ops = ops_o;

	/* third: register generic netlink */
	if (genl_register_family(&power_genl)) {
		hwlog_err("genl register failed\n");
		kfree(ops_o);
		power_genl.ops = NULL;
		return POWER_GENL_EPROBE;
	}

	list_for_each_entry(temp, &power_genl_node_head, node) {
		if ((temp->target > TARGET_PORT_MAX) ||
			nl_target[temp->target].probed)
			continue;

		if (power_mesg_create_attr(&nl_target[temp->target].dev_attr))
			hwlog_err("attribute %s created failed\n",
				nl_target[temp->target].dev_attr.attr.name);

		nl_target[temp->target].probed = 1;
	}

	return POWER_GENL_SUCCESS;
}
