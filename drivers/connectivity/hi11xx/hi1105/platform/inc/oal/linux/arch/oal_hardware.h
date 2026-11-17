

#ifndef __OAL_LINUX_HARDWARE_H__
#define __OAL_LINUX_HARDWARE_H__

/* 其他头文件包含 */
/*lint -e322*/
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/pci.h>
#include <linux/ioport.h>
#include <linux/pm.h>
#include <linux/version.h>
#if (_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_E5) ||  \
    (_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_CPE)
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/syscore_ops.h>
#include "mdrv_timer.h"
#endif
#include <linux/gpio.h>
/*lint +e322*/
#include "oal_util.h"
#include "hi_drv_gpio.h"

/* 宏定义 */
typedef irq_handler_t oal_irq_handler_t;

#define OAL_HI_TIMER_REG_BASE 0x10105000

#define OAL_HI_TIMER_NUM            2
#define OAL_HI_TIMER_ENABLE         1
#define OAL_HI_TIMER_DISABLE        0
#define OAL_HI_TIMER_INT_DISABLE    1
#define OAL_HI_TIMER_INT_CLEAR      0
#define OAL_HI_TIMER_DEFAULT_PERIOD 1

#define OAL_HI_TIMER_IRQ_NO 80 /* 5113 : 5   5115:80 */

#define OAL_HI_TIMER_FREE_MODE   0 /* 1101测试新增 */
#define OAL_HI_TIMER_CYCLE_MODE  1
#define OAL_HI_TIMER_SIZE_32_BIT 1
#define OAL_HI_TIMER_WRAPPING    0
#define OAL_HI_TIMER_INT_OCCUR   1
#define OAL_HI_TIMER_INT_VALID   0x01
#define OAL_HI_TIMER_NO_DIV_FREQ 0x0

#define OAL_HI_SC_REG_BASE 0x10100000
#define OAL_HI_SC_CTRL     (OAL_HI_SC_REG_BASE + 0x0000)

#define OAL_IRQ_ENABLE    1 /* 可以中断 */
#define OAL_IRQ_FORBIDDEN 0 /* 禁止中断 */

/* 数组最后一个成员用来保存save时间 */
/* 数组倒数第二个成员用来保存save的类型 */
#define OAL_TIMER_IRQ_TYPE_MAX_NUM    255
#define OAL_TIMER_IRQ_SAVE_TIME_INDEX (OAL_TIMER_IRQ_TYPE_MAX_NUM - 1)
#define OAL_TIMER_IRQ_SAVE_TYPE_INDEX (OAL_TIMER_IRQ_TYPE_MAX_NUM - 2)
typedef irqreturn_t oal_irqreturn_t;

/* 枚举定义 */
typedef enum {
    OAL_5115TIMER_ONE,
    OAL_5115TIMER_SEC,

    OAL_5115TIMER_BUTT
} oal_5115timer_enum;

/* STRUCT定义 */
typedef struct cpumask *oal_cpumask;

#define OAL_SA_SHIRQ IRQF_SHARED /* 中断类型 */

typedef uint32_t (*oal_irq_intr_func)(void *);
typedef int32_t (*oal_dbac_isr_func)(int);

typedef struct resource oal_resource;

typedef struct {
    volatile uint32_t timerx_load;
    volatile uint32_t timerx_value;
    volatile uint32_t timerx_control;
    volatile uint32_t timerx_intclr;
    volatile uint32_t timerx_ris;
    volatile uint32_t timerx_mis;
    volatile uint32_t timerx_bgload;
    volatile uint32_t reserve;
} oal_hi_timerx_reg_stru;
/* timer控制寄存器 */
typedef union {
    volatile uint32_t _value;
    struct {
        volatile uint32_t oneshot : 1;    /* 选择计数模式 0：回卷计数 1：一次性计数 */
        volatile uint32_t timersize : 1;  /* 16bit|32bit计数操作模式 0：16bit 1：32bit */
        volatile uint32_t timerpre : 2;   /* 预分频因子 00：不分频 01：4级分频 10：8级分频 11：未定义，设置相当于分频因子10 */
        volatile uint32_t reserved0 : 1;  /* 保留位 */
        volatile uint32_t intenable : 1;  /* 中断屏蔽位 0：屏蔽 1：不屏蔽 */
        volatile uint32_t timermode : 1;  /* 计数模式 0：自由模式 1：周期模式 */
        volatile uint32_t timeren : 1;    /* 定时器使能位 0：禁止 1：使能 */
        volatile uint32_t reserved1 : 24; /* 保留位 */
    } bits_stru;
} oal_hi_timer_control_union;

