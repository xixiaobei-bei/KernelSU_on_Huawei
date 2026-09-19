#ifndef __HI_PSAM_H__
#define __HI_PSAM_H__ 
enum HI_PSAM_REG_OFFSET_E
{
    HI_PSAM_SRST_OFFSET = 0x0,
    HI_PSAM_CONFIG_OFFSET = 0x4,
    HI_PSAM_VERSION_OFFSET = 0x8,
    HI_PSAM_EN_OFFSET = 0xc,
    HI_PSAM_INT0_STAT_OFFSET = 0x40,
    HI_PSAM_INT0_MSTAT_OFFSET = 0x44,
    HI_PSAM_INT0_MASK_OFFSET = 0x48,
    HI_PSAM_INT1_STAT_OFFSET = 0x50,
    HI_PSAM_INT1_MSTAT_OFFSET = 0x54,
    HI_PSAM_INT1_MASK_OFFSET = 0x58,
    HI_PSAM_INT2_STAT_OFFSET = 0x60,
    HI_PSAM_INT2_MSTAT_OFFSET = 0x64,
    HI_PSAM_INT2_MASK_OFFSET = 0x68,
    HI_PSAM_CIPHER_SOFTRESET_OFFSET = 0x70,
    HI_PSAM_CBDQ_UPDATE_OFFSET = 0x74,
    HI_PSAM32_CIPHER_CH_SOFTRESET_0_OFFSET = 0x80,
    HI_PSAM32_CIPHER_CH_SOFTRESET_1_OFFSET = 0xa0,
    HI_PSAM32_CIPHER_CH_SOFTRESET_2_OFFSET = 0xc0,
 HI_PSAM64_CIPHER_CH_SOFTRESET_0_OFFSET = 0x80,
    HI_PSAM64_CIPHER_CH_SOFTRESET_1_OFFSET = 0xc0,
    HI_PSAM64_CIPHER_CH_SOFTRESET_2_OFFSET = 0x100,
    HI_PSAM_CIPHER_CH_EN_0_OFFSET = 0x84,
    HI_PSAM32_CIPHER_CH_EN_1_OFFSET = 0xa4,
    HI_PSAM32_CIPHER_CH_EN_2_OFFSET = 0xc4,
 HI_PSAM64_CIPHER_CH_EN_1_OFFSET = 0xc4,
    HI_PSAM64_CIPHER_CH_EN_2_OFFSET = 0x104,
    HI_PSAM32_CBDQ_CONFIG_0_OFFSET = 0x88,
    HI_PSAM32_CBDQ_CONFIG_1_OFFSET = 0xa8,
    HI_PSAM32_CBDQ_CONFIG_2_OFFSET = 0xc8,
 HI_PSAM64_CBDQ_CONFIG_0_OFFSET = 0x88,
    HI_PSAM64_CBDQ_CONFIG_1_OFFSET = 0xc8,
    HI_PSAM64_CBDQ_CONFIG_2_OFFSET = 0x108,
    HI_PSAM32_CBDQ_BADDR_0_OFFSET = 0x8c,
    HI_PSAM32_CBDQ_BADDR_1_OFFSET = 0xac,
    HI_PSAM32_CBDQ_BADDR_2_OFFSET = 0xcc,
 HI_PSAM64_CBDQ_BADDR_L_0_OFFSET = 0x90,
    HI_PSAM64_CBDQ_BADDR_L_1_OFFSET = 0xd0,
    HI_PSAM64_CBDQ_BADDR_L_2_OFFSET = 0x110,
    HI_PSAM64_CBDQ_BADDR_H_0_OFFSET = 0x94,
    HI_PSAM64_CBDQ_BADDR_H_1_OFFSET = 0xd4,
    HI_PSAM64_CBDQ_BADDR_H_2_OFFSET = 0x114,
    HI_PSAM32_CBDQ_SIZE_0_OFFSET = 0x90,
    HI_PSAM32_CBDQ_SIZE_1_OFFSET = 0xb0,
    HI_PSAM32_CBDQ_SIZE_2_OFFSET = 0xd0,
 HI_PSAM64_CBDQ_SIZE_0_OFFSET = 0x8c,
    HI_PSAM64_CBDQ_SIZE_1_OFFSET = 0xcc,
    HI_PSAM64_CBDQ_SIZE_2_OFFSET = 0x10c,
    HI_PSAM32_CBDQ_WPTR_0_OFFSET = 0x94,
    HI_PSAM32_CBDQ_WPTR_1_OFFSET = 0xb4,
    HI_PSAM32_CBDQ_WPTR_2_OFFSET = 0xd4,
 HI_PSAM64_CBDQ_WPTR_0_OFFSET = 0x98,
    HI_PSAM64_CBDQ_WPTR_1_OFFSET = 0xd8,
    HI_PSAM64_CBDQ_WPTR_2_OFFSET = 0x118,
    HI_PSAM32_CBDQ_STAT_0_OFFSET = 0x98,
    HI_PSAM32_CBDQ_STAT_1_OFFSET = 0xb8,
    HI_PSAM32_CBDQ_STAT_2_OFFSET = 0xd8,
 HI_PSAM64_CBDQ_STAT_0_OFFSET = 0x9c,
    HI_PSAM64_CBDQ_STAT_1_OFFSET = 0xdc,
    HI_PSAM64_CBDQ_STAT_2_OFFSET = 0x11c,
    HI_PSAM32_CBDQ_WPTR_ADDR_0_OFFSET = 0x9c,
    HI_PSAM32_CBDQ_WPTR_ADDR_1_OFFSET = 0xbc,
    HI_PSAM32_CBDQ_WPTR_ADDR_2_OFFSET = 0xdc,
 HI_PSAM64_CBDQ_WPTR_ADDR_L_0_OFFSET = 0xa0,
    HI_PSAM64_CBDQ_WPTR_ADDR_L_1_OFFSET = 0xe0,
    HI_PSAM64_CBDQ_WPTR_ADDR_L_2_OFFSET = 0x120,
    HI_PSAM64_CBDQ_WPTR_ADDR_H_0_OFFSET = 0xa4,
    HI_PSAM64_CBDQ_WPTR_ADDR_H_1_OFFSET = 0xe4,
    HI_PSAM64_CBDQ_WPTR_ADDR_H_2_OFFSET = 0x124,
    HI_PSAM32_CRDQ_CTRL_0_OFFSET = 0x100,
    HI_PSAM32_CRDQ_CTRL_1_OFFSET = 0x110,
    HI_PSAM32_CRDQ_CTRL_2_OFFSET = 0x120,
 HI_PSAM64_CRDQ_CTRL_0_OFFSET = 0x180,
    HI_PSAM64_CRDQ_CTRL_1_OFFSET = 0x1a0,
    HI_PSAM64_CRDQ_CTRL_2_OFFSET = 0x1c0,
    HI_PSAM32_CRDQ_STAT_0_OFFSET = 0x104,
    HI_PSAM32_CRDQ_STAT_1_OFFSET = 0x114,
    HI_PSAM32_CRDQ_STAT_2_OFFSET = 0x124,
 HI_PSAM64_CRDQ_STAT_0_OFFSET = 0x184,
    HI_PSAM64_CRDQ_STAT_1_OFFSET = 0x1a4,
    HI_PSAM64_CRDQ_STAT_2_OFFSET = 0x1c4,
    HI_PSAM32_CRDQ_PTR_0_OFFSET = 0x108,
    HI_PSAM32_CRDQ_PTR_1_OFFSET = 0x118,
    HI_PSAM32_CRDQ_PTR_2_OFFSET = 0x128,
 HI_PSAM64_CRDQ_PTR_0_OFFSET = 0x188,
    HI_PSAM64_CRDQ_PTR_1_OFFSET = 0x1a8,
    HI_PSAM64_CRDQ_PTR_2_OFFSET = 0x1c8,
    HI_PSAM32_CRDQ_RPTR_ADDR_0_OFFSET = 0x10c,
    HI_PSAM32_CRDQ_RPTR_ADDR_1_OFFSET = 0x11c,
    HI_PSAM32_CRDQ_RPTR_ADDR_2_OFFSET = 0x12c,
 HI_PSAM64_CRDQ_RPTR_ADDR_L_0_OFFSET = 0x190,
    HI_PSAM64_CRDQ_RPTR_ADDR_L_1_OFFSET = 0x1b0,
    HI_PSAM64_CRDQ_RPTR_ADDR_L_2_OFFSET = 0x1d0,
 HI_PSAM64_CRDQ_RPTR_ADDR_H_0_OFFSET = 0x194,
    HI_PSAM64_CRDQ_RPTR_ADDR_H_1_OFFSET = 0x1b4,
    HI_PSAM64_CRDQ_RPTR_ADDR_H_2_OFFSET = 0x1d4,
    HI_PSAM_IBDQ_STAT_OFFSET = 0x154,
 HI_PSAM64_IBDQ_STAT_OFFSET = 0x208,
    HI_PSAM32_IBDQ_BADDR_OFFSET = 0x158,
 HI_PSAM64_IBDQ_BADDR_L_OFFSET = 0x200,
    HI_PSAM64_IBDQ_BADDR_H_OFFSET = 0x204,
    HI_PSAM32_IBDQ_SIZE_OFFSET = 0x15c,
 HI_PSAM64_IBDQ_SIZE_OFFSET = 0x20c,
    HI_PSAM32_IBDQ_WPTR_OFFSET = 0x160,
 HI_PSAM64_IBDQ_WPTR_OFFSET = 0x210,
    HI_PSAM32_IBDQ_RPTR_OFFSET = 0x164,
 HI_PSAM64_IBDQ_RPTR_OFFSET = 0x214,
    HI_PSAM32_IBDQ_UPDATE_OFFSET = 0x170,
 HI_PSAM64_IBDQ_UPDATE_OFFSET = 0x220,
    HI_PSAM32_IBDQ_PKT_CNT_OFFSET = 0x174,
 HI_PSAM64_IBDQ_PKT_CNT_OFFSET = 0x224,
    HI_PSAM32_LBDQ_STAT_OFFSET = 0x254,
 HI_PSAM64_LBDQ_STAT_OFFSET = 0x258,
    HI_PSAM32_LBDQ_BADDR_OFFSET = 0x258,
 HI_PSAM64_LBDQ_BADDR_L_OFFSET = 0x250,
    HI_PSAM64_LBDQ_BADDR_H_OFFSET = 0x254,
    HI_PSAM_LBDQ_SIZE_OFFSET = 0x25c,
    HI_PSAM_LBDQ_WPTR_OFFSET = 0x260,
    HI_PSAM_LBDQ_RPTR_OFFSET = 0x264,
    HI_PSAM_LBDQ_DEPTH_OFFSET = 0x268,
    HI_PSAM_ADQ_CTRL_OFFSET = 0x284,
    HI_PSAM32_ADQ0_BASE_OFFSET = 0x290,
 HI_PSAM64_ADQ0_BASE_L_OFFSET = 0x2b0,
    HI_PSAM64_ADQ0_BASE_H_OFFSET = 0x2b4,
    HI_PSAM_ADQ0_STAT_OFFSET = 0x294,
    HI_PSAM_ADQ0_WPTR_OFFSET = 0x298,
    HI_PSAM_ADQ0_RPTR_OFFSET = 0x29c,
    HI_PSAM32_ADQ1_BASE_OFFSET = 0x2a0,
 HI_PSAM64_ADQ1_BASE_L_OFFSET = 0x2b8,
    HI_PSAM64_ADQ1_BASE_H_OFFSET = 0x2bc,
    HI_PSAM_ADQ1_STAT_OFFSET = 0x2a4,
    HI_PSAM_ADQ1_WPTR_OFFSET = 0x2a8,
    HI_PSAM_ADQ1_RPTR_OFFSET = 0x2ac,
    HI_PSAM32_ADQ_PADDR_ERR_OFFSET = 0x2b0,
    HI_PSAM32_ADQ_FAMA_ATTR_OFFSET = 0x2b4,
    HI_PSAM32_ADQ_PADDR_STR0_OFFSET = 0x300,
    HI_PSAM32_ADQ_PADDR_END0_OFFSET = 0x304,
    HI_PSAM32_ADQ_PADDR_STR1_OFFSET = 0x308,
    HI_PSAM32_ADQ_PADDR_END1_OFFSET = 0x30c,
    HI_PSAM32_ADQ_PADDR_STR2_OFFSET = 0x310,
    HI_PSAM32_ADQ_PADDR_END2_OFFSET = 0x314,
    HI_PSAM32_ADQ_PADDR_STR3_OFFSET = 0x318,
    HI_PSAM32_ADQ_PADDR_END3_OFFSET = 0x31c,
    HI_PSAM32_ADQ_PADDR_CTRL_OFFSET = 0x320,
    HI_PSAM32_CBDQ_FAMA_ATTR_OFFSET = 0x330,
    HI_PSAM32_IBDQ_FAMA_ATTR_OFFSET = 0x334,
    HI_PSAM32_LBDQ_FAMA_ATTR_OFFSET = 0x338,
    HI_PSAM32_ADQ_PADDR_FAMA0_OFFSET = 0x340,
    HI_PSAM32_ADQ_PADDR_FAMA1_OFFSET = 0x344,
    HI_PSAM32_ADQ_PADDR_FAMA2_OFFSET = 0x348,
    HI_PSAM32_ADQ_PADDR_FAMA3_OFFSET = 0x34c,
    HI_PSAM32_REG_END_OFFSET =0x350,
 HI_PSAM64_SEC_CTRL_OFFSET = 0x300,
    HI_PSAM_CRDQ0_BADDR_OFFSET = 0x400,
    HI_PSAM_CRDQ1_BADDR_OFFSET = 0x800,
    HI_PSAM_CRDQ2_BADDR_OFFSET = 0xc00
};
#define HI_PSAM32_CIPHER_CH_SOFTRESET_OFFSET(chn) HI_PSAM32_CIPHER_CH_SOFTRESET_0_OFFSET+0x20*chn
#define HI_PSAM32_CIPHER_CH_EN_OFFSET(chn) HI_PSAM_CIPHER_CH_EN_0_OFFSET+0x20*chn
#define HI_PSAM32_CBDQ_CONFIG_OFFSET(chn) HI_PSAM32_CBDQ_CONFIG_0_OFFSET+0x20*chn
#define HI_PSAM32_CBDQ_BADDR_OFFSET(chn) HI_PSAM32_CBDQ_BADDR_0_OFFSET+0x20*chn
#define HI_PSAM32_CBDQ_SIZE_OFFSET(chn) HI_PSAM32_CBDQ_SIZE_0_OFFSET+0x20*chn
#define HI_PSAM32_CBDQ_WPTR_OFFSET(chn) HI_PSAM32_CBDQ_WPTR_0_OFFSET+0x20*chn
#define HI_PSAM32_CBDQ_STAT_OFFSET(chn) HI_PSAM32_CBDQ_STAT_0_OFFSET+0x20*chn
#define HI_PSAM32_CBDQ_WPTR_ADDR_OFFSET(chn) HI_PSAM32_CBDQ_WPTR_ADDR_0_OFFSET+0x20*chn
#define HI_PSAM32_CRDQ_CTRL_OFFSET(chn) HI_PSAM32_CRDQ_CTRL_0_OFFSET+0x10*chn
#define HI_PSAM32_CRDQ_STAT_OFFSET(chn) HI_PSAM32_CRDQ_STAT_0_OFFSET+0x10*chn
#define HI_PSAM32_CRDQ_PTR_OFFSET(chn) HI_PSAM32_CRDQ_PTR_0_OFFSET+0x10*chn
#define HI_PSAM32_CRDQ_RPTR_ADDR_OFFSET(chn) HI_PSAM32_CRDQ_RPTR_ADDR_0_OFFSET+0x10*chn
#define HI_PSAM_CRDQ_BADDR_OFFSET(chn) HI_PSAM_CRDQ0_BADDR_OFFSET+0x400*chn
#define HI_PSAM64_CIPHER_CH_SOFTRESET_OFFSET(chn) HI_PSAM64_CIPHER_CH_SOFTRESET_0_OFFSET+0x40*chn
#define HI_PSAM64_CIPHER_CH_EN_OFFSET(chn) HI_PSAM_CIPHER_CH_EN_0_OFFSET+0x40*chn
#define HI_PSAM64_CBDQ_CONFIG_OFFSET(chn) HI_PSAM64_CBDQ_CONFIG_0_OFFSET+0x40*chn
#define HI_PSAM64_CBDQ_BADDR_L_OFFSET(chn) HI_PSAM64_CBDQ_BADDR_L_0_OFFSET+0x40*chn
#define HI_PSAM64_CBDQ_BADDR_H_OFFSET(chn) HI_PSAM64_CBDQ_BADDR_H_0_OFFSET+0x40*chn
#define HI_PSAM64_CBDQ_SIZE_OFFSET(chn) HI_PSAM64_CBDQ_SIZE_0_OFFSET+0x40*chn
#define HI_PSAM64_CBDQ_WPTR_OFFSET(chn) HI_PSAM64_CBDQ_WPTR_0_OFFSET+0x40*chn
#define HI_PSAM64_CBDQ_STAT_OFFSET(chn) HI_PSAM64_CBDQ_STAT_0_OFFSET+0x40*chn
#define HI_PSAM64_CBDQ_WPTR_ADDR_H_OFFSET(chn) HI_PSAM64_CBDQ_WPTR_ADDR_H_0_OFFSET+0x40*chn
#define HI_PSAM64_CBDQ_WPTR_ADDR_L_OFFSET(chn) HI_PSAM64_CBDQ_WPTR_ADDR_L_0_OFFSET+0x40*chn
#define HI_PSAM64_CRDQ_CTRL_OFFSET(chn) HI_PSAM64_CRDQ_CTRL_0_OFFSET+0x20*chn
#define HI_PSAM64_CRDQ_STAT_OFFSET(chn) HI_PSAM64_CRDQ_STAT_0_OFFSET+0x20*chn
#define HI_PSAM64_CRDQ_PTR_OFFSET(chn) HI_PSAM64_CRDQ_PTR_0_OFFSET+0x20*chn
#define HI_PSAM64_CRDQ_RPTR_ADDR_H_OFFSET(chn) HI_PSAM64_CRDQ_RPTR_ADDR_H_0_OFFSET+0x20*chn
#define HI_PSAM64_CRDQ_RPTR_ADDR_L_OFFSET(chn) HI_PSAM64_CRDQ_RPTR_ADDR_L_0_OFFSET+0x20*chn
#define HI_PSAM32_ADQ_BASE_OFFSET(chn) HI_PSAM32_ADQ0_BASE_OFFSET+0x10*chn
#define HI_PSAM64_ADQ_BASE_L_OFFSET(chn) HI_PSAM64_ADQ0_BASE_L_OFFSET+0x8*chn
#define HI_PSAM64_ADQ_BASE_H_OFFSET(chn) HI_PSAM64_ADQ0_BASE_H_OFFSET+0x8*chn
typedef union
{
    struct
    {
        unsigned int psam_srst : 1 ;
        unsigned int psam_idle : 1 ;
        unsigned int psam_cipher_ch_idle : 1 ;
        unsigned int reserved_0 : 29 ;
    } bits;
    unsigned int u32;
} HI_PSAM_SRST_T;
typedef union
{
    struct
    {
        unsigned int psam_rd_pri : 3 ;
        unsigned int psam_wr_pri : 3 ;
        unsigned int reserved_0 : 2 ;
        unsigned int ufield_len : 3 ;
        unsigned int psam_cfg_rsvd : 21 ;
    } bits;
    unsigned int u32;
} HI_PSAM_CONFIG_T;
typedef union
{
    struct
    {
        unsigned int psam_en : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} HI_PSAM_EN_T;
typedef union
{
    struct
    {
        unsigned int adq0_epty_int0 : 1 ;
        unsigned int adq1_epty_int0 : 1 ;
        unsigned int lbdq_epty_int0 : 1 ;
        unsigned int reserved_0 : 9 ;
        unsigned int rd0_wbuf_overflow_int0 : 1 ;
        unsigned int rd1_wbuf_overflow_int0 : 1 ;
        unsigned int rd2_wbuf_overflow_int0 : 1 ;
        unsigned int reserved_1 : 2 ;
        unsigned int adq0_upoverflow_int0 : 1 ;
        unsigned int adq1_upoverflow_int0 : 1 ;
        unsigned int lbdq_upoverflow_int0 : 1 ;
        unsigned int rd0_wbuf_full_int0 : 1 ;
        unsigned int rd1_wbuf_full_int0 : 1 ;
        unsigned int rd2_wbuf_full_int0 : 1 ;
        unsigned int reserved_2 : 2 ;
        unsigned int adq0_full_int0 : 1 ;
        unsigned int adq1_full_int0 : 1 ;
        unsigned int lbdq_full_int0 : 1 ;
        unsigned int reserved_3 : 4 ;
    } bits;
    unsigned int u32;
} HI_PSAM_INT0_STAT_T;
typedef union
{
    struct
    {
        unsigned int adq0_epty_mstat0 : 1 ;
        unsigned int adq1_epty_mstat0 : 1 ;
        unsigned int lbdq_epty_mstat0 : 1 ;
        unsigned int reserved_0 : 9 ;
        unsigned int rd0_wbuf_overflow_mstat0 : 1 ;
        unsigned int rd1_wbuf_overflow_mstat0 : 1 ;
        unsigned int rd2_wbuf_overflow_mstat0 : 1 ;
        unsigned int reserved_1 : 2 ;
        unsigned int adq0_upoverflow_mstat0 : 1 ;
        unsigned int adq1_upoverflow_mstat0 : 1 ;
        unsigned int lbdq_upoverflow_mstat0 : 1 ;
        unsigned int rd0_wbuf_full_mstat0 : 1 ;
        unsigned int rd1_wbuf_full_mstat0 : 1 ;
        unsigned int rd2_wbuf_full_mstat0 : 1 ;
        unsigned int reserved_2 : 2 ;
        unsigned int adq0_full_mstat0 : 1 ;
        unsigned int adq1_full_mstat0 : 1 ;
        unsigned int lbdq_full_mstat0 : 1 ;
        unsigned int reserved_3 : 4 ;
    } bits;
    unsigned int u32;
} HI_PSAM_INT0_MSTAT_T;
typedef union
{
    struct
    {
        unsigned int adq0_epty_mask0 : 1 ;
        unsigned int adq1_epty_mask0 : 1 ;
        unsigned int lbdq_epty_mask0 : 1 ;
        unsigned int reserved_0 : 9 ;
        unsigned int rd0_wbuf_overflow_mask0 : 1 ;
        unsigned int rd1_wbuf_overflow_mask0 : 1 ;
        unsigned int rd2_wbuf_overflow_mask0 : 1 ;
        unsigned int reserved_1 : 2 ;
        unsigned int adq0_upoverflow_mask0 : 1 ;
        unsigned int adq1_upoverflow_mask0 : 1 ;
        unsigned int lbdq_upoverflow_mask0 : 1 ;
        unsigned int rd0_wbuf_full_mask0 : 1 ;
        unsigned int rd1_wbuf_full_mask0 : 1 ;
        unsigned int rd2_wbuf_full_mask0 : 1 ;
        unsigned int reserved_2 : 2 ;
        unsigned int adq0_full_mask0 : 1 ;
        unsigned int adq1_full_mask0 : 1 ;
        unsigned int lbdq_full_mask0 : 1 ;
        unsigned int reserved_3 : 4 ;
    } bits;
    unsigned int u32;
} HI_PSAM_INT0_MASK_T;
typedef union
{
    struct
    {
        unsigned int adq0_epty_int1 : 1 ;
        unsigned int adq1_epty_int1 : 1 ;
        unsigned int lbdq_epty_int1 : 1 ;
        unsigned int reserved_0 : 9 ;
        unsigned int rd0_wbuf_overflow_int1 : 1 ;
        unsigned int rd1_wbuf_overflow_int1 : 1 ;
        unsigned int rd2_wbuf_overflow_int1 : 1 ;
        unsigned int reserved_1 : 2 ;
        unsigned int adq0_upoverflow_int1 : 1 ;
        unsigned int adq1_upoverflow_int1 : 1 ;
        unsigned int lbdq_upoverflow_int1 : 1 ;
        unsigned int rd0_wbuf_full_int1 : 1 ;
        unsigned int rd1_wbuf_full_int1 : 1 ;
        unsigned int rd2_wbuf_full_int1 : 1 ;
        unsigned int reserved_2 : 2 ;
        unsigned int adq0_full_int1 : 1 ;
        unsigned int adq1_full_int1 : 1 ;
        unsigned int lbdq_full_int1 : 1 ;
        unsigned int reserved_3 : 4 ;
    } bits;
    unsigned int u32;
} HI_PSAM_INT1_STAT_T;
typedef union
{
    struct
    {
        unsigned int adq0_epty_mstat1 : 1 ;
        unsigned int adq1_epty_mstat1 : 1 ;
        unsigned int lbdq_epty_mstat1 : 1 ;
        unsigned int reserved_0 : 9 ;
        unsigned int rd0_wbuf_overflow_mstat1 : 1 ;
        unsigned int rd1_wbuf_overflow_mstat1 : 1 ;
        unsigned int rd2_wbuf_overflow_mstat1 : 1 ;
        unsigned int reserved_1 : 2 ;
        unsigned int adq0_upoverflow_mstat1 : 1 ;
        unsigned int adq1_upoverflow_mstat1 : 1 ;
        unsigned int lbdq_upoverflow_mstat1 : 1 ;
        unsigned int rd0_wbuf_full_mstat1 : 1 ;
        unsigned int rd1_wbuf_full_mstat1 : 1 ;
        unsigned int rd2_wbuf_full_mstat1 : 1 ;
        unsigned int reserved_2 : 2 ;
        unsigned int adq0_full_mstat1 : 1 ;
        unsigned int adq1_full_mstat1 : 1 ;
        unsigned int lbdq_full_mstat1 : 1 ;
        unsigned int reserved_3 : 4 ;
    } bits;
    unsigned int u32;
} HI_PSAM_INT1_MSTAT_T;
typedef union
{
    struct
    {
        unsigned int adq0_epty_mask1 : 1 ;
        unsigned int adq1_epty_mask1 : 1 ;
        unsigned int lbdq_epty_mask1 : 1 ;
        unsigned int reserved_0 : 9 ;
        unsigned int rd0_wbuf_overflow_mask1 : 1 ;
        unsigned int rd1_wbuf_overflow_mask1 : 1 ;
        unsigned int rd2_wbuf_overflow_mask1 : 1 ;
        unsigned int reserved_1 : 2 ;
        unsigned int adq0_upoverflow_mask1 : 1 ;
        unsigned int adq1_upoverflow_mask1 : 1 ;
        unsigned int lbdq_upoverflow_mask1 : 1 ;
        unsigned int rd0_wbuf_full_mask1 : 1 ;
        unsigned int rd1_wbuf_full_mask1 : 1 ;
        unsigned int rd2_wbuf_full_mask1 : 1 ;
        unsigned int reserved_2 : 2 ;
        unsigned int adq0_full_mask1 : 1 ;
        unsigned int adq1_full_mask1 : 1 ;
        unsigned int lbdq_full_mask1 : 1 ;
        unsigned int reserved_3 : 4 ;
    } bits;
    unsigned int u32;
} HI_PSAM_INT1_MASK_T;
typedef union
{
    struct
    {
        unsigned int adq0_epty_int2 : 1 ;
        unsigned int adq1_epty_int2 : 1 ;
        unsigned int lbdq_epty_int2 : 1 ;
        unsigned int reserved_0 : 9 ;
        unsigned int rd0_wbuf_overflow_int2 : 1 ;
        unsigned int rd1_wbuf_overflow_int2 : 1 ;
        unsigned int rd2_wbuf_overflow_int2 : 1 ;
        unsigned int reserved_1 : 2 ;
        unsigned int adq0_upoverflow_int2 : 1 ;
        unsigned int adq1_upoverflow_int2 : 1 ;
        unsigned int lbdq_upoverflow_int2 : 1 ;
        unsigned int rd0_wbuf_full_int2 : 1 ;
        unsigned int rd1_wbuf_full_int2 : 1 ;
        unsigned int rd2_wbuf_full_int2 : 1 ;
        unsigned int reserved_2 : 2 ;
        unsigned int adq0_full_int2 : 1 ;
        unsigned int adq1_full_int2 : 1 ;
        unsigned int lbdq_full_int2 : 1 ;
        unsigned int reserved_3 : 4 ;
    } bits;
    unsigned int u32;
} HI_PSAM_INT2_STAT_T;
typedef union
{
    struct
    {
        unsigned int adq0_epty_mstat2 : 1 ;
        unsigned int adq1_epty_mstat2 : 1 ;
        unsigned int lbdq_epty_mstat2 : 1 ;
        unsigned int reserved_0 : 9 ;
        unsigned int rd0_wbuf_overflow_mstat2 : 1 ;
        unsigned int rd1_wbuf_overflow_mstat2 : 1 ;
        unsigned int rd2_wbuf_overflow_mstat2 : 1 ;
        unsigned int reserved_1 : 2 ;
        unsigned int adq0_upoverflow_mstat2 : 1 ;
        unsigned int adq1_upoverflow_mstat2 : 1 ;
        unsigned int lbdq_upoverflow_mstat2 : 1 ;
        unsigned int rd0_wbuf_full_mstat2 : 1 ;
        unsigned int rd1_wbuf_full_mstat2 : 1 ;
        unsigned int rd2_wbuf_full_mstat2 : 1 ;
        unsigned int reserved_2 : 2 ;
        unsigned int adq0_full_mstat2 : 1 ;
        unsigned int adq1_full_mstat2 : 1 ;
        unsigned int lbdq_full_mstat2 : 1 ;
        unsigned int reserved_3 : 4 ;
    } bits;
    unsigned int u32;
} HI_PSAM_INT2_MSTAT_T;
typedef union
{
    struct
    {
        unsigned int adq0_epty_mask2 : 1 ;
        unsigned int adq1_epty_mask2 : 1 ;
        unsigned int lbdq_epty_mask2 : 1 ;
        unsigned int reserved_0 : 9 ;
        unsigned int rd0_wbuf_overflow_mask2 : 1 ;
        unsigned int rd1_wbuf_overflow_mask2 : 1 ;
        unsigned int rd2_wbuf_overflow_mask2 : 1 ;
        unsigned int reserved_1 : 2 ;
        unsigned int adq0_upoverflow_mask2 : 1 ;
        unsigned int adq1_upoverflow_mask2 : 1 ;
        unsigned int lbdq_upoverflow_mask2 : 1 ;
        unsigned int rd0_wbuf_full_mask2 : 1 ;
        unsigned int rd1_wbuf_full_mask2 : 1 ;
        unsigned int rd2_wbuf_full_mask2 : 1 ;
        unsigned int reserved_2 : 2 ;
        unsigned int adq0_full_mask2 : 1 ;
        unsigned int adq1_full_mask2 : 1 ;
        unsigned int lbdq_full_mask2 : 1 ;
        unsigned int reserved_3 : 4 ;
    } bits;
    unsigned int u32;
} HI_PSAM_INT2_MASK_T;
typedef union
{
    struct
    {
        unsigned int cipher_srst : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} HI_PSAM_CIPHER_SOFTRESET_T;
typedef union
{
    struct
    {
        unsigned int cbdq_update_upper : 10 ;
        unsigned int reserved_0 : 6 ;
        unsigned int cbdq_wptr_update_num : 10 ;
        unsigned int reserved_1 : 6 ;
    } bits;
    unsigned int u32;
} HI_PSAM_CBDQ_UPDATE_T;
typedef union
{
    struct
    {
        unsigned int cipher_ch_srst : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} HI_PSAM_CIPHER_CH_SOFTRESET_T;
typedef union
{
    struct
    {
        unsigned int cipher_ch_en : 1 ;
        unsigned int reserved_0 : 30 ;
        unsigned int cipher_ch_busy : 1 ;
    } bits;
    unsigned int u32;
} HI_PSAM_CIPHER_CH_EN_T;
typedef union
{
    struct
    {
        unsigned int cbd_iv_sel : 1 ;
        unsigned int cbd_iv_num : 1 ;
        unsigned int reserved_0 : 2 ;
        unsigned int reserved_1 : 1 ;
        unsigned int reserved_2 : 2 ;
        unsigned int reserved_3 : 25 ;
    } bits;
    unsigned int u32;
} HI_PSAM_CBDQ_CONFIG_T;
typedef union
{
    struct
    {
        unsigned int cbdq_size : 10 ;
        unsigned int reserved_0 : 22 ;
    } bits;
    unsigned int u32;
} HI_PSAM_CBDQ_SIZE_T;
typedef union
{
    struct
    {
        unsigned int reserved_0 : 3 ;
        unsigned int cbdq_base_addr_l : 29 ;
    } bits;
    unsigned int u32;
} HI_PSAM_CBDQ_BADDR_L_T;
typedef union
{
    struct
    {
        unsigned int cbdq_base_addr_h : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} HI_PSAM_CBDQ_BADDR_H_T;
typedef union
{
    struct
    {
        unsigned int cbdq_wptr : 10 ;
        unsigned int reserved_0 : 22 ;
    } bits;
    unsigned int u32;
} HI_PSAM_CBDQ_WPTR_T;
typedef union
{
    struct
    {
        unsigned int local_cbdq_wptr : 10 ;
        unsigned int cbdq_wptr_invalid : 1 ;
        unsigned int reserved_0 : 5 ;
        unsigned int local_cbdq_rptr : 10 ;
        unsigned int reserved_1 : 6 ;
    } bits;
    unsigned int u32;
} HI_PSAM_CBDQ_STAT_T;
typedef union
{
    struct
    {
        unsigned int cbdq_wptr_addr_h : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} HI_PSAM_CBDQ_WPTR_OFFSET_H_T;
typedef union
{
    struct
    {
        unsigned int fc_head : 4 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} HI_PSAM_CRDQ_CTRL_T;
typedef union
{
    struct
    {
        unsigned int crd_wbuf_full : 1 ;
        unsigned int crd_wbuf_epty : 1 ;
        unsigned int reserved_0 : 30 ;
    } bits;
    unsigned int u32;
} HI_PSAM_CRDQ_STAT_T;
typedef union
{
    struct
    {
        unsigned int local_crdq_wptr : 3 ;
        unsigned int reserved_0 : 13 ;
        unsigned int local_crdq_rptr : 3 ;
        unsigned int reserved_1 : 13 ;
    } bits;
    unsigned int u32;
} HI_PSAM_CRDQ_PTR_T;
typedef union
{
    struct
    {
        unsigned int crdq_rptr_addr_h : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} HI_PSAM_CRDQ_RPTR_OFFSET_H_T;
typedef union
{
    struct
    {
        unsigned int reserved_0 : 3 ;
        unsigned int ibdq_addr_l : 29 ;
    } bits;
    unsigned int u32;
} HI_PSAM_IBDQ_BADDR_L_T;
typedef union
{
    struct
    {
        unsigned int ibdq_addr_h : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} HI_PSAM_IBDQ_BADDR_H_T;
typedef union
{
    struct
    {
        unsigned int last_lbd_ibdq_wptr : 10 ;
        unsigned int reserved_0 : 6 ;
        unsigned int last_lbd_lbdq_rptr : 10 ;
        unsigned int reserved_1 : 5 ;
        unsigned int need_to_update : 1 ;
    } bits;
    unsigned int u32;
} HI_PSAM_IBDQ_STAT_T;
typedef union
{
    struct
    {
        unsigned int ibdq_size : 10 ;
        unsigned int reserved_0 : 22 ;
    } bits;
    unsigned int u32;
} HI_PSAM_IBDQ_SIZE_T;
typedef union
{
    struct
    {
        unsigned int ibdq_wptr : 10 ;
        unsigned int reserved_0 : 22 ;
    } bits;
    unsigned int u32;
} HI_PSAM_IBDQ_WPTR_T;
typedef union
{
    struct
    {
        unsigned int ibdq_rptr : 10 ;
        unsigned int reserved_0 : 22 ;
    } bits;
    unsigned int u32;
} HI_PSAM_IBDQ_RPTR_T;
typedef union
{
    struct
    {
        unsigned int ibdq_update_upper : 10 ;
        unsigned int reserved_0 : 22 ;
    } bits;
    unsigned int u32;
} HI_PSAM_IBDQ_UPDATE_T;
typedef union
{
    struct
    {
        unsigned int reserved_0 : 3 ;
        unsigned int lbdq_addr_l : 29 ;
    } bits;
    unsigned int u32;
} HI_PSAM_LBDQ_BADDR_L_T;
typedef union
{
    struct
    {
        unsigned int lbdq_addr_h : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} HI_PSAM_LBDQ_BADDR_H_T;
typedef union
{
    struct
    {
        unsigned int lbdq_epty : 1 ;
        unsigned int lbdq_full : 1 ;
        unsigned int lbd_rbuf_epty : 1 ;
        unsigned int lbd_rbuf_full : 1 ;
        unsigned int reserved_0 : 12 ;
        unsigned int lbdq_wptr_invalid : 1 ;
        unsigned int reserved_1 : 3 ;
        unsigned int local_lbdq_rptr : 10 ;
        unsigned int reserved_2 : 2 ;
    } bits;
    unsigned int u32;
} HI_PSAM_LBDQ_STAT_T;
typedef union
{
    struct
    {
        unsigned int lbdq_size : 10 ;
        unsigned int reserved_0 : 22 ;
    } bits;
    unsigned int u32;
} HI_PSAM_LBDQ_SIZE_T;
typedef union
{
    struct
    {
        unsigned int lbdq_wptr : 10 ;
        unsigned int reserved_0 : 22 ;
    } bits;
    unsigned int u32;
} HI_PSAM_LBDQ_WPTR_T;
typedef union
{
    struct
    {
        unsigned int real_lbdq_rptr : 10 ;
        unsigned int reserved_0 : 22 ;
    } bits;
    unsigned int u32;
} HI_PSAM_LBDQ_RPTR_T;
typedef union
{
    struct
    {
        unsigned int lbdq_depth : 10 ;
        unsigned int reserved_0 : 22 ;
    } bits;
    unsigned int u32;
} HI_PSAM_LBDQ_DEPTH_T;
typedef union
{
    struct
    {
        unsigned int adq_en : 2 ;
        unsigned int reserved_0 : 2 ;
        unsigned int adq0_size_sel : 3 ;
        unsigned int reserved_1 : 1 ;
        unsigned int adq1_size_sel : 3 ;
        unsigned int reserved_2 : 5 ;
        unsigned int adq_plen_th : 16 ;
    } bits;
    unsigned int u32;
} HI_PSAM_ADQ_CTRL_T;
typedef union
{
    struct
    {
        unsigned int adq0_epty : 1 ;
        unsigned int adq0_full : 1 ;
        unsigned int ad0_buf_epty : 1 ;
        unsigned int ad0_buf_full : 1 ;
        unsigned int adq0_rptr_invalid : 1 ;
        unsigned int adq0_wptr_invalid : 1 ;
        unsigned int reserved_0 : 26 ;
    } bits;
    unsigned int u32;
} HI_PSAM_ADQ0_STAT_T;
typedef union
{
    struct
    {
        unsigned int adq0_wptr : 11 ;
        unsigned int reserved_0 : 21 ;
    } bits;
    unsigned int u32;
} HI_PSAM_ADQ0_WPTR_T;
typedef union
{
    struct
    {
        unsigned int adq0_rptr : 11 ;
        unsigned int reserved_0 : 21 ;
    } bits;
    unsigned int u32;
} HI_PSAM_ADQ0_RPTR_T;
typedef union
{
    struct
    {
        unsigned int adq1_epty : 1 ;
        unsigned int adq1_full : 1 ;
        unsigned int ad1_buf_epty : 1 ;
        unsigned int ad1_buf_full : 1 ;
        unsigned int adq1_rptr_invalid : 1 ;
        unsigned int adq1_wptr_invalid : 1 ;
        unsigned int reserved_0 : 26 ;
    } bits;
    unsigned int u32;
} HI_PSAM_ADQ1_STAT_T;
typedef union
{
    struct
    {
        unsigned int adq1_wptr : 11 ;
        unsigned int reserved_0 : 21 ;
    } bits;
    unsigned int u32;
} HI_PSAM_ADQ1_WPTR_T;
typedef union
{
    struct
    {
        unsigned int adq1_rptr : 11 ;
        unsigned int reserved_0 : 21 ;
    } bits;
    unsigned int u32;
} HI_PSAM_ADQ1_RPTR_T;
typedef union
{
    struct
    {
        unsigned int reserved_0 : 3 ;
        unsigned int adq0_base_l : 29 ;
    } bits;
    unsigned int u32;
} HI_PSAM_ADQ0_BASE_L_T;
typedef union
{
    struct
    {
        unsigned int adq0_base_h : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} HI_PSAM_ADQ0_BASE_H_T;
typedef union
{
    struct
    {
        unsigned int reserved_0 : 3 ;
        unsigned int adq1_base_l : 29 ;
    } bits;
    unsigned int u32;
} HI_PSAM_ADQ1_BASE_L_T;
typedef union
{
    struct
    {
        unsigned int adq1_base_h : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} HI_PSAM_ADQ1_BASE_H_T;
typedef union
{
    struct
    {
        unsigned int cbdq_sec_ctrl : 1 ;
        unsigned int ibdq_sec_ctrl : 1 ;
        unsigned int lbdq_sec_ctrl : 1 ;
        unsigned int adq_sec_ctrl : 1 ;
        unsigned int creg_sec_ctrl : 1 ;
        unsigned int reserved_0 : 27 ;
    } bits;
    unsigned int u32;
} HI_PSAM_SEC_CTRL_T;
typedef union
{
    struct
    {
        unsigned int adq_fama_attr : 7 ;
        unsigned int reserved_0 : 25 ;
    } bits;
    unsigned int u32;
} HI_PSAM_ADQ_FAMA_ATTR_T;
typedef union
{
    struct
    {
        unsigned int adq_paddr_valid0 : 1 ;
        unsigned int adq_paddr_valid1 : 1 ;
        unsigned int adq_paddr_valid2 : 1 ;
        unsigned int adq_paddr_valid3 : 1 ;
        unsigned int adq_paddr_bypass : 1 ;
        unsigned int adq_paddr_fama_bypass : 1 ;
        unsigned int reserved_0 : 26 ;
    } bits;
    unsigned int u32;
} HI_PSAM_ADQ_PADDR_CTRL_T;
typedef union
{
    struct
    {
        unsigned int cbdq_fama_attr : 7 ;
        unsigned int reserved_0 : 25 ;
    } bits;
    unsigned int u32;
} HI_PSAM_CBDQ_FAMA_ATTR_T;
typedef union
{
    struct
    {
        unsigned int ibdq_fama_attr : 7 ;
        unsigned int reserved_0 : 25 ;
    } bits;
    unsigned int u32;
} HI_PSAM_IBDQ_FAMA_ATTR_T;
typedef union
{
    struct
    {
        unsigned int lbdq_fama_attr : 7 ;
        unsigned int reserved_0 : 25 ;
    } bits;
    unsigned int u32;
} HI_PSAM_LBDQ_FAMA_ATTR_T;
typedef union
{
    struct
    {
        unsigned int adq_paddr_fama0 : 4 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} HI_PSAM_ADQ_PADDR_FAMA0_T;
typedef union
{
    struct
    {
        unsigned int adq_paddr_fama1 : 4 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} HI_PSAM_ADQ_PADDR_FAMA1_T;
typedef union
{
    struct
    {
        unsigned int adq_paddr_fama2 : 4 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} HI_PSAM_ADQ_PADDR_FAMA2_T;
typedef union
{
    struct
    {
        unsigned int adq_paddr_fama3 : 4 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} HI_PSAM_ADQ_PADDR_FAMA3_T;
#endif
