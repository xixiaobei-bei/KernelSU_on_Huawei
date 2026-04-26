/*
 * Hisilicon IPP Common Driver
 *
 * Copyright (c) 2018 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*lint -e570 -e578
-esym(570,*)*/

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/genalloc.h>
#include <asm/uaccess.h>
#include <linux/hisi-iommu.h>
#include <linux/hisi/hipp.h>
#include <linux/ion.h>
#include <linux/hisi/hisi_ion.h>
#include "ipp.h"
#include <linux/dma-buf.h>
#include <linux/version.h>
#include <linux/syscalls.h>
#include <linux/scatterlist.h>

#define DTSNAME_HIPPCOM    "hisilicon,ippcom"
#define TRUSTED_TIMEOUT_MS  (10000)
#define MEM_SEC_PTABLE_SIZE (0x00300000)/*  the sec boot size : 3M   */
#define MEM_IPP_ERR_ADDR_OFFSET (0x0)

#define SMMU_NS_INTSTAT  (0x18)
#define SMMU_NS_INTCLR   (0x1C)
#define IPPSMMU_EXC_TYPE_NUM  6

struct hipp_sec_ion_s {
	u64 sec_phymem_addr;
	struct ion_handle *ion_handle;
	struct ion_client *ion_client;
};

struct map_sglist_s {
	unsigned long long addr;
	unsigned int size;
};

struct hipp_comdevice_s {
	struct platform_device *pdev;                   /* com platform device */
	int initialized;                                /* com Device Probe */
	unsigned int irq;                               /* Smmu IRQ */
	struct resource *r;                             /* Smmu Resource from DTS */
	void __iomem *smmubase;                         /* Smmu Base Address */
	struct iommu_domain *domain;                    /* Iommu Domain */
	unsigned int drvnum;                            /* */
	const char *drvname[MAX_IPP_COM];               /* */
	struct clk *jpegclk;                            /* */
	const char *clkname;                            /* */
	unsigned long clock[MAX_CLOCK_LEVEL];           /* */
	unsigned long jpeg_clock[MAX_IPP_COM];
	unsigned long last_clock;                       /* */
	unsigned int clock_busy;                        /* */
	struct mutex mutex_dev;                         /* Mutex for Device */
	struct mutex mutex_smmu;                        /* Mutex for Smmu Enable/Disable */
	struct mutex mutex_usecase;                     /* Mutex for Use Case */
	struct mutex mutex_clock;                       /* Mutex for Clock Enable/Disable */
	struct hipp_common_s *drv[MAX_IPP_COM];         /* com Driver Arrays */
	unsigned int refs_smmu[TRUSTED + 1];            /* Voting notrus/trusted */
	unsigned int refs_lock[TRUSTED + 1];            /* Voting Usecase Lock/Unlock */
	unsigned int refs_clck;                         /* Voting Jpeg Clock Enable/Disable */
	struct task_struct *kthread;                    /* Hipp Smmu Mntn Kernel Thread */
	wait_queue_head_t wait_trusted;                 /* Wait Point for Hipp Lock */
	wait_queue_head_t wait_mntn;                    /* Wait Point for Hipp Mntn */
	wait_queue_head_t wait_clken;                   /* Wait Point for Clock Enable */
	wait_queue_head_t wait_clkdi;                   /* Wait Point for Clock Disable */
	int wake_mntn;                                  /* Wake Up Condition */
	struct hipp_commntn_s mntn_smmu;                /* Device Mntn Smmu */
	unsigned int fault_count[TRUSTED + 1];          /* Fault Count notrus/trusted */
	unsigned long long fault_ts;                    /* Driver TimeStamp for Fault Last Time */
	void *smem_vaddr;
	dma_addr_t smem_dma;
	size_t smem_size;
};

static struct hipp_comdevice_s *ippcomdev = NULL;

static int invalid_secmode(unsigned int mode)
{
	if (mode >= MAX_SECMODE) {
		e("mode.(0 < %d < %d)\n", mode, MAX_SECMODE);
		return -1;
	}

	return 0;
}

static int invalid_type(int type)
{
	if (type < 0 || type >= MAX_IPP_COM) {
		e("Failed : Invalid Type.%d\n", type);
		return -EINVAL;
	}

	return 0;
}

static struct hipp_comdevice_s *hipp_drv2dev(struct hipp_common_s *drv)
{
	struct hipp_comdevice_s *dev = NULL;
	int ret = 0;

	if (drv == NULL) {
		e("Failed : drv.%pK\n", drv);
		return NULL;
	}

	if ((ret = invalid_type(drv->type))) {
		e("Failed : invalid_type.%d\n", ret);
		return NULL;
	}

