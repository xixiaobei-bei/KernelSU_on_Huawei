/*
 * Hisi ISP FD
 *
 * Copyright (c) 2017 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "hispfd.h"
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <asm/io.h>
#include <linux/delay.h>
#include "smmureg.h"

static int kmsgcat_mask = (ERROR_BIT);
//static int kmsgcat_mask = (INFO_BIT | ERROR_BIT | DEBUG_BIT);

/*lint -e21 -e846 -e514 -e778 -e866 -e84*/
module_param_named(kmsgcat_mask, kmsgcat_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);
/*lint +e21 +e846 +e514 +e778 +e866 +e84*/

//lint -save -e778 -e666



const struct hispfd_pic_type_s hispfd_PropConfig_big_common[NUM_PRE_CFG] = {
        {0x1001f410,ADDR_FDAI_PROP_CFG1},
        {0x100402cd,ADDR_FDAI_PROP_CFG2},
        {0x00002800,ADDR_FDAI_PROP_CFG4},
        {0x0a000000,ADDR_FDAI_PROP_CFG5},
        {0x2ff02600,ADDR_FDAI_PROP_CFG6},
        {0x2d002900,ADDR_FDAI_PROP_CFG7},
        {0x27d02e00,ADDR_FDAI_PROP_CFG8},
        {0x00002600,ADDR_FDAI_PROP_CFG10},
        {0x00d20d07,ADDR_FDAI_PROP_CFG13},
        {0x00d2341a,ADDR_FDAI_PROP_CFG14},
        {0x26002b00,ADDR_FDAI_PROP_CFG15},
        {0x28002800,ADDR_FDAI_PROP_CFG16},
        {0x0c030133,ADDR_FDAI_PROP_CFG17},
        {0x000000cd,ADDR_FDAI_PROP_CFG18},
        {0x00000246,ADDR_FDAI_PROP_CFG19},
        {0x0000011b,ADDR_FDAI_PROP_CFG20},
        {0x00000001,ADDR_FDAI_RDMA_BLOCK_ROT_EN},
        {0x00000001,ADDR_FDAI_WDMA_BLOCK_ROT_EN},

        {0x00000000,0x00000000}
};

const unsigned int anchor_type[2] = {
    15,     //default  ,only supported 15 just now
    9 
};


const unsigned int dtype_value[DTYPE_MAX] = {
    0x1,  // signed_16
    0x0   // unsigned_8
};


struct hispfd_pic_type_s g_stPreCfg_small[] = {
    {0x00000000,ADDR_FDAI_WDMA_RAW_EN},
    {0x00000000,ADDR_FDAI_WDMA_BLOCK_ROT_EN},
    {0x00000000,ADDR_FDAI_WDMA_DEBUG1},
    {0x00000000,0x00000000}
};

struct hispfd_s *g_sthispfdcfg;


static long hispfd_ioctl(struct file *filp, unsigned int cmd, unsigned long args)
{
    int ret = 0;
    return ret;
}
static int hispfd_open(struct inode *inode, struct file *filp)
{
    return -EINVAL;
}

static int hispfd_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static ssize_t hispfd_show(struct device *pdev, struct device_attribute *attr, char *buf)
{
    char *s = buf;
    D("+\n");
    D("-\n");
    return (ssize_t)(s - buf);
}

static ssize_t hispfd_store(struct device *pdev, struct device_attribute *attr, const char *buf, size_t count)
{
    D("+\n");
    D("-\n");
    return (ssize_t)count;
}

// declare dev_attr_hispfd
static DEVICE_ATTR(hispfd, (S_IRUGO | S_IWUSR | S_IWGRP), hispfd_show, hispfd_store);

static struct file_operations hispfd_fops = {
    .owner          = THIS_MODULE,
    .open           = hispfd_open,
    .release        = hispfd_release,
    .unlocked_ioctl = hispfd_ioctl,
};


