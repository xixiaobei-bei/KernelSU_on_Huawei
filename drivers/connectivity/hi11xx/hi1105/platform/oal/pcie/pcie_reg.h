

#ifndef __PCIE_REG_H__
#define __PCIE_REG_H__
#include "oal_types.h"

/* PCIe Config Register */
#define HI_PCI_IATU_VIEWPORT_OFF 0x900
#define HI_PCI_IATU_OUTBOUND     0
#define HI_PCI_IATU_INBOUND      1

#define HI_PCI_IATU_BOUND_BASE_OFF 0x904 /* 4.70a */

#define hi_pci_iatu_inbound_base_off(i)  (0x100 + ((i)*0x200)) /* 5.00a  base address 0x40104000 size:0x2000 */
#define hi_pci_iatu_outbound_base_off(i) (0x000 + ((i)*0x200))

#define hi_pci_iatu_region_ctrl_1_off_outbound_i(base)        (0x0 + (base))
#define hi_pci_iatu_region_ctrl_1_off_inbound_i(base)         (0x0 + (base)) /* same as outbound, use viewport select */
#define hi_pci_iatu_region_ctrl_2_off_inbound_i(base)         (0x4 + (base))
#define hi_pci_iatu_region_ctrl_2_off_outbound_i(base)        (0x4 + (base))
#define hi_pci_iatu_lwr_base_addr_off_inbound_i(base)         (0x8 + (base))
#define hi_pci_iatu_lwr_base_addr_off_outbound_i(base)        (0x8 + (base))
#define hi_pci_iatu_upper_base_addr_off_outbound_i(base)      (0xc + (base))
#define hi_pci_iatu_upper_base_addr_off_inbound_i(base)       (0xc + (base))
#define hi_pci_iatu_limit_addr_off_outbound_i(base)           (0x10 + (base))
#define hi_pci_iatu_limit_addr_off_inbound_i(base)            (0x10 + (base))
#define hi_pci_iatu_lwr_target_addr_off_outbound_i(base)      (0x14 + (base))
#define hi_pci_iatu_lwr_target_addr_off_inbound_i(base)       (0x14 + (base))
#define hi_pci_iatu_upper_target_addr_off_outbound_i(base)    (0x18 + (base))
#define hi_pci_iatu_upper_target_addr_off_inbound_i(base)     (0x18 + (base))
#define hi_pci_iatu_region_ctrl_3_off_outbound_i(base)        (0x1c + (base))
#define hi_pci_iatu_region_ctrl_3_off_inbound_i(base)         (0x1c + (base))
#define hi_pci_iatu_uppr_limit_addr_off_inbound_i(base)       (0x20 + (base))
#define hi_pci_iatu_uppr_limit_addr_off_outbound_i(base)      (0x20 + (base))

typedef union {
    struct {
        // LSB
        uint32_t region_index : 31;  // 0:30
        uint32_t region_dir : 1;     // 31
        // MSB
    } bits;
    uint32_t as_dword;
} iatu_viewport_off;

typedef union {
    struct {
        // LSB
        uint32_t type : 5;             // 0:4
        uint32_t tc : 3;               // 5:7
        uint32_t td : 1;               // 8
        uint32_t attr : 2;             // 9:10
        uint32_t ido : 1;              // 11
        uint32_t th : 1;               // 12
        uint32_t inc_region_size : 1;  // 13
        uint32_t reserved14 : 2;       // 14:15
        uint32_t at : 2;               // 16:17
        uint32_t ph : 2;               // 18:19
        uint32_t func_num : 3;         // 20:22
        uint32_t reserved23 : 9;       // 23:31
        // MSB
    } bits;
    uint32_t as_dword;
} iatu_region_ctrl_1_off;

typedef union {
    struct {
        // LSB
        uint32_t msg_code : 8;                  // 0:7
        uint32_t bar_num : 3;                   // 8:10
        uint32_t rsvdp11 : 2;                   // 11:12
        uint32_t msg_type_match_mode : 1;       // 13
        uint32_t tc_match_en : 1;               // 14
        uint32_t td_match_en : 1;               // 15
        uint32_t attr_match_en : 1;             // 16
        uint32_t th_match_en : 1;               // 17
        uint32_t at_match_en : 1;               // 18
        uint32_t func_num_match_en : 1;         // 19
        uint32_t vf_match_en : 1;               // 20
        uint32_t msg_code_match_en : 1;         // 21
        uint32_t ph_match_en : 1;               // 22
        uint32_t single_addr_loc_trans_en : 1;  // 23
        uint32_t response_code : 2;             // 24:25
        uint32_t vfbar_match_mode_en : 1;       // 26
        uint32_t fuzzy_type_match_code : 1;     // 27
        uint32_t cfg_shift_mode : 1;            // 28
        uint32_t invert_mode : 1;               // 29
        uint32_t match_mode : 1;                // 30
        uint32_t region_en : 1;                 // 31
        // MSB
    } bits;
    uint32_t as_dword;
} iatu_region_ctrl_2_off;

#endif

