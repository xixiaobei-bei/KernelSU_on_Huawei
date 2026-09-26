/* SPDX-License-Identifier: GPL-2.0 */
/*
 * inode.c
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 * Author: chenjinglong1@huawei.com
 * Create: 2021-01-19
 *
 */

#include "inode.h"
#include "comm/connection.h"

/**
 * Rules to generate inode numbers:
 *
 * "/", "/device_view", "/merge_view", "/device_view/local", "/device_view/cid"
 * = DOMAIN {3} : dev_id {29} : HMDFS_ROOT {32}
 *
 * "/device_view/cid/xxx"
 * = DOMAIN {3} : dev_id {29} : hash(remote_ino){32}
 *
 * "/merge_view/xxx"
 * = DOMAIN {3} : lower's dev_id {29} : lower's ino_raw {32}
 */

#define BIT_WIDE_TOTAL 64

#define BIT_WIDE_DOMAIN 3
#define BIT_WIDE_DEVID 29
#define BIT_WIDE_INO_RAW 32

#if BIT_WIDE_TOTAL != BIT_WIDE_DOMAIN + BIT_WIDE_DEVID + BIT_WIDE_INO_RAW
#error Bit wides unmatched
#endif

enum DOMAIN {
	DOMAIN_ROOT,
	DOMAIN_DEVICE_LOCAL,
	DOMAIN_DEVICE_REMOTE,
	DOMAIN_MERGE_VIEW,
	DOMAIN_DFS_1_0,

	DOMAIN_INVALID,
};

union hmdfs_ino {
	const uint64_t ino_output;
	struct {
		uint64_t ino_raw : BIT_WIDE_INO_RAW;
		uint64_t dev_id : BIT_WIDE_DEVID;
		uint8_t domain : BIT_WIDE_DOMAIN;
	};
};

static uint8_t read_ino_domain(uint64_t ino)
{
	union hmdfs_ino _ino = {
		.ino_output = ino,
	};

	return _ino.domain;
}

static uint64_t read_ino_dev_id(uint64_t ino)
{
	union hmdfs_ino _ino = {
		.ino_output = ino,
	};

	return _ino.dev_id;
}

static uint64_t read_ino_raw(uint64_t ino)
{
	union hmdfs_ino _ino = {
		.ino_output = ino,
	};

	return _ino.ino_raw;
}

struct iget_args {
	/* The lower inode of local/merge/root(part) inode */
	struct inode *lo_i;
	/* The peer of remote inode */
	struct hmdfs_peer *peer;
	/* The ino of remote inode */
	uint64_t remote_ino;

	/* Returned inode's ino */
	union hmdfs_ino ino;
};

/**
 * iget_test - whether or not the inode with matched hashval is the one we are
 * looking for
 *
 * @inode: the local inode we found in inode cache with matched hashval
 * @data: struct iget_args
 */
static int iget_test(struct inode *inode, void *data)
{
	struct hmdfs_inode_info *hii = hmdfs_i(inode);
	struct iget_args *ia = data;
	int res = 0;

	WARN_ON(ia->ino.domain < DOMAIN_ROOT ||
		ia->ino.domain >= DOMAIN_INVALID);

	if (read_ino_domain(inode->i_ino) == DOMAIN_DFS_1_0 ||
	    read_ino_domain(inode->i_ino) == DOMAIN_ROOT)
		return 0;
	if (read_ino_domain(inode->i_ino) != ia->ino.domain)
		return 0;

	switch (ia->ino.domain) {
	case DOMAIN_MERGE_VIEW:
		res = (ia->lo_i == hii->lower_inode);
		break;
	case DOMAIN_DEVICE_LOCAL:
		res = (ia->lo_i == hii->lower_inode);
		break;
	case DOMAIN_DEVICE_REMOTE:
		res = (ia->peer == hii->conn &&
		       ia->remote_ino == hii->remote_ino);
		break;
	}

	return res;
}

