
#ifndef _IVP_SEC_H_
#define _IVP_SEC_H_
#include "ivp_platform.h"

#define SIZE_1M_ALIGN          (1ul<<20)
#define DEFAULT_MSG_SIZE       (32)
#define MAX_FD_NUM             (DEFAULT_MSG_SIZE/sizeof(unsigned int) - 3)   //head + fdnum + sharefd= 3, now 5
struct ivp_sec_device {
    struct task_struct *secivp_kthread;
    wait_queue_head_t secivp_wait;
    bool secivp_wake;
    unsigned int ivp_addr;
    atomic_t ivp_image_success;
    struct completion load_completion;
};

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
struct ivp_sec_ion_s {
    unsigned long sec_phymem_addr;
    struct ion_handle *ivp_ion_handle;
    struct ion_client *ivp_ion_client;
};
extern int ivp_alloc_secbuff(struct device *ivp_dev, unsigned int size);
extern int ivp_ion_phys(struct device *dev, struct ion_client *client, struct ion_handle *handle,dma_addr_t *addr);
#else
extern unsigned long g_ivp_sec_phymem_addr;
extern int ivp_get_secbuff(int sec_buf_fd, unsigned long *sec_buf_phy_addr);
#endif

extern struct mutex ivp_ipc_ion_mutex;
extern void ivp_free_secbuff(void);
extern int ivp_trans_sharefd_to_phyaddr(struct device *dev, unsigned int* buff);
extern int ivp_create_secimage_thread(struct ivp_device *ivp_devp);
extern int ivp_destroy_secimage_thread(struct ivp_device *ivp_devp);
extern int ivp_sec_load(void);

#endif

