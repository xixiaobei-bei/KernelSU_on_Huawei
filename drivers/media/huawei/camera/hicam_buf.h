/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: camera buf
 * Author: wenjianyue
 * Create: 2018-11-22
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 */

#ifndef __HICAM_BUF_H_INCLUDED__
#define __HICAM_BUF_H_INCLUDED__
#include <linux/types.h>

struct device;
struct sg_table;

#define HICAM_BUF_FILE_MAX_LEN 20
#define HICAM_BUF_SYNC_READ (1u << 0)
#define HICAM_BUF_SYNC_WRITE (1u << 1)

/* need align to AP CameraBufDevType */
enum hicam_buf_dev_type {
	HICAM_DEV_ISP,
	HICAM_DEV_ARCH,
	HICAM_DEV_JENC,
	HICAM_DEV_ORB,
	HICAM_DEV_MC,
	HICAM_DEV_CMP,
	HICAM_DEV_ANF,
	HICAM_DEV_ORB_ENH,
	HICAM_DEV_JDEC,
	HICAM_DEV_CMDLIST,
	HICAM_DEV_RDR,
	HICAM_DEV_GF,
	HICAM_DEV_VBK,
	HICAM_DEV_IVP,
	HICAM_DEV_IVP1,
	HICAM_DEV_MAX
};

enum hicam_buf_cfg_type {
	HICAM_BUF_MAP_IOMMU,
	HICAM_BUF_UNMAP_IOMMU,
	HICAM_BUF_SYNC,
	HICAM_BUF_LOCAL_SYNC,
	HICAM_BUF_GET_PHYS,
	HICAM_BUF_OPEN_SECURITY_TA,
	HICAM_BUF_CLOSE_SECURITY_TA,
	HICAM_BUF_SET_SECBOOT,
	HICAM_BUF_RELEASE_SECBOOT,
	HICAM_BUF_GET_SECMEMTYPE,
	HICAM_BUF_SC_AVAILABLE_SIZE,
	HICAM_BUF_SC_WAKEUP,
};
/* kernel coding style prefers __xx as types shared with userspace */
struct iommu_format {
	__u32 prot;
	__u64 iova;
	__u64 size;
};

struct phys_format {
	__u64 phys;
};

struct sync_format {
	/* sync direction read/write from CPU view point */
	__u32 direction;
};

struct local_sync_format {
	/* sync direction read/write from CPU view point */
	__u32 direction;
	/* local sync needs to set apva/offset/length */
	__u64 apva;
	__u64 offset;
	__u64 length;
};

struct sec_mem_format {
	/* sec mem buffer type */
	__u32 type;
	__u32 prot;
	__u64 iova;
	__u64 size;
};

struct systemcache_format {
	/* systemcache info */
	__u32 pid;
	__u32 prio;
	__u32 size;
};

struct hicam_buf_cfg {
	__s32 fd;
	__u32 secmemtype;
	enum hicam_buf_cfg_type type;
	union {
		struct iommu_format iommu_format;
		struct phys_format phys_format;
		struct sync_format sync_format;
		struct local_sync_format local_sync_format;
		struct sec_mem_format sec_mem_format;
		struct systemcache_format systemcache_format;
	};
};

#define HICAM_BUF_BASE 'H'
#define HICAM_BUF_IOC_CFG _IOWR(HICAM_BUF_BASE, 0, struct hicam_buf_cfg)

int hicam_buf_map_iommu(int fd, struct iommu_format *fmt);
void hicam_buf_unmap_iommu(int fd, struct iommu_format *fmt);
void hicam_buf_sc_available_size(struct systemcache_format *fmt);
int hicam_buf_sc_wakeup(struct systemcache_format *fmt);
int hicam_buf_get_phys(int fd, struct phys_format *fmt);
phys_addr_t hicam_buf_get_pgd_base(void);

/*
 * keep in mind, get sgtable will hold things of sg_table,
 * please release after use.
 */
struct sg_table *hicam_buf_get_sgtable(int fd);
void hicam_buf_put_sgtable(struct sg_table *sgt);

int hicam_buf_v3_map_iommu(struct device *dev, int fd, struct iommu_format *fmt,
	int padding_support);
void hicam_buf_v3_unmap_iommu(struct device *dev, int fd,
	struct iommu_format *fmt, int padding_support);
#endif /* ifndef __HICAM_BUF_H_INCLUDED__ */