/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2008-2019. All rights reserved.
 * Description: The common data type defination
 * Author: z44949
 * Create: 2008-10-31
 */
#ifndef __HI_DRV_MEM_H__
#define __HI_DRV_MEM_H__

#include <linux/seq_file.h>
#include <linux/vmalloc.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/dma-buf.h>

#include "hi_type.h"
#include "drv_venc_ioctl.h"

#define MAX_MEM_NAME_LEN       (15)
#define MAX_KMALLOC_MEM_NODE   (16)    /* 1 channel need 2 node ,there is have max 8 channels */
#define MAX_ION_MEM_NODE       (200)
#define SMMU_RWERRADDR_SIZE    (128)

typedef struct {
	HI_U64 read_addr;
	HI_U64 write_addr;
} venc_smmu_err_add_t;

typedef struct {
	HI_VOID  *start_virt_addr;
	HI_U64   start_phys_addr;
	HI_U32   size;
	HI_U8    is_mapped;
	HI_S32   share_fd;
} mem_buffer_t;

typedef struct  {
	HI_CHAR            node_name[MAX_MEM_NAME_LEN];
	HI_CHAR            zone_name[MAX_MEM_NAME_LEN];
	HI_VOID           *virt_addr;
	HI_U64             phys_addr;
	HI_U32             size;
	HI_S32             shared_fd;
	struct ion_handle *handle;
	struct dmabuf    *dmabuf;
} venc_mem_buf_t;

HI_S32 drv_mem_copy_from_user(HI_U32 cmd, const void __user *user_arg, HI_VOID **kernel_arg);
HI_S32 drv_mem_init(void);
HI_S32 drv_mem_exit(void);
HI_S32 drv_mem_kalloc(const HI_CHAR *bufname, const HI_CHAR *zone_name, mem_buffer_t *psmbuf);
HI_S32 drv_mem_kfree(const mem_buffer_t *psmbuf);
HI_S32 drv_mem_map_kernel(HI_S32 shared_fd, mem_buffer_t *psmbuf);
HI_S32 drv_mem_unmap_kernel(mem_buffer_t *psmbuf);
HI_S32 drv_mem_get_map_info(HI_S32 shared_fd, mem_buffer_t *psmbuf);
HI_S32 drv_mem_put_map_info(mem_buffer_t *psmbuf);
HI_S32 drv_mem_iommumap(venc_buffer_record_t *node, struct platform_device *pdev);
HI_S32 drv_mem_iommuunmap(HI_S32 shared_fd, HI_S32 phys_addr, struct platform_device *pdev);

#endif /* __HI_DRV_MEM_H__ */

