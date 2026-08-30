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
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/hisi-iommu.h>
#include <linux/list.h>
#include <linux/hisi/hipp.h>
#include "smmureg.h"
#include "securec.h"


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a)/(sizeof((a)[0])))
#endif

static int kmsgcat_mask = (ERROR_BIT);

/*lint -e21 -e846 -e514 -e778 -e866 -e84*/
module_param_named(kmsgcat_mask, kmsgcat_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);
/*lint +e21 +e846 +e514 +e778 +e866 +e84*/

//lint -save -e778 -e666



struct hispfd_s *g_sthispfdcfg = NULL;

extern int ipp_top_fd_switch_lock(void);
extern void ipp_top_fd_switch_unlock(void);

static void hispfd_debug(struct hispfd_s *dev)
{
    int i = 0;
    unsigned int value;
    
    E("in\n");
    
    value = readl((volatile void __iomem *)(dev->reg[HFD_REG] + FD_REG_CAU_ID_PC_OFFSET));
    printk("0x104-CAU ID PC: 0x%x.\n", value);
    value = readl((volatile void __iomem *)(dev->reg[HFD_REG] + FD_REG_CAU_ID_PC_OFFSET));
    printk("0x104-CAU ID PC: 0x%x.\n", value);

    writel(0x01, (volatile void __iomem *)(dev->reg[HFD_REG] + FD_REG_DEBUG_MODE_OFFSET));
    
    for(i = 0; i < 0x0c20; i+=0x40)
    {
        printk("hispfd addr 0x%04x:0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x.\n", i,
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x4)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x8)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0xc)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x10)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x14)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x18)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x1c)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x20)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x24)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x28)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x2c)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x30)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x34)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x38)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x3c)));
    }

    for(i = 0x1000; i < 0x2000; i+=0x40)
    {
        printk("imem addr 0x%04x:0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x.\n", i,
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x4)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x8)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0xc)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x10)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x14)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x18)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x1c)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x20)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x24)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x28)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x2c)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x30)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x34)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x38)),
            readl((volatile void __iomem *)(dev->reg[HFD_REG] + i+0x3c)));
    }

    writel(0x00, (volatile void __iomem *)(dev->reg[HFD_REG] + FD_REG_DEBUG_MODE_OFFSET));
    
    writel(0x01, (volatile void __iomem *)(dev->reg[HFD_REG] + FD_REG_CLEAR_EN_OFFSET));
    writel(0x00, (volatile void __iomem *)(dev->reg[HFD_REG] + FD_REG_CLEAR_EN_OFFSET));
}


/* first write 0x0 then write 0x3 to kick the hardware */
static inline void hispfd_hw_kick(struct hispfd_s *dev)
{
    D("hispfd_hw_kick");

    writel_relaxed(0x00, (volatile void __iomem *)(dev->reg[HFD_REG] + FD_REG_START_OFFSET));
    
    // set fd start and clean finish status
    writel(0x03, (volatile void __iomem *)(dev->reg[HFD_REG] + FD_REG_START_OFFSET));
}


static int hispfd_reg_check(struct hispfd_s *dev, struct hispfd_reg_s *pstreg)
{
    if((NULL == pstreg) || (NULL == dev))
    {
        return -EFAULT;
    }

    if(pstreg->index >= HISP_MIN(MAX_HISPFD_REG, dev->reg_num))
    {
        E("Failed : streg.index %d.\n", pstreg->index);
        return -EINVAL;
    }
    
    if(pstreg->offset % sizeof(unsigned int))
    {
        E(" Failed : streg.offset %d.\n",  pstreg->offset);
        return -EINVAL;
    }

    if(pstreg->offset >= resource_size(dev->r[pstreg->index]))
    {
        E(" Failed : streg.offset %d.\n",  pstreg->offset);
        return -EINVAL;
    }

    return 0;
}


int hispfd_store_reg(struct hispfd_s *dev, struct hispfd_start_burst *start_burst)
{
    int ret = 0;
    unsigned int size, i;

    size = sizeof(struct hispfd_start_st) * start_burst->num;
    
    if((NULL == start_burst->start_tbl) || (0 == size))
    {
        E("start_burst->num is 0.\n");
        return -EINVAL;
    }

    ret = copy_from_user(&dev->start_tbl[0], (void __user *)(start_burst->start_tbl), size);
    if (ret < 0) 
    {
        E(" copy_from_user.%d\n",  ret);
        return -EFAULT;
    }

    dev->start_idx = 0;
    dev->start_num = start_burst->num;

    for(i = 0; i < dev->start_num; i++)
    {
        if(dev->start_tbl[i].reg_num > FD_COMM_REG_MAX)
        {
            E("start_tbl[%d]->reg_num(%d) > FD_COMM_REG_MAX(%d).\n", i, dev->start_tbl[i].reg_num, FD_COMM_REG_MAX);
            return -EINVAL;
        }

        size = dev->start_tbl[i].reg_num * sizeof(struct hispfd_fd_reg_s);
        
        if((NULL == dev->start_tbl[i].reg) || (0 == size))
        {
            E("dev->start_tbl[%d].reg or num is null.\n", i);
            return -EINVAL;
        }

        ret = copy_from_user(&(dev->pre_reg[i][0]), (void __user *)(dev->start_tbl[i].reg), size);
        if (ret < 0) 
        {
            E(" copy_from_user.%d\n",  ret);
            return -EFAULT;
        }
    }

    return 0;
}


static int hispfd_pre_reg_config(struct hispfd_s *dev)
{
    unsigned int i, index;
    struct hispfd_start_st *hifd_start;

    index = dev->start_idx++;

    hifd_start = &dev->start_tbl[index];

    if(hifd_start->reg_num > FD_COMM_REG_MAX)
    {
        E(" hifd_start->reg_num(%d) > FD_COMM_REG_MAX(%d).\n",  hifd_start->reg_num, FD_COMM_REG_MAX);
        return -EINVAL;
    }
    
    for(i = 0; i < hifd_start->reg_num; i++)
    {
        if((dev->pre_reg[index][i].offset % sizeof(unsigned int))
            || (dev->pre_reg[index][i].offset >= resource_size(dev->r[HFD_REG])))
        {
            E(" Failed : streg.offset %d.\n",  dev->pre_reg[index][i].offset);
            return -EINVAL;
        }
        
        D("write reg offset 0x%08x, value 0x%08x\n", dev->pre_reg[index][i].offset, dev->pre_reg[index][i].value);
    
        writel(dev->pre_reg[index][i].value, (volatile void __iomem *)(dev->reg[HFD_REG] + dev->pre_reg[index][i].offset));
    }
    
    return 0;
}

