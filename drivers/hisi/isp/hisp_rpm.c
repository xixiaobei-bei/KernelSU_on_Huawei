/*
 * Histarisp rpmsg client driver
 *
 * Copyright (c) 2013- ISP Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/jiffies.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/skbuff.h>
#include <linux/sched.h>
#include <linux/rpmsg.h>
#include <linux/completion.h>
#include <linux/ion.h>
#include <linux/dma-buf.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/hisi-iommu.h>
#include <linux/iommu.h>
#include <linux/version.h>
#include <linux/miscdevice.h>
#include <uapi/linux/histarisp.h>
#include <linux/platform_data/remoteproc_hisp.h>
#include <linux/platform_data/hisp_mempool.h>
#include <securec.h>

#define DBG_BIT   (1 << 2)
#define INF_BIT   (1 << 1)
#define ERR_BIT   (1 << 0)
#define HISP_NAME_LENGTH (64)
#define HISP_LOG_SIZE    128

static unsigned int debug_mask = (INF_BIT | ERR_BIT);



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

struct rpmsg_isp_service {
	struct rpmsg_device *rpdev;
};


struct hisp_rpmsgrefs_s {
	atomic_t sendin_refs;
	atomic_t sendx_refs;
	atomic_t recvtask_refs;
	atomic_t recvthread_refs;
	atomic_t recvin_refs;
	atomic_t recvcb_refs;
	atomic_t recvdone_refs;
	atomic_t rdr_refs;
};

struct hisp_rpmsgrefs_s *hisp_rpmsgrefs;


static struct rpmsg_isp_service hisp_serv;
struct completion channel_sync;

struct rpmsg_channel_info chinfo = {
	.src = RPMSG_ADDR_ANY,
};

static void hisplog(unsigned int on, const char *tag,
		const char *func, const char *fmt, ...)
{
	va_list args;
	char pbuf[HISP_LOG_SIZE] = {0};
	int size;

	if ((debug_mask & on) == 0 || fmt == NULL)
		return;

	va_start(args, fmt);
	size = vsnprintf_s(pbuf, HISP_LOG_SIZE,
		HISP_LOG_SIZE - 1, fmt, args);/*lint !e592 */
	va_end(args);

	if ((size < 0) || (size > HISP_LOG_SIZE - 1))
		return;

	pbuf[size] = '\0';
	pr_info("Rpmsg ISP [%s][%s] %s", tag, func, pbuf);
}

#define rpmsg_dbg(fmt, ...) hisplog(DBG_BIT, "D", __func__, fmt, ##__VA_ARGS__)
#define rpmsg_info(fmt, ...) hisplog(INF_BIT, "I", __func__, fmt, ##__VA_ARGS__)
#define rpmsg_err(fmt, ...) hisplog(ERR_BIT, "E", __func__, fmt, ##__VA_ARGS__)

static void hisp_rpmsg_rdr_save(unsigned int num, unsigned int type, void *data)
{
}

int hisp_rpmsg_rdr_init(void)
{
	struct hisp_rpmsgrefs_s *dev = NULL;

	dev = kzalloc(sizeof(struct hisp_rpmsgrefs_s), GFP_KERNEL);
	if (dev == NULL)
		return -ENOMEM;

	rpmsg_info("kzalloc size.0x%lx\n", sizeof(struct hisp_rpmsgrefs_s));
	hisp_rpmsgrefs = dev;

	return 0;
}

int hisp_rpmsg_rdr_deinit(void)
{
	struct hisp_rpmsgrefs_s *dev = hisp_rpmsgrefs;

	if (dev == NULL) {
		rpmsg_err("Failed: hisp_rpmsgrefs_s is null\n");
		return -ENOMEM;
	}

	kfree(dev);
	hisp_rpmsgrefs = NULL;

	return 0;
}

