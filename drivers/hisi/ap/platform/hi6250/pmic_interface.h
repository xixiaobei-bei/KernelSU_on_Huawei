#ifndef __PMIC_INTERFACE_H__
#define __PMIC_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define PMIC_VERSION0_ADDR(base) ((base) + (0x000))
#define PMIC_VERSION1_ADDR(base) ((base) + (0x001))
#define PMIC_VERSION2_ADDR(base) ((base) + (0x002))
#define PMIC_VERSION3_ADDR(base) ((base) + (0x003))
#define PMIC_VERSION4_ADDR(base) ((base) + (0x004))
#define PMIC_VERSION5_ADDR(base) ((base) + (0x005))
#define PMIC_STATUS0_ADDR(base) ((base) + (0x006))
#define PMIC_STATUS1_ADDR(base) ((base) + (0x007))
#define PMIC_NP_D2A_RES0_ADDR(base) ((base) + (0x00A))
#define PMIC_NP_D2A_RES1_ADDR(base) ((base) + (0x00B))
#define PMIC_D2A_RES0_ADDR(base) ((base) + (0x00C))
#define PMIC_D2A_RES1_ADDR(base) ((base) + (0x00D))
#define PMIC_A2D_RES0_ADDR(base) ((base) + (0x00E))
#define PMIC_A2D_RES1_ADDR(base) ((base) + (0x00F))
#define PMIC_BUCK0_ONOFF_ECO_ADDR(base) ((base) + (0x010))
#define PMIC_BUCK1_ONOFF_ECO_ADDR(base) ((base) + (0x011))
#define PMIC_BUCK2_ONOFF_ECO_ADDR(base) ((base) + (0x012))
#define PMIC_BUCK3_ONOFF_ECO_ADDR(base) ((base) + (0x013))
#define PMIC_LDO0_1_ONOFF_ADDR(base) ((base) + (0x014))
#define PMIC_LDO0_2_ONOFF_ECO_ADDR(base) ((base) + (0x015))
#define PMIC_LDO1_ONOFF_ECO_ADDR(base) ((base) + (0x016))
#define PMIC_LDO2_ONOFF_ECO_ADDR(base) ((base) + (0x017))
#define PMIC_LDO3_ONOFF_ECO_ADDR(base) ((base) + (0x018))
#define PMIC_LDO4_ONOFF_ECO_ADDR(base) ((base) + (0x019))
#define PMIC_LDO5_ONOFF_ECO_ADDR(base) ((base) + (0x01A))
#define PMIC_LDO7_ONOFF_ECO_ADDR(base) ((base) + (0x01B))
#define PMIC_LDO8_ONOFF_ECO_ADDR(base) ((base) + (0x01C))
#define PMIC_LDO9_ONOFF_ECO_ADDR(base) ((base) + (0x01D))
#define PMIC_LDO10_ONOFF_ECO_ADDR(base) ((base) + (0x01E))
#define PMIC_LDO11_ONOFF_ECO_ADDR(base) ((base) + (0x01F))
#define PMIC_LDO12_ONOFF_ECO_ADDR(base) ((base) + (0x020))
#define PMIC_LDO13_ONOFF_ECO_ADDR(base) ((base) + (0x021))
#define PMIC_LDO14_ONOFF_ECO_ADDR(base) ((base) + (0x022))
#define PMIC_LDO15_ONOFF_ECO_ADDR(base) ((base) + (0x023))
#define PMIC_LDO16_ONOFF_ECO_ADDR(base) ((base) + (0x024))
#define PMIC_LDO17_ONOFF_ECO_ADDR(base) ((base) + (0x025))
#define PMIC_LDO19_ONOFF_ECO_ADDR(base) ((base) + (0x026))
#define PMIC_LDO20_ONOFF_ECO_ADDR(base) ((base) + (0x027))
#define PMIC_LDO21_ONOFF_ECO_ADDR(base) ((base) + (0x028))
#define PMIC_LDO22_ONOFF_ECO_ADDR(base) ((base) + (0x029))
#define PMIC_LDO23_ONOFF_ECO_ADDR(base) ((base) + (0x02A))
#define PMIC_LDO24_ONOFF_ECO_ADDR(base) ((base) + (0x02B))
#define PMIC_LDO25_ONOFF_ECO_ADDR(base) ((base) + (0x02C))
#define PMIC_LDO26_ONOFF_ECO_ADDR(base) ((base) + (0x02D))
#define PMIC_LDO27_ONOFF_ECO_ADDR(base) ((base) + (0x02E))
#define PMIC_LDO28_ONOFF_ECO_ADDR(base) ((base) + (0x02F))
#define PMIC_LDO29_ONOFF_ECO_ADDR(base) ((base) + (0x030))
#define PMIC_LDO31_ONOFF_ECO_ADDR(base) ((base) + (0x031))
#define PMIC_LDO32_ONOFF_ECO_ADDR(base) ((base) + (0x032))
#define PMIC_LDO33_ONOFF_ECO_ADDR(base) ((base) + (0x033))
#define PMIC_LDO34_ONOFF_ECO_ADDR(base) ((base) + (0x034))
#define PMIC_BUCK0_VSET_ADDR(base) ((base) + (0x03A))
#define PMIC_BUCK1_VSET_ADDR(base) ((base) + (0x03B))
#define PMIC_BUCK2_VSET_ADDR(base) ((base) + (0x03C))
#define PMIC_BUCK3_VSET_ADDR(base) ((base) + (0x03D))
#define PMIC_LDO0_2_VSET_ADDR(base) ((base) + (0x03E))
#define PMIC_LDO1_VSET_ADDR(base) ((base) + (0x03F))
#define PMIC_LDO2_VSET_ADDR(base) ((base) + (0x040))
#define PMIC_LDO3_VSET_ADDR(base) ((base) + (0x041))
#define PMIC_LDO4_VSET_ADDR(base) ((base) + (0x042))
#define PMIC_LDO5_VSET_ADDR(base) ((base) + (0x043))
#define PMIC_LDO7_VSET_ADDR(base) ((base) + (0x044))
#define PMIC_LDO8_VSET_ADDR(base) ((base) + (0x045))
#define PMIC_LDO9_VSET_ADDR(base) ((base) + (0x046))
#define PMIC_LDO10_VSET_ADDR(base) ((base) + (0x047))
#define PMIC_LDO11_VSET_ADDR(base) ((base) + (0x048))
#define PMIC_LDO12_VSET_ADDR(base) ((base) + (0x049))
#define PMIC_LDO13_VSET_ADDR(base) ((base) + (0x04A))
#define PMIC_LDO14_VSET_ADDR(base) ((base) + (0x04B))
#define PMIC_LDO15_VSET_ADDR(base) ((base) + (0x04C))
#define PMIC_LDO16_VSET_ADDR(base) ((base) + (0x04D))
#define PMIC_LDO17_VSET_ADDR(base) ((base) + (0x04E))
#define PMIC_LDO19_VSET1_ADDR(base) ((base) + (0x04F))
#define PMIC_LDO20_VSET_ADDR(base) ((base) + (0x050))
#define PMIC_LDO21_VSET_ADDR(base) ((base) + (0x051))
#define PMIC_LDO22_VSET_ADDR(base) ((base) + (0x052))
#define PMIC_LDO23_VSET_ADDR(base) ((base) + (0x053))
#define PMIC_LDO24_VSET_ADDR(base) ((base) + (0x054))
#define PMIC_LDO25_VSET_ADDR(base) ((base) + (0x055))
#define PMIC_LDO26_VSET_ADDR(base) ((base) + (0x056))
#define PMIC_LDO27_VSET_ADDR(base) ((base) + (0x057))
#define PMIC_LDO28_VSET_ADDR(base) ((base) + (0x058))
#define PMIC_LDO29_VSET_ADDR(base) ((base) + (0x059))
#define PMIC_LDO31_VSET_ADDR(base) ((base) + (0x05A))
#define PMIC_LDO32_VSET_ADDR(base) ((base) + (0x05B))
#define PMIC_LDO33_VSET_ADDR(base) ((base) + (0x05C))
#define PMIC_LDO34_VSET_ADDR(base) ((base) + (0x05D))
#define PMIC_BUCK0_CTRL0_ADDR(base) ((base) + (0x064))
#define PMIC_BUCK0_CTRL1_ADDR(base) ((base) + (0x065))
#define PMIC_BUCK0_CTRL2_ADDR(base) ((base) + (0x066))
#define PMIC_BUCK0_CTRL3_ADDR(base) ((base) + (0x067))
#define PMIC_BUCK0_CTRL4_ADDR(base) ((base) + (0x068))
#define PMIC_BUCK0_CTRL5_ADDR(base) ((base) + (0x069))
#define PMIC_BUCK0_CTRL6_ADDR(base) ((base) + (0x06A))
#define PMIC_BUCK0_CTRL7_ADDR(base) ((base) + (0x06B))
#define PMIC_BUCK0_CTRL8_ADDR(base) ((base) + (0x06C))
#define PMIC_BUCK1_CTRL0_ADDR(base) ((base) + (0x06D))
#define PMIC_BUCK1_CTRL1_ADDR(base) ((base) + (0x06E))
#define PMIC_BUCK1_CTRL2_ADDR(base) ((base) + (0x06F))
#define PMIC_BUCK1_CTRL3_ADDR(base) ((base) + (0x070))
#define PMIC_BUCK1_CTRL4_ADDR(base) ((base) + (0x071))
#define PMIC_BUCK1_CTRL5_ADDR(base) ((base) + (0x072))
#define PMIC_BUCK1_CTRL6_ADDR(base) ((base) + (0x073))
#define PMIC_BUCK1_CTRL7_ADDR(base) ((base) + (0x074))
#define PMIC_BUCK1_CTRL8_ADDR(base) ((base) + (0x075))
#define PMIC_BUCK2_CTRL0_ADDR(base) ((base) + (0x076))
#define PMIC_BUCK2_CTRL1_ADDR(base) ((base) + (0x077))
#define PMIC_BUCK2_CTRL2_ADDR(base) ((base) + (0x078))
#define PMIC_BUCK2_CTRL3_ADDR(base) ((base) + (0x079))
#define PMIC_BUCK2_CTRL4_ADDR(base) ((base) + (0x07A))
#define PMIC_BUCK2_CTRL5_ADDR(base) ((base) + (0x07B))
#define PMIC_BUCK2_CTRL6_ADDR(base) ((base) + (0x07C))
#define PMIC_BUCK2_CTRL7_ADDR(base) ((base) + (0x07D))
#define PMIC_BUCK2_CTRL8_ADDR(base) ((base) + (0x07E))
#define PMIC_BUCK3_CTRL0_ADDR(base) ((base) + (0x07F))
#define PMIC_BUCK3_CTRL1_ADDR(base) ((base) + (0x080))
#define PMIC_BUCK3_CTRL2_ADDR(base) ((base) + (0x081))
#define PMIC_BUCK3_CTRL3_ADDR(base) ((base) + (0x082))
#define PMIC_BUCK3_CTRL4_ADDR(base) ((base) + (0x083))
#define PMIC_BUCK3_CTRL5_ADDR(base) ((base) + (0x084))
#define PMIC_BUCK3_CTRL6_ADDR(base) ((base) + (0x085))
#define PMIC_BUCK3_CTRL7_ADDR(base) ((base) + (0x086))
#define PMIC_BUCK3_CTRL8_ADDR(base) ((base) + (0x087))
#define PMIC_BUCK_RESERVE0_ADDR(base) ((base) + (0x088))
#define PMIC_BUCK_RESERVE1_ADDR(base) ((base) + (0x089))
#define PMIC_LDO_1_CTRL_ADDR(base) ((base) + (0x08A))
#define PMIC_LDO1_CTRL_0_ADDR(base) ((base) + (0x08B))
#define PMIC_LDO1_CTRL_1_ADDR(base) ((base) + (0x08C))
#define PMIC_LDO2_3_CTRL_ADDR(base) ((base) + (0x08D))
#define PMIC_LDO3_CTRL_ADDR(base) ((base) + (0x08E))
#define PMIC_LDO4_5_CTRL_ADDR(base) ((base) + (0x08F))
#define PMIC_LDO7_8_CTRL_ADDR(base) ((base) + (0x090))
#define PMIC_LDO9_10_CTRL_ADDR(base) ((base) + (0x091))
#define PMIC_LD11_12_CTRL_ADDR(base) ((base) + (0x092))
#define PMIC_LDO13_14_CTRL_ADDR(base) ((base) + (0x093))
#define PMIC_LDO15_16_CTRL_ADDR(base) ((base) + (0x094))
#define PMIC_LDO17_19_CTRL_ADDR(base) ((base) + (0x095))
#define PMIC_LDO20_21_CTRL_ADDR(base) ((base) + (0x096))
#define PMIC_LDO22_CTRL0_ADDR(base) ((base) + (0x097))
#define PMIC_LDO22_CTRL1_ADDR(base) ((base) + (0x098))
#define PMIC_LDO22_CTRL2_ADDR(base) ((base) + (0x099))
#define PMIC_LDO23_CTRL_ADDR(base) ((base) + (0x09A))
#define PMIC_LDO24_25_CTRL_ADDR(base) ((base) + (0x09B))
#define PMIC_LDO26_CTRL_ADDR(base) ((base) + (0x09C))
#define PMIC_LDO27_CTRL_ADDR(base) ((base) + (0x09D))
#define PMIC_LDO28_29_CTRL_ADDR(base) ((base) + (0x09E))
#define PMIC_LDO31_32_CTRL_ADDR(base) ((base) + (0x09F))
#define PMIC_LDO33_LDO34_CTRL_ADDR(base) ((base) + (0x0A0))
#define PMIC_LDO_RESERVE0_ADDR(base) ((base) + (0x0A1))
#define PMIC_LDO_RESERVE1_ADDR(base) ((base) + (0x0A2))
#define PMIC_BUCK0_3_OCP_CTRL_ADDR(base) ((base) + (0x100))
#define PMIC_LDO0_3_OCP_CTRL_ADDR(base) ((base) + (0x101))
#define PMIC_LDO4_8_OCP_CTRL_ADDR(base) ((base) + (0x102))
#define PMIC_LDO9_12_OCP_CTRL_ADDR(base) ((base) + (0x103))
#define PMIC_LDO13_16_OCP_CTRL_ADDR(base) ((base) + (0x104))
#define PMIC_LDO17_21_OCP_CTRL_ADDR(base) ((base) + (0x105))
#define PMIC_LDO22_25_OCP_CTRL_ADDR(base) ((base) + (0x106))
#define PMIC_LDO26_29_OCP_CTRL_ADDR(base) ((base) + (0x107))
#define PMIC_LDO31_34_OCP_CTRL_ADDR(base) ((base) + (0x108))
#define PMIC_LDO_BUFF_CLASS_SCP_CTRL_ADDR(base) ((base) + (0x109))
#define PMIC_BUCK0_3_SCP_CTRL_ADDR(base) ((base) + (0x10A))
#define PMIC_LDO_PMUA_ECO_ADDR(base) ((base) + (0x10D))
#define PMIC_LDO_PMUA_VSET_ADDR(base) ((base) + (0x10E))
#define PMIC_CLK_ABB_EN_ADDR(base) ((base) + (0x10F))
#define PMIC_CLK_WIFI_EN_ADDR(base) ((base) + (0x110))
#define PMIC_CLK_RF0_EN_ADDR(base) ((base) + (0x111))
#define PMIC_CLK_RF1_EN_ADDR(base) ((base) + (0x112))
#define PMIC_CLK_SYS_EN_ADDR(base) ((base) + (0x113))
#define PMIC_CLK_CODEC_EN_ADDR(base) ((base) + (0x114))
#define PMIC_CLK_ABB_CTRL_ADDR(base) ((base) + (0x115))
#define PMIC_CLK_WIFI_CTRL_ADDR(base) ((base) + (0x116))
#define PMIC_CLK_RF0_CTRL_ADDR(base) ((base) + (0x117))
#define PMIC_CLK_RF1_CTRL_ADDR(base) ((base) + (0x118))
#define PMIC_CLK_SYS_CTRL_ADDR(base) ((base) + (0x119))
#define PMIC_CLK_CODEC_CTRL_ADDR(base) ((base) + (0x11A))
#define PMIC_CLK_TOP_CTRL0_ADDR(base) ((base) + (0x11B))
#define PMIC_CLK_TOP_CTRL1_ADDR(base) ((base) + (0x11C))
#define PMIC_CLK_TOP_CTRL2_ADDR(base) ((base) + (0x11D))
#define PMIC_CLK_TOP_CTRL3_ADDR(base) ((base) + (0x11E))
#define PMIC_CLK_256K_CTRL0_ADDR(base) ((base) + (0x11F))
#define PMIC_CLK_256K_CTRL1_ADDR(base) ((base) + (0x120))
#define PMIC_OSC32K_ONOFF_CTRL_ADDR(base) ((base) + (0x121))
#define PMIC_HARDWIRE_CTRL0_ADDR(base) ((base) + (0x125))
#define PMIC_HARDWIRE_CTRL1_ADDR(base) ((base) + (0x126))
#define PMIC_PERI_CTRL0_ADDR(base) ((base) + (0x127))
#define PMIC_PERI_CTRL1_ADDR(base) ((base) + (0x128))
#define PMIC_PERI_CTRL2_ADDR(base) ((base) + (0x129))
#define PMIC_PERI_CTRL3_ADDR(base) ((base) + (0x12A))
#define PMIC_PERI_VSET_CTRL_ADDR(base) ((base) + (0x12B))
#define PMIC_PERI_TIME__CTRL_ADDR(base) ((base) + (0x12C))
#define PMIC_COUL_ECO_MASK_ADDR(base) ((base) + (0x12D))
#define PMIC_IRQ_MASK_0_ADDR(base) ((base) + (0x130))
#define PMIC_IRQ_MASK_1_ADDR(base) ((base) + (0x131))
#define PMIC_IRQ_MASK_2_ADDR(base) ((base) + (0x132))
#define PMIC_IRQ_MASK_3_ADDR(base) ((base) + (0x133))
#define PMIC_IRQ_MASK_4_ADDR(base) ((base) + (0x134))
#define PMIC_IRQ_MASK_5_ADDR(base) ((base) + (0x135))
#define PMIC_IRQ_MASK_6_ADDR(base) ((base) + (0x136))
#define PMIC_IRQ_MASK_7_ADDR(base) ((base) + (0x137))
#define PMIC_BACKUP_CHG_ADDR(base) ((base) + (0x13A))
#define PMIC_CLASSD_CTRL0_ADDR(base) ((base) + (0x13B))
#define PMIC_CLASSD_CTRL1_ADDR(base) ((base) + (0x13C))
#define PMIC_CLASSD_CTRL2_ADDR(base) ((base) + (0x13D))
#define PMIC_CLASSD_CTRL3_ADDR(base) ((base) + (0x13E))
#define PMIC_TH_CTRL_ADDR(base) ((base) + (0x13F))
#define PMIC_BG_TEST_ADDR(base) ((base) + (0x140))
#define PMIC_DR_EN_MODE_345_ADDR(base) ((base) + (0x141))
#define PMIC_DR_EN_MODE_12_ADDR(base) ((base) + (0x142))
#define PMIC_FLASH_PERIOD_DR12_ADDR(base) ((base) + (0x143))
#define PMIC_FLASH_ON_DR12_ADDR(base) ((base) + (0x144))
#define PMIC_FLASH_PERIOD_DR345_ADDR(base) ((base) + (0x145))
#define PMIC_FLASH_ON_DR345_ADDR(base) ((base) + (0x146))
#define PMIC_DR_MODE_SEL_ADDR(base) ((base) + (0x147))
#define PMIC_DR_BRE_CTRL_ADDR(base) ((base) + (0x148))
#define PMIC_DR12_TIM_CONF0_ADDR(base) ((base) + (0x149))
#define PMIC_DR12_TIM_CONF1_ADDR(base) ((base) + (0x14A))
#define PMIC_DR1_ISET_ADDR(base) ((base) + (0x14B))
#define PMIC_DR2_ISET_ADDR(base) ((base) + (0x14C))
#define PMIC_DR_LED_CTRL_ADDR(base) ((base) + (0x14D))
#define PMIC_DR_OUT_CTRL_ADDR(base) ((base) + (0x14E))
#define PMIC_DR3_ISET_ADDR(base) ((base) + (0x14F))
#define PMIC_DR3_START_DEL_ADDR(base) ((base) + (0x150))
#define PMIC_DR4_ISET_ADDR(base) ((base) + (0x151))
#define PMIC_DR4_START_DEL_ADDR(base) ((base) + (0x152))
#define PMIC_DR5_ISET_ADDR(base) ((base) + (0x153))
#define PMIC_DR5_START_DEL_ADDR(base) ((base) + (0x154))
#define PMIC_DR334_TIM_CONF0_ADDR(base) ((base) + (0x155))
#define PMIC_DR345_TIM_CONF1_ADDR(base) ((base) + (0x156))
#define PMIC_VSYS_LOW_SET_ADDR(base) ((base) + (0x157))
#define PMIC_NFC_ON_CTRL_ADDR(base) ((base) + (0x158))
#define PMIC_SYS_CTRL_RESERVE_ADDR(base) ((base) + (0x159))
#define PMIC_HRESET_PWRDOWN_CTRL_ADDR(base) ((base) + (0x15A))
#define PMIC_OCP_DEB_CTRL0_ADDR(base) ((base) + (0x15B))
#define PMIC_OCP_DEB_CTRL1_ADDR(base) ((base) + (0x15C))
#define PMIC_OCP_SCP_ONOFF_ADDR(base) ((base) + (0x15D))
#define PMIC_PWROFF_DEB_CTRL_ADDR(base) ((base) + (0x15E))
#define PMIC_SYS_CTRL0_ADDR(base) ((base) + (0x15F))
#define PMIC_SMPL_CTRL_ADDR(base) ((base) + (0x160))
#define PMIC_SYS_CTRL1_ADDR(base) ((base) + (0x161))
#define PMIC_SIM_CTRL_ADDR(base) ((base) + (0x162))
#define PMIC_SIM_DEB_CTRL_ADDR(base) ((base) + (0x163))
#define PMIC_AUX0_IBIAS_CFG_ADDR(base) ((base) + (0x164))
#define PMIC_AUX1_IBIAS_CFG_ADDR(base) ((base) + (0x165))
#define PMIC_OTP0_0_ADDR(base) ((base) + (0x16A))
#define PMIC_OTP0_1_ADDR(base) ((base) + (0x16B))
#define PMIC_OTP0_2_ADDR(base) ((base) + (0x16C))
#define PMIC_OTP0_3_ADDR(base) ((base) + (0x16D))
#define PMIC_OTP0_CTRL_0_ADDR(base) ((base) + (0x16E))
#define PMIC_OTP0_CTRL_1_ADDR(base) ((base) + (0x16F))
#define PMIC_OTP0_WDATA_ADDR(base) ((base) + (0x170))
#define PMIC_OTP0_0_W_ADDR(base) ((base) + (0x171))
#define PMIC_OTP0_1_W_ADDR(base) ((base) + (0x172))
#define PMIC_OTP0_2_W_ADDR(base) ((base) + (0x173))
#define PMIC_OTP0_3_W_ADDR(base) ((base) + (0x174))
#define PMIC_OTP1_0_ADDR(base) ((base) + (0x175))
#define PMIC_OTP1_1_ADDR(base) ((base) + (0x176))
#define PMIC_OTP1_2_ADDR(base) ((base) + (0x177))
#define PMIC_OTP1_3_ADDR(base) ((base) + (0x178))
#define PMIC_OTP1_CTRL_0_ADDR(base) ((base) + (0x179))
#define PMIC_OTP1_CTRL_1_ADDR(base) ((base) + (0x17A))
#define PMIC_OTP1_WDATA_ADDR(base) ((base) + (0x17B))
#define PMIC_OTP1_0_W_ADDR(base) ((base) + (0x17C))
#define PMIC_OTP1_1_W_ADDR(base) ((base) + (0x17D))
#define PMIC_OTP1_2_W_ADDR(base) ((base) + (0x17E))
#define PMIC_OTP1_3_W_ADDR(base) ((base) + (0x17F))
#define PMIC_OTP2_0_ADDR(base) ((base) + (0x180))
#define PMIC_OTP2_1_ADDR(base) ((base) + (0x181))
#define PMIC_OTP2_2_ADDR(base) ((base) + (0x182))
#define PMIC_OTP2_3_ADDR(base) ((base) + (0x183))
#define PMIC_OTP2_CTRL_0_ADDR(base) ((base) + (0x184))
#define PMIC_OTP2_CTRL_1_ADDR(base) ((base) + (0x185))
#define PMIC_OTP2_WDATA_ADDR(base) ((base) + (0x186))
#define PMIC_OTP2_0_W_ADDR(base) ((base) + (0x187))
#define PMIC_OTP2_1_W_ADDR(base) ((base) + (0x188))
#define PMIC_OTP2_2_W_ADDR(base) ((base) + (0x189))
#define PMIC_OTP2_3_W_ADDR(base) ((base) + (0x18A))
#define PMIC_HRST_REG0_ADDR(base) ((base) + (0x18B))
#define PMIC_HRST_REG1_ADDR(base) ((base) + (0x18C))
#define PMIC_HRST_REG2_ADDR(base) ((base) + (0x18D))
#define PMIC_HRST_REG3_ADDR(base) ((base) + (0x18E))
#define PMIC_HRST_REG4_ADDR(base) ((base) + (0x18F))
#define PMIC_HRST_REG5_ADDR(base) ((base) + (0x190))
#define PMIC_HRST_REG6_ADDR(base) ((base) + (0x191))
#define PMIC_HRST_REG7_ADDR(base) ((base) + (0x192))
#define PMIC_HRST_REG8_ADDR(base) ((base) + (0x193))
#define PMIC_HRST_REG9_ADDR(base) ((base) + (0x194))
#define PMIC_HRST_REG10_ADDR(base) ((base) + (0x195))
#define PMIC_HRST_REG11_ADDR(base) ((base) + (0x196))
#define PMIC_HRST_REG12_ADDR(base) ((base) + (0x197))
#define PMIC_HRST_REG13_ADDR(base) ((base) + (0x198))
#define PMIC_HRST_REG14_ADDR(base) ((base) + (0x199))
#define PMIC_DEBUG_CTRL_ADDR(base) ((base) + (0x19A))
#define PMIC_SYS_DEBUG0_ADDR(base) ((base) + (0x19B))
#define PMIC_SYS_DEBUG1_ADDR(base) ((base) + (0x19C))
#define PMIC_DEBUG_LOCK_ADDR(base) ((base) + (0x19D))
#define PMIC_DAC0_DIN_MSB_ADDR(base) ((base) + (0x19F))
#define PMIC_DAC0_DIN_LSB_ADDR(base) ((base) + (0x1A0))
#define PMIC_DAC1_DIN_MSB_ADDR(base) ((base) + (0x1A1))
#define PMIC_DAC1_DIN_LSB_ADDR(base) ((base) + (0x1A2))
#define PMIC_IRQ0_ADDR(base) ((base) + (0x1B0))
#define PMIC_IRQ1_ADDR(base) ((base) + (0x1B1))
#define PMIC_OCP_IRQ0_ADDR(base) ((base) + (0x1B2))
#define PMIC_OCP_IRQ1_ADDR(base) ((base) + (0x1B3))
#define PMIC_OCP_IRQ2_ADDR(base) ((base) + (0x1B4))
#define PMIC_OCP_IRQ3_ADDR(base) ((base) + (0x1B5))
#define PMIC_OCP_IRQ4_ADDR(base) ((base) + (0x1B6))
#define PMIC_OCP_IRQ5_ADDR(base) ((base) + (0x1B7))
#define PMIC_NP_OCP0_ADDR(base) ((base) + (0x1B8))
#define PMIC_NP_OCP1_ADDR(base) ((base) + (0x1B9))
#define PMIC_NP_OCP2_ADDR(base) ((base) + (0x1BA))
#define PMIC_NP_OCP3_ADDR(base) ((base) + (0x1BB))
#define PMIC_NP_OCP4_ADDR(base) ((base) + (0x1BC))
#define PMIC_NP_SCP_ADDR(base) ((base) + (0x1BD))
#define PMIC_NP_RECORD0_ADDR(base) ((base) + (0x1BE))
#define PMIC_NP_RECORD1_ADDR(base) ((base) + (0x1BF))
#define PMIC_NP_RECORD2_ADDR(base) ((base) + (0x1C0))
#define PMIC_NP_RECORD3_ADDR(base) ((base) + (0x1C1))
#define PMIC_RTCDR0_ADDR(base) ((base) + (0x1D0))
#define PMIC_RTCDR1_ADDR(base) ((base) + (0x1D1))
#define PMIC_RTCDR2_ADDR(base) ((base) + (0x1D2))
#define PMIC_RTCDR3_ADDR(base) ((base) + (0x1D3))
#define PMIC_RTCMR0_ADDR(base) ((base) + (0x1D4))
#define PMIC_RTCMR1_ADDR(base) ((base) + (0x1D5))
#define PMIC_RTCMR2_ADDR(base) ((base) + (0x1D6))
#define PMIC_RTCMR3_ADDR(base) ((base) + (0x1D7))
#define PMIC_RTCLR0_ADDR(base) ((base) + (0x1D8))
#define PMIC_RTCLR1_ADDR(base) ((base) + (0x1D9))
#define PMIC_RTCLR2_ADDR(base) ((base) + (0x1DA))
#define PMIC_RTCLR3_ADDR(base) ((base) + (0x1DB))
#define PMIC_RTCCTRL_ADDR(base) ((base) + (0x1DC))
#define PMIC_XO_THRESOLD0_ADDR(base) ((base) + (0x1DD))
#define PMIC_XO_THRESOLD1_ADDR(base) ((base) + (0x1DE))
#define PMIC_CRC_VAULE0_ADDR(base) ((base) + (0x1DF))
#define PMIC_CRC_VAULE1_ADDR(base) ((base) + (0x1E0))
#define PMIC_CRC_VAULE2_ADDR(base) ((base) + (0x1E1))
#define PMIC_RTC_PWRUP_TIMER0_ADDR(base) ((base) + (0x1E2))
#define PMIC_RTC_PWRUP_TIMER1_ADDR(base) ((base) + (0x1E3))
#define PMIC_RTC_PWRUP_TIMER2_ADDR(base) ((base) + (0x1E4))
#define PMIC_RTC_PWRUP_TIMER3_ADDR(base) ((base) + (0x1E5))
#define PMIC_RTC_PWRDOWN_TIMER0_ADDR(base) ((base) + (0x1E6))
#define PMIC_RTC_PWRDOWN_TIMER1_ADDR(base) ((base) + (0x1E7))
#define PMIC_RTC_PWRDOWN_TIMER2_ADDR(base) ((base) + (0x1E8))
#define PMIC_RTC_PWRDOWN_TIMER3_ADDR(base) ((base) + (0x1E9))
#define PMIC_COUL_IRQ_ADDR(base) ((base) + (0x1F0))
#define PMIC_COUL_IRQ_MASK_ADDR(base) ((base) + (0x1F1))
#define PMIC_CURRENT_0_ADDR(base) ((base) + (0x1F2))
#define PMIC_CURRENT_1_ADDR(base) ((base) + (0x1F3))
#define PMIC_V_OUT_0_ADDR(base) ((base) + (0x1F4))
#define PMIC_V_OUT_1_ADDR(base) ((base) + (0x1F5))
#define PMIC_CLJ_CTRL_REG_ADDR(base) ((base) + (0x1F6))
#define PMIC_ECO_REFALSH_TIME_ADDR(base) ((base) + (0x1F7))
#define PMIC_CL_OUT0_ADDR(base) ((base) + (0x1F8))
#define PMIC_CL_OUT1_ADDR(base) ((base) + (0x1F9))
#define PMIC_CL_OUT2_ADDR(base) ((base) + (0x1FA))
#define PMIC_CL_OUT3_ADDR(base) ((base) + (0x1FB))
#define PMIC_CL_IN0_ADDR(base) ((base) + (0x1FC))
#define PMIC_CL_IN1_ADDR(base) ((base) + (0x1FD))
#define PMIC_CL_IN2_ADDR(base) ((base) + (0x1FE))
#define PMIC_CL_IN3_ADDR(base) ((base) + (0x1FF))
#define PMIC_CHG_TIMER0_ADDR(base) ((base) + (0x200))
#define PMIC_CHG_TIMER1_ADDR(base) ((base) + (0x201))
#define PMIC_CHG_TIMER2_ADDR(base) ((base) + (0x202))
#define PMIC_CHG_TIMER3_ADDR(base) ((base) + (0x203))
#define PMIC_LOAD_TIMER0_ADDR(base) ((base) + (0x204))
#define PMIC_LOAD_TIMER1_ADDR(base) ((base) + (0x205))
#define PMIC_LOAD_TIMER2_ADDR(base) ((base) + (0x206))
#define PMIC_LOAD_TIMER3_ADDR(base) ((base) + (0x207))
#define PMIC_CL_INT0_ADDR(base) ((base) + (0x208))
#define PMIC_CL_INT1_ADDR(base) ((base) + (0x209))
#define PMIC_CL_INT2_ADDR(base) ((base) + (0x20A))
#define PMIC_CL_INT3_ADDR(base) ((base) + (0x20B))
#define PMIC_V_INT0_ADDR(base) ((base) + (0x20C))
#define PMIC_V_INT1_ADDR(base) ((base) + (0x20D))
#define PMIC_OFFSET_CURRENT0_ADDR(base) ((base) + (0x20E))
#define PMIC_OFFSET_CURRENT1_ADDR(base) ((base) + (0x20F))
#define PMIC_OFFSET_VOLTAGE0_ADDR(base) ((base) + (0x210))
#define PMIC_OFFSET_VOLTAGE1_ADDR(base) ((base) + (0x211))
#define PMIC_OCV_VOLTAGE0_ADDR(base) ((base) + (0x212))
#define PMIC_OCV_VOLTAGE1_ADDR(base) ((base) + (0x213))
#define PMIC_OCV_CURRENT0_ADDR(base) ((base) + (0x214))
#define PMIC_OCV_CURRENT1_ADDR(base) ((base) + (0x215))
#define PMIC_ECO_OUT_CLIN_0_ADDR(base) ((base) + (0x216))
#define PMIC_ECO_OUT_CLIN_1_ADDR(base) ((base) + (0x217))
#define PMIC_ECO_OUT_CLIN_2_ADDR(base) ((base) + (0x218))
#define PMIC_ECO_OUT_CLIN_3_ADDR(base) ((base) + (0x219))
#define PMIC_ECO_OUT_CLOUT_0_ADDR(base) ((base) + (0x21A))
#define PMIC_ECO_OUT_CLOUT_1_ADDR(base) ((base) + (0x21B))
#define PMIC_ECO_OUT_CLOUT_2_ADDR(base) ((base) + (0x21C))
#define PMIC_ECO_OUT_CLOUT_3_ADDR(base) ((base) + (0x21D))
#define PMIC_V_OUT0_PRE0_ADDR(base) ((base) + (0x21E))
#define PMIC_V_OUT1_PRE0_ADDR(base) ((base) + (0x21F))
#define PMIC_V_OUT0_PRE1_ADDR(base) ((base) + (0x220))
#define PMIC_V_OUT1_PRE1_ADDR(base) ((base) + (0x221))
#define PMIC_V_OUT0_PRE2_ADDR(base) ((base) + (0x222))
#define PMIC_V_OUT1_PRE2_ADDR(base) ((base) + (0x223))
#define PMIC_V_OUT0_PRE3_ADDR(base) ((base) + (0x224))
#define PMIC_V_OUT1_PRE3_ADDR(base) ((base) + (0x225))
#define PMIC_V_OUT0_PRE4_ADDR(base) ((base) + (0x226))
#define PMIC_V_OUT1_PRE4_ADDR(base) ((base) + (0x227))
#define PMIC_V_OUT0_PRE5_ADDR(base) ((base) + (0x228))
#define PMIC_V_OUT1_PRE5_ADDR(base) ((base) + (0x229))
#define PMIC_V_OUT0_PRE6_ADDR(base) ((base) + (0x22A))
#define PMIC_V_OUT1_PRE6_ADDR(base) ((base) + (0x22B))
#define PMIC_V_OUT0_PRE7_ADDR(base) ((base) + (0x22C))
#define PMIC_V_OUT1_PRE7_ADDR(base) ((base) + (0x22D))
#define PMIC_V_OUT0_PRE8_ADDR(base) ((base) + (0x22E))
#define PMIC_V_OUT1_PRE8_ADDR(base) ((base) + (0x22F))
#define PMIC_V_OUT0_PRE9_ADDR(base) ((base) + (0x230))
#define PMIC_V_OUT1_PRE9_ADDR(base) ((base) + (0x231))
#define PMIC_CURRENT0_PRE0_ADDR(base) ((base) + (0x232))
#define PMIC_CURRENT1_PRE0_ADDR(base) ((base) + (0x233))
#define PMIC_CURRENT0_PRE1_ADDR(base) ((base) + (0x234))
#define PMIC_CURRENT1_PRE1_ADDR(base) ((base) + (0x235))
#define PMIC_CURRENT0_PRE2_ADDR(base) ((base) + (0x236))
#define PMIC_CURRENT1_PRE2_ADDR(base) ((base) + (0x237))
#define PMIC_CURRENT0_PRE3_ADDR(base) ((base) + (0x238))
#define PMIC_CURRENT1_PRE3_ADDR(base) ((base) + (0x239))
#define PMIC_CURRENT0_PRE4_ADDR(base) ((base) + (0x23A))
#define PMIC_CURRENT1_PRE4_ADDR(base) ((base) + (0x23B))
#define PMIC_CURRENT0_PRE5_ADDR(base) ((base) + (0x23C))
#define PMIC_CURRENT1_PRE5_ADDR(base) ((base) + (0x23D))
#define PMIC_CURRENT0_PRE6_ADDR(base) ((base) + (0x23E))
#define PMIC_CURRENT1_PRE6_ADDR(base) ((base) + (0x23F))
#define PMIC_CURRENT0_PRE7_ADDR(base) ((base) + (0x240))
#define PMIC_CURRENT1_PRE7_ADDR(base) ((base) + (0x241))
#define PMIC_CURRENT0_PRE8_ADDR(base) ((base) + (0x242))
#define PMIC_CURRENT1_PRE8_ADDR(base) ((base) + (0x243))
#define PMIC_CURRENT0_PRE9_ADDR(base) ((base) + (0x244))
#define PMIC_CURRENT1_PRE9_ADDR(base) ((base) + (0x245))
#define PMIC_OFFSET_CURRENT_MOD_0_ADDR(base) ((base) + (0x246))
#define PMIC_OFFSET_CURRENT_MOD_1_ADDR(base) ((base) + (0x247))
#define PMIC_OFFSET_VOLTAGE_MOD_0_ADDR(base) ((base) + (0x248))
#define PMIC_OFFSET_VOLTAGE_MOD_1_ADDR(base) ((base) + (0x249))
#define PMIC_COUL_RESERVE0_ADDR(base) ((base) + (0x24A))
#define PMIC_CLJ_RESERVED0_ADDR(base) ((base) + (0x24B))
#define PMIC_CLJ_RESERVED1_ADDR(base) ((base) + (0x24C))
#define PMIC_CLJ_RESERVED2_ADDR(base) ((base) + (0x24D))
#define PMIC_PMU_SOFT_RST_ADDR(base) ((base) + (0x24E))
#define PMIC_CLJ_DEBUG_ADDR(base) ((base) + (0x24F))
#define PMIC_CLJ_DEBUG_2_ADDR(base) ((base) + (0x250))
#define PMIC_STATE_TEST_ADDR(base) ((base) + (0x251))
#define PMIC_CLJ_CTRL_REGS2_ADDR(base) ((base) + (0x252))
#define PMIC_DEBUG_WRITE_PRO_ADDR(base) ((base) + (0x253))
#define PMIC_CLK_EN_CFG_ADDR(base) ((base) + (0x260))
#define PMIC_CLK_EDGE_CFG_ADDR(base) ((base) + (0x261))
#define PMIC_LOOPBACK_CFG_ADDR(base) ((base) + (0x262))
#define PMIC_ANA_IRQ_SIG_STAT_ADDR(base) ((base) + (0x263))
#define PMIC_ANA_IRQM_REG0_ADDR(base) ((base) + (0x264))
#define PMIC_ANA_IRQ_REG0_ADDR(base) ((base) + (0x265))
#define PMIC_DEB_CNT_HS_DET_CFG_ADDR(base) ((base) + (0x266))
#define PMIC_DEB_CNT_HS_MIC_CFG_ADDR(base) ((base) + (0x267))
#define PMIC_CODEC_DIG_RES02_ADDR(base) ((base) + (0x268))
#define PMIC_CODEC_DIG_RES03_ADDR(base) ((base) + (0x269))
#define PMIC_CODEC_ANA_RW01_ADDR(base) ((base) + (0x26A))
#define PMIC_CODEC_ANA_RW02_ADDR(base) ((base) + (0x26B))
#define PMIC_CODEC_ANA_RW03_ADDR(base) ((base) + (0x26C))
#define PMIC_CODEC_ANA_RW04_ADDR(base) ((base) + (0x26D))
#define PMIC_CODEC_ANA_RW05_ADDR(base) ((base) + (0x26E))
#define PMIC_CODEC_ANA_RW06_ADDR(base) ((base) + (0x26F))
#define PMIC_CODEC_ANA_RW07_ADDR(base) ((base) + (0x270))
#define PMIC_CODEC_ANA_RW08_ADDR(base) ((base) + (0x271))
#define PMIC_CODEC_ANA_RW09_ADDR(base) ((base) + (0x272))
#define PMIC_CODEC_ANA_RW10_ADDR(base) ((base) + (0x273))
#define PMIC_CODEC_ANA_RW11_ADDR(base) ((base) + (0x274))
#define PMIC_CODEC_ANA_RW12_ADDR(base) ((base) + (0x275))
#define PMIC_CODEC_ANA_RW13_ADDR(base) ((base) + (0x276))
#define PMIC_CODEC_ANA_RW14_ADDR(base) ((base) + (0x277))
#define PMIC_CODEC_ANA_RW15_ADDR(base) ((base) + (0x278))
#define PMIC_CODEC_ANA_RW16_ADDR(base) ((base) + (0x279))
#define PMIC_CODEC_ANA_RW17_ADDR(base) ((base) + (0x27A))
#define PMIC_CODEC_ANA_RW18_ADDR(base) ((base) + (0x27B))
#define PMIC_CODEC_ANA_RW19_ADDR(base) ((base) + (0x27C))
#define PMIC_CODEC_ANA_RW20_ADDR(base) ((base) + (0x27D))
#define PMIC_CODEC_ANA_RW21_ADDR(base) ((base) + (0x27E))
#define PMIC_CODEC_ANA_RW22_ADDR(base) ((base) + (0x27F))
#define PMIC_CODEC_ANA_RW23_ADDR(base) ((base) + (0x280))
#define PMIC_CODEC_ANA_RW24_ADDR(base) ((base) + (0x281))
#define PMIC_CODEC_ANA_RW25_ADDR(base) ((base) + (0x282))
#define PMIC_CODEC_ANA_RW26_ADDR(base) ((base) + (0x283))
#define PMIC_CODEC_ANA_RW27_ADDR(base) ((base) + (0x284))
#define PMIC_CODEC_ANA_RW28_ADDR(base) ((base) + (0x285))
#define PMIC_CODEC_ANA_RW29_ADDR(base) ((base) + (0x286))
#define PMIC_CODEC_ANA_RW30_ADDR(base) ((base) + (0x287))
#define PMIC_CODEC_ANA_RW31_ADDR(base) ((base) + (0x288))
#define PMIC_CODEC_ANA_RW32_ADDR(base) ((base) + (0x289))
#define PMIC_CODEC_ANA_RW33_ADDR(base) ((base) + (0x28A))
#define PMIC_CODEC_ANA_RW34_ADDR(base) ((base) + (0x28B))
#define PMIC_CODEC_ANA_RW35_ADDR(base) ((base) + (0x28C))
#define PMIC_CODEC_ANA_RW36_ADDR(base) ((base) + (0x28D))
#define PMIC_CODEC_ANA_RW37_ADDR(base) ((base) + (0x28E))
#define PMIC_CODEC_ANA_RW38_ADDR(base) ((base) + (0x28F))
#define PMIC_CODEC_ANA_RW39_ADDR(base) ((base) + (0x290))
#define PMIC_CODEC_ANA_RW40_ADDR(base) ((base) + (0x291))
#define PMIC_CODEC_ANA_RO01_ADDR(base) ((base) + (0x292))
#define PMIC_CODEC_ANA_RO02_ADDR(base) ((base) + (0x293))
#define PMIC_CODEC_ANA_RW41_ADDR(base) ((base) + (0x294))
#define PMIC_CODEC_ANA_RW42_ADDR(base) ((base) + (0x295))
#define PMIC_CODEC_ANA_RW43_ADDR(base) ((base) + (0x296))
#define PMIC_CODEC_ANA_RW44_ADDR(base) ((base) + (0x297))
#define PMIC_CODEC_ANA_RW45_ADDR(base) ((base) + (0x298))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char project_num0 : 8;
    } reg;
} PMIC_VERSION0_UNION;
#endif
#define PMIC_VERSION0_project_num0_START (0)
#define PMIC_VERSION0_project_num0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char project_num1 : 8;
    } reg;
} PMIC_VERSION1_UNION;
#endif
#define PMIC_VERSION1_project_num1_START (0)
#define PMIC_VERSION1_project_num1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char project_num2 : 8;
    } reg;
} PMIC_VERSION2_UNION;
#endif
#define PMIC_VERSION2_project_num2_START (0)
#define PMIC_VERSION2_project_num2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char project_num3 : 8;
    } reg;
} PMIC_VERSION3_UNION;
#endif
#define PMIC_VERSION3_project_num3_START (0)
#define PMIC_VERSION3_project_num3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char version : 8;
    } reg;
} PMIC_VERSION4_UNION;
#endif
#define PMIC_VERSION4_version_START (0)
#define PMIC_VERSION4_version_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chip_id : 8;
    } reg;
} PMIC_VERSION5_UNION;
#endif
#define PMIC_VERSION5_chip_id_START (0)
#define PMIC_VERSION5_chip_id_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char st_vsys_ov_d200ur : 1;
        unsigned char st_vbus_det_insert_d20m : 1;
        unsigned char st_vsys_pwroff_abs : 1;
        unsigned char st_vsys_pwroff_deb_d80mr : 1;
        unsigned char st_vsys_pwron_d60ur : 1;
        unsigned char st_thsd_otmp140_d1mr : 1;
        unsigned char st_thsd_otmp125_d1mr : 1;
        unsigned char st_pwron_d20m : 1;
    } reg;
} PMIC_STATUS0_UNION;
#endif
#define PMIC_STATUS0_st_vsys_ov_d200ur_START (0)
#define PMIC_STATUS0_st_vsys_ov_d200ur_END (0)
#define PMIC_STATUS0_st_vbus_det_insert_d20m_START (1)
#define PMIC_STATUS0_st_vbus_det_insert_d20m_END (1)
#define PMIC_STATUS0_st_vsys_pwroff_abs_START (2)
#define PMIC_STATUS0_st_vsys_pwroff_abs_END (2)
#define PMIC_STATUS0_st_vsys_pwroff_deb_d80mr_START (3)
#define PMIC_STATUS0_st_vsys_pwroff_deb_d80mr_END (3)
#define PMIC_STATUS0_st_vsys_pwron_d60ur_START (4)
#define PMIC_STATUS0_st_vsys_pwron_d60ur_END (4)
#define PMIC_STATUS0_st_thsd_otmp140_d1mr_START (5)
#define PMIC_STATUS0_st_thsd_otmp140_d1mr_END (5)
#define PMIC_STATUS0_st_thsd_otmp125_d1mr_START (6)
#define PMIC_STATUS0_st_thsd_otmp125_d1mr_END (6)
#define PMIC_STATUS0_st_pwron_d20m_START (7)
#define PMIC_STATUS0_st_pwron_d20m_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char st_dcxo_clk_sel : 1;
        unsigned char st_tcxo_clk_sel : 1;
        unsigned char st_xo_32k_sel : 1;
        unsigned char st_xo_19m2_sel : 1;
        unsigned char st_buck1_vol_sel : 1;
        unsigned char st_sim0_hpd_d540u : 1;
        unsigned char st_sim1_hpd_d540u : 1;
        unsigned char st_avdd_osc_vld_d20nf : 1;
    } reg;
} PMIC_STATUS1_UNION;
#endif
#define PMIC_STATUS1_st_dcxo_clk_sel_START (0)
#define PMIC_STATUS1_st_dcxo_clk_sel_END (0)
#define PMIC_STATUS1_st_tcxo_clk_sel_START (1)
#define PMIC_STATUS1_st_tcxo_clk_sel_END (1)
#define PMIC_STATUS1_st_xo_32k_sel_START (2)
#define PMIC_STATUS1_st_xo_32k_sel_END (2)
#define PMIC_STATUS1_st_xo_19m2_sel_START (3)
#define PMIC_STATUS1_st_xo_19m2_sel_END (3)
#define PMIC_STATUS1_st_buck1_vol_sel_START (4)
#define PMIC_STATUS1_st_buck1_vol_sel_END (4)
#define PMIC_STATUS1_st_sim0_hpd_d540u_START (5)
#define PMIC_STATUS1_st_sim0_hpd_d540u_END (5)
#define PMIC_STATUS1_st_sim1_hpd_d540u_START (6)
#define PMIC_STATUS1_st_sim1_hpd_d540u_END (6)
#define PMIC_STATUS1_st_avdd_osc_vld_d20nf_START (7)
#define PMIC_STATUS1_st_avdd_osc_vld_d20nf_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_d2a_reserve0 : 8;
    } reg;
} PMIC_NP_D2A_RES0_UNION;
#endif
#define PMIC_NP_D2A_RES0_np_d2a_reserve0_START (0)
#define PMIC_NP_D2A_RES0_np_d2a_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_d2a_reserve1 : 8;
    } reg;
} PMIC_NP_D2A_RES1_UNION;
#endif
#define PMIC_NP_D2A_RES1_np_d2a_reserve1_START (0)
#define PMIC_NP_D2A_RES1_np_d2a_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char d2a_reserve0 : 8;
    } reg;
} PMIC_D2A_RES0_UNION;
#endif
#define PMIC_D2A_RES0_d2a_reserve0_START (0)
#define PMIC_D2A_RES0_d2a_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char d2a_reserve1 : 8;
    } reg;
} PMIC_D2A_RES1_UNION;
#endif
#define PMIC_D2A_RES1_d2a_reserve1_START (0)
#define PMIC_D2A_RES1_d2a_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char a2d_reserve0 : 8;
    } reg;
} PMIC_A2D_RES0_UNION;
#endif
#define PMIC_A2D_RES0_a2d_reserve0_START (0)
#define PMIC_A2D_RES0_a2d_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char a2d_reserve1 : 8;
    } reg;
} PMIC_A2D_RES1_UNION;
#endif
#define PMIC_A2D_RES1_a2d_reserve1_START (0)
#define PMIC_A2D_RES1_a2d_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_buck0_en : 1;
        unsigned char st_buck0_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_buck0_eco_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_BUCK0_ONOFF_ECO_UNION;
