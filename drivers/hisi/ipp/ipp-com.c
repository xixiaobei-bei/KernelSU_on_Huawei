/*
 * Hisilicon IPP Common Driver
 *
 * Copyright (c) 2018 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

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
#include <linux/hisi/hisi-iommu.h>
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

#define SMMU_NS_INTSTAT  (0x18)
#define SMMU_NS_INTCLR   (0x1C)
#define IPPSMMU_EXC_TYPE_NUM  6

typedef enum smmu_int_stat {
    ints_permis_stat      = 1<<0,
    ints_ext_stat         = 1<<1,
    ints_tlbmiss_stat     = 1<<2,
    ints_ptw_trans_stat   = 1<<3,
    ints_ptw_invalid_stat = 1<<4,
    ints_ptw_ns_stat      = 1<<5,
} smmu_inti_stat_e;

typedef enum smmu_int_clr {
    ints_permis_clr      = 1<<0,
    ints_ext_clr         = 1<<1,
    ints_tlbmiss_clr     = 1<<2,
    ints_ptw_trans_clr   = 1<<3,
    ints_ptw_invalid_clr = 1<<4,
    ints_ptw_ns_clr      = 1<<5,
} smmu_inti_clr_e;

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
    struct mutex mutex_genpool;
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
    struct hipp_smem_s *bl31_vaddr;
    dma_addr_t bl31_dma;
    size_t bl31_size;
    void *sgpool_vaddr;
    dma_addr_t sgpool_dma;
    size_t sgpool_size;
    struct gen_pool *genpool;
    unsigned long genpool_start;
    size_t genpool_size;
    struct hipp_sec_ion_s *ptable_secmem_ion;
};

static struct hipp_comdevice_s *ippcomdev = NULL;

static int invalid_secmode(unsigned int mode)
{
    if (mode >= MAX_SECMODE) {
        E("mode.(0 < %d < %d)\n", mode, MAX_SECMODE);
        return -1;
    }

    return 0;
}

static int invalid_type(int type)
{
    if (type < 0 || type >= MAX_IPP_COM) {
        E("Failed : Invalid Type.%d\n", type);
        return -EINVAL;
    }

    return 0;
}

static struct hipp_comdevice_s *hipp_drv2dev(struct hipp_common_s *drv)
{
    struct hipp_comdevice_s *dev = NULL;
    int ret = 0;

    if (!drv) {
        E("Failed : drv.%pK\n", drv);
        return NULL;
    }

    if ((ret = invalid_type(drv->type))) {
        E("Failed : invalid_type.%d\n", ret);
        return NULL;
    }

    if ((ret = invalid_secmode(drv->mode))) {
        E("Failed : invalid_secmode.%d\n", ret);
        return NULL;
    }

    if (!(dev = (struct hipp_comdevice_s *)drv->comdev)) {
        E("Failed : dev.%pK\n", dev);
        return NULL;
    }

    if (drv != dev->drv[drv->type]) {
        E("Failed : Mismatch drv.(%pK, %pK), type.%d\n", drv, dev->drv[drv->type], drv->type);
        return NULL;
    }

    return dev;
}

static int do_ippsmmu_enable(struct hipp_common_s *drv)
{
    struct hipp_comdevice_s *dev = NULL;
    int ret = 0;

    if (!(dev = hipp_drv2dev(drv))) {
        E("Failed : hipp_drv2dev.(%pK).%pK\n", drv, dev);
        return -ENOMEM;
    }

    I("+ %s\n", drv->name);
/*
    if (drv->mode) {
        if ((ret = hipp_smmu_nontrus_init(dev->smmubase, dev->bl31_vaddr->pte_nontrus)) != 0) {
            E("Failed : %s.hisp_smmu_enable.(0x%lx)%d\n", drv->name, dev->bl31_vaddr->pte_nontrus, ret);
            return ret;
        }
*/
        if ((ret = atfhipp_smmu_enable(drv->mode)) != 0) {
            E("Failed : %s.atfhipp_smmu_enable.%d\n", drv->name, ret);
            return ret;
        }
/*
    } else {
        if ((ret = hipp_smmu_nontrus_init(dev->smmubase, dev->bl31_vaddr->pte_nontrus)) != 0) {
            E("Failed : %s.hisp_smmu_enable.(0x%lx)%d\n", drv->name, dev->bl31_vaddr->pte_nontrus, ret);
            return ret;
        }
    }
*/
    I("- %s\n", drv->name);

    return 0;
}

static int do_ippsmmu_disable(struct hipp_common_s *drv)
{
    struct hipp_comdevice_s *dev = NULL;
    int ret = 0;

    if (!(dev = hipp_drv2dev(drv))) {
        E("Failed : hipp_drv2dev.(%pK).%pK\n", drv, dev);
        return -ENOMEM;
    }

    I("+ %s\n", drv->name);
    if (drv->mode) {
        if ((ret = atfhipp_smmu_disable()) != 0) {
            E("Failed : %s.atfhipp_smmu_disable.%d\n", drv->name, ret);
            return ret;
        }
    }
    I("- %s\n", drv->name);

    return 0;
}

static int do_ippdump(struct hipp_comdevice_s *dev)
{
    struct hipp_common_s *drv = NULL;
    int i = 0;

    if (!dev) {
        E("Failed : dev.%pK\n", dev);
        return -ENOMEM;
    }

    I("Device: refs_smmu.(0x%x, 0x%x), fault_count.(0x%x, 0x%x), initialized.0x%x\n",
        dev->refs_smmu[NONTRUS], dev->refs_smmu[TRUSTED], dev->fault_count[NONTRUS], dev->fault_count[TRUSTED], dev->initialized);
    for (i = 0; i < MAX_IPP_COM; i ++) {
        drv = dev->drv[i];
        if (!drv) {
            E("%d Not Registered\n", i);
            continue;
        }
        I("Driver.%d: %s.type.0x%x, initialized.0x%x, mode.0x%x, count.(e.0x%x, d.0x%x), timestamp.(f.[%5u.%06u], l.[%5u.%06u], fault.[%5u.%06u])\n", i,
            drv->name, drv->type, drv->initialized, drv->mode, drv->mntn_smmu.icount, drv->mntn_smmu.xcount,
            do_div(drv->mntn_smmu.ifirst_ts, 1000000000), do_div(drv->mntn_smmu.ifirst_ts ,1000),
            do_div(drv->mntn_smmu.ilast_ts, 1000000000), do_div(drv->mntn_smmu.xlast_ts ,1000),
            do_div(dev->fault_ts, 1000000000), do_div(dev->fault_ts ,1000));
    }

    return 0;
}

