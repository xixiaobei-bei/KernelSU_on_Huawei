#ifndef __SOC_SCHARGER_INTERFACE_H__
#define __SOC_SCHARGER_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_SCHARGER_a_ADDR(base) ((base) + (0x0000))
#define SOC_SCHARGER_b_ADDR(base) ((base) + (0x0000))
#define SOC_SCHARGER_c_ADDR(base) ((base) + (0x0000))
#define SOC_SCHARGER_d_ADDR(base) ((base) + (0x0000))
#define SOC_SCHARGER_e_ADDR(base) ((base) + (0x0000))
#define SOC_SCHARGER_CHIP_VERSION_ADDR(base) ((base) + (0x00))
#define SOC_SCHARGER_STATUS0_ADDR(base) ((base) + (0x01))
#define SOC_SCHARGER_STATUS1_ADDR(base) ((base) + (0x02))
#define SOC_SCHARGER_STATUS2_ADDR(base) ((base) + (0x03))
#define SOC_SCHARGER_STATUS3_ADDR(base) ((base) + (0x04))
#define SOC_SCHARGER_STATUS4_ADDR(base) ((base) + (0x05))
#define SOC_SCHARGER_STATUS5_ADDR(base) ((base) + (0x06))
#define SOC_SCHARGER_IRQ0_ADDR(base) ((base) + (0x07))
#define SOC_SCHARGER_IRQ1_ADDR(base) ((base) + (0x08))
#define SOC_SCHARGER_IRQ2_ADDR(base) ((base) + (0x09))
#define SOC_SCHARGER_IRQ3_ADDR(base) ((base) + (0x0A))
#define SOC_SCHARGER_IRQ4_ADDR(base) ((base) + (0x0B))
#define SOC_SCHARGER_IRQM0_ADDR(base) ((base) + (0x0C))
#define SOC_SCHARGER_IRQM1_ADDR(base) ((base) + (0x0D))
#define SOC_SCHARGER_IRQM2_ADDR(base) ((base) + (0x0E))
#define SOC_SCHARGER_IRQM3_ADDR(base) ((base) + (0x0F))
#define SOC_SCHARGER_IRQM4_ADDR(base) ((base) + (0x10))
#define SOC_SCHARGER_ONFF0_ADDR(base) ((base) + (0x11))
#define SOC_SCHARGER_ONFF1_ADDR(base) ((base) + (0x12))
#define SOC_SCHARGER_REF_REG_ADDR(base) ((base) + (0x13))
#define SOC_SCHARGER_VBAT_OV_SEL_ADDR(base) ((base) + (0x14))
#define SOC_SCHARGER_WLED_REG0_ADDR(base) ((base) + (0x15))
#define SOC_SCHARGER_WLED_REG1_ADDR(base) ((base) + (0x16))
#define SOC_SCHARGER_WLED_REG2_ADDR(base) ((base) + (0x17))
#define SOC_SCHARGER_WLED_REG3_ADDR(base) ((base) + (0x18))
#define SOC_SCHARGER_WLED_REG4_ADDR(base) ((base) + (0x19))
#define SOC_SCHARGER_WLED_REG5_ADDR(base) ((base) + (0x1A))
#define SOC_SCHARGER_WLED_REG6_ADDR(base) ((base) + (0x1B))
#define SOC_SCHARGER_WLED_REG7_ADDR(base) ((base) + (0x1C))
#define SOC_SCHARGER_WLED_REG8_ADDR(base) ((base) + (0x1D))
#define SOC_SCHARGER_WLED_REG9_ADDR(base) ((base) + (0x1E))
#define SOC_SCHARGER_LCD_BOOST_REG0_ADDR(base) ((base) + (0x1F))
#define SOC_SCHARGER_LCD_BOOST_REG1_ADDR(base) ((base) + (0x20))
#define SOC_SCHARGER_LCD_BOOST_REG2_ADDR(base) ((base) + (0x21))
#define SOC_SCHARGER_LCD_BOOST_REG3_ADDR(base) ((base) + (0x22))
#define SOC_SCHARGER_LCD_BOOST_REG4_ADDR(base) ((base) + (0x23))
#define SOC_SCHARGER_LCD_BOOST_REG5_ADDR(base) ((base) + (0x24))
#define SOC_SCHARGER_LCD_BOOST_REG6_ADDR(base) ((base) + (0x25))
#define SOC_SCHARGER_LCD_BOOST_REG7_ADDR(base) ((base) + (0x26))
#define SOC_SCHARGER_LCD_BOOST_REG8_ADDR(base) ((base) + (0x27))
#define SOC_SCHARGER_LCD_BOOST_REG9_ADDR(base) ((base) + (0x28))
#define SOC_SCHARGER_LCD_BOOST_REG10_ADDR(base) ((base) + (0x29))
#define SOC_SCHARGER_LCD_BOOST_REG11_ADDR(base) ((base) + (0x2A))
#define SOC_SCHARGER_LCD_BOOST_REG12_ADDR(base) ((base) + (0x2B))
#define SOC_SCHARGER_LCD_BOOST_REG13_ADDR(base) ((base) + (0x2C))
#define SOC_SCHARGER_LCD_BOOST_REG14_ADDR(base) ((base) + (0x2D))
#define SOC_SCHARGER_LCD_NCP_REG0_ADDR(base) ((base) + (0x2E))
#define SOC_SCHARGER_LCD_NCP_REG1_ADDR(base) ((base) + (0x2F))
#define SOC_SCHARGER_LCD_NCP_REG2_ADDR(base) ((base) + (0x30))
#define SOC_SCHARGER_LCD_NCP_REG3_ADDR(base) ((base) + (0x31))
#define SOC_SCHARGER_LCD_NCP_REG4_ADDR(base) ((base) + (0x32))
#define SOC_SCHARGER_LCD_NCP_REG5_ADDR(base) ((base) + (0x33))
#define SOC_SCHARGER_LCD_LDO_REG0_ADDR(base) ((base) + (0x34))
#define SOC_SCHARGER_LDO_REG0_ADDR(base) ((base) + (0x35))
#define SOC_SCHARGER_BUCK_REG0_ADDR(base) ((base) + (0x36))
#define SOC_SCHARGER_BUCK_REG1_ADDR(base) ((base) + (0x37))
#define SOC_SCHARGER_BUCK_REG2_ADDR(base) ((base) + (0x38))
#define SOC_SCHARGER_BUCK_REG3_ADDR(base) ((base) + (0x39))
#define SOC_SCHARGER_BUCK_REG4_ADDR(base) ((base) + (0x3A))
#define SOC_SCHARGER_BUCK_REG5_ADDR(base) ((base) + (0x3B))
#define SOC_SCHARGER_BUCK_REG6_ADDR(base) ((base) + (0x3C))
#define SOC_SCHARGER_OTG_REG0_ADDR(base) ((base) + (0x3D))
#define SOC_SCHARGER_OTG_REG1_ADDR(base) ((base) + (0x3E))
#define SOC_SCHARGER_OTG_REG2_ADDR(base) ((base) + (0x3F))
#define SOC_SCHARGER_OTG_REG3_ADDR(base) ((base) + (0x40))
#define SOC_SCHARGER_OTG_REG4_ADDR(base) ((base) + (0x41))
#define SOC_SCHARGER_OTG_REG5_ADDR(base) ((base) + (0x42))
#define SOC_SCHARGER_OTG_REG6_ADDR(base) ((base) + (0x43))
#define SOC_SCHARGER_OTG_REG7_ADDR(base) ((base) + (0x44))
#define SOC_SCHARGER_OTG_REG8_ADDR(base) ((base) + (0x45))
#define SOC_SCHARGER_OTG_REG9_ADDR(base) ((base) + (0x46))
#define SOC_SCHARGER_OTG_REG10_ADDR(base) ((base) + (0x47))
#define SOC_SCHARGER_CHG_REG0_ADDR(base) ((base) + (0x48))
#define SOC_SCHARGER_CHG_REG1_ADDR(base) ((base) + (0x49))
#define SOC_SCHARGER_CHG_REG2_ADDR(base) ((base) + (0x4A))
#define SOC_SCHARGER_CHG_REG3_ADDR(base) ((base) + (0x4B))
#define SOC_SCHARGER_CHG_REG4_ADDR(base) ((base) + (0x4C))
#define SOC_SCHARGER_CHG_REG5_ADDR(base) ((base) + (0x4D))
#define SOC_SCHARGER_CHG_REG6_ADDR(base) ((base) + (0x4E))
#define SOC_SCHARGER_CHG_REG7_ADDR(base) ((base) + (0x4F))
#define SOC_SCHARGER_CHG_REG8_ADDR(base) ((base) + (0x50))
#define SOC_SCHARGER_CHG_REG9_ADDR(base) ((base) + (0x51))
#define SOC_SCHARGER_CHG_REG10_ADDR(base) ((base) + (0x52))
#define SOC_SCHARGER_CHG_REG11_ADDR(base) ((base) + (0x53))
#define SOC_SCHARGER_PSEL_REG0_ADDR(base) ((base) + (0x54))
#define SOC_SCHARGER_FLASH_BST_REG0_ADDR(base) ((base) + (0x55))
#define SOC_SCHARGER_FLASH_BST_REG1_ADDR(base) ((base) + (0x56))
#define SOC_SCHARGER_FLASH_BST_REG2_ADDR(base) ((base) + (0x57))
#define SOC_SCHARGER_FLASH_BST_REG3_ADDR(base) ((base) + (0x58))
#define SOC_SCHARGER_FLASH_BST_REG4_ADDR(base) ((base) + (0x59))
#define SOC_SCHARGER_FLASH_BST_REG5_ADDR(base) ((base) + (0x5A))
#define SOC_SCHARGER_FLASH_BST_REG6_ADDR(base) ((base) + (0x5B))
#define SOC_SCHARGER_FLASH_BST_REG7_ADDR(base) ((base) + (0x5C))
#define SOC_SCHARGER_FLASH_BST_REG8_ADDR(base) ((base) + (0x5D))
#define SOC_SCHARGER_FLASH_BST_REG9_ADDR(base) ((base) + (0x5E))
#define SOC_SCHARGER_FLASH_BST_REG10_ADDR(base) ((base) + (0x5F))
#define SOC_SCHARGER_FLASH_BST_REG11_ADDR(base) ((base) + (0x60))
#define SOC_SCHARGER_FLASH_LED_REG0_ADDR(base) ((base) + (0x61))
#define SOC_SCHARGER_FLASH_LED_REG1_ADDR(base) ((base) + (0x62))
#define SOC_SCHARGER_SYS_REG_ADDR(base) ((base) + (0x63))
#define SOC_SCHARGER_FLASH_LED_REG2_ADDR(base) ((base) + (0x64))
#define SOC_SCHARGER_FLASH_LED_REG3_ADDR(base) ((base) + (0x65))
#define SOC_SCHARGER_FLASH_LED_REG4_ADDR(base) ((base) + (0x66))
#define SOC_SCHARGER_FLASH_LED_REG5_ADDR(base) ((base) + (0x67))
#define SOC_SCHARGER_FLASH_LED_REG6_ADDR(base) ((base) + (0x68))
#define SOC_SCHARGER_FLASH_LED_REG7_ADDR(base) ((base) + (0x69))
#define SOC_SCHARGER_CTRL_REG_ADDR(base) ((base) + (0x6A))
#define SOC_SCHARGER_SYS_SET_REG_ADDR(base) ((base) + (0x6B))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned short value;
    struct
    {
        unsigned short f_5 : 12;
        unsigned short f_4 : 1;
        unsigned short f_3 : 1;
        unsigned short f_2 : 1;
        unsigned short f_1 : 1;
    } reg;
} SOC_SCHARGER_a_UNION;
#endif
#define SOC_SCHARGER_a_f_5_START (0)
#define SOC_SCHARGER_a_f_5_END (11)
#define SOC_SCHARGER_a_f_4_START (12)
#define SOC_SCHARGER_a_f_4_END (12)
#define SOC_SCHARGER_a_f_3_START (13)
#define SOC_SCHARGER_a_f_3_END (13)
#define SOC_SCHARGER_a_f_2_START (14)
#define SOC_SCHARGER_a_f_2_END (14)
#define SOC_SCHARGER_a_f_1_START (15)
#define SOC_SCHARGER_a_f_1_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned short value;
    struct
    {
        unsigned short f_10 : 12;
        unsigned short f_9 : 1;
        unsigned short f_8 : 1;
        unsigned short f_7 : 1;
        unsigned short f_6 : 1;
    } reg;
} SOC_SCHARGER_b_UNION;
#endif
#define SOC_SCHARGER_b_f_10_START (0)
#define SOC_SCHARGER_b_f_10_END (11)
#define SOC_SCHARGER_b_f_9_START (12)
#define SOC_SCHARGER_b_f_9_END (12)
#define SOC_SCHARGER_b_f_8_START (13)
#define SOC_SCHARGER_b_f_8_END (13)
#define SOC_SCHARGER_b_f_7_START (14)
#define SOC_SCHARGER_b_f_7_END (14)
#define SOC_SCHARGER_b_f_6_START (15)
#define SOC_SCHARGER_b_f_6_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned short value;
    struct
    {
        unsigned short f_15 : 12;
        unsigned short f_14 : 1;
        unsigned short f_13 : 1;
        unsigned short f_12 : 1;
        unsigned short f_11 : 1;
    } reg;
} SOC_SCHARGER_c_UNION;
#endif
#define SOC_SCHARGER_c_f_15_START (0)
#define SOC_SCHARGER_c_f_15_END (11)
#define SOC_SCHARGER_c_f_14_START (12)
#define SOC_SCHARGER_c_f_14_END (12)
#define SOC_SCHARGER_c_f_13_START (13)
#define SOC_SCHARGER_c_f_13_END (13)
#define SOC_SCHARGER_c_f_12_START (14)
#define SOC_SCHARGER_c_f_12_END (14)
#define SOC_SCHARGER_c_f_11_START (15)
#define SOC_SCHARGER_c_f_11_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned short value;
    struct
    {
        unsigned short f_20 : 12;
        unsigned short f_19 : 1;
        unsigned short f_18 : 1;
        unsigned short f_17 : 1;
        unsigned short f_16 : 1;
    } reg;
} SOC_SCHARGER_d_UNION;
#endif
#define SOC_SCHARGER_d_f_20_START (0)
#define SOC_SCHARGER_d_f_20_END (11)
#define SOC_SCHARGER_d_f_19_START (12)
#define SOC_SCHARGER_d_f_19_END (12)
#define SOC_SCHARGER_d_f_18_START (13)
#define SOC_SCHARGER_d_f_18_END (13)
#define SOC_SCHARGER_d_f_17_START (14)
#define SOC_SCHARGER_d_f_17_END (14)
#define SOC_SCHARGER_d_f_16_START (15)
#define SOC_SCHARGER_d_f_16_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned short value;
    struct
    {
        unsigned short f_25 : 12;
        unsigned short f_24 : 1;
        unsigned short f_23 : 1;
        unsigned short f_22 : 1;
        unsigned short f_21 : 1;
    } reg;
} SOC_SCHARGER_e_UNION;
#endif
#define SOC_SCHARGER_e_f_25_START (0)
#define SOC_SCHARGER_e_f_25_END (11)
#define SOC_SCHARGER_e_f_24_START (12)
#define SOC_SCHARGER_e_f_24_END (12)
#define SOC_SCHARGER_e_f_23_START (13)
#define SOC_SCHARGER_e_f_23_END (13)
#define SOC_SCHARGER_e_f_22_START (14)
#define SOC_SCHARGER_e_f_22_END (14)
#define SOC_SCHARGER_e_f_21_START (15)
#define SOC_SCHARGER_e_f_21_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char version_id_v : 8;
    } reg;
} SOC_SCHARGER_CHIP_VERSION_UNION;
#endif
#define SOC_SCHARGER_CHIP_VERSION_version_id_v_START (0)
#define SOC_SCHARGER_CHIP_VERSION_version_id_v_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char to_happened : 1;
        unsigned char wt_happened : 1;
        unsigned char buck_dpm_stat_2d : 1;
        unsigned char buck_vsys_stat_2d : 1;
        unsigned char wdt_time_out_n : 1;
        unsigned char chg_chgstate : 2;
        unsigned char otg_on : 1;
    } reg;
} SOC_SCHARGER_STATUS0_UNION;
#endif
#define SOC_SCHARGER_STATUS0_to_happened_START (0)
#define SOC_SCHARGER_STATUS0_to_happened_END (0)
#define SOC_SCHARGER_STATUS0_wt_happened_START (1)
#define SOC_SCHARGER_STATUS0_wt_happened_END (1)
#define SOC_SCHARGER_STATUS0_buck_dpm_stat_2d_START (2)
#define SOC_SCHARGER_STATUS0_buck_dpm_stat_2d_END (2)
#define SOC_SCHARGER_STATUS0_buck_vsys_stat_2d_START (3)
#define SOC_SCHARGER_STATUS0_buck_vsys_stat_2d_END (3)
#define SOC_SCHARGER_STATUS0_wdt_time_out_n_START (4)
#define SOC_SCHARGER_STATUS0_wdt_time_out_n_END (4)
#define SOC_SCHARGER_STATUS0_chg_chgstate_START (5)
#define SOC_SCHARGER_STATUS0_chg_chgstate_END (6)
#define SOC_SCHARGER_STATUS0_otg_on_START (7)
#define SOC_SCHARGER_STATUS0_otg_on_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otmp_150_r : 1;
        unsigned char otmp_jstp_r : 1;
        unsigned char otmp_jreg_r : 1;
        unsigned char vbus_ovp_6p5v_r : 1;
        unsigned char vbus_uvp_3p5v_r : 1;
        unsigned char fast_chg_fault_r : 1;
        unsigned char pre_chg_fault_r : 1;
        unsigned char trickle_chg_fault_r : 1;
    } reg;
} SOC_SCHARGER_STATUS1_UNION;
#endif
#define SOC_SCHARGER_STATUS1_otmp_150_r_START (0)
#define SOC_SCHARGER_STATUS1_otmp_150_r_END (0)
#define SOC_SCHARGER_STATUS1_otmp_jstp_r_START (1)
#define SOC_SCHARGER_STATUS1_otmp_jstp_r_END (1)
#define SOC_SCHARGER_STATUS1_otmp_jreg_r_START (2)
#define SOC_SCHARGER_STATUS1_otmp_jreg_r_END (2)
#define SOC_SCHARGER_STATUS1_vbus_ovp_6p5v_r_START (3)
#define SOC_SCHARGER_STATUS1_vbus_ovp_6p5v_r_END (3)
#define SOC_SCHARGER_STATUS1_vbus_uvp_3p5v_r_START (4)
#define SOC_SCHARGER_STATUS1_vbus_uvp_3p5v_r_END (4)
#define SOC_SCHARGER_STATUS1_fast_chg_fault_r_START (5)
#define SOC_SCHARGER_STATUS1_fast_chg_fault_r_END (5)
#define SOC_SCHARGER_STATUS1_pre_chg_fault_r_START (6)
#define SOC_SCHARGER_STATUS1_pre_chg_fault_r_END (6)
#define SOC_SCHARGER_STATUS1_trickle_chg_fault_r_START (7)
#define SOC_SCHARGER_STATUS1_trickle_chg_fault_r_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char wled_scp_r : 1;
        unsigned char wled_ocp_r : 1;
        unsigned char wled_uvp_r : 1;
        unsigned char wled_ovp_4p5v_r : 1;
        unsigned char wled_ovp_38v_r : 1;
        unsigned char buck_scp_r : 1;
        unsigned char chg_rechg_r : 1;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_STATUS2_UNION;
