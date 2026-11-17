/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: Bastet head file.
 * Author: zhuweichen@huawei.com
 * Create: 2014-09-01
 */

#ifndef _BASTET_UTILS_H
#define _BASTET_UTILS_H

#include "bastet_dev.h"

#define BASTET_LOG_TAG "Bastet"

#define BASTET_DEBUG 0
#define BASTET_INFO 1
#define IPV4_ADDR_LENGTH 4

#define bastet_logd(fmt, ...) do { if (BASTET_DEBUG) { \
	pr_debug("["BASTET_LOG_TAG"] %s: "fmt"\n", __func__, ##__VA_ARGS__); \
		} } while (0)

#define bastet_logi(fmt, ...) do { if (BASTET_INFO) { \
	pr_info("["BASTET_LOG_TAG"] %s: "fmt"\n", __func__, ##__VA_ARGS__); \
		} } while (0)

#define bastet_loge(fmt, ...) do { \
	pr_err("["BASTET_LOG_TAG"] %s: "fmt"\n", __func__, ##__VA_ARGS__); \
} while (0)

extern char cur_netdev_name[IFNAMSIZ];

struct socket *sockfd_lookup_by_fd_pid(int fd, pid_t pid, int *err);
struct sock *get_sock_by_fd_pid(int fd, pid_t pid);
int bastet_get_comm_prop(struct sock *sk, struct bst_sock_comm_prop *prop);
void bastet_get_sock_prop(struct sock *sk, struct bst_sock_sync_prop *prop);
struct sock *get_sock_by_comm_prop(struct bst_sock_comm_prop *guide);
void bastet_wakelock_acquire(void);
void bastet_wakelock_acquire_timeout(long timeout);
void bastet_wakelock_release(void);
int post_indicate_packet(enum bst_ind_type ind_type,
	const void *info, u32 len);
void ind_hisi_com(const void *info, u32 len);
bool is_uid_valid(__u32 uid);
bool get_bastet_of_status(void);
bool is_bastet_enabled(void);
bool is_proxy_available(void);
bool is_buffer_available(void);
bool is_channel_available(void);
bool is_channel_enable(void);
bool is_cp_reset(void);
bool is_sock_foreground(struct sock *sk);
bool is_wifi_proxy(struct sock *sk);
int get_bastet_sock_state(struct sock *sk);
int get_fd_by_addr(struct addr_to_fd *guide);
int get_pid_cmdline(struct get_cmdline *cmdline);
int bastet_get_comm_prop_ipv6(struct sock *sk,
	struct bst_sock_comm_prop_ipv6 *prop);
struct sock *get_sock_by_comm_prop_ipv6(
	struct bst_sock_comm_prop_ipv6 *guide);
void print_bastet_sock_seq(struct bst_sock_sync_prop *prop);
bool is_ipv6_addr(struct sock *sk);

#endif /* _BASTET_UTILS_H */
