/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: mpdns module implemention
 * Author: shishuai shishuai.shi@huawei.com
 * Create: 2020-05-20
 */
#include "emcom/emcom_mpdns.h"
#include <linux/string.h>
#include "securec.h"
#include "../emcom_netlink.h"
#include "../emcom_utils.h"

#undef HWLOG_TAG
#define HWLOG_TAG emcom_mpdns
HWLOG_REGIST();

struct emcom_mpdns_cache g_mpdns_cache;
struct emcom_mpdns_app_configs g_mpdns_app_configs;

static spinlock_t g_mpdns_cache_lock;
static spinlock_t g_mpdns_app_conf_lock;

void emcom_mpdns_init(void)
{
	unsigned int i;
	spin_lock_init(&g_mpdns_cache_lock);
	spin_lock_init(&g_mpdns_app_conf_lock);

	spin_lock_bh(&g_mpdns_cache_lock);
	(void)memset_s(&g_mpdns_cache, sizeof(struct emcom_mpdns_cache), 0, sizeof(struct emcom_mpdns_cache));
	spin_unlock_bh(&g_mpdns_cache_lock);

	spin_lock_bh(&g_mpdns_app_conf_lock);
	g_mpdns_app_configs.next_update_id = 0;
	for (i = 0; i < EMCOM_MPDNS_MAX_APP_NUM; i++) {
		g_mpdns_app_configs.entries[i].uid = INDEX_INVALID;
		g_mpdns_app_configs.entries[i].hosts = NULL;
	}
	spin_unlock_bh(&g_mpdns_app_conf_lock);
}

inline bool emcom_is_num(char c)
{
	if (c >= '0' && c <= '9')
		return true;
	return false;
}

bool emcom_is_v6_addr_valid(struct in6_addr *v6_addr)
{
	unsigned int rst = 0;
	unsigned int i;
	for (i = 0; i < IPV6_ADDR32_SEG_NUM; i++)
		rst |= v6_addr->s6_addr32[i];
	return rst != 0;
}

struct emcom_mpdns_entry *emcom_mpdns_lookup_cache_entry(const char *host, int *entry_id)
{
	unsigned int id;
	for (id = 0; id < EMCOM_MPDNS_MAX_ENTRY_NUM; id++) {
		if (strcmp(host, g_mpdns_cache.entries[id].host) == 0) {
			*entry_id = id;
			return &(g_mpdns_cache.entries[id]);
		}
	}
	*entry_id = INDEX_INVALID;
	return NULL;
}

bool emcom_get_first_v4_addr(struct in_addr *out_addr, struct in_addr *v4_addrs, const unsigned num)
{
	unsigned int addr_id;
	for (addr_id = 0; addr_id < num; addr_id++) {
		if (v4_addrs[addr_id].s_addr != 0) {
			out_addr->s_addr = v4_addrs[addr_id].s_addr;
			return true;
		}
	}
	return false;
}

bool emcom_get_first_v6_addr(struct in6_addr *out_addr, struct in6_addr *v6_addrs, const unsigned num)
{
	errno_t err;
	unsigned addr_id;
	for (addr_id = 0; addr_id < num; addr_id++) {
		if (emcom_is_v6_addr_valid(&(v6_addrs[addr_id]))) {
			err = memcpy_s(out_addr, sizeof(struct in6_addr), &(v6_addrs[addr_id]), sizeof(struct in6_addr));
			if (err != EOK)
				continue;
			return true;
		}
	}
	return false;
}

bool emcom_v4_addr_match(struct in_addr *cur_addr, struct in_addr *v4_addrs, const unsigned num)
{
	unsigned addr_id;
	for (addr_id = 0; addr_id < num; addr_id++) {
		if (cur_addr->s_addr == v4_addrs[addr_id].s_addr)
			return true;
	}
	return false;
}

bool emcom_v6_addr_match(struct in6_addr *cur_addr, struct in6_addr *v6_addrs, const unsigned num)
{
	unsigned addr_id;
	for (addr_id = 0; addr_id < num; addr_id++) {
		if (memcmp(cur_addr, &(v6_addrs[addr_id]), sizeof(struct in6_addr)) == 0)
			return true;
	}
	return false;
}