#endif
#define SOC_SCHARGER_STATUS2_wled_scp_r_START (0)
#define SOC_SCHARGER_STATUS2_wled_scp_r_END (0)
#define SOC_SCHARGER_STATUS2_wled_ocp_r_START (1)
#define SOC_SCHARGER_STATUS2_wled_ocp_r_END (1)
#define SOC_SCHARGER_STATUS2_wled_uvp_r_START (2)
#define SOC_SCHARGER_STATUS2_wled_uvp_r_END (2)
#define SOC_SCHARGER_STATUS2_wled_ovp_4p5v_r_START (3)
#define SOC_SCHARGER_STATUS2_wled_ovp_4p5v_r_END (3)
#define SOC_SCHARGER_STATUS2_wled_ovp_38v_r_START (4)
#define SOC_SCHARGER_STATUS2_wled_ovp_38v_r_END (4)
#define SOC_SCHARGER_STATUS2_buck_scp_r_START (5)
#define SOC_SCHARGER_STATUS2_buck_scp_r_END (5)
#define SOC_SCHARGER_STATUS2_chg_rechg_r_START (6)
#define SOC_SCHARGER_STATUS2_chg_rechg_r_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_bst_uvp_r : 1;
        unsigned char flash_bst_ovp_r : 1;
        unsigned char flash_bst_scp_r : 1;
        unsigned char reserved_0 : 1;
        unsigned char flash_led_open_r : 1;
        unsigned char flash_led_short_r : 1;
        unsigned char reserved_1 : 2;
    } reg;
} SOC_SCHARGER_STATUS3_UNION;
#endif
#define SOC_SCHARGER_STATUS3_flash_bst_uvp_r_START (0)
#define SOC_SCHARGER_STATUS3_flash_bst_uvp_r_END (0)
#define SOC_SCHARGER_STATUS3_flash_bst_ovp_r_START (1)
#define SOC_SCHARGER_STATUS3_flash_bst_ovp_r_END (1)
#define SOC_SCHARGER_STATUS3_flash_bst_scp_r_START (2)
#define SOC_SCHARGER_STATUS3_flash_bst_scp_r_END (2)
#define SOC_SCHARGER_STATUS3_flash_led_open_r_START (4)
#define SOC_SCHARGER_STATUS3_flash_led_open_r_END (4)
#define SOC_SCHARGER_STATUS3_flash_led_short_r_START (5)
#define SOC_SCHARGER_STATUS3_flash_led_short_r_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo2_200ma_ocp_r : 1;
        unsigned char ldo1_200ma_ocp_r : 1;
        unsigned char lcd_ldo_ocp_r : 1;
        unsigned char lcd_ncp_scp_r : 1;
        unsigned char lcd_bst_uvp_r : 1;
        unsigned char lcd_bst_ovp_r : 1;
        unsigned char lcd_bst_scp_r : 1;
        unsigned char lcd_bst_ocp_r : 1;
    } reg;
} SOC_SCHARGER_STATUS4_UNION;
#endif
#define SOC_SCHARGER_STATUS4_ldo2_200ma_ocp_r_START (0)
#define SOC_SCHARGER_STATUS4_ldo2_200ma_ocp_r_END (0)
#define SOC_SCHARGER_STATUS4_ldo1_200ma_ocp_r_START (1)
#define SOC_SCHARGER_STATUS4_ldo1_200ma_ocp_r_END (1)
#define SOC_SCHARGER_STATUS4_lcd_ldo_ocp_r_START (2)
#define SOC_SCHARGER_STATUS4_lcd_ldo_ocp_r_END (2)
#define SOC_SCHARGER_STATUS4_lcd_ncp_scp_r_START (3)
#define SOC_SCHARGER_STATUS4_lcd_ncp_scp_r_END (3)
#define SOC_SCHARGER_STATUS4_lcd_bst_uvp_r_START (4)
#define SOC_SCHARGER_STATUS4_lcd_bst_uvp_r_END (4)
#define SOC_SCHARGER_STATUS4_lcd_bst_ovp_r_START (5)
#define SOC_SCHARGER_STATUS4_lcd_bst_ovp_r_END (5)
#define SOC_SCHARGER_STATUS4_lcd_bst_scp_r_START (6)
#define SOC_SCHARGER_STATUS4_lcd_bst_scp_r_END (6)
#define SOC_SCHARGER_STATUS4_lcd_bst_ocp_r_START (7)
#define SOC_SCHARGER_STATUS4_lcd_bst_ocp_r_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otg_ovp_5p3v_r : 1;
        unsigned char otg_uvp_r : 1;
        unsigned char otg_ocp_r : 1;
        unsigned char otg_scp_r : 1;
        unsigned char bat_ltmp_r : 1;
        unsigned char bat_htmp_r : 1;
        unsigned char vbat_ovp_r : 1;
        unsigned char otg_vbat_ok : 1;
    } reg;
} SOC_SCHARGER_STATUS5_UNION;
#endif
#define SOC_SCHARGER_STATUS5_otg_ovp_5p3v_r_START (0)
#define SOC_SCHARGER_STATUS5_otg_ovp_5p3v_r_END (0)
#define SOC_SCHARGER_STATUS5_otg_uvp_r_START (1)
#define SOC_SCHARGER_STATUS5_otg_uvp_r_END (1)
#define SOC_SCHARGER_STATUS5_otg_ocp_r_START (2)
#define SOC_SCHARGER_STATUS5_otg_ocp_r_END (2)
#define SOC_SCHARGER_STATUS5_otg_scp_r_START (3)
#define SOC_SCHARGER_STATUS5_otg_scp_r_END (3)
#define SOC_SCHARGER_STATUS5_bat_ltmp_r_START (4)
#define SOC_SCHARGER_STATUS5_bat_ltmp_r_END (4)
#define SOC_SCHARGER_STATUS5_bat_htmp_r_START (5)
#define SOC_SCHARGER_STATUS5_bat_htmp_r_END (5)
#define SOC_SCHARGER_STATUS5_vbat_ovp_r_START (6)
#define SOC_SCHARGER_STATUS5_vbat_ovp_r_END (6)
#define SOC_SCHARGER_STATUS5_otg_vbat_ok_START (7)
#define SOC_SCHARGER_STATUS5_otg_vbat_ok_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otmp_150_r : 1;
        unsigned char otmp_jstp_r : 1;
        unsigned char otmp_jreg_r : 1;
        unsigned char vbus_ovp_6p5v_r : 1;
        unsigned char vbus_uvp_3p5v_r : 1;
        unsigned char fast_chg_fault_r : 1;
        unsigned char pre_chg_fault_r : 1;
        unsigned char trickle_chg_fault_r : 1;
    } reg;
} SOC_SCHARGER_IRQ0_UNION;
#endif
#define SOC_SCHARGER_IRQ0_otmp_150_r_START (0)
#define SOC_SCHARGER_IRQ0_otmp_150_r_END (0)
#define SOC_SCHARGER_IRQ0_otmp_jstp_r_START (1)
#define SOC_SCHARGER_IRQ0_otmp_jstp_r_END (1)
#define SOC_SCHARGER_IRQ0_otmp_jreg_r_START (2)
#define SOC_SCHARGER_IRQ0_otmp_jreg_r_END (2)
#define SOC_SCHARGER_IRQ0_vbus_ovp_6p5v_r_START (3)
#define SOC_SCHARGER_IRQ0_vbus_ovp_6p5v_r_END (3)
#define SOC_SCHARGER_IRQ0_vbus_uvp_3p5v_r_START (4)
#define SOC_SCHARGER_IRQ0_vbus_uvp_3p5v_r_END (4)
#define SOC_SCHARGER_IRQ0_fast_chg_fault_r_START (5)
#define SOC_SCHARGER_IRQ0_fast_chg_fault_r_END (5)
#define SOC_SCHARGER_IRQ0_pre_chg_fault_r_START (6)
#define SOC_SCHARGER_IRQ0_pre_chg_fault_r_END (6)
#define SOC_SCHARGER_IRQ0_trickle_chg_fault_r_START (7)
#define SOC_SCHARGER_IRQ0_trickle_chg_fault_r_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char wled_scp_r : 1;
        unsigned char wled_ocp_r : 1;
        unsigned char wled_uvp_r : 1;
        unsigned char wled_ovp_4p5v_r : 1;
        unsigned char wled_ovp_38v_r : 1;
        unsigned char buck_scp_r : 1;
        unsigned char chg_rechg_r : 1;
        unsigned char chg_charge_done_r : 1;
    } reg;
} SOC_SCHARGER_IRQ1_UNION;
#endif
#define SOC_SCHARGER_IRQ1_wled_scp_r_START (0)
#define SOC_SCHARGER_IRQ1_wled_scp_r_END (0)
#define SOC_SCHARGER_IRQ1_wled_ocp_r_START (1)
#define SOC_SCHARGER_IRQ1_wled_ocp_r_END (1)
#define SOC_SCHARGER_IRQ1_wled_uvp_r_START (2)
#define SOC_SCHARGER_IRQ1_wled_uvp_r_END (2)
#define SOC_SCHARGER_IRQ1_wled_ovp_4p5v_r_START (3)
#define SOC_SCHARGER_IRQ1_wled_ovp_4p5v_r_END (3)
#define SOC_SCHARGER_IRQ1_wled_ovp_38v_r_START (4)
#define SOC_SCHARGER_IRQ1_wled_ovp_38v_r_END (4)
#define SOC_SCHARGER_IRQ1_buck_scp_r_START (5)
#define SOC_SCHARGER_IRQ1_buck_scp_r_END (5)
#define SOC_SCHARGER_IRQ1_chg_rechg_r_START (6)
#define SOC_SCHARGER_IRQ1_chg_rechg_r_END (6)
#define SOC_SCHARGER_IRQ1_chg_charge_done_r_START (7)
#define SOC_SCHARGER_IRQ1_chg_charge_done_r_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_bst_uvp_r : 1;
        unsigned char flash_bst_ovp_r : 1;
        unsigned char flash_bst_scp_r : 1;
        unsigned char reserved_0 : 1;
        unsigned char flash_led_open_r : 1;
        unsigned char flash_led_short_r : 1;
        unsigned char reserved_1 : 2;
    } reg;
} SOC_SCHARGER_IRQ2_UNION;
#endif
#define SOC_SCHARGER_IRQ2_flash_bst_uvp_r_START (0)
#define SOC_SCHARGER_IRQ2_flash_bst_uvp_r_END (0)
#define SOC_SCHARGER_IRQ2_flash_bst_ovp_r_START (1)
#define SOC_SCHARGER_IRQ2_flash_bst_ovp_r_END (1)
#define SOC_SCHARGER_IRQ2_flash_bst_scp_r_START (2)
#define SOC_SCHARGER_IRQ2_flash_bst_scp_r_END (2)
#define SOC_SCHARGER_IRQ2_flash_led_open_r_START (4)
#define SOC_SCHARGER_IRQ2_flash_led_open_r_END (4)
#define SOC_SCHARGER_IRQ2_flash_led_short_r_START (5)
#define SOC_SCHARGER_IRQ2_flash_led_short_r_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo2_200ma_ocp_r : 1;
        unsigned char ldo1_200ma_ocp_r : 1;
        unsigned char lcd_ldo_ocp_r : 1;
        unsigned char lcd_ncp_scp_r : 1;
        unsigned char lcd_bst_uvp_r : 1;
        unsigned char lcd_bst_ovp_r : 1;
        unsigned char lcd_bst_scp_r : 1;
        unsigned char lcd_bst_ocp_r : 1;
    } reg;
} SOC_SCHARGER_IRQ3_UNION;
#endif
#define SOC_SCHARGER_IRQ3_ldo2_200ma_ocp_r_START (0)
#define SOC_SCHARGER_IRQ3_ldo2_200ma_ocp_r_END (0)
#define SOC_SCHARGER_IRQ3_ldo1_200ma_ocp_r_START (1)
#define SOC_SCHARGER_IRQ3_ldo1_200ma_ocp_r_END (1)
#define SOC_SCHARGER_IRQ3_lcd_ldo_ocp_r_START (2)
#define SOC_SCHARGER_IRQ3_lcd_ldo_ocp_r_END (2)
#define SOC_SCHARGER_IRQ3_lcd_ncp_scp_r_START (3)
#define SOC_SCHARGER_IRQ3_lcd_ncp_scp_r_END (3)
#define SOC_SCHARGER_IRQ3_lcd_bst_uvp_r_START (4)
#define SOC_SCHARGER_IRQ3_lcd_bst_uvp_r_END (4)
#define SOC_SCHARGER_IRQ3_lcd_bst_ovp_r_START (5)
#define SOC_SCHARGER_IRQ3_lcd_bst_ovp_r_END (5)
#define SOC_SCHARGER_IRQ3_lcd_bst_scp_r_START (6)
#define SOC_SCHARGER_IRQ3_lcd_bst_scp_r_END (6)
#define SOC_SCHARGER_IRQ3_lcd_bst_ocp_r_START (7)
#define SOC_SCHARGER_IRQ3_lcd_bst_ocp_r_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otg_ovp_5p3v_r : 1;
        unsigned char otg_uvp_r : 1;
        unsigned char otg_ocp_r : 1;
        unsigned char otg_scp_r : 1;
        unsigned char bat_ltmp_r : 1;
        unsigned char bat_htmp_r : 1;
        unsigned char vbat_ovp_r : 1;
        unsigned char otg_vbat_ok_f : 1;
    } reg;
} SOC_SCHARGER_IRQ4_UNION;
#endif
#define SOC_SCHARGER_IRQ4_otg_ovp_5p3v_r_START (0)
#define SOC_SCHARGER_IRQ4_otg_ovp_5p3v_r_END (0)
#define SOC_SCHARGER_IRQ4_otg_uvp_r_START (1)
#define SOC_SCHARGER_IRQ4_otg_uvp_r_END (1)
#define SOC_SCHARGER_IRQ4_otg_ocp_r_START (2)
#define SOC_SCHARGER_IRQ4_otg_ocp_r_END (2)
#define SOC_SCHARGER_IRQ4_otg_scp_r_START (3)
#define SOC_SCHARGER_IRQ4_otg_scp_r_END (3)
#define SOC_SCHARGER_IRQ4_bat_ltmp_r_START (4)
#define SOC_SCHARGER_IRQ4_bat_ltmp_r_END (4)
#define SOC_SCHARGER_IRQ4_bat_htmp_r_START (5)
#define SOC_SCHARGER_IRQ4_bat_htmp_r_END (5)
#define SOC_SCHARGER_IRQ4_vbat_ovp_r_START (6)
#define SOC_SCHARGER_IRQ4_vbat_ovp_r_END (6)
#define SOC_SCHARGER_IRQ4_otg_vbat_ok_f_START (7)
#define SOC_SCHARGER_IRQ4_otg_vbat_ok_f_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otmp_150_rm : 1;
        unsigned char otmp_jstp_rm : 1;
        unsigned char otmp_jreg_rm : 1;
        unsigned char vbus_ovp_6p5v_rm : 1;
        unsigned char vbus_uvp_3p5v_rm : 1;
        unsigned char fast_chg_fault_rm : 1;
        unsigned char pre_chg_fault_rm : 1;
        unsigned char trickle_chg_fault_rm : 1;
    } reg;
} SOC_SCHARGER_IRQM0_UNION;
#endif
#define SOC_SCHARGER_IRQM0_otmp_150_rm_START (0)
#define SOC_SCHARGER_IRQM0_otmp_150_rm_END (0)
#define SOC_SCHARGER_IRQM0_otmp_jstp_rm_START (1)
#define SOC_SCHARGER_IRQM0_otmp_jstp_rm_END (1)
#define SOC_SCHARGER_IRQM0_otmp_jreg_rm_START (2)
#define SOC_SCHARGER_IRQM0_otmp_jreg_rm_END (2)
#define SOC_SCHARGER_IRQM0_vbus_ovp_6p5v_rm_START (3)
#define SOC_SCHARGER_IRQM0_vbus_ovp_6p5v_rm_END (3)
#define SOC_SCHARGER_IRQM0_vbus_uvp_3p5v_rm_START (4)
#define SOC_SCHARGER_IRQM0_vbus_uvp_3p5v_rm_END (4)
#define SOC_SCHARGER_IRQM0_fast_chg_fault_rm_START (5)
#define SOC_SCHARGER_IRQM0_fast_chg_fault_rm_END (5)
#define SOC_SCHARGER_IRQM0_pre_chg_fault_rm_START (6)
#define SOC_SCHARGER_IRQM0_pre_chg_fault_rm_END (6)
#define SOC_SCHARGER_IRQM0_trickle_chg_fault_rm_START (7)
#define SOC_SCHARGER_IRQM0_trickle_chg_fault_rm_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char wled_scp_rm : 1;
        unsigned char wled_ocp_rm : 1;
        unsigned char wled_uvp_rm : 1;
        unsigned char wled_ovp_4p5v_rm : 1;
        unsigned char wled_ovp_38v_rm : 1;
        unsigned char buck_scp_rm : 1;
        unsigned char chg_rechg_rm : 1;
        unsigned char chg_charge_done_rm : 1;
    } reg;
} SOC_SCHARGER_IRQM1_UNION;
#endif
#define SOC_SCHARGER_IRQM1_wled_scp_rm_START (0)
#define SOC_SCHARGER_IRQM1_wled_scp_rm_END (0)
#define SOC_SCHARGER_IRQM1_wled_ocp_rm_START (1)
#define SOC_SCHARGER_IRQM1_wled_ocp_rm_END (1)
#define SOC_SCHARGER_IRQM1_wled_uvp_rm_START (2)
#define SOC_SCHARGER_IRQM1_wled_uvp_rm_END (2)
#define SOC_SCHARGER_IRQM1_wled_ovp_4p5v_rm_START (3)
#define SOC_SCHARGER_IRQM1_wled_ovp_4p5v_rm_END (3)
#define SOC_SCHARGER_IRQM1_wled_ovp_38v_rm_START (4)
#define SOC_SCHARGER_IRQM1_wled_ovp_38v_rm_END (4)
#define SOC_SCHARGER_IRQM1_buck_scp_rm_START (5)
#define SOC_SCHARGER_IRQM1_buck_scp_rm_END (5)
#define SOC_SCHARGER_IRQM1_chg_rechg_rm_START (6)
#define SOC_SCHARGER_IRQM1_chg_rechg_rm_END (6)
#define SOC_SCHARGER_IRQM1_chg_charge_done_rm_START (7)
#define SOC_SCHARGER_IRQM1_chg_charge_done_rm_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_bst_uvp_rm : 1;
        unsigned char flash_bst_ovp_rm : 1;
        unsigned char flash_bst_scp_rm : 1;
        unsigned char reserved_0 : 1;
        unsigned char flash_led_open_rm : 1;
        unsigned char flash_led_short_rm : 1;
        unsigned char reserved_1 : 2;
    } reg;
} SOC_SCHARGER_IRQM2_UNION;
#endif
#define SOC_SCHARGER_IRQM2_flash_bst_uvp_rm_START (0)
#define SOC_SCHARGER_IRQM2_flash_bst_uvp_rm_END (0)
#define SOC_SCHARGER_IRQM2_flash_bst_ovp_rm_START (1)
#define SOC_SCHARGER_IRQM2_flash_bst_ovp_rm_END (1)
#define SOC_SCHARGER_IRQM2_flash_bst_scp_rm_START (2)
#define SOC_SCHARGER_IRQM2_flash_bst_scp_rm_END (2)
#define SOC_SCHARGER_IRQM2_flash_led_open_rm_START (4)
#define SOC_SCHARGER_IRQM2_flash_led_open_rm_END (4)
#define SOC_SCHARGER_IRQM2_flash_led_short_rm_START (5)
#define SOC_SCHARGER_IRQM2_flash_led_short_rm_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo2_200ma_ocp_rm : 1;
        unsigned char ldo1_200ma_ocp_rm : 1;
        unsigned char lcd_ldo_ocp_rm : 1;
        unsigned char lcd_ncp_scp_rm : 1;
        unsigned char lcd_bst_uvp_rm : 1;
        unsigned char lcd_bst_ovp_rm : 1;
        unsigned char lcd_bst_scp_rm : 1;
        unsigned char lcd_bst_ocp_rm : 1;
    } reg;
} SOC_SCHARGER_IRQM3_UNION;
#endif
#define SOC_SCHARGER_IRQM3_ldo2_200ma_ocp_rm_START (0)
#define SOC_SCHARGER_IRQM3_ldo2_200ma_ocp_rm_END (0)
#define SOC_SCHARGER_IRQM3_ldo1_200ma_ocp_rm_START (1)
#define SOC_SCHARGER_IRQM3_ldo1_200ma_ocp_rm_END (1)
#define SOC_SCHARGER_IRQM3_lcd_ldo_ocp_rm_START (2)
#define SOC_SCHARGER_IRQM3_lcd_ldo_ocp_rm_END (2)
#define SOC_SCHARGER_IRQM3_lcd_ncp_scp_rm_START (3)
#define SOC_SCHARGER_IRQM3_lcd_ncp_scp_rm_END (3)
#define SOC_SCHARGER_IRQM3_lcd_bst_uvp_rm_START (4)
#define SOC_SCHARGER_IRQM3_lcd_bst_uvp_rm_END (4)
#define SOC_SCHARGER_IRQM3_lcd_bst_ovp_rm_START (5)
#define SOC_SCHARGER_IRQM3_lcd_bst_ovp_rm_END (5)
#define SOC_SCHARGER_IRQM3_lcd_bst_scp_rm_START (6)
#define SOC_SCHARGER_IRQM3_lcd_bst_scp_rm_END (6)
#define SOC_SCHARGER_IRQM3_lcd_bst_ocp_rm_START (7)
#define SOC_SCHARGER_IRQM3_lcd_bst_ocp_rm_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otg_ovp_5p3v_rm : 1;
        unsigned char otg_uvp_rm : 1;
        unsigned char otg_ocp_rm : 1;
        unsigned char otg_scp_rm : 1;
        unsigned char bat_ltmp_rm : 1;
        unsigned char bat_htmp_rm : 1;
        unsigned char vbat_ovp_rm : 1;
        unsigned char otg_vbat_ok_fm : 1;
    } reg;
} SOC_SCHARGER_IRQM4_UNION;
#endif
#define SOC_SCHARGER_IRQM4_otg_ovp_5p3v_rm_START (0)
#define SOC_SCHARGER_IRQM4_otg_ovp_5p3v_rm_END (0)
#define SOC_SCHARGER_IRQM4_otg_uvp_rm_START (1)
#define SOC_SCHARGER_IRQM4_otg_uvp_rm_END (1)
#define SOC_SCHARGER_IRQM4_otg_ocp_rm_START (2)
#define SOC_SCHARGER_IRQM4_otg_ocp_rm_END (2)
#define SOC_SCHARGER_IRQM4_otg_scp_rm_START (3)
#define SOC_SCHARGER_IRQM4_otg_scp_rm_END (3)
#define SOC_SCHARGER_IRQM4_bat_ltmp_rm_START (4)
#define SOC_SCHARGER_IRQM4_bat_ltmp_rm_END (4)
#define SOC_SCHARGER_IRQM4_bat_htmp_rm_START (5)
#define SOC_SCHARGER_IRQM4_bat_htmp_rm_END (5)
#define SOC_SCHARGER_IRQM4_vbat_ovp_rm_START (6)
#define SOC_SCHARGER_IRQM4_vbat_ovp_rm_END (6)
#define SOC_SCHARGER_IRQM4_otg_vbat_ok_fm_START (7)
#define SOC_SCHARGER_IRQM4_otg_vbat_ok_fm_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_ncp_en_int : 1;
        unsigned char lcd_ldo_en_int : 1;
        unsigned char lcd_bst_en_int : 1;
        unsigned char flash_bst_en_int : 1;
        unsigned char flash_led_flash_en : 1;
        unsigned char flash_led_torch_en : 1;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_ONFF0_UNION;
