/*
 * power_genl.c
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
#include <chipset_common/hwpower/common_module/power_genl.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_genl
HWLOG_REGIST();

static struct power_genl_dev *g_power_genl_dev;
static LIST_HEAD(g_power_genl_list);

static struct genl_family g_power_genl_family = {
	.hdrsize = POWER_GENL_HDR_LEN,
	.name = POWER_GENL_NAME,
	.maxattr = POWER_GENL_MAX_ATTR_INDEX,
	.parallel_ops = 1,
	.n_ops = 0,
};

static int power_genl_get_port_id(unsigned int target)
{
	unsigned int port_id;
	struct power_genl_dev *l_dev = g_power_genl_dev;

	if (!l_dev)
		return 0;

	if (target >= POWER_GENL_TP_END) {
		hwlog_err("invalid target=%u\n", target);
		return 0;
	}

	port_id = l_dev->port_id[target];
	if (port_id == 0) {
		hwlog_err("target port_id not set\n");
		return 0;
	}

	return port_id;
}

static int power_genl_check_port_id(unsigned int target, unsigned int pid)
{
	struct power_genl_dev *l_dev = g_power_genl_dev;

	if (!l_dev)
		return -EINVAL;

	if (target >= POWER_GENL_TP_END) {
		hwlog_err("invalid target=%u\n", target);
		return -EINVAL;
	}

	return !(l_dev->port_id[target] == pid);
}

static int power_genl_check_attrs_length(const struct power_genl_attr *attrs,
	unsigned char attr_num)
{
	unsigned int len = 0;
	int i;

	for (i = 0; i < attr_num; i++)
		len += attrs[i].len;

	if (len > (NLMSG_GOODSIZE - POWER_GENL_MEM_MARGIN)) {
		hwlog_err("invalid attrs_len=%u\n", len);
		return -EINVAL;
	}

	hwlog_info("check_attrs_length: len=%u\n", len);
	return 0;
}

static int power_genl_send_msg(unsigned int port_id,
	unsigned char cmd, unsigned char version,
	struct power_genl_attr *attrs, unsigned char attr_num)
{
	int i;
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh = NULL;
	struct genlmsghdr *hdr = NULL;
	void *msg_head = NULL;

	/*
	 * allocate some memory,
	 * since the size is not yet known use NLMSG_GOODSIZE
	 */
	skb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!skb) {
		hwlog_err("genlmsg new failed\n");
		return POWER_GENL_EALLOC;
	}

	/*
	 * genlmsg_put is not used because diffent version of genl_family
	 * may used at same time
	 */
	nlh = nlmsg_put(skb, POWER_GENL_PORTID, POWER_GENL_SEQ,
		g_power_genl_family.id,
		GENL_HDRLEN + g_power_genl_family.hdrsize, POWER_GENL_FLAG);
	if (!nlh) {
		hwlog_err("genlmsg put failed\n");
		nlmsg_free(skb);
		return POWER_GENL_EPUTMESG;
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
			hwlog_err("genlmsg add attribute failed\n");
			nlmsg_free(skb);
			return POWER_GENL_EADDATTR;
		}
	}

	/* finalize the message */
	genlmsg_end(skb, msg_head);

	/* send the message */
	if (genlmsg_unicast(&init_net, skb, port_id)) {
		hwlog_err("genlmsg unicast failed\n");
		return POWER_GENL_EUNICAST;
	}

	return POWER_GENL_SUCCESS;
}

int power_genl_send(unsigned int target,
	unsigned char cmd, unsigned char version,
	struct power_genl_attr *attrs, unsigned char attr_num)
{
	unsigned int port_id;
	int ret;

	if (!attrs) {
		hwlog_err("attrs is null\n");
		return POWER_GENL_ENULL;
	}

	port_id = power_genl_get_port_id(target);
	if (port_id == 0)
		return POWER_GENL_EPORTID;

	if (power_genl_check_attrs_length(attrs, attr_num))
		return POWER_GENL_EMESGLEN;

	ret = power_genl_send_msg(port_id, cmd, version, attrs, attr_num);
	if (ret)
		return ret;

	hwlog_info("send: target=%u cmd=%u version=%u port_id=%x ok\n",
		target, cmd, version, port_id);
	return POWER_GENL_SUCCESS;
}

