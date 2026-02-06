// SPDX-License-Identifier: GPL-2.0
/*
 * fs/f2fs/hmfs.h
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 */
#ifndef _LINUX_F2FS_H
#define _LINUX_F2FS_H

#include <linux/uio.h>
#include <linux/types.h>
#include <linux/page-flags.h>
#include <linux/buffer_head.h>
#include <linux/slab.h>
#include <linux/crc32.h>
#include <linux/magic.h>
#include <linux/kobject.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/vmalloc.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/quotaops.h>
#include <crypto/hash.h>
#include <linux/overflow.h>
#include <linux/math64.h>

#define __FS_HAS_ENCRYPTION IS_ENABLED(CONFIG_HMFS_FS_ENCRYPTION)
#include <linux/fscrypt.h>
#include <linux/reboot.h>

#include "sdp.h"

#ifdef CONFIG_HUAWEI_HMFS_DSM
#include <dsm/dsm_pub.h>
extern struct dsm_client *hmfs_dclient;
#endif

#ifdef CONFIG_HMFS_CHECK_FS

#ifdef CONFIG_HUAWEI_HMFS_DSM
#define f2fs_bug_on(sbi, condition) \
	do { \
		if (unlikely(condition)) { \
			print_bdev_access_info(); \
			if(hmfs_dclient && !dsm_client_ocuppy(hmfs_dclient)) \
			{ \
				dsm_client_record(hmfs_dclient,"HMFS bug: %s:%d\n", __func__, __LINE__); \
				dsm_client_notify(hmfs_dclient, DSM_HMFS_NEED_FSCK); \
			} \
			hmfs_print_raw_sb_info(sbi); \
			hmfs_print_ckpt_info(sbi); \
			hmfs_print_sbi_info(sbi); \
		} \
		if (likely(sbi && ((struct f2fs_sb_info *)sbi)->print_sbi_safe)) { \
			BUG_ON(condition); \
		} else { \
			WARN_ON(condition); \
		} \
	} while (0)
#else
#define f2fs_bug_on(sbi, condition) \
	do { \
		if (likely(sbi && ((struct f2fs_sb_info *)sbi)->print_sbi_safe)) { \
			BUG_ON(condition); \
		} else { \
			WARN_ON(condition); \
		} \
	} while (0)
#endif
#define f2fs_bug_on_atomic(sbi, condition) f2fs_bug_on(sbi, condition)

#else
struct need_fsck_work_struct {
        struct f2fs_sb_info *sbi;
        struct work_struct work;
};
#ifdef CONFIG_HUAWEI_HMFS_DSM
#define f2fs_bug_on(sbi, condition)					\
	do {								\
		if (unlikely(condition)) {				\
			print_bdev_access_info(); \
			if(hmfs_dclient && !dsm_client_ocuppy(hmfs_dclient)) \
			{ \
				dsm_client_record(hmfs_dclient,"HMFS bug: %s:%d\n", __func__, __LINE__); \
				dsm_client_notify(hmfs_dclient, DSM_HMFS_NEED_FSCK); \
			} \
			hmfs_print_raw_sb_info(sbi); \
			hmfs_print_ckpt_info(sbi); \
			hmfs_print_sbi_info(sbi); \
			WARN_ON(1);					\
			set_sbi_flag(sbi, SBI_NEED_FSCK);		\
		}							\
	} while (0)
#else
#define f2fs_bug_on(sbi, condition)					\
	do {								\
		if (unlikely(condition)) {				\
			print_bdev_access_info();			\
			WARN_ON(1);					\
			set_sbi_flag(sbi, SBI_NEED_FSCK);		\
		}							\
	} while (0)
#endif
#endif

#ifdef CONFIG_HUAWEI_HMFS_DSM
#define hmfs_set_need_fsck_report()					\
	do {								\
		if (hmfs_dclient && !dsm_client_ocuppy(hmfs_dclient)) {  \
			dsm_client_record(hmfs_dclient,			\
				"SBI_NEED_FSCK:%s:%d\n", __func__, __LINE__); \
			dsm_client_notify(hmfs_dclient,			\
				DSM_HMFS_SET_SBI_NEED_FSCK);		\
		}							\
	} while (0)
#else
#define hmfs_set_need_fsck_report() do { } while (0)
#endif

#define f2fs_restart() do { \
	if (system_state > SYSTEM_RUNNING) \
		WARN_ON(1); \
	else \
		BUG_ON(1); \
} while (0)

enum {
	FAULT_KMALLOC,
	FAULT_KVMALLOC,
	FAULT_PAGE_ALLOC,
	FAULT_PAGE_GET,
	FAULT_ALLOC_BIO,
	FAULT_ALLOC_NID,
	FAULT_ORPHAN,
	FAULT_BLOCK,
	FAULT_DIR_DEPTH,
	FAULT_EVICT_INODE,
	FAULT_TRUNCATE,
	FAULT_READ_IO,
	FAULT_CHECKPOINT,
	FAULT_DISCARD,
	FAULT_WRITE_IO,
	FAULT_MAX,
};

#ifdef CONFIG_HMFS_FAULT_INJECTION
#define F2FS_ALL_FAULT_TYPE		((1 << FAULT_MAX) - 1)

struct f2fs_fault_info {
	atomic_t inject_ops;
	unsigned int inject_rate;
	unsigned int inject_type;
};

extern char *f2fs_fault_name[FAULT_MAX];
#define IS_FAULT_SET(fi, type) ((fi)->inject_type & (1 << (type)))
#endif

/*
 * For mount options
 */
#define F2FS_MOUNT_BG_GC		0x00000001
#define F2FS_MOUNT_DISABLE_ROLL_FORWARD	0x00000002
#define F2FS_MOUNT_DISCARD		0x00000004
#define F2FS_MOUNT_NOHEAP		0x00000008
#define F2FS_MOUNT_XATTR_USER		0x00000010
#define F2FS_MOUNT_POSIX_ACL		0x00000020
#define F2FS_MOUNT_DISABLE_EXT_IDENTIFY	0x00000040
#define F2FS_MOUNT_INLINE_XATTR		0x00000080
#define F2FS_MOUNT_INLINE_DATA		0x00000100
#define F2FS_MOUNT_INLINE_DENTRY	0x00000200
#define F2FS_MOUNT_FLUSH_MERGE		0x00000400
#define F2FS_MOUNT_NOBARRIER		0x00000800
#define F2FS_MOUNT_FASTBOOT		0x00001000
#define F2FS_MOUNT_EXTENT_CACHE		0x00002000
#define F2FS_MOUNT_FORCE_FG_GC		0x00004000
#define F2FS_MOUNT_DATA_FLUSH		0x00008000
#define F2FS_MOUNT_FAULT_INJECTION	0x00010000
#define F2FS_MOUNT_ADAPTIVE		0x00020000
#define F2FS_MOUNT_LFS			0x00040000
#define F2FS_MOUNT_USRQUOTA		0x00080000
#define F2FS_MOUNT_GRPQUOTA		0x00100000
#define F2FS_MOUNT_PRJQUOTA		0x00200000
#define F2FS_MOUNT_QUOTA		0x00400000
#define F2FS_MOUNT_INLINE_XATTR_SIZE	0x00800000
#define F2FS_MOUNT_RESERVE_ROOT		0x01000000
#define F2FS_MOUNT_DISABLE_CHECKPOINT	0x02000000

/*
 * Huawei mount options
 */
#define F2FS_MOUNT_HW_VERIFY_ENCRYPT   0x00000001
#define F2FS_MOUNT_HW_INLINE_ENCRYPT   0x00000002
#define F2FS_MOUNT_HW_FORCE_CRC        0x00000004
#define F2FS_MOUNT_HW_SDP_ENCRYPT      0x00000010
#define F2FS_MOUNT_HW_NOATGC           0x00000020
#define F2FS_MOUNT_HW_NOSUBDIVISION    0x00000040
#define F2FS_MOUNT_HW_SLC_MODE         0x00000100
#define F2FS_MOUNT_HW_DATAMOVE         0x00000080

#define F2FS_OPTION(sbi)	((sbi)->mount_opt)
#define clear_opt(sbi, option)	(F2FS_OPTION(sbi).opt &= ~F2FS_MOUNT_##option)
#define set_opt(sbi, option)	(F2FS_OPTION(sbi).opt |= F2FS_MOUNT_##option)
#define test_opt(sbi, option)	(F2FS_OPTION(sbi).opt & F2FS_MOUNT_##option)
#define clear_hw_opt(sbi, option)      (sbi->mount_opt.hw_opt &= ~F2FS_MOUNT_HW_##option)
#define set_hw_opt(sbi, option)                (sbi->mount_opt.hw_opt |= F2FS_MOUNT_HW_##option)
#define test_hw_opt(sbi, option)       (sbi->mount_opt.hw_opt & F2FS_MOUNT_HW_##option)

#define ver_after(a, b)	(typecheck(unsigned long long, a) &&		\
		typecheck(unsigned long long, b) &&			\
		((long long)((a) - (b)) > 0))

/*
 * Storage_turbo stream id
 */
enum {
	STREAM_META,	  /* meta data stream id 0x00 */
	STREAM_COLD_NODE, /* cold node stream id 0x01 */
	STREAM_COLD_DATA, /* cold data stream id 0x02 */
	STREAM_HOT_NODE,  /* hot  node stream id 0x03 */
	STREAM_HOT_DATA,  /* hot  data stream id 0x04 */
	STREAM_NR
};

enum {
	STREAM_DATA_MOVE1,
	STREAM_DATA_MOVE2
};

#define OOB_FSYNC_BIT	(1 << 31)

typedef u32 block_t;	/*
			 * should not change u32, since it is the on-disk block
			 * address format, __le32.
			 */
typedef u32 nid_t;

struct f2fs_mount_info {
	unsigned int opt;
	unsigned int    hw_opt;
	int write_io_size_bits;		/* Write IO size bits */
	block_t root_reserved_blocks;	/* root reserved blocks */
	kuid_t s_resuid;		/* reserved blocks for uid */
	kgid_t s_resgid;		/* reserved blocks for gid */
	int active_logs;		/* # of active logs */
	int inline_xattr_size;		/* inline xattr size */
#ifdef CONFIG_HMFS_FAULT_INJECTION
	struct f2fs_fault_info fault_info;	/* For fault injection */
#endif
#ifdef CONFIG_QUOTA
	/* Names of quota files with journalled quota */
	char *s_qf_names[MAXQUOTAS];
	int s_jquota_fmt;			/* Format of quota to use */
#endif
	/* For which write hints are passed down to block layer */
	int whint_mode;
	int alloc_mode;			/* segment allocation policy */
	int fsync_mode;			/* fsync policy */
	bool test_dummy_encryption;	/* test dummy encryption */
};

struct gc_loop_info {
        unsigned long count;
        unsigned int segno;
        bool check;
	unsigned long *segmap;
};
#ifdef CONFIG_HMFS_CHECK_FS
#define F2FS_GC_LOOP_MOD 10000 /* must be larger than 1 */
#define F2FS_GC_LOOP_NOMEM_MOD 50 /* must be larger than 1 */
#define hmfs_gc_loop_debug(sbi) \
        do {    \
                if (unlikely((sbi)->gc_loop.check && \
                        !((sbi)->gc_loop.count % F2FS_GC_LOOP_MOD)))    \
                        hmfs_msg((sbi)->sb, KERN_ERR, "hmfs_gc_loop aborts from %s:%d\n", __func__, __LINE__);  \
        } while (0)
#else
#define F2FS_GC_LOOP_MOD 100 /* must be larger than 1 */
#define F2FS_GC_LOOP_NOMEM_MOD 500 /* must be larger than 1 */
#define hmfs_gc_loop_debug(sbi)
#endif
#define F2FS_GC_LOOP_MAX 10000 /* must be equal or larger than F2FS_GC_LOOP_MOD */
#define init_hmfs_gc_loop(sbi) \
        do {    \
                (sbi)->gc_loop.check = false;   \
                (sbi)->gc_loop.count = 0;       \
                (sbi)->gc_loop.segno = NULL_SEGNO;      \
        } while (0)
#define IS_HMFS_GC_THREAD() (strncmp("hmfs_gc", current->comm, 7) == 0)
#define IS_CUR_GC_SEC(secno) (sbi->next_victim_seg !=	\
		NULL_SEGNO && (secno) ==	\
		GET_SEC_FROM_SEG(sbi, sbi->next_victim_seg))

#define F2FS_FEATURE_ENCRYPT		0x0001
#define F2FS_FEATURE_BLKZONED		0x0002
#define F2FS_FEATURE_ATOMIC_WRITE	0x0004
#define F2FS_FEATURE_EXTRA_ATTR		0x0008
#define F2FS_FEATURE_PRJQUOTA		0x0010
#define F2FS_FEATURE_INODE_CHKSUM	0x0020
#define F2FS_FEATURE_FLEXIBLE_INLINE_XATTR	0x0040
#define F2FS_FEATURE_QUOTA_INO		0x0080
#define F2FS_FEATURE_INODE_CRTIME	0x0100
#define F2FS_FEATURE_LOST_FOUND		0x0200
#define F2FS_FEATURE_VERITY		0x0400	/* reserved */
#define F2FS_FEATURE_SB_CHKSUM		0x0800

#define F2FS_HAS_FEATURE(sb, mask)					\
	((F2FS_SB(sb)->raw_super->feature & cpu_to_le32(mask)) != 0)
#define F2FS_SET_FEATURE(sb, mask)					\
	(F2FS_SB(sb)->raw_super->feature |= cpu_to_le32(mask))
#define F2FS_CLEAR_FEATURE(sb, mask)					\
	(F2FS_SB(sb)->raw_super->feature &= ~cpu_to_le32(mask))

/*
 * Default values for user and/or group using reserved blocks
 */
#define	F2FS_DEF_RESUID		0
#define	F2FS_DEF_RESGID		0

/*
 * extra_flags is placed at the last block of CP 0 segment
 * if no flags are set, crc must be 0
 */
struct extra_flags_block {
       __le32 need_fsck;
       __u8 reserved[4088];
       __le32 crc;
} __packed;

#define EXTRA_NEED_FSCK_FLAG   0x4653434b      // ascii of "FSCK"

/*
 * For checkpoint manager
 */
enum {
	NAT_BITMAP,
	SIT_BITMAP
};

#define	CP_UMOUNT	0x00000001
#define	CP_FASTBOOT	0x00000002
#define	CP_SYNC		0x00000004
#define	CP_RECOVERY	0x00000008
#define	CP_DISCARD	0x00000010
#define CP_TRIMMED	0x00000020
#define CP_PAUSE	0x00000040

#define MAX_DISCARD_BLOCKS(sbi)		BLKS_PER_SEC(sbi)
#define DEF_MAX_DISCARD_REQUEST		8	/* issue 8 discards per round */
#define DEF_MIN_DISCARD_ISSUE_TIME	50	/* 50 ms, if exists */
#define DEF_MID_DISCARD_ISSUE_TIME	2000	/* 2 s, if device busy */
#define DEF_MAX_DISCARD_ISSUE_TIME	60000	/* 60 s, if no candidates */
#define DEF_DISCARD_URGENT_UTIL		80	/* do more discard over 80% */
#define DEF_CP_INTERVAL			60	/* 60 secs */
#define DEF_IDLE_INTERVAL		0	/* 0 secs */
#define DEF_DISABLE_INTERVAL		5	/* 5 secs */

struct cp_control {
	int reason;
	__u64 trim_start;
	__u64 trim_end;
	__u64 trim_minlen;
};

/*
 * indicate meta/data type
 */
enum {
	META_CP,
	META_NAT,
	META_SIT,
	META_SSA,
	META_MAX,
	META_POR,
	DATA_GENERIC,
	META_GENERIC,
};

/* for the list of ino */
enum {
	ORPHAN_INO,		/* for orphan ino list */
	APPEND_INO,		/* for append ino list */
	UPDATE_INO,		/* for update ino list */
	TRANS_DIR_INO,		/* for trasactions dir ino list */
	FLUSH_INO,		/* for multiple device flushing */
	MAX_INO_ENTRY,		/* max. list */
};

struct ino_entry {
	struct list_head list;		/* list head */
	nid_t ino;			/* inode number */
	unsigned int dirty_device;	/* dirty device bitmap */
};

/* for the list of inodes to be GCed */
struct inode_entry {
	struct list_head list;	/* list head */
	struct list_head offset_list;	/* gc block offset list head */
	struct inode *inode;	/* vfs inode pointer */
};

struct fsync_node_entry {
	struct list_head list;	/* list head */
	struct page *page;	/* warm node page pointer */
	unsigned int seq_id;	/* sequence id */
};

/* for the bitmap indicate blocks to be discarded */
struct discard_entry {
	struct list_head list;	/* list head */
	block_t start_blkaddr;	/* start blockaddr of current segment */
	unsigned char discard_map[SIT_VBLOCK_MAP_SIZE];	/* segment discard bitmap */
};

struct inode_gc_entry {
	struct list_head list;	/* list head */
	int offset;				/* block offset of one segment in a file */
};

/* default discard granularity of inner discard thread, unit: block count */
#define DEFAULT_DISCARD_GRANULARITY             16
#define DISCARD_GRAN_BL                16
#define DISCARD_GRAN_BG                512
#define DISCARD_GRAN_FORCE     1

#define DISCARD_MAX_TIME	(50 * 1000 * 1000UL)
#define DISCARD_MAX_BLOCKS	(1024 * 256UL)
#define DISCARD_2M_GRAN                512

/* max discard pend list number */
#define MAX_PLIST_NUM		512
#define plist_idx(blk_num)	((blk_num) >= MAX_PLIST_NUM ?		\
					(MAX_PLIST_NUM - 1) : (blk_num - 1))
#define FS_FREE_SPACE_PERCENT          20
#define DEVICE_FREE_SPACE_PERCENT      10

enum {
	D_PREP,			/* initial */
	D_PARTIAL,		/* partially submitted */
	D_SUBMIT,		/* all submitted */
	D_DONE,			/* finished */
};

struct discard_info {
	block_t lstart;			/* logical start address */
	block_t len;			/* length */
	block_t start;			/* actual start address in dev */
};

struct discard_cmd {
	struct rb_node rb_node;		/* rb node located in rb-tree */
	union {
		struct {
			block_t lstart;	/* logical start address */
			block_t len;	/* length */
			block_t start;	/* actual start address in dev */
		};
		struct discard_info di;	/* discard info */

	};
	struct list_head list;		/* command list */
	struct completion wait;		/* compleation */
	struct block_device *bdev;	/* bdev */
	unsigned short ref;		/* reference count */
	unsigned char state;		/* state */
	unsigned char issuing;		/* issuing discard */
	int error;			/* bio error */
        u64 discard_time;
	spinlock_t lock;		/* for state/bio_ref updating */
	unsigned short bio_ref;		/* bio reference count */
};

enum {
	DPOLICY_BG,
	DPOLICY_BL,
	DPOLICY_FORCE,
	DPOLICY_FSTRIM,
	DPOLICY_UMOUNT,
	MAX_DPOLICY,
};

enum {
        SUB_POLICY_BIG,
        SUB_POLICY_MID,
        SUB_POLICY_SMALL,
        NR_SUB_POLICY,
};

struct discard_sub_policy {
        unsigned int max_requests;
        int interval;
};



struct discard_policy {
	int type;			/* type of discard */
	unsigned int granularity;       /* discard granularity */
	bool io_aware;			/* issue discard in idle time */
	unsigned int io_aware_gran;     /* minimum granularity discard not be aware of I/O */
	bool sync;			/* submit discard with REQ_SYNC flag */
	bool ordered;			/* issue discard by lba order */
        struct discard_sub_policy sub_policy[NR_SUB_POLICY];
        unsigned int min_interval;      /* used for candidates exist */
        unsigned int mid_interval;      /* used for dev is busy */
        unsigned int max_interval;      /* used for candidates not exist */
};

struct discard_cmd_control {
	struct task_struct *f2fs_issue_discard;	/* discard thread */
	struct list_head entry_list;		/* 4KB discard entry list */
	struct list_head pend_list[MAX_PLIST_NUM];/* store pending entries */
	struct list_head wait_list;		/* store on-flushing entries */
	struct list_head fstrim_list;		/* in-flight discard from fstrim */
	wait_queue_head_t discard_wait_queue;	/* waiting queue for wake-up */
	unsigned int discard_wake;		/* to wake up discard thread */
	struct mutex cmd_lock;
	unsigned int nr_discards;		/* # of discards in the list */
	unsigned int max_discards;		/* max. discards to be issued */
	unsigned int discard_granularity;	/* discard granularity */
	unsigned int undiscard_blks;		/* # of undiscard blocks */
	unsigned int next_pos;			/* next discard position */
	atomic_t issued_discard;		/* # of issued discard */
	atomic_t issing_discard;		/* # of issing discard */
	atomic_t discard_cmd_cnt;		/* # of cached cmd count */
	struct rb_root_cached root;		/* root of discard rb-tree */
	bool rbtree_check;			/* config for consistence check */
};

/* for the list of fsync inodes, used only during recovery */
struct fsync_inode_entry {
	struct list_head list;	/* list head */
	struct inode *inode;	/* vfs inode pointer */
	block_t blkaddr;	/* block address locating the last fsync */
	block_t last_dentry;	/* block address locating the last dentry */
};

struct section_order {
	struct list_head list;
	unsigned int secno;
};

#define nats_in_cursum(jnl)		(le16_to_cpu((jnl)->n_nats))
#define sits_in_cursum(jnl)		(le16_to_cpu((jnl)->n_sits))

#define nat_in_journal(jnl, i)		((jnl)->nat_j.entries[i].ne)
#define nid_in_journal(jnl, i)		((jnl)->nat_j.entries[i].nid)
#define sit_in_journal(jnl, i)		((jnl)->sit_j.entries[i].se)
#define segno_in_journal(jnl, i)	((jnl)->sit_j.entries[i].segno)

#ifdef CONFIG_HMFS_JOURNAL_APPEND
extern unsigned int write_opt;
#define APPEND_BLOCKS 2
#define MAX_NAT_JENTRIES(jnl)	(NAT_JOURNAL_ENTRIES +\
				(write_opt ? NAT_APPEND_JOURNAL_ENTRIES : 0) -\
				nats_in_cursum(jnl))
#define MAX_SIT_JENTRIES(jnl)	(SIT_JOURNAL_ENTRIES +\
				(write_opt ? SIT_APPEND_JOURNAL_ENTRIES : 0) -\
				sits_in_cursum(jnl))

#define need_append_nat_journal(jnl)	(nats_in_cursum(jnl) > NAT_JOURNAL_ENTRIES)
#define need_append_sit_journal(jnl)	(sits_in_cursum(jnl) > SIT_JOURNAL_ENTRIES)
#else
#define MAX_NAT_JENTRIES(jnl)  (NAT_JOURNAL_ENTRIES - nats_in_cursum(jnl))
#define MAX_SIT_JENTRIES(jnl)  (SIT_JOURNAL_ENTRIES - sits_in_cursum(jnl))
#endif

static inline int update_nats_in_cursum(struct f2fs_journal *journal, int i)
{
	int before = nats_in_cursum(journal);

	journal->n_nats = cpu_to_le16(before + i);
	return before;
}

static inline int update_sits_in_cursum(struct f2fs_journal *journal, int i)
{
	int before = sits_in_cursum(journal);

	journal->n_sits = cpu_to_le16(before + i);
	return before;
}

static inline bool __has_cursum_space(struct f2fs_journal *journal,
							int size, int type)
{
	if (type == NAT_JOURNAL)
                return size <= (int)MAX_NAT_JENTRIES(journal);
        return size <= (int)MAX_SIT_JENTRIES(journal);
}

/*
 * ioctl commands
 */
#define F2FS_IOC_GETFLAGS		FS_IOC_GETFLAGS
#define F2FS_IOC_SETFLAGS		FS_IOC_SETFLAGS
#define F2FS_IOC_GETVERSION		FS_IOC_GETVERSION

