/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Use prmem to enhance CFI protection.
 * Author: security-ap
 * Create: 2020/10/19
 */

#ifndef __HKIP_CFI_HARDEN__
#define __HKIP_CFI_HARDEN__

#include <linux/cfi.h>
#include <linux/gfp.h>
#include <linux/module.h>
#include <linux/set_memory.h>

#ifdef CONFIG_CFI_CLANG
#ifdef CONFIG_MODULES
#ifdef CONFIG_HKIP_CFI_HARDEN

void cfi_protect_shadow_pages(void *addr);
void *cfi_alloc_shadow_pages(void);
void cfi_free_shadow_pages(void *addr);
cfi_check_fn fetch_cfi_check_fn(struct module *mod);
void cfi_harden_module_init(struct module *mod, unsigned long sym);
void cfi_harden_module_cleanup(struct module *mod);

#else

static inline void cfi_protect_shadow_pages(void *addr)
{
	set_memory_ro((uintptr_t)addr, SHADOW_PAGES);
}

static inline void *cfi_alloc_shadow_pages(void)
{
	return (void *)(uintptr_t)__get_free_pages(GFP_KERNEL, SHADOW_ORDER);
}

static inline void cfi_free_shadow_pages(void *addr)
{
	set_memory_rw((uintptr_t)addr, SHADOW_PAGES);
	free_pages((uintptr_t)addr, SHADOW_ORDER);
}

static inline cfi_check_fn fetch_cfi_check_fn(struct module *mod)
{
	return mod->cfi_check;
}

static inline void cfi_harden_module_init(struct module *mod, uintptr_t sym)
{
	mod->cfi_check = (cfi_check_fn)sym;
}

static inline void cfi_harden_module_cleanup(struct module *mod)
{
	(void)mod;
}

#endif /* CONFIG_HKIP_CFI_HARDEN */
#endif /* CONFIG_MODULES */
#endif /* CONFIG_CFI_CLANG */

#ifdef CONFIG_HKIP_CFI_HARDEN

void cfi_harden_init(void);

#else

static inline void cfi_harden_init(void)
{
}

#endif /* CONFIG_HKIP_CFI_HARDEN */

#endif /* __HKIP_CFI_HARDEN__ */
