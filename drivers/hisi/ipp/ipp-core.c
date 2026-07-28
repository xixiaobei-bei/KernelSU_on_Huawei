/*
 * Hisi ISP CPE
 *
 * Copyright (c) 2017 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>
#include <linux/of_irq.h>
#include <linux/iommu.h>
#include <linux/ion.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/genalloc.h>
#include <linux/hisi/hisi-iommu.h>
#include <linux/hisi/ion-iommu.h>
#include <linux/version.h>
#include <linux/hisi/hipp.h>
#include "mfnr.h"
#include "mcf.h"
#include "slam.h"
#include "ipp.h"
#include "slamtest.h"
#include "sub_ctrl_drv_priv.h"
#include "sub_ctrl_reg_offset.h"
#include "memory.h"
#include <media/camera/jpeg/jpeg_base.h>


#define SLAM_BI_SID     (30)
#define SLAM_STAT_SID   (32)
#define SLAM_RATE_INDEX_MAX (2)

int kmsgcat_mask = (INFO_BIT | ERROR_BIT);
/*lint -e21 -e846 -e514 -e778 -e866 -e84 -e429 -e613 -e668*/
module_param_named(kmsgcat_mask, kmsgcat_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);

enum HISP_CPE_CLK_TYPE {
    CLK_HIGH_RATE    = 600000000,//600MHZ
    CLK_NOMAL_RATE    = 400000000,//415M
    CLK_LOW_RATE    = 300000000,//300M
    MAX_CPE_CLK
};

enum HISP_CPE_IRQ_TYPE {
    CPE_IRQ_0   = 0,
    CPE_IRQ_1   = 1,
    MAX_HISP_CPE_IRQ
};

enum hipp_fd_type_e {
    BI_SHARE_FD    = 0,
    STAT_SHARE_FD  = 1,
    MAX_HIPP_TYPE_ORB
};


unsigned int  CLK_RATE_MAP[3] =
{
    [0] = CLK_HIGH_RATE,
    [1] = CLK_NOMAL_RATE,
    [2]= CLK_LOW_RATE,
};

struct hispcpe_s {
    struct miscdevice miscdev;
    struct platform_device *pdev;
    wait_queue_head_t cpe_wait;
    wait_queue_head_t slam_wait;
    int cpe_ready;
    int slam_ready;
    int initialized;
    unsigned int  sec_path;
    unsigned int  sec_req;
    atomic_t open_refs;
    struct regulator *cpe_supply;
    atomic_t cpe_supply_refs;
    atomic_t sec_slam_supply_refs;
    atomic_t cpe_mfnr_refs;
    atomic_t cpe_slam_refs;
    atomic_t sec_slam_refs;
    struct mutex lock;
    unsigned int irq_num;
    unsigned int reg_num;
    unsigned int curr_slam_rate_value;
    unsigned int curr_cpe_rate_value;
    unsigned int ipp_set_rate_value;
    int irq[MAX_HISP_CPE_IRQ];
    struct resource *r[MAX_HISP_CPE_REG];
    void __iomem *reg[MAX_HISP_CPE_REG];
    struct hipp_common_s *ippdrv;
    struct hipp_common_s *sec_slamdrv;
    struct ion_handle *handle;
    struct ion_client *ion_client;
    unsigned int daddr;
    void *virt_addr;
    unsigned int jpg_clk_rate;
    unsigned int jpgclk_low_frequency;
    unsigned int jpgclk_off_frequency;
    struct clk *jpg_clk;
};

struct hispcpe_s *hispcpe_dev = NULL;

#define MEM_HISPCPE_SIZE                (0x00300000) /*  the cpe iova size */

#define ISP_TEXT_MEM_ATTR               (IOMMU_READ | IOMMU_CACHE | IOMMU_EXEC)
#define ISP_DATA_MEM_ATTR               (IOMMU_READ | IOMMU_WRITE | IOMMU_CACHE)
#define ISP_DATA_MEM_NO_CACHE_ATTR      (IOMMU_READ | IOMMU_WRITE)
#define ISP_DEV_MEM_ATTR                (IOMMU_DEVICE)

struct memory_block_s {
    int shared_fd;
    int size;
    unsigned long prot;
};

#define HISP_CPE_PWRUP          _IOWR('C', 0x1001, int)
#define HISP_CPE_PWRDN          _IOWR('C', 0x1002, int)
#define HISP_CPE_MCF_REQ        _IOWR('C', 0x2001, int)
#define HISP_CPE_MFNR_REQ       _IOWR('C', 0x2002, int)
#define HISP_CPE_MAP_BUF        _IOWR('C', 0x2003, int)
#define HISP_CPE_UNMAP_BUF      _IOWR('C', 0x2004, int)
#define HISP_CPE_SLAM_REQ       _IOWR('C', 0x2005, int)
#define HISP_CPE_SLAMTEST_REQ   _IOWR('C', 0x2006, int)

#define DTS_NAME_HISI_IPP "hisilicon,ipp"
#define HISP_CPE_TIMEOUT_MS (3000)
#define HISP_MIN(a, b) (((a) < (b)) ? (a) : (b))

unsigned int g_share_fd_size[MAX_HIPP_TYPE_ORB] = { 0 };

static struct hispcpe_s *get_cpedev_bymisc(struct miscdevice *mdev)
{
    struct hispcpe_s *dev = NULL;

    if (!mdev) {
        pr_err("[%s] Failed : mdev.%pK\n", __func__, mdev);
        return NULL;
    }

    dev = container_of(mdev, struct hispcpe_s, miscdev);

    return dev;
}

void hispcpe_reg_set(unsigned int mode, unsigned int offset, unsigned int value)
{
    struct hispcpe_s *dev = hispcpe_dev;
    void __iomem *reg_base = dev->reg[mode];

    D("[%s] +\n", __func__);
    I("mode = %d, value = %d, reg = 0x%pK \n", mode, value, reg_base + offset);
    writel(value, reg_base + offset);
    D("[%s] -\n", __func__);
}

unsigned int hispcpe_reg_get(unsigned int mode, unsigned int offset)
{
    struct hispcpe_s *dev = hispcpe_dev;
    unsigned int value = 0;
    void __iomem *reg_base = dev->reg[mode];

    D("[%s] +\n", __func__);
    if (mode == SMMU_REG) {
        E("Failed : mode.(0x%x = 0x%x)\n", mode, SMMU_REG);
        return 0;
    }
    value = readl(reg_base + offset);
    D("[%s] -\n", __func__);
    return value;
}

static irqreturn_t hispcpe_irq_handler(int irq, void *devid)
{
    struct hispcpe_s *dev = NULL;
    unsigned int value = 0;
    U_CPE_IRQ1_REG0 reg0;
    U_CPE_COMP_IRQ_REG regp;
    unsigned int slam_value;

    D("+\n");
    if ((dev = (struct hispcpe_s *)devid) == NULL) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return IRQ_NONE;
    }

    if (!dev->initialized) {
        pr_err("[%s] Failed : CPE Device Not Exist.%d\n", __func__, dev->initialized);
        return IRQ_NONE;
    }

    if (irq == dev->irq[CPE_IRQ_0]) {
        regp.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP]+CPE_COMP_IRQ_REG_OFFSET));
        reg0.bits.cvdr_cpe_vprd_eof    = regp.bits.cvdr_cpe_vprd_eof;
        reg0.bits.cvdr_cpe_vpwr_eof    = regp.bits.cvdr_cpe_vpwr_eof;
        reg0.bits.cvdr_cmdlst_vpwr_eof = regp.bits.cvdr_cmdlst_vpwr_eof;
        reg0.bits.cvdr_cmdlst_vprd_eof = regp.bits.cvdr_cmdlst_vprd_eof;
        reg0.bits.cpe_crop_vpwr_eof    = regp.bits.cpe_crop_vpwr_eof;
        writel(reg0.u32, (volatile void __iomem *)(dev->reg[CPE_TOP]+CPE_IRQ1_REG0_OFFSET));

        value = readl((volatile void __iomem *)(dev->reg[CPE_TOP]+CPE_IRQ0_REG4_OFFSET));
        writel(value, (volatile void __iomem *)(dev->reg[CPE_TOP]+CPE_IRQ0_REG0_OFFSET));

        dev->cpe_ready = 1;
        wake_up_interruptible(&dev->cpe_wait);

        pr_info("[%s] CPE_IRQ_0. value = 0x%x.\n", __func__, value);
    } else if (irq == dev->irq[CPE_IRQ_1]) {
        value = readl((volatile void __iomem*)(dev->reg[CPE_TOP] + CPE_IRQ1_REG4_OFFSET));
        writel(value, (volatile void __iomem*)(dev->reg[CPE_TOP] + CPE_IRQ1_REG0_OFFSET));
        pr_info("[%s] CPE_IRQ_1. value = 0x%x.\n", __func__, value);
        slam_value = readl((volatile void __iomem*)(dev->reg[CPE_TOP] + 0x50));
        if (slam_value & (1 << 15))
        {
            pr_info("SLAM_COMP_EOF COME!");
            writel(0x1D240 & 0x7FFC0, (volatile void __iomem*)(dev->reg[CPE_TOP] + 0x48));
            dev->slam_ready = 1;
            wake_up_interruptible(&dev->slam_wait);
        }  
    }

    D("-\n");

    return IRQ_HANDLED;
}

static int hispcpe_cmdlst_clock_enable(struct hispcpe_s *dev)
{
    U_CPE_CRG_CFG0 cfg;

    I("+\n");
    if (!dev) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -EINVAL;
    }

    cfg.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP]+SUB_CTRL_CPE_CRG_CFG0_REG));
    cfg.bits.cmdlst_clken = 1;
    writel(cfg.u32, (volatile void __iomem *)(dev->reg[CPE_TOP]+SUB_CTRL_CPE_CRG_CFG0_REG));
    I("-\n");

    return 0;
}

static int hispcpe_cmdlst_clock_disable(struct hispcpe_s *dev)
{
    U_CPE_CRG_CFG0 cfg;

    D("+\n");
    if (!dev) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -EINVAL;
    }

    cfg.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP]+SUB_CTRL_CPE_CRG_CFG0_REG));
    cfg.bits.cmdlst_clken = 0;
    writel(cfg.u32, (volatile void __iomem *)(dev->reg[CPE_TOP]+SUB_CTRL_CPE_CRG_CFG0_REG));
    D("-\n");

    return 0;
}