#define F2FS_IOCTL_MAGIC		0xf8
#define F2FS_IOC_START_ATOMIC_WRITE	_IO(F2FS_IOCTL_MAGIC, 1)
#define F2FS_IOC_COMMIT_ATOMIC_WRITE	_IO(F2FS_IOCTL_MAGIC, 2)
#define F2FS_IOC_START_VOLATILE_WRITE	_IO(F2FS_IOCTL_MAGIC, 3)
#define F2FS_IOC_RELEASE_VOLATILE_WRITE	_IO(F2FS_IOCTL_MAGIC, 4)
#define F2FS_IOC_ABORT_VOLATILE_WRITE	_IO(F2FS_IOCTL_MAGIC, 5)
#define F2FS_IOC_GARBAGE_COLLECT	_IOW(F2FS_IOCTL_MAGIC, 6, __u32)
#define F2FS_IOC_WRITE_CHECKPOINT	_IO(F2FS_IOCTL_MAGIC, 7)
#define F2FS_IOC_DEFRAGMENT		_IOWR(F2FS_IOCTL_MAGIC, 8,	\
						struct f2fs_defragment)
#define F2FS_IOC_MOVE_RANGE		_IOWR(F2FS_IOCTL_MAGIC, 9,	\
						struct f2fs_move_range)
#define F2FS_IOC_FLUSH_DEVICE		_IOW(F2FS_IOCTL_MAGIC, 10,	\
						struct f2fs_flush_device)
#define F2FS_IOC_GARBAGE_COLLECT_RANGE	_IOW(F2FS_IOCTL_MAGIC, 11,	\
						struct hmfs_gc_range)
#define F2FS_IOC_GET_FEATURES		_IOR(F2FS_IOCTL_MAGIC, 12, __u32)
#define F2FS_IOC_SET_PIN_FILE		_IOW(F2FS_IOCTL_MAGIC, 13, __u32)
#define F2FS_IOC_GET_PIN_FILE		_IOR(F2FS_IOCTL_MAGIC, 14, __u32)
#define F2FS_IOC_PRECACHE_EXTENTS	_IO(F2FS_IOCTL_MAGIC, 15)
#define F2FS_IOC_RESIZE_FS		_IOWR(F2FS_IOCTL_MAGIC, 16,     \
						struct f2fs_resize_from_end)

#define F2FS_IOC_SET_ENCRYPTION_POLICY	FS_IOC_SET_ENCRYPTION_POLICY
#define F2FS_IOC_GET_ENCRYPTION_POLICY	FS_IOC_GET_ENCRYPTION_POLICY
#define F2FS_IOC_GET_ENCRYPTION_PWSALT	FS_IOC_GET_ENCRYPTION_PWSALT


/*
 * should be same as XFS_IOC_GOINGDOWN.
 * Flags for going down operation used by FS_IOC_GOINGDOWN
 */
#define F2FS_IOC_SHUTDOWN	_IOR('X', 125, __u32)	/* Shutdown */
#define F2FS_GOING_DOWN_FULLSYNC	0x0	/* going down with full sync */
#define F2FS_GOING_DOWN_METASYNC	0x1	/* going down with metadata */
#define F2FS_GOING_DOWN_NOSYNC		0x2	/* going down */
#define F2FS_GOING_DOWN_METAFLUSH	0x3	/* going down with meta flush */

#if defined(__KERNEL__) && defined(CONFIG_COMPAT)
/*
 * ioctl commands in 32 bit emulation
 */
#define F2FS_IOC32_GETFLAGS		FS_IOC32_GETFLAGS
#define F2FS_IOC32_SETFLAGS		FS_IOC32_SETFLAGS
#define F2FS_IOC32_GETVERSION		FS_IOC32_GETVERSION
#endif

#define F2FS_IOC_FSGETXATTR		FS_IOC_FSGETXATTR
#define F2FS_IOC_FSSETXATTR		FS_IOC_FSSETXATTR

struct hmfs_gc_range {
	u32 sync;
	u64 start;
	u64 len;
};

struct f2fs_defragment {
	u64 start;
	u64 len;
};

struct f2fs_move_range {
	u32 dst_fd;		/* destination fd */
	u64 pos_in;		/* start position in src_fd */
	u64 pos_out;		/* start position in dst_fd */
	u64 len;		/* size to move */
};

struct f2fs_flush_device {
	u32 dev_num;		/* device number to flush */
	u32 segments;		/* # of segments to flush */
};

struct f2fs_resize_from_end {
	u64 len;		/* bytes to shrink */
};

/* for inline stuff */
#define DEF_INLINE_RESERVED_SIZE	1
#define DEF_MIN_INLINE_SIZE		1
static inline int get_extra_isize(struct inode *inode);
static inline int get_inline_xattr_addrs(struct inode *inode);
#define MAX_INLINE_DATA(inode)	(sizeof(__le32) *			\
				(CUR_ADDRS_PER_INODE(inode) -		\
				get_inline_xattr_addrs(inode) -	\
				DEF_INLINE_RESERVED_SIZE))

/* for inline dir */
#define NR_INLINE_DENTRY(inode)	(MAX_INLINE_DATA(inode) * BITS_PER_BYTE / \
				((SIZE_OF_DIR_ENTRY + F2FS_SLOT_LEN) * \
				BITS_PER_BYTE + 1))
#define INLINE_DENTRY_BITMAP_SIZE(inode)	((NR_INLINE_DENTRY(inode) + \
					BITS_PER_BYTE - 1) / BITS_PER_BYTE)
#define INLINE_RESERVED_SIZE(inode)	(MAX_INLINE_DATA(inode) - \
				((SIZE_OF_DIR_ENTRY + F2FS_SLOT_LEN) * \
				NR_INLINE_DENTRY(inode) + \
				INLINE_DENTRY_BITMAP_SIZE(inode)))

/*
 * For INODE and NODE manager
 */
/* for directory operations */
struct f2fs_dentry_ptr {
	struct inode *inode;
	void *bitmap;
	struct f2fs_dir_entry *dentry;
	__u8 (*filename)[F2FS_SLOT_LEN];
	int max;
	int nr_bitmap;
};

static inline void make_dentry_ptr_block(struct inode *inode,
		struct f2fs_dentry_ptr *d, struct f2fs_dentry_block *t)
{
	d->inode = inode;
	d->max = NR_DENTRY_IN_BLOCK;
	d->nr_bitmap = SIZE_OF_DENTRY_BITMAP;
	d->bitmap = t->dentry_bitmap;
	d->dentry = t->dentry;
	d->filename = t->filename;
}

static inline void make_dentry_ptr_inline(struct inode *inode,
					struct f2fs_dentry_ptr *d, void *t)
{
	int entry_cnt = NR_INLINE_DENTRY(inode);
	int bitmap_size = INLINE_DENTRY_BITMAP_SIZE(inode);
	int reserved_size = INLINE_RESERVED_SIZE(inode);

	d->inode = inode;
	d->max = entry_cnt;
	d->nr_bitmap = bitmap_size;
	d->bitmap = t;
	d->dentry = t + bitmap_size + reserved_size;
	d->filename = t + bitmap_size + reserved_size +
					SIZE_OF_DIR_ENTRY * entry_cnt;
}

/*
 * XATTR_NODE_OFFSET stores xattrs to one node block per file keeping -1
 * as its node offset to distinguish from index node blocks.
 * But some bits are used to mark the node block.
 */
#define XATTR_NODE_OFFSET	((((unsigned int)-1) << OFFSET_BIT_SHIFT) \
				>> OFFSET_BIT_SHIFT)
enum {
	ALLOC_NODE,			/* allocate a new node page if needed */
	LOOKUP_NODE,			/* look up a node without readahead */
	LOOKUP_NODE_RA,			/*
					 * look up a node with readahead called
					 * by get_data_block.
					 */
};

#define DEFAULT_RETRY_IO_COUNT	8	/* maximum retry read IO count */

/* maximum retry quota flush count */
#define DEFAULT_RETRY_QUOTA_FLUSH_COUNT		8

#define F2FS_LINK_MAX	0xffffffff	/* maximum link count per file */

#define MAX_DIR_RA_PAGES	4	/* maximum ra pages of dir */

/* for in-memory extent cache entry */
#define F2FS_MIN_EXTENT_LEN	64	/* minimum extent length */

/* number of extent info in extent cache we try to shrink */
#define EXTENT_CACHE_SHRINK_NUMBER	128

/* (us) throttle io to ensure GC in progress */
#define IO_THROTTLE_TIME_MAX		10000
#define IO_THROTTLE_TIME		64

enum {
	THROTTLE_LEVEL1 = 0,
	THROTTLE_LEVEL2 = 2,
	THROTTLE_LEVEL3 = 3,
	THROTTLE_LEVEL4 = 4,
	THROTTLE_LEVEL5 = 5,
	THROTTLE_LEVEL6 = 16,
	THROTTLE_LEVELS_NR
};

struct rb_entry {
	struct rb_node rb_node;		/* rb node located in rb-tree */
	union {
		struct {
			unsigned int ofs;	/* start offset of the entry */
			unsigned int len;	/* length of the entry */
		};
		unsigned long long key;		/* 64bits key */
	};
};

struct extent_info {
	unsigned int fofs;		/* start offset in a file */
	unsigned int len;		/* length of the extent */
	u32 blk;			/* start block address of the extent */
};

struct extent_node {
	struct rb_node rb_node;
	union {
		struct {
			unsigned int fofs;
			unsigned int len;
			u32 blk;
		};
		struct extent_info ei;	/* extent info */

	};
	struct list_head list;		/* node in global extent list of sbi */
	struct extent_tree *et;		/* extent tree pointer */
};

struct extent_tree {
	nid_t ino;			/* inode number */
	struct rb_root_cached root;	/* root of extent info rb-tree */
	struct extent_node *cached_en;	/* recently accessed extent node */
	struct extent_info largest;	/* largested extent info */
	struct list_head list;		/* to be used by sbi->zombie_list */
	rwlock_t lock;			/* protect extent info rb-tree */
	atomic_t node_cnt;		/* # of extent node in rb-tree*/
	bool largest_updated;		/* largest extent updated */
};

/*
 * This structure is taken from ext4_map_blocks.
 *
 * Note that, however, f2fs uses NEW and MAPPED flags for hmfs_map_blocks().
 */
#define F2FS_MAP_NEW		(1 << BH_New)
#define F2FS_MAP_MAPPED		(1 << BH_Mapped)
#define F2FS_MAP_UNWRITTEN	(1 << BH_Unwritten)
#define F2FS_MAP_FLAGS		(F2FS_MAP_NEW | F2FS_MAP_MAPPED |\
				F2FS_MAP_UNWRITTEN)

struct f2fs_map_blocks {
	block_t m_pblk;
	block_t m_lblk;
	unsigned int m_len;
	unsigned int m_flags;
	pgoff_t *m_next_pgofs;		/* point next possible non-hole pgofs */
	pgoff_t *m_next_extent;		/* point to next possible extent */
	int m_seg_type;
};

/* for flag in get_data_block */
enum {
	F2FS_GET_BLOCK_DEFAULT,
	F2FS_GET_BLOCK_FIEMAP,
	F2FS_GET_BLOCK_BMAP,
	F2FS_GET_BLOCK_DIO,
	F2FS_GET_BLOCK_PRE_DIO,
	F2FS_GET_BLOCK_PRE_AIO,
	F2FS_GET_BLOCK_PRECACHE,
};

/*
 * i_advise uses FADVISE_XXX_BIT. We can add additional hints later.
 */
#define FADVISE_COLD_BIT	0x01
#define FADVISE_LOST_PINO_BIT	0x02
#define FADVISE_ENCRYPT_BIT	0x04
#define FADVISE_ENC_NAME_BIT	0x08
#define FADVISE_KEEP_SIZE_BIT	0x10
#define FADVISE_HOT_BIT		0x20
#define FADVISE_VERITY_BIT	0x40	/* reserved */
#define FADVISE_INLINE_ENCRYPT_BIT     0x80
#define FADVISE_ENCRYPT_CORRUPT_BIT    0x40
#define FADVISE_ENCRYPT_FIXED_BIT      0x20


#define FADVISE_MODIFIABLE_BITS	(FADVISE_COLD_BIT | FADVISE_HOT_BIT)

#define file_is_cold(inode)	is_file(inode, FADVISE_COLD_BIT)
#define file_wrong_pino(inode)	is_file(inode, FADVISE_LOST_PINO_BIT)
#define file_set_cold(inode)	set_file(inode, FADVISE_COLD_BIT)
#define file_lost_pino(inode)	set_file(inode, FADVISE_LOST_PINO_BIT)
#define file_clear_cold(inode)	clear_file(inode, FADVISE_COLD_BIT)
#define file_got_pino(inode)	clear_file(inode, FADVISE_LOST_PINO_BIT)
#define file_is_encrypt(inode)	is_file(inode, FADVISE_ENCRYPT_BIT)
#define file_set_encrypt(inode)	set_file(inode, FADVISE_ENCRYPT_BIT)
#define file_clear_encrypt(inode) clear_file(inode, FADVISE_ENCRYPT_BIT)
#define file_enc_name(inode)	is_file(inode, FADVISE_ENC_NAME_BIT)
#define file_set_enc_name(inode) set_file(inode, FADVISE_ENC_NAME_BIT)
#define file_keep_isize(inode)	is_file(inode, FADVISE_KEEP_SIZE_BIT)
#define file_set_keep_isize(inode) set_file(inode, FADVISE_KEEP_SIZE_BIT)
#define file_is_hot(inode)	is_file(inode, FADVISE_HOT_BIT)
#define file_set_hot(inode)	set_file(inode, FADVISE_HOT_BIT)
#define file_clear_hot(inode)	clear_file(inode, FADVISE_HOT_BIT)
#define file_is_inline_encrypt(inode) \
                is_file(inode, FADVISE_INLINE_ENCRYPT_BIT)
#define file_set_inline_encrypt(inode) \
                set_file(inode, FADVISE_INLINE_ENCRYPT_BIT)
#define file_clear_inline_encrypt(inode) \
                clear_file(inode, FADVISE_INLINE_ENCRYPT_BIT)

#define DEF_DIR_LEVEL		0

enum {
	GC_FAILURE_PIN,
	GC_FAILURE_ATOMIC,
	MAX_GC_FAILURE
};

enum {
	SKIP_LOG_FILE,
	SKIP_PU_ALIGN,
	NR_SKIP
};

struct f2fs_inode_info {
	struct inode vfs_inode;		/* serve a vfs inode */
	unsigned long i_flags;		/* keep an inode flags for ioctl */
	unsigned char i_advise;		/* use to give file attribute hints */
	unsigned char i_dir_level;	/* use for dentry level for large dir */
	unsigned int i_current_depth;	/* only for directory depth */
	/* for gc failure statistic */
	unsigned int i_gc_failures[MAX_GC_FAILURE];
	unsigned int i_pino;		/* parent inode number */
	umode_t i_acl_mode;		/* keep file acl mode temporarily */

	/* Use below internally in f2fs*/
	unsigned long flags;		/* use to pass per-file flags */
	struct rw_semaphore i_sem;	/* protect fi info */
#if F2FS_FS_SDP_ENCRYPTION
	struct rw_semaphore i_sdp_sem;	/* protect sdp */
#endif
	atomic_t dirty_pages;		/* # of dirty pages */
	f2fs_hash_t chash;		/* hash value of given file name */
	unsigned int clevel;		/* maximum level of given file name */
	struct task_struct *task;	/* lookup and create consistency */
	struct task_struct *cp_task;	/* separate cp/wb IO stats*/
	nid_t i_xattr_nid;		/* node id that contains xattrs */
	loff_t	last_disk_size;		/* lastly written file size */

#ifdef CONFIG_QUOTA
	struct dquot *i_dquot[MAXQUOTAS];

	/* quota space reservation, managed internally by quota code */
	qsize_t i_reserved_quota;
#endif
	/* use for oob recovery */
	bool has_wb;			/* whether has write back after CP or fsync */
	bool fsync_atomic;
	bool last_atomic;
	struct page *oob_last_page;
	struct task_struct *fsync_task;	/* whether in fsync flow, protect by i_rwsem */
	unsigned long long fsync_ver;	/* cp version of last fsync */

	int last_stream;		/* STREAM_NR(init) or last stream id */
	/* whether file stream id is switched or not after last CP */
	bool is_switch;
	atomic_t switch_count;		/* file stream switch count */
	spinlock_t stream_lock;		/* protect last_stream/is_switch */

	struct list_head dirty_list;	/* dirty list for dirs and files */
	struct list_head gdirty_list;	/* linked in global dirty list */
	struct list_head inmem_ilist;	/* list for inmem inodes */
	struct list_head inmem_pages;	/* inmemory pages managed by f2fs */
	struct task_struct *inmem_task;	/* store inmemory task */
	struct mutex inmem_lock;	/* lock for inmemory pages */
	struct extent_tree *extent_tree;	/* cached extent_tree entry */

	/* avoid racing between foreground op and gc */
	struct rw_semaphore i_gc_rwsem[2];
	struct rw_semaphore i_mmap_sem;
	struct rw_semaphore i_xattr_sem; /* avoid racing between reading and changing EAs */

	int i_extra_isize;		/* size of extra space located in i_addr */
	kprojid_t i_projid;		/* id for project quota */
	int i_inline_xattr_size;	/* inline xattr size */
	struct timespec i_crtime;	/* inode creation time */
	struct timespec i_disk_time[4];	/* inode disk times */
	unsigned int skip_count[NR_SKIP];

	struct delayed_work fsync_work;
	unsigned char i_fsync_flag;
	wait_queue_head_t fsync_wq;
};

static inline void get_extent_info(struct extent_info *ext,
					struct f2fs_extent *i_ext)
{
	ext->fofs = le32_to_cpu(i_ext->fofs);
	ext->blk = le32_to_cpu(i_ext->blk);
	ext->len = le32_to_cpu(i_ext->len);
}

static inline void set_raw_extent(struct extent_info *ext,
					struct f2fs_extent *i_ext)
{
	i_ext->fofs = cpu_to_le32(ext->fofs);
	i_ext->blk = cpu_to_le32(ext->blk);
	i_ext->len = cpu_to_le32(ext->len);
}

static inline void set_extent_info(struct extent_info *ei, unsigned int fofs,
						u32 blk, unsigned int len)
{
	ei->fofs = fofs;
	ei->blk = blk;
	ei->len = len;
}

static inline bool __is_discard_mergeable(struct discard_info *back,
			struct discard_info *front, unsigned int max_len)
{
	return (back->lstart + back->len == front->lstart) &&
		(back->len + front->len <= max_len);
}

static inline bool __is_discard_back_mergeable(struct discard_info *cur,
			struct discard_info *back, unsigned int max_len)
{
	return __is_discard_mergeable(back, cur, max_len);
}

static inline bool __is_discard_front_mergeable(struct discard_info *cur,
			struct discard_info *front, unsigned int max_len)
{
	return __is_discard_mergeable(cur, front, max_len);
}

static inline bool __is_extent_mergeable(struct extent_info *back,
						struct extent_info *front)
{
	return (back->fofs + back->len == front->fofs &&
			back->blk + back->len == front->blk);
}

static inline bool __is_back_mergeable(struct extent_info *cur,
						struct extent_info *back)
{
	return __is_extent_mergeable(back, cur);
}

static inline bool __is_front_mergeable(struct extent_info *cur,
						struct extent_info *front)
{
	return __is_extent_mergeable(cur, front);
}

extern void hmfs_mark_inode_dirty_sync(struct inode *inode, bool sync);
static inline void __try_update_largest_extent(struct extent_tree *et,
						struct extent_node *en)
{
	if (en->ei.len > et->largest.len) {
		et->largest = en->ei;
		et->largest_updated = true;
	}
}

/*
 * For free nid management
 */
enum nid_state {
	FREE_NID,		/* newly added to free nid list */
	PREALLOC_NID,		/* it is preallocated */
	MAX_NID_STATE,
};

struct f2fs_nm_info {
	block_t nat_blkaddr;		/* base disk address of NAT */
	nid_t max_nid;			/* maximum possible node ids */
	nid_t available_nids;		/* # of available node ids */
	nid_t next_scan_nid;		/* the next nid to be scanned */
	unsigned int ram_thresh;	/* control the memory footprint */
	unsigned int ra_nid_pages;	/* # of nid pages to be readaheaded */
	unsigned int dirty_nats_ratio;	/* control dirty nats ratio threshold */

	/* NAT cache management */
	struct radix_tree_root nat_root;/* root of the nat entry cache */
	struct radix_tree_root nat_set_root;/* root of the nat set cache */
	struct rw_semaphore nat_tree_lock;	/* protect nat_tree_lock */
	struct list_head nat_entries;	/* cached nat entry list (clean) */
	spinlock_t nat_list_lock;	/* protect clean nat entry list */
	unsigned int nat_cnt;		/* the # of cached nat entries */
	unsigned int dirty_nat_cnt;	/* total num of nat entries in set */
	unsigned int nat_blocks;	/* # of nat blocks */

	/* free node ids management */
	struct radix_tree_root free_nid_root;/* root of the free_nid cache */
	struct list_head free_nid_list;		/* list for free nids excluding preallocated nids */
	unsigned int nid_cnt[MAX_NID_STATE];	/* the number of free node id */
	spinlock_t nid_list_lock;	/* protect nid lists ops */
	struct mutex build_lock;	/* lock for build free nids */
	unsigned char **free_nid_bitmap;
	unsigned long *nat_block_bitmap;
	unsigned int scaned_nat_blocks;  /* # of nat blocks has been scaned */
	unsigned short *free_nid_count;	/* free nid count of NAT block */

	/* for checkpoint */
	char *nat_bitmap;		/* NAT bitmap pointer */

	unsigned int nat_bits_blocks;	/* # of nat bits blocks */
	unsigned char *nat_bits;	/* NAT bits blocks */
	unsigned char *full_nat_bits;	/* full NAT pages */
	unsigned char *empty_nat_bits;	/* empty NAT pages */
#ifdef CONFIG_HMFS_CHECK_FS
	char *nat_bitmap_mir;		/* NAT bitmap mirror */
#endif
	int bitmap_size;		/* bitmap size */
};

/*
 * this structure is used as one of function parameters.
 * all the information are dedicated to a given direct node block determined
 * by the data offset in a file.
 */
struct dnode_of_data {
	struct inode *inode;		/* vfs inode pointer */
	struct page *inode_page;	/* its inode page, NULL is possible */
	struct page *node_page;		/* cached direct node page */
	nid_t nid;			/* node id of the direct node block */
	unsigned int ofs_in_node;	/* data offset in the node page */
	bool inode_page_locked;		/* inode page is locked or not */
	bool node_changed;		/* is node block changed */
	char cur_level;			/* level of hole node page */
	char max_level;			/* level of current page located */
	block_t	data_blkaddr;		/* block address of the node block */
	int mem_control;		/* use less memory, even failed */
};

static inline void set_new_dnode(struct dnode_of_data *dn, struct inode *inode,
		struct page *ipage, struct page *npage, nid_t nid)
{
	memset(dn, 0, sizeof(*dn));
	dn->inode = inode;
	dn->inode_page = ipage;
	dn->node_page = npage;
	dn->nid = nid;
}

/*
 * For SIT manager
 *
 * By default, there are 6 active log areas across the whole main area.
 * When considering hot and cold data separation to reduce cleaning overhead,
 * we split 3 for data logs and 3 for node logs as hot, warm, and cold types,
 * respectively.
 * In the current design, you should not change the numbers intentionally.
 * Instead, as a mount option such as active_logs=x, you can use 2, 4, and 6
 * logs individually according to the underlying devices. (default: 6)
 * Just in case, on-disk layout covers maximum 16 logs that consist of 8 for
 * data and 8 for node logs.
 */
#define	NR_CURSEG_DATA_TYPE	(3)
#define NR_CURSEG_NODE_TYPE	(3)
#define NR_CURSEG_DM_TYPE	(2)
#define NR_CURSEG_SPECIAL_TYPE	(1)
#define NR_CURSEG_TYPE	(NR_CURSEG_DATA_TYPE +	\
		NR_CURSEG_NODE_TYPE + NR_CURSEG_DM_TYPE)
#define NR_INMEM_CURSEG_TYPE	(NR_CURSEG_TYPE + NR_CURSEG_SPECIAL_TYPE)

enum {
	CURSEG_HOT_DATA	= 0,	/* directory entry blocks */
	CURSEG_WARM_DATA,	/* data blocks */
	CURSEG_COLD_DATA,	/* multimedia or GCed data blocks */
	CURSEG_HOT_NODE,	/* direct node blocks of directory files */
	CURSEG_WARM_NODE,	/* direct node blocks of normal files */
	CURSEG_COLD_NODE,	/* indirect node blocks */
	CURSEG_DATA_MOVE1,	/* data move log 1 */
	CURSEG_DATA_MOVE2,	/* data move log 2 */
	CURSEG_FRAGMENT_DATA,	/* do SSR located in hot/warm/cold data area */
	NO_CHECK_TYPE,
};

