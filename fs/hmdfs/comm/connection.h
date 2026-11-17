/* SPDX-License-Identifier: GPL-2.0 */
/*
 * connection.h
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Description: send and recv msg  for  hmdfs
 * Author: liuxuesong3@huawei.com
 * Create: 2020-04-14
 *
 */

#ifndef HMDFS_CONNECTION_H
#define HMDFS_CONNECTION_H

#ifdef CONFIG_HMDFS_CRYPTO
#include <linux/tls.h>
#endif

#include <crypto/aead.h>
#include <net/sock.h>
#include "protocol.h"
#include "node_cb.h"

#define HMDFS_KEY_SIZE	  32
#define HMDFS_IV_SIZE	  12
#define HMDFS_TAG_SIZE	  16
#define HMDFS_CID_SIZE	  64
#define DEVICE_IID_LENGTH 21

enum {
	CONNECT_MESG_HANDSHAKE_REQUEST = 1,
	CONNECT_MESG_HANDSHAKE_RESPONSE = 2,
	CONNECT_MESG_HANDSHAKE_ACK = 3,
};

enum {
	CONNECT_STAT_WAIT_REQUEST = 0,
	CONNECT_STAT_WAIT_RESPONSE,
	CONNECT_STAT_WORKING,
	CONNECT_STAT_STOP,
	CONNECT_STAT_WAIT_ACK,
	CONNECT_STAT_NEGO_FAIL,
	CONNECT_STAT_COUNT
};

enum {
	CONNECT_TYPE_TCP = 0,
	CONNECT_TYPE_UNSUPPORT,
};

struct connection_stat {
	int64_t send_bytes;
	int64_t recv_bytes;
	int send_message_count;
	int recv_message_count;
	unsigned long rekey_time;
};

struct connection {
	struct list_head list;
	struct kref ref_cnt;
	struct mutex ref_lock;
	struct hmdfs_peer *node;
	int type;
	int status;
	void *connect_handle;
	struct crypto_aead *tfm;
	u8 master_key[HMDFS_KEY_SIZE];
	u8 send_key[HMDFS_KEY_SIZE];
	u8 recv_key[HMDFS_KEY_SIZE];
	struct connection_stat stat;
	struct work_struct reget_work;
#ifdef CONFIG_HMDFS_CRYPTO
	struct tls12_crypto_info_aes_gcm_128 send_crypto_info;
	struct tls12_crypto_info_aes_gcm_128 recv_crypto_info;
#endif
	void (*close)(struct connection *connect);
	int (*send_message)(struct connection *connect,
			    struct hmdfs_send_data *msg);
	uint32_t crypto;
};

enum {
	NODE_STAT_SHAKING = 0,
	NODE_STAT_ONLINE,
	NODE_STAT_OFFLINE,
};

struct hmdfs_async_work {
	struct hmdfs_msg_idr_head head;
	struct page *page;
	struct delayed_work d_work;
	unsigned long start;
};

enum {
	RAW_NODE_EVT_OFF = 0,
	RAW_NODE_EVT_ON,
	RAW_NODE_EVT_NR,
};

#define RAW_NODE_EVT_MAX_NR 4

struct hmdfs_stash_statistics {
	unsigned int cur_ok;
	unsigned int cur_nothing;
	unsigned int cur_fail;
	unsigned int total_ok;
	unsigned int total_nothing;
	unsigned int total_fail;
	unsigned long long ok_pages;
	unsigned long long fail_pages;
};

struct hmdfs_restore_statistics {
	unsigned int cur_ok;
	unsigned int cur_fail;
	unsigned int cur_keep;
	unsigned int total_ok;
	unsigned int total_fail;
	unsigned int total_keep;
	unsigned long long ok_pages;
	unsigned long long fail_pages;
};

struct hmdfs_rebuild_statistics {
	unsigned int cur_ok;
	unsigned int cur_fail;
	unsigned int cur_invalid;
	unsigned int total_ok;
	unsigned int total_fail;
	unsigned int total_invalid;
	unsigned int time;
};

