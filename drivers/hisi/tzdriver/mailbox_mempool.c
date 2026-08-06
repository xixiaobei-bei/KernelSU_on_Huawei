/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 * Description: Mailbox memory managing for sharing memory with TEE.
 * Author: qiqingchao  q00XXXXXX
 * Create: 2016-06-21
 */
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

#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/vmalloc.h>
#endif

#include "teek_client_constants.h"
#include "mailbox_mempool.h"
#include "tc_ns_log.h"
#include "smc.h"
#include "libhwsecurec/securec.h"

#define MAILBOX_PAGE_MAX (MAILBOX_POOL_SIZE >> PAGE_SHIFT)
#define MAILBOX_ORDER_MAX get_order(MAILBOX_POOL_SIZE)

struct mb_page_t {
	struct list_head node;
	struct page *page;
	int order; //block size
	unsigned int count; //whether be used
};

struct mb_free_area_t {
	struct list_head page_list;
	int order;
};

struct mb_zone_t {
	struct page *all_pages;
	struct mb_page_t pages[MAILBOX_PAGE_MAX];
	struct mb_free_area_t free_areas[MAILBOX_ORDER_MAX + 1];
};

static struct mb_zone_t m_zone;
static struct mutex mb_lock;

static void mailbox_show_status(void)
{
	unsigned int i;
	struct mb_page_t *pos = NULL;
	struct list_head *head = NULL;
	unsigned int used = 0;

	pr_info("########################################\n");
	mutex_lock(&mb_lock);
	for (i = 0; i < MAILBOX_PAGE_MAX; i++) {
		if (m_zone.pages[i].count) {
			pr_info("page[%02d], order=%02d, count=%d, page=%pK\n",
			i, m_zone.pages[i].order, m_zone.pages[i].count,
			m_zone.pages[i].page);
			used += (1 << m_zone.pages[i].order);
		}
	}
	pr_info("total usage:%u/%u\n", used, MAILBOX_PAGE_MAX);
	pr_info("----------------------------------------\n");

	for (i = 0; i < (unsigned int)MAILBOX_ORDER_MAX; i++) {
		head = &m_zone.free_areas[i].page_list;
		if (list_empty(head))
			pr_info("order[%02d] is empty\n", i);
		else {
			list_for_each_entry(pos, head, node)
			pr_info("order[%02d], address=%pK\n", i, pos->page);
		}
	}
	mutex_unlock(&mb_lock);

	pr_info("########################################\n");
}

#define MB_SHOW_LINE 64
static void mailbox_show_details(void)
{
	unsigned int i= 0;
	unsigned int used = 0;
	unsigned int left = 0;
	unsigned int order = 0;

	pr_info("----- show mailbox details -----");
	mutex_lock(&mb_lock);
	for (i = 0; i < MAILBOX_PAGE_MAX; i++) {

		if (i == 0 % MB_SHOW_LINE) {
			printk("\n");
			printk("%04d-%04d:", i, i + MB_SHOW_LINE);
		}

		if (m_zone.pages[i].count) {
			left = 1 << m_zone.pages[i].order;
			order = m_zone.pages[i].order;
			used += (1 << m_zone.pages[i].order);
		}

		if (left) {
			left--;
			printk("%01d", order);
		} else
			printk("X");

		if (i > 1 && (i + 1) % (MB_SHOW_LINE / 8) == 0)
			printk(" ");
	}
	pr_info("total usage:%u/%u\n", used, MAILBOX_PAGE_MAX);
	mutex_unlock(&mb_lock);
}

void *mailbox_alloc(size_t size, unsigned int flag)
{
	unsigned int i;
	struct mb_page_t *pos = (struct mb_page_t *)NULL;
	struct list_head *head = NULL;
	unsigned int order = get_order(ALIGN(size, SZ_4K));

	if (size == 0) {
		tlogw("alloc 0 size mailbox\n");
		return NULL;
	}

	if (order > (unsigned int)MAILBOX_ORDER_MAX) {
		tloge("invalid order %d\n", order);
		return NULL;
	}

	mutex_lock(&mb_lock);
	for (i = order; i <= (unsigned int)MAILBOX_ORDER_MAX; i++) {
		unsigned int j;

		head = &m_zone.free_areas[i].page_list;
		if (list_empty(head))
			continue;

		pos = list_first_entry(head, struct mb_page_t, node);

		pos->count = 1;
		pos->order = order;

		/* split and add free list */
		for (j = order; j < i; j++) {
			struct mb_page_t *new_page = NULL;

			new_page = pos + (1 << j); /*lint !e679 */
			new_page->count = 0;
			new_page->order = j;
			list_add_tail(&new_page->node, &m_zone.free_areas[j].page_list);
		}
		list_del(&pos->node);
		mutex_unlock(&mb_lock);

		if (flag & MB_FLAG_ZERO) {
			if (memset_s(page_address(pos->page), ALIGN(size, SZ_4K), 0,
				ALIGN(size, SZ_4K)))
				tloge("clean mailbox failed\n");
		}
		return page_address(pos->page);
	}
	mutex_unlock(&mb_lock);

	return NULL;
}

