/* Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2020. All rights reserved.
 * FileName: vltmm.c
 * Description: This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;
 * either version 2 of the License,
 * or (at your option) any later version.
 */

#define pr_fmt(fmt) "secmem: " fmt

#include <linux/cma.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dma-contiguous.h>
#include <linux/err.h>
#include <linux/genalloc.h>
#include <linux/kthread.h>
#include <linux/memblock.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#include <linux/of_fdt.h>
#include <linux/of_reserved_mem.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/sizes.h>
#include <linux/workqueue.h>
#include <linux/swap.h>

#include <asm/cacheflush.h>
#include <asm/tlbflush.h>
#include <securec.h>

#include "vltmm.h"
#include "hisi_ion.h"
#include "mm_ion_priv.h"
#include "ion_tee_op.h"
#include "sec_alloc.h"
#include "vltmm_agent.h"

static struct smem_heap smemheaps[SMEM_HEAPS_MAX] = {{0}};
static struct smem_free_task smem_async_free = {0};

static int ta_init = 0;
static TEEC_Context *context = NULL;
static TEEC_Session *session = NULL;

static u64 smem_parse_dt_node(unsigned long node,
				const char *name, u64 default_value)
{
	u64 *val = NULL;

	val = (u64 *)of_get_flat_dt_prop(node, name, NULL);
	if (val)
		return be64_to_cpu(*val);
	else
		return default_value;
}

int smemheap_setup(struct reserved_mem *rmem, struct cma *cma)
{
	phys_addr_t align;
	phys_addr_t mask;
	struct smem_heap smemheap;
	unsigned long node;
	const char *smemheap_name = NULL;

	if (!rmem || !cma) {
		pr_err("%s input is null\n", __func__);
		return -EINVAL;
	}

	node = rmem->fdt_node;
	smemheap_name = (const char *)of_get_flat_dt_prop(node, "compatible", NULL);
	if (!smemheap_name) {
		pr_err("%s can't get compatible\n", __func__);
		return -EINVAL;
	}

	/* only smemheap-cma can setup smemheap */
	if (strncmp(smemheap_name, "smemheap-cma", strlen("smemheap-cma")) != 0)
		return 0;

	memset_s(&smemheap, sizeof(smemheap), 0, sizeof(smemheap));

	smemheap.gran = smem_parse_dt_node(node, "subbit-align", SZ_2M);

	align = smemheap.gran * SMEM_GRAN_CNT;
	mask = align - 1;

	smemheap.cma = cma;

	smemheap.min =
		smem_parse_dt_node(node, "water-mark-min", SMEM_DEFAULT_MIN);
	smemheap.low =
		smem_parse_dt_node(node, "water-mark-low", SMEM_DEFAULT_LOW);
	smemheap.high =
		smem_parse_dt_node(node, "water-mark-high", SMEM_DEFAULT_HIGH);

	smemheap.sid = smem_parse_dt_node(node, "sid", SMEM_SECOS_2M);
	smemheap.sid %= SMEM_HEAPS_MAX;

	smemheap.escape_flag = smem_parse_dt_node(node, "escape-flag", 0);

	smemheap.avail = 0;
	smemheap.max = rmem->size / smemheap.gran;

	smemheaps[smemheap.sid] = smemheap;

	pr_err("cma base: %llx, size: %llx\n"
		"\t\t gran: %llx min: %llx low: %llx high: %llx max: %llx\n"
		"\t\t sid: %llx escape-flag: %llu\n",
		rmem->base, rmem->size,
		smemheap.gran, smemheap.min, smemheap.low, smemheap.high,
		smemheap.max, smemheap.sid, smemheap.escape_flag);

	return 0;
}

void tick_the_thread(int async, u64 sid)
{
	pr_info("tick thread %d\n", async);
	if (async == ASYNC_ALLOC) {
		atomic_set(&smemheaps[sid].wait_flag, 1);
		wake_up_interruptible(&smemheaps[sid].alloc_wq);
	} else {
		atomic_set(&smem_async_free.wait_flag, 1);
		wake_up_interruptible(&smem_async_free.free_wq);
	}
}