static int hispfd_resource_init(struct hispfd_s *dev)
{
    struct device *device = NULL;
    int i = 0;
    
    D("+\n");
    
    device = &dev->pdev->dev;
    
    for (i = 0; i < (int)(HISP_MIN(MAX_HISPFD_REG, dev->reg_num)); i ++) 
    {
        dev->reg[i] = devm_ioremap(device, dev->r[i]->start, resource_size(dev->r[i]));
        if (!dev->reg[i]) {
            pr_err("[%s] Failed : %d.devm_ioremap.%pK\n", __func__, i, dev->reg[i]);
            return -ENOMEM;
        }
    }
    
    D("-\n");
    
    return 0;
}

static void hispfd_resource_deinit(struct hispfd_s *dev)
{
    struct device *device = NULL;
    int i = 0;

    D("+\n");
    device = &dev->pdev->dev;
    
    for (i = 0; i < (int)(HISP_MIN(MAX_HISPFD_REG, dev->reg_num)); i ++) 
    {
        if(NULL != dev->reg[i])
        {
            devm_iounmap(device, dev->reg[i]);
            dev->reg[i] = 0;
        }
    }
    
    D("-\n");

    return ;
}

static int hispfd_getdts_irq(struct hispfd_s *dev)
{
    struct device *device = NULL;
    int i = 0, irq = 0, ret = 0;

    D("+\n");
    
    device = &dev->pdev->dev;
    if ((ret = of_property_read_u32(device->of_node, "irq-num", (unsigned int *)(&dev->irq_num))) < 0 ) {
        pr_err("[%s] Failed: irq-num of_property_read_u32.%d\n", __func__, ret);
        return -EINVAL;
    }

    for (i = 0; i < (int)(HISP_MIN(MAX_HISPFD_IRQ, dev->irq_num)); i ++) {
        if ((irq = platform_get_irq(dev->pdev, i)) <= 0) {
            pr_err("[%s] Failed : platform_get_irq.%d\n", __func__, irq);
            return -EINVAL;
        }
        dev->irq[i] = irq;
    }
    D("-\n");

    return 0;
}

static int hispfd_getdts_reg(struct hispfd_s *dev)
{
    struct device *device = NULL;
    int i = 0, ret = 0;

    D("+\n");
    
    device = &dev->pdev->dev;
    if ((ret = of_property_read_u32(device->of_node, "reg-num", (unsigned int *)(&dev->reg_num))) < 0 ) {
        pr_err("[%s] Failed: reg-num of_property_read_u32.%d\n", __func__, ret);
        return -EINVAL;
    }

    for (i = 0; i < (int)(HISP_MIN(MAX_HISPFD_REG, dev->reg_num)); i ++) {
        if ((dev->r[i] = platform_get_resource(dev->pdev, IORESOURCE_MEM, i)) == NULL) {
            pr_err("[%s] Failed : platform_get_resource.%pK\n", __func__, dev->r[i]);
            return -ENXIO;
        }
    }
    
    D("-\n");

    return 0;
}