static irqreturn_t hispfd_irq_handler(int irq, void *devid)
{
    struct hispfd_s *dev = NULL;
    unsigned int value;

    if (((dev = (struct hispfd_s *)devid) == NULL) || (!dev->initialized))
    {
        E(" Failed : dev.%pK\n",  dev);
        return IRQ_NONE;
    }

    if(irq == dev->irq[SMMU_IRQ])
    {
        return IRQ_NONE;
    }

    value = readl((volatile void __iomem *)(dev->reg[HFD_REG] + FD_REG_INTS_OFFSET));   // read intr status 
    writel(0xff, (volatile void __iomem *)(dev->reg[HFD_REG] + FD_REG_INTS_OFFSET));    // clear intr status

    /* abnormal interrupt */
    if(unlikely(value & (FDAI_INTS_PC_OVERFLOW |FDAI_INTS_RDMA_ERROR | FDAI_INTS_WDMA_ERROR)))
    {
        dev->hfd_ready = 0;
        wake_up_interruptible(&dev->wait);

        writel(0x00, (volatile void __iomem *)(dev->reg[HFD_REG] + FD_REG_START_OFFSET));

        E(": int reg 0x%x.\n",  value);

        return IRQ_NONE;
    }

    if(dev->start_idx == dev->start_num) //the last configuration, just wakeup thread
    {
        dev->hfd_ready = 1;
        wake_up_interruptible(&dev->wait);

        writel(0x00, (volatile void __iomem *)(dev->reg[HFD_REG] + FD_REG_START_OFFSET));
    }
    else //continue to start next process
    {

        hispfd_pre_reg_config(dev);
        
        hispfd_hw_kick(dev);

    }

    return IRQ_HANDLED;
}

int hispfd_set_clk(struct hispfd_s *dev, unsigned int level)
{
    int ret = 0;
    unsigned int i;
    unsigned int freq;

    for(i = (level+1); i > HIFD_CLK_LEVEL_LOW; i--)
    {
        freq = dev->clk_rate[i-1];

        if ((ret = clk_set_rate(dev->hfdclk, freq)) != 0)
        {
            E(" Failed: clk_set_rate.%d\n",  ret);
            continue;
        }
        
        if ((ret = clk_prepare_enable(dev->hfdclk)) != 0)
        {
            E(" Failed: clk_prepare_enable hfdclk.%d\n",  ret);
            continue;
        }
        
        if ((ret = clk_prepare_enable(dev->jpgclk)) != 0)
        {
            E(" Failed: clk_prepare_enable jpgclk.%d\n",  ret);
            goto clk_err;
        }

        return 0;  // set clock success

clk_err:
        clk_disable_unprepare(dev->hfdclk);
        
        E(" set clock frequent %d.\n",  dev->poweroff_clk_rate);
    
        if ((ret = clk_set_rate(dev->hfdclk, dev->poweroff_clk_rate)) != 0) {
            E(" Failed: clk_set_rate.%d\n",  ret);
        }
    }
    
    return ret;
}


static void hispfd_clock_disable(struct hispfd_s *dev)
{
    int ret = 0;
    
    D("+\n");

    clk_disable_unprepare(dev->jpgclk);
    
    clk_disable_unprepare(dev->hfdclk);
    
    if ((ret = clk_set_rate(dev->hfdclk, dev->poweroff_clk_rate)) != 0)
    {
        E(" Failed: clk_set_rate.%d\n",  ret);
        return ;
    }
    D("-\n");
    
    return ;
}

void open_power_gate(struct hispfd_s *dev)
{
    writel_relaxed(0xfffffffe, (volatile void __iomem *)(dev->reg[HFD_REG] + FD_REG_HIFD_CLK_SEL_OFFSET));
    writel_relaxed(0xffffffff, (volatile void __iomem *)(dev->reg[HFD_REG] + FD_REG_HIFD_CLK_EN_OFFSET));

    writel_relaxed(0xffffffe0, (volatile void __iomem *)(dev->reg[HFD_REG] + ADDR_FDAI_MODULE_CLK_SEL));
    writel_relaxed(0xffffffff, (volatile void __iomem *)(dev->reg[HFD_REG] + ADDR_FDAI_MODULE_CLK_EN));

    writel_relaxed(0xfffffff8, (volatile void __iomem *)(dev->reg[HFD_REG] + ADDR_FDAI_CH_CLK_SEL));
    writel_relaxed(0xffffffff, (volatile void __iomem *)(dev->reg[HFD_REG] + ADDR_FDAI_CH_CLK_EN));
    
    writel_relaxed(0x0000000f, (volatile void __iomem *)(dev->reg[HFD_REG] + ADDR_FDAI_PRE_SCF_CH_CORE_GT));
    
    writel(0x00000000, (volatile void __iomem *)(dev->reg[HFD_REG] + FD_REG_MEM_DEEP_SLP_OFF));
}


void hifd_top_cfg_on(struct hispfd_s *dev)
{
    unsigned int value;

    // smmu master clock off
    value = readl((volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_TOP_AXI_CFG0));
    value &= 0xffffffef;
    writel_relaxed(value, (volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_TOP_AXI_CFG0));

    // smmu master reset
    value = readl((volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_DMA_CRG_CFG1));
    value |= 0x04;
    writel_relaxed(value, (volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_DMA_CRG_CFG1));

    // HiFD axi/apb/func clock off
    value = readl((volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_HIFD_CRG_CFG0));
    value &= 0xfffffff8;
    writel_relaxed(value, (volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_HIFD_CRG_CFG0));
    
    // HiFD axi/apb/func reset
    value = readl((volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_HIFD_CRG_CFG1));
    value |= 0x07;
    writel_relaxed(value, (volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_HIFD_CRG_CFG1));
    
    // HiFD axi/apb/func clear reset
    value = readl((volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_HIFD_CRG_CFG1));
    value &= 0xfffffff8;
    writel_relaxed(value, (volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_HIFD_CRG_CFG1));
    
    // HiFD axi/apb/func clock on
    value = readl((volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_HIFD_CRG_CFG0));
    value |= 0x07;
    writel_relaxed(value, (volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_HIFD_CRG_CFG0));

    // smmu master clear reset
    value = readl((volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_DMA_CRG_CFG1));
    value &= 0xfffffffb;
    writel_relaxed(value, (volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_DMA_CRG_CFG1));

    // smmu master clock on
    value = readl((volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_TOP_AXI_CFG0));
    value |= 0x15;
    writel(value, (volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_TOP_AXI_CFG0));
}

