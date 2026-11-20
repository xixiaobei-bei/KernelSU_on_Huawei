/*
 * client_hash_auth.c
 *
 * function for CA code hash auth
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
#include "client_hash_auth.h"
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/rwsem.h>

#include <linux/mm.h>
#include <linux/dcache.h>
#include <linux/mm_types.h>
#include <linux/highmem.h>
#include <linux/cred.h>
#include <linux/slab.h>
#include <linux/sched/mm.h>

#include "tc_ns_log.h"
#include "auth_base_impl.h"

#ifdef CONFIG_ANDROID_HIDL

/* hash for : /vendor/bin/hw/vendor.huawei.hardware.libteec@3.0-service1000 */
static unsigned char g_hidl_path_hash[SHA256_DIGEST_LENTH] = {
	0x0a, 0xce, 0x6c, 0x1a,
	0x6a, 0x54, 0x0f, 0xbd,
	0xe3, 0xc8, 0xf8, 0x5e,
	0xe1, 0xf3, 0x0a, 0x3a,
	0x43, 0x6a, 0x45, 0x1c,
	0x26, 0x7e, 0xa7, 0x08,
	0xfa, 0x01, 0x3b, 0x25,
	0x38, 0x51, 0x9e, 0xcb,
};

static unsigned char g_hidl_calc_hash[SHA256_DIGEST_LENTH];
static bool g_hidl_hash_calced = false;
DEFINE_MUTEX(g_hidl_calc_lock);

static int check_hidl_code_hash()
{
	unsigned char digest[SHA256_DIGEST_LENTH] = {0};

	if (!g_hidl_hash_calced)
		return CHECK_ACCESS_SUCC;

	if (calc_task_hash(digest, (uint32_t)SHA256_DIGEST_LENTH,
		current)) {
		tloge("calc task hash failed\n");
		return CHECK_ACCESS_FAIL;
	}

	if (memcmp(digest, g_hidl_calc_hash, SHA256_DIGEST_LENTH)) {
		tloge("compare libteec hidl hash error\n");
		return CHECK_ACCESS_FAIL;
	}

	return CHECK_ACCESS_SUCC;
}

static int calc_hidl_process_hash(void)
{
	mutex_lock(&g_hidl_calc_lock);

	if (g_hidl_hash_calced) {
		mutex_unlock(&g_hidl_calc_lock);
		return CHECK_ACCESS_SUCC;
	}

	if (memset_s((void *)g_hidl_calc_hash,
		sizeof(g_hidl_calc_hash), 0x00,
		sizeof(g_hidl_calc_hash))) {
		tloge("memset failed\n");
		mutex_unlock(&g_hidl_calc_lock);
		return CHECK_ACCESS_FAIL;
	}

	g_hidl_hash_calced = !calc_task_hash(g_hidl_calc_hash,
		(uint32_t)SHA256_DIGEST_LENTH, current);
	if (!g_hidl_hash_calced) {
		tloge("calc libteec hidl hash failed\n");
		mutex_unlock(&g_hidl_calc_lock);
		return CHECK_ACCESS_FAIL;
	}

	mutex_unlock(&g_hidl_calc_lock);
	return CHECK_ACCESS_SUCC;
}

static int check_hidl_path_access(void)
{
	unsigned char digest[SHA256_DIGEST_LENTH] = {0};

	if (calc_path_hash(true, digest, SHA256_DIGEST_LENTH)) {
		tloge("calc path hash failed\n");
		return CHECK_PATH_HASH_FAIL;
	}

	if (memcmp(digest, g_hidl_path_hash, SHA256_DIGEST_LENTH)) {
		tlogd("process is not libteec hidl service, keep going\n");
		return ENTER_BYPASS_CHANNEL;
	}

	if (check_proc_selinux_access(current,
		"u:r:hal_libteec_default:s0")) {
		tloge("check libteec hidl service seclabel failed\n");
		return CHECK_SECLABEL_FAIL;
	}

	return CHECK_ACCESS_SUCC;
}

static int check_proc_state(bool is_hidl_srvc, struct task_struct **hidl_struct,
	const struct tc_ns_client_context *context)
{
	bool check_value = false;

	if (is_hidl_srvc) {
		rcu_read_lock();
		*hidl_struct = pid_task(find_vpid(context->calling_pid),
			PIDTYPE_PID);
		check_value = !*hidl_struct ||
			(*hidl_struct)->state == TASK_DEAD;
		if (check_value) {
			tloge("task is dead\n");
			rcu_read_unlock();
			return -EFAULT;
		}

		get_task_struct(*hidl_struct);
		rcu_read_unlock();
		return EOK;
	}

	check_value = (context->calling_pid && current->mm);
	if (check_value) {
		tloge("non hidl service, non-zero callingpid, reject\n");
		return -EFAULT;
	}

	return EOK;
}

