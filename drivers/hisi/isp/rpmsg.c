/*
 * Histarisp rpmsg client driver
 *
 * Copyright (c) 2013- Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/*lint -e666  -e529 -e438 -e713 -e715 -e559 -e626 -e719 -e846 -e514 -e778 -e866 -e84 -e437 -esym(666,*) -esym(529,*) -esym(438,*) -esym(713,*) -esym(715,*) -esym(559,*) -esym(626,*) -esym(719,*) -esym(846,*) -esym(514,*) -esym(778,*) -esym(866,*) -esym(84,*) -esym(437,*)*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/cdev.h>
#include <linux/jiffies.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/skbuff.h>
#include <linux/sched.h>
#include <linux/rpmsg.h>
#include <linux/completion.h>
#include <uapi/linux/histarisp.h>
#include <linux/platform_data/remoteproc-hisi.h>
#include <linux/ion.h>
#include <linux/dma-buf.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/version.h>

static int debug_mask = 0x3;
#define rpmsg_err(fmt, args...) \
    do {                         \
        if (debug_mask & 0x01)   \
            printk(KERN_INFO "Rpmsg HISI Err: [%s] " fmt, __func__, ##args); \
    } while (0)
#define rpmsg_info(fmt, args...)  \
    do {                         \
        if (debug_mask & 0x02)   \
            printk(KERN_INFO "Rpmsg HISI Info: [%s] " fmt, __func__, ##args);  \
    } while (0)
#define rpmsg_dbg(fmt, args...)  \
    do {                         \
        if (debug_mask & 0x04) \
            printk(KERN_INFO "Rpmsg HISI Debug: [%s] " fmt, __func__, ##args); \
    } while (0)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
/**
 * struct rpmsg_hdr - common header for all rpmsg messages
 * @src: source address
 * @dst: destination address
 * @reserved: reserved for future use
 * @len: length of payload (in bytes)
 * @flags: message flags
 * @data: @len bytes of message payload data
 *
 * Every message sent(/received) on the rpmsg bus begins with this header.
 */
struct rpmsg_hdr {
	u32 src;
	u32 dst;
	u32 reserved;
	u16 len;
	u16 flags;
	u8 data[0];
} __packed;
#endif

struct rpmsg_hisi_service {
    struct cdev *cdev;
    struct device *dev;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
    struct rpmsg_device *rpdev;
#else
    struct rpmsg_channel *rpdev;
#endif
    struct list_head list;
    struct mutex lock;
};
struct hisp_rpmsgrefs_s{
    atomic_t sendin_refs;
    atomic_t sendx_refs;
    unsigned long long sendx_last;
    atomic_t recvin_refs;
    atomic_t recvx_refs;
    atomic_t recvtask_refs;
    unsigned long long recvx_last;
}hisp_rpmsgrefs;
static struct rpmsg_hisi_service hisi_isp_serv;
static atomic_t instances;
struct completion channel_sync;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
struct rpmsg_channel_info chinfo = {
    .src = RPMSG_ADDR_ANY,
};
#endif

int rpmsg_client_debug = INVALID_CLIENT;
static unsigned long long get_hisitime(void)
{
    return hisi_getcurtime();
}
void hisp_sendin(void)
{
    struct hisp_rpmsgrefs_s *dev = (struct hisp_rpmsgrefs_s *)&hisp_rpmsgrefs;
    atomic_inc(&dev->sendin_refs);
}

void hisp_sendx(void)
{
    struct hisp_rpmsgrefs_s *dev = (struct hisp_rpmsgrefs_s *)&hisp_rpmsgrefs;
    atomic_inc(&dev->sendx_refs);
    dev->sendx_last = get_hisitime();
}

void hisp_recvin(void)
{
    struct hisp_rpmsgrefs_s *dev = (struct hisp_rpmsgrefs_s *)&hisp_rpmsgrefs;
    atomic_inc(&dev->recvin_refs);
}
void hisp_recvx(void)
{
    struct hisp_rpmsgrefs_s *dev = (struct hisp_rpmsgrefs_s *)&hisp_rpmsgrefs;
    atomic_inc(&dev->recvx_refs);
    dev->recvx_last = get_hisitime();
}

void hisp_recvtask(void)
{
    struct hisp_rpmsgrefs_s *dev = (struct hisp_rpmsgrefs_s *)&hisp_rpmsgrefs;
    atomic_inc(&dev->recvtask_refs);
}

