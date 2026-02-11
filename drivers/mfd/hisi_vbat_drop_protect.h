/*
 * Header file for hisi vbat drop protect device driver
 */

#ifndef	__HISI_VBAT_DROP_PROTECT_H
#define	__HISI_VBAT_DROP_PROTECT_H

#include <linux/workqueue.h>
#include <linux/mfd/hisi_pmic.h>
#include <soc_pmctrl_interface.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <pmic_interface.h>

#if defined (CONFIG_HISI_VBAT_DROP_PRT_ATLA)
#include <hisi_vbat_drop_protect_atla.h>
#elif defined (CONFIG_HISI_VBAT_DROP_PRT_PHON)
#include <hisi_vbat_drop_protect_phon.h>
#elif defined (CONFIG_HISI_VBAT_DROP_PRT_ORL)
#include <hisi_vbat_drop_protect_orl.h>
#endif

#ifndef BIT
#define BIT(x)      (1 << (x))
#endif


/*interrupt reg in pmic*/
#define PMIC_VSYS_DROP_VOL_SET              PMIC_VSYS_DROP_SET_ADDR(0)
#define PMIC_VSYS_DROP_IRQ_REG              PMIC_VSYS_DROP_IRQ_ADDR(0)
#define PMIC_VSYS_DROP_IRQ_CLEAR            BIT(0)
#ifdef CONFIG_HISI_VBAT_DROP_PRT_ORL
#define PMIC_VSYS_DROP_IRQ_MASK_REG         PMIC_IRQ_MASK_12_ADDR(0)
#else
#define PMIC_VSYS_DROP_IRQ_MASK_REG         PMIC_IRQ_MASK_13_ADDR(0)
#endif
#define PMIC_VSYS_DROP_IRQ_MASK             BIT(0)
#define PMIC_VBAT_DROP_VOL_SET_REG          PMIC_VSYS_DROP_SET_ADDR(0)
#define VBAT_DROP_VOL_DEFAULT               3100

#define VBAT_DROP_VOL_NORMAL_CNT            2
#define VBAT_DROP_VOL_INC_MV                200

/*pmic reg read and write interface macro */
#define HISI_VBAT_DROP_PMIC_REG_READ(regAddr)             hisi_pmic_reg_read(regAddr)
#define HISI_VBAT_DROP_PMIC_REG_WRITE(regAddr,regval)     hisi_pmic_reg_write((int)(regAddr),(int)regval)
#define HISI_VBAT_DROP_PMIC_REGS_READ(regAddr,buf,size)   hisi_pmic_array_read((int)(regAddr),(char*)(buf),(int)(size))
#define HISI_VBAT_DROP_PMIC_REGS_WRITE(regAddr,buf,size)  hisi_pmic_array_write((int)(regAddr),(char*)(buf),(int)(size))


/*cpu and gpu freq state*/
enum vbat_drop_freq {

    MIN_FREQ,
    RESTOR_FREQ
};

/*auto div core type*/
enum drop_freq_en {

    BIG_CPU    = 0,
    MIDDLE_CPU = 1,
    LITTLE_CPU = 2,
    L3_CPU     = 3,
    GPU_CPU    = 4,
    NPU_CPU    = 5,
    ALL
};


struct hisi_vbat_drop_protect_dev {
	struct device		    *dev;
    void __iomem            *core_big_base;
    void __iomem            *core_mid_base;
    void __iomem            *core_lit_base;
    void __iomem            *core_l3_base;
	void __iomem            *core_gpu_base;
	void __iomem            *core_npu_base;
    struct wakeup_source    vbatt_check_lock;
	struct delayed_work     vbat_drop_irq_work;
    int                     big_cpu_auto_div_en;     /* control big cpu auto 2 div enable */
    int                     middle_cpu_auto_div_en;
    int                     little_cpu_auto_div_en;
    int                     l3_auto_div_en;          /* control l3 auto 2 div enable */
    int                     gpu_auto_div_en;
    int                     npu_auto_div_en;
    unsigned int            vbat_drop_vol_mv;
    int                     vbat_drop_irq;
};

#endif		/* __HISI_VBAT_DROP_PROTECT_H */
