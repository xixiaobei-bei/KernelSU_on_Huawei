

#ifndef __SMMU_COMN_REG_OFFSET_H__
#define __SMMU_COMN_REG_OFFSET_H__


/******************************************************************************
                        HiMMU SMMU_COMN Registers' Definitions
 ******************************************************************************/

#define SMMU_COMN_SMMU_SCR_REG               0x0     /* SMMU Global Control Register */
#define SMMU_COMN_SMMU_MEMCTRL_REG           0x4     /* SMMU Memory Control Register */
#define SMMU_COMN_SMMU_LP_CTRL_REG           0x8     /* SMMU Low Power Control Register */
#define SMMU_COMN_SMMU_PRESS_REMAP_REG       0xC     /* SMMU Pressure Remap register */
#define SMMU_COMN_SMMU_INTMASK_NS_REG        0x10    /* SMMU Interrupt Mask Register for Non Secure contest bank */
#define SMMU_COMN_SMMU_INTRAW_NS_REG         0x14    /* SMMU Interrupt Raw Status for Non Secure contest bank */
#define SMMU_COMN_SMMU_INTSTAT_NS_REG        0x18    /* SMMU Interrupt Status after Mask for Non Secure contest bank */
#define SMMU_COMN_SMMU_INTCLR_NS_REG         0x1C    /* SMMU Interrupt Clear Register for Secure contest bank */
#define SMMU_COMN_SMMU_SMRX_NS_0_REG         0x20    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_1_REG         0x24    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_2_REG         0x28    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_3_REG         0x2C    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_4_REG         0x30    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_5_REG         0x34    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_6_REG         0x38    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_7_REG         0x3C    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_8_REG         0x40    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_9_REG         0x44    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_10_REG        0x48    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_11_REG        0x4C    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_12_REG        0x50    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_13_REG        0x54    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_14_REG        0x58    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_15_REG        0x5C    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_16_REG        0x60    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_17_REG        0x64    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_18_REG        0x68    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_19_REG        0x6C    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_20_REG        0x70    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_21_REG        0x74    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_22_REG        0x78    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_23_REG        0x7C    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_24_REG        0x80    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_25_REG        0x84    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_26_REG        0x88    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_27_REG        0x8C    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_28_REG        0x90    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_29_REG        0x94    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_30_REG        0x98    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_31_REG        0x9C    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_32_REG        0xA0    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_33_REG        0xA4    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_34_REG        0xA8    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_35_REG        0xAC    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_36_REG        0xB0    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_37_REG        0xB4    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_38_REG        0xB8    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_39_REG        0xBC    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_40_REG        0xC0    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_41_REG        0xC4    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_42_REG        0xC8    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_43_REG        0xCC    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_44_REG        0xD0    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_45_REG        0xD4    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_46_REG        0xD8    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_47_REG        0xDC    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_48_REG        0xE0    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_49_REG        0xE4    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_50_REG        0xE8    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_51_REG        0xEC    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_52_REG        0xF0    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_53_REG        0xF4    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_54_REG        0xF8    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_55_REG        0xFC    /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_56_REG        0x100   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_57_REG        0x104   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_58_REG        0x108   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_59_REG        0x10C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_60_REG        0x110   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_61_REG        0x114   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_62_REG        0x118   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_63_REG        0x11C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_64_REG        0x120   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_65_REG        0x124   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_66_REG        0x128   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_67_REG        0x12C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_68_REG        0x130   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_69_REG        0x134   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_70_REG        0x138   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_71_REG        0x13C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_72_REG        0x140   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_73_REG        0x144   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_74_REG        0x148   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_75_REG        0x14C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_76_REG        0x150   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_77_REG        0x154   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_78_REG        0x158   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_79_REG        0x15C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_80_REG        0x160   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_81_REG        0x164   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_82_REG        0x168   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_83_REG        0x16C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_84_REG        0x170   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_85_REG        0x174   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_86_REG        0x178   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_NS_87_REG        0x17C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_RLD_EN0_NS_REG        0x1F0   /* SMMU SMR Reload Enable Register0 */
#define SMMU_COMN_SMMU_RLD_EN1_NS_REG        0x1F4   /* SMMU SMR Reload Enable Register1 */
#define SMMU_COMN_SMMU_RLD_EN2_NS_REG        0x1F8   /* SMMU SMR Reload Enable Register2 */
#define SMMU_COMN_SMMU_CB_SCTRL_REG          0x200   /* SMMU System Control Register for Non-Secure Context BankCan be accessed in Non-Secure mode */
#define SMMU_COMN_SMMU_CB_TTBR0_REG          0x204   /* SMMU Translation Table Base Register for Non-Secure Context Bank0Can be accessed in Non-Secure mode */
#define SMMU_COMN_SMMU_CB_TTBR1_REG          0x208   /* SMMU Translation Table Base Register for Non-Secure Context Bank1Can be accessed in Non-Secure mode */
#define SMMU_COMN_SMMU_CB_TTBCR_REG          0x20C   /* SMMU Translation Table Base Control Register for Non-Secure Context BankCan be accessed in Non-Secure mode */
#define SMMU_COMN_SMMU_OFFSET_ADDR_NS_REG    0x210   /* SMMU Offset Address Register for Non-Secure Context BankCan be accessed in Non-Secure mode */
#define SMMU_COMN_SMMU_SCACHEI_ALL_REG       0x214   /* Cache Invalid Register of all invalidation */
#define SMMU_COMN_SMMU_SCACHEI_L1_REG        0x218   /* Cache Invalid Register of Level1 invalidation */
#define SMMU_COMN_SMMU_SCACHEI_L2L3_REG      0x21C   /* Cache Invalid Register of Level2 and Level3 invalidation */
#define SMMU_COMN_SMMU_FAMA_CTRL0_NS_REG     0x220   /* SMMU Control Register for FAMA for TBU of Non-Secure Context Bank */
#define SMMU_COMN_SMMU_FAMA_CTRL1_NS_REG     0x224   /* SMMU Control Register for FAMA for TCU of Non-Secure Context Bank */
#define SMMU_COMN_SMMU_ADDR_MSB_REG          0x300   /* Register for MSB of all 33-bits address configuration */
#define SMMU_COMN_SMMU_ERR_RDADDR_REG        0x304   /* SMMU Error Address of TLB miss for Read transaction */
#define SMMU_COMN_SMMU_ERR_WRADDR_REG        0x308   /* SMMU Error Address of TLB miss for Write transaction */
#define SMMU_COMN_SMMU_FAULT_ADDR_TCU_REG    0x310   /* Register of record of address information for the error scenario of TCU */
#define SMMU_COMN_SMMU_FAULT_ID_TCU_REG      0x314   /* Register of record of stream id and index id information for the error scenario of TCU */
#define SMMU_COMN_SMMU_FAULT_ADDR_TBUX_0_REG 0x320   /* Register of record of address information for the error scenariof is the number of TBU port */
#define SMMU_COMN_SMMU_FAULT_ADDR_TBUX_1_REG 0x330   /* Register of record of address information for the error scenariof is the number of TBU port */
#define SMMU_COMN_SMMU_FAULT_ADDR_TBUX_2_REG 0x340   /* Register of record of address information for the error scenariof is the number of TBU port */
#define SMMU_COMN_SMMU_FAULT_ADDR_TBUX_3_REG 0x350   /* Register of record of address information for the error scenariof is the number of TBU port */
#define SMMU_COMN_SMMU_FAULT_ID_TBUX_0_REG   0x324   /* Register of record of stream id and index id information for the error scenariof is the number of TBU port */
#define SMMU_COMN_SMMU_FAULT_ID_TBUX_1_REG   0x334   /* Register of record of stream id and index id information for the error scenariof is the number of TBU port */
#define SMMU_COMN_SMMU_FAULT_ID_TBUX_2_REG   0x344   /* Register of record of stream id and index id information for the error scenariof is the number of TBU port */
#define SMMU_COMN_SMMU_FAULT_ID_TBUX_3_REG   0x354   /* Register of record of stream id and index id information for the error scenariof is the number of TBU port */
#define SMMU_COMN_SMMU_FAULT_INFOX_0_REG     0x328   /* Register of record of  faults of TBUf is the number of TBU port */
#define SMMU_COMN_SMMU_FAULT_INFOX_1_REG     0x338   /* Register of record of  faults of TBUf is the number of TBU port */
#define SMMU_COMN_SMMU_FAULT_INFOX_2_REG     0x348   /* Register of record of  faults of TBUf is the number of TBU port */
#define SMMU_COMN_SMMU_FAULT_INFOX_3_REG     0x358   /* Register of record of  faults of TBUf is the number of TBU port */
#define SMMU_COMN_SMMU_DBGRPTR_TLB_REG       0x380   /* SMMU Debug Pointer of TLB */
#define SMMU_COMN_SMMU_DBGRDATA_TLB_REG      0x384   /* SMMU Debug Data of TLB */
#define SMMU_COMN_SMMU_DBGRPTR_CACHE_REG     0x388   /* SMMU Debug Pointer of Cache */
#define SMMU_COMN_SMMU_DBGRDATA0_CACHE_REG   0x38C   /* SMMU Debug Data of Cache */
#define SMMU_COMN_SMMU_DBGRDATA1_CACHE_REG   0x390   /* SMMU Debug Data of Cache */
#define SMMU_COMN_SMMU_DBGAXI_CTRL_REG       0x394   /* SMMU Debug of AXI interface control */
#define SMMU_COMN_SMMU_OVA_ADDR_REG          0x398   /* SMMU Override of VA address */
#define SMMU_COMN_SMMU_OPA_ADDR_REG          0x39C   /* SMMU Override of PA address */
#define SMMU_COMN_SMMU_OVA_CTRL_REG          0x3A0   /* SMMU Override of VA configuration */
#define SMMU_COMN_SMMU_OPREF_ADDR_REG        0x3A4   /* SMMU Override of Prefetch address */
#define SMMU_COMN_SMMU_OPREF_CTRL_REG        0x3A8   /* SMMU Override of Prefetch configuration */
#define SMMU_COMN_SMMU_OPREF_CNT_REG         0x3AC   /* Counter for Override of prefetch */
#define SMMU_COMN_SMMU_SMRX_S_0_REG          0x500   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_1_REG          0x504   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_2_REG          0x508   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_3_REG          0x50C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_4_REG          0x510   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_5_REG          0x514   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_6_REG          0x518   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_7_REG          0x51C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_8_REG          0x520   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_9_REG          0x524   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_10_REG         0x528   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_11_REG         0x52C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_12_REG         0x530   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_13_REG         0x534   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_14_REG         0x538   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_15_REG         0x53C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_16_REG         0x540   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_17_REG         0x544   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_18_REG         0x548   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_19_REG         0x54C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_20_REG         0x550   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_21_REG         0x554   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_22_REG         0x558   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_23_REG         0x55C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_24_REG         0x560   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_25_REG         0x564   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_26_REG         0x568   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_27_REG         0x56C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_28_REG         0x570   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_29_REG         0x574   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_30_REG         0x578   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_31_REG         0x57C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_32_REG         0x580   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_33_REG         0x584   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_34_REG         0x588   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_35_REG         0x58C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_36_REG         0x590   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_37_REG         0x594   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_38_REG         0x598   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_39_REG         0x59C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_40_REG         0x5A0   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_41_REG         0x5A4   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_42_REG         0x5A8   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_43_REG         0x5AC   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_44_REG         0x5B0   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_45_REG         0x5B4   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_46_REG         0x5B8   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_47_REG         0x5BC   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_48_REG         0x5C0   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_49_REG         0x5C4   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_50_REG         0x5C8   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_51_REG         0x5CC   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_52_REG         0x5D0   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_53_REG         0x5D4   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_54_REG         0x5D8   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_55_REG         0x5DC   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_56_REG         0x5E0   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_57_REG         0x5E4   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_58_REG         0x5E8   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_59_REG         0x5EC   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_60_REG         0x5F0   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_61_REG         0x5F4   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_62_REG         0x5F8   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_63_REG         0x5FC   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_64_REG         0x600   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_65_REG         0x604   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_66_REG         0x608   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_67_REG         0x60C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_68_REG         0x610   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_69_REG         0x614   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_70_REG         0x618   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_71_REG         0x61C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_72_REG         0x620   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_73_REG         0x624   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_74_REG         0x628   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_75_REG         0x62C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_76_REG         0x630   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_77_REG         0x634   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_78_REG         0x638   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_79_REG         0x63C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_80_REG         0x640   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_81_REG         0x644   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_82_REG         0x648   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_83_REG         0x64C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_84_REG         0x650   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_85_REG         0x654   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_86_REG         0x658   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_S_87_REG         0x65C   /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_RLD_EN0_S_REG         0x6F0   /* SMMU SMR Reload Enable Register0 */
#define SMMU_COMN_SMMU_RLD_EN1_S_REG         0x6F4   /* SMMU SMR Reload Enable Register1 */
#define SMMU_COMN_SMMU_RLD_EN2_S_REG         0x6F8   /* SMMU SMR Reload Enable Register2 */
#define SMMU_COMN_SMMU_INTMAS_S_REG          0x700   /* SMMU Interrupt Mask Register for Secure contest bank */
#define SMMU_COMN_SMMU_INTRAW_S_REG          0x704   /* SMMU Interrupt Raw Status for Secure contest bank */
#define SMMU_COMN_SMMU_INTSTAT_S_REG         0x708   /* SMMU Interrupt Status after Mask for Secure contest bank */
#define SMMU_COMN_SMMU_INTCLR_S_REG          0x70C   /* SMMU Interrupt Clear Register for Secure contest bank */
#define SMMU_COMN_SMMU_SCR_S_REG             0x710   /* SMMU Global Control Register */
#define SMMU_COMN_SMMU_SCB_SCTRL_REG         0x714   /* SMMU System Control Register for Secure Context Bank */
#define SMMU_COMN_SMMU_SCB_TTBR_REG          0x718   /* SMMU Translation Table Base Register for Secure Context Bank */
#define SMMU_COMN_SMMU_SCB_TTBCR_REG         0x71C   /* SMMU Translation Table Base Control Register for Secure Context Bank */
#define SMMU_COMN_SMMU_OFFSET_ADDR_S_REG     0x720   /* SMMU Offset Address Register for Secure Context Bank */
#define SMMU_COMN_SMMU_FAMA_CTRL0_S_REG      0x724   /* SMMU Control Register for FAMA for TBU of Secure Context Bank */
#define SMMU_COMN_SMMU_FAMA_CTRL1_S_REG      0x728   /* SMMU Control Register for FAMA for TCU of Secure Context Bank */
#define SMMU_COMN_SMMU_DBGRPTR_TLB_S_REG     0x72C   /* SMMU Debug Pointer of TLB */
#define SMMU_COMN_SMMU_DBGRPTR_CACHE_S_REG   0x730   /* SMMU Debug Pointer of Cache */
#define SMMU_COMN_SMMU_OVERRIDE_CTRL_S_REG   0x734   /* SMMU Override security control */
#define SMMU_COMN_SMMU_SMRX_P_0_REG          0x10000 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_1_REG          0x10004 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_2_REG          0x10008 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_3_REG          0x1000C /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_4_REG          0x10010 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_5_REG          0x10014 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_6_REG          0x10018 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_7_REG          0x1001C /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_8_REG          0x10020 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_9_REG          0x10024 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_10_REG         0x10028 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_11_REG         0x1002C /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_12_REG         0x10030 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_13_REG         0x10034 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_14_REG         0x10038 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_15_REG         0x1003C /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_16_REG         0x10040 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_17_REG         0x10044 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_18_REG         0x10048 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_19_REG         0x1004C /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_20_REG         0x10050 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_21_REG         0x10054 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_22_REG         0x10058 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_23_REG         0x1005C /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_24_REG         0x10060 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_25_REG         0x10064 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_26_REG         0x10068 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_27_REG         0x1006C /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_28_REG         0x10070 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_29_REG         0x10074 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_30_REG         0x10078 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_31_REG         0x1007C /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_32_REG         0x10080 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_33_REG         0x10084 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_34_REG         0x10088 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_35_REG         0x1008C /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_36_REG         0x10090 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_37_REG         0x10094 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_38_REG         0x10098 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_39_REG         0x1009C /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_40_REG         0x100A0 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_41_REG         0x100A4 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_42_REG         0x100A8 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_43_REG         0x100AC /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_44_REG         0x100B0 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_45_REG         0x100B4 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_46_REG         0x100B8 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_47_REG         0x100BC /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_48_REG         0x100C0 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_49_REG         0x100C4 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_50_REG         0x100C8 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_51_REG         0x100CC /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_52_REG         0x100D0 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_53_REG         0x100D4 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_54_REG         0x100D8 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_55_REG         0x100DC /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_56_REG         0x100E0 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_57_REG         0x100E4 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_58_REG         0x100E8 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_59_REG         0x100EC /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_60_REG         0x100F0 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_61_REG         0x100F4 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_62_REG         0x100F8 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_63_REG         0x100FC /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_64_REG         0x10100 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_65_REG         0x10104 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_66_REG         0x10108 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_67_REG         0x1010C /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_68_REG         0x10110 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_69_REG         0x10114 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_70_REG         0x10118 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_71_REG         0x1011C /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_72_REG         0x10120 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_73_REG         0x10124 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_74_REG         0x10128 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_75_REG         0x1012C /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_76_REG         0x10130 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_77_REG         0x10134 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_78_REG         0x10138 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_79_REG         0x1013C /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_80_REG         0x10140 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_81_REG         0x10144 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_82_REG         0x10148 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_83_REG         0x1014C /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_84_REG         0x10150 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_85_REG         0x10154 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_86_REG         0x10158 /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_SMRX_P_87_REG         0x1015C /* SMMU Control Register per Stream */
#define SMMU_COMN_SMMU_RLD_EN0_P_REG         0x101F0 /* SMMU SMR Reload Enable Register0 */
#define SMMU_COMN_SMMU_RLD_EN1_P_REG         0x101F4 /* SMMU SMR Reload Enable Register1 */
#define SMMU_COMN_SMMU_RLD_EN2_P_REG         0x101F8 /* SMMU SMR Reload Enable Register2 */
#define SMMU_COMN_SMMU_INTMAS_P_REG          0x10200 /* SMMU Interrupt Mask Register for Protected contest bank */
#define SMMU_COMN_SMMU_INTRAW_P_REG          0x10204 /* SMMU Interrupt Raw Status for Protected contest bank */
#define SMMU_COMN_SMMU_INTSTAT_P_REG         0x10208 /* SMMU Interrupt Status after Mask for Protected contest bank */
#define SMMU_COMN_SMMU_INTCLR_P_REG          0x1020C /* SMMU Interrupt Clear Register for Protected contest bank */
#define SMMU_COMN_SMMU_SCR_P_REG             0x10210 /* SMMU Global Control Register for Protected Context Bank */
#define SMMU_COMN_SMMU_PCB_SCTRL_REG         0x10214 /* SMMU System Control Register for Protected Context Bank */
#define SMMU_COMN_SMMU_PCB_TTBR_REG          0x10218 /* SMMU Translation Table Base Register for Protected Context Bank */
#define SMMU_COMN_SMMU_PCB_TTBCR_REG         0x1021C /* SMMU Translation Table Base Control Register for Protected Context Bank */
#define SMMU_COMN_SMMU_OFFSET_ADDR_P_REG     0x10220 /* SMMU Offset Address Register for Protected Context Bank */
#define SMMU_COMN_SMMU_FAMA_CTRL0_P_REG      0x10224 /* SMMU Control Register for FAMA for TBU of Protected Context Bank */
#define SMMU_COMN_SMMU_FAMA_CTRL1_P_REG      0x10228 /* SMMU Control Register for FAMA for TCU of Protected Context Bank */

#endif /* __SMMU_COMN_REG_OFFSET_H__ */