static int hispcpe_irq_config(struct hispcpe_s *dev)
{
    U_CPE_IRQ0_REG2 cfg_irq0;
    U_CPE_IRQ1_REG2 cfg_irq1;
    U_CVDR_IRQ_REG1 cfg_cvdr;

    I("+\n");
    if (!dev) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -EINVAL;
    }

    //sw unmask cpe_irq0
    cfg_irq0.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP]+SUB_CTRL_CPE_IRQ0_REG2_REG));
    cfg_irq0.bits.cpe_irq0_mask = 0;
    writel(cfg_irq0.u32, (volatile void __iomem *)(dev->reg[CPE_TOP]+SUB_CTRL_CPE_IRQ0_REG2_REG));

    cfg_irq1.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP]+SUB_CTRL_CPE_IRQ1_REG2_REG));
    cfg_irq1.bits.cpe_irq1_mask = 0x1C702FC0;
    writel(cfg_irq1.u32, (volatile void __iomem *)(dev->reg[CPE_TOP]+SUB_CTRL_CPE_IRQ1_REG2_REG));

    //comp irq en
    cfg_cvdr.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP]+SUB_CTRL_CVDR_IRQ_REG1_REG));
    cfg_cvdr.bits.cvdr_irq_mask &= 0xFFFF7FFF;
    writel(cfg_cvdr.u32, (volatile void __iomem *)(dev->reg[CPE_TOP]+SUB_CTRL_CVDR_IRQ_REG1_REG));
    I("-\n");

    return 0;
}

static int dataflow_cvdr_global_config(struct hispcpe_s *dev)
{
    unsigned int reg_val;

    reg_val = (0 << 0)        //axiwrite_du_threshold = 0
              | (34 << 8)     //du_threshold_reached = 34
              | (19 << 16)     //max_axiread_id = 19
              | (11 << 24);     //max_axiwrite_id = 11
    writel(reg_val, (volatile void __iomem *)(dev->reg[CVDR_REG] + 0x0));

    return CPE_FW_OK;
}

int sec_slam_powerup(struct hispcpe_s *dev)
{
    int ret = 0, err = 0;
    
    I("+\n");
    if (!dev) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -EINVAL;
    }

    if (atomic_read(&dev->sec_slam_supply_refs) > 0) {
        pr_err("[%s] Failed: sec_slam_supply Opened.0x%x\n", __func__, atomic_read(&dev->sec_slam_supply_refs));
        return -EBUSY;
    }

    if (!dev->sec_slamdrv) {
        pr_err("[%s] Failed : sec_slamdrv.%pK\n", __func__, dev->sec_slamdrv);
        return -ENOMEM;
    }

    if (!dev->sec_slamdrv->enable_jpgclk) {
        pr_err("[%s] Failed : sec_slamdrv->enable_jpgclk.%pK\n", __func__, dev->sec_slamdrv->enable_jpgclk);
        return -ENOMEM;
    }
    

    if ((ret = regulator_enable(dev->cpe_supply)) != 0) {
        pr_err("[%s] Failed: sec_slam regulator_enable.%d\n", __func__, ret);
        return ret;
    }

   if ((ret = dev->sec_slamdrv->enable_jpgclk(dev->sec_slamdrv, CLKLVL_SVS)) != 0)
    {
        pr_err("[%s] Failed to set sec_slam powerup rate mode:( %d).\n", __func__, CLKLVL_SVS);
        goto fail_enjgpclk;
    }

    if ((ret = atfhipp_orb_init()) != 0) {
        pr_err("[%s] Failed : orb_init.%d\n", __func__, ret);
        return ret;
    }

    if ((ret = hispcpe_cmdlst_clock_enable(dev)) != 0) {
        pr_err("[%s] Failed: hispcpe_cmdlst_clock_enable.%d\n", __func__, ret);
        goto fail_cmdlist;
    }

    if ((ret = hispcpe_irq_config(dev)) != 0) {
        pr_err("[%s] Failed: hispcpe_cmdlst_clock_enable.%d\n", __func__, ret);
        goto fail_irq;
    }

    if ((ret = dataflow_cvdr_global_config(dev)) != 0) {
        pr_err("[%s] Failed: dataflow_cvdr_global_config.%d\n", __func__, ret);
        return ret;
    }

    if ((ret = hipp_ptable_secmem_init()) < 0) {
        E("[%s] Failed : hisp_secmem_init.%d\n", __func__, ret);
        goto fail_irq;
    }

    atomic_set(&dev->sec_slam_supply_refs, 1);
    dev->sec_path = 1;
    I("-\n");
    return 0;

fail_irq:
    if ((err = hispcpe_cmdlst_clock_disable(dev)) != 0)
        pr_err("[%s] Failed: sec_slam_cmdlst_clock_disable.%d\n", __func__, err);
fail_cmdlist:
    if ((err = dev->sec_slamdrv->disable_jpgclk(dev->sec_slamdrv, CLKLVL_DISCLOCK)) != 0)
        pr_err("[%s] Failed : dev->sec_slamdrv->enable_jpgclk.%d\n", __func__, err);
fail_enjgpclk:
    if ((err = regulator_disable(dev->cpe_supply)) != 0)
        pr_err("[%s] Failed: sec_slam regulator_disable.%d\n", __func__, err);

    return ret;
}
EXPORT_SYMBOL(sec_slam_powerup);

static int ipp_setclk_enable(struct hispcpe_s* dev)
{
    int ret;

    if ((ret = jpeg_ipp_set_rate(dev->jpg_clk, dev->jpgclk_low_frequency)) != 0)
    {
        pr_err("[%s] Failed: jpeg_ipp_set_rate( %d).%d\n", __func__ , dev->jpgclk_low_frequency, ret);
    }

    if ((ret = jpeg_ipp_clk_prepare_enable(dev->jpg_clk)) != 0)
    {
        pr_err("[%s] Failed: jpeg_ipp_clk_prepare_enable.%d\n", __func__, ret);
        return ret;
    }

    return ret;
}

int hispcpe_powerup(struct hispcpe_s *dev)
{
    int ret = 0, err = 0;

    D("+\n");
    if (!dev) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -EINVAL;
    }

    if (atomic_read(&dev->cpe_supply_refs) > 0) {
        pr_err("[%s] Failed: cpe_supply Opened.0x%x\n", __func__, atomic_read(&dev->cpe_supply_refs));
        return -EBUSY;
    }
/*
    if (!dev->ippdrv) {
        pr_err("[%s] Failed : ippdrv.%pK\n", __func__, dev->ippdrv);
        return -ENOMEM;
    }

    if (!dev->ippdrv->enable_jpgclk) {
        pr_err("[%s] Failed : ippdrv->enable_jpgclk.%pK\n", __func__, dev->ippdrv->enable_jpgclk);
        return -ENOMEM;
    }
*/
    if ((ret = regulator_enable(dev->cpe_supply)) != 0) {
        pr_err("[%s] Failed: cpe regulator_enable.%d\n", __func__, ret);
        return ret;
    }

    if ((ret = ipp_setclk_enable(dev)) != 0)
    {
        pr_err("[%s] Failed: ipp_setclk_enable.%d\n", __func__, ret);
        goto fail_enjgpclk;
    }

  /*  if ((ret = dev->ippdrv->enable_jpgclk(dev->ippdrv, CLKLVL_SVS)) != 0)
    {
        pr_err("[%s] Failed to set ipp powerup rate mode:( %d).\n", __func__, CLKLVL_SVS);
        goto fail_enjgpclk;
    }*/
    
    dev->curr_slam_rate_value = dev->jpgclk_low_frequency;	
    dev->curr_cpe_rate_value = dev->jpgclk_low_frequency;
    dev->ipp_set_rate_value = dev->jpgclk_low_frequency;
  
    if ((ret = hispcpe_cmdlst_clock_enable(dev)) != 0) {
        pr_err("[%s] Failed: hispcpe_cmdlst_clock_enable.%d\n", __func__, ret);
        goto fail_cmdlist;
    }

    if ((ret = hispcpe_irq_config(dev)) != 0) {
        pr_err("[%s] Failed: hispcpe_cmdlst_clock_enable.%d\n", __func__, ret);
        goto fail_irq;
    }

    if ((ret = dataflow_cvdr_global_config(dev)) != 0) {
        pr_err("[%s] Failed: dataflow_cvdr_global_config.%d\n", __func__, ret);
        return ret;
    }

    if (!dev->ippdrv->enable_smmu)
    {
        pr_err("[%s] Failed : ippdrv->enable_smmu.%pK\n", __func__, dev->ippdrv->enable_smmu);
        return -ENOMEM;
    }

    if ((ret = dev->ippdrv->enable_smmu(dev->ippdrv)) != 0)
    {
        pr_err("[%s] Failed : dev->ippdrv->enable_smmu.%d\n", __func__, ret);
        return ret;
    }

    atomic_set(&dev->cpe_supply_refs, 1);
    D("-\n");

    return 0;
fail_irq:
    if ((err = hispcpe_cmdlst_clock_disable(dev)) != 0)
        pr_err("[%s] Failed: hispcpe_cmdlst_clock_disable.%d\n", __func__, err);
fail_cmdlist:
    if ((err = dev->ippdrv->disable_jpgclk(dev->ippdrv, CLKLVL_DISCLOCK)) != 0)
        pr_err("[%s] Failed : dev->ippdrv->enable_jpgclk.%d\n", __func__, err);
fail_enjgpclk:
    if ((err = regulator_disable(dev->cpe_supply)) != 0)
        pr_err("[%s] Failed: cpe regulator_disable.%d\n", __func__, err);

    return ret;
}
EXPORT_SYMBOL(hispcpe_powerup);

static int ipp_setclk_disable(struct hispcpe_s* dev)
{
    int ret = 0;
    jpeg_ipp_clk_disable_unprepare(dev->jpg_clk);

    if ((ret = jpeg_ipp_set_rate(dev->jpg_clk, dev->jpgclk_off_frequency)) != 0)
    {
        pr_err("[%s] Failed: jpeg_ipp_set_rate.%d\n", __func__, ret);
	return ret;
    }
   return 0;
}

int hispcpe_powerdn(struct hispcpe_s *dev)
{
    int ret = 0;

    D("+\n");

    if (atomic_read(&dev->cpe_supply_refs) <= 0) {
        pr_err("[%s] Failed: cpe_supply not Opened.0x%x\n", __func__, atomic_read(&dev->cpe_supply_refs));
        return -EINVAL;
    }
    atomic_set(&dev->cpe_supply_refs, 0);

     if (!dev) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -EINVAL;
    }

    if (!dev->ippdrv) {
        pr_err("[%s] Failed : ippdrv.%pK\n", __func__, dev->ippdrv);
        return -ENOMEM;
    }

    if (!dev->ippdrv->disable_smmu)
    {
        pr_err("[%s] Failed : ippdrv->disable_smmu.%pK\n", __func__, dev->ippdrv->disable_smmu);
        return -ENOMEM;
    }

    if ((ret = dev->ippdrv->disable_smmu(dev->ippdrv)) != 0)
    {
        pr_err("[%s] Failed : dev->ippdrv->disable_smmu.%d\n", __func__, ret);
        return ret;
    }
/*
    if (!dev->ippdrv->disable_jpgclk) {
        pr_err("[%s] Failed : ippdrv->disable_jpgclk.%pK\n", __func__, dev->ippdrv->disable_jpgclk);
        return -ENOMEM;
    }*/

    if ((ret = hispcpe_cmdlst_clock_disable(dev)) != 0)
        pr_err("[%s] Failed: hispcpe_cmdlst_clock_disable.%d\n", __func__, ret);