void hispfd_intr_init(struct hispfd_s *dev)
{
    D("+\n");

    writel_relaxed(0xff, (volatile void __iomem *)(dev->reg[HFD_REG] + FD_REG_INTS_OFFSET));    // clear intr status
    writel(FD_INTR_MASK, (volatile void __iomem *)(dev->reg[HFD_REG] + FD_REG_INT_MASK_OFFSET)); // set interrupt mask

    D("-\n");
}

static void hispfd_switch(struct hispfd_s *dev)
{
    unsigned int value;
    D("+\n");
    value = readl((volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_TOP_CFG0));
    value |= 0x0100;
    writel(value, (volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_TOP_CFG0));

    D("-\n");
}

static void hispfd_deswitch(struct hispfd_s *dev)
{
    unsigned int value;
    D("+\n");
    value = readl((volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_TOP_CFG0));
    value &= (0xfffffeff);
    writel(value, (volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_TOP_CFG0));

    D("-\n");
}

static int hispfd_smmu_init(struct hispfd_s *dev)
{

    unsigned int high, low;
    
    D("+\n");

    writel_relaxed(0x00000000, (volatile void __iomem *)(dev->reg[SMMU_MST_REG] + 0));         // normal mode
    writel_relaxed(0x00000000, (volatile void __iomem *)(dev->reg[SMMU_MST_REG] + 0x40));      // no interrupt mask
    writel_relaxed(0x00000003, (volatile void __iomem *)(dev->reg[SMMU_MST_REG] + 0x34));      // SMMU_MSTR_INPT_SEL(end_req_sel | smr_start_sel)
    writel_relaxed(0x000000ff, (volatile void __iomem *)(dev->reg[SMMU_MST_REG] + 0x4c));      // clear interrupt

    writel_relaxed(0x0001a000, (volatile void __iomem *)(dev->reg[SMMU_MST_REG] + 0x100));     // stream config
    writel_relaxed(0x00009030, (volatile void __iomem *)(dev->reg[SMMU_MST_REG] + 0x104));     // stream config
    writel_relaxed(0x00028000, (volatile void __iomem *)(dev->reg[SMMU_MST_REG] + 0x108));     // stream config
    writel_relaxed(0x00015000, (volatile void __iomem *)(dev->reg[SMMU_MST_REG] + 0x10c));     // stream config
    writel_relaxed(0x00001000, (volatile void __iomem *)(dev->reg[SMMU_MST_REG] + 0x110));     // stream config


    high = (unsigned int)(dev->pteaddr >> 32);
    low  = (unsigned int)dev->pteaddr;
    writel_relaxed(0x000F8CF6, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_SCR_REG));
    writel_relaxed(0x00000001, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_LP_CTRL_REG));        // auto clock by hardware;
    writel_relaxed(0xe4e4e4e0, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_PRESS_REMAP_REG));
    writel_relaxed(0x00000000, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_INTMASK_NS_REG));      // no interrupt mask
    writel_relaxed(0x0000003f, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_INTCLR_NS_REG));      // clear interrupt
    writel_relaxed(0x00000000, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_RLD_EN0_NS_REG));
    writel_relaxed(0x00000000, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_RLD_EN1_NS_REG));
    writel_relaxed(0x000000BC, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_SMRX_NS_0_REG));
    writel_relaxed(0x000000BC, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_SMRX_NS_1_REG));
    writel_relaxed(0x000000BC, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_SMRX_NS_2_REG));
    writel_relaxed(0x000000BC, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_SMRX_NS_3_REG));
    writel_relaxed(0x000000BC, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_SMRX_NS_4_REG));
    writel_relaxed(0x000000BC, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_SMRX_NS_5_REG));
    writel_relaxed(0x000000BC, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_SMRX_NS_6_REG));
    writel_relaxed(0x000000BC, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_SMRX_NS_7_REG));
    writel_relaxed(0x000000BC, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_SMRX_NS_8_REG));
    writel_relaxed(0x000000BC, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_SMRX_NS_9_REG));
    writel_relaxed(0x000000BC, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_SMRX_NS_10_REG));
    writel_relaxed(0x000000BC, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_SMRX_NS_11_REG));
    writel_relaxed(0x000000BC, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_SMRX_NS_12_REG));
    writel_relaxed(0x00000000, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_CB_SCTRL_REG));
    writel_relaxed(low,        (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_CB_TTBR0_REG));
    writel_relaxed(low,        (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_CB_TTBR1_REG));
    writel_relaxed(0x00000001, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_CB_TTBCR_REG));
    writel_relaxed(0x00000000, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_OFFSET_ADDR_NS_REG));
    writel_relaxed(0x00000002, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_SCACHEI_ALL_REG));
    writel_relaxed(0x00000000, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_SCACHEI_L1_REG));
    writel_relaxed(0x00003eee, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_SCACHEI_L2L3_REG));
    writel_relaxed(high,       (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_FAMA_CTRL0_NS_REG));
    writel_relaxed(high,       (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_FAMA_CTRL1_NS_REG));

    writel_relaxed(0x00004101, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_ADDR_MSB_REG));
    writel_relaxed(0x00000000, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_ERR_RDADDR_REG));
    writel_relaxed(0x00000000, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_ERR_WRADDR_REG));

    writel_relaxed(0x00000000, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_DBGRPTR_TLB_REG));
    writel_relaxed(0x00000000, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_DBGRPTR_CACHE_REG));
    writel_relaxed(0x00000000, (volatile void __iomem *)(dev->reg[SMMU_REG] + SMMU_COMN_SMMU_DBGAXI_CTRL_REG));
    
    writel_relaxed(0x00000003, (volatile void __iomem *)(dev->reg[SMMU_REG] + FD_REG_SMMU_SCACHEI_ALL));        
    writel(0x00001F00, (volatile void __iomem *)(dev->reg[SMMU_MST_REG] + FD_REG_SMMU_MSTR_SMRX_START_0));
    D("-\n");
    return 0;
}