#endif
#define SOC_SCHARGER_ONFF0_lcd_ncp_en_int_START (0)
#define SOC_SCHARGER_ONFF0_lcd_ncp_en_int_END (0)
#define SOC_SCHARGER_ONFF0_lcd_ldo_en_int_START (1)
#define SOC_SCHARGER_ONFF0_lcd_ldo_en_int_END (1)
#define SOC_SCHARGER_ONFF0_lcd_bst_en_int_START (2)
#define SOC_SCHARGER_ONFF0_lcd_bst_en_int_END (2)
#define SOC_SCHARGER_ONFF0_flash_bst_en_int_START (3)
#define SOC_SCHARGER_ONFF0_flash_bst_en_int_END (3)
#define SOC_SCHARGER_ONFF0_flash_led_flash_en_START (4)
#define SOC_SCHARGER_ONFF0_flash_led_flash_en_END (4)
#define SOC_SCHARGER_ONFF0_flash_led_torch_en_START (5)
#define SOC_SCHARGER_ONFF0_flash_led_torch_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo2_200ma_en_int : 1;
        unsigned char ldo1_200ma_en_int : 1;
        unsigned char wled_en_int : 1;
        unsigned char otg_en_int : 1;
        unsigned char chg_en_int : 1;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_ONFF1_UNION;