static void __smem_config_mem_attr(phys_addr_t phy,
				unsigned long addr, unsigned long size)
{
#ifdef CONFIG_NEED_CHANGE_MAPPING
	/*
	 * Before set memory in secure region, we need change kernel
	 * pgtable from normal to device to avoid big CPU Speculative
	 * read.
	 */
	change_secpage_range(phy, addr, size, __pgprot(PROT_DEVICE_nGnRE));
	flush_tlb_all();
#endif

	__dma_unmap_area((void *)(uintptr_t)addr, size, DMA_FROM_DEVICE);
}

static int __smem_alloc_size_check(uint32_t sid, uint32_t num)
{
	struct smem_heap *p = NULL;

	p = &smemheaps[sid];

	if (num > p->max)
		return -EINVAL;

	if (num + p->used > p->high) {
		pr_err("alloc check failed, num: %u, used: %llu, high: %llu\n",
				num, p->used, p->high);
		return -EINVAL;
	}

	return 0;
}

int __smem_allocator_msg_alloc(uint32_t sid, phys_addr_t *data, uint32_t len, uint32_t num)
{
	phys_addr_t *pphy = NULL;
	struct page *pg = NULL;
	struct smem_heap *p = NULL;
	int ret = -ENOMEM;

	if ((sid >= SMEM_HEAPS_MAX) || !num || !data)
		return -EINVAL;

	p = &smemheaps[sid];

	if (!p->cma)
		return -ENOMEM;
	pr_info("into smem allocator alloc, num: %u\n", num);

	mutex_lock(&p->mutex);
	pr_info("alloc begin, num: %u\n", num);
	if (__smem_alloc_size_check(sid, num))
		goto end;
	if (num > p->avail) {
		int n = num - p->avail;

		while (n--) {
			pg = cma_alloc(p->cma, p->gran >> PAGE_SHIFT,
					(u32)get_order((u32)p->gran), GFP_KERNEL);
			if (!pg) {
				pr_err("cma alloc failed, avail:%llu used: %llu num: %u\n",
					p->avail, p->used, num);
				goto end;
			}
			__smem_config_mem_attr(page_to_phys(pg),
				(unsigned long)(uintptr_t)page_address(pg), p->gran);
			p->phy_array[p->avail++] = page_to_phys(pg);
		}
	}

	p->avail -= num;
	pphy = &p->phy_array[p->avail];
	p->used += num;
	memcpy(data, pphy, num * sizeof(phys_addr_t));
	pr_info("alloc succ\n");

	if (p->used + p->avail < p->high)
		tick_the_thread(ASYNC_ALLOC, sid);
	ret = EOK;
end:
	mutex_unlock(&p->mutex);
	return ret;
}

void __smem_allocator_msg_free(uint32_t sid, phys_addr_t *data, uint32_t num)
{
	uint32_t loop;
	struct smem_heap *p = NULL;

	if ((sid >= SMEM_HEAPS_MAX) || !num || !data)
		return;

	p = &smemheaps[sid];

	mutex_lock(&p->mutex);

	if (p->avail + num > p->max ||
		p->used < num)
		goto overflow;

	for (loop = 0; loop < num; loop++)
		p->phy_array[p->avail++] = data[loop];

	p->used -= num;

overflow:
	mutex_unlock(&p->mutex);
}

