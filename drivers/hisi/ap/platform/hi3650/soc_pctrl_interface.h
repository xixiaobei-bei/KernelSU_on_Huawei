#ifndef __SOC_PCTRL_INTERFACE_H__
#define __SOC_PCTRL_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_PCTRL_G3D_RASTER_ADDR(base) ((base) + (0x000))
#define SOC_PCTRL_PERI_CTRL0_ADDR(base) ((base) + (0x004))
#define SOC_PCTRL_PERI_CTRL1_ADDR(base) ((base) + (0x008))
#define SOC_PCTRL_PERI_CTRL2_ADDR(base) ((base) + (0x00C))
#define SOC_PCTRL_PERI_CTRL3_ADDR(base) ((base) + (0x010))
#define SOC_PCTRL_PERI_CTRL4_ADDR(base) ((base) + (0x014))
#define SOC_PCTRL_PERI_CTRL12_ADDR(base) ((base) + (0x034))
#define SOC_PCTRL_PERI_CTRL13_ADDR(base) ((base) + (0x038))
#define SOC_PCTRL_PERI_CTRL14_ADDR(base) ((base) + (0x03C))
#define SOC_PCTRL_PERI_CTRL15_ADDR(base) ((base) + (0x040))
#define SOC_PCTRL_PERI_CTRL16_ADDR(base) ((base) + (0x044))
#define SOC_PCTRL_PERI_CTRL17_ADDR(base) ((base) + (0x048))
#define SOC_PCTRL_PERI_CTRL18_ADDR(base) ((base) + (0x04C))
#define SOC_PCTRL_PERI_CTRL19_ADDR(base) ((base) + (0x050))
#define SOC_PCTRL_PERI_CTRL20_ADDR(base) ((base) + (0x054))
#define SOC_PCTRL_PERI_CTRL21_ADDR(base) ((base) + (0x058))
#define SOC_PCTRL_PERI_CTRL22_ADDR(base) ((base) + (0x05C))
#define SOC_PCTRL_PERI_CTRL23_ADDR(base) ((base) + (0x060))
#define SOC_PCTRL_PERI_CTRL24_ADDR(base) ((base) + (0x064))
#define SOC_PCTRL_PERI_CTRL25_ADDR(base) ((base) + (0x068))
#define SOC_PCTRL_PERI_CTRL26_ADDR(base) ((base) + (0x06C))
#define SOC_PCTRL_PERI_CTRL27_ADDR(base) ((base) + (0x070))
#define SOC_PCTRL_PERI_CTRL28_ADDR(base) ((base) + (0x074))
#define SOC_PCTRL_PERI_CTRL29_ADDR(base) ((base) + (0x078))
#define SOC_PCTRL_PERI_CTRL30_ADDR(base) ((base) + (0x07C))
#define SOC_PCTRL_PERI_CTRL31_ADDR(base) ((base) + (0x080))
#define SOC_PCTRL_PERI_CTRL32_ADDR(base) ((base) + (0x084))
#define SOC_PCTRL_PERI_CTRL33_ADDR(base) ((base) + (0x088))
#define SOC_PCTRL_PERI_STAT0_ADDR(base) ((base) + (0x094))
#define SOC_PCTRL_PERI_STAT1_ADDR(base) ((base) + (0x098))
#define SOC_PCTRL_PERI_STAT2_ADDR(base) ((base) + (0x09C))
#define SOC_PCTRL_PERI_STAT3_ADDR(base) ((base) + (0x0A0))
#define SOC_PCTRL_PERI_STAT4_ADDR(base) ((base) + (0x0A4))
#define SOC_PCTRL_PERI_STAT5_ADDR(base) ((base) + (0x0A8))
#define SOC_PCTRL_PERI_STAT6_ADDR(base) ((base) + (0x0AC))
#define SOC_PCTRL_PERI_STAT7_ADDR(base) ((base) + (0x0B0))
#define SOC_PCTRL_PERI_STAT8_ADDR(base) ((base) + (0x0B4))
#define SOC_PCTRL_PERI_STAT9_ADDR(base) ((base) + (0x0B8))
#define SOC_PCTRL_PERI_STAT10_ADDR(base) ((base) + (0x0BC))
#define SOC_PCTRL_PERI_STAT11_ADDR(base) ((base) + (0x0C0))
#define SOC_PCTRL_PERI_STAT12_ADDR(base) ((base) + (0x0C4))
#define SOC_PCTRL_PERI_STAT13_ADDR(base) ((base) + (0x0C8))
#define SOC_PCTRL_PERI_STAT14_ADDR(base) ((base) + (0x0CC))
#define SOC_PCTRL_PERI_STAT15_ADDR(base) ((base) + (0x0D0))
#define SOC_PCTRL_PERI_STAT16_ADDR(base) ((base) + (0x0D4))
#define SOC_PCTRL_PERI_STAT17_ADDR(base) ((base) + (0x0D8))
#define SOC_PCTRL_PERI_STAT18_ADDR(base) ((base) + (0x0DC))
#define SOC_PCTRL_PERI_STAT19_ADDR(base) ((base) + (0x0E0))
#define SOC_PCTRL_USB2_HOST_CTRL0_ADDR(base) ((base) + (0x0F0))
#define SOC_PCTRL_USB2_HOST_CTRL1_ADDR(base) ((base) + (0x0F4))
#define SOC_PCTRL_USB2_HOST_CTRL2_ADDR(base) ((base) + (0x0F8))
#define SOC_PCTRL_RESOURCE0_LOCK_ADDR(base) ((base) + (0x400))
#define SOC_PCTRL_RESOURCE0_UNLOCK_ADDR(base) ((base) + (0x404))
#define SOC_PCTRL_RESOURCE0_LOCK_ST_ADDR(base) ((base) + (0x408))
#define SOC_PCTRL_RESOURCE1_LOCK_ADDR(base) ((base) + (0x40C))
#define SOC_PCTRL_RESOURCE1_UNLOCK_ADDR(base) ((base) + (0x410))
#define SOC_PCTRL_RESOURCE1_LOCK_ST_ADDR(base) ((base) + (0x414))
#define SOC_PCTRL_RESOURCE2_LOCK_ADDR(base) ((base) + (0x418))
#define SOC_PCTRL_RESOURCE2_UNLOCK_ADDR(base) ((base) + (0x41C))
#define SOC_PCTRL_RESOURCE2_LOCK_ST_ADDR(base) ((base) + (0x420))
#define SOC_PCTRL_RESOURCE3_LOCK_ADDR(base) ((base) + (0x424))
#define SOC_PCTRL_RESOURCE3_UNLOCK_ADDR(base) ((base) + (0x428))
#define SOC_PCTRL_RESOURCE3_LOCK_ST_ADDR(base) ((base) + (0x42C))
#define SOC_PCTRL_RESOURCE4_LOCK_ADDR(base) ((base) + (0x800))
#define SOC_PCTRL_RESOURCE4_UNLOCK_ADDR(base) ((base) + (0x804))
#define SOC_PCTRL_RESOURCE4_LOCK_ST_ADDR(base) ((base) + (0x808))
#define SOC_PCTRL_RESOURCE5_LOCK_ADDR(base) ((base) + (0x80C))
#define SOC_PCTRL_RESOURCE5_UNLOCK_ADDR(base) ((base) + (0x810))
#define SOC_PCTRL_RESOURCE5_LOCK_ST_ADDR(base) ((base) + (0x814))
#define SOC_PCTRL_RESOURCE6_LOCK_ADDR(base) ((base) + (0x818))
#define SOC_PCTRL_RESOURCE6_UNLOCK_ADDR(base) ((base) + (0x81C))
#define SOC_PCTRL_RESOURCE6_LOCK_ST_ADDR(base) ((base) + (0x820))
#define SOC_PCTRL_RESOURCE7_LOCK_ADDR(base) ((base) + (0x824))
#define SOC_PCTRL_RESOURCE7_UNLOCK_ADDR(base) ((base) + (0x828))
#define SOC_PCTRL_RESOURCE7_LOCK_ST_ADDR(base) ((base) + (0x82C))
#define SOC_PCTRL_PERI_CTRL5_ADDR(base) ((base) + (0xC00))
#define SOC_PCTRL_PERI_CTRL6_ADDR(base) ((base) + (0xC04))
#define SOC_PCTRL_PERI_CTRL7_ADDR(base) ((base) + (0xC08))
#define SOC_PCTRL_PERI_CTRL8_ADDR(base) ((base) + (0xC0C))
#define SOC_PCTRL_PERI_CTRL9_ADDR(base) ((base) + (0xC10))
#define SOC_PCTRL_PERI_CTRL10_ADDR(base) ((base) + (0xC14))
#define SOC_PCTRL_PERI_CTRL11_ADDR(base) ((base) + (0xC18))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_div : 10;
        unsigned int portrait_landscape : 1;
        unsigned int lcd_3d_2d : 1;
        unsigned int g3d_raster_en : 1;
        unsigned int lcd_3d_sw_inv : 4;
        unsigned int reserved : 15;
    } reg;
} SOC_PCTRL_G3D_RASTER_UNION;
#endif
#define SOC_PCTRL_G3D_RASTER_g3d_div_START (0)
#define SOC_PCTRL_G3D_RASTER_g3d_div_END (9)
#define SOC_PCTRL_G3D_RASTER_portrait_landscape_START (10)
#define SOC_PCTRL_G3D_RASTER_portrait_landscape_END (10)
#define SOC_PCTRL_G3D_RASTER_lcd_3d_2d_START (11)
#define SOC_PCTRL_G3D_RASTER_lcd_3d_2d_END (11)
#define SOC_PCTRL_G3D_RASTER_g3d_raster_en_START (12)
#define SOC_PCTRL_G3D_RASTER_g3d_raster_en_END (12)
#define SOC_PCTRL_G3D_RASTER_lcd_3d_sw_inv_START (13)
#define SOC_PCTRL_G3D_RASTER_lcd_3d_sw_inv_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_1wire_e : 1;
        unsigned int peri_ctrl0_cmd : 15;
        unsigned int peri_ctrl0_msk : 16;
    } reg;
} SOC_PCTRL_PERI_CTRL0_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL0_sc_1wire_e_START (0)
#define SOC_PCTRL_PERI_CTRL0_sc_1wire_e_END (0)
#define SOC_PCTRL_PERI_CTRL0_peri_ctrl0_cmd_START (1)
#define SOC_PCTRL_PERI_CTRL0_peri_ctrl0_cmd_END (15)
#define SOC_PCTRL_PERI_CTRL0_peri_ctrl0_msk_START (16)
#define SOC_PCTRL_PERI_CTRL0_peri_ctrl0_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int test_point_sel : 7;
        unsigned int peri_ctrl0_cmd : 9;
        unsigned int peri_ctrl1_msk : 16;
    } reg;
} SOC_PCTRL_PERI_CTRL1_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL1_test_point_sel_START (0)
#define SOC_PCTRL_PERI_CTRL1_test_point_sel_END (6)
#define SOC_PCTRL_PERI_CTRL1_peri_ctrl0_cmd_START (7)
#define SOC_PCTRL_PERI_CTRL1_peri_ctrl0_cmd_END (15)
#define SOC_PCTRL_PERI_CTRL1_peri_ctrl1_msk_START (16)
#define SOC_PCTRL_PERI_CTRL1_peri_ctrl1_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_ctrl2_cmd : 16;
        unsigned int peri_ctrl2_msk : 16;
    } reg;
} SOC_PCTRL_PERI_CTRL2_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL2_peri_ctrl2_cmd_START (0)
#define SOC_PCTRL_PERI_CTRL2_peri_ctrl2_cmd_END (15)
#define SOC_PCTRL_PERI_CTRL2_peri_ctrl2_msk_START (16)
#define SOC_PCTRL_PERI_CTRL2_peri_ctrl2_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 6;
        unsigned int venc_force_clk_enable : 1;
        unsigned int vdec_force_clk_enable : 1;
        unsigned int reserved_1 : 8;
        unsigned int peri_ctrl3_msk : 16;
    } reg;
} SOC_PCTRL_PERI_CTRL3_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL3_venc_force_clk_enable_START (6)
#define SOC_PCTRL_PERI_CTRL3_venc_force_clk_enable_END (6)
#define SOC_PCTRL_PERI_CTRL3_vdec_force_clk_enable_START (7)
#define SOC_PCTRL_PERI_CTRL3_vdec_force_clk_enable_END (7)
#define SOC_PCTRL_PERI_CTRL3_peri_ctrl3_msk_START (16)
#define SOC_PCTRL_PERI_CTRL3_peri_ctrl3_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_pctrl_qos : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL4_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL4_sc_pctrl_qos_START (0)
#define SOC_PCTRL_PERI_CTRL4_sc_pctrl_qos_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cssys_araddr_cohe : 1;
        unsigned int cssys_awaddr_cohe : 1;
        unsigned int noc_emmc1_haddr_cohe : 1;
        unsigned int noc_sd3_haddr_cohe : 1;
        unsigned int noc_sdio0_haddr_cohe : 1;
        unsigned int iomcu_haddr_cohe : 1;
        unsigned int secp_araddr_cohe : 1;
        unsigned int secp_awaddr_cohe : 1;
        unsigned int secs_araddr_cohe : 1;
        unsigned int secs_awaddr_cohe : 1;
        unsigned int usb3otg_araddr_cohe : 1;
        unsigned int usb3otg_awaddr_cohe : 1;
        unsigned int socp_araddr_cohe : 1;
        unsigned int socp_awaddr_cohe : 1;
        unsigned int emmc0_araddr_cohe : 1;
        unsigned int emmc0_awaddr_cohe : 1;
        unsigned int noc_sdio1_haddr_cohe : 1;
        unsigned int reserved : 15;
    } reg;
} SOC_PCTRL_PERI_CTRL12_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL12_cssys_araddr_cohe_START (0)
#define SOC_PCTRL_PERI_CTRL12_cssys_araddr_cohe_END (0)
#define SOC_PCTRL_PERI_CTRL12_cssys_awaddr_cohe_START (1)
#define SOC_PCTRL_PERI_CTRL12_cssys_awaddr_cohe_END (1)
#define SOC_PCTRL_PERI_CTRL12_noc_emmc1_haddr_cohe_START (2)
#define SOC_PCTRL_PERI_CTRL12_noc_emmc1_haddr_cohe_END (2)
#define SOC_PCTRL_PERI_CTRL12_noc_sd3_haddr_cohe_START (3)
#define SOC_PCTRL_PERI_CTRL12_noc_sd3_haddr_cohe_END (3)
#define SOC_PCTRL_PERI_CTRL12_noc_sdio0_haddr_cohe_START (4)
#define SOC_PCTRL_PERI_CTRL12_noc_sdio0_haddr_cohe_END (4)
#define SOC_PCTRL_PERI_CTRL12_iomcu_haddr_cohe_START (5)
#define SOC_PCTRL_PERI_CTRL12_iomcu_haddr_cohe_END (5)
#define SOC_PCTRL_PERI_CTRL12_secp_araddr_cohe_START (6)
#define SOC_PCTRL_PERI_CTRL12_secp_araddr_cohe_END (6)
#define SOC_PCTRL_PERI_CTRL12_secp_awaddr_cohe_START (7)
#define SOC_PCTRL_PERI_CTRL12_secp_awaddr_cohe_END (7)
#define SOC_PCTRL_PERI_CTRL12_secs_araddr_cohe_START (8)
#define SOC_PCTRL_PERI_CTRL12_secs_araddr_cohe_END (8)
#define SOC_PCTRL_PERI_CTRL12_secs_awaddr_cohe_START (9)
#define SOC_PCTRL_PERI_CTRL12_secs_awaddr_cohe_END (9)
#define SOC_PCTRL_PERI_CTRL12_usb3otg_araddr_cohe_START (10)
#define SOC_PCTRL_PERI_CTRL12_usb3otg_araddr_cohe_END (10)
#define SOC_PCTRL_PERI_CTRL12_usb3otg_awaddr_cohe_START (11)
#define SOC_PCTRL_PERI_CTRL12_usb3otg_awaddr_cohe_END (11)
#define SOC_PCTRL_PERI_CTRL12_socp_araddr_cohe_START (12)
#define SOC_PCTRL_PERI_CTRL12_socp_araddr_cohe_END (12)
#define SOC_PCTRL_PERI_CTRL12_socp_awaddr_cohe_START (13)
#define SOC_PCTRL_PERI_CTRL12_socp_awaddr_cohe_END (13)
#define SOC_PCTRL_PERI_CTRL12_emmc0_araddr_cohe_START (14)
#define SOC_PCTRL_PERI_CTRL12_emmc0_araddr_cohe_END (14)
#define SOC_PCTRL_PERI_CTRL12_emmc0_awaddr_cohe_START (15)
#define SOC_PCTRL_PERI_CTRL12_emmc0_awaddr_cohe_END (15)
#define SOC_PCTRL_PERI_CTRL12_noc_sdio1_haddr_cohe_START (16)
#define SOC_PCTRL_PERI_CTRL12_noc_sdio1_haddr_cohe_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_pctrl_cohe0 : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL13_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL13_sc_pctrl_cohe0_START (0)
#define SOC_PCTRL_PERI_CTRL13_sc_pctrl_cohe0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_pctrl_cohe1 : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL14_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL14_sc_pctrl_cohe1_START (0)
#define SOC_PCTRL_PERI_CTRL14_sc_pctrl_cohe1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_pctrl_cohe2 : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL15_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL15_sc_pctrl_cohe2_START (0)
#define SOC_PCTRL_PERI_CTRL15_sc_pctrl_cohe2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL16_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL17_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL18_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_barrierdisable : 1;
        unsigned int g3d_slp_ext0 : 1;
        unsigned int g3d_slp_ext1 : 1;
        unsigned int g3d_sd_ext : 1;
        unsigned int g3d_pw_sel : 1;
        unsigned int g3d_slp_sel : 1;
        unsigned int reserved_0 : 3;
        unsigned int gpu_striping_granule : 3;
        unsigned int mediabus_interleave_stripe_select : 3;
        unsigned int vivobus_interleave_stripe_select : 3;
        unsigned int reserved_1 : 1;
        unsigned int isp_csi_sel : 1;
        unsigned int isp_fsin0_oen : 1;
        unsigned int isp_fsin1_oen : 1;
        unsigned int p_csi_dphy_sel : 1;
        unsigned int s_csi_dphy_sel : 1;
        unsigned int isp_p_csi_clk_gate : 1;
        unsigned int isp_s_csi_clk_gate : 1;
        unsigned int isp_dw_axi_s1_cg_en : 1;
        unsigned int isp_dw_axi_cg_en : 1;
        unsigned int dphy1_clk_gate : 1;
        unsigned int dphy2_clk_gate : 1;
        unsigned int dphy3_clk_gate : 1;
        unsigned int reserved_2 : 1;
    } reg;
} SOC_PCTRL_PERI_CTRL19_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL19_g3d_barrierdisable_START (0)
#define SOC_PCTRL_PERI_CTRL19_g3d_barrierdisable_END (0)
#define SOC_PCTRL_PERI_CTRL19_g3d_slp_ext0_START (1)
#define SOC_PCTRL_PERI_CTRL19_g3d_slp_ext0_END (1)
#define SOC_PCTRL_PERI_CTRL19_g3d_slp_ext1_START (2)
#define SOC_PCTRL_PERI_CTRL19_g3d_slp_ext1_END (2)
#define SOC_PCTRL_PERI_CTRL19_g3d_sd_ext_START (3)
#define SOC_PCTRL_PERI_CTRL19_g3d_sd_ext_END (3)
#define SOC_PCTRL_PERI_CTRL19_g3d_pw_sel_START (4)
#define SOC_PCTRL_PERI_CTRL19_g3d_pw_sel_END (4)
#define SOC_PCTRL_PERI_CTRL19_g3d_slp_sel_START (5)
#define SOC_PCTRL_PERI_CTRL19_g3d_slp_sel_END (5)
#define SOC_PCTRL_PERI_CTRL19_gpu_striping_granule_START (9)
#define SOC_PCTRL_PERI_CTRL19_gpu_striping_granule_END (11)
#define SOC_PCTRL_PERI_CTRL19_mediabus_interleave_stripe_select_START (12)
#define SOC_PCTRL_PERI_CTRL19_mediabus_interleave_stripe_select_END (14)
#define SOC_PCTRL_PERI_CTRL19_vivobus_interleave_stripe_select_START (15)
#define SOC_PCTRL_PERI_CTRL19_vivobus_interleave_stripe_select_END (17)
#define SOC_PCTRL_PERI_CTRL19_isp_csi_sel_START (19)
#define SOC_PCTRL_PERI_CTRL19_isp_csi_sel_END (19)
#define SOC_PCTRL_PERI_CTRL19_isp_fsin0_oen_START (20)
#define SOC_PCTRL_PERI_CTRL19_isp_fsin0_oen_END (20)
#define SOC_PCTRL_PERI_CTRL19_isp_fsin1_oen_START (21)
#define SOC_PCTRL_PERI_CTRL19_isp_fsin1_oen_END (21)
#define SOC_PCTRL_PERI_CTRL19_p_csi_dphy_sel_START (22)
#define SOC_PCTRL_PERI_CTRL19_p_csi_dphy_sel_END (22)
#define SOC_PCTRL_PERI_CTRL19_s_csi_dphy_sel_START (23)
#define SOC_PCTRL_PERI_CTRL19_s_csi_dphy_sel_END (23)
#define SOC_PCTRL_PERI_CTRL19_isp_p_csi_clk_gate_START (24)
#define SOC_PCTRL_PERI_CTRL19_isp_p_csi_clk_gate_END (24)
#define SOC_PCTRL_PERI_CTRL19_isp_s_csi_clk_gate_START (25)
#define SOC_PCTRL_PERI_CTRL19_isp_s_csi_clk_gate_END (25)
#define SOC_PCTRL_PERI_CTRL19_isp_dw_axi_s1_cg_en_START (26)
#define SOC_PCTRL_PERI_CTRL19_isp_dw_axi_s1_cg_en_END (26)
#define SOC_PCTRL_PERI_CTRL19_isp_dw_axi_cg_en_START (27)
#define SOC_PCTRL_PERI_CTRL19_isp_dw_axi_cg_en_END (27)
#define SOC_PCTRL_PERI_CTRL19_dphy1_clk_gate_START (28)
#define SOC_PCTRL_PERI_CTRL19_dphy1_clk_gate_END (28)
#define SOC_PCTRL_PERI_CTRL19_dphy2_clk_gate_START (29)
#define SOC_PCTRL_PERI_CTRL19_dphy2_clk_gate_END (29)
#define SOC_PCTRL_PERI_CTRL19_dphy3_clk_gate_START (30)
#define SOC_PCTRL_PERI_CTRL19_dphy3_clk_gate_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_ctrl20_cmd : 16;
        unsigned int socpsprammemctrl : 16;
    } reg;
} SOC_PCTRL_PERI_CTRL20_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL20_peri_ctrl20_cmd_START (0)
#define SOC_PCTRL_PERI_CTRL20_peri_ctrl20_cmd_END (15)
#define SOC_PCTRL_PERI_CTRL20_socpsprammemctrl_START (16)
#define SOC_PCTRL_PERI_CTRL20_socpsprammemctrl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int venc_designer_field2 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_PCTRL_PERI_CTRL22_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL22_venc_designer_field2_START (0)
#define SOC_PCTRL_PERI_CTRL22_venc_designer_field2_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int venc_mem_limited_throughput : 1;
        unsigned int vdec_mem_limited_throughput : 1;
        unsigned int dmac_ckgt_dis : 1;
        unsigned int pctrl_dphytx_ulpsexit0 : 1;
        unsigned int pctrl_dphytx_ulpsexit1 : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_PCTRL_PERI_CTRL23_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL23_venc_mem_limited_throughput_START (0)
