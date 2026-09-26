/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: bl31_hibernate header file
 * Author : security-ap
 * Create : 2020/03/06
 */

#ifndef __HISI_BL31_HIBERNATE_H__
#define __HISI_BL31_HIBERNATE_H__

struct hibernate_meminfo_t {
	/* pyhs addr of kernel and bl31 shamre memory */
	phys_addr_t paddr;
	/* addr size of kernel and bl31 shamre memory */
	u64 addr_size;
	/* virt addr of kernel and bl31 shamre memory */
	void *smem_vaddr;
	/* virt addr of kernel memory for kernel management */
	void *kmem_vaddr;
};

#ifdef CONFIG_HISI_BL31_HIBERNATE
int bl31_hibernate_freeze(void);
int bl31_hibernate_restore(void);
struct hibernate_meminfo_t get_bl31_hibernate_mem_info(void);
int bl31_hibernate_store_k(void);
int bl31_hibernate_clean_k(void);
int bl31_hibernate_get_k(void);
#else
static inline int bl31_hibernate_freeze(void) { return 0; }
static inline int bl31_hibernate_restore(void) { return 0; }
static inline struct hibernate_meminfo_t get_bl31_hibernate_mem_info(void) { return 0; }
static inline int bl31_hibernate_store_k(void) { return 0; }
static inline int bl31_hibernate_clean_k(void) { return 0; }
static inline int bl31_hibernate_get_k(void) { return 0; }
#endif

#endif /* __HISI_BL31_HIBERNATE_H__ */