void hisp_sendin(void *data)
{
	struct hisp_rpmsgrefs_s *dev = hisp_rpmsgrefs;

	if (dev == NULL) {
		rpmsg_err("Failed: hisp_rpmsgrefs_s is null\n");
		return;
	}

	atomic_inc(&dev->sendin_refs);
	atomic_inc(&dev->rdr_refs);
	hisp_rpmsg_rdr_save(atomic_read(&dev->rdr_refs),
				RPMSG_CAMERA_SEND_MSG, data);
}

void hisp_sendx(void)
{
	struct hisp_rpmsgrefs_s *dev = hisp_rpmsgrefs;

	if (dev == NULL) {
		rpmsg_err("Failed: hisp_rpmsgrefs_s is null\n");
		return;
	}

	atomic_inc(&dev->sendx_refs);
	atomic_inc(&dev->rdr_refs);
	hisp_rpmsg_rdr_save(atomic_read(&dev->rdr_refs),
				RPMSG_SEND_MSG_TO_MAILBOX, NULL);
}

void hisp_recvtask(void)
{
	struct hisp_rpmsgrefs_s *dev = hisp_rpmsgrefs;

	if (dev == NULL) {
		rpmsg_err("Failed: hisp_rpmsgrefs_s is null\n");
		return;
	}

	atomic_inc(&dev->recvtask_refs);
	atomic_inc(&dev->rdr_refs);
	hisp_rpmsg_rdr_save(atomic_read(&dev->rdr_refs),
				RPMSG_RECV_MAILBOX_FROM_ISPCPU, NULL);
}

void hisp_recvthread(void)
{
	struct hisp_rpmsgrefs_s *dev = hisp_rpmsgrefs;

	if (dev == NULL) {
		rpmsg_err("Failed: hisp_rpmsgrefs_s is null\n");
		return;
	}

	atomic_inc(&dev->recvthread_refs);
	atomic_inc(&dev->rdr_refs);
	hisp_rpmsg_rdr_save(atomic_read(&dev->rdr_refs),
				RPMSG_ISP_THREAD_RECVED, NULL);
}

void hisp_recvin(void *data)
{
	struct hisp_rpmsgrefs_s *dev = hisp_rpmsgrefs;

	if (dev == NULL) {
		rpmsg_err("Failed: hisp_rpmsgrefs_s is null\n");
		return;
	}

	atomic_inc(&dev->recvin_refs);
	atomic_inc(&dev->rdr_refs);
	hisp_rpmsg_rdr_save(atomic_read(&dev->rdr_refs),
				RPMSG_RECV_SINGLE_MSG, data);
}

void hisp_recvx(void *data)
{
	struct hisp_rpmsgrefs_s *dev = hisp_rpmsgrefs;

	if (dev == NULL) {
		rpmsg_err("Failed: hisp_rpmsgrefs_s is null\n");
		return;
	}

	atomic_inc(&dev->recvcb_refs);
	atomic_inc(&dev->rdr_refs);
	hisp_rpmsg_rdr_save(atomic_read(&dev->rdr_refs),
				RPMSG_SINGLE_MSG_TO_CAMERA, data);
}

void hisp_recvdone(void *data)
{
	struct hisp_rpmsgrefs_s *dev = hisp_rpmsgrefs;

	if (dev == NULL) {
		rpmsg_err("Failed: hisp_rpmsgrefs_s is null\n");
		return;
	}

	atomic_inc(&dev->recvdone_refs);
	atomic_inc(&dev->rdr_refs);
	hisp_rpmsg_rdr_save(atomic_read(&dev->rdr_refs),
				RPMSG_CAMERA_MSG_RECVED, data);
}