/*    if ((ret = dev->ippdrv->disable_jpgclk(dev->ippdrv, CLKLVL_DISCLOCK)) != 0)
    {
        pr_err("[%s] Failed to set ipp powerdn rate mode:( %d).\n", __func__, CLKLVL_DISCLOCK);
    }*/

     if ((ret = ipp_setclk_disable(dev)) != 0)
    {
        pr_err("[%s] Failed: ipp_setclk_enable.%d\n", __func__, ret);
        return ret;
    }

    if ((ret = regulator_disable(dev->cpe_supply)) != 0)
        pr_err("[%s] Failed: cpe regulator_disable.%d\n", __func__, ret);

    D("-\n");

    return 0;
}

EXPORT_SYMBOL(hispcpe_powerdn);

int sec_slam_powerdn(struct hispcpe_s *dev)
{
    int ret = 0;
    dev->sec_path = 0;
    D("+\n");
    
    if (atomic_read(&dev->sec_slam_supply_refs) <= 0) {
        pr_err("[%s] Failed: sec_slam_supply not Opened.0x%x\n", __func__, atomic_read(&dev->sec_slam_supply_refs));
        return -EINVAL;
    }

    atomic_set(&dev->sec_slam_supply_refs, 0);

    if (!dev)
    {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -EINVAL;
    }

    if ((ret = hipp_ptable_secmem_deinit()) != 0) {
        pr_err("[%s] Failed: hipp_ptable_secmem_deinit.%d\n", __func__, ret);
    }

    if ((ret = atfhipp_orb_deinit()) != 0) {
        pr_err("[%s] Failed : orb_deinit.%d\n", __func__, ret);
        return ret;
    }

    if (!dev->sec_slamdrv) {
        pr_err("[%s] Failed : sec_slamdrv.%pK\n", __func__, dev->sec_slamdrv);
        return -ENOMEM;
    }

    if (!dev->sec_slamdrv->disable_jpgclk) {
        pr_err("[%s] Failed : sec_slamdrv->disable_jpgclk.%pK\n", __func__, dev->sec_slamdrv->disable_jpgclk);
        return -ENOMEM;
    }

    if ((ret = dev->sec_slamdrv->disable_jpgclk(dev->sec_slamdrv, CLKLVL_DISCLOCK)) != 0) {
        pr_err("[%s] Failed to set sec_slam powerdn rate mode:( %d).\n", __func__, CLKLVL_DISCLOCK);
    }

    if ((ret = regulator_disable(dev->cpe_supply)) != 0)
        pr_err("[%s] Failed: slam regulator_disable.%d\n", __func__, ret);

    D("-\n");

    return 0;
}
EXPORT_SYMBOL(sec_slam_powerdn);

static int hispcpe_mfnr_mcf_clk_enable(struct hispcpe_s *dev)
{
    U_CPE_CRG_CFG0 cfg;

    D("+\n");
    if (!dev) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -EINVAL;
    }

    if (atomic_read(&dev->cpe_supply_refs) <= 0) {
        pr_err("[%s] Failed: cpe_supply not Opened.0x%x\n", __func__, atomic_read(&dev->cpe_supply_refs));
        return -EINVAL;
    }

    if (atomic_read(&dev->cpe_mfnr_refs) > 0) {
        pr_err("[%s] Failed: cpe mfnr mcf Opened.0x%x\n", __func__, atomic_read(&dev->cpe_mfnr_refs));
        return -EINVAL;
    }

    cfg.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP]+SUB_CTRL_CPE_CRG_CFG0_REG));
    cfg.bits.mcf_clken = 1;
    cfg.bits.mfnr_clken = 1;
    writel(cfg.u32, (volatile void __iomem *)(dev->reg[CPE_TOP]+SUB_CTRL_CPE_CRG_CFG0_REG));

    atomic_set(&dev->cpe_mfnr_refs, 1);
    D("-\n");

    return 0;
}

static int hispcpe_mfnr_mcf_clk_disable(struct hispcpe_s *dev)
{
    U_CPE_CRG_CFG0 cfg;

    D("+\n");
    if (!dev) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -EINVAL;
    }

    if (atomic_read(&dev->cpe_supply_refs) <= 0) {
        pr_err("[%s] Failed: cpe_supply not Opened.0x%x\n", __func__, atomic_read(&dev->cpe_supply_refs));
        return -EINVAL;
    }

    if (atomic_read(&dev->cpe_mfnr_refs) <= 0) {
        pr_err("[%s] Failed: cpe mfnr mcf not Opened.0x%x\n", __func__, atomic_read(&dev->cpe_mfnr_refs));
        return -EINVAL;
    }

    cfg.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP]+SUB_CTRL_CPE_CRG_CFG0_REG));
    cfg.bits.mcf_clken = 0;
    cfg.bits.mfnr_clken = 0;
    writel(cfg.u32, (volatile void __iomem *)(dev->reg[CPE_TOP]+SUB_CTRL_CPE_CRG_CFG0_REG));

    atomic_set(&dev->cpe_mfnr_refs, 0);
    D("-\n");

    return 0;
}

static int hispcpe_slam_clk_enable(struct hispcpe_s *dev)
{
    U_SLAM_CRG_CFG0 cfg;

    D("+\n");
    if (!dev) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -EINVAL;
    }
    
    if(0 == dev->sec_path)
    {
      if (atomic_read(&dev->cpe_supply_refs) <= 0) 
      {
        pr_err("[%s] Failed: cpe_supply not Opened.0x%x\n", __func__, atomic_read(&dev->cpe_supply_refs));
        return -EINVAL;
      }
    }
    else if (1 == dev->sec_path)
    {
      if (atomic_read(&dev->sec_slam_supply_refs) <= 0) 
      {
        pr_err("[%s] Failed: sec_slam_supply not Opened.0x%x\n", __func__, atomic_read(&dev->sec_slam_supply_refs));
        return -EINVAL;
      }
    }
    else
    {
      pr_err("[%s] Failed: check sec_path\n", __func__);
    }

    if(0 == dev->sec_path)
    {
      if (atomic_read(&dev->cpe_slam_refs) > 0) 
        {
        pr_err("[%s] Failed: cpe slam Opened.0x%x\n", __func__, atomic_read(&dev->cpe_slam_refs));
        return -EINVAL;
       }
    }
    else if(1 == dev->sec_path)
    {
      if (atomic_read(&dev->sec_slam_refs) > 0) 
        {
        pr_err("[%s] Failed: cpe slam Opened.0x%x\n", __func__, atomic_read(&dev->sec_slam_refs));
        return -EINVAL;
       }
    }
    else
    {
      pr_err("[%s] Failed: check sec_path\n", __func__);
    }
    
    cfg.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP]+SUB_CTRL_SLAM_CRG_CFG0_REG));
    cfg.bits.slam_clken = 1;
    writel(cfg.u32, (volatile void __iomem *)(dev->reg[CPE_TOP]+SUB_CTRL_SLAM_CRG_CFG0_REG));
    
    if(0 == dev->sec_path)
    {
      atomic_set(&dev->cpe_slam_refs, 1);
    }
    else if (1 == dev->sec_path)
    {
     atomic_set(&dev->sec_slam_refs, 1); 
    }
    
    D("-\n");

    return 0;
}

static int hispcpe_slam_clk_disable(struct hispcpe_s *dev)
{
    U_SLAM_CRG_CFG0 cfg;

    D("+\n");
    if (!dev) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -EINVAL;
    }

    if(0 == dev->sec_path)
    {
      if (atomic_read(&dev->cpe_supply_refs) <= 0) 
      {
        pr_err("[%s] Failed: cpe_supply not Opened.0x%x\n", __func__, atomic_read(&dev->cpe_supply_refs));
        return -EINVAL;
      }
    }
    else if (1 == dev->sec_path)
    {
      if (atomic_read(&dev->sec_slam_supply_refs) <= 0) 
      {
        pr_err("[%s] Failed: sec_slam_supply not Opened.0x%x\n", __func__, atomic_read(&dev->sec_slam_supply_refs));
        return -EINVAL;
      }
    }
    else
    {
      pr_err("[%s] Failed: check sec_path\n", __func__);
    }
    

    if(0 == dev->sec_path)
    {
      if (atomic_read(&dev->cpe_slam_refs) <= 0) 
        {
        pr_err("[%s] Failed: cpe slam Opened.0x%x\n", __func__, atomic_read(&dev->cpe_slam_refs));
        return -EINVAL;
       }
    }
    else if(1 == dev->sec_path)
    {
      if (atomic_read(&dev->sec_slam_refs) <= 0) 
        {
        pr_err("[%s] Failed: cpe slam Opened.0x%x\n", __func__, atomic_read(&dev->sec_slam_refs));
        return -EINVAL;
       }
    }
    else
    {
      pr_err("[%s] Failed: check sec_path\n", __func__);
    }
    

    cfg.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP]+SUB_CTRL_SLAM_CRG_CFG0_REG));
    cfg.bits.slam_clken = 0;
    writel(cfg.u32, (volatile void __iomem *)(dev->reg[CPE_TOP]+SUB_CTRL_SLAM_CRG_CFG0_REG));

    if(0 == dev->sec_path)
    {
      atomic_set(&dev->cpe_slam_refs, 0);
    }
    else if (1 == dev->sec_path)
    {
     atomic_set(&dev->sec_slam_refs, 0); 
    }
    
    D("-\n");

    return 0;
}

static int hispcpe_clean_wait_flag(struct hispcpe_s *dev, unsigned int wait_mode)
{
    D("+\n");
    if (!dev) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -EINVAL;
    }

    if (wait_mode == 0) {
        dev->cpe_ready = 0;
    } else if (wait_mode == 1) {
        dev->slam_ready = 0;
    }
    D("-\n");

    return 0;
}

static int hispcpe_wait(struct hispcpe_s *dev,unsigned int wait_mode)
{
    int ret = 0;

    D("+\n");
    if (!dev) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -EINVAL;
    }

    if(wait_mode == 0) {
        if ((ret = wait_event_interruptible_timeout(dev->cpe_wait, dev->cpe_ready, HISP_CPE_TIMEOUT_MS)) < 0) {
            pr_err("[%s] Failed : wait_event_interruptible_timeout.%d\n", __func__, ret);
            return ret;
        }
    } else {
        if ((ret = wait_event_interruptible_timeout(dev->slam_wait, dev->slam_ready, HISP_CPE_TIMEOUT_MS)) < 0) {
            pr_err("[%s] Failed : wait_event_interruptible_timeout.%d\n", __func__, ret);
            return ret;
        }
    }
    D("-\n");

    return 0;
}

