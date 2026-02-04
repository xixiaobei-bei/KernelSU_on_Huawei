#ifndef __SOC_USB3OTG_BC_INTERFACE_H__
#define __SOC_USB3OTG_BC_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_ADDR(base) ((base) + (0x00))
#define SOC_USB3OTG_BC_USBOTG3_CTRL1_ADDR(base) ((base) + (0x04))
#define SOC_USB3OTG_BC_USBOTG3_CTRL2_ADDR(base) ((base) + (0x08))
#define SOC_USB3OTG_BC_USBOTG3_CTRL3_ADDR(base) ((base) + (0x0C))
#define SOC_USB3OTG_BC_USBOTG3_CTRL4_ADDR(base) ((base) + (0x10))
#define SOC_USB3OTG_BC_USBOTG3_CTRL5_ADDR(base) ((base) + (0x14))
#define SOC_USB3OTG_BC_USBOTG3_CTRL6_ADDR(base) ((base) + (0x18))
#define SOC_USB3OTG_BC_USBOTG3_CTRL7_ADDR(base) ((base) + (0x1C))
#define SOC_USB3OTG_BC_USBOTG3_STS0_ADDR(base) ((base) + (0x20))
#define SOC_USB3OTG_BC_USBOTG3_STS1_ADDR(base) ((base) + (0x24))
#define SOC_USB3OTG_BC_USBOTG3_STS2_ADDR(base) ((base) + (0x28))
#define SOC_USB3OTG_BC_USBOTG3_STS3_ADDR(base) ((base) + (0x2C))
#define SOC_USB3OTG_BC_BC_CTRL0_ADDR(base) ((base) + (0x30))
#define SOC_USB3OTG_BC_BC_CTRL1_ADDR(base) ((base) + (0x34))
#define SOC_USB3OTG_BC_BC_CTRL2_ADDR(base) ((base) + (0x38))
#define SOC_USB3OTG_BC_BC_STS0_ADDR(base) ((base) + (0x3C))
#define SOC_USB3OTG_BC_RAM_CTRL_ADDR(base) ((base) + (0x40))
#define SOC_USB3OTG_BC_USBOTG3_STS4_ADDR(base) ((base) + (0x44))
#define SOC_USB3OTG_BC_USB3PHY_CTRL_ADDR(base) ((base) + (0x48))
#define SOC_USB3OTG_BC_USB3PHY_STS_ADDR(base) ((base) + (0x4C))
#define SOC_USB3OTG_BC_USB3PHY_CR_STS_ADDR(base) ((base) + (0x50))
#define SOC_USB3OTG_BC_USB3PHY_CR_CTRL_ADDR(base) ((base) + (0x54))
#define SOC_USB3OTG_BC_USB3_RES_ADDR(base) ((base) + (0x58))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int idpullup_sel : 1;
        unsigned int idpullup : 1;
        unsigned int iddig_sel : 1;
        unsigned int iddig : 1;
        unsigned int drvvbus_sel : 1;
        unsigned int drvvbus : 1;
        unsigned int vbusvalid_sel : 1;
        unsigned int sc_avalid : 1;
        unsigned int avalid_sel : 1;
        unsigned int sc_bvalid : 1;
        unsigned int bvalid_sel : 1;
        unsigned int sc_powerpresent : 1;
        unsigned int powerpresent_sel : 1;
        unsigned int sc_sessvld : 1;
        unsigned int sessvld_sel : 1;
        unsigned int reserved : 17;
    } reg;
} SOC_USB3OTG_BC_USBOTG3_CTRL0_UNION;
#endif
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_idpullup_sel_START (0)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_idpullup_sel_END (0)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_idpullup_START (1)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_idpullup_END (1)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_iddig_sel_START (2)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_iddig_sel_END (2)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_iddig_START (3)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_iddig_END (3)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_drvvbus_sel_START (4)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_drvvbus_sel_END (4)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_drvvbus_START (5)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_drvvbus_END (5)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_vbusvalid_sel_START (6)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_vbusvalid_sel_END (6)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_sc_avalid_START (7)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_sc_avalid_END (7)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_avalid_sel_START (8)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_avalid_sel_END (8)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_sc_bvalid_START (9)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_sc_bvalid_END (9)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_bvalid_sel_START (10)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_bvalid_sel_END (10)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_sc_powerpresent_START (11)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_sc_powerpresent_END (11)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_powerpresent_sel_START (12)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_powerpresent_sel_END (12)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_sc_sessvld_START (13)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_sc_sessvld_END (13)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_sessvld_sel_START (14)
#define SOC_USB3OTG_BC_USBOTG3_CTRL0_sessvld_sel_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int host_u2_port_disable : 1;
        unsigned int host_u3_port_disable : 1;
        unsigned int hub_port_overcurrent : 2;
        unsigned int hub_port_perm_attach : 2;
        unsigned int host_port_power_control_present : 1;
        unsigned int xhc_bme : 1;
        unsigned int bus_filter_bypass : 4;
        unsigned int fladj_30mhz_reg : 6;
        unsigned int reserved : 14;
    } reg;
} SOC_USB3OTG_BC_USBOTG3_CTRL1_UNION;
#endif
#define SOC_USB3OTG_BC_USBOTG3_CTRL1_host_u2_port_disable_START (0)
#define SOC_USB3OTG_BC_USBOTG3_CTRL1_host_u2_port_disable_END (0)
#define SOC_USB3OTG_BC_USBOTG3_CTRL1_host_u3_port_disable_START (1)
#define SOC_USB3OTG_BC_USBOTG3_CTRL1_host_u3_port_disable_END (1)
#define SOC_USB3OTG_BC_USBOTG3_CTRL1_hub_port_overcurrent_START (2)
#define SOC_USB3OTG_BC_USBOTG3_CTRL1_hub_port_overcurrent_END (3)
#define SOC_USB3OTG_BC_USBOTG3_CTRL1_hub_port_perm_attach_START (4)
#define SOC_USB3OTG_BC_USBOTG3_CTRL1_hub_port_perm_attach_END (5)
#define SOC_USB3OTG_BC_USBOTG3_CTRL1_host_port_power_control_present_START (6)
#define SOC_USB3OTG_BC_USBOTG3_CTRL1_host_port_power_control_present_END (6)
#define SOC_USB3OTG_BC_USBOTG3_CTRL1_xhc_bme_START (7)
#define SOC_USB3OTG_BC_USBOTG3_CTRL1_xhc_bme_END (7)
#define SOC_USB3OTG_BC_USBOTG3_CTRL1_bus_filter_bypass_START (8)
#define SOC_USB3OTG_BC_USBOTG3_CTRL1_bus_filter_bypass_END (11)
#define SOC_USB3OTG_BC_USBOTG3_CTRL1_fladj_30mhz_reg_START (12)
#define SOC_USB3OTG_BC_USBOTG3_CTRL1_fladj_30mhz_reg_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int test_powerdown_hsp : 1;
        unsigned int test_powerdown_ssp : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_USB3OTG_BC_USBOTG3_CTRL2_UNION;
