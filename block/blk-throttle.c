// SPDX-License-Identifier: GPL-2.0
/*
 * Interface for controlling IO bandwidth on a request queue
 *
 * Copyright (C) 2010 Vivek Goyal <vgoyal@redhat.com>
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/blktrace_api.h>
#include <linux/blk-cgroup.h>
#include <linux/sched/signal.h>
#include "blk.h"

#define SHARE_SHIFT (14)
#define MAX_SHARE (1 << SHARE_SHIFT)

#define MAX_INFLIGHT		(128)
#define MIN_INFLIGHT		(0)

#define QUANTUM_INQUEUE_MIN	(1)
#define QUANTUM_INQUEUE_MAX	(128)
#define QUANTUM_INQUEUE_DEF	(16)

#define BW_SLICE_MIN	(8ULL << 20)	/* 8MB/s */
#define BW_SLICE_MAX	(256ULL << 20)	/* 256MB/s */
#define BW_SLICE_DEF	(32ULL << 20)	/* 32MB/s */

#define IOPS_SLICE_MIN	(32)
#define IOPS_SLICE_MAX	(4096)
#define IOPS_SLICE_DEF	(256)

enum blk_throtl_weight_onoff_mode {
	BLK_THROTL_WEIGHT_OFF,
	BLK_THROTL_WEIGHT_ON_FS,
	BLK_THROTL_WEIGHT_ON_BLK,
};

/* Max dispatch from a group in 1 round */
static int throtl_grp_quantum = 8;

/* Total max dispatch from all groups in one round */
static int throtl_quantum = 32;

/* Throttling is performed over a slice and after that slice is renewed */
#define DFL_THROTL_SLICE_HD (HZ / 10)
#define DFL_THROTL_SLICE_SSD (HZ / 50)
#define MAX_THROTL_SLICE (HZ)
#define MAX_IDLE_TIME (5L * 1000 * 1000) /* 5 s */
#define MIN_THROTL_BPS (320 * 1024)
#define MIN_THROTL_IOPS (10)
#define DFL_LATENCY_TARGET (-1L)
#define DFL_IDLE_THRESHOLD (0)
#define DFL_HD_BASELINE_LATENCY (4000L) /* 4ms */
#define LATENCY_FILTERED_SSD (0)
/*
 * For HD, very small latency comes from sequential IO. Such IO is helpless to
 * help determine if its IO is impacted by others, hence we ignore the IO
 */
#define LATENCY_FILTERED_HD (1000L) /* 1ms */

#define SKIP_LATENCY (((u64)1) << BLK_STAT_RES_SHIFT)

static struct blkcg_policy blkcg_policy_throtl;

/* A workqueue to queue throttle related work */
static struct workqueue_struct *kthrotld_workqueue;

static DEFINE_MUTEX(throtl_mode_lock);

int blk_throtl_weight_offon = BLK_THROTL_WEIGHT_OFF;

/*
 * To implement hierarchical throttling, throtl_grps form a tree and bios
 * are dispatched upwards level by level until they reach the top and get
 * issued.  When dispatching bios from the children and local group at each
 * level, if the bios are dispatched into a single bio_list, there's a risk
 * of a local or child group which can queue many bios at once filling up
 * the list starving others.
 *
 * To avoid such starvation, dispatched bios are queued separately
 * according to where they came from.  When they are again dispatched to
 * the parent, they're popped in round-robin order so that no single source
 * hogs the dispatch window.
 *
 * throtl_qnode is used to keep the queued bios separated by their sources.
 * Bios are queued to throtl_qnode which in turn is queued to
 * throtl_service_queue and then dispatched in round-robin order.
 *
 * It's also used to track the reference counts on blkg's.  A qnode always
 * belongs to a throtl_grp and gets queued on itself or the parent, so
 * incrementing the reference of the associated throtl_grp when a qnode is
 * queued and decrementing when dequeued is enough to keep the whole blkg
 * tree pinned while bios are in flight.
 */
struct throtl_qnode {
	struct list_head	node;		/* service_queue->queued[] */
	struct bio_list		bios;		/* queued bios */
	struct throtl_grp	*tg;		/* tg this qnode belongs to */
};

struct throtl_service_queue {
	struct throtl_service_queue *parent_sq;	/* the parent service_queue */

	/*
	 * Bios queued directly to this service_queue or dispatched from
	 * children throtl_grp's.
	 */
	struct list_head	queued[2];	/* throtl_qnode [READ/WRITE] */
	unsigned int		nr_queued[2];	/* number of queued bios */

	/*
	 * RB tree of active children throtl_grp's, which are sorted by
	 * their ->disptime.
	 */
	struct rb_root		pending_tree;	/* RB tree of active tgs */
	struct rb_node		*first_pending;	/* first node in the tree */
	unsigned int		nr_pending;	/* # queued in the tree */
	unsigned long		first_pending_disptime;	/* disptime of the first tg */
	struct timer_list	pending_timer;	/* fires on first_pending_disptime */

	unsigned int		children_weight; /* children weight */
	unsigned int		share; /* disk bandwidth share of the queue */
};

enum tg_state_flags {
	THROTL_TG_PENDING	= 1 << 0,	/* on parent's pending tree */
	THROTL_TG_WAS_EMPTY	= 1 << 1,	/* bio_lists[] became non-empty */
	THROTL_TG_ONLINE	= 1 << 2,	/* tg online */
};

#define rb_entry_tg(node)	rb_entry((node), struct throtl_grp, rb_node)

enum {
	LIMIT_LOW,
	LIMIT_MAX,
	LIMIT_CNT,
};

struct throtl_grp {
	/* must be the first member */
	struct blkg_policy_data pd;

	/* active throtl group service_queue member */
	struct rb_node rb_node;

	/* throtl_data this group belongs to */
	struct throtl_data *td;

	/* this group's service queue */
	struct throtl_service_queue service_queue;

	/*
	 * qnode_on_self is used when bios are directly queued to this
	 * throtl_grp so that local bios compete fairly with bios
	 * dispatched from children.  qnode_on_parent is used when bios are
	 * dispatched from this throtl_grp into its parent and will compete
	 * with the sibling qnode_on_parents and the parent's
	 * qnode_on_self.
	 */
	struct throtl_qnode qnode_on_self[2];
	struct throtl_qnode qnode_on_parent[2];

	/*
	 * Dispatch time in jiffies. This is the estimated time when group
	 * will unthrottle and is ready to dispatch more bio. It is used as
	 * key to sort active groups in service tree.
	 */
	unsigned long disptime;

	unsigned int flags;

	/* are there any throtl rules between this group and td? */
	bool has_rules[2];

	/* internally used bytes per second rate limits */
	uint64_t bps[2][LIMIT_CNT];
	/* user configured bps limits */
	uint64_t bps_conf[2][LIMIT_CNT];

	/* internally used IOPS limits */
	unsigned int iops[2][LIMIT_CNT];
	/* user configured IOPS limits */
	unsigned int iops_conf[2][LIMIT_CNT];

	/* Number of bytes disptached in current slice */
	uint64_t bytes_disp[2];
	/* Number of bio's dispatched in current slice */
	unsigned int io_disp[2];

	unsigned long last_low_overflow_time[2];

	uint64_t last_bytes_disp[2];
	unsigned int last_io_disp[2];

	unsigned long last_check_time;

	unsigned long latency_target; /* us */
	unsigned long latency_target_conf; /* us */
	/* When did we start a new slice */
	unsigned long slice_start[2];
	unsigned long slice_end[2];

	unsigned long last_finish_time; /* ns / 1024 */
	unsigned long checked_last_finish_time; /* ns / 1024 */
	unsigned long avg_idletime; /* ns / 1024 */
	unsigned long idletime_threshold; /* us */
	unsigned long idletime_threshold_conf; /* us */

	unsigned int bio_cnt; /* total bios */
	unsigned int bad_bio_cnt; /* bios exceeding latency threshold */
	unsigned long bio_cnt_reset_time;

	unsigned long		intime;

	atomic_t		inflights[2];
	struct bio_list		bios;
	struct list_head	node;
	wait_queue_head_t	wait;
	bool			expired;

	int			quantum;
};

enum run_mode {
	MODE_NONE = 0,
	MODE_THROTTLE = 1, /* bandwidth/iops based throttle */
	/* below are weight based */
	MODE_WEIGHT_BANDWIDTH = 2,
	MODE_WEIGHT_IOPS = 3,
	MAX_MODE = 4,
};

static char *run_mode_name[MAX_MODE] = {
	[MODE_NONE] = "none",
	[MODE_THROTTLE] = "throttle",
	[MODE_WEIGHT_BANDWIDTH] = "weight_bw",
	[MODE_WEIGHT_IOPS] = "weight_iops",
};

unsigned int THROTL_IDLE_INTERVAL = 40;  /* ms */

/* We measure latency for request size from <= 4k to >= 1M */
#define LATENCY_BUCKET_SIZE 9

struct latency_bucket {
	unsigned long total_latency; /* ns / 1024 */
	int samples;
};

struct avg_latency_bucket {
	unsigned long latency; /* ns / 1024 */
	bool valid;
};

struct throtl_data
{
	/* service tree for active throtl groups */
	struct throtl_service_queue service_queue;

	struct request_queue *queue;

	/* Total Number of queued bios on READ and WRITE lists */
	unsigned int nr_queued[2];

	unsigned int throtl_slice;

	/* Work for dispatching throttled bios */
	struct work_struct dispatch_work;
	unsigned int limit_index;
	bool limit_valid[LIMIT_CNT];

	unsigned long low_upgrade_time;
	unsigned long low_downgrade_time;

	unsigned int scale;

	struct latency_bucket tmp_buckets[LATENCY_BUCKET_SIZE];
	struct avg_latency_bucket avg_buckets[LATENCY_BUCKET_SIZE];
	struct latency_bucket __percpu *latency_buckets;
	unsigned long last_calculate_time;
	unsigned long filtered_latency;

	bool track_bio_latency;

	uint64_t		bw_slice;
	unsigned int		iops_slice;
	int			quantum;

	enum run_mode		mode;

	int			max_inflights;
	atomic_t		inflights;

	struct list_head	active;
	struct list_head	expired;

	wait_queue_head_t	waitq;
	struct timer_list	rescue_timer;

	struct timer_list	idle_timer;
};

static bool td_weight_based(struct throtl_data *td)
{
	return td->mode > MODE_THROTTLE;
}

static void throtl_pending_timer_fn(unsigned long arg);

static inline struct throtl_grp *pd_to_tg(struct blkg_policy_data *pd)
{
	return pd ? container_of(pd, struct throtl_grp, pd) : NULL;
}

static inline struct throtl_grp *blkg_to_tg(struct blkcg_gq *blkg)
{
	return pd_to_tg(blkg_to_pd(blkg, &blkcg_policy_throtl));
}

static inline struct blkcg_gq *tg_to_blkg(struct throtl_grp *tg)
{
	return pd_to_blkg(&tg->pd);
}

/**
 * sq_to_tg - return the throl_grp the specified service queue belongs to
 * @sq: the throtl_service_queue of interest
 *
 * Return the throtl_grp @sq belongs to.  If @sq is the top-level one
 * embedded in throtl_data, %NULL is returned.
 */
static struct throtl_grp *sq_to_tg(struct throtl_service_queue *sq)
{
	if (sq && sq->parent_sq)
		return container_of(sq, struct throtl_grp, service_queue);
	else
		return NULL;
}

/**
 * sq_to_td - return throtl_data the specified service queue belongs to
 * @sq: the throtl_service_queue of interest
 *
 * A service_queue can be embedded in either a throtl_grp or throtl_data.
 * Determine the associated throtl_data accordingly and return it.
 */
static struct throtl_data *sq_to_td(struct throtl_service_queue *sq)
{
	struct throtl_grp *tg = sq_to_tg(sq);

	if (tg)
		return tg->td;
	else
		return container_of(sq, struct throtl_data, service_queue);
}

static inline int tg_data_index(struct throtl_grp *tg, bool rw)
{
	if (td_weight_based(tg->td))
		return 0;
	return rw;
}

static inline uint64_t queue_bandwidth_slice(struct throtl_data *td)
{
	return td->bw_slice;
}

static inline unsigned int queue_iops_slice(struct throtl_data *td)
{
	return td->iops_slice;
}

/*
 * cgroup's limit in LIMIT_MAX is scaled if low limit is set. This scale is to
 * make the IO dispatch more smooth.
 * Scale up: linearly scale up according to lapsed time since upgrade. For
 *           every throtl_slice, the limit scales up 1/2 .low limit till the
 *           limit hits .max limit
 * Scale down: exponentially scale down if a cgroup doesn't hit its .low limit
 */
static uint64_t throtl_adjusted_limit(uint64_t low, struct throtl_data *td)
{
	/* arbitrary value to avoid too big scale */
	if (td->scale < 4096 && time_after_eq(jiffies,
	    td->low_upgrade_time + td->scale * td->throtl_slice))
		td->scale = (jiffies - td->low_upgrade_time) / td->throtl_slice;

	return low + (low >> 1) * td->scale;
}

static uint64_t tg_bps_limit(struct throtl_grp *tg, int rw)
{
	struct blkcg_gq *blkg = tg_to_blkg(tg);
	struct throtl_data *td;
	uint64_t ret;

	if (cgroup_subsys_on_dfl(io_cgrp_subsys) && !blkg->parent)
		return U64_MAX;

	td = tg->td;
	ret = tg->bps[rw][td->limit_index];
	if (ret == 0 && td->limit_index == LIMIT_LOW) {
		/* intermediate node or iops isn't 0 */
		if (!list_empty(&blkg->blkcg->css.children) ||
		    tg->iops[rw][td->limit_index])
			return U64_MAX;
		else
			return MIN_THROTL_BPS;
	}

	if (td->limit_index == LIMIT_MAX && tg->bps[rw][LIMIT_LOW] &&
	    tg->bps[rw][LIMIT_LOW] != tg->bps[rw][LIMIT_MAX]) {
		uint64_t adjusted;

		adjusted = throtl_adjusted_limit(tg->bps[rw][LIMIT_LOW], td);
		ret = min(tg->bps[rw][LIMIT_MAX], adjusted);
	}
	return ret;
}

static unsigned int tg_iops_limit(struct throtl_grp *tg, int rw)
{
	struct blkcg_gq *blkg = tg_to_blkg(tg);
	struct throtl_data *td;
	unsigned int ret;

	if (cgroup_subsys_on_dfl(io_cgrp_subsys) && !blkg->parent)
		return UINT_MAX;

	td = tg->td;
	ret = tg->iops[rw][td->limit_index];
	if (ret == 0 && tg->td->limit_index == LIMIT_LOW) {
		/* intermediate node or bps isn't 0 */
		if (!list_empty(&blkg->blkcg->css.children) ||
		    tg->bps[rw][td->limit_index])
			return UINT_MAX;
		else
			return MIN_THROTL_IOPS;
	}

	if (td->limit_index == LIMIT_MAX && tg->iops[rw][LIMIT_LOW] &&
	    tg->iops[rw][LIMIT_LOW] != tg->iops[rw][LIMIT_MAX]) {
		uint64_t adjusted;

		adjusted = throtl_adjusted_limit(tg->iops[rw][LIMIT_LOW], td);
		if (adjusted > UINT_MAX)
			adjusted = UINT_MAX;
		ret = min_t(unsigned int, tg->iops[rw][LIMIT_MAX], adjusted);
	}
	return ret;
}

#define request_bucket_index(sectors) \
	clamp_t(int, order_base_2(sectors) - 3, 0, LATENCY_BUCKET_SIZE - 1)

/**
 * throtl_log - log debug message via blktrace
 * @sq: the service_queue being reported
 * @fmt: printf format string
 * @args: printf args
 *
 * The messages are prefixed with "throtl BLKG_NAME" if @sq belongs to a
 * throtl_grp; otherwise, just "throtl".
 */
