/*
 * mailbox_mempool.c
 *
 * mailbox memory managing for sharing memory with TEE.
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "mailbox_mempool.h"
#include <linux/list.h>
#include <linux/sizes.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <securec.h>
#include <linux/vmalloc.h>
#include "teek_client_constants.h"
#include "tc_ns_log.h"
#include "smc_smp.h"
#include "ko_adapt.h"

#define MAILBOX_PAGE_MAX (MAILBOX_POOL_SIZE >> PAGE_SHIFT)
static int g_max_oder;

#define OPT_MODE   0660U
#define STATE_MODE 0440U

struct mb_page_t {
	struct list_head node;
	struct page *page;
	int order;
	unsigned int count; /* whether be used */
};

struct mb_free_area_t {
	struct list_head page_list;
	int order;
};

struct mb_zone_t {
	struct page *all_pages;
	struct mb_page_t pages[MAILBOX_PAGE_MAX];
	struct mb_free_area_t free_areas[0];
};

static struct mb_zone_t *g_m_zone;
static struct mutex g_mb_lock;

static void mailbox_show_status(void)
{
	unsigned int i;
	struct mb_page_t *pos = NULL;
	struct list_head *head = NULL;
	unsigned int used = 0;

	if (!g_m_zone) {
		tloge("zone struct is NULL\n");
		return;
	}

	tloge("########################################\n");
	mutex_lock(&g_mb_lock);
	for (i = 0; i < MAILBOX_PAGE_MAX; i++) {
		if (g_m_zone->pages[i].count) {
			tloge("page[%02d], order=%02d, count=%d\n",
				i, g_m_zone->pages[i].order,
				g_m_zone->pages[i].count);
			used += (1 << (uint32_t)g_m_zone->pages[i].order);
		}
	}
	tloge("total usage:%u/%u\n", used, MAILBOX_PAGE_MAX);
	tloge("----------------------------------------\n");

	for (i = 0; i < (unsigned int)g_max_oder; i++) {
		head = &g_m_zone->free_areas[i].page_list;
		if (list_empty(head)) {
			tloge("order[%02d] is empty\n", i);
		} else {
			list_for_each_entry(pos, head, node)
				tloge("order[%02d]\n", i);
		}
	}
	mutex_unlock(&g_mb_lock);

	tloge("########################################\n");
}

#define MB_SHOW_LINE 64
#define BITS_OF_BYTE  8
static void mailbox_show_details(void)
{
	unsigned int i;
	unsigned int used = 0;
	unsigned int left = 0;
	unsigned int order = 0;

	if (!g_m_zone) {
		tloge("zone struct is NULL\n");
		return;
	}

	tloge("----- show mailbox details -----");
	mutex_lock(&g_mb_lock);
	for (i = 0; i < MAILBOX_PAGE_MAX; i++) {
		if (i % MB_SHOW_LINE  == 0) {
			tloge("\n");
			tloge("%04d-%04d:", i, i + MB_SHOW_LINE);
		}

		if (g_m_zone->pages[i].count) {
			left = 1 << (uint32_t)g_m_zone->pages[i].order;
			order = g_m_zone->pages[i].order;
			used += (1 << (uint32_t)g_m_zone->pages[i].order);
		}

		if (left) {
			left--;
			tloge("%01d", order);
		} else {
			tloge("X");
		}

		if (i > 1 && (i + 1) % (MB_SHOW_LINE / BITS_OF_BYTE) == 0)
			tloge(" ");
	}
	tloge("total usage:%u/%u\n", used, MAILBOX_PAGE_MAX);
	mutex_unlock(&g_mb_lock);
}

void *mailbox_alloc(size_t size, unsigned int flag)
{
	unsigned int i;
	struct mb_page_t *pos = (struct mb_page_t *)NULL;
	struct list_head *head = NULL;
	int order = get_order(ALIGN(size, SZ_4K));
	void *addr = NULL;

	if (!size || !g_m_zone) {
		tlogw("alloc 0 size mailbox or zone struct is NULL\n");
		return NULL;
	}

	if (order > g_max_oder || order < 0) {
		tloge("invalid order %d\n", order);
		return NULL;
	}

	mutex_lock(&g_mb_lock);
	for (i = (unsigned int)order; i <= (unsigned int)g_max_oder; i++) {
		unsigned int j;

		head = &g_m_zone->free_areas[i].page_list;
		if (list_empty(head))
			continue;

		pos = list_first_entry(head, struct mb_page_t, node);

		pos->count = 1;
		pos->order = order;

		/* split and add free list */
		for (j = order; j < i; j++) {
			struct mb_page_t *new_page = NULL;

			new_page = pos + (1 << j);
			new_page->count = 0;
			new_page->order = j;
			list_add_tail(&new_page->node,
				&g_m_zone->free_areas[j].page_list);
		}
		list_del(&pos->node);
		addr = page_address(pos->page);
		break;
	}
	mutex_unlock(&g_mb_lock);

	if (addr && (flag & MB_FLAG_ZERO)) {
		if (memset_s(addr, ALIGN(size, SZ_4K),
			0, ALIGN(size, SZ_4K))) {
			tloge("clean mailbox failed\n");
			mailbox_free(addr);
			return NULL;
		}
	}
	return addr;
}