static int smem_allocator_async_alloc_thread(void *data)
{
	struct smem_heap *p = (struct smem_heap *)data;
	int ret;

	while (!kthread_should_stop()) {
		u64 need;
		u64 high;

		ret = wait_event_interruptible(p->alloc_wq, atomic_read(&p->wait_flag));
		if (ret)
			continue;
		atomic_set(&p->wait_flag, 0);
		atomic_inc(&p->alloc_run);
		mutex_lock(&p->mutex);
		high = max(p->high, (u64)p->watermark);
		if (p->used + p->avail >= high) {
			mutex_unlock(&p->mutex);
			goto overflow;
		}
		need = high - p->used - p->avail;
		mutex_unlock(&p->mutex);
		pr_err("async alloc, need: %lu\n", need);
		while (need--) {
			struct page *pg = NULL;

			pg = cma_alloc(p->cma, p->gran >> PAGE_SHIFT,
					(u32)get_order((u32)p->gran), GFP_KERNEL);
			if (!pg)
				goto overflow;

			__smem_config_mem_attr(page_to_phys(pg),
				(unsigned long)(uintptr_t)page_address(pg), p->gran);

			mutex_lock(&p->mutex);
			p->phy_array[p->avail++] = page_to_phys(pg);

			if (p->used + p->avail >= high) {
				mutex_unlock(&p->mutex);
				goto overflow;
			}

			mutex_unlock(&p->mutex);
		}

overflow:
	atomic_dec(&p->alloc_run);
	}

	return 0;
}

static inline struct task_struct *smem_allocator_create_thread(
				struct smem_heap *data, const char *name)
{
	return kthread_run(smem_allocator_async_alloc_thread, data, "%s", name);
}

static void smem_allocator_prealloc(struct smem_heap *p, unsigned int num)
{
	struct page *pg = NULL;

	if (!p || !p->init || !p->cma)
		return;

	pr_err("pre alloc smem, num: %u\n", num);
	mutex_lock(&p->mutex);
	while (num--) {
		pg = cma_alloc(p->cma, p->gran >> PAGE_SHIFT,
				(u32)get_order((u32)p->gran), GFP_KERNEL);
		if (!pg) {
			mutex_unlock(&p->mutex);
			return;
		}
		__smem_config_mem_attr(page_to_phys(pg),
				(unsigned long)(uintptr_t)page_address(pg), p->gran);
		p->phy_array[p->avail++] = page_to_phys(pg);
		if (p->avail >= p->max)
			break;
	}

	mutex_unlock(&p->mutex);
}

static int smem_allocator_init(const char *name)
{
	struct task_struct *task = NULL;
	struct smem_heap *p = &smemheaps[SMEM_SECOS_2M];

	mutex_init(&p->mutex);
	init_waitqueue_head(&p->alloc_wq);
	atomic_set(&p->alloc_run, 0);

	task = smem_allocator_create_thread(p, name);
	if (!task)
		return -1;
	p->alloc_task = task;

	p->phy_array = kzalloc(sizeof(phys_addr_t) * p->max + 1, GFP_KERNEL);
	if (!p->phy_array)
		return -1;

	p->init = 1;

	smem_allocator_prealloc(p, p->min);

	return 0;
}

#define SMEM_SZ_1K   1024
#define SMEM_MINFREE_MIN  (300 * 256)
#define SMEM_MINFREE_LOW  (500 * 256)
#define SMEM_MINFREE_HIGH (800 * 256)
#define SMEM_MAX_ADJ 3
static unsigned long smem_adj[SMEM_MAX_ADJ] = {
	8,
	4,
	2,
};

static unsigned long smem_minfree[SMEM_MAX_ADJ] = {
	SMEM_MINFREE_MIN,
	SMEM_MINFREE_LOW,
	SMEM_MINFREE_HIGH,
};

static unsigned long smem_get_adj(void)
{
	unsigned long other_file;
	unsigned long scan_adj;
	unsigned long minfree;
	int i;

	other_file = global_node_page_state(NR_FILE_PAGES) -
				global_node_page_state(NR_SHMEM) -
				global_node_page_state(NR_UNEVICTABLE) -
				total_swapcache_pages();
	scan_adj = 0;
	for (i = 0; i < SMEM_MAX_ADJ; i++) {
		minfree = smem_minfree[i];
		if (other_file < minfree) {
			scan_adj = smem_adj[i];
			break;
		}
	}

	return scan_adj;
}

