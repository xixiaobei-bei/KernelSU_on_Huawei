/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: This module is to manage network slice route function
 * Author: yuxiaoliang@huawei.com
 * Create: 2019-11-20
 */

#include "network_slice_route.h"
#include <linux/byteorder/generic.h>
#include <linux/sock_diag.h>
#include <net/sock.h>
#include <log/hw_log.h>
#include <securec.h>

#define SLICE_SK_STRATEGY_CT (1 << 0)
#define SLICE_SK_STRATEGY_FUTE (1 << 1)
#define SLICE_SK_STRATEGY_FURE (1 << 2)
#define skip_byte(p, num) ((p) = (p) + (num))
#define FIRST_APPLICATION_UID 10000
#define IPV4_ADDR_LEN 4
#define IPV4_ADDR_MASK_LEN 8
#define IPV6_ADDR_LEN 16
#define DEL_BIND_ALL 0
#define DEL_BIND_NETID 1
#define DEL_BIND_PRECEDENCE 2

#undef HWLOG_TAG
#define HWLOG_TAG slice_route
HWLOG_REGIST();

struct list_head g_slice_route_map;

static int close_socket(struct sock *sk, u8 protocol)
{
	int err = 0;
	if (protocol == IPPROTO_TCP) {
		if (IS_ERR(sk))
			return PTR_ERR(sk);

		err = sock_diag_destroy(sk, ECONNABORTED);

		sock_gen_put(sk);
	} else if (protocol == IPPROTO_UDP ||
		protocol == IPPROTO_UDPLITE) {
		if (sk && !refcount_inc_not_zero(&sk->sk_refcnt))
			sk = NULL;

		if (!sk)
			return -ENOENT;

		err = sock_diag_destroy(sk, ECONNABORTED);
		sock_put(sk);
	}
	return err;
}

static int bind_socket_to_slice_net(struct slice_route_rule *rule,
	struct sock *sk, u8 protocol)
{
	int err = -EINVAL;
	if (rule == NULL || sk == NULL)
		return err;

	if (time_before(sk->sk_born_stamp, rule->stamp)) {
		hwlog_info("%s: sk->sk_born_stamp = %u, rule->stamp= %u\n",
			__func__, sk->sk_born_stamp, rule->stamp);
		close_socket(sk, protocol);
		return 0;
	}

	sk->sk_mark = rule->net_id;
	sk->sk_slice_binded = 1;
	hwlog_info("%s: set sk_slice_binded to 1\n", __func__);

	return 0;
}

static bool is_rule_in_list_by_net_id(u32 net_id)
{
	struct slice_route_rule *rule = NULL;

	rcu_read_lock();
	list_for_each_entry_rcu(rule, &g_slice_route_map, list) {
		if (rule->net_id == net_id) {
			rcu_read_unlock();
			return true;
		}
	}
	rcu_read_unlock();

	return false;
}

static bool is_rule_in_list(u32 precedence)
{
	struct slice_route_rule *rule = NULL;

	rcu_read_lock();
	list_for_each_entry_rcu(rule, &g_slice_route_map, list) {
		if (rule->priority == precedence) {
			rcu_read_unlock();
			return true;
		}
	}
	rcu_read_unlock();

	return false;
}

static bool is_port_match_rule(struct slice_route_rule *rule, __be16 port)
{
	u32 i;
	if (rule == NULL)
		return false;

	if (rule->port_num == 0)
		return true;

	for (i = 0; i < rule->port_num; i++) {
		if (port >= rule->port[i].start && port <= rule->port[i].end) {
			hwlog_info("%s: port = %d\n", __func__, port);
			return true;
		}
	}

	return false;
}

static bool is_uid_in_rule(struct slice_route_rule *rule, u32 uid)
{
	struct uid_list *curr = NULL;
	if (rule == NULL)
		return false;

	rcu_read_lock();
	list_for_each_entry_rcu(curr, &rule->uid_list, list) {
		if (curr->uid == uid) {
			rcu_read_unlock();
			hwlog_info("%s: uid = %d\n", __func__, uid);
			return true;
		}
	}
	rcu_read_unlock();

	return false;
}

static bool is_uid_match_rule(struct slice_route_rule *rule, u32 uid)
{
	if (rule == NULL)
		return false;

	if (list_empty(&rule->uid_list))
		return true;

	return is_uid_in_rule(rule, uid);
}

static bool is_addrs_match(
	struct remote_address *addr, struct remote_address *addr_in_rule)
{
	if (addr == NULL || addr_in_rule == NULL)
		return false;