int hispfd_comm_init(struct hispfd_s *dev)
{
    int ret;
    D("+\n");

    hispfd_switch(dev);

    ret=hispfd_smmu_init(dev);
    if (ret){
        pr_err("[%s] Failed: smmuinit failed!, ret = %d\n",__func__,ret);
        return ret;
    }
        
    hispfd_intr_init(dev);
    
    D("-\n");
    
    return 0;
}

int hispfd_powerup(struct hispfd_s *dev)
{
    int ret = 0;
    
    E("+\n");

    if ((ret = regulator_enable(dev->hfd_supply)) != 0) {
        E(" Failed: HFD regulator_enable hfd_supply .%d\n",  ret);
        return ret;
    }

    if ((ret = hispfd_set_clk(dev, HIFD_CLK_LEVEL_LOW)) < 0) {
        E(" Failed: hispfd_clock_enable %d.\n",  ret);
        goto err_hfdclk_1;
    }

    hifd_top_cfg_on(dev);
    open_power_gate(dev);

    dev->pwrupflag = 1;

    E("-\n");

    return 0;
    
err_hfdclk_1:
    
    if ((ret = regulator_disable(dev->hfd_supply)) != 0)
        E(" Failed: HFD regulator_disable.%d\n",  ret);

    E("-\n");

    return ret;
}

void hifd_top_cfg_off(struct hispfd_s *dev)
{
    unsigned int i, value;

    for(i = 0; i < 20; i++)
    {
        value = readl((volatile void __iomem *)(dev->reg[SMMU_MST_REG] + FD_REG_SMMU_MSTR_END_ACK_0));
        if((value&0x7c0) == 0x7c0)
        {
            break ;
        }
    }

    if(i > 19)
    {
        E(" wait over 19 times.\n");
    }
    // smmu master clock off
    value = readl((volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_TOP_AXI_CFG0));
    value &= 0xffffffef;
    writel(value, (volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_TOP_AXI_CFG0));

    // smmu master reset
    value = readl((volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_DMA_CRG_CFG1));
    value |= 0x04;
    writel(value, (volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_DMA_CRG_CFG1));

    // HiFD axi/apb/func clock off
    value = readl((volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_HIFD_CRG_CFG0));
    value &= 0xfffffff8;
    writel(value, (volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_HIFD_CRG_CFG0));
    
    // HiFD axi/apb/func reset
    value = readl((volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_HIFD_CRG_CFG1));
    value |= 0x07;
    writel(value, (volatile void __iomem *)(dev->reg[JPG_TOP_REG] + FD_REG_JPG_HIFD_CRG_CFG1));
}

static int hispfd_powerdn(struct hispfd_s *dev)
{
    int ret = 0;

    E("+\n");

    dev->pwrupflag = 0;

    hifd_top_cfg_off(dev);

    hispfd_clock_disable(dev);

    if ((ret = regulator_disable(dev->hfd_supply)) != 0)
        E(" Failed: HFD regulator_disable.%d\n",  ret);

    E("-\n");

    return 0;
}

static int hispfd_wait(struct hispfd_s *dev, struct hispfd_start_st *ctrl)
{
    int ret = 0;
    unsigned int i;
    unsigned char boot[HISPFD_BOOT_SIZE];


    D("+\n");

    if(ctrl->step >= BOOT_TYPE_MAX)
    {
        E(" info : ctrl->flag %d, ctrl->size %d, ctrl->step %d.\n",  \
            ctrl->flag, ctrl->pdata_size, ctrl->step);
        return -1;
    }

    dev->hfd_ready = 0;

    writel_relaxed(0xFFFFFFFF, (volatile void __iomem *)(dev->reg[SMMU_MST_REG] + FD_REG_SMMU_MSTR_SMRX_START_0));

    if((HIFD_VALID == ctrl->flag) && (ctrl->pdata_size != 0) && (ctrl->pdata_size <= HISPFD_BOOT_SIZE) && (!(ctrl->pdata_size & 0x07 )))
    {
        if(NULL == ctrl->pdata)
        {
            E("ctrl->pdata is NULL.\n");
            return -EINVAL;
        }

        ret = copy_from_user(&boot[0], (void __user *)(ctrl->pdata), ctrl->pdata_size);
        if (ret < 0)
        {
            E(" copy_from_user.%d\n",  ret);
            return -EFAULT;
        }
        
        // load boot code to instruct mem
        for(i = 0; i < ctrl->pdata_size; i+=sizeof(unsigned int))
        {
            writel_relaxed(*(unsigned int *)(&boot[i]), \
                (volatile void __iomem *)(dev->reg[HFD_REG] + FD_REG_INST_ADDR_OFFSET + i));
        }
    }
    else if(HIFD_INVALID != ctrl->flag)
    {
        E(" info : ctrl->flag %d, ctrl->pdata_size %d.\n",  ctrl->flag, ctrl->pdata_size);
        return -1;
    }
    


    hispfd_hw_kick(dev);

    ret = wait_event_interruptible_timeout(dev->wait, dev->hfd_ready, (unsigned long)msecs_to_jiffies(300)); /* coverity[check_return] */
    if (ret <= 0)
    {
        E(" Failed : wait_event_interruptible_timeout.%d\n",  ret);

        hispfd_debug(dev);
        ret = -1;
        goto unlock_usecase;
    }

    ret = 0;

unlock_usecase:
    D("-\n");
    return ret;
}


static int hispfd_ioctl_wrreg(struct hispfd_s *dev, unsigned long args)
{
    int ret = 0;
    struct hispfd_reg_s streg;
    
    if(0 == args)
    {
        E("args is NULL.\n");
        return -EINVAL;
    }

    ret = copy_from_user(&streg, (void __user *)args, sizeof(struct hispfd_reg_s));
    if (ret < 0)
    {
        E(" copy_from_user.%d\n",  ret);
        return -EFAULT;
    }

    ret = hispfd_reg_check(dev, &streg);

    if(ret)
    {
        return ret;
    }
    

    writel(streg.value, (volatile void __iomem *)(dev->reg[streg.index] + streg.offset));

    return 0;
}


