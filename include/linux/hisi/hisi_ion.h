/*
 *
 * Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _LINUX_HISI_ION_H
#define _LINUX_HISI_ION_H

#include <linux/dma-buf.h>
#include <linux/ion.h>
#include <linux/sizes.h>
#include <linux/version.h>

#ifdef CONFIG_HISI_LB
#include <linux/hisi/hisi_lb.h>
enum ion_lb_pid_ids {
	INVALID_PID = -1,
	ION_LB_BYPASS = PID_BY_PASS,
	ION_LB_CAMAIP = PID_CAMAIP,
	ION_LB_GPUFBO = PID_GPUFBO,
	ION_LB_GPUTXT = PID_GPUTXT,
	ION_LB_IDISPLAY = PID_IDISPLAY,
	ION_LB_NPU = PID_NPU,
	ION_LB_VIDEO = PID_VIDEO,
	ION_LB_CAMTOF = PID_CAMTOF,
	ION_LB_TINY = PID_TINY,
	ION_LB_AUDIO = PID_AUDIO,
	ION_LB_VOICE = PID_VOICE,
	ION_LB_MAX = PID_MAX,
};
#endif

/**
 * These are the only ids that should be used for Ion heap ids.
 * The ids listed are the order in which allocation will be attempted
 * if specified. Don't swap the order of heap ids unless you know what
 * you are doing!
 * Id's are spaced by purpose to allow new Id's to be inserted in-between (for
 * possible fallbacks)
 */

enum ion_heap_ids {
	INVALID_HEAP_ID = -1,
	ION_SYSTEM_HEAP_ID = 0,
	ION_SYSTEM_CONTIG_HEAP_ID = 1,
	ION_GRALLOC_HEAP_ID = 2,
	ION_DMA_HEAP_ID = 3,
	ION_DMA_POOL_HEAP_ID = 4,
	ION_IRIS_DAEMON_HEAP_ID = 5,
	ION_CAMERA_DAEMON_HEAP_ID = 6,
	ION_TINY_HEAP_ID = 7,
	ION_VCODEC_HEAP_ID = 8,
	ION_ISP_HEAP_ID = 9,
	ION_FB_HEAP_ID = 10,
	ION_VPU_HEAP_ID = 11,
	ION_JPU_HEAP_ID = 12,
	HISI_ION_HEAP_IOMMU_ID = 13,
	ION_MISC_HEAP_ID = 14,
	ION_DRM_GRALLOC_HEAP_ID = 15,
	ION_DRM_VCODEC_HEAP_ID = 16,
	ION_TUI_HEAP_ID = 17,
	ION_IRIS_HEAP_ID = 18,
	ION_CAMERA_HEAP_ID = 19,
	ION_DRM_HEAP_ID = 20,
	ION_ALGO_HEAP_ID = 21,
	ION_DRM_CPA_HEAP_ID = 22,
	ION_HEAP_ID_RESERVED = 31, /* Bit reserved */
};

/**
 * Macro should be used with ion_heap_ids defined above.
 */
#define ION_HEAP(bit) (1 << (bit))
#define ION_ALL_HEAP (~0U)
#define ION_8K_ALIGN(len)  ALIGN(len, SZ_8K)
#define IOMMU_PAGE_SIZE SZ_8K

#define ION_VMALLOC_HEAP_NAME	"vmalloc"
#define ION_KMALLOC_HEAP_NAME	"kmalloc"
#define ION_GRALLOC_HEAP_NAME   "gralloc"


#define ION_SET_CACHED(__cache)		(__cache | ION_FLAG_CACHED)
#define ION_SET_UNCACHED(__cache)	(__cache & ~ION_FLAG_CACHED)

#define ION_IS_CACHED(__flags)	((__flags) & ION_FLAG_CACHED)

/* struct used for get phys addr of contig heap */
struct ion_phys_data {
	int fd_buffer;
	unsigned int size;
	union {
		unsigned int phys;
		unsigned int phys_l;
	};
	unsigned int phys_h;
};

struct ion_flag_data {
	int shared_fd;
	int flags;
};

struct ion_smart_pool_info_data {
	int water_mark;
};

#define HISI_ION_NAME_LEN 16