/* timer2_3寄存器 */
typedef struct {
    oal_hi_timerx_reg_stru ast_timer[OAL_5115TIMER_BUTT];
} oal_hi_timer_reg_stru;
typedef struct {
    oal_hi_timer_control_union u_timerx_config;
} oal_hi_timerx_config_stru;

/* PCI驱动相关定义 */
typedef struct pci_driver oal_pci_driver_stru;
typedef struct pci_device_id oal_pci_device_id_stru;
typedef struct pci_dev oal_pci_dev_stru;
typedef struct dev_pm_ops oal_dev_pm_ops_stru;
typedef pm_message_t oal_pm_message_t;
#if (_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_E5)
typedef struct syscore_ops oal_pm_syscore_ops;
#endif

/* 中断设备结构体 */
typedef struct {
    uint32_t irq;                 /* 中断号 */
    int32_t l_irq_type;              /* 中断类型标志 */
    void *p_drv_arg;               /* 中断处理函数参数 */
    char *pc_name;                 /* 中断设备名字 只为界面友好 */
    oal_irq_intr_func p_irq_intr_func; /* 中断处理函数地址 */
} oal_irq_dev_stru;

/* 全局变量声明 */
extern oal_hi_timer_reg_stru *g_reg_timer;
extern uint32_t g_irq_save_time[][OAL_TIMER_IRQ_TYPE_MAX_NUM];

/* 函数声明 */
/*
 * 函 数 名  : oal_irq_free
 * 功能描述  : 释放中断处理程序
 * 输入参数  : st_osdev: 中断设备结构体
 */
OAL_STATIC OAL_INLINE void oal_irq_free(oal_irq_dev_stru *st_osdev)
{
    if (oal_unlikely(st_osdev == NULL)) {
        oal_warn_on(1);
        return;
    }
    free_irq(st_osdev->irq, st_osdev);
}

/*
 * 函 数 名  : oal_irq_interrupt
 * 功能描述  : 中断服务程序
 * 输入参数  : l_irq: 中断号
 *             p_dev: 中断设备
 * 返 回 值  : IRQ_HANDLED中断程序处理完毕
 */
OAL_STATIC OAL_INLINE irqreturn_t oal_irq_interrupt(int32_t l_irq, void *p_dev)
{
    oal_irq_dev_stru *st_osdev = (oal_irq_dev_stru *)p_dev;

    st_osdev->p_irq_intr_func((void *)st_osdev);

    return IRQ_HANDLED;
}

/*
 * 函 数 名  : oal_irq_setup
 * 功能描述  : 注册中断。
 * 输入参数  : st_osdev: 中断设备结构体
 */
OAL_STATIC OAL_INLINE int32_t oal_irq_setup(oal_irq_dev_stru *st_osdev)
{
    int32_t l_err;

    if (oal_unlikely(st_osdev == NULL)) {
        oal_warn_on(1);
        return OAL_FAIL;
    }

    l_err = request_irq(st_osdev->irq, oal_irq_interrupt,
                        st_osdev->l_irq_type, st_osdev->pc_name, (void *)st_osdev);

    return l_err;
}

/*
 * 函 数 名  : oal_irq_trigger
 * 功能描述  : 软件启动目标核的硬件中断
 */
OAL_STATIC OAL_INLINE void oal_irq_trigger(uint8_t uc_cpuid)
{
}

OAL_STATIC OAL_INLINE int32_t oal_gpio_is_valid(int32_t i_number)
{
    return gpio_is_valid(i_number);
}

