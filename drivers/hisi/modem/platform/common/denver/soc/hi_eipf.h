#ifndef __IPF_REG_OFFSET_H__
#define __IPF_REG_OFFSET_H__ 
#define IPF_REG_FILTER_NUM (0x20 * 128)
#define IPF_REG_TABLE_NUM (0x80)
#define IPF_REG_FILTER_SIZE (IPF_REG_FILTER_NUM * 4)
#define IPF_REG_TABLE_SIZE (IPF_REG_TABLE_NUM * 4)
#define IPF_SRST_REG (0x0)
#define IPF_SRST_STATE_REG (0x4)
#define IPF_CH_EN_REG (0x8)
#define IPF_EN_STATE_REG (0xC)
#define IPF_GATE_REG (0x10)
#define IPF_CTRL_REG (0x14)
#define IPF_DMA_CTRL0_REG (0x18)
#define IPF_DMA_CTRL1_REG (0x1C)
#define IPF_VERSION_REG (0x20)
#define IPF_STATE0_REG (0x24)
#define IPF_STATE1_REG (0x28)
#define IPF_INT_CTRL_REG (0x2C)
#define IPF_INT0_SUP_TIME_REG (0x30)
#define IPF_INT1_SUP_TIME_REG (0x34)
#define IPF_INT2_SUP_TIME_REG (0x38)
#define IPF_GEN_INT_REG (0x3C)
#define IPF_INT0_UL_REG (0x40)
#define IPF_INT0_DL_REG (0x44)
#define IPF_INT1_UL_REG (0x48)
#define IPF_INT1_DL_REG (0x4C)
#define IPF_INT2_UL_REG (0x50)
#define IPF_INT2_DL_REG (0x54)
#define IPF_INT0_EN_UL_REG (0x58)
#define IPF_INT0_EN_DL_REG (0x5C)
#define IPF_INT1_EN_UL_REG (0x60)
#define IPF_INT1_EN_DL_REG (0x64)
#define IPF_INT2_EN_UL_REG (0x68)
#define IPF_INT2_EN_DL_REG (0x6C)
#define IPF_INT_STATE_UL_REG (0x70)
#define IPF_INT_STATE_DL_REG (0x74)
#define IPF_FIFO_STATE0_REG (0x78)
#define IPF_FIFO_STATE1_REG (0x7C)
#define IPF_FIFO_STATE2_REG (0x80)
#define IPF_FIFO_STATE3_REG (0x84)
#define IPF_FIFO_STATE4_REG (0x88)
#define IPF_FIFO_STATE5_REG (0x8C)
#define IPF_TIME_OUT_REG (0xA0)
#define IPF_PKT_LEN_REG (0xA4)
#define IPF_FILTER_ZERO_INDEX_REG (0xA8)
#define IPF_EF_BADDR_L_REG (0xAC)
#define IPF_EF_BADDR_H_REG (0xB0)
#define FLT_CHAIN_LOOP_REG (0xB4)
#define IPF_TRANS_CNT_CTRL_REG (0xB8)
#define IPF_TIMER_LOAD_REG (0xBC)
#define IPF_TIMER_COUNTER_REG (0xC0)
#define IPF_CH0_PKT_CNT0_REG (0xC4)
#define IPF_CH0_PKT_CNT1_REG (0xC8)
#define IPF_CH1_PKT_CNT_REG (0xCC)
#define IPF_CH1_SPEC_FLT_RABID0_REG (0xD0)
#define IPF_CH1_SPEC_FLT_RABID1_REG (0xD4)
#define IPF_CH1_SPEC_FLT_RABID2_REG (0xD8)
#define IPF_CH0_CTRL_REG (0x100)
#define IPF_CH0_STATE_REG (0x104)
#define IPF_CH0_BDQ0_BADDR_L_REG (0x108)
#define IPF_CH0_BDQ0_BADDR_H_REG (0x10C)
#define IPF_CH0_BDQ0_SIZE_REG (0x110)
#define IPF_CH0_BDQ0_RPTR_REG (0x114)
#define IPF_CH0_BDQ0_WPTR_REG (0x118)
#define IPF_CH0_BDQ0_WADDR_L_REG (0x11C)
#define IPF_CH0_BDQ0_WADDR_H_REG (0x120)
#define IPF_CH0_BDQ0_RADDR_L_REG (0x124)
#define IPF_CH0_BDQ0_RADDR_H_REG (0x128)
#define IPF_CH0_BDQ0_DEPTH_REG (0x12C)
#define IPF_CH0_BDQ0_RPTR_UPDATE_ADDR_L_REG (0x130)
#define IPF_CH0_BDQ0_RPTR_UPDATE_ADDR_H_REG (0x134)
#define IPF_CH0_BDQ1_BADDR_L_REG (0x138)
#define IPF_CH0_BDQ1_BADDR_H_REG (0x13C)
#define IPF_CH0_BDQ1_SIZE_REG (0x140)
#define IPF_CH0_BDQ1_RPTR_REG (0x144)
#define IPF_CH0_BDQ1_WPTR_REG (0x148)
#define IPF_CH0_BDQ1_WADDR_L_REG (0x14C)
#define IPF_CH0_BDQ1_WADDR_H_REG (0x150)
#define IPF_CH0_BDQ1_RADDR_L_REG (0x154)
#define IPF_CH0_BDQ1_RADDR_H_REG (0x158)
#define IPF_CH0_BDQ1_DEPTH_REG (0x15C)
#define IPF_CH0_BDQ1_RPTR_UPDATE_ADDR_L_REG (0x160)
#define IPF_CH0_BDQ1_RPTR_UPDATE_ADDR_H_REG (0x164)
#define IPF_CH1_CTRL_REG (0x168)
#define IPF_CH1_STATE_REG (0x16C)
#define IPF_CH1_RDQ_BADDR_L_REG (0x170)
#define IPF_CH1_RDQ_BADDR_H_REG (0x174)
#define IPF_CH1_RDQ_SIZE_REG (0x178)
#define IPF_CH1_RDQ_WPTR_REG (0x17C)
#define IPF_CH1_RDQ_RPTR_REG (0x180)
#define IPF_CH1_RDQ_WADDR_L_REG (0x184)
#define IPF_CH1_RDQ_WADDR_H_REG (0x188)
#define IPF_CH1_RDQ_RADDR_L_REG (0x18C)
#define IPF_CH1_RDQ_RADDR_H_REG (0x190)
#define IPF_CH1_RDQ_DEPTH_REG (0x194)
#define IPF_CH1_RDQ_WPTR_UPDATE_ADDR_L_REG (0x198)
#define IPF_CH1_RDQ_WPTR_UPDATE_ADDR_H_REG (0x19C)
#define IPF_CH1_ADQ_EN_CTRL_REG (0x1A0)
#define IPF_CH1_ADQ_SIZE_CTRL_REG (0x1A4)
#define IPF_CH1_ADQ0_BASE_L_REG (0x1A8)
#define IPF_CH1_ADQ0_BASE_H_REG (0x1AC)
#define IPF_CH1_ADQ0_STAT_REG (0x1B0)
#define IPF_CH1_ADQ0_WPTR_REG (0x1B4)
#define IPF_CH1_ADQ0_RPTR_REG (0x1B8)
#define IPF_CH1_ADQ0_RPTR_UPDATE_ADDR_L_REG (0x1BC)
#define IPF_CH1_ADQ0_RPTR_UPDATE_ADDR_H_REG (0x1C0)
#define IPF_CH1_ADQ1_BASE_L_REG (0x1C4)
#define IPF_CH1_ADQ1_BASE_H_REG (0x1C8)
#define IPF_CH1_ADQ1_STAT_REG (0x1CC)
#define IPF_CH1_ADQ1_WPTR_REG (0x1D0)
#define IPF_CH1_ADQ1_RPTR_REG (0x1D4)
#define IPF_CH1_ADQ1_RPTR_UPDATE_ADDR_L_REG (0x1D8)
#define IPF_CH1_ADQ1_RPTR_UPDATE_ADDR_H_REG (0x1DC)
#define IPF_CH1_RNRDQ_BADDR_L_REG (0x1E0)
#define IPF_CH1_RNRDQ_BADDR_H_REG (0x1E4)
#define IPF_CH1_RNRDQ_SIZE_REG (0x1E8)
#define IPF_CH1_RNRDQ_WPTR_REG (0x1EC)
#define IPF_CH1_RNRDQ_RPTR_REG (0x1F0)
#define IPF_CH1_RNRDQ_WADDR_L_REG (0x1F4)
#define IPF_CH1_RNRDQ_WADDR_H_REG (0x1F8)
#define IPF_CH1_RNRDQ_RADDR_L_REG (0x1FC)
#define IPF_CH1_RNRDQ_RADDR_H_REG (0x200)
#define IPF_CH1_RNRDQ_DEPTH_REG (0x204)
#define IPF_CH1_RNRDQ_WPTR_UPDATE_ADDR_L_REG (0x208)
#define IPF_CH1_RNRDQ_WPTR_UPDATE_ADDR_H_REG (0x20C)
#define IPF_CH1_RARDQ_BADDR_L_REG (0x210)
#define IPF_CH1_RARDQ_BADDR_H_REG (0x214)
#define IPF_CH1_RARDQ_SIZE_REG (0x218)
#define IPF_CH1_RARDQ_WPTR_REG (0x21C)
#define IPF_CH1_RARDQ_RPTR_REG (0x220)
#define IPF_CH1_RARDQ_WADDR_L_REG (0x224)
#define IPF_CH1_RARDQ_WADDR_H_REG (0x228)
#define IPF_CH1_RARDQ_RADDR_L_REG (0x22C)
#define IPF_CH1_RARDQ_RADDR_H_REG (0x230)
#define IPF_CH1_RARDQ_DEPTH_REG (0x234)
#define IPF_CH1_RARDQ_WPTR_UPDATE_ADDR_L_REG (0x238)
#define IPF_CH1_RARDQ_WPTR_UPDATE_ADDR_H_REG (0x23C)
#define FRAG_WPTR_REG (0x240)
#define TBL_UPDATE_REQ_REG (0x24C)
#define TBL_UPDATE_ACK_REG (0x250)
#define FRAG_TLB_EN_IPV4_REG (0x25C)
#define FRAG_TLB_EN_IPV6_REG (0x260)
#define TBL_QOS_DRB_EN0_REG (0x264)
#define TBL_QOS_DRB_EN1_REG (0x268)
#define TBL_PDU_DRB_EN_REG (0x26C)
#define TBL_RABID_DRB_EN0_REG (0x270)
#define TBL_RABID_DRB_EN1_REG (0x274)
#define IPF_UL_RDQ0_FULL_CNT_REG (0x400)
#define IPF_UL_RDQ1_FULL_CNT_REG (0x404)
#define IPF_UL_BDQ0_EPTY_CNT_REG (0x408)
#define IPF_UL_BDQ1_EPTY_CNT_REG (0x40C)
#define IPF_UL_ADQ0_EPTY_CNT_REG (0x410)
#define IPF_UL_ADQ1_EPTY_CNT_REG (0x414)
#define IPF_DL_RDQ_FULL_CNT_REG (0x418)
#define IPF_DL_BDQ0_EPTY_CNT_REG (0x41C)
#define IPF_DL_BDQ1_EPTY_CNT_REG (0x420)
#define IPF_DL_RNRDQ_FULL_CNT_REG (0x424)
#define IPF_DL_RARDQ_FULL_CNT_REG (0x428)
#define IPF_DL_ADQ0_EPTY_CNT_REG (0x42C)
#define IPF_DL_ADQ1_EPTY_CNT_REG (0x430)
#define TBLM_QOS_REFL_0_REG (0x500)
#define TBLM_QOS_REFL_1_REG (0x504)
#define TBLM_QOS_REFL_2_REG (0x508)
#define TBLM_QOS_REFL_3_REG (0x50C)
#define TBLM_QOS_REFL_4_REG (0x510)
#define TBLM_QOS_REFL_5_REG (0x514)
#define TBLM_QOS_REFL_6_REG (0x518)
#define TBLM_QOS_REFL_7_REG (0x51C)
#define TBLM_QOS_REFL_8_REG (0x520)
#define TBLM_QOS_REFL_9_REG (0x524)
#define TBLM_QOS_REFL_10_REG (0x528)
#define TBLM_QOS_REFL_11_REG (0x52C)
#define TBLM_QOS_REFL_12_REG (0x530)
#define TBLM_QOS_REFL_13_REG (0x534)
#define TBLM_QOS_REFL_14_REG (0x538)
#define TBLM_QOS_REFL_15_REG (0x53C)
#define TBLM_QOS_REFL_16_REG (0x540)
#define TBLM_QOS_REFL_17_REG (0x544)
#define TBLM_QOS_REFL_18_REG (0x548)
#define TBLM_QOS_REFL_19_REG (0x54C)
#define TBLM_QOS_REFL_20_REG (0x550)
#define TBLM_QOS_REFL_21_REG (0x554)
#define TBLM_QOS_REFL_22_REG (0x558)
#define TBLM_QOS_REFL_23_REG (0x55C)
#define TBLM_QOS_REFL_24_REG (0x560)
#define TBLM_QOS_REFL_25_REG (0x564)
#define TBLM_QOS_REFL_26_REG (0x568)
#define TBLM_QOS_REFL_27_REG (0x56C)
#define TBLM_QOS_REFL_28_REG (0x570)
#define TBLM_QOS_REFL_29_REG (0x574)
#define TBLM_QOS_REFL_30_REG (0x578)
#define TBLM_QOS_REFL_31_REG (0x57C)
#define TBLM_QOS_REFL_32_REG (0x580)
#define TBLM_QOS_REFL_33_REG (0x584)
#define TBLM_QOS_REFL_34_REG (0x588)
#define TBLM_QOS_REFL_35_REG (0x58C)
#define TBLM_QOS_REFL_36_REG (0x590)
#define TBLM_QOS_REFL_37_REG (0x594)
#define TBLM_QOS_REFL_38_REG (0x598)
#define TBLM_QOS_REFL_39_REG (0x59C)
#define TBLM_QOS_REFL_40_REG (0x5A0)
#define TBLM_QOS_REFL_41_REG (0x5A4)
#define TBLM_QOS_REFL_42_REG (0x5A8)
#define TBLM_QOS_REFL_43_REG (0x5AC)
#define TBLM_QOS_REFL_44_REG (0x5B0)
#define TBLM_QOS_REFL_45_REG (0x5B4)
#define TBLM_QOS_REFL_46_REG (0x5B8)
#define TBLM_QOS_REFL_47_REG (0x5BC)
#define TBLM_QOS_REFL_48_REG (0x5C0)
#define TBLM_QOS_REFL_49_REG (0x5C4)
#define TBLM_QOS_REFL_50_REG (0x5C8)
#define TBLM_QOS_REFL_51_REG (0x5CC)
#define TBLM_QOS_REFL_52_REG (0x5D0)
#define TBLM_QOS_REFL_53_REG (0x5D4)
#define TBLM_QOS_REFL_54_REG (0x5D8)
#define TBLM_QOS_REFL_55_REG (0x5DC)
#define TBLM_QOS_REFL_56_REG (0x5E0)
#define TBLM_QOS_REFL_57_REG (0x5E4)
#define TBLM_QOS_REFL_58_REG (0x5E8)
#define TBLM_QOS_REFL_59_REG (0x5EC)
#define TBLM_QOS_REFL_60_REG (0x5F0)
#define TBLM_QOS_REFL_61_REG (0x5F4)
#define TBLM_QOS_REFL_62_REG (0x5F8)
#define TBLM_QOS_REFL_63_REG (0x5FC)
#define TBLK_PDU_REFL_0_REG (0x600)
#define TBLK_PDU_REFL_1_REG (0x604)
#define TBLK_PDU_REFL_2_REG (0x608)
#define TBLK_PDU_REFL_3_REG (0x60C)
#define TBLK_PDU_REFL_4_REG (0x610)
#define TBLK_PDU_REFL_5_REG (0x614)
#define TBLK_PDU_REFL_6_REG (0x618)
#define TBLK_PDU_REFL_7_REG (0x61C)
#define TBLK_PDU_REFL_8_REG (0x620)
#define TBLK_PDU_REFL_9_REG (0x624)
#define TBLK_PDU_REFL_10_REG (0x628)
#define TBLK_PDU_REFL_11_REG (0x62C)
#define TBLK_PDU_REFL_12_REG (0x630)
#define TBLK_PDU_REFL_13_REG (0x634)
#define TBLK_PDU_REFL_14_REG (0x638)
#define TBLK_PDU_REFL_15_REG (0x63C)
#define TBLK_PDU_REFL_16_REG (0x640)
#define TBLK_PDU_REFL_17_REG (0x644)
#define TBLK_PDU_REFL_18_REG (0x648)
#define TBLK_PDU_REFL_19_REG (0x64C)
#define TBLK_PDU_REFL_20_REG (0x650)
#define TBLK_PDU_REFL_21_REG (0x654)
#define TBLK_PDU_REFL_22_REG (0x658)
#define TBLK_PDU_REFL_23_REG (0x65C)
#define TBLK_PDU_REFL_24_REG (0x660)
#define TBLK_PDU_REFL_25_REG (0x664)
#define TBLK_PDU_REFL_26_REG (0x668)
#define TBLK_PDU_REFL_27_REG (0x66C)
#define TBLK_PDU_REFL_28_REG (0x670)
#define TBLK_PDU_REFL_29_REG (0x674)
#define TBLK_PDU_REFL_30_REG (0x678)
#define TBLK_PDU_REFL_31_REG (0x67C)
#define TBLM_RAB_REFL_0_REG (0x680)
#define TBLM_RAB_REFL_1_REG (0x684)
#define TBLM_RAB_REFL_2_REG (0x688)
#define TBLM_RAB_REFL_3_REG (0x68C)
#define TBLM_RAB_REFL_4_REG (0x690)
#define TBLM_RAB_REFL_5_REG (0x694)
#define TBLM_RAB_REFL_6_REG (0x698)
#define TBLM_RAB_REFL_7_REG (0x69C)
#define TBLM_RAB_REFL_8_REG (0x6A0)
#define TBLM_RAB_REFL_9_REG (0x6A4)
#define TBLM_RAB_REFL_10_REG (0x6A8)
#define TBLM_RAB_REFL_11_REG (0x6AC)
#define TBLM_RAB_REFL_12_REG (0x6B0)
#define TBLM_RAB_REFL_13_REG (0x6B4)
#define TBLM_RAB_REFL_14_REG (0x6B8)
#define TBLM_RAB_REFL_15_REG (0x6BC)
#define TBLM_RAB_REFL_16_REG (0x6C0)
#define TBLM_RAB_REFL_17_REG (0x6C4)
#define TBLM_RAB_REFL_18_REG (0x6C8)
#define TBLM_RAB_REFL_19_REG (0x6CC)
#define TBLM_RAB_REFL_20_REG (0x6D0)
#define TBLM_RAB_REFL_21_REG (0x6D4)
#define TBLM_RAB_REFL_22_REG (0x6D8)
#define TBLM_RAB_REFL_23_REG (0x6DC)
#define TBLM_RAB_REFL_24_REG (0x6E0)
#define TBLM_RAB_REFL_25_REG (0x6E4)
#define TBLM_RAB_REFL_26_REG (0x6E8)
#define TBLM_RAB_REFL_27_REG (0x6EC)
#define TBLM_RAB_REFL_28_REG (0x6F0)
#define TBLM_RAB_REFL_29_REG (0x6F4)
#define TBLM_RAB_REFL_30_REG (0x6F8)
#define TBLM_RAB_REFL_31_REG (0x6FC)
#define TBLM_RAB_REFL_32_REG (0x700)
#define TBLM_RAB_REFL_33_REG (0x704)
#define TBLM_RAB_REFL_34_REG (0x708)
#define TBLM_RAB_REFL_35_REG (0x70C)
#define TBLM_RAB_REFL_36_REG (0x710)
#define TBLM_RAB_REFL_37_REG (0x714)
#define TBLM_RAB_REFL_38_REG (0x718)
#define TBLM_RAB_REFL_39_REG (0x71C)
#define TBLM_RAB_REFL_40_REG (0x720)
#define TBLM_RAB_REFL_41_REG (0x724)
#define TBLM_RAB_REFL_42_REG (0x728)
#define TBLM_RAB_REFL_43_REG (0x72C)
#define TBLM_RAB_REFL_44_REG (0x730)
#define TBLM_RAB_REFL_45_REG (0x734)
#define TBLM_RAB_REFL_46_REG (0x738)
#define TBLM_RAB_REFL_47_REG (0x73C)
#define TBLM_RAB_REFL_48_REG (0x740)
#define TBLM_RAB_REFL_49_REG (0x744)
#define TBLM_RAB_REFL_50_REG (0x748)
#define TBLM_RAB_REFL_51_REG (0x74C)
#define TBLM_RAB_REFL_52_REG (0x750)
#define TBLM_RAB_REFL_53_REG (0x754)
#define TBLM_RAB_REFL_54_REG (0x758)
#define TBLM_RAB_REFL_55_REG (0x75C)
#define TBLM_RAB_REFL_56_REG (0x760)
#define TBLM_RAB_REFL_57_REG (0x764)
#define TBLM_RAB_REFL_58_REG (0x768)
#define TBLM_RAB_REFL_59_REG (0x76C)
#define TBLM_RAB_REFL_60_REG (0x770)
#define TBLM_RAB_REFL_61_REG (0x774)
#define TBLM_RAB_REFL_62_REG (0x778)
#define TBLM_RAB_REFL_63_REG (0x77C)
#define IPF_SEC_ATTR_REG (0x800)
#define IPF_CH0_RDQ0_BADDR_L_REG (0x804)
#define IPF_CH0_RDQ0_BADDR_H_REG (0x808)
#define IPF_CH0_RDQ0_SIZE_REG (0x80C)
#define IPF_CH0_RDQ0_WPTR_REG (0x810)
#define IPF_CH0_RDQ0_RPTR_REG (0x814)
#define IPF_CH0_RDQ0_WADDR_L_REG (0x818)
#define IPF_CH0_RDQ0_WADDR_H_REG (0x81C)
#define IPF_CH0_RDQ0_RADDR_L_REG (0x820)
#define IPF_CH0_RDQ0_RADDR_H_REG (0x824)
#define IPF_CH0_RDQ0_DEPTH_REG (0x828)
#define IPF_CH0_RDQ0_WPTR_UPDATE_ADDR_L_REG (0x82C)
#define IPF_CH0_RDQ0_WPTR_UPDATE_ADDR_H_REG (0x830)
#define IPF_CH0_RDQ1_BADDR_L_REG (0x834)
#define IPF_CH0_RDQ1_BADDR_H_REG (0x838)
#define IPF_CH0_RDQ1_SIZE_REG (0x83C)
#define IPF_CH0_RDQ1_WPTR_REG (0x840)
#define IPF_CH0_RDQ1_RPTR_REG (0x844)
#define IPF_CH0_RDQ1_WADDR_L_REG (0x848)
#define IPF_CH0_RDQ1_WADDR_H_REG (0x84C)
#define IPF_CH0_RDQ1_RADDR_L_REG (0x850)
#define IPF_CH0_RDQ1_RADDR_H_REG (0x854)
#define IPF_CH0_RDQ1_DEPTH_REG (0x858)
#define IPF_CH0_RDQ1_WPTR_UPDATE_ADDR_L_REG (0x85C)
#define IPF_CH0_RDQ1_WPTR_UPDATE_ADDR_H_REG (0x860)
#define IPF_CH1_BDQ0_BADDR_L_REG (0x864)
#define IPF_CH1_BDQ0_BADDR_H_REG (0x868)
#define IPF_CH1_BDQ0_SIZE_REG (0x86C)
#define IPF_CH1_BDQ0_WPTR_REG (0x870)
#define IPF_CH1_BDQ0_RPTR_REG (0x874)
#define IPF_CH1_BDQ0_WADDR_L_REG (0x878)
#define IPF_CH1_BDQ0_WADDR_H_REG (0x87C)
#define IPF_CH1_BDQ0_RADDR_L_REG (0x880)
#define IPF_CH1_BDQ0_RADDR_H_REG (0x884)
#define IPF_CH1_BDQ0_DEPTH_REG (0x888)
#define IPF_CH1_BDQ0_RPTR_UPDATE_ADDR_L_REG (0x88C)
#define IPF_CH1_BDQ0_RPTR_UPDATE_ADDR_H_REG (0x890)
#define IPF_CH1_BDQ1_BADDR_L_REG (0x894)
#define IPF_CH1_BDQ1_BADDR_H_REG (0x898)
#define IPF_CH1_BDQ1_SIZE_REG (0x89C)
#define IPF_CH1_BDQ1_WPTR_REG (0x8A0)
#define IPF_CH1_BDQ1_RPTR_REG (0x8A4)
#define IPF_CH1_BDQ1_WADDR_L_REG (0x8A8)
#define IPF_CH1_BDQ1_WADDR_H_REG (0x8AC)
#define IPF_CH1_BDQ1_RADDR_L_REG (0x8B0)
#define IPF_CH1_BDQ1_RADDR_H_REG (0x8B4)
#define IPF_CH1_BDQ1_DEPTH_REG (0x8B8)
#define IPF_CH1_BDQ1_RPTR_UPDATE_ADDR_L_REG (0x8BC)
#define IPF_CH1_BDQ1_RPTR_UPDATE_ADDR_H_REG (0x8C0)
#define IPF_CH0_ADQ_EN_CTRL_REG (0x8C4)
#define IPF_CH0_ADQ_SIZE_CTRL_REG (0x8C8)
#define IPF_CH0_ADQ0_BASE_L_REG (0x8CC)
#define IPF_CH0_ADQ0_BASE_H_REG (0x8D0)
#define IPF_CH0_ADQ0_STAT_REG (0x8D4)
#define IPF_CH0_ADQ0_WPTR_REG (0x8D8)
#define IPF_CH0_ADQ0_RPTR_REG (0x8DC)
#define IPF_CH0_ADQ0_RPTR_UPDATE_ADDR_L_REG (0x8E0)
#define IPF_CH0_ADQ0_RPTR_UPDATE_ADDR_H_REG (0x8E4)
#define IPF_CH0_ADQ1_BASE_L_REG (0x8E8)
#define IPF_CH0_ADQ1_BASE_H_REG (0x8EC)
#define IPF_CH0_ADQ1_STAT_REG (0x8F0)
#define IPF_CH0_ADQ1_WPTR_REG (0x8F4)
#define IPF_CH0_ADQ1_RPTR_REG (0x8F8)
#define IPF_CH0_ADQ1_RPTR_UPDATE_ADDR_L_REG (0x8FC)
#define IPF_CH0_ADQ1_RPTR_UPDATE_ADDR_H_REG (0x900)
#define FLTN_LOCAL_ADDR0_0_REG (0x1000)
#define IPF_INT_SUP_TIME(n) (IPF_INT0_SUP_TIME_REG + n * 0x04)
#define IPF_CH0_BDQ_BADDR_L_REG(n) (IPF_CH0_BDQ0_BADDR_L_REG + n * 0x30)
#define IPF_CH0_BDQ_BADDR_H_REG(n) (IPF_CH0_BDQ0_BADDR_H_REG + n * 0x30)
#define IPF_CH0_BDQ_SIZE_REG(n) (IPF_CH0_BDQ0_SIZE_REG + n * 0x30)
#define IPF_CH0_BDQ_RPTR_REG(n) (IPF_CH0_BDQ0_RPTR_REG + n * 0x30)
#define IPF_CH0_BDQ_WPTR_REG(n) (IPF_CH0_BDQ0_WPTR_REG + n * 0x30)
#define IPF_CH0_BDQ_WADDR_L_REG(n) (IPF_CH0_BDQ0_WADDR_L_REG + n * 0x30)
#define IPF_CH0_BDQ_WADDR_H_REG(n) (IPF_CH0_BDQ0_WADDR_H_REG + n * 0x30)
#define IPF_CH0_BDQ_RADDR_L_REG(n) (IPF_CH0_BDQ0_RADDR_L_REG + n * 0x30)
#define IPF_CH0_BDQ_RADDR_H_REG(n) (IPF_CH0_BDQ0_RADDR_H_REG + n * 0x30)
#define IPF_CH0_BDQ_DEPTH_REG(n) (IPF_CH0_BDQ0_DEPTH_REG + n * 0x30)
#define IPF_CH0_BDQ_RPTR_UPDATE_ADDR_L_REG(n) (IPF_CH0_BDQ0_RPTR_UPDATE_ADDR_L_REG + n * 0x30)
#define IPF_CH0_BDQ_RPTR_UPDATE_ADDR_H_REG(n) (IPF_CH0_BDQ0_RPTR_UPDATE_ADDR_H_REG + n * 0x30)
#define IPF_CH0_RDQ_BADDR_L_REG(n) (IPF_CH0_RDQ0_BADDR_L_REG + n * 0x30)
#define IPF_CH0_RDQ_BADDR_H_REG(n) (IPF_CH0_RDQ0_BADDR_H_REG + n * 0x30)
#define IPF_CH0_RDQ_SIZE_REG(n) (IPF_CH0_RDQ0_SIZE_REG + n * 0x30)
#define IPF_CH0_RDQ_WPTR_REG(n) (IPF_CH0_RDQ0_WPTR_REG + n * 0x30)
#define IPF_CH0_RDQ_RPTR_REG(n) (IPF_CH0_RDQ0_RPTR_REG + n * 0x30)
#define IPF_CH0_RDQ_WADDR_L_REG(n) (IPF_CH0_RDQ0_WADDR_L_REG + n * 0x30)
#define IPF_CH0_RDQ_WADDR_H_REG(n) (IPF_CH0_RDQ0_WADDR_H_REG + n * 0x30)
#define IPF_CH0_RDQ_RADDR_L_REG(n) (IPF_CH0_RDQ0_RADDR_L_REG + n * 0x30)
#define IPF_CH0_RDQ_RADDR_H_REG(n) (IPF_CH0_RDQ0_RADDR_H_REG + n * 0x30)
#define IPF_CH0_RDQ_DEPTH_REG(n) (IPF_CH0_RDQ0_DEPTH_REG + n * 0x30)
#define IPF_CH0_RDQ_WPTR_UPDATE_ADDR_L_REG(n) (IPF_CH0_RDQ0_WPTR_UPDATE_ADDR_L_REG + n * 0x30)
#define IPF_CH0_RDQ_WPTR_UPDATE_ADDR_H_REG(n) (IPF_CH0_RDQ0_WPTR_UPDATE_ADDR_H_REG + n * 0x30)
#define IPF_CH1_BDQ_BADDR_L_REG(n) (IPF_CH1_BDQ0_BADDR_L_REG + n * 0x30)
#define IPF_CH1_BDQ_BADDR_H_REG(n) (IPF_CH1_BDQ0_BADDR_H_REG + n * 0x30)
#define IPF_CH1_BDQ_SIZE_REG(n) (IPF_CH1_BDQ0_SIZE_REG + n * 0x30)
#define IPF_CH1_BDQ_WPTR_REG(n) (IPF_CH1_BDQ0_WPTR_REG + n * 0x30)
#define IPF_CH1_BDQ_RPTR_REG(n) (IPF_CH1_BDQ0_RPTR_REG + n * 0x30)
#define IPF_CH1_BDQ_WADDR_L_REG(n) (IPF_CH1_BDQ0_WADDR_L_REG + n * 0x30)
#define IPF_CH1_BDQ_WADDR_H_REG(n) (IPF_CH1_BDQ0_WADDR_H_REG + n * 0x30)
#define IPF_CH1_BDQ_RADDR_L_REG(n) (IPF_CH1_BDQ0_RADDR_L_REG + n * 0x30)
#define IPF_CH1_BDQ_RADDR_H_REG(n) (IPF_CH1_BDQ0_RADDR_H_REG + n * 0x30)
#define IPF_CH1_BDQ_DEPTH_REG(n) (IPF_CH1_BDQ0_DEPTH_REG + n * 0x30)
#define IPF_CH1_BDQ_RPTR_UPDATE_ADDR_L_REG(n) (IPF_CH1_BDQ0_RPTR_UPDATE_ADDR_L_REG + n * 0x30)
#define IPF_CH1_BDQ_RPTR_UPDATE_ADDR_H_REG(n) (IPF_CH1_BDQ0_RPTR_UPDATE_ADDR_H_REG + n * 0x30)
#define FLTN_LOCAL_ADDR_REG(n) (FLTN_LOCAL_ADDR0_0_REG + n * 0x80)
#define IPF_CH1_BDQ_DEPTH_REG(n) (IPF_CH1_BDQ0_DEPTH_REG + n * 0x30)
#define IPF_CH0_RDQ_DEPTH_REG(n) (IPF_CH0_RDQ0_DEPTH_REG + n * 0x30)
typedef union {
    struct {
        unsigned int ul_en : 1;
        unsigned int dl_en : 1;
        unsigned int reserved_0 : 30;
    } bits;
    unsigned int u32;
} U_IPF_CH_EN;
typedef union {
    struct {
        unsigned int dma_clk_sel : 1;
        unsigned int core_clk_sel : 1;
        unsigned int cfg_clk_sel : 1;
        unsigned int ram_clk_sel : 1;
        unsigned int reserved_0 : 28;
    } bits;
    unsigned int u32;
} U_IPF_GATE;
typedef union {
    struct {
        unsigned int filter_seq : 1;
        unsigned int flt_addr_reverse : 1;
        unsigned int sp_cfg : 1;
        unsigned int sp_wrr_sel : 1;
        unsigned int max_burst_len : 2;
        unsigned int ipv6_nh_sel : 1;
        unsigned int ah_esp_sel : 1;
        unsigned int ah_disable : 1;
        unsigned int esp_disable : 1;
        unsigned int mfc_en : 1;
        unsigned int mm_en : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int cd_en_sel : 1;
        unsigned int qos_flow_id_width : 1;
        unsigned int reserved_2 : 1;
        unsigned int dl_refl_en : 1;
        unsigned int dl_relf_as_en : 1;
        unsigned int dl_dbl_bdq_en : 1;
        unsigned int dl_rdq_with_head : 1;
        unsigned int ul_dbl_rdq_en : 1;
        unsigned int ul_dbl_bdq_en : 1;
        unsigned int ul_bdq_with_head : 1;
        unsigned int ul_frag_tbl_en : 1;
        unsigned int reserved_3 : 7;
    } bits;
    unsigned int u32;
} U_IPF_CTRL;
typedef union {
    struct {
        unsigned int int0_supress_en : 1;
        unsigned int int1_supress_en : 1;
        unsigned int int2_supress_en : 1;
        unsigned int reserved_0 : 29;
    } bits;
    unsigned int u32;
} U_IPF_INT_CTRL;
typedef union {
    struct {
        unsigned int int2_supress_time : 10;
        unsigned int reserved_0 : 22;
    } bits;
    unsigned int u32;
} U_IPF_INT2_SUP_TIME;
typedef union {
    struct {
        unsigned int ul_rdq0_rpt_en0 : 1;
        unsigned int ul_rdq0_timeout_en0 : 1;
        unsigned int ul_rdq0_full_en0 : 1;
        unsigned int ul_rdq0_pkt_cnt_of_en0 : 1;
        unsigned int ul_rdq0_wptr_update_en0 : 1;
        unsigned int ul_rdq1_rpt_en0 : 1;
        unsigned int ul_rdq1_timeout_en0 : 1;
        unsigned int ul_rdq1_full_en0 : 1;
        unsigned int ul_rdq1_pkt_cnt_of_en0 : 1;
        unsigned int ul_rdq1_wptr_update_en0 : 1;
        unsigned int ul_bdq0_disable_end_en0 : 1;
        unsigned int ul_bdq0_epty_en0 : 1;
        unsigned int ul_bdq0_rptr_update_en0 : 1;
        unsigned int ul_bdq1_disable_end_en0 : 1;
        unsigned int ul_bdq1_epty_en0 : 1;
        unsigned int ul_bdq1_rptr_update_en0 : 1;
        unsigned int ul_adq0_amst_epty_en0 : 1;
        unsigned int ul_adq1_amst_epty_en0 : 1;
        unsigned int bus_error_w_en0 : 1;
        unsigned int bus_error_r_en0 : 1;
        unsigned int reserved_0 : 12;
    } bits;
    unsigned int u32;
} U_IPF_INT0_EN_UL;
typedef union {
    struct {
        unsigned int dl_rdq_rpt_en0 : 1;
        unsigned int dl_rdq_timeout_en0 : 1;
        unsigned int dl_rdq_full_en0 : 1;
        unsigned int dl_rdq_pkt_cnt_of_en0 : 1;
        unsigned int dl_rdq_wptr_update_en0 : 1;
        unsigned int dl_bdq0_disable_end_en0 : 1;
        unsigned int dl_bdq0_epty_en0 : 1;
        unsigned int dl_bdq0_rptr_update_en0 : 1;
        unsigned int dl_bdq1_disable_end_en0 : 1;
        unsigned int dl_bdq1_epty_en0 : 1;
        unsigned int dl_bdq1_rptr_update_en0 : 1;
        unsigned int dl_adq0_amst_epty_en0 : 1;
        unsigned int dl_adq1_amst_epty_en0 : 1;
        unsigned int dl_refl_nas_done_en0 : 1;
        unsigned int dl_rnrdq_wptr_update_en0 : 1;
        unsigned int dl_rnrdq_full_en0 : 1;
        unsigned int dl_rnrdq_timeout_en0 : 1;
        unsigned int dl_refl_as_done_en0 : 1;
        unsigned int dl_rardq_wptr_update_en0 : 1;
        unsigned int dl_rardq_full_en0 : 1;
        unsigned int dl_rardq_timeout_en0 : 1;
        unsigned int timer_cnt_eq0_en0 : 1;
        unsigned int ipf_gen0_en0 : 1;
        unsigned int ipf_gen1_en0 : 1;
        unsigned int ipf_gen2_en0 : 1;
        unsigned int reserved_0 : 7;
    } bits;
    unsigned int u32;
} U_IPF_INT0_EN_DL;
typedef union {
    struct {
        unsigned int ul_rdq0_rpt_en1 : 1;
        unsigned int ul_rdq0_timeout_en1 : 1;
        unsigned int ul_rdq0_full_en1 : 1;
        unsigned int ul_rdq0_pkt_cnt_of_en1 : 1;
        unsigned int ul_rdq0_wptr_update_en1 : 1;
        unsigned int ul_rdq1_rpt_en1 : 1;
        unsigned int ul_rdq1_timeout_en1 : 1;
        unsigned int ul_rdq1_full_en1 : 1;
        unsigned int ul_rdq1_pkt_cnt_of_en1 : 1;
        unsigned int ul_rdq1_wptr_update_en1 : 1;
        unsigned int ul_bdq0_disable_end_en1 : 1;
        unsigned int ul_bdq0_epty_en1 : 1;
        unsigned int ul_bdq0_rptr_update_en1 : 1;
        unsigned int ul_bdq1_disable_end_en1 : 1;
        unsigned int ul_bdq1_epty_en1 : 1;
        unsigned int ul_bdq1_rptr_update_en1 : 1;
        unsigned int ul_adq0_amst_epty_en1 : 1;
        unsigned int ul_adq1_amst_epty_en1 : 1;
        unsigned int bus_error_w_en1 : 1;
        unsigned int bus_error_r_en1 : 1;
        unsigned int reserved_0 : 12;
    } bits;
    unsigned int u32;
} U_IPF_INT1_EN_UL;
typedef union {
    struct {
        unsigned int dl_rdq_rpt_en1 : 1;
        unsigned int dl_rdq_timeout_en1 : 1;
        unsigned int dl_rdq_full_en1 : 1;
        unsigned int dl_rdq_pkt_cnt_of_en1 : 1;
        unsigned int dl_rdq_wptr_update_en1 : 1;
        unsigned int dl_bdq0_disable_end_en1 : 1;
        unsigned int dl_bdq0_epty_en1 : 1;
        unsigned int dl_bdq0_rptr_update_en1 : 1;
        unsigned int dl_bdq1_disable_end_en1 : 1;
        unsigned int dl_bdq1_epty_en1 : 1;
        unsigned int dl_bdq1_rptr_update_en1 : 1;
        unsigned int dl_adq0_amst_epty_en1 : 1;
        unsigned int dl_adq1_amst_epty_en1 : 1;
        unsigned int dl_refl_nas_done_en1 : 1;
        unsigned int dl_rnrdq_wptr_update_en1 : 1;
        unsigned int dl_rnrdq_full_en1 : 1;
        unsigned int dl_rnrdq_timeout_en1 : 1;
        unsigned int dl_refl_as_done_en1 : 1;
        unsigned int dl_rardq_wptr_update_en1 : 1;
        unsigned int dl_rardq_full_en1 : 1;
        unsigned int dl_rardq_timeout_en1 : 1;
        unsigned int timer_cnt_eq0_en1 : 1;
        unsigned int ipf_gen0_en1 : 1;
        unsigned int ipf_gen1_en1 : 1;
        unsigned int ipf_gen2_en1 : 1;
        unsigned int reserved_0 : 7;
    } bits;
    unsigned int u32;
} U_IPF_INT1_EN_DL;
typedef union {
    struct {
        unsigned int ul_rdq0_rpt_en2 : 1;
        unsigned int ul_rdq0_timeout_en2 : 1;
        unsigned int ul_rdq0_full_en2 : 1;
        unsigned int ul_rdq0_pkt_cnt_of_en2 : 1;
        unsigned int ul_rdq0_wptr_update_en2 : 1;
        unsigned int ul_rdq1_rpt_en2 : 1;
        unsigned int ul_rdq1_timeout_en2 : 1;
        unsigned int ul_rdq1_full_en2 : 1;
        unsigned int ul_rdq1_pkt_cnt_of_en2 : 1;
        unsigned int ul_rdq1_wptr_update_en2 : 1;
        unsigned int ul_bdq0_disable_end_en2 : 1;
        unsigned int ul_bdq0_epty_en2 : 1;
        unsigned int ul_bdq0_rptr_update_en2 : 1;
        unsigned int ul_bdq1_disable_end_en2 : 1;
        unsigned int ul_bdq1_epty_en2 : 1;
        unsigned int ul_bdq1_rptr_update_en2 : 1;
        unsigned int ul_adq0_amst_epty_en2 : 1;
        unsigned int ul_adq1_amst_epty_en2 : 1;
        unsigned int bus_error_w_en2 : 1;
        unsigned int bus_error_r_en2 : 1;
        unsigned int reserved_0 : 12;
    } bits;
    unsigned int u32;
} U_IPF_INT2_EN_UL;
typedef union {
    struct {
        unsigned int dl_rdq_rpt_en2 : 1;
        unsigned int dl_rdq_timeout_en2 : 1;
        unsigned int dl_rdq_full_en2 : 1;
        unsigned int dl_rdq_pkt_cnt_of_en2 : 1;
        unsigned int dl_rdq_wptr_update_en2 : 1;
        unsigned int dl_bdq0_disable_end_en2 : 1;
        unsigned int dl_bdq0_epty_en2 : 1;
        unsigned int dl_bdq0_rptr_update_en2 : 1;
        unsigned int dl_bdq1_disable_end_en2 : 1;
        unsigned int dl_bdq1_epty_en2 : 1;
        unsigned int dl_bdq1_rptr_update_en2 : 1;
        unsigned int dl_adq0_amst_epty_en2 : 1;
        unsigned int dl_adq1_amst_epty_en2 : 1;
        unsigned int dl_refl_nas_done_en2 : 1;
        unsigned int dl_rnrdq_wptr_update_en2 : 1;
        unsigned int dl_rnrdq_full_en2 : 1;
        unsigned int dl_rnrdq_timeout_en2 : 1;
        unsigned int dl_refl_as_done_en2 : 1;
        unsigned int dl_rardq_wptr_update_en2 : 1;
        unsigned int dl_rardq_full_en2 : 1;
        unsigned int dl_rardq_timeout_en2 : 1;
        unsigned int timer_cnt_eq0_en2 : 1;
        unsigned int ipf_gen0_en2 : 1;
        unsigned int ipf_gen1_en2 : 1;
        unsigned int ipf_gen2_en2 : 1;
        unsigned int reserved_0 : 7;
    } bits;
    unsigned int u32;
} U_IPF_INT2_EN_DL;
typedef union {
    struct {
        unsigned int ul_rdq0_rpt : 1;
        unsigned int ul_rdq0_timeout : 1;
        unsigned int ul_rdq0_full : 1;
        unsigned int ul_rdq0_pkt_cnt_of : 1;
        unsigned int ul_rdq0_wptr_update : 1;
        unsigned int ul_rdq1_rpt : 1;
        unsigned int ul_rdq1_timeout : 1;
        unsigned int ul_rdq1_full : 1;
        unsigned int ul_rdq1_pkt_cnt_of : 1;
        unsigned int ul_rdq1_wptr_update : 1;
        unsigned int ul_bdq0_disable_end : 1;
        unsigned int ul_bdq0_epty : 1;
        unsigned int ul_bdq0_rptr_update : 1;
        unsigned int ul_bdq1_disable_end : 1;
        unsigned int ul_bdq1_epty : 1;
        unsigned int ul_bdq1_rptr_update : 1;
        unsigned int ul_adq0_amst_epty : 1;
        unsigned int ul_adq1_amst_epty : 1;
        unsigned int bus_error_w : 1;
        unsigned int bus_error_r : 1;
        unsigned int reserved_0 : 12;
    } bits;
    unsigned int u32;
} U_IPF_INT_STATE_UL;
typedef union {
    struct {
        unsigned int dl_rdq_rpt : 1;
        unsigned int dl_rdq_timeout : 1;
        unsigned int dl_rdq_full : 1;
        unsigned int dl_rdq_pkt_cnt_of : 1;
        unsigned int dl_rdq_wptr_update : 1;
        unsigned int dl_bdq0_disable_end : 1;
        unsigned int dl_bdq0_epty : 1;
        unsigned int dl_bdq0_rptr_update : 1;
        unsigned int dl_bdq1_disable_end : 1;
        unsigned int dl_bdq1_epty : 1;
        unsigned int dl_bdq1_rptr_update : 1;
        unsigned int dl_adq0_amst_epty : 1;
        unsigned int dl_adq1_amst_epty : 1;
        unsigned int dl_refl_nas_done : 1;
        unsigned int dl_rnrdq_wptr_update : 1;
        unsigned int dl_rnrdq_full : 1;
        unsigned int dl_rnrdq_timeout : 1;
        unsigned int dl_refl_as_done : 1;
        unsigned int dl_rardq_wptr_update : 1;
        unsigned int dl_rardq_full : 1;
        unsigned int dl_rardq_timeout : 1;
        unsigned int timer_cnt_eq0 : 1;
        unsigned int ipf_gen0 : 1;
        unsigned int ipf_gen1 : 1;
        unsigned int ipf_gen2 : 1;
        unsigned int reserved_0 : 7;
    } bits;
    unsigned int u32;
} U_IPF_INT_STATE_DL;
typedef union {
    struct {
        unsigned int ul_mod : 2;
        unsigned int ul_endian : 1;
        unsigned int ul_data_chain : 1;
        unsigned int ul_bdq0_clr : 1;
        unsigned int ul_bdq1_clr : 1;
        unsigned int ul_rdq0_clr : 1;
        unsigned int ul_rdq1_clr : 1;
        unsigned int ul_bdq_pri : 1;
        unsigned int reserved_0 : 7;
        unsigned int ul_wrr_value : 16;
    } bits;
    unsigned int u32;
} U_IPF_CH0_CTRL;
typedef union {
    struct {
        unsigned int ul_bdq0_busy : 1;
        unsigned int ul_bdq1_busy : 1;
        unsigned int reserved_0 : 2;
        unsigned int ul_rdq0_full : 1;
        unsigned int ul_rdq0_empty : 1;
        unsigned int ul_bdq0_full : 1;
        unsigned int ul_bdq0_empty : 1;
        unsigned int ul_rdq1_full : 1;
        unsigned int ul_rdq1_empty : 1;
        unsigned int ul_bdq1_full : 1;
        unsigned int ul_bdq1_empty : 1;
        unsigned int ul_rdq0_rptr_invalid : 1;
        unsigned int ul_rdq1_rptr_invalid : 1;
        unsigned int ul_bdq0_wptr_invalid : 1;
        unsigned int ul_bdq1_wptr_invalid : 1;
        unsigned int reserved_1 : 16;
    } bits;
    unsigned int u32;
} U_IPF_CH0_STATE;
typedef union {
    struct {
        unsigned int ul_bdq0_baddr_h : 8;
        unsigned int reserved_0 : 24;
    } bits;
    unsigned int u32;
} U_IPF_CH0_BDQ0_BADDR_H;
typedef union {
    struct {
        unsigned int ul_bdq0_size : 14;
        unsigned int reserved_0 : 18;
    } bits;
    unsigned int u32;
} U_IPF_CH0_BDQ0_SIZE;
typedef union {
    struct {
        unsigned int ul_bdq0_depth : 15;
        unsigned int reserved_0 : 17;
    } bits;
    unsigned int u32;
} U_IPF_CH0_BDQ0_DEPTH;
typedef union {
    struct {
        unsigned int ul_bdq1_depth : 15;
        unsigned int reserved_0 : 17;
    } bits;
    unsigned int u32;
} U_IPF_CH0_BDQ1_DEPTH;
typedef union {
    struct {
        unsigned int ul_bdq1_rptr_update_addr_h : 8;
        unsigned int reserved_0 : 24;
    } bits;
    unsigned int u32;
} U_IPF_CH0_BDQ1_RPTR_UPDATE_ADDR_H;
typedef union {
    struct {
        unsigned int dl_mod : 2;
        unsigned int dl_endian : 1;
        unsigned int dl_data_chain : 1;
        unsigned int dl_bdq0_clr : 1;
        unsigned int dl_bdq1_clr : 1;
        unsigned int dl_rdq_clr : 1;
        unsigned int dl_bdq_pri : 1;
        unsigned int dl_rnrdq_clr : 1;
        unsigned int dl_rardq_clr : 1;
        unsigned int reserved_0 : 6;
        unsigned int dl_wrr_value : 16;
    } bits;
    unsigned int u32;
} U_IPF_CH1_CTRL;
typedef union {
    struct {
        unsigned int dl_bdq0_busy : 1;
        unsigned int dl_bdq1_busy : 1;
        unsigned int dl_rdq_full : 1;
        unsigned int dl_rdq_empty : 1;
        unsigned int dl_bdq0_full : 1;
        unsigned int dl_bdq1_full : 1;
        unsigned int dl_bdq0_empty : 1;
        unsigned int dl_bdq1_empty : 1;
        unsigned int dl_rdq_rptr_invalid : 1;
        unsigned int dl_bdq0_wptr_invalid : 1;
        unsigned int dl_bdq1_wptr_invalid : 1;
        unsigned int dl_rnrdq_empty : 1;
        unsigned int dl_rnrdq_full : 1;
        unsigned int dl_rardq_empty : 1;
        unsigned int dl_rardq_full : 1;
        unsigned int dl_rnrdq_rptr_invalid : 1;
        unsigned int dl_rardq_rptr_invalid : 1;
        unsigned int reserved_0 : 15;
    } bits;
    unsigned int u32;
} U_IPF_CH1_STATE;
typedef union {
    struct {
        unsigned int dl_adq_en : 2;
        unsigned int dl_ad_buf_flush : 1;
        unsigned int reserved_0 : 29;
    } bits;
    unsigned int u32;
} U_IPF_CH1_ADQ_EN_CTRL;
typedef union {
    struct {
        unsigned int dl_adq0_size_sel : 3;
        unsigned int dl_adq1_size_sel : 3;
        unsigned int dl_adq_empty_th : 10;
        unsigned int dl_adq_plen_th : 16;
    } bits;
    unsigned int u32;
} U_IPF_CH1_ADQ_SIZE_CTRL;
typedef union {
    struct {
        unsigned int dl_rardq_size : 10;
        unsigned int reserved_0 : 22;
    } bits;
    unsigned int u32;
} U_IPF_CH1_RARDQ_SIZE;
typedef union {
    struct {
        unsigned int dl_rardq_wptr : 10;
        unsigned int reserved_0 : 22;
    } bits;
    unsigned int u32;
} U_IPF_CH1_RARDQ_WPTR;
typedef union {
    struct {
        unsigned int dl_rardq_rptr : 10;
        unsigned int reserved_0 : 22;
    } bits;
    unsigned int u32;
} U_IPF_CH1_RARDQ_RPTR;
typedef union {
    struct {
        unsigned int dl_rardq_waddr_h : 8;
        unsigned int reserved_0 : 24;
    } bits;
    unsigned int u32;
} U_IPF_CH1_RARDQ_WADDR_H;
typedef union {
    struct {
        unsigned int dl_rardq_raddr_h : 8;
        unsigned int reserved_0 : 24;
    } bits;
    unsigned int u32;
} U_IPF_CH1_RARDQ_RADDR_H;
typedef union {
    struct {
        unsigned int dl_rardq_depth : 11;
        unsigned int reserved_0 : 21;
    } bits;
    unsigned int u32;
} U_IPF_CH1_RARDQ_DEPTH;
typedef union {
    struct {
        unsigned int dl_rardq_wptr_update_addr_h : 8;
        unsigned int reserved_0 : 24;
    } bits;
    unsigned int u32;
} U_IPF_CH1_RARDQ_WPTR_UPDATE_ADDR_H;
typedef union {
    struct {
        unsigned int reserved_0 : 5;
        unsigned int frag_ipv4_wptr_clr : 1;
        unsigned int reserved_1 : 2;
        unsigned int reserved_2 : 5;
        unsigned int frag_ipv6_wptr_clr : 1;
        unsigned int reserved_3 : 18;
    } bits;
    unsigned int u32;
} U_FRAG_WPTR;
typedef union {
    struct {
        unsigned int ul_wdata_sec_n : 1;
        unsigned int ul_rdata_sec_n : 1;
        unsigned int ul_rd_sec_n : 1;
        unsigned int ul_bd_sec_n : 1;
        unsigned int ul_ad_sec_n : 1;
        unsigned int ul_cd_sec_n : 1;
        unsigned int ul_bdq_rptr_sec_n : 1;
        unsigned int ul_adq_rptr_sec_n : 1;
        unsigned int ul_rdq_wptr_sec_n : 1;
        unsigned int reserved_0 : 7;
        unsigned int dl_wdata_sec_n : 1;
        unsigned int reserved_1 : 1;
        unsigned int dl_rd_sec_n : 1;
        unsigned int dl_bd_sec_n : 1;
        unsigned int dl_ad_sec_n : 1;
        unsigned int dl_cd_sec_n : 1;
        unsigned int dl_bdq_rptr_sec_n : 1;
        unsigned int dl_adq_rptr_sec_n : 1;
        unsigned int dl_rdq_wptr_sec_n : 1;
        unsigned int dl_rnrd_sec_n : 1;
        unsigned int dl_rard_sec_n : 1;
        unsigned int dl_rnrd_wptr_sec_n : 1;
        unsigned int dl_rard_wptr_sec_n : 1;
        unsigned int ef_sec_n : 1;
        unsigned int reserved_2 : 2;
    } bits;
    unsigned int u32;
} U_IPF_SEC_ATTR;
typedef union {
    struct {
        unsigned int dl_bdq0_depth : 15;
        unsigned int reserved_0 : 17;
    } bits;
    unsigned int u32;
} U_IPF_CH1_BDQ0_DEPTH;
typedef union {
    struct {
        unsigned int dl_bdq1_depth : 15;
        unsigned int reserved_0 : 17;
    } bits;
    unsigned int u32;
} U_IPF_CH1_BDQ1_DEPTH;
typedef union {
    struct {
        unsigned int ul_adq1_wptr : 12;
        unsigned int reserved_0 : 20;
    } bits;
    unsigned int u32;
} U_IPF_CH0_ADQ1_WPTR;
typedef union {
    struct {
        unsigned int ul_adq1_rptr : 12;
        unsigned int reserved_0 : 20;
    } bits;
    unsigned int u32;
} U_IPF_CH0_ADQ1_RPTR;
typedef union {
    struct {
        unsigned int time_out_cfg : 16;
        unsigned int time_out_valid : 1;
        unsigned int reserved_0 : 15;
    } bits;
    unsigned int u32;
} U_IPF_TIME_OUT;
typedef union {
    struct {
        unsigned int min_pkt_len : 14;
        unsigned int reserved_0 : 2;
        unsigned int max_pkt_len : 14;
        unsigned int reserved_1 : 2;
    } bits;
    unsigned int u32;
} U_IPF_PKT_LEN;
typedef union {
    struct {
        unsigned int ul_pkt_cnt_en : 1;
        unsigned int ul_pkt_cnt_clear : 1;
        unsigned int ul_pkt_cnt_prescaler_sel : 2;
        unsigned int reserved_0 : 4;
        unsigned int dl_pkt_cnt_en : 1;
        unsigned int dl_pkt_cnt_clear : 1;
        unsigned int dl_pkt_cnt_prescaler_sel : 2;
        unsigned int reserved_1 : 4;
        unsigned int timer_en : 1;
        unsigned int timer_auto_reload : 1;
        unsigned int timer_clear : 1;
        unsigned int reserved_2 : 13;
    } bits;
    unsigned int u32;
} U_IPF_TRANS_CNT_CTRL;
typedef union {
    struct {
        unsigned int timer_load : 10;
        unsigned int reserved_0 : 22;
    } bits;
    unsigned int u32;
} U_IPF_TIMER_LOAD;
typedef union {
    struct {
        unsigned int dl_rdq_depth : 15;
        unsigned int reserved_0 : 17;
    } bits;
    unsigned int u32;
} U_IPF_CH1_RDQ_DEPTH;
typedef union {
    struct {
        unsigned int ul_rdq0_depth : 15;
        unsigned int reserved_0 : 17;
    } bits;
    unsigned int u32;
} U_IPF_CH0_RDQ0_DEPTH;
typedef union {
    struct {
        unsigned int ul_adq_en : 2;
        unsigned int ul_ad_buf_flush : 1;
        unsigned int reserved_0 : 29;
    } bits;
    unsigned int u32;
} U_IPF_CH0_ADQ_EN_CTRL;
typedef union {
    struct {
        unsigned int ul_adq0_size_sel : 3;
        unsigned int ul_adq1_size_sel : 3;
        unsigned int ul_adq_empty_th : 10;
        unsigned int ul_adq_plen_th : 16;
    } bits;
    unsigned int u32;
} U_IPF_CH0_ADQ_SIZE_CTRL;
typedef union {
    struct {
        unsigned int ul_adq0_empty : 1;
        unsigned int reserved_0 : 1;
        unsigned int ul_adq0_buf_epty : 1;
        unsigned int ul_adq0_buf_full : 1;
        unsigned int ul_adq0_rptr_invalid : 1;
        unsigned int ul_adq0_wptr_invalid : 1;
        unsigned int reserved_1 : 2;
        unsigned int ul_adq0_depth : 13;
        unsigned int reserved_2 : 11;
    } bits;
    unsigned int u32;
} U_IPF_CH0_ADQ0_STAT;
typedef union {
    struct {
        unsigned int ul_adq1_empty : 1;
        unsigned int reserved_0 : 1;
        unsigned int ul_adq1_buf_epty : 1;
        unsigned int ul_adq1_buf_full : 1;
        unsigned int ul_adq1_rptr_invalid : 1;
        unsigned int ul_adq1_wptr_invalid : 1;
        unsigned int reserved_1 : 2;
        unsigned int ul_adq1_depth : 13;
        unsigned int reserved_2 : 11;
    } bits;
    unsigned int u32;
} U_IPF_CH0_ADQ1_STAT;
struct ipf_reg_s {
    volatile U_IPF_CH_EN IPF_CH_EN;
    volatile U_IPF_GATE IPF_GATE;
    volatile U_IPF_CTRL IPF_CTRL;
    volatile U_IPF_INT_CTRL IPF_INT_CTRL;
    volatile U_IPF_INT2_SUP_TIME IPF_INT2_SUP_TIME;
    volatile U_IPF_INT0_EN_UL IPF_INT0_EN_UL;
    volatile U_IPF_INT0_EN_DL IPF_INT0_EN_DL;
    volatile U_IPF_INT1_EN_UL IPF_INT1_EN_UL;
    volatile U_IPF_INT1_EN_DL IPF_INT1_EN_DL;
    volatile U_IPF_INT2_EN_UL IPF_INT2_EN_UL;
    volatile U_IPF_INT2_EN_DL IPF_INT2_EN_DL;
    volatile U_IPF_INT_STATE_UL IPF_INT_STATE_UL;
    volatile U_IPF_INT_STATE_DL IPF_INT_STATE_DL;
    volatile U_IPF_CH0_CTRL IPF_CH0_CTRL;
    volatile U_IPF_CH0_STATE IPF_CH0_STATE;
    volatile unsigned int IPF_CH0_BDQ0_BADDR_L;
    volatile U_IPF_CH0_BDQ0_BADDR_H IPF_CH0_BDQ0_BADDR_H;
    volatile U_IPF_CH0_BDQ0_SIZE IPF_CH0_BDQ0_SIZE;
    volatile U_IPF_CH0_BDQ0_DEPTH IPF_CH0_BDQ0_DEPTH;
    volatile U_IPF_CH0_BDQ1_DEPTH IPF_CH0_BDQ1_DEPTH;
    volatile unsigned int IPF_CH0_BDQ1_RPTR_UPDATE_ADDR_L;
    volatile U_IPF_CH0_BDQ1_RPTR_UPDATE_ADDR_H IPF_CH0_BDQ1_RPTR_UPDATE_ADDR_H;
    volatile U_IPF_CH1_CTRL IPF_CH1_CTRL;
    volatile U_IPF_CH1_STATE IPF_CH1_STATE;
    volatile U_IPF_CH1_ADQ_EN_CTRL IPF_CH1_ADQ_EN_CTRL;
    volatile U_IPF_CH1_ADQ_SIZE_CTRL IPF_CH1_ADQ_SIZE_CTRL;
    volatile U_IPF_CH1_RARDQ_SIZE IPF_CH1_RARDQ_SIZE;
    volatile U_IPF_CH1_RARDQ_WPTR IPF_CH1_RARDQ_WPTR;
    volatile U_IPF_CH1_RARDQ_RPTR IPF_CH1_RARDQ_RPTR;
    volatile unsigned int IPF_CH1_RARDQ_WADDR_L;
    volatile U_IPF_CH1_RARDQ_WADDR_H IPF_CH1_RARDQ_WADDR_H;
    volatile unsigned int IPF_CH1_RARDQ_RADDR_L;
    volatile U_IPF_CH1_RARDQ_RADDR_H IPF_CH1_RARDQ_RADDR_H;
    volatile U_IPF_CH1_RARDQ_DEPTH IPF_CH1_RARDQ_DEPTH;
    volatile unsigned int IPF_CH1_RARDQ_WPTR_UPDATE_ADDR_L;
    volatile U_IPF_CH1_RARDQ_WPTR_UPDATE_ADDR_H IPF_CH1_RARDQ_WPTR_UPDATE_ADDR_H;
    volatile U_FRAG_WPTR FRAG_WPTR;
    volatile U_IPF_SEC_ATTR IPF_SEC_ATTR;
    volatile U_IPF_CH1_BDQ0_DEPTH IPF_CH1_BDQ0_DEPTH;
    volatile U_IPF_CH1_BDQ1_DEPTH IPF_CH1_BDQ1_DEPTH;
    volatile U_IPF_CH0_ADQ1_WPTR IPF_CH0_ADQ1_WPTR;
    volatile U_IPF_CH0_ADQ1_RPTR IPF_CH0_ADQ1_RPTR;
};
#endif