struct flush_cmd {
	struct completion wait;
	struct llist_node llnode;
	nid_t ino;
	int ret;
};

struct flush_cmd_control {
	struct task_struct *hmfs_issue_flush;	/* flush thread */
	wait_queue_head_t flush_wait_queue;	/* waiting queue for wake-up */
	atomic_t issued_flush;			/* # of issued flushes */
	atomic_t issing_flush;			/* # of issing flushes */
	struct llist_head issue_list;		/* list for command issue */
	struct llist_node *dispatch_list;	/* list for command dispatch */
};

struct f2fs_sm_info {
	struct sit_info *sit_info;		/* whole segment information */
	struct free_segmap_info *free_info;	/* free segment information */
	struct dirty_seglist_info *dirty_info;	/* dirty segment information */
	struct curseg_info *curseg_array;	/* active segment information */

	struct rw_semaphore curseg_lock;	/* for preventing curseg change */

	block_t seg0_blkaddr;		/* block address of 0'th segment */
	block_t main_blkaddr;		/* start block address of main area */
	block_t ssa_blkaddr;		/* start block address of SSA area */

	unsigned int segment_count;	/* total # of segments */
	unsigned int main_segments;	/* # of segments in main area */
	unsigned int reserved_segments;	/* # of reserved segments */
	unsigned int cp_reserved_segments;	/* # of reserved segments in cp */
	unsigned int ovp_segments;	/* # of overprovision segments */
	unsigned int extra_op_segments;	/* # of extra overprovision segments */

	/* a threshold to reclaim prefree segments */
	unsigned int rec_prefree_segments;

	/* for batched trimming */
	unsigned int trim_sections;		/* # of sections to trim */

	struct list_head sit_entry_set;	/* sit entry set list */

	unsigned int ipu_policy;	/* in-place-update policy */
	unsigned int min_ipu_util;	/* in-place-update threshold */
	unsigned int min_fsync_blocks;	/* threshold for fsync */
	unsigned int min_seq_blocks;	/* threshold for sequential blocks */
	unsigned int min_hot_blocks;	/* threshold for hot block allocation */
	unsigned int min_ssr_sections;	/* threshold to trigger SSR allocation */

	/* for flush command control */
	struct flush_cmd_control *fcc_info;

	/* for discard command control */
	struct discard_cmd_control *dcc_info;
};

/*
 * For superblock
 */
/*
 * COUNT_TYPE for monitoring
 *
 * f2fs monitors the number of several block types such as on-writeback,
 * dirty dentry blocks, dirty node blocks, and dirty meta blocks.
 */
#define WB_DATA_TYPE(p)	(__is_cp_guaranteed(p) ? F2FS_WB_CP_DATA : F2FS_WB_DATA)
enum count_type {
	F2FS_DIRTY_DENTS,
	F2FS_DIRTY_DATA,
	F2FS_DIRTY_QDATA,
	F2FS_DIRTY_NODES,
	F2FS_DIRTY_META,
	F2FS_INMEM_PAGES,
	F2FS_DIRTY_IMETA,
	F2FS_WB_CP_DATA,
	F2FS_WB_DATA,
	F2FS_RD_DATA,
	F2FS_RD_NODE,
	F2FS_RD_META,
	F2FS_LOG_FILE,
	NR_COUNT_TYPE,
};

/*
 * The below are the page types of bios used in submit_bio().
 * The available types are:
 * DATA			User data pages. It operates as async mode.
 * NODE			Node pages. It operates as async mode.
 * META			FS metadata pages such as SIT, NAT, CP.
 * NR_PAGE_TYPE		The number of page types.
 * META_FLUSH		Make sure the previous pages are written
 *			with waiting the bio's completion
 * ...			Only can be used with META.
 */
#define PAGE_TYPE_OF_BIO(type)	((type) > META ? META : (type))
enum page_type {
	DATA,
	NODE,
	META,
	NR_PAGE_TYPE,
	META_FLUSH,
	INMEM,		/* the below types are used by tracepoints only. */
	INMEM_DROP,
	INMEM_INVALIDATE,
	INMEM_REVOKE,
	IPU,
	OPU,
};

enum temp_type {
	HOT = 0,	/* must be zero for meta bio */
	WARM,
	COLD,
	NR_TEMP_TYPE,
};

enum need_lock_type {
	LOCK_REQ = 0,
	LOCK_DONE,
	LOCK_RETRY,
};

enum cp_reason_type {
	CP_NO_NEEDED,
	CP_NON_REGULAR,
	CP_HARDLINK,
	CP_SB_NEED_CP,
	CP_WRONG_PINO,
	CP_NO_SPC_ROLL,
	CP_NODE_NEED_CP,
	CP_FASTBOOT_MODE,
	CP_SPEC_LOG_NUM,
	CP_RECOVER_DIR,
	/* start: for oob recovery */
	CP_SWITCH_STREAM,
	CP_XATTR_DIRTY,
	CP_OOB_OVERFLOW,
	CP_OOB_CHG_ATOMIC,
	/* end: for oob recovery */
};

enum iostat_type {
	APP_DIRECT_IO,			/* app direct IOs */
	APP_BUFFERED_IO,		/* app buffered IOs */
	APP_WRITE_IO,			/* app write IOs */
	APP_MAPPED_IO,			/* app mapped IOs */
	FS_DATA_IO,			/* data IOs from kworker/fsync/reclaimer */
	FS_NODE_IO,			/* node IOs from kworker/fsync/reclaimer */
	FS_META_IO,			/* meta IOs from kworker/reclaimer */
	FS_GC_DATA_IO,			/* data IOs from forground gc */
	FS_GC_NODE_IO,			/* node IOs from forground gc */
	FS_CP_DATA_IO,			/* data IOs from checkpoint */
	FS_CP_NODE_IO,			/* node IOs from checkpoint */
	FS_CP_META_IO,			/* meta IOs from checkpoint */
	FS_DISCARD,			/* discard */
	NR_IO_TYPE,
};

struct f2fs_io_info {
	struct f2fs_sb_info *sbi;	/* f2fs_sb_info pointer */
	nid_t ino;		/* inode number */
	enum page_type type;	/* contains DATA/NODE/META/META_FLUSH */
	enum temp_type temp;	/* contains HOT/WARM/COLD */
	int op;			/* contains REQ_OP_ */
	int op_flags;		/* req_flag_bits */
	block_t new_blkaddr;	/* new block address to be written */
	block_t old_blkaddr;	/* old block address before Cow */
	struct page *page;	/* page to be written */
	struct page *encrypted_page;	/* encrypted page */
#ifdef CONFIG_HMFS_FS_ENCRYPTION
	void *ci_key;
	int ci_key_len;
	int ci_key_index;
	unsigned char *ci_metadata;
#endif
	struct list_head list;		/* serialize IOs */
	bool submitted;		/* indicate IO submission */
	int need_lock;		/* indicate we need to lock cp_rwsem */
	bool in_list;		/* indicate fio is in io_list */
	bool is_meta;		/* indicate borrow meta inode mapping or not */
	bool retry;		/* need to reallocate block address */
	enum iostat_type io_type;	/* io type */
	struct writeback_control *io_wbc; /* writeback control */
	unsigned char version;		/* version of the node */
	int mem_control;                /* use less memory, even failed */
};

#define is_read_io(rw) ((rw) == READ)
struct f2fs_bio_info {
	struct f2fs_sb_info *sbi;	/* f2fs superblock */
	struct bio *bio;		/* bios to merge */
	sector_t last_block_in_bio;	/* last block number */
	struct f2fs_io_info fio;	/* store buffered io info. */
	struct rw_semaphore io_rwsem;	/* blocking op for bio */
	pgoff_t last_index_in_bio;	/* for fs encryption/oob recover */
	spinlock_t io_lock;		/* serialize DATA/NODE IOs */
	struct list_head io_list;	/* track fios */
};

#define FDEV(i)				(sbi->devs[i])
#define RDEV(i)				(raw_super->devs[i])
struct f2fs_dev_info {
	struct block_device *bdev;
	char path[MAX_PATH_LEN];
	unsigned int total_segments;
	block_t start_blk;
	block_t end_blk;
#ifdef CONFIG_BLK_DEV_ZONED
	unsigned int nr_blkz;			/* Total number of zones */
	u8 *blkz_type;				/* Array of zones type */
#endif
};

struct rescue_seg_entry {
	struct list_head list;
	unsigned int segno;
};

#define F2FS_TZ_RESERVED_DEV "RESERVED"

struct hmfs_gc_control_info {
	atomic_t inflight;		/* only limit write IO */
	wait_queue_head_t wait;
	int iolimit;
	int loop_cnt;
	int gc_skip_count;
	bool is_greedy;
	bool idle_enabled;

	/*
	 *  no need spinlock:
	 *  1) add entry at mount
	 *  2) delete entry at hmfs_gc
	 *  3) start IO or gc thread after mount is done
	 */
	struct list_head rescue_segment_list;	/* ufs report rescue segments to f2fs after SPOR */
	struct rescue_seg_entry *rs_entry;	/* memory for ufs_gc_list */
};

enum flash_mode {
	TLC_MODE,
	SLC_MODE,
	NR_FLASH_MODE
};

struct slc_mode_control_info {
	bool pe_limited;		/* disable slc_enable when reach PE limits */
	atomic_t alloc_secs;
	struct workqueue_struct *query_wq;
	struct work_struct       query_work;	/* query PE limits */

	int cur_util_rate;		/* user utilization */
	int slc_mode_type;			/* section threshold */
	bool hmfs_is_slc_mode_enable;	/* disabled by PE limits or util_rate */
	atomic_t sec_count[NR_FLASH_MODE];

	bool closed;			/* closed because of lack of free sections */

	struct f2fs_sb_info *sbi;
};

enum {
	RESET_FTL,
	CLOSE_SEC
};

enum {
	NORMAL_STREAM,
	DATA_MOVE_STREAM
};

#define MAX_RECOVER_EXT_CNT	(2)
struct recover_ext_info {
	block_t start_lba;
	block_t end_lba;
	int len;

	bool rsvd_space;
};

struct hmfs_oob_control_info {
	struct recover_ext_info recover_ext[MAX_RECOVER_EXT_CNT];
	int recover_ext_cnt;

	struct stor_dev_stream_oob_info *oob_info;
	int oob_cnt;
};

enum inode_type {
	DIR_INODE,			/* for dirty dir inode */
	FILE_INODE,			/* for dirty regular/symlink inode */
	DIRTY_META,			/* for all dirtied inode metadata */
	ATOMIC_FILE,			/* for all atomic files */
	NR_INODE_TYPE,
};

/* for inner inode cache management */
struct inode_management {
	struct radix_tree_root ino_root;	/* ino entry array */
	spinlock_t ino_lock;			/* for ino entry lock */
	struct list_head ino_list;		/* inode list head */
	unsigned long ino_num;			/* number of entries */
};

struct hmfs_gc_kthread {
	struct task_struct *hmfs_gc_task;
	wait_queue_head_t gc_wait_queue_head;

	/* for gc sleep time */
	unsigned int urgent_sleep_time;
	unsigned int min_sleep_time;
	unsigned int max_sleep_time;
	unsigned int no_gc_sleep_time;

	/* for changing gc mode */
	unsigned int gc_idle;
	unsigned int gc_preference;

#ifdef CONFIG_MAS_BLK
	/* block idle notify */
	struct blk_busyidle_event_node gc_event_node;
	struct timer_list nb_timer;
	bool block_idle;
#endif

	unsigned int gc_urgent;
	unsigned int gc_wake;
	/* for GC_AT */
	struct rb_root root;		/* root of victim rb-tree */
	struct list_head victim_list;	/* linked with all victim entries */
	unsigned int victim_count;	/* victim count in rb-tree */
	unsigned int dirty_count_threshold;
	unsigned int dirty_rate_threshold;
	unsigned long long age_threshold;
	unsigned int age_weight;
	bool atgc_enabled;

	wait_queue_head_t fg_gc_wait;
};

/* For s_flag in struct f2fs_sb_info */
enum {
	SBI_IS_DIRTY,				/* dirty flag for checkpoint */
	SBI_IS_CLOSE,				/* specify unmounting */
	SBI_NEED_FSCK,				/* need fsck.f2fs to fix */
	SBI_POR_DOING,				/* recovery is doing or not */
	SBI_NEED_SB_WRITE,			/* need to recover superblock */
	SBI_NEED_CP,				/* need to checkpoint */
	SBI_IS_SHUTDOWN,			/* shutdown by ioctl */
	SBI_IS_RECOVERED,			/* recovered orphan/data */
	SBI_CP_DISABLED,			/* CP was disabled last mount */
	SBI_QUOTA_NEED_FLUSH,			/* need to flush quota info in CP */
	SBI_QUOTA_SKIP_FLUSH,			/* skip flushing quota in current CP */
	SBI_QUOTA_NEED_REPAIR,			/* quota file may be corrupted */
	SBI_IS_RESIZEFS,
	SBI_BKOPS_WORK_START,			/* bkops work has already start */
	SBI_DATAMOVE_GC,			/* datamove gc is on */
};

enum {
	CP_TIME,
	REQ_TIME,
	DISCARD_TIME,
	GC_TIME,
	DISABLE_TIME,
	MAX_TIME,
};

enum {
	GC_NORMAL,
	GC_IDLE_CB,
	GC_IDLE_GREEDY,
	GC_IDLE_AT,
	GC_URGENT,
};

enum {
	WHINT_MODE_OFF,		/* not pass down write hints */
	WHINT_MODE_USER,	/* try to pass down hints given by users */
	WHINT_MODE_FS,		/* pass down hints with F2FS policy */
};

enum {
	ALLOC_MODE_DEFAULT,	/* stay default */
	ALLOC_MODE_REUSE,	/* reuse segments as much as possible */
};

enum fsync_mode {
	FSYNC_MODE_POSIX,	/* fsync follows posix semantics */
	FSYNC_MODE_STRICT,	/* fsync behaves in line with ext4 */
	FSYNC_MODE_NOBARRIER,	/* fsync behaves nobarrier based on posix */
};

enum {
	BG_GC_LEVEL1 = 0,
	BG_GC_LEVEL2,
	BG_GC_LEVEL3,
	BG_GC_LEVEL4,
	BG_GC_LEVEL5,
	BG_GC_LEVEL6,
	FG_GC_LEVEL,
	ALL_GC_LEVELS
};

struct gc_stat {
	unsigned long times[ALL_GC_LEVELS];
	unsigned int level[ALL_GC_LEVELS];
};

/* for data move */
/* FIXME: PU should be setted according to device feature */
#define HMFS_DATAMOVE_PU_SIZE(sbi)		(sbi->pu_size[TLC_MODE])
struct hmfs_dm_entry {
	block_t	src_blkaddr;
	block_t dst_blkaddr;
	unsigned int ino;
	unsigned int offset;
};

struct hmfs_dm_info {
	struct hmfs_dm_entry *entries;		/* entries cached before submited */
	unsigned int index;		/* current index */
	block_t verified_blkaddr;	/* The address before verified_blkaddr has been verified */
	block_t next_blkaddr;		/* next block address to do datamove to */
	block_t cached_last_blkaddr;	/* last block address not submited */

	struct hmfs_dm_entry *rescue_entries;	/* entries need to be rescued */
	unsigned int rescue_index;
};

struct hmfs_dm_manager {
	struct radix_tree_root root;
	atomic_t count;			/* entries count in radix tree */
	struct rw_semaphore rw_sem;
	wait_queue_head_t wait;
	atomic_t refs;
	struct stor_dev_data_move_source_addr *source_addrs;
	struct stor_dev_data_move_source_inode *source_inodes;

	struct hmfs_dm_info dm_info[NR_CURSEG_DM_TYPE];
};

#ifdef CONFIG_HMFS_FS_ENCRYPTION
#define DUMMY_ENCRYPTION_ENABLED(sbi) \
			(unlikely(F2FS_OPTION(sbi).test_dummy_encryption))
#else
#define DUMMY_ENCRYPTION_ENABLED(sbi) (0)
#endif

static inline long long nsec_to_sec(unsigned long long nsec)
{
        if ((long long)nsec < 0) {
                nsec = -nsec;
                do_div(nsec, 1000000);
                return -nsec;
        }
        do_div(nsec, 1000000);

        return nsec;
}

struct f2fs_hot_cold_params {
         unsigned int enable;
         unsigned int hot_data_lower_limit;
         unsigned int hot_data_waterline;
         unsigned int warm_data_lower_limit;
         unsigned int warm_data_waterline;
         unsigned int hot_node_lower_limit;
         unsigned int hot_node_waterline;
         unsigned int warm_node_lower_limit;
         unsigned int warm_node_waterline;
};

enum gc_test_type {
	GC_TEST_DISABLE_IO_AWARE,
	GC_TEST_DISABLE_SYNCFS,
	GC_TEST_DISABLE_FRAG_URGENT,
	GC_TEST_DISABLE_GC_URGENT,
	GC_TEST_ENABLE_GC_STAT,
};
#define is_gc_test_set(sbi, type) ((sbi)->gc_test_cond & (1 << (type)))

enum {
	PU_ALIGN_HEAD = 0,	/* BIO num that start addr isn't align with PU */
	PU_ALIGN_LEN,		/* BIO num that length isn't align with PU */
	PU_ALIGN_IO,		/* all write BIO num */

	PU_FLUSH_SYNC,		/* flush pu-no-align writeback due to sync */
	PU_FLUSH_FSYNC,		/* flush pu-no-align writeback due to fsync */
	PU_FLUSH_SKIP,		/* flush pu-no-align writeback due to skip limits */

	PU_ALIGN_NR,
};
#define stat_inc_pu_info(sbi, type, bios)	\
	atomic64_add((bios), &((sbi)->pu_align_info[type]))

#define stat_get_pu_info(sbi, type)	\
	atomic64_read(&((sbi)->pu_align_info[type]))

struct f2fs_sb_info {
	struct super_block *sb;			/* pointer to VFS super block */
	struct proc_dir_entry *s_proc;		/* proc entry */
	struct f2fs_super_block *raw_super;	/* raw super block pointer */
	struct rw_semaphore sb_lock;		/* lock for raw super block */
	int valid_super_block;			/* valid super block no */
	unsigned long s_flag;				/* flags for sbi */
	struct mutex writepages;		/* mutex for writepages() */

#ifdef CONFIG_BLK_DEV_ZONED
	unsigned int blocks_per_blkz;		/* F2FS blocks per zone */
	unsigned int log_blocks_per_blkz;	/* log2 F2FS blocks per zone */
#endif

	/* for node-related operations */
	struct f2fs_nm_info *nm_info;		/* node manager */
	struct inode *node_inode;		/* cache node blocks */

	/* for segment-related operations */
	struct f2fs_sm_info *sm_info;		/* segment manager */

	/* for bio operations */
	struct f2fs_bio_info *write_io[NR_PAGE_TYPE];	/* for write bios */
	struct mutex wio_mutex[NR_PAGE_TYPE - 1][NR_TEMP_TYPE];
						/* bio ordering for NODE/DATA */
	/* keep migration IO order for LFS mode */
	struct rw_semaphore io_order_lock;
	mempool_t *write_io_dummy;		/* Dummy pages */

	/* for checkpoint */
	struct f2fs_checkpoint *ckpt;		/* raw checkpoint pointer */
	int cur_cp_pack;			/* remain current cp pack */
	spinlock_t cp_lock;			/* for flag in ckpt */
	u64 ckpt_crc;				/* CRC of cp1 in a cp pack */
	struct inode *meta_inode;		/* cache meta blocks */
	struct mutex cp_mutex;			/* checkpoint procedure lock */
	struct rw_semaphore cp_rwsem;		/* blocking FS operations */

#if 1
	/* used for debugging cp rwsem */
	struct task_struct *cp_rwsem_owner;
	unsigned long cp_rwsem_owner_j;
	spinlock_t cp_rwsem_lock;
#endif

	struct rw_semaphore node_write;		/* locking node writes */
	struct rw_semaphore node_change;	/* locking node change */
	wait_queue_head_t cp_wait;
	unsigned long last_time[MAX_TIME];	/* to store time in jiffies */
	long interval_time[MAX_TIME];		/* to store thresholds */

	struct inode_management im[MAX_INO_ENTRY];      /* manage inode cache */

	spinlock_t fsync_node_lock;		/* for node entry lock */
	struct list_head fsync_node_list;	/* node list head */
	unsigned int fsync_seg_id;		/* sequence id */
	unsigned int fsync_node_num;		/* number of node entries */

	/* for orphan inode, use 0'th array */
	unsigned int max_orphans;		/* max orphan inodes */

	/* for inode management */
	struct list_head inode_list[NR_INODE_TYPE];	/* dirty inode list */
	spinlock_t inode_lock[NR_INODE_TYPE];	/* for dirty inode list lock */

	/* for storage turbo guarantee order of bio */
	struct bio_list bio_list[STREAM_NR];
	struct mutex bio_lock[STREAM_NR];
	struct list_head section_list[STREAM_NR];
	spinlock_t section_list_lock[STREAM_NR];	/* lock for section list */
	atomic_t last_blkaddr[STREAM_NR];
	unsigned int order_stat_times[2]; /* 0:total times   1:scan order list times */

	/* for extent tree cache */
	struct radix_tree_root extent_tree_root;/* cache extent cache entries */
	struct mutex extent_tree_lock;	/* locking extent radix tree */
	struct list_head extent_list;		/* lru list for shrinker */
	spinlock_t extent_lock;			/* locking extent lru list */
	atomic_t total_ext_tree;		/* extent tree count */
	struct list_head zombie_list;		/* extent zombie tree list */
	atomic_t total_zombie_tree;		/* extent zombie tree count */
	atomic_t total_ext_node;		/* extent info count */

	/* basic filesystem units */
	unsigned int log_sectors_per_block;	/* log2 sectors per block */
	unsigned int log_blocksize;		/* log2 block size */
	unsigned int blocksize;			/* block size */
	unsigned int root_ino_num;		/* root inode number*/
	unsigned int node_ino_num;		/* node inode number*/
	unsigned int meta_ino_num;		/* meta inode number*/
	unsigned int log_blocks_per_seg;	/* log2 blocks per segment */
	unsigned int blocks_per_seg;		/* blocks per segment */
	unsigned int segs_per_sec;		/* segments per section */
	unsigned int secs_per_zone;		/* sections per zone */
	unsigned int total_sections;		/* total section count */
	bool resized;
	struct mutex resize_mutex;		/* for resize exclusion */
	unsigned int total_node_count;		/* total node block count */
	unsigned int total_valid_node_count;	/* valid node block count */
	loff_t max_file_blocks;			/* max block index of file */
	int dir_level;				/* directory level */
	int readdir_ra;				/* readahead inode in readdir */
	unsigned int gc_test_cond;		/* condition for gc test */

	block_t user_block_count;		/* # of user blocks */
	block_t total_valid_block_count;	/* # of valid blocks */
	block_t discard_blks;			/* discard command candidats */
	block_t last_valid_block_count;		/* for recovery */
	block_t reserved_blocks;		/* configurable reserved blocks */
	block_t current_reserved_blocks;	/* current reserved blocks */

	/* Additional tracking for no checkpoint mode */
	block_t unusable_block_count;		/* # of blocks saved by last cp */

	unsigned int nquota_files;		/* # of quota sysfile */

	u32 s_next_generation;			/* for NFS support */

	/* # of pages, see count_type */
	atomic_t nr_pages[NR_COUNT_TYPE];
	/* # of allocated blocks */
	struct percpu_counter alloc_valid_block_count;

	/* writeback control */
	atomic_t wb_sync_req[META];	/* count # of WB_SYNC threads */

	/* valid inode count */
	struct percpu_counter total_valid_inode_count;

	struct f2fs_mount_info mount_opt;	/* mount options */

	/* for cleaning operations */
	struct mutex gc_mutex;
	struct hmfs_gc_kthread	gc_thread;	/* GC thread */
	unsigned int cur_victim_sec;		/* current victim section num */
	unsigned int next_victim_seg;		/* next victim segment num */
	unsigned int gc_mode;			/* current GC state */
	struct hmfs_dm_manager dm_mng;  /* datamove management info */
	/* for skip statistic */
	unsigned int atomic_files;		/* # of opened atomic file */
	unsigned long long skipped_atomic_files[2];	/* FG_GC and BG_GC */
	unsigned long long skipped_gc_rwsem;		/* FG_GC only */

