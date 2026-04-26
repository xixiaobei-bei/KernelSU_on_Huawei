/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM hmfs

#if !defined(_TRACE_F2FS_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_F2FS_H

#include <linux/tracepoint.h>

#define show_dev(dev)		MAJOR(dev), MINOR(dev)
#define show_dev_ino(entry)	show_dev(entry->dev), (unsigned long)entry->ino

TRACE_DEFINE_ENUM(NODE);
TRACE_DEFINE_ENUM(DATA);
TRACE_DEFINE_ENUM(META);
TRACE_DEFINE_ENUM(META_FLUSH);
TRACE_DEFINE_ENUM(INMEM);
TRACE_DEFINE_ENUM(INMEM_DROP);
TRACE_DEFINE_ENUM(INMEM_INVALIDATE);
TRACE_DEFINE_ENUM(INMEM_REVOKE);
TRACE_DEFINE_ENUM(IPU);
TRACE_DEFINE_ENUM(OPU);
TRACE_DEFINE_ENUM(HOT);
TRACE_DEFINE_ENUM(WARM);
TRACE_DEFINE_ENUM(COLD);
TRACE_DEFINE_ENUM(CURSEG_HOT_DATA);
TRACE_DEFINE_ENUM(CURSEG_WARM_DATA);
TRACE_DEFINE_ENUM(CURSEG_COLD_DATA);
TRACE_DEFINE_ENUM(CURSEG_HOT_NODE);
TRACE_DEFINE_ENUM(CURSEG_WARM_NODE);
TRACE_DEFINE_ENUM(CURSEG_COLD_NODE);
TRACE_DEFINE_ENUM(NO_CHECK_TYPE);
TRACE_DEFINE_ENUM(GC_GREEDY);
TRACE_DEFINE_ENUM(GC_CB);
TRACE_DEFINE_ENUM(FG_GC);
TRACE_DEFINE_ENUM(BG_GC);
TRACE_DEFINE_ENUM(LFS);
TRACE_DEFINE_ENUM(SSR);
TRACE_DEFINE_ENUM(__REQ_RAHEAD);
TRACE_DEFINE_ENUM(__REQ_SYNC);
TRACE_DEFINE_ENUM(__REQ_IDLE);
TRACE_DEFINE_ENUM(__REQ_PREFLUSH);
TRACE_DEFINE_ENUM(__REQ_FUA);
TRACE_DEFINE_ENUM(__REQ_PRIO);
TRACE_DEFINE_ENUM(__REQ_META);
TRACE_DEFINE_ENUM(CP_UMOUNT);
TRACE_DEFINE_ENUM(CP_FASTBOOT);
TRACE_DEFINE_ENUM(CP_SYNC);
TRACE_DEFINE_ENUM(CP_RECOVERY);
TRACE_DEFINE_ENUM(CP_DISCARD);
TRACE_DEFINE_ENUM(CP_TRIMMED);

#define show_block_type(type)						\
	__print_symbolic(type,						\
		{ NODE,		"NODE" },				\
		{ DATA,		"DATA" },				\
		{ META,		"META" },				\
		{ META_FLUSH,	"META_FLUSH" },				\
		{ INMEM,	"INMEM" },				\
		{ INMEM_DROP,	"INMEM_DROP" },				\
		{ INMEM_INVALIDATE,	"INMEM_INVALIDATE" },		\
		{ INMEM_REVOKE,	"INMEM_REVOKE" },			\
		{ IPU,		"IN-PLACE" },				\
		{ OPU,		"OUT-OF-PLACE" })

#define show_block_temp(temp)						\
	__print_symbolic(temp,						\
		{ HOT,		"HOT" },				\
		{ WARM,		"WARM" },				\
		{ COLD,		"COLD" })

#define F2FS_OP_FLAGS (REQ_RAHEAD | REQ_SYNC | REQ_META | REQ_PRIO |	\
			REQ_PREFLUSH | REQ_FUA)
#define F2FS_BIO_FLAG_MASK(t)	(t & F2FS_OP_FLAGS)

#define show_bio_type(op,op_flags)	show_bio_op(op),		\
						show_bio_op_flags(op_flags)

#define show_bio_op(op)							\
	__print_symbolic(op,						\
		{ REQ_OP_READ,			"READ" },		\
		{ REQ_OP_WRITE,			"WRITE" },		\
		{ REQ_OP_FLUSH,			"FLUSH" },		\
		{ REQ_OP_DISCARD,		"DISCARD" },		\
		{ REQ_OP_ZONE_REPORT,		"ZONE_REPORT" },	\
		{ REQ_OP_SECURE_ERASE,		"SECURE_ERASE" },	\
		{ REQ_OP_ZONE_RESET,		"ZONE_RESET" },		\
		{ REQ_OP_WRITE_SAME,		"WRITE_SAME" },		\
		{ REQ_OP_WRITE_ZEROES,		"WRITE_ZEROES" })

#define show_bio_op_flags(flags)					\
	__print_flags(F2FS_BIO_FLAG_MASK(flags), "|",			\
		{ REQ_RAHEAD,		"R" },				\
		{ REQ_SYNC,		"S" },				\
		{ REQ_META,		"M" },				\
		{ REQ_PRIO,		"P" },				\
		{ REQ_PREFLUSH,		"PF" },				\
		{ REQ_FUA,		"FUA" })

#define show_data_type(type)						\
	__print_symbolic(type,						\
		{ CURSEG_HOT_DATA,	"Hot DATA" },			\
		{ CURSEG_WARM_DATA,	"Warm DATA" },			\
		{ CURSEG_COLD_DATA,	"Cold DATA" },			\
		{ CURSEG_HOT_NODE,	"Hot NODE" },			\
		{ CURSEG_WARM_NODE,	"Warm NODE" },			\
		{ CURSEG_COLD_NODE,	"Cold NODE" },			\
		{ NO_CHECK_TYPE,	"No TYPE" })

#define show_file_type(type)						\
	__print_symbolic(type,						\
		{ 0,		"FILE" },				\
		{ 1,		"DIR" })

#define show_gc_type(type)						\
	__print_symbolic(type,						\
		{ FG_GC,	"Foreground GC" },			\
		{ BG_GC,	"Background GC" })

#define show_alloc_mode(type)						\
	__print_symbolic(type,						\
		{ LFS,	"LFS-mode" },					\
		{ SSR,	"SSR-mode" })

#define show_victim_policy(type)					\
	__print_symbolic(type,						\
		{ GC_GREEDY,	"Greedy" },				\
		{ GC_CB,	"Cost-Benefit" })

#define show_cpreason(type)						\
	__print_flags(type, "|",					\
		{ CP_UMOUNT,	"Umount" },				\
		{ CP_FASTBOOT,	"Fastboot" },				\
		{ CP_SYNC,	"Sync" },				\
		{ CP_RECOVERY,	"Recovery" },				\
		{ CP_DISCARD,	"Discard" },				\
		{ CP_UMOUNT,	"Umount" },				\
		{ CP_TRIMMED,	"Trimmed" })

#define show_fsync_cpreason(type)					\
	__print_symbolic(type,						\
		{ CP_NO_NEEDED,		"no needed" },			\
		{ CP_NON_REGULAR,	"non regular" },		\
		{ CP_HARDLINK,		"hardlink" },			\
		{ CP_SB_NEED_CP,	"sb needs cp" },		\
		{ CP_WRONG_PINO,	"wrong pino" },			\
		{ CP_NO_SPC_ROLL,	"no space roll forward" },	\
		{ CP_NODE_NEED_CP,	"node needs cp" },		\
		{ CP_FASTBOOT_MODE,	"fastboot mode" },		\
		{ CP_SPEC_LOG_NUM,	"log type is 2" },		\
		{ CP_RECOVER_DIR,	"dir needs recovery" },		\
		{ CP_SWITCH_STREAM,	"stream has switched" },	\
		{ CP_XATTR_DIRTY,	"xattr needs recovery" },	\
		{ CP_OOB_OVERFLOW,	"exceed oob write limit" },	\
		{ CP_OOB_CHG_ATOMIC,	"exchange atomic write" })


struct f2fs_io_info;
struct extent_info;
struct f2fs_sb_info;
struct victim_sel_policy;
struct f2fs_map_blocks;

