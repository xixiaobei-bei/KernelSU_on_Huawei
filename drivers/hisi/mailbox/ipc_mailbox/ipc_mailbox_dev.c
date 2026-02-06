/*
 * ipc_mailbox_dev.c
 *
 * IPC mailbox device driver
 *
 * Copyright (c) 2018-2020 Huawei Technologies Co., Ltd.
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
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/hisi/hisi_irq_affinity.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kern_levels.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <securec.h>
#include <linux/hisi/hisi_rproc.h>
#include <soc_sctrl_interface.h>
#include <pr_log.h>

#include "ipc_mailbox.h"
#include "ipc_mailbox_dev.h"
#include "ipc_rproc_id_mgr.h"
#include "ipc_rproc_inner.h"

#define PR_LOG_TAG AP_MAILBOX_TAG

#define FAST_MBOX (1 << 0)
#define COMM_MBOX (1 << 1)
#define SOURCE_MBOX (1 << 2)
#define DESTINATION_MBOX (1 << 3)

#define EVERY_LOOP_TIME_MS 5

#define IPC_UNLOCKED 0x00000000
#define IPCACKMSG 0x00000000
#define COMM_MBOX_IRQ (-2)
#define AUTOMATIC_ACK_CONFIG (1 << 0)
#define NO_FUNC_CONFIG (0 << 0)

/* Optimize interrupts assignment */
#define IPC_IRQ_AFFINITY_CPU 1

#define STATE_NUMBER 0x4

#define MAILBOX_ASYNC_UDELAY_CNT 1000
#define DEFAULT_MAILBOX_TIMEOUT 300
#define DEFAULT_FIFO_SIZE 256
#define DEFAULT_SCHED_PRIORITY 20
#define MAILBOX_NO_USED 0
#define MAX_AP_IPC_INDEX 99

#define RPROC_NUMBER 16
#define IPC_MAX_SRC_ID 2
#define IPC_FUNC 3
#define IPC_RESIDUE 100

#define MAX_MBX_CHANNEL_ID   256
#define MBX_IRQ_REGION_SIZE  32
#define MAX_MBX_CHANNEL_REGION_NUM (MAX_MBX_CHANNEL_ID / MBX_IRQ_REGION_SIZE)

#define SCTRL_TIME_ADDR (SOC_SCTRL_SCBBPDRXSTAT1_ADDR(0))
#define MDEV_MAX_ANALYSIS_BUFFER 512
#define INVALID_RPROC_ID              0xFFFFFFFF
#define MAX_MBOX_NODE                 256

#define mdev_err(fmt, args...)   pr_err(fmt "\n", ##args)

#define mdev_info(fmt, args...)  pr_info(fmt "\n", ##args)

#define mdev_debug(fmt, args...)

enum {
	RX_BUFFER_TYPE = 0,
	ACK_BUFFER_TYPE,
	MBOX_BUFFER_TYPE_MAX
};

/*
 * Table for available remote processors. DTS sub-node, "remote_processor_type",
 * of node, "ipc_mdev", is configured according to the table.
 * If the table was modified, DTS configiuration should be updated accordingly.
 */
enum remote_processor_type_t {
	GIC = 0,
	GIC_1 = 0,
	GIC_2,
	IOM3,
	LPM3,
	HIFI,
	MCPU,
	BBE16,
	IVP32,
	ISP,
	UNCERTAIN_REMOTE_PROCESSOR,
	NPU_IPC_GIC = 2,
	HI3XXX_RP_TYPES,
	INVALID_PROCESSOR_TYPE
};

struct ipc_common_mbox_info {
	int gic_1_irq_requested;
	int gic_2_irq_requested;
	int cmbox_gic_1_irq;
	int cmbox_gic_2_irq;
	struct hisi_mbox_device *cmdev;
};

struct ipc_device {
	void __iomem *base;
	void __iomem *system_time_reg_addr;
	unsigned int unlock;
	mbox_msg_t *buf_pool;
	unsigned int ipc_type;
	unsigned int capability;
	unsigned int rproc_src_id[IPC_MAX_SRC_ID];
	const char *rproc_name[RPROC_NUMBER];
	struct ipc_common_mbox_info *cmbox_info;
	struct hisi_mbox_device **mdev_res;
	unsigned int ipc_version;
	unsigned int reg_spc;
};

struct ipc_mbox_device_priv {
	u8 func;
	enum remote_processor_type_t src;
	enum remote_processor_type_t des;
	unsigned int mbox_channel;
	int irq;
	int capability;
	int used;
	unsigned int timeout;
	unsigned int fifo_size;
	unsigned int sched_priority;
	unsigned int sched_policy;
	struct ipc_device *idev;
	int rproc_id;
};

/*
 * HiIPCV230 have a state machine, the state machine have 4 status:
 * 4'b0001:IDLE_STATE
 * 4'b0010:SOURCE_STATE
 * 4'b0100:DEST_STATE
 * 4'b1000:ACK_STATE
 */
char *ipc_state_name[STATE_NUMBER] = {
	"%s  is idle\n",
	"%s  is occupied\n",
	"%s  may be power off or freeze\n",
	"%s  have no time to handle ACK\n"
};

enum IPC_STATE_MACHINE {
	IDLE_STATE,
	SOURCE_STATE,
	DEST_STATE,
	ACK_STATE
};

static unsigned char _rproc_find_index(unsigned int pro_code)
{
	unsigned char index = 0;

	while (pro_code) {
		index++;
		pro_code >>= 1;
	}
	return index;
}

static const char *rproc_analysis(
	struct hisi_mbox_device *mdev, unsigned int pro_code)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;
	unsigned char index = _rproc_find_index(pro_code);

	if (!index)
		return "ERR_RPROC";
	index--;
	if (likely(index < RPROC_NUMBER))
		return (priv->idev->rproc_name[index]);
	else
		return "ERR_RPROC";
}

static char *ipc_state_analysis(unsigned int mode, unsigned char *outp)
{
	unsigned char index = 0;

	if (!outp)
		return "%s ERR_STATE\n";

	mode >>= 4; /* bit4~bit7 is the state machine index */
	while (mode) {
		index++;
		mode >>= 1;
	}
	if (likely(index))
		index--;
	else
		return "%s ERR_STATE\n";

	*outp = index;

	if (likely(index < STATE_NUMBER))
		return ipc_state_name[index];
	else
		return "%s ERR_STATE\n";
}