int power_genl_easy_send(unsigned int target,
	unsigned char cmd, unsigned char version,
	void *data, unsigned int len)
{
	struct power_genl_attr attr;

	attr.data = data;
	attr.len = len;
	attr.type = POWER_GENL_RAW_DATA_ATTR;

	return power_genl_send(target, cmd, version, &attr, 1);
}

int power_genl_node_register(struct power_genl_node *genl_node)
{
	struct power_genl_node *temp = NULL;
	struct power_genl_dev *l_dev = g_power_genl_dev;

	if (!genl_node)
		return POWER_GENL_ELATE;

	if (l_dev && (l_dev->probe_status == POWER_GENL_PROBE_START))
		return POWER_GENL_ELATE;

	if (strnlen(genl_node->name, POWER_GENL_NODE_NAME_LEN) >=
		POWER_GENL_NODE_NAME_LEN) {
		hwlog_err("invalid name=%s\n", genl_node->name);
		return POWER_GENL_ENAME;
	}

	list_for_each_entry(temp, &g_power_genl_list, node) {
		if (!strncmp(genl_node->name, temp->name,
			POWER_GENL_NODE_NAME_LEN)) {
			hwlog_err("node %s has registered\n", genl_node->name);
			return POWER_GENL_EREGED;
		}

		if (genl_node->target >= POWER_GENL_TP_END) {
			hwlog_err("invalid target=%u\n", genl_node->target);
			return POWER_GENL_ETARGET;
		}
	}

	INIT_LIST_HEAD(&genl_node->node);
	list_add(&genl_node->node, &g_power_genl_list);

	hwlog_info("name=%s targer=%d n_easy_ops=%u n_normal_ops=%u register ok\n",
		genl_node->name, genl_node->target,
		genl_node->n_easy_ops, genl_node->n_normal_ops);

	return POWER_GENL_SUCCESS;
}

int power_genl_easy_node_register(struct power_genl_node *genl_node)
{
	if (!genl_node)
		return POWER_GENL_EUNCHG;

	if (genl_node->normal_ops || genl_node->n_normal_ops) {
		hwlog_err("normal_ops is illegal with easy register\n");
		return POWER_GENL_EUNCHG;
	}

	if (!genl_node->easy_ops || (genl_node->n_easy_ops == 0)) {
		hwlog_err("invalid easy_ops\n");
		return POWER_GENL_EUNCHG;
	}

	return power_genl_node_register(genl_node);
}

int power_genl_normal_node_register(struct power_genl_node *genl_node)
{
	if (!genl_node)
		return POWER_GENL_EUNCHG;

	if (genl_node->easy_ops || genl_node->n_easy_ops) {
		hwlog_err("easy_ops is illegal with normal register\n");
		return POWER_GENL_EUNCHG;
	}

	if (!genl_node->normal_ops || (genl_node->n_normal_ops == 0)) {
		hwlog_err("invalid normal_ops\n");
		return POWER_GENL_EUNCHG;
	}

	return power_genl_node_register(genl_node);
}

static int power_genl_easy_doit(struct sk_buff *in, struct genl_info *info)
{
	struct power_genl_node *temp = NULL;
	struct nlattr *raw_nlattr = NULL;
	int len;
	void *data = NULL;
	int i;

	if (!info || !info->attrs) {
		hwlog_err("info or attrs is null\n");
		return -1;
	}

	raw_nlattr = info->attrs[POWER_GENL_RAW_DATA_ATTR];
	if (!raw_nlattr) {
		hwlog_err("raw_nlattr is null\n");
		return -1;
	}

	len = nla_len(raw_nlattr);
	data = nla_data(raw_nlattr);
	hwlog_info("easy_doit: snd_portid=%x cmd=%u len=%d\n",
		info->snd_portid, info->genlhdr->cmd, len);

	list_for_each_entry(temp, &g_power_genl_list, node) {
		if (power_genl_check_port_id(temp->target, info->snd_portid)) {
			hwlog_info("easy_doit: name=%s filted by checking port_id\n",
				temp->name);
			continue;
		}

		for (i = 0; i < temp->n_easy_ops; i++) {
			if (!temp->easy_ops)
				break;

			if (info->genlhdr->cmd != temp->easy_ops[i].cmd)
				continue;

			if (temp->easy_ops[i].doit) {
				hwlog_info("easy_doit: name=%s cmd=%u len=%d called\n",
					temp->name, info->genlhdr->cmd, len);
				temp->easy_ops[i].doit(info->genlhdr->version, data, len);
			} else {
				hwlog_info("easy_doit: name=%s cmd=%u doit is null\n",
					temp->name, info->genlhdr->cmd);
			}

			break;
		}
	}

	return 0;
}