DECLARE_EVENT_CLASS(f2fs__inode,

	TP_PROTO(struct inode *inode),

	TP_ARGS(inode),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(ino_t,	pino)
		__field(umode_t, mode)
		__field(loff_t,	size)
		__field(unsigned int, nlink)
		__field(blkcnt_t, blocks)
		__field(__u8,	advise)
	),

	TP_fast_assign(
		__entry->dev	= inode->i_sb->s_dev;
		__entry->ino	= inode->i_ino;
		__entry->pino	= F2FS_I(inode)->i_pino;
		__entry->mode	= inode->i_mode;
		__entry->nlink	= inode->i_nlink;
		__entry->size	= inode->i_size;
		__entry->blocks	= inode->i_blocks;
		__entry->advise	= F2FS_I(inode)->i_advise;
	),

	TP_printk("dev = (%d,%d), ino = %lu, pino = %lu, i_mode = 0x%hx, "
		"i_size = %lld, i_nlink = %u, i_blocks = %llu, i_advise = 0x%x",
		show_dev_ino(__entry),
		(unsigned long)__entry->pino,
		__entry->mode,
		__entry->size,
		(unsigned int)__entry->nlink,
		(unsigned long long)__entry->blocks,
		(unsigned char)__entry->advise)
);

DECLARE_EVENT_CLASS(f2fs__inode_exit,

	TP_PROTO(struct inode *inode, int ret),

	TP_ARGS(inode, ret),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(int,	ret)
	),

	TP_fast_assign(
		__entry->dev	= inode->i_sb->s_dev;
		__entry->ino	= inode->i_ino;
		__entry->ret	= ret;
	),

	TP_printk("dev = (%d,%d), ino = %lu, ret = %d",
		show_dev_ino(__entry),
		__entry->ret)
);

DEFINE_EVENT(f2fs__inode, hmfs_sync_file_enter,

	TP_PROTO(struct inode *inode),

	TP_ARGS(inode)
);

TRACE_EVENT(hmfs_sync_file_exit,

	TP_PROTO(struct inode *inode, int cp_reason, int datasync, int ret),

	TP_ARGS(inode, cp_reason, datasync, ret),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(int,	cp_reason)
		__field(int,	datasync)
		__field(int,	ret)
	),

	TP_fast_assign(
		__entry->dev		= inode->i_sb->s_dev;
		__entry->ino		= inode->i_ino;
		__entry->cp_reason	= cp_reason;
		__entry->datasync	= datasync;
		__entry->ret		= ret;
	),

	TP_printk("dev = (%d,%d), ino = %lu, cp_reason: %s, "
		"datasync = %d, ret = %d",
		show_dev_ino(__entry),
		show_fsync_cpreason(__entry->cp_reason),
		__entry->datasync,
		__entry->ret)
);

TRACE_EVENT(hmfs_sync_fs,

	TP_PROTO(struct super_block *sb, int wait),

	TP_ARGS(sb, wait),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(int,	dirty)
		__field(int,	wait)
	),

	TP_fast_assign(
		__entry->dev	= sb->s_dev;
		__entry->dirty	= is_sbi_flag_set(F2FS_SB(sb), SBI_IS_DIRTY);
		__entry->wait	= wait;
	),

	TP_printk("dev = (%d,%d), superblock is %s, wait = %d",
		show_dev(__entry->dev),
		__entry->dirty ? "dirty" : "not dirty",
		__entry->wait)
);

DEFINE_EVENT(f2fs__inode, hmfs_iget,

	TP_PROTO(struct inode *inode),

	TP_ARGS(inode)
);

DEFINE_EVENT(f2fs__inode_exit, hmfs_iget_exit,

	TP_PROTO(struct inode *inode, int ret),

	TP_ARGS(inode, ret)
);

DEFINE_EVENT(f2fs__inode, hmfs_evict_inode,

	TP_PROTO(struct inode *inode),

	TP_ARGS(inode)
);

DEFINE_EVENT(f2fs__inode_exit, hmfs_new_inode,

	TP_PROTO(struct inode *inode, int ret),

	TP_ARGS(inode, ret)
);

TRACE_EVENT(hmfs_unlink_enter,

	TP_PROTO(struct inode *dir, struct dentry *dentry),

	TP_ARGS(dir, dentry),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(loff_t,	size)
		__field(blkcnt_t, blocks)
		__field(const char *,	name)
	),

	TP_fast_assign(
		__entry->dev	= dir->i_sb->s_dev;
		__entry->ino	= dir->i_ino;
		__entry->size	= dir->i_size;
		__entry->blocks	= dir->i_blocks;
		__entry->name	= dentry->d_name.name;
	),

	TP_printk("dev = (%d,%d), dir ino = %lu, i_size = %lld, "
		"i_blocks = %llu, name = %s",
		show_dev_ino(__entry),
		__entry->size,
		(unsigned long long)__entry->blocks,
		__entry->name)
);

DEFINE_EVENT(f2fs__inode_exit, hmfs_unlink_exit,

	TP_PROTO(struct inode *inode, int ret),

	TP_ARGS(inode, ret)
);

DEFINE_EVENT(f2fs__inode_exit, hmfs_drop_inode,

	TP_PROTO(struct inode *inode, int ret),

	TP_ARGS(inode, ret)
);

DEFINE_EVENT(f2fs__inode, hmfs_truncate,

	TP_PROTO(struct inode *inode),

	TP_ARGS(inode)
);

TRACE_EVENT(hmfs_truncate_data_blocks_range,

	TP_PROTO(struct inode *inode, nid_t nid, unsigned int ofs, int free),

	TP_ARGS(inode, nid,  ofs, free),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(nid_t,	nid)
		__field(unsigned int,	ofs)
		__field(int,	free)
	),

	TP_fast_assign(
		__entry->dev	= inode->i_sb->s_dev;
		__entry->ino	= inode->i_ino;
		__entry->nid	= nid;
		__entry->ofs	= ofs;
		__entry->free	= free;
	),

	TP_printk("dev = (%d,%d), ino = %lu, nid = %u, offset = %u, freed = %d",
		show_dev_ino(__entry),
		(unsigned int)__entry->nid,
		__entry->ofs,
		__entry->free)
);

DECLARE_EVENT_CLASS(f2fs__truncate_op,

	TP_PROTO(struct inode *inode, u64 from),

	TP_ARGS(inode, from),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(loff_t,	size)
		__field(blkcnt_t, blocks)
		__field(u64,	from)
	),

	TP_fast_assign(
		__entry->dev	= inode->i_sb->s_dev;
		__entry->ino	= inode->i_ino;
		__entry->size	= inode->i_size;
		__entry->blocks	= inode->i_blocks;
		__entry->from	= from;
	),

	TP_printk("dev = (%d,%d), ino = %lu, i_size = %lld, i_blocks = %llu, "
		"start file offset = %llu",
		show_dev_ino(__entry),
		__entry->size,
		(unsigned long long)__entry->blocks,
		(unsigned long long)__entry->from)
);

DEFINE_EVENT(f2fs__truncate_op, hmfs_truncate_blocks_enter,

	TP_PROTO(struct inode *inode, u64 from),

	TP_ARGS(inode, from)
);

DEFINE_EVENT(f2fs__inode_exit, hmfs_truncate_blocks_exit,

	TP_PROTO(struct inode *inode, int ret),

	TP_ARGS(inode, ret)
);

DEFINE_EVENT(f2fs__truncate_op, hmfs_truncate_inode_blocks_enter,

	TP_PROTO(struct inode *inode, u64 from),

	TP_ARGS(inode, from)
);

DEFINE_EVENT(f2fs__inode_exit, hmfs_truncate_inode_blocks_exit,

	TP_PROTO(struct inode *inode, int ret),

	TP_ARGS(inode, ret)
);

DECLARE_EVENT_CLASS(f2fs__truncate_node,

	TP_PROTO(struct inode *inode, nid_t nid, block_t blk_addr),

	TP_ARGS(inode, nid, blk_addr),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(nid_t,	nid)
		__field(block_t,	blk_addr)
	),

	TP_fast_assign(
		__entry->dev		= inode->i_sb->s_dev;
		__entry->ino		= inode->i_ino;
		__entry->nid		= nid;
		__entry->blk_addr	= blk_addr;
	),

	TP_printk("dev = (%d,%d), ino = %lu, nid = %u, block_address = 0x%llx",
		show_dev_ino(__entry),
		(unsigned int)__entry->nid,
		(unsigned long long)__entry->blk_addr)
);

