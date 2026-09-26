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
#define SOC_PCTRL_PERI_STAT29_ADDR(base) ((base) + (0x100))
#define SOC_PCTRL_PERI_STAT30_ADDR(base) ((base) + (0x104))
#define SOC_PCTRL_PERI_STAT31_ADDR(base) ((base) + (0x108))
#define SOC_PCTRL_PERI_STAT32_ADDR(base) ((base) + (0x10C))
#define SOC_PCTRL_PERI_STAT33_ADDR(base) ((base) + (0x110))
#define SOC_PCTRL_PERI_STAT34_ADDR(base) ((base) + (0x114))
#define SOC_PCTRL_PERI_STAT35_ADDR(base) ((base) + (0x118))
#define SOC_PCTRL_PERI_STAT36_ADDR(base) ((base) + (0x11C))
#define SOC_PCTRL_PERI_STAT37_ADDR(base) ((base) + (0x120))
#define SOC_PCTRL_PERI_STAT38_ADDR(base) ((base) + (0x124))
#define SOC_PCTRL_PERI_STAT39_ADDR(base) ((base) + (0x128))
#define SOC_PCTRL_PERI_STAT40_ADDR(base) ((base) + (0x12C))
#define SOC_PCTRL_PERI_STAT41_ADDR(base) ((base) + (0x130))
#define SOC_PCTRL_PERI_STAT42_ADDR(base) ((base) + (0x134))
#define SOC_PCTRL_PERI_STAT43_ADDR(base) ((base) + (0x138))
#define SOC_PCTRL_PERI_STAT44_ADDR(base) ((base) + (0x13C))
#define SOC_PCTRL_PERI_STAT45_ADDR(base) ((base) + (0x140))
#define SOC_PCTRL_PERI_STAT46_ADDR(base) ((base) + (0x144))
#define SOC_PCTRL_PERI_STAT47_ADDR(base) ((base) + (0x148))
#define SOC_PCTRL_PERI_STAT48_ADDR(base) ((base) + (0x14C))
#define SOC_PCTRL_PERI_STAT49_ADDR(base) ((base) + (0x150))
#define SOC_PCTRL_PERI_STAT50_ADDR(base) ((base) + (0x154))
#define SOC_PCTRL_PERI_STAT51_ADDR(base) ((base) + (0x158))
#define SOC_PCTRL_PERI_STAT52_ADDR(base) ((base) + (0x15C))
#define SOC_PCTRL_PERI_STAT53_ADDR(base) ((base) + (0x160))
#define SOC_PCTRL_PERI_STAT54_ADDR(base) ((base) + (0x164))
#define SOC_PCTRL_PERI_STAT55_ADDR(base) ((base) + (0x168))
#define SOC_PCTRL_PERI_STAT56_ADDR(base) ((base) + (0x16C))
#define SOC_PCTRL_PERI_STAT57_ADDR(base) ((base) + (0x170))
#define SOC_PCTRL_PERI_STAT58_ADDR(base) ((base) + (0x174))
#define SOC_PCTRL_PERI_STAT59_ADDR(base) ((base) + (0x178))
#define SOC_PCTRL_PERI_STAT60_ADDR(base) ((base) + (0x17C))
#define SOC_PCTRL_PERI_STAT61_ADDR(base) ((base) + (0x180))
#define SOC_PCTRL_PERI_STAT62_ADDR(base) ((base) + (0x184))
#define SOC_PCTRL_PERI_STAT63_ADDR(base) ((base) + (0x188))
#define SOC_PCTRL_PERI_STAT64_ADDR(base) ((base) + (0x18C))
#define SOC_PCTRL_PERI_STAT66_ADDR(base) ((base) + (0x190))
#define SOC_PCTRL_PERI_STAT67_ADDR(base) ((base) + (0x194))
#define SOC_PCTRL_PERI_STAT68_ADDR(base) ((base) + (0x198))
#define SOC_PCTRL_PERI_STAT69_ADDR(base) ((base) + (0x19C))
#define SOC_PCTRL_PERI_STAT70_ADDR(base) ((base) + (0x1A0))
#define SOC_PCTRL_PERI_CTRL67_ADDR(base) ((base) + (0x200))
#define SOC_PCTRL_PERI_CTRL68_ADDR(base) ((base) + (0x204))
#define SOC_PCTRL_PERI_CTRL69_ADDR(base) ((base) + (0x208))
#define SOC_PCTRL_PERI_CTRL70_ADDR(base) ((base) + (0x20C))
#define SOC_PCTRL_PERI_CTRL71_ADDR(base) ((base) + (0x210))
#define SOC_PCTRL_PERI_CTRL72_ADDR(base) ((base) + (0x214))
#define SOC_PCTRL_PERI_CTRL73_ADDR(base) ((base) + (0x218))
#define SOC_PCTRL_PERI_CTRL74_ADDR(base) ((base) + (0x21C))
#define SOC_PCTRL_PERI_CTRL88_ADDR(base) ((base) + (0x220))
#define SOC_PCTRL_PERI_CTRL89_ADDR(base) ((base) + (0x224))
#define SOC_PCTRL_PERI_CTRL90_ADDR(base) ((base) + (0x228))
#define SOC_PCTRL_PERI_CTRL91_ADDR(base) ((base) + (0x22C))
#define SOC_PCTRL_PERI_CTRL92_ADDR(base) ((base) + (0x230))
#define SOC_PCTRL_PERI_CTRL93_ADDR(base) ((base) + (0x234))
#define SOC_PCTRL_PERI_CTRL95_ADDR(base) ((base) + (0x238))
#define SOC_PCTRL_PERI_CTRL96_ADDR(base) ((base) + (0x23C))
#define SOC_PCTRL_PERI_CTRL97_ADDR(base) ((base) + (0x240))
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
#define SOC_PCTRL_PERI_CTRL34_ADDR(base) ((base) + (0xC1C))
#define SOC_PCTRL_PERI_CTRL35_ADDR(base) ((base) + (0xC20))
#define SOC_PCTRL_PERI_CTRL38_ADDR(base) ((base) + (0xC2C))
#define SOC_PCTRL_PERI_CTRL39_ADDR(base) ((base) + (0xC30))
#define SOC_PCTRL_PERI_CTRL40_ADDR(base) ((base) + (0xC34))
#define SOC_PCTRL_PERI_CTRL41_ADDR(base) ((base) + (0xC38))
#define SOC_PCTRL_PERI_CTRL42_ADDR(base) ((base) + (0xC3C))
#define SOC_PCTRL_PERI_CTRL43_ADDR(base) ((base) + (0xC40))
#define SOC_PCTRL_PERI_CTRL44_ADDR(base) ((base) + (0xC44))
#define SOC_PCTRL_PERI_CTRL45_ADDR(base) ((base) + (0xC48))
#define SOC_PCTRL_PERI_CTRL46_ADDR(base) ((base) + (0xC4C))
#define SOC_PCTRL_PERI_CTRL47_ADDR(base) ((base) + (0xC50))
#define SOC_PCTRL_PERI_CTRL48_ADDR(base) ((base) + (0xC54))
#define SOC_PCTRL_PERI_CTRL49_ADDR(base) ((base) + (0xC58))
#define SOC_PCTRL_PERI_CTRL50_ADDR(base) ((base) + (0xC5C))
#define SOC_PCTRL_PERI_CTRL51_ADDR(base) ((base) + (0xC60))
#define SOC_PCTRL_PERI_CTRL52_ADDR(base) ((base) + (0xC64))
#define SOC_PCTRL_PERI_CTRL53_ADDR(base) ((base) + (0xC68))
#define SOC_PCTRL_PERI_CTRL54_ADDR(base) ((base) + (0xC6C))
#define SOC_PCTRL_PERI_CTRL55_ADDR(base) ((base) + (0xC70))
#define SOC_PCTRL_PERI_CTRL56_ADDR(base) ((base) + (0xC74))
#define SOC_PCTRL_PERI_CTRL57_ADDR(base) ((base) + (0xC78))
#define SOC_PCTRL_PERI_CTRL58_ADDR(base) ((base) + (0xC7C))
#define SOC_PCTRL_PERI_CTRL59_ADDR(base) ((base) + (0xC80))
#define SOC_PCTRL_PERI_CTRL60_ADDR(base) ((base) + (0xC84))
#define SOC_PCTRL_PERI_CTRL61_ADDR(base) ((base) + (0xC88))
#define SOC_PCTRL_PERI_CTRL62_ADDR(base) ((base) + (0xC8C))
#define SOC_PCTRL_PERI_CTRL63_ADDR(base) ((base) + (0xC90))
#define SOC_PCTRL_PERI_CTRL64_ADDR(base) ((base) + (0xC94))
#define SOC_PCTRL_PERI_CTRL65_ADDR(base) ((base) + (0xC98))
#define SOC_PCTRL_PERI_CTRL66_ADDR(base) ((base) + (0xC9C))
#define SOC_PCTRL_PERI_CTRL75_ADDR(base) ((base) + (0xCA0))
#define SOC_PCTRL_PERI_CTRL76_ADDR(base) ((base) + (0xCA4))
#define SOC_PCTRL_PERI_CTRL77_ADDR(base) ((base) + (0xCA8))
#define SOC_PCTRL_PERI_CTRL78_ADDR(base) ((base) + (0xCAC))
#define SOC_PCTRL_PERI_CTRL79_ADDR(base) ((base) + (0xCB0))
#define SOC_PCTRL_PERI_CTRL80_ADDR(base) ((base) + (0xCB4))
#define SOC_PCTRL_PERI_CTRL81_ADDR(base) ((base) + (0xCB8))
#define SOC_PCTRL_PERI_CTRL82_ADDR(base) ((base) + (0xCBC))
#define SOC_PCTRL_PERI_CTRL83_ADDR(base) ((base) + (0xCC0))
#define SOC_PCTRL_PERI_CTRL84_ADDR(base) ((base) + (0xCC4))
#define SOC_PCTRL_PERI_CTRL85_ADDR(base) ((base) + (0xCC8))
#define SOC_PCTRL_PERI_CTRL86_ADDR(base) ((base) + (0xCCC))
#define SOC_PCTRL_PERI_CTRL87_ADDR(base) ((base) + (0xCD0))
#define SOC_PCTRL_PERI_CTRL94_ADDR(base) ((base) + (0xCD4))
#define SOC_PCTRL_PERI_STAT20_ADDR(base) ((base) + (0xE00))
#define SOC_PCTRL_PERI_STAT21_ADDR(base) ((base) + (0xE04))
#define SOC_PCTRL_PERI_STAT22_ADDR(base) ((base) + (0xE08))
#define SOC_PCTRL_PERI_STAT23_ADDR(base) ((base) + (0xE0C))
#define SOC_PCTRL_PERI_STAT24_ADDR(base) ((base) + (0xE10))
#define SOC_PCTRL_PERI_STAT25_ADDR(base) ((base) + (0xE14))
#define SOC_PCTRL_PERI_STAT26_ADDR(base) ((base) + (0xE18))
#define SOC_PCTRL_PERI_STAT27_ADDR(base) ((base) + (0xE1C))
#define SOC_PCTRL_PERI_STAT28_ADDR(base) ((base) + (0xE20))
#define SOC_PCTRL_PERI_STAT65_ADDR(base) ((base) + (0xE24))
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
        unsigned int ufs_tcxo_en : 1;
        unsigned int usb_tcxo_en : 1;
        unsigned int reserved : 14;
        unsigned int peri_ctrl3_msk : 16;
    } reg;
} SOC_PCTRL_PERI_CTRL3_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL3_ufs_tcxo_en_START (0)
#define SOC_PCTRL_PERI_CTRL3_ufs_tcxo_en_END (0)
#define SOC_PCTRL_PERI_CTRL3_usb_tcxo_en_START (1)
#define SOC_PCTRL_PERI_CTRL3_usb_tcxo_en_END (1)
#define SOC_PCTRL_PERI_CTRL3_peri_ctrl3_msk_START (16)
#define SOC_PCTRL_PERI_CTRL3_peri_ctrl3_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL13_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL14_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int press_push_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PCTRL_PERI_CTRL15_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL15_press_push_en_START (0)
#define SOC_PCTRL_PERI_CTRL15_press_push_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spi4_cs_sel : 4;
        unsigned int spi3_cs_sel : 4;
        unsigned int reserved : 24;
    } reg;
} SOC_PCTRL_PERI_CTRL16_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL16_spi4_cs_sel_START (0)
#define SOC_PCTRL_PERI_CTRL16_spi4_cs_sel_END (3)
#define SOC_PCTRL_PERI_CTRL16_spi3_cs_sel_START (4)
#define SOC_PCTRL_PERI_CTRL16_spi3_cs_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int modem_debug_sel : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_PCTRL_PERI_CTRL17_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL17_modem_debug_sel_START (0)
#define SOC_PCTRL_PERI_CTRL17_modem_debug_sel_END (7)
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
        unsigned int g3d_drm_mode_cfg : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int g3d_pw_sel : 1;
        unsigned int g3d_slp_sel : 1;
        unsigned int reserved_3 : 3;
        unsigned int gpu_striping_granule : 3;
        unsigned int reserved_4 : 3;
        unsigned int reserved_5 : 3;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int gpu_x2p_gatedclock_en : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
    } reg;
} SOC_PCTRL_PERI_CTRL19_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL19_g3d_drm_mode_cfg_START (0)
#define SOC_PCTRL_PERI_CTRL19_g3d_drm_mode_cfg_END (0)
#define SOC_PCTRL_PERI_CTRL19_g3d_pw_sel_START (4)
#define SOC_PCTRL_PERI_CTRL19_g3d_pw_sel_END (4)
#define SOC_PCTRL_PERI_CTRL19_g3d_slp_sel_START (5)
#define SOC_PCTRL_PERI_CTRL19_g3d_slp_sel_END (5)
#define SOC_PCTRL_PERI_CTRL19_gpu_striping_granule_START (9)
#define SOC_PCTRL_PERI_CTRL19_gpu_striping_granule_END (11)
#define SOC_PCTRL_PERI_CTRL19_gpu_x2p_gatedclock_en_START (24)
#define SOC_PCTRL_PERI_CTRL19_gpu_x2p_gatedclock_en_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_test : 3;
        unsigned int g3d_tselw : 3;
        unsigned int g3d_tselr : 3;
        unsigned int g3d_twlpa : 1;
        unsigned int g3d_tsel : 3;
        unsigned int g3d_ls : 1;
        unsigned int g3d_wtsel : 2;
        unsigned int g3d_rtsel : 3;
        unsigned int g3d_cre : 1;
        unsigned int g3d_fca : 8;
        unsigned int reserved : 4;
    } reg;
} SOC_PCTRL_PERI_CTRL20_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL20_g3d_test_START (0)
#define SOC_PCTRL_PERI_CTRL20_g3d_test_END (2)
#define SOC_PCTRL_PERI_CTRL20_g3d_tselw_START (3)
#define SOC_PCTRL_PERI_CTRL20_g3d_tselw_END (5)
#define SOC_PCTRL_PERI_CTRL20_g3d_tselr_START (6)
#define SOC_PCTRL_PERI_CTRL20_g3d_tselr_END (8)
#define SOC_PCTRL_PERI_CTRL20_g3d_twlpa_START (9)
#define SOC_PCTRL_PERI_CTRL20_g3d_twlpa_END (9)
#define SOC_PCTRL_PERI_CTRL20_g3d_tsel_START (10)
#define SOC_PCTRL_PERI_CTRL20_g3d_tsel_END (12)
#define SOC_PCTRL_PERI_CTRL20_g3d_ls_START (13)
#define SOC_PCTRL_PERI_CTRL20_g3d_ls_END (13)
#define SOC_PCTRL_PERI_CTRL20_g3d_wtsel_START (14)
#define SOC_PCTRL_PERI_CTRL20_g3d_wtsel_END (15)
#define SOC_PCTRL_PERI_CTRL20_g3d_rtsel_START (16)
#define SOC_PCTRL_PERI_CTRL20_g3d_rtsel_END (18)
#define SOC_PCTRL_PERI_CTRL20_g3d_cre_START (19)
#define SOC_PCTRL_PERI_CTRL20_g3d_cre_END (19)
#define SOC_PCTRL_PERI_CTRL20_g3d_fca_START (20)
#define SOC_PCTRL_PERI_CTRL20_g3d_fca_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_mem_sd : 17;
        unsigned int reserved : 15;
    } reg;
} SOC_PCTRL_PERI_CTRL21_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL21_g3d_mem_sd_START (0)
#define SOC_PCTRL_PERI_CTRL21_g3d_mem_sd_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int dmac_ckgt_dis : 1;
        unsigned int pctrl_dphytx_ulpsexit0 : 1;
        unsigned int pctrl_dphytx_ulpsexit1 : 1;
        unsigned int reserved_2 : 27;
    } reg;
} SOC_PCTRL_PERI_CTRL23_UNION;
#endif
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
        unsigned int pmussi0_mst_cnt : 3;
        unsigned int djtag_mst_pstrb : 4;
        unsigned int i2c_freq_sel : 1;
        unsigned int reserved_0 : 1;
        unsigned int sdio0_resp_ctrl : 1;
        unsigned int sd3_resp_ctrl : 1;
        unsigned int reserved_1 : 1;
        unsigned int lpmcu_resp_ctrl : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int isp_axi_xdcdr_sel : 1;
        unsigned int reserved_4 : 3;
        unsigned int reserved_5 : 1;
        unsigned int codecssi_mst_cnt : 3;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int sc_clk_usb3phy_3mux1_sel : 2;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 3;
    } reg;
} SOC_PCTRL_PERI_CTRL24_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL24_pmussi0_mst_cnt_START (0)
#define SOC_PCTRL_PERI_CTRL24_pmussi0_mst_cnt_END (2)
#define SOC_PCTRL_PERI_CTRL24_djtag_mst_pstrb_START (3)
#define SOC_PCTRL_PERI_CTRL24_djtag_mst_pstrb_END (6)
#define SOC_PCTRL_PERI_CTRL24_i2c_freq_sel_START (7)
#define SOC_PCTRL_PERI_CTRL24_i2c_freq_sel_END (7)
#define SOC_PCTRL_PERI_CTRL24_sdio0_resp_ctrl_START (9)
#define SOC_PCTRL_PERI_CTRL24_sdio0_resp_ctrl_END (9)
#define SOC_PCTRL_PERI_CTRL24_sd3_resp_ctrl_START (10)
#define SOC_PCTRL_PERI_CTRL24_sd3_resp_ctrl_END (10)
#define SOC_PCTRL_PERI_CTRL24_lpmcu_resp_ctrl_START (12)
#define SOC_PCTRL_PERI_CTRL24_lpmcu_resp_ctrl_END (12)
#define SOC_PCTRL_PERI_CTRL24_isp_axi_xdcdr_sel_START (15)
#define SOC_PCTRL_PERI_CTRL24_isp_axi_xdcdr_sel_END (15)
#define SOC_PCTRL_PERI_CTRL24_codecssi_mst_cnt_START (20)
#define SOC_PCTRL_PERI_CTRL24_codecssi_mst_cnt_END (22)
#define SOC_PCTRL_PERI_CTRL24_sc_clk_usb3phy_3mux1_sel_START (25)
#define SOC_PCTRL_PERI_CTRL24_sc_clk_usb3phy_3mux1_sel_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 8;
        unsigned int secp_mem_ctrl_sd : 1;
        unsigned int secs_mem_ctrl_sd : 1;
        unsigned int mmbuf_sram_sd : 1;
        unsigned int smmu_integ_sec_override : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int sc_g3d_dw_axi_m1_cg_en : 1;
        unsigned int sc_g3d_dw_axi_s0_cg_en : 1;
        unsigned int sc_g3d_dw_axi_s1_cg_en : 1;
        unsigned int sc_g3d_dw_axi_s2_cg_en : 1;
        unsigned int reserved_6 : 6;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int sc_modem_ipc_auto_clk_gate_en : 1;
        unsigned int sc_modem_ipc_soft_gate_clk_en : 1;
        unsigned int reserved_9 : 1;
    } reg;
} SOC_PCTRL_PERI_CTRL25_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL25_secp_mem_ctrl_sd_START (8)
#define SOC_PCTRL_PERI_CTRL25_secp_mem_ctrl_sd_END (8)
#define SOC_PCTRL_PERI_CTRL25_secs_mem_ctrl_sd_START (9)
#define SOC_PCTRL_PERI_CTRL25_secs_mem_ctrl_sd_END (9)
#define SOC_PCTRL_PERI_CTRL25_mmbuf_sram_sd_START (10)
#define SOC_PCTRL_PERI_CTRL25_mmbuf_sram_sd_END (10)
#define SOC_PCTRL_PERI_CTRL25_smmu_integ_sec_override_START (11)
#define SOC_PCTRL_PERI_CTRL25_smmu_integ_sec_override_END (11)
#define SOC_PCTRL_PERI_CTRL25_sc_g3d_dw_axi_m1_cg_en_START (17)
#define SOC_PCTRL_PERI_CTRL25_sc_g3d_dw_axi_m1_cg_en_END (17)
#define SOC_PCTRL_PERI_CTRL25_sc_g3d_dw_axi_s0_cg_en_START (18)
#define SOC_PCTRL_PERI_CTRL25_sc_g3d_dw_axi_s0_cg_en_END (18)
#define SOC_PCTRL_PERI_CTRL25_sc_g3d_dw_axi_s1_cg_en_START (19)
#define SOC_PCTRL_PERI_CTRL25_sc_g3d_dw_axi_s1_cg_en_END (19)
#define SOC_PCTRL_PERI_CTRL25_sc_g3d_dw_axi_s2_cg_en_START (20)
#define SOC_PCTRL_PERI_CTRL25_sc_g3d_dw_axi_s2_cg_en_END (20)
#define SOC_PCTRL_PERI_CTRL25_sc_modem_ipc_auto_clk_gate_en_START (29)
#define SOC_PCTRL_PERI_CTRL25_sc_modem_ipc_auto_clk_gate_en_END (29)
#define SOC_PCTRL_PERI_CTRL25_sc_modem_ipc_soft_gate_clk_en_START (30)
#define SOC_PCTRL_PERI_CTRL25_sc_modem_ipc_soft_gate_clk_en_END (30)
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
        unsigned int ivp32_dw_axi_gatedclock_en : 1;
        unsigned int spi_tprf_slp : 1;
        unsigned int spi_tprf_dslp : 1;
        unsigned int spi_tprf_sd : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int g3d_spsram_slp : 1;
        unsigned int g3d_spsram_dslp : 1;
        unsigned int g3d_spsram_sd : 1;
        unsigned int reserved_4 : 7;
    } reg;
} SOC_PCTRL_PERI_CTRL26_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL26_pack_dbg_mode_en_START (0)
#define SOC_PCTRL_PERI_CTRL26_pack_dbg_mode_en_END (3)
#define SOC_PCTRL_PERI_CTRL26_pack_dbg_mode_sel_START (4)
#define SOC_PCTRL_PERI_CTRL26_pack_dbg_mode_sel_END (8)
#define SOC_PCTRL_PERI_CTRL26_isp_dw_axi_gatedclock_en_START (14)
#define SOC_PCTRL_PERI_CTRL26_isp_dw_axi_gatedclock_en_END (14)
#define SOC_PCTRL_PERI_CTRL26_ivp32_dw_axi_gatedclock_en_START (15)
#define SOC_PCTRL_PERI_CTRL26_ivp32_dw_axi_gatedclock_en_END (15)
#define SOC_PCTRL_PERI_CTRL26_spi_tprf_slp_START (16)
#define SOC_PCTRL_PERI_CTRL26_spi_tprf_slp_END (16)
#define SOC_PCTRL_PERI_CTRL26_spi_tprf_dslp_START (17)
#define SOC_PCTRL_PERI_CTRL26_spi_tprf_dslp_END (17)
#define SOC_PCTRL_PERI_CTRL26_spi_tprf_sd_START (18)
#define SOC_PCTRL_PERI_CTRL26_spi_tprf_sd_END (18)
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
        unsigned int reserved : 16;
        unsigned int pctrl_dphytx_ctrl1 : 16;
    } reg;
} SOC_PCTRL_PERI_CTRL30_UNION;
#endif
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
        unsigned int peri_tprf_kp : 2;
        unsigned int reserved_1 : 2;
        unsigned int peri_dpsram_rtsel : 2;
        unsigned int peri_dpsram_wtsel : 2;
        unsigned int peri_spsbsram_rtsel : 2;
        unsigned int peri_spsbsram_wtsel : 2;
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
#define SOC_PCTRL_PERI_CTRL31_peri_tprf_kp_END (21)
#define SOC_PCTRL_PERI_CTRL31_peri_dpsram_rtsel_START (24)
#define SOC_PCTRL_PERI_CTRL31_peri_dpsram_rtsel_END (25)
#define SOC_PCTRL_PERI_CTRL31_peri_dpsram_wtsel_START (26)
#define SOC_PCTRL_PERI_CTRL31_peri_dpsram_wtsel_END (27)
#define SOC_PCTRL_PERI_CTRL31_peri_spsbsram_rtsel_START (28)
#define SOC_PCTRL_PERI_CTRL31_peri_spsbsram_rtsel_END (29)
#define SOC_PCTRL_PERI_CTRL31_peri_spsbsram_wtsel_START (30)
#define SOC_PCTRL_PERI_CTRL31_peri_spsbsram_wtsel_END (31)
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
        unsigned int pctrl_dphytx_ctrl0 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_PCTRL_PERI_CTRL33_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL33_pctrl_dphytx_ctrl0_START (0)