bool emcom_lookup_target_v4_addr(struct in_addr *cur_addr, int cur_net, int tar_net, struct in_addr *tar_addr)
{
	unsigned int entry_id;
	for (entry_id = 0; entry_id < EMCOM_MPDNS_MAX_ENTRY_NUM; entry_id++) {
		struct emcom_mpdns_entry *cur_entry = &(g_mpdns_cache.entries[entry_id]);
		struct in_addr *cur_forward_addrs = cur_entry->net_addrs[cur_net].ipv4_addrs;
		struct in_addr *cur_reverse_addrs = cur_entry->net_addrs[cur_net].ipv4_reverse_addrs;
		struct in_addr *tar_forward_addrs = cur_entry->net_addrs[tar_net].ipv4_addrs;
		struct in_addr *tar_reverse_addrs = cur_entry->net_addrs[tar_net].ipv4_reverse_addrs;
		if (emcom_v4_addr_match(cur_addr, cur_forward_addrs, EMCOM_MPDNS_MAX_IPV4_ADDR_NUM) ||
			emcom_v4_addr_match(cur_addr, cur_reverse_addrs, EMCOM_MPDNS_MAX_IPV4_ADDR_NUM)) {
			if (emcom_get_first_v4_addr(tar_addr, tar_forward_addrs, EMCOM_MPDNS_MAX_IPV4_ADDR_NUM) ||
				emcom_get_first_v4_addr(tar_addr, tar_reverse_addrs, EMCOM_MPDNS_MAX_IPV4_ADDR_NUM))
				return true;
		}
	}
	return false;
}

bool emcom_lookup_target_v6_addr(struct in6_addr *cur_addr, int cur_net, int tar_net, struct in6_addr *tar_addr)
{
	unsigned int entry_id;
	for (entry_id = 0; entry_id < EMCOM_MPDNS_MAX_ENTRY_NUM; entry_id++) {
		struct emcom_mpdns_entry *cur_entry = &(g_mpdns_cache.entries[entry_id]);
		struct in6_addr *cur_forward_addrs = cur_entry->net_addrs[cur_net].ipv6_addrs;
		struct in6_addr *cur_reverse_addrs = cur_entry->net_addrs[cur_net].ipv6_reverse_addrs;
		struct in6_addr *tar_forward_addrs = cur_entry->net_addrs[tar_net].ipv6_addrs;
		struct in6_addr *tar_reverse_addrs = cur_entry->net_addrs[tar_net].ipv6_reverse_addrs;
		if (emcom_v6_addr_match(cur_addr, cur_forward_addrs, EMCOM_MPDNS_MAX_IPV6_ADDR_NUM) ||
			emcom_v6_addr_match(cur_addr, cur_reverse_addrs, EMCOM_MPDNS_MAX_IPV6_ADDR_NUM)) {
			if (emcom_get_first_v6_addr(tar_addr, tar_forward_addrs, EMCOM_MPDNS_MAX_IPV6_ADDR_NUM) ||
				emcom_get_first_v6_addr(tar_addr, tar_reverse_addrs, EMCOM_MPDNS_MAX_IPV6_ADDR_NUM))
				return true;
		}
	}
	return false;
}

bool emcom_clear_v4_addr(struct in_addr *cur_addr, struct in_addr *v4_addrs, const unsigned num)
{
	unsigned int addr_id;
	unsigned int i = 0;
	errno_t err;

	struct in_addr *tmp_v4_addrs = kzalloc(sizeof(struct in_addr) * num, GFP_ATOMIC);
	if (tmp_v4_addrs == NULL)
		return false;

	for (addr_id = 0; addr_id < num; addr_id++) {
		if (v4_addrs[addr_id].s_addr != cur_addr->s_addr)
			tmp_v4_addrs[i++].s_addr = v4_addrs[addr_id].s_addr;
	}

	err = memcpy_s(v4_addrs, sizeof(struct in_addr) * num, tmp_v4_addrs, sizeof(struct in_addr) * num);
	kfree(tmp_v4_addrs);
	return (err == EOK);
}

void emcom_clear_v4_reverse_addr(struct in_addr *cur_addr, struct in_addr *v4_addrs, const unsigned num)
{
	unsigned int addr_id;
	for (addr_id = 0; addr_id < num; addr_id++) {
		if (v4_addrs[addr_id].s_addr == cur_addr->s_addr)
			v4_addrs[addr_id].s_addr = 0;
	}
}