#endif
#define SOC_SCHARGER_ONFF1_ldo2_200ma_en_int_START (0)
#define SOC_SCHARGER_ONFF1_ldo2_200ma_en_int_END (0)
#define SOC_SCHARGER_ONFF1_ldo1_200ma_en_int_START (1)
#define SOC_SCHARGER_ONFF1_ldo1_200ma_en_int_END (1)
#define SOC_SCHARGER_ONFF1_wled_en_int_START (2)
#define SOC_SCHARGER_ONFF1_wled_en_int_END (2)
#define SOC_SCHARGER_ONFF1_otg_en_int_START (3)
#define SOC_SCHARGER_ONFF1_otg_en_int_END (3)
#define SOC_SCHARGER_ONFF1_chg_en_int_START (4)
#define SOC_SCHARGER_ONFF1_chg_en_int_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char thsd_tstp_set : 1;
        unsigned char thsd_treg_set : 2;
        unsigned char reserved : 5;
    } reg;
} SOC_SCHARGER_REF_REG_UNION;
#endif
#define SOC_SCHARGER_REF_REG_thsd_tstp_set_START (0)
#define SOC_SCHARGER_REF_REG_thsd_tstp_set_END (0)
#define SOC_SCHARGER_REF_REG_thsd_treg_set_START (1)
#define SOC_SCHARGER_REF_REG_thsd_treg_set_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vbat_ov_sel : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_VBAT_OV_SEL_UNION;
#endif
#define SOC_SCHARGER_VBAT_OV_SEL_vbat_ov_sel_START (0)
#define SOC_SCHARGER_VBAT_OV_SEL_vbat_ov_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char wled_bst_ovp_en : 1;
        unsigned char wled_bst_en_pfm : 1;
        unsigned char wled_bst_en_clp : 1;
        unsigned char wled_bst_slop : 1;
        unsigned char wled_bst_ocp_en : 1;
        unsigned char wled_bst_scp : 1;
        unsigned char wled_ifb_uvp_sel : 2;
    } reg;
} SOC_SCHARGER_WLED_REG0_UNION;
#endif
#define SOC_SCHARGER_WLED_REG0_wled_bst_ovp_en_START (0)
#define SOC_SCHARGER_WLED_REG0_wled_bst_ovp_en_END (0)
#define SOC_SCHARGER_WLED_REG0_wled_bst_en_pfm_START (1)
#define SOC_SCHARGER_WLED_REG0_wled_bst_en_pfm_END (1)
#define SOC_SCHARGER_WLED_REG0_wled_bst_en_clp_START (2)
#define SOC_SCHARGER_WLED_REG0_wled_bst_en_clp_END (2)
#define SOC_SCHARGER_WLED_REG0_wled_bst_slop_START (3)
#define SOC_SCHARGER_WLED_REG0_wled_bst_slop_END (3)
#define SOC_SCHARGER_WLED_REG0_wled_bst_ocp_en_START (4)
#define SOC_SCHARGER_WLED_REG0_wled_bst_ocp_en_END (4)
#define SOC_SCHARGER_WLED_REG0_wled_bst_scp_START (5)
#define SOC_SCHARGER_WLED_REG0_wled_bst_scp_END (5)
#define SOC_SCHARGER_WLED_REG0_wled_ifb_uvp_sel_START (6)
#define SOC_SCHARGER_WLED_REG0_wled_ifb_uvp_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char wled_bst_clp_l : 2;
        unsigned char wled_bst_gm : 3;
        unsigned char wled_bst_lmos_ocp : 2;
        unsigned char wled_bst_ocp : 1;
    } reg;
} SOC_SCHARGER_WLED_REG1_UNION;
#endif
#define SOC_SCHARGER_WLED_REG1_wled_bst_clp_l_START (0)
#define SOC_SCHARGER_WLED_REG1_wled_bst_clp_l_END (1)
#define SOC_SCHARGER_WLED_REG1_wled_bst_gm_START (2)
#define SOC_SCHARGER_WLED_REG1_wled_bst_gm_END (4)
#define SOC_SCHARGER_WLED_REG1_wled_bst_lmos_ocp_START (5)
#define SOC_SCHARGER_WLED_REG1_wled_bst_lmos_ocp_END (6)
#define SOC_SCHARGER_WLED_REG1_wled_bst_ocp_START (7)
#define SOC_SCHARGER_WLED_REG1_wled_bst_ocp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char wled_bst_drv : 6;
        unsigned char wled_bst_scp_sel : 2;
    } reg;
} SOC_SCHARGER_WLED_REG2_UNION;
#endif
#define SOC_SCHARGER_WLED_REG2_wled_bst_drv_START (0)
#define SOC_SCHARGER_WLED_REG2_wled_bst_drv_END (5)
#define SOC_SCHARGER_WLED_REG2_wled_bst_scp_sel_START (6)
#define SOC_SCHARGER_WLED_REG2_wled_bst_scp_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char wled_bst_osc : 8;
    } reg;
} SOC_SCHARGER_WLED_REG3_UNION;
#endif
#define SOC_SCHARGER_WLED_REG3_wled_bst_osc_START (0)
#define SOC_SCHARGER_WLED_REG3_wled_bst_osc_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char wled_ifb_prot_en : 4;
        unsigned char wled_bst_rf : 2;
        unsigned char wled_bst_ckmin : 2;
    } reg;
} SOC_SCHARGER_WLED_REG4_UNION;
#endif
#define SOC_SCHARGER_WLED_REG4_wled_ifb_prot_en_START (0)
#define SOC_SCHARGER_WLED_REG4_wled_ifb_prot_en_END (3)
#define SOC_SCHARGER_WLED_REG4_wled_bst_rf_START (4)
#define SOC_SCHARGER_WLED_REG4_wled_bst_rf_END (5)
#define SOC_SCHARGER_WLED_REG4_wled_bst_ckmin_START (6)
#define SOC_SCHARGER_WLED_REG4_wled_bst_ckmin_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char wled_resv1 : 8;
    } reg;
} SOC_SCHARGER_WLED_REG5_UNION;
#endif
#define SOC_SCHARGER_WLED_REG5_wled_resv1_START (0)
#define SOC_SCHARGER_WLED_REG5_wled_resv1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char wled_resv2 : 8;
    } reg;
} SOC_SCHARGER_WLED_REG6_UNION;
#endif
#define SOC_SCHARGER_WLED_REG6_wled_resv2_START (0)
#define SOC_SCHARGER_WLED_REG6_wled_resv2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char wled_resv3 : 8;
    } reg;
} SOC_SCHARGER_WLED_REG7_UNION;
#endif
#define SOC_SCHARGER_WLED_REG7_wled_resv3_START (0)
#define SOC_SCHARGER_WLED_REG7_wled_resv3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char wled_resv4 : 8;
    } reg;
} SOC_SCHARGER_WLED_REG8_UNION;
#endif
#define SOC_SCHARGER_WLED_REG8_wled_resv4_START (0)
#define SOC_SCHARGER_WLED_REG8_wled_resv4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char wled_driver_trim_test : 6;
        unsigned char wled_ovp_sel : 2;
    } reg;
} SOC_SCHARGER_WLED_REG9_UNION;
#endif
#define SOC_SCHARGER_WLED_REG9_wled_driver_trim_test_START (0)
#define SOC_SCHARGER_WLED_REG9_wled_driver_trim_test_END (5)
#define SOC_SCHARGER_WLED_REG9_wled_ovp_sel_START (6)
#define SOC_SCHARGER_WLED_REG9_wled_ovp_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_bst_en_uvp : 1;
        unsigned char lcd_bst_en_scp : 1;
        unsigned char lcd_bst_en_ovp : 1;
        unsigned char lcd_bst_en_ocp : 1;
        unsigned char lcd_bst_rcom : 3;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_LCD_BOOST_REG0_UNION;