	if ((ret = invalid_secmode(drv->mode))) {
		e("Failed : invalid_secmode.%d\n", ret);
		return NULL;
	}

	if ((dev = (struct hipp_comdevice_s *)drv->comdev) == NULL) {
		e("Failed : dev.%pK\n", dev);
		return NULL;
	}

	if (drv != dev->drv[drv->type]) {
		e("Failed : Mismatch drv.(%pK, %pK), type.%d\n", drv, dev->drv[drv->type], drv->type);
		return NULL;
	}

	return dev;
}

extern int hispcpe_smmu_nontrus_init(void);

int do_ippsmmu_enable(unsigned int mode)
{
	int ret = 0;

	ret = hispcpe_smmu_nontrus_init();
	if (ret != 0) {
		pr_err("%s Failed :smmu_nontrus.%d\n", __func__, ret);
		return ret;
	}

	ret = atfhipp_smmu_enable(mode);
	if (ret != 0) {
		pr_err("%s Failed :atfhipp_smmu_enable.%d\n", __func__, ret);
		return ret;
	}

	return 0;
}

int do_ippsmmu_disable(unsigned int mode)
{
	int ret;

	if (mode == NONTRUS)
		return 0;

	ret = atfhipp_smmu_disable();
	if (ret != 0)
		pr_err("%s Failed : ret.%d\n", __func__, ret);

	return ret;
}

static int do_ippdump(struct hipp_comdevice_s *dev)
{
	struct hipp_common_s *drv = NULL;
	int i = 0;

	if (dev == NULL) {
		e("Failed : dev.%pK\n", dev);
		return -ENOMEM;
	}

	i("Device: refs_smmu.(0x%x, 0x%x), fault_count.(0x%x, 0x%x), initialized.0x%x\n",
	  dev->refs_smmu[NONTRUS], dev->refs_smmu[TRUSTED], dev->fault_count[NONTRUS], dev->fault_count[TRUSTED],
	  dev->initialized);

	for (i = 0; i < MAX_IPP_COM; i ++) {
		drv = dev->drv[i];

		if (drv == NULL) {
			e("%d Not Registered\n", i);
			continue;
		}

		i("Driver.%d: %s.type.0x%x, initialized.0x%x, mode.0x%x, count.(e.0x%x, d.0x%x), timestamp.(f.[%5u.%06u], l.[%5u.%06u], fault.[%5u.%06u])\n",
		  i,
		  drv->name, drv->type, drv->initialized, drv->mode, drv->mntn_smmu.icount, drv->mntn_smmu.xcount,
		  do_div(drv->mntn_smmu.ifirst_ts, 1000000000), do_div(drv->mntn_smmu.ifirst_ts, 1000),
		  do_div(drv->mntn_smmu.ilast_ts, 1000000000), do_div(drv->mntn_smmu.xlast_ts, 1000),
		  do_div(dev->fault_ts, 1000000000), do_div(dev->fault_ts, 1000));
	}

	return 0;
}

static int hippsmmu_enable(struct hipp_common_s *drv)
{
	struct hipp_comdevice_s *dev = NULL;
	int ret = 0;

	if ((dev = hipp_drv2dev(drv)) == NULL) {
		e("Failed : hipp_drv2dev.(%pK).%pK\n", drv, dev);
		return -ENOMEM;
	}

	pr_info("%s: drv name.%s\n", __func__, drv->name);
	mutex_lock(&dev->mutex_smmu);

	if (dev->refs_smmu[drv->mode] ++)
		i("FF refs_smmu mode.%d, num .%d\n", drv->mode, dev->refs_smmu[drv->mode]);
	else
		i("refs_smmu mode.%d, num .%d\n", drv->mode, dev->refs_smmu[drv->mode]);

	ret = do_ippsmmu_enable(drv->mode);
	if (ret != 0) {
		e("Failed : %s.do_ippsmmu_enable.%d\n", drv->name, ret);
		dev->fault_count[drv->mode] ++;
		dev->refs_smmu[drv->mode] --;
		e("Failed : %s.refs_smmu[%u] -> %u\n", drv->name, drv->mode, dev->refs_smmu[drv->mode]);
		mutex_unlock(&dev->mutex_smmu);
		drv->dump(drv);
		return ret;
	}

	mutex_unlock(&dev->mutex_smmu);
	return 0;
}