static unsigned long smemheaps_scan(struct shrinker *s,
				struct shrink_control *sc)
{
	int i;
	unsigned long sum = 0;
	struct smem_heap *p = NULL;
	unsigned long long num;
	unsigned long max_scan_count;

#ifndef CONFIG_SECMEM_TEST
	if (!current_is_kswapd())
		return SHRINK_STOP;
#endif

	max_scan_count = smem_get_adj();
	if (max_scan_count == 0)
		return SHRINK_STOP;

	for (i = 0; i < SMEM_HEAPS_MAX; i++) {
		p = &smemheaps[i];

		if (atomic_read(&p->alloc_run))
			continue;

		if (p->escape_flag || !p->init)
			continue;

		mutex_lock(&p->mutex);

		if (p->used > 0)
			tick_the_thread(ASYNC_FREE, i);

		num = p->avail;
		if (num > p->watermark) {
			num -= p->watermark;
		} else {
			mutex_unlock(&p->mutex);
			continue;
		}

		if (max_scan_count != SMEM_SCAN_CNT && num > p->min)
			num -= p->min;

		num = min((unsigned long long)max_scan_count, num);
		while (num--) {
			struct page *page =
				phys_to_page(p->phy_array[--p->avail]);

#ifdef CONFIG_NEED_CHANGE_MAPPING
			change_secpage_range(page_to_phys(page),
				(unsigned long)(uintptr_t)page_address(page), p->gran,
				PAGE_KERNEL);
			flush_tlb_all();
#endif
			cma_release(p->cma, page, p->gran >> PAGE_SHIFT);
			sum += p->gran >> PAGE_SHIFT;
		}
		mutex_unlock(&p->mutex);
#ifdef CONFIG_SECMEM_TEST
		pr_err("smem heap[%d] scan: %lu\n", i, sum);
#endif
	}

	return sum ? sum : SHRINK_STOP;
}

static unsigned long smemheaps_count(struct shrinker *s, struct shrink_control *sc)
{
	int i;
	unsigned long sum = 0;
	unsigned long max_scan_count;

	if (!current_is_kswapd())
		return sum;

	if (smem_get_adj() == 0)
		return sum;

	for (i = 0; i < SMEM_HEAPS_MAX; i++) {
		struct smem_heap *p = &smemheaps[i];

		if (p->escape_flag || !p->init)
			continue;

		mutex_lock(&p->mutex);
		if (p->avail + p->used >= p->watermark)
			sum += (p->avail + p->used - p->watermark) *
				(p->gran >> PAGE_SHIFT);
		mutex_unlock(&p->mutex);
	}

	max_scan_count = (unsigned long)SMEM_SCAN_CNT *
		((unsigned long)SMEM_GRAN_2M >> PAGE_SHIFT);
	sum = min(max_scan_count, sum);

	return sum;
}

static struct shrinker smemheaps_shrinker = {
	.scan_objects = smemheaps_scan,
	.count_objects = smemheaps_count,
	.seeks = DEFAULT_SEEKS * SMEM_SHRINKER_NUM
};

static void smem_send_reclaim_msg(void)
{
	struct mem_chunk_list mcl = {0};
	int ret, num;
	u64 addr[SMEM_SCAN_CNT] = {0};
	u32 i;

	if (!ta_init)
		return;

	mcl.protect_id = SUB_CMD_FREE;
	mcl.phys_addr = addr;
	mcl.size = sizeof(addr);

	ret = secmem_tee_exec_cmd(session, &mcl, ION_SEC_CMD_VLTMM);
	pr_info("smem reclaim cmd, result: %d count: %u\n", ret, mcl.nents);
	num = 0;
	for (i = 0; i < mcl.nents; i++) {
		if (addr[i] != 0)
			num++;
		else
			break;
	}
	__smem_allocator_msg_free(SMEM_SECOS_2M, addr, num);

	return;
}