#define SOC_PCTRL_PERI_CTRL33_pctrl_dphytx_ctrl0_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_dmabus_error_probe_observer_mainfault0 : 1;
        unsigned int int_sysbus_error_probe_observer_mainfault0 : 1;
        unsigned int int_aobus_error_probe_observer_mainfault0 : 1;
        unsigned int int_modemcfg_error_probe_observer_mainfault0 : 1;
        unsigned int reserved_0 : 1;
        unsigned int int_vcodec_error_probe_observer_mainfault0 : 1;
        unsigned int int_vivobus_error_probe_observer_mainfault0 : 1;
        unsigned int int_cfgbus_error_probe_observer_mainfault0 : 1;
        unsigned int int_mmc0bus_error_probe_observer_mainfault0 : 1;
        unsigned int int_mmc1bus_error_probe_observer_mainfault0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 2;
        unsigned int int_modem_transaction_probe_mainstatalarm : 1;
        unsigned int reserved_3 : 1;
        unsigned int intr_dss0_rd_transaction_probe_mainstatalarm : 1;
        unsigned int intr_dss0_wr_transaction_probe_mainstatalarm : 1;
        unsigned int intr_dss1_rd_transaction_probe_mainstatalarm : 1;
        unsigned int intr_dss1_wr_transaction_probe_mainstatalarm : 1;
        unsigned int intr_isp_wr_transaction_probe_mainstatalarm : 1;
        unsigned int intr_isp_rd_transaction_probe_mainstatalarm : 1;
        unsigned int reserved_4 : 2;
        unsigned int intr_a7_wr_transaction_probe_mainstatalarm : 1;
        unsigned int intr_a7_rd_transaction_probe_mainstatalarm : 1;
        unsigned int intr_dpctrl_transaction_probe_mainstatalarm : 1;
        unsigned int intr_pimon_transaction_probe_mainstatalarm : 1;
        unsigned int int_sysbusddrc_packet_probe_mainstatalarm : 1;
        unsigned int intr_asp_transaction_probe_mainstatalarm : 1;
        unsigned int reserved_5 : 1;
        unsigned int dphytx_pctrl_trstop_flag1 : 1;
        unsigned int dphytx_pctrl_trstop_flag0 : 1;
    } reg;
} SOC_PCTRL_PERI_STAT0_UNION;
#endif
#define SOC_PCTRL_PERI_STAT0_int_dmabus_error_probe_observer_mainfault0_START (0)
#define SOC_PCTRL_PERI_STAT0_int_dmabus_error_probe_observer_mainfault0_END (0)
#define SOC_PCTRL_PERI_STAT0_int_sysbus_error_probe_observer_mainfault0_START (1)
#define SOC_PCTRL_PERI_STAT0_int_sysbus_error_probe_observer_mainfault0_END (1)
#define SOC_PCTRL_PERI_STAT0_int_aobus_error_probe_observer_mainfault0_START (2)
#define SOC_PCTRL_PERI_STAT0_int_aobus_error_probe_observer_mainfault0_END (2)
#define SOC_PCTRL_PERI_STAT0_int_modemcfg_error_probe_observer_mainfault0_START (3)
#define SOC_PCTRL_PERI_STAT0_int_modemcfg_error_probe_observer_mainfault0_END (3)
#define SOC_PCTRL_PERI_STAT0_int_vcodec_error_probe_observer_mainfault0_START (5)
#define SOC_PCTRL_PERI_STAT0_int_vcodec_error_probe_observer_mainfault0_END (5)
#define SOC_PCTRL_PERI_STAT0_int_vivobus_error_probe_observer_mainfault0_START (6)
#define SOC_PCTRL_PERI_STAT0_int_vivobus_error_probe_observer_mainfault0_END (6)
#define SOC_PCTRL_PERI_STAT0_int_cfgbus_error_probe_observer_mainfault0_START (7)
#define SOC_PCTRL_PERI_STAT0_int_cfgbus_error_probe_observer_mainfault0_END (7)
#define SOC_PCTRL_PERI_STAT0_int_mmc0bus_error_probe_observer_mainfault0_START (8)
#define SOC_PCTRL_PERI_STAT0_int_mmc0bus_error_probe_observer_mainfault0_END (8)
#define SOC_PCTRL_PERI_STAT0_int_mmc1bus_error_probe_observer_mainfault0_START (9)
#define SOC_PCTRL_PERI_STAT0_int_mmc1bus_error_probe_observer_mainfault0_END (9)
#define SOC_PCTRL_PERI_STAT0_int_modem_transaction_probe_mainstatalarm_START (13)
#define SOC_PCTRL_PERI_STAT0_int_modem_transaction_probe_mainstatalarm_END (13)
#define SOC_PCTRL_PERI_STAT0_intr_dss0_rd_transaction_probe_mainstatalarm_START (15)
#define SOC_PCTRL_PERI_STAT0_intr_dss0_rd_transaction_probe_mainstatalarm_END (15)
#define SOC_PCTRL_PERI_STAT0_intr_dss0_wr_transaction_probe_mainstatalarm_START (16)
#define SOC_PCTRL_PERI_STAT0_intr_dss0_wr_transaction_probe_mainstatalarm_END (16)
#define SOC_PCTRL_PERI_STAT0_intr_dss1_rd_transaction_probe_mainstatalarm_START (17)
#define SOC_PCTRL_PERI_STAT0_intr_dss1_rd_transaction_probe_mainstatalarm_END (17)
#define SOC_PCTRL_PERI_STAT0_intr_dss1_wr_transaction_probe_mainstatalarm_START (18)
#define SOC_PCTRL_PERI_STAT0_intr_dss1_wr_transaction_probe_mainstatalarm_END (18)
#define SOC_PCTRL_PERI_STAT0_intr_isp_wr_transaction_probe_mainstatalarm_START (19)
#define SOC_PCTRL_PERI_STAT0_intr_isp_wr_transaction_probe_mainstatalarm_END (19)
#define SOC_PCTRL_PERI_STAT0_intr_isp_rd_transaction_probe_mainstatalarm_START (20)
#define SOC_PCTRL_PERI_STAT0_intr_isp_rd_transaction_probe_mainstatalarm_END (20)
#define SOC_PCTRL_PERI_STAT0_intr_a7_wr_transaction_probe_mainstatalarm_START (23)
#define SOC_PCTRL_PERI_STAT0_intr_a7_wr_transaction_probe_mainstatalarm_END (23)
#define SOC_PCTRL_PERI_STAT0_intr_a7_rd_transaction_probe_mainstatalarm_START (24)
#define SOC_PCTRL_PERI_STAT0_intr_a7_rd_transaction_probe_mainstatalarm_END (24)
#define SOC_PCTRL_PERI_STAT0_intr_dpctrl_transaction_probe_mainstatalarm_START (25)
#define SOC_PCTRL_PERI_STAT0_intr_dpctrl_transaction_probe_mainstatalarm_END (25)
#define SOC_PCTRL_PERI_STAT0_intr_pimon_transaction_probe_mainstatalarm_START (26)
#define SOC_PCTRL_PERI_STAT0_intr_pimon_transaction_probe_mainstatalarm_END (26)
#define SOC_PCTRL_PERI_STAT0_int_sysbusddrc_packet_probe_mainstatalarm_START (27)
#define SOC_PCTRL_PERI_STAT0_int_sysbusddrc_packet_probe_mainstatalarm_END (27)
#define SOC_PCTRL_PERI_STAT0_intr_asp_transaction_probe_mainstatalarm_START (28)
#define SOC_PCTRL_PERI_STAT0_intr_asp_transaction_probe_mainstatalarm_END (28)
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
        unsigned int cfgbus_servicetarget_mainnopendingtrans : 1;
        unsigned int noc_djtag_mst_i_mainnopendingtrans : 1;
        unsigned int noc_perfstat_i_mainnopendingtrans : 1;
        unsigned int noc_lpm3mst_i_mainnopendingtrans : 1;
        unsigned int noc_sd3stat_i_mainnopendingtrans : 1;
        unsigned int noc_usb3_i_mainnopendingtrans : 1;
        unsigned int noc_secp_i_mainnopendingtrans : 1;
        unsigned int noc_secs_i_mainnopendingtrans : 1;
        unsigned int noc_socp_i_mainnopendingtrans : 1;
        unsigned int noc_topcssys_i_mainnopendingtrans : 1;
        unsigned int dss_service_target_mainnopendingtrans : 1;
        unsigned int noc_dss0_rd_i_mainnopendingtrans : 1;
        unsigned int noc_dss0_wr_i_mainnopendingtrans : 1;
        unsigned int noc_dss1_rd_i_mainnopendingtrans : 1;
        unsigned int noc_ipf_i_mainnopendingtrans : 1;
        unsigned int noc_dss1_wr_i_mainnopendingtrans : 1;
        unsigned int noc_media_com_rd_i_mainnopendingtrans : 1;
        unsigned int dmabus_service_target_mainnopendingtrans : 1;
        unsigned int noc_dmacmst_i_mainnopendingtrans : 1;
        unsigned int isp_service_target_mainnopendingtrans : 1;
        unsigned int noc_isp_rd_i_mainnopendingtrans : 1;
        unsigned int noc_isp_wr_i_mainnopendingtrans : 1;
        unsigned int noc_a7tovivobus_rd_i_mainnopendingtrans : 1;
        unsigned int noc_a7tovivobus_wr_i_mainnopendingtrans : 1;
        unsigned int noc_isp1_rd_i_mainnopendingtrans : 1;
        unsigned int noc_isp1_wr_i_mainnopendingtrans : 1;
        unsigned int sc_noc_media_com_wr_i_mainnopendingtrans : 1;
        unsigned int noc_vcodeccfg_i_mainnopendingtrans : 1;
        unsigned int noc_sdio0_i_mainnopendingtrans : 1;
        unsigned int noc_pcie_i_mainnopendingtrans : 1;
        unsigned int vdec_service_target_mainnopendingtrans : 1;
        unsigned int noc_venc_i_mainnopendingtrans : 1;
    } reg;
} SOC_PCTRL_PERI_STAT2_UNION;
#endif
#define SOC_PCTRL_PERI_STAT2_cfgbus_servicetarget_mainnopendingtrans_START (0)
#define SOC_PCTRL_PERI_STAT2_cfgbus_servicetarget_mainnopendingtrans_END (0)
#define SOC_PCTRL_PERI_STAT2_noc_djtag_mst_i_mainnopendingtrans_START (1)
#define SOC_PCTRL_PERI_STAT2_noc_djtag_mst_i_mainnopendingtrans_END (1)
#define SOC_PCTRL_PERI_STAT2_noc_perfstat_i_mainnopendingtrans_START (2)
#define SOC_PCTRL_PERI_STAT2_noc_perfstat_i_mainnopendingtrans_END (2)
#define SOC_PCTRL_PERI_STAT2_noc_lpm3mst_i_mainnopendingtrans_START (3)
#define SOC_PCTRL_PERI_STAT2_noc_lpm3mst_i_mainnopendingtrans_END (3)
#define SOC_PCTRL_PERI_STAT2_noc_sd3stat_i_mainnopendingtrans_START (4)
#define SOC_PCTRL_PERI_STAT2_noc_sd3stat_i_mainnopendingtrans_END (4)
#define SOC_PCTRL_PERI_STAT2_noc_usb3_i_mainnopendingtrans_START (5)
#define SOC_PCTRL_PERI_STAT2_noc_usb3_i_mainnopendingtrans_END (5)
#define SOC_PCTRL_PERI_STAT2_noc_secp_i_mainnopendingtrans_START (6)
#define SOC_PCTRL_PERI_STAT2_noc_secp_i_mainnopendingtrans_END (6)
#define SOC_PCTRL_PERI_STAT2_noc_secs_i_mainnopendingtrans_START (7)
#define SOC_PCTRL_PERI_STAT2_noc_secs_i_mainnopendingtrans_END (7)
#define SOC_PCTRL_PERI_STAT2_noc_socp_i_mainnopendingtrans_START (8)
#define SOC_PCTRL_PERI_STAT2_noc_socp_i_mainnopendingtrans_END (8)
#define SOC_PCTRL_PERI_STAT2_noc_topcssys_i_mainnopendingtrans_START (9)
#define SOC_PCTRL_PERI_STAT2_noc_topcssys_i_mainnopendingtrans_END (9)
#define SOC_PCTRL_PERI_STAT2_dss_service_target_mainnopendingtrans_START (10)
#define SOC_PCTRL_PERI_STAT2_dss_service_target_mainnopendingtrans_END (10)
#define SOC_PCTRL_PERI_STAT2_noc_dss0_rd_i_mainnopendingtrans_START (11)
#define SOC_PCTRL_PERI_STAT2_noc_dss0_rd_i_mainnopendingtrans_END (11)
#define SOC_PCTRL_PERI_STAT2_noc_dss0_wr_i_mainnopendingtrans_START (12)
#define SOC_PCTRL_PERI_STAT2_noc_dss0_wr_i_mainnopendingtrans_END (12)
#define SOC_PCTRL_PERI_STAT2_noc_dss1_rd_i_mainnopendingtrans_START (13)
#define SOC_PCTRL_PERI_STAT2_noc_dss1_rd_i_mainnopendingtrans_END (13)
#define SOC_PCTRL_PERI_STAT2_noc_ipf_i_mainnopendingtrans_START (14)
#define SOC_PCTRL_PERI_STAT2_noc_ipf_i_mainnopendingtrans_END (14)
#define SOC_PCTRL_PERI_STAT2_noc_dss1_wr_i_mainnopendingtrans_START (15)
#define SOC_PCTRL_PERI_STAT2_noc_dss1_wr_i_mainnopendingtrans_END (15)
#define SOC_PCTRL_PERI_STAT2_noc_media_com_rd_i_mainnopendingtrans_START (16)
#define SOC_PCTRL_PERI_STAT2_noc_media_com_rd_i_mainnopendingtrans_END (16)
#define SOC_PCTRL_PERI_STAT2_dmabus_service_target_mainnopendingtrans_START (17)
#define SOC_PCTRL_PERI_STAT2_dmabus_service_target_mainnopendingtrans_END (17)
#define SOC_PCTRL_PERI_STAT2_noc_dmacmst_i_mainnopendingtrans_START (18)
#define SOC_PCTRL_PERI_STAT2_noc_dmacmst_i_mainnopendingtrans_END (18)
#define SOC_PCTRL_PERI_STAT2_isp_service_target_mainnopendingtrans_START (19)
#define SOC_PCTRL_PERI_STAT2_isp_service_target_mainnopendingtrans_END (19)
#define SOC_PCTRL_PERI_STAT2_noc_isp_rd_i_mainnopendingtrans_START (20)
#define SOC_PCTRL_PERI_STAT2_noc_isp_rd_i_mainnopendingtrans_END (20)
#define SOC_PCTRL_PERI_STAT2_noc_isp_wr_i_mainnopendingtrans_START (21)
#define SOC_PCTRL_PERI_STAT2_noc_isp_wr_i_mainnopendingtrans_END (21)
#define SOC_PCTRL_PERI_STAT2_noc_a7tovivobus_rd_i_mainnopendingtrans_START (22)
#define SOC_PCTRL_PERI_STAT2_noc_a7tovivobus_rd_i_mainnopendingtrans_END (22)
#define SOC_PCTRL_PERI_STAT2_noc_a7tovivobus_wr_i_mainnopendingtrans_START (23)
#define SOC_PCTRL_PERI_STAT2_noc_a7tovivobus_wr_i_mainnopendingtrans_END (23)
#define SOC_PCTRL_PERI_STAT2_noc_isp1_rd_i_mainnopendingtrans_START (24)
#define SOC_PCTRL_PERI_STAT2_noc_isp1_rd_i_mainnopendingtrans_END (24)
#define SOC_PCTRL_PERI_STAT2_noc_isp1_wr_i_mainnopendingtrans_START (25)
#define SOC_PCTRL_PERI_STAT2_noc_isp1_wr_i_mainnopendingtrans_END (25)
#define SOC_PCTRL_PERI_STAT2_sc_noc_media_com_wr_i_mainnopendingtrans_START (26)
#define SOC_PCTRL_PERI_STAT2_sc_noc_media_com_wr_i_mainnopendingtrans_END (26)
#define SOC_PCTRL_PERI_STAT2_noc_vcodeccfg_i_mainnopendingtrans_START (27)
#define SOC_PCTRL_PERI_STAT2_noc_vcodeccfg_i_mainnopendingtrans_END (27)
#define SOC_PCTRL_PERI_STAT2_noc_sdio0_i_mainnopendingtrans_START (28)
#define SOC_PCTRL_PERI_STAT2_noc_sdio0_i_mainnopendingtrans_END (28)
#define SOC_PCTRL_PERI_STAT2_noc_pcie_i_mainnopendingtrans_START (29)
#define SOC_PCTRL_PERI_STAT2_noc_pcie_i_mainnopendingtrans_END (29)
#define SOC_PCTRL_PERI_STAT2_vdec_service_target_mainnopendingtrans_START (30)
#define SOC_PCTRL_PERI_STAT2_vdec_service_target_mainnopendingtrans_END (30)
#define SOC_PCTRL_PERI_STAT2_noc_venc_i_mainnopendingtrans_START (31)
#define SOC_PCTRL_PERI_STAT2_noc_venc_i_mainnopendingtrans_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int venc_service_target_mainnopendingtrans : 1;
        unsigned int ics_service_target_mainnopendingtrans : 1;
        unsigned int noc_jpeg_fd_rd_i_mainnopendingtrans : 1;
        unsigned int noc_jpeg_fd_wr_i_mainnopendingtrans : 1;
        unsigned int noc_isp_cpu_per_i_mainnopendingtrans : 1;
        unsigned int modembus_service_target_mainnopendingtrans : 1;
        unsigned int reserved_0 : 1;
        unsigned int noc_cci2sysbus_i_mainnopendingtrans : 1;
        unsigned int noc_ufsmst_i_mainnopendingtrans : 1;
        unsigned int noc_modem_mst_i_mainnopendingtrans : 1;
        unsigned int sysbus_service_target_mainnopendingtrans : 1;
        unsigned int mmc0bus_service_target_mainnopendingtrans : 1;
        unsigned int mmc1bus_service_target_mainnopendingtrans : 1;
        unsigned int reserved_1 : 1;
        unsigned int noc_vivo_service_mainnopendingtrans : 1;
        unsigned int ivp32peribus_service_mainnopendingtrans : 1;
        unsigned int noc_ivp32mst_i_mainnopendingtrans : 1;
        unsigned int noc_cfg2media_i_mainnopendingtrans : 1;
        unsigned int sc_noc_mediatocfg_i_mainnopendingtrans : 1;
        unsigned int noc_vdec_i_mainnopendingtrans : 1;
        unsigned int noc_vcodecbus_target_mainnopendingtrans : 1;
        unsigned int reserved_2 : 1;
        unsigned int noc_ics_i_mainnopendingtrans : 1;
        unsigned int noc_pimon_i_mainnopendingtrans : 1;
        unsigned int reserved_3 : 1;
        unsigned int noc_dpctrl_i_mainnopendingtrans : 1;
        unsigned int reserved_4 : 1;
        unsigned int peri_stat3 : 5;
    } reg;
} SOC_PCTRL_PERI_STAT3_UNION;
#endif
#define SOC_PCTRL_PERI_STAT3_venc_service_target_mainnopendingtrans_START (0)
#define SOC_PCTRL_PERI_STAT3_venc_service_target_mainnopendingtrans_END (0)
#define SOC_PCTRL_PERI_STAT3_ics_service_target_mainnopendingtrans_START (1)
#define SOC_PCTRL_PERI_STAT3_ics_service_target_mainnopendingtrans_END (1)
#define SOC_PCTRL_PERI_STAT3_noc_jpeg_fd_rd_i_mainnopendingtrans_START (2)
#define SOC_PCTRL_PERI_STAT3_noc_jpeg_fd_rd_i_mainnopendingtrans_END (2)
#define SOC_PCTRL_PERI_STAT3_noc_jpeg_fd_wr_i_mainnopendingtrans_START (3)
#define SOC_PCTRL_PERI_STAT3_noc_jpeg_fd_wr_i_mainnopendingtrans_END (3)
#define SOC_PCTRL_PERI_STAT3_noc_isp_cpu_per_i_mainnopendingtrans_START (4)
#define SOC_PCTRL_PERI_STAT3_noc_isp_cpu_per_i_mainnopendingtrans_END (4)
#define SOC_PCTRL_PERI_STAT3_modembus_service_target_mainnopendingtrans_START (5)
#define SOC_PCTRL_PERI_STAT3_modembus_service_target_mainnopendingtrans_END (5)
#define SOC_PCTRL_PERI_STAT3_noc_cci2sysbus_i_mainnopendingtrans_START (7)
#define SOC_PCTRL_PERI_STAT3_noc_cci2sysbus_i_mainnopendingtrans_END (7)
#define SOC_PCTRL_PERI_STAT3_noc_ufsmst_i_mainnopendingtrans_START (8)
#define SOC_PCTRL_PERI_STAT3_noc_ufsmst_i_mainnopendingtrans_END (8)
#define SOC_PCTRL_PERI_STAT3_noc_modem_mst_i_mainnopendingtrans_START (9)
#define SOC_PCTRL_PERI_STAT3_noc_modem_mst_i_mainnopendingtrans_END (9)
#define SOC_PCTRL_PERI_STAT3_sysbus_service_target_mainnopendingtrans_START (10)
#define SOC_PCTRL_PERI_STAT3_sysbus_service_target_mainnopendingtrans_END (10)
#define SOC_PCTRL_PERI_STAT3_mmc0bus_service_target_mainnopendingtrans_START (11)
#define SOC_PCTRL_PERI_STAT3_mmc0bus_service_target_mainnopendingtrans_END (11)
#define SOC_PCTRL_PERI_STAT3_mmc1bus_service_target_mainnopendingtrans_START (12)
#define SOC_PCTRL_PERI_STAT3_mmc1bus_service_target_mainnopendingtrans_END (12)
#define SOC_PCTRL_PERI_STAT3_noc_vivo_service_mainnopendingtrans_START (14)
#define SOC_PCTRL_PERI_STAT3_noc_vivo_service_mainnopendingtrans_END (14)
#define SOC_PCTRL_PERI_STAT3_ivp32peribus_service_mainnopendingtrans_START (15)
#define SOC_PCTRL_PERI_STAT3_ivp32peribus_service_mainnopendingtrans_END (15)
#define SOC_PCTRL_PERI_STAT3_noc_ivp32mst_i_mainnopendingtrans_START (16)
#define SOC_PCTRL_PERI_STAT3_noc_ivp32mst_i_mainnopendingtrans_END (16)
#define SOC_PCTRL_PERI_STAT3_noc_cfg2media_i_mainnopendingtrans_START (17)
#define SOC_PCTRL_PERI_STAT3_noc_cfg2media_i_mainnopendingtrans_END (17)
#define SOC_PCTRL_PERI_STAT3_sc_noc_mediatocfg_i_mainnopendingtrans_START (18)
#define SOC_PCTRL_PERI_STAT3_sc_noc_mediatocfg_i_mainnopendingtrans_END (18)
#define SOC_PCTRL_PERI_STAT3_noc_vdec_i_mainnopendingtrans_START (19)
#define SOC_PCTRL_PERI_STAT3_noc_vdec_i_mainnopendingtrans_END (19)
#define SOC_PCTRL_PERI_STAT3_noc_vcodecbus_target_mainnopendingtrans_START (20)
#define SOC_PCTRL_PERI_STAT3_noc_vcodecbus_target_mainnopendingtrans_END (20)
#define SOC_PCTRL_PERI_STAT3_noc_ics_i_mainnopendingtrans_START (22)
#define SOC_PCTRL_PERI_STAT3_noc_ics_i_mainnopendingtrans_END (22)
#define SOC_PCTRL_PERI_STAT3_noc_pimon_i_mainnopendingtrans_START (23)
#define SOC_PCTRL_PERI_STAT3_noc_pimon_i_mainnopendingtrans_END (23)
#define SOC_PCTRL_PERI_STAT3_noc_dpctrl_i_mainnopendingtrans_START (25)
#define SOC_PCTRL_PERI_STAT3_noc_dpctrl_i_mainnopendingtrans_END (25)
#define SOC_PCTRL_PERI_STAT3_peri_stat3_START (27)
#define SOC_PCTRL_PERI_STAT3_peri_stat3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int psam_idle : 1;
        unsigned int ipf_idle : 1;
        unsigned int vdec_idle : 1;
        unsigned int reserved_0 : 1;
        unsigned int venc_idle : 1;
        unsigned int reserved_1 : 1;
        unsigned int socp_idle : 1;
        unsigned int ivp32_dwaxi_dlock_mst : 3;
        unsigned int ivp32_dwaxi_dlock_slv : 3;
        unsigned int ivp32_dwaxi_dlock_id : 6;
        unsigned int ivp32_dwaxi_dlock_wr : 1;
        unsigned int ivp32_dwaxi_dlock_irq : 1;
        unsigned int reserved_2 : 1;
        unsigned int intr_cci_err : 1;
        unsigned int mdm_ipc_s_clk_state : 3;
        unsigned int mdm_ipc_ns_clk_state : 3;
        unsigned int reserved_3 : 3;
    } reg;
} SOC_PCTRL_PERI_STAT4_UNION;
#endif
#define SOC_PCTRL_PERI_STAT4_psam_idle_START (0)
#define SOC_PCTRL_PERI_STAT4_psam_idle_END (0)
#define SOC_PCTRL_PERI_STAT4_ipf_idle_START (1)
#define SOC_PCTRL_PERI_STAT4_ipf_idle_END (1)
#define SOC_PCTRL_PERI_STAT4_vdec_idle_START (2)
#define SOC_PCTRL_PERI_STAT4_vdec_idle_END (2)
#define SOC_PCTRL_PERI_STAT4_venc_idle_START (4)
#define SOC_PCTRL_PERI_STAT4_venc_idle_END (4)
#define SOC_PCTRL_PERI_STAT4_socp_idle_START (6)
#define SOC_PCTRL_PERI_STAT4_socp_idle_END (6)
#define SOC_PCTRL_PERI_STAT4_ivp32_dwaxi_dlock_mst_START (7)
#define SOC_PCTRL_PERI_STAT4_ivp32_dwaxi_dlock_mst_END (9)
#define SOC_PCTRL_PERI_STAT4_ivp32_dwaxi_dlock_slv_START (10)
#define SOC_PCTRL_PERI_STAT4_ivp32_dwaxi_dlock_slv_END (12)
#define SOC_PCTRL_PERI_STAT4_ivp32_dwaxi_dlock_id_START (13)
#define SOC_PCTRL_PERI_STAT4_ivp32_dwaxi_dlock_id_END (18)
#define SOC_PCTRL_PERI_STAT4_ivp32_dwaxi_dlock_wr_START (19)
#define SOC_PCTRL_PERI_STAT4_ivp32_dwaxi_dlock_wr_END (19)
#define SOC_PCTRL_PERI_STAT4_ivp32_dwaxi_dlock_irq_START (20)
#define SOC_PCTRL_PERI_STAT4_ivp32_dwaxi_dlock_irq_END (20)
#define SOC_PCTRL_PERI_STAT4_intr_cci_err_START (22)
#define SOC_PCTRL_PERI_STAT4_intr_cci_err_END (22)
#define SOC_PCTRL_PERI_STAT4_mdm_ipc_s_clk_state_START (23)
#define SOC_PCTRL_PERI_STAT4_mdm_ipc_s_clk_state_END (25)
#define SOC_PCTRL_PERI_STAT4_mdm_ipc_ns_clk_state_START (26)
#define SOC_PCTRL_PERI_STAT4_mdm_ipc_ns_clk_state_END (28)
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
        unsigned int tp_s25_grp : 16;
    } reg;
} SOC_PCTRL_PERI_STAT12_UNION;
#endif
#define SOC_PCTRL_PERI_STAT12_tp_s10_grp_START (0)
#define SOC_PCTRL_PERI_STAT12_tp_s10_grp_END (15)
#define SOC_PCTRL_PERI_STAT12_tp_s25_grp_START (16)
#define SOC_PCTRL_PERI_STAT12_tp_s25_grp_END (31)
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
        unsigned int vdm_clk_en_0 : 1;
        unsigned int vdm_clk_en_1 : 1;
        unsigned int vdm_clk_en_2 : 1;
        unsigned int reserved : 18;
    } reg;
} SOC_PCTRL_PERI_STAT15_UNION;
#endif
#define SOC_PCTRL_PERI_STAT15_axi_isp_dlock_START (0)
#define SOC_PCTRL_PERI_STAT15_axi_isp_dlock_END (10)
#define SOC_PCTRL_PERI_STAT15_vdm_clk_en_0_START (11)
#define SOC_PCTRL_PERI_STAT15_vdm_clk_en_0_END (11)
#define SOC_PCTRL_PERI_STAT15_vdm_clk_en_1_START (12)
#define SOC_PCTRL_PERI_STAT15_vdm_clk_en_1_END (12)
#define SOC_PCTRL_PERI_STAT15_vdm_clk_en_2_START (13)
#define SOC_PCTRL_PERI_STAT15_vdm_clk_en_2_END (13)
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
        unsigned int tp_s3_grp : 16;
        unsigned int tp_s4_grp : 16;
    } reg;
} SOC_PCTRL_PERI_STAT17_UNION;
#endif
#define SOC_PCTRL_PERI_STAT17_tp_s3_grp_START (0)
#define SOC_PCTRL_PERI_STAT17_tp_s3_grp_END (15)
#define SOC_PCTRL_PERI_STAT17_tp_s4_grp_START (16)
#define SOC_PCTRL_PERI_STAT17_tp_s4_grp_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tp_s5_grp : 16;
        unsigned int tp_s6_grp : 16;
    } reg;
} SOC_PCTRL_PERI_STAT18_UNION;
#endif
#define SOC_PCTRL_PERI_STAT18_tp_s5_grp_START (0)
#define SOC_PCTRL_PERI_STAT18_tp_s5_grp_END (15)
#define SOC_PCTRL_PERI_STAT18_tp_s6_grp_START (16)
#define SOC_PCTRL_PERI_STAT18_tp_s6_grp_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tp_s11_grp : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_PCTRL_PERI_STAT19_UNION;
#endif
#define SOC_PCTRL_PERI_STAT19_tp_s11_grp_START (0)
#define SOC_PCTRL_PERI_STAT19_tp_s11_grp_END (15)
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
        unsigned int pa_valid_a53 : 1;
        unsigned int reserved_0 : 15;
        unsigned int pa_cfg_ready_a53 : 1;
        unsigned int reserved_1 : 15;
    } reg;
} SOC_PCTRL_PERI_STAT29_UNION;
#endif
#define SOC_PCTRL_PERI_STAT29_pa_valid_a53_START (0)
#define SOC_PCTRL_PERI_STAT29_pa_valid_a53_END (0)
#define SOC_PCTRL_PERI_STAT29_pa_cfg_ready_a53_START (16)
#define SOC_PCTRL_PERI_STAT29_pa_cfg_ready_a53_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pa_data_out_a53 : 9;
        unsigned int reserved : 23;
    } reg;
} SOC_PCTRL_PERI_STAT30_UNION;
#endif
#define SOC_PCTRL_PERI_STAT30_pa_data_out_a53_START (0)
#define SOC_PCTRL_PERI_STAT30_pa_data_out_a53_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT31_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT32_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT33_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT34_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT35_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT36_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT37_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT38_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT39_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT40_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pa_valid_g3d : 1;
        unsigned int reserved_0 : 15;
        unsigned int pa_cfg_ready_g3d : 1;
        unsigned int reserved_1 : 15;
    } reg;
} SOC_PCTRL_PERI_STAT41_UNION;
#endif
#define SOC_PCTRL_PERI_STAT41_pa_valid_g3d_START (0)
#define SOC_PCTRL_PERI_STAT41_pa_valid_g3d_END (0)
#define SOC_PCTRL_PERI_STAT41_pa_cfg_ready_g3d_START (16)
#define SOC_PCTRL_PERI_STAT41_pa_cfg_ready_g3d_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pa_data_out_g3d : 9;
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 21;
    } reg;
} SOC_PCTRL_PERI_STAT42_UNION;
#endif
#define SOC_PCTRL_PERI_STAT42_pa_data_out_g3d_START (0)
#define SOC_PCTRL_PERI_STAT42_pa_data_out_g3d_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT43_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT44_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT45_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pa_valid_a57 : 1;
        unsigned int reserved_0 : 15;
        unsigned int pa_cfg_ready_a57 : 1;
        unsigned int reserved_1 : 15;
    } reg;
} SOC_PCTRL_PERI_STAT46_UNION;
#endif
#define SOC_PCTRL_PERI_STAT46_pa_valid_a57_START (0)
#define SOC_PCTRL_PERI_STAT46_pa_valid_a57_END (0)
#define SOC_PCTRL_PERI_STAT46_pa_cfg_ready_a57_START (16)
#define SOC_PCTRL_PERI_STAT46_pa_cfg_ready_a57_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pa_data_out_a57 : 9;
        unsigned int reserved : 23;
    } reg;
} SOC_PCTRL_PERI_STAT47_UNION;
#endif
#define SOC_PCTRL_PERI_STAT47_pa_data_out_a57_START (0)
#define SOC_PCTRL_PERI_STAT47_pa_data_out_a57_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT48_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT49_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT50_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT51_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT52_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT53_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT54_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT55_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT56_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT57_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT58_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT59_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT60_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT61_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_STAT62_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int schiseestatus : 5;
        unsigned int reserved : 27;
    } reg;
} SOC_PCTRL_PERI_STAT63_UNION;
#endif
#define SOC_PCTRL_PERI_STAT63_schiseestatus_START (0)
#define SOC_PCTRL_PERI_STAT63_schiseestatus_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int repair_ack : 32;
    } reg;
} SOC_PCTRL_PERI_STAT64_UNION;
#endif
#define SOC_PCTRL_PERI_STAT64_repair_ack_START (0)
#define SOC_PCTRL_PERI_STAT64_repair_ack_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mrb_busy_flag : 32;
    } reg;
} SOC_PCTRL_PERI_STAT66_UNION;
#endif
#define SOC_PCTRL_PERI_STAT66_mrb_busy_flag_START (0)
#define SOC_PCTRL_PERI_STAT66_mrb_busy_flag_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int repair_done : 32;
    } reg;
} SOC_PCTRL_PERI_STAT67_UNION;
#endif
#define SOC_PCTRL_PERI_STAT67_repair_done_START (0)
#define SOC_PCTRL_PERI_STAT67_repair_done_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bisr_dbg_info : 32;
    } reg;
} SOC_PCTRL_PERI_STAT68_UNION;
#endif
#define SOC_PCTRL_PERI_STAT68_bisr_dbg_info_START (0)
#define SOC_PCTRL_PERI_STAT68_bisr_dbg_info_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_repair_err : 1;
        unsigned int pack_is0_err : 1;
        unsigned int repair_start_point_err : 1;
        unsigned int reserved : 29;
    } reg;
} SOC_PCTRL_PERI_STAT69_UNION;
#endif
#define SOC_PCTRL_PERI_STAT69_intr_repair_err_START (0)
#define SOC_PCTRL_PERI_STAT69_intr_repair_err_END (0)
#define SOC_PCTRL_PERI_STAT69_pack_is0_err_START (1)
#define SOC_PCTRL_PERI_STAT69_pack_is0_err_END (1)
#define SOC_PCTRL_PERI_STAT69_repair_start_point_err_START (2)
#define SOC_PCTRL_PERI_STAT69_repair_start_point_err_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_ddrc0_err : 1;
        unsigned int intr_ddrc1_err : 1;
        unsigned int intr_ddrc2_err : 1;
        unsigned int intr_ddrc3_err : 1;
        unsigned int intr_ddrphy : 4;
        unsigned int intr_uce0_wdog : 1;
        unsigned int intr_uce1_wdog : 1;
        unsigned int intr_uce2_wdog : 1;
        unsigned int intr_uce3_wdog : 1;
        unsigned int intr_ddrc_inban_ecc_err : 4;
        unsigned int intr_qosbuf0 : 1;
        unsigned int intr_qosbuf1 : 1;
        unsigned int reserved : 14;
    } reg;
} SOC_PCTRL_PERI_STAT70_UNION;
#endif
#define SOC_PCTRL_PERI_STAT70_intr_ddrc0_err_START (0)
#define SOC_PCTRL_PERI_STAT70_intr_ddrc0_err_END (0)
#define SOC_PCTRL_PERI_STAT70_intr_ddrc1_err_START (1)
#define SOC_PCTRL_PERI_STAT70_intr_ddrc1_err_END (1)
#define SOC_PCTRL_PERI_STAT70_intr_ddrc2_err_START (2)
#define SOC_PCTRL_PERI_STAT70_intr_ddrc2_err_END (2)
#define SOC_PCTRL_PERI_STAT70_intr_ddrc3_err_START (3)
#define SOC_PCTRL_PERI_STAT70_intr_ddrc3_err_END (3)
#define SOC_PCTRL_PERI_STAT70_intr_ddrphy_START (4)
#define SOC_PCTRL_PERI_STAT70_intr_ddrphy_END (7)
#define SOC_PCTRL_PERI_STAT70_intr_uce0_wdog_START (8)
#define SOC_PCTRL_PERI_STAT70_intr_uce0_wdog_END (8)
#define SOC_PCTRL_PERI_STAT70_intr_uce1_wdog_START (9)
#define SOC_PCTRL_PERI_STAT70_intr_uce1_wdog_END (9)
#define SOC_PCTRL_PERI_STAT70_intr_uce2_wdog_START (10)
#define SOC_PCTRL_PERI_STAT70_intr_uce2_wdog_END (10)
#define SOC_PCTRL_PERI_STAT70_intr_uce3_wdog_START (11)
#define SOC_PCTRL_PERI_STAT70_intr_uce3_wdog_END (11)
#define SOC_PCTRL_PERI_STAT70_intr_ddrc_inban_ecc_err_START (12)
#define SOC_PCTRL_PERI_STAT70_intr_ddrc_inban_ecc_err_END (15)
#define SOC_PCTRL_PERI_STAT70_intr_qosbuf0_START (16)
#define SOC_PCTRL_PERI_STAT70_intr_qosbuf0_END (16)
#define SOC_PCTRL_PERI_STAT70_intr_qosbuf1_START (17)
#define SOC_PCTRL_PERI_STAT70_intr_qosbuf1_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pa_ctrl_a53 : 6;
        unsigned int reserved_0 : 10;
        unsigned int pa_en_a53 : 1;
        unsigned int reserved_1 : 15;
    } reg;
} SOC_PCTRL_PERI_CTRL67_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL67_pa_ctrl_a53_START (0)
#define SOC_PCTRL_PERI_CTRL67_pa_ctrl_a53_END (5)
#define SOC_PCTRL_PERI_CTRL67_pa_en_a53_START (16)
#define SOC_PCTRL_PERI_CTRL67_pa_en_a53_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pa_cmd_a53 : 4;
        unsigned int pa_msg_a53 : 4;
        unsigned int reserved_0 : 8;
        unsigned int pc_cfg_valid_a53 : 1;
        unsigned int reserved_1 : 15;
    } reg;
} SOC_PCTRL_PERI_CTRL68_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL68_pa_cmd_a53_START (0)
#define SOC_PCTRL_PERI_CTRL68_pa_cmd_a53_END (3)
#define SOC_PCTRL_PERI_CTRL68_pa_msg_a53_START (4)
#define SOC_PCTRL_PERI_CTRL68_pa_msg_a53_END (7)
#define SOC_PCTRL_PERI_CTRL68_pc_cfg_valid_a53_START (16)
#define SOC_PCTRL_PERI_CTRL68_pc_cfg_valid_a53_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pa_sel_a53 : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_PCTRL_PERI_CTRL69_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL69_pa_sel_a53_START (0)
#define SOC_PCTRL_PERI_CTRL69_pa_sel_a53_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL70_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int schiseectrl : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_PCTRL_PERI_CTRL71_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL71_schiseectrl_START (0)
#define SOC_PCTRL_PERI_CTRL71_schiseectrl_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hisee_jtag_sel : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PCTRL_PERI_CTRL72_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL72_hisee_jtag_sel_START (0)
#define SOC_PCTRL_PERI_CTRL72_hisee_jtag_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int schiseefamactrl : 7;
        unsigned int reserved : 25;
    } reg;
} SOC_PCTRL_PERI_CTRL73_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL73_schiseefamactrl_START (0)
#define SOC_PCTRL_PERI_CTRL73_schiseefamactrl_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 12;
        unsigned int hisee_qos : 2;
        unsigned int reserved_1: 18;
    } reg;
} SOC_PCTRL_PERI_CTRL74_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL74_hisee_qos_START (12)
#define SOC_PCTRL_PERI_CTRL74_hisee_qos_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tprf_kp : 3;
        unsigned int tprf_rct : 2;
        unsigned int tprf_wct : 2;
        unsigned int reserved : 25;
    } reg;
} SOC_PCTRL_PERI_CTRL88_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL88_tprf_kp_START (0)
#define SOC_PCTRL_PERI_CTRL88_tprf_kp_END (2)
#define SOC_PCTRL_PERI_CTRL88_tprf_rct_START (3)
#define SOC_PCTRL_PERI_CTRL88_tprf_rct_END (4)
#define SOC_PCTRL_PERI_CTRL88_tprf_wct_START (5)
#define SOC_PCTRL_PERI_CTRL88_tprf_wct_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 14;
        unsigned int intr_repair_err_msk : 1;
        unsigned int bisr_dbg_sel : 4;
        unsigned int reserved_1 : 13;
    } reg;
} SOC_PCTRL_PERI_CTRL89_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL89_intr_repair_err_msk_START (14)
#define SOC_PCTRL_PERI_CTRL89_intr_repair_err_msk_END (14)
#define SOC_PCTRL_PERI_CTRL89_bisr_dbg_sel_START (15)
#define SOC_PCTRL_PERI_CTRL89_bisr_dbg_sel_END (18)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pa_ctrl_g3d : 6;
        unsigned int reserved_0 : 10;
        unsigned int pa_en_g3d : 1;
        unsigned int reserved_1 : 15;
    } reg;
} SOC_PCTRL_PERI_CTRL90_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL90_pa_ctrl_g3d_START (0)
#define SOC_PCTRL_PERI_CTRL90_pa_ctrl_g3d_END (5)
#define SOC_PCTRL_PERI_CTRL90_pa_en_g3d_START (16)
#define SOC_PCTRL_PERI_CTRL90_pa_en_g3d_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pa_cmd_g3d : 4;
        unsigned int pa_msg_g3d : 4;
        unsigned int reserved_0 : 8;
        unsigned int pa_cfg_vld_g3d : 1;
        unsigned int reserved_1 : 15;
    } reg;
} SOC_PCTRL_PERI_CTRL91_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL91_pa_cmd_g3d_START (0)
#define SOC_PCTRL_PERI_CTRL91_pa_cmd_g3d_END (3)
#define SOC_PCTRL_PERI_CTRL91_pa_msg_g3d_START (4)
#define SOC_PCTRL_PERI_CTRL91_pa_msg_g3d_END (7)
#define SOC_PCTRL_PERI_CTRL91_pa_cfg_vld_g3d_START (16)
#define SOC_PCTRL_PERI_CTRL91_pa_cfg_vld_g3d_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pa_sel_g3d : 6;
        unsigned int reserved : 26;
    } reg;
} SOC_PCTRL_PERI_CTRL92_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL92_pa_sel_g3d_START (0)
#define SOC_PCTRL_PERI_CTRL92_pa_sel_g3d_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_ctrl_reserved : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL93_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL93_g3d_ctrl_reserved_START (0)
#define SOC_PCTRL_PERI_CTRL93_g3d_ctrl_reserved_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pa_ctrl_a57 : 6;
        unsigned int reserved_0 : 10;
        unsigned int pa_en_a57 : 1;
        unsigned int reserved_1 : 15;
    } reg;
} SOC_PCTRL_PERI_CTRL95_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL95_pa_ctrl_a57_START (0)
#define SOC_PCTRL_PERI_CTRL95_pa_ctrl_a57_END (5)
#define SOC_PCTRL_PERI_CTRL95_pa_en_a57_START (16)
#define SOC_PCTRL_PERI_CTRL95_pa_en_a57_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pa_cmd_a57 : 4;
        unsigned int pa_msg_a57 : 4;
        unsigned int reserved_0 : 8;
        unsigned int pc_cfg_valid_a57 : 1;
        unsigned int reserved_1 : 15;
    } reg;
} SOC_PCTRL_PERI_CTRL96_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL96_pa_cmd_a57_START (0)
#define SOC_PCTRL_PERI_CTRL96_pa_cmd_a57_END (3)
#define SOC_PCTRL_PERI_CTRL96_pa_msg_a57_START (4)
#define SOC_PCTRL_PERI_CTRL96_pa_msg_a57_END (7)
#define SOC_PCTRL_PERI_CTRL96_pc_cfg_valid_a57_START (16)
#define SOC_PCTRL_PERI_CTRL96_pc_cfg_valid_a57_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pa_sel_a57 : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_PCTRL_PERI_CTRL97_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL97_pa_sel_a57_START (0)
#define SOC_PCTRL_PERI_CTRL97_pa_sel_a57_END (3)
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
        unsigned int lpm3_mid : 6;
        unsigned int iomcu_mid : 6;
        unsigned int ufs_mid : 6;
        unsigned int sd3_mid : 6;
        unsigned int sdio_mid : 6;
        unsigned int reserved : 2;
    } reg;
} SOC_PCTRL_PERI_CTRL5_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL5_lpm3_mid_START (0)
#define SOC_PCTRL_PERI_CTRL5_lpm3_mid_END (5)
#define SOC_PCTRL_PERI_CTRL5_iomcu_mid_START (6)
#define SOC_PCTRL_PERI_CTRL5_iomcu_mid_END (11)
#define SOC_PCTRL_PERI_CTRL5_ufs_mid_START (12)
#define SOC_PCTRL_PERI_CTRL5_ufs_mid_END (17)
#define SOC_PCTRL_PERI_CTRL5_sd3_mid_START (18)
#define SOC_PCTRL_PERI_CTRL5_sd3_mid_END (23)
#define SOC_PCTRL_PERI_CTRL5_sdio_mid_START (24)
#define SOC_PCTRL_PERI_CTRL5_sdio_mid_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int secs_mid : 6;
        unsigned int secp_mid : 6;
        unsigned int socp_mid : 6;
        unsigned int usb3otg_mid : 6;
        unsigned int sc_perf_stat_mid : 6;
        unsigned int reserved : 2;
    } reg;
} SOC_PCTRL_PERI_CTRL6_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL6_secs_mid_START (0)
#define SOC_PCTRL_PERI_CTRL6_secs_mid_END (5)
#define SOC_PCTRL_PERI_CTRL6_secp_mid_START (6)
#define SOC_PCTRL_PERI_CTRL6_secp_mid_END (11)
#define SOC_PCTRL_PERI_CTRL6_socp_mid_START (12)
#define SOC_PCTRL_PERI_CTRL6_socp_mid_END (17)
#define SOC_PCTRL_PERI_CTRL6_usb3otg_mid_START (18)
#define SOC_PCTRL_PERI_CTRL6_usb3otg_mid_END (23)
#define SOC_PCTRL_PERI_CTRL6_sc_perf_stat_mid_START (24)
#define SOC_PCTRL_PERI_CTRL6_sc_perf_stat_mid_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int modem_arqos : 5;
        unsigned int modem_awqos : 5;
        unsigned int reserved_0 : 6;
        unsigned int reserved_1 : 4;
        unsigned int a53_mid : 6;
        unsigned int a57_mid : 6;
    } reg;
} SOC_PCTRL_PERI_CTRL7_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL7_modem_arqos_START (0)
#define SOC_PCTRL_PERI_CTRL7_modem_arqos_END (4)
#define SOC_PCTRL_PERI_CTRL7_modem_awqos_START (5)
#define SOC_PCTRL_PERI_CTRL7_modem_awqos_END (9)
#define SOC_PCTRL_PERI_CTRL7_a53_mid_START (20)
#define SOC_PCTRL_PERI_CTRL7_a53_mid_END (25)
#define SOC_PCTRL_PERI_CTRL7_a57_mid_START (26)
#define SOC_PCTRL_PERI_CTRL7_a57_mid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d0_arnasid : 5;
        unsigned int g3d0_awnasid : 5;
        unsigned int g3d1_arnasid : 5;
        unsigned int g3d1_awnasid : 5;
        unsigned int g3d_arqos : 4;
        unsigned int reserved_0 : 1;
        unsigned int g3d_awqos : 4;
        unsigned int reserved_1 : 3;
    } reg;
} SOC_PCTRL_PERI_CTRL8_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL8_g3d0_arnasid_START (0)
#define SOC_PCTRL_PERI_CTRL8_g3d0_arnasid_END (4)
#define SOC_PCTRL_PERI_CTRL8_g3d0_awnasid_START (5)
#define SOC_PCTRL_PERI_CTRL8_g3d0_awnasid_END (9)
#define SOC_PCTRL_PERI_CTRL8_g3d1_arnasid_START (10)
#define SOC_PCTRL_PERI_CTRL8_g3d1_arnasid_END (14)
#define SOC_PCTRL_PERI_CTRL8_g3d1_awnasid_START (15)
#define SOC_PCTRL_PERI_CTRL8_g3d1_awnasid_END (19)
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
        unsigned int asp_mid : 6;
        unsigned int pcie_mid : 6;
        unsigned int a7_toviviobus_mid : 6;
        unsigned int reserved_0 : 6;
        unsigned int reserved_1 : 2;
        unsigned int mdc_mid : 2;
        unsigned int dss0_mid : 2;
        unsigned int dss1_mid : 2;
    } reg;
} SOC_PCTRL_PERI_CTRL9_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL9_asp_mid_START (0)
#define SOC_PCTRL_PERI_CTRL9_asp_mid_END (5)
#define SOC_PCTRL_PERI_CTRL9_pcie_mid_START (6)
#define SOC_PCTRL_PERI_CTRL9_pcie_mid_END (11)
#define SOC_PCTRL_PERI_CTRL9_a7_toviviobus_mid_START (12)
#define SOC_PCTRL_PERI_CTRL9_a7_toviviobus_mid_END (17)
#define SOC_PCTRL_PERI_CTRL9_mdc_mid_START (26)
#define SOC_PCTRL_PERI_CTRL9_mdc_mid_END (27)
#define SOC_PCTRL_PERI_CTRL9_dss0_mid_START (28)
#define SOC_PCTRL_PERI_CTRL9_dss0_mid_END (29)
#define SOC_PCTRL_PERI_CTRL9_dss1_mid_START (30)
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
        unsigned int reserved : 3;
        unsigned int ipf_mid : 6;
        unsigned int cssys_mid : 6;
        unsigned int iomcu_dma_mid : 6;
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
#define SOC_PCTRL_PERI_CTRL10_ipf_mid_START (14)
#define SOC_PCTRL_PERI_CTRL10_ipf_mid_END (19)
#define SOC_PCTRL_PERI_CTRL10_cssys_mid_START (20)
#define SOC_PCTRL_PERI_CTRL10_cssys_mid_END (25)
#define SOC_PCTRL_PERI_CTRL10_iomcu_dma_mid_START (26)
#define SOC_PCTRL_PERI_CTRL10_iomcu_dma_mid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mcpu_boot_addr : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_PCTRL_PERI_CTRL11_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL11_mcpu_boot_addr_START (0)
#define SOC_PCTRL_PERI_CTRL11_mcpu_boot_addr_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d0_mid0 : 6;
        unsigned int g3d0_mid1 : 6;
        unsigned int g3d1_mid0 : 6;
        unsigned int g3d1_mid1 : 6;
        unsigned int reserved_0: 6;
        unsigned int reserved_1: 2;
    } reg;
} SOC_PCTRL_PERI_CTRL34_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL34_g3d0_mid0_START (0)
#define SOC_PCTRL_PERI_CTRL34_g3d0_mid0_END (5)
#define SOC_PCTRL_PERI_CTRL34_g3d0_mid1_START (6)
#define SOC_PCTRL_PERI_CTRL34_g3d0_mid1_END (11)
#define SOC_PCTRL_PERI_CTRL34_g3d1_mid0_START (12)
#define SOC_PCTRL_PERI_CTRL34_g3d1_mid0_END (17)
#define SOC_PCTRL_PERI_CTRL34_g3d1_mid1_START (18)
#define SOC_PCTRL_PERI_CTRL34_g3d1_mid1_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 6;
        unsigned int hisee_mid : 6;
        unsigned int djtag_mid : 6;
        unsigned int dmac_mid : 6;
        unsigned int dpc_mid : 6;
        unsigned int reserved_1: 2;
    } reg;
} SOC_PCTRL_PERI_CTRL35_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL35_hisee_mid_START (6)
#define SOC_PCTRL_PERI_CTRL35_hisee_mid_END (11)
#define SOC_PCTRL_PERI_CTRL35_djtag_mid_START (12)
#define SOC_PCTRL_PERI_CTRL35_djtag_mid_END (17)
#define SOC_PCTRL_PERI_CTRL35_dmac_mid_START (18)
#define SOC_PCTRL_PERI_CTRL35_dmac_mid_END (23)
#define SOC_PCTRL_PERI_CTRL35_dpc_mid_START (24)
#define SOC_PCTRL_PERI_CTRL35_dpc_mid_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int venc_mid : 6;
        unsigned int vdec_mid : 6;
        unsigned int g3d1_crnasid : 5;
        unsigned int g3d0_crnasid : 5;
        unsigned int reserved : 10;
    } reg;
} SOC_PCTRL_PERI_CTRL38_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL38_venc_mid_START (0)
#define SOC_PCTRL_PERI_CTRL38_venc_mid_END (5)
#define SOC_PCTRL_PERI_CTRL38_vdec_mid_START (6)
#define SOC_PCTRL_PERI_CTRL38_vdec_mid_END (11)
#define SOC_PCTRL_PERI_CTRL38_g3d1_crnasid_START (12)
#define SOC_PCTRL_PERI_CTRL38_g3d1_crnasid_END (16)
#define SOC_PCTRL_PERI_CTRL38_g3d0_crnasid_START (17)
#define SOC_PCTRL_PERI_CTRL38_g3d0_crnasid_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pimon_mid : 6;
        unsigned int reserved_0 : 6;
        unsigned int ics_mid : 6;
        unsigned int isp_cpu_mid : 6;
        unsigned int reserved_1 : 6;
        unsigned int reserved_2 : 2;
    } reg;
} SOC_PCTRL_PERI_CTRL39_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL39_pimon_mid_START (0)
#define SOC_PCTRL_PERI_CTRL39_pimon_mid_END (5)
#define SOC_PCTRL_PERI_CTRL39_ics_mid_START (12)
#define SOC_PCTRL_PERI_CTRL39_ics_mid_END (17)
#define SOC_PCTRL_PERI_CTRL39_isp_cpu_mid_START (18)
#define SOC_PCTRL_PERI_CTRL39_isp_cpu_mid_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL40_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL41_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL42_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL43_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int djtag_addr_msb : 7;
        unsigned int reserved : 25;
    } reg;
} SOC_PCTRL_PERI_CTRL44_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL44_djtag_addr_msb_START (0)
#define SOC_PCTRL_PERI_CTRL44_djtag_addr_msb_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL45_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL46_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL47_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL48_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL49_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int modem_fama_icfg_addr_src0 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PCTRL_PERI_CTRL50_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL50_modem_fama_icfg_addr_src0_START (0)
#define SOC_PCTRL_PERI_CTRL50_modem_fama_icfg_addr_src0_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int modem_fama_icfg_addr_len0 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PCTRL_PERI_CTRL51_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL51_modem_fama_icfg_addr_len0_START (0)
#define SOC_PCTRL_PERI_CTRL51_modem_fama_icfg_addr_len0_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int modem_fama_icfg_addr_dst0 : 20;
        unsigned int modem_fama_addr_msb0 : 7;
        unsigned int reserved : 5;
    } reg;
} SOC_PCTRL_PERI_CTRL52_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL52_modem_fama_icfg_addr_dst0_START (0)
#define SOC_PCTRL_PERI_CTRL52_modem_fama_icfg_addr_dst0_END (19)
#define SOC_PCTRL_PERI_CTRL52_modem_fama_addr_msb0_START (20)
#define SOC_PCTRL_PERI_CTRL52_modem_fama_addr_msb0_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int modem_fama_icfg_addr_src1 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PCTRL_PERI_CTRL53_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL53_modem_fama_icfg_addr_src1_START (0)
#define SOC_PCTRL_PERI_CTRL53_modem_fama_icfg_addr_src1_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int modem_fama_icfg_addr_len1 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PCTRL_PERI_CTRL54_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL54_modem_fama_icfg_addr_len1_START (0)
#define SOC_PCTRL_PERI_CTRL54_modem_fama_icfg_addr_len1_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int modem_fama_icfg_addr_dst1 : 20;
        unsigned int modem_fama_addr_msb1 : 7;
        unsigned int reserved : 5;
    } reg;
} SOC_PCTRL_PERI_CTRL55_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL55_modem_fama_icfg_addr_dst1_START (0)
#define SOC_PCTRL_PERI_CTRL55_modem_fama_icfg_addr_dst1_END (19)
#define SOC_PCTRL_PERI_CTRL55_modem_fama_addr_msb1_START (20)
#define SOC_PCTRL_PERI_CTRL55_modem_fama_addr_msb1_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int modem_fama_icfg_addr_src2 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PCTRL_PERI_CTRL56_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL56_modem_fama_icfg_addr_src2_START (0)
#define SOC_PCTRL_PERI_CTRL56_modem_fama_icfg_addr_src2_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int modem_fama_icfg_addr_len2 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PCTRL_PERI_CTRL57_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL57_modem_fama_icfg_addr_len2_START (0)
#define SOC_PCTRL_PERI_CTRL57_modem_fama_icfg_addr_len2_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int modem_fama_icfg_addr_dst2 : 20;
        unsigned int modem_fama_addr_msb2 : 7;
        unsigned int reserved : 5;
    } reg;
} SOC_PCTRL_PERI_CTRL58_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL58_modem_fama_icfg_addr_dst2_START (0)
#define SOC_PCTRL_PERI_CTRL58_modem_fama_icfg_addr_dst2_END (19)
#define SOC_PCTRL_PERI_CTRL58_modem_fama_addr_msb2_START (20)
#define SOC_PCTRL_PERI_CTRL58_modem_fama_addr_msb2_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int modem_fama_icfg_addr_src3 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PCTRL_PERI_CTRL59_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL59_modem_fama_icfg_addr_src3_START (0)
#define SOC_PCTRL_PERI_CTRL59_modem_fama_icfg_addr_src3_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int modem_fama_icfg_addr_len3 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PCTRL_PERI_CTRL60_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL60_modem_fama_icfg_addr_len3_START (0)
#define SOC_PCTRL_PERI_CTRL60_modem_fama_icfg_addr_len3_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int modem_fama_icfg_addr_dst3 : 20;
        unsigned int modem_fama_addr_msb3 : 7;
        unsigned int reserved : 5;
    } reg;
} SOC_PCTRL_PERI_CTRL61_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL61_modem_fama_icfg_addr_dst3_START (0)
#define SOC_PCTRL_PERI_CTRL61_modem_fama_icfg_addr_dst3_END (19)
#define SOC_PCTRL_PERI_CTRL61_modem_fama_addr_msb3_START (20)
#define SOC_PCTRL_PERI_CTRL61_modem_fama_addr_msb3_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cipher_id_flt_en : 1;
        unsigned int cipher_fama_addr_id_remap : 7;
        unsigned int reserved_0 : 8;
        unsigned int reserved_1 : 7;
        unsigned int harq_id_flt_en : 1;
        unsigned int modem_fama_addr_default : 7;
        unsigned int reserved_2 : 1;
    } reg;
} SOC_PCTRL_PERI_CTRL62_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL62_cipher_id_flt_en_START (0)
#define SOC_PCTRL_PERI_CTRL62_cipher_id_flt_en_END (0)
#define SOC_PCTRL_PERI_CTRL62_cipher_fama_addr_id_remap_START (1)
#define SOC_PCTRL_PERI_CTRL62_cipher_fama_addr_id_remap_END (7)
#define SOC_PCTRL_PERI_CTRL62_harq_id_flt_en_START (23)
#define SOC_PCTRL_PERI_CTRL62_harq_id_flt_en_END (23)
#define SOC_PCTRL_PERI_CTRL62_modem_fama_addr_default_START (24)
#define SOC_PCTRL_PERI_CTRL62_modem_fama_addr_default_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL63_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pm_en_0 : 1;
        unsigned int pm_en_1 : 1;
        unsigned int pm_en_2 : 1;
        unsigned int pm_en_3 : 1;
        unsigned int pm_en_4 : 1;
        unsigned int pm_en_5 : 1;
        unsigned int pm_en_6 : 1;
        unsigned int pm_en_7 : 1;
        unsigned int pm_en_8 : 1;
        unsigned int reserved : 23;
    } reg;
} SOC_PCTRL_PERI_CTRL64_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL64_pm_en_0_START (0)
#define SOC_PCTRL_PERI_CTRL64_pm_en_0_END (0)
#define SOC_PCTRL_PERI_CTRL64_pm_en_1_START (1)
#define SOC_PCTRL_PERI_CTRL64_pm_en_1_END (1)
#define SOC_PCTRL_PERI_CTRL64_pm_en_2_START (2)
#define SOC_PCTRL_PERI_CTRL64_pm_en_2_END (2)
#define SOC_PCTRL_PERI_CTRL64_pm_en_3_START (3)
#define SOC_PCTRL_PERI_CTRL64_pm_en_3_END (3)
#define SOC_PCTRL_PERI_CTRL64_pm_en_4_START (4)
#define SOC_PCTRL_PERI_CTRL64_pm_en_4_END (4)
#define SOC_PCTRL_PERI_CTRL64_pm_en_5_START (5)
#define SOC_PCTRL_PERI_CTRL64_pm_en_5_END (5)
#define SOC_PCTRL_PERI_CTRL64_pm_en_6_START (6)
#define SOC_PCTRL_PERI_CTRL64_pm_en_6_END (6)
#define SOC_PCTRL_PERI_CTRL64_pm_en_7_START (7)
#define SOC_PCTRL_PERI_CTRL64_pm_en_7_END (7)
#define SOC_PCTRL_PERI_CTRL64_pm_en_8_START (8)
#define SOC_PCTRL_PERI_CTRL64_pm_en_8_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pm_sys_cfg_l : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL65_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL65_pm_sys_cfg_l_START (0)
#define SOC_PCTRL_PERI_CTRL65_pm_sys_cfg_l_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pm_sys_cfg_h : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL66_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL66_pm_sys_cfg_h_START (0)
#define SOC_PCTRL_PERI_CTRL66_pm_sys_cfg_h_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_addr_harqh_s : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PCTRL_PERI_CTRL75_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL75_sc_addr_harqh_s_START (0)
#define SOC_PCTRL_PERI_CTRL75_sc_addr_harqh_s_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_addr_harqh_e : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PCTRL_PERI_CTRL76_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL76_sc_addr_harqh_e_START (0)
#define SOC_PCTRL_PERI_CTRL76_sc_addr_harqh_e_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_addr_harqh_dst : 27;
        unsigned int reserved : 5;
    } reg;
} SOC_PCTRL_PERI_CTRL77_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL77_sc_addr_harqh_dst_START (0)
#define SOC_PCTRL_PERI_CTRL77_sc_addr_harqh_dst_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_addr_harqh_default : 7;
        unsigned int reserved : 25;
    } reg;
} SOC_PCTRL_PERI_CTRL78_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL78_sc_addr_harqh_default_START (0)
#define SOC_PCTRL_PERI_CTRL78_sc_addr_harqh_default_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_addr_harql_s : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PCTRL_PERI_CTRL79_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL79_sc_addr_harql_s_START (0)
#define SOC_PCTRL_PERI_CTRL79_sc_addr_harql_s_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_addr_harql_e : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_PCTRL_PERI_CTRL80_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL80_sc_addr_harql_e_START (0)
#define SOC_PCTRL_PERI_CTRL80_sc_addr_harql_e_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_addr_harql_dst : 27;
        unsigned int reserved : 5;
    } reg;
} SOC_PCTRL_PERI_CTRL81_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL81_sc_addr_harql_dst_START (0)
#define SOC_PCTRL_PERI_CTRL81_sc_addr_harql_dst_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sc_addr_harql_default : 7;
        unsigned int reserved : 25;
    } reg;
} SOC_PCTRL_PERI_CTRL82_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL82_sc_addr_harql_default_START (0)
#define SOC_PCTRL_PERI_CTRL82_sc_addr_harql_default_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int otp_trim_port_msb : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL83_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL83_otp_trim_port_msb_START (0)
#define SOC_PCTRL_PERI_CTRL83_otp_trim_port_msb_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int otp_trim_port_lsb : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL84_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL84_otp_trim_port_lsb_START (0)
#define SOC_PCTRL_PERI_CTRL84_otp_trim_port_lsb_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cc_path_sel : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_PCTRL_PERI_CTRL85_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL85_cc_path_sel_START (0)
#define SOC_PCTRL_PERI_CTRL85_cc_path_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cc_secfg : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL86_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL86_cc_secfg_START (0)
#define SOC_PCTRL_PERI_CTRL86_cc_secfg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cc_trng_data : 32;
    } reg;
} SOC_PCTRL_PERI_CTRL87_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL87_cc_trng_data_START (0)
#define SOC_PCTRL_PERI_CTRL87_cc_trng_data_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3d_nigen : 1;
        unsigned int g3d_bdgen : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PCTRL_PERI_CTRL94_UNION;