	if (addr->ai_family != addr_in_rule->ai_family)
		return false;

	if (addr->ai_family == AF_INET) {
		if (addr_in_rule->addr_u.ip4_addr.mask <= 0) {
			return (addr->addr_u.ip4_addr.addr ==
				addr_in_rule->addr_u.ip4_addr.addr);
		}
		if ((addr->addr_u.ip4_addr.addr ^
			addr_in_rule->addr_u.ip4_addr.addr) &
			addr_in_rule->addr_u.ip4_addr.mask)
			return false;
		else
			return true;
	} else if (addr->ai_family == AF_INET6) {
		if (addr_in_rule->addr_u.ip6_addr.prefix_len == 0) {
			return ipv6_addr_equal(&addr->addr_u.ip6_addr.addr,
				&addr_in_rule->addr_u.ip6_addr.addr);
		} else {
			return ipv6_prefix_equal(&addr->addr_u.ip6_addr.addr,
				&addr_in_rule->addr_u.ip6_addr.addr,
				addr_in_rule->addr_u.ip6_addr.prefix_len);
		}
	}

	return false;
}

static bool is_addr_in_rule(
	struct slice_route_rule *rule, struct remote_address *addr)
{
	struct remote_address *curr_addr = NULL;
	if (rule == NULL || addr == NULL)
		return false;

	rcu_read_lock();
	list_for_each_entry_rcu(curr_addr, &rule->addr_list, list) {
		if (is_addrs_match(addr, curr_addr)) {
			rcu_read_unlock();
			hwlog_info("%s\n", __func__);
			return true;
		}
	}
	rcu_read_unlock();

	return false;
}

static bool is_addr_match_rule(
	struct slice_route_rule *rule, struct remote_address *addr)
{
	if (rule == NULL || addr == NULL)
		return false;

	if (list_empty(&rule->addr_list))
		return true;

	return is_addr_in_rule(rule, addr);
}

static bool is_protocol_match_rule(struct slice_route_rule *rule, u8 protocol)
{
	u32 i;

	if (rule == NULL)
		return false;

	if (rule->protocol_num == 0)
		return true;

	for (i = 0; i < rule->protocol_num; i++) {
		if (protocol == rule->protocol[i]) {
			hwlog_info("%s: protocol = %d\n", __func__, protocol);
			return true;
		}
	}

	return false;
}

static bool slice_rule_match(u32 uid, struct sockaddr *uaddr, u8 protocol,
	struct slice_route_rule *rule)
{
	struct remote_address address;
	struct sockaddr_in6 *usin6 = (struct sockaddr_in6 *)uaddr;
	struct sockaddr_in *usin4 = (struct sockaddr_in *)uaddr;
	__be16 port = 0;

	if (uaddr == NULL || rule == NULL)
		return false;

	if (!is_protocol_match_rule(rule, protocol))
		return false;

	if (!is_uid_match_rule(rule, uid))
		return false;

	memset(&address, 0, sizeof(address));
	if (uaddr->sa_family == AF_INET) {
		address.ai_family = AF_INET;
		address.addr_u.ip4_addr.addr = usin4->sin_addr.s_addr;
		port = usin4->sin_port;
	} else if (uaddr->sa_family == AF_INET6) {
		address.ai_family = AF_INET6;
		memcpy(address.addr_u.ip6_addr.addr.s6_addr,
			usin6->sin6_addr.s6_addr, IPV6_ADDR_LEN);
		port = usin6->sin6_port;
	}

	if (!is_addr_match_rule(rule, &address))
		return false;

	if (!is_port_match_rule(rule, port))
		return false;

	return true;
}

int slice_rules_lookup(struct sock *sk, struct sockaddr *uaddr, u8 protocol)
{
	int uid;
	int err = -EINVAL;
	struct slice_route_rule *rule = NULL;

	if (sk == NULL || uaddr == NULL)
		return err;

	/*
	 * When in TCP_TIME_WAIT the sk is not a "struct sock" but
	 * "struct inet_timewait_sock" which is missing fields.
	 * So we ignore it.
	 */
	if (sk && sk->sk_state == TCP_TIME_WAIT)
		return err;

	uid = sk->sk_uid.val;

	if (uid < FIRST_APPLICATION_UID)
		return err;

	if (sk->sk_slice_binded) {
		hwlog_info("%s: sk_slice_binded, sk_mark = %d\n",
			__func__, sk->sk_mark);
		if (is_rule_in_list_by_net_id(sk->sk_mark & 0xFFFF)) {
			return 0;
		} else {
			hwlog_info("%s: sk_slice_binded, need close socket\n",
				__func__);
			close_socket(sk, protocol);
			return 0;
		}
	}

	rcu_read_lock();
	list_for_each_entry_rcu(rule, &g_slice_route_map, list) {
		if (slice_rule_match(uid, uaddr, protocol, rule)) {
			hwlog_info("%s: slice_rule_match, net_id = %d\n",
				__func__, rule->net_id);
			rcu_read_unlock();
			bind_socket_to_slice_net(rule, sk, protocol);
			return 0;
		}
	}
	rcu_read_unlock();

	return err;
}