static int get_hidl_client_task(struct tc_ns_client_context *context,
	bool is_hidl_srvc, struct task_struct **cur_struct)
{
	int ret;
	struct task_struct *hidl_struct = NULL;

	ret = check_proc_state(is_hidl_srvc, &hidl_struct, context);
	if (ret)
		return ret;

	if (hidl_struct)
		*cur_struct = hidl_struct;
	else
		*cur_struct = current;

	return EOK;
}

int check_hidl_access()
{
	int ret;

	if (!current->mm) {
		tlogd("kernel thread need not check\n");
		return ENTER_BYPASS_CHANNEL;
	}

	ret = check_hidl_path_access();
	if (ret != CHECK_ACCESS_SUCC)
		return ret;

	if (calc_hidl_process_hash() != CHECK_ACCESS_SUCC) {
		tloge("calc hidl process hash failed\n");
		return CHECK_ACCESS_FAIL;
	}

	if (check_hidl_code_hash() != CHECK_ACCESS_SUCC) {
		tloge("check hidl process hash failed\n");
		return CHECK_CODE_HASH_FAIL;
	}

	return CHECK_ACCESS_SUCC;
}
#endif

#define LIBTEEC_CODE_PAGE_SIZE 8
#define DEFAULT_TEXT_OFF 0
#define LIBTEEC_NAME_MAX_LEN 50
const char g_libso[KIND_OF_SO][LIBTEEC_NAME_MAX_LEN] = {
						"libteec_vendor.so",
						"libteec.huawei.so",
};

static int find_lib_code_area(struct mm_struct *mm,
	struct vm_area_struct **lib_code_area, int so_index)
{
	struct vm_area_struct *vma = NULL;
	bool is_valid_vma = false;
	bool is_so_exists = false;
	bool param_check = (!mm || !mm->mmap ||
		!lib_code_area || so_index >= KIND_OF_SO);

	if (param_check) {
		tloge("illegal input params\n");
		return -EFAULT;
	}
	for (vma = mm->mmap; vma; vma = vma->vm_next) {
		is_valid_vma = (vma->vm_file &&
			vma->vm_file->f_path.dentry &&
			vma->vm_file->f_path.dentry->d_name.name);
		if (is_valid_vma) {
			is_so_exists = !strcmp(g_libso[so_index],
				vma->vm_file->f_path.dentry->d_name.name);
			if (is_so_exists && (vma->vm_flags & VM_EXEC)) {
				*lib_code_area = vma;
				tlogd("so name is %s\n",
					vma->vm_file->f_path.dentry->d_name.name);
				return EOK;
			}
		}
	}
	return -EFAULT;
}

struct get_code_info {
	unsigned long code_start;
	unsigned long code_end;
	unsigned long code_size;
};
static int update_so_hash(struct mm_struct *mm,
	struct task_struct *cur_struct, struct shash_desc *shash, int so_index)
{
	struct vm_area_struct *vma = NULL;
	int rc = -EFAULT;
	struct get_code_info code_info;
	unsigned long in_size;
	struct page *ptr_page = NULL;
	void *ptr_base = NULL;

	if (find_lib_code_area(mm, &vma, so_index)) {
		tlogd("get lib code vma area failed\n");
		return -EFAULT;
	}

	code_info.code_start = vma->vm_start;
	code_info.code_end = vma->vm_end;
	code_info.code_size = code_info.code_end - code_info.code_start;

	while (code_info.code_start < code_info.code_end) {
		// Get a handle of the page we want to read
		rc = get_user_pages_remote(cur_struct, mm, code_info.code_start,
			1, FOLL_FORCE, &ptr_page, NULL, NULL);
		if (rc != 1) {
			tloge("get user pages locked error[0x%x]\n", rc);
			rc = -EFAULT;
			break;
		}

		ptr_base = kmap_atomic(ptr_page);
		if (!ptr_base) {
			rc = -EFAULT;
			put_page(ptr_page);
			break;
		}
		in_size = (code_info.code_size > PAGE_SIZE) ? PAGE_SIZE : code_info.code_size;

		rc = crypto_shash_update(shash, ptr_base, in_size);
		if (rc) {
			kunmap_atomic(ptr_base);
			put_page(ptr_page);
			break;
		}
		kunmap_atomic(ptr_base);
		put_page(ptr_page);
		code_info.code_start += in_size;
		code_info.code_size = code_info.code_end - code_info.code_start;
	}
	return rc;
}