static int hispfd_ioctl_rdreg(struct hispfd_s *dev, unsigned long args)
{
    int ret = 0;
    struct hispfd_reg_s streg;

    if(0 == args)
    {
        E("args is NULL.\n");
        return -EINVAL;
    }

    ret = copy_from_user(&streg, (void __user *)args, sizeof(struct hispfd_reg_s));
    if (ret < 0)
    {
        E(" copy_from_user.%d\n",  ret);
        return -EFAULT;
    }

    ret = hispfd_reg_check(dev, &streg);
    if(ret)
    {
        return ret;
    }

    streg.value = readl((volatile void __iomem *)(dev->reg[streg.index] + streg.offset));
    

    if ((ret = copy_to_user((void __user *)args, &streg, sizeof(streg))) < 0) {
        E(" copy_to_user.%d\n",  ret);
        return -EFAULT;
    }
    return 0;
}



static int hispfd_ioctl_semp(struct hispfd_s *dev, unsigned long args)
{
    return down_interruptible(&dev->hifd_sem);
}

static int hispfd_ioctl_semv(struct hispfd_s *dev, unsigned long args)
{
    up(&dev->hifd_sem);
    return 0;
}



static int hispfd_ioctl_clk_setrate(struct hispfd_s *dev, unsigned long args)
{
    int ret = 0;
    unsigned int i;
    unsigned int freq;
    unsigned int level;

    if(0 == args)
    {
        return -EINVAL;
    }

    if ((ret = copy_from_user(&level, (void __user *)args, sizeof(unsigned int))) < 0) 
    {
        E(" copy_from_user.%d\n",  ret);
        return -EFAULT;
    }

    if(level >= dev->clk_num)
    {
        E(" level %d is larger than %d.\n",  level, dev->clk_num);
        return -EINVAL;
    }

    for(i = (level+1); i > HIFD_CLK_LEVEL_LOW; i--)
    {
        freq = dev->clk_rate[i-1];

        D(" set clock frequent %d.\n",  freq);
        
        ret = clk_set_rate(dev->hfdclk, freq);
        if(0 == ret)
        {
            return 0;
        }
    }

    E(" set clock frequent %d.\n",  dev->poweroff_clk_rate);
    
    ret = clk_set_rate(dev->hfdclk, dev->poweroff_clk_rate);

    return ret;
}

static int hispfd_ioctl_start_burst(struct hispfd_s *dev, unsigned long args)
{
    int ret = 0;
    unsigned int i;
    unsigned int value;
    struct hispfd_start_st *hifd_start;
    struct hispfd_start_burst start_burst;

    D("+\n");

    if(0 == args)
    {
        return -EFAULT;
    }

    if ((ret = copy_from_user(&start_burst, (void __user *)args, sizeof(struct hispfd_start_burst))) < 0) 
    {
        E(" copy_from_user.%d\n",  ret);
        return -EFAULT;
    }

    if((0 == start_burst.num) || (start_burst.num > FD_BURST_NUM))
    {
        E(" start_burst.num is %d.\n",  start_burst.num);
        return -EFAULT;
    }
    
    ret = hispfd_store_reg(dev, &start_burst);
    if(ret)
    {
        E(" Failed : hispfd_store_reg failed.%d\n",  ret);
        return -EINVAL;
    }
    

    ret = hispfd_pre_reg_config(dev);
    if(ret)
    {
        E(" Failed : hispfd_pre_reg_config failed.%d\n",  ret);
        return -EINVAL;
    }

    hifd_start = &dev->start_tbl[0];
    ret = hispfd_wait(dev, hifd_start);
    if (ret)
    {
        E(" Failed : hispfd_wait.%d\n",  ret);
        return -EINVAL;
    }

    for(i = 0; i < 20; i++)
    {
        value = readl((volatile void __iomem *)(dev->reg[SMMU_MST_REG] + FD_REG_SMMU_MSTR_END_ACK_0));
        if((value&0x7c0) == 0x7c0)
        {
            break ;
        }
        E("smmu master end ack value = 0x%x.\n", value);
        udelay(10);
    }
    
    D("-\n");

    return 0;
}

static int hispfd_ioctl_check(struct hispfd_s *dev, unsigned int cmd)
{
    if (!dev->initialized) {
        E(" Failed : HFD Device Not Exist.%d\n",  dev->initialized);
        return -ENXIO;
    }

    if(!dev->pwrupflag)
    {
        E(" Failed : device is power down.\n");
        return -EINVAL;
    }

    return 0;
}