bool emcom_clear_v6_addr(struct in6_addr *cur_addr, struct in6_addr *v6_addrs, const unsigned num)
{
	unsigned addr_id;
	unsigned int i = 0;
	errno_t err;

	struct in6_addr *tmp_v6_addrs = kzalloc(sizeof(struct in6_addr) * num, GFP_ATOMIC);
	if (tmp_v6_addrs == NULL)
		return false;

	for (addr_id = 0; addr_id < num; addr_id++) {
		if (memcmp(cur_addr, &(v6_addrs[addr_id]), sizeof(struct in6_addr)) != 0) {
			err = memcpy_s(&(tmp_v6_addrs[i++]), sizeof(struct in6_addr),
				&(v6_addrs[addr_id]), sizeof(struct in6_addr));
			if (err != EOK) {
				kfree(tmp_v6_addrs);
				return false;
			}
		}
	}
	err = memcpy_s(v6_addrs, sizeof(struct in6_addr) * num, tmp_v6_addrs, sizeof(struct in6_addr) * num);
	kfree(tmp_v6_addrs);
	return (err == EOK);
}

void emcom_clear_v6_reverse_addr(struct in6_addr *cur_addr, struct in6_addr *v6_addrs, const unsigned num)
{
	unsigned addr_id;
	for (addr_id = 0; addr_id < num; addr_id++) {
		if (memcmp(cur_addr, &(v6_addrs[addr_id]), sizeof(struct in6_addr)) == 0)
			(void)memset_s(&(v6_addrs[addr_id]), sizeof(struct in6_addr), 0, sizeof(struct in6_addr));
	}
}

bool emcom_mpdns_fill_v4_cache(const char *host, struct in_addr *v4_addr, int cur_net)
{
	errno_t err;
	int existed_id;
	struct in_addr *v4_reverse_addrs = NULL;

	struct emcom_mpdns_entry *existed_entry = emcom_mpdns_lookup_cache_entry(host, &existed_id);
	if (existed_entry == NULL) {
		struct emcom_mpdns_entry *entry = &(g_mpdns_cache.entries[g_mpdns_cache.next_update_id]);
		(void)memset_s(entry, sizeof(struct emcom_mpdns_entry), 0, sizeof(struct emcom_mpdns_entry));

		err = memcpy_s(entry->host, EMCOM_MAX_HOST_LEN, host, strlen(host));
		if (err != EOK)
			return true;

		if (v4_addr == NULL)
			return true;

		v4_reverse_addrs = entry->net_addrs[cur_net].ipv4_reverse_addrs;
		v4_reverse_addrs[entry->net_addrs[cur_net].v4_raddr_update_id].s_addr = v4_addr->s_addr;
		entry->net_addrs[cur_net].v4_raddr_update_id++;
		entry->net_addrs[cur_net].v4_raddr_update_id %= EMCOM_MPDNS_MAX_IPV4_ADDR_NUM;
		g_mpdns_cache.next_update_id++;
		g_mpdns_cache.next_update_id %= EMCOM_MPDNS_MAX_ENTRY_NUM;
		return true;
	} else {
		struct emcom_mpdns_entry *entry = &(g_mpdns_cache.entries[existed_id]);
		bool v4_forward_addrs_empty = (entry->net_addrs[cur_net].ipv4_addrs[0].s_addr == 0);
		if (v4_addr == NULL)
			return v4_forward_addrs_empty;
		v4_reverse_addrs = entry->net_addrs[cur_net].ipv4_reverse_addrs;
		if (emcom_v4_addr_match(v4_addr, v4_reverse_addrs, EMCOM_MPDNS_MAX_IPV4_ADDR_NUM))
			return v4_forward_addrs_empty;

		v4_reverse_addrs[entry->net_addrs[cur_net].v4_raddr_update_id].s_addr = v4_addr->s_addr;
		entry->net_addrs[cur_net].v4_raddr_update_id++;
		entry->net_addrs[cur_net].v4_raddr_update_id %= EMCOM_MPDNS_MAX_IPV4_ADDR_NUM;
		return v4_forward_addrs_empty;
	}
}