static inline void __ipc_unlock(struct ipc_device *idev)
{
	__raw_writel(idev->unlock, idev->base + ipc_lock(idev->ipc_version));
}

static inline unsigned int __ipc_lock_status(struct ipc_device *idev)
{
	return __raw_readl(idev->base + ipc_lock(idev->ipc_version));
}

static inline void __ipc_set_src(struct ipc_mbox_device_priv *priv)
{
	__raw_writel(ipc_bitmask((unsigned int)priv->src),
		priv->idev->base +
		ipc_mbx_source(priv->mbox_channel, priv->idev->reg_spc));
}

static inline unsigned int __ipc_read_src(struct ipc_mbox_device_priv *priv)
{
	return __raw_readl(priv->idev->base +
		ipc_mbx_source(priv->mbox_channel, priv->idev->reg_spc));
}

static inline void __ipc_set_des(struct ipc_mbox_device_priv *priv)
{
	__raw_writel(ipc_bitmask((unsigned int)priv->des), priv->idev->base +
		ipc_mbx_dset(priv->mbox_channel, priv->idev->reg_spc));
}

static inline unsigned int __ipc_des_status(struct ipc_mbox_device_priv *priv)
{
	return __raw_readl(priv->idev->base +
		ipc_mbx_dstatus(priv->mbox_channel, priv->idev->reg_spc));
}

static inline void __ipc_send(
	struct ipc_mbox_device_priv *priv, unsigned int tosend)
{
	__raw_writel(tosend, priv->idev->base +
		ipc_mbx_send(priv->mbox_channel, priv->idev->reg_spc));
}

static inline unsigned int __ipc_read(
	struct ipc_mbox_device_priv *priv, int index)
{
	return __raw_readl(priv->idev->base + ipc_mbx_data(priv->mbox_channel,
		(unsigned int)index, priv->idev->reg_spc));
}

static inline void __ipc_write(
	struct ipc_mbox_device_priv *priv, u32 data, int index)
{
	__raw_writel(data, priv->idev->base + ipc_mbx_data(priv->mbox_channel,
		(unsigned int)index, priv->idev->reg_spc));
}

static inline unsigned int __ipc_cpu_imask_get(
	struct ipc_mbox_device_priv *priv)
{
	return __raw_readl(priv->idev->base +
		ipc_mbx_imask(priv->mbox_channel, priv->idev->reg_spc));
}

static void __ipc_cpu_imask_clr(
	struct ipc_mbox_device_priv *priv, unsigned int toclr)
{
	unsigned int reg;

	reg = __raw_readl(priv->idev->base +
		ipc_mbx_imask(priv->mbox_channel, priv->idev->reg_spc));
	reg = reg & (~(toclr));

	__raw_writel(reg, priv->idev->base +
		ipc_mbx_imask(priv->mbox_channel, priv->idev->reg_spc));
}

static inline void __ipc_cpu_imask_all(struct ipc_mbox_device_priv *priv)
{
	__raw_writel((~0), priv->idev->base +
		ipc_mbx_imask(priv->mbox_channel, priv->idev->reg_spc));
}

static inline void __ipc_cpu_iclr(
	struct ipc_mbox_device_priv *priv, unsigned int toclr)
{
	__raw_writel(toclr, priv->idev->base +
		ipc_mbx_iclr(priv->mbox_channel, priv->idev->reg_spc));
}

static inline unsigned int __ipc_status(struct ipc_mbox_device_priv *priv)
{
	return __raw_readl(priv->idev->base +
		ipc_mbx_mode(priv->mbox_channel, priv->idev->reg_spc));
}


static unsigned int __ipc_mbox_istatus(
	struct ipc_mbox_device_priv *priv, unsigned int src,
	unsigned int channel_region)
{
	if (priv->idev->ipc_version == IPC_VERSION_BOY)
		return __raw_readl(priv->idev->base +
			ipc_cpux_imst_ex((unsigned int)src, channel_region));
	else
		return __raw_readl(
			priv->idev->base + ipc_cpux_imst((unsigned int)src));
}

static inline void __ipc_mode(
	struct ipc_mbox_device_priv *priv, unsigned int mode)
{
	__raw_writel(mode, priv->idev->base +
		ipc_mbx_mode(priv->mbox_channel, priv->idev->reg_spc));
}

static int ipc_mdev_startup(struct hisi_mbox_device *mdev)
{
	/*
	 * nothing will be done during suspend & resume flow for HI3xxx IPC.
	 * see dummy like SR function, ipc_mdev_suspend & ipc_mdev_resume.
	 * reserve runtime power management proceeding for further modification,
	 * if necessary.
	 */
	return 0;
}

static void ipc_mdev_shutdown(struct hisi_mbox_device *mdev)
{
	/*
	 * nothing will be done during suspend & resume flow for HI3xxx IPC.
	 * see dummy like SR function, ipc_mdev_suspend & ipc_mdev_resume.
	 * reserve runtime power management proceeding for further modification,
	 * if necessary.
	 */
}

static int ipc_mdev_build_dump_fmt_str(
	char *finalfortmat, unsigned int fmt_size,
	struct ipc_mbox_device_priv *priv, unsigned char *statem)
{
	/* \0013 is the  KERN_SOH KERN_ERR */
	char *direcstr = KERN_ERR "[ap_ipc]: [%d] [%s]-->[%s], ";
	char *machinestr = ipc_state_analysis(__ipc_status(priv), statem);

	if (memcpy_s(finalfortmat, fmt_size, direcstr, strlen(direcstr))
		!= EOK) {
		mdev_err("%s memcpy_s error", __func__);
		return -1;
	}

	if (strncat_s(finalfortmat, fmt_size - strlen(direcstr), machinestr,
		strlen(machinestr)) != EOK) {
		mdev_err("%s strncat_s error", __func__);
		return -1;
	}

	return 0;
}

static void ipc_mdev_dump_status(struct hisi_mbox_device *mdev)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;
	/*
	 * the size 512 is the sumary max size of sys_rproc_name and
	 * ipc_state_name
	 */
	char finalfortmat[MDEV_MAX_ANALYSIS_BUFFER] = {0};
	char statem = 0;
	const char *src_name = rproc_analysis(mdev, __ipc_read_src(priv));
	const char *dest_name = rproc_analysis(mdev, __ipc_des_status(priv));

	if (ipc_mdev_build_dump_fmt_str(
		finalfortmat, MDEV_MAX_ANALYSIS_BUFFER, priv,
		(unsigned char *)&statem))
		return;

	if (statem == DEST_STATE)
		printk(finalfortmat, priv->rproc_id,
			src_name, dest_name, dest_name);
	else if (statem == ACK_STATE)
		printk(finalfortmat, priv->rproc_id,
			src_name, dest_name, src_name);
	else
		printk(finalfortmat, priv->rproc_id,
			src_name, dest_name, mdev->name);
}