static struct slice_route_rule *new_rule()
{
	struct slice_route_rule *rule = kmalloc(sizeof(*rule), GFP_ATOMIC);
	if (rule == NULL)
		return NULL;

	memset(rule, 0, sizeof(*rule));

	INIT_LIST_HEAD(&rule->uid_list);
	INIT_LIST_HEAD(&rule->addr_list);
	rule->stamp = jiffies;
	return rule;
}

static struct uid_list *new_uid_node()
{
	struct uid_list *uid_node = kmalloc(sizeof(*uid_node), GFP_ATOMIC);
	if (uid_node == NULL)
		return NULL;

	memset(uid_node, 0, sizeof(*uid_node));

	INIT_LIST_HEAD(&uid_node->list);
	uid_node->uid = 0;
	return uid_node;
}

static struct remote_address *new_addr_node()
{
	struct remote_address *addr_node =
		kmalloc(sizeof(*addr_node), GFP_ATOMIC);
	if (addr_node == NULL)
		return NULL;

	memset(addr_node, 0, sizeof(*addr_node));
	INIT_LIST_HEAD(&addr_node->list);
	return addr_node;
}

// if err != 0,need free memory of new_rule
static int add_rule_to_list(struct slice_route_rule *new_rule)
{
	struct slice_route_rule *rule = NULL;
	struct slice_route_rule *last = NULL;

	if (new_rule == NULL)
		return -EINVAL;

	if (is_rule_in_list(new_rule->priority))
		return -EINVAL;

	list_for_each_entry(rule, &g_slice_route_map, list) {
		if (rule->priority > new_rule->priority)
			break;
		last = rule;
	}

	if (last)
		list_add_rcu(&new_rule->list, &last->list);
	else
		list_add_rcu(&new_rule->list, &g_slice_route_map);

	return 0;
}

static int add_address_to_rule(
	struct slice_route_rule *rule, struct remote_address *addr)
{
	int err = -EINVAL;
	if (rule == NULL || addr == NULL)
		return err;

	if (is_addr_in_rule(rule, addr))
		return err;

	list_add_rcu(&addr->list, &rule->addr_list);
	rule->addr_num++;
	hwlog_info("%s: rule->addr_num is = %d\n", __func__, rule->addr_num);

	return 0;
}

static int add_uid_to_rule(
	struct slice_route_rule *rule, struct uid_list *curr_uid)
{
	int err = -EINVAL;
	if (rule == NULL || curr_uid == NULL)
		return err;

	if (is_uid_in_rule(rule, curr_uid->uid))
		return err;

	list_add_rcu(&curr_uid->list, &rule->uid_list);
	rule->uid_num++;
	hwlog_info("%s: rule->uid_num is = %d\n", __func__, rule->uid_num);

	return 0;
}

int add_uid_netid_to_rule_list(u32 uid, u32 precedence)
{
	struct slice_route_rule *rule = NULL;
	struct slice_route_rule *temp = NULL;
	struct uid_list *uid_node = NULL;
	int err = -EINVAL;

	uid_node = new_uid_node();

	if (uid_node == NULL)
		return err;

	uid_node->uid = uid;

	list_for_each_entry_safe(rule, temp, &g_slice_route_map, list) {
		if (precedence != rule->priority)
			continue;

		err = add_uid_to_rule(rule, uid_node);
		if (err != 0)
			break;

		return 0;
	}

	kfree(uid_node);
	return err;
}

// if err != 0,need free memory of addr
static int add_addr_netid_to_rule_list(struct remote_address *addr,
	u32 precedence)
{
	struct slice_route_rule *rule = NULL;
	struct slice_route_rule *temp = NULL;
	int err = -EINVAL;

	if (addr == NULL)
		return err;

	list_for_each_entry_safe(rule, temp, &g_slice_route_map, list) {
		if (precedence != rule->priority)
			continue;

		return add_address_to_rule(rule, addr);
	}
	return err;
}