#endif
#define SOC_USB3OTG_BC_USBOTG3_CTRL2_test_powerdown_hsp_START (0)
#define SOC_USB3OTG_BC_USBOTG3_CTRL2_test_powerdown_hsp_END (0)
#define SOC_USB3OTG_BC_USBOTG3_CTRL2_test_powerdown_ssp_START (1)
#define SOC_USB3OTG_BC_USBOTG3_CTRL2_test_powerdown_ssp_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int commononn : 1;
        unsigned int otgdisable : 1;
        unsigned int retenablen : 1;
        unsigned int txbitstuffen : 1;
        unsigned int txbitstuffehn : 1;
        unsigned int vbusvldextsel : 1;
        unsigned int vbusvldext : 1;
        unsigned int reserved : 25;
    } reg;
} SOC_USB3OTG_BC_USBOTG3_CTRL3_UNION;
#endif
#define SOC_USB3OTG_BC_USBOTG3_CTRL3_commononn_START (0)
#define SOC_USB3OTG_BC_USBOTG3_CTRL3_commononn_END (0)
#define SOC_USB3OTG_BC_USBOTG3_CTRL3_otgdisable_START (1)
#define SOC_USB3OTG_BC_USBOTG3_CTRL3_otgdisable_END (1)
#define SOC_USB3OTG_BC_USBOTG3_CTRL3_retenablen_START (2)
#define SOC_USB3OTG_BC_USBOTG3_CTRL3_retenablen_END (2)
#define SOC_USB3OTG_BC_USBOTG3_CTRL3_txbitstuffen_START (3)
#define SOC_USB3OTG_BC_USBOTG3_CTRL3_txbitstuffen_END (3)
#define SOC_USB3OTG_BC_USBOTG3_CTRL3_txbitstuffehn_START (4)
#define SOC_USB3OTG_BC_USBOTG3_CTRL3_txbitstuffehn_END (4)
#define SOC_USB3OTG_BC_USBOTG3_CTRL3_vbusvldextsel_START (5)
#define SOC_USB3OTG_BC_USBOTG3_CTRL3_vbusvldextsel_END (5)
#define SOC_USB3OTG_BC_USBOTG3_CTRL3_vbusvldext_START (6)
#define SOC_USB3OTG_BC_USBOTG3_CTRL3_vbusvldext_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int compdistune : 3;
        unsigned int otgtune : 3;
        unsigned int sqrxtune : 3;
        unsigned int txfslstune : 4;
        unsigned int txhsxvtune : 2;
        unsigned int txpreempamptune : 2;
        unsigned int txpreemppulsetune : 1;
        unsigned int txrestune : 2;
        unsigned int txrisetune : 2;
        unsigned int txvreftune : 4;
        unsigned int reserved : 6;
    } reg;
} SOC_USB3OTG_BC_USBOTG3_CTRL4_UNION;
#endif
#define SOC_USB3OTG_BC_USBOTG3_CTRL4_compdistune_START (0)
#define SOC_USB3OTG_BC_USBOTG3_CTRL4_compdistune_END (2)
#define SOC_USB3OTG_BC_USBOTG3_CTRL4_otgtune_START (3)
#define SOC_USB3OTG_BC_USBOTG3_CTRL4_otgtune_END (5)
#define SOC_USB3OTG_BC_USBOTG3_CTRL4_sqrxtune_START (6)
#define SOC_USB3OTG_BC_USBOTG3_CTRL4_sqrxtune_END (8)
#define SOC_USB3OTG_BC_USBOTG3_CTRL4_txfslstune_START (9)
#define SOC_USB3OTG_BC_USBOTG3_CTRL4_txfslstune_END (12)
#define SOC_USB3OTG_BC_USBOTG3_CTRL4_txhsxvtune_START (13)
#define SOC_USB3OTG_BC_USBOTG3_CTRL4_txhsxvtune_END (14)
#define SOC_USB3OTG_BC_USBOTG3_CTRL4_txpreempamptune_START (15)
#define SOC_USB3OTG_BC_USBOTG3_CTRL4_txpreempamptune_END (16)
#define SOC_USB3OTG_BC_USBOTG3_CTRL4_txpreemppulsetune_START (17)
#define SOC_USB3OTG_BC_USBOTG3_CTRL4_txpreemppulsetune_END (17)
#define SOC_USB3OTG_BC_USBOTG3_CTRL4_txrestune_START (18)
#define SOC_USB3OTG_BC_USBOTG3_CTRL4_txrestune_END (19)
#define SOC_USB3OTG_BC_USBOTG3_CTRL4_txrisetune_START (20)
#define SOC_USB3OTG_BC_USBOTG3_CTRL4_txrisetune_END (21)
#define SOC_USB3OTG_BC_USBOTG3_CTRL4_txvreftune_START (22)
#define SOC_USB3OTG_BC_USBOTG3_CTRL4_txvreftune_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int los_bias : 3;
        unsigned int rx_los_mask_val : 10;
        unsigned int tx_deemph_3p5db : 6;
        unsigned int tx_deemph_6db : 6;
        unsigned int tx_swing_full : 7;
    } reg;
} SOC_USB3OTG_BC_USBOTG3_CTRL5_UNION;
#endif
#define SOC_USB3OTG_BC_USBOTG3_CTRL5_los_bias_START (0)
#define SOC_USB3OTG_BC_USBOTG3_CTRL5_los_bias_END (2)
#define SOC_USB3OTG_BC_USBOTG3_CTRL5_rx_los_mask_val_START (3)
#define SOC_USB3OTG_BC_USBOTG3_CTRL5_rx_los_mask_val_END (12)
#define SOC_USB3OTG_BC_USBOTG3_CTRL5_tx_deemph_3p5db_START (13)
#define SOC_USB3OTG_BC_USBOTG3_CTRL5_tx_deemph_3p5db_END (18)
#define SOC_USB3OTG_BC_USBOTG3_CTRL5_tx_deemph_6db_START (19)
#define SOC_USB3OTG_BC_USBOTG3_CTRL5_tx_deemph_6db_END (24)
#define SOC_USB3OTG_BC_USBOTG3_CTRL5_tx_swing_full_START (25)
#define SOC_USB3OTG_BC_USBOTG3_CTRL5_tx_swing_full_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tx_vboost_lvl : 3;
        unsigned int testpoint_en : 1;
        unsigned int refclk_mux_sel : 1;
        unsigned int testpoint_sel : 5;
        unsigned int hsxcvrextctl0 : 1;
        unsigned int reserved : 21;
    } reg;
} SOC_USB3OTG_BC_USBOTG3_CTRL6_UNION;
#endif
#define SOC_USB3OTG_BC_USBOTG3_CTRL6_tx_vboost_lvl_START (0)
#define SOC_USB3OTG_BC_USBOTG3_CTRL6_tx_vboost_lvl_END (2)
#define SOC_USB3OTG_BC_USBOTG3_CTRL6_testpoint_en_START (3)
#define SOC_USB3OTG_BC_USBOTG3_CTRL6_testpoint_en_END (3)
#define SOC_USB3OTG_BC_USBOTG3_CTRL6_refclk_mux_sel_START (4)
#define SOC_USB3OTG_BC_USBOTG3_CTRL6_refclk_mux_sel_END (4)
#define SOC_USB3OTG_BC_USBOTG3_CTRL6_testpoint_sel_START (5)
#define SOC_USB3OTG_BC_USBOTG3_CTRL6_testpoint_sel_END (9)
#define SOC_USB3OTG_BC_USBOTG3_CTRL6_hsxcvrextctl0_START (10)
#define SOC_USB3OTG_BC_USBOTG3_CTRL6_hsxcvrextctl0_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int refclksel : 2;
        unsigned int fsel : 6;
        unsigned int mpll_multiplier : 7;
        unsigned int ref_clkdiv2 : 1;
        unsigned int ref_ssp_en : 1;
        unsigned int ssc_en : 1;
        unsigned int ssc_range : 3;
        unsigned int ssc_ref_clk_sel : 9;
        unsigned int lane0_ext_pclk_req : 1;
        unsigned int lane0_tx2rx_loopbk : 1;
    } reg;
} SOC_USB3OTG_BC_USBOTG3_CTRL7_UNION;
#endif
#define SOC_USB3OTG_BC_USBOTG3_CTRL7_refclksel_START (0)
#define SOC_USB3OTG_BC_USBOTG3_CTRL7_refclksel_END (1)
#define SOC_USB3OTG_BC_USBOTG3_CTRL7_fsel_START (2)
#define SOC_USB3OTG_BC_USBOTG3_CTRL7_fsel_END (7)
#define SOC_USB3OTG_BC_USBOTG3_CTRL7_mpll_multiplier_START (8)
#define SOC_USB3OTG_BC_USBOTG3_CTRL7_mpll_multiplier_END (14)
#define SOC_USB3OTG_BC_USBOTG3_CTRL7_ref_clkdiv2_START (15)
#define SOC_USB3OTG_BC_USBOTG3_CTRL7_ref_clkdiv2_END (15)
#define SOC_USB3OTG_BC_USBOTG3_CTRL7_ref_ssp_en_START (16)
#define SOC_USB3OTG_BC_USBOTG3_CTRL7_ref_ssp_en_END (16)
#define SOC_USB3OTG_BC_USBOTG3_CTRL7_ssc_en_START (17)
#define SOC_USB3OTG_BC_USBOTG3_CTRL7_ssc_en_END (17)
#define SOC_USB3OTG_BC_USBOTG3_CTRL7_ssc_range_START (18)
#define SOC_USB3OTG_BC_USBOTG3_CTRL7_ssc_range_END (20)
#define SOC_USB3OTG_BC_USBOTG3_CTRL7_ssc_ref_clk_sel_START (21)
#define SOC_USB3OTG_BC_USBOTG3_CTRL7_ssc_ref_clk_sel_END (29)
#define SOC_USB3OTG_BC_USBOTG3_CTRL7_lane0_ext_pclk_req_START (30)
#define SOC_USB3OTG_BC_USBOTG3_CTRL7_lane0_ext_pclk_req_END (30)
#define SOC_USB3OTG_BC_USBOTG3_CTRL7_lane0_tx2rx_loopbk_START (31)
#define SOC_USB3OTG_BC_USBOTG3_CTRL7_lane0_tx2rx_loopbk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int debug : 32;
    } reg;
} SOC_USB3OTG_BC_USBOTG3_STS0_UNION;
#endif
#define SOC_USB3OTG_BC_USBOTG3_STS0_debug_START (0)
#define SOC_USB3OTG_BC_USBOTG3_STS0_debug_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int debugh : 32;
    } reg;
} SOC_USB3OTG_BC_USBOTG3_STS1_UNION;
#endif
#define SOC_USB3OTG_BC_USBOTG3_STS1_debugh_START (0)
#define SOC_USB3OTG_BC_USBOTG3_STS1_debugh_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int logic_analyzer_trace : 32;
    } reg;
} SOC_USB3OTG_BC_USBOTG3_STS2_UNION;
#endif
#define SOC_USB3OTG_BC_USBOTG3_STS2_logic_analyzer_trace_START (0)
#define SOC_USB3OTG_BC_USBOTG3_STS2_logic_analyzer_trace_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int logic_analyzer_traceh : 32;
    } reg;
} SOC_USB3OTG_BC_USBOTG3_STS3_UNION;
#endif
#define SOC_USB3OTG_BC_USBOTG3_STS3_logic_analyzer_traceh_START (0)
#define SOC_USB3OTG_BC_USBOTG3_STS3_logic_analyzer_traceh_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bc_opmode : 2;
        unsigned int bc_xcvrselect : 2;
        unsigned int bc_termselect : 1;
        unsigned int bc_txvalid : 1;
        unsigned int bc_txvalidh : 1;
        unsigned int bc_dppulldown : 1;
        unsigned int bc_dmpulldown : 1;
        unsigned int bc_suspend_n : 1;
        unsigned int bc_idpullup : 1;
        unsigned int reserved : 21;
    } reg;
} SOC_USB3OTG_BC_BC_CTRL0_UNION;
#endif
#define SOC_USB3OTG_BC_BC_CTRL0_bc_opmode_START (0)
#define SOC_USB3OTG_BC_BC_CTRL0_bc_opmode_END (1)
#define SOC_USB3OTG_BC_BC_CTRL0_bc_xcvrselect_START (2)
#define SOC_USB3OTG_BC_BC_CTRL0_bc_xcvrselect_END (3)
#define SOC_USB3OTG_BC_BC_CTRL0_bc_termselect_START (4)
#define SOC_USB3OTG_BC_BC_CTRL0_bc_termselect_END (4)
#define SOC_USB3OTG_BC_BC_CTRL0_bc_txvalid_START (5)
#define SOC_USB3OTG_BC_BC_CTRL0_bc_txvalid_END (5)
#define SOC_USB3OTG_BC_BC_CTRL0_bc_txvalidh_START (6)
#define SOC_USB3OTG_BC_BC_CTRL0_bc_txvalidh_END (6)
#define SOC_USB3OTG_BC_BC_CTRL0_bc_dppulldown_START (7)
#define SOC_USB3OTG_BC_BC_CTRL0_bc_dppulldown_END (7)
#define SOC_USB3OTG_BC_BC_CTRL0_bc_dmpulldown_START (8)
#define SOC_USB3OTG_BC_BC_CTRL0_bc_dmpulldown_END (8)
#define SOC_USB3OTG_BC_BC_CTRL0_bc_suspend_n_START (9)
#define SOC_USB3OTG_BC_BC_CTRL0_bc_suspend_n_END (9)
#define SOC_USB3OTG_BC_BC_CTRL0_bc_idpullup_START (10)
#define SOC_USB3OTG_BC_BC_CTRL0_bc_idpullup_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bc_mode : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_USB3OTG_BC_BC_CTRL1_UNION;
#endif
#define SOC_USB3OTG_BC_BC_CTRL1_bc_mode_START (0)
#define SOC_USB3OTG_BC_BC_CTRL1_bc_mode_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bc_phy_acaenb : 1;
        unsigned int bc_phy_dcdenb : 1;
        unsigned int bc_phy_chrgsel : 1;
        unsigned int bc_phy_vdatarcenb : 1;
        unsigned int bc_phy_vdatdetenb : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_USB3OTG_BC_BC_CTRL2_UNION;