/* Calculate the SHA256 library digest */
static int calc_task_so_hash(unsigned char *digest, uint32_t dig_len,
	struct task_struct *cur_struct, int so_index)
{
	struct mm_struct *mm = NULL;
	int rc;
	size_t size;
	size_t shash_size;
	struct sdesc *desc = NULL;

	if (!digest || dig_len != SHA256_DIGEST_LENTH) {
		tloge("tee hash: digest is NULL\n");
		return -EFAULT;
	}

	shash_size = crypto_shash_descsize(get_shash_handle());
	size = sizeof(desc->shash) + shash_size;
	if (size < sizeof(desc->shash) || size < shash_size) {
		tloge("size overflow\n");
		return -ENOMEM;
	}

	desc = kzalloc(size, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)desc)) {
		tloge("alloc desc failed\n");
		return -ENOMEM;
	}

	desc->shash.tfm = get_shash_handle();
	desc->shash.flags = 0;
	if (crypto_shash_init(&desc->shash)) {
		kfree(desc);
		return -EFAULT;
	}

	mm = get_task_mm(cur_struct);
	if (!mm) {
		tloge("so does not have mm struct\n");
		if (memset_s(digest, MAX_SHA_256_SZ, 0, dig_len))
			tloge("memset digest failed\n");
		kfree(desc);
		return -EFAULT;
	}

	down_read(&mm->mmap_sem);
	rc = update_so_hash(mm, cur_struct, &desc->shash, so_index);
	up_read(&mm->mmap_sem);
	mmput(mm);
	if (!rc)
		rc = crypto_shash_final(&desc->shash, digest);

	kfree(desc);
	return rc;
}

static int proc_calc_hash(uint8_t kernel_api, struct tc_ns_session *session,
	struct task_struct *cur_struct)
{
	int rc, i;
	int so_found = 0;

	mutex_crypto_hash_lock();
	if (kernel_api == TEE_REQ_FROM_USER_MODE) {
		for (i = 0; so_found < NUM_OF_SO && i < KIND_OF_SO; i++) {
			rc = calc_task_so_hash(session->auth_hash_buf + MAX_SHA_256_SZ * so_found,
				(uint32_t)SHA256_DIGEST_LENTH, cur_struct, i);
			if (!rc)
				so_found++;
		}
		if (so_found != NUM_OF_SO)
			tlogd("so library found: %d\n", so_found);
	} else {
		tlogd("request from kernel\n");
	}


	rc = calc_task_hash(session->auth_hash_buf + MAX_SHA_256_SZ * NUM_OF_SO,
		(uint32_t)SHA256_DIGEST_LENTH, cur_struct);
	if (rc) {
		mutex_crypto_hash_unlock();
		tloge("tee calc ca hash failed\n");
		return -EFAULT;
	}
	mutex_crypto_hash_unlock();
	return EOK;
}

int calc_client_auth_hash(struct tc_ns_dev_file *dev_file,
	struct tc_ns_client_context *context, struct tc_ns_session *session)
{
	int ret;
	struct task_struct *cur_struct = NULL;
	bool check = false;
#ifdef CONFIG_ANDROID_HIDL
	bool is_hidl_srvc = false;
#endif
	check = (!dev_file || !context || !session);
	if (check) {
		tloge("bad params\n");
		return -EFAULT;
	}

	if (tee_init_shash_handle("sha256")) {
		tloge("init code hash error\n");
		return -EFAULT;
	}

#ifdef CONFIG_ANDROID_HIDL
	ret = check_hidl_access();
	if (ret != CHECK_ACCESS_SUCC) {
		if (ret != ENTER_BYPASS_CHANNEL) {
			tloge("libteec hidl service may be exploited ret 0x%x\n", ret);
			return -EACCES;
		}
		/* vendor ca take this branch */
	} else {
		is_hidl_srvc = true;
	}

	ret = get_hidl_client_task(context, is_hidl_srvc, &cur_struct);
	if (ret)
		return -EFAULT;
#else
	cur_struct = current;
#endif

	ret = proc_calc_hash(dev_file->kernel_api, session, cur_struct);
#ifdef CONFIG_ANDROID_HIDL
	if (is_hidl_srvc)
		put_task_struct(cur_struct);
#endif
	return ret;
}