static int del_all_uid_list_from_rule(struct slice_route_rule *rule)
{
	int err = -EINVAL;
	struct uid_list *curr = NULL;
	struct uid_list *temp = NULL;
	if (rule == NULL)
		return err;

	list_for_each_entry_safe(curr, temp, &rule->uid_list, list) {
		list_del_rcu(&curr->list);
		kfree(curr);
	}
	rule->uid_num = 0;

	return err;
}

static int del_all_addr_list_from_rule(struct slice_route_rule *rule)
{
	int err = -EINVAL;
	struct remote_address *curr = NULL;
	struct remote_address *temp = NULL;
	if (rule == NULL)
		return err;

	list_for_each_entry_safe(curr, temp, &rule->addr_list, list) {
		list_del_rcu(&curr->list);
		kfree(curr);
	}
	rule->addr_num = 0;

	return err;
}

static int del_uid_from_rule(struct slice_route_rule *rule, u32 uid)
{
	struct uid_list *curr = NULL;
	struct uid_list *temp = NULL;
	int err = -EINVAL;
	if (rule == NULL)
		return err;

	list_for_each_entry_safe(curr, temp, &rule->uid_list, list) {
		if (uid != curr->uid)
			continue;

		list_del_rcu(&curr->list);
		kfree(curr);
		rule->uid_num--;
		break;
	}

	if (list_empty(&rule->uid_list)) {
		list_del_rcu(&rule->list);
		del_all_addr_list_from_rule(rule);
		del_all_uid_list_from_rule(rule);
		kfree(rule);
	}
	return 0;
}

static int del_all_slice_rule()
{
	struct slice_route_rule *rule = NULL;
	struct slice_route_rule *temp = NULL;

	list_for_each_entry_safe(rule, temp, &g_slice_route_map, list) {
		list_del_rcu(&rule->list);
		del_all_addr_list_from_rule(rule);
		del_all_uid_list_from_rule(rule);
		kfree(rule);
	}
	return 0;
}

static int del_slice_rule_by_netid(u32 net_id)
{
	struct slice_route_rule *rule = NULL;
	struct slice_route_rule *temp = NULL;
	int err = -EINVAL;

	list_for_each_entry_safe(rule, temp, &g_slice_route_map, list) {
		if (net_id != rule->net_id)
			continue;

		list_del_rcu(&rule->list);
		del_all_addr_list_from_rule(rule);
		del_all_uid_list_from_rule(rule);
		kfree(rule);
		return 0;
	}
	return err;
}

static int del_uid_netid_from_rule_list(u32 uid, u32 net_id, u32 precedence)
{
	struct slice_route_rule *rule = NULL;
	struct slice_route_rule *temp = NULL;
	int err = -EINVAL;
	int del_num = 0;

	list_for_each_entry_safe(rule, temp, &g_slice_route_map, list) {
		if (net_id != rule->net_id)
			continue;
		if (precedence != rule->priority)
			continue;
		hwlog_info("%s: delete precedence = %d\n", __func__, precedence);
		del_uid_from_rule(rule, uid);
		del_num++;
	}
	if (del_num != 0) {
		return 0;
	} else {
		return err;
	}
}

static int set_uid_to_route_rule(
	struct slice_route_rule *route_rule, char **pos, int len)
{
	char *position = NULL;
	u32 uid_num;
	u32 i;
	int msg_len = len;
	const int msg_len_limit = 4;
	if (route_rule == NULL || pos == NULL || *pos == NULL)
		return -EINVAL;

	// msg len < 1B(uid num)
	if (msg_len < 1)
		return -EINVAL;
	position = *pos;
	uid_num = *position;
	hwlog_info("%s: uid_num is = %d\n", __func__, uid_num);

	// msg len < 1B(uid num) + 4B * uid_num(uid)
	if (msg_len < (1 + msg_len_limit * uid_num))
		return -EINVAL;
	msg_len = msg_len - (1 + msg_len_limit * uid_num);
	skip_byte(position, 1);
	for (i = 0; i < uid_num; i++) {
		struct uid_list *uid_node = new_uid_node();
		if (uid_node == NULL)
			return -EINVAL;
		memcpy(&uid_node->uid, position, sizeof(u32));
		hwlog_info("%s: uid = %d\n", __func__, uid_node->uid);
		if (add_uid_to_rule(route_rule, uid_node) == -EINVAL) {
			kfree(uid_node);
			return -EINVAL;
		}
		skip_byte(position, sizeof(u32));
	}
	*pos = position;
	return msg_len;
}