static int hippsmmu_disable(struct hipp_common_s *drv)
{
	struct hipp_comdevice_s *dev = NULL;
	int ret = 0;

	if ((dev = hipp_drv2dev(drv)) == NULL) {
		pr_err("%s Failed : hipp_drv2dev.NULL\n", __func__);
		return -ENOMEM;
	}

	pr_info("%s: drv name.%s\n", __func__, drv->name);

	mutex_lock(&dev->mutex_smmu);

	if (!dev->refs_smmu[drv->mode]) {
		pr_err("%s Failed : refs_smmu[%u].%d\n",
			__func__, drv->mode, dev->refs_smmu[drv->mode]);
		mutex_unlock(&dev->mutex_smmu);
		return -EINVAL;
	}

	if (-- dev->refs_smmu[drv->mode]) {
		pr_info("%s: refs_smmu.%d, num.%d\n",
			__func__, drv->mode, dev->refs_smmu[drv->mode]);
		mutex_unlock(&dev->mutex_smmu);
		return 0;
	}

	pr_info("%s: refs_smmu mode.%d, num.%d\n",
		__func__, drv->mode, dev->refs_smmu[drv->mode]);

	ret = do_ippsmmu_disable(drv->mode);
	if (ret != 0) {
		dev->fault_count[drv->mode] ++;
		dev->refs_smmu[drv->mode] ++;
		pr_err("%s Failed : refs_smmu[%u] -> %u\n",
			__func__, drv->mode, dev->refs_smmu[drv->mode]);
		mutex_unlock(&dev->mutex_smmu);
		return ret;
	}

	mutex_unlock(&dev->mutex_smmu);
	return 0;
}

static int hippsmmu_setsid(struct hipp_common_s *drv, unsigned int sid, unsigned int mode)
{
	struct hipp_comdevice_s *dev = NULL;
	int ret = 0;

	if ((dev = hipp_drv2dev(drv)) == NULL) {
		e("Failed : hipp_drv2dev.(%pK).%pK\n", drv, dev);
		return -ENOMEM;
	}

	i("+ %s\n", drv->name);
	mutex_lock(&dev->mutex_smmu);

	if ((ret = atfhipp_smmu_smrx(sid, mode)) != 0) {
		e("Failed : %s.atfhipp_smmu_smrx.%d\n", drv->name, ret);
		dev->fault_count[drv->mode] ++;
		dev->fault_ts = hisi_getcurtime();
		mutex_unlock(&dev->mutex_smmu);
		drv->dump(drv);
		return ret;
	}

	mutex_unlock(&dev->mutex_smmu);
	i("- %s\n", drv->name);
	return 0;
}

static int hipp_dump(struct hipp_common_s *drv)
{
	struct hipp_comdevice_s *dev = NULL;

	if ((dev = hipp_drv2dev(drv)) == NULL) {
		e("Failed : hipp_drv2dev.(%pK).%pK\n", drv, dev);
		return -ENOMEM;
	}

	i("+ %s\n", drv->name);
	mutex_lock(&dev->mutex_smmu);
	do_ippdump(dev);
	mutex_unlock(&dev->mutex_smmu);
	i("- %s\n", drv->name);
	return 0;
}

static int hippusecase_lock(struct hipp_common_s *drv)
{
	struct hipp_comdevice_s *dev = NULL;
	unsigned long timeout = 0;
	int ret = 0;

	if ((dev = hipp_drv2dev(drv)) == NULL) {
		e("Failed : hipp_drv2dev.(%pK).%pK\n", drv, dev);
		return -ENOMEM;
	}

	d("+ %s\n", drv->name);
	mutex_lock(&dev->mutex_clock);

	if (dev->clock_busy) {
		e("Failed : clock_busy.%d\n", dev->clock_busy);
		mutex_unlock(&dev->mutex_clock);
		return -EBUSY;
	}

	mutex_unlock(&dev->mutex_clock);
	mutex_lock(&dev->mutex_usecase);

	if (dev->refs_lock[TRUSTED]) {
		e("Failed : %s.refs_clck[%d].%u, Try Again...\n", drv->name, TRUSTED, dev->refs_lock[TRUSTED]);
		mutex_unlock(&dev->mutex_usecase);
		return -EAGAIN;
	}

	dev->refs_lock[drv->mode] ++;

	if (dev->refs_lock[TRUSTED]) {
		if (dev->refs_lock[NONTRUS]) {
			timeout = msecs_to_jiffies(TRUSTED_TIMEOUT_MS);
			mutex_unlock(&dev->mutex_usecase);

			if (!(ret = wait_event_timeout(dev->wait_trusted, dev->refs_lock[NONTRUS] == 0, timeout))) {
				e("Failed : %s.wait_event_timeout.(0x%x, 0x%lx).0x%x\n", drv->name, dev->refs_lock[NONTRUS] == 0, timeout, ret);
				return -ETIMEDOUT;
			}

			mutex_lock(&dev->mutex_usecase);
		}
	}

	mutex_unlock(&dev->mutex_usecase);
	i("- %s\n", drv->name);
	return 0;
}