int hispcpe_mcf_request(struct hispcpe_s *dev, msg_req_mcf_request_t *ctrl)
{
    int ret = 0;

    D("+\n");
    if (!dev || !ctrl) {
        pr_err("[%s] Failed : dev.%pK, ctrl.%pK\n", __func__, dev, ctrl);
        return -EINVAL;
    }
    pr_err("[%s] Failed : dev.%pK, ctrl.%pK\n", __func__, dev, ctrl);
    pr_err("frame_number = %d", ctrl->frame_number);
    pr_err("mcf_cfg.mode = %d", ctrl->mcf_cfg.mode);
    pr_err("mcf_cfg.uv_pack = %d", ctrl->mcf_cfg.uv_pack);

    if((ret = hispcpe_clean_wait_flag(dev, 0)) < 0) {
        pr_err("[%s] Failed : hispcpe_clean_wait_flag.%d\n", __func__, ret);
        return ret;
    }
    mcf_request_handler(ctrl);
    if((ret = hispcpe_wait(dev, 0)) < 0) {
        pr_err("[%s] Failed : hispcpe_wait.%d\n", __func__, ret);
        return ret;
    }

    mcf_eof_handler();
    D("-\n");

    return 0;
}
EXPORT_SYMBOL(hispcpe_mcf_request);

int hispcpe_mfnr_request(struct hispcpe_s *dev, msg_req_mfnr_request_t *ctrl)
{
    int ret = 0;

    D("+\n");
    if (!dev || !ctrl) {
        pr_err("[%s] Failed : dev.%pK, ctrl.%pK\n", __func__, dev, ctrl);
        return -EINVAL;
    }
    if((ret = hispcpe_clean_wait_flag(dev, 0)) < 0) {
        pr_err("[%s] Failed : hispcpe_clean_wait_flag.%d\n", __func__, ret);
        return ret;
    }
    mfnr_request_handler(ctrl);
    if((ret = hispcpe_wait(dev, 0)) < 0) {
        pr_err("[%s] Failed : hispcpe_wait.%d\n", __func__, ret);
        return ret;
    }

    mfnr_eof_handler();
    D("-\n");

    return 0;
}
EXPORT_SYMBOL(hispcpe_mfnr_request);

int hispcpe_slam_request(struct hispcpe_s *dev, msg_req_slam_request_t *ctrl)
{
    int ret = 0;

    D("+\n");
    if (!dev || !ctrl) {
        pr_err("[%s] Failed : dev.%pK, ctrl.%pK\n", __func__, dev, ctrl);
        return -EINVAL;
    }
    if((ret = hispcpe_clean_wait_flag(dev, 1)) < 0) {
        pr_err("[%s] Failed : hispcpe_clean_wait_flag.%d\n", __func__, ret);
        return ret;
    }
   
    slam_request_handler(ctrl);
    if((ret = hispcpe_wait(dev, 1)) < 0) {
        pr_err("[%s] Failed : hispcpe_wait.%d\n", __func__, ret);
        return ret;
    }
	
    slam_eof_handler(ctrl);	
    D("-\n");

    return 0;
}
EXPORT_SYMBOL(hispcpe_slam_request);

int hispcpe_slamtest_request(struct hispcpe_s *dev, msg_req_slamtest_request_t *ctrl)
{
    int ret = 0;

    D("+\n");
    if (!dev || !ctrl) {
        pr_err("[%s] Failed : dev.%pK, ctrl.%pK\n", __func__, dev, ctrl);
        return -EINVAL;
    }
    if((ret = hispcpe_clean_wait_flag(dev, 1)) < 0) {
        pr_err("[%s] Failed : hispcpe_clean_wait_flag.%d\n", __func__, ret);
        return ret;
    }
    slamtest_request_handler(ctrl);
    if((ret = hispcpe_wait(dev, 1)) < 0) {
        pr_err("[%s] Failed : hispcpe_wait.%d\n", __func__, ret);
        return ret;
    }

    slamtest_eof_handler();
    D("-\n");

    return 0;
}

EXPORT_SYMBOL(hispcpe_slamtest_request);

static int hispcpe_map_kernel(struct hispcpe_s *dev, unsigned long args)
{
    struct iommu_map_format iommu_fmt = {0};
    struct ion_handle *handle = NULL;
    void *virt_addr = NULL;
    struct memory_block_s  pstFrameBuf = { 0 };
    int ret = 0;

    if (args == 0)
    {
        pr_err("[%s] args.0x%lx\n", __func__, args);
        return -EINVAL;
    }

    I("+\n");
    if ((ret = copy_from_user(&pstFrameBuf, (void __user*)args, sizeof(struct memory_block_s))) != 0)
    {
        pr_err("[%s] copy_from_user.%d\n", __func__, ret);
        return -EFAULT;
    }

    if (!dev ) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -EINVAL;
    }

    if (pstFrameBuf.shared_fd < 0) {
        pr_err("[%s] Failed : hare_fd.%d\n", __func__, pstFrameBuf.shared_fd);
        return -EINVAL;
    }

    #if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
        handle = ion_import_dma_buf_fd(dev->ion_client, pstFrameBuf.shared_fd);
    #else
       handle = ion_import_dma_buf(dev->ion_client, pstFrameBuf.shared_fd);
    #endif
	
    if (IS_ERR_OR_NULL(handle)) {
        pr_err("[%s] Failed : failed to create ion handle!\n", __func__);
        return -ENOMEM;
    }

    iommu_fmt.prot = pstFrameBuf.prot;
    if ((ret = ion_map_iommu(dev->ion_client, handle, &iommu_fmt)) != 0) {
        pr_err("Failed : ion_map_iommu.%d", ret);
        goto err_ion_map_iommu;
    }

    if (pstFrameBuf.size != iommu_fmt.iova_size){
        pr_err("hisi_iommu_map_range failed:  %d len 0x%lx\n", pstFrameBuf.size, iommu_fmt.iova_size);
        goto err_ion_map_check;
    }

    virt_addr = ion_map_kernel(dev->ion_client, handle);
    if (NULL == virt_addr) {
        pr_err("[%s] Failed : ion_map_kernel.%pK\n", __func__, virt_addr);
        goto err_ion_map_check;
    }

    dev->handle = handle;
    dev->virt_addr = virt_addr;
    dev->daddr = iommu_fmt.iova_start;

    pr_info("[after map kernel]virt_addr = 0x%lx, da.(0x%lx, 0x%x)",(unsigned long)dev->virt_addr, (unsigned long)iommu_fmt.iova_start, dev->daddr);/*lint !e626 */
    I("-\n");

    return ret;

err_ion_map_check:
    ion_unmap_iommu(dev->ion_client, handle);
err_ion_map_iommu:
    ion_free(dev->ion_client, handle);

    return -ENOMEM;
}

static int hispcpe_unmap_kernel(struct hispcpe_s *dev)
{
    I("+\n");
    if (!dev) {
        pr_err("[%s] Failed : dev.%pK \n", __func__, dev);
        return -EINVAL;
    }

    if (!dev->handle) {
        pr_err("[%s] Failed : dev->handle.%pK \n", __func__, dev->handle);
        return -EINVAL;
    }

   if (!dev->ion_client) {
        pr_err("[%s] Failed : dev->ion_client.%pK \n", __func__, dev->ion_client);
        return -EINVAL;
    }

    ion_unmap_iommu(dev->ion_client, dev->handle);
    ion_unmap_kernel(dev->ion_client, dev->handle);
    ion_free(dev->ion_client, dev->handle);
    dev->handle = NULL;
    I("-\n");

    return 0;
}

int ipp_set_rate(struct hispcpe_s* dev)
{
    unsigned int ret = 0;
    unsigned int curr_max_rate = dev->curr_cpe_rate_value;
 /*
    if (!dev->ippdrv)
    {
        pr_err("[%s] Failed : ippdrv.%pK\n", __func__, dev->ippdrv);
        return -EINVAL;
    }

    if (!dev->ippdrv->enable_jpgclk)
    {
        pr_err("[%s] Failed : ippdrv->enable_jpgclk.%pK\n", __func__, dev->ippdrv->enable_jpgclk);
        return -EINVAL;
    }
*/
    if (dev->curr_slam_rate_value > dev->curr_cpe_rate_value)
    {
        curr_max_rate = dev->curr_slam_rate_value;
    }

    if (curr_max_rate !=  dev->ipp_set_rate_value)
    {
        pr_info("curr_max_rate  = %d\n", curr_max_rate);
        if ((ret = jpeg_ipp_set_rate(dev->jpg_clk, curr_max_rate)) != 0)
        {
            pr_err("[%s] Failed to set ipp  rate mode:( %d).\n", __func__, curr_max_rate);

            if ((ret = jpeg_ipp_set_rate(dev->jpg_clk, dev->jpgclk_low_frequency)) != 0)
            {
                pr_err("[%s] Failed to set ipp  rate mode:( %d).\n", __func__, dev->jpgclk_low_frequency);

                return -EINVAL;
            }

            dev->ipp_set_rate_value = curr_max_rate;
        }
    }
    return 0;
}

static int mcf_process(struct hispcpe_s* dev, unsigned long args)
{
    unsigned int ret = 0;
    msg_req_mcf_request_t* ctrl_mcf = NULL;

    if (args == 0)
    {
        pr_err("[%s] args.0x%lx\n", __func__, args);
        return -EINVAL;
    }

    if (!dev->ippdrv)
    {
        pr_err("[%s] Failed : ippdrv.%pK\n", __func__, dev->ippdrv);
        return -EINVAL;
    }

    if (!dev->ippdrv->lock_usecase)
    {
        pr_err("[%s] Failed : ippdrv->lock_usecase.%pK\n", __func__, dev->ippdrv->lock_usecase);
        return -EINVAL;
    }

    if ((ret = dev->ippdrv->lock_usecase(dev->ippdrv)) != 0)
    {
        pr_err("[%s] Failed : dev->ippdrv->lock_usecase.%d\n", __func__, ret);
        return ret;
    }

    ctrl_mcf = (msg_req_mcf_request_t*)kmalloc(sizeof(msg_req_mcf_request_t), GFP_KERNEL);
    if (NULL == ctrl_mcf)
    {
        pr_err(" fail to kmalloc ctrl_mcf\n");
	goto free_mcf_kmalloc_memory;
    }

    if ((ret = copy_from_user(ctrl_mcf, (void __user*)args, sizeof(msg_req_mcf_request_t))) != 0)
    {
        pr_err("[%s] copy_from_user.%d\n", __func__, ret);
        goto free_mcf_kmalloc_memory;
    }
	
   dev->curr_cpe_rate_value = dev->jpg_clk_rate;

    if ((ret = ipp_set_rate(dev)) != 0)
    {
        pr_err("[%s] Failed : ipp_set_rate.%d\n", __func__, ret);
        goto free_mcf_kmalloc_memory;
    }
	
    if ((ret = hispcpe_mfnr_mcf_clk_enable(dev)) != 0)
    {
        pr_err("[%s] Failed : hispcpe_mfnr_mcf_clk_enable.%d\n", __func__, ret);
        goto free_mcf_kmalloc_memory;
    }

    if ((ret = hispcpe_mcf_request(dev, ctrl_mcf)) != 0)
    {
        pr_err("[%s] Failed : hispcpe_mcf_request.%d\n", __func__, ret);
        goto free_mcf_kmalloc_memory;
    }

    if ((ret = hispcpe_mfnr_mcf_clk_disable(dev)) != 0)
    {
        pr_err("[%s] Failed : hispcpe_mfnr_mcf_clk_disable.%d\n", __func__, ret);
        goto free_mcf_kmalloc_memory;
    }

   dev->curr_cpe_rate_value = dev->jpgclk_low_frequency;

    if ((ret = ipp_set_rate(dev)) != 0)
    {
        pr_err("[%s] Failed : ipp_set_rate.%d\n", __func__, ret);
        goto free_mcf_kmalloc_memory;
    }

free_mcf_kmalloc_memory:
   dev->curr_cpe_rate_value = dev->jpgclk_low_frequency;
    if (ctrl_mcf){
        kfree(ctrl_mcf);
	ctrl_mcf = NULL;
    }

    if ((ret = dev->ippdrv->unlock_usecase(dev->ippdrv)) != 0)
    {
        pr_err("[%s] Failed : dev->ippdrv->unlock_usecase.%d\n", __func__, ret);
        return ret;
    }

    return ret;
}