DEFINE_EVENT(f2fs__truncate_node, hmfs_truncate_nodes_enter,

	TP_PROTO(struct inode *inode, nid_t nid, block_t blk_addr),

	TP_ARGS(inode, nid, blk_addr)
);

DEFINE_EVENT(f2fs__inode_exit, hmfs_truncate_nodes_exit,

	TP_PROTO(struct inode *inode, int ret),

	TP_ARGS(inode, ret)
);

DEFINE_EVENT(f2fs__truncate_node, hmfs_truncate_node,

	TP_PROTO(struct inode *inode, nid_t nid, block_t blk_addr),

	TP_ARGS(inode, nid, blk_addr)
);

TRACE_EVENT(hmfs_truncate_partial_nodes,

	TP_PROTO(struct inode *inode, nid_t nid[], int depth, int err),

	TP_ARGS(inode, nid, depth, err),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(nid_t,	nid[3])
		__field(int,	depth)
		__field(int,	err)
	),

	TP_fast_assign(
		__entry->dev	= inode->i_sb->s_dev;
		__entry->ino	= inode->i_ino;
		__entry->nid[0]	= nid[0];
		__entry->nid[1]	= nid[1];
		__entry->nid[2]	= nid[2];
		__entry->depth	= depth;
		__entry->err	= err;
	),

	TP_printk("dev = (%d,%d), ino = %lu, "
		"nid[0] = %u, nid[1] = %u, nid[2] = %u, depth = %d, err = %d",
		show_dev_ino(__entry),
		(unsigned int)__entry->nid[0],
		(unsigned int)__entry->nid[1],
		(unsigned int)__entry->nid[2],
		__entry->depth,
		__entry->err)
);

TRACE_EVENT(hmfs_file_write_iter,

	TP_PROTO(struct inode *inode, unsigned long offset,
		unsigned long length, int ret),

	TP_ARGS(inode, offset, length, ret),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(unsigned long, offset)
		__field(unsigned long, length)
		__field(int,	ret)
	),

	TP_fast_assign(
		__entry->dev	= inode->i_sb->s_dev;
		__entry->ino	= inode->i_ino;
		__entry->offset	= offset;
		__entry->length	= length;
		__entry->ret	= ret;
	),

	TP_printk("dev = (%d,%d), ino = %lu, "
		"offset = %lu, length = %lu, written(err) = %d",
		show_dev_ino(__entry),
		__entry->offset,
		__entry->length,
		__entry->ret)
);

TRACE_EVENT(hmfs_map_blocks,
	TP_PROTO(struct inode *inode, struct f2fs_map_blocks *map, int ret),

	TP_ARGS(inode, map, ret),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(block_t,	m_lblk)
		__field(block_t,	m_pblk)
		__field(unsigned int,	m_len)
		__field(int,	ret)
	),

	TP_fast_assign(
		__entry->dev		= inode->i_sb->s_dev;
		__entry->ino		= inode->i_ino;
		__entry->m_lblk		= map->m_lblk;
		__entry->m_pblk		= map->m_pblk;
		__entry->m_len		= map->m_len;
		__entry->ret		= ret;
	),

	TP_printk("dev = (%d,%d), ino = %lu, file offset = %llu, "
		"start blkaddr = 0x%llx, len = 0x%llx, err = %d",
		show_dev_ino(__entry),
		(unsigned long long)__entry->m_lblk,
		(unsigned long long)__entry->m_pblk,
		(unsigned long long)__entry->m_len,
		__entry->ret)
);

TRACE_EVENT(hmfs_gc_policy,

	TP_PROTO(struct f2fs_sb_info *sbi, unsigned int wait_ms,
			unsigned int free_sections, bool fg_gc),

	TP_ARGS(sbi, wait_ms, free_sections, fg_gc),

	TP_STRUCT__entry(
		__field(unsigned int,	free_sections)
		__field(unsigned int,	wait_ms)
		__field(int,	iolimit)
		__field(bool,	is_greedy)
		__field(bool,   enable_idle)
		__field(bool,   fg_gc)
	),

	TP_fast_assign(
		__entry->free_sections	= free_sections;
		__entry->wait_ms		= wait_ms;
		__entry->iolimit		= sbi->gc_control_info.iolimit;
		__entry->is_greedy		= sbi->gc_control_info.is_greedy;
		__entry->enable_idle	= sbi->gc_control_info.idle_enabled;
		__entry->fg_gc			= fg_gc;
	),

	TP_printk("%s: free sections:%u, wait_ms:%u, iolimit:%d, "
                  "select:%s, enable_io_idle:%d",
		__entry->fg_gc ? "FG_GC" : "BG_GC",
		__entry->free_sections,
		__entry->wait_ms,
		__entry->iolimit,
		__entry->is_greedy ? "greedy" : "cost-benefit",
		__entry->enable_idle)
);

TRACE_EVENT(hmfs_background_gc,

	TP_PROTO(struct super_block *sb, unsigned int wait_ms,
			unsigned int prefree, unsigned int free),

	TP_ARGS(sb, wait_ms, prefree, free),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(unsigned int,	wait_ms)
		__field(unsigned int,	prefree)
		__field(unsigned int,	free)
	),

	TP_fast_assign(
		__entry->dev		= sb->s_dev;
		__entry->wait_ms	= wait_ms;
		__entry->prefree	= prefree;
		__entry->free		= free;
	),

	TP_printk("dev = (%d,%d), wait_ms = %u, prefree = %u, free = %u",
		show_dev(__entry->dev),
		__entry->wait_ms,
		__entry->prefree,
		__entry->free)
);

TRACE_EVENT(hmfs_gc_begin,

	TP_PROTO(struct super_block *sb, bool sync, bool background,
			long long dirty_nodes, long long dirty_dents,
			long long dirty_imeta, unsigned int free_sec,
			unsigned int free_seg, int reserved_seg,
			unsigned int prefree_seg),

	TP_ARGS(sb, sync, background, dirty_nodes, dirty_dents, dirty_imeta,
		free_sec, free_seg, reserved_seg, prefree_seg),

	TP_STRUCT__entry(
		__field(dev_t,		dev)
		__field(bool,		sync)
		__field(bool,		background)
		__field(long long,	dirty_nodes)
		__field(long long,	dirty_dents)
		__field(long long,	dirty_imeta)
		__field(unsigned int,	free_sec)
		__field(unsigned int,	free_seg)
		__field(int,		reserved_seg)
		__field(unsigned int,	prefree_seg)
	),

	TP_fast_assign(
		__entry->dev		= sb->s_dev;
		__entry->sync		= sync;
		__entry->background	= background;
		__entry->dirty_nodes	= dirty_nodes;
		__entry->dirty_dents	= dirty_dents;
		__entry->dirty_imeta	= dirty_imeta;
		__entry->free_sec	= free_sec;
		__entry->free_seg	= free_seg;
		__entry->reserved_seg	= reserved_seg;
		__entry->prefree_seg	= prefree_seg;
	),

	TP_printk("dev = (%d,%d), sync = %d, background = %d, nodes = %lld, "
		"dents = %lld, imeta = %lld, free_sec:%u, free_seg:%u, "
		"rsv_seg:%d, prefree_seg:%u",
		show_dev(__entry->dev),
		__entry->sync,
		__entry->background,
		__entry->dirty_nodes,
		__entry->dirty_dents,
		__entry->dirty_imeta,
		__entry->free_sec,
		__entry->free_seg,
		__entry->reserved_seg,
		__entry->prefree_seg)
);

