/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: implement the chipsets's common interface of HISI
 * Author: qidechun
 * Create: 2019-03-05
 */

/* ---- includes ---- */
#include <hwbootfail/chipsets/common/adapter_common.h>
#include <linux/fs.h>
#include <linux/printk.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/semaphore.h>
#include <linux/vmalloc.h>
#include <crypto/hash.h>
#include <crypto/sha.h>
#include <hwbootfail/chipsets/common/bootfail_common.h>
#include <hwbootfail/core/boot_interface.h>

/* ---- local macroes ---- */
#define SHA256_DATA_LEN 32

/* ---- local prototypes  ---- */
/* ---- local function prototypes ---- */
/* ---- local variables ---- */
static DEFINE_SEMAPHORE(rrecord_sem);

/* ---- function definitions ---- */
static int is_bootloader(void)
{
	return 0;
}

static void sha256(unsigned char *pout, unsigned long out_len,
	const void *pin, unsigned long in_len)
{
	struct crypto_shash *tfm = NULL;
	struct shash_desc *desc = NULL;
	int ret = 0;
	size_t desc_size;

	tfm = crypto_alloc_shash("sha256", 0, 0);
	if (IS_ERR(tfm)) {
		ret = PTR_ERR(tfm);
		return;
	}

	desc_size = crypto_shash_descsize(tfm) + sizeof(*desc);
	desc = kzalloc(desc_size, GFP_KERNEL);
	if (desc == NULL) {
		kfree(tfm);
		return;
	}
	desc->tfm = tfm;
	desc->flags = 0;
	ret = crypto_shash_init(desc);
	if (ret != 0) {
		kfree(tfm);
		kfree(desc);
		return;
	}
	ret = crypto_shash_update(desc, pin, in_len);
	ret = crypto_shash_final(desc, pout);
	crypto_free_shash(desc->tfm);
	kfree(desc);
}

static void lock_rrecord(void)
{
	down(&rrecord_sem);
}

static void unlock_rrecord(void)
{
	up(&rrecord_sem);
}

static int is_safe_mode_supported(void)
{
	return (is_bopd_supported() != 0) ? 0 : 1;
}

static void save_misc_msg(const char *pmsg)
{
}

static int bootfail_memset(void *dst, unsigned long dst_max,
	int c, unsigned long count)
{
	size_t cnt;

	if (dst == NULL || dst_max == 0UL) {
		print_invalid_params("dts: %p, dst_max: %lu\n",
			dst, dst_max);
		return -1;
	}

	cnt = min((size_t)dst_max, (size_t)count);
	(void)memset(dst, c, cnt);

	return 0;
}

static int bootfail_memcpy(void *dst, unsigned long dst_max,
	const void *src,
	unsigned long count)
{
	size_t cnt;

	if (dst == NULL || src == NULL || dst_max == 0UL) {
		print_invalid_params("dts: %p, src: %p, dst_max: %lu\n",
			dst, src, dst_max);
		return -1;
	}

	cnt = min((size_t)dst_max, (size_t)count);
	(void)memcpy(dst, src, cnt);

	return 0;
}

static int bootfail_strncpy(char *dst,  unsigned long dst_max,
	const char *src, unsigned long count)
{
	size_t cnt;

	if (dst == NULL || src == NULL || dst_max == 0UL) {
		print_invalid_params("dts: %p, src: %p, dst_max: %lu\n",
			dst, src, dst_max);
		return -1;
	}

	cnt = min((size_t)dst_max - 1, (size_t)count);
	(void)strncpy(dst, src, (size_t)cnt);

	return 0;
}

static int bootfail_memcmp(const void *dst,
	const void *src,
	unsigned long count)
{
	size_t cnt = (size_t)count;

	return memcmp(dst, src, cnt);
}

static unsigned long bootfail_strlen(const char *s)
{
	return (unsigned long)strlen(s);
}

static void bootfail_print(const char *fmt, ...)
{
	va_list args;
	char buf[BF_SIZE_256] = {0};

	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf) - 1, fmt, args);
	va_end(args);
	buf[sizeof(buf) - 1] = '\0';

	printk(KERN_ERR "bootfail: %s", buf);
}

static void get_adapter(struct adapter *padp)
{
	padp->bfi_part.ops.read = read_part;
	padp->bfi_part.ops.write = write_part;
	padp->pyhs_mem_info.ops.read = read_from_phys_mem;
	padp->pyhs_mem_info.ops.write = write_to_phys_mem;
	padp->sec_funcs.memset_s = bootfail_memset;
	padp->sec_funcs.memcpy_s = bootfail_memcpy;
	padp->sec_funcs.strncpy_s = bootfail_strncpy;
	padp->sec_funcs.memcmp = bootfail_memcmp;
	padp->sec_funcs.strlen = bootfail_strlen;
	padp->mem_ops.malloc = vmalloc;
	padp->mem_ops.free = vfree;
	padp->mutex_ops.lock = lock_rrecord;
	padp->mutex_ops.unlock = unlock_rrecord;
	padp->is_boot_detector_enabled = is_boot_detector_enabled;
	padp->is_boot_recovery_enabled = is_boot_recovery_enabled;
	padp->is_bootloader = is_bootloader;
	padp->print = bootfail_print;
	padp->sha256 = sha256;
	padp->save_misc_msg = save_misc_msg;
	padp->is_bopd_supported = is_bopd_supported;
	padp->is_safe_mode_supported = is_safe_mode_supported;
}

int common_adapter_init(struct adapter *padp)
{
	if (padp == NULL) {
		print_invalid_params("padp: %p\n", padp);
		return -1;
	}

	get_adapter(padp);

	return 0;
}