static int hispfd_ioctl_common_reg(struct hispfd_s *dev, unsigned long args)
{
    int ret = 0;
    unsigned int size, i;
    struct hispfd_common_reg_st common_reg;
    struct hispfd_fd_reg_s *pstreg;

    if(0 == args)
    {
        E("args is NULL.\n");
        return -EINVAL;
    }

    ret = copy_from_user(&common_reg, (void __user *)args, sizeof(struct hispfd_common_reg_st));
    if (ret < 0) 
    {
        E(" copy_from_user failed %d.\n",  ret);
        return -EFAULT;
    }

    if(common_reg.reg_num > FD_COMM_REG_MAX)
    {
        E(" common_reg.reg_num(%d) > FD_COMM_REG_MAX(%d).\n",  common_reg.reg_num, FD_COMM_REG_MAX);
        return -EINVAL;
    }
    
    size = common_reg.reg_num * sizeof(struct hispfd_fd_reg_s);

    pstreg = (struct hispfd_fd_reg_s *)kzalloc(size, GFP_KERNEL);
    if(NULL == pstreg)
    {
        E(" kzalloc failed.\n");
        return -EFAULT;
    }

    if((NULL == common_reg.preg) || (0 == size))
    {
        E("common_reg.preg is NULL.\n");
        goto reg_failed;
    }

    ret = copy_from_user(pstreg, (void __user *)(common_reg.preg), size);
    if(ret < 0) 
    {
        E(" copy_from_user failed %d.\n",  ret);
        goto reg_failed;
    }

    for(i = 0; i < common_reg.reg_num; i++)
    {
        if((pstreg[i].offset % sizeof(unsigned int))
            || (pstreg[i].offset >= resource_size(dev->r[HFD_REG])))
        {
            E(" Failed : streg.offset %d.\n",  pstreg[i].offset);
            ret = -EINVAL;
            goto reg_failed;
        }
        

        writel(pstreg[i].value, (volatile void __iomem *)(dev->reg[HFD_REG] + pstreg[i].offset));
    }
    ret = 0;

reg_failed:
    
    kfree(pstreg);
    return ret;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
static int hispfd_ioctl_map_iommu(struct hispfd_s *dev, unsigned long args)
{
    struct dma_buf *dmabuf;
    struct hifd_memory_block_st buf;
    struct hifd_memory_node *memnode;
    unsigned int iova;
    unsigned long size;
    int ret = 0;

    D("+\n");

    if ((ret = copy_from_user(&buf, (void __user*)args, sizeof(struct hifd_memory_block_st))) != 0){
          pr_err("[%s] copy_from_user.%d\n", __func__, ret);
          return -EFAULT;
    }

    if (buf.shared_fd < 0) {
        pr_err("[%s] Failed : hare_fd.%d\n", __func__, buf.shared_fd);
        return -EINVAL;
    }

    if (!dev ) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -EINVAL;
    }
    memnode = (struct hifd_memory_node*) kzalloc(sizeof(struct hifd_memory_node),GFP_KERNEL);
     if ( NULL == memnode ){
        pr_err("[%s] alloc list node error.\n", __func__);
        return -EINVAL;
     }

    memnode->shared_fd = buf.shared_fd;
    INIT_LIST_HEAD( &memnode->nd );
    dmabuf = dma_buf_get(buf.shared_fd);
    if (IS_ERR_OR_NULL(dmabuf)) {
        pr_err("[%s] Failed : dma_buf_get, buf.%pK\n", __func__, dmabuf);
        return -EINVAL;
    }
    memnode->dmabuf = dmabuf;

    iova = hisi_iommu_map_dmabuf(&dev->pdev->dev, dmabuf, buf.prot, &size);
    if (iova == 0) {
        pr_err("[%s] Failed : hisi_iommu_map_dmabuf\n", __func__);
        goto err_dma_buf_get;
    }

    buf.da = iova;

    pr_info("[after map iommu]da.(0x%x)", buf.da);/*lint !e626 */

    if ((ret = copy_to_user((void __user*)args, &buf, sizeof(struct hifd_memory_block_st))) != 0) {
         pr_err("[%s] copy_to_user.%d\n", __func__, ret);
         goto err_dma_buf_map;
     }
    list_add ( &memnode->nd , &dev->dma_buf_list );

    return 0;
err_dma_buf_map:
    hisi_iommu_unmap_dmabuf(&dev->pdev->dev, dmabuf, buf.da);
err_dma_buf_get:
    dma_buf_put(dmabuf);
err_memnode:
    kfree(memnode);
    D("-\n");
    return ret;
}

static int hispfd_ioctl_unmap_iommu(struct hispfd_s *dev, unsigned long args)
{
    struct hifd_memory_block_st buf;
    struct dma_buf *dmabuf;
    int ret = 0;
    struct hifd_memory_node *memnode;
    int flag = 0;

    D("+\n");

    if ((ret = copy_from_user(&buf, (void __user*)args, sizeof(struct hifd_memory_block_st))) != 0){
        pr_err("[%s] copy_from_user.%d\n", __func__, ret);
        return -EFAULT;
    }

    if ( buf.shared_fd < 0) {
        pr_err("[%s] Failed : hare_fd.%d\n", __func__, buf.shared_fd);
        return -EFAULT;
    }

    if (!dev ) {
        pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
        return -EFAULT;
    }

    D("shared_fd is 0x%x",buf.shared_fd);

    list_for_each_entry(memnode, &dev->dma_buf_list,nd){
        if ( memnode->shared_fd == buf.shared_fd ){
            dmabuf = memnode->dmabuf;
            flag = 1;
            break;
        }
    }
    if ( flag == 0 )
    {
        pr_err("[%s] Failed : cannot find dmabufK\n", __func__);
        return -EFAULT;
    }

    ret = hisi_iommu_unmap_dmabuf(&dev->pdev->dev, dmabuf, buf.da);
    E("iommu_unmap ret %d \n",ret);
    if (ret < 0) {
        pr_err("[%s] Failed : hisi_iommu_unmap_dmabuf\n", __func__);
    }
    else {
        ret = 0;
    }

    dma_buf_put(dmabuf);

    list_del( &memnode->nd );
    kfree( memnode );

    D("-\n");
    return 0;
}
#endif

struct hisp_ioctl_s g_sthifdioctl[] = {
    {HISPFD_CLK_SETRATE,    hispfd_ioctl_clk_setrate},
    {HISPFD_START_BURST,    hispfd_ioctl_start_burst},
    {HISPFD_COMMON_REG,     hispfd_ioctl_common_reg},
    {HISPFD_WR_REG,         hispfd_ioctl_wrreg},
    {HISPFD_RD_REG,         hispfd_ioctl_rdreg},
    {HISPFD_SEM_P,          hispfd_ioctl_semp},
    {HISPFD_SEM_V,          hispfd_ioctl_semv},
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    {HISPFD_IOMMU_MAP,      hispfd_ioctl_map_iommu},
    {HISPFD_IOMMU_UNMAP,    hispfd_ioctl_unmap_iommu},
#endif
};

static long hispfd_ioctl(struct file *filp, unsigned int cmd, unsigned long args)
{
    int ret = 0;
    unsigned int i;
    struct miscdevice *mdev = filp->private_data;
    struct hispfd_s *dev = container_of(mdev, struct hispfd_s, miscdev);

    ret = hispfd_ioctl_check(dev, cmd);
    if(ret)
    {
        return ret;
    }

    for(i = 0; i < ARRAY_SIZE(g_sthifdioctl); i++)
    {
        if(cmd == g_sthifdioctl[i].ioctl_id)
        {
            ret = g_sthifdioctl[i].hifd_ioctl(dev, args);
            return ret;
        }
    }
    
    E(" Failed : Invalid cmd.0x%x\n",  cmd);
    return -EFAULT;
}