TRACE_EVENT(hmfs_gc_end,

	TP_PROTO(struct super_block *sb, int ret, int seg_freed,
			int sec_freed, long long dirty_nodes,
			long long dirty_dents, long long dirty_imeta,
			unsigned int free_sec, unsigned int free_seg,
			int reserved_seg, unsigned int prefree_seg),

	TP_ARGS(sb, ret, seg_freed, sec_freed, dirty_nodes, dirty_dents,
		dirty_imeta, free_sec, free_seg, reserved_seg, prefree_seg),

	TP_STRUCT__entry(
		__field(dev_t,		dev)
		__field(int,		ret)
		__field(int,		seg_freed)
		__field(int,		sec_freed)
		__field(long long,	dirty_nodes)
		__field(long long,	dirty_dents)
		__field(long long,	dirty_imeta)
		__field(unsigned int,	free_sec)
		__field(unsigned int,	free_seg)
		__field(int,		reserved_seg)
		__field(unsigned int,	prefree_seg)
	),

	TP_fast_assign(
		__entry->dev		= sb->s_dev;
		__entry->ret		= ret;
		__entry->seg_freed	= seg_freed;
		__entry->sec_freed	= sec_freed;
		__entry->dirty_nodes	= dirty_nodes;
		__entry->dirty_dents	= dirty_dents;
		__entry->dirty_imeta	= dirty_imeta;
		__entry->free_sec	= free_sec;
		__entry->free_seg	= free_seg;
		__entry->reserved_seg	= reserved_seg;
		__entry->prefree_seg	= prefree_seg;
	),

	TP_printk("dev = (%d,%d), ret = %d, seg_freed = %d, sec_freed = %d, "
		"nodes = %lld, dents = %lld, imeta = %lld, free_sec:%u, "
		"free_seg:%u, rsv_seg:%d, prefree_seg:%u",
		show_dev(__entry->dev),
		__entry->ret,
		__entry->seg_freed,
		__entry->sec_freed,
		__entry->dirty_nodes,
		__entry->dirty_dents,
		__entry->dirty_imeta,
		__entry->free_sec,
		__entry->free_seg,
		__entry->reserved_seg,
		__entry->prefree_seg)
);

TRACE_EVENT(hmfs_get_victim,

	TP_PROTO(struct super_block *sb, int type, int gc_type,
			struct victim_sel_policy *p, unsigned int pre_victim,
			unsigned int prefree, unsigned int free),

	TP_ARGS(sb, type, gc_type, p, pre_victim, prefree, free),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(int,	type)
		__field(int,	gc_type)
		__field(int,	alloc_mode)
		__field(int,	gc_mode)
		__field(unsigned int,	victim)
		__field(unsigned int,	cost)
		__field(unsigned int,	ofs_unit)
		__field(unsigned int,	pre_victim)
		__field(unsigned int,	prefree)
		__field(unsigned int,	free)
	),

	TP_fast_assign(
		__entry->dev		= sb->s_dev;
		__entry->type		= type;
		__entry->gc_type	= gc_type;
		__entry->alloc_mode	= p->alloc_mode;
		__entry->gc_mode	= p->gc_mode;
		__entry->victim		= p->min_segno;
		__entry->cost		= p->min_cost;
		__entry->ofs_unit	= p->ofs_unit;
		__entry->pre_victim	= pre_victim;
		__entry->prefree	= prefree;
		__entry->free		= free;
	),

	TP_printk("dev = (%d,%d), type = %s, policy = (%s, %s, %s), "
		"victim = %u, cost = %u, ofs_unit = %u, "
		"pre_victim_secno = %d, prefree = %u, free = %u",
		show_dev(__entry->dev),
		show_data_type(__entry->type),
		show_gc_type(__entry->gc_type),
		show_alloc_mode(__entry->alloc_mode),
		show_victim_policy(__entry->gc_mode),
		__entry->victim,
		__entry->cost,
		__entry->ofs_unit,
		(int)__entry->pre_victim,
		__entry->prefree,
		__entry->free)
);

TRACE_EVENT(hmfs_lookup_start,

	TP_PROTO(struct inode *dir, struct dentry *dentry, unsigned int flags),

	TP_ARGS(dir, dentry, flags),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(const char *,	name)
		__field(unsigned int, flags)
	),

	TP_fast_assign(
		__entry->dev	= dir->i_sb->s_dev;
		__entry->ino	= dir->i_ino;
		__entry->name	= dentry->d_name.name;
		__entry->flags	= flags;
	),

	TP_printk("dev = (%d,%d), pino = %lu, name:%s, flags:%u",
		show_dev_ino(__entry),
		__entry->name,
		__entry->flags)
);

TRACE_EVENT(hmfs_lookup_end,

	TP_PROTO(struct inode *dir, struct dentry *dentry, nid_t ino,
		int err),

	TP_ARGS(dir, dentry, ino, err),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(const char *,	name)
		__field(nid_t,	cino)
		__field(int,	err)
	),

	TP_fast_assign(
		__entry->dev	= dir->i_sb->s_dev;
		__entry->ino	= dir->i_ino;
		__entry->name	= dentry->d_name.name;
		__entry->cino	= ino;
		__entry->err	= err;
	),

	TP_printk("dev = (%d,%d), pino = %lu, name:%s, ino:%u, err:%d",
		show_dev_ino(__entry),
		__entry->name,
		__entry->cino,
		__entry->err)
);

TRACE_EVENT(hmfs_readdir,

	TP_PROTO(struct inode *dir, loff_t start_pos, loff_t end_pos, int err),

	TP_ARGS(dir, start_pos, end_pos, err),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(loff_t,	start)
		__field(loff_t,	end)
		__field(int,	err)
	),

	TP_fast_assign(
		__entry->dev	= dir->i_sb->s_dev;
		__entry->ino	= dir->i_ino;
		__entry->start	= start_pos;
		__entry->end	= end_pos;
		__entry->err	= err;
	),

	TP_printk("dev = (%d,%d), ino = %lu, start_pos:%llu, end_pos:%llu, err:%d",
		show_dev_ino(__entry),
		__entry->start,
		__entry->end,
		__entry->err)
);

TRACE_EVENT(hmfs_fallocate,

	TP_PROTO(struct inode *inode, int mode,
				loff_t offset, loff_t len, int ret),

	TP_ARGS(inode, mode, offset, len, ret),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(int,	mode)
		__field(loff_t,	offset)
		__field(loff_t,	len)
		__field(loff_t, size)
		__field(blkcnt_t, blocks)
		__field(int,	ret)
	),

	TP_fast_assign(
		__entry->dev	= inode->i_sb->s_dev;
		__entry->ino	= inode->i_ino;
		__entry->mode	= mode;
		__entry->offset	= offset;
		__entry->len	= len;
		__entry->size	= inode->i_size;
		__entry->blocks = inode->i_blocks;
		__entry->ret	= ret;
	),

	TP_printk("dev = (%d,%d), ino = %lu, mode = %x, offset = %lld, "
		"len = %lld,  i_size = %lld, i_blocks = %llu, ret = %d",
		show_dev_ino(__entry),
		__entry->mode,
		(unsigned long long)__entry->offset,
		(unsigned long long)__entry->len,
		(unsigned long long)__entry->size,
		(unsigned long long)__entry->blocks,
		__entry->ret)
);

TRACE_EVENT(hmfs_direct_IO_enter,

	TP_PROTO(struct inode *inode, loff_t offset, unsigned long len, int rw),

	TP_ARGS(inode, offset, len, rw),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(loff_t,	pos)
		__field(unsigned long,	len)
		__field(int,	rw)
	),

	TP_fast_assign(
		__entry->dev	= inode->i_sb->s_dev;
		__entry->ino	= inode->i_ino;
		__entry->pos	= offset;
		__entry->len	= len;
		__entry->rw	= rw;
	),

	TP_printk("dev = (%d,%d), ino = %lu pos = %lld len = %lu rw = %d",
		show_dev_ino(__entry),
		__entry->pos,
		__entry->len,
		__entry->rw)
);

TRACE_EVENT(hmfs_direct_IO_exit,

	TP_PROTO(struct inode *inode, loff_t offset, unsigned long len,
		 int rw, int ret),

	TP_ARGS(inode, offset, len, rw, ret),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(loff_t,	pos)
		__field(unsigned long,	len)
		__field(int,	rw)
		__field(int,	ret)
	),

	TP_fast_assign(
		__entry->dev	= inode->i_sb->s_dev;
		__entry->ino	= inode->i_ino;
		__entry->pos	= offset;
		__entry->len	= len;
		__entry->rw	= rw;
		__entry->ret	= ret;
	),

	TP_printk("dev = (%d,%d), ino = %lu pos = %lld len = %lu "
		"rw = %d ret = %d",
		show_dev_ino(__entry),
		__entry->pos,
		__entry->len,
		__entry->rw,
		__entry->ret)
);

TRACE_EVENT(hmfs_reserve_new_blocks,

	TP_PROTO(struct inode *inode, nid_t nid, unsigned int ofs_in_node,
							blkcnt_t count),

	TP_ARGS(inode, nid, ofs_in_node, count),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(nid_t, nid)
		__field(unsigned int, ofs_in_node)
		__field(blkcnt_t, count)
	),

	TP_fast_assign(
		__entry->dev	= inode->i_sb->s_dev;
		__entry->nid	= nid;
		__entry->ofs_in_node = ofs_in_node;
		__entry->count = count;
	),

	TP_printk("dev = (%d,%d), nid = %u, ofs_in_node = %u, count = %llu",
		show_dev(__entry->dev),
		(unsigned int)__entry->nid,
		__entry->ofs_in_node,
		(unsigned long long)__entry->count)
);