#define SOC_PCTRL_PERI_CTRL23_venc_mem_limited_throughput_END (0)
#define SOC_PCTRL_PERI_CTRL23_vdec_mem_limited_throughput_START (1)
#define SOC_PCTRL_PERI_CTRL23_vdec_mem_limited_throughput_END (1)
#define SOC_PCTRL_PERI_CTRL23_dmac_ckgt_dis_START (2)
#define SOC_PCTRL_PERI_CTRL23_dmac_ckgt_dis_END (2)
#define SOC_PCTRL_PERI_CTRL23_pctrl_dphytx_ulpsexit0_START (3)
#define SOC_PCTRL_PERI_CTRL23_pctrl_dphytx_ulpsexit0_END (3)
#define SOC_PCTRL_PERI_CTRL23_pctrl_dphytx_ulpsexit1_START (4)
#define SOC_PCTRL_PERI_CTRL23_pctrl_dphytx_ulpsexit1_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pmussi_mst_cnt : 3;
        unsigned int djtag_mst_pstrb : 4;
        unsigned int i2c_freq_sel : 1;
        unsigned int reserved_0 : 1;
        unsigned int sdio0_resp_ctrl : 1;
        unsigned int sd3_resp_ctrl : 1;
        unsigned int sdio1_resp_ctrl : 1;
        unsigned int lpmcu_resp_ctrl : 1;
        unsigned int iomcu_cfgbus_resp_ctrl : 1;
        unsigned int iomcu_sysbus_resp_ctrl : 1;
        unsigned int isp_axi_xdcdr_sel : 1;
        unsigned int pmussi1_mst_cnt : 3;
        unsigned int reserved_1 : 1;
        unsigned int codecssi_mst_cnt : 3;
        unsigned int reserved_2 : 1;
        unsigned int sc_usb3phy_abb_gt_en : 1;
        unsigned int sc_clk_usb3phy_3mux1_sel : 2;
        unsigned int reserved_3 : 1;
        unsigned int sc_usb3phy_ioclk_gt_en : 1;
        unsigned int reserved_4 : 3;
    } reg;
} SOC_PCTRL_PERI_CTRL24_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL24_pmussi_mst_cnt_START (0)
#define SOC_PCTRL_PERI_CTRL24_pmussi_mst_cnt_END (2)
#define SOC_PCTRL_PERI_CTRL24_djtag_mst_pstrb_START (3)
#define SOC_PCTRL_PERI_CTRL24_djtag_mst_pstrb_END (6)
#define SOC_PCTRL_PERI_CTRL24_i2c_freq_sel_START (7)
#define SOC_PCTRL_PERI_CTRL24_i2c_freq_sel_END (7)
#define SOC_PCTRL_PERI_CTRL24_sdio0_resp_ctrl_START (9)
#define SOC_PCTRL_PERI_CTRL24_sdio0_resp_ctrl_END (9)
#define SOC_PCTRL_PERI_CTRL24_sd3_resp_ctrl_START (10)
#define SOC_PCTRL_PERI_CTRL24_sd3_resp_ctrl_END (10)
#define SOC_PCTRL_PERI_CTRL24_sdio1_resp_ctrl_START (11)
#define SOC_PCTRL_PERI_CTRL24_sdio1_resp_ctrl_END (11)
#define SOC_PCTRL_PERI_CTRL24_lpmcu_resp_ctrl_START (12)
#define SOC_PCTRL_PERI_CTRL24_lpmcu_resp_ctrl_END (12)
#define SOC_PCTRL_PERI_CTRL24_iomcu_cfgbus_resp_ctrl_START (13)
#define SOC_PCTRL_PERI_CTRL24_iomcu_cfgbus_resp_ctrl_END (13)
#define SOC_PCTRL_PERI_CTRL24_iomcu_sysbus_resp_ctrl_START (14)
#define SOC_PCTRL_PERI_CTRL24_iomcu_sysbus_resp_ctrl_END (14)
#define SOC_PCTRL_PERI_CTRL24_isp_axi_xdcdr_sel_START (15)
#define SOC_PCTRL_PERI_CTRL24_isp_axi_xdcdr_sel_END (15)
#define SOC_PCTRL_PERI_CTRL24_pmussi1_mst_cnt_START (16)
#define SOC_PCTRL_PERI_CTRL24_pmussi1_mst_cnt_END (18)
#define SOC_PCTRL_PERI_CTRL24_codecssi_mst_cnt_START (20)
#define SOC_PCTRL_PERI_CTRL24_codecssi_mst_cnt_END (22)
#define SOC_PCTRL_PERI_CTRL24_sc_usb3phy_abb_gt_en_START (24)
#define SOC_PCTRL_PERI_CTRL24_sc_usb3phy_abb_gt_en_END (24)
#define SOC_PCTRL_PERI_CTRL24_sc_clk_usb3phy_3mux1_sel_START (25)
#define SOC_PCTRL_PERI_CTRL24_sc_clk_usb3phy_3mux1_sel_END (26)
#define SOC_PCTRL_PERI_CTRL24_sc_usb3phy_ioclk_gt_en_START (28)
#define SOC_PCTRL_PERI_CTRL24_sc_usb3phy_ioclk_gt_en_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vdec_qos_0 : 2;
        unsigned int vdec_qos_1 : 2;
        unsigned int venc_qos_0 : 2;
        unsigned int venc_qos_1 : 2;
        unsigned int secp_mem_ctrl_sd : 1;
        unsigned int secs_mem_ctrl_sd : 1;
        unsigned int mmbuf_sram_sd : 1;
        unsigned int smmu_integ_sec_override : 1;
        unsigned int reserved_0 : 1;
        unsigned int dphy2l_rx_enableclk : 1;
        unsigned int dphy4l_rx_enableclk0 : 1;
        unsigned int dphy4l_rx_enableclk1 : 1;
        unsigned int reserved_1 : 1;
        unsigned int sc_g3d_dw_axi_m1_cg_en : 1;
        unsigned int sc_g3d_dw_axi_s0_cg_en : 1;
        unsigned int sc_g3d_dw_axi_s1_cg_en : 1;
        unsigned int sc_g3d_dw_axi_s2_cg_en : 1;
        unsigned int sc_div_usb3phy : 6;
        unsigned int sc_usb3phy_pll_div_en : 1;
        unsigned int reserved_2 : 1;
        unsigned int sc_modem_ipc_auto_clk_gate_en : 1;
        unsigned int sc_modem_ipc_soft_gate_clk_en : 1;
        unsigned int peri_ctrl25_cmd : 1;
    } reg;
} SOC_PCTRL_PERI_CTRL25_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL25_vdec_qos_0_START (0)
#define SOC_PCTRL_PERI_CTRL25_vdec_qos_0_END (1)
#define SOC_PCTRL_PERI_CTRL25_vdec_qos_1_START (2)
#define SOC_PCTRL_PERI_CTRL25_vdec_qos_1_END (3)
#define SOC_PCTRL_PERI_CTRL25_venc_qos_0_START (4)
#define SOC_PCTRL_PERI_CTRL25_venc_qos_0_END (5)
#define SOC_PCTRL_PERI_CTRL25_venc_qos_1_START (6)
#define SOC_PCTRL_PERI_CTRL25_venc_qos_1_END (7)
#define SOC_PCTRL_PERI_CTRL25_secp_mem_ctrl_sd_START (8)
#define SOC_PCTRL_PERI_CTRL25_secp_mem_ctrl_sd_END (8)
#define SOC_PCTRL_PERI_CTRL25_secs_mem_ctrl_sd_START (9)
#define SOC_PCTRL_PERI_CTRL25_secs_mem_ctrl_sd_END (9)
#define SOC_PCTRL_PERI_CTRL25_mmbuf_sram_sd_START (10)
#define SOC_PCTRL_PERI_CTRL25_mmbuf_sram_sd_END (10)
#define SOC_PCTRL_PERI_CTRL25_smmu_integ_sec_override_START (11)
#define SOC_PCTRL_PERI_CTRL25_smmu_integ_sec_override_END (11)
#define SOC_PCTRL_PERI_CTRL25_dphy2l_rx_enableclk_START (13)
#define SOC_PCTRL_PERI_CTRL25_dphy2l_rx_enableclk_END (13)
#define SOC_PCTRL_PERI_CTRL25_dphy4l_rx_enableclk0_START (14)
#define SOC_PCTRL_PERI_CTRL25_dphy4l_rx_enableclk0_END (14)
#define SOC_PCTRL_PERI_CTRL25_dphy4l_rx_enableclk1_START (15)
#define SOC_PCTRL_PERI_CTRL25_dphy4l_rx_enableclk1_END (15)
#define SOC_PCTRL_PERI_CTRL25_sc_g3d_dw_axi_m1_cg_en_START (17)
#define SOC_PCTRL_PERI_CTRL25_sc_g3d_dw_axi_m1_cg_en_END (17)
#define SOC_PCTRL_PERI_CTRL25_sc_g3d_dw_axi_s0_cg_en_START (18)
#define SOC_PCTRL_PERI_CTRL25_sc_g3d_dw_axi_s0_cg_en_END (18)
#define SOC_PCTRL_PERI_CTRL25_sc_g3d_dw_axi_s1_cg_en_START (19)
#define SOC_PCTRL_PERI_CTRL25_sc_g3d_dw_axi_s1_cg_en_END (19)
#define SOC_PCTRL_PERI_CTRL25_sc_g3d_dw_axi_s2_cg_en_START (20)
#define SOC_PCTRL_PERI_CTRL25_sc_g3d_dw_axi_s2_cg_en_END (20)
#define SOC_PCTRL_PERI_CTRL25_sc_div_usb3phy_START (21)
#define SOC_PCTRL_PERI_CTRL25_sc_div_usb3phy_END (26)
#define SOC_PCTRL_PERI_CTRL25_sc_usb3phy_pll_div_en_START (27)
#define SOC_PCTRL_PERI_CTRL25_sc_usb3phy_pll_div_en_END (27)
#define SOC_PCTRL_PERI_CTRL25_sc_modem_ipc_auto_clk_gate_en_START (29)
#define SOC_PCTRL_PERI_CTRL25_sc_modem_ipc_auto_clk_gate_en_END (29)
#define SOC_PCTRL_PERI_CTRL25_sc_modem_ipc_soft_gate_clk_en_START (30)
#define SOC_PCTRL_PERI_CTRL25_sc_modem_ipc_soft_gate_clk_en_END (30)
#define SOC_PCTRL_PERI_CTRL25_peri_ctrl25_cmd_START (31)
#define SOC_PCTRL_PERI_CTRL25_peri_ctrl25_cmd_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pack_dbg_mode_en : 4;
        unsigned int pack_dbg_mode_sel : 5;
        unsigned int reserved_0 : 5;
        unsigned int isp_dw_axi_gatedclock_en : 1;
        unsigned int reserved_1 : 1;
        unsigned int spi_tprf_slp : 1;
        unsigned int spi_tprf_dslp : 1;
        unsigned int spi_tprf_sd : 1;
        unsigned int g3d_tprf_slp : 1;
        unsigned int g3d_tprf_dslp : 1;
        unsigned int g3d_tprf_sd : 1;
        unsigned int g3d_spsram_slp : 1;
        unsigned int g3d_spsram_dslp : 1;
        unsigned int g3d_spsram_sd : 1;
        unsigned int reserved_2 : 7;
    } reg;
} SOC_PCTRL_PERI_CTRL26_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL26_pack_dbg_mode_en_START (0)
#define SOC_PCTRL_PERI_CTRL26_pack_dbg_mode_en_END (3)
#define SOC_PCTRL_PERI_CTRL26_pack_dbg_mode_sel_START (4)
#define SOC_PCTRL_PERI_CTRL26_pack_dbg_mode_sel_END (8)
#define SOC_PCTRL_PERI_CTRL26_isp_dw_axi_gatedclock_en_START (14)
#define SOC_PCTRL_PERI_CTRL26_isp_dw_axi_gatedclock_en_END (14)
#define SOC_PCTRL_PERI_CTRL26_spi_tprf_slp_START (16)
#define SOC_PCTRL_PERI_CTRL26_spi_tprf_slp_END (16)
#define SOC_PCTRL_PERI_CTRL26_spi_tprf_dslp_START (17)
#define SOC_PCTRL_PERI_CTRL26_spi_tprf_dslp_END (17)
#define SOC_PCTRL_PERI_CTRL26_spi_tprf_sd_START (18)
#define SOC_PCTRL_PERI_CTRL26_spi_tprf_sd_END (18)
#define SOC_PCTRL_PERI_CTRL26_g3d_tprf_slp_START (19)
#define SOC_PCTRL_PERI_CTRL26_g3d_tprf_slp_END (19)
#define SOC_PCTRL_PERI_CTRL26_g3d_tprf_dslp_START (20)
#define SOC_PCTRL_PERI_CTRL26_g3d_tprf_dslp_END (20)
#define SOC_PCTRL_PERI_CTRL26_g3d_tprf_sd_START (21)
#define SOC_PCTRL_PERI_CTRL26_g3d_tprf_sd_END (21)
#define SOC_PCTRL_PERI_CTRL26_g3d_spsram_slp_START (22)
#define SOC_PCTRL_PERI_CTRL26_g3d_spsram_slp_END (22)
#define SOC_PCTRL_PERI_CTRL26_g3d_spsram_dslp_START (23)
#define SOC_PCTRL_PERI_CTRL26_g3d_spsram_dslp_END (23)
#define SOC_PCTRL_PERI_CTRL26_g3d_spsram_sd_START (24)
#define SOC_PCTRL_PERI_CTRL26_g3d_spsram_sd_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int isp_sft_fiq : 1;
        unsigned int isp_sys_ctrl_0 : 31;
    } reg;
} SOC_PCTRL_PERI_CTRL27_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL27_isp_sft_fiq_START (0)
#define SOC_PCTRL_PERI_CTRL27_isp_sft_fiq_END (0)
#define SOC_PCTRL_PERI_CTRL27_isp_sys_ctrl_0_START (1)
#define SOC_PCTRL_PERI_CTRL27_isp_sys_ctrl_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int isp_sys_ctrl_1 : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL28_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL28_isp_sys_ctrl_1_START (0)
#define SOC_PCTRL_PERI_CTRL28_isp_sys_ctrl_1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pctrl_dphytx_stopcnt0 : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL29_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL29_pctrl_dphytx_stopcnt0_START (0)
#define SOC_PCTRL_PERI_CTRL29_pctrl_dphytx_stopcnt0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pctrl_dphytx_ctrl0 : 16;
        unsigned int pctrl_dphytx_ctrl1 : 16;
    } reg;
} SOC_PCTRL_PERI_CTRL30_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL30_pctrl_dphytx_ctrl0_START (0)
#define SOC_PCTRL_PERI_CTRL30_pctrl_dphytx_ctrl0_END (15)
#define SOC_PCTRL_PERI_CTRL30_pctrl_dphytx_ctrl1_START (16)
#define SOC_PCTRL_PERI_CTRL30_pctrl_dphytx_ctrl1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_spram_rtsel : 2;
        unsigned int peri_spram_wtsel : 2;
        unsigned int peri_spmbsram_rtsel : 2;
        unsigned int peri_spmbsram_wtsel : 2;
        unsigned int peri_rom_rtsel : 2;
        unsigned int peri_rom_ptsel : 2;
        unsigned int peri_rom_trb : 2;
        unsigned int peri_rom_tm : 1;
        unsigned int reserved_0 : 1;
        unsigned int peri_tprf_rct : 2;
        unsigned int peri_tprf_wct : 2;
        unsigned int peri_tprf_kp : 3;
        unsigned int reserved_1 : 1;
        unsigned int peri_dpsram_rtsel : 2;
        unsigned int peri_dpsram_wtsel : 2;
        unsigned int reserved_2 : 4;
    } reg;
} SOC_PCTRL_PERI_CTRL31_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL31_peri_spram_rtsel_START (0)
#define SOC_PCTRL_PERI_CTRL31_peri_spram_rtsel_END (1)
#define SOC_PCTRL_PERI_CTRL31_peri_spram_wtsel_START (2)
#define SOC_PCTRL_PERI_CTRL31_peri_spram_wtsel_END (3)
#define SOC_PCTRL_PERI_CTRL31_peri_spmbsram_rtsel_START (4)
#define SOC_PCTRL_PERI_CTRL31_peri_spmbsram_rtsel_END (5)
#define SOC_PCTRL_PERI_CTRL31_peri_spmbsram_wtsel_START (6)
#define SOC_PCTRL_PERI_CTRL31_peri_spmbsram_wtsel_END (7)
#define SOC_PCTRL_PERI_CTRL31_peri_rom_rtsel_START (8)
#define SOC_PCTRL_PERI_CTRL31_peri_rom_rtsel_END (9)
#define SOC_PCTRL_PERI_CTRL31_peri_rom_ptsel_START (10)
#define SOC_PCTRL_PERI_CTRL31_peri_rom_ptsel_END (11)
#define SOC_PCTRL_PERI_CTRL31_peri_rom_trb_START (12)
#define SOC_PCTRL_PERI_CTRL31_peri_rom_trb_END (13)
#define SOC_PCTRL_PERI_CTRL31_peri_rom_tm_START (14)
#define SOC_PCTRL_PERI_CTRL31_peri_rom_tm_END (14)
#define SOC_PCTRL_PERI_CTRL31_peri_tprf_rct_START (16)
#define SOC_PCTRL_PERI_CTRL31_peri_tprf_rct_END (17)
#define SOC_PCTRL_PERI_CTRL31_peri_tprf_wct_START (18)
#define SOC_PCTRL_PERI_CTRL31_peri_tprf_wct_END (19)
#define SOC_PCTRL_PERI_CTRL31_peri_tprf_kp_START (20)
#define SOC_PCTRL_PERI_CTRL31_peri_tprf_kp_END (22)
#define SOC_PCTRL_PERI_CTRL31_peri_dpsram_rtsel_START (24)
#define SOC_PCTRL_PERI_CTRL31_peri_dpsram_rtsel_END (25)
#define SOC_PCTRL_PERI_CTRL31_peri_dpsram_wtsel_START (26)
#define SOC_PCTRL_PERI_CTRL31_peri_dpsram_wtsel_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pctrl_dphytx_stopcnt1 : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL32_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL32_pctrl_dphytx_stopcnt1_START (0)
#define SOC_PCTRL_PERI_CTRL32_pctrl_dphytx_stopcnt1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL33_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_stat0 : 30;
        unsigned int dphytx_pctrl_trstop_flag1 : 1;
        unsigned int dphytx_pctrl_trstop_flag0 : 1;
    } reg;
} SOC_PCTRL_PERI_STAT0_UNION;
#endif
#define SOC_PCTRL_PERI_STAT0_peri_stat0_START (0)
#define SOC_PCTRL_PERI_STAT0_peri_stat0_END (29)
#define SOC_PCTRL_PERI_STAT0_dphytx_pctrl_trstop_flag1_START (30)
#define SOC_PCTRL_PERI_STAT0_dphytx_pctrl_trstop_flag1_END (30)
#define SOC_PCTRL_PERI_STAT0_dphytx_pctrl_trstop_flag0_START (31)
#define SOC_PCTRL_PERI_STAT0_dphytx_pctrl_trstop_flag0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dphytx_pctrl_status0 : 32;
    } reg;
} SOC_PCTRL_PERI_STAT1_UNION;
#endif
#define SOC_PCTRL_PERI_STAT1_dphytx_pctrl_status0_START (0)
#define SOC_PCTRL_PERI_STAT1_dphytx_pctrl_status0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_stat2 : 31;
        unsigned int nocvencimainnopendingtrans : 1;
    } reg;
} SOC_PCTRL_PERI_STAT2_UNION;
#endif
#define SOC_PCTRL_PERI_STAT2_peri_stat2_START (0)
#define SOC_PCTRL_PERI_STAT2_peri_stat2_END (30)
#define SOC_PCTRL_PERI_STAT2_nocvencimainnopendingtrans_START (31)
#define SOC_PCTRL_PERI_STAT2_nocvencimainnopendingtrans_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_stat3 : 32;
    } reg;
} SOC_PCTRL_PERI_STAT3_UNION;
#endif
#define SOC_PCTRL_PERI_STAT3_peri_stat3_START (0)
#define SOC_PCTRL_PERI_STAT3_peri_stat3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_stat4 : 23;
        unsigned int reserved : 9;
    } reg;
} SOC_PCTRL_PERI_STAT4_UNION;
#endif
#define SOC_PCTRL_PERI_STAT4_peri_stat4_START (0)
#define SOC_PCTRL_PERI_STAT4_peri_stat4_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_mdm2ap_states : 32;
    } reg;
} SOC_PCTRL_PERI_STAT5_UNION;
#endif
#define SOC_PCTRL_PERI_STAT5_sc_mdm2ap_states_START (0)
#define SOC_PCTRL_PERI_STAT5_sc_mdm2ap_states_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_mdm2ap_states : 32;
    } reg;
} SOC_PCTRL_PERI_STAT6_UNION;
#endif
#define SOC_PCTRL_PERI_STAT6_sc_mdm2ap_states_START (0)
#define SOC_PCTRL_PERI_STAT6_sc_mdm2ap_states_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_mdm2ap_states : 32;
    } reg;
} SOC_PCTRL_PERI_STAT7_UNION;
#endif
#define SOC_PCTRL_PERI_STAT7_sc_mdm2ap_states_START (0)
#define SOC_PCTRL_PERI_STAT7_sc_mdm2ap_states_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_mdm2ap_states : 32;
    } reg;
} SOC_PCTRL_PERI_STAT8_UNION;
#endif
#define SOC_PCTRL_PERI_STAT8_sc_mdm2ap_states_START (0)
#define SOC_PCTRL_PERI_STAT8_sc_mdm2ap_states_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tp_s0_grp : 16;
        unsigned int tp_s1_grp : 16;
    } reg;
} SOC_PCTRL_PERI_STAT9_UNION;
#endif
#define SOC_PCTRL_PERI_STAT9_tp_s0_grp_START (0)
#define SOC_PCTRL_PERI_STAT9_tp_s0_grp_END (15)
#define SOC_PCTRL_PERI_STAT9_tp_s1_grp_START (16)
#define SOC_PCTRL_PERI_STAT9_tp_s1_grp_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tp_s2_grp : 16;
        unsigned int tp_s7_grp : 16;
    } reg;
} SOC_PCTRL_PERI_STAT10_UNION;
#endif
#define SOC_PCTRL_PERI_STAT10_tp_s2_grp_START (0)
#define SOC_PCTRL_PERI_STAT10_tp_s2_grp_END (15)
#define SOC_PCTRL_PERI_STAT10_tp_s7_grp_START (16)
#define SOC_PCTRL_PERI_STAT10_tp_s7_grp_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tp_s8_grp : 16;
        unsigned int tp_s9_grp : 16;
    } reg;
} SOC_PCTRL_PERI_STAT11_UNION;
#endif
#define SOC_PCTRL_PERI_STAT11_tp_s8_grp_START (0)
#define SOC_PCTRL_PERI_STAT11_tp_s8_grp_END (15)
#define SOC_PCTRL_PERI_STAT11_tp_s9_grp_START (16)
#define SOC_PCTRL_PERI_STAT11_tp_s9_grp_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tp_s10_grp : 16;
        unsigned int tp_s11_grp : 16;
    } reg;
} SOC_PCTRL_PERI_STAT12_UNION;
#endif
#define SOC_PCTRL_PERI_STAT12_tp_s10_grp_START (0)
#define SOC_PCTRL_PERI_STAT12_tp_s10_grp_END (15)
#define SOC_PCTRL_PERI_STAT12_tp_s11_grp_START (16)
#define SOC_PCTRL_PERI_STAT12_tp_s11_grp_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tp_s12_grp : 16;
        unsigned int tp_s13_grp : 16;
    } reg;
} SOC_PCTRL_PERI_STAT13_UNION;
#endif
#define SOC_PCTRL_PERI_STAT13_tp_s12_grp_START (0)
#define SOC_PCTRL_PERI_STAT13_tp_s12_grp_END (15)
#define SOC_PCTRL_PERI_STAT13_tp_s13_grp_START (16)
#define SOC_PCTRL_PERI_STAT13_tp_s13_grp_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tp_s14_grp : 16;
        unsigned int tp_s15_grp : 16;
    } reg;
} SOC_PCTRL_PERI_STAT14_UNION;
#endif
#define SOC_PCTRL_PERI_STAT14_tp_s14_grp_START (0)
#define SOC_PCTRL_PERI_STAT14_tp_s14_grp_END (15)
#define SOC_PCTRL_PERI_STAT14_tp_s15_grp_START (16)
#define SOC_PCTRL_PERI_STAT14_tp_s15_grp_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi_isp_dlock : 11;
        unsigned int peri_stat15 : 21;
    } reg;
} SOC_PCTRL_PERI_STAT15_UNION;
#endif
#define SOC_PCTRL_PERI_STAT15_axi_isp_dlock_START (0)
#define SOC_PCTRL_PERI_STAT15_axi_isp_dlock_END (10)
#define SOC_PCTRL_PERI_STAT15_peri_stat15_START (11)
#define SOC_PCTRL_PERI_STAT15_peri_stat15_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dphytx_pctrl_status1 : 32;
    } reg;
} SOC_PCTRL_PERI_STAT16_UNION;
#endif
#define SOC_PCTRL_PERI_STAT16_dphytx_pctrl_status1_START (0)
#define SOC_PCTRL_PERI_STAT16_dphytx_pctrl_status1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_stat17 : 32;
    } reg;
} SOC_PCTRL_PERI_STAT17_UNION;
#endif
#define SOC_PCTRL_PERI_STAT17_peri_stat17_START (0)
#define SOC_PCTRL_PERI_STAT17_peri_stat17_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_stat18 : 32;
    } reg;
} SOC_PCTRL_PERI_STAT18_UNION;
#endif
#define SOC_PCTRL_PERI_STAT18_peri_stat18_START (0)
#define SOC_PCTRL_PERI_STAT18_peri_stat18_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_stat19 : 32;
    } reg;
} SOC_PCTRL_PERI_STAT19_UNION;
#endif
#define SOC_PCTRL_PERI_STAT19_peri_stat19_START (0)
#define SOC_PCTRL_PERI_STAT19_peri_stat19_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb2_host_ctrl0 : 32;
    } reg;
} SOC_PCTRL_USB2_HOST_CTRL0_UNION;
#endif
#define SOC_PCTRL_USB2_HOST_CTRL0_usb2_host_ctrl0_START (0)
#define SOC_PCTRL_USB2_HOST_CTRL0_usb2_host_ctrl0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb2_host_ctrl1 : 32;
    } reg;
} SOC_PCTRL_USB2_HOST_CTRL1_UNION;
#endif
#define SOC_PCTRL_USB2_HOST_CTRL1_usb2_host_ctrl1_START (0)
#define SOC_PCTRL_USB2_HOST_CTRL1_usb2_host_ctrl1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int usb2_host_ctrl2 : 32;
    } reg;
} SOC_PCTRL_USB2_HOST_CTRL2_UNION;
#endif
#define SOC_PCTRL_USB2_HOST_CTRL2_usb2_host_ctrl2_START (0)
#define SOC_PCTRL_USB2_HOST_CTRL2_usb2_host_ctrl2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource0lock_cmd0 : 1;
        unsigned int resource0lock_id0 : 3;
        unsigned int resource0lock_cmd1 : 1;
        unsigned int resource0lock_id1 : 3;
        unsigned int resource0lock_cmd2 : 1;
        unsigned int resource0lock_id2 : 3;
        unsigned int resource0lock_cmd3 : 1;
        unsigned int resource0lock_id3 : 3;
        unsigned int resource0lock_cmd4 : 1;
        unsigned int resource0lock_id4 : 3;
        unsigned int resource0lock_cmd5 : 1;
        unsigned int resource0lock_id5 : 3;
        unsigned int resource0lock_cmd6 : 1;
        unsigned int resource0lock_id6 : 3;
        unsigned int resource0lock_cmd7 : 1;
        unsigned int resource0lock_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE0_LOCK_UNION;