#endif
#define PMIC_BUCK0_ONOFF_ECO_reg_buck0_en_START (0)
#define PMIC_BUCK0_ONOFF_ECO_reg_buck0_en_END (0)
#define PMIC_BUCK0_ONOFF_ECO_st_buck0_en_START (1)
#define PMIC_BUCK0_ONOFF_ECO_st_buck0_en_END (1)
#define PMIC_BUCK0_ONOFF_ECO_reg_buck0_eco_en_START (4)
#define PMIC_BUCK0_ONOFF_ECO_reg_buck0_eco_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_buck1_en : 1;
        unsigned char st_buck1_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_buck1_eco_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_BUCK1_ONOFF_ECO_UNION;
#endif
#define PMIC_BUCK1_ONOFF_ECO_reg_buck1_en_START (0)
#define PMIC_BUCK1_ONOFF_ECO_reg_buck1_en_END (0)
#define PMIC_BUCK1_ONOFF_ECO_st_buck1_en_START (1)
#define PMIC_BUCK1_ONOFF_ECO_st_buck1_en_END (1)
#define PMIC_BUCK1_ONOFF_ECO_reg_buck1_eco_en_START (4)
#define PMIC_BUCK1_ONOFF_ECO_reg_buck1_eco_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_buck2_en : 1;
        unsigned char st_buck2_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_buck2_eco_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_BUCK2_ONOFF_ECO_UNION;
#endif
#define PMIC_BUCK2_ONOFF_ECO_reg_buck2_en_START (0)
#define PMIC_BUCK2_ONOFF_ECO_reg_buck2_en_END (0)
#define PMIC_BUCK2_ONOFF_ECO_st_buck2_en_START (1)
#define PMIC_BUCK2_ONOFF_ECO_st_buck2_en_END (1)
#define PMIC_BUCK2_ONOFF_ECO_reg_buck2_eco_en_START (4)
#define PMIC_BUCK2_ONOFF_ECO_reg_buck2_eco_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_buck3_en : 1;
        unsigned char st_buck3_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_buck3_eco_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_BUCK3_ONOFF_ECO_UNION;
#endif
#define PMIC_BUCK3_ONOFF_ECO_reg_buck3_en_START (0)
#define PMIC_BUCK3_ONOFF_ECO_reg_buck3_en_END (0)
#define PMIC_BUCK3_ONOFF_ECO_st_buck3_en_START (1)
#define PMIC_BUCK3_ONOFF_ECO_st_buck3_en_END (1)
#define PMIC_BUCK3_ONOFF_ECO_reg_buck3_eco_en_START (4)
#define PMIC_BUCK3_ONOFF_ECO_reg_buck3_eco_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo0_1_en : 1;
        unsigned char st_ldo0_1_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO0_1_ONOFF_UNION;
#endif
#define PMIC_LDO0_1_ONOFF_reg_ldo0_1_en_START (0)
#define PMIC_LDO0_1_ONOFF_reg_ldo0_1_en_END (0)
#define PMIC_LDO0_1_ONOFF_st_ldo0_1_en_START (1)
#define PMIC_LDO0_1_ONOFF_st_ldo0_1_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo0_2_en : 1;
        unsigned char st_ldo0_2_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo0_2_eco_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_LDO0_2_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO0_2_ONOFF_ECO_reg_ldo0_2_en_START (0)
#define PMIC_LDO0_2_ONOFF_ECO_reg_ldo0_2_en_END (0)
#define PMIC_LDO0_2_ONOFF_ECO_st_ldo0_2_en_START (1)
#define PMIC_LDO0_2_ONOFF_ECO_st_ldo0_2_en_END (1)
#define PMIC_LDO0_2_ONOFF_ECO_reg_ldo0_2_eco_en_START (4)
#define PMIC_LDO0_2_ONOFF_ECO_reg_ldo0_2_eco_en_END (4)
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
} PMIC_LDO1_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO1_ONOFF_ECO_reg_ldo1_en_START (0)
#define PMIC_LDO1_ONOFF_ECO_reg_ldo1_en_END (0)
#define PMIC_LDO1_ONOFF_ECO_st_ldo1_en_START (1)
#define PMIC_LDO1_ONOFF_ECO_st_ldo1_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo2_en : 1;
        unsigned char st_ldo2_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo2_eco_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_LDO2_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO2_ONOFF_ECO_reg_ldo2_en_START (0)
#define PMIC_LDO2_ONOFF_ECO_reg_ldo2_en_END (0)
#define PMIC_LDO2_ONOFF_ECO_st_ldo2_en_START (1)
#define PMIC_LDO2_ONOFF_ECO_st_ldo2_en_END (1)
#define PMIC_LDO2_ONOFF_ECO_reg_ldo2_eco_en_START (4)
#define PMIC_LDO2_ONOFF_ECO_reg_ldo2_eco_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo3_en : 1;
        unsigned char st_ldo3_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO3_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO3_ONOFF_ECO_reg_ldo3_en_START (0)
#define PMIC_LDO3_ONOFF_ECO_reg_ldo3_en_END (0)
#define PMIC_LDO3_ONOFF_ECO_st_ldo3_en_START (1)
#define PMIC_LDO3_ONOFF_ECO_st_ldo3_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo4_en : 1;
        unsigned char st_ldo4_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo4_eco_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_LDO4_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO4_ONOFF_ECO_reg_ldo4_en_START (0)
#define PMIC_LDO4_ONOFF_ECO_reg_ldo4_en_END (0)
#define PMIC_LDO4_ONOFF_ECO_st_ldo4_en_START (1)
#define PMIC_LDO4_ONOFF_ECO_st_ldo4_en_END (1)
#define PMIC_LDO4_ONOFF_ECO_reg_ldo4_eco_en_START (4)
#define PMIC_LDO4_ONOFF_ECO_reg_ldo4_eco_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo5_en : 1;
        unsigned char st_ldo5_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO5_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO5_ONOFF_ECO_reg_ldo5_en_START (0)
#define PMIC_LDO5_ONOFF_ECO_reg_ldo5_en_END (0)
#define PMIC_LDO5_ONOFF_ECO_st_ldo5_en_START (1)
#define PMIC_LDO5_ONOFF_ECO_st_ldo5_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo7_en : 1;
        unsigned char st_ldo7_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO7_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO7_ONOFF_ECO_reg_ldo7_en_START (0)
#define PMIC_LDO7_ONOFF_ECO_reg_ldo7_en_END (0)
#define PMIC_LDO7_ONOFF_ECO_st_ldo7_en_START (1)
#define PMIC_LDO7_ONOFF_ECO_st_ldo7_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo8_en : 1;
        unsigned char st_ldo8_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo8_eco_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_LDO8_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO8_ONOFF_ECO_reg_ldo8_en_START (0)
#define PMIC_LDO8_ONOFF_ECO_reg_ldo8_en_END (0)
#define PMIC_LDO8_ONOFF_ECO_st_ldo8_en_START (1)
#define PMIC_LDO8_ONOFF_ECO_st_ldo8_en_END (1)
#define PMIC_LDO8_ONOFF_ECO_reg_ldo8_eco_en_START (4)
#define PMIC_LDO8_ONOFF_ECO_reg_ldo8_eco_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo9_en : 1;
        unsigned char st_ldo9_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo9_eco_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_LDO9_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO9_ONOFF_ECO_reg_ldo9_en_START (0)
#define PMIC_LDO9_ONOFF_ECO_reg_ldo9_en_END (0)
#define PMIC_LDO9_ONOFF_ECO_st_ldo9_en_START (1)
#define PMIC_LDO9_ONOFF_ECO_st_ldo9_en_END (1)
#define PMIC_LDO9_ONOFF_ECO_reg_ldo9_eco_en_START (4)
#define PMIC_LDO9_ONOFF_ECO_reg_ldo9_eco_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo10_en : 1;
        unsigned char st_ldo10_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo10_eco_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_LDO10_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO10_ONOFF_ECO_reg_ldo10_en_START (0)
#define PMIC_LDO10_ONOFF_ECO_reg_ldo10_en_END (0)
#define PMIC_LDO10_ONOFF_ECO_st_ldo10_en_START (1)
#define PMIC_LDO10_ONOFF_ECO_st_ldo10_en_END (1)
#define PMIC_LDO10_ONOFF_ECO_reg_ldo10_eco_en_START (4)
#define PMIC_LDO10_ONOFF_ECO_reg_ldo10_eco_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo11_en : 1;
        unsigned char st_ldo11_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo11_eco_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_LDO11_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO11_ONOFF_ECO_reg_ldo11_en_START (0)
#define PMIC_LDO11_ONOFF_ECO_reg_ldo11_en_END (0)
#define PMIC_LDO11_ONOFF_ECO_st_ldo11_en_START (1)
#define PMIC_LDO11_ONOFF_ECO_st_ldo11_en_END (1)
#define PMIC_LDO11_ONOFF_ECO_reg_ldo11_eco_en_START (4)
#define PMIC_LDO11_ONOFF_ECO_reg_ldo11_eco_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo12_en : 1;
        unsigned char st_ldo12_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo12_eco_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_LDO12_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO12_ONOFF_ECO_reg_ldo12_en_START (0)
#define PMIC_LDO12_ONOFF_ECO_reg_ldo12_en_END (0)
#define PMIC_LDO12_ONOFF_ECO_st_ldo12_en_START (1)
#define PMIC_LDO12_ONOFF_ECO_st_ldo12_en_END (1)
#define PMIC_LDO12_ONOFF_ECO_reg_ldo12_eco_en_START (4)
#define PMIC_LDO12_ONOFF_ECO_reg_ldo12_eco_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo13_en : 1;
        unsigned char st_ldo13_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO13_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO13_ONOFF_ECO_reg_ldo13_en_START (0)
#define PMIC_LDO13_ONOFF_ECO_reg_ldo13_en_END (0)
#define PMIC_LDO13_ONOFF_ECO_st_ldo13_en_START (1)
#define PMIC_LDO13_ONOFF_ECO_st_ldo13_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo14_en : 1;
        unsigned char st_ldo14_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO14_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO14_ONOFF_ECO_reg_ldo14_en_START (0)
#define PMIC_LDO14_ONOFF_ECO_reg_ldo14_en_END (0)
#define PMIC_LDO14_ONOFF_ECO_st_ldo14_en_START (1)
#define PMIC_LDO14_ONOFF_ECO_st_ldo14_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo15_en : 1;
        unsigned char st_ldo15_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo15_eco_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_LDO15_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO15_ONOFF_ECO_reg_ldo15_en_START (0)
#define PMIC_LDO15_ONOFF_ECO_reg_ldo15_en_END (0)
#define PMIC_LDO15_ONOFF_ECO_st_ldo15_en_START (1)
#define PMIC_LDO15_ONOFF_ECO_st_ldo15_en_END (1)
#define PMIC_LDO15_ONOFF_ECO_reg_ldo15_eco_en_START (4)
#define PMIC_LDO15_ONOFF_ECO_reg_ldo15_eco_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo16_en : 1;
        unsigned char st_ldo16_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo16_eco_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_LDO16_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO16_ONOFF_ECO_reg_ldo16_en_START (0)
#define PMIC_LDO16_ONOFF_ECO_reg_ldo16_en_END (0)
#define PMIC_LDO16_ONOFF_ECO_st_ldo16_en_START (1)
#define PMIC_LDO16_ONOFF_ECO_st_ldo16_en_END (1)
#define PMIC_LDO16_ONOFF_ECO_reg_ldo16_eco_en_START (4)
#define PMIC_LDO16_ONOFF_ECO_reg_ldo16_eco_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo17_en : 1;
        unsigned char st_ldo17_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo17_eco_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_LDO17_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO17_ONOFF_ECO_reg_ldo17_en_START (0)
#define PMIC_LDO17_ONOFF_ECO_reg_ldo17_en_END (0)
#define PMIC_LDO17_ONOFF_ECO_st_ldo17_en_START (1)
#define PMIC_LDO17_ONOFF_ECO_st_ldo17_en_END (1)
#define PMIC_LDO17_ONOFF_ECO_reg_ldo17_eco_en_START (4)
#define PMIC_LDO17_ONOFF_ECO_reg_ldo17_eco_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo19_en : 1;
        unsigned char st_ldo19_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO19_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO19_ONOFF_ECO_reg_ldo19_en_START (0)
#define PMIC_LDO19_ONOFF_ECO_reg_ldo19_en_END (0)
#define PMIC_LDO19_ONOFF_ECO_st_ldo19_en_START (1)
#define PMIC_LDO19_ONOFF_ECO_st_ldo19_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo20_en : 1;
        unsigned char st_ldo20_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO20_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO20_ONOFF_ECO_reg_ldo20_en_START (0)
#define PMIC_LDO20_ONOFF_ECO_reg_ldo20_en_END (0)
#define PMIC_LDO20_ONOFF_ECO_st_ldo20_en_START (1)
#define PMIC_LDO20_ONOFF_ECO_st_ldo20_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo21_en : 1;
        unsigned char st_ldo21_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO21_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO21_ONOFF_ECO_reg_ldo21_en_START (0)
#define PMIC_LDO21_ONOFF_ECO_reg_ldo21_en_END (0)
#define PMIC_LDO21_ONOFF_ECO_st_ldo21_en_START (1)
#define PMIC_LDO21_ONOFF_ECO_st_ldo21_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo22_en : 1;
        unsigned char st_ldo22_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO22_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO22_ONOFF_ECO_reg_ldo22_en_START (0)
#define PMIC_LDO22_ONOFF_ECO_reg_ldo22_en_END (0)
#define PMIC_LDO22_ONOFF_ECO_st_ldo22_en_START (1)
#define PMIC_LDO22_ONOFF_ECO_st_ldo22_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo23_en : 1;
        unsigned char st_ldo23_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO23_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO23_ONOFF_ECO_reg_ldo23_en_START (0)
#define PMIC_LDO23_ONOFF_ECO_reg_ldo23_en_END (0)
#define PMIC_LDO23_ONOFF_ECO_st_ldo23_en_START (1)
#define PMIC_LDO23_ONOFF_ECO_st_ldo23_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo24_en : 1;
        unsigned char st_ldo24_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo24_eco_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_LDO24_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO24_ONOFF_ECO_reg_ldo24_en_START (0)
#define PMIC_LDO24_ONOFF_ECO_reg_ldo24_en_END (0)
#define PMIC_LDO24_ONOFF_ECO_st_ldo24_en_START (1)
#define PMIC_LDO24_ONOFF_ECO_st_ldo24_en_END (1)
#define PMIC_LDO24_ONOFF_ECO_reg_ldo24_eco_en_START (4)
#define PMIC_LDO24_ONOFF_ECO_reg_ldo24_eco_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo25_en : 1;
        unsigned char st_ldo25_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO25_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO25_ONOFF_ECO_reg_ldo25_en_START (0)
#define PMIC_LDO25_ONOFF_ECO_reg_ldo25_en_END (0)
#define PMIC_LDO25_ONOFF_ECO_st_ldo25_en_START (1)
#define PMIC_LDO25_ONOFF_ECO_st_ldo25_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo26_en : 1;
        unsigned char st_ldo26_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo26_eco_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_LDO26_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO26_ONOFF_ECO_reg_ldo26_en_START (0)
#define PMIC_LDO26_ONOFF_ECO_reg_ldo26_en_END (0)
#define PMIC_LDO26_ONOFF_ECO_st_ldo26_en_START (1)
#define PMIC_LDO26_ONOFF_ECO_st_ldo26_en_END (1)
#define PMIC_LDO26_ONOFF_ECO_reg_ldo26_eco_en_START (4)
#define PMIC_LDO26_ONOFF_ECO_reg_ldo26_eco_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo27_en : 1;
        unsigned char st_ldo27_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO27_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO27_ONOFF_ECO_reg_ldo27_en_START (0)
#define PMIC_LDO27_ONOFF_ECO_reg_ldo27_en_END (0)
#define PMIC_LDO27_ONOFF_ECO_st_ldo27_en_START (1)
#define PMIC_LDO27_ONOFF_ECO_st_ldo27_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo28_en : 1;
        unsigned char st_ldo28_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO28_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO28_ONOFF_ECO_reg_ldo28_en_START (0)
#define PMIC_LDO28_ONOFF_ECO_reg_ldo28_en_END (0)
#define PMIC_LDO28_ONOFF_ECO_st_ldo28_en_START (1)
#define PMIC_LDO28_ONOFF_ECO_st_ldo28_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo29_en : 1;
        unsigned char st_ldo29_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo29_eco_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_LDO29_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO29_ONOFF_ECO_reg_ldo29_en_START (0)
#define PMIC_LDO29_ONOFF_ECO_reg_ldo29_en_END (0)
#define PMIC_LDO29_ONOFF_ECO_st_ldo29_en_START (1)
#define PMIC_LDO29_ONOFF_ECO_st_ldo29_en_END (1)
#define PMIC_LDO29_ONOFF_ECO_reg_ldo29_eco_en_START (4)
#define PMIC_LDO29_ONOFF_ECO_reg_ldo29_eco_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo31_en : 1;
        unsigned char st_ldo31_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO31_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO31_ONOFF_ECO_reg_ldo31_en_START (0)
#define PMIC_LDO31_ONOFF_ECO_reg_ldo31_en_END (0)
#define PMIC_LDO31_ONOFF_ECO_st_ldo31_en_START (1)
#define PMIC_LDO31_ONOFF_ECO_st_ldo31_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo32_en : 1;
        unsigned char st_ldo32_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO32_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO32_ONOFF_ECO_reg_ldo32_en_START (0)
#define PMIC_LDO32_ONOFF_ECO_reg_ldo32_en_END (0)
#define PMIC_LDO32_ONOFF_ECO_st_ldo32_en_START (1)
#define PMIC_LDO32_ONOFF_ECO_st_ldo32_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo33_en : 1;
        unsigned char st_ldo33_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO33_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO33_ONOFF_ECO_reg_ldo33_en_START (0)
#define PMIC_LDO33_ONOFF_ECO_reg_ldo33_en_END (0)
#define PMIC_LDO33_ONOFF_ECO_st_ldo33_en_START (1)
#define PMIC_LDO33_ONOFF_ECO_st_ldo33_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo34_en : 1;
        unsigned char st_ldo34_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo34_eco_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_LDO34_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO34_ONOFF_ECO_reg_ldo34_en_START (0)
#define PMIC_LDO34_ONOFF_ECO_reg_ldo34_en_END (0)
#define PMIC_LDO34_ONOFF_ECO_st_ldo34_en_START (1)
#define PMIC_LDO34_ONOFF_ECO_st_ldo34_en_END (1)
#define PMIC_LDO34_ONOFF_ECO_reg_ldo34_eco_en_START (4)
#define PMIC_LDO34_ONOFF_ECO_reg_ldo34_eco_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_BUCK0_VSET_UNION;
#endif
#define PMIC_BUCK0_VSET_buck0_vset_START (0)
#define PMIC_BUCK0_VSET_buck0_vset_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_vset : 4;
        unsigned char buck1_vset_adj : 4;
    } reg;
} PMIC_BUCK1_VSET_UNION;
#endif
#define PMIC_BUCK1_VSET_buck1_vset_START (0)
#define PMIC_BUCK1_VSET_buck1_vset_END (3)
#define PMIC_BUCK1_VSET_buck1_vset_adj_START (4)
#define PMIC_BUCK1_VSET_buck1_vset_adj_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_vset : 4;
        unsigned char buck2_vset_adj : 4;
    } reg;
} PMIC_BUCK2_VSET_UNION;
#endif
#define PMIC_BUCK2_VSET_buck2_vset_START (0)
#define PMIC_BUCK2_VSET_buck2_vset_END (3)
#define PMIC_BUCK2_VSET_buck2_vset_adj_START (4)
#define PMIC_BUCK2_VSET_buck2_vset_adj_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_vset : 4;
        unsigned char buck3_vset_adj : 4;
    } reg;
} PMIC_BUCK3_VSET_UNION;
#endif
#define PMIC_BUCK3_VSET_buck3_vset_START (0)
#define PMIC_BUCK3_VSET_buck3_vset_END (3)
#define PMIC_BUCK3_VSET_buck3_vset_adj_START (4)
#define PMIC_BUCK3_VSET_buck3_vset_adj_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo0_2_vset : 3;
        unsigned char reserved_0 : 1;
        unsigned char ldo0_2_vset_adj : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_LDO0_2_VSET_UNION;
#endif
#define PMIC_LDO0_2_VSET_ldo0_2_vset_START (0)
#define PMIC_LDO0_2_VSET_ldo0_2_vset_END (2)
#define PMIC_LDO0_2_VSET_ldo0_2_vset_adj_START (4)
#define PMIC_LDO0_2_VSET_ldo0_2_vset_adj_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo1_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO1_VSET_UNION;
#endif
#define PMIC_LDO1_VSET_ldo1_vset_START (0)
#define PMIC_LDO1_VSET_ldo1_vset_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo2_vset : 3;
        unsigned char reserved_0 : 1;
        unsigned char ldo2_vset_adj : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_LDO2_VSET_UNION;
#endif
#define PMIC_LDO2_VSET_ldo2_vset_START (0)
#define PMIC_LDO2_VSET_ldo2_vset_END (2)
#define PMIC_LDO2_VSET_ldo2_vset_adj_START (4)
#define PMIC_LDO2_VSET_ldo2_vset_adj_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo3_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO3_VSET_UNION;
#endif
#define PMIC_LDO3_VSET_ldo3_vset_START (0)
#define PMIC_LDO3_VSET_ldo3_vset_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo4_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO4_VSET_UNION;
#endif
#define PMIC_LDO4_VSET_ldo4_vset_START (0)
#define PMIC_LDO4_VSET_ldo4_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo5_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO5_VSET_UNION;
#endif
#define PMIC_LDO5_VSET_ldo5_vset_START (0)
#define PMIC_LDO5_VSET_ldo5_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo7_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO7_VSET_UNION;
#endif
#define PMIC_LDO7_VSET_ldo7_vset_START (0)
#define PMIC_LDO7_VSET_ldo7_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo8_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO8_VSET_UNION;
#endif
#define PMIC_LDO8_VSET_ldo8_vset_START (0)
#define PMIC_LDO8_VSET_ldo8_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo9_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO9_VSET_UNION;
#endif
#define PMIC_LDO9_VSET_ldo9_vset_START (0)
#define PMIC_LDO9_VSET_ldo9_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo10_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO10_VSET_UNION;
#endif
#define PMIC_LDO10_VSET_ldo10_vset_START (0)
#define PMIC_LDO10_VSET_ldo10_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo11_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO11_VSET_UNION;
#endif
#define PMIC_LDO11_VSET_ldo11_vset_START (0)
#define PMIC_LDO11_VSET_ldo11_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo12_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO12_VSET_UNION;
#endif
#define PMIC_LDO12_VSET_ldo12_vset_START (0)
#define PMIC_LDO12_VSET_ldo12_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo13_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO13_VSET_UNION;
#endif
#define PMIC_LDO13_VSET_ldo13_vset_START (0)
#define PMIC_LDO13_VSET_ldo13_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo14_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO14_VSET_UNION;
#endif
#define PMIC_LDO14_VSET_ldo14_vset_START (0)
#define PMIC_LDO14_VSET_ldo14_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo15_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO15_VSET_UNION;
#endif
#define PMIC_LDO15_VSET_ldo15_vset_START (0)
#define PMIC_LDO15_VSET_ldo15_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo16_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO16_VSET_UNION;
#endif
#define PMIC_LDO16_VSET_ldo16_vset_START (0)
#define PMIC_LDO16_VSET_ldo16_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo17_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO17_VSET_UNION;
#endif
#define PMIC_LDO17_VSET_ldo17_vset_START (0)
#define PMIC_LDO17_VSET_ldo17_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo19_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO19_VSET1_UNION;
#endif
#define PMIC_LDO19_VSET1_ldo19_vset_START (0)
#define PMIC_LDO19_VSET1_ldo19_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo20_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO20_VSET_UNION;
#endif
#define PMIC_LDO20_VSET_ldo20_vset_START (0)
#define PMIC_LDO20_VSET_ldo20_vset_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo21_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO21_VSET_UNION;
#endif
#define PMIC_LDO21_VSET_ldo21_vset_START (0)
#define PMIC_LDO21_VSET_ldo21_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo22_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO22_VSET_UNION;
#endif
#define PMIC_LDO22_VSET_ldo22_vset_START (0)
#define PMIC_LDO22_VSET_ldo22_vset_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo23_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO23_VSET_UNION;
#endif
#define PMIC_LDO23_VSET_ldo23_vset_START (0)
#define PMIC_LDO23_VSET_ldo23_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo24_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO24_VSET_UNION;
#endif
#define PMIC_LDO24_VSET_ldo24_vset_START (0)
#define PMIC_LDO24_VSET_ldo24_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo25_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO25_VSET_UNION;
#endif
#define PMIC_LDO25_VSET_ldo25_vset_START (0)
#define PMIC_LDO25_VSET_ldo25_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo26_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO26_VSET_UNION;
#endif
#define PMIC_LDO26_VSET_ldo26_vset_START (0)
#define PMIC_LDO26_VSET_ldo26_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo27_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO27_VSET_UNION;
#endif
#define PMIC_LDO27_VSET_ldo27_vset_START (0)
#define PMIC_LDO27_VSET_ldo27_vset_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo28_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO28_VSET_UNION;
#endif
#define PMIC_LDO28_VSET_ldo28_vset_START (0)
#define PMIC_LDO28_VSET_ldo28_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo29_vset : 3;
        unsigned char reserved_0 : 1;
        unsigned char ldo29_vset_adj : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_LDO29_VSET_UNION;