static void ipc_mdev_dump_regs(struct hisi_mbox_device *mdev)
{
}

static int ipc_mdev_check(struct hisi_mbox_device *mdev,
	enum mbox_mail_type_t mtype, int rproc_id)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;
	int index = priv->rproc_id;

	if ((mtype == TX_MAIL) && (SOURCE_MBOX & priv->func) &&
		(index == rproc_id) && (priv->used == 1))
		return RPACCESSIBLE;

	if ((mtype == RX_MAIL) && (DESTINATION_MBOX & priv->func) &&
		 (index == rproc_id) && (priv->used == 1))
		return RPACCESSIBLE;

	return RPUNACCESSIBLE;
}

static void ipc_mdev_clr_ack(struct hisi_mbox_device *mdev)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;
	unsigned int imask;
	unsigned int toclr;

	imask = __ipc_cpu_imask_get(priv);
	toclr = (ipc_bitmask(priv->idev->rproc_src_id[0]) |
		ipc_bitmask(priv->idev->rproc_src_id[1])) & (~imask);

	__ipc_cpu_iclr(priv, toclr);
}

static void ipc_mdev_clr_irq_and_ack(struct hisi_mbox_device *mdev)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;
	unsigned int status;
	unsigned int imask;
	unsigned int todo;
	int i;

	/*
	 * Temporarily, local processor will clean msg register,
	 * and ack zero for an ipc from remote processors.
	 */
	for (i = 0; i < priv->capability; i++)
		__ipc_write(priv, IPCACKMSG, i);

	imask = __ipc_cpu_imask_get(priv);
	/*
	 * Get the irq unmask core bits, and clear the irq according to the
	 * unmask core bits,
	 * because the irq to be sure triggered to the unmasked cores
	 */
	todo = (ipc_bitmask(priv->idev->rproc_src_id[0]) |
		ipc_bitmask(priv->idev->rproc_src_id[1])) & (~imask);

	__ipc_cpu_iclr(priv, todo);

	status = __ipc_status(priv);
	if ((DESTINATION_STATUS & status) &&
		(!(AUTOMATIC_ACK_CONFIG & status)))
		__ipc_send(priv, todo);
}

static void ipc_mdev_ack(
	struct hisi_mbox_device *mdev, mbox_msg_t *msg, mbox_msg_len_t len)
{
}

static mbox_msg_len_t ipc_mdev_hw_read(
	struct hisi_mbox_device *mdev, mbox_msg_t *msg)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;
	mbox_msg_len_t cap = priv->capability;
	int i;

	for (i = 0; i < cap; i++)
		msg[i] = __ipc_read(priv, i);

	return cap;
}

/*
 * to judge the four kind machine status of the ip,
 * they are idle,src,des,ack
 */
static int ipc_mdev_is_stm(struct hisi_mbox_device *mdev, unsigned int stm)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;
	int is_stm = 0;

	if ((stm & __ipc_status(priv)))
		is_stm = 1;

	return is_stm;
}

static mbox_msg_len_t ipc_mdev_receive_msg(
	struct hisi_mbox_device *mdev, mbox_msg_t **buf)
{
	mbox_msg_t *_buf = NULL;
	mbox_msg_len_t len = 0;

	if (!buf)
		return len;

	if (ipc_mdev_is_stm(mdev, ACK_STATUS))
		_buf = mdev->ack_buffer;
	else
		_buf = mdev->rx_buffer;

	if (_buf)
		len = ipc_mdev_hw_read(mdev, _buf);
	*buf = _buf;

	ipc_mdev_clr_irq_and_ack(mdev);
	return len;
}

static int ipc_mdev_unlock(struct hisi_mbox_device *mdev)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;
	int retry = 3; /* unclok retry cnt */

	do {
		__ipc_unlock(priv->idev);
		if (__ipc_lock_status(priv->idev) == IPC_UNLOCKED)
			break;

		udelay(10); /* wait for ipc unlock */
		retry--;
	} while (retry);

	if (!retry)
		return -ENODEV;

	return 0;
}

static int ipc_mdev_occupy(struct hisi_mbox_device *mdev)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;
	int retry = 10; /* Mailbox status judgment times */

	do {
		/*
		 * Hardware lock
		 * A hardware lock is needed here to lock a mailbox resource,
		 * which could be used by another remote proccessor, such as
		 * a HiIPCV230 common mailbox-25/mailbox-26.
		 */
		if (!(__ipc_status(priv) & IDLE_STATUS)) {
			asm volatile("wfe");
		} else {
			/*
			 * set the source processor bit, we set common mailbox's
			 * source processor bit through dtsi
			 */
			__ipc_set_src(priv);
			if (__ipc_read_src(priv) &
				ipc_bitmask((unsigned int)priv->src))
				break;
		}
		retry--;
		/* Hardware unlock */
	} while (retry);

	if (!retry)
		return -ENODEV;

	return 0;
}

static int ipc_mdev_hw_send(struct hisi_mbox_device *mdev, mbox_msg_t *msg,
	mbox_msg_len_t len, int ack_mode)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;
	int i, min_len;
	unsigned int temp;

	/* interrupts unmask */
	__ipc_cpu_imask_all(priv);

	if (ack_mode == AUTO_ACK)
		temp = ipc_bitmask((unsigned int)priv->des);
	else
		temp = ipc_bitmask((unsigned int)priv->src) |
			ipc_bitmask((unsigned int)priv->des);

	__ipc_cpu_imask_clr(priv, temp);

	/* des config */
	__ipc_set_des(priv);

	/* ipc mode config */
	if (ack_mode == AUTO_ACK)
		temp = AUTOMATIC_ACK_CONFIG;
	else
		temp = NO_FUNC_CONFIG;

	__ipc_mode(priv, temp);

	/* write data */
	min_len = (priv->capability < len) ? priv->capability : len;
	for (i = 0; i < min_len; i++)
		__ipc_write(priv, msg[i], i);

	/* enable sending */
	__ipc_send(priv, ipc_bitmask((unsigned int)priv->src));
	return 0;
}