static void add_max_order_block(unsigned int idex)
{
	struct mb_page_t *self = NULL;

	if (idex != g_max_oder || !g_m_zone)
		return;

	/*
	 * when idex equal max order, no one use mailbox mem,
	 * we need to hang all pages in the last free area page list
	 */
	self = &g_m_zone->pages[0];
	list_add_tail(&self->node,
		&g_m_zone->free_areas[g_max_oder].page_list);
}

static bool is_ptr_valid(struct page *page)
{
	if (!g_m_zone)
		return false;

	if (page < g_m_zone->all_pages ||
		page >= (g_m_zone->all_pages + MAILBOX_PAGE_MAX)) {
		tloge("invalid ptr to free in mailbox\n");
		return false;
	}

	return true;
}

void mailbox_free(const void *ptr)
{
	unsigned int i;
	struct page *page = NULL;
	struct mb_page_t *self = NULL;
	struct mb_page_t *buddy = NULL;
	unsigned int self_idx;
	unsigned int buddy_idx;

	if (!ptr || !g_m_zone) {
		tloge("invalid ptr\n");
		return;
	}

	page = virt_to_page((uint64_t)(uintptr_t)ptr);
	if (!is_ptr_valid(page))
		return;

	mutex_lock(&g_mb_lock);
	self_idx = page - g_m_zone->all_pages;
	self = &g_m_zone->pages[self_idx];
	if (!self->count) {
		tloge("already freed in mailbox\n");
		mutex_unlock(&g_mb_lock);
		return;
	}

	for (i = (unsigned int)self->order; i <
		(unsigned int)g_max_oder; i++) {
		self_idx = page - g_m_zone->all_pages;
		buddy_idx = self_idx ^ (uint32_t)(1 << i);
		self = &g_m_zone->pages[self_idx];
		buddy = &g_m_zone->pages[buddy_idx];
		self->count = 0;
		/* is buddy free  */
		if ((unsigned int)buddy->order == i && buddy->count == 0) {
			/* release buddy */
			list_del(&buddy->node);
			/* combine self and buddy */
			if (self_idx > buddy_idx) {
				page = buddy->page;
				buddy->order = (int)i + 1;
				self->order = -1;
			} else {
				self->order = (int)i + 1;
				buddy->order = -1;
			}
		} else {
			/* release self */
			list_add_tail(&self->node,
				&g_m_zone->free_areas[i].page_list);
			mutex_unlock(&g_mb_lock);
			return;
		}
	}

	add_max_order_block(i);
	mutex_unlock(&g_mb_lock);
}

struct mb_cmd_pack *mailbox_alloc_cmd_pack(void)
{
	void *pack = mailbox_alloc(SZ_4K, MB_FLAG_ZERO);

	if (!pack)
		tloge("alloc mb cmd pack failed\n");

	return (struct mb_cmd_pack *)pack;
}

void *mailbox_copy_alloc(const void *src, size_t size)
{
	void *mb_ptr = NULL;

	if (!src || !size) {
		tloge("invali src to alloc mailbox copy\n");
		return NULL;
	}

	mb_ptr = mailbox_alloc(size, 0);
	if (!mb_ptr) {
		tloge("alloc size %zu mailbox failed\n", size);
		return NULL;
	}

	if (memcpy_s(mb_ptr, size, src, size)) {
		tloge("memcpy to mailbox failed\n");
		mailbox_free(mb_ptr);
		return NULL;
	}

	return mb_ptr;
}

struct mb_dbg_entry {
	struct list_head node;
	unsigned int idx;
	void *ptr;
};

static LIST_HEAD(mb_dbg_list);
static DEFINE_MUTEX(mb_dbg_lock);
static unsigned int g_mb_dbg_entry_count = 1;
static unsigned int g_mb_dbg_last_res; /* only cache 1 opt result */
static struct dentry *g_mb_dbg_dentry;