#endif
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_cmd0_START (0)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_cmd0_END (0)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_id0_START (1)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_id0_END (3)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_cmd1_START (4)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_cmd1_END (4)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_id1_START (5)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_id1_END (7)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_cmd2_START (8)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_cmd2_END (8)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_id2_START (9)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_id2_END (11)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_cmd3_START (12)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_cmd3_END (12)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_id3_START (13)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_id3_END (15)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_cmd4_START (16)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_cmd4_END (16)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_id4_START (17)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_id4_END (19)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_cmd5_START (20)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_cmd5_END (20)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_id5_START (21)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_id5_END (23)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_cmd6_START (24)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_cmd6_END (24)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_id6_START (25)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_id6_END (27)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_cmd7_START (28)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_cmd7_END (28)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_id7_START (29)
#define SOC_PCTRL_RESOURCE0_LOCK_resource0lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource0unlock_cmd0 : 1;
        unsigned int resource0unlock_id0 : 3;
        unsigned int resource0unlock_cmd1 : 1;
        unsigned int resource0unlock_id1 : 3;
        unsigned int resource0unlock_cmd2 : 1;
        unsigned int resource0unlock_id2 : 3;
        unsigned int resource0unlock_cmd3 : 1;
        unsigned int resource0unlock_id3 : 3;
        unsigned int resource0unlock_cmd4 : 1;
        unsigned int resource0unlock_id4 : 3;
        unsigned int resource0unlock_cmd5 : 1;
        unsigned int resource0unlock_id5 : 3;
        unsigned int resource0unlock_cmd6 : 1;
        unsigned int resource0unlock_id6 : 3;
        unsigned int resource0unlock_cmd7 : 1;
        unsigned int resource0unlock_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE0_UNLOCK_UNION;