static int hippusecase_unlock(struct hipp_common_s *drv)
{
	struct hipp_comdevice_s *dev = NULL;

	if ((dev = hipp_drv2dev(drv)) == NULL) {
		e("Failed : hipp_drv2dev.(%pK).%pK\n", drv, dev);
		return -ENOMEM;
	}

	d("+ %s\n", drv->name);
	mutex_lock(&dev->mutex_usecase);

	if (!dev->refs_lock[drv->mode]) {
		e("Failed : %s.UnderFlow... refs_lock.0x%x @ %u\n", drv->name, dev->refs_lock[drv->mode], drv->mode);
		mutex_unlock(&dev->mutex_usecase);
		return -EINVAL;
	}

	dev->refs_lock[drv->mode] --;

	if (dev->refs_lock[TRUSTED] && !dev->refs_lock[NONTRUS])
		wake_up(&dev->wait_trusted);

	if (!dev->refs_lock[TRUSTED] && !dev->refs_lock[NONTRUS]) {
		wake_up(&dev->wait_clken);
		wake_up(&dev->wait_clkdi);
	}

	mutex_unlock(&dev->mutex_usecase);
	i("- %s\n", drv->name);
	return 0;
}

static unsigned long hippjpgclk_max(struct hipp_comdevice_s *dev)
{
	unsigned long max;
	int i = 0;

	if (dev == NULL) {
		e("Failed : dev is NULL\n");
		return 0;
	}

	max = dev->jpeg_clock[0];

	for (i = 1; i < JPEGD_UNIT + 1; i++) {
		if (max < dev->jpeg_clock[i])
			max = dev->jpeg_clock[i];
	}

	return max;
}

static int hippjpgclk_enable(struct hipp_common_s *drv, unsigned int dvfs)
{
	struct hipp_comdevice_s *dev = NULL;
	unsigned long value = 0;
	int ret = 0;
	unsigned long max_clk = 0;

	if ((dev = hipp_drv2dev(drv)) == NULL) {
		e("Failed : hipp_drv2dev.(%pK).%pK\n", drv, dev);
		return -ENOMEM;
	}

	i("+ %s, refs_clck.%u, clk_dvfs.%d, refs_lock(N.0x%x, T.0x%x)\n", drv->name, dev->refs_clck, dvfs,
	  dev->refs_lock[NONTRUS], dev->refs_lock[TRUSTED]);

	if (dvfs > CLKLVL_SVS) {
		e("Failed : drv->dvfs.%d\n", dvfs);
		return -EINVAL;
	}

	if (drv->type > JPEGD_UNIT) {
		e("Failed : drv->type.%d\n", drv->type);
		return -EINVAL;
	}

	mutex_lock(&dev->mutex_clock);
	value = dev->clock[dvfs];
	max_clk = hippjpgclk_max(dev);

	if (value > max_clk) {
		dev->clock_busy = 1;

		if ((ret = clk_set_rate(dev->jpegclk, value)) < 0) {
			e("Failed: %s.(max_clk.%d.%d M, value.%d.%d M), clk_set_rate.%d\n", drv->name,
			  (int)max_clk / 1000000, (int)max_clk % 1000000, (int)value / 1000000, (int)value % 1000000, ret);
			dev->clock_busy = 0;
			mutex_unlock(&dev->mutex_clock);
			return ret;
		}

		dev->last_clock = value;
		dev->clock_busy = 0;
	}

	dev->jpeg_clock[drv->type] = value;

	if ((ret = clk_prepare_enable(dev->jpegclk)) < 0) {
		e("Failed: %s.(max_clk.%d.%d M, value.%d.%d M), clk_prepare_enable.%d\n", drv->name,
		  (int)max_clk / 1000000, (int)max_clk % 1000000, (int)value / 1000000, (int)value % 1000000, ret);
		mutex_unlock(&dev->mutex_clock);
		return ret;
	}

	dev->refs_clck ++;
	i("- %s.clk_set_rate.[%s].(max_clk.%d.%d M, value.%d.%d M), refs_clck.%u, refs_lock(N.0x%x, T.0x%x)\n", drv->name,
	  ((value > max_clk) ? "Run" : "Bypass"),
	  (int)max_clk / 1000000, (int)max_clk % 1000000, (int)value / 1000000, (int)value % 1000000, dev->refs_clck,
	  dev->refs_lock[NONTRUS], dev->refs_lock[TRUSTED]);
	mutex_unlock(&dev->mutex_clock);
	return 0;
}

