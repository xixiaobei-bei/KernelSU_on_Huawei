/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: mpdns module implemention
 * Author: shishuai shishuai.shi@huawei.com
 * Create: 2020-05-20
 */
#ifndef __EMCOM_MPDNS_H__
#define __EMCOM_MPDNS_H__

#include <emcom/emcom_xengine.h>

#define EMCOM_MAX_HOST_LEN  60
#define EMCOM_MIN_HTTP_LEN  100
#define EMCOM_MAX_HOSTS_WHILTELIST_LEN  1000
#define MAX_IPV4_ADDR_STR_LEN 15
#define MIN_IPV4_ADDR_STR_LEN 7
#define IPV4_ADDR_STR_SEG_NUM 3
#define IPV6_ADDR32_SEG_NUM 4

#define COMMON_HOST_PREFIX  "*."
#define COMMON_HOST_PREFIX_LEN  2
#define HTTP_REQUEST_GET_HOST_STR "Host: "
#define HTTP_REQUEST_ENTER_STR "\r\n"
#define IPV6_ADDR_STR_SEP  ":"
#define IPV4_ADDR_STR_SEP  "."

#define EMCOM_MPDNS_MAX_IPV4_ADDR_NUM    4
#define EMCOM_MPDNS_MAX_IPV6_ADDR_NUM    2
#define EMCOM_MPDNS_MAX_ENTRY_NUM    100

#define EMCOM_MPDNS_MAX_APP_NUM  10

typedef enum {
	EMCOM_MPDNS_APP_CONF_DELETE,
	EMCOM_MPDNS_APP_CONF_ADD,
	EMCOM_MPDNS_APP_CONF_UPDATE,
} emcom_mpdns_app_conf_act;

struct emcom_host_info {
	int uid;
	int net_type;
	int ip_type;
	char host[EMCOM_MAX_HOST_LEN];
};

struct emcom_mpdns_config {
	int uid;
	int act;
	char hosts[EMCOM_MAX_HOSTS_WHILTELIST_LEN];
};

struct emcom_mpdns_app_conf_entry {
	int uid;
	char *hosts;
};

struct emcom_mpdns_app_configs {
	struct emcom_mpdns_app_conf_entry entries[EMCOM_MPDNS_MAX_APP_NUM];
	unsigned int next_update_id;
};

struct emcom_mpdns_result {
	char host[EMCOM_MAX_HOST_LEN];
	int net_type;
	struct in_addr v4_addrs[EMCOM_MPDNS_MAX_IPV4_ADDR_NUM];
	struct in6_addr v6_addrs[EMCOM_MPDNS_MAX_IPV6_ADDR_NUM];
};

struct emcom_mpdns_net_addrs {
	struct in_addr ipv4_addrs[EMCOM_MPDNS_MAX_IPV4_ADDR_NUM];
	struct in6_addr ipv6_addrs[EMCOM_MPDNS_MAX_IPV6_ADDR_NUM];
	unsigned long expire_time;
	struct in_addr ipv4_reverse_addrs[EMCOM_MPDNS_MAX_IPV4_ADDR_NUM];
	unsigned int v4_raddr_update_id;
	struct in6_addr ipv6_reverse_addrs[EMCOM_MPDNS_MAX_IPV6_ADDR_NUM];
	unsigned int v6_raddr_update_id;
};

struct emcom_mpdns_entry {
	char host[EMCOM_MAX_HOST_LEN];    // need to restrict it's lengh
	struct emcom_mpdns_net_addrs net_addrs[EMCOM_XENGINE_NET_MAX_NUM];  // index is net iface No
};

struct emcom_mpdns_cache {
	struct emcom_mpdns_entry entries[EMCOM_MPDNS_MAX_ENTRY_NUM];
	unsigned int next_update_id;
};

void emcom_mpdns_init(void);
void emcom_mpdns_flush_cache(int net_type);
void emcom_mpdns_update_dst_addr(struct sock *sk, struct sockaddr *uaddr, int cur_net, int tar_net);
void emcom_mpdns_set_app_config(struct emcom_mpdns_config *mpdns_config);
bool emcom_mpdns_is_allowed(uid_t uid, const char *host);
void emcom_mpdns_update_cache(struct emcom_mpdns_result *mpdns_result);
bool emcom_mpdns_supply_v4_cache(const char *host, struct in_addr *v4_addr, int cur_net);
bool emcom_mpdns_supply_v6_cache(const char *host, struct in6_addr *v6_addr, int cur_net);
void emcom_mpdns_clear_v4_addr(struct in_addr *addr, int net_type);
void emcom_mpdns_clear_v6_addr(struct in6_addr *addr, int net_type);
bool emcom_is_alp_host(const char *host);
bool emcom_parse_httpget_host(const unsigned char *req, char *host_buf, unsigned int host_len);
void emcom_mpdns_host_send(uid_t uid, int net_type, int ip_type, const char *host);
#endif