/**
 * iget_set - initialize a inode with iget_args
 *
 * @sb: the superblock of current hmdfs instance
 * @data: struct iget_args
 */
static int iget_set(struct inode *inode, void *data)
{
	struct hmdfs_inode_info *hii = hmdfs_i(inode);
	struct iget_args *ia = (struct iget_args *)data;

	inode->i_ino = ia->ino.ino_output;
	inode_inc_iversion(inode);

	hii->conn = ia->peer;
	hii->remote_ino = ia->remote_ino;
	hii->lower_inode = ia->lo_i;

	return 0;
}

static uint64_t make_ino_raw_dev_local(uint64_t lo_ino)
{
	if (!(lo_ino >> BIT_WIDE_INO_RAW))
		return lo_ino;

	return lo_ino * GOLDEN_RATIO_64 >> BIT_WIDE_INO_RAW;
}

static uint64_t make_ino_raw_dev_remote(uint64_t remote_ino)
{
	return hash_long(remote_ino, BIT_WIDE_INO_RAW);
}

/**
 * hmdfs_iget5_locked_merge - obtain an inode for the merge-view
 *
 * @sb: superblock of current instance
 * @fst_lo_i: the lower inode of it's first comrade
 *
 * Simply replace the lower's domain for a new ino.
 */
struct inode *hmdfs_iget5_locked_merge(struct super_block *sb,
				       struct inode *fst_lo_i)
{
	struct iget_args ia = {
		.lo_i = fst_lo_i,
		.peer = NULL,
		.remote_ino = 0,
		.ino.ino_output = 0,
	};

	if (unlikely(!fst_lo_i)) {
		hmdfs_err("Received a invalid lower inode");
		return NULL;
	}

	ia.ino.ino_raw = read_ino_raw(fst_lo_i->i_ino);
	ia.ino.dev_id = read_ino_dev_id(fst_lo_i->i_ino);
	ia.ino.domain = DOMAIN_MERGE_VIEW;
	return iget5_locked(sb, ia.ino.ino_output, iget_test, iget_set, &ia);
}

/**
 * hmdfs_iget5_locked_local - obtain an inode for the the local-dev-view
 *
 * @sb: superblock of current instance
 * @lo_i: the lower inode from local filesystem
 *
 * Hashing local inode's ino to generate our ino. We continue to compare the
 * address of the lower_inode for uniqueness when collisions occured.
 */
struct inode *hmdfs_iget5_locked_local(struct super_block *sb,
				       struct inode *lo_i)
{
	struct iget_args ia = {
		.lo_i = lo_i,
		.peer = NULL,
		.remote_ino = 0,
		.ino.ino_output = 0,
	};

	if (unlikely(!lo_i)) {
		hmdfs_err("Received a invalid lower inode");
		return NULL;
	}
	ia.ino.ino_raw = make_ino_raw_dev_local(lo_i->i_ino);
	ia.ino.dev_id = 0;
	ia.ino.domain = DOMAIN_DEVICE_LOCAL;
	return iget5_locked(sb, ia.ino.ino_output, iget_test, iget_set, &ia);
}

/**
 * hmdfs_iget5_locked_remote - obtain an inode for the the remote-dev-view
 *
 * @sb: superblock of current instance
 * @peer: corresponding device node
 * @remote_ino: remote inode's ino
 *
 * Hash remote ino for ino's 32bit~1bit.
 *
 * Note that currenly implementation assume the each remote inode has unique
 * ino. Thus the combination of the peer's unique dev_id and the remote_ino
 * is enough to determine a unique remote inode.
 */
