/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: Use prmem to enhance CFI protection.
 * Create: 2020/10/19
 */

#include <linux/bug.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/hisi/cfi_harden.h>
#include <linux/hisi/prmem.h>

#define CFI_CHECK_POOL_SIZE  \
	(CONFIG_CFI_CHECK_CACHE_NUM * sizeof(struct safe_cfi_area))
#define CFI_GUARD_SIZE       (2 * PAGE_SIZE)
#define CFI_CHECK_MAX_SIZE   (CFI_CHECK_POOL_SIZE + CFI_GUARD_SIZE)
#define CFI_SHADOW_POOL_SIZE (SHADOW_PAGES * PAGE_SIZE)

PRMEM_POOL(cfi_check_pool, start_wr_no_recl, sizeof(void *),
	   CFI_CHECK_POOL_SIZE, CFI_CHECK_MAX_SIZE);
PRMEM_CACHE(cfi_check_cache, &cfi_check_pool,
	    sizeof(struct safe_cfi_area), sizeof(void *));
PRMEM_POOL(cfi_shadow_pool, wr_recl, sizeof(void *),
	   CFI_SHADOW_POOL_SIZE, PRMEM_NO_CAP);

void cfi_protect_shadow_pages(void *addr)
{
	prmem_protect_addr(addr);
}

void *cfi_alloc_shadow_pages(void)
{
	return pmalloc(&cfi_shadow_pool,
		       CFI_SHADOW_POOL_SIZE,
		       PRMEM_FREEABLE_NODE);
}

void cfi_free_shadow_pages(void *addr)
{
	pfree(addr);
}

static inline bool is_cfi_valid(void *p, __kernel_size_t n)
{
	return is_wr(p, n);
}

cfi_check_fn fetch_cfi_check_fn(struct module *mod)
{
	/*
	 * In order to prevent forging the entire malicious module,
	 * the verification of the module is also necessary in the future.
	 */
	if (WARN(!is_cfi_valid(mod->safe_cfi_area, sizeof(struct safe_cfi_area)) ||
		 (mod != mod->safe_cfi_area->owner),
		 "Attempt to alter cfi_check!"))
		return 0;
	return mod->safe_cfi_area->cfi_check;
}

void cfi_harden_module_init(struct module *mod, uintptr_t sym)
{
	mod->safe_cfi_area = (struct safe_cfi_area *)
	prmem_cache_alloc(&cfi_check_cache, PRMEM_NO_FLAGS);
	if (unlikely(!mod->safe_cfi_area)) {
		pr_err("%s, Memory allocation failed in hkip cfi harden\n",
		       __func__);
		BUG();
	}
	wr_assign(mod->safe_cfi_area->cfi_check, (cfi_check_fn)sym);
	wr_assign(mod->safe_cfi_area->owner, mod);
}

void cfi_harden_module_cleanup(struct module *mod)
{
	prmem_cache_free(&cfi_check_cache, mod->safe_cfi_area);
}

/* Init cfi harden feature during kernel init */
void cfi_harden_init(void)
{
	if (unlikely(prmem_cache_preload(&cfi_check_cache,
					CONFIG_CFI_CHECK_CACHE_NUM,
					PRMEM_NO_FLAGS)
					!= CONFIG_CFI_CHECK_CACHE_NUM)) {
		pr_err("%s, cfi_check initialization failed\n", __func__);
		BUG();
	}
}