	/* threshold for gc trials on pinned files */
	u64 gc_pin_file_threshold;
	struct gc_loop_info gc_loop;

	/* maximum # of trials to find a victim segment for SSR and GC */
	unsigned int max_victim_search;

	/*
	 * for stat information.
	 * one is for the LFS mode, and the other is for the SSR mode.
	 */
#ifdef CONFIG_HMFS_STAT_FS
	struct f2fs_stat_info *stat_info;	/* FS status information */
	atomic_t meta_count[META_MAX];		/* # of meta blocks */
	unsigned int segment_count[2];		/* # of allocated segments */
	unsigned int block_count[2];		/* # of allocated blocks */
	atomic_t inplace_count;		/* # of inplace update */
	atomic64_t total_hit_ext;		/* # of lookup extent cache */
	atomic64_t read_hit_rbtree;		/* # of hit rbtree extent node */
	atomic64_t read_hit_largest;		/* # of hit largest extent node */
	atomic64_t read_hit_cached;		/* # of hit cached extent node */
	atomic_t inline_xattr;			/* # of inline_xattr inodes */
	atomic_t inline_inode;			/* # of inline_data inodes */
	atomic_t inline_dir;			/* # of inline_dentry inodes */
	atomic_t aw_cnt;			/* # of atomic writes */
	atomic_t vw_cnt;			/* # of volatile writes */
	atomic_t max_aw_cnt;			/* max # of atomic writes */
	atomic_t max_vw_cnt;			/* max # of volatile writes */
	int bg_gc;				/* background gc calls */
	unsigned int io_skip_bggc;		/* skip background gc for in-flight IO */
	unsigned int other_skip_bggc;		/* skip background gc for other reasons */
	int assr_lfs_segs, assr_ssr_segs, assr_lfs_blks, assr_ssr_blks;
	int bggc_node_lfs_blks, bggc_node_ssr_blks;
	unsigned int ndirty_inode[NR_INODE_TYPE];	/* # of dirty inodes */
        struct mutex bd_mutex;
        struct f2fs_bigdata_info *bd_info;      /* big data collections */
	struct gc_stat gc_stat;
	int gc_stat_enable;
#endif
	atomic_t need_ssr_gc;
	spinlock_t stat_lock;			/* lock for stat operations */

	/* For app/fs IO statistics */
	spinlock_t iostat_lock;
	unsigned long long write_iostat[NR_IO_TYPE];
	bool iostat_enable;

	/* For sysfs suppport */
	struct kobject s_kobj;
	struct completion s_kobj_unregister;

	/* For shrinker support */
	struct list_head s_list;
	int s_ndevs;				/* number of devices */
	struct f2fs_dev_info *devs;		/* for device list */
	unsigned int dirty_device;		/* for checkpoint data flush */
	spinlock_t dev_lock;			/* protect dirty_device */
	struct mutex umount_mutex;
	unsigned int shrinker_run_no;

	/* For write statistics */
	u64 sectors_written_start;
	u64 kbytes_written;
	u64 blocks_gc_written;

	/* Reference to checksum algorithm driver via cryptoapi */
	struct crypto_shash *s_chksum_driver;

	/* Precomputed FS UUID checksum for seeding other checksums */
	__u32 s_chksum_seed;
#if CONFIG_HMFS_FS_ENCRYPTION
	unsigned char encryption_ver;
#endif
#if F2FS_FS_SDP_ENCRYPTION
	const struct f2fs_sdp_fscrypt_operations *s_sdp_cop;
#endif
	bool is_extra_flag_set;

/* should make sure everything is ready and safe for hmfs_print_sbi_info */
        bool print_sbi_safe;
       /* For need ssr parameters */
#ifdef CONFIG_HMFS_GRADING_SSR
        struct f2fs_hot_cold_params hot_cold_params;
#endif
#ifndef CONFIG_HMFS_CHECK_FS
        struct workqueue_struct *need_fsck_wq;
        struct need_fsck_work_struct need_fsck_work;
#endif
        bool is_frag;                           /* urgent gc flag */
        unsigned long last_urgent_check;        /* last urgent check jiffies */

#ifdef CONFIG_HMFS_CHECK_FS
	atomic_t in_cp;
#endif


#ifdef CONFIG_FSCK_BOOST
	int inited;
#endif
	struct hmfs_gc_control_info gc_control_info;
	struct workqueue_struct *need_bkops_wq;
	struct delayed_work start_bkops_work;

	struct slc_mode_control_info slc_mode_ctrl;

	struct hmfs_oob_control_info oob_ctrl[STREAM_NR];
	atomic_t oob_wr_cnt[NR_CURSEG_TYPE];	/* write section count after CP */
	bool oob_enable;
	bool skip_sync_node;

	/* the max time in usec that io will be restricted */
	unsigned int io_throttle_time;
	unsigned int io_throttle_time_max;

	atomic64_t pu_align_info[PU_ALIGN_NR];	/* stat pu align of IOs */
	int pu_size[NR_FLASH_MODE];		/* for TLC passthrough in future */
	bool skip_pu_align;			/* use for debug */

	/* threshold for writing checkpoint during gc */
	unsigned int prefree_sec_threshold;
};

#ifdef CONFIG_HMFS_FAULT_INJECTION
#define f2fs_show_injection_info(type)					\
	printk_ratelimited("%sHMFS-fs : inject %s in %s of %pF\n",	\
		KERN_INFO, f2fs_fault_name[type],			\
		__func__, __builtin_return_address(0))
static inline bool time_to_inject(struct f2fs_sb_info *sbi, int type)
{
	struct f2fs_fault_info *ffi = &F2FS_OPTION(sbi).fault_info;

	if (!ffi->inject_rate)
		return false;

	if (!IS_FAULT_SET(ffi, type))
		return false;

	atomic_inc(&ffi->inject_ops);
	if (atomic_read(&ffi->inject_ops) >= ffi->inject_rate) {
		atomic_set(&ffi->inject_ops, 0);
		return true;
	}
	return false;
}
#else
#define f2fs_show_injection_info(type) do { } while (0)
static inline bool time_to_inject(struct f2fs_sb_info *sbi, int type)
{
	return false;
}
#endif

/* For write statistics. Suppose sector size is 512 bytes,
 * and the return value is in kbytes. s is of struct f2fs_sb_info.
 */
#define BD_PART_WRITTEN(s)						 \
(((u64)part_stat_read((s)->sb->s_bdev->bd_part, sectors[1]) -		 \
		(s)->sectors_written_start) >> 1)
/* Lifetime write IO statistic takes up 8 types on disk,
 * and is recorded right before CRC of cp block 2.
 */
#define SIZEOF_KBYTES_WRITTEN  8

static inline void f2fs_update_time(struct f2fs_sb_info *sbi, int type)
{
	unsigned long now = jiffies;

	sbi->last_time[type] = now;

	/* DISCARD_TIME and GC_TIME are based on REQ_TIME */
	if (type == REQ_TIME) {
		sbi->last_time[DISCARD_TIME] = now;
		sbi->last_time[GC_TIME] = now;
	}
}

static inline bool f2fs_time_over(struct f2fs_sb_info *sbi, int type)
{
	unsigned long interval = sbi->interval_time[type] * HZ;

	return time_after(jiffies, sbi->last_time[type] + interval);
}

static inline unsigned int f2fs_time_to_wait(struct f2fs_sb_info *sbi,
						int type)
{
	unsigned long interval = sbi->interval_time[type] * HZ;
	unsigned int wait_ms = 0;
	long delta;

	delta = (sbi->last_time[type] + interval) - jiffies;
	if (delta > 0)
		wait_ms = jiffies_to_msecs(delta);

	return wait_ms;
}

/*
 * Inline functions
 */
static inline u32 __f2fs_crc32(struct f2fs_sb_info *sbi, u32 crc,
			      const void *address, unsigned int length)
{
	struct {
		struct shash_desc shash;
		char ctx[4];
	} desc;
	int err;

	BUG_ON(crypto_shash_descsize(sbi->s_chksum_driver) != sizeof(desc.ctx));

	desc.shash.tfm = sbi->s_chksum_driver;
	desc.shash.flags = 0;
	*(u32 *)desc.ctx = crc;

	err = crypto_shash_update(&desc.shash, address, length);
	BUG_ON(err);

	return *(u32 *)desc.ctx;
}

static inline u32 f2fs_crc32(struct f2fs_sb_info *sbi, const void *address,
			   unsigned int length)
{
	return __f2fs_crc32(sbi, HMFS_SUPER_MAGIC, address, length);
}

static inline bool f2fs_crc_valid(struct f2fs_sb_info *sbi, __u32 blk_crc,
				  void *buf, size_t buf_size)
{
	return f2fs_crc32(sbi, buf, buf_size) == blk_crc;
}

static inline u32 f2fs_chksum(struct f2fs_sb_info *sbi, u32 crc,
			      const void *address, unsigned int length)
{
	return __f2fs_crc32(sbi, crc, address, length);
}

static inline struct f2fs_inode_info *F2FS_I(struct inode *inode)
{
	return container_of(inode, struct f2fs_inode_info, vfs_inode);
}

static inline struct f2fs_sb_info *F2FS_SB(struct super_block *sb)
{
	return sb->s_fs_info;
}

static inline struct f2fs_sb_info *F2FS_I_SB(struct inode *inode)
{
	return F2FS_SB(inode->i_sb);
}

static inline struct f2fs_sb_info *F2FS_M_SB(struct address_space *mapping)
{
	return F2FS_I_SB(mapping->host);
}

static inline struct f2fs_sb_info *F2FS_P_SB(struct page *page)
{
	return F2FS_M_SB(page->mapping);
}

static inline struct f2fs_super_block *F2FS_RAW_SUPER(struct f2fs_sb_info *sbi)
{
	return (struct f2fs_super_block *)(sbi->raw_super);
}

static inline struct f2fs_checkpoint *F2FS_CKPT(struct f2fs_sb_info *sbi)
{
	return (struct f2fs_checkpoint *)(sbi->ckpt);
}

static inline struct f2fs_node *F2FS_NODE(struct page *page)
{
	return (struct f2fs_node *)page_address(page);
}

static inline struct f2fs_inode *F2FS_INODE(struct page *page)
{
	return &((struct f2fs_node *)page_address(page))->i;
}

static inline struct f2fs_nm_info *NM_I(struct f2fs_sb_info *sbi)
{
	return (struct f2fs_nm_info *)(sbi->nm_info);
}

static inline struct f2fs_sm_info *SM_I(struct f2fs_sb_info *sbi)
{
	return (struct f2fs_sm_info *)(sbi->sm_info);
}

static inline struct sit_info *SIT_I(struct f2fs_sb_info *sbi)
{
	return (struct sit_info *)(SM_I(sbi)->sit_info);
}

static inline struct free_segmap_info *FREE_I(struct f2fs_sb_info *sbi)
{
	return (struct free_segmap_info *)(SM_I(sbi)->free_info);
}

static inline struct dirty_seglist_info *DIRTY_I(struct f2fs_sb_info *sbi)
{
	return (struct dirty_seglist_info *)(SM_I(sbi)->dirty_info);
}

static inline struct address_space *META_MAPPING(struct f2fs_sb_info *sbi)
{
	return sbi->meta_inode->i_mapping;
}

static inline struct address_space *NODE_MAPPING(struct f2fs_sb_info *sbi)
{
	return sbi->node_inode->i_mapping;
}

#ifdef CONFIG_FSCK_BOOST
static inline struct address_space *BDEV_MAPPING(struct f2fs_sb_info *sbi)
{
	return sbi->sb->s_bdev->bd_inode->i_mapping;
}
#endif

static inline bool is_sbi_flag_set(struct f2fs_sb_info *sbi, unsigned int type)
{
	return test_bit(type, &sbi->s_flag);
}

#include "f2fs_dump_info.h"

static inline void set_extra_flag(struct f2fs_sb_info *sbi, int flag)
{
	struct extra_flags_block *ef_blk;
	block_t cp_blkaddr = le32_to_cpu(sbi->raw_super->cp_blkaddr);
	struct buffer_head *bh;
	__u32 crc32 = 0;

	might_sleep();

	if (sbi->is_extra_flag_set)
		return;

	/* get the last block in CP 0 segment */
	bh = sb_bread(sbi->sb, cp_blkaddr + sbi->blocks_per_seg - 1);
	if (!bh) {
		pr_err("Cannot get buffer_head of extra_flags_block\n");
		return;
	}

	lock_buffer(bh);
	ef_blk = (struct extra_flags_block *)bh->b_data;
	switch (flag) {
	case EXTRA_NEED_FSCK_FLAG:
		if (ef_blk->need_fsck == cpu_to_le32(EXTRA_NEED_FSCK_FLAG)) {
			unlock_buffer(bh);
			brelse(bh);
			return;
		}
		ef_blk->need_fsck = cpu_to_le32(EXTRA_NEED_FSCK_FLAG);
		break;
	}
	crc32 = f2fs_crc32(sbi, ef_blk, sizeof(struct extra_flags_block) - 4);
	ef_blk->crc = cpu_to_le32(crc32);
	set_buffer_uptodate(bh);
	set_buffer_dirty(bh);
	unlock_buffer(bh);
	__sync_dirty_buffer(bh, WRITE_FLUSH_FUA);

	brelse(bh);
	sbi->is_extra_flag_set = 1;
}

static inline void set_sbi_flag(struct f2fs_sb_info *sbi, unsigned int type)
{
	if (!sbi)
		return;

	set_bit(type, &sbi->s_flag);

#ifndef CONFIG_HMFS_CHECK_FS
	if (type == SBI_NEED_FSCK && !sbi->is_extra_flag_set &&
						sbi->need_fsck_wq) {
		if (system_state > SYSTEM_RUNNING)
			return;
		queue_work(sbi->need_fsck_wq, &sbi->need_fsck_work.work);
	}
#endif
}

static inline void clear_sbi_flag(struct f2fs_sb_info *sbi, unsigned int type)
{
	clear_bit(type, &sbi->s_flag);
}

static inline unsigned long long cur_cp_version(struct f2fs_checkpoint *cp)
{
	return le64_to_cpu(cp->checkpoint_ver);
}

static inline unsigned long f2fs_qf_ino(struct super_block *sb, int type)
{
	if (type < F2FS_MAX_QUOTAS)
		return le32_to_cpu(F2FS_SB(sb)->raw_super->qf_ino[type]);
	return 0;
}

static inline __u64 cur_cp_crc(struct f2fs_checkpoint *cp)
{
	size_t crc_offset = le32_to_cpu(cp->checksum_offset);
	return le32_to_cpu(*((__le32 *)((unsigned char *)cp + crc_offset)));
}

static inline bool __is_set_ckpt_flags(struct f2fs_checkpoint *cp, unsigned int f)
{
	unsigned int ckpt_flags = le32_to_cpu(cp->ckpt_flags);

	return ckpt_flags & f;
}

static inline bool is_set_ckpt_flags(struct f2fs_sb_info *sbi, unsigned int f)
{
	return __is_set_ckpt_flags(F2FS_CKPT(sbi), f);
}

static inline void __set_ckpt_flags(struct f2fs_checkpoint *cp, unsigned int f)
{
	unsigned int ckpt_flags;

	ckpt_flags = le32_to_cpu(cp->ckpt_flags);
	ckpt_flags |= f;
	cp->ckpt_flags = cpu_to_le32(ckpt_flags);
}

static inline void set_ckpt_flags(struct f2fs_sb_info *sbi, unsigned int f)
{
	unsigned long flags;

	spin_lock_irqsave(&sbi->cp_lock, flags);
	__set_ckpt_flags(F2FS_CKPT(sbi), f);
	spin_unlock_irqrestore(&sbi->cp_lock, flags);
}

static inline void __clear_ckpt_flags(struct f2fs_checkpoint *cp, unsigned int f)
{
	unsigned int ckpt_flags;

	ckpt_flags = le32_to_cpu(cp->ckpt_flags);
	ckpt_flags &= (~f);
	cp->ckpt_flags = cpu_to_le32(ckpt_flags);
}

static inline void clear_ckpt_flags(struct f2fs_sb_info *sbi, unsigned int f)
{
	unsigned long flags;

	spin_lock_irqsave(&sbi->cp_lock, flags);
	__clear_ckpt_flags(F2FS_CKPT(sbi), f);
	spin_unlock_irqrestore(&sbi->cp_lock, flags);
}

static inline void disable_nat_bits(struct f2fs_sb_info *sbi, bool lock)
{
	unsigned long flags;

	set_bit(SBI_NEED_FSCK, &sbi->s_flag);

	if (lock)
		spin_lock_irqsave(&sbi->cp_lock, flags);
	__clear_ckpt_flags(F2FS_CKPT(sbi), CP_NAT_BITS_FLAG);
	kfree(NM_I(sbi)->nat_bits);
	NM_I(sbi)->nat_bits = NULL;
	if (lock)
		spin_unlock_irqrestore(&sbi->cp_lock, flags);
}

static inline bool enabled_nat_bits(struct f2fs_sb_info *sbi,
					struct cp_control *cpc)
{
	return 0;
}

static inline void f2fs_lock_op(struct f2fs_sb_info *sbi)
{
#if 1
	spin_lock(&sbi->cp_rwsem_lock);
	if (sbi->cp_rwsem_owner &&
		time_after(jiffies, sbi->cp_rwsem_owner_j + 10 * HZ)) {
		show_stack(current, NULL);
		printk(KERN_ERR "cp_rwsem_timeout: start: %lu now: %lu\n",
			sbi->cp_rwsem_owner_j, jiffies);
		show_stack(sbi->cp_rwsem_owner, NULL);
	}
	spin_unlock(&sbi->cp_rwsem_lock);
#endif
	down_read(&sbi->cp_rwsem);
}

static inline int f2fs_trylock_op(struct f2fs_sb_info *sbi)
{
	return down_read_trylock(&sbi->cp_rwsem);
}

static inline void f2fs_unlock_op(struct f2fs_sb_info *sbi)
{
	up_read(&sbi->cp_rwsem);
}

static inline void f2fs_lock_all(struct f2fs_sb_info *sbi)
{
	down_write(&sbi->cp_rwsem);
#if 1
	spin_lock(&sbi->cp_rwsem_lock);
	sbi->cp_rwsem_owner = current;
	sbi->cp_rwsem_owner_j = jiffies;
	spin_unlock(&sbi->cp_rwsem_lock);
#endif
}

static inline void f2fs_unlock_all(struct f2fs_sb_info *sbi)
{
#if 1
	spin_lock(&sbi->cp_rwsem_lock);
	sbi->cp_rwsem_owner = NULL;
	sbi->cp_rwsem_owner_j = 0;
	spin_unlock(&sbi->cp_rwsem_lock);
#endif
	up_write(&sbi->cp_rwsem);
}

static inline int __get_cp_reason(struct f2fs_sb_info *sbi)
{
	int reason = CP_SYNC;

	if (test_opt(sbi, FASTBOOT))
		reason = CP_FASTBOOT;
	if (is_sbi_flag_set(sbi, SBI_IS_CLOSE))
		reason = CP_UMOUNT;
	return reason;
}

static inline bool __remain_node_summaries(int reason)
{
	return (reason & (CP_UMOUNT | CP_FASTBOOT));
}

static inline bool __exist_node_summaries(struct f2fs_sb_info *sbi)
{
	return (is_set_ckpt_flags(sbi, CP_UMOUNT_FLAG) ||
			is_set_ckpt_flags(sbi, CP_FASTBOOT_FLAG));
}

/*
 * Handling some dquot errors caused by quota file inconsistency
 */
static inline int f2fs_dquot_initialize(struct inode *inode)
{
	int err = dquot_initialize(inode);

	if (err == -EDQUOT || err == -ENOSPC || err == -EIO)
		err = 0;

	return err;
}

static inline int f2fs_dquot_transfer(struct inode *inode, struct iattr *iattr)
{
	int err = dquot_transfer(inode, iattr);

	if (err == -EDQUOT || err == -ENOSPC || err == -EIO)
		err = 0;

	return err;
}

static inline int f2fs_dquot_alloc_inode(struct inode *inode)
{
	int err = dquot_alloc_inode(inode);

	if (err == -EDQUOT || err == -ENOSPC || err == -EIO)
		err = 0;

	return err;
}

static inline int f2fs_dquot_reserve_block(struct inode *inode, qsize_t count)
{
	int err = dquot_reserve_block(inode, count);

	if (err == -EDQUOT || err == -ENOSPC || err == -EIO)
		err = 0;

	return err;
}

/*
 * Check whether the inode has blocks or not
 */
static inline int F2FS_HAS_BLOCKS(struct inode *inode)
{
	block_t xattr_block = F2FS_I(inode)->i_xattr_nid ? 1 : 0;

	return (inode->i_blocks >> F2FS_LOG_SECTORS_PER_BLOCK) > xattr_block;
}

static inline bool f2fs_has_xattr_block(unsigned int ofs)
{
	return ofs == XATTR_NODE_OFFSET;
}

static inline bool __allow_reserved_blocks(struct f2fs_sb_info *sbi,
					struct inode *inode, bool cap)
{
	if (!inode)
		return true;
	if (!test_opt(sbi, RESERVE_ROOT))
		return false;
	if (IS_NOQUOTA(inode))
		return true;
	if (uid_eq(F2FS_OPTION(sbi).s_resuid, current_fsuid()))
		return true;
	if (!gid_eq(F2FS_OPTION(sbi).s_resgid, GLOBAL_ROOT_GID) &&
					in_group_p(F2FS_OPTION(sbi).s_resgid))
		return true;
	if (cap && capable(CAP_SYS_RESOURCE))
		return true;
	return false;
}


static inline void f2fs_i_blocks_write(struct inode *, block_t, bool, bool);
static inline int inc_valid_block_count(struct f2fs_sb_info *sbi,
				 struct inode *inode, blkcnt_t *count)
{
	blkcnt_t diff = 0, release = 0;
	block_t avail_user_block_count;
	int ret;

	ret = f2fs_dquot_reserve_block(inode, *count);
	if (ret)
		return ret;

	if (time_to_inject(sbi, FAULT_BLOCK)) {
		f2fs_show_injection_info(FAULT_BLOCK);
		release = *count;
		goto enospc;
	}

	/*
	 * let's increase this in prior to actual block count change in order
	 * for hmfs_sync_file to avoid data races when deciding checkpoint.
	 */
	percpu_counter_add(&sbi->alloc_valid_block_count, (*count));

	spin_lock(&sbi->stat_lock);
	sbi->total_valid_block_count += (block_t)(*count);
	avail_user_block_count = sbi->user_block_count -
					sbi->current_reserved_blocks;

	if (!__allow_reserved_blocks(sbi, inode, true))
		avail_user_block_count -= F2FS_OPTION(sbi).root_reserved_blocks;
	if (unlikely(is_sbi_flag_set(sbi, SBI_CP_DISABLED)))
		avail_user_block_count -= sbi->unusable_block_count;
	if (unlikely(sbi->total_valid_block_count > avail_user_block_count)) {
		diff = sbi->total_valid_block_count - avail_user_block_count;
		if (diff > *count)
			diff = *count;
		*count -= diff;
		release = diff;
		sbi->total_valid_block_count -= diff;
		if (!*count) {
			spin_unlock(&sbi->stat_lock);
			goto enospc;
		}
	}
	spin_unlock(&sbi->stat_lock);

	if (unlikely(release)) {
		percpu_counter_sub(&sbi->alloc_valid_block_count, release);
		dquot_release_reservation_block(inode, release);
	}
	f2fs_i_blocks_write(inode, *count, true, true);
	return 0;

enospc:
	percpu_counter_sub(&sbi->alloc_valid_block_count, release);
	dquot_release_reservation_block(inode, release);
	return -ENOSPC;
}

static inline void dec_valid_block_count(struct f2fs_sb_info *sbi,
						struct inode *inode,
						block_t count)
{
	blkcnt_t sectors = count << F2FS_LOG_SECTORS_PER_BLOCK;

	spin_lock(&sbi->stat_lock);
	f2fs_bug_on(sbi, sbi->total_valid_block_count < (block_t) count);
	f2fs_bug_on(sbi, inode->i_blocks < sectors);
	sbi->total_valid_block_count -= (block_t)count;
	if (sbi->reserved_blocks &&
		sbi->current_reserved_blocks < sbi->reserved_blocks)
		sbi->current_reserved_blocks = min(sbi->reserved_blocks,
					sbi->current_reserved_blocks + count);
	spin_unlock(&sbi->stat_lock);
	f2fs_i_blocks_write(inode, count, false, true);
}