static int set_ipv4_to_route_rule(
	struct slice_route_rule *route_rule, char **pos, int len)
{
	u32 i;
	u8 ipv4_num;
	char *position = NULL;
	int msg_len = len;

	if (route_rule == NULL || pos == NULL)
		return -EINVAL;

	// msg len < 1B(ipv4 num)
	if (msg_len < 1)
		return -EINVAL;
	position = *pos;
	ipv4_num = *position;
	hwlog_info("%s: ipv4_num is = %d\n", __func__, ipv4_num);

	// msg len < 1B(ipv4 num) + 8B * ipv4_num(ipv4 addr + mask)
	if (msg_len < 1 + IPV4_ADDR_MASK_LEN * ipv4_num)
		return -EINVAL;
	msg_len = msg_len - (1 + IPV4_ADDR_MASK_LEN * ipv4_num);
	skip_byte(position, 1);
	for (i = 0; i < ipv4_num; i++) {
		struct remote_address *ipv4_addr = new_addr_node();
		if (ipv4_addr == NULL)
			return -EINVAL;
		ipv4_addr->ai_family = AF_INET;
		memcpy(&ipv4_addr->addr_u.ip4_addr.addr, position, IPV4_ADDR_LEN);
		skip_byte(position, IPV4_ADDR_LEN);
		memcpy(&ipv4_addr->addr_u.ip4_addr.mask, position, IPV4_ADDR_LEN);
		skip_byte(position, IPV4_ADDR_LEN);
		if (add_address_to_rule(route_rule, ipv4_addr) != 0) {
			kfree(ipv4_addr);
			return -EINVAL;
		}
	}

	*pos = position;
	return msg_len;
}

static int set_ipv6_to_route_rule(
	struct slice_route_rule *route_rule, char **pos, int len)
{
	u32 i;
	u8 ipv6_num;
	char *position = NULL;
	int msg_len = len;

	if (route_rule == NULL || pos == NULL)
		return -EINVAL;

	// msg len < 1B(ipv6 num)
	if (msg_len < 1)
		return -EINVAL;
	position = *pos;
	ipv6_num = *position;
	hwlog_info("%s: ipv6_num is = %d\n", __func__, ipv6_num);

	// msg len < 1B(ipv6 num) + 17B * ipv6_num(ipv6 addr + prefix len)
	if (msg_len < 1 + (IPV6_ADDR_LEN + 1) * ipv6_num)
		return -EINVAL;
	msg_len = msg_len - (1 + (IPV6_ADDR_LEN + 1) * ipv6_num);
	skip_byte(position, 1);
	for (i = 0; i < ipv6_num; i++) {
		struct remote_address *ipv6_addr = new_addr_node();
		if (ipv6_addr == NULL)
			return -EINVAL;
		ipv6_addr->ai_family = AF_INET6;
		memcpy(&ipv6_addr->addr_u.ip6_addr.addr, position, IPV6_ADDR_LEN);
		skip_byte(position, IPV6_ADDR_LEN);
		ipv6_addr->addr_u.ip6_addr.prefix_len = *position;
		skip_byte(position, 1);
		hwlog_info("%s: prefix_len is = %d\n", __func__,
			ipv6_addr->addr_u.ip6_addr.prefix_len);
		if (add_address_to_rule(route_rule, ipv6_addr) != 0) {
			kfree(ipv6_addr);
			return -EINVAL;
		}
	}
	*pos = position;
	return msg_len;
}

static int set_protocol_to_route_rule(
	struct slice_route_rule *route_rule, char **pos, int len)
{
	char *position = NULL;
	u32 i;
	int msg_len = len;

	if (route_rule == NULL || pos == NULL)
		return -EINVAL;

	// msg len < 1B(protocol num)
	if (msg_len < 1)
		return -EINVAL;
	position = *pos;
	route_rule->protocol_num = *position;
	hwlog_info("%s: protocol_num = %d\n",
		__func__, route_rule->protocol_num);
	if (route_rule->protocol_num > MAX_PROTOCOL_NUM)
		return -EINVAL;

	// msg len < 1B(protocol num) + 1B * protocol_num(protocol)
	if (msg_len < 1 + 1 * route_rule->protocol_num)
		return -EINVAL;
	msg_len = msg_len - (1 + 1 * route_rule->protocol_num);
	skip_byte(position, 1);
	for (i = 0; i < route_rule->protocol_num; i++) {
		route_rule->protocol[i] = *position;
		hwlog_info("%s: protocol = %d\n",
			__func__, route_rule->protocol[i]);
		skip_byte(position, 1);
	}
	*pos = position;
	return msg_len;
}