#endif
#define PMIC_LDO29_VSET_ldo29_vset_START (0)
#define PMIC_LDO29_VSET_ldo29_vset_END (2)
#define PMIC_LDO29_VSET_ldo29_vset_adj_START (4)
#define PMIC_LDO29_VSET_ldo29_vset_adj_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo31_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO31_VSET_UNION;
#endif
#define PMIC_LDO31_VSET_ldo31_vset_START (0)
#define PMIC_LDO31_VSET_ldo31_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo32_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO32_VSET_UNION;
#endif
#define PMIC_LDO32_VSET_ldo32_vset_START (0)
#define PMIC_LDO32_VSET_ldo32_vset_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo33_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO33_VSET_UNION;
#endif
#define PMIC_LDO33_VSET_ldo33_vset_START (0)
#define PMIC_LDO33_VSET_ldo33_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo34_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO34_VSET_UNION;
#endif
#define PMIC_LDO34_VSET_ldo34_vset_START (0)
#define PMIC_LDO34_VSET_ldo34_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_dbias : 4;
        unsigned char buck0_adj_rlx : 4;
    } reg;
} PMIC_BUCK0_CTRL0_UNION;
#endif
#define PMIC_BUCK0_CTRL0_buck0_dbias_START (0)
#define PMIC_BUCK0_CTRL0_buck0_dbias_END (3)
#define PMIC_BUCK0_CTRL0_buck0_adj_rlx_START (4)
#define PMIC_BUCK0_CTRL0_buck0_adj_rlx_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_ng_dt_sel : 1;
        unsigned char buck0_pg_dt_sel : 1;
        unsigned char buck0_nmos_switch : 1;
        unsigned char reserved : 1;
        unsigned char buck0_dt_sel : 2;
        unsigned char buck0_ocp_sel : 2;
    } reg;
} PMIC_BUCK0_CTRL1_UNION;
#endif
#define PMIC_BUCK0_CTRL1_buck0_ng_dt_sel_START (0)
#define PMIC_BUCK0_CTRL1_buck0_ng_dt_sel_END (0)
#define PMIC_BUCK0_CTRL1_buck0_pg_dt_sel_START (1)
#define PMIC_BUCK0_CTRL1_buck0_pg_dt_sel_END (1)
#define PMIC_BUCK0_CTRL1_buck0_nmos_switch_START (2)
#define PMIC_BUCK0_CTRL1_buck0_nmos_switch_END (2)
#define PMIC_BUCK0_CTRL1_buck0_dt_sel_START (4)
#define PMIC_BUCK0_CTRL1_buck0_dt_sel_END (5)
#define PMIC_BUCK0_CTRL1_buck0_ocp_sel_START (6)
#define PMIC_BUCK0_CTRL1_buck0_ocp_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_ng_n_sel : 2;
        unsigned char buck0_ng_p_sel : 2;
        unsigned char buck0_pg_n_sel : 2;
        unsigned char buck0_pg_p_sel : 2;
    } reg;
} PMIC_BUCK0_CTRL2_UNION;
#endif
#define PMIC_BUCK0_CTRL2_buck0_ng_n_sel_START (0)
#define PMIC_BUCK0_CTRL2_buck0_ng_n_sel_END (1)
#define PMIC_BUCK0_CTRL2_buck0_ng_p_sel_START (2)
#define PMIC_BUCK0_CTRL2_buck0_ng_p_sel_END (3)
#define PMIC_BUCK0_CTRL2_buck0_pg_n_sel_START (4)
#define PMIC_BUCK0_CTRL2_buck0_pg_n_sel_END (5)
#define PMIC_BUCK0_CTRL2_buck0_pg_p_sel_START (6)
#define PMIC_BUCK0_CTRL2_buck0_pg_p_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_reg_r : 2;
        unsigned char reserved : 2;
        unsigned char buck0_reg_en : 1;
        unsigned char buck0_adj_clx : 2;
        unsigned char buck0_ocp_dis : 1;
    } reg;
} PMIC_BUCK0_CTRL3_UNION;
#endif
#define PMIC_BUCK0_CTRL3_buck0_reg_r_START (0)
#define PMIC_BUCK0_CTRL3_buck0_reg_r_END (1)
#define PMIC_BUCK0_CTRL3_buck0_reg_en_START (4)
#define PMIC_BUCK0_CTRL3_buck0_reg_en_END (4)
#define PMIC_BUCK0_CTRL3_buck0_adj_clx_START (5)
#define PMIC_BUCK0_CTRL3_buck0_adj_clx_END (6)
#define PMIC_BUCK0_CTRL3_buck0_ocp_dis_START (7)
#define PMIC_BUCK0_CTRL3_buck0_ocp_dis_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_short_pdp : 1;
        unsigned char buck0_reg_ss : 1;
        unsigned char buck0_regop_c : 1;
        unsigned char buck0_filter_ton : 2;
        unsigned char buck0_reg_dr : 3;
    } reg;
} PMIC_BUCK0_CTRL4_UNION;
#endif
#define PMIC_BUCK0_CTRL4_buck0_short_pdp_START (0)
#define PMIC_BUCK0_CTRL4_buck0_short_pdp_END (0)
#define PMIC_BUCK0_CTRL4_buck0_reg_ss_START (1)
#define PMIC_BUCK0_CTRL4_buck0_reg_ss_END (1)
#define PMIC_BUCK0_CTRL4_buck0_regop_c_START (2)
#define PMIC_BUCK0_CTRL4_buck0_regop_c_END (2)
#define PMIC_BUCK0_CTRL4_buck0_filter_ton_START (3)
#define PMIC_BUCK0_CTRL4_buck0_filter_ton_END (4)
#define PMIC_BUCK0_CTRL4_buck0_reg_dr_START (5)
#define PMIC_BUCK0_CTRL4_buck0_reg_dr_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_ton : 2;
        unsigned char buck0_eco_ng : 1;
        unsigned char buck0_reg_bias : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_BUCK0_CTRL5_UNION;
#endif
#define PMIC_BUCK0_CTRL5_buck0_ton_START (0)
#define PMIC_BUCK0_CTRL5_buck0_ton_END (1)
#define PMIC_BUCK0_CTRL5_buck0_eco_ng_START (2)
#define PMIC_BUCK0_CTRL5_buck0_eco_ng_END (2)
#define PMIC_BUCK0_CTRL5_buck0_reg_bias_START (3)
#define PMIC_BUCK0_CTRL5_buck0_reg_bias_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_dmd_ton : 3;
        unsigned char buck0_ocp_toff : 2;
        unsigned char buck0_dmd_sel : 3;
    } reg;
} PMIC_BUCK0_CTRL6_UNION;
#endif
#define PMIC_BUCK0_CTRL6_buck0_dmd_ton_START (0)
#define PMIC_BUCK0_CTRL6_buck0_dmd_ton_END (2)
#define PMIC_BUCK0_CTRL6_buck0_ocp_toff_START (3)
#define PMIC_BUCK0_CTRL6_buck0_ocp_toff_END (4)
#define PMIC_BUCK0_CTRL6_buck0_dmd_sel_START (5)
#define PMIC_BUCK0_CTRL6_buck0_dmd_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_ton_dmd : 1;
        unsigned char buck0_eco_dmd : 1;
        unsigned char buck0_cmp_filter : 1;
        unsigned char buck0_ocp_delay : 1;
        unsigned char buck0_dmd_clamp : 1;
        unsigned char buck0_regop_clamp : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK0_CTRL7_UNION;
#endif
#define PMIC_BUCK0_CTRL7_buck0_ton_dmd_START (0)
#define PMIC_BUCK0_CTRL7_buck0_ton_dmd_END (0)
#define PMIC_BUCK0_CTRL7_buck0_eco_dmd_START (1)
#define PMIC_BUCK0_CTRL7_buck0_eco_dmd_END (1)
#define PMIC_BUCK0_CTRL7_buck0_cmp_filter_START (2)
#define PMIC_BUCK0_CTRL7_buck0_cmp_filter_END (2)
#define PMIC_BUCK0_CTRL7_buck0_ocp_delay_START (3)
#define PMIC_BUCK0_CTRL7_buck0_ocp_delay_END (3)
#define PMIC_BUCK0_CTRL7_buck0_dmd_clamp_START (4)
#define PMIC_BUCK0_CTRL7_buck0_dmd_clamp_END (4)
#define PMIC_BUCK0_CTRL7_buck0_regop_clamp_START (5)
#define PMIC_BUCK0_CTRL7_buck0_regop_clamp_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_reserve : 8;
    } reg;
} PMIC_BUCK0_CTRL8_UNION;
#endif
#define PMIC_BUCK0_CTRL8_buck0_reserve_START (0)
#define PMIC_BUCK0_CTRL8_buck0_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_dbias : 4;
        unsigned char buck1_adj_rlx : 4;
    } reg;
} PMIC_BUCK1_CTRL0_UNION;
#endif
#define PMIC_BUCK1_CTRL0_buck1_dbias_START (0)
#define PMIC_BUCK1_CTRL0_buck1_dbias_END (3)
#define PMIC_BUCK1_CTRL0_buck1_adj_rlx_START (4)
#define PMIC_BUCK1_CTRL0_buck1_adj_rlx_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_ng_dt_sel : 1;
        unsigned char buck1_pg_dt_sel : 1;
        unsigned char buck1_nmos_switch : 1;
        unsigned char reserved : 1;
        unsigned char buck1_dt_sel : 2;
        unsigned char buck1_ocp_sel : 2;
    } reg;
} PMIC_BUCK1_CTRL1_UNION;
#endif
#define PMIC_BUCK1_CTRL1_buck1_ng_dt_sel_START (0)
#define PMIC_BUCK1_CTRL1_buck1_ng_dt_sel_END (0)
#define PMIC_BUCK1_CTRL1_buck1_pg_dt_sel_START (1)
#define PMIC_BUCK1_CTRL1_buck1_pg_dt_sel_END (1)
#define PMIC_BUCK1_CTRL1_buck1_nmos_switch_START (2)
#define PMIC_BUCK1_CTRL1_buck1_nmos_switch_END (2)
#define PMIC_BUCK1_CTRL1_buck1_dt_sel_START (4)
#define PMIC_BUCK1_CTRL1_buck1_dt_sel_END (5)
#define PMIC_BUCK1_CTRL1_buck1_ocp_sel_START (6)
#define PMIC_BUCK1_CTRL1_buck1_ocp_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_ng_n_sel : 2;
        unsigned char buck1_ng_p_sel : 2;
        unsigned char buck1_pg_n_sel : 2;
        unsigned char buck1_pg_p_sel : 2;
    } reg;
} PMIC_BUCK1_CTRL2_UNION;
#endif
#define PMIC_BUCK1_CTRL2_buck1_ng_n_sel_START (0)
#define PMIC_BUCK1_CTRL2_buck1_ng_n_sel_END (1)
#define PMIC_BUCK1_CTRL2_buck1_ng_p_sel_START (2)
#define PMIC_BUCK1_CTRL2_buck1_ng_p_sel_END (3)
#define PMIC_BUCK1_CTRL2_buck1_pg_n_sel_START (4)
#define PMIC_BUCK1_CTRL2_buck1_pg_n_sel_END (5)
#define PMIC_BUCK1_CTRL2_buck1_pg_p_sel_START (6)
#define PMIC_BUCK1_CTRL2_buck1_pg_p_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_reg_r : 2;
        unsigned char reserved : 2;
        unsigned char buck1_reg_en : 1;
        unsigned char buck1_adj_clx : 2;
        unsigned char buck1_ocp_dis : 1;
    } reg;
} PMIC_BUCK1_CTRL3_UNION;
#endif
#define PMIC_BUCK1_CTRL3_buck1_reg_r_START (0)
#define PMIC_BUCK1_CTRL3_buck1_reg_r_END (1)
#define PMIC_BUCK1_CTRL3_buck1_reg_en_START (4)
#define PMIC_BUCK1_CTRL3_buck1_reg_en_END (4)
#define PMIC_BUCK1_CTRL3_buck1_adj_clx_START (5)
#define PMIC_BUCK1_CTRL3_buck1_adj_clx_END (6)
#define PMIC_BUCK1_CTRL3_buck1_ocp_dis_START (7)
#define PMIC_BUCK1_CTRL3_buck1_ocp_dis_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_short_pdp : 1;
        unsigned char buck1_reg_ss : 1;
        unsigned char buck1_regop_c : 1;
        unsigned char buck1_filter_ton : 2;
        unsigned char buck1_reg_dr : 3;
    } reg;
} PMIC_BUCK1_CTRL4_UNION;
#endif
#define PMIC_BUCK1_CTRL4_buck1_short_pdp_START (0)
#define PMIC_BUCK1_CTRL4_buck1_short_pdp_END (0)
#define PMIC_BUCK1_CTRL4_buck1_reg_ss_START (1)
#define PMIC_BUCK1_CTRL4_buck1_reg_ss_END (1)
#define PMIC_BUCK1_CTRL4_buck1_regop_c_START (2)
#define PMIC_BUCK1_CTRL4_buck1_regop_c_END (2)
#define PMIC_BUCK1_CTRL4_buck1_filter_ton_START (3)
#define PMIC_BUCK1_CTRL4_buck1_filter_ton_END (4)
#define PMIC_BUCK1_CTRL4_buck1_reg_dr_START (5)
#define PMIC_BUCK1_CTRL4_buck1_reg_dr_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_ton : 2;
        unsigned char buck1_eco_ng : 1;
        unsigned char buck1_reg_bias : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_BUCK1_CTRL5_UNION;
#endif
#define PMIC_BUCK1_CTRL5_buck1_ton_START (0)
#define PMIC_BUCK1_CTRL5_buck1_ton_END (1)
#define PMIC_BUCK1_CTRL5_buck1_eco_ng_START (2)
#define PMIC_BUCK1_CTRL5_buck1_eco_ng_END (2)
#define PMIC_BUCK1_CTRL5_buck1_reg_bias_START (3)
#define PMIC_BUCK1_CTRL5_buck1_reg_bias_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_dmd_ton : 3;
        unsigned char buck1_ocp_toff : 2;
        unsigned char buck1_dmd_sel : 3;
    } reg;
} PMIC_BUCK1_CTRL6_UNION;
#endif
#define PMIC_BUCK1_CTRL6_buck1_dmd_ton_START (0)
#define PMIC_BUCK1_CTRL6_buck1_dmd_ton_END (2)
#define PMIC_BUCK1_CTRL6_buck1_ocp_toff_START (3)
#define PMIC_BUCK1_CTRL6_buck1_ocp_toff_END (4)
#define PMIC_BUCK1_CTRL6_buck1_dmd_sel_START (5)
#define PMIC_BUCK1_CTRL6_buck1_dmd_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_ton_dmd : 1;
        unsigned char buck1_eco_dmd : 1;
        unsigned char buck1_cmp_filter : 1;
        unsigned char buck1_ocp_delay : 1;
        unsigned char buck1_dmd_clamp : 1;
        unsigned char buck1_regop_clamp : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK1_CTRL7_UNION;
#endif
#define PMIC_BUCK1_CTRL7_buck1_ton_dmd_START (0)
#define PMIC_BUCK1_CTRL7_buck1_ton_dmd_END (0)
#define PMIC_BUCK1_CTRL7_buck1_eco_dmd_START (1)
#define PMIC_BUCK1_CTRL7_buck1_eco_dmd_END (1)
#define PMIC_BUCK1_CTRL7_buck1_cmp_filter_START (2)
#define PMIC_BUCK1_CTRL7_buck1_cmp_filter_END (2)
#define PMIC_BUCK1_CTRL7_buck1_ocp_delay_START (3)
#define PMIC_BUCK1_CTRL7_buck1_ocp_delay_END (3)
#define PMIC_BUCK1_CTRL7_buck1_dmd_clamp_START (4)
#define PMIC_BUCK1_CTRL7_buck1_dmd_clamp_END (4)
#define PMIC_BUCK1_CTRL7_buck1_regop_clamp_START (5)
#define PMIC_BUCK1_CTRL7_buck1_regop_clamp_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_reserve : 8;
    } reg;
} PMIC_BUCK1_CTRL8_UNION;
#endif
#define PMIC_BUCK1_CTRL8_buck1_reserve_START (0)
#define PMIC_BUCK1_CTRL8_buck1_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_dbias : 4;
        unsigned char buck2_adj_rlx : 4;
    } reg;
} PMIC_BUCK2_CTRL0_UNION;
#endif
#define PMIC_BUCK2_CTRL0_buck2_dbias_START (0)
#define PMIC_BUCK2_CTRL0_buck2_dbias_END (3)
#define PMIC_BUCK2_CTRL0_buck2_adj_rlx_START (4)
#define PMIC_BUCK2_CTRL0_buck2_adj_rlx_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_ng_dt_sel : 1;
        unsigned char buck2_pg_dt_sel : 1;
        unsigned char buck2_nmos_switch : 1;
        unsigned char reserved : 1;
        unsigned char buck2_dt_sel : 2;
        unsigned char buck2_ocp_sel : 2;
    } reg;
} PMIC_BUCK2_CTRL1_UNION;
#endif
#define PMIC_BUCK2_CTRL1_buck2_ng_dt_sel_START (0)
#define PMIC_BUCK2_CTRL1_buck2_ng_dt_sel_END (0)
#define PMIC_BUCK2_CTRL1_buck2_pg_dt_sel_START (1)
#define PMIC_BUCK2_CTRL1_buck2_pg_dt_sel_END (1)
#define PMIC_BUCK2_CTRL1_buck2_nmos_switch_START (2)
#define PMIC_BUCK2_CTRL1_buck2_nmos_switch_END (2)
#define PMIC_BUCK2_CTRL1_buck2_dt_sel_START (4)
#define PMIC_BUCK2_CTRL1_buck2_dt_sel_END (5)
#define PMIC_BUCK2_CTRL1_buck2_ocp_sel_START (6)
#define PMIC_BUCK2_CTRL1_buck2_ocp_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_ng_n_sel : 2;
        unsigned char buck2_ng_p_sel : 2;
        unsigned char buck2_pg_n_sel : 2;
        unsigned char buck2_pg_p_sel : 2;
    } reg;
} PMIC_BUCK2_CTRL2_UNION;
#endif
#define PMIC_BUCK2_CTRL2_buck2_ng_n_sel_START (0)
#define PMIC_BUCK2_CTRL2_buck2_ng_n_sel_END (1)
#define PMIC_BUCK2_CTRL2_buck2_ng_p_sel_START (2)
#define PMIC_BUCK2_CTRL2_buck2_ng_p_sel_END (3)
#define PMIC_BUCK2_CTRL2_buck2_pg_n_sel_START (4)
#define PMIC_BUCK2_CTRL2_buck2_pg_n_sel_END (5)
#define PMIC_BUCK2_CTRL2_buck2_pg_p_sel_START (6)
#define PMIC_BUCK2_CTRL2_buck2_pg_p_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_reg_r : 2;
        unsigned char reserved : 2;
        unsigned char buck2_reg_en : 1;
        unsigned char buck2_adj_clx : 2;
        unsigned char buck2_ocp_dis : 1;
    } reg;
} PMIC_BUCK2_CTRL3_UNION;
#endif
#define PMIC_BUCK2_CTRL3_buck2_reg_r_START (0)
#define PMIC_BUCK2_CTRL3_buck2_reg_r_END (1)
#define PMIC_BUCK2_CTRL3_buck2_reg_en_START (4)
#define PMIC_BUCK2_CTRL3_buck2_reg_en_END (4)
#define PMIC_BUCK2_CTRL3_buck2_adj_clx_START (5)
#define PMIC_BUCK2_CTRL3_buck2_adj_clx_END (6)
#define PMIC_BUCK2_CTRL3_buck2_ocp_dis_START (7)
#define PMIC_BUCK2_CTRL3_buck2_ocp_dis_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_short_pdp : 1;
        unsigned char buck2_reg_ss : 1;
        unsigned char buck2_regop_c : 1;
        unsigned char buck2_filter_ton : 2;
        unsigned char buck2_reg_dr : 3;
    } reg;
} PMIC_BUCK2_CTRL4_UNION;
#endif
#define PMIC_BUCK2_CTRL4_buck2_short_pdp_START (0)
#define PMIC_BUCK2_CTRL4_buck2_short_pdp_END (0)
#define PMIC_BUCK2_CTRL4_buck2_reg_ss_START (1)
#define PMIC_BUCK2_CTRL4_buck2_reg_ss_END (1)
#define PMIC_BUCK2_CTRL4_buck2_regop_c_START (2)
#define PMIC_BUCK2_CTRL4_buck2_regop_c_END (2)
#define PMIC_BUCK2_CTRL4_buck2_filter_ton_START (3)
#define PMIC_BUCK2_CTRL4_buck2_filter_ton_END (4)
#define PMIC_BUCK2_CTRL4_buck2_reg_dr_START (5)
#define PMIC_BUCK2_CTRL4_buck2_reg_dr_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_ton : 2;
        unsigned char buck2_eco_ng : 1;
        unsigned char buck2_reg_bias : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_BUCK2_CTRL5_UNION;
#endif
#define PMIC_BUCK2_CTRL5_buck2_ton_START (0)
#define PMIC_BUCK2_CTRL5_buck2_ton_END (1)
#define PMIC_BUCK2_CTRL5_buck2_eco_ng_START (2)
#define PMIC_BUCK2_CTRL5_buck2_eco_ng_END (2)
#define PMIC_BUCK2_CTRL5_buck2_reg_bias_START (3)
#define PMIC_BUCK2_CTRL5_buck2_reg_bias_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_dmd_ton : 3;
        unsigned char buck2_ocp_toff : 2;
        unsigned char buck2_dmd_sel : 3;
    } reg;
} PMIC_BUCK2_CTRL6_UNION;
#endif
#define PMIC_BUCK2_CTRL6_buck2_dmd_ton_START (0)
#define PMIC_BUCK2_CTRL6_buck2_dmd_ton_END (2)
#define PMIC_BUCK2_CTRL6_buck2_ocp_toff_START (3)
#define PMIC_BUCK2_CTRL6_buck2_ocp_toff_END (4)
#define PMIC_BUCK2_CTRL6_buck2_dmd_sel_START (5)
#define PMIC_BUCK2_CTRL6_buck2_dmd_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_ton_dmd : 1;
        unsigned char buck2_eco_dmd : 1;
        unsigned char buck2_cmp_filter : 1;
        unsigned char buck2_ocp_delay : 1;
        unsigned char buck2_dmd_clamp : 1;
        unsigned char buck2_regop_clamp : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK2_CTRL7_UNION;
#endif
#define PMIC_BUCK2_CTRL7_buck2_ton_dmd_START (0)
#define PMIC_BUCK2_CTRL7_buck2_ton_dmd_END (0)
#define PMIC_BUCK2_CTRL7_buck2_eco_dmd_START (1)
#define PMIC_BUCK2_CTRL7_buck2_eco_dmd_END (1)
#define PMIC_BUCK2_CTRL7_buck2_cmp_filter_START (2)
#define PMIC_BUCK2_CTRL7_buck2_cmp_filter_END (2)
#define PMIC_BUCK2_CTRL7_buck2_ocp_delay_START (3)
#define PMIC_BUCK2_CTRL7_buck2_ocp_delay_END (3)
#define PMIC_BUCK2_CTRL7_buck2_dmd_clamp_START (4)
#define PMIC_BUCK2_CTRL7_buck2_dmd_clamp_END (4)
#define PMIC_BUCK2_CTRL7_buck2_regop_clamp_START (5)
#define PMIC_BUCK2_CTRL7_buck2_regop_clamp_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_reserve : 8;
    } reg;
} PMIC_BUCK2_CTRL8_UNION;
#endif
#define PMIC_BUCK2_CTRL8_buck2_reserve_START (0)
#define PMIC_BUCK2_CTRL8_buck2_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_dbias : 4;
        unsigned char buck3_adj_rlx : 4;
    } reg;
} PMIC_BUCK3_CTRL0_UNION;
#endif
#define PMIC_BUCK3_CTRL0_buck3_dbias_START (0)
#define PMIC_BUCK3_CTRL0_buck3_dbias_END (3)
#define PMIC_BUCK3_CTRL0_buck3_adj_rlx_START (4)
#define PMIC_BUCK3_CTRL0_buck3_adj_rlx_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_ng_dt_sel : 1;
        unsigned char buck3_pg_dt_sel : 1;
        unsigned char buck3_nmos_switch : 1;
        unsigned char reserved : 1;
        unsigned char buck3_dt_sel : 2;
        unsigned char buck3_ocp_sel : 2;
    } reg;
} PMIC_BUCK3_CTRL1_UNION;
#endif
#define PMIC_BUCK3_CTRL1_buck3_ng_dt_sel_START (0)
#define PMIC_BUCK3_CTRL1_buck3_ng_dt_sel_END (0)
#define PMIC_BUCK3_CTRL1_buck3_pg_dt_sel_START (1)
#define PMIC_BUCK3_CTRL1_buck3_pg_dt_sel_END (1)
#define PMIC_BUCK3_CTRL1_buck3_nmos_switch_START (2)
#define PMIC_BUCK3_CTRL1_buck3_nmos_switch_END (2)
#define PMIC_BUCK3_CTRL1_buck3_dt_sel_START (4)
#define PMIC_BUCK3_CTRL1_buck3_dt_sel_END (5)
#define PMIC_BUCK3_CTRL1_buck3_ocp_sel_START (6)
#define PMIC_BUCK3_CTRL1_buck3_ocp_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_ng_n_sel : 2;
        unsigned char buck3_ng_p_sel : 2;
        unsigned char buck3_pg_n_sel : 2;
        unsigned char buck3_pg_p_sel : 2;
    } reg;
} PMIC_BUCK3_CTRL2_UNION;
#endif
#define PMIC_BUCK3_CTRL2_buck3_ng_n_sel_START (0)
#define PMIC_BUCK3_CTRL2_buck3_ng_n_sel_END (1)
#define PMIC_BUCK3_CTRL2_buck3_ng_p_sel_START (2)
#define PMIC_BUCK3_CTRL2_buck3_ng_p_sel_END (3)
#define PMIC_BUCK3_CTRL2_buck3_pg_n_sel_START (4)
#define PMIC_BUCK3_CTRL2_buck3_pg_n_sel_END (5)
#define PMIC_BUCK3_CTRL2_buck3_pg_p_sel_START (6)
#define PMIC_BUCK3_CTRL2_buck3_pg_p_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_reg_r : 2;
        unsigned char reserved : 2;
        unsigned char buck3_reg_en : 1;
        unsigned char buck3_adj_clx : 2;
        unsigned char buck3_ocp_dis : 1;
    } reg;
} PMIC_BUCK3_CTRL3_UNION;
#endif
#define PMIC_BUCK3_CTRL3_buck3_reg_r_START (0)
#define PMIC_BUCK3_CTRL3_buck3_reg_r_END (1)
#define PMIC_BUCK3_CTRL3_buck3_reg_en_START (4)
#define PMIC_BUCK3_CTRL3_buck3_reg_en_END (4)
#define PMIC_BUCK3_CTRL3_buck3_adj_clx_START (5)
#define PMIC_BUCK3_CTRL3_buck3_adj_clx_END (6)
#define PMIC_BUCK3_CTRL3_buck3_ocp_dis_START (7)
#define PMIC_BUCK3_CTRL3_buck3_ocp_dis_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_short_pdp : 1;
        unsigned char buck3_reg_ss : 1;
        unsigned char buck3_regop_c : 1;
        unsigned char buck3_filter_ton : 2;
        unsigned char buck3_reg_dr : 3;
    } reg;
} PMIC_BUCK3_CTRL4_UNION;
#endif
#define PMIC_BUCK3_CTRL4_buck3_short_pdp_START (0)
#define PMIC_BUCK3_CTRL4_buck3_short_pdp_END (0)
#define PMIC_BUCK3_CTRL4_buck3_reg_ss_START (1)
#define PMIC_BUCK3_CTRL4_buck3_reg_ss_END (1)
#define PMIC_BUCK3_CTRL4_buck3_regop_c_START (2)
#define PMIC_BUCK3_CTRL4_buck3_regop_c_END (2)
#define PMIC_BUCK3_CTRL4_buck3_filter_ton_START (3)
#define PMIC_BUCK3_CTRL4_buck3_filter_ton_END (4)
#define PMIC_BUCK3_CTRL4_buck3_reg_dr_START (5)
#define PMIC_BUCK3_CTRL4_buck3_reg_dr_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_ton : 2;
        unsigned char buck3_eco_ng : 1;
        unsigned char buck3_reg_bias : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_BUCK3_CTRL5_UNION;
#endif
#define PMIC_BUCK3_CTRL5_buck3_ton_START (0)
#define PMIC_BUCK3_CTRL5_buck3_ton_END (1)
#define PMIC_BUCK3_CTRL5_buck3_eco_ng_START (2)
#define PMIC_BUCK3_CTRL5_buck3_eco_ng_END (2)
#define PMIC_BUCK3_CTRL5_buck3_reg_bias_START (3)
#define PMIC_BUCK3_CTRL5_buck3_reg_bias_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_dmd_ton : 3;
        unsigned char buck3_ocp_toff : 2;
        unsigned char buck3_dmd_sel : 3;
    } reg;
} PMIC_BUCK3_CTRL6_UNION;
#endif
#define PMIC_BUCK3_CTRL6_buck3_dmd_ton_START (0)
#define PMIC_BUCK3_CTRL6_buck3_dmd_ton_END (2)
#define PMIC_BUCK3_CTRL6_buck3_ocp_toff_START (3)
#define PMIC_BUCK3_CTRL6_buck3_ocp_toff_END (4)
#define PMIC_BUCK3_CTRL6_buck3_dmd_sel_START (5)
#define PMIC_BUCK3_CTRL6_buck3_dmd_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_ton_dmd : 1;
        unsigned char buck3_eco_dmd : 1;
        unsigned char buck3_cmp_filter : 1;
        unsigned char buck3_ocp_delay : 1;
        unsigned char buck3_dmd_clamp : 1;
        unsigned char buck3_regop_clamp : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK3_CTRL7_UNION;
