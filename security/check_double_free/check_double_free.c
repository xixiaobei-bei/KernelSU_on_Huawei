/*
 * check_double_free.c
 *
 * the check_double_free.c is used to check whether double free happend
 *
 * chenli <chenli45@huawei.com>
 *
 * Copyright (c) 2017-2021, Huawei Tech. Co., Ltd. All rights reserved.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/security.h>
#include <linux/lsm_hooks.h>
#include <linux/mman.h>
#include <linux/tpm.h>
#include <crypto/hash.h>
#include <crypto/hash_info.h>
#include <linux/slub_def.h>
#include <linux/delay.h>
#include <linux/mm_types.h>
#include <linux/mm.h>
#include <linux/list.h>

#include <chipset_common/security/hw_kernel_stp_interface.h>

#ifdef CONFIG_HW_SLUB_SANITIZE
int stp_double_free_check_trigger(void)
{
    struct stp_item item;
    (void)memset(&item, 0, sizeof(item));
    item.id = item_info[DOUBLE_FREE].id;
    item.status = STP_RISK;
    item.credible = STP_CREDIBLE;
    item.version = 0;
    (void)strncpy(item.name, item_info[DOUBLE_FREE].name, STP_ITEM_NAME_LEN - 1);
    int ret = 0;
    /* create kmem_cache allocate object which size is 128bytes */
    struct kmem_cache *my_cachep = kmem_cache_create("my_cache", 128, 0, SLAB_HWCACHE_ALIGN, NULL);
    struct kmem_cache *cur = my_cachep;
    if (cur->flags & SLAB_CLEAR)
    {
        ret = kernel_stp_upload(item, cur->name);
        if (ret != 0)
            printk("stp double free check upload failed");
        else
            printk("stp double free check upload suc");
    }
    cur = list_next_entry(cur, list);
    list_for_each_entry_from(cur, &my_cachep->list, list)
    {
	if (NULL == cur->name)
	{
	    continue;
	}
	if (cur->flags & SLAB_CLEAR)
        {
            ret = kernel_stp_upload(item, cur->name);
            if (ret != 0)
                printk("stp double free check upload failed");
            else
                printk("stp double free check upload suc");
        }
    }
    return 0;
}
#endif

static int __init double_free_check_module_init(void)
{
#ifdef CONFIG_HW_SLUB_SANITIZE
    (void)kernel_stp_scanner_register(stp_double_free_check_trigger);
#endif
    printk("check double free module init success.");
    return 0;
}

/* double free check module is statically compiled into the kernel, module_exit() has no effect. */
#if 0
static void __exit double_free_check_module_exit(void)
{
	return;
}
#endif

late_initcall(double_free_check_module_init);

/* double free check is statically compiled into the kernel, module_exit() has no effect. */
#if 0
module_exit(double_free_check_module_exit);
#endif

MODULE_DESCRIPTION("EMUI double free check module");
MODULE_LICENSE("GPL v2");