static int set_port_to_route_rule(
	struct slice_route_rule *route_rule, char **pos, int len)
{
	char *position = NULL;
	u8 port_range_num;
	u32 i;
	u16 port;
	int msg_len = len;
	const int port_num_single = 2;
	const int port_num_cout = 4;

	if (route_rule == NULL || pos == NULL)
		return -EINVAL;

	// msg len < 1B(single port num)
	if (msg_len < 1)
		return -EINVAL;
	position = *pos;
	route_rule->port_num = *position;
	if (route_rule->port_num > MAX_REMOTE_PORT_RANGE_NUM)
		return -EINVAL;

	// msg len < 1B(single port num) + 2B * port_num(single port)
	if (msg_len < (1 + port_num_single * route_rule->port_num))
		return -EINVAL;
	msg_len = msg_len - (1 + port_num_single * route_rule->port_num);
	skip_byte(position, 1);
	for (i = 0; i < route_rule->port_num; i++) {
		memcpy(&port, position, sizeof(u16));
		route_rule->port[i].start = cpu_to_be16(port);
		route_rule->port[i].end = route_rule->port[i].start;
		skip_byte(position, sizeof(u16));
	}
	port_range_num = *position;
	skip_byte(position, 1);
	route_rule->port_num += port_range_num;
	if (route_rule->port_num > MAX_REMOTE_PORT_RANGE_NUM)
		return -EINVAL;
	hwlog_info("%s: port num is = %d\n", __func__, route_rule->port_num);

	// msg len < 1B(port range num) + 4B * port_num(port range)
	if (msg_len < (1 + port_num_cout * port_range_num))
		return -EINVAL;
	msg_len = msg_len - (1 + port_num_cout * port_range_num);
	for (; i < route_rule->port_num; i++) {
		memcpy(&port, position, sizeof(u16));
		route_rule->port[i].start = cpu_to_be16(port);
		skip_byte(position, sizeof(u16));
		memcpy(&port, position, sizeof(u16));
		route_rule->port[i].end = cpu_to_be16(port);
		skip_byte(position, sizeof(u16));
		if (route_rule->port[i].start > route_rule->port[i].end) {
			port = route_rule->port[i].start;
			route_rule->port[i].start = route_rule->port[i].end;
			route_rule->port[i].end = port;
		}
	}
	*pos = position;
	return msg_len;
}

static int add_uid_to_exist_rule_list(char **pos, u32 precedence, int len)
{
	u32 uid_num;
	u32 uid;
	u32 i;
	char *position = NULL;
	int msg_len = len;
	const int uid_num_limit = 4;

	if (pos == NULL)
		return -EINVAL;

	// msg len < 1B(uid num)
	if (msg_len < 1)
		return -EINVAL;
	position = *pos;
	uid_num = *position;
	hwlog_info("%s: uid_num is = %d\n", __func__, uid_num);

	// msg len < 1B(uid num) + 4B * uid_num(uid)
	if (msg_len < (1 + uid_num_limit * uid_num))
		return -EINVAL;
	msg_len = msg_len - (1 + uid_num_limit * uid_num);
	skip_byte(position, 1);
	for (i = 0; i < uid_num; i++) {
		memcpy(&uid, position, sizeof(u32));
		if (add_uid_netid_to_rule_list(uid, precedence) == -EINVAL)
			return -EINVAL;
		skip_byte(position, sizeof(u32));
	}
	*pos = position;
	return msg_len;
}

static int add_ipv4_to_exist_rule_list(char **pos, u32 precedence, int len)
{
	u32 i;
	u8 ipv4_num;
	char *position = NULL;
	int msg_len = len;

	if (pos == NULL)
		return -EINVAL;

	// msg len < 1B(ipv4 num)
	if (msg_len < 1)
		return -EINVAL;
	position = *pos;
	ipv4_num = *position;
	hwlog_info("%s: ipv4_num is = %d\n", __func__, ipv4_num);

	// msg len < 1B(ipv4 num) + 8B * ipv4_num(ipv4 addr + mask)
	if (msg_len < 1 + IPV4_ADDR_MASK_LEN * ipv4_num)
		return -EINVAL;
	msg_len = msg_len - (1 + IPV4_ADDR_MASK_LEN * ipv4_num);
	skip_byte(position, 1);
	for (i = 0; i < ipv4_num; i++) {
		struct remote_address *ipv4_addr = new_addr_node();
		if (ipv4_addr == NULL)
			return -EINVAL;
		ipv4_addr->ai_family = AF_INET;
		memcpy(&ipv4_addr->addr_u.ip4_addr.addr, position, IPV4_ADDR_LEN);
		skip_byte(position, IPV4_ADDR_LEN);
		memcpy(&ipv4_addr->addr_u.ip4_addr.mask, position, IPV4_ADDR_LEN);
		skip_byte(position, IPV4_ADDR_LEN);
		if (add_addr_netid_to_rule_list(ipv4_addr, precedence) != 0) {
			kfree(ipv4_addr);
			return -EINVAL;
		}
	}
	*pos = position;
	return msg_len;
}