DECLARE_EVENT_CLASS(f2fs__submit_page_bio,

	TP_PROTO(struct page *page, struct f2fs_io_info *fio),

	TP_ARGS(page, fio),

	TP_STRUCT__entry(
		__field(dev_t, dev)
		__field(ino_t, ino)
		__field(pgoff_t, index)
		__field(block_t, old_blkaddr)
		__field(block_t, new_blkaddr)
		__field(int, op)
		__field(int, op_flags)
		__field(int, temp)
		__field(int, type)
	),

	TP_fast_assign(
		__entry->dev		= page->mapping->host->i_sb->s_dev;
		__entry->ino		= page->mapping->host->i_ino;
		__entry->index		= page->index;
		__entry->old_blkaddr	= fio->old_blkaddr;
		__entry->new_blkaddr	= fio->new_blkaddr;
		__entry->op		= fio->op;
		__entry->op_flags	= fio->op_flags;
		__entry->temp		= fio->temp;
		__entry->type		= fio->type;
	),

	TP_printk("dev = (%d,%d), ino = %lu, page_index = 0x%lx, "
		"oldaddr = 0x%llx, newaddr = 0x%llx, rw = %s(%s), type = %s_%s",
		show_dev_ino(__entry),
		(unsigned long)__entry->index,
		(unsigned long long)__entry->old_blkaddr,
		(unsigned long long)__entry->new_blkaddr,
		show_bio_type(__entry->op, __entry->op_flags),
		show_block_temp(__entry->temp),
		show_block_type(__entry->type))
);

DEFINE_EVENT_CONDITION(f2fs__submit_page_bio, hmfs_submit_page_bio,

	TP_PROTO(struct page *page, struct f2fs_io_info *fio),

	TP_ARGS(page, fio),

	TP_CONDITION(page->mapping)
);

DEFINE_EVENT_CONDITION(f2fs__submit_page_bio, hmfs_submit_page_write,

	TP_PROTO(struct page *page, struct f2fs_io_info *fio),

	TP_ARGS(page, fio),

	TP_CONDITION(page->mapping)
);

DECLARE_EVENT_CLASS(f2fs__bio,

	TP_PROTO(struct super_block *sb, int type, struct bio *bio),

	TP_ARGS(sb, type, bio),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(dev_t,	target)
		__field(int,	op)
		__field(int,	op_flags)
		__field(int,	type)
		__field(sector_t,	sector)
		__field(unsigned int,	size)
	),

	TP_fast_assign(
		__entry->dev		= sb->s_dev;
		__entry->target		= bio_dev(bio);
		__entry->op		= bio_op(bio);
		__entry->op_flags	= bio->bi_opf;
		__entry->type		= type;
		__entry->sector		= bio->bi_iter.bi_sector;
		__entry->size		= bio->bi_iter.bi_size;
	),

	TP_printk("dev = (%d,%d)/(%d,%d), rw = %s(%s), %s, sector = %lld, size = %u",
		show_dev(__entry->target),
		show_dev(__entry->dev),
		show_bio_type(__entry->op, __entry->op_flags),
		show_block_type(__entry->type),
		(unsigned long long)__entry->sector,
		__entry->size)
);

DEFINE_EVENT_CONDITION(f2fs__bio, hmfs_prepare_write_bio,

	TP_PROTO(struct super_block *sb, int type, struct bio *bio),

	TP_ARGS(sb, type, bio),

	TP_CONDITION(bio)
);

DEFINE_EVENT_CONDITION(f2fs__bio, hmfs_prepare_read_bio,

	TP_PROTO(struct super_block *sb, int type, struct bio *bio),

	TP_ARGS(sb, type, bio),

	TP_CONDITION(bio)
);

DEFINE_EVENT_CONDITION(f2fs__bio, hmfs_submit_read_bio,

	TP_PROTO(struct super_block *sb, int type, struct bio *bio),

	TP_ARGS(sb, type, bio),

	TP_CONDITION(bio)
);

DEFINE_EVENT_CONDITION(f2fs__bio, hmfs_submit_write_bio,

	TP_PROTO(struct super_block *sb, int type, struct bio *bio),

	TP_ARGS(sb, type, bio),

	TP_CONDITION(bio)
);

TRACE_EVENT(hmfs_datamove,

	TP_PROTO(unsigned int verified_blkaddr, unsigned int next_blkaddr,
		int array_size, bool force_flush),

	TP_ARGS(verified_blkaddr, next_blkaddr, array_size, force_flush),

	TP_STRUCT__entry(
		__field(unsigned int, verified_blkaddr)
		__field(unsigned int, next_blkaddr)
		__field(int, array_size)
		__field(bool, force_flush)
	),

	TP_fast_assign(
		__entry->verified_blkaddr = verified_blkaddr;
		__entry->next_blkaddr = next_blkaddr;
		__entry->array_size = array_size;
		__entry->force_flush = force_flush;
	),

	TP_printk("verified_blkaddr = %u, next_available_write = %u, "
			"array_size= %d force_flush (%s)",
		__entry->verified_blkaddr,
		__entry->next_blkaddr,
		__entry->array_size,
		__entry->force_flush ? "Yes" : "No")
);

TRACE_EVENT(hmfs_datamove_align,

	TP_PROTO(int type, unsigned int old_segno, unsigned int old_blkoff,
		unsigned int new_segno, unsigned int new_blkoff),

	TP_ARGS(type, old_segno, old_blkoff, new_segno, new_blkoff),

	TP_STRUCT__entry(
		__field(int, type)
		__field(unsigned int, old_segno)
		__field(unsigned int, old_blkoff)
		__field(unsigned int, new_segno)
		__field(unsigned int, new_blkoff)
	),

	TP_fast_assign(
		__entry->type = type;
		__entry->old_segno = old_segno;
		__entry->old_blkoff = old_blkoff;
		__entry->new_segno = new_segno;
		__entry->new_blkoff = new_blkoff;
	),

	TP_printk("type %d old curseg (%u, %u) => new curseg (%u, %u)",
		__entry->type,
		__entry->old_segno,
		__entry->old_blkoff,
		__entry->new_segno,
		__entry->new_blkoff)
);

DECLARE_EVENT_CLASS(f2fs__bio_list,

	TP_PROTO(struct bio *bio, block_t last_blkaddr, int type),

	TP_ARGS(bio, last_blkaddr, type),

	TP_STRUCT__entry(
		__field(int,	op)
		__field(int,	op_flags)
		__field(int,	type)
		__field(block_t,	last_blkaddr)
		__field(sector_t,	sector)
		__field(unsigned int,	size)
		__field(unsigned int,	stream_id)
	),

	TP_fast_assign(
		__entry->op		= bio_op(bio);
		__entry->op_flags	= bio->bi_opf;
		__entry->type		= type;
		__entry->last_blkaddr   = last_blkaddr;
		__entry->sector		= bio->bi_iter.bi_sector;
		__entry->size		= bio->bi_iter.bi_size;
		__entry->stream_id	= bio->mas_bio.stream_type;
	),

	TP_printk("rw = %s(%s), %s, sbi_last_blkaddr = %lld, "
			"blkaddr = %lld, nr_blk = %u (%u)",
		show_bio_type(__entry->op, __entry->op_flags),
		show_block_type(__entry->type),
		(unsigned long long)__entry->last_blkaddr,
		(unsigned long long)__entry->sector / 8,
		__entry->size / 4096,
		__entry->stream_id)
);

DEFINE_EVENT_CONDITION(f2fs__bio_list, hmfs_bio_list_insert,

	TP_PROTO(struct bio *bio, block_t last_blkaddr, int type),

	TP_ARGS(bio, last_blkaddr, type),

	TP_CONDITION(bio)
);
DEFINE_EVENT_CONDITION(f2fs__bio_list, hmfs_bio_list_pre_submit,

	TP_PROTO(struct bio *bio, block_t last_blkaddr, int type),

	TP_ARGS(bio, last_blkaddr, type),

	TP_CONDITION(bio)
);
DEFINE_EVENT_CONDITION(f2fs__bio_list, hmfs_bio_list_submit_direct,

	TP_PROTO(struct bio *bio, block_t last_blkaddr, int type),

	TP_ARGS(bio, last_blkaddr, type),

	TP_CONDITION(bio)
);
DEFINE_EVENT_CONDITION(f2fs__bio_list, hmfs_bio_list_submit_inlist,

	TP_PROTO(struct bio *bio, block_t last_blkaddr, int type),

	TP_ARGS(bio, last_blkaddr, type),

	TP_CONDITION(bio)
);