#endif
#define PMIC_BUCK3_CTRL7_buck3_ton_dmd_START (0)
#define PMIC_BUCK3_CTRL7_buck3_ton_dmd_END (0)
#define PMIC_BUCK3_CTRL7_buck3_eco_dmd_START (1)
#define PMIC_BUCK3_CTRL7_buck3_eco_dmd_END (1)
#define PMIC_BUCK3_CTRL7_buck3_cmp_filter_START (2)
#define PMIC_BUCK3_CTRL7_buck3_cmp_filter_END (2)
#define PMIC_BUCK3_CTRL7_buck3_ocp_delay_START (3)
#define PMIC_BUCK3_CTRL7_buck3_ocp_delay_END (3)
#define PMIC_BUCK3_CTRL7_buck3_dmd_clamp_START (4)
#define PMIC_BUCK3_CTRL7_buck3_dmd_clamp_END (4)
#define PMIC_BUCK3_CTRL7_buck3_regop_clamp_START (5)
#define PMIC_BUCK3_CTRL7_buck3_regop_clamp_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_reserve : 8;
    } reg;
} PMIC_BUCK3_CTRL8_UNION;
#endif
#define PMIC_BUCK3_CTRL8_buck3_reserve_START (0)
#define PMIC_BUCK3_CTRL8_buck3_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck_reserve0 : 8;
    } reg;
} PMIC_BUCK_RESERVE0_UNION;
#endif
#define PMIC_BUCK_RESERVE0_buck_reserve0_START (0)
#define PMIC_BUCK_RESERVE0_buck_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck_reserve1 : 8;
    } reg;
} PMIC_BUCK_RESERVE1_UNION;
#endif
#define PMIC_BUCK_RESERVE1_buck_reserve1_START (0)
#define PMIC_BUCK_RESERVE1_buck_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo1_vrset : 3;
        unsigned char ldo1_ocp_enn : 1;
        unsigned char ldo0_2_vrset : 3;
        unsigned char ldo0_2_ocp_enn : 1;
    } reg;
} PMIC_LDO_1_CTRL_UNION;
#endif
#define PMIC_LDO_1_CTRL_ldo1_vrset_START (0)
#define PMIC_LDO_1_CTRL_ldo1_vrset_END (2)
#define PMIC_LDO_1_CTRL_ldo1_ocp_enn_START (3)
#define PMIC_LDO_1_CTRL_ldo1_ocp_enn_END (3)
#define PMIC_LDO_1_CTRL_ldo0_2_vrset_START (4)
#define PMIC_LDO_1_CTRL_ldo0_2_vrset_END (6)
#define PMIC_LDO_1_CTRL_ldo0_2_ocp_enn_START (7)
#define PMIC_LDO_1_CTRL_ldo0_2_ocp_enn_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo1_ref_buf_bypass : 1;
        unsigned char ldo1_psrr2_set : 3;
        unsigned char ldo1_psrr1_set : 3;
        unsigned char ldo1_psrr_en : 1;
    } reg;
} PMIC_LDO1_CTRL_0_UNION;
#endif
#define PMIC_LDO1_CTRL_0_ldo1_ref_buf_bypass_START (0)
#define PMIC_LDO1_CTRL_0_ldo1_ref_buf_bypass_END (0)
#define PMIC_LDO1_CTRL_0_ldo1_psrr2_set_START (1)
#define PMIC_LDO1_CTRL_0_ldo1_psrr2_set_END (3)
#define PMIC_LDO1_CTRL_0_ldo1_psrr1_set_START (4)
#define PMIC_LDO1_CTRL_0_ldo1_psrr1_set_END (6)
#define PMIC_LDO1_CTRL_0_ldo1_psrr_en_START (7)
#define PMIC_LDO1_CTRL_0_ldo1_psrr_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo1_ocps_en : 1;
        unsigned char ldo1_bw_en : 1;
        unsigned char ldo1_vgpr_en : 1;
        unsigned char ldo1_ref_buf_rset : 2;
        unsigned char ldo1_comp : 3;
    } reg;
} PMIC_LDO1_CTRL_1_UNION;
#endif
#define PMIC_LDO1_CTRL_1_ldo1_ocps_en_START (0)
#define PMIC_LDO1_CTRL_1_ldo1_ocps_en_END (0)
#define PMIC_LDO1_CTRL_1_ldo1_bw_en_START (1)
#define PMIC_LDO1_CTRL_1_ldo1_bw_en_END (1)
#define PMIC_LDO1_CTRL_1_ldo1_vgpr_en_START (2)
#define PMIC_LDO1_CTRL_1_ldo1_vgpr_en_END (2)
#define PMIC_LDO1_CTRL_1_ldo1_ref_buf_rset_START (3)
#define PMIC_LDO1_CTRL_1_ldo1_ref_buf_rset_END (4)
#define PMIC_LDO1_CTRL_1_ldo1_comp_START (5)
#define PMIC_LDO1_CTRL_1_ldo1_comp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo3_vrset : 3;
        unsigned char ldo3_ocp_enn : 1;
        unsigned char ldo2_vrset : 3;
        unsigned char ldo2_ocp_enn : 1;
    } reg;
} PMIC_LDO2_3_CTRL_UNION;
#endif
#define PMIC_LDO2_3_CTRL_ldo3_vrset_START (0)
#define PMIC_LDO2_3_CTRL_ldo3_vrset_END (2)
#define PMIC_LDO2_3_CTRL_ldo3_ocp_enn_START (3)
#define PMIC_LDO2_3_CTRL_ldo3_ocp_enn_END (3)
#define PMIC_LDO2_3_CTRL_ldo2_vrset_START (4)
#define PMIC_LDO2_3_CTRL_ldo2_vrset_END (6)
#define PMIC_LDO2_3_CTRL_ldo2_ocp_enn_START (7)
#define PMIC_LDO2_3_CTRL_ldo2_ocp_enn_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo3_i_sst : 1;
        unsigned char ldo3_psrr2_set : 3;
        unsigned char ldo3_psrr1_set : 3;
        unsigned char ldo3_psrr_en : 1;
    } reg;
} PMIC_LDO3_CTRL_UNION;
#endif
#define PMIC_LDO3_CTRL_ldo3_i_sst_START (0)
#define PMIC_LDO3_CTRL_ldo3_i_sst_END (0)
#define PMIC_LDO3_CTRL_ldo3_psrr2_set_START (1)
#define PMIC_LDO3_CTRL_ldo3_psrr2_set_END (3)
#define PMIC_LDO3_CTRL_ldo3_psrr1_set_START (4)
#define PMIC_LDO3_CTRL_ldo3_psrr1_set_END (6)
#define PMIC_LDO3_CTRL_ldo3_psrr_en_START (7)
#define PMIC_LDO3_CTRL_ldo3_psrr_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo5_vrset : 3;
        unsigned char ldo5_ocp_enn : 1;
        unsigned char ldo4_vrset : 3;
        unsigned char ldo4_ocp_enn : 1;
    } reg;
} PMIC_LDO4_5_CTRL_UNION;
#endif
#define PMIC_LDO4_5_CTRL_ldo5_vrset_START (0)
#define PMIC_LDO4_5_CTRL_ldo5_vrset_END (2)
#define PMIC_LDO4_5_CTRL_ldo5_ocp_enn_START (3)
#define PMIC_LDO4_5_CTRL_ldo5_ocp_enn_END (3)
#define PMIC_LDO4_5_CTRL_ldo4_vrset_START (4)
#define PMIC_LDO4_5_CTRL_ldo4_vrset_END (6)
#define PMIC_LDO4_5_CTRL_ldo4_ocp_enn_START (7)
#define PMIC_LDO4_5_CTRL_ldo4_ocp_enn_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo8_vrset : 3;
        unsigned char ldo8_ocp_enn : 1;
        unsigned char ldo7_vrset : 3;
        unsigned char ldo7_ocp_enn : 1;
    } reg;
} PMIC_LDO7_8_CTRL_UNION;
#endif
#define PMIC_LDO7_8_CTRL_ldo8_vrset_START (0)
#define PMIC_LDO7_8_CTRL_ldo8_vrset_END (2)
#define PMIC_LDO7_8_CTRL_ldo8_ocp_enn_START (3)
#define PMIC_LDO7_8_CTRL_ldo8_ocp_enn_END (3)
#define PMIC_LDO7_8_CTRL_ldo7_vrset_START (4)
#define PMIC_LDO7_8_CTRL_ldo7_vrset_END (6)
#define PMIC_LDO7_8_CTRL_ldo7_ocp_enn_START (7)
#define PMIC_LDO7_8_CTRL_ldo7_ocp_enn_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo10_vrset : 3;
        unsigned char ldo10_ocp_enn : 1;
        unsigned char ldo9_vrset : 3;
        unsigned char ldo9_ocp_enn : 1;
    } reg;
} PMIC_LDO9_10_CTRL_UNION;
#endif
#define PMIC_LDO9_10_CTRL_ldo10_vrset_START (0)
#define PMIC_LDO9_10_CTRL_ldo10_vrset_END (2)
#define PMIC_LDO9_10_CTRL_ldo10_ocp_enn_START (3)
#define PMIC_LDO9_10_CTRL_ldo10_ocp_enn_END (3)
#define PMIC_LDO9_10_CTRL_ldo9_vrset_START (4)
#define PMIC_LDO9_10_CTRL_ldo9_vrset_END (6)
#define PMIC_LDO9_10_CTRL_ldo9_ocp_enn_START (7)
#define PMIC_LDO9_10_CTRL_ldo9_ocp_enn_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo12_vrset : 3;
        unsigned char ldo12_ocp_enn : 1;
        unsigned char ldo11_vrset : 3;
        unsigned char ldo11_ocp_enn : 1;
    } reg;
} PMIC_LD11_12_CTRL_UNION;
#endif
#define PMIC_LD11_12_CTRL_ldo12_vrset_START (0)
#define PMIC_LD11_12_CTRL_ldo12_vrset_END (2)
#define PMIC_LD11_12_CTRL_ldo12_ocp_enn_START (3)
#define PMIC_LD11_12_CTRL_ldo12_ocp_enn_END (3)
#define PMIC_LD11_12_CTRL_ldo11_vrset_START (4)
#define PMIC_LD11_12_CTRL_ldo11_vrset_END (6)
#define PMIC_LD11_12_CTRL_ldo11_ocp_enn_START (7)
#define PMIC_LD11_12_CTRL_ldo11_ocp_enn_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo14_vrset : 3;
        unsigned char ldo14_ocp_enn : 1;
        unsigned char ldo13_vrset : 3;
        unsigned char ldo13_ocp_enn : 1;
    } reg;
} PMIC_LDO13_14_CTRL_UNION;
#endif
#define PMIC_LDO13_14_CTRL_ldo14_vrset_START (0)
#define PMIC_LDO13_14_CTRL_ldo14_vrset_END (2)
#define PMIC_LDO13_14_CTRL_ldo14_ocp_enn_START (3)
#define PMIC_LDO13_14_CTRL_ldo14_ocp_enn_END (3)
#define PMIC_LDO13_14_CTRL_ldo13_vrset_START (4)
#define PMIC_LDO13_14_CTRL_ldo13_vrset_END (6)
#define PMIC_LDO13_14_CTRL_ldo13_ocp_enn_START (7)
#define PMIC_LDO13_14_CTRL_ldo13_ocp_enn_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo16_vrset : 3;
        unsigned char ldo16_ocp_enn : 1;
        unsigned char ldo15_vrset : 3;
        unsigned char ldo15_ocp_enn : 1;
    } reg;
} PMIC_LDO15_16_CTRL_UNION;
#endif
#define PMIC_LDO15_16_CTRL_ldo16_vrset_START (0)
#define PMIC_LDO15_16_CTRL_ldo16_vrset_END (2)
#define PMIC_LDO15_16_CTRL_ldo16_ocp_enn_START (3)
#define PMIC_LDO15_16_CTRL_ldo16_ocp_enn_END (3)
#define PMIC_LDO15_16_CTRL_ldo15_vrset_START (4)
#define PMIC_LDO15_16_CTRL_ldo15_vrset_END (6)
#define PMIC_LDO15_16_CTRL_ldo15_ocp_enn_START (7)
#define PMIC_LDO15_16_CTRL_ldo15_ocp_enn_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo19_vrset : 3;
        unsigned char ldo19_ocp_enn : 1;
        unsigned char ldo17_vrset : 3;
        unsigned char ldo17_ocp_enn : 1;
    } reg;
} PMIC_LDO17_19_CTRL_UNION;
#endif
#define PMIC_LDO17_19_CTRL_ldo19_vrset_START (0)
#define PMIC_LDO17_19_CTRL_ldo19_vrset_END (2)
#define PMIC_LDO17_19_CTRL_ldo19_ocp_enn_START (3)
#define PMIC_LDO17_19_CTRL_ldo19_ocp_enn_END (3)
#define PMIC_LDO17_19_CTRL_ldo17_vrset_START (4)
#define PMIC_LDO17_19_CTRL_ldo17_vrset_END (6)
#define PMIC_LDO17_19_CTRL_ldo17_ocp_enn_START (7)
#define PMIC_LDO17_19_CTRL_ldo17_ocp_enn_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo21_vrset : 3;
        unsigned char ldo21_ocp_enn : 1;
        unsigned char ldo20_vrset : 3;
        unsigned char ldo20_ocp_enn : 1;
    } reg;
} PMIC_LDO20_21_CTRL_UNION;
#endif
#define PMIC_LDO20_21_CTRL_ldo21_vrset_START (0)
#define PMIC_LDO20_21_CTRL_ldo21_vrset_END (2)
#define PMIC_LDO20_21_CTRL_ldo21_ocp_enn_START (3)
#define PMIC_LDO20_21_CTRL_ldo21_ocp_enn_END (3)
#define PMIC_LDO20_21_CTRL_ldo20_vrset_START (4)
#define PMIC_LDO20_21_CTRL_ldo20_vrset_END (6)
#define PMIC_LDO20_21_CTRL_ldo20_ocp_enn_START (7)
#define PMIC_LDO20_21_CTRL_ldo20_ocp_enn_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo22_vrset : 3;
        unsigned char ldo22_ocp_enn : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO22_CTRL0_UNION;
#endif
#define PMIC_LDO22_CTRL0_ldo22_vrset_START (0)
#define PMIC_LDO22_CTRL0_ldo22_vrset_END (2)
#define PMIC_LDO22_CTRL0_ldo22_ocp_enn_START (3)
#define PMIC_LDO22_CTRL0_ldo22_ocp_enn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo22_ref_buf_bypass : 1;
        unsigned char ldo22_psrr2_set : 3;
        unsigned char ldo22_psrr1_set : 3;
        unsigned char ldo22_psrr_en : 1;
    } reg;
} PMIC_LDO22_CTRL1_UNION;
#endif
#define PMIC_LDO22_CTRL1_ldo22_ref_buf_bypass_START (0)
#define PMIC_LDO22_CTRL1_ldo22_ref_buf_bypass_END (0)
#define PMIC_LDO22_CTRL1_ldo22_psrr2_set_START (1)
#define PMIC_LDO22_CTRL1_ldo22_psrr2_set_END (3)
#define PMIC_LDO22_CTRL1_ldo22_psrr1_set_START (4)
#define PMIC_LDO22_CTRL1_ldo22_psrr1_set_END (6)
#define PMIC_LDO22_CTRL1_ldo22_psrr_en_START (7)
#define PMIC_LDO22_CTRL1_ldo22_psrr_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo22_ocps_en : 1;
        unsigned char ldo22_bw_en : 1;
        unsigned char ldo22_vgpr_en : 1;
        unsigned char ldo22_ref_buf_rset : 2;
        unsigned char ldo22_comp : 3;
    } reg;
} PMIC_LDO22_CTRL2_UNION;
#endif
#define PMIC_LDO22_CTRL2_ldo22_ocps_en_START (0)
#define PMIC_LDO22_CTRL2_ldo22_ocps_en_END (0)
#define PMIC_LDO22_CTRL2_ldo22_bw_en_START (1)
#define PMIC_LDO22_CTRL2_ldo22_bw_en_END (1)
#define PMIC_LDO22_CTRL2_ldo22_vgpr_en_START (2)
#define PMIC_LDO22_CTRL2_ldo22_vgpr_en_END (2)
#define PMIC_LDO22_CTRL2_ldo22_ref_buf_rset_START (3)
#define PMIC_LDO22_CTRL2_ldo22_ref_buf_rset_END (4)
#define PMIC_LDO22_CTRL2_ldo22_comp_START (5)
#define PMIC_LDO22_CTRL2_ldo22_comp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo23_vrset : 3;
        unsigned char ldo23_ocp_enn : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO23_CTRL_UNION;
#endif
#define PMIC_LDO23_CTRL_ldo23_vrset_START (0)
#define PMIC_LDO23_CTRL_ldo23_vrset_END (2)
#define PMIC_LDO23_CTRL_ldo23_ocp_enn_START (3)
#define PMIC_LDO23_CTRL_ldo23_ocp_enn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo25_vrset : 3;
        unsigned char ldo25_ocp_enn : 1;
        unsigned char ldo24_vrset : 3;
        unsigned char ldo24_ocp_enn : 1;
    } reg;
} PMIC_LDO24_25_CTRL_UNION;
#endif
#define PMIC_LDO24_25_CTRL_ldo25_vrset_START (0)
#define PMIC_LDO24_25_CTRL_ldo25_vrset_END (2)
#define PMIC_LDO24_25_CTRL_ldo25_ocp_enn_START (3)
#define PMIC_LDO24_25_CTRL_ldo25_ocp_enn_END (3)
#define PMIC_LDO24_25_CTRL_ldo24_vrset_START (4)
#define PMIC_LDO24_25_CTRL_ldo24_vrset_END (6)
#define PMIC_LDO24_25_CTRL_ldo24_ocp_enn_START (7)
#define PMIC_LDO24_25_CTRL_ldo24_ocp_enn_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo26_vrset : 3;
        unsigned char ldo26_ocp_enn : 1;
        unsigned char ldo26_i_sst : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO26_CTRL_UNION;
#endif
#define PMIC_LDO26_CTRL_ldo26_vrset_START (0)
#define PMIC_LDO26_CTRL_ldo26_vrset_END (2)
#define PMIC_LDO26_CTRL_ldo26_ocp_enn_START (3)
#define PMIC_LDO26_CTRL_ldo26_ocp_enn_END (3)
#define PMIC_LDO26_CTRL_ldo26_i_sst_START (4)
#define PMIC_LDO26_CTRL_ldo26_i_sst_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo27_vrset : 3;
        unsigned char ldo27_ocp_enn : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO27_CTRL_UNION;
#endif
#define PMIC_LDO27_CTRL_ldo27_vrset_START (0)
#define PMIC_LDO27_CTRL_ldo27_vrset_END (2)
#define PMIC_LDO27_CTRL_ldo27_ocp_enn_START (3)
#define PMIC_LDO27_CTRL_ldo27_ocp_enn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo29_vrset : 3;
        unsigned char ldo29_ocp_enn : 1;
        unsigned char ldo28_vrset : 3;
        unsigned char ldo28_ocp_enn : 1;
    } reg;
} PMIC_LDO28_29_CTRL_UNION;
#endif
#define PMIC_LDO28_29_CTRL_ldo29_vrset_START (0)
#define PMIC_LDO28_29_CTRL_ldo29_vrset_END (2)
#define PMIC_LDO28_29_CTRL_ldo29_ocp_enn_START (3)
#define PMIC_LDO28_29_CTRL_ldo29_ocp_enn_END (3)
#define PMIC_LDO28_29_CTRL_ldo28_vrset_START (4)
#define PMIC_LDO28_29_CTRL_ldo28_vrset_END (6)
#define PMIC_LDO28_29_CTRL_ldo28_ocp_enn_START (7)
#define PMIC_LDO28_29_CTRL_ldo28_ocp_enn_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo32_vrset : 3;
        unsigned char ldo32_ocp_enn : 1;
        unsigned char ldo31_vrset : 3;
        unsigned char ldo31_ocp_enn : 1;
    } reg;
} PMIC_LDO31_32_CTRL_UNION;
#endif
#define PMIC_LDO31_32_CTRL_ldo32_vrset_START (0)
#define PMIC_LDO31_32_CTRL_ldo32_vrset_END (2)
#define PMIC_LDO31_32_CTRL_ldo32_ocp_enn_START (3)
#define PMIC_LDO31_32_CTRL_ldo32_ocp_enn_END (3)
#define PMIC_LDO31_32_CTRL_ldo31_vrset_START (4)
#define PMIC_LDO31_32_CTRL_ldo31_vrset_END (6)
#define PMIC_LDO31_32_CTRL_ldo31_ocp_enn_START (7)
#define PMIC_LDO31_32_CTRL_ldo31_ocp_enn_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo34_vrset : 3;
        unsigned char ldo34_ocp_enn : 1;
        unsigned char ldo33_vrset : 3;
        unsigned char ldo33_ocp_enn : 1;
    } reg;
} PMIC_LDO33_LDO34_CTRL_UNION;
#endif
#define PMIC_LDO33_LDO34_CTRL_ldo34_vrset_START (0)
#define PMIC_LDO33_LDO34_CTRL_ldo34_vrset_END (2)
#define PMIC_LDO33_LDO34_CTRL_ldo34_ocp_enn_START (3)
#define PMIC_LDO33_LDO34_CTRL_ldo34_ocp_enn_END (3)
#define PMIC_LDO33_LDO34_CTRL_ldo33_vrset_START (4)
#define PMIC_LDO33_LDO34_CTRL_ldo33_vrset_END (6)
#define PMIC_LDO33_LDO34_CTRL_ldo33_ocp_enn_START (7)
#define PMIC_LDO33_LDO34_CTRL_ldo33_ocp_enn_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo_reserve0 : 8;
    } reg;
} PMIC_LDO_RESERVE0_UNION;
#endif
#define PMIC_LDO_RESERVE0_ldo_reserve0_START (0)
#define PMIC_LDO_RESERVE0_ldo_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo_reserve1 : 8;
    } reg;
} PMIC_LDO_RESERVE1_UNION;
#endif
#define PMIC_LDO_RESERVE1_ldo_reserve1_START (0)
#define PMIC_LDO_RESERVE1_ldo_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_ocp_auto_stop : 2;
        unsigned char buck2_ocp_auto_stop : 2;
        unsigned char buck1_ocp_auto_stop : 2;
        unsigned char buck0_ocp_auto_stop : 2;
    } reg;
} PMIC_BUCK0_3_OCP_CTRL_UNION;
#endif
#define PMIC_BUCK0_3_OCP_CTRL_buck3_ocp_auto_stop_START (0)
#define PMIC_BUCK0_3_OCP_CTRL_buck3_ocp_auto_stop_END (1)
#define PMIC_BUCK0_3_OCP_CTRL_buck2_ocp_auto_stop_START (2)
#define PMIC_BUCK0_3_OCP_CTRL_buck2_ocp_auto_stop_END (3)
#define PMIC_BUCK0_3_OCP_CTRL_buck1_ocp_auto_stop_START (4)
#define PMIC_BUCK0_3_OCP_CTRL_buck1_ocp_auto_stop_END (5)
#define PMIC_BUCK0_3_OCP_CTRL_buck0_ocp_auto_stop_START (6)
#define PMIC_BUCK0_3_OCP_CTRL_buck0_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo3_ocp_auto_stop : 2;
        unsigned char ldo2_ocp_auto_stop : 2;
        unsigned char ldo1_ocp_auto_stop : 2;
        unsigned char ldo0_ocp_auto_stop : 2;
    } reg;
} PMIC_LDO0_3_OCP_CTRL_UNION;
#endif
#define PMIC_LDO0_3_OCP_CTRL_ldo3_ocp_auto_stop_START (0)
#define PMIC_LDO0_3_OCP_CTRL_ldo3_ocp_auto_stop_END (1)
#define PMIC_LDO0_3_OCP_CTRL_ldo2_ocp_auto_stop_START (2)
#define PMIC_LDO0_3_OCP_CTRL_ldo2_ocp_auto_stop_END (3)
#define PMIC_LDO0_3_OCP_CTRL_ldo1_ocp_auto_stop_START (4)
#define PMIC_LDO0_3_OCP_CTRL_ldo1_ocp_auto_stop_END (5)
#define PMIC_LDO0_3_OCP_CTRL_ldo0_ocp_auto_stop_START (6)
#define PMIC_LDO0_3_OCP_CTRL_ldo0_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo8_ocp_auto_stop : 2;
        unsigned char ldo7_ocp_auto_stop : 2;
        unsigned char ldo5_ocp_auto_stop : 2;
        unsigned char ldo4_ocp_auto_stop : 2;
    } reg;
} PMIC_LDO4_8_OCP_CTRL_UNION;
#endif
#define PMIC_LDO4_8_OCP_CTRL_ldo8_ocp_auto_stop_START (0)
#define PMIC_LDO4_8_OCP_CTRL_ldo8_ocp_auto_stop_END (1)
#define PMIC_LDO4_8_OCP_CTRL_ldo7_ocp_auto_stop_START (2)
#define PMIC_LDO4_8_OCP_CTRL_ldo7_ocp_auto_stop_END (3)
#define PMIC_LDO4_8_OCP_CTRL_ldo5_ocp_auto_stop_START (4)
#define PMIC_LDO4_8_OCP_CTRL_ldo5_ocp_auto_stop_END (5)
#define PMIC_LDO4_8_OCP_CTRL_ldo4_ocp_auto_stop_START (6)
#define PMIC_LDO4_8_OCP_CTRL_ldo4_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo12_ocp_auto_stop : 2;
        unsigned char ldo11_ocp_auto_stop : 2;
        unsigned char ldo10_ocp_auto_stop : 2;
        unsigned char ldo9_ocp_auto_stop : 2;
    } reg;
} PMIC_LDO9_12_OCP_CTRL_UNION;
#endif
#define PMIC_LDO9_12_OCP_CTRL_ldo12_ocp_auto_stop_START (0)
#define PMIC_LDO9_12_OCP_CTRL_ldo12_ocp_auto_stop_END (1)
#define PMIC_LDO9_12_OCP_CTRL_ldo11_ocp_auto_stop_START (2)
#define PMIC_LDO9_12_OCP_CTRL_ldo11_ocp_auto_stop_END (3)
#define PMIC_LDO9_12_OCP_CTRL_ldo10_ocp_auto_stop_START (4)
#define PMIC_LDO9_12_OCP_CTRL_ldo10_ocp_auto_stop_END (5)
#define PMIC_LDO9_12_OCP_CTRL_ldo9_ocp_auto_stop_START (6)
#define PMIC_LDO9_12_OCP_CTRL_ldo9_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo16_ocp_auto_stop : 2;
        unsigned char ldo15_ocp_auto_stop : 2;
        unsigned char ldo14_ocp_auto_stop : 2;
        unsigned char ldo13_ocp_auto_stop : 2;
    } reg;
} PMIC_LDO13_16_OCP_CTRL_UNION;
#endif
#define PMIC_LDO13_16_OCP_CTRL_ldo16_ocp_auto_stop_START (0)
#define PMIC_LDO13_16_OCP_CTRL_ldo16_ocp_auto_stop_END (1)
#define PMIC_LDO13_16_OCP_CTRL_ldo15_ocp_auto_stop_START (2)
#define PMIC_LDO13_16_OCP_CTRL_ldo15_ocp_auto_stop_END (3)
#define PMIC_LDO13_16_OCP_CTRL_ldo14_ocp_auto_stop_START (4)
#define PMIC_LDO13_16_OCP_CTRL_ldo14_ocp_auto_stop_END (5)
#define PMIC_LDO13_16_OCP_CTRL_ldo13_ocp_auto_stop_START (6)
#define PMIC_LDO13_16_OCP_CTRL_ldo13_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo21_ocp_auto_stop : 2;
        unsigned char ldo20_ocp_auto_stop : 2;
        unsigned char ldo19_ocp_auto_stop : 2;
        unsigned char ldo17_ocp_auto_stop : 2;
    } reg;
} PMIC_LDO17_21_OCP_CTRL_UNION;
#endif
#define PMIC_LDO17_21_OCP_CTRL_ldo21_ocp_auto_stop_START (0)
#define PMIC_LDO17_21_OCP_CTRL_ldo21_ocp_auto_stop_END (1)
#define PMIC_LDO17_21_OCP_CTRL_ldo20_ocp_auto_stop_START (2)
#define PMIC_LDO17_21_OCP_CTRL_ldo20_ocp_auto_stop_END (3)
#define PMIC_LDO17_21_OCP_CTRL_ldo19_ocp_auto_stop_START (4)
#define PMIC_LDO17_21_OCP_CTRL_ldo19_ocp_auto_stop_END (5)
#define PMIC_LDO17_21_OCP_CTRL_ldo17_ocp_auto_stop_START (6)
#define PMIC_LDO17_21_OCP_CTRL_ldo17_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo25_ocp_auto_stop : 2;
        unsigned char ldo24_ocp_auto_stop : 2;
        unsigned char ldo23_ocp_auto_stop : 2;
        unsigned char ldo22_ocp_auto_stop : 2;
    } reg;
} PMIC_LDO22_25_OCP_CTRL_UNION;
#endif
#define PMIC_LDO22_25_OCP_CTRL_ldo25_ocp_auto_stop_START (0)
#define PMIC_LDO22_25_OCP_CTRL_ldo25_ocp_auto_stop_END (1)
#define PMIC_LDO22_25_OCP_CTRL_ldo24_ocp_auto_stop_START (2)
#define PMIC_LDO22_25_OCP_CTRL_ldo24_ocp_auto_stop_END (3)
#define PMIC_LDO22_25_OCP_CTRL_ldo23_ocp_auto_stop_START (4)
#define PMIC_LDO22_25_OCP_CTRL_ldo23_ocp_auto_stop_END (5)
#define PMIC_LDO22_25_OCP_CTRL_ldo22_ocp_auto_stop_START (6)
#define PMIC_LDO22_25_OCP_CTRL_ldo22_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo29_ocp_auto_stop : 2;
        unsigned char ldo28_ocp_auto_stop : 2;
        unsigned char ldo27_ocp_auto_stop : 2;
        unsigned char ldo26_ocp_auto_stop : 2;
    } reg;
} PMIC_LDO26_29_OCP_CTRL_UNION;
#endif
#define PMIC_LDO26_29_OCP_CTRL_ldo29_ocp_auto_stop_START (0)
#define PMIC_LDO26_29_OCP_CTRL_ldo29_ocp_auto_stop_END (1)
#define PMIC_LDO26_29_OCP_CTRL_ldo28_ocp_auto_stop_START (2)
#define PMIC_LDO26_29_OCP_CTRL_ldo28_ocp_auto_stop_END (3)
#define PMIC_LDO26_29_OCP_CTRL_ldo27_ocp_auto_stop_START (4)
#define PMIC_LDO26_29_OCP_CTRL_ldo27_ocp_auto_stop_END (5)
#define PMIC_LDO26_29_OCP_CTRL_ldo26_ocp_auto_stop_START (6)
#define PMIC_LDO26_29_OCP_CTRL_ldo26_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo34_ocp_auto_stop : 2;
        unsigned char ldo33_ocp_auto_stop : 2;
        unsigned char ldo32_ocp_auto_stop : 2;
        unsigned char ldo31_ocp_auto_stop : 2;
    } reg;
} PMIC_LDO31_34_OCP_CTRL_UNION;
#endif
#define PMIC_LDO31_34_OCP_CTRL_ldo34_ocp_auto_stop_START (0)
#define PMIC_LDO31_34_OCP_CTRL_ldo34_ocp_auto_stop_END (1)
#define PMIC_LDO31_34_OCP_CTRL_ldo33_ocp_auto_stop_START (2)
#define PMIC_LDO31_34_OCP_CTRL_ldo33_ocp_auto_stop_END (3)
#define PMIC_LDO31_34_OCP_CTRL_ldo32_ocp_auto_stop_START (4)
#define PMIC_LDO31_34_OCP_CTRL_ldo32_ocp_auto_stop_END (5)
#define PMIC_LDO31_34_OCP_CTRL_ldo31_ocp_auto_stop_START (6)
#define PMIC_LDO31_34_OCP_CTRL_ldo31_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char classd_ocp_auto_stop : 2;
        unsigned char xo_ldo_buff_scp_auto_stop : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO_BUFF_CLASS_SCP_CTRL_UNION;
#endif
#define PMIC_LDO_BUFF_CLASS_SCP_CTRL_classd_ocp_auto_stop_START (0)
#define PMIC_LDO_BUFF_CLASS_SCP_CTRL_classd_ocp_auto_stop_END (1)
#define PMIC_LDO_BUFF_CLASS_SCP_CTRL_xo_ldo_buff_scp_auto_stop_START (2)
#define PMIC_LDO_BUFF_CLASS_SCP_CTRL_xo_ldo_buff_scp_auto_stop_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_scp_auto_stop : 2;
        unsigned char buck2_scp_auto_stop : 2;
        unsigned char buck1_scp_auto_stop : 2;
        unsigned char buck0_scp_auto_stop : 2;
    } reg;
} PMIC_BUCK0_3_SCP_CTRL_UNION;
#endif
#define PMIC_BUCK0_3_SCP_CTRL_buck3_scp_auto_stop_START (0)
#define PMIC_BUCK0_3_SCP_CTRL_buck3_scp_auto_stop_END (1)
#define PMIC_BUCK0_3_SCP_CTRL_buck2_scp_auto_stop_START (2)
#define PMIC_BUCK0_3_SCP_CTRL_buck2_scp_auto_stop_END (3)
#define PMIC_BUCK0_3_SCP_CTRL_buck1_scp_auto_stop_START (4)
#define PMIC_BUCK0_3_SCP_CTRL_buck1_scp_auto_stop_END (5)
#define PMIC_BUCK0_3_SCP_CTRL_buck0_scp_auto_stop_START (6)
#define PMIC_BUCK0_3_SCP_CTRL_buck0_scp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char st_pmua_en : 1;
        unsigned char reserved_0 : 3;
        unsigned char reg_pmua_eco_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_LDO_PMUA_ECO_UNION;
#endif
#define PMIC_LDO_PMUA_ECO_st_pmua_en_START (0)
#define PMIC_LDO_PMUA_ECO_st_pmua_en_END (0)
#define PMIC_LDO_PMUA_ECO_reg_pmua_eco_en_START (4)
#define PMIC_LDO_PMUA_ECO_reg_pmua_eco_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pmua_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO_PMUA_VSET_UNION;
#endif
#define PMIC_LDO_PMUA_VSET_pmua_vset_START (0)
#define PMIC_LDO_PMUA_VSET_pmua_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_xo_abb_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_CLK_ABB_EN_UNION;
#endif
#define PMIC_CLK_ABB_EN_reg_xo_abb_en_START (0)
#define PMIC_CLK_ABB_EN_reg_xo_abb_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_xo_wifi_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_CLK_WIFI_EN_UNION;
#endif
#define PMIC_CLK_WIFI_EN_reg_xo_wifi_en_START (0)
#define PMIC_CLK_WIFI_EN_reg_xo_wifi_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_rf0_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_CLK_RF0_EN_UNION;
#endif
#define PMIC_CLK_RF0_EN_xo_rf0_en_START (0)
#define PMIC_CLK_RF0_EN_xo_rf0_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_rf1_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_CLK_RF1_EN_UNION;
#endif
#define PMIC_CLK_RF1_EN_xo_rf1_en_START (0)
#define PMIC_CLK_RF1_EN_xo_rf1_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_xo_sys_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_CLK_SYS_EN_UNION;
#endif
#define PMIC_CLK_SYS_EN_reg_xo_sys_en_START (0)
#define PMIC_CLK_SYS_EN_reg_xo_sys_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_xo_codec_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_CLK_CODEC_EN_UNION;
#endif
#define PMIC_CLK_CODEC_EN_reg_xo_codec_en_START (0)
#define PMIC_CLK_CODEC_EN_reg_xo_codec_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_abb_drv : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_CLK_ABB_CTRL_UNION;
#endif
#define PMIC_CLK_ABB_CTRL_xo_abb_drv_START (0)
#define PMIC_CLK_ABB_CTRL_xo_abb_drv_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_wifi_drv : 2;
        unsigned char np_xo_dig_wifi_sel : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_CLK_WIFI_CTRL_UNION;
#endif
#define PMIC_CLK_WIFI_CTRL_xo_wifi_drv_START (0)
#define PMIC_CLK_WIFI_CTRL_xo_wifi_drv_END (1)
#define PMIC_CLK_WIFI_CTRL_np_xo_dig_wifi_sel_START (2)
#define PMIC_CLK_WIFI_CTRL_np_xo_dig_wifi_sel_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_rf0_drv : 2;
        unsigned char np_xo_dig_rf0_sel : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_CLK_RF0_CTRL_UNION;
#endif
#define PMIC_CLK_RF0_CTRL_xo_rf0_drv_START (0)
#define PMIC_CLK_RF0_CTRL_xo_rf0_drv_END (1)
#define PMIC_CLK_RF0_CTRL_np_xo_dig_rf0_sel_START (2)
#define PMIC_CLK_RF0_CTRL_np_xo_dig_rf0_sel_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_rf1_drv : 2;
        unsigned char np_xo_dig_rf1_sel : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_CLK_RF1_CTRL_UNION;
#endif
#define PMIC_CLK_RF1_CTRL_xo_rf1_drv_START (0)
#define PMIC_CLK_RF1_CTRL_xo_rf1_drv_END (1)
#define PMIC_CLK_RF1_CTRL_np_xo_dig_rf1_sel_START (2)
#define PMIC_CLK_RF1_CTRL_np_xo_dig_rf1_sel_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_sys_drv : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_CLK_SYS_CTRL_UNION;
#endif
#define PMIC_CLK_SYS_CTRL_xo_sys_drv_START (0)
#define PMIC_CLK_SYS_CTRL_xo_sys_drv_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_codec_drv : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_CLK_CODEC_CTRL_UNION;
#endif
#define PMIC_CLK_CODEC_CTRL_xo_codec_drv_START (0)
#define PMIC_CLK_CODEC_CTRL_xo_codec_drv_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_ldo_buff_curr : 2;
        unsigned char reserved : 2;
        unsigned char np_xo_trim_c2fix : 4;
    } reg;
} PMIC_CLK_TOP_CTRL0_UNION;
#endif
#define PMIC_CLK_TOP_CTRL0_xo_ldo_buff_curr_START (0)
#define PMIC_CLK_TOP_CTRL0_xo_ldo_buff_curr_END (1)
#define PMIC_CLK_TOP_CTRL0_np_xo_trim_c2fix_START (4)
#define PMIC_CLK_TOP_CTRL0_np_xo_trim_c2fix_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_xo_trim_c1fix : 8;
    } reg;
} PMIC_CLK_TOP_CTRL1_UNION;
#endif
#define PMIC_CLK_TOP_CTRL1_np_xo_trim_c1fix_START (0)
#define PMIC_CLK_TOP_CTRL1_np_xo_trim_c1fix_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_delay_sel : 2;
        unsigned char xo_tri_cap : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_CLK_TOP_CTRL2_UNION;
#endif
#define PMIC_CLK_TOP_CTRL2_xo_delay_sel_START (0)
#define PMIC_CLK_TOP_CTRL2_xo_delay_sel_END (1)
#define PMIC_CLK_TOP_CTRL2_xo_tri_cap_START (2)
#define PMIC_CLK_TOP_CTRL2_xo_tri_cap_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_reserve : 8;
    } reg;
} PMIC_CLK_TOP_CTRL3_UNION;
#endif
#define PMIC_CLK_TOP_CTRL3_xo_reserve_START (0)
#define PMIC_CLK_TOP_CTRL3_xo_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_reg_256k_en0 : 8;
    } reg;
} PMIC_CLK_256K_CTRL0_UNION;
#endif
#define PMIC_CLK_256K_CTRL0_np_reg_256k_en0_START (0)
#define PMIC_CLK_256K_CTRL0_np_reg_256k_en0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_reg_256k_en1 : 8;
    } reg;
} PMIC_CLK_256K_CTRL1_UNION;
#endif
#define PMIC_CLK_256K_CTRL1_np_reg_256k_en1_START (0)
#define PMIC_CLK_256K_CTRL1_np_reg_256k_en1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char en_32k_sys : 1;
        unsigned char en_32k_bt : 1;
        unsigned char en_32k_gps : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_OSC32K_ONOFF_CTRL_UNION;
#endif
#define PMIC_OSC32K_ONOFF_CTRL_en_32k_sys_START (0)
#define PMIC_OSC32K_ONOFF_CTRL_en_32k_sys_END (0)
#define PMIC_OSC32K_ONOFF_CTRL_en_32k_bt_START (1)
#define PMIC_OSC32K_ONOFF_CTRL_en_32k_bt_END (1)
#define PMIC_OSC32K_ONOFF_CTRL_en_32k_gps_START (2)
#define PMIC_OSC32K_ONOFF_CTRL_en_32k_gps_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_eco_in_hd_mask : 1;
        unsigned char reg_xo_ldo_buff_hd_mask : 1;
        unsigned char reg_sys_clk_hd_mask : 1;
        unsigned char reg_abb_clk_hd_mask : 1;
        unsigned char reg_wifi_clk_hd_mask : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_HARDWIRE_CTRL0_UNION;
#endif
#define PMIC_HARDWIRE_CTRL0_reg_eco_in_hd_mask_START (0)
#define PMIC_HARDWIRE_CTRL0_reg_eco_in_hd_mask_END (0)
#define PMIC_HARDWIRE_CTRL0_reg_xo_ldo_buff_hd_mask_START (1)
#define PMIC_HARDWIRE_CTRL0_reg_xo_ldo_buff_hd_mask_END (1)
#define PMIC_HARDWIRE_CTRL0_reg_sys_clk_hd_mask_START (2)
#define PMIC_HARDWIRE_CTRL0_reg_sys_clk_hd_mask_END (2)
#define PMIC_HARDWIRE_CTRL0_reg_abb_clk_hd_mask_START (3)
#define PMIC_HARDWIRE_CTRL0_reg_abb_clk_hd_mask_END (3)
#define PMIC_HARDWIRE_CTRL0_reg_wifi_clk_hd_mask_START (4)
#define PMIC_HARDWIRE_CTRL0_reg_wifi_clk_hd_mask_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo27_hd_mask : 1;
        unsigned char reg_ldo26_hd_mask : 2;
        unsigned char reg_ldo13_hd_mask : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_HARDWIRE_CTRL1_UNION;
#endif
#define PMIC_HARDWIRE_CTRL1_reg_ldo27_hd_mask_START (0)
#define PMIC_HARDWIRE_CTRL1_reg_ldo27_hd_mask_END (0)
#define PMIC_HARDWIRE_CTRL1_reg_ldo26_hd_mask_START (1)
#define PMIC_HARDWIRE_CTRL1_reg_ldo26_hd_mask_END (2)
#define PMIC_HARDWIRE_CTRL1_reg_ldo13_hd_mask_START (3)
#define PMIC_HARDWIRE_CTRL1_reg_ldo13_hd_mask_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_ldo23_on : 1;
        unsigned char peri_en_ldo10_on : 1;
        unsigned char peri_en_ldo7_on : 1;
        unsigned char peri_en_ldo5_on : 1;
        unsigned char peri_en_buck0_on : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL0_UNION;