OAL_STATIC OAL_INLINE int32_t oal_gpio_request(uint32_t ul_gpio, const char *pc_label)
{
    return gpio_request(ul_gpio, pc_label);
}

OAL_STATIC OAL_INLINE void oal_gpio_free(uint32_t ul_gpio)
{
    gpio_free(ul_gpio);
}

OAL_STATIC OAL_INLINE int32_t oal_gpio_direction_input(uint32_t ul_gpio)
{
#ifdef _PRE_HI_DRV_GPIO
    return hitv_gpio_request(ul_gpio, 1);
#else
    return gpio_direction_input(ul_gpio);
#endif
}

OAL_STATIC OAL_INLINE int oal_gpio_direction_output(uint32_t ul_gpio, int l_level)
{
#ifdef _PRE_HI_DRV_GPIO
    hitv_gpio_request(ul_gpio, 0);
    return hitv_gpio_direction_output(ul_gpio, l_level);
#else
    return gpio_direction_output(ul_gpio, l_level);
#endif
}

OAL_STATIC OAL_INLINE int32_t oal_gpio_to_irq(uint32_t ul_gpio)
{
    return gpio_to_irq(ul_gpio);
}

OAL_STATIC OAL_INLINE int32_t oal_request_irq(uint32_t ul_irq,
                                              oal_irq_handler_t p_handler,
                                              uint32_t ul_flags,
                                              const char *p_name,
                                              void *p_dev)
{
    return request_irq(ul_irq, p_handler, ul_flags, p_name, p_dev);
}

OAL_STATIC OAL_INLINE int32_t oal_gpio_get_value(uint32_t ul_gpio)
{
#ifdef _PRE_HI_DRV_GPIO
    return hitv_gpio_get_value(ul_gpio);
#else
    return gpio_get_value(ul_gpio);
#endif
}

OAL_STATIC OAL_INLINE void oal_gpio_set_value(uint32_t ul_gpio, int32_t value)
{
    gpio_set_value(ul_gpio, value);
}

#define DELAY_TIME_MS 500
OAL_STATIC OAL_INLINE void oal_wifi_reg_on_pull_up(int32_t wifi_gpio_addr)
{
    if (!oal_gpio_is_valid(wifi_gpio_addr)) {
        oal_io_print("wifi_reg_on_pull_up:fail to get wifi gpio!\n");
        return;
    }
    /* 如果已经上过电，则直接返回 */
    if (oal_gpio_get_value(wifi_gpio_addr) == 1) {
        oal_io_print("wifi_reg_on_pull_up:WL_REG_ON has been pulled up in wifi_reg_on_pull_up!!!\n");
        return;
    }
    oal_mdelay(DELAY_TIME_MS);
    oal_gpio_direction_output(wifi_gpio_addr, 1);
    oal_mdelay(DELAY_TIME_MS);
}

OAL_STATIC OAL_INLINE void oal_wifi_reg_on_pull_down(int32_t wifi_gpio_addr)
{
    if (!oal_gpio_is_valid(wifi_gpio_addr)) {
        oal_io_print("wifi_reg_on_pull_down:fail to get wifi gpio!\n");
        return;
    }
    /* 如果已经下过电，则直接返回 */
    if (oal_gpio_get_value(wifi_gpio_addr) == 0) {
        oal_io_print("wifi_reg_on_pull_down:WL_REG_ON has been pulled down in wifi_reg_on_pull_down!!!\n");
        return;
    }

    oal_gpio_direction_output(wifi_gpio_addr, 0);
    oal_mdelay(DELAY_TIME_MS);
}

/*
 * 函 数 名  : oal_irq_set_affinity
 * 功能描述  : 将指定的中断号绑定到指定的cpu上执行
 * 输入参数  : irq: 中断号
 *             ul_cpu: cpu掩码
 */
