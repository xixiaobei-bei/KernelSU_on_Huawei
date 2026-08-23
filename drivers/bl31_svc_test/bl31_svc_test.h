#ifndef __BL31_SVC_TEST_H__
#define __BL31_SVC_TEST_H__

#define PRINT_FUNINFO printk("%s,%d\n",__FUNCTION__,__LINE__)

/*RPMB shared mem address and size*/
#define RPMB_SHARED_MEM_OFFSET                         (0x8000)
#define RPMB_SHARED_MEM_SIZE                           (0x8000)
#define RPMB_SHARED_MEM_DADDR (HISI_SUB_RESERVED_BL31_SHARE_MEM_PHYMEM_BASE \
    + RPMB_SHARED_MEM_OFFSET)

struct bl31_svc_args
{
    u64 x1;
    u64 x2;
    u64 x3;
};

struct bl31_fun_id{
    u32  magic;
    char name[50];
    u32  id;
};

enum bootdevice_type {
    BOOT_DEVICE_EMMC = 0,
    BOOT_DEVICE_UFS = 1
};

#endif /* __BL31_SVC_TEST_H__ */