#endif
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_cmd0_START (0)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_cmd0_END (0)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_id0_START (1)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_id0_END (3)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_cmd1_START (4)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_cmd1_END (4)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_id1_START (5)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_id1_END (7)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_cmd2_START (8)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_cmd2_END (8)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_id2_START (9)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_id2_END (11)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_cmd3_START (12)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_cmd3_END (12)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_id3_START (13)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_id3_END (15)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_cmd4_START (16)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_cmd4_END (16)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_id4_START (17)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_id4_END (19)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_cmd5_START (20)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_cmd5_END (20)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_id5_START (21)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_id5_END (23)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_cmd6_START (24)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_cmd6_END (24)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_id6_START (25)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_id6_END (27)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_cmd7_START (28)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_cmd7_END (28)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_id7_START (29)
#define SOC_PCTRL_RESOURCE0_UNLOCK_resource0unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource0lock_st0 : 1;
        unsigned int resource0lock_st_id0 : 3;
        unsigned int resource0lock_st1 : 1;
        unsigned int resource0lock_st_id1 : 3;
        unsigned int resource0lock_st2 : 1;
        unsigned int resource0lock_st_id2 : 3;
        unsigned int resource0lock_st3 : 1;
        unsigned int resource0lock_st_id3 : 3;
        unsigned int resource0lock_st4 : 1;
        unsigned int resource0lock_st_id4 : 3;
        unsigned int resource0lock_st5 : 1;
        unsigned int resource0lock_st_id5 : 3;
        unsigned int resource0lock_st6 : 1;
        unsigned int resource0lock_st_id6 : 3;
        unsigned int resource0lock_st7 : 1;
        unsigned int resource0lock_st_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE0_LOCK_ST_UNION;