#define throtl_log(sq, fmt, args...)	do {				\
	struct throtl_grp *__tg = sq_to_tg((sq));			\
	struct throtl_data *__td = sq_to_td((sq));			\
									\
	(void)__td;							\
	if (likely(!blk_trace_note_message_enabled(__td->queue)))	\
		break;							\
	if ((__tg)) {							\
		blk_add_cgroup_trace_msg(__td->queue,			\
			tg_to_blkg(__tg)->blkcg, "throtl " fmt, ##args);\
	} else {							\
		blk_add_trace_msg(__td->queue, "throtl " fmt, ##args);	\
	}								\
} while (0)

static inline unsigned int throtl_bio_data_size(struct bio *bio)
{
	/* assume it's one sector */
	if (unlikely(bio_op(bio) == REQ_OP_DISCARD))
		return 512;
	return bio->bi_iter.bi_size;
}

static void throtl_qnode_init(struct throtl_qnode *qn, struct throtl_grp *tg)
{
	INIT_LIST_HEAD(&qn->node);
	bio_list_init(&qn->bios);
	qn->tg = tg;
}

/**
 * throtl_qnode_add_bio - add a bio to a throtl_qnode and activate it
 * @bio: bio being added
 * @qn: qnode to add bio to
 * @queued: the service_queue->queued[] list @qn belongs to
 *
 * Add @bio to @qn and put @qn on @queued if it's not already on.
 * @qn->tg's reference count is bumped when @qn is activated.  See the
 * comment on top of throtl_qnode definition for details.
 */
static void throtl_qnode_add_bio(struct bio *bio, struct throtl_qnode *qn,
				 struct list_head *queued)
{
	bio_list_add(&qn->bios, bio);
	if (list_empty(&qn->node)) {
		list_add_tail(&qn->node, queued);
		blkg_get(tg_to_blkg(qn->tg));
	}
}

/**
 * throtl_peek_queued - peek the first bio on a qnode list
 * @queued: the qnode list to peek
 */
static struct bio *throtl_peek_queued(struct list_head *queued)
{
	struct throtl_qnode *qn = list_first_entry(queued, struct throtl_qnode, node);
	struct bio *bio;

	if (list_empty(queued))
		return NULL;

	bio = bio_list_peek(&qn->bios);
	WARN_ON_ONCE(!bio);
	return bio;
}

/**
 * throtl_pop_queued - pop the first bio form a qnode list
 * @queued: the qnode list to pop a bio from
 * @tg_to_put: optional out argument for throtl_grp to put
 *
 * Pop the first bio from the qnode list @queued.  After popping, the first
 * qnode is removed from @queued if empty or moved to the end of @queued so
 * that the popping order is round-robin.
 *
 * When the first qnode is removed, its associated throtl_grp should be put
 * too.  If @tg_to_put is NULL, this function automatically puts it;
 * otherwise, *@tg_to_put is set to the throtl_grp to put and the caller is
 * responsible for putting it.
 */
static struct bio *throtl_pop_queued(struct list_head *queued,
				     struct throtl_grp **tg_to_put)
{
	struct throtl_qnode *qn = list_first_entry(queued, struct throtl_qnode, node);
	struct bio *bio;

	if (list_empty(queued))
		return NULL;

	bio = bio_list_pop(&qn->bios);
	WARN_ON_ONCE(!bio);

	if (bio_list_empty(&qn->bios)) {
		list_del_init(&qn->node);
		if (tg_to_put)
			*tg_to_put = qn->tg;
		else
			blkg_put(tg_to_blkg(qn->tg));
	} else {
		list_move_tail(&qn->node, queued);
	}

	return bio;
}

/* init a service_queue, assumes the caller zeroed it */
static void throtl_service_queue_init(struct throtl_service_queue *sq)
{
	INIT_LIST_HEAD(&sq->queued[0]);
	INIT_LIST_HEAD(&sq->queued[1]);
	sq->pending_tree = RB_ROOT;
	setup_timer(&sq->pending_timer, throtl_pending_timer_fn,
		    (unsigned long)sq);
}

static struct blkg_policy_data *throtl_pd_alloc(gfp_t gfp, int node)
{
	struct throtl_grp *tg;
	int rw;

	tg = kzalloc_node(sizeof(*tg), gfp, node);
	if (!tg)
		return NULL;

	throtl_service_queue_init(&tg->service_queue);

	for (rw = READ; rw <= WRITE; rw++) {
		throtl_qnode_init(&tg->qnode_on_self[rw], tg);
		throtl_qnode_init(&tg->qnode_on_parent[rw], tg);
	}

	tg->intime = 0;
	atomic_set(&tg->inflights[0], 0);
	atomic_set(&tg->inflights[1], 0);
	bio_list_init(&tg->bios);
	INIT_LIST_HEAD(&tg->node);
	init_waitqueue_head(&tg->wait);
	tg->expired = false;

	RB_CLEAR_NODE(&tg->rb_node);
	tg->bps[READ][LIMIT_MAX] = U64_MAX;
	tg->bps[WRITE][LIMIT_MAX] = U64_MAX;
	tg->iops[READ][LIMIT_MAX] = UINT_MAX;
	tg->iops[WRITE][LIMIT_MAX] = UINT_MAX;
	tg->bps_conf[READ][LIMIT_MAX] = U64_MAX;
	tg->bps_conf[WRITE][LIMIT_MAX] = U64_MAX;
	tg->iops_conf[READ][LIMIT_MAX] = UINT_MAX;
	tg->iops_conf[WRITE][LIMIT_MAX] = UINT_MAX;
	/* LIMIT_LOW will have default value 0 */

	tg->latency_target = DFL_LATENCY_TARGET;
	tg->latency_target_conf = DFL_LATENCY_TARGET;
	tg->idletime_threshold = DFL_IDLE_THRESHOLD;
	tg->idletime_threshold_conf = DFL_IDLE_THRESHOLD;

	return &tg->pd;
}

static void throtl_pd_init(struct blkg_policy_data *pd)
{
	struct throtl_grp *tg = pd_to_tg(pd);
	struct blkcg_gq *blkg = tg_to_blkg(tg);
	struct throtl_data *td = blkg->q->td;
	struct throtl_service_queue *sq = &tg->service_queue;

	/*
	 * If on the default hierarchy, we switch to properly hierarchical
	 * behavior where limits on a given throtl_grp are applied to the
	 * whole subtree rather than just the group itself.  e.g. If 16M
	 * read_bps limit is set on the root group, the whole system can't
	 * exceed 16M for the device.
	 *
	 * If not on the default hierarchy, the broken flat hierarchy
	 * behavior is retained where all throtl_grps are treated as if
	 * they're all separate root groups right below throtl_data.
	 * Limits of a group don't interact with limits of other groups
	 * regardless of the position of the group in the hierarchy.
	 */
	sq->parent_sq = &td->service_queue;
	if (cgroup_subsys_on_dfl(io_cgrp_subsys) && blkg->parent)
		sq->parent_sq = &blkg_to_tg(blkg->parent)->service_queue;
	tg->td = td;
}

/*
 * Set has_rules[] if @tg or any of its parents have limits configured.
 * This doesn't require walking up to the top of the hierarchy as the
 * parent's has_rules[] is guaranteed to be correct.
 */
static void tg_update_has_rules(struct throtl_grp *tg)
{
	struct throtl_grp *parent_tg = sq_to_tg(tg->service_queue.parent_sq);
	struct throtl_data *td = tg->td;
	int rw;

	for (rw = READ; rw <= WRITE; rw++)
		tg->has_rules[rw] = (parent_tg && parent_tg->has_rules[rw]) ||
			(td->limit_valid[td->limit_index] &&
			 (tg_bps_limit(tg, rw) != U64_MAX ||
			  tg_iops_limit(tg, rw) != UINT_MAX)) ||
			td_weight_based(td);
}

static void tg_update_perf(struct throtl_grp *tg)
{
	struct throtl_service_queue *sq;
	u64 new_bps;
	unsigned int new_iops;

	sq = &tg->service_queue;

	if (!sq->parent_sq)
		return;

	if (tg->td->mode == MODE_WEIGHT_BANDWIDTH) {
		new_bps = max_t(uint64_t,
			(queue_bandwidth_slice(tg->td) * sq->share) >> SHARE_SHIFT,
			1024);
		tg->bps[0][0] = new_bps;
		tg->iops[0][0] = (u32)-1;
	} else if (tg->td->mode == MODE_WEIGHT_IOPS) {
		new_iops = max_t(unsigned int,
			(queue_iops_slice(tg->td) * sq->share) >> SHARE_SHIFT,
			1);
		tg->iops[0][0] = new_iops;
		tg->bps[0][0] = (u64)(s64)-1;
	}
	tg->quantum = max_t(int,
		(throtl_quantum * sq->share) >> SHARE_SHIFT,
		1);
}

/* update share of tg's siblings */
static void tg_update_share(struct throtl_data *td, struct throtl_grp *tg)
{
	struct cgroup_subsys_state *pos_css;
	struct blkcg_gq *blkg, *parent_blkg;
	struct throtl_grp *child;

	if (!tg || !tg->service_queue.parent_sq ||
	    !tg->service_queue.parent_sq->parent_sq)
		parent_blkg = td->queue->root_blkg;
	else
		parent_blkg = tg_to_blkg(sq_to_tg(tg->service_queue.parent_sq));

	blkg_for_each_descendant_pre(blkg, pos_css, parent_blkg) {
		struct throtl_service_queue *sq;

		child = blkg_to_tg(blkg);
		sq = &child->service_queue;

		if (!sq->parent_sq || !(child->flags & THROTL_TG_ONLINE))
			continue;

		sq->share = max_t(unsigned int,
			sq->parent_sq->share * blkg->weight /
				sq->parent_sq->children_weight,
			1);

		tg_update_perf(child);
	}
}

static void throtl_pd_online(struct blkg_policy_data *pd)
{
	struct throtl_grp *tg = pd_to_tg(pd);
	struct throtl_service_queue *parent_sq;
	struct blkcg_gq *blkg = tg_to_blkg(tg);

	/*
	 * We don't want new groups to escape the limits of its ancestors.
	 * Update has_rules[] after a new group is brought online.
	 */
	tg_update_has_rules(tg);

	tg->flags |= THROTL_TG_ONLINE;

	parent_sq = tg->service_queue.parent_sq;
	if (parent_sq)
		parent_sq->children_weight += blkg->weight;

	tg_update_share(tg->td, tg);
}

static void blk_throtl_update_limit_valid(struct throtl_data *td)
{
	struct cgroup_subsys_state *pos_css;
	struct blkcg_gq *blkg;
	bool low_valid = false;

	rcu_read_lock();
	blkg_for_each_descendant_post(blkg, pos_css, td->queue->root_blkg) {
		struct throtl_grp *tg = blkg_to_tg(blkg);

		if (tg->bps[READ][LIMIT_LOW] || tg->bps[WRITE][LIMIT_LOW] ||
		    tg->iops[READ][LIMIT_LOW] || tg->iops[WRITE][LIMIT_LOW])
			low_valid = true;
	}
	rcu_read_unlock();

	td->limit_valid[LIMIT_LOW] = low_valid;
}

static void throtl_upgrade_state(struct throtl_data *td);
static void throtl_pd_offline(struct blkg_policy_data *pd)
{
	struct throtl_grp *tg = pd_to_tg(pd);
	struct throtl_service_queue *sq = &tg->service_queue;
	struct blkcg_gq *blkg = tg_to_blkg(tg);

	tg->bps[READ][LIMIT_LOW] = 0;
	tg->bps[WRITE][LIMIT_LOW] = 0;
	tg->iops[READ][LIMIT_LOW] = 0;
	tg->iops[WRITE][LIMIT_LOW] = 0;

	blk_throtl_update_limit_valid(tg->td);

	if (!tg->td->limit_valid[tg->td->limit_index])
		throtl_upgrade_state(tg->td);

	if (!(tg->flags & THROTL_TG_ONLINE))
		return;

	if (bio_list_empty(&tg->bios) && !list_empty(&tg->node)) {
		list_del_init(&tg->node);
		blkg_put(tg_to_blkg(tg));
	}

	tg->flags &= ~THROTL_TG_ONLINE;

	if (sq->parent_sq)
		sq->parent_sq->children_weight -= blkg->weight;

	rcu_read_lock();
	tg_update_share(tg->td, tg);
	rcu_read_unlock();
}

static void throtl_pd_free(struct blkg_policy_data *pd)
{
	struct throtl_grp *tg = pd_to_tg(pd);

	del_timer_sync(&tg->service_queue.pending_timer);
	kfree(tg);
}

static struct throtl_grp *
throtl_rb_first(struct throtl_service_queue *parent_sq)
{
	/* Service tree is empty */
	if (!parent_sq->nr_pending)
		return NULL;

	if (!parent_sq->first_pending)
		parent_sq->first_pending = rb_first(&parent_sq->pending_tree);

	if (parent_sq->first_pending)
		return rb_entry_tg(parent_sq->first_pending);

	return NULL;
}

static void rb_erase_init(struct rb_node *n, struct rb_root *root)
{
	rb_erase(n, root);
	RB_CLEAR_NODE(n);
}

static void throtl_rb_erase(struct rb_node *n,
			    struct throtl_service_queue *parent_sq)
{
	if (parent_sq->first_pending == n)
		parent_sq->first_pending = NULL;
	rb_erase_init(n, &parent_sq->pending_tree);
	--parent_sq->nr_pending;
}

static void update_min_dispatch_time(struct throtl_service_queue *parent_sq)
{
	struct throtl_grp *tg;

	tg = throtl_rb_first(parent_sq);
	if (!tg)
		return;

	parent_sq->first_pending_disptime = tg->disptime;
}

static void tg_service_queue_add(struct throtl_grp *tg)
{
	struct throtl_service_queue *parent_sq = tg->service_queue.parent_sq;
	struct rb_node **node = &parent_sq->pending_tree.rb_node;
	struct rb_node *parent = NULL;
	struct throtl_grp *__tg;
	unsigned long key = tg->disptime;
	int left = 1;

	while (*node != NULL) {
		parent = *node;
		__tg = rb_entry_tg(parent);

		if (time_before(key, __tg->disptime))
			node = &parent->rb_left;
		else {
			node = &parent->rb_right;
			left = 0;
		}
	}

	if (left)
		parent_sq->first_pending = &tg->rb_node;

	rb_link_node(&tg->rb_node, parent, node);
	rb_insert_color(&tg->rb_node, &parent_sq->pending_tree);
}

static void __throtl_enqueue_tg(struct throtl_grp *tg)
{
	tg_service_queue_add(tg);
	tg->flags |= THROTL_TG_PENDING;
	tg->service_queue.parent_sq->nr_pending++;
}

static void throtl_enqueue_tg(struct throtl_grp *tg)
{
	if (!(tg->flags & THROTL_TG_PENDING))
		__throtl_enqueue_tg(tg);
}

static void __throtl_dequeue_tg(struct throtl_grp *tg)
{
	throtl_rb_erase(&tg->rb_node, tg->service_queue.parent_sq);
	tg->flags &= ~THROTL_TG_PENDING;
}

static void throtl_dequeue_tg(struct throtl_grp *tg)
{
	if (tg->flags & THROTL_TG_PENDING)
		__throtl_dequeue_tg(tg);
}

/* Call with queue lock held */
static void throtl_schedule_pending_timer(struct throtl_service_queue *sq,
					  unsigned long expires)
{
	unsigned long max_expire = jiffies + 8 * sq_to_td(sq)->throtl_slice;

	/*
	 * Since we are adjusting the throttle limit dynamically, the sleep
	 * time calculated according to previous limit might be invalid. It's
	 * possible the cgroup sleep time is very long and no other cgroups
	 * have IO running so notify the limit changes. Make sure the cgroup
	 * doesn't sleep too long to avoid the missed notification.
	 */
	if (time_after(expires, max_expire))
		expires = max_expire;
	mod_timer(&sq->pending_timer, expires);
	throtl_log(sq, "schedule timer. delay=%lu jiffies=%lu",
		   expires - jiffies, jiffies);
}

/**
 * throtl_schedule_next_dispatch - schedule the next dispatch cycle
 * @sq: the service_queue to schedule dispatch for
 * @force: force scheduling
 *
 * Arm @sq->pending_timer so that the next dispatch cycle starts on the
 * dispatch time of the first pending child.  Returns %true if either timer
 * is armed or there's no pending child left.  %false if the current
 * dispatch window is still open and the caller should continue
 * dispatching.
 *
 * If @force is %true, the dispatch timer is always scheduled and this
 * function is guaranteed to return %true.  This is to be used when the
 * caller can't dispatch itself and needs to invoke pending_timer
 * unconditionally.  Note that forced scheduling is likely to induce short
 * delay before dispatch starts even if @sq->first_pending_disptime is not
 * in the future and thus shouldn't be used in hot paths.
 */
static bool throtl_schedule_next_dispatch(struct throtl_service_queue *sq,
					  bool force)
{
	/* any pending children left? */
	if (!sq->nr_pending)
		return true;

	update_min_dispatch_time(sq);

	/* is the next dispatch time in the future? */
	if (force || time_after(sq->first_pending_disptime, jiffies)) {
		throtl_schedule_pending_timer(sq, sq->first_pending_disptime);
		return true;
	}

	/* tell the caller to continue dispatching */
	return false;
}

static inline void throtl_start_new_slice_with_credit(struct throtl_grp *tg,
		bool rw, unsigned long start)
{
	int index = tg_data_index(tg, rw);
	tg->bytes_disp[index] = 0;
	tg->io_disp[index] = 0;

	/*
	 * Previous slice has expired. We must have trimmed it after last
	 * bio dispatch. That means since start of last slice, we never used
	 * that bandwidth. Do try to make use of that bandwidth while giving
	 * credit.
	 */
	if (time_after_eq(start, tg->slice_start[index]))
		tg->slice_start[index] = start;

	tg->slice_end[index] = jiffies + tg->td->throtl_slice;
	throtl_log(&tg->service_queue,
		   "[%c] new slice with credit start=%lu end=%lu jiffies=%lu",
		   rw == READ ? 'R' : 'W', tg->slice_start[index],
		   tg->slice_end[index], jiffies);
}

static inline void throtl_start_new_slice(struct throtl_grp *tg, bool rw)
{
	int index = tg_data_index(tg, rw);

	tg->bytes_disp[index] = 0;
	tg->io_disp[index] = 0;
	tg->slice_start[index] = jiffies;
	tg->slice_end[index] = jiffies + tg->td->throtl_slice;
	throtl_log(&tg->service_queue,
		   "[%c] new slice start=%lu end=%lu jiffies=%lu",
		   rw == READ ? 'R' : 'W', tg->slice_start[index],
		   tg->slice_end[index], jiffies);
}

static inline void throtl_set_slice_end(struct throtl_grp *tg, bool rw,
					unsigned long jiffy_end)
{
	int index = tg_data_index(tg, rw);
	tg->slice_end[index] = roundup(jiffy_end, tg->td->throtl_slice);
}

static inline void throtl_extend_slice(struct throtl_grp *tg, bool rw,
				       unsigned long jiffy_end)
{
	int index = tg_data_index(tg, rw);
	tg->slice_end[index] = roundup(jiffy_end, tg->td->throtl_slice);
	throtl_log(&tg->service_queue,
		   "[%c] extend slice start=%lu end=%lu jiffies=%lu",
		   rw == READ ? 'R' : 'W', tg->slice_start[index],
		   tg->slice_end[rw], jiffies);
}

/* Determine if previously allocated or extended slice is complete or not */
static bool throtl_slice_used(struct throtl_grp *tg, bool rw)
{
	int index = tg_data_index(tg, rw);
	if (time_in_range(jiffies, tg->slice_start[index], tg->slice_end[index]))
		return false;

	return 1;
}

/* Trim the used slices and adjust slice start accordingly */
static inline void throtl_trim_slice(struct throtl_grp *tg, bool rw)
{
	unsigned long nr_slices, time_elapsed, io_trim;
	u64 bytes_trim, tmp;
	int index = tg_data_index(tg, rw);

	BUG_ON(time_before(tg->slice_end[index], tg->slice_start[index]));

	/*
	 * If bps are unlimited (-1), then time slice don't get
	 * renewed. Don't try to trim the slice if slice is used. A new
	 * slice will start when appropriate.
	 */
	if (throtl_slice_used(tg, rw))
		return;

	/*
	 * A bio has been dispatched. Also adjust slice_end. It might happen
	 * that initially cgroup limit was very low resulting in high
	 * slice_end, but later limit was bumped up and bio was dispached
	 * sooner, then we need to reduce slice_end. A high bogus slice_end
	 * is bad because it does not allow new slice to start.
	 */

	throtl_set_slice_end(tg, rw, jiffies + tg->td->throtl_slice);

	time_elapsed = jiffies - tg->slice_start[index];

	nr_slices = time_elapsed / tg->td->throtl_slice;

	if (!nr_slices)
		return;
	tmp = tg_bps_limit(tg, index) * tg->td->throtl_slice * nr_slices;
	do_div(tmp, HZ);
	bytes_trim = tmp;

	io_trim = (tg_iops_limit(tg, index) * tg->td->throtl_slice * nr_slices) /
		HZ;

	if (!bytes_trim && !io_trim)
		return;

	if (tg->bytes_disp[index] >= bytes_trim)
		tg->bytes_disp[index] -= bytes_trim;
	else
		tg->bytes_disp[index] = 0;

	if (tg->io_disp[index] >= io_trim)
		tg->io_disp[index] -= io_trim;
	else
		tg->io_disp[index] = 0;

	tg->slice_start[index] += nr_slices * tg->td->throtl_slice;

	throtl_log(&tg->service_queue,
		   "[%c] trim slice nr=%lu bytes=%llu io=%lu start=%lu end=%lu jiffies=%lu",
		   rw == READ ? 'R' : 'W', nr_slices, bytes_trim, io_trim,
		   tg->slice_start[index], tg->slice_end[index], jiffies);
}

static bool tg_with_in_iops_limit(struct throtl_grp *tg, struct bio *bio,
				  unsigned long *wait)
{
	bool rw = bio_data_dir(bio);
	int index = tg_data_index(tg, rw);
	unsigned int io_allowed;
	unsigned long jiffy_elapsed, jiffy_wait, jiffy_elapsed_rnd;
	u64 tmp;

	jiffy_elapsed = jiffy_elapsed_rnd = jiffies - tg->slice_start[index];

	/* Slice has just started. Consider one slice interval */
	if (!jiffy_elapsed)
		jiffy_elapsed_rnd = tg->td->throtl_slice;

	jiffy_elapsed_rnd = roundup(jiffy_elapsed_rnd, tg->td->throtl_slice);

	/*
	 * jiffy_elapsed_rnd should not be a big value as minimum iops can be
	 * 1 then at max jiffy elapsed should be equivalent of 1 second as we
	 * will allow dispatch after 1 second and after that slice should
	 * have been trimmed.
	 */

	tmp = (u64)tg_iops_limit(tg, index) * jiffy_elapsed_rnd;
	do_div(tmp, HZ);

	if (tmp > UINT_MAX)
		io_allowed = UINT_MAX;
	else
		io_allowed = tmp;

	if (tg->io_disp[index] + 1 <= io_allowed) {
		if (wait)
			*wait = 0;
		return true;
	}

	/* Calc approx time to dispatch */
	jiffy_wait = ((tg->io_disp[index] + 1) * HZ) / tg_iops_limit(tg, index) + 1;

	if (jiffy_wait > jiffy_elapsed)
		jiffy_wait = jiffy_wait - jiffy_elapsed;
	else
		jiffy_wait = 1;

	if (wait)
		*wait = jiffy_wait;
	return 0;
}

static bool tg_with_in_bps_limit(struct throtl_grp *tg, struct bio *bio,
				 unsigned long *wait)
{
	bool rw = bio_data_dir(bio);
	int index = tg_data_index(tg, rw);
	u64 bytes_allowed, extra_bytes, tmp;
	unsigned long jiffy_elapsed, jiffy_wait, jiffy_elapsed_rnd;
	unsigned int bio_size = throtl_bio_data_size(bio);

	jiffy_elapsed = jiffy_elapsed_rnd = jiffies - tg->slice_start[index];

	/* Slice has just started. Consider one slice interval */
	if (!jiffy_elapsed)
		jiffy_elapsed_rnd = tg->td->throtl_slice;

	jiffy_elapsed_rnd = roundup(jiffy_elapsed_rnd, tg->td->throtl_slice);

	tmp = tg_bps_limit(tg, index) * jiffy_elapsed_rnd;
	do_div(tmp, HZ);
	bytes_allowed = tmp;

	if (tg->bytes_disp[index] + bio_size <= bytes_allowed) {
		if (wait)
			*wait = 0;
		return true;
	}

	/* Calc approx time to dispatch */
	extra_bytes = tg->bytes_disp[index] + bio_size - bytes_allowed;
	jiffy_wait = div64_u64(extra_bytes * HZ, tg_bps_limit(tg, index));

	if (!jiffy_wait)
		jiffy_wait = 1;

	/*
	 * This wait time is without taking into consideration the rounding
	 * up we did. Add that time also.
	 */
	jiffy_wait = jiffy_wait + (jiffy_elapsed_rnd - jiffy_elapsed);
	if (wait)
		*wait = jiffy_wait;
	return false;
}

/*
 * Returns whether one can dispatch a bio or not. Also returns approx number
 * of jiffies to wait before this bio is with-in IO rate and can be dispatched
 */
static bool tg_may_dispatch(struct throtl_grp *tg, struct bio *bio,
			    unsigned long *wait)
{
	bool rw = bio_data_dir(bio);
	int index = tg_data_index(tg, rw);
	unsigned long bps_wait = 0, iops_wait = 0, max_wait = 0;

	/*
 	 * Currently whole state machine of group depends on first bio
	 * queued in the group bio list. So one should not be calling
	 * this function with a different bio if there are other bios
	 * queued.
	 */
	BUG_ON(tg->service_queue.nr_queued[index] &&
	       bio != throtl_peek_queued(&tg->service_queue.queued[index]));

	/* If tg->bps = -1, then BW is unlimited */
	if (tg_bps_limit(tg, index) == U64_MAX &&
	    tg_iops_limit(tg, index) == UINT_MAX) {
		if (wait)
			*wait = 0;
		return true;
	}

	/*
	 * If previous slice expired, start a new one otherwise renew/extend
	 * existing slice to make sure it is at least throtl_slice interval
	 * long since now. New slice is started only for empty throttle group.
	 * If there is queued bio, that means there should be an active
	 * slice and it should be extended instead.
	 */
	if (throtl_slice_used(tg, rw) && !(tg->service_queue.nr_queued[rw]))
		throtl_start_new_slice(tg, rw);
	else {
		if (time_before(tg->slice_end[index],
		    jiffies + tg->td->throtl_slice))
			throtl_extend_slice(tg, rw,
				jiffies + tg->td->throtl_slice);
	}

	if (tg_with_in_bps_limit(tg, bio, &bps_wait) &&
	    tg_with_in_iops_limit(tg, bio, &iops_wait)) {
		if (wait)
			*wait = 0;
		return 1;
	}

	max_wait = max(bps_wait, iops_wait);

	if (wait)
		*wait = max_wait;

	if (time_before(tg->slice_end[index], jiffies + max_wait))
		throtl_extend_slice(tg, rw, jiffies + max_wait);

	return 0;
}

static void throtl_charge_bio(struct throtl_grp *tg, struct bio *bio)
{
	bool rw = bio_data_dir(bio);
	int index = tg_data_index(tg, rw);
	unsigned int bio_size = throtl_bio_data_size(bio);

	/* Charge the bio to the group */
	tg->bytes_disp[index] += bio_size;
	tg->io_disp[index]++;
	tg->last_bytes_disp[index] += bio_size;
	tg->last_io_disp[index]++;

	/*
	 * BIO_THROTTLED is used to prevent the same bio to be throttled
	 * more than once as a throttled bio will go through blk-throtl the
	 * second time when it eventually gets issued.  Set it when a bio
	 * is being charged to a tg.
	 */
	if (!bio_flagged(bio, BIO_THROTTLED))
		bio_set_flag(bio, BIO_THROTTLED);
}

/**
 * throtl_add_bio_tg - add a bio to the specified throtl_grp
 * @bio: bio to add
 * @qn: qnode to use
 * @tg: the target throtl_grp
 *
 * Add @bio to @tg's service_queue using @qn.  If @qn is not specified,
 * tg->qnode_on_self[] is used.
 */
static void throtl_add_bio_tg(struct bio *bio, struct throtl_qnode *qn,
			      struct throtl_grp *tg)
{
	struct throtl_service_queue *sq = &tg->service_queue;
	bool rw = bio_data_dir(bio);
	int index = tg_data_index(tg, rw);

	if (!qn)
		qn = &tg->qnode_on_self[index];

	/*
	 * If @tg doesn't currently have any bios queued in the same
	 * direction, queueing @bio can change when @tg should be
	 * dispatched.  Mark that @tg was empty.  This is automatically
	 * cleaered on the next tg_update_disptime().
	 */
	if (!sq->nr_queued[index])
		tg->flags |= THROTL_TG_WAS_EMPTY;

	throtl_qnode_add_bio(bio, qn, &sq->queued[index]);

	sq->nr_queued[index]++;
	throtl_enqueue_tg(tg);
}

static void tg_update_disptime(struct throtl_grp *tg)
{
	struct throtl_service_queue *sq = &tg->service_queue;
	unsigned long read_wait = -1, write_wait = -1, min_wait = -1, disptime;
	struct bio *bio;

	bio = throtl_peek_queued(&sq->queued[READ]);
	if (bio)
		tg_may_dispatch(tg, bio, &read_wait);

	bio = throtl_peek_queued(&sq->queued[WRITE]);
	if (bio)
		tg_may_dispatch(tg, bio, &write_wait);

	min_wait = min(read_wait, write_wait);
	disptime = jiffies + min_wait;

	/* Update dispatch time */
	throtl_dequeue_tg(tg);
	tg->disptime = disptime;
	throtl_enqueue_tg(tg);

	/* see throtl_add_bio_tg() */
	tg->flags &= ~THROTL_TG_WAS_EMPTY;
}

static void start_parent_slice_with_credit(struct throtl_grp *child_tg,
					struct throtl_grp *parent_tg, bool rw)
{
	int index = tg_data_index(child_tg, rw);
	if (throtl_slice_used(parent_tg, rw)) {
		throtl_start_new_slice_with_credit(parent_tg, rw,
				child_tg->slice_start[index]);
	}

}

static void tg_dispatch_one_bio(struct throtl_grp *tg, bool rw)
{
	struct throtl_service_queue *sq = &tg->service_queue;
	struct throtl_service_queue *parent_sq = sq->parent_sq;
	struct throtl_grp *parent_tg = sq_to_tg(parent_sq);
	struct throtl_grp *tg_to_put = NULL;
	struct bio *bio;
	int index = tg_data_index(tg, rw);

	/*
	 * @bio is being transferred from @tg to @parent_sq.  Popping a bio
	 * from @tg may put its reference and @parent_sq might end up
	 * getting released prematurely.  Remember the tg to put and put it
	 * after @bio is transferred to @parent_sq.
	 */
	bio = throtl_pop_queued(&sq->queued[index], &tg_to_put);
	sq->nr_queued[index]--;

	throtl_charge_bio(tg, bio);

	/*
	 * If our parent is another tg, we just need to transfer @bio to
	 * the parent using throtl_add_bio_tg().  If our parent is
	 * @td->service_queue, @bio is ready to be issued.  Put it on its
	 * bio_lists[] and decrease total number queued.  The caller is
	 * responsible for issuing these bios.
	 */
	if (parent_tg) {
		throtl_add_bio_tg(bio, &tg->qnode_on_parent[index], parent_tg);
		start_parent_slice_with_credit(tg, parent_tg, rw);
	} else {
		throtl_qnode_add_bio(bio, &tg->qnode_on_parent[index],
				     &parent_sq->queued[index]);
		BUG_ON(tg->td->nr_queued[index] <= 0);
		tg->td->nr_queued[index]--;
	}

	throtl_trim_slice(tg, rw);

	if (tg_to_put)
		blkg_put(tg_to_blkg(tg_to_put));
}

static int throtl_dispatch_tg(struct throtl_grp *tg)
{
	struct throtl_service_queue *sq = &tg->service_queue;
	unsigned int nr_reads = 0, nr_writes = 0;
	unsigned int max_nr_reads = throtl_grp_quantum*3/4;
	unsigned int max_nr_writes = throtl_grp_quantum - max_nr_reads;
	struct bio *bio;

	/* Try to dispatch 75% READS and 25% WRITES */

	while ((bio = throtl_peek_queued(&sq->queued[READ])) &&
	       tg_may_dispatch(tg, bio, NULL)) {

		tg_dispatch_one_bio(tg, bio_data_dir(bio));
		nr_reads++;

		if (nr_reads >= max_nr_reads)
			break;
	}

	while ((bio = throtl_peek_queued(&sq->queued[WRITE])) &&
	       tg_may_dispatch(tg, bio, NULL)) {

		tg_dispatch_one_bio(tg, bio_data_dir(bio));
		nr_writes++;

		if (nr_writes >= max_nr_writes)
			break;
	}

	return nr_reads + nr_writes;
}

static int throtl_select_dispatch(struct throtl_service_queue *parent_sq)
{
	unsigned int nr_disp = 0;

	while (1) {
		struct throtl_grp *tg = throtl_rb_first(parent_sq);
		struct throtl_service_queue *sq = &tg->service_queue;

		if (!tg)
			break;

		if (time_before(jiffies, tg->disptime))
			break;

		throtl_dequeue_tg(tg);

		nr_disp += throtl_dispatch_tg(tg);

		if (sq->nr_queued[0] || sq->nr_queued[1])
			tg_update_disptime(tg);

		if (nr_disp >= throtl_quantum)
			break;
	}

	return nr_disp;
}

static bool throtl_can_upgrade(struct throtl_data *td,
	struct throtl_grp *this_tg);
/**
 * throtl_pending_timer_fn - timer function for service_queue->pending_timer
 * @arg: the throtl_service_queue being serviced
 *
 * This timer is armed when a child throtl_grp with active bio's become
 * pending and queued on the service_queue's pending_tree and expires when
 * the first child throtl_grp should be dispatched.  This function
 * dispatches bio's from the children throtl_grps to the parent
 * service_queue.
 *
 * If the parent's parent is another throtl_grp, dispatching is propagated
 * by either arming its pending_timer or repeating dispatch directly.  If
 * the top-level service_tree is reached, throtl_data->dispatch_work is
 * kicked so that the ready bio's are issued.
 */
static void throtl_pending_timer_fn(unsigned long arg)
{
	struct throtl_service_queue *sq = (void *)arg;
	struct throtl_grp *tg = sq_to_tg(sq);
	struct throtl_data *td = sq_to_td(sq);
	struct request_queue *q = td->queue;
	struct throtl_service_queue *parent_sq;
	bool dispatched;
	int ret;

	spin_lock_irq(q->queue_lock);
	if (throtl_can_upgrade(td, NULL))
		throtl_upgrade_state(td);

again:
	parent_sq = sq->parent_sq;
	dispatched = false;

	while (true) {
		throtl_log(sq, "dispatch nr_queued=%u read=%u write=%u",
			   sq->nr_queued[READ] + sq->nr_queued[WRITE],
			   sq->nr_queued[READ], sq->nr_queued[WRITE]);

		ret = throtl_select_dispatch(sq);
		if (ret) {
			throtl_log(sq, "bios disp=%u", ret);
			dispatched = true;
		}

		if (throtl_schedule_next_dispatch(sq, false))
			break;

		/* this dispatch windows is still open, relax and repeat */
		spin_unlock_irq(q->queue_lock);
		cpu_relax();
		spin_lock_irq(q->queue_lock);
	}

	if (!dispatched)
		goto out_unlock;

	if (parent_sq) {
		/* @parent_sq is another throl_grp, propagate dispatch */
		if (tg->flags & THROTL_TG_WAS_EMPTY) {
			tg_update_disptime(tg);
			if (!throtl_schedule_next_dispatch(parent_sq, false)) {
				/* window is already open, repeat dispatching */
				sq = parent_sq;
				tg = sq_to_tg(sq);
				goto again;
			}
		}
	} else {
		/* reached the top-level, queue issueing */
		queue_work(kthrotld_workqueue, &td->dispatch_work);
	}
out_unlock:
	spin_unlock_irq(q->queue_lock);
}

/**
 * blk_throtl_dispatch_work_fn - work function for throtl_data->dispatch_work
 * @work: work item being executed
 *
 * This function is queued for execution when bio's reach the bio_lists[]
 * of throtl_data->service_queue.  Those bio's are ready and issued by this
 * function.
 */
static void blk_throtl_dispatch_work_fn(struct work_struct *work)
{
	struct throtl_data *td = container_of(work, struct throtl_data,
					      dispatch_work);
	struct throtl_service_queue *td_sq = &td->service_queue;
	struct request_queue *q = td->queue;
	struct bio_list bio_list_on_stack;
	struct bio *bio;
	struct blk_plug plug;
	int i;

	bio_list_init(&bio_list_on_stack);

	spin_lock_irq(q->queue_lock);
	for (i = READ; i <= WRITE; i++)
		while ((bio = throtl_pop_queued(&td_sq->queued[i], NULL)))
			bio_list_add(&bio_list_on_stack, bio);
	spin_unlock_irq(q->queue_lock);

	if (!bio_list_empty(&bio_list_on_stack)) {
		blk_start_plug(&plug);
		while((bio = bio_list_pop(&bio_list_on_stack)))
			generic_make_request(bio);
		blk_finish_plug(&plug);
	}
}

static bool tg_may_dispatch_weight(struct throtl_grp *tg)
{
	if (tg_bps_limit(tg, 0) == U64_MAX &&
	    tg_iops_limit(tg, 0) == UINT_MAX)
		return true;

	if (!(tg->flags & THROTL_TG_ONLINE))
		return true;

	if (tg->td->mode == MODE_WEIGHT_BANDWIDTH)
		return (tg->bytes_disp[0] < tg->bps[0][0]);
	else if (tg->td->mode == MODE_WEIGHT_IOPS)
		return (tg->io_disp[0] < tg->iops[0][0]);
	else
		return true;
}

static void throtl_add_bio_weight(struct bio *bio, struct throtl_grp *tg)
{
	struct throtl_data *td = tg->td;

	bio_list_add(&tg->bios, bio);
	tg->service_queue.nr_queued[0]++;
	tg->td->nr_queued[0]++;
	tg->intime = jiffies;

	if (tg->td->nr_queued[0] == 1 && !timer_pending(&tg->td->rescue_timer))
		mod_timer(&tg->td->rescue_timer,
			  jiffies + msecs_to_jiffies(THROTL_IDLE_INTERVAL));

	if (!list_empty(&tg->node))
		return;

	blkg_get(tg_to_blkg(tg));

	if (tg_may_dispatch_weight(tg))
		list_add_tail(&tg->node, &td->active);
	else
		list_add_tail(&tg->node, &td->expired);
}

static struct bio *throtl_pop_queued_weight(struct throtl_grp *tg)
{
	struct bio *bio;

	bio = bio_list_pop(&tg->bios);
	WARN_ON_ONCE(!bio);

	return bio;
}

static void blk_throtl_weight_io_done(struct bio *bio)
{
	struct throtl_grp *tg;
	struct throtl_data *td;
	int index = (int)bio_data_dir(bio);
	int inflights;

	tg = (struct throtl_grp *)bio->bi_throtl_private2;
	td = tg->td;

	atomic_dec(&tg->inflights[index]);
	inflights = atomic_dec_return(&td->inflights);
	smp_mb__after_atomic();
	if (inflights >= td->quantum)
		goto out;
	else if (inflights == 0 && waitqueue_active(&td->waitq))
		wake_up(&td->waitq);

	if (td->nr_queued[0])
		queue_work(kthrotld_workqueue, &td->dispatch_work);
out:
	bio->bi_throtl_private2 = NULL;
	bio->bi_throtl_end_io2 = NULL;
	blkg_put(tg_to_blkg(tg));
}

static void tg_dispatch_one_bio_weight(struct throtl_grp *tg,
				       struct bio_list *bios, bool charge)
{
	struct bio *bio;

	/*
	 * @bio is being transferred from @tg to @parent_sq.  Popping a bio
	 * from @tg may put its reference and @parent_sq might end up
	 * getting released prematurely.  Remember the tg to put and put it
	 * after @bio is transferred to @parent_sq.
	 */
	bio = throtl_pop_queued_weight(tg);
	tg->service_queue.nr_queued[0]--;

	if (unlikely(!charge))
		goto skip_charge;

	throtl_charge_bio(tg, bio);

	if (!bio->bi_throtl_private2) {
		int index = (int)bio_data_dir(bio);

		atomic_inc(&tg->inflights[index]);
		/* Don't reorder td->nr_queued and inflights */
		smp_mb__before_atomic();
		atomic_inc(&tg->td->inflights);
		smp_mb__after_atomic();

		bio->bi_throtl_private2 = tg;

		/* We might access tg when end io, so grab it */
		blkg_get(tg_to_blkg(tg));
		bio->bi_throtl_end_io2 = blk_throtl_weight_io_done;
	}

skip_charge:
	BUG_ON(tg->td->nr_queued[0] <= 0);
	tg->td->nr_queued[0]--;

	bio_list_add(bios, bio);
}

static int throtl_dispatch_tg_weight(struct throtl_grp *tg,
				     struct bio_list *bios)
{
	int nr = 0;

	BUG_ON(!td_weight_based(tg->td));

	while (!bio_list_empty(&tg->bios) &&
	       tg_may_dispatch_weight(tg)) {

		tg_dispatch_one_bio_weight(tg, bios, (bool)true);
		nr++;

		if (nr >= tg->quantum)
			break;
	}

	if (bio_list_empty(&tg->bios) && !(tg->flags & THROTL_TG_ONLINE)) {
		list_del_init(&tg->node);
		blkg_put(tg_to_blkg(tg));
	} else if (!tg_may_dispatch_weight(tg)) {
		list_move_tail(&tg->node, &tg->td->expired);
	}

	return nr;
}

static void throtl_start_new_slice_weight(struct throtl_data *td)
{
	struct throtl_grp *tg;

	list_for_each_entry(tg, &td->expired, node) {
		tg->expired = false;
		if (tg->td->mode == MODE_WEIGHT_BANDWIDTH) {
			tg->bytes_disp[0] = 0;
			wake_up_all(&tg->wait);
		} else if (tg->td->mode == MODE_WEIGHT_IOPS) {
			wake_up_nr(&tg->wait, tg->iops[0][0]);
			tg->io_disp[0] = 0;
		}
	}

	list_splice_tail_init(&td->expired, &td->active);
}

static bool should_start_new_slice(struct throtl_data *td)
{
	struct throtl_grp *tg;
	unsigned long interval;

	interval = msecs_to_jiffies(THROTL_IDLE_INTERVAL / 2);

	list_for_each_entry(tg, &td->expired, node) {
		if (tg_to_blkg(tg)->blkcg->type <= BLK_THROTL_FG)
			return true;
	}

	list_for_each_entry(tg, &td->active, node) {
		if (time_before(jiffies, tg->intime + interval))
			return false;
	}

	return true;
}

static void blk_run_throtl_bios_weight(struct throtl_data *td,
				       struct throtl_grp *tg,
				       struct bio_list *bios)
{
	struct throtl_grp *curr, *next;

	if (tg)
		throtl_dispatch_tg_weight(tg, bios);

	if (list_empty(&td->active) && bio_list_empty(bios))
		goto start_new_slice;

	list_for_each_entry_safe(curr, next, &td->active, node) {
		if (curr == tg)
			continue;
		throtl_dispatch_tg_weight(curr, bios);
	}

	if (!bio_list_empty(bios))
		return;

	/*
	 * If no bios in active queues and many bios are pending in
	 * the device queue, just idle.
	 */
start_new_slice:
	if (!should_start_new_slice(td)) {
		unsigned int interval = THROTL_IDLE_INTERVAL / 2;

		mod_timer(&td->idle_timer,
			  jiffies + msecs_to_jiffies(interval));
		return;
	}

	throtl_start_new_slice_weight(td);

	list_for_each_entry_safe(curr, next, &td->active, node)
		throtl_dispatch_tg_weight(curr, bios);
}

static void blk_throtl_kick_queue_fn(struct work_struct *work)
{
	struct throtl_data *td = container_of(work, struct throtl_data,
					      dispatch_work);
	struct request_queue *q = td->queue;
	struct bio_list bio_list_on_stack;
	struct bio *bio;
	struct blk_plug plug;

	bio_list_init(&bio_list_on_stack);

	spin_lock_irq(q->queue_lock);
	blk_run_throtl_bios_weight(td, NULL, &bio_list_on_stack);
	spin_unlock_irq(q->queue_lock);

	if (!bio_list_empty(&bio_list_on_stack)) {
		blk_start_plug(&plug);
		while((bio = bio_list_pop(&bio_list_on_stack)))
			generic_make_request(bio);
		blk_finish_plug(&plug);
	}
}

static void blk_throtl_rescue_timer_fn(unsigned long arg)
{
	struct throtl_data *td = (struct throtl_data *)arg;
	struct request_queue *q = td->queue;
	unsigned long flags;

	if (ACCESS_ONCE(td->nr_queued[0])) {
		smp_mb__after_atomic();
		if (!atomic_read(&td->inflights))
			queue_work(kthrotld_workqueue, &td->dispatch_work);

		mod_timer(&td->rescue_timer,
			  jiffies + msecs_to_jiffies(THROTL_IDLE_INTERVAL));
		return;
	}

	spin_lock_irqsave(q->queue_lock, flags);
	if (ACCESS_ONCE(td->nr_queued[0])) {
		smp_mb__after_atomic();
		if (!atomic_read(&td->inflights))
			queue_work(kthrotld_workqueue, &td->dispatch_work);

		mod_timer(&td->rescue_timer,
			  jiffies + msecs_to_jiffies(THROTL_IDLE_INTERVAL));
	}
	spin_unlock_irqrestore(q->queue_lock, flags);
}

static void blk_throtl_idle_timer_fn(unsigned long arg)
{
	struct throtl_data *td = (struct throtl_data *)arg;
	struct request_queue *q = td->queue;
	unsigned long flags;

	spin_lock_irqsave(q->queue_lock, flags);
	if (td->nr_queued[0])
		queue_work(kthrotld_workqueue, &td->dispatch_work);
	spin_unlock_irqrestore(q->queue_lock, flags);
}

static void tg_drain_bios_weight(struct throtl_grp *tg, struct bio_list *bios)
{
	WARN_ON(list_empty(&tg->node));

	list_del_init(&tg->node);

	while (!bio_list_empty(&tg->bios))
		tg_dispatch_one_bio_weight(tg, bios, false);

	tg->bytes_disp[0] = 0;
	tg->io_disp[0] = 0;

	blkg_put(tg_to_blkg(tg));
}

static void blk_throtl_drain_weight(struct request_queue *q)
{
	struct throtl_data *td = q->td;
	struct throtl_grp *curr, *next;
	struct bio *bio;
	struct bio_list bios;

	bio_list_init(&bios);

	list_for_each_entry_safe(curr, next, &td->active, node)
		tg_drain_bios_weight(curr, &bios);

	list_for_each_entry_safe(curr, next, &td->expired, node)
		tg_drain_bios_weight(curr, &bios);

	spin_unlock_irq(q->queue_lock);

	while((bio = bio_list_pop(&bios)))
		generic_make_request(bio);

	spin_lock_irq(q->queue_lock);
}

static void tg_drain_node_weight(struct throtl_grp *tg)
{
	WARN_ON(list_empty(&tg->node));

	list_del_init(&tg->node);
	tg->bytes_disp[0] = 0;
	tg->io_disp[0] = 0;
	tg->expired = false;

	wake_up_all(&tg->wait);

	blkg_put(tg_to_blkg(tg));
}

static void __blk_throtl_drain_weight(struct request_queue *q)
{
	struct throtl_data *td = q->td;
	struct throtl_grp *curr, *next;

	list_for_each_entry_safe(curr, next, &td->active, node)
		tg_drain_node_weight(curr);

	list_for_each_entry_safe(curr, next, &td->expired, node)
		tg_drain_node_weight(curr);
}

static bool atomic_inc_below(atomic_t *v, int below)
{
	int cur = atomic_read(v);

	for (;;) {
		int old;

		if (cur >= below)
			return false;

		old = atomic_cmpxchg(v, cur, cur + 1);
		if (old == cur)
			break;

		cur = old;
	}

	return true;
}

static bool io_event_ok(struct throtl_io_limit *io_limit, int limit)
{
	if (limit == 0)
		limit = INT_MAX;

	return atomic_inc_below(&io_limit->inflights, limit);
}

void blk_throtl_update_limit(struct blk_throtl_io_limit *io_limit,
			     int limit)
{
	io_limit->max_inflights = limit;
	wake_up_all(&io_limit->io_limits[0].waitq);
	wake_up_all(&io_limit->io_limits[1].waitq);
}

static void __throtl_limit_io_done(struct throtl_io_limit *io_limit,
				   int limit)
{
	int inflights;

	inflights = atomic_dec_return(&io_limit->inflights);
	if (inflights >= limit) {
		if (!limit && waitqueue_active(&io_limit->waitq))
			wake_up_all(&io_limit->waitq);
		return;
	}

	if (waitqueue_active(&io_limit->waitq)) {
		int diff = max(limit - inflights, 1);

		wake_up_nr(&io_limit->waitq, diff);
	}
}

static void blk_throtl_limit_io_done(struct bio *bio)
{
	struct blk_throtl_io_limit *io_limit;
	int limit;
	int index = (int)bio_data_dir(bio);

	io_limit = (struct blk_throtl_io_limit *)bio->bi_throtl_private1;
	limit = ACCESS_ONCE(io_limit->max_inflights);
	__throtl_limit_io_done(&io_limit->io_limits[index], limit);

	bio->bi_throtl_private1 = NULL;
	bio->bi_throtl_end_io1 = NULL;
	blk_throtl_io_limit_put(io_limit);
}

/* We must hold rcu lock. */
static int blk_throtl_io_limit_wait(struct blkcg *blkcg,
				    struct blk_throtl_io_limit *task_limit,
				    struct throtl_grp *tg,
				    struct bio *bio)
{
	struct blkcg *new;
	DEFINE_WAIT(wait);
	int limit;
	int index = (int)bio_data_dir(bio);
	struct throtl_io_limit *io_limit = &task_limit->io_limits[index];
	int ret;

	WARN_ON_ONCE(!rcu_read_lock_held());

	limit = ACCESS_ONCE(task_limit->max_inflights);
	if (io_event_ok(io_limit, limit))
		return 0;

	ret = 0;
	for (;;) {
		prepare_to_wait(&io_limit->waitq, &wait, TASK_UNINTERRUPTIBLE);
		limit = ACCESS_ONCE(task_limit->max_inflights);
		if (io_event_ok(io_limit, limit))
			break;

		new = bio_blkcg(bio);
		if (new != blkcg) {
			ret = 1;
			break;
		}

		if (!tg->td->max_inflights) {
			ret = 2;
			break;
		}

		task_set_sleep_on_throtl(current);
		rcu_read_unlock();
		io_schedule();
		rcu_read_lock();
		task_clear_sleep_on_throtl(current);
	}
	finish_wait(&io_limit->waitq, &wait);
	return ret;
}

static u64 tg_prfill_conf_u64(struct seq_file *sf, struct blkg_policy_data *pd,
			      int off)
{
	struct throtl_grp *tg = pd_to_tg(pd);
	u64 v = *(u64 *)((void *)tg + off);

	if (v == U64_MAX)
		return 0;
	return __blkg_prfill_u64(sf, pd, v);
}

static u64 tg_prfill_conf_uint(struct seq_file *sf, struct blkg_policy_data *pd,
			       int off)
{
	struct throtl_grp *tg = pd_to_tg(pd);
	unsigned int v = *(unsigned int *)((void *)tg + off);

	if (v == UINT_MAX)
		return 0;
	return __blkg_prfill_u64(sf, pd, v);
}

static int tg_print_conf_u64(struct seq_file *sf, void *v)
{
	blkcg_print_blkgs(sf, css_to_blkcg(seq_css(sf)), tg_prfill_conf_u64,
			  &blkcg_policy_throtl, seq_cft(sf)->private, false);
	return 0;
}

static int tg_print_conf_uint(struct seq_file *sf, void *v)
{
	blkcg_print_blkgs(sf, css_to_blkcg(seq_css(sf)), tg_prfill_conf_uint,
			  &blkcg_policy_throtl, seq_cft(sf)->private, false);
	return 0;
}

static void tg_conf_updated(struct throtl_grp *tg, bool global)
{
	struct throtl_service_queue *sq = &tg->service_queue;
	struct cgroup_subsys_state *pos_css;
	struct blkcg_gq *blkg;

	throtl_log(&tg->service_queue,
		   "limit change rbps=%llu wbps=%llu riops=%u wiops=%u",
		   tg_bps_limit(tg, READ), tg_bps_limit(tg, WRITE),
		   tg_iops_limit(tg, READ), tg_iops_limit(tg, WRITE));

	/*
	 * Update has_rules[] flags for the updated tg's subtree.  A tg is
	 * considered to have rules if either the tg itself or any of its
	 * ancestors has rules.  This identifies groups without any
	 * restrictions in the whole hierarchy and allows them to bypass
	 * blk-throttle.
	 */
	blkg_for_each_descendant_pre(blkg, pos_css,
			global ? tg->td->queue->root_blkg : tg_to_blkg(tg)) {
		struct throtl_grp *this_tg = blkg_to_tg(blkg);
		struct throtl_grp *parent_tg;

		tg_update_has_rules(this_tg);
		/* ignore root/second level */
		if (!cgroup_subsys_on_dfl(io_cgrp_subsys) || !blkg->parent ||
		    !blkg->parent->parent)
			continue;
		parent_tg = blkg_to_tg(blkg->parent);
		/*
		 * make sure all children has lower idle time threshold and
		 * higher latency target
		 */
		this_tg->idletime_threshold = min(this_tg->idletime_threshold,
				parent_tg->idletime_threshold);
		this_tg->latency_target = max(this_tg->latency_target,
				parent_tg->latency_target);
	}

	/*
	 * We're already holding queue_lock and know @tg is valid.  Let's
	 * apply the new config directly.
	 *
	 * Restart the slices for both READ and WRITES. It might happen
	 * that a group's limit are dropped suddenly and we don't want to
	 * account recently dispatched IO with new low rate.
	 */
	throtl_start_new_slice(tg, 0);
	throtl_start_new_slice(tg, 1);

	if (tg->flags & THROTL_TG_PENDING) {
		tg_update_disptime(tg);
		throtl_schedule_next_dispatch(sq->parent_sq, true);
	}
}

static ssize_t tg_set_conf(struct kernfs_open_file *of,
			   char *buf, size_t nbytes, loff_t off, bool is_u64)
{
	struct blkcg *blkcg = css_to_blkcg(of_css(of));
	struct blkg_conf_ctx ctx;
	struct throtl_grp *tg;
	int ret;
	u64 v;

	ret = blkg_conf_prep(blkcg, &blkcg_policy_throtl, buf, &ctx);
	if (ret)
		return ret;

	ret = -EINVAL;
	if (sscanf(ctx.body, "%llu", &v) != 1)
		goto out_finish;
	if (!v)
		v = U64_MAX;

	tg = blkg_to_tg(ctx.blkg);

	if (is_u64)
		*(u64 *)((void *)tg + of_cft(of)->private) = v;
	else
		*(unsigned int *)((void *)tg + of_cft(of)->private) = v;

	tg_conf_updated(tg, false);
	ret = 0;
out_finish:
	blkg_conf_finish(&ctx);
	return ret ?: nbytes;
}

static ssize_t tg_set_conf_u64(struct kernfs_open_file *of,
			       char *buf, size_t nbytes, loff_t off)
{
	return tg_set_conf(of, buf, nbytes, off, true);
}

static ssize_t tg_set_conf_uint(struct kernfs_open_file *of,
				char *buf, size_t nbytes, loff_t off)
{
	return tg_set_conf(of, buf, nbytes, off, false);
}

static u64 tg_prfill_mode_device(struct seq_file *sf,
	struct blkg_policy_data *pd, int off)
{
	struct throtl_grp *tg = pd_to_tg(pd);
	const char *dname = blkg_dev_name(pd->blkg);

	if (!dname)
		return 0;
	if (tg->td->mode == MODE_NONE)
		return 0;
	seq_printf(sf, "%s %s\n", dname, run_mode_name[tg->td->mode]);
	return 0;
}

static int throtl_print_mode_device(struct seq_file *sf, void *v)
{
	int i;

	seq_printf(sf, "available ");
	for (i = 0; i < MAX_MODE; i++)
		seq_printf(sf, "%s(%d) ", run_mode_name[i], i);
	seq_printf(sf, "\n");
	seq_printf(sf, "default %s\n", run_mode_name[MODE_NONE]);

	mutex_lock(&throtl_mode_lock);
	blkcg_print_blkgs(sf, css_to_blkcg(seq_css(sf)),
		tg_prfill_mode_device,  &blkcg_policy_throtl, 0, false);
	mutex_unlock(&throtl_mode_lock);
	return 0;
}

static void tg_update_rules(struct throtl_data *td)
{
	struct cgroup_subsys_state *pos_css;
	struct blkcg_gq *blkg, *top_blkg;

	top_blkg = td->queue->root_blkg;

	blkg_for_each_descendant_pre(blkg, pos_css, top_blkg)
		tg_update_has_rules(blkg_to_tg(blkg));
}

static void tg_change_mode(struct throtl_data *td, int mode)
{
	struct request_queue *q = td->queue;

	if (q->mq_ops)
		blk_mq_freeze_queue(q);
	else
		blk_queue_bypass_start(q);

	/*
	 * Maybe it is unnecessary, but we need make sure all bios in the
	 * block throttle layer are drained.
	 */
	spin_lock_irq(q->queue_lock);
	blkcg_drain_queue(q);
	spin_unlock_irq(q->queue_lock);

	WARN_ON(td->nr_queued[0]);
	WARN_ON(td->nr_queued[1]);

	wait_event(td->waitq, !atomic_read(&td->inflights));

	del_timer_sync(&td->rescue_timer);
	del_timer_sync(&td->idle_timer);
	flush_work(&td->dispatch_work);

	spin_lock_irq(q->queue_lock);
	td->mode = mode;

	if (td_weight_based(td))
		INIT_WORK(&td->dispatch_work, blk_throtl_kick_queue_fn);
	else
		INIT_WORK(&td->dispatch_work, blk_throtl_dispatch_work_fn);

	rcu_read_lock();
	tg_update_share(td, NULL);
	tg_update_rules(td);
	rcu_read_unlock();
	spin_unlock_irq(q->queue_lock);

	if (q->mq_ops)
		blk_mq_unfreeze_queue(q);
	else
		blk_queue_bypass_end(q);
}

static ssize_t tg_set_mode_device(struct kernfs_open_file *of,
				  char *buf, size_t nbytes, loff_t off)
{
	struct blkcg *blkcg = css_to_blkcg(of_css(of));
	struct blkg_conf_ctx ctx;
	struct throtl_grp *tg;
	struct throtl_data *td;
	struct gendisk *disk;
	int ret;
	int mode;
	bool need_change = false;

	mutex_lock(&throtl_mode_lock);
	ret = blkg_conf_prep(blkcg, &blkcg_policy_throtl, buf, &ctx);
	if (ret)
		goto out;

	if (sscanf(ctx.body, "%d", &mode) != 1)
		goto out_finish;

	ret = -EINVAL;
	if (mode < 0 || mode >= MAX_MODE)
		goto out_finish;

	tg = blkg_to_tg(ctx.blkg);
	if (tg->td->mode == mode) {
		ret = 0;
		goto out_finish;
	}

	disk = ctx.disk;
	if (!get_disk(disk)) {
		ret = -EBUSY;
		goto out_finish;
	}

	td = tg->td;
	need_change = true;
	ret = 0;

out_finish:
	blkg_conf_finish(&ctx);

	if (need_change) {
		tg_change_mode(td, mode);
		put_disk(disk);
	}
out:
	mutex_unlock(&throtl_mode_lock);
	return ret ?: nbytes;
}

static u64 tg_prfill_weight_device(struct seq_file *sf,
		struct blkg_policy_data *pd, int off)
{
	struct blkcg_gq *blkg = pd_to_blkg(pd);
	const char *dname = blkg_dev_name(pd->blkg);

	if (!dname)
		return 0;
	if (!blkg)
		return 0;
	if (blkg->weight == blkg->blkcg->weight)
		return 0;
	seq_printf(sf, "%s: weight %u\n", dname, blkg->weight);
	return 0;
}

static int tg_print_weight(struct seq_file *sf, void *v)
{
	struct blkcg *blkcg = css_to_blkcg(seq_css(sf));

	seq_printf(sf, "default: %u\n", blkcg->weight);
	blkcg_print_blkgs(sf, blkcg, tg_prfill_weight_device,
			  &blkcg_policy_throtl, 0, false);
	return 0;
}

static int tg_print_weight_device(struct seq_file *sf, void *v)
{
	blkcg_print_blkgs(sf, css_to_blkcg(seq_css(sf)),
			  tg_prfill_weight_device,
			  &blkcg_policy_throtl, 0, false);
	return 0;
}

static ssize_t tg_set_weight_device(struct kernfs_open_file *of,
			   char *buf, size_t nbytes, loff_t off)
{
	struct blkcg *blkcg = css_to_blkcg(of_css(of));
	struct throtl_grp *tg;
	struct throtl_service_queue *parent_sq;
	struct blkg_conf_ctx ctx;
	unsigned int val;
	int ret;

	ret = blkg_conf_prep(blkcg, &blkcg_policy_throtl, buf, &ctx);
	if (ret)
		return ret;

	ret = -EINVAL;
	if (sscanf(ctx.body, "%d", &val) != 1)
		goto out_finish;

	if (val < BLKIO_WEIGHT_MIN || val > BLKIO_WEIGHT_MAX)
		goto out_finish;

	tg = blkg_to_tg(ctx.blkg);
	parent_sq = tg->service_queue.parent_sq;
	if (parent_sq) {
		parent_sq->children_weight -= ctx.blkg->weight;
		parent_sq->children_weight += val;
	}

	ctx.blkg->weight = val;

	tg_update_share(tg->td, tg);

	ret = 0;
out_finish:
	blkg_conf_finish(&ctx);
	return ret ?: nbytes;
}

static ssize_t tg_set_weight_offon(struct kernfs_open_file *of,
				   char *buf, size_t nbytes,
				   loff_t off)
{
	int ret;
	char offon[8];

	if (sscanf(buf, "%7s", offon) != 1)
		return -EINVAL;

	ret = 0;
	if (strnlen(offon, (size_t)7) == 5 &&
	    !strncmp(offon, "on-fs", (size_t)6))
		blk_throtl_weight_offon = BLK_THROTL_WEIGHT_ON_FS;
	else if (strnlen(offon, (size_t)7) == 6 &&
	    !strncmp(offon, "on-blk", (size_t)7))
		blk_throtl_weight_offon = BLK_THROTL_WEIGHT_ON_BLK;
	else if (strnlen(offon, (size_t)4) == 3 &&
		 !strncmp(offon, "off", (size_t)4))
		blk_throtl_weight_offon = BLK_THROTL_WEIGHT_OFF;
	else
		ret = -EINVAL;

	return ret ?: nbytes;
}

static int tg_print_weight_offon(struct seq_file *sf, void *v)
{
	seq_printf(sf, "Weight Based Throttle: %s\n",
		   (blk_throtl_weight_offon == BLK_THROTL_WEIGHT_ON_FS) ?
		    "on-fs" :
		    ((blk_throtl_weight_offon == BLK_THROTL_WEIGHT_ON_BLK) ?
		     "on-blk" : "off"));
	return 0;
}

static u64 tg_prfill_max_inflights_device(struct seq_file *sf,
					  struct blkg_policy_data *pd, int off)
{
	struct throtl_grp *tg = pd_to_tg(pd);
	const char *dname = blkg_dev_name(pd->blkg);

	if (!dname)
		return 0;

	seq_printf(sf, "%s: queued %u, read inflight %d, write inflight %d, devices inflights %d\n",
		   dname, tg->service_queue.nr_queued[0],
		   atomic_read(&tg->inflights[0]),
		   atomic_read(&tg->inflights[1]),
		   atomic_read(&tg->td->inflights));

	return 0;
}

static int tg_print_max_inflights(struct seq_file *sf, void *v)
{
	struct blkcg *blkcg = css_to_blkcg(seq_css(sf));

	seq_printf(sf, "max limit: %d\n", blkcg->max_inflights);

	blkcg_print_blkgs(sf, css_to_blkcg(seq_css(sf)),
			  tg_prfill_max_inflights_device,
			  &blkcg_policy_throtl, 0, false);

	return 0;
}

static int tg_set_max_inflights(struct cgroup_subsys_state *css,
				struct cftype *cft, s64 val)
{
	struct blkcg *blkcg = css_to_blkcg(css);
	struct task_struct *task;
	struct css_task_iter it;
	int max_inflights = (int)val;

	if (max_inflights < MIN_INFLIGHT || max_inflights > MAX_INFLIGHT)
		return -EINVAL;

	spin_lock_irq(&blkcg->lock);
	if (blkcg->max_inflights == max_inflights)
		goto out_unlock;

	blkcg->max_inflights = max_inflights;

	rcu_read_lock();
	css_task_iter_start(css, 0, &it);
	while ((task = css_task_iter_next(&it))) {
		if (!task->mm)
			continue;

		if (!task->mm->io_limit)
			continue;

		blk_throtl_update_limit(task->mm->io_limit, max_inflights);
	}
	css_task_iter_end(&it);
	rcu_read_unlock();
out_unlock:
	spin_unlock_irq(&blkcg->lock);
	return 0;
}

static u64 tg_prfill_enable_max_inflights_device(struct seq_file *sf,
		struct blkg_policy_data *pd, int off)
{
	struct throtl_grp *tg = pd_to_tg(pd);
	const char *dname = blkg_dev_name(pd->blkg);

	if (!dname)
		return 0;

	seq_printf(sf, "%s: limit %s\n",
		   dname, (tg->td->max_inflights ? "open" : "close"));
	return 0;
}

static int tg_print_enable_max_inflight_device(struct seq_file *sf, void *v)
{
	blkcg_print_blkgs(sf, css_to_blkcg(seq_css(sf)),
			  tg_prfill_enable_max_inflights_device,
			  &blkcg_policy_throtl, 0, false);
	return 0;
}

static ssize_t tg_set_enable_max_inflights_device(struct kernfs_open_file *of,
			   char *buf, size_t nbytes, loff_t off)
{
	struct cgroup_subsys_state *css = of_css(of);
	struct blkcg *blkcg = css_to_blkcg(css);
	struct blkcg_gq *blkg;
	struct blkg_conf_ctx ctx;
	struct throtl_grp *tg;
	struct task_struct *task;
	struct css_task_iter it;
	unsigned long flags;
	int max_inflight;
	int ret;

	ret = blkg_conf_prep(blkcg, &blkcg_policy_throtl, buf, &ctx);
	if (ret)
		return ret;

	ret = -EINVAL;

	if (sscanf(ctx.body, "%d", &max_inflight) != 1)
		goto out_finish;

	if (max_inflight != 0 && max_inflight != 1)
		goto out_finish;

	ret = 0;
	tg = blkg_to_tg(ctx.blkg);
	if (tg->td->max_inflights == max_inflight)
		goto out_finish;

	spin_lock_irqsave(&blkcg->lock, flags);

	if (!max_inflight)
		goto skip_check;

	/* we just open only one device for limit */
	hlist_for_each_entry(blkg, &blkcg->blkg_list, blkcg_node) {
		struct throtl_grp *curr_tg = blkg_to_tg(blkg);

		if (!curr_tg || curr_tg == tg)
			continue;

		if (curr_tg->td->max_inflights) {
			ret = -EBUSY;
			goto out_unlock;
		}
	}
skip_check:
	tg->td->max_inflights = max_inflight;
	smp_mb();
	if (max_inflight)
		goto out_unlock;

	rcu_read_lock();

	css_task_iter_start(css, 0, &it);
	while ((task = css_task_iter_next(&it))) {
		if (task_sleep_on_throtl(task))
			wake_up_process(task);
	}
	css_task_iter_end(&it);

	rcu_read_unlock();
out_unlock:
	spin_unlock_irqrestore(&blkcg->lock, flags);
out_finish:
	blkg_conf_finish(&ctx);
	return ret ?: nbytes;
}

static u64 tg_prfill_quantum_device(struct seq_file *sf,
				    struct blkg_policy_data *pd, int off)
{
	struct blkcg_gq *blkg = pd_to_blkg(pd);
	const char *dname = blkg_dev_name(pd->blkg);

	if (!dname)
		return 0;

	if (!blkg)
		return 0;

	if ((blkg_to_tg(blkg)->td->quantum) == QUANTUM_INQUEUE_DEF)
		return 0;

	seq_printf(sf, "%s: quantum %u\n", dname,
		   blkg_to_tg(blkg)->td->quantum);
	return 0;
}

static int tg_print_quantum_device(struct seq_file *sf, void *v)
{
	seq_printf(sf, "default: %d\n", QUANTUM_INQUEUE_DEF);

	blkcg_print_blkgs(sf, css_to_blkcg(seq_css(sf)),
			  tg_prfill_quantum_device,
			  &blkcg_policy_throtl, 0, false);
	return 0;
}

static ssize_t tg_set_quantum_device(struct kernfs_open_file *of,
				     char *buf, size_t nbytes, loff_t off)
{
	struct blkcg *blkcg = css_to_blkcg(of_css(of));
	struct throtl_grp *tg;
	struct blkg_conf_ctx ctx;
	int val;
	int ret;

	ret = blkg_conf_prep(blkcg, &blkcg_policy_throtl, buf, &ctx);
	if (ret)
		return ret;

	ret = -EINVAL;
	if (sscanf(ctx.body, "%d", &val) != 1)
		goto out_finish;

	if (val < QUANTUM_INQUEUE_MIN || val > QUANTUM_INQUEUE_MAX)
		goto out_finish;

	tg = blkg_to_tg(ctx.blkg);
	tg->td->quantum = val;

	ret = 0;
out_finish:
	blkg_conf_finish(&ctx);
	return ret ?: nbytes;
}

static u64 tg_prfill_bw_slice_device(struct seq_file *sf,
				     struct blkg_policy_data *pd, int off)
{
	struct blkcg_gq *blkg = pd_to_blkg(pd);
	const char *dname = blkg_dev_name(pd->blkg);

	if (!dname)
		return 0;

	if (!blkg)
		return 0;

	if ((blkg_to_tg(blkg)->td->bw_slice) == BW_SLICE_DEF)
		return 0;

	seq_printf(sf, "%s: bw_slice %llu\n", dname,
		   blkg_to_tg(blkg)->td->bw_slice);
	 return 0;
}

static int tg_print_bw_slice_device(struct seq_file *sf, void *v)
{
	seq_printf(sf, "default: %llu\n", BW_SLICE_DEF);

	blkcg_print_blkgs(sf, css_to_blkcg(seq_css(sf)),
			  tg_prfill_bw_slice_device,
			  &blkcg_policy_throtl, 0, false);
	return 0;
}

static ssize_t tg_set_bw_slice_device(struct kernfs_open_file *of,
				      char *buf, size_t nbytes, loff_t off)
{
	struct blkcg *blkcg = css_to_blkcg(of_css(of));
	struct throtl_grp *tg;
	struct blkg_conf_ctx ctx;
	uint64_t val;
	int ret;

	ret = blkg_conf_prep(blkcg, &blkcg_policy_throtl, buf, &ctx);
	if (ret)
		return ret;

	ret = -EINVAL;
	if (sscanf(ctx.body, "%llu", &val) != 1)
		goto out_finish;

	if (val < BW_SLICE_MIN || val > BW_SLICE_MAX)
		goto out_finish;

	tg = blkg_to_tg(ctx.blkg);

	tg->td->bw_slice = val;

	tg_update_share(tg->td, NULL);

	ret = 0;
out_finish:
	blkg_conf_finish(&ctx);
	return ret ?: nbytes;
}

static u64 tg_prfill_iops_slice_device(struct seq_file *sf,
				       struct blkg_policy_data *pd,
				       int off)
{
	struct blkcg_gq *blkg = pd_to_blkg(pd);
	const char *dname = blkg_dev_name(pd->blkg);

	if (!dname)
		return 0;

	if (!blkg)
		return 0;

	if (blkg_to_tg(blkg)->td->iops_slice == IOPS_SLICE_DEF)
		return 0;

	seq_printf(sf, "%s: iops_slice %u\n", dname,
		   blkg_to_tg(blkg)->td->iops_slice);
	return 0;
}

static int tg_print_iops_slice_device(struct seq_file *sf, void *v)
{
	seq_printf(sf, "default: %d\n", IOPS_SLICE_DEF);

	blkcg_print_blkgs(sf, css_to_blkcg(seq_css(sf)),
			  tg_prfill_iops_slice_device,
			  &blkcg_policy_throtl, 0, false);
	return 0;
}

static ssize_t tg_set_iops_slice_device(struct kernfs_open_file *of,
			   char *buf, size_t nbytes, loff_t off)
{
	struct blkcg *blkcg = css_to_blkcg(of_css(of));
	struct throtl_grp *tg;
	struct blkg_conf_ctx ctx;
	unsigned int val;
	int ret;

	ret = blkg_conf_prep(blkcg, &blkcg_policy_throtl, buf, &ctx);
	if (ret)
		return ret;

	ret = -EINVAL;
	if (sscanf(ctx.body, "%d", &val) != 1)
		goto out_finish;

	if (val < IOPS_SLICE_MIN || val > IOPS_SLICE_MAX)
		goto out_finish;

	tg = blkg_to_tg(ctx.blkg);

	tg->td->iops_slice = val;

	tg_update_share(tg->td, NULL);

	ret = 0;
out_finish:
	blkg_conf_finish(&ctx);
	return ret ?: nbytes;
}

static int tg_print_cgroup_type(struct seq_file *sf, void *v)
{
	struct blkcg *blkcg = css_to_blkcg(seq_css(sf));

	seq_printf(sf, "type: %d\n", blkcg->type);

	return 0;
}

static int tg_set_cgroup_type(struct cgroup_subsys_state *css,
				struct cftype *cft, u64 val)
{
	struct blkcg *blkcg = css_to_blkcg(css);
	unsigned int type = (unsigned int)val;

	if (type >= BLK_THROTL_TYPE_NR)
		return -EINVAL;

	blkcg->type = type;
	return 0;
}

static struct cftype throtl_legacy_files[] = {
	{
		.name = "throttle.mode_device",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.seq_show = throtl_print_mode_device,
		.write = tg_set_mode_device,
	},
	{
		.name = "throttle.read_bps_device",
		.private = offsetof(struct throtl_grp, bps[READ][LIMIT_MAX]),
		.seq_show = tg_print_conf_u64,
		.write = tg_set_conf_u64,
	},
	{
		.name = "throttle.write_bps_device",
		.private = offsetof(struct throtl_grp, bps[WRITE][LIMIT_MAX]),
		.seq_show = tg_print_conf_u64,
		.write = tg_set_conf_u64,
	},
	{
		.name = "throttle.read_iops_device",
		.private = offsetof(struct throtl_grp, iops[READ][LIMIT_MAX]),
		.seq_show = tg_print_conf_uint,
		.write = tg_set_conf_uint,
	},
	{
		.name = "throttle.write_iops_device",
		.private = offsetof(struct throtl_grp, iops[WRITE][LIMIT_MAX]),
		.seq_show = tg_print_conf_uint,
		.write = tg_set_conf_uint,
	},
	{
		.name = "throttle.io_service_bytes",
		.private = (unsigned long)&blkcg_policy_throtl,
		.seq_show = blkg_print_stat_bytes,
	},
	{
		.name = "throttle.io_serviced",
		.private = (unsigned long)&blkcg_policy_throtl,
		.seq_show = blkg_print_stat_ios,
	},
	{
		.name = "throttle.weight_device",
		.flags = CFTYPE_NOT_ON_ROOT,
		.seq_show = tg_print_weight_device,
		.write = tg_set_weight_device,
	},
	{
		.name = "throttle.weight",
		.flags = CFTYPE_NOT_ON_ROOT,
		.seq_show = tg_print_weight,
	},
	{
		.name = "throttle.weight_based_offon",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.seq_show = tg_print_weight_offon,
		.write =  tg_set_weight_offon,
	},
	{
		.name = "throttle.max_inflights",
		.flags = CFTYPE_NOT_ON_ROOT,
		.seq_show = tg_print_max_inflights,
		.write_s64 = tg_set_max_inflights,
	},
	{
		.name = "throttle.enable_max_inflights_device",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.seq_show = tg_print_enable_max_inflight_device,
		.write = tg_set_enable_max_inflights_device,
	},
	{
		.name = "throttle.quantum_device",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.seq_show = tg_print_quantum_device,
		.write = tg_set_quantum_device,
	},
	{
		.name = "throttle.bw_slice_device",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.seq_show = tg_print_bw_slice_device,
		.write = tg_set_bw_slice_device,
	},
	{
		.name = "throttle.iops_slice_device",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.seq_show = tg_print_iops_slice_device,
		.write = tg_set_iops_slice_device,
	},
	{
		.name = "throttle.type",
		.seq_show = tg_print_cgroup_type,
		.write_u64 = tg_set_cgroup_type,
	},
	{ }	/* terminate */
};

static u64 tg_prfill_limit(struct seq_file *sf, struct blkg_policy_data *pd,
			 int off)
{
	struct throtl_grp *tg = pd_to_tg(pd);
	const char *dname = blkg_dev_name(pd->blkg);
	char bufs[4][21] = { "max", "max", "max", "max" };
	u64 bps_dft;
	unsigned int iops_dft;
	char idle_time[26] = "";
	char latency_time[26] = "";

	if (!dname)
		return 0;

	if (off == LIMIT_LOW) {
		bps_dft = 0;
		iops_dft = 0;
	} else {
		bps_dft = U64_MAX;
		iops_dft = UINT_MAX;
	}

	if (tg->bps_conf[READ][off] == bps_dft &&
	    tg->bps_conf[WRITE][off] == bps_dft &&
	    tg->iops_conf[READ][off] == iops_dft &&
	    tg->iops_conf[WRITE][off] == iops_dft &&
	    (off != LIMIT_LOW ||
	     (tg->idletime_threshold_conf == DFL_IDLE_THRESHOLD &&
	      tg->latency_target_conf == DFL_LATENCY_TARGET)))
		return 0;

	if (tg->bps_conf[READ][off] != U64_MAX)
		snprintf(bufs[0], sizeof(bufs[0]), "%llu",
			tg->bps_conf[READ][off]);
	if (tg->bps_conf[WRITE][off] != U64_MAX)
		snprintf(bufs[1], sizeof(bufs[1]), "%llu",
			tg->bps_conf[WRITE][off]);
	if (tg->iops_conf[READ][off] != UINT_MAX)
		snprintf(bufs[2], sizeof(bufs[2]), "%u",
			tg->iops_conf[READ][off]);
	if (tg->iops_conf[WRITE][off] != UINT_MAX)
		snprintf(bufs[3], sizeof(bufs[3]), "%u",
			tg->iops_conf[WRITE][off]);
	if (off == LIMIT_LOW) {
		if (tg->idletime_threshold_conf == ULONG_MAX)
			strcpy(idle_time, " idle=max");
		else
			snprintf(idle_time, sizeof(idle_time), " idle=%lu",
				tg->idletime_threshold_conf);

		if (tg->latency_target_conf == ULONG_MAX)
			strcpy(latency_time, " latency=max");
		else
			snprintf(latency_time, sizeof(latency_time),
				" latency=%lu", tg->latency_target_conf);
	}

	seq_printf(sf, "%s rbps=%s wbps=%s riops=%s wiops=%s%s%s\n",
		   dname, bufs[0], bufs[1], bufs[2], bufs[3], idle_time,
		   latency_time);
	return 0;
}

static int tg_print_limit(struct seq_file *sf, void *v)
{
	blkcg_print_blkgs(sf, css_to_blkcg(seq_css(sf)), tg_prfill_limit,
			  &blkcg_policy_throtl, seq_cft(sf)->private, false);
	return 0;
}

static ssize_t tg_set_limit(struct kernfs_open_file *of,
			  char *buf, size_t nbytes, loff_t off)
{
	struct blkcg *blkcg = css_to_blkcg(of_css(of));
	struct blkg_conf_ctx ctx;
	struct throtl_grp *tg;
	u64 v[4];
	unsigned long idle_time;
	unsigned long latency_time;
	int ret;
	int index = of_cft(of)->private;

	ret = blkg_conf_prep(blkcg, &blkcg_policy_throtl, buf, &ctx);
	if (ret)
		return ret;

	tg = blkg_to_tg(ctx.blkg);

	v[0] = tg->bps_conf[READ][index];
	v[1] = tg->bps_conf[WRITE][index];
	v[2] = tg->iops_conf[READ][index];
	v[3] = tg->iops_conf[WRITE][index];

	idle_time = tg->idletime_threshold_conf;
	latency_time = tg->latency_target_conf;
	while (true) {
		char tok[27];	/* wiops=18446744073709551616 */
		char *p;
		u64 val = U64_MAX;
		int len;

		if (sscanf(ctx.body, "%26s%n", tok, &len) != 1)
			break;
		if (tok[0] == '\0')
			break;
		ctx.body += len;

		ret = -EINVAL;
		p = tok;
		strsep(&p, "=");
		if (!p || (sscanf(p, "%llu", &val) != 1 && strcmp(p, "max")))
			goto out_finish;

		ret = -ERANGE;
		if (!val)
			goto out_finish;

		ret = -EINVAL;
		if (!strcmp(tok, "rbps"))
			v[0] = val;
		else if (!strcmp(tok, "wbps"))
			v[1] = val;
		else if (!strcmp(tok, "riops"))
			v[2] = min_t(u64, val, UINT_MAX);
		else if (!strcmp(tok, "wiops"))
			v[3] = min_t(u64, val, UINT_MAX);
		else if (off == LIMIT_LOW && !strcmp(tok, "idle"))
			idle_time = val;
		else if (off == LIMIT_LOW && !strcmp(tok, "latency"))
			latency_time = val;
		else
			goto out_finish;
	}

	tg->bps_conf[READ][index] = v[0];
	tg->bps_conf[WRITE][index] = v[1];
	tg->iops_conf[READ][index] = v[2];
	tg->iops_conf[WRITE][index] = v[3];

	if (index == LIMIT_MAX) {
		tg->bps[READ][index] = v[0];
		tg->bps[WRITE][index] = v[1];
		tg->iops[READ][index] = v[2];
		tg->iops[WRITE][index] = v[3];
	}
	tg->bps[READ][LIMIT_LOW] = min(tg->bps_conf[READ][LIMIT_LOW],
		tg->bps_conf[READ][LIMIT_MAX]);
	tg->bps[WRITE][LIMIT_LOW] = min(tg->bps_conf[WRITE][LIMIT_LOW],
		tg->bps_conf[WRITE][LIMIT_MAX]);
	tg->iops[READ][LIMIT_LOW] = min(tg->iops_conf[READ][LIMIT_LOW],
		tg->iops_conf[READ][LIMIT_MAX]);
	tg->iops[WRITE][LIMIT_LOW] = min(tg->iops_conf[WRITE][LIMIT_LOW],
		tg->iops_conf[WRITE][LIMIT_MAX]);
	tg->idletime_threshold_conf = idle_time;
	tg->latency_target_conf = latency_time;

	/* force user to configure all settings for low limit  */
	if (!(tg->bps[READ][LIMIT_LOW] || tg->iops[READ][LIMIT_LOW] ||
	      tg->bps[WRITE][LIMIT_LOW] || tg->iops[WRITE][LIMIT_LOW]) ||
	    tg->idletime_threshold_conf == DFL_IDLE_THRESHOLD ||
	    tg->latency_target_conf == DFL_LATENCY_TARGET) {
		tg->bps[READ][LIMIT_LOW] = 0;
		tg->bps[WRITE][LIMIT_LOW] = 0;
		tg->iops[READ][LIMIT_LOW] = 0;
		tg->iops[WRITE][LIMIT_LOW] = 0;
		tg->idletime_threshold = DFL_IDLE_THRESHOLD;
		tg->latency_target = DFL_LATENCY_TARGET;
	} else if (index == LIMIT_LOW) {
		tg->idletime_threshold = tg->idletime_threshold_conf;
		tg->latency_target = tg->latency_target_conf;
	}

	blk_throtl_update_limit_valid(tg->td);
	if (tg->td->limit_valid[LIMIT_LOW]) {
		if (index == LIMIT_LOW)
			tg->td->limit_index = LIMIT_LOW;
	} else
		tg->td->limit_index = LIMIT_MAX;
	tg_conf_updated(tg, index == LIMIT_LOW &&
		tg->td->limit_valid[LIMIT_LOW]);
	ret = 0;
out_finish:
	blkg_conf_finish(&ctx);
	return ret ?: nbytes;
}

static struct cftype throtl_files[] = {
#ifdef CONFIG_BLK_DEV_THROTTLING_LOW
	{
		.name = "low",
		.flags = CFTYPE_NOT_ON_ROOT,
		.seq_show = tg_print_limit,
		.write = tg_set_limit,
		.private = LIMIT_LOW,
	},
#endif
	{
		.name = "max",
		.flags = CFTYPE_NOT_ON_ROOT,
		.seq_show = tg_print_limit,
		.write = tg_set_limit,
		.private = LIMIT_MAX,
	},
	{ }	/* terminate */
};

static void throtl_shutdown_wq(struct request_queue *q)
{
	struct throtl_data *td = q->td;

	del_timer_sync(&td->rescue_timer);
	del_timer_sync(&td->idle_timer);
	cancel_work_sync(&td->dispatch_work);
}

static struct blkcg_policy blkcg_policy_throtl = {
	.dfl_cftypes		= throtl_files,
	.legacy_cftypes		= throtl_legacy_files,

	.pd_alloc_fn		= throtl_pd_alloc,
	.pd_init_fn		= throtl_pd_init,
	.pd_online_fn		= throtl_pd_online,
	.pd_offline_fn		= throtl_pd_offline,
	.pd_free_fn		= throtl_pd_free,
};

static unsigned long __tg_last_low_overflow_time(struct throtl_grp *tg)
{
	unsigned long rtime = jiffies, wtime = jiffies;

	if (tg->bps[READ][LIMIT_LOW] || tg->iops[READ][LIMIT_LOW])
		rtime = tg->last_low_overflow_time[READ];
	if (tg->bps[WRITE][LIMIT_LOW] || tg->iops[WRITE][LIMIT_LOW])
		wtime = tg->last_low_overflow_time[WRITE];
	return min(rtime, wtime);
}

/* tg should not be an intermediate node */
static unsigned long tg_last_low_overflow_time(struct throtl_grp *tg)
{
	struct throtl_service_queue *parent_sq;
	struct throtl_grp *parent = tg;
	unsigned long ret = __tg_last_low_overflow_time(tg);

	while (true) {
		parent_sq = parent->service_queue.parent_sq;
		parent = sq_to_tg(parent_sq);
		if (!parent)
			break;

		/*
		 * The parent doesn't have low limit, it always reaches low
		 * limit. Its overflow time is useless for children
		 */
		if (!parent->bps[READ][LIMIT_LOW] &&
		    !parent->iops[READ][LIMIT_LOW] &&
		    !parent->bps[WRITE][LIMIT_LOW] &&
		    !parent->iops[WRITE][LIMIT_LOW])
			continue;
		if (time_after(__tg_last_low_overflow_time(parent), ret))
			ret = __tg_last_low_overflow_time(parent);
	}
	return ret;
}

static bool throtl_tg_is_idle(struct throtl_grp *tg)
{
	/*
	 * cgroup is idle if:
	 * - single idle is too long, longer than a fixed value (in case user
	 *   configure a too big threshold) or 4 times of idletime threshold
	 * - average think time is more than threshold
	 * - IO latency is largely below threshold
	 */
	unsigned long time;
	bool ret;

	time = min_t(unsigned long, MAX_IDLE_TIME, 4 * tg->idletime_threshold);
	ret = tg->latency_target == DFL_LATENCY_TARGET ||
	      tg->idletime_threshold == DFL_IDLE_THRESHOLD ||
	      (ktime_get_ns() >> 10) - tg->last_finish_time > time ||
	      tg->avg_idletime > tg->idletime_threshold ||
	      (tg->latency_target && tg->bio_cnt &&
		tg->bad_bio_cnt * 5 < tg->bio_cnt);
	throtl_log(&tg->service_queue,
		"avg_idle=%ld, idle_threshold=%ld, bad_bio=%d, total_bio=%d, is_idle=%d, scale=%d",
		tg->avg_idletime, tg->idletime_threshold, tg->bad_bio_cnt,
		tg->bio_cnt, ret, tg->td->scale);
	return ret;
}

static bool throtl_tg_can_upgrade(struct throtl_grp *tg)
{
	struct throtl_service_queue *sq = &tg->service_queue;
	bool read_limit, write_limit;

	/*
	 * if cgroup reaches low limit (if low limit is 0, the cgroup always
	 * reaches), it's ok to upgrade to next limit
	 */
	read_limit = tg->bps[READ][LIMIT_LOW] || tg->iops[READ][LIMIT_LOW];
	write_limit = tg->bps[WRITE][LIMIT_LOW] || tg->iops[WRITE][LIMIT_LOW];
	if (!read_limit && !write_limit)
		return true;
	if (read_limit && sq->nr_queued[READ] &&
	    (!write_limit || sq->nr_queued[WRITE]))
		return true;
	if (write_limit && sq->nr_queued[WRITE] &&
	    (!read_limit || sq->nr_queued[READ]))
		return true;

	if (time_after_eq(jiffies,
		tg_last_low_overflow_time(tg) + tg->td->throtl_slice) &&
	    throtl_tg_is_idle(tg))
		return true;
	return false;
}

static bool throtl_hierarchy_can_upgrade(struct throtl_grp *tg)
{
	while (true) {
		if (throtl_tg_can_upgrade(tg))
			return true;
		tg = sq_to_tg(tg->service_queue.parent_sq);
		if (!tg || !tg_to_blkg(tg)->parent)
			return false;
	}
	return false;
}

static bool throtl_can_upgrade(struct throtl_data *td,
	struct throtl_grp *this_tg)
{
	struct cgroup_subsys_state *pos_css;
	struct blkcg_gq *blkg;

	if (td->limit_index != LIMIT_LOW)
		return false;

	if (time_before(jiffies, td->low_downgrade_time + td->throtl_slice))
		return false;

	rcu_read_lock();
	blkg_for_each_descendant_post(blkg, pos_css, td->queue->root_blkg) {
		struct throtl_grp *tg = blkg_to_tg(blkg);

		if (tg == this_tg)
			continue;
		if (!list_empty(&tg_to_blkg(tg)->blkcg->css.children))
			continue;
		if (!throtl_hierarchy_can_upgrade(tg)) {
			rcu_read_unlock();
			return false;
		}
	}
	rcu_read_unlock();
	return true;
}

static void throtl_upgrade_check(struct throtl_grp *tg)
{
	unsigned long now = jiffies;

	if (tg->td->limit_index != LIMIT_LOW)
		return;

	if (time_after(tg->last_check_time + tg->td->throtl_slice, now))
		return;

	tg->last_check_time = now;

	if (!time_after_eq(now,
	     __tg_last_low_overflow_time(tg) + tg->td->throtl_slice))
		return;

	if (throtl_can_upgrade(tg->td, NULL))
		throtl_upgrade_state(tg->td);
}

static void throtl_upgrade_state(struct throtl_data *td)
{
	struct cgroup_subsys_state *pos_css;
	struct blkcg_gq *blkg;

	throtl_log(&td->service_queue, "upgrade to max");
	td->limit_index = LIMIT_MAX;
	td->low_upgrade_time = jiffies;
	td->scale = 0;
	rcu_read_lock();
	blkg_for_each_descendant_post(blkg, pos_css, td->queue->root_blkg) {
		struct throtl_grp *tg = blkg_to_tg(blkg);
		struct throtl_service_queue *sq = &tg->service_queue;

		tg->disptime = jiffies - 1;
		throtl_select_dispatch(sq);
		throtl_schedule_next_dispatch(sq, true);
	}
	rcu_read_unlock();
	throtl_select_dispatch(&td->service_queue);
	throtl_schedule_next_dispatch(&td->service_queue, true);
	queue_work(kthrotld_workqueue, &td->dispatch_work);
}

static void throtl_downgrade_state(struct throtl_data *td, int new)
{
	td->scale /= 2;

	throtl_log(&td->service_queue, "downgrade, scale %d", td->scale);
	if (td->scale) {
		td->low_upgrade_time = jiffies - td->scale * td->throtl_slice;
		return;
	}

	td->limit_index = new;
	td->low_downgrade_time = jiffies;
}

static bool throtl_tg_can_downgrade(struct throtl_grp *tg)
{
	struct throtl_data *td = tg->td;
	unsigned long now = jiffies;

	/*
	 * If cgroup is below low limit, consider downgrade and throttle other
	 * cgroups
	 */
	if (time_after_eq(now, td->low_upgrade_time + td->throtl_slice) &&
	    time_after_eq(now, tg_last_low_overflow_time(tg) +
					td->throtl_slice) &&
	    (!throtl_tg_is_idle(tg) ||
	     !list_empty(&tg_to_blkg(tg)->blkcg->css.children)))
		return true;
	return false;
}

static bool throtl_hierarchy_can_downgrade(struct throtl_grp *tg)
{
	while (true) {
		if (!throtl_tg_can_downgrade(tg))
			return false;
		tg = sq_to_tg(tg->service_queue.parent_sq);
		if (!tg || !tg_to_blkg(tg)->parent)
			break;
	}
	return true;
}

static void throtl_downgrade_check(struct throtl_grp *tg)
{
	uint64_t bps;
	unsigned int iops;
	unsigned long elapsed_time;
	unsigned long now = jiffies;

	if (tg->td->limit_index != LIMIT_MAX ||
	    !tg->td->limit_valid[LIMIT_LOW])
		return;
	if (!list_empty(&tg_to_blkg(tg)->blkcg->css.children))
		return;
	if (time_after(tg->last_check_time + tg->td->throtl_slice, now))
		return;

	elapsed_time = now - tg->last_check_time;
	tg->last_check_time = now;

	if (time_before(now, tg_last_low_overflow_time(tg) +
			tg->td->throtl_slice))
		return;

	if (tg->bps[READ][LIMIT_LOW]) {
		bps = tg->last_bytes_disp[READ] * HZ;
		do_div(bps, elapsed_time);
		if (bps >= tg->bps[READ][LIMIT_LOW])
			tg->last_low_overflow_time[READ] = now;
	}

	if (tg->bps[WRITE][LIMIT_LOW]) {
		bps = tg->last_bytes_disp[WRITE] * HZ;
		do_div(bps, elapsed_time);
		if (bps >= tg->bps[WRITE][LIMIT_LOW])
			tg->last_low_overflow_time[WRITE] = now;
	}

	if (tg->iops[READ][LIMIT_LOW]) {
		iops = tg->last_io_disp[READ] * HZ / elapsed_time;
		if (iops >= tg->iops[READ][LIMIT_LOW])
			tg->last_low_overflow_time[READ] = now;
	}

	if (tg->iops[WRITE][LIMIT_LOW]) {
		iops = tg->last_io_disp[WRITE] * HZ / elapsed_time;
		if (iops >= tg->iops[WRITE][LIMIT_LOW])
			tg->last_low_overflow_time[WRITE] = now;
	}

	/*
	 * If cgroup is below low limit, consider downgrade and throttle other
	 * cgroups
	 */
	if (throtl_hierarchy_can_downgrade(tg))
		throtl_downgrade_state(tg->td, LIMIT_LOW);

	tg->last_bytes_disp[READ] = 0;
	tg->last_bytes_disp[WRITE] = 0;
	tg->last_io_disp[READ] = 0;
	tg->last_io_disp[WRITE] = 0;
}

static void blk_throtl_update_idletime(struct throtl_grp *tg)
{
	unsigned long now = ktime_get_ns() >> 10;
	unsigned long last_finish_time = tg->last_finish_time;

	if (now <= last_finish_time || last_finish_time == 0 ||
	    last_finish_time == tg->checked_last_finish_time)
		return;

	tg->avg_idletime = (tg->avg_idletime * 7 + now - last_finish_time) >> 3;
	tg->checked_last_finish_time = last_finish_time;
}

#ifdef CONFIG_BLK_DEV_THROTTLING_LOW
static void throtl_update_latency_buckets(struct throtl_data *td)
{
	struct avg_latency_bucket avg_latency[LATENCY_BUCKET_SIZE];
	int i, cpu;
	unsigned long last_latency = 0;
	unsigned long latency;

	if (!blk_queue_nonrot(td->queue))
		return;
	if (time_before(jiffies, td->last_calculate_time + HZ))
		return;
	td->last_calculate_time = jiffies;

	memset(avg_latency, 0, sizeof(avg_latency));
	for (i = 0; i < LATENCY_BUCKET_SIZE; i++) {
		struct latency_bucket *tmp = &td->tmp_buckets[i];

		for_each_possible_cpu(cpu) {
			struct latency_bucket *bucket;

			/* this isn't race free, but ok in practice */
			bucket = per_cpu_ptr(td->latency_buckets, cpu);
			tmp->total_latency += bucket[i].total_latency;
			tmp->samples += bucket[i].samples;
			bucket[i].total_latency = 0;
			bucket[i].samples = 0;
		}

		if (tmp->samples >= 32) {
			int samples = tmp->samples;

			latency = tmp->total_latency;

			tmp->total_latency = 0;
			tmp->samples = 0;
			latency /= samples;
			if (latency == 0)
				continue;
			avg_latency[i].latency = latency;
		}
	}

	for (i = 0; i < LATENCY_BUCKET_SIZE; i++) {
		if (!avg_latency[i].latency) {
			if (td->avg_buckets[i].latency < last_latency)
				td->avg_buckets[i].latency = last_latency;
			continue;
		}

		if (!td->avg_buckets[i].valid)
			latency = avg_latency[i].latency;
		else
			latency = (td->avg_buckets[i].latency * 7 +
				avg_latency[i].latency) >> 3;

		td->avg_buckets[i].latency = max(latency, last_latency);
		td->avg_buckets[i].valid = true;
		last_latency = td->avg_buckets[i].latency;
	}

	for (i = 0; i < LATENCY_BUCKET_SIZE; i++)
		throtl_log(&td->service_queue,
			"Latency bucket %d: latency=%ld, valid=%d", i,
			td->avg_buckets[i].latency, td->avg_buckets[i].valid);
}
#else
static inline void throtl_update_latency_buckets(struct throtl_data *td)
{
}
#endif

static void blk_throtl_assoc_bio(struct throtl_grp *tg, struct bio *bio)
{
#ifdef CONFIG_BLK_DEV_THROTTLING_LOW
	if (bio->bi_css)
		bio->bi_cg_private = tg;
	blk_stat_set_issue(&bio->bi_issue_stat, bio_sectors(bio));
#endif
}

bool blk_throtl_bio(struct request_queue *q, struct blkcg_gq *blkg,
		    struct bio *bio)
{
	struct throtl_qnode *qn = NULL;
	struct throtl_grp *tg = blkg_to_tg(blkg ?: q->root_blkg);
	struct throtl_service_queue *sq;
	bool rw = bio_data_dir(bio);
	int index;
	bool throttled = false;
	struct throtl_data *td = tg->td;
	struct blkcg *blkcg;

	WARN_ON_ONCE(!rcu_read_lock_held());

	/* see throtl_charge_bio() */
	if (bio_flagged(bio, BIO_THROTTLED))
		goto out;

	if (bio_op(bio) == REQ_OP_DISCARD)
		goto out;

	if ((bio_op(bio) == REQ_OP_WRITE) && !(bio->bi_opf & REQ_SYNC))
		goto out;

again:
	if (!blkg)
		goto out;

	index = tg_data_index(tg, rw);
	if (!tg->has_rules[index])
		goto out;

	blkcg = blkg->blkcg;
	if (td_weight_based(td) &&
	    (blkcg == &blkcg_root || !blk_throtl_weight_offon))
		goto out;

#if defined(CONFIG_HUAWEI_QOS_BLKIO) || defined(CONFIG_ROW_VIP_QUEUE)
	if (!blk_queue_qos_on(q)) {
		if (blkcg->type <= BLK_THROTL_FG)
			bio->bi_opf |= REQ_FG;
	}
#else
	if (blkcg->type <= BLK_THROTL_FG)
		bio->bi_opf |= REQ_FG;
#endif

	if (bio->bi_opf & (REQ_META | REQ_PREFLUSH | REQ_FUA))
		goto out;

	/* If we are on the way of memory reclaim, skip throttle */
	if (current->flags & (PF_MEMALLOC | PF_SWAPWRITE | PF_KSWAPD))
		goto out;

	if (task_in_pagefault(current))
		goto out;

	if (td_weight_based(td) &&
	    (blk_throtl_weight_offon == BLK_THROTL_WEIGHT_ON_FS))
		goto out;

	/*
	 * Now we just support limit control for one layer.
	 */
	if (ACCESS_ONCE(tg->td->max_inflights) && !bio->bi_throtl_private1 &&
	    current->mm &&
	    ACCESS_ONCE(current->mm->io_limit->max_inflights)) {
		struct blk_throtl_io_limit *io_limit = current->mm->io_limit;
		int ret;

		blkg_get(tg_to_blkg(tg));
		ret = blk_throtl_io_limit_wait(blkcg, io_limit, tg, bio);
		blkg_put(tg_to_blkg(tg));
		if (ret == 1)
			goto again;
		else if (ret == 2)
			goto skip;

		blk_throtl_io_limit_get(io_limit);
		bio->bi_throtl_private1 = io_limit;
		bio->bi_throtl_end_io1 = blk_throtl_limit_io_done;
	}
skip:
	spin_lock_irq(q->queue_lock);

	throtl_update_latency_buckets(td);

	if (unlikely(blk_queue_bypass(q)))
		goto out_unlock;

	if (!(tg->flags & THROTL_TG_ONLINE))
		goto out_unlock;

	if (unlikely(blk_throtl_weight_offon != BLK_THROTL_WEIGHT_ON_BLK))
		goto out_unlock;

	if (td_weight_based(td)) {
		struct bio_list bios;

		/*
		 * Now we just support weight based throttle for
		 * one layer.
		 */
		if (bio->bi_throtl_end_io2)
			goto out_unlock;

		bio_list_init(&bios);

		throttled = true;
		bio_associate_current(bio);
		throtl_add_bio_weight(bio, tg);
		blk_run_throtl_bios_weight(td, tg, &bios);
		spin_unlock_irq(q->queue_lock);
		rcu_read_unlock();

		while((bio = bio_list_pop(&bios)))
			generic_make_request(bio);

		rcu_read_lock();
		return throttled;
	}

	blk_throtl_assoc_bio(tg, bio);
	blk_throtl_update_idletime(tg);

	sq = &tg->service_queue;

	while (true) {
		if (tg->last_low_overflow_time[rw] == 0)
			tg->last_low_overflow_time[rw] = jiffies;
		throtl_downgrade_check(tg);
		throtl_upgrade_check(tg);
		/* throtl is FIFO - if bios are already queued, should queue */
		if (sq->nr_queued[rw])
			break;

		/* if above limits, break to queue */
		if (!tg_may_dispatch(tg, bio, NULL)) {
			tg->last_low_overflow_time[rw] = jiffies;
			if (throtl_can_upgrade(td, tg)) {
				throtl_upgrade_state(td);
				goto again;
			}
			break;
		}

		/* within limits, let's charge and dispatch directly */
		throtl_charge_bio(tg, bio);

		/*
		 * We need to trim slice even when bios are not being queued
		 * otherwise it might happen that a bio is not queued for
		 * a long time and slice keeps on extending and trim is not
		 * called for a long time. Now if limits are reduced suddenly
		 * we take into account all the IO dispatched so far at new
		 * low rate and * newly queued IO gets a really long dispatch
		 * time.
		 *
		 * So keep on trimming slice even if bio is not queued.
		 */
		throtl_trim_slice(tg, rw);

		/*
		 * @bio passed through this layer without being throttled.
		 * Climb up the ladder.  If we''re already at the top, it
		 * can be executed directly.
		 */
		qn = &tg->qnode_on_parent[index];
		sq = sq->parent_sq;
		tg = sq_to_tg(sq);
		if (!tg)
			goto out_unlock;
	}

	/* out-of-limit, queue to @tg */
	throtl_log(sq, "[%c] bio. bdisp=%llu sz=%u bps=%llu iodisp=%u iops=%u queued=%d/%d",
		   rw == READ ? 'R' : 'W',
		   tg->bytes_disp[rw], bio->bi_iter.bi_size,
		   tg_bps_limit(tg, rw),
		   tg->io_disp[rw], tg_iops_limit(tg, rw),
		   sq->nr_queued[READ], sq->nr_queued[WRITE]);

	tg->last_low_overflow_time[rw] = jiffies;

	td->nr_queued[rw]++;
	throtl_add_bio_tg(bio, qn, tg);
	throttled = true;

	/*
	 * Update @tg's dispatch time and force schedule dispatch if @tg
	 * was empty before @bio.  The forced scheduling isn't likely to
	 * cause undue delay as @bio is likely to be dispatched directly if
	 * its @tg's disptime is not in the future.
	 */
	if (tg->flags & THROTL_TG_WAS_EMPTY) {
		tg_update_disptime(tg);
		throtl_schedule_next_dispatch(tg->service_queue.parent_sq, true);
	}

out_unlock:
	spin_unlock_irq(q->queue_lock);
out:
	bio_set_flag(bio, BIO_THROTTLED);

#ifdef CONFIG_BLK_DEV_THROTTLING_LOW
	if (throttled || !td->track_bio_latency)
		bio->bi_issue_stat.stat |= SKIP_LATENCY;
#endif
	return throttled;
}

static inline void blk_throtl_update_quota(struct throtl_grp *tg,
					   unsigned int size)
{
	tg->intime = jiffies;
	tg->bytes_disp[0] += size;
	tg->io_disp[0]++;
}

bool blk_throtl_get_quota(struct block_device *bdev, unsigned int size,
			  unsigned long jiffies_time_out, bool wait)
{
	struct request_queue *q;
	struct throtl_data *td;
	struct throtl_grp *tg = NULL;
	struct blkcg *blkcg = NULL;
	struct blkcg *new_blkcg;
	unsigned long start_jiffies;
	unsigned long wait_jiffies = jiffies_time_out;
	int ret;

	if (current->flags & (PF_MEMALLOC | PF_SWAPWRITE |
			      PF_KSWAPD | PF_MUTEX_GC))
		return true;

	/*
	 * bdev has been got at mount(), and will be freed at kill_sb() after
	 * all files closed and all dirty pages flushed. So it's safe to
	 * using bdev without lock here.
	 */
	if (unlikely(!bdev) || unlikely(!bdev->bd_disk))
		return true;

	q = bdev_get_queue(bdev);
	if (unlikely(!q))
		return true;

	if (q->tz_weighted_bdev)
		q = bdev_get_queue(q->tz_weighted_bdev);

	td = q->td;

	start_jiffies = jiffies;
get_new_group:
	/*
	 * A throtl_grp pointer retrieved under rcu can be used to access
	 * basic fields.
	 */
	rcu_read_lock();

	if (!td_weight_based(td))
		goto out_unlock_rcu;

	if (blk_throtl_weight_offon != BLK_THROTL_WEIGHT_ON_FS)
		goto out_unlock_rcu;

	new_blkcg = task_blkcg(current);
	if (new_blkcg == &blkcg_root)
		goto out_unlock_rcu;
	if (!blkcg || blkcg != new_blkcg) {
		struct blkcg_gq *blkg;

		blkcg = new_blkcg;

		blkg = blkg_lookup(blkcg, q);
		if (unlikely(!blkg)) {
			spin_lock_irq(q->queue_lock);
			blkg = blkg_lookup_create(blkcg, q);
			if (IS_ERR(blkg))
				blkg = q->root_blkg;
			spin_unlock_irq(q->queue_lock);
		}

		tg = blkg_to_tg(blkg);
	}

	if(unlikely(!tg))
		goto out_unlock_rcu;

	spin_lock_irq(q->queue_lock);

	if (unlikely(blk_queue_bypass(q)))
		goto out_unlock;

	if (unlikely(!(tg->flags & THROTL_TG_ONLINE)))
		goto out_unlock;

	if (unlikely(!td_weight_based(td)))
		goto out_unlock;

	if (unlikely(blk_throtl_weight_offon != BLK_THROTL_WEIGHT_ON_FS))
		goto out_unlock;
retry:
	if (tg_may_dispatch_weight(tg)) {
		blk_throtl_update_quota(tg, size);
		if (!list_empty(&tg->node))
			goto out_unlock;
		blkg_get(tg_to_blkg(tg));
		list_add_tail(&tg->node, &td->active);
		goto out_unlock;
	} else if (!wait) {
		spin_unlock_irq(q->queue_lock);
		rcu_read_unlock();
		return false;
	}

	if (!tg->expired) {
		tg->expired = true;
		list_move_tail(&tg->node, &td->expired);

		if (list_empty(&td->active))
			goto start_new_slice;
	}

	/*
	 * If jiffies_time_out expires, just return to continue
	 * to read/write.
	 */
	wait_jiffies = jiffies_time_out - (jiffies - start_jiffies);
	if (jiffies - start_jiffies >= jiffies_time_out)
		goto out_unlock;

	if (should_start_new_slice(td))
		goto start_new_slice;

	blkg_get(tg_to_blkg(tg));
	spin_unlock_irq(q->queue_lock);
	rcu_read_unlock();

	/* Waiting for starting new slice, max waiting time is 20ms */
	ret = wait_event_interruptible_timeout(tg->wait, !tg->expired,
			wait_jiffies >
			msecs_to_jiffies(THROTL_IDLE_INTERVAL / 2) ?
			msecs_to_jiffies(THROTL_IDLE_INTERVAL / 2) :
			wait_jiffies);

	blkg_put(tg_to_blkg(tg));

	/* If this task is interrupted, just return */
	if (signal_pending_state(TASK_INTERRUPTIBLE, current))
		return true;

	/* this task may switch to other group, get tg again */
	goto get_new_group;

start_new_slice:
	/* Start new slice and wake up waiting tgs */
	throtl_start_new_slice_weight(td);
	goto retry;

out_unlock:
	spin_unlock_irq(q->queue_lock);

out_unlock_rcu:
	rcu_read_unlock();
	return true;
}

#ifdef CONFIG_BLK_DEV_THROTTLING_LOW
static void throtl_track_latency(struct throtl_data *td, sector_t size,
	int op, unsigned long time)
{
	struct latency_bucket *latency;
	int index;

	if (!td || td->limit_index != LIMIT_LOW || op != REQ_OP_READ ||
	    !blk_queue_nonrot(td->queue))
		return;

	index = request_bucket_index(size);

	latency = get_cpu_ptr(td->latency_buckets);
	latency[index].total_latency += time;
	latency[index].samples++;
	put_cpu_ptr(td->latency_buckets);
}

void blk_throtl_stat_add(struct request *rq, u64 time_ns)
{
	struct request_queue *q = rq->q;
	struct throtl_data *td = q->td;

	throtl_track_latency(td, blk_stat_size(&rq->issue_stat),
		req_op(rq), time_ns >> 10);
}

void blk_throtl_bio_endio(struct bio *bio)
{
	struct throtl_grp *tg;
	u64 finish_time_ns;
	unsigned long finish_time;
	unsigned long start_time;
	unsigned long lat;

	tg = bio->bi_cg_private;
	if (!tg)
		return;
	bio->bi_cg_private = NULL;

	finish_time_ns = ktime_get_ns();
	tg->last_finish_time = finish_time_ns >> 10;

	start_time = blk_stat_time(&bio->bi_issue_stat) >> 10;
	finish_time = __blk_stat_time(finish_time_ns) >> 10;
	if (!start_time || finish_time <= start_time)
		return;

	lat = finish_time - start_time;
	/* this is only for bio based driver */
	if (!(bio->bi_issue_stat.stat & SKIP_LATENCY))
		throtl_track_latency(tg->td, blk_stat_size(&bio->bi_issue_stat),
			bio_op(bio), lat);

	if (tg->latency_target && lat >= tg->td->filtered_latency) {
		int bucket;
		unsigned int threshold;

		bucket = request_bucket_index(
			blk_stat_size(&bio->bi_issue_stat));
		threshold = tg->td->avg_buckets[bucket].latency +
			tg->latency_target;
		if (lat > threshold)
			tg->bad_bio_cnt++;
		/*
		 * Not race free, could get wrong count, which means cgroups
		 * will be throttled
		 */
		tg->bio_cnt++;
	}

	if (time_after(jiffies, tg->bio_cnt_reset_time) || tg->bio_cnt > 1024) {
		tg->bio_cnt_reset_time = tg->td->throtl_slice + jiffies;
		tg->bio_cnt /= 2;
		tg->bad_bio_cnt /= 2;
	}
}
#endif

/*
 * Dispatch all bios from all children tg's queued on @parent_sq.  On
 * return, @parent_sq is guaranteed to not have any active children tg's
 * and all bios from previously active tg's are on @parent_sq->bio_lists[].
 */
static void tg_drain_bios(struct throtl_service_queue *parent_sq)
{
	struct throtl_grp *tg;

	while ((tg = throtl_rb_first(parent_sq))) {
		struct throtl_service_queue *sq = &tg->service_queue;
		struct bio *bio;

		throtl_dequeue_tg(tg);

		while ((bio = throtl_peek_queued(&sq->queued[READ])))
			tg_dispatch_one_bio(tg, bio_data_dir(bio));
		while ((bio = throtl_peek_queued(&sq->queued[WRITE])))
			tg_dispatch_one_bio(tg, bio_data_dir(bio));
	}
}

/**
 * blk_throtl_drain - drain throttled bios
 * @q: request_queue to drain throttled bios for
 *
 * Dispatch all currently throttled bios on @q through ->make_request_fn().
 */
void blk_throtl_drain(struct request_queue *q)
	__releases(q->queue_lock) __acquires(q->queue_lock)
{
	struct throtl_data *td = q->td;
	struct blkcg_gq *blkg;
	struct cgroup_subsys_state *pos_css;
	struct bio *bio;
	int i;

	if (td_weight_based(td)) {
		if (blk_throtl_weight_offon == BLK_THROTL_WEIGHT_ON_FS)
			__blk_throtl_drain_weight(q);
		else
			blk_throtl_drain_weight(q);
		return;
	}

	queue_lockdep_assert_held(q);
	rcu_read_lock();

	/*
	 * Drain each tg while doing post-order walk on the blkg tree, so
	 * that all bios are propagated to td->service_queue.  It'd be
	 * better to walk service_queue tree directly but blkg walk is
	 * easier.
	 */
	blkg_for_each_descendant_post(blkg, pos_css, td->queue->root_blkg)
		tg_drain_bios(&blkg_to_tg(blkg)->service_queue);

	/* finally, transfer bios from top-level tg's into the td */
	tg_drain_bios(&td->service_queue);

	rcu_read_unlock();
	spin_unlock_irq(q->queue_lock);

	/* all bios now should be in td->service_queue, issue them */
	for (i = READ; i <= WRITE; i++)
		while ((bio = throtl_pop_queued(&td->service_queue.queued[i],
						NULL)))
			generic_make_request(bio);

	spin_lock_irq(q->queue_lock);
}

int blk_throtl_init(struct request_queue *q)
{
	struct throtl_data *td;
	int ret;

	td = kzalloc_node(sizeof(*td), GFP_KERNEL, q->node);
	if (!td)
		return -ENOMEM;
	td->latency_buckets = __alloc_percpu(sizeof(struct latency_bucket) *
		LATENCY_BUCKET_SIZE, __alignof__(u64));
	if (!td->latency_buckets) {
		kfree(td);
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&td->active);
	INIT_LIST_HEAD(&td->expired);

	atomic_set(&td->inflights, 0);
	init_waitqueue_head(&td->waitq);
	setup_timer(&td->rescue_timer, blk_throtl_rescue_timer_fn,
		    (unsigned long)td);
	setup_timer(&td->idle_timer, blk_throtl_idle_timer_fn,
		    (unsigned long)td);
	INIT_WORK(&td->dispatch_work, blk_throtl_dispatch_work_fn);
	throtl_service_queue_init(&td->service_queue);
	td->service_queue.share = MAX_SHARE;
	td->mode = MODE_NONE;

	td->quantum = QUANTUM_INQUEUE_DEF;
	td->bw_slice = BW_SLICE_DEF;
	td->iops_slice = IOPS_SLICE_DEF;

	q->tz_weighted_bdev = NULL;

	q->td = td;
	td->queue = q;

	td->limit_valid[LIMIT_MAX] = true;
	td->limit_index = LIMIT_MAX;
	td->low_upgrade_time = jiffies;
	td->low_downgrade_time = jiffies;

	/* activate policy */
	ret = blkcg_activate_policy(q, &blkcg_policy_throtl);
	if (ret) {
		free_percpu(td->latency_buckets);
		kfree(td);
	}
	return ret;
}

void blk_throtl_exit(struct request_queue *q)
{
	BUG_ON(!q->td);
	throtl_shutdown_wq(q);
	blkcg_deactivate_policy(q, &blkcg_policy_throtl);
	free_percpu(q->td->latency_buckets);
	kfree(q->td);
}

void blk_throtl_register_queue(struct request_queue *q)
{
	struct throtl_data *td;
	int i;

	td = q->td;
	BUG_ON(!td);

	if (blk_queue_nonrot(q)) {
		td->throtl_slice = DFL_THROTL_SLICE_SSD;
		td->filtered_latency = LATENCY_FILTERED_SSD;
	} else {
		td->throtl_slice = DFL_THROTL_SLICE_HD;
		td->filtered_latency = LATENCY_FILTERED_HD;
		for (i = 0; i < LATENCY_BUCKET_SIZE; i++)
			td->avg_buckets[i].latency = DFL_HD_BASELINE_LATENCY;
	}
#ifndef CONFIG_BLK_DEV_THROTTLING_LOW
	/* if no low limit, use previous default */
	td->throtl_slice = DFL_THROTL_SLICE_HD;
#endif

	td->track_bio_latency = !q->mq_ops && !q->request_fn;
	if (!td->track_bio_latency)
		blk_stat_enable_accounting(q);
}

#ifdef CONFIG_BLK_DEV_THROTTLING_LOW
ssize_t blk_throtl_sample_time_show(struct request_queue *q, char *page)
{
	if (!q->td)
		return -EINVAL;
	return sprintf(page, "%u\n", jiffies_to_msecs(q->td->throtl_slice));
}

ssize_t blk_throtl_sample_time_store(struct request_queue *q,
	const char *page, size_t count)
{
	unsigned long v;
	unsigned long t;

	if (!q->td)
		return -EINVAL;
	if (kstrtoul(page, 10, &v))
		return -EINVAL;
	t = msecs_to_jiffies(v);
	if (t == 0 || t > MAX_THROTL_SLICE)
		return -EINVAL;
	q->td->throtl_slice = t;
	return count;
}
#endif

static int __init throtl_init(void)
{
	kthrotld_workqueue = alloc_workqueue("kthrotld", WQ_MEM_RECLAIM, 0);
	if (!kthrotld_workqueue)
		panic("Failed to create kthrotld\n");

	return blkcg_policy_register(&blkcg_policy_throtl);
}

module_init(throtl_init);
