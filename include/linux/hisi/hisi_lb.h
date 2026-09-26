/*
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __HISI_LB_H
#define __HISI_LB_H
#include <asm/pgtable-prot.h>
#include <linux/mm_types.h>
#include <linux/scatterlist.h>

/* it need same with dts */
enum lb_pid {
	PID_BY_PASS = 0,

	PID_CAMAIP = 1,
	PID_GPUFBO  = 2,
	PID_GPUTXT  = 3,
	PID_IDISPLAY = 4,
	PID_NPU     = 5,
	PID_VIDEO   = 6,
	PID_CAMTOF = 7,
	PID_TINY    = 8,
	PID_AUDIO   = 9,
	PID_VOICE   = 10,
	PID_ISPNN   = 11,

	PID_MAX,
};

struct lb_policy_config {
	unsigned int pid;
	unsigned int quota;
	unsigned int prio;
	unsigned int available;
};

enum lb_prio {
	LB_CACHE_LOW_PRIO = 0x99,
	LB_CACHE_HIGH_PRIO = 0x50,
	LB_BUF_LOW_PRIO = 0x40,
	LB_BUF_HIGH_PRIO = 0x20,
};

#ifdef CONFIG_HISI_LB
extern void __lb_assert_page(struct page *pg);
extern void __lb_assert_phys(phys_addr_t phys);
extern void __lb_assert_pte(pte_t pte);
extern void *lb_page_to_virt(struct page *page);
extern unsigned int lb_page_to_gid(struct page *page);
extern int lb_request_quota(unsigned int pid);
extern int lb_release_quota(unsigned int policy_id);
extern struct page *lb_alloc_pages(unsigned int pid, gfp_t gfp_mask,
		unsigned int order);
extern int lb_free_pages(unsigned int pid, struct page *pages,
		unsigned int order);
extern unsigned long lb_pte_attr(phys_addr_t phy_addr);
extern int lb_prot_build(struct page *pages, pgprot_t *pageprot);
extern int lb_pid_prot_build(unsigned int pid, pgprot_t *pageprot);
extern int lb_sg_attach(unsigned int pid, struct scatterlist *sgl,
		unsigned int nents);
extern int lb_sg_detach(unsigned int pid, struct scatterlist *sgl,
		unsigned int nents);
extern int lb_pages_attach(unsigned int pid, struct page *pg, size_t count);
extern int lb_pages_detach(unsigned int pid, struct page *pg, size_t count);
extern u64 lb_pid_to_gidphys(u32 pid);
extern unsigned int is_lb_available(void);
extern unsigned int lb_get_available_size(void);
extern int lb_reset_master_policy(unsigned int pid);
extern int lb_set_master_policy(unsigned int pid, unsigned int quota,
		unsigned int prio);
extern unsigned int lb_get_master_policy(unsigned int pid, unsigned int type);
extern int lb_down_policy_prio(unsigned int pid);
extern int lb_up_policy_prio(unsigned int pid);
#else
static inline void __lb_assert_page(struct page *pg) {}
static inline void __lb_assert_phys(phys_addr_t phys) {}
static inline void __lb_assert_pte(pte_t pte) {}
static inline void *lb_page_to_virt(struct page *page) { return 0; }
static inline unsigned int lb_page_to_gid(struct page *page) { return 0; }
static inline int lb_request_quota(unsigned int pid) { return 0; }
static inline int lb_release_quota(unsigned int policy_id) { return 0; }
static inline struct page *lb_alloc_pages(unsigned int pid, gfp_t gfp_mask,
		unsigned int order) { return 0; }
static inline int lb_free_pages(unsigned int pid, struct page *pages,
		unsigned int order) { return 0; }
static inline unsigned long lb_pte_attr(phys_addr_t phy_addr) { return 0; }
static inline int lb_prot_build(struct page *pages, pgprot_t *pageprot)
{ return 0; }
static inline int lb_pid_prot_build(struct page *pages, pgprot_t *pageprot)
{ return 0; }
static inline int lb_sg_attach(unsigned int pid, struct scatterlist *sgl,
		unsigned int nents) { return 0; }
static inline int lb_sg_detach(unsigned int pid, struct scatterlist *sgl,
		unsigned int nents) { return 0; }
static inline int lb_pages_attach(unsigned int pid, struct page *pg,
		size_t count) { return 0; }
static inline int lb_pages_detach(unsigned int pid, struct page *pg,
		size_t count) { return 0; }
static inline u64 lb_pid_to_gidphys(u32 pid) { return 0; }
static inline unsigned int is_lb_available(void) { return 0; }
static inline unsigned int lb_get_available_size(void) { return 0; }
static inline int lb_reset_master_policy(unsigned int pid) { return 0; }
static inline int lb_set_master_policy(unsigned int pid, unsigned int quota,
		unsigned int prio) { return 0; }
static inline unsigned int lb_get_master_policy(unsigned int pid,
		unsigned int type) { return 0; }
static inline int lb_down_policy_prio(unsigned int pid) { return 0; }
static inline int lb_up_policy_prio(unsigned int pid) { return 0; }
#endif

#ifdef CONFIG_HISI_LB_L3_EXTENSION
extern void lb_ci_cache_exclusive(void);
#else
static inline void lb_ci_cache_exclusive(void) { return; }
#endif

#ifdef CONFIG_HISI_LB_GEMINI
extern int lb_gid_enable(unsigned int pid);
extern int lb_gid_bypass(unsigned int pid);
extern int perf_ctrl_lb_set_policy(const void __user *uarg);
extern int perf_ctrl_lb_reset_policy(const void __user *uarg);
extern int perf_ctrl_lb_lite_info(void __user *uarg);
#else
static inline int lb_gid_enable(unsigned int pid) { return 0; }
static inline int lb_gid_bypass(unsigned int pid) { return 0; }
static inline int perf_ctrl_lb_set_policy(const void __user *uarg) { return 0; }
static inline int perf_ctrl_lb_reset_policy(const void __user *uarg) { return 0; }
static inline int perf_ctrl_lb_lite_info(void __user *uarg) { return 0; }
#endif

#endif
