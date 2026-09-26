/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: Bastet extern method.
 * Author: guningyi@huawei.com
 * Create: 2019-08-20
 */

#ifndef BASTET_INTERFACE_H
#define BASTET_INTERFACE_H

void bastet_sync_prop_cancel(struct sock *sk);
void set_channel_occupied(void);
void clear_channel_occupied(void);
extern void ind_hisi_com(const void *info, u32 len);
extern void ind_modem_reset(uint8_t *value, uint32_t len);

#ifdef CONFIG_HUAWEI_EMCOM
extern void Emcom_Ind_Modem_Support(u8 ucState);
#endif

extern void bastet_traffic_flow_init(void);
extern void bastet_utils_init(void);
extern void bastet_utils_exit(void);

#ifdef CONFIG_HUAWEI_BASTET_COMM
int bastet_comm_init(void);
int bastet_comm_write(u8 *msg, u32 len, u32 type);
int get_modem_rab_id(struct bst_modem_rab_id *info);
int get_ipv6_modem_rab_id(struct bst_modem_rab_id *info);
#endif

int start_bastet_sock(struct bst_set_sock_sync_delay *init_prop);
int stop_bastet_sock(struct bst_sock_id *guide);
int prepare_bastet_sock(struct bst_set_sock_sync_delay *sync_prop);
int get_tcp_sock_comm_prop(struct bst_get_sock_comm_prop *get_prop);
int set_tcp_sock_sync_prop(struct bst_set_sock_sync_prop *set_prop);
int set_tcp_sock_closed(struct bst_sock_comm_prop *guide);
int get_tcp_bastet_sock_state(struct bst_get_bastet_sock_state *get_prop);
extern int adjust_traffic_flow_by_pkg(uint8_t *buf, int cnt);
extern void bastet_wake_up_traffic_flow(void);
extern int set_current_net_device_name(const char *iface);
extern void bastet_reconn_failed(struct bst_sock_comm_prop prop);
extern int bastet_reconn_proxyid_set(struct reconn_id id);
extern int bastet_sync_prop_start(struct bst_set_sock_sync_prop *set_prop);
extern int bastet_sync_prop_stop(struct bst_sock_comm_prop *comm_prop);
extern int set_sock_sync_hold_time(struct bst_set_sock_sync_delay hold_delay);
extern void clear_channel_occupied(void);
extern int bastet_partner_process_cmd(struct app_monitor_prop *prop,
	int32_t *tids);
extern int bastet_partner_init(void);
extern void bastet_partner_release(void);
extern void bastet_partner_clear(void);
extern int set_proc_info(struct set_process_info *info);
extern int set_special_uid(int32_t uid);
extern int set_netfilter(bool state);
extern void bastet_filter_init(void);
extern int get_sock_net_dev_name(struct get_netdev_name *dev_name);
extern int bastet_send_msg(struct bst_sock_id *guide,
	uint8_t *data, uint32_t len);
extern int bastet_set_hb_reply(struct bst_sock_id *guide,
	uint8_t *data, uint32_t len);
extern int bastet_filter_hb_reply(struct bst_sock_id *guide);
extern int bastet_set_freezer(struct freezer_state freezer);

extern int set_tcp_sock_sync_prop_ipv6(
	struct bst_set_sock_sync_prop_ipv6 *set_prop);
extern int bastet_sync_prop_start_ipv6(
	struct bst_set_sock_sync_prop_ipv6 *set_prop);
extern int bastet_sync_prop_stop_ipv6(
	struct bst_sock_comm_prop_ipv6 *comm_prop);
extern int get_tcp_sock_comm_prop_ipv6(
	struct bst_get_sock_comm_prop_ipv6 *get_prop);
extern int set_tcp_sock_closed_ipv6(
	struct bst_sock_comm_prop_ipv6 *guide);

extern bool bastet_dev_en;
extern atomic_t proxy;
extern atomic_t buffer;
extern atomic_t cp_reset;
extern atomic_t channel_en;
extern uid_t hrt_uid;
int adjust_traffic_flow_by_sock(struct sock *sk,
	unsigned long tx, unsigned long rx);
#endif