void mailbox_free(const void *ptr)
{
	int i;
	struct page *page = NULL;
	struct mb_page_t *self = NULL;
	struct mb_page_t *buddy = NULL;
	unsigned int self_idx;
	unsigned int buddy_idx;

	if (ptr == NULL) {
		tloge("invalid ptr\n");
		return;
	}

	page = virt_to_page(ptr);
	if (page < m_zone.all_pages || page >= (m_zone.all_pages + MAILBOX_PAGE_MAX)) {
		tloge("invalid ptr to free in mailbox\n");
		return;
	}

	mutex_lock(&mb_lock);
	self_idx = page - m_zone.all_pages;
	self = &m_zone.pages[self_idx];
	if (!self->count) {
		tloge("already freed in mailbox\n");
		mutex_unlock(&mb_lock);
		return;
	}

	for (i = (unsigned int)self->order; i <= MAILBOX_ORDER_MAX; i++) {
		self_idx = page - m_zone.all_pages;
		buddy_idx = self_idx ^ (1 << i);
		self = &m_zone.pages[self_idx];
		buddy = &m_zone.pages[buddy_idx];
		self->count = 0;
		/* is buddy free  */
		if (buddy->order == i && buddy->count == 0) {
			/* release buddy */
			list_del(&buddy->node);
			/* combine self and buddy */
			if (self_idx > buddy_idx) {
				page = buddy->page;
				buddy->order = i + 1;
				self->order = -1;
			} else {
				self->order = i + 1;
				buddy->order = -1;
			}
		} else {
			/* release self */
			list_add_tail(&self->node, &m_zone.free_areas[i].page_list);
			mutex_unlock(&mb_lock);
			return;
		}
	}
	mutex_unlock(&mb_lock);
}

struct mb_cmd_pack *mailbox_alloc_cmd_pack(void)
{
	void *pack = mailbox_alloc(SZ_4K, MB_FLAG_ZERO);

	if (pack == NULL)
		tloge("alloc mb cmd pack failed\n");
	return (struct mb_cmd_pack *)pack;
}