#endif
#define SOC_SCHARGER_LCD_BOOST_REG0_lcd_bst_en_uvp_START (0)
#define SOC_SCHARGER_LCD_BOOST_REG0_lcd_bst_en_uvp_END (0)
#define SOC_SCHARGER_LCD_BOOST_REG0_lcd_bst_en_scp_START (1)
#define SOC_SCHARGER_LCD_BOOST_REG0_lcd_bst_en_scp_END (1)
#define SOC_SCHARGER_LCD_BOOST_REG0_lcd_bst_en_ovp_START (2)
#define SOC_SCHARGER_LCD_BOOST_REG0_lcd_bst_en_ovp_END (2)
#define SOC_SCHARGER_LCD_BOOST_REG0_lcd_bst_en_ocp_START (3)
#define SOC_SCHARGER_LCD_BOOST_REG0_lcd_bst_en_ocp_END (3)
#define SOC_SCHARGER_LCD_BOOST_REG0_lcd_bst_rcom_START (4)
#define SOC_SCHARGER_LCD_BOOST_REG0_lcd_bst_rcom_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_bst_ckmin : 2;
        unsigned char lcd_bst_en_clp_os : 1;
        unsigned char lcd_bst_en_slop : 1;
        unsigned char lcd_bst_en_pfm : 1;
        unsigned char lcd_bst_en_nring : 1;
        unsigned char lcd_bst_en_dmd : 1;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_LCD_BOOST_REG1_UNION;
#endif
#define SOC_SCHARGER_LCD_BOOST_REG1_lcd_bst_ckmin_START (0)
#define SOC_SCHARGER_LCD_BOOST_REG1_lcd_bst_ckmin_END (1)
#define SOC_SCHARGER_LCD_BOOST_REG1_lcd_bst_en_clp_os_START (2)
#define SOC_SCHARGER_LCD_BOOST_REG1_lcd_bst_en_clp_os_END (2)
#define SOC_SCHARGER_LCD_BOOST_REG1_lcd_bst_en_slop_START (3)
#define SOC_SCHARGER_LCD_BOOST_REG1_lcd_bst_en_slop_END (3)
#define SOC_SCHARGER_LCD_BOOST_REG1_lcd_bst_en_pfm_START (4)
#define SOC_SCHARGER_LCD_BOOST_REG1_lcd_bst_en_pfm_END (4)
#define SOC_SCHARGER_LCD_BOOST_REG1_lcd_bst_en_nring_START (5)
#define SOC_SCHARGER_LCD_BOOST_REG1_lcd_bst_en_nring_END (5)
#define SOC_SCHARGER_LCD_BOOST_REG1_lcd_bst_en_dmd_START (6)
#define SOC_SCHARGER_LCD_BOOST_REG1_lcd_bst_en_dmd_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_bst_ri : 2;
        unsigned char lcd_bst_gm : 4;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_LCD_BOOST_REG2_UNION;
#endif
#define SOC_SCHARGER_LCD_BOOST_REG2_lcd_bst_ri_START (0)
#define SOC_SCHARGER_LCD_BOOST_REG2_lcd_bst_ri_END (1)
#define SOC_SCHARGER_LCD_BOOST_REG2_lcd_bst_gm_START (2)
#define SOC_SCHARGER_LCD_BOOST_REG2_lcd_bst_gm_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_bst_clp : 3;
        unsigned char lcd_bst_sel_scp : 1;
        unsigned char lcd_bst_ccom2 : 2;
        unsigned char lcd_bst_ccom1 : 2;
    } reg;
} SOC_SCHARGER_LCD_BOOST_REG3_UNION;
#endif
#define SOC_SCHARGER_LCD_BOOST_REG3_lcd_bst_clp_START (0)
#define SOC_SCHARGER_LCD_BOOST_REG3_lcd_bst_clp_END (2)
#define SOC_SCHARGER_LCD_BOOST_REG3_lcd_bst_sel_scp_START (3)
#define SOC_SCHARGER_LCD_BOOST_REG3_lcd_bst_sel_scp_END (3)
#define SOC_SCHARGER_LCD_BOOST_REG3_lcd_bst_ccom2_START (4)
#define SOC_SCHARGER_LCD_BOOST_REG3_lcd_bst_ccom2_END (5)
#define SOC_SCHARGER_LCD_BOOST_REG3_lcd_bst_ccom1_START (6)
#define SOC_SCHARGER_LCD_BOOST_REG3_lcd_bst_ccom1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_bst_psn : 2;
        unsigned char lcd_bst_psp : 2;
        unsigned char lcd_bst_sel_ocp : 2;
        unsigned char lcd_bst_slop : 2;
    } reg;
} SOC_SCHARGER_LCD_BOOST_REG4_UNION;
#endif
#define SOC_SCHARGER_LCD_BOOST_REG4_lcd_bst_psn_START (0)
#define SOC_SCHARGER_LCD_BOOST_REG4_lcd_bst_psn_END (1)
#define SOC_SCHARGER_LCD_BOOST_REG4_lcd_bst_psp_START (2)
#define SOC_SCHARGER_LCD_BOOST_REG4_lcd_bst_psp_END (3)
#define SOC_SCHARGER_LCD_BOOST_REG4_lcd_bst_sel_ocp_START (4)
#define SOC_SCHARGER_LCD_BOOST_REG4_lcd_bst_sel_ocp_END (5)
#define SOC_SCHARGER_LCD_BOOST_REG4_lcd_bst_slop_START (6)
#define SOC_SCHARGER_LCD_BOOST_REG4_lcd_bst_slop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_bst_dt_nm : 2;
        unsigned char lcd_bst_dt_pm : 2;
        unsigned char lcd_bst_nsn : 2;
        unsigned char lcd_bst_nsp : 2;
    } reg;
} SOC_SCHARGER_LCD_BOOST_REG5_UNION;
#endif
#define SOC_SCHARGER_LCD_BOOST_REG5_lcd_bst_dt_nm_START (0)
#define SOC_SCHARGER_LCD_BOOST_REG5_lcd_bst_dt_nm_END (1)
#define SOC_SCHARGER_LCD_BOOST_REG5_lcd_bst_dt_pm_START (2)
#define SOC_SCHARGER_LCD_BOOST_REG5_lcd_bst_dt_pm_END (3)
#define SOC_SCHARGER_LCD_BOOST_REG5_lcd_bst_nsn_START (4)
#define SOC_SCHARGER_LCD_BOOST_REG5_lcd_bst_nsn_END (5)
#define SOC_SCHARGER_LCD_BOOST_REG5_lcd_bst_nsp_START (6)
#define SOC_SCHARGER_LCD_BOOST_REG5_lcd_bst_nsp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_bst_vdrop : 3;
        unsigned char lcd_bst_osc : 4;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_LCD_BOOST_REG6_UNION;
#endif
#define SOC_SCHARGER_LCD_BOOST_REG6_lcd_bst_vdrop_START (0)
#define SOC_SCHARGER_LCD_BOOST_REG6_lcd_bst_vdrop_END (2)
#define SOC_SCHARGER_LCD_BOOST_REG6_lcd_bst_osc_START (3)
#define SOC_SCHARGER_LCD_BOOST_REG6_lcd_bst_osc_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_bst_vo : 5;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_LCD_BOOST_REG7_UNION;
#endif
#define SOC_SCHARGER_LCD_BOOST_REG7_lcd_bst_vo_START (0)
#define SOC_SCHARGER_LCD_BOOST_REG7_lcd_bst_vo_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_bst_dmd_ofs : 8;
    } reg;
} SOC_SCHARGER_LCD_BOOST_REG8_UNION;
#endif
#define SOC_SCHARGER_LCD_BOOST_REG8_lcd_bst_dmd_ofs_START (0)
#define SOC_SCHARGER_LCD_BOOST_REG8_lcd_bst_dmd_ofs_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_bst_rsv1 : 8;
    } reg;
} SOC_SCHARGER_LCD_BOOST_REG9_UNION;
#endif
#define SOC_SCHARGER_LCD_BOOST_REG9_lcd_bst_rsv1_START (0)
#define SOC_SCHARGER_LCD_BOOST_REG9_lcd_bst_rsv1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_bst_rsv2 : 8;
    } reg;
} SOC_SCHARGER_LCD_BOOST_REG10_UNION;
#endif
#define SOC_SCHARGER_LCD_BOOST_REG10_lcd_bst_rsv2_START (0)
#define SOC_SCHARGER_LCD_BOOST_REG10_lcd_bst_rsv2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_bst_rsv3 : 8;
    } reg;
} SOC_SCHARGER_LCD_BOOST_REG11_UNION;
#endif
#define SOC_SCHARGER_LCD_BOOST_REG11_lcd_bst_rsv3_START (0)
#define SOC_SCHARGER_LCD_BOOST_REG11_lcd_bst_rsv3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_bst_rsv4 : 8;
    } reg;
} SOC_SCHARGER_LCD_BOOST_REG12_UNION;
#endif
#define SOC_SCHARGER_LCD_BOOST_REG12_lcd_bst_rsv4_START (0)
#define SOC_SCHARGER_LCD_BOOST_REG12_lcd_bst_rsv4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_bst_rsv5 : 8;
    } reg;
} SOC_SCHARGER_LCD_BOOST_REG13_UNION;
#endif
#define SOC_SCHARGER_LCD_BOOST_REG13_lcd_bst_rsv5_START (0)
#define SOC_SCHARGER_LCD_BOOST_REG13_lcd_bst_rsv5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_bst_rsv6 : 8;
    } reg;
} SOC_SCHARGER_LCD_BOOST_REG14_UNION;
#endif
#define SOC_SCHARGER_LCD_BOOST_REG14_lcd_bst_rsv6_START (0)
#define SOC_SCHARGER_LCD_BOOST_REG14_lcd_bst_rsv6_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_ncp_bwl_set : 2;
        unsigned char lcd_ncp_vset : 5;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_LCD_NCP_REG0_UNION;
#endif
#define SOC_SCHARGER_LCD_NCP_REG0_lcd_ncp_bwl_set_START (0)
#define SOC_SCHARGER_LCD_NCP_REG0_lcd_ncp_bwl_set_END (1)
#define SOC_SCHARGER_LCD_NCP_REG0_lcd_ncp_vset_START (2)
#define SOC_SCHARGER_LCD_NCP_REG0_lcd_ncp_vset_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_ncp_regop_clamp : 1;
        unsigned char lcd_ncp_regop_c : 1;
        unsigned char lcd_ncp_trm_sel : 1;
        unsigned char lcd_ncp_dtset : 2;
        unsigned char lcd_ncp_clk_ctr : 3;
    } reg;
} SOC_SCHARGER_LCD_NCP_REG1_UNION;
#endif
#define SOC_SCHARGER_LCD_NCP_REG1_lcd_ncp_regop_clamp_START (0)
#define SOC_SCHARGER_LCD_NCP_REG1_lcd_ncp_regop_clamp_END (0)
#define SOC_SCHARGER_LCD_NCP_REG1_lcd_ncp_regop_c_START (1)
#define SOC_SCHARGER_LCD_NCP_REG1_lcd_ncp_regop_c_END (1)
#define SOC_SCHARGER_LCD_NCP_REG1_lcd_ncp_trm_sel_START (2)
#define SOC_SCHARGER_LCD_NCP_REG1_lcd_ncp_trm_sel_END (2)
#define SOC_SCHARGER_LCD_NCP_REG1_lcd_ncp_dtset_START (3)
#define SOC_SCHARGER_LCD_NCP_REG1_lcd_ncp_dtset_END (4)
#define SOC_SCHARGER_LCD_NCP_REG1_lcd_ncp_clk_ctr_START (5)
#define SOC_SCHARGER_LCD_NCP_REG1_lcd_ncp_clk_ctr_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_ncp_trim_bak : 5;
        unsigned char lcd_ncp_reg_p : 2;
        unsigned char lcd_ncp_reg_c : 1;
    } reg;
} SOC_SCHARGER_LCD_NCP_REG2_UNION;
#endif
#define SOC_SCHARGER_LCD_NCP_REG2_lcd_ncp_trim_bak_START (0)
#define SOC_SCHARGER_LCD_NCP_REG2_lcd_ncp_trim_bak_END (4)
#define SOC_SCHARGER_LCD_NCP_REG2_lcd_ncp_reg_p_START (5)
#define SOC_SCHARGER_LCD_NCP_REG2_lcd_ncp_reg_p_END (6)
#define SOC_SCHARGER_LCD_NCP_REG2_lcd_ncp_reg_c_START (7)
#define SOC_SCHARGER_LCD_NCP_REG2_lcd_ncp_reg_c_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_ncp_rap_vx : 1;
        unsigned char lcd_ncp_rap_set : 2;
        unsigned char lcd_ncp_reg_n : 3;
        unsigned char lcd_ncp_vcmp_sel : 1;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_LCD_NCP_REG3_UNION;
#endif
#define SOC_SCHARGER_LCD_NCP_REG3_lcd_ncp_rap_vx_START (0)
#define SOC_SCHARGER_LCD_NCP_REG3_lcd_ncp_rap_vx_END (0)
#define SOC_SCHARGER_LCD_NCP_REG3_lcd_ncp_rap_set_START (1)
#define SOC_SCHARGER_LCD_NCP_REG3_lcd_ncp_rap_set_END (2)
#define SOC_SCHARGER_LCD_NCP_REG3_lcd_ncp_reg_n_START (3)
#define SOC_SCHARGER_LCD_NCP_REG3_lcd_ncp_reg_n_END (5)
#define SOC_SCHARGER_LCD_NCP_REG3_lcd_ncp_vcmp_sel_START (6)
#define SOC_SCHARGER_LCD_NCP_REG3_lcd_ncp_vcmp_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_ncp_vl1_disclamp : 1;
        unsigned char lcd_ncp_vh1_disclamp : 1;
        unsigned char lcd_ncp_g3dr_set : 2;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_LCD_NCP_REG4_UNION;
