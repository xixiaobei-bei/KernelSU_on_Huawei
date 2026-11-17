/*
 * auth_base_impl.c
 *
 * function for base hash operation
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
#include "auth_base_impl.h"
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/rwsem.h>
#include <linux/path.h>
#include <linux/file.h>
#include <linux/fs.h>

#include <linux/mm.h>
#include <linux/dcache.h>
#include <linux/mm_types.h>
#include <linux/highmem.h>
#include <linux/cred.h>
#include <linux/slab.h>
#include <linux/sched/mm.h>

#ifdef CONFIG_SECURITY_SELINUX
#include <linux/security.h>
#endif

#include <securec.h>
#include "tc_ns_log.h"
#include "tc_ns_client.h"
#include "agent.h" /* for get_proc_dpath */
#include "ko_adapt.h"

/* for crypto */
struct crypto_shash *g_shash_handle;
bool g_shash_handle_state;
struct mutex g_shash_handle_lock;

#ifdef CONFIG_SELINUX_ADAPT

int security_context_to_sid(const char *scontext, u32 scontext_len,
	u32 *out_sid, gfp_t gfp);

int check_proc_selinux_access(struct task_struct *task, const char *context)
{
	u32 sid;
	u32 tid;
	int rc;

	if (!task || !context)
		return -EACCES;

	security_task_getsecid(task, &sid);
	rc = security_context_to_sid(context, strlen(context), &tid, GFP_KERNEL);
	if (rc) {
		tloge("convert context to sid failed\n");
		return rc;
	}

	if (sid != tid) {
		tloge("invalid access process judged by selinux side\n");
		return -EACCES;
	}

	return EOK;
}

#else

int check_proc_selinux_access(struct task_struct *task, const char *context)
{
	return 0;
}

#endif

void init_crypto_hash_lock(void)
{
	mutex_init(&g_shash_handle_lock);
}

void mutex_crypto_hash_lock(void)
{
	mutex_lock(&g_shash_handle_lock);
}

void mutex_crypto_hash_unlock(void)
{
	mutex_unlock(&g_shash_handle_lock);
}

/* begin: prepare crypto context */
struct crypto_shash *get_shash_handle(void)
{
	return g_shash_handle;
}

void tee_exit_shash_handle(void)
{
	if (g_shash_handle) {
		crypto_free_shash(g_shash_handle);
		g_shash_handle_state = false;
		g_shash_handle = NULL;
	}
}

int tee_init_shash_handle(char *hash_type)
{
	long rc;

	if (!hash_type) {
		tloge("tee init crypto: error input parameter\n");
		return -EFAULT;
	}

	mutex_crypto_hash_lock();
	if (g_shash_handle_state) {
		mutex_crypto_hash_unlock();
		return 0;
	}

	g_shash_handle = crypto_alloc_shash(hash_type, 0, 0);
	if (IS_ERR_OR_NULL(g_shash_handle)) {
		rc = PTR_ERR(g_shash_handle);
		tloge("Can not allocate %s reason: %ld\n", hash_type, rc);
		mutex_crypto_hash_unlock();
		return rc;
	}
	g_shash_handle_state = true;

	mutex_crypto_hash_unlock();
	return 0;
}
/* end: prepare crypto context */

/* begin: Calculate path hash */
static long pack_path_cert(bool is_hidl_srvc, char *path_cert,
	unsigned long cert_len)
{
	char *dpath = NULL;
	char *path = NULL;
	const struct cred *cred = NULL;
	int message_size;

	path = kmalloc(MAX_PATH_SIZE, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)path)) {
		tloge("path kmalloc fail\n");
		return -EFAULT;
	}

	dpath = get_proc_dpath(path, MAX_PATH_SIZE);
	if (IS_ERR_OR_NULL(dpath)) {
		kfree(path);
		return -EACCES;
	}

	get_task_struct(current);
	cred = koadpt_get_task_cred(current);
	if (!cred) {
		tloge("cred is NULL\n");
		kfree(path);
		put_task_struct(current);
		return -EACCES;
	}

	if (is_hidl_srvc) {
		message_size = snprintf_s(path_cert, BUF_MAX_SIZE,
			BUF_MAX_SIZE - 1, "%s%u",  dpath, cred->uid.val);
	} else {
		message_size = snprintf_s(path_cert, BUF_MAX_SIZE,
			BUF_MAX_SIZE - 1, "%s%s%u",
			current->comm, dpath, cred->uid.val);
	}

	put_cred(cred);
	put_task_struct(current);
	kfree(path);

	return (long)message_size;
}

static int proc_calc_path_hash(const unsigned char *data, unsigned long data_len,
	unsigned char *digest, unsigned int dig_len)
{
	int rc;
	size_t ctx_size;
	size_t desc_size;
	struct sdesc {
		struct shash_desc shash;
		char ctx[];
	};
	struct sdesc *desc = NULL;
	bool check_value = false;

	check_value = (!data_len || dig_len != SHA256_DIGEST_LENTH);
	if (check_value == true) {
		tloge("bad parameter len\n");
		return -EFAULT;
	}

	if (tee_init_shash_handle("sha256")) {
		tloge("init tee crypto failed\n");
		return -EFAULT;
	}

	ctx_size = crypto_shash_descsize(g_shash_handle);
	desc_size = sizeof(desc->shash) + ctx_size;
	if (desc_size < sizeof(desc->shash) || desc_size < ctx_size) {
		tloge("desc_size flow\n");
		return -ENOMEM;
	}

	desc = kzalloc(desc_size, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)desc)) {
		tloge("alloc desc failed\n");
		return -ENOMEM;
	}

	desc->shash.tfm = g_shash_handle;
	desc->shash.flags = 0;
	rc = crypto_shash_digest(&desc->shash, data, data_len, digest);

	kfree(desc);
	return rc;
}