struct inode *hmdfs_iget5_locked_remote(struct super_block *sb,
					struct hmdfs_peer *peer,
					uint64_t remote_ino)
{
	struct iget_args ia = {
		.lo_i = NULL,
		.peer = peer,
		.remote_ino = remote_ino,
		.ino.ino_output = 0,
	};

	if (unlikely(!peer)) {
		hmdfs_err("Received a invalid peer");
		return NULL;
	}

	ia.ino.ino_raw = make_ino_raw_dev_remote(remote_ino);
	ia.ino.dev_id = peer->device_id;
	ia.ino.domain = DOMAIN_DEVICE_REMOTE;
	return iget5_locked(sb, ia.ino.ino_output, iget_test, iget_set, &ia);
}

static struct inode *iget_locked_internal(struct super_block *sb,
					  struct iget_args *ia)
{
	struct inode *inode = iget_locked(sb, ia->ino.ino_output);

	if (inode && (inode->i_state & I_NEW))
		iget_set(inode, ia);
	return inode;
}

struct inode *hmdfs_iget_locked_root(struct super_block *sb, uint64_t root_ino,
				     struct inode *lo_i,
				     struct hmdfs_peer *peer)
{
	struct iget_args ia = {
		.lo_i = lo_i,
		.peer = peer,
		.remote_ino = 0,
		.ino.ino_raw = root_ino,
		.ino.dev_id = peer ? peer->device_id : 0,
		.ino.domain = DOMAIN_ROOT,
	};

	if (unlikely(root_ino < 0 || root_ino >= HMDFS_ROOT_INVALID)) {
		hmdfs_err("Root %llu is invalid", root_ino);
		return NULL;
	}
	if (unlikely(root_ino == HMDFS_ROOT_DEV_REMOTE && !peer)) {
		hmdfs_err("Root %llu received a invalid peer", root_ino);
		return NULL;
	}

	return iget_locked_internal(sb, &ia);
}

struct inode *hmdfs_iget_locked_dfs_1_0(struct super_block *sb,
					struct hmdfs_peer *peer)
{
	static atomic64_t i_seed;
	struct iget_args ia = {
		.lo_i = NULL,
		.peer = peer,
		.remote_ino = 0,
		.ino.ino_raw = atomic64_inc_return(&i_seed),
		.ino.dev_id = peer ? peer->device_id : 0,
		.ino.domain = DOMAIN_DFS_1_0,
	};

	if (unlikely(!peer)) {
		hmdfs_err("Received a invalid peer");
		return NULL;
	}

	return iget_locked_internal(sb, &ia);
}

/**
 * hmdfs_inode_type - to parse an inode's type from its ino
 *
 * @inode: inode from hmdfs
 */
int hmdfs_inode_type(struct inode *inode)
{
	int ret = HMDFS_LAYER_INVALID;
	union hmdfs_ino _ino = {
		.ino_output = inode->i_ino,
	};

	if (!_ino.domain) {
		switch (_ino.ino_raw) {
		case HMDFS_ROOT_ANCESTOR:
			ret = HMDFS_LAYER_ZERO;
			break;
		case HMDFS_ROOT_DEV:
			ret = HMDFS_LAYER_FIRST_DEVICE;
			break;
		case HMDFS_ROOT_DEV_LOCAL:
			ret = HMDFS_LAYER_SECOND_LOCAL;
			break;
		case HMDFS_ROOT_DEV_REMOTE:
			ret = HMDFS_LAYER_SECOND_REMOTE;
			break;
		case HMDFS_ROOT_MERGE:
			ret = HMDFS_LAYER_FIRST_MERGE;
			break;
		}
	} else {
		switch (_ino.domain) {
		case DOMAIN_DEVICE_LOCAL:
			ret = HMDFS_LAYER_OTHER_LOCAL;
			break;
		case DOMAIN_DEVICE_REMOTE:
			ret = HMDFS_LAYER_OTHER_REMOTE;
			break;
		case DOMAIN_MERGE_VIEW:
			ret = HMDFS_LAYER_OTHER_MERGE;
			break;
		case DOMAIN_DFS_1_0:
			ret = HMDFS_LAYER_DFS_1_0;
			break;
		}
	}

	WARN_ON(ret == HMDFS_LAYER_INVALID);
	return ret;
}