static inline block_t valid_user_blocks(struct f2fs_sb_info *sbi)
{
	return sbi->total_valid_block_count;
}

static inline block_t discard_blocks(struct f2fs_sb_info *sbi)
{
	return sbi->discard_blks;
}

static inline unsigned long __bitmap_size(struct f2fs_sb_info *sbi, int flag)
{
	struct f2fs_checkpoint *ckpt = F2FS_CKPT(sbi);

	/* return NAT or SIT bitmap */
	if (flag == NAT_BITMAP)
		return le32_to_cpu(ckpt->nat_ver_bitmap_bytesize);
	else if (flag == SIT_BITMAP)
		return le32_to_cpu(ckpt->sit_ver_bitmap_bytesize);

	return 0;
}

static inline block_t __cp_payload(struct f2fs_sb_info *sbi)
{
	return le32_to_cpu(F2FS_RAW_SUPER(sbi)->cp_payload);
}

static inline void *__bitmap_ptr(struct f2fs_sb_info *sbi, int flag)
{
	struct f2fs_checkpoint *ckpt = F2FS_CKPT(sbi);
	int offset;

	if (is_set_ckpt_flags(sbi, CP_LARGE_NAT_BITMAP_FLAG)) {
		offset = (flag == SIT_BITMAP) ?
			le32_to_cpu(ckpt->nat_ver_bitmap_bytesize) : 0;
		return &ckpt->sit_nat_version_bitmap + offset;
	}

	if (__cp_payload(sbi) > 0) {
		if (flag == NAT_BITMAP)
			return &ckpt->sit_nat_version_bitmap;
		else
			return (unsigned char *)ckpt + F2FS_BLKSIZE;
	} else {
		offset = (flag == NAT_BITMAP) ?
			le32_to_cpu(ckpt->sit_ver_bitmap_bytesize) : 0;
		return &ckpt->sit_nat_version_bitmap + offset;
	}
}

static inline block_t __start_cp_addr(struct f2fs_sb_info *sbi)
{
	block_t start_addr = le32_to_cpu(F2FS_RAW_SUPER(sbi)->cp_blkaddr);

	if (sbi->cur_cp_pack == 2)
		start_addr += sbi->blocks_per_seg;
	return start_addr;
}

static inline block_t __start_cp_next_addr(struct f2fs_sb_info *sbi)
{
	block_t start_addr = le32_to_cpu(F2FS_RAW_SUPER(sbi)->cp_blkaddr);

	if (sbi->cur_cp_pack == 1)
		start_addr += sbi->blocks_per_seg;
	return start_addr;
}

static inline void __set_cp_next_pack(struct f2fs_sb_info *sbi)
{
	sbi->cur_cp_pack = (sbi->cur_cp_pack == 1) ? 2 : 1;
}

static inline block_t __start_sum_addr(struct f2fs_sb_info *sbi)
{
	return le32_to_cpu(F2FS_CKPT(sbi)->cp_pack_start_sum);
}

static inline int inc_valid_node_count(struct f2fs_sb_info *sbi,
					struct inode *inode, bool is_inode)
{
	block_t	valid_block_count;
	unsigned int valid_node_count;
	int err;

	if (is_inode) {
		if (inode) {
			err = dquot_alloc_inode(inode);
			if (err)
				return err;
		}
	} else {
		err = f2fs_dquot_reserve_block(inode, 1);
		if (err)
			return err;
	}

	if (time_to_inject(sbi, FAULT_BLOCK)) {
		f2fs_show_injection_info(FAULT_BLOCK);
		goto enospc;
	}

	spin_lock(&sbi->stat_lock);

	valid_block_count = sbi->total_valid_block_count +
					sbi->current_reserved_blocks + 1;

	if (!__allow_reserved_blocks(sbi, inode, false))
		valid_block_count += F2FS_OPTION(sbi).root_reserved_blocks;
	if (unlikely(is_sbi_flag_set(sbi, SBI_CP_DISABLED)))
		valid_block_count += sbi->unusable_block_count;

	if (unlikely(valid_block_count > sbi->user_block_count)) {
		spin_unlock(&sbi->stat_lock);
		goto enospc;
	}

	valid_node_count = sbi->total_valid_node_count + 1;
	if (unlikely(valid_node_count > sbi->total_node_count)) {
		spin_unlock(&sbi->stat_lock);
		goto enospc;
	}

	sbi->total_valid_node_count++;
	sbi->total_valid_block_count++;
	spin_unlock(&sbi->stat_lock);

	if (inode) {
		if (is_inode)
			hmfs_mark_inode_dirty_sync(inode, true);
		else
			f2fs_i_blocks_write(inode, 1, true, true);
	}

	percpu_counter_inc(&sbi->alloc_valid_block_count);
	return 0;

enospc:
	if (is_inode) {
		if (inode)
			dquot_free_inode(inode);
	} else {
		dquot_release_reservation_block(inode, 1);
	}
	return -ENOSPC;
}

static inline void dec_valid_node_count(struct f2fs_sb_info *sbi,
					struct inode *inode, bool is_inode)
{
	spin_lock(&sbi->stat_lock);

	f2fs_bug_on(sbi, !sbi->total_valid_block_count);
	f2fs_bug_on(sbi, !sbi->total_valid_node_count);
	f2fs_bug_on(sbi, !is_inode && !inode->i_blocks);

	sbi->total_valid_node_count--;
	sbi->total_valid_block_count--;
	if (sbi->reserved_blocks &&
		sbi->current_reserved_blocks < sbi->reserved_blocks)
		sbi->current_reserved_blocks++;

	spin_unlock(&sbi->stat_lock);

	if (is_inode)
		dquot_free_inode(inode);
	else
		f2fs_i_blocks_write(inode, 1, false, true);
}

static inline unsigned int valid_node_count(struct f2fs_sb_info *sbi)
{
	return sbi->total_valid_node_count;
}

static inline void inc_valid_inode_count(struct f2fs_sb_info *sbi)
{
	percpu_counter_inc(&sbi->total_valid_inode_count);
}

static inline void dec_valid_inode_count(struct f2fs_sb_info *sbi)
{
	percpu_counter_dec(&sbi->total_valid_inode_count);
}

static inline s64 valid_inode_count(struct f2fs_sb_info *sbi)
{
	return percpu_counter_sum_positive(&sbi->total_valid_inode_count);
}

static inline struct page *f2fs_grab_cache_page(struct address_space *mapping,
						pgoff_t index, bool for_write)
{
	struct page *page;

	if (IS_ENABLED(CONFIG_HMFS_FAULT_INJECTION)) {
		if (!for_write)
			page = find_get_page_flags(mapping, index,
							FGP_LOCK | FGP_ACCESSED);
		else
			page = find_lock_page(mapping, index);
		if (page)
			return page;

		if (time_to_inject(F2FS_M_SB(mapping), FAULT_PAGE_ALLOC)) {
			f2fs_show_injection_info(FAULT_PAGE_ALLOC);
			return NULL;
		}
	}

	if (!for_write)
		return grab_cache_page(mapping, index);
	return grab_cache_page_write_begin(mapping, index, AOP_FLAG_NOFS);
}

static inline struct page *f2fs_pagecache_get_page(
				struct address_space *mapping, pgoff_t index,
				int fgp_flags, gfp_t gfp_mask)
{
	if (time_to_inject(F2FS_M_SB(mapping), FAULT_PAGE_GET)) {
		f2fs_show_injection_info(FAULT_PAGE_GET);
		return NULL;
	}

	return pagecache_get_page(mapping, index, fgp_flags, gfp_mask);
}

static inline void f2fs_copy_page(struct page *src, struct page *dst)
{
	char *src_kaddr = kmap(src);
	char *dst_kaddr = kmap(dst);

	memcpy(dst_kaddr, src_kaddr, PAGE_SIZE);
	kunmap(dst);
	kunmap(src);
}

static inline int is_f2fs_page(struct page *page)
{
	return page->mapping->host->i_sb->s_magic == HMFS_SUPER_MAGIC;
}

static inline void f2fs_put_page(struct page *page, int unlock)
{
	if (!page)
		return;

	if (unlock) {
		if (is_f2fs_page(page))
			f2fs_bug_on(F2FS_P_SB(page), !PageLocked(page));
		else
			f2fs_bug_on(NULL, !PageLocked(page));
		unlock_page(page);
	}
	put_page(page);
}

static inline void f2fs_put_dnode(struct dnode_of_data *dn)
{
	if (dn->node_page)
		f2fs_put_page(dn->node_page, 1);
	if (dn->inode_page && dn->node_page != dn->inode_page)
		f2fs_put_page(dn->inode_page, 0);
	dn->node_page = NULL;
	dn->inode_page = NULL;
}

static inline struct kmem_cache *f2fs_kmem_cache_create(const char *name,
					size_t size)
{
	return kmem_cache_create(name, size, 0, SLAB_RECLAIM_ACCOUNT, NULL);
}

static inline void *f2fs_kmem_cache_alloc(struct kmem_cache *cachep,
						gfp_t flags)
{
	void *entry;

	entry = kmem_cache_alloc(cachep, flags);
	if (!entry)
		entry = kmem_cache_alloc(cachep, flags | __GFP_NOFAIL);
	return entry;
}

extern struct bio_set *hmfs_bioset;
#define HMFS_BIO_DM_REF		(1ULL << 0)

struct hmfs_bio {
	u8 flags;
	unsigned int mapped_blks;
	struct f2fs_sb_info *sbi;
	struct bio bio;
};

static inline struct hmfs_bio *HMFS_BIO(struct bio *bio)
{
	return container_of(bio, struct hmfs_bio, bio);
}

static inline struct bio *f2fs_bio_alloc(struct f2fs_sb_info *sbi,
						int npages, bool no_fail)
{
	struct bio *bio;

	if (no_fail) {
		/* No failure on bio allocation */
		bio = bio_alloc_bioset(GFP_NOIO, npages, hmfs_bioset);
		if (!bio)
			bio = bio_alloc_bioset(GFP_NOIO | __GFP_NOFAIL,
					npages, hmfs_bioset);
		if (bio)
			memset(HMFS_BIO(bio), 0, offsetof(struct hmfs_bio, bio));
#ifdef CONFIG_MAS_BLK
		if (bio)
			bio->dump_fs = hmfs_print_frag_info;
#endif
		return bio;
	}
	if (time_to_inject(sbi, FAULT_ALLOC_BIO)) {
		f2fs_show_injection_info(FAULT_ALLOC_BIO);
		return NULL;
	}

	bio = bio_alloc_bioset(GFP_KERNEL, npages, hmfs_bioset);
	if (bio)
		memset(HMFS_BIO(bio), 0, offsetof(struct hmfs_bio, bio));
#ifdef CONFIG_MAS_BLK
	if (bio)
		bio->dump_fs = hmfs_print_frag_info;
#endif
	return bio;
}

static inline void f2fs_radix_tree_insert(struct radix_tree_root *root,
				unsigned long index, void *item)
{
	while (radix_tree_insert(root, index, item))
		cond_resched();
}

#define RAW_IS_INODE(p)	((p)->footer.nid == (p)->footer.ino)

static inline bool IS_INODE(struct page *page)
{
	struct f2fs_node *p = F2FS_NODE(page);

	return RAW_IS_INODE(p);
}

static inline int offset_in_addr(struct f2fs_inode *i)
{
	return (i->i_inline & F2FS_EXTRA_ATTR) ?
			(le16_to_cpu(i->i_extra_isize) / sizeof(__le32)) : 0;
}

static inline __le32 *blkaddr_in_node(struct f2fs_node *node)
{
	return RAW_IS_INODE(node) ? node->i.i_addr : node->dn.addr;
}

static inline int f2fs_has_extra_attr(struct inode *inode);
static inline block_t datablock_addr(struct inode *inode,
			struct page *node_page, unsigned int offset)
{
	struct f2fs_node *raw_node;
	__le32 *addr_array;
	int base = 0;
	bool is_inode = IS_INODE(node_page);

	raw_node = F2FS_NODE(node_page);

	/* from GC path only */
	if (is_inode) {
		if (!inode)
			base = offset_in_addr(&raw_node->i);
		else if (f2fs_has_extra_attr(inode))
			base = get_extra_isize(inode);
	}

	addr_array = blkaddr_in_node(raw_node);
	return le32_to_cpu(addr_array[base + offset]);
}

static inline int f2fs_test_bit(unsigned int nr, char *addr)
{
	int mask;

	addr += (nr >> 3);
	mask = 1 << (7 - (nr & 0x07));
	return mask & *addr;
}

static inline void f2fs_set_bit(unsigned int nr, char *addr)
{
	int mask;

	addr += (nr >> 3);
	mask = 1 << (7 - (nr & 0x07));
	*addr |= mask;
}

static inline void f2fs_clear_bit(unsigned int nr, char *addr)
{
	int mask;

	addr += (nr >> 3);
	mask = 1 << (7 - (nr & 0x07));
	*addr &= ~mask;
}

static inline int f2fs_test_and_set_bit(unsigned int nr, char *addr)
{
	int mask;
	int ret;

	addr += (nr >> 3);
	mask = 1 << (7 - (nr & 0x07));
	ret = mask & *addr;
	*addr |= mask;
	return ret;
}

static inline int f2fs_test_and_clear_bit(unsigned int nr, char *addr)
{
	int mask;
	int ret;

	addr += (nr >> 3);
	mask = 1 << (7 - (nr & 0x07));
	ret = mask & *addr;
	*addr &= ~mask;
	return ret;
}

static inline void f2fs_change_bit(unsigned int nr, char *addr)
{
	int mask;

	addr += (nr >> 3);
	mask = 1 << (7 - (nr & 0x07));
	*addr ^= mask;
}

/*
 * Inode flags
 */
#define F2FS_SECRM_FL			0x00000001 /* Secure deletion */
#define F2FS_UNRM_FL			0x00000002 /* Undelete */
#define F2FS_COMPR_FL			0x00000004 /* Compress file */
#define F2FS_SYNC_FL			0x00000008 /* Synchronous updates */
#define F2FS_IMMUTABLE_FL		0x00000010 /* Immutable file */
#define F2FS_APPEND_FL			0x00000020 /* writes to file may only append */
#define F2FS_NODUMP_FL			0x00000040 /* do not dump file */
#define F2FS_NOATIME_FL			0x00000080 /* do not update atime */
/* Reserved for compression usage... */
#define F2FS_DIRTY_FL			0x00000100
#define F2FS_COMPRBLK_FL		0x00000200 /* One or more compressed clusters */
#define F2FS_NOCOMPR_FL			0x00000400 /* Don't compress */
#define F2FS_ENCRYPT_FL			0x00000800 /* encrypted file */
/* End compression flags --- maybe not all used */
#define F2FS_INDEX_FL			0x00001000 /* hash-indexed directory */
#define F2FS_IMAGIC_FL			0x00002000 /* AFS directory */
#define F2FS_JOURNAL_DATA_FL		0x00004000 /* file data should be journaled */
#define F2FS_NOTAIL_FL			0x00008000 /* file tail should not be merged */
#define F2FS_DIRSYNC_FL			0x00010000 /* dirsync behaviour (directories only) */
#define F2FS_TOPDIR_FL			0x00020000 /* Top of directory hierarchies*/
#define F2FS_HUGE_FILE_FL               0x00040000 /* Set to each huge file */
#define F2FS_EXTENTS_FL			0x00080000 /* Inode uses extents */
#define F2FS_EA_INODE_FL	        0x00200000 /* Inode used for large EA */
#define F2FS_EOFBLOCKS_FL		0x00400000 /* Blocks allocated beyond EOF */
#define F2FS_NOCOW_FL			0x00800000 /* Do not cow file */
#define F2FS_INLINE_DATA_FL		0x10000000 /* Inode has inline data. */
#define F2FS_PROJINHERIT_FL		0x20000000 /* Create with parents projid */
#define F2FS_RESERVED_FL		0x80000000 /* reserved for ext4 lib */

#ifdef CONFIG_ACM
#define F2FS_UNRM_PHOTO_FL	FS_UNRM_FL
#define F2FS_UNRM_VIDEO_FL	0x00100000
#define F2FS_UNRM_DMD_PHOTO_FL	0x02000000
#define F2FS_UNRM_DMD_VIDEO_FL	0x04000000

/* User visible flags */
#define F2FS_FL_USER_VISIBLE		(0x30CBDFFF | F2FS_UNRM_VIDEO_FL | \
					 F2FS_UNRM_DMD_PHOTO_FL | \
					 F2FS_UNRM_DMD_VIDEO_FL)
/* User modifiable flags */
#define F2FS_FL_USER_MODIFIABLE		(0x204BC0FF | F2FS_UNRM_VIDEO_FL | \
					 F2FS_UNRM_DMD_VIDEO_FL | \
					 F2FS_UNRM_DMD_PHOTO_FL)
void acm_hmfs_init_cache(void);
void acm_hmfs_free_cache(void);
#else
#define F2FS_FL_USER_VISIBLE		0x30CBDFFF /* User visible flags */
#define F2FS_FL_USER_MODIFIABLE		0x204BC0FF /* User modifiable flags */
#endif

/* Flags we can manipulate with through F2FS_IOC_FSSETXATTR */
#define F2FS_FL_XFLAG_VISIBLE		(F2FS_SYNC_FL | \
					 F2FS_IMMUTABLE_FL | \
					 F2FS_APPEND_FL | \
					 F2FS_NODUMP_FL | \
					 F2FS_NOATIME_FL | \
					 F2FS_PROJINHERIT_FL)

/* Flags that should be inherited by new inodes from their parent. */
#define F2FS_FL_INHERITED (F2FS_SECRM_FL | F2FS_UNRM_FL | F2FS_COMPR_FL |\
			   F2FS_SYNC_FL | F2FS_NODUMP_FL | F2FS_NOATIME_FL |\
			   F2FS_NOCOMPR_FL | F2FS_JOURNAL_DATA_FL |\
			   F2FS_NOTAIL_FL | F2FS_DIRSYNC_FL |\
			   F2FS_PROJINHERIT_FL)

/* Flags that are appropriate for regular files (all but dir-specific ones). */
#define F2FS_REG_FLMASK		(~(F2FS_DIRSYNC_FL | F2FS_TOPDIR_FL))

/* Flags that are appropriate for non-directories/regular files. */
#define F2FS_OTHER_FLMASK	(F2FS_NODUMP_FL | F2FS_NOATIME_FL | FS_UNRM_FL)

static inline __u32 f2fs_mask_flags(umode_t mode, __u32 flags)
{
	if (S_ISDIR(mode))
		return flags;
	else if (S_ISREG(mode))
		return flags & F2FS_REG_FLMASK;
	else
		return flags & F2FS_OTHER_FLMASK;
}

/* used for f2fs_inode_info->flags */
enum {
	FI_NEW_INODE,		/* indicate newly allocated inode */
	FI_DIRTY_INODE,		/* indicate inode is dirty or not */
	FI_AUTO_RECOVER,	/* indicate inode is recoverable */
	FI_DIRTY_DIR,		/* indicate directory has dirty pages */
	FI_INC_LINK,		/* need to increment i_nlink */
	FI_ACL_MODE,		/* indicate acl mode */
	FI_NO_ALLOC,		/* should not allocate any blocks */
	FI_FREE_NID,		/* free allocated nide */
	FI_NO_EXTENT,		/* not to use the extent cache */
	FI_INLINE_XATTR,	/* used for inline xattr */
	FI_INLINE_DATA,		/* used for inline data*/
	FI_INLINE_DENTRY,	/* used for inline dentry */
	FI_APPEND_WRITE,	/* inode has appended data */
	FI_UPDATE_WRITE,	/* inode has in-place-update data */
	FI_NEED_IPU,		/* used for ipu per file */
	FI_ATOMIC_FILE,		/* indicate atomic file */
	FI_ATOMIC_COMMIT,	/* indicate the state of atomical committing */
	FI_VOLATILE_FILE,	/* indicate volatile file */
	FI_FIRST_BLOCK_WRITTEN,	/* indicate #0 data block was written */
	FI_DROP_CACHE,		/* drop dirty page cache */
	FI_DATA_EXIST,		/* indicate data exists */
	FI_INLINE_DOTS,		/* indicate inline dot dentries */
	FI_DO_DEFRAG,		/* indicate defragment is running */
	FI_DIRTY_FILE,		/* indicate regular/symlink has dirty pages */
	FI_NO_PREALLOC,		/* indicate skipped preallocated blocks */
	FI_HOT_DATA,		/* indicate file is hot */
	FI_EXTRA_ATTR,		/* indicate file has extra attribute */
	FI_PROJ_INHERIT,	/* indicate file inherits projectid */
	FI_PIN_FILE,		/* indicate file should not be gced */
	FI_ATOMIC_REVOKE_REQUEST, /* request to drop atomic data */
        FI_LOG_FILE,            /* indicate file is a log */
        FI_HOT_FILE,            /* indicate file is hot */
	FI_ONLY_LARGEST_EXT_CHG,/* indicate inode changed largest extent only */
};

static inline void __mark_inode_dirty_flag(struct inode *inode,
						int flag, bool set)
{
	switch (flag) {
	case FI_INLINE_XATTR:
	case FI_INLINE_DATA:
	case FI_INLINE_DENTRY:
	case FI_NEW_INODE:
		if (set)
			return;
	case FI_DATA_EXIST:
	case FI_INLINE_DOTS:
	case FI_PIN_FILE:
		hmfs_mark_inode_dirty_sync(inode, true);
	}
}

static inline void set_inode_flag(struct inode *inode, int flag)
{
	if (!test_bit(flag, &F2FS_I(inode)->flags))
		set_bit(flag, &F2FS_I(inode)->flags);
	__mark_inode_dirty_flag(inode, flag, true);
}

static inline int is_inode_flag_set(struct inode *inode, int flag)
{
	return test_bit(flag, &F2FS_I(inode)->flags);
}

static inline void clear_inode_flag(struct inode *inode, int flag)
{
	if (test_bit(flag, &F2FS_I(inode)->flags))
		clear_bit(flag, &F2FS_I(inode)->flags);
	__mark_inode_dirty_flag(inode, flag, false);
}

#define IS_MULTI_SEGS_IN_SEC(sbi) (sbi->segs_per_sec > 1)
static inline void update_oob_wr_sec(struct f2fs_sb_info *sbi,
		int type, int add)
{
	if (!IS_MULTI_SEGS_IN_SEC(sbi))
		return;

	atomic_add(add, &sbi->oob_wr_cnt[type]);
}

static inline void inc_page_count(struct f2fs_sb_info *sbi, int count_type)
{
	atomic_inc(&sbi->nr_pages[count_type]);

	if (count_type == F2FS_DIRTY_DATA || count_type == F2FS_INMEM_PAGES ||
		count_type == F2FS_WB_CP_DATA || count_type == F2FS_WB_DATA)
		return;

	set_sbi_flag(sbi, SBI_IS_DIRTY);
}

static inline void inode_inc_dirty_pages(struct inode *inode)
{
	atomic_inc(&F2FS_I(inode)->dirty_pages);
	if (is_inode_flag_set(inode, FI_LOG_FILE))
		atomic_inc(&F2FS_I_SB(inode)->nr_pages[F2FS_LOG_FILE]);
	inc_page_count(F2FS_I_SB(inode), S_ISDIR(inode->i_mode) ?
				F2FS_DIRTY_DENTS : F2FS_DIRTY_DATA);
	if (IS_NOQUOTA(inode))
		inc_page_count(F2FS_I_SB(inode), F2FS_DIRTY_QDATA);
}

static inline void dec_page_count(struct f2fs_sb_info *sbi, int count_type)
{
	atomic_dec(&sbi->nr_pages[count_type]);
}

static inline void inode_dec_dirty_pages(struct inode *inode)
{
	if (!S_ISDIR(inode->i_mode) && !S_ISREG(inode->i_mode) &&
			!S_ISLNK(inode->i_mode))
		return;

	atomic_dec(&F2FS_I(inode)->dirty_pages);
	if (is_inode_flag_set(inode, FI_LOG_FILE))
		atomic_dec(&F2FS_I_SB(inode)->nr_pages[F2FS_LOG_FILE]);
	dec_page_count(F2FS_I_SB(inode), S_ISDIR(inode->i_mode) ?
				F2FS_DIRTY_DENTS : F2FS_DIRTY_DATA);
	if (IS_NOQUOTA(inode))
		dec_page_count(F2FS_I_SB(inode), F2FS_DIRTY_QDATA);
}