#endif
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st0_START (0)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st0_END (0)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st_id0_START (1)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st_id0_END (3)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st1_START (4)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st1_END (4)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st_id1_START (5)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st_id1_END (7)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st2_START (8)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st2_END (8)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st_id2_START (9)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st_id2_END (11)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st3_START (12)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st3_END (12)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st_id3_START (13)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st_id3_END (15)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st4_START (16)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st4_END (16)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st_id4_START (17)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st_id4_END (19)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st5_START (20)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st5_END (20)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st_id5_START (21)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st_id5_END (23)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st6_START (24)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st6_END (24)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st_id6_START (25)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st_id6_END (27)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st7_START (28)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st7_END (28)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st_id7_START (29)
#define SOC_PCTRL_RESOURCE0_LOCK_ST_resource0lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource1lock_cmd0 : 1;
        unsigned int resource1lock_id0 : 3;
        unsigned int resource1lock_cmd1 : 1;
        unsigned int resource1lock_id1 : 3;
        unsigned int resource1lock_cmd2 : 1;
        unsigned int resource1lock_id2 : 3;
        unsigned int resource1lock_cmd3 : 1;
        unsigned int resource1lock_id3 : 3;
        unsigned int resource1lock_cmd4 : 1;
        unsigned int resource1lock_id4 : 3;
        unsigned int resource1lock_cmd5 : 1;
        unsigned int resource1lock_id5 : 3;
        unsigned int resource1lock_cmd6 : 1;
        unsigned int resource1lock_id6 : 3;
        unsigned int resource1lock_cmd7 : 1;
        unsigned int resource1lock_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE1_LOCK_UNION;
#endif
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_cmd0_START (0)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_cmd0_END (0)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_id0_START (1)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_id0_END (3)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_cmd1_START (4)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_cmd1_END (4)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_id1_START (5)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_id1_END (7)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_cmd2_START (8)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_cmd2_END (8)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_id2_START (9)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_id2_END (11)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_cmd3_START (12)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_cmd3_END (12)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_id3_START (13)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_id3_END (15)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_cmd4_START (16)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_cmd4_END (16)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_id4_START (17)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_id4_END (19)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_cmd5_START (20)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_cmd5_END (20)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_id5_START (21)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_id5_END (23)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_cmd6_START (24)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_cmd6_END (24)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_id6_START (25)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_id6_END (27)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_cmd7_START (28)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_cmd7_END (28)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_id7_START (29)
#define SOC_PCTRL_RESOURCE1_LOCK_resource1lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource1unlock_cmd0 : 1;
        unsigned int resource1unlock_id0 : 3;
        unsigned int resource1unlock_cmd1 : 1;
        unsigned int resource1unlock_id1 : 3;
        unsigned int resource1unlock_cmd2 : 1;
        unsigned int resource1unlock_id2 : 3;
        unsigned int resource1unlock_cmd3 : 1;
        unsigned int resource1unlock_id3 : 3;
        unsigned int resource1unlock_cmd4 : 1;
        unsigned int resource1unlock_id4 : 3;
        unsigned int resource1unlock_cmd5 : 1;
        unsigned int resource1unlock_id5 : 3;
        unsigned int resource1unlock_cmd6 : 1;
        unsigned int resource1unlock_id6 : 3;
        unsigned int resource1unlock_cmd7 : 1;
        unsigned int resource1unlock_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE1_UNLOCK_UNION;
#endif
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_cmd0_START (0)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_cmd0_END (0)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_id0_START (1)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_id0_END (3)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_cmd1_START (4)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_cmd1_END (4)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_id1_START (5)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_id1_END (7)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_cmd2_START (8)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_cmd2_END (8)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_id2_START (9)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_id2_END (11)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_cmd3_START (12)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_cmd3_END (12)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_id3_START (13)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_id3_END (15)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_cmd4_START (16)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_cmd4_END (16)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_id4_START (17)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_id4_END (19)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_cmd5_START (20)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_cmd5_END (20)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_id5_START (21)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_id5_END (23)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_cmd6_START (24)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_cmd6_END (24)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_id6_START (25)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_id6_END (27)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_cmd7_START (28)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_cmd7_END (28)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_id7_START (29)
#define SOC_PCTRL_RESOURCE1_UNLOCK_resource1unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource1lock_st0 : 1;
        unsigned int resource1lock_st_id0 : 3;
        unsigned int resource1lock_st1 : 1;
        unsigned int resource1lock_st_id1 : 3;
        unsigned int resource1lock_st2 : 1;
        unsigned int resource1lock_st_id2 : 3;
        unsigned int resource1lock_st3 : 1;
        unsigned int resource1lock_st_id3 : 3;
        unsigned int resource1lock_st4 : 1;
        unsigned int resource1lock_st_id4 : 3;
        unsigned int resource1lock_st5 : 1;
        unsigned int resource1lock_st_id5 : 3;
        unsigned int resource1lock_st6 : 1;
        unsigned int resource1lock_st_id6 : 3;
        unsigned int resource1lock_st7 : 1;
        unsigned int resource1lock_st_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE1_LOCK_ST_UNION;
#endif
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st0_START (0)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st0_END (0)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st_id0_START (1)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st_id0_END (3)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st1_START (4)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st1_END (4)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st_id1_START (5)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st_id1_END (7)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st2_START (8)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st2_END (8)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st_id2_START (9)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st_id2_END (11)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st3_START (12)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st3_END (12)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st_id3_START (13)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st_id3_END (15)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st4_START (16)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st4_END (16)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st_id4_START (17)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st_id4_END (19)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st5_START (20)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st5_END (20)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st_id5_START (21)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st_id5_END (23)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st6_START (24)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st6_END (24)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st_id6_START (25)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st_id6_END (27)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st7_START (28)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st7_END (28)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st_id7_START (29)
#define SOC_PCTRL_RESOURCE1_LOCK_ST_resource1lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource2lock_cmd0 : 1;
        unsigned int resource2lock_id0 : 3;
        unsigned int resource2lock_cmd1 : 1;
        unsigned int resource2lock_id1 : 3;
        unsigned int resource2lock_cmd2 : 1;
        unsigned int resource2lock_id2 : 3;
        unsigned int resource2lock_cmd3 : 1;
        unsigned int resource2lock_id3 : 3;
        unsigned int resource2lock_cmd4 : 1;
        unsigned int resource2lock_id4 : 3;
        unsigned int resource2lock_cmd5 : 1;
        unsigned int resource2lock_id5 : 3;
        unsigned int resource2lock_cmd6 : 1;
        unsigned int resource2lock_id6 : 3;
        unsigned int resource2lock_cmd7 : 1;
        unsigned int resource2lock_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE2_LOCK_UNION;
#endif
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_cmd0_START (0)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_cmd0_END (0)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_id0_START (1)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_id0_END (3)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_cmd1_START (4)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_cmd1_END (4)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_id1_START (5)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_id1_END (7)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_cmd2_START (8)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_cmd2_END (8)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_id2_START (9)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_id2_END (11)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_cmd3_START (12)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_cmd3_END (12)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_id3_START (13)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_id3_END (15)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_cmd4_START (16)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_cmd4_END (16)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_id4_START (17)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_id4_END (19)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_cmd5_START (20)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_cmd5_END (20)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_id5_START (21)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_id5_END (23)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_cmd6_START (24)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_cmd6_END (24)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_id6_START (25)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_id6_END (27)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_cmd7_START (28)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_cmd7_END (28)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_id7_START (29)
#define SOC_PCTRL_RESOURCE2_LOCK_resource2lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource2unlock_cmd0 : 1;
        unsigned int resource2unlock_id0 : 3;
        unsigned int resource2unlock_cmd1 : 1;
        unsigned int resource2unlock_id1 : 3;
        unsigned int resource2unlock_cmd2 : 1;
        unsigned int resource2unlock_id2 : 3;
        unsigned int resource2unlock_cmd3 : 1;
        unsigned int resource2unlock_id3 : 3;
        unsigned int resource2unlock_cmd4 : 1;
        unsigned int resource2unlock_id4 : 3;
        unsigned int resource2unlock_cmd5 : 1;
        unsigned int resource2unlock_id5 : 3;
        unsigned int resource2unlock_cmd6 : 1;
        unsigned int resource2unlock_id6 : 3;
        unsigned int resource2unlock_cmd7 : 1;
        unsigned int resource2unlock_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE2_UNLOCK_UNION;
