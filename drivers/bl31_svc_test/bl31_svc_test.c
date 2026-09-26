/*
 * bl31_svc_test.c for the bl31 driver.
 *
 * Copyright (c) 2018 Hisilicon Technologies CO.Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <asm/uaccess.h>
#include <asm/device.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <global_ddr_map.h>
#include "bl31_svc_test.h"

#define BL31_IOC_MAGIC 'x'
#define BL31_IOC_MAX_NR 99 //定义命令的最大序列

struct bl31_fun_id fun_ids[BL31_IOC_MAX_NR]={
    {_IO(BL31_IOC_MAGIC, 1 ),  "RPMB_SVC_CALL_COUNT",                            0xc6000000},
    {_IO(BL31_IOC_MAGIC, 2 ),  "RPMB_SVC_UID",                                   0xc6000001},
    {_IO(BL31_IOC_MAGIC, 3 ),  "RPMB_SVC_VERSION",                               0xc6000002},
    {_IO(BL31_IOC_MAGIC, 4 ),  "RPMB_SVC_TEST",                                  0xc6000003},
    {_IO(BL31_IOC_MAGIC, 5 ),  "RPMB_SVC_REQUEST_ADDR",                          0xc600FF04},
    {_IO(BL31_IOC_MAGIC, 6 ),  "RPMB_SVC_SET_KEY",                               0xc600FF05},
    {_IO(BL31_IOC_MAGIC, 7 ),  "RPMB_SVC_REQUEST_DONE",                          0xc600FF06},
    {_IO(BL31_IOC_MAGIC, 8 ),  "RPMB_SVC_SECURE_OS_INFO",                        0x8600FF10},
    {_IO(BL31_IOC_MAGIC, 9 ),  "RPMB_SVC_READ",                                  0xc600FFF1},
    {_IO(BL31_IOC_MAGIC, 10),  "RPMB_SVC_WRITE",                                 0xc600FFF2},
    {_IO(BL31_IOC_MAGIC, 11),  "RPMB_SVC_COUNTER",                               0xc600FFF3},
    {_IO(BL31_IOC_MAGIC, 12),  "RPMB_SVC_FORMAT",                                0xc600FFF4},
    {_IO(BL31_IOC_MAGIC, 13),  "UFS_SVC_REG_SET",                                0xc600FFF5},
    {_IO(BL31_IOC_MAGIC, 14),  "RPMB_SVC_WRITE_CAPABILITY",                      0xc600FFF6},
    {_IO(BL31_IOC_MAGIC, 15),  "RPMB_SVC_READ_CAPABILITY",                       0xc600FFF7},
    {_IO(BL31_IOC_MAGIC, 16),  "RPMB_SVC_PARTITION",                             0xc600FFF8},
    {_IO(BL31_IOC_MAGIC, 17),  "RPMB_SVC_MULTI_KEY",                             0xc600FFF9},
    {_IO(BL31_IOC_MAGIC, 18),  "RPMB_SVC_CONFIG_VIEW",                           0xc600FFFA},
    {_IO(BL31_IOC_MAGIC, 19),  "RPMB_SVC_MULTI_KEY_STATUS",                      0xc600FFFB},
    {_IO(BL31_IOC_MAGIC, 20),  "RPMB_SVC_GET_DEV_VER",                           0xc600FFFC},
    {_IO(BL31_IOC_MAGIC, 21),  "ISP_FN_SET_PARAMS",                              0xC500AB00},
    {_IO(BL31_IOC_MAGIC, 22),  "ISP_FN_PARAMS_DUMP",                             0xC500AB01},
    {_IO(BL31_IOC_MAGIC, 23),  "ISP_FN_ISPCPU_NS_DUMP",                          0xC500AB02},
    {_IO(BL31_IOC_MAGIC, 24),  "ISP_FN_ISP_INIT",                                0xC500AB11},
    {_IO(BL31_IOC_MAGIC, 25),  "ISP_FN_ISP_EXIT",                                0xC500AB12},
    {_IO(BL31_IOC_MAGIC, 26),  "ISP_FN_ISPCPU_INIT",                             0xC500AB22},
    {_IO(BL31_IOC_MAGIC, 27),  "ISP_FN_ISPCPU_EXIT",                             0xC500AB23},
    {_IO(BL31_IOC_MAGIC, 28),  "ISP_FN_ISPCPU_MAP",                              0xC500AB24},
    {_IO(BL31_IOC_MAGIC, 29),  "ISP_FN_ISPCPU_UNMAP",                            0xC500AB25},
    {_IO(BL31_IOC_MAGIC, 30),  "ISP_FN_SET_NONSEC",                              0xC500AB26},
    {_IO(BL31_IOC_MAGIC, 31),  "ISP_FN_DISRESET_ISPCPU",                         0xC500AB27},
    {_IO(BL31_IOC_MAGIC, 32),  "ISP_FN_ISPSMMU_NS_INIT",                         0xC500AB28},
    {_IO(BL31_IOC_MAGIC, 33),  "ISP_FN_RESET_ISPCPU",                            0xC500AB29},
    {_IO(BL31_IOC_MAGIC, 34),  "ISP_FN_ISPTOP_PU",                               0xC500AB30},
    {_IO(BL31_IOC_MAGIC, 35),  "ISP_FN_ISPTOP_PD",                               0xC500AB31},
    {_IO(BL31_IOC_MAGIC, 36),  "HISPFD_SMMUENABLE",                              0xC500AB3E},
    {_IO(BL31_IOC_MAGIC, 37),  "EFUSE_FN_VERSION",                               0xc5000000},
    {_IO(BL31_IOC_MAGIC, 38),  "EFUSE_FN_GET_DIEID",                             0xc5000001},
    {_IO(BL31_IOC_MAGIC, 39),  "EFUSE_FN_GET_CHIPID",                            0xc5000002},
    {_IO(BL31_IOC_MAGIC, 40),  "EFUSE_FN_SET_CHIPID",                            0xc5000003},
    {_IO(BL31_IOC_MAGIC, 41),  "EFUSE_FN_GET_AUTHKEY",                           0xc5000004},
    {_IO(BL31_IOC_MAGIC, 42),  "EFUSE_FN_SET_AUTHKEY",                           0xc5000005},
    {_IO(BL31_IOC_MAGIC, 43),  "EFUSE_FN_GET_SECURITYDEBUG",                     0xc5000006},
    {_IO(BL31_IOC_MAGIC, 44),  "EFUSE_FN_SET_SECURITYDEBUG",                     0xc5000007},
    {_IO(BL31_IOC_MAGIC, 45),  "EFUSE_FN_GET_THERMAL",                           0xc5000008},
    {_IO(BL31_IOC_MAGIC, 46),  "EFUSE_FN_TEST_DISPLAY",                          0xc5000009},
    {_IO(BL31_IOC_MAGIC, 47),  "EFUSE_FN_TEST_WR",                               0xc500000A},
    {_IO(BL31_IOC_MAGIC, 48),  "EFUSE_FN_GET_KCE",                               0xc500000B},
    {_IO(BL31_IOC_MAGIC, 49),  "EFUSE_FN_SET_KCE",                               0xc500000C},
    {_IO(BL31_IOC_MAGIC, 50),  "EFUSE_FN_GET_CARRIERID",                         0xc500000D},
    {_IO(BL31_IOC_MAGIC, 51),  "EFUSE_FN_GET_FREQ",                              0xc500000E},
    {_IO(BL31_IOC_MAGIC, 52),  "EFUSE_FN_GET_SLTFINISHFLAG",                     0xc500000F},
    {_IO(BL31_IOC_MAGIC, 53),  "EFUSE_FN_SET_SLTFINISHFLAG",                     0xc5000010},
    {_IO(BL31_IOC_MAGIC, 54),  "EFUSE_FN_SET_DJTAGDEBUG",                        0xc5000011},
    {_IO(BL31_IOC_MAGIC, 55),  "EFUSE_FN_GET_DJTAGDEBUG",                        0xc5000012},
    {_IO(BL31_IOC_MAGIC, 56),  "EFUSE_FN_SET_HISEE",                             0xc5000013},
    {_IO(BL31_IOC_MAGIC, 57),  "EFUSE_FN_GET_HISEE",                             0xc5000014},
    {_IO(BL31_IOC_MAGIC, 58),  "ACCESS_REGISTER_FN_MAIN_ID",                     0xc500aa01},
    {_IO(BL31_IOC_MAGIC, 59),  "IVP_SLV_SECMODE",                                0xC500bb00},
    {_IO(BL31_IOC_MAGIC, 60),  "IVP_MST_SECMODE",                                0xC500bb01},
    {_IO(BL31_IOC_MAGIC, 61),  "IVP_REMAP_SECMODE",                              0xC500bb02},
    {_IO(BL31_IOC_MAGIC, 62),  "FREQDUMP_SVC_CALL_COUNT",                        0xC7000000},
    {_IO(BL31_IOC_MAGIC, 63),  "FREQDUMP_SVC_UID",                               0xC7000001},
    {_IO(BL31_IOC_MAGIC, 64),  "FREQDUMP_SVC_VERSION",                           0xC7000002},
    {_IO(BL31_IOC_MAGIC, 65),  "FREQDUMP_SVC_REG_RD",                            0xC700EE00},
    {_IO(BL31_IOC_MAGIC, 66),  "FREQDUMP_LOADMONITOR_SVC_REG_RD",                0xC700EE01},
    {_IO(BL31_IOC_MAGIC, 67),  "FREQDUMP_LOADMONITOR_SVC_ENABLE",                0xC700EE02},
    {_IO(BL31_IOC_MAGIC, 68),  "FREQDUMP_LOADMONITOR_SVC_DISABLE",               0xC700EE03},
    {_IO(BL31_IOC_MAGIC, 69),  "FREQDUMP_ADC_SVC_SET_PARAM",                     0xC700EE04},
    {_IO(BL31_IOC_MAGIC, 70),  "FREQDUMP_ADC_SVC_DISABLE",                       0xC700EE05},
    {_IO(BL31_IOC_MAGIC, 71),  "FREQDUMP_LOADMONITOR_SVC_ENABLE_READ",           0xC700EE06},
    {_IO(BL31_IOC_MAGIC, 72),  "MEDIA_MONITOR_SVC_ENABLE",                       0xC700EE07},
    {_IO(BL31_IOC_MAGIC, 73),  "MEDIA_MONITOR_SVC_DISABLE",                      0xC700EE08},
    {_IO(BL31_IOC_MAGIC, 74),  "MEDIA_MONITOR_SVC_SET_FLAG",                     0xC700EE09},
    {_IO(BL31_IOC_MAGIC, 75),  "HISI_SERVICE_STD_SVC_CALL_COUNT",                0xc500ff00},
    {_IO(BL31_IOC_MAGIC, 76),  "HISI_SERVICE_STD_SVC_UID",                       0xc500ff01},
    {_IO(BL31_IOC_MAGIC, 77),  "HISI_SERVICE_STD_SVC_VERSION",                   0xc500ff03},
    {_IO(BL31_IOC_MAGIC, 78),  "HISI_GET_VAL_STD_SVC_VER",                       0xc8000000},
    {_IO(BL31_IOC_MAGIC, 79),  "HISI_GET_VAL_SVC_CALL_COUNT",                    0xc8000001},
    {_IO(BL31_IOC_MAGIC, 80),  "HISI_GET_VAL_STD_SVC_UID",                       0xc8000002},
    {_IO(BL31_IOC_MAGIC, 81),  "ARM_STD_SVC_CALL_COUNT",                         0x8400ff00},
    {_IO(BL31_IOC_MAGIC, 82),  "ARM_STD_SVC_UID",                                0x8400ff01},
    {_IO(BL31_IOC_MAGIC, 83),  "ARM_STD_SVC_VERSION",                            0x8400ff03},
    {_IO(BL31_IOC_MAGIC, 84),  "IPC_SVC_SECURE_OS_FIQ",                          0x85000060},
    {_IO(BL31_IOC_MAGIC, 85),  "CPU_VOLT_FN_GET_VAL",                            0xc800aa01},
    {_IO(BL31_IOC_MAGIC, 86),  "BL31_DEBUG_FN_VAL",                              0xc800aa02},
    /*86个确定functionID,13个泛ID，以下用mask_value进行测试*/
    {_IO(BL31_IOC_MAGIC, 87),  "BL31_IP_REGULATOR_FID",                          0xc500fff0},
    {_IO(BL31_IOC_MAGIC, 88),  "BL31_L3SHARE_FID",                               0xc500f008},
    {_IO(BL31_IOC_MAGIC, 89),  "BL31_DSU_PCTRL_FID",                             0xc500f080},
    {_IO(BL31_IOC_MAGIC, 90),  "BL31_L3DEBUG_FID",                               0xc500f001},
    {_IO(BL31_IOC_MAGIC, 91),  "BL31_L3DEBUG_PMU_FID",                           0xc500f002},
    {_IO(BL31_IOC_MAGIC, 92),  "BL31_L3SHARE_MNTN_FID",                          0xc500f004},
    {_IO(BL31_IOC_MAGIC, 93),  "BL31_HISEE_KERNEL_FID",                          0xc5000020},
    {_IO(BL31_IOC_MAGIC, 94),  "BL31_HISEE_TEST_FID",                            0xc5000040},
    {_IO(BL31_IOC_MAGIC, 95),  "BL31_HISEE_MNTN_FID",                            0xc500cc00},
    {_IO(BL31_IOC_MAGIC, 96),  "BL31_RTC_FID",                                   0xc500ddd0},
    {_IO(BL31_IOC_MAGIC, 97),  "BL31_PINCTRL_FID",                               0xc500ccc0},
    {_IO(BL31_IOC_MAGIC, 98),  "BL31_HISI_SPMI_FID",                             0xc500eee0},
    {_IO(BL31_IOC_MAGIC, 99),  "BL31_THERMAL_FID",                               0xc5009900}
};

