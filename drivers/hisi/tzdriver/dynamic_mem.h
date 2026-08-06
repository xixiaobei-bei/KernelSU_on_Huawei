

#ifndef _DYNAMIC_MMEM_H_
#define _DYNAMIC_MMEM_H_
#include <linux/version.h>
#include <linux/hisi/hisi_ion.h>
#include "teek_ns_client.h"
struct sg_memory {
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
	int dyn_shared_fd;
	struct dma_buf *dyn_dma_buf;
	phys_addr_t ion_phys_addr;
#else
	struct ion_handle *ion_handle;
	ion_phys_addr_t ion_phys_addr;
#endif
	size_t len;
	void *ion_virt_addr;
};
struct dynamic_mem_item{
	struct list_head head;
	uint32_t configid;
	uint32_t size;
	struct sg_memory memory;
	uint32_t cafd;
	teec_uuid uuid;
};
int init_dynamic_mem(void);
void exit_dynamic_mem(void);
int load_app_use_configid(uint32_t configid, uint32_t cafd,
	teec_uuid* uuid, uint32_t size);
void kill_ion_by_cafd(unsigned int cafd);
void kill_ion_by_uuid(teec_uuid *uuid);
int is_used_dynamic_mem(teec_uuid *uuid);

#endif