bool emcom_mpdns_fill_v6_cache(const char *host, struct in6_addr *v6_addr, int cur_net)
{
	errno_t err;
	int existed_id;
	struct in6_addr *v6_reverse_addrs = NULL;

	struct emcom_mpdns_entry *existed_entry = emcom_mpdns_lookup_cache_entry(host, &existed_id);
	if (existed_entry == NULL) {
		struct emcom_mpdns_entry *entry = &(g_mpdns_cache.entries[g_mpdns_cache.next_update_id]);
		(void)memset_s(entry, sizeof(struct emcom_mpdns_entry), 0, sizeof(struct emcom_mpdns_entry));

		err = memcpy_s(entry->host, EMCOM_MAX_HOST_LEN, host, strlen(host));
		if (err != EOK) {
			return true;
		}
		if (v6_addr == NULL)
			return true;

		v6_reverse_addrs = entry->net_addrs[cur_net].ipv6_reverse_addrs;
		err = memcpy_s(&(v6_reverse_addrs[entry->net_addrs[cur_net].v6_raddr_update_id]), sizeof(struct in6_addr),
			v6_addr, sizeof(struct in6_addr));
		if (err != EOK)
			return true;
		entry->net_addrs[cur_net].v6_raddr_update_id++;
		entry->net_addrs[cur_net].v6_raddr_update_id %= EMCOM_MPDNS_MAX_IPV6_ADDR_NUM;
		g_mpdns_cache.next_update_id++;
		g_mpdns_cache.next_update_id %= EMCOM_MPDNS_MAX_ENTRY_NUM;
		return true;
	} else {
		struct emcom_mpdns_entry *entry = &(g_mpdns_cache.entries[existed_id]);
		bool v6_forward_addrs_empty = !emcom_is_v6_addr_valid(&(entry->net_addrs[cur_net].ipv6_addrs[0]));
		if (v6_addr == NULL)
			return v6_forward_addrs_empty;

		v6_reverse_addrs = entry->net_addrs[cur_net].ipv6_reverse_addrs;
		if (emcom_v6_addr_match(v6_addr, v6_reverse_addrs, EMCOM_MPDNS_MAX_IPV6_ADDR_NUM))
			return v6_forward_addrs_empty;   // already in, no need add

		err = memcpy_s(&(v6_reverse_addrs[entry->net_addrs[cur_net].v6_raddr_update_id]), sizeof(struct in6_addr),
			v6_addr, sizeof(struct in6_addr));
		if (err != EOK)
			return v6_forward_addrs_empty;
		entry->net_addrs[cur_net].v6_raddr_update_id++;
		entry->net_addrs[cur_net].v6_raddr_update_id %= EMCOM_MPDNS_MAX_IPV6_ADDR_NUM;
		return v6_forward_addrs_empty;
	}
}

void emcom_mpdns_add_cache(struct emcom_mpdns_result *mpdns_result)
{
	errno_t err;
	bool rst = false;
	int cur_net_type = mpdns_result->net_type;
	struct in_addr *v4_addrs = NULL;
	struct in6_addr *v6_addrs = NULL;
	struct emcom_mpdns_entry *entry = &(g_mpdns_cache.entries[g_mpdns_cache.next_update_id]);
	(void)memset_s(entry, sizeof(struct emcom_mpdns_entry), 0, sizeof(struct emcom_mpdns_entry));

	err = memcpy_s(entry->host, EMCOM_MAX_HOST_LEN, mpdns_result->host, strlen(mpdns_result->host));
	if (err != EOK)
		return;

	v4_addrs = entry->net_addrs[cur_net_type].ipv4_addrs;
	err = memcpy_s(v4_addrs, sizeof(struct in_addr) * EMCOM_MPDNS_MAX_IPV4_ADDR_NUM,
		mpdns_result->v4_addrs, sizeof(struct in_addr) * EMCOM_MPDNS_MAX_IPV4_ADDR_NUM);
	if (err == EOK)
		rst = true;

	v6_addrs = entry->net_addrs[cur_net_type].ipv6_addrs;
	err = memcpy_s(v6_addrs, sizeof(struct in6_addr) * EMCOM_MPDNS_MAX_IPV6_ADDR_NUM,
		mpdns_result->v6_addrs, sizeof(struct in6_addr) * EMCOM_MPDNS_MAX_IPV6_ADDR_NUM);
	if (err == EOK)
		rst = true;

	if (rst) {
		g_mpdns_cache.next_update_id++;
		g_mpdns_cache.next_update_id %= EMCOM_MPDNS_MAX_ENTRY_NUM;
	}
}

void emcom_mpdns_mod_cache(struct emcom_mpdns_result *mpdns_result, struct emcom_mpdns_entry *entry)
{
	errno_t err;
	int cur_net_type = mpdns_result->net_type;
	struct in_addr *v4_addrs = NULL;
	struct in6_addr *v6_addrs = NULL;

	v4_addrs = entry->net_addrs[cur_net_type].ipv4_addrs;
	if (mpdns_result->v4_addrs[0].s_addr != 0) {
		err = memcpy_s(v4_addrs, sizeof(struct in_addr) * EMCOM_MPDNS_MAX_IPV4_ADDR_NUM,
			mpdns_result->v4_addrs, sizeof(struct in_addr) * EMCOM_MPDNS_MAX_IPV4_ADDR_NUM);
		if (err != EOK)
			emcom_loge("modify v4 addrs fail");
	}

	v6_addrs = entry->net_addrs[cur_net_type].ipv6_addrs;
	if (emcom_is_v6_addr_valid(&(mpdns_result->v6_addrs[0]))) {
		err = memcpy_s(v6_addrs, sizeof(struct in6_addr) * EMCOM_MPDNS_MAX_IPV6_ADDR_NUM,
			mpdns_result->v6_addrs, sizeof(struct in6_addr) * EMCOM_MPDNS_MAX_IPV6_ADDR_NUM);
		if (err != EOK)
			emcom_loge("modify v6 addrs fail");
	}
}