void smem_send_dump_msg(void)
{
	struct mem_chunk_list mcl = {0};
	int ret;
	u64 addr = 0;

	if (!ta_init)
		return;

	mcl.protect_id = SUB_CMD_DUMP;
	mcl.phys_addr = &addr;
	mcl.size = sizeof(addr);

	ret = secmem_tee_exec_cmd(session, &mcl, ION_SEC_CMD_VLTMM);
	pr_info("smem dump cmd, result: %d\n", ret);

	return;
}

static int smemheaps_async_free(void *data)
{
	struct smem_free_task *fp = &smem_async_free;

	while (!kthread_should_stop()) {
		int i, ret;

		ret = wait_event_interruptible(fp->free_wq, atomic_read(&fp->wait_flag));
		if (ret)
			continue;
		atomic_set(&fp->wait_flag, 0);

		for (i = 0; i < SMEM_HEAPS_MAX; i++) {
			struct smem_heap *p = &smemheaps[i];

			if (p->init && !atomic_read(&p->alloc_run))
				smem_send_reclaim_msg();
		}
	}
	return 0;
}

static void smem_pool_set_watermark(unsigned long value, u64 sid)
{
	unsigned long watermark;
	struct smem_heap *p = NULL;

	if (sid >= SMEM_HEAPS_MAX)
		return;

	p = &smemheaps[sid];
	watermark = value / p->gran;
	if (watermark > p->max)
		watermark = p->max;

	mutex_lock(&p->mutex);
	p->watermark = watermark;
	mutex_unlock(&p->mutex);
}

static ssize_t smem_pool_watermark_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	unsigned long val;
	int ret;

	ret = kstrtoul(buf, 0, &val);
	if (ret)
		return ret;

	if (val > SMEM_POOL_MAX)
		return -EINVAL;

	pr_err("smem pool watermark store: %lu", val);

	smem_pool_set_watermark(val, SMEM_SECOS_2M);

	if (val > 0)
		tick_the_thread(ASYNC_ALLOC, SMEM_SECOS_2M);

	return size;
}

static ssize_t smem_pool_watermark_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	ssize_t len = 0;
	struct smem_heap *p = &smemheaps[SMEM_SECOS_2M];
	int ret;
	size_t maxlen = PAGE_SIZE;

	ret = snprintf_s(buf, maxlen, maxlen - 1, "%s", "smem heap show:\n");
	if (ret < 0)
		return 0;

	len += ret;
	ret = snprintf_s(buf + len, maxlen - len, maxlen - 1 - len,
			"min: %llu\nlow: %llu\nhigh: %llu\n",
			p->min, p->low, p->high);
	if (ret < 0)
		return 0;

	len += ret;
	ret = snprintf_s(buf + len, maxlen - len, maxlen - 1 - len,
			"max: %llu\ngran: %llu MB\nwatermark: %lu\n",
			p->max, p->gran / SZ_1M, p->watermark);
	if (ret < 0)
		return 0;

	len += ret;
	ret = snprintf_s(buf + len, maxlen - len, maxlen - 1 - len,
			"avail: %llu\nused: %llu\n", p->avail, p->used);
	if (ret < 0)
		return 0;
	len += ret;

	return len;
}

static DEVICE_ATTR_RW(smem_pool_watermark);
static struct attribute *smem_pool_attrs[] = {
	&dev_attr_smem_pool_watermark.attr,
	NULL,
};
static const struct attribute_group smem_pool_attr_group = {
	.name = NULL,
	.attrs = smem_pool_attrs,
};

void smem_sysfs_init(void)
{
	int ret;
	struct platform_device *mm_ion_dev = get_mm_ion_platform_device();

	ret = sysfs_create_group(&mm_ion_dev->dev.kobj, &smem_pool_attr_group);
	if (ret)
		pr_err("create smem_pool attr fail!\n");
}

int smem_free_task_init(void)
{
	init_waitqueue_head(&smem_async_free.free_wq);
	atomic_set(&smem_async_free.wait_flag, 0);
	smem_async_free.free_task = kthread_run(smemheaps_async_free,
								NULL, "%s", "smem_async_free");
	if (!smem_async_free.free_task)
		return -1;

	return 0;
}