TRACE_EVENT(hmfs_datamove_write_cp,

	TP_PROTO(unsigned int free_secs,
		unsigned int count,
		unsigned int nblocks,
		unsigned int verified_blkaddr1,
		unsigned int verified_blkaddr2,
		unsigned int next_blkaddr1,
		unsigned int next_blkaddr2,
		unsigned int cached_last_blkaddr1,
		unsigned int cached_last_blkaddr2),

	TP_ARGS(free_secs, count, nblocks,
		verified_blkaddr1, verified_blkaddr2,
		next_blkaddr1, next_blkaddr2,
		cached_last_blkaddr1, cached_last_blkaddr2),

	TP_STRUCT__entry(
		__field(unsigned int,	free_secs)
		__field(unsigned int,	count)
		__field(unsigned int,	nblocks)
		__field(unsigned int,	verified_blkaddr1)
		__field(unsigned int,	verified_blkaddr2)
		__field(unsigned int,	next_blkaddr1)
		__field(unsigned int,	next_blkaddr2)
		__field(unsigned int,	cached_last_blkaddr1)
		__field(unsigned int,	cached_last_blkaddr2)
	),

	TP_fast_assign(
		__entry->free_secs	= free_secs;
		__entry->count		= count;
		__entry->nblocks	= nblocks;
		__entry->verified_blkaddr1	= verified_blkaddr1;
		__entry->verified_blkaddr2	= verified_blkaddr2;
		__entry->next_blkaddr1	= next_blkaddr1;
		__entry->next_blkaddr2	= next_blkaddr2;
		__entry->cached_last_blkaddr1	= cached_last_blkaddr1;
		__entry->cached_last_blkaddr2	= cached_last_blkaddr2;
	),

	TP_printk("free_sec: %u, count: %u, nblock: %u, "
			"verified_blkaddr1: %u, verified_blkaddr2: %u, "
			"next_blkaddr1: %u, next_blkaddr2: %u, "
			"cached_last_blkaddr1: %u, "
			"cached_last_blkaddr2: %u",
			__entry->free_secs,
			__entry->count,
			__entry->nblocks,
			__entry->verified_blkaddr1,
			__entry->verified_blkaddr2,
			__entry->next_blkaddr1,
			__entry->next_blkaddr2,
			__entry->cached_last_blkaddr1,
			__entry->cached_last_blkaddr2)
);

TRACE_EVENT(hmfs_datamove_submit_discard,

	TP_PROTO(unsigned int start, unsigned int end, block_t startblk,
			unsigned int len),

	TP_ARGS(start, end, startblk, len),

	TP_STRUCT__entry(
		__field(unsigned int,	start)
		__field(unsigned int,	end)
		__field(block_t,	startblk)
		__field(unsigned int,	len)
	),

	TP_fast_assign(
		__entry->start		= start;
		__entry->end		= end;
		__entry->startblk	= startblk;
		__entry->len		= len;
	),

	TP_printk("submit discard start_seg: %u, "
			"end_seg: %u, startblk: %u, len: %u",
		__entry->start,
		__entry->end,
		__entry->startblk,
		__entry->len)
);

TRACE_EVENT(hmfs_write_begin,

	TP_PROTO(struct inode *inode, loff_t pos, unsigned int len,
				unsigned int flags),

	TP_ARGS(inode, pos, len, flags),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(loff_t,	pos)
		__field(unsigned int, len)
		__field(unsigned int, flags)
	),

	TP_fast_assign(
		__entry->dev	= inode->i_sb->s_dev;
		__entry->ino	= inode->i_ino;
		__entry->pos	= pos;
		__entry->len	= len;
		__entry->flags	= flags;
	),

	TP_printk("dev = (%d,%d), ino = %lu, pos = %llu, len = %u, flags = %u",
		show_dev_ino(__entry),
		(unsigned long long)__entry->pos,
		__entry->len,
		__entry->flags)
);

TRACE_EVENT(hmfs_write_end,

	TP_PROTO(struct inode *inode, loff_t pos, unsigned int len,
				unsigned int copied),

	TP_ARGS(inode, pos, len, copied),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(loff_t,	pos)
		__field(unsigned int, len)
		__field(unsigned int, copied)
	),

	TP_fast_assign(
		__entry->dev	= inode->i_sb->s_dev;
		__entry->ino	= inode->i_ino;
		__entry->pos	= pos;
		__entry->len	= len;
		__entry->copied	= copied;
	),

	TP_printk("dev = (%d,%d), ino = %lu, pos = %llu, len = %u, copied = %u",
		show_dev_ino(__entry),
		(unsigned long long)__entry->pos,
		__entry->len,
		__entry->copied)
);

DECLARE_EVENT_CLASS(f2fs__page,

	TP_PROTO(struct page *page, int type),

	TP_ARGS(page, type),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(int, type)
		__field(int, dir)
		__field(pgoff_t, index)
		__field(int, dirty)
		__field(int, uptodate)
	),

	TP_fast_assign(
		__entry->dev	= page->mapping->host->i_sb->s_dev;
		__entry->ino	= page->mapping->host->i_ino;
		__entry->type	= type;
		__entry->dir	= S_ISDIR(page->mapping->host->i_mode);
		__entry->index	= page->index;
		__entry->dirty	= PageDirty(page);
		__entry->uptodate = PageUptodate(page);
	),

	TP_printk("dev = (%d,%d), ino = %lu, %s, %s, index = %lu, "
		"dirty = %d, uptodate = %d",
		show_dev_ino(__entry),
		show_block_type(__entry->type),
		show_file_type(__entry->dir),
		(unsigned long)__entry->index,
		__entry->dirty,
		__entry->uptodate)
);

DEFINE_EVENT(f2fs__page, hmfs_writepage,

	TP_PROTO(struct page *page, int type),

	TP_ARGS(page, type)
);

DEFINE_EVENT(f2fs__page, hmfs_do_write_data_page,

	TP_PROTO(struct page *page, int type),

	TP_ARGS(page, type)
);

DEFINE_EVENT(f2fs__page, hmfs_readpage,

	TP_PROTO(struct page *page, int type),

	TP_ARGS(page, type)
);

DEFINE_EVENT(f2fs__page, hmfs_set_page_dirty,

	TP_PROTO(struct page *page, int type),

	TP_ARGS(page, type)
);

DEFINE_EVENT(f2fs__page, hmfs_vm_page_mkwrite,

	TP_PROTO(struct page *page, int type),

	TP_ARGS(page, type)
);

DEFINE_EVENT(f2fs__page, hmfs_register_inmem_page,

	TP_PROTO(struct page *page, int type),

	TP_ARGS(page, type)
);

DEFINE_EVENT(f2fs__page, hmfs_commit_inmem_page,

	TP_PROTO(struct page *page, int type),

	TP_ARGS(page, type)
);