static unsigned int power_genl_get_total_ops(void)
{
	struct power_genl_node *temp = NULL;
	unsigned int total_ops = 0;

	list_for_each_entry(temp, &g_power_genl_list, node)
		total_ops += temp->n_easy_ops + temp->n_normal_ops;

	if ((total_ops > POWER_GENL_MAX_OPS) ||
		(total_ops <= POWER_GENL_MIN_OPS)) {
		hwlog_err("invalid total_ops=%u,%d,%d\n",
			total_ops, POWER_GENL_MIN_OPS, POWER_GENL_MAX_OPS);
		return 0;
	}

	hwlog_info("get_total_ops: total=%u\n", total_ops);
	return total_ops;
}

static int power_genl_create_ops(const struct power_genl_dev *l_dev)
{
	struct power_genl_node *temp = NULL;
	struct genl_ops *ops = l_dev->ops_head;
	struct genl_ops *ops_o = l_dev->ops_head;
	struct genl_ops *ops_temp = NULL;
	int i;

	list_for_each_entry(temp, &g_power_genl_list, node) {
		for (i = 0; i < temp->n_easy_ops; i++) {
			for (ops_temp = ops_o; ops_temp < ops; ops_temp++) {
				if (temp->easy_ops[i].cmd == ops_temp->cmd) {
					hwlog_err("easy_ops: cmd=%u in %s has registered\n",
						ops->cmd, temp->name);
					return -EINVAL;
				}
			}

			hwlog_info("easy_ops: name=%s [%d]cmd=%u\n",
				temp->name, i, temp->easy_ops[i].cmd);
			ops->cmd = temp->easy_ops[i].cmd;
			ops->doit = power_genl_easy_doit;
			ops++;
		}

		for (i = 0; i < temp->n_normal_ops; i++) {
			for (ops_temp = ops_o; ops_temp < ops; ops_temp++) {
				if (temp->normal_ops[i].cmd == ops_temp->cmd) {
					hwlog_err("normal_ops: cmd=%u in %s has registered\n",
						ops->cmd, temp->name);
					return -EINVAL;
				}
			}

			hwlog_info("normal_ops: name=%s [%d]cmd=%u\n",
				temp->name, i, temp->normal_ops[i].cmd);
			ops->cmd = temp->normal_ops[i].cmd;
			ops->doit = temp->normal_ops[i].doit;
			ops++;
		}
	}

	return 0;
}

static int power_genl_register_family(const struct power_genl_dev *l_dev)
{
	g_power_genl_family.n_ops = l_dev->total_ops;
	g_power_genl_family.ops = l_dev->ops_head;

	if (genl_register_family(&g_power_genl_family)) {
		hwlog_err("family register failed\n");
		g_power_genl_family.n_ops = 0;
		g_power_genl_family.ops = NULL;
		return -EINVAL;
	}

	return 0;
}

static int power_genl_init_node(struct power_genl_dev *l_dev)
{
	struct genl_ops *ops_head = NULL;

	l_dev->probe_status = POWER_GENL_PROBE_START;

	/* step1: calculate the number of registered ops */
	l_dev->total_ops = power_genl_get_total_ops();
	if (l_dev->total_ops == 0)
		return POWER_GENL_EPROBE;

	/* step2: malloc memory for all ops */
	ops_head = kcalloc(l_dev->total_ops, sizeof(*ops_head), GFP_KERNEL);
	if (!ops_head)
		return POWER_GENL_EPROBE;

	l_dev->ops_head = ops_head;

	/* step3: create all genl ops */
	if (power_genl_create_ops(l_dev)) {
		kfree(ops_head);
		l_dev->ops_head = NULL;
		return POWER_GENL_EPROBE;
	}

	/* step4: register generic netlink family */
	if (power_genl_register_family(l_dev)) {
		kfree(ops_head);
		l_dev->ops_head = NULL;
		return POWER_GENL_EPROBE;
	}

	return POWER_GENL_SUCCESS;
}