static unsigned int mb_dbg_add_entry(void *ptr)
{
	struct mb_dbg_entry *new_entry = NULL;

	new_entry = kmalloc(sizeof(*new_entry), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)new_entry)) {
		tloge("alloc entry failed\n");
		return 0;
	}

	INIT_LIST_HEAD(&new_entry->node);
	new_entry->ptr = ptr;
	mutex_lock(&mb_dbg_lock);
	new_entry->idx = g_mb_dbg_entry_count;

	if ((g_mb_dbg_entry_count++) == 0)
		g_mb_dbg_entry_count++;
	list_add_tail(&new_entry->node, &mb_dbg_list);
	mutex_unlock(&mb_dbg_lock);

	return new_entry->idx;
}

static void mb_dbg_remove_entry(unsigned int idx)
{
	struct mb_dbg_entry *pos = NULL;
	struct mb_dbg_entry *temp = NULL;

	mutex_lock(&mb_dbg_lock);
	list_for_each_entry_safe(pos, temp, &mb_dbg_list, node) {
		if (pos->idx == idx) {
			mailbox_free(pos->ptr);
			list_del(&pos->node);
			kfree(pos);
			mutex_unlock(&mb_dbg_lock);
			return;
		}
	}
	mutex_unlock(&mb_dbg_lock);

	tloge("entry %u invalid\n", idx);
}

static void mb_dbg_reset(void)
{
	struct mb_dbg_entry *pos = NULL;
	struct mb_dbg_entry *tmp = NULL;

	mutex_lock(&mb_dbg_lock);
	list_for_each_entry_safe(pos, tmp, &mb_dbg_list, node) {
		mailbox_free(pos->ptr);
		list_del(&pos->node);
		kfree(pos);
	}
	g_mb_dbg_entry_count = 0;
	mutex_unlock(&mb_dbg_lock);
}

#define MB_WRITE_SIZE 64

static bool is_opt_write_param_valid(struct file *filp,
	const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	if (!filp || !ppos || !ubuf)
		return false;

	if (cnt >= MB_WRITE_SIZE || !cnt)
		return false;

	return true;
}

static void alloc_dbg_entry(unsigned int alloc_size)
{
	unsigned int idx;
	void *ptr = NULL;

	ptr = mailbox_alloc(alloc_size, 0);
	if (!ptr) {
		tloge("alloc order=%u in mailbox failed\n", alloc_size);
		return;
	}

	idx = mb_dbg_add_entry(ptr);
	if (!idx)
		mailbox_free(ptr);
	g_mb_dbg_last_res = idx;
}

static ssize_t mb_dbg_opt_write(struct file *filp,
	const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	char buf[MB_WRITE_SIZE] = {0};
	char *cmd = NULL;
	char *value = NULL;
	unsigned int alloc_size;
	unsigned int free_idx;

	if (!is_opt_write_param_valid(filp, ubuf, cnt, ppos))
		return -EINVAL;

	if (copy_from_user(buf, ubuf, cnt))
		return -EFAULT;

	buf[cnt] = 0;
	value = buf;
	if (!strncmp(value, "reset", strlen("reset"))) {
		tlogi("mb dbg reset\n");
		mb_dbg_reset();
		return cnt;
	}

	cmd = strsep(&value, ":");
	if (!cmd || !value) {
		tloge("no valid cmd or value for mb dbg\n");
		return -EFAULT;
	}

	if (!strncmp(cmd, "alloc", strlen("alloc"))) {
		if (kstrtou32(value, 10, &alloc_size) == 0)
			alloc_dbg_entry(alloc_size);
		else
			tloge("invalid value format for mb dbg\n");
	} else if (!strncmp(cmd, "free", strlen("free"))) {
		if (kstrtou32(value, 10, &free_idx) == 0)
			mb_dbg_remove_entry(free_idx);
		else
			tloge("invalid value format for mb dbg\n");
	} else {
		tloge("invalid format for mb dbg\n");
	}

	return cnt;
}

static ssize_t mb_dbg_opt_read(struct file *filp, char __user *ubuf,
	size_t cnt, loff_t *ppos)
{
	char buf[16] = {0};
	ssize_t ret;

	(void)(filp);

	ret = snprintf_s(buf, sizeof(buf), 15, "%u\n", g_mb_dbg_last_res);
	if (ret < 0) {
		tloge("snprintf idx failed\n");
		return -EINVAL;
	}

	return simple_read_from_buffer(ubuf, cnt, ppos, buf, ret);
}

static const struct file_operations g_mb_dbg_opt_fops = {
	.owner = THIS_MODULE,
	.read = mb_dbg_opt_read,
	.write = mb_dbg_opt_write,
};