TRACE_EVENT(hmfs_writepages,

	TP_PROTO(struct inode *inode, struct writeback_control *wbc, int type),

	TP_ARGS(inode, wbc, type),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(int,	type)
		__field(int,	dir)
		__field(long,	nr_to_write)
		__field(long,	pages_skipped)
		__field(loff_t,	range_start)
		__field(loff_t,	range_end)
		__field(pgoff_t, writeback_index)
		__field(int,	sync_mode)
		__field(char,	for_kupdate)
		__field(char,	for_background)
		__field(char,	tagged_writepages)
		__field(char,	for_reclaim)
		__field(char,	range_cyclic)
		__field(char,	for_sync)
		__field(int,	write_hint)
		__field(bool,	is_cold)
	),

	TP_fast_assign(
		__entry->dev		= inode->i_sb->s_dev;
		__entry->ino		= inode->i_ino;
		__entry->type		= type;
		__entry->dir		= S_ISDIR(inode->i_mode);
		__entry->nr_to_write	= wbc->nr_to_write;
		__entry->pages_skipped	= wbc->pages_skipped;
		__entry->range_start	= wbc->range_start;
		__entry->range_end	= wbc->range_end;
		__entry->writeback_index = inode->i_mapping->writeback_index;
		__entry->sync_mode	= wbc->sync_mode;
		__entry->for_kupdate	= wbc->for_kupdate;
		__entry->for_background	= wbc->for_background;
		__entry->tagged_writepages	= wbc->tagged_writepages;
		__entry->for_reclaim	= wbc->for_reclaim;
		__entry->range_cyclic	= wbc->range_cyclic;
		__entry->for_sync	= wbc->for_sync;
		__entry->write_hint	= inode->i_write_hint;
		__entry->is_cold	= file_is_cold(inode);
	),

	TP_printk("dev = (%d,%d), ino = %lu(%d, %s file), %s, %s, nr_to_write %ld, "
		"skipped %ld, start %lld, end %lld, wb_idx %lu, sync_mode %d, "
		"kupdate %u background %u tagged %u reclaim %u cyclic %u sync %u",
		show_dev_ino(__entry),
		__entry->write_hint,
		__entry->is_cold ? "cold" : "hot",
		show_block_type(__entry->type),
		show_file_type(__entry->dir),
		__entry->nr_to_write,
		__entry->pages_skipped,
		__entry->range_start,
		__entry->range_end,
		(unsigned long)__entry->writeback_index,
		__entry->sync_mode,
		__entry->for_kupdate,
		__entry->for_background,
		__entry->tagged_writepages,
		__entry->for_reclaim,
		__entry->range_cyclic,
		__entry->for_sync)
);

TRACE_EVENT(hmfs_readpages,

	TP_PROTO(struct inode *inode, struct page *page, unsigned int nrpage),

	TP_ARGS(inode, page, nrpage),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(pgoff_t,	start)
		__field(unsigned int,	nrpage)
	),

	TP_fast_assign(
		__entry->dev	= inode->i_sb->s_dev;
		__entry->ino	= inode->i_ino;
		__entry->start	= page->index;
		__entry->nrpage	= nrpage;
	),

	TP_printk("dev = (%d,%d), ino = %lu, start = %lu nrpage = %u",
		show_dev_ino(__entry),
		(unsigned long)__entry->start,
		__entry->nrpage)
);

TRACE_EVENT(hmfs_write_checkpoint,

	TP_PROTO(struct super_block *sb, int reason, char *msg),

	TP_ARGS(sb, reason, msg),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(int,	reason)
		__field(char *,	msg)
	),

	TP_fast_assign(
		__entry->dev		= sb->s_dev;
		__entry->reason		= reason;
		__entry->msg		= msg;
	),

	TP_printk("dev = (%d,%d), checkpoint for %s, state = %s",
		show_dev(__entry->dev),
		show_cpreason(__entry->reason),
		__entry->msg)
);

DECLARE_EVENT_CLASS(f2fs_discard,

	TP_PROTO(struct block_device *dev, block_t blkstart, block_t blklen),

	TP_ARGS(dev, blkstart, blklen),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(block_t, blkstart)
		__field(block_t, blklen)
	),

	TP_fast_assign(
		__entry->dev	= dev->bd_dev;
		__entry->blkstart = blkstart;
		__entry->blklen = blklen;
	),

	TP_printk("dev = (%d,%d), blkstart = 0x%llx, blklen = 0x%llx",
		show_dev(__entry->dev),
		(unsigned long long)__entry->blkstart,
		(unsigned long long)__entry->blklen)
);

DEFINE_EVENT(f2fs_discard, hmfs_queue_discard,

	TP_PROTO(struct block_device *dev, block_t blkstart, block_t blklen),

	TP_ARGS(dev, blkstart, blklen)
);

DEFINE_EVENT(f2fs_discard, hmfs_issue_discard,

	TP_PROTO(struct block_device *dev, block_t blkstart, block_t blklen),

	TP_ARGS(dev, blkstart, blklen)
);

DEFINE_EVENT(f2fs_discard, hmfs_remove_discard,

	TP_PROTO(struct block_device *dev, block_t blkstart, block_t blklen),

	TP_ARGS(dev, blkstart, blklen)
);

TRACE_EVENT(hmfs_issue_reset_zone,

	TP_PROTO(struct block_device *dev, block_t blkstart),

	TP_ARGS(dev, blkstart),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(block_t, blkstart)
	),

	TP_fast_assign(
		__entry->dev	= dev->bd_dev;
		__entry->blkstart = blkstart;
	),

	TP_printk("dev = (%d,%d), reset zone at block = 0x%llx",
		show_dev(__entry->dev),
		(unsigned long long)__entry->blkstart)
);

TRACE_EVENT(hmfs_issue_flush,

	TP_PROTO(struct block_device *dev, unsigned int nobarrier,
				unsigned int flush_merge, int ret),

	TP_ARGS(dev, nobarrier, flush_merge, ret),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(unsigned int, nobarrier)
		__field(unsigned int, flush_merge)
		__field(int,  ret)
	),

	TP_fast_assign(
		__entry->dev	= dev->bd_dev;
		__entry->nobarrier = nobarrier;
		__entry->flush_merge = flush_merge;
		__entry->ret = ret;
	),

	TP_printk("dev = (%d,%d), %s %s, ret = %d",
		show_dev(__entry->dev),
		__entry->nobarrier ? "skip (nobarrier)" : "issue",
		__entry->flush_merge ? " with flush_merge" : "",
		__entry->ret)
);

TRACE_EVENT(hmfs_lookup_extent_tree_start,

	TP_PROTO(struct inode *inode, unsigned int pgofs),

	TP_ARGS(inode, pgofs),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(unsigned int, pgofs)
	),

	TP_fast_assign(
		__entry->dev = inode->i_sb->s_dev;
		__entry->ino = inode->i_ino;
		__entry->pgofs = pgofs;
	),

	TP_printk("dev = (%d,%d), ino = %lu, pgofs = %u",
		show_dev_ino(__entry),
		__entry->pgofs)
);

TRACE_EVENT_CONDITION(hmfs_lookup_extent_tree_end,

	TP_PROTO(struct inode *inode, unsigned int pgofs,
						struct extent_info *ei),

	TP_ARGS(inode, pgofs, ei),

	TP_CONDITION(ei),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(unsigned int, pgofs)
		__field(unsigned int, fofs)
		__field(u32, blk)
		__field(unsigned int, len)
	),

	TP_fast_assign(
		__entry->dev = inode->i_sb->s_dev;
		__entry->ino = inode->i_ino;
		__entry->pgofs = pgofs;
		__entry->fofs = ei->fofs;
		__entry->blk = ei->blk;
		__entry->len = ei->len;
	),

	TP_printk("dev = (%d,%d), ino = %lu, pgofs = %u, "
		"ext_info(fofs: %u, blk: %u, len: %u)",
		show_dev_ino(__entry),
		__entry->pgofs,
		__entry->fofs,
		__entry->blk,
		__entry->len)
);

TRACE_EVENT(hmfs_update_extent_tree_range,

	TP_PROTO(struct inode *inode, unsigned int pgofs, block_t blkaddr,
						unsigned int len),

	TP_ARGS(inode, pgofs, blkaddr, len),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(unsigned int, pgofs)
		__field(u32, blk)
		__field(unsigned int, len)
	),

	TP_fast_assign(
		__entry->dev = inode->i_sb->s_dev;
		__entry->ino = inode->i_ino;
		__entry->pgofs = pgofs;
		__entry->blk = blkaddr;
		__entry->len = len;
	),

	TP_printk("dev = (%d,%d), ino = %lu, pgofs = %u, "
					"blkaddr = %u, len = %u",
		show_dev_ino(__entry),
		__entry->pgofs,
		__entry->blk,
		__entry->len)
);

TRACE_EVENT(hmfs_shrink_extent_tree,

	TP_PROTO(struct f2fs_sb_info *sbi, unsigned int node_cnt,
						unsigned int tree_cnt),

	TP_ARGS(sbi, node_cnt, tree_cnt),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(unsigned int, node_cnt)
		__field(unsigned int, tree_cnt)
	),

	TP_fast_assign(
		__entry->dev = sbi->sb->s_dev;
		__entry->node_cnt = node_cnt;
		__entry->tree_cnt = tree_cnt;
	),

	TP_printk("dev = (%d,%d), shrunk: node_cnt = %u, tree_cnt = %u",
		show_dev(__entry->dev),
		__entry->node_cnt,
		__entry->tree_cnt)
);