#endif
#define PMIC_PERI_CTRL0_peri_en_ldo23_on_START (0)
#define PMIC_PERI_CTRL0_peri_en_ldo23_on_END (0)
#define PMIC_PERI_CTRL0_peri_en_ldo10_on_START (1)
#define PMIC_PERI_CTRL0_peri_en_ldo10_on_END (1)
#define PMIC_PERI_CTRL0_peri_en_ldo7_on_START (2)
#define PMIC_PERI_CTRL0_peri_en_ldo7_on_END (2)
#define PMIC_PERI_CTRL0_peri_en_ldo5_on_START (3)
#define PMIC_PERI_CTRL0_peri_en_ldo5_on_END (3)
#define PMIC_PERI_CTRL0_peri_en_buck0_on_START (4)
#define PMIC_PERI_CTRL0_peri_en_buck0_on_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_ldo8_eco : 1;
        unsigned char peri_en_ldo4_eco : 1;
        unsigned char peri_en_ldo2_eco : 1;
        unsigned char peri_en_ldo0_2_eco : 1;
        unsigned char peri_en_buck3_eco : 1;
        unsigned char peri_en_buck2_eco : 1;
        unsigned char peri_en_buck1_eco : 1;
        unsigned char peri_en_buck0_eco : 1;
    } reg;
} PMIC_PERI_CTRL1_UNION;
#endif
#define PMIC_PERI_CTRL1_peri_en_ldo8_eco_START (0)
#define PMIC_PERI_CTRL1_peri_en_ldo8_eco_END (0)
#define PMIC_PERI_CTRL1_peri_en_ldo4_eco_START (1)
#define PMIC_PERI_CTRL1_peri_en_ldo4_eco_END (1)
#define PMIC_PERI_CTRL1_peri_en_ldo2_eco_START (2)
#define PMIC_PERI_CTRL1_peri_en_ldo2_eco_END (2)
#define PMIC_PERI_CTRL1_peri_en_ldo0_2_eco_START (3)
#define PMIC_PERI_CTRL1_peri_en_ldo0_2_eco_END (3)
#define PMIC_PERI_CTRL1_peri_en_buck3_eco_START (4)
#define PMIC_PERI_CTRL1_peri_en_buck3_eco_END (4)
#define PMIC_PERI_CTRL1_peri_en_buck2_eco_START (5)
#define PMIC_PERI_CTRL1_peri_en_buck2_eco_END (5)
#define PMIC_PERI_CTRL1_peri_en_buck1_eco_START (6)
#define PMIC_PERI_CTRL1_peri_en_buck1_eco_END (6)
#define PMIC_PERI_CTRL1_peri_en_buck0_eco_START (7)
#define PMIC_PERI_CTRL1_peri_en_buck0_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_ldo24_eco : 1;
        unsigned char peri_en_ldo17_eco : 1;
        unsigned char peri_en_ldo16_eco : 1;
        unsigned char peri_en_ldo15_eco : 1;
        unsigned char peri_en_ldo12_eco : 1;
        unsigned char peri_en_ldo11_eco : 1;
        unsigned char peri_en_ldo10_eco : 1;
        unsigned char peri_en_ldo9_eco : 1;
    } reg;
} PMIC_PERI_CTRL2_UNION;
#endif
#define PMIC_PERI_CTRL2_peri_en_ldo24_eco_START (0)
#define PMIC_PERI_CTRL2_peri_en_ldo24_eco_END (0)
#define PMIC_PERI_CTRL2_peri_en_ldo17_eco_START (1)
#define PMIC_PERI_CTRL2_peri_en_ldo17_eco_END (1)
#define PMIC_PERI_CTRL2_peri_en_ldo16_eco_START (2)
#define PMIC_PERI_CTRL2_peri_en_ldo16_eco_END (2)
#define PMIC_PERI_CTRL2_peri_en_ldo15_eco_START (3)
#define PMIC_PERI_CTRL2_peri_en_ldo15_eco_END (3)
#define PMIC_PERI_CTRL2_peri_en_ldo12_eco_START (4)
#define PMIC_PERI_CTRL2_peri_en_ldo12_eco_END (4)
#define PMIC_PERI_CTRL2_peri_en_ldo11_eco_START (5)
#define PMIC_PERI_CTRL2_peri_en_ldo11_eco_END (5)
#define PMIC_PERI_CTRL2_peri_en_ldo10_eco_START (6)
#define PMIC_PERI_CTRL2_peri_en_ldo10_eco_END (6)
#define PMIC_PERI_CTRL2_peri_en_ldo9_eco_START (7)
#define PMIC_PERI_CTRL2_peri_en_ldo9_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_thsd_eco : 1;
        unsigned char peri_en_pmua_eco : 1;
        unsigned char peri_en_ldo34_eco : 1;
        unsigned char peri_en_ldo29_eco : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_PERI_CTRL3_UNION;
#endif
#define PMIC_PERI_CTRL3_peri_en_thsd_eco_START (0)
#define PMIC_PERI_CTRL3_peri_en_thsd_eco_END (0)
#define PMIC_PERI_CTRL3_peri_en_pmua_eco_START (1)
#define PMIC_PERI_CTRL3_peri_en_pmua_eco_END (1)
#define PMIC_PERI_CTRL3_peri_en_ldo34_eco_START (2)
#define PMIC_PERI_CTRL3_peri_en_ldo34_eco_END (2)
#define PMIC_PERI_CTRL3_peri_en_ldo29_eco_START (3)
#define PMIC_PERI_CTRL3_peri_en_ldo29_eco_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_buck3_vset : 1;
        unsigned char peri_en_buck2_vset : 1;
        unsigned char peri_en_buck1_vset : 1;
        unsigned char peri_en_ldo29_vset : 1;
        unsigned char peri_en_ldo2_vset : 1;
        unsigned char peri_en_ldo0_2_vset : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_PERI_VSET_CTRL_UNION;
#endif
#define PMIC_PERI_VSET_CTRL_peri_en_buck3_vset_START (0)
#define PMIC_PERI_VSET_CTRL_peri_en_buck3_vset_END (0)
#define PMIC_PERI_VSET_CTRL_peri_en_buck2_vset_START (1)
#define PMIC_PERI_VSET_CTRL_peri_en_buck2_vset_END (1)
#define PMIC_PERI_VSET_CTRL_peri_en_buck1_vset_START (2)
#define PMIC_PERI_VSET_CTRL_peri_en_buck1_vset_END (2)
#define PMIC_PERI_VSET_CTRL_peri_en_ldo29_vset_START (3)
#define PMIC_PERI_VSET_CTRL_peri_en_ldo29_vset_END (3)
#define PMIC_PERI_VSET_CTRL_peri_en_ldo2_vset_START (4)
#define PMIC_PERI_VSET_CTRL_peri_en_ldo2_vset_END (4)
#define PMIC_PERI_VSET_CTRL_peri_en_ldo0_2_vset_START (5)
#define PMIC_PERI_VSET_CTRL_peri_en_ldo0_2_vset_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_time_sel : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_PERI_TIME__CTRL_UNION;
#endif
#define PMIC_PERI_TIME__CTRL_peri_time_sel_START (0)
#define PMIC_PERI_TIME__CTRL_peri_time_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char coul_codec_clk_en_mask : 1;
        unsigned char coul_wifi_clk_en_mask : 1;
        unsigned char coul_sys_clk_en_mask : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_COUL_ECO_MASK_UNION;
#endif
#define PMIC_COUL_ECO_MASK_coul_codec_clk_en_mask_START (0)
#define PMIC_COUL_ECO_MASK_coul_codec_clk_en_mask_END (0)
#define PMIC_COUL_ECO_MASK_coul_wifi_clk_en_mask_START (1)
#define PMIC_COUL_ECO_MASK_coul_wifi_clk_en_mask_END (1)
#define PMIC_COUL_ECO_MASK_coul_sys_clk_en_mask_START (2)
#define PMIC_COUL_ECO_MASK_coul_sys_clk_en_mask_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_0 : 8;
    } reg;
} PMIC_IRQ_MASK_0_UNION;
#endif
#define PMIC_IRQ_MASK_0_irq_mask_0_START (0)
#define PMIC_IRQ_MASK_0_irq_mask_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_1 : 8;
    } reg;
} PMIC_IRQ_MASK_1_UNION;
#endif
#define PMIC_IRQ_MASK_1_irq_mask_1_START (0)
#define PMIC_IRQ_MASK_1_irq_mask_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_2 : 8;
    } reg;
} PMIC_IRQ_MASK_2_UNION;
#endif
#define PMIC_IRQ_MASK_2_irq_mask_2_START (0)
#define PMIC_IRQ_MASK_2_irq_mask_2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_3 : 8;
    } reg;
} PMIC_IRQ_MASK_3_UNION;
#endif
#define PMIC_IRQ_MASK_3_irq_mask_3_START (0)
#define PMIC_IRQ_MASK_3_irq_mask_3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_4 : 8;
    } reg;
} PMIC_IRQ_MASK_4_UNION;
#endif
#define PMIC_IRQ_MASK_4_irq_mask_4_START (0)
#define PMIC_IRQ_MASK_4_irq_mask_4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_5 : 8;
    } reg;
} PMIC_IRQ_MASK_5_UNION;
#endif
#define PMIC_IRQ_MASK_5_irq_mask_5_START (0)
#define PMIC_IRQ_MASK_5_irq_mask_5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_6 : 8;
    } reg;
} PMIC_IRQ_MASK_6_UNION;
#endif
#define PMIC_IRQ_MASK_6_irq_mask_6_START (0)
#define PMIC_IRQ_MASK_6_irq_mask_6_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_7 : 8;
    } reg;
} PMIC_IRQ_MASK_7_UNION;
#endif
#define PMIC_IRQ_MASK_7_irq_mask_7_START (0)
#define PMIC_IRQ_MASK_7_irq_mask_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_chg_en : 1;
        unsigned char np_chg_bypass : 1;
        unsigned char np_chg_vset : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_BACKUP_CHG_UNION;
#endif
#define PMIC_BACKUP_CHG_np_chg_en_START (0)
#define PMIC_BACKUP_CHG_np_chg_en_END (0)
#define PMIC_BACKUP_CHG_np_chg_bypass_START (1)
#define PMIC_BACKUP_CHG_np_chg_bypass_END (1)
#define PMIC_BACKUP_CHG_np_chg_vset_START (2)
#define PMIC_BACKUP_CHG_np_chg_vset_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_classd_en : 1;
        unsigned char classd_mute : 1;
        unsigned char classd_mute_sel : 1;
        unsigned char classd_drv_en : 1;
        unsigned char classd_i_ocp : 2;
        unsigned char classd_gain : 2;
    } reg;
} PMIC_CLASSD_CTRL0_UNION;
#endif
#define PMIC_CLASSD_CTRL0_reg_classd_en_START (0)
#define PMIC_CLASSD_CTRL0_reg_classd_en_END (0)
#define PMIC_CLASSD_CTRL0_classd_mute_START (1)
#define PMIC_CLASSD_CTRL0_classd_mute_END (1)
#define PMIC_CLASSD_CTRL0_classd_mute_sel_START (2)
#define PMIC_CLASSD_CTRL0_classd_mute_sel_END (2)
#define PMIC_CLASSD_CTRL0_classd_drv_en_START (3)
#define PMIC_CLASSD_CTRL0_classd_drv_en_END (3)
#define PMIC_CLASSD_CTRL0_classd_i_ocp_START (4)
#define PMIC_CLASSD_CTRL0_classd_i_ocp_END (5)
#define PMIC_CLASSD_CTRL0_classd_gain_START (6)
#define PMIC_CLASSD_CTRL0_classd_gain_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char classd_i_pump : 2;
        unsigned char classd_p_sel : 2;
        unsigned char classd_n_sel : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_CLASSD_CTRL1_UNION;
#endif
#define PMIC_CLASSD_CTRL1_classd_i_pump_START (0)
#define PMIC_CLASSD_CTRL1_classd_i_pump_END (1)
#define PMIC_CLASSD_CTRL1_classd_p_sel_START (2)
#define PMIC_CLASSD_CTRL1_classd_p_sel_END (3)
#define PMIC_CLASSD_CTRL1_classd_n_sel_START (4)
#define PMIC_CLASSD_CTRL1_classd_n_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char classd_pls_byp : 1;
        unsigned char classd_dt_sel : 1;
        unsigned char classd_ocp_bps : 1;
        unsigned char reserved : 1;
        unsigned char classd_reserve0 : 4;
    } reg;
} PMIC_CLASSD_CTRL2_UNION;
#endif
#define PMIC_CLASSD_CTRL2_classd_pls_byp_START (0)
#define PMIC_CLASSD_CTRL2_classd_pls_byp_END (0)
#define PMIC_CLASSD_CTRL2_classd_dt_sel_START (1)
#define PMIC_CLASSD_CTRL2_classd_dt_sel_END (1)
#define PMIC_CLASSD_CTRL2_classd_ocp_bps_START (2)
#define PMIC_CLASSD_CTRL2_classd_ocp_bps_END (2)
#define PMIC_CLASSD_CTRL2_classd_reserve0_START (4)
#define PMIC_CLASSD_CTRL2_classd_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char classd_reserve1 : 8;
    } reg;
} PMIC_CLASSD_CTRL3_UNION;
#endif
#define PMIC_CLASSD_CTRL3_classd_reserve1_START (0)
#define PMIC_CLASSD_CTRL3_classd_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ref_res_sel_int : 1;
        unsigned char ref_ibias_trim_en : 2;
        unsigned char reserved : 1;
        unsigned char reg_thsd_en : 1;
        unsigned char reg_thsd_eco_en : 1;
        unsigned char thsd_tmp_set : 2;
    } reg;
} PMIC_TH_CTRL_UNION;
#endif
#define PMIC_TH_CTRL_ref_res_sel_int_START (0)
#define PMIC_TH_CTRL_ref_res_sel_int_END (0)
#define PMIC_TH_CTRL_ref_ibias_trim_en_START (1)
#define PMIC_TH_CTRL_ref_ibias_trim_en_END (2)
#define PMIC_TH_CTRL_reg_thsd_en_START (4)
#define PMIC_TH_CTRL_reg_thsd_en_END (4)
#define PMIC_TH_CTRL_reg_thsd_eco_en_START (5)
#define PMIC_TH_CTRL_reg_thsd_eco_en_END (5)
#define PMIC_TH_CTRL_thsd_tmp_set_START (6)
#define PMIC_TH_CTRL_thsd_tmp_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ref_reserve : 8;
    } reg;
} PMIC_BG_TEST_UNION;
#endif
#define PMIC_BG_TEST_ref_reserve_START (0)
#define PMIC_BG_TEST_ref_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char en_dr3_int : 1;
        unsigned char dr3_mode : 1;
        unsigned char en_dr4_int : 1;
        unsigned char dr4_mode : 1;
        unsigned char en_dr5_int : 1;
        unsigned char dr5_mode : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_DR_EN_MODE_345_UNION;
#endif
#define PMIC_DR_EN_MODE_345_en_dr3_int_START (0)
#define PMIC_DR_EN_MODE_345_en_dr3_int_END (0)
#define PMIC_DR_EN_MODE_345_dr3_mode_START (1)
#define PMIC_DR_EN_MODE_345_dr3_mode_END (1)
#define PMIC_DR_EN_MODE_345_en_dr4_int_START (2)
#define PMIC_DR_EN_MODE_345_en_dr4_int_END (2)
#define PMIC_DR_EN_MODE_345_dr4_mode_START (3)
#define PMIC_DR_EN_MODE_345_dr4_mode_END (3)
#define PMIC_DR_EN_MODE_345_en_dr5_int_START (4)
#define PMIC_DR_EN_MODE_345_en_dr5_int_END (4)
#define PMIC_DR_EN_MODE_345_dr5_mode_START (5)
#define PMIC_DR_EN_MODE_345_dr5_mode_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char en_dr1_int : 1;
        unsigned char dr1_mode : 1;
        unsigned char en_dr2_int : 1;
        unsigned char dr2_mode : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_DR_EN_MODE_12_UNION;
#endif
#define PMIC_DR_EN_MODE_12_en_dr1_int_START (0)
#define PMIC_DR_EN_MODE_12_en_dr1_int_END (0)
#define PMIC_DR_EN_MODE_12_dr1_mode_START (1)
#define PMIC_DR_EN_MODE_12_dr1_mode_END (1)
#define PMIC_DR_EN_MODE_12_en_dr2_int_START (2)
#define PMIC_DR_EN_MODE_12_en_dr2_int_END (2)
#define PMIC_DR_EN_MODE_12_dr2_mode_START (3)
#define PMIC_DR_EN_MODE_12_dr2_mode_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_period_dr12 : 8;
    } reg;
} PMIC_FLASH_PERIOD_DR12_UNION;
#endif
#define PMIC_FLASH_PERIOD_DR12_flash_period_dr12_START (0)
#define PMIC_FLASH_PERIOD_DR12_flash_period_dr12_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_on_dr12 : 8;
    } reg;
} PMIC_FLASH_ON_DR12_UNION;
#endif
#define PMIC_FLASH_ON_DR12_flash_on_dr12_START (0)
#define PMIC_FLASH_ON_DR12_flash_on_dr12_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_period_dr345 : 8;
    } reg;
} PMIC_FLASH_PERIOD_DR345_UNION;
#endif
#define PMIC_FLASH_PERIOD_DR345_flash_period_dr345_START (0)
#define PMIC_FLASH_PERIOD_DR345_flash_period_dr345_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_on_dr345 : 8;
    } reg;
} PMIC_FLASH_ON_DR345_UNION;
#endif
#define PMIC_FLASH_ON_DR345_flash_on_dr345_START (0)
#define PMIC_FLASH_ON_DR345_flash_on_dr345_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr1_mode_sel : 1;
        unsigned char dr2_mode_sel : 1;
        unsigned char dr3_mode_sel : 1;
        unsigned char dr4_mode_sel : 1;
        unsigned char dr5_mode_sel : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_DR_MODE_SEL_UNION;
#endif
#define PMIC_DR_MODE_SEL_dr1_mode_sel_START (0)
#define PMIC_DR_MODE_SEL_dr1_mode_sel_END (0)
#define PMIC_DR_MODE_SEL_dr2_mode_sel_START (1)
#define PMIC_DR_MODE_SEL_dr2_mode_sel_END (1)
#define PMIC_DR_MODE_SEL_dr3_mode_sel_START (2)
#define PMIC_DR_MODE_SEL_dr3_mode_sel_END (2)
#define PMIC_DR_MODE_SEL_dr4_mode_sel_START (3)
#define PMIC_DR_MODE_SEL_dr4_mode_sel_END (3)
#define PMIC_DR_MODE_SEL_dr5_mode_sel_START (4)
#define PMIC_DR_MODE_SEL_dr5_mode_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_dr1_en : 1;
        unsigned char dr1_flash_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_dr2_en : 1;
        unsigned char dr2_flash_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_DR_BRE_CTRL_UNION;
#endif
#define PMIC_DR_BRE_CTRL_reg_dr1_en_START (0)
#define PMIC_DR_BRE_CTRL_reg_dr1_en_END (0)
#define PMIC_DR_BRE_CTRL_dr1_flash_en_START (1)
#define PMIC_DR_BRE_CTRL_dr1_flash_en_END (1)
#define PMIC_DR_BRE_CTRL_reg_dr2_en_START (4)
#define PMIC_DR_BRE_CTRL_reg_dr2_en_END (4)
#define PMIC_DR_BRE_CTRL_dr2_flash_en_START (5)
#define PMIC_DR_BRE_CTRL_dr2_flash_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr12_t_off : 3;
        unsigned char reserved_0 : 1;
        unsigned char dr12_t_on : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_DR12_TIM_CONF0_UNION;
#endif
#define PMIC_DR12_TIM_CONF0_dr12_t_off_START (0)
#define PMIC_DR12_TIM_CONF0_dr12_t_off_END (2)
#define PMIC_DR12_TIM_CONF0_dr12_t_on_START (4)
#define PMIC_DR12_TIM_CONF0_dr12_t_on_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr12_t_rise : 3;
        unsigned char reserved_0 : 1;
        unsigned char dr12_t_fall : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_DR12_TIM_CONF1_UNION;
#endif
#define PMIC_DR12_TIM_CONF1_dr12_t_rise_START (0)
#define PMIC_DR12_TIM_CONF1_dr12_t_rise_END (2)
#define PMIC_DR12_TIM_CONF1_dr12_t_fall_START (4)
#define PMIC_DR12_TIM_CONF1_dr12_t_fall_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr1_iset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_DR1_ISET_UNION;
#endif
#define PMIC_DR1_ISET_dr1_iset_START (0)
#define PMIC_DR1_ISET_dr1_iset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr2_iset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_DR2_ISET_UNION;
#endif
#define PMIC_DR2_ISET_dr2_iset_START (0)
#define PMIC_DR2_ISET_dr2_iset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_dr3_en : 1;
        unsigned char reg_dr4_en : 1;
        unsigned char reg_dr5_en : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_DR_LED_CTRL_UNION;
#endif
#define PMIC_DR_LED_CTRL_reg_dr3_en_START (0)
#define PMIC_DR_LED_CTRL_reg_dr3_en_END (0)
#define PMIC_DR_LED_CTRL_reg_dr4_en_START (1)
#define PMIC_DR_LED_CTRL_reg_dr4_en_END (1)
#define PMIC_DR_LED_CTRL_reg_dr5_en_START (2)
#define PMIC_DR_LED_CTRL_reg_dr5_en_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr3_out_ctrl : 2;
        unsigned char dr4_out_ctrl : 2;
        unsigned char dr5_out_ctrl : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_DR_OUT_CTRL_UNION;
#endif
#define PMIC_DR_OUT_CTRL_dr3_out_ctrl_START (0)
#define PMIC_DR_OUT_CTRL_dr3_out_ctrl_END (1)
#define PMIC_DR_OUT_CTRL_dr4_out_ctrl_START (2)
#define PMIC_DR_OUT_CTRL_dr4_out_ctrl_END (3)
#define PMIC_DR_OUT_CTRL_dr5_out_ctrl_START (4)
#define PMIC_DR_OUT_CTRL_dr5_out_ctrl_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr3_iset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_DR3_ISET_UNION;
#endif
#define PMIC_DR3_ISET_dr3_iset_START (0)
#define PMIC_DR3_ISET_dr3_iset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr3_start_delay : 8;
    } reg;
} PMIC_DR3_START_DEL_UNION;
#endif
#define PMIC_DR3_START_DEL_dr3_start_delay_START (0)
#define PMIC_DR3_START_DEL_dr3_start_delay_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr4_iset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_DR4_ISET_UNION;
#endif
#define PMIC_DR4_ISET_dr4_iset_START (0)
#define PMIC_DR4_ISET_dr4_iset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr4_start_delay : 8;
    } reg;
} PMIC_DR4_START_DEL_UNION;
#endif
#define PMIC_DR4_START_DEL_dr4_start_delay_START (0)
#define PMIC_DR4_START_DEL_dr4_start_delay_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr5_iset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_DR5_ISET_UNION;
#endif
#define PMIC_DR5_ISET_dr5_iset_START (0)
#define PMIC_DR5_ISET_dr5_iset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr5_start_delay : 8;
    } reg;
} PMIC_DR5_START_DEL_UNION;
#endif
#define PMIC_DR5_START_DEL_dr5_start_delay_START (0)
#define PMIC_DR5_START_DEL_dr5_start_delay_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr345_t_off : 4;
        unsigned char dr345_t_on : 4;
    } reg;
} PMIC_DR334_TIM_CONF0_UNION;
#endif
#define PMIC_DR334_TIM_CONF0_dr345_t_off_START (0)
#define PMIC_DR334_TIM_CONF0_dr345_t_off_END (3)
#define PMIC_DR334_TIM_CONF0_dr345_t_on_START (4)
#define PMIC_DR334_TIM_CONF0_dr345_t_on_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr345_t_rise : 3;
        unsigned char reserved_0 : 1;
        unsigned char dr345_t_fall : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_DR345_TIM_CONF1_UNION;
#endif
#define PMIC_DR345_TIM_CONF1_dr345_t_rise_START (0)
#define PMIC_DR345_TIM_CONF1_dr345_t_rise_END (2)
#define PMIC_DR345_TIM_CONF1_dr345_t_fall_START (4)
#define PMIC_DR345_TIM_CONF1_dr345_t_fall_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_vsys_pwroff_deb_set : 3;
        unsigned char np_vsys_pwroff_abs_set : 1;
        unsigned char np_vsys_pwron_set : 3;
        unsigned char reserved : 1;
    } reg;
} PMIC_VSYS_LOW_SET_UNION;
#endif
#define PMIC_VSYS_LOW_SET_np_vsys_pwroff_deb_set_START (0)
#define PMIC_VSYS_LOW_SET_np_vsys_pwroff_deb_set_END (2)
#define PMIC_VSYS_LOW_SET_np_vsys_pwroff_abs_set_START (3)
#define PMIC_VSYS_LOW_SET_np_vsys_pwroff_abs_set_END (3)
#define PMIC_VSYS_LOW_SET_np_vsys_pwron_set_START (4)
#define PMIC_VSYS_LOW_SET_np_vsys_pwron_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_nfc_on_d2a : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_NFC_ON_CTRL_UNION;
#endif
#define PMIC_NFC_ON_CTRL_np_nfc_on_d2a_START (0)
#define PMIC_NFC_ON_CTRL_np_nfc_on_d2a_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sys_ctrl_reserve : 8;
    } reg;
} PMIC_SYS_CTRL_RESERVE_UNION;
#endif
#define PMIC_SYS_CTRL_RESERVE_sys_ctrl_reserve_START (0)
#define PMIC_SYS_CTRL_RESERVE_sys_ctrl_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hreset_mode : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_HRESET_PWRDOWN_CTRL_UNION;
#endif
#define PMIC_HRESET_PWRDOWN_CTRL_hreset_mode_START (0)
#define PMIC_HRESET_PWRDOWN_CTRL_hreset_mode_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo31_ocp_deb_sel : 2;
        unsigned char ldo_ocp_deb_sel : 2;
        unsigned char buck_scp_deb_sel : 2;
        unsigned char buck_ocp_deb_sel : 2;
    } reg;
} PMIC_OCP_DEB_CTRL0_UNION;
#endif
#define PMIC_OCP_DEB_CTRL0_ldo31_ocp_deb_sel_START (0)
#define PMIC_OCP_DEB_CTRL0_ldo31_ocp_deb_sel_END (1)
#define PMIC_OCP_DEB_CTRL0_ldo_ocp_deb_sel_START (2)
#define PMIC_OCP_DEB_CTRL0_ldo_ocp_deb_sel_END (3)
#define PMIC_OCP_DEB_CTRL0_buck_scp_deb_sel_START (4)
#define PMIC_OCP_DEB_CTRL0_buck_scp_deb_sel_END (5)
#define PMIC_OCP_DEB_CTRL0_buck_ocp_deb_sel_START (6)
#define PMIC_OCP_DEB_CTRL0_buck_ocp_deb_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vin_ldoh_deb_sel : 2;
        unsigned char xo_ldo_buff_scp_deb_sel : 2;
        unsigned char classd_ocp_deb_sel : 2;
        unsigned char ldo27_ocp_deb_sel : 2;
    } reg;
} PMIC_OCP_DEB_CTRL1_UNION;
#endif
#define PMIC_OCP_DEB_CTRL1_vin_ldoh_deb_sel_START (0)
#define PMIC_OCP_DEB_CTRL1_vin_ldoh_deb_sel_END (1)
#define PMIC_OCP_DEB_CTRL1_xo_ldo_buff_scp_deb_sel_START (2)
#define PMIC_OCP_DEB_CTRL1_xo_ldo_buff_scp_deb_sel_END (3)
#define PMIC_OCP_DEB_CTRL1_classd_ocp_deb_sel_START (4)
#define PMIC_OCP_DEB_CTRL1_classd_ocp_deb_sel_END (5)
#define PMIC_OCP_DEB_CTRL1_ldo27_ocp_deb_sel_START (6)
#define PMIC_OCP_DEB_CTRL1_ldo27_ocp_deb_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char en_vin_ldoh_deb : 1;
        unsigned char en_xo_ldo_buff_scp_deb : 1;
        unsigned char en_classd_ocp_deb : 1;
        unsigned char en_ldo31_ocp_deb : 1;
        unsigned char en_ldo_ocp_deb : 1;
        unsigned char en_buck_scp_deb : 1;
        unsigned char en_buck_ocp_deb : 1;
        unsigned char en_ldo27_ocp_deb : 1;
    } reg;
} PMIC_OCP_SCP_ONOFF_UNION;
#endif
#define PMIC_OCP_SCP_ONOFF_en_vin_ldoh_deb_START (0)
#define PMIC_OCP_SCP_ONOFF_en_vin_ldoh_deb_END (0)
#define PMIC_OCP_SCP_ONOFF_en_xo_ldo_buff_scp_deb_START (1)
#define PMIC_OCP_SCP_ONOFF_en_xo_ldo_buff_scp_deb_END (1)
#define PMIC_OCP_SCP_ONOFF_en_classd_ocp_deb_START (2)
#define PMIC_OCP_SCP_ONOFF_en_classd_ocp_deb_END (2)
#define PMIC_OCP_SCP_ONOFF_en_ldo31_ocp_deb_START (3)
#define PMIC_OCP_SCP_ONOFF_en_ldo31_ocp_deb_END (3)
#define PMIC_OCP_SCP_ONOFF_en_ldo_ocp_deb_START (4)
#define PMIC_OCP_SCP_ONOFF_en_ldo_ocp_deb_END (4)
#define PMIC_OCP_SCP_ONOFF_en_buck_scp_deb_START (5)
#define PMIC_OCP_SCP_ONOFF_en_buck_scp_deb_END (5)
#define PMIC_OCP_SCP_ONOFF_en_buck_ocp_deb_START (6)
#define PMIC_OCP_SCP_ONOFF_en_buck_ocp_deb_END (6)
#define PMIC_OCP_SCP_ONOFF_en_ldo27_ocp_deb_START (7)
#define PMIC_OCP_SCP_ONOFF_en_ldo27_ocp_deb_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vsys_pwroff_deb_sel : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_PWROFF_DEB_CTRL_UNION;
#endif
#define PMIC_PWROFF_DEB_CTRL_vsys_pwroff_deb_sel_START (0)
#define PMIC_PWROFF_DEB_CTRL_vsys_pwroff_deb_sel_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vsys_pwroff_abs_pd_mask : 1;
        unsigned char vsys_pwroff_deb_pd_mask : 1;
        unsigned char thsd_otmp140_pd_mask : 1;
        unsigned char vsys_ov_pd_mask : 1;
        unsigned char vin_ldoh_vld_pd_mask : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_SYS_CTRL0_UNION;
#endif
#define PMIC_SYS_CTRL0_vsys_pwroff_abs_pd_mask_START (0)
#define PMIC_SYS_CTRL0_vsys_pwroff_abs_pd_mask_END (0)
#define PMIC_SYS_CTRL0_vsys_pwroff_deb_pd_mask_START (1)
#define PMIC_SYS_CTRL0_vsys_pwroff_deb_pd_mask_END (1)
#define PMIC_SYS_CTRL0_thsd_otmp140_pd_mask_START (2)
#define PMIC_SYS_CTRL0_thsd_otmp140_pd_mask_END (2)
#define PMIC_SYS_CTRL0_vsys_ov_pd_mask_START (3)
#define PMIC_SYS_CTRL0_vsys_ov_pd_mask_END (3)
#define PMIC_SYS_CTRL0_vin_ldoh_vld_pd_mask_START (4)
#define PMIC_SYS_CTRL0_vin_ldoh_vld_pd_mask_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_smpl_open_en : 1;
        unsigned char np_smpl_time_sel : 2;
        unsigned char reserved : 5;
    } reg;
} PMIC_SMPL_CTRL_UNION;
#endif
#define PMIC_SMPL_CTRL_np_smpl_open_en_START (0)
#define PMIC_SMPL_CTRL_np_smpl_open_en_END (0)
#define PMIC_SMPL_CTRL_np_smpl_time_sel_START (1)
#define PMIC_SMPL_CTRL_np_smpl_time_sel_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_pwron_8s_sel : 1;
        unsigned char np_pwron_time_sel : 2;
        unsigned char reserved : 5;
    } reg;
} PMIC_SYS_CTRL1_UNION;
#endif
#define PMIC_SYS_CTRL1_np_pwron_8s_sel_START (0)
#define PMIC_SYS_CTRL1_np_pwron_8s_sel_END (0)
#define PMIC_SYS_CTRL1_np_pwron_time_sel_START (1)
#define PMIC_SYS_CTRL1_np_pwron_time_sel_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sim0_hpd_r_pd_en : 1;
        unsigned char sim0_hpd_f_pd_en : 1;
        unsigned char sim1_hpd_r_pd_en : 1;
        unsigned char sim1_hpd_f_pd_en : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_SIM_CTRL_UNION;
#endif
#define PMIC_SIM_CTRL_sim0_hpd_r_pd_en_START (0)
#define PMIC_SIM_CTRL_sim0_hpd_r_pd_en_END (0)
#define PMIC_SIM_CTRL_sim0_hpd_f_pd_en_START (1)
#define PMIC_SIM_CTRL_sim0_hpd_f_pd_en_END (1)
#define PMIC_SIM_CTRL_sim1_hpd_r_pd_en_START (2)
#define PMIC_SIM_CTRL_sim1_hpd_r_pd_en_END (2)
#define PMIC_SIM_CTRL_sim1_hpd_f_pd_en_START (3)
#define PMIC_SIM_CTRL_sim1_hpd_f_pd_en_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sim_hpd_deb_sel : 5;
        unsigned char sim_del_sel : 3;
    } reg;
} PMIC_SIM_DEB_CTRL_UNION;
#endif
#define PMIC_SIM_DEB_CTRL_sim_hpd_deb_sel_START (0)
#define PMIC_SIM_DEB_CTRL_sim_hpd_deb_sel_END (4)
#define PMIC_SIM_DEB_CTRL_sim_del_sel_START (5)
#define PMIC_SIM_DEB_CTRL_sim_del_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char aux0_offset_cfg : 2;
        unsigned char aux0_ibias_cfg : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_AUX0_IBIAS_CFG_UNION;
#endif
#define PMIC_AUX0_IBIAS_CFG_aux0_offset_cfg_START (0)
#define PMIC_AUX0_IBIAS_CFG_aux0_offset_cfg_END (1)
#define PMIC_AUX0_IBIAS_CFG_aux0_ibias_cfg_START (2)
#define PMIC_AUX0_IBIAS_CFG_aux0_ibias_cfg_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char aux1_offset_cfg : 2;
        unsigned char aux1_ibias_cfg : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_AUX1_IBIAS_CFG_UNION;
#endif
#define PMIC_AUX1_IBIAS_CFG_aux1_offset_cfg_START (0)
#define PMIC_AUX1_IBIAS_CFG_aux1_offset_cfg_END (1)
#define PMIC_AUX1_IBIAS_CFG_aux1_ibias_cfg_START (2)
#define PMIC_AUX1_IBIAS_CFG_aux1_ibias_cfg_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob0 : 8;
    } reg;
} PMIC_OTP0_0_UNION;
#endif
#define PMIC_OTP0_0_otp0_pdob0_START (0)
#define PMIC_OTP0_0_otp0_pdob0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob1 : 8;
    } reg;
} PMIC_OTP0_1_UNION;
#endif
#define PMIC_OTP0_1_otp0_pdob1_START (0)
#define PMIC_OTP0_1_otp0_pdob1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob2 : 8;
    } reg;
} PMIC_OTP0_2_UNION;
#endif
#define PMIC_OTP0_2_otp0_pdob2_START (0)
#define PMIC_OTP0_2_otp0_pdob2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob3 : 8;
    } reg;
} PMIC_OTP0_3_UNION;
#endif
#define PMIC_OTP0_3_otp0_pdob3_START (0)
#define PMIC_OTP0_3_otp0_pdob3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pwe_int : 1;
        unsigned char otp0_pwe_pulse : 1;
        unsigned char otp0_por_int : 1;
        unsigned char otp0_por_pulse : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_OTP0_CTRL_0_UNION;
#endif
#define PMIC_OTP0_CTRL_0_otp0_pwe_int_START (0)
#define PMIC_OTP0_CTRL_0_otp0_pwe_int_END (0)
#define PMIC_OTP0_CTRL_0_otp0_pwe_pulse_START (1)
#define PMIC_OTP0_CTRL_0_otp0_pwe_pulse_END (1)
#define PMIC_OTP0_CTRL_0_otp0_por_int_START (2)
#define PMIC_OTP0_CTRL_0_otp0_por_int_END (2)
#define PMIC_OTP0_CTRL_0_otp0_por_pulse_START (3)
#define PMIC_OTP0_CTRL_0_otp0_por_pulse_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pa : 2;
        unsigned char otp0_ptm : 2;
        unsigned char otp012_pprog : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_OTP0_CTRL_1_UNION;
#endif
#define PMIC_OTP0_CTRL_1_otp0_pa_START (0)
#define PMIC_OTP0_CTRL_1_otp0_pa_END (1)
#define PMIC_OTP0_CTRL_1_otp0_ptm_START (2)
#define PMIC_OTP0_CTRL_1_otp0_ptm_END (3)
#define PMIC_OTP0_CTRL_1_otp012_pprog_START (4)
#define PMIC_OTP0_CTRL_1_otp012_pprog_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdin : 8;
    } reg;
} PMIC_OTP0_WDATA_UNION;
#endif
#define PMIC_OTP0_WDATA_otp0_pdin_START (0)
#define PMIC_OTP0_WDATA_otp0_pdin_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob0_w : 8;
    } reg;
} PMIC_OTP0_0_W_UNION;
#endif
#define PMIC_OTP0_0_W_otp0_pdob0_w_START (0)
#define PMIC_OTP0_0_W_otp0_pdob0_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob1_w : 8;
    } reg;
} PMIC_OTP0_1_W_UNION;
#endif
#define PMIC_OTP0_1_W_otp0_pdob1_w_START (0)
#define PMIC_OTP0_1_W_otp0_pdob1_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob2_w : 8;
    } reg;
} PMIC_OTP0_2_W_UNION;
#endif
#define PMIC_OTP0_2_W_otp0_pdob2_w_START (0)
#define PMIC_OTP0_2_W_otp0_pdob2_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob3_w : 8;
    } reg;
} PMIC_OTP0_3_W_UNION;
#endif
#define PMIC_OTP0_3_W_otp0_pdob3_w_START (0)
#define PMIC_OTP0_3_W_otp0_pdob3_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp1_pdob0 : 8;
    } reg;
} PMIC_OTP1_0_UNION;
#endif
#define PMIC_OTP1_0_otp1_pdob0_START (0)
#define PMIC_OTP1_0_otp1_pdob0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp1_pdob1 : 8;
    } reg;
} PMIC_OTP1_1_UNION;
#endif
#define PMIC_OTP1_1_otp1_pdob1_START (0)
#define PMIC_OTP1_1_otp1_pdob1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp1_pdob2 : 8;
    } reg;
} PMIC_OTP1_2_UNION;
#endif
#define PMIC_OTP1_2_otp1_pdob2_START (0)
#define PMIC_OTP1_2_otp1_pdob2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp1_pdob3 : 8;
    } reg;
} PMIC_OTP1_3_UNION;
#endif
#define PMIC_OTP1_3_otp1_pdob3_START (0)
#define PMIC_OTP1_3_otp1_pdob3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp1_pwe_int : 1;
        unsigned char otp1_pwe_pulse : 1;
        unsigned char otp1_por_int : 1;
        unsigned char otp1_por_pulse : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_OTP1_CTRL_0_UNION;