struct ion_heap_info_data {
	char name[HISI_ION_NAME_LEN];
	phys_addr_t heap_phy;
	unsigned int  heap_size;
};
struct ion_kern_va_data {
	int handle_id;
	unsigned int kern_va_h;
	unsigned int kern_va_l;
};
struct ion_issupport_iommu_data {
	int is_support_iommu;
};

struct ion_flush_data {
	int fd;
	void *vaddr;
	unsigned int offset;
	unsigned int length;
};


/* user command add for additional use */
enum ION_HISI_CUSTOM_CMD {
	ION_HISI_CUSTOM_PHYS,
	ION_HISI_CLEAN_CACHES,
	ION_HISI_INV_CACHES,
	ION_HISI_CLEAN_INV_CACHES,
	ION_HISI_CUSTOM_GET_KERN_VA,
	ION_HISI_CUSTOM_FREE_KERN_VA,
	ION_HISI_CUSTOM_ISSUPPORT_IOMMU,
	ION_HISI_CUSTOM_GET_MEDIA_HEAP_MODE,
	ION_HISI_CUSTOM_SET_FLAG,
	ION_HISI_CUSTOM_SET_SMART_POOL_INFO,
};

enum ION_HISI_HEAP_MODE {
	ION_CARVEROUT_MODE = 0,
	ION_IOMMU_MODE = 1,
};

enum ion_ta_tag {
	ION_SEC_CMD_PGATBLE_INIT = 0,
	ION_SEC_CMD_ALLOC,
	ION_SEC_CMD_FREE,
	ION_SEC_CMD_MAP_IOMMU,
	ION_SEC_CMD_UNMAP_IOMMU,
	ION_SEC_CMD_MAP_USER,
	ION_SEC_CMD_UNMAP_USER,
	ION_SEC_CMD_TABLE_SET,
	ION_SEC_CMD_TABLE_CLEAN,
#ifdef CONFIG_SECMEM_TEST
	ION_SEC_CMD_TEST,
#endif
	ION_SEC_CMD_MAX,
};

enum SEC_SVC {
	SEC_TUI = 0,
	SEC_EID = 1,
	SEC_TINY = 2,
	SEC_FACE_ID = 3,
	SEC_FACE_ID_3D = 4,
	SEC_DRM_TEE = 5,
	SEC_SVC_MAX,
};

#define TINY_SYSTEM   0x0        /* tiny version system for chip test*/
#define FULL_SYSTEM   0x1        /* full version system */

struct platform_device *get_hisi_ion_platform_device(void);

#define ION_IOC_HISI_MAGIC 'H'
/**
 *DOC: ION_IOC_FLUSH_ALL_CACHES - flush all the caches pf L1 and L2
 *
 *flush all the caches pf L1 and L2
 */
#define ION_IOC_FLUSH_ALL_CACHES _IOWR(ION_IOC_HISI_MAGIC, 3, \
				struct ion_flush_data)

#ifdef CONFIG_ION
extern unsigned long hisi_ion_total(void);
#else
static inline unsigned long hisi_ion_total(void)
{
	return 0;
}
#endif

int hisi_ion_memory_info(bool verbose);
int hisi_ion_proecss_info(void);

int ion_secmem_get_phys(struct dma_buf *dmabuf, phys_addr_t *addr, size_t *len);
int hisi_ion_cache_operate(int fd, unsigned long uaddr,
	unsigned long offset, unsigned long length,
	unsigned int cmd);

#ifdef CONFIG_ION_HISI_SECSG
struct cma *get_sec_cma(void);
int secmem_get_buffer(int fd, struct sg_table **table, u64 *id,
		      enum SEC_SVC *type);
#else
static inline struct cma *get_sec_cma(void)
{
	return NULL;
}

static inline int secmem_get_buffer(int fd, struct sg_table **table,
				    u64 *id, enum SEC_SVC *type)
{
	return -EINVAL;
}
#endif

#ifdef CONFIG_ION_HISI
struct sg_table *hisi_secmem_alloc(int id, unsigned long size);
void hisi_secmem_free(int id, struct sg_table *table);
#else
static inline struct sg_table *hisi_secmem_alloc(int id, unsigned long size)
{
	return NULL;
}

static inline void hisi_secmem_free(int id, struct sg_table *table) {}
#endif

#endif