static int add_ipv6_to_exist_rule_list(char **pos, u32 precedence, int len)
{
	u32 i;
	u8 ipv6_num;
	char *position = NULL;
	int msg_len = len;

	if (pos == NULL)
		return -EINVAL;

	// msg len < 1B(ipv6 num)
	if (msg_len < 1)
		return -EINVAL;
	position = *pos;
	ipv6_num = *position;
	hwlog_info("%s: ipv6_num is = %d\n", __func__, ipv6_num);

	// msg len < 1B(ipv6 num) + 17B * ipv6_num(ipv6 addr + prefix len)
	if (msg_len < 1 + (IPV6_ADDR_LEN + 1) * ipv6_num)
		return -EINVAL;
	msg_len = msg_len - (1 + (IPV6_ADDR_LEN + 1) * ipv6_num);
	skip_byte(position, 1);
	for (i = 0; i < ipv6_num; i++) {
		struct remote_address *ipv6_addr = new_addr_node();
		if (ipv6_addr == NULL)
			return -EINVAL;
		ipv6_addr->ai_family = AF_INET6;
		memcpy(&ipv6_addr->addr_u.ip6_addr.addr, position, IPV6_ADDR_LEN);
		skip_byte(position, IPV6_ADDR_LEN);
		ipv6_addr->addr_u.ip6_addr.prefix_len = *position;
		skip_byte(position, 1);
		if (add_addr_netid_to_rule_list(ipv6_addr, precedence) != 0) {
			kfree(ipv6_addr);
			return -EINVAL;
		}
	}
	*pos = position;
	return msg_len;
}

static void get_new_rule_and_add_to_list(
	struct slice_route_rule *route_rule, char **pos, int len)
{
	int msg_len = len;
	msg_len = set_uid_to_route_rule(route_rule, pos, msg_len);
	if (msg_len < 0) {
		hwlog_info("%s: set uid to route rule fail\n", __func__);
		kfree(route_rule);
		return;
	}
	msg_len = set_ipv4_to_route_rule(route_rule, pos, msg_len);
	if (msg_len < 0) {
		hwlog_info("%s: set ipv4 to route rule fail\n", __func__);
		kfree(route_rule);
		return;
	}
	msg_len = set_ipv6_to_route_rule(route_rule, pos, msg_len);
	if (msg_len < 0) {
		hwlog_info("%s: set ipv6 to route rule fail\n", __func__);
		kfree(route_rule);
		return;
	}
	msg_len = set_protocol_to_route_rule(route_rule, pos, msg_len);
	if (msg_len < 0) {
		hwlog_info("%s: set protocol to route rule fail\n", __func__);
		kfree(route_rule);
		return;
	}
	msg_len = set_port_to_route_rule(route_rule, pos, msg_len);
	if (msg_len < 0) {
		hwlog_info("%s: set port to route rule fail\n", __func__);
		kfree(route_rule);
		return;
	}

	if (add_rule_to_list(route_rule) != 0) {
		hwlog_info("%s: add_rule_to_list fail\n", __func__);
		kfree(route_rule);
		return;
	}
	hwlog_info("%s: success\n", __func__);
}

static void delete_uid(u32 *position, u32 net_id, u32 len, u32 precedence_num,
	u32 uid_num)
{
	u32 precedence;
	u32 uid;
	u32 i;
	u32 j;
	u32 *position_pre = position;
	u32 *position_uid = NULL;

	// delete uid by precedence
	hwlog_info("%s: precedence_num = %d, uid_num = %d\n", __func__,
		precedence_num, uid_num);
	for (i = 0; i < precedence_num; i++) {
		precedence = *position_pre;
		position_pre++;
		hwlog_info("%s: precedence: %d\n", __func__, precedence);
		position_uid = position + (precedence_num + 1);
		for (j = 0; j < uid_num; j++) {
			uid =  *position_uid;
			hwlog_info("%s: uid: %d\n", __func__, uid);
			position_uid++;
			del_uid_netid_from_rule_list(uid, net_id, precedence);
		}
	}
}