static int mfnr_process(struct hispcpe_s* dev, unsigned long args)
{
    unsigned int ret = 0;
    msg_req_mfnr_request_t* ctrl_mfnr = NULL;

    if (args == 0)
    {
        pr_err("[%s] args.0x%lx\n", __func__, args);
        return -EINVAL;
    }

    if (!dev->ippdrv)
    {
        pr_err("[%s] Failed : ippdrv.%pK\n", __func__, dev->ippdrv);
        return -EINVAL;
    }

    if (!dev->ippdrv->lock_usecase)
    {
        pr_err("[%s] Failed : ippdrv->lock_usecase.%pK\n", __func__, dev->ippdrv->lock_usecase);
        return -EINVAL;
    }

    if ((ret = dev->ippdrv->lock_usecase(dev->ippdrv)) != 0)
    {
        pr_err("[%s] Failed : dev->ippdrv->lock_usecase.%d\n", __func__, ret);
        return ret;
    }

    ctrl_mfnr = (msg_req_mfnr_request_t*)kmalloc(sizeof(msg_req_mfnr_request_t), GFP_KERNEL);
   if (NULL == ctrl_mfnr)
    {
        pr_err(" fail to kmalloc ctrl_mfnr\n");
	goto free_mfnr_kmalloc_memory;
    }

    if ((ret = copy_from_user(ctrl_mfnr, (void __user*)args, sizeof(msg_req_mfnr_request_t))) != 0)
    {
        pr_err("[%s] copy_from_user.%d\n", __func__, ret);
        goto free_mfnr_kmalloc_memory;
    }

   dev->curr_cpe_rate_value = dev->jpg_clk_rate;

    if ((ret = ipp_set_rate(dev)) != 0)
    {
        pr_err("[%s] Failed : ipp_set_rate.%d\n", __func__, ret);
        goto free_mfnr_kmalloc_memory;
    }
	
    if ((ret = hispcpe_mfnr_mcf_clk_enable(dev)) != 0)
    {
        pr_err("[%s] Failed : hispcpe_mfnr_mcf_clk_enable.%d\n", __func__, ret);
        goto free_mfnr_kmalloc_memory;
    }

    if ((ret = hispcpe_mfnr_request(dev, ctrl_mfnr)) != 0)
    {
        pr_err("[%s] Failed : hispcpe_mfnr_request.%d\n", __func__, ret);
        goto free_mfnr_kmalloc_memory;
    }

    if ((ret = hispcpe_mfnr_mcf_clk_disable(dev)) != 0)
    {
        pr_err("[%s] Failed : hispcpe_mfnr_mcf_clk_disable.%d\n", __func__, ret);
        goto free_mfnr_kmalloc_memory;
    }
	
   dev->curr_cpe_rate_value = dev->jpgclk_low_frequency;

    if ((ret = ipp_set_rate(dev)) != 0)
    {
        pr_err("[%s] Failed : ipp_set_rate.%d\n", __func__, ret);
        goto free_mfnr_kmalloc_memory;
    }

free_mfnr_kmalloc_memory:
   dev->curr_cpe_rate_value = dev->jpgclk_low_frequency;
    if (ctrl_mfnr){
        kfree(ctrl_mfnr);
	ctrl_mfnr = NULL;
    }

    if ((ret = dev->ippdrv->unlock_usecase(dev->ippdrv)) != 0)
    {
        pr_err("[%s] Failed : dev->ippdrv->unlock_usecase.%d\n", __func__, ret);
        return ret;
    }

    return ret ;
}

static int slam_test_process(struct hispcpe_s* dev, unsigned long args)
{
    unsigned int ret = 0;
    msg_req_slamtest_request_t* ctrl_slamtest = NULL;

    if (args == 0)
    {
        pr_err("[%s] args.0x%lx\n", __func__, args);
        return -EINVAL;
    }

    ctrl_slamtest = (msg_req_slamtest_request_t*)kmalloc(sizeof(msg_req_slamtest_request_t), GFP_KERNEL);
    if (NULL == ctrl_slamtest)
    {
        pr_err(" fail to kmalloc ctrl_slamtest\n");
	goto free_slamtest_kmalloc_memory;
    }

    if ((ret = copy_from_user(ctrl_slamtest, (void __user*)args, sizeof(msg_req_slamtest_request_t))) != 0)
    {
        pr_err("[%s] copy_from_user.%d\n", __func__, ret);
        goto free_slamtest_kmalloc_memory;
    }

    if ((ret = hispcpe_slam_clk_enable(dev)) != 0)
    {
        pr_err("[%s] Failed : hispcpe_slam_clk_enable.%d\n", __func__, ret);
        goto free_slamtest_kmalloc_memory;
    }

    if ((ret = hispcpe_slamtest_request(dev, ctrl_slamtest)) != 0)
    {
        pr_err("[%s] Failed : hispcpe_slam_request.%d\n", __func__, ret);
        goto free_slamtest_kmalloc_memory;
    }

    if ((ret = hispcpe_slam_clk_disable(dev)) != 0)
    {
        pr_err("[%s] Failed : hispcpe_slam_clk_disable.%d\n", __func__, ret);
        goto free_slamtest_kmalloc_memory;
    }

free_slamtest_kmalloc_memory:
    if (ctrl_slamtest){
        kfree(ctrl_slamtest);
	ctrl_slamtest = NULL;
    }

    return ret ;
}

static int slam_process(struct hispcpe_s* dev, unsigned long args)
{
    unsigned int ret = 0;
    unsigned int slam_rate_index = 0;
    msg_req_slam_request_t* ctrl_slam = NULL;
    
    if (args == 0)
    {
        pr_err("[%s] args.0x%lx\n", __func__, args);
        return -EINVAL;
    }

    if (!dev->ippdrv)
    {
        pr_err("[%s] Failed : ippdrv.%pK\n", __func__, dev->ippdrv);
        return -EINVAL;
    }

    if (!dev->ippdrv->lock_usecase)
    {
        pr_err("[%s] Failed : ippdrv->lock_usecase.%pK\n", __func__, dev->ippdrv->lock_usecase);
        return -EINVAL;
    }

    if ((ret = dev->ippdrv->lock_usecase(dev->ippdrv)) != 0)
    {
        pr_err("[%s] Failed : dev->ippdrv->lock_usecase.%d\n", __func__, ret);
        return ret;
    }

    ctrl_slam = (msg_req_slam_request_t*)kmalloc(sizeof(msg_req_slam_request_t), GFP_KERNEL);
    if (NULL == ctrl_slam)
    {
        pr_err(" fail to kmalloc ctrl_slam\n");
	goto free_slam_kmalloc_memory;
    }
    if ((ret = copy_from_user(ctrl_slam, (void __user*)args, sizeof(msg_req_slam_request_t))) != 0)
    {
        pr_err("[%s] copy_from_user.%d\n", __func__, ret);
         goto free_slam_kmalloc_memory;
    }

    slam_rate_index = ctrl_slam->slam_rate_value;
    if (slam_rate_index > SLAM_RATE_INDEX_MAX )
    {
        pr_err("[%s] check slam rate index.%d\n", __func__, slam_rate_index);
        goto free_slam_kmalloc_memory;
    }
    else
    {
       dev->curr_slam_rate_value = CLK_RATE_MAP[slam_rate_index];
    }

     if ((ret = ipp_set_rate(dev)) != 0)
    {
        pr_err("[%s] Failed : slam_set_rate.%d\n", __func__, ret);
        goto free_slam_kmalloc_memory;	
    }

    if ((ret = hispcpe_slam_clk_enable(dev)) != 0)
    {
        pr_err("[%s] Failed : hispcpe_slam_clk_enable.%d\n", __func__, ret);
        goto free_slam_kmalloc_memory;
    }

    if ((ret = hispcpe_slam_request(dev, ctrl_slam)) != 0)
    {
        pr_err("[%s] Failed : hispcpe_slam_request.%d\n", __func__, ret);
        goto free_slam_kmalloc_memory;
    }

    if ((ret = hispcpe_slam_clk_disable(dev)) != 0)
    {
        pr_err("[%s] Failed : hispcpe_slam_clk_disable.%d\n", __func__, ret);
        goto free_slam_kmalloc_memory;
    }

free_slam_kmalloc_memory:
    if (ctrl_slam){
        kfree(ctrl_slam);
	ctrl_slam = NULL;
    }

    if ((ret = dev->ippdrv->unlock_usecase(dev->ippdrv)) != 0)
    {
        pr_err("[%s] Failed : dev->ippdrv->unlock_usecase.%d\n", __func__, ret);
        return ret;
    }

    return ret ;
}

static int sec_slam_smmu_int(struct hispcpe_s* dev)
{
    unsigned int ret = 0;

    if (!dev) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -EINVAL;
    }

    if (!dev->sec_slamdrv) {
        pr_err("[%s] Failed : sec_slamdrv.%pK\n", __func__, dev->sec_slamdrv);
        return -EINVAL;
    }

    if (!dev->sec_slamdrv->enable_smmu)
    {
        pr_err("[%s] Failed : sec_slamdrv->enable_smmu.%pK\n", __func__, dev->sec_slamdrv->enable_smmu);
        return -EINVAL;
    }

    if (!dev->sec_slamdrv->setsid_smmu)
    {
        pr_err("[%s] Failed : sec_slamdrv->setsid_smmu.%pK\n", __func__, dev->sec_slamdrv->setsid_smmu);
        return -EINVAL;
    }

    if ((ret = dev->sec_slamdrv->enable_smmu(dev->sec_slamdrv)) != 0)
    {
        pr_err("[%s] Failed : dev->sec_slamdrv->enable_smmu.%d\n", __func__, ret);
        return ret;
    }

    if ((ret = dev->sec_slamdrv->setsid_smmu(dev->sec_slamdrv, SLAM_BI_SID, TRUSTED)) != 0)
    {
        pr_err("[%s] Failed : sec_slamdrv->setsid_smmu.%pK, sid.%d\n", __func__, dev->sec_slamdrv->setsid_smmu, SLAM_BI_SID);
        return ret;
    }

    if ((ret = dev->sec_slamdrv->setsid_smmu(dev->sec_slamdrv, SLAM_STAT_SID, TRUSTED)) != 0)
    {
        pr_err("[%s] Failed : sec_slamdrv->setsid_smmu.%pK, sid.%d\n", __func__, dev->sec_slamdrv->setsid_smmu, SLAM_STAT_SID);
        return ret;
    }
   return ret;
}