#endif
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_cmd0_START (0)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_cmd0_END (0)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_id0_START (1)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_id0_END (3)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_cmd1_START (4)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_cmd1_END (4)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_id1_START (5)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_id1_END (7)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_cmd2_START (8)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_cmd2_END (8)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_id2_START (9)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_id2_END (11)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_cmd3_START (12)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_cmd3_END (12)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_id3_START (13)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_id3_END (15)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_cmd4_START (16)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_cmd4_END (16)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_id4_START (17)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_id4_END (19)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_cmd5_START (20)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_cmd5_END (20)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_id5_START (21)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_id5_END (23)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_cmd6_START (24)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_cmd6_END (24)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_id6_START (25)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_id6_END (27)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_cmd7_START (28)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_cmd7_END (28)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_id7_START (29)
#define SOC_PCTRL_RESOURCE2_UNLOCK_resource2unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource2lock_st0 : 1;
        unsigned int resource2lock_st_id0 : 3;
        unsigned int resource2lock_st1 : 1;
        unsigned int resource2lock_st_id1 : 3;
        unsigned int resource2lock_st2 : 1;
        unsigned int resource2lock_st_id2 : 3;
        unsigned int resource2lock_st3 : 1;
        unsigned int resource2lock_st_id3 : 3;
        unsigned int resource2lock_st4 : 1;
        unsigned int resource2lock_st_id4 : 3;
        unsigned int resource2lock_st5 : 1;
        unsigned int resource2lock_st_id5 : 3;
        unsigned int resource2lock_st6 : 1;
        unsigned int resource2lock_st_id6 : 3;
        unsigned int resource2lock_st7 : 1;
        unsigned int resource2lock_st_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE2_LOCK_ST_UNION;
#endif
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st0_START (0)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st0_END (0)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st_id0_START (1)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st_id0_END (3)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st1_START (4)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st1_END (4)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st_id1_START (5)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st_id1_END (7)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st2_START (8)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st2_END (8)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st_id2_START (9)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st_id2_END (11)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st3_START (12)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st3_END (12)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st_id3_START (13)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st_id3_END (15)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st4_START (16)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st4_END (16)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st_id4_START (17)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st_id4_END (19)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st5_START (20)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st5_END (20)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st_id5_START (21)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st_id5_END (23)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st6_START (24)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st6_END (24)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st_id6_START (25)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st_id6_END (27)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st7_START (28)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st7_END (28)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st_id7_START (29)
#define SOC_PCTRL_RESOURCE2_LOCK_ST_resource2lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource3lock_cmd0 : 1;
        unsigned int resource3lock_id0 : 3;
        unsigned int resource3lock_cmd1 : 1;
        unsigned int resource3lock_id1 : 3;
        unsigned int resource3lock_cmd2 : 1;
        unsigned int resource3lock_id2 : 3;
        unsigned int resource3lock_cmd3 : 1;
        unsigned int resource3lock_id3 : 3;
        unsigned int resource3lock_cmd4 : 1;
        unsigned int resource3lock_id4 : 3;
        unsigned int resource3lock_cmd5 : 1;
        unsigned int resource3lock_id5 : 3;
        unsigned int resource3lock_cmd6 : 1;
        unsigned int resource3lock_id6 : 3;
        unsigned int resource3lock_cmd7 : 1;
        unsigned int resource3lock_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE3_LOCK_UNION;
#endif
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_cmd0_START (0)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_cmd0_END (0)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_id0_START (1)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_id0_END (3)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_cmd1_START (4)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_cmd1_END (4)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_id1_START (5)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_id1_END (7)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_cmd2_START (8)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_cmd2_END (8)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_id2_START (9)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_id2_END (11)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_cmd3_START (12)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_cmd3_END (12)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_id3_START (13)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_id3_END (15)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_cmd4_START (16)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_cmd4_END (16)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_id4_START (17)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_id4_END (19)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_cmd5_START (20)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_cmd5_END (20)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_id5_START (21)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_id5_END (23)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_cmd6_START (24)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_cmd6_END (24)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_id6_START (25)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_id6_END (27)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_cmd7_START (28)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_cmd7_END (28)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_id7_START (29)
#define SOC_PCTRL_RESOURCE3_LOCK_resource3lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource3unlock_cmd0 : 1;
        unsigned int resource3unlock_id0 : 3;
        unsigned int resource3unlock_cmd1 : 1;
        unsigned int resource3unlock_id1 : 3;
        unsigned int resource3unlock_cmd2 : 1;
        unsigned int resource3unlock_id2 : 3;
        unsigned int resource3unlock_cmd3 : 1;
        unsigned int resource3unlock_id3 : 3;
        unsigned int resource3unlock_cmd4 : 1;
        unsigned int resource3unlock_id4 : 3;
        unsigned int resource3unlock_cmd5 : 1;
        unsigned int resource3unlock_id5 : 3;
        unsigned int resource3unlock_cmd6 : 1;
        unsigned int resource3unlock_id6 : 3;
        unsigned int resource3unlock_cmd7 : 1;
        unsigned int resource3unlock_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE3_UNLOCK_UNION;
#endif
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_cmd0_START (0)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_cmd0_END (0)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_id0_START (1)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_id0_END (3)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_cmd1_START (4)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_cmd1_END (4)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_id1_START (5)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_id1_END (7)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_cmd2_START (8)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_cmd2_END (8)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_id2_START (9)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_id2_END (11)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_cmd3_START (12)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_cmd3_END (12)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_id3_START (13)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_id3_END (15)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_cmd4_START (16)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_cmd4_END (16)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_id4_START (17)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_id4_END (19)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_cmd5_START (20)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_cmd5_END (20)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_id5_START (21)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_id5_END (23)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_cmd6_START (24)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_cmd6_END (24)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_id6_START (25)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_id6_END (27)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_cmd7_START (28)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_cmd7_END (28)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_id7_START (29)
#define SOC_PCTRL_RESOURCE3_UNLOCK_resource3unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource3lock_st0 : 1;
        unsigned int resource3lock_st_id0 : 3;
        unsigned int resource3lock_st1 : 1;
        unsigned int resource3lock_st_id1 : 3;
        unsigned int resource3lock_st2 : 1;
        unsigned int resource3lock_st_id2 : 3;
        unsigned int resource3lock_st3 : 1;
        unsigned int resource3lock_st_id3 : 3;
        unsigned int resource3lock_st4 : 1;
        unsigned int resource3lock_st_id4 : 3;
        unsigned int resource3lock_st5 : 1;
        unsigned int resource3lock_st_id5 : 3;
        unsigned int resource3lock_st6 : 1;
        unsigned int resource3lock_st_id6 : 3;
        unsigned int resource3lock_st7 : 1;
        unsigned int resource3lock_st_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE3_LOCK_ST_UNION;
#endif
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st0_START (0)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st0_END (0)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st_id0_START (1)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st_id0_END (3)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st1_START (4)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st1_END (4)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st_id1_START (5)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st_id1_END (7)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st2_START (8)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st2_END (8)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st_id2_START (9)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st_id2_END (11)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st3_START (12)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st3_END (12)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st_id3_START (13)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st_id3_END (15)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st4_START (16)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st4_END (16)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st_id4_START (17)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st_id4_END (19)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st5_START (20)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st5_END (20)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st_id5_START (21)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st_id5_END (23)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st6_START (24)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st6_END (24)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st_id6_START (25)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st_id6_END (27)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st7_START (28)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st7_END (28)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st_id7_START (29)
#define SOC_PCTRL_RESOURCE3_LOCK_ST_resource3lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource4lock_cmd0 : 1;
        unsigned int resource4lock_id0 : 3;
        unsigned int resource4lock_cmd1 : 1;
        unsigned int resource4lock_id1 : 3;
        unsigned int resource4lock_cmd2 : 1;
        unsigned int resource4lock_id2 : 3;
        unsigned int resource4lock_cmd3 : 1;
        unsigned int resource4lock_id3 : 3;
        unsigned int resource4lock_cmd4 : 1;
        unsigned int resource4lock_id4 : 3;
        unsigned int resource4lock_cmd5 : 1;
        unsigned int resource4lock_id5 : 3;
        unsigned int resource4lock_cmd6 : 1;
        unsigned int resource4lock_id6 : 3;
        unsigned int resource4lock_cmd7 : 1;
        unsigned int resource4lock_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE4_LOCK_UNION;
#endif
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_cmd0_START (0)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_cmd0_END (0)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_id0_START (1)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_id0_END (3)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_cmd1_START (4)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_cmd1_END (4)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_id1_START (5)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_id1_END (7)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_cmd2_START (8)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_cmd2_END (8)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_id2_START (9)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_id2_END (11)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_cmd3_START (12)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_cmd3_END (12)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_id3_START (13)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_id3_END (15)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_cmd4_START (16)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_cmd4_END (16)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_id4_START (17)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_id4_END (19)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_cmd5_START (20)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_cmd5_END (20)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_id5_START (21)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_id5_END (23)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_cmd6_START (24)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_cmd6_END (24)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_id6_START (25)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_id6_END (27)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_cmd7_START (28)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_cmd7_END (28)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_id7_START (29)
#define SOC_PCTRL_RESOURCE4_LOCK_resource4lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource4unlock_cmd0 : 1;
        unsigned int resource4unlock_id0 : 3;
        unsigned int resource4unlock_cmd1 : 1;
        unsigned int resource4unlock_id1 : 3;
        unsigned int resource4unlock_cmd2 : 1;
        unsigned int resource4unlock_id2 : 3;
        unsigned int resource4unlock_cmd3 : 1;
        unsigned int resource4unlock_id3 : 3;
        unsigned int resource4unlock_cmd4 : 1;
        unsigned int resource4unlock_id4 : 3;
        unsigned int resource4unlock_cmd5 : 1;
        unsigned int resource4unlock_id5 : 3;
        unsigned int resource4unlock_cmd6 : 1;
        unsigned int resource4unlock_id6 : 3;
        unsigned int resource4unlock_cmd7 : 1;
        unsigned int resource4unlock_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE4_UNLOCK_UNION;
#endif
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_cmd0_START (0)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_cmd0_END (0)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_id0_START (1)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_id0_END (3)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_cmd1_START (4)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_cmd1_END (4)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_id1_START (5)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_id1_END (7)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_cmd2_START (8)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_cmd2_END (8)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_id2_START (9)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_id2_END (11)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_cmd3_START (12)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_cmd3_END (12)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_id3_START (13)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_id3_END (15)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_cmd4_START (16)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_cmd4_END (16)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_id4_START (17)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_id4_END (19)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_cmd5_START (20)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_cmd5_END (20)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_id5_START (21)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_id5_END (23)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_cmd6_START (24)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_cmd6_END (24)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_id6_START (25)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_id6_END (27)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_cmd7_START (28)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_cmd7_END (28)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_id7_START (29)
#define SOC_PCTRL_RESOURCE4_UNLOCK_resource4unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource4lock_st0 : 1;
        unsigned int resource4lock_st_id0 : 3;
        unsigned int resource4lock_st1 : 1;
        unsigned int resource4lock_st_id1 : 3;
        unsigned int resource4lock_st2 : 1;
        unsigned int resource4lock_st_id2 : 3;
        unsigned int resource4lock_st3 : 1;
        unsigned int resource4lock_st_id3 : 3;
        unsigned int resource4lock_st4 : 1;
        unsigned int resource4lock_st_id4 : 3;
        unsigned int resource4lock_st5 : 1;
        unsigned int resource4lock_st_id5 : 3;
        unsigned int resource4lock_st6 : 1;
        unsigned int resource4lock_st_id6 : 3;
        unsigned int resource4lock_st7 : 1;
        unsigned int resource4lock_st_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE4_LOCK_ST_UNION;
