#include <linux/sizes.h>
#include <linux/types.h>
#include <linux/spinlock_types.h>
#include <linux/err.h>
#include <linux/vmalloc.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include "slab.h"
#include "linux/hisi/mem_trace.h"

#define invalid_slub_type(type) (type != SLUB_ALLOC && type != SLUB_FREE)
#define SLUB_RINGBUF_LEN   SZ_8K

struct slub_ring_buf {
	int type;
	unsigned long *buf;/*save caller*/
	spinlock_t lock;/*protect ringbuf*/
	size_t size;
	unsigned long in;
	unsigned long out;
};

struct slub_rb_node {
	struct rb_node node;
	unsigned long caller;
	atomic_t ref;
};

struct slub_track {
	int type;/*kslubtrackd need type to distinct rbtree*/
	wait_queue_head_t slub_wait;
	struct mutex slub_mutex;/*protect rbtree*/
	struct task_struct *slub_ktrackd;
	struct rb_root slubrb;
};

static int slub_track_flag;
static struct slub_track slub_track[NR_SLUB_ID];
static struct slub_ring_buf srb_array[NR_SLUB_ID];
static struct kmem_cache *slub_track_cache;

static inline unsigned long get_ring_buf_len(struct slub_ring_buf *srb)
{
	return (srb->in - srb->out);
}

static inline int ring_buf_is_full(struct slub_ring_buf *srb)
{
	return (get_ring_buf_len(srb) == srb->size);
}

static inline int ring_buf_is_half(struct slub_ring_buf *srb)
{
	return (get_ring_buf_len(srb) >= (srb->size / 2));
}

static inline int ring_buf_is_empty(struct slub_ring_buf *srb)
{
	return (srb->in == srb->out);
}

static inline void __set_caller(struct slub_ring_buf *srb, unsigned long caller)
{
	size_t idx = srb->in & (srb->size - 1);

	/* if srb is not full,
	 * first put the data starting from srb->in to buffer end,
	 * then put the rest (if any) from the beginning of the buffer
	 * update the index, when overflow, turn to 0
	 */

	if (ring_buf_is_full(srb))
		return;
	if (idx < srb->size)
		*(srb->buf + idx) = caller;
	else
		*(srb->buf) = caller;
	srb->in++;
}

static inline int __get_caller(
	struct slub_ring_buf *srb, unsigned long *caller)
{
	size_t idx = srb->out & (srb->size - 1);

	if (ring_buf_is_empty(srb))
		return -ENOSPC;
	/*
	 * len: buf with data in it
	 * first get the data from srb->out until the end of the buffer
	 * then get the rest (if any) from the beginning of the buffer
	 * update the index, when overflow, turn to 0
	 */
	if (idx < srb->size)
		*caller = *(srb->buf + idx);
	else
		*caller = *(srb->buf);

	srb->out++;
	return 0;
}

static inline void set_caller(struct slub_ring_buf *srb,
	unsigned long caller, int type)
{
	if (slub_track_flag) {
		__set_caller(srb, caller);
		if (ring_buf_is_half(srb))
			wake_up(&slub_track[type].slub_wait);
	}
}

void set_free_track(unsigned long caller)
{
	unsigned long flags;
	struct slub_ring_buf *srb = &srb_array[SLUB_FREE];

	if (!slub_track_flag)
		return;

	spin_lock_irqsave(&srb->lock, flags);
	set_caller(srb, caller, SLUB_FREE);
	spin_unlock_irqrestore(&srb->lock, flags);
}

void set_alloc_track(unsigned long caller)
{
	unsigned long flags;
	struct slub_ring_buf *srb = &srb_array[SLUB_ALLOC];

	if (!slub_track_flag)
		return;
	spin_lock_irqsave(&srb->lock, flags);
	set_caller(srb, caller, SLUB_ALLOC);
	spin_unlock_irqrestore(&srb->lock, flags);
}

static void slub_add_node(unsigned long caller, int type)
{
	struct rb_node **p = &slub_track[type].slubrb.rb_node;
	struct rb_node *parent = NULL;
	struct slub_rb_node *entry = NULL;
	struct slub_rb_node *rbnode = NULL;

	while (*p) {
		parent = *p;
		entry = rb_entry(parent, struct slub_rb_node, node);

		if (caller < entry->caller)
			p = &(*p)->rb_left;
		else if (caller > entry->caller)
			p = &(*p)->rb_right;
		else {
			atomic_inc(&entry->ref);
			return;
		}
	}
	rbnode = kmem_cache_alloc(slub_track_cache, GFP_ATOMIC);
	if (rbnode) {
		rbnode->caller = caller;
		rbnode->ref.counter = 0;
		atomic_inc(&rbnode->ref);
		rb_link_node(&rbnode->node, parent, p);
		rb_insert_color(&rbnode->node, &slub_track[type].slubrb);
	}
}