#endif
#define SOC_USB3OTG_BC_BC_CTRL2_bc_phy_acaenb_START (0)
#define SOC_USB3OTG_BC_BC_CTRL2_bc_phy_acaenb_END (0)
#define SOC_USB3OTG_BC_BC_CTRL2_bc_phy_dcdenb_START (1)
#define SOC_USB3OTG_BC_BC_CTRL2_bc_phy_dcdenb_END (1)
#define SOC_USB3OTG_BC_BC_CTRL2_bc_phy_chrgsel_START (2)
#define SOC_USB3OTG_BC_BC_CTRL2_bc_phy_chrgsel_END (2)
#define SOC_USB3OTG_BC_BC_CTRL2_bc_phy_vdatarcenb_START (3)
#define SOC_USB3OTG_BC_BC_CTRL2_bc_phy_vdatarcenb_END (3)
#define SOC_USB3OTG_BC_BC_CTRL2_bc_phy_vdatdetenb_START (4)
#define SOC_USB3OTG_BC_BC_CTRL2_bc_phy_vdatdetenb_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bc_sessvld : 1;
        unsigned int bc_rid_a : 1;
        unsigned int bc_rid_b : 1;
        unsigned int bc_rid_c : 1;
        unsigned int bc_rid_float : 1;
        unsigned int bc_rid_gnd : 1;
        unsigned int bc_phy_fsvplus : 1;
        unsigned int bc_phy_fsvminus : 1;
        unsigned int bc_phy_chgdet : 1;
        unsigned int bc_linestate : 2;
        unsigned int reserved : 21;
    } reg;
} SOC_USB3OTG_BC_BC_STS0_UNION;
#endif
#define SOC_USB3OTG_BC_BC_STS0_bc_sessvld_START (0)
#define SOC_USB3OTG_BC_BC_STS0_bc_sessvld_END (0)
#define SOC_USB3OTG_BC_BC_STS0_bc_rid_a_START (1)
#define SOC_USB3OTG_BC_BC_STS0_bc_rid_a_END (1)
#define SOC_USB3OTG_BC_BC_STS0_bc_rid_b_START (2)
#define SOC_USB3OTG_BC_BC_STS0_bc_rid_b_END (2)
#define SOC_USB3OTG_BC_BC_STS0_bc_rid_c_START (3)
#define SOC_USB3OTG_BC_BC_STS0_bc_rid_c_END (3)
#define SOC_USB3OTG_BC_BC_STS0_bc_rid_float_START (4)
#define SOC_USB3OTG_BC_BC_STS0_bc_rid_float_END (4)
#define SOC_USB3OTG_BC_BC_STS0_bc_rid_gnd_START (5)
#define SOC_USB3OTG_BC_BC_STS0_bc_rid_gnd_END (5)
#define SOC_USB3OTG_BC_BC_STS0_bc_phy_fsvplus_START (6)
#define SOC_USB3OTG_BC_BC_STS0_bc_phy_fsvplus_END (6)
#define SOC_USB3OTG_BC_BC_STS0_bc_phy_fsvminus_START (7)
#define SOC_USB3OTG_BC_BC_STS0_bc_phy_fsvminus_END (7)
#define SOC_USB3OTG_BC_BC_STS0_bc_phy_chgdet_START (8)
#define SOC_USB3OTG_BC_BC_STS0_bc_phy_chgdet_END (8)
#define SOC_USB3OTG_BC_BC_STS0_bc_linestate_START (9)
#define SOC_USB3OTG_BC_BC_STS0_bc_linestate_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ram0_slp : 1;
        unsigned int ram1_slp : 1;
        unsigned int ram2_slp : 1;
        unsigned int reserved : 29;
    } reg;
} SOC_USB3OTG_BC_RAM_CTRL_UNION;
#endif
#define SOC_USB3OTG_BC_RAM_CTRL_ram0_slp_START (0)
#define SOC_USB3OTG_BC_RAM_CTRL_ram0_slp_END (0)
#define SOC_USB3OTG_BC_RAM_CTRL_ram1_slp_START (1)
#define SOC_USB3OTG_BC_RAM_CTRL_ram1_slp_END (1)
#define SOC_USB3OTG_BC_RAM_CTRL_ram2_slp_START (2)
#define SOC_USB3OTG_BC_RAM_CTRL_ram2_slp_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bc_chirp_on : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_USB3OTG_BC_USBOTG3_STS4_UNION;
#endif
#define SOC_USB3OTG_BC_USBOTG3_STS4_bc_chirp_on_START (0)
#define SOC_USB3OTG_BC_USBOTG3_STS4_bc_chirp_on_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb3otg_phy_atereset : 1;
        unsigned int usb3otg_phy_autorsmenb : 1;
        unsigned int usb3otg_phy_loopbackenb : 1;
        unsigned int usb3otg_phy_vatestenb : 1;
        unsigned int usb3otg_phy_vdatreftune : 2;
        unsigned int usb3otg_phy_alt_clk_en : 1;
        unsigned int usb3otg_phy_alt_clk_sel : 1;
        unsigned int usb3otg_phy_lane0_tx_term_offset : 5;
        unsigned int usb3otg_phy_los_level : 5;
        unsigned int usb3otg_phy_mpll_refssc_clk_en : 1;
        unsigned int usb3otg_phy_ref_use_pad : 1;
        unsigned int usb3otg_phy_ref_use_xo : 1;
        unsigned int usb3otg_phy_ref_xo_en : 1;
        unsigned int usb3otg_phy_rtune_req : 1;
        unsigned int usb3otg_phy_test_burnin : 1;
        unsigned int reserved : 8;
    } reg;
} SOC_USB3OTG_BC_USB3PHY_CTRL_UNION;
#endif
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_atereset_START (0)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_atereset_END (0)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_autorsmenb_START (1)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_autorsmenb_END (1)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_loopbackenb_START (2)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_loopbackenb_END (2)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_vatestenb_START (3)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_vatestenb_END (3)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_vdatreftune_START (4)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_vdatreftune_END (5)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_alt_clk_en_START (6)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_alt_clk_en_END (6)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_alt_clk_sel_START (7)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_alt_clk_sel_END (7)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_lane0_tx_term_offset_START (8)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_lane0_tx_term_offset_END (12)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_los_level_START (13)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_los_level_END (17)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_mpll_refssc_clk_en_START (18)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_mpll_refssc_clk_en_END (18)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_ref_use_pad_START (19)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_ref_use_pad_END (19)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_ref_use_xo_START (20)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_ref_use_xo_END (20)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_ref_xo_en_START (21)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_ref_xo_en_END (21)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_rtune_req_START (22)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_rtune_req_END (22)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_test_burnin_START (23)
#define SOC_USB3OTG_BC_USB3PHY_CTRL_usb3otg_phy_test_burnin_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb3otg_phy_ref_clkreq_n : 1;
        unsigned int usb3otg_phy_rtune_ack : 1;
        unsigned int hssquelch0 : 1;
        unsigned int hsrxdat0 : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_USB3OTG_BC_USB3PHY_STS_UNION;