static int hippsmmu_enable(struct hipp_common_s *drv)
{
    struct hipp_comdevice_s *dev = NULL;
    unsigned long long timestamp = 0;
    int ret = 0;

    if (!(dev = hipp_drv2dev(drv))) {
        E("Failed : hipp_drv2dev.(%pK).%pK\n", drv, dev);
        return -ENOMEM;
    }

    I("+ %s\n", drv->name);
    mutex_lock(&dev->mutex_smmu);
    timestamp = hisi_getcurtime();
    if (unlikely(!drv->mntn_smmu.ifirst_ts))
        drv->mntn_smmu.ifirst_ts = timestamp;

    drv->mntn_smmu.ilast_ts = timestamp;
    drv->mntn_smmu.icount ++;

    if (dev->refs_smmu[drv->mode] ++) {
        mutex_unlock(&dev->mutex_smmu);
        return 0;
    }

    if ((ret = do_ippsmmu_enable(drv)) != 0) {
        E("Failed : %s.do_ippsmmu_enable.%d\n", drv->name, ret);
        dev->fault_count[drv->mode] ++;
        dev->fault_ts = timestamp;
        dev->refs_smmu[drv->mode] --;
        E("Failed : %s.refs_smmu[%u] -> %u\n", drv->name, drv->mode, dev->refs_smmu[drv->mode]);
        mutex_unlock(&dev->mutex_smmu);
        drv->dump(drv);
        return ret;
    }

    if (unlikely(!dev->mntn_smmu.ifirst_ts))
        dev->mntn_smmu.ifirst_ts = timestamp;

    dev->mntn_smmu.ilast_ts = timestamp;
    mutex_unlock(&dev->mutex_smmu);
    I("- %s\n", drv->name);

    return 0;
}

static int hippsmmu_disable(struct hipp_common_s *drv)
{
    struct hipp_comdevice_s *dev = NULL;
    unsigned long long timestamp = 0;
    int ret = 0;

    if (!(dev = hipp_drv2dev(drv))) {
        E("Failed : hipp_drv2dev.(%pK).%pK\n", drv, dev);
        return -ENOMEM;
    }

    I("+ %s\n", drv->name);
    mutex_lock(&dev->mutex_smmu);
    timestamp = hisi_getcurtime();
    if (unlikely(!drv->mntn_smmu.xfirst_ts))
        drv->mntn_smmu.xfirst_ts = timestamp;

    drv->mntn_smmu.xlast_ts = timestamp;
    drv->mntn_smmu.xcount ++;
    if (!dev->refs_smmu[drv->mode]) {
        E("Failed : %s.refs_smmu[%u].%d\n", drv->name, drv->mode, dev->refs_smmu[drv->mode]);
        mutex_unlock(&dev->mutex_smmu);
        return -EINVAL;
    }
    if (-- dev->refs_smmu[drv->mode]) {
        mutex_unlock(&dev->mutex_smmu);
        return 0;
    }

    if ((ret = do_ippsmmu_disable(drv)) != 0) {
        E("Failed : %s.do_ippsmmu_disable.%d\n", drv->name, ret);
        dev->fault_count[drv->mode] ++;
        dev->fault_ts = timestamp;
        dev->refs_smmu[drv->mode] ++;
        E("Failed : %s.refs_smmu[%u] -> %u\n", drv->name, drv->mode, dev->refs_smmu[drv->mode]);
        mutex_unlock(&dev->mutex_smmu);
        drv->dump(drv);
        return ret;
    }

    if (unlikely(!dev->mntn_smmu.xfirst_ts))
        dev->mntn_smmu.xfirst_ts = timestamp;

    dev->mntn_smmu.xlast_ts = timestamp;
    mutex_unlock(&dev->mutex_smmu);
    I("- %s\n", drv->name);

    return 0;
}

static int hippsmmu_setsid(struct hipp_common_s *drv, unsigned int sid, unsigned int mode)
{
    struct hipp_comdevice_s *dev = NULL;
    int ret = 0;

    if (!(dev = hipp_drv2dev(drv))) {
        E("Failed : hipp_drv2dev.(%pK).%pK\n", drv, dev);
        return -ENOMEM;
    }

    I("+ %s\n", drv->name);
    mutex_lock(&dev->mutex_smmu);
    if ((ret = atfhipp_smmu_smrx(sid, mode)) != 0) {
        E("Failed : %s.atfhipp_smmu_smrx.%d\n", drv->name, ret);
        dev->fault_count[drv->mode] ++;
        dev->fault_ts = hisi_getcurtime();
        mutex_unlock(&dev->mutex_smmu);
        drv->dump(drv);
        return ret;
    }
    mutex_unlock(&dev->mutex_smmu);
    I("- %s\n", drv->name);

    return 0;
}

static int hipp_dump(struct hipp_common_s *drv)
{
    struct hipp_comdevice_s *dev = NULL;

    if (!(dev = hipp_drv2dev(drv))) {
        E("Failed : hipp_drv2dev.(%pK).%pK\n", drv, dev);
        return -ENOMEM;
    }

    I("+ %s\n", drv->name);
    mutex_lock(&dev->mutex_smmu);
    do_ippdump(dev);
    mutex_unlock(&dev->mutex_smmu);
    I("- %s\n", drv->name);

    return 0;
}

static int hippusecase_lock(struct hipp_common_s *drv)
{
    struct hipp_comdevice_s *dev = NULL;
    unsigned long timeout = 0;
    int ret = 0;

    if (!(dev = hipp_drv2dev(drv))) {
        E("Failed : hipp_drv2dev.(%pK).%pK\n", drv, dev);
        return -ENOMEM;
    }

    I("+ %s\n", drv->name);
    mutex_lock(&dev->mutex_clock);
    if (dev->clock_busy) {
        E("Failed : clock_busy.%d\n", dev->clock_busy);
        mutex_unlock(&dev->mutex_clock);
        return -EBUSY;
    }
    mutex_unlock(&dev->mutex_clock);

    mutex_lock(&dev->mutex_usecase);
    if (dev->refs_lock[TRUSTED]) {
        E("Failed : %s.refs_clck[%d].%u, Try Again...\n", drv->name, TRUSTED, dev->refs_lock[TRUSTED]);
        mutex_unlock(&dev->mutex_usecase);
        return -EAGAIN;
    }
    dev->refs_lock[drv->mode] ++;

    if (dev->refs_lock[TRUSTED]) {
        if (dev->refs_lock[NONTRUS]) {
            timeout = msecs_to_jiffies(TRUSTED_TIMEOUT_MS);
            mutex_unlock(&dev->mutex_usecase);
            if (!(ret = wait_event_timeout(dev->wait_trusted, dev->refs_lock[NONTRUS] == 0, timeout))) {
                E("Failed : %s.wait_event_timeout.(0x%x, 0x%lx).0x%x\n", drv->name, dev->refs_lock[NONTRUS] == 0, timeout, ret);
                return -ETIMEDOUT;
            }
            mutex_lock(&dev->mutex_usecase);
        }
    }
    mutex_unlock(&dev->mutex_usecase);
    I("- %s\n", drv->name);

    return 0;
}