struct emcom_mpdns_app_conf_entry* emcom_mpdns_lookup_app_config(int uid, int *conf_id)
{
	unsigned int i;
	for (i = 0; i < EMCOM_MPDNS_MAX_APP_NUM; i++) {
		if (g_mpdns_app_configs.entries[i].uid == uid) {
			*conf_id = i;
			return &(g_mpdns_app_configs.entries[i]);
		}
	}
	*conf_id = INDEX_INVALID;
	return NULL;
}

struct emcom_mpdns_app_conf_entry* emcom_mpdns_lookup_free_app_config(int *conf_id)
{
	unsigned int i;
	for (i = 0; i < EMCOM_MPDNS_MAX_APP_NUM; i++) {
		if (g_mpdns_app_configs.entries[i].uid == INDEX_INVALID) {
			*conf_id = i;
			return &(g_mpdns_app_configs.entries[i]);
		}
	}
	*conf_id = INDEX_INVALID;
	return NULL;
}

void emcom_mpdns_delete_app_config(int uid)
{
	int conf_id = INDEX_INVALID;
	struct emcom_mpdns_app_conf_entry *found_entry = emcom_mpdns_lookup_app_config(uid, &conf_id);
	if (found_entry == NULL)
		return;

	found_entry->uid = INDEX_INVALID;
	if (found_entry->hosts != NULL) {
		kfree(found_entry->hosts);
		found_entry->hosts = NULL;
	}
}

int emcom_mpdns_update_app_config_hosts(struct emcom_mpdns_app_conf_entry *entry, const char *hosts)
{
	errno_t err;
	unsigned int len;
	if (entry->hosts != NULL) {
		kfree(entry->hosts);
		entry->hosts = NULL;
	}
	len = strlen(hosts) + 1;
	entry->hosts = kzalloc(len, GFP_ATOMIC);
	if (entry->hosts == NULL)
		return -1;

	err = memcpy_s(entry->hosts, len, hosts, len);
	if (err != EOK)
		return -1;

	return 0;
}

void emcom_mpdns_add_app_config(int uid, const char *hosts)
{
	int conf_id = INDEX_INVALID;
	struct emcom_mpdns_app_conf_entry *found_entry = NULL;
	struct emcom_mpdns_app_conf_entry *free_entry = NULL;
	struct emcom_mpdns_app_conf_entry *add_entry = NULL;
	found_entry = emcom_mpdns_lookup_app_config(uid, &conf_id);
	if (found_entry != NULL) {  // do update
		emcom_mpdns_update_app_config_hosts(found_entry, hosts);
		return;
	}
	free_entry = emcom_mpdns_lookup_free_app_config(&conf_id);
	if (free_entry != NULL) {
		free_entry->uid = uid;
		emcom_mpdns_update_app_config_hosts(free_entry, hosts);
		return;
	}
	add_entry = &(g_mpdns_app_configs.entries[g_mpdns_app_configs.next_update_id]);
	add_entry->uid = uid;
	emcom_mpdns_update_app_config_hosts(add_entry, hosts);
	g_mpdns_app_configs.next_update_id++;
	g_mpdns_app_configs.next_update_id %= EMCOM_MPDNS_MAX_APP_NUM;
}

void emcom_mpdns_update_app_config(int uid, const char *hosts)
{
	int conf_id = INDEX_INVALID;
	struct emcom_mpdns_app_conf_entry *found_entry = emcom_mpdns_lookup_app_config(uid, &conf_id);
	if (found_entry == NULL)
		return;

	emcom_mpdns_update_app_config_hosts(found_entry, hosts);
}

bool emcom_mpdns_host_config_match(const char *conf_host, const char *host)
{
	if (strstr(conf_host, COMMON_HOST_PREFIX) == conf_host) {  // at the begin pos
		char *match_pos = strstr(host, conf_host + strlen(COMMON_HOST_PREFIX));
		if (match_pos != NULL && strlen(match_pos) == strlen(conf_host + strlen(COMMON_HOST_PREFIX)))
			return true;
		return false;
	}
	return (strcmp(conf_host, host) == 0);
}