OAL_STATIC OAL_INLINE int32_t oal_irq_set_affinity(uint32_t irq, uint32_t ul_cpu)
{
    int32_t l_ret;
    struct cpumask mask;

    cpumask_clear(&mask);
    cpumask_set_cpu(ul_cpu, &mask);

    l_ret = irq_set_affinity(irq, &mask);
    if (l_ret != 0) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/* 创建一个新的被使用资源区 */
#define oal_request_mem_region(start, n, name) request_mem_region(start, n, name)
#define oal_release_mem_region(start, n)       release_mem_region(start, n)

#define oal_ioremap(cookie, size)           ioremap(cookie, size)
#define oal_ioremap_nocache(_cookie, _size) ioremap_nocache(_cookie, _size)
#define oal_iounmap(cookie)                 iounmap(cookie)

#define oal_writel(_ul_val, _ul_addr) writel(_ul_val, (void*)(_ul_addr))

#define oal_readl(_ul_addr) readl((void*)(_ul_addr))

#define oal_writew(_us_val, _ul_addr) writew(_us_val, _ul_addr)

#define oal_readw(_ul_addr) readw(_ul_addr)

/* 将var中[pos, pos + bits-1]比特清零,  pos从0开始编号 e.g oal_clearl_bits(var, 4, 2) 表示将Bit5~4清零 */
OAL_STATIC OAL_INLINE void oal_clearl_bits(void *addr, uint32_t pos, uint32_t bits)
{
    uint32_t value;
    if (oal_unlikely(addr == NULL)) {
        oal_warn_on(1);
        return;
    }
    value = oal_readl(addr);
    value &= ~((((uint32_t)1 << (bits)) - 1) << (pos));
    oal_writel(value, (addr));
}

/* 将var中[pos, pos + bits-1]比特设置为val,  pos从0开始编号 e.g oal_setl_bits(var, 4, 2, 2) 表示将Bit5~4设置为b'10 */
OAL_STATIC OAL_INLINE void oal_setl_bits(void *addr, uint32_t pos, uint32_t bits, uint32_t val)
{
    uint32_t value;
    if (oal_unlikely(addr == NULL)) {
        oal_warn_on(1);
        return;
    }
    value = oal_readl(addr);
    value &= ~((((uint32_t)1 << (bits)) - 1) << (pos));
    value |= ((uint32_t)((val) & (((uint32_t)1 << (bits)) - 1)) << (pos));
    oal_writel(value, (addr));
}

OAL_STATIC OAL_INLINE void oal_clearl_bit(void *addr, uint32_t pos)
{
    uint32_t value;
    if (oal_unlikely(addr == NULL)) {
        oal_warn_on(1);
        return;
    }
    value = oal_readl(addr);
    value &= ~(1 << (pos));
    oal_writel(value, (addr));
}

OAL_STATIC OAL_INLINE void oal_setl_bit(void *addr, uint32_t pos)
{
    uint32_t value;
    if (oal_unlikely(addr == NULL)) {
        oal_warn_on(1);
        return;
    }
    value = oal_readl(addr);
    value |= (1 << pos);
    oal_writel(value, (addr));
}

OAL_STATIC OAL_INLINE void oal_clearl_mask(void *addr, uint32_t mask)
{
    uint32_t value;
    if (oal_unlikely(addr == NULL)) {
        oal_warn_on(1);
        return;
    }
    value = oal_readl(addr);
    value &= ~(mask);
    oal_writel(value, (addr));
}

OAL_STATIC OAL_INLINE void oal_setl_mask(void *addr, uint32_t mask)
{
    uint32_t value;
    if (oal_unlikely(addr == NULL)) {
        oal_warn_on(1);
        return;
    }
    value = oal_readl(addr);
    value |= (mask);
    oal_writel(value, (addr));
}

OAL_STATIC OAL_INLINE void oal_value_mask(void *addr, uint32_t value, uint32_t mask)
{
    uint32_t reg;
    if (oal_unlikely(addr == NULL)) {
        oal_warn_on(1);
        return;
    }
    reg = oal_readl(addr);
    reg &= ~(mask);
    value &= mask;
    reg |= value;
    oal_writel(reg, (addr));
}

#endif /* end of oal_hardware.h */
