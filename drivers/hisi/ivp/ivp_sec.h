
#ifndef _IVP_SEC_H_
#define _IVP_SEC_H_
#include "ivp_platform.h"

#define SIZE_1M_ALIGN          BIT(20)
#define DEFAULT_MSG_SIZE       (32)
// head + fdnum + sharefd= 3, now 5
#define MAX_FD_NUM	(DEFAULT_MSG_SIZE / sizeof(unsigned int) - 3)

struct ivp_sec_device {
	struct task_struct *secivp_kthread;
	wait_queue_head_t secivp_wait;
	bool secivp_wake;
	unsigned int ivp_addr;
	atomic_t ivp_image_success;
	struct completion load_completion;
};

#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
struct ivp_sec_ion_s {
	unsigned long sec_phymem_addr;
	struct ion_handle *ivp_ion_handle;
	struct ion_client *ivp_ion_client;
};
extern struct ivp_sec_ion_s *ivp_secmem_ion;
int ivp_alloc_secbuff(struct device *ivp_dev, unsigned int size);
int ivp_ion_phys(
	struct device *dev,
	struct ion_client *client,
	struct ion_handle *handle,
	dma_addr_t *addr);
#else
extern unsigned long g_ivp_sec_phymem_addr;
int ivp_get_secbuff(
	struct device *dev,
	int sec_buf_fd,
	unsigned long *sec_buf_phy_addr);
#endif

extern struct ivp_sec_device ivp_sec_dev;
extern struct mutex ivp_ipc_ion_mutex; /* generic mutex for COMEDI core */
void ivp_free_secbuff(void);
int ivp_trans_sharefd_to_phyaddr(struct device *dev, unsigned int *buff);
int ivp_create_secimage_thread(struct ivp_device *ivp_devp);
int ivp_destroy_secimage_thread(struct ivp_device *ivp_devp);
int ivp_sec_load(void);

#endif