static void slub_del_node(struct slub_rb_node *rbnode, int type)
{
	rb_erase(&rbnode->node, &slub_track[type].slubrb);
	kmem_cache_free(slub_track_cache, rbnode);
}

static size_t slub_read_node(struct hisi_stack_info *buf,
	const size_t len, int type)
{
	struct rb_node *n = NULL;
	struct slub_rb_node *vnode = NULL;
	size_t length = len;
	size_t cnt = 0;

	if (!buf)
		return 0;

	n = rb_first(&slub_track[type].slubrb);
	while (n && (length--)) {
		vnode = rb_entry(n, struct slub_rb_node, node);
		(buf + cnt)->caller = vnode->caller;
		(buf + cnt)->ref.counter = vnode->ref.counter;
		n = rb_next(n);
		slub_del_node(vnode, type);
		cnt++;
	}
	return cnt;
}

static void __slub_fetch_node(struct slub_ring_buf *srb, int type)
{
	unsigned long caller;
	int ret;

	ret = __get_caller(srb, &caller);
	if (ret)
		return;
	slub_add_node(caller, type);
}

/*move entry from ringbuf to rb tree */
static void slub_fetch_node(int slubtype)
{
	unsigned long i, len, flags;
	struct slub_ring_buf *srb = &srb_array[slubtype];

	spin_lock_irqsave(&srb->lock, flags);
	len = get_ring_buf_len(srb);
	for (i = 0; i < len; i++)
		__slub_fetch_node(srb, slubtype);
	spin_unlock_irqrestore(&srb->lock, flags);
}

static int kslubtrackd(void *p)
{
	DEFINE_WAIT(wait);/*lint !e446*/
	int type;
	struct slub_track *track = (struct slub_track *)p;

	type = track->type;
	while (!kthread_should_stop()) {
		prepare_to_wait(&slub_track[type].slub_wait,
			&wait, TASK_UNINTERRUPTIBLE);
		schedule();
		finish_wait(&slub_track[type].slub_wait, &wait);
		mutex_lock(&slub_track[type].slub_mutex);
		if (slub_track_flag)
			slub_fetch_node(type);
		mutex_unlock(&slub_track[type].slub_mutex);
	};
	return 0;
}

static inline void slub_build_rb_tree(int type)
{
	slub_track[type].slubrb = RB_ROOT;
}

static void slub_node_type_show(int type)
{
	struct rb_node *n = NULL;
	struct slub_rb_node *slub = NULL;

	mutex_lock(&slub_track[type].slub_mutex);
	for (n = rb_first(&slub_track[type].slubrb); n; n = rb_next(n)) {
		slub = rb_entry(n, struct slub_rb_node, node);
		if (slub)
			pr_err("caller:0x%lx, %pS, ref:0x%x\n",
			slub->caller, (void *)slub->caller, atomic_read(&slub->ref));
	}
	mutex_unlock(&slub_track[type].slub_mutex);
}

void slub_node_show(void)
{
	pr_err("slub_alloc info:\n");
	slub_node_type_show(SLUB_ALLOC);
	pr_err("slub_free info:\n");
	slub_node_type_show(SLUB_FREE);
	pr_err("%s end\n", __func__);
}

static long slub_create_track(int type)
{
	long err = 0;

	init_waitqueue_head(&slub_track[type].slub_wait);
	mutex_init(&slub_track[type].slub_mutex);
	slub_build_rb_tree(type);
	slub_track[type].type = type;
	slub_track[type].slub_ktrackd = kthread_run(kslubtrackd,
		&slub_track[type], "slub-%s",
		type == SLUB_ALLOC ? "alloc" : "free");
	if (IS_ERR(slub_track[type].slub_ktrackd)) {
		err = PTR_ERR(slub_track[type].slub_ktrackd);
		pr_err("%s: slub_ktrackd failed!\n", __func__);
		return err;
	}
	return 0;
}

static int __slub_create_ringbuf(int type, size_t size)
{
	void *buf = NULL;
	struct slub_ring_buf *srb = &srb_array[type];

	if (!is_power_of_2(size)) {
		pr_err("%s: size must be 2 power!\n", __func__);
		return -EINVAL;
	}

	if (!size) {
		pr_err("%s: size must be not zero!\n", __func__);
		return -EINVAL;
	}

	buf = vmalloc(size * sizeof(long));
	if (!buf) {
		pr_err("%s: memory alloc failed!\n", __func__);
		return -ENOMEM;
	}

	srb->in = 0;
	srb->out = 0;
	srb->buf = buf;
	srb->size = size;
	srb->type = type;
	spin_lock_init(&srb->lock);
	return 0;
}