#endif
#define PMIC_OTP1_CTRL_0_otp1_pwe_int_START (0)
#define PMIC_OTP1_CTRL_0_otp1_pwe_int_END (0)
#define PMIC_OTP1_CTRL_0_otp1_pwe_pulse_START (1)
#define PMIC_OTP1_CTRL_0_otp1_pwe_pulse_END (1)
#define PMIC_OTP1_CTRL_0_otp1_por_int_START (2)
#define PMIC_OTP1_CTRL_0_otp1_por_int_END (2)
#define PMIC_OTP1_CTRL_0_otp1_por_pulse_START (3)
#define PMIC_OTP1_CTRL_0_otp1_por_pulse_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp1_pa : 2;
        unsigned char otp1_ptm : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_OTP1_CTRL_1_UNION;
#endif
#define PMIC_OTP1_CTRL_1_otp1_pa_START (0)
#define PMIC_OTP1_CTRL_1_otp1_pa_END (1)
#define PMIC_OTP1_CTRL_1_otp1_ptm_START (2)
#define PMIC_OTP1_CTRL_1_otp1_ptm_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp1_pdin : 8;
    } reg;
} PMIC_OTP1_WDATA_UNION;
#endif
#define PMIC_OTP1_WDATA_otp1_pdin_START (0)
#define PMIC_OTP1_WDATA_otp1_pdin_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp1_pdob0_w : 8;
    } reg;
} PMIC_OTP1_0_W_UNION;
#endif
#define PMIC_OTP1_0_W_otp1_pdob0_w_START (0)
#define PMIC_OTP1_0_W_otp1_pdob0_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp1_pdob1_w : 8;
    } reg;
} PMIC_OTP1_1_W_UNION;
#endif
#define PMIC_OTP1_1_W_otp1_pdob1_w_START (0)
#define PMIC_OTP1_1_W_otp1_pdob1_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp1_pdob2_w : 8;
    } reg;
} PMIC_OTP1_2_W_UNION;
#endif
#define PMIC_OTP1_2_W_otp1_pdob2_w_START (0)
#define PMIC_OTP1_2_W_otp1_pdob2_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp1_pdob3_w : 8;
    } reg;
} PMIC_OTP1_3_W_UNION;
#endif
#define PMIC_OTP1_3_W_otp1_pdob3_w_START (0)
#define PMIC_OTP1_3_W_otp1_pdob3_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp2_pdob0 : 8;
    } reg;
} PMIC_OTP2_0_UNION;
#endif
#define PMIC_OTP2_0_otp2_pdob0_START (0)
#define PMIC_OTP2_0_otp2_pdob0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp2_pdob1 : 8;
    } reg;
} PMIC_OTP2_1_UNION;
#endif
#define PMIC_OTP2_1_otp2_pdob1_START (0)
#define PMIC_OTP2_1_otp2_pdob1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp2_pdob2 : 8;
    } reg;
} PMIC_OTP2_2_UNION;
#endif
#define PMIC_OTP2_2_otp2_pdob2_START (0)
#define PMIC_OTP2_2_otp2_pdob2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp2_pdob3 : 8;
    } reg;
} PMIC_OTP2_3_UNION;
#endif
#define PMIC_OTP2_3_otp2_pdob3_START (0)
#define PMIC_OTP2_3_otp2_pdob3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp2_pwe_int : 1;
        unsigned char otp2_pwe_pulse : 1;
        unsigned char otp2_por_int : 1;
        unsigned char otp2_por_pulse : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_OTP2_CTRL_0_UNION;
#endif
#define PMIC_OTP2_CTRL_0_otp2_pwe_int_START (0)
#define PMIC_OTP2_CTRL_0_otp2_pwe_int_END (0)
#define PMIC_OTP2_CTRL_0_otp2_pwe_pulse_START (1)
#define PMIC_OTP2_CTRL_0_otp2_pwe_pulse_END (1)
#define PMIC_OTP2_CTRL_0_otp2_por_int_START (2)
#define PMIC_OTP2_CTRL_0_otp2_por_int_END (2)
#define PMIC_OTP2_CTRL_0_otp2_por_pulse_START (3)
#define PMIC_OTP2_CTRL_0_otp2_por_pulse_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp2_pa : 2;
        unsigned char otp2_ptm : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_OTP2_CTRL_1_UNION;
#endif
#define PMIC_OTP2_CTRL_1_otp2_pa_START (0)
#define PMIC_OTP2_CTRL_1_otp2_pa_END (1)
#define PMIC_OTP2_CTRL_1_otp2_ptm_START (2)
#define PMIC_OTP2_CTRL_1_otp2_ptm_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp2_pdin : 8;
    } reg;
} PMIC_OTP2_WDATA_UNION;
#endif
#define PMIC_OTP2_WDATA_otp2_pdin_START (0)
#define PMIC_OTP2_WDATA_otp2_pdin_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp2_pdob0_w : 8;
    } reg;
} PMIC_OTP2_0_W_UNION;
#endif
#define PMIC_OTP2_0_W_otp2_pdob0_w_START (0)
#define PMIC_OTP2_0_W_otp2_pdob0_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp2_pdob1_w : 8;
    } reg;
} PMIC_OTP2_1_W_UNION;
#endif
#define PMIC_OTP2_1_W_otp2_pdob1_w_START (0)
#define PMIC_OTP2_1_W_otp2_pdob1_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp2_pdob2_w : 8;
    } reg;
} PMIC_OTP2_2_W_UNION;
#endif
#define PMIC_OTP2_2_W_otp2_pdob2_w_START (0)
#define PMIC_OTP2_2_W_otp2_pdob2_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp2_pdob3_w : 8;
    } reg;
} PMIC_OTP2_3_W_UNION;
#endif
#define PMIC_OTP2_3_W_otp2_pdob3_w_START (0)
#define PMIC_OTP2_3_W_otp2_pdob3_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg0 : 8;
    } reg;
} PMIC_HRST_REG0_UNION;
#endif
#define PMIC_HRST_REG0_np_hrst_reg0_START (0)
#define PMIC_HRST_REG0_np_hrst_reg0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg1 : 8;
    } reg;
} PMIC_HRST_REG1_UNION;
#endif
#define PMIC_HRST_REG1_np_hrst_reg1_START (0)
#define PMIC_HRST_REG1_np_hrst_reg1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg2 : 8;
    } reg;
} PMIC_HRST_REG2_UNION;
#endif
#define PMIC_HRST_REG2_np_hrst_reg2_START (0)
#define PMIC_HRST_REG2_np_hrst_reg2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg3 : 8;
    } reg;
} PMIC_HRST_REG3_UNION;
#endif
#define PMIC_HRST_REG3_np_hrst_reg3_START (0)
#define PMIC_HRST_REG3_np_hrst_reg3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg4 : 8;
    } reg;
} PMIC_HRST_REG4_UNION;
#endif
#define PMIC_HRST_REG4_np_hrst_reg4_START (0)
#define PMIC_HRST_REG4_np_hrst_reg4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg5 : 8;
    } reg;
} PMIC_HRST_REG5_UNION;
#endif
#define PMIC_HRST_REG5_np_hrst_reg5_START (0)
#define PMIC_HRST_REG5_np_hrst_reg5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg6 : 8;
    } reg;
} PMIC_HRST_REG6_UNION;
#endif
#define PMIC_HRST_REG6_np_hrst_reg6_START (0)
#define PMIC_HRST_REG6_np_hrst_reg6_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg7 : 8;
    } reg;
} PMIC_HRST_REG7_UNION;
#endif
#define PMIC_HRST_REG7_np_hrst_reg7_START (0)
#define PMIC_HRST_REG7_np_hrst_reg7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg8 : 8;
    } reg;
} PMIC_HRST_REG8_UNION;
#endif
#define PMIC_HRST_REG8_np_hrst_reg8_START (0)
#define PMIC_HRST_REG8_np_hrst_reg8_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg9 : 8;
    } reg;
} PMIC_HRST_REG9_UNION;
#endif
#define PMIC_HRST_REG9_np_hrst_reg9_START (0)
#define PMIC_HRST_REG9_np_hrst_reg9_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg10 : 8;
    } reg;
} PMIC_HRST_REG10_UNION;
#endif
#define PMIC_HRST_REG10_np_hrst_reg10_START (0)
#define PMIC_HRST_REG10_np_hrst_reg10_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg11 : 8;
    } reg;
} PMIC_HRST_REG11_UNION;
#endif
#define PMIC_HRST_REG11_np_hrst_reg11_START (0)
#define PMIC_HRST_REG11_np_hrst_reg11_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg12 : 8;
    } reg;
} PMIC_HRST_REG12_UNION;
#endif
#define PMIC_HRST_REG12_np_hrst_reg12_START (0)
#define PMIC_HRST_REG12_np_hrst_reg12_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg13 : 8;
    } reg;
} PMIC_HRST_REG13_UNION;
#endif
#define PMIC_HRST_REG13_np_hrst_reg13_START (0)
#define PMIC_HRST_REG13_np_hrst_reg13_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg14 : 8;
    } reg;
} PMIC_HRST_REG14_UNION;
#endif
#define PMIC_HRST_REG14_np_hrst_reg14_START (0)
#define PMIC_HRST_REG14_np_hrst_reg14_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_reg_rc_debug : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_DEBUG_CTRL_UNION;
#endif
#define PMIC_DEBUG_CTRL_np_reg_rc_debug_START (0)
#define PMIC_DEBUG_CTRL_np_reg_rc_debug_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sys_debug0 : 8;
    } reg;
} PMIC_SYS_DEBUG0_UNION;
#endif
#define PMIC_SYS_DEBUG0_sys_debug0_START (0)
#define PMIC_SYS_DEBUG0_sys_debug0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sys_debug1 : 8;
    } reg;
} PMIC_SYS_DEBUG1_UNION;
#endif
#define PMIC_SYS_DEBUG1_sys_debug1_START (0)
#define PMIC_SYS_DEBUG1_sys_debug1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char debug_lock : 8;
    } reg;
} PMIC_DEBUG_LOCK_UNION;
#endif
#define PMIC_DEBUG_LOCK_debug_lock_START (0)
#define PMIC_DEBUG_LOCK_debug_lock_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dac0_din_msb : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_DAC0_DIN_MSB_UNION;
#endif
#define PMIC_DAC0_DIN_MSB_dac0_din_msb_START (0)
#define PMIC_DAC0_DIN_MSB_dac0_din_msb_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dac0_din_lsb : 8;
    } reg;
} PMIC_DAC0_DIN_LSB_UNION;
#endif
#define PMIC_DAC0_DIN_LSB_dac0_din_lsb_START (0)
#define PMIC_DAC0_DIN_LSB_dac0_din_lsb_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dac1_din_msb : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_DAC1_DIN_MSB_UNION;
#endif
#define PMIC_DAC1_DIN_MSB_dac1_din_msb_START (0)
#define PMIC_DAC1_DIN_MSB_dac1_din_msb_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dac1_din_lsb : 8;
    } reg;
} PMIC_DAC1_DIN_LSB_UNION;
#endif
#define PMIC_DAC1_DIN_LSB_dac1_din_lsb_START (0)
#define PMIC_DAC1_DIN_LSB_dac1_din_lsb_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char thsd_otmp125_d1mr : 1;
        unsigned char vbus_det_insert_d20mr : 1;
        unsigned char vbus_det_insert_d20mf : 1;
        unsigned char alarmon_r : 1;
        unsigned char pwronn_d6sf : 1;
        unsigned char pwronn_d1sf : 1;
        unsigned char pwronn_d20mr : 1;
        unsigned char pwronn_d20mf : 1;
    } reg;
} PMIC_IRQ0_UNION;
#endif
#define PMIC_IRQ0_thsd_otmp125_d1mr_START (0)
#define PMIC_IRQ0_thsd_otmp125_d1mr_END (0)
#define PMIC_IRQ0_vbus_det_insert_d20mr_START (1)
#define PMIC_IRQ0_vbus_det_insert_d20mr_END (1)
#define PMIC_IRQ0_vbus_det_insert_d20mf_START (2)
#define PMIC_IRQ0_vbus_det_insert_d20mf_END (2)
#define PMIC_IRQ0_alarmon_r_START (3)
#define PMIC_IRQ0_alarmon_r_END (3)
#define PMIC_IRQ0_pwronn_d6sf_START (4)
#define PMIC_IRQ0_pwronn_d6sf_END (4)
#define PMIC_IRQ0_pwronn_d1sf_START (5)
#define PMIC_IRQ0_pwronn_d1sf_END (5)
#define PMIC_IRQ0_pwronn_d20mr_START (6)
#define PMIC_IRQ0_pwronn_d20mr_END (6)
#define PMIC_IRQ0_pwronn_d20mf_START (7)
#define PMIC_IRQ0_pwronn_d20mf_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ocp_scp_r : 1;
        unsigned char coul_r : 1;
        unsigned char sim0_hpd_r : 1;
        unsigned char sim0_hpd_f : 1;
        unsigned char sim1_hpd_r : 1;
        unsigned char sim1_hpd_f : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_IRQ1_UNION;
#endif
#define PMIC_IRQ1_ocp_scp_r_START (0)
#define PMIC_IRQ1_ocp_scp_r_END (0)
#define PMIC_IRQ1_coul_r_START (1)
#define PMIC_IRQ1_coul_r_END (1)
#define PMIC_IRQ1_sim0_hpd_r_START (2)
#define PMIC_IRQ1_sim0_hpd_r_END (2)
#define PMIC_IRQ1_sim0_hpd_f_START (3)
#define PMIC_IRQ1_sim0_hpd_f_END (3)
#define PMIC_IRQ1_sim1_hpd_r_START (4)
#define PMIC_IRQ1_sim1_hpd_r_END (4)
#define PMIC_IRQ1_sim1_hpd_f_START (5)
#define PMIC_IRQ1_sim1_hpd_f_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ocpldo3 : 1;
        unsigned char ocpldo2 : 1;
        unsigned char ocpldo1 : 1;
        unsigned char ocpldo0_2 : 1;
        unsigned char ocpbuck3 : 1;
        unsigned char ocpbuck2 : 1;
        unsigned char ocpbuck1 : 1;
        unsigned char ocpbuck0 : 1;
    } reg;
} PMIC_OCP_IRQ0_UNION;
#endif
#define PMIC_OCP_IRQ0_ocpldo3_START (0)
#define PMIC_OCP_IRQ0_ocpldo3_END (0)
#define PMIC_OCP_IRQ0_ocpldo2_START (1)
#define PMIC_OCP_IRQ0_ocpldo2_END (1)
#define PMIC_OCP_IRQ0_ocpldo1_START (2)
#define PMIC_OCP_IRQ0_ocpldo1_END (2)
#define PMIC_OCP_IRQ0_ocpldo0_2_START (3)
#define PMIC_OCP_IRQ0_ocpldo0_2_END (3)
#define PMIC_OCP_IRQ0_ocpbuck3_START (4)
#define PMIC_OCP_IRQ0_ocpbuck3_END (4)
#define PMIC_OCP_IRQ0_ocpbuck2_START (5)
#define PMIC_OCP_IRQ0_ocpbuck2_END (5)
#define PMIC_OCP_IRQ0_ocpbuck1_START (6)
#define PMIC_OCP_IRQ0_ocpbuck1_END (6)
#define PMIC_OCP_IRQ0_ocpbuck0_START (7)
#define PMIC_OCP_IRQ0_ocpbuck0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ocpldo12 : 1;
        unsigned char ocpldo11 : 1;
        unsigned char ocpldo10 : 1;
        unsigned char ocpldo9 : 1;
        unsigned char ocpldo8 : 1;
        unsigned char ocpldo7 : 1;
        unsigned char ocpldo5 : 1;
        unsigned char ocpldo4 : 1;
    } reg;
} PMIC_OCP_IRQ1_UNION;
#endif
#define PMIC_OCP_IRQ1_ocpldo12_START (0)
#define PMIC_OCP_IRQ1_ocpldo12_END (0)
#define PMIC_OCP_IRQ1_ocpldo11_START (1)
#define PMIC_OCP_IRQ1_ocpldo11_END (1)
#define PMIC_OCP_IRQ1_ocpldo10_START (2)
#define PMIC_OCP_IRQ1_ocpldo10_END (2)
#define PMIC_OCP_IRQ1_ocpldo9_START (3)
#define PMIC_OCP_IRQ1_ocpldo9_END (3)
#define PMIC_OCP_IRQ1_ocpldo8_START (4)
#define PMIC_OCP_IRQ1_ocpldo8_END (4)
#define PMIC_OCP_IRQ1_ocpldo7_START (5)
#define PMIC_OCP_IRQ1_ocpldo7_END (5)
#define PMIC_OCP_IRQ1_ocpldo5_START (6)
#define PMIC_OCP_IRQ1_ocpldo5_END (6)
#define PMIC_OCP_IRQ1_ocpldo4_START (7)
#define PMIC_OCP_IRQ1_ocpldo4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ocpldo21 : 1;
        unsigned char ocpldo20 : 1;
        unsigned char ocpldo19 : 1;
        unsigned char ocpldo17 : 1;
        unsigned char ocpldo16 : 1;
        unsigned char ocpldo15 : 1;
        unsigned char ocpldo14 : 1;
        unsigned char ocpldo13 : 1;
    } reg;
} PMIC_OCP_IRQ2_UNION;
#endif
#define PMIC_OCP_IRQ2_ocpldo21_START (0)
#define PMIC_OCP_IRQ2_ocpldo21_END (0)
#define PMIC_OCP_IRQ2_ocpldo20_START (1)
#define PMIC_OCP_IRQ2_ocpldo20_END (1)
#define PMIC_OCP_IRQ2_ocpldo19_START (2)
#define PMIC_OCP_IRQ2_ocpldo19_END (2)
#define PMIC_OCP_IRQ2_ocpldo17_START (3)
#define PMIC_OCP_IRQ2_ocpldo17_END (3)
#define PMIC_OCP_IRQ2_ocpldo16_START (4)
#define PMIC_OCP_IRQ2_ocpldo16_END (4)
#define PMIC_OCP_IRQ2_ocpldo15_START (5)
#define PMIC_OCP_IRQ2_ocpldo15_END (5)
#define PMIC_OCP_IRQ2_ocpldo14_START (6)
#define PMIC_OCP_IRQ2_ocpldo14_END (6)
#define PMIC_OCP_IRQ2_ocpldo13_START (7)
#define PMIC_OCP_IRQ2_ocpldo13_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ocpldo29 : 1;
        unsigned char ocpldo28 : 1;
        unsigned char ocpldo27 : 1;
        unsigned char ocpldo26 : 1;
        unsigned char ocpldo25 : 1;
        unsigned char ocpldo24 : 1;
        unsigned char ocpldo23 : 1;
        unsigned char ocpldo22 : 1;
    } reg;
} PMIC_OCP_IRQ3_UNION;
#endif
#define PMIC_OCP_IRQ3_ocpldo29_START (0)
#define PMIC_OCP_IRQ3_ocpldo29_END (0)
#define PMIC_OCP_IRQ3_ocpldo28_START (1)
#define PMIC_OCP_IRQ3_ocpldo28_END (1)
#define PMIC_OCP_IRQ3_ocpldo27_START (2)
#define PMIC_OCP_IRQ3_ocpldo27_END (2)
#define PMIC_OCP_IRQ3_ocpldo26_START (3)
#define PMIC_OCP_IRQ3_ocpldo26_END (3)
#define PMIC_OCP_IRQ3_ocpldo25_START (4)
#define PMIC_OCP_IRQ3_ocpldo25_END (4)
#define PMIC_OCP_IRQ3_ocpldo24_START (5)
#define PMIC_OCP_IRQ3_ocpldo24_END (5)
#define PMIC_OCP_IRQ3_ocpldo23_START (6)
#define PMIC_OCP_IRQ3_ocpldo23_END (6)
#define PMIC_OCP_IRQ3_ocpldo22_START (7)
#define PMIC_OCP_IRQ3_ocpldo22_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char classd_ocp : 1;
        unsigned char ocpldo34 : 1;
        unsigned char ocpldo33 : 1;
        unsigned char ocpldo32 : 1;
        unsigned char ocpldo31 : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_OCP_IRQ4_UNION;
#endif
#define PMIC_OCP_IRQ4_classd_ocp_START (0)
#define PMIC_OCP_IRQ4_classd_ocp_END (0)
#define PMIC_OCP_IRQ4_ocpldo34_START (1)
#define PMIC_OCP_IRQ4_ocpldo34_END (1)
#define PMIC_OCP_IRQ4_ocpldo33_START (2)
#define PMIC_OCP_IRQ4_ocpldo33_END (2)
#define PMIC_OCP_IRQ4_ocpldo32_START (3)
#define PMIC_OCP_IRQ4_ocpldo32_END (3)
#define PMIC_OCP_IRQ4_ocpldo31_START (4)
#define PMIC_OCP_IRQ4_ocpldo31_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_scp : 1;
        unsigned char buck2_scp : 1;
        unsigned char buck1_scp : 1;
        unsigned char buck0_scp : 1;
        unsigned char xo_ldo_buff_scp : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_OCP_IRQ5_UNION;
#endif
#define PMIC_OCP_IRQ5_buck3_scp_START (0)
#define PMIC_OCP_IRQ5_buck3_scp_END (0)
#define PMIC_OCP_IRQ5_buck2_scp_START (1)
#define PMIC_OCP_IRQ5_buck2_scp_END (1)
#define PMIC_OCP_IRQ5_buck1_scp_START (2)
#define PMIC_OCP_IRQ5_buck1_scp_END (2)
#define PMIC_OCP_IRQ5_buck0_scp_START (3)
#define PMIC_OCP_IRQ5_buck0_scp_END (3)
#define PMIC_OCP_IRQ5_xo_ldo_buff_scp_START (4)
#define PMIC_OCP_IRQ5_xo_ldo_buff_scp_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ocpldo3 : 1;
        unsigned char np_ocpldo2 : 1;
        unsigned char np_ocpldo1 : 1;
        unsigned char np_ocpldo0_2 : 1;
        unsigned char np_ocpbuck3 : 1;
        unsigned char np_ocpbuck2 : 1;
        unsigned char np_ocpbuck1 : 1;
        unsigned char np_ocpbuck0 : 1;
    } reg;
} PMIC_NP_OCP0_UNION;
#endif
#define PMIC_NP_OCP0_np_ocpldo3_START (0)
#define PMIC_NP_OCP0_np_ocpldo3_END (0)
#define PMIC_NP_OCP0_np_ocpldo2_START (1)
#define PMIC_NP_OCP0_np_ocpldo2_END (1)
#define PMIC_NP_OCP0_np_ocpldo1_START (2)
#define PMIC_NP_OCP0_np_ocpldo1_END (2)
#define PMIC_NP_OCP0_np_ocpldo0_2_START (3)
#define PMIC_NP_OCP0_np_ocpldo0_2_END (3)
#define PMIC_NP_OCP0_np_ocpbuck3_START (4)
#define PMIC_NP_OCP0_np_ocpbuck3_END (4)
#define PMIC_NP_OCP0_np_ocpbuck2_START (5)
#define PMIC_NP_OCP0_np_ocpbuck2_END (5)
#define PMIC_NP_OCP0_np_ocpbuck1_START (6)
#define PMIC_NP_OCP0_np_ocpbuck1_END (6)
#define PMIC_NP_OCP0_np_ocpbuck0_START (7)
#define PMIC_NP_OCP0_np_ocpbuck0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ocpldo12 : 1;
        unsigned char np_ocpldo11 : 1;
        unsigned char np_ocpldo10 : 1;
        unsigned char np_ocpldo9 : 1;
        unsigned char np_ocpldo8 : 1;
        unsigned char np_ocpldo7 : 1;
        unsigned char np_ocpldo5 : 1;
        unsigned char np_ocpldo4 : 1;
    } reg;
} PMIC_NP_OCP1_UNION;
#endif
#define PMIC_NP_OCP1_np_ocpldo12_START (0)
#define PMIC_NP_OCP1_np_ocpldo12_END (0)
#define PMIC_NP_OCP1_np_ocpldo11_START (1)
#define PMIC_NP_OCP1_np_ocpldo11_END (1)
#define PMIC_NP_OCP1_np_ocpldo10_START (2)
#define PMIC_NP_OCP1_np_ocpldo10_END (2)
#define PMIC_NP_OCP1_np_ocpldo9_START (3)
#define PMIC_NP_OCP1_np_ocpldo9_END (3)
#define PMIC_NP_OCP1_np_ocpldo8_START (4)
#define PMIC_NP_OCP1_np_ocpldo8_END (4)
#define PMIC_NP_OCP1_np_ocpldo7_START (5)
#define PMIC_NP_OCP1_np_ocpldo7_END (5)
#define PMIC_NP_OCP1_np_ocpldo5_START (6)
#define PMIC_NP_OCP1_np_ocpldo5_END (6)
#define PMIC_NP_OCP1_np_ocpldo4_START (7)
#define PMIC_NP_OCP1_np_ocpldo4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ocpldo21 : 1;
        unsigned char np_ocpldo20 : 1;
        unsigned char np_ocpldo19 : 1;
        unsigned char np_ocpldo17 : 1;
        unsigned char np_ocpldo16 : 1;
        unsigned char np_ocpldo15 : 1;
        unsigned char np_ocpldo14 : 1;
        unsigned char np_ocpldo13 : 1;
    } reg;
} PMIC_NP_OCP2_UNION;
#endif
#define PMIC_NP_OCP2_np_ocpldo21_START (0)
#define PMIC_NP_OCP2_np_ocpldo21_END (0)
#define PMIC_NP_OCP2_np_ocpldo20_START (1)
#define PMIC_NP_OCP2_np_ocpldo20_END (1)
#define PMIC_NP_OCP2_np_ocpldo19_START (2)
#define PMIC_NP_OCP2_np_ocpldo19_END (2)
#define PMIC_NP_OCP2_np_ocpldo17_START (3)
#define PMIC_NP_OCP2_np_ocpldo17_END (3)
#define PMIC_NP_OCP2_np_ocpldo16_START (4)
#define PMIC_NP_OCP2_np_ocpldo16_END (4)
#define PMIC_NP_OCP2_np_ocpldo15_START (5)
#define PMIC_NP_OCP2_np_ocpldo15_END (5)
#define PMIC_NP_OCP2_np_ocpldo14_START (6)
#define PMIC_NP_OCP2_np_ocpldo14_END (6)
#define PMIC_NP_OCP2_np_ocpldo13_START (7)
#define PMIC_NP_OCP2_np_ocpldo13_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ocpldo29 : 1;
        unsigned char np_ocpldo28 : 1;
        unsigned char np_ocpldo27 : 1;
        unsigned char np_ocpldo26 : 1;
        unsigned char np_ocpldo25 : 1;
        unsigned char np_ocpldo24 : 1;
        unsigned char np_ocpldo23 : 1;
        unsigned char np_ocpldo22 : 1;
    } reg;
} PMIC_NP_OCP3_UNION;
#endif
#define PMIC_NP_OCP3_np_ocpldo29_START (0)
#define PMIC_NP_OCP3_np_ocpldo29_END (0)
#define PMIC_NP_OCP3_np_ocpldo28_START (1)
#define PMIC_NP_OCP3_np_ocpldo28_END (1)
#define PMIC_NP_OCP3_np_ocpldo27_START (2)
#define PMIC_NP_OCP3_np_ocpldo27_END (2)
#define PMIC_NP_OCP3_np_ocpldo26_START (3)
#define PMIC_NP_OCP3_np_ocpldo26_END (3)
#define PMIC_NP_OCP3_np_ocpldo25_START (4)
#define PMIC_NP_OCP3_np_ocpldo25_END (4)
#define PMIC_NP_OCP3_np_ocpldo24_START (5)
#define PMIC_NP_OCP3_np_ocpldo24_END (5)
#define PMIC_NP_OCP3_np_ocpldo23_START (6)
#define PMIC_NP_OCP3_np_ocpldo23_END (6)
#define PMIC_NP_OCP3_np_ocpldo22_START (7)
#define PMIC_NP_OCP3_np_ocpldo22_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_classd_ocp : 1;
        unsigned char np_ocpldo34 : 1;
        unsigned char np_ocpldo33 : 1;
        unsigned char np_ocpldo32 : 1;
        unsigned char np_ocpldo31 : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_NP_OCP4_UNION;
#endif
#define PMIC_NP_OCP4_np_classd_ocp_START (0)
#define PMIC_NP_OCP4_np_classd_ocp_END (0)
#define PMIC_NP_OCP4_np_ocpldo34_START (1)
#define PMIC_NP_OCP4_np_ocpldo34_END (1)
#define PMIC_NP_OCP4_np_ocpldo33_START (2)
#define PMIC_NP_OCP4_np_ocpldo33_END (2)
#define PMIC_NP_OCP4_np_ocpldo32_START (3)
#define PMIC_NP_OCP4_np_ocpldo32_END (3)
#define PMIC_NP_OCP4_np_ocpldo31_START (4)
#define PMIC_NP_OCP4_np_ocpldo31_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_buck3_scp : 1;
        unsigned char np_buck2_scp : 1;
        unsigned char np_buck1_scp : 1;
        unsigned char np_buck0_scp : 1;
        unsigned char np_xo_ldo_buff_scp : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_NP_SCP_UNION;
#endif
#define PMIC_NP_SCP_np_buck3_scp_START (0)
#define PMIC_NP_SCP_np_buck3_scp_END (0)
#define PMIC_NP_SCP_np_buck2_scp_START (1)
#define PMIC_NP_SCP_np_buck2_scp_END (1)
#define PMIC_NP_SCP_np_buck1_scp_START (2)
#define PMIC_NP_SCP_np_buck1_scp_END (2)
#define PMIC_NP_SCP_np_buck0_scp_START (3)
#define PMIC_NP_SCP_np_buck0_scp_END (3)
#define PMIC_NP_SCP_np_xo_ldo_buff_scp_START (4)
#define PMIC_NP_SCP_np_xo_ldo_buff_scp_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_vsys_ov_d200ur : 1;
        unsigned char np_vsys_pwroff_abs_2d : 1;
        unsigned char np_vsys_pwroff_deb_d80mr : 1;
        unsigned char np_thsd_otmp140_d1mr : 1;
        unsigned char np_thsd_otmp125_d1mr : 1;
        unsigned char np_hresetn_d90uf : 1;
        unsigned char np_avdd_osc_vld_d20nf : 1;
        unsigned char np_19m2_dis : 1;
    } reg;
} PMIC_NP_RECORD0_UNION;
#endif
#define PMIC_NP_RECORD0_np_vsys_ov_d200ur_START (0)
#define PMIC_NP_RECORD0_np_vsys_ov_d200ur_END (0)
#define PMIC_NP_RECORD0_np_vsys_pwroff_abs_2d_START (1)
#define PMIC_NP_RECORD0_np_vsys_pwroff_abs_2d_END (1)
#define PMIC_NP_RECORD0_np_vsys_pwroff_deb_d80mr_START (2)
#define PMIC_NP_RECORD0_np_vsys_pwroff_deb_d80mr_END (2)
#define PMIC_NP_RECORD0_np_thsd_otmp140_d1mr_START (3)
#define PMIC_NP_RECORD0_np_thsd_otmp140_d1mr_END (3)
#define PMIC_NP_RECORD0_np_thsd_otmp125_d1mr_START (4)
#define PMIC_NP_RECORD0_np_thsd_otmp125_d1mr_END (4)
#define PMIC_NP_RECORD0_np_hresetn_d90uf_START (5)
#define PMIC_NP_RECORD0_np_hresetn_d90uf_END (5)
#define PMIC_NP_RECORD0_np_avdd_osc_vld_d20nf_START (6)
#define PMIC_NP_RECORD0_np_avdd_osc_vld_d20nf_END (6)
#define PMIC_NP_RECORD0_np_19m2_dis_START (7)
#define PMIC_NP_RECORD0_np_19m2_dis_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_pwronn_restart : 1;
        unsigned char np_pwrhold_shutdown : 1;
        unsigned char np_pwronn_shutdown : 1;
        unsigned char np_pwrhold_pwrup : 1;
        unsigned char np_alarmon_pwrup : 1;
        unsigned char np_vbus_pwrup : 1;
        unsigned char np_pwronn_pwrup : 1;
        unsigned char np_fast_pwrup : 1;
    } reg;
} PMIC_NP_RECORD1_UNION;
#endif
#define PMIC_NP_RECORD1_np_pwronn_restart_START (0)
#define PMIC_NP_RECORD1_np_pwronn_restart_END (0)
#define PMIC_NP_RECORD1_np_pwrhold_shutdown_START (1)
#define PMIC_NP_RECORD1_np_pwrhold_shutdown_END (1)
#define PMIC_NP_RECORD1_np_pwronn_shutdown_START (2)
#define PMIC_NP_RECORD1_np_pwronn_shutdown_END (2)
#define PMIC_NP_RECORD1_np_pwrhold_pwrup_START (3)
#define PMIC_NP_RECORD1_np_pwrhold_pwrup_END (3)
#define PMIC_NP_RECORD1_np_alarmon_pwrup_START (4)
#define PMIC_NP_RECORD1_np_alarmon_pwrup_END (4)
#define PMIC_NP_RECORD1_np_vbus_pwrup_START (5)
#define PMIC_NP_RECORD1_np_vbus_pwrup_END (5)
#define PMIC_NP_RECORD1_np_pwronn_pwrup_START (6)
#define PMIC_NP_RECORD1_np_pwronn_pwrup_END (6)
#define PMIC_NP_RECORD1_np_fast_pwrup_START (7)
#define PMIC_NP_RECORD1_np_fast_pwrup_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_tcxo_clk_sel_r : 1;
        unsigned char np_tcxo_clk_sel_f : 1;
        unsigned char np_dcxo_clk_sel_r : 1;
        unsigned char np_dcxo_clk_sel_f : 1;
        unsigned char np_vsys_vcoin_sel : 1;
        unsigned char np_smpl : 1;
        unsigned char np_core_io_vld_f : 1;
        unsigned char np_pwrhold_1s : 1;
    } reg;
} PMIC_NP_RECORD2_UNION;
#endif
#define PMIC_NP_RECORD2_np_tcxo_clk_sel_r_START (0)
#define PMIC_NP_RECORD2_np_tcxo_clk_sel_r_END (0)
#define PMIC_NP_RECORD2_np_tcxo_clk_sel_f_START (1)
#define PMIC_NP_RECORD2_np_tcxo_clk_sel_f_END (1)
#define PMIC_NP_RECORD2_np_dcxo_clk_sel_r_START (2)
#define PMIC_NP_RECORD2_np_dcxo_clk_sel_r_END (2)
#define PMIC_NP_RECORD2_np_dcxo_clk_sel_f_START (3)
#define PMIC_NP_RECORD2_np_dcxo_clk_sel_f_END (3)
#define PMIC_NP_RECORD2_np_vsys_vcoin_sel_START (4)
#define PMIC_NP_RECORD2_np_vsys_vcoin_sel_END (4)
#define PMIC_NP_RECORD2_np_smpl_START (5)
#define PMIC_NP_RECORD2_np_smpl_END (5)
#define PMIC_NP_RECORD2_np_core_io_vld_f_START (6)
#define PMIC_NP_RECORD2_np_core_io_vld_f_END (6)
#define PMIC_NP_RECORD2_np_pwrhold_1s_START (7)
#define PMIC_NP_RECORD2_np_pwrhold_1s_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_pmua_short_f : 1;
        unsigned char np_ldo34_short_f : 1;
        unsigned char np_vin_ldoh_shutdown : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_NP_RECORD3_UNION;