#endif
#define SOC_SCHARGER_LCD_NCP_REG4_lcd_ncp_vl1_disclamp_START (0)
#define SOC_SCHARGER_LCD_NCP_REG4_lcd_ncp_vl1_disclamp_END (0)
#define SOC_SCHARGER_LCD_NCP_REG4_lcd_ncp_vh1_disclamp_START (1)
#define SOC_SCHARGER_LCD_NCP_REG4_lcd_ncp_vh1_disclamp_END (1)
#define SOC_SCHARGER_LCD_NCP_REG4_lcd_ncp_g3dr_set_START (2)
#define SOC_SCHARGER_LCD_NCP_REG4_lcd_ncp_g3dr_set_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_ncp_resved : 8;
    } reg;
} SOC_SCHARGER_LCD_NCP_REG5_UNION;
#endif
#define SOC_SCHARGER_LCD_NCP_REG5_lcd_ncp_resved_START (0)
#define SOC_SCHARGER_LCD_NCP_REG5_lcd_ncp_resved_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lcd_ldo_vset : 5;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_LCD_LDO_REG0_UNION;
#endif
#define SOC_SCHARGER_LCD_LDO_REG0_lcd_ldo_vset_START (0)
#define SOC_SCHARGER_LCD_LDO_REG0_lcd_ldo_vset_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo2_200ma_vset : 4;
        unsigned char ldo1_200ma_vset : 4;
    } reg;
} SOC_SCHARGER_LDO_REG0_UNION;
#endif
#define SOC_SCHARGER_LDO_REG0_ldo2_200ma_vset_START (0)
#define SOC_SCHARGER_LDO_REG0_ldo2_200ma_vset_END (3)
#define SOC_SCHARGER_LDO_REG0_ldo1_200ma_vset_START (4)
#define SOC_SCHARGER_LDO_REG0_ldo1_200ma_vset_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck_max_ontime : 2;
        unsigned char buck_c2_sel : 2;
        unsigned char buck_c1_sel : 2;
        unsigned char buck_max_offtime : 2;
    } reg;
} SOC_SCHARGER_BUCK_REG0_UNION;
#endif
#define SOC_SCHARGER_BUCK_REG0_buck_max_ontime_START (0)
#define SOC_SCHARGER_BUCK_REG0_buck_max_ontime_END (1)
#define SOC_SCHARGER_BUCK_REG0_buck_c2_sel_START (2)
#define SOC_SCHARGER_BUCK_REG0_buck_c2_sel_END (3)
#define SOC_SCHARGER_BUCK_REG0_buck_c1_sel_START (4)
#define SOC_SCHARGER_BUCK_REG0_buck_c1_sel_END (5)
#define SOC_SCHARGER_BUCK_REG0_buck_max_offtime_START (6)
#define SOC_SCHARGER_BUCK_REG0_buck_max_offtime_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck_hmos_fall_ramp : 2;
        unsigned char buck_hmos_rise_ramp : 2;
        unsigned char buck_min_offtime : 2;
        unsigned char buck_min_ontime : 2;
    } reg;
} SOC_SCHARGER_BUCK_REG1_UNION;
#endif
#define SOC_SCHARGER_BUCK_REG1_buck_hmos_fall_ramp_START (0)
#define SOC_SCHARGER_BUCK_REG1_buck_hmos_fall_ramp_END (1)
#define SOC_SCHARGER_BUCK_REG1_buck_hmos_rise_ramp_START (2)
#define SOC_SCHARGER_BUCK_REG1_buck_hmos_rise_ramp_END (3)
#define SOC_SCHARGER_BUCK_REG1_buck_min_offtime_START (4)
#define SOC_SCHARGER_BUCK_REG1_buck_min_offtime_END (5)
#define SOC_SCHARGER_BUCK_REG1_buck_min_ontime_START (6)
#define SOC_SCHARGER_BUCK_REG1_buck_min_ontime_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck_en_dmd : 1;
        unsigned char buck_dt_lx_rise_delay_sel : 1;
        unsigned char buck_dt_lx_fall_sel : 1;
        unsigned char buck_dt_lx_rise_sel : 1;
        unsigned char buck_lmos_fall_ramp : 2;
        unsigned char buck_lmos_rise_ramp : 2;
    } reg;
} SOC_SCHARGER_BUCK_REG2_UNION;
#endif
#define SOC_SCHARGER_BUCK_REG2_buck_en_dmd_START (0)
#define SOC_SCHARGER_BUCK_REG2_buck_en_dmd_END (0)
#define SOC_SCHARGER_BUCK_REG2_buck_dt_lx_rise_delay_sel_START (1)
#define SOC_SCHARGER_BUCK_REG2_buck_dt_lx_rise_delay_sel_END (1)
#define SOC_SCHARGER_BUCK_REG2_buck_dt_lx_fall_sel_START (2)
#define SOC_SCHARGER_BUCK_REG2_buck_dt_lx_fall_sel_END (2)
#define SOC_SCHARGER_BUCK_REG2_buck_dt_lx_rise_sel_START (3)
#define SOC_SCHARGER_BUCK_REG2_buck_dt_lx_rise_sel_END (3)
#define SOC_SCHARGER_BUCK_REG2_buck_lmos_fall_ramp_START (4)
#define SOC_SCHARGER_BUCK_REG2_buck_lmos_fall_ramp_END (5)
#define SOC_SCHARGER_BUCK_REG2_buck_lmos_rise_ramp_START (6)
#define SOC_SCHARGER_BUCK_REG2_buck_lmos_rise_ramp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck_sysmin_sel : 2;
        unsigned char buck_dmd_offset : 3;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_BUCK_REG3_UNION;
#endif
#define SOC_SCHARGER_BUCK_REG3_buck_sysmin_sel_START (0)
#define SOC_SCHARGER_BUCK_REG3_buck_sysmin_sel_END (1)
#define SOC_SCHARGER_BUCK_REG3_buck_dmd_offset_START (2)
#define SOC_SCHARGER_BUCK_REG3_buck_dmd_offset_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck_dpm_sel : 4;
        unsigned char buck_int_lim_sel : 4;
    } reg;
} SOC_SCHARGER_BUCK_REG4_UNION;
#endif
#define SOC_SCHARGER_BUCK_REG4_buck_dpm_sel_START (0)
#define SOC_SCHARGER_BUCK_REG4_buck_dpm_sel_END (3)
#define SOC_SCHARGER_BUCK_REG4_buck_int_lim_sel_START (4)
#define SOC_SCHARGER_BUCK_REG4_buck_int_lim_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck_reserve : 8;
    } reg;
} SOC_SCHARGER_BUCK_REG5_UNION;
#endif
#define SOC_SCHARGER_BUCK_REG5_buck_reserve_START (0)
#define SOC_SCHARGER_BUCK_REG5_buck_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck_ocp_off : 1;
        unsigned char buck_ocp_sel : 2;
        unsigned char buck_gap_sel : 3;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_BUCK_REG6_UNION;
#endif
#define SOC_SCHARGER_BUCK_REG6_buck_ocp_off_START (0)
#define SOC_SCHARGER_BUCK_REG6_buck_ocp_off_END (0)
#define SOC_SCHARGER_BUCK_REG6_buck_ocp_sel_START (1)
#define SOC_SCHARGER_BUCK_REG6_buck_ocp_sel_END (2)
#define SOC_SCHARGER_BUCK_REG6_buck_gap_sel_START (3)
#define SOC_SCHARGER_BUCK_REG6_buck_gap_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otg_uvp_en : 1;
        unsigned char otg_scp_time : 1;
        unsigned char otg_scp_en : 1;
        unsigned char otg_ocp_sys : 1;
        unsigned char otg_ocp_en : 1;
        unsigned char otg_lim_set : 1;
        unsigned char otg_lim_en : 1;
        unsigned char otg_force_en_int : 1;
    } reg;
} SOC_SCHARGER_OTG_REG0_UNION;
#endif
#define SOC_SCHARGER_OTG_REG0_otg_uvp_en_START (0)
#define SOC_SCHARGER_OTG_REG0_otg_uvp_en_END (0)
#define SOC_SCHARGER_OTG_REG0_otg_scp_time_START (1)
#define SOC_SCHARGER_OTG_REG0_otg_scp_time_END (1)
#define SOC_SCHARGER_OTG_REG0_otg_scp_en_START (2)
#define SOC_SCHARGER_OTG_REG0_otg_scp_en_END (2)
#define SOC_SCHARGER_OTG_REG0_otg_ocp_sys_START (3)
#define SOC_SCHARGER_OTG_REG0_otg_ocp_sys_END (3)
#define SOC_SCHARGER_OTG_REG0_otg_ocp_en_START (4)
#define SOC_SCHARGER_OTG_REG0_otg_ocp_en_END (4)
#define SOC_SCHARGER_OTG_REG0_otg_lim_set_START (5)
#define SOC_SCHARGER_OTG_REG0_otg_lim_set_END (5)
#define SOC_SCHARGER_OTG_REG0_otg_lim_en_START (6)
#define SOC_SCHARGER_OTG_REG0_otg_lim_en_END (6)
#define SOC_SCHARGER_OTG_REG0_otg_force_en_int_START (7)
#define SOC_SCHARGER_OTG_REG0_otg_force_en_int_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otg_ovp_set_t : 2;
        unsigned char otg_skip_set : 1;
        unsigned char otg_ovp_en : 1;
        unsigned char otg_uvp_set_t : 2;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_OTG_REG1_UNION;
#endif
#define SOC_SCHARGER_OTG_REG1_otg_ovp_set_t_START (0)
#define SOC_SCHARGER_OTG_REG1_otg_ovp_set_t_END (1)
#define SOC_SCHARGER_OTG_REG1_otg_skip_set_START (2)
#define SOC_SCHARGER_OTG_REG1_otg_skip_set_END (2)
#define SOC_SCHARGER_OTG_REG1_otg_ovp_en_START (3)
#define SOC_SCHARGER_OTG_REG1_otg_ovp_en_END (3)
#define SOC_SCHARGER_OTG_REG1_otg_uvp_set_t_START (4)
#define SOC_SCHARGER_OTG_REG1_otg_uvp_set_t_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otg_clp_l_iav_en : 1;
        unsigned char otg_clp_h_iav_en : 1;
        unsigned char otg_clp_l_en : 1;
        unsigned char otg_clp_h_en : 1;
        unsigned char otg_slop_set : 2;
        unsigned char otg_slop_en : 1;
        unsigned char otg_dmd : 1;
    } reg;
} SOC_SCHARGER_OTG_REG2_UNION;
#endif
#define SOC_SCHARGER_OTG_REG2_otg_clp_l_iav_en_START (0)
#define SOC_SCHARGER_OTG_REG2_otg_clp_l_iav_en_END (0)
#define SOC_SCHARGER_OTG_REG2_otg_clp_h_iav_en_START (1)
#define SOC_SCHARGER_OTG_REG2_otg_clp_h_iav_en_END (1)
#define SOC_SCHARGER_OTG_REG2_otg_clp_l_en_START (2)
#define SOC_SCHARGER_OTG_REG2_otg_clp_l_en_END (2)
#define SOC_SCHARGER_OTG_REG2_otg_clp_h_en_START (3)
#define SOC_SCHARGER_OTG_REG2_otg_clp_h_en_END (3)
#define SOC_SCHARGER_OTG_REG2_otg_slop_set_START (4)
#define SOC_SCHARGER_OTG_REG2_otg_slop_set_END (5)
#define SOC_SCHARGER_OTG_REG2_otg_slop_en_START (6)
#define SOC_SCHARGER_OTG_REG2_otg_slop_en_END (6)
#define SOC_SCHARGER_OTG_REG2_otg_dmd_START (7)
#define SOC_SCHARGER_OTG_REG2_otg_dmd_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otg_gm : 3;
        unsigned char otg_iav_t_set : 2;
        unsigned char reseved : 1;
        unsigned char reserved : 1;
        unsigned char otg_pfm_v_en : 1;
    } reg;
} SOC_SCHARGER_OTG_REG3_UNION;
#endif
#define SOC_SCHARGER_OTG_REG3_otg_gm_START (0)
#define SOC_SCHARGER_OTG_REG3_otg_gm_END (2)
#define SOC_SCHARGER_OTG_REG3_otg_iav_t_set_START (3)
#define SOC_SCHARGER_OTG_REG3_otg_iav_t_set_END (4)
#define SOC_SCHARGER_OTG_REG3_reseved_START (5)
#define SOC_SCHARGER_OTG_REG3_reseved_END (5)
#define SOC_SCHARGER_OTG_REG3_otg_pfm_v_en_START (7)
#define SOC_SCHARGER_OTG_REG3_otg_pfm_v_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otg_ccom2_cmp : 2;
        unsigned char otg_ccom1_cmp : 2;
        unsigned char otg_rcom_cmp : 2;
        unsigned char otg_clp_l_set : 1;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_OTG_REG4_UNION;