/**
 * flush mpdns cache on given net
 */
void emcom_mpdns_flush_cache(int net_type)
{
	unsigned int entry_id;

	if (!emcom_xengine_is_net_type_valid(net_type))
		return;

	spin_lock_bh(&g_mpdns_cache_lock);
	for (entry_id = 0; entry_id < EMCOM_MPDNS_MAX_ENTRY_NUM; entry_id++) {
		struct emcom_mpdns_entry *entry = &(g_mpdns_cache.entries[entry_id]);
		(void)memset_s(&(entry->net_addrs[net_type]), sizeof(struct emcom_mpdns_net_addrs),
			0, sizeof(struct emcom_mpdns_net_addrs));
	}
	spin_unlock_bh(&g_mpdns_cache_lock);
	emcom_logi("mpdns cache on net:%d flushed", net_type);
}

/**
 * set app mpdns config
 */
void emcom_mpdns_set_app_config(struct emcom_mpdns_config *mpdns_config)
{
	if (mpdns_config == NULL || mpdns_config->uid < 0 || mpdns_config->hosts == NULL)
		return;

	spin_lock_bh(&g_mpdns_app_conf_lock);
	switch (mpdns_config->act) {
	case EMCOM_MPDNS_APP_CONF_DELETE:
		emcom_mpdns_delete_app_config(mpdns_config->uid);
		break;
	case EMCOM_MPDNS_APP_CONF_ADD:
		emcom_mpdns_add_app_config(mpdns_config->uid, mpdns_config->hosts);
		break;
	case EMCOM_MPDNS_APP_CONF_UPDATE:
		emcom_mpdns_update_app_config(mpdns_config->uid, mpdns_config->hosts);
		break;
	default:
		break;
	}
	spin_unlock_bh(&g_mpdns_app_conf_lock);
}

/**
 * is app and host allowed mpdns feature
 */
bool emcom_mpdns_is_allowed(uid_t uid, const char *host)
{
	int conf_id = INDEX_INVALID;
	struct emcom_mpdns_app_conf_entry *found_entry = NULL;
	unsigned int hosts_len;
	char conf_host[EMCOM_MAX_HOST_LEN] = {0};
	unsigned int left_idx = 0;
	unsigned int right_idx = 0;
	errno_t err;

	if (uid < 0)
		return false;

	spin_lock_bh(&g_mpdns_app_conf_lock);
	found_entry = emcom_mpdns_lookup_app_config(uid, &conf_id);
	if (found_entry == NULL || found_entry->hosts == NULL) {
		spin_unlock_bh(&g_mpdns_app_conf_lock);
		return false;
	}
	hosts_len = strlen(found_entry->hosts);
	if (hosts_len == 0 || host == NULL) {
		spin_unlock_bh(&g_mpdns_app_conf_lock);
		return true;   // match all
	}

	while (left_idx <= right_idx && right_idx <= hosts_len) {
		if (found_entry->hosts[right_idx] == ',' || found_entry->hosts[right_idx] == '\0') {
			err = memcpy_s(conf_host, EMCOM_MAX_HOST_LEN, found_entry->hosts + left_idx, right_idx - left_idx);
			if (err == EOK) {
				if (emcom_mpdns_host_config_match(conf_host, host)) {
					spin_unlock_bh(&g_mpdns_app_conf_lock);
					return true;
				}
			}
			(void)memset_s(conf_host, EMCOM_MAX_HOST_LEN, 0, EMCOM_MAX_HOST_LEN);
			left_idx = right_idx + 1;
		}
		right_idx++;
	}
	spin_unlock_bh(&g_mpdns_app_conf_lock);
	return false;
}

/**
 * try update sock dst addr according mpdns cache finding rst
 */