static void delete_uid_by_precedence(u32 *position, u32 net_id, u32 len)
{
	u32 precedence_num;
	u32 uid_num;
	u32 *position_pre = position;
	u32 *position_uid = NULL;

	// msg len < 4B(precedence_num)
	if (len < sizeof(u32))
		return;
	len -= sizeof(u32);
	precedence_num = *position_pre;
	position_pre++;

	// msg len < 4B * precedence_num
	if (len < sizeof(u32) * precedence_num)
		return;
	len -= sizeof(u32) * precedence_num;

	// msg len < 4B(uid num)
	if (len < sizeof(u32))
		return;
	len -= sizeof(u32);
	position_uid = position_pre + precedence_num;
	uid_num = *position_uid;
	// msg len < 4B(uid) * uid_num
	if ((len < sizeof(u32) * uid_num) || uid_num == 0)
		return;
	delete_uid(position_pre, net_id, len, precedence_num, uid_num);
}

void bind_process_to_slice(struct req_msg_head *msg)
{
	char *position = (char *)msg;
	u32 net_id;
	u8 precedence;
	int len;
	struct slice_route_rule *route_rule = NULL;
	// len = 2B(type) + 2B(len) + 4B(net id) + 1B(precedence)
	const int msg_len_limit = 9;

	if (msg == NULL)
		return;

	hwlog_info("%s\n", __func__);
	len = msg->len;

	// msg len < 2B(type) + 2B(len) + 4B(net id) + 1B(precedence)
	if (len < msg_len_limit)
		return;
	len -= msg_len_limit;

	skip_byte(position, sizeof(u32));
	memcpy(&net_id, position, sizeof(u32));
	hwlog_info("%s: net id = %d\n", __func__, net_id);
	skip_byte(position, sizeof(u32));
	precedence = *position;
	hwlog_info("%s: precedence = %d\n", __func__, precedence);
	skip_byte(position, 1);

	if (is_rule_in_list((u32)precedence)) {
		hwlog_info("%s: precedence %d is in list\n",
			__func__, precedence);
		len = add_uid_to_exist_rule_list(&position, (u32)precedence,
			len);
		if (len < 0)
			return;
		len = add_ipv4_to_exist_rule_list(&position, (u32)precedence,
			len);
		if (len < 0)
			return;
		len = add_ipv6_to_exist_rule_list(&position, (u32)precedence,
			len);
		return;
	}

	route_rule = new_rule();
	if (route_rule == NULL)
		return;
	route_rule->net_id = net_id;
	route_rule->priority = precedence;

	get_new_rule_and_add_to_list(route_rule, &position, len);
}

void unbind_process_to_slice(struct req_msg_head *msg)
{
	char *position = (char *)msg;
	u32 del_type;
	u32 net_id;
	u32 len;
	u32 rst;
	const int msg_len_limit = 8;

	if (msg == NULL)
		return;

	hwlog_info("%s: msg len = %d\n", __func__, msg->len);
	len = msg->len;

	// msg len < 2B(type) + 2B(len) + 4B(del_type)
	if (len < msg_len_limit)
		return;
	len -= msg_len_limit;
	skip_byte(position, sizeof(u32));
	rst = memcpy_s(&del_type, sizeof(u32), position, sizeof(u32));
	if (rst != 0) {
		hwlog_info("%s: get del_type error\n", __func__);
		return;
	}
	skip_byte(position, sizeof(u32));

	// delete all
	if (del_type == DEL_BIND_ALL) {
		hwlog_info("%s: del_all_slice_rule\n", __func__);
		del_all_slice_rule();
		return;
	}

	// msg len < 4B(net_id)
	if (len < sizeof(u32))
		return;
	len -= sizeof(u32);
	memcpy(&net_id, position, sizeof(u32));
	skip_byte(position, sizeof(u32));

	// delete net_id
	if (del_type == DEL_BIND_NETID) {
		hwlog_info("%s: del_slice_rule_by_netid\n", __func__);
		del_slice_rule_by_netid(net_id);
		return;
	}
	if (del_type != DEL_BIND_PRECEDENCE)
		return;
	delete_uid_by_precedence((u32 *)position, net_id, len);
}

static int __init network_slice_route_init(void)
{
	hwlog_info("%s\n", __func__);
	INIT_LIST_HEAD(&g_slice_route_map);
	return 0;
}

static void __exit network_slice_route_exit(void)
{
	hwlog_info("%s\n", __func__);
}

module_init(network_slice_route_init);
module_exit(network_slice_route_exit);