void hisp_rpmsgrefs_dump(void)
{
	struct hisp_rpmsgrefs_s *dev = hisp_rpmsgrefs;

	rpmsg_info("cam send info: sendin_refs.0x%x\n",
			atomic_read(&dev->sendin_refs));
	rpmsg_info("cam send info: sendx_refs.0x%x\n",
			atomic_read(&dev->sendx_refs));
	rpmsg_info("Rpg recv info: recvtask_refs.0x%x\n",
			atomic_read(&dev->recvtask_refs));
	rpmsg_info("Rpg recv info: recvthread_refs.0x%x\n",
			atomic_read(&dev->recvthread_refs));
	rpmsg_info("Rpg recv info: recvin_refs.0x%x\n",
			atomic_read(&dev->recvin_refs));
	rpmsg_info("cam recv info: recvcb_refs.0x%x\n",
			atomic_read(&dev->recvcb_refs));
	rpmsg_info("cam recv info: recvdone_refs.0x%x\n",
			atomic_read(&dev->recvdone_refs));
	rpmsg_info("cam recv info: total: rdr_refs.0x%x\n",
			atomic_read(&dev->rdr_refs));
}

void hisp_rpmsgrefs_reset(void)
{
	struct hisp_rpmsgrefs_s *dev = hisp_rpmsgrefs;

	if (dev == NULL) {
		rpmsg_err("Failed: hisp_rpmsgrefs_s is null\n");
		return;
	}

	atomic_set(&dev->sendin_refs, 0);
	atomic_set(&dev->sendx_refs, 0);
	atomic_set(&dev->recvtask_refs, 0);
	atomic_set(&dev->recvthread_refs, 0);
	atomic_set(&dev->recvin_refs, 0);
	atomic_set(&dev->recvcb_refs, 0);
	atomic_set(&dev->recvdone_refs, 0);
	atomic_set(&dev->rdr_refs, 0);
}
void hisp_dump_rpmsg_with_id(const unsigned int message_id)
{
}

static int rpmsg_isp_probe(struct rpmsg_device *rpdev)
{
	struct rpmsg_isp_service *isp_serv = &hisp_serv;

	isp_serv->rpdev = rpdev;

	rpmsg_info("new HISP connection srv channel: %u -> %u!\n",
			rpdev->src, rpdev->dst);
	complete(&channel_sync);
	rpmsg_dbg("Exit ...\n");
	return 0;
}

static void rpmsg_isp_remove(struct rpmsg_device *rpdev)
{
	rpmsg_info("Exit ...\n");
}

static int rpmsg_isp_driver_cb(struct rpmsg_device *rpdev,
				void *data, int len, void *priv, u32 src)
{
	rpmsg_dbg("Enter ...\n");
	dev_warn(&rpdev->dev, "uhm, unexpected message\n");

	print_hex_dump(KERN_DEBUG, __func__, DUMP_PREFIX_NONE, 16, 1,
			data, len, true);
	rpmsg_dbg("Exit ...\n");
	return 0;
}

static struct rpmsg_device_id rpmsg_isp_id_table[] = {
	{ .name    = "rpmsg-isp-debug" },
	{ },
};
MODULE_DEVICE_TABLE(platform, rpmsg_isp_id_table);
/*lint -save -e485*/
static struct rpmsg_driver rpmsg_isp_driver = {
	.drv.name   = KBUILD_MODNAME,
	.drv.owner  = THIS_MODULE,
	.id_table   = rpmsg_isp_id_table,
	.probe      = rpmsg_isp_probe,
	.callback   = rpmsg_isp_driver_cb,
	.remove     = rpmsg_isp_remove,
};
/*lint -restore */


static int __init rpmsg_isp_init(void)
{
	init_completion(&channel_sync);
	return register_rpmsg_driver(&rpmsg_isp_driver);
}
module_init(rpmsg_isp_init);

static void __exit rpmsg_isp_exit(void)
{
	rpmsg_dbg("Enter ...\n");

	unregister_rpmsg_driver(&rpmsg_isp_driver);


	rpmsg_dbg("Exit ...\n");
}
module_exit(rpmsg_isp_exit);

MODULE_DESCRIPTION("ISP offloading rpmsg driver");
MODULE_LICENSE("GPL v2");
