#ifndef _IVP_SEC_H_
#define _IVP_SEC_H_

#define SIZE_1M_ALIGN          (1ul<<20)
struct ivp_sec_device {
    struct task_struct *secivp_kthread;
    wait_queue_head_t secivp_wait;
    bool secivp_wake;
    unsigned int ivp_addr;
    atomic_t ivp_image_success;
    struct completion load_completion;
};

struct ivp_sec_ion_s {
    unsigned long sec_phymem_addr;
    struct ion_handle *ivp_ion_handle;
    struct ion_client *ivp_ion_client;
};

extern int ivp_alloc_secbuff(unsigned int size);
extern void ivp_free_secbuff(void);
extern int ivp_create_secimage_thread(struct ivp_device *ivp_devp);
extern int ivp_destroy_secimage_thread(struct ivp_device *ivp_devp);
extern int ivp_sec_load(void);

#endif