#endif
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st0_START (0)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st0_END (0)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st_id0_START (1)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st_id0_END (3)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st1_START (4)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st1_END (4)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st_id1_START (5)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st_id1_END (7)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st2_START (8)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st2_END (8)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st_id2_START (9)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st_id2_END (11)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st3_START (12)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st3_END (12)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st_id3_START (13)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st_id3_END (15)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st4_START (16)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st4_END (16)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st_id4_START (17)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st_id4_END (19)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st5_START (20)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st5_END (20)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st_id5_START (21)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st_id5_END (23)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st6_START (24)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st6_END (24)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st_id6_START (25)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st_id6_END (27)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st7_START (28)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st7_END (28)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st_id7_START (29)
#define SOC_PCTRL_RESOURCE4_LOCK_ST_resource4lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource5lock_cmd0 : 1;
        unsigned int resource5lock_id0 : 3;
        unsigned int resource5lock_cmd1 : 1;
        unsigned int resource5lock_id1 : 3;
        unsigned int resource5lock_cmd2 : 1;
        unsigned int resource5lock_id2 : 3;
        unsigned int resource5lock_cmd3 : 1;
        unsigned int resource5lock_id3 : 3;
        unsigned int resource5lock_cmd4 : 1;
        unsigned int resource5lock_id4 : 3;
        unsigned int resource5lock_cmd5 : 1;
        unsigned int resource5lock_id5 : 3;
        unsigned int resource5lock_cmd6 : 1;
        unsigned int resource5lock_id6 : 3;
        unsigned int resource5lock_cmd7 : 1;
        unsigned int resource5lock_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE5_LOCK_UNION;
#endif
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_cmd0_START (0)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_cmd0_END (0)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_id0_START (1)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_id0_END (3)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_cmd1_START (4)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_cmd1_END (4)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_id1_START (5)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_id1_END (7)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_cmd2_START (8)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_cmd2_END (8)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_id2_START (9)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_id2_END (11)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_cmd3_START (12)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_cmd3_END (12)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_id3_START (13)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_id3_END (15)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_cmd4_START (16)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_cmd4_END (16)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_id4_START (17)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_id4_END (19)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_cmd5_START (20)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_cmd5_END (20)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_id5_START (21)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_id5_END (23)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_cmd6_START (24)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_cmd6_END (24)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_id6_START (25)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_id6_END (27)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_cmd7_START (28)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_cmd7_END (28)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_id7_START (29)
#define SOC_PCTRL_RESOURCE5_LOCK_resource5lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource5unlock_cmd0 : 1;
        unsigned int resource5unlock_id0 : 3;
        unsigned int resource5unlock_cmd1 : 1;
        unsigned int resource5unlock_id1 : 3;
        unsigned int resource5unlock_cmd2 : 1;
        unsigned int resource5unlock_id2 : 3;
        unsigned int resource5unlock_cmd3 : 1;
        unsigned int resource5unlock_id3 : 3;
        unsigned int resource5unlock_cmd4 : 1;
        unsigned int resource5unlock_id4 : 3;
        unsigned int resource5unlock_cmd5 : 1;
        unsigned int resource5unlock_id5 : 3;
        unsigned int resource5unlock_cmd6 : 1;
        unsigned int resource5unlock_id6 : 3;
        unsigned int resource5unlock_cmd7 : 1;
        unsigned int resource5unlock_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE5_UNLOCK_UNION;
#endif
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_cmd0_START (0)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_cmd0_END (0)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_id0_START (1)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_id0_END (3)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_cmd1_START (4)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_cmd1_END (4)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_id1_START (5)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_id1_END (7)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_cmd2_START (8)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_cmd2_END (8)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_id2_START (9)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_id2_END (11)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_cmd3_START (12)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_cmd3_END (12)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_id3_START (13)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_id3_END (15)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_cmd4_START (16)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_cmd4_END (16)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_id4_START (17)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_id4_END (19)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_cmd5_START (20)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_cmd5_END (20)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_id5_START (21)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_id5_END (23)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_cmd6_START (24)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_cmd6_END (24)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_id6_START (25)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_id6_END (27)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_cmd7_START (28)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_cmd7_END (28)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_id7_START (29)
#define SOC_PCTRL_RESOURCE5_UNLOCK_resource5unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource5lock_st0 : 1;
        unsigned int resource5lock_st_id0 : 3;
        unsigned int resource5lock_st1 : 1;
        unsigned int resource5lock_st_id1 : 3;
        unsigned int resource5lock_st2 : 1;
        unsigned int resource5lock_st_id2 : 3;
        unsigned int resource5lock_st3 : 1;
        unsigned int resource5lock_st_id3 : 3;
        unsigned int resource5lock_st4 : 1;
        unsigned int resource5lock_st_id4 : 3;
        unsigned int resource5lock_st5 : 1;
        unsigned int resource5lock_st_id5 : 3;
        unsigned int resource5lock_st6 : 1;
        unsigned int resource5lock_st_id6 : 3;
        unsigned int resource5lock_st7 : 1;
        unsigned int resource5lock_st_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE5_LOCK_ST_UNION;
#endif
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st0_START (0)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st0_END (0)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st_id0_START (1)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st_id0_END (3)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st1_START (4)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st1_END (4)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st_id1_START (5)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st_id1_END (7)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st2_START (8)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st2_END (8)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st_id2_START (9)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st_id2_END (11)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st3_START (12)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st3_END (12)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st_id3_START (13)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st_id3_END (15)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st4_START (16)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st4_END (16)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st_id4_START (17)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st_id4_END (19)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st5_START (20)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st5_END (20)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st_id5_START (21)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st_id5_END (23)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st6_START (24)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st6_END (24)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st_id6_START (25)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st_id6_END (27)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st7_START (28)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st7_END (28)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st_id7_START (29)
#define SOC_PCTRL_RESOURCE5_LOCK_ST_resource5lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource6lock_cmd0 : 1;
        unsigned int resource6lock_id0 : 3;
        unsigned int resource6lock_cmd1 : 1;
        unsigned int resource6lock_id1 : 3;
        unsigned int resource6lock_cmd2 : 1;
        unsigned int resource6lock_id2 : 3;
        unsigned int resource6lock_cmd3 : 1;
        unsigned int resource6lock_id3 : 3;
        unsigned int resource6lock_cmd4 : 1;
        unsigned int resource6lock_id4 : 3;
        unsigned int resource6lock_cmd5 : 1;
        unsigned int resource6lock_id5 : 3;
        unsigned int resource6lock_cmd6 : 1;
        unsigned int resource6lock_id6 : 3;
        unsigned int resource6lock_cmd7 : 1;
        unsigned int resource6lock_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE6_LOCK_UNION;
#endif
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_cmd0_START (0)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_cmd0_END (0)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_id0_START (1)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_id0_END (3)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_cmd1_START (4)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_cmd1_END (4)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_id1_START (5)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_id1_END (7)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_cmd2_START (8)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_cmd2_END (8)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_id2_START (9)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_id2_END (11)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_cmd3_START (12)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_cmd3_END (12)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_id3_START (13)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_id3_END (15)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_cmd4_START (16)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_cmd4_END (16)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_id4_START (17)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_id4_END (19)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_cmd5_START (20)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_cmd5_END (20)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_id5_START (21)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_id5_END (23)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_cmd6_START (24)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_cmd6_END (24)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_id6_START (25)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_id6_END (27)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_cmd7_START (28)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_cmd7_END (28)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_id7_START (29)
#define SOC_PCTRL_RESOURCE6_LOCK_resource6lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource6unlock_cmd0 : 1;
        unsigned int resource6unlock_id0 : 3;
        unsigned int resource6unlock_cmd1 : 1;
        unsigned int resource6unlock_id1 : 3;
        unsigned int resource6unlock_cmd2 : 1;
        unsigned int resource6unlock_id2 : 3;
        unsigned int resource6unlock_cmd3 : 1;
        unsigned int resource6unlock_id3 : 3;
        unsigned int resource6unlock_cmd4 : 1;
        unsigned int resource6unlock_id4 : 3;
        unsigned int resource6unlock_cmd5 : 1;
        unsigned int resource6unlock_id5 : 3;
        unsigned int resource6unlock_cmd6 : 1;
        unsigned int resource6unlock_id6 : 3;
        unsigned int resource6unlock_cmd7 : 1;
        unsigned int resource6unlock_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE6_UNLOCK_UNION;
#endif
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_cmd0_START (0)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_cmd0_END (0)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_id0_START (1)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_id0_END (3)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_cmd1_START (4)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_cmd1_END (4)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_id1_START (5)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_id1_END (7)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_cmd2_START (8)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_cmd2_END (8)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_id2_START (9)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_id2_END (11)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_cmd3_START (12)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_cmd3_END (12)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_id3_START (13)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_id3_END (15)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_cmd4_START (16)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_cmd4_END (16)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_id4_START (17)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_id4_END (19)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_cmd5_START (20)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_cmd5_END (20)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_id5_START (21)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_id5_END (23)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_cmd6_START (24)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_cmd6_END (24)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_id6_START (25)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_id6_END (27)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_cmd7_START (28)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_cmd7_END (28)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_id7_START (29)
#define SOC_PCTRL_RESOURCE6_UNLOCK_resource6unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource6lock_st0 : 1;
        unsigned int resource6lock_st_id0 : 3;
        unsigned int resource6lock_st1 : 1;
        unsigned int resource6lock_st_id1 : 3;
        unsigned int resource6lock_st2 : 1;
        unsigned int resource6lock_st_id2 : 3;
        unsigned int resource6lock_st3 : 1;
        unsigned int resource6lock_st_id3 : 3;
        unsigned int resource6lock_st4 : 1;
        unsigned int resource6lock_st_id4 : 3;
        unsigned int resource6lock_st5 : 1;
        unsigned int resource6lock_st_id5 : 3;
        unsigned int resource6lock_st6 : 1;
        unsigned int resource6lock_st_id6 : 3;
        unsigned int resource6lock_st7 : 1;
        unsigned int resource6lock_st_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE6_LOCK_ST_UNION;
#endif
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st0_START (0)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st0_END (0)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st_id0_START (1)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st_id0_END (3)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st1_START (4)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st1_END (4)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st_id1_START (5)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st_id1_END (7)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st2_START (8)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st2_END (8)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st_id2_START (9)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st_id2_END (11)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st3_START (12)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st3_END (12)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st_id3_START (13)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st_id3_END (15)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st4_START (16)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st4_END (16)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st_id4_START (17)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st_id4_END (19)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st5_START (20)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st5_END (20)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st_id5_START (21)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st_id5_END (23)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st6_START (24)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st6_END (24)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st_id6_START (25)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st_id6_END (27)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st7_START (28)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st7_END (28)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st_id7_START (29)
#define SOC_PCTRL_RESOURCE6_LOCK_ST_resource6lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource7lock_cmd0 : 1;
        unsigned int resource7lock_id0 : 3;
        unsigned int resource7lock_cmd1 : 1;
        unsigned int resource7lock_id1 : 3;
        unsigned int resource7lock_cmd2 : 1;
        unsigned int resource7lock_id2 : 3;
        unsigned int resource7lock_cmd3 : 1;
        unsigned int resource7lock_id3 : 3;
        unsigned int resource7lock_cmd4 : 1;
        unsigned int resource7lock_id4 : 3;
        unsigned int resource7lock_cmd5 : 1;
        unsigned int resource7lock_id5 : 3;
        unsigned int resource7lock_cmd6 : 1;
        unsigned int resource7lock_id6 : 3;
        unsigned int resource7lock_cmd7 : 1;
        unsigned int resource7lock_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE7_LOCK_UNION;
#endif
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_cmd0_START (0)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_cmd0_END (0)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_id0_START (1)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_id0_END (3)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_cmd1_START (4)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_cmd1_END (4)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_id1_START (5)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_id1_END (7)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_cmd2_START (8)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_cmd2_END (8)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_id2_START (9)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_id2_END (11)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_cmd3_START (12)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_cmd3_END (12)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_id3_START (13)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_id3_END (15)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_cmd4_START (16)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_cmd4_END (16)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_id4_START (17)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_id4_END (19)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_cmd5_START (20)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_cmd5_END (20)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_id5_START (21)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_id5_END (23)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_cmd6_START (24)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_cmd6_END (24)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_id6_START (25)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_id6_END (27)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_cmd7_START (28)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_cmd7_END (28)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_id7_START (29)
#define SOC_PCTRL_RESOURCE7_LOCK_resource7lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource7unlock_cmd0 : 1;
        unsigned int resource7unlock_id0 : 3;
        unsigned int resource7unlock_cmd1 : 1;
        unsigned int resource7unlock_id1 : 3;
        unsigned int resource7unlock_cmd2 : 1;
        unsigned int resource7unlock_id2 : 3;
        unsigned int resource7unlock_cmd3 : 1;
        unsigned int resource7unlock_id3 : 3;
        unsigned int resource7unlock_cmd4 : 1;
        unsigned int resource7unlock_id4 : 3;
        unsigned int resource7unlock_cmd5 : 1;
        unsigned int resource7unlock_id5 : 3;
        unsigned int resource7unlock_cmd6 : 1;
        unsigned int resource7unlock_id6 : 3;
        unsigned int resource7unlock_cmd7 : 1;
        unsigned int resource7unlock_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE7_UNLOCK_UNION;
