

#ifndef __OAL_HARDWARE_H__
#define __OAL_HARDWARE_H__

/* 其他头文件包含 */
#include "oal_types.h"
#include "oal_pci_if.h"
#include "arch/oal_hardware.h"

#define OAL_IRQ_NUM 5

#define PCI_ANY_ID (~0)

#define oal_irq_init_mac_dev(_dev, _irq, _type, _name, _arg, _func) \
    do {                                                            \
        (_dev).ul_irq = (_irq);                                     \
        (_dev).l_irq_type = (_type);                                \
        (_dev).pc_name = (_name);                                   \
        (_dev).p_drv_arg = (_arg);                                  \
        (_dev).p_irq_intr_func = (_func);                           \
    } while (0)

#define oal_pci_get_dev_id(_dev) ((_dev)->device);

#define MAX_NUM_CORES 2

/* TIMER外部控制寄存器 */
typedef struct {
    volatile uint32_t *pul_sc_ctrl;
} oal_hi_timer_ctrl_reg_stru;

#endif /* end of oal_hardware.h */