static int hippjpgclk_dvfs(struct hipp_common_s *drv, unsigned int dvfs)
{
	struct hipp_comdevice_s *dev = NULL;
	unsigned long value = 0;
	int ret = 0;
	unsigned long max_clk   = 0;
	unsigned long value_buf = 0;

	if ((dev = hipp_drv2dev(drv)) == NULL) {
		e("Failed : hipp_drv2dev.(%pK).%pK\n", drv, dev);
		return -ENOMEM;
	}

	i("+ %s, clk_dvfs.%d\n", drv->name, dvfs);

	if (dvfs > CLKLVL_SVS) {
		e("Failed : drv->dvfs.%d\n", dvfs);
		return -EINVAL;
	}

	if (drv->type > JPEGD_UNIT) {
		e("Failed : drv->type.%d\n", drv->type);
		return -EINVAL;
	}

	mutex_lock(&dev->mutex_clock);
	i("+ %s, refs_clck.%u\n", drv->name, dev->refs_clck);

	if (!dev->refs_clck) {
		e("Failed : %s.UnderFlow... refs_clck.%u, \n", drv->name, dev->refs_clck);
		mutex_unlock(&dev->mutex_clock);
		return -EINVAL;
	}

	value = dev->clock[dvfs];
	value_buf = dev->jpeg_clock[drv->type];
	dev->jpeg_clock[drv->type] = value;
	max_clk = hippjpgclk_max(dev);

	if (max_clk == dev->last_clock) {
		i("- %s.max_clk.%d.%d M, value.%d.%d M\n",
		  drv->name, (int)max_clk / 1000000, (int)max_clk % 1000000, (int)value / 1000000, (int)value % 1000000);
		mutex_unlock(&dev->mutex_clock);
		return 0;
	}

	dev->clock_busy = 1;

	if ((ret = clk_set_rate(dev->jpegclk, max_clk)) < 0) {
		e("Failed: %s.%d.%d M, clk_set_rate.%d\n", drv->name, (int)max_clk / 1000000, (int)max_clk % 1000000, ret);
		dev->clock_busy = 0;
		dev->jpeg_clock[drv->type] = value_buf;
		mutex_unlock(&dev->mutex_clock);
		return ret;
	}

	dev->last_clock = max_clk;
	dev->clock_busy = 0;
	i("- %s.clk_rate.%d.%d M, refs_clck.%u\n", drv->name, (int)max_clk / 1000000, (int)max_clk % 1000000, dev->refs_clck);
	mutex_unlock(&dev->mutex_clock);
	return 0;
}

static int hippjpgclk_disable(struct hipp_common_s *drv, unsigned int dvfs)
{
	struct hipp_comdevice_s *dev = NULL;
	unsigned long value = 0;
	int ret = 0;
	unsigned long max_clk = 0;

	if ((dev = hipp_drv2dev(drv)) == NULL) {
		e("Failed : hipp_drv2dev.(%pK).%pK\n", drv, dev);
		return -ENOMEM;
	}

	i("+ %s, clk_dvfs.%d\n", drv->name, dvfs);

	if (dvfs != CLKLVL_DISCLOCK) {
		e("Failed : drv->dvfs.%d is not CLKLVL_DISCLOCK\n", dvfs);
		return -EINVAL;
	}

	if (drv->type > JPEGD_UNIT) {
		e("Failed : drv->type.%d\n", drv->type);
		return -EINVAL;
	}

	mutex_lock(&dev->mutex_clock);
	i("+ %s, refs_clck.%u\n", drv->name, dev->refs_clck);

	if (!dev->refs_clck) {
		e("Failed : %s.UnderFlow... refs_clck.%u\n", drv->name, dev->refs_clck);
		mutex_unlock(&dev->mutex_clock);
		return -EINVAL;
	}

	value = dev->clock[dvfs];
	dev->jpeg_clock[drv->type] = value;
	max_clk = hippjpgclk_max(dev);

	if (max_clk == 0) {
		e("Failed : max_clk.0\n");
		mutex_unlock(&dev->mutex_clock);
		return -EINVAL;
	}

	if (max_clk != dev->last_clock) {
		dev->clock_busy = 1;

		if ((ret = clk_set_rate(dev->jpegclk, max_clk)) < 0) {
			e("Failed: %s.%d.%d M, clk_set_rate.%d\n", drv->name, (int)max_clk / 1000000, (int)max_clk % 1000000, ret);
			dev->clock_busy = 0;
			mutex_unlock(&dev->mutex_clock);
			return ret;
		}

		dev->last_clock = max_clk;
		dev->clock_busy = 0;
	}

	clk_disable_unprepare(dev->jpegclk);
	dev->refs_clck --;
	i("- %s.refs_clck.%u\n", drv->name, dev->refs_clck);
	mutex_unlock(&dev->mutex_clock);
	return 0;
}