static inline s64 get_pages(struct f2fs_sb_info *sbi, int count_type)
{
	return atomic_read(&sbi->nr_pages[count_type]);
}

static inline int get_dirty_pages(struct inode *inode)
{
	return atomic_read(&F2FS_I(inode)->dirty_pages);
}

static inline int get_blocktype_secs(struct f2fs_sb_info *sbi, int block_type)
{
	unsigned int pages_per_sec = sbi->segs_per_sec * sbi->blocks_per_seg;
	unsigned int segs = (get_pages(sbi, block_type) + pages_per_sec - 1) >>
						sbi->log_blocks_per_seg;

	return segs / sbi->segs_per_sec;
}

static inline void set_acl_inode(struct inode *inode, umode_t mode)
{
	F2FS_I(inode)->i_acl_mode = mode;
	set_inode_flag(inode, FI_ACL_MODE);
	hmfs_mark_inode_dirty_sync(inode, false);
}

static inline bool is_idle(struct f2fs_sb_info *sbi, int type)
{
	if (get_pages(sbi, F2FS_RD_DATA) || get_pages(sbi, F2FS_RD_NODE) ||
		get_pages(sbi, F2FS_RD_META) || get_pages(sbi, F2FS_WB_DATA) ||
		get_pages(sbi, F2FS_WB_CP_DATA))
		return false;
	return f2fs_time_over(sbi, type);
}

static inline void f2fs_i_links_write(struct inode *inode, bool inc)
{
	if (inc)
		inc_nlink(inode);
	else
		drop_nlink(inode);
	hmfs_mark_inode_dirty_sync(inode, true);
}

static inline void f2fs_i_blocks_write(struct inode *inode,
					block_t diff, bool add, bool claim)
{
	bool clean = !is_inode_flag_set(inode, FI_DIRTY_INODE);
	bool recover = is_inode_flag_set(inode, FI_AUTO_RECOVER);

	/* add = 1, claim = 1 should be dquot_reserve_block in pair */
	if (add) {
		if (claim)
			dquot_claim_block(inode, diff);
		else
			dquot_alloc_block_nofail(inode, diff);
	} else {
		dquot_free_block(inode, diff);
	}

	hmfs_mark_inode_dirty_sync(inode, true);
	if (clean || recover)
		set_inode_flag(inode, FI_AUTO_RECOVER);
}

static inline void f2fs_i_size_write(struct inode *inode, loff_t i_size)
{
	bool clean = !is_inode_flag_set(inode, FI_DIRTY_INODE);
	bool recover = is_inode_flag_set(inode, FI_AUTO_RECOVER);

	if (i_size_read(inode) == i_size)
		return;

	i_size_write(inode, i_size);
	hmfs_mark_inode_dirty_sync(inode, true);
	if (clean || recover)
		set_inode_flag(inode, FI_AUTO_RECOVER);
}

static inline void f2fs_i_depth_write(struct inode *inode, unsigned int depth)
{
	F2FS_I(inode)->i_current_depth = depth;
	hmfs_mark_inode_dirty_sync(inode, true);
}

static inline void f2fs_i_gc_failures_write(struct inode *inode,
					unsigned int count)
{
	F2FS_I(inode)->i_gc_failures[GC_FAILURE_PIN] = count;
	hmfs_mark_inode_dirty_sync(inode, true);
}

static inline void f2fs_i_xnid_write(struct inode *inode, nid_t xnid)
{
	F2FS_I(inode)->i_xattr_nid = xnid;
	hmfs_mark_inode_dirty_sync(inode, true);
}

static inline void f2fs_i_pino_write(struct inode *inode, nid_t pino)
{
	F2FS_I(inode)->i_pino = pino;
	hmfs_mark_inode_dirty_sync(inode, true);
}

static inline void get_inline_info(struct inode *inode, struct f2fs_inode *ri)
{
	struct f2fs_inode_info *fi = F2FS_I(inode);

	if (ri->i_inline & F2FS_INLINE_XATTR)
		set_bit(FI_INLINE_XATTR, &fi->flags);
	if (ri->i_inline & F2FS_INLINE_DATA)
		set_bit(FI_INLINE_DATA, &fi->flags);
	if (ri->i_inline & F2FS_INLINE_DENTRY)
		set_bit(FI_INLINE_DENTRY, &fi->flags);
	if (ri->i_inline & F2FS_DATA_EXIST)
		set_bit(FI_DATA_EXIST, &fi->flags);
	if (ri->i_inline & F2FS_INLINE_DOTS)
		set_bit(FI_INLINE_DOTS, &fi->flags);
	if (ri->i_inline & F2FS_EXTRA_ATTR)
		set_bit(FI_EXTRA_ATTR, &fi->flags);
	if (ri->i_inline & F2FS_PIN_FILE)
		set_bit(FI_PIN_FILE, &fi->flags);
}

static inline void set_raw_inline(struct inode *inode, struct f2fs_inode *ri)
{
	ri->i_inline = 0;

	if (is_inode_flag_set(inode, FI_INLINE_XATTR))
		ri->i_inline |= F2FS_INLINE_XATTR;
	if (is_inode_flag_set(inode, FI_INLINE_DATA))
		ri->i_inline |= F2FS_INLINE_DATA;
	if (is_inode_flag_set(inode, FI_INLINE_DENTRY))
		ri->i_inline |= F2FS_INLINE_DENTRY;
	if (is_inode_flag_set(inode, FI_DATA_EXIST))
		ri->i_inline |= F2FS_DATA_EXIST;
	if (is_inode_flag_set(inode, FI_INLINE_DOTS))
		ri->i_inline |= F2FS_INLINE_DOTS;
	if (is_inode_flag_set(inode, FI_EXTRA_ATTR))
		ri->i_inline |= F2FS_EXTRA_ATTR;
	if (is_inode_flag_set(inode, FI_PIN_FILE))
		ri->i_inline |= F2FS_PIN_FILE;
}

static inline int f2fs_has_extra_attr(struct inode *inode)
{
	return is_inode_flag_set(inode, FI_EXTRA_ATTR);
}

static inline int f2fs_has_inline_xattr(struct inode *inode)
{
	return is_inode_flag_set(inode, FI_INLINE_XATTR);
}

static inline unsigned int addrs_per_inode(struct inode *inode)
{
	return CUR_ADDRS_PER_INODE(inode) - get_inline_xattr_addrs(inode);
}

static inline void *inline_xattr_addr(struct inode *inode, struct page *page)
{
	struct f2fs_inode *ri = F2FS_INODE(page);

	return (void *)&(ri->i_addr[DEF_ADDRS_PER_INODE -
					get_inline_xattr_addrs(inode)]);
}

static inline int inline_xattr_size(struct inode *inode)
{
	return get_inline_xattr_addrs(inode) * sizeof(__le32);
}

static inline int f2fs_has_inline_data(struct inode *inode)
{
	return is_inode_flag_set(inode, FI_INLINE_DATA);
}

static inline int f2fs_exist_data(struct inode *inode)
{
	return is_inode_flag_set(inode, FI_DATA_EXIST);
}

static inline int f2fs_has_inline_dots(struct inode *inode)
{
	return is_inode_flag_set(inode, FI_INLINE_DOTS);
}

static inline bool f2fs_is_pinned_file(struct inode *inode)
{
	return is_inode_flag_set(inode, FI_PIN_FILE);
}

static inline bool f2fs_is_atomic_file(struct inode *inode)
{
	return is_inode_flag_set(inode, FI_ATOMIC_FILE);
}

static inline bool f2fs_is_commit_atomic_write(struct inode *inode)
{
	return is_inode_flag_set(inode, FI_ATOMIC_COMMIT);
}

static inline bool f2fs_is_volatile_file(struct inode *inode)
{
	return is_inode_flag_set(inode, FI_VOLATILE_FILE);
}

static inline bool f2fs_is_first_block_written(struct inode *inode)
{
	return is_inode_flag_set(inode, FI_FIRST_BLOCK_WRITTEN);
}

static inline bool f2fs_is_drop_cache(struct inode *inode)
{
	return is_inode_flag_set(inode, FI_DROP_CACHE);
}

static inline void *inline_data_addr(struct inode *inode, struct page *page)
{
	struct f2fs_inode *ri = F2FS_INODE(page);
	int extra_size = get_extra_isize(inode);

	return (void *)&(ri->i_addr[extra_size + DEF_INLINE_RESERVED_SIZE]);
}

static inline int f2fs_has_inline_dentry(struct inode *inode)
{
	return is_inode_flag_set(inode, FI_INLINE_DENTRY);
}

static inline int is_file(struct inode *inode, int type)
{
	return F2FS_I(inode)->i_advise & type;
}

static inline void set_file(struct inode *inode, int type)
{
	F2FS_I(inode)->i_advise |= type;
	hmfs_mark_inode_dirty_sync(inode, true);
}

static inline void clear_file(struct inode *inode, int type)
{
	F2FS_I(inode)->i_advise &= ~type;
	hmfs_mark_inode_dirty_sync(inode, true);
}

static inline bool f2fs_skip_inode_update(struct inode *inode, int dsync)
{
	bool ret;

	if (dsync) {
		struct f2fs_sb_info *sbi = F2FS_I_SB(inode);

		spin_lock(&sbi->inode_lock[DIRTY_META]);
		ret = list_empty(&F2FS_I(inode)->gdirty_list);
		spin_unlock(&sbi->inode_lock[DIRTY_META]);
		return ret;
	}
	if (!is_inode_flag_set(inode, FI_AUTO_RECOVER) ||
			file_keep_isize(inode) ||
			i_size_read(inode) & ~PAGE_MASK)
		return false;

	if (!timespec_equal(F2FS_I(inode)->i_disk_time, &inode->i_atime))
		return false;
	if (!timespec_equal(F2FS_I(inode)->i_disk_time + 1, &inode->i_ctime))
		return false;
	if (!timespec_equal(F2FS_I(inode)->i_disk_time + 2, &inode->i_mtime))
		return false;
	if (!timespec_equal(F2FS_I(inode)->i_disk_time + 3,
						&F2FS_I(inode)->i_crtime))
		return false;

	down_read(&F2FS_I(inode)->i_sem);
	ret = F2FS_I(inode)->last_disk_size == i_size_read(inode);
	up_read(&F2FS_I(inode)->i_sem);

	return ret;
}

static inline bool f2fs_readonly(struct super_block *sb)
{
	return sb_rdonly(sb);
}

static inline bool f2fs_cp_error(struct f2fs_sb_info *sbi)
{
	return is_set_ckpt_flags(sbi, CP_ERROR_FLAG);
}

static inline bool is_dot_dotdot(const struct qstr *str)
{
	if (str->len == 1 && str->name[0] == '.')
		return true;

	if (str->len == 2 && str->name[0] == '.' && str->name[1] == '.')
		return true;

	return false;
}

static inline bool f2fs_may_extent_tree(struct inode *inode)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);

	if (!test_opt(sbi, EXTENT_CACHE) ||
			is_inode_flag_set(inode, FI_NO_EXTENT))
		return false;

	/*
	 * for recovered files during mount do not create extents
	 * if shrinker is not registered.
	 */
	if (list_empty(&sbi->s_list))
		return false;

	return S_ISREG(inode->i_mode);
}

static inline void *f2fs_kmalloc(struct f2fs_sb_info *sbi,
					size_t size, gfp_t flags)
{
	if (time_to_inject(sbi, FAULT_KMALLOC)) {
		f2fs_show_injection_info(FAULT_KMALLOC);
		return NULL;
	}

	return kmalloc(size, flags);
}

static inline void *f2fs_kzalloc(struct f2fs_sb_info *sbi,
					size_t size, gfp_t flags)
{
	return f2fs_kmalloc(sbi, size, flags | __GFP_ZERO);
}

static inline void *f2fs_kvmalloc(struct f2fs_sb_info *sbi,
					size_t size, gfp_t flags)
{
	if (time_to_inject(sbi, FAULT_KVMALLOC)) {
		f2fs_show_injection_info(FAULT_KVMALLOC);
		return NULL;
	}

	return kvmalloc(size, flags);
}

static inline void *f2fs_kvzalloc(struct f2fs_sb_info *sbi,
					size_t size, gfp_t flags)
{
	return f2fs_kvmalloc(sbi, size, flags | __GFP_ZERO);
}

/* wrapper for __vmalloc */
static inline void *f2fs_vzalloc(size_t size, gfp_t flags)
{
       return __vmalloc(size, flags | __GFP_ZERO, PAGE_KERNEL);
}


static inline int get_extra_isize(struct inode *inode)
{
	return F2FS_I(inode)->i_extra_isize / sizeof(__le32);
}

static inline int get_inline_xattr_addrs(struct inode *inode)
{
	return F2FS_I(inode)->i_inline_xattr_size;
}

#define f2fs_get_inode_mode(i) \
	((is_inode_flag_set(i, FI_ACL_MODE)) ? \
	 (F2FS_I(i)->i_acl_mode) : ((i)->i_mode))

#define F2FS_TOTAL_EXTRA_ATTR_SIZE			\
	(offsetof(struct f2fs_inode, i_extra_end) -	\
	offsetof(struct f2fs_inode, i_extra_isize))	\

#define F2FS_OLD_ATTRIBUTE_SIZE	(offsetof(struct f2fs_inode, i_addr))
#define F2FS_FITS_IN_INODE(f2fs_inode, extra_isize, field)		\
		((offsetof(typeof(*f2fs_inode), field) +	\
		sizeof((f2fs_inode)->field))			\
		<= (F2FS_OLD_ATTRIBUTE_SIZE + extra_isize))	\

static inline void f2fs_reset_iostat(struct f2fs_sb_info *sbi)
{
	int i;

	spin_lock(&sbi->iostat_lock);
	for (i = 0; i < NR_IO_TYPE; i++)
		sbi->write_iostat[i] = 0;
	spin_unlock(&sbi->iostat_lock);
}

static inline void f2fs_update_iostat(struct f2fs_sb_info *sbi,
			enum iostat_type type, unsigned long long io_bytes)
{
	if (!sbi->iostat_enable)
		return;
	spin_lock(&sbi->iostat_lock);
	sbi->write_iostat[type] += io_bytes;

	if (type == APP_WRITE_IO || type == APP_DIRECT_IO)
		sbi->write_iostat[APP_BUFFERED_IO] =
			sbi->write_iostat[APP_WRITE_IO] -
			sbi->write_iostat[APP_DIRECT_IO];
	spin_unlock(&sbi->iostat_lock);
}

static inline block_t fs_free_space_threshold(struct f2fs_sb_info *sbi)
{
        return (block_t)(SM_I(sbi)->main_segments * sbi->blocks_per_seg *
                                        FS_FREE_SPACE_PERCENT) / 100;
}

static inline block_t device_free_space_threshold(struct f2fs_sb_info *sbi)
{
        return (block_t)(SM_I(sbi)->main_segments * sbi->blocks_per_seg *
                                        DEVICE_FREE_SPACE_PERCENT) / 100;
}


#define __is_meta_io(fio) (PAGE_TYPE_OF_BIO(fio->type) == META &&	\
				(!is_read_io(fio->op) || fio->is_meta))

bool hmfs_is_valid_blkaddr(struct f2fs_sb_info *sbi,
					block_t blkaddr, int type);
void hmfs_msg(struct super_block *sb, const char *level, const char *fmt, ...);
static inline void verify_blkaddr(struct f2fs_sb_info *sbi,
					block_t blkaddr, int type)
{
	if (!hmfs_is_valid_blkaddr(sbi, blkaddr, type)) {
		hmfs_msg(sbi->sb, KERN_ERR,
			"invalid blkaddr: %u, type: %d, run fsck to fix.",
			blkaddr, type);
		f2fs_bug_on(sbi, 1);
	}
}

static inline bool __is_valid_data_blkaddr(block_t blkaddr)
{
	if (blkaddr == NEW_ADDR || blkaddr == NULL_ADDR)
		return false;
	return true;
}

static inline bool is_valid_data_blkaddr(struct f2fs_sb_info *sbi,
						block_t blkaddr)
{
	if (!__is_valid_data_blkaddr(blkaddr))
		return false;
	verify_blkaddr(sbi, blkaddr, DATA_GENERIC);
	return true;
}

static inline void f2fs_set_page_private(struct page *page,
						unsigned long data)
{
	if (PagePrivate(page))
		return;

	get_page(page);
	SetPagePrivate(page);
	set_page_private(page, data);
}

static inline void f2fs_clear_page_private(struct page *page)
{
	if (!PagePrivate(page))
		return;

	set_page_private(page, 0);
	ClearPagePrivate(page);
	f2fs_put_page(page, 0);
}

/*
 * file.c
 */
int hmfs_sync_file(struct file *file, loff_t start, loff_t end, int datasync);
void hmfs_truncate_data_blocks(struct dnode_of_data *dn);
int hmfs_truncate_blocks(struct inode *inode, u64 from, bool lock,
							bool buf_write);
int hmfs_truncate(struct inode *inode);
int hmfs_getattr(const struct path *path, struct kstat *stat,
			u32 request_mask, unsigned int flags);
int hmfs_setattr(struct dentry *dentry, struct iattr *attr);
int hmfs_truncate_hole(struct inode *inode, pgoff_t pg_start, pgoff_t pg_end);
void hmfs_truncate_data_blocks_range(struct dnode_of_data *dn, int count);
int hmfs_precache_extents(struct inode *inode);
long hmfs_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
long hmfs_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
int hmfs_transfer_project_quota(struct inode *inode, kprojid_t kprojid);
int hmfs_pin_file_control(struct inode *inode, bool inc);
void hmfs_flush_wait_fsync(struct file *file);
void hmfs_wait_writeback_work_fn(struct work_struct *work);

/*
 * inode.c
 */
void hmfs_set_inode_flags(struct inode *inode);
bool hmfs_inode_chksum_verify(struct f2fs_sb_info *sbi, struct page *page);
void hmfs_inode_chksum_set(struct f2fs_sb_info *sbi, struct page *page);
struct inode *hmfs_iget(struct super_block *sb, unsigned long ino);
struct inode *hmfs_iget_retry(struct super_block *sb, unsigned long ino);
int hmfs_try_to_free_nats(struct f2fs_sb_info *sbi, int nr_shrink);
void hmfs_update_inode(struct inode *inode, struct page *node_page);
void hmfs_update_inode_page(struct inode *inode);
int hmfs_write_inode(struct inode *inode, struct writeback_control *wbc);
void hmfs_evict_inode(struct inode *inode);
void hmfs_handle_failed_inode(struct inode *inode);
bool hmfs_is_node_page_dirty(struct f2fs_sb_info *sbi, nid_t nid);

#define IS_FI_ONLY_CHG_EXT(inode) \
	is_inode_flag_set(inode, FI_ONLY_LARGEST_EXT_CHG)

/*
 * namei.c
 */
int hmfs_update_extension_list(struct f2fs_sb_info *sbi, const char *name,
							bool hot, bool set);
struct dentry *hmfs_get_parent(struct dentry *child);

/*
 * dir.c
 */
unsigned char hmfs_get_de_type(struct f2fs_dir_entry *de);
struct f2fs_dir_entry *hmfs_find_target_dentry(struct fscrypt_name *fname,
			f2fs_hash_t namehash, int *max_slots,
			struct f2fs_dentry_ptr *d);
int hmfs_fill_dentries(struct dir_context *ctx, struct f2fs_dentry_ptr *d,
			unsigned int start_pos, struct fscrypt_str *fstr);
void hmfs_do_make_empty_dir(struct inode *inode, struct inode *parent,
			struct f2fs_dentry_ptr *d);
struct page *hmfs_init_inode_metadata(struct inode *inode,
			struct dentry *dentry, struct inode *dir,
			const struct qstr *new_name,
			const struct qstr *orig_name, struct page *dpage);
void hmfs_update_parent_metadata(struct inode *dir, struct inode *inode,
			unsigned int current_depth);
int hmfs_room_for_filename(const void *bitmap, int slots, int max_slots);
void hmfs_drop_nlink(struct inode *dir, struct inode *inode);
struct f2fs_dir_entry *__hmfs_find_entry(struct inode *dir,
			struct fscrypt_name *fname, struct page **res_page);
struct f2fs_dir_entry *hmfs_find_entry(struct inode *dir,
			const struct qstr *child, struct page **res_page);
struct f2fs_dir_entry *hmfs_parent_dir(struct inode *dir, struct page **p);
ino_t hmfs_inode_by_name(struct inode *dir, const struct qstr *qstr,
			struct page **page);
void hmfs_set_link(struct inode *dir, struct f2fs_dir_entry *de,
			struct page *page, struct inode *inode);
void hmfs_update_dentry(nid_t ino, umode_t mode, struct f2fs_dentry_ptr *d,
			const struct qstr *name, f2fs_hash_t name_hash,
			unsigned int bit_pos);
int hmfs_add_regular_entry(struct inode *dir, const struct qstr *new_name,
			const struct qstr *orig_name, struct dentry *dentry,
			struct inode *inode, nid_t ino, umode_t mode);
int hmfs_add_dentry(struct inode *dir, struct fscrypt_name *fname,
			struct dentry *dentry, struct inode *inode,
			nid_t ino, umode_t mode);
int hmfs_do_add_link(struct inode *dir, struct dentry *dentry,
			const struct qstr *name, struct inode *inode,
			nid_t ino, umode_t mode);
void hmfs_delete_entry(struct f2fs_dir_entry *dentry, struct page *page,
			struct inode *dir, struct inode *inode);
int hmfs_do_tmpfile(struct inode *inode, struct dentry *dentry,
			struct inode *dir);
bool hmfs_empty_dir(struct inode *dir);

static inline int f2fs_add_link(struct dentry *dentry, struct inode *inode)
{
	return hmfs_do_add_link(d_inode(dentry->d_parent), dentry,
				&dentry->d_name, inode,
				inode->i_ino, inode->i_mode);
}

/*
 * super.c
 */
int hmfs_inode_dirtied(struct inode *inode, bool sync);
void hmfs_inode_synced(struct inode *inode);
int hmfs_enable_quota_files(struct f2fs_sb_info *sbi, bool rdonly);
int hmfs_quota_sync(struct super_block *sb, int type);
void hmfs_quota_off_umount(struct super_block *sb);
int hmfs_commit_super(struct f2fs_sb_info *sbi, bool recover);
int hmfs_sync_fs(struct super_block *sb, int sync);
extern __printf(3, 4)
void hmfs_msg(struct super_block *sb, const char *level, const char *fmt, ...);
int hmfs_sanity_check_ckpt(struct f2fs_sb_info *sbi);

/*
 * hash.c
 */
f2fs_hash_t hmfs_dentry_hash(const struct qstr *name_info,
				struct fscrypt_name *fname);

/*
 * node.c
 */
struct dnode_of_data;
struct node_info;

int hmfs_check_nid_range(struct f2fs_sb_info *sbi, nid_t nid);
bool hmfs_available_free_memory(struct f2fs_sb_info *sbi, int type);
bool hmfs_in_warm_node_list(struct f2fs_sb_info *sbi, struct page *page);
void hmfs_init_fsync_node_info(struct f2fs_sb_info *sbi);
void hmfs_del_fsync_node_entry(struct f2fs_sb_info *sbi, struct page *page);
void hmfs_reset_fsync_node_info(struct f2fs_sb_info *sbi);
int hmfs_need_dentry_mark(struct f2fs_sb_info *sbi, nid_t nid);
bool hmfs_is_checkpointed_node(struct f2fs_sb_info *sbi, nid_t nid);
bool hmfs_need_inode_block_update(struct f2fs_sb_info *sbi, nid_t ino);
int hmfs_get_node_info_ex(struct f2fs_sb_info *sbi, nid_t nid,
                       struct node_info *ni, bool cache_only);
static inline int  f2fs_get_node_info(struct f2fs_sb_info *sbi,
                       nid_t nid, struct node_info *ni)
{
       return hmfs_get_node_info_ex(sbi, nid, ni, false);
}
pgoff_t hmfs_get_next_page_offset(struct dnode_of_data *dn, pgoff_t pgofs);
int hmfs_get_dnode_of_data(struct dnode_of_data *dn, pgoff_t index, int mode);
int hmfs_truncate_inode_blocks(struct inode *inode, pgoff_t from);
int hmfs_truncate_xattr_node(struct inode *inode);
int hmfs_wait_on_node_pages_writeback(struct f2fs_sb_info *sbi,
					unsigned int seq_id);