#endif
#define SOC_SCHARGER_OTG_REG4_otg_ccom2_cmp_START (0)
#define SOC_SCHARGER_OTG_REG4_otg_ccom2_cmp_END (1)
#define SOC_SCHARGER_OTG_REG4_otg_ccom1_cmp_START (2)
#define SOC_SCHARGER_OTG_REG4_otg_ccom1_cmp_END (3)
#define SOC_SCHARGER_OTG_REG4_otg_rcom_cmp_START (4)
#define SOC_SCHARGER_OTG_REG4_otg_rcom_cmp_END (5)
#define SOC_SCHARGER_OTG_REG4_otg_clp_l_set_START (6)
#define SOC_SCHARGER_OTG_REG4_otg_clp_l_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otg_ccom_iav : 2;
        unsigned char otg_rcom2_iav : 2;
        unsigned char otg_rcom1_iav : 2;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_OTG_REG5_UNION;
#endif
#define SOC_SCHARGER_OTG_REG5_otg_ccom_iav_START (0)
#define SOC_SCHARGER_OTG_REG5_otg_ccom_iav_END (1)
#define SOC_SCHARGER_OTG_REG5_otg_rcom2_iav_START (2)
#define SOC_SCHARGER_OTG_REG5_otg_rcom2_iav_END (3)
#define SOC_SCHARGER_OTG_REG5_otg_rcom1_iav_START (4)
#define SOC_SCHARGER_OTG_REG5_otg_rcom1_iav_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otg_lmos_ocp : 2;
        unsigned char otg_osc_ckmax : 2;
        unsigned char otg_osc : 4;
    } reg;
} SOC_SCHARGER_OTG_REG6_UNION;
#endif
#define SOC_SCHARGER_OTG_REG6_otg_lmos_ocp_START (0)
#define SOC_SCHARGER_OTG_REG6_otg_lmos_ocp_END (1)
#define SOC_SCHARGER_OTG_REG6_otg_osc_ckmax_START (2)
#define SOC_SCHARGER_OTG_REG6_otg_osc_ckmax_END (3)
#define SOC_SCHARGER_OTG_REG6_otg_osc_START (4)
#define SOC_SCHARGER_OTG_REG6_otg_osc_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otg_dmd_ofs : 4;
        unsigned char otg_rf : 2;
        unsigned char otg_ckmin : 2;
    } reg;
} SOC_SCHARGER_OTG_REG7_UNION;
#endif
#define SOC_SCHARGER_OTG_REG7_otg_dmd_ofs_START (0)
#define SOC_SCHARGER_OTG_REG7_otg_dmd_ofs_END (3)
#define SOC_SCHARGER_OTG_REG7_otg_rf_START (4)
#define SOC_SCHARGER_OTG_REG7_otg_rf_END (5)
#define SOC_SCHARGER_OTG_REG7_otg_ckmin_START (6)
#define SOC_SCHARGER_OTG_REG7_otg_ckmin_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otg_vo : 2;
        unsigned char otg_vbat_time : 2;
        unsigned char otg_vbat : 4;
    } reg;
} SOC_SCHARGER_OTG_REG8_UNION;
#endif
#define SOC_SCHARGER_OTG_REG8_otg_vo_START (0)
#define SOC_SCHARGER_OTG_REG8_otg_vo_END (1)
#define SOC_SCHARGER_OTG_REG8_otg_vbat_time_START (2)
#define SOC_SCHARGER_OTG_REG8_otg_vbat_time_END (3)
#define SOC_SCHARGER_OTG_REG8_otg_vbat_START (4)
#define SOC_SCHARGER_OTG_REG8_otg_vbat_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otg_delay2 : 2;
        unsigned char otg_delay1 : 2;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_OTG_REG9_UNION;
#endif
#define SOC_SCHARGER_OTG_REG9_otg_delay2_START (0)
#define SOC_SCHARGER_OTG_REG9_otg_delay2_END (1)
#define SOC_SCHARGER_OTG_REG9_otg_delay1_START (2)
#define SOC_SCHARGER_OTG_REG9_otg_delay1_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otg_resved : 8;
    } reg;
} SOC_SCHARGER_OTG_REG10_UNION;
#endif
#define SOC_SCHARGER_OTG_REG10_otg_resved_START (0)
#define SOC_SCHARGER_OTG_REG10_otg_resved_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chg_pre_vchg : 2;
        unsigned char chg_pre_ichg : 2;
        unsigned char batfet_ctrl : 1;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_CHG_REG0_UNION;
#endif
#define SOC_SCHARGER_CHG_REG0_chg_pre_vchg_START (0)
#define SOC_SCHARGER_CHG_REG0_chg_pre_vchg_END (1)
#define SOC_SCHARGER_CHG_REG0_chg_pre_ichg_START (2)
#define SOC_SCHARGER_CHG_REG0_chg_pre_ichg_END (3)
#define SOC_SCHARGER_CHG_REG0_batfet_ctrl_START (4)
#define SOC_SCHARGER_CHG_REG0_batfet_ctrl_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chg_fast_ichg : 5;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_CHG_REG1_UNION;
#endif
#define SOC_SCHARGER_CHG_REG1_chg_fast_ichg_START (0)
#define SOC_SCHARGER_CHG_REG1_chg_fast_ichg_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chg_term_ichg : 2;
        unsigned char chg_fast_vchg : 4;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_CHG_REG2_UNION;
#endif
#define SOC_SCHARGER_CHG_REG2_chg_term_ichg_START (0)
#define SOC_SCHARGER_CHG_REG2_chg_term_ichg_END (1)
#define SOC_SCHARGER_CHG_REG2_chg_fast_vchg_START (2)
#define SOC_SCHARGER_CHG_REG2_chg_fast_vchg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chg_prechg_timer : 2;
        unsigned char chg_rechg_time : 2;
        unsigned char chg_vrechg_hys : 3;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_CHG_REG3_UNION;
#endif
#define SOC_SCHARGER_CHG_REG3_chg_prechg_timer_START (0)
#define SOC_SCHARGER_CHG_REG3_chg_prechg_timer_END (1)
#define SOC_SCHARGER_CHG_REG3_chg_rechg_time_START (2)
#define SOC_SCHARGER_CHG_REG3_chg_rechg_time_END (3)
#define SOC_SCHARGER_CHG_REG3_chg_vrechg_hys_START (4)
#define SOC_SCHARGER_CHG_REG3_chg_vrechg_hys_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chg_vbatir_adj : 4;
        unsigned char chg_fastchg_safe_timer : 2;
        unsigned char chg_termination_ctrl : 1;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_CHG_REG4_UNION;
#endif
#define SOC_SCHARGER_CHG_REG4_chg_vbatir_adj_START (0)
#define SOC_SCHARGER_CHG_REG4_chg_vbatir_adj_END (3)
#define SOC_SCHARGER_CHG_REG4_chg_fastchg_safe_timer_START (4)
#define SOC_SCHARGER_CHG_REG4_chg_fastchg_safe_timer_END (5)
#define SOC_SCHARGER_CHG_REG4_chg_termination_ctrl_START (6)
#define SOC_SCHARGER_CHG_REG4_chg_termination_ctrl_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chg_hot_hsys : 2;
        unsigned char chg_cold_hsys : 2;
        unsigned char chg_smhys_adj : 3;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_CHG_REG5_UNION;
#endif
#define SOC_SCHARGER_CHG_REG5_chg_hot_hsys_START (0)
#define SOC_SCHARGER_CHG_REG5_chg_hot_hsys_END (1)
#define SOC_SCHARGER_CHG_REG5_chg_cold_hsys_START (2)
#define SOC_SCHARGER_CHG_REG5_chg_cold_hsys_END (3)
#define SOC_SCHARGER_CHG_REG5_chg_smhys_adj_START (4)
#define SOC_SCHARGER_CHG_REG5_chg_smhys_adj_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char therm_state : 1;
        unsigned char wled_ovp_ifb_state : 2;
        unsigned char buck_ok : 1;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_CHG_REG6_UNION;
#endif
#define SOC_SCHARGER_CHG_REG6_therm_state_START (0)
#define SOC_SCHARGER_CHG_REG6_therm_state_END (0)
#define SOC_SCHARGER_CHG_REG6_wled_ovp_ifb_state_START (1)
#define SOC_SCHARGER_CHG_REG6_wled_ovp_ifb_state_END (2)
#define SOC_SCHARGER_CHG_REG6_buck_ok_START (3)
#define SOC_SCHARGER_CHG_REG6_buck_ok_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chg_clk_ctl1 : 1;
        unsigned char ts_ctrl : 1;
        unsigned char reserved : 6;
    } reg;
} SOC_SCHARGER_CHG_REG7_UNION;
#endif
#define SOC_SCHARGER_CHG_REG7_chg_clk_ctl1_START (0)
#define SOC_SCHARGER_CHG_REG7_chg_clk_ctl1_END (0)
#define SOC_SCHARGER_CHG_REG7_ts_ctrl_START (1)
#define SOC_SCHARGER_CHG_REG7_ts_ctrl_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chg_resvi1_2d : 8;
    } reg;
} SOC_SCHARGER_CHG_REG8_UNION;
#endif
#define SOC_SCHARGER_CHG_REG8_chg_resvi1_2d_START (0)
#define SOC_SCHARGER_CHG_REG8_chg_resvi1_2d_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chg_resvi2_2d : 8;
    } reg;
} SOC_SCHARGER_CHG_REG9_UNION;
#endif
#define SOC_SCHARGER_CHG_REG9_chg_resvi2_2d_START (0)
#define SOC_SCHARGER_CHG_REG9_chg_resvi2_2d_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chg_resvo1 : 8;
    } reg;
} SOC_SCHARGER_CHG_REG10_UNION;
#endif
#define SOC_SCHARGER_CHG_REG10_chg_resvo1_START (0)
#define SOC_SCHARGER_CHG_REG10_chg_resvo1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chg_resvo2 : 8;
    } reg;
} SOC_SCHARGER_CHG_REG11_UNION;
#endif
#define SOC_SCHARGER_CHG_REG11_chg_resvo2_START (0)
#define SOC_SCHARGER_CHG_REG11_chg_resvo2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vbus_ovp_uptime : 2;
        unsigned char regn_lv_sel : 2;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_PSEL_REG0_UNION;
#endif
#define SOC_SCHARGER_PSEL_REG0_vbus_ovp_uptime_START (0)
#define SOC_SCHARGER_PSEL_REG0_vbus_ovp_uptime_END (1)
#define SOC_SCHARGER_PSEL_REG0_regn_lv_sel_START (2)
#define SOC_SCHARGER_PSEL_REG0_regn_lv_sel_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved_0 : 5;
        unsigned char flash_bst_pm_cut : 1;
        unsigned char flash_bst_pm_th : 1;
        unsigned char reserved_1 : 1;
    } reg;
} SOC_SCHARGER_FLASH_BST_REG0_UNION;
#endif
#define SOC_SCHARGER_FLASH_BST_REG0_flash_bst_pm_cut_START (5)
#define SOC_SCHARGER_FLASH_BST_REG0_flash_bst_pm_cut_END (5)
#define SOC_SCHARGER_FLASH_BST_REG0_flash_bst_pm_th_START (6)
#define SOC_SCHARGER_FLASH_BST_REG0_flash_bst_pm_th_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_bst_sel_scp : 1;
        unsigned char flash_bst_en_uvp : 1;
        unsigned char flash_bst_en_scp : 1;
        unsigned char flash_bst_en_ovp : 1;
        unsigned char flash_bst_en_ocp : 1;
        unsigned char reserved_0 : 1;
        unsigned char reserved_1 : 2;
    } reg;
} SOC_SCHARGER_FLASH_BST_REG1_UNION;
#endif
#define SOC_SCHARGER_FLASH_BST_REG1_flash_bst_sel_scp_START (0)
#define SOC_SCHARGER_FLASH_BST_REG1_flash_bst_sel_scp_END (0)
#define SOC_SCHARGER_FLASH_BST_REG1_flash_bst_en_uvp_START (1)
#define SOC_SCHARGER_FLASH_BST_REG1_flash_bst_en_uvp_END (1)
#define SOC_SCHARGER_FLASH_BST_REG1_flash_bst_en_scp_START (2)
#define SOC_SCHARGER_FLASH_BST_REG1_flash_bst_en_scp_END (2)
#define SOC_SCHARGER_FLASH_BST_REG1_flash_bst_en_ovp_START (3)
#define SOC_SCHARGER_FLASH_BST_REG1_flash_bst_en_ovp_END (3)
#define SOC_SCHARGER_FLASH_BST_REG1_flash_bst_en_ocp_START (4)
#define SOC_SCHARGER_FLASH_BST_REG1_flash_bst_en_ocp_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_bst_en_dmd : 1;
        unsigned char flash_bst_en_slop : 1;
        unsigned char flash_bst_en_clp_os : 1;
        unsigned char flash_bst_en_nring : 1;
        unsigned char flash_bst_en_pfm : 1;
        unsigned char flash_bst_drv_mode : 1;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_FLASH_BST_REG2_UNION;