void __weak hipp_trusted_unmap(struct hipp_common_s *drv, unsigned int da, size_t size)
{
}

struct hipp_common_s *hipp_register(int type, int mode)
{
	struct hipp_comdevice_s *dev = NULL;
	struct hipp_common_s *drv = NULL;
	int ret = 0;
	i("+\n");

	if ((ret = invalid_type(type))) {
		e("Failed : invalid_type.%d\n", ret);
		return NULL;
	}

	if ((ret = invalid_secmode(mode))) {
		e("Failed : invalid_secmode.%d\n", ret);
		return NULL;
	}

	if ((dev = ippcomdev) == NULL) {
		e("Failed : dev.%pK\n", dev);
		return NULL;
	}

	if (!dev->initialized)
		e("Warning : comDevice Maynotbe initialized.%d\n", dev->initialized);

	mutex_lock(&dev->mutex_dev);

	if (dev->drv[type]) {
		e("Failed : %d has been Registered.%pK\n", type, dev->drv[type]);
		mutex_unlock(&dev->mutex_dev);
		return NULL;
	}

	if ((drv = (struct hipp_common_s *)kzalloc(sizeof(struct hipp_common_s), GFP_KERNEL)) == NULL) {
		e("Failed : kzalloc.%pK\n", drv);
		mutex_unlock(&dev->mutex_dev);
		return NULL;
	}

	drv->type           = type;
	drv->mode           = mode;
	drv->name           = dev->drvname[type];
	drv->enable_smmu    = hippsmmu_enable;
	drv->disable_smmu   = hippsmmu_disable;
	drv->setsid_smmu    = hippsmmu_setsid;
	drv->lock_usecase   = hippusecase_lock;
	drv->unlock_usecase = hippusecase_unlock;
	drv->enable_jpgclk  = hippjpgclk_enable;
	drv->disable_jpgclk = hippjpgclk_disable;
	drv->set_jpgclk     = hippjpgclk_dvfs;
	drv->dump           = hipp_dump;
	drv->comdev         = (void *)dev;
	drv->initialized    = 1;
	dev->drv[type]      = drv;
	mutex_unlock(&dev->mutex_dev);
	i("- %s\n", drv->name);
	return drv;
}

int hipp_unregister(int type)
{
	struct hipp_comdevice_s *dev = NULL;
	int ret = 0;

	if ((ret = invalid_type(type))) {
		e("Failed : invalid_type.%d\n", ret);
		return ret;
	}

	if ((dev = ippcomdev) == NULL) {
		e("Failed : dev.%pK\n", dev);
		return -ENOMEM;
	}

	mutex_lock(&dev->mutex_dev);

	if (dev->drv[type] == NULL) {
		e("Failed : %d has not been Registered.%pK\n", type, dev->drv[type]);
		mutex_unlock(&dev->mutex_dev);
		return -EINVAL;
	}

	i("+ %s\n", dev->drv[type]->name);
	kfree(dev->drv[type]);
	dev->drv[type] = NULL;
	mutex_unlock(&dev->mutex_dev);
	i("-\n");
	return 0;
}

u64 hipp_smmu_err_addr(void)
{
	struct hipp_comdevice_s *dev = ippcomdev;
	return (u64)(dev->smem_dma + MEM_IPP_ERR_ADDR_OFFSET);
}

static int hippmntn_thread(void *data)
{
	struct hipp_comdevice_s *dev = NULL;
	i("+\n");

	if ((dev = (struct hipp_comdevice_s *)data) == NULL) {
		e("Failed : dev.%pK\n", dev);
		return -ENOMEM;
	}

	while (1) {
		if (kthread_should_stop())
			break;

		wait_event(dev->wait_mntn, dev->wake_mntn);
		dev->wake_mntn = 0;
		do_ippdump(dev);
	}

	i("-\n");
	return 0;
}