int calc_path_hash(bool is_hidl_srvc, unsigned char *digest,
	unsigned int dig_len)
{
	char *path_cert = NULL;
	int ret;
	long packet_size;

	if (!digest || dig_len != SHA256_DIGEST_LENTH)
		return -EFAULT;

	path_cert = kzalloc(BUF_MAX_SIZE, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)path_cert)) {
		tloge("ca_cert kmalloc fail\n");
		return -EFAULT;
	}

	packet_size = pack_path_cert(is_hidl_srvc, path_cert, BUF_MAX_SIZE);
	if (packet_size <= 0) {
		kfree(path_cert);
		return -EFAULT;
	}

	ret = proc_calc_path_hash(path_cert, (unsigned long)packet_size,
		digest, dig_len);
	if (ret) {
		kfree(path_cert);
		return -ret;
	}

	kfree(path_cert);
	return EOK;
}
/* end: Calculate path hash */

/* begin: Calculate the SHA256 file digest */
static int prepare_desc(struct sdesc **desc)
{
	size_t size;
	size_t shash_size;

	shash_size = crypto_shash_descsize(g_shash_handle);
	size = sizeof((*desc)->shash) + shash_size;
	if (size < sizeof((*desc)->shash) || size < shash_size) {
		tloge("size flow\n");
		return -ENOMEM;
	}

	*desc = kzalloc(size, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)(*desc))) {
		tloge("alloc desc failed\n");
		return -ENOMEM;
	}

	return EOK;
}

#define PINED_PAGE_NUMBER 1
static int update_task_hash(struct mm_struct *mm,
	struct task_struct *cur_struct, struct shash_desc *shash)
{
	int rc = -1;
	unsigned long start_code;
	unsigned long end_code;
	unsigned long code_size;
	unsigned long in_size;
	struct page *ptr_page = NULL;
	void *ptr_base = NULL;

	start_code = mm->start_code;
	end_code = mm->end_code;
	code_size = end_code - start_code;
	if (code_size == 0) {
		tloge("bad code size\n");
		return -EINVAL;
	}

	while (start_code < end_code) {
		/* Get a handle of the page we want to read */
		rc = get_user_pages_remote(cur_struct, mm, start_code,
			(unsigned long)PINED_PAGE_NUMBER, FOLL_FORCE,
			&ptr_page, NULL, NULL);
		if (rc != PINED_PAGE_NUMBER) {
			tloge("get user pages error[0x%x]\n", rc);
			rc = -EFAULT;
			break;
		}

		ptr_base = kmap_atomic(ptr_page);
		if (!ptr_base) {
			rc = -EFAULT;
			put_page(ptr_page);
			break;
		}

		in_size = (code_size > PAGE_SIZE) ? PAGE_SIZE : code_size;
		rc = crypto_shash_update(shash, ptr_base, in_size);
		if (rc) {
			kunmap_atomic(ptr_base);
			put_page(ptr_page);
			break;
		}

		kunmap_atomic(ptr_base);
		put_page(ptr_page);
		start_code += in_size;
		code_size = end_code - start_code;
	}
	return rc;
}

int calc_task_hash(unsigned char *digest, uint32_t dig_len,
	struct task_struct *cur_struct)
{
	struct mm_struct *mm = NULL;
	struct sdesc *desc = NULL;
	bool check_value = false;
	int rc;

	check_value = (!cur_struct || !digest ||
		dig_len != SHA256_DIGEST_LENTH);
	if (check_value) {
		tloge("tee hash: input param is error\n");
		return -EFAULT;
	}

	mm = get_task_mm(cur_struct);
	if (!mm) {
		if (memset_s(digest, dig_len, 0, MAX_SHA_256_SZ))
			return -EFAULT;
		tloge("kernel proc need not check\n");
		return EOK;
	}

	if (prepare_desc(&desc) != EOK) {
		mmput(mm);
		return -ENOMEM;
	}

	desc->shash.tfm = g_shash_handle;
	desc->shash.flags = 0;
	if (crypto_shash_init(&desc->shash)) {
		tloge("shash init failed\n");
		rc = -ENOMEM;
		goto free_res;
	}

	down_read(&mm->mmap_sem);
	if (update_task_hash(mm, cur_struct, &desc->shash)) {
		up_read(&mm->mmap_sem);
		rc = -ENOMEM;
		goto free_res;
	}
	up_read(&mm->mmap_sem);

	rc = crypto_shash_final(&desc->shash, digest);
free_res:
	mmput(mm);
	kfree(desc);
	return rc;
}
/* end: Calculate the SHA256 file digest */