static int hispfd_getdts_pwrclk(struct hispfd_s *dev)
{
    struct device *device = NULL;
    int ret = 0;
    struct device_node *np;
    unsigned int len = 0;
    struct property *prop = NULL;

    D("+\n");

    device = &dev->pdev->dev;
    
    dev->hfd_media1_supply = devm_regulator_get(device, "hisi-media");
    if (IS_ERR(dev->hfd_media1_supply)) {
        pr_err("[%s] Failed : HFD devm_regulator_get hisi-media .%pK\n", __func__, dev->hfd_media1_supply);
        return -EINVAL;
    }
    
    dev->hfd_supply = devm_regulator_get(device, "isp-hfd");
    if (IS_ERR(dev->hfd_supply)) {
        pr_err("[%s] Failed : HFD devm_regulator_get isp-hfd .%pK\n", __func__, dev->hfd_supply);
        return -EINVAL;
    }

    //get clk parameters
    dev->hfdclk = devm_clk_get(device, "clk_fdai_func");
    if (IS_ERR(dev->hfdclk)) {
        pr_err("get clk failed");
        return -EINVAL;
    }
    
    dev->jpgclk = devm_clk_get(device, "clk_jpg_func");
    if (dev->jpgclk == NULL) {
        pr_err("get jpg clk failed");
        return -EINVAL;
    }

    np = device->of_node;
    if (NULL == np) {
        pr_err("%s: of node NULL.", __func__);
        return -EINVAL;
    }

    prop = of_find_property(device->of_node, "clock_frequency", NULL);
    if (!prop) {
        return -EINVAL;
    }
    
    if (!prop->value) {
        return -ENODATA;
    }
    
    len = ((unsigned int)prop->length) /sizeof(unsigned int);
    if(len > HIFD_CLK_NUM)
    {
        pr_err("[%s] Failed : len %d.\n", __func__, len);
        return -1;
    }
    
    if (of_property_read_u32_array(device->of_node, "clock_frequency", dev->clk_rate, len)) {
        pr_err("[%s] Failed : of_property_read_u32_array clock_frequency.\n", __func__);
        return -1;
    }

    ret = of_property_read_u32(np, "clock_poweroff_freq", &dev->poweroff_clk_rate);
    if (ret) {
        pr_err("get clock_poweroff_freq failed, ret:%d", ret);
        return -EINVAL;
    }

    D("-\n");

    return 0;
}


static int hispfd_getdts_pre(struct hispfd_s *dev)
{
    struct device *device = NULL;
    struct property *prop = NULL;
    unsigned int len = 0;

    D("+\n");

    device = &dev->pdev->dev;

    prop = of_find_property(device->of_node, "fd_pre_reg", NULL);
    if (!prop) {
        return -EINVAL;
    }
    if (!prop->value) {
        return -ENODATA;
    }
    
    len = ((unsigned int)prop->length) /sizeof(uint32_t);
    if(len > FD_PRE_MAX_NUM)
    {
        pr_err("[%s] Failed : len %d > FD_PRE_MAX_NUM %d\n", __func__, len, FD_PRE_MAX_NUM);
        return -1;
    }
    
    if (of_property_read_u32_array(device->of_node, "fd_pre_reg", dev->pre_reg, len)) {
        pr_err("[%s] Failed : of_property_read_u32_array fd_pre_reg.\n", __func__);
        return -1;
    }

    if (of_property_read_u32_array(device->of_node, "fd_pre_val", dev->pre_val, len)) {
        pr_err("[%s] Failed : of_property_read_u32_array fd_pre_val.\n", __func__);
        return -1;
    }

    D("-\n");

    return 0;
}

static int hispfd_getdts(struct hispfd_s *dev)
{
    int ret = 0;

    if ((ret = hispfd_getdts_pwrclk(dev)) != 0) {
        pr_err("[%s] Failed : hispfd_getdts_pwrclk.%d\n", __func__, ret);
        return ret;
    }

    if ((ret = hispfd_getdts_irq(dev)) != 0) {
        pr_err("[%s] Failed : hispfd_getdts_interrupt.%d\n", __func__, ret);
        return ret;
    }

    if ((ret = hispfd_getdts_reg(dev)) != 0) {
        pr_err("[%s] Failed : hispfd_getdts_register.%d\n", __func__, ret);
        return ret;
    }

    if ((ret = hispfd_getdts_pre(dev)) != 0) {
        pr_err("[%s] Failed : hispfd_getdts_pre.%d\n", __func__, ret);
        return ret;
    }

    return 0;
}