TRACE_EVENT(hmfs_destroy_extent_tree,

	TP_PROTO(struct inode *inode, unsigned int node_cnt),

	TP_ARGS(inode, node_cnt),

	TP_STRUCT__entry(
		__field(dev_t,	dev)
		__field(ino_t,	ino)
		__field(unsigned int, node_cnt)
	),

	TP_fast_assign(
		__entry->dev = inode->i_sb->s_dev;
		__entry->ino = inode->i_ino;
		__entry->node_cnt = node_cnt;
	),

	TP_printk("dev = (%d,%d), ino = %lu, destroyed: node_cnt = %u",
		show_dev_ino(__entry),
		__entry->node_cnt)
);

DECLARE_EVENT_CLASS(hmfs_sync_dirty_inodes,

	TP_PROTO(struct super_block *sb, int type, s64 count),

	TP_ARGS(sb, type, count),

	TP_STRUCT__entry(
		__field(dev_t, dev)
		__field(int, type)
		__field(s64, count)
	),

	TP_fast_assign(
		__entry->dev	= sb->s_dev;
		__entry->type	= type;
		__entry->count	= count;
	),

	TP_printk("dev = (%d,%d), %s, dirty count = %lld",
		show_dev(__entry->dev),
		show_file_type(__entry->type),
		__entry->count)
);

DEFINE_EVENT(hmfs_sync_dirty_inodes, hmfs_sync_dirty_inodes_enter,

	TP_PROTO(struct super_block *sb, int type, s64 count),

	TP_ARGS(sb, type, count)
);

DEFINE_EVENT(hmfs_sync_dirty_inodes, hmfs_sync_dirty_inodes_exit,

	TP_PROTO(struct super_block *sb, int type, s64 count),

	TP_ARGS(sb, type, count)
);


TRACE_EVENT(hmfs_skip_log_writeback,

	TP_PROTO(unsigned int ino_num),

	TP_ARGS(ino_num),

	TP_STRUCT__entry(
		__field(unsigned int, ino_num)
	),

	TP_fast_assign(
		__entry->ino_num = ino_num;
	),

	TP_printk("f2fs skip log writeback :ino %u", __entry->ino_num)
);

TRACE_EVENT(hmfs_cold_file_should_IPU,

	TP_PROTO(unsigned int ino_num),

	TP_ARGS(ino_num),

	TP_STRUCT__entry(
		__field(unsigned int, ino_num)
	),

	TP_fast_assign(
		__entry->ino_num = ino_num;
	),

	TP_printk("f2fs cold file need ipu :ino %u", __entry->ino_num)
);

#ifdef CONFIG_HMFS_GRADING_SSR
DECLARE_EVENT_CLASS(f2fs_grading_ssr,

	TP_PROTO(unsigned int left, unsigned int free,
					unsigned int seq),

	TP_ARGS(left, free, seq),

	TP_STRUCT__entry(
		__field(unsigned int, left)
		__field(unsigned int, free)
		__field(unsigned int, seq)
	),

	TP_fast_assign(
		__entry->left = left;
		__entry->free = free;
		__entry->seq  = seq;
	),

	TP_printk("ssr :left_space %u free_segments: %u is_seq: %u ",
		__entry->left, __entry->free, __entry->seq)
);

DEFINE_EVENT(f2fs_grading_ssr, f2fs_grading_ssr_allocate,

	TP_PROTO(unsigned int left, unsigned int free,
					unsigned int seq),

	TP_ARGS(left, free, seq)
);
#endif

TRACE_EVENT(hmfs_last_flush_data,

	TP_PROTO(ino_t ino_num, const struct page *last_page),

	TP_ARGS(ino_num, last_page),

	TP_STRUCT__entry(
		__field(ino_t, ino_num)
		__field(const struct page *, last_page)
	),

	TP_fast_assign(
		__entry->ino_num = ino_num;
		__entry->last_page = last_page;
	),

	TP_printk("ino[%u] %s last flush data[%d]",
		__entry->ino_num,
		__entry->last_page ? "has" : "no",
		__entry->last_page ? __entry->last_page->index : 0)
);

TRACE_EVENT(hmfs_ipage_update,

	TP_PROTO(ino_t ino_num, bool is_need),

	TP_ARGS(ino_num, is_need),

	TP_STRUCT__entry(
		__field(ino_t, ino_num)
		__field(bool, is_need)
	),

	TP_fast_assign(
		__entry->ino_num = ino_num;
		__entry->is_need = is_need;
	),

	TP_printk("ino[%u] %s update ipage",
		__entry->ino_num, __entry->is_need ? "need" : "skip")
);

TRACE_EVENT(hmfs_inode_update,

	TP_PROTO(ino_t ino_num, int datasync),

	TP_ARGS(ino_num, datasync),

	TP_STRUCT__entry(
		__field(ino_t, ino_num)
		__field(int, datasync)
	),

	TP_fast_assign(
		__entry->ino_num = ino_num;
		__entry->datasync = datasync;
	),

	TP_printk("ino[%u] need update inode[%d]",
		__entry->ino_num, __entry->datasync)
);

TRACE_EVENT(hmfs_node_sync,

	TP_PROTO(struct inode *inode, int dsync, bool same_ver,
		int dirty_pages, bool dirty_inode, bool need_sync),

	TP_ARGS(inode, dsync, same_ver, dirty_pages, dirty_inode, need_sync),

	TP_STRUCT__entry(
		__field(ino_t, ino_num)
		__field(bool, dsync)
		__field(bool, same_ver)
		__field(bool, has_wb)
		__field(bool, dirty_page)
		__field(bool, dirty_inode)
		__field(bool, need_sync)
	),

	TP_fast_assign(
		__entry->ino_num = inode->i_ino;
		__entry->dsync = (dsync == 1);
		__entry->same_ver = same_ver;
		__entry->has_wb = F2FS_I(inode)->has_wb;
		__entry->dirty_page = (dirty_pages != 0);
		__entry->dirty_inode = dirty_inode;
		__entry->need_sync = need_sync;
	),

	TP_printk("ino[%u] %s sync: dsync[%s], same cp ver[%s], "
			"writeback data[%s], dirty page[%s]/inode[%s]",
			__entry->ino_num, __entry->need_sync ? "need" : "no",
			__entry->dsync ? "y" : "n",
			__entry->same_ver ? "y" : "n",
			__entry->has_wb ? "y" : "n",
			__entry->dirty_page ? "y" : "n",
			__entry->dirty_inode ? "y" : "n")
);

TRACE_EVENT(hmfs_skip_write_inode,

	TP_PROTO(ino_t ino_num),

	TP_ARGS(ino_num),

	TP_STRUCT__entry(
		__field(ino_t, ino_num)
	),

	TP_fast_assign(
		__entry->ino_num = ino_num;
	),

	TP_printk("skip write inode[%u]", __entry->ino_num)
);

TRACE_EVENT(hmfs_sync_inode_xattr,

	TP_PROTO(ino_t ino_num, int nwritten),

	TP_ARGS(ino_num, nwritten),

	TP_STRUCT__entry(
		__field(ino_t, ino_num)
		__field(int, nwritten)
	),

	TP_fast_assign(
		__entry->ino_num = ino_num;
		__entry->nwritten = nwritten;
	),

	TP_printk("sync inode[%u] and/or xattr, written num[%u]",
		__entry->ino_num, __entry->nwritten)
);

TRACE_EVENT(hmfs_sync_file_flush,

	TP_PROTO(ino_t ino_num, bool has_updated),

	TP_ARGS(ino_num, has_updated),

	TP_STRUCT__entry(
		__field(ino_t, ino_num)
		__field(bool, has_updated)
	),

	TP_fast_assign(
		__entry->ino_num = ino_num;
		__entry->has_updated = has_updated;
	),

	TP_printk("sync file ino[%u] %s",
		__entry->ino_num,
		__entry->has_updated ? "need flush" : "skip sync node")
);

TRACE_EVENT(hmfs_sync_file_cp,

	TP_PROTO(struct inode *inode, int cp_reason),

	TP_ARGS(inode, cp_reason),

	TP_STRUCT__entry(
		__field(ino_t,  ino)
		__field(int,    cp_reason)
		),

	TP_fast_assign(
		__entry->ino            = inode->i_ino;
		__entry->cp_reason      = cp_reason;
		),

	TP_printk("sync file ino[%lu] need cp, reason: %s",
		__entry->ino, show_fsync_cpreason(__entry->cp_reason))
);
#endif /* _TRACE_F2FS_H */

 /* This part must be outside protection */
#include <trace/define_trace.h>