void emcom_mpdns_update_dst_addr(struct sock *sk, struct sockaddr *uaddr, int cur_net, int tar_net)
{
	errno_t err;
	bool rst = false;
	if (!emcom_xengine_is_net_type_valid(cur_net) || !emcom_xengine_is_net_type_valid(tar_net) ||
		cur_net == tar_net)
		return;

	if (uaddr->sa_family == AF_INET) {
		struct sockaddr_in *usin = (struct sockaddr_in *)uaddr;
		struct in_addr *cur_v4_addr = &(usin->sin_addr);
		struct in_addr tar_v4_addr;
		spin_lock_bh(&g_mpdns_cache_lock);
		rst = emcom_lookup_target_v4_addr(cur_v4_addr, cur_net, tar_net, &tar_v4_addr);
		spin_unlock_bh(&g_mpdns_cache_lock);
		if (!rst)
			return;
		usin->sin_addr.s_addr = tar_v4_addr.s_addr;
		emcom_logi("got new v4 addr");
	} else if (uaddr->sa_family == AF_INET6) {
		struct sockaddr_in6 *usin = (struct sockaddr_in6 *)uaddr;
		struct in6_addr *cur_v6_addr = &(usin->sin6_addr);
		struct in6_addr tar_v6_addr;
		spin_lock_bh(&g_mpdns_cache_lock);
		rst = emcom_lookup_target_v6_addr(cur_v6_addr, cur_net, tar_net, &tar_v6_addr);
		spin_unlock_bh(&g_mpdns_cache_lock);
		if (!rst)
			return;
		emcom_logi("got new v6 addr");
		err = memcpy_s(&(usin->sin6_addr), sizeof(struct in6_addr), &tar_v6_addr, sizeof(struct in6_addr));
		if (err != EOK)
			return;
	} else {
		return;
	}
}

/**
 * update forward mpdns cache, input rst is delivered by emcom(hicom) daemon
 */
void emcom_mpdns_update_cache(struct emcom_mpdns_result *mpdns_result)
{
	int net_type;
	unsigned int host_len;
	int existed_id = INDEX_INVALID;
	struct emcom_mpdns_entry *existed_entry = NULL;

	if (mpdns_result == NULL)
		return;

	net_type = mpdns_result->net_type;
	host_len = strlen(mpdns_result->host);
	if (!emcom_xengine_is_net_type_valid(net_type)
		|| host_len == 0 || host_len >= EMCOM_MAX_HOST_LEN)
		return;

	spin_lock_bh(&g_mpdns_cache_lock);
	existed_entry = emcom_mpdns_lookup_cache_entry(mpdns_result->host, &existed_id);
	if (existed_entry == NULL) {
		emcom_mpdns_add_cache(mpdns_result);
	} else {
		struct emcom_mpdns_entry *entry = &(g_mpdns_cache.entries[existed_id]);
		emcom_mpdns_mod_cache(mpdns_result, entry);
	}
	spin_unlock_bh(&g_mpdns_cache_lock);
}

/**
 * supply mpdns reverse and forward cache, input para is delivered by kernel hooking and parsing result
 * note: the input v4_addr may be null when need a special process
 */
bool emcom_mpdns_supply_v4_cache(const char *host, struct in_addr *v4_addr, int cur_net)
{
	bool rst = false;
	if (host == NULL || strlen(host) >= EMCOM_MAX_HOST_LEN)
		return false;

	if (!emcom_xengine_is_net_type_valid(cur_net))
		return false;

	spin_lock_bh(&g_mpdns_cache_lock);
	rst = emcom_mpdns_fill_v4_cache(host, v4_addr, cur_net);
	spin_unlock_bh(&g_mpdns_cache_lock);
	return rst;
}

/**
 * supply mpdns reverse and forward cache, input para is delivered by kernel hooking and parsing result
 * note: the input v6_addr may be null when need a special process
 */
bool emcom_mpdns_supply_v6_cache(const char *host, struct in6_addr *v6_addr, int cur_net)
{
	bool rst = false;
	if (host == NULL || strlen(host) >= EMCOM_MAX_HOST_LEN)
		return false;

	if (!emcom_xengine_is_net_type_valid(cur_net))
		return false;

	spin_lock_bh(&g_mpdns_cache_lock);
	rst = emcom_mpdns_fill_v6_cache(host, v6_addr, cur_net);
	spin_unlock_bh(&g_mpdns_cache_lock);
	return rst;
}

void emcom_mpdns_clear_v4_addr(struct in_addr *addr, int net_type)
{
	unsigned entry_id;
	if (!emcom_xengine_is_net_type_valid(net_type))
		return;

	spin_lock_bh(&g_mpdns_cache_lock);
	for (entry_id = 0; entry_id < EMCOM_MPDNS_MAX_ENTRY_NUM; entry_id++) {
		struct emcom_mpdns_entry *cur_entry = &(g_mpdns_cache.entries[entry_id]);
		struct emcom_mpdns_net_addrs *cur_addrs = &(cur_entry->net_addrs[net_type]);
		struct in_addr *cur_forward_addrs = cur_addrs->ipv4_addrs;
		struct in_addr *cur_reverse_addrs = cur_addrs->ipv4_reverse_addrs;
		emcom_clear_v4_addr(addr, cur_forward_addrs, EMCOM_MPDNS_MAX_IPV4_ADDR_NUM);
		emcom_clear_v4_reverse_addr(addr, cur_reverse_addrs, EMCOM_MPDNS_MAX_IPV4_ADDR_NUM);
	}
	spin_unlock_bh(&g_mpdns_cache_lock);
}