void hisp_rpmsgrefs_dump(void)
{
    struct hisp_rpmsgrefs_s *dev = (struct hisp_rpmsgrefs_s *)&hisp_rpmsgrefs;
    char time_log[80] = "";
    char *ptime_log;
    ptime_log = time_log;
    print_time(dev->sendx_last, ptime_log);
    pr_info("sendin_refs.0x%x, sendx_refs.0x%x, sendx_timer.%s\n", atomic_read(&dev->sendin_refs),atomic_read(&dev->sendx_refs),ptime_log);
    print_time(dev->recvx_last, ptime_log);
    pr_info("recvin_refs.0x%x, recvx_refs.0x%x, recvtask_refs.0x%x recvx_timer.%s\n", atomic_read(&dev->recvin_refs),atomic_read(&dev->recvx_refs),atomic_read(&dev->recvtask_refs),ptime_log);
}

void hisp_rpmsgrefs_reset(void)
{
    struct hisp_rpmsgrefs_s *dev = (struct hisp_rpmsgrefs_s *)&hisp_rpmsgrefs;
    hisp_rpmsgrefs_dump();
    atomic_set(&dev->sendin_refs, 0);
    atomic_set(&dev->sendx_refs, 0);
    dev->sendx_last = 0;
    atomic_set(&dev->recvin_refs, 0);
    atomic_set(&dev->recvx_refs, 0);
    atomic_set(&dev->recvtask_refs, 0);
    dev->recvx_last = 0;
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
static int rpmsg_hisi_probe(struct rpmsg_device *rpdev)
#else
static int rpmsg_hisi_probe(struct rpmsg_channel *rpdev)
#endif
{
    struct rpmsg_hisi_service *hisi_serv = &hisi_isp_serv;

    INIT_LIST_HEAD(&hisi_serv->list);
    mutex_init(&hisi_serv->lock);

    hisi_serv->rpdev = rpdev;

    rpmsg_info("new HISI connection srv channel: %u -> %u!\n",
                        rpdev->src, rpdev->dst);

    rpmsg_dbg("Exit ...\n");
    return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
static void rpmsg_hisi_remove(struct rpmsg_device *rpdev)
#else
static void rpmsg_hisi_remove(struct rpmsg_channel *rpdev)
#endif
{
    struct rpmsg_hisi_service *hisi_serv = &hisi_isp_serv;

    /* check list */

    mutex_lock(&hisi_serv->lock);
    if (list_empty(&hisi_serv->list)) {
        mutex_unlock(&hisi_serv->lock);

        rpmsg_info("hisi_serv->list is empty, instances = %d.\n", atomic_read(&instances));
    } else {
        mutex_unlock(&hisi_serv->lock);
        /* Maybe here need to debug, in case more exception case */
        rpmsg_err("rpmsg remove exception, instances = %d \n", atomic_read(&instances));
        WARN_ON(1);/*lint !e548 */
    }

    rpmsg_info("Exit ...\n");
    return;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
static int rpmsg_hisi_driver_cb(struct rpmsg_device *rpdev, void *data, int len, void *priv, u32 src)
#else
static void rpmsg_hisi_driver_cb(struct rpmsg_channel *rpdev, void *data, int len, void *priv, u32 src)
#endif
{
    rpmsg_dbg("Enter ...\n");
    dev_warn(&rpdev->dev, "uhm, unexpected message\n");

    print_hex_dump(KERN_DEBUG, __func__, DUMP_PREFIX_NONE, 16, 1,
               data, len,  true);
    rpmsg_dbg("Exit ...\n");
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
    return 0;
#endif
}

static struct rpmsg_device_id rpmsg_hisi_id_table[] = {
    { .name    = "rpmsg-isp-debug" },
    { },
};
MODULE_DEVICE_TABLE(platform, rpmsg_hisi_id_table);
/*lint -save -e485*/
static struct rpmsg_driver rpmsg_hisi_driver = {
    .drv.name   = KBUILD_MODNAME,
    .drv.owner  = THIS_MODULE,
    .id_table   = rpmsg_hisi_id_table,
    .probe      = rpmsg_hisi_probe,
    .callback   = rpmsg_hisi_driver_cb,
    .remove     = rpmsg_hisi_remove,
};
/*lint -restore */
static int __init rpmsg_hisi_init(void)
{
    return register_rpmsg_driver(&rpmsg_hisi_driver);
}
module_init(rpmsg_hisi_init);

static void __exit rpmsg_hisi_exit(void)
{
    unregister_rpmsg_driver(&rpmsg_hisi_driver);
}
module_exit(rpmsg_hisi_exit);

MODULE_DESCRIPTION("HISI offloading rpmsg driver");
MODULE_LICENSE("GPL v2");