#endif
#define SOC_PCTRL_PERI_CTRL94_g3d_nigen_START (0)
#define SOC_PCTRL_PERI_CTRL94_g3d_nigen_END (0)
#define SOC_PCTRL_PERI_CTRL94_g3d_bdgen_START (1)
#define SOC_PCTRL_PERI_CTRL94_g3d_bdgen_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pm_data_rpt_0 : 32;
    } reg;
} SOC_PCTRL_PERI_STAT20_UNION;
#endif
#define SOC_PCTRL_PERI_STAT20_pm_data_rpt_0_START (0)
#define SOC_PCTRL_PERI_STAT20_pm_data_rpt_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pm_data_rpt_1 : 32;
    } reg;
} SOC_PCTRL_PERI_STAT21_UNION;
#endif
#define SOC_PCTRL_PERI_STAT21_pm_data_rpt_1_START (0)
#define SOC_PCTRL_PERI_STAT21_pm_data_rpt_1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pm_data_rpt_2 : 32;
    } reg;
} SOC_PCTRL_PERI_STAT22_UNION;
#endif
#define SOC_PCTRL_PERI_STAT22_pm_data_rpt_2_START (0)
#define SOC_PCTRL_PERI_STAT22_pm_data_rpt_2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pm_data_rpt_3 : 32;
    } reg;
} SOC_PCTRL_PERI_STAT23_UNION;
#endif
#define SOC_PCTRL_PERI_STAT23_pm_data_rpt_3_START (0)
#define SOC_PCTRL_PERI_STAT23_pm_data_rpt_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pm_data_rpt_4 : 32;
    } reg;
} SOC_PCTRL_PERI_STAT24_UNION;
#endif
#define SOC_PCTRL_PERI_STAT24_pm_data_rpt_4_START (0)
#define SOC_PCTRL_PERI_STAT24_pm_data_rpt_4_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pm_data_rpt_5 : 32;
    } reg;
} SOC_PCTRL_PERI_STAT25_UNION;
#endif
#define SOC_PCTRL_PERI_STAT25_pm_data_rpt_5_START (0)
#define SOC_PCTRL_PERI_STAT25_pm_data_rpt_5_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pm_data_rpt_6 : 32;
    } reg;
} SOC_PCTRL_PERI_STAT26_UNION;
#endif
#define SOC_PCTRL_PERI_STAT26_pm_data_rpt_6_START (0)
#define SOC_PCTRL_PERI_STAT26_pm_data_rpt_6_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pm_data_rpt_7 : 32;
    } reg;
} SOC_PCTRL_PERI_STAT27_UNION;
#endif
#define SOC_PCTRL_PERI_STAT27_pm_data_rpt_7_START (0)
#define SOC_PCTRL_PERI_STAT27_pm_data_rpt_7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pm_data_rpt_8 : 32;
    } reg;
} SOC_PCTRL_PERI_STAT28_UNION;
#endif
#define SOC_PCTRL_PERI_STAT28_pm_data_rpt_8_START (0)
#define SOC_PCTRL_PERI_STAT28_pm_data_rpt_8_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ps_avdd18_ready : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PCTRL_PERI_STAT65_UNION;
#endif
#define SOC_PCTRL_PERI_STAT65_ps_avdd18_ready_START (0)
#define SOC_PCTRL_PERI_STAT65_ps_avdd18_ready_END (0)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
