#include "ufstt.h"

#include "asm-generic/bug.h"
#include <asm/unaligned.h>

#include <linux/bitops.h>
#include <linux/blkdev.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/gfp.h>
#include <linux/list.h>
#include <linux/radix-tree.h>
#include <linux/reboot.h>
#include <linux/sched.h>
#include <linux/sizes.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/stddef.h>
#include <linux/types.h>
#include <linux/workqueue.h>

#include "ufshcd.h"
#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
#include "ufs_vcmd_proc.h"
#endif

/* clang-format off */

/* constant definition */
#define UFSTT_LUN_NUM		3

#define SECTOR_SHIFT		9

/* bitmap fixed 16KB */
#define BITMAP_SHIFT		14
#define BITMAP_SIZE		BIT(BITMAP_SHIFT)

#define BLK_SHIFT		12
#define SECTORS_PER_BLK		BIT(BLK_SHIFT - SECTOR_SHIFT)

#define PPN_SHIFT		2 /* 4byte ppn */

#define PPNT_SHIFT		(bitmap_expresser + SECTOR_SHIFT) /* 11/11/12/13 */
#define PPNTS_NODE_SHIFT	3 /* bits of u8 */
#define PPNTS_THRESHOLD		(PPNTS_NODE_SHIFT - 2) /* 1/4 of one node */
#define PPNT_NODE_SHIFT		(PPNT_SHIFT + PPNTS_NODE_SHIFT)
#define PPNT_NODE_SIZE		BIT(PPNT_NODE_SHIFT)

#define NODE_SHIFT		(PPNT_NODE_SHIFT - PPN_SHIFT + BLK_SHIFT)

#define UFSTT_ALLOC_MEM_COUNT 31
/* 1M */
#define UFSTT_MEMSIZE_SHIFT_PER_LOOP 20
#define UFSTT_MEMSIZE_PER_LOOP BIT(UFSTT_MEMSIZE_SHIFT_PER_LOOP)
#define MAX_NODE_COUNT		(UFSTT_ALLOC_MEM_COUNT * BIT(UFSTT_MEMSIZE_SHIFT_PER_LOOP - PPNT_NODE_SHIFT))

#define LBA_TO_NID(__lba)	((__lba) >> (NODE_SHIFT - BLK_SHIFT))
#define NID_TO_LBA(__nid)	((__nid) << (NODE_SHIFT - BLK_SHIFT))
#define LBA_OFF_OF_NID(__lba)	((__lba) & (BIT(NODE_SHIFT - BLK_SHIFT) - 1))

#define NODE_LEAST_ALIVE_TIME (2 * 1000)
#define LOAD_BITMAP_INTEVAL (5  * 1000)
#define NODE_BATCH_UPDATE_THRS 64

#define NID_WEIGHT_THRSH 8
#define NID_WEIGHT_TIME1 0
#define NID_WEIGHT_VAL1 4
#define NID_WEIGHT_TIME2 100
#define NID_WEIGHT_VAL2 4
#define NID_WEIGHT_TIME3 200
#define NID_WEIGHT_VAL3 2
#define NID_WEIGHT_VAL4 1

/* clang-format on */

#define list_to_node(l) container_of(l, struct ufstt_node, entry)

enum node_state {
	NODE_UPDAT, /* in updating */
	NODE_FAIL, /* update error or allocate ppn fail */
	NODE_CLEAN, /* usable node */
};

struct ufstt_node {
	atomic_t read_ref;
#define WAIT_READ_REF_TIMEOUT 1000
	ktime_t last_hit_time;
	wait_queue_head_t waitq;
	struct list_head entry;
	uint32_t *ppn;
	uint16_t nid;
	enum node_state state;
};

static void ufstt_alloc_node(struct ufs_hba *hba, uint16_t nid);
static void ufstt_node_miss(struct ufs_hba *hba, uint16_t nid, uint32_t tag);
static void ufstt_node_init_always(struct ufstt_node *node, uint16_t nid);

/*
 * node management
 *     1. node_tree, search engine, indexed by nid
 *     2. node_lru, swap, reclaim order
 *     3. node_lock, node_tree and node_lru protector
 */
static RADIX_TREE(node_tree, GFP_ATOMIC);
static LIST_HEAD(node_lru);
static spinlock_t node_lock;

static LIST_HEAD(node_update_list);
static atomic_t node_update_cnt;
static struct work_struct node_update_work;

static atomic_t node_counts;
static struct delayed_work ufstt_idle_worker;

/*
 * bitmap management:
 *     1. bitmap, fixed 16KB buffer used for read buffer command
 *     2. bitmap_expresser, l2p size expressed by order of 2 multiplied by 512
 */