#ifdef CONFIG_SYSFS
static void power_genl_callback_srv_on(unsigned int target)
{
	struct power_genl_node *temp = NULL;

	list_for_each_entry(temp, &g_power_genl_list, node) {
		if (temp->target != target)
			continue;

		if (temp->srv_on_cb && temp->srv_on_cb())
			hwlog_err("%s srv_on_cb failed\n", temp->name);
	}
}

static ssize_t power_genl_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t power_genl_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info power_genl_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(power_genl, 0660, POWER_GENL_SYSFS_POWERCT, powerct),
	power_sysfs_attr_rw(power_genl, 0660, POWER_GENL_SYSFS_AF, af),
	power_sysfs_attr_rw(power_genl, 0660, POWER_GENL_SYSFS_PROT, prot),
};

#define POWER_GENL_SYSFS_ATTRS_SIZE  ARRAY_SIZE(power_genl_sysfs_field_tbl)

static struct attribute *power_genl_sysfs_attrs[POWER_GENL_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group power_genl_sysfs_attr_group = {
	.attrs = power_genl_sysfs_attrs,
};

static ssize_t power_genl_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct power_genl_dev *l_dev = g_power_genl_dev;
	unsigned int target;

	if (!l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		power_genl_sysfs_field_tbl, POWER_GENL_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case POWER_GENL_SYSFS_POWERCT:
		target = POWER_GENL_TP_POWERCT;
		break;
	case POWER_GENL_SYSFS_AF:
		target = POWER_GENL_TP_AF;
		break;
	case POWER_GENL_SYSFS_PROT:
		target = POWER_GENL_TP_PROT;
		break;
	default:
		return 0;
	}

	return snprintf(buf, PAGE_SIZE, "%u", l_dev->port_id[target]);
}

static ssize_t power_genl_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct power_genl_dev *l_dev = g_power_genl_dev;
	unsigned int target;

	if (!l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		power_genl_sysfs_field_tbl, POWER_GENL_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	if (count != sizeof(unsigned int)) {
		hwlog_err("invalid length=%zu\n", count);
		return -EINVAL;
	}

	switch (info->name) {
	case POWER_GENL_SYSFS_POWERCT:
		target = POWER_GENL_TP_POWERCT;
		break;
	case POWER_GENL_SYSFS_AF:
		target = POWER_GENL_TP_AF;
		break;
	case POWER_GENL_SYSFS_PROT:
		target = POWER_GENL_TP_PROT;
		break;
	default:
		return count;
	}

	memcpy(&l_dev->port_id[target], buf, count);
	power_genl_callback_srv_on(target);

	hwlog_info("set: target=%d pid=%x\n", target, l_dev->port_id[target]);
	return count;
}

static struct device *power_genl_sysfs_create_group(void)
{
	power_sysfs_init_attrs(power_genl_sysfs_attrs,
		power_genl_sysfs_field_tbl, POWER_GENL_SYSFS_ATTRS_SIZE);
	return power_sysfs_create_group("hw_power", "power_mesg",
		&power_genl_sysfs_attr_group);
}

static void power_genl_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &power_genl_sysfs_attr_group);
}
#else
static inline struct device *power_genl_sysfs_create_group(void)
{
	return NULL;
}

static inline void power_genl_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int __init power_genl_init(void)
{
	struct power_genl_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_power_genl_dev = l_dev;
	l_dev->dev = power_genl_sysfs_create_group();

	power_genl_init_node(l_dev);
	return 0;
}

static void __exit power_genl_exit(void)
{
	struct power_genl_dev *l_dev = g_power_genl_dev;

	if (!l_dev)
		return;

	power_genl_sysfs_remove_group(l_dev->dev);
	kfree(l_dev->ops_head);
	kfree(l_dev);
	g_power_genl_dev = NULL;
}

late_initcall(power_genl_init);
module_exit(power_genl_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("netlink for power module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