#endif
#define PMIC_NP_RECORD3_np_pmua_short_f_START (0)
#define PMIC_NP_RECORD3_np_pmua_short_f_END (0)
#define PMIC_NP_RECORD3_np_ldo34_short_f_START (1)
#define PMIC_NP_RECORD3_np_ldo34_short_f_END (1)
#define PMIC_NP_RECORD3_np_vin_ldoh_shutdown_START (2)
#define PMIC_NP_RECORD3_np_vin_ldoh_shutdown_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtcdr0 : 8;
    } reg;
} PMIC_RTCDR0_UNION;
#endif
#define PMIC_RTCDR0_rtcdr0_START (0)
#define PMIC_RTCDR0_rtcdr0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtcdr1 : 8;
    } reg;
} PMIC_RTCDR1_UNION;
#endif
#define PMIC_RTCDR1_rtcdr1_START (0)
#define PMIC_RTCDR1_rtcdr1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtcdr2 : 8;
    } reg;
} PMIC_RTCDR2_UNION;
#endif
#define PMIC_RTCDR2_rtcdr2_START (0)
#define PMIC_RTCDR2_rtcdr2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtcdr3 : 8;
    } reg;
} PMIC_RTCDR3_UNION;
#endif
#define PMIC_RTCDR3_rtcdr3_START (0)
#define PMIC_RTCDR3_rtcdr3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtcmr0 : 8;
    } reg;
} PMIC_RTCMR0_UNION;
#endif
#define PMIC_RTCMR0_rtcmr0_START (0)
#define PMIC_RTCMR0_rtcmr0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtcmr1 : 8;
    } reg;
} PMIC_RTCMR1_UNION;
#endif
#define PMIC_RTCMR1_rtcmr1_START (0)
#define PMIC_RTCMR1_rtcmr1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtcmr2 : 8;
    } reg;
} PMIC_RTCMR2_UNION;
#endif
#define PMIC_RTCMR2_rtcmr2_START (0)
#define PMIC_RTCMR2_rtcmr2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtcmr3 : 8;
    } reg;
} PMIC_RTCMR3_UNION;
#endif
#define PMIC_RTCMR3_rtcmr3_START (0)
#define PMIC_RTCMR3_rtcmr3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtcclr0 : 8;
    } reg;
} PMIC_RTCLR0_UNION;
#endif
#define PMIC_RTCLR0_rtcclr0_START (0)
#define PMIC_RTCLR0_rtcclr0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtcclr1 : 8;
    } reg;
} PMIC_RTCLR1_UNION;
#endif
#define PMIC_RTCLR1_rtcclr1_START (0)
#define PMIC_RTCLR1_rtcclr1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtcclr2 : 8;
    } reg;
} PMIC_RTCLR2_UNION;
#endif
#define PMIC_RTCLR2_rtcclr2_START (0)
#define PMIC_RTCLR2_rtcclr2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtcclr3 : 8;
    } reg;
} PMIC_RTCLR3_UNION;
#endif
#define PMIC_RTCLR3_rtcclr3_START (0)
#define PMIC_RTCLR3_rtcclr3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtccr : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_RTCCTRL_UNION;
#endif
#define PMIC_RTCCTRL_rtccr_START (0)
#define PMIC_RTCCTRL_rtccr_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_cali_thresold_low : 8;
    } reg;
} PMIC_XO_THRESOLD0_UNION;
#endif
#define PMIC_XO_THRESOLD0_xo_cali_thresold_low_START (0)
#define PMIC_XO_THRESOLD0_xo_cali_thresold_low_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_cali_thresold1_high : 8;
    } reg;
} PMIC_XO_THRESOLD1_UNION;
#endif
#define PMIC_XO_THRESOLD1_xo_cali_thresold1_high_START (0)
#define PMIC_XO_THRESOLD1_xo_cali_thresold1_high_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char crc_value0 : 8;
    } reg;
} PMIC_CRC_VAULE0_UNION;
#endif
#define PMIC_CRC_VAULE0_crc_value0_START (0)
#define PMIC_CRC_VAULE0_crc_value0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char crc_value1 : 8;
    } reg;
} PMIC_CRC_VAULE1_UNION;
#endif
#define PMIC_CRC_VAULE1_crc_value1_START (0)
#define PMIC_CRC_VAULE1_crc_value1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char crc_value2 : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_CRC_VAULE2_UNION;
#endif
#define PMIC_CRC_VAULE2_crc_value2_START (0)
#define PMIC_CRC_VAULE2_crc_value2_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_pwrup_timer0 : 8;
    } reg;
} PMIC_RTC_PWRUP_TIMER0_UNION;
#endif
#define PMIC_RTC_PWRUP_TIMER0_rtc_pwrup_timer0_START (0)
#define PMIC_RTC_PWRUP_TIMER0_rtc_pwrup_timer0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_pwrup_timer1 : 8;
    } reg;
} PMIC_RTC_PWRUP_TIMER1_UNION;
#endif
#define PMIC_RTC_PWRUP_TIMER1_rtc_pwrup_timer1_START (0)
#define PMIC_RTC_PWRUP_TIMER1_rtc_pwrup_timer1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_pwrup_timer2 : 8;
    } reg;
} PMIC_RTC_PWRUP_TIMER2_UNION;
#endif
#define PMIC_RTC_PWRUP_TIMER2_rtc_pwrup_timer2_START (0)
#define PMIC_RTC_PWRUP_TIMER2_rtc_pwrup_timer2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_pwrup_timer3 : 8;
    } reg;
} PMIC_RTC_PWRUP_TIMER3_UNION;
#endif
#define PMIC_RTC_PWRUP_TIMER3_rtc_pwrup_timer3_START (0)
#define PMIC_RTC_PWRUP_TIMER3_rtc_pwrup_timer3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_pwrdown_timer0 : 8;
    } reg;
} PMIC_RTC_PWRDOWN_TIMER0_UNION;
#endif
#define PMIC_RTC_PWRDOWN_TIMER0_rtc_pwrdown_timer0_START (0)
#define PMIC_RTC_PWRDOWN_TIMER0_rtc_pwrdown_timer0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_pwrdown_timer1 : 8;
    } reg;
} PMIC_RTC_PWRDOWN_TIMER1_UNION;
#endif
#define PMIC_RTC_PWRDOWN_TIMER1_rtc_pwrdown_timer1_START (0)
#define PMIC_RTC_PWRDOWN_TIMER1_rtc_pwrdown_timer1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_pwrdown_timer2 : 8;
    } reg;
} PMIC_RTC_PWRDOWN_TIMER2_UNION;
#endif
#define PMIC_RTC_PWRDOWN_TIMER2_rtc_pwrdown_timer2_START (0)
#define PMIC_RTC_PWRDOWN_TIMER2_rtc_pwrdown_timer2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_pwrdown_timer3 : 8;
    } reg;
} PMIC_RTC_PWRDOWN_TIMER3_UNION;
#endif
#define PMIC_RTC_PWRDOWN_TIMER3_rtc_pwrdown_timer3_START (0)
#define PMIC_RTC_PWRDOWN_TIMER3_rtc_pwrdown_timer3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cl_gate_int : 1;
        unsigned char cl_out_int : 1;
        unsigned char cl_in_int : 1;
        unsigned char v_gate_int : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_COUL_IRQ_UNION;
#endif
#define PMIC_COUL_IRQ_cl_gate_int_START (0)
#define PMIC_COUL_IRQ_cl_gate_int_END (0)
#define PMIC_COUL_IRQ_cl_out_int_START (1)
#define PMIC_COUL_IRQ_cl_out_int_END (1)
#define PMIC_COUL_IRQ_cl_in_int_START (2)
#define PMIC_COUL_IRQ_cl_in_int_END (2)
#define PMIC_COUL_IRQ_v_gate_int_START (3)
#define PMIC_COUL_IRQ_v_gate_int_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cl_gate_int_mk : 1;
        unsigned char cl_out_int_mk : 1;
        unsigned char cl_in_int_mk : 1;
        unsigned char v_gate_int_mk : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_COUL_IRQ_MASK_UNION;
#endif
#define PMIC_COUL_IRQ_MASK_cl_gate_int_mk_START (0)
#define PMIC_COUL_IRQ_MASK_cl_gate_int_mk_END (0)
#define PMIC_COUL_IRQ_MASK_cl_out_int_mk_START (1)
#define PMIC_COUL_IRQ_MASK_cl_out_int_mk_END (1)
#define PMIC_COUL_IRQ_MASK_cl_in_int_mk_START (2)
#define PMIC_COUL_IRQ_MASK_cl_in_int_mk_END (2)
#define PMIC_COUL_IRQ_MASK_v_gate_int_mk_START (3)
#define PMIC_COUL_IRQ_MASK_v_gate_int_mk_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_s0 : 8;
    } reg;
} PMIC_CURRENT_0_UNION;
#endif
#define PMIC_CURRENT_0_current_s0_START (0)
#define PMIC_CURRENT_0_current_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_s1 : 8;
    } reg;
} PMIC_CURRENT_1_UNION;
#endif
#define PMIC_CURRENT_1_current_s1_START (0)
#define PMIC_CURRENT_1_current_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_s0 : 8;
    } reg;
} PMIC_V_OUT_0_UNION;
#endif
#define PMIC_V_OUT_0_v_out_s0_START (0)
#define PMIC_V_OUT_0_v_out_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_s1 : 8;
    } reg;
} PMIC_V_OUT_1_UNION;
#endif
#define PMIC_V_OUT_1_v_out_s1_START (0)
#define PMIC_V_OUT_1_v_out_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_ctrl : 3;
        unsigned char reflash_value_ctrl : 1;
        unsigned char eco_filter_time : 3;
        unsigned char calibration_ctrl : 1;
    } reg;
} PMIC_CLJ_CTRL_REG_UNION;
#endif
#define PMIC_CLJ_CTRL_REG_eco_ctrl_START (0)
#define PMIC_CLJ_CTRL_REG_eco_ctrl_END (2)
#define PMIC_CLJ_CTRL_REG_reflash_value_ctrl_START (3)
#define PMIC_CLJ_CTRL_REG_reflash_value_ctrl_END (3)
#define PMIC_CLJ_CTRL_REG_eco_filter_time_START (4)
#define PMIC_CLJ_CTRL_REG_eco_filter_time_END (6)
#define PMIC_CLJ_CTRL_REG_calibration_ctrl_START (7)
#define PMIC_CLJ_CTRL_REG_calibration_ctrl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_reflash_time : 8;
    } reg;
} PMIC_ECO_REFALSH_TIME_UNION;
#endif
#define PMIC_ECO_REFALSH_TIME_eco_reflash_time_START (0)
#define PMIC_ECO_REFALSH_TIME_eco_reflash_time_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cl_out_s0 : 8;
    } reg;
} PMIC_CL_OUT0_UNION;
#endif
#define PMIC_CL_OUT0_cl_out_s0_START (0)
#define PMIC_CL_OUT0_cl_out_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cl_out_s1 : 8;
    } reg;
} PMIC_CL_OUT1_UNION;
#endif
#define PMIC_CL_OUT1_cl_out_s1_START (0)
#define PMIC_CL_OUT1_cl_out_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cl_out_s2 : 8;
    } reg;
} PMIC_CL_OUT2_UNION;
#endif
#define PMIC_CL_OUT2_cl_out_s2_START (0)
#define PMIC_CL_OUT2_cl_out_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cl_out_s3 : 8;
    } reg;
} PMIC_CL_OUT3_UNION;
#endif
#define PMIC_CL_OUT3_cl_out_s3_START (0)
#define PMIC_CL_OUT3_cl_out_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cl_in_s0 : 8;
    } reg;
} PMIC_CL_IN0_UNION;
#endif
#define PMIC_CL_IN0_cl_in_s0_START (0)
#define PMIC_CL_IN0_cl_in_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cl_in_s1 : 8;
    } reg;
} PMIC_CL_IN1_UNION;
#endif
#define PMIC_CL_IN1_cl_in_s1_START (0)
#define PMIC_CL_IN1_cl_in_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cl_in_s2 : 8;
    } reg;
} PMIC_CL_IN2_UNION;
#endif
#define PMIC_CL_IN2_cl_in_s2_START (0)
#define PMIC_CL_IN2_cl_in_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cl_in_s3 : 8;
    } reg;
} PMIC_CL_IN3_UNION;
#endif
#define PMIC_CL_IN3_cl_in_s3_START (0)
#define PMIC_CL_IN3_cl_in_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chg_timer_s0 : 8;
    } reg;
} PMIC_CHG_TIMER0_UNION;
#endif
#define PMIC_CHG_TIMER0_chg_timer_s0_START (0)
#define PMIC_CHG_TIMER0_chg_timer_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chg_timer_s1 : 8;
    } reg;
} PMIC_CHG_TIMER1_UNION;
#endif
#define PMIC_CHG_TIMER1_chg_timer_s1_START (0)
#define PMIC_CHG_TIMER1_chg_timer_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chg_timer_s2 : 8;
    } reg;
} PMIC_CHG_TIMER2_UNION;
#endif
#define PMIC_CHG_TIMER2_chg_timer_s2_START (0)
#define PMIC_CHG_TIMER2_chg_timer_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chg_timer_s3 : 8;
    } reg;
} PMIC_CHG_TIMER3_UNION;
#endif
#define PMIC_CHG_TIMER3_chg_timer_s3_START (0)
#define PMIC_CHG_TIMER3_chg_timer_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char load_timer_s0 : 8;
    } reg;
} PMIC_LOAD_TIMER0_UNION;
#endif
#define PMIC_LOAD_TIMER0_load_timer_s0_START (0)
#define PMIC_LOAD_TIMER0_load_timer_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char load_timer_s1 : 8;
    } reg;
} PMIC_LOAD_TIMER1_UNION;
#endif
#define PMIC_LOAD_TIMER1_load_timer_s1_START (0)
#define PMIC_LOAD_TIMER1_load_timer_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char load_timer_s2 : 8;
    } reg;
} PMIC_LOAD_TIMER2_UNION;
#endif
#define PMIC_LOAD_TIMER2_load_timer_s2_START (0)
#define PMIC_LOAD_TIMER2_load_timer_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char load_timer_s3 : 8;
    } reg;
} PMIC_LOAD_TIMER3_UNION;
#endif
#define PMIC_LOAD_TIMER3_load_timer_s3_START (0)
#define PMIC_LOAD_TIMER3_load_timer_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cl_int_s0 : 8;
    } reg;
} PMIC_CL_INT0_UNION;
#endif
#define PMIC_CL_INT0_cl_int_s0_START (0)
#define PMIC_CL_INT0_cl_int_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cl_int_s1 : 8;
    } reg;
} PMIC_CL_INT1_UNION;
#endif
#define PMIC_CL_INT1_cl_int_s1_START (0)
#define PMIC_CL_INT1_cl_int_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cl_int_s2 : 8;
    } reg;
} PMIC_CL_INT2_UNION;
#endif
#define PMIC_CL_INT2_cl_int_s2_START (0)
#define PMIC_CL_INT2_cl_int_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cl_int_s3 : 8;
    } reg;
} PMIC_CL_INT3_UNION;
#endif
#define PMIC_CL_INT3_cl_int_s3_START (0)
#define PMIC_CL_INT3_cl_int_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_int_s0 : 8;
    } reg;
} PMIC_V_INT0_UNION;
#endif
#define PMIC_V_INT0_v_int_s0_START (0)
#define PMIC_V_INT0_v_int_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_int_s1 : 8;
    } reg;
} PMIC_V_INT1_UNION;
#endif
#define PMIC_V_INT1_v_int_s1_START (0)
#define PMIC_V_INT1_v_int_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char offset_current_s0 : 8;
    } reg;
} PMIC_OFFSET_CURRENT0_UNION;
#endif
#define PMIC_OFFSET_CURRENT0_offset_current_s0_START (0)
#define PMIC_OFFSET_CURRENT0_offset_current_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char offset_current_s1 : 8;
    } reg;
} PMIC_OFFSET_CURRENT1_UNION;
#endif
#define PMIC_OFFSET_CURRENT1_offset_current_s1_START (0)
#define PMIC_OFFSET_CURRENT1_offset_current_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char offset_voltage_s0 : 8;
    } reg;
} PMIC_OFFSET_VOLTAGE0_UNION;
#endif
#define PMIC_OFFSET_VOLTAGE0_offset_voltage_s0_START (0)
#define PMIC_OFFSET_VOLTAGE0_offset_voltage_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char offset_voltage_s1 : 8;
    } reg;
} PMIC_OFFSET_VOLTAGE1_UNION;
#endif
#define PMIC_OFFSET_VOLTAGE1_offset_voltage_s1_START (0)
#define PMIC_OFFSET_VOLTAGE1_offset_voltage_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_data_s0 : 8;
    } reg;
} PMIC_OCV_VOLTAGE0_UNION;
#endif
#define PMIC_OCV_VOLTAGE0_v_ocv_data_s0_START (0)
#define PMIC_OCV_VOLTAGE0_v_ocv_data_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_data_s1 : 8;
    } reg;
} PMIC_OCV_VOLTAGE1_UNION;
#endif
#define PMIC_OCV_VOLTAGE1_v_ocv_data_s1_START (0)
#define PMIC_OCV_VOLTAGE1_v_ocv_data_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_data_s0 : 8;
    } reg;
} PMIC_OCV_CURRENT0_UNION;
#endif
#define PMIC_OCV_CURRENT0_i_ocv_data_s0_START (0)
#define PMIC_OCV_CURRENT0_i_ocv_data_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_data_s1 : 8;
    } reg;
} PMIC_OCV_CURRENT1_UNION;
#endif
#define PMIC_OCV_CURRENT1_i_ocv_data_s1_START (0)
#define PMIC_OCV_CURRENT1_i_ocv_data_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_out_clin_s0 : 8;
    } reg;
} PMIC_ECO_OUT_CLIN_0_UNION;
#endif
#define PMIC_ECO_OUT_CLIN_0_eco_out_clin_s0_START (0)
#define PMIC_ECO_OUT_CLIN_0_eco_out_clin_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_out_clin_s1 : 8;
    } reg;
} PMIC_ECO_OUT_CLIN_1_UNION;
#endif
#define PMIC_ECO_OUT_CLIN_1_eco_out_clin_s1_START (0)
#define PMIC_ECO_OUT_CLIN_1_eco_out_clin_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_out_clin_s2 : 8;
    } reg;
} PMIC_ECO_OUT_CLIN_2_UNION;
#endif
#define PMIC_ECO_OUT_CLIN_2_eco_out_clin_s2_START (0)
#define PMIC_ECO_OUT_CLIN_2_eco_out_clin_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_out_clin_s3 : 8;
    } reg;
} PMIC_ECO_OUT_CLIN_3_UNION;
#endif
#define PMIC_ECO_OUT_CLIN_3_eco_out_clin_s3_START (0)
#define PMIC_ECO_OUT_CLIN_3_eco_out_clin_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_out_clout_s0 : 8;
    } reg;
} PMIC_ECO_OUT_CLOUT_0_UNION;
#endif
#define PMIC_ECO_OUT_CLOUT_0_eco_out_clout_s0_START (0)
#define PMIC_ECO_OUT_CLOUT_0_eco_out_clout_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_out_clout_s1 : 8;
    } reg;
} PMIC_ECO_OUT_CLOUT_1_UNION;
#endif
#define PMIC_ECO_OUT_CLOUT_1_eco_out_clout_s1_START (0)
#define PMIC_ECO_OUT_CLOUT_1_eco_out_clout_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_out_clout_s2 : 8;
    } reg;
} PMIC_ECO_OUT_CLOUT_2_UNION;
#endif
#define PMIC_ECO_OUT_CLOUT_2_eco_out_clout_s2_START (0)
#define PMIC_ECO_OUT_CLOUT_2_eco_out_clout_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_out_clout_s3 : 8;
    } reg;
} PMIC_ECO_OUT_CLOUT_3_UNION;
#endif
#define PMIC_ECO_OUT_CLOUT_3_eco_out_clout_s3_START (0)
#define PMIC_ECO_OUT_CLOUT_3_eco_out_clout_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre0_s0 : 8;
    } reg;
} PMIC_V_OUT0_PRE0_UNION;
#endif
#define PMIC_V_OUT0_PRE0_v_out_pre0_s0_START (0)
#define PMIC_V_OUT0_PRE0_v_out_pre0_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre0_s1 : 8;
    } reg;
} PMIC_V_OUT1_PRE0_UNION;
#endif
#define PMIC_V_OUT1_PRE0_v_out_pre0_s1_START (0)
#define PMIC_V_OUT1_PRE0_v_out_pre0_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre1_s0 : 8;
    } reg;
} PMIC_V_OUT0_PRE1_UNION;
#endif
#define PMIC_V_OUT0_PRE1_v_out_pre1_s0_START (0)
#define PMIC_V_OUT0_PRE1_v_out_pre1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre1_s1 : 8;
    } reg;
} PMIC_V_OUT1_PRE1_UNION;
#endif
#define PMIC_V_OUT1_PRE1_v_out_pre1_s1_START (0)
#define PMIC_V_OUT1_PRE1_v_out_pre1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre2_s0 : 8;
    } reg;
} PMIC_V_OUT0_PRE2_UNION;
#endif
#define PMIC_V_OUT0_PRE2_v_out_pre2_s0_START (0)
#define PMIC_V_OUT0_PRE2_v_out_pre2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre2_s1 : 8;
    } reg;
} PMIC_V_OUT1_PRE2_UNION;
#endif
#define PMIC_V_OUT1_PRE2_v_out_pre2_s1_START (0)
#define PMIC_V_OUT1_PRE2_v_out_pre2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre3_s0 : 8;
    } reg;
} PMIC_V_OUT0_PRE3_UNION;
#endif
#define PMIC_V_OUT0_PRE3_v_out_pre3_s0_START (0)
#define PMIC_V_OUT0_PRE3_v_out_pre3_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre3_s1 : 8;
    } reg;
} PMIC_V_OUT1_PRE3_UNION;
#endif
#define PMIC_V_OUT1_PRE3_v_out_pre3_s1_START (0)
#define PMIC_V_OUT1_PRE3_v_out_pre3_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre4_s0 : 8;
    } reg;
} PMIC_V_OUT0_PRE4_UNION;
#endif
#define PMIC_V_OUT0_PRE4_v_out_pre4_s0_START (0)
#define PMIC_V_OUT0_PRE4_v_out_pre4_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre4_s1 : 8;
    } reg;
} PMIC_V_OUT1_PRE4_UNION;
#endif
#define PMIC_V_OUT1_PRE4_v_out_pre4_s1_START (0)
#define PMIC_V_OUT1_PRE4_v_out_pre4_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre5_s0 : 8;
    } reg;
} PMIC_V_OUT0_PRE5_UNION;
#endif
#define PMIC_V_OUT0_PRE5_v_out_pre5_s0_START (0)
#define PMIC_V_OUT0_PRE5_v_out_pre5_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre5_s1 : 8;
    } reg;
} PMIC_V_OUT1_PRE5_UNION;
#endif
#define PMIC_V_OUT1_PRE5_v_out_pre5_s1_START (0)
#define PMIC_V_OUT1_PRE5_v_out_pre5_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre6_s0 : 8;
    } reg;
} PMIC_V_OUT0_PRE6_UNION;
#endif
#define PMIC_V_OUT0_PRE6_v_out_pre6_s0_START (0)
#define PMIC_V_OUT0_PRE6_v_out_pre6_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre6_s1 : 8;
    } reg;
} PMIC_V_OUT1_PRE6_UNION;
#endif
#define PMIC_V_OUT1_PRE6_v_out_pre6_s1_START (0)
#define PMIC_V_OUT1_PRE6_v_out_pre6_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre7_s0 : 8;
    } reg;
} PMIC_V_OUT0_PRE7_UNION;
#endif
#define PMIC_V_OUT0_PRE7_v_out_pre7_s0_START (0)
#define PMIC_V_OUT0_PRE7_v_out_pre7_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre7_s1 : 8;
    } reg;
} PMIC_V_OUT1_PRE7_UNION;
#endif
#define PMIC_V_OUT1_PRE7_v_out_pre7_s1_START (0)
#define PMIC_V_OUT1_PRE7_v_out_pre7_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre8_s0 : 8;
    } reg;
} PMIC_V_OUT0_PRE8_UNION;
#endif
#define PMIC_V_OUT0_PRE8_v_out_pre8_s0_START (0)
#define PMIC_V_OUT0_PRE8_v_out_pre8_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre8_s1 : 8;
    } reg;
} PMIC_V_OUT1_PRE8_UNION;
#endif
#define PMIC_V_OUT1_PRE8_v_out_pre8_s1_START (0)
#define PMIC_V_OUT1_PRE8_v_out_pre8_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre9_s0 : 8;
    } reg;
} PMIC_V_OUT0_PRE9_UNION;
#endif
#define PMIC_V_OUT0_PRE9_v_out_pre9_s0_START (0)
#define PMIC_V_OUT0_PRE9_v_out_pre9_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre9_s1 : 8;
    } reg;
} PMIC_V_OUT1_PRE9_UNION;
#endif
#define PMIC_V_OUT1_PRE9_v_out_pre9_s1_START (0)
#define PMIC_V_OUT1_PRE9_v_out_pre9_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre0_s0 : 8;
    } reg;
} PMIC_CURRENT0_PRE0_UNION;
#endif
#define PMIC_CURRENT0_PRE0_current_pre0_s0_START (0)
#define PMIC_CURRENT0_PRE0_current_pre0_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre0_s1 : 8;
    } reg;
} PMIC_CURRENT1_PRE0_UNION;
#endif
#define PMIC_CURRENT1_PRE0_current_pre0_s1_START (0)
#define PMIC_CURRENT1_PRE0_current_pre0_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre1_s0 : 8;
    } reg;
} PMIC_CURRENT0_PRE1_UNION;
#endif
#define PMIC_CURRENT0_PRE1_current_pre1_s0_START (0)
#define PMIC_CURRENT0_PRE1_current_pre1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre1_s1 : 8;
    } reg;
} PMIC_CURRENT1_PRE1_UNION;
#endif
#define PMIC_CURRENT1_PRE1_current_pre1_s1_START (0)
#define PMIC_CURRENT1_PRE1_current_pre1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre2_s0 : 8;
    } reg;
} PMIC_CURRENT0_PRE2_UNION;
#endif
#define PMIC_CURRENT0_PRE2_current_pre2_s0_START (0)
#define PMIC_CURRENT0_PRE2_current_pre2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre2_s1 : 8;
    } reg;
} PMIC_CURRENT1_PRE2_UNION;
#endif
#define PMIC_CURRENT1_PRE2_current_pre2_s1_START (0)
#define PMIC_CURRENT1_PRE2_current_pre2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre3_s0 : 8;
    } reg;
} PMIC_CURRENT0_PRE3_UNION;
#endif
#define PMIC_CURRENT0_PRE3_current_pre3_s0_START (0)
#define PMIC_CURRENT0_PRE3_current_pre3_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre3_s1 : 8;
    } reg;
} PMIC_CURRENT1_PRE3_UNION;
#endif
#define PMIC_CURRENT1_PRE3_current_pre3_s1_START (0)
#define PMIC_CURRENT1_PRE3_current_pre3_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre4_s0 : 8;
    } reg;
} PMIC_CURRENT0_PRE4_UNION;
#endif
#define PMIC_CURRENT0_PRE4_current_pre4_s0_START (0)
#define PMIC_CURRENT0_PRE4_current_pre4_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre4_s1 : 8;
    } reg;
} PMIC_CURRENT1_PRE4_UNION;
#endif
#define PMIC_CURRENT1_PRE4_current_pre4_s1_START (0)
#define PMIC_CURRENT1_PRE4_current_pre4_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre5_s0 : 8;
    } reg;
} PMIC_CURRENT0_PRE5_UNION;
#endif
#define PMIC_CURRENT0_PRE5_current_pre5_s0_START (0)
#define PMIC_CURRENT0_PRE5_current_pre5_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre5_s1 : 8;
    } reg;
} PMIC_CURRENT1_PRE5_UNION;
#endif
#define PMIC_CURRENT1_PRE5_current_pre5_s1_START (0)
#define PMIC_CURRENT1_PRE5_current_pre5_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre6_s0 : 8;
    } reg;
} PMIC_CURRENT0_PRE6_UNION;
#endif
#define PMIC_CURRENT0_PRE6_current_pre6_s0_START (0)
#define PMIC_CURRENT0_PRE6_current_pre6_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre6_s1 : 8;
    } reg;
} PMIC_CURRENT1_PRE6_UNION;
#endif
#define PMIC_CURRENT1_PRE6_current_pre6_s1_START (0)
#define PMIC_CURRENT1_PRE6_current_pre6_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre7_s0 : 8;
    } reg;
} PMIC_CURRENT0_PRE7_UNION;
#endif
#define PMIC_CURRENT0_PRE7_current_pre7_s0_START (0)
#define PMIC_CURRENT0_PRE7_current_pre7_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre7_s1 : 8;
    } reg;
} PMIC_CURRENT1_PRE7_UNION;
#endif
#define PMIC_CURRENT1_PRE7_current_pre7_s1_START (0)
#define PMIC_CURRENT1_PRE7_current_pre7_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre8_s0 : 8;
    } reg;
} PMIC_CURRENT0_PRE8_UNION;
#endif
#define PMIC_CURRENT0_PRE8_current_pre8_s0_START (0)
#define PMIC_CURRENT0_PRE8_current_pre8_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre8_s1 : 8;
    } reg;
} PMIC_CURRENT1_PRE8_UNION;
#endif
#define PMIC_CURRENT1_PRE8_current_pre8_s1_START (0)
#define PMIC_CURRENT1_PRE8_current_pre8_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre9_s0 : 8;
    } reg;
} PMIC_CURRENT0_PRE9_UNION;
#endif
#define PMIC_CURRENT0_PRE9_current_pre9_s0_START (0)
#define PMIC_CURRENT0_PRE9_current_pre9_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre9_s1 : 8;
    } reg;
} PMIC_CURRENT1_PRE9_UNION;
#endif
#define PMIC_CURRENT1_PRE9_current_pre9_s1_START (0)
#define PMIC_CURRENT1_PRE9_current_pre9_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char offset_current_mod_s0 : 8;
    } reg;
} PMIC_OFFSET_CURRENT_MOD_0_UNION;
#endif
#define PMIC_OFFSET_CURRENT_MOD_0_offset_current_mod_s0_START (0)
#define PMIC_OFFSET_CURRENT_MOD_0_offset_current_mod_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char offset_current_mod_s1 : 8;
    } reg;
} PMIC_OFFSET_CURRENT_MOD_1_UNION;
#endif
#define PMIC_OFFSET_CURRENT_MOD_1_offset_current_mod_s1_START (0)
#define PMIC_OFFSET_CURRENT_MOD_1_offset_current_mod_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char offset_voltage_mod_s0 : 8;
    } reg;
} PMIC_OFFSET_VOLTAGE_MOD_0_UNION;
#endif
#define PMIC_OFFSET_VOLTAGE_MOD_0_offset_voltage_mod_s0_START (0)
#define PMIC_OFFSET_VOLTAGE_MOD_0_offset_voltage_mod_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char offset_voltage_mod_s1 : 8;
    } reg;
} PMIC_OFFSET_VOLTAGE_MOD_1_UNION;
#endif
#define PMIC_OFFSET_VOLTAGE_MOD_1_offset_voltage_mod_s1_START (0)
#define PMIC_OFFSET_VOLTAGE_MOD_1_offset_voltage_mod_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char coul_reserve0 : 8;
    } reg;
} PMIC_COUL_RESERVE0_UNION;
#endif
#define PMIC_COUL_RESERVE0_coul_reserve0_START (0)
#define PMIC_COUL_RESERVE0_coul_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_reserve_1 : 8;
    } reg;
} PMIC_CLJ_RESERVED0_UNION;
#endif
#define PMIC_CLJ_RESERVED0_i_reserve_1_START (0)
#define PMIC_CLJ_RESERVED0_i_reserve_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_reserve_2 : 8;
    } reg;
} PMIC_CLJ_RESERVED1_UNION;
#endif
#define PMIC_CLJ_RESERVED1_i_reserve_2_START (0)
#define PMIC_CLJ_RESERVED1_i_reserve_2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_reserve_1 : 8;
    } reg;
} PMIC_CLJ_RESERVED2_UNION;
#endif
#define PMIC_CLJ_RESERVED2_v_reserve_1_START (0)
#define PMIC_CLJ_RESERVED2_v_reserve_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soft_rst_n : 8;
    } reg;
} PMIC_PMU_SOFT_RST_UNION;
#endif
#define PMIC_PMU_SOFT_RST_soft_rst_n_START (0)
#define PMIC_PMU_SOFT_RST_soft_rst_n_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char clj_debug_0 : 1;
        unsigned char clj_debug_1 : 1;
        unsigned char clj_debug_2 : 1;
        unsigned char clj_debug_3 : 1;
        unsigned char cali_en_i : 1;
        unsigned char cali_en_i_force : 1;
        unsigned char cali_en_v_force : 1;
        unsigned char cali_en_v : 1;
    } reg;
} PMIC_CLJ_DEBUG_UNION;
#endif
#define PMIC_CLJ_DEBUG_clj_debug_0_START (0)
#define PMIC_CLJ_DEBUG_clj_debug_0_END (0)
#define PMIC_CLJ_DEBUG_clj_debug_1_START (1)
#define PMIC_CLJ_DEBUG_clj_debug_1_END (1)
#define PMIC_CLJ_DEBUG_clj_debug_2_START (2)
#define PMIC_CLJ_DEBUG_clj_debug_2_END (2)
#define PMIC_CLJ_DEBUG_clj_debug_3_START (3)
#define PMIC_CLJ_DEBUG_clj_debug_3_END (3)
#define PMIC_CLJ_DEBUG_cali_en_i_START (4)
#define PMIC_CLJ_DEBUG_cali_en_i_END (4)
#define PMIC_CLJ_DEBUG_cali_en_i_force_START (5)
#define PMIC_CLJ_DEBUG_cali_en_i_force_END (5)
#define PMIC_CLJ_DEBUG_cali_en_v_force_START (6)
#define PMIC_CLJ_DEBUG_cali_en_v_force_END (6)
#define PMIC_CLJ_DEBUG_cali_en_v_START (7)
#define PMIC_CLJ_DEBUG_cali_en_v_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_coul_always_off : 1;
        unsigned char voltage_coul_always_off : 1;
        unsigned char clj_debug_2_2 : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_CLJ_DEBUG_2_UNION;
#endif
#define PMIC_CLJ_DEBUG_2_current_coul_always_off_START (0)
#define PMIC_CLJ_DEBUG_2_current_coul_always_off_END (0)
#define PMIC_CLJ_DEBUG_2_voltage_coul_always_off_START (1)
#define PMIC_CLJ_DEBUG_2_voltage_coul_always_off_END (1)
#define PMIC_CLJ_DEBUG_2_clj_debug_2_2_START (2)
#define PMIC_CLJ_DEBUG_2_clj_debug_2_2_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char mstate : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_STATE_TEST_UNION;
#endif
#define PMIC_STATE_TEST_mstate_START (0)
#define PMIC_STATE_TEST_mstate_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char coul_ctrl_onoff_reg : 1;
        unsigned char reg_data_clr : 1;
        unsigned char cali_auto_time : 3;
        unsigned char cali_auto_onoff_ctrl : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_CLJ_CTRL_REGS2_UNION;
#endif
#define PMIC_CLJ_CTRL_REGS2_coul_ctrl_onoff_reg_START (0)
#define PMIC_CLJ_CTRL_REGS2_coul_ctrl_onoff_reg_END (0)
#define PMIC_CLJ_CTRL_REGS2_reg_data_clr_START (1)
#define PMIC_CLJ_CTRL_REGS2_reg_data_clr_END (1)
#define PMIC_CLJ_CTRL_REGS2_cali_auto_time_START (2)
#define PMIC_CLJ_CTRL_REGS2_cali_auto_time_END (4)
#define PMIC_CLJ_CTRL_REGS2_cali_auto_onoff_ctrl_START (5)
#define PMIC_CLJ_CTRL_REGS2_cali_auto_onoff_ctrl_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char debug_write_pro : 8;
    } reg;
} PMIC_DEBUG_WRITE_PRO_UNION;
#endif
#define PMIC_DEBUG_WRITE_PRO_debug_write_pro_START (0)
#define PMIC_DEBUG_WRITE_PRO_debug_write_pro_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dacl_en : 1;
        unsigned char dacr_en : 1;
        unsigned char adcl_en : 1;
        unsigned char adcr_en : 1;
        unsigned char sif_en : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_CLK_EN_CFG_UNION;
#endif
#define PMIC_CLK_EN_CFG_dacl_en_START (0)
#define PMIC_CLK_EN_CFG_dacl_en_END (0)
#define PMIC_CLK_EN_CFG_dacr_en_START (1)
#define PMIC_CLK_EN_CFG_dacr_en_END (1)
#define PMIC_CLK_EN_CFG_adcl_en_START (2)
#define PMIC_CLK_EN_CFG_adcl_en_END (2)
#define PMIC_CLK_EN_CFG_adcr_en_START (3)
#define PMIC_CLK_EN_CFG_adcr_en_END (3)
#define PMIC_CLK_EN_CFG_sif_en_START (4)
#define PMIC_CLK_EN_CFG_sif_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dacl_clk_edge_sel : 1;
        unsigned char dacr_clk_edge_sel : 1;
        unsigned char adcl_clk_edge_sel : 1;
        unsigned char adcr_clk_edge_sel : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_CLK_EDGE_CFG_UNION;
#endif
#define PMIC_CLK_EDGE_CFG_dacl_clk_edge_sel_START (0)
#define PMIC_CLK_EDGE_CFG_dacl_clk_edge_sel_END (0)
#define PMIC_CLK_EDGE_CFG_dacr_clk_edge_sel_START (1)
#define PMIC_CLK_EDGE_CFG_dacr_clk_edge_sel_END (1)
#define PMIC_CLK_EDGE_CFG_adcl_clk_edge_sel_START (2)
#define PMIC_CLK_EDGE_CFG_adcl_clk_edge_sel_END (2)
#define PMIC_CLK_EDGE_CFG_adcr_clk_edge_sel_START (3)
#define PMIC_CLK_EDGE_CFG_adcr_clk_edge_sel_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dac_loopback : 1;
        unsigned char adc_loopback : 1;
        unsigned char sif_s2p_loopback : 1;
        unsigned char dac_dem_din_sel : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LOOPBACK_CFG_UNION;
#endif
#define PMIC_LOOPBACK_CFG_dac_loopback_START (0)
#define PMIC_LOOPBACK_CFG_dac_loopback_END (0)
#define PMIC_LOOPBACK_CFG_adc_loopback_START (1)
#define PMIC_LOOPBACK_CFG_adc_loopback_END (1)
#define PMIC_LOOPBACK_CFG_sif_s2p_loopback_START (2)
#define PMIC_LOOPBACK_CFG_sif_s2p_loopback_END (2)
#define PMIC_LOOPBACK_CFG_dac_dem_din_sel_START (3)
#define PMIC_LOOPBACK_CFG_dac_dem_din_sel_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hs_mic_nor1_irq : 1;
        unsigned char hs_mic_nor2_irq : 1;
        unsigned char hs_mic_eco_irq : 1;
        unsigned char hs_det_irq : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_ANA_IRQ_SIG_STAT_UNION;