void emcom_mpdns_clear_v6_addr(struct in6_addr *addr, int net_type)
{
	unsigned entry_id;
	if (!emcom_xengine_is_net_type_valid(net_type))
		return;

	spin_lock_bh(&g_mpdns_cache_lock);
	for (entry_id = 0; entry_id < EMCOM_MPDNS_MAX_ENTRY_NUM; entry_id++) {
		struct emcom_mpdns_entry *cur_entry = &(g_mpdns_cache.entries[entry_id]);
		struct emcom_mpdns_net_addrs *cur_addrs = &(cur_entry->net_addrs[net_type]);
		struct in6_addr *cur_forward_addrs = cur_addrs->ipv6_addrs;
		struct in6_addr *cur_reverse_addrs = cur_addrs->ipv6_reverse_addrs;
		emcom_clear_v6_addr(addr, cur_forward_addrs, EMCOM_MPDNS_MAX_IPV6_ADDR_NUM);
		emcom_clear_v6_reverse_addr(addr, cur_reverse_addrs, EMCOM_MPDNS_MAX_IPV6_ADDR_NUM);
	}
	spin_unlock_bh(&g_mpdns_cache_lock);
}

/**
 * Check if the paresed host is alphabet format
 */
bool emcom_is_alp_host(const char *host)
{
	unsigned int len;
	char *sep_pos = NULL;
	unsigned int num_num = 0;
	unsigned int sep_num = 0;
	unsigned int i;

	if (host == NULL)
		return false;
	len = strlen(host);
	if (len == 0)
		return false;
	sep_pos = strstr(host, IPV6_ADDR_STR_SEP);
	if (sep_pos != NULL)
		return false;
	if (len > MAX_IPV4_ADDR_STR_LEN || len < MIN_IPV4_ADDR_STR_LEN)
		return true;

	for (i = 0; i < len; i++) {
		if (emcom_is_num(host[i]))
			num_num++;
		else if (host[i] == '.')
			sep_num++;
		else
			continue;
	}
	return !((num_num + sep_num) == len && sep_num == IPV4_ADDR_STR_SEG_NUM);
}

/**
 * parse host infomation from hooked data
 */
bool emcom_parse_httpget_host(const unsigned char *req, char *host_buf, unsigned int host_len)
{
	const char *http_req_host = NULL;
	const char *http_req_enter_pos = NULL;
	unsigned int len = 0;
	errno_t err;

	if ((req == NULL) || (host_buf == NULL) ||
		(host_len == 0) || (host_len > EMCOM_MAX_HOST_LEN))
		return false;

	/* Get address of Host field */
	http_req_host = strstr(req, HTTP_REQUEST_GET_HOST_STR);
	if (http_req_host == NULL)
		return false;

	/* Get Address of "\r\n" */
	http_req_enter_pos = strstr(http_req_host, HTTP_REQUEST_ENTER_STR);
	if (http_req_enter_pos == NULL)
		return false;
	len = http_req_enter_pos - http_req_host - strlen(HTTP_REQUEST_GET_HOST_STR);
	if (len >= host_len)
		return false;

	err = memcpy_s(host_buf, host_len,
		http_req_host + strlen(HTTP_REQUEST_GET_HOST_STR), len);
	if (err != EOK)
		return false;

	return true;
}

/**
 * send host info to daemon
 */
void emcom_mpdns_host_send(uid_t uid, int net_type, int ip_type, const char *host)
{
	errno_t err;
	struct emcom_host_info host_info;
	if (host == NULL || strlen(host) >= EMCOM_MAX_HOST_LEN)
		return;
	host_info.uid = uid;
	host_info.net_type = net_type;
	host_info.ip_type = ip_type;
	err = memset_s(host_info.host, EMCOM_MAX_HOST_LEN, 0, sizeof(host_info.host));
	if (err != EOK)
		return;
	err = memcpy_s(host_info.host, EMCOM_MAX_HOST_LEN, host, strlen(host));
	if (err != EOK)
		return;

	emcom_send_msg2daemon(NETLINK_EMCOM_KD_HOST_IND, &host_info, sizeof(struct emcom_host_info));
}
