#ifndef _IVP_PLATFORM_H_
#define _IVP_PLATFORM_H_

#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include "ivp.h"

/******addr defines for block IVP_CFG_NON_SEC_REG(fama)******/
#define ADDR_IVP_CFG_SEC_REG_IVP_FAMA_CTRL0        (0x0530)
#define ADDR_IVP_CFG_SEC_REG_FAMA_ADDR_REMAP0_0    (0x0534)
#define ADDR_IVP_CFG_SEC_REG_FAMA_ADDR_REMAP0_1    (0x0538)
#define ADDR_IVP_CFG_SEC_REG_FAMA_ADDR_REMAP0_2    (0x053C)

#define IVP_CLK_LEVEL_DEFAULT         0
#define IVP_CLK_LEVEL_LOW             1
#define IVP_CLK_LEVEL_MEDIUM          2
#define IVP_CLK_LEVEL_HIGH            3

#define LISTENTRY_SIZE         (0x00600000)
#define GIC_IRQ_CLEAR_REG      (0xe82b11a4)

#define IVP_CLKRATE_HIGH       (554000000)
#define IVP_CLKRATE_MEDIUM     (415000000)
#define IVP_CLKRATE_LOW        (238000000)

struct ivp_iomem_res {
    char __iomem *cfg_base_addr;
    char __iomem *pctrl_base_addr;
    char __iomem *pericrg_base_addr;
    char __iomem *gic_base_addr;
};

struct ivp_device {
    struct ivp_iomem_res io_res;
    struct clk * clk;
    unsigned int clk_rate;
    unsigned int clk_level;
    unsigned int clk_usrsetrate;
    unsigned int lowfrq_pd_clk_rate;
    unsigned int wdg_irq;
    atomic_t wdg_sleep;
    unsigned int dwaxi_dlock_irq;
    struct semaphore wdg_sem;
    int sect_count;
    struct ivp_sect_info *sects;
    struct ivp_sect_info *sec_sects;
    struct dentry *debugfs;
    struct miscdevice device;
    struct regulator *regulator;
    struct regulator *ivp_media1_regulator;
    unsigned long       smmu_pgd_base;
    struct ivp_smmu_dev *smmu_dev;
    atomic_t accessible;
    atomic_t poweron_access;
    atomic_t poweron_success;
    void *vaddr_memory;

    int ivp_meminddr_len;
    unsigned int dynamic_mem_size;
    unsigned int dynamic_mem_section_size;
    unsigned int ivp_sec_support;
    unsigned int ivp_secmode;
    struct platform_device *ivp_pdev;
};

extern int ivp_poweron_pri(struct ivp_device *ivp_devp);
extern int ivp_poweron_remap(struct ivp_device *ivp_devp);
extern int ivp_poweroff_pri(struct ivp_device *ivp_devp);
extern int ivp_init_pri(struct platform_device *pdev, struct ivp_device *ivp_devp);
extern void ivp_deinit_pri(struct ivp_device *ivp_devp);
extern int ivp_change_clk(struct ivp_device *ivp_devp, unsigned int level);
extern int ivp_init_resethandler(struct ivp_device *pdev);
extern void ivp_deinit_resethandler(struct ivp_device *pdev);
extern int ivp_check_resethandler(struct ivp_device *pdev);
extern int  ivp_sec_loadimage(struct ivp_device *pdev);
extern void ivp_dev_hwa_enable(void);
extern void ivp_hw_enable_reset(struct ivp_device *devp);

#endif /* _IVP_PLATFORM_H_ */