struct cdev cdev;
struct class *cdev_class;
dev_t devno;

/* smc function call */
noinline int atfd_hisi_smc(u64 function_id, u64 arg0, u64 arg1,  u64 arg2)
{
    asm volatile(
            __asmeq("%0", "x0")
            __asmeq("%1", "x1")
            __asmeq("%2", "x2")
            __asmeq("%3", "x3")
            "smc    #0\n"
        : "+r" (function_id)
        : "r" (arg0), "r" (arg1), "r" (arg2));

    return (int)function_id;
}


/* ioctl use for bl31 service fuzz test */
static long bl31_svc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct bl31_svc_args data;
    int ret;
    if(_IOC_TYPE(cmd) != BL31_IOC_MAGIC)
    {
        printk("bad magic:%d\n", _IOC_TYPE(cmd));
        return -EINVAL;
    }
    int nr=(int)_IOC_NR(cmd);
    if(nr>BL31_IOC_MAX_NR || nr<=0)
    {
        printk("bad nr:%d\n", nr);
        return -EINVAL;
    }
    if(copy_from_user(&data, (void*)arg, sizeof(struct bl31_svc_args))){
        printk("Failed to copy data from user\n");
        return -EFAULT;
    }

    struct bl31_fun_id fid=fun_ids[nr-1];
    if(cmd!=fid.magic){
        printk("bl31_svc Module unknown ioctl cmd, cmd is:%u, 0x%x\n", cmd, cmd);
        return 0;
    }

    /*if(cmd==BL31_IOC_RPMB_SVC_REQUEST_ADDR){
        if(data.x1 != (u64)RPMB_SHARED_MEM_DADDR || data.x2 > BOOT_DEVICE_UFS){
            return -EFAULT;
        }
    }*/
    printk("cmd is:%u, 0x%x, cmd name is %s, arg is:%llu, %llu, %llu\n", cmd, cmd, fid.name, data.x1, data.x2, data.x3);
    ret=atfd_hisi_smc((u64)fid.id, data.x1, data.x2, data.x3);

    if(ret){
        printk("smc call failure, cmd is:%u, cmd name is %s, arg is:%llu, %llu, %llu, ret is: %d\n", cmd, fid.name, data.x1, data.x2, data.x3, ret);
        return -EINVAL;
    }
    return 0;
}