int hmfs_remove_inode_page(struct inode *inode);
struct page *hmfs_new_inode_page(struct inode *inode);
struct page *hmfs_new_node_page(struct dnode_of_data *dn, unsigned int ofs);
void hmfs_ra_node_page(struct f2fs_sb_info *sbi, nid_t nid);
struct page *hmfs_get_node_page_ex(struct f2fs_sb_info *sbi, pgoff_t nid,
                        struct page *parent, int start, bool cache_only);
struct page *hmfs_get_node_page(struct f2fs_sb_info *sbi, pgoff_t nid);
struct page *hmfs_get_node_page_ra(struct page *parent, int start);
int hmfs_move_node_page(struct page *node_page, int gc_type);
int hmfs_fsync_node_pages(struct f2fs_sb_info *sbi, struct inode *inode,
			struct writeback_control *wbc, bool atomic, bool ff_enable);
int hmfs_sync_node_pages(struct f2fs_sb_info *sbi,
			struct writeback_control *wbc,
			bool do_balance, enum iostat_type io_type);
int hmfs_build_free_nids(struct f2fs_sb_info *sbi, bool sync, bool mount);
bool hmfs_alloc_nid(struct f2fs_sb_info *sbi, nid_t *nid);
void hmfs_alloc_nid_done(struct f2fs_sb_info *sbi, nid_t nid);
void hmfs_alloc_nid_failed(struct f2fs_sb_info *sbi, nid_t nid);
int hmfs_try_to_free_nids(struct f2fs_sb_info *sbi, int nr_shrink);
void hmfs_recover_inline_xattr(struct inode *inode, struct page *page);
int hmfs_recover_xattr_data(struct inode *inode, struct page *page);
int hmfs_recover_inode_page(struct f2fs_sb_info *sbi, struct page *page);
int hmfs_restore_node_summary(struct f2fs_sb_info *sbi,
			unsigned int segno, struct f2fs_summary_block *sum);
int hmfs_flush_nat_entries(struct f2fs_sb_info *sbi, struct cp_control *cpc);
int hmfs_build_node_manager(struct f2fs_sb_info *sbi);
void hmfs_destroy_node_manager(struct f2fs_sb_info *sbi);
int __init hmfs_create_node_manager_caches(void);
void hmfs_destroy_node_manager_caches(void);

/*
 * segment.c
 */
unsigned long __hmfs_find_rev_next_bit(const unsigned long *,
                        unsigned long , unsigned long);
unsigned long __hmfs_find_rev_next_zero_bit(const unsigned long *,
                        unsigned long , unsigned long);
bool hmfs_need_SSR(struct f2fs_sb_info *sbi);
void hmfs_register_inmem_page(struct inode *inode, struct page *page);
void hmfs_drop_inmem_pages_all(struct f2fs_sb_info *sbi, bool gc_failure);
void hmfs_drop_inmem_pages(struct inode *inode);
void hmfs_drop_inmem_page(struct inode *inode, struct page *page);
int hmfs_commit_inmem_pages(struct inode *inode);
void hmfs_balance_fs(struct f2fs_sb_info *sbi, bool need);
void hmfs_balance_fs_bg(struct f2fs_sb_info *sbi);
int hmfs_issue_flush(struct f2fs_sb_info *sbi, nid_t ino);
int hmfs_create_flush_cmd_control(struct f2fs_sb_info *sbi);
int hmfs_flush_device_cache(struct f2fs_sb_info *sbi);
void hmfs_destroy_flush_cmd_control(struct f2fs_sb_info *sbi, bool free);
void hmfs_invalidate_blocks(struct f2fs_sb_info *sbi, block_t addr);
bool hmfs_is_checkpointed_data(struct f2fs_sb_info *sbi, block_t blkaddr);
void hmfs_drop_discard_cmd(struct f2fs_sb_info *sbi);
void hmfs_stop_discard_thread(struct f2fs_sb_info *sbi);
bool hmfs_wait_discard_bios(struct f2fs_sb_info *sbi);

void hmfs_refresh_sit_entry(struct f2fs_sb_info *sbi, block_t old, block_t new,
                                                                bool from_gc);
void hmfs_clear_prefree_segments(struct f2fs_sb_info *sbi,
					struct cp_control *cpc);
void hmfs_dirty_to_prefree(struct f2fs_sb_info *sbi);
int hmfs_disable_cp_again(struct f2fs_sb_info *sbi);
void hmfs_release_discard_addrs(struct f2fs_sb_info *sbi);
int hmfs_npages_for_summary_flush(struct f2fs_sb_info *sbi, bool for_ra);
void hmfs_get_new_segment(struct f2fs_sb_info *sbi,
                       unsigned int *newseg, bool new_sec, int dir);
void hmfs_store_virtual_curseg_summary(struct f2fs_sb_info *sbi);
void hmfs_restore_virtual_curseg_status(struct f2fs_sb_info * sbi, bool recover);
void hmfs_datamove_change_curseg(struct f2fs_sb_info *sbi,
		struct curseg_info *curseg, int type, block_t next_blk_addr);
int hmfs_allocate_segment_for_resize(struct f2fs_sb_info *sbi, int type,
					unsigned int start, unsigned int end);
void hmfs_allocate_new_segments(struct f2fs_sb_info *sbi);
int hmfs_trim_fs(struct f2fs_sb_info *sbi, struct fstrim_range *range);
bool hmfs_exist_trim_candidates(struct f2fs_sb_info *sbi,
					struct cp_control *cpc);
struct page *hmfs_get_sum_page(struct f2fs_sb_info *sbi, unsigned int segno);
void hmfs_update_meta_page(struct f2fs_sb_info *sbi, void *src,
					block_t blk_addr);
void hmfs_do_write_meta_page(struct f2fs_sb_info *sbi, struct page *page,
						enum iostat_type io_type);
void hmfs_do_write_node_page(unsigned int nid, struct f2fs_io_info *fio);
int hmfs_outplace_write_data(struct dnode_of_data *dn,
			struct f2fs_io_info *fio);
int hmfs_inplace_write_data(struct f2fs_io_info *fio);
void hmfs_do_replace_block(struct f2fs_sb_info *sbi, struct f2fs_summary *sum,
			block_t old_blkaddr, block_t new_blkaddr,
			bool recover_curseg, bool recover_newaddr,
			bool from_recover, bool from_gc);
void hmfs_replace_block(struct f2fs_sb_info *sbi, struct dnode_of_data *dn,
			block_t old_addr, block_t new_addr,
			unsigned char version, bool recover_curseg,
			bool recover_newaddr, bool from_recover);
int hmfs_allocate_data_block(struct f2fs_sb_info *sbi, struct page *page,
			block_t old_blkaddr, block_t *new_blkaddr,
			struct f2fs_summary *sum, int type,
			struct f2fs_io_info *fio, bool add_list,
			int contig_level);
void hmfs_wait_on_page_writeback(struct page *page,
			enum page_type type, bool ordered);
void hmfs_wait_on_block_writeback(struct inode *inode, block_t blkaddr);
void hmfs_wait_on_block_writeback_range(struct inode *inode, block_t blkaddr,
								block_t len);
void hmfs_init_virtual_curseg(struct f2fs_sb_info *sbi);
void hmfs_write_data_summaries(struct f2fs_sb_info *sbi, block_t start_blk);
void hmfs_write_node_summaries(struct f2fs_sb_info *sbi, block_t start_blk);
void hmfs_write_datamove_summaries(struct f2fs_sb_info *sbi,
			block_t start_blk);
#ifdef CONFIG_HMFS_JOURNAL_APPEND
void write_append_journal(struct f2fs_sb_info *sbi, block_t start_blk);
#endif
int hmfs_lookup_journal_in_cursum(struct f2fs_journal *journal, int type,
			unsigned int val, int alloc);
void hmfs_flush_sit_entries(struct f2fs_sb_info *sbi, struct cp_control *cpc);
int hmfs_build_segment_manager(struct f2fs_sb_info *sbi);
void hmfs_destroy_segment_manager(struct f2fs_sb_info *sbi);
int __init hmfs_create_segment_manager_caches(void);
void hmfs_destroy_segment_manager_caches(void);
int hmfs_rw_hint_to_seg_type(struct f2fs_sb_info *sbi, enum rw_hint hint);
enum rw_hint hmfs_io_type_to_rw_hint(struct f2fs_sb_info *sbi,
			enum page_type type, enum temp_type temp);
int hmfs_sync_device_info(struct f2fs_sb_info *sbi, bool *need_cp);
void hmfs_bio_set_flash_mode(struct f2fs_sb_info *sbi, struct bio *bio,
			block_t blkaddr, int stream_id);
void hmfs_file_check_switch_stream(struct inode *inode, int stream_id);
void hmfs_insert_section_order_list(struct f2fs_sb_info *sbi,
				unsigned int segno, int curseg_type);
unsigned int hmfs_section_order_list_peek(struct f2fs_sb_info *sbi, int stream_id);
void hmfs_section_order_list_pop(struct f2fs_sb_info *sbi, block_t blkaddr, int stream_id);

/*
 * checkpoint.c
 */
void hmfs_stop_checkpoint(struct f2fs_sb_info *sbi, bool end_io);
struct page *hmfs_grab_meta_page(struct f2fs_sb_info *sbi, pgoff_t index);
struct page *hmfs_get_meta_page_ex(struct f2fs_sb_info *sbi, pgoff_t index,
                       bool is_meta, bool cache_only);
struct page *hmfs_get_meta_page(struct f2fs_sb_info *sbi, pgoff_t index);
struct page *hmfs_get_meta_page_nofail(struct f2fs_sb_info *sbi, pgoff_t index);
struct page *hmfs_get_tmp_page(struct f2fs_sb_info *sbi, pgoff_t index);
bool hmfs_is_valid_blkaddr(struct f2fs_sb_info *sbi,
					block_t blkaddr, int type);
int hmfs_ra_meta_pages(struct f2fs_sb_info *sbi, block_t start, int nrpages,
			int type, bool sync);
void hmfs_ra_meta_pages_cond(struct f2fs_sb_info *sbi, pgoff_t index);
long hmfs_sync_meta_pages(struct f2fs_sb_info *sbi, enum page_type type,
			long nr_to_write, enum iostat_type io_type);
void hmfs_add_ino_entry(struct f2fs_sb_info *sbi, nid_t ino, int type);
void hmfs_remove_ino_entry(struct f2fs_sb_info *sbi, nid_t ino, int type);
void hmfs_release_ino_entry(struct f2fs_sb_info *sbi, bool all);
bool hmfs_exist_written_data(struct f2fs_sb_info *sbi, nid_t ino, int mode);
void hmfs_set_dirty_device(struct f2fs_sb_info *sbi, nid_t ino,
					unsigned int devidx, int type);
bool hmfs_is_dirty_device(struct f2fs_sb_info *sbi, nid_t ino,
					unsigned int devidx, int type);
int hmfs_sync_inode_meta(struct f2fs_sb_info *sbi);
int hmfs_acquire_orphan_inode(struct f2fs_sb_info *sbi);
void hmfs_release_orphan_inode(struct f2fs_sb_info *sbi);
void hmfs_add_orphan_inode(struct inode *inode);
void hmfs_remove_orphan_inode(struct f2fs_sb_info *sbi, nid_t ino);
int hmfs_recover_orphan_inodes(struct f2fs_sb_info *sbi);
int hmfs_get_valid_checkpoint(struct f2fs_sb_info *sbi);
void hmfs_update_dirty_page(struct inode *inode, struct page *page);
void hmfs_remove_dirty_inode(struct inode *inode);
int hmfs_sync_dirty_inodes(struct f2fs_sb_info *sbi, enum inode_type type);
void hmfs_wait_on_all_pages_writeback(struct f2fs_sb_info *sbi);
int hmfs_write_checkpoint(struct f2fs_sb_info *sbi, struct cp_control *cpc);
void hmfs_init_ino_entry_info(struct f2fs_sb_info *sbi);
int __init hmfs_create_checkpoint_caches(void);
void hmfs_destroy_checkpoint_caches(void);

/*
 * data.c
 */
int hmfs_init_post_read_processing(void);
void hmfs_destroy_post_read_processing(void);
void hmfs_submit_merged_write(struct f2fs_sb_info *sbi, enum page_type type);
void hmfs_submit_merged_write_cond(struct f2fs_sb_info *sbi,
				struct inode *inode, struct page *page,
				nid_t ino, enum page_type type);
void hmfs_flush_merged_writes(struct f2fs_sb_info *sbi);
int hmfs_submit_page_bio(struct f2fs_io_info *fio);
void hmfs_submit_page_write(struct f2fs_io_info *fio);
struct block_device *hmfs_target_device(struct f2fs_sb_info *sbi,
			block_t blk_addr, struct bio *bio);
int hmfs_target_device_index(struct f2fs_sb_info *sbi, block_t blkaddr);
void hmfs_set_data_blkaddr(struct dnode_of_data *dn);
void hmfs_update_data_blkaddr(struct dnode_of_data *dn, block_t blkaddr);
int hmfs_reserve_new_blocks(struct dnode_of_data *dn, blkcnt_t count);
int hmfs_reserve_new_block(struct dnode_of_data *dn);
int hmfs_get_block(struct dnode_of_data *dn, pgoff_t index);
int hmfs_preallocate_blocks(struct kiocb *iocb, struct iov_iter *from);
int hmfs_reserve_block(struct dnode_of_data *dn, pgoff_t index);
struct page *hmfs_get_read_data_page(struct inode *inode, pgoff_t index,
			int op_flags, bool for_write);
struct page *hmfs_find_data_page(struct inode *inode, pgoff_t index);
struct page *hmfs_get_lock_data_page(struct inode *inode, pgoff_t index,
			bool for_write);
struct page *hmfs_get_new_data_page(struct inode *inode,
			struct page *ipage, pgoff_t index, bool new_i_size);
int hmfs_do_write_data_page(struct f2fs_io_info *fio);
bool __hmfs_do_map_lock(struct f2fs_sb_info *sbi, int flag, bool lock, bool lock2);
int hmfs_map_blocks(struct inode *inode, struct f2fs_map_blocks *map,
			int create, int flag);
int hmfs_fiemap(struct inode *inode, struct fiemap_extent_info *fieinfo,
			u64 start, u64 len);
bool hmfs_should_update_inplace(struct inode *inode, struct f2fs_io_info *fio);
bool hmfs_should_update_outplace(struct inode *inode, struct f2fs_io_info *fio);
void hmfs_invalidate_page(struct page *page, unsigned int offset,
			unsigned int length);
int hmfs_release_page(struct page *page, gfp_t wait);
#ifdef CONFIG_MIGRATION
int hmfs_migrate_page(struct address_space *mapping, struct page *newpage,
			struct page *page, enum migrate_mode mode);
#endif
bool hmfs_overwrite_io(struct inode *inode, loff_t pos, size_t len);
void hmfs_clear_radix_tree_dirty_tag(struct page *page);
void hmfs_io_throttle(struct f2fs_sb_info *sbi, unsigned int len);

/*
 * gc.c
 */
int hmfs_start_gc_thread(struct f2fs_sb_info *sbi);
void hmfs_stop_gc_thread(struct f2fs_sb_info *sbi);
block_t hmfs_start_bidx_of_node(unsigned int node_ofs, struct inode *inode);
int hmfs_gc(struct f2fs_sb_info *sbi, bool sync, bool background,
			unsigned int segno);
int __init hmfs_create_garbage_collection_cache(void);
void hmfs_destroy_garbage_collection_cache(void);
void hmfs_build_gc_manager(struct f2fs_sb_info *sbi);
int hmfs_resize_fs(struct f2fs_sb_info *sbi, size_t resize_len);
int hmfs_build_gc_control_info(struct f2fs_sb_info *sbi);
void hmfs_destroy_gc_control_info(struct f2fs_sb_info *sbi);
block_t hmfs_get_free_block_count(struct f2fs_sb_info *sbi);

/* datamove */
int hmfs_datamove_init(struct f2fs_sb_info *sbi);
void hmfs_datamove_destroy(struct f2fs_sb_info *sbi);
bool hmfs_datamove_get_mapped_blk(struct f2fs_sb_info *sbi,
				block_t dst_blk, block_t *src_blk);
void hmfs_datamove_put_mapped_blk(struct f2fs_sb_info *sbi);
void hmfs_datamove_add_to_bio(struct f2fs_sb_info *sbi, struct bio *bio);
void hmfs_datamove_check_mapped_blk(struct f2fs_sb_info *sbi,
				struct bio *bio, block_t dst_blk);
bool hmfs_datamove_check_discard(struct f2fs_sb_info *sbi,
				unsigned int start_seg, unsigned int end_seg,
				unsigned long *bitmap);
void hmfs_datamove_add_tree_entry(struct f2fs_sb_info *sbi,
				block_t src, block_t dst,  unsigned int i_ino,
				unsigned int offset);
int hmfs_datamove_write_entry(struct f2fs_sb_info *sbi,
				block_t blkaddr);
void hmfs_datamove_force_flush(struct f2fs_sb_info *sbi,
				struct cp_control *cpc);
void hmfs_datamove(struct f2fs_sb_info *sbi, int dm_stream_id,
				bool force_flush);
void hmfs_datamove_tree_print_info(struct f2fs_sb_info *sbi,
				unsigned int *unverify_sec,
				unsigned int *unverify_free_sec);
void hmfs_datamove_fill_array(struct f2fs_sb_info *sbi,
				block_t next_submit_addr, block_t cur_max_addr,
				int dm_stream_id);
void hmfs_datamove_rescue(struct f2fs_sb_info *sbi, int dm_stream_id,
				block_t last_repeat_addr);
void hmfs_datamove_drop_verified_entries(struct f2fs_sb_info *sbi,
				block_t verified_blkaddr, bool force);
void __hmfs_datamove_add_entry(struct f2fs_sb_info *sbi, block_t src,
			    block_t dst, enum page_type type, int dm_stream_id);
void __hmfs_datamove_remove_entry(struct f2fs_sb_info *sbi,
				block_t blkaddr);

static inline bool hmfs_datamove_on(struct f2fs_sb_info *sbi)
{
	return is_sbi_flag_set(sbi, SBI_DATAMOVE_GC);
}

static inline struct hmfs_dm_manager *HMFS_DM(struct f2fs_sb_info *sbi)
{
	return (struct hmfs_dm_manager *)&sbi->dm_mng;
}

static inline void hmfs_datamove_add_entry(struct f2fs_sb_info *sbi,
		block_t src, block_t dst, enum page_type type, int dm_stream_id)
{
	if (!hmfs_datamove_on(sbi))
		return;

	__hmfs_datamove_add_entry(sbi, src, dst, type, dm_stream_id);
}

static inline void hmfs_datamove_remove_entry(struct f2fs_sb_info *sbi,
					      block_t blkaddr)
{
	if (!hmfs_datamove_on(sbi))
		return;

	__hmfs_datamove_remove_entry(sbi, blkaddr);
}

/*
 * recovery.c
 */
int hmfs_recover_fsync_data(struct f2fs_sb_info *sbi, bool check_only,
		bool *need_cp);
bool hmfs_space_for_roll_forward(struct f2fs_sb_info *sbi);
void hmfs_set_oob_switch(struct f2fs_sb_info *sbi, bool is_on);
bool hmfs_is_oob_enable(struct f2fs_sb_info *sbi);
void hmfs_log_oob_extent(struct f2fs_sb_info *sbi,
		struct stor_dev_pwron_info *stor_info);
void hmfs_free_oob_rsvd_space(struct f2fs_sb_info *sbi);

/*
 * debug.c
 */
#ifdef CONFIG_HMFS_STAT_FS
struct f2fs_stat_info {
	struct list_head stat_list;
	struct f2fs_sb_info *sbi;
	int all_area_segs, sit_area_segs, nat_area_segs, ssa_area_segs;
	int main_area_segs, main_area_sections, main_area_zones;
	unsigned long long hit_largest, hit_cached, hit_rbtree;
	unsigned long long hit_total, total_ext;
	int ext_tree, zombie_tree, ext_node;
	int ndirty_node, ndirty_dent, ndirty_meta, ndirty_imeta;
	int ndirty_data, ndirty_qdata;
	int inmem_pages;
	unsigned int ndirty_dirs, ndirty_files, nquota_files, ndirty_all;
	int nats, dirty_nats, sits, dirty_sits;
	int free_nids, avail_nids, alloc_nids;
	int total_count, utilization;
	int bg_gc, nr_wb_cp_data, nr_wb_data;
	int nr_rd_data, nr_rd_node, nr_rd_meta;
	unsigned int io_skip_bggc, other_skip_bggc;
	int nr_flushing, nr_flushed, flush_list_empty;
	int nr_discarding, nr_discarded;
	int nr_discard_cmd;
	unsigned int undiscard_blks;
	int inline_xattr, inline_inode, inline_dir, append, update, orphans;
	int aw_cnt, max_aw_cnt, vw_cnt, max_vw_cnt;
	unsigned int valid_count, valid_node_count, valid_inode_count, discard_blks;
	unsigned int bimodal, avg_vblocks;
	int util_free, util_valid, util_invalid;
	int rsvd_segs, overp_segs, extra_op_segs;
	int dirty_count, dirty_node_count, dirty_data_count, node_pages, meta_pages;
	int prefree_count, prefree_sec_count, call_count, cp_count, bg_cp_count;
	int tot_segs, node_segs, data_segs, free_segs, free_secs;
	int bg_node_segs, bg_data_segs;
	int tot_blks, data_blks, node_blks;
	int bg_data_blks, bg_node_blks;
	unsigned long long skipped_atomic_files[2];
	int curseg[NR_INMEM_CURSEG_TYPE];
	int cursec[NR_INMEM_CURSEG_TYPE];
	int curzone[NR_INMEM_CURSEG_TYPE];
	unsigned int dirty_seg[NR_INMEM_CURSEG_TYPE];
	unsigned int full_seg[NR_INMEM_CURSEG_TYPE];
	unsigned int valid_blks[NR_INMEM_CURSEG_TYPE];

	unsigned int meta_count[META_MAX];
	int assr_lfs_blks, assr_ssr_blks;
	int assr_lfs_segs, assr_ssr_segs;
	int bggc_node_lfs_blks, bggc_node_ssr_blks;

	unsigned int segment_count[2];
	unsigned int block_count[2];
	unsigned int inplace_count;
	unsigned long long base_mem, cache_mem, page_mem;

	unsigned int gc_level[ALL_GC_LEVELS];
	unsigned long gc_times[ALL_GC_LEVELS];
};

static inline struct f2fs_stat_info *F2FS_STAT(struct f2fs_sb_info *sbi)
{
	return (struct f2fs_stat_info *)sbi->stat_info;
}


#define stat_inc_cp_count(si)		((si)->cp_count++)
#define stat_inc_bg_cp_count(si)	((si)->bg_cp_count++)
#define stat_inc_call_count(si)		((si)->call_count++)
#define stat_inc_bggc_count(sbi)	((sbi)->bg_gc++)
#define stat_io_skip_bggc_count(sbi)	((sbi)->io_skip_bggc++)
#define stat_other_skip_bggc_count(sbi)	((sbi)->other_skip_bggc++)
#define stat_inc_assr_lfs_blks(sbi)		((sbi)->assr_lfs_blks++)
#define stat_inc_assr_ssr_blks(sbi)		((sbi)->assr_ssr_blks++)
#define stat_inc_bggc_node_lfs_blks(sbi)	((sbi)->bggc_node_lfs_blks++)
#define stat_inc_bggc_node_ssr_blks(sbi)	((sbi)->bggc_node_ssr_blks++)
#define stat_inc_assr_lfs_segs(sbi)		((sbi)->assr_lfs_segs++)
#define stat_inc_assr_ssr_segs(sbi)		((sbi)->assr_ssr_segs++)
#define stat_io_skip_bggc_count(sbi)		((sbi)->io_skip_bggc++)
#define stat_other_skip_bggc_count(sbi)        ((sbi)->other_skip_bggc++)
#define stat_inc_dirty_inode(sbi, type)	((sbi)->ndirty_inode[type]++)
#define stat_dec_dirty_inode(sbi, type)	((sbi)->ndirty_inode[type]--)
#define stat_inc_total_hit(sbi)		(atomic64_inc(&(sbi)->total_hit_ext))
#define stat_inc_rbtree_node_hit(sbi)	(atomic64_inc(&(sbi)->read_hit_rbtree))
#define stat_inc_largest_node_hit(sbi)	(atomic64_inc(&(sbi)->read_hit_largest))
#define stat_inc_cached_node_hit(sbi)	(atomic64_inc(&(sbi)->read_hit_cached))
#define stat_inc_inline_xattr(inode)					\
	do {								\
		if (f2fs_has_inline_xattr(inode))			\
			(atomic_inc(&F2FS_I_SB(inode)->inline_xattr));	\
	} while (0)
