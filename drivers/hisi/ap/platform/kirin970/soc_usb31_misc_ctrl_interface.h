#ifndef __SOC_USB31_MISC_CTRL_INTERFACE_H__
#define __SOC_USB31_MISC_CTRL_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_ADDR(base) ((base) + (0x00))
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL1_ADDR(base) ((base) + (0x04))
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL2_ADDR(base) ((base) + (0x08))
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL3_ADDR(base) ((base) + (0x0C))
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL4_ADDR(base) ((base) + (0x10))
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL5_ADDR(base) ((base) + (0x14))
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL6_ADDR(base) ((base) + (0x18))
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_ADDR(base) ((base) + (0x1C))
#define SOC_USB31_MISC_CTRL_USBOTG3_STS0_ADDR(base) ((base) + (0x20))
#define SOC_USB31_MISC_CTRL_USBOTG3_STS1_ADDR(base) ((base) + (0x24))
#define SOC_USB31_MISC_CTRL_USBOTG3_STS2_ADDR(base) ((base) + (0x28))
#define SOC_USB31_MISC_CTRL_USBOTG3_STS3_ADDR(base) ((base) + (0x2C))
#define SOC_USB31_MISC_CTRL_BC_CTRL0_ADDR(base) ((base) + (0x30))
#define SOC_USB31_MISC_CTRL_BC_CTRL1_ADDR(base) ((base) + (0x34))
#define SOC_USB31_MISC_CTRL_BC_CTRL2_ADDR(base) ((base) + (0x38))
#define SOC_USB31_MISC_CTRL_BC_STS0_ADDR(base) ((base) + (0x3C))
#define SOC_USB31_MISC_CTRL_RAM_CTRL_ADDR(base) ((base) + (0x40))
#define SOC_USB31_MISC_CTRL_USBOTG3_STS4_ADDR(base) ((base) + (0x44))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG48_ADDR(base) ((base) + (0x48))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_ADDR(base) ((base) + (0x4C))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG50_ADDR(base) ((base) + (0x50))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_ADDR(base) ((base) + (0x54))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG58_ADDR(base) ((base) + (0x58))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_ADDR(base) ((base) + (0x5C))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG60_ADDR(base) ((base) + (0x60))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_ADDR(base) ((base) + (0x64))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG68_ADDR(base) ((base) + (0x68))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG6C_ADDR(base) ((base) + (0x6C))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG70_ADDR(base) ((base) + (0x70))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG74_ADDR(base) ((base) + (0x74))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG78_ADDR(base) ((base) + (0x78))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG7C_ADDR(base) ((base) + (0x7C))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG80_ADDR(base) ((base) + (0x80))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG84_ADDR(base) ((base) + (0x84))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG88_ADDR(base) ((base) + (0x88))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG8C_ADDR(base) ((base) + (0x8C))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG90_ADDR(base) ((base) + (0x90))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG94_ADDR(base) ((base) + (0x94))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG98_ADDR(base) ((base) + (0x98))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG9C_ADDR(base) ((base) + (0x9C))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_ADDR(base) ((base) + (0xA0))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA4_ADDR(base) ((base) + (0xA4))
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA8_ADDR(base) ((base) + (0xA8))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int usb3_idpullup : 1;
        unsigned int usb3_iddig_sel : 1;
        unsigned int usb3_iddig : 1;
        unsigned int usb3_drvvbus_sel : 1;
        unsigned int usb3_drvvbus : 1;
        unsigned int usb3_vbusvld_sel : 1;
        unsigned int usb3_vbusvld : 1;
        unsigned int reserved_1 : 2;
        unsigned int usb3_bvalid : 1;
        unsigned int usb3_bvalid_sel : 1;
        unsigned int usb3_powerpreset : 1;
        unsigned int usb3_powerpresent_sel : 1;
        unsigned int usb3_sessvld : 1;
        unsigned int usb3_sessvld_sel : 1;
        unsigned int reserved_2 : 16;
    } reg;
} SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_idpullup_START (1)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_idpullup_END (1)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_iddig_sel_START (2)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_iddig_sel_END (2)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_iddig_START (3)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_iddig_END (3)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_drvvbus_sel_START (4)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_drvvbus_sel_END (4)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_drvvbus_START (5)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_drvvbus_END (5)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_vbusvld_sel_START (6)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_vbusvld_sel_END (6)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_vbusvld_START (7)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_vbusvld_END (7)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_bvalid_START (10)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_bvalid_END (10)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_bvalid_sel_START (11)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_bvalid_sel_END (11)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_powerpreset_START (12)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_powerpreset_END (12)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_powerpresent_sel_START (13)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_powerpresent_sel_END (13)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_sessvld_START (14)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_sessvld_END (14)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_sessvld_sel_START (15)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL0_usb3_sessvld_sel_END (15)
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
} SOC_USB31_MISC_CTRL_USBOTG3_CTRL1_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL1_host_u2_port_disable_START (0)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL1_host_u2_port_disable_END (0)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL1_host_u3_port_disable_START (1)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL1_host_u3_port_disable_END (1)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL1_hub_port_overcurrent_START (2)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL1_hub_port_overcurrent_END (3)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL1_hub_port_perm_attach_START (4)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL1_hub_port_perm_attach_END (5)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL1_host_port_power_control_present_START (6)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL1_host_port_power_control_present_END (6)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL1_xhc_bme_START (7)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL1_xhc_bme_END (7)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL1_bus_filter_bypass_START (8)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL1_bus_filter_bypass_END (11)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL1_fladj_30mhz_reg_START (12)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL1_fladj_30mhz_reg_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_USB31_MISC_CTRL_USBOTG3_CTRL2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb2_commononn : 1;
        unsigned int usb2_otgdisable0 : 1;
        unsigned int usb2_retenablen : 1;
        unsigned int usb2_txbitstuffen0 : 1;
        unsigned int usb2_txbitstuffenh0 : 1;
        unsigned int usb2_vbusvldextsel0 : 1;
        unsigned int usb2_vbusvldext0 : 1;
        unsigned int reserved : 25;
    } reg;
} SOC_USB31_MISC_CTRL_USBOTG3_CTRL3_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL3_usb2_commononn_START (0)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL3_usb2_commononn_END (0)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL3_usb2_otgdisable0_START (1)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL3_usb2_otgdisable0_END (1)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL3_usb2_retenablen_START (2)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL3_usb2_retenablen_END (2)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL3_usb2_txbitstuffen0_START (3)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL3_usb2_txbitstuffen0_END (3)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL3_usb2_txbitstuffenh0_START (4)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL3_usb2_txbitstuffenh0_END (4)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL3_usb2_vbusvldextsel0_START (5)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL3_usb2_vbusvldextsel0_END (5)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL3_usb2_vbusvldext0_START (6)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL3_usb2_vbusvldext0_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb2_compdistune0 : 3;
        unsigned int usb2_otgtune0 : 3;
        unsigned int usb2_sqrxtune0 : 3;
        unsigned int usb2_txfslstune0 : 4;
        unsigned int usb2_txhsxvtune0 : 2;
        unsigned int usb2_txpreempamptune0 : 2;
        unsigned int usb2_txpreemppulsetune0 : 1;
        unsigned int usb2_txrestune0 : 2;
        unsigned int usb2_txrisetune0 : 2;
        unsigned int usb2_txvreftune0 : 4;
        unsigned int reserved : 6;
    } reg;
} SOC_USB31_MISC_CTRL_USBOTG3_CTRL4_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL4_usb2_compdistune0_START (0)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL4_usb2_compdistune0_END (2)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL4_usb2_otgtune0_START (3)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL4_usb2_otgtune0_END (5)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL4_usb2_sqrxtune0_START (6)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL4_usb2_sqrxtune0_END (8)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL4_usb2_txfslstune0_START (9)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL4_usb2_txfslstune0_END (12)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL4_usb2_txhsxvtune0_START (13)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL4_usb2_txhsxvtune0_END (14)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL4_usb2_txpreempamptune0_START (15)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL4_usb2_txpreempamptune0_END (16)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL4_usb2_txpreemppulsetune0_START (17)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL4_usb2_txpreemppulsetune0_END (17)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL4_usb2_txrestune0_START (18)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL4_usb2_txrestune0_END (19)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL4_usb2_txrisetune0_START (20)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL4_usb2_txrisetune0_END (21)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL4_usb2_txvreftune0_START (22)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL4_usb2_txvreftune0_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb2_siddq : 1;
        unsigned int usb2_vatestenb : 2;
        unsigned int reserved : 29;
    } reg;
} SOC_USB31_MISC_CTRL_USBOTG3_CTRL5_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL5_usb2_siddq_START (0)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL5_usb2_siddq_END (0)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL5_usb2_vatestenb_START (1)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL5_usb2_vatestenb_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_USB31_MISC_CTRL_USBOTG3_CTRL6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb2_fsel : 3;
        unsigned int usb2_refclksel : 2;
        unsigned int usb2_pllitune : 2;
        unsigned int usb2_pllptune : 4;
        unsigned int usb2_pllbtune : 1;
        unsigned int usb2_vregbypass : 1;
        unsigned int usb2_loopbackenb0 : 1;
        unsigned int usb2_autorsmenb0 : 1;
        unsigned int usb2_wordinterface0 : 1;
        unsigned int usb2_vdatreftune0 : 2;
        unsigned int chrgsrcpuenb0 : 2;
        unsigned int reserved : 12;
    } reg;
} SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_usb2_fsel_START (0)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_usb2_fsel_END (2)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_usb2_refclksel_START (3)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_usb2_refclksel_END (4)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_usb2_pllitune_START (5)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_usb2_pllitune_END (6)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_usb2_pllptune_START (7)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_usb2_pllptune_END (10)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_usb2_pllbtune_START (11)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_usb2_pllbtune_END (11)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_usb2_vregbypass_START (12)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_usb2_vregbypass_END (12)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_usb2_loopbackenb0_START (13)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_usb2_loopbackenb0_END (13)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_usb2_autorsmenb0_START (14)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_usb2_autorsmenb0_END (14)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_usb2_wordinterface0_START (15)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_usb2_wordinterface0_END (15)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_usb2_vdatreftune0_START (16)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_usb2_vdatreftune0_END (17)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_chrgsrcpuenb0_START (18)
#define SOC_USB31_MISC_CTRL_USBOTG3_CTRL7_chrgsrcpuenb0_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int debug : 32;
    } reg;
} SOC_USB31_MISC_CTRL_USBOTG3_STS0_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USBOTG3_STS0_debug_START (0)
#define SOC_USB31_MISC_CTRL_USBOTG3_STS0_debug_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int debugh : 32;
    } reg;
} SOC_USB31_MISC_CTRL_USBOTG3_STS1_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USBOTG3_STS1_debugh_START (0)
#define SOC_USB31_MISC_CTRL_USBOTG3_STS1_debugh_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int logic_analyzer_trace : 32;
    } reg;
} SOC_USB31_MISC_CTRL_USBOTG3_STS2_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USBOTG3_STS2_logic_analyzer_trace_START (0)
#define SOC_USB31_MISC_CTRL_USBOTG3_STS2_logic_analyzer_trace_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int logic_analyzer_traceh : 32;
    } reg;
} SOC_USB31_MISC_CTRL_USBOTG3_STS3_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USBOTG3_STS3_logic_analyzer_traceh_START (0)
#define SOC_USB31_MISC_CTRL_USBOTG3_STS3_logic_analyzer_traceh_END (31)
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
} SOC_USB31_MISC_CTRL_BC_CTRL0_UNION;
#endif
#define SOC_USB31_MISC_CTRL_BC_CTRL0_bc_opmode_START (0)
#define SOC_USB31_MISC_CTRL_BC_CTRL0_bc_opmode_END (1)
#define SOC_USB31_MISC_CTRL_BC_CTRL0_bc_xcvrselect_START (2)
#define SOC_USB31_MISC_CTRL_BC_CTRL0_bc_xcvrselect_END (3)
#define SOC_USB31_MISC_CTRL_BC_CTRL0_bc_termselect_START (4)
#define SOC_USB31_MISC_CTRL_BC_CTRL0_bc_termselect_END (4)
#define SOC_USB31_MISC_CTRL_BC_CTRL0_bc_txvalid_START (5)
#define SOC_USB31_MISC_CTRL_BC_CTRL0_bc_txvalid_END (5)
#define SOC_USB31_MISC_CTRL_BC_CTRL0_bc_txvalidh_START (6)
#define SOC_USB31_MISC_CTRL_BC_CTRL0_bc_txvalidh_END (6)
#define SOC_USB31_MISC_CTRL_BC_CTRL0_bc_dppulldown_START (7)
#define SOC_USB31_MISC_CTRL_BC_CTRL0_bc_dppulldown_END (7)
#define SOC_USB31_MISC_CTRL_BC_CTRL0_bc_dmpulldown_START (8)
#define SOC_USB31_MISC_CTRL_BC_CTRL0_bc_dmpulldown_END (8)
#define SOC_USB31_MISC_CTRL_BC_CTRL0_bc_suspend_n_START (9)
#define SOC_USB31_MISC_CTRL_BC_CTRL0_bc_suspend_n_END (9)
#define SOC_USB31_MISC_CTRL_BC_CTRL0_bc_idpullup_START (10)
#define SOC_USB31_MISC_CTRL_BC_CTRL0_bc_idpullup_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bc_mode : 32;
    } reg;
} SOC_USB31_MISC_CTRL_BC_CTRL1_UNION;
#endif
#define SOC_USB31_MISC_CTRL_BC_CTRL1_bc_mode_START (0)
#define SOC_USB31_MISC_CTRL_BC_CTRL1_bc_mode_END (31)
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
} SOC_USB31_MISC_CTRL_BC_CTRL2_UNION;
#endif
#define SOC_USB31_MISC_CTRL_BC_CTRL2_bc_phy_acaenb_START (0)
#define SOC_USB31_MISC_CTRL_BC_CTRL2_bc_phy_acaenb_END (0)
#define SOC_USB31_MISC_CTRL_BC_CTRL2_bc_phy_dcdenb_START (1)
#define SOC_USB31_MISC_CTRL_BC_CTRL2_bc_phy_dcdenb_END (1)
#define SOC_USB31_MISC_CTRL_BC_CTRL2_bc_phy_chrgsel_START (2)
#define SOC_USB31_MISC_CTRL_BC_CTRL2_bc_phy_chrgsel_END (2)
#define SOC_USB31_MISC_CTRL_BC_CTRL2_bc_phy_vdatarcenb_START (3)
#define SOC_USB31_MISC_CTRL_BC_CTRL2_bc_phy_vdatarcenb_END (3)
#define SOC_USB31_MISC_CTRL_BC_CTRL2_bc_phy_vdatdetenb_START (4)
#define SOC_USB31_MISC_CTRL_BC_CTRL2_bc_phy_vdatdetenb_END (4)
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
} SOC_USB31_MISC_CTRL_BC_STS0_UNION;
#endif
#define SOC_USB31_MISC_CTRL_BC_STS0_bc_sessvld_START (0)
#define SOC_USB31_MISC_CTRL_BC_STS0_bc_sessvld_END (0)
#define SOC_USB31_MISC_CTRL_BC_STS0_bc_rid_a_START (1)
#define SOC_USB31_MISC_CTRL_BC_STS0_bc_rid_a_END (1)
#define SOC_USB31_MISC_CTRL_BC_STS0_bc_rid_b_START (2)
#define SOC_USB31_MISC_CTRL_BC_STS0_bc_rid_b_END (2)
#define SOC_USB31_MISC_CTRL_BC_STS0_bc_rid_c_START (3)
#define SOC_USB31_MISC_CTRL_BC_STS0_bc_rid_c_END (3)
#define SOC_USB31_MISC_CTRL_BC_STS0_bc_rid_float_START (4)
#define SOC_USB31_MISC_CTRL_BC_STS0_bc_rid_float_END (4)
#define SOC_USB31_MISC_CTRL_BC_STS0_bc_rid_gnd_START (5)
#define SOC_USB31_MISC_CTRL_BC_STS0_bc_rid_gnd_END (5)
#define SOC_USB31_MISC_CTRL_BC_STS0_bc_phy_fsvplus_START (6)
#define SOC_USB31_MISC_CTRL_BC_STS0_bc_phy_fsvplus_END (6)
#define SOC_USB31_MISC_CTRL_BC_STS0_bc_phy_fsvminus_START (7)
#define SOC_USB31_MISC_CTRL_BC_STS0_bc_phy_fsvminus_END (7)
#define SOC_USB31_MISC_CTRL_BC_STS0_bc_phy_chgdet_START (8)
#define SOC_USB31_MISC_CTRL_BC_STS0_bc_phy_chgdet_END (8)
#define SOC_USB31_MISC_CTRL_BC_STS0_bc_linestate_START (9)
#define SOC_USB31_MISC_CTRL_BC_STS0_bc_linestate_END (10)
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
} SOC_USB31_MISC_CTRL_RAM_CTRL_UNION;
#endif
#define SOC_USB31_MISC_CTRL_RAM_CTRL_ram0_slp_START (0)
#define SOC_USB31_MISC_CTRL_RAM_CTRL_ram0_slp_END (0)
#define SOC_USB31_MISC_CTRL_RAM_CTRL_ram1_slp_START (1)
#define SOC_USB31_MISC_CTRL_RAM_CTRL_ram1_slp_END (1)
#define SOC_USB31_MISC_CTRL_RAM_CTRL_ram2_slp_START (2)
#define SOC_USB31_MISC_CTRL_RAM_CTRL_ram2_slp_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int host_legacy_smi_interrupt : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_USB31_MISC_CTRL_USBOTG3_STS4_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USBOTG3_STS4_host_legacy_smi_interrupt_START (0)
#define SOC_USB31_MISC_CTRL_USBOTG3_STS4_host_legacy_smi_interrupt_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb3_phy_ext_mplla_handwidth : 16;
        unsigned int usb3_phy_ext_mplla_div_multiplier : 8;
        unsigned int usb3_phy_ext_mplla_ssc_clk_sel : 3;
        unsigned int usb3_phy_ext_ref_clk_div2_en : 1;
        unsigned int reserved : 4;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFG48_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG48_usb3_phy_ext_mplla_handwidth_START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG48_usb3_phy_ext_mplla_handwidth_END (15)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG48_usb3_phy_ext_mplla_div_multiplier_START (16)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG48_usb3_phy_ext_mplla_div_multiplier_END (23)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG48_usb3_phy_ext_mplla_ssc_clk_sel_START (24)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG48_usb3_phy_ext_mplla_ssc_clk_sel_END (26)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG48_usb3_phy_ext_ref_clk_div2_en_START (27)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG48_usb3_phy_ext_ref_clk_div2_en_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb2_testclk0 : 1;
        unsigned int usb2_test_dataoutsel0 : 1;
        unsigned int usb2_testdatain0 : 8;
        unsigned int usb2_testaddr0 : 4;
        unsigned int usb2_test_burnin : 1;
        unsigned int reserved_0 : 1;
        unsigned int usb2_testdataout0 : 4;
        unsigned int usb2_fslsrcv0 : 1;
        unsigned int usb2_fsvplus0 : 1;
        unsigned int usb2_fsvminus0 : 1;
        unsigned int usb2_bypassdpdata0 : 1;
        unsigned int usb2_bypassdmdata0 : 1;
        unsigned int usb2_bypassdpen0 : 1;
        unsigned int usb2_bypassmen0 : 1;
        unsigned int usb2_bypasssel0 : 1;
        unsigned int usb2_hsrxdat0 : 1;
        unsigned int usb2_hssquelch0 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_testclk0_START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_testclk0_END (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_test_dataoutsel0_START (1)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_test_dataoutsel0_END (1)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_testdatain0_START (2)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_testdatain0_END (9)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_testaddr0_START (10)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_testaddr0_END (13)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_test_burnin_START (14)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_test_burnin_END (14)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_testdataout0_START (16)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_testdataout0_END (19)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_fslsrcv0_START (20)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_fslsrcv0_END (20)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_fsvplus0_START (21)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_fsvplus0_END (21)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_fsvminus0_START (22)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_fsvminus0_END (22)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_bypassdpdata0_START (23)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_bypassdpdata0_END (23)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_bypassdmdata0_START (24)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_bypassdmdata0_END (24)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_bypassdpen0_START (25)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_bypassdpen0_END (25)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_bypassmen0_START (26)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_bypassmen0_END (26)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_bypasssel0_START (27)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_bypasssel0_END (27)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_hsrxdat0_START (28)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_hsrxdat0_END (28)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_hssquelch0_START (29)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG4C_usb2_hssquelch0_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 4;
        unsigned int usb3_typec_flip_invert : 1;
        unsigned int usb3_upcs_pipe_config : 16;
        unsigned int usb3_upcs_pwr_stable : 1;
        unsigned int usb3_upcs_pwr_en : 1;
        unsigned int usb3_phy_test_powerdown : 1;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFG50_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG50_usb3_typec_flip_invert_START (4)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG50_usb3_typec_flip_invert_END (4)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG50_usb3_upcs_pipe_config_START (5)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG50_usb3_upcs_pipe_config_END (20)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG50_usb3_upcs_pwr_stable_START (21)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG50_usb3_upcs_pwr_stable_END (21)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG50_usb3_upcs_pwr_en_START (22)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG50_usb3_upcs_pwr_en_END (22)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG50_usb3_phy_test_powerdown_START (23)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG50_usb3_phy_test_powerdown_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb3_ext_pclk_req : 1;
        unsigned int usb3_phy0_ana_pwr_en : 1;
        unsigned int usb3_phy0_cr_para_clk : 1;
        unsigned int usb3_phy0_cr_para_rd_en : 1;
        unsigned int usb3_phy0_cr_para_sel : 1;
        unsigned int usb3_phy0_cr_para_wr_en : 1;
        unsigned int usb3_phy0_ana_pwr_stable : 1;
        unsigned int usb3_phy0_cr_para_ack : 1;
        unsigned int phy0_pcs_pwr_en : 1;
        unsigned int phy0_pcs_pwr_stable : 1;
        unsigned int phy0_pma_pwr_en : 1;
        unsigned int phy0_pma_pwr_stable : 1;
        unsigned int usb3_phy_ref_use_pad : 1;
        unsigned int usb3_phy0_test_stop_clk_en : 1;
        unsigned int usb3_phy0_test_tx_ref_clk_en : 1;
        unsigned int reserved : 1;
        unsigned int usb3_phy0_cr_para_addr : 16;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFG54_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_ext_pclk_req_START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_ext_pclk_req_END (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_phy0_ana_pwr_en_START (1)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_phy0_ana_pwr_en_END (1)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_phy0_cr_para_clk_START (2)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_phy0_cr_para_clk_END (2)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_phy0_cr_para_rd_en_START (3)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_phy0_cr_para_rd_en_END (3)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_phy0_cr_para_sel_START (4)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_phy0_cr_para_sel_END (4)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_phy0_cr_para_wr_en_START (5)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_phy0_cr_para_wr_en_END (5)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_phy0_ana_pwr_stable_START (6)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_phy0_ana_pwr_stable_END (6)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_phy0_cr_para_ack_START (7)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_phy0_cr_para_ack_END (7)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_phy0_pcs_pwr_en_START (8)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_phy0_pcs_pwr_en_END (8)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_phy0_pcs_pwr_stable_START (9)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_phy0_pcs_pwr_stable_END (9)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_phy0_pma_pwr_en_START (10)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_phy0_pma_pwr_en_END (10)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_phy0_pma_pwr_stable_START (11)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_phy0_pma_pwr_stable_END (11)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_phy_ref_use_pad_START (12)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_phy_ref_use_pad_END (12)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_phy0_test_stop_clk_en_START (13)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_phy0_test_stop_clk_en_END (13)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_phy0_test_tx_ref_clk_en_START (14)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_phy0_test_tx_ref_clk_en_END (14)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_phy0_cr_para_addr_START (16)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG54_usb3_phy0_cr_para_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb3_phy0_cr_para_wr_data : 16;
        unsigned int usb3_phy0_cr_para_rd_data : 16;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFG58_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG58_usb3_phy0_cr_para_wr_data_START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG58_usb3_phy0_cr_para_wr_data_END (15)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG58_usb3_phy0_cr_para_rd_data_START (16)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG58_usb3_phy0_cr_para_rd_data_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb3_phy0_ss_mplla_force_en : 1;
        unsigned int usb3_phy0_ss_mplla_ssc_en : 1;
        unsigned int usb3_phy0_sram_bypass : 1;
        unsigned int usb3_phy0_sram_ext_ld_done : 1;
        unsigned int phy_ext_ctrl_sel : 1;
        unsigned int phy_test_burnin : 1;
        unsigned int reserved_0 : 6;
        unsigned int phy0_sram_init_done : 1;
        unsigned int usb3_phy_ext_mplla_div10_clk_en : 1;
        unsigned int usb3_phy_ext_mplla_div16p5_clk_en : 1;
        unsigned int usb3_phy_ext_mplla_div8_clk_en : 1;
        unsigned int usb3_phy_ext_mplla_div_clk_en : 1;
        unsigned int reserved_1 : 7;
        unsigned int usb3_phy_ext_mplla_multiplier : 8;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_usb3_phy0_ss_mplla_force_en_START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_usb3_phy0_ss_mplla_force_en_END (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_usb3_phy0_ss_mplla_ssc_en_START (1)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_usb3_phy0_ss_mplla_ssc_en_END (1)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_usb3_phy0_sram_bypass_START (2)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_usb3_phy0_sram_bypass_END (2)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_usb3_phy0_sram_ext_ld_done_START (3)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_usb3_phy0_sram_ext_ld_done_END (3)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_phy_ext_ctrl_sel_START (4)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_phy_ext_ctrl_sel_END (4)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_phy_test_burnin_START (5)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_phy_test_burnin_END (5)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_phy0_sram_init_done_START (12)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_phy0_sram_init_done_END (12)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_usb3_phy_ext_mplla_div10_clk_en_START (13)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_usb3_phy_ext_mplla_div10_clk_en_END (13)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_usb3_phy_ext_mplla_div16p5_clk_en_START (14)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_usb3_phy_ext_mplla_div16p5_clk_en_END (14)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_usb3_phy_ext_mplla_div8_clk_en_START (15)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_usb3_phy_ext_mplla_div8_clk_en_END (15)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_usb3_phy_ext_mplla_div_clk_en_START (16)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_usb3_phy_ext_mplla_div_clk_en_END (16)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_usb3_phy_ext_mplla_multiplier_START (24)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG5C_usb3_phy_ext_mplla_multiplier_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb3_phy_ext_mplla_fracn_ctrl : 11;
        unsigned int reserved : 1;
        unsigned int usb3_phy_ext_mplla_ssc_range : 3;
        unsigned int usb3_phy_ext_mplla_tx_clk_div : 3;
        unsigned int usb3_phy_ext_ref_clk_mplla_div2_en : 1;
        unsigned int usb3_phy_ext_ref_range : 3;
        unsigned int usb3_phy0_ss_mplla_state : 1;
        unsigned int usb3_pipe_lane0_link_num : 4;
        unsigned int usb3_pipe_lane0_phy_src_sel : 2;
        unsigned int usb3_pipe_lane0_tx2rx_loopbk : 1;
        unsigned int usb3_pipe_lane0_databuswidth : 2;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFG60_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG60_usb3_phy_ext_mplla_fracn_ctrl_START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG60_usb3_phy_ext_mplla_fracn_ctrl_END (10)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG60_usb3_phy_ext_mplla_ssc_range_START (12)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG60_usb3_phy_ext_mplla_ssc_range_END (14)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG60_usb3_phy_ext_mplla_tx_clk_div_START (15)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG60_usb3_phy_ext_mplla_tx_clk_div_END (17)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG60_usb3_phy_ext_ref_clk_mplla_div2_en_START (18)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG60_usb3_phy_ext_ref_clk_mplla_div2_en_END (18)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG60_usb3_phy_ext_ref_range_START (19)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG60_usb3_phy_ext_ref_range_END (21)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG60_usb3_phy0_ss_mplla_state_START (22)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG60_usb3_phy0_ss_mplla_state_END (22)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG60_usb3_pipe_lane0_link_num_START (23)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG60_usb3_pipe_lane0_link_num_END (26)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG60_usb3_pipe_lane0_phy_src_sel_START (27)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG60_usb3_pipe_lane0_phy_src_sel_END (28)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG60_usb3_pipe_lane0_tx2rx_loopbk_START (29)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG60_usb3_pipe_lane0_tx2rx_loopbk_END (29)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG60_usb3_pipe_lane0_databuswidth_START (30)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG60_usb3_pipe_lane0_databuswidth_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb3_phy_ext_rx_adapt_afe_en_g1 : 1;
        unsigned int usb3_phy_ext_rx_adapt_afe_en_g2 : 1;
        unsigned int usb3_phy_ext_rx_adapt_dfe_en_g1 : 1;
        unsigned int usb3_phy_ext_rx_adapt_dfe_en_g2 : 1;
        unsigned int usb3_phy_ext_rx_eq_afe_gain_g1 : 4;
        unsigned int usb3_phy_ext_rx_eq_afe_gain_g2 : 4;
        unsigned int usb3_phy_ext_rx_eq_att_lvl_g1 : 3;
        unsigned int usb3_phy_ext_rx_eq_att_lvl_g2 : 3;
        unsigned int usb3_phy_ext_rx_eq_ctle_boost_g1 : 5;
        unsigned int usb3_phy_ext_rx_eq_ctle_boost_g2 : 5;
        unsigned int usb3_phy_ext_rx_los_lfps_en : 1;
        unsigned int reserved : 3;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFG64_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_usb3_phy_ext_rx_adapt_afe_en_g1_START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_usb3_phy_ext_rx_adapt_afe_en_g1_END (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_usb3_phy_ext_rx_adapt_afe_en_g2_START (1)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_usb3_phy_ext_rx_adapt_afe_en_g2_END (1)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_usb3_phy_ext_rx_adapt_dfe_en_g1_START (2)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_usb3_phy_ext_rx_adapt_dfe_en_g1_END (2)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_usb3_phy_ext_rx_adapt_dfe_en_g2_START (3)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_usb3_phy_ext_rx_adapt_dfe_en_g2_END (3)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_usb3_phy_ext_rx_eq_afe_gain_g1_START (4)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_usb3_phy_ext_rx_eq_afe_gain_g1_END (7)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_usb3_phy_ext_rx_eq_afe_gain_g2_START (8)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_usb3_phy_ext_rx_eq_afe_gain_g2_END (11)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_usb3_phy_ext_rx_eq_att_lvl_g1_START (12)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_usb3_phy_ext_rx_eq_att_lvl_g1_END (14)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_usb3_phy_ext_rx_eq_att_lvl_g2_START (15)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_usb3_phy_ext_rx_eq_att_lvl_g2_END (17)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_usb3_phy_ext_rx_eq_ctle_boost_g1_START (18)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_usb3_phy_ext_rx_eq_ctle_boost_g1_END (22)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_usb3_phy_ext_rx_eq_ctle_boost_g2_START (23)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_usb3_phy_ext_rx_eq_ctle_boost_g2_END (27)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_usb3_phy_ext_rx_los_lfps_en_START (28)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG64_usb3_phy_ext_rx_los_lfps_en_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb3_phy_ext_rx_eq_dfe_tap1_g1 : 8;
        unsigned int usb3_phy_ext_rx_eq_dfe_tap1_g2 : 8;
        unsigned int usb3_phy_ext_rx_los_pwr_up_cnt : 11;
        unsigned int usb3_phy_ext_rx_los_threshold : 3;
        unsigned int reserved : 2;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFG68_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG68_usb3_phy_ext_rx_eq_dfe_tap1_g1_START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG68_usb3_phy_ext_rx_eq_dfe_tap1_g1_END (7)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG68_usb3_phy_ext_rx_eq_dfe_tap1_g2_START (8)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG68_usb3_phy_ext_rx_eq_dfe_tap1_g2_END (15)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG68_usb3_phy_ext_rx_los_pwr_up_cnt_START (16)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG68_usb3_phy_ext_rx_los_pwr_up_cnt_END (26)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG68_usb3_phy_ext_rx_los_threshold_START (27)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG68_usb3_phy_ext_rx_los_threshold_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb3_phy_ext_rx_ref_ld_val_g1 : 6;
        unsigned int usb3_phy_ext_tx_ref_ld_val_g2 : 6;
        unsigned int usb3_phy_ext_rx_vco_ld_val_g1 : 13;
        unsigned int usb3_phy_ext_rx_vref_ctrl : 5;
        unsigned int reserved : 2;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFG6C_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG6C_usb3_phy_ext_rx_ref_ld_val_g1_START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG6C_usb3_phy_ext_rx_ref_ld_val_g1_END (5)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG6C_usb3_phy_ext_tx_ref_ld_val_g2_START (6)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG6C_usb3_phy_ext_tx_ref_ld_val_g2_END (11)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG6C_usb3_phy_ext_rx_vco_ld_val_g1_START (12)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG6C_usb3_phy_ext_rx_vco_ld_val_g1_END (24)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG6C_usb3_phy_ext_rx_vref_ctrl_START (25)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG6C_usb3_phy_ext_rx_vref_ctrl_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb3_phy_ext_rx_vco_ld_val_g2 : 13;
        unsigned int reserved : 11;
        unsigned int usb20_databus_test_rdata : 8;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFG70_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG70_usb3_phy_ext_rx_vco_ld_val_g2_START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG70_usb3_phy_ext_rx_vco_ld_val_g2_END (12)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG70_usb20_databus_test_rdata_START (24)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG70_usb20_databus_test_rdata_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb3_phy_ext_tx_eq_main_g1 : 5;
        unsigned int usb3_phy_ext_tx_eq_main_g2 : 5;
        unsigned int usb3_phy_ext_tx_eq_ovrd_g1 : 1;
        unsigned int usb3_phy_ext_tx_eq_ovrd_g2 : 1;
        unsigned int usb3_phy_ext_tx_eq_post_g1 : 4;
        unsigned int usb3_phy_ext_tx_eq_post_g2 : 4;
        unsigned int usb3_phy_ext_tx_eq_pre_g1 : 4;
        unsigned int usb3_phy_ext_tx_eq_pre_g2 : 4;
        unsigned int usb3_phy_ext_tx_iboost_lvl : 4;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFG74_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG74_usb3_phy_ext_tx_eq_main_g1_START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG74_usb3_phy_ext_tx_eq_main_g1_END (4)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG74_usb3_phy_ext_tx_eq_main_g2_START (5)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG74_usb3_phy_ext_tx_eq_main_g2_END (9)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG74_usb3_phy_ext_tx_eq_ovrd_g1_START (10)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG74_usb3_phy_ext_tx_eq_ovrd_g1_END (10)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG74_usb3_phy_ext_tx_eq_ovrd_g2_START (11)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG74_usb3_phy_ext_tx_eq_ovrd_g2_END (11)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG74_usb3_phy_ext_tx_eq_post_g1_START (12)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG74_usb3_phy_ext_tx_eq_post_g1_END (15)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG74_usb3_phy_ext_tx_eq_post_g2_START (16)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG74_usb3_phy_ext_tx_eq_post_g2_END (19)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG74_usb3_phy_ext_tx_eq_pre_g1_START (20)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG74_usb3_phy_ext_tx_eq_pre_g1_END (23)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG74_usb3_phy_ext_tx_eq_pre_g2_START (24)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG74_usb3_phy_ext_tx_eq_pre_g2_END (27)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG74_usb3_phy_ext_tx_iboost_lvl_START (28)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG74_usb3_phy_ext_tx_iboost_lvl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb3_phy_ext_tx_vboost_lvl : 3;
        unsigned int usb3_phy_lane0_power_present : 1;
        unsigned int reserved_0 : 1;
        unsigned int phy_res_req_in : 1;
        unsigned int phy_rtune_req : 1;
        unsigned int usb3_phy_ss_rx2tx_par_lb_en : 1;
        unsigned int phy_res_ack_out : 1;
        unsigned int phy_res_req_out : 1;
        unsigned int phy_rtune_ack : 1;
        unsigned int reserved_1 : 21;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFG78_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG78_usb3_phy_ext_tx_vboost_lvl_START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG78_usb3_phy_ext_tx_vboost_lvl_END (2)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG78_usb3_phy_lane0_power_present_START (3)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG78_usb3_phy_lane0_power_present_END (3)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG78_phy_res_req_in_START (5)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG78_phy_res_req_in_END (5)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG78_phy_rtune_req_START (6)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG78_phy_rtune_req_END (6)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG78_usb3_phy_ss_rx2tx_par_lb_en_START (7)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG78_usb3_phy_ss_rx2tx_par_lb_en_END (7)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG78_phy_res_ack_out_START (8)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG78_phy_res_ack_out_END (8)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG78_phy_res_req_out_START (9)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG78_phy_res_req_out_END (9)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG78_phy_rtune_ack_START (10)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG78_phy_rtune_ack_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int host_num_u2_port : 4;
        unsigned int host_num_u3_port : 4;
        unsigned int reserved_0 : 1;
        unsigned int startrxdetu3rxdet : 1;
        unsigned int host_msi_enable : 1;
        unsigned int host_legacy_smi_pci_cmd_reg_wr : 1;
        unsigned int host_legacy_smi_bar_wr : 1;
        unsigned int ptm_time_vld : 1;
        unsigned int soc_common_rd_wr_bus : 1;
        unsigned int soc_rd_uf_kb_bandwidth : 15;
        unsigned int host_force_gen1_speed : 1;
        unsigned int reserved_1 : 1;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFG7C_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG7C_host_num_u2_port_START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG7C_host_num_u2_port_END (3)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG7C_host_num_u3_port_START (4)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG7C_host_num_u3_port_END (7)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG7C_startrxdetu3rxdet_START (9)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG7C_startrxdetu3rxdet_END (9)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG7C_host_msi_enable_START (10)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG7C_host_msi_enable_END (10)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG7C_host_legacy_smi_pci_cmd_reg_wr_START (11)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG7C_host_legacy_smi_pci_cmd_reg_wr_END (11)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG7C_host_legacy_smi_bar_wr_START (12)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG7C_host_legacy_smi_bar_wr_END (12)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG7C_ptm_time_vld_START (13)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG7C_ptm_time_vld_END (13)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG7C_soc_common_rd_wr_bus_START (14)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG7C_soc_common_rd_wr_bus_END (14)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG7C_soc_rd_uf_kb_bandwidth_START (15)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG7C_soc_rd_uf_kb_bandwidth_END (29)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG7C_host_force_gen1_speed_START (30)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG7C_host_force_gen1_speed_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ptm_time : 32;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFG80_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG80_ptm_time_START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG80_ptm_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int current_power_state_u2pmu : 2;
        unsigned int pme_generation_u2pmu : 1;
        unsigned int connect_state_u2pmu : 1;
        unsigned int current_power_state_u3pmu : 2;
        unsigned int pme_generation_u3pmu : 1;
        unsigned int connect_state_u3pmu : 1;
        unsigned int sleep : 1;
        unsigned int pme_en : 1;
        unsigned int pm_power_state_request : 2;
        unsigned int reserved_0 : 4;
        unsigned int soc_wr_uf_kb_bandwith : 15;
        unsigned int reserved_1 : 1;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFG84_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG84_current_power_state_u2pmu_START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG84_current_power_state_u2pmu_END (1)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG84_pme_generation_u2pmu_START (2)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG84_pme_generation_u2pmu_END (2)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG84_connect_state_u2pmu_START (3)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG84_connect_state_u2pmu_END (3)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG84_current_power_state_u3pmu_START (4)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG84_current_power_state_u3pmu_END (5)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG84_pme_generation_u3pmu_START (6)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG84_pme_generation_u3pmu_END (6)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG84_connect_state_u3pmu_START (7)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG84_connect_state_u3pmu_END (7)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG84_sleep_START (8)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG84_sleep_END (8)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG84_pme_en_START (9)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG84_pme_en_END (9)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG84_pm_power_state_request_START (10)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG84_pm_power_state_request_END (11)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG84_soc_wr_uf_kb_bandwith_START (16)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG84_soc_wr_uf_kb_bandwith_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pm_pmu_config_strap : 30;
        unsigned int pwr_gate : 1;
        unsigned int reserved : 1;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFG88_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG88_pm_pmu_config_strap_START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG88_pm_pmu_config_strap_END (29)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG88_pwr_gate_START (30)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG88_pwr_gate_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int debug_u3pmu_31_0_ : 32;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFG8C_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG8C_debug_u3pmu_31_0__START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG8C_debug_u3pmu_31_0__END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int debug_u3pmu_high_38_32_ : 8;
        unsigned int reserved_0 : 8;
        unsigned int debug_u2pmu_67_64_ : 4;
        unsigned int reserved_1 : 12;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFG90_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG90_debug_u3pmu_high_38_32__START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG90_debug_u3pmu_high_38_32__END (7)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG90_debug_u2pmu_67_64__START (16)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG90_debug_u2pmu_67_64__END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int debug_u2pmu_31_0_ : 32;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFG94_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG94_debug_u2pmu_31_0__START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG94_debug_u2pmu_31_0__END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFG98_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int debug_u2pmu_63_32_ : 32;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFG9C_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG9C_debug_u2pmu_63_32__START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFG9C_debug_u2pmu_63_32__END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb2phy_por_n : 1;
        unsigned int usb3phy_reset_n : 1;
        unsigned int reserved_0 : 2;
        unsigned int usb2phy_refclk_select : 1;
        unsigned int asp_use_phy : 1;
        unsigned int drv_vbus_sel2 : 2;
        unsigned int usb31c_resetn : 1;
        unsigned int vaux_reset_n : 1;
        unsigned int reserved_1 : 2;
        unsigned int tca_dp4_por : 1;
        unsigned int tca_usb_dev_por : 1;
        unsigned int tca_mux_enable : 1;
        unsigned int reserved_2 : 17;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_usb2phy_por_n_START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_usb2phy_por_n_END (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_usb3phy_reset_n_START (1)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_usb3phy_reset_n_END (1)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_usb2phy_refclk_select_START (4)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_usb2phy_refclk_select_END (4)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_asp_use_phy_START (5)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_asp_use_phy_END (5)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_drv_vbus_sel2_START (6)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_drv_vbus_sel2_END (7)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_usb31c_resetn_START (8)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_usb31c_resetn_END (8)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_vaux_reset_n_START (9)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_vaux_reset_n_END (9)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_tca_dp4_por_START (12)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_tca_dp4_por_END (12)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_tca_usb_dev_por_START (13)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_tca_usb_dev_por_END (13)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_tca_mux_enable_START (14)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA0_tca_mux_enable_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb3_phy_ext_mplla_ssc_freq_cnt_init : 12;
        unsigned int usb3_phy_ext_mplla_ssc_freq_cnt_ovrd_en : 1;
        unsigned int usb3_phy_ext_mplla_ssc_freq_cnt_peak : 8;
        unsigned int usb3_phy_ext_mplla_word_div2_en : 1;
        unsigned int usb3_phy_ext_rx_eq_delta_iq_g1 : 4;
        unsigned int usb3_phy_ext_rx_eq_delta_iq_g2 : 4;
        unsigned int reserved : 2;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFGA4_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA4_usb3_phy_ext_mplla_ssc_freq_cnt_init_START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA4_usb3_phy_ext_mplla_ssc_freq_cnt_init_END (11)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA4_usb3_phy_ext_mplla_ssc_freq_cnt_ovrd_en_START (12)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA4_usb3_phy_ext_mplla_ssc_freq_cnt_ovrd_en_END (12)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA4_usb3_phy_ext_mplla_ssc_freq_cnt_peak_START (13)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA4_usb3_phy_ext_mplla_ssc_freq_cnt_peak_END (20)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA4_usb3_phy_ext_mplla_word_div2_en_START (21)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA4_usb3_phy_ext_mplla_word_div2_en_END (21)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA4_usb3_phy_ext_rx_eq_delta_iq_g1_START (22)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA4_usb3_phy_ext_rx_eq_delta_iq_g1_END (25)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA4_usb3_phy_ext_rx_eq_delta_iq_g2_START (26)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA4_usb3_phy_ext_rx_eq_delta_iq_g2_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb3_ext_rx_term_ctrl : 3;
        unsigned int usb3_ext_tx_term_ctrl : 3;
        unsigned int reserved_0 : 2;
        unsigned int usb20_databus_test_data : 8;
        unsigned int usb20_databus_test_en : 1;
        unsigned int usb20_databus_test_vld : 1;
        unsigned int reserved_1 : 2;
        unsigned int dppulldown_when_sel_asp : 1;
        unsigned int dmpulldown_when_sel_asp : 1;
        unsigned int idpullup_when_sel_asp : 1;
        unsigned int reserved_2 : 9;
    } reg;
} SOC_USB31_MISC_CTRL_USB_MISC_CFGA8_UNION;
#endif
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA8_usb3_ext_rx_term_ctrl_START (0)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA8_usb3_ext_rx_term_ctrl_END (2)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA8_usb3_ext_tx_term_ctrl_START (3)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA8_usb3_ext_tx_term_ctrl_END (5)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA8_usb20_databus_test_data_START (8)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA8_usb20_databus_test_data_END (15)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA8_usb20_databus_test_en_START (16)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA8_usb20_databus_test_en_END (16)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA8_usb20_databus_test_vld_START (17)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA8_usb20_databus_test_vld_END (17)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA8_dppulldown_when_sel_asp_START (20)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA8_dppulldown_when_sel_asp_END (20)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA8_dmpulldown_when_sel_asp_START (21)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA8_dmpulldown_when_sel_asp_END (21)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA8_idpullup_when_sel_asp_START (22)
#define SOC_USB31_MISC_CTRL_USB_MISC_CFGA8_idpullup_when_sel_asp_END (22)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