static int hispfd_register_irq(struct hispfd_s *dev)
{
    int i = 0, ret = 0;

    D("+\n");
    
    for (i = 0; i < (int)(HISP_MIN(MAX_HISPFD_IRQ, dev->irq_num)); i ++) 
    {
        if ((ret = request_irq(dev->irq[i], hispfd_irq_handler, 0, "HISPFD_IRQ", (void *)dev)) != 0) {
            E(" Failed : %d.request_irq.%d\n",  i, ret);
            return ret;
        }
    }
    D("-\n");

    return 0;
}

static void hispfd_deregister_irq(struct hispfd_s *dev)
{
    int i = 0;

    D("+\n");
    for (i = 0; i < (int)(HISP_MIN(MAX_HISPFD_IRQ, dev->irq_num)); i ++) 
    {
        free_irq(dev->irq[i], dev);
    }
    D("-\n");

    return ;
}

static int hispfd_open(struct inode *inode, struct file *filp)
{
    int ret;
    struct miscdevice *mdev = filp->private_data;
    struct hispfd_s *dev = container_of(mdev, struct hispfd_s, miscdev);

    D("+\n");

    if (!dev->initialized)
    {
        E(" Failed : HFD Device Not Exist.%d\n",  dev->initialized);
        return -ENXIO;
    }

    ret = down_interruptible(&dev->open_sem);
    if(ret)
    {
        E(" down_interruptible Failed : %d\n",  ret);
        return ret;
    }

    dev->open_refs--;

    E(" dev->open_refs %d.\n",  dev->open_refs);

    if(dev->open_refs < 0)
    {
        up(&dev->open_sem);
        return 0;
    }
    else if(dev->open_refs > 0)
    {
        E(" HiFD has been power down too many times. open_refs : %d\n",  dev->open_refs);
        dev->open_refs = 0;
    }
    else
    {
        // do nothing
    }
    
    if ((ret = hispfd_register_irq(dev)) != 0)
    {
        E(" Failed : hispfd_register_irq.%d\n",  ret);
        goto ERROR_REGISTER_IRQ;
    }
    
    ret = ipp_top_fd_switch_lock();
    if(ret)
    {
        E("fd release lock failed!");
    }
    
    if ((ret = hispfd_powerup(dev)) != 0)
    {
        E(" Failed : hispfd_powerup.%d\n",  ret);
        goto ERR_POWER_UP;
    }

    
    if ((ret = hispfd_comm_init(dev)) != 0)
    {
        E(" Failed : hispfd_comm_init.%d\n",  ret);
        goto ERR_COMM_INIT;
    }

    up(&dev->open_sem);

    D("-\n");

    return 0;
    
ERR_COMM_INIT:
    
    if ((ret = hispfd_powerdn(dev)) != 0)
    {
        E(" Failed : hispfd_powerdn.%d\n",  ret);
    }

ERR_POWER_UP:

    ipp_top_fd_switch_unlock();
    hispfd_deregister_irq(dev);

ERROR_REGISTER_IRQ:
    dev->open_refs++;

    E(" dev->open_refs %d.\n",  dev->open_refs);

    up(&dev->open_sem);

    return -EINVAL;
}

static int hispfd_release(struct inode *inode, struct file *filp)
{
    int ret;
    struct miscdevice *mdev = filp->private_data;
    struct hispfd_s *dev = container_of(mdev, struct hispfd_s, miscdev);

    D("+\n");

    if (!dev->initialized) 
    {
        E(" Failed : HFD Device Not Exist.%d\n",  dev->initialized);
        return -ENXIO;
    }

    ret = down_interruptible(&dev->open_sem);
    if(ret)
    {
        E(" down_interruptible Failed : %d\n",  ret);
        return ret;
    }

    dev->open_refs++;

    E(" dev->open_refs %d.\n",  dev->open_refs);

    if(1 == dev->open_refs)
    {
        hispfd_deregister_irq(dev);


        hispfd_deswitch(dev);

        ret = hispfd_powerdn(dev);
        if (0 != ret) 
        {
            E(" Failed : hispfd_powerdn.%d\n",  ret);
        }

        ipp_top_fd_switch_unlock();
    }
    else if(dev->open_refs < 1)
    {
        // do nothing
    }
    else
    {
        E(" HiFD has been power down too many times. open_refs : %d\n",  dev->open_refs);
        dev->open_refs = 1;
    }

    up(&dev->open_sem);

    D("-\n");

    return 0;
}

static ssize_t hispfd_show(struct device *pdev, struct device_attribute *attr, char *buf)
{
    struct platform_device *platform_dev = to_platform_device(pdev);
    struct hispfd_s *dev = platform_get_drvdata(platform_dev);
    int count = 0;
    
    D("+\n");

    /* omflag show */
    count += snprintf_s(buf + count, PAGE_SIZE - count, PAGE_SIZE - count, "om flag:0x%.8x\n", dev->om_flag);

    /* irq time show */
    
    D("-\n");

    return (ssize_t)count;
}