static int hippusecase_unlock(struct hipp_common_s *drv)
{
    struct hipp_comdevice_s *dev = NULL;

    if (!(dev = hipp_drv2dev(drv))) {
        E("Failed : hipp_drv2dev.(%pK).%pK\n", drv, dev);
        return -ENOMEM;
    }

    I("+ %s\n", drv->name);
    mutex_lock(&dev->mutex_usecase);
    if (!dev->refs_lock[drv->mode]) {
        E("Failed : %s.UnderFlow... refs_lock.0x%x @ %u\n", drv->name, dev->refs_lock[drv->mode], drv->mode);
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
    I("- %s\n", drv->name);

    return 0;
}

static unsigned long hippjpgclk_max(struct hipp_comdevice_s *dev)
{
    unsigned long max;
    int i = 0;

    if (dev == NULL) {
        E("Failed : dev is NULL\n");
        return 0;
    }

    max = dev->jpeg_clock[0];
    for (i=1; i<HISI_JPEGD_UNIT+1; i++) {
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

    if (!(dev = hipp_drv2dev(drv))) {
        E("Failed : hipp_drv2dev.(%pK).%pK\n", drv, dev);
        return -ENOMEM;
    }

    I("+ %s, refs_clck.%u, clk_dvfs.%d, refs_lock(N.0x%x, T.0x%x)\n", drv->name, dev->refs_clck, dvfs,
        dev->refs_lock[NONTRUS], dev->refs_lock[TRUSTED]);
    if (dvfs > CLKLVL_SVS) {
        E("Failed : drv->dvfs.%d\n", dvfs);
        return -EINVAL;
    }

    if (drv->type > HISI_JPEGD_UNIT) {
        E("Failed : drv->type.%d\n", drv->type);
        return -EINVAL;
    }

    mutex_lock(&dev->mutex_clock);

    value = dev->clock[dvfs];
    //dev->jpeg_clock[drv->type] = value;
    max_clk = hippjpgclk_max(dev);

    if (value > max_clk) {
        dev->clock_busy = 1;
        if ((ret = clk_set_rate(dev->jpegclk, value)) < 0) {
            E("Failed: %s.(max_clk.%d.%d M, value.%d.%d M), clk_set_rate.%d\n", drv->name,
                (int)max_clk/1000000, (int)max_clk%1000000, (int)value/1000000, (int)value%1000000, ret);
            dev->clock_busy = 0;
            mutex_unlock(&dev->mutex_clock);
            return ret;
        }
        dev->last_clock = value;
        dev->clock_busy = 0;
    }
    dev->jpeg_clock[drv->type] = value;

    if ((ret = clk_prepare_enable(dev->jpegclk)) < 0) {
        E("Failed: %s.(max_clk.%d.%d M, value.%d.%d M), clk_prepare_enable.%d\n", drv->name,
            (int)max_clk/1000000, (int)max_clk%1000000, (int)value/1000000, (int)value%1000000, ret);
        mutex_unlock(&dev->mutex_clock);
        return ret;
    }

    dev->refs_clck ++;
    I("- %s.clk_set_rate.[%s].(max_clk.%d.%d M, value.%d.%d M), refs_clck.%u, refs_lock(N.0x%x, T.0x%x)\n", drv->name,
        ((value > max_clk) ? "Run" : "Bypass"),
        (int)max_clk/1000000, (int)max_clk%1000000, (int)value/1000000, (int)value%1000000, dev->refs_clck,
		dev->refs_lock[NONTRUS], dev->refs_lock[TRUSTED]);
    mutex_unlock(&dev->mutex_clock);

    return 0;
}

static int hippjpgclk_dvfs(struct hipp_common_s *drv, unsigned int dvfs)
{
    struct hipp_comdevice_s *dev = NULL;
    unsigned long value = 0;
    int ret = 0;
    unsigned long max_clk = 0;
    unsigned long value_buf = 0;

    if (!(dev = hipp_drv2dev(drv))) {
        E("Failed : hipp_drv2dev.(%pK).%pK\n", drv, dev);
        return -ENOMEM;
    }

    I("+ %s, clk_dvfs.%d\n", drv->name, dvfs);
    if (dvfs > CLKLVL_SVS) {
        E("Failed : drv->dvfs.%d\n", dvfs);
        return -EINVAL;
    }

    if (drv->type > HISI_JPEGD_UNIT) {
        E("Failed : drv->type.%d\n", drv->type);
        return -EINVAL;
    }

    mutex_lock(&dev->mutex_clock);
    I("+ %s, refs_clck.%u\n", drv->name, dev->refs_clck);
    if (!dev->refs_clck) {
        E("Failed : %s.UnderFlow... refs_clck.%u, \n", drv->name, dev->refs_clck);
        mutex_unlock(&dev->mutex_clock);
        return -EINVAL;
    }

    value = dev->clock[dvfs];
    value_buf = dev->jpeg_clock[drv->type];
    dev->jpeg_clock[drv->type] = value;

    max_clk = hippjpgclk_max(dev);
    if (max_clk == dev->last_clock) {
        I("- %s.max_clk.%d.%d M, value.%d.%d M\n",
            drv->name, (int)max_clk/1000000, (int)max_clk%1000000, (int)value/1000000, (int)value%1000000);
        mutex_unlock(&dev->mutex_clock);
        return 0;
    }

    dev->clock_busy = 1;
    if ((ret = clk_set_rate(dev->jpegclk, max_clk)) < 0) {
        E("Failed: %s.%d.%d M, clk_set_rate.%d\n", drv->name, (int)max_clk/1000000, (int)max_clk%1000000, ret);
        dev->clock_busy = 0;
        dev->jpeg_clock[drv->type] = value_buf;
        mutex_unlock(&dev->mutex_clock);
        return ret;
    }
    dev->last_clock = max_clk;
    dev->clock_busy = 0;

    I("- %s.clk_set_rate.%d.%d M, refs_clck.%u\n", drv->name, (int)max_clk/1000000, (int)max_clk%1000000, dev->refs_clck);
    mutex_unlock(&dev->mutex_clock);

    return 0;
}

static int hippjpgclk_disable(struct hipp_common_s *drv, unsigned int dvfs)
{
    struct hipp_comdevice_s *dev = NULL;
    unsigned long value = 0;
    int ret = 0;
    unsigned long max_clk = 0;

    if (!(dev = hipp_drv2dev(drv))) {
        E("Failed : hipp_drv2dev.(%pK).%pK\n", drv, dev);
        return -ENOMEM;
    }

    I("+ %s, clk_dvfs.%d\n", drv->name, dvfs);
    if (dvfs != CLKLVL_DISCLOCK) {
        E("Failed : drv->dvfs.%d is not CLKLVL_DISCLOCK\n", dvfs);
        return -EINVAL;
    }

    if (drv->type > HISI_JPEGD_UNIT) {
        E("Failed : drv->type.%d\n", drv->type);
        return -EINVAL;
    }

    mutex_lock(&dev->mutex_clock);
    I("+ %s, refs_clck.%u\n", drv->name, dev->refs_clck);
    if (!dev->refs_clck) {
        E("Failed : %s.UnderFlow... refs_clck.%u\n", drv->name, dev->refs_clck);
        mutex_unlock(&dev->mutex_clock);
        return -EINVAL;
    }

    value = dev->clock[dvfs];
    dev->jpeg_clock[drv->type] = value;

    max_clk = hippjpgclk_max(dev);
	if (max_clk == 0) {
        E("Failed : max_clk.0\n");
		mutex_unlock(&dev->mutex_clock);
        return -EINVAL;
    }

    if (max_clk != dev->last_clock) {
        dev->clock_busy = 1;
        if ((ret = clk_set_rate(dev->jpegclk, max_clk)) < 0) {
            E("Failed: %s.%d.%d M, clk_set_rate.%d\n", drv->name, (int)max_clk/1000000, (int)max_clk%1000000, ret);
            dev->clock_busy = 0;
            mutex_unlock(&dev->mutex_clock);
            return ret;
        }
        dev->last_clock = max_clk;
        dev->clock_busy = 0;
    }

    clk_disable_unprepare(dev->jpegclk);
    dev->refs_clck --;
    I("- %s.refs_clck.%u\n", drv->name, dev->refs_clck);
    mutex_unlock(&dev->mutex_clock);

    return 0;
}

static unsigned long hipp_genpool_alloc(struct hipp_comdevice_s *dev, unsigned long size)
{
    unsigned long iova = 0;

    if (!dev) {
        E("Failed : dev.%pK\n", dev);
        return 0;
    }

    if (!dev->genpool) {
        E("Failed : genpool.%pK\n", dev->genpool);
        return 0;
    }

    mutex_lock(&dev->mutex_genpool);
    if (!(iova = gen_pool_alloc(dev->genpool, size)))
        E("Failed : gen_pool_alloc.(%pK, 0x%lx).0x%lx\n", dev->genpool, size, iova);
    mutex_unlock(&dev->mutex_genpool);

    return iova;
}

static void hipp_genpool_free(struct hipp_comdevice_s *dev, unsigned long iova, size_t size)
{
    if (!dev) {
        E("Failed : dev.%pK\n", dev);
        return;
    }

    if (!dev->genpool) {
        E("Failed : genpool.%pK\n", dev->genpool);
        return;
    }

    mutex_lock(&dev->mutex_genpool);
    gen_pool_free(dev->genpool, iova, size);
    mutex_unlock(&dev->mutex_genpool);
}

static struct gen_pool *hipp_genpool_setup(unsigned long start, unsigned long size, unsigned int align)
{
    struct gen_pool *pool = NULL;
    int ret = 0;

    if (!start || !size) {
        E("Failed : start.0x%lx, size.0x%lx\n", start, size);
        return NULL;
    }

    if (!(pool = gen_pool_create((int)(order_base_2(align)), -1))) {/*lint !e666 !e835 !e747 !e516 !e866 !e712 */
        E("Failed : gen_pool_create.%pK\n", pool);
        return NULL;
    }

    if ((ret = gen_pool_add(pool, start, size, -1)) != 0) {
        E("Failed : gen_pool_add.(%pK, 0x%lx, 0x%lx).%d\n", pool, start, size, ret);
        gen_pool_destroy(pool);
        return NULL;
    }

    return pool;
}

void hipp_trusted_unmap(struct hipp_common_s *drv, unsigned int da, size_t size)
{
    struct hipp_comdevice_s *dev = NULL;
    struct hipp_mapping_s *mapping = NULL;
    size_t mapsize = 0;
    int ret = 0;

    if (!(dev = hipp_drv2dev(drv))) {
        E("Failed : hipp_drv2dev.(%pK).%pK\n", drv, dev);
        return;
    }

    if (drv->mode != TRUSTED || !size || !dev->bl31_vaddr) {
        E("Failed : mode.(%d != %d), size.0x%x, bl31_vaddr.%pK\n", drv->mode, TRUSTED, (unsigned int)size, dev->bl31_vaddr);
        return;
    }

    mapping = &dev->bl31_vaddr->mapping[HIPP_DYNAMIC_MAPPING];
    mapsize = PAGE_ALIGN(size);/*lint !e50 */
    mapping->daddr= da;
    mapping->size = mapsize;
    I("da.0x%x, size.0x%x, mapsize.0x%x\n", da, (unsigned int)size, (unsigned int)mapsize);

    if ((ret = atfhipp_trusted_unmap()) != 0)
        E("Failed : atfhipp_trusted_unmap.%d\n", ret);

    if ((dev->genpool_start <= mapping->daddr) && (mapping->daddr< (dev->genpool_start + dev->genpool_size))) {
        I("mapping->da.0x%lx mapping->size.0x%x\n", mapping->daddr, (unsigned int)mapping->size);
        hipp_genpool_free(dev, (unsigned long)mapping->daddr, (size_t)mapping->size);
    }

    return;
}

static unsigned int ipp_map_set_pa_list(void *listmem, struct scatterlist *sg, unsigned int size)
{
    dma_addr_t dma_addr = 0;
    unsigned int len, set_size = 0;
    struct map_sglist_s *maplist = listmem;
    unsigned int last_counts = 0, last_len = 0;

    while (sg) {
        if ((dma_addr = sg_dma_address(sg)) == 0)
            dma_addr = sg_phys(sg);

        len = sg->length;
        if (len == 0) {
            pr_err("[%s] break len.0x%x\n", __func__, len);
            break;
        }
        set_size += len;
        if (set_size > size) {
            pr_err("[%s] break size.(0x%x > 0x%x), len.0x%x\n", __func__, set_size, size, len);
            maplist->addr = (unsigned long long)dma_addr;
            maplist->size = len - (set_size - size);
            break;
        }

        maplist->addr = (unsigned long long)dma_addr;
        maplist->size = len;
        if (last_len != len) {
            if (last_len != 0)
                pr_info("[%s] list.(%pK + %pK), maplist.(0x%llx, 0x%x X 0x%x)\n", 
                __func__, listmem, maplist, (unsigned long long)dma_addr, last_counts, last_len);
            last_counts = 1;
            last_len = len;
        } else {
            last_counts ++;
        }
        maplist++;
        sg = sg_next(sg);
    }

    pr_info("[%s] list.(%pK + %pK), maplist.(0x%llx, 0x%x X 0x%x)\n", 
        __func__, listmem, maplist, (unsigned long long)dma_addr, last_counts, last_len);
    pr_info("%s: size.0x%x == set_size.0x%x\n", __func__, size, set_size);

    return last_counts;
}

/*lint -save -e838 */
static unsigned int ipp_mmu_map(struct scatterlist *sg, unsigned int *size)
{
    unsigned int map_size = 0, map_prot;
    struct hipp_mapping_s *map_info;
    void *addr = NULL;
    unsigned long iova;
    int ret = 0;
    struct scatterlist *sgbuf;
    unsigned int len = 0, set_size = 0;
    struct hipp_comdevice_s *dev = NULL;

    I("+\n");
    if(sg == NULL) {
        E("Failed : sg.%pK\n", sg);
    }

    if (!(dev = ippcomdev)) {
        E("Failed : dev.%pK\n", dev);
        return 0;
    }

    addr = dev->sgpool_vaddr;
    map_info = &dev->bl31_vaddr->mapping[HIPP_DYNAMIC_MAPPING];

    sgbuf = sg;
    while (sgbuf) {
        len = sgbuf->length;
        if (len == 0) {
            pr_err("[%s] break len.0x%x\n", __func__, len);
            break;
        }
        set_size += len;
        sgbuf = sg_next(sgbuf);
    }
    map_size = PAGE_ALIGN(set_size);
    if (map_size >= HIPP_SECDDR_MAX) {
        pr_err("[%s] Failed : map_size.0x%x\n", __func__,map_size);
        return 0;
    }

    map_prot = MEM_ATTR_READ | MEM_ATTR_WRITE | MEM_ATTR_SEC;
    map_info->prot = map_prot;

    iova = hipp_genpool_alloc(dev, (unsigned long)map_size);
    if(iova == 0){
        pr_err("[%s] Failed : hisi_isp_alloc_iova iova.0x%lx\n", __func__,iova);
        return 0;
    }
    map_info->daddr= (unsigned int)iova;
    map_info->size = map_size;
    map_info->offset = 0;
    map_info->sglpaddr = (unsigned long)dev->sgpool_dma;

    pr_info("[%s] a7va.0x%lx, size.0x%x, map_size.0x%x, map_prot.0x%x\n", __func__,
                            map_info->daddr, set_size, map_size, map_prot);
    map_info->sglnum = ipp_map_set_pa_list(addr, sg, map_size);
    if ((ret = atfhipp_trusted_map()) != 0) {
        hipp_genpool_free(dev, iova, map_size);
        E("Failed : atfhipp_trusted_map.%d\n", ret);
        return 0;
    }
    *size = map_size;
    return map_info->daddr;
}
/*lint -restore */

static unsigned int hipp_trusted_map(struct hipp_common_s *drv, unsigned int sharefd, unsigned int* size)
{
    struct ion_handle* hdl = NULL;
    struct sg_table *table = NULL;
    struct ion_client *ion_client = NULL;
    struct dma_buf_attachment *attach = NULL;
    struct dma_buf *dmabuf = NULL;
    struct device *device = NULL;
    unsigned int va;
    struct hipp_comdevice_s *dev = NULL;

    I("[%s] : sharefd.%d\n", __func__, sharefd);
    if (!(dev = hipp_drv2dev(drv))) {
        E("Failed : hipp_drv2dev.(%pK).%pK\n", drv, dev);
        return 0;
    }

	if (drv->mode != TRUSTED) {
		E("[%s] Failed : mode.%d, TRUSTED.%d\n", __func__, drv->mode, TRUSTED);
		return 0;
	}

    device = &dev->pdev->dev;
    ion_client = hisi_ion_client_create("secmem_ion_client");
    if (IS_ERR(ion_client)) {
        pr_err("[%s] secmem_ion_client create failed \n", __func__);
        return 0;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
    hdl = ion_import_dma_buf_fd(ion_client, sharefd);
#else
    hdl = ion_import_dma_buf(ion_client, sharefd);
#endif
    if (IS_ERR_OR_NULL(hdl)) {
        pr_err("[%s] Failed : ion_import_dma_buf\n", __func__);
        return 0;
    }

    dmabuf = ion_share_dma_buf(ion_client, hdl);
    if (!dmabuf) {
        pr_err("[%s] Failed : ion_share_dma_buf.%pK.(%pK, %pK)\n", __func__, dmabuf, ion_client, hdl);
        ion_free(ion_client, hdl);
        return 0;
    }

    attach = dma_buf_attach(dmabuf, device);
    if (IS_ERR_OR_NULL(attach)) {
        pr_err("[%s] Failed : dma_buf_attach.%pK.(%pK, %pK)\n", __func__, attach, dmabuf, device);
        dma_buf_put(dmabuf);
        ion_free(ion_client, hdl);
        return 0;
    }

    table = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
    if (IS_ERR_OR_NULL(table)) {
        pr_err("[%s] Failed : dma_buf_map_attachment.%pK.(%pK)\n", __func__, table, attach);
        dma_buf_detach(dmabuf, attach);
        dma_buf_put(dmabuf);
        ion_free(ion_client, hdl);
        return 0;
    }

    va = ipp_mmu_map(table->sgl, size);
    dma_buf_unmap_attachment(attach, table, DMA_BIDIRECTIONAL);
    dma_buf_detach(dmabuf, attach);
    dma_buf_put(dmabuf);
    ion_free(ion_client, hdl);

    return va;
}

struct hipp_common_s *hipp_register(int type, int mode)
{
    struct hipp_comdevice_s *dev = NULL;
    struct hipp_common_s *drv = NULL;
    int ret = 0;

    I("+\n");
    if ((ret = invalid_type(type))) {
        E("Failed : invalid_type.%d\n", ret);
        return NULL;
    }

    if ((ret = invalid_secmode(mode))) {
        E("Failed : invalid_secmode.%d\n", ret);
        return NULL;
    }

    if (!(dev = ippcomdev)) {
        E("Failed : dev.%pK\n", dev);
        return NULL;
    }

    if (!dev->initialized)
        E("Warning : comDevice Maynotbe initialized.%d\n", dev->initialized);

    mutex_lock(&dev->mutex_dev);
    if (dev->drv[type]) {
        E("Failed : %d has been Registered.%pK\n", type, dev->drv[type]);
        mutex_unlock(&dev->mutex_dev);
        return NULL;
    }

    if ((drv = (struct hipp_common_s *)kzalloc(sizeof(struct hipp_common_s), GFP_KERNEL)) == NULL) {
        E("Failed : kzalloc.%pK\n", drv);
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
	drv->shrf2da        = hipp_trusted_map;
    drv->comdev         = (void *)dev;
    drv->initialized    = 1;
    dev->drv[type]      = drv;
    mutex_unlock(&dev->mutex_dev);
    I("- %s\n", drv->name);

    return drv;
}

int hipp_unregister(int type)
{
    struct hipp_comdevice_s *dev = NULL;
    int ret = 0;

    if ((ret = invalid_type(type))) {
        E("Failed : invalid_type.%d\n", ret);
        return ret;
    }

    if (!(dev = ippcomdev)) {
        E("Failed : dev.%pK\n", dev);
        return -ENOMEM;
    }

    mutex_lock(&dev->mutex_dev);
    if (!dev->drv[type]) {
        E("Failed : %d has not been Registered.%pK\n", type, dev->drv[type]);
        mutex_unlock(&dev->mutex_dev);
        return -EINVAL;
    }

    I("+ %s\n", dev->drv[type]->name);
    kfree(dev->drv[type]);
    dev->drv[type] = NULL;
    mutex_unlock(&dev->mutex_dev);
    I("-\n");

    return 0;
}

static int hippmntn_thread(void *data)
{
    struct hipp_comdevice_s *dev = NULL;

    I("+\n");
    if (!(dev = (struct hipp_comdevice_s *)data)) {
        E("Failed : dev.%pK\n", dev);
        return -ENOMEM;
    }

    while (1) {
        if (kthread_should_stop())
            break;

        wait_event(dev->wait_mntn, dev->wake_mntn);
        dev->wake_mntn = 0;
        do_ippdump(dev);
    }
    I("-\n");

    return 0;
}

static int hippcom_resource_init(struct hipp_comdevice_s *dev)
{
    struct device *device = NULL;
    struct device_node *np = NULL;
    int i = 0;
    int ret = 0;

    I("+\n");
    if (!dev) {
        E("Failed : hipp com dev.%pK\n", dev);
        return -ENODEV;
    }

    if (!dev->pdev) {
        E("Failed : hipp com pdev.%pK\n", dev->pdev);
        return -ENODEV;
    }

    if (!(device = &dev->pdev->dev)) {
        E("Failed : device.%pK\n", device);
        return -ENODEV;
    }

    if (!(np = device->of_node)) {;
        E("Failed : np.%pK\n", np);
        return -ENXIO;
    }

    if ((dev->r = platform_get_resource(dev->pdev, IORESOURCE_MEM, 0)) == NULL) {
        E("Failed : platform_get_resource.%pK\n", dev->r);
        return -ENXIO;
    }

    device = &dev->pdev->dev;
    if (!(dev->smmubase = devm_ioremap_resource(device, dev->r))) {
        E("Failed : devm_ioremap_resource.%pK\n", dev->smmubase);
        return -ENOMEM;
    }
    I("Hipp Smmu Resource.%pK.(n.%s, s.0x%llx, e.0x%llx, f.0x%lx)\n", dev->r,
        dev->r->name, (unsigned long long)dev->r->start, (unsigned long long)dev->r->end, dev->r->flags);

    if ((ret = of_property_read_string(np, "clk-name", &dev->clkname)) != 0) {
        E("Failed : clk-name of_property_read_string.%d\n", ret);
        return -EINVAL;
    }

    dev->jpegclk = devm_clk_get(device, dev->clkname);
    if (IS_ERR_OR_NULL(dev->jpegclk)) {
        E("Failed : jpegclk.%s.%ld\n", dev->clkname, PTR_ERR(dev->jpegclk));
        return -EINVAL;
    }

    if ((ret = of_property_read_u32(np, "clk-tur", (unsigned int *)(&dev->clock[CLKLVL_TURBO]))) != 0 ) {
        E("Failed: clk-tur of_property_read_u32.%d\n", ret);
        return -EINVAL;
    }
    I("clk-tur.%lu\n", dev->clock[CLKLVL_TURBO]);

    if ((ret = of_property_read_u32(np, "clk-nor", (unsigned int *)(&dev->clock[CLKLVL_NORMINAL]))) != 0 ) {
        E("Failed: clk-nor of_property_read_u32.%d\n", ret);
        return -EINVAL;
    }
    I("clk-nor.%lu\n", dev->clock[CLKLVL_NORMINAL]);

    if ((ret = of_property_read_u32(np, "clk-svs", (unsigned int *)(&dev->clock[CLKLVL_SVS]))) != 0 ) {
        E("Failed: clk-svs of_property_read_u32.%d\n", ret);
        return -EINVAL;
    }
    I("clk-svs.%lu\n", dev->clock[CLKLVL_SVS]);

    if ((ret = of_property_read_u32(np, "clk-dis", (unsigned int *)(&dev->clock[CLKLVL_DISCLOCK]))) != 0 ) {
        E("Failed: clk-dis of_property_read_u32.%d\n", ret);
        return -EINVAL;
    }
    I("clk-dis.%lu\n", dev->clock[CLKLVL_DISCLOCK]);

    if (!(dev->irq = irq_of_parse_and_map(np, 0))) {
        E("Failed : irq_of_parse_and_map.%d\n", dev->irq);
        return -ENXIO;
    }
    I("IRQ.%d\n", dev->irq);
/*
    if ((ret = request_irq(dev->irq, hippsmmu_irq_handler, 0, "HIPP_SMMU_IRQ", (void *)dev)) != 0) {
        E("Failed : request_irq.%d\n", ret);
        return ret;
    }
    I("Hipp Smmu IRQ.%d, handler.%pK\n", dev->irq, hippsmmu_irq_handler);
*/
    for(i=0; i< MAX_IPP_COM; i++) {
        dev->jpeg_clock[i] = 0;
    }

    return 0;
}

static int hippcom_earlydts_init(struct hipp_comdevice_s *dev)
{
    struct device_node *np = NULL;
    char *name = DTSNAME_HIPPCOM;
    int ret = 0, index = 0;

    I("+\n");
    if (!dev) {
        E("Failed : hipp com dev.%pK\n", dev);
        return -ENODEV;
    }

    if (!(np = of_find_compatible_node(NULL, NULL, name))) {
        E("Failed : %s.of_find_compatible_node.%pK\n", name, np);
        return -ENXIO;
    }

    if ((ret = of_property_read_u32(np, "drv-num", (unsigned int *)(&dev->drvnum))) != 0 ) {
        E("Failed: drv-num of_property_read_u32.%d\n", ret);
        return -EINVAL;
    }

    if (dev->drvnum > MAX_IPP_COM) {
        E("Failed: drvnum.(%d > %d), Please Check Codes/DTS\n", dev->drvnum, MAX_IPP_COM);
        return -EINVAL;
    }

    if ((ret = of_property_read_string_array(np, "drv-names", dev->drvname, dev->drvnum)) < dev->drvnum) { //lint !e574
        E("Failed : drv-names of_property_read_string_array.(%d < %d)\n", ret, dev->drvnum);
        return -EINVAL;
    }

    for (index = 0; index < dev->drvnum; index ++)//lint !e574
        I("HippDrvList.(%d < %d).%s\n", index, MAX_IPP_COM, dev->drvname[index]);

    return 0;
}

static int hippsmmu_get_pteaddr(struct hipp_comdevice_s *dev)
{
    struct iommu_domain_data *info = NULL;

    I("+\n");
    if (!dev) {
        E("Failed : hipp smmu dev.%pK\n", dev);
        return -ENODEV;
    }

    if (!dev->pdev) {
        E("Failed : pdev.%pK\n", dev->pdev);
        return -ENODEV;
    }

    if ((dev->domain = hisi_ion_enable_iommu(dev->pdev)) == NULL) {
        E("Failed : hisi_ion_enable_iommu domain.%pK\n", dev->domain);
        return -ENODEV;
    }

    if ((info = (struct iommu_domain_data *)dev->domain->priv) == NULL) {
        E("Failed : info.%pK\n", info);
        return -ENODEV;
    }

    if (dev->bl31_vaddr == NULL) {
        E("Failed : dev->bl31_vaddr is NULL\n");
        return -ENODEV;
    }

    dev->bl31_vaddr->pte_nontrus = info->phy_pgd_base;

    I("- PTE.0x%lx\n", dev->bl31_vaddr->pte_nontrus);

    return 0;
}

static int hipp_ion_phys(struct ion_client *client, struct ion_handle *handle,
         dma_addr_t *addr)
{
    int ret = -ENODEV;
    int share_fd = 0;
    struct dma_buf *buf = NULL;
    struct dma_buf_attachment *attach = NULL;
    struct sg_table *sgt = NULL;
    struct scatterlist *sgl;
    struct hipp_comdevice_s *dev = NULL;
    struct device *device = NULL;

    pr_info("[%s] +\n", __func__);

    if ((IS_ERR(client))||(IS_ERR(handle))) {
        pr_err("hisp_ion_phys failed \n");
        return -ENODEV;
    }

    if (!(dev = ippcomdev)) {
        E("Failed : dev.%pK\n", dev);
        return -ENOMEM;
    }

    device = &dev->pdev->dev;

    share_fd = ion_share_dma_buf_fd(client, handle);
    if (share_fd < 0) {
        pr_err("[%s] Failed : ion_share_dma_buf_fd, share_fd.%d\n", __func__, share_fd);
        return share_fd;
    }

    buf = dma_buf_get(share_fd);
    if (IS_ERR_OR_NULL(buf)) {
        pr_err("[%s] Failed : dma_buf_get, buf.%pK\n", __func__, buf);
        goto err_dma_buf_get;
    }

    attach = dma_buf_attach(buf, device);
    if (IS_ERR_OR_NULL(attach)) {
        pr_err("[%s] Failed : dma_buf_attach, attach.%pK\n", __func__, attach);
        goto err_dma_buf_attach;
    }

    sgt = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
    if (IS_ERR_OR_NULL(sgt)) {
        pr_err("[%s] Failed : dma_buf_map_attachment, sgt.%pK\n", __func__, sgt);
        goto err_dma_buf_map_attachment;
    }

    sgl = sgt->sgl;
    if (sgl == NULL) {
        pr_err("[%s] Failed : sgl.NULL\n", __func__);
        goto err_sgl;
    }

    // Get physical addresses from scatter list
    *addr = sg_phys(sgl);/*[false alarm]:it's not the bounds of allocated memory */

    pr_info("[%s] -\n", __func__);
    ret = 0;

err_sgl:
    dma_buf_unmap_attachment(attach, sgt, DMA_BIDIRECTIONAL);
err_dma_buf_map_attachment:
    dma_buf_detach(buf, attach);
err_dma_buf_attach:
    dma_buf_put(buf);
err_dma_buf_get:
    sys_close(share_fd);
    return ret;
}

int hipp_ptable_secmem_init(void)
{
    struct hipp_comdevice_s *dev = NULL;
    struct hipp_sec_ion_s *hipp_secmem_ion = NULL;
    dma_addr_t secmem_ion_phys = 0x0 ;
    int ret = 0;

    pr_info("[%s] +\n", __func__);
    if (!(dev = ippcomdev)) {
        E("Failed : dev.%pK\n", dev);
        return -ENOMEM;
    }

    hipp_secmem_ion = kzalloc(sizeof(struct hipp_sec_ion_s), GFP_KERNEL);/*lint !e838 */
    if (!hipp_secmem_ion) {
        pr_err("in %s hipp_secmem_ion kzalloc is failed\n", __func__);
        return -ENODEV;
    }

    hipp_secmem_ion->ion_client = hisi_ion_client_create("secmem_ion_client");
    if (IS_ERR(hipp_secmem_ion->ion_client)) {
        pr_err("hisi hipp_secmem_ion_client create failed \n");
        kfree(hipp_secmem_ion);
        return -ENODEV;
    }
/* alloc sec pagetable mem */
    hipp_secmem_ion->ion_handle= ion_alloc(hipp_secmem_ion->ion_client, MEM_SEC_PTABLE_SIZE, PAGE_SIZE, (1 << ION_IRIS_DAEMON_HEAP_ID), ION_FLAG_SECURE_BUFFER);
    if (IS_ERR(hipp_secmem_ion->ion_handle)){
        pr_err("hisi_secmem_ion_alloc failed \n");
        ion_client_destroy(hipp_secmem_ion->ion_client);
        kfree(hipp_secmem_ion);
        return -ENODEV;
    }
    ret = hipp_ion_phys(hipp_secmem_ion->ion_client, hipp_secmem_ion->ion_handle,&secmem_ion_phys);/*lint !e838 */
    if (ret < 0)
    {
        pr_err("%s, failed to get phy addr,ret:%d!\n", __func__, ret);
        ion_free(hipp_secmem_ion->ion_client, hipp_secmem_ion->ion_handle);
        ion_client_destroy(hipp_secmem_ion->ion_client);
        kfree(hipp_secmem_ion);
        return -ENODEV;
    }
    hipp_secmem_ion->sec_phymem_addr = (u64)secmem_ion_phys;

    pr_debug("[%s] atf_pagetable_phymem.0x%llx\n", __func__,hipp_secmem_ion->sec_phymem_addr);
    dev->ptable_secmem_ion = hipp_secmem_ion;
    dev->bl31_vaddr->pte_trusted = hipp_secmem_ion->sec_phymem_addr;

    if ((ret = atfhipp_ptable_init()) != 0) {
        E("Failed : atfhipp_ptable_init.(0x%lx).%d\n", (unsigned long)hipp_secmem_ion->sec_phymem_addr, ret);
        ion_free(hipp_secmem_ion->ion_client, hipp_secmem_ion->ion_handle);
        ion_client_destroy(hipp_secmem_ion->ion_client);
        kfree(hipp_secmem_ion);
        return -ENODEV;
    }    

    pr_info("[%s] -\n", __func__);

    return 0;
}

int hipp_ptable_secmem_deinit(void)
{
    struct hipp_comdevice_s *dev = NULL;
    struct hipp_sec_ion_s *hipp_secmem_ion = NULL;
    int ret = 0;

    pr_info("[%s] +\n", __func__);
    if (!(dev = ippcomdev)) {
        E("Failed : dev.%pK\n", dev);
        return -ENOMEM;
    }

    hipp_secmem_ion = dev->ptable_secmem_ion;
    if(hipp_secmem_ion == NULL){
        E("[%s] ptable_secmem_ion is NULL\n", __func__);
        return -ENODEV;
    }

    if((hipp_secmem_ion->ion_client == NULL) || (hipp_secmem_ion->ion_handle == NULL)) {
        E("[%s] ion_client.%pK, ion_handle.%pK\n", __func__, hipp_secmem_ion->ion_client, hipp_secmem_ion->ion_handle);
        return -ENODEV;
    }
    ion_free(hipp_secmem_ion->ion_client, hipp_secmem_ion->ion_handle);
    ion_client_destroy(hipp_secmem_ion->ion_client);
    hipp_secmem_ion->ion_client = NULL;
    hipp_secmem_ion->ion_handle= NULL;
    kfree(hipp_secmem_ion);
    dev->ptable_secmem_ion = NULL;

    if ((ret = atfhipp_ptable_exit()) != 0) {
        E("Failed : atfhipp_ptable_exit.%d\n", ret);
        return ret;
    }

    I("[%s] -\n", __func__);
    return 0;
}

static int hippcom_probe(struct platform_device *pdev)
{
    struct hipp_comdevice_s *dev = NULL;
    int ret = 0;

    I("+\n");
    if (!(dev = ippcomdev)) {
        E("Failed : ippcomdev.%pK\n", ippcomdev);
        return -ENOMEM;
    }

    dev->initialized = 0;
    dev->pdev = pdev;
    platform_set_drvdata(pdev, dev);

    if ((ret = hippcom_resource_init(dev)) != 0) {
        E("Failed : hippcom_resource_init.%d\n", ret);
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
    mutex_init(&dev->mutex_genpool);

    dev->smem_size = HIPP_SHAREDMEM_SIZE;
    if ((dev->smem_vaddr = hisi_fstcma_alloc(&dev->smem_dma, dev->smem_size, GFP_KERNEL)) == NULL) {
        pr_err("[%s] smem_vaddr.%pK\n", __func__, dev->smem_vaddr);
        goto free_dev;
    }
    dev->bl31_vaddr     = (struct hipp_smem_s *)(HIPP_BL31_OFFSET + dev->smem_vaddr);
    dev->bl31_dma       = (dma_addr_t)(HIPP_BL31_OFFSET + dev->smem_dma);
    dev->bl31_size      = (size_t)HIPP_BL31_SIZE;
    dev->sgpool_vaddr   = (void *)(HIPP_SGPOOL_OFFSET + dev->smem_vaddr);
    dev->sgpool_dma     = (dma_addr_t)(HIPP_SGPOOL_OFFSET + dev->smem_dma);
    dev->sgpool_size    = (size_t)HIPP_SGPOOL_SIZE;
    if ((ret = atfhipp_trusted_smem((uint64_t)dev->smem_dma)) != 0) {
        E("Failed : atfhipp_trusted_smem.(0x%lx).%d\n", (unsigned long)dev->smem_dma, ret);
        goto free_fstcma;
    }

    if ((ret = hippsmmu_get_pteaddr(dev)) != 0) {
        E("Failed : hippsmmu_get_pteaddr.%d\n", ret);
        goto free_fstcma;
    }

    dev->genpool_start  = 0xC0000000;
    dev->genpool_size   = 0x10000000;
    if (!(dev->genpool = hipp_genpool_setup(dev->genpool_start, dev->genpool_size, 0x8000))) {
        E("Failed : hipp_genpool_setup.(0x%lx, 0x%x).%pK\n", dev->genpool_start, (unsigned int)dev->genpool_size, dev->genpool);
        goto free_fstcma;
    }

    if (!(dev->kthread = kthread_run(hippmntn_thread, dev, "hippmntn"))) {
        E("Failed : kthread_run.%pK\n", dev->kthread);
        goto free_genpool;
    }

    dev->initialized = 1;
    I("-\n");

    return 0;

free_genpool:
    gen_pool_destroy(dev->genpool);
free_fstcma:
    hisi_fstcma_free((void *)dev->smem_vaddr, dev->smem_dma, dev->smem_size);
free_dev:
    kfree(dev);
    if (dev)
        dev = NULL;
    if (ippcomdev)
        ippcomdev = NULL;

    return ret;
}

static int hippcom_remove(struct platform_device *pdev)
{
    struct hipp_comdevice_s *dev = NULL;

    I("+\n");
    if (!(dev = (struct hipp_comdevice_s *)platform_get_drvdata(pdev))) {
        E("Failed : platform_get_drvdata, dev.%pK.pdev.%pK\n", dev, pdev);
        return -ENODEV;
    }

    if (dev->kthread) {
        kthread_stop(dev->kthread);
        dev->kthread = NULL;
    }

    if (dev->genpool) {
        gen_pool_destroy(dev->genpool);
        dev->genpool = NULL;
    }

    if (dev->smem_vaddr && dev->smem_dma)
        hisi_fstcma_free((void *)dev->smem_vaddr, dev->smem_dma, dev->smem_size);

    dev->initialized = 0;
    kfree(dev);
    if (dev)
        dev = NULL;
    if (ippcomdev)
        ippcomdev = NULL;
    I("-\n");

    return 0;
}

static struct of_device_id hippcom_of_id[] = {
    {.compatible = DTSNAME_HIPPCOM},
    {}
};

static struct platform_driver hippcom_pdrvr = {
    .probe          = hippcom_probe,
    .remove         = hippcom_remove,
    .driver         = {
        .name           = "hippcom",
        .owner          = THIS_MODULE,
        .of_match_table = of_match_ptr(hippcom_of_id),
    },
};

static int __init hippcom_init(void)
{
    struct hipp_comdevice_s *dev = NULL;
    int ret = 0;

    I("+\n");
    if (!(dev = (struct hipp_comdevice_s *)kzalloc(sizeof(struct hipp_comdevice_s), GFP_KERNEL))) {
        E("Failed : kzalloc.%pK\n", dev);
        return -ENOMEM;
    }

    mutex_init(&dev->mutex_dev);
    if ((ret = hippcom_earlydts_init(dev)) != 0) {
        E("Failed : hippcom_earlydts_init.%d\n", ret);
        goto free_dev;
    }

    ippcomdev = dev;
    I("-\n");

    return 0;

free_dev:
    kfree(dev);
    if (dev)
        dev = NULL;
    if (ippcomdev)
        ippcomdev = NULL;

    return ret;
}

static void __exit hippcom_exit(void)
{
    I("+\n");
    if (ippcomdev) {
        kfree(ippcomdev);
        ippcomdev = NULL;
    }
    I("-\n");

    return;
}

module_platform_driver(hippcom_pdrvr);
subsys_initcall(hippcom_init);
module_exit(hippcom_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Hisilicon IPP Common Driver");
MODULE_AUTHOR("c00261891 <chentao20@huawei.com>");