static uint8_t *bitmap;
static uint8_t bitmap_expresser;
struct nodes_visit {
	uint8_t weight;
	ktime_t last;
};
static struct nodes_visit *nodes;

static u8 *ppn_buffer_list[UFSTT_ALLOC_MEM_COUNT];
static u32 ppn_buffer_index;
static u32 ppn_buffer_offset;

static u8 *node_mem_list;
static u32 node_mem_index;

/*
 * specified scsi device for lun, used by read buffer to get request queue
 */
static struct scsi_device *ufstt_sdev;
static struct ufs_hba *ufstt_hba;
static u64 ufstt_capacity;

#ifdef CONFIG_HISI_DEBUG_FS
/* debugfs gathering ... */
struct ufstt_debugfs {
	struct dentry *root;
	struct dentry *enable;
	struct dentry *statistics;
	int32_t hits;
	int32_t total;
	int32_t update_work_times;
	int32_t update_cnt;
	u32 batch_mode_met;
};
static struct ufstt_debugfs debugfs;
#endif

static struct ufstt_node *ufstt_node_lookup(uint16_t nid)
{
	return radix_tree_lookup(&node_tree, nid);
}

static int ufstt_node_insert(uint16_t nid, struct ufstt_node *node)
{
	list_add(&node->entry, &node_lru);

	return radix_tree_insert(&node_tree, nid, node);
}

static void *ufstt_node_delete(struct ufstt_node *node)
{
	list_del_init(&node->entry);

	return radix_tree_delete(&node_tree, node->nid);
}

static int ufstt_load_bitmap(struct ufs_hba *hba)
{
	struct ufs_query_vcmd cmd = { 0 };

	cmd.buf_len = BITMAP_SIZE;
	cmd.desc_buf = (u8 *)bitmap;
	cmd.opcode = UPIU_QUERY_OPCODE_VENDOR_READ;
	cmd.idn = TURBO_TABLE_READ_BITMAP;
	cmd.query_func = UPIU_QUERY_FUNC_STANDARD_READ_REQUEST;

	return ufshcd_query_vcmd_retry(hba, &cmd);
}

static bool ufstt_valid_ppn(uint32_t ppn)
{
	/*
	 * 0x0		: invalide ppn
	 * 0xFFFFFFFF	: not write yet
	 * 0xFEFEFEFE	: unmapped lba
	 */
	return !(ppn == 0x0 || ppn == 0xFFFFFFFF || ppn == 0xFEFEFEFE);
}

bool is_ufstt_batch_mode(void)
{
	return ufstt_hba->ufstt_enabled ? ufstt_hba->ufstt_batch_mode : false;
}

#define GET_BYTE(num, n) ((num >> (8 * n)) & 0xFF)
#define INVALID_LBA (-EINVAL)
#define READ_BUFFER_CDB_LEN 10
#define READ_BUFFER_TIMEOUT 5
#define READ_BUFFER_RETRIES 3
static int __ufstt_ppn_load(uint32_t lba, void *buf, uint32_t len)
{
	struct scsi_sense_hdr sshdr;
	u64 flags = REQ_TT_RB;

	const char cmd[READ_BUFFER_CDB_LEN] = { UFSTT_READ_BUFFER,
			       UFSTT_READ_BUFFER_ID,
			       (unsigned char)GET_BYTE(lba, 3),
			       (unsigned char)GET_BYTE(lba, 2),
			       (unsigned char)GET_BYTE(lba, 1),
			       (unsigned char)GET_BYTE(lba, 0),
			       (unsigned char)GET_BYTE(len, 2),
			       (unsigned char)GET_BYTE(len, 1),
			       (unsigned char)GET_BYTE(len, 0),
			       UFSTT_READ_BUFFER_CONTROL };
#ifdef CONFIG_MAS_BLK
	if (ufstt_hba->ufstt_batch_mode)
		flags |= REQ_CP;
#endif
	return scsi_execute_req_with_flags(ufstt_sdev, cmd, DMA_FROM_DEVICE,
					   buf, len, &sshdr,
					   READ_BUFFER_TIMEOUT * HZ,
					   READ_BUFFER_RETRIES, flags, NULL);
}