static ssize_t hispfd_store(struct device *pdev, struct device_attribute *attr, const char *buf, size_t count)
{
    D("+\n");
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
            E(" Failed : %d.devm_ioremap.%pK\n",  i, dev->reg[i]);
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
        E(" Failed: irq-num of_property_read_u32.%d\n",  ret);
        return -EINVAL;
    }

    for (i = 0; i < (int)(HISP_MIN(MAX_HISPFD_IRQ, dev->irq_num)); i ++) {
        if ((irq = platform_get_irq(dev->pdev, i)) <= 0) {
            E(" Failed : platform_get_irq.%d\n",  irq);
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
        E(" Failed: reg-num of_property_read_u32.%d\n",  ret);
        return -EINVAL;
    }

    for (i = 0; i < (int)(HISP_MIN(MAX_HISPFD_REG, dev->reg_num)); i ++) {
        if ((dev->r[i] = platform_get_resource(dev->pdev, IORESOURCE_MEM, i)) == NULL) {
            E(" Failed : platform_get_resource.%pK\n",  dev->r[i]);
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
        E(" Failed : HFD devm_regulator_get hisi-media .%pK\n",  dev->hfd_media1_supply);
        return -EINVAL;
    }
    
    dev->hfd_supply = devm_regulator_get(device, "isp-hfd");
    if (IS_ERR(dev->hfd_supply)) {
        E(" Failed : HFD devm_regulator_get isp-hfd .%pK\n",  dev->hfd_supply);
        return -EINVAL;
    }

    //get clk parameters
    dev->hfdclk = devm_clk_get(device, "clk_fdai_func");
    if (IS_ERR(dev->hfdclk)) {
        E("get clk failed");
        return -EINVAL;
    }
    
    dev->jpgclk = devm_clk_get(device, "clk_jpg_func");
    if (dev->jpgclk == NULL) {
        E("get jpg clk failed");
        return -EINVAL;
    }

    np = device->of_node;
    if (NULL == np) {
        E("of_node is NULL.");
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
    if(len > HIFD_CLK_MAX_NUM)
    {
        E(" Failed : len %d.\n",  len);
        return -1;
    }

    dev->clk_num = len;
    
    if (of_property_read_u32_array(device->of_node, "clock_frequency", dev->clk_rate, len))
    {
        E(" Failed : of_property_read_u32_array clock_frequency.\n");
        return -1;
    }

    ret = of_property_read_u32(np, "clock_poweroff_freq", &dev->poweroff_clk_rate);
    if (ret)
    {
        E("get clock_poweroff_freq failed, ret:%d", ret);
        return -EINVAL;
    }

    D("-\n");

    return 0;
}

static int hispfd_getdts(struct hispfd_s *dev)
{
    int ret = 0;

    if ((ret = hispfd_getdts_pwrclk(dev)) != 0) {
        E(" Failed : hispfd_getdts_pwrclk.%d\n",  ret);
        return ret;
    }

    if ((ret = hispfd_getdts_irq(dev)) != 0) {
        E(" Failed : hispfd_getdts_interrupt.%d\n",  ret);
        return ret;
    }

    if ((ret = hispfd_getdts_reg(dev)) != 0) {
        E(" Failed : hispfd_getdts_register.%d\n",  ret);
        return ret;
    }

    return 0;
}

static int hispfd_get_pteaddr(struct hispfd_s *dev)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
    struct iommu_domain_data *info = NULL;

    I("+\n");
    if ((dev->domain = hisi_ion_enable_iommu(NULL)) == NULL) {
        E(" Failed : hisi_ion_enable_iommu.%pK\n",  dev->domain);
        return -ENODEV;
    }

    if ((info = (struct iommu_domain_data *)dev->domain->priv) == NULL) {
        E(" Failed : info.%pK\n", info);
        return -ENODEV;
    }

    dev->pteaddr = info->phy_pgd_base;
    I("-\n");
#else
    struct device *device= NULL;

    I("+\n");
    if (!dev) {
         E("Failed : hipp smmu dev.%pK\n", dev);
         return -ENODEV;
    }

    if (!dev->pdev) {
        E("Failed : pdev.%pK\n", dev->pdev);
        return -ENODEV;
    }

    device = &dev->pdev->dev;

    if (!dev) {
         E("Failed : dev->pdev->dev.%pK\n", device);
         return -ENODEV;
    }

    dev->pteaddr = hisi_domain_get_ttbr(device);
    I("- PTE.0x%lx\n", dev->pteaddr);
#endif
    return 0;
}

static int hispfd_probe(struct platform_device *pdev)
{
    struct hispfd_s *dev = NULL;
    int ret = 0;

    I("+\n");
    if ((dev = (struct hispfd_s *)kzalloc(sizeof(struct hispfd_s), GFP_KERNEL)) == NULL) {
        E(" Failed : kzalloc.%pK\n",  dev);
        return -ENOMEM;
    }

    g_sthispfdcfg = dev;

    dev->initialized = 0;
    dev->pdev = pdev;
    platform_set_drvdata(pdev, dev);

    if ((ret = hispfd_getdts(dev)) != 0) {
        E(" Failed : hispfd_getdts.%d\n",  ret);
        return ret;
    }

    if ((ret = hispfd_resource_init(dev)) != 0) {
        E(" Failed : hispfd_resource_init.%d\n",  ret);
        return ret;
    }

    init_waitqueue_head(&dev->wait);

    if ((ret = hispfd_get_pteaddr(dev)) < 0) {
        E(" Failed : hispfd_get_pteaddr.%d\n",  ret);
        return ret;
    }

    dev->miscdev.minor  = MISC_DYNAMIC_MINOR;
    dev->miscdev.name   = KBUILD_MODNAME;
    dev->miscdev.fops   = &hispfd_fops;

    if ((ret = misc_register((struct miscdevice *)&dev->miscdev)) != 0) {
        E(" Failed : misc_register.%d\n",  ret);
        return ret;
    }

    if ((ret = device_create_file(dev->miscdev.this_device, &dev_attr_hispfd)) != 0) {
        E(" Faield : hispfd device_create_file.%d\n",  ret);
        return ret;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    INIT_LIST_HEAD(&dev->dma_buf_list);
#endif


    dev->open_refs= 1;

    sema_init(&dev->hifd_sem, 1);
    sema_init(&dev->open_sem, 1);
    
    sema_init(&dev->fd_switch_sem, 1);
    
    
    dev->pwrupflag = 0;
    dev->initialized = 1;
    
    I("-\n");

    return ret;
}

static int hispfd_remove(struct platform_device *pdev)
{
    struct hispfd_s *dev = NULL;

    I("+\n");
    if ((dev = (struct hispfd_s *)platform_get_drvdata(pdev)) == NULL) {
        E(" Failed : platform_get_drvdata, dev.%pK.pdev.%pK\n",  dev, pdev);
        return -ENODEV;
    }
    
    hispfd_resource_deinit(dev);

    
    misc_deregister(&dev->miscdev);
    
    dev->open_refs= 1;
    
    dev->initialized = 0;
    dev->pwrupflag = 0;
    
    kfree(dev);
    dev = NULL;
    
    I("-\n");

    return 0;
}

int ipp_top_fd_switch_lock(void)
{
    if((!g_sthispfdcfg) || (!g_sthispfdcfg->initialized))
    {
        return 0;
    }

    return down_interruptible(&g_sthispfdcfg->fd_switch_sem);
}
EXPORT_SYMBOL(ipp_top_fd_switch_lock);

void ipp_top_fd_switch_unlock(void)
{
    if((!g_sthispfdcfg) || (!g_sthispfdcfg->initialized))
    {
        return ;
    }
    
    up(&g_sthispfdcfg->fd_switch_sem);
}
EXPORT_SYMBOL(ipp_top_fd_switch_unlock);

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

