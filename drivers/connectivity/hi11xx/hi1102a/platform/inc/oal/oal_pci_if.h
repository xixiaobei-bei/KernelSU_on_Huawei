

#ifndef __OAL_PCI_IF_H__
#define __OAL_PCI_IF_H__

/* 其他头文件包含 */
#include "oal_types.h"
#include "oal_util.h"
#include "oal_hardware.h"
#include "arch/oal_pci_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAL_PCI_IF_H

/* 枚举定义 */
typedef enum {
    OAL_PCI_BAR_0 = 0,
    OAL_PCI_BAR_1,
    OAL_PCI_BAR_2,
    OAL_PCI_BAR_3,
    OAL_PCI_BAR_4,
    OAL_PCI_BAR_5,

    OAL_PCI_BAR_BUTT
} oal_pci_bar_idx_enum;
typedef oal_uint8 oal_pci_bar_idx_enum_uint8;

typedef enum {
    OAL_PCI_GEN_1_0 = 0,
    OAL_PCI_GEN_2_0,
    OAL_PCI_GEN_3_0,

    OAL_PCI_GEN_BUTT
} oal_pci_gen_enum;
typedef oal_uint8 oal_pci_gen_enum_uint8;
#endif /* end of oal_pci_if.h */