static void ipc_mdev_ensure_channel(struct hisi_mbox_device *mdev)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;
	int loop = priv->timeout /
		EVERY_LOOP_TIME_MS + MAILBOX_ASYNC_UDELAY_CNT;
	int timeout = 0;

	if (mdev->ops->is_stm(mdev, IDLE_STATUS)) {
		return; /* IDLE STATUS, return directly */
	} else if (mdev->ops->is_stm(mdev, ACK_STATUS)) {
		/*
		 * the ack status is reached, just release, the sync and async
		 * is mutexed by by mdev->dev_lock
		 */
		goto release; /* ACK STATUS, release the channel directly */
	} else { /* DEST and SRC STATUS, the dest is processing, wait here */
		/* the worst situation is to delay 1000*5us+60*5ms = 305ms */
		while (timeout < loop) {
			if (timeout < MAILBOX_ASYNC_UDELAY_CNT) {
				udelay(5);
			} else {
				/*
				 * the hifi may power off when send ipc msg, so
				 * the ack status may wait 20ms
				 *
				 * Min pause 3000us and max pause 5000us
				 */
				usleep_range(3000, 5000);
			}
			/* if the ack status is ready, break out */
			if (mdev->ops->is_stm(mdev, ACK_STATUS))
				break;
			timeout++;
		}
		if (unlikely(timeout == loop)) {
			mdev_err("%s ipc_timeout", mdev->name);
			if (mdev->ops->status)
				mdev->ops->status(mdev);
		}
		goto release;
	}

release:
	/* release the channel */
	mdev->ops->refresh(mdev);
}

static int ipc_mdev_send_msg(struct hisi_mbox_device *mdev, mbox_msg_t *msg,
	mbox_msg_len_t len, int ack_mode)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;
	int err = 0;

	/* all the mailbox channel is treated as fast-mailbox */
	if (DESTINATION_MBOX & priv->func) {
		mdev_err("mdev %s has no tx ability", mdev->name);
		err = -EMDEVCLEAN;
		return err;
	}
	/*
	 * Whenever an ipc starts,
	 * ipc module has to be unlocked at the very beginning.
	 */
	if (ipc_mdev_unlock(mdev)) {
		mdev_err("mdev %s can not be unlocked", mdev->name);
		err = -EMDEVCLEAN;
		return err;
	}

	if (ipc_mdev_occupy(mdev)) {
		mdev_err("mdev %s can not be occupied", mdev->name);
		err = -EMDEVCLEAN;
		return err;
	}

	(void)ipc_mdev_hw_send(mdev, msg, len, ack_mode);

	return err;
}

static void ipc_mdev_release(struct hisi_mbox_device *mdev)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;

	__ipc_cpu_imask_all(priv);
	__ipc_set_src(priv);

	asm volatile("sev");
}

static int ipc_mdev_gic_irq_request(struct hisi_mbox_device *mdev,
	irq_handler_t handler, void *dev_id)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;
	int ret;

	if (!strncmp("ao-mailbox", mdev->name, strlen("ao-mailbox")))
		ret = request_irq(priv->irq, handler,
			IRQF_NO_SUSPEND, mdev->name, dev_id);
	else if (!strncmp("mailbox-4", mdev->name, strlen("mailbox-4")))
		ret = request_irq(priv->irq, handler,
			IRQF_NO_SUSPEND, mdev->name, dev_id);
	else
		ret = request_irq(priv->irq, handler, 0, mdev->name, dev_id);

	return ret;
}

static int ipc_mdev_irq_request(
	struct hisi_mbox_device *mdev, irq_handler_t handler, void *p)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;
	int ret = 0;

	if (priv->idev->cmbox_info->cmbox_gic_1_irq == priv->irq) {
		if (priv->idev->cmbox_info->gic_1_irq_requested)
			return ret;

		ret = ipc_mdev_gic_irq_request(mdev, handler,
			(void *)priv->idev->cmbox_info->cmdev);
		if (ret) {
			mdev_err("fast source %s request gic_1_irq %d failed",
				mdev->name, priv->irq);
			return ret;
		}
		priv->idev->cmbox_info->gic_1_irq_requested++;
		hisi_irqaffinity_register(priv->irq, IPC_IRQ_AFFINITY_CPU);
	} else if (priv->idev->cmbox_info->cmbox_gic_2_irq == priv->irq) {
		if (priv->idev->cmbox_info->gic_2_irq_requested)
			return ret;

		ret = ipc_mdev_gic_irq_request(mdev, handler,
			(void *)priv->idev->cmbox_info->cmdev);
		if (ret) {
			mdev_err("fast source %s request gic_2_irq %d failed",
				mdev->name, priv->irq);
			return ret;
		}
		priv->idev->cmbox_info->gic_2_irq_requested++;
		hisi_irqaffinity_register(priv->irq, IPC_IRQ_AFFINITY_CPU);
	} else {
		ret = ipc_mdev_gic_irq_request(mdev, handler, p);
		if (ret) {
			mdev_err("fast desitnation %s request irq %d failed",
				mdev->name, priv->irq);
			return ret;
		}

		hisi_irqaffinity_register(priv->irq, IPC_IRQ_AFFINITY_CPU);
	}

	return ret;
}

static void ipc_mdev_irq_free(struct hisi_mbox_device *mdev, void *p)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;

	if (priv->idev->cmbox_info->cmbox_gic_1_irq == priv->irq) {
		if (!--priv->idev->cmbox_info->gic_1_irq_requested)
			free_irq(priv->irq,
				(void *)priv->idev->cmbox_info->cmdev);
	} else if (priv->idev->cmbox_info->cmbox_gic_2_irq == priv->irq) {
		if (!--priv->idev->cmbox_info->gic_2_irq_requested)
			free_irq(priv->irq,
				(void *)priv->idev->cmbox_info->cmdev);
	} else {
		free_irq(priv->irq, p);
	}
}

static void ipc_mdev_irq_enable(struct hisi_mbox_device *mdev)
{
	enable_irq((unsigned int)mdev->cur_irq);
}

static void ipc_mdev_irq_disable(struct hisi_mbox_device *mdev)
{
	disable_irq_nosync((unsigned int)mdev->cur_irq);
}

static void ipc_mbox_read_imst(unsigned int *reg_val, int arr_size,
	struct ipc_mbox_device_priv *priv, unsigned int src)
{
	int i;

	for (i = 0; i < arr_size; i++)
		reg_val[i] = __ipc_mbox_istatus(priv, src, i);
}