void *mailbox_copy_alloc(const void *src, size_t size)
{
	void *mb_ptr = NULL;

	if ((src == NULL) || (size == 0)) {
		tloge("invali src to alloc mailbox copy\n");
		return NULL;
	}

	mb_ptr = mailbox_alloc(size, 0);
	if (mb_ptr == NULL) {
		tloge("alloc size(%zu) mailbox failed\n", size);
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
static unsigned int mb_dbg_entry_count = 1;
static unsigned int mb_dbg_last_res; /* only cache 1 opt result */
static struct dentry *mb_dbg_dentry = NULL;

/*lint -e429 */
static unsigned int mb_dbg_add_entry(void *ptr)
{
	struct mb_dbg_entry *new_entry = NULL;

	new_entry = kmalloc(sizeof(*new_entry), GFP_KERNEL);
	if (new_entry == NULL) {
		tloge("alloc entry failed\n");
		return 0;
	}
	INIT_LIST_HEAD(&new_entry->node);
	new_entry->ptr = ptr;
	new_entry->idx = mb_dbg_entry_count;
	/* to make sure mb_dbg_entry_count==0 is invalid */
	if ((mb_dbg_entry_count++) == 0)
		mb_dbg_entry_count++;

	mutex_lock(&mb_dbg_lock);
	list_add_tail(&new_entry->node, &mb_dbg_list);
	mutex_unlock(&mb_dbg_lock);

	return new_entry->idx;
}
/*lint +e429 */

static void mb_dbg_remove_entry(unsigned int idx)
{
	struct mb_dbg_entry *pos = NULL;

	mutex_lock(&mb_dbg_lock);
	list_for_each_entry(pos, &mb_dbg_list, node) {
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
	mb_dbg_entry_count = 0;
	mutex_unlock(&mb_dbg_lock);
}

static ssize_t mb_dbg_opt_write(struct file *filp,
	const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	char buf[64] = {0};
	char *cmd = NULL;
	char *value = NULL;
	bool check_value;

	check_value = filp == NULL || ppos == NULL;
	if (check_value || ubuf == NULL)
		return -EINVAL;
	if (cnt >= sizeof(buf) || cnt == 0)
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
	if (cmd == NULL || value == NULL) {
		tloge("no valid cmd or value for mb dbg\n");
		return -EFAULT;
	}

	if (!strncmp(cmd, "alloc", strlen("alloc"))) {
		unsigned int alloc_size = 0;
		if (kstrtou32(value, 10, &alloc_size) == 0) {
			unsigned int idx;
			void *ptr = mailbox_alloc(alloc_size, 0);
			if (ptr != NULL) {
				idx = mb_dbg_add_entry(ptr);
				if (idx == 0)
					mailbox_free(ptr);
				mb_dbg_last_res = idx;

			} else {
				tloge("alloc order=%u in mailbox failed\n", alloc_size);
			}
		} else {
			tloge("invalid value format for mb dbg\n");
		}
	} else if (!strncmp(cmd, "free", strlen("free"))) {
		unsigned int free_idx = 0;
		if (kstrtou32(value, 10, &free_idx) == 0) {
			mb_dbg_remove_entry(free_idx);
		} else {
			tloge("invalid value format for mb dbg\n");
		}
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

	ret = snprintf_s(buf, sizeof(buf), 15, "%u\n", mb_dbg_last_res);
	if (ret < 0) {
		tloge("snprintf idx failed\n");
		return -EINVAL;
	}

	return simple_read_from_buffer(ubuf, cnt, ppos, buf, ret);
}

static const struct file_operations mb_dbg_opt_fops = {
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

static const struct file_operations mb_dbg_state_fops = {
	.owner = THIS_MODULE,
	.read = mb_dbg_state_read,
};

static int mailbox_register(const void *mb_pool, unsigned int size)
{

	tc_ns_operation *operation;
	char *uuid;
	tc_ns_smc_cmd *smc_cmd;
	int ret = 0;

	smc_cmd = kzalloc(sizeof(*smc_cmd), GFP_KERNEL);
	if (smc_cmd == NULL) {
		tloge("alloc smc_cmd failed\n");
		return -EIO;
	}
	uuid = kzalloc(sizeof(char) * GLOBAL_UUID_LEN, GFP_KERNEL);
	if (uuid == NULL) {
		ret = -EIO;
		goto free_smc_cmd;
	}
	operation = kzalloc(sizeof(*operation), GFP_KERNEL);
	if (operation == NULL) {
		tloge("alloc operation failed\n");
		ret = -EIO;
		goto free_uuid;
	}

	operation->paramtypes = TEE_PARAM_TYPE_VALUE_INPUT |
		(TEE_PARAM_TYPE_VALUE_INPUT << 4);
	operation->params[0].value.a = virt_to_phys(mb_pool);
	operation->params[0].value.b = virt_to_phys(mb_pool) >> 32;
	operation->params[1].value.a = size;

	uuid[0] = 1;
	smc_cmd->uuid_phys = virt_to_phys(uuid);
	smc_cmd->uuid_h_phys = virt_to_phys(uuid) >> 32;
	smc_cmd->cmd_id = GLOBAL_CMD_ID_REGISTER_MAILBOX;
	smc_cmd->operation_phys = virt_to_phys(operation);
	smc_cmd->operation_h_phys = virt_to_phys(operation) >> 32;

	ret = tc_ns_smc(smc_cmd);
	if (ret != TEEC_SUCCESS) {
		tloge("resigter mailbox failed\n");
		ret = -EIO;
	}

	kfree(operation);
	operation = NULL;
free_uuid:
	kfree(uuid);
	uuid = NULL;
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

	all_pages = alloc_pages(GFP_KERNEL, MAILBOX_ORDER_MAX);
	if (all_pages == NULL) {
		tloge("fail to alloc mailbox mempool\n");
		return -ENOMEM;
	}
	if (mailbox_register(page_address(all_pages), MAILBOX_POOL_SIZE)) {
		tloge("register mailbox failed\n");
		free_pages((unsigned long)all_pages, MAILBOX_ORDER_MAX);
		return -EIO;
	}
	for (i = 0; i < MAILBOX_PAGE_MAX; i++) {
		m_zone.pages[i].order = -1;
		m_zone.pages[i].count = 0;
		m_zone.pages[i].page = &all_pages[i];
	}
	m_zone.pages[0].order = MAILBOX_ORDER_MAX;

	for (i = 0; i <= MAILBOX_ORDER_MAX; i++) {
		area = &m_zone.free_areas[i];
		INIT_LIST_HEAD(&area->page_list);
		area->order = i;
	}

	mb_page = &m_zone.pages[0];
	list_add_tail(&mb_page->node, &area->page_list);  /* [false alarm]:area arrays need init */
	m_zone.all_pages = all_pages;
	mutex_init(&mb_lock);
	mb_dbg_dentry = debugfs_create_dir("tz_mailbox", NULL);
	debugfs_create_file("opt", 0660, mb_dbg_dentry, NULL, &mb_dbg_opt_fops);
	debugfs_create_file("state", 0440, mb_dbg_dentry, NULL, &mb_dbg_state_fops);
	return 0;
}

void mailbox_mempool_destroy(void)
{
	free_pages((unsigned long)m_zone.all_pages, MAILBOX_ORDER_MAX);
	m_zone.all_pages = NULL;
}