#endif
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_cmd0_START (0)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_cmd0_END (0)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_id0_START (1)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_id0_END (3)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_cmd1_START (4)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_cmd1_END (4)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_id1_START (5)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_id1_END (7)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_cmd2_START (8)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_cmd2_END (8)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_id2_START (9)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_id2_END (11)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_cmd3_START (12)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_cmd3_END (12)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_id3_START (13)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_id3_END (15)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_cmd4_START (16)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_cmd4_END (16)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_id4_START (17)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_id4_END (19)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_cmd5_START (20)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_cmd5_END (20)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_id5_START (21)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_id5_END (23)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_cmd6_START (24)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_cmd6_END (24)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_id6_START (25)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_id6_END (27)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_cmd7_START (28)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_cmd7_END (28)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_id7_START (29)
#define SOC_PCTRL_RESOURCE7_UNLOCK_resource7unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource7lock_st0 : 1;
        unsigned int resource7lock_st_id0 : 3;
        unsigned int resource7lock_st1 : 1;
        unsigned int resource7lock_st_id1 : 3;
        unsigned int resource7lock_st2 : 1;
        unsigned int resource7lock_st_id2 : 3;
        unsigned int resource7lock_st3 : 1;
        unsigned int resource7lock_st_id3 : 3;
        unsigned int resource7lock_st4 : 1;
        unsigned int resource7lock_st_id4 : 3;
        unsigned int resource7lock_st5 : 1;
        unsigned int resource7lock_st_id5 : 3;
        unsigned int resource7lock_st6 : 1;
        unsigned int resource7lock_st_id6 : 3;
        unsigned int resource7lock_st7 : 1;
        unsigned int resource7lock_st_id7 : 3;
    } reg;
} SOC_PCTRL_RESOURCE7_LOCK_ST_UNION;
#endif
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st0_START (0)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st0_END (0)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st_id0_START (1)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st_id0_END (3)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st1_START (4)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st1_END (4)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st_id1_START (5)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st_id1_END (7)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st2_START (8)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st2_END (8)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st_id2_START (9)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st_id2_END (11)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st3_START (12)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st3_END (12)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st_id3_START (13)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st_id3_END (15)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st4_START (16)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st4_END (16)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st_id4_START (17)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st_id4_END (19)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st5_START (20)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st5_END (20)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st_id5_START (21)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st_id5_END (23)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st6_START (24)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st6_END (24)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st_id6_START (25)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st_id6_END (27)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st7_START (28)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st7_END (28)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st_id7_START (29)
#define SOC_PCTRL_RESOURCE7_LOCK_ST_resource7lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lpm3_mid : 5;
        unsigned int iomcu_mid : 5;
        unsigned int noc_emmc01_mid : 5;
        unsigned int sd3_mid : 5;
        unsigned int sdio0_mid : 5;
        unsigned int dmac_mid : 5;
        unsigned int reserved : 2;
    } reg;
} SOC_PCTRL_PERI_CTRL5_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL5_lpm3_mid_START (0)
#define SOC_PCTRL_PERI_CTRL5_lpm3_mid_END (4)
#define SOC_PCTRL_PERI_CTRL5_iomcu_mid_START (5)
#define SOC_PCTRL_PERI_CTRL5_iomcu_mid_END (9)
#define SOC_PCTRL_PERI_CTRL5_noc_emmc01_mid_START (10)
#define SOC_PCTRL_PERI_CTRL5_noc_emmc01_mid_END (14)
#define SOC_PCTRL_PERI_CTRL5_sd3_mid_START (15)
#define SOC_PCTRL_PERI_CTRL5_sd3_mid_END (19)
#define SOC_PCTRL_PERI_CTRL5_sdio0_mid_START (20)
#define SOC_PCTRL_PERI_CTRL5_sdio0_mid_END (24)
#define SOC_PCTRL_PERI_CTRL5_dmac_mid_START (25)
#define SOC_PCTRL_PERI_CTRL5_dmac_mid_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int secp_mid : 5;
        unsigned int secs_mid : 5;
        unsigned int socp_mid : 5;
        unsigned int usb3otg_mid : 5;
        unsigned int sc_perf_stat_mid : 5;
        unsigned int cssys_mid : 5;
        unsigned int reserved : 2;
    } reg;
} SOC_PCTRL_PERI_CTRL6_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL6_secp_mid_START (0)
#define SOC_PCTRL_PERI_CTRL6_secp_mid_END (4)
#define SOC_PCTRL_PERI_CTRL6_secs_mid_START (5)
#define SOC_PCTRL_PERI_CTRL6_secs_mid_END (9)
#define SOC_PCTRL_PERI_CTRL6_socp_mid_START (10)
#define SOC_PCTRL_PERI_CTRL6_socp_mid_END (14)
#define SOC_PCTRL_PERI_CTRL6_usb3otg_mid_START (15)
#define SOC_PCTRL_PERI_CTRL6_usb3otg_mid_END (19)
#define SOC_PCTRL_PERI_CTRL6_sc_perf_stat_mid_START (20)
#define SOC_PCTRL_PERI_CTRL6_sc_perf_stat_mid_END (24)
#define SOC_PCTRL_PERI_CTRL6_cssys_mid_START (25)
#define SOC_PCTRL_PERI_CTRL6_cssys_mid_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int modem_arqos : 5;
        unsigned int modem_awqos : 5;
        unsigned int modemcpu_mid : 5;
        unsigned int reserved_0 : 5;
        unsigned int a53_mid1 : 5;
        unsigned int a57_mid0 : 5;
        unsigned int reserved_1 : 2;
    } reg;
} SOC_PCTRL_PERI_CTRL7_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL7_modem_arqos_START (0)
#define SOC_PCTRL_PERI_CTRL7_modem_arqos_END (4)
#define SOC_PCTRL_PERI_CTRL7_modem_awqos_START (5)
#define SOC_PCTRL_PERI_CTRL7_modem_awqos_END (9)
#define SOC_PCTRL_PERI_CTRL7_modemcpu_mid_START (10)
#define SOC_PCTRL_PERI_CTRL7_modemcpu_mid_END (14)
#define SOC_PCTRL_PERI_CTRL7_a53_mid1_START (20)
#define SOC_PCTRL_PERI_CTRL7_a53_mid1_END (24)
#define SOC_PCTRL_PERI_CTRL7_a57_mid0_START (25)
#define SOC_PCTRL_PERI_CTRL7_a57_mid0_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_armid0 : 5;
        unsigned int g3d_awmid0 : 5;
        unsigned int g3d_armid1 : 5;
        unsigned int g3d_awmid1 : 5;
        unsigned int g3d_arqos : 4;
        unsigned int reserved_0 : 1;
        unsigned int g3d_awqos : 4;
        unsigned int reserved_1 : 3;
    } reg;
} SOC_PCTRL_PERI_CTRL8_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL8_g3d_armid0_START (0)
#define SOC_PCTRL_PERI_CTRL8_g3d_armid0_END (4)
#define SOC_PCTRL_PERI_CTRL8_g3d_awmid0_START (5)
#define SOC_PCTRL_PERI_CTRL8_g3d_awmid0_END (9)
#define SOC_PCTRL_PERI_CTRL8_g3d_armid1_START (10)
#define SOC_PCTRL_PERI_CTRL8_g3d_armid1_END (14)
#define SOC_PCTRL_PERI_CTRL8_g3d_awmid1_START (15)
#define SOC_PCTRL_PERI_CTRL8_g3d_awmid1_END (19)
#define SOC_PCTRL_PERI_CTRL8_g3d_arqos_START (20)
#define SOC_PCTRL_PERI_CTRL8_g3d_arqos_END (23)
#define SOC_PCTRL_PERI_CTRL8_g3d_awqos_START (25)
#define SOC_PCTRL_PERI_CTRL8_g3d_awqos_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int asp_mid : 5;
        unsigned int reserved : 5;
        unsigned int venc_mid : 5;
        unsigned int a7_toviviobus_mid : 5;
        unsigned int a7_cfg_mid : 5;
        unsigned int djtag_mid : 5;
        unsigned int dss0_mid : 1;
        unsigned int dss1_mid : 1;
    } reg;
} SOC_PCTRL_PERI_CTRL9_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL9_asp_mid_START (0)
#define SOC_PCTRL_PERI_CTRL9_asp_mid_END (4)
#define SOC_PCTRL_PERI_CTRL9_venc_mid_START (10)
#define SOC_PCTRL_PERI_CTRL9_venc_mid_END (14)
#define SOC_PCTRL_PERI_CTRL9_a7_toviviobus_mid_START (15)
#define SOC_PCTRL_PERI_CTRL9_a7_toviviobus_mid_END (19)
#define SOC_PCTRL_PERI_CTRL9_a7_cfg_mid_START (20)
#define SOC_PCTRL_PERI_CTRL9_a7_cfg_mid_END (24)
#define SOC_PCTRL_PERI_CTRL9_djtag_mid_START (25)
#define SOC_PCTRL_PERI_CTRL9_djtag_mid_END (29)
#define SOC_PCTRL_PERI_CTRL9_dss0_mid_START (30)
#define SOC_PCTRL_PERI_CTRL9_dss0_mid_END (30)
#define SOC_PCTRL_PERI_CTRL9_dss1_mid_START (31)
#define SOC_PCTRL_PERI_CTRL9_dss1_mid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddrc0_mid : 1;
        unsigned int ddrc1_mid : 1;
        unsigned int ddrc2_mid : 1;
        unsigned int ddrc3_mid : 1;
        unsigned int ddrc4_mid : 1;
        unsigned int ddrc5_mid : 1;
        unsigned int ddrc6_mid : 1;
        unsigned int ddrc7_mid : 1;
        unsigned int ddrc8_mid : 1;
        unsigned int ddrc9_mid : 1;
        unsigned int ddrc10_mid : 1;
        unsigned int reserved_0 : 4;
        unsigned int sdio1_mid : 5;
        unsigned int sc_vdec_mid : 5;
        unsigned int sc_iomcu_dma_mid : 5;
        unsigned int reserved_1 : 2;
    } reg;
} SOC_PCTRL_PERI_CTRL10_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL10_ddrc0_mid_START (0)
#define SOC_PCTRL_PERI_CTRL10_ddrc0_mid_END (0)
#define SOC_PCTRL_PERI_CTRL10_ddrc1_mid_START (1)
#define SOC_PCTRL_PERI_CTRL10_ddrc1_mid_END (1)
#define SOC_PCTRL_PERI_CTRL10_ddrc2_mid_START (2)
#define SOC_PCTRL_PERI_CTRL10_ddrc2_mid_END (2)
#define SOC_PCTRL_PERI_CTRL10_ddrc3_mid_START (3)
#define SOC_PCTRL_PERI_CTRL10_ddrc3_mid_END (3)
#define SOC_PCTRL_PERI_CTRL10_ddrc4_mid_START (4)
#define SOC_PCTRL_PERI_CTRL10_ddrc4_mid_END (4)
#define SOC_PCTRL_PERI_CTRL10_ddrc5_mid_START (5)
#define SOC_PCTRL_PERI_CTRL10_ddrc5_mid_END (5)
#define SOC_PCTRL_PERI_CTRL10_ddrc6_mid_START (6)
#define SOC_PCTRL_PERI_CTRL10_ddrc6_mid_END (6)
#define SOC_PCTRL_PERI_CTRL10_ddrc7_mid_START (7)
#define SOC_PCTRL_PERI_CTRL10_ddrc7_mid_END (7)
#define SOC_PCTRL_PERI_CTRL10_ddrc8_mid_START (8)
#define SOC_PCTRL_PERI_CTRL10_ddrc8_mid_END (8)
#define SOC_PCTRL_PERI_CTRL10_ddrc9_mid_START (9)
#define SOC_PCTRL_PERI_CTRL10_ddrc9_mid_END (9)
#define SOC_PCTRL_PERI_CTRL10_ddrc10_mid_START (10)
#define SOC_PCTRL_PERI_CTRL10_ddrc10_mid_END (10)
#define SOC_PCTRL_PERI_CTRL10_sdio1_mid_START (15)
#define SOC_PCTRL_PERI_CTRL10_sdio1_mid_END (19)
#define SOC_PCTRL_PERI_CTRL10_sc_vdec_mid_START (20)
#define SOC_PCTRL_PERI_CTRL10_sc_vdec_mid_END (24)
#define SOC_PCTRL_PERI_CTRL10_sc_iomcu_dma_mid_START (25)
#define SOC_PCTRL_PERI_CTRL10_sc_iomcu_dma_mid_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int peri_ctrl11_cmd : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_PCTRL_PERI_CTRL11_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL11_peri_ctrl11_cmd_START (0)
#define SOC_PCTRL_PERI_CTRL11_peri_ctrl11_cmd_END (15)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