static struct hisi_mbox_device *ipc_mdev_match_irq_dev(
	struct list_head *list, struct ipc_mbox_device_priv *_priv,
	unsigned int src)
{
	struct ipc_mbox_device_priv *priv = NULL;
	struct hisi_mbox_device *mdev = NULL;
	unsigned int regval[MAX_MBX_CHANNEL_REGION_NUM] = {0};
	int ipc_type = _priv->idev->ipc_type;

	ipc_mbox_read_imst(regval, ARRAY_SIZE(regval), _priv, src);

	list_for_each_entry(mdev, list, node) {
		priv = mdev->priv;

		if (!(priv->func & SOURCE_MBOX) ||
			ipc_type != priv->idev->ipc_type)
			continue;

		if (regval[priv->mbox_channel / MBX_IRQ_REGION_SIZE] &
			ipc_bitmask(priv->mbox_channel % MBX_IRQ_REGION_SIZE))
			return mdev;
	}

	mdev_err("%s:unexpect irq", __func__);
	return NULL;
}

static enum remote_processor_type_t ipc_mbx_match_irq_src(
	struct ipc_mbox_device_priv *priv, int irq)
{
	if (priv->idev->rproc_src_id[0] == priv->idev->rproc_src_id[1])
		return priv->idev->rproc_src_id[0];

	/*
	 * fast source & common mailboxes share
	 * GIC_1 & GIC_2 irq number
	 */
	if (irq == priv->idev->cmbox_info->cmbox_gic_1_irq)
		return priv->idev->rproc_src_id[0];

	if (irq == priv->idev->cmbox_info->cmbox_gic_2_irq)
		return priv->idev->rproc_src_id[1];

	mdev_err("odd irq for ipc mailboxes");

	return INVALID_PROCESSOR_TYPE;
}

static struct hisi_mbox_device *ipc_mdev_irq_to_mdev(
	struct hisi_mbox_device *mdev, struct list_head *list, int irq)
{
	struct ipc_mbox_device_priv *priv = NULL;
	enum remote_processor_type_t src;

	if ((list_empty(list)) || (!mdev)) {
		mdev_err("invalid input");
		return NULL;
	}

	priv = mdev->priv;

	/* fast destination mailboxes use unique irq number */
	if ((DESTINATION_MBOX & priv->func) && (FAST_MBOX & priv->func))
		return mdev;

	src = ipc_mbx_match_irq_src(priv, irq);
	if (src == INVALID_PROCESSOR_TYPE)
		return NULL;

	return ipc_mdev_match_irq_dev(list, priv, src);
}

static unsigned int ipc_mdev_timeout(struct hisi_mbox_device *mdev)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;

	return priv->timeout;
}

static unsigned int ipc_mdev_fifo_size(struct hisi_mbox_device *mdev)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;

	return priv->fifo_size;
}

static unsigned int ipc_mdev_sched_priority(struct hisi_mbox_device *mdev)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;

	return priv->sched_priority;
}

static unsigned int ipc_mdev_sched_policy(struct hisi_mbox_device *mdev)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;

	return priv->sched_policy;
}

static int ipc_mdev_get_rproc_id(struct hisi_mbox_device *mdev)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;

	return priv->rproc_id;
}

static int ipc_mdev_get_ipc_version(struct hisi_mbox_device *mdev)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;

	return priv->idev->ipc_version;
}

static int ipc_mdev_get_channel_size(struct hisi_mbox_device *mdev)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;

	return priv->capability;
}

static int ipc_mdev_get_channel_id(struct hisi_mbox_device *mdev)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;

	return (int)priv->mbox_channel;
}

static int ipc_mdev_get_slice(struct hisi_mbox_device *mdev, u64 *slice)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;

	if (!priv->idev->system_time_reg_addr)
		return -ENODEV;

	*slice = readl(priv->idev->system_time_reg_addr);

	return 0;
}

static void ipc_mdev_show_mdev_info_title(void)
{
	mdev_err("%s%s%s",
		"----------------------------------",
		"----------------------------------",
		"----------------------------------");
	mdev_err("%14s %32s %10s %5s %11s %6s %6s %10s",
		"mdev_name", "rproc_name:id", "channel_id", "irq",
		"ipc_version", "from", "to", "fifo_size");
	mdev_err("%s%s%s",
		"----------------------------------",
		"----------------------------------",
		"----------------------------------");
}

static void ipc_mdev_show_one_mdev_info(struct hisi_mbox_device *mdev)
{
	struct ipc_mbox_device_priv *priv = mdev->priv;

	mdev_err("%14s %28s:%3d %10u %5d %11d %6s %6s %10u",
		mdev->name, ipc_get_rproc_name(priv->rproc_id), priv->rproc_id,
		priv->mbox_channel, priv->irq, priv->idev->ipc_version,
		priv->idev->rproc_name[priv->src],
		priv->idev->rproc_name[priv->des], priv->fifo_size);
}

static void ipc_mdev_show_mdev_info(struct hisi_mbox_device *mdev, int idx)
{
	if (idx == 0)
		ipc_mdev_show_mdev_info_title();

	ipc_mdev_show_one_mdev_info(mdev);
}

struct ipc_mbox_dev_ops ipc_mdev_ops = {
	.startup = ipc_mdev_startup,
	.shutdown = ipc_mdev_shutdown,
	.check = ipc_mdev_check,
	.recv = ipc_mdev_receive_msg,
	.send = ipc_mdev_send_msg,
	.ack = ipc_mdev_ack,
	.refresh = ipc_mdev_release,
	.get_timeout = ipc_mdev_timeout,
	.get_fifo_size = ipc_mdev_fifo_size,
	.get_sched_priority = ipc_mdev_sched_priority,
	.get_sched_policy = ipc_mdev_sched_policy,
	.get_rproc_id = ipc_mdev_get_rproc_id,
	.get_ipc_version = ipc_mdev_get_ipc_version,
	.get_channel_size = ipc_mdev_get_channel_size,
	.get_channel_id = ipc_mdev_get_channel_id,
	.get_slice = ipc_mdev_get_slice,
	.request_irq = ipc_mdev_irq_request,
	.free_irq = ipc_mdev_irq_free,
	.enable_irq = ipc_mdev_irq_enable,
	.disable_irq = ipc_mdev_irq_disable,
	.irq_to_mdev = ipc_mdev_irq_to_mdev,
	.is_stm = ipc_mdev_is_stm,
	.clr_ack = ipc_mdev_clr_ack,
	.ensure_channel = ipc_mdev_ensure_channel,
	.status = ipc_mdev_dump_status,
	.dump_regs = ipc_mdev_dump_regs,
	.show_mdev_info = ipc_mdev_show_mdev_info
};

