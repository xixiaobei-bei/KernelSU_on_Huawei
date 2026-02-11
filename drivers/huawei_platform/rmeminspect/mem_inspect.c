/* mem_inspect.c
 *
 * function required for phys_mem_devs module init
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "phys_mem_int.h"

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/printk.h>
#include <securec.h>

#define DRIVER_AUTHOR "mRAS"
#define DRIVER_DESC   "Memory RAS"

#define CHAR_DEVICE_NAME	"phys_mem"
#define DEVICE_CLASS_NAME	"phys_mem"
#define DEVICE_FILE_NAME	"phys_mem"

int g_phys_mem_major = PHYS_MEM_MAJOR;
int g_phys_mem_devs = MEMDEV_NR_DEVS; /* number of bare phys_mem devices */

#define MEMDEV_SIZE 4096

static struct class *dev_class;
struct phys_mem_dev *g_phys_mem_devices;

static void memdev_exit(void);

/* declare one cache pointer: use it for all session */
struct kmem_cache *g_session_mem_cache;

/*
 * Free all frame-stati in session->frame_stati  and
 * reset the session so, that the frame-stati-part is
 * 'Unconfigured'
 * Claimed pages in the frame-stati are freed.
 *
 *	The session lock must be held, when calling this function.
 */
unsigned long g_pre_isolate_ddr;
void free_page_stati(struct phys_mem_session *session)
{
	if (session == NULL)
		return;

	if (session->frame_stati == NULL) {
		g_pre_isolate_ddr = 0;
		session->num_frame_stati = 0;
		session->frame_stati = NULL;
		return;
	}

	if (session->num_frame_stati) {
		size_t i;

		for (i = 0; i < session->num_frame_stati; i++) {
			unsigned long start_pfn =
			session->frame_stati[i].pfn;

			if (start_pfn != 0) {
				session->frame_stati[i].page = NULL;
				if (likely(g_pre_isolate_ddr !=
					PRE_IOSLATE_NUM)) {
					free_contig_range(start_pfn,
					1<<g_conti_order);
				}
			}
		}
	}
	SESSION_FREE_FRAME_STATI(session->frame_stati);

	g_pre_isolate_ddr = 0;

	session->num_frame_stati = 0;
	session->frame_stati = NULL;
}

static void phys_mem_setup_cdev(struct phys_mem_dev *dev, int index)
{
	int err;
	int devno = MKDEV(g_phys_mem_major, index);

	if (dev == NULL)
		return;

	cdev_init(&dev->cdev, &phys_mem_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &phys_mem_fops;
	err = cdev_add(&dev->cdev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		pr_err("Error %d adding "CHAR_DEVICE_NAME"%d", err, index);
}

/* load mem dev driver module*/
static int __init memdev_init(void)
{
	int result;
	int i;
	int malloc_size;

	dev_t devno = MKDEV(g_phys_mem_major, 0);

	/* apply device number statically/dynamically */
	if (g_phys_mem_major)
		result = register_chrdev_region(devno,
		g_phys_mem_devs, CHAR_DEVICE_NAME);
	else {
		result = alloc_chrdev_region(&devno, 0,
		g_phys_mem_devs, CHAR_DEVICE_NAME);
		g_phys_mem_major = MAJOR(devno);
	}

	if (result < 0)
		return result;

	dev_class = class_create(THIS_MODULE, DEVICE_CLASS_NAME);
	if (IS_ERR(dev_class))
		pr_err("no udev support\n");

	/* alloc memory for device descriptor */
	malloc_size = g_phys_mem_devs * sizeof(struct phys_mem_dev);
	g_phys_mem_devices = kmalloc(malloc_size, GFP_KERNEL);
	if (!g_phys_mem_devices) {
		result = -ENOMEM;
		unregister_chrdev_region(devno, 1);
		return result;
	}

	if (memset_s(g_phys_mem_devices,
		g_phys_mem_devs * sizeof(struct phys_mem_dev), 0,
		g_phys_mem_devs * sizeof(struct phys_mem_dev)) != EOK) {
		kfree(g_phys_mem_devices);
		pr_err("%s memset_s failed\n", __func__);
		return -ENOMEM;
	}

	for (i = 0; i < g_phys_mem_devs; i++) {
		sema_init(&g_phys_mem_devices[i].sem, 1);
		phys_mem_setup_cdev(g_phys_mem_devices + i, i);
		if (!IS_ERR(dev_class)) {
			device_create(dev_class, NULL,
			MKDEV(g_phys_mem_major, i),
			NULL, CHAR_DEVICE_NAME);
		}
	}

	g_session_mem_cache = kmem_cache_create("session_mem",
		sizeof(struct phys_mem_session),
		0, SLAB_HWCACHE_ALIGN, NULL);
	if (!g_session_mem_cache) {
		memdev_exit();
		return -ENOMEM;
	}

	return 0;
}

/* unload device driver module */
static void __exit memdev_exit(void)
{
	int i;

	if (!IS_ERR(dev_class)) {
		for (i = 0; i < g_phys_mem_devs; i++)
			device_destroy(dev_class, MKDEV(g_phys_mem_major, i));
		class_destroy(dev_class);
	}

	for (i = 0; i < g_phys_mem_devs; i++)
		cdev_del(&g_phys_mem_devices[i].cdev);
	kfree(g_phys_mem_devices);

	if (g_session_mem_cache != NULL)
		kmem_cache_destroy(g_session_mem_cache);

	/* release device number */
	unregister_chrdev_region(MKDEV(g_phys_mem_major, 0), g_phys_mem_devs);

}

module_init(memdev_init);
module_exit(memdev_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