static int hippcom_resource_do_init(struct hipp_comdevice_s *dev, struct device_node *np, struct device *device)
{
	int ret = 0;

	if ((ret = of_property_read_string(np, "clk-name", &dev->clkname)) != 0) {
		e("Failed : clk-name of_property_read_string.%d\n", ret);
		return -EINVAL;
	}

	dev->jpegclk = devm_clk_get(device, dev->clkname);

	if (IS_ERR_OR_NULL(dev->jpegclk)) {
		e("Failed : jpegclk.%s.%ld\n", dev->clkname, PTR_ERR(dev->jpegclk));
		return -EINVAL;
	}

	if ((ret = of_property_read_u32(np, "clk-tur", (unsigned int *)(&dev->clock[CLKLVL_TURBO]))) != 0) {
		e("Failed: clk-tur of_property_read_u32.%d\n", ret);
		return -EINVAL;
	}

	i("clk-tur.%lu\n", dev->clock[CLKLVL_TURBO]);

	if ((ret = of_property_read_u32(np, "clk-nor", (unsigned int *)(&dev->clock[CLKLVL_NORMINAL]))) != 0) {
		e("Failed: clk-nor of_property_read_u32.%d\n", ret);
		return -EINVAL;
	}

	i("clk-nor.%lu\n", dev->clock[CLKLVL_NORMINAL]);

	if ((ret = of_property_read_u32(np, "clk-svs", (unsigned int *)(&dev->clock[CLKLVL_SVS]))) != 0) {
		e("Failed: clk-svs of_property_read_u32.%d\n", ret);
		return -EINVAL;
	}

	i("clk-svs.%lu\n", dev->clock[CLKLVL_SVS]);

	if ((ret = of_property_read_u32(np, "clk-dis", (unsigned int *)(&dev->clock[CLKLVL_DISCLOCK]))) != 0) {
		e("Failed: clk-dis of_property_read_u32.%d\n", ret);
		return -EINVAL;
	}

	i("clk-dis.%lu\n", dev->clock[CLKLVL_DISCLOCK]);
	return 0;
}

static int hippcom_resource_init(struct hipp_comdevice_s *dev)
{
	struct device *device = NULL;
	struct device_node *np = NULL;
	int i = 0;
	i("+\n");

	if (dev == NULL) {
		e("Failed : hipp com dev.%pK\n", dev);
		return -ENODEV;
	}

	if (dev->pdev == NULL) {
		e("Failed : hipp com pdev.%pK\n", dev->pdev);
		return -ENODEV;
	}

	if ((device = &dev->pdev->dev) == NULL) {
		e("Failed : device.%pK\n", device);
		return -ENODEV;
	}

	if ((np = device->of_node) == NULL) {
		;
		e("Failed : np.%pK\n", np);
		return -ENXIO;
	}

	if ((dev->r = platform_get_resource(dev->pdev, IORESOURCE_MEM, 0)) == NULL) {
		e("Failed : platform_get_resource.%pK\n", dev->r);
		return -ENXIO;
	}

	device = &dev->pdev->dev;

	if ((dev->smmubase = devm_ioremap_resource(device, dev->r)) == NULL) {
		e("Failed : devm_ioremap_resource.%pK\n", dev->smmubase);
		return -ENOMEM;
	}

	i("Hipp Smmu Resource.%pK.(n.%s, s.0x%llx, e.0x%llx, f.0x%lx)\n", dev->r,
	  dev->r->name, (unsigned long long)dev->r->start, (unsigned long long)dev->r->end, dev->r->flags);

	if (hippcom_resource_do_init(dev, np, device) != 0)
		return -EINVAL;

	if (!(dev->irq = irq_of_parse_and_map(np, 0))) {
		e("Failed : irq_of_parse_and_map.%d\n", dev->irq);
		return -ENXIO;
	}

	i("IRQ.%d\n", dev->irq);

	for (i = 0; i < MAX_IPP_COM; i++)
		dev->jpeg_clock[i] = 0;

	return 0;
}

static int hippcom_earlydts_init(struct hipp_comdevice_s *dev)
{
	struct device_node *np = NULL;
	char *name = DTSNAME_HIPPCOM;
	int ret = 0, index = 0;
	i("+\n");

	if (dev == NULL) {
		e("Failed : hipp com dev.%pK\n", dev);
		return -ENODEV;
	}

	if ((np = of_find_compatible_node(NULL, NULL, name)) == NULL) {
		e("Failed : %s.of_find_compatible_node.%pK\n", name, np);
		return -ENXIO;
	}

	if ((ret = of_property_read_u32(np, "drv-num", (unsigned int *)(&dev->drvnum))) != 0) {
		e("Failed: drv-num of_property_read_u32.%d\n", ret);
		return -EINVAL;
	}

	if (dev->drvnum > MAX_IPP_COM) {
		e("Failed: drvnum.%d > %d, Please Check Codes/DTS\n", dev->drvnum, MAX_IPP_COM);
		return -EINVAL;
	}

	if ((ret = of_property_read_string_array(np, "drv-names", dev->drvname, dev->drvnum)) < dev->drvnum) { //lint !e574
		e("Failed : drv-names of_property_read_string_array.%d < %d\n", ret, dev->drvnum);
		return -EINVAL;
	}

	for (index = 0; index < dev->drvnum; index ++)//lint !e574
		i("HippDrvList.%d < %d.%s\n", index, MAX_IPP_COM, dev->drvname[index]);

	return 0;
}