static void ipc_mdev_put(struct ipc_device *idev)
{
	struct hisi_mbox_device **list = idev->mdev_res;
	struct hisi_mbox_device *mdev = NULL;
	int i = 0;

	iounmap(idev->base);

	kfree(idev->cmbox_info);
	kfree(idev->buf_pool);
	mdev = list[0];
	while (mdev) {
		i++;
		kfree(mdev->priv);
		kfree(mdev);
		mdev = list[i];
	}
}

static int ipc_mdev_remove(struct platform_device *pdev)
{
	struct ipc_device *idev = platform_get_drvdata(pdev);

	platform_set_drvdata(pdev, NULL);

	if (idev) {
		ipc_mbox_device_unregister(idev->mdev_res);
		ipc_mdev_put(idev);
		kfree(idev);
	}

	return 0;
}

static struct ipc_common_mbox_info *mdev_get_cmbox_info(
	struct device_node *node)
{
	int cm_gic_1_irq;
	int cm_gic_2_irq;
	struct ipc_common_mbox_info *cmbox_info = NULL;

	cmbox_info = kzalloc(sizeof(*cmbox_info), GFP_KERNEL);
	if (!cmbox_info)
		return NULL;

	cm_gic_1_irq = irq_of_parse_and_map(node, 0);
	cm_gic_2_irq = irq_of_parse_and_map(node, 1);
	cmbox_info->gic_1_irq_requested = 0;
	cmbox_info->gic_2_irq_requested = 0;
	cmbox_info->cmbox_gic_1_irq = cm_gic_1_irq;
	cmbox_info->cmbox_gic_2_irq = cm_gic_2_irq;
	cmbox_info->cmdev = NULL;

	return cmbox_info;
}

static int mdev_get_rproc_name(struct device_node *node,
	struct ipc_device *idev)
{
	int ret;
	const char *ipc_rproc_name[RPROC_NUMBER] = {0};
	unsigned int rproc_num = 0;
	unsigned int i;

	ret = of_property_read_u32(node, "rproc_num", &rproc_num);
	if (ret) {
		mdev_err("prop \"rproc_num\" error %d", ret);
		return -ENODEV;
	}

	for (i = 0; i < RPROC_NUMBER; i++) {
		if (i < rproc_num) {
			ret = of_property_read_string_index(
				node, "rproc_name", i, &ipc_rproc_name[i]);
			if (ret < 0) {
				mdev_err("prop rproc_name error %d", ret);
				return -ENODEV;
			}
		} else {
			ipc_rproc_name[i] = "ERR_RPROC";
		}
		mdev_debug("%s rproc_name is %s", __func__, ipc_rproc_name[i]);
	}

	for (i = 0; i < RPROC_NUMBER; i++)
		idev->rproc_name[i] = ipc_rproc_name[i];

	return 0;
}

static void mdev_idev_info_print(struct ipc_device *idev,
	unsigned int mdev_num)
{
	mdev_debug("ipc_base: 0x%lx", (unsigned long)(uintptr_t)idev->base);
	mdev_debug("capability: %u", idev->capability);
	mdev_debug("unlock_key: 0x%x", idev->unlock);
	mdev_debug("reg_spc: %u", idev->reg_spc);
	mdev_debug("mailboxes: %u", mdev_num);
	mdev_debug("%s ipc type[%u] rproc_src_id is [%u] [%u]",
		__func__, idev->ipc_type,
		idev->rproc_src_id[0], idev->rproc_src_id[1]);
	mdev_debug("buffer pool: 0x%lx",
		(unsigned long)(uintptr_t)idev->buf_pool);
}

static int mdev_idev_basic_info_get(struct ipc_device *idev,
	struct device_node *node)
{
	int ret;

	ret = of_property_read_u32(node, "capability", &(idev->capability));
	if (ret) {
		mdev_err("prop \"capability\" error %d", ret);
		return ret;
	}

	ret = of_property_read_u32(node, "unlock_key", &(idev->unlock));
	if (ret) {
		mdev_err("prop \"key\" error %d", ret);
		return ret;
	}

	/* some mbox have 2 src */
	ret = of_property_read_u32_array(
		node, "rproc_src_id", idev->rproc_src_id, IPC_MAX_SRC_ID);
	if (ret) {
		mdev_err("prop \"rproc_src_id\" error %d", ret);
		return ret;
	}

	ret = of_property_read_u32(node, "ipc_type", &(idev->ipc_type));
	if (ret) {
		mdev_err("prop \"ipc_type\" error %d", ret);
		return ret;
	}

	ret = of_property_read_u32(node, "ipc_version", &(idev->ipc_version));
	if (ret) {
		mdev_err("prop \"ipc_version\" error %d", ret);
		idev->ipc_version = IPC_VERSION_APPLE;
	}

	return 0;
}

static int mdev_idev_init(struct ipc_device *idev,
	struct device_node *node, struct hisi_mbox_device **mdevs)
{
	mbox_msg_t *buf_pool = NULL;
	void __iomem *sys_time_reg_addr = NULL;
	mbox_msg_len_t buf_pool_len;
	unsigned int mdev_num;
	int ret;

	idev->base = of_iomap(node, 0);
	if (!idev->base) {
		mdev_err("iomap error");
		goto out;
	}

	sys_time_reg_addr = of_iomap(node, 1);
	if (!sys_time_reg_addr)
		idev->system_time_reg_addr = NULL;
	else
		idev->system_time_reg_addr = sys_time_reg_addr +
			SCTRL_TIME_ADDR;

	ret = mdev_idev_basic_info_get(idev, node);
	if (ret)
		goto to_iounmap;

	idev->reg_spc = (idev->ipc_version == IPC_VERSION_BOY) ?
		IPC_REG_SPACE_EX : IPC_REG_SPACE;

	ret = mdev_get_rproc_name(node, idev);
	if (ret)
		goto to_iounmap;

	idev->cmbox_info = mdev_get_cmbox_info(node);
	if (!idev->cmbox_info)
		goto free_cmbox;

	ret = of_property_read_u32(node, "mailboxes", &mdev_num);
	if (ret) {
		mdev_err("prop \"mailboxes\" error %d", ret);
		goto free_cmbox;
	}