static int sec_slam_int(struct hispcpe_s* dev, msg_req_slam_request_t* ctrl_slam)
{
    unsigned int ret = 0;
    if (TRUSTED ==  ctrl_slam->secure_flag)
    {
        if ((ret = sec_slam_smmu_int(dev)) != 0)
        {
            pr_err("[%s] Failed : sec_slam_smmu_int.%d\n", __func__, ret);
            return ret;
        }

        if (!dev->sec_slamdrv->shrf2da)
        {
            pr_err("[%s] Failed : sec_slamdrv->shrf2da.%pK\n", __func__, dev->sec_slamdrv->shrf2da);
            return -EINVAL;
        }

        ctrl_slam->streams[0][0].buf = dev->sec_slamdrv->shrf2da(dev->sec_slamdrv, ctrl_slam->streams[0][0].buf, &g_share_fd_size[BI_SHARE_FD]);

        if (ctrl_slam->streams[0][0].buf == 0)
        {
            E("[%s] Failed : hipp_shrf2da.BI_SHARE_FD\n", __func__);
            return -ENOMEM;
        }

        ctrl_slam->streams[0][2].buf = dev->sec_slamdrv->shrf2da(dev->sec_slamdrv, ctrl_slam->streams[0][2].buf, &g_share_fd_size[STAT_SHARE_FD]);

        if (ctrl_slam->streams[0][2].buf == 0)
        {
            E("[%s] Failed : hipp_shrf2da.STAT_SHARE_FD\n", __func__);
            return -ENOMEM;
        }
    }
    else if (NONTRUS ==  ctrl_slam->secure_flag)
    {
        if (!dev->ippdrv) {
            pr_err("Failed : ippdrv.%pK\n", dev->ippdrv);
            return -EINVAL;
        }

        if (!dev->ippdrv->enable_smmu)
        {
            pr_err("Failed : ippdrv->enable_smmu.%pK \n", dev->ippdrv->enable_smmu);
            return -EINVAL;
        }

        if ((ret = dev->ippdrv->enable_smmu(dev->ippdrv)) != 0)
        {
            pr_err("[%s] Failed : dev->ippdrv->enable_smmu.%d\n", __func__, ret);
            return ret;
        }
    }

   return ret;
}

static int sec_slam_smmu_deint(struct hispcpe_s* dev)
{
    unsigned int ret = 0;

    if (!dev)
    {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -EINVAL;
    }

    if (!dev->sec_slamdrv)
    {
        pr_err("[%s] Failed : sec_slamdrv.%pK\n", __func__, dev->sec_slamdrv);
        return -EINVAL;
    }

    if (!dev->sec_slamdrv->enable_smmu)
    {
        pr_err("[%s] Failed : sec_slamdrv->enable_smmu.%pK\n", __func__, dev->sec_slamdrv->enable_smmu);
        return -EINVAL;
    }

    if (!dev->sec_slamdrv->setsid_smmu)
    {
        pr_err("[%s] Failed : sec_slamdrv->setsid_smmu.%pK\n", __func__, dev->sec_slamdrv->setsid_smmu);
        return -EINVAL;
    }
  
    if ((ret = dev->sec_slamdrv->disable_smmu(dev->sec_slamdrv)) != 0)
    {
        pr_err("[%s] Failed : dev->sec_slamdrv->enable_smmu.%d\n", __func__, ret);
        return ret;
    }

    if ((ret = dev->sec_slamdrv->setsid_smmu(dev->sec_slamdrv, SLAM_BI_SID, NONTRUS)) != 0)
    {
        pr_err("[%s] Failed : sec_slamdrv->setsid_smmu.%pK\n", __func__, dev->sec_slamdrv->setsid_smmu);
        return ret;
    }

    if ((ret = dev->sec_slamdrv->setsid_smmu(dev->sec_slamdrv, SLAM_STAT_SID, NONTRUS)) != 0)
    {
        pr_err("[%s] Failed : sec_slamdrv->setsid_smmu.%pK\n", __func__, dev->sec_slamdrv->setsid_smmu);
        return ret;
    }
  return ret;
}

static int sec_slam_deint(struct hispcpe_s* dev, msg_req_slam_request_t* ctrl_slam)
{
    unsigned int ret = 0;
    if (TRUSTED ==  ctrl_slam->secure_flag)
    {
         if ((ret = sec_slam_smmu_deint(dev)) != 0)
        {
            pr_err("[%s] Failed : sec_slam_smmu_int.%d\n", __func__, ret);
            return ret;
        }

        hipp_trusted_unmap(dev->sec_slamdrv, ctrl_slam->streams[0][0].buf, g_share_fd_size[BI_SHARE_FD]);
        hipp_trusted_unmap(dev->sec_slamdrv, ctrl_slam->streams[0][2].buf, g_share_fd_size[STAT_SHARE_FD]);
    }
    else if (NONTRUS ==  ctrl_slam->secure_flag) {
        if (!dev->ippdrv) {
            pr_err("Failed : ippdrv.%pK\n", dev->ippdrv);
            return -EINVAL;
        }

        if (!dev->ippdrv->enable_smmu) {
            pr_err("Failed : ippdrv->enable_smmu.%pK \n", dev->ippdrv->enable_smmu);
            return -EINVAL;
        }

        if ((ret = dev->ippdrv->disable_smmu(dev->ippdrv)) != 0)
        {
            pr_err("[%s] Failed : dev->ippdrv->enable_smmu.%d\n", __func__, ret);
            return ret;
        }
    }
    return ret;
}

static int sec_slam_process(struct hispcpe_s* dev, unsigned long args)
{
    unsigned int ret = 0;
    msg_req_slam_request_t* ctrl_slam = NULL;

    if (args == 0 ) {
        pr_err("[%s] args.0x%lx\n", __func__, args);
        return -EINVAL;
    }

    if (!dev->sec_slamdrv) {
        pr_err("[%s] Failed : sec_slamdrv.%pK\n", __func__, dev->sec_slamdrv);
        return -EINVAL;
    }

    if (!dev->sec_slamdrv->lock_usecase) {
        pr_err("[%s] Failed : sec_slamdrv->lock_usecase.%pK\n", __func__, dev->sec_slamdrv->lock_usecase);
        return -EINVAL;
    }

    if ((ret = dev->sec_slamdrv->lock_usecase(dev->sec_slamdrv)) != 0) {
        pr_err("[%s] Failed : dev->sec_slamdrv->lock_usecase.%d\n", __func__, ret);
        return ret;
    }

    ctrl_slam = (msg_req_slam_request_t*)kmalloc(sizeof(msg_req_slam_request_t), GFP_KERNEL);
    if (NULL == ctrl_slam)
    {
        pr_err(" fail to kmalloc ctrl_slam\n");
        goto free_slam_kmalloc_memory;
    }
    if ((ret = copy_from_user(ctrl_slam, (void __user*)args, sizeof(msg_req_slam_request_t))) != 0)
    {
        pr_err("[%s] copy_from_user.%d\n", __func__, ret);
        goto free_slam_kmalloc_memory;
    }
   
    if ((ret = sec_slam_int(dev, ctrl_slam)) != 0)
    {
        pr_err("[%s] Failed : sec_slam_int.%d\n", __func__, ret);
        goto free_slam_kmalloc_memory;
    }

    if ((ret = hispcpe_slam_clk_enable(dev)) != 0)
    {
        pr_err("[%s] Failed : hispcpe_slam_clk_enable.%d\n", __func__, ret);
        goto free_slam_kmalloc_memory;
    }

    if ((ret = hispcpe_slam_request(dev, ctrl_slam)) != 0)
    {
        pr_err("[%s] Failed : hispcpe_slam_request.%d\n", __func__, ret);
        goto free_slam_kmalloc_memory;
    }

    if ((ret = hispcpe_slam_clk_disable(dev)) != 0)
    {
        pr_err("[%s] Failed : hispcpe_slam_clk_disable.%d\n", __func__, ret);
        goto free_slam_kmalloc_memory;
    }
 
    if ((ret = sec_slam_deint(dev, ctrl_slam)) != 0)
    {
        pr_err("[%s] Failed : sec_slam_int.%d\n", __func__, ret);
        goto free_slam_kmalloc_memory;
    }

free_slam_kmalloc_memory:
    if (ctrl_slam) {
        kfree(ctrl_slam);
        ctrl_slam = NULL;
    }

    if ((ret = dev->sec_slamdrv->unlock_usecase(dev->sec_slamdrv)) != 0)
    {
        pr_err("[%s] Failed : dev->sec_slamdrv->unlock_usecase.%d\n", __func__, ret);
        return ret;
    }

    return ret;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wframe-larger-than="
static long hispcpe_ioctl(struct file *filp, unsigned int cmd, unsigned long args)
{
    unsigned int pw_flag = 0;   
    struct hispcpe_s *dev = NULL;
    int ret = 0;

    D("+\n");
    if ((dev = get_cpedev_bymisc((struct miscdevice *)filp->private_data)) == NULL) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -EINVAL;
    }

    if (!dev->initialized) {
        pr_err("[%s] Failed : CPE Device Not Exist.%d\n", __func__, dev->initialized);
        return -ENXIO;
    }

    pr_info("[%s] cmd.0x%x\n", __func__, cmd);

    switch (cmd) {
        case HISP_CPE_PWRUP:
            if (args == 0) {
                pr_err("[%s] args.0x%lx\n", __func__, args);
                return -EINVAL;
            }

            if ((ret = copy_from_user(&pw_flag, (void __user *)args, sizeof(unsigned int))) != 0) {
                pr_err("[%s] copy_from_user.%d\n", __func__, ret);
                return -EFAULT;
            }

            I(" info : HISP_CPE_PWRUP.pw_flag.%d\n",pw_flag);
            
            if (0 == pw_flag)
            {
                if ((ret = hispcpe_powerup(dev)) != 0)
                {
                    pr_err("[%s] Failed : hispcpe_powerup.%d\n", __func__, ret);
                    return -EINVAL;
                }
            }
            else if (1 == pw_flag )
            {
                if ((ret = sec_slam_powerup(dev)) != 0)
                {
                    pr_err("[%s] Failed : sec_slam_powerup.%d\n", __func__, ret);
                    return -EINVAL;
                }
            }
            else
            {
                pr_err(" Failed : check up layer pw_flag.%d\n",pw_flag);
                return -EINVAL;
            }
            break;
   
        case HISP_CPE_PWRDN:
           
           if(0 == dev->sec_path)
            {
                if ((ret = hispcpe_powerdn(dev)) != 0)
                {
                    pr_err("[%s] Failed : hispcpe_powerdn.%d\n", __func__, ret);
                    return -EINVAL;
                }
            }
            else if (1 == dev->sec_path)
            {  
                if ((ret = sec_slam_powerdn(dev)) != 0)
                {
                    pr_err("[%s] Failed : sec_slam_powerdn.%d\n", __func__, ret);
                    return -EINVAL;
                }
            }
            else
            {
                pr_err(" Failed :check sec_path.%d\n", dev->sec_path);
                return -EINVAL;
            }
            
            break;
        
        case HISP_CPE_MCF_REQ:
            if ((ret = mcf_process(dev, args)) != 0) {
                pr_err("[%s] Failed : hispcpe_mcf_request.%d\n", __func__, ret);
                return ret;
            }		
            break;

        case HISP_CPE_MFNR_REQ:		
            if ((ret = mfnr_process(dev, args)) != 0) {
                pr_err("[%s] Failed : hispcpe_mcf_request.%d\n", __func__, ret);
                return ret;
            }	
            break;

        case HISP_CPE_SLAM_REQ:
            pr_info("[%s] HISP_CPE_SLAM_REQ\n", __func__);		
            if (0 == dev->sec_path)
            {
                if ((ret = slam_process(dev, args)) != 0)
                {
                    pr_err("[%s] Failed : slam_process.%d\n", __func__, ret);
                    return ret;
                }
            }
            else if (1 == dev->sec_path)
            {
                if ((ret = sec_slam_process(dev, args)) != 0)
                {
                    pr_err("[%s] Failed : sec_slam_process.%d\n", __func__, ret);
                    return ret;
                }
            }
            else
            {
                pr_err(" Failed : check sec_path \n");
                return -EINVAL;
            }
            break;

       case HISP_CPE_SLAMTEST_REQ:
            if ((ret = slam_test_process(dev, args)) != 0) {
                pr_err("[%s] Failed : hispcpe_slam_request.%d\n", __func__, ret);
                return ret;
            }
            break;

        case HISP_CPE_MAP_BUF:
            pr_info("[%s] cmd.HISP_CPE_MAP_BUF\n", __func__);
            if ((ret = hispcpe_map_kernel(dev, args)) != 0) {
                pr_err("[%s] Failed : hispcpe_map_kernel.%d\n", __func__, ret);
                return ret;
            }

            if ((ret = cpe_init_memory()) != 0) {
                pr_err("[%s] Failed : cpe_init_memory.%d\n", __func__, ret);
                return ret;
            }
            break;

        case HISP_CPE_UNMAP_BUF:
            if (args == 0) {
                pr_err("[%s] args.0x%lx\n", __func__, args);
                return -EINVAL;
            }

            if ((ret = hispcpe_unmap_kernel(dev)) != 0) {
                pr_err("[%s] Failed : hispcpe_umap_kernel.%d\n", __func__, ret);
                return -EINVAL;
            }
            break;
			
        default :
            pr_err("[%s] Failed : Invalid cmd.0x%x\n", __func__, cmd);
            break;
    }

    D("-\n");

    return 0;
}
#pragma GCC diagnostic pop