struct hmdfs_peer_statistics {
	/* stash statistics */
	struct hmdfs_stash_statistics stash;
	/* restore statistics */
	struct hmdfs_restore_statistics restore;
	/* rebuild statistics */
	struct hmdfs_rebuild_statistics rebuild;
};

struct hmdfs_peer {
	struct list_head list;
	struct kref ref_cnt;
	unsigned int owner;
	uint64_t device_id;
	uint64_t iid;
	unsigned long conn_time;
	uint8_t version;
	u8 status;
	u64 features;
	long long old_sb_dirty_count;
	atomic64_t sb_dirty_count;
	/*
	 * cookie for opened file id.
	 * It will be increased if peer has offlined
	 */
	uint16_t fid_cookie;
	struct mutex conn_impl_list_lock;
	struct list_head conn_impl_list;
	/*
	 * when async message process context call hmdfs_reget_connection
	 * add conn node to conn_deleting_list, so call hmdfs_disconnect_node
	 * can wait all receive thread exit
	 */
	struct list_head conn_deleting_list;
	wait_queue_head_t deleting_list_wq;
	struct idr msg_idr;
	spinlock_t idr_lock;
	struct idr file_id_idr;
	spinlock_t file_id_lock;
	int recvbuf_maxsize;
	struct crypto_aead *tfm;
	char cid[HMDFS_CID_SIZE + 1];
	const struct connection_operations *conn_operations;
	struct hmdfs_sb_info *sbi;
	struct workqueue_struct *async_wq;
	struct workqueue_struct *req_handle_wq;
	struct workqueue_struct *dentry_wq;
	struct workqueue_struct *retry_wb_wq;
	struct workqueue_struct *reget_conn_wq;
	atomic_t evt_seq;
	/* sync cb may be blocking */
	struct mutex seq_lock;
	struct mutex offline_cb_lock;
	struct mutex evt_lock;
	unsigned char pending_evt;
	unsigned char last_evt;
	unsigned char waiting_evt[RAW_NODE_EVT_NR];
	unsigned char seq_rd_idx;
	unsigned char seq_wr_idx;
	unsigned int seq_tbl[RAW_NODE_EVT_MAX_NR];
	unsigned int pending_evt_seq;
	unsigned char cur_evt[NODE_EVT_TYPE_NR];
	unsigned int cur_evt_seq[NODE_EVT_TYPE_NR];
	unsigned int merged_evt;
	unsigned int dup_evt[RAW_NODE_EVT_NR];
	struct delayed_work evt_dwork;
	/* protected by idr_lock */
	uint64_t msg_idr_process;
	bool offline_start;
	spinlock_t wr_opened_inode_lock;
	struct list_head wr_opened_inode_list;
	/*
	 * protect @stashed_inode_list and @stashed_inode_nr in stash process
	 * and fill_inode_remote->hmdfs_remote_init_stash_status process
	 */
	spinlock_t stashed_inode_lock;
	unsigned int stashed_inode_nr;
	struct list_head stashed_inode_list;
	bool need_rebuild_stash_list;
	/* how many inodes are rebuilding statsh status */
	atomic_t rebuild_inode_status_nr;
	wait_queue_head_t rebuild_inode_status_wq;
	struct hmdfs_peer_statistics stats;
	/* sysfs */
	struct kobject kobj;
	struct completion kobj_unregister;
};

#define HMDFS_DEVID_LOCAL 0

/* Be Compatible to DFS1.0, dont add packed attribute so far */
struct connection_msg_head {
	__u8 magic;
	__u8 version;
	__u8 operations;
	__u8 flags;
	__le32 datasize;
	__le64 source;
	__le16 msg_id;
	__le16 request_id;
};

struct connection_echo_param {
	__le64 iid;
	__le64 length;
} __packed;

struct connection_handshake_req {
	__le32 len;
	char dev_id[0];
} __packed;

enum {
	HS_EXTEND_CODE_CRYPTO = 0,
	HS_EXTEND_CODE_CASE_SENSE,
	HS_EXTEND_CODE_FEATURE_SUPPORT,
	HS_EXTEND_CODE_COUNT
};