#endif
#define SOC_SCHARGER_FLASH_BST_REG2_flash_bst_en_dmd_START (0)
#define SOC_SCHARGER_FLASH_BST_REG2_flash_bst_en_dmd_END (0)
#define SOC_SCHARGER_FLASH_BST_REG2_flash_bst_en_slop_START (1)
#define SOC_SCHARGER_FLASH_BST_REG2_flash_bst_en_slop_END (1)
#define SOC_SCHARGER_FLASH_BST_REG2_flash_bst_en_clp_os_START (2)
#define SOC_SCHARGER_FLASH_BST_REG2_flash_bst_en_clp_os_END (2)
#define SOC_SCHARGER_FLASH_BST_REG2_flash_bst_en_nring_START (3)
#define SOC_SCHARGER_FLASH_BST_REG2_flash_bst_en_nring_END (3)
#define SOC_SCHARGER_FLASH_BST_REG2_flash_bst_en_pfm_START (4)
#define SOC_SCHARGER_FLASH_BST_REG2_flash_bst_en_pfm_END (4)
#define SOC_SCHARGER_FLASH_BST_REG2_flash_bst_drv_mode_START (5)
#define SOC_SCHARGER_FLASH_BST_REG2_flash_bst_drv_mode_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_bst_clp : 3;
        unsigned char flash_bst_itail : 1;
        unsigned char flash_bst_gm : 3;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_FLASH_BST_REG3_UNION;
#endif
#define SOC_SCHARGER_FLASH_BST_REG3_flash_bst_clp_START (0)
#define SOC_SCHARGER_FLASH_BST_REG3_flash_bst_clp_END (2)
#define SOC_SCHARGER_FLASH_BST_REG3_flash_bst_itail_START (3)
#define SOC_SCHARGER_FLASH_BST_REG3_flash_bst_itail_END (3)
#define SOC_SCHARGER_FLASH_BST_REG3_flash_bst_gm_START (4)
#define SOC_SCHARGER_FLASH_BST_REG3_flash_bst_gm_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_bst_ccom2 : 2;
        unsigned char flash_bst_ccom1 : 2;
        unsigned char flash_bst_rcom : 3;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_FLASH_BST_REG4_UNION;
#endif
#define SOC_SCHARGER_FLASH_BST_REG4_flash_bst_ccom2_START (0)
#define SOC_SCHARGER_FLASH_BST_REG4_flash_bst_ccom2_END (1)
#define SOC_SCHARGER_FLASH_BST_REG4_flash_bst_ccom1_START (2)
#define SOC_SCHARGER_FLASH_BST_REG4_flash_bst_ccom1_END (3)
#define SOC_SCHARGER_FLASH_BST_REG4_flash_bst_rcom_START (4)
#define SOC_SCHARGER_FLASH_BST_REG4_flash_bst_rcom_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_bst_slop : 2;
        unsigned char flash_bst_ri : 2;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_FLASH_BST_REG5_UNION;
#endif
#define SOC_SCHARGER_FLASH_BST_REG5_flash_bst_slop_START (0)
#define SOC_SCHARGER_FLASH_BST_REG5_flash_bst_slop_END (1)
#define SOC_SCHARGER_FLASH_BST_REG5_flash_bst_ri_START (2)
#define SOC_SCHARGER_FLASH_BST_REG5_flash_bst_ri_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_bst_nsn : 2;
        unsigned char flash_bst_nsp : 2;
        unsigned char flash_bst_psn : 2;
        unsigned char flash_bst_psp : 2;
    } reg;
} SOC_SCHARGER_FLASH_BST_REG6_UNION;
#endif
#define SOC_SCHARGER_FLASH_BST_REG6_flash_bst_nsn_START (0)
#define SOC_SCHARGER_FLASH_BST_REG6_flash_bst_nsn_END (1)
#define SOC_SCHARGER_FLASH_BST_REG6_flash_bst_nsp_START (2)
#define SOC_SCHARGER_FLASH_BST_REG6_flash_bst_nsp_END (3)
#define SOC_SCHARGER_FLASH_BST_REG6_flash_bst_psn_START (4)
#define SOC_SCHARGER_FLASH_BST_REG6_flash_bst_psn_END (5)
#define SOC_SCHARGER_FLASH_BST_REG6_flash_bst_psp_START (6)
#define SOC_SCHARGER_FLASH_BST_REG6_flash_bst_psp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_bst_dt_nm : 2;
        unsigned char flash_bst_dt_pm : 2;
        unsigned char flash_bst_lxde : 2;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_FLASH_BST_REG7_UNION;
#endif
#define SOC_SCHARGER_FLASH_BST_REG7_flash_bst_dt_nm_START (0)
#define SOC_SCHARGER_FLASH_BST_REG7_flash_bst_dt_nm_END (1)
#define SOC_SCHARGER_FLASH_BST_REG7_flash_bst_dt_pm_START (2)
#define SOC_SCHARGER_FLASH_BST_REG7_flash_bst_dt_pm_END (3)
#define SOC_SCHARGER_FLASH_BST_REG7_flash_bst_lxde_START (4)
#define SOC_SCHARGER_FLASH_BST_REG7_flash_bst_lxde_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_bst_ckmin : 2;
        unsigned char flash_bst_osc : 4;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_FLASH_BST_REG8_UNION;
#endif
#define SOC_SCHARGER_FLASH_BST_REG8_flash_bst_ckmin_START (0)
#define SOC_SCHARGER_FLASH_BST_REG8_flash_bst_ckmin_END (1)
#define SOC_SCHARGER_FLASH_BST_REG8_flash_bst_osc_START (2)
#define SOC_SCHARGER_FLASH_BST_REG8_flash_bst_osc_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_bst_sel_ocp : 2;
        unsigned char flash_bst_start : 3;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_FLASH_BST_REG9_UNION;
#endif
#define SOC_SCHARGER_FLASH_BST_REG9_flash_bst_sel_ocp_START (0)
#define SOC_SCHARGER_FLASH_BST_REG9_flash_bst_sel_ocp_END (1)
#define SOC_SCHARGER_FLASH_BST_REG9_flash_bst_start_START (2)
#define SOC_SCHARGER_FLASH_BST_REG9_flash_bst_start_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_bst_vo : 5;
        unsigned char flash_bst_dmd_isel : 1;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_FLASH_BST_REG10_UNION;
#endif
#define SOC_SCHARGER_FLASH_BST_REG10_flash_bst_vo_START (0)
#define SOC_SCHARGER_FLASH_BST_REG10_flash_bst_vo_END (4)
#define SOC_SCHARGER_FLASH_BST_REG10_flash_bst_dmd_isel_START (5)
#define SOC_SCHARGER_FLASH_BST_REG10_flash_bst_dmd_isel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_bst_dmd_ofs : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_FLASH_BST_REG11_UNION;
#endif
#define SOC_SCHARGER_FLASH_BST_REG11_flash_bst_dmd_ofs_START (0)
#define SOC_SCHARGER_FLASH_BST_REG11_flash_bst_dmd_ofs_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_led_trimstep_sel : 2;
        unsigned char flash_led_ledfault_vsel : 2;
        unsigned char flash_led_ledfault_en : 1;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_FLASH_LED_REG0_UNION;
#endif
#define SOC_SCHARGER_FLASH_LED_REG0_flash_led_trimstep_sel_START (0)
#define SOC_SCHARGER_FLASH_LED_REG0_flash_led_trimstep_sel_END (1)
#define SOC_SCHARGER_FLASH_LED_REG0_flash_led_ledfault_vsel_START (2)
#define SOC_SCHARGER_FLASH_LED_REG0_flash_led_ledfault_vsel_END (3)
#define SOC_SCHARGER_FLASH_LED_REG0_flash_led_ledfault_en_START (4)
#define SOC_SCHARGER_FLASH_LED_REG0_flash_led_ledfault_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_led_vbatdroop : 3;
        unsigned char flash_led_vbattorch : 4;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_FLASH_LED_REG1_UNION;
#endif
#define SOC_SCHARGER_FLASH_LED_REG1_flash_led_vbatdroop_START (0)
#define SOC_SCHARGER_FLASH_LED_REG1_flash_led_vbatdroop_END (2)
#define SOC_SCHARGER_FLASH_LED_REG1_flash_led_vbattorch_START (3)
#define SOC_SCHARGER_FLASH_LED_REG1_flash_led_vbattorch_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sys_resv : 8;
    } reg;
} SOC_SCHARGER_SYS_REG_UNION;
#endif
#define SOC_SCHARGER_SYS_REG_sys_resv_START (0)
#define SOC_SCHARGER_SYS_REG_sys_resv_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_led_tbatdroop : 4;
        unsigned char flash_led_tfault : 3;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_FLASH_LED_REG2_UNION;
#endif
#define SOC_SCHARGER_FLASH_LED_REG2_flash_led_tbatdroop_START (0)
#define SOC_SCHARGER_FLASH_LED_REG2_flash_led_tbatdroop_END (3)
#define SOC_SCHARGER_FLASH_LED_REG2_flash_led_tfault_START (4)
#define SOC_SCHARGER_FLASH_LED_REG2_flash_led_tfault_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_led_tf : 3;
        unsigned char flash_led_tr : 3;
        unsigned char flash_led_mask_mask : 1;
        unsigned char flash_led_mask_once_down : 1;
    } reg;
} SOC_SCHARGER_FLASH_LED_REG3_UNION;
#endif
#define SOC_SCHARGER_FLASH_LED_REG3_flash_led_tf_START (0)
#define SOC_SCHARGER_FLASH_LED_REG3_flash_led_tf_END (2)
#define SOC_SCHARGER_FLASH_LED_REG3_flash_led_tr_START (3)
#define SOC_SCHARGER_FLASH_LED_REG3_flash_led_tr_END (5)
#define SOC_SCHARGER_FLASH_LED_REG3_flash_led_mask_mask_START (6)
#define SOC_SCHARGER_FLASH_LED_REG3_flash_led_mask_mask_END (6)
#define SOC_SCHARGER_FLASH_LED_REG3_flash_led_mask_once_down_START (7)
#define SOC_SCHARGER_FLASH_LED_REG3_flash_led_mask_once_down_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_led_itorch_set : 3;
        unsigned char flash_led_iflash_set : 4;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_FLASH_LED_REG4_UNION;
#endif
#define SOC_SCHARGER_FLASH_LED_REG4_flash_led_itorch_set_START (0)
#define SOC_SCHARGER_FLASH_LED_REG4_flash_led_itorch_set_END (2)
#define SOC_SCHARGER_FLASH_LED_REG4_flash_led_iflash_set_START (3)
#define SOC_SCHARGER_FLASH_LED_REG4_flash_led_iflash_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_led_isafe_set : 3;
        unsigned char reserved : 5;
    } reg;
} SOC_SCHARGER_FLASH_LED_REG5_UNION;
#endif
#define SOC_SCHARGER_FLASH_LED_REG5_flash_led_isafe_set_START (0)
#define SOC_SCHARGER_FLASH_LED_REG5_flash_led_isafe_set_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_led_watchdog_timer : 4;
        unsigned char flash_led_timeout : 2;
        unsigned char flash_led_tpwrsel : 2;
    } reg;
} SOC_SCHARGER_FLASH_LED_REG6_UNION;
#endif
#define SOC_SCHARGER_FLASH_LED_REG6_flash_led_watchdog_timer_START (0)
#define SOC_SCHARGER_FLASH_LED_REG6_flash_led_watchdog_timer_END (3)
#define SOC_SCHARGER_FLASH_LED_REG6_flash_led_timeout_START (4)
#define SOC_SCHARGER_FLASH_LED_REG6_flash_led_timeout_END (5)
#define SOC_SCHARGER_FLASH_LED_REG6_flash_led_tpwrsel_START (6)
#define SOC_SCHARGER_FLASH_LED_REG6_flash_led_tpwrsel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_led_timeout_en : 1;
        unsigned char flash_led_open_autostop : 1;
        unsigned char flash_led_short_autostop : 1;
        unsigned char flash_strobe_mask : 1;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_FLASH_LED_REG7_UNION;
#endif
#define SOC_SCHARGER_FLASH_LED_REG7_flash_led_timeout_en_START (0)
#define SOC_SCHARGER_FLASH_LED_REG7_flash_led_timeout_en_END (0)
#define SOC_SCHARGER_FLASH_LED_REG7_flash_led_open_autostop_START (1)
#define SOC_SCHARGER_FLASH_LED_REG7_flash_led_open_autostop_END (1)
#define SOC_SCHARGER_FLASH_LED_REG7_flash_led_short_autostop_START (2)
#define SOC_SCHARGER_FLASH_LED_REG7_flash_led_short_autostop_END (2)
#define SOC_SCHARGER_FLASH_LED_REG7_flash_strobe_mask_START (3)
#define SOC_SCHARGER_FLASH_LED_REG7_flash_strobe_mask_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soft_rst_n : 1;
        unsigned char wd_rst_n : 1;
        unsigned char flash_led_wd_rst : 1;
        unsigned char reserved : 5;
    } reg;
} SOC_SCHARGER_CTRL_REG_UNION;
#endif
#define SOC_SCHARGER_CTRL_REG_soft_rst_n_START (0)
#define SOC_SCHARGER_CTRL_REG_soft_rst_n_END (0)
#define SOC_SCHARGER_CTRL_REG_wd_rst_n_START (1)
#define SOC_SCHARGER_CTRL_REG_wd_rst_n_END (1)
#define SOC_SCHARGER_CTRL_REG_flash_led_wd_rst_START (2)
#define SOC_SCHARGER_CTRL_REG_flash_led_wd_rst_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char watchdog_timer : 2;
        unsigned char reserved : 6;
    } reg;
} SOC_SCHARGER_SYS_SET_REG_UNION;
#endif
#define SOC_SCHARGER_SYS_SET_REG_watchdog_timer_START (0)
#define SOC_SCHARGER_SYS_SET_REG_watchdog_timer_END (1)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