static const struct file_operations mem_fops =
{
    .llseek = NULL,
    .read = NULL,
    .write = NULL,
    .open = NULL,
    .release = NULL,
    .unlocked_ioctl = bl31_svc_ioctl,
    .mmap = NULL,
};

static int bl31_svc_test_init(void)
{
    int ret;
    PRINT_FUNINFO;

    cdev_init(&cdev, &mem_fops);

    /* register char device */
    ret=alloc_chrdev_region(&devno, 0, 2, "bl31_svc_test");
    if(ret){
        return -1;
    }

    ret=cdev_add(&cdev, devno, 2);
    if(ret)
    {
        printk("cdev add fail.\n");
        goto error_unregister;
    }

    /* create module class, use for creating device node */
    cdev_class = class_create(THIS_MODULE,"bl31_svc_test");
    if(IS_ERR(cdev_class))
    {
        printk("Create class fail!\n");
        goto error_unregister;
    }

    /* create device node */
    if(IS_ERR(device_create(cdev_class,NULL,devno,0,"bl31_svc_test%d",0))){
        printk("Create device fail!\n");
        goto error_unregister;
    }
    printk("bl31_svc_test_init success!\n");
    return 0;

error_unregister:
    unregister_chrdev_region(devno,2);
    return -1;
}

static void bl31_svc_test_exit(void)
{
    PRINT_FUNINFO;
    device_destroy(cdev_class,devno);
    class_destroy(cdev_class);

    /*unregister device*/
    cdev_del(&cdev);
    /*release dice number*/
    unregister_chrdev_region(devno, 2);

    printk("bl31_svc_test_exit sucess!\n");
}

MODULE_LICENSE("GPL");
module_init(bl31_svc_test_init);
module_exit(bl31_svc_test_exit);