struct conn_hs_extend_reg {
	__u16 len;
	__u16 resv;
	void (*filler)(struct connection *conn_impl, __u8 ops,
			       void *data, __u32 len);
	int (*parser)(struct connection *conn_impl, __u8 ops,
				void *data, __u32 len);
};

struct conn_hs_extend_head {
	__le32 field_cn;
	char data[0];
};

struct extend_field_head {
	__le16 code;
	__le16 len;
} __packed;

struct crypto_body {
	__le32 crypto;
} __packed;

struct case_sense_body {
	__u8 case_sensitive;
} __packed;

struct feature_body {
	__u64 features;
	__u64 reserved;
} __packed;

#define HMDFS_HS_CRYPTO_KTLS_AES128 0x00000001
#define HMDFS_HS_CRYPTO_KTLS_AES256 0x00000002

static inline bool hmdfs_is_node_online(const struct hmdfs_peer *node)
{
	return READ_ONCE(node->status) == NODE_STAT_ONLINE;
}

static inline unsigned int hmdfs_node_inc_evt_seq(struct hmdfs_peer *node)
{
	/* Use the atomic as an unsigned integer */
	return atomic_inc_return(&node->evt_seq);
}

static inline unsigned int hmdfs_node_evt_seq(const struct hmdfs_peer *node)
{
	return atomic_read(&node->evt_seq);
}

struct connection *get_conn_impl(struct hmdfs_peer *node, int connect_type);

void set_conn_sock_quickack(struct hmdfs_peer *node);

struct hmdfs_peer *hmdfs_get_peer(struct hmdfs_sb_info *sbi, uint8_t *cid,
				  uint64_t iid);

struct hmdfs_peer *hmdfs_lookup_from_devid(struct hmdfs_sb_info *sbi,
					   uint64_t device_id);
struct hmdfs_peer *hmdfs_lookup_from_cid(struct hmdfs_sb_info *sbi,
					 uint8_t *cid);
void connection_send_handshake(struct connection *conn_impl, __u8 operations,
			       __le16 request_id);
void connection_handshake_recv_handler(struct connection *conn_impl, void *buf,
				       void *data, __u32 data_len);
void connection_working_recv_handler(struct connection *conn_impl, void *head,
				     void *data, __u32 data_len);
static inline void connection_get(struct connection *conn)
{
	kref_get(&conn->ref_cnt);
}

void connection_put(struct connection *conn);
static inline void peer_get(struct hmdfs_peer *peer)
{
	kref_get(&peer->ref_cnt);
}

void peer_put(struct hmdfs_peer *peer);

int hmdfs_sendmessage(struct hmdfs_peer *node, struct hmdfs_send_data *msg);
void hmdfs_connections_stop(struct hmdfs_sb_info *sbi);

void hmdfs_disconnect_node(struct hmdfs_peer *node);
void connection_send_echo_async_cb(struct hmdfs_peer *peer,
				   const struct hmdfs_req *req,
				   const struct hmdfs_resp *resp);
void connection_send_echo(struct hmdfs_peer *con);
void connection_recv_echo(struct hmdfs_peer *con, struct hmdfs_head_cmd *head,
			  void *recv);
void connection_to_working(struct hmdfs_peer *node);

int hmdfs_alloc_msg_idr(struct hmdfs_peer *peer, enum MSG_IDR_TYPE type,
			void *ptr);
struct hmdfs_msg_idr_head *hmdfs_find_msg_head(struct hmdfs_peer *peer, int id);

static inline void hmdfs_start_process_offline(struct hmdfs_peer *peer)
{
	spin_lock(&peer->idr_lock);
	peer->offline_start = true;
	spin_unlock(&peer->idr_lock);
}

static inline void hmdfs_stop_process_offline(struct hmdfs_peer *peer)
{
	spin_lock(&peer->idr_lock);
	peer->offline_start = false;
	spin_unlock(&peer->idr_lock);
}

static inline void hmdfs_dec_msg_idr_process(struct hmdfs_peer *peer)
{
	spin_lock(&peer->idr_lock);
	peer->msg_idr_process--;
	spin_unlock(&peer->idr_lock);
}
#endif