	buf_pool_len = idev->capability * MBOX_BUFFER_TYPE_MAX * mdev_num;
	buf_pool = kcalloc(buf_pool_len, sizeof(*buf_pool), GFP_KERNEL);
	if (!buf_pool)
		goto free_cmbox;

	idev->mdev_res = mdevs;
	idev->buf_pool = buf_pool;

	mdev_idev_info_print(idev, mdev_num);

	return 0;

free_cmbox:
	kfree(idev->cmbox_info);
to_iounmap:
	if (sys_time_reg_addr)
		iounmap(sys_time_reg_addr);
	iounmap(idev->base);
out:
	return -ENODEV;
}

static void mdev_idev_free(struct ipc_device *idev)
{
	kfree(idev->buf_pool);
	kfree(idev->cmbox_info);
	iounmap(idev->base);
}

static int mdev_priv_get_irq(struct ipc_device *idev,
	struct ipc_mbox_device_priv *priv, struct device_node *son,
	struct hisi_mbox_device *mdev)
{
	unsigned int output[IPC_FUNC] = {0};
	int ret;

	ret = of_property_read_u32_array(son, "func", output, IPC_FUNC);
	if (ret)
		return -ENODEV;

	priv->func |= (output[0] ? FAST_MBOX : COMM_MBOX); /* mbox_type */

	priv->func |= (output[1] ? SOURCE_MBOX : 0); /* is_src_mbox */

	priv->func |= (output[2] ? DESTINATION_MBOX : 0); /* is_des_mbox */

	if ((FAST_MBOX & priv->func) && (DESTINATION_MBOX & priv->func)) {
		mdev_debug("func FAST DES MBOX");
		priv->irq = irq_of_parse_and_map(son, 0);
	} else if ((FAST_MBOX & priv->func) && (SOURCE_MBOX & priv->func)) {
		mdev_debug("func FAST SRC MBOX");
		priv->irq = (priv->src == GIC_1) ?
			idev->cmbox_info->cmbox_gic_1_irq :
			idev->cmbox_info->cmbox_gic_2_irq;
		/*
		 * set the cmdev, the cmdev will be used in acore't
		 * interrupts
		 */
		if (!idev->cmbox_info->cmdev)
			idev->cmbox_info->cmdev = mdev;
	} else {
		/* maybe GIC_1 OR GIC_2 */
		mdev_debug("we don't use comm-mailbox, we use it as fast-mailbox");
		/*
		 * we don't use comm-mailbox ,we use it as fast-mailbox,
		 * please set the comm to fast in the dtsi
		 */
		priv->irq = COMM_MBOX_IRQ;
		idev->cmbox_info->cmdev = mdev;
	}
	mdev_debug("irq: %d", priv->irq);

	return 0;
}

static void mdev_priv_info_print(struct ipc_mbox_device_priv *priv)
{
	mdev_debug("src_bit: %d", priv->src);
	mdev_debug("des_bit: %d", priv->des);
	mdev_debug("index: %d", priv->mbox_channel);
	mdev_debug("timeout: %u", priv->timeout);
	mdev_debug("fifo_size: %u", priv->fifo_size);
	mdev_debug("sched_priority: %u", priv->sched_priority);
	mdev_debug("sched_policy: %u", priv->sched_policy);
}

static int of_property_rproc_id(struct device_node *son,
	struct ipc_mbox_device_priv *priv)
{
	const char *rproc_id_name = NULL;
	int ret;

	ret = of_property_read_string(son, "rproc", &rproc_id_name);
	if (ret || !rproc_id_name) {
		mdev_err("Miss 'rproc' property");
		return -ENODEV;
	}

	priv->rproc_id = ipc_find_rproc_id(rproc_id_name);
	if (priv->rproc_id < 0) {
		mdev_err("Invalid rproc:[%s] property", rproc_id_name);
		return -ENODEV;
	}

	return 0;
}

static struct ipc_mbox_device_priv *mdev_priv_init(struct device_node *son,
	struct ipc_device *idev, struct hisi_mbox_device *mdev,
	unsigned int used)
{
	int ret;
	struct ipc_mbox_device_priv *priv = NULL;

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return NULL;

	ret = of_property_read_u32(son, "src_bit", (u32 *)&(priv->src));
	if (ret)
		goto free_priv;

	ret = of_property_read_u32(son, "des_bit", (u32 *)&(priv->des));
	if (ret)
		goto free_priv;

	/*
	 * get software code-index to mbox_channel and calculate the
	 * right mbox_channel
	 */
	ret = of_property_read_u32(son, "index", &(priv->mbox_channel));
	if (ret)
		goto free_priv;

	/*
	 * to distinguish different ipc and calculate the true
	 * mailbox-index(100)
	 */
	if (priv->mbox_channel > MAX_AP_IPC_INDEX)
		priv->mbox_channel = priv->mbox_channel % IPC_RESIDUE;

	if (of_property_rproc_id(son, priv))
		priv->rproc_id = INVALID_RPROC_ID;

	ret = of_property_read_u32(son, "timeout", &(priv->timeout));
	if (ret || (priv->timeout % EVERY_LOOP_TIME_MS != 0))
		priv->timeout = DEFAULT_MAILBOX_TIMEOUT;

	ret = of_property_read_u32(son, "fifo_size", &(priv->fifo_size));
	if (ret)
		priv->fifo_size = DEFAULT_FIFO_SIZE;

	ret = of_property_read_u32(son, "sched_priority",
		&(priv->sched_priority));
	if (ret)
		priv->sched_priority = DEFAULT_SCHED_PRIORITY;

	ret = of_property_read_u32(son, "sched_policy", &(priv->sched_policy));
	if (ret)
		priv->sched_policy = SCHED_RR; /* default is SCHED_RR */

	ret = mdev_priv_get_irq(idev, priv, son, mdev);
	if (ret)
		goto free_priv;

	priv->capability = idev->capability;
	priv->idev = idev;
	priv->used = used;
	mdev_priv_info_print(priv);
	return priv;

free_priv:
	kfree(priv);
	return NULL;
}