static int hispcpe_open(struct inode *inode, struct file *filp)
{
    struct hispcpe_s *dev = NULL;

    D("+\n");
    if ((dev = get_cpedev_bymisc((struct miscdevice *)filp->private_data)) == NULL) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -EINVAL;
    }

    if (!dev->initialized) {
        pr_err("[%s] Failed : CPE Device Not Exist.%d\n", __func__, dev->initialized);
        return -ENXIO;
    }

    if (atomic_read(&dev->open_refs) != 0) {
        pr_err("[%s] Failed: Opened, open_refs.0x%x\n", __func__, atomic_read(&dev->open_refs));
        return -EBUSY;
    }

    atomic_inc(&dev->open_refs);
    D("-\n");

    return 0;
}

static int hispcpe_release(struct inode *inode, struct file *filp)
{
    struct hispcpe_s *dev = NULL;
    unsigned int ret = 0;

    D("+\n");
    if ((dev = get_cpedev_bymisc((struct miscdevice *)filp->private_data)) == NULL) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -EINVAL;
    }

    if (!dev->initialized) {
        pr_err("[%s] Failed : CPE Device Not Exist.%d\n", __func__, dev->initialized);
        return -ENXIO;
    }

    if (atomic_read(&dev->open_refs) <= 0) {
        pr_err("[%s] Failed: Closed, open_refs.0x%x\n", __func__, atomic_read(&dev->open_refs));
        return -EBUSY;
    }

    atomic_set(&dev->open_refs, 0);
    if (atomic_read(&dev->cpe_supply_refs) > 0)
    {
        pr_err("[%s] Failed: cpe_supply Opened.0x%x\n", __func__, atomic_read(&dev->cpe_supply_refs));

        if ((ret = hispcpe_powerdn(dev)) != 0)
        {
            pr_err("[%s] Failed : hispcpe_powerdn.%d\n", __func__, ret);
            return -EINVAL;
        }
    }

    if (atomic_read(&dev->sec_slam_supply_refs) > 0)
    {
        pr_err("[%s] Failed: sec_slam_supply Opened.0x%x\n", __func__, atomic_read(&dev->sec_slam_supply_refs));

        if ((ret = sec_slam_powerdn(dev)) != 0)
        {
            pr_err("[%s] Failed : sec_slam_powerdn.%d\n", __func__, ret);
            return -EINVAL;
        }
    }

    D("-\n");

    return 0;
}

static ssize_t hispcpe_show(struct device *pdev, struct device_attribute *attr, char *buf)
{
    char *s = buf;
    D("+\n");
    D("-\n");
    return (ssize_t)(s - buf);
}

static ssize_t hispcpe_store(struct device *pdev, struct device_attribute *attr, const char *buf, size_t count)
{
    D("+\n");
    D("-\n");
    return (ssize_t)count;
}

static DEVICE_ATTR(hispcpe, (S_IRUGO | S_IWUSR | S_IWGRP), hispcpe_show, hispcpe_store);

static struct file_operations hispcpe_fops = {
    .owner          = THIS_MODULE,
    .open           = hispcpe_open,
    .release        = hispcpe_release,
    .unlocked_ioctl = hispcpe_ioctl,
};

static struct miscdevice hispcpe_miscdev = {
    .minor  = MISC_DYNAMIC_MINOR,
    .name   = KBUILD_MODNAME,
    .fops   = &hispcpe_fops,
};

static int hispcpe_register_irq(struct hispcpe_s *dev)
{
    int i = 0, ret = 0;

    D("+\n");
    for (i = 0; i < HISP_MIN(MAX_HISP_CPE_IRQ, dev->irq_num); i ++) {/*lint !e574 */
        if ((ret = request_irq(dev->irq[i], hispcpe_irq_handler, 0, "HISP_CPE_IRQ", (void *)dev)) != 0) {
            pr_err("[%s] Failed : %d.request_irq.%d\n", __func__, i, ret);
            return ret;
        }
        pr_info("[%s] Hisp CPE %d.IRQ.%d, handler.%pK\n", __func__, i, dev->irq[i], hispcpe_irq_handler);
    }
    D("-\n");

    return 0;
}

static int hispcpe_ioremap_reg(struct hispcpe_s *dev)
{
    struct device *device = NULL;
    int i = 0;

    D("+\n");
    device = &dev->pdev->dev;
    for (i = 0; i < HISP_MIN(MAX_HISP_CPE_REG, dev->reg_num); i ++) {/*lint !e574 */
        dev->reg[i] = devm_ioremap_resource(device, dev->r[i]);
        if (!dev->reg[i]) {
            pr_err("[%s] Failed : %d.devm_ioremap_resource.%pK\n", __func__, i, dev->reg[i]);
            return -ENOMEM;
        }
        pr_info("[%s] Hisp CPE %d.Resource.%pK.(n.%s, s.0x%llx, e.0x%llx, f.0x%lx) > %pK\n", __func__, i, dev->r[i],
            dev->r[i]->name, (unsigned long long)dev->r[i]->start, (unsigned long long)dev->r[i]->end, dev->r[i]->flags, dev->reg[i]);
    }
    D("-\n");

    return 0;
}

static int hispcpe_resource_init(struct hispcpe_s *dev)
{
    int ret = 0;

    if ((ret = hispcpe_register_irq(dev)) != 0) {
        pr_err("[%s] Failed : hispcpe_register_irq.%d\n", __func__, ret);
        return ret;
    }

    if ((ret = hispcpe_ioremap_reg(dev)) != 0) {
        pr_err("[%s] Failed : hispcpe_ioremap_reg.%d\n", __func__, ret);
        return ret;
    }

    return 0;
}

static int hispcpe_getdts_pwrclk(struct hispcpe_s *dev)
{
    struct device *device = NULL;
    struct device_node *np;
    unsigned int ret = 0;

    D("+\n");
    if (!dev) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -ENXIO;
    }
  
    if (!dev->pdev) {
        pr_err("[%s] Failed : pdev.%pK\n", __func__, dev->pdev);
        return -ENXIO;
    }

    device = &dev->pdev->dev;
    dev->cpe_supply = devm_regulator_get(device, "ipp-cpe");
    if (IS_ERR(dev->cpe_supply)) {
        pr_err("[%s] Failed : CPE devm_regulator_get.%pK\n", __func__, dev->cpe_supply);
        return -EINVAL;
    }
    pr_info("[%s] Hisp CPE cpe_supply.%pK\n", __func__, dev->cpe_supply);

    dev->jpg_clk = devm_clk_get(device, "clk_jpg_func");
    if (IS_ERR(dev->jpg_clk)) {
        pr_err("get jpg_clk failed");
        return -EINVAL;
    }
    np = device->of_node;

    if (NULL == np)
    {
        pr_err("%s: of node NULL.", __func__);
        return -EINVAL;
    }
  
    ret = of_property_read_u32(np, "clock-value", &dev->jpg_clk_rate);

    if (ret)
    {
        pr_err("ipp get rate failed, ret:%d", ret);
        return -EINVAL;
    }

    ret = of_property_read_u32(np, "clock-low-frequency", &dev->jpgclk_low_frequency);

    if (ret)
    {
        pr_err("ipp get jpgclk_low_frequency failed, ret:%d", ret);
        return -EINVAL;
    }

    ret = of_property_read_u32(np, "power-off-frequency", &dev->jpgclk_off_frequency);

    if (ret)
    {
        pr_err("ipp get jpgclk_off_frequency failed, ret:%d", ret);
        return -EINVAL;
    }

    pr_err("ipp get jpgclk frequency 1:%d 2:%d 3:%d", dev->jpg_clk_rate, dev->jpgclk_low_frequency, dev->jpgclk_off_frequency);
    I("-\n");

    return 0;
}