#endif
#define PMIC_ANA_IRQ_SIG_STAT_hs_mic_nor1_irq_START (0)
#define PMIC_ANA_IRQ_SIG_STAT_hs_mic_nor1_irq_END (0)
#define PMIC_ANA_IRQ_SIG_STAT_hs_mic_nor2_irq_START (1)
#define PMIC_ANA_IRQ_SIG_STAT_hs_mic_nor2_irq_END (1)
#define PMIC_ANA_IRQ_SIG_STAT_hs_mic_eco_irq_START (2)
#define PMIC_ANA_IRQ_SIG_STAT_hs_mic_eco_irq_END (2)
#define PMIC_ANA_IRQ_SIG_STAT_hs_det_irq_START (3)
#define PMIC_ANA_IRQ_SIG_STAT_hs_det_irq_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ana_irq_mask : 8;
    } reg;
} PMIC_ANA_IRQM_REG0_UNION;
#endif
#define PMIC_ANA_IRQM_REG0_ana_irq_mask_START (0)
#define PMIC_ANA_IRQM_REG0_ana_irq_mask_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hs_mic_nor1_irq_pos : 1;
        unsigned char hs_mic_nor1_irq_neg : 1;
        unsigned char hs_mic_nor2_irq_pos : 1;
        unsigned char hs_mic_nor2_irq_neg : 1;
        unsigned char hs_mic_eco_irq_pos : 1;
        unsigned char hs_mic_eco_irq_neg : 1;
        unsigned char hs_det_irq_pos : 1;
        unsigned char hs_det_irq_neg : 1;
    } reg;
} PMIC_ANA_IRQ_REG0_UNION;
#endif
#define PMIC_ANA_IRQ_REG0_hs_mic_nor1_irq_pos_START (0)
#define PMIC_ANA_IRQ_REG0_hs_mic_nor1_irq_pos_END (0)
#define PMIC_ANA_IRQ_REG0_hs_mic_nor1_irq_neg_START (1)
#define PMIC_ANA_IRQ_REG0_hs_mic_nor1_irq_neg_END (1)
#define PMIC_ANA_IRQ_REG0_hs_mic_nor2_irq_pos_START (2)
#define PMIC_ANA_IRQ_REG0_hs_mic_nor2_irq_pos_END (2)
#define PMIC_ANA_IRQ_REG0_hs_mic_nor2_irq_neg_START (3)
#define PMIC_ANA_IRQ_REG0_hs_mic_nor2_irq_neg_END (3)
#define PMIC_ANA_IRQ_REG0_hs_mic_eco_irq_pos_START (4)
#define PMIC_ANA_IRQ_REG0_hs_mic_eco_irq_pos_END (4)
#define PMIC_ANA_IRQ_REG0_hs_mic_eco_irq_neg_START (5)
#define PMIC_ANA_IRQ_REG0_hs_mic_eco_irq_neg_END (5)
#define PMIC_ANA_IRQ_REG0_hs_det_irq_pos_START (6)
#define PMIC_ANA_IRQ_REG0_hs_det_irq_pos_END (6)
#define PMIC_ANA_IRQ_REG0_hs_det_irq_neg_START (7)
#define PMIC_ANA_IRQ_REG0_hs_det_irq_neg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char deb_cnt_hs_det_irq : 5;
        unsigned char bypass_deb_hs_det_irq : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_DEB_CNT_HS_DET_CFG_UNION;
#endif
#define PMIC_DEB_CNT_HS_DET_CFG_deb_cnt_hs_det_irq_START (0)
#define PMIC_DEB_CNT_HS_DET_CFG_deb_cnt_hs_det_irq_END (4)
#define PMIC_DEB_CNT_HS_DET_CFG_bypass_deb_hs_det_irq_START (5)
#define PMIC_DEB_CNT_HS_DET_CFG_bypass_deb_hs_det_irq_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char deb_cnt_hs_mic_irq : 5;
        unsigned char bypass_deb_hs_mic_irq : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_DEB_CNT_HS_MIC_CFG_UNION;
#endif
#define PMIC_DEB_CNT_HS_MIC_CFG_deb_cnt_hs_mic_irq_START (0)
#define PMIC_DEB_CNT_HS_MIC_CFG_deb_cnt_hs_mic_irq_END (4)
#define PMIC_DEB_CNT_HS_MIC_CFG_bypass_deb_hs_mic_irq_START (5)
#define PMIC_DEB_CNT_HS_MIC_CFG_bypass_deb_hs_mic_irq_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 8;
    } reg;
} PMIC_CODEC_DIG_RES02_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 8;
    } reg;
} PMIC_CODEC_DIG_RES03_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char linepgar_pd : 1;
        unsigned char linepgal_pd : 1;
        unsigned char auxpga_pd : 1;
        unsigned char mainpga_pd : 1;
        unsigned char adcr_pd : 1;
        unsigned char adcl_pd : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_CODEC_ANA_RW01_UNION;
#endif
#define PMIC_CODEC_ANA_RW01_linepgar_pd_START (0)
#define PMIC_CODEC_ANA_RW01_linepgar_pd_END (0)
#define PMIC_CODEC_ANA_RW01_linepgal_pd_START (1)
#define PMIC_CODEC_ANA_RW01_linepgal_pd_END (1)
#define PMIC_CODEC_ANA_RW01_auxpga_pd_START (2)
#define PMIC_CODEC_ANA_RW01_auxpga_pd_END (2)
#define PMIC_CODEC_ANA_RW01_mainpga_pd_START (3)
#define PMIC_CODEC_ANA_RW01_mainpga_pd_END (3)
#define PMIC_CODEC_ANA_RW01_adcr_pd_START (4)
#define PMIC_CODEC_ANA_RW01_adcr_pd_END (4)
#define PMIC_CODEC_ANA_RW01_adcl_pd_START (5)
#define PMIC_CODEC_ANA_RW01_adcl_pd_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ibias_pd : 1;
        unsigned char reserved_0 : 1;
        unsigned char reserved_1 : 1;
        unsigned char reserved_2 : 1;
        unsigned char mbhd_buff_pd : 1;
        unsigned char mbhd_comp_pd : 1;
        unsigned char hsmicb_pd : 1;
        unsigned char micb1_pd : 1;
    } reg;
} PMIC_CODEC_ANA_RW02_UNION;
#endif
#define PMIC_CODEC_ANA_RW02_ibias_pd_START (0)
#define PMIC_CODEC_ANA_RW02_ibias_pd_END (0)
#define PMIC_CODEC_ANA_RW02_mbhd_buff_pd_START (4)
#define PMIC_CODEC_ANA_RW02_mbhd_buff_pd_END (4)
#define PMIC_CODEC_ANA_RW02_mbhd_comp_pd_START (5)
#define PMIC_CODEC_ANA_RW02_mbhd_comp_pd_END (5)
#define PMIC_CODEC_ANA_RW02_hsmicb_pd_START (6)
#define PMIC_CODEC_ANA_RW02_hsmicb_pd_END (6)
#define PMIC_CODEC_ANA_RW02_micb1_pd_START (7)
#define PMIC_CODEC_ANA_RW02_micb1_pd_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved_0 : 1;
        unsigned char hsr_pd : 1;
        unsigned char hsl_pd : 1;
        unsigned char reserved_1 : 1;
        unsigned char mixout_hsr_pd : 1;
        unsigned char mixout_hsl_pd : 1;
        unsigned char reserved_2 : 1;
        unsigned char reserved_3 : 1;
    } reg;
} PMIC_CODEC_ANA_RW03_UNION;
#endif
#define PMIC_CODEC_ANA_RW03_hsr_pd_START (1)
#define PMIC_CODEC_ANA_RW03_hsr_pd_END (1)
#define PMIC_CODEC_ANA_RW03_hsl_pd_START (2)
#define PMIC_CODEC_ANA_RW03_hsl_pd_END (2)
#define PMIC_CODEC_ANA_RW03_mixout_hsr_pd_START (4)
#define PMIC_CODEC_ANA_RW03_mixout_hsr_pd_END (4)
#define PMIC_CODEC_ANA_RW03_mixout_hsl_pd_START (5)
#define PMIC_CODEC_ANA_RW03_mixout_hsl_pd_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vrefsel : 2;
        unsigned char analog_loop : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_CODEC_ANA_RW04_UNION;
#endif
#define PMIC_CODEC_ANA_RW04_vrefsel_START (0)
#define PMIC_CODEC_ANA_RW04_vrefsel_END (1)
#define PMIC_CODEC_ANA_RW04_analog_loop_START (2)
#define PMIC_CODEC_ANA_RW04_analog_loop_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char linepgal_mute : 1;
        unsigned char linepgal_gain : 4;
        unsigned char linepgal_sel : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_CODEC_ANA_RW05_UNION;
#endif
#define PMIC_CODEC_ANA_RW05_linepgal_mute_START (0)
#define PMIC_CODEC_ANA_RW05_linepgal_mute_END (0)
#define PMIC_CODEC_ANA_RW05_linepgal_gain_START (1)
#define PMIC_CODEC_ANA_RW05_linepgal_gain_END (4)
#define PMIC_CODEC_ANA_RW05_linepgal_sel_START (5)
#define PMIC_CODEC_ANA_RW05_linepgal_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char linepgar_mute : 1;
        unsigned char linepgar_gain : 4;
        unsigned char linepgar_sel : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_CODEC_ANA_RW06_UNION;
#endif
#define PMIC_CODEC_ANA_RW06_linepgar_mute_START (0)
#define PMIC_CODEC_ANA_RW06_linepgar_mute_END (0)
#define PMIC_CODEC_ANA_RW06_linepgar_gain_START (1)
#define PMIC_CODEC_ANA_RW06_linepgar_gain_END (4)
#define PMIC_CODEC_ANA_RW06_linepgar_sel_START (5)
#define PMIC_CODEC_ANA_RW06_linepgar_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char auxpga_sel : 2;
        unsigned char auxpga_mute : 1;
        unsigned char auxpga_gain : 3;
        unsigned char auxpga_boost : 1;
        unsigned char micpga_unlock_bps : 1;
    } reg;
} PMIC_CODEC_ANA_RW07_UNION;
#endif
#define PMIC_CODEC_ANA_RW07_auxpga_sel_START (0)
#define PMIC_CODEC_ANA_RW07_auxpga_sel_END (1)
#define PMIC_CODEC_ANA_RW07_auxpga_mute_START (2)
#define PMIC_CODEC_ANA_RW07_auxpga_mute_END (2)
#define PMIC_CODEC_ANA_RW07_auxpga_gain_START (3)
#define PMIC_CODEC_ANA_RW07_auxpga_gain_END (5)
#define PMIC_CODEC_ANA_RW07_auxpga_boost_START (6)
#define PMIC_CODEC_ANA_RW07_auxpga_boost_END (6)
#define PMIC_CODEC_ANA_RW07_micpga_unlock_bps_START (7)
#define PMIC_CODEC_ANA_RW07_micpga_unlock_bps_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char mainpga_mute : 1;
        unsigned char mainpga_gain : 3;
        unsigned char mainpga_boost : 1;
        unsigned char mainpga_sel : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_CODEC_ANA_RW08_UNION;
#endif
#define PMIC_CODEC_ANA_RW08_mainpga_mute_START (0)
#define PMIC_CODEC_ANA_RW08_mainpga_mute_END (0)
#define PMIC_CODEC_ANA_RW08_mainpga_gain_START (1)
#define PMIC_CODEC_ANA_RW08_mainpga_gain_END (3)
#define PMIC_CODEC_ANA_RW08_mainpga_boost_START (4)
#define PMIC_CODEC_ANA_RW08_mainpga_boost_END (4)
#define PMIC_CODEC_ANA_RW08_mainpga_sel_START (5)
#define PMIC_CODEC_ANA_RW08_mainpga_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char adcr_mixin : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_CODEC_ANA_RW09_UNION;
#endif
#define PMIC_CODEC_ANA_RW09_adcr_mixin_START (0)
#define PMIC_CODEC_ANA_RW09_adcr_mixin_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char adcl_mixin : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_CODEC_ANA_RW10_UNION;
#endif
#define PMIC_CODEC_ANA_RW10_adcl_mixin_START (0)
#define PMIC_CODEC_ANA_RW10_adcl_mixin_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char adcl_dwa_bps : 1;
        unsigned char adcl_flstn : 2;
        unsigned char adcl_dac_bias : 2;
        unsigned char adcl_clk_delay_0 : 1;
        unsigned char adcl_clk_delay_1 : 1;
        unsigned char adcl_clk_delay_2 : 1;
    } reg;
} PMIC_CODEC_ANA_RW11_UNION;
#endif
#define PMIC_CODEC_ANA_RW11_adcl_dwa_bps_START (0)
#define PMIC_CODEC_ANA_RW11_adcl_dwa_bps_END (0)
#define PMIC_CODEC_ANA_RW11_adcl_flstn_START (1)
#define PMIC_CODEC_ANA_RW11_adcl_flstn_END (2)
#define PMIC_CODEC_ANA_RW11_adcl_dac_bias_START (3)
#define PMIC_CODEC_ANA_RW11_adcl_dac_bias_END (4)
#define PMIC_CODEC_ANA_RW11_adcl_clk_delay_0_START (5)
#define PMIC_CODEC_ANA_RW11_adcl_clk_delay_0_END (5)
#define PMIC_CODEC_ANA_RW11_adcl_clk_delay_1_START (6)
#define PMIC_CODEC_ANA_RW11_adcl_clk_delay_1_END (6)
#define PMIC_CODEC_ANA_RW11_adcl_clk_delay_2_START (7)
#define PMIC_CODEC_ANA_RW11_adcl_clk_delay_2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char adcr_dwa_bps : 1;
        unsigned char adcr_flstn : 2;
        unsigned char adcr_dac_bias : 2;
        unsigned char adcr_clk_delay_0 : 1;
        unsigned char adcr_clk_delay_1 : 1;
        unsigned char adcr_clk_delay_2 : 1;
    } reg;
} PMIC_CODEC_ANA_RW12_UNION;
#endif
#define PMIC_CODEC_ANA_RW12_adcr_dwa_bps_START (0)
#define PMIC_CODEC_ANA_RW12_adcr_dwa_bps_END (0)
#define PMIC_CODEC_ANA_RW12_adcr_flstn_START (1)
#define PMIC_CODEC_ANA_RW12_adcr_flstn_END (2)
#define PMIC_CODEC_ANA_RW12_adcr_dac_bias_START (3)
#define PMIC_CODEC_ANA_RW12_adcr_dac_bias_END (4)
#define PMIC_CODEC_ANA_RW12_adcr_clk_delay_0_START (5)
#define PMIC_CODEC_ANA_RW12_adcr_clk_delay_0_END (5)
#define PMIC_CODEC_ANA_RW12_adcr_clk_delay_1_START (6)
#define PMIC_CODEC_ANA_RW12_adcr_clk_delay_1_END (6)
#define PMIC_CODEC_ANA_RW12_adcr_clk_delay_2_START (7)
#define PMIC_CODEC_ANA_RW12_adcr_clk_delay_2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char mixout_hsr : 4;
        unsigned char mixout_hsl : 4;
    } reg;
} PMIC_CODEC_ANA_RW13_UNION;
#endif
#define PMIC_CODEC_ANA_RW13_mixout_hsr_START (0)
#define PMIC_CODEC_ANA_RW13_mixout_hsr_END (3)
#define PMIC_CODEC_ANA_RW13_mixout_hsl_START (4)
#define PMIC_CODEC_ANA_RW13_mixout_hsl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cr_mixf : 4;
        unsigned char mixout_ear : 4;
    } reg;
} PMIC_CODEC_ANA_RW14_UNION;
#endif
#define PMIC_CODEC_ANA_RW14_cr_mixf_START (0)
#define PMIC_CODEC_ANA_RW14_cr_mixf_END (3)
#define PMIC_CODEC_ANA_RW14_mixout_ear_START (4)
#define PMIC_CODEC_ANA_RW14_mixout_ear_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cp_dtc_3 : 2;
        unsigned char cp_dtc_2 : 2;
        unsigned char dacr_ictrl : 2;
        unsigned char dacl_ictrl : 2;
    } reg;
} PMIC_CODEC_ANA_RW15_UNION;
#endif
#define PMIC_CODEC_ANA_RW15_cp_dtc_3_START (0)
#define PMIC_CODEC_ANA_RW15_cp_dtc_3_END (1)
#define PMIC_CODEC_ANA_RW15_cp_dtc_2_START (2)
#define PMIC_CODEC_ANA_RW15_cp_dtc_2_END (3)
#define PMIC_CODEC_ANA_RW15_dacr_ictrl_START (4)
#define PMIC_CODEC_ANA_RW15_dacr_ictrl_END (5)
#define PMIC_CODEC_ANA_RW15_dacl_ictrl_START (6)
#define PMIC_CODEC_ANA_RW15_dacl_ictrl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cp_dr_ctrl : 2;
        unsigned char classg_ref : 2;
        unsigned char cp_mode : 2;
        unsigned char en_cp_mode : 1;
        unsigned char hp_soft_en : 1;
    } reg;
} PMIC_CODEC_ANA_RW16_UNION;
#endif
#define PMIC_CODEC_ANA_RW16_cp_dr_ctrl_START (0)
#define PMIC_CODEC_ANA_RW16_cp_dr_ctrl_END (1)
#define PMIC_CODEC_ANA_RW16_classg_ref_START (2)
#define PMIC_CODEC_ANA_RW16_classg_ref_END (3)
#define PMIC_CODEC_ANA_RW16_cp_mode_START (4)
#define PMIC_CODEC_ANA_RW16_cp_mode_END (5)
#define PMIC_CODEC_ANA_RW16_en_cp_mode_START (6)
#define PMIC_CODEC_ANA_RW16_en_cp_mode_END (6)
#define PMIC_CODEC_ANA_RW16_hp_soft_en_START (7)
#define PMIC_CODEC_ANA_RW16_hp_soft_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hsl_min_gain : 1;
        unsigned char hsl_gain : 4;
        unsigned char hsl_mute : 1;
        unsigned char HS_LIT_LP_L : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_CODEC_ANA_RW17_UNION;
#endif
#define PMIC_CODEC_ANA_RW17_hsl_min_gain_START (0)
#define PMIC_CODEC_ANA_RW17_hsl_min_gain_END (0)
#define PMIC_CODEC_ANA_RW17_hsl_gain_START (1)
#define PMIC_CODEC_ANA_RW17_hsl_gain_END (4)
#define PMIC_CODEC_ANA_RW17_hsl_mute_START (5)
#define PMIC_CODEC_ANA_RW17_hsl_mute_END (5)
#define PMIC_CODEC_ANA_RW17_HS_LIT_LP_L_START (6)
#define PMIC_CODEC_ANA_RW17_HS_LIT_LP_L_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hsr_min_gain : 1;
        unsigned char hsr_gain : 4;
        unsigned char hsr_mute : 1;
        unsigned char HS_LIT_LP_R : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_CODEC_ANA_RW18_UNION;
#endif
#define PMIC_CODEC_ANA_RW18_hsr_min_gain_START (0)
#define PMIC_CODEC_ANA_RW18_hsr_min_gain_END (0)
#define PMIC_CODEC_ANA_RW18_hsr_gain_START (1)
#define PMIC_CODEC_ANA_RW18_hsr_gain_END (4)
#define PMIC_CODEC_ANA_RW18_hsr_mute_START (5)
#define PMIC_CODEC_ANA_RW18_hsr_mute_END (5)
#define PMIC_CODEC_ANA_RW18_HS_LIT_LP_R_START (6)
#define PMIC_CODEC_ANA_RW18_HS_LIT_LP_R_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ear_cm_ctrl : 1;
        unsigned char ear_gain : 4;
        unsigned char ear_mute : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_CODEC_ANA_RW19_UNION;
#endif
#define PMIC_CODEC_ANA_RW19_ear_cm_ctrl_START (0)
#define PMIC_CODEC_ANA_RW19_ear_cm_ctrl_END (0)
#define PMIC_CODEC_ANA_RW19_ear_gain_START (1)
#define PMIC_CODEC_ANA_RW19_ear_gain_END (4)
#define PMIC_CODEC_ANA_RW19_ear_mute_START (5)
#define PMIC_CODEC_ANA_RW19_ear_mute_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pop_ramp_ct_2_0 : 3;
        unsigned char pop_ramp_ct_3 : 1;
        unsigned char pop_ramp_ct_4 : 1;
        unsigned char clk_dl : 3;
    } reg;
} PMIC_CODEC_ANA_RW20_UNION;
#endif
#define PMIC_CODEC_ANA_RW20_pop_ramp_ct_2_0_START (0)
#define PMIC_CODEC_ANA_RW20_pop_ramp_ct_2_0_END (2)
#define PMIC_CODEC_ANA_RW20_pop_ramp_ct_3_START (3)
#define PMIC_CODEC_ANA_RW20_pop_ramp_ct_3_END (3)
#define PMIC_CODEC_ANA_RW20_pop_ramp_ct_4_START (4)
#define PMIC_CODEC_ANA_RW20_pop_ramp_ct_4_END (4)
#define PMIC_CODEC_ANA_RW20_clk_dl_START (5)
#define PMIC_CODEC_ANA_RW20_clk_dl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rst_n_r : 1;
        unsigned char rst_n_l : 1;
        unsigned char pop_dis : 1;
        unsigned char clk_sel : 1;
        unsigned char pdd_en : 1;
        unsigned char pop_new_bypass : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_CODEC_ANA_RW21_UNION;
#endif
#define PMIC_CODEC_ANA_RW21_rst_n_r_START (0)
#define PMIC_CODEC_ANA_RW21_rst_n_r_END (0)
#define PMIC_CODEC_ANA_RW21_rst_n_l_START (1)
#define PMIC_CODEC_ANA_RW21_rst_n_l_END (1)
#define PMIC_CODEC_ANA_RW21_pop_dis_START (2)
#define PMIC_CODEC_ANA_RW21_pop_dis_END (2)
#define PMIC_CODEC_ANA_RW21_clk_sel_START (3)
#define PMIC_CODEC_ANA_RW21_clk_sel_END (3)
#define PMIC_CODEC_ANA_RW21_pdd_en_START (4)
#define PMIC_CODEC_ANA_RW21_pdd_en_END (4)
#define PMIC_CODEC_ANA_RW21_pop_new_bypass_START (5)
#define PMIC_CODEC_ANA_RW21_pop_new_bypass_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char micb1_adj : 3;
        unsigned char hsmicb_adj : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_CODEC_ANA_RW22_UNION;
#endif
#define PMIC_CODEC_ANA_RW22_micb1_adj_START (0)
#define PMIC_CODEC_ANA_RW22_micb1_adj_END (2)
#define PMIC_CODEC_ANA_RW22_hsmicb_adj_START (3)
#define PMIC_CODEC_ANA_RW22_hsmicb_adj_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char mbhd_eco_en : 1;
        unsigned char micb1_dschg : 1;
        unsigned char hsmicb_dschg : 1;
        unsigned char hsd_ctrl_0 : 1;
        unsigned char hsd_ctrl_1 : 1;
        unsigned char hsd_ctrl_2 : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_CODEC_ANA_RW23_UNION;
#endif
#define PMIC_CODEC_ANA_RW23_mbhd_eco_en_START (0)
#define PMIC_CODEC_ANA_RW23_mbhd_eco_en_END (0)
#define PMIC_CODEC_ANA_RW23_micb1_dschg_START (1)
#define PMIC_CODEC_ANA_RW23_micb1_dschg_END (1)
#define PMIC_CODEC_ANA_RW23_hsmicb_dschg_START (2)
#define PMIC_CODEC_ANA_RW23_hsmicb_dschg_END (2)
#define PMIC_CODEC_ANA_RW23_hsd_ctrl_0_START (3)
#define PMIC_CODEC_ANA_RW23_hsd_ctrl_0_END (3)
#define PMIC_CODEC_ANA_RW23_hsd_ctrl_1_START (4)
#define PMIC_CODEC_ANA_RW23_hsd_ctrl_1_END (4)
#define PMIC_CODEC_ANA_RW23_hsd_ctrl_2_START (5)
#define PMIC_CODEC_ANA_RW23_hsd_ctrl_2_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char mbhd_vref_ctrl_1_0 : 2;
        unsigned char mbhd_vref_ctrl_3_2 : 2;
        unsigned char mbhd_vref_ctrl_6_4 : 3;
        unsigned char mbhd_vref_pd : 1;
    } reg;
} PMIC_CODEC_ANA_RW24_UNION;
#endif
#define PMIC_CODEC_ANA_RW24_mbhd_vref_ctrl_1_0_START (0)
#define PMIC_CODEC_ANA_RW24_mbhd_vref_ctrl_1_0_END (1)
#define PMIC_CODEC_ANA_RW24_mbhd_vref_ctrl_3_2_START (2)
#define PMIC_CODEC_ANA_RW24_mbhd_vref_ctrl_3_2_END (3)
#define PMIC_CODEC_ANA_RW24_mbhd_vref_ctrl_6_4_START (4)
#define PMIC_CODEC_ANA_RW24_mbhd_vref_ctrl_6_4_END (6)
#define PMIC_CODEC_ANA_RW24_mbhd_vref_pd_START (7)
#define PMIC_CODEC_ANA_RW24_mbhd_vref_pd_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cp_det_sel : 2;
        unsigned char cp_det_clk_pd : 1;
        unsigned char cp_clk_pd : 1;
        unsigned char cp_sel : 3;
        unsigned char hs_sys_clk_pd : 1;
    } reg;
} PMIC_CODEC_ANA_RW25_UNION;
#endif
#define PMIC_CODEC_ANA_RW25_cp_det_sel_START (0)
#define PMIC_CODEC_ANA_RW25_cp_det_sel_END (1)
#define PMIC_CODEC_ANA_RW25_cp_det_clk_pd_START (2)
#define PMIC_CODEC_ANA_RW25_cp_det_clk_pd_END (2)
#define PMIC_CODEC_ANA_RW25_cp_clk_pd_START (3)
#define PMIC_CODEC_ANA_RW25_cp_clk_pd_END (3)
#define PMIC_CODEC_ANA_RW25_cp_sel_START (4)
#define PMIC_CODEC_ANA_RW25_cp_sel_END (6)
#define PMIC_CODEC_ANA_RW25_hs_sys_clk_pd_START (7)
#define PMIC_CODEC_ANA_RW25_hs_sys_clk_pd_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dac_chop_clk_sel : 2;
        unsigned char adc_chop_clk_sel : 2;
        unsigned char dac_clk_sys_edge_0 : 1;
        unsigned char dac_clk_sys_edge_1 : 1;
        unsigned char adc_clk_sys_edge_0 : 1;
        unsigned char adc_clk_sys_edge_1 : 1;
    } reg;
} PMIC_CODEC_ANA_RW26_UNION;
#endif
#define PMIC_CODEC_ANA_RW26_dac_chop_clk_sel_START (0)
#define PMIC_CODEC_ANA_RW26_dac_chop_clk_sel_END (1)
#define PMIC_CODEC_ANA_RW26_adc_chop_clk_sel_START (2)
#define PMIC_CODEC_ANA_RW26_adc_chop_clk_sel_END (3)
#define PMIC_CODEC_ANA_RW26_dac_clk_sys_edge_0_START (4)
#define PMIC_CODEC_ANA_RW26_dac_clk_sys_edge_0_END (4)
#define PMIC_CODEC_ANA_RW26_dac_clk_sys_edge_1_START (5)
#define PMIC_CODEC_ANA_RW26_dac_clk_sys_edge_1_END (5)
#define PMIC_CODEC_ANA_RW26_adc_clk_sys_edge_0_START (6)
#define PMIC_CODEC_ANA_RW26_adc_clk_sys_edge_0_END (6)
#define PMIC_CODEC_ANA_RW26_adc_clk_sys_edge_1_START (7)
#define PMIC_CODEC_ANA_RW26_adc_clk_sys_edge_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ctcm_chop_bps : 1;
        unsigned char micpga_chop_bps : 1;
        unsigned char linein_chop_bps : 1;
        unsigned char adc_sdm_chop_bps : 1;
        unsigned char rx_chop_bps : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_CODEC_ANA_RW27_UNION;
#endif
#define PMIC_CODEC_ANA_RW27_ctcm_chop_bps_START (0)
#define PMIC_CODEC_ANA_RW27_ctcm_chop_bps_END (0)
#define PMIC_CODEC_ANA_RW27_micpga_chop_bps_START (1)
#define PMIC_CODEC_ANA_RW27_micpga_chop_bps_END (1)
#define PMIC_CODEC_ANA_RW27_linein_chop_bps_START (2)
#define PMIC_CODEC_ANA_RW27_linein_chop_bps_END (2)
#define PMIC_CODEC_ANA_RW27_adc_sdm_chop_bps_START (3)
#define PMIC_CODEC_ANA_RW27_adc_sdm_chop_bps_END (3)
#define PMIC_CODEC_ANA_RW27_rx_chop_bps_START (4)
#define PMIC_CODEC_ANA_RW27_rx_chop_bps_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hs_chop_bps : 1;
        unsigned char ear_cld_chop_bps : 1;
        unsigned char dac_chop_bps : 1;
        unsigned char tx_chop_bps : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_CODEC_ANA_RW28_UNION;
#endif
#define PMIC_CODEC_ANA_RW28_hs_chop_bps_START (0)
#define PMIC_CODEC_ANA_RW28_hs_chop_bps_END (0)
#define PMIC_CODEC_ANA_RW28_ear_cld_chop_bps_START (1)
#define PMIC_CODEC_ANA_RW28_ear_cld_chop_bps_END (1)
#define PMIC_CODEC_ANA_RW28_dac_chop_bps_START (2)
#define PMIC_CODEC_ANA_RW28_dac_chop_bps_END (2)
#define PMIC_CODEC_ANA_RW28_tx_chop_bps_START (3)
#define PMIC_CODEC_ANA_RW28_tx_chop_bps_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ib05_mixer_hp : 3;
        unsigned char ib05_hp : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_CODEC_ANA_RW29_UNION;
#endif
#define PMIC_CODEC_ANA_RW29_ib05_mixer_hp_START (0)
#define PMIC_CODEC_ANA_RW29_ib05_mixer_hp_END (2)
#define PMIC_CODEC_ANA_RW29_ib05_hp_START (3)
#define PMIC_CODEC_ANA_RW29_ib05_hp_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ib05_adc_opa1 : 3;
        unsigned char ib05_dac : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_CODEC_ANA_RW30_UNION;
#endif
#define PMIC_CODEC_ANA_RW30_ib05_adc_opa1_START (0)
#define PMIC_CODEC_ANA_RW30_ib05_adc_opa1_END (2)
#define PMIC_CODEC_ANA_RW30_ib05_dac_START (3)
#define PMIC_CODEC_ANA_RW30_ib05_dac_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ib05_adc_comp : 3;
        unsigned char ib05_adc_opa2 : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_CODEC_ANA_RW31_UNION;
#endif
#define PMIC_CODEC_ANA_RW31_ib05_adc_comp_START (0)
#define PMIC_CODEC_ANA_RW31_ib05_adc_comp_END (2)
#define PMIC_CODEC_ANA_RW31_ib05_adc_opa2_START (3)
#define PMIC_CODEC_ANA_RW31_ib05_adc_opa2_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ib05_micbias : 3;
        unsigned char ib05_adc_linein : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_CODEC_ANA_RW32_UNION;
#endif
#define PMIC_CODEC_ANA_RW32_ib05_micbias_START (0)
#define PMIC_CODEC_ANA_RW32_ib05_micbias_END (2)
#define PMIC_CODEC_ANA_RW32_ib05_adc_linein_START (3)
#define PMIC_CODEC_ANA_RW32_ib05_adc_linein_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ib05_auxmic : 3;
        unsigned char ib05_rev : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_CODEC_ANA_RW33_UNION;
#endif
#define PMIC_CODEC_ANA_RW33_ib05_auxmic_START (0)
#define PMIC_CODEC_ANA_RW33_ib05_auxmic_END (2)
#define PMIC_CODEC_ANA_RW33_ib05_rev_START (3)
#define PMIC_CODEC_ANA_RW33_ib05_rev_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ib05_mixer_ear : 3;
        unsigned char ib05_mainmic : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_CODEC_ANA_RW34_UNION;
#endif
#define PMIC_CODEC_ANA_RW34_ib05_mixer_ear_START (0)
#define PMIC_CODEC_ANA_RW34_ib05_mixer_ear_END (2)
#define PMIC_CODEC_ANA_RW34_ib05_mainmic_START (3)
#define PMIC_CODEC_ANA_RW34_ib05_mainmic_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_bias_adj : 1;
        unsigned char ib05_ear : 3;
        unsigned char reserved : 4;
    } reg;
} PMIC_CODEC_ANA_RW35_UNION;
#endif
#define PMIC_CODEC_ANA_RW35_codec_bias_adj_START (0)
#define PMIC_CODEC_ANA_RW35_codec_bias_adj_END (0)
#define PMIC_CODEC_ANA_RW35_ib05_ear_START (1)
#define PMIC_CODEC_ANA_RW35_ib05_ear_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hpl_pop_res_c : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_CODEC_ANA_RW36_UNION;
#endif
#define PMIC_CODEC_ANA_RW36_hpl_pop_res_c_START (0)
#define PMIC_CODEC_ANA_RW36_hpl_pop_res_c_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cp_det_pd : 1;
        unsigned char cp_pd : 1;
        unsigned char hp_bias_pd : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_CODEC_ANA_RW37_UNION;
#endif
#define PMIC_CODEC_ANA_RW37_cp_det_pd_START (0)
#define PMIC_CODEC_ANA_RW37_cp_det_pd_END (0)
#define PMIC_CODEC_ANA_RW37_cp_pd_START (1)
#define PMIC_CODEC_ANA_RW37_cp_pd_END (1)
#define PMIC_CODEC_ANA_RW37_hp_bias_pd_START (2)
#define PMIC_CODEC_ANA_RW37_hp_bias_pd_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dacr_pd : 1;
        unsigned char dacl_pd : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_CODEC_ANA_RW38_UNION;
#endif
#define PMIC_CODEC_ANA_RW38_dacr_pd_START (0)
#define PMIC_CODEC_ANA_RW38_dacr_pd_END (0)
#define PMIC_CODEC_ANA_RW38_dacl_pd_START (1)
#define PMIC_CODEC_ANA_RW38_dacl_pd_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ear_pd : 1;
        unsigned char mixout_ear_pd : 1;
        unsigned char ear_vref_en : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_CODEC_ANA_RW39_UNION;
#endif
#define PMIC_CODEC_ANA_RW39_ear_pd_START (0)
#define PMIC_CODEC_ANA_RW39_ear_pd_END (0)
#define PMIC_CODEC_ANA_RW39_mixout_ear_pd_START (1)
#define PMIC_CODEC_ANA_RW39_mixout_ear_pd_END (1)
#define PMIC_CODEC_ANA_RW39_ear_vref_en_START (2)
#define PMIC_CODEC_ANA_RW39_ear_vref_en_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hpr_pop_res_c : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_CODEC_ANA_RW40_UNION;
#endif
#define PMIC_CODEC_ANA_RW40_hpr_pop_res_c_START (0)
#define PMIC_CODEC_ANA_RW40_hpr_pop_res_c_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_ro01 : 8;
    } reg;
} PMIC_CODEC_ANA_RO01_UNION;
#endif
#define PMIC_CODEC_ANA_RO01_codec_ana_ro01_START (0)
#define PMIC_CODEC_ANA_RO01_codec_ana_ro01_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_ro02 : 8;
    } reg;
} PMIC_CODEC_ANA_RO02_UNION;
#endif
#define PMIC_CODEC_ANA_RO02_codec_ana_ro02_START (0)
#define PMIC_CODEC_ANA_RO02_codec_ana_ro02_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char osc_bak_adj : 4;
        unsigned char osc_bak_ins_sel : 1;
        unsigned char osc_bak_pd : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_CODEC_ANA_RW41_UNION;
#endif
#define PMIC_CODEC_ANA_RW41_osc_bak_adj_START (0)
#define PMIC_CODEC_ANA_RW41_osc_bak_adj_END (3)
#define PMIC_CODEC_ANA_RW41_osc_bak_ins_sel_START (4)
#define PMIC_CODEC_ANA_RW41_osc_bak_ins_sel_END (4)
#define PMIC_CODEC_ANA_RW41_osc_bak_pd_START (5)
#define PMIC_CODEC_ANA_RW41_osc_bak_pd_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char clk_cp_src_sel : 1;
        unsigned char clk_txchop_src_sel : 1;
        unsigned char clk_rxchop_src_sel : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_CODEC_ANA_RW42_UNION;
#endif
#define PMIC_CODEC_ANA_RW42_clk_cp_src_sel_START (0)
#define PMIC_CODEC_ANA_RW42_clk_cp_src_sel_END (0)
#define PMIC_CODEC_ANA_RW42_clk_txchop_src_sel_START (1)
#define PMIC_CODEC_ANA_RW42_clk_txchop_src_sel_END (1)
#define PMIC_CODEC_ANA_RW42_clk_rxchop_src_sel_START (2)
#define PMIC_CODEC_ANA_RW42_clk_rxchop_src_sel_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char push_adj_n : 3;
        unsigned char reserved_0 : 1;
        unsigned char push_adj_p : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_CODEC_ANA_RW43_UNION;
#endif
#define PMIC_CODEC_ANA_RW43_push_adj_n_START (0)
#define PMIC_CODEC_ANA_RW43_push_adj_n_END (2)
#define PMIC_CODEC_ANA_RW43_push_adj_p_START (4)
#define PMIC_CODEC_ANA_RW43_push_adj_p_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 8;
    } reg;
} PMIC_CODEC_ANA_RW44_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 8;
    } reg;
} PMIC_CODEC_ANA_RW45_UNION;
#endif
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