static ssize_t mb_dbg_state_read(struct file *filp, char __user *ubuf,
	size_t cnt, loff_t *ppos)
{
	(void)(filp);
	(void)(ubuf);
	(void)(ppos);
	mailbox_show_status();
	mailbox_show_details();
	return 0;
}

static const struct file_operations g_mb_dbg_state_fops = {
	.owner = THIS_MODULE,
	.read = mb_dbg_state_read,
};

static int mailbox_register(const void *mb_pool, unsigned int size)
{
	struct tc_ns_operation *operation = NULL;
	struct tc_ns_smc_cmd *smc_cmd = NULL;
	int ret = 0;

	smc_cmd = kzalloc(sizeof(*smc_cmd), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)smc_cmd)) {
		tloge("alloc smc_cmd failed\n");
		return -EIO;
	}

	operation = kzalloc(sizeof(*operation), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)operation)) {
		tloge("alloc operation failed\n");
		ret = -EIO;
		goto free_smc_cmd;
	}

	operation->paramtypes = TEE_PARAM_TYPE_VALUE_INPUT |
		(TEE_PARAM_TYPE_VALUE_INPUT << TEE_PARAM_NUM);
	operation->params[0].value.a = virt_to_phys(mb_pool);
	operation->params[0].value.b =
		virt_to_phys(mb_pool) >> ADDR_TRANS_NUM;
	operation->params[1].value.a = size;

	smc_cmd->global_cmd = true;
	smc_cmd->cmd_id = GLOBAL_CMD_ID_REGISTER_MAILBOX;
	smc_cmd->operation_phys = virt_to_phys(operation);
	smc_cmd->operation_h_phys = virt_to_phys(operation) >> ADDR_TRANS_NUM;

	if (tc_ns_smc(smc_cmd)) {
		tloge("resigter mailbox failed\n");
		ret = -EIO;
	}

	kfree(operation);
	operation = NULL;
free_smc_cmd:
	kfree(smc_cmd);
	smc_cmd = NULL;
	return ret;
}

int mailbox_mempool_init(void)
{
	int i;
	struct mb_page_t *mb_page = NULL;
	struct mb_free_area_t *area = NULL;
	struct page *all_pages = NULL;
	size_t zone_len;

	g_max_oder = get_order(MAILBOX_POOL_SIZE);
	tloge("in this RE, mailbox max order is: %d\n", g_max_oder);

	/* zone len is fixed, will not overflow */
	zone_len = sizeof(*area) * (g_max_oder + 1) + sizeof(*g_m_zone);
	g_m_zone = kzalloc(zone_len, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)g_m_zone)) {
		tloge("fail to alloc zone struct\n");
		return -ENOMEM;
	}

	all_pages = koadpt_alloc_pages(GFP_KERNEL, g_max_oder);
	if (!all_pages) {
		tloge("fail to alloc mailbox mempool\n");
		kfree(g_m_zone);
		g_m_zone = NULL;
		return -ENOMEM;
	}

	if (mailbox_register(page_address(all_pages), MAILBOX_POOL_SIZE)) {
		tloge("register mailbox failed\n");
		__free_pages(all_pages, g_max_oder);
		kfree(g_m_zone);
		g_m_zone = NULL;
		return -EIO;
	}

	for (i = 0; i < MAILBOX_PAGE_MAX; i++) {
		g_m_zone->pages[i].order = -1;
		g_m_zone->pages[i].count = 0;
		g_m_zone->pages[i].page = &all_pages[i];
	}

	g_m_zone->pages[0].order = g_max_oder;

	for (i = 0; i <= g_max_oder; i++) {
		area = &g_m_zone->free_areas[i];
		INIT_LIST_HEAD(&area->page_list);
		area->order = i;
	}

	mb_page = &g_m_zone->pages[0];
	list_add_tail(&mb_page->node, &area->page_list);
	g_m_zone->all_pages = all_pages;
	mutex_init(&g_mb_lock);
	g_mb_dbg_dentry = debugfs_create_dir("tz_mailbox", NULL);
	debugfs_create_file("opt", OPT_MODE, g_mb_dbg_dentry, NULL,
		&g_mb_dbg_opt_fops);
	debugfs_create_file("state", STATE_MODE, g_mb_dbg_dentry, NULL,
		&g_mb_dbg_state_fops);
	return 0;
}

void mailbox_mempool_destroy(void)
{
	__free_pages(g_m_zone->all_pages, g_max_oder);
	g_m_zone->all_pages = NULL;
	kfree(g_m_zone);
	g_m_zone = NULL;
}