static int ipc_init_one_mdev(struct hisi_mbox_device **mdev_list,
	struct device_node *son, struct ipc_device *idev,
	int *mdev_idx_temp)
{
	int ret;
	mbox_msg_t *rx_buffer = NULL;
	mbox_msg_t *ack_buffer = NULL;
	struct ipc_mbox_device_priv *priv = NULL;
	unsigned int used = 0;
	struct hisi_mbox_device *mdev = NULL;

	ret = of_property_read_u32(son, "used", &used);
	if (ret) {
		mdev_err("mailbox-%d has no tag <used>", *mdev_idx_temp);
		return ret;
	}

	if (used == MAILBOX_NO_USED) {
		mdev_debug("mailbox node %s is not used", son->name);
		return 0;
	}

	mdev = kzalloc(sizeof(struct hisi_mbox_device), GFP_KERNEL);
	if (!mdev)
		return -ENOMEM;

	priv = mdev_priv_init(son, idev, mdev, used);
	if (!priv) {
		kfree(mdev);
		return -ENOMEM;
	}

	mdev_debug("mailbox node: %s", son->name);

	rx_buffer = idev->buf_pool + idev->capability * RX_BUFFER_TYPE;
	ack_buffer = idev->buf_pool + idev->capability * ACK_BUFFER_TYPE;
	idev->buf_pool = idev->buf_pool +
		idev->capability * MBOX_BUFFER_TYPE_MAX;
	mdev_debug("rx_buffer: 0x%pK\nack_buffer: 0x%pK",
		rx_buffer, ack_buffer);

	mdev->name = son->name;
	mdev->priv = priv;
	mdev->rx_buffer = rx_buffer;
	mdev->ack_buffer = ack_buffer;
	mdev->ops = &ipc_mdev_ops;

	mdev_list[*mdev_idx_temp] = mdev;
	*mdev_idx_temp = (*mdev_idx_temp) + 1;

	return 0;
}

static int ipc_mdev_get(struct ipc_device *idev,
	struct hisi_mbox_device **mdevs, int mdev_num, struct device_node *node)
{
	struct device_node *son = NULL;
	int mdev_idx_temp, ret;
	unsigned int i;

	ret = mdev_idev_init(idev, node, mdevs);
	if (ret)
		return ret;

	for (i = 0, mdev_idx_temp = 0; (son = of_get_next_child(node, son)) &&
		mdev_idx_temp <  mdev_num && i < MAX_MBOX_NODE; i++) {
		ret = ipc_init_one_mdev(mdevs, son, idev, &mdev_idx_temp);
		if (ret)
			break;
	}

	if (!ret)
		return ret;

	while (i--) {
		kfree(mdevs[i]->priv);
		kfree(mdevs[i]);
	}
	mdev_idev_free(idev);

	return ret;
}

static int ipc_mdev_probe(struct platform_device *pdev)
{
	struct ipc_device *idev = NULL;
	struct hisi_mbox_device **mdev_res = NULL;
	struct device_node *node = pdev->dev.of_node;
	int mdev_num;
	int ret;

	if (!node) {
		mdev_err("dts node not found");
		ret = -ENODEV;
		goto out;
	}

	idev = kzalloc(sizeof(*idev), GFP_KERNEL);
	if (!idev) {
		ret = -ENOMEM;
		goto out;
	}

	ret = of_property_read_u32(node, "mailboxes", (u32 *)&mdev_num);
	if (ret) {
		mdev_err("no mailboxes resources");
		ret = -ENODEV;
		goto free_idev;
	}

	mdev_res = kzalloc((sizeof(*mdev_res) * (mdev_num + 1)), GFP_KERNEL);
	if (!mdev_res) {
		ret = -ENOMEM;
		goto free_idev;
	}
	mdev_res[mdev_num] = NULL;

	ret = ipc_mdev_get(idev, mdev_res, mdev_num, node);
	if (ret) {
		mdev_err("can not get ipc resource");
		ret = -ENODEV;
		goto free_mdevs;
	}

	ret = ipc_mbox_device_register(&pdev->dev, mdev_res);
	if (ret) {
		mdev_err("mdevs register failed");
		ret = -ENODEV;
		goto put_res;
	}

	platform_set_drvdata(pdev, idev);

	mdev_debug("HiIPCV230 mailboxes are ready");
	/*
	 * We call it here to let the module which depend IPC can use the
	 * rproc send function
	 */
	ipc_rproc_init(mdev_res);

	return 0;

put_res:
	ipc_mdev_put(idev);
free_mdevs:
	kfree(idev->mdev_res);
	idev->mdev_res = NULL;
free_idev:
	kfree(idev);
out:
	return ret;
}

static int ipc_mdev_suspend(struct device *dev)
{
	struct platform_device *pdev =
		container_of(dev, struct platform_device, dev);
	struct ipc_device *idev = platform_get_drvdata(pdev);

	mdev_info("%s: suspend +", __func__);
	if (idev)
		ipc_mbox_device_deactivate(idev->mdev_res);
	mdev_info("%s: suspend -", __func__);
	return 0;
}

static int ipc_mdev_resume(struct device *dev)
{
	struct platform_device *pdev =
		container_of(dev, struct platform_device, dev);
	struct ipc_device *idev = platform_get_drvdata(pdev);

	mdev_info("%s: resume +", __func__);
	if (idev)
		ipc_mbox_device_activate(idev->mdev_res);
	mdev_info("%s: resume -", __func__);
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id ipc_mdev_of_match[] = {
	{
		.compatible = "hisilicon,HiIPCV230",
	},
	{},
};

MODULE_DEVICE_TABLE(of, ipc_mdev_of_match);
#endif

static const struct dev_pm_ops ipc_mdev_pm_ops = {
	.suspend_late = ipc_mdev_suspend,
	.resume_early = ipc_mdev_resume,
};

static struct platform_driver ipc_mdev_driver = {
	.probe = ipc_mdev_probe,
	.remove = ipc_mdev_remove,
	.driver = {
		.name = "HiIPCV230-mailbox",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(ipc_mdev_of_match),
		.pm = &ipc_mdev_pm_ops,
	},
};

static int __init ipc_mdev_init(void)
{
	int ret;

	mdev_err("%s init!", __func__);

	ret = ipc_check_rproc_id_cfg();
	if (ret)
		mdev_err("%d rproc_name not be configured", ret);

	platform_driver_register(&ipc_mdev_driver);
	return 0;
}

core_initcall(ipc_mdev_init);

static void __exit ipc_mdev_exit(void)
{
	platform_driver_unregister(&ipc_mdev_driver);
}

module_exit(ipc_mdev_exit);

MODULE_DESCRIPTION("HiIPCV230 ipc, mailbox device driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
