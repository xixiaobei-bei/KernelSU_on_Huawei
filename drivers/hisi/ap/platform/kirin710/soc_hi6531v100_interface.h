#ifndef __SOC_HI6531V100_INTERFACE_H__
#define __SOC_HI6531V100_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_Hi6531V100_VERSION0_ADDR(base) ((base) + (0x00))
#define SOC_Hi6531V100_VERSION1_ADDR(base) ((base) + (0x01))
#define SOC_Hi6531V100_VERSION2_ADDR(base) ((base) + (0x02))
#define SOC_Hi6531V100_VERSION3_ADDR(base) ((base) + (0x03))
#define SOC_Hi6531V100_VERSION4_ADDR(base) ((base) + (0x04))
#define SOC_Hi6531V100_VERSION5_ADDR(base) ((base) + (0x05))
#define SOC_Hi6531V100_LDO_LOCK_ADDR(base) ((base) + (0x06))
#define SOC_Hi6531V100_REF_EN_STATE_ADDR(base) ((base) + (0x07))
#define SOC_Hi6531V100_LDO0_1_STATE_ADDR(base) ((base) + (0x08))
#define SOC_Hi6531V100_OSC_3M84_EN_STATE_ADDR(base) ((base) + (0x09))
#define SOC_Hi6531V100_LDO1_EN_ONOFF_ADDR(base) ((base) + (0x0A))
#define SOC_Hi6531V100_IRQ_ADDR(base) ((base) + (0x0B))
#define SOC_Hi6531V100_IRQ_STATE_ADDR(base) ((base) + (0x0C))
#define SOC_Hi6531V100_IRQ_MASK_ADDR(base) ((base) + (0x0D))
#define SOC_Hi6531V100_ACR_READY_ADDR(base) ((base) + (0x0E))
#define SOC_Hi6531V100_OTP_READY_ADDR(base) ((base) + (0x0F))
#define SOC_Hi6531V100_LDO1_VSET_CFG_ADDR(base) ((base) + (0x10))
#define SOC_Hi6531V100_LDO1_VRSET_CFG_ADDR(base) ((base) + (0x11))
#define SOC_Hi6531V100_ANA_CFG0_ADDR(base) ((base) + (0x12))
#define SOC_Hi6531V100_OSC_RESERVE_CFG_ADDR(base) ((base) + (0x13))
#define SOC_Hi6531V100_REF_RESERVED0_CFG_ADDR(base) ((base) + (0x14))
#define SOC_Hi6531V100_REF_RESERVED1_CFG_ADDR(base) ((base) + (0x15))
#define SOC_Hi6531V100_SYS_RESERVED_ADDR(base) ((base) + (0x16))
#define SOC_Hi6531V100_A2D_RESERVED_ADDR(base) ((base) + (0x17))
#define SOC_Hi6531V100_D2A_RESERVED0_ADDR(base) ((base) + (0x18))
#define SOC_Hi6531V100_D2A_RESERVED1_ADDR(base) ((base) + (0x19))
#define SOC_Hi6531V100_D2A_RESERVED2_ADDR(base) ((base) + (0x1A))
#define SOC_Hi6531V100_D2A_RESERVED3_ADDR(base) ((base) + (0x1B))
#define SOC_Hi6531V100_OTP_0_ADDR(base) ((base) + (0x26))
#define SOC_Hi6531V100_OTP_1_ADDR(base) ((base) + (0x27))
#define SOC_Hi6531V100_OTP_CTRL0_ADDR(base) ((base) + (0x28))
#define SOC_Hi6531V100_OTP_CTRL1_ADDR(base) ((base) + (0x29))
#define SOC_Hi6531V100_OTP_CTRL2_ADDR(base) ((base) + (0x2A))
#define SOC_Hi6531V100_OTP_WDATA_ADDR(base) ((base) + (0x2B))
#define SOC_Hi6531V100_OTP_0_W_ADDR(base) ((base) + (0x2C))
#define SOC_Hi6531V100_OTP_1_W_ADDR(base) ((base) + (0x2D))
#define SOC_Hi6531V100_OTP_2_W_ADDR(base) ((base) + (0x2E))
#define SOC_Hi6531V100_OTP_3_W_ADDR(base) ((base) + (0x2F))
#define SOC_Hi6531V100_OTP_4_W_ADDR(base) ((base) + (0x30))
#define SOC_Hi6531V100_OTP_5_W_ADDR(base) ((base) + (0x31))
#define SOC_Hi6531V100_OTP_6_W_ADDR(base) ((base) + (0x32))
#define SOC_Hi6531V100_OTP_7_W_ADDR(base) ((base) + (0x33))
#define SOC_Hi6531V100_OTP_8_W_ADDR(base) ((base) + (0x34))
#define SOC_Hi6531V100_OTP_9_W_ADDR(base) ((base) + (0x35))
#define SOC_Hi6531V100_OTP_10_W_ADDR(base) ((base) + (0x36))
#define SOC_Hi6531V100_OTP_11_W_ADDR(base) ((base) + (0x37))
#define SOC_Hi6531V100_OTP_12_W_ADDR(base) ((base) + (0x38))
#define SOC_Hi6531V100_OTP_13_W_ADDR(base) ((base) + (0x39))
#define SOC_Hi6531V100_OTP_14_W_ADDR(base) ((base) + (0x3A))
#define SOC_Hi6531V100_OTP_15_W_ADDR(base) ((base) + (0x3B))
#define SOC_Hi6531V100_OTP_16_W_ADDR(base) ((base) + (0x3C))
#define SOC_Hi6531V100_OTP_17_W_ADDR(base) ((base) + (0x3D))
#define SOC_Hi6531V100_ACRADC_CTRL_ADDR(base) ((base) + (0x50))
#define SOC_Hi6531V100_ACRADC_START_ADDR(base) ((base) + (0x51))
#define SOC_Hi6531V100_ACRADC_MODE_CFG_ADDR(base) ((base) + (0x52))
#define SOC_Hi6531V100_ACRADC_CALI_CTRL_ADDR(base) ((base) + (0x53))
#define SOC_Hi6531V100_ACRADC_CALI_CFG_ADDR(base) ((base) + (0x54))
#define SOC_Hi6531V100_ACRADC_CALIVALUE_CFG1_ADDR(base) ((base) + (0x55))
#define SOC_Hi6531V100_ACRADC_CALIVALUE_CFG2_ADDR(base) ((base) + (0x56))
#define SOC_Hi6531V100_ACRADC_CALI_VALUE_ADDR(base) ((base) + (0x57))
#define SOC_Hi6531V100_ACRCONV_STATUS_ADDR(base) ((base) + (0x58))
#define SOC_Hi6531V100_ACRADC_DATA0_ADDR(base) ((base) + (0x59))
#define SOC_Hi6531V100_ACRADC_DATA1_ADDR(base) ((base) + (0x5A))
#define SOC_Hi6531V100_ACRADC_CHOPPER_1ST_DATA1_ADDR(base) ((base) + (0x5B))
#define SOC_Hi6531V100_ACRADC_CHOPPER_1ST_DATA2_ADDR(base) ((base) + (0x5C))
#define SOC_Hi6531V100_ACRADC_CHOPPER_2ND_DATA1_ADDR(base) ((base) + (0x5D))
#define SOC_Hi6531V100_ACRADC_CHOPPER_2ND_DATA2_ADDR(base) ((base) + (0x5E))
#define SOC_Hi6531V100_ACRADC_CONV_ADDR(base) ((base) + (0x5F))
#define SOC_Hi6531V100_ACRADC_CURRENT_ADDR(base) ((base) + (0x60))
#define SOC_Hi6531V100_ACR_EN_ADDR(base) ((base) + (0x67))
#define SOC_Hi6531V100_ACR_PULSE_NUM_ADDR(base) ((base) + (0x68))
#define SOC_Hi6531V100_ACR_SAMPLE_TIME_H_ADDR(base) ((base) + (0x69))
#define SOC_Hi6531V100_ACR_SAMPLE_TIME_L_ADDR(base) ((base) + (0x6A))
#define SOC_Hi6531V100_ACR_DATA0_L_ADDR(base) ((base) + (0x6B))
#define SOC_Hi6531V100_ACR_DATA0_H_ADDR(base) ((base) + (0x6C))
#define SOC_Hi6531V100_ACR_DATA1_L_ADDR(base) ((base) + (0x6D))
#define SOC_Hi6531V100_ACR_DATA1_H_ADDR(base) ((base) + (0x6E))
#define SOC_Hi6531V100_ACR_DATA2_L_ADDR(base) ((base) + (0x6F))
#define SOC_Hi6531V100_ACR_DATA2_H_ADDR(base) ((base) + (0x70))
#define SOC_Hi6531V100_ACR_DATA3_L_ADDR(base) ((base) + (0x71))
#define SOC_Hi6531V100_ACR_DATA3_H_ADDR(base) ((base) + (0x72))
#define SOC_Hi6531V100_ACR_DATA4_L_ADDR(base) ((base) + (0x73))
#define SOC_Hi6531V100_ACR_DATA4_H_ADDR(base) ((base) + (0x74))
#define SOC_Hi6531V100_ACR_DATA5_L_ADDR(base) ((base) + (0x75))
#define SOC_Hi6531V100_ACR_DATA5_H_ADDR(base) ((base) + (0x76))
#define SOC_Hi6531V100_ACR_DATA6_L_ADDR(base) ((base) + (0x77))
#define SOC_Hi6531V100_ACR_DATA6_H_ADDR(base) ((base) + (0x78))
#define SOC_Hi6531V100_ACR_DATA7_L_ADDR(base) ((base) + (0x79))
#define SOC_Hi6531V100_ACR_DATA7_H_ADDR(base) ((base) + (0x7A))
#define SOC_Hi6531V100_TEST_BUS_SEL_ADDR(base) ((base) + (0x7B))
#define SOC_Hi6531V100_ACR_TB_BUS_0_ADDR(base) ((base) + (0x7C))
#define SOC_Hi6531V100_ACR_TB_BUS_1_ADDR(base) ((base) + (0x7D))
#define SOC_Hi6531V100_ACR_CLK_GT_EN_ADDR(base) ((base) + (0x7E))
#define SOC_Hi6531V100_SC_DISCHG_CFG_ADDR(base) ((base) + (0x7F))
#define SOC_Hi6531V100_ACR_CFG0_ADDR(base) ((base) + (0x86))
#define SOC_Hi6531V100_ACR_CFG1_ADDR(base) ((base) + (0x87))
#define SOC_Hi6531V100_ACR_RESERVE_CFG_ADDR(base) ((base) + (0x88))
#define SOC_Hi6531V100_RECORD_ADDR(base) ((base) + (0x90))
#define SOC_Hi6531V100_ABN_PRT_MASK_ADDR(base) ((base) + (0x91))
#define SOC_Hi6531V100_DEB_LOCK_ADDR(base) ((base) + (0x92))
#define SOC_Hi6531V100_DEB_CFG_ADDR(base) ((base) + (0x93))
#define SOC_Hi6531V100_OCP_DEB_ONOFF_ADDR(base) ((base) + (0x94))
#define SOC_Hi6531V100_ANA_CFG2_ADDR(base) ((base) + (0x95))
#define SOC_Hi6531V100_ANA_CFG3_ADDR(base) ((base) + (0x96))
#define SOC_Hi6531V100_INT_IO_CFG_ADDR(base) ((base) + (0x97))
#define SOC_Hi6531V100_DEBUG_CLK_CFG_ADDR(base) ((base) + (0x98))
#define SOC_Hi6531V100_NP_D2A_RESERVED0_ADDR(base) ((base) + (0x99))
#define SOC_Hi6531V100_NP_D2A_RESERVED1_ADDR(base) ((base) + (0x9A))
#define SOC_Hi6531V100_NP_D2A_RESERVED2_ADDR(base) ((base) + (0x9B))
#define SOC_Hi6531V100_NP_D2A_RESERVED3_ADDR(base) ((base) + (0x9C))
#define SOC_Hi6531V100_OTP_0_R_ADDR(base) ((base) + (0xA0))
#define SOC_Hi6531V100_OTP_1_R_ADDR(base) ((base) + (0xA1))
#define SOC_Hi6531V100_OTP_2_R_ADDR(base) ((base) + (0xA2))
#define SOC_Hi6531V100_OTP_3_R_ADDR(base) ((base) + (0xA3))
#define SOC_Hi6531V100_OTP_4_R_ADDR(base) ((base) + (0xA4))
#define SOC_Hi6531V100_OTP_5_R_ADDR(base) ((base) + (0xA5))
#define SOC_Hi6531V100_OTP_6_R_ADDR(base) ((base) + (0xA6))
#define SOC_Hi6531V100_OTP_7_R_ADDR(base) ((base) + (0xA7))
#define SOC_Hi6531V100_OTP_8_R_ADDR(base) ((base) + (0xA8))
#define SOC_Hi6531V100_OTP_9_R_ADDR(base) ((base) + (0xA9))
#define SOC_Hi6531V100_OTP_10_R_ADDR(base) ((base) + (0xAA))
#define SOC_Hi6531V100_OTP_11_R_ADDR(base) ((base) + (0xAB))
#define SOC_Hi6531V100_OTP_12_R_ADDR(base) ((base) + (0xAC))
#define SOC_Hi6531V100_OTP_13_R_ADDR(base) ((base) + (0xAD))
#define SOC_Hi6531V100_OTP_14_R_ADDR(base) ((base) + (0xAE))
#define SOC_Hi6531V100_OTP_15_R_ADDR(base) ((base) + (0xAF))
#define SOC_Hi6531V100_OTP_16_R_ADDR(base) ((base) + (0xB0))
#define SOC_Hi6531V100_OTP_17_R_ADDR(base) ((base) + (0xB1))
#define SOC_Hi6531V100_OTP_18_R_ADDR(base) ((base) + (0xB2))
#define SOC_Hi6531V100_OTP_19_R_ADDR(base) ((base) + (0xB3))
#define SOC_Hi6531V100_OTP_20_R_ADDR(base) ((base) + (0xB4))
#define SOC_Hi6531V100_OTP_21_R_ADDR(base) ((base) + (0xB5))
#define SOC_Hi6531V100_OTP_22_R_ADDR(base) ((base) + (0xB6))
#define SOC_Hi6531V100_OTP_23_R_ADDR(base) ((base) + (0xB7))
#define SOC_Hi6531V100_OTP_24_R_ADDR(base) ((base) + (0xB8))
#define SOC_Hi6531V100_OTP_25_R_ADDR(base) ((base) + (0xB9))
#define SOC_Hi6531V100_OTP_26_R_ADDR(base) ((base) + (0xBA))
#define SOC_Hi6531V100_OTP_27_R_ADDR(base) ((base) + (0xBB))
#define SOC_Hi6531V100_OTP_28_R_ADDR(base) ((base) + (0xBC))
#define SOC_Hi6531V100_OTP_29_R_ADDR(base) ((base) + (0xBD))
#define SOC_Hi6531V100_OTP_30_R_ADDR(base) ((base) + (0xBE))
#define SOC_Hi6531V100_OTP_31_R_ADDR(base) ((base) + (0xBF))
#define SOC_Hi6531V100_OTP_32_R_ADDR(base) ((base) + (0xC0))
#define SOC_Hi6531V100_OTP_33_R_ADDR(base) ((base) + (0xC1))
#define SOC_Hi6531V100_OTP_34_R_ADDR(base) ((base) + (0xC2))
#define SOC_Hi6531V100_OTP_35_R_ADDR(base) ((base) + (0xC3))
#define SOC_Hi6531V100_OTP_36_R_ADDR(base) ((base) + (0xC4))
#define SOC_Hi6531V100_OTP_37_R_ADDR(base) ((base) + (0xC5))
#define SOC_Hi6531V100_OTP_38_R_ADDR(base) ((base) + (0xC6))
#define SOC_Hi6531V100_OTP_39_R_ADDR(base) ((base) + (0xC7))
#define SOC_Hi6531V100_OTP_40_R_ADDR(base) ((base) + (0xC8))
#define SOC_Hi6531V100_OTP_41_R_ADDR(base) ((base) + (0xC9))
#define SOC_Hi6531V100_OTP_42_R_ADDR(base) ((base) + (0xCA))
#define SOC_Hi6531V100_OTP_43_R_ADDR(base) ((base) + (0xCB))
#define SOC_Hi6531V100_OTP_44_R_ADDR(base) ((base) + (0xCC))
#define SOC_Hi6531V100_OTP_45_R_ADDR(base) ((base) + (0xCD))
#define SOC_Hi6531V100_OTP_46_R_ADDR(base) ((base) + (0xCE))
#define SOC_Hi6531V100_OTP_47_R_ADDR(base) ((base) + (0xCF))
#define SOC_Hi6531V100_OTP_48_R_ADDR(base) ((base) + (0xD0))
#define SOC_Hi6531V100_OTP_49_R_ADDR(base) ((base) + (0xD1))
#define SOC_Hi6531V100_OTP_50_R_ADDR(base) ((base) + (0xD2))
#define SOC_Hi6531V100_OTP_51_R_ADDR(base) ((base) + (0xD3))
#define SOC_Hi6531V100_OTP_52_R_ADDR(base) ((base) + (0xD4))
#define SOC_Hi6531V100_OTP_53_R_ADDR(base) ((base) + (0xD5))
#define SOC_Hi6531V100_OTP_54_R_ADDR(base) ((base) + (0xD6))
#define SOC_Hi6531V100_OTP_55_R_ADDR(base) ((base) + (0xD7))
#define SOC_Hi6531V100_OTP_56_R_ADDR(base) ((base) + (0xD8))
#define SOC_Hi6531V100_OTP_57_R_ADDR(base) ((base) + (0xD9))
#define SOC_Hi6531V100_OTP_58_R_ADDR(base) ((base) + (0xDA))
#define SOC_Hi6531V100_OTP_59_R_ADDR(base) ((base) + (0xDB))
#define SOC_Hi6531V100_OTP_60_R_ADDR(base) ((base) + (0xDC))
#define SOC_Hi6531V100_OTP_61_R_ADDR(base) ((base) + (0xDD))
#define SOC_Hi6531V100_OTP_62_R_ADDR(base) ((base) + (0xDE))
#define SOC_Hi6531V100_OTP_63_R_ADDR(base) ((base) + (0xDF))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char project_num0 : 8;
    } reg;
} SOC_Hi6531V100_VERSION0_UNION;
#endif
#define SOC_Hi6531V100_VERSION0_project_num0_START (0)
#define SOC_Hi6531V100_VERSION0_project_num0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char project_num1 : 8;
    } reg;
} SOC_Hi6531V100_VERSION1_UNION;
#endif
#define SOC_Hi6531V100_VERSION1_project_num1_START (0)
#define SOC_Hi6531V100_VERSION1_project_num1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char project_num2 : 8;
    } reg;
} SOC_Hi6531V100_VERSION2_UNION;
#endif
#define SOC_Hi6531V100_VERSION2_project_num2_START (0)
#define SOC_Hi6531V100_VERSION2_project_num2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char project_num3 : 8;
    } reg;
} SOC_Hi6531V100_VERSION3_UNION;
#endif
#define SOC_Hi6531V100_VERSION3_project_num3_START (0)
#define SOC_Hi6531V100_VERSION3_project_num3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char version : 8;
    } reg;
} SOC_Hi6531V100_VERSION4_UNION;
#endif
#define SOC_Hi6531V100_VERSION4_version_START (0)
#define SOC_Hi6531V100_VERSION4_version_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chip_id : 8;
    } reg;
} SOC_Hi6531V100_VERSION5_UNION;
#endif
#define SOC_Hi6531V100_VERSION5_chip_id_START (0)
#define SOC_Hi6531V100_VERSION5_chip_id_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo_lock_cfg : 8;
    } reg;
} SOC_Hi6531V100_LDO_LOCK_UNION;
#endif
#define SOC_Hi6531V100_LDO_LOCK_ldo_lock_cfg_START (0)
#define SOC_Hi6531V100_LDO_LOCK_ldo_lock_cfg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char st_ref_en : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_Hi6531V100_REF_EN_STATE_UNION;
#endif
#define SOC_Hi6531V100_REF_EN_STATE_st_ref_en_START (0)
#define SOC_Hi6531V100_REF_EN_STATE_st_ref_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char st_ldo0_1_en : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_Hi6531V100_LDO0_1_STATE_UNION;
#endif
#define SOC_Hi6531V100_LDO0_1_STATE_st_ldo0_1_en_START (0)
#define SOC_Hi6531V100_LDO0_1_STATE_st_ldo0_1_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_osc_3m84_clk_en : 1;
        unsigned char st_osc_3m84_clk_en : 1;
        unsigned char reserved : 6;
    } reg;
} SOC_Hi6531V100_OSC_3M84_EN_STATE_UNION;
#endif
#define SOC_Hi6531V100_OSC_3M84_EN_STATE_reg_osc_3m84_clk_en_START (0)
#define SOC_Hi6531V100_OSC_3M84_EN_STATE_reg_osc_3m84_clk_en_END (0)
#define SOC_Hi6531V100_OSC_3M84_EN_STATE_st_osc_3m84_clk_en_START (1)
#define SOC_Hi6531V100_OSC_3M84_EN_STATE_st_osc_3m84_clk_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo1_en : 1;
        unsigned char st_ldo1_en : 1;
        unsigned char reserved : 6;
    } reg;
} SOC_Hi6531V100_LDO1_EN_ONOFF_UNION;
#endif
#define SOC_Hi6531V100_LDO1_EN_ONOFF_reg_ldo1_en_START (0)
#define SOC_Hi6531V100_LDO1_EN_ONOFF_reg_ldo1_en_END (0)
#define SOC_Hi6531V100_LDO1_EN_ONOFF_st_ldo1_en_START (1)
#define SOC_Hi6531V100_LDO1_EN_ONOFF_st_ldo1_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_flag_irq : 1;
        unsigned char ldo1_ocp_irq : 1;
        unsigned char acr_ocp_irq : 1;
        unsigned char thsd_otmp140_irq : 1;
        unsigned char thsd_otmp125_irq : 1;
        unsigned char vbat_uvp_abs_irq : 1;
        unsigned char reserved : 2;
    } reg;
} SOC_Hi6531V100_IRQ_UNION;
#endif
#define SOC_Hi6531V100_IRQ_acr_flag_irq_START (0)
#define SOC_Hi6531V100_IRQ_acr_flag_irq_END (0)
#define SOC_Hi6531V100_IRQ_ldo1_ocp_irq_START (1)
#define SOC_Hi6531V100_IRQ_ldo1_ocp_irq_END (1)
#define SOC_Hi6531V100_IRQ_acr_ocp_irq_START (2)
#define SOC_Hi6531V100_IRQ_acr_ocp_irq_END (2)
#define SOC_Hi6531V100_IRQ_thsd_otmp140_irq_START (3)
#define SOC_Hi6531V100_IRQ_thsd_otmp140_irq_END (3)
#define SOC_Hi6531V100_IRQ_thsd_otmp125_irq_START (4)
#define SOC_Hi6531V100_IRQ_thsd_otmp125_irq_END (4)
#define SOC_Hi6531V100_IRQ_vbat_uvp_abs_irq_START (5)
#define SOC_Hi6531V100_IRQ_vbat_uvp_abs_irq_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char st_acr_flag : 1;
        unsigned char st_ldo1_ocp_deb : 1;
        unsigned char st_acr_ocp_deb : 1;
        unsigned char st_thsd_otmp140_deb : 1;
        unsigned char st_thsd_otmp125_deb : 1;
        unsigned char st_vbat_uvp_abs_sy : 1;
        unsigned char reserved : 2;
    } reg;
} SOC_Hi6531V100_IRQ_STATE_UNION;
#endif
#define SOC_Hi6531V100_IRQ_STATE_st_acr_flag_START (0)
#define SOC_Hi6531V100_IRQ_STATE_st_acr_flag_END (0)
#define SOC_Hi6531V100_IRQ_STATE_st_ldo1_ocp_deb_START (1)
#define SOC_Hi6531V100_IRQ_STATE_st_ldo1_ocp_deb_END (1)
#define SOC_Hi6531V100_IRQ_STATE_st_acr_ocp_deb_START (2)
#define SOC_Hi6531V100_IRQ_STATE_st_acr_ocp_deb_END (2)
#define SOC_Hi6531V100_IRQ_STATE_st_thsd_otmp140_deb_START (3)
#define SOC_Hi6531V100_IRQ_STATE_st_thsd_otmp140_deb_END (3)
#define SOC_Hi6531V100_IRQ_STATE_st_thsd_otmp125_deb_START (4)
#define SOC_Hi6531V100_IRQ_STATE_st_thsd_otmp125_deb_END (4)
#define SOC_Hi6531V100_IRQ_STATE_st_vbat_uvp_abs_sy_START (5)
#define SOC_Hi6531V100_IRQ_STATE_st_vbat_uvp_abs_sy_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_flag_mk : 1;
        unsigned char ldo1_ocp_mk : 1;
        unsigned char acr_ocp_mk : 1;
        unsigned char thsd_otmp_140_mk : 1;
        unsigned char thsd_otmp_125_mk : 1;
        unsigned char vbat_uvp_abs_mk : 1;
        unsigned char reserved : 2;
    } reg;
} SOC_Hi6531V100_IRQ_MASK_UNION;
#endif
#define SOC_Hi6531V100_IRQ_MASK_acr_flag_mk_START (0)
#define SOC_Hi6531V100_IRQ_MASK_acr_flag_mk_END (0)
#define SOC_Hi6531V100_IRQ_MASK_ldo1_ocp_mk_START (1)
#define SOC_Hi6531V100_IRQ_MASK_ldo1_ocp_mk_END (1)
#define SOC_Hi6531V100_IRQ_MASK_acr_ocp_mk_START (2)
#define SOC_Hi6531V100_IRQ_MASK_acr_ocp_mk_END (2)
#define SOC_Hi6531V100_IRQ_MASK_thsd_otmp_140_mk_START (3)
#define SOC_Hi6531V100_IRQ_MASK_thsd_otmp_140_mk_END (3)
#define SOC_Hi6531V100_IRQ_MASK_thsd_otmp_125_mk_START (4)
#define SOC_Hi6531V100_IRQ_MASK_thsd_otmp_125_mk_END (4)
#define SOC_Hi6531V100_IRQ_MASK_vbat_uvp_abs_mk_START (5)
#define SOC_Hi6531V100_IRQ_MASK_vbat_uvp_abs_mk_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_ready : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_Hi6531V100_ACR_READY_UNION;
#endif
#define SOC_Hi6531V100_ACR_READY_acr_ready_START (0)
#define SOC_Hi6531V100_ACR_READY_acr_ready_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_ready : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_Hi6531V100_OTP_READY_UNION;
#endif
#define SOC_Hi6531V100_OTP_READY_otp_ready_START (0)
#define SOC_Hi6531V100_OTP_READY_otp_ready_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo1_vset : 3;
        unsigned char reserved : 5;
    } reg;
} SOC_Hi6531V100_LDO1_VSET_CFG_UNION;
#endif
#define SOC_Hi6531V100_LDO1_VSET_CFG_ldo1_vset_START (0)
#define SOC_Hi6531V100_LDO1_VSET_CFG_ldo1_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo1_vrset : 3;
        unsigned char reserved : 5;
    } reg;
} SOC_Hi6531V100_LDO1_VRSET_CFG_UNION;
#endif
#define SOC_Hi6531V100_LDO1_VRSET_CFG_ldo1_vrset_START (0)
#define SOC_Hi6531V100_LDO1_VRSET_CFG_ldo1_vrset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ref_ibias_trim_en : 2;
        unsigned char ref_out_sel : 2;
        unsigned char reserved_0 : 2;
        unsigned char ref_chop_sel : 1;
        unsigned char reserved_1 : 1;
    } reg;
} SOC_Hi6531V100_ANA_CFG0_UNION;
#endif
#define SOC_Hi6531V100_ANA_CFG0_ref_ibias_trim_en_START (0)
#define SOC_Hi6531V100_ANA_CFG0_ref_ibias_trim_en_END (1)
#define SOC_Hi6531V100_ANA_CFG0_ref_out_sel_START (2)
#define SOC_Hi6531V100_ANA_CFG0_ref_out_sel_END (3)
#define SOC_Hi6531V100_ANA_CFG0_ref_chop_sel_START (6)
#define SOC_Hi6531V100_ANA_CFG0_ref_chop_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char osc_3m84_reserve : 8;
    } reg;
} SOC_Hi6531V100_OSC_RESERVE_CFG_UNION;
#endif
#define SOC_Hi6531V100_OSC_RESERVE_CFG_osc_3m84_reserve_START (0)
#define SOC_Hi6531V100_OSC_RESERVE_CFG_osc_3m84_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ref_reserve0 : 8;
    } reg;
} SOC_Hi6531V100_REF_RESERVED0_CFG_UNION;
#endif
#define SOC_Hi6531V100_REF_RESERVED0_CFG_ref_reserve0_START (0)
#define SOC_Hi6531V100_REF_RESERVED0_CFG_ref_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ref_reserve1 : 8;
    } reg;
} SOC_Hi6531V100_REF_RESERVED1_CFG_UNION;
#endif
#define SOC_Hi6531V100_REF_RESERVED1_CFG_ref_reserve1_START (0)
#define SOC_Hi6531V100_REF_RESERVED1_CFG_ref_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sys_ctrl_reserve : 8;
    } reg;
} SOC_Hi6531V100_SYS_RESERVED_UNION;
#endif
#define SOC_Hi6531V100_SYS_RESERVED_sys_ctrl_reserve_START (0)
#define SOC_Hi6531V100_SYS_RESERVED_sys_ctrl_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char a2d_reserve0 : 8;
    } reg;
} SOC_Hi6531V100_A2D_RESERVED_UNION;
#endif
#define SOC_Hi6531V100_A2D_RESERVED_a2d_reserve0_START (0)
#define SOC_Hi6531V100_A2D_RESERVED_a2d_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char d2a_reserve0 : 8;
    } reg;
} SOC_Hi6531V100_D2A_RESERVED0_UNION;
#endif
#define SOC_Hi6531V100_D2A_RESERVED0_d2a_reserve0_START (0)
#define SOC_Hi6531V100_D2A_RESERVED0_d2a_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char d2a_reserve1 : 8;
    } reg;
} SOC_Hi6531V100_D2A_RESERVED1_UNION;
#endif
#define SOC_Hi6531V100_D2A_RESERVED1_d2a_reserve1_START (0)
#define SOC_Hi6531V100_D2A_RESERVED1_d2a_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char d2a_reserve2 : 8;
    } reg;
} SOC_Hi6531V100_D2A_RESERVED2_UNION;
#endif
#define SOC_Hi6531V100_D2A_RESERVED2_d2a_reserve2_START (0)
#define SOC_Hi6531V100_D2A_RESERVED2_d2a_reserve2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char d2a_reserve3 : 8;
    } reg;
} SOC_Hi6531V100_D2A_RESERVED3_UNION;
#endif
#define SOC_Hi6531V100_D2A_RESERVED3_d2a_reserve3_START (0)
#define SOC_Hi6531V100_D2A_RESERVED3_d2a_reserve3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob0 : 8;
    } reg;
} SOC_Hi6531V100_OTP_0_UNION;
#endif
#define SOC_Hi6531V100_OTP_0_otp_pdob0_START (0)
#define SOC_Hi6531V100_OTP_0_otp_pdob0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob1 : 8;
    } reg;
} SOC_Hi6531V100_OTP_1_UNION;
#endif
#define SOC_Hi6531V100_OTP_1_otp_pdob1_START (0)
#define SOC_Hi6531V100_OTP_1_otp_pdob1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pwe_int : 1;
        unsigned char otp_pwe_pulse : 1;
        unsigned char otp_por_int : 1;
        unsigned char otp_por_pulse : 1;
        unsigned char reserved : 4;
    } reg;
} SOC_Hi6531V100_OTP_CTRL0_UNION;
#endif
#define SOC_Hi6531V100_OTP_CTRL0_otp_pwe_int_START (0)
#define SOC_Hi6531V100_OTP_CTRL0_otp_pwe_int_END (0)
#define SOC_Hi6531V100_OTP_CTRL0_otp_pwe_pulse_START (1)
#define SOC_Hi6531V100_OTP_CTRL0_otp_pwe_pulse_END (1)
#define SOC_Hi6531V100_OTP_CTRL0_otp_por_int_START (2)
#define SOC_Hi6531V100_OTP_CTRL0_otp_por_int_END (2)
#define SOC_Hi6531V100_OTP_CTRL0_otp_por_pulse_START (3)
#define SOC_Hi6531V100_OTP_CTRL0_otp_por_pulse_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pprog : 1;
        unsigned char otp_inctrl_sel : 1;
        unsigned char reserved : 6;
    } reg;
} SOC_Hi6531V100_OTP_CTRL1_UNION;
#endif
#define SOC_Hi6531V100_OTP_CTRL1_otp_pprog_START (0)
#define SOC_Hi6531V100_OTP_CTRL1_otp_pprog_END (0)
#define SOC_Hi6531V100_OTP_CTRL1_otp_inctrl_sel_START (1)
#define SOC_Hi6531V100_OTP_CTRL1_otp_inctrl_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pa_cfg : 6;
        unsigned char otp_ptm : 2;
    } reg;
} SOC_Hi6531V100_OTP_CTRL2_UNION;
#endif
#define SOC_Hi6531V100_OTP_CTRL2_otp_pa_cfg_START (0)
#define SOC_Hi6531V100_OTP_CTRL2_otp_pa_cfg_END (5)
#define SOC_Hi6531V100_OTP_CTRL2_otp_ptm_START (6)
#define SOC_Hi6531V100_OTP_CTRL2_otp_ptm_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdin : 8;
    } reg;
} SOC_Hi6531V100_OTP_WDATA_UNION;
#endif
#define SOC_Hi6531V100_OTP_WDATA_otp_pdin_START (0)
#define SOC_Hi6531V100_OTP_WDATA_otp_pdin_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob0_w : 8;
    } reg;
} SOC_Hi6531V100_OTP_0_W_UNION;
#endif
#define SOC_Hi6531V100_OTP_0_W_otp_pdob0_w_START (0)
#define SOC_Hi6531V100_OTP_0_W_otp_pdob0_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob1_w : 8;
    } reg;
} SOC_Hi6531V100_OTP_1_W_UNION;
#endif
#define SOC_Hi6531V100_OTP_1_W_otp_pdob1_w_START (0)
#define SOC_Hi6531V100_OTP_1_W_otp_pdob1_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob2_w : 8;
    } reg;
} SOC_Hi6531V100_OTP_2_W_UNION;
#endif
#define SOC_Hi6531V100_OTP_2_W_otp_pdob2_w_START (0)
#define SOC_Hi6531V100_OTP_2_W_otp_pdob2_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob3_w : 8;
    } reg;
} SOC_Hi6531V100_OTP_3_W_UNION;
#endif
#define SOC_Hi6531V100_OTP_3_W_otp_pdob3_w_START (0)
#define SOC_Hi6531V100_OTP_3_W_otp_pdob3_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob4_w : 8;
    } reg;
} SOC_Hi6531V100_OTP_4_W_UNION;
#endif
#define SOC_Hi6531V100_OTP_4_W_otp_pdob4_w_START (0)
#define SOC_Hi6531V100_OTP_4_W_otp_pdob4_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob5_w : 8;
    } reg;
} SOC_Hi6531V100_OTP_5_W_UNION;
#endif
#define SOC_Hi6531V100_OTP_5_W_otp_pdob5_w_START (0)
#define SOC_Hi6531V100_OTP_5_W_otp_pdob5_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob6_w : 8;
    } reg;
} SOC_Hi6531V100_OTP_6_W_UNION;
#endif
#define SOC_Hi6531V100_OTP_6_W_otp_pdob6_w_START (0)
#define SOC_Hi6531V100_OTP_6_W_otp_pdob6_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob7_w : 8;
    } reg;
} SOC_Hi6531V100_OTP_7_W_UNION;
#endif
#define SOC_Hi6531V100_OTP_7_W_otp_pdob7_w_START (0)
#define SOC_Hi6531V100_OTP_7_W_otp_pdob7_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob8_w : 8;
    } reg;
} SOC_Hi6531V100_OTP_8_W_UNION;
#endif
#define SOC_Hi6531V100_OTP_8_W_otp_pdob8_w_START (0)
#define SOC_Hi6531V100_OTP_8_W_otp_pdob8_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob9_w : 8;
    } reg;
} SOC_Hi6531V100_OTP_9_W_UNION;
#endif
#define SOC_Hi6531V100_OTP_9_W_otp_pdob9_w_START (0)
#define SOC_Hi6531V100_OTP_9_W_otp_pdob9_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob10_w : 8;
    } reg;
} SOC_Hi6531V100_OTP_10_W_UNION;
#endif
#define SOC_Hi6531V100_OTP_10_W_otp_pdob10_w_START (0)
#define SOC_Hi6531V100_OTP_10_W_otp_pdob10_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob11_w : 8;
    } reg;
} SOC_Hi6531V100_OTP_11_W_UNION;
#endif
#define SOC_Hi6531V100_OTP_11_W_otp_pdob11_w_START (0)
#define SOC_Hi6531V100_OTP_11_W_otp_pdob11_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob12_w : 8;
    } reg;
} SOC_Hi6531V100_OTP_12_W_UNION;
#endif
#define SOC_Hi6531V100_OTP_12_W_otp_pdob12_w_START (0)
#define SOC_Hi6531V100_OTP_12_W_otp_pdob12_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob13_w : 8;
    } reg;
} SOC_Hi6531V100_OTP_13_W_UNION;
#endif
#define SOC_Hi6531V100_OTP_13_W_otp_pdob13_w_START (0)
#define SOC_Hi6531V100_OTP_13_W_otp_pdob13_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob14_w : 8;
    } reg;
} SOC_Hi6531V100_OTP_14_W_UNION;
#endif
#define SOC_Hi6531V100_OTP_14_W_otp_pdob14_w_START (0)
#define SOC_Hi6531V100_OTP_14_W_otp_pdob14_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob15_w : 8;
    } reg;
} SOC_Hi6531V100_OTP_15_W_UNION;
#endif
#define SOC_Hi6531V100_OTP_15_W_otp_pdob15_w_START (0)
#define SOC_Hi6531V100_OTP_15_W_otp_pdob15_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob16_w : 8;
    } reg;
} SOC_Hi6531V100_OTP_16_W_UNION;
#endif
#define SOC_Hi6531V100_OTP_16_W_otp_pdob16_w_START (0)
#define SOC_Hi6531V100_OTP_16_W_otp_pdob16_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob17_w : 8;
    } reg;
} SOC_Hi6531V100_OTP_17_W_UNION;
#endif
#define SOC_Hi6531V100_OTP_17_W_otp_pdob17_w_START (0)
#define SOC_Hi6531V100_OTP_17_W_otp_pdob17_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hkadc_chanl_sel : 5;
        unsigned char reserved : 2;
        unsigned char sc_hkadc_bypass : 1;
    } reg;
} SOC_Hi6531V100_ACRADC_CTRL_UNION;
#endif
#define SOC_Hi6531V100_ACRADC_CTRL_sc_hkadc_chanl_sel_START (0)
#define SOC_Hi6531V100_ACRADC_CTRL_sc_hkadc_chanl_sel_END (4)
#define SOC_Hi6531V100_ACRADC_CTRL_sc_hkadc_bypass_START (7)
#define SOC_Hi6531V100_ACRADC_CTRL_sc_hkadc_bypass_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hkadc_start : 1;
        unsigned char hkadc_reserve : 7;
    } reg;
} SOC_Hi6531V100_ACRADC_START_UNION;
#endif
#define SOC_Hi6531V100_ACRADC_START_sc_hkadc_start_START (0)
#define SOC_Hi6531V100_ACRADC_START_sc_hkadc_start_END (0)
#define SOC_Hi6531V100_ACRADC_START_hkadc_reserve_START (1)
#define SOC_Hi6531V100_ACRADC_START_hkadc_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hkadc_chopper_en : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_Hi6531V100_ACRADC_MODE_CFG_UNION;
#endif
#define SOC_Hi6531V100_ACRADC_MODE_CFG_sc_hkadc_chopper_en_START (0)
#define SOC_Hi6531V100_ACRADC_MODE_CFG_sc_hkadc_chopper_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hkadc_cali_en : 1;
        unsigned char sc_hkadc_cali_sel : 1;
        unsigned char reserved : 6;
    } reg;
} SOC_Hi6531V100_ACRADC_CALI_CTRL_UNION;
#endif
#define SOC_Hi6531V100_ACRADC_CALI_CTRL_sc_hkadc_cali_en_START (0)
#define SOC_Hi6531V100_ACRADC_CALI_CTRL_sc_hkadc_cali_en_END (0)
#define SOC_Hi6531V100_ACRADC_CALI_CTRL_sc_hkadc_cali_sel_START (1)
#define SOC_Hi6531V100_ACRADC_CALI_CTRL_sc_hkadc_cali_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hkadc_cali : 8;
    } reg;
} SOC_Hi6531V100_ACRADC_CALI_CFG_UNION;
#endif
#define SOC_Hi6531V100_ACRADC_CALI_CFG_sc_hkadc_cali_START (0)
#define SOC_Hi6531V100_ACRADC_CALI_CFG_sc_hkadc_cali_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hkadc_cali_offset_l : 8;
    } reg;
} SOC_Hi6531V100_ACRADC_CALIVALUE_CFG1_UNION;
#endif
#define SOC_Hi6531V100_ACRADC_CALIVALUE_CFG1_sc_hkadc_cali_offset_l_START (0)
#define SOC_Hi6531V100_ACRADC_CALIVALUE_CFG1_sc_hkadc_cali_offset_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hkadc_cali_offset_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_Hi6531V100_ACRADC_CALIVALUE_CFG2_UNION;
#endif
#define SOC_Hi6531V100_ACRADC_CALIVALUE_CFG2_sc_hkadc_cali_offset_h_START (0)
#define SOC_Hi6531V100_ACRADC_CALIVALUE_CFG2_sc_hkadc_cali_offset_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_cali_data : 8;
    } reg;
} SOC_Hi6531V100_ACRADC_CALI_VALUE_UNION;
#endif
#define SOC_Hi6531V100_ACRADC_CALI_VALUE_hkadc_cali_data_START (0)
#define SOC_Hi6531V100_ACRADC_CALI_VALUE_hkadc_cali_data_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_valid : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_Hi6531V100_ACRCONV_STATUS_UNION;
#endif
#define SOC_Hi6531V100_ACRCONV_STATUS_hkadc_valid_START (0)
#define SOC_Hi6531V100_ACRCONV_STATUS_hkadc_valid_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_smp_data_l : 8;
    } reg;
} SOC_Hi6531V100_ACRADC_DATA0_UNION;
#endif
#define SOC_Hi6531V100_ACRADC_DATA0_hkadc_smp_data_l_START (0)
#define SOC_Hi6531V100_ACRADC_DATA0_hkadc_smp_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_smp_data_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_Hi6531V100_ACRADC_DATA1_UNION;
#endif
#define SOC_Hi6531V100_ACRADC_DATA1_hkadc_smp_data_h_START (0)
#define SOC_Hi6531V100_ACRADC_DATA1_hkadc_smp_data_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_chopper_1st_data_l : 8;
    } reg;
} SOC_Hi6531V100_ACRADC_CHOPPER_1ST_DATA1_UNION;
#endif
#define SOC_Hi6531V100_ACRADC_CHOPPER_1ST_DATA1_hkadc_chopper_1st_data_l_START (0)
#define SOC_Hi6531V100_ACRADC_CHOPPER_1ST_DATA1_hkadc_chopper_1st_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_chopper_1st_data_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_Hi6531V100_ACRADC_CHOPPER_1ST_DATA2_UNION;
#endif
#define SOC_Hi6531V100_ACRADC_CHOPPER_1ST_DATA2_hkadc_chopper_1st_data_h_START (0)
#define SOC_Hi6531V100_ACRADC_CHOPPER_1ST_DATA2_hkadc_chopper_1st_data_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_chopper_2nd_data_l : 8;
    } reg;
} SOC_Hi6531V100_ACRADC_CHOPPER_2ND_DATA1_UNION;
#endif
#define SOC_Hi6531V100_ACRADC_CHOPPER_2ND_DATA1_hkadc_chopper_2nd_data_l_START (0)
#define SOC_Hi6531V100_ACRADC_CHOPPER_2ND_DATA1_hkadc_chopper_2nd_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_chopper_2nd_data_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_Hi6531V100_ACRADC_CHOPPER_2ND_DATA2_UNION;
#endif
#define SOC_Hi6531V100_ACRADC_CHOPPER_2ND_DATA2_hkadc_chopper_2nd_data_h_START (0)
#define SOC_Hi6531V100_ACRADC_CHOPPER_2ND_DATA2_hkadc_chopper_2nd_data_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_buffer_sel : 1;
        unsigned char hkadc_config : 7;
    } reg;
} SOC_Hi6531V100_ACRADC_CONV_UNION;
#endif
#define SOC_Hi6531V100_ACRADC_CONV_hkadc_buffer_sel_START (0)
#define SOC_Hi6531V100_ACRADC_CONV_hkadc_buffer_sel_END (0)
#define SOC_Hi6531V100_ACRADC_CONV_hkadc_config_START (1)
#define SOC_Hi6531V100_ACRADC_CONV_hkadc_config_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_ibias_sel : 8;
    } reg;
} SOC_Hi6531V100_ACRADC_CURRENT_UNION;
#endif
#define SOC_Hi6531V100_ACRADC_CURRENT_hkadc_ibias_sel_START (0)
#define SOC_Hi6531V100_ACRADC_CURRENT_hkadc_ibias_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_acr_en : 8;
    } reg;
} SOC_Hi6531V100_ACR_EN_UNION;
#endif
#define SOC_Hi6531V100_ACR_EN_sc_acr_en_START (0)
#define SOC_Hi6531V100_ACR_EN_sc_acr_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_acr_pulse_num : 2;
        unsigned char reserved : 6;
    } reg;
} SOC_Hi6531V100_ACR_PULSE_NUM_UNION;
#endif
#define SOC_Hi6531V100_ACR_PULSE_NUM_sc_acr_pulse_num_START (0)
#define SOC_Hi6531V100_ACR_PULSE_NUM_sc_acr_pulse_num_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_acr_sample_t2 : 3;
        unsigned char sc_acr_sample_t1 : 3;
        unsigned char reserved : 2;
    } reg;
} SOC_Hi6531V100_ACR_SAMPLE_TIME_H_UNION;
#endif
#define SOC_Hi6531V100_ACR_SAMPLE_TIME_H_sc_acr_sample_t2_START (0)
#define SOC_Hi6531V100_ACR_SAMPLE_TIME_H_sc_acr_sample_t2_END (2)
#define SOC_Hi6531V100_ACR_SAMPLE_TIME_H_sc_acr_sample_t1_START (3)
#define SOC_Hi6531V100_ACR_SAMPLE_TIME_H_sc_acr_sample_t1_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_acr_sample_t4 : 3;
        unsigned char sc_acr_sample_t3 : 3;
        unsigned char reserved : 2;
    } reg;
} SOC_Hi6531V100_ACR_SAMPLE_TIME_L_UNION;
#endif
#define SOC_Hi6531V100_ACR_SAMPLE_TIME_L_sc_acr_sample_t4_START (0)
#define SOC_Hi6531V100_ACR_SAMPLE_TIME_L_sc_acr_sample_t4_END (2)
#define SOC_Hi6531V100_ACR_SAMPLE_TIME_L_sc_acr_sample_t3_START (3)
#define SOC_Hi6531V100_ACR_SAMPLE_TIME_L_sc_acr_sample_t3_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data0_l : 8;
    } reg;
} SOC_Hi6531V100_ACR_DATA0_L_UNION;
#endif
#define SOC_Hi6531V100_ACR_DATA0_L_acr_data0_l_START (0)
#define SOC_Hi6531V100_ACR_DATA0_L_acr_data0_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data0_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_Hi6531V100_ACR_DATA0_H_UNION;
#endif
#define SOC_Hi6531V100_ACR_DATA0_H_acr_data0_h_START (0)
#define SOC_Hi6531V100_ACR_DATA0_H_acr_data0_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data1_l : 8;
    } reg;
} SOC_Hi6531V100_ACR_DATA1_L_UNION;
#endif
#define SOC_Hi6531V100_ACR_DATA1_L_acr_data1_l_START (0)
#define SOC_Hi6531V100_ACR_DATA1_L_acr_data1_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data1_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_Hi6531V100_ACR_DATA1_H_UNION;
#endif
#define SOC_Hi6531V100_ACR_DATA1_H_acr_data1_h_START (0)
#define SOC_Hi6531V100_ACR_DATA1_H_acr_data1_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data2_l : 8;
    } reg;
} SOC_Hi6531V100_ACR_DATA2_L_UNION;
#endif
#define SOC_Hi6531V100_ACR_DATA2_L_acr_data2_l_START (0)
#define SOC_Hi6531V100_ACR_DATA2_L_acr_data2_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data2_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_Hi6531V100_ACR_DATA2_H_UNION;
#endif
#define SOC_Hi6531V100_ACR_DATA2_H_acr_data2_h_START (0)
#define SOC_Hi6531V100_ACR_DATA2_H_acr_data2_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data3_l : 8;
    } reg;
} SOC_Hi6531V100_ACR_DATA3_L_UNION;
#endif
#define SOC_Hi6531V100_ACR_DATA3_L_acr_data3_l_START (0)
#define SOC_Hi6531V100_ACR_DATA3_L_acr_data3_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data3_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_Hi6531V100_ACR_DATA3_H_UNION;
#endif
#define SOC_Hi6531V100_ACR_DATA3_H_acr_data3_h_START (0)
#define SOC_Hi6531V100_ACR_DATA3_H_acr_data3_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data4_l : 8;
    } reg;
} SOC_Hi6531V100_ACR_DATA4_L_UNION;
#endif
#define SOC_Hi6531V100_ACR_DATA4_L_acr_data4_l_START (0)
#define SOC_Hi6531V100_ACR_DATA4_L_acr_data4_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data4_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_Hi6531V100_ACR_DATA4_H_UNION;
#endif
#define SOC_Hi6531V100_ACR_DATA4_H_acr_data4_h_START (0)
#define SOC_Hi6531V100_ACR_DATA4_H_acr_data4_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data5_l : 8;
    } reg;
} SOC_Hi6531V100_ACR_DATA5_L_UNION;
#endif
#define SOC_Hi6531V100_ACR_DATA5_L_acr_data5_l_START (0)
#define SOC_Hi6531V100_ACR_DATA5_L_acr_data5_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data5_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_Hi6531V100_ACR_DATA5_H_UNION;
#endif
#define SOC_Hi6531V100_ACR_DATA5_H_acr_data5_h_START (0)
#define SOC_Hi6531V100_ACR_DATA5_H_acr_data5_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data6_l : 8;
    } reg;
} SOC_Hi6531V100_ACR_DATA6_L_UNION;
#endif
#define SOC_Hi6531V100_ACR_DATA6_L_acr_data6_l_START (0)
#define SOC_Hi6531V100_ACR_DATA6_L_acr_data6_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data6_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_Hi6531V100_ACR_DATA6_H_UNION;
#endif
#define SOC_Hi6531V100_ACR_DATA6_H_acr_data6_h_START (0)
#define SOC_Hi6531V100_ACR_DATA6_H_acr_data6_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data7_l : 8;
    } reg;
} SOC_Hi6531V100_ACR_DATA7_L_UNION;
#endif
#define SOC_Hi6531V100_ACR_DATA7_L_acr_data7_l_START (0)
#define SOC_Hi6531V100_ACR_DATA7_L_acr_data7_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data7_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_Hi6531V100_ACR_DATA7_H_UNION;
#endif
#define SOC_Hi6531V100_ACR_DATA7_H_acr_data7_h_START (0)
#define SOC_Hi6531V100_ACR_DATA7_H_acr_data7_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_acr_tb_sel : 4;
        unsigned char sc_acr_tb_en : 1;
        unsigned char reserved : 3;
    } reg;
} SOC_Hi6531V100_TEST_BUS_SEL_UNION;
#endif
#define SOC_Hi6531V100_TEST_BUS_SEL_sc_acr_tb_sel_START (0)
#define SOC_Hi6531V100_TEST_BUS_SEL_sc_acr_tb_sel_END (3)
#define SOC_Hi6531V100_TEST_BUS_SEL_sc_acr_tb_en_START (4)
#define SOC_Hi6531V100_TEST_BUS_SEL_sc_acr_tb_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_tb_bus_0 : 8;
    } reg;
} SOC_Hi6531V100_ACR_TB_BUS_0_UNION;
#endif
#define SOC_Hi6531V100_ACR_TB_BUS_0_acr_tb_bus_0_START (0)
#define SOC_Hi6531V100_ACR_TB_BUS_0_acr_tb_bus_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_tb_bus_1 : 8;
    } reg;
} SOC_Hi6531V100_ACR_TB_BUS_1_UNION;
#endif
#define SOC_Hi6531V100_ACR_TB_BUS_1_acr_tb_bus_1_START (0)
#define SOC_Hi6531V100_ACR_TB_BUS_1_acr_tb_bus_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_acr_clk_en : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_Hi6531V100_ACR_CLK_GT_EN_UNION;
#endif
#define SOC_Hi6531V100_ACR_CLK_GT_EN_sc_acr_clk_en_START (0)
#define SOC_Hi6531V100_ACR_CLK_GT_EN_sc_acr_clk_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_dischg_en : 8;
    } reg;
} SOC_Hi6531V100_SC_DISCHG_CFG_UNION;
#endif
#define SOC_Hi6531V100_SC_DISCHG_CFG_sc_dischg_en_START (0)
#define SOC_Hi6531V100_SC_DISCHG_CFG_sc_dischg_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ovp_idis_sel : 4;
        unsigned char acr_cap_sel : 2;
        unsigned char acr_mul_sel : 2;
    } reg;
} SOC_Hi6531V100_ACR_CFG0_UNION;
#endif
#define SOC_Hi6531V100_ACR_CFG0_ovp_idis_sel_START (0)
#define SOC_Hi6531V100_ACR_CFG0_ovp_idis_sel_END (3)
#define SOC_Hi6531V100_ACR_CFG0_acr_cap_sel_START (4)
#define SOC_Hi6531V100_ACR_CFG0_acr_cap_sel_END (5)
#define SOC_Hi6531V100_ACR_CFG0_acr_mul_sel_START (6)
#define SOC_Hi6531V100_ACR_CFG0_acr_mul_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_testmode : 1;
        unsigned char acr_iref_sel : 1;
        unsigned char reserved : 6;
    } reg;
} SOC_Hi6531V100_ACR_CFG1_UNION;
#endif
#define SOC_Hi6531V100_ACR_CFG1_acr_testmode_START (0)
#define SOC_Hi6531V100_ACR_CFG1_acr_testmode_END (0)
#define SOC_Hi6531V100_ACR_CFG1_acr_iref_sel_START (1)
#define SOC_Hi6531V100_ACR_CFG1_acr_iref_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_reserve : 8;
    } reg;
} SOC_Hi6531V100_ACR_RESERVE_CFG_UNION;
#endif
#define SOC_Hi6531V100_ACR_RESERVE_CFG_acr_reserve_START (0)
#define SOC_Hi6531V100_ACR_RESERVE_CFG_acr_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_acr_flag : 1;
        unsigned char np_ldo1_ocp : 1;
        unsigned char np_acr_ocp : 1;
        unsigned char np_thsd_otmp140 : 1;
        unsigned char np_thsd_otmp125 : 1;
        unsigned char np_vbat_uvp_abs : 1;
        unsigned char np_vbat_pwron : 1;
        unsigned char reserved : 1;
    } reg;
} SOC_Hi6531V100_RECORD_UNION;
#endif
#define SOC_Hi6531V100_RECORD_np_acr_flag_START (0)
#define SOC_Hi6531V100_RECORD_np_acr_flag_END (0)
#define SOC_Hi6531V100_RECORD_np_ldo1_ocp_START (1)
#define SOC_Hi6531V100_RECORD_np_ldo1_ocp_END (1)
#define SOC_Hi6531V100_RECORD_np_acr_ocp_START (2)
#define SOC_Hi6531V100_RECORD_np_acr_ocp_END (2)
#define SOC_Hi6531V100_RECORD_np_thsd_otmp140_START (3)
#define SOC_Hi6531V100_RECORD_np_thsd_otmp140_END (3)
#define SOC_Hi6531V100_RECORD_np_thsd_otmp125_START (4)
#define SOC_Hi6531V100_RECORD_np_thsd_otmp125_END (4)
#define SOC_Hi6531V100_RECORD_np_vbat_uvp_abs_START (5)
#define SOC_Hi6531V100_RECORD_np_vbat_uvp_abs_END (5)
#define SOC_Hi6531V100_RECORD_np_vbat_pwron_START (6)
#define SOC_Hi6531V100_RECORD_np_vbat_pwron_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo1_ocp_pm : 1;
        unsigned char np_acr_ocp_pm : 1;
        unsigned char np_thsd_otmp140_pm : 1;
        unsigned char np_thsd_otmp125_pm : 1;
        unsigned char np_vbat_uvp_abs_pm : 1;
        unsigned char np_vbat_pwron_pm : 1;
        unsigned char reserved : 2;
    } reg;
} SOC_Hi6531V100_ABN_PRT_MASK_UNION;
#endif
#define SOC_Hi6531V100_ABN_PRT_MASK_np_ldo1_ocp_pm_START (0)
#define SOC_Hi6531V100_ABN_PRT_MASK_np_ldo1_ocp_pm_END (0)
#define SOC_Hi6531V100_ABN_PRT_MASK_np_acr_ocp_pm_START (1)
#define SOC_Hi6531V100_ABN_PRT_MASK_np_acr_ocp_pm_END (1)
#define SOC_Hi6531V100_ABN_PRT_MASK_np_thsd_otmp140_pm_START (2)
#define SOC_Hi6531V100_ABN_PRT_MASK_np_thsd_otmp140_pm_END (2)
#define SOC_Hi6531V100_ABN_PRT_MASK_np_thsd_otmp125_pm_START (3)
#define SOC_Hi6531V100_ABN_PRT_MASK_np_thsd_otmp125_pm_END (3)
#define SOC_Hi6531V100_ABN_PRT_MASK_np_vbat_uvp_abs_pm_START (4)
#define SOC_Hi6531V100_ABN_PRT_MASK_np_vbat_uvp_abs_pm_END (4)
#define SOC_Hi6531V100_ABN_PRT_MASK_np_vbat_pwron_pm_START (5)
#define SOC_Hi6531V100_ABN_PRT_MASK_np_vbat_pwron_pm_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_deb_lock_cfg : 8;
    } reg;
} SOC_Hi6531V100_DEB_LOCK_UNION;
#endif
#define SOC_Hi6531V100_DEB_LOCK_np_deb_lock_cfg_START (0)
#define SOC_Hi6531V100_DEB_LOCK_np_deb_lock_cfg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo1_ocp_deb_sel : 2;
        unsigned char np_acr_ocp_deb_sel : 2;
        unsigned char np_otmp125_deb_sel : 1;
        unsigned char np_otmp140_deb_sel : 1;
        unsigned char reserved : 2;
    } reg;
} SOC_Hi6531V100_DEB_CFG_UNION;
#endif
#define SOC_Hi6531V100_DEB_CFG_np_ldo1_ocp_deb_sel_START (0)
#define SOC_Hi6531V100_DEB_CFG_np_ldo1_ocp_deb_sel_END (1)
#define SOC_Hi6531V100_DEB_CFG_np_acr_ocp_deb_sel_START (2)
#define SOC_Hi6531V100_DEB_CFG_np_acr_ocp_deb_sel_END (3)
#define SOC_Hi6531V100_DEB_CFG_np_otmp125_deb_sel_START (4)
#define SOC_Hi6531V100_DEB_CFG_np_otmp125_deb_sel_END (4)
#define SOC_Hi6531V100_DEB_CFG_np_otmp140_deb_sel_START (5)
#define SOC_Hi6531V100_DEB_CFG_np_otmp140_deb_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_en_ldo1_ocp_deb : 1;
        unsigned char np_en_acr_ocp_deb : 1;
        unsigned char reserved : 6;
    } reg;
} SOC_Hi6531V100_OCP_DEB_ONOFF_UNION;
#endif
#define SOC_Hi6531V100_OCP_DEB_ONOFF_np_en_ldo1_ocp_deb_START (0)
#define SOC_Hi6531V100_OCP_DEB_ONOFF_np_en_ldo1_ocp_deb_END (0)
#define SOC_Hi6531V100_OCP_DEB_ONOFF_np_en_acr_ocp_deb_START (1)
#define SOC_Hi6531V100_OCP_DEB_ONOFF_np_en_acr_ocp_deb_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_vbat_uvp_abs_set : 2;
        unsigned char np_vbat_pwron_set : 2;
        unsigned char reserved : 4;
    } reg;
} SOC_Hi6531V100_ANA_CFG2_UNION;
#endif
#define SOC_Hi6531V100_ANA_CFG2_np_vbat_uvp_abs_set_START (0)
#define SOC_Hi6531V100_ANA_CFG2_np_vbat_uvp_abs_set_END (1)
#define SOC_Hi6531V100_ANA_CFG2_np_vbat_pwron_set_START (2)
#define SOC_Hi6531V100_ANA_CFG2_np_vbat_pwron_set_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_thsd_tmp_set : 2;
        unsigned char reserved : 6;
    } reg;
} SOC_Hi6531V100_ANA_CFG3_UNION;
#endif
#define SOC_Hi6531V100_ANA_CFG3_np_thsd_tmp_set_START (0)
#define SOC_Hi6531V100_ANA_CFG3_np_thsd_tmp_set_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_sc_int_io_sel : 8;
    } reg;
} SOC_Hi6531V100_INT_IO_CFG_UNION;
#endif
#define SOC_Hi6531V100_INT_IO_CFG_np_sc_int_io_sel_START (0)
#define SOC_Hi6531V100_INT_IO_CFG_np_sc_int_io_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_debug_io_sig_sel : 8;
    } reg;
} SOC_Hi6531V100_DEBUG_CLK_CFG_UNION;
#endif
#define SOC_Hi6531V100_DEBUG_CLK_CFG_np_debug_io_sig_sel_START (0)
#define SOC_Hi6531V100_DEBUG_CLK_CFG_np_debug_io_sig_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_d2a_reserve0 : 8;
    } reg;
} SOC_Hi6531V100_NP_D2A_RESERVED0_UNION;
#endif
#define SOC_Hi6531V100_NP_D2A_RESERVED0_np_d2a_reserve0_START (0)
#define SOC_Hi6531V100_NP_D2A_RESERVED0_np_d2a_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_d2a_reserve1 : 8;
    } reg;
} SOC_Hi6531V100_NP_D2A_RESERVED1_UNION;
#endif
#define SOC_Hi6531V100_NP_D2A_RESERVED1_np_d2a_reserve1_START (0)
#define SOC_Hi6531V100_NP_D2A_RESERVED1_np_d2a_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_d2a_reserve2 : 8;
    } reg;
} SOC_Hi6531V100_NP_D2A_RESERVED2_UNION;
#endif
#define SOC_Hi6531V100_NP_D2A_RESERVED2_np_d2a_reserve2_START (0)
#define SOC_Hi6531V100_NP_D2A_RESERVED2_np_d2a_reserve2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_d2a_reserve3 : 8;
    } reg;
} SOC_Hi6531V100_NP_D2A_RESERVED3_UNION;
#endif
#define SOC_Hi6531V100_NP_D2A_RESERVED3_np_d2a_reserve3_START (0)
#define SOC_Hi6531V100_NP_D2A_RESERVED3_np_d2a_reserve3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob0_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_0_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_0_R_np_otp_pdob0_d2a_START (0)
#define SOC_Hi6531V100_OTP_0_R_np_otp_pdob0_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob1_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_1_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_1_R_np_otp_pdob1_d2a_START (0)
#define SOC_Hi6531V100_OTP_1_R_np_otp_pdob1_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob2_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_2_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_2_R_np_otp_pdob2_d2a_START (0)
#define SOC_Hi6531V100_OTP_2_R_np_otp_pdob2_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob3_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_3_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_3_R_np_otp_pdob3_d2a_START (0)
#define SOC_Hi6531V100_OTP_3_R_np_otp_pdob3_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob4_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_4_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_4_R_np_otp_pdob4_d2a_START (0)
#define SOC_Hi6531V100_OTP_4_R_np_otp_pdob4_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob5_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_5_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_5_R_np_otp_pdob5_d2a_START (0)
#define SOC_Hi6531V100_OTP_5_R_np_otp_pdob5_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob6_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_6_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_6_R_np_otp_pdob6_d2a_START (0)
#define SOC_Hi6531V100_OTP_6_R_np_otp_pdob6_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob7_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_7_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_7_R_np_otp_pdob7_d2a_START (0)
#define SOC_Hi6531V100_OTP_7_R_np_otp_pdob7_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob8_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_8_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_8_R_np_otp_pdob8_d2a_START (0)
#define SOC_Hi6531V100_OTP_8_R_np_otp_pdob8_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob9_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_9_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_9_R_np_otp_pdob9_d2a_START (0)
#define SOC_Hi6531V100_OTP_9_R_np_otp_pdob9_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob10_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_10_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_10_R_np_otp_pdob10_d2a_START (0)
#define SOC_Hi6531V100_OTP_10_R_np_otp_pdob10_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob11_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_11_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_11_R_np_otp_pdob11_d2a_START (0)
#define SOC_Hi6531V100_OTP_11_R_np_otp_pdob11_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob12_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_12_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_12_R_np_otp_pdob12_d2a_START (0)
#define SOC_Hi6531V100_OTP_12_R_np_otp_pdob12_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob13_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_13_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_13_R_np_otp_pdob13_d2a_START (0)
#define SOC_Hi6531V100_OTP_13_R_np_otp_pdob13_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob14_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_14_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_14_R_np_otp_pdob14_d2a_START (0)
#define SOC_Hi6531V100_OTP_14_R_np_otp_pdob14_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob15_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_15_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_15_R_np_otp_pdob15_d2a_START (0)
#define SOC_Hi6531V100_OTP_15_R_np_otp_pdob15_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob16_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_16_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_16_R_np_otp_pdob16_d2a_START (0)
#define SOC_Hi6531V100_OTP_16_R_np_otp_pdob16_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob17_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_17_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_17_R_np_otp_pdob17_d2a_START (0)
#define SOC_Hi6531V100_OTP_17_R_np_otp_pdob17_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob18_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_18_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_18_R_np_otp_pdob18_d2a_START (0)
#define SOC_Hi6531V100_OTP_18_R_np_otp_pdob18_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob19_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_19_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_19_R_np_otp_pdob19_d2a_START (0)
#define SOC_Hi6531V100_OTP_19_R_np_otp_pdob19_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob20_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_20_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_20_R_np_otp_pdob20_d2a_START (0)
#define SOC_Hi6531V100_OTP_20_R_np_otp_pdob20_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob21_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_21_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_21_R_np_otp_pdob21_d2a_START (0)
#define SOC_Hi6531V100_OTP_21_R_np_otp_pdob21_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob22_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_22_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_22_R_np_otp_pdob22_d2a_START (0)
#define SOC_Hi6531V100_OTP_22_R_np_otp_pdob22_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob23_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_23_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_23_R_np_otp_pdob23_d2a_START (0)
#define SOC_Hi6531V100_OTP_23_R_np_otp_pdob23_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob24_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_24_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_24_R_np_otp_pdob24_d2a_START (0)
#define SOC_Hi6531V100_OTP_24_R_np_otp_pdob24_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob25_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_25_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_25_R_np_otp_pdob25_d2a_START (0)
#define SOC_Hi6531V100_OTP_25_R_np_otp_pdob25_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob26_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_26_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_26_R_np_otp_pdob26_d2a_START (0)
#define SOC_Hi6531V100_OTP_26_R_np_otp_pdob26_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob27_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_27_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_27_R_np_otp_pdob27_d2a_START (0)
#define SOC_Hi6531V100_OTP_27_R_np_otp_pdob27_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob28_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_28_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_28_R_np_otp_pdob28_d2a_START (0)
#define SOC_Hi6531V100_OTP_28_R_np_otp_pdob28_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob29_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_29_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_29_R_np_otp_pdob29_d2a_START (0)
#define SOC_Hi6531V100_OTP_29_R_np_otp_pdob29_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob30_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_30_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_30_R_np_otp_pdob30_d2a_START (0)
#define SOC_Hi6531V100_OTP_30_R_np_otp_pdob30_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob31_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_31_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_31_R_np_otp_pdob31_d2a_START (0)
#define SOC_Hi6531V100_OTP_31_R_np_otp_pdob31_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob32_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_32_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_32_R_np_otp_pdob32_d2a_START (0)
#define SOC_Hi6531V100_OTP_32_R_np_otp_pdob32_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob33_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_33_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_33_R_np_otp_pdob33_d2a_START (0)
#define SOC_Hi6531V100_OTP_33_R_np_otp_pdob33_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob34_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_34_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_34_R_np_otp_pdob34_d2a_START (0)
#define SOC_Hi6531V100_OTP_34_R_np_otp_pdob34_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob35_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_35_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_35_R_np_otp_pdob35_d2a_START (0)
#define SOC_Hi6531V100_OTP_35_R_np_otp_pdob35_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob36_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_36_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_36_R_np_otp_pdob36_d2a_START (0)
#define SOC_Hi6531V100_OTP_36_R_np_otp_pdob36_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob37_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_37_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_37_R_np_otp_pdob37_d2a_START (0)
#define SOC_Hi6531V100_OTP_37_R_np_otp_pdob37_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob38_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_38_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_38_R_np_otp_pdob38_d2a_START (0)
#define SOC_Hi6531V100_OTP_38_R_np_otp_pdob38_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob39_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_39_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_39_R_np_otp_pdob39_d2a_START (0)
#define SOC_Hi6531V100_OTP_39_R_np_otp_pdob39_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob40_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_40_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_40_R_np_otp_pdob40_d2a_START (0)
#define SOC_Hi6531V100_OTP_40_R_np_otp_pdob40_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob41_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_41_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_41_R_np_otp_pdob41_d2a_START (0)
#define SOC_Hi6531V100_OTP_41_R_np_otp_pdob41_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob42_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_42_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_42_R_np_otp_pdob42_d2a_START (0)
#define SOC_Hi6531V100_OTP_42_R_np_otp_pdob42_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob43_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_43_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_43_R_np_otp_pdob43_d2a_START (0)
#define SOC_Hi6531V100_OTP_43_R_np_otp_pdob43_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob44_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_44_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_44_R_np_otp_pdob44_d2a_START (0)
#define SOC_Hi6531V100_OTP_44_R_np_otp_pdob44_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob45_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_45_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_45_R_np_otp_pdob45_d2a_START (0)
#define SOC_Hi6531V100_OTP_45_R_np_otp_pdob45_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob46_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_46_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_46_R_np_otp_pdob46_d2a_START (0)
#define SOC_Hi6531V100_OTP_46_R_np_otp_pdob46_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob47_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_47_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_47_R_np_otp_pdob47_d2a_START (0)
#define SOC_Hi6531V100_OTP_47_R_np_otp_pdob47_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob48_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_48_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_48_R_np_otp_pdob48_d2a_START (0)
#define SOC_Hi6531V100_OTP_48_R_np_otp_pdob48_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob49_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_49_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_49_R_np_otp_pdob49_d2a_START (0)
#define SOC_Hi6531V100_OTP_49_R_np_otp_pdob49_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob50_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_50_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_50_R_np_otp_pdob50_d2a_START (0)
#define SOC_Hi6531V100_OTP_50_R_np_otp_pdob50_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob51_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_51_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_51_R_np_otp_pdob51_d2a_START (0)
#define SOC_Hi6531V100_OTP_51_R_np_otp_pdob51_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob52_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_52_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_52_R_np_otp_pdob52_d2a_START (0)
#define SOC_Hi6531V100_OTP_52_R_np_otp_pdob52_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob53_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_53_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_53_R_np_otp_pdob53_d2a_START (0)
#define SOC_Hi6531V100_OTP_53_R_np_otp_pdob53_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob54_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_54_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_54_R_np_otp_pdob54_d2a_START (0)
#define SOC_Hi6531V100_OTP_54_R_np_otp_pdob54_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob55_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_55_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_55_R_np_otp_pdob55_d2a_START (0)
#define SOC_Hi6531V100_OTP_55_R_np_otp_pdob55_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob56_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_56_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_56_R_np_otp_pdob56_d2a_START (0)
#define SOC_Hi6531V100_OTP_56_R_np_otp_pdob56_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob57_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_57_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_57_R_np_otp_pdob57_d2a_START (0)
#define SOC_Hi6531V100_OTP_57_R_np_otp_pdob57_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob58_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_58_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_58_R_np_otp_pdob58_d2a_START (0)
#define SOC_Hi6531V100_OTP_58_R_np_otp_pdob58_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob59_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_59_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_59_R_np_otp_pdob59_d2a_START (0)
#define SOC_Hi6531V100_OTP_59_R_np_otp_pdob59_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob60_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_60_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_60_R_np_otp_pdob60_d2a_START (0)
#define SOC_Hi6531V100_OTP_60_R_np_otp_pdob60_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob61_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_61_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_61_R_np_otp_pdob61_d2a_START (0)
#define SOC_Hi6531V100_OTP_61_R_np_otp_pdob61_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob62_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_62_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_62_R_np_otp_pdob62_d2a_START (0)
#define SOC_Hi6531V100_OTP_62_R_np_otp_pdob62_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob63_d2a : 8;
    } reg;
} SOC_Hi6531V100_OTP_63_R_UNION;
#endif
#define SOC_Hi6531V100_OTP_63_R_np_otp_pdob63_d2a_START (0)
#define SOC_Hi6531V100_OTP_63_R_np_otp_pdob63_d2a_END (7)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