static int hispfd_get_pteaddr(struct hispfd_s *dev)
{
    struct iommu_domain_data *info = NULL;

    I("+\n");
    if ((dev->domain = hisi_ion_enable_iommu(NULL)) == NULL) {
        pr_err("[%s] Failed : hisi_ion_enable_iommu.%pK\n", __func__, dev->domain);
        return -ENODEV;
    }

    if ((info = (struct iommu_domain_data *)dev->domain->priv) == NULL) {
        pr_err("[%s] Failed : info.%pK\n",__func__, info);
        return -ENODEV;
    }

    dev->pteaddr = info->phy_pgd_base;
    I("-\n");

    return 0;
}


static int hispfd_probe(struct platform_device *pdev)
{
    struct hispfd_s *dev = NULL;
    int ret = 0;

    I("+\n");
    if ((dev = (struct hispfd_s *)kzalloc(sizeof(struct hispfd_s), GFP_KERNEL)) == NULL) {
        pr_err("[%s] Failed : kzalloc.%pK\n", __func__, dev);
        return -ENOMEM;
    }

    g_sthispfdcfg = dev;

    dev->initialized = 0;
    dev->pdev = pdev;
    platform_set_drvdata(pdev, dev);

    if ((ret = hispfd_getdts(dev)) != 0) {
        pr_err("[%s] Failed : hispfd_getdts.%d\n", __func__, ret);
        return ret;
    }

    if ((ret = hispfd_resource_init(dev)) != 0) {
        pr_err("[%s] Failed : hispfd_resource_init.%d\n", __func__, ret);
        return ret;
    }

    init_waitqueue_head(&dev->wait);

    if ((ret = hispfd_get_pteaddr(dev)) < 0) {
        pr_err("[%s] Failed : hispfd_get_pteaddr.%d\n", __func__, ret);
        return ret;
    }

    dev->miscdev.minor  = MISC_DYNAMIC_MINOR;
    dev->miscdev.name   = KBUILD_MODNAME;
    dev->miscdev.fops   = &hispfd_fops;

    if ((ret = misc_register((struct miscdevice *)&dev->miscdev)) != 0) {
        pr_err("[%s] Failed : misc_register.%d\n", __func__, ret);
        return ret;
    }

    if ((ret = device_create_file(dev->miscdev.this_device, &dev_attr_hispfd)) != 0) {
        pr_err("[%s] Faield : hispfd device_create_file.%d\n", __func__, ret);
        return ret;
    }

    atomic_set(&dev->open_refs, 1);

    sema_init(&dev->hifd_sem, 1);
    
    
    dev->pwrupflag = 0;
    dev->initialized = 1;
    dev->switchflag = 0;
    
    I("-\n");

    return ret;
}

static int hispfd_remove(struct platform_device *pdev)
{
    struct hispfd_s *dev = NULL;

    I("+\n");
    if ((dev = (struct hispfd_s *)platform_get_drvdata(pdev)) == NULL) {
        pr_err("[%s] Failed : platform_get_drvdata, dev.%pK.pdev.%pK\n", __func__, dev, pdev);
        return -ENODEV;
    }
    
    hispfd_resource_deinit(dev);

    misc_deregister(&dev->miscdev);
    
    atomic_set(&dev->open_refs, 0);
    
    dev->initialized = 0;
    dev->pwrupflag = 0;
    dev->switchflag = 0;
    
    kfree(dev);
    dev = NULL;
    
    I("-\n");

    return 0;
}

static struct of_device_id hispfd_of_id[] = {
    {.compatible = DTS_NAME_HISPFD},
    {}
};

static struct platform_driver hispfd_pdrvr = {
    .probe          = hispfd_probe,
    .remove         = hispfd_remove,
    .driver         = {
        .name           = "hispfd",
        .owner          = THIS_MODULE,
        .of_match_table = of_match_ptr(hispfd_of_id),
    },
};

//lint -restore

module_platform_driver(hispfd_pdrvr);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Hisilicon ISP HiFD Driver");
MODULE_AUTHOR("c00326366 <cuijunqiang@huawei.com>");