#endif
#define SOC_USB3OTG_BC_USB3PHY_STS_usb3otg_phy_ref_clkreq_n_START (0)
#define SOC_USB3OTG_BC_USB3PHY_STS_usb3otg_phy_ref_clkreq_n_END (0)
#define SOC_USB3OTG_BC_USB3PHY_STS_usb3otg_phy_rtune_ack_START (1)
#define SOC_USB3OTG_BC_USB3PHY_STS_usb3otg_phy_rtune_ack_END (1)
#define SOC_USB3OTG_BC_USB3PHY_STS_hssquelch0_START (2)
#define SOC_USB3OTG_BC_USB3PHY_STS_hssquelch0_END (2)
#define SOC_USB3OTG_BC_USB3PHY_STS_hsrxdat0_START (3)
#define SOC_USB3OTG_BC_USB3PHY_STS_hsrxdat0_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb3otg_phy_cr_ack : 1;
        unsigned int usb3otg_phy_cr_data_out : 16;
        unsigned int reserved : 15;
    } reg;
} SOC_USB3OTG_BC_USB3PHY_CR_STS_UNION;
#endif
#define SOC_USB3OTG_BC_USB3PHY_CR_STS_usb3otg_phy_cr_ack_START (0)
#define SOC_USB3OTG_BC_USB3PHY_CR_STS_usb3otg_phy_cr_ack_END (0)
#define SOC_USB3OTG_BC_USB3PHY_CR_STS_usb3otg_phy_cr_data_out_START (1)
#define SOC_USB3OTG_BC_USB3PHY_CR_STS_usb3otg_phy_cr_data_out_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb3otg_phy_cr_cap_addr : 1;
        unsigned int usb3otg_phy_cr_cap_data : 1;
        unsigned int usb3otg_phy_cr_read : 1;
        unsigned int usb3otg_phy_cr_write : 1;
        unsigned int usb3otg_phy_cr_data_in : 16;
        unsigned int reserved : 12;
    } reg;
} SOC_USB3OTG_BC_USB3PHY_CR_CTRL_UNION;
#endif
#define SOC_USB3OTG_BC_USB3PHY_CR_CTRL_usb3otg_phy_cr_cap_addr_START (0)
#define SOC_USB3OTG_BC_USB3PHY_CR_CTRL_usb3otg_phy_cr_cap_addr_END (0)
#define SOC_USB3OTG_BC_USB3PHY_CR_CTRL_usb3otg_phy_cr_cap_data_START (1)
#define SOC_USB3OTG_BC_USB3PHY_CR_CTRL_usb3otg_phy_cr_cap_data_END (1)
#define SOC_USB3OTG_BC_USB3PHY_CR_CTRL_usb3otg_phy_cr_read_START (2)
#define SOC_USB3OTG_BC_USB3PHY_CR_CTRL_usb3otg_phy_cr_read_END (2)
#define SOC_USB3OTG_BC_USB3PHY_CR_CTRL_usb3otg_phy_cr_write_START (3)
#define SOC_USB3OTG_BC_USB3PHY_CR_CTRL_usb3otg_phy_cr_write_END (3)
#define SOC_USB3OTG_BC_USB3PHY_CR_CTRL_usb3otg_phy_cr_data_in_START (4)
#define SOC_USB3OTG_BC_USB3PHY_CR_CTRL_usb3otg_phy_cr_data_in_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb3_res : 32;
    } reg;
} SOC_USB3OTG_BC_USB3_RES_UNION;
#endif
#define SOC_USB3OTG_BC_USB3_RES_usb3_res_START (0)
#define SOC_USB3OTG_BC_USB3_RES_usb3_res_END (31)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