#define RC16_CDB_LEN 16
#define RC16_LEN 32
#define READ_CAPACITY_RETRIES 5
#define SCSI_CMD_TIMEOUT 30
static int ufstt_read_capacity(void)
{
	struct scsi_sense_hdr sshdr;
	unsigned char cmd[RC16_CDB_LEN];
	void *buffer = NULL;
	int retries = READ_CAPACITY_RETRIES;
	int the_result;

	memset(cmd, 0, RC16_CDB_LEN);
	/*
	 * offset 0: opcode
	 * offset 0: service action
	 * offset 13: rc16 length
	 */
	cmd[0] = SERVICE_ACTION_IN_16;
	cmd[1] = SAI_READ_CAPACITY_16;
	cmd[13] = RC16_LEN;

	buffer = kzalloc(RC16_LEN, GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;
	do {
		the_result = scsi_execute_req(ufstt_sdev, cmd, DMA_FROM_DEVICE,
					      buffer, RC16_LEN, &sshdr,
					      SCSI_CMD_TIMEOUT * HZ,
					      READ_CAPACITY_RETRIES, NULL);
	} while (the_result && retries--);
	if (!the_result)
		ufstt_capacity = get_unaligned_be64(&buffer[0]);
	kfree(buffer);
	return the_result;
}

static bool ufstt_is_mq_prio_tag_used(struct request_queue *q)
{
#ifdef CONFIG_MAS_BLK
	if (get_mq_prio_tag_used(q) > 0)
		return true;
	else
#endif
		return false;
}

static int ufstt_update_ppn_node(struct ufstt_node *node)
{
	unsigned long timeout;

	timeout = msecs_to_jiffies(WAIT_READ_REF_TIMEOUT);
	/* wait for node release by all readers */
	if (!wait_event_timeout(node->waitq, !atomic_read(&node->read_ref), /*lint !e578*/
				timeout)) {
		printk_ratelimited(KERN_ERR "node wait read ref 1s");
		return -ETIMEDOUT;
	}

	return __ufstt_ppn_load(NID_TO_LBA(node->nid), node->ppn,
				PPNT_NODE_SIZE);
}

static void ufstt_ppn_prep(struct ufshcd_lrb *lrbp, uint32_t ppn)
{
	unsigned char *cdb = lrbp->ucd_req_ptr->sc.cdb;

	/*
	 * cdb 0: read16 opcode
	 * cdb 6: ppn
	 * cdb 10: 0
	 * cdb 14: Transfer length
	 * cdb 15:  Control set to 0x1 to distinguish from normal read16
	 */
	cdb[0] = READ_16;
	put_unaligned_be32(ppn, &cdb[6]);
	put_unaligned_be32(0x0, &cdb[10]);
	cdb[14] = 0x01;
	cdb[15] = 0x01;
}

static void __node_rget(struct ufstt_node *node)
{
	atomic_inc(&node->read_ref);
}

static void __node_rput(struct ufstt_node *node)
{
	if (atomic_dec_and_test(&node->read_ref))
		wake_up(&node->waitq);
}

static void schedule_node_update_work(void)
{
	if (!work_busy(&node_update_work))
		queue_work(system_freezable_wq, &node_update_work);
}

static void ufstt_put_to_active_list(struct ufstt_node *node)
{
	list_del_init(&node->entry);
	list_add(&node->entry, &node_update_list);
	atomic_inc(&node_update_cnt);
	schedule_node_update_work();
}

static void ufstt_occupy_tail(struct ufs_hba *hba, uint16_t nid, uint32_t tag)
{
	struct ufstt_node *node = NULL;

	if (list_empty(&node_lru))
		return;
	node = list_last_entry(&node_lru, struct ufstt_node, entry);
	if (ktime_to_ms(ktime_sub(ktime_get(), node->last_hit_time)) <
	    NODE_LEAST_ALIVE_TIME)
		return;
	if (node->state == NODE_UPDAT)
		return;
	ufstt_node_delete(node);
	ufstt_node_init_always(node, nid);
}

static void ufstt_node_miss(struct ufs_hba *hba, uint16_t nid, uint32_t tag)
{
	ktime_t now = ktime_get();
	ktime_t delta = ktime_sub(now, nodes[nid].last);

	if (nodes[nid].last == NID_WEIGHT_TIME1)
		nodes[nid].weight += NID_WEIGHT_VAL1;
	else if (ktime_to_ms(delta) < NID_WEIGHT_TIME2)
		nodes[nid].weight += NID_WEIGHT_VAL2;
	else if (ktime_to_ms(delta) < NID_WEIGHT_TIME3)
		nodes[nid].weight += NID_WEIGHT_VAL3;
	else
		nodes[nid].weight += NID_WEIGHT_VAL4;

	nodes[nid].last = now;

	if (nodes[nid].weight < NID_WEIGHT_THRSH)
		return;

	nodes[nid].weight = 0;
	nodes[nid].last = 0;
	/* reach the top threshold of node count */
	if (atomic_read(&node_counts) < (int)MAX_NODE_COUNT) {
		ufstt_alloc_node(hba, nid);
		return;
	}

	ufstt_occupy_tail(hba, nid, tag);
}

static void ufstt_node_hit(struct ufs_hba *hba, struct ufstt_node *node,
			   struct ufshcd_lrb *lrbp)
{
	uint32_t ppn;
	uint32_t lba = blk_rq_pos(lrbp->cmd->request) / SECTORS_PER_BLK;

	if (node->state == NODE_CLEAN) {
		/* move to lru's head */
		list_del_init(&node->entry);
		list_add(&node->entry, &node_lru);

		ppn = node->ppn[LBA_OFF_OF_NID(lba)];
		if (ufstt_valid_ppn(ppn)) {
#ifdef CONFIG_HISI_DEBUG_FS
			debugfs.hits++;
#endif
			ufstt_ppn_prep(lrbp, ppn);

			if (lrbp->ufstt_private)
				printk_ratelimited(KERN_ERR
						   "ufstt private not null");
			lrbp->ufstt_private = node;
			__node_rget(node);
		}
	}
}

static void ufstt_prep_lrbp(struct ufs_hba *hba, struct ufshcd_lrb *lrbp)
{
	struct ufstt_node *node = NULL;
	unsigned long flags;
	/* request -> sector -> lba -> nid */
	uint32_t lba = blk_rq_pos(lrbp->cmd->request) / SECTORS_PER_BLK;
	uint32_t nid = LBA_TO_NID(lba);

	if (lba + PPNT_NODE_SIZE / BIT(PPN_SHIFT) >= ufstt_capacity)
		return;
	spin_lock_irqsave(&node_lock, flags);
#ifdef CONFIG_HISI_DEBUG_FS
	debugfs.total++;
#endif
	node = ufstt_node_lookup(nid);

	if (!node) {
		ufstt_node_miss(hba, nid, lrbp->task_tag);
	} else {
		ufstt_node_hit(hba, node, lrbp);
		node->last_hit_time = ktime_get();
	}
	spin_unlock_irqrestore(&node_lock, flags);
}

static bool ufstt_allowed(struct ufs_hba *hba, struct ufshcd_lrb *lrbp)
{
	struct request *rq = NULL;

	if (!hba->ufstt_enabled || !lrbp)
		return false;

	rq = lrbp->cmd->request;

	/* turbo table enabled for specified lun: UFSTT_LUN_NUM */
	if (lrbp->lun != UFSTT_LUN_NUM)
		return false;

	/* turbo table only support 4K read, excluding tz2.0 */
	if (!rq
#ifdef CONFIG_MAS_BLK
			|| req_tz(rq)
#endif
			|| blk_rq_sectors(rq) != SECTORS_PER_BLK)
		return false;

	if (lrbp->cmd->cmnd[0] != READ_10 && lrbp->cmd->cmnd[0] != READ_16)
		return false;

	return true;
}

void ufstt_prep_fn(struct ufs_hba *hba, struct ufshcd_lrb *lrbp)
{
	if (!ufstt_allowed(hba, lrbp))
		return;
	ufstt_prep_lrbp(hba, lrbp);
}

void ufstt_unprep_fn(struct ufs_hba *hba, struct ufshcd_lrb *lrbp)
{
	struct ufstt_node *node = lrbp->ufstt_private;
	unsigned long flags;

	if (!hba->ufstt_enabled || !node)
		return;

	spin_lock_irqsave(&node_lock, flags);
	if (node->nid >= BITMAP_SIZE)
		goto out;
	if ((node->state != NODE_UPDAT) &&
	    (hweight8(bitmap[node->nid]) >= BIT(PPNTS_THRESHOLD))) { /*lint !e514*/
		bitmap[node->nid] = 0;
		node->state = NODE_UPDAT;
		ufstt_put_to_active_list(node);
	}
	__node_rput(node);
out:
	spin_unlock_irqrestore(&node_lock, flags);
	lrbp->ufstt_private = NULL;
}

void ufstt_node_update(void)
{
	if (atomic_read(&node_update_cnt))
		schedule_node_update_work();
}

void ufstt_idle_handler(struct ufs_hba *hba, ktime_t now_time)
{
	static ktime_t load_bitmap_last_time;

	if (!hba->ufstt_enabled)
		return;
	ufstt_node_update();

	if (ktime_ms_delta(now_time, load_bitmap_last_time) > LOAD_BITMAP_INTEVAL) {
		queue_delayed_work(system_freezable_wq, &ufstt_idle_worker,
				   msecs_to_jiffies(LOAD_BITMAP_INTEVAL));
		load_bitmap_last_time = now_time;
	}
}

void ufstt_unprep_handler(struct ufs_hba *hba, struct ufshcd_lrb *lrbp,
			  ktime_t now_time)
{
	if (!hba->lrb_in_use)
		ufstt_idle_handler(hba, now_time);
	ufstt_unprep_fn(hba, lrbp);
}

static int ufstt_alloc_ppn(struct ufstt_node *node)
{
	if (ppn_buffer_index >= UFSTT_ALLOC_MEM_COUNT ||
	    ppn_buffer_offset >= UFSTT_MEMSIZE_PER_LOOP)
		return -ENOMEM;
	node->ppn =
		(uint32_t *)(ppn_buffer_list[ppn_buffer_index] + ppn_buffer_offset);

	if (ppn_buffer_offset == UFSTT_MEMSIZE_PER_LOOP - PPNT_NODE_SIZE) {
		ppn_buffer_index++;
		ppn_buffer_offset = 0;
	} else {
		ppn_buffer_offset += PPNT_NODE_SIZE;
	}

	return 0;
}

static void ufstt_node_free(struct ufstt_node *node)
{
	ufstt_node_delete(node);
	atomic_dec(&node_counts);
}

static void ufstt_mem_free(void)
{
	int i;

	kfree(bitmap);
	bitmap = NULL;
	kfree(nodes);
	nodes = NULL;
	for (i = 0; i < UFSTT_ALLOC_MEM_COUNT; i++) {
		kfree(ppn_buffer_list[i]);
		ppn_buffer_list[i] = NULL;
	}
	kfree(node_mem_list);
	node_mem_list = NULL;
}

static bool ufshcd_in_shutdown_or_suspend(void)
{
	if (ufstt_hba->in_shutdown || ufstt_hba->in_suspend)
		return true;
	else
		return false;
}

static inline bool node_batch_mode_met(void)
{
	return atomic_read(&node_update_cnt) >= NODE_BATCH_UPDATE_THRS;
}

static void ufstt_node_update_fn(struct work_struct *work)
{
	int ret;
	struct ufstt_node *node = NULL;
	unsigned long flags;
	bool batch_mode = node_batch_mode_met();

	spin_lock_irqsave(&node_lock, flags);
#ifdef CONFIG_HISI_DEBUG_FS
	debugfs.update_work_times++;
	if (batch_mode)
		debugfs.batch_mode_met++;
#endif

	while (!ufshcd_in_shutdown_or_suspend() &&
	       !list_empty(&node_update_list) &&
	       (!ufstt_is_mq_prio_tag_used(ufstt_sdev->request_queue) ||
		batch_mode)) {
		if (!batch_mode)
			batch_mode = node_batch_mode_met();
		ufstt_hba->ufstt_batch_mode = batch_mode;
		node = list_last_entry(&node_update_list, struct ufstt_node,
				       entry);

		if (!node->ppn) {
			/* lazy allocation of ppn buffer */
			ret = ufstt_alloc_ppn(node);
			if (ret) {
				spin_unlock_irqrestore(&node_lock, flags);
				goto node_state_update;
			}
		}
		spin_unlock_irqrestore(&node_lock, flags);

		ret = ufstt_update_ppn_node(node);
		ufsdbg_error_inject_dispatcher(ufstt_hba, ERR_UFSTT, 0, &ret);
	node_state_update:
		spin_lock_irqsave(&node_lock, flags);
		node->state = ret ? NODE_FAIL : NODE_CLEAN;
		list_del_init(&node->entry);
		list_add(&node->entry, &node_lru);
		atomic_dec(&node_update_cnt);
#ifdef CONFIG_HISI_DEBUG_FS
		if (!ret)
			debugfs.update_cnt++;
#endif
	}
	spin_unlock_irqrestore(&node_lock, flags);
}

static void ufstt_idle_worker_fn(struct work_struct *work)
{
	if (!ufshcd_in_shutdown_or_suspend())
		ufstt_load_bitmap(ufstt_hba);
}

static void ufstt_node_init_always(struct ufstt_node *node, uint16_t nid)
{
	node->nid = nid;
	node->state = NODE_UPDAT;

	ufstt_node_insert(nid, node);
	ufstt_put_to_active_list(node);
}

static void ufstt_node_init_once(struct ufstt_node *node)
{
	atomic_set(&node->read_ref, 0);
	init_waitqueue_head(&node->waitq);
	INIT_LIST_HEAD(&node->entry);

}

static void ufstt_alloc_node(struct ufs_hba *hba, uint16_t nid)
{
	struct ufstt_node *node = NULL;

	if (node_mem_index >= MAX_NODE_COUNT)
		return;
	node = (struct ufstt_node *)node_mem_list + node_mem_index;
	node_mem_index++;
	atomic_inc(&node_counts);
	ufstt_node_init_once(node);
	ufstt_node_init_always(node, nid);
}

static bool ufstt_valid_ltop_weight(uint8_t order)
{
	/*
	 *  128G = 2: 512 * 2 ^ 2 =  2KB,
	 *  256G = 2: 512 * 2 ^ 2 =  2KB,
	 *  512G = 3: 512 * 2 ^ 3 =  4KB,
	 * 1024G = 5: 512 * 2 ^ 5 = 8KB,
	 */
	return order == 2 || order == 3 || order == 4 || order == 5;
}

static int ufstt_work_halt(struct notifier_block *nb, ulong event, void *buf)
{
	ufstt_hba->in_shutdown = true;
	cancel_delayed_work_sync(&ufstt_idle_worker);
	cancel_work_sync(&node_update_work);
	return 0;
}

static struct notifier_block ufstt_notifier = { ufstt_work_halt, NULL, 0 };

#ifdef CONFIG_HISI_DEBUG_FS
static int ufstt_dbg_enable_show(struct seq_file *file, void *data)
{
	struct ufs_hba *hba = (struct ufs_hba *)file->private;

	seq_printf(file, "%s\n", hba->ufstt_enabled ? "true" : "false");
	return 0;
}

static ssize_t ufstt_dbg_enable_store(struct file *filp,
				      const char __user *ubuf, size_t cnt,
				      loff_t *ppos)
{
	bool val = false;
	int ret;
	struct ufs_hba *hba = filp->f_mapping->host->i_private;

	ret = kstrtobool_from_user(ubuf, cnt, &val);
	if (ret) {
		dev_err(hba->dev, "%s: Invalid argument\n", __func__);
		return ret;
	}

	hba->ufstt_enabled = val;
	return cnt;
}

static int ufstt_dbg_enable_open(struct inode *inode, struct file *file)
{
	return single_open(file, ufstt_dbg_enable_show, inode->i_private);
}

static const struct file_operations ufstt_dbg_enable_fops = {
	.open = ufstt_dbg_enable_open,
	.read = seq_read,
	.write = ufstt_dbg_enable_store,
};

static int ufstt_dbg_statistics_show(struct seq_file *file, void *data)
{
	struct ufstt_debugfs *d = (struct ufstt_debugfs *)file->private;

	seq_printf(file, "rand read totals  : 0x%x\n", d->total);
	seq_printf(file, "rand read hits    : 0x%x\n", d->hits);
	seq_printf(file, "io_timeout_cnt    : 0x%x\n",
		   ufstt_hba->io_timeout_cnt);
	seq_printf(file, "batch_mode_met    : 0x%x\n", d->batch_mode_met);
#ifdef CONFIG_MAS_BLK
	seq_printf(file, "prio tag used   : 0x%x\n",
		   get_mq_prio_tag_used(ufstt_sdev->request_queue));
	seq_printf(file, "all tag used    : 0x%x\n",
		   get_mq_all_tag_used(ufstt_sdev->request_queue));
#endif
	seq_printf(file, "current node count: 0x%x\n",
		   atomic_read(&node_counts));
	seq_printf(file, "node cnt in update list: 0x%x\n",
		   atomic_read(&node_update_cnt));
	seq_printf(file, "update work times  : 0x%x\n", d->update_work_times);
	seq_printf(file, "node update succ times : 0x%x\n", d->update_cnt);
	return 0;
}

static int ufstt_dbg_statistics_open(struct inode *inode, struct file *file)
{
	return single_open(file, ufstt_dbg_statistics_show, inode->i_private);
}

static const struct file_operations ufstt_dbg_statistics_fops = {
	.open = ufstt_dbg_statistics_open,
	.read = seq_read,
};

static void ufstt_add_debugfs(struct ufs_hba *hba)
{
	debugfs.root = debugfs_create_dir("ufstt", NULL);
	if (IS_ERR(debugfs.root))
		goto err_no_root;

	debugfs.enable = debugfs_create_file("enable", S_IRUSR, debugfs.root,
					     hba, &ufstt_dbg_enable_fops);
	if (IS_ERR(debugfs.enable))
		goto err_remove_debugfs_file;

	debugfs.statistics =
		debugfs_create_file("statistics", S_IRUSR, debugfs.root,
				    &debugfs, &ufstt_dbg_statistics_fops);
	if (IS_ERR(debugfs.statistics))
		goto err_remove_debugfs_file;

	return;

err_remove_debugfs_file:
	debugfs_remove_recursive(debugfs.root);
err_no_root:
	return;
}

static void ufstt_remove_debugfs(struct ufs_hba *hba)
{
	if (!IS_ERR(debugfs.root))
		debugfs_remove_recursive(debugfs.root);
}
#else
static void ufstt_add_debugfs(struct ufs_hba *hba)
{
}
static void ufstt_remove_debugfs(struct ufs_hba *hba)
{
}
#endif

void ufstt_set_sdev(struct scsi_device *dev)
{
	if (dev->lun == UFSTT_LUN_NUM) {
		/* if scsi device already exist, that's fine. */
		ufstt_sdev = dev;
		ufstt_hba = shost_priv(dev->host);
	}
}

static int ufstt_enable(struct ufs_hba *hba)
{
	bool enabled = false;
	uint8_t desc_buf[QUERY_DESC_TTUNIT_MAX_SIZE] = { 0 };
	struct ufs_query_vcmd cmd = { 0 };

	cmd.buf_len = QUERY_DESC_TTUNIT_MAX_SIZE;
	cmd.desc_buf = desc_buf;
	cmd.opcode = UPIU_QUERY_OPCODE_VENDOR_READ;
	cmd.idn = VENDOR_IDN_TT_UNIT_RD;
	cmd.query_func = UPIU_QUERY_FUNC_STANDARD_READ_REQUEST;

	if (ufshcd_query_vcmd_retry(hba, &cmd))
		return -EIO;

	bitmap_expresser = desc_buf[TTUNIT_DESC_PARAM_L2P_SIZE];
	if (!ufstt_valid_ltop_weight(bitmap_expresser))
		return -EINVAL;

	enabled = !!desc_buf[TTUNIT_DESC_PARAM_TURBO_TABLE_EN];
	if (!enabled) {
		/* set UFSTT_LUN_NUM bit to enable turbo table */
		desc_buf[TTUNIT_DESC_PARAM_TURBO_TABLE_EN] |=
			BIT(UFSTT_LUN_NUM);
		cmd.buf_len = QUERY_DESC_TTUNIT_MAX_SIZE;
		cmd.opcode = UPIU_QUERY_OPCODE_VENDOR_WRITE;
		cmd.idn = VENDOR_IDN_TT_UNIT_WT;
		cmd.has_data = true;
		cmd.query_func = UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST;
		if (ufshcd_query_vcmd_retry(hba, &cmd))
			return -EIO;

		cmd.buf_len = QUERY_DESC_TTUNIT_MAX_SIZE;
		cmd.opcode = UPIU_QUERY_OPCODE_VENDOR_READ;
		cmd.idn = VENDOR_IDN_TT_UNIT_RD;
		cmd.has_data = false;
		cmd.query_func = UPIU_QUERY_FUNC_STANDARD_READ_REQUEST;
		if (ufshcd_query_vcmd_retry(hba, &cmd))
			return -EIO;
		enabled = !!desc_buf[TTUNIT_DESC_PARAM_TURBO_TABLE_EN];
	}

	return enabled == true ? 0 : -EINVAL;
}

static int ufstt_alloc_node_ppn_mem(void)
{
	int i;
	int temp;

	node_mem_list =
		kzalloc(sizeof(struct ufstt_node) * MAX_NODE_COUNT, GFP_KERNEL);
	if (!node_mem_list)
		goto fail;
	for (i = 0; i < UFSTT_ALLOC_MEM_COUNT; i++) {
		ppn_buffer_list[i] = kzalloc(UFSTT_MEMSIZE_PER_LOOP, GFP_KERNEL);
		if (!ppn_buffer_list[i])
			goto free_mem;
	}
	return 0;
free_mem:
	kfree(node_mem_list);
	node_mem_list = NULL;
	for (temp = 0; temp < i; temp++) {
		kfree(ppn_buffer_list[temp]);
		ppn_buffer_list[temp] = NULL;
	}

fail:
	return -ENOMEM;
}

#define INQUIRY_DAT_LEN 36
#define INQUIRY_CDB_LEN 6
#define PRODUCT_REVISION_LVL_OFF 32
#define DEV_REV_LEN 4
static int ufstt_device_get_rev(char *rev, int len)
{
	int ret;
	const int buffer_len = INQUIRY_DAT_LEN + 1;
	char buffer[buffer_len];
	unsigned char cmd[INQUIRY_CDB_LEN] = {INQUIRY, 0, 0, 0,
				(char)buffer_len, 0};

	/* timeout 30s retry 3 times */
	ret = scsi_execute_req(ufstt_sdev, cmd,
				DMA_FROM_DEVICE, buffer,
				buffer_len, NULL,
				msecs_to_jiffies(30000), 3, NULL);
	buffer[INQUIRY_DAT_LEN] = '\0';
	if (ret)
		pr_err("%s: failed with err %d\n",
			__func__, ret);
	else
		snprintf(rev, len, "%s", buffer + PRODUCT_REVISION_LVL_OFF);

	return ret;
}

#define TT_REV_MIN 0022
static bool ufstt_dev_rev_validate(struct ufs_hba *hba)
{
	int rev;
	char dev_rev[DEV_REV_LEN + 1];

	if (ufstt_device_get_rev(dev_rev, DEV_REV_LEN + 1)) {
		return false;
	}

	rev = simple_strtoul(dev_rev, NULL, 10);
	/* fw version >= B022, such as 0322 0222 0122 */
	if (rev % 100 >= TT_REV_MIN)
		return true;
	else
		return false;
}

void ufstt_probe(struct ufs_hba *hba)
{
	/* can not get correct scsi device for UFSTT_LUN_NUM */
	if (!ufstt_sdev || ufstt_sdev->lun != UFSTT_LUN_NUM) {
		pr_err("ufstt sdev or lun fail\n");
		goto fail;
	}
	/* device capability and host capability */
	if (!hba->ufstt_support || !(hba->caps & UFSHCD_CAP_HPB_TURBO_TABLE)) {
		pr_err("ufstt not support or no caps\n");
		goto fail;
	}

	if (!ufstt_dev_rev_validate(hba)) {
		pr_err("ufstt dev rev not meet\n");
		goto fail;
	}
	/* query and set enable it in turbo table unit table */
	if (ufstt_enable(hba)) {
		pr_err("ufstt_enable fail\n");
		goto fail;
	}
	INIT_WORK(&node_update_work, ufstt_node_update_fn);

	if (ufstt_read_capacity()) {
		pr_err("ufstt_read_capacity fail\n");
		goto fail;
	}
	bitmap = kzalloc(BITMAP_SIZE, GFP_KERNEL);
	if (!bitmap) {
		pr_err("alloc bitmap fail\n");
		goto fail;
	}
	nodes = kzalloc(LBA_TO_NID(ufstt_capacity) * sizeof(struct nodes_visit),
			GFP_KERNEL);
	if (!nodes) {
		pr_err("alloc nodes fail\n");
		goto free_bitmap;
	}
	if (ufstt_alloc_node_ppn_mem())
		goto free_nodes;
	register_reboot_notifier(&ufstt_notifier);
	ufstt_add_debugfs(hba);
	spin_lock_init(&node_lock);
	atomic_set(&node_counts, 0);
	atomic_set(&node_update_cnt, 0);
	INIT_DELAYED_WORK(&ufstt_idle_worker, ufstt_idle_worker_fn);

	dev_err(hba->dev, " UFSTT_LUN_NUM    = 0x%x\n", UFSTT_LUN_NUM);
	dev_err(hba->dev, " SECTOR_SHIFT     = 0x%x\n", SECTOR_SHIFT);
	dev_err(hba->dev, " BITMAP_SHIFT     = 0x%x\n", BITMAP_SHIFT);
	dev_err(hba->dev, " BITMAP_SIZE	     = 0x%llx\n", BITMAP_SIZE);
	dev_err(hba->dev, " BLK_SHIFT        = 0x%x\n", BLK_SHIFT);
	dev_err(hba->dev, " SECTORS_PER_BLK  = 0x%llx\n", SECTORS_PER_BLK);
	dev_err(hba->dev, " PPN_SHIFT        = 0x%x\n", PPN_SHIFT);
	dev_err(hba->dev, " PPNT_SHIFT       = 0x%x\n", PPNT_SHIFT);
	dev_err(hba->dev, " PPNTS_NODE_SHIFT = 0x%x\n", PPNTS_NODE_SHIFT);
	dev_err(hba->dev, " PPNTS_THRESHOLD  = 0x%x\n", PPNTS_THRESHOLD);
	dev_err(hba->dev, " PPNT_NODE_SHIFT  = 0x%x\n", PPNT_NODE_SHIFT);
	dev_err(hba->dev, " PPNT_NODE_SIZE   = 0x%llx\n", PPNT_NODE_SIZE);
	dev_err(hba->dev, " NODE_SHIFT       = 0x%x\n", NODE_SHIFT);
	dev_err(hba->dev, " MAX_NODE_COUNT   = 0x%llx\n", MAX_NODE_COUNT);

	hba->ufstt_enabled = true;
	dev_err(hba->dev, "ufstt init succeed\n");
	return;

free_nodes:
	kfree(nodes);
	nodes = NULL;
free_bitmap:
	kfree(bitmap);
	bitmap = NULL;
fail:
	hba->ufstt_enabled = false;
	return;
}

void ufstt_remove(struct ufs_hba *hba)
{
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	spin_lock(&node_lock);
	list_for_each_safe (pos, n, &node_lru)
		ufstt_node_free(list_to_node(pos));
	spin_unlock(&node_lock);

	ufstt_remove_debugfs(hba);
	unregister_reboot_notifier(&ufstt_notifier);
	ufstt_mem_free();

	dev_err(hba->dev, "ufstt removed\n");
}