int hispcpe_get_irq(int index)
{
    struct device_node *np = NULL;
    char *name = DTS_NAME_HISI_IPP;
    int irq = 0;

    np = of_find_compatible_node(NULL, NULL, name);
    if (!np) {
        pr_err("[%s] Failed : %s.of_find_compatible_node.%pK\n", __func__, name, np);
        return -ENXIO;
    }

    irq = irq_of_parse_and_map(np, index);
    if (!irq) {
        pr_err("[%s] Failed : irq_of_parse_and_map.%d\n", __func__, irq);
        return -ENXIO;
    }

    pr_info("%s: comp.%s, cpe irq.%d.\n", __func__, name, irq);
    return irq;
}

static int hispcpe_getdts_irq(struct hispcpe_s *dev)
{
    struct device *device = NULL;
    int i = 0, irq = 0, ret = 0;

    D("+\n");
    if (!dev) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -ENXIO;
    }

    if (!dev->pdev) {
        pr_err("[%s] Failed : pdev.%pK\n", __func__, dev->pdev);
        return -ENXIO;
    }

    device = &dev->pdev->dev;
    if ((ret = of_property_read_u32(device->of_node, "irq-num", (unsigned int *)(&dev->irq_num))) < 0 ) {
        pr_err("[%s] Failed: irq-num of_property_read_u32.%d\n", __func__, ret);
        return -EINVAL;
    }
    pr_info("[%s] Hisp CPE irq_num.%d\n", __func__, dev->irq_num);

    for (i = 0; i < HISP_MIN(MAX_HISP_CPE_IRQ, dev->irq_num); i ++) {/*lint !e574 */
        //if ((irq = platform_get_irq(dev->pdev, i)) <= 0) {
        if ((irq = hispcpe_get_irq(i)) <= 0) {
            pr_err("[%s] Failed : platform_get_irq.%d\n", __func__, irq);
            return -EINVAL;
        }
        dev->irq[i] = irq;
        pr_info("[%s] Hisp CPE %d.IRQ.%d\n", __func__, i, dev->irq[i]);
    }
    I("-\n");

    return 0;
}

static int hispcpe_getdts_reg(struct hispcpe_s *dev)
{
    struct device *device = NULL;
    int i = 0, ret = 0;

    D("+\n");
    if (!dev) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -ENXIO;
    }

    if (!dev->pdev) {
        pr_err("[%s] Failed : pdev.%pK\n", __func__, dev->pdev);
        return -ENXIO;
    }

    device = &dev->pdev->dev;
    if ((ret = of_property_read_u32(device->of_node, "reg-num", (unsigned int *)(&dev->reg_num))) < 0 ) {
        pr_err("[%s] Failed: reg-num of_property_read_u32.%d\n", __func__, ret);
        return -EINVAL;
    }
    pr_info("[%s] Hisp CPE reg_num.%d\n", __func__, dev->reg_num);

    for (i = 0; i < HISP_MIN(MAX_HISP_CPE_REG, dev->reg_num); i ++) {/*lint !e574 */
        if ((dev->r[i] = platform_get_resource(dev->pdev, IORESOURCE_MEM, i)) == NULL) {
            pr_err("[%s] Failed : platform_get_resource.%pK\n", __func__, dev->r[i]);
            return -ENXIO;
        }
        pr_info("[%s] Hisp CPE %d.Resource.%pK.(n.%s, s.0x%llx, e.0x%llx, f.0x%lx)\n", __func__, i, dev->r[i],
            dev->r[i]->name, (unsigned long long)dev->r[i]->start, (unsigned long long)dev->r[i]->end, dev->r[i]->flags);
    }
    I("-\n");

    return 0;
}

static int hispcpe_getdts(struct hispcpe_s *dev)
{
    int ret = 0;

    if ((ret = hispcpe_getdts_pwrclk(dev)) != 0) {
        pr_err("[%s] Failed : hispcpe_getdts_pwrclk.%d\n", __func__, ret);
        return ret;
    }

    if ((ret = hispcpe_getdts_irq(dev)) != 0) {
        pr_err("[%s] Failed : hispcpe_getdts_irq.%d\n", __func__, ret);
        return ret;
    }

    if ((ret = hispcpe_getdts_reg(dev)) != 0) {
        pr_err("[%s] Failed : hispcpe_getdts_reg.%d\n", __func__, ret);
        return ret;
    }

    return 0;
}

static int hispcpe_attach_misc(struct hispcpe_s *dev, struct miscdevice *mdev)
{
    I("+\n");
    if (!dev || !mdev) {
        pr_err("[%s] Failed : dev.%pK, mdev.%pK\n", __func__, dev, mdev);
        return -EINVAL;
    }

    dev->miscdev = hispcpe_miscdev;
    I("-\n");

    return 0;
}

unsigned int get_cpe_addr_da(void)
{
    if(hispcpe_dev == NULL)
    {
        pr_err("[%s], NONE cpe_mem_info!\n", __func__);
        return 0;
    }

    return hispcpe_dev->daddr;
}

void *get_cpe_addr_va(void)
{
    if(hispcpe_dev == NULL)
    {
        pr_err("[%s], NONE cpe_mem_info!\n", __func__);
        return 0;
    }

    return hispcpe_dev->virt_addr;
}
//lint -save -e454 -e455
int ippdev_lock(void)
{
    I("+\n");
    if(hispcpe_dev == NULL)
    {
        pr_err("[%s], NONE cpe_mem_info!\n", __func__);
        return -1;
    }

    mutex_lock(&hispcpe_dev->lock);
    I("-\n");
    return 0;
}

int ippdev_unlock(void)
{
    I("+\n");
    if(hispcpe_dev == NULL)
    {
        pr_err("[%s], NONE cpe_mem_info!\n", __func__);
        return -1;
    }

    mutex_unlock(&hispcpe_dev->lock);
    I("-\n");
    return 0;
}
//lint -restore

static int hispcpe_probe(struct platform_device *pdev)
{
    struct hispcpe_s *dev = NULL;
    int ret = 0;

    I("+\n");

    if ((dev = (struct hispcpe_s *)kzalloc(sizeof(struct hispcpe_s), GFP_KERNEL)) == NULL) {
        pr_err("[%s] Failed : kzalloc\n", __func__);
        return -ENOMEM;
    }

    dev->pdev = pdev;
    platform_set_drvdata(pdev, dev);

    if ((ret = hispcpe_getdts(dev)) != 0) {
        pr_err("[%s] Failed : hispcpe_getdts.%d\n", __func__, ret);
        goto free_dev;
    }

    if ((ret = hispcpe_resource_init(dev)) != 0) {
        pr_err("[%s] Failed : hispcpe_resource_init.%d\n", __func__, ret);
        goto free_dev;
    }

    dev->initialized = 0;
    init_waitqueue_head(&dev->cpe_wait);
    init_waitqueue_head(&dev->slam_wait);
    hispcpe_attach_misc(dev, &hispcpe_miscdev);

    if (!(dev->ippdrv = hipp_register(HISI_CPE_UNIT, NONTRUS))) {
        pr_err("[%s] Failed : hipp_register.%pK\n", __func__, dev->ippdrv);
        goto free_dev;
    }
	
     if (!(dev->sec_slamdrv = hipp_register(HISI_ORB_UNIT, TRUSTED))) {
        pr_err("[%s] Failed : sec_slam_register.%pK\n", __func__, dev->sec_slamdrv);
        goto free_dev;
    }
	 
    dev->ion_client = hisi_ion_client_create("hisp_ion_client");
    if (IS_ERR(dev->ion_client)) {
        pr_err("[%s] Failed : hisi_ion_client_create.%pK\n", __func__, dev->ion_client);
        goto free_dev;
    }

    if ((ret = misc_register((struct miscdevice *)&dev->miscdev)) != 0) {
        pr_err("[%s] Failed : misc_register.%d\n", __func__, ret);
        goto clean_client;
    }

    if ((ret = device_create_file(dev->miscdev.this_device, &dev_attr_hispcpe)) != 0) {
        pr_err("[%s] Faield : hispcpe device_create_file.%d\n", __func__, ret);
        goto clean_misc;
    }

    atomic_set(&dev->open_refs, 0);
    atomic_set(&dev->cpe_supply_refs, 0);
    atomic_set(&dev->sec_slam_supply_refs, 0);
    atomic_set(&dev->sec_slam_refs, 0);
    atomic_set(&dev->cpe_mfnr_refs, 0);
    atomic_set(&dev->cpe_slam_refs, 0);
    mutex_init(&dev->lock);
    dev->initialized = 1;
    hispcpe_dev = dev;
   dev->sec_path = 0;
   dev->sec_req = 0;
    I("-\n");

    return 0;

clean_misc:
    misc_deregister(&dev->miscdev);
clean_client:
    ion_client_destroy(dev->ion_client);
free_dev:
    kfree(dev);

    return ret;
}

static int hispcpe_remove(struct platform_device *pdev)
{
    struct hispcpe_s *dev = NULL;

    I("+\n");
    if ((dev = (struct hispcpe_s *)platform_get_drvdata(pdev)) == NULL) {
        pr_err("[%s] Failed : platform_get_drvdata, dev.%pK.pdev.%pK\n", __func__, dev, pdev);
        return -ENODEV;
    }

    misc_deregister(&dev->miscdev);
    atomic_set(&dev->open_refs, 0);
    atomic_set(&dev->cpe_supply_refs, 0);
    atomic_set(&dev->cpe_mfnr_refs, 0);
    atomic_set(&dev->cpe_slam_refs, 0);

    if (dev->ion_client) {
        I("[%s] : deinit memory pool ion client\n", __func__);
        ion_client_destroy(dev->ion_client);
    }

    if (hipp_unregister(HISI_CPE_UNIT))
        pr_err("[%s] Failed : hipp_unregister\n", __func__);
	
   if (hipp_unregister(HISI_ORB_UNIT))
        pr_err("[%s] Failed : sec_slam_unregister\n", __func__);
   
    if (dev->ippdrv)
        dev->ippdrv = NULL;
	
    if (dev->sec_slamdrv)
        dev->sec_slamdrv = NULL;
	
    dev->initialized = 0;
    kfree(dev);
    dev = NULL;
    I("-\n");

    return 0;
}

static struct of_device_id hisiipp_of_id[] = {
    {.compatible = DTS_NAME_HISI_IPP},
    {}
};

static struct platform_driver hispcpe_pdrvr = {
    .probe          = hispcpe_probe,
    .remove         = hispcpe_remove,
    .driver         = {
        .name           = "hisiipp",
        .owner          = THIS_MODULE,
        .of_match_table = of_match_ptr(hisiipp_of_id),
    },
};

module_platform_driver(hispcpe_pdrvr);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Hisilicon ISP CPE Driver");
MODULE_AUTHOR("c00261891 <chentao20@huawei.com>");

/*lint +e21 +e846 +e514 +e778 +e866 +e84 +e429 +e613 +e668*/