#define stat_dec_inline_xattr(inode)					\
	do {								\
		if (f2fs_has_inline_xattr(inode))			\
			(atomic_dec(&F2FS_I_SB(inode)->inline_xattr));	\
	} while (0)
#define stat_inc_inline_inode(inode)					\
	do {								\
		if (f2fs_has_inline_data(inode))			\
			(atomic_inc(&F2FS_I_SB(inode)->inline_inode));	\
	} while (0)
#define stat_dec_inline_inode(inode)					\
	do {								\
		if (f2fs_has_inline_data(inode))			\
			(atomic_dec(&F2FS_I_SB(inode)->inline_inode));	\
	} while (0)
#define stat_inc_inline_dir(inode)					\
	do {								\
		if (f2fs_has_inline_dentry(inode))			\
			(atomic_inc(&F2FS_I_SB(inode)->inline_dir));	\
	} while (0)
#define stat_dec_inline_dir(inode)					\
	do {								\
		if (f2fs_has_inline_dentry(inode))			\
			(atomic_dec(&F2FS_I_SB(inode)->inline_dir));	\
	} while (0)
#define stat_inc_meta_count(sbi, blkaddr)				\
	do {								\
		if (blkaddr < SIT_I(sbi)->sit_base_addr)		\
			atomic_inc(&(sbi)->meta_count[META_CP]);	\
		else if (blkaddr < NM_I(sbi)->nat_blkaddr)		\
			atomic_inc(&(sbi)->meta_count[META_SIT]);	\
		else if (blkaddr < SM_I(sbi)->ssa_blkaddr)		\
			atomic_inc(&(sbi)->meta_count[META_NAT]);	\
		else if (blkaddr < SM_I(sbi)->main_blkaddr)		\
			atomic_inc(&(sbi)->meta_count[META_SSA]);	\
	} while (0)
#define stat_inc_seg_type(sbi, curseg)					\
		((sbi)->segment_count[(curseg)->alloc_type]++)
#define stat_inc_block_count(sbi, curseg)				\
		((sbi)->block_count[(curseg)->alloc_type]++)
#define stat_inc_inplace_blocks(sbi)					\
		(atomic_inc(&(sbi)->inplace_count))
#define stat_inc_atomic_write(inode)					\
		(atomic_inc(&F2FS_I_SB(inode)->aw_cnt))
#define stat_dec_atomic_write(inode)					\
		(atomic_dec(&F2FS_I_SB(inode)->aw_cnt))
#define stat_update_max_atomic_write(inode)				\
	do {								\
		int cur = atomic_read(&F2FS_I_SB(inode)->aw_cnt);	\
		int max = atomic_read(&F2FS_I_SB(inode)->max_aw_cnt);	\
		if (cur > max)						\
			atomic_set(&F2FS_I_SB(inode)->max_aw_cnt, cur);	\
	} while (0)
#define stat_inc_volatile_write(inode)					\
		(atomic_inc(&F2FS_I_SB(inode)->vw_cnt))
#define stat_dec_volatile_write(inode)					\
		(atomic_dec(&F2FS_I_SB(inode)->vw_cnt))
#define stat_update_max_volatile_write(inode)				\
	do {								\
		int cur = atomic_read(&F2FS_I_SB(inode)->vw_cnt);	\
		int max = atomic_read(&F2FS_I_SB(inode)->max_vw_cnt);	\
		if (cur > max)						\
			atomic_set(&F2FS_I_SB(inode)->max_vw_cnt, cur);	\
	} while (0)
#define stat_inc_seg_count(sbi, type, gc_type)				\
	do {								\
		struct f2fs_stat_info *si = F2FS_STAT(sbi);		\
		si->tot_segs++;						\
		if ((type) == SUM_TYPE_DATA) {				\
			si->data_segs++;				\
			si->bg_data_segs += (gc_type == BG_GC) ? 1 : 0;	\
		} else {						\
			si->node_segs++;				\
			si->bg_node_segs += (gc_type == BG_GC) ? 1 : 0;	\
		}							\
	} while (0)

#define stat_inc_tot_blk_count(si, blks)				\
	((si)->tot_blks += (blks))

#define stat_inc_data_blk_count(sbi, blks, gc_type)			\
	do {								\
		struct f2fs_stat_info *si = F2FS_STAT(sbi);		\
		stat_inc_tot_blk_count(si, blks);			\
		si->data_blks += (blks);				\
		si->bg_data_blks += ((gc_type) == BG_GC) ? (blks) : 0;	\
	} while (0)

#define stat_inc_node_blk_count(sbi, blks, gc_type)			\
	do {								\
		struct f2fs_stat_info *si = F2FS_STAT(sbi);		\
		stat_inc_tot_blk_count(si, blks);			\
		si->node_blks += (blks);				\
		si->bg_node_blks += ((gc_type) == BG_GC) ? (blks) : 0;	\
	} while (0)

int hmfs_build_stats(struct f2fs_sb_info *sbi);
void hmfs_destroy_stats(struct f2fs_sb_info *sbi);
int __init hmfs_create_root_stats(void);
void hmfs_destroy_root_stats(void);

/* for f2fs bigdata collections */
/* unit for time: ns */
enum {
	HC_UNSET,
	HC_HOT_DATA,
	HC_WARM_DATA,
	HC_COLD_DATA,
	HC_HOT_NODE,
	HC_WARM_NODE,
	HC_COLD_NODE,
	NR_CURSEG,
	HC_META = NR_CURSEG,
	HC_META_SB,
	HC_META_CP,
	HC_META_SIT,
	HC_META_NAT,
	HC_META_SSA,
	HC_DIRECTIO,
	HC_GC_COLD_DATA,
	HC_REWRITE_HOT_DATA,
	HC_REWRITE_WARM_DATA,
	NR_HOTCOLD_TYPE,
};
struct f2fs_bigdata_info {
	unsigned int cp_cnt, cp_succ_cnt;
	u64 cp_time, max_cp_time, max_cp_submit_time,
	    max_f2fs_cp_flush_meta_time, max_cp_discard_time;
	/* avg_cp_time = cp_time / sbi->cp_cnt */

	unsigned int discard_cnt, discard_blk_cnt, undiscard_cnt,
		     undiscard_blk_cnt;
	u64 discard_time, max_discard_time;
	/* avg_discard_time = discard_time / discard_cnt */

	/* BG_GC = 0, FG_GC = 1 */
	unsigned int gc_cnt[2], gc_fail_cnt[2],
		     gc_data_cnt[2], gc_node_cnt[2];
	unsigned int gc_data_seg_cnt[2], gc_data_blk_cnt[2],
		     gc_node_seg_cnt[2], gc_node_blk_cnt[2];
	u64 fggc_time;
	/*
	 * avg_[bg|fg]gc_data_seg_cnt = [bg|fg]gc_data_seg_cnt / [bg|fg]gc_data_cnt
	 * avg_[bg|fg]gc_data_blk_cnt = [bg|fg]gc_data_blk_cnt / [bg|fg]gc_data_cnt
	 * avg_fggc_time = fggc_time / fggc_cnt
	 */
	/* LFS = 0, SSR = 1 */
	unsigned int node_alloc_cnt[2], data_alloc_cnt[2], data_ipu_cnt;

	unsigned long last_node_alloc_cnt, last_data_alloc_cnt;
	unsigned long curr_node_alloc_cnt, curr_data_alloc_cnt;
	unsigned long ssr_last_jiffies;

	unsigned int fsync_reg_file_cnt, fsync_dir_cnt;
	u64 fsync_time, max_fsync_time, fsync_cp_time, max_fsync_cp_time,
	    fsync_wr_file_time, max_fsync_wr_file_time, fsync_sync_node_time,
	    max_fsync_sync_node_time, fsync_flush_time, max_fsync_flush_time;

	unsigned long hotcold_cnt[NR_HOTCOLD_TYPE];
	unsigned long hotcold_gc_seg_cnt[NR_CURSEG];
	unsigned long hotcold_gc_blk_cnt[NR_CURSEG];

	union {
		struct encrypt_struct {
			unsigned int orig_corrupt:1;
			unsigned int orig_nonexist:1;
			unsigned int bak_corrupt:1;
			unsigned int bak_nonexist:1;
			unsigned int crc_corrupt:1;
			unsigned int crc_nonexist:1;
			unsigned int fixed:1;
		} encrypt_struct;
		unsigned int encrypt_val;
	} encrypt;

	u64 throttle_cnt_fg, throttle_time_fg, max_throttle_time_fg,
		throttle_cnt_bg, throttle_time_bg, max_throttle_time_bg;
};

static inline struct f2fs_bigdata_info *F2FS_BD_STAT(struct f2fs_sb_info *sbi)
{
	return (struct f2fs_bigdata_info *)sbi->bd_info;
}

#define inc_bd_val(sbi, member, val) do {			\
	struct f2fs_bigdata_info *bd = F2FS_BD_STAT(sbi);	\
	if (bd)							\
		bd->member += (val);				\
} while (0)
#define inc_bd_array_val(sbi, member, idx, val) do {		\
	struct f2fs_bigdata_info *bd = F2FS_BD_STAT(sbi);	\
	if (bd)							\
		bd->member[(idx)] += (val);			\
} while (0)

#define set_bd_val(sbi, member, val) do {			\
	struct f2fs_bigdata_info *bd = F2FS_BD_STAT(sbi);	\
	if (bd)							\
		bd->member = (val);				\
} while (0)
#define set_bd_array_val(sbi, member, idx, val) do {		\
	struct f2fs_bigdata_info *bd = F2FS_BD_STAT(sbi);	\
	if (bd)							\
		bd->member[(idx)] = (val);			\
} while (0)

#define max_bd_val(sbi, member, val) do {			\
	struct f2fs_bigdata_info *bd = F2FS_BD_STAT(sbi);	\
	if (bd) {						\
		if (bd->member < (val))				\
			bd->member = (val);			\
	}							\
} while (0)

#define bd_mutex_lock(mutex) mutex_lock((mutex))
#define bd_mutex_unlock(mutex) mutex_unlock((mutex))
#else
#define stat_inc_cp_count(si)				do { } while (0)
#define stat_inc_bg_cp_count(si)			do { } while (0)
#define stat_inc_call_count(si)				do { } while (0)
#define stat_inc_bggc_count(si)				do { } while (0)
#define stat_io_skip_bggc_count(sbi)			do { } while (0)
#define stat_other_skip_bggc_count(sbi)			do { } while (0)
#define stat_inc_dirty_inode(sbi, type)			do { } while (0)
#define stat_dec_dirty_inode(sbi, type)			do { } while (0)
#define stat_inc_total_hit(sb)				do { } while (0)
#define stat_inc_rbtree_node_hit(sb)			do { } while (0)
#define stat_inc_largest_node_hit(sbi)			do { } while (0)
#define stat_inc_cached_node_hit(sbi)			do { } while (0)
#define stat_inc_inline_xattr(inode)			do { } while (0)
#define stat_dec_inline_xattr(inode)			do { } while (0)
#define stat_inc_inline_inode(inode)			do { } while (0)
#define stat_dec_inline_inode(inode)			do { } while (0)
#define stat_inc_inline_dir(inode)			do { } while (0)
#define stat_dec_inline_dir(inode)			do { } while (0)
#define stat_inc_atomic_write(inode)			do { } while (0)
#define stat_dec_atomic_write(inode)			do { } while (0)
#define stat_update_max_atomic_write(inode)		do { } while (0)
#define stat_inc_volatile_write(inode)			do { } while (0)
#define stat_dec_volatile_write(inode)			do { } while (0)
#define stat_update_max_volatile_write(inode)		do { } while (0)
#define stat_inc_meta_count(sbi, blkaddr)		do { } while (0)
#define stat_inc_seg_type(sbi, curseg)			do { } while (0)
#define stat_inc_block_count(sbi, curseg)		do { } while (0)
#define stat_inc_inplace_blocks(sbi)			do { } while (0)
#define stat_inc_seg_count(sbi, type, gc_type)		do { } while (0)
#define stat_inc_tot_blk_count(si, blks)		do { } while (0)
#define stat_inc_data_blk_count(sbi, blks, gc_type)	do { } while (0)
#define stat_inc_node_blk_count(sbi, blks, gc_type)	do { } while (0)


static inline int hmfs_build_stats(struct f2fs_sb_info *sbi) { return 0; }
static inline void hmfs_destroy_stats(struct f2fs_sb_info *sbi) { }
static inline int __init hmfs_create_root_stats(void) { return 0; }
static inline void hmfs_destroy_root_stats(void) { }

#define inc_bd_val(sbi, member, val)
#define inc_bd_array_val(sbi, member, idx, val)
#define set_bd_val(sbi, member, val)
#define set_bd_array_val(sbi, member, idx, val)
#define max_bd_val(sbi, member, val)
#define bd_mutex_lock(mutex)
#define bd_mutex_unlock(mutex)
#endif

extern const struct file_operations hmfs_dir_operations;
extern const struct file_operations hmfs_file_operations;
extern const struct inode_operations hmfs_file_inode_operations;
extern const struct address_space_operations hmfs_dblock_aops;
extern const struct address_space_operations hmfs_node_aops;
extern const struct address_space_operations hmfs_meta_aops;
extern const struct inode_operations hmfs_dir_inode_operations;
extern const struct inode_operations hmfs_symlink_inode_operations;
extern const struct inode_operations hmfs_encrypted_symlink_inode_operations;
extern const struct inode_operations hmfs_special_inode_operations;
extern struct kmem_cache *hmfs_inode_entry_slab;

/*
 * inline.c
 */
bool hmfs_may_inline_data(struct inode *inode);
bool hmfs_may_inline_dentry(struct inode *inode);
void hmfs_do_read_inline_data(struct page *page, struct page *ipage);
void hmfs_truncate_inline_inode(struct inode *inode,
						struct page *ipage, u64 from);
int hmfs_read_inline_data(struct inode *inode, struct page *page);
int hmfs_convert_inline_page(struct dnode_of_data *dn, struct page *page);
int hmfs_convert_inline_inode(struct inode *inode);
int hmfs_write_inline_data_ex(struct inode *inode,
                       struct page *page, bool quick_write);
static inline int f2fs_write_inline_data(struct inode *inode, struct page *page)
{
       return hmfs_write_inline_data_ex(inode, page, false);
}
bool hmfs_recover_inline_data(struct inode *inode, struct page *npage);
struct f2fs_dir_entry *hmfs_find_in_inline_dir(struct inode *dir,
			struct fscrypt_name *fname, struct page **res_page);
int hmfs_make_empty_inline_dir(struct inode *inode, struct inode *parent,
			struct page *ipage);
int hmfs_add_inline_entry(struct inode *dir, const struct qstr *new_name,
			const struct qstr *orig_name, struct dentry *dentry,
			struct inode *inode, nid_t ino, umode_t mode);
void hmfs_delete_inline_entry(struct f2fs_dir_entry *dentry,
				struct page *page, struct inode *dir,
				struct inode *inode);
bool hmfs_empty_inline_dir(struct inode *dir);
int hmfs_read_inline_dir(struct file *file, struct dir_context *ctx,
			struct fscrypt_str *fstr);
int hmfs_inline_data_fiemap(struct inode *inode,
			struct fiemap_extent_info *fieinfo,
			__u64 start, __u64 len);

/*
 * shrinker.c
 */
unsigned long hmfs_shrink_count(struct shrinker *shrink,
			struct shrink_control *sc);
unsigned long hmfs_shrink_scan(struct shrinker *shrink,
			struct shrink_control *sc);
void hmfs_join_shrinker(struct f2fs_sb_info *sbi);
void hmfs_leave_shrinker(struct f2fs_sb_info *sbi);

/*
 * extent_cache.c
 */
struct rb_entry *hmfs_lookup_rb_tree(struct rb_root_cached *root,
				struct rb_entry *cached_re, unsigned int ofs);
struct rb_node **__hmfs_lookup_rb_tree_ext(struct f2fs_sb_info *sbi,
                                struct rb_root *root, struct rb_node **parent,
                                unsigned long long key);
struct rb_node **hmfs_lookup_rb_tree_for_insert(struct f2fs_sb_info *sbi,
				struct rb_root_cached *root,
				struct rb_node **parent,
				unsigned int ofs, bool *leftmost);
struct rb_entry *hmfs_lookup_rb_tree_ret(struct rb_root_cached *root,
		struct rb_entry *cached_re, unsigned int ofs,
		struct rb_entry **prev_entry, struct rb_entry **next_entry,
		struct rb_node ***insert_p, struct rb_node **insert_parent,
		bool force, bool *leftmost);
bool f2fs_check_rb_tree_consistence_discard(struct f2fs_sb_info *sbi,
						struct rb_root_cached *root);
unsigned int hmfs_shrink_extent_tree(struct f2fs_sb_info *sbi, int nr_shrink);
bool hmfs_init_extent_tree(struct inode *inode, struct f2fs_extent *i_ext);
void hmfs_drop_extent_tree(struct inode *inode);
unsigned int hmfs_destroy_extent_node(struct inode *inode);
void hmfs_destroy_extent_tree(struct inode *inode);
bool hmfs_lookup_extent_cache(struct inode *inode, pgoff_t pgofs,
			struct extent_info *ei);
void hmfs_update_extent_cache(struct dnode_of_data *dn);
void hmfs_update_extent_cache_range(struct dnode_of_data *dn,
			pgoff_t fofs, block_t blkaddr, unsigned int len);
void hmfs_init_extent_cache_info(struct f2fs_sb_info *sbi);
int __init hmfs_create_extent_cache(void);
void hmfs_destroy_extent_cache(void);

/*
 * sysfs.c
 */
int __init hmfs_init_sysfs(void);
void hmfs_exit_sysfs(void);
int hmfs_register_sysfs(struct f2fs_sb_info *sbi);
void hmfs_unregister_sysfs(struct f2fs_sb_info *sbi);

/*
 * crypto support
 */
static inline bool f2fs_encrypted_inode(struct inode *inode)
{
	return file_is_encrypt(inode);
}

static inline bool f2fs_encrypted_file(struct inode *inode)
{
	return f2fs_encrypted_inode(inode) && S_ISREG(inode->i_mode);
}

static inline void f2fs_set_encrypted_inode(struct inode *inode)
{
#ifdef CONFIG_HMFS_FS_ENCRYPTION
	file_set_encrypt(inode);
	hmfs_set_inode_flags(inode);
#endif
}

static inline bool f2fs_inline_encrypted_inode(struct inode *inode)
{
        return file_is_inline_encrypt(inode);
}

static inline void f2fs_set_inline_encrypted_inode(struct inode *inode)
{
#ifdef CONFIG_HMFS_FS_ENCRYPTION
        file_set_inline_encrypt(inode);
#endif
}


/*
 * Returns true if the reads of the inode's data need to undergo some
 * postprocessing step, like decryption or authenticity verification.
 */
static inline bool f2fs_post_read_required(struct inode *inode)
{
	return f2fs_encrypted_file(inode);
}

#define F2FS_FEATURE_FUNCS(name, flagname) \
static inline int f2fs_sb_has_##name(struct super_block *sb) \
{ \
	return F2FS_HAS_FEATURE(sb, F2FS_FEATURE_##flagname); \
}

F2FS_FEATURE_FUNCS(encrypt, ENCRYPT);
F2FS_FEATURE_FUNCS(blkzoned, BLKZONED);
F2FS_FEATURE_FUNCS(extra_attr, EXTRA_ATTR);
F2FS_FEATURE_FUNCS(project_quota, PRJQUOTA);
F2FS_FEATURE_FUNCS(inode_chksum, INODE_CHKSUM);
F2FS_FEATURE_FUNCS(flexible_inline_xattr, FLEXIBLE_INLINE_XATTR);
F2FS_FEATURE_FUNCS(quota_ino, QUOTA_INO);
F2FS_FEATURE_FUNCS(inode_crtime, INODE_CRTIME);
F2FS_FEATURE_FUNCS(lost_found, LOST_FOUND);
F2FS_FEATURE_FUNCS(sb_chksum, SB_CHKSUM);

#ifdef CONFIG_BLK_DEV_ZONED
static inline int get_blkz_type(struct f2fs_sb_info *sbi,
			struct block_device *bdev, block_t blkaddr)
{
	unsigned int zno = blkaddr >> sbi->log_blocks_per_blkz;
	int i;

	for (i = 0; i < sbi->s_ndevs; i++)
		if (FDEV(i).bdev == bdev)
			return FDEV(i).blkz_type[zno];
	return -EINVAL;
}
#endif

static inline bool f2fs_hw_should_discard(struct f2fs_sb_info *sbi)
{
	return f2fs_sb_has_blkzoned(sbi->sb);
}

static inline bool f2fs_hw_support_discard(struct f2fs_sb_info *sbi)
{
	return blk_queue_discard(bdev_get_queue(sbi->sb->s_bdev));
}

static inline bool f2fs_realtime_discard_enable(struct f2fs_sb_info *sbi)
{
	return (test_opt(sbi, DISCARD) && f2fs_hw_support_discard(sbi)) ||
					f2fs_hw_should_discard(sbi);
}

static inline void set_opt_mode(struct f2fs_sb_info *sbi, unsigned int mt)
{
	clear_opt(sbi, ADAPTIVE);
	clear_opt(sbi, LFS);

	switch (mt) {
	case F2FS_MOUNT_ADAPTIVE:
		set_opt(sbi, ADAPTIVE);
		break;
	case F2FS_MOUNT_LFS:
		set_opt(sbi, LFS);
		break;
	}
}

static inline bool f2fs_may_encrypt(struct inode *inode)
{
#ifdef CONFIG_HMFS_FS_ENCRYPTION
	umode_t mode = inode->i_mode;

	return (S_ISREG(mode) || S_ISDIR(mode) || S_ISLNK(mode));
#else
	return false;
#endif
}

static inline int block_unaligned_IO(struct inode *inode,
				struct kiocb *iocb, struct iov_iter *iter)
{
	unsigned int i_blkbits = READ_ONCE(inode->i_blkbits);
	unsigned int blocksize_mask = (1 << i_blkbits) - 1;
	loff_t offset = iocb->ki_pos;
	unsigned long align = offset | iov_iter_alignment(iter);

	return align & blocksize_mask;
}

static inline int allow_outplace_dio(struct inode *inode,
				struct kiocb *iocb, struct iov_iter *iter)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	int rw = iov_iter_rw(iter);

	return (test_opt(sbi, LFS) && (rw == WRITE) &&
				!block_unaligned_IO(inode, iocb, iter));
}

static inline bool f2fs_force_buffered_io(struct inode *inode,
				struct kiocb *iocb, struct iov_iter *iter)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	int rw = iov_iter_rw(iter);

	if (sbi->s_ndevs)
		return true;
	/*
	 * for blkzoned device, fallback direct IO to buffered IO, so
	 * all IOs can be serialized by log-structured write.
	 */
	if (f2fs_sb_has_blkzoned(sbi->sb))
		return true;
	if (test_opt(sbi, LFS) && (rw == WRITE) &&
				block_unaligned_IO(inode, iocb, iter))
		return true;
	if (is_sbi_flag_set(F2FS_I_SB(inode), SBI_CP_DISABLED))
		return true;

	return false;
}

#ifdef CONFIG_HMFS_FAULT_INJECTION
extern void f2fs_build_fault_attr(struct f2fs_sb_info *sbi, unsigned int rate,
							unsigned int type);
#else
#define f2fs_build_fault_attr(sbi, rate, type)		do { } while (0)
#endif

#define F2FS_SIZE_MB (1024 * 1024)

static inline unsigned long long blks_to_mb(block_t blks,
		unsigned int blocksize)
{
	return div_u64((unsigned long long)blks * blocksize, F2FS_SIZE_MB);
}

static inline bool is_journalled_quota(struct f2fs_sb_info *sbi)
{
#ifdef CONFIG_QUOTA
	if (f2fs_sb_has_quota_ino(sbi->sb))
		return true;
	if (F2FS_OPTION(sbi).s_qf_names[USRQUOTA] ||
		F2FS_OPTION(sbi).s_qf_names[GRPQUOTA] ||
		F2FS_OPTION(sbi).s_qf_names[PRJQUOTA])
		return true;
#endif
	return false;
}

#define EFSCORRUPTED	EUCLEAN		/* Filesystem is corrupted */

#endif