static int slub_create_ringbuf(size_t size)
{
	int ret;

	ret =  __slub_create_ringbuf(SLUB_ALLOC, SLUB_RINGBUF_LEN);
	if (ret) {
		pr_err("%s,create alloc ringbuf failed\n", __func__);
		return -ENOMEM;
	}
	ret = __slub_create_ringbuf(SLUB_FREE, SLUB_RINGBUF_LEN);
	if (ret) {
		pr_err("%s,create free ringbuf failed\n", __func__);
		return -ENOMEM;
	}
	return 0;
}
static void __slub_del_ringbuf(int type)
{
	unsigned long flags;
	struct slub_ring_buf *srb = &srb_array[type];

	spin_lock_irqsave(&srb->lock, flags);
	if (srb->buf) {
		vfree(srb->buf);
		srb->buf = NULL;
	}
	spin_unlock_irqrestore(&srb->lock, flags);
}

static int slub_del_ringbuf(void)
{
	__slub_del_ringbuf(SLUB_ALLOC);
	__slub_del_ringbuf(SLUB_FREE);
	return 0;
}

/*on->off->open->read->close*/
int hisi_slub_track_on(char *name)
{
	int ret;
	int found = 0;
	struct kmem_cache *cachep = NULL;

	if (slub_track_flag)
		return 0;

	mutex_lock(&slab_mutex);
	list_for_each_entry(cachep, &slab_caches, list) {/*lint !e666 !e413*/
		if (slub_track_cache &&
			strncmp(cachep->name, slub_track_cache->name,
			strlen(cachep->name) + 1) == 0)/*slub track cache cannot track*/
			continue;

		if (strncmp(cachep->name, name,
			strlen(cachep->name) + 1) == 0) {
			cachep->flags |= SLAB_HISI_TRACE;
			found = 1;
			break;
		}
	}
	mutex_unlock(&slab_mutex);
	if (!found) {
		pr_err("%s, slub name[%s] not found\n", __func__, name);
		return -EINVAL;
	}

	ret = slub_create_ringbuf(SLUB_RINGBUF_LEN);
	if (ret) {
		pr_err("%s, create ringbuf failed\n", __func__);
		return -ENOMEM;
	}
	pr_err("%s, slub name[%s] ok\n", __func__, name);
	slub_track_flag = 1;
	return 0;
}

int hisi_slub_track_off(char *name)
{
	int found = 0;
	struct kmem_cache *cachep = NULL;

	if (!slub_track_flag)
		return 0;

	mutex_lock(&slab_mutex);
	list_for_each_entry(cachep, &slab_caches, list) {/*lint !e666 !e413*/
		if (strncmp(cachep->name, name,
			strlen(cachep->name) + 1) == 0) {
			cachep->flags &= ~SLAB_HISI_TRACE;
			found = 1;
			break;
		}
	}
	mutex_unlock(&slab_mutex);
	if (!found) {
		pr_err("%s, slub name[%s] not found\n", __func__, name);
		return -EINVAL;
	}
	slub_track_flag = 0;
	slub_fetch_node(SLUB_ALLOC);
	slub_fetch_node(SLUB_FREE);
	slub_del_ringbuf();
	pr_err("%s, slub name[%s] ok\n", __func__, name);
	return 0;
}

size_t hisi_slub_stack_read(struct hisi_stack_info *buf, size_t len, int type)
{
	size_t cnt;
	unsigned long flags;
	struct slub_ring_buf *srb = NULL;

	if (invalid_slub_type(type))
		return 0;
	srb = &srb_array[type];
	spin_lock_irqsave(&srb->lock, flags);
	cnt = slub_read_node(buf, len, type);
	spin_unlock_irqrestore(&srb->lock, flags);
	pr_err("%s, ok\n", __func__);
	return cnt;
}

static int __init slub_track_init(void)
{
	slub_track_cache = kmem_cache_create("slub_track_cache",
			sizeof(struct slub_rb_node), 0, SLAB_HISI_NOTRACE, NULL);
	if (!slub_track_cache) {
		pr_err("%s: alloc failed!\n", __func__);
		return -EINVAL;
	}
	slub_create_track(SLUB_ALLOC);
	slub_create_track(SLUB_FREE);
	return 0;
}

late_initcall(slub_track_init);