static int smem_ta_session_init(void)
{
	int ret;

	if (!ta_init) {
		context = kzalloc(sizeof(TEEC_Context), GFP_KERNEL);
		if (!context)
			return -1;

		session = kzalloc(sizeof(TEEC_Session), GFP_KERNEL);
		if (!session) {
			kfree(context);
			context = NULL;
			return -1;
		}

		ret = secmem_tee_init(context, session, TEE_VLTMM_NAME);
		if (ret) {
			pr_err("TA session init failed\n");
			kfree(context);
			kfree(session);
			context = NULL;
			session = NULL;
			return -1;
		}
		ta_init = 1;
	}

	return 0;
}

int smemheaps_init(void)
{
	pr_err("smemheaps init.\n");

	if (smem_free_task_init())
		return -1;

	register_shrinker(&smemheaps_shrinker);

	if (smem_allocator_init("smem-2m-allocator"))
		return -1;

	smem_sysfs_init();

	return 0;
}

device_initcall(smem_ta_session_init);

#ifdef CONFIG_SECMEM_TEST
struct task_struct *vltm_test_task = NULL;
wait_queue_head_t vltm_test_wq;
atomic_t vltm_test_flag;
static u32 vltmm_test_p1 = 0;
static u32 vltmm_test_p2 = 0;
static u32 vltmm_test_p3 = 0;

static void vltmm_test_init()
{
	init_waitqueue_head(&vltm_test_wq);
	atomic_set(&vltm_test_flag, 0);
}

static void vltmm_test_sendcmd()
{
	struct mem_chunk_list mcl;
	int ret;
	u64 addr = 0;

	mcl.phys_addr = &addr;
	mcl.size = sizeof(addr);
	mcl.protect_id = vltmm_test_p1;
	mcl.nents = vltmm_test_p2;
	mcl.va = vltmm_test_p3;

	ret = secmem_tee_exec_cmd(session, &mcl, ION_SEC_CMD_VLTMM);
	if (ret)
		pr_err("vltmm test fail, %d\n", ret);
	else
		pr_err("vltmm test, cmd: %u, p2: 0x%x p3: 0x%x\n",
			mcl.protect_id, mcl.nents, mcl.va);
}

void vltmm_test_wakeup(u32 p1, u32 p2, u32 p3)
{
	if (!vltm_test_task)
		return;
	vltmm_test_p1 = p1;
	vltmm_test_p2 = p2;
	vltmm_test_p3 = p3;
	atomic_set(&vltm_test_flag, 1);
	wake_up_interruptible(&vltm_test_wq);
}

static int vltmm_test_kthread(void *p)
{
	int ret;

	vltmm_test_init();
	while (!kthread_should_stop()) {
		ret = wait_event_interruptible(vltm_test_wq, atomic_read(&vltm_test_flag));
		if (ret)
			continue;
		atomic_set(&vltm_test_flag, 0);

		vltmm_test_sendcmd();

		msleep(1);
	}

	return 0;
}

void vltmm_create_thread(void)
{
	if (!vltm_test_task) {
		pr_err("vlt create test thread\n");
		vltm_test_task = kthread_run(vltmm_test_kthread, NULL,
						"%s", "vltmm_test");
	}
}

void vltmm_test_secmem(u64 phy, u32 len)
{
	void *va = NULL;
	int ret;

	va = phys_to_virt(phy);
	if (va) {
		ret = memset_s(va, len, SMEM_TEST_MAGIC, len);
		if (ret) {
			pr_err("vlt mem set failed\n");
			return;
		}
		pr_err("vlt mem write: %x\n", SMEM_TEST_MAGIC);
		pr_err("vlt mem read: %llx\n", *(u64*)va);
	}
}

void smem_set_minfree(unsigned long p1, unsigned long p2, unsigned long p3)
{
	int i;

	i = 0;
	smem_minfree[i++] = p1;
	smem_minfree[i++] = p2;
	smem_minfree[i] = p3;
}
#endif