static int hippcom_probe(struct platform_device *pdev)
{
	struct hipp_comdevice_s *dev = NULL;
	int ret = 0;
	i("+\n");

	if ((dev = ippcomdev) == NULL) {
		e("Failed : ippcomdev.%pK\n", ippcomdev);
		return -ENOMEM;
	}

	dev->initialized = 0;
	dev->pdev = pdev;
	platform_set_drvdata(pdev, dev);

	if ((ret = hippcom_resource_init(dev)) != 0) {
		e("Failed : hippcom_resource_init.%d\n", ret);
		goto free_dev;
	}

	init_waitqueue_head(&dev->wait_mntn);
	init_waitqueue_head(&dev->wait_trusted);
	init_waitqueue_head(&dev->wait_clken);
	init_waitqueue_head(&dev->wait_clkdi);
	mutex_init(&dev->mutex_dev);
	mutex_init(&dev->mutex_smmu);
	mutex_init(&dev->mutex_usecase);
	mutex_init(&dev->mutex_clock);
	dev->smem_size = HIPP_SHAREDMEM_SIZE;

	if ((dev->smem_vaddr = isp_fstcma_alloc(&dev->smem_dma, dev->smem_size, GFP_KERNEL)) == NULL) {
		pr_err("[%s] smem_vaddr.%pK\n", __func__, dev->smem_vaddr);
		goto free_dev;
	}

	if ((dev->kthread = kthread_run(hippmntn_thread, dev, "hippmntn")) == NULL) {
		e("Failed : kthread_run.%pK\n", dev->kthread);
		goto free_fstcma;
	}

	dev->initialized = 1;
	i("-\n");
	return 0;
free_fstcma:
	isp_fstcma_free((void *)dev->smem_vaddr, dev->smem_dma, dev->smem_size);
free_dev:
	kfree(dev);

	if (dev != NULL)
		dev = NULL;

	if (ippcomdev != NULL)
		ippcomdev = NULL;

	return ret;
}

static int hippcom_remove(struct platform_device *pdev)
{
	struct hipp_comdevice_s *dev = NULL;
	i("+\n");

	if ((dev = (struct hipp_comdevice_s *)platform_get_drvdata(pdev)) == NULL) {
		e("Failed : platform_get_drvdata, dev.%pK.pdev.%pK\n", dev, pdev);
		return -ENODEV;
	}

	if (dev->kthread != NULL) {
		kthread_stop(dev->kthread);
		dev->kthread = NULL;
	}

	if (dev->smem_vaddr != NULL && dev->smem_dma)
		isp_fstcma_free((void *)dev->smem_vaddr, dev->smem_dma, dev->smem_size);

	dev->initialized = 0;
	kfree(dev);

	if (dev != NULL)
		dev = NULL;

	if (ippcomdev != NULL)
		ippcomdev = NULL;

	i("-\n");
	return 0;
}

#ifdef CONFIG_OF
static struct of_device_id hippcom_of_id[] = {
	{.compatible = DTSNAME_HIPPCOM},
	{}
};
#endif

static struct platform_driver hippcom_pdrvr = {
	.probe          = hippcom_probe,
	.remove         = hippcom_remove,
	.driver         = {
		.name           = "hippcom",
		.owner          = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(hippcom_of_id),
#endif
	},
};

static int __init hippcom_init(void)
{
	struct hipp_comdevice_s *dev = NULL;
	int ret = 0;
	i("+\n");

	if ((dev = (struct hipp_comdevice_s *)kzalloc(sizeof(struct hipp_comdevice_s), GFP_KERNEL)) == NULL) {
		e("Failed : kzalloc.%pK\n", dev);
		return -ENOMEM;
	}

	mutex_init(&dev->mutex_dev);

	if ((ret = hippcom_earlydts_init(dev)) != 0) {
		e("Failed : hippcom_earlydts_init.%d\n", ret);
		goto free_dev;
	}

	ippcomdev = dev;
	i("-\n");
	return 0;
free_dev:
	kfree(dev);

	if (dev != NULL)
		dev = NULL;

	if (ippcomdev != NULL)
		ippcomdev = NULL;

	return ret;
}

static void __exit hippcom_exit(void)
{
	i("+\n");

	if (ippcomdev != NULL) {
		kfree(ippcomdev);
		ippcomdev = NULL;
	}

	i("-\n");
	return;
}

module_platform_driver(hippcom_pdrvr);
subsys_initcall(hippcom_init);
module_exit(hippcom_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Hisilicon IPP Common Driver");
MODULE_AUTHOR("isp");
