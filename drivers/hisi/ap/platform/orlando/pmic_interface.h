#ifndef __PMIC_INTERFACE_H__
#define __PMIC_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_VERSION0_ADDR(base) ((base) + (0x0000UL))
#define PMIC_VERSION1_ADDR(base) ((base) + (0x0001UL))
#define PMIC_VERSION2_ADDR(base) ((base) + (0x0002UL))
#define PMIC_VERSION3_ADDR(base) ((base) + (0x0003UL))
#define PMIC_VERSION4_ADDR(base) ((base) + (0x0004UL))
#define PMIC_VERSION5_ADDR(base) ((base) + (0x0005UL))
#define PMIC_STATUS0_ADDR(base) ((base) + (0x0006UL))
#define PMIC_STATUS1_ADDR(base) ((base) + (0x0007UL))
#define PMIC_STATUS2_ADDR(base) ((base) + (0x0008UL))
#define PMIC_BUCK0_ONOFF_ECO_ADDR(base) ((base) + (0x000AUL))
#define PMIC_BUCK1_ONOFF_ECO_ADDR(base) ((base) + (0x000BUL))
#define PMIC_BUCK2_ONOFF_ECO_ADDR(base) ((base) + (0x000CUL))
#define PMIC_BUCK3_ONOFF_ECO_ADDR(base) ((base) + (0x000DUL))
#define PMIC_BUCK4_ONOFF_ECO_ADDR(base) ((base) + (0x000EUL))
#define PMIC_BUCK5_ONOFF_ECO_ADDR(base) ((base) + (0x000FUL))
#define PMIC_LDO0_1_ONOFF_ADDR(base) ((base) + (0x0010UL))
#define PMIC_LDO0_2_ONOFF_ECO_ADDR(base) ((base) + (0x0011UL))
#define PMIC_LDO1_ONOFF_ADDR(base) ((base) + (0x0012UL))
#define PMIC_LDO2_ONOFF_ECO_ADDR(base) ((base) + (0x0013UL))
#define PMIC_LDO3_ONOFF_ADDR(base) ((base) + (0x0014UL))
#define PMIC_LDO4_ONOFF_ECO_ADDR(base) ((base) + (0x0015UL))
#define PMIC_LDO5_ONOFF_ECO_ADDR(base) ((base) + (0x0016UL))
#define PMIC_LDO6_ONOFF_ECO_ADDR(base) ((base) + (0x0017UL))
#define PMIC_LDO8_ONOFF_ECO_ADDR(base) ((base) + (0x0018UL))
#define PMIC_LDO9_ONOFF_ECO_ADDR(base) ((base) + (0x0019UL))
#define PMIC_LDO11_ONOFF_ECO_ADDR(base) ((base) + (0x001AUL))
#define PMIC_LDO12_ONOFF_ECO_ADDR(base) ((base) + (0x001BUL))
#define PMIC_LDO13_ONOFF_ADDR(base) ((base) + (0x001CUL))
#define PMIC_LDO14_ONOFF_ADDR(base) ((base) + (0x001DUL))
#define PMIC_LDO15_ONOFF_ECO_ADDR(base) ((base) + (0x001EUL))
#define PMIC_LDO16_ONOFF_ECO_ADDR(base) ((base) + (0x001FUL))
#define PMIC_LDO17_ONOFF_ECO_ADDR(base) ((base) + (0x0020UL))
#define PMIC_LDO18_ONOFF_ECO_ADDR(base) ((base) + (0x0021UL))
#define PMIC_LDO19_ONOFF_ADDR(base) ((base) + (0x0022UL))
#define PMIC_LDO20_ONOFF_ADDR(base) ((base) + (0x0023UL))
#define PMIC_LDO21_ONOFF_ADDR(base) ((base) + (0x0024UL))
#define PMIC_LDO22_ONOFF_ADDR(base) ((base) + (0x0025UL))
#define PMIC_LDO23_ONOFF_ECO_ADDR(base) ((base) + (0x0026UL))
#define PMIC_LDO24_ONOFF_ECO_ADDR(base) ((base) + (0x0027UL))
#define PMIC_LDO25_ONOFF_ADDR(base) ((base) + (0x0028UL))
#define PMIC_LDO26_ONOFF_ECO_ADDR(base) ((base) + (0x0029UL))
#define PMIC_LDO27_ONOFF_ECO_ADDR(base) ((base) + (0x002AUL))
#define PMIC_LDO28_ONOFF_ADDR(base) ((base) + (0x002BUL))
#define PMIC_LDO29_ONOFF_ECO_ADDR(base) ((base) + (0x002CUL))
#define PMIC_LDO30_1_ONOFF_ADDR(base) ((base) + (0x002DUL))
#define PMIC_LDO30_2_ONOFF_ECO_ADDR(base) ((base) + (0x002EUL))
#define PMIC_LDO31_ONOFF_ADDR(base) ((base) + (0x002FUL))
#define PMIC_LDO32_ONOFF_ADDR(base) ((base) + (0x0030UL))
#define PMIC_LDO33_ONOFF1_ADDR(base) ((base) + (0x0031UL))
#define PMIC_LDO33_ONOFF2_ADDR(base) ((base) + (0x0032UL))
#define PMIC_LDO34_ONOFF_ECO_ADDR(base) ((base) + (0x0033UL))
#define PMIC_PMUH_ONOFF_ECO_ADDR(base) ((base) + (0x0034UL))
#define PMIC_LDO37_ONOFF_ECO_ADDR(base) ((base) + (0x0035UL))
#define PMIC_LDO38_ONOFF_ECO_ADDR(base) ((base) + (0x0036UL))
#define PMIC_LDO39_ONOFF_ECO_ADDR(base) ((base) + (0x0037UL))
#define PMIC_LDO40_ONOFF_ADDR(base) ((base) + (0x0038UL))
#define PMIC_LDO_PMUA_ECO_ADDR(base) ((base) + (0x0039UL))
#define PMIC_CLK_ABB_EN_ADDR(base) ((base) + (0x003DUL))
#define PMIC_CLK_WIFI_EN_ADDR(base) ((base) + (0x003EUL))
#define PMIC_CLK_NFC_EN_ADDR(base) ((base) + (0x003FUL))
#define PMIC_CLK_RF_EN_ADDR(base) ((base) + (0x0040UL))
#define PMIC_CLK_SYS_EN_ADDR(base) ((base) + (0x0041UL))
#define PMIC_CLK_CODEC_EN_ADDR(base) ((base) + (0x0042UL))
#define PMIC_CLK_UFS_EN_ADDR(base) ((base) + (0x0043UL))
#define PMIC_OSC32K_GPS_ONOFF_CTRL_ADDR(base) ((base) + (0x0044UL))
#define PMIC_OSC32K_BT_ONOFF_CTRL_ADDR(base) ((base) + (0x0045UL))
#define PMIC_OSC32K_SYS_ONOFF_CTRL_ADDR(base) ((base) + (0x0046UL))
#define PMIC_BUCK0_VSET_ADDR(base) ((base) + (0x0048UL))
#define PMIC_BUCK0_VSET_ECO_ADDR(base) ((base) + (0x0049UL))
#define PMIC_BUCK1_VSET_ADDR(base) ((base) + (0x004AUL))
#define PMIC_BUCK2_VSET_ADDR(base) ((base) + (0x004BUL))
#define PMIC_BUCK2_VSET_ECO_ADDR(base) ((base) + (0x004CUL))
#define PMIC_BUCK3_VSET_ADDR(base) ((base) + (0x004DUL))
#define PMIC_BUCK3_VSET_ECO_ADDR(base) ((base) + (0x004EUL))
#define PMIC_BUCK4_VSET_ADDR(base) ((base) + (0x004FUL))
#define PMIC_BUCK4_VSET_ECO_ADDR(base) ((base) + (0x0050UL))
#define PMIC_BUCK4_VSET_IDLE_ADDR(base) ((base) + (0x0051UL))
#define PMIC_BUCK5_VSET_ADDR(base) ((base) + (0x0052UL))
#define PMIC_LDO0_2_VSET_ADDR(base) ((base) + (0x0053UL))
#define PMIC_LDO0_2_VSET_ECO_ADDR(base) ((base) + (0x0054UL))
#define PMIC_LDO1_VSET_ADDR(base) ((base) + (0x0055UL))
#define PMIC_LDO2_VSET_ADDR(base) ((base) + (0x0056UL))
#define PMIC_LDO2_VSET_ECO_ADDR(base) ((base) + (0x0057UL))
#define PMIC_LDO3_VSET_ADDR(base) ((base) + (0x0058UL))
#define PMIC_LDO4_VSET_ADDR(base) ((base) + (0x0059UL))
#define PMIC_LDO5_VSET_ADDR(base) ((base) + (0x005AUL))
#define PMIC_LDO6_VSET_ADDR(base) ((base) + (0x005BUL))
#define PMIC_LDO8_VSET_ADDR(base) ((base) + (0x005CUL))
#define PMIC_LDO9_VSET_ADDR(base) ((base) + (0x005DUL))
#define PMIC_LDO11_VSET_ADDR(base) ((base) + (0x005EUL))
#define PMIC_LDO12_VSET_ADDR(base) ((base) + (0x005FUL))
#define PMIC_LDO13_VSET_ADDR(base) ((base) + (0x0060UL))
#define PMIC_LDO14_VSET_ADDR(base) ((base) + (0x0061UL))
#define PMIC_LDO15_VSET_ADDR(base) ((base) + (0x0062UL))
#define PMIC_LDO16_VSET_ADDR(base) ((base) + (0x0063UL))
#define PMIC_LDO17_VSET_ADDR(base) ((base) + (0x0064UL))
#define PMIC_LDO18_VSET_ADDR(base) ((base) + (0x0065UL))
#define PMIC_LDO19_VSET_ADDR(base) ((base) + (0x0066UL))
#define PMIC_LDO20_VSET_ADDR(base) ((base) + (0x0067UL))
#define PMIC_LDO21_VSET_ADDR(base) ((base) + (0x0068UL))
#define PMIC_LDO22_VSET_ADDR(base) ((base) + (0x0069UL))
#define PMIC_LDO23_VSET_ADDR(base) ((base) + (0x006AUL))
#define PMIC_LDO24_VSET_ADDR(base) ((base) + (0x006BUL))
#define PMIC_LDO25_VSET_ADDR(base) ((base) + (0x006CUL))
#define PMIC_LDO26_VSET_ADDR(base) ((base) + (0x006DUL))
#define PMIC_LDO27_VSET_ADDR(base) ((base) + (0x006EUL))
#define PMIC_LDO28_VSET_ADDR(base) ((base) + (0x006FUL))
#define PMIC_LDO29_VSET_ADDR(base) ((base) + (0x0070UL))
#define PMIC_LDO30_VSET_ADDR(base) ((base) + (0x0071UL))
#define PMIC_LDO31_VSET_ADDR(base) ((base) + (0x0072UL))
#define PMIC_LDO32_VSET_ADDR(base) ((base) + (0x0073UL))
#define PMIC_LDO33_VSET_ADDR(base) ((base) + (0x0074UL))
#define PMIC_LDO34_VSET_ADDR(base) ((base) + (0x0075UL))
#define PMIC_PMUH_VSET_ADDR(base) ((base) + (0x0076UL))
#define PMIC_LDO37_VSET_ADDR(base) ((base) + (0x0077UL))
#define PMIC_LDO38_VSET_ADDR(base) ((base) + (0x0078UL))
#define PMIC_LDO39_VSET_ADDR(base) ((base) + (0x0079UL))
#define PMIC_LDO40_VSET_ADDR(base) ((base) + (0x007AUL))
#define PMIC_LDO_BUF_VSET_ADDR(base) ((base) + (0x007BUL))
#define PMIC_LDO_PMUA_VSET_ADDR(base) ((base) + (0x007CUL))
#define PMIC_BUCK0_CTRL0_ADDR(base) ((base) + (0x07FUL))
#define PMIC_BUCK0_CTRL1_ADDR(base) ((base) + (0x080UL))
#define PMIC_BUCK0_CTRL2_ADDR(base) ((base) + (0x081UL))
#define PMIC_BUCK0_CTRL3_ADDR(base) ((base) + (0x082UL))
#define PMIC_BUCK0_CTRL4_ADDR(base) ((base) + (0x083UL))
#define PMIC_BUCK0_CTRL5_ADDR(base) ((base) + (0x084UL))
#define PMIC_BUCK0_CTRL6_ADDR(base) ((base) + (0x085UL))
#define PMIC_BUCK0_CTRL7_ADDR(base) ((base) + (0x086UL))
#define PMIC_BUCK0_CTRL8_ADDR(base) ((base) + (0x087UL))
#define PMIC_BUCK0_CTRL9_ADDR(base) ((base) + (0x088UL))
#define PMIC_BUCK0_CTRL10_ADDR(base) ((base) + (0x089UL))
#define PMIC_BUCK0_CTRL11_ADDR(base) ((base) + (0x08AUL))
#define PMIC_BUCK1_CTRL0_ADDR(base) ((base) + (0x08BUL))
#define PMIC_BUCK1_CTRL1_ADDR(base) ((base) + (0x08CUL))
#define PMIC_BUCK1_CTRL2_ADDR(base) ((base) + (0x08DUL))
#define PMIC_BUCK1_CTRL3_ADDR(base) ((base) + (0x08EUL))
#define PMIC_BUCK1_CTRL4_ADDR(base) ((base) + (0x08FUL))
#define PMIC_BUCK1_CTRL5_ADDR(base) ((base) + (0x090UL))
#define PMIC_BUCK1_CTRL6_ADDR(base) ((base) + (0x091UL))
#define PMIC_BUCK1_CTRL7_ADDR(base) ((base) + (0x092UL))
#define PMIC_BUCK1_CTRL8_ADDR(base) ((base) + (0x093UL))
#define PMIC_BUCK1_CTRL9_ADDR(base) ((base) + (0x094UL))
#define PMIC_BUCK1_CTRL10_ADDR(base) ((base) + (0x095UL))
#define PMIC_BUCK2_CTRL0_ADDR(base) ((base) + (0x096UL))
#define PMIC_BUCK2_CTRL1_ADDR(base) ((base) + (0x097UL))
#define PMIC_BUCK2_CTRL2_ADDR(base) ((base) + (0x098UL))
#define PMIC_BUCK2_CTRL3_ADDR(base) ((base) + (0x099UL))
#define PMIC_BUCK2_CTRL4_ADDR(base) ((base) + (0x09AUL))
#define PMIC_BUCK2_CTRL5_ADDR(base) ((base) + (0x09BUL))
#define PMIC_BUCK2_CTRL6_ADDR(base) ((base) + (0x09CUL))
#define PMIC_BUCK2_CTRL7_ADDR(base) ((base) + (0x09DUL))
#define PMIC_BUCK2_CTRL8_ADDR(base) ((base) + (0x09EUL))
#define PMIC_BUCK2_CTRL9_ADDR(base) ((base) + (0x09FUL))
#define PMIC_BUCK2_CTRL10_ADDR(base) ((base) + (0x0A0UL))
#define PMIC_BUCK3_CTRL0_ADDR(base) ((base) + (0x0A1UL))
#define PMIC_BUCK3_CTRL1_ADDR(base) ((base) + (0x0A2UL))
#define PMIC_BUCK3_CTRL2_ADDR(base) ((base) + (0x0A3UL))
#define PMIC_BUCK3_CTRL3_ADDR(base) ((base) + (0x0A4UL))
#define PMIC_BUCK3_CTRL4_ADDR(base) ((base) + (0x0A5UL))
#define PMIC_BUCK3_CTRL5_ADDR(base) ((base) + (0x0A6UL))
#define PMIC_BUCK3_CTRL6_ADDR(base) ((base) + (0x0A7UL))
#define PMIC_BUCK3_CTRL7_ADDR(base) ((base) + (0x0A8UL))
#define PMIC_BUCK3_CTRL8_ADDR(base) ((base) + (0x0A9UL))
#define PMIC_BUCK3_CTRL9_ADDR(base) ((base) + (0x0AAUL))
#define PMIC_BUCK4_CTRL0_ADDR(base) ((base) + (0x0ABUL))
#define PMIC_BUCK4_CTRL1_ADDR(base) ((base) + (0x0ACUL))
#define PMIC_BUCK4_CTRL2_ADDR(base) ((base) + (0x0ADUL))
#define PMIC_BUCK4_CTRL3_ADDR(base) ((base) + (0x0AEUL))
#define PMIC_BUCK4_CTRL4_ADDR(base) ((base) + (0x0AFUL))
#define PMIC_BUCK4_CTRL5_ADDR(base) ((base) + (0x0B0UL))
#define PMIC_BUCK4_CTRL6_ADDR(base) ((base) + (0x0B1UL))
#define PMIC_BUCK4_CTRL7_ADDR(base) ((base) + (0x0B2UL))
#define PMIC_BUCK4_CTRL8_ADDR(base) ((base) + (0x0B3UL))
#define PMIC_BUCK4_CTRL9_ADDR(base) ((base) + (0x0B4UL))
#define PMIC_BUCK4_CTRL10_ADDR(base) ((base) + (0x0B5UL))
#define PMIC_BUCK4_CTRL11_ADDR(base) ((base) + (0x0B6UL))
#define PMIC_BUCK5_CTRL0_ADDR(base) ((base) + (0x0B7UL))
#define PMIC_BUCK5_CTRL1_ADDR(base) ((base) + (0x0B8UL))
#define PMIC_BUCK5_CTRL2_ADDR(base) ((base) + (0x0B9UL))
#define PMIC_BUCK5_CTRL3_ADDR(base) ((base) + (0x0BAUL))
#define PMIC_BUCK5_CTRL4_ADDR(base) ((base) + (0x0BBUL))
#define PMIC_BUCK5_CTRL5_ADDR(base) ((base) + (0x0BCUL))
#define PMIC_BUCK5_CTRL6_ADDR(base) ((base) + (0x0BDUL))
#define PMIC_BUCK5_CTRL7_ADDR(base) ((base) + (0x0BEUL))
#define PMIC_BUCK5_CTRL8_ADDR(base) ((base) + (0x0BFUL))
#define PMIC_BUCK5_CTRL9_ADDR(base) ((base) + (0x0C0UL))
#define PMIC_BUCK5_CTRL10_ADDR(base) ((base) + (0x0C1UL))
#define PMIC_BUCK5_CTRL11_ADDR(base) ((base) + (0x0C2UL))
#define PMIC_BUCK_CTRL0_ADDR(base) ((base) + (0x0C3UL))
#define PMIC_BUCK_CTRL1_ADDR(base) ((base) + (0x0C4UL))
#define PMIC_BUCK_RESERVE0_ADDR(base) ((base) + (0x0C5UL))
#define PMIC_BUCK_RESERVE1_ADDR(base) ((base) + (0x0C6UL))
#define PMIC_BUCK_RESERVE2_ADDR(base) ((base) + (0x0C7UL))
#define PMIC_LDO0_CTRL0_ADDR(base) ((base) + (0x0CCUL))
#define PMIC_LDO0_CTRL1_ADDR(base) ((base) + (0x0CDUL))
#define PMIC_LDO1_CTRL0_ADDR(base) ((base) + (0x0CEUL))
#define PMIC_LDO1_CTRL1_ADDR(base) ((base) + (0x0CFUL))
#define PMIC_LDO1_CTRL2_ADDR(base) ((base) + (0x0D0UL))
#define PMIC_LDO2_CTRL0_ADDR(base) ((base) + (0x0D1UL))
#define PMIC_LDO2_CTRL1_ADDR(base) ((base) + (0x0D2UL))
#define PMIC_LDO3_CTRL0_ADDR(base) ((base) + (0x0D3UL))
#define PMIC_LDO3_CTRL1_ADDR(base) ((base) + (0x0D4UL))
#define PMIC_LDO4_CTRL_ADDR(base) ((base) + (0x0D5UL))
#define PMIC_LDO5_CTRL_ADDR(base) ((base) + (0x0D6UL))
#define PMIC_LDO6_CTRL0_ADDR(base) ((base) + (0x0D7UL))
#define PMIC_LDO6_CTRL1_ADDR(base) ((base) + (0x0D8UL))
#define PMIC_LDO8_CTRL_ADDR(base) ((base) + (0x0D9UL))
#define PMIC_LDO9_CTRL_ADDR(base) ((base) + (0x0DAUL))
#define PMIC_LD11_CTRL_ADDR(base) ((base) + (0x0DBUL))
#define PMIC_LD12_CTRL_ADDR(base) ((base) + (0x0DCUL))
#define PMIC_LD13_CTRL0_ADDR(base) ((base) + (0x0DDUL))
#define PMIC_LD13_CTRL1_ADDR(base) ((base) + (0x0DEUL))
#define PMIC_LDO14_CTRL_ADDR(base) ((base) + (0x0DFUL))
#define PMIC_LDO15_CTRL_ADDR(base) ((base) + (0x0E0UL))
#define PMIC_LDO16_CTRL_ADDR(base) ((base) + (0x0E1UL))
#define PMIC_LDO17_CTRL_ADDR(base) ((base) + (0x0E2UL))
#define PMIC_LDO18_CTRL0_ADDR(base) ((base) + (0x0E3UL))
#define PMIC_LDO19_CTRL0_ADDR(base) ((base) + (0x0E4UL))
#define PMIC_LD19_CTRL1_ADDR(base) ((base) + (0x0E5UL))
#define PMIC_LDO20_CTRL0_ADDR(base) ((base) + (0x0E6UL))
#define PMIC_LDO20_CTRL1_ADDR(base) ((base) + (0x0E7UL))
#define PMIC_LDO20_CTRL2_ADDR(base) ((base) + (0x0E8UL))
#define PMIC_LDO20_CTRL3_ADDR(base) ((base) + (0x0E9UL))
#define PMIC_LDO21_CTRL_ADDR(base) ((base) + (0x0EAUL))
#define PMIC_LDO22_CTRL0_ADDR(base) ((base) + (0x0EBUL))
#define PMIC_LDO22_CTRL1_ADDR(base) ((base) + (0x0ECUL))
#define PMIC_LDO23_CTRL_ADDR(base) ((base) + (0x0EDUL))
#define PMIC_LDO24_CTRL0_ADDR(base) ((base) + (0x0EEUL))
#define PMIC_LDO24_CTRL1_ADDR(base) ((base) + (0x0EFUL))
#define PMIC_LDO25_CTRL0_ADDR(base) ((base) + (0x0F0UL))
#define PMIC_LDO26_CTRL_ADDR(base) ((base) + (0x0F1UL))
#define PMIC_LDO27_CTRL_ADDR(base) ((base) + (0x0F2UL))
#define PMIC_LDO28_CTRL_ADDR(base) ((base) + (0x0F3UL))
#define PMIC_LDO29_CTRL0_ADDR(base) ((base) + (0x0F4UL))
#define PMIC_LDO29_CTRL1_ADDR(base) ((base) + (0x0F5UL))
#define PMIC_LDO30_CTRL0_ADDR(base) ((base) + (0x0F6UL))
#define PMIC_LDO30_2_CTRL1_ADDR(base) ((base) + (0x0F7UL))
#define PMIC_LDO31_CTRL_ADDR(base) ((base) + (0x0F8UL))
#define PMIC_LDO32_CTRL0_ADDR(base) ((base) + (0x0F9UL))
#define PMIC_LDO32_CTRL1_ADDR(base) ((base) + (0x0FAUL))
#define PMIC_LDO32_CTRL2_ADDR(base) ((base) + (0x0FBUL))
#define PMIC_LDO32_CTRL3_ADDR(base) ((base) + (0x0FCUL))
#define PMIC_LDO33_CTRL0_ADDR(base) ((base) + (0x0FDUL))
#define PMIC_LDO34_CTRL0_ADDR(base) ((base) + (0x0FEUL))
#define PMIC_LDO34_CTRL1_ADDR(base) ((base) + (0x0FFUL))
#define PMIC_PMUH_CTRL_ADDR(base) ((base) + (0x100UL))
#define PMIC_LDO37_CTRL0_ADDR(base) ((base) + (0x101UL))
#define PMIC_LDO37_CTRL1_ADDR(base) ((base) + (0x102UL))
#define PMIC_LDO37_CTRL2_ADDR(base) ((base) + (0x103UL))
#define PMIC_LDO37_CTRL3_ADDR(base) ((base) + (0x104UL))
#define PMIC_LDO38_CTRL0_ADDR(base) ((base) + (0x105UL))
#define PMIC_LDO38_CTRL1_ADDR(base) ((base) + (0x106UL))
#define PMIC_LDO38_CTRL2_ADDR(base) ((base) + (0x107UL))
#define PMIC_LDO39_CTRL0_ADDR(base) ((base) + (0x108UL))
#define PMIC_LDO39_CTRL1_ADDR(base) ((base) + (0x109UL))
#define PMIC_LDO39_CTRL2_ADDR(base) ((base) + (0x10AUL))
#define PMIC_LDO39_CTRL3_ADDR(base) ((base) + (0x10BUL))
#define PMIC_LDO40_CTRL0_ADDR(base) ((base) + (0x10CUL))
#define PMIC_LDO40_CTRL1_ADDR(base) ((base) + (0x10DUL))
#define PMIC_LDO40_CTRL2_ADDR(base) ((base) + (0x10EUL))
#define PMIC_LDO40_CTRL3_ADDR(base) ((base) + (0x10FUL))
#define PMIC_LDO_BUF_PMUA_CTRL_ADDR(base) ((base) + (0x110UL))
#define PMIC_LDO_RESERVE0_ADDR(base) ((base) + (0x111UL))
#define PMIC_LDO_RESERVE1_ADDR(base) ((base) + (0x112UL))
#define PMIC_LDO1_CTRL3_ADDR(base) ((base) + (0x113UL))
#define PMIC_BUCK0_3_OCP_CTRL_ADDR(base) ((base) + (0x0118UL))
#define PMIC_BUCK4_5_OCP_CTRL_ADDR(base) ((base) + (0x0119UL))
#define PMIC_LDO0_3_OCP_CTRL_ADDR(base) ((base) + (0x011AUL))
#define PMIC_LDO4_8_OCP_CTRL_ADDR(base) ((base) + (0x011BUL))
#define PMIC_LDO9_13_OCP_CTRL_ADDR(base) ((base) + (0x011CUL))
#define PMIC_LDO14_17_OCP_CTRL_ADDR(base) ((base) + (0x011DUL))
#define PMIC_LDO18_21_OCP_CTRL_ADDR(base) ((base) + (0x011EUL))
#define PMIC_LDO22_25_OCP_CTRL_ADDR(base) ((base) + (0x011FUL))
#define PMIC_LDO26_29_OCP_CTRL_ADDR(base) ((base) + (0x0120UL))
#define PMIC_LDO30_33_OCP_CTRL_ADDR(base) ((base) + (0x0121UL))
#define PMIC_LDO34_39_OCP_CTRL_ADDR(base) ((base) + (0x0122UL))
#define PMIC_LDO_BUFF_SCP_CTRL_ADDR(base) ((base) + (0x0123UL))
#define PMIC_BUCK0_3_SCP_CTRL_ADDR(base) ((base) + (0x0124UL))
#define PMIC_BUCK4_5_SCP_CTRL_ADDR(base) ((base) + (0x0125UL))
#define PMIC_SYS_CTRL_RESERVE_ADDR(base) ((base) + (0x0128UL))
#define PMIC_OCP_DEB_CTRL0_ADDR(base) ((base) + (0x0129UL))
#define PMIC_OCP_DEB_CTRL1_ADDR(base) ((base) + (0x012AUL))
#define PMIC_PWROFF_DEB_CTRL_ADDR(base) ((base) + (0x012BUL))
#define PMIC_VSYS_DROP_DEB_CTRL_ADDR(base) ((base) + (0x012CUL))
#define PMIC_OCP_SCP_ONOFF_ADDR(base) ((base) + (0x012DUL))
#define PMIC_CLK_ABB_CTRL0_ADDR(base) ((base) + (0x0130UL))
#define PMIC_CLK_WIFI_CTRL0_ADDR(base) ((base) + (0x0131UL))
#define PMIC_CLK_NFC_CTRL0_ADDR(base) ((base) + (0x0132UL))
#define PMIC_CLK_RF_CTRL0_ADDR(base) ((base) + (0x0133UL))
#define PMIC_CLK_USB_CTRL_ADDR(base) ((base) + (0x0134UL))
#define PMIC_CLK_CODEC_CTRL_ADDR(base) ((base) + (0x0135UL))
#define PMIC_CLK_UFS_CTRL_ADDR(base) ((base) + (0x0136UL))
#define PMIC_TON_ECO_SHIELD_CTRL_ADDR(base) ((base) + (0x0137UL))
#define PMIC_CLK_TOP_CTRL0_ADDR(base) ((base) + (0x0138UL))
#define PMIC_CLK_TOP_CTRL1_ADDR(base) ((base) + (0x0139UL))
#define PMIC_BG_THSD_CTRL0_ADDR(base) ((base) + (0x013AUL))
#define PMIC_BG_THSD_CTRL1_ADDR(base) ((base) + (0x013BUL))
#define PMIC_BG_TEST0_ADDR(base) ((base) + (0x013CUL))
#define PMIC_BG_TEST1_ADDR(base) ((base) + (0x013DUL))
#define PMIC_HARDWIRE_CTRL0_ADDR(base) ((base) + (0x0140UL))
#define PMIC_HARDWIRE_CTRL1_ADDR(base) ((base) + (0x0141UL))
#define PMIC_LOW_POWER_CFG_ADDR(base) ((base) + (0x0142UL))
#define PMIC_PERI_CTRL0_ADDR(base) ((base) + (0x0143UL))
#define PMIC_PERI_CTRL1_ADDR(base) ((base) + (0x0144UL))
#define PMIC_PERI_CTRL2_ADDR(base) ((base) + (0x0145UL))
#define PMIC_PERI_CTRL3_ADDR(base) ((base) + (0x0146UL))
#define PMIC_PERI_CTRL4_ADDR(base) ((base) + (0x0147UL))
#define PMIC_PERI_CTRL5_ADDR(base) ((base) + (0x0148UL))
#define PMIC_PERI_CNT_CFG_ADDR(base) ((base) + (0x0149UL))
#define PMIC_PERI_CTRL6_ADDR(base) ((base) + (0x014AUL))
#define PMIC_PERI_CTRL7_ADDR(base) ((base) + (0x014BUL))
#define PMIC_PERI_CTRL8_ADDR(base) ((base) + (0x014CUL))
#define PMIC_PERI_CTRL9_ADDR(base) ((base) + (0x014DUL))
#define PMIC_PERI_CTRL10_ADDR(base) ((base) + (0x014EUL))
#define PMIC_PERI_CTRL11_ADDR(base) ((base) + (0x014FUL))
#define PMIC_PERI_CTRL12_ADDR(base) ((base) + (0x0150UL))
#define PMIC_PERI_CTRL13_ADDR(base) ((base) + (0x0151UL))
#define PMIC_PERI_CTRL14_ADDR(base) ((base) + (0x0152UL))
#define PMIC_PERI_CTRL15_ADDR(base) ((base) + (0x0153UL))
#define PMIC_PERI_CTRL16_ADDR(base) ((base) + (0x0154UL))
#define PMIC_PERI_CTRL17_ADDR(base) ((base) + (0x0155UL))
#define PMIC_PERI_CTRL18_ADDR(base) ((base) + (0x0156UL))
#define PMIC_PERI_CTRL19_ADDR(base) ((base) + (0x0157UL))
#define PMIC_PERI_CTRL20_ADDR(base) ((base) + (0x0158UL))
#define PMIC_PERI_CTRL21_ADDR(base) ((base) + (0x0159UL))
#define PMIC_WIFI_CTRL_ADDR(base) ((base) + (0x015BUL))
#define PMIC_PERI_VSET_CTRL_ADDR(base) ((base) + (0x015CUL))
#define PMIC_IDLE_CTRL0_ADDR(base) ((base) + (0x015DUL))
#define PMIC_IDLE_CTRL1_ADDR(base) ((base) + (0x015EUL))
#define PMIC_IDLE_CTRL2_ADDR(base) ((base) + (0x015FUL))
#define PMIC_IDLE_CTRL3_ADDR(base) ((base) + (0x0160UL))
#define PMIC_IDLE_CTRL4_ADDR(base) ((base) + (0x0161UL))
#define PMIC_IDLE_CTRL5_ADDR(base) ((base) + (0x0162UL))
#define PMIC_IDLE_CNT_CFG_ADDR(base) ((base) + (0x0163UL))
#define PMIC_IDLE_CTRL6_ADDR(base) ((base) + (0x0164UL))
#define PMIC_IDLE_CTRL7_ADDR(base) ((base) + (0x0165UL))
#define PMIC_IDLE_CTRL8_ADDR(base) ((base) + (0x0166UL))
#define PMIC_IDLE_CTRL9_ADDR(base) ((base) + (0x0167UL))
#define PMIC_IDLE_CTRL10_ADDR(base) ((base) + (0x0168UL))
#define PMIC_IDLE_CTRL11_ADDR(base) ((base) + (0x0169UL))
#define PMIC_IDLE_CTRL12_ADDR(base) ((base) + (0x016AUL))
#define PMIC_IDLE_CTRL13_ADDR(base) ((base) + (0x016BUL))
#define PMIC_IDLE_CTRL14_ADDR(base) ((base) + (0x016CUL))
#define PMIC_IDLE_CTRL15_ADDR(base) ((base) + (0x016DUL))
#define PMIC_IDLE_CTRL16_ADDR(base) ((base) + (0x016EUL))
#define PMIC_IDLE_CTRL17_ADDR(base) ((base) + (0x016FUL))
#define PMIC_IDLE_CTRL18_ADDR(base) ((base) + (0x0170UL))
#define PMIC_IDLE_CTRL19_ADDR(base) ((base) + (0x0171UL))
#define PMIC_IDLE_CTRL20_ADDR(base) ((base) + (0x0172UL))
#define PMIC_IDLE_CTRL21_ADDR(base) ((base) + (0x0173UL))
#define PMIC_IDLE_VSET_CFG_ADDR(base) ((base) + (0x0174UL))
#define PMIC_COUL_ECO_MASK_ADDR(base) ((base) + (0x0175UL))
#define PMIC_SOH_ECO_MASK_ADDR(base) ((base) + (0x0176UL))
#define PMIC_CLASSD_CTRL0_ADDR(base) ((base) + (0x017AUL))
#define PMIC_CLASSD_CTRL1_ADDR(base) ((base) + (0x017BUL))
#define PMIC_CLASSD_CTRL2_ADDR(base) ((base) + (0x017CUL))
#define PMIC_CLASSD_CTRL3_ADDR(base) ((base) + (0x017DUL))
#define PMIC_PMU_SOFT_RST_ADDR(base) ((base) + (0x017EUL))
#define PMIC_LOCK_ADDR(base) ((base) + (0x017FUL))
#define PMIC_DR_EN_MODE_345_ADDR(base) ((base) + (0x0180UL))
#define PMIC_FLASH_PERIOD_DR345_ADDR(base) ((base) + (0x0181UL))
#define PMIC_FLASH_ON_DR345_ADDR(base) ((base) + (0x0182UL))
#define PMIC_DR_MODE_SEL_ADDR(base) ((base) + (0x0183UL))
#define PMIC_DR_1_2_CTRL0_ADDR(base) ((base) + (0x0184UL))
#define PMIC_DR_1_2_CTRL1_ADDR(base) ((base) + (0x0185UL))
#define PMIC_DR1_ISET_ADDR(base) ((base) + (0x0186UL))
#define PMIC_DR2_ISET_ADDR(base) ((base) + (0x0187UL))
#define PMIC_DR_LED_CTRL_ADDR(base) ((base) + (0x0188UL))
#define PMIC_DR_OUT_CTRL_ADDR(base) ((base) + (0x0189UL))
#define PMIC_DR3_ISET_ADDR(base) ((base) + (0x018AUL))
#define PMIC_DR3_START_DEL_ADDR(base) ((base) + (0x018BUL))
#define PMIC_DR4_ISET_ADDR(base) ((base) + (0x018CUL))
#define PMIC_DR4_START_DEL_ADDR(base) ((base) + (0x018DUL))
#define PMIC_DR5_ISET_ADDR(base) ((base) + (0x018EUL))
#define PMIC_DR5_START_DEL_ADDR(base) ((base) + (0x018FUL))
#define PMIC_DR345_TIM_CONF0_ADDR(base) ((base) + (0x0190UL))
#define PMIC_DR345_TIM_CONF1_ADDR(base) ((base) + (0x0191UL))
#define PMIC_DR_CTRLRESERVE8_ADDR(base) ((base) + (0x0192UL))
#define PMIC_DR_CTRLRESERVE9_ADDR(base) ((base) + (0x0193UL))
#define PMIC_OTP_0_ADDR(base) ((base) + (0x0194UL))
#define PMIC_OTP_1_ADDR(base) ((base) + (0x0195UL))
#define PMIC_OTP_CTRL0_ADDR(base) ((base) + (0x0196UL))
#define PMIC_OTP_CTRL1_ADDR(base) ((base) + (0x0197UL))
#define PMIC_OTP_CTRL2_ADDR(base) ((base) + (0x0198UL))
#define PMIC_OTP_WDATA_ADDR(base) ((base) + (0x0199UL))
#define PMIC_OTP_0_W_ADDR(base) ((base) + (0x019AUL))
#define PMIC_OTP_1_W_ADDR(base) ((base) + (0x019BUL))
#define PMIC_OTP_2_W_ADDR(base) ((base) + (0x019CUL))
#define PMIC_OTP_4_W_ADDR(base) ((base) + (0x019DUL))
#define PMIC_OTP_5_W_ADDR(base) ((base) + (0x019EUL))
#define PMIC_OTP_6_W_ADDR(base) ((base) + (0x019FUL))
#define PMIC_OTP_7_W_ADDR(base) ((base) + (0x01A0UL))
#define PMIC_OTP_8_W_ADDR(base) ((base) + (0x01A1UL))
#define PMIC_OTP_9_W_ADDR(base) ((base) + (0x01A2UL))
#define PMIC_OTP_10_W_ADDR(base) ((base) + (0x01A3UL))
#define PMIC_OTP_11_W_ADDR(base) ((base) + (0x01A4UL))
#define PMIC_OTP_12_W_ADDR(base) ((base) + (0x01A5UL))
#define PMIC_OTP_13_W_ADDR(base) ((base) + (0x01A6UL))
#define PMIC_OTP_14_W_ADDR(base) ((base) + (0x01A7UL))
#define PMIC_OTP_15_W_ADDR(base) ((base) + (0x01A8UL))
#define PMIC_OTP_16_W_ADDR(base) ((base) + (0x01A9UL))
#define PMIC_OTP_17_W_ADDR(base) ((base) + (0x01AAUL))
#define PMIC_OTP_18_W_ADDR(base) ((base) + (0x01ABUL))
#define PMIC_OTP_19_W_ADDR(base) ((base) + (0x01ACUL))
#define PMIC_OTP_20_W_ADDR(base) ((base) + (0x01ADUL))
#define PMIC_OTP_21_W_ADDR(base) ((base) + (0x01AEUL))
#define PMIC_OTP_22_W_ADDR(base) ((base) + (0x01AFUL))
#define PMIC_OTP_23_W_ADDR(base) ((base) + (0x01B0UL))
#define PMIC_OTP_24_W_ADDR(base) ((base) + (0x01B1UL))
#define PMIC_OTP_25_W_ADDR(base) ((base) + (0x01B2UL))
#define PMIC_OTP_26_W_ADDR(base) ((base) + (0x01B3UL))
#define PMIC_OTP_27_W_ADDR(base) ((base) + (0x01B4UL))
#define PMIC_OTP_28_W_ADDR(base) ((base) + (0x01B5UL))
#define PMIC_OTP_29_W_ADDR(base) ((base) + (0x01B6UL))
#define PMIC_OTP_30_W_ADDR(base) ((base) + (0x01B7UL))
#define PMIC_OTP_31_W_ADDR(base) ((base) + (0x01B8UL))
#define PMIC_OTP_32_W_ADDR(base) ((base) + (0x01B9UL))
#define PMIC_OTP_33_W_ADDR(base) ((base) + (0x01BAUL))
#define PMIC_OTP_34_W_ADDR(base) ((base) + (0x01BBUL))
#define PMIC_OTP_35_W_ADDR(base) ((base) + (0x01BCUL))
#define PMIC_OTP_36_W_ADDR(base) ((base) + (0x01BDUL))
#define PMIC_OTP_37_W_ADDR(base) ((base) + (0x01BEUL))
#define PMIC_OTP_38_W_ADDR(base) ((base) + (0x01BFUL))
#define PMIC_OTP_39_W_ADDR(base) ((base) + (0x01C0UL))
#define PMIC_OTP_40_W_ADDR(base) ((base) + (0x01C1UL))
#define PMIC_OTP_41_W_ADDR(base) ((base) + (0x01C2UL))
#define PMIC_OTP_42_W_ADDR(base) ((base) + (0x01C3UL))
#define PMIC_OTP_43_W_ADDR(base) ((base) + (0x01C4UL))
#define PMIC_OTP_44_W_ADDR(base) ((base) + (0x01C5UL))
#define PMIC_OTP_45_W_ADDR(base) ((base) + (0x01C6UL))
#define PMIC_D2A_RES0_ADDR(base) ((base) + (0x01CAUL))
#define PMIC_D2A_RES1_ADDR(base) ((base) + (0x01CBUL))
#define PMIC_A2D_RES0_ADDR(base) ((base) + (0x01CCUL))
#define PMIC_SIM_CTRL0_ADDR(base) ((base) + (0x01CDUL))
#define PMIC_SIM_CTRL1_ADDR(base) ((base) + (0x01CEUL))
#define PMIC_SIM_DEB_CTRL1_ADDR(base) ((base) + (0x01CFUL))
#define PMIC_SIM_DEB_CTRL2_ADDR(base) ((base) + (0x01D0UL))
#define PMIC_GPIO0DATA_ADDR(base) ((base) + (0x01D1UL))
#define PMIC_GPIO0_DATA_RESERVE_ADDR(base) ((base) + (0x01D2UL))
#define PMIC_GPIO0DIR_ADDR(base) ((base) + (0x01D3UL))
#define PMIC_GPIO0_DIR_RESERVE_ADDR(base) ((base) + (0x01D4UL))
#define PMIC_GPIO0IS_ADDR(base) ((base) + (0x01D5UL))
#define PMIC_GPIO0IBE_ADDR(base) ((base) + (0x01D6UL))
#define PMIC_GPIO0IEV_ADDR(base) ((base) + (0x01D7UL))
#define PMIC_GPIO0AFSEL_ADDR(base) ((base) + (0x01D8UL))
#define PMIC_GPIO0DSSEL_ADDR(base) ((base) + (0x01D9UL))
#define PMIC_GPIO0PUSEL_ADDR(base) ((base) + (0x01DAUL))
#define PMIC_GPIO0PDSEL_ADDR(base) ((base) + (0x01DBUL))
#define PMIC_GPIO0DEBSEL_ADDR(base) ((base) + (0x01DCUL))
#define PMIC_SIM0PUSEL_ADDR(base) ((base) + (0x01DDUL))
#define PMIC_SIM0PDSEL_ADDR(base) ((base) + (0x01DEUL))
#define PMIC_GPIO1DATA_ADDR(base) ((base) + (0x01DFUL))
#define PMIC_GPIO1_DATA_RESERVE_ADDR(base) ((base) + (0x01E0UL))
#define PMIC_GPIO1DIR_ADDR(base) ((base) + (0x01E1UL))
#define PMIC_GPIO1_DIR_RESERVE_ADDR(base) ((base) + (0x01E2UL))
#define PMIC_GPIO1IS_ADDR(base) ((base) + (0x01E3UL))
#define PMIC_GPIO1IBE_ADDR(base) ((base) + (0x01E4UL))
#define PMIC_GPIO1IEV_ADDR(base) ((base) + (0x01E5UL))
#define PMIC_GPIO1AFSEL_ADDR(base) ((base) + (0x01E6UL))
#define PMIC_GPIO1DSSEL_ADDR(base) ((base) + (0x01E7UL))
#define PMIC_GPIO1PUSEL_ADDR(base) ((base) + (0x01E8UL))
#define PMIC_GPIO1PDSEL_ADDR(base) ((base) + (0x01E9UL))
#define PMIC_GPIO1DEBSEL_ADDR(base) ((base) + (0x01EAUL))
#define PMIC_SIM1PUSEL_ADDR(base) ((base) + (0x01EBUL))
#define PMIC_SIM1PDSEL_ADDR(base) ((base) + (0x01ECUL))
#define PMIC_GPIO2DATA_ADDR(base) ((base) + (0x01EDUL))
#define PMIC_GPIO2_DATA_RESERVE_ADDR(base) ((base) + (0x01EEUL))
#define PMIC_GPIO2DIR_ADDR(base) ((base) + (0x01EFUL))
#define PMIC_GPIO2_DIR_RESERVE_ADDR(base) ((base) + (0x01F0UL))
#define PMIC_GPIO2IS_ADDR(base) ((base) + (0x01F1UL))
#define PMIC_GPIO2IBE_ADDR(base) ((base) + (0x01F2UL))
#define PMIC_GPIO2IEV_ADDR(base) ((base) + (0x01F3UL))
#define PMIC_GPIO2DSSEL_ADDR(base) ((base) + (0x01F4UL))
#define PMIC_GPIO2PUSEL_ADDR(base) ((base) + (0x01F5UL))
#define PMIC_GPIO2PDSEL_ADDR(base) ((base) + (0x01F6UL))
#define PMIC_GPIO2DEBSEL_ADDR(base) ((base) + (0x01F7UL))
#define PMIC_GPIO3DATA_ADDR(base) ((base) + (0x01F8UL))
#define PMIC_GPIO3_DATA_RESERVE_ADDR(base) ((base) + (0x01F9UL))
#define PMIC_GPIO3DIR_ADDR(base) ((base) + (0x01FAUL))
#define PMIC_GPIO3_DIR_RESERVE_ADDR(base) ((base) + (0x01FBUL))
#define PMIC_GPIO3IS_ADDR(base) ((base) + (0x01FCUL))
#define PMIC_GPIO3IBE_ADDR(base) ((base) + (0x01FDUL))
#define PMIC_GPIO3IEV_ADDR(base) ((base) + (0x01FEUL))
#define PMIC_GPIO3DSSEL_ADDR(base) ((base) + (0x01FFUL))
#define PMIC_GPIO3PUSEL_ADDR(base) ((base) + (0x0200UL))
#define PMIC_GPIO3PDSEL_ADDR(base) ((base) + (0x0201UL))
#define PMIC_GPIO3DEBSEL_ADDR(base) ((base) + (0x0202UL))
#define PMIC_AUX_IBIAS_CFG_ADDR(base) ((base) + (0x0203UL))
#define PMIC_DAC_DIN_MSB_ADDR(base) ((base) + (0x0204UL))
#define PMIC_DAC_DIN_LSB_ADDR(base) ((base) + (0x0205UL))
#define PMIC_RAMP_BUCK0_CTRL0_ADDR(base) ((base) + (0x0206UL))
#define PMIC_RAMP_BUCK0_CTRL1_ADDR(base) ((base) + (0x0207UL))
#define PMIC_BUCK0_RAMPUP_STATE_ADDR(base) ((base) + (0x0208UL))
#define PMIC_BUCK0_RAMPDOWN_STATE_ADDR(base) ((base) + (0x0209UL))
#define PMIC_BUCK0_RAMP_CHG_CTRL_ADDR(base) ((base) + (0x020AUL))
#define PMIC_RAMP_BUCK4_CTRL0_ADDR(base) ((base) + (0x020BUL))
#define PMIC_RAMP_BUCK4_CTRL1_ADDR(base) ((base) + (0x020CUL))
#define PMIC_BUCK4_RAMPUP_STATE_ADDR(base) ((base) + (0x020DUL))
#define PMIC_BUCK4_RAMPDOWN_STATE_ADDR(base) ((base) + (0x020EUL))
#define PMIC_BUCK4_RAMP_CHG_CTRL_ADDR(base) ((base) + (0x020FUL))
#define PMIC_RAMP_BUCK5_CTRL0_ADDR(base) ((base) + (0x0210UL))
#define PMIC_RAMP_BUCK5_CTRL1_ADDR(base) ((base) + (0x0211UL))
#define PMIC_BUCK5_RAMPUP_STATE_ADDR(base) ((base) + (0x0212UL))
#define PMIC_BUCK5_RAMPDOWN_STATE_ADDR(base) ((base) + (0x0213UL))
#define PMIC_BUCK5_RAMP_CHG_CTRL_ADDR(base) ((base) + (0x0214UL))
#define PMIC_RAMP_LDO0_2_CTRL0_ADDR(base) ((base) + (0x0215UL))
#define PMIC_RAMP_LDO0_2_CTRL1_ADDR(base) ((base) + (0x0216UL))
#define PMIC_RAMP_LDO40_CTRL0_ADDR(base) ((base) + (0x0217UL))
#define PMIC_RAMP_LDO40_CTRL1_ADDR(base) ((base) + (0x0218UL))
#define PMIC_DIG_IO_DS_32K_CFG_ADDR(base) ((base) + (0x0219UL))
#define PMIC_DIG_IO_DS_CODEC_CFG_ADDR(base) ((base) + (0x021AUL))
#define PMIC_PULL_DOWN_CFG_ADDR(base) ((base) + (0x021BUL))
#define PMIC_SPMI_ECO_GT_BYPASS_ADDR(base) ((base) + (0x021CUL))
#define PMIC_PWRONN_8S_XOADC_DEBUG_CTRL_ADDR(base) ((base) + (0x021DUL))
#define PMIC_DIS_19M2_CTRL_ADDR(base) ((base) + (0x021EUL))
#else
#define PMIC_VERSION0_ADDR(base) ((base) + (0x0000))
#define PMIC_VERSION1_ADDR(base) ((base) + (0x0001))
#define PMIC_VERSION2_ADDR(base) ((base) + (0x0002))
#define PMIC_VERSION3_ADDR(base) ((base) + (0x0003))
#define PMIC_VERSION4_ADDR(base) ((base) + (0x0004))
#define PMIC_VERSION5_ADDR(base) ((base) + (0x0005))
#define PMIC_STATUS0_ADDR(base) ((base) + (0x0006))
#define PMIC_STATUS1_ADDR(base) ((base) + (0x0007))
#define PMIC_STATUS2_ADDR(base) ((base) + (0x0008))
#define PMIC_BUCK0_ONOFF_ECO_ADDR(base) ((base) + (0x000A))
#define PMIC_BUCK1_ONOFF_ECO_ADDR(base) ((base) + (0x000B))
#define PMIC_BUCK2_ONOFF_ECO_ADDR(base) ((base) + (0x000C))
#define PMIC_BUCK3_ONOFF_ECO_ADDR(base) ((base) + (0x000D))
#define PMIC_BUCK4_ONOFF_ECO_ADDR(base) ((base) + (0x000E))
#define PMIC_BUCK5_ONOFF_ECO_ADDR(base) ((base) + (0x000F))
#define PMIC_LDO0_1_ONOFF_ADDR(base) ((base) + (0x0010))
#define PMIC_LDO0_2_ONOFF_ECO_ADDR(base) ((base) + (0x0011))
#define PMIC_LDO1_ONOFF_ADDR(base) ((base) + (0x0012))
#define PMIC_LDO2_ONOFF_ECO_ADDR(base) ((base) + (0x0013))
#define PMIC_LDO3_ONOFF_ADDR(base) ((base) + (0x0014))
#define PMIC_LDO4_ONOFF_ECO_ADDR(base) ((base) + (0x0015))
#define PMIC_LDO5_ONOFF_ECO_ADDR(base) ((base) + (0x0016))
#define PMIC_LDO6_ONOFF_ECO_ADDR(base) ((base) + (0x0017))
#define PMIC_LDO8_ONOFF_ECO_ADDR(base) ((base) + (0x0018))
#define PMIC_LDO9_ONOFF_ECO_ADDR(base) ((base) + (0x0019))
#define PMIC_LDO11_ONOFF_ECO_ADDR(base) ((base) + (0x001A))
#define PMIC_LDO12_ONOFF_ECO_ADDR(base) ((base) + (0x001B))
#define PMIC_LDO13_ONOFF_ADDR(base) ((base) + (0x001C))
#define PMIC_LDO14_ONOFF_ADDR(base) ((base) + (0x001D))
#define PMIC_LDO15_ONOFF_ECO_ADDR(base) ((base) + (0x001E))
#define PMIC_LDO16_ONOFF_ECO_ADDR(base) ((base) + (0x001F))
#define PMIC_LDO17_ONOFF_ECO_ADDR(base) ((base) + (0x0020))
#define PMIC_LDO18_ONOFF_ECO_ADDR(base) ((base) + (0x0021))
#define PMIC_LDO19_ONOFF_ADDR(base) ((base) + (0x0022))
#define PMIC_LDO20_ONOFF_ADDR(base) ((base) + (0x0023))
#define PMIC_LDO21_ONOFF_ADDR(base) ((base) + (0x0024))
#define PMIC_LDO22_ONOFF_ADDR(base) ((base) + (0x0025))
#define PMIC_LDO23_ONOFF_ECO_ADDR(base) ((base) + (0x0026))
#define PMIC_LDO24_ONOFF_ECO_ADDR(base) ((base) + (0x0027))
#define PMIC_LDO25_ONOFF_ADDR(base) ((base) + (0x0028))
#define PMIC_LDO26_ONOFF_ECO_ADDR(base) ((base) + (0x0029))
#define PMIC_LDO27_ONOFF_ECO_ADDR(base) ((base) + (0x002A))
#define PMIC_LDO28_ONOFF_ADDR(base) ((base) + (0x002B))
#define PMIC_LDO29_ONOFF_ECO_ADDR(base) ((base) + (0x002C))
#define PMIC_LDO30_1_ONOFF_ADDR(base) ((base) + (0x002D))
#define PMIC_LDO30_2_ONOFF_ECO_ADDR(base) ((base) + (0x002E))
#define PMIC_LDO31_ONOFF_ADDR(base) ((base) + (0x002F))
#define PMIC_LDO32_ONOFF_ADDR(base) ((base) + (0x0030))
#define PMIC_LDO33_ONOFF1_ADDR(base) ((base) + (0x0031))
#define PMIC_LDO33_ONOFF2_ADDR(base) ((base) + (0x0032))
#define PMIC_LDO34_ONOFF_ECO_ADDR(base) ((base) + (0x0033))
#define PMIC_PMUH_ONOFF_ECO_ADDR(base) ((base) + (0x0034))
#define PMIC_LDO37_ONOFF_ECO_ADDR(base) ((base) + (0x0035))
#define PMIC_LDO38_ONOFF_ECO_ADDR(base) ((base) + (0x0036))
#define PMIC_LDO39_ONOFF_ECO_ADDR(base) ((base) + (0x0037))
#define PMIC_LDO40_ONOFF_ADDR(base) ((base) + (0x0038))
#define PMIC_LDO_PMUA_ECO_ADDR(base) ((base) + (0x0039))
#define PMIC_CLK_ABB_EN_ADDR(base) ((base) + (0x003D))
#define PMIC_CLK_WIFI_EN_ADDR(base) ((base) + (0x003E))
#define PMIC_CLK_NFC_EN_ADDR(base) ((base) + (0x003F))
#define PMIC_CLK_RF_EN_ADDR(base) ((base) + (0x0040))
#define PMIC_CLK_SYS_EN_ADDR(base) ((base) + (0x0041))
#define PMIC_CLK_CODEC_EN_ADDR(base) ((base) + (0x0042))
#define PMIC_CLK_UFS_EN_ADDR(base) ((base) + (0x0043))
#define PMIC_OSC32K_GPS_ONOFF_CTRL_ADDR(base) ((base) + (0x0044))
#define PMIC_OSC32K_BT_ONOFF_CTRL_ADDR(base) ((base) + (0x0045))
#define PMIC_OSC32K_SYS_ONOFF_CTRL_ADDR(base) ((base) + (0x0046))
#define PMIC_BUCK0_VSET_ADDR(base) ((base) + (0x0048))
#define PMIC_BUCK0_VSET_ECO_ADDR(base) ((base) + (0x0049))
#define PMIC_BUCK1_VSET_ADDR(base) ((base) + (0x004A))
#define PMIC_BUCK2_VSET_ADDR(base) ((base) + (0x004B))
#define PMIC_BUCK2_VSET_ECO_ADDR(base) ((base) + (0x004C))
#define PMIC_BUCK3_VSET_ADDR(base) ((base) + (0x004D))
#define PMIC_BUCK3_VSET_ECO_ADDR(base) ((base) + (0x004E))
#define PMIC_BUCK4_VSET_ADDR(base) ((base) + (0x004F))
#define PMIC_BUCK4_VSET_ECO_ADDR(base) ((base) + (0x0050))
#define PMIC_BUCK4_VSET_IDLE_ADDR(base) ((base) + (0x0051))
#define PMIC_BUCK5_VSET_ADDR(base) ((base) + (0x0052))
#define PMIC_LDO0_2_VSET_ADDR(base) ((base) + (0x0053))
#define PMIC_LDO0_2_VSET_ECO_ADDR(base) ((base) + (0x0054))
#define PMIC_LDO1_VSET_ADDR(base) ((base) + (0x0055))
#define PMIC_LDO2_VSET_ADDR(base) ((base) + (0x0056))
#define PMIC_LDO2_VSET_ECO_ADDR(base) ((base) + (0x0057))
#define PMIC_LDO3_VSET_ADDR(base) ((base) + (0x0058))
#define PMIC_LDO4_VSET_ADDR(base) ((base) + (0x0059))
#define PMIC_LDO5_VSET_ADDR(base) ((base) + (0x005A))
#define PMIC_LDO6_VSET_ADDR(base) ((base) + (0x005B))
#define PMIC_LDO8_VSET_ADDR(base) ((base) + (0x005C))
#define PMIC_LDO9_VSET_ADDR(base) ((base) + (0x005D))
#define PMIC_LDO11_VSET_ADDR(base) ((base) + (0x005E))
#define PMIC_LDO12_VSET_ADDR(base) ((base) + (0x005F))
#define PMIC_LDO13_VSET_ADDR(base) ((base) + (0x0060))
#define PMIC_LDO14_VSET_ADDR(base) ((base) + (0x0061))
#define PMIC_LDO15_VSET_ADDR(base) ((base) + (0x0062))
#define PMIC_LDO16_VSET_ADDR(base) ((base) + (0x0063))
#define PMIC_LDO17_VSET_ADDR(base) ((base) + (0x0064))
#define PMIC_LDO18_VSET_ADDR(base) ((base) + (0x0065))
#define PMIC_LDO19_VSET_ADDR(base) ((base) + (0x0066))
#define PMIC_LDO20_VSET_ADDR(base) ((base) + (0x0067))
#define PMIC_LDO21_VSET_ADDR(base) ((base) + (0x0068))
#define PMIC_LDO22_VSET_ADDR(base) ((base) + (0x0069))
#define PMIC_LDO23_VSET_ADDR(base) ((base) + (0x006A))
#define PMIC_LDO24_VSET_ADDR(base) ((base) + (0x006B))
#define PMIC_LDO25_VSET_ADDR(base) ((base) + (0x006C))
#define PMIC_LDO26_VSET_ADDR(base) ((base) + (0x006D))
#define PMIC_LDO27_VSET_ADDR(base) ((base) + (0x006E))
#define PMIC_LDO28_VSET_ADDR(base) ((base) + (0x006F))
#define PMIC_LDO29_VSET_ADDR(base) ((base) + (0x0070))
#define PMIC_LDO30_VSET_ADDR(base) ((base) + (0x0071))
#define PMIC_LDO31_VSET_ADDR(base) ((base) + (0x0072))
#define PMIC_LDO32_VSET_ADDR(base) ((base) + (0x0073))
#define PMIC_LDO33_VSET_ADDR(base) ((base) + (0x0074))
#define PMIC_LDO34_VSET_ADDR(base) ((base) + (0x0075))
#define PMIC_PMUH_VSET_ADDR(base) ((base) + (0x0076))
#define PMIC_LDO37_VSET_ADDR(base) ((base) + (0x0077))
#define PMIC_LDO38_VSET_ADDR(base) ((base) + (0x0078))
#define PMIC_LDO39_VSET_ADDR(base) ((base) + (0x0079))
#define PMIC_LDO40_VSET_ADDR(base) ((base) + (0x007A))
#define PMIC_LDO_BUF_VSET_ADDR(base) ((base) + (0x007B))
#define PMIC_LDO_PMUA_VSET_ADDR(base) ((base) + (0x007C))
#define PMIC_BUCK0_CTRL0_ADDR(base) ((base) + (0x07F))
#define PMIC_BUCK0_CTRL1_ADDR(base) ((base) + (0x080))
#define PMIC_BUCK0_CTRL2_ADDR(base) ((base) + (0x081))
#define PMIC_BUCK0_CTRL3_ADDR(base) ((base) + (0x082))
#define PMIC_BUCK0_CTRL4_ADDR(base) ((base) + (0x083))
#define PMIC_BUCK0_CTRL5_ADDR(base) ((base) + (0x084))
#define PMIC_BUCK0_CTRL6_ADDR(base) ((base) + (0x085))
#define PMIC_BUCK0_CTRL7_ADDR(base) ((base) + (0x086))
#define PMIC_BUCK0_CTRL8_ADDR(base) ((base) + (0x087))
#define PMIC_BUCK0_CTRL9_ADDR(base) ((base) + (0x088))
#define PMIC_BUCK0_CTRL10_ADDR(base) ((base) + (0x089))
#define PMIC_BUCK0_CTRL11_ADDR(base) ((base) + (0x08A))
#define PMIC_BUCK1_CTRL0_ADDR(base) ((base) + (0x08B))
#define PMIC_BUCK1_CTRL1_ADDR(base) ((base) + (0x08C))
#define PMIC_BUCK1_CTRL2_ADDR(base) ((base) + (0x08D))
#define PMIC_BUCK1_CTRL3_ADDR(base) ((base) + (0x08E))
#define PMIC_BUCK1_CTRL4_ADDR(base) ((base) + (0x08F))
#define PMIC_BUCK1_CTRL5_ADDR(base) ((base) + (0x090))
#define PMIC_BUCK1_CTRL6_ADDR(base) ((base) + (0x091))
#define PMIC_BUCK1_CTRL7_ADDR(base) ((base) + (0x092))
#define PMIC_BUCK1_CTRL8_ADDR(base) ((base) + (0x093))
#define PMIC_BUCK1_CTRL9_ADDR(base) ((base) + (0x094))
#define PMIC_BUCK1_CTRL10_ADDR(base) ((base) + (0x095))
#define PMIC_BUCK2_CTRL0_ADDR(base) ((base) + (0x096))
#define PMIC_BUCK2_CTRL1_ADDR(base) ((base) + (0x097))
#define PMIC_BUCK2_CTRL2_ADDR(base) ((base) + (0x098))
#define PMIC_BUCK2_CTRL3_ADDR(base) ((base) + (0x099))
#define PMIC_BUCK2_CTRL4_ADDR(base) ((base) + (0x09A))
#define PMIC_BUCK2_CTRL5_ADDR(base) ((base) + (0x09B))
#define PMIC_BUCK2_CTRL6_ADDR(base) ((base) + (0x09C))
#define PMIC_BUCK2_CTRL7_ADDR(base) ((base) + (0x09D))
#define PMIC_BUCK2_CTRL8_ADDR(base) ((base) + (0x09E))
#define PMIC_BUCK2_CTRL9_ADDR(base) ((base) + (0x09F))
#define PMIC_BUCK2_CTRL10_ADDR(base) ((base) + (0x0A0))
#define PMIC_BUCK3_CTRL0_ADDR(base) ((base) + (0x0A1))
#define PMIC_BUCK3_CTRL1_ADDR(base) ((base) + (0x0A2))
#define PMIC_BUCK3_CTRL2_ADDR(base) ((base) + (0x0A3))
#define PMIC_BUCK3_CTRL3_ADDR(base) ((base) + (0x0A4))
#define PMIC_BUCK3_CTRL4_ADDR(base) ((base) + (0x0A5))
#define PMIC_BUCK3_CTRL5_ADDR(base) ((base) + (0x0A6))
#define PMIC_BUCK3_CTRL6_ADDR(base) ((base) + (0x0A7))
#define PMIC_BUCK3_CTRL7_ADDR(base) ((base) + (0x0A8))
#define PMIC_BUCK3_CTRL8_ADDR(base) ((base) + (0x0A9))
#define PMIC_BUCK3_CTRL9_ADDR(base) ((base) + (0x0AA))
#define PMIC_BUCK4_CTRL0_ADDR(base) ((base) + (0x0AB))
#define PMIC_BUCK4_CTRL1_ADDR(base) ((base) + (0x0AC))
#define PMIC_BUCK4_CTRL2_ADDR(base) ((base) + (0x0AD))
#define PMIC_BUCK4_CTRL3_ADDR(base) ((base) + (0x0AE))
#define PMIC_BUCK4_CTRL4_ADDR(base) ((base) + (0x0AF))
#define PMIC_BUCK4_CTRL5_ADDR(base) ((base) + (0x0B0))
#define PMIC_BUCK4_CTRL6_ADDR(base) ((base) + (0x0B1))
#define PMIC_BUCK4_CTRL7_ADDR(base) ((base) + (0x0B2))
#define PMIC_BUCK4_CTRL8_ADDR(base) ((base) + (0x0B3))
#define PMIC_BUCK4_CTRL9_ADDR(base) ((base) + (0x0B4))
#define PMIC_BUCK4_CTRL10_ADDR(base) ((base) + (0x0B5))
#define PMIC_BUCK4_CTRL11_ADDR(base) ((base) + (0x0B6))
#define PMIC_BUCK5_CTRL0_ADDR(base) ((base) + (0x0B7))
#define PMIC_BUCK5_CTRL1_ADDR(base) ((base) + (0x0B8))
#define PMIC_BUCK5_CTRL2_ADDR(base) ((base) + (0x0B9))
#define PMIC_BUCK5_CTRL3_ADDR(base) ((base) + (0x0BA))
#define PMIC_BUCK5_CTRL4_ADDR(base) ((base) + (0x0BB))
#define PMIC_BUCK5_CTRL5_ADDR(base) ((base) + (0x0BC))
#define PMIC_BUCK5_CTRL6_ADDR(base) ((base) + (0x0BD))
#define PMIC_BUCK5_CTRL7_ADDR(base) ((base) + (0x0BE))
#define PMIC_BUCK5_CTRL8_ADDR(base) ((base) + (0x0BF))
#define PMIC_BUCK5_CTRL9_ADDR(base) ((base) + (0x0C0))
#define PMIC_BUCK5_CTRL10_ADDR(base) ((base) + (0x0C1))
#define PMIC_BUCK5_CTRL11_ADDR(base) ((base) + (0x0C2))
#define PMIC_BUCK_CTRL0_ADDR(base) ((base) + (0x0C3))
#define PMIC_BUCK_CTRL1_ADDR(base) ((base) + (0x0C4))
#define PMIC_BUCK_RESERVE0_ADDR(base) ((base) + (0x0C5))
#define PMIC_BUCK_RESERVE1_ADDR(base) ((base) + (0x0C6))
#define PMIC_BUCK_RESERVE2_ADDR(base) ((base) + (0x0C7))
#define PMIC_LDO0_CTRL0_ADDR(base) ((base) + (0x0CC))
#define PMIC_LDO0_CTRL1_ADDR(base) ((base) + (0x0CD))
#define PMIC_LDO1_CTRL0_ADDR(base) ((base) + (0x0CE))
#define PMIC_LDO1_CTRL1_ADDR(base) ((base) + (0x0CF))
#define PMIC_LDO1_CTRL2_ADDR(base) ((base) + (0x0D0))
#define PMIC_LDO2_CTRL0_ADDR(base) ((base) + (0x0D1))
#define PMIC_LDO2_CTRL1_ADDR(base) ((base) + (0x0D2))
#define PMIC_LDO3_CTRL0_ADDR(base) ((base) + (0x0D3))
#define PMIC_LDO3_CTRL1_ADDR(base) ((base) + (0x0D4))
#define PMIC_LDO4_CTRL_ADDR(base) ((base) + (0x0D5))
#define PMIC_LDO5_CTRL_ADDR(base) ((base) + (0x0D6))
#define PMIC_LDO6_CTRL0_ADDR(base) ((base) + (0x0D7))
#define PMIC_LDO6_CTRL1_ADDR(base) ((base) + (0x0D8))
#define PMIC_LDO8_CTRL_ADDR(base) ((base) + (0x0D9))
#define PMIC_LDO9_CTRL_ADDR(base) ((base) + (0x0DA))
#define PMIC_LD11_CTRL_ADDR(base) ((base) + (0x0DB))
#define PMIC_LD12_CTRL_ADDR(base) ((base) + (0x0DC))
#define PMIC_LD13_CTRL0_ADDR(base) ((base) + (0x0DD))
#define PMIC_LD13_CTRL1_ADDR(base) ((base) + (0x0DE))
#define PMIC_LDO14_CTRL_ADDR(base) ((base) + (0x0DF))
#define PMIC_LDO15_CTRL_ADDR(base) ((base) + (0x0E0))
#define PMIC_LDO16_CTRL_ADDR(base) ((base) + (0x0E1))
#define PMIC_LDO17_CTRL_ADDR(base) ((base) + (0x0E2))
#define PMIC_LDO18_CTRL0_ADDR(base) ((base) + (0x0E3))
#define PMIC_LDO19_CTRL0_ADDR(base) ((base) + (0x0E4))
#define PMIC_LD19_CTRL1_ADDR(base) ((base) + (0x0E5))
#define PMIC_LDO20_CTRL0_ADDR(base) ((base) + (0x0E6))
#define PMIC_LDO20_CTRL1_ADDR(base) ((base) + (0x0E7))
#define PMIC_LDO20_CTRL2_ADDR(base) ((base) + (0x0E8))
#define PMIC_LDO20_CTRL3_ADDR(base) ((base) + (0x0E9))
#define PMIC_LDO21_CTRL_ADDR(base) ((base) + (0x0EA))
#define PMIC_LDO22_CTRL0_ADDR(base) ((base) + (0x0EB))
#define PMIC_LDO22_CTRL1_ADDR(base) ((base) + (0x0EC))
#define PMIC_LDO23_CTRL_ADDR(base) ((base) + (0x0ED))
#define PMIC_LDO24_CTRL0_ADDR(base) ((base) + (0x0EE))
#define PMIC_LDO24_CTRL1_ADDR(base) ((base) + (0x0EF))
#define PMIC_LDO25_CTRL0_ADDR(base) ((base) + (0x0F0))
#define PMIC_LDO26_CTRL_ADDR(base) ((base) + (0x0F1))
#define PMIC_LDO27_CTRL_ADDR(base) ((base) + (0x0F2))
#define PMIC_LDO28_CTRL_ADDR(base) ((base) + (0x0F3))
#define PMIC_LDO29_CTRL0_ADDR(base) ((base) + (0x0F4))
#define PMIC_LDO29_CTRL1_ADDR(base) ((base) + (0x0F5))
#define PMIC_LDO30_CTRL0_ADDR(base) ((base) + (0x0F6))
#define PMIC_LDO30_2_CTRL1_ADDR(base) ((base) + (0x0F7))
#define PMIC_LDO31_CTRL_ADDR(base) ((base) + (0x0F8))
#define PMIC_LDO32_CTRL0_ADDR(base) ((base) + (0x0F9))
#define PMIC_LDO32_CTRL1_ADDR(base) ((base) + (0x0FA))
#define PMIC_LDO32_CTRL2_ADDR(base) ((base) + (0x0FB))
#define PMIC_LDO32_CTRL3_ADDR(base) ((base) + (0x0FC))
#define PMIC_LDO33_CTRL0_ADDR(base) ((base) + (0x0FD))
#define PMIC_LDO34_CTRL0_ADDR(base) ((base) + (0x0FE))
#define PMIC_LDO34_CTRL1_ADDR(base) ((base) + (0x0FF))
#define PMIC_PMUH_CTRL_ADDR(base) ((base) + (0x100))
#define PMIC_LDO37_CTRL0_ADDR(base) ((base) + (0x101))
#define PMIC_LDO37_CTRL1_ADDR(base) ((base) + (0x102))
#define PMIC_LDO37_CTRL2_ADDR(base) ((base) + (0x103))
#define PMIC_LDO37_CTRL3_ADDR(base) ((base) + (0x104))
#define PMIC_LDO38_CTRL0_ADDR(base) ((base) + (0x105))
#define PMIC_LDO38_CTRL1_ADDR(base) ((base) + (0x106))
#define PMIC_LDO38_CTRL2_ADDR(base) ((base) + (0x107))
#define PMIC_LDO39_CTRL0_ADDR(base) ((base) + (0x108))
#define PMIC_LDO39_CTRL1_ADDR(base) ((base) + (0x109))
#define PMIC_LDO39_CTRL2_ADDR(base) ((base) + (0x10A))
#define PMIC_LDO39_CTRL3_ADDR(base) ((base) + (0x10B))
#define PMIC_LDO40_CTRL0_ADDR(base) ((base) + (0x10C))
#define PMIC_LDO40_CTRL1_ADDR(base) ((base) + (0x10D))
#define PMIC_LDO40_CTRL2_ADDR(base) ((base) + (0x10E))
#define PMIC_LDO40_CTRL3_ADDR(base) ((base) + (0x10F))
#define PMIC_LDO_BUF_PMUA_CTRL_ADDR(base) ((base) + (0x110))
#define PMIC_LDO_RESERVE0_ADDR(base) ((base) + (0x111))
#define PMIC_LDO_RESERVE1_ADDR(base) ((base) + (0x112))
#define PMIC_LDO1_CTRL3_ADDR(base) ((base) + (0x113))
#define PMIC_BUCK0_3_OCP_CTRL_ADDR(base) ((base) + (0x0118))
#define PMIC_BUCK4_5_OCP_CTRL_ADDR(base) ((base) + (0x0119))
#define PMIC_LDO0_3_OCP_CTRL_ADDR(base) ((base) + (0x011A))
#define PMIC_LDO4_8_OCP_CTRL_ADDR(base) ((base) + (0x011B))
#define PMIC_LDO9_13_OCP_CTRL_ADDR(base) ((base) + (0x011C))
#define PMIC_LDO14_17_OCP_CTRL_ADDR(base) ((base) + (0x011D))
#define PMIC_LDO18_21_OCP_CTRL_ADDR(base) ((base) + (0x011E))
#define PMIC_LDO22_25_OCP_CTRL_ADDR(base) ((base) + (0x011F))
#define PMIC_LDO26_29_OCP_CTRL_ADDR(base) ((base) + (0x0120))
#define PMIC_LDO30_33_OCP_CTRL_ADDR(base) ((base) + (0x0121))
#define PMIC_LDO34_39_OCP_CTRL_ADDR(base) ((base) + (0x0122))
#define PMIC_LDO_BUFF_SCP_CTRL_ADDR(base) ((base) + (0x0123))
#define PMIC_BUCK0_3_SCP_CTRL_ADDR(base) ((base) + (0x0124))
#define PMIC_BUCK4_5_SCP_CTRL_ADDR(base) ((base) + (0x0125))
#define PMIC_SYS_CTRL_RESERVE_ADDR(base) ((base) + (0x0128))
#define PMIC_OCP_DEB_CTRL0_ADDR(base) ((base) + (0x0129))
#define PMIC_OCP_DEB_CTRL1_ADDR(base) ((base) + (0x012A))
#define PMIC_PWROFF_DEB_CTRL_ADDR(base) ((base) + (0x012B))
#define PMIC_VSYS_DROP_DEB_CTRL_ADDR(base) ((base) + (0x012C))
#define PMIC_OCP_SCP_ONOFF_ADDR(base) ((base) + (0x012D))
#define PMIC_CLK_ABB_CTRL0_ADDR(base) ((base) + (0x0130))
#define PMIC_CLK_WIFI_CTRL0_ADDR(base) ((base) + (0x0131))
#define PMIC_CLK_NFC_CTRL0_ADDR(base) ((base) + (0x0132))
#define PMIC_CLK_RF_CTRL0_ADDR(base) ((base) + (0x0133))
#define PMIC_CLK_USB_CTRL_ADDR(base) ((base) + (0x0134))
#define PMIC_CLK_CODEC_CTRL_ADDR(base) ((base) + (0x0135))
#define PMIC_CLK_UFS_CTRL_ADDR(base) ((base) + (0x0136))
#define PMIC_TON_ECO_SHIELD_CTRL_ADDR(base) ((base) + (0x0137))
#define PMIC_CLK_TOP_CTRL0_ADDR(base) ((base) + (0x0138))
#define PMIC_CLK_TOP_CTRL1_ADDR(base) ((base) + (0x0139))
#define PMIC_BG_THSD_CTRL0_ADDR(base) ((base) + (0x013A))
#define PMIC_BG_THSD_CTRL1_ADDR(base) ((base) + (0x013B))
#define PMIC_BG_TEST0_ADDR(base) ((base) + (0x013C))
#define PMIC_BG_TEST1_ADDR(base) ((base) + (0x013D))
#define PMIC_HARDWIRE_CTRL0_ADDR(base) ((base) + (0x0140))
#define PMIC_HARDWIRE_CTRL1_ADDR(base) ((base) + (0x0141))
#define PMIC_LOW_POWER_CFG_ADDR(base) ((base) + (0x0142))
#define PMIC_PERI_CTRL0_ADDR(base) ((base) + (0x0143))
#define PMIC_PERI_CTRL1_ADDR(base) ((base) + (0x0144))
#define PMIC_PERI_CTRL2_ADDR(base) ((base) + (0x0145))
#define PMIC_PERI_CTRL3_ADDR(base) ((base) + (0x0146))
#define PMIC_PERI_CTRL4_ADDR(base) ((base) + (0x0147))
#define PMIC_PERI_CTRL5_ADDR(base) ((base) + (0x0148))
#define PMIC_PERI_CNT_CFG_ADDR(base) ((base) + (0x0149))
#define PMIC_PERI_CTRL6_ADDR(base) ((base) + (0x014A))
#define PMIC_PERI_CTRL7_ADDR(base) ((base) + (0x014B))
#define PMIC_PERI_CTRL8_ADDR(base) ((base) + (0x014C))
#define PMIC_PERI_CTRL9_ADDR(base) ((base) + (0x014D))
#define PMIC_PERI_CTRL10_ADDR(base) ((base) + (0x014E))
#define PMIC_PERI_CTRL11_ADDR(base) ((base) + (0x014F))
#define PMIC_PERI_CTRL12_ADDR(base) ((base) + (0x0150))
#define PMIC_PERI_CTRL13_ADDR(base) ((base) + (0x0151))
#define PMIC_PERI_CTRL14_ADDR(base) ((base) + (0x0152))
#define PMIC_PERI_CTRL15_ADDR(base) ((base) + (0x0153))
#define PMIC_PERI_CTRL16_ADDR(base) ((base) + (0x0154))
#define PMIC_PERI_CTRL17_ADDR(base) ((base) + (0x0155))
#define PMIC_PERI_CTRL18_ADDR(base) ((base) + (0x0156))
#define PMIC_PERI_CTRL19_ADDR(base) ((base) + (0x0157))
#define PMIC_PERI_CTRL20_ADDR(base) ((base) + (0x0158))
#define PMIC_PERI_CTRL21_ADDR(base) ((base) + (0x0159))
#define PMIC_WIFI_CTRL_ADDR(base) ((base) + (0x015B))
#define PMIC_PERI_VSET_CTRL_ADDR(base) ((base) + (0x015C))
#define PMIC_IDLE_CTRL0_ADDR(base) ((base) + (0x015D))
#define PMIC_IDLE_CTRL1_ADDR(base) ((base) + (0x015E))
#define PMIC_IDLE_CTRL2_ADDR(base) ((base) + (0x015F))
#define PMIC_IDLE_CTRL3_ADDR(base) ((base) + (0x0160))
#define PMIC_IDLE_CTRL4_ADDR(base) ((base) + (0x0161))
#define PMIC_IDLE_CTRL5_ADDR(base) ((base) + (0x0162))
#define PMIC_IDLE_CNT_CFG_ADDR(base) ((base) + (0x0163))
#define PMIC_IDLE_CTRL6_ADDR(base) ((base) + (0x0164))
#define PMIC_IDLE_CTRL7_ADDR(base) ((base) + (0x0165))
#define PMIC_IDLE_CTRL8_ADDR(base) ((base) + (0x0166))
#define PMIC_IDLE_CTRL9_ADDR(base) ((base) + (0x0167))
#define PMIC_IDLE_CTRL10_ADDR(base) ((base) + (0x0168))
#define PMIC_IDLE_CTRL11_ADDR(base) ((base) + (0x0169))
#define PMIC_IDLE_CTRL12_ADDR(base) ((base) + (0x016A))
#define PMIC_IDLE_CTRL13_ADDR(base) ((base) + (0x016B))
#define PMIC_IDLE_CTRL14_ADDR(base) ((base) + (0x016C))
#define PMIC_IDLE_CTRL15_ADDR(base) ((base) + (0x016D))
#define PMIC_IDLE_CTRL16_ADDR(base) ((base) + (0x016E))
#define PMIC_IDLE_CTRL17_ADDR(base) ((base) + (0x016F))
#define PMIC_IDLE_CTRL18_ADDR(base) ((base) + (0x0170))
#define PMIC_IDLE_CTRL19_ADDR(base) ((base) + (0x0171))
#define PMIC_IDLE_CTRL20_ADDR(base) ((base) + (0x0172))
#define PMIC_IDLE_CTRL21_ADDR(base) ((base) + (0x0173))
#define PMIC_IDLE_VSET_CFG_ADDR(base) ((base) + (0x0174))
#define PMIC_COUL_ECO_MASK_ADDR(base) ((base) + (0x0175))
#define PMIC_SOH_ECO_MASK_ADDR(base) ((base) + (0x0176))
#define PMIC_CLASSD_CTRL0_ADDR(base) ((base) + (0x017A))
#define PMIC_CLASSD_CTRL1_ADDR(base) ((base) + (0x017B))
#define PMIC_CLASSD_CTRL2_ADDR(base) ((base) + (0x017C))
#define PMIC_CLASSD_CTRL3_ADDR(base) ((base) + (0x017D))
#define PMIC_PMU_SOFT_RST_ADDR(base) ((base) + (0x017E))
#define PMIC_LOCK_ADDR(base) ((base) + (0x017F))
#define PMIC_DR_EN_MODE_345_ADDR(base) ((base) + (0x0180))
#define PMIC_FLASH_PERIOD_DR345_ADDR(base) ((base) + (0x0181))
#define PMIC_FLASH_ON_DR345_ADDR(base) ((base) + (0x0182))
#define PMIC_DR_MODE_SEL_ADDR(base) ((base) + (0x0183))
#define PMIC_DR_1_2_CTRL0_ADDR(base) ((base) + (0x0184))
#define PMIC_DR_1_2_CTRL1_ADDR(base) ((base) + (0x0185))
#define PMIC_DR1_ISET_ADDR(base) ((base) + (0x0186))
#define PMIC_DR2_ISET_ADDR(base) ((base) + (0x0187))
#define PMIC_DR_LED_CTRL_ADDR(base) ((base) + (0x0188))
#define PMIC_DR_OUT_CTRL_ADDR(base) ((base) + (0x0189))
#define PMIC_DR3_ISET_ADDR(base) ((base) + (0x018A))
#define PMIC_DR3_START_DEL_ADDR(base) ((base) + (0x018B))
#define PMIC_DR4_ISET_ADDR(base) ((base) + (0x018C))
#define PMIC_DR4_START_DEL_ADDR(base) ((base) + (0x018D))
#define PMIC_DR5_ISET_ADDR(base) ((base) + (0x018E))
#define PMIC_DR5_START_DEL_ADDR(base) ((base) + (0x018F))
#define PMIC_DR345_TIM_CONF0_ADDR(base) ((base) + (0x0190))
#define PMIC_DR345_TIM_CONF1_ADDR(base) ((base) + (0x0191))
#define PMIC_DR_CTRLRESERVE8_ADDR(base) ((base) + (0x0192))
#define PMIC_DR_CTRLRESERVE9_ADDR(base) ((base) + (0x0193))
#define PMIC_OTP_0_ADDR(base) ((base) + (0x0194))
#define PMIC_OTP_1_ADDR(base) ((base) + (0x0195))
#define PMIC_OTP_CTRL0_ADDR(base) ((base) + (0x0196))
#define PMIC_OTP_CTRL1_ADDR(base) ((base) + (0x0197))
#define PMIC_OTP_CTRL2_ADDR(base) ((base) + (0x0198))
#define PMIC_OTP_WDATA_ADDR(base) ((base) + (0x0199))
#define PMIC_OTP_0_W_ADDR(base) ((base) + (0x019A))
#define PMIC_OTP_1_W_ADDR(base) ((base) + (0x019B))
#define PMIC_OTP_2_W_ADDR(base) ((base) + (0x019C))
#define PMIC_OTP_4_W_ADDR(base) ((base) + (0x019D))
#define PMIC_OTP_5_W_ADDR(base) ((base) + (0x019E))
#define PMIC_OTP_6_W_ADDR(base) ((base) + (0x019F))
#define PMIC_OTP_7_W_ADDR(base) ((base) + (0x01A0))
#define PMIC_OTP_8_W_ADDR(base) ((base) + (0x01A1))
#define PMIC_OTP_9_W_ADDR(base) ((base) + (0x01A2))
#define PMIC_OTP_10_W_ADDR(base) ((base) + (0x01A3))
#define PMIC_OTP_11_W_ADDR(base) ((base) + (0x01A4))
#define PMIC_OTP_12_W_ADDR(base) ((base) + (0x01A5))
#define PMIC_OTP_13_W_ADDR(base) ((base) + (0x01A6))
#define PMIC_OTP_14_W_ADDR(base) ((base) + (0x01A7))
#define PMIC_OTP_15_W_ADDR(base) ((base) + (0x01A8))
#define PMIC_OTP_16_W_ADDR(base) ((base) + (0x01A9))
#define PMIC_OTP_17_W_ADDR(base) ((base) + (0x01AA))
#define PMIC_OTP_18_W_ADDR(base) ((base) + (0x01AB))
#define PMIC_OTP_19_W_ADDR(base) ((base) + (0x01AC))
#define PMIC_OTP_20_W_ADDR(base) ((base) + (0x01AD))
#define PMIC_OTP_21_W_ADDR(base) ((base) + (0x01AE))
#define PMIC_OTP_22_W_ADDR(base) ((base) + (0x01AF))
#define PMIC_OTP_23_W_ADDR(base) ((base) + (0x01B0))
#define PMIC_OTP_24_W_ADDR(base) ((base) + (0x01B1))
#define PMIC_OTP_25_W_ADDR(base) ((base) + (0x01B2))
#define PMIC_OTP_26_W_ADDR(base) ((base) + (0x01B3))
#define PMIC_OTP_27_W_ADDR(base) ((base) + (0x01B4))
#define PMIC_OTP_28_W_ADDR(base) ((base) + (0x01B5))
#define PMIC_OTP_29_W_ADDR(base) ((base) + (0x01B6))
#define PMIC_OTP_30_W_ADDR(base) ((base) + (0x01B7))
#define PMIC_OTP_31_W_ADDR(base) ((base) + (0x01B8))
#define PMIC_OTP_32_W_ADDR(base) ((base) + (0x01B9))
#define PMIC_OTP_33_W_ADDR(base) ((base) + (0x01BA))
#define PMIC_OTP_34_W_ADDR(base) ((base) + (0x01BB))
#define PMIC_OTP_35_W_ADDR(base) ((base) + (0x01BC))
#define PMIC_OTP_36_W_ADDR(base) ((base) + (0x01BD))
#define PMIC_OTP_37_W_ADDR(base) ((base) + (0x01BE))
#define PMIC_OTP_38_W_ADDR(base) ((base) + (0x01BF))
#define PMIC_OTP_39_W_ADDR(base) ((base) + (0x01C0))
#define PMIC_OTP_40_W_ADDR(base) ((base) + (0x01C1))
#define PMIC_OTP_41_W_ADDR(base) ((base) + (0x01C2))
#define PMIC_OTP_42_W_ADDR(base) ((base) + (0x01C3))
#define PMIC_OTP_43_W_ADDR(base) ((base) + (0x01C4))
#define PMIC_OTP_44_W_ADDR(base) ((base) + (0x01C5))
#define PMIC_OTP_45_W_ADDR(base) ((base) + (0x01C6))
#define PMIC_D2A_RES0_ADDR(base) ((base) + (0x01CA))
#define PMIC_D2A_RES1_ADDR(base) ((base) + (0x01CB))
#define PMIC_A2D_RES0_ADDR(base) ((base) + (0x01CC))
#define PMIC_SIM_CTRL0_ADDR(base) ((base) + (0x01CD))
#define PMIC_SIM_CTRL1_ADDR(base) ((base) + (0x01CE))
#define PMIC_SIM_DEB_CTRL1_ADDR(base) ((base) + (0x01CF))
#define PMIC_SIM_DEB_CTRL2_ADDR(base) ((base) + (0x01D0))
#define PMIC_GPIO0DATA_ADDR(base) ((base) + (0x01D1))
#define PMIC_GPIO0_DATA_RESERVE_ADDR(base) ((base) + (0x01D2))
#define PMIC_GPIO0DIR_ADDR(base) ((base) + (0x01D3))
#define PMIC_GPIO0_DIR_RESERVE_ADDR(base) ((base) + (0x01D4))
#define PMIC_GPIO0IS_ADDR(base) ((base) + (0x01D5))
#define PMIC_GPIO0IBE_ADDR(base) ((base) + (0x01D6))
#define PMIC_GPIO0IEV_ADDR(base) ((base) + (0x01D7))
#define PMIC_GPIO0AFSEL_ADDR(base) ((base) + (0x01D8))
#define PMIC_GPIO0DSSEL_ADDR(base) ((base) + (0x01D9))
#define PMIC_GPIO0PUSEL_ADDR(base) ((base) + (0x01DA))
#define PMIC_GPIO0PDSEL_ADDR(base) ((base) + (0x01DB))
#define PMIC_GPIO0DEBSEL_ADDR(base) ((base) + (0x01DC))
#define PMIC_SIM0PUSEL_ADDR(base) ((base) + (0x01DD))
#define PMIC_SIM0PDSEL_ADDR(base) ((base) + (0x01DE))
#define PMIC_GPIO1DATA_ADDR(base) ((base) + (0x01DF))
#define PMIC_GPIO1_DATA_RESERVE_ADDR(base) ((base) + (0x01E0))
#define PMIC_GPIO1DIR_ADDR(base) ((base) + (0x01E1))
#define PMIC_GPIO1_DIR_RESERVE_ADDR(base) ((base) + (0x01E2))
#define PMIC_GPIO1IS_ADDR(base) ((base) + (0x01E3))
#define PMIC_GPIO1IBE_ADDR(base) ((base) + (0x01E4))
#define PMIC_GPIO1IEV_ADDR(base) ((base) + (0x01E5))
#define PMIC_GPIO1AFSEL_ADDR(base) ((base) + (0x01E6))
#define PMIC_GPIO1DSSEL_ADDR(base) ((base) + (0x01E7))
#define PMIC_GPIO1PUSEL_ADDR(base) ((base) + (0x01E8))
#define PMIC_GPIO1PDSEL_ADDR(base) ((base) + (0x01E9))
#define PMIC_GPIO1DEBSEL_ADDR(base) ((base) + (0x01EA))
#define PMIC_SIM1PUSEL_ADDR(base) ((base) + (0x01EB))
#define PMIC_SIM1PDSEL_ADDR(base) ((base) + (0x01EC))
#define PMIC_GPIO2DATA_ADDR(base) ((base) + (0x01ED))
#define PMIC_GPIO2_DATA_RESERVE_ADDR(base) ((base) + (0x01EE))
#define PMIC_GPIO2DIR_ADDR(base) ((base) + (0x01EF))
#define PMIC_GPIO2_DIR_RESERVE_ADDR(base) ((base) + (0x01F0))
#define PMIC_GPIO2IS_ADDR(base) ((base) + (0x01F1))
#define PMIC_GPIO2IBE_ADDR(base) ((base) + (0x01F2))
#define PMIC_GPIO2IEV_ADDR(base) ((base) + (0x01F3))
#define PMIC_GPIO2DSSEL_ADDR(base) ((base) + (0x01F4))
#define PMIC_GPIO2PUSEL_ADDR(base) ((base) + (0x01F5))
#define PMIC_GPIO2PDSEL_ADDR(base) ((base) + (0x01F6))
#define PMIC_GPIO2DEBSEL_ADDR(base) ((base) + (0x01F7))
#define PMIC_GPIO3DATA_ADDR(base) ((base) + (0x01F8))
#define PMIC_GPIO3_DATA_RESERVE_ADDR(base) ((base) + (0x01F9))
#define PMIC_GPIO3DIR_ADDR(base) ((base) + (0x01FA))
#define PMIC_GPIO3_DIR_RESERVE_ADDR(base) ((base) + (0x01FB))
#define PMIC_GPIO3IS_ADDR(base) ((base) + (0x01FC))
#define PMIC_GPIO3IBE_ADDR(base) ((base) + (0x01FD))
#define PMIC_GPIO3IEV_ADDR(base) ((base) + (0x01FE))
#define PMIC_GPIO3DSSEL_ADDR(base) ((base) + (0x01FF))
#define PMIC_GPIO3PUSEL_ADDR(base) ((base) + (0x0200))
#define PMIC_GPIO3PDSEL_ADDR(base) ((base) + (0x0201))
#define PMIC_GPIO3DEBSEL_ADDR(base) ((base) + (0x0202))
#define PMIC_AUX_IBIAS_CFG_ADDR(base) ((base) + (0x0203))
#define PMIC_DAC_DIN_MSB_ADDR(base) ((base) + (0x0204))
#define PMIC_DAC_DIN_LSB_ADDR(base) ((base) + (0x0205))
#define PMIC_RAMP_BUCK0_CTRL0_ADDR(base) ((base) + (0x0206))
#define PMIC_RAMP_BUCK0_CTRL1_ADDR(base) ((base) + (0x0207))
#define PMIC_BUCK0_RAMPUP_STATE_ADDR(base) ((base) + (0x0208))
#define PMIC_BUCK0_RAMPDOWN_STATE_ADDR(base) ((base) + (0x0209))
#define PMIC_BUCK0_RAMP_CHG_CTRL_ADDR(base) ((base) + (0x020A))
#define PMIC_RAMP_BUCK4_CTRL0_ADDR(base) ((base) + (0x020B))
#define PMIC_RAMP_BUCK4_CTRL1_ADDR(base) ((base) + (0x020C))
#define PMIC_BUCK4_RAMPUP_STATE_ADDR(base) ((base) + (0x020D))
#define PMIC_BUCK4_RAMPDOWN_STATE_ADDR(base) ((base) + (0x020E))
#define PMIC_BUCK4_RAMP_CHG_CTRL_ADDR(base) ((base) + (0x020F))
#define PMIC_RAMP_BUCK5_CTRL0_ADDR(base) ((base) + (0x0210))
#define PMIC_RAMP_BUCK5_CTRL1_ADDR(base) ((base) + (0x0211))
#define PMIC_BUCK5_RAMPUP_STATE_ADDR(base) ((base) + (0x0212))
#define PMIC_BUCK5_RAMPDOWN_STATE_ADDR(base) ((base) + (0x0213))
#define PMIC_BUCK5_RAMP_CHG_CTRL_ADDR(base) ((base) + (0x0214))
#define PMIC_RAMP_LDO0_2_CTRL0_ADDR(base) ((base) + (0x0215))
#define PMIC_RAMP_LDO0_2_CTRL1_ADDR(base) ((base) + (0x0216))
#define PMIC_RAMP_LDO40_CTRL0_ADDR(base) ((base) + (0x0217))
#define PMIC_RAMP_LDO40_CTRL1_ADDR(base) ((base) + (0x0218))
#define PMIC_DIG_IO_DS_32K_CFG_ADDR(base) ((base) + (0x0219))
#define PMIC_DIG_IO_DS_CODEC_CFG_ADDR(base) ((base) + (0x021A))
#define PMIC_PULL_DOWN_CFG_ADDR(base) ((base) + (0x021B))
#define PMIC_SPMI_ECO_GT_BYPASS_ADDR(base) ((base) + (0x021C))
#define PMIC_PWRONN_8S_XOADC_DEBUG_CTRL_ADDR(base) ((base) + (0x021D))
#define PMIC_DIS_19M2_CTRL_ADDR(base) ((base) + (0x021E))
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_CLK_ABB_CTRL1_ADDR(base) ((base) + (0x222UL))
#define PMIC_CLK_WIFI_CTRL1_ADDR(base) ((base) + (0x223UL))
#define PMIC_CLK_NFC_CTRL1_ADDR(base) ((base) + (0x224UL))
#define PMIC_CLK_RF0_CTRL1_ADDR(base) ((base) + (0x225UL))
#define PMIC_CLK_TOP_CTRL1_0_ADDR(base) ((base) + (0x226UL))
#define PMIC_CLK_TOP_CTRL1_1_ADDR(base) ((base) + (0x227UL))
#define PMIC_CLK_256K_CTRL0_ADDR(base) ((base) + (0x228UL))
#define PMIC_CLK_256K_CTRL1_ADDR(base) ((base) + (0x229UL))
#define PMIC_CLK_UFS_FRE_CTRL1_ADDR(base) ((base) + (0x22AUL))
#define PMIC_VSYS_LOW_SET_ADDR(base) ((base) + (0x22BUL))
#define PMIC_VSYS_DROP_SET_ADDR(base) ((base) + (0x22CUL))
#define PMIC_NFC_ON_CTRL_ADDR(base) ((base) + (0x22DUL))
#define PMIC_HRESET_PWRDOWN_CTRL_ADDR(base) ((base) + (0x22EUL))
#define PMIC_SMPL_CTRL_ADDR(base) ((base) + (0x22FUL))
#define PMIC_SYS_CTRL0_ADDR(base) ((base) + (0x230UL))
#define PMIC_SYS_CTRL1_ADDR(base) ((base) + (0x2EAUL))
#define PMIC_DEBUG_LOCK_ADDR(base) ((base) + (0x232UL))
#define PMIC_SYS_DEBUG0_ADDR(base) ((base) + (0x233UL))
#define PMIC_SYS_DEBUG1_ADDR(base) ((base) + (0x234UL))
#define PMIC_SYS_DEBUG2_ADDR(base) ((base) + (0x235UL))
#define PMIC_SYS_DEBUG3_ADDR(base) ((base) + (0x236UL))
#define PMIC_SYS_DEBUG4_ADDR(base) ((base) + (0x237UL))
#define PMIC_SYS_DEBUG5_ADDR(base) ((base) + (0x238UL))
#define PMIC_BACKUP_CHG_ADDR(base) ((base) + (0x239UL))
#define PMIC_NP_CTRL_ADDR(base) ((base) + (0x23AUL))
#define PMIC_NP_D2A_RES0_ADDR(base) ((base) + (0x23BUL))
#define PMIC_NP_D2A_RES1_ADDR(base) ((base) + (0x23CUL))
#define PMIC_HRST_REG0_ADDR(base) ((base) + (0x23DUL))
#define PMIC_HRST_REG1_ADDR(base) ((base) + (0x23EUL))
#define PMIC_HRST_REG2_ADDR(base) ((base) + (0x23FUL))
#define PMIC_HRST_REG3_ADDR(base) ((base) + (0x240UL))
#define PMIC_HRST_REG4_ADDR(base) ((base) + (0x241UL))
#define PMIC_HRST_REG5_ADDR(base) ((base) + (0x242UL))
#define PMIC_HRST_REG6_ADDR(base) ((base) + (0x243UL))
#define PMIC_HRST_REG7_ADDR(base) ((base) + (0x244UL))
#define PMIC_HRST_REG8_ADDR(base) ((base) + (0x245UL))
#define PMIC_HRST_REG9_ADDR(base) ((base) + (0x246UL))
#define PMIC_HRST_REG10_ADDR(base) ((base) + (0x247UL))
#define PMIC_HRST_REG11_ADDR(base) ((base) + (0x248UL))
#define PMIC_HRST_REG12_ADDR(base) ((base) + (0x249UL))
#define PMIC_HRST_REG13_ADDR(base) ((base) + (0x24AUL))
#define PMIC_HRST_REG14_ADDR(base) ((base) + (0x24BUL))
#define PMIC_HRST_REG15_ADDR(base) ((base) + (0x24CUL))
#define PMIC_HRST_REG16_ADDR(base) ((base) + (0x24DUL))
#define PMIC_HRST_REG17_ADDR(base) ((base) + (0x24EUL))
#define PMIC_HRST_REG18_ADDR(base) ((base) + (0x24FUL))
#define PMIC_HRST_REG19_ADDR(base) ((base) + (0x250UL))
#define PMIC_HRST_REG20_ADDR(base) ((base) + (0x251UL))
#define PMIC_HRST_REG21_ADDR(base) ((base) + (0x252UL))
#define PMIC_HRST_REG22_ADDR(base) ((base) + (0x253UL))
#define PMIC_HRST_REG23_ADDR(base) ((base) + (0x254UL))
#define PMIC_HRST_REG24_ADDR(base) ((base) + (0x255UL))
#define PMIC_HRST_REG25_ADDR(base) ((base) + (0x256UL))
#define PMIC_HRST_REG26_ADDR(base) ((base) + (0x257UL))
#define PMIC_HRST_REG27_ADDR(base) ((base) + (0x258UL))
#define PMIC_HRST_REG28_ADDR(base) ((base) + (0x259UL))
#define PMIC_HRST_REG29_ADDR(base) ((base) + (0x25AUL))
#define PMIC_HRST_REG30_ADDR(base) ((base) + (0x25BUL))
#define PMIC_HRST_REG31_ADDR(base) ((base) + (0x25CUL))
#define PMIC_OTP_0_R_ADDR(base) ((base) + (0x25DUL))
#define PMIC_OTP_1_R_ADDR(base) ((base) + (0x25EUL))
#define PMIC_OTP_2_R_ADDR(base) ((base) + (0x25FUL))
#define PMIC_OTP_3_R_ADDR(base) ((base) + (0x260UL))
#define PMIC_OTP_4_R_ADDR(base) ((base) + (0x261UL))
#define PMIC_OTP_5_R_ADDR(base) ((base) + (0x262UL))
#define PMIC_OTP_6_R_ADDR(base) ((base) + (0x263UL))
#define PMIC_OTP_7_R_ADDR(base) ((base) + (0x264UL))
#define PMIC_OTP_8_R_ADDR(base) ((base) + (0x265UL))
#define PMIC_OTP_9_R_ADDR(base) ((base) + (0x266UL))
#define PMIC_OTP_10_R_ADDR(base) ((base) + (0x267UL))
#define PMIC_OTP_11_R_ADDR(base) ((base) + (0x268UL))
#define PMIC_OTP_12_R_ADDR(base) ((base) + (0x269UL))
#define PMIC_OTP_13_R_ADDR(base) ((base) + (0x26AUL))
#define PMIC_OTP_14_R_ADDR(base) ((base) + (0x26BUL))
#define PMIC_OTP_15_R_ADDR(base) ((base) + (0x26CUL))
#define PMIC_OTP_16_R_ADDR(base) ((base) + (0x26DUL))
#define PMIC_OTP_17_R_ADDR(base) ((base) + (0x26EUL))
#define PMIC_OTP_18_R_ADDR(base) ((base) + (0x26FUL))
#define PMIC_OTP_19_R_ADDR(base) ((base) + (0x270UL))
#define PMIC_OTP_20_R_ADDR(base) ((base) + (0x271UL))
#define PMIC_OTP_21_R_ADDR(base) ((base) + (0x272UL))
#define PMIC_OTP_22_R_ADDR(base) ((base) + (0x273UL))
#define PMIC_OTP_23_R_ADDR(base) ((base) + (0x274UL))
#define PMIC_OTP_24_R_ADDR(base) ((base) + (0x275UL))
#define PMIC_OTP_25_R_ADDR(base) ((base) + (0x276UL))
#define PMIC_OTP_26_R_ADDR(base) ((base) + (0x277UL))
#define PMIC_OTP_27_R_ADDR(base) ((base) + (0x278UL))
#define PMIC_OTP_28_R_ADDR(base) ((base) + (0x279UL))
#define PMIC_OTP_29_R_ADDR(base) ((base) + (0x27AUL))
#define PMIC_OTP_30_R_ADDR(base) ((base) + (0x27BUL))
#define PMIC_OTP_31_R_ADDR(base) ((base) + (0x27CUL))
#define PMIC_OTP_32_R_ADDR(base) ((base) + (0x27DUL))
#define PMIC_OTP_33_R_ADDR(base) ((base) + (0x27EUL))
#define PMIC_OTP_34_R_ADDR(base) ((base) + (0x27FUL))
#define PMIC_OTP_35_R_ADDR(base) ((base) + (0x280UL))
#define PMIC_OTP_36_R_ADDR(base) ((base) + (0x281UL))
#define PMIC_OTP_37_R_ADDR(base) ((base) + (0x282UL))
#define PMIC_OTP_38_R_ADDR(base) ((base) + (0x283UL))
#define PMIC_OTP_39_R_ADDR(base) ((base) + (0x284UL))
#define PMIC_OTP_40_R_ADDR(base) ((base) + (0x285UL))
#define PMIC_OTP_41_R_ADDR(base) ((base) + (0x286UL))
#define PMIC_OTP_42_R_ADDR(base) ((base) + (0x287UL))
#define PMIC_OTP_43_R_ADDR(base) ((base) + (0x288UL))
#define PMIC_OTP_44_R_ADDR(base) ((base) + (0x289UL))
#define PMIC_OTP_45_R_ADDR(base) ((base) + (0x28AUL))
#define PMIC_OTP_46_R_ADDR(base) ((base) + (0x28BUL))
#define PMIC_OTP_47_R_ADDR(base) ((base) + (0x28CUL))
#define PMIC_OTP_48_R_ADDR(base) ((base) + (0x28DUL))
#define PMIC_OTP_49_R_ADDR(base) ((base) + (0x28EUL))
#define PMIC_OTP_50_R_ADDR(base) ((base) + (0x28FUL))
#define PMIC_OTP_51_R_ADDR(base) ((base) + (0x290UL))
#define PMIC_OTP_52_R_ADDR(base) ((base) + (0x291UL))
#define PMIC_OTP_53_R_ADDR(base) ((base) + (0x292UL))
#define PMIC_OTP_54_R_ADDR(base) ((base) + (0x293UL))
#define PMIC_OTP_55_R_ADDR(base) ((base) + (0x294UL))
#define PMIC_OTP_56_R_ADDR(base) ((base) + (0x295UL))
#define PMIC_OTP_57_R_ADDR(base) ((base) + (0x296UL))
#define PMIC_OTP_58_R_ADDR(base) ((base) + (0x297UL))
#define PMIC_OTP_59_R_ADDR(base) ((base) + (0x298UL))
#define PMIC_OTP_60_R_ADDR(base) ((base) + (0x299UL))
#define PMIC_OTP_61_R_ADDR(base) ((base) + (0x29AUL))
#define PMIC_OTP_62_R_ADDR(base) ((base) + (0x29BUL))
#define PMIC_OTP_63_R_ADDR(base) ((base) + (0x29CUL))
#define PMIC_EN_PMUA_PMUD_ADDR(base) ((base) + (0x29DUL))
#define PMIC_PMUD_VSET_ADDR(base) ((base) + (0x29EUL))
#else
#define PMIC_CLK_ABB_CTRL1_ADDR(base) ((base) + (0x222))
#define PMIC_CLK_WIFI_CTRL1_ADDR(base) ((base) + (0x223))
#define PMIC_CLK_NFC_CTRL1_ADDR(base) ((base) + (0x224))
#define PMIC_CLK_RF0_CTRL1_ADDR(base) ((base) + (0x225))
#define PMIC_CLK_TOP_CTRL1_0_ADDR(base) ((base) + (0x226))
#define PMIC_CLK_TOP_CTRL1_1_ADDR(base) ((base) + (0x227))
#define PMIC_CLK_256K_CTRL0_ADDR(base) ((base) + (0x228))
#define PMIC_CLK_256K_CTRL1_ADDR(base) ((base) + (0x229))
#define PMIC_CLK_UFS_FRE_CTRL1_ADDR(base) ((base) + (0x22A))
#define PMIC_VSYS_LOW_SET_ADDR(base) ((base) + (0x22B))
#define PMIC_VSYS_DROP_SET_ADDR(base) ((base) + (0x22C))
#define PMIC_NFC_ON_CTRL_ADDR(base) ((base) + (0x22D))
#define PMIC_HRESET_PWRDOWN_CTRL_ADDR(base) ((base) + (0x22E))
#define PMIC_SMPL_CTRL_ADDR(base) ((base) + (0x22F))
#define PMIC_SYS_CTRL0_ADDR(base) ((base) + (0x230))
#define PMIC_SYS_CTRL1_ADDR(base) ((base) + (0x2EA))
#define PMIC_DEBUG_LOCK_ADDR(base) ((base) + (0x232))
#define PMIC_SYS_DEBUG0_ADDR(base) ((base) + (0x233))
#define PMIC_SYS_DEBUG1_ADDR(base) ((base) + (0x234))
#define PMIC_SYS_DEBUG2_ADDR(base) ((base) + (0x235))
#define PMIC_SYS_DEBUG3_ADDR(base) ((base) + (0x236))
#define PMIC_SYS_DEBUG4_ADDR(base) ((base) + (0x237))
#define PMIC_SYS_DEBUG5_ADDR(base) ((base) + (0x238))
#define PMIC_BACKUP_CHG_ADDR(base) ((base) + (0x239))
#define PMIC_NP_CTRL_ADDR(base) ((base) + (0x23A))
#define PMIC_NP_D2A_RES0_ADDR(base) ((base) + (0x23B))
#define PMIC_NP_D2A_RES1_ADDR(base) ((base) + (0x23C))
#define PMIC_HRST_REG0_ADDR(base) ((base) + (0x23D))
#define PMIC_HRST_REG1_ADDR(base) ((base) + (0x23E))
#define PMIC_HRST_REG2_ADDR(base) ((base) + (0x23F))
#define PMIC_HRST_REG3_ADDR(base) ((base) + (0x240))
#define PMIC_HRST_REG4_ADDR(base) ((base) + (0x241))
#define PMIC_HRST_REG5_ADDR(base) ((base) + (0x242))
#define PMIC_HRST_REG6_ADDR(base) ((base) + (0x243))
#define PMIC_HRST_REG7_ADDR(base) ((base) + (0x244))
#define PMIC_HRST_REG8_ADDR(base) ((base) + (0x245))
#define PMIC_HRST_REG9_ADDR(base) ((base) + (0x246))
#define PMIC_HRST_REG10_ADDR(base) ((base) + (0x247))
#define PMIC_HRST_REG11_ADDR(base) ((base) + (0x248))
#define PMIC_HRST_REG12_ADDR(base) ((base) + (0x249))
#define PMIC_HRST_REG13_ADDR(base) ((base) + (0x24A))
#define PMIC_HRST_REG14_ADDR(base) ((base) + (0x24B))
#define PMIC_HRST_REG15_ADDR(base) ((base) + (0x24C))
#define PMIC_HRST_REG16_ADDR(base) ((base) + (0x24D))
#define PMIC_HRST_REG17_ADDR(base) ((base) + (0x24E))
#define PMIC_HRST_REG18_ADDR(base) ((base) + (0x24F))
#define PMIC_HRST_REG19_ADDR(base) ((base) + (0x250))
#define PMIC_HRST_REG20_ADDR(base) ((base) + (0x251))
#define PMIC_HRST_REG21_ADDR(base) ((base) + (0x252))
#define PMIC_HRST_REG22_ADDR(base) ((base) + (0x253))
#define PMIC_HRST_REG23_ADDR(base) ((base) + (0x254))
#define PMIC_HRST_REG24_ADDR(base) ((base) + (0x255))
#define PMIC_HRST_REG25_ADDR(base) ((base) + (0x256))
#define PMIC_HRST_REG26_ADDR(base) ((base) + (0x257))
#define PMIC_HRST_REG27_ADDR(base) ((base) + (0x258))
#define PMIC_HRST_REG28_ADDR(base) ((base) + (0x259))
#define PMIC_HRST_REG29_ADDR(base) ((base) + (0x25A))
#define PMIC_HRST_REG30_ADDR(base) ((base) + (0x25B))
#define PMIC_HRST_REG31_ADDR(base) ((base) + (0x25C))
#define PMIC_OTP_0_R_ADDR(base) ((base) + (0x25D))
#define PMIC_OTP_1_R_ADDR(base) ((base) + (0x25E))
#define PMIC_OTP_2_R_ADDR(base) ((base) + (0x25F))
#define PMIC_OTP_3_R_ADDR(base) ((base) + (0x260))
#define PMIC_OTP_4_R_ADDR(base) ((base) + (0x261))
#define PMIC_OTP_5_R_ADDR(base) ((base) + (0x262))
#define PMIC_OTP_6_R_ADDR(base) ((base) + (0x263))
#define PMIC_OTP_7_R_ADDR(base) ((base) + (0x264))
#define PMIC_OTP_8_R_ADDR(base) ((base) + (0x265))
#define PMIC_OTP_9_R_ADDR(base) ((base) + (0x266))
#define PMIC_OTP_10_R_ADDR(base) ((base) + (0x267))
#define PMIC_OTP_11_R_ADDR(base) ((base) + (0x268))
#define PMIC_OTP_12_R_ADDR(base) ((base) + (0x269))
#define PMIC_OTP_13_R_ADDR(base) ((base) + (0x26A))
#define PMIC_OTP_14_R_ADDR(base) ((base) + (0x26B))
#define PMIC_OTP_15_R_ADDR(base) ((base) + (0x26C))
#define PMIC_OTP_16_R_ADDR(base) ((base) + (0x26D))
#define PMIC_OTP_17_R_ADDR(base) ((base) + (0x26E))
#define PMIC_OTP_18_R_ADDR(base) ((base) + (0x26F))
#define PMIC_OTP_19_R_ADDR(base) ((base) + (0x270))
#define PMIC_OTP_20_R_ADDR(base) ((base) + (0x271))
#define PMIC_OTP_21_R_ADDR(base) ((base) + (0x272))
#define PMIC_OTP_22_R_ADDR(base) ((base) + (0x273))
#define PMIC_OTP_23_R_ADDR(base) ((base) + (0x274))
#define PMIC_OTP_24_R_ADDR(base) ((base) + (0x275))
#define PMIC_OTP_25_R_ADDR(base) ((base) + (0x276))
#define PMIC_OTP_26_R_ADDR(base) ((base) + (0x277))
#define PMIC_OTP_27_R_ADDR(base) ((base) + (0x278))
#define PMIC_OTP_28_R_ADDR(base) ((base) + (0x279))
#define PMIC_OTP_29_R_ADDR(base) ((base) + (0x27A))
#define PMIC_OTP_30_R_ADDR(base) ((base) + (0x27B))
#define PMIC_OTP_31_R_ADDR(base) ((base) + (0x27C))
#define PMIC_OTP_32_R_ADDR(base) ((base) + (0x27D))
#define PMIC_OTP_33_R_ADDR(base) ((base) + (0x27E))
#define PMIC_OTP_34_R_ADDR(base) ((base) + (0x27F))
#define PMIC_OTP_35_R_ADDR(base) ((base) + (0x280))
#define PMIC_OTP_36_R_ADDR(base) ((base) + (0x281))
#define PMIC_OTP_37_R_ADDR(base) ((base) + (0x282))
#define PMIC_OTP_38_R_ADDR(base) ((base) + (0x283))
#define PMIC_OTP_39_R_ADDR(base) ((base) + (0x284))
#define PMIC_OTP_40_R_ADDR(base) ((base) + (0x285))
#define PMIC_OTP_41_R_ADDR(base) ((base) + (0x286))
#define PMIC_OTP_42_R_ADDR(base) ((base) + (0x287))
#define PMIC_OTP_43_R_ADDR(base) ((base) + (0x288))
#define PMIC_OTP_44_R_ADDR(base) ((base) + (0x289))
#define PMIC_OTP_45_R_ADDR(base) ((base) + (0x28A))
#define PMIC_OTP_46_R_ADDR(base) ((base) + (0x28B))
#define PMIC_OTP_47_R_ADDR(base) ((base) + (0x28C))
#define PMIC_OTP_48_R_ADDR(base) ((base) + (0x28D))
#define PMIC_OTP_49_R_ADDR(base) ((base) + (0x28E))
#define PMIC_OTP_50_R_ADDR(base) ((base) + (0x28F))
#define PMIC_OTP_51_R_ADDR(base) ((base) + (0x290))
#define PMIC_OTP_52_R_ADDR(base) ((base) + (0x291))
#define PMIC_OTP_53_R_ADDR(base) ((base) + (0x292))
#define PMIC_OTP_54_R_ADDR(base) ((base) + (0x293))
#define PMIC_OTP_55_R_ADDR(base) ((base) + (0x294))
#define PMIC_OTP_56_R_ADDR(base) ((base) + (0x295))
#define PMIC_OTP_57_R_ADDR(base) ((base) + (0x296))
#define PMIC_OTP_58_R_ADDR(base) ((base) + (0x297))
#define PMIC_OTP_59_R_ADDR(base) ((base) + (0x298))
#define PMIC_OTP_60_R_ADDR(base) ((base) + (0x299))
#define PMIC_OTP_61_R_ADDR(base) ((base) + (0x29A))
#define PMIC_OTP_62_R_ADDR(base) ((base) + (0x29B))
#define PMIC_OTP_63_R_ADDR(base) ((base) + (0x29C))
#define PMIC_EN_PMUA_PMUD_ADDR(base) ((base) + (0x29D))
#define PMIC_PMUD_VSET_ADDR(base) ((base) + (0x29E))
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_IRQ_MASK_0_ADDR(base) ((base) + (0x02A2UL))
#define PMIC_IRQ_MASK_1_ADDR(base) ((base) + (0x02A3UL))
#define PMIC_IRQ_MASK_2_ADDR(base) ((base) + (0x02A4UL))
#define PMIC_IRQ_MASK_3_ADDR(base) ((base) + (0x02A5UL))
#define PMIC_IRQ_MASK_4_ADDR(base) ((base) + (0x02A6UL))
#define PMIC_IRQ_MASK_5_ADDR(base) ((base) + (0x02A7UL))
#define PMIC_IRQ_MASK_6_ADDR(base) ((base) + (0x02A8UL))
#define PMIC_IRQ_MASK_7_ADDR(base) ((base) + (0x02A9UL))
#define PMIC_IRQ_MASK_8_ADDR(base) ((base) + (0x02AAUL))
#define PMIC_IRQ_MASK_9_ADDR(base) ((base) + (0x02ACUL))
#define PMIC_IRQ_MASK_10_ADDR(base) ((base) + (0x02ADUL))
#define PMIC_IRQ_MASK_11_ADDR(base) ((base) + (0x02AEUL))
#define PMIC_IRQ_MASK_12_ADDR(base) ((base) + (0x02AFUL))
#else
#define PMIC_IRQ_MASK_0_ADDR(base) ((base) + (0x02A2))
#define PMIC_IRQ_MASK_1_ADDR(base) ((base) + (0x02A3))
#define PMIC_IRQ_MASK_2_ADDR(base) ((base) + (0x02A4))
#define PMIC_IRQ_MASK_3_ADDR(base) ((base) + (0x02A5))
#define PMIC_IRQ_MASK_4_ADDR(base) ((base) + (0x02A6))
#define PMIC_IRQ_MASK_5_ADDR(base) ((base) + (0x02A7))
#define PMIC_IRQ_MASK_6_ADDR(base) ((base) + (0x02A8))
#define PMIC_IRQ_MASK_7_ADDR(base) ((base) + (0x02A9))
#define PMIC_IRQ_MASK_8_ADDR(base) ((base) + (0x02AA))
#define PMIC_IRQ_MASK_9_ADDR(base) ((base) + (0x02AC))
#define PMIC_IRQ_MASK_10_ADDR(base) ((base) + (0x02AD))
#define PMIC_IRQ_MASK_11_ADDR(base) ((base) + (0x02AE))
#define PMIC_IRQ_MASK_12_ADDR(base) ((base) + (0x02AF))
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_IRQ0_ADDR(base) ((base) + (0x02B3UL))
#define PMIC_IRQ1_ADDR(base) ((base) + (0x02B4UL))
#define PMIC_OCP_IRQ0_ADDR(base) ((base) + (0x02B5UL))
#define PMIC_OCP_IRQ1_ADDR(base) ((base) + (0x02B6UL))
#define PMIC_OCP_IRQ2_ADDR(base) ((base) + (0x02B7UL))
#define PMIC_OCP_IRQ3_ADDR(base) ((base) + (0x02B8UL))
#define PMIC_OCP_IRQ4_ADDR(base) ((base) + (0x02B9UL))
#define PMIC_OCP_IRQ5_ADDR(base) ((base) + (0x02BAUL))
#define PMIC_SCP_IRQ0_ADDR(base) ((base) + (0x02BBUL))
#define PMIC_GPIO_IRQ_ADDR(base) ((base) + (0x02BDUL))
#define PMIC_ACR_SOH_IRQ_ADDR(base) ((base) + (0x02BEUL))
#define PMIC_DCR_IRQ_ADDR(base) ((base) + (0x02BFUL))
#define PMIC_VSYS_DROP_IRQ_ADDR(base) ((base) + (0x02C0UL))
#else
#define PMIC_IRQ0_ADDR(base) ((base) + (0x02B3))
#define PMIC_IRQ1_ADDR(base) ((base) + (0x02B4))
#define PMIC_OCP_IRQ0_ADDR(base) ((base) + (0x02B5))
#define PMIC_OCP_IRQ1_ADDR(base) ((base) + (0x02B6))
#define PMIC_OCP_IRQ2_ADDR(base) ((base) + (0x02B7))
#define PMIC_OCP_IRQ3_ADDR(base) ((base) + (0x02B8))
#define PMIC_OCP_IRQ4_ADDR(base) ((base) + (0x02B9))
#define PMIC_OCP_IRQ5_ADDR(base) ((base) + (0x02BA))
#define PMIC_SCP_IRQ0_ADDR(base) ((base) + (0x02BB))
#define PMIC_GPIO_IRQ_ADDR(base) ((base) + (0x02BD))
#define PMIC_ACR_SOH_IRQ_ADDR(base) ((base) + (0x02BE))
#define PMIC_DCR_IRQ_ADDR(base) ((base) + (0x02BF))
#define PMIC_VSYS_DROP_IRQ_ADDR(base) ((base) + (0x02C0))
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_NP_OCP0_ADDR(base) ((base) + (0x02C4UL))
#define PMIC_NP_OCP1_ADDR(base) ((base) + (0x02C5UL))
#define PMIC_NP_OCP2_ADDR(base) ((base) + (0x02C6UL))
#define PMIC_NP_OCP3_ADDR(base) ((base) + (0x02C7UL))
#define PMIC_NP_OCP4_ADDR(base) ((base) + (0x02C8UL))
#define PMIC_NP_OCP5_ADDR(base) ((base) + (0x02C9UL))
#define PMIC_NP_SCP0_ADDR(base) ((base) + (0x02CAUL))
#define PMIC_NP_RECORD0_ADDR(base) ((base) + (0x02CCUL))
#define PMIC_NP_RECORD1_ADDR(base) ((base) + (0x02CDUL))
#define PMIC_NP_RECORD2_ADDR(base) ((base) + (0x02CEUL))
#define PMIC_NP_RECORD3_ADDR(base) ((base) + (0x02CFUL))
#define PMIC_NP_RECORD4_ADDR(base) ((base) + (0x02D0UL))
#define PMIC_NP_RECORD5_ADDR(base) ((base) + (0x02D1UL))
#define PMIC_RAMP_EVENT_ADDR(base) ((base) + (0x02D2UL))
#define PMIC_COUL_RECORD0_ADDR(base) ((base) + (0x02D5UL))
#define PMIC_COUL_RECORD1_ADDR(base) ((base) + (0x02D6UL))
#define PMIC_NP_VSYS_DROP_RECORD_ADDR(base) ((base) + (0x02D7UL))
#else
#define PMIC_NP_OCP0_ADDR(base) ((base) + (0x02C4))
#define PMIC_NP_OCP1_ADDR(base) ((base) + (0x02C5))
#define PMIC_NP_OCP2_ADDR(base) ((base) + (0x02C6))
#define PMIC_NP_OCP3_ADDR(base) ((base) + (0x02C7))
#define PMIC_NP_OCP4_ADDR(base) ((base) + (0x02C8))
#define PMIC_NP_OCP5_ADDR(base) ((base) + (0x02C9))
#define PMIC_NP_SCP0_ADDR(base) ((base) + (0x02CA))
#define PMIC_NP_RECORD0_ADDR(base) ((base) + (0x02CC))
#define PMIC_NP_RECORD1_ADDR(base) ((base) + (0x02CD))
#define PMIC_NP_RECORD2_ADDR(base) ((base) + (0x02CE))
#define PMIC_NP_RECORD3_ADDR(base) ((base) + (0x02CF))
#define PMIC_NP_RECORD4_ADDR(base) ((base) + (0x02D0))
#define PMIC_NP_RECORD5_ADDR(base) ((base) + (0x02D1))
#define PMIC_RAMP_EVENT_ADDR(base) ((base) + (0x02D2))
#define PMIC_COUL_RECORD0_ADDR(base) ((base) + (0x02D5))
#define PMIC_COUL_RECORD1_ADDR(base) ((base) + (0x02D6))
#define PMIC_NP_VSYS_DROP_RECORD_ADDR(base) ((base) + (0x02D7))
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_CLK_EN_CFG_ADDR(base) ((base) + (0x2F0UL))
#define PMIC_CLK_EDGE_CFG_ADDR(base) ((base) + (0x2F1UL))
#define PMIC_SIF_LOOPBACK_CFG_ADDR(base) ((base) + (0x2F2UL))
#define PMIC_DAC_CHAN_CTRL_ADDR(base) ((base) + (0x2F3UL))
#define PMIC_ADC_CHAN_CTRL_ADDR(base) ((base) + (0x2F4UL))
#define PMIC_ANA_IRQ_SIG_STAT_ADDR(base) ((base) + (0x2F5UL))
#define PMIC_ANA_IRQM_REG0_ADDR(base) ((base) + (0x2F6UL))
#define PMIC_ANA_IRQ_REG0_ADDR(base) ((base) + (0x2F7UL))
#define PMIC_DEB_CNT_HS_DET_CFG_ADDR(base) ((base) + (0x2F8UL))
#define PMIC_DEB_CNT_HS_MIC_CFG_ADDR(base) ((base) + (0x2F9UL))
#define PMIC_CODEC_ANA_RW1_ADDR(base) ((base) + (0x2FAUL))
#define PMIC_CODEC_ANA_RW2_ADDR(base) ((base) + (0x2FBUL))
#define PMIC_CODEC_ANA_RW3_ADDR(base) ((base) + (0x2FCUL))
#define PMIC_CODEC_ANA_RW4_ADDR(base) ((base) + (0x2FDUL))
#define PMIC_CODEC_ANA_RW5_ADDR(base) ((base) + (0x2FEUL))
#define PMIC_CODEC_ANA_RW6_ADDR(base) ((base) + (0x2FFUL))
#define PMIC_CODEC_ANA_RW7_ADDR(base) ((base) + (0x300UL))
#define PMIC_CODEC_ANA_RW8_ADDR(base) ((base) + (0x301UL))
#define PMIC_CODEC_ANA_RW9_ADDR(base) ((base) + (0x302UL))
#define PMIC_CODEC_ANA_RW10_ADDR(base) ((base) + (0x303UL))
#define PMIC_CODEC_ANA_RW11_ADDR(base) ((base) + (0x304UL))
#define PMIC_CODEC_ANA_RW12_ADDR(base) ((base) + (0x305UL))
#define PMIC_CODEC_ANA_RW13_ADDR(base) ((base) + (0x306UL))
#define PMIC_CODEC_ANA_RW14_ADDR(base) ((base) + (0x307UL))
#define PMIC_CODEC_ANA_RW15_ADDR(base) ((base) + (0x308UL))
#define PMIC_CODEC_ANA_RW16_ADDR(base) ((base) + (0x309UL))
#define PMIC_CODEC_ANA_RW17_ADDR(base) ((base) + (0x30AUL))
#define PMIC_CODEC_ANA_RW18_ADDR(base) ((base) + (0x30BUL))
#define PMIC_CODEC_ANA_RW19_ADDR(base) ((base) + (0x30CUL))
#define PMIC_CODEC_ANA_RW20_ADDR(base) ((base) + (0x30DUL))
#define PMIC_CODEC_ANA_RW21_ADDR(base) ((base) + (0x30EUL))
#define PMIC_CODEC_ANA_RW22_ADDR(base) ((base) + (0x30FUL))
#define PMIC_CODEC_ANA_RW23_ADDR(base) ((base) + (0x310UL))
#define PMIC_CODEC_ANA_RW24_ADDR(base) ((base) + (0x311UL))
#define PMIC_CODEC_ANA_RW25_ADDR(base) ((base) + (0x312UL))
#define PMIC_CODEC_ANA_RW26_ADDR(base) ((base) + (0x313UL))
#define PMIC_CODEC_ANA_RW27_ADDR(base) ((base) + (0x314UL))
#define PMIC_CODEC_ANA_RW28_ADDR(base) ((base) + (0x315UL))
#define PMIC_CODEC_ANA_RW29_ADDR(base) ((base) + (0x316UL))
#define PMIC_CODEC_ANA_RW30_ADDR(base) ((base) + (0x317UL))
#define PMIC_CODEC_ANA_RW31_ADDR(base) ((base) + (0x318UL))
#define PMIC_CODEC_ANA_RW32_ADDR(base) ((base) + (0x319UL))
#define PMIC_CODEC_ANA_RW33_ADDR(base) ((base) + (0x31AUL))
#define PMIC_CODEC_ANA_RW34_ADDR(base) ((base) + (0x31BUL))
#define PMIC_CODEC_ANA_RW35_ADDR(base) ((base) + (0x31CUL))
#define PMIC_CODEC_ANA_RW36_ADDR(base) ((base) + (0x31DUL))
#define PMIC_CODEC_ANA_RW37_ADDR(base) ((base) + (0x31EUL))
#define PMIC_CODEC_ANA_RW38_ADDR(base) ((base) + (0x31FUL))
#define PMIC_CODEC_ANA_RW39_ADDR(base) ((base) + (0x320UL))
#define PMIC_CODEC_ANA_RW40_ADDR(base) ((base) + (0x321UL))
#define PMIC_CODEC_ANA_RW41_ADDR(base) ((base) + (0x322UL))
#define PMIC_CODEC_ANA_RW42_ADDR(base) ((base) + (0x323UL))
#define PMIC_CODEC_ANA_RW43_ADDR(base) ((base) + (0x324UL))
#define PMIC_CODEC_ANA_RW44_ADDR(base) ((base) + (0x325UL))
#define PMIC_CODEC_ANA_RW45_ADDR(base) ((base) + (0x326UL))
#define PMIC_CODEC_ANA_RW46_ADDR(base) ((base) + (0x327UL))
#define PMIC_CODEC_ANA_RW47_ADDR(base) ((base) + (0x328UL))
#define PMIC_CODEC_ANA_RW48_ADDR(base) ((base) + (0x329UL))
#define PMIC_CODEC_ANA_RW49_ADDR(base) ((base) + (0x32AUL))
#define PMIC_CODEC_ANA_RW50_ADDR(base) ((base) + (0x32BUL))
#define PMIC_CODEC_ANA_RW51_ADDR(base) ((base) + (0x32CUL))
#define PMIC_CODEC_ANA_RW52_ADDR(base) ((base) + (0x32DUL))
#define PMIC_CODEC_ANA_RO01_ADDR(base) ((base) + (0x32EUL))
#define PMIC_CODEC_ANA_RO02_ADDR(base) ((base) + (0x32FUL))
#else
#define PMIC_CLK_EN_CFG_ADDR(base) ((base) + (0x2F0))
#define PMIC_CLK_EDGE_CFG_ADDR(base) ((base) + (0x2F1))
#define PMIC_SIF_LOOPBACK_CFG_ADDR(base) ((base) + (0x2F2))
#define PMIC_DAC_CHAN_CTRL_ADDR(base) ((base) + (0x2F3))
#define PMIC_ADC_CHAN_CTRL_ADDR(base) ((base) + (0x2F4))
#define PMIC_ANA_IRQ_SIG_STAT_ADDR(base) ((base) + (0x2F5))
#define PMIC_ANA_IRQM_REG0_ADDR(base) ((base) + (0x2F6))
#define PMIC_ANA_IRQ_REG0_ADDR(base) ((base) + (0x2F7))
#define PMIC_DEB_CNT_HS_DET_CFG_ADDR(base) ((base) + (0x2F8))
#define PMIC_DEB_CNT_HS_MIC_CFG_ADDR(base) ((base) + (0x2F9))
#define PMIC_CODEC_ANA_RW1_ADDR(base) ((base) + (0x2FA))
#define PMIC_CODEC_ANA_RW2_ADDR(base) ((base) + (0x2FB))
#define PMIC_CODEC_ANA_RW3_ADDR(base) ((base) + (0x2FC))
#define PMIC_CODEC_ANA_RW4_ADDR(base) ((base) + (0x2FD))
#define PMIC_CODEC_ANA_RW5_ADDR(base) ((base) + (0x2FE))
#define PMIC_CODEC_ANA_RW6_ADDR(base) ((base) + (0x2FF))
#define PMIC_CODEC_ANA_RW7_ADDR(base) ((base) + (0x300))
#define PMIC_CODEC_ANA_RW8_ADDR(base) ((base) + (0x301))
#define PMIC_CODEC_ANA_RW9_ADDR(base) ((base) + (0x302))
#define PMIC_CODEC_ANA_RW10_ADDR(base) ((base) + (0x303))
#define PMIC_CODEC_ANA_RW11_ADDR(base) ((base) + (0x304))
#define PMIC_CODEC_ANA_RW12_ADDR(base) ((base) + (0x305))
#define PMIC_CODEC_ANA_RW13_ADDR(base) ((base) + (0x306))
#define PMIC_CODEC_ANA_RW14_ADDR(base) ((base) + (0x307))
#define PMIC_CODEC_ANA_RW15_ADDR(base) ((base) + (0x308))
#define PMIC_CODEC_ANA_RW16_ADDR(base) ((base) + (0x309))
#define PMIC_CODEC_ANA_RW17_ADDR(base) ((base) + (0x30A))
#define PMIC_CODEC_ANA_RW18_ADDR(base) ((base) + (0x30B))
#define PMIC_CODEC_ANA_RW19_ADDR(base) ((base) + (0x30C))
#define PMIC_CODEC_ANA_RW20_ADDR(base) ((base) + (0x30D))
#define PMIC_CODEC_ANA_RW21_ADDR(base) ((base) + (0x30E))
#define PMIC_CODEC_ANA_RW22_ADDR(base) ((base) + (0x30F))
#define PMIC_CODEC_ANA_RW23_ADDR(base) ((base) + (0x310))
#define PMIC_CODEC_ANA_RW24_ADDR(base) ((base) + (0x311))
#define PMIC_CODEC_ANA_RW25_ADDR(base) ((base) + (0x312))
#define PMIC_CODEC_ANA_RW26_ADDR(base) ((base) + (0x313))
#define PMIC_CODEC_ANA_RW27_ADDR(base) ((base) + (0x314))
#define PMIC_CODEC_ANA_RW28_ADDR(base) ((base) + (0x315))
#define PMIC_CODEC_ANA_RW29_ADDR(base) ((base) + (0x316))
#define PMIC_CODEC_ANA_RW30_ADDR(base) ((base) + (0x317))
#define PMIC_CODEC_ANA_RW31_ADDR(base) ((base) + (0x318))
#define PMIC_CODEC_ANA_RW32_ADDR(base) ((base) + (0x319))
#define PMIC_CODEC_ANA_RW33_ADDR(base) ((base) + (0x31A))
#define PMIC_CODEC_ANA_RW34_ADDR(base) ((base) + (0x31B))
#define PMIC_CODEC_ANA_RW35_ADDR(base) ((base) + (0x31C))
#define PMIC_CODEC_ANA_RW36_ADDR(base) ((base) + (0x31D))
#define PMIC_CODEC_ANA_RW37_ADDR(base) ((base) + (0x31E))
#define PMIC_CODEC_ANA_RW38_ADDR(base) ((base) + (0x31F))
#define PMIC_CODEC_ANA_RW39_ADDR(base) ((base) + (0x320))
#define PMIC_CODEC_ANA_RW40_ADDR(base) ((base) + (0x321))
#define PMIC_CODEC_ANA_RW41_ADDR(base) ((base) + (0x322))
#define PMIC_CODEC_ANA_RW42_ADDR(base) ((base) + (0x323))
#define PMIC_CODEC_ANA_RW43_ADDR(base) ((base) + (0x324))
#define PMIC_CODEC_ANA_RW44_ADDR(base) ((base) + (0x325))
#define PMIC_CODEC_ANA_RW45_ADDR(base) ((base) + (0x326))
#define PMIC_CODEC_ANA_RW46_ADDR(base) ((base) + (0x327))
#define PMIC_CODEC_ANA_RW47_ADDR(base) ((base) + (0x328))
#define PMIC_CODEC_ANA_RW48_ADDR(base) ((base) + (0x329))
#define PMIC_CODEC_ANA_RW49_ADDR(base) ((base) + (0x32A))
#define PMIC_CODEC_ANA_RW50_ADDR(base) ((base) + (0x32B))
#define PMIC_CODEC_ANA_RW51_ADDR(base) ((base) + (0x32C))
#define PMIC_CODEC_ANA_RW52_ADDR(base) ((base) + (0x32D))
#define PMIC_CODEC_ANA_RO01_ADDR(base) ((base) + (0x32E))
#define PMIC_CODEC_ANA_RO02_ADDR(base) ((base) + (0x32F))
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_RTCDR0_ADDR(base) ((base) + (0x340UL))
#define PMIC_RTCDR1_ADDR(base) ((base) + (0x341UL))
#define PMIC_RTCDR2_ADDR(base) ((base) + (0x342UL))
#define PMIC_RTCDR3_ADDR(base) ((base) + (0x343UL))
#define PMIC_RTCMR0_ADDR(base) ((base) + (0x344UL))
#define PMIC_RTCMR1_ADDR(base) ((base) + (0x345UL))
#define PMIC_RTCMR2_ADDR(base) ((base) + (0x346UL))
#define PMIC_RTCMR3_ADDR(base) ((base) + (0x347UL))
#define PMIC_RTCLR0_ADDR(base) ((base) + (0x348UL))
#define PMIC_RTCLR1_ADDR(base) ((base) + (0x349UL))
#define PMIC_RTCLR2_ADDR(base) ((base) + (0x34AUL))
#define PMIC_RTCLR3_ADDR(base) ((base) + (0x34BUL))
#define PMIC_RTCCTRL_ADDR(base) ((base) + (0x34CUL))
#define PMIC_CRC_VAULE0_ADDR(base) ((base) + (0x34DUL))
#define PMIC_CRC_VAULE1_ADDR(base) ((base) + (0x34EUL))
#define PMIC_CRC_VAULE2_ADDR(base) ((base) + (0x34FUL))
#define PMIC_RTC_PWRUP_TIMER0_ADDR(base) ((base) + (0x350UL))
#define PMIC_RTC_PWRUP_TIMER1_ADDR(base) ((base) + (0x351UL))
#define PMIC_RTC_PWRUP_TIMER2_ADDR(base) ((base) + (0x352UL))
#define PMIC_RTC_PWRUP_TIMER3_ADDR(base) ((base) + (0x353UL))
#define PMIC_RTC_PWRDOWN_TIMER0_ADDR(base) ((base) + (0x354UL))
#define PMIC_RTC_PWRDOWN_TIMER1_ADDR(base) ((base) + (0x355UL))
#define PMIC_RTC_PWRDOWN_TIMER2_ADDR(base) ((base) + (0x356UL))
#define PMIC_RTC_PWRDOWN_TIMER3_ADDR(base) ((base) + (0x357UL))
#define PMIC_SER_RTCDR0_ADDR(base) ((base) + (0x360UL))
#define PMIC_SER_RTCDR1_ADDR(base) ((base) + (0x361UL))
#define PMIC_SER_RTCDR2_ADDR(base) ((base) + (0x362UL))
#define PMIC_SER_RTCDR3_ADDR(base) ((base) + (0x363UL))
#define PMIC_SER_RTCMR0_ADDR(base) ((base) + (0x364UL))
#define PMIC_SER_RTCMR1_ADDR(base) ((base) + (0x365UL))
#define PMIC_SER_RTCMR2_ADDR(base) ((base) + (0x366UL))
#define PMIC_SER_RTCMR3_ADDR(base) ((base) + (0x367UL))
#define PMIC_SER_RTCLR0_ADDR(base) ((base) + (0x368UL))
#define PMIC_SER_RTCLR1_ADDR(base) ((base) + (0x369UL))
#define PMIC_SER_RTCLR2_ADDR(base) ((base) + (0x36AUL))
#define PMIC_SER_RTCLR3_ADDR(base) ((base) + (0x36BUL))
#define PMIC_SER_RTCCTRL_ADDR(base) ((base) + (0x36CUL))
#define PMIC_SER_XO_THRESOLD0_ADDR(base) ((base) + (0x36DUL))
#define PMIC_SER_XO_THRESOLD1_ADDR(base) ((base) + (0x36EUL))
#define PMIC_SER_CRC_VAULE0_ADDR(base) ((base) + (0x36FUL))
#define PMIC_SER_CRC_VAULE1_ADDR(base) ((base) + (0x370UL))
#define PMIC_SER_CRC_VAULE2_ADDR(base) ((base) + (0x371UL))
#define PMIC_SER_RTC_PWRUP_TIMER0_ADDR(base) ((base) + (0x372UL))
#define PMIC_SER_RTC_PWRUP_TIMER1_ADDR(base) ((base) + (0x373UL))
#define PMIC_SER_RTC_PWRUP_TIMER2_ADDR(base) ((base) + (0x374UL))
#define PMIC_SER_RTC_PWRUP_TIMER3_ADDR(base) ((base) + (0x375UL))
#define PMIC_SER_RTC_PWRDOWN_TIMER0_ADDR(base) ((base) + (0x376UL))
#define PMIC_SER_RTC_PWRDOWN_TIMER1_ADDR(base) ((base) + (0x377UL))
#define PMIC_SER_RTC_PWRDOWN_TIMER2_ADDR(base) ((base) + (0x378UL))
#define PMIC_SER_RTC_PWRDOWN_TIMER3_ADDR(base) ((base) + (0x379UL))
#else
#define PMIC_RTCDR0_ADDR(base) ((base) + (0x340))
#define PMIC_RTCDR1_ADDR(base) ((base) + (0x341))
#define PMIC_RTCDR2_ADDR(base) ((base) + (0x342))
#define PMIC_RTCDR3_ADDR(base) ((base) + (0x343))
#define PMIC_RTCMR0_ADDR(base) ((base) + (0x344))
#define PMIC_RTCMR1_ADDR(base) ((base) + (0x345))
#define PMIC_RTCMR2_ADDR(base) ((base) + (0x346))
#define PMIC_RTCMR3_ADDR(base) ((base) + (0x347))
#define PMIC_RTCLR0_ADDR(base) ((base) + (0x348))
#define PMIC_RTCLR1_ADDR(base) ((base) + (0x349))
#define PMIC_RTCLR2_ADDR(base) ((base) + (0x34A))
#define PMIC_RTCLR3_ADDR(base) ((base) + (0x34B))
#define PMIC_RTCCTRL_ADDR(base) ((base) + (0x34C))
#define PMIC_CRC_VAULE0_ADDR(base) ((base) + (0x34D))
#define PMIC_CRC_VAULE1_ADDR(base) ((base) + (0x34E))
#define PMIC_CRC_VAULE2_ADDR(base) ((base) + (0x34F))
#define PMIC_RTC_PWRUP_TIMER0_ADDR(base) ((base) + (0x350))
#define PMIC_RTC_PWRUP_TIMER1_ADDR(base) ((base) + (0x351))
#define PMIC_RTC_PWRUP_TIMER2_ADDR(base) ((base) + (0x352))
#define PMIC_RTC_PWRUP_TIMER3_ADDR(base) ((base) + (0x353))
#define PMIC_RTC_PWRDOWN_TIMER0_ADDR(base) ((base) + (0x354))
#define PMIC_RTC_PWRDOWN_TIMER1_ADDR(base) ((base) + (0x355))
#define PMIC_RTC_PWRDOWN_TIMER2_ADDR(base) ((base) + (0x356))
#define PMIC_RTC_PWRDOWN_TIMER3_ADDR(base) ((base) + (0x357))
#define PMIC_SER_RTCDR0_ADDR(base) ((base) + (0x360))
#define PMIC_SER_RTCDR1_ADDR(base) ((base) + (0x361))
#define PMIC_SER_RTCDR2_ADDR(base) ((base) + (0x362))
#define PMIC_SER_RTCDR3_ADDR(base) ((base) + (0x363))
#define PMIC_SER_RTCMR0_ADDR(base) ((base) + (0x364))
#define PMIC_SER_RTCMR1_ADDR(base) ((base) + (0x365))
#define PMIC_SER_RTCMR2_ADDR(base) ((base) + (0x366))
#define PMIC_SER_RTCMR3_ADDR(base) ((base) + (0x367))
#define PMIC_SER_RTCLR0_ADDR(base) ((base) + (0x368))
#define PMIC_SER_RTCLR1_ADDR(base) ((base) + (0x369))
#define PMIC_SER_RTCLR2_ADDR(base) ((base) + (0x36A))
#define PMIC_SER_RTCLR3_ADDR(base) ((base) + (0x36B))
#define PMIC_SER_RTCCTRL_ADDR(base) ((base) + (0x36C))
#define PMIC_SER_XO_THRESOLD0_ADDR(base) ((base) + (0x36D))
#define PMIC_SER_XO_THRESOLD1_ADDR(base) ((base) + (0x36E))
#define PMIC_SER_CRC_VAULE0_ADDR(base) ((base) + (0x36F))
#define PMIC_SER_CRC_VAULE1_ADDR(base) ((base) + (0x370))
#define PMIC_SER_CRC_VAULE2_ADDR(base) ((base) + (0x371))
#define PMIC_SER_RTC_PWRUP_TIMER0_ADDR(base) ((base) + (0x372))
#define PMIC_SER_RTC_PWRUP_TIMER1_ADDR(base) ((base) + (0x373))
#define PMIC_SER_RTC_PWRUP_TIMER2_ADDR(base) ((base) + (0x374))
#define PMIC_SER_RTC_PWRUP_TIMER3_ADDR(base) ((base) + (0x375))
#define PMIC_SER_RTC_PWRDOWN_TIMER0_ADDR(base) ((base) + (0x376))
#define PMIC_SER_RTC_PWRDOWN_TIMER1_ADDR(base) ((base) + (0x377))
#define PMIC_SER_RTC_PWRDOWN_TIMER2_ADDR(base) ((base) + (0x378))
#define PMIC_SER_RTC_PWRDOWN_TIMER3_ADDR(base) ((base) + (0x379))
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_PWRUP_CALI_END_ADDR(base) ((base) + (0x03A0UL))
#define PMIC_XOADC_AUTOCALI_AVE0_ADDR(base) ((base) + (0x03A1UL))
#define PMIC_XOADC_AUTOCALI_AVE1_ADDR(base) ((base) + (0x03A2UL))
#define PMIC_XOADC_AUTOCALI_AVE2_ADDR(base) ((base) + (0x03A3UL))
#define PMIC_XOADC_AUTOCALI_AVE3_ADDR(base) ((base) + (0x03A4UL))
#define PMIC_XOADC_CTRL_ADDR(base) ((base) + (0x03A5UL))
#define PMIC_XOADC_SAMP_PHASE_ADDR(base) ((base) + (0x03A6UL))
#define PMIC_XOADC_OPT_0_ADDR(base) ((base) + (0x03A7UL))
#define PMIC_XOADC_OPT_1_ADDR(base) ((base) + (0x03A8UL))
#define PMIC_XOADC_AIN_SEL_ADDR(base) ((base) + (0x03A9UL))
#define PMIC_XOADC_WIFI_ANA_EN_ADDR(base) ((base) + (0x03AAUL))
#define PMIC_XOADC_SOC_ANA_EN_ADDR(base) ((base) + (0x03ABUL))
#define PMIC_XOADC_STATE_ADDR(base) ((base) + (0x03ACUL))
#define PMIC_XOADC_DATA0_ADDR(base) ((base) + (0x03ADUL))
#define PMIC_XOADC_DATA1_ADDR(base) ((base) + (0x03AEUL))
#define PMIC_XOADC_CALI_DATA0_ADDR(base) ((base) + (0x03AFUL))
#define PMIC_XOADC_CALI_DATA1_ADDR(base) ((base) + (0x03B0UL))
#define PMIC_XOADC_CFG_EN_ADDR(base) ((base) + (0x03B1UL))
#define PMIC_XOADC_ARB_DEBUG_ADDR(base) ((base) + (0x03B2UL))
#define PMIC_XOADC_CTRL_S_ADDR(base) ((base) + (0x03B3UL))
#define PMIC_XOADC_SAMP_PHASE_S_ADDR(base) ((base) + (0x03B4UL))
#define PMIC_XOADC_OPT_0_S_ADDR(base) ((base) + (0x03B5UL))
#define PMIC_XOADC_OPT_1_S_ADDR(base) ((base) + (0x03B6UL))
#define PMIC_XOADC_AIN_SEL_S_ADDR(base) ((base) + (0x03B7UL))
#define PMIC_XOADC_ANA_EN_S_ADDR(base) ((base) + (0x03B8UL))
#define PMIC_XOADC_SOFT_CFG0_ADDR(base) ((base) + (0x03B9UL))
#define PMIC_XOADC_SOFT_CFG1_ADDR(base) ((base) + (0x03BAUL))
#define PMIC_XOADC_SOFT_CFG2_ADDR(base) ((base) + (0x03BBUL))
#define PMIC_XOADC_SOFT_CFG3_ADDR(base) ((base) + (0x03BCUL))
#define PMIC_XOADC_SOFT_CFG4_ADDR(base) ((base) + (0x03BDUL))
#define PMIC_XOADC_SOFT_CFG5_ADDR(base) ((base) + (0x03BEUL))
#define PMIC_XOADC_SOFT_CFG6_ADDR(base) ((base) + (0x03BFUL))
#define PMIC_XOADC_SOFT_CFG7_ADDR(base) ((base) + (0x03C0UL))
#define PMIC_XOADC_RESERVE_ADDR(base) ((base) + (0x03C1UL))
#define PMIC_HI1103_RDATA_OUT0_ADDR(base) ((base) + (0x03C2UL))
#define PMIC_HI1103_RDATA_OUT1_ADDR(base) ((base) + (0x03C3UL))
#define PMIC_HI1103_RDATA_OUT2_ADDR(base) ((base) + (0x03C4UL))
#define PMIC_HI1103_RDATA_OUT3_ADDR(base) ((base) + (0x03C5UL))
#define PMIC_HI1103_RDATA_OUT4_ADDR(base) ((base) + (0x03C6UL))
#define PMIC_HI1103_RDATA_OUT5_ADDR(base) ((base) + (0x03C7UL))
#define PMIC_HI1103_RDATA_OUT6_ADDR(base) ((base) + (0x03C8UL))
#define PMIC_HI1103_RDATA_OUT7_ADDR(base) ((base) + (0x03C9UL))
#define PMIC_RTC_LOAD_FLAG_ADDR(base) ((base) + (0x03CAUL))
#define PMIC_HI1103_REFRESH_LOCK_ADDR(base) ((base) + (0x03CBUL))
#define PMIC_SPMI_DEBUG0_ADDR(base) ((base) + (0x03CEUL))
#define PMIC_SPMI_DEBUG1_ADDR(base) ((base) + (0x03CFUL))
#define PMIC_SPMI_DEBUG2_ADDR(base) ((base) + (0x03D0UL))
#define PMIC_SPMI_DEBUG3_ADDR(base) ((base) + (0x03D1UL))
#define PMIC_SPMI_DEBUG4_ADDR(base) ((base) + (0x03D2UL))
#define PMIC_SPMI_DEBUG5_ADDR(base) ((base) + (0x03D3UL))
#define PMIC_SPMI_DEBUG6_ADDR(base) ((base) + (0x03D4UL))
#define PMIC_SPMI_DEBUG7_ADDR(base) ((base) + (0x03D5UL))
#define PMIC_DIG_IO_DS_SEL_ADDR(base) ((base) + (0x03D9UL))
#define PMIC_DIG_IO_DS_CFG0_ADDR(base) ((base) + (0x03DAUL))
#define PMIC_DIG_IO_DS_CFG1_ADDR(base) ((base) + (0x03DBUL))
#define PMIC_SOC_RDATA_OUT0_ADDR(base) ((base) + (0x03DCUL))
#define PMIC_SOC_RDATA_OUT1_ADDR(base) ((base) + (0x03DDUL))
#define PMIC_SOC_RDATA_OUT2_ADDR(base) ((base) + (0x03DEUL))
#define PMIC_SOC_RDATA_OUT3_ADDR(base) ((base) + (0x03DFUL))
#define PMIC_SOC_RDATA_OUT4_ADDR(base) ((base) + (0x03E0UL))
#define PMIC_SOC_RDATA_OUT5_ADDR(base) ((base) + (0x03E1UL))
#define PMIC_SOC_RDATA_OUT6_ADDR(base) ((base) + (0x03E2UL))
#define PMIC_SOC_RDATA_OUT7_ADDR(base) ((base) + (0x03E3UL))
#define PMIC_SOC_REFRESH_LOCK_ADDR(base) ((base) + (0x03E4UL))
#else
#define PMIC_PWRUP_CALI_END_ADDR(base) ((base) + (0x03A0))
#define PMIC_XOADC_AUTOCALI_AVE0_ADDR(base) ((base) + (0x03A1))
#define PMIC_XOADC_AUTOCALI_AVE1_ADDR(base) ((base) + (0x03A2))
#define PMIC_XOADC_AUTOCALI_AVE2_ADDR(base) ((base) + (0x03A3))
#define PMIC_XOADC_AUTOCALI_AVE3_ADDR(base) ((base) + (0x03A4))
#define PMIC_XOADC_CTRL_ADDR(base) ((base) + (0x03A5))
#define PMIC_XOADC_SAMP_PHASE_ADDR(base) ((base) + (0x03A6))
#define PMIC_XOADC_OPT_0_ADDR(base) ((base) + (0x03A7))
#define PMIC_XOADC_OPT_1_ADDR(base) ((base) + (0x03A8))
#define PMIC_XOADC_AIN_SEL_ADDR(base) ((base) + (0x03A9))
#define PMIC_XOADC_WIFI_ANA_EN_ADDR(base) ((base) + (0x03AA))
#define PMIC_XOADC_SOC_ANA_EN_ADDR(base) ((base) + (0x03AB))
#define PMIC_XOADC_STATE_ADDR(base) ((base) + (0x03AC))
#define PMIC_XOADC_DATA0_ADDR(base) ((base) + (0x03AD))
#define PMIC_XOADC_DATA1_ADDR(base) ((base) + (0x03AE))
#define PMIC_XOADC_CALI_DATA0_ADDR(base) ((base) + (0x03AF))
#define PMIC_XOADC_CALI_DATA1_ADDR(base) ((base) + (0x03B0))
#define PMIC_XOADC_CFG_EN_ADDR(base) ((base) + (0x03B1))
#define PMIC_XOADC_ARB_DEBUG_ADDR(base) ((base) + (0x03B2))
#define PMIC_XOADC_CTRL_S_ADDR(base) ((base) + (0x03B3))
#define PMIC_XOADC_SAMP_PHASE_S_ADDR(base) ((base) + (0x03B4))
#define PMIC_XOADC_OPT_0_S_ADDR(base) ((base) + (0x03B5))
#define PMIC_XOADC_OPT_1_S_ADDR(base) ((base) + (0x03B6))
#define PMIC_XOADC_AIN_SEL_S_ADDR(base) ((base) + (0x03B7))
#define PMIC_XOADC_ANA_EN_S_ADDR(base) ((base) + (0x03B8))
#define PMIC_XOADC_SOFT_CFG0_ADDR(base) ((base) + (0x03B9))
#define PMIC_XOADC_SOFT_CFG1_ADDR(base) ((base) + (0x03BA))
#define PMIC_XOADC_SOFT_CFG2_ADDR(base) ((base) + (0x03BB))
#define PMIC_XOADC_SOFT_CFG3_ADDR(base) ((base) + (0x03BC))
#define PMIC_XOADC_SOFT_CFG4_ADDR(base) ((base) + (0x03BD))
#define PMIC_XOADC_SOFT_CFG5_ADDR(base) ((base) + (0x03BE))
#define PMIC_XOADC_SOFT_CFG6_ADDR(base) ((base) + (0x03BF))
#define PMIC_XOADC_SOFT_CFG7_ADDR(base) ((base) + (0x03C0))
#define PMIC_XOADC_RESERVE_ADDR(base) ((base) + (0x03C1))
#define PMIC_HI1103_RDATA_OUT0_ADDR(base) ((base) + (0x03C2))
#define PMIC_HI1103_RDATA_OUT1_ADDR(base) ((base) + (0x03C3))
#define PMIC_HI1103_RDATA_OUT2_ADDR(base) ((base) + (0x03C4))
#define PMIC_HI1103_RDATA_OUT3_ADDR(base) ((base) + (0x03C5))
#define PMIC_HI1103_RDATA_OUT4_ADDR(base) ((base) + (0x03C6))
#define PMIC_HI1103_RDATA_OUT5_ADDR(base) ((base) + (0x03C7))
#define PMIC_HI1103_RDATA_OUT6_ADDR(base) ((base) + (0x03C8))
#define PMIC_HI1103_RDATA_OUT7_ADDR(base) ((base) + (0x03C9))
#define PMIC_RTC_LOAD_FLAG_ADDR(base) ((base) + (0x03CA))
#define PMIC_HI1103_REFRESH_LOCK_ADDR(base) ((base) + (0x03CB))
#define PMIC_SPMI_DEBUG0_ADDR(base) ((base) + (0x03CE))
#define PMIC_SPMI_DEBUG1_ADDR(base) ((base) + (0x03CF))
#define PMIC_SPMI_DEBUG2_ADDR(base) ((base) + (0x03D0))
#define PMIC_SPMI_DEBUG3_ADDR(base) ((base) + (0x03D1))
#define PMIC_SPMI_DEBUG4_ADDR(base) ((base) + (0x03D2))
#define PMIC_SPMI_DEBUG5_ADDR(base) ((base) + (0x03D3))
#define PMIC_SPMI_DEBUG6_ADDR(base) ((base) + (0x03D4))
#define PMIC_SPMI_DEBUG7_ADDR(base) ((base) + (0x03D5))
#define PMIC_DIG_IO_DS_SEL_ADDR(base) ((base) + (0x03D9))
#define PMIC_DIG_IO_DS_CFG0_ADDR(base) ((base) + (0x03DA))
#define PMIC_DIG_IO_DS_CFG1_ADDR(base) ((base) + (0x03DB))
#define PMIC_SOC_RDATA_OUT0_ADDR(base) ((base) + (0x03DC))
#define PMIC_SOC_RDATA_OUT1_ADDR(base) ((base) + (0x03DD))
#define PMIC_SOC_RDATA_OUT2_ADDR(base) ((base) + (0x03DE))
#define PMIC_SOC_RDATA_OUT3_ADDR(base) ((base) + (0x03DF))
#define PMIC_SOC_RDATA_OUT4_ADDR(base) ((base) + (0x03E0))
#define PMIC_SOC_RDATA_OUT5_ADDR(base) ((base) + (0x03E1))
#define PMIC_SOC_RDATA_OUT6_ADDR(base) ((base) + (0x03E2))
#define PMIC_SOC_RDATA_OUT7_ADDR(base) ((base) + (0x03E3))
#define PMIC_SOC_REFRESH_LOCK_ADDR(base) ((base) + (0x03E4))
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_ADC_CTRL_ADDR(base) ((base) + (0x2000UL))
#define PMIC_ADC_START_ADDR(base) ((base) + (0x2001UL))
#define PMIC_CONV_STATUS_ADDR(base) ((base) + (0x2002UL))
#define PMIC_ADC_DATA1_ADDR(base) ((base) + (0x2003UL))
#define PMIC_ADC_DATA0_ADDR(base) ((base) + (0x2004UL))
#define PMIC_ADC_CONV_ADDR(base) ((base) + (0x2005UL))
#define PMIC_ADC_CURRENT_ADDR(base) ((base) + (0x2006UL))
#define PMIC_ADC_CALI_CTRL_ADDR(base) ((base) + (0x2007UL))
#define PMIC_ADC_CALI_VALUE_ADDR(base) ((base) + (0x2008UL))
#define PMIC_ADC_CALI_CFG_ADDR(base) ((base) + (0x2009UL))
#define PMIC_ADC_MODE_CFG_ADDR(base) ((base) + (0x200AUL))
#define PMIC_ADC_CHOPPER_1ST_DATA1_ADDR(base) ((base) + (0x200BUL))
#define PMIC_ADC_CHOPPER_1ST_DATA2_ADDR(base) ((base) + (0x200CUL))
#define PMIC_ADC_CHOPPER_2ND_DATA1_ADDR(base) ((base) + (0x200DUL))
#define PMIC_ADC_CHOPPER_2ND_DATA2_ADDR(base) ((base) + (0x200EUL))
#define PMIC_ADC_CALIVALUE_CFG1_ADDR(base) ((base) + (0x200FUL))
#define PMIC_ADC_CALIVALUE_CFG2_ADDR(base) ((base) + (0x2010UL))
#else
#define PMIC_ADC_CTRL_ADDR(base) ((base) + (0x2000))
#define PMIC_ADC_START_ADDR(base) ((base) + (0x2001))
#define PMIC_CONV_STATUS_ADDR(base) ((base) + (0x2002))
#define PMIC_ADC_DATA1_ADDR(base) ((base) + (0x2003))
#define PMIC_ADC_DATA0_ADDR(base) ((base) + (0x2004))
#define PMIC_ADC_CONV_ADDR(base) ((base) + (0x2005))
#define PMIC_ADC_CURRENT_ADDR(base) ((base) + (0x2006))
#define PMIC_ADC_CALI_CTRL_ADDR(base) ((base) + (0x2007))
#define PMIC_ADC_CALI_VALUE_ADDR(base) ((base) + (0x2008))
#define PMIC_ADC_CALI_CFG_ADDR(base) ((base) + (0x2009))
#define PMIC_ADC_MODE_CFG_ADDR(base) ((base) + (0x200A))
#define PMIC_ADC_CHOPPER_1ST_DATA1_ADDR(base) ((base) + (0x200B))
#define PMIC_ADC_CHOPPER_1ST_DATA2_ADDR(base) ((base) + (0x200C))
#define PMIC_ADC_CHOPPER_2ND_DATA1_ADDR(base) ((base) + (0x200D))
#define PMIC_ADC_CHOPPER_2ND_DATA2_ADDR(base) ((base) + (0x200E))
#define PMIC_ADC_CALIVALUE_CFG1_ADDR(base) ((base) + (0x200F))
#define PMIC_ADC_CALIVALUE_CFG2_ADDR(base) ((base) + (0x2010))
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_COUL_IRQ_ADDR(base) ((base) + (0x4000UL))
#define PMIC_COUL_IRQ_MASK_ADDR(base) ((base) + (0x4001UL))
#define PMIC_COUL_IRQ_NP_ADDR(base) ((base) + (0x4002UL))
#define PMIC_CLJ_CTRL_REG_ADDR(base) ((base) + (0x4003UL))
#define PMIC_CLJ_CTRL_REGS2_ADDR(base) ((base) + (0x4004UL))
#define PMIC_CLJ_CTRL_REGS3_ADDR(base) ((base) + (0x4005UL))
#define PMIC_CLJ_CTRL_REGS4_ADDR(base) ((base) + (0x4006UL))
#define PMIC_CLJ_CTRL_REGS5_ADDR(base) ((base) + (0x4007UL))
#define PMIC_CIC_CTRL_ADDR(base) ((base) + (0x4008UL))
#define PMIC_COUL_TEMP_CTRL_ADDR(base) ((base) + (0x4009UL))
#define PMIC_COUL_VI_CTRL0_ADDR(base) ((base) + (0x400AUL))
#define PMIC_COUL_VI_CTRL1_ADDR(base) ((base) + (0x400BUL))
#define PMIC_COUL_VI_CTRL2_ADDR(base) ((base) + (0x400CUL))
#define PMIC_COUL_VI_CTRL3_ADDR(base) ((base) + (0x400DUL))
#define PMIC_COUL_VI_CTRL4_ADDR(base) ((base) + (0x400EUL))
#define PMIC_COUL_VI_CTRL5_ADDR(base) ((base) + (0x400FUL))
#define PMIC_CL_OUT0_ADDR(base) ((base) + (0x4010UL))
#define PMIC_CL_OUT1_ADDR(base) ((base) + (0x4011UL))
#define PMIC_CL_OUT2_ADDR(base) ((base) + (0x4012UL))
#define PMIC_CL_OUT3_ADDR(base) ((base) + (0x4013UL))
#define PMIC_CL_OUT4_ADDR(base) ((base) + (0x4014UL))
#define PMIC_CL_IN0_ADDR(base) ((base) + (0x4015UL))
#define PMIC_CL_IN1_ADDR(base) ((base) + (0x4016UL))
#define PMIC_CL_IN2_ADDR(base) ((base) + (0x4017UL))
#define PMIC_CL_IN3_ADDR(base) ((base) + (0x4018UL))
#define PMIC_CL_IN4_ADDR(base) ((base) + (0x4019UL))
#define PMIC_CHG_TIMER0_ADDR(base) ((base) + (0x401AUL))
#define PMIC_CHG_TIMER1_ADDR(base) ((base) + (0x401BUL))
#define PMIC_CHG_TIMER2_ADDR(base) ((base) + (0x401CUL))
#define PMIC_CHG_TIMER3_ADDR(base) ((base) + (0x401DUL))
#define PMIC_LOAD_TIMER0_ADDR(base) ((base) + (0x401EUL))
#define PMIC_LOAD_TIMER1_ADDR(base) ((base) + (0x401FUL))
#define PMIC_LOAD_TIMER2_ADDR(base) ((base) + (0x4020UL))
#define PMIC_LOAD_TIMER3_ADDR(base) ((base) + (0x4021UL))
#define PMIC_CL_INT0_ADDR(base) ((base) + (0x4022UL))
#define PMIC_CL_INT1_ADDR(base) ((base) + (0x4023UL))
#define PMIC_CL_INT2_ADDR(base) ((base) + (0x4024UL))
#define PMIC_CL_INT3_ADDR(base) ((base) + (0x4025UL))
#define PMIC_CL_INT4_ADDR(base) ((base) + (0x4026UL))
#define PMIC_V_INT0_ADDR(base) ((base) + (0x4027UL))
#define PMIC_V_INT1_ADDR(base) ((base) + (0x4028UL))
#define PMIC_V_INT2_ADDR(base) ((base) + (0x4029UL))
#define PMIC_I_OUT_GATE0_ADDR(base) ((base) + (0x402AUL))
#define PMIC_I_OUT_GATE1_ADDR(base) ((base) + (0x402BUL))
#define PMIC_I_OUT_GATE2_ADDR(base) ((base) + (0x402CUL))
#define PMIC_I_IN_GATE0_ADDR(base) ((base) + (0x402DUL))
#define PMIC_I_IN_GATE1_ADDR(base) ((base) + (0x402EUL))
#define PMIC_I_IN_GATE2_ADDR(base) ((base) + (0x402FUL))
#define PMIC_OFFSET_CURRENT_MOD_0_ADDR(base) ((base) + (0x4030UL))
#define PMIC_OFFSET_CURRENT_MOD_1_ADDR(base) ((base) + (0x4031UL))
#define PMIC_OFFSET_CURRENT_MOD_2_ADDR(base) ((base) + (0x4032UL))
#define PMIC_OFFSET_VOLTAGE_MOD_0_ADDR(base) ((base) + (0x4033UL))
#define PMIC_OFFSET_VOLTAGE_MOD_1_ADDR(base) ((base) + (0x4034UL))
#define PMIC_OFFSET_VOLTAGE_MOD_2_ADDR(base) ((base) + (0x4035UL))
#define PMIC_STATE_TEST_ADDR(base) ((base) + (0x4036UL))
#define PMIC_CURRENT_0_ADDR(base) ((base) + (0x4037UL))
#define PMIC_CURRENT_1_ADDR(base) ((base) + (0x4038UL))
#define PMIC_CURRENT_2_ADDR(base) ((base) + (0x4039UL))
#define PMIC_V_OUT_0_ADDR(base) ((base) + (0x403AUL))
#define PMIC_V_OUT_1_ADDR(base) ((base) + (0x403BUL))
#define PMIC_V_OUT_2_ADDR(base) ((base) + (0x403CUL))
#define PMIC_OFFSET_CURRENT0_ADDR(base) ((base) + (0x403DUL))
#define PMIC_OFFSET_CURRENT1_ADDR(base) ((base) + (0x403EUL))
#define PMIC_OFFSET_CURRENT2_ADDR(base) ((base) + (0x403FUL))
#define PMIC_OFFSET_VOLTAGE0_ADDR(base) ((base) + (0x4040UL))
#define PMIC_OFFSET_VOLTAGE1_ADDR(base) ((base) + (0x4041UL))
#define PMIC_OFFSET_VOLTAGE2_ADDR(base) ((base) + (0x4042UL))
#define PMIC_OCV_VOLTAGE0_ADDR(base) ((base) + (0x4043UL))
#define PMIC_OCV_VOLTAGE1_ADDR(base) ((base) + (0x4044UL))
#define PMIC_OCV_VOLTAGE2_ADDR(base) ((base) + (0x4045UL))
#define PMIC_OCV_CURRENT0_ADDR(base) ((base) + (0x4046UL))
#define PMIC_OCV_CURRENT1_ADDR(base) ((base) + (0x4047UL))
#define PMIC_OCV_CURRENT2_ADDR(base) ((base) + (0x4048UL))
#define PMIC_OCV_TEMP0_ADDR(base) ((base) + (0x4049UL))
#define PMIC_OCV_TEMP1_ADDR(base) ((base) + (0x404AUL))
#define PMIC_OCV_TEMP2_ADDR(base) ((base) + (0x404BUL))
#define PMIC_ECO_OUT_CLIN_0_ADDR(base) ((base) + (0x404CUL))
#define PMIC_ECO_OUT_CLIN_1_ADDR(base) ((base) + (0x404DUL))
#define PMIC_ECO_OUT_CLIN_2_ADDR(base) ((base) + (0x404EUL))
#define PMIC_ECO_OUT_CLIN_3_ADDR(base) ((base) + (0x404FUL))
#define PMIC_ECO_OUT_CLIN_4_ADDR(base) ((base) + (0x4050UL))
#define PMIC_ECO_OUT_CLOUT_0_ADDR(base) ((base) + (0x4051UL))
#define PMIC_ECO_OUT_CLOUT_1_ADDR(base) ((base) + (0x4052UL))
#define PMIC_ECO_OUT_CLOUT_2_ADDR(base) ((base) + (0x4053UL))
#define PMIC_ECO_OUT_CLOUT_3_ADDR(base) ((base) + (0x4054UL))
#define PMIC_ECO_OUT_CLOUT_4_ADDR(base) ((base) + (0x4055UL))
#define PMIC_ECO_OUT_TEMP_0_ADDR(base) ((base) + (0x4056UL))
#define PMIC_ECO_OUT_TEMP_1_ADDR(base) ((base) + (0x4057UL))
#define PMIC_ECO_OUT_TEMP_2_ADDR(base) ((base) + (0x4058UL))
#define PMIC_TEMP0_RDATA_ADDR(base) ((base) + (0x4059UL))
#define PMIC_TEMP1_RDATA_ADDR(base) ((base) + (0x405AUL))
#define PMIC_TEMP2_RDATA_ADDR(base) ((base) + (0x405BUL))
#define PMIC_V_PRE0_OUT0_ADDR(base) ((base) + (0x405CUL))
#define PMIC_V_PRE0_OUT1_ADDR(base) ((base) + (0x405DUL))
#define PMIC_V_PRE0_OUT2_ADDR(base) ((base) + (0x405EUL))
#define PMIC_V_PRE1_OUT0_ADDR(base) ((base) + (0x405FUL))
#define PMIC_V_PRE1_OUT1_ADDR(base) ((base) + (0x4060UL))
#define PMIC_V_PRE1_OUT2_ADDR(base) ((base) + (0x4061UL))
#define PMIC_V_PRE2_OUT0_ADDR(base) ((base) + (0x4062UL))
#define PMIC_V_PRE2_OUT1_ADDR(base) ((base) + (0x4063UL))
#define PMIC_V_PRE2_OUT2_ADDR(base) ((base) + (0x4064UL))
#define PMIC_V_PRE3_OUT0_ADDR(base) ((base) + (0x4065UL))
#define PMIC_V_PRE3_OUT1_ADDR(base) ((base) + (0x4066UL))
#define PMIC_V_PRE3_OUT2_ADDR(base) ((base) + (0x4067UL))
#define PMIC_V_PRE4_OUT0_ADDR(base) ((base) + (0x4068UL))
#define PMIC_V_PRE4_OUT1_ADDR(base) ((base) + (0x4069UL))
#define PMIC_V_PRE4_OUT2_ADDR(base) ((base) + (0x406AUL))
#define PMIC_V_PRE5_OUT0_ADDR(base) ((base) + (0x406BUL))
#define PMIC_V_PRE5_OUT1_ADDR(base) ((base) + (0x406CUL))
#define PMIC_V_PRE5_OUT2_ADDR(base) ((base) + (0x406DUL))
#define PMIC_V_PRE6_OUT0_ADDR(base) ((base) + (0x406EUL))
#define PMIC_V_PRE6_OUT1_ADDR(base) ((base) + (0x406FUL))
#define PMIC_V_PRE6_OUT2_ADDR(base) ((base) + (0x4070UL))
#define PMIC_V_PRE7_OUT0_ADDR(base) ((base) + (0x4071UL))
#define PMIC_V_PRE7_OUT1_ADDR(base) ((base) + (0x4072UL))
#define PMIC_V_PRE7_OUT2_ADDR(base) ((base) + (0x4073UL))
#define PMIC_CURRENT_PRE0_OUT0_ADDR(base) ((base) + (0x4074UL))
#define PMIC_CURRENT_PRE0_OUT1_ADDR(base) ((base) + (0x4075UL))
#define PMIC_CURRENT_PRE0_OUT2_ADDR(base) ((base) + (0x4076UL))
#define PMIC_CURRENT_PRE1_OUT0_ADDR(base) ((base) + (0x4077UL))
#define PMIC_CURRENT_PRE1_OUT1_ADDR(base) ((base) + (0x4078UL))
#define PMIC_CURRENT_PRE1_OUT2_ADDR(base) ((base) + (0x4079UL))
#define PMIC_CURRENT_PRE2_OUT0_ADDR(base) ((base) + (0x407AUL))
#define PMIC_CURRENT_PRE2_OUT1_ADDR(base) ((base) + (0x407BUL))
#define PMIC_CURRENT_PRE2_OUT2_ADDR(base) ((base) + (0x407CUL))
#define PMIC_CURRENT_PRE3_OUT0_ADDR(base) ((base) + (0x407DUL))
#define PMIC_CURRENT_PRE3_OUT1_ADDR(base) ((base) + (0x407EUL))
#define PMIC_CURRENT_PRE3_OUT2_ADDR(base) ((base) + (0x407FUL))
#define PMIC_CURRENT_PRE4_OUT0_ADDR(base) ((base) + (0x4080UL))
#define PMIC_CURRENT_PRE4_OUT1_ADDR(base) ((base) + (0x4081UL))
#define PMIC_CURRENT_PRE4_OUT2_ADDR(base) ((base) + (0x4082UL))
#define PMIC_CURRENT_PRE5_OUT0_ADDR(base) ((base) + (0x4083UL))
#define PMIC_CURRENT_PRE5_OUT1_ADDR(base) ((base) + (0x4084UL))
#define PMIC_CURRENT_PRE5_OUT2_ADDR(base) ((base) + (0x4085UL))
#define PMIC_CURRENT_PRE6_OUT0_ADDR(base) ((base) + (0x4086UL))
#define PMIC_CURRENT_PRE6_OUT1_ADDR(base) ((base) + (0x4087UL))
#define PMIC_CURRENT_PRE6_OUT2_ADDR(base) ((base) + (0x4088UL))
#define PMIC_CURRENT_PRE7_OUT0_ADDR(base) ((base) + (0x4089UL))
#define PMIC_CURRENT_PRE7_OUT1_ADDR(base) ((base) + (0x408AUL))
#define PMIC_CURRENT_PRE7_OUT2_ADDR(base) ((base) + (0x408BUL))
#define PMIC_V_OCV_PRE1_OUT0_ADDR(base) ((base) + (0x408CUL))
#define PMIC_V_OCV_PRE1_OUT1_ADDR(base) ((base) + (0x408DUL))
#define PMIC_V_OCV_PRE1_OUT2_ADDR(base) ((base) + (0x408EUL))
#define PMIC_V_OCV_PRE2_OUT0_ADDR(base) ((base) + (0x408FUL))
#define PMIC_V_OCV_PRE2_OUT1_ADDR(base) ((base) + (0x4090UL))
#define PMIC_V_OCV_PRE2_OUT2_ADDR(base) ((base) + (0x4091UL))
#define PMIC_V_OCV_PRE3_OUT0_ADDR(base) ((base) + (0x4092UL))
#define PMIC_V_OCV_PRE3_OUT1_ADDR(base) ((base) + (0x4093UL))
#define PMIC_V_OCV_PRE3_OUT2_ADDR(base) ((base) + (0x4094UL))
#define PMIC_V_OCV_PRE4_OUT0_ADDR(base) ((base) + (0x4095UL))
#define PMIC_V_OCV_PRE4_OUT1_ADDR(base) ((base) + (0x4096UL))
#define PMIC_V_OCV_PRE4_OUT2_ADDR(base) ((base) + (0x4097UL))
#define PMIC_I_OCV_PRE1_OUT0_ADDR(base) ((base) + (0x4098UL))
#define PMIC_I_OCV_PRE1_OUT1_ADDR(base) ((base) + (0x4099UL))
#define PMIC_I_OCV_PRE1_OUT2_ADDR(base) ((base) + (0x409AUL))
#define PMIC_I_OCV_PRE2_OUT0_ADDR(base) ((base) + (0x409BUL))
#define PMIC_I_OCV_PRE2_OUT1_ADDR(base) ((base) + (0x409CUL))
#define PMIC_I_OCV_PRE2_OUT2_ADDR(base) ((base) + (0x409DUL))
#define PMIC_I_OCV_PRE3_OUT0_ADDR(base) ((base) + (0x409EUL))
#define PMIC_I_OCV_PRE3_OUT1_ADDR(base) ((base) + (0x409FUL))
#define PMIC_I_OCV_PRE3_OUT2_ADDR(base) ((base) + (0x40A0UL))
#define PMIC_I_OCV_PRE4_OUT0_ADDR(base) ((base) + (0x40A1UL))
#define PMIC_I_OCV_PRE4_OUT1_ADDR(base) ((base) + (0x40A2UL))
#define PMIC_I_OCV_PRE4_OUT2_ADDR(base) ((base) + (0x40A3UL))
#define PMIC_T_OCV_PRE1_OUT0_ADDR(base) ((base) + (0x40A4UL))
#define PMIC_T_OCV_PRE1_OUT1_ADDR(base) ((base) + (0x40A5UL))
#define PMIC_T_OCV_PRE2_OUT0_ADDR(base) ((base) + (0x40A6UL))
#define PMIC_T_OCV_PRE2_OUT1_ADDR(base) ((base) + (0x40A7UL))
#define PMIC_T_OCV_PRE3_OUT0_ADDR(base) ((base) + (0x40A8UL))
#define PMIC_T_OCV_PRE3_OUT1_ADDR(base) ((base) + (0x40A9UL))
#define PMIC_T_OCV_PRE4_OUT0_ADDR(base) ((base) + (0x40AAUL))
#define PMIC_T_OCV_PRE4_OUT1_ADDR(base) ((base) + (0x40ABUL))
#define PMIC_RTC_OCV_OUT_ADDR(base) ((base) + (0x40ACUL))
#define PMIC_SLIDE_I_DATA0_OUT0_ADDR(base) ((base) + (0x40ADUL))
#define PMIC_SLIDE_I_DATA0_OUT1_ADDR(base) ((base) + (0x40AEUL))
#define PMIC_SLIDE_I_DATA0_OUT2_ADDR(base) ((base) + (0x40AFUL))
#define PMIC_SLIDE_I_DATA1_OUT0_ADDR(base) ((base) + (0x40B0UL))
#define PMIC_SLIDE_I_DATA1_OUT1_ADDR(base) ((base) + (0x40B1UL))
#define PMIC_SLIDE_I_DATA1_OUT2_ADDR(base) ((base) + (0x40B2UL))
#define PMIC_SLIDE_I_DATA2_OUT0_ADDR(base) ((base) + (0x40B3UL))
#define PMIC_SLIDE_I_DATA2_OUT1_ADDR(base) ((base) + (0x40B4UL))
#define PMIC_SLIDE_I_DATA2_OUT2_ADDR(base) ((base) + (0x40B5UL))
#define PMIC_SLIDE_I_DATA3_OUT0_ADDR(base) ((base) + (0x40B6UL))
#define PMIC_SLIDE_I_DATA3_OUT1_ADDR(base) ((base) + (0x40B7UL))
#define PMIC_SLIDE_I_DATA3_OUT2_ADDR(base) ((base) + (0x40B8UL))
#define PMIC_SLIDE_V_DATA0_OUT0_ADDR(base) ((base) + (0x40B9UL))
#define PMIC_SLIDE_V_DATA0_OUT1_ADDR(base) ((base) + (0x40BAUL))
#define PMIC_SLIDE_V_DATA0_OUT2_ADDR(base) ((base) + (0x40BBUL))
#define PMIC_SLIDE_V_DATA1_OUT0_ADDR(base) ((base) + (0x40BCUL))
#define PMIC_SLIDE_V_DATA1_OUT1_ADDR(base) ((base) + (0x40BDUL))
#define PMIC_SLIDE_V_DATA1_OUT2_ADDR(base) ((base) + (0x40BEUL))
#define PMIC_SLIDE_V_DATA2_OUT0_ADDR(base) ((base) + (0x40BFUL))
#define PMIC_SLIDE_V_DATA2_OUT1_ADDR(base) ((base) + (0x40C0UL))
#define PMIC_SLIDE_V_DATA2_OUT2_ADDR(base) ((base) + (0x40C1UL))
#define PMIC_SLIDE_V_DATA3_OUT0_ADDR(base) ((base) + (0x40C2UL))
#define PMIC_SLIDE_V_DATA3_OUT1_ADDR(base) ((base) + (0x40C3UL))
#define PMIC_SLIDE_V_DATA3_OUT2_ADDR(base) ((base) + (0x40C4UL))
#define PMIC_SLIDE_CNT_DATA_OUT0_ADDR(base) ((base) + (0x40C5UL))
#define PMIC_SLIDE_CNT_DATA_OUT1_ADDR(base) ((base) + (0x40C6UL))
#define PMIC_SLIDE_CNT_DATA_OUT2_ADDR(base) ((base) + (0x40C7UL))
#define PMIC_COUL_RESERVE0_ADDR(base) ((base) + (0x40C8UL))
#define PMIC_COUL_RESERVE1_ADDR(base) ((base) + (0x40C9UL))
#define PMIC_CLJ_DEBUG0_ADDR(base) ((base) + (0x40CAUL))
#define PMIC_CLJ_DEBUG1_ADDR(base) ((base) + (0x40CBUL))
#define PMIC_DEBUG_CIC_I0_ADDR(base) ((base) + (0x40CCUL))
#define PMIC_DEBUG_CIC_I1_ADDR(base) ((base) + (0x40CDUL))
#define PMIC_DEBUG_CIC_I2_ADDR(base) ((base) + (0x40CEUL))
#define PMIC_DEBUG_CIC_V0_ADDR(base) ((base) + (0x40CFUL))
#define PMIC_DEBUG_CIC_V1_ADDR(base) ((base) + (0x40D0UL))
#define PMIC_DEBUG_CIC_V2_ADDR(base) ((base) + (0x40D1UL))
#define PMIC_DEBUG_WRITE_PRO_ADDR(base) ((base) + (0x40D2UL))
#else
#define PMIC_COUL_IRQ_ADDR(base) ((base) + (0x4000))
#define PMIC_COUL_IRQ_MASK_ADDR(base) ((base) + (0x4001))
#define PMIC_COUL_IRQ_NP_ADDR(base) ((base) + (0x4002))
#define PMIC_CLJ_CTRL_REG_ADDR(base) ((base) + (0x4003))
#define PMIC_CLJ_CTRL_REGS2_ADDR(base) ((base) + (0x4004))
#define PMIC_CLJ_CTRL_REGS3_ADDR(base) ((base) + (0x4005))
#define PMIC_CLJ_CTRL_REGS4_ADDR(base) ((base) + (0x4006))
#define PMIC_CLJ_CTRL_REGS5_ADDR(base) ((base) + (0x4007))
#define PMIC_CIC_CTRL_ADDR(base) ((base) + (0x4008))
#define PMIC_COUL_TEMP_CTRL_ADDR(base) ((base) + (0x4009))
#define PMIC_COUL_VI_CTRL0_ADDR(base) ((base) + (0x400A))
#define PMIC_COUL_VI_CTRL1_ADDR(base) ((base) + (0x400B))
#define PMIC_COUL_VI_CTRL2_ADDR(base) ((base) + (0x400C))
#define PMIC_COUL_VI_CTRL3_ADDR(base) ((base) + (0x400D))
#define PMIC_COUL_VI_CTRL4_ADDR(base) ((base) + (0x400E))
#define PMIC_COUL_VI_CTRL5_ADDR(base) ((base) + (0x400F))
#define PMIC_CL_OUT0_ADDR(base) ((base) + (0x4010))
#define PMIC_CL_OUT1_ADDR(base) ((base) + (0x4011))
#define PMIC_CL_OUT2_ADDR(base) ((base) + (0x4012))
#define PMIC_CL_OUT3_ADDR(base) ((base) + (0x4013))
#define PMIC_CL_OUT4_ADDR(base) ((base) + (0x4014))
#define PMIC_CL_IN0_ADDR(base) ((base) + (0x4015))
#define PMIC_CL_IN1_ADDR(base) ((base) + (0x4016))
#define PMIC_CL_IN2_ADDR(base) ((base) + (0x4017))
#define PMIC_CL_IN3_ADDR(base) ((base) + (0x4018))
#define PMIC_CL_IN4_ADDR(base) ((base) + (0x4019))
#define PMIC_CHG_TIMER0_ADDR(base) ((base) + (0x401A))
#define PMIC_CHG_TIMER1_ADDR(base) ((base) + (0x401B))
#define PMIC_CHG_TIMER2_ADDR(base) ((base) + (0x401C))
#define PMIC_CHG_TIMER3_ADDR(base) ((base) + (0x401D))
#define PMIC_LOAD_TIMER0_ADDR(base) ((base) + (0x401E))
#define PMIC_LOAD_TIMER1_ADDR(base) ((base) + (0x401F))
#define PMIC_LOAD_TIMER2_ADDR(base) ((base) + (0x4020))
#define PMIC_LOAD_TIMER3_ADDR(base) ((base) + (0x4021))
#define PMIC_CL_INT0_ADDR(base) ((base) + (0x4022))
#define PMIC_CL_INT1_ADDR(base) ((base) + (0x4023))
#define PMIC_CL_INT2_ADDR(base) ((base) + (0x4024))
#define PMIC_CL_INT3_ADDR(base) ((base) + (0x4025))
#define PMIC_CL_INT4_ADDR(base) ((base) + (0x4026))
#define PMIC_V_INT0_ADDR(base) ((base) + (0x4027))
#define PMIC_V_INT1_ADDR(base) ((base) + (0x4028))
#define PMIC_V_INT2_ADDR(base) ((base) + (0x4029))
#define PMIC_I_OUT_GATE0_ADDR(base) ((base) + (0x402A))
#define PMIC_I_OUT_GATE1_ADDR(base) ((base) + (0x402B))
#define PMIC_I_OUT_GATE2_ADDR(base) ((base) + (0x402C))
#define PMIC_I_IN_GATE0_ADDR(base) ((base) + (0x402D))
#define PMIC_I_IN_GATE1_ADDR(base) ((base) + (0x402E))
#define PMIC_I_IN_GATE2_ADDR(base) ((base) + (0x402F))
#define PMIC_OFFSET_CURRENT_MOD_0_ADDR(base) ((base) + (0x4030))
#define PMIC_OFFSET_CURRENT_MOD_1_ADDR(base) ((base) + (0x4031))
#define PMIC_OFFSET_CURRENT_MOD_2_ADDR(base) ((base) + (0x4032))
#define PMIC_OFFSET_VOLTAGE_MOD_0_ADDR(base) ((base) + (0x4033))
#define PMIC_OFFSET_VOLTAGE_MOD_1_ADDR(base) ((base) + (0x4034))
#define PMIC_OFFSET_VOLTAGE_MOD_2_ADDR(base) ((base) + (0x4035))
#define PMIC_STATE_TEST_ADDR(base) ((base) + (0x4036))
#define PMIC_CURRENT_0_ADDR(base) ((base) + (0x4037))
#define PMIC_CURRENT_1_ADDR(base) ((base) + (0x4038))
#define PMIC_CURRENT_2_ADDR(base) ((base) + (0x4039))
#define PMIC_V_OUT_0_ADDR(base) ((base) + (0x403A))
#define PMIC_V_OUT_1_ADDR(base) ((base) + (0x403B))
#define PMIC_V_OUT_2_ADDR(base) ((base) + (0x403C))
#define PMIC_OFFSET_CURRENT0_ADDR(base) ((base) + (0x403D))
#define PMIC_OFFSET_CURRENT1_ADDR(base) ((base) + (0x403E))
#define PMIC_OFFSET_CURRENT2_ADDR(base) ((base) + (0x403F))
#define PMIC_OFFSET_VOLTAGE0_ADDR(base) ((base) + (0x4040))
#define PMIC_OFFSET_VOLTAGE1_ADDR(base) ((base) + (0x4041))
#define PMIC_OFFSET_VOLTAGE2_ADDR(base) ((base) + (0x4042))
#define PMIC_OCV_VOLTAGE0_ADDR(base) ((base) + (0x4043))
#define PMIC_OCV_VOLTAGE1_ADDR(base) ((base) + (0x4044))
#define PMIC_OCV_VOLTAGE2_ADDR(base) ((base) + (0x4045))
#define PMIC_OCV_CURRENT0_ADDR(base) ((base) + (0x4046))
#define PMIC_OCV_CURRENT1_ADDR(base) ((base) + (0x4047))
#define PMIC_OCV_CURRENT2_ADDR(base) ((base) + (0x4048))
#define PMIC_OCV_TEMP0_ADDR(base) ((base) + (0x4049))
#define PMIC_OCV_TEMP1_ADDR(base) ((base) + (0x404A))
#define PMIC_OCV_TEMP2_ADDR(base) ((base) + (0x404B))
#define PMIC_ECO_OUT_CLIN_0_ADDR(base) ((base) + (0x404C))
#define PMIC_ECO_OUT_CLIN_1_ADDR(base) ((base) + (0x404D))
#define PMIC_ECO_OUT_CLIN_2_ADDR(base) ((base) + (0x404E))
#define PMIC_ECO_OUT_CLIN_3_ADDR(base) ((base) + (0x404F))
#define PMIC_ECO_OUT_CLIN_4_ADDR(base) ((base) + (0x4050))
#define PMIC_ECO_OUT_CLOUT_0_ADDR(base) ((base) + (0x4051))
#define PMIC_ECO_OUT_CLOUT_1_ADDR(base) ((base) + (0x4052))
#define PMIC_ECO_OUT_CLOUT_2_ADDR(base) ((base) + (0x4053))
#define PMIC_ECO_OUT_CLOUT_3_ADDR(base) ((base) + (0x4054))
#define PMIC_ECO_OUT_CLOUT_4_ADDR(base) ((base) + (0x4055))
#define PMIC_ECO_OUT_TEMP_0_ADDR(base) ((base) + (0x4056))
#define PMIC_ECO_OUT_TEMP_1_ADDR(base) ((base) + (0x4057))
#define PMIC_ECO_OUT_TEMP_2_ADDR(base) ((base) + (0x4058))
#define PMIC_TEMP0_RDATA_ADDR(base) ((base) + (0x4059))
#define PMIC_TEMP1_RDATA_ADDR(base) ((base) + (0x405A))
#define PMIC_TEMP2_RDATA_ADDR(base) ((base) + (0x405B))
#define PMIC_V_PRE0_OUT0_ADDR(base) ((base) + (0x405C))
#define PMIC_V_PRE0_OUT1_ADDR(base) ((base) + (0x405D))
#define PMIC_V_PRE0_OUT2_ADDR(base) ((base) + (0x405E))
#define PMIC_V_PRE1_OUT0_ADDR(base) ((base) + (0x405F))
#define PMIC_V_PRE1_OUT1_ADDR(base) ((base) + (0x4060))
#define PMIC_V_PRE1_OUT2_ADDR(base) ((base) + (0x4061))
#define PMIC_V_PRE2_OUT0_ADDR(base) ((base) + (0x4062))
#define PMIC_V_PRE2_OUT1_ADDR(base) ((base) + (0x4063))
#define PMIC_V_PRE2_OUT2_ADDR(base) ((base) + (0x4064))
#define PMIC_V_PRE3_OUT0_ADDR(base) ((base) + (0x4065))
#define PMIC_V_PRE3_OUT1_ADDR(base) ((base) + (0x4066))
#define PMIC_V_PRE3_OUT2_ADDR(base) ((base) + (0x4067))
#define PMIC_V_PRE4_OUT0_ADDR(base) ((base) + (0x4068))
#define PMIC_V_PRE4_OUT1_ADDR(base) ((base) + (0x4069))
#define PMIC_V_PRE4_OUT2_ADDR(base) ((base) + (0x406A))
#define PMIC_V_PRE5_OUT0_ADDR(base) ((base) + (0x406B))
#define PMIC_V_PRE5_OUT1_ADDR(base) ((base) + (0x406C))
#define PMIC_V_PRE5_OUT2_ADDR(base) ((base) + (0x406D))
#define PMIC_V_PRE6_OUT0_ADDR(base) ((base) + (0x406E))
#define PMIC_V_PRE6_OUT1_ADDR(base) ((base) + (0x406F))
#define PMIC_V_PRE6_OUT2_ADDR(base) ((base) + (0x4070))
#define PMIC_V_PRE7_OUT0_ADDR(base) ((base) + (0x4071))
#define PMIC_V_PRE7_OUT1_ADDR(base) ((base) + (0x4072))
#define PMIC_V_PRE7_OUT2_ADDR(base) ((base) + (0x4073))
#define PMIC_CURRENT_PRE0_OUT0_ADDR(base) ((base) + (0x4074))
#define PMIC_CURRENT_PRE0_OUT1_ADDR(base) ((base) + (0x4075))
#define PMIC_CURRENT_PRE0_OUT2_ADDR(base) ((base) + (0x4076))
#define PMIC_CURRENT_PRE1_OUT0_ADDR(base) ((base) + (0x4077))
#define PMIC_CURRENT_PRE1_OUT1_ADDR(base) ((base) + (0x4078))
#define PMIC_CURRENT_PRE1_OUT2_ADDR(base) ((base) + (0x4079))
#define PMIC_CURRENT_PRE2_OUT0_ADDR(base) ((base) + (0x407A))
#define PMIC_CURRENT_PRE2_OUT1_ADDR(base) ((base) + (0x407B))
#define PMIC_CURRENT_PRE2_OUT2_ADDR(base) ((base) + (0x407C))
#define PMIC_CURRENT_PRE3_OUT0_ADDR(base) ((base) + (0x407D))
#define PMIC_CURRENT_PRE3_OUT1_ADDR(base) ((base) + (0x407E))
#define PMIC_CURRENT_PRE3_OUT2_ADDR(base) ((base) + (0x407F))
#define PMIC_CURRENT_PRE4_OUT0_ADDR(base) ((base) + (0x4080))
#define PMIC_CURRENT_PRE4_OUT1_ADDR(base) ((base) + (0x4081))
#define PMIC_CURRENT_PRE4_OUT2_ADDR(base) ((base) + (0x4082))
#define PMIC_CURRENT_PRE5_OUT0_ADDR(base) ((base) + (0x4083))
#define PMIC_CURRENT_PRE5_OUT1_ADDR(base) ((base) + (0x4084))
#define PMIC_CURRENT_PRE5_OUT2_ADDR(base) ((base) + (0x4085))
#define PMIC_CURRENT_PRE6_OUT0_ADDR(base) ((base) + (0x4086))
#define PMIC_CURRENT_PRE6_OUT1_ADDR(base) ((base) + (0x4087))
#define PMIC_CURRENT_PRE6_OUT2_ADDR(base) ((base) + (0x4088))
#define PMIC_CURRENT_PRE7_OUT0_ADDR(base) ((base) + (0x4089))
#define PMIC_CURRENT_PRE7_OUT1_ADDR(base) ((base) + (0x408A))
#define PMIC_CURRENT_PRE7_OUT2_ADDR(base) ((base) + (0x408B))
#define PMIC_V_OCV_PRE1_OUT0_ADDR(base) ((base) + (0x408C))
#define PMIC_V_OCV_PRE1_OUT1_ADDR(base) ((base) + (0x408D))
#define PMIC_V_OCV_PRE1_OUT2_ADDR(base) ((base) + (0x408E))
#define PMIC_V_OCV_PRE2_OUT0_ADDR(base) ((base) + (0x408F))
#define PMIC_V_OCV_PRE2_OUT1_ADDR(base) ((base) + (0x4090))
#define PMIC_V_OCV_PRE2_OUT2_ADDR(base) ((base) + (0x4091))
#define PMIC_V_OCV_PRE3_OUT0_ADDR(base) ((base) + (0x4092))
#define PMIC_V_OCV_PRE3_OUT1_ADDR(base) ((base) + (0x4093))
#define PMIC_V_OCV_PRE3_OUT2_ADDR(base) ((base) + (0x4094))
#define PMIC_V_OCV_PRE4_OUT0_ADDR(base) ((base) + (0x4095))
#define PMIC_V_OCV_PRE4_OUT1_ADDR(base) ((base) + (0x4096))
#define PMIC_V_OCV_PRE4_OUT2_ADDR(base) ((base) + (0x4097))
#define PMIC_I_OCV_PRE1_OUT0_ADDR(base) ((base) + (0x4098))
#define PMIC_I_OCV_PRE1_OUT1_ADDR(base) ((base) + (0x4099))
#define PMIC_I_OCV_PRE1_OUT2_ADDR(base) ((base) + (0x409A))
#define PMIC_I_OCV_PRE2_OUT0_ADDR(base) ((base) + (0x409B))
#define PMIC_I_OCV_PRE2_OUT1_ADDR(base) ((base) + (0x409C))
#define PMIC_I_OCV_PRE2_OUT2_ADDR(base) ((base) + (0x409D))
#define PMIC_I_OCV_PRE3_OUT0_ADDR(base) ((base) + (0x409E))
#define PMIC_I_OCV_PRE3_OUT1_ADDR(base) ((base) + (0x409F))
#define PMIC_I_OCV_PRE3_OUT2_ADDR(base) ((base) + (0x40A0))
#define PMIC_I_OCV_PRE4_OUT0_ADDR(base) ((base) + (0x40A1))
#define PMIC_I_OCV_PRE4_OUT1_ADDR(base) ((base) + (0x40A2))
#define PMIC_I_OCV_PRE4_OUT2_ADDR(base) ((base) + (0x40A3))
#define PMIC_T_OCV_PRE1_OUT0_ADDR(base) ((base) + (0x40A4))
#define PMIC_T_OCV_PRE1_OUT1_ADDR(base) ((base) + (0x40A5))
#define PMIC_T_OCV_PRE2_OUT0_ADDR(base) ((base) + (0x40A6))
#define PMIC_T_OCV_PRE2_OUT1_ADDR(base) ((base) + (0x40A7))
#define PMIC_T_OCV_PRE3_OUT0_ADDR(base) ((base) + (0x40A8))
#define PMIC_T_OCV_PRE3_OUT1_ADDR(base) ((base) + (0x40A9))
#define PMIC_T_OCV_PRE4_OUT0_ADDR(base) ((base) + (0x40AA))
#define PMIC_T_OCV_PRE4_OUT1_ADDR(base) ((base) + (0x40AB))
#define PMIC_RTC_OCV_OUT_ADDR(base) ((base) + (0x40AC))
#define PMIC_SLIDE_I_DATA0_OUT0_ADDR(base) ((base) + (0x40AD))
#define PMIC_SLIDE_I_DATA0_OUT1_ADDR(base) ((base) + (0x40AE))
#define PMIC_SLIDE_I_DATA0_OUT2_ADDR(base) ((base) + (0x40AF))
#define PMIC_SLIDE_I_DATA1_OUT0_ADDR(base) ((base) + (0x40B0))
#define PMIC_SLIDE_I_DATA1_OUT1_ADDR(base) ((base) + (0x40B1))
#define PMIC_SLIDE_I_DATA1_OUT2_ADDR(base) ((base) + (0x40B2))
#define PMIC_SLIDE_I_DATA2_OUT0_ADDR(base) ((base) + (0x40B3))
#define PMIC_SLIDE_I_DATA2_OUT1_ADDR(base) ((base) + (0x40B4))
#define PMIC_SLIDE_I_DATA2_OUT2_ADDR(base) ((base) + (0x40B5))
#define PMIC_SLIDE_I_DATA3_OUT0_ADDR(base) ((base) + (0x40B6))
#define PMIC_SLIDE_I_DATA3_OUT1_ADDR(base) ((base) + (0x40B7))
#define PMIC_SLIDE_I_DATA3_OUT2_ADDR(base) ((base) + (0x40B8))
#define PMIC_SLIDE_V_DATA0_OUT0_ADDR(base) ((base) + (0x40B9))
#define PMIC_SLIDE_V_DATA0_OUT1_ADDR(base) ((base) + (0x40BA))
#define PMIC_SLIDE_V_DATA0_OUT2_ADDR(base) ((base) + (0x40BB))
#define PMIC_SLIDE_V_DATA1_OUT0_ADDR(base) ((base) + (0x40BC))
#define PMIC_SLIDE_V_DATA1_OUT1_ADDR(base) ((base) + (0x40BD))
#define PMIC_SLIDE_V_DATA1_OUT2_ADDR(base) ((base) + (0x40BE))
#define PMIC_SLIDE_V_DATA2_OUT0_ADDR(base) ((base) + (0x40BF))
#define PMIC_SLIDE_V_DATA2_OUT1_ADDR(base) ((base) + (0x40C0))
#define PMIC_SLIDE_V_DATA2_OUT2_ADDR(base) ((base) + (0x40C1))
#define PMIC_SLIDE_V_DATA3_OUT0_ADDR(base) ((base) + (0x40C2))
#define PMIC_SLIDE_V_DATA3_OUT1_ADDR(base) ((base) + (0x40C3))
#define PMIC_SLIDE_V_DATA3_OUT2_ADDR(base) ((base) + (0x40C4))
#define PMIC_SLIDE_CNT_DATA_OUT0_ADDR(base) ((base) + (0x40C5))
#define PMIC_SLIDE_CNT_DATA_OUT1_ADDR(base) ((base) + (0x40C6))
#define PMIC_SLIDE_CNT_DATA_OUT2_ADDR(base) ((base) + (0x40C7))
#define PMIC_COUL_RESERVE0_ADDR(base) ((base) + (0x40C8))
#define PMIC_COUL_RESERVE1_ADDR(base) ((base) + (0x40C9))
#define PMIC_CLJ_DEBUG0_ADDR(base) ((base) + (0x40CA))
#define PMIC_CLJ_DEBUG1_ADDR(base) ((base) + (0x40CB))
#define PMIC_DEBUG_CIC_I0_ADDR(base) ((base) + (0x40CC))
#define PMIC_DEBUG_CIC_I1_ADDR(base) ((base) + (0x40CD))
#define PMIC_DEBUG_CIC_I2_ADDR(base) ((base) + (0x40CE))
#define PMIC_DEBUG_CIC_V0_ADDR(base) ((base) + (0x40CF))
#define PMIC_DEBUG_CIC_V1_ADDR(base) ((base) + (0x40D0))
#define PMIC_DEBUG_CIC_V2_ADDR(base) ((base) + (0x40D1))
#define PMIC_DEBUG_WRITE_PRO_ADDR(base) ((base) + (0x40D2))
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_ACR_CFG0_ADDR(base) ((base) + (0x8000UL))
#define PMIC_ACR_CFG1_ADDR(base) ((base) + (0x8001UL))
#define PMIC_ACR_RESERVE_CFG_ADDR(base) ((base) + (0x8002UL))
#define PMIC_ACRADC_CTRL_ADDR(base) ((base) + (0x8010UL))
#define PMIC_ACRADC_START_ADDR(base) ((base) + (0x8011UL))
#define PMIC_ACRCONV_STATUS_ADDR(base) ((base) + (0x8012UL))
#define PMIC_ACRADC_DATA_L_ADDR(base) ((base) + (0x8013UL))
#define PMIC_ACRADC_DATA_H_ADDR(base) ((base) + (0x8014UL))
#define PMIC_ACRADC_TDIE_DATA_L_ADDR(base) ((base) + (0x8015UL))
#define PMIC_ACRADC_TDIE_DATA_H_ADDR(base) ((base) + (0x8016UL))
#define PMIC_ACRADC_TBAT_DATA_L_ADDR(base) ((base) + (0x8017UL))
#define PMIC_ACRADC_TBAT_DATA_H_ADDR(base) ((base) + (0x8018UL))
#define PMIC_ACRADC_CONV_ADDR(base) ((base) + (0x8019UL))
#define PMIC_ACRADC_CURRENT_ADDR(base) ((base) + (0x801AUL))
#define PMIC_ACRADC_CALI_CTRL_ADDR(base) ((base) + (0x801BUL))
#define PMIC_ACRADC_CALI_VALUE_ADDR(base) ((base) + (0x801CUL))
#define PMIC_ACRADC_CALI_CFG_ADDR(base) ((base) + (0x801DUL))
#define PMIC_ACRADC_TDIE_CALI_DATA_ADDR(base) ((base) + (0x801EUL))
#define PMIC_ACRADC_TBAT_CALI_DATA_ADDR(base) ((base) + (0x801FUL))
#define PMIC_ACRADC_TDIE_CALI_CFG_ADDR(base) ((base) + (0x8020UL))
#define PMIC_ACRADC_TBAT_CALI_CFG_ADDR(base) ((base) + (0x8021UL))
#define PMIC_ACRADC_MODE_CFG_ADDR(base) ((base) + (0x8022UL))
#define PMIC_ACRADC_CALIVALUE_CFG1_ADDR(base) ((base) + (0x8023UL))
#define PMIC_ACRADC_CALIVALUE_CFG2_ADDR(base) ((base) + (0x8024UL))
#define PMIC_ACRADC_TDIE_OFFSET_CFGL_ADDR(base) ((base) + (0x8025UL))
#define PMIC_ACRADC_TDIE_OFFSET_CFGH_ADDR(base) ((base) + (0x8026UL))
#define PMIC_ACRADC_TBAT_OFFSET_CFGL_ADDR(base) ((base) + (0x8027UL))
#define PMIC_ACRADC_TBAT_OFFSET_CFGH_ADDR(base) ((base) + (0x8028UL))
#define PMIC_SOH_EN_ADDR(base) ((base) + (0x8040UL))
#define PMIC_SOH_DET_TIMER_ADDR(base) ((base) + (0x8041UL))
#define PMIC_SOH_TBAT_OVH_L_ADDR(base) ((base) + (0x8042UL))
#define PMIC_SOH_TBAT_OVH_H_ADDR(base) ((base) + (0x8043UL))
#define PMIC_SOH_VBAT_UVP_L_ADDR(base) ((base) + (0x8044UL))
#define PMIC_SOH_VBAT_UVP_H_ADDR(base) ((base) + (0x8045UL))
#define PMIC_SOH_MODE_ADDR(base) ((base) + (0x8060UL))
#define PMIC_ACR_PULSE_NUM_ADDR(base) ((base) + (0x8061UL))
#define PMIC_ACR_SAMPLE_TIME_H_ADDR(base) ((base) + (0x8062UL))
#define PMIC_ACR_SAMPLE_TIME_L_ADDR(base) ((base) + (0x8063UL))
#define PMIC_ACR_DATA0_L_ADDR(base) ((base) + (0x8064UL))
#define PMIC_ACR_DATA0_H_ADDR(base) ((base) + (0x8065UL))
#define PMIC_ACR_DATA1_L_ADDR(base) ((base) + (0x8066UL))
#define PMIC_ACR_DATA1_H_ADDR(base) ((base) + (0x8067UL))
#define PMIC_ACR_DATA2_L_ADDR(base) ((base) + (0x8068UL))
#define PMIC_ACR_DATA2_H_ADDR(base) ((base) + (0x8069UL))
#define PMIC_ACR_DATA3_L_ADDR(base) ((base) + (0x806AUL))
#define PMIC_ACR_DATA3_H_ADDR(base) ((base) + (0x806BUL))
#define PMIC_ACR_DATA4_L_ADDR(base) ((base) + (0x806CUL))
#define PMIC_ACR_DATA4_H_ADDR(base) ((base) + (0x806DUL))
#define PMIC_ACR_DATA5_L_ADDR(base) ((base) + (0x806EUL))
#define PMIC_ACR_DATA5_H_ADDR(base) ((base) + (0x806FUL))
#define PMIC_ACR_DATA6_L_ADDR(base) ((base) + (0x8070UL))
#define PMIC_ACR_DATA6_H_ADDR(base) ((base) + (0x8071UL))
#define PMIC_ACR_DATA7_L_ADDR(base) ((base) + (0x8072UL))
#define PMIC_ACR_DATA7_H_ADDR(base) ((base) + (0x8073UL))
#define PMIC_OVP_CTRL_ADDR(base) ((base) + (0x8090UL))
#define PMIC_OVP_VBAT_OVH_TH0_L_ADDR(base) ((base) + (0x8091UL))
#define PMIC_OVP_VBAT_OVH_TH0_H_ADDR(base) ((base) + (0x8092UL))
#define PMIC_OVP_TBAT_OVH_TH0_L_ADDR(base) ((base) + (0x8093UL))
#define PMIC_OVP_TBAT_OVH_TH0_H_ADDR(base) ((base) + (0x8094UL))
#define PMIC_OVP_VBAT_OVH_TH1_L_ADDR(base) ((base) + (0x8095UL))
#define PMIC_OVP_VBAT_OVH_TH1_H_ADDR(base) ((base) + (0x8096UL))
#define PMIC_OVP_TBAT_OVH_TH1_L_ADDR(base) ((base) + (0x8097UL))
#define PMIC_OVP_TBAT_OVH_TH1_H_ADDR(base) ((base) + (0x8098UL))
#define PMIC_OVP_VBAT_OVH_TH2_L_ADDR(base) ((base) + (0x8099UL))
#define PMIC_OVP_VBAT_OVH_TH2_H_ADDR(base) ((base) + (0x809AUL))
#define PMIC_OVP_TBAT_OVH_TH2_L_ADDR(base) ((base) + (0x809BUL))
#define PMIC_OVP_TBAT_OVH_TH2_H_ADDR(base) ((base) + (0x809CUL))
#define PMIC_SOH_OVP_REAL_ADDR(base) ((base) + (0x809DUL))
#define PMIC_DCR_CONFIG_ADDR(base) ((base) + (0x80B0UL))
#define PMIC_DCR_TIMER_CONFIG0_ADDR(base) ((base) + (0x80B1UL))
#define PMIC_DCR_TIMER_CONFIG1_ADDR(base) ((base) + (0x80B2UL))
#define PMIC_DCR_COUNT_CONFIG_ADDR(base) ((base) + (0x80B3UL))
#define PMIC_IBAT1_0_ADDR(base) ((base) + (0x80B4UL))
#define PMIC_IBAT1_1_ADDR(base) ((base) + (0x80B5UL))
#define PMIC_IBAT1_2_ADDR(base) ((base) + (0x80B6UL))
#define PMIC_VBAT1_0_ADDR(base) ((base) + (0x80B7UL))
#define PMIC_VBAT1_1_ADDR(base) ((base) + (0x80B8UL))
#define PMIC_VBAT1_2_ADDR(base) ((base) + (0x80B9UL))
#define PMIC_TMR1_0_ADDR(base) ((base) + (0x80BAUL))
#define PMIC_TMR1_1_ADDR(base) ((base) + (0x80BBUL))
#define PMIC_IBAT2_0_ADDR(base) ((base) + (0x80BCUL))
#define PMIC_IBAT2_1_ADDR(base) ((base) + (0x80BDUL))
#define PMIC_IBAT2_2_ADDR(base) ((base) + (0x80BEUL))
#define PMIC_VBAT2_0_ADDR(base) ((base) + (0x80BFUL))
#define PMIC_VBAT2_1_ADDR(base) ((base) + (0x80C0UL))
#define PMIC_VBAT2_2_ADDR(base) ((base) + (0x80C1UL))
#define PMIC_TMR2_0_ADDR(base) ((base) + (0x80C2UL))
#define PMIC_TMR2_1_ADDR(base) ((base) + (0x80C3UL))
#define PMIC_SOH_SOFT_RST_ADDR(base) ((base) + (0x80E0UL))
#define PMIC_TEST_BUS_SEL_ADDR(base) ((base) + (0x80E1UL))
#define PMIC_ACR_TB_BUS_0_ADDR(base) ((base) + (0x80E2UL))
#define PMIC_ACR_TB_BUS_1_ADDR(base) ((base) + (0x80E3UL))
#define PMIC_ACR_CLK_GT_EN_ADDR(base) ((base) + (0x80E4UL))
#else
#define PMIC_ACR_CFG0_ADDR(base) ((base) + (0x8000))
#define PMIC_ACR_CFG1_ADDR(base) ((base) + (0x8001))
#define PMIC_ACR_RESERVE_CFG_ADDR(base) ((base) + (0x8002))
#define PMIC_ACRADC_CTRL_ADDR(base) ((base) + (0x8010))
#define PMIC_ACRADC_START_ADDR(base) ((base) + (0x8011))
#define PMIC_ACRCONV_STATUS_ADDR(base) ((base) + (0x8012))
#define PMIC_ACRADC_DATA_L_ADDR(base) ((base) + (0x8013))
#define PMIC_ACRADC_DATA_H_ADDR(base) ((base) + (0x8014))
#define PMIC_ACRADC_TDIE_DATA_L_ADDR(base) ((base) + (0x8015))
#define PMIC_ACRADC_TDIE_DATA_H_ADDR(base) ((base) + (0x8016))
#define PMIC_ACRADC_TBAT_DATA_L_ADDR(base) ((base) + (0x8017))
#define PMIC_ACRADC_TBAT_DATA_H_ADDR(base) ((base) + (0x8018))
#define PMIC_ACRADC_CONV_ADDR(base) ((base) + (0x8019))
#define PMIC_ACRADC_CURRENT_ADDR(base) ((base) + (0x801A))
#define PMIC_ACRADC_CALI_CTRL_ADDR(base) ((base) + (0x801B))
#define PMIC_ACRADC_CALI_VALUE_ADDR(base) ((base) + (0x801C))
#define PMIC_ACRADC_CALI_CFG_ADDR(base) ((base) + (0x801D))
#define PMIC_ACRADC_TDIE_CALI_DATA_ADDR(base) ((base) + (0x801E))
#define PMIC_ACRADC_TBAT_CALI_DATA_ADDR(base) ((base) + (0x801F))
#define PMIC_ACRADC_TDIE_CALI_CFG_ADDR(base) ((base) + (0x8020))
#define PMIC_ACRADC_TBAT_CALI_CFG_ADDR(base) ((base) + (0x8021))
#define PMIC_ACRADC_MODE_CFG_ADDR(base) ((base) + (0x8022))
#define PMIC_ACRADC_CALIVALUE_CFG1_ADDR(base) ((base) + (0x8023))
#define PMIC_ACRADC_CALIVALUE_CFG2_ADDR(base) ((base) + (0x8024))
#define PMIC_ACRADC_TDIE_OFFSET_CFGL_ADDR(base) ((base) + (0x8025))
#define PMIC_ACRADC_TDIE_OFFSET_CFGH_ADDR(base) ((base) + (0x8026))
#define PMIC_ACRADC_TBAT_OFFSET_CFGL_ADDR(base) ((base) + (0x8027))
#define PMIC_ACRADC_TBAT_OFFSET_CFGH_ADDR(base) ((base) + (0x8028))
#define PMIC_SOH_EN_ADDR(base) ((base) + (0x8040))
#define PMIC_SOH_DET_TIMER_ADDR(base) ((base) + (0x8041))
#define PMIC_SOH_TBAT_OVH_L_ADDR(base) ((base) + (0x8042))
#define PMIC_SOH_TBAT_OVH_H_ADDR(base) ((base) + (0x8043))
#define PMIC_SOH_VBAT_UVP_L_ADDR(base) ((base) + (0x8044))
#define PMIC_SOH_VBAT_UVP_H_ADDR(base) ((base) + (0x8045))
#define PMIC_SOH_MODE_ADDR(base) ((base) + (0x8060))
#define PMIC_ACR_PULSE_NUM_ADDR(base) ((base) + (0x8061))
#define PMIC_ACR_SAMPLE_TIME_H_ADDR(base) ((base) + (0x8062))
#define PMIC_ACR_SAMPLE_TIME_L_ADDR(base) ((base) + (0x8063))
#define PMIC_ACR_DATA0_L_ADDR(base) ((base) + (0x8064))
#define PMIC_ACR_DATA0_H_ADDR(base) ((base) + (0x8065))
#define PMIC_ACR_DATA1_L_ADDR(base) ((base) + (0x8066))
#define PMIC_ACR_DATA1_H_ADDR(base) ((base) + (0x8067))
#define PMIC_ACR_DATA2_L_ADDR(base) ((base) + (0x8068))
#define PMIC_ACR_DATA2_H_ADDR(base) ((base) + (0x8069))
#define PMIC_ACR_DATA3_L_ADDR(base) ((base) + (0x806A))
#define PMIC_ACR_DATA3_H_ADDR(base) ((base) + (0x806B))
#define PMIC_ACR_DATA4_L_ADDR(base) ((base) + (0x806C))
#define PMIC_ACR_DATA4_H_ADDR(base) ((base) + (0x806D))
#define PMIC_ACR_DATA5_L_ADDR(base) ((base) + (0x806E))
#define PMIC_ACR_DATA5_H_ADDR(base) ((base) + (0x806F))
#define PMIC_ACR_DATA6_L_ADDR(base) ((base) + (0x8070))
#define PMIC_ACR_DATA6_H_ADDR(base) ((base) + (0x8071))
#define PMIC_ACR_DATA7_L_ADDR(base) ((base) + (0x8072))
#define PMIC_ACR_DATA7_H_ADDR(base) ((base) + (0x8073))
#define PMIC_OVP_CTRL_ADDR(base) ((base) + (0x8090))
#define PMIC_OVP_VBAT_OVH_TH0_L_ADDR(base) ((base) + (0x8091))
#define PMIC_OVP_VBAT_OVH_TH0_H_ADDR(base) ((base) + (0x8092))
#define PMIC_OVP_TBAT_OVH_TH0_L_ADDR(base) ((base) + (0x8093))
#define PMIC_OVP_TBAT_OVH_TH0_H_ADDR(base) ((base) + (0x8094))
#define PMIC_OVP_VBAT_OVH_TH1_L_ADDR(base) ((base) + (0x8095))
#define PMIC_OVP_VBAT_OVH_TH1_H_ADDR(base) ((base) + (0x8096))
#define PMIC_OVP_TBAT_OVH_TH1_L_ADDR(base) ((base) + (0x8097))
#define PMIC_OVP_TBAT_OVH_TH1_H_ADDR(base) ((base) + (0x8098))
#define PMIC_OVP_VBAT_OVH_TH2_L_ADDR(base) ((base) + (0x8099))
#define PMIC_OVP_VBAT_OVH_TH2_H_ADDR(base) ((base) + (0x809A))
#define PMIC_OVP_TBAT_OVH_TH2_L_ADDR(base) ((base) + (0x809B))
#define PMIC_OVP_TBAT_OVH_TH2_H_ADDR(base) ((base) + (0x809C))
#define PMIC_SOH_OVP_REAL_ADDR(base) ((base) + (0x809D))
#define PMIC_DCR_CONFIG_ADDR(base) ((base) + (0x80B0))
#define PMIC_DCR_TIMER_CONFIG0_ADDR(base) ((base) + (0x80B1))
#define PMIC_DCR_TIMER_CONFIG1_ADDR(base) ((base) + (0x80B2))
#define PMIC_DCR_COUNT_CONFIG_ADDR(base) ((base) + (0x80B3))
#define PMIC_IBAT1_0_ADDR(base) ((base) + (0x80B4))
#define PMIC_IBAT1_1_ADDR(base) ((base) + (0x80B5))
#define PMIC_IBAT1_2_ADDR(base) ((base) + (0x80B6))
#define PMIC_VBAT1_0_ADDR(base) ((base) + (0x80B7))
#define PMIC_VBAT1_1_ADDR(base) ((base) + (0x80B8))
#define PMIC_VBAT1_2_ADDR(base) ((base) + (0x80B9))
#define PMIC_TMR1_0_ADDR(base) ((base) + (0x80BA))
#define PMIC_TMR1_1_ADDR(base) ((base) + (0x80BB))
#define PMIC_IBAT2_0_ADDR(base) ((base) + (0x80BC))
#define PMIC_IBAT2_1_ADDR(base) ((base) + (0x80BD))
#define PMIC_IBAT2_2_ADDR(base) ((base) + (0x80BE))
#define PMIC_VBAT2_0_ADDR(base) ((base) + (0x80BF))
#define PMIC_VBAT2_1_ADDR(base) ((base) + (0x80C0))
#define PMIC_VBAT2_2_ADDR(base) ((base) + (0x80C1))
#define PMIC_TMR2_0_ADDR(base) ((base) + (0x80C2))
#define PMIC_TMR2_1_ADDR(base) ((base) + (0x80C3))
#define PMIC_SOH_SOFT_RST_ADDR(base) ((base) + (0x80E0))
#define PMIC_TEST_BUS_SEL_ADDR(base) ((base) + (0x80E1))
#define PMIC_ACR_TB_BUS_0_ADDR(base) ((base) + (0x80E2))
#define PMIC_ACR_TB_BUS_1_ADDR(base) ((base) + (0x80E3))
#define PMIC_ACR_CLK_GT_EN_ADDR(base) ((base) + (0x80E4))
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_DAC0_OUT_ADDR(base) ((base) + (0x0UL))
#define PMIC_DAC0_LSB_ADDR(base) ((base) + (0x1UL))
#else
#define PMIC_DAC0_OUT_ADDR(base) ((base) + (0x0))
#define PMIC_DAC0_LSB_ADDR(base) ((base) + (0x1))
#endif
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
        unsigned char st_vsys_pwroff_abs_d20nr : 1;
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
#define PMIC_STATUS0_st_vsys_pwroff_abs_d20nr_START (2)
#define PMIC_STATUS0_st_vsys_pwroff_abs_d20nr_END (2)
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
        unsigned char st_sim0_hpd_d540u : 1;
        unsigned char st_sim1_hpd_d540u : 1;
        unsigned char st_avdd_osc_vld_d20nf : 1;
        unsigned char st_nfc_pwr_en_d5mr : 1;
        unsigned char st_ufs_sel_d20n : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_STATUS1_UNION;
#endif
#define PMIC_STATUS1_st_dcxo_clk_sel_START (0)
#define PMIC_STATUS1_st_dcxo_clk_sel_END (0)
#define PMIC_STATUS1_st_sim0_hpd_d540u_START (1)
#define PMIC_STATUS1_st_sim0_hpd_d540u_END (1)
#define PMIC_STATUS1_st_sim1_hpd_d540u_START (2)
#define PMIC_STATUS1_st_sim1_hpd_d540u_END (2)
#define PMIC_STATUS1_st_avdd_osc_vld_d20nf_START (3)
#define PMIC_STATUS1_st_avdd_osc_vld_d20nf_END (3)
#define PMIC_STATUS1_st_nfc_pwr_en_d5mr_START (4)
#define PMIC_STATUS1_st_nfc_pwr_en_d5mr_END (4)
#define PMIC_STATUS1_st_ufs_sel_d20n_START (5)
#define PMIC_STATUS1_st_ufs_sel_d20n_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_ocp_state : 1;
        unsigned char buck4_ocp_state : 1;
        unsigned char buck3_ocp_state : 1;
        unsigned char buck2_ocp_state : 1;
        unsigned char buck1_ocp_state : 1;
        unsigned char buck0_ocp_state : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_STATUS2_UNION;
#endif
#define PMIC_STATUS2_buck5_ocp_state_START (0)
#define PMIC_STATUS2_buck5_ocp_state_END (0)
#define PMIC_STATUS2_buck4_ocp_state_START (1)
#define PMIC_STATUS2_buck4_ocp_state_END (1)
#define PMIC_STATUS2_buck3_ocp_state_START (2)
#define PMIC_STATUS2_buck3_ocp_state_END (2)
#define PMIC_STATUS2_buck2_ocp_state_START (3)
#define PMIC_STATUS2_buck2_ocp_state_END (3)
#define PMIC_STATUS2_buck1_ocp_state_START (4)
#define PMIC_STATUS2_buck1_ocp_state_END (4)
#define PMIC_STATUS2_buck0_ocp_state_START (5)
#define PMIC_STATUS2_buck0_ocp_state_END (5)
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
        unsigned char st_buck0_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_BUCK0_ONOFF_ECO_UNION;
#endif
#define PMIC_BUCK0_ONOFF_ECO_reg_buck0_en_START (0)
#define PMIC_BUCK0_ONOFF_ECO_reg_buck0_en_END (0)
#define PMIC_BUCK0_ONOFF_ECO_st_buck0_en_START (1)
#define PMIC_BUCK0_ONOFF_ECO_st_buck0_en_END (1)
#define PMIC_BUCK0_ONOFF_ECO_reg_buck0_eco_en_START (4)
#define PMIC_BUCK0_ONOFF_ECO_reg_buck0_eco_en_END (4)
#define PMIC_BUCK0_ONOFF_ECO_st_buck0_eco_en_START (5)
#define PMIC_BUCK0_ONOFF_ECO_st_buck0_eco_en_END (5)
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
        unsigned char st_buck1_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_BUCK1_ONOFF_ECO_UNION;
#endif
#define PMIC_BUCK1_ONOFF_ECO_reg_buck1_en_START (0)
#define PMIC_BUCK1_ONOFF_ECO_reg_buck1_en_END (0)
#define PMIC_BUCK1_ONOFF_ECO_st_buck1_en_START (1)
#define PMIC_BUCK1_ONOFF_ECO_st_buck1_en_END (1)
#define PMIC_BUCK1_ONOFF_ECO_reg_buck1_eco_en_START (4)
#define PMIC_BUCK1_ONOFF_ECO_reg_buck1_eco_en_END (4)
#define PMIC_BUCK1_ONOFF_ECO_st_buck1_eco_en_START (5)
#define PMIC_BUCK1_ONOFF_ECO_st_buck1_eco_en_END (5)
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
        unsigned char st_buck2_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_BUCK2_ONOFF_ECO_UNION;
#endif
#define PMIC_BUCK2_ONOFF_ECO_reg_buck2_en_START (0)
#define PMIC_BUCK2_ONOFF_ECO_reg_buck2_en_END (0)
#define PMIC_BUCK2_ONOFF_ECO_st_buck2_en_START (1)
#define PMIC_BUCK2_ONOFF_ECO_st_buck2_en_END (1)
#define PMIC_BUCK2_ONOFF_ECO_reg_buck2_eco_en_START (4)
#define PMIC_BUCK2_ONOFF_ECO_reg_buck2_eco_en_END (4)
#define PMIC_BUCK2_ONOFF_ECO_st_buck2_eco_en_START (5)
#define PMIC_BUCK2_ONOFF_ECO_st_buck2_eco_en_END (5)
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
        unsigned char st_buck3_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_BUCK3_ONOFF_ECO_UNION;
#endif
#define PMIC_BUCK3_ONOFF_ECO_reg_buck3_en_START (0)
#define PMIC_BUCK3_ONOFF_ECO_reg_buck3_en_END (0)
#define PMIC_BUCK3_ONOFF_ECO_st_buck3_en_START (1)
#define PMIC_BUCK3_ONOFF_ECO_st_buck3_en_END (1)
#define PMIC_BUCK3_ONOFF_ECO_reg_buck3_eco_en_START (4)
#define PMIC_BUCK3_ONOFF_ECO_reg_buck3_eco_en_END (4)
#define PMIC_BUCK3_ONOFF_ECO_st_buck3_eco_en_START (5)
#define PMIC_BUCK3_ONOFF_ECO_st_buck3_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_buck4_en : 1;
        unsigned char st_buck4_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_buck4_eco_en : 1;
        unsigned char st_buck4_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_BUCK4_ONOFF_ECO_UNION;
#endif
#define PMIC_BUCK4_ONOFF_ECO_reg_buck4_en_START (0)
#define PMIC_BUCK4_ONOFF_ECO_reg_buck4_en_END (0)
#define PMIC_BUCK4_ONOFF_ECO_st_buck4_en_START (1)
#define PMIC_BUCK4_ONOFF_ECO_st_buck4_en_END (1)
#define PMIC_BUCK4_ONOFF_ECO_reg_buck4_eco_en_START (4)
#define PMIC_BUCK4_ONOFF_ECO_reg_buck4_eco_en_END (4)
#define PMIC_BUCK4_ONOFF_ECO_st_buck4_eco_en_START (5)
#define PMIC_BUCK4_ONOFF_ECO_st_buck4_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_buck5_en : 1;
        unsigned char st_buck5_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_buck5_eco_en : 1;
        unsigned char st_buck5_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_BUCK5_ONOFF_ECO_UNION;
#endif
#define PMIC_BUCK5_ONOFF_ECO_reg_buck5_en_START (0)
#define PMIC_BUCK5_ONOFF_ECO_reg_buck5_en_END (0)
#define PMIC_BUCK5_ONOFF_ECO_st_buck5_en_START (1)
#define PMIC_BUCK5_ONOFF_ECO_st_buck5_en_END (1)
#define PMIC_BUCK5_ONOFF_ECO_reg_buck5_eco_en_START (4)
#define PMIC_BUCK5_ONOFF_ECO_reg_buck5_eco_en_END (4)
#define PMIC_BUCK5_ONOFF_ECO_st_buck5_eco_en_START (5)
#define PMIC_BUCK5_ONOFF_ECO_st_buck5_eco_en_END (5)
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
        unsigned char st_ldo0_2_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO0_2_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO0_2_ONOFF_ECO_reg_ldo0_2_en_START (0)
#define PMIC_LDO0_2_ONOFF_ECO_reg_ldo0_2_en_END (0)
#define PMIC_LDO0_2_ONOFF_ECO_st_ldo0_2_en_START (1)
#define PMIC_LDO0_2_ONOFF_ECO_st_ldo0_2_en_END (1)
#define PMIC_LDO0_2_ONOFF_ECO_reg_ldo0_2_eco_en_START (4)
#define PMIC_LDO0_2_ONOFF_ECO_reg_ldo0_2_eco_en_END (4)
#define PMIC_LDO0_2_ONOFF_ECO_st_ldo0_2_eco_en_START (5)
#define PMIC_LDO0_2_ONOFF_ECO_st_ldo0_2_eco_en_END (5)
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
} PMIC_LDO1_ONOFF_UNION;
#endif
#define PMIC_LDO1_ONOFF_reg_ldo1_en_START (0)
#define PMIC_LDO1_ONOFF_reg_ldo1_en_END (0)
#define PMIC_LDO1_ONOFF_st_ldo1_en_START (1)
#define PMIC_LDO1_ONOFF_st_ldo1_en_END (1)
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
        unsigned char st_ldo2_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO2_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO2_ONOFF_ECO_reg_ldo2_en_START (0)
#define PMIC_LDO2_ONOFF_ECO_reg_ldo2_en_END (0)
#define PMIC_LDO2_ONOFF_ECO_st_ldo2_en_START (1)
#define PMIC_LDO2_ONOFF_ECO_st_ldo2_en_END (1)
#define PMIC_LDO2_ONOFF_ECO_reg_ldo2_eco_en_START (4)
#define PMIC_LDO2_ONOFF_ECO_reg_ldo2_eco_en_END (4)
#define PMIC_LDO2_ONOFF_ECO_st_ldo2_eco_en_START (5)
#define PMIC_LDO2_ONOFF_ECO_st_ldo2_eco_en_END (5)
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
} PMIC_LDO3_ONOFF_UNION;
#endif
#define PMIC_LDO3_ONOFF_reg_ldo3_en_START (0)
#define PMIC_LDO3_ONOFF_reg_ldo3_en_END (0)
#define PMIC_LDO3_ONOFF_st_ldo3_en_START (1)
#define PMIC_LDO3_ONOFF_st_ldo3_en_END (1)
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
        unsigned char st_ldo4_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO4_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO4_ONOFF_ECO_reg_ldo4_en_START (0)
#define PMIC_LDO4_ONOFF_ECO_reg_ldo4_en_END (0)
#define PMIC_LDO4_ONOFF_ECO_st_ldo4_en_START (1)
#define PMIC_LDO4_ONOFF_ECO_st_ldo4_en_END (1)
#define PMIC_LDO4_ONOFF_ECO_reg_ldo4_eco_en_START (4)
#define PMIC_LDO4_ONOFF_ECO_reg_ldo4_eco_en_END (4)
#define PMIC_LDO4_ONOFF_ECO_st_ldo4_eco_en_START (5)
#define PMIC_LDO4_ONOFF_ECO_st_ldo4_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo5_en : 1;
        unsigned char st_ldo5_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo5_eco_en : 1;
        unsigned char st_ldo5_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO5_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO5_ONOFF_ECO_reg_ldo5_en_START (0)
#define PMIC_LDO5_ONOFF_ECO_reg_ldo5_en_END (0)
#define PMIC_LDO5_ONOFF_ECO_st_ldo5_en_START (1)
#define PMIC_LDO5_ONOFF_ECO_st_ldo5_en_END (1)
#define PMIC_LDO5_ONOFF_ECO_reg_ldo5_eco_en_START (4)
#define PMIC_LDO5_ONOFF_ECO_reg_ldo5_eco_en_END (4)
#define PMIC_LDO5_ONOFF_ECO_st_ldo5_eco_en_START (5)
#define PMIC_LDO5_ONOFF_ECO_st_ldo5_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo6_en : 1;
        unsigned char st_ldo6_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo6_eco_en : 1;
        unsigned char st_ldo6_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO6_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO6_ONOFF_ECO_reg_ldo6_en_START (0)
#define PMIC_LDO6_ONOFF_ECO_reg_ldo6_en_END (0)
#define PMIC_LDO6_ONOFF_ECO_st_ldo6_en_START (1)
#define PMIC_LDO6_ONOFF_ECO_st_ldo6_en_END (1)
#define PMIC_LDO6_ONOFF_ECO_reg_ldo6_eco_en_START (4)
#define PMIC_LDO6_ONOFF_ECO_reg_ldo6_eco_en_END (4)
#define PMIC_LDO6_ONOFF_ECO_st_ldo6_eco_en_START (5)
#define PMIC_LDO6_ONOFF_ECO_st_ldo6_eco_en_END (5)
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
        unsigned char st_ldo8_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO8_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO8_ONOFF_ECO_reg_ldo8_en_START (0)
#define PMIC_LDO8_ONOFF_ECO_reg_ldo8_en_END (0)
#define PMIC_LDO8_ONOFF_ECO_st_ldo8_en_START (1)
#define PMIC_LDO8_ONOFF_ECO_st_ldo8_en_END (1)
#define PMIC_LDO8_ONOFF_ECO_reg_ldo8_eco_en_START (4)
#define PMIC_LDO8_ONOFF_ECO_reg_ldo8_eco_en_END (4)
#define PMIC_LDO8_ONOFF_ECO_st_ldo8_eco_en_START (5)
#define PMIC_LDO8_ONOFF_ECO_st_ldo8_eco_en_END (5)
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
        unsigned char st_ldo9_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO9_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO9_ONOFF_ECO_reg_ldo9_en_START (0)
#define PMIC_LDO9_ONOFF_ECO_reg_ldo9_en_END (0)
#define PMIC_LDO9_ONOFF_ECO_st_ldo9_en_START (1)
#define PMIC_LDO9_ONOFF_ECO_st_ldo9_en_END (1)
#define PMIC_LDO9_ONOFF_ECO_reg_ldo9_eco_en_START (4)
#define PMIC_LDO9_ONOFF_ECO_reg_ldo9_eco_en_END (4)
#define PMIC_LDO9_ONOFF_ECO_st_ldo9_eco_en_START (5)
#define PMIC_LDO9_ONOFF_ECO_st_ldo9_eco_en_END (5)
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
        unsigned char st_ldo11_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO11_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO11_ONOFF_ECO_reg_ldo11_en_START (0)
#define PMIC_LDO11_ONOFF_ECO_reg_ldo11_en_END (0)
#define PMIC_LDO11_ONOFF_ECO_st_ldo11_en_START (1)
#define PMIC_LDO11_ONOFF_ECO_st_ldo11_en_END (1)
#define PMIC_LDO11_ONOFF_ECO_reg_ldo11_eco_en_START (4)
#define PMIC_LDO11_ONOFF_ECO_reg_ldo11_eco_en_END (4)
#define PMIC_LDO11_ONOFF_ECO_st_ldo11_eco_en_START (5)
#define PMIC_LDO11_ONOFF_ECO_st_ldo11_eco_en_END (5)
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
        unsigned char st_ldo12_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO12_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO12_ONOFF_ECO_reg_ldo12_en_START (0)
#define PMIC_LDO12_ONOFF_ECO_reg_ldo12_en_END (0)
#define PMIC_LDO12_ONOFF_ECO_st_ldo12_en_START (1)
#define PMIC_LDO12_ONOFF_ECO_st_ldo12_en_END (1)
#define PMIC_LDO12_ONOFF_ECO_reg_ldo12_eco_en_START (4)
#define PMIC_LDO12_ONOFF_ECO_reg_ldo12_eco_en_END (4)
#define PMIC_LDO12_ONOFF_ECO_st_ldo12_eco_en_START (5)
#define PMIC_LDO12_ONOFF_ECO_st_ldo12_eco_en_END (5)
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
} PMIC_LDO13_ONOFF_UNION;
#endif
#define PMIC_LDO13_ONOFF_reg_ldo13_en_START (0)
#define PMIC_LDO13_ONOFF_reg_ldo13_en_END (0)
#define PMIC_LDO13_ONOFF_st_ldo13_en_START (1)
#define PMIC_LDO13_ONOFF_st_ldo13_en_END (1)
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
} PMIC_LDO14_ONOFF_UNION;
#endif
#define PMIC_LDO14_ONOFF_reg_ldo14_en_START (0)
#define PMIC_LDO14_ONOFF_reg_ldo14_en_END (0)
#define PMIC_LDO14_ONOFF_st_ldo14_en_START (1)
#define PMIC_LDO14_ONOFF_st_ldo14_en_END (1)
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
        unsigned char st_ldo15_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO15_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO15_ONOFF_ECO_reg_ldo15_en_START (0)
#define PMIC_LDO15_ONOFF_ECO_reg_ldo15_en_END (0)
#define PMIC_LDO15_ONOFF_ECO_st_ldo15_en_START (1)
#define PMIC_LDO15_ONOFF_ECO_st_ldo15_en_END (1)
#define PMIC_LDO15_ONOFF_ECO_reg_ldo15_eco_en_START (4)
#define PMIC_LDO15_ONOFF_ECO_reg_ldo15_eco_en_END (4)
#define PMIC_LDO15_ONOFF_ECO_st_ldo15_eco_en_START (5)
#define PMIC_LDO15_ONOFF_ECO_st_ldo15_eco_en_END (5)
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
        unsigned char st_ldo16_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO16_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO16_ONOFF_ECO_reg_ldo16_en_START (0)
#define PMIC_LDO16_ONOFF_ECO_reg_ldo16_en_END (0)
#define PMIC_LDO16_ONOFF_ECO_st_ldo16_en_START (1)
#define PMIC_LDO16_ONOFF_ECO_st_ldo16_en_END (1)
#define PMIC_LDO16_ONOFF_ECO_reg_ldo16_eco_en_START (4)
#define PMIC_LDO16_ONOFF_ECO_reg_ldo16_eco_en_END (4)
#define PMIC_LDO16_ONOFF_ECO_st_ldo16_eco_en_START (5)
#define PMIC_LDO16_ONOFF_ECO_st_ldo16_eco_en_END (5)
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
        unsigned char st_ldo17_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO17_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO17_ONOFF_ECO_reg_ldo17_en_START (0)
#define PMIC_LDO17_ONOFF_ECO_reg_ldo17_en_END (0)
#define PMIC_LDO17_ONOFF_ECO_st_ldo17_en_START (1)
#define PMIC_LDO17_ONOFF_ECO_st_ldo17_en_END (1)
#define PMIC_LDO17_ONOFF_ECO_reg_ldo17_eco_en_START (4)
#define PMIC_LDO17_ONOFF_ECO_reg_ldo17_eco_en_END (4)
#define PMIC_LDO17_ONOFF_ECO_st_ldo17_eco_en_START (5)
#define PMIC_LDO17_ONOFF_ECO_st_ldo17_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo18_en : 1;
        unsigned char st_ldo18_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo18_eco_en : 1;
        unsigned char st_ldo18_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO18_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO18_ONOFF_ECO_reg_ldo18_en_START (0)
#define PMIC_LDO18_ONOFF_ECO_reg_ldo18_en_END (0)
#define PMIC_LDO18_ONOFF_ECO_st_ldo18_en_START (1)
#define PMIC_LDO18_ONOFF_ECO_st_ldo18_en_END (1)
#define PMIC_LDO18_ONOFF_ECO_reg_ldo18_eco_en_START (4)
#define PMIC_LDO18_ONOFF_ECO_reg_ldo18_eco_en_END (4)
#define PMIC_LDO18_ONOFF_ECO_st_ldo18_eco_en_START (5)
#define PMIC_LDO18_ONOFF_ECO_st_ldo18_eco_en_END (5)
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
} PMIC_LDO19_ONOFF_UNION;
#endif
#define PMIC_LDO19_ONOFF_reg_ldo19_en_START (0)
#define PMIC_LDO19_ONOFF_reg_ldo19_en_END (0)
#define PMIC_LDO19_ONOFF_st_ldo19_en_START (1)
#define PMIC_LDO19_ONOFF_st_ldo19_en_END (1)
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
} PMIC_LDO20_ONOFF_UNION;
#endif
#define PMIC_LDO20_ONOFF_reg_ldo20_en_START (0)
#define PMIC_LDO20_ONOFF_reg_ldo20_en_END (0)
#define PMIC_LDO20_ONOFF_st_ldo20_en_START (1)
#define PMIC_LDO20_ONOFF_st_ldo20_en_END (1)
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
} PMIC_LDO21_ONOFF_UNION;
#endif
#define PMIC_LDO21_ONOFF_reg_ldo21_en_START (0)
#define PMIC_LDO21_ONOFF_reg_ldo21_en_END (0)
#define PMIC_LDO21_ONOFF_st_ldo21_en_START (1)
#define PMIC_LDO21_ONOFF_st_ldo21_en_END (1)
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
} PMIC_LDO22_ONOFF_UNION;
#endif
#define PMIC_LDO22_ONOFF_reg_ldo22_en_START (0)
#define PMIC_LDO22_ONOFF_reg_ldo22_en_END (0)
#define PMIC_LDO22_ONOFF_st_ldo22_en_START (1)
#define PMIC_LDO22_ONOFF_st_ldo22_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo23_en : 1;
        unsigned char st_ldo23_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo23_eco_en : 1;
        unsigned char st_ldo23_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO23_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO23_ONOFF_ECO_reg_ldo23_en_START (0)
#define PMIC_LDO23_ONOFF_ECO_reg_ldo23_en_END (0)
#define PMIC_LDO23_ONOFF_ECO_st_ldo23_en_START (1)
#define PMIC_LDO23_ONOFF_ECO_st_ldo23_en_END (1)
#define PMIC_LDO23_ONOFF_ECO_reg_ldo23_eco_en_START (4)
#define PMIC_LDO23_ONOFF_ECO_reg_ldo23_eco_en_END (4)
#define PMIC_LDO23_ONOFF_ECO_st_ldo23_eco_en_START (5)
#define PMIC_LDO23_ONOFF_ECO_st_ldo23_eco_en_END (5)
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
        unsigned char st_ldo24_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO24_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO24_ONOFF_ECO_reg_ldo24_en_START (0)
#define PMIC_LDO24_ONOFF_ECO_reg_ldo24_en_END (0)
#define PMIC_LDO24_ONOFF_ECO_st_ldo24_en_START (1)
#define PMIC_LDO24_ONOFF_ECO_st_ldo24_en_END (1)
#define PMIC_LDO24_ONOFF_ECO_reg_ldo24_eco_en_START (4)
#define PMIC_LDO24_ONOFF_ECO_reg_ldo24_eco_en_END (4)
#define PMIC_LDO24_ONOFF_ECO_st_ldo24_eco_en_START (5)
#define PMIC_LDO24_ONOFF_ECO_st_ldo24_eco_en_END (5)
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
} PMIC_LDO25_ONOFF_UNION;
#endif
#define PMIC_LDO25_ONOFF_reg_ldo25_en_START (0)
#define PMIC_LDO25_ONOFF_reg_ldo25_en_END (0)
#define PMIC_LDO25_ONOFF_st_ldo25_en_START (1)
#define PMIC_LDO25_ONOFF_st_ldo25_en_END (1)
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
        unsigned char st_ldo26_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO26_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO26_ONOFF_ECO_reg_ldo26_en_START (0)
#define PMIC_LDO26_ONOFF_ECO_reg_ldo26_en_END (0)
#define PMIC_LDO26_ONOFF_ECO_st_ldo26_en_START (1)
#define PMIC_LDO26_ONOFF_ECO_st_ldo26_en_END (1)
#define PMIC_LDO26_ONOFF_ECO_reg_ldo26_eco_en_START (4)
#define PMIC_LDO26_ONOFF_ECO_reg_ldo26_eco_en_END (4)
#define PMIC_LDO26_ONOFF_ECO_st_ldo26_eco_en_START (5)
#define PMIC_LDO26_ONOFF_ECO_st_ldo26_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo27_en : 1;
        unsigned char st_ldo27_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo27_eco_en : 1;
        unsigned char st_ldo27_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO27_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO27_ONOFF_ECO_reg_ldo27_en_START (0)
#define PMIC_LDO27_ONOFF_ECO_reg_ldo27_en_END (0)
#define PMIC_LDO27_ONOFF_ECO_st_ldo27_en_START (1)
#define PMIC_LDO27_ONOFF_ECO_st_ldo27_en_END (1)
#define PMIC_LDO27_ONOFF_ECO_reg_ldo27_eco_en_START (4)
#define PMIC_LDO27_ONOFF_ECO_reg_ldo27_eco_en_END (4)
#define PMIC_LDO27_ONOFF_ECO_st_ldo27_eco_en_START (5)
#define PMIC_LDO27_ONOFF_ECO_st_ldo27_eco_en_END (5)
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
} PMIC_LDO28_ONOFF_UNION;
#endif
#define PMIC_LDO28_ONOFF_reg_ldo28_en_START (0)
#define PMIC_LDO28_ONOFF_reg_ldo28_en_END (0)
#define PMIC_LDO28_ONOFF_st_ldo28_en_START (1)
#define PMIC_LDO28_ONOFF_st_ldo28_en_END (1)
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
        unsigned char st_ldo29_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO29_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO29_ONOFF_ECO_reg_ldo29_en_START (0)
#define PMIC_LDO29_ONOFF_ECO_reg_ldo29_en_END (0)
#define PMIC_LDO29_ONOFF_ECO_st_ldo29_en_START (1)
#define PMIC_LDO29_ONOFF_ECO_st_ldo29_en_END (1)
#define PMIC_LDO29_ONOFF_ECO_reg_ldo29_eco_en_START (4)
#define PMIC_LDO29_ONOFF_ECO_reg_ldo29_eco_en_END (4)
#define PMIC_LDO29_ONOFF_ECO_st_ldo29_eco_en_START (5)
#define PMIC_LDO29_ONOFF_ECO_st_ldo29_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo30_1_en : 1;
        unsigned char st_ldo30_1_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO30_1_ONOFF_UNION;
#endif
#define PMIC_LDO30_1_ONOFF_reg_ldo30_1_en_START (0)
#define PMIC_LDO30_1_ONOFF_reg_ldo30_1_en_END (0)
#define PMIC_LDO30_1_ONOFF_st_ldo30_1_en_START (1)
#define PMIC_LDO30_1_ONOFF_st_ldo30_1_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo30_2_en : 1;
        unsigned char st_ldo30_2_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo30_2_eco_en : 1;
        unsigned char st_ldo30_2_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO30_2_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO30_2_ONOFF_ECO_reg_ldo30_2_en_START (0)
#define PMIC_LDO30_2_ONOFF_ECO_reg_ldo30_2_en_END (0)
#define PMIC_LDO30_2_ONOFF_ECO_st_ldo30_2_en_START (1)
#define PMIC_LDO30_2_ONOFF_ECO_st_ldo30_2_en_END (1)
#define PMIC_LDO30_2_ONOFF_ECO_reg_ldo30_2_eco_en_START (4)
#define PMIC_LDO30_2_ONOFF_ECO_reg_ldo30_2_eco_en_END (4)
#define PMIC_LDO30_2_ONOFF_ECO_st_ldo30_2_eco_en_START (5)
#define PMIC_LDO30_2_ONOFF_ECO_st_ldo30_2_eco_en_END (5)
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
} PMIC_LDO31_ONOFF_UNION;
#endif
#define PMIC_LDO31_ONOFF_reg_ldo31_en_START (0)
#define PMIC_LDO31_ONOFF_reg_ldo31_en_END (0)
#define PMIC_LDO31_ONOFF_st_ldo31_en_START (1)
#define PMIC_LDO31_ONOFF_st_ldo31_en_END (1)
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
} PMIC_LDO32_ONOFF_UNION;
#endif
#define PMIC_LDO32_ONOFF_reg_ldo32_en_START (0)
#define PMIC_LDO32_ONOFF_reg_ldo32_en_END (0)
#define PMIC_LDO32_ONOFF_st_ldo32_en_START (1)
#define PMIC_LDO32_ONOFF_st_ldo32_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo33_en_a : 1;
        unsigned char st_ldo33_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO33_ONOFF1_UNION;
#endif
#define PMIC_LDO33_ONOFF1_reg_ldo33_en_a_START (0)
#define PMIC_LDO33_ONOFF1_reg_ldo33_en_a_END (0)
#define PMIC_LDO33_ONOFF1_st_ldo33_en_START (1)
#define PMIC_LDO33_ONOFF1_st_ldo33_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo33_en_b : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_LDO33_ONOFF2_UNION;
#endif
#define PMIC_LDO33_ONOFF2_reg_ldo33_en_b_START (0)
#define PMIC_LDO33_ONOFF2_reg_ldo33_en_b_END (0)
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
        unsigned char st_ldo34_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO34_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO34_ONOFF_ECO_reg_ldo34_en_START (0)
#define PMIC_LDO34_ONOFF_ECO_reg_ldo34_en_END (0)
#define PMIC_LDO34_ONOFF_ECO_st_ldo34_en_START (1)
#define PMIC_LDO34_ONOFF_ECO_st_ldo34_en_END (1)
#define PMIC_LDO34_ONOFF_ECO_reg_ldo34_eco_en_START (4)
#define PMIC_LDO34_ONOFF_ECO_reg_ldo34_eco_en_END (4)
#define PMIC_LDO34_ONOFF_ECO_st_ldo34_eco_en_START (5)
#define PMIC_LDO34_ONOFF_ECO_st_ldo34_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_pmuh_en : 1;
        unsigned char st_pmuh_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_pmuh_eco_en : 1;
        unsigned char st_pmuh_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_PMUH_ONOFF_ECO_UNION;
#endif
#define PMIC_PMUH_ONOFF_ECO_reg_pmuh_en_START (0)
#define PMIC_PMUH_ONOFF_ECO_reg_pmuh_en_END (0)
#define PMIC_PMUH_ONOFF_ECO_st_pmuh_en_START (1)
#define PMIC_PMUH_ONOFF_ECO_st_pmuh_en_END (1)
#define PMIC_PMUH_ONOFF_ECO_reg_pmuh_eco_en_START (4)
#define PMIC_PMUH_ONOFF_ECO_reg_pmuh_eco_en_END (4)
#define PMIC_PMUH_ONOFF_ECO_st_pmuh_eco_en_START (5)
#define PMIC_PMUH_ONOFF_ECO_st_pmuh_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo37_en : 1;
        unsigned char st_ldo37_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo37_eco_en : 1;
        unsigned char st_ldo37_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO37_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO37_ONOFF_ECO_reg_ldo37_en_START (0)
#define PMIC_LDO37_ONOFF_ECO_reg_ldo37_en_END (0)
#define PMIC_LDO37_ONOFF_ECO_st_ldo37_en_START (1)
#define PMIC_LDO37_ONOFF_ECO_st_ldo37_en_END (1)
#define PMIC_LDO37_ONOFF_ECO_reg_ldo37_eco_en_START (4)
#define PMIC_LDO37_ONOFF_ECO_reg_ldo37_eco_en_END (4)
#define PMIC_LDO37_ONOFF_ECO_st_ldo37_eco_en_START (5)
#define PMIC_LDO37_ONOFF_ECO_st_ldo37_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo38_en : 1;
        unsigned char st_ldo38_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo38_eco_en : 1;
        unsigned char st_ldo38_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO38_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO38_ONOFF_ECO_reg_ldo38_en_START (0)
#define PMIC_LDO38_ONOFF_ECO_reg_ldo38_en_END (0)
#define PMIC_LDO38_ONOFF_ECO_st_ldo38_en_START (1)
#define PMIC_LDO38_ONOFF_ECO_st_ldo38_en_END (1)
#define PMIC_LDO38_ONOFF_ECO_reg_ldo38_eco_en_START (4)
#define PMIC_LDO38_ONOFF_ECO_reg_ldo38_eco_en_END (4)
#define PMIC_LDO38_ONOFF_ECO_st_ldo38_eco_en_START (5)
#define PMIC_LDO38_ONOFF_ECO_st_ldo38_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo39_en : 1;
        unsigned char st_ldo39_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo39_eco_en : 1;
        unsigned char st_ldo39_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO39_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO39_ONOFF_ECO_reg_ldo39_en_START (0)
#define PMIC_LDO39_ONOFF_ECO_reg_ldo39_en_END (0)
#define PMIC_LDO39_ONOFF_ECO_st_ldo39_en_START (1)
#define PMIC_LDO39_ONOFF_ECO_st_ldo39_en_END (1)
#define PMIC_LDO39_ONOFF_ECO_reg_ldo39_eco_en_START (4)
#define PMIC_LDO39_ONOFF_ECO_reg_ldo39_eco_en_END (4)
#define PMIC_LDO39_ONOFF_ECO_st_ldo39_eco_en_START (5)
#define PMIC_LDO39_ONOFF_ECO_st_ldo39_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo40_en : 1;
        unsigned char st_ldo40_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO40_ONOFF_UNION;
#endif
#define PMIC_LDO40_ONOFF_reg_ldo40_en_START (0)
#define PMIC_LDO40_ONOFF_reg_ldo40_en_END (0)
#define PMIC_LDO40_ONOFF_st_ldo40_en_START (1)
#define PMIC_LDO40_ONOFF_st_ldo40_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char st_pmua_en : 1;
        unsigned char reserved : 3;
        unsigned char reg_pmua_eco_en : 1;
        unsigned char st_pmua_eco_en : 1;
        unsigned char reg_thsd_eco_en : 1;
        unsigned char st_thsd_eco_en : 1;
    } reg;
} PMIC_LDO_PMUA_ECO_UNION;
#endif
#define PMIC_LDO_PMUA_ECO_st_pmua_en_START (0)
#define PMIC_LDO_PMUA_ECO_st_pmua_en_END (0)
#define PMIC_LDO_PMUA_ECO_reg_pmua_eco_en_START (4)
#define PMIC_LDO_PMUA_ECO_reg_pmua_eco_en_END (4)
#define PMIC_LDO_PMUA_ECO_st_pmua_eco_en_START (5)
#define PMIC_LDO_PMUA_ECO_st_pmua_eco_en_END (5)
#define PMIC_LDO_PMUA_ECO_reg_thsd_eco_en_START (6)
#define PMIC_LDO_PMUA_ECO_reg_thsd_eco_en_END (6)
#define PMIC_LDO_PMUA_ECO_st_thsd_eco_en_START (7)
#define PMIC_LDO_PMUA_ECO_st_thsd_eco_en_END (7)
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
        unsigned char reg_xo_nfc_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_CLK_NFC_EN_UNION;
#endif
#define PMIC_CLK_NFC_EN_reg_xo_nfc_en_START (0)
#define PMIC_CLK_NFC_EN_reg_xo_nfc_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_rf_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_CLK_RF_EN_UNION;
#endif
#define PMIC_CLK_RF_EN_xo_rf_en_START (0)
#define PMIC_CLK_RF_EN_xo_rf_en_END (0)
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
        unsigned char reg_xo_ufs_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_CLK_UFS_EN_UNION;
#endif
#define PMIC_CLK_UFS_EN_reg_xo_ufs_en_START (0)
#define PMIC_CLK_UFS_EN_reg_xo_ufs_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char en_32k_gps : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_OSC32K_GPS_ONOFF_CTRL_UNION;
#endif
#define PMIC_OSC32K_GPS_ONOFF_CTRL_en_32k_gps_START (0)
#define PMIC_OSC32K_GPS_ONOFF_CTRL_en_32k_gps_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char en_32k_bt : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_OSC32K_BT_ONOFF_CTRL_UNION;
#endif
#define PMIC_OSC32K_BT_ONOFF_CTRL_en_32k_bt_START (0)
#define PMIC_OSC32K_BT_ONOFF_CTRL_en_32k_bt_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char en_32k_sys : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_OSC32K_SYS_ONOFF_CTRL_UNION;
#endif
#define PMIC_OSC32K_SYS_ONOFF_CTRL_en_32k_sys_START (0)
#define PMIC_OSC32K_SYS_ONOFF_CTRL_en_32k_sys_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_vset_cfg : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK0_VSET_UNION;
#endif
#define PMIC_BUCK0_VSET_buck0_vset_cfg_START (0)
#define PMIC_BUCK0_VSET_buck0_vset_cfg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_vset_eco : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK0_VSET_ECO_UNION;
#endif
#define PMIC_BUCK0_VSET_ECO_buck0_vset_eco_START (0)
#define PMIC_BUCK0_VSET_ECO_buck0_vset_eco_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_vset : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK1_VSET_UNION;
#endif
#define PMIC_BUCK1_VSET_buck1_vset_START (0)
#define PMIC_BUCK1_VSET_buck1_vset_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_vset_cfg : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK2_VSET_UNION;
#endif
#define PMIC_BUCK2_VSET_buck2_vset_cfg_START (0)
#define PMIC_BUCK2_VSET_buck2_vset_cfg_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_vset_eco : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK2_VSET_ECO_UNION;
#endif
#define PMIC_BUCK2_VSET_ECO_buck2_vset_eco_START (0)
#define PMIC_BUCK2_VSET_ECO_buck2_vset_eco_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_vset_cfg : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK3_VSET_UNION;
#endif
#define PMIC_BUCK3_VSET_buck3_vset_cfg_START (0)
#define PMIC_BUCK3_VSET_buck3_vset_cfg_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_vset_eco : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK3_VSET_ECO_UNION;
#endif
#define PMIC_BUCK3_VSET_ECO_buck3_vset_eco_START (0)
#define PMIC_BUCK3_VSET_ECO_buck3_vset_eco_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_vset_cfg : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK4_VSET_UNION;
#endif
#define PMIC_BUCK4_VSET_buck4_vset_cfg_START (0)
#define PMIC_BUCK4_VSET_buck4_vset_cfg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_vset_eco : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK4_VSET_ECO_UNION;
#endif
#define PMIC_BUCK4_VSET_ECO_buck4_vset_eco_START (0)
#define PMIC_BUCK4_VSET_ECO_buck4_vset_eco_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_vset_idle : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK4_VSET_IDLE_UNION;
#endif
#define PMIC_BUCK4_VSET_IDLE_buck4_vset_idle_START (0)
#define PMIC_BUCK4_VSET_IDLE_buck4_vset_idle_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_vset_cfg : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK5_VSET_UNION;
#endif
#define PMIC_BUCK5_VSET_buck5_vset_cfg_START (0)
#define PMIC_BUCK5_VSET_buck5_vset_cfg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo0_2_vset_cfg : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO0_2_VSET_UNION;
#endif
#define PMIC_LDO0_2_VSET_ldo0_2_vset_cfg_START (0)
#define PMIC_LDO0_2_VSET_ldo0_2_vset_cfg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo0_2_vset_eco : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO0_2_VSET_ECO_UNION;
#endif
#define PMIC_LDO0_2_VSET_ECO_ldo0_2_vset_eco_START (0)
#define PMIC_LDO0_2_VSET_ECO_ldo0_2_vset_eco_END (5)
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
        unsigned char ldo2_vset : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO2_VSET_UNION;
#endif
#define PMIC_LDO2_VSET_ldo2_vset_START (0)
#define PMIC_LDO2_VSET_ldo2_vset_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo2_vset_eco : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO2_VSET_ECO_UNION;
#endif
#define PMIC_LDO2_VSET_ECO_ldo2_vset_eco_START (0)
#define PMIC_LDO2_VSET_ECO_ldo2_vset_eco_END (4)
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
        unsigned char ldo6_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO6_VSET_UNION;
#endif
#define PMIC_LDO6_VSET_ldo6_vset_START (0)
#define PMIC_LDO6_VSET_ldo6_vset_END (3)
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
        unsigned char ldo9_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO9_VSET_UNION;
#endif
#define PMIC_LDO9_VSET_ldo9_vset_START (0)
#define PMIC_LDO9_VSET_ldo9_vset_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo11_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO11_VSET_UNION;
#endif
#define PMIC_LDO11_VSET_ldo11_vset_START (0)
#define PMIC_LDO11_VSET_ldo11_vset_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo12_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO12_VSET_UNION;
#endif
#define PMIC_LDO12_VSET_ldo12_vset_START (0)
#define PMIC_LDO12_VSET_ldo12_vset_END (3)
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
        unsigned char ldo18_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO18_VSET_UNION;
#endif
#define PMIC_LDO18_VSET_ldo18_vset_START (0)
#define PMIC_LDO18_VSET_ldo18_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo19_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO19_VSET_UNION;
#endif
#define PMIC_LDO19_VSET_ldo19_vset_START (0)
#define PMIC_LDO19_VSET_ldo19_vset_END (2)
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
        unsigned char ldo22_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO22_VSET_UNION;
#endif
#define PMIC_LDO22_VSET_ldo22_vset_START (0)
#define PMIC_LDO22_VSET_ldo22_vset_END (2)
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
        unsigned char ldo27_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO27_VSET_UNION;
#endif
#define PMIC_LDO27_VSET_ldo27_vset_START (0)
#define PMIC_LDO27_VSET_ldo27_vset_END (2)
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
        unsigned char ldo29_vset : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO29_VSET_UNION;
#endif
#define PMIC_LDO29_VSET_ldo29_vset_START (0)
#define PMIC_LDO29_VSET_ldo29_vset_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo30_2_vset_cfg : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO30_VSET_UNION;
#endif
#define PMIC_LDO30_VSET_ldo30_2_vset_cfg_START (0)
#define PMIC_LDO30_VSET_ldo30_2_vset_cfg_END (2)
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
        unsigned char pmuh_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_PMUH_VSET_UNION;
#endif
#define PMIC_PMUH_VSET_pmuh_vset_START (0)
#define PMIC_PMUH_VSET_pmuh_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo37_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO37_VSET_UNION;
#endif
#define PMIC_LDO37_VSET_ldo37_vset_START (0)
#define PMIC_LDO37_VSET_ldo37_vset_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo38_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO38_VSET_UNION;
#endif
#define PMIC_LDO38_VSET_ldo38_vset_START (0)
#define PMIC_LDO38_VSET_ldo38_vset_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo39_vset : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO39_VSET_UNION;
#endif
#define PMIC_LDO39_VSET_ldo39_vset_START (0)
#define PMIC_LDO39_VSET_ldo39_vset_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo40_vset_cfg : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO40_VSET_UNION;
#endif
#define PMIC_LDO40_VSET_ldo40_vset_cfg_START (0)
#define PMIC_LDO40_VSET_ldo40_vset_cfg_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo_buff_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO_BUF_VSET_UNION;
#endif
#define PMIC_LDO_BUF_VSET_ldo_buff_vset_START (0)
#define PMIC_LDO_BUF_VSET_ldo_buff_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pmua_vset : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO_PMUA_VSET_UNION;
#endif
#define PMIC_LDO_PMUA_VSET_pmua_vset_START (0)
#define PMIC_LDO_PMUA_VSET_pmua_vset_END (1)
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
        unsigned char buck0_dt_sel : 3;
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
#define PMIC_BUCK0_CTRL1_buck0_dt_sel_START (3)
#define PMIC_BUCK0_CTRL1_buck0_dt_sel_END (5)
#define PMIC_BUCK0_CTRL1_buck0_ocp_sel_START (6)
#define PMIC_BUCK0_CTRL1_buck0_ocp_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_pg_n_sel : 3;
        unsigned char buck0_pg_p_sel : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK0_CTRL2_UNION;
#endif
#define PMIC_BUCK0_CTRL2_buck0_pg_n_sel_START (0)
#define PMIC_BUCK0_CTRL2_buck0_pg_n_sel_END (2)
#define PMIC_BUCK0_CTRL2_buck0_pg_p_sel_START (3)
#define PMIC_BUCK0_CTRL2_buck0_pg_p_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_ng_n_sel : 3;
        unsigned char buck0_ng_p_sel : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK0_CTRL3_UNION;
#endif
#define PMIC_BUCK0_CTRL3_buck0_ng_n_sel_START (0)
#define PMIC_BUCK0_CTRL3_buck0_ng_n_sel_END (2)
#define PMIC_BUCK0_CTRL3_buck0_ng_p_sel_START (3)
#define PMIC_BUCK0_CTRL3_buck0_ng_p_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_reg_r : 2;
        unsigned char buck0_filter_ton : 2;
        unsigned char buck0_short_pdp : 1;
        unsigned char buck0_adj_clx : 2;
        unsigned char buck0_ocp_dis : 1;
    } reg;
} PMIC_BUCK0_CTRL4_UNION;
#endif
#define PMIC_BUCK0_CTRL4_buck0_reg_r_START (0)
#define PMIC_BUCK0_CTRL4_buck0_reg_r_END (1)
#define PMIC_BUCK0_CTRL4_buck0_filter_ton_START (2)
#define PMIC_BUCK0_CTRL4_buck0_filter_ton_END (3)
#define PMIC_BUCK0_CTRL4_buck0_short_pdp_START (4)
#define PMIC_BUCK0_CTRL4_buck0_short_pdp_END (4)
#define PMIC_BUCK0_CTRL4_buck0_adj_clx_START (5)
#define PMIC_BUCK0_CTRL4_buck0_adj_clx_END (6)
#define PMIC_BUCK0_CTRL4_buck0_ocp_dis_START (7)
#define PMIC_BUCK0_CTRL4_buck0_ocp_dis_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_ton : 3;
        unsigned char buck0_reg_bias : 1;
        unsigned char buck0_regop_c : 1;
        unsigned char buck0_reg_dr : 3;
    } reg;
} PMIC_BUCK0_CTRL5_UNION;
#endif
#define PMIC_BUCK0_CTRL5_buck0_ton_START (0)
#define PMIC_BUCK0_CTRL5_buck0_ton_END (2)
#define PMIC_BUCK0_CTRL5_buck0_reg_bias_START (3)
#define PMIC_BUCK0_CTRL5_buck0_reg_bias_END (3)
#define PMIC_BUCK0_CTRL5_buck0_regop_c_START (4)
#define PMIC_BUCK0_CTRL5_buck0_regop_c_END (4)
#define PMIC_BUCK0_CTRL5_buck0_reg_dr_START (5)
#define PMIC_BUCK0_CTRL5_buck0_reg_dr_END (7)
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
        unsigned char buck0_dmd_clamp : 1;
        unsigned char buck0_bias_dmd_sel : 1;
        unsigned char buck0_regop_clamp : 1;
        unsigned char buck0_ton_dmd : 1;
        unsigned char buck0_ocp_delay : 1;
        unsigned char buck0_lx_dt : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK0_CTRL7_UNION;
#endif
#define PMIC_BUCK0_CTRL7_buck0_dmd_clamp_START (0)
#define PMIC_BUCK0_CTRL7_buck0_dmd_clamp_END (0)
#define PMIC_BUCK0_CTRL7_buck0_bias_dmd_sel_START (1)
#define PMIC_BUCK0_CTRL7_buck0_bias_dmd_sel_END (1)
#define PMIC_BUCK0_CTRL7_buck0_regop_clamp_START (2)
#define PMIC_BUCK0_CTRL7_buck0_regop_clamp_END (2)
#define PMIC_BUCK0_CTRL7_buck0_ton_dmd_START (3)
#define PMIC_BUCK0_CTRL7_buck0_ton_dmd_END (3)
#define PMIC_BUCK0_CTRL7_buck0_ocp_delay_START (4)
#define PMIC_BUCK0_CTRL7_buck0_ocp_delay_END (4)
#define PMIC_BUCK0_CTRL7_buck0_lx_dt_START (5)
#define PMIC_BUCK0_CTRL7_buck0_lx_dt_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_dmd_cmp_sel : 1;
        unsigned char buck0_drv_delay : 1;
        unsigned char buck0_pow_drv_ctrl : 1;
        unsigned char buck0_autoton_ctrl : 1;
        unsigned char buck0_eco_dmd : 1;
        unsigned char buck0_cmp_filter : 1;
        unsigned char buck0_fix_ton : 2;
    } reg;
} PMIC_BUCK0_CTRL8_UNION;
#endif
#define PMIC_BUCK0_CTRL8_buck0_dmd_cmp_sel_START (0)
#define PMIC_BUCK0_CTRL8_buck0_dmd_cmp_sel_END (0)
#define PMIC_BUCK0_CTRL8_buck0_drv_delay_START (1)
#define PMIC_BUCK0_CTRL8_buck0_drv_delay_END (1)
#define PMIC_BUCK0_CTRL8_buck0_pow_drv_ctrl_START (2)
#define PMIC_BUCK0_CTRL8_buck0_pow_drv_ctrl_END (2)
#define PMIC_BUCK0_CTRL8_buck0_autoton_ctrl_START (3)
#define PMIC_BUCK0_CTRL8_buck0_autoton_ctrl_END (3)
#define PMIC_BUCK0_CTRL8_buck0_eco_dmd_START (4)
#define PMIC_BUCK0_CTRL8_buck0_eco_dmd_END (4)
#define PMIC_BUCK0_CTRL8_buck0_cmp_filter_START (5)
#define PMIC_BUCK0_CTRL8_buck0_cmp_filter_END (5)
#define PMIC_BUCK0_CTRL8_buck0_fix_ton_START (6)
#define PMIC_BUCK0_CTRL8_buck0_fix_ton_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_rampconfig : 3;
        unsigned char buck0_reg_zero_en : 1;
        unsigned char buck0_rampdown_delay : 2;
        unsigned char buck0_ramp_cap : 2;
    } reg;
} PMIC_BUCK0_CTRL9_UNION;
#endif
#define PMIC_BUCK0_CTRL9_buck0_rampconfig_START (0)
#define PMIC_BUCK0_CTRL9_buck0_rampconfig_END (2)
#define PMIC_BUCK0_CTRL9_buck0_reg_zero_en_START (3)
#define PMIC_BUCK0_CTRL9_buck0_reg_zero_en_END (3)
#define PMIC_BUCK0_CTRL9_buck0_rampdown_delay_START (4)
#define PMIC_BUCK0_CTRL9_buck0_rampdown_delay_END (5)
#define PMIC_BUCK0_CTRL9_buck0_ramp_cap_START (6)
#define PMIC_BUCK0_CTRL9_buck0_ramp_cap_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_autoton_sel : 2;
        unsigned char buck0_ramp_eco : 1;
        unsigned char buck0_adj_rlx_ramp : 4;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK0_CTRL10_UNION;
#endif
#define PMIC_BUCK0_CTRL10_buck0_autoton_sel_START (0)
#define PMIC_BUCK0_CTRL10_buck0_autoton_sel_END (1)
#define PMIC_BUCK0_CTRL10_buck0_ramp_eco_START (2)
#define PMIC_BUCK0_CTRL10_buck0_ramp_eco_END (2)
#define PMIC_BUCK0_CTRL10_buck0_adj_rlx_ramp_START (3)
#define PMIC_BUCK0_CTRL10_buck0_adj_rlx_ramp_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_mul_func : 8;
    } reg;
} PMIC_BUCK0_CTRL11_UNION;
#endif
#define PMIC_BUCK0_CTRL11_buck0_mul_func_START (0)
#define PMIC_BUCK0_CTRL11_buck0_mul_func_END (7)
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
        unsigned char buck1_dt_sel : 3;
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
#define PMIC_BUCK1_CTRL1_buck1_dt_sel_START (3)
#define PMIC_BUCK1_CTRL1_buck1_dt_sel_END (5)
#define PMIC_BUCK1_CTRL1_buck1_ocp_sel_START (6)
#define PMIC_BUCK1_CTRL1_buck1_ocp_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_pg_n_sel : 3;
        unsigned char buck1_pg_p_sel : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK1_CTRL2_UNION;
#endif
#define PMIC_BUCK1_CTRL2_buck1_pg_n_sel_START (0)
#define PMIC_BUCK1_CTRL2_buck1_pg_n_sel_END (2)
#define PMIC_BUCK1_CTRL2_buck1_pg_p_sel_START (3)
#define PMIC_BUCK1_CTRL2_buck1_pg_p_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_ng_n_sel : 3;
        unsigned char buck1_ng_p_sel : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK1_CTRL3_UNION;
#endif
#define PMIC_BUCK1_CTRL3_buck1_ng_n_sel_START (0)
#define PMIC_BUCK1_CTRL3_buck1_ng_n_sel_END (2)
#define PMIC_BUCK1_CTRL3_buck1_ng_p_sel_START (3)
#define PMIC_BUCK1_CTRL3_buck1_ng_p_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_reg_r : 2;
        unsigned char buck1_adj_clx : 2;
        unsigned char buck1_ocp_dis : 1;
        unsigned char buck1_filter_ton : 2;
        unsigned char buck1_short_pdp : 1;
    } reg;
} PMIC_BUCK1_CTRL4_UNION;
#endif
#define PMIC_BUCK1_CTRL4_buck1_reg_r_START (0)
#define PMIC_BUCK1_CTRL4_buck1_reg_r_END (1)
#define PMIC_BUCK1_CTRL4_buck1_adj_clx_START (2)
#define PMIC_BUCK1_CTRL4_buck1_adj_clx_END (3)
#define PMIC_BUCK1_CTRL4_buck1_ocp_dis_START (4)
#define PMIC_BUCK1_CTRL4_buck1_ocp_dis_END (4)
#define PMIC_BUCK1_CTRL4_buck1_filter_ton_START (5)
#define PMIC_BUCK1_CTRL4_buck1_filter_ton_END (6)
#define PMIC_BUCK1_CTRL4_buck1_short_pdp_START (7)
#define PMIC_BUCK1_CTRL4_buck1_short_pdp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_ton : 3;
        unsigned char buck1_reg_bias : 1;
        unsigned char buck1_regop_c : 1;
        unsigned char buck1_reg_dr : 3;
    } reg;
} PMIC_BUCK1_CTRL5_UNION;
#endif
#define PMIC_BUCK1_CTRL5_buck1_ton_START (0)
#define PMIC_BUCK1_CTRL5_buck1_ton_END (2)
#define PMIC_BUCK1_CTRL5_buck1_reg_bias_START (3)
#define PMIC_BUCK1_CTRL5_buck1_reg_bias_END (3)
#define PMIC_BUCK1_CTRL5_buck1_regop_c_START (4)
#define PMIC_BUCK1_CTRL5_buck1_regop_c_END (4)
#define PMIC_BUCK1_CTRL5_buck1_reg_dr_START (5)
#define PMIC_BUCK1_CTRL5_buck1_reg_dr_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_mul_func : 8;
    } reg;
} PMIC_BUCK1_CTRL6_UNION;
#endif
#define PMIC_BUCK1_CTRL6_buck1_mul_func_START (0)
#define PMIC_BUCK1_CTRL6_buck1_mul_func_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_autoton_ctrl : 1;
        unsigned char buck1_ton_dmd : 1;
        unsigned char buck1_eco_dmd : 1;
        unsigned char buck1_cmp_filter : 1;
        unsigned char buck1_ocp_delay : 1;
        unsigned char buck1_bias_dmd_sel : 1;
        unsigned char buck1_dmd_clamp : 1;
        unsigned char buck1_regop_clamp : 1;
    } reg;
} PMIC_BUCK1_CTRL7_UNION;
#endif
#define PMIC_BUCK1_CTRL7_buck1_autoton_ctrl_START (0)
#define PMIC_BUCK1_CTRL7_buck1_autoton_ctrl_END (0)
#define PMIC_BUCK1_CTRL7_buck1_ton_dmd_START (1)
#define PMIC_BUCK1_CTRL7_buck1_ton_dmd_END (1)
#define PMIC_BUCK1_CTRL7_buck1_eco_dmd_START (2)
#define PMIC_BUCK1_CTRL7_buck1_eco_dmd_END (2)
#define PMIC_BUCK1_CTRL7_buck1_cmp_filter_START (3)
#define PMIC_BUCK1_CTRL7_buck1_cmp_filter_END (3)
#define PMIC_BUCK1_CTRL7_buck1_ocp_delay_START (4)
#define PMIC_BUCK1_CTRL7_buck1_ocp_delay_END (4)
#define PMIC_BUCK1_CTRL7_buck1_bias_dmd_sel_START (5)
#define PMIC_BUCK1_CTRL7_buck1_bias_dmd_sel_END (5)
#define PMIC_BUCK1_CTRL7_buck1_dmd_clamp_START (6)
#define PMIC_BUCK1_CTRL7_buck1_dmd_clamp_END (6)
#define PMIC_BUCK1_CTRL7_buck1_regop_clamp_START (7)
#define PMIC_BUCK1_CTRL7_buck1_regop_clamp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_drv_delay : 1;
        unsigned char buck1_pow_drv_ctrl : 1;
        unsigned char buck1_autoton_sel : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_BUCK1_CTRL8_UNION;
#endif
#define PMIC_BUCK1_CTRL8_buck1_drv_delay_START (0)
#define PMIC_BUCK1_CTRL8_buck1_drv_delay_END (0)
#define PMIC_BUCK1_CTRL8_buck1_pow_drv_ctrl_START (1)
#define PMIC_BUCK1_CTRL8_buck1_pow_drv_ctrl_END (1)
#define PMIC_BUCK1_CTRL8_buck1_autoton_sel_START (2)
#define PMIC_BUCK1_CTRL8_buck1_autoton_sel_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_lx_dt : 2;
        unsigned char buck1_fix_ton : 2;
        unsigned char buck1_dmd_cmp_sel : 1;
        unsigned char buck1_dmd_sel : 3;
    } reg;
} PMIC_BUCK1_CTRL9_UNION;
#endif
#define PMIC_BUCK1_CTRL9_buck1_lx_dt_START (0)
#define PMIC_BUCK1_CTRL9_buck1_lx_dt_END (1)
#define PMIC_BUCK1_CTRL9_buck1_fix_ton_START (2)
#define PMIC_BUCK1_CTRL9_buck1_fix_ton_END (3)
#define PMIC_BUCK1_CTRL9_buck1_dmd_cmp_sel_START (4)
#define PMIC_BUCK1_CTRL9_buck1_dmd_cmp_sel_END (4)
#define PMIC_BUCK1_CTRL9_buck1_dmd_sel_START (5)
#define PMIC_BUCK1_CTRL9_buck1_dmd_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_dmd_ton : 3;
        unsigned char buck1_ocp_toff : 2;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK1_CTRL10_UNION;
#endif
#define PMIC_BUCK1_CTRL10_buck1_dmd_ton_START (0)
#define PMIC_BUCK1_CTRL10_buck1_dmd_ton_END (2)
#define PMIC_BUCK1_CTRL10_buck1_ocp_toff_START (3)
#define PMIC_BUCK1_CTRL10_buck1_ocp_toff_END (4)
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
        unsigned char buck2_ocp_dis : 1;
        unsigned char buck2_dt_sel : 3;
        unsigned char buck2_ocp_sel : 2;
    } reg;
} PMIC_BUCK2_CTRL1_UNION;
#endif
#define PMIC_BUCK2_CTRL1_buck2_ng_dt_sel_START (0)
#define PMIC_BUCK2_CTRL1_buck2_ng_dt_sel_END (0)
#define PMIC_BUCK2_CTRL1_buck2_pg_dt_sel_START (1)
#define PMIC_BUCK2_CTRL1_buck2_pg_dt_sel_END (1)
#define PMIC_BUCK2_CTRL1_buck2_ocp_dis_START (2)
#define PMIC_BUCK2_CTRL1_buck2_ocp_dis_END (2)
#define PMIC_BUCK2_CTRL1_buck2_dt_sel_START (3)
#define PMIC_BUCK2_CTRL1_buck2_dt_sel_END (5)
#define PMIC_BUCK2_CTRL1_buck2_ocp_sel_START (6)
#define PMIC_BUCK2_CTRL1_buck2_ocp_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_pg_n_sel : 3;
        unsigned char buck2_pg_p_sel : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK2_CTRL2_UNION;
#endif
#define PMIC_BUCK2_CTRL2_buck2_pg_n_sel_START (0)
#define PMIC_BUCK2_CTRL2_buck2_pg_n_sel_END (2)
#define PMIC_BUCK2_CTRL2_buck2_pg_p_sel_START (3)
#define PMIC_BUCK2_CTRL2_buck2_pg_p_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_ng_n_sel : 3;
        unsigned char buck2_ng_p_sel : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK2_CTRL3_UNION;
#endif
#define PMIC_BUCK2_CTRL3_buck2_ng_n_sel_START (0)
#define PMIC_BUCK2_CTRL3_buck2_ng_n_sel_END (2)
#define PMIC_BUCK2_CTRL3_buck2_ng_p_sel_START (3)
#define PMIC_BUCK2_CTRL3_buck2_ng_p_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_reg_r : 2;
        unsigned char buck2_nmos_switch : 1;
        unsigned char buck2_regop_clamp : 1;
        unsigned char buck2_dmd_clamp : 1;
        unsigned char buck2_adj_clx : 2;
        unsigned char buck2_short_pdp : 1;
    } reg;
} PMIC_BUCK2_CTRL4_UNION;
#endif
#define PMIC_BUCK2_CTRL4_buck2_reg_r_START (0)
#define PMIC_BUCK2_CTRL4_buck2_reg_r_END (1)
#define PMIC_BUCK2_CTRL4_buck2_nmos_switch_START (2)
#define PMIC_BUCK2_CTRL4_buck2_nmos_switch_END (2)
#define PMIC_BUCK2_CTRL4_buck2_regop_clamp_START (3)
#define PMIC_BUCK2_CTRL4_buck2_regop_clamp_END (3)
#define PMIC_BUCK2_CTRL4_buck2_dmd_clamp_START (4)
#define PMIC_BUCK2_CTRL4_buck2_dmd_clamp_END (4)
#define PMIC_BUCK2_CTRL4_buck2_adj_clx_START (5)
#define PMIC_BUCK2_CTRL4_buck2_adj_clx_END (6)
#define PMIC_BUCK2_CTRL4_buck2_short_pdp_START (7)
#define PMIC_BUCK2_CTRL4_buck2_short_pdp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_ton : 3;
        unsigned char buck2_reg_bias : 1;
        unsigned char buck2_regop_c : 1;
        unsigned char buck2_reg_dr : 3;
    } reg;
} PMIC_BUCK2_CTRL5_UNION;
#endif
#define PMIC_BUCK2_CTRL5_buck2_ton_START (0)
#define PMIC_BUCK2_CTRL5_buck2_ton_END (2)
#define PMIC_BUCK2_CTRL5_buck2_reg_bias_START (3)
#define PMIC_BUCK2_CTRL5_buck2_reg_bias_END (3)
#define PMIC_BUCK2_CTRL5_buck2_regop_c_START (4)
#define PMIC_BUCK2_CTRL5_buck2_regop_c_END (4)
#define PMIC_BUCK2_CTRL5_buck2_reg_dr_START (5)
#define PMIC_BUCK2_CTRL5_buck2_reg_dr_END (7)
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
        unsigned char buck2_drv_delay : 1;
        unsigned char buck2_bias_dmd_sel : 1;
        unsigned char buck2_pow_drv_ctrl : 1;
        unsigned char buck2_ton_dmd : 1;
        unsigned char buck2_ocp_delay : 1;
        unsigned char buck2_lx_dt : 2;
        unsigned char buck2_cmp_filter : 1;
    } reg;
} PMIC_BUCK2_CTRL7_UNION;
#endif
#define PMIC_BUCK2_CTRL7_buck2_drv_delay_START (0)
#define PMIC_BUCK2_CTRL7_buck2_drv_delay_END (0)
#define PMIC_BUCK2_CTRL7_buck2_bias_dmd_sel_START (1)
#define PMIC_BUCK2_CTRL7_buck2_bias_dmd_sel_END (1)
#define PMIC_BUCK2_CTRL7_buck2_pow_drv_ctrl_START (2)
#define PMIC_BUCK2_CTRL7_buck2_pow_drv_ctrl_END (2)
#define PMIC_BUCK2_CTRL7_buck2_ton_dmd_START (3)
#define PMIC_BUCK2_CTRL7_buck2_ton_dmd_END (3)
#define PMIC_BUCK2_CTRL7_buck2_ocp_delay_START (4)
#define PMIC_BUCK2_CTRL7_buck2_ocp_delay_END (4)
#define PMIC_BUCK2_CTRL7_buck2_lx_dt_START (5)
#define PMIC_BUCK2_CTRL7_buck2_lx_dt_END (6)
#define PMIC_BUCK2_CTRL7_buck2_cmp_filter_START (7)
#define PMIC_BUCK2_CTRL7_buck2_cmp_filter_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_fix_fq : 5;
        unsigned char buck2_fix_fq_ctrl : 1;
        unsigned char buck2_fix_ton : 2;
    } reg;
} PMIC_BUCK2_CTRL8_UNION;
#endif
#define PMIC_BUCK2_CTRL8_buck2_fix_fq_START (0)
#define PMIC_BUCK2_CTRL8_buck2_fix_fq_END (4)
#define PMIC_BUCK2_CTRL8_buck2_fix_fq_ctrl_START (5)
#define PMIC_BUCK2_CTRL8_buck2_fix_fq_ctrl_END (5)
#define PMIC_BUCK2_CTRL8_buck2_fix_ton_START (6)
#define PMIC_BUCK2_CTRL8_buck2_fix_ton_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_mul_func : 8;
    } reg;
} PMIC_BUCK2_CTRL9_UNION;
#endif
#define PMIC_BUCK2_CTRL9_buck2_mul_func_START (0)
#define PMIC_BUCK2_CTRL9_buck2_mul_func_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_autoton_sel : 2;
        unsigned char buck2_autoton_ctrl : 1;
        unsigned char buck2_eco_dmd : 1;
        unsigned char buck2_filter_ton : 2;
        unsigned char buck2_dmd_cmp_sel : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK2_CTRL10_UNION;
#endif
#define PMIC_BUCK2_CTRL10_buck2_autoton_sel_START (0)
#define PMIC_BUCK2_CTRL10_buck2_autoton_sel_END (1)
#define PMIC_BUCK2_CTRL10_buck2_autoton_ctrl_START (2)
#define PMIC_BUCK2_CTRL10_buck2_autoton_ctrl_END (2)
#define PMIC_BUCK2_CTRL10_buck2_eco_dmd_START (3)
#define PMIC_BUCK2_CTRL10_buck2_eco_dmd_END (3)
#define PMIC_BUCK2_CTRL10_buck2_filter_ton_START (4)
#define PMIC_BUCK2_CTRL10_buck2_filter_ton_END (5)
#define PMIC_BUCK2_CTRL10_buck2_dmd_cmp_sel_START (6)
#define PMIC_BUCK2_CTRL10_buck2_dmd_cmp_sel_END (6)
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
        unsigned char buck3_ocp_dis : 1;
        unsigned char buck3_dt_sel : 3;
        unsigned char buck3_ocp_sel : 2;
    } reg;
} PMIC_BUCK3_CTRL1_UNION;
#endif
#define PMIC_BUCK3_CTRL1_buck3_ng_dt_sel_START (0)
#define PMIC_BUCK3_CTRL1_buck3_ng_dt_sel_END (0)
#define PMIC_BUCK3_CTRL1_buck3_pg_dt_sel_START (1)
#define PMIC_BUCK3_CTRL1_buck3_pg_dt_sel_END (1)
#define PMIC_BUCK3_CTRL1_buck3_ocp_dis_START (2)
#define PMIC_BUCK3_CTRL1_buck3_ocp_dis_END (2)
#define PMIC_BUCK3_CTRL1_buck3_dt_sel_START (3)
#define PMIC_BUCK3_CTRL1_buck3_dt_sel_END (5)
#define PMIC_BUCK3_CTRL1_buck3_ocp_sel_START (6)
#define PMIC_BUCK3_CTRL1_buck3_ocp_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_pg_n_sel : 3;
        unsigned char buck3_pg_p_sel : 3;
        unsigned char buck3_nmos_switch : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK3_CTRL2_UNION;
#endif
#define PMIC_BUCK3_CTRL2_buck3_pg_n_sel_START (0)
#define PMIC_BUCK3_CTRL2_buck3_pg_n_sel_END (2)
#define PMIC_BUCK3_CTRL2_buck3_pg_p_sel_START (3)
#define PMIC_BUCK3_CTRL2_buck3_pg_p_sel_END (5)
#define PMIC_BUCK3_CTRL2_buck3_nmos_switch_START (6)
#define PMIC_BUCK3_CTRL2_buck3_nmos_switch_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_ng_n_sel : 3;
        unsigned char buck3_ng_p_sel : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK3_CTRL3_UNION;
#endif
#define PMIC_BUCK3_CTRL3_buck3_ng_n_sel_START (0)
#define PMIC_BUCK3_CTRL3_buck3_ng_n_sel_END (2)
#define PMIC_BUCK3_CTRL3_buck3_ng_p_sel_START (3)
#define PMIC_BUCK3_CTRL3_buck3_ng_p_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_reg_r : 2;
        unsigned char buck3_filter_ton : 2;
        unsigned char buck3_short_pdp : 1;
        unsigned char buck3_adj_clx : 2;
        unsigned char buck3_eco_dmd : 1;
    } reg;
} PMIC_BUCK3_CTRL4_UNION;
#endif
#define PMIC_BUCK3_CTRL4_buck3_reg_r_START (0)
#define PMIC_BUCK3_CTRL4_buck3_reg_r_END (1)
#define PMIC_BUCK3_CTRL4_buck3_filter_ton_START (2)
#define PMIC_BUCK3_CTRL4_buck3_filter_ton_END (3)
#define PMIC_BUCK3_CTRL4_buck3_short_pdp_START (4)
#define PMIC_BUCK3_CTRL4_buck3_short_pdp_END (4)
#define PMIC_BUCK3_CTRL4_buck3_adj_clx_START (5)
#define PMIC_BUCK3_CTRL4_buck3_adj_clx_END (6)
#define PMIC_BUCK3_CTRL4_buck3_eco_dmd_START (7)
#define PMIC_BUCK3_CTRL4_buck3_eco_dmd_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_ton : 3;
        unsigned char buck3_reg_bias : 1;
        unsigned char buck3_regop_c : 1;
        unsigned char buck3_reg_dr : 3;
    } reg;
} PMIC_BUCK3_CTRL5_UNION;
#endif
#define PMIC_BUCK3_CTRL5_buck3_ton_START (0)
#define PMIC_BUCK3_CTRL5_buck3_ton_END (2)
#define PMIC_BUCK3_CTRL5_buck3_reg_bias_START (3)
#define PMIC_BUCK3_CTRL5_buck3_reg_bias_END (3)
#define PMIC_BUCK3_CTRL5_buck3_regop_c_START (4)
#define PMIC_BUCK3_CTRL5_buck3_regop_c_END (4)
#define PMIC_BUCK3_CTRL5_buck3_reg_dr_START (5)
#define PMIC_BUCK3_CTRL5_buck3_reg_dr_END (7)
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
        unsigned char buck3_dmd_clamp : 1;
        unsigned char buck3_bias_dmd_sel : 1;
        unsigned char buck3_regop_clamp : 1;
        unsigned char buck3_ton_dmd : 1;
        unsigned char buck3_ocp_delay : 1;
        unsigned char buck3_lx_dt : 2;
        unsigned char buck3_cmp_filter : 1;
    } reg;
} PMIC_BUCK3_CTRL7_UNION;
#endif
#define PMIC_BUCK3_CTRL7_buck3_dmd_clamp_START (0)
#define PMIC_BUCK3_CTRL7_buck3_dmd_clamp_END (0)
#define PMIC_BUCK3_CTRL7_buck3_bias_dmd_sel_START (1)
#define PMIC_BUCK3_CTRL7_buck3_bias_dmd_sel_END (1)
#define PMIC_BUCK3_CTRL7_buck3_regop_clamp_START (2)
#define PMIC_BUCK3_CTRL7_buck3_regop_clamp_END (2)
#define PMIC_BUCK3_CTRL7_buck3_ton_dmd_START (3)
#define PMIC_BUCK3_CTRL7_buck3_ton_dmd_END (3)
#define PMIC_BUCK3_CTRL7_buck3_ocp_delay_START (4)
#define PMIC_BUCK3_CTRL7_buck3_ocp_delay_END (4)
#define PMIC_BUCK3_CTRL7_buck3_lx_dt_START (5)
#define PMIC_BUCK3_CTRL7_buck3_lx_dt_END (6)
#define PMIC_BUCK3_CTRL7_buck3_cmp_filter_START (7)
#define PMIC_BUCK3_CTRL7_buck3_cmp_filter_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_dmd_cmp_sel : 1;
        unsigned char buck3_drv_delay : 1;
        unsigned char buck3_pow_drv_ctrl : 1;
        unsigned char buck3_autoton_sel : 2;
        unsigned char buck3_autoton_ctrl : 1;
        unsigned char buck3_fix_ton : 2;
    } reg;
} PMIC_BUCK3_CTRL8_UNION;
#endif
#define PMIC_BUCK3_CTRL8_buck3_dmd_cmp_sel_START (0)
#define PMIC_BUCK3_CTRL8_buck3_dmd_cmp_sel_END (0)
#define PMIC_BUCK3_CTRL8_buck3_drv_delay_START (1)
#define PMIC_BUCK3_CTRL8_buck3_drv_delay_END (1)
#define PMIC_BUCK3_CTRL8_buck3_pow_drv_ctrl_START (2)
#define PMIC_BUCK3_CTRL8_buck3_pow_drv_ctrl_END (2)
#define PMIC_BUCK3_CTRL8_buck3_autoton_sel_START (3)
#define PMIC_BUCK3_CTRL8_buck3_autoton_sel_END (4)
#define PMIC_BUCK3_CTRL8_buck3_autoton_ctrl_START (5)
#define PMIC_BUCK3_CTRL8_buck3_autoton_ctrl_END (5)
#define PMIC_BUCK3_CTRL8_buck3_fix_ton_START (6)
#define PMIC_BUCK3_CTRL8_buck3_fix_ton_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_mul_func : 8;
    } reg;
} PMIC_BUCK3_CTRL9_UNION;
#endif
#define PMIC_BUCK3_CTRL9_buck3_mul_func_START (0)
#define PMIC_BUCK3_CTRL9_buck3_mul_func_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_dbias : 4;
        unsigned char buck4_adj_rlx : 4;
    } reg;
} PMIC_BUCK4_CTRL0_UNION;
#endif
#define PMIC_BUCK4_CTRL0_buck4_dbias_START (0)
#define PMIC_BUCK4_CTRL0_buck4_dbias_END (3)
#define PMIC_BUCK4_CTRL0_buck4_adj_rlx_START (4)
#define PMIC_BUCK4_CTRL0_buck4_adj_rlx_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_ng_dt_sel : 1;
        unsigned char buck4_pg_dt_sel : 1;
        unsigned char buck4_nmos_switch : 1;
        unsigned char buck4_dt_sel : 3;
        unsigned char buck4_ocp_sel : 2;
    } reg;
} PMIC_BUCK4_CTRL1_UNION;
#endif
#define PMIC_BUCK4_CTRL1_buck4_ng_dt_sel_START (0)
#define PMIC_BUCK4_CTRL1_buck4_ng_dt_sel_END (0)
#define PMIC_BUCK4_CTRL1_buck4_pg_dt_sel_START (1)
#define PMIC_BUCK4_CTRL1_buck4_pg_dt_sel_END (1)
#define PMIC_BUCK4_CTRL1_buck4_nmos_switch_START (2)
#define PMIC_BUCK4_CTRL1_buck4_nmos_switch_END (2)
#define PMIC_BUCK4_CTRL1_buck4_dt_sel_START (3)
#define PMIC_BUCK4_CTRL1_buck4_dt_sel_END (5)
#define PMIC_BUCK4_CTRL1_buck4_ocp_sel_START (6)
#define PMIC_BUCK4_CTRL1_buck4_ocp_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_pg_n_sel : 3;
        unsigned char buck4_pg_p_sel : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK4_CTRL2_UNION;
#endif
#define PMIC_BUCK4_CTRL2_buck4_pg_n_sel_START (0)
#define PMIC_BUCK4_CTRL2_buck4_pg_n_sel_END (2)
#define PMIC_BUCK4_CTRL2_buck4_pg_p_sel_START (3)
#define PMIC_BUCK4_CTRL2_buck4_pg_p_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_ng_n_sel : 3;
        unsigned char buck4_ng_p_sel : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK4_CTRL3_UNION;
#endif
#define PMIC_BUCK4_CTRL3_buck4_ng_n_sel_START (0)
#define PMIC_BUCK4_CTRL3_buck4_ng_n_sel_END (2)
#define PMIC_BUCK4_CTRL3_buck4_ng_p_sel_START (3)
#define PMIC_BUCK4_CTRL3_buck4_ng_p_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_reg_r : 2;
        unsigned char buck4_filter_ton : 2;
        unsigned char buck4_short_pdp : 1;
        unsigned char buck4_adj_clx : 2;
        unsigned char buck4_ocp_dis : 1;
    } reg;
} PMIC_BUCK4_CTRL4_UNION;
#endif
#define PMIC_BUCK4_CTRL4_buck4_reg_r_START (0)
#define PMIC_BUCK4_CTRL4_buck4_reg_r_END (1)
#define PMIC_BUCK4_CTRL4_buck4_filter_ton_START (2)
#define PMIC_BUCK4_CTRL4_buck4_filter_ton_END (3)
#define PMIC_BUCK4_CTRL4_buck4_short_pdp_START (4)
#define PMIC_BUCK4_CTRL4_buck4_short_pdp_END (4)
#define PMIC_BUCK4_CTRL4_buck4_adj_clx_START (5)
#define PMIC_BUCK4_CTRL4_buck4_adj_clx_END (6)
#define PMIC_BUCK4_CTRL4_buck4_ocp_dis_START (7)
#define PMIC_BUCK4_CTRL4_buck4_ocp_dis_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_ton : 3;
        unsigned char buck4_reg_bias : 1;
        unsigned char buck4_regop_c : 1;
        unsigned char buck4_reg_dr : 3;
    } reg;
} PMIC_BUCK4_CTRL5_UNION;
#endif
#define PMIC_BUCK4_CTRL5_buck4_ton_START (0)
#define PMIC_BUCK4_CTRL5_buck4_ton_END (2)
#define PMIC_BUCK4_CTRL5_buck4_reg_bias_START (3)
#define PMIC_BUCK4_CTRL5_buck4_reg_bias_END (3)
#define PMIC_BUCK4_CTRL5_buck4_regop_c_START (4)
#define PMIC_BUCK4_CTRL5_buck4_regop_c_END (4)
#define PMIC_BUCK4_CTRL5_buck4_reg_dr_START (5)
#define PMIC_BUCK4_CTRL5_buck4_reg_dr_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_dmd_ton : 3;
        unsigned char buck4_ocp_toff : 2;
        unsigned char buck4_dmd_sel : 3;
    } reg;
} PMIC_BUCK4_CTRL6_UNION;
#endif
#define PMIC_BUCK4_CTRL6_buck4_dmd_ton_START (0)
#define PMIC_BUCK4_CTRL6_buck4_dmd_ton_END (2)
#define PMIC_BUCK4_CTRL6_buck4_ocp_toff_START (3)
#define PMIC_BUCK4_CTRL6_buck4_ocp_toff_END (4)
#define PMIC_BUCK4_CTRL6_buck4_dmd_sel_START (5)
#define PMIC_BUCK4_CTRL6_buck4_dmd_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_dmd_clamp : 1;
        unsigned char buck4_bias_dmd_sel : 1;
        unsigned char buck4_regop_clamp : 1;
        unsigned char buck4_ton_dmd : 1;
        unsigned char buck4_ocp_delay : 1;
        unsigned char buck4_lx_dt : 2;
        unsigned char buck4_min_vo_ton_sel : 1;
    } reg;
} PMIC_BUCK4_CTRL7_UNION;
#endif
#define PMIC_BUCK4_CTRL7_buck4_dmd_clamp_START (0)
#define PMIC_BUCK4_CTRL7_buck4_dmd_clamp_END (0)
#define PMIC_BUCK4_CTRL7_buck4_bias_dmd_sel_START (1)
#define PMIC_BUCK4_CTRL7_buck4_bias_dmd_sel_END (1)
#define PMIC_BUCK4_CTRL7_buck4_regop_clamp_START (2)
#define PMIC_BUCK4_CTRL7_buck4_regop_clamp_END (2)
#define PMIC_BUCK4_CTRL7_buck4_ton_dmd_START (3)
#define PMIC_BUCK4_CTRL7_buck4_ton_dmd_END (3)
#define PMIC_BUCK4_CTRL7_buck4_ocp_delay_START (4)
#define PMIC_BUCK4_CTRL7_buck4_ocp_delay_END (4)
#define PMIC_BUCK4_CTRL7_buck4_lx_dt_START (5)
#define PMIC_BUCK4_CTRL7_buck4_lx_dt_END (6)
#define PMIC_BUCK4_CTRL7_buck4_min_vo_ton_sel_START (7)
#define PMIC_BUCK4_CTRL7_buck4_min_vo_ton_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_dmd_cmp_sel : 1;
        unsigned char buck4_drv_delay : 1;
        unsigned char buck4_pow_drv_ctrl : 1;
        unsigned char buck4_autoton_ctrl : 1;
        unsigned char buck4_eco_dmd : 1;
        unsigned char buck4_cmp_filter : 1;
        unsigned char buck4_fix_ton : 2;
    } reg;
} PMIC_BUCK4_CTRL8_UNION;
#endif
#define PMIC_BUCK4_CTRL8_buck4_dmd_cmp_sel_START (0)
#define PMIC_BUCK4_CTRL8_buck4_dmd_cmp_sel_END (0)
#define PMIC_BUCK4_CTRL8_buck4_drv_delay_START (1)
#define PMIC_BUCK4_CTRL8_buck4_drv_delay_END (1)
#define PMIC_BUCK4_CTRL8_buck4_pow_drv_ctrl_START (2)
#define PMIC_BUCK4_CTRL8_buck4_pow_drv_ctrl_END (2)
#define PMIC_BUCK4_CTRL8_buck4_autoton_ctrl_START (3)
#define PMIC_BUCK4_CTRL8_buck4_autoton_ctrl_END (3)
#define PMIC_BUCK4_CTRL8_buck4_eco_dmd_START (4)
#define PMIC_BUCK4_CTRL8_buck4_eco_dmd_END (4)
#define PMIC_BUCK4_CTRL8_buck4_cmp_filter_START (5)
#define PMIC_BUCK4_CTRL8_buck4_cmp_filter_END (5)
#define PMIC_BUCK4_CTRL8_buck4_fix_ton_START (6)
#define PMIC_BUCK4_CTRL8_buck4_fix_ton_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_rampconfig : 3;
        unsigned char buck4_reg_zero_en : 1;
        unsigned char buck4_rampdown_delay : 2;
        unsigned char buck4_ramp_cap : 2;
    } reg;
} PMIC_BUCK4_CTRL9_UNION;
#endif
#define PMIC_BUCK4_CTRL9_buck4_rampconfig_START (0)
#define PMIC_BUCK4_CTRL9_buck4_rampconfig_END (2)
#define PMIC_BUCK4_CTRL9_buck4_reg_zero_en_START (3)
#define PMIC_BUCK4_CTRL9_buck4_reg_zero_en_END (3)
#define PMIC_BUCK4_CTRL9_buck4_rampdown_delay_START (4)
#define PMIC_BUCK4_CTRL9_buck4_rampdown_delay_END (5)
#define PMIC_BUCK4_CTRL9_buck4_ramp_cap_START (6)
#define PMIC_BUCK4_CTRL9_buck4_ramp_cap_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_autoton_sel : 2;
        unsigned char buck4_ramp_eco : 1;
        unsigned char buck4_adj_rlx_ramp : 4;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK4_CTRL10_UNION;
#endif
#define PMIC_BUCK4_CTRL10_buck4_autoton_sel_START (0)
#define PMIC_BUCK4_CTRL10_buck4_autoton_sel_END (1)
#define PMIC_BUCK4_CTRL10_buck4_ramp_eco_START (2)
#define PMIC_BUCK4_CTRL10_buck4_ramp_eco_END (2)
#define PMIC_BUCK4_CTRL10_buck4_adj_rlx_ramp_START (3)
#define PMIC_BUCK4_CTRL10_buck4_adj_rlx_ramp_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_mul_func : 8;
    } reg;
} PMIC_BUCK4_CTRL11_UNION;
#endif
#define PMIC_BUCK4_CTRL11_buck4_mul_func_START (0)
#define PMIC_BUCK4_CTRL11_buck4_mul_func_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_dbias : 4;
        unsigned char buck5_adj_rlx : 4;
    } reg;
} PMIC_BUCK5_CTRL0_UNION;
#endif
#define PMIC_BUCK5_CTRL0_buck5_dbias_START (0)
#define PMIC_BUCK5_CTRL0_buck5_dbias_END (3)
#define PMIC_BUCK5_CTRL0_buck5_adj_rlx_START (4)
#define PMIC_BUCK5_CTRL0_buck5_adj_rlx_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_ng_dt_sel : 1;
        unsigned char buck5_pg_dt_sel : 1;
        unsigned char buck5_nmos_switch : 1;
        unsigned char buck5_dt_sel : 3;
        unsigned char buck5_ocp_sel : 2;
    } reg;
} PMIC_BUCK5_CTRL1_UNION;
#endif
#define PMIC_BUCK5_CTRL1_buck5_ng_dt_sel_START (0)
#define PMIC_BUCK5_CTRL1_buck5_ng_dt_sel_END (0)
#define PMIC_BUCK5_CTRL1_buck5_pg_dt_sel_START (1)
#define PMIC_BUCK5_CTRL1_buck5_pg_dt_sel_END (1)
#define PMIC_BUCK5_CTRL1_buck5_nmos_switch_START (2)
#define PMIC_BUCK5_CTRL1_buck5_nmos_switch_END (2)
#define PMIC_BUCK5_CTRL1_buck5_dt_sel_START (3)
#define PMIC_BUCK5_CTRL1_buck5_dt_sel_END (5)
#define PMIC_BUCK5_CTRL1_buck5_ocp_sel_START (6)
#define PMIC_BUCK5_CTRL1_buck5_ocp_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_pg_n_sel : 3;
        unsigned char buck5_pg_p_sel : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK5_CTRL2_UNION;
#endif
#define PMIC_BUCK5_CTRL2_buck5_pg_n_sel_START (0)
#define PMIC_BUCK5_CTRL2_buck5_pg_n_sel_END (2)
#define PMIC_BUCK5_CTRL2_buck5_pg_p_sel_START (3)
#define PMIC_BUCK5_CTRL2_buck5_pg_p_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_ng_n_sel : 3;
        unsigned char buck5_ng_p_sel : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK5_CTRL3_UNION;
#endif
#define PMIC_BUCK5_CTRL3_buck5_ng_n_sel_START (0)
#define PMIC_BUCK5_CTRL3_buck5_ng_n_sel_END (2)
#define PMIC_BUCK5_CTRL3_buck5_ng_p_sel_START (3)
#define PMIC_BUCK5_CTRL3_buck5_ng_p_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_reg_r : 2;
        unsigned char buck5_filter_ton : 2;
        unsigned char buck5_short_pdp : 1;
        unsigned char buck5_adj_clx : 2;
        unsigned char buck5_ocp_dis : 1;
    } reg;
} PMIC_BUCK5_CTRL4_UNION;
#endif
#define PMIC_BUCK5_CTRL4_buck5_reg_r_START (0)
#define PMIC_BUCK5_CTRL4_buck5_reg_r_END (1)
#define PMIC_BUCK5_CTRL4_buck5_filter_ton_START (2)
#define PMIC_BUCK5_CTRL4_buck5_filter_ton_END (3)
#define PMIC_BUCK5_CTRL4_buck5_short_pdp_START (4)
#define PMIC_BUCK5_CTRL4_buck5_short_pdp_END (4)
#define PMIC_BUCK5_CTRL4_buck5_adj_clx_START (5)
#define PMIC_BUCK5_CTRL4_buck5_adj_clx_END (6)
#define PMIC_BUCK5_CTRL4_buck5_ocp_dis_START (7)
#define PMIC_BUCK5_CTRL4_buck5_ocp_dis_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_ton : 3;
        unsigned char buck5_reg_bias : 1;
        unsigned char buck5_regop_c : 1;
        unsigned char buck5_reg_dr : 3;
    } reg;
} PMIC_BUCK5_CTRL5_UNION;
#endif
#define PMIC_BUCK5_CTRL5_buck5_ton_START (0)
#define PMIC_BUCK5_CTRL5_buck5_ton_END (2)
#define PMIC_BUCK5_CTRL5_buck5_reg_bias_START (3)
#define PMIC_BUCK5_CTRL5_buck5_reg_bias_END (3)
#define PMIC_BUCK5_CTRL5_buck5_regop_c_START (4)
#define PMIC_BUCK5_CTRL5_buck5_regop_c_END (4)
#define PMIC_BUCK5_CTRL5_buck5_reg_dr_START (5)
#define PMIC_BUCK5_CTRL5_buck5_reg_dr_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_dmd_ton : 3;
        unsigned char buck5_ocp_toff : 2;
        unsigned char buck5_dmd_sel : 3;
    } reg;
} PMIC_BUCK5_CTRL6_UNION;
#endif
#define PMIC_BUCK5_CTRL6_buck5_dmd_ton_START (0)
#define PMIC_BUCK5_CTRL6_buck5_dmd_ton_END (2)
#define PMIC_BUCK5_CTRL6_buck5_ocp_toff_START (3)
#define PMIC_BUCK5_CTRL6_buck5_ocp_toff_END (4)
#define PMIC_BUCK5_CTRL6_buck5_dmd_sel_START (5)
#define PMIC_BUCK5_CTRL6_buck5_dmd_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_dmd_clamp : 1;
        unsigned char buck5_bias_dmd_sel : 1;
        unsigned char buck5_regop_clamp : 1;
        unsigned char buck5_ton_dmd : 1;
        unsigned char buck5_ocp_delay : 1;
        unsigned char buck5_lx_dt : 2;
        unsigned char buck5_min_vo_ton_sel : 1;
    } reg;
} PMIC_BUCK5_CTRL7_UNION;
#endif
#define PMIC_BUCK5_CTRL7_buck5_dmd_clamp_START (0)
#define PMIC_BUCK5_CTRL7_buck5_dmd_clamp_END (0)
#define PMIC_BUCK5_CTRL7_buck5_bias_dmd_sel_START (1)
#define PMIC_BUCK5_CTRL7_buck5_bias_dmd_sel_END (1)
#define PMIC_BUCK5_CTRL7_buck5_regop_clamp_START (2)
#define PMIC_BUCK5_CTRL7_buck5_regop_clamp_END (2)
#define PMIC_BUCK5_CTRL7_buck5_ton_dmd_START (3)
#define PMIC_BUCK5_CTRL7_buck5_ton_dmd_END (3)
#define PMIC_BUCK5_CTRL7_buck5_ocp_delay_START (4)
#define PMIC_BUCK5_CTRL7_buck5_ocp_delay_END (4)
#define PMIC_BUCK5_CTRL7_buck5_lx_dt_START (5)
#define PMIC_BUCK5_CTRL7_buck5_lx_dt_END (6)
#define PMIC_BUCK5_CTRL7_buck5_min_vo_ton_sel_START (7)
#define PMIC_BUCK5_CTRL7_buck5_min_vo_ton_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_dmd_cmp_sel : 1;
        unsigned char buck5_drv_delay : 1;
        unsigned char buck5_pow_drv_ctrl : 1;
        unsigned char buck5_autoton_ctrl : 1;
        unsigned char buck5_eco_dmd : 1;
        unsigned char buck5_cmp_filter : 1;
        unsigned char buck5_fix_ton : 2;
    } reg;
} PMIC_BUCK5_CTRL8_UNION;
#endif
#define PMIC_BUCK5_CTRL8_buck5_dmd_cmp_sel_START (0)
#define PMIC_BUCK5_CTRL8_buck5_dmd_cmp_sel_END (0)
#define PMIC_BUCK5_CTRL8_buck5_drv_delay_START (1)
#define PMIC_BUCK5_CTRL8_buck5_drv_delay_END (1)
#define PMIC_BUCK5_CTRL8_buck5_pow_drv_ctrl_START (2)
#define PMIC_BUCK5_CTRL8_buck5_pow_drv_ctrl_END (2)
#define PMIC_BUCK5_CTRL8_buck5_autoton_ctrl_START (3)
#define PMIC_BUCK5_CTRL8_buck5_autoton_ctrl_END (3)
#define PMIC_BUCK5_CTRL8_buck5_eco_dmd_START (4)
#define PMIC_BUCK5_CTRL8_buck5_eco_dmd_END (4)
#define PMIC_BUCK5_CTRL8_buck5_cmp_filter_START (5)
#define PMIC_BUCK5_CTRL8_buck5_cmp_filter_END (5)
#define PMIC_BUCK5_CTRL8_buck5_fix_ton_START (6)
#define PMIC_BUCK5_CTRL8_buck5_fix_ton_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_rampconfig : 3;
        unsigned char buck5_reg_zero_en : 1;
        unsigned char buck5_rampdown_delay : 2;
        unsigned char buck5_ramp_cap : 2;
    } reg;
} PMIC_BUCK5_CTRL9_UNION;
#endif
#define PMIC_BUCK5_CTRL9_buck5_rampconfig_START (0)
#define PMIC_BUCK5_CTRL9_buck5_rampconfig_END (2)
#define PMIC_BUCK5_CTRL9_buck5_reg_zero_en_START (3)
#define PMIC_BUCK5_CTRL9_buck5_reg_zero_en_END (3)
#define PMIC_BUCK5_CTRL9_buck5_rampdown_delay_START (4)
#define PMIC_BUCK5_CTRL9_buck5_rampdown_delay_END (5)
#define PMIC_BUCK5_CTRL9_buck5_ramp_cap_START (6)
#define PMIC_BUCK5_CTRL9_buck5_ramp_cap_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_autoton_sel : 2;
        unsigned char buck5_ramp_eco : 1;
        unsigned char buck5_adj_rlx_ramp : 4;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK5_CTRL10_UNION;
#endif
#define PMIC_BUCK5_CTRL10_buck5_autoton_sel_START (0)
#define PMIC_BUCK5_CTRL10_buck5_autoton_sel_END (1)
#define PMIC_BUCK5_CTRL10_buck5_ramp_eco_START (2)
#define PMIC_BUCK5_CTRL10_buck5_ramp_eco_END (2)
#define PMIC_BUCK5_CTRL10_buck5_adj_rlx_ramp_START (3)
#define PMIC_BUCK5_CTRL10_buck5_adj_rlx_ramp_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_mul_func : 8;
    } reg;
} PMIC_BUCK5_CTRL11_UNION;
#endif
#define PMIC_BUCK5_CTRL11_buck5_mul_func_START (0)
#define PMIC_BUCK5_CTRL11_buck5_mul_func_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck_ton_trim : 1;
        unsigned char buck_vin_filter : 2;
        unsigned char buck_vin_det_en : 1;
        unsigned char buck_eco_isel : 1;
        unsigned char buck_ocptrim_ctl : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK_CTRL0_UNION;
#endif
#define PMIC_BUCK_CTRL0_buck_ton_trim_START (0)
#define PMIC_BUCK_CTRL0_buck_ton_trim_END (0)
#define PMIC_BUCK_CTRL0_buck_vin_filter_START (1)
#define PMIC_BUCK_CTRL0_buck_vin_filter_END (2)
#define PMIC_BUCK_CTRL0_buck_vin_det_en_START (3)
#define PMIC_BUCK_CTRL0_buck_vin_det_en_END (3)
#define PMIC_BUCK_CTRL0_buck_eco_isel_START (4)
#define PMIC_BUCK_CTRL0_buck_eco_isel_END (4)
#define PMIC_BUCK_CTRL0_buck_ocptrim_ctl_START (5)
#define PMIC_BUCK_CTRL0_buck_ocptrim_ctl_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck_test : 8;
    } reg;
} PMIC_BUCK_CTRL1_UNION;
#endif
#define PMIC_BUCK_CTRL1_buck_test_START (0)
#define PMIC_BUCK_CTRL1_buck_test_END (7)
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
        unsigned char buck_reserve2 : 8;
    } reg;
} PMIC_BUCK_RESERVE2_UNION;
#endif
#define PMIC_BUCK_RESERVE2_buck_reserve2_START (0)
#define PMIC_BUCK_RESERVE2_buck_reserve2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo0_2_vrset : 3;
        unsigned char ldo0_2_ocp_enn : 1;
        unsigned char ldo0_2_eco_set : 1;
        unsigned char ldo0_2_ocpd_vos_set : 1;
        unsigned char ldo0_2_sst_set : 1;
        unsigned char ldo0_2_vea_ctrl : 1;
    } reg;
} PMIC_LDO0_CTRL0_UNION;
#endif
#define PMIC_LDO0_CTRL0_ldo0_2_vrset_START (0)
#define PMIC_LDO0_CTRL0_ldo0_2_vrset_END (2)
#define PMIC_LDO0_CTRL0_ldo0_2_ocp_enn_START (3)
#define PMIC_LDO0_CTRL0_ldo0_2_ocp_enn_END (3)
#define PMIC_LDO0_CTRL0_ldo0_2_eco_set_START (4)
#define PMIC_LDO0_CTRL0_ldo0_2_eco_set_END (4)
#define PMIC_LDO0_CTRL0_ldo0_2_ocpd_vos_set_START (5)
#define PMIC_LDO0_CTRL0_ldo0_2_ocpd_vos_set_END (5)
#define PMIC_LDO0_CTRL0_ldo0_2_sst_set_START (6)
#define PMIC_LDO0_CTRL0_ldo0_2_sst_set_END (6)
#define PMIC_LDO0_CTRL0_ldo0_2_vea_ctrl_START (7)
#define PMIC_LDO0_CTRL0_ldo0_2_vea_ctrl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo0_2_ocpd_set : 2;
        unsigned char ldo0_2_ocps_set : 2;
        unsigned char ldo0_2_ocps_en : 1;
        unsigned char ldo0_2_vgpr_en : 1;
        unsigned char ldo0_2_pd_set : 1;
        unsigned char ldo0_2_stb_set : 1;
    } reg;
} PMIC_LDO0_CTRL1_UNION;
#endif
#define PMIC_LDO0_CTRL1_ldo0_2_ocpd_set_START (0)
#define PMIC_LDO0_CTRL1_ldo0_2_ocpd_set_END (1)
#define PMIC_LDO0_CTRL1_ldo0_2_ocps_set_START (2)
#define PMIC_LDO0_CTRL1_ldo0_2_ocps_set_END (3)
#define PMIC_LDO0_CTRL1_ldo0_2_ocps_en_START (4)
#define PMIC_LDO0_CTRL1_ldo0_2_ocps_en_END (4)
#define PMIC_LDO0_CTRL1_ldo0_2_vgpr_en_START (5)
#define PMIC_LDO0_CTRL1_ldo0_2_vgpr_en_END (5)
#define PMIC_LDO0_CTRL1_ldo0_2_pd_set_START (6)
#define PMIC_LDO0_CTRL1_ldo0_2_pd_set_END (6)
#define PMIC_LDO0_CTRL1_ldo0_2_stb_set_START (7)
#define PMIC_LDO0_CTRL1_ldo0_2_stb_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo1_vrset : 3;
        unsigned char ldo1_ocp_enn : 1;
        unsigned char ldo1_ref_buf_rset : 2;
        unsigned char ldo1_ref_buf_bypass : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO1_CTRL0_UNION;
#endif
#define PMIC_LDO1_CTRL0_ldo1_vrset_START (0)
#define PMIC_LDO1_CTRL0_ldo1_vrset_END (2)
#define PMIC_LDO1_CTRL0_ldo1_ocp_enn_START (3)
#define PMIC_LDO1_CTRL0_ldo1_ocp_enn_END (3)
#define PMIC_LDO1_CTRL0_ldo1_ref_buf_rset_START (4)
#define PMIC_LDO1_CTRL0_ldo1_ref_buf_rset_END (5)
#define PMIC_LDO1_CTRL0_ldo1_ref_buf_bypass_START (6)
#define PMIC_LDO1_CTRL0_ldo1_ref_buf_bypass_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo1_psrr2_set : 3;
        unsigned char ldo1_psrr1_set : 3;
        unsigned char ldo1_psrr_en : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO1_CTRL1_UNION;
#endif
#define PMIC_LDO1_CTRL1_ldo1_psrr2_set_START (0)
#define PMIC_LDO1_CTRL1_ldo1_psrr2_set_END (2)
#define PMIC_LDO1_CTRL1_ldo1_psrr1_set_START (3)
#define PMIC_LDO1_CTRL1_ldo1_psrr1_set_END (5)
#define PMIC_LDO1_CTRL1_ldo1_psrr_en_START (6)
#define PMIC_LDO1_CTRL1_ldo1_psrr_en_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo1_ocps_en : 1;
        unsigned char ldo1_bw_en : 1;
        unsigned char ldo1_vgpr_en : 1;
        unsigned char ldo1_comp : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO1_CTRL2_UNION;
#endif
#define PMIC_LDO1_CTRL2_ldo1_ocps_en_START (0)
#define PMIC_LDO1_CTRL2_ldo1_ocps_en_END (0)
#define PMIC_LDO1_CTRL2_ldo1_bw_en_START (1)
#define PMIC_LDO1_CTRL2_ldo1_bw_en_END (1)
#define PMIC_LDO1_CTRL2_ldo1_vgpr_en_START (2)
#define PMIC_LDO1_CTRL2_ldo1_vgpr_en_END (2)
#define PMIC_LDO1_CTRL2_ldo1_comp_START (3)
#define PMIC_LDO1_CTRL2_ldo1_comp_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo2_vrset : 3;
        unsigned char ldo2_ocp_enn : 1;
        unsigned char ldo2_ocp_set : 2;
        unsigned char ldo2_eco_set : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO2_CTRL0_UNION;
#endif
#define PMIC_LDO2_CTRL0_ldo2_vrset_START (0)
#define PMIC_LDO2_CTRL0_ldo2_vrset_END (2)
#define PMIC_LDO2_CTRL0_ldo2_ocp_enn_START (3)
#define PMIC_LDO2_CTRL0_ldo2_ocp_enn_END (3)
#define PMIC_LDO2_CTRL0_ldo2_ocp_set_START (4)
#define PMIC_LDO2_CTRL0_ldo2_ocp_set_END (5)
#define PMIC_LDO2_CTRL0_ldo2_eco_set_START (6)
#define PMIC_LDO2_CTRL0_ldo2_eco_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo2_stb_set : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO2_CTRL1_UNION;
#endif
#define PMIC_LDO2_CTRL1_ldo2_stb_set_START (0)
#define PMIC_LDO2_CTRL1_ldo2_stb_set_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo3_vrset : 3;
        unsigned char ldo3_ocp_enn : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO3_CTRL0_UNION;
#endif
#define PMIC_LDO3_CTRL0_ldo3_vrset_START (0)
#define PMIC_LDO3_CTRL0_ldo3_vrset_END (2)
#define PMIC_LDO3_CTRL0_ldo3_ocp_enn_START (3)
#define PMIC_LDO3_CTRL0_ldo3_ocp_enn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo3_psrr2_set : 3;
        unsigned char ldo3_psrr1_set : 3;
        unsigned char ldo3_psrr_en : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO3_CTRL1_UNION;
#endif
#define PMIC_LDO3_CTRL1_ldo3_psrr2_set_START (0)
#define PMIC_LDO3_CTRL1_ldo3_psrr2_set_END (2)
#define PMIC_LDO3_CTRL1_ldo3_psrr1_set_START (3)
#define PMIC_LDO3_CTRL1_ldo3_psrr1_set_END (5)
#define PMIC_LDO3_CTRL1_ldo3_psrr_en_START (6)
#define PMIC_LDO3_CTRL1_ldo3_psrr_en_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo4_vrset : 3;
        unsigned char ldo4_ocp_enn : 1;
        unsigned char ldo4_eco_set : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO4_CTRL_UNION;
#endif
#define PMIC_LDO4_CTRL_ldo4_vrset_START (0)
#define PMIC_LDO4_CTRL_ldo4_vrset_END (2)
#define PMIC_LDO4_CTRL_ldo4_ocp_enn_START (3)
#define PMIC_LDO4_CTRL_ldo4_ocp_enn_END (3)
#define PMIC_LDO4_CTRL_ldo4_eco_set_START (4)
#define PMIC_LDO4_CTRL_ldo4_eco_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo5_vrset : 3;
        unsigned char ldo5_ocp_enn : 1;
        unsigned char ldo5_ocp_set : 1;
        unsigned char ldo5_eco_set : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO5_CTRL_UNION;
#endif
#define PMIC_LDO5_CTRL_ldo5_vrset_START (0)
#define PMIC_LDO5_CTRL_ldo5_vrset_END (2)
#define PMIC_LDO5_CTRL_ldo5_ocp_enn_START (3)
#define PMIC_LDO5_CTRL_ldo5_ocp_enn_END (3)
#define PMIC_LDO5_CTRL_ldo5_ocp_set_START (4)
#define PMIC_LDO5_CTRL_ldo5_ocp_set_END (4)
#define PMIC_LDO5_CTRL_ldo5_eco_set_START (5)
#define PMIC_LDO5_CTRL_ldo5_eco_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo6_vrset : 3;
        unsigned char ldo6_ocp_enn : 1;
        unsigned char ldo6_ocps_en : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO6_CTRL0_UNION;
#endif
#define PMIC_LDO6_CTRL0_ldo6_vrset_START (0)
#define PMIC_LDO6_CTRL0_ldo6_vrset_END (2)
#define PMIC_LDO6_CTRL0_ldo6_ocp_enn_START (3)
#define PMIC_LDO6_CTRL0_ldo6_ocp_enn_END (3)
#define PMIC_LDO6_CTRL0_ldo6_ocps_en_START (4)
#define PMIC_LDO6_CTRL0_ldo6_ocps_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo6_ocpd_set : 2;
        unsigned char ldo6_ocps_set : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO6_CTRL1_UNION;
#endif
#define PMIC_LDO6_CTRL1_ldo6_ocpd_set_START (0)
#define PMIC_LDO6_CTRL1_ldo6_ocpd_set_END (1)
#define PMIC_LDO6_CTRL1_ldo6_ocps_set_START (2)
#define PMIC_LDO6_CTRL1_ldo6_ocps_set_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo8_vrset : 3;
        unsigned char ldo8_ocp_enn : 1;
        unsigned char ldo8_eco_set : 1;
        unsigned char ldo8_ocp_set : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO8_CTRL_UNION;
#endif
#define PMIC_LDO8_CTRL_ldo8_vrset_START (0)
#define PMIC_LDO8_CTRL_ldo8_vrset_END (2)
#define PMIC_LDO8_CTRL_ldo8_ocp_enn_START (3)
#define PMIC_LDO8_CTRL_ldo8_ocp_enn_END (3)
#define PMIC_LDO8_CTRL_ldo8_eco_set_START (4)
#define PMIC_LDO8_CTRL_ldo8_eco_set_END (4)
#define PMIC_LDO8_CTRL_ldo8_ocp_set_START (5)
#define PMIC_LDO8_CTRL_ldo8_ocp_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo9_vrset : 3;
        unsigned char ldo9_ocp_enn : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO9_CTRL_UNION;
#endif
#define PMIC_LDO9_CTRL_ldo9_vrset_START (0)
#define PMIC_LDO9_CTRL_ldo9_vrset_END (2)
#define PMIC_LDO9_CTRL_ldo9_ocp_enn_START (3)
#define PMIC_LDO9_CTRL_ldo9_ocp_enn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo11_vrset : 3;
        unsigned char ldo11_ocp_enn : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LD11_CTRL_UNION;
#endif
#define PMIC_LD11_CTRL_ldo11_vrset_START (0)
#define PMIC_LD11_CTRL_ldo11_vrset_END (2)
#define PMIC_LD11_CTRL_ldo11_ocp_enn_START (3)
#define PMIC_LD11_CTRL_ldo11_ocp_enn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo12_vrset : 3;
        unsigned char ldo12_ocp_enn : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LD12_CTRL_UNION;
#endif
#define PMIC_LD12_CTRL_ldo12_vrset_START (0)
#define PMIC_LD12_CTRL_ldo12_vrset_END (2)
#define PMIC_LD12_CTRL_ldo12_ocp_enn_START (3)
#define PMIC_LD12_CTRL_ldo12_ocp_enn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo13_vrset : 3;
        unsigned char ldo13_ocp_enn : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LD13_CTRL0_UNION;
#endif
#define PMIC_LD13_CTRL0_ldo13_vrset_START (0)
#define PMIC_LD13_CTRL0_ldo13_vrset_END (2)
#define PMIC_LD13_CTRL0_ldo13_ocp_enn_START (3)
#define PMIC_LD13_CTRL0_ldo13_ocp_enn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo13_psrr2_set : 3;
        unsigned char ldo13_psrr1_set : 3;
        unsigned char ldo13_psrr_en : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LD13_CTRL1_UNION;
#endif
#define PMIC_LD13_CTRL1_ldo13_psrr2_set_START (0)
#define PMIC_LD13_CTRL1_ldo13_psrr2_set_END (2)
#define PMIC_LD13_CTRL1_ldo13_psrr1_set_START (3)
#define PMIC_LD13_CTRL1_ldo13_psrr1_set_END (5)
#define PMIC_LD13_CTRL1_ldo13_psrr_en_START (6)
#define PMIC_LD13_CTRL1_ldo13_psrr_en_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo14_vrset : 3;
        unsigned char ldo14_ocp_enn : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO14_CTRL_UNION;
#endif
#define PMIC_LDO14_CTRL_ldo14_vrset_START (0)
#define PMIC_LDO14_CTRL_ldo14_vrset_END (2)
#define PMIC_LDO14_CTRL_ldo14_ocp_enn_START (3)
#define PMIC_LDO14_CTRL_ldo14_ocp_enn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo15_vrset : 3;
        unsigned char ldo15_ocp_enn : 1;
        unsigned char ldo15_eco_set : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO15_CTRL_UNION;
#endif
#define PMIC_LDO15_CTRL_ldo15_vrset_START (0)
#define PMIC_LDO15_CTRL_ldo15_vrset_END (2)
#define PMIC_LDO15_CTRL_ldo15_ocp_enn_START (3)
#define PMIC_LDO15_CTRL_ldo15_ocp_enn_END (3)
#define PMIC_LDO15_CTRL_ldo15_eco_set_START (4)
#define PMIC_LDO15_CTRL_ldo15_eco_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo16_vrset : 3;
        unsigned char ldo16_ocp_enn : 1;
        unsigned char ldo16_eco_set : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO16_CTRL_UNION;
#endif
#define PMIC_LDO16_CTRL_ldo16_vrset_START (0)
#define PMIC_LDO16_CTRL_ldo16_vrset_END (2)
#define PMIC_LDO16_CTRL_ldo16_ocp_enn_START (3)
#define PMIC_LDO16_CTRL_ldo16_ocp_enn_END (3)
#define PMIC_LDO16_CTRL_ldo16_eco_set_START (4)
#define PMIC_LDO16_CTRL_ldo16_eco_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo17_vrset : 3;
        unsigned char ldo17_ocp_enn : 1;
        unsigned char ldo17_iss_en : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO17_CTRL_UNION;
#endif
#define PMIC_LDO17_CTRL_ldo17_vrset_START (0)
#define PMIC_LDO17_CTRL_ldo17_vrset_END (2)
#define PMIC_LDO17_CTRL_ldo17_ocp_enn_START (3)
#define PMIC_LDO17_CTRL_ldo17_ocp_enn_END (3)
#define PMIC_LDO17_CTRL_ldo17_iss_en_START (4)
#define PMIC_LDO17_CTRL_ldo17_iss_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo18_vrset : 3;
        unsigned char ldo18_ocp_enn : 1;
        unsigned char ldo18_eco_set : 1;
        unsigned char ldo18_ocp_set : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO18_CTRL0_UNION;
#endif
#define PMIC_LDO18_CTRL0_ldo18_vrset_START (0)
#define PMIC_LDO18_CTRL0_ldo18_vrset_END (2)
#define PMIC_LDO18_CTRL0_ldo18_ocp_enn_START (3)
#define PMIC_LDO18_CTRL0_ldo18_ocp_enn_END (3)
#define PMIC_LDO18_CTRL0_ldo18_eco_set_START (4)
#define PMIC_LDO18_CTRL0_ldo18_eco_set_END (4)
#define PMIC_LDO18_CTRL0_ldo18_ocp_set_START (5)
#define PMIC_LDO18_CTRL0_ldo18_ocp_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo19_vrset : 3;
        unsigned char ldo19_ocp_enn : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO19_CTRL0_UNION;
#endif
#define PMIC_LDO19_CTRL0_ldo19_vrset_START (0)
#define PMIC_LDO19_CTRL0_ldo19_vrset_END (2)
#define PMIC_LDO19_CTRL0_ldo19_ocp_enn_START (3)
#define PMIC_LDO19_CTRL0_ldo19_ocp_enn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo19_psrr2_set : 3;
        unsigned char ldo19_psrr1_set : 3;
        unsigned char ldo19_psrr_en : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LD19_CTRL1_UNION;
#endif
#define PMIC_LD19_CTRL1_ldo19_psrr2_set_START (0)
#define PMIC_LD19_CTRL1_ldo19_psrr2_set_END (2)
#define PMIC_LD19_CTRL1_ldo19_psrr1_set_START (3)
#define PMIC_LD19_CTRL1_ldo19_psrr1_set_END (5)
#define PMIC_LD19_CTRL1_ldo19_psrr_en_START (6)
#define PMIC_LD19_CTRL1_ldo19_psrr_en_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo20_vrset : 3;
        unsigned char ldo20_ocp_enn : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO20_CTRL0_UNION;
#endif
#define PMIC_LDO20_CTRL0_ldo20_vrset_START (0)
#define PMIC_LDO20_CTRL0_ldo20_vrset_END (2)
#define PMIC_LDO20_CTRL0_ldo20_ocp_enn_START (3)
#define PMIC_LDO20_CTRL0_ldo20_ocp_enn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo20_psrr2_set : 3;
        unsigned char ldo20_psrr1_set : 3;
        unsigned char ldo20_psrr_en : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO20_CTRL1_UNION;
#endif
#define PMIC_LDO20_CTRL1_ldo20_psrr2_set_START (0)
#define PMIC_LDO20_CTRL1_ldo20_psrr2_set_END (2)
#define PMIC_LDO20_CTRL1_ldo20_psrr1_set_START (3)
#define PMIC_LDO20_CTRL1_ldo20_psrr1_set_END (5)
#define PMIC_LDO20_CTRL1_ldo20_psrr_en_START (6)
#define PMIC_LDO20_CTRL1_ldo20_psrr_en_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo20_ocps_en : 1;
        unsigned char ldo20_bw_en : 1;
        unsigned char ldo20_vgpr_en : 1;
        unsigned char ldo20_comp : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO20_CTRL2_UNION;
#endif
#define PMIC_LDO20_CTRL2_ldo20_ocps_en_START (0)
#define PMIC_LDO20_CTRL2_ldo20_ocps_en_END (0)
#define PMIC_LDO20_CTRL2_ldo20_bw_en_START (1)
#define PMIC_LDO20_CTRL2_ldo20_bw_en_END (1)
#define PMIC_LDO20_CTRL2_ldo20_vgpr_en_START (2)
#define PMIC_LDO20_CTRL2_ldo20_vgpr_en_END (2)
#define PMIC_LDO20_CTRL2_ldo20_comp_START (3)
#define PMIC_LDO20_CTRL2_ldo20_comp_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo20_ocpd_set : 4;
        unsigned char ldo20_ocps_set : 4;
    } reg;
} PMIC_LDO20_CTRL3_UNION;
#endif
#define PMIC_LDO20_CTRL3_ldo20_ocpd_set_START (0)
#define PMIC_LDO20_CTRL3_ldo20_ocpd_set_END (3)
#define PMIC_LDO20_CTRL3_ldo20_ocps_set_START (4)
#define PMIC_LDO20_CTRL3_ldo20_ocps_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo21_vrset : 3;
        unsigned char ldo21_ocp_enn : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO21_CTRL_UNION;
#endif
#define PMIC_LDO21_CTRL_ldo21_vrset_START (0)
#define PMIC_LDO21_CTRL_ldo21_vrset_END (2)
#define PMIC_LDO21_CTRL_ldo21_ocp_enn_START (3)
#define PMIC_LDO21_CTRL_ldo21_ocp_enn_END (3)
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
        unsigned char ldo22_psrr2_set : 3;
        unsigned char ldo22_psrr1_set : 3;
        unsigned char ldo22_psrr_en : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO22_CTRL1_UNION;
#endif
#define PMIC_LDO22_CTRL1_ldo22_psrr2_set_START (0)
#define PMIC_LDO22_CTRL1_ldo22_psrr2_set_END (2)
#define PMIC_LDO22_CTRL1_ldo22_psrr1_set_START (3)
#define PMIC_LDO22_CTRL1_ldo22_psrr1_set_END (5)
#define PMIC_LDO22_CTRL1_ldo22_psrr_en_START (6)
#define PMIC_LDO22_CTRL1_ldo22_psrr_en_END (6)
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
        unsigned char ldo24_vrset : 3;
        unsigned char ldo24_ocp_enn : 1;
        unsigned char ldo24_bw_set : 3;
        unsigned char ldo24_en_bw : 1;
    } reg;
} PMIC_LDO24_CTRL0_UNION;
#endif
#define PMIC_LDO24_CTRL0_ldo24_vrset_START (0)
#define PMIC_LDO24_CTRL0_ldo24_vrset_END (2)
#define PMIC_LDO24_CTRL0_ldo24_ocp_enn_START (3)
#define PMIC_LDO24_CTRL0_ldo24_ocp_enn_END (3)
#define PMIC_LDO24_CTRL0_ldo24_bw_set_START (4)
#define PMIC_LDO24_CTRL0_ldo24_bw_set_END (6)
#define PMIC_LDO24_CTRL0_ldo24_en_bw_START (7)
#define PMIC_LDO24_CTRL0_ldo24_en_bw_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo24_eco_set : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_LDO24_CTRL1_UNION;
#endif
#define PMIC_LDO24_CTRL1_ldo24_eco_set_START (0)
#define PMIC_LDO24_CTRL1_ldo24_eco_set_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo25_vrset : 3;
        unsigned char ldo25_ocp_enn : 1;
        unsigned char ldo25_ocp_set : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO25_CTRL0_UNION;
#endif
#define PMIC_LDO25_CTRL0_ldo25_vrset_START (0)
#define PMIC_LDO25_CTRL0_ldo25_vrset_END (2)
#define PMIC_LDO25_CTRL0_ldo25_ocp_enn_START (3)
#define PMIC_LDO25_CTRL0_ldo25_ocp_enn_END (3)
#define PMIC_LDO25_CTRL0_ldo25_ocp_set_START (4)
#define PMIC_LDO25_CTRL0_ldo25_ocp_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo26_vrset : 3;
        unsigned char ldo26_ocp_enn : 1;
        unsigned char ldo26_i_sst : 1;
        unsigned char ldo26_eco_set : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO26_CTRL_UNION;
#endif
#define PMIC_LDO26_CTRL_ldo26_vrset_START (0)
#define PMIC_LDO26_CTRL_ldo26_vrset_END (2)
#define PMIC_LDO26_CTRL_ldo26_ocp_enn_START (3)
#define PMIC_LDO26_CTRL_ldo26_ocp_enn_END (3)
#define PMIC_LDO26_CTRL_ldo26_i_sst_START (4)
#define PMIC_LDO26_CTRL_ldo26_i_sst_END (4)
#define PMIC_LDO26_CTRL_ldo26_eco_set_START (5)
#define PMIC_LDO26_CTRL_ldo26_eco_set_END (5)
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
        unsigned char ldo28_vrset : 3;
        unsigned char ldo28_ocp_enn : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO28_CTRL_UNION;
#endif
#define PMIC_LDO28_CTRL_ldo28_vrset_START (0)
#define PMIC_LDO28_CTRL_ldo28_vrset_END (2)
#define PMIC_LDO28_CTRL_ldo28_ocp_enn_START (3)
#define PMIC_LDO28_CTRL_ldo28_ocp_enn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo29_vrset : 3;
        unsigned char ldo29_ocp_enn : 1;
        unsigned char ldo29_eco_set : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO29_CTRL0_UNION;
#endif
#define PMIC_LDO29_CTRL0_ldo29_vrset_START (0)
#define PMIC_LDO29_CTRL0_ldo29_vrset_END (2)
#define PMIC_LDO29_CTRL0_ldo29_ocp_enn_START (3)
#define PMIC_LDO29_CTRL0_ldo29_ocp_enn_END (3)
#define PMIC_LDO29_CTRL0_ldo29_eco_set_START (4)
#define PMIC_LDO29_CTRL0_ldo29_eco_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo29_ocps_en : 1;
        unsigned char ldo29_bw_en : 1;
        unsigned char ldo29_vgpr_en : 1;
        unsigned char ldo29_ocpd_set : 2;
        unsigned char ldo29_ocps_set : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO29_CTRL1_UNION;
#endif
#define PMIC_LDO29_CTRL1_ldo29_ocps_en_START (0)
#define PMIC_LDO29_CTRL1_ldo29_ocps_en_END (0)
#define PMIC_LDO29_CTRL1_ldo29_bw_en_START (1)
#define PMIC_LDO29_CTRL1_ldo29_bw_en_END (1)
#define PMIC_LDO29_CTRL1_ldo29_vgpr_en_START (2)
#define PMIC_LDO29_CTRL1_ldo29_vgpr_en_END (2)
#define PMIC_LDO29_CTRL1_ldo29_ocpd_set_START (3)
#define PMIC_LDO29_CTRL1_ldo29_ocpd_set_END (4)
#define PMIC_LDO29_CTRL1_ldo29_ocps_set_START (5)
#define PMIC_LDO29_CTRL1_ldo29_ocps_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo30_2_vrset : 3;
        unsigned char ldo30_2_ocp_enn : 1;
        unsigned char ldo30_2_eco_set : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO30_CTRL0_UNION;
#endif
#define PMIC_LDO30_CTRL0_ldo30_2_vrset_START (0)
#define PMIC_LDO30_CTRL0_ldo30_2_vrset_END (2)
#define PMIC_LDO30_CTRL0_ldo30_2_ocp_enn_START (3)
#define PMIC_LDO30_CTRL0_ldo30_2_ocp_enn_END (3)
#define PMIC_LDO30_CTRL0_ldo30_2_eco_set_START (4)
#define PMIC_LDO30_CTRL0_ldo30_2_eco_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo30_2_ocps_en : 1;
        unsigned char ldo30_2_vgpr_en : 1;
        unsigned char ldo30_2_ocpd_set : 2;
        unsigned char ldo30_2_ocps_set : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO30_2_CTRL1_UNION;
#endif
#define PMIC_LDO30_2_CTRL1_ldo30_2_ocps_en_START (0)
#define PMIC_LDO30_2_CTRL1_ldo30_2_ocps_en_END (0)
#define PMIC_LDO30_2_CTRL1_ldo30_2_vgpr_en_START (1)
#define PMIC_LDO30_2_CTRL1_ldo30_2_vgpr_en_END (1)
#define PMIC_LDO30_2_CTRL1_ldo30_2_ocpd_set_START (2)
#define PMIC_LDO30_2_CTRL1_ldo30_2_ocpd_set_END (3)
#define PMIC_LDO30_2_CTRL1_ldo30_2_ocps_set_START (4)
#define PMIC_LDO30_2_CTRL1_ldo30_2_ocps_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo31_vrset : 3;
        unsigned char ldo31_ocp_enn : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO31_CTRL_UNION;
#endif
#define PMIC_LDO31_CTRL_ldo31_vrset_START (0)
#define PMIC_LDO31_CTRL_ldo31_vrset_END (2)
#define PMIC_LDO31_CTRL_ldo31_ocp_enn_START (3)
#define PMIC_LDO31_CTRL_ldo31_ocp_enn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo32_vrset : 3;
        unsigned char ldo32_ocp_enn : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO32_CTRL0_UNION;
#endif
#define PMIC_LDO32_CTRL0_ldo32_vrset_START (0)
#define PMIC_LDO32_CTRL0_ldo32_vrset_END (2)
#define PMIC_LDO32_CTRL0_ldo32_ocp_enn_START (3)
#define PMIC_LDO32_CTRL0_ldo32_ocp_enn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo32_psrr2_set : 3;
        unsigned char ldo32_psrr1_set : 3;
        unsigned char ldo32_psrr_en : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO32_CTRL1_UNION;
#endif
#define PMIC_LDO32_CTRL1_ldo32_psrr2_set_START (0)
#define PMIC_LDO32_CTRL1_ldo32_psrr2_set_END (2)
#define PMIC_LDO32_CTRL1_ldo32_psrr1_set_START (3)
#define PMIC_LDO32_CTRL1_ldo32_psrr1_set_END (5)
#define PMIC_LDO32_CTRL1_ldo32_psrr_en_START (6)
#define PMIC_LDO32_CTRL1_ldo32_psrr_en_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo32_ocps_en : 1;
        unsigned char ldo32_bw_en : 1;
        unsigned char ldo32_vgpr_en : 1;
        unsigned char ldo32_comp : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO32_CTRL2_UNION;
#endif
#define PMIC_LDO32_CTRL2_ldo32_ocps_en_START (0)
#define PMIC_LDO32_CTRL2_ldo32_ocps_en_END (0)
#define PMIC_LDO32_CTRL2_ldo32_bw_en_START (1)
#define PMIC_LDO32_CTRL2_ldo32_bw_en_END (1)
#define PMIC_LDO32_CTRL2_ldo32_vgpr_en_START (2)
#define PMIC_LDO32_CTRL2_ldo32_vgpr_en_END (2)
#define PMIC_LDO32_CTRL2_ldo32_comp_START (3)
#define PMIC_LDO32_CTRL2_ldo32_comp_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo32_ocpd_set : 4;
        unsigned char ldo32_ocps_set : 4;
    } reg;
} PMIC_LDO32_CTRL3_UNION;
#endif
#define PMIC_LDO32_CTRL3_ldo32_ocpd_set_START (0)
#define PMIC_LDO32_CTRL3_ldo32_ocpd_set_END (3)
#define PMIC_LDO32_CTRL3_ldo32_ocps_set_START (4)
#define PMIC_LDO32_CTRL3_ldo32_ocps_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo33_vrset : 3;
        unsigned char ldo33_ocp_enn : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO33_CTRL0_UNION;
#endif
#define PMIC_LDO33_CTRL0_ldo33_vrset_START (0)
#define PMIC_LDO33_CTRL0_ldo33_vrset_END (2)
#define PMIC_LDO33_CTRL0_ldo33_ocp_enn_START (3)
#define PMIC_LDO33_CTRL0_ldo33_ocp_enn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo34_vrset : 3;
        unsigned char ldo34_ocp_enn : 1;
        unsigned char ldo34_eco_set : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO34_CTRL0_UNION;
#endif
#define PMIC_LDO34_CTRL0_ldo34_vrset_START (0)
#define PMIC_LDO34_CTRL0_ldo34_vrset_END (2)
#define PMIC_LDO34_CTRL0_ldo34_ocp_enn_START (3)
#define PMIC_LDO34_CTRL0_ldo34_ocp_enn_END (3)
#define PMIC_LDO34_CTRL0_ldo34_eco_set_START (4)
#define PMIC_LDO34_CTRL0_ldo34_eco_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo34_bw_set : 3;
        unsigned char ldo34_en_bw : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO34_CTRL1_UNION;
#endif
#define PMIC_LDO34_CTRL1_ldo34_bw_set_START (0)
#define PMIC_LDO34_CTRL1_ldo34_bw_set_END (2)
#define PMIC_LDO34_CTRL1_ldo34_en_bw_START (3)
#define PMIC_LDO34_CTRL1_ldo34_en_bw_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pmuh_vrset : 3;
        unsigned char pmuh_ocp_enn : 1;
        unsigned char pmuh_eco_set : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_PMUH_CTRL_UNION;
#endif
#define PMIC_PMUH_CTRL_pmuh_vrset_START (0)
#define PMIC_PMUH_CTRL_pmuh_vrset_END (2)
#define PMIC_PMUH_CTRL_pmuh_ocp_enn_START (3)
#define PMIC_PMUH_CTRL_pmuh_ocp_enn_END (3)
#define PMIC_PMUH_CTRL_pmuh_eco_set_START (4)
#define PMIC_PMUH_CTRL_pmuh_eco_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo37_vrset : 3;
        unsigned char ldo37_ocp_enn : 1;
        unsigned char ldo37_eco_set : 1;
        unsigned char ldo37_sst_set : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO37_CTRL0_UNION;
#endif
#define PMIC_LDO37_CTRL0_ldo37_vrset_START (0)
#define PMIC_LDO37_CTRL0_ldo37_vrset_END (2)
#define PMIC_LDO37_CTRL0_ldo37_ocp_enn_START (3)
#define PMIC_LDO37_CTRL0_ldo37_ocp_enn_END (3)
#define PMIC_LDO37_CTRL0_ldo37_eco_set_START (4)
#define PMIC_LDO37_CTRL0_ldo37_eco_set_END (4)
#define PMIC_LDO37_CTRL0_ldo37_sst_set_START (5)
#define PMIC_LDO37_CTRL0_ldo37_sst_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo37_ref_buf_bypass : 1;
        unsigned char ldo37_psrr2_set : 3;
        unsigned char ldo37_psrr1_set : 3;
        unsigned char ldo37_psrr_en : 1;
    } reg;
} PMIC_LDO37_CTRL1_UNION;
#endif
#define PMIC_LDO37_CTRL1_ldo37_ref_buf_bypass_START (0)
#define PMIC_LDO37_CTRL1_ldo37_ref_buf_bypass_END (0)
#define PMIC_LDO37_CTRL1_ldo37_psrr2_set_START (1)
#define PMIC_LDO37_CTRL1_ldo37_psrr2_set_END (3)
#define PMIC_LDO37_CTRL1_ldo37_psrr1_set_START (4)
#define PMIC_LDO37_CTRL1_ldo37_psrr1_set_END (6)
#define PMIC_LDO37_CTRL1_ldo37_psrr_en_START (7)
#define PMIC_LDO37_CTRL1_ldo37_psrr_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo37_ocps_en : 1;
        unsigned char ldo37_bw_en : 1;
        unsigned char ldo37_vgpr_en : 1;
        unsigned char ldo37_ref_buf_rset : 2;
        unsigned char ldo37_comp : 3;
    } reg;
} PMIC_LDO37_CTRL2_UNION;
#endif
#define PMIC_LDO37_CTRL2_ldo37_ocps_en_START (0)
#define PMIC_LDO37_CTRL2_ldo37_ocps_en_END (0)
#define PMIC_LDO37_CTRL2_ldo37_bw_en_START (1)
#define PMIC_LDO37_CTRL2_ldo37_bw_en_END (1)
#define PMIC_LDO37_CTRL2_ldo37_vgpr_en_START (2)
#define PMIC_LDO37_CTRL2_ldo37_vgpr_en_END (2)
#define PMIC_LDO37_CTRL2_ldo37_ref_buf_rset_START (3)
#define PMIC_LDO37_CTRL2_ldo37_ref_buf_rset_END (4)
#define PMIC_LDO37_CTRL2_ldo37_comp_START (5)
#define PMIC_LDO37_CTRL2_ldo37_comp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo37_ocpd_set : 4;
        unsigned char ldo37_ocps_set : 4;
    } reg;
} PMIC_LDO37_CTRL3_UNION;
#endif
#define PMIC_LDO37_CTRL3_ldo37_ocpd_set_START (0)
#define PMIC_LDO37_CTRL3_ldo37_ocpd_set_END (3)
#define PMIC_LDO37_CTRL3_ldo37_ocps_set_START (4)
#define PMIC_LDO37_CTRL3_ldo37_ocps_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo38_vrset : 3;
        unsigned char ldo38_ocp_enn : 1;
        unsigned char ldo38_eco_set : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO38_CTRL0_UNION;
#endif
#define PMIC_LDO38_CTRL0_ldo38_vrset_START (0)
#define PMIC_LDO38_CTRL0_ldo38_vrset_END (2)
#define PMIC_LDO38_CTRL0_ldo38_ocp_enn_START (3)
#define PMIC_LDO38_CTRL0_ldo38_ocp_enn_END (3)
#define PMIC_LDO38_CTRL0_ldo38_eco_set_START (4)
#define PMIC_LDO38_CTRL0_ldo38_eco_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo38_ocps_en : 1;
        unsigned char ldo38_bw_en : 1;
        unsigned char ldo38_vgpr_en : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO38_CTRL1_UNION;
#endif
#define PMIC_LDO38_CTRL1_ldo38_ocps_en_START (0)
#define PMIC_LDO38_CTRL1_ldo38_ocps_en_END (0)
#define PMIC_LDO38_CTRL1_ldo38_bw_en_START (1)
#define PMIC_LDO38_CTRL1_ldo38_bw_en_END (1)
#define PMIC_LDO38_CTRL1_ldo38_vgpr_en_START (2)
#define PMIC_LDO38_CTRL1_ldo38_vgpr_en_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo38_ocpd_set : 2;
        unsigned char ldo38_ocps_set : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO38_CTRL2_UNION;
#endif
#define PMIC_LDO38_CTRL2_ldo38_ocpd_set_START (0)
#define PMIC_LDO38_CTRL2_ldo38_ocpd_set_END (1)
#define PMIC_LDO38_CTRL2_ldo38_ocps_set_START (2)
#define PMIC_LDO38_CTRL2_ldo38_ocps_set_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo39_vrset : 3;
        unsigned char ldo39_ocp_enn : 1;
        unsigned char ldo39_eco_set : 1;
        unsigned char ldo39_vea_ctrl : 1;
        unsigned char ldo39_stb_set : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO39_CTRL0_UNION;
#endif
#define PMIC_LDO39_CTRL0_ldo39_vrset_START (0)
#define PMIC_LDO39_CTRL0_ldo39_vrset_END (2)
#define PMIC_LDO39_CTRL0_ldo39_ocp_enn_START (3)
#define PMIC_LDO39_CTRL0_ldo39_ocp_enn_END (3)
#define PMIC_LDO39_CTRL0_ldo39_eco_set_START (4)
#define PMIC_LDO39_CTRL0_ldo39_eco_set_END (4)
#define PMIC_LDO39_CTRL0_ldo39_vea_ctrl_START (5)
#define PMIC_LDO39_CTRL0_ldo39_vea_ctrl_END (5)
#define PMIC_LDO39_CTRL0_ldo39_stb_set_START (6)
#define PMIC_LDO39_CTRL0_ldo39_stb_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 1;
        unsigned char ldo39_psrr2_set : 3;
        unsigned char ldo39_psrr1_set : 3;
        unsigned char ldo39_psrr_en : 1;
    } reg;
} PMIC_LDO39_CTRL1_UNION;
#endif
#define PMIC_LDO39_CTRL1_ldo39_psrr2_set_START (1)
#define PMIC_LDO39_CTRL1_ldo39_psrr2_set_END (3)
#define PMIC_LDO39_CTRL1_ldo39_psrr1_set_START (4)
#define PMIC_LDO39_CTRL1_ldo39_psrr1_set_END (6)
#define PMIC_LDO39_CTRL1_ldo39_psrr_en_START (7)
#define PMIC_LDO39_CTRL1_ldo39_psrr_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo39_ocps_en : 1;
        unsigned char ldo39_bw_en : 1;
        unsigned char ldo39_vgpr_en : 1;
        unsigned char reserved : 2;
        unsigned char ldo39_comp : 3;
    } reg;
} PMIC_LDO39_CTRL2_UNION;
#endif
#define PMIC_LDO39_CTRL2_ldo39_ocps_en_START (0)
#define PMIC_LDO39_CTRL2_ldo39_ocps_en_END (0)
#define PMIC_LDO39_CTRL2_ldo39_bw_en_START (1)
#define PMIC_LDO39_CTRL2_ldo39_bw_en_END (1)
#define PMIC_LDO39_CTRL2_ldo39_vgpr_en_START (2)
#define PMIC_LDO39_CTRL2_ldo39_vgpr_en_END (2)
#define PMIC_LDO39_CTRL2_ldo39_comp_START (5)
#define PMIC_LDO39_CTRL2_ldo39_comp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo39_ocpd_set : 2;
        unsigned char ldo39_ocps_set : 2;
        unsigned char ldo39_ocpd_vos_set : 1;
        unsigned char ldo39_sst_set : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO39_CTRL3_UNION;
#endif
#define PMIC_LDO39_CTRL3_ldo39_ocpd_set_START (0)
#define PMIC_LDO39_CTRL3_ldo39_ocpd_set_END (1)
#define PMIC_LDO39_CTRL3_ldo39_ocps_set_START (2)
#define PMIC_LDO39_CTRL3_ldo39_ocps_set_END (3)
#define PMIC_LDO39_CTRL3_ldo39_ocpd_vos_set_START (4)
#define PMIC_LDO39_CTRL3_ldo39_ocpd_vos_set_END (4)
#define PMIC_LDO39_CTRL3_ldo39_sst_set_START (5)
#define PMIC_LDO39_CTRL3_ldo39_sst_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo40_vrset : 3;
        unsigned char ldo40_ocp_enn : 1;
        unsigned char ldo40_eco_set : 1;
        unsigned char ldo40_vea_ctrl : 1;
        unsigned char ldo40_stb_set : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO40_CTRL0_UNION;
#endif
#define PMIC_LDO40_CTRL0_ldo40_vrset_START (0)
#define PMIC_LDO40_CTRL0_ldo40_vrset_END (2)
#define PMIC_LDO40_CTRL0_ldo40_ocp_enn_START (3)
#define PMIC_LDO40_CTRL0_ldo40_ocp_enn_END (3)
#define PMIC_LDO40_CTRL0_ldo40_eco_set_START (4)
#define PMIC_LDO40_CTRL0_ldo40_eco_set_END (4)
#define PMIC_LDO40_CTRL0_ldo40_vea_ctrl_START (5)
#define PMIC_LDO40_CTRL0_ldo40_vea_ctrl_END (5)
#define PMIC_LDO40_CTRL0_ldo40_stb_set_START (6)
#define PMIC_LDO40_CTRL0_ldo40_stb_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 1;
        unsigned char ldo40_psrr2_set : 3;
        unsigned char ldo40_psrr1_set : 3;
        unsigned char ldo40_psrr_en : 1;
    } reg;
} PMIC_LDO40_CTRL1_UNION;
#endif
#define PMIC_LDO40_CTRL1_ldo40_psrr2_set_START (1)
#define PMIC_LDO40_CTRL1_ldo40_psrr2_set_END (3)
#define PMIC_LDO40_CTRL1_ldo40_psrr1_set_START (4)
#define PMIC_LDO40_CTRL1_ldo40_psrr1_set_END (6)
#define PMIC_LDO40_CTRL1_ldo40_psrr_en_START (7)
#define PMIC_LDO40_CTRL1_ldo40_psrr_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo40_ocps_en : 1;
        unsigned char ldo40_bw_en : 1;
        unsigned char ldo40_vgpr_en : 1;
        unsigned char reserved : 2;
        unsigned char ldo40_comp : 3;
    } reg;
} PMIC_LDO40_CTRL2_UNION;
#endif
#define PMIC_LDO40_CTRL2_ldo40_ocps_en_START (0)
#define PMIC_LDO40_CTRL2_ldo40_ocps_en_END (0)
#define PMIC_LDO40_CTRL2_ldo40_bw_en_START (1)
#define PMIC_LDO40_CTRL2_ldo40_bw_en_END (1)
#define PMIC_LDO40_CTRL2_ldo40_vgpr_en_START (2)
#define PMIC_LDO40_CTRL2_ldo40_vgpr_en_END (2)
#define PMIC_LDO40_CTRL2_ldo40_comp_START (5)
#define PMIC_LDO40_CTRL2_ldo40_comp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo40_ocpd_set : 2;
        unsigned char ldo40_ocps_set : 2;
        unsigned char ldo40_pd_set : 1;
        unsigned char ldo40_ocpd_vos_set : 1;
        unsigned char ldo40_sst_set : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO40_CTRL3_UNION;
#endif
#define PMIC_LDO40_CTRL3_ldo40_ocpd_set_START (0)
#define PMIC_LDO40_CTRL3_ldo40_ocpd_set_END (1)
#define PMIC_LDO40_CTRL3_ldo40_ocps_set_START (2)
#define PMIC_LDO40_CTRL3_ldo40_ocps_set_END (3)
#define PMIC_LDO40_CTRL3_ldo40_pd_set_START (4)
#define PMIC_LDO40_CTRL3_ldo40_pd_set_END (4)
#define PMIC_LDO40_CTRL3_ldo40_ocpd_vos_set_START (5)
#define PMIC_LDO40_CTRL3_ldo40_ocpd_vos_set_END (5)
#define PMIC_LDO40_CTRL3_ldo40_sst_set_START (6)
#define PMIC_LDO40_CTRL3_ldo40_sst_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo_buff_curr_sel : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO_BUF_PMUA_CTRL_UNION;
#endif
#define PMIC_LDO_BUF_PMUA_CTRL_ldo_buff_curr_sel_START (0)
#define PMIC_LDO_BUF_PMUA_CTRL_ldo_buff_curr_sel_END (1)
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
        unsigned char ldo1_ocpd_set : 2;
        unsigned char ldo1_ocps_set : 2;
        unsigned char ldo1_ocpd_enn : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO1_CTRL3_UNION;
#endif
#define PMIC_LDO1_CTRL3_ldo1_ocpd_set_START (0)
#define PMIC_LDO1_CTRL3_ldo1_ocpd_set_END (1)
#define PMIC_LDO1_CTRL3_ldo1_ocps_set_START (2)
#define PMIC_LDO1_CTRL3_ldo1_ocps_set_END (3)
#define PMIC_LDO1_CTRL3_ldo1_ocpd_enn_START (4)
#define PMIC_LDO1_CTRL3_ldo1_ocpd_enn_END (4)
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
        unsigned char buck5_ocp_auto_stop : 2;
        unsigned char buck4_ocp_auto_stop : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_BUCK4_5_OCP_CTRL_UNION;
#endif
#define PMIC_BUCK4_5_OCP_CTRL_buck5_ocp_auto_stop_START (0)
#define PMIC_BUCK4_5_OCP_CTRL_buck5_ocp_auto_stop_END (1)
#define PMIC_BUCK4_5_OCP_CTRL_buck4_ocp_auto_stop_START (2)
#define PMIC_BUCK4_5_OCP_CTRL_buck4_ocp_auto_stop_END (3)
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
        unsigned char ldo6_ocp_auto_stop : 2;
        unsigned char ldo5_ocp_auto_stop : 2;
        unsigned char ldo4_ocp_auto_stop : 2;
    } reg;
} PMIC_LDO4_8_OCP_CTRL_UNION;
#endif
#define PMIC_LDO4_8_OCP_CTRL_ldo8_ocp_auto_stop_START (0)
#define PMIC_LDO4_8_OCP_CTRL_ldo8_ocp_auto_stop_END (1)
#define PMIC_LDO4_8_OCP_CTRL_ldo6_ocp_auto_stop_START (2)
#define PMIC_LDO4_8_OCP_CTRL_ldo6_ocp_auto_stop_END (3)
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
        unsigned char ldo13_ocp_auto_stop : 2;
        unsigned char ldo12_ocp_auto_stop : 2;
        unsigned char ldo11_ocp_auto_stop : 2;
        unsigned char ldo9_ocp_auto_stop : 2;
    } reg;
} PMIC_LDO9_13_OCP_CTRL_UNION;
#endif
#define PMIC_LDO9_13_OCP_CTRL_ldo13_ocp_auto_stop_START (0)
#define PMIC_LDO9_13_OCP_CTRL_ldo13_ocp_auto_stop_END (1)
#define PMIC_LDO9_13_OCP_CTRL_ldo12_ocp_auto_stop_START (2)
#define PMIC_LDO9_13_OCP_CTRL_ldo12_ocp_auto_stop_END (3)
#define PMIC_LDO9_13_OCP_CTRL_ldo11_ocp_auto_stop_START (4)
#define PMIC_LDO9_13_OCP_CTRL_ldo11_ocp_auto_stop_END (5)
#define PMIC_LDO9_13_OCP_CTRL_ldo9_ocp_auto_stop_START (6)
#define PMIC_LDO9_13_OCP_CTRL_ldo9_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo17_ocp_auto_stop : 2;
        unsigned char ldo16_ocp_auto_stop : 2;
        unsigned char ldo15_ocp_auto_stop : 2;
        unsigned char ldo14_ocp_auto_stop : 2;
    } reg;
} PMIC_LDO14_17_OCP_CTRL_UNION;
#endif
#define PMIC_LDO14_17_OCP_CTRL_ldo17_ocp_auto_stop_START (0)
#define PMIC_LDO14_17_OCP_CTRL_ldo17_ocp_auto_stop_END (1)
#define PMIC_LDO14_17_OCP_CTRL_ldo16_ocp_auto_stop_START (2)
#define PMIC_LDO14_17_OCP_CTRL_ldo16_ocp_auto_stop_END (3)
#define PMIC_LDO14_17_OCP_CTRL_ldo15_ocp_auto_stop_START (4)
#define PMIC_LDO14_17_OCP_CTRL_ldo15_ocp_auto_stop_END (5)
#define PMIC_LDO14_17_OCP_CTRL_ldo14_ocp_auto_stop_START (6)
#define PMIC_LDO14_17_OCP_CTRL_ldo14_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo21_ocp_auto_stop : 2;
        unsigned char ldo20_ocp_auto_stop : 2;
        unsigned char ldo19_ocp_auto_stop : 2;
        unsigned char ldo18_ocp_auto_stop : 2;
    } reg;
} PMIC_LDO18_21_OCP_CTRL_UNION;
#endif
#define PMIC_LDO18_21_OCP_CTRL_ldo21_ocp_auto_stop_START (0)
#define PMIC_LDO18_21_OCP_CTRL_ldo21_ocp_auto_stop_END (1)
#define PMIC_LDO18_21_OCP_CTRL_ldo20_ocp_auto_stop_START (2)
#define PMIC_LDO18_21_OCP_CTRL_ldo20_ocp_auto_stop_END (3)
#define PMIC_LDO18_21_OCP_CTRL_ldo19_ocp_auto_stop_START (4)
#define PMIC_LDO18_21_OCP_CTRL_ldo19_ocp_auto_stop_END (5)
#define PMIC_LDO18_21_OCP_CTRL_ldo18_ocp_auto_stop_START (6)
#define PMIC_LDO18_21_OCP_CTRL_ldo18_ocp_auto_stop_END (7)
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
        unsigned char ldo33_ocp_auto_stop : 2;
        unsigned char ldo32_ocp_auto_stop : 2;
        unsigned char ldo31_ocp_auto_stop : 2;
        unsigned char ldo30_ocp_auto_stop : 2;
    } reg;
} PMIC_LDO30_33_OCP_CTRL_UNION;
#endif
#define PMIC_LDO30_33_OCP_CTRL_ldo33_ocp_auto_stop_START (0)
#define PMIC_LDO30_33_OCP_CTRL_ldo33_ocp_auto_stop_END (1)
#define PMIC_LDO30_33_OCP_CTRL_ldo32_ocp_auto_stop_START (2)
#define PMIC_LDO30_33_OCP_CTRL_ldo32_ocp_auto_stop_END (3)
#define PMIC_LDO30_33_OCP_CTRL_ldo31_ocp_auto_stop_START (4)
#define PMIC_LDO30_33_OCP_CTRL_ldo31_ocp_auto_stop_END (5)
#define PMIC_LDO30_33_OCP_CTRL_ldo30_ocp_auto_stop_START (6)
#define PMIC_LDO30_33_OCP_CTRL_ldo30_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo39_ocp_auto_stop : 2;
        unsigned char ldo38_ocp_auto_stop : 2;
        unsigned char ldo37_ocp_auto_stop : 2;
        unsigned char ldo34_ocp_auto_stop : 2;
    } reg;
} PMIC_LDO34_39_OCP_CTRL_UNION;
#endif
#define PMIC_LDO34_39_OCP_CTRL_ldo39_ocp_auto_stop_START (0)
#define PMIC_LDO34_39_OCP_CTRL_ldo39_ocp_auto_stop_END (1)
#define PMIC_LDO34_39_OCP_CTRL_ldo38_ocp_auto_stop_START (2)
#define PMIC_LDO34_39_OCP_CTRL_ldo38_ocp_auto_stop_END (3)
#define PMIC_LDO34_39_OCP_CTRL_ldo37_ocp_auto_stop_START (4)
#define PMIC_LDO34_39_OCP_CTRL_ldo37_ocp_auto_stop_END (5)
#define PMIC_LDO34_39_OCP_CTRL_ldo34_ocp_auto_stop_START (6)
#define PMIC_LDO34_39_OCP_CTRL_ldo34_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo_buff_scp_auto_stop : 2;
        unsigned char classd_ocp_auto_stop : 2;
        unsigned char ldo40_ocp_auto_stop : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO_BUFF_SCP_CTRL_UNION;
#endif
#define PMIC_LDO_BUFF_SCP_CTRL_ldo_buff_scp_auto_stop_START (0)
#define PMIC_LDO_BUFF_SCP_CTRL_ldo_buff_scp_auto_stop_END (1)
#define PMIC_LDO_BUFF_SCP_CTRL_classd_ocp_auto_stop_START (2)
#define PMIC_LDO_BUFF_SCP_CTRL_classd_ocp_auto_stop_END (3)
#define PMIC_LDO_BUFF_SCP_CTRL_ldo40_ocp_auto_stop_START (4)
#define PMIC_LDO_BUFF_SCP_CTRL_ldo40_ocp_auto_stop_END (5)
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
        unsigned char buck5_scp_auto_stop : 2;
        unsigned char buck4_scp_auto_stop : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_BUCK4_5_SCP_CTRL_UNION;
#endif
#define PMIC_BUCK4_5_SCP_CTRL_buck5_scp_auto_stop_START (0)
#define PMIC_BUCK4_5_SCP_CTRL_buck5_scp_auto_stop_END (1)
#define PMIC_BUCK4_5_SCP_CTRL_buck4_scp_auto_stop_START (2)
#define PMIC_BUCK4_5_SCP_CTRL_buck4_scp_auto_stop_END (3)
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
        unsigned char ldo_buff_scp_deb_sel : 2;
        unsigned char discharge_ocp_deb_sel : 2;
        unsigned char classd_ocp_deb_sel : 2;
    } reg;
} PMIC_OCP_DEB_CTRL1_UNION;
#endif
#define PMIC_OCP_DEB_CTRL1_vin_ldoh_deb_sel_START (0)
#define PMIC_OCP_DEB_CTRL1_vin_ldoh_deb_sel_END (1)
#define PMIC_OCP_DEB_CTRL1_ldo_buff_scp_deb_sel_START (2)
#define PMIC_OCP_DEB_CTRL1_ldo_buff_scp_deb_sel_END (3)
#define PMIC_OCP_DEB_CTRL1_discharge_ocp_deb_sel_START (4)
#define PMIC_OCP_DEB_CTRL1_discharge_ocp_deb_sel_END (5)
#define PMIC_OCP_DEB_CTRL1_classd_ocp_deb_sel_START (6)
#define PMIC_OCP_DEB_CTRL1_classd_ocp_deb_sel_END (7)
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
        unsigned char vsys_drop_deb_sel : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_VSYS_DROP_DEB_CTRL_UNION;
#endif
#define PMIC_VSYS_DROP_DEB_CTRL_vsys_drop_deb_sel_START (0)
#define PMIC_VSYS_DROP_DEB_CTRL_vsys_drop_deb_sel_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char en_vin_ldoh_deb : 1;
        unsigned char en_ldo_buff_scp_deb : 1;
        unsigned char en_ldo_ocp_deb : 1;
        unsigned char en_buck_scp_deb : 1;
        unsigned char en_buck_ocp_deb : 1;
        unsigned char en_discharge_ocp_deb : 1;
        unsigned char en_ldo31_ocp_deb : 1;
        unsigned char en_classd_ocp_deb : 1;
    } reg;
} PMIC_OCP_SCP_ONOFF_UNION;
#endif
#define PMIC_OCP_SCP_ONOFF_en_vin_ldoh_deb_START (0)
#define PMIC_OCP_SCP_ONOFF_en_vin_ldoh_deb_END (0)
#define PMIC_OCP_SCP_ONOFF_en_ldo_buff_scp_deb_START (1)
#define PMIC_OCP_SCP_ONOFF_en_ldo_buff_scp_deb_END (1)
#define PMIC_OCP_SCP_ONOFF_en_ldo_ocp_deb_START (2)
#define PMIC_OCP_SCP_ONOFF_en_ldo_ocp_deb_END (2)
#define PMIC_OCP_SCP_ONOFF_en_buck_scp_deb_START (3)
#define PMIC_OCP_SCP_ONOFF_en_buck_scp_deb_END (3)
#define PMIC_OCP_SCP_ONOFF_en_buck_ocp_deb_START (4)
#define PMIC_OCP_SCP_ONOFF_en_buck_ocp_deb_END (4)
#define PMIC_OCP_SCP_ONOFF_en_discharge_ocp_deb_START (5)
#define PMIC_OCP_SCP_ONOFF_en_discharge_ocp_deb_END (5)
#define PMIC_OCP_SCP_ONOFF_en_ldo31_ocp_deb_START (6)
#define PMIC_OCP_SCP_ONOFF_en_ldo31_ocp_deb_END (6)
#define PMIC_OCP_SCP_ONOFF_en_classd_ocp_deb_START (7)
#define PMIC_OCP_SCP_ONOFF_en_classd_ocp_deb_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_abb_drv : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_CLK_ABB_CTRL0_UNION;
#endif
#define PMIC_CLK_ABB_CTRL0_xo_abb_drv_START (0)
#define PMIC_CLK_ABB_CTRL0_xo_abb_drv_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_wifi_drv : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_CLK_WIFI_CTRL0_UNION;
#endif
#define PMIC_CLK_WIFI_CTRL0_xo_wifi_drv_START (0)
#define PMIC_CLK_WIFI_CTRL0_xo_wifi_drv_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_nfc_drv : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_CLK_NFC_CTRL0_UNION;
#endif
#define PMIC_CLK_NFC_CTRL0_xo_nfc_drv_START (0)
#define PMIC_CLK_NFC_CTRL0_xo_nfc_drv_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_rf_drv : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_CLK_RF_CTRL0_UNION;
#endif
#define PMIC_CLK_RF_CTRL0_xo_rf_drv_START (0)
#define PMIC_CLK_RF_CTRL0_xo_rf_drv_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_sys_drv : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_CLK_USB_CTRL_UNION;
#endif
#define PMIC_CLK_USB_CTRL_xo_sys_drv_START (0)
#define PMIC_CLK_USB_CTRL_xo_sys_drv_END (1)
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
        unsigned char xo_ufs_drv : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_CLK_UFS_CTRL_UNION;
#endif
#define PMIC_CLK_UFS_CTRL_xo_ufs_drv_START (0)
#define PMIC_CLK_UFS_CTRL_xo_ufs_drv_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_ton_eco_shield : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_TON_ECO_SHIELD_CTRL_UNION;
#endif
#define PMIC_TON_ECO_SHIELD_CTRL_xo_ton_eco_shield_START (0)
#define PMIC_TON_ECO_SHIELD_CTRL_xo_ton_eco_shield_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_rf_ph_sel : 1;
        unsigned char xo_tri_cap : 1;
        unsigned char xo_delay_sel : 2;
        unsigned char xo_core_curr_sel : 2;
        unsigned char xo_eco_force_en : 1;
        unsigned char xo_eco_en_shield : 1;
    } reg;
} PMIC_CLK_TOP_CTRL0_UNION;
#endif
#define PMIC_CLK_TOP_CTRL0_xo_rf_ph_sel_START (0)
#define PMIC_CLK_TOP_CTRL0_xo_rf_ph_sel_END (0)
#define PMIC_CLK_TOP_CTRL0_xo_tri_cap_START (1)
#define PMIC_CLK_TOP_CTRL0_xo_tri_cap_END (1)
#define PMIC_CLK_TOP_CTRL0_xo_delay_sel_START (2)
#define PMIC_CLK_TOP_CTRL0_xo_delay_sel_END (3)
#define PMIC_CLK_TOP_CTRL0_xo_core_curr_sel_START (4)
#define PMIC_CLK_TOP_CTRL0_xo_core_curr_sel_END (5)
#define PMIC_CLK_TOP_CTRL0_xo_eco_force_en_START (6)
#define PMIC_CLK_TOP_CTRL0_xo_eco_force_en_END (6)
#define PMIC_CLK_TOP_CTRL0_xo_eco_en_shield_START (7)
#define PMIC_CLK_TOP_CTRL0_xo_eco_en_shield_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_reserve : 8;
    } reg;
} PMIC_CLK_TOP_CTRL1_UNION;
#endif
#define PMIC_CLK_TOP_CTRL1_xo_reserve_START (0)
#define PMIC_CLK_TOP_CTRL1_xo_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char thsd_tmp_set : 2;
        unsigned char reg_thsd_en : 1;
        unsigned char ref_out_sel : 2;
        unsigned char ref_ibias_trim_en : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_BG_THSD_CTRL0_UNION;
#endif
#define PMIC_BG_THSD_CTRL0_thsd_tmp_set_START (0)
#define PMIC_BG_THSD_CTRL0_thsd_tmp_set_END (1)
#define PMIC_BG_THSD_CTRL0_reg_thsd_en_START (2)
#define PMIC_BG_THSD_CTRL0_reg_thsd_en_END (2)
#define PMIC_BG_THSD_CTRL0_ref_out_sel_START (3)
#define PMIC_BG_THSD_CTRL0_ref_out_sel_END (4)
#define PMIC_BG_THSD_CTRL0_ref_ibias_trim_en_START (5)
#define PMIC_BG_THSD_CTRL0_ref_ibias_trim_en_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ref_res_sel : 1;
        unsigned char reserved_0 : 2;
        unsigned char ref_chop_sel : 1;
        unsigned char reserved_1 : 4;
    } reg;
} PMIC_BG_THSD_CTRL1_UNION;
#endif
#define PMIC_BG_THSD_CTRL1_ref_res_sel_START (0)
#define PMIC_BG_THSD_CTRL1_ref_res_sel_END (0)
#define PMIC_BG_THSD_CTRL1_ref_chop_sel_START (3)
#define PMIC_BG_THSD_CTRL1_ref_chop_sel_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ref_reserve0 : 8;
    } reg;
} PMIC_BG_TEST0_UNION;
#endif
#define PMIC_BG_TEST0_ref_reserve0_START (0)
#define PMIC_BG_TEST0_ref_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ref_reserve1 : 8;
    } reg;
} PMIC_BG_TEST1_UNION;
#endif
#define PMIC_BG_TEST1_ref_reserve1_START (0)
#define PMIC_BG_TEST1_ref_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo26_hd_mask : 1;
        unsigned char reg_ldo_buf_hd_mask : 1;
        unsigned char reg_eco_in_hd_mask : 1;
        unsigned char reg_ldo29_hd_mask : 1;
        unsigned char reg_sys_clk_hd_mask : 1;
        unsigned char reg_abb_clk_hd_mask : 1;
        unsigned char reg_wifi_clk_hd_mask : 1;
        unsigned char reg_nfc_clk_hd_mask : 1;
    } reg;
} PMIC_HARDWIRE_CTRL0_UNION;
#endif
#define PMIC_HARDWIRE_CTRL0_reg_ldo26_hd_mask_START (0)
#define PMIC_HARDWIRE_CTRL0_reg_ldo26_hd_mask_END (0)
#define PMIC_HARDWIRE_CTRL0_reg_ldo_buf_hd_mask_START (1)
#define PMIC_HARDWIRE_CTRL0_reg_ldo_buf_hd_mask_END (1)
#define PMIC_HARDWIRE_CTRL0_reg_eco_in_hd_mask_START (2)
#define PMIC_HARDWIRE_CTRL0_reg_eco_in_hd_mask_END (2)
#define PMIC_HARDWIRE_CTRL0_reg_ldo29_hd_mask_START (3)
#define PMIC_HARDWIRE_CTRL0_reg_ldo29_hd_mask_END (3)
#define PMIC_HARDWIRE_CTRL0_reg_sys_clk_hd_mask_START (4)
#define PMIC_HARDWIRE_CTRL0_reg_sys_clk_hd_mask_END (4)
#define PMIC_HARDWIRE_CTRL0_reg_abb_clk_hd_mask_START (5)
#define PMIC_HARDWIRE_CTRL0_reg_abb_clk_hd_mask_END (5)
#define PMIC_HARDWIRE_CTRL0_reg_wifi_clk_hd_mask_START (6)
#define PMIC_HARDWIRE_CTRL0_reg_wifi_clk_hd_mask_END (6)
#define PMIC_HARDWIRE_CTRL0_reg_nfc_clk_hd_mask_START (7)
#define PMIC_HARDWIRE_CTRL0_reg_nfc_clk_hd_mask_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_idle_hd_mask : 1;
        unsigned char reg_buck0_vset_hd_mask : 1;
        unsigned char reg_ldo0_2_vset_hd_mask : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_HARDWIRE_CTRL1_UNION;
#endif
#define PMIC_HARDWIRE_CTRL1_reg_idle_hd_mask_START (0)
#define PMIC_HARDWIRE_CTRL1_reg_idle_hd_mask_END (0)
#define PMIC_HARDWIRE_CTRL1_reg_buck0_vset_hd_mask_START (1)
#define PMIC_HARDWIRE_CTRL1_reg_buck0_vset_hd_mask_END (1)
#define PMIC_HARDWIRE_CTRL1_reg_ldo0_2_vset_hd_mask_START (2)
#define PMIC_HARDWIRE_CTRL1_reg_ldo0_2_vset_hd_mask_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pmu_idle_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_LOW_POWER_CFG_UNION;
#endif
#define PMIC_LOW_POWER_CFG_pmu_idle_en_START (0)
#define PMIC_LOW_POWER_CFG_pmu_idle_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_buck5_on : 1;
        unsigned char peri_en_buck4_on : 1;
        unsigned char peri_en_buck2_on : 1;
        unsigned char peri_en_buck0_on : 1;
        unsigned char peri_en_ldo8_on : 1;
        unsigned char peri_en_ldo6_on : 1;
        unsigned char peri_en_ldo5_on : 1;
        unsigned char peri_en_ldo0_2_on : 1;
    } reg;
} PMIC_PERI_CTRL0_UNION;
#endif
#define PMIC_PERI_CTRL0_peri_en_buck5_on_START (0)
#define PMIC_PERI_CTRL0_peri_en_buck5_on_END (0)
#define PMIC_PERI_CTRL0_peri_en_buck4_on_START (1)
#define PMIC_PERI_CTRL0_peri_en_buck4_on_END (1)
#define PMIC_PERI_CTRL0_peri_en_buck2_on_START (2)
#define PMIC_PERI_CTRL0_peri_en_buck2_on_END (2)
#define PMIC_PERI_CTRL0_peri_en_buck0_on_START (3)
#define PMIC_PERI_CTRL0_peri_en_buck0_on_END (3)
#define PMIC_PERI_CTRL0_peri_en_ldo8_on_START (4)
#define PMIC_PERI_CTRL0_peri_en_ldo8_on_END (4)
#define PMIC_PERI_CTRL0_peri_en_ldo6_on_START (5)
#define PMIC_PERI_CTRL0_peri_en_ldo6_on_END (5)
#define PMIC_PERI_CTRL0_peri_en_ldo5_on_START (6)
#define PMIC_PERI_CTRL0_peri_en_ldo5_on_END (6)
#define PMIC_PERI_CTRL0_peri_en_ldo0_2_on_START (7)
#define PMIC_PERI_CTRL0_peri_en_ldo0_2_on_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_ldo39_on : 1;
        unsigned char peri_en_ldo38_on : 1;
        unsigned char peri_en_ldo37_on : 1;
        unsigned char peri_en_ldo30_2_on : 1;
        unsigned char peri_en_ldo28_on : 1;
        unsigned char peri_en_ldo27_on : 1;
        unsigned char peri_en_ldo23_on : 1;
        unsigned char peri_en_ldo9_on : 1;
    } reg;
} PMIC_PERI_CTRL1_UNION;
#endif
#define PMIC_PERI_CTRL1_peri_en_ldo39_on_START (0)
#define PMIC_PERI_CTRL1_peri_en_ldo39_on_END (0)
#define PMIC_PERI_CTRL1_peri_en_ldo38_on_START (1)
#define PMIC_PERI_CTRL1_peri_en_ldo38_on_END (1)
#define PMIC_PERI_CTRL1_peri_en_ldo37_on_START (2)
#define PMIC_PERI_CTRL1_peri_en_ldo37_on_END (2)
#define PMIC_PERI_CTRL1_peri_en_ldo30_2_on_START (3)
#define PMIC_PERI_CTRL1_peri_en_ldo30_2_on_END (3)
#define PMIC_PERI_CTRL1_peri_en_ldo28_on_START (4)
#define PMIC_PERI_CTRL1_peri_en_ldo28_on_END (4)
#define PMIC_PERI_CTRL1_peri_en_ldo27_on_START (5)
#define PMIC_PERI_CTRL1_peri_en_ldo27_on_END (5)
#define PMIC_PERI_CTRL1_peri_en_ldo23_on_START (6)
#define PMIC_PERI_CTRL1_peri_en_ldo23_on_END (6)
#define PMIC_PERI_CTRL1_peri_en_ldo9_on_START (7)
#define PMIC_PERI_CTRL1_peri_en_ldo9_on_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_ldo2_eco : 1;
        unsigned char peri_en_ldo0_2_eco : 1;
        unsigned char peri_en_buck5_eco : 1;
        unsigned char peri_en_buck4_eco : 1;
        unsigned char peri_en_buck3_eco : 1;
        unsigned char peri_en_buck2_eco : 1;
        unsigned char peri_en_buck1_eco : 1;
        unsigned char peri_en_buck0_eco : 1;
    } reg;
} PMIC_PERI_CTRL2_UNION;
#endif
#define PMIC_PERI_CTRL2_peri_en_ldo2_eco_START (0)
#define PMIC_PERI_CTRL2_peri_en_ldo2_eco_END (0)
#define PMIC_PERI_CTRL2_peri_en_ldo0_2_eco_START (1)
#define PMIC_PERI_CTRL2_peri_en_ldo0_2_eco_END (1)
#define PMIC_PERI_CTRL2_peri_en_buck5_eco_START (2)
#define PMIC_PERI_CTRL2_peri_en_buck5_eco_END (2)
#define PMIC_PERI_CTRL2_peri_en_buck4_eco_START (3)
#define PMIC_PERI_CTRL2_peri_en_buck4_eco_END (3)
#define PMIC_PERI_CTRL2_peri_en_buck3_eco_START (4)
#define PMIC_PERI_CTRL2_peri_en_buck3_eco_END (4)
#define PMIC_PERI_CTRL2_peri_en_buck2_eco_START (5)
#define PMIC_PERI_CTRL2_peri_en_buck2_eco_END (5)
#define PMIC_PERI_CTRL2_peri_en_buck1_eco_START (6)
#define PMIC_PERI_CTRL2_peri_en_buck1_eco_END (6)
#define PMIC_PERI_CTRL2_peri_en_buck0_eco_START (7)
#define PMIC_PERI_CTRL2_peri_en_buck0_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_ldo18_eco : 1;
        unsigned char peri_en_ldo15_eco : 1;
        unsigned char peri_en_ldo12_eco : 1;
        unsigned char peri_en_ldo11_eco : 1;
        unsigned char peri_en_ldo9_eco : 1;
        unsigned char peri_en_ldo8_eco : 1;
        unsigned char peri_en_ldo6_eco : 1;
        unsigned char peri_en_ldo5_eco : 1;
    } reg;
} PMIC_PERI_CTRL3_UNION;
#endif
#define PMIC_PERI_CTRL3_peri_en_ldo18_eco_START (0)
#define PMIC_PERI_CTRL3_peri_en_ldo18_eco_END (0)
#define PMIC_PERI_CTRL3_peri_en_ldo15_eco_START (1)
#define PMIC_PERI_CTRL3_peri_en_ldo15_eco_END (1)
#define PMIC_PERI_CTRL3_peri_en_ldo12_eco_START (2)
#define PMIC_PERI_CTRL3_peri_en_ldo12_eco_END (2)
#define PMIC_PERI_CTRL3_peri_en_ldo11_eco_START (3)
#define PMIC_PERI_CTRL3_peri_en_ldo11_eco_END (3)
#define PMIC_PERI_CTRL3_peri_en_ldo9_eco_START (4)
#define PMIC_PERI_CTRL3_peri_en_ldo9_eco_END (4)
#define PMIC_PERI_CTRL3_peri_en_ldo8_eco_START (5)
#define PMIC_PERI_CTRL3_peri_en_ldo8_eco_END (5)
#define PMIC_PERI_CTRL3_peri_en_ldo6_eco_START (6)
#define PMIC_PERI_CTRL3_peri_en_ldo6_eco_END (6)
#define PMIC_PERI_CTRL3_peri_en_ldo5_eco_START (7)
#define PMIC_PERI_CTRL3_peri_en_ldo5_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_ldo38_eco : 1;
        unsigned char peri_en_ldo37_eco : 1;
        unsigned char peri_en_ldo34_eco : 1;
        unsigned char peri_en_ldo30_2_eco : 1;
        unsigned char peri_en_ldo29_eco : 1;
        unsigned char peri_en_ldo27_eco : 1;
        unsigned char peri_en_ldo24_eco : 1;
        unsigned char peri_en_ldo23_eco : 1;
    } reg;
} PMIC_PERI_CTRL4_UNION;
#endif
#define PMIC_PERI_CTRL4_peri_en_ldo38_eco_START (0)
#define PMIC_PERI_CTRL4_peri_en_ldo38_eco_END (0)
#define PMIC_PERI_CTRL4_peri_en_ldo37_eco_START (1)
#define PMIC_PERI_CTRL4_peri_en_ldo37_eco_END (1)
#define PMIC_PERI_CTRL4_peri_en_ldo34_eco_START (2)
#define PMIC_PERI_CTRL4_peri_en_ldo34_eco_END (2)
#define PMIC_PERI_CTRL4_peri_en_ldo30_2_eco_START (3)
#define PMIC_PERI_CTRL4_peri_en_ldo30_2_eco_END (3)
#define PMIC_PERI_CTRL4_peri_en_ldo29_eco_START (4)
#define PMIC_PERI_CTRL4_peri_en_ldo29_eco_END (4)
#define PMIC_PERI_CTRL4_peri_en_ldo27_eco_START (5)
#define PMIC_PERI_CTRL4_peri_en_ldo27_eco_END (5)
#define PMIC_PERI_CTRL4_peri_en_ldo24_eco_START (6)
#define PMIC_PERI_CTRL4_peri_en_ldo24_eco_END (6)
#define PMIC_PERI_CTRL4_peri_en_ldo23_eco_START (7)
#define PMIC_PERI_CTRL4_peri_en_ldo23_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_pmua_eco : 1;
        unsigned char peri_en_pmuh_eco : 1;
        unsigned char peri_en_ldo39_eco : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_PERI_CTRL5_UNION;
#endif
#define PMIC_PERI_CTRL5_peri_en_pmua_eco_START (0)
#define PMIC_PERI_CTRL5_peri_en_pmua_eco_END (0)
#define PMIC_PERI_CTRL5_peri_en_pmuh_eco_START (1)
#define PMIC_PERI_CTRL5_peri_en_pmuh_eco_END (1)
#define PMIC_PERI_CTRL5_peri_en_ldo39_eco_START (2)
#define PMIC_PERI_CTRL5_peri_en_ldo39_eco_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_cnt_cfg : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CNT_CFG_UNION;
#endif
#define PMIC_PERI_CNT_CFG_peri_cnt_cfg_START (0)
#define PMIC_PERI_CNT_CFG_peri_cnt_cfg_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_buck0_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL6_UNION;
#endif
#define PMIC_PERI_CTRL6_peri_buck0_sel_START (0)
#define PMIC_PERI_CTRL6_peri_buck0_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_buck2_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL7_UNION;
#endif
#define PMIC_PERI_CTRL7_peri_buck2_sel_START (0)
#define PMIC_PERI_CTRL7_peri_buck2_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_buck4_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL8_UNION;
#endif
#define PMIC_PERI_CTRL8_peri_buck4_sel_START (0)
#define PMIC_PERI_CTRL8_peri_buck4_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_buck5_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL9_UNION;
#endif
#define PMIC_PERI_CTRL9_peri_buck5_sel_START (0)
#define PMIC_PERI_CTRL9_peri_buck5_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo0_2_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL10_UNION;
#endif
#define PMIC_PERI_CTRL10_peri_ldo0_2_sel_START (0)
#define PMIC_PERI_CTRL10_peri_ldo0_2_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo5_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL11_UNION;
#endif
#define PMIC_PERI_CTRL11_peri_ldo5_sel_START (0)
#define PMIC_PERI_CTRL11_peri_ldo5_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo6_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL12_UNION;
#endif
#define PMIC_PERI_CTRL12_peri_ldo6_sel_START (0)
#define PMIC_PERI_CTRL12_peri_ldo6_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo8_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL13_UNION;
#endif
#define PMIC_PERI_CTRL13_peri_ldo8_sel_START (0)
#define PMIC_PERI_CTRL13_peri_ldo8_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo9_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL14_UNION;
#endif
#define PMIC_PERI_CTRL14_peri_ldo9_sel_START (0)
#define PMIC_PERI_CTRL14_peri_ldo9_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo23_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL15_UNION;
#endif
#define PMIC_PERI_CTRL15_peri_ldo23_sel_START (0)
#define PMIC_PERI_CTRL15_peri_ldo23_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo27_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL16_UNION;
#endif
#define PMIC_PERI_CTRL16_peri_ldo27_sel_START (0)
#define PMIC_PERI_CTRL16_peri_ldo27_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo28_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL17_UNION;
#endif
#define PMIC_PERI_CTRL17_peri_ldo28_sel_START (0)
#define PMIC_PERI_CTRL17_peri_ldo28_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo30_2_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL18_UNION;
#endif
#define PMIC_PERI_CTRL18_peri_ldo30_2_sel_START (0)
#define PMIC_PERI_CTRL18_peri_ldo30_2_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo37_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL19_UNION;
#endif
#define PMIC_PERI_CTRL19_peri_ldo37_sel_START (0)
#define PMIC_PERI_CTRL19_peri_ldo37_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo38_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL20_UNION;
#endif
#define PMIC_PERI_CTRL20_peri_ldo38_sel_START (0)
#define PMIC_PERI_CTRL20_peri_ldo38_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo39_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL21_UNION;
#endif
#define PMIC_PERI_CTRL21_peri_ldo39_sel_START (0)
#define PMIC_PERI_CTRL21_peri_ldo39_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char wifi_en_ldo27_eco : 1;
        unsigned char wifi_en_buck3_eco : 1;
        unsigned char peri_en_thsd_eco : 1;
        unsigned char idle_en_thsd_eco : 1;
        unsigned char soh_thsd_en : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_WIFI_CTRL_UNION;
#endif
#define PMIC_WIFI_CTRL_wifi_en_ldo27_eco_START (0)
#define PMIC_WIFI_CTRL_wifi_en_ldo27_eco_END (0)
#define PMIC_WIFI_CTRL_wifi_en_buck3_eco_START (1)
#define PMIC_WIFI_CTRL_wifi_en_buck3_eco_END (1)
#define PMIC_WIFI_CTRL_peri_en_thsd_eco_START (2)
#define PMIC_WIFI_CTRL_peri_en_thsd_eco_END (2)
#define PMIC_WIFI_CTRL_idle_en_thsd_eco_START (3)
#define PMIC_WIFI_CTRL_idle_en_thsd_eco_END (3)
#define PMIC_WIFI_CTRL_soh_thsd_en_START (4)
#define PMIC_WIFI_CTRL_soh_thsd_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_buck4_vset : 1;
        unsigned char peri_en_buck3_vset : 1;
        unsigned char peri_en_buck2_vset : 1;
        unsigned char peri_en_ldo2_vset : 1;
        unsigned char peri_en_ldo0_2_vset : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_VSET_CTRL_UNION;
#endif
#define PMIC_PERI_VSET_CTRL_peri_en_buck4_vset_START (0)
#define PMIC_PERI_VSET_CTRL_peri_en_buck4_vset_END (0)
#define PMIC_PERI_VSET_CTRL_peri_en_buck3_vset_START (1)
#define PMIC_PERI_VSET_CTRL_peri_en_buck3_vset_END (1)
#define PMIC_PERI_VSET_CTRL_peri_en_buck2_vset_START (2)
#define PMIC_PERI_VSET_CTRL_peri_en_buck2_vset_END (2)
#define PMIC_PERI_VSET_CTRL_peri_en_ldo2_vset_START (3)
#define PMIC_PERI_VSET_CTRL_peri_en_ldo2_vset_END (3)
#define PMIC_PERI_VSET_CTRL_peri_en_ldo0_2_vset_START (4)
#define PMIC_PERI_VSET_CTRL_peri_en_ldo0_2_vset_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_en_buck5_on : 1;
        unsigned char idle_en_buck4_on : 1;
        unsigned char idle_en_buck2_on : 1;
        unsigned char idle_en_buck0_on : 1;
        unsigned char idle_en_ldo8_on : 1;
        unsigned char idle_en_ldo6_on : 1;
        unsigned char idle_en_ldo5_on : 1;
        unsigned char idle_en_ldo0_2_on : 1;
    } reg;
} PMIC_IDLE_CTRL0_UNION;
#endif
#define PMIC_IDLE_CTRL0_idle_en_buck5_on_START (0)
#define PMIC_IDLE_CTRL0_idle_en_buck5_on_END (0)
#define PMIC_IDLE_CTRL0_idle_en_buck4_on_START (1)
#define PMIC_IDLE_CTRL0_idle_en_buck4_on_END (1)
#define PMIC_IDLE_CTRL0_idle_en_buck2_on_START (2)
#define PMIC_IDLE_CTRL0_idle_en_buck2_on_END (2)
#define PMIC_IDLE_CTRL0_idle_en_buck0_on_START (3)
#define PMIC_IDLE_CTRL0_idle_en_buck0_on_END (3)
#define PMIC_IDLE_CTRL0_idle_en_ldo8_on_START (4)
#define PMIC_IDLE_CTRL0_idle_en_ldo8_on_END (4)
#define PMIC_IDLE_CTRL0_idle_en_ldo6_on_START (5)
#define PMIC_IDLE_CTRL0_idle_en_ldo6_on_END (5)
#define PMIC_IDLE_CTRL0_idle_en_ldo5_on_START (6)
#define PMIC_IDLE_CTRL0_idle_en_ldo5_on_END (6)
#define PMIC_IDLE_CTRL0_idle_en_ldo0_2_on_START (7)
#define PMIC_IDLE_CTRL0_idle_en_ldo0_2_on_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_en_ldo39_on : 1;
        unsigned char idle_en_ldo38_on : 1;
        unsigned char idle_en_ldo37_on : 1;
        unsigned char idle_en_ldo30_2_on : 1;
        unsigned char idle_en_ldo28_on : 1;
        unsigned char idle_en_ldo27_on : 1;
        unsigned char idle_en_ldo23_on : 1;
        unsigned char idle_en_ldo9_on : 1;
    } reg;
} PMIC_IDLE_CTRL1_UNION;
#endif
#define PMIC_IDLE_CTRL1_idle_en_ldo39_on_START (0)
#define PMIC_IDLE_CTRL1_idle_en_ldo39_on_END (0)
#define PMIC_IDLE_CTRL1_idle_en_ldo38_on_START (1)
#define PMIC_IDLE_CTRL1_idle_en_ldo38_on_END (1)
#define PMIC_IDLE_CTRL1_idle_en_ldo37_on_START (2)
#define PMIC_IDLE_CTRL1_idle_en_ldo37_on_END (2)
#define PMIC_IDLE_CTRL1_idle_en_ldo30_2_on_START (3)
#define PMIC_IDLE_CTRL1_idle_en_ldo30_2_on_END (3)
#define PMIC_IDLE_CTRL1_idle_en_ldo28_on_START (4)
#define PMIC_IDLE_CTRL1_idle_en_ldo28_on_END (4)
#define PMIC_IDLE_CTRL1_idle_en_ldo27_on_START (5)
#define PMIC_IDLE_CTRL1_idle_en_ldo27_on_END (5)
#define PMIC_IDLE_CTRL1_idle_en_ldo23_on_START (6)
#define PMIC_IDLE_CTRL1_idle_en_ldo23_on_END (6)
#define PMIC_IDLE_CTRL1_idle_en_ldo9_on_START (7)
#define PMIC_IDLE_CTRL1_idle_en_ldo9_on_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_en_ldo2_eco : 1;
        unsigned char idle_en_ldo0_2_eco : 1;
        unsigned char idle_en_buck5_eco : 1;
        unsigned char idle_en_buck4_eco : 1;
        unsigned char idle_en_buck3_eco : 1;
        unsigned char idle_en_buck2_eco : 1;
        unsigned char idle_en_buck1_eco : 1;
        unsigned char idle_en_buck0_eco : 1;
    } reg;
} PMIC_IDLE_CTRL2_UNION;
#endif
#define PMIC_IDLE_CTRL2_idle_en_ldo2_eco_START (0)
#define PMIC_IDLE_CTRL2_idle_en_ldo2_eco_END (0)
#define PMIC_IDLE_CTRL2_idle_en_ldo0_2_eco_START (1)
#define PMIC_IDLE_CTRL2_idle_en_ldo0_2_eco_END (1)
#define PMIC_IDLE_CTRL2_idle_en_buck5_eco_START (2)
#define PMIC_IDLE_CTRL2_idle_en_buck5_eco_END (2)
#define PMIC_IDLE_CTRL2_idle_en_buck4_eco_START (3)
#define PMIC_IDLE_CTRL2_idle_en_buck4_eco_END (3)
#define PMIC_IDLE_CTRL2_idle_en_buck3_eco_START (4)
#define PMIC_IDLE_CTRL2_idle_en_buck3_eco_END (4)
#define PMIC_IDLE_CTRL2_idle_en_buck2_eco_START (5)
#define PMIC_IDLE_CTRL2_idle_en_buck2_eco_END (5)
#define PMIC_IDLE_CTRL2_idle_en_buck1_eco_START (6)
#define PMIC_IDLE_CTRL2_idle_en_buck1_eco_END (6)
#define PMIC_IDLE_CTRL2_idle_en_buck0_eco_START (7)
#define PMIC_IDLE_CTRL2_idle_en_buck0_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_en_ldo18_eco : 1;
        unsigned char idle_en_ldo15_eco : 1;
        unsigned char idle_en_ldo12_eco : 1;
        unsigned char idle_en_ldo11_eco : 1;
        unsigned char idle_en_ldo9_eco : 1;
        unsigned char idle_en_ldo8_eco : 1;
        unsigned char idle_en_ldo6_eco : 1;
        unsigned char idle_en_ldo5_eco : 1;
    } reg;
} PMIC_IDLE_CTRL3_UNION;
#endif
#define PMIC_IDLE_CTRL3_idle_en_ldo18_eco_START (0)
#define PMIC_IDLE_CTRL3_idle_en_ldo18_eco_END (0)
#define PMIC_IDLE_CTRL3_idle_en_ldo15_eco_START (1)
#define PMIC_IDLE_CTRL3_idle_en_ldo15_eco_END (1)
#define PMIC_IDLE_CTRL3_idle_en_ldo12_eco_START (2)
#define PMIC_IDLE_CTRL3_idle_en_ldo12_eco_END (2)
#define PMIC_IDLE_CTRL3_idle_en_ldo11_eco_START (3)
#define PMIC_IDLE_CTRL3_idle_en_ldo11_eco_END (3)
#define PMIC_IDLE_CTRL3_idle_en_ldo9_eco_START (4)
#define PMIC_IDLE_CTRL3_idle_en_ldo9_eco_END (4)
#define PMIC_IDLE_CTRL3_idle_en_ldo8_eco_START (5)
#define PMIC_IDLE_CTRL3_idle_en_ldo8_eco_END (5)
#define PMIC_IDLE_CTRL3_idle_en_ldo6_eco_START (6)
#define PMIC_IDLE_CTRL3_idle_en_ldo6_eco_END (6)
#define PMIC_IDLE_CTRL3_idle_en_ldo5_eco_START (7)
#define PMIC_IDLE_CTRL3_idle_en_ldo5_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_en_ldo38_eco : 1;
        unsigned char idle_en_ldo37_eco : 1;
        unsigned char idle_en_ldo34_eco : 1;
        unsigned char idle_en_ldo30_2_eco : 1;
        unsigned char idle_en_ldo29_eco : 1;
        unsigned char idle_en_ldo27_eco : 1;
        unsigned char idle_en_ldo24_eco : 1;
        unsigned char idle_en_ldo23_eco : 1;
    } reg;
} PMIC_IDLE_CTRL4_UNION;
#endif
#define PMIC_IDLE_CTRL4_idle_en_ldo38_eco_START (0)
#define PMIC_IDLE_CTRL4_idle_en_ldo38_eco_END (0)
#define PMIC_IDLE_CTRL4_idle_en_ldo37_eco_START (1)
#define PMIC_IDLE_CTRL4_idle_en_ldo37_eco_END (1)
#define PMIC_IDLE_CTRL4_idle_en_ldo34_eco_START (2)
#define PMIC_IDLE_CTRL4_idle_en_ldo34_eco_END (2)
#define PMIC_IDLE_CTRL4_idle_en_ldo30_2_eco_START (3)
#define PMIC_IDLE_CTRL4_idle_en_ldo30_2_eco_END (3)
#define PMIC_IDLE_CTRL4_idle_en_ldo29_eco_START (4)
#define PMIC_IDLE_CTRL4_idle_en_ldo29_eco_END (4)
#define PMIC_IDLE_CTRL4_idle_en_ldo27_eco_START (5)
#define PMIC_IDLE_CTRL4_idle_en_ldo27_eco_END (5)
#define PMIC_IDLE_CTRL4_idle_en_ldo24_eco_START (6)
#define PMIC_IDLE_CTRL4_idle_en_ldo24_eco_END (6)
#define PMIC_IDLE_CTRL4_idle_en_ldo23_eco_START (7)
#define PMIC_IDLE_CTRL4_idle_en_ldo23_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_en_pmua_eco : 1;
        unsigned char idle_en_pmuh_eco : 1;
        unsigned char idle_en_ldo39_eco : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_IDLE_CTRL5_UNION;
#endif
#define PMIC_IDLE_CTRL5_idle_en_pmua_eco_START (0)
#define PMIC_IDLE_CTRL5_idle_en_pmua_eco_END (0)
#define PMIC_IDLE_CTRL5_idle_en_pmuh_eco_START (1)
#define PMIC_IDLE_CTRL5_idle_en_pmuh_eco_END (1)
#define PMIC_IDLE_CTRL5_idle_en_ldo39_eco_START (2)
#define PMIC_IDLE_CTRL5_idle_en_ldo39_eco_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_cnt_cfg : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CNT_CFG_UNION;
#endif
#define PMIC_IDLE_CNT_CFG_idle_cnt_cfg_START (0)
#define PMIC_IDLE_CNT_CFG_idle_cnt_cfg_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_buck0_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL6_UNION;
#endif
#define PMIC_IDLE_CTRL6_idle_buck0_sel_START (0)
#define PMIC_IDLE_CTRL6_idle_buck0_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_buck2_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL7_UNION;
#endif
#define PMIC_IDLE_CTRL7_idle_buck2_sel_START (0)
#define PMIC_IDLE_CTRL7_idle_buck2_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_buck4_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL8_UNION;
#endif
#define PMIC_IDLE_CTRL8_idle_buck4_sel_START (0)
#define PMIC_IDLE_CTRL8_idle_buck4_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_buck5_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL9_UNION;
#endif
#define PMIC_IDLE_CTRL9_idle_buck5_sel_START (0)
#define PMIC_IDLE_CTRL9_idle_buck5_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo0_2_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL10_UNION;
#endif
#define PMIC_IDLE_CTRL10_idle_ldo0_2_sel_START (0)
#define PMIC_IDLE_CTRL10_idle_ldo0_2_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo5_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL11_UNION;
#endif
#define PMIC_IDLE_CTRL11_idle_ldo5_sel_START (0)
#define PMIC_IDLE_CTRL11_idle_ldo5_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo6_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL12_UNION;
#endif
#define PMIC_IDLE_CTRL12_idle_ldo6_sel_START (0)
#define PMIC_IDLE_CTRL12_idle_ldo6_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo8_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL13_UNION;
#endif
#define PMIC_IDLE_CTRL13_idle_ldo8_sel_START (0)
#define PMIC_IDLE_CTRL13_idle_ldo8_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo9_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL14_UNION;
#endif
#define PMIC_IDLE_CTRL14_idle_ldo9_sel_START (0)
#define PMIC_IDLE_CTRL14_idle_ldo9_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo23_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL15_UNION;
#endif
#define PMIC_IDLE_CTRL15_idle_ldo23_sel_START (0)
#define PMIC_IDLE_CTRL15_idle_ldo23_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo27_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL16_UNION;
#endif
#define PMIC_IDLE_CTRL16_idle_ldo27_sel_START (0)
#define PMIC_IDLE_CTRL16_idle_ldo27_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo28_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL17_UNION;
#endif
#define PMIC_IDLE_CTRL17_idle_ldo28_sel_START (0)
#define PMIC_IDLE_CTRL17_idle_ldo28_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo30_2_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL18_UNION;
#endif
#define PMIC_IDLE_CTRL18_idle_ldo30_2_sel_START (0)
#define PMIC_IDLE_CTRL18_idle_ldo30_2_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo37_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL19_UNION;
#endif
#define PMIC_IDLE_CTRL19_idle_ldo37_sel_START (0)
#define PMIC_IDLE_CTRL19_idle_ldo37_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo38_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL20_UNION;
#endif
#define PMIC_IDLE_CTRL20_idle_ldo38_sel_START (0)
#define PMIC_IDLE_CTRL20_idle_ldo38_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo39_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL21_UNION;
#endif
#define PMIC_IDLE_CTRL21_idle_ldo39_sel_START (0)
#define PMIC_IDLE_CTRL21_idle_ldo39_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_en_buck4_vset : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_IDLE_VSET_CFG_UNION;
#endif
#define PMIC_IDLE_VSET_CFG_idle_en_buck4_vset_START (0)
#define PMIC_IDLE_VSET_CFG_idle_en_buck4_vset_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char coul_codec_clk_en_mask : 1;
        unsigned char coul_nfc_clk_en_mask : 1;
        unsigned char coul_wifi_clk_en_mask : 1;
        unsigned char coul_sys_clk_en_mask : 1;
        unsigned char reg_coul_idle_mask : 1;
        unsigned char reg_coul_eco_mask : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_COUL_ECO_MASK_UNION;
#endif
#define PMIC_COUL_ECO_MASK_coul_codec_clk_en_mask_START (0)
#define PMIC_COUL_ECO_MASK_coul_codec_clk_en_mask_END (0)
#define PMIC_COUL_ECO_MASK_coul_nfc_clk_en_mask_START (1)
#define PMIC_COUL_ECO_MASK_coul_nfc_clk_en_mask_END (1)
#define PMIC_COUL_ECO_MASK_coul_wifi_clk_en_mask_START (2)
#define PMIC_COUL_ECO_MASK_coul_wifi_clk_en_mask_END (2)
#define PMIC_COUL_ECO_MASK_coul_sys_clk_en_mask_START (3)
#define PMIC_COUL_ECO_MASK_coul_sys_clk_en_mask_END (3)
#define PMIC_COUL_ECO_MASK_reg_coul_idle_mask_START (4)
#define PMIC_COUL_ECO_MASK_reg_coul_idle_mask_END (4)
#define PMIC_COUL_ECO_MASK_reg_coul_eco_mask_START (5)
#define PMIC_COUL_ECO_MASK_reg_coul_eco_mask_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_codec_clk_en_mask : 1;
        unsigned char dcr_nfc_clk_en_mask : 1;
        unsigned char dcr_wifi_clk_en_mask : 1;
        unsigned char dcr_sys_clk_en_mask : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_SOH_ECO_MASK_UNION;
#endif
#define PMIC_SOH_ECO_MASK_dcr_codec_clk_en_mask_START (0)
#define PMIC_SOH_ECO_MASK_dcr_codec_clk_en_mask_END (0)
#define PMIC_SOH_ECO_MASK_dcr_nfc_clk_en_mask_START (1)
#define PMIC_SOH_ECO_MASK_dcr_nfc_clk_en_mask_END (1)
#define PMIC_SOH_ECO_MASK_dcr_wifi_clk_en_mask_START (2)
#define PMIC_SOH_ECO_MASK_dcr_wifi_clk_en_mask_END (2)
#define PMIC_SOH_ECO_MASK_dcr_sys_clk_en_mask_START (3)
#define PMIC_SOH_ECO_MASK_dcr_sys_clk_en_mask_END (3)
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
        unsigned char classd_erm_ctrl : 8;
    } reg;
} PMIC_CLASSD_CTRL3_UNION;
#endif
#define PMIC_CLASSD_CTRL3_classd_erm_ctrl_START (0)
#define PMIC_CLASSD_CTRL3_classd_erm_ctrl_END (7)
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
        unsigned char lock : 8;
    } reg;
} PMIC_LOCK_UNION;
#endif
#define PMIC_LOCK_lock_START (0)
#define PMIC_LOCK_lock_END (7)
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
        unsigned char dr3_mode_sel : 1;
        unsigned char dr4_mode_sel : 1;
        unsigned char dr5_mode_sel : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_DR_MODE_SEL_UNION;
#endif
#define PMIC_DR_MODE_SEL_dr3_mode_sel_START (0)
#define PMIC_DR_MODE_SEL_dr3_mode_sel_END (0)
#define PMIC_DR_MODE_SEL_dr4_mode_sel_START (1)
#define PMIC_DR_MODE_SEL_dr4_mode_sel_END (1)
#define PMIC_DR_MODE_SEL_dr5_mode_sel_START (2)
#define PMIC_DR_MODE_SEL_dr5_mode_sel_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_dr1_en : 1;
        unsigned char reserved_0 : 3;
        unsigned char reg_dr2_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_DR_1_2_CTRL0_UNION;
#endif
#define PMIC_DR_1_2_CTRL0_reg_dr1_en_START (0)
#define PMIC_DR_1_2_CTRL0_reg_dr1_en_END (0)
#define PMIC_DR_1_2_CTRL0_reg_dr2_en_START (4)
#define PMIC_DR_1_2_CTRL0_reg_dr2_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_dr12_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_DR_1_2_CTRL1_UNION;
#endif
#define PMIC_DR_1_2_CTRL1_reg_dr12_en_START (0)
#define PMIC_DR_1_2_CTRL1_reg_dr12_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr1_iset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_DR1_ISET_UNION;
#endif
#define PMIC_DR1_ISET_dr1_iset_START (0)
#define PMIC_DR1_ISET_dr1_iset_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr2_iset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_DR2_ISET_UNION;
#endif
#define PMIC_DR2_ISET_dr2_iset_START (0)
#define PMIC_DR2_ISET_dr2_iset_END (3)
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
        unsigned char dr3_iset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_DR3_ISET_UNION;
#endif
#define PMIC_DR3_ISET_dr3_iset_START (0)
#define PMIC_DR3_ISET_dr3_iset_END (3)
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
        unsigned char dr4_iset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_DR4_ISET_UNION;
#endif
#define PMIC_DR4_ISET_dr4_iset_START (0)
#define PMIC_DR4_ISET_dr4_iset_END (3)
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
        unsigned char dr5_iset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_DR5_ISET_UNION;
#endif
#define PMIC_DR5_ISET_dr5_iset_START (0)
#define PMIC_DR5_ISET_dr5_iset_END (3)
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
} PMIC_DR345_TIM_CONF0_UNION;
#endif
#define PMIC_DR345_TIM_CONF0_dr345_t_off_START (0)
#define PMIC_DR345_TIM_CONF0_dr345_t_off_END (3)
#define PMIC_DR345_TIM_CONF0_dr345_t_on_START (4)
#define PMIC_DR345_TIM_CONF0_dr345_t_on_END (7)
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
        unsigned char reserved : 8;
    } reg;
} PMIC_DR_CTRLRESERVE8_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 8;
    } reg;
} PMIC_DR_CTRLRESERVE9_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob0 : 8;
    } reg;
} PMIC_OTP_0_UNION;
#endif
#define PMIC_OTP_0_otp_pdob0_START (0)
#define PMIC_OTP_0_otp_pdob0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob1 : 8;
    } reg;
} PMIC_OTP_1_UNION;
#endif
#define PMIC_OTP_1_otp_pdob1_START (0)
#define PMIC_OTP_1_otp_pdob1_END (7)
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
} PMIC_OTP_CTRL0_UNION;
#endif
#define PMIC_OTP_CTRL0_otp_pwe_int_START (0)
#define PMIC_OTP_CTRL0_otp_pwe_int_END (0)
#define PMIC_OTP_CTRL0_otp_pwe_pulse_START (1)
#define PMIC_OTP_CTRL0_otp_pwe_pulse_END (1)
#define PMIC_OTP_CTRL0_otp_por_int_START (2)
#define PMIC_OTP_CTRL0_otp_por_int_END (2)
#define PMIC_OTP_CTRL0_otp_por_pulse_START (3)
#define PMIC_OTP_CTRL0_otp_por_pulse_END (3)
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
} PMIC_OTP_CTRL1_UNION;
#endif
#define PMIC_OTP_CTRL1_otp_pprog_START (0)
#define PMIC_OTP_CTRL1_otp_pprog_END (0)
#define PMIC_OTP_CTRL1_otp_inctrl_sel_START (1)
#define PMIC_OTP_CTRL1_otp_inctrl_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pa_cfg : 6;
        unsigned char otp_ptm : 2;
    } reg;
} PMIC_OTP_CTRL2_UNION;
#endif
#define PMIC_OTP_CTRL2_otp_pa_cfg_START (0)
#define PMIC_OTP_CTRL2_otp_pa_cfg_END (5)
#define PMIC_OTP_CTRL2_otp_ptm_START (6)
#define PMIC_OTP_CTRL2_otp_ptm_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdin : 8;
    } reg;
} PMIC_OTP_WDATA_UNION;
#endif
#define PMIC_OTP_WDATA_otp_pdin_START (0)
#define PMIC_OTP_WDATA_otp_pdin_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob0_w : 8;
    } reg;
} PMIC_OTP_0_W_UNION;
#endif
#define PMIC_OTP_0_W_otp_pdob0_w_START (0)
#define PMIC_OTP_0_W_otp_pdob0_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob1_w : 8;
    } reg;
} PMIC_OTP_1_W_UNION;
#endif
#define PMIC_OTP_1_W_otp_pdob1_w_START (0)
#define PMIC_OTP_1_W_otp_pdob1_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob2_w : 8;
    } reg;
} PMIC_OTP_2_W_UNION;
#endif
#define PMIC_OTP_2_W_otp_pdob2_w_START (0)
#define PMIC_OTP_2_W_otp_pdob2_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob4_w : 8;
    } reg;
} PMIC_OTP_4_W_UNION;
#endif
#define PMIC_OTP_4_W_otp_pdob4_w_START (0)
#define PMIC_OTP_4_W_otp_pdob4_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob5_w : 8;
    } reg;
} PMIC_OTP_5_W_UNION;
#endif
#define PMIC_OTP_5_W_otp_pdob5_w_START (0)
#define PMIC_OTP_5_W_otp_pdob5_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob6_w : 8;
    } reg;
} PMIC_OTP_6_W_UNION;
#endif
#define PMIC_OTP_6_W_otp_pdob6_w_START (0)
#define PMIC_OTP_6_W_otp_pdob6_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob7_w : 8;
    } reg;
} PMIC_OTP_7_W_UNION;
#endif
#define PMIC_OTP_7_W_otp_pdob7_w_START (0)
#define PMIC_OTP_7_W_otp_pdob7_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob8_w : 8;
    } reg;
} PMIC_OTP_8_W_UNION;
#endif
#define PMIC_OTP_8_W_otp_pdob8_w_START (0)
#define PMIC_OTP_8_W_otp_pdob8_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob9_w : 8;
    } reg;
} PMIC_OTP_9_W_UNION;
#endif
#define PMIC_OTP_9_W_otp_pdob9_w_START (0)
#define PMIC_OTP_9_W_otp_pdob9_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob10_w : 8;
    } reg;
} PMIC_OTP_10_W_UNION;
#endif
#define PMIC_OTP_10_W_otp_pdob10_w_START (0)
#define PMIC_OTP_10_W_otp_pdob10_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob11_w : 8;
    } reg;
} PMIC_OTP_11_W_UNION;
#endif
#define PMIC_OTP_11_W_otp_pdob11_w_START (0)
#define PMIC_OTP_11_W_otp_pdob11_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob12_w : 8;
    } reg;
} PMIC_OTP_12_W_UNION;
#endif
#define PMIC_OTP_12_W_otp_pdob12_w_START (0)
#define PMIC_OTP_12_W_otp_pdob12_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob13_w : 8;
    } reg;
} PMIC_OTP_13_W_UNION;
#endif
#define PMIC_OTP_13_W_otp_pdob13_w_START (0)
#define PMIC_OTP_13_W_otp_pdob13_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob14_w : 8;
    } reg;
} PMIC_OTP_14_W_UNION;
#endif
#define PMIC_OTP_14_W_otp_pdob14_w_START (0)
#define PMIC_OTP_14_W_otp_pdob14_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob15_w : 8;
    } reg;
} PMIC_OTP_15_W_UNION;
#endif
#define PMIC_OTP_15_W_otp_pdob15_w_START (0)
#define PMIC_OTP_15_W_otp_pdob15_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob16_w : 8;
    } reg;
} PMIC_OTP_16_W_UNION;
#endif
#define PMIC_OTP_16_W_otp_pdob16_w_START (0)
#define PMIC_OTP_16_W_otp_pdob16_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob17_w : 8;
    } reg;
} PMIC_OTP_17_W_UNION;
#endif
#define PMIC_OTP_17_W_otp_pdob17_w_START (0)
#define PMIC_OTP_17_W_otp_pdob17_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob18_w : 8;
    } reg;
} PMIC_OTP_18_W_UNION;
#endif
#define PMIC_OTP_18_W_otp_pdob18_w_START (0)
#define PMIC_OTP_18_W_otp_pdob18_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob19_w : 8;
    } reg;
} PMIC_OTP_19_W_UNION;
#endif
#define PMIC_OTP_19_W_otp_pdob19_w_START (0)
#define PMIC_OTP_19_W_otp_pdob19_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob20_w : 8;
    } reg;
} PMIC_OTP_20_W_UNION;
#endif
#define PMIC_OTP_20_W_otp_pdob20_w_START (0)
#define PMIC_OTP_20_W_otp_pdob20_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob21_w : 8;
    } reg;
} PMIC_OTP_21_W_UNION;
#endif
#define PMIC_OTP_21_W_otp_pdob21_w_START (0)
#define PMIC_OTP_21_W_otp_pdob21_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob22_w : 8;
    } reg;
} PMIC_OTP_22_W_UNION;
#endif
#define PMIC_OTP_22_W_otp_pdob22_w_START (0)
#define PMIC_OTP_22_W_otp_pdob22_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob23_w : 8;
    } reg;
} PMIC_OTP_23_W_UNION;
#endif
#define PMIC_OTP_23_W_otp_pdob23_w_START (0)
#define PMIC_OTP_23_W_otp_pdob23_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob24_w : 8;
    } reg;
} PMIC_OTP_24_W_UNION;
#endif
#define PMIC_OTP_24_W_otp_pdob24_w_START (0)
#define PMIC_OTP_24_W_otp_pdob24_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob25_w : 8;
    } reg;
} PMIC_OTP_25_W_UNION;
#endif
#define PMIC_OTP_25_W_otp_pdob25_w_START (0)
#define PMIC_OTP_25_W_otp_pdob25_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob26_w : 8;
    } reg;
} PMIC_OTP_26_W_UNION;
#endif
#define PMIC_OTP_26_W_otp_pdob26_w_START (0)
#define PMIC_OTP_26_W_otp_pdob26_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob27_w : 8;
    } reg;
} PMIC_OTP_27_W_UNION;
#endif
#define PMIC_OTP_27_W_otp_pdob27_w_START (0)
#define PMIC_OTP_27_W_otp_pdob27_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob28_w : 8;
    } reg;
} PMIC_OTP_28_W_UNION;
#endif
#define PMIC_OTP_28_W_otp_pdob28_w_START (0)
#define PMIC_OTP_28_W_otp_pdob28_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob29_w : 8;
    } reg;
} PMIC_OTP_29_W_UNION;
#endif
#define PMIC_OTP_29_W_otp_pdob29_w_START (0)
#define PMIC_OTP_29_W_otp_pdob29_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob30_w : 8;
    } reg;
} PMIC_OTP_30_W_UNION;
#endif
#define PMIC_OTP_30_W_otp_pdob30_w_START (0)
#define PMIC_OTP_30_W_otp_pdob30_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob31_w : 8;
    } reg;
} PMIC_OTP_31_W_UNION;
#endif
#define PMIC_OTP_31_W_otp_pdob31_w_START (0)
#define PMIC_OTP_31_W_otp_pdob31_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob32_w : 8;
    } reg;
} PMIC_OTP_32_W_UNION;
#endif
#define PMIC_OTP_32_W_otp_pdob32_w_START (0)
#define PMIC_OTP_32_W_otp_pdob32_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob33_w : 8;
    } reg;
} PMIC_OTP_33_W_UNION;
#endif
#define PMIC_OTP_33_W_otp_pdob33_w_START (0)
#define PMIC_OTP_33_W_otp_pdob33_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob34_w : 8;
    } reg;
} PMIC_OTP_34_W_UNION;
#endif
#define PMIC_OTP_34_W_otp_pdob34_w_START (0)
#define PMIC_OTP_34_W_otp_pdob34_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob35_w : 8;
    } reg;
} PMIC_OTP_35_W_UNION;
#endif
#define PMIC_OTP_35_W_otp_pdob35_w_START (0)
#define PMIC_OTP_35_W_otp_pdob35_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob36_w : 8;
    } reg;
} PMIC_OTP_36_W_UNION;
#endif
#define PMIC_OTP_36_W_otp_pdob36_w_START (0)
#define PMIC_OTP_36_W_otp_pdob36_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob37_w : 8;
    } reg;
} PMIC_OTP_37_W_UNION;
#endif
#define PMIC_OTP_37_W_otp_pdob37_w_START (0)
#define PMIC_OTP_37_W_otp_pdob37_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob38_w : 8;
    } reg;
} PMIC_OTP_38_W_UNION;
#endif
#define PMIC_OTP_38_W_otp_pdob38_w_START (0)
#define PMIC_OTP_38_W_otp_pdob38_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob39_w : 8;
    } reg;
} PMIC_OTP_39_W_UNION;
#endif
#define PMIC_OTP_39_W_otp_pdob39_w_START (0)
#define PMIC_OTP_39_W_otp_pdob39_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob40_w : 8;
    } reg;
} PMIC_OTP_40_W_UNION;
#endif
#define PMIC_OTP_40_W_otp_pdob40_w_START (0)
#define PMIC_OTP_40_W_otp_pdob40_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob41_w : 8;
    } reg;
} PMIC_OTP_41_W_UNION;
#endif
#define PMIC_OTP_41_W_otp_pdob41_w_START (0)
#define PMIC_OTP_41_W_otp_pdob41_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob42_w : 8;
    } reg;
} PMIC_OTP_42_W_UNION;
#endif
#define PMIC_OTP_42_W_otp_pdob42_w_START (0)
#define PMIC_OTP_42_W_otp_pdob42_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob43_w : 8;
    } reg;
} PMIC_OTP_43_W_UNION;
#endif
#define PMIC_OTP_43_W_otp_pdob43_w_START (0)
#define PMIC_OTP_43_W_otp_pdob43_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob44_w : 8;
    } reg;
} PMIC_OTP_44_W_UNION;
#endif
#define PMIC_OTP_44_W_otp_pdob44_w_START (0)
#define PMIC_OTP_44_W_otp_pdob44_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp_pdob45_w : 8;
    } reg;
} PMIC_OTP_45_W_UNION;
#endif
#define PMIC_OTP_45_W_otp_pdob45_w_START (0)
#define PMIC_OTP_45_W_otp_pdob45_w_END (7)
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
        unsigned char sim0_hpd_r_pd_en : 1;
        unsigned char sim0_hpd_f_pd_en : 1;
        unsigned char sim1_hpd_r_pd_en : 1;
        unsigned char sim1_hpd_f_pd_en : 1;
        unsigned char sim0_hpd_pd_ldo12_en : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_SIM_CTRL0_UNION;
#endif
#define PMIC_SIM_CTRL0_sim0_hpd_r_pd_en_START (0)
#define PMIC_SIM_CTRL0_sim0_hpd_r_pd_en_END (0)
#define PMIC_SIM_CTRL0_sim0_hpd_f_pd_en_START (1)
#define PMIC_SIM_CTRL0_sim0_hpd_f_pd_en_END (1)
#define PMIC_SIM_CTRL0_sim1_hpd_r_pd_en_START (2)
#define PMIC_SIM_CTRL0_sim1_hpd_r_pd_en_END (2)
#define PMIC_SIM_CTRL0_sim1_hpd_f_pd_en_START (3)
#define PMIC_SIM_CTRL0_sim1_hpd_f_pd_en_END (3)
#define PMIC_SIM_CTRL0_sim0_hpd_pd_ldo12_en_START (4)
#define PMIC_SIM_CTRL0_sim0_hpd_pd_ldo12_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sim0_hpd_ldo16_en3 : 1;
        unsigned char sim0_hpd_ldo16_en2 : 1;
        unsigned char sim1_hpd_ldo16_en1 : 1;
        unsigned char sim1_hpd_ldo16_en0 : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_SIM_CTRL1_UNION;
#endif
#define PMIC_SIM_CTRL1_sim0_hpd_ldo16_en3_START (0)
#define PMIC_SIM_CTRL1_sim0_hpd_ldo16_en3_END (0)
#define PMIC_SIM_CTRL1_sim0_hpd_ldo16_en2_START (1)
#define PMIC_SIM_CTRL1_sim0_hpd_ldo16_en2_END (1)
#define PMIC_SIM_CTRL1_sim1_hpd_ldo16_en1_START (2)
#define PMIC_SIM_CTRL1_sim1_hpd_ldo16_en1_END (2)
#define PMIC_SIM_CTRL1_sim1_hpd_ldo16_en0_START (3)
#define PMIC_SIM_CTRL1_sim1_hpd_ldo16_en0_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sim_hpd_deb_sel : 5;
        unsigned char sim_del_sel0 : 3;
    } reg;
} PMIC_SIM_DEB_CTRL1_UNION;
#endif
#define PMIC_SIM_DEB_CTRL1_sim_hpd_deb_sel_START (0)
#define PMIC_SIM_DEB_CTRL1_sim_hpd_deb_sel_END (4)
#define PMIC_SIM_DEB_CTRL1_sim_del_sel0_START (5)
#define PMIC_SIM_DEB_CTRL1_sim_del_sel0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sim_del_sel : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_SIM_DEB_CTRL2_UNION;
#endif
#define PMIC_SIM_DEB_CTRL2_sim_del_sel_START (0)
#define PMIC_SIM_DEB_CTRL2_sim_del_sel_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio0_data : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO0DATA_UNION;
#endif
#define PMIC_GPIO0DATA_gpio0_data_START (0)
#define PMIC_GPIO0DATA_gpio0_data_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 8;
    } reg;
} PMIC_GPIO0_DATA_RESERVE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio0_dir : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO0DIR_UNION;
#endif
#define PMIC_GPIO0DIR_gpio0_dir_START (0)
#define PMIC_GPIO0DIR_gpio0_dir_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 8;
    } reg;
} PMIC_GPIO0_DIR_RESERVE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio0_int_sense : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO0IS_UNION;
#endif
#define PMIC_GPIO0IS_gpio0_int_sense_START (0)
#define PMIC_GPIO0IS_gpio0_int_sense_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio0_int_edge : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO0IBE_UNION;
#endif
#define PMIC_GPIO0IBE_gpio0_int_edge_START (0)
#define PMIC_GPIO0IBE_gpio0_int_edge_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio0_int_f : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO0IEV_UNION;
#endif
#define PMIC_GPIO0IEV_gpio0_int_f_START (0)
#define PMIC_GPIO0IEV_gpio0_int_f_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio0_mode_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO0AFSEL_UNION;
#endif
#define PMIC_GPIO0AFSEL_gpio0_mode_ctrl_START (0)
#define PMIC_GPIO0AFSEL_gpio0_mode_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio0_out_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO0DSSEL_UNION;
#endif
#define PMIC_GPIO0DSSEL_gpio0_out_ctrl_START (0)
#define PMIC_GPIO0DSSEL_gpio0_out_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio0_pullup_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO0PUSEL_UNION;
#endif
#define PMIC_GPIO0PUSEL_gpio0_pullup_ctrl_START (0)
#define PMIC_GPIO0PUSEL_gpio0_pullup_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio0_pulldown_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO0PDSEL_UNION;
#endif
#define PMIC_GPIO0PDSEL_gpio0_pulldown_ctrl_START (0)
#define PMIC_GPIO0PDSEL_gpio0_pulldown_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio0_del_sel : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_GPIO0DEBSEL_UNION;
#endif
#define PMIC_GPIO0DEBSEL_gpio0_del_sel_START (0)
#define PMIC_GPIO0DEBSEL_gpio0_del_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sim0_pullup_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_SIM0PUSEL_UNION;
#endif
#define PMIC_SIM0PUSEL_sim0_pullup_ctrl_START (0)
#define PMIC_SIM0PUSEL_sim0_pullup_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sim0_pulldown_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_SIM0PDSEL_UNION;
#endif
#define PMIC_SIM0PDSEL_sim0_pulldown_ctrl_START (0)
#define PMIC_SIM0PDSEL_sim0_pulldown_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio1_data : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO1DATA_UNION;
#endif
#define PMIC_GPIO1DATA_gpio1_data_START (0)
#define PMIC_GPIO1DATA_gpio1_data_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 8;
    } reg;
} PMIC_GPIO1_DATA_RESERVE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio1_dir : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO1DIR_UNION;
#endif
#define PMIC_GPIO1DIR_gpio1_dir_START (0)
#define PMIC_GPIO1DIR_gpio1_dir_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 8;
    } reg;
} PMIC_GPIO1_DIR_RESERVE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio1_int_sense : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO1IS_UNION;
#endif
#define PMIC_GPIO1IS_gpio1_int_sense_START (0)
#define PMIC_GPIO1IS_gpio1_int_sense_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio1_int_edge : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO1IBE_UNION;
#endif
#define PMIC_GPIO1IBE_gpio1_int_edge_START (0)
#define PMIC_GPIO1IBE_gpio1_int_edge_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio1_int_f : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO1IEV_UNION;
#endif
#define PMIC_GPIO1IEV_gpio1_int_f_START (0)
#define PMIC_GPIO1IEV_gpio1_int_f_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio1_mode_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO1AFSEL_UNION;
#endif
#define PMIC_GPIO1AFSEL_gpio1_mode_ctrl_START (0)
#define PMIC_GPIO1AFSEL_gpio1_mode_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio1_out_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO1DSSEL_UNION;
#endif
#define PMIC_GPIO1DSSEL_gpio1_out_ctrl_START (0)
#define PMIC_GPIO1DSSEL_gpio1_out_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio1_pullup_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO1PUSEL_UNION;
#endif
#define PMIC_GPIO1PUSEL_gpio1_pullup_ctrl_START (0)
#define PMIC_GPIO1PUSEL_gpio1_pullup_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio1_pulldown_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO1PDSEL_UNION;
#endif
#define PMIC_GPIO1PDSEL_gpio1_pulldown_ctrl_START (0)
#define PMIC_GPIO1PDSEL_gpio1_pulldown_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio1_del_sel : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_GPIO1DEBSEL_UNION;
#endif
#define PMIC_GPIO1DEBSEL_gpio1_del_sel_START (0)
#define PMIC_GPIO1DEBSEL_gpio1_del_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sim1_pullup_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_SIM1PUSEL_UNION;
#endif
#define PMIC_SIM1PUSEL_sim1_pullup_ctrl_START (0)
#define PMIC_SIM1PUSEL_sim1_pullup_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sim1_pulldown_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_SIM1PDSEL_UNION;
#endif
#define PMIC_SIM1PDSEL_sim1_pulldown_ctrl_START (0)
#define PMIC_SIM1PDSEL_sim1_pulldown_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio2_data : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO2DATA_UNION;
#endif
#define PMIC_GPIO2DATA_gpio2_data_START (0)
#define PMIC_GPIO2DATA_gpio2_data_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 8;
    } reg;
} PMIC_GPIO2_DATA_RESERVE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio2_dir : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO2DIR_UNION;
#endif
#define PMIC_GPIO2DIR_gpio2_dir_START (0)
#define PMIC_GPIO2DIR_gpio2_dir_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 8;
    } reg;
} PMIC_GPIO2_DIR_RESERVE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio2_int_sense : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO2IS_UNION;
#endif
#define PMIC_GPIO2IS_gpio2_int_sense_START (0)
#define PMIC_GPIO2IS_gpio2_int_sense_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio2_int_edge : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO2IBE_UNION;
#endif
#define PMIC_GPIO2IBE_gpio2_int_edge_START (0)
#define PMIC_GPIO2IBE_gpio2_int_edge_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio2_int_f : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO2IEV_UNION;
#endif
#define PMIC_GPIO2IEV_gpio2_int_f_START (0)
#define PMIC_GPIO2IEV_gpio2_int_f_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio2_out_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO2DSSEL_UNION;
#endif
#define PMIC_GPIO2DSSEL_gpio2_out_ctrl_START (0)
#define PMIC_GPIO2DSSEL_gpio2_out_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio2_pullup_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO2PUSEL_UNION;
#endif
#define PMIC_GPIO2PUSEL_gpio2_pullup_ctrl_START (0)
#define PMIC_GPIO2PUSEL_gpio2_pullup_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio2_pulldown_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO2PDSEL_UNION;
#endif
#define PMIC_GPIO2PDSEL_gpio2_pulldown_ctrl_START (0)
#define PMIC_GPIO2PDSEL_gpio2_pulldown_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio2_del_sel : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_GPIO2DEBSEL_UNION;
#endif
#define PMIC_GPIO2DEBSEL_gpio2_del_sel_START (0)
#define PMIC_GPIO2DEBSEL_gpio2_del_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio3_data : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO3DATA_UNION;
#endif
#define PMIC_GPIO3DATA_gpio3_data_START (0)
#define PMIC_GPIO3DATA_gpio3_data_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 8;
    } reg;
} PMIC_GPIO3_DATA_RESERVE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio3_dir : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO3DIR_UNION;
#endif
#define PMIC_GPIO3DIR_gpio3_dir_START (0)
#define PMIC_GPIO3DIR_gpio3_dir_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 8;
    } reg;
} PMIC_GPIO3_DIR_RESERVE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio3_int_sense : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO3IS_UNION;
#endif
#define PMIC_GPIO3IS_gpio3_int_sense_START (0)
#define PMIC_GPIO3IS_gpio3_int_sense_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio3_int_edge : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO3IBE_UNION;
#endif
#define PMIC_GPIO3IBE_gpio3_int_edge_START (0)
#define PMIC_GPIO3IBE_gpio3_int_edge_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio3_int_f : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO3IEV_UNION;
#endif
#define PMIC_GPIO3IEV_gpio3_int_f_START (0)
#define PMIC_GPIO3IEV_gpio3_int_f_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio3_out_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO3DSSEL_UNION;
#endif
#define PMIC_GPIO3DSSEL_gpio3_out_ctrl_START (0)
#define PMIC_GPIO3DSSEL_gpio3_out_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio3_pullup_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO3PUSEL_UNION;
#endif
#define PMIC_GPIO3PUSEL_gpio3_pullup_ctrl_START (0)
#define PMIC_GPIO3PUSEL_gpio3_pullup_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio3_pulldown_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_GPIO3PDSEL_UNION;
#endif
#define PMIC_GPIO3PDSEL_gpio3_pulldown_ctrl_START (0)
#define PMIC_GPIO3PDSEL_gpio3_pulldown_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio3_del_sel : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_GPIO3DEBSEL_UNION;
#endif
#define PMIC_GPIO3DEBSEL_gpio3_del_sel_START (0)
#define PMIC_GPIO3DEBSEL_gpio3_del_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char aux_offset_cfg : 2;
        unsigned char aux_ibias_cfg : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_AUX_IBIAS_CFG_UNION;
#endif
#define PMIC_AUX_IBIAS_CFG_aux_offset_cfg_START (0)
#define PMIC_AUX_IBIAS_CFG_aux_offset_cfg_END (1)
#define PMIC_AUX_IBIAS_CFG_aux_ibias_cfg_START (2)
#define PMIC_AUX_IBIAS_CFG_aux_ibias_cfg_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dac_din_msb : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_DAC_DIN_MSB_UNION;
#endif
#define PMIC_DAC_DIN_MSB_dac_din_msb_START (0)
#define PMIC_DAC_DIN_MSB_dac_din_msb_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dac_din_lsb : 8;
    } reg;
} PMIC_DAC_DIN_LSB_UNION;
#endif
#define PMIC_DAC_DIN_LSB_dac_din_lsb_START (0)
#define PMIC_DAC_DIN_LSB_dac_din_lsb_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_rampdown_ctrl : 3;
        unsigned char buck0_rampup_ctrl : 3;
        unsigned char buck0_ramp_en_cfg : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_RAMP_BUCK0_CTRL0_UNION;
#endif
#define PMIC_RAMP_BUCK0_CTRL0_buck0_rampdown_ctrl_START (0)
#define PMIC_RAMP_BUCK0_CTRL0_buck0_rampdown_ctrl_END (2)
#define PMIC_RAMP_BUCK0_CTRL0_buck0_rampup_ctrl_START (3)
#define PMIC_RAMP_BUCK0_CTRL0_buck0_rampup_ctrl_END (5)
#define PMIC_RAMP_BUCK0_CTRL0_buck0_ramp_en_cfg_START (6)
#define PMIC_RAMP_BUCK0_CTRL0_buck0_ramp_en_cfg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_pull_down_off_cfg : 3;
        unsigned char buck0_pull_down_on_cfg : 3;
        unsigned char buck0_pull_down_cfg : 1;
        unsigned char buck0_pull_down_mode : 1;
    } reg;
} PMIC_RAMP_BUCK0_CTRL1_UNION;
#endif
#define PMIC_RAMP_BUCK0_CTRL1_buck0_pull_down_off_cfg_START (0)
#define PMIC_RAMP_BUCK0_CTRL1_buck0_pull_down_off_cfg_END (2)
#define PMIC_RAMP_BUCK0_CTRL1_buck0_pull_down_on_cfg_START (3)
#define PMIC_RAMP_BUCK0_CTRL1_buck0_pull_down_on_cfg_END (5)
#define PMIC_RAMP_BUCK0_CTRL1_buck0_pull_down_cfg_START (6)
#define PMIC_RAMP_BUCK0_CTRL1_buck0_pull_down_cfg_END (6)
#define PMIC_RAMP_BUCK0_CTRL1_buck0_pull_down_mode_START (7)
#define PMIC_RAMP_BUCK0_CTRL1_buck0_pull_down_mode_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_dly_ru_off_cfg : 3;
        unsigned char buck0_dly_ru_on_cfg : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK0_RAMPUP_STATE_UNION;
#endif
#define PMIC_BUCK0_RAMPUP_STATE_buck0_dly_ru_off_cfg_START (0)
#define PMIC_BUCK0_RAMPUP_STATE_buck0_dly_ru_off_cfg_END (2)
#define PMIC_BUCK0_RAMPUP_STATE_buck0_dly_ru_on_cfg_START (3)
#define PMIC_BUCK0_RAMPUP_STATE_buck0_dly_ru_on_cfg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_dly_rd_pd_off_cfg : 3;
        unsigned char buck0_dly_rd_pd_on_cfg : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK0_RAMPDOWN_STATE_UNION;
#endif
#define PMIC_BUCK0_RAMPDOWN_STATE_buck0_dly_rd_pd_off_cfg_START (0)
#define PMIC_BUCK0_RAMPDOWN_STATE_buck0_dly_rd_pd_off_cfg_END (2)
#define PMIC_BUCK0_RAMPDOWN_STATE_buck0_dly_rd_pd_on_cfg_START (3)
#define PMIC_BUCK0_RAMPDOWN_STATE_buck0_dly_rd_pd_on_cfg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_rampdown_chg_rate : 2;
        unsigned char buck0_rampdown_chg_vset : 2;
        unsigned char buck0_rampup_chg_rate : 2;
        unsigned char buck0_rampup_chg_vset : 2;
    } reg;
} PMIC_BUCK0_RAMP_CHG_CTRL_UNION;
#endif
#define PMIC_BUCK0_RAMP_CHG_CTRL_buck0_rampdown_chg_rate_START (0)
#define PMIC_BUCK0_RAMP_CHG_CTRL_buck0_rampdown_chg_rate_END (1)
#define PMIC_BUCK0_RAMP_CHG_CTRL_buck0_rampdown_chg_vset_START (2)
#define PMIC_BUCK0_RAMP_CHG_CTRL_buck0_rampdown_chg_vset_END (3)
#define PMIC_BUCK0_RAMP_CHG_CTRL_buck0_rampup_chg_rate_START (4)
#define PMIC_BUCK0_RAMP_CHG_CTRL_buck0_rampup_chg_rate_END (5)
#define PMIC_BUCK0_RAMP_CHG_CTRL_buck0_rampup_chg_vset_START (6)
#define PMIC_BUCK0_RAMP_CHG_CTRL_buck0_rampup_chg_vset_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_rampdown_ctrl : 3;
        unsigned char buck4_rampup_ctrl : 3;
        unsigned char buck4_ramp_en_cfg : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_RAMP_BUCK4_CTRL0_UNION;
#endif
#define PMIC_RAMP_BUCK4_CTRL0_buck4_rampdown_ctrl_START (0)
#define PMIC_RAMP_BUCK4_CTRL0_buck4_rampdown_ctrl_END (2)
#define PMIC_RAMP_BUCK4_CTRL0_buck4_rampup_ctrl_START (3)
#define PMIC_RAMP_BUCK4_CTRL0_buck4_rampup_ctrl_END (5)
#define PMIC_RAMP_BUCK4_CTRL0_buck4_ramp_en_cfg_START (6)
#define PMIC_RAMP_BUCK4_CTRL0_buck4_ramp_en_cfg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_pull_down_off_cfg : 3;
        unsigned char buck4_pull_down_on_cfg : 3;
        unsigned char buck4_pull_down_cfg : 1;
        unsigned char buck4_pull_down_mode : 1;
    } reg;
} PMIC_RAMP_BUCK4_CTRL1_UNION;
#endif
#define PMIC_RAMP_BUCK4_CTRL1_buck4_pull_down_off_cfg_START (0)
#define PMIC_RAMP_BUCK4_CTRL1_buck4_pull_down_off_cfg_END (2)
#define PMIC_RAMP_BUCK4_CTRL1_buck4_pull_down_on_cfg_START (3)
#define PMIC_RAMP_BUCK4_CTRL1_buck4_pull_down_on_cfg_END (5)
#define PMIC_RAMP_BUCK4_CTRL1_buck4_pull_down_cfg_START (6)
#define PMIC_RAMP_BUCK4_CTRL1_buck4_pull_down_cfg_END (6)
#define PMIC_RAMP_BUCK4_CTRL1_buck4_pull_down_mode_START (7)
#define PMIC_RAMP_BUCK4_CTRL1_buck4_pull_down_mode_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_dly_ru_off_cfg : 3;
        unsigned char buck4_dly_ru_on_cfg : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK4_RAMPUP_STATE_UNION;
#endif
#define PMIC_BUCK4_RAMPUP_STATE_buck4_dly_ru_off_cfg_START (0)
#define PMIC_BUCK4_RAMPUP_STATE_buck4_dly_ru_off_cfg_END (2)
#define PMIC_BUCK4_RAMPUP_STATE_buck4_dly_ru_on_cfg_START (3)
#define PMIC_BUCK4_RAMPUP_STATE_buck4_dly_ru_on_cfg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_dly_rd_pd_off_cfg : 3;
        unsigned char buck4_dly_rd_pd_on_cfg : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK4_RAMPDOWN_STATE_UNION;
#endif
#define PMIC_BUCK4_RAMPDOWN_STATE_buck4_dly_rd_pd_off_cfg_START (0)
#define PMIC_BUCK4_RAMPDOWN_STATE_buck4_dly_rd_pd_off_cfg_END (2)
#define PMIC_BUCK4_RAMPDOWN_STATE_buck4_dly_rd_pd_on_cfg_START (3)
#define PMIC_BUCK4_RAMPDOWN_STATE_buck4_dly_rd_pd_on_cfg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_rampdown_chg_rate : 2;
        unsigned char buck4_rampdown_chg_vset : 2;
        unsigned char buck4_rampup_chg_rate : 2;
        unsigned char buck4_rampup_chg_vset : 2;
    } reg;
} PMIC_BUCK4_RAMP_CHG_CTRL_UNION;
#endif
#define PMIC_BUCK4_RAMP_CHG_CTRL_buck4_rampdown_chg_rate_START (0)
#define PMIC_BUCK4_RAMP_CHG_CTRL_buck4_rampdown_chg_rate_END (1)
#define PMIC_BUCK4_RAMP_CHG_CTRL_buck4_rampdown_chg_vset_START (2)
#define PMIC_BUCK4_RAMP_CHG_CTRL_buck4_rampdown_chg_vset_END (3)
#define PMIC_BUCK4_RAMP_CHG_CTRL_buck4_rampup_chg_rate_START (4)
#define PMIC_BUCK4_RAMP_CHG_CTRL_buck4_rampup_chg_rate_END (5)
#define PMIC_BUCK4_RAMP_CHG_CTRL_buck4_rampup_chg_vset_START (6)
#define PMIC_BUCK4_RAMP_CHG_CTRL_buck4_rampup_chg_vset_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_rampdown_ctrl : 3;
        unsigned char buck5_rampup_ctrl : 3;
        unsigned char buck5_ramp_en_cfg : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_RAMP_BUCK5_CTRL0_UNION;
#endif
#define PMIC_RAMP_BUCK5_CTRL0_buck5_rampdown_ctrl_START (0)
#define PMIC_RAMP_BUCK5_CTRL0_buck5_rampdown_ctrl_END (2)
#define PMIC_RAMP_BUCK5_CTRL0_buck5_rampup_ctrl_START (3)
#define PMIC_RAMP_BUCK5_CTRL0_buck5_rampup_ctrl_END (5)
#define PMIC_RAMP_BUCK5_CTRL0_buck5_ramp_en_cfg_START (6)
#define PMIC_RAMP_BUCK5_CTRL0_buck5_ramp_en_cfg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_pull_down_off_cfg : 3;
        unsigned char buck5_pull_down_on_cfg : 3;
        unsigned char buck5_pull_down_cfg : 1;
        unsigned char buck5_pull_down_mode : 1;
    } reg;
} PMIC_RAMP_BUCK5_CTRL1_UNION;
#endif
#define PMIC_RAMP_BUCK5_CTRL1_buck5_pull_down_off_cfg_START (0)
#define PMIC_RAMP_BUCK5_CTRL1_buck5_pull_down_off_cfg_END (2)
#define PMIC_RAMP_BUCK5_CTRL1_buck5_pull_down_on_cfg_START (3)
#define PMIC_RAMP_BUCK5_CTRL1_buck5_pull_down_on_cfg_END (5)
#define PMIC_RAMP_BUCK5_CTRL1_buck5_pull_down_cfg_START (6)
#define PMIC_RAMP_BUCK5_CTRL1_buck5_pull_down_cfg_END (6)
#define PMIC_RAMP_BUCK5_CTRL1_buck5_pull_down_mode_START (7)
#define PMIC_RAMP_BUCK5_CTRL1_buck5_pull_down_mode_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_dly_ru_off_cfg : 3;
        unsigned char buck5_dly_ru_on_cfg : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK5_RAMPUP_STATE_UNION;
#endif
#define PMIC_BUCK5_RAMPUP_STATE_buck5_dly_ru_off_cfg_START (0)
#define PMIC_BUCK5_RAMPUP_STATE_buck5_dly_ru_off_cfg_END (2)
#define PMIC_BUCK5_RAMPUP_STATE_buck5_dly_ru_on_cfg_START (3)
#define PMIC_BUCK5_RAMPUP_STATE_buck5_dly_ru_on_cfg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_dly_rd_pd_off_cfg : 3;
        unsigned char buck5_dly_rd_pd_on_cfg : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK5_RAMPDOWN_STATE_UNION;
#endif
#define PMIC_BUCK5_RAMPDOWN_STATE_buck5_dly_rd_pd_off_cfg_START (0)
#define PMIC_BUCK5_RAMPDOWN_STATE_buck5_dly_rd_pd_off_cfg_END (2)
#define PMIC_BUCK5_RAMPDOWN_STATE_buck5_dly_rd_pd_on_cfg_START (3)
#define PMIC_BUCK5_RAMPDOWN_STATE_buck5_dly_rd_pd_on_cfg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_rampdown_chg_rate : 2;
        unsigned char buck5_rampdown_chg_vset : 2;
        unsigned char buck5_rampup_chg_rate : 2;
        unsigned char buck5_rampup_chg_vset : 2;
    } reg;
} PMIC_BUCK5_RAMP_CHG_CTRL_UNION;
#endif
#define PMIC_BUCK5_RAMP_CHG_CTRL_buck5_rampdown_chg_rate_START (0)
#define PMIC_BUCK5_RAMP_CHG_CTRL_buck5_rampdown_chg_rate_END (1)
#define PMIC_BUCK5_RAMP_CHG_CTRL_buck5_rampdown_chg_vset_START (2)
#define PMIC_BUCK5_RAMP_CHG_CTRL_buck5_rampdown_chg_vset_END (3)
#define PMIC_BUCK5_RAMP_CHG_CTRL_buck5_rampup_chg_rate_START (4)
#define PMIC_BUCK5_RAMP_CHG_CTRL_buck5_rampup_chg_rate_END (5)
#define PMIC_BUCK5_RAMP_CHG_CTRL_buck5_rampup_chg_vset_START (6)
#define PMIC_BUCK5_RAMP_CHG_CTRL_buck5_rampup_chg_vset_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo0_2_rampdown_ctrl : 3;
        unsigned char ldo0_2_rampup_ctrl : 3;
        unsigned char ldo0_2_ramp_en_cfg : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_RAMP_LDO0_2_CTRL0_UNION;
#endif
#define PMIC_RAMP_LDO0_2_CTRL0_ldo0_2_rampdown_ctrl_START (0)
#define PMIC_RAMP_LDO0_2_CTRL0_ldo0_2_rampdown_ctrl_END (2)
#define PMIC_RAMP_LDO0_2_CTRL0_ldo0_2_rampup_ctrl_START (3)
#define PMIC_RAMP_LDO0_2_CTRL0_ldo0_2_rampup_ctrl_END (5)
#define PMIC_RAMP_LDO0_2_CTRL0_ldo0_2_ramp_en_cfg_START (6)
#define PMIC_RAMP_LDO0_2_CTRL0_ldo0_2_ramp_en_cfg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo0_2_pull_down_off_cfg : 3;
        unsigned char ldo0_2_pull_down_on_cfg : 3;
        unsigned char ldo0_2_pull_down_cfg : 1;
        unsigned char ldo0_2_pull_down_mode : 1;
    } reg;
} PMIC_RAMP_LDO0_2_CTRL1_UNION;
#endif
#define PMIC_RAMP_LDO0_2_CTRL1_ldo0_2_pull_down_off_cfg_START (0)
#define PMIC_RAMP_LDO0_2_CTRL1_ldo0_2_pull_down_off_cfg_END (2)
#define PMIC_RAMP_LDO0_2_CTRL1_ldo0_2_pull_down_on_cfg_START (3)
#define PMIC_RAMP_LDO0_2_CTRL1_ldo0_2_pull_down_on_cfg_END (5)
#define PMIC_RAMP_LDO0_2_CTRL1_ldo0_2_pull_down_cfg_START (6)
#define PMIC_RAMP_LDO0_2_CTRL1_ldo0_2_pull_down_cfg_END (6)
#define PMIC_RAMP_LDO0_2_CTRL1_ldo0_2_pull_down_mode_START (7)
#define PMIC_RAMP_LDO0_2_CTRL1_ldo0_2_pull_down_mode_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo40_rampdown_ctrl : 3;
        unsigned char ldo40_rampup_ctrl : 3;
        unsigned char ldo40_ramp_en_cfg : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_RAMP_LDO40_CTRL0_UNION;
#endif
#define PMIC_RAMP_LDO40_CTRL0_ldo40_rampdown_ctrl_START (0)
#define PMIC_RAMP_LDO40_CTRL0_ldo40_rampdown_ctrl_END (2)
#define PMIC_RAMP_LDO40_CTRL0_ldo40_rampup_ctrl_START (3)
#define PMIC_RAMP_LDO40_CTRL0_ldo40_rampup_ctrl_END (5)
#define PMIC_RAMP_LDO40_CTRL0_ldo40_ramp_en_cfg_START (6)
#define PMIC_RAMP_LDO40_CTRL0_ldo40_ramp_en_cfg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo40_pull_down_off_cfg : 3;
        unsigned char ldo40_pull_down_on_cfg : 3;
        unsigned char ldo40_pull_down_cfg : 1;
        unsigned char ldo40_pull_down_mode : 1;
    } reg;
} PMIC_RAMP_LDO40_CTRL1_UNION;
#endif
#define PMIC_RAMP_LDO40_CTRL1_ldo40_pull_down_off_cfg_START (0)
#define PMIC_RAMP_LDO40_CTRL1_ldo40_pull_down_off_cfg_END (2)
#define PMIC_RAMP_LDO40_CTRL1_ldo40_pull_down_on_cfg_START (3)
#define PMIC_RAMP_LDO40_CTRL1_ldo40_pull_down_on_cfg_END (5)
#define PMIC_RAMP_LDO40_CTRL1_ldo40_pull_down_cfg_START (6)
#define PMIC_RAMP_LDO40_CTRL1_ldo40_pull_down_cfg_END (6)
#define PMIC_RAMP_LDO40_CTRL1_ldo40_pull_down_mode_START (7)
#define PMIC_RAMP_LDO40_CTRL1_ldo40_pull_down_mode_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char clk32_bt_ds : 2;
        unsigned char clk32_sys_ds : 2;
        unsigned char clk32_gps_ds : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_DIG_IO_DS_32K_CFG_UNION;
#endif
#define PMIC_DIG_IO_DS_32K_CFG_clk32_bt_ds_START (0)
#define PMIC_DIG_IO_DS_32K_CFG_clk32_bt_ds_END (1)
#define PMIC_DIG_IO_DS_32K_CFG_clk32_sys_ds_START (2)
#define PMIC_DIG_IO_DS_32K_CFG_clk32_sys_ds_END (3)
#define PMIC_DIG_IO_DS_32K_CFG_clk32_gps_ds_START (4)
#define PMIC_DIG_IO_DS_32K_CFG_clk32_gps_ds_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char adc_sdata1_ds : 2;
        unsigned char adc_sdata0_ds : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_DIG_IO_DS_CODEC_CFG_UNION;
#endif
#define PMIC_DIG_IO_DS_CODEC_CFG_adc_sdata1_ds_START (0)
#define PMIC_DIG_IO_DS_CODEC_CFG_adc_sdata1_ds_END (1)
#define PMIC_DIG_IO_DS_CODEC_CFG_adc_sdata0_ds_START (2)
#define PMIC_DIG_IO_DS_CODEC_CFG_adc_sdata0_ds_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char nfc_pd_res : 1;
        unsigned char wifi_pd_res : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_PULL_DOWN_CFG_UNION;
#endif
#define PMIC_PULL_DOWN_CFG_nfc_pd_res_START (0)
#define PMIC_PULL_DOWN_CFG_nfc_pd_res_END (0)
#define PMIC_PULL_DOWN_CFG_wifi_pd_res_START (1)
#define PMIC_PULL_DOWN_CFG_wifi_pd_res_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spmi_eco_gt_bypass : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_SPMI_ECO_GT_BYPASS_UNION;
#endif
#define PMIC_SPMI_ECO_GT_BYPASS_spmi_eco_gt_bypass_START (0)
#define PMIC_SPMI_ECO_GT_BYPASS_spmi_eco_gt_bypass_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xoadc_debug : 1;
        unsigned char pwronn_8s_hreset_mode : 1;
        unsigned char ramp_gt_debug : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_PWRONN_8S_XOADC_DEBUG_CTRL_UNION;
#endif
#define PMIC_PWRONN_8S_XOADC_DEBUG_CTRL_xoadc_debug_START (0)
#define PMIC_PWRONN_8S_XOADC_DEBUG_CTRL_xoadc_debug_END (0)
#define PMIC_PWRONN_8S_XOADC_DEBUG_CTRL_pwronn_8s_hreset_mode_START (1)
#define PMIC_PWRONN_8S_XOADC_DEBUG_CTRL_pwronn_8s_hreset_mode_END (1)
#define PMIC_PWRONN_8S_XOADC_DEBUG_CTRL_ramp_gt_debug_START (2)
#define PMIC_PWRONN_8S_XOADC_DEBUG_CTRL_ramp_gt_debug_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dis_19m2_hreset_mode : 1;
        unsigned char dis_19m2_hreset_mask : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_DIS_19M2_CTRL_UNION;
#endif
#define PMIC_DIS_19M2_CTRL_dis_19m2_hreset_mode_START (0)
#define PMIC_DIS_19M2_CTRL_dis_19m2_hreset_mode_END (0)
#define PMIC_DIS_19M2_CTRL_dis_19m2_hreset_mask_START (1)
#define PMIC_DIS_19M2_CTRL_dis_19m2_hreset_mask_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_xo_dig_abb_sel : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_CLK_ABB_CTRL1_UNION;
#endif
#define PMIC_CLK_ABB_CTRL1_np_xo_dig_abb_sel_START (0)
#define PMIC_CLK_ABB_CTRL1_np_xo_dig_abb_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_xo_dig_wifi_sel : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_CLK_WIFI_CTRL1_UNION;
#endif
#define PMIC_CLK_WIFI_CTRL1_np_xo_dig_wifi_sel_START (0)
#define PMIC_CLK_WIFI_CTRL1_np_xo_dig_wifi_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_xo_dig_nfc_sel : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_CLK_NFC_CTRL1_UNION;
#endif
#define PMIC_CLK_NFC_CTRL1_np_xo_dig_nfc_sel_START (0)
#define PMIC_CLK_NFC_CTRL1_np_xo_dig_nfc_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_xo_dig_rf_sel : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_CLK_RF0_CTRL1_UNION;
#endif
#define PMIC_CLK_RF0_CTRL1_np_xo_dig_rf_sel_START (0)
#define PMIC_CLK_RF0_CTRL1_np_xo_dig_rf_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_xo_trim_c2fix : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_CLK_TOP_CTRL1_0_UNION;
#endif
#define PMIC_CLK_TOP_CTRL1_0_np_xo_trim_c2fix_START (0)
#define PMIC_CLK_TOP_CTRL1_0_np_xo_trim_c2fix_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_xo_trim_c1fix : 8;
    } reg;
} PMIC_CLK_TOP_CTRL1_1_UNION;
#endif
#define PMIC_CLK_TOP_CTRL1_1_np_xo_trim_c1fix_START (0)
#define PMIC_CLK_TOP_CTRL1_1_np_xo_trim_c1fix_END (7)
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
        unsigned char np_xo_ufs_sel : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_CLK_UFS_FRE_CTRL1_UNION;
#endif
#define PMIC_CLK_UFS_FRE_CTRL1_np_xo_ufs_sel_START (0)
#define PMIC_CLK_UFS_FRE_CTRL1_np_xo_ufs_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_vsys_pwroff_deb_set : 2;
        unsigned char np_vsys_pwroff_abs_set : 2;
        unsigned char np_vsys_pwron_set : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_VSYS_LOW_SET_UNION;
#endif
#define PMIC_VSYS_LOW_SET_np_vsys_pwroff_deb_set_START (0)
#define PMIC_VSYS_LOW_SET_np_vsys_pwroff_deb_set_END (1)
#define PMIC_VSYS_LOW_SET_np_vsys_pwroff_abs_set_START (2)
#define PMIC_VSYS_LOW_SET_np_vsys_pwroff_abs_set_END (3)
#define PMIC_VSYS_LOW_SET_np_vsys_pwron_set_START (4)
#define PMIC_VSYS_LOW_SET_np_vsys_pwron_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_vsys_drop_set : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_VSYS_DROP_SET_UNION;
#endif
#define PMIC_VSYS_DROP_SET_np_vsys_drop_set_START (0)
#define PMIC_VSYS_DROP_SET_np_vsys_drop_set_END (2)
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
        unsigned char np_hreset_mode : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_HRESET_PWRDOWN_CTRL_UNION;
#endif
#define PMIC_HRESET_PWRDOWN_CTRL_np_hreset_mode_START (0)
#define PMIC_HRESET_PWRDOWN_CTRL_np_hreset_mode_END (0)
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
} PMIC_SYS_CTRL0_UNION;
#endif
#define PMIC_SYS_CTRL0_np_pwron_8s_sel_START (0)
#define PMIC_SYS_CTRL0_np_pwron_8s_sel_END (0)
#define PMIC_SYS_CTRL0_np_pwron_time_sel_START (1)
#define PMIC_SYS_CTRL0_np_pwron_time_sel_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_nfc_pwron_mask : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_SYS_CTRL1_UNION;
#endif
#define PMIC_SYS_CTRL1_np_nfc_pwron_mask_START (0)
#define PMIC_SYS_CTRL1_np_nfc_pwron_mask_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_debug_lock : 8;
    } reg;
} PMIC_DEBUG_LOCK_UNION;
#endif
#define PMIC_DEBUG_LOCK_np_debug_lock_START (0)
#define PMIC_DEBUG_LOCK_np_debug_lock_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_sys_debug0 : 8;
    } reg;
} PMIC_SYS_DEBUG0_UNION;
#endif
#define PMIC_SYS_DEBUG0_np_sys_debug0_START (0)
#define PMIC_SYS_DEBUG0_np_sys_debug0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_sys_debug1 : 8;
    } reg;
} PMIC_SYS_DEBUG1_UNION;
#endif
#define PMIC_SYS_DEBUG1_np_sys_debug1_START (0)
#define PMIC_SYS_DEBUG1_np_sys_debug1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_reg_rc_debug : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_SYS_DEBUG2_UNION;
#endif
#define PMIC_SYS_DEBUG2_np_reg_rc_debug_START (0)
#define PMIC_SYS_DEBUG2_np_reg_rc_debug_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_vsys_pwroff_abs_pd_mask : 1;
        unsigned char np_vsys_pwroff_deb_pd_mask : 1;
        unsigned char np_thsd_otmp140_pd_mask : 1;
        unsigned char np_vsys_ov_pd_mask : 1;
        unsigned char np_vin_ldoh_vld_pd_mask : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_SYS_DEBUG3_UNION;
#endif
#define PMIC_SYS_DEBUG3_np_vsys_pwroff_abs_pd_mask_START (0)
#define PMIC_SYS_DEBUG3_np_vsys_pwroff_abs_pd_mask_END (0)
#define PMIC_SYS_DEBUG3_np_vsys_pwroff_deb_pd_mask_START (1)
#define PMIC_SYS_DEBUG3_np_vsys_pwroff_deb_pd_mask_END (1)
#define PMIC_SYS_DEBUG3_np_thsd_otmp140_pd_mask_START (2)
#define PMIC_SYS_DEBUG3_np_thsd_otmp140_pd_mask_END (2)
#define PMIC_SYS_DEBUG3_np_vsys_ov_pd_mask_START (3)
#define PMIC_SYS_DEBUG3_np_vsys_ov_pd_mask_END (3)
#define PMIC_SYS_DEBUG3_np_vin_ldoh_vld_pd_mask_START (4)
#define PMIC_SYS_DEBUG3_np_vin_ldoh_vld_pd_mask_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_sys_debug2 : 8;
    } reg;
} PMIC_SYS_DEBUG4_UNION;
#endif
#define PMIC_SYS_DEBUG4_np_sys_debug2_START (0)
#define PMIC_SYS_DEBUG4_np_sys_debug2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_sys_debug3 : 8;
    } reg;
} PMIC_SYS_DEBUG5_UNION;
#endif
#define PMIC_SYS_DEBUG5_np_sys_debug3_START (0)
#define PMIC_SYS_DEBUG5_np_sys_debug3_END (7)
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
        unsigned char np_rtc_cali_ctrl : 1;
        unsigned char np_hreset_d_sel : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_NP_CTRL_UNION;
#endif
#define PMIC_NP_CTRL_np_rtc_cali_ctrl_START (0)
#define PMIC_NP_CTRL_np_rtc_cali_ctrl_END (0)
#define PMIC_NP_CTRL_np_hreset_d_sel_START (1)
#define PMIC_NP_CTRL_np_hreset_d_sel_END (1)
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
        unsigned char np_hrst_reg15 : 8;
    } reg;
} PMIC_HRST_REG15_UNION;
#endif
#define PMIC_HRST_REG15_np_hrst_reg15_START (0)
#define PMIC_HRST_REG15_np_hrst_reg15_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg16 : 8;
    } reg;
} PMIC_HRST_REG16_UNION;
#endif
#define PMIC_HRST_REG16_np_hrst_reg16_START (0)
#define PMIC_HRST_REG16_np_hrst_reg16_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg17 : 8;
    } reg;
} PMIC_HRST_REG17_UNION;
#endif
#define PMIC_HRST_REG17_np_hrst_reg17_START (0)
#define PMIC_HRST_REG17_np_hrst_reg17_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg18 : 8;
    } reg;
} PMIC_HRST_REG18_UNION;
#endif
#define PMIC_HRST_REG18_np_hrst_reg18_START (0)
#define PMIC_HRST_REG18_np_hrst_reg18_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg19 : 8;
    } reg;
} PMIC_HRST_REG19_UNION;
#endif
#define PMIC_HRST_REG19_np_hrst_reg19_START (0)
#define PMIC_HRST_REG19_np_hrst_reg19_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg20 : 8;
    } reg;
} PMIC_HRST_REG20_UNION;
#endif
#define PMIC_HRST_REG20_np_hrst_reg20_START (0)
#define PMIC_HRST_REG20_np_hrst_reg20_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg21 : 8;
    } reg;
} PMIC_HRST_REG21_UNION;
#endif
#define PMIC_HRST_REG21_np_hrst_reg21_START (0)
#define PMIC_HRST_REG21_np_hrst_reg21_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg22 : 8;
    } reg;
} PMIC_HRST_REG22_UNION;
#endif
#define PMIC_HRST_REG22_np_hrst_reg22_START (0)
#define PMIC_HRST_REG22_np_hrst_reg22_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg23 : 8;
    } reg;
} PMIC_HRST_REG23_UNION;
#endif
#define PMIC_HRST_REG23_np_hrst_reg23_START (0)
#define PMIC_HRST_REG23_np_hrst_reg23_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg24 : 8;
    } reg;
} PMIC_HRST_REG24_UNION;
#endif
#define PMIC_HRST_REG24_np_hrst_reg24_START (0)
#define PMIC_HRST_REG24_np_hrst_reg24_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg25 : 8;
    } reg;
} PMIC_HRST_REG25_UNION;
#endif
#define PMIC_HRST_REG25_np_hrst_reg25_START (0)
#define PMIC_HRST_REG25_np_hrst_reg25_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg26 : 8;
    } reg;
} PMIC_HRST_REG26_UNION;
#endif
#define PMIC_HRST_REG26_np_hrst_reg26_START (0)
#define PMIC_HRST_REG26_np_hrst_reg26_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg27 : 8;
    } reg;
} PMIC_HRST_REG27_UNION;
#endif
#define PMIC_HRST_REG27_np_hrst_reg27_START (0)
#define PMIC_HRST_REG27_np_hrst_reg27_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg28 : 8;
    } reg;
} PMIC_HRST_REG28_UNION;
#endif
#define PMIC_HRST_REG28_np_hrst_reg28_START (0)
#define PMIC_HRST_REG28_np_hrst_reg28_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg29 : 8;
    } reg;
} PMIC_HRST_REG29_UNION;
#endif
#define PMIC_HRST_REG29_np_hrst_reg29_START (0)
#define PMIC_HRST_REG29_np_hrst_reg29_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg30 : 8;
    } reg;
} PMIC_HRST_REG30_UNION;
#endif
#define PMIC_HRST_REG30_np_hrst_reg30_START (0)
#define PMIC_HRST_REG30_np_hrst_reg30_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_hrst_reg31 : 8;
    } reg;
} PMIC_HRST_REG31_UNION;
#endif
#define PMIC_HRST_REG31_np_hrst_reg31_START (0)
#define PMIC_HRST_REG31_np_hrst_reg31_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob0 : 8;
    } reg;
} PMIC_OTP_0_R_UNION;
#endif
#define PMIC_OTP_0_R_np_otp_pdob0_START (0)
#define PMIC_OTP_0_R_np_otp_pdob0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob1 : 8;
    } reg;
} PMIC_OTP_1_R_UNION;
#endif
#define PMIC_OTP_1_R_np_otp_pdob1_START (0)
#define PMIC_OTP_1_R_np_otp_pdob1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob2 : 8;
    } reg;
} PMIC_OTP_2_R_UNION;
#endif
#define PMIC_OTP_2_R_np_otp_pdob2_START (0)
#define PMIC_OTP_2_R_np_otp_pdob2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob3 : 8;
    } reg;
} PMIC_OTP_3_R_UNION;
#endif
#define PMIC_OTP_3_R_np_otp_pdob3_START (0)
#define PMIC_OTP_3_R_np_otp_pdob3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob4_d2a : 8;
    } reg;
} PMIC_OTP_4_R_UNION;
#endif
#define PMIC_OTP_4_R_np_otp_pdob4_d2a_START (0)
#define PMIC_OTP_4_R_np_otp_pdob4_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob5_d2a : 8;
    } reg;
} PMIC_OTP_5_R_UNION;
#endif
#define PMIC_OTP_5_R_np_otp_pdob5_d2a_START (0)
#define PMIC_OTP_5_R_np_otp_pdob5_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob6_d2a : 8;
    } reg;
} PMIC_OTP_6_R_UNION;
#endif
#define PMIC_OTP_6_R_np_otp_pdob6_d2a_START (0)
#define PMIC_OTP_6_R_np_otp_pdob6_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob7_d2a : 8;
    } reg;
} PMIC_OTP_7_R_UNION;
#endif
#define PMIC_OTP_7_R_np_otp_pdob7_d2a_START (0)
#define PMIC_OTP_7_R_np_otp_pdob7_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob8_d2a : 8;
    } reg;
} PMIC_OTP_8_R_UNION;
#endif
#define PMIC_OTP_8_R_np_otp_pdob8_d2a_START (0)
#define PMIC_OTP_8_R_np_otp_pdob8_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob9_d2a : 8;
    } reg;
} PMIC_OTP_9_R_UNION;
#endif
#define PMIC_OTP_9_R_np_otp_pdob9_d2a_START (0)
#define PMIC_OTP_9_R_np_otp_pdob9_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob10_d2a : 8;
    } reg;
} PMIC_OTP_10_R_UNION;
#endif
#define PMIC_OTP_10_R_np_otp_pdob10_d2a_START (0)
#define PMIC_OTP_10_R_np_otp_pdob10_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob11_d2a : 8;
    } reg;
} PMIC_OTP_11_R_UNION;
#endif
#define PMIC_OTP_11_R_np_otp_pdob11_d2a_START (0)
#define PMIC_OTP_11_R_np_otp_pdob11_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob12_d2a : 8;
    } reg;
} PMIC_OTP_12_R_UNION;
#endif
#define PMIC_OTP_12_R_np_otp_pdob12_d2a_START (0)
#define PMIC_OTP_12_R_np_otp_pdob12_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob13_d2a : 8;
    } reg;
} PMIC_OTP_13_R_UNION;
#endif
#define PMIC_OTP_13_R_np_otp_pdob13_d2a_START (0)
#define PMIC_OTP_13_R_np_otp_pdob13_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob14_d2a : 8;
    } reg;
} PMIC_OTP_14_R_UNION;
#endif
#define PMIC_OTP_14_R_np_otp_pdob14_d2a_START (0)
#define PMIC_OTP_14_R_np_otp_pdob14_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob15_d2a : 8;
    } reg;
} PMIC_OTP_15_R_UNION;
#endif
#define PMIC_OTP_15_R_np_otp_pdob15_d2a_START (0)
#define PMIC_OTP_15_R_np_otp_pdob15_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob16_d2a : 8;
    } reg;
} PMIC_OTP_16_R_UNION;
#endif
#define PMIC_OTP_16_R_np_otp_pdob16_d2a_START (0)
#define PMIC_OTP_16_R_np_otp_pdob16_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob17_d2a : 8;
    } reg;
} PMIC_OTP_17_R_UNION;
#endif
#define PMIC_OTP_17_R_np_otp_pdob17_d2a_START (0)
#define PMIC_OTP_17_R_np_otp_pdob17_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob18_d2a : 8;
    } reg;
} PMIC_OTP_18_R_UNION;
#endif
#define PMIC_OTP_18_R_np_otp_pdob18_d2a_START (0)
#define PMIC_OTP_18_R_np_otp_pdob18_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob19_d2a : 8;
    } reg;
} PMIC_OTP_19_R_UNION;
#endif
#define PMIC_OTP_19_R_np_otp_pdob19_d2a_START (0)
#define PMIC_OTP_19_R_np_otp_pdob19_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob20_d2a : 8;
    } reg;
} PMIC_OTP_20_R_UNION;
#endif
#define PMIC_OTP_20_R_np_otp_pdob20_d2a_START (0)
#define PMIC_OTP_20_R_np_otp_pdob20_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob21_d2a : 8;
    } reg;
} PMIC_OTP_21_R_UNION;
#endif
#define PMIC_OTP_21_R_np_otp_pdob21_d2a_START (0)
#define PMIC_OTP_21_R_np_otp_pdob21_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob22_d2a : 8;
    } reg;
} PMIC_OTP_22_R_UNION;
#endif
#define PMIC_OTP_22_R_np_otp_pdob22_d2a_START (0)
#define PMIC_OTP_22_R_np_otp_pdob22_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob23_d2a : 8;
    } reg;
} PMIC_OTP_23_R_UNION;
#endif
#define PMIC_OTP_23_R_np_otp_pdob23_d2a_START (0)
#define PMIC_OTP_23_R_np_otp_pdob23_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob24_d2a : 8;
    } reg;
} PMIC_OTP_24_R_UNION;
#endif
#define PMIC_OTP_24_R_np_otp_pdob24_d2a_START (0)
#define PMIC_OTP_24_R_np_otp_pdob24_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob25_d2a : 8;
    } reg;
} PMIC_OTP_25_R_UNION;
#endif
#define PMIC_OTP_25_R_np_otp_pdob25_d2a_START (0)
#define PMIC_OTP_25_R_np_otp_pdob25_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob26_d2a : 8;
    } reg;
} PMIC_OTP_26_R_UNION;
#endif
#define PMIC_OTP_26_R_np_otp_pdob26_d2a_START (0)
#define PMIC_OTP_26_R_np_otp_pdob26_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob27_d2a : 8;
    } reg;
} PMIC_OTP_27_R_UNION;
#endif
#define PMIC_OTP_27_R_np_otp_pdob27_d2a_START (0)
#define PMIC_OTP_27_R_np_otp_pdob27_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob28_d2a : 8;
    } reg;
} PMIC_OTP_28_R_UNION;
#endif
#define PMIC_OTP_28_R_np_otp_pdob28_d2a_START (0)
#define PMIC_OTP_28_R_np_otp_pdob28_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob29_d2a : 8;
    } reg;
} PMIC_OTP_29_R_UNION;
#endif
#define PMIC_OTP_29_R_np_otp_pdob29_d2a_START (0)
#define PMIC_OTP_29_R_np_otp_pdob29_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob30_d2a : 8;
    } reg;
} PMIC_OTP_30_R_UNION;
#endif
#define PMIC_OTP_30_R_np_otp_pdob30_d2a_START (0)
#define PMIC_OTP_30_R_np_otp_pdob30_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob31_d2a : 8;
    } reg;
} PMIC_OTP_31_R_UNION;
#endif
#define PMIC_OTP_31_R_np_otp_pdob31_d2a_START (0)
#define PMIC_OTP_31_R_np_otp_pdob31_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob32_d2a : 8;
    } reg;
} PMIC_OTP_32_R_UNION;
#endif
#define PMIC_OTP_32_R_np_otp_pdob32_d2a_START (0)
#define PMIC_OTP_32_R_np_otp_pdob32_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob33_d2a : 8;
    } reg;
} PMIC_OTP_33_R_UNION;
#endif
#define PMIC_OTP_33_R_np_otp_pdob33_d2a_START (0)
#define PMIC_OTP_33_R_np_otp_pdob33_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob34_d2a : 8;
    } reg;
} PMIC_OTP_34_R_UNION;
#endif
#define PMIC_OTP_34_R_np_otp_pdob34_d2a_START (0)
#define PMIC_OTP_34_R_np_otp_pdob34_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob35_d2a : 8;
    } reg;
} PMIC_OTP_35_R_UNION;
#endif
#define PMIC_OTP_35_R_np_otp_pdob35_d2a_START (0)
#define PMIC_OTP_35_R_np_otp_pdob35_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob36_d2a : 8;
    } reg;
} PMIC_OTP_36_R_UNION;
#endif
#define PMIC_OTP_36_R_np_otp_pdob36_d2a_START (0)
#define PMIC_OTP_36_R_np_otp_pdob36_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob37_d2a : 8;
    } reg;
} PMIC_OTP_37_R_UNION;
#endif
#define PMIC_OTP_37_R_np_otp_pdob37_d2a_START (0)
#define PMIC_OTP_37_R_np_otp_pdob37_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob38_d2a : 8;
    } reg;
} PMIC_OTP_38_R_UNION;
#endif
#define PMIC_OTP_38_R_np_otp_pdob38_d2a_START (0)
#define PMIC_OTP_38_R_np_otp_pdob38_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob39_d2a : 8;
    } reg;
} PMIC_OTP_39_R_UNION;
#endif
#define PMIC_OTP_39_R_np_otp_pdob39_d2a_START (0)
#define PMIC_OTP_39_R_np_otp_pdob39_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob40_d2a : 8;
    } reg;
} PMIC_OTP_40_R_UNION;
#endif
#define PMIC_OTP_40_R_np_otp_pdob40_d2a_START (0)
#define PMIC_OTP_40_R_np_otp_pdob40_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob41_d2a : 8;
    } reg;
} PMIC_OTP_41_R_UNION;
#endif
#define PMIC_OTP_41_R_np_otp_pdob41_d2a_START (0)
#define PMIC_OTP_41_R_np_otp_pdob41_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob42_d2a : 8;
    } reg;
} PMIC_OTP_42_R_UNION;
#endif
#define PMIC_OTP_42_R_np_otp_pdob42_d2a_START (0)
#define PMIC_OTP_42_R_np_otp_pdob42_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob43_d2a : 8;
    } reg;
} PMIC_OTP_43_R_UNION;
#endif
#define PMIC_OTP_43_R_np_otp_pdob43_d2a_START (0)
#define PMIC_OTP_43_R_np_otp_pdob43_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob44_d2a : 8;
    } reg;
} PMIC_OTP_44_R_UNION;
#endif
#define PMIC_OTP_44_R_np_otp_pdob44_d2a_START (0)
#define PMIC_OTP_44_R_np_otp_pdob44_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob45_d2a : 8;
    } reg;
} PMIC_OTP_45_R_UNION;
#endif
#define PMIC_OTP_45_R_np_otp_pdob45_d2a_START (0)
#define PMIC_OTP_45_R_np_otp_pdob45_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob46 : 8;
    } reg;
} PMIC_OTP_46_R_UNION;
#endif
#define PMIC_OTP_46_R_np_otp_pdob46_START (0)
#define PMIC_OTP_46_R_np_otp_pdob46_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob47 : 8;
    } reg;
} PMIC_OTP_47_R_UNION;
#endif
#define PMIC_OTP_47_R_np_otp_pdob47_START (0)
#define PMIC_OTP_47_R_np_otp_pdob47_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob48 : 8;
    } reg;
} PMIC_OTP_48_R_UNION;
#endif
#define PMIC_OTP_48_R_np_otp_pdob48_START (0)
#define PMIC_OTP_48_R_np_otp_pdob48_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob49 : 8;
    } reg;
} PMIC_OTP_49_R_UNION;
#endif
#define PMIC_OTP_49_R_np_otp_pdob49_START (0)
#define PMIC_OTP_49_R_np_otp_pdob49_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob50 : 8;
    } reg;
} PMIC_OTP_50_R_UNION;
#endif
#define PMIC_OTP_50_R_np_otp_pdob50_START (0)
#define PMIC_OTP_50_R_np_otp_pdob50_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob51 : 8;
    } reg;
} PMIC_OTP_51_R_UNION;
#endif
#define PMIC_OTP_51_R_np_otp_pdob51_START (0)
#define PMIC_OTP_51_R_np_otp_pdob51_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob52 : 8;
    } reg;
} PMIC_OTP_52_R_UNION;
#endif
#define PMIC_OTP_52_R_np_otp_pdob52_START (0)
#define PMIC_OTP_52_R_np_otp_pdob52_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob53 : 8;
    } reg;
} PMIC_OTP_53_R_UNION;
#endif
#define PMIC_OTP_53_R_np_otp_pdob53_START (0)
#define PMIC_OTP_53_R_np_otp_pdob53_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob54 : 8;
    } reg;
} PMIC_OTP_54_R_UNION;
#endif
#define PMIC_OTP_54_R_np_otp_pdob54_START (0)
#define PMIC_OTP_54_R_np_otp_pdob54_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob55 : 8;
    } reg;
} PMIC_OTP_55_R_UNION;
#endif
#define PMIC_OTP_55_R_np_otp_pdob55_START (0)
#define PMIC_OTP_55_R_np_otp_pdob55_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob56 : 8;
    } reg;
} PMIC_OTP_56_R_UNION;
#endif
#define PMIC_OTP_56_R_np_otp_pdob56_START (0)
#define PMIC_OTP_56_R_np_otp_pdob56_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob57 : 8;
    } reg;
} PMIC_OTP_57_R_UNION;
#endif
#define PMIC_OTP_57_R_np_otp_pdob57_START (0)
#define PMIC_OTP_57_R_np_otp_pdob57_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob58 : 8;
    } reg;
} PMIC_OTP_58_R_UNION;
#endif
#define PMIC_OTP_58_R_np_otp_pdob58_START (0)
#define PMIC_OTP_58_R_np_otp_pdob58_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob59 : 8;
    } reg;
} PMIC_OTP_59_R_UNION;
#endif
#define PMIC_OTP_59_R_np_otp_pdob59_START (0)
#define PMIC_OTP_59_R_np_otp_pdob59_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob60 : 8;
    } reg;
} PMIC_OTP_60_R_UNION;
#endif
#define PMIC_OTP_60_R_np_otp_pdob60_START (0)
#define PMIC_OTP_60_R_np_otp_pdob60_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob61 : 8;
    } reg;
} PMIC_OTP_61_R_UNION;
#endif
#define PMIC_OTP_61_R_np_otp_pdob61_START (0)
#define PMIC_OTP_61_R_np_otp_pdob61_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob62 : 8;
    } reg;
} PMIC_OTP_62_R_UNION;
#endif
#define PMIC_OTP_62_R_np_otp_pdob62_START (0)
#define PMIC_OTP_62_R_np_otp_pdob62_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp_pdob63 : 8;
    } reg;
} PMIC_OTP_63_R_UNION;
#endif
#define PMIC_OTP_63_R_np_otp_pdob63_START (0)
#define PMIC_OTP_63_R_np_otp_pdob63_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_pmua_buck_en_cfg : 1;
        unsigned char np_pmud_buck_en_cfg : 1;
        unsigned char np_pmua_res_sel : 2;
        unsigned char np_pmud_res_sel : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_EN_PMUA_PMUD_UNION;
#endif
#define PMIC_EN_PMUA_PMUD_np_pmua_buck_en_cfg_START (0)
#define PMIC_EN_PMUA_PMUD_np_pmua_buck_en_cfg_END (0)
#define PMIC_EN_PMUA_PMUD_np_pmud_buck_en_cfg_START (1)
#define PMIC_EN_PMUA_PMUD_np_pmud_buck_en_cfg_END (1)
#define PMIC_EN_PMUA_PMUD_np_pmua_res_sel_START (2)
#define PMIC_EN_PMUA_PMUD_np_pmua_res_sel_END (3)
#define PMIC_EN_PMUA_PMUD_np_pmud_res_sel_START (4)
#define PMIC_EN_PMUA_PMUD_np_pmud_res_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_pmud_vset : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_PMUD_VSET_UNION;
#endif
#define PMIC_PMUD_VSET_np_pmud_vset_START (0)
#define PMIC_PMUD_VSET_np_pmud_vset_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_0_0 : 1;
        unsigned char irq_mask_0_1 : 1;
        unsigned char irq_mask_0_2 : 1;
        unsigned char irq_mask_0_3 : 1;
        unsigned char irq_mask_0_4 : 1;
        unsigned char irq_mask_0_5 : 1;
        unsigned char irq_mask_0_6 : 1;
        unsigned char irq_mask_0_7 : 1;
    } reg;
} PMIC_IRQ_MASK_0_UNION;
#endif
#define PMIC_IRQ_MASK_0_irq_mask_0_0_START (0)
#define PMIC_IRQ_MASK_0_irq_mask_0_0_END (0)
#define PMIC_IRQ_MASK_0_irq_mask_0_1_START (1)
#define PMIC_IRQ_MASK_0_irq_mask_0_1_END (1)
#define PMIC_IRQ_MASK_0_irq_mask_0_2_START (2)
#define PMIC_IRQ_MASK_0_irq_mask_0_2_END (2)
#define PMIC_IRQ_MASK_0_irq_mask_0_3_START (3)
#define PMIC_IRQ_MASK_0_irq_mask_0_3_END (3)
#define PMIC_IRQ_MASK_0_irq_mask_0_4_START (4)
#define PMIC_IRQ_MASK_0_irq_mask_0_4_END (4)
#define PMIC_IRQ_MASK_0_irq_mask_0_5_START (5)
#define PMIC_IRQ_MASK_0_irq_mask_0_5_END (5)
#define PMIC_IRQ_MASK_0_irq_mask_0_6_START (6)
#define PMIC_IRQ_MASK_0_irq_mask_0_6_END (6)
#define PMIC_IRQ_MASK_0_irq_mask_0_7_START (7)
#define PMIC_IRQ_MASK_0_irq_mask_0_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_1_0 : 1;
        unsigned char irq_mask_1_1 : 1;
        unsigned char irq_mask_1_2 : 1;
        unsigned char irq_mask_1_3 : 1;
        unsigned char irq_mask_1_4 : 1;
        unsigned char irq_mask_1_5 : 1;
        unsigned char irq_mask_1_6 : 1;
        unsigned char irq_mask_1_7 : 1;
    } reg;
} PMIC_IRQ_MASK_1_UNION;
#endif
#define PMIC_IRQ_MASK_1_irq_mask_1_0_START (0)
#define PMIC_IRQ_MASK_1_irq_mask_1_0_END (0)
#define PMIC_IRQ_MASK_1_irq_mask_1_1_START (1)
#define PMIC_IRQ_MASK_1_irq_mask_1_1_END (1)
#define PMIC_IRQ_MASK_1_irq_mask_1_2_START (2)
#define PMIC_IRQ_MASK_1_irq_mask_1_2_END (2)
#define PMIC_IRQ_MASK_1_irq_mask_1_3_START (3)
#define PMIC_IRQ_MASK_1_irq_mask_1_3_END (3)
#define PMIC_IRQ_MASK_1_irq_mask_1_4_START (4)
#define PMIC_IRQ_MASK_1_irq_mask_1_4_END (4)
#define PMIC_IRQ_MASK_1_irq_mask_1_5_START (5)
#define PMIC_IRQ_MASK_1_irq_mask_1_5_END (5)
#define PMIC_IRQ_MASK_1_irq_mask_1_6_START (6)
#define PMIC_IRQ_MASK_1_irq_mask_1_6_END (6)
#define PMIC_IRQ_MASK_1_irq_mask_1_7_START (7)
#define PMIC_IRQ_MASK_1_irq_mask_1_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_2_0 : 1;
        unsigned char irq_mask_2_1 : 1;
        unsigned char irq_mask_2_2 : 1;
        unsigned char irq_mask_2_3 : 1;
        unsigned char irq_mask_2_4 : 1;
        unsigned char irq_mask_2_5 : 1;
        unsigned char irq_mask_2_6 : 1;
        unsigned char irq_mask_2_7 : 1;
    } reg;
} PMIC_IRQ_MASK_2_UNION;
#endif
#define PMIC_IRQ_MASK_2_irq_mask_2_0_START (0)
#define PMIC_IRQ_MASK_2_irq_mask_2_0_END (0)
#define PMIC_IRQ_MASK_2_irq_mask_2_1_START (1)
#define PMIC_IRQ_MASK_2_irq_mask_2_1_END (1)
#define PMIC_IRQ_MASK_2_irq_mask_2_2_START (2)
#define PMIC_IRQ_MASK_2_irq_mask_2_2_END (2)
#define PMIC_IRQ_MASK_2_irq_mask_2_3_START (3)
#define PMIC_IRQ_MASK_2_irq_mask_2_3_END (3)
#define PMIC_IRQ_MASK_2_irq_mask_2_4_START (4)
#define PMIC_IRQ_MASK_2_irq_mask_2_4_END (4)
#define PMIC_IRQ_MASK_2_irq_mask_2_5_START (5)
#define PMIC_IRQ_MASK_2_irq_mask_2_5_END (5)
#define PMIC_IRQ_MASK_2_irq_mask_2_6_START (6)
#define PMIC_IRQ_MASK_2_irq_mask_2_6_END (6)
#define PMIC_IRQ_MASK_2_irq_mask_2_7_START (7)
#define PMIC_IRQ_MASK_2_irq_mask_2_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_3_0 : 1;
        unsigned char irq_mask_3_1 : 1;
        unsigned char irq_mask_3_2 : 1;
        unsigned char irq_mask_3_3 : 1;
        unsigned char irq_mask_3_4 : 1;
        unsigned char irq_mask_3_5 : 1;
        unsigned char irq_mask_3_6 : 1;
        unsigned char irq_mask_3_7 : 1;
    } reg;
} PMIC_IRQ_MASK_3_UNION;
#endif
#define PMIC_IRQ_MASK_3_irq_mask_3_0_START (0)
#define PMIC_IRQ_MASK_3_irq_mask_3_0_END (0)
#define PMIC_IRQ_MASK_3_irq_mask_3_1_START (1)
#define PMIC_IRQ_MASK_3_irq_mask_3_1_END (1)
#define PMIC_IRQ_MASK_3_irq_mask_3_2_START (2)
#define PMIC_IRQ_MASK_3_irq_mask_3_2_END (2)
#define PMIC_IRQ_MASK_3_irq_mask_3_3_START (3)
#define PMIC_IRQ_MASK_3_irq_mask_3_3_END (3)
#define PMIC_IRQ_MASK_3_irq_mask_3_4_START (4)
#define PMIC_IRQ_MASK_3_irq_mask_3_4_END (4)
#define PMIC_IRQ_MASK_3_irq_mask_3_5_START (5)
#define PMIC_IRQ_MASK_3_irq_mask_3_5_END (5)
#define PMIC_IRQ_MASK_3_irq_mask_3_6_START (6)
#define PMIC_IRQ_MASK_3_irq_mask_3_6_END (6)
#define PMIC_IRQ_MASK_3_irq_mask_3_7_START (7)
#define PMIC_IRQ_MASK_3_irq_mask_3_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_4_0 : 1;
        unsigned char irq_mask_4_1 : 1;
        unsigned char irq_mask_4_2 : 1;
        unsigned char irq_mask_4_3 : 1;
        unsigned char irq_mask_4_4 : 1;
        unsigned char irq_mask_4_5 : 1;
        unsigned char irq_mask_4_6 : 1;
        unsigned char irq_mask_4_7 : 1;
    } reg;
} PMIC_IRQ_MASK_4_UNION;
#endif
#define PMIC_IRQ_MASK_4_irq_mask_4_0_START (0)
#define PMIC_IRQ_MASK_4_irq_mask_4_0_END (0)
#define PMIC_IRQ_MASK_4_irq_mask_4_1_START (1)
#define PMIC_IRQ_MASK_4_irq_mask_4_1_END (1)
#define PMIC_IRQ_MASK_4_irq_mask_4_2_START (2)
#define PMIC_IRQ_MASK_4_irq_mask_4_2_END (2)
#define PMIC_IRQ_MASK_4_irq_mask_4_3_START (3)
#define PMIC_IRQ_MASK_4_irq_mask_4_3_END (3)
#define PMIC_IRQ_MASK_4_irq_mask_4_4_START (4)
#define PMIC_IRQ_MASK_4_irq_mask_4_4_END (4)
#define PMIC_IRQ_MASK_4_irq_mask_4_5_START (5)
#define PMIC_IRQ_MASK_4_irq_mask_4_5_END (5)
#define PMIC_IRQ_MASK_4_irq_mask_4_6_START (6)
#define PMIC_IRQ_MASK_4_irq_mask_4_6_END (6)
#define PMIC_IRQ_MASK_4_irq_mask_4_7_START (7)
#define PMIC_IRQ_MASK_4_irq_mask_4_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_5_0 : 1;
        unsigned char irq_mask_5_1 : 1;
        unsigned char irq_mask_5_2 : 1;
        unsigned char irq_mask_5_3 : 1;
        unsigned char irq_mask_5_4 : 1;
        unsigned char irq_mask_5_5 : 1;
        unsigned char irq_mask_5_6 : 1;
        unsigned char irq_mask_5_7 : 1;
    } reg;
} PMIC_IRQ_MASK_5_UNION;
#endif
#define PMIC_IRQ_MASK_5_irq_mask_5_0_START (0)
#define PMIC_IRQ_MASK_5_irq_mask_5_0_END (0)
#define PMIC_IRQ_MASK_5_irq_mask_5_1_START (1)
#define PMIC_IRQ_MASK_5_irq_mask_5_1_END (1)
#define PMIC_IRQ_MASK_5_irq_mask_5_2_START (2)
#define PMIC_IRQ_MASK_5_irq_mask_5_2_END (2)
#define PMIC_IRQ_MASK_5_irq_mask_5_3_START (3)
#define PMIC_IRQ_MASK_5_irq_mask_5_3_END (3)
#define PMIC_IRQ_MASK_5_irq_mask_5_4_START (4)
#define PMIC_IRQ_MASK_5_irq_mask_5_4_END (4)
#define PMIC_IRQ_MASK_5_irq_mask_5_5_START (5)
#define PMIC_IRQ_MASK_5_irq_mask_5_5_END (5)
#define PMIC_IRQ_MASK_5_irq_mask_5_6_START (6)
#define PMIC_IRQ_MASK_5_irq_mask_5_6_END (6)
#define PMIC_IRQ_MASK_5_irq_mask_5_7_START (7)
#define PMIC_IRQ_MASK_5_irq_mask_5_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_6_0 : 1;
        unsigned char irq_mask_6_1 : 1;
        unsigned char irq_mask_6_2 : 1;
        unsigned char irq_mask_6_3 : 1;
        unsigned char irq_mask_6_4 : 1;
        unsigned char irq_mask_6_5 : 1;
        unsigned char irq_mask_6_6 : 1;
        unsigned char irq_mask_6_7 : 1;
    } reg;
} PMIC_IRQ_MASK_6_UNION;
#endif
#define PMIC_IRQ_MASK_6_irq_mask_6_0_START (0)
#define PMIC_IRQ_MASK_6_irq_mask_6_0_END (0)
#define PMIC_IRQ_MASK_6_irq_mask_6_1_START (1)
#define PMIC_IRQ_MASK_6_irq_mask_6_1_END (1)
#define PMIC_IRQ_MASK_6_irq_mask_6_2_START (2)
#define PMIC_IRQ_MASK_6_irq_mask_6_2_END (2)
#define PMIC_IRQ_MASK_6_irq_mask_6_3_START (3)
#define PMIC_IRQ_MASK_6_irq_mask_6_3_END (3)
#define PMIC_IRQ_MASK_6_irq_mask_6_4_START (4)
#define PMIC_IRQ_MASK_6_irq_mask_6_4_END (4)
#define PMIC_IRQ_MASK_6_irq_mask_6_5_START (5)
#define PMIC_IRQ_MASK_6_irq_mask_6_5_END (5)
#define PMIC_IRQ_MASK_6_irq_mask_6_6_START (6)
#define PMIC_IRQ_MASK_6_irq_mask_6_6_END (6)
#define PMIC_IRQ_MASK_6_irq_mask_6_7_START (7)
#define PMIC_IRQ_MASK_6_irq_mask_6_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_7_0 : 1;
        unsigned char irq_mask_7_1 : 1;
        unsigned char irq_mask_7_2 : 1;
        unsigned char irq_mask_7_3 : 1;
        unsigned char irq_mask_7_4 : 1;
        unsigned char irq_mask_7_5 : 1;
        unsigned char irq_mask_7_6 : 1;
        unsigned char irq_mask_7_7 : 1;
    } reg;
} PMIC_IRQ_MASK_7_UNION;
#endif
#define PMIC_IRQ_MASK_7_irq_mask_7_0_START (0)
#define PMIC_IRQ_MASK_7_irq_mask_7_0_END (0)
#define PMIC_IRQ_MASK_7_irq_mask_7_1_START (1)
#define PMIC_IRQ_MASK_7_irq_mask_7_1_END (1)
#define PMIC_IRQ_MASK_7_irq_mask_7_2_START (2)
#define PMIC_IRQ_MASK_7_irq_mask_7_2_END (2)
#define PMIC_IRQ_MASK_7_irq_mask_7_3_START (3)
#define PMIC_IRQ_MASK_7_irq_mask_7_3_END (3)
#define PMIC_IRQ_MASK_7_irq_mask_7_4_START (4)
#define PMIC_IRQ_MASK_7_irq_mask_7_4_END (4)
#define PMIC_IRQ_MASK_7_irq_mask_7_5_START (5)
#define PMIC_IRQ_MASK_7_irq_mask_7_5_END (5)
#define PMIC_IRQ_MASK_7_irq_mask_7_6_START (6)
#define PMIC_IRQ_MASK_7_irq_mask_7_6_END (6)
#define PMIC_IRQ_MASK_7_irq_mask_7_7_START (7)
#define PMIC_IRQ_MASK_7_irq_mask_7_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_8_0 : 1;
        unsigned char irq_mask_8_1 : 1;
        unsigned char irq_mask_8_2 : 1;
        unsigned char irq_mask_8_3 : 1;
        unsigned char irq_mask_8_4 : 1;
        unsigned char irq_mask_8_5 : 1;
        unsigned char irq_mask_8_6 : 1;
        unsigned char irq_mask_8_7 : 1;
    } reg;
} PMIC_IRQ_MASK_8_UNION;
#endif
#define PMIC_IRQ_MASK_8_irq_mask_8_0_START (0)
#define PMIC_IRQ_MASK_8_irq_mask_8_0_END (0)
#define PMIC_IRQ_MASK_8_irq_mask_8_1_START (1)
#define PMIC_IRQ_MASK_8_irq_mask_8_1_END (1)
#define PMIC_IRQ_MASK_8_irq_mask_8_2_START (2)
#define PMIC_IRQ_MASK_8_irq_mask_8_2_END (2)
#define PMIC_IRQ_MASK_8_irq_mask_8_3_START (3)
#define PMIC_IRQ_MASK_8_irq_mask_8_3_END (3)
#define PMIC_IRQ_MASK_8_irq_mask_8_4_START (4)
#define PMIC_IRQ_MASK_8_irq_mask_8_4_END (4)
#define PMIC_IRQ_MASK_8_irq_mask_8_5_START (5)
#define PMIC_IRQ_MASK_8_irq_mask_8_5_END (5)
#define PMIC_IRQ_MASK_8_irq_mask_8_6_START (6)
#define PMIC_IRQ_MASK_8_irq_mask_8_6_END (6)
#define PMIC_IRQ_MASK_8_irq_mask_8_7_START (7)
#define PMIC_IRQ_MASK_8_irq_mask_8_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_gpio_0 : 1;
        unsigned char irq_mask_gpio_1 : 1;
        unsigned char irq_mask_gpio_2 : 1;
        unsigned char irq_mask_gpio_3 : 1;
        unsigned char irq_mask_gpio_7_4 : 4;
    } reg;
} PMIC_IRQ_MASK_9_UNION;
#endif
#define PMIC_IRQ_MASK_9_irq_mask_gpio_0_START (0)
#define PMIC_IRQ_MASK_9_irq_mask_gpio_0_END (0)
#define PMIC_IRQ_MASK_9_irq_mask_gpio_1_START (1)
#define PMIC_IRQ_MASK_9_irq_mask_gpio_1_END (1)
#define PMIC_IRQ_MASK_9_irq_mask_gpio_2_START (2)
#define PMIC_IRQ_MASK_9_irq_mask_gpio_2_END (2)
#define PMIC_IRQ_MASK_9_irq_mask_gpio_3_START (3)
#define PMIC_IRQ_MASK_9_irq_mask_gpio_3_END (3)
#define PMIC_IRQ_MASK_9_irq_mask_gpio_7_4_START (4)
#define PMIC_IRQ_MASK_9_irq_mask_gpio_7_4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_soh_0_0 : 1;
        unsigned char irq_mask_soh_0_1 : 1;
        unsigned char irq_mask_soh_0_2 : 1;
        unsigned char irq_mask_soh_0_3 : 1;
        unsigned char irq_mask_soh_0_4 : 1;
        unsigned char irq_mask_soh_0_5 : 1;
        unsigned char irq_mask_soh_0_6 : 1;
        unsigned char irq_mask_soh_0_7 : 1;
    } reg;
} PMIC_IRQ_MASK_10_UNION;
#endif
#define PMIC_IRQ_MASK_10_irq_mask_soh_0_0_START (0)
#define PMIC_IRQ_MASK_10_irq_mask_soh_0_0_END (0)
#define PMIC_IRQ_MASK_10_irq_mask_soh_0_1_START (1)
#define PMIC_IRQ_MASK_10_irq_mask_soh_0_1_END (1)
#define PMIC_IRQ_MASK_10_irq_mask_soh_0_2_START (2)
#define PMIC_IRQ_MASK_10_irq_mask_soh_0_2_END (2)
#define PMIC_IRQ_MASK_10_irq_mask_soh_0_3_START (3)
#define PMIC_IRQ_MASK_10_irq_mask_soh_0_3_END (3)
#define PMIC_IRQ_MASK_10_irq_mask_soh_0_4_START (4)
#define PMIC_IRQ_MASK_10_irq_mask_soh_0_4_END (4)
#define PMIC_IRQ_MASK_10_irq_mask_soh_0_5_START (5)
#define PMIC_IRQ_MASK_10_irq_mask_soh_0_5_END (5)
#define PMIC_IRQ_MASK_10_irq_mask_soh_0_6_START (6)
#define PMIC_IRQ_MASK_10_irq_mask_soh_0_6_END (6)
#define PMIC_IRQ_MASK_10_irq_mask_soh_0_7_START (7)
#define PMIC_IRQ_MASK_10_irq_mask_soh_0_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_dcr_0 : 1;
        unsigned char irq_mask_dcr_1 : 1;
        unsigned char irq_mask_dcr_2 : 1;
        unsigned char irq_mask_dcr_7_3 : 5;
    } reg;
} PMIC_IRQ_MASK_11_UNION;
#endif
#define PMIC_IRQ_MASK_11_irq_mask_dcr_0_START (0)
#define PMIC_IRQ_MASK_11_irq_mask_dcr_0_END (0)
#define PMIC_IRQ_MASK_11_irq_mask_dcr_1_START (1)
#define PMIC_IRQ_MASK_11_irq_mask_dcr_1_END (1)
#define PMIC_IRQ_MASK_11_irq_mask_dcr_2_START (2)
#define PMIC_IRQ_MASK_11_irq_mask_dcr_2_END (2)
#define PMIC_IRQ_MASK_11_irq_mask_dcr_7_3_START (3)
#define PMIC_IRQ_MASK_11_irq_mask_dcr_7_3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_vsys_drop_0 : 1;
        unsigned char irq_mask_vsys_drop_7_1 : 7;
    } reg;
} PMIC_IRQ_MASK_12_UNION;
#endif
#define PMIC_IRQ_MASK_12_irq_mask_vsys_drop_0_START (0)
#define PMIC_IRQ_MASK_12_irq_mask_vsys_drop_0_END (0)
#define PMIC_IRQ_MASK_12_irq_mask_vsys_drop_7_1_START (1)
#define PMIC_IRQ_MASK_12_irq_mask_vsys_drop_7_1_END (7)
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
        unsigned char ocpbuck0 : 1;
        unsigned char ocpbuck1 : 1;
        unsigned char ocpbuck2 : 1;
        unsigned char ocpbuck3 : 1;
        unsigned char ocpbuck4 : 1;
        unsigned char ocpbuck5 : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_OCP_IRQ0_UNION;
#endif
#define PMIC_OCP_IRQ0_ocpbuck0_START (0)
#define PMIC_OCP_IRQ0_ocpbuck0_END (0)
#define PMIC_OCP_IRQ0_ocpbuck1_START (1)
#define PMIC_OCP_IRQ0_ocpbuck1_END (1)
#define PMIC_OCP_IRQ0_ocpbuck2_START (2)
#define PMIC_OCP_IRQ0_ocpbuck2_END (2)
#define PMIC_OCP_IRQ0_ocpbuck3_START (3)
#define PMIC_OCP_IRQ0_ocpbuck3_END (3)
#define PMIC_OCP_IRQ0_ocpbuck4_START (4)
#define PMIC_OCP_IRQ0_ocpbuck4_END (4)
#define PMIC_OCP_IRQ0_ocpbuck5_START (5)
#define PMIC_OCP_IRQ0_ocpbuck5_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ocpldo5 : 1;
        unsigned char ocpldo4 : 1;
        unsigned char ocpldo3 : 1;
        unsigned char ocpldo2 : 1;
        unsigned char ocpldo1 : 1;
        unsigned char ocpldo0_2 : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_OCP_IRQ1_UNION;
#endif
#define PMIC_OCP_IRQ1_ocpldo5_START (0)
#define PMIC_OCP_IRQ1_ocpldo5_END (0)
#define PMIC_OCP_IRQ1_ocpldo4_START (1)
#define PMIC_OCP_IRQ1_ocpldo4_END (1)
#define PMIC_OCP_IRQ1_ocpldo3_START (2)
#define PMIC_OCP_IRQ1_ocpldo3_END (2)
#define PMIC_OCP_IRQ1_ocpldo2_START (3)
#define PMIC_OCP_IRQ1_ocpldo2_END (3)
#define PMIC_OCP_IRQ1_ocpldo1_START (4)
#define PMIC_OCP_IRQ1_ocpldo1_END (4)
#define PMIC_OCP_IRQ1_ocpldo0_2_START (5)
#define PMIC_OCP_IRQ1_ocpldo0_2_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ocpldo15 : 1;
        unsigned char ocpldo14 : 1;
        unsigned char ocpldo13 : 1;
        unsigned char ocpldo12 : 1;
        unsigned char ocpldo11 : 1;
        unsigned char ocpldo9 : 1;
        unsigned char ocpldo8 : 1;
        unsigned char ocpldo6 : 1;
    } reg;
} PMIC_OCP_IRQ2_UNION;
#endif
#define PMIC_OCP_IRQ2_ocpldo15_START (0)
#define PMIC_OCP_IRQ2_ocpldo15_END (0)
#define PMIC_OCP_IRQ2_ocpldo14_START (1)
#define PMIC_OCP_IRQ2_ocpldo14_END (1)
#define PMIC_OCP_IRQ2_ocpldo13_START (2)
#define PMIC_OCP_IRQ2_ocpldo13_END (2)
#define PMIC_OCP_IRQ2_ocpldo12_START (3)
#define PMIC_OCP_IRQ2_ocpldo12_END (3)
#define PMIC_OCP_IRQ2_ocpldo11_START (4)
#define PMIC_OCP_IRQ2_ocpldo11_END (4)
#define PMIC_OCP_IRQ2_ocpldo9_START (5)
#define PMIC_OCP_IRQ2_ocpldo9_END (5)
#define PMIC_OCP_IRQ2_ocpldo8_START (6)
#define PMIC_OCP_IRQ2_ocpldo8_END (6)
#define PMIC_OCP_IRQ2_ocpldo6_START (7)
#define PMIC_OCP_IRQ2_ocpldo6_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ocpldo23 : 1;
        unsigned char ocpldo22 : 1;
        unsigned char ocpldo21 : 1;
        unsigned char ocpldo20 : 1;
        unsigned char ocpldo19 : 1;
        unsigned char ocpldo18 : 1;
        unsigned char ocpldo17 : 1;
        unsigned char ocpldo16 : 1;
    } reg;
} PMIC_OCP_IRQ3_UNION;
#endif
#define PMIC_OCP_IRQ3_ocpldo23_START (0)
#define PMIC_OCP_IRQ3_ocpldo23_END (0)
#define PMIC_OCP_IRQ3_ocpldo22_START (1)
#define PMIC_OCP_IRQ3_ocpldo22_END (1)
#define PMIC_OCP_IRQ3_ocpldo21_START (2)
#define PMIC_OCP_IRQ3_ocpldo21_END (2)
#define PMIC_OCP_IRQ3_ocpldo20_START (3)
#define PMIC_OCP_IRQ3_ocpldo20_END (3)
#define PMIC_OCP_IRQ3_ocpldo19_START (4)
#define PMIC_OCP_IRQ3_ocpldo19_END (4)
#define PMIC_OCP_IRQ3_ocpldo18_START (5)
#define PMIC_OCP_IRQ3_ocpldo18_END (5)
#define PMIC_OCP_IRQ3_ocpldo17_START (6)
#define PMIC_OCP_IRQ3_ocpldo17_END (6)
#define PMIC_OCP_IRQ3_ocpldo16_START (7)
#define PMIC_OCP_IRQ3_ocpldo16_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ocpldo31 : 1;
        unsigned char ocpldo30_2 : 1;
        unsigned char ocpldo29 : 1;
        unsigned char ocpldo28 : 1;
        unsigned char ocpldo27 : 1;
        unsigned char ocpldo26 : 1;
        unsigned char ocpldo25 : 1;
        unsigned char ocpldo24 : 1;
    } reg;
} PMIC_OCP_IRQ4_UNION;
#endif
#define PMIC_OCP_IRQ4_ocpldo31_START (0)
#define PMIC_OCP_IRQ4_ocpldo31_END (0)
#define PMIC_OCP_IRQ4_ocpldo30_2_START (1)
#define PMIC_OCP_IRQ4_ocpldo30_2_END (1)
#define PMIC_OCP_IRQ4_ocpldo29_START (2)
#define PMIC_OCP_IRQ4_ocpldo29_END (2)
#define PMIC_OCP_IRQ4_ocpldo28_START (3)
#define PMIC_OCP_IRQ4_ocpldo28_END (3)
#define PMIC_OCP_IRQ4_ocpldo27_START (4)
#define PMIC_OCP_IRQ4_ocpldo27_END (4)
#define PMIC_OCP_IRQ4_ocpldo26_START (5)
#define PMIC_OCP_IRQ4_ocpldo26_END (5)
#define PMIC_OCP_IRQ4_ocpldo25_START (6)
#define PMIC_OCP_IRQ4_ocpldo25_END (6)
#define PMIC_OCP_IRQ4_ocpldo24_START (7)
#define PMIC_OCP_IRQ4_ocpldo24_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ocpldo40 : 1;
        unsigned char ocpldo39 : 1;
        unsigned char ocpldo38 : 1;
        unsigned char ocpldo37 : 1;
        unsigned char pmuh_ocp : 1;
        unsigned char ocpldo34 : 1;
        unsigned char ocpldo33 : 1;
        unsigned char ocpldo32 : 1;
    } reg;
} PMIC_OCP_IRQ5_UNION;
#endif
#define PMIC_OCP_IRQ5_ocpldo40_START (0)
#define PMIC_OCP_IRQ5_ocpldo40_END (0)
#define PMIC_OCP_IRQ5_ocpldo39_START (1)
#define PMIC_OCP_IRQ5_ocpldo39_END (1)
#define PMIC_OCP_IRQ5_ocpldo38_START (2)
#define PMIC_OCP_IRQ5_ocpldo38_END (2)
#define PMIC_OCP_IRQ5_ocpldo37_START (3)
#define PMIC_OCP_IRQ5_ocpldo37_END (3)
#define PMIC_OCP_IRQ5_pmuh_ocp_START (4)
#define PMIC_OCP_IRQ5_pmuh_ocp_END (4)
#define PMIC_OCP_IRQ5_ocpldo34_START (5)
#define PMIC_OCP_IRQ5_ocpldo34_END (5)
#define PMIC_OCP_IRQ5_ocpldo33_START (6)
#define PMIC_OCP_IRQ5_ocpldo33_END (6)
#define PMIC_OCP_IRQ5_ocpldo32_START (7)
#define PMIC_OCP_IRQ5_ocpldo32_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_scp : 1;
        unsigned char buck1_scp : 1;
        unsigned char buck2_scp : 1;
        unsigned char buck3_scp : 1;
        unsigned char buck4_scp : 1;
        unsigned char buck5_scp : 1;
        unsigned char ldo_buff_scp : 1;
        unsigned char classd_ocp : 1;
    } reg;
} PMIC_SCP_IRQ0_UNION;
#endif
#define PMIC_SCP_IRQ0_buck0_scp_START (0)
#define PMIC_SCP_IRQ0_buck0_scp_END (0)
#define PMIC_SCP_IRQ0_buck1_scp_START (1)
#define PMIC_SCP_IRQ0_buck1_scp_END (1)
#define PMIC_SCP_IRQ0_buck2_scp_START (2)
#define PMIC_SCP_IRQ0_buck2_scp_END (2)
#define PMIC_SCP_IRQ0_buck3_scp_START (3)
#define PMIC_SCP_IRQ0_buck3_scp_END (3)
#define PMIC_SCP_IRQ0_buck4_scp_START (4)
#define PMIC_SCP_IRQ0_buck4_scp_END (4)
#define PMIC_SCP_IRQ0_buck5_scp_START (5)
#define PMIC_SCP_IRQ0_buck5_scp_END (5)
#define PMIC_SCP_IRQ0_ldo_buff_scp_START (6)
#define PMIC_SCP_IRQ0_ldo_buff_scp_END (6)
#define PMIC_SCP_IRQ0_classd_ocp_START (7)
#define PMIC_SCP_IRQ0_classd_ocp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio0_int_status : 1;
        unsigned char gpio1_int_status : 1;
        unsigned char gpio2_int_status : 1;
        unsigned char gpio3_int_status : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_GPIO_IRQ_UNION;
#endif
#define PMIC_GPIO_IRQ_gpio0_int_status_START (0)
#define PMIC_GPIO_IRQ_gpio0_int_status_END (0)
#define PMIC_GPIO_IRQ_gpio1_int_status_START (1)
#define PMIC_GPIO_IRQ_gpio1_int_status_END (1)
#define PMIC_GPIO_IRQ_gpio2_int_status_START (2)
#define PMIC_GPIO_IRQ_gpio2_int_status_END (2)
#define PMIC_GPIO_IRQ_gpio3_int_status_START (3)
#define PMIC_GPIO_IRQ_gpio3_int_status_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_flag : 1;
        unsigned char soh_ovp_end : 1;
        unsigned char soh_tbat_tmp_ov : 1;
        unsigned char soh_mode_abn : 1;
        unsigned char soh_vbat_uv : 1;
        unsigned char soh_otmp125 : 1;
        unsigned char soh_30min_timeout : 1;
        unsigned char discharge_ocp : 1;
    } reg;
} PMIC_ACR_SOH_IRQ_UNION;
#endif
#define PMIC_ACR_SOH_IRQ_acr_flag_START (0)
#define PMIC_ACR_SOH_IRQ_acr_flag_END (0)
#define PMIC_ACR_SOH_IRQ_soh_ovp_end_START (1)
#define PMIC_ACR_SOH_IRQ_soh_ovp_end_END (1)
#define PMIC_ACR_SOH_IRQ_soh_tbat_tmp_ov_START (2)
#define PMIC_ACR_SOH_IRQ_soh_tbat_tmp_ov_END (2)
#define PMIC_ACR_SOH_IRQ_soh_mode_abn_START (3)
#define PMIC_ACR_SOH_IRQ_soh_mode_abn_END (3)
#define PMIC_ACR_SOH_IRQ_soh_vbat_uv_START (4)
#define PMIC_ACR_SOH_IRQ_soh_vbat_uv_END (4)
#define PMIC_ACR_SOH_IRQ_soh_otmp125_START (5)
#define PMIC_ACR_SOH_IRQ_soh_otmp125_END (5)
#define PMIC_ACR_SOH_IRQ_soh_30min_timeout_START (6)
#define PMIC_ACR_SOH_IRQ_soh_30min_timeout_END (6)
#define PMIC_ACR_SOH_IRQ_discharge_ocp_START (7)
#define PMIC_ACR_SOH_IRQ_discharge_ocp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_end : 1;
        unsigned char dcr_done2 : 1;
        unsigned char dcr_done1 : 1;
        unsigned char dcr_err2 : 1;
        unsigned char dcr_err1 : 1;
        unsigned char dcr_tout2 : 1;
        unsigned char dcr_tout1 : 1;
        unsigned char dcr_tout0 : 1;
    } reg;
} PMIC_DCR_IRQ_UNION;
#endif
#define PMIC_DCR_IRQ_dcr_end_START (0)
#define PMIC_DCR_IRQ_dcr_end_END (0)
#define PMIC_DCR_IRQ_dcr_done2_START (1)
#define PMIC_DCR_IRQ_dcr_done2_END (1)
#define PMIC_DCR_IRQ_dcr_done1_START (2)
#define PMIC_DCR_IRQ_dcr_done1_END (2)
#define PMIC_DCR_IRQ_dcr_err2_START (3)
#define PMIC_DCR_IRQ_dcr_err2_END (3)
#define PMIC_DCR_IRQ_dcr_err1_START (4)
#define PMIC_DCR_IRQ_dcr_err1_END (4)
#define PMIC_DCR_IRQ_dcr_tout2_START (5)
#define PMIC_DCR_IRQ_dcr_tout2_END (5)
#define PMIC_DCR_IRQ_dcr_tout1_START (6)
#define PMIC_DCR_IRQ_dcr_tout1_END (6)
#define PMIC_DCR_IRQ_dcr_tout0_START (7)
#define PMIC_DCR_IRQ_dcr_tout0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vsys_drop_r : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_VSYS_DROP_IRQ_UNION;
#endif
#define PMIC_VSYS_DROP_IRQ_vsys_drop_r_START (0)
#define PMIC_VSYS_DROP_IRQ_vsys_drop_r_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ocpbuck0 : 1;
        unsigned char np_ocpbuck1 : 1;
        unsigned char np_ocpbuck2 : 1;
        unsigned char np_ocpbuck3 : 1;
        unsigned char np_ocpbuck4 : 1;
        unsigned char np_ocpbuck5 : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_NP_OCP0_UNION;
#endif
#define PMIC_NP_OCP0_np_ocpbuck0_START (0)
#define PMIC_NP_OCP0_np_ocpbuck0_END (0)
#define PMIC_NP_OCP0_np_ocpbuck1_START (1)
#define PMIC_NP_OCP0_np_ocpbuck1_END (1)
#define PMIC_NP_OCP0_np_ocpbuck2_START (2)
#define PMIC_NP_OCP0_np_ocpbuck2_END (2)
#define PMIC_NP_OCP0_np_ocpbuck3_START (3)
#define PMIC_NP_OCP0_np_ocpbuck3_END (3)
#define PMIC_NP_OCP0_np_ocpbuck4_START (4)
#define PMIC_NP_OCP0_np_ocpbuck4_END (4)
#define PMIC_NP_OCP0_np_ocpbuck5_START (5)
#define PMIC_NP_OCP0_np_ocpbuck5_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ocpldo5 : 1;
        unsigned char np_ocpldo4 : 1;
        unsigned char np_ocpldo3 : 1;
        unsigned char np_ocpldo2 : 1;
        unsigned char np_ocpldo1 : 1;
        unsigned char np_ocpldo0_2 : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_NP_OCP1_UNION;
#endif
#define PMIC_NP_OCP1_np_ocpldo5_START (0)
#define PMIC_NP_OCP1_np_ocpldo5_END (0)
#define PMIC_NP_OCP1_np_ocpldo4_START (1)
#define PMIC_NP_OCP1_np_ocpldo4_END (1)
#define PMIC_NP_OCP1_np_ocpldo3_START (2)
#define PMIC_NP_OCP1_np_ocpldo3_END (2)
#define PMIC_NP_OCP1_np_ocpldo2_START (3)
#define PMIC_NP_OCP1_np_ocpldo2_END (3)
#define PMIC_NP_OCP1_np_ocpldo1_START (4)
#define PMIC_NP_OCP1_np_ocpldo1_END (4)
#define PMIC_NP_OCP1_np_ocpldo0_2_START (5)
#define PMIC_NP_OCP1_np_ocpldo0_2_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ocpldo15 : 1;
        unsigned char np_ocpldo14 : 1;
        unsigned char np_ocpldo13 : 1;
        unsigned char np_ocpldo12 : 1;
        unsigned char np_ocpldo11 : 1;
        unsigned char np_ocpldo9 : 1;
        unsigned char np_ocpldo8 : 1;
        unsigned char np_ocpldo6 : 1;
    } reg;
} PMIC_NP_OCP2_UNION;
#endif
#define PMIC_NP_OCP2_np_ocpldo15_START (0)
#define PMIC_NP_OCP2_np_ocpldo15_END (0)
#define PMIC_NP_OCP2_np_ocpldo14_START (1)
#define PMIC_NP_OCP2_np_ocpldo14_END (1)
#define PMIC_NP_OCP2_np_ocpldo13_START (2)
#define PMIC_NP_OCP2_np_ocpldo13_END (2)
#define PMIC_NP_OCP2_np_ocpldo12_START (3)
#define PMIC_NP_OCP2_np_ocpldo12_END (3)
#define PMIC_NP_OCP2_np_ocpldo11_START (4)
#define PMIC_NP_OCP2_np_ocpldo11_END (4)
#define PMIC_NP_OCP2_np_ocpldo9_START (5)
#define PMIC_NP_OCP2_np_ocpldo9_END (5)
#define PMIC_NP_OCP2_np_ocpldo8_START (6)
#define PMIC_NP_OCP2_np_ocpldo8_END (6)
#define PMIC_NP_OCP2_np_ocpldo6_START (7)
#define PMIC_NP_OCP2_np_ocpldo6_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ocpldo23 : 1;
        unsigned char np_ocpldo22 : 1;
        unsigned char np_ocpldo21 : 1;
        unsigned char np_ocpldo20 : 1;
        unsigned char np_ocpldo19 : 1;
        unsigned char np_ocpldo18 : 1;
        unsigned char np_ocpldo17 : 1;
        unsigned char np_ocpldo16 : 1;
    } reg;
} PMIC_NP_OCP3_UNION;
#endif
#define PMIC_NP_OCP3_np_ocpldo23_START (0)
#define PMIC_NP_OCP3_np_ocpldo23_END (0)
#define PMIC_NP_OCP3_np_ocpldo22_START (1)
#define PMIC_NP_OCP3_np_ocpldo22_END (1)
#define PMIC_NP_OCP3_np_ocpldo21_START (2)
#define PMIC_NP_OCP3_np_ocpldo21_END (2)
#define PMIC_NP_OCP3_np_ocpldo20_START (3)
#define PMIC_NP_OCP3_np_ocpldo20_END (3)
#define PMIC_NP_OCP3_np_ocpldo19_START (4)
#define PMIC_NP_OCP3_np_ocpldo19_END (4)
#define PMIC_NP_OCP3_np_ocpldo18_START (5)
#define PMIC_NP_OCP3_np_ocpldo18_END (5)
#define PMIC_NP_OCP3_np_ocpldo17_START (6)
#define PMIC_NP_OCP3_np_ocpldo17_END (6)
#define PMIC_NP_OCP3_np_ocpldo16_START (7)
#define PMIC_NP_OCP3_np_ocpldo16_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ocpldo31 : 1;
        unsigned char np_ocpldo30_2 : 1;
        unsigned char np_ocpldo29 : 1;
        unsigned char np_ocpldo28 : 1;
        unsigned char np_ocpldo27 : 1;
        unsigned char np_ocpldo26 : 1;
        unsigned char np_ocpldo25 : 1;
        unsigned char np_ocpldo24 : 1;
    } reg;
} PMIC_NP_OCP4_UNION;
#endif
#define PMIC_NP_OCP4_np_ocpldo31_START (0)
#define PMIC_NP_OCP4_np_ocpldo31_END (0)
#define PMIC_NP_OCP4_np_ocpldo30_2_START (1)
#define PMIC_NP_OCP4_np_ocpldo30_2_END (1)
#define PMIC_NP_OCP4_np_ocpldo29_START (2)
#define PMIC_NP_OCP4_np_ocpldo29_END (2)
#define PMIC_NP_OCP4_np_ocpldo28_START (3)
#define PMIC_NP_OCP4_np_ocpldo28_END (3)
#define PMIC_NP_OCP4_np_ocpldo27_START (4)
#define PMIC_NP_OCP4_np_ocpldo27_END (4)
#define PMIC_NP_OCP4_np_ocpldo26_START (5)
#define PMIC_NP_OCP4_np_ocpldo26_END (5)
#define PMIC_NP_OCP4_np_ocpldo25_START (6)
#define PMIC_NP_OCP4_np_ocpldo25_END (6)
#define PMIC_NP_OCP4_np_ocpldo24_START (7)
#define PMIC_NP_OCP4_np_ocpldo24_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ocpldo40 : 1;
        unsigned char np_ocpldo39 : 1;
        unsigned char np_ocpldo38 : 1;
        unsigned char np_ocpldo37 : 1;
        unsigned char np_pmuh_ocp : 1;
        unsigned char np_ocpldo34 : 1;
        unsigned char np_ocpldo33 : 1;
        unsigned char np_ocpldo32 : 1;
    } reg;
} PMIC_NP_OCP5_UNION;
#endif
#define PMIC_NP_OCP5_np_ocpldo40_START (0)
#define PMIC_NP_OCP5_np_ocpldo40_END (0)
#define PMIC_NP_OCP5_np_ocpldo39_START (1)
#define PMIC_NP_OCP5_np_ocpldo39_END (1)
#define PMIC_NP_OCP5_np_ocpldo38_START (2)
#define PMIC_NP_OCP5_np_ocpldo38_END (2)
#define PMIC_NP_OCP5_np_ocpldo37_START (3)
#define PMIC_NP_OCP5_np_ocpldo37_END (3)
#define PMIC_NP_OCP5_np_pmuh_ocp_START (4)
#define PMIC_NP_OCP5_np_pmuh_ocp_END (4)
#define PMIC_NP_OCP5_np_ocpldo34_START (5)
#define PMIC_NP_OCP5_np_ocpldo34_END (5)
#define PMIC_NP_OCP5_np_ocpldo33_START (6)
#define PMIC_NP_OCP5_np_ocpldo33_END (6)
#define PMIC_NP_OCP5_np_ocpldo32_START (7)
#define PMIC_NP_OCP5_np_ocpldo32_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_buck0_scp : 1;
        unsigned char np_buck1_scp : 1;
        unsigned char np_buck2_scp : 1;
        unsigned char np_buck3_scp : 1;
        unsigned char np_buck4_scp : 1;
        unsigned char np_buck5_scp : 1;
        unsigned char np_ldo_buff_scp : 1;
        unsigned char np_classd_ocp : 1;
    } reg;
} PMIC_NP_SCP0_UNION;
#endif
#define PMIC_NP_SCP0_np_buck0_scp_START (0)
#define PMIC_NP_SCP0_np_buck0_scp_END (0)
#define PMIC_NP_SCP0_np_buck1_scp_START (1)
#define PMIC_NP_SCP0_np_buck1_scp_END (1)
#define PMIC_NP_SCP0_np_buck2_scp_START (2)
#define PMIC_NP_SCP0_np_buck2_scp_END (2)
#define PMIC_NP_SCP0_np_buck3_scp_START (3)
#define PMIC_NP_SCP0_np_buck3_scp_END (3)
#define PMIC_NP_SCP0_np_buck4_scp_START (4)
#define PMIC_NP_SCP0_np_buck4_scp_END (4)
#define PMIC_NP_SCP0_np_buck5_scp_START (5)
#define PMIC_NP_SCP0_np_buck5_scp_END (5)
#define PMIC_NP_SCP0_np_ldo_buff_scp_START (6)
#define PMIC_NP_SCP0_np_ldo_buff_scp_END (6)
#define PMIC_NP_SCP0_np_classd_ocp_START (7)
#define PMIC_NP_SCP0_np_classd_ocp_END (7)
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
        unsigned char reserved : 1;
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
        unsigned char np_dcxo_clk_sel_r : 1;
        unsigned char np_dcxo_clk_sel_f : 1;
        unsigned char np_vsys_vcoin_sel : 1;
        unsigned char np_smpl : 1;
        unsigned char np_core_io_vld_f : 1;
        unsigned char np_pwrhold_4s : 1;
        unsigned char np_pwron_n_hrst : 1;
        unsigned char np_nfc_pwrup : 1;
    } reg;
} PMIC_NP_RECORD2_UNION;
#endif
#define PMIC_NP_RECORD2_np_dcxo_clk_sel_r_START (0)
#define PMIC_NP_RECORD2_np_dcxo_clk_sel_r_END (0)
#define PMIC_NP_RECORD2_np_dcxo_clk_sel_f_START (1)
#define PMIC_NP_RECORD2_np_dcxo_clk_sel_f_END (1)
#define PMIC_NP_RECORD2_np_vsys_vcoin_sel_START (2)
#define PMIC_NP_RECORD2_np_vsys_vcoin_sel_END (2)
#define PMIC_NP_RECORD2_np_smpl_START (3)
#define PMIC_NP_RECORD2_np_smpl_END (3)
#define PMIC_NP_RECORD2_np_core_io_vld_f_START (4)
#define PMIC_NP_RECORD2_np_core_io_vld_f_END (4)
#define PMIC_NP_RECORD2_np_pwrhold_4s_START (5)
#define PMIC_NP_RECORD2_np_pwrhold_4s_END (5)
#define PMIC_NP_RECORD2_np_pwron_n_hrst_START (6)
#define PMIC_NP_RECORD2_np_pwron_n_hrst_END (6)
#define PMIC_NP_RECORD2_np_nfc_pwrup_START (7)
#define PMIC_NP_RECORD2_np_nfc_pwrup_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_pmua_short_f : 1;
        unsigned char np_pmuh_short_f : 1;
        unsigned char np_vin_ldoh_shutdown : 1;
        unsigned char np_vsys_pwron_shutdown : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_NP_RECORD3_UNION;
#endif
#define PMIC_NP_RECORD3_np_pmua_short_f_START (0)
#define PMIC_NP_RECORD3_np_pmua_short_f_END (0)
#define PMIC_NP_RECORD3_np_pmuh_short_f_START (1)
#define PMIC_NP_RECORD3_np_pmuh_short_f_END (1)
#define PMIC_NP_RECORD3_np_vin_ldoh_shutdown_START (2)
#define PMIC_NP_RECORD3_np_vin_ldoh_shutdown_END (2)
#define PMIC_NP_RECORD3_np_vsys_pwron_shutdown_START (3)
#define PMIC_NP_RECORD3_np_vsys_pwron_shutdown_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_cali_pmuh_ocp : 1;
        unsigned char np_cali_ldo26_ocp : 1;
        unsigned char np_cali_buck3_scp : 1;
        unsigned char np_cali_buck3_ocp : 1;
        unsigned char np_cali_pmuh_short : 1;
        unsigned char np_cali_pmua_short : 1;
        unsigned char np_cali_vsys_pwroff_deb : 1;
        unsigned char np_cali_vsys_pwroff_abs : 1;
    } reg;
} PMIC_NP_RECORD4_UNION;
#endif
#define PMIC_NP_RECORD4_np_cali_pmuh_ocp_START (0)
#define PMIC_NP_RECORD4_np_cali_pmuh_ocp_END (0)
#define PMIC_NP_RECORD4_np_cali_ldo26_ocp_START (1)
#define PMIC_NP_RECORD4_np_cali_ldo26_ocp_END (1)
#define PMIC_NP_RECORD4_np_cali_buck3_scp_START (2)
#define PMIC_NP_RECORD4_np_cali_buck3_scp_END (2)
#define PMIC_NP_RECORD4_np_cali_buck3_ocp_START (3)
#define PMIC_NP_RECORD4_np_cali_buck3_ocp_END (3)
#define PMIC_NP_RECORD4_np_cali_pmuh_short_START (4)
#define PMIC_NP_RECORD4_np_cali_pmuh_short_END (4)
#define PMIC_NP_RECORD4_np_cali_pmua_short_START (5)
#define PMIC_NP_RECORD4_np_cali_pmua_short_END (5)
#define PMIC_NP_RECORD4_np_cali_vsys_pwroff_deb_START (6)
#define PMIC_NP_RECORD4_np_cali_vsys_pwroff_deb_END (6)
#define PMIC_NP_RECORD4_np_cali_vsys_pwroff_abs_START (7)
#define PMIC_NP_RECORD4_np_cali_vsys_pwroff_abs_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_cali_avdd_osc_vld : 1;
        unsigned char np_cali_thsd_otmp140 : 1;
        unsigned char np_cali_thsd_otmp125 : 1;
        unsigned char np_cali_vsys_ov : 1;
        unsigned char np_cali_19m2_dis : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_NP_RECORD5_UNION;
#endif
#define PMIC_NP_RECORD5_np_cali_avdd_osc_vld_START (0)
#define PMIC_NP_RECORD5_np_cali_avdd_osc_vld_END (0)
#define PMIC_NP_RECORD5_np_cali_thsd_otmp140_START (1)
#define PMIC_NP_RECORD5_np_cali_thsd_otmp140_END (1)
#define PMIC_NP_RECORD5_np_cali_thsd_otmp125_START (2)
#define PMIC_NP_RECORD5_np_cali_thsd_otmp125_END (2)
#define PMIC_NP_RECORD5_np_cali_vsys_ov_START (3)
#define PMIC_NP_RECORD5_np_cali_vsys_ov_END (3)
#define PMIC_NP_RECORD5_np_cali_19m2_dis_START (4)
#define PMIC_NP_RECORD5_np_cali_19m2_dis_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_buck0_ramp_abnor : 1;
        unsigned char np_buck4_ramp_abnor : 1;
        unsigned char np_buck5_ramp_abnor : 1;
        unsigned char np_ldo0_2_ramp_abnor : 1;
        unsigned char np_ldo40_ramp_abnor : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_RAMP_EVENT_UNION;
#endif
#define PMIC_RAMP_EVENT_np_buck0_ramp_abnor_START (0)
#define PMIC_RAMP_EVENT_np_buck0_ramp_abnor_END (0)
#define PMIC_RAMP_EVENT_np_buck4_ramp_abnor_START (1)
#define PMIC_RAMP_EVENT_np_buck4_ramp_abnor_END (1)
#define PMIC_RAMP_EVENT_np_buck5_ramp_abnor_START (2)
#define PMIC_RAMP_EVENT_np_buck5_ramp_abnor_END (2)
#define PMIC_RAMP_EVENT_np_ldo0_2_ramp_abnor_START (3)
#define PMIC_RAMP_EVENT_np_ldo0_2_ramp_abnor_END (3)
#define PMIC_RAMP_EVENT_np_ldo40_ramp_abnor_START (4)
#define PMIC_RAMP_EVENT_np_ldo40_ramp_abnor_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo26_ocp_ocv : 1;
        unsigned char np_buck3_scp_ocv : 1;
        unsigned char np_buck3_ocp_ocv : 1;
        unsigned char np_pmua_short_ocv : 1;
        unsigned char np_pmuh_short_ocv : 1;
        unsigned char np_vsys_pwroff_abs_d20nr_ocv : 1;
        unsigned char np_vsys_pwroff_deb_d80mr_ocv : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_COUL_RECORD0_UNION;
#endif
#define PMIC_COUL_RECORD0_np_ldo26_ocp_ocv_START (0)
#define PMIC_COUL_RECORD0_np_ldo26_ocp_ocv_END (0)
#define PMIC_COUL_RECORD0_np_buck3_scp_ocv_START (1)
#define PMIC_COUL_RECORD0_np_buck3_scp_ocv_END (1)
#define PMIC_COUL_RECORD0_np_buck3_ocp_ocv_START (2)
#define PMIC_COUL_RECORD0_np_buck3_ocp_ocv_END (2)
#define PMIC_COUL_RECORD0_np_pmua_short_ocv_START (3)
#define PMIC_COUL_RECORD0_np_pmua_short_ocv_END (3)
#define PMIC_COUL_RECORD0_np_pmuh_short_ocv_START (4)
#define PMIC_COUL_RECORD0_np_pmuh_short_ocv_END (4)
#define PMIC_COUL_RECORD0_np_vsys_pwroff_abs_d20nr_ocv_START (5)
#define PMIC_COUL_RECORD0_np_vsys_pwroff_abs_d20nr_ocv_END (5)
#define PMIC_COUL_RECORD0_np_vsys_pwroff_deb_d80mr_ocv_START (6)
#define PMIC_COUL_RECORD0_np_vsys_pwroff_deb_d80mr_ocv_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo27_ocp_ocv : 1;
        unsigned char np_pmuh_ocp_ocv : 1;
        unsigned char np_avdd_osc_vld_ocv : 1;
        unsigned char np_thsd_otmp140_ocv : 1;
        unsigned char np_thsd_otmp125_ocv : 1;
        unsigned char np_vsys_ov_ocv : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_COUL_RECORD1_UNION;
#endif
#define PMIC_COUL_RECORD1_np_ldo27_ocp_ocv_START (0)
#define PMIC_COUL_RECORD1_np_ldo27_ocp_ocv_END (0)
#define PMIC_COUL_RECORD1_np_pmuh_ocp_ocv_START (1)
#define PMIC_COUL_RECORD1_np_pmuh_ocp_ocv_END (1)
#define PMIC_COUL_RECORD1_np_avdd_osc_vld_ocv_START (2)
#define PMIC_COUL_RECORD1_np_avdd_osc_vld_ocv_END (2)
#define PMIC_COUL_RECORD1_np_thsd_otmp140_ocv_START (3)
#define PMIC_COUL_RECORD1_np_thsd_otmp140_ocv_END (3)
#define PMIC_COUL_RECORD1_np_thsd_otmp125_ocv_START (4)
#define PMIC_COUL_RECORD1_np_thsd_otmp125_ocv_END (4)
#define PMIC_COUL_RECORD1_np_vsys_ov_ocv_START (5)
#define PMIC_COUL_RECORD1_np_vsys_ov_ocv_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_vsys_drop_r : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_NP_VSYS_DROP_RECORD_UNION;
#endif
#define PMIC_NP_VSYS_DROP_RECORD_np_vsys_drop_r_START (0)
#define PMIC_NP_VSYS_DROP_RECORD_np_vsys_drop_r_END (0)
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
        unsigned char adc_mic3_en : 1;
        unsigned char sif_en : 1;
        unsigned char codec_ana_en : 1;
        unsigned char dac_ultr_en : 1;
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
#define PMIC_CLK_EN_CFG_adc_mic3_en_START (4)
#define PMIC_CLK_EN_CFG_adc_mic3_en_END (4)
#define PMIC_CLK_EN_CFG_sif_en_START (5)
#define PMIC_CLK_EN_CFG_sif_en_END (5)
#define PMIC_CLK_EN_CFG_codec_ana_en_START (6)
#define PMIC_CLK_EN_CFG_codec_ana_en_END (6)
#define PMIC_CLK_EN_CFG_dac_ultr_en_START (7)
#define PMIC_CLK_EN_CFG_dac_ultr_en_END (7)
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
        unsigned char adc_mic3_clk_edge_sel : 1;
        unsigned char dac_ultr_clk_edge_sel : 1;
        unsigned char reserved : 2;
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
#define PMIC_CLK_EDGE_CFG_adc_mic3_clk_edge_sel_START (4)
#define PMIC_CLK_EDGE_CFG_adc_mic3_clk_edge_sel_END (4)
#define PMIC_CLK_EDGE_CFG_dac_ultr_clk_edge_sel_START (5)
#define PMIC_CLK_EDGE_CFG_dac_ultr_clk_edge_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dac_loopback : 2;
        unsigned char adc_loopback : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_SIF_LOOPBACK_CFG_UNION;
#endif
#define PMIC_SIF_LOOPBACK_CFG_dac_loopback_START (0)
#define PMIC_SIF_LOOPBACK_CFG_dac_loopback_END (1)
#define PMIC_SIF_LOOPBACK_CFG_adc_loopback_START (2)
#define PMIC_SIF_LOOPBACK_CFG_adc_loopback_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dacl_dwa_en : 1;
        unsigned char dacr_dwa_en : 1;
        unsigned char dacl_din_sel : 2;
        unsigned char dacr_din_sel : 2;
        unsigned char dac_ultr_dwa_en : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_DAC_CHAN_CTRL_UNION;
#endif
#define PMIC_DAC_CHAN_CTRL_dacl_dwa_en_START (0)
#define PMIC_DAC_CHAN_CTRL_dacl_dwa_en_END (0)
#define PMIC_DAC_CHAN_CTRL_dacr_dwa_en_START (1)
#define PMIC_DAC_CHAN_CTRL_dacr_dwa_en_END (1)
#define PMIC_DAC_CHAN_CTRL_dacl_din_sel_START (2)
#define PMIC_DAC_CHAN_CTRL_dacl_din_sel_END (3)
#define PMIC_DAC_CHAN_CTRL_dacr_din_sel_START (4)
#define PMIC_DAC_CHAN_CTRL_dacr_din_sel_END (5)
#define PMIC_DAC_CHAN_CTRL_dac_ultr_dwa_en_START (6)
#define PMIC_DAC_CHAN_CTRL_dac_ultr_dwa_en_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char adcl_dout_sel : 2;
        unsigned char adcr_dout_sel : 2;
        unsigned char adc_mic3_dout_sel : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_ADC_CHAN_CTRL_UNION;
#endif
#define PMIC_ADC_CHAN_CTRL_adcl_dout_sel_START (0)
#define PMIC_ADC_CHAN_CTRL_adcl_dout_sel_END (1)
#define PMIC_ADC_CHAN_CTRL_adcr_dout_sel_START (2)
#define PMIC_ADC_CHAN_CTRL_adcr_dout_sel_END (3)
#define PMIC_ADC_CHAN_CTRL_adc_mic3_dout_sel_START (4)
#define PMIC_ADC_CHAN_CTRL_adc_mic3_dout_sel_END (5)
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
        unsigned char codec_ana_rw_01 : 8;
    } reg;
} PMIC_CODEC_ANA_RW1_UNION;
#endif
#define PMIC_CODEC_ANA_RW1_codec_ana_rw_01_START (0)
#define PMIC_CODEC_ANA_RW1_codec_ana_rw_01_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_02 : 8;
    } reg;
} PMIC_CODEC_ANA_RW2_UNION;
#endif
#define PMIC_CODEC_ANA_RW2_codec_ana_rw_02_START (0)
#define PMIC_CODEC_ANA_RW2_codec_ana_rw_02_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_03 : 8;
    } reg;
} PMIC_CODEC_ANA_RW3_UNION;
#endif
#define PMIC_CODEC_ANA_RW3_codec_ana_rw_03_START (0)
#define PMIC_CODEC_ANA_RW3_codec_ana_rw_03_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_04 : 8;
    } reg;
} PMIC_CODEC_ANA_RW4_UNION;
#endif
#define PMIC_CODEC_ANA_RW4_codec_ana_rw_04_START (0)
#define PMIC_CODEC_ANA_RW4_codec_ana_rw_04_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_05 : 8;
    } reg;
} PMIC_CODEC_ANA_RW5_UNION;
#endif
#define PMIC_CODEC_ANA_RW5_codec_ana_rw_05_START (0)
#define PMIC_CODEC_ANA_RW5_codec_ana_rw_05_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_06 : 8;
    } reg;
} PMIC_CODEC_ANA_RW6_UNION;
#endif
#define PMIC_CODEC_ANA_RW6_codec_ana_rw_06_START (0)
#define PMIC_CODEC_ANA_RW6_codec_ana_rw_06_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_07 : 8;
    } reg;
} PMIC_CODEC_ANA_RW7_UNION;
#endif
#define PMIC_CODEC_ANA_RW7_codec_ana_rw_07_START (0)
#define PMIC_CODEC_ANA_RW7_codec_ana_rw_07_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_08 : 8;
    } reg;
} PMIC_CODEC_ANA_RW8_UNION;
#endif
#define PMIC_CODEC_ANA_RW8_codec_ana_rw_08_START (0)
#define PMIC_CODEC_ANA_RW8_codec_ana_rw_08_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_09 : 8;
    } reg;
} PMIC_CODEC_ANA_RW9_UNION;
#endif
#define PMIC_CODEC_ANA_RW9_codec_ana_rw_09_START (0)
#define PMIC_CODEC_ANA_RW9_codec_ana_rw_09_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_10 : 8;
    } reg;
} PMIC_CODEC_ANA_RW10_UNION;
#endif
#define PMIC_CODEC_ANA_RW10_codec_ana_rw_10_START (0)
#define PMIC_CODEC_ANA_RW10_codec_ana_rw_10_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_11 : 8;
    } reg;
} PMIC_CODEC_ANA_RW11_UNION;
#endif
#define PMIC_CODEC_ANA_RW11_codec_ana_rw_11_START (0)
#define PMIC_CODEC_ANA_RW11_codec_ana_rw_11_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_12 : 8;
    } reg;
} PMIC_CODEC_ANA_RW12_UNION;
#endif
#define PMIC_CODEC_ANA_RW12_codec_ana_rw_12_START (0)
#define PMIC_CODEC_ANA_RW12_codec_ana_rw_12_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_13 : 8;
    } reg;
} PMIC_CODEC_ANA_RW13_UNION;
#endif
#define PMIC_CODEC_ANA_RW13_codec_ana_rw_13_START (0)
#define PMIC_CODEC_ANA_RW13_codec_ana_rw_13_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_14 : 8;
    } reg;
} PMIC_CODEC_ANA_RW14_UNION;
#endif
#define PMIC_CODEC_ANA_RW14_codec_ana_rw_14_START (0)
#define PMIC_CODEC_ANA_RW14_codec_ana_rw_14_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_15 : 8;
    } reg;
} PMIC_CODEC_ANA_RW15_UNION;
#endif
#define PMIC_CODEC_ANA_RW15_codec_ana_rw_15_START (0)
#define PMIC_CODEC_ANA_RW15_codec_ana_rw_15_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_16 : 8;
    } reg;
} PMIC_CODEC_ANA_RW16_UNION;
#endif
#define PMIC_CODEC_ANA_RW16_codec_ana_rw_16_START (0)
#define PMIC_CODEC_ANA_RW16_codec_ana_rw_16_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_17 : 8;
    } reg;
} PMIC_CODEC_ANA_RW17_UNION;
#endif
#define PMIC_CODEC_ANA_RW17_codec_ana_rw_17_START (0)
#define PMIC_CODEC_ANA_RW17_codec_ana_rw_17_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_18 : 8;
    } reg;
} PMIC_CODEC_ANA_RW18_UNION;
#endif
#define PMIC_CODEC_ANA_RW18_codec_ana_rw_18_START (0)
#define PMIC_CODEC_ANA_RW18_codec_ana_rw_18_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_19 : 8;
    } reg;
} PMIC_CODEC_ANA_RW19_UNION;
#endif
#define PMIC_CODEC_ANA_RW19_codec_ana_rw_19_START (0)
#define PMIC_CODEC_ANA_RW19_codec_ana_rw_19_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_20 : 8;
    } reg;
} PMIC_CODEC_ANA_RW20_UNION;
#endif
#define PMIC_CODEC_ANA_RW20_codec_ana_rw_20_START (0)
#define PMIC_CODEC_ANA_RW20_codec_ana_rw_20_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_21 : 8;
    } reg;
} PMIC_CODEC_ANA_RW21_UNION;
#endif
#define PMIC_CODEC_ANA_RW21_codec_ana_rw_21_START (0)
#define PMIC_CODEC_ANA_RW21_codec_ana_rw_21_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_22 : 8;
    } reg;
} PMIC_CODEC_ANA_RW22_UNION;
#endif
#define PMIC_CODEC_ANA_RW22_codec_ana_rw_22_START (0)
#define PMIC_CODEC_ANA_RW22_codec_ana_rw_22_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_23 : 8;
    } reg;
} PMIC_CODEC_ANA_RW23_UNION;
#endif
#define PMIC_CODEC_ANA_RW23_codec_ana_rw_23_START (0)
#define PMIC_CODEC_ANA_RW23_codec_ana_rw_23_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_24 : 8;
    } reg;
} PMIC_CODEC_ANA_RW24_UNION;
#endif
#define PMIC_CODEC_ANA_RW24_codec_ana_rw_24_START (0)
#define PMIC_CODEC_ANA_RW24_codec_ana_rw_24_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_25 : 8;
    } reg;
} PMIC_CODEC_ANA_RW25_UNION;
#endif
#define PMIC_CODEC_ANA_RW25_codec_ana_rw_25_START (0)
#define PMIC_CODEC_ANA_RW25_codec_ana_rw_25_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_26 : 8;
    } reg;
} PMIC_CODEC_ANA_RW26_UNION;
#endif
#define PMIC_CODEC_ANA_RW26_codec_ana_rw_26_START (0)
#define PMIC_CODEC_ANA_RW26_codec_ana_rw_26_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_27 : 8;
    } reg;
} PMIC_CODEC_ANA_RW27_UNION;
#endif
#define PMIC_CODEC_ANA_RW27_codec_ana_rw_27_START (0)
#define PMIC_CODEC_ANA_RW27_codec_ana_rw_27_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_28 : 8;
    } reg;
} PMIC_CODEC_ANA_RW28_UNION;
#endif
#define PMIC_CODEC_ANA_RW28_codec_ana_rw_28_START (0)
#define PMIC_CODEC_ANA_RW28_codec_ana_rw_28_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_29 : 8;
    } reg;
} PMIC_CODEC_ANA_RW29_UNION;
#endif
#define PMIC_CODEC_ANA_RW29_codec_ana_rw_29_START (0)
#define PMIC_CODEC_ANA_RW29_codec_ana_rw_29_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_30 : 8;
    } reg;
} PMIC_CODEC_ANA_RW30_UNION;
#endif
#define PMIC_CODEC_ANA_RW30_codec_ana_rw_30_START (0)
#define PMIC_CODEC_ANA_RW30_codec_ana_rw_30_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_31 : 8;
    } reg;
} PMIC_CODEC_ANA_RW31_UNION;
#endif
#define PMIC_CODEC_ANA_RW31_codec_ana_rw_31_START (0)
#define PMIC_CODEC_ANA_RW31_codec_ana_rw_31_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_32 : 8;
    } reg;
} PMIC_CODEC_ANA_RW32_UNION;
#endif
#define PMIC_CODEC_ANA_RW32_codec_ana_rw_32_START (0)
#define PMIC_CODEC_ANA_RW32_codec_ana_rw_32_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_33 : 8;
    } reg;
} PMIC_CODEC_ANA_RW33_UNION;
#endif
#define PMIC_CODEC_ANA_RW33_codec_ana_rw_33_START (0)
#define PMIC_CODEC_ANA_RW33_codec_ana_rw_33_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_34 : 8;
    } reg;
} PMIC_CODEC_ANA_RW34_UNION;
#endif
#define PMIC_CODEC_ANA_RW34_codec_ana_rw_34_START (0)
#define PMIC_CODEC_ANA_RW34_codec_ana_rw_34_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_35 : 8;
    } reg;
} PMIC_CODEC_ANA_RW35_UNION;
#endif
#define PMIC_CODEC_ANA_RW35_codec_ana_rw_35_START (0)
#define PMIC_CODEC_ANA_RW35_codec_ana_rw_35_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_36 : 8;
    } reg;
} PMIC_CODEC_ANA_RW36_UNION;
#endif
#define PMIC_CODEC_ANA_RW36_codec_ana_rw_36_START (0)
#define PMIC_CODEC_ANA_RW36_codec_ana_rw_36_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_37 : 8;
    } reg;
} PMIC_CODEC_ANA_RW37_UNION;
#endif
#define PMIC_CODEC_ANA_RW37_codec_ana_rw_37_START (0)
#define PMIC_CODEC_ANA_RW37_codec_ana_rw_37_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_38 : 8;
    } reg;
} PMIC_CODEC_ANA_RW38_UNION;
#endif
#define PMIC_CODEC_ANA_RW38_codec_ana_rw_38_START (0)
#define PMIC_CODEC_ANA_RW38_codec_ana_rw_38_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_39 : 8;
    } reg;
} PMIC_CODEC_ANA_RW39_UNION;
#endif
#define PMIC_CODEC_ANA_RW39_codec_ana_rw_39_START (0)
#define PMIC_CODEC_ANA_RW39_codec_ana_rw_39_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_40 : 8;
    } reg;
} PMIC_CODEC_ANA_RW40_UNION;
#endif
#define PMIC_CODEC_ANA_RW40_codec_ana_rw_40_START (0)
#define PMIC_CODEC_ANA_RW40_codec_ana_rw_40_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_41 : 8;
    } reg;
} PMIC_CODEC_ANA_RW41_UNION;
#endif
#define PMIC_CODEC_ANA_RW41_codec_ana_rw_41_START (0)
#define PMIC_CODEC_ANA_RW41_codec_ana_rw_41_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_42 : 8;
    } reg;
} PMIC_CODEC_ANA_RW42_UNION;
#endif
#define PMIC_CODEC_ANA_RW42_codec_ana_rw_42_START (0)
#define PMIC_CODEC_ANA_RW42_codec_ana_rw_42_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_43 : 8;
    } reg;
} PMIC_CODEC_ANA_RW43_UNION;
#endif
#define PMIC_CODEC_ANA_RW43_codec_ana_rw_43_START (0)
#define PMIC_CODEC_ANA_RW43_codec_ana_rw_43_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_44 : 8;
    } reg;
} PMIC_CODEC_ANA_RW44_UNION;
#endif
#define PMIC_CODEC_ANA_RW44_codec_ana_rw_44_START (0)
#define PMIC_CODEC_ANA_RW44_codec_ana_rw_44_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_45 : 8;
    } reg;
} PMIC_CODEC_ANA_RW45_UNION;
#endif
#define PMIC_CODEC_ANA_RW45_codec_ana_rw_45_START (0)
#define PMIC_CODEC_ANA_RW45_codec_ana_rw_45_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_46 : 8;
    } reg;
} PMIC_CODEC_ANA_RW46_UNION;
#endif
#define PMIC_CODEC_ANA_RW46_codec_ana_rw_46_START (0)
#define PMIC_CODEC_ANA_RW46_codec_ana_rw_46_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_47 : 8;
    } reg;
} PMIC_CODEC_ANA_RW47_UNION;
#endif
#define PMIC_CODEC_ANA_RW47_codec_ana_rw_47_START (0)
#define PMIC_CODEC_ANA_RW47_codec_ana_rw_47_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_48 : 8;
    } reg;
} PMIC_CODEC_ANA_RW48_UNION;
#endif
#define PMIC_CODEC_ANA_RW48_codec_ana_rw_48_START (0)
#define PMIC_CODEC_ANA_RW48_codec_ana_rw_48_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_49 : 8;
    } reg;
} PMIC_CODEC_ANA_RW49_UNION;
#endif
#define PMIC_CODEC_ANA_RW49_codec_ana_rw_49_START (0)
#define PMIC_CODEC_ANA_RW49_codec_ana_rw_49_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_50 : 8;
    } reg;
} PMIC_CODEC_ANA_RW50_UNION;
#endif
#define PMIC_CODEC_ANA_RW50_codec_ana_rw_50_START (0)
#define PMIC_CODEC_ANA_RW50_codec_ana_rw_50_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_51 : 8;
    } reg;
} PMIC_CODEC_ANA_RW51_UNION;
#endif
#define PMIC_CODEC_ANA_RW51_codec_ana_rw_51_START (0)
#define PMIC_CODEC_ANA_RW51_codec_ana_rw_51_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_52 : 8;
    } reg;
} PMIC_CODEC_ANA_RW52_UNION;
#endif
#define PMIC_CODEC_ANA_RW52_codec_ana_rw_52_START (0)
#define PMIC_CODEC_ANA_RW52_codec_ana_rw_52_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_ro_01 : 8;
    } reg;
} PMIC_CODEC_ANA_RO01_UNION;
#endif
#define PMIC_CODEC_ANA_RO01_codec_ana_ro_01_START (0)
#define PMIC_CODEC_ANA_RO01_codec_ana_ro_01_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_ro_02 : 8;
    } reg;
} PMIC_CODEC_ANA_RO02_UNION;
#endif
#define PMIC_CODEC_ANA_RO02_codec_ana_ro_02_START (0)
#define PMIC_CODEC_ANA_RO02_codec_ana_ro_02_END (7)
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
        unsigned char ser_rtcdr0 : 8;
    } reg;
} PMIC_SER_RTCDR0_UNION;
#endif
#define PMIC_SER_RTCDR0_ser_rtcdr0_START (0)
#define PMIC_SER_RTCDR0_ser_rtcdr0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_rtcdr1 : 8;
    } reg;
} PMIC_SER_RTCDR1_UNION;
#endif
#define PMIC_SER_RTCDR1_ser_rtcdr1_START (0)
#define PMIC_SER_RTCDR1_ser_rtcdr1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_rtcdr2 : 8;
    } reg;
} PMIC_SER_RTCDR2_UNION;
#endif
#define PMIC_SER_RTCDR2_ser_rtcdr2_START (0)
#define PMIC_SER_RTCDR2_ser_rtcdr2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_rtcdr3 : 8;
    } reg;
} PMIC_SER_RTCDR3_UNION;
#endif
#define PMIC_SER_RTCDR3_ser_rtcdr3_START (0)
#define PMIC_SER_RTCDR3_ser_rtcdr3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_rtcmr0 : 8;
    } reg;
} PMIC_SER_RTCMR0_UNION;
#endif
#define PMIC_SER_RTCMR0_ser_rtcmr0_START (0)
#define PMIC_SER_RTCMR0_ser_rtcmr0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_rtcmr1 : 8;
    } reg;
} PMIC_SER_RTCMR1_UNION;
#endif
#define PMIC_SER_RTCMR1_ser_rtcmr1_START (0)
#define PMIC_SER_RTCMR1_ser_rtcmr1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_rtcmr2 : 8;
    } reg;
} PMIC_SER_RTCMR2_UNION;
#endif
#define PMIC_SER_RTCMR2_ser_rtcmr2_START (0)
#define PMIC_SER_RTCMR2_ser_rtcmr2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_rtcmr3 : 8;
    } reg;
} PMIC_SER_RTCMR3_UNION;
#endif
#define PMIC_SER_RTCMR3_ser_rtcmr3_START (0)
#define PMIC_SER_RTCMR3_ser_rtcmr3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_rtcclr0 : 8;
    } reg;
} PMIC_SER_RTCLR0_UNION;
#endif
#define PMIC_SER_RTCLR0_ser_rtcclr0_START (0)
#define PMIC_SER_RTCLR0_ser_rtcclr0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_rtcclr1 : 8;
    } reg;
} PMIC_SER_RTCLR1_UNION;
#endif
#define PMIC_SER_RTCLR1_ser_rtcclr1_START (0)
#define PMIC_SER_RTCLR1_ser_rtcclr1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_rtcclr2 : 8;
    } reg;
} PMIC_SER_RTCLR2_UNION;
#endif
#define PMIC_SER_RTCLR2_ser_rtcclr2_START (0)
#define PMIC_SER_RTCLR2_ser_rtcclr2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_rtcclr3 : 8;
    } reg;
} PMIC_SER_RTCLR3_UNION;
#endif
#define PMIC_SER_RTCLR3_ser_rtcclr3_START (0)
#define PMIC_SER_RTCLR3_ser_rtcclr3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_rtccr : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_SER_RTCCTRL_UNION;
#endif
#define PMIC_SER_RTCCTRL_ser_rtccr_START (0)
#define PMIC_SER_RTCCTRL_ser_rtccr_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_xo_cali_thresold_low : 8;
    } reg;
} PMIC_SER_XO_THRESOLD0_UNION;
#endif
#define PMIC_SER_XO_THRESOLD0_ser_xo_cali_thresold_low_START (0)
#define PMIC_SER_XO_THRESOLD0_ser_xo_cali_thresold_low_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_xo_cali_thresold_high : 8;
    } reg;
} PMIC_SER_XO_THRESOLD1_UNION;
#endif
#define PMIC_SER_XO_THRESOLD1_ser_xo_cali_thresold_high_START (0)
#define PMIC_SER_XO_THRESOLD1_ser_xo_cali_thresold_high_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_crc_value0 : 8;
    } reg;
} PMIC_SER_CRC_VAULE0_UNION;
#endif
#define PMIC_SER_CRC_VAULE0_ser_crc_value0_START (0)
#define PMIC_SER_CRC_VAULE0_ser_crc_value0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_crc_value1 : 8;
    } reg;
} PMIC_SER_CRC_VAULE1_UNION;
#endif
#define PMIC_SER_CRC_VAULE1_ser_crc_value1_START (0)
#define PMIC_SER_CRC_VAULE1_ser_crc_value1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_crc_value2 : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_SER_CRC_VAULE2_UNION;
#endif
#define PMIC_SER_CRC_VAULE2_ser_crc_value2_START (0)
#define PMIC_SER_CRC_VAULE2_ser_crc_value2_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_rtc_pwrup_timer0 : 8;
    } reg;
} PMIC_SER_RTC_PWRUP_TIMER0_UNION;
#endif
#define PMIC_SER_RTC_PWRUP_TIMER0_ser_rtc_pwrup_timer0_START (0)
#define PMIC_SER_RTC_PWRUP_TIMER0_ser_rtc_pwrup_timer0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_rtc_pwrup_timer1 : 8;
    } reg;
} PMIC_SER_RTC_PWRUP_TIMER1_UNION;
#endif
#define PMIC_SER_RTC_PWRUP_TIMER1_ser_rtc_pwrup_timer1_START (0)
#define PMIC_SER_RTC_PWRUP_TIMER1_ser_rtc_pwrup_timer1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_rtc_pwrup_timer2 : 8;
    } reg;
} PMIC_SER_RTC_PWRUP_TIMER2_UNION;
#endif
#define PMIC_SER_RTC_PWRUP_TIMER2_ser_rtc_pwrup_timer2_START (0)
#define PMIC_SER_RTC_PWRUP_TIMER2_ser_rtc_pwrup_timer2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_rtc_pwrup_timer3 : 8;
    } reg;
} PMIC_SER_RTC_PWRUP_TIMER3_UNION;
#endif
#define PMIC_SER_RTC_PWRUP_TIMER3_ser_rtc_pwrup_timer3_START (0)
#define PMIC_SER_RTC_PWRUP_TIMER3_ser_rtc_pwrup_timer3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_rtc_pwrdown_timer0 : 8;
    } reg;
} PMIC_SER_RTC_PWRDOWN_TIMER0_UNION;
#endif
#define PMIC_SER_RTC_PWRDOWN_TIMER0_ser_rtc_pwrdown_timer0_START (0)
#define PMIC_SER_RTC_PWRDOWN_TIMER0_ser_rtc_pwrdown_timer0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_rtc_pwrdown_timer1 : 8;
    } reg;
} PMIC_SER_RTC_PWRDOWN_TIMER1_UNION;
#endif
#define PMIC_SER_RTC_PWRDOWN_TIMER1_ser_rtc_pwrdown_timer1_START (0)
#define PMIC_SER_RTC_PWRDOWN_TIMER1_ser_rtc_pwrdown_timer1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_rtc_pwrdown_timer2 : 8;
    } reg;
} PMIC_SER_RTC_PWRDOWN_TIMER2_UNION;
#endif
#define PMIC_SER_RTC_PWRDOWN_TIMER2_ser_rtc_pwrdown_timer2_START (0)
#define PMIC_SER_RTC_PWRDOWN_TIMER2_ser_rtc_pwrdown_timer2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ser_rtc_pwrdown_timer3 : 8;
    } reg;
} PMIC_SER_RTC_PWRDOWN_TIMER3_UNION;
#endif
#define PMIC_SER_RTC_PWRDOWN_TIMER3_ser_rtc_pwrdown_timer3_START (0)
#define PMIC_SER_RTC_PWRDOWN_TIMER3_ser_rtc_pwrdown_timer3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pwrup_cali_end : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_PWRUP_CALI_END_UNION;
#endif
#define PMIC_PWRUP_CALI_END_pwrup_cali_end_START (0)
#define PMIC_PWRUP_CALI_END_pwrup_cali_end_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_autocali_ave0_l : 8;
    } reg;
} PMIC_XOADC_AUTOCALI_AVE0_UNION;
#endif
#define PMIC_XOADC_AUTOCALI_AVE0_xo_autocali_ave0_l_START (0)
#define PMIC_XOADC_AUTOCALI_AVE0_xo_autocali_ave0_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_autocali_ave0_h : 8;
    } reg;
} PMIC_XOADC_AUTOCALI_AVE1_UNION;
#endif
#define PMIC_XOADC_AUTOCALI_AVE1_xo_autocali_ave0_h_START (0)
#define PMIC_XOADC_AUTOCALI_AVE1_xo_autocali_ave0_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_autocali_ave1_l : 8;
    } reg;
} PMIC_XOADC_AUTOCALI_AVE2_UNION;
#endif
#define PMIC_XOADC_AUTOCALI_AVE2_xo_autocali_ave1_l_START (0)
#define PMIC_XOADC_AUTOCALI_AVE2_xo_autocali_ave1_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_autocali_ave1_h : 8;
    } reg;
} PMIC_XOADC_AUTOCALI_AVE3_UNION;
#endif
#define PMIC_XOADC_AUTOCALI_AVE3_xo_autocali_ave1_h_START (0)
#define PMIC_XOADC_AUTOCALI_AVE3_xo_autocali_ave1_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xoadc_data_rate_sel : 3;
        unsigned char xoadc_sdm_clk_sel : 1;
        unsigned char xo_chop_fre_sel : 2;
        unsigned char xo_chop_en : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_XOADC_CTRL_UNION;
#endif
#define PMIC_XOADC_CTRL_xoadc_data_rate_sel_START (0)
#define PMIC_XOADC_CTRL_xoadc_data_rate_sel_END (2)
#define PMIC_XOADC_CTRL_xoadc_sdm_clk_sel_START (3)
#define PMIC_XOADC_CTRL_xoadc_sdm_clk_sel_END (3)
#define PMIC_XOADC_CTRL_xo_chop_fre_sel_START (4)
#define PMIC_XOADC_CTRL_xo_chop_fre_sel_END (5)
#define PMIC_XOADC_CTRL_xo_chop_en_START (6)
#define PMIC_XOADC_CTRL_xo_chop_en_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xoadc_samp_phase_sel : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_XOADC_SAMP_PHASE_UNION;
#endif
#define PMIC_XOADC_SAMP_PHASE_xoadc_samp_phase_sel_START (0)
#define PMIC_XOADC_SAMP_PHASE_xoadc_samp_phase_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xoadc_opt_0 : 8;
    } reg;
} PMIC_XOADC_OPT_0_UNION;
#endif
#define PMIC_XOADC_OPT_0_xoadc_opt_0_START (0)
#define PMIC_XOADC_OPT_0_xoadc_opt_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xoadc_opt_1 : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_XOADC_OPT_1_UNION;
#endif
#define PMIC_XOADC_OPT_1_xoadc_opt_1_START (0)
#define PMIC_XOADC_OPT_1_xoadc_opt_1_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xoadc_ain_sel : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_XOADC_AIN_SEL_UNION;
#endif
#define PMIC_XOADC_AIN_SEL_xoadc_ain_sel_START (0)
#define PMIC_XOADC_AIN_SEL_xoadc_ain_sel_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char wifi_xoadc_ana_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_XOADC_WIFI_ANA_EN_UNION;
#endif
#define PMIC_XOADC_WIFI_ANA_EN_wifi_xoadc_ana_en_START (0)
#define PMIC_XOADC_WIFI_ANA_EN_wifi_xoadc_ana_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soc_initial : 1;
        unsigned char soc_xoadc_ana_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_XOADC_SOC_ANA_EN_UNION;
#endif
#define PMIC_XOADC_SOC_ANA_EN_soc_initial_START (0)
#define PMIC_XOADC_SOC_ANA_EN_soc_initial_END (0)
#define PMIC_XOADC_SOC_ANA_EN_soc_xoadc_ana_en_START (1)
#define PMIC_XOADC_SOC_ANA_EN_soc_xoadc_ana_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data_valid : 1;
        unsigned char soc_initial_ro : 1;
        unsigned char soc_xoadc_ana_en_ro : 1;
        unsigned char wifi_xoadc_ana_en_ro : 1;
        unsigned char soc_xo_cfg_en_ro : 2;
        unsigned char wifi_xo_cfg_en_ro : 2;
    } reg;
} PMIC_XOADC_STATE_UNION;
#endif
#define PMIC_XOADC_STATE_data_valid_START (0)
#define PMIC_XOADC_STATE_data_valid_END (0)
#define PMIC_XOADC_STATE_soc_initial_ro_START (1)
#define PMIC_XOADC_STATE_soc_initial_ro_END (1)
#define PMIC_XOADC_STATE_soc_xoadc_ana_en_ro_START (2)
#define PMIC_XOADC_STATE_soc_xoadc_ana_en_ro_END (2)
#define PMIC_XOADC_STATE_wifi_xoadc_ana_en_ro_START (3)
#define PMIC_XOADC_STATE_wifi_xoadc_ana_en_ro_END (3)
#define PMIC_XOADC_STATE_soc_xo_cfg_en_ro_START (4)
#define PMIC_XOADC_STATE_soc_xo_cfg_en_ro_END (5)
#define PMIC_XOADC_STATE_wifi_xo_cfg_en_ro_START (6)
#define PMIC_XOADC_STATE_wifi_xo_cfg_en_ro_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_data_0 : 8;
    } reg;
} PMIC_XOADC_DATA0_UNION;
#endif
#define PMIC_XOADC_DATA0_xo_data_0_START (0)
#define PMIC_XOADC_DATA0_xo_data_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_data_1 : 8;
    } reg;
} PMIC_XOADC_DATA1_UNION;
#endif
#define PMIC_XOADC_DATA1_xo_data_1_START (0)
#define PMIC_XOADC_DATA1_xo_data_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_cali_data_0 : 8;
    } reg;
} PMIC_XOADC_CALI_DATA0_UNION;
#endif
#define PMIC_XOADC_CALI_DATA0_xo_cali_data_0_START (0)
#define PMIC_XOADC_CALI_DATA0_xo_cali_data_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_cali_data_1 : 8;
    } reg;
} PMIC_XOADC_CALI_DATA1_UNION;
#endif
#define PMIC_XOADC_CALI_DATA1_xo_cali_data_1_START (0)
#define PMIC_XOADC_CALI_DATA1_xo_cali_data_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soc_xo_cfg_en : 2;
        unsigned char wifi_xo_cfg_en : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_XOADC_CFG_EN_UNION;
#endif
#define PMIC_XOADC_CFG_EN_soc_xo_cfg_en_START (0)
#define PMIC_XOADC_CFG_EN_soc_xo_cfg_en_END (1)
#define PMIC_XOADC_CFG_EN_wifi_xo_cfg_en_START (2)
#define PMIC_XOADC_CFG_EN_wifi_xo_cfg_en_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char abnm_cfg : 2;
        unsigned char data_valid_ori : 1;
        unsigned char abnm_info : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_XOADC_ARB_DEBUG_UNION;
#endif
#define PMIC_XOADC_ARB_DEBUG_abnm_cfg_START (0)
#define PMIC_XOADC_ARB_DEBUG_abnm_cfg_END (1)
#define PMIC_XOADC_ARB_DEBUG_data_valid_ori_START (2)
#define PMIC_XOADC_ARB_DEBUG_data_valid_ori_END (2)
#define PMIC_XOADC_ARB_DEBUG_abnm_info_START (3)
#define PMIC_XOADC_ARB_DEBUG_abnm_info_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xoadc_data_rate_sel_s : 3;
        unsigned char xoadc_sdm_clk_sel_s : 1;
        unsigned char xo_chop_fre_sel_s : 2;
        unsigned char xo_chop_en_s : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_XOADC_CTRL_S_UNION;
#endif
#define PMIC_XOADC_CTRL_S_xoadc_data_rate_sel_s_START (0)
#define PMIC_XOADC_CTRL_S_xoadc_data_rate_sel_s_END (2)
#define PMIC_XOADC_CTRL_S_xoadc_sdm_clk_sel_s_START (3)
#define PMIC_XOADC_CTRL_S_xoadc_sdm_clk_sel_s_END (3)
#define PMIC_XOADC_CTRL_S_xo_chop_fre_sel_s_START (4)
#define PMIC_XOADC_CTRL_S_xo_chop_fre_sel_s_END (5)
#define PMIC_XOADC_CTRL_S_xo_chop_en_s_START (6)
#define PMIC_XOADC_CTRL_S_xo_chop_en_s_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xoadc_samp_phase_sel_s : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_XOADC_SAMP_PHASE_S_UNION;
#endif
#define PMIC_XOADC_SAMP_PHASE_S_xoadc_samp_phase_sel_s_START (0)
#define PMIC_XOADC_SAMP_PHASE_S_xoadc_samp_phase_sel_s_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xoadc_opt_0_s : 8;
    } reg;
} PMIC_XOADC_OPT_0_S_UNION;
#endif
#define PMIC_XOADC_OPT_0_S_xoadc_opt_0_s_START (0)
#define PMIC_XOADC_OPT_0_S_xoadc_opt_0_s_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xoadc_opt_1_s : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_XOADC_OPT_1_S_UNION;
#endif
#define PMIC_XOADC_OPT_1_S_xoadc_opt_1_s_START (0)
#define PMIC_XOADC_OPT_1_S_xoadc_opt_1_s_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xoadc_ain_sel_s : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_XOADC_AIN_SEL_S_UNION;
#endif
#define PMIC_XOADC_AIN_SEL_S_xoadc_ain_sel_s_START (0)
#define PMIC_XOADC_AIN_SEL_S_xoadc_ain_sel_s_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xoadc_ana_en_s : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_XOADC_ANA_EN_S_UNION;
#endif
#define PMIC_XOADC_ANA_EN_S_xoadc_ana_en_s_START (0)
#define PMIC_XOADC_ANA_EN_S_xoadc_ana_en_s_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soft_cfg0 : 8;
    } reg;
} PMIC_XOADC_SOFT_CFG0_UNION;
#endif
#define PMIC_XOADC_SOFT_CFG0_soft_cfg0_START (0)
#define PMIC_XOADC_SOFT_CFG0_soft_cfg0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soft_cfg1 : 8;
    } reg;
} PMIC_XOADC_SOFT_CFG1_UNION;
#endif
#define PMIC_XOADC_SOFT_CFG1_soft_cfg1_START (0)
#define PMIC_XOADC_SOFT_CFG1_soft_cfg1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soft_cfg2 : 8;
    } reg;
} PMIC_XOADC_SOFT_CFG2_UNION;
#endif
#define PMIC_XOADC_SOFT_CFG2_soft_cfg2_START (0)
#define PMIC_XOADC_SOFT_CFG2_soft_cfg2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soft_cfg3 : 8;
    } reg;
} PMIC_XOADC_SOFT_CFG3_UNION;
#endif
#define PMIC_XOADC_SOFT_CFG3_soft_cfg3_START (0)
#define PMIC_XOADC_SOFT_CFG3_soft_cfg3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soft_cfg4 : 8;
    } reg;
} PMIC_XOADC_SOFT_CFG4_UNION;
#endif
#define PMIC_XOADC_SOFT_CFG4_soft_cfg4_START (0)
#define PMIC_XOADC_SOFT_CFG4_soft_cfg4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soft_cfg5 : 8;
    } reg;
} PMIC_XOADC_SOFT_CFG5_UNION;
#endif
#define PMIC_XOADC_SOFT_CFG5_soft_cfg5_START (0)
#define PMIC_XOADC_SOFT_CFG5_soft_cfg5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soft_cfg6 : 8;
    } reg;
} PMIC_XOADC_SOFT_CFG6_UNION;
#endif
#define PMIC_XOADC_SOFT_CFG6_soft_cfg6_START (0)
#define PMIC_XOADC_SOFT_CFG6_soft_cfg6_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soft_cfg7 : 8;
    } reg;
} PMIC_XOADC_SOFT_CFG7_UNION;
#endif
#define PMIC_XOADC_SOFT_CFG7_soft_cfg7_START (0)
#define PMIC_XOADC_SOFT_CFG7_soft_cfg7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xoadc_reserve : 8;
    } reg;
} PMIC_XOADC_RESERVE_UNION;
#endif
#define PMIC_XOADC_RESERVE_xoadc_reserve_START (0)
#define PMIC_XOADC_RESERVE_xoadc_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hi1103_rdata_out0 : 8;
    } reg;
} PMIC_HI1103_RDATA_OUT0_UNION;
#endif
#define PMIC_HI1103_RDATA_OUT0_hi1103_rdata_out0_START (0)
#define PMIC_HI1103_RDATA_OUT0_hi1103_rdata_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hi1103_rdata_out1 : 8;
    } reg;
} PMIC_HI1103_RDATA_OUT1_UNION;
#endif
#define PMIC_HI1103_RDATA_OUT1_hi1103_rdata_out1_START (0)
#define PMIC_HI1103_RDATA_OUT1_hi1103_rdata_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hi1103_rdata_out2 : 8;
    } reg;
} PMIC_HI1103_RDATA_OUT2_UNION;
#endif
#define PMIC_HI1103_RDATA_OUT2_hi1103_rdata_out2_START (0)
#define PMIC_HI1103_RDATA_OUT2_hi1103_rdata_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hi1103_rdata_out3 : 8;
    } reg;
} PMIC_HI1103_RDATA_OUT3_UNION;
#endif
#define PMIC_HI1103_RDATA_OUT3_hi1103_rdata_out3_START (0)
#define PMIC_HI1103_RDATA_OUT3_hi1103_rdata_out3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hi1103_rdata_out4 : 8;
    } reg;
} PMIC_HI1103_RDATA_OUT4_UNION;
#endif
#define PMIC_HI1103_RDATA_OUT4_hi1103_rdata_out4_START (0)
#define PMIC_HI1103_RDATA_OUT4_hi1103_rdata_out4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hi1103_rdata_out5 : 8;
    } reg;
} PMIC_HI1103_RDATA_OUT5_UNION;
#endif
#define PMIC_HI1103_RDATA_OUT5_hi1103_rdata_out5_START (0)
#define PMIC_HI1103_RDATA_OUT5_hi1103_rdata_out5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hi1103_rdata_out6 : 8;
    } reg;
} PMIC_HI1103_RDATA_OUT6_UNION;
#endif
#define PMIC_HI1103_RDATA_OUT6_hi1103_rdata_out6_START (0)
#define PMIC_HI1103_RDATA_OUT6_hi1103_rdata_out6_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hi1103_rdata_out7 : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_HI1103_RDATA_OUT7_UNION;
#endif
#define PMIC_HI1103_RDATA_OUT7_hi1103_rdata_out7_START (0)
#define PMIC_HI1103_RDATA_OUT7_hi1103_rdata_out7_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_load_flag : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_RTC_LOAD_FLAG_UNION;
#endif
#define PMIC_RTC_LOAD_FLAG_rtc_load_flag_START (0)
#define PMIC_RTC_LOAD_FLAG_rtc_load_flag_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hi1103_refresh_data : 8;
    } reg;
} PMIC_HI1103_REFRESH_LOCK_UNION;
#endif
#define PMIC_HI1103_REFRESH_LOCK_hi1103_refresh_data_START (0)
#define PMIC_HI1103_REFRESH_LOCK_hi1103_refresh_data_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spmi_debug0 : 8;
    } reg;
} PMIC_SPMI_DEBUG0_UNION;
#endif
#define PMIC_SPMI_DEBUG0_spmi_debug0_START (0)
#define PMIC_SPMI_DEBUG0_spmi_debug0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spmi_debug1 : 8;
    } reg;
} PMIC_SPMI_DEBUG1_UNION;
#endif
#define PMIC_SPMI_DEBUG1_spmi_debug1_START (0)
#define PMIC_SPMI_DEBUG1_spmi_debug1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spmi_debug2 : 8;
    } reg;
} PMIC_SPMI_DEBUG2_UNION;
#endif
#define PMIC_SPMI_DEBUG2_spmi_debug2_START (0)
#define PMIC_SPMI_DEBUG2_spmi_debug2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spmi_debug3 : 8;
    } reg;
} PMIC_SPMI_DEBUG3_UNION;
#endif
#define PMIC_SPMI_DEBUG3_spmi_debug3_START (0)
#define PMIC_SPMI_DEBUG3_spmi_debug3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spmi_debug4 : 8;
    } reg;
} PMIC_SPMI_DEBUG4_UNION;
#endif
#define PMIC_SPMI_DEBUG4_spmi_debug4_START (0)
#define PMIC_SPMI_DEBUG4_spmi_debug4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spmi_debug5 : 8;
    } reg;
} PMIC_SPMI_DEBUG5_UNION;
#endif
#define PMIC_SPMI_DEBUG5_spmi_debug5_START (0)
#define PMIC_SPMI_DEBUG5_spmi_debug5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spmi_debug6 : 8;
    } reg;
} PMIC_SPMI_DEBUG6_UNION;
#endif
#define PMIC_SPMI_DEBUG6_spmi_debug6_START (0)
#define PMIC_SPMI_DEBUG6_spmi_debug6_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spmi_debug7 : 8;
    } reg;
} PMIC_SPMI_DEBUG7_UNION;
#endif
#define PMIC_SPMI_DEBUG7_spmi_debug7_START (0)
#define PMIC_SPMI_DEBUG7_spmi_debug7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xoadc_ssi_ds_sel : 1;
        unsigned char auxdac_ssi_ds_sel : 1;
        unsigned char spmi_data_ds_sel : 1;
        unsigned char sys_rst_n_ds_sel : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_DIG_IO_DS_SEL_UNION;
#endif
#define PMIC_DIG_IO_DS_SEL_xoadc_ssi_ds_sel_START (0)
#define PMIC_DIG_IO_DS_SEL_xoadc_ssi_ds_sel_END (0)
#define PMIC_DIG_IO_DS_SEL_auxdac_ssi_ds_sel_START (1)
#define PMIC_DIG_IO_DS_SEL_auxdac_ssi_ds_sel_END (1)
#define PMIC_DIG_IO_DS_SEL_spmi_data_ds_sel_START (2)
#define PMIC_DIG_IO_DS_SEL_spmi_data_ds_sel_END (2)
#define PMIC_DIG_IO_DS_SEL_sys_rst_n_ds_sel_START (3)
#define PMIC_DIG_IO_DS_SEL_sys_rst_n_ds_sel_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_xoadc_ssi_ds : 2;
        unsigned char np_auxdac_ssi_ds : 2;
        unsigned char np_spmi_data_ds : 2;
        unsigned char np_sys_rst_n_ds : 2;
    } reg;
} PMIC_DIG_IO_DS_CFG0_UNION;
#endif
#define PMIC_DIG_IO_DS_CFG0_np_xoadc_ssi_ds_START (0)
#define PMIC_DIG_IO_DS_CFG0_np_xoadc_ssi_ds_END (1)
#define PMIC_DIG_IO_DS_CFG0_np_auxdac_ssi_ds_START (2)
#define PMIC_DIG_IO_DS_CFG0_np_auxdac_ssi_ds_END (3)
#define PMIC_DIG_IO_DS_CFG0_np_spmi_data_ds_START (4)
#define PMIC_DIG_IO_DS_CFG0_np_spmi_data_ds_END (5)
#define PMIC_DIG_IO_DS_CFG0_np_sys_rst_n_ds_START (6)
#define PMIC_DIG_IO_DS_CFG0_np_sys_rst_n_ds_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_pmu0_irq_n_ds : 2;
        unsigned char np_vbat_drop_irq_ds : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_DIG_IO_DS_CFG1_UNION;
#endif
#define PMIC_DIG_IO_DS_CFG1_np_pmu0_irq_n_ds_START (0)
#define PMIC_DIG_IO_DS_CFG1_np_pmu0_irq_n_ds_END (1)
#define PMIC_DIG_IO_DS_CFG1_np_vbat_drop_irq_ds_START (2)
#define PMIC_DIG_IO_DS_CFG1_np_vbat_drop_irq_ds_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soc_rdata_out0 : 8;
    } reg;
} PMIC_SOC_RDATA_OUT0_UNION;
#endif
#define PMIC_SOC_RDATA_OUT0_soc_rdata_out0_START (0)
#define PMIC_SOC_RDATA_OUT0_soc_rdata_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soc_rdata_out1 : 8;
    } reg;
} PMIC_SOC_RDATA_OUT1_UNION;
#endif
#define PMIC_SOC_RDATA_OUT1_soc_rdata_out1_START (0)
#define PMIC_SOC_RDATA_OUT1_soc_rdata_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soc_rdata_out2 : 8;
    } reg;
} PMIC_SOC_RDATA_OUT2_UNION;
#endif
#define PMIC_SOC_RDATA_OUT2_soc_rdata_out2_START (0)
#define PMIC_SOC_RDATA_OUT2_soc_rdata_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soc_rdata_out3 : 8;
    } reg;
} PMIC_SOC_RDATA_OUT3_UNION;
#endif
#define PMIC_SOC_RDATA_OUT3_soc_rdata_out3_START (0)
#define PMIC_SOC_RDATA_OUT3_soc_rdata_out3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soc_rdata_out4 : 8;
    } reg;
} PMIC_SOC_RDATA_OUT4_UNION;
#endif
#define PMIC_SOC_RDATA_OUT4_soc_rdata_out4_START (0)
#define PMIC_SOC_RDATA_OUT4_soc_rdata_out4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soc_rdata_out5 : 8;
    } reg;
} PMIC_SOC_RDATA_OUT5_UNION;
#endif
#define PMIC_SOC_RDATA_OUT5_soc_rdata_out5_START (0)
#define PMIC_SOC_RDATA_OUT5_soc_rdata_out5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soc_rdata_out6 : 8;
    } reg;
} PMIC_SOC_RDATA_OUT6_UNION;
#endif
#define PMIC_SOC_RDATA_OUT6_soc_rdata_out6_START (0)
#define PMIC_SOC_RDATA_OUT6_soc_rdata_out6_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soc_rdata_out7 : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_SOC_RDATA_OUT7_UNION;
#endif
#define PMIC_SOC_RDATA_OUT7_soc_rdata_out7_START (0)
#define PMIC_SOC_RDATA_OUT7_soc_rdata_out7_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soc_refresh_data : 8;
    } reg;
} PMIC_SOC_REFRESH_LOCK_UNION;
#endif
#define PMIC_SOC_REFRESH_LOCK_soc_refresh_data_START (0)
#define PMIC_SOC_REFRESH_LOCK_soc_refresh_data_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_chanl_sel : 5;
        unsigned char reserved : 2;
        unsigned char hkadc_bypass : 1;
    } reg;
} PMIC_ADC_CTRL_UNION;
#endif
#define PMIC_ADC_CTRL_hkadc_chanl_sel_START (0)
#define PMIC_ADC_CTRL_hkadc_chanl_sel_END (4)
#define PMIC_ADC_CTRL_hkadc_bypass_START (7)
#define PMIC_ADC_CTRL_hkadc_bypass_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_start : 1;
        unsigned char hkadc_reserve : 7;
    } reg;
} PMIC_ADC_START_UNION;
#endif
#define PMIC_ADC_START_hkadc_start_START (0)
#define PMIC_ADC_START_hkadc_start_END (0)
#define PMIC_ADC_START_hkadc_reserve_START (1)
#define PMIC_ADC_START_hkadc_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_valid : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_CONV_STATUS_UNION;
#endif
#define PMIC_CONV_STATUS_hkadc_valid_START (0)
#define PMIC_CONV_STATUS_hkadc_valid_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_data11_4 : 8;
    } reg;
} PMIC_ADC_DATA1_UNION;
#endif
#define PMIC_ADC_DATA1_hkadc_data11_4_START (0)
#define PMIC_ADC_DATA1_hkadc_data11_4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char hkadc_data3_0 : 4;
    } reg;
} PMIC_ADC_DATA0_UNION;
#endif
#define PMIC_ADC_DATA0_hkadc_data3_0_START (4)
#define PMIC_ADC_DATA0_hkadc_data3_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_buffer_sel : 1;
        unsigned char hkadc_config : 7;
    } reg;
} PMIC_ADC_CONV_UNION;
#endif
#define PMIC_ADC_CONV_hkadc_buffer_sel_START (0)
#define PMIC_ADC_CONV_hkadc_buffer_sel_END (0)
#define PMIC_ADC_CONV_hkadc_config_START (1)
#define PMIC_ADC_CONV_hkadc_config_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_ibias_sel : 8;
    } reg;
} PMIC_ADC_CURRENT_UNION;
#endif
#define PMIC_ADC_CURRENT_hkadc_ibias_sel_START (0)
#define PMIC_ADC_CURRENT_hkadc_ibias_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_cali_en : 1;
        unsigned char hkadc_cali_sel : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_ADC_CALI_CTRL_UNION;
#endif
#define PMIC_ADC_CALI_CTRL_hkadc_cali_en_START (0)
#define PMIC_ADC_CALI_CTRL_hkadc_cali_en_END (0)
#define PMIC_ADC_CALI_CTRL_hkadc_cali_sel_START (1)
#define PMIC_ADC_CALI_CTRL_hkadc_cali_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_cali_data : 8;
    } reg;
} PMIC_ADC_CALI_VALUE_UNION;
#endif
#define PMIC_ADC_CALI_VALUE_hkadc_cali_data_START (0)
#define PMIC_ADC_CALI_VALUE_hkadc_cali_data_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_cali_cfg : 8;
    } reg;
} PMIC_ADC_CALI_CFG_UNION;
#endif
#define PMIC_ADC_CALI_CFG_hkadc_cali_cfg_START (0)
#define PMIC_ADC_CALI_CFG_hkadc_cali_cfg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_chopper_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_ADC_MODE_CFG_UNION;
#endif
#define PMIC_ADC_MODE_CFG_hkadc_chopper_en_START (0)
#define PMIC_ADC_MODE_CFG_hkadc_chopper_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_chopper_1st_data11_4 : 8;
    } reg;
} PMIC_ADC_CHOPPER_1ST_DATA1_UNION;
#endif
#define PMIC_ADC_CHOPPER_1ST_DATA1_hkadc_chopper_1st_data11_4_START (0)
#define PMIC_ADC_CHOPPER_1ST_DATA1_hkadc_chopper_1st_data11_4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char hkadc_chopper_1st_data3_0 : 4;
    } reg;
} PMIC_ADC_CHOPPER_1ST_DATA2_UNION;
#endif
#define PMIC_ADC_CHOPPER_1ST_DATA2_hkadc_chopper_1st_data3_0_START (4)
#define PMIC_ADC_CHOPPER_1ST_DATA2_hkadc_chopper_1st_data3_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_chopper_2nd_data11_4 : 8;
    } reg;
} PMIC_ADC_CHOPPER_2ND_DATA1_UNION;
#endif
#define PMIC_ADC_CHOPPER_2ND_DATA1_hkadc_chopper_2nd_data11_4_START (0)
#define PMIC_ADC_CHOPPER_2ND_DATA1_hkadc_chopper_2nd_data11_4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char hkadc_chopper_2nd_data3_0 : 4;
    } reg;
} PMIC_ADC_CHOPPER_2ND_DATA2_UNION;
#endif
#define PMIC_ADC_CHOPPER_2ND_DATA2_hkadc_chopper_2nd_data3_0_START (4)
#define PMIC_ADC_CHOPPER_2ND_DATA2_hkadc_chopper_2nd_data3_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_cali_offset_cfg11_4 : 8;
    } reg;
} PMIC_ADC_CALIVALUE_CFG1_UNION;
#endif
#define PMIC_ADC_CALIVALUE_CFG1_hkadc_cali_offset_cfg11_4_START (0)
#define PMIC_ADC_CALIVALUE_CFG1_hkadc_cali_offset_cfg11_4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char hkadc_cali_offset_cfg3_0 : 4;
    } reg;
} PMIC_ADC_CALIVALUE_CFG2_UNION;
#endif
#define PMIC_ADC_CALIVALUE_CFG2_hkadc_cali_offset_cfg3_0_START (4)
#define PMIC_ADC_CALIVALUE_CFG2_hkadc_cali_offset_cfg3_0_END (7)
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
        unsigned char i_in_int : 1;
        unsigned char i_out_int : 1;
        unsigned char reserved : 2;
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
#define PMIC_COUL_IRQ_i_in_int_START (4)
#define PMIC_COUL_IRQ_i_in_int_END (4)
#define PMIC_COUL_IRQ_i_out_int_START (5)
#define PMIC_COUL_IRQ_i_out_int_END (5)
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
        unsigned char i_in_int_mk : 1;
        unsigned char i_out_int_mk : 1;
        unsigned char reserved : 2;
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
#define PMIC_COUL_IRQ_MASK_i_in_int_mk_START (4)
#define PMIC_COUL_IRQ_MASK_i_in_int_mk_END (4)
#define PMIC_COUL_IRQ_MASK_i_out_int_mk_START (5)
#define PMIC_COUL_IRQ_MASK_i_out_int_mk_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_i_in_s : 1;
        unsigned char np_i_out_s : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_COUL_IRQ_NP_UNION;
#endif
#define PMIC_COUL_IRQ_NP_np_i_in_s_START (0)
#define PMIC_COUL_IRQ_NP_np_i_in_s_END (0)
#define PMIC_COUL_IRQ_NP_np_i_out_s_START (1)
#define PMIC_COUL_IRQ_NP_np_i_out_s_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_ctrl : 3;
        unsigned char reflash_value_ctrl : 1;
        unsigned char eco_filter_time : 2;
        unsigned char reserved : 1;
        unsigned char calibration_ctrl : 1;
    } reg;
} PMIC_CLJ_CTRL_REG_UNION;
#endif
#define PMIC_CLJ_CTRL_REG_eco_ctrl_START (0)
#define PMIC_CLJ_CTRL_REG_eco_ctrl_END (2)
#define PMIC_CLJ_CTRL_REG_reflash_value_ctrl_START (3)
#define PMIC_CLJ_CTRL_REG_reflash_value_ctrl_END (3)
#define PMIC_CLJ_CTRL_REG_eco_filter_time_START (4)
#define PMIC_CLJ_CTRL_REG_eco_filter_time_END (5)
#define PMIC_CLJ_CTRL_REG_calibration_ctrl_START (7)
#define PMIC_CLJ_CTRL_REG_calibration_ctrl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_ocv_i_onoff : 1;
        unsigned char reg_data_clr : 1;
        unsigned char cali_auto_time : 2;
        unsigned char cali_auto_onoff_ctrl : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_CLJ_CTRL_REGS2_UNION;
#endif
#define PMIC_CLJ_CTRL_REGS2_pd_ocv_i_onoff_START (0)
#define PMIC_CLJ_CTRL_REGS2_pd_ocv_i_onoff_END (0)
#define PMIC_CLJ_CTRL_REGS2_reg_data_clr_START (1)
#define PMIC_CLJ_CTRL_REGS2_reg_data_clr_END (1)
#define PMIC_CLJ_CTRL_REGS2_cali_auto_time_START (2)
#define PMIC_CLJ_CTRL_REGS2_cali_auto_time_END (3)
#define PMIC_CLJ_CTRL_REGS2_cali_auto_onoff_ctrl_START (4)
#define PMIC_CLJ_CTRL_REGS2_cali_auto_onoff_ctrl_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_delay_en : 1;
        unsigned char coul_eco_dly_sel : 2;
        unsigned char wait_comp_en : 1;
        unsigned char wait_comp_sel : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_CLJ_CTRL_REGS3_UNION;
#endif
#define PMIC_CLJ_CTRL_REGS3_eco_delay_en_START (0)
#define PMIC_CLJ_CTRL_REGS3_eco_delay_en_END (0)
#define PMIC_CLJ_CTRL_REGS3_coul_eco_dly_sel_START (1)
#define PMIC_CLJ_CTRL_REGS3_coul_eco_dly_sel_END (2)
#define PMIC_CLJ_CTRL_REGS3_wait_comp_en_START (3)
#define PMIC_CLJ_CTRL_REGS3_wait_comp_en_END (3)
#define PMIC_CLJ_CTRL_REGS3_wait_comp_sel_START (4)
#define PMIC_CLJ_CTRL_REGS3_wait_comp_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data_eco_en : 1;
        unsigned char eco_data_clr : 1;
        unsigned char coul_charge_en : 1;
        unsigned char coul_charge_flag : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_CLJ_CTRL_REGS4_UNION;
#endif
#define PMIC_CLJ_CTRL_REGS4_data_eco_en_START (0)
#define PMIC_CLJ_CTRL_REGS4_data_eco_en_END (0)
#define PMIC_CLJ_CTRL_REGS4_eco_data_clr_START (1)
#define PMIC_CLJ_CTRL_REGS4_eco_data_clr_END (1)
#define PMIC_CLJ_CTRL_REGS4_coul_charge_en_START (2)
#define PMIC_CLJ_CTRL_REGS4_coul_charge_en_END (2)
#define PMIC_CLJ_CTRL_REGS4_coul_charge_flag_START (3)
#define PMIC_CLJ_CTRL_REGS4_coul_charge_flag_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_en : 1;
        unsigned char slide_refresh_en : 1;
        unsigned char slide_cnt_cfg : 2;
        unsigned char slide_ready : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_CLJ_CTRL_REGS5_UNION;
#endif
#define PMIC_CLJ_CTRL_REGS5_slide_en_START (0)
#define PMIC_CLJ_CTRL_REGS5_slide_en_END (0)
#define PMIC_CLJ_CTRL_REGS5_slide_refresh_en_START (1)
#define PMIC_CLJ_CTRL_REGS5_slide_refresh_en_END (1)
#define PMIC_CLJ_CTRL_REGS5_slide_cnt_cfg_START (2)
#define PMIC_CLJ_CTRL_REGS5_slide_cnt_cfg_END (3)
#define PMIC_CLJ_CTRL_REGS5_slide_ready_START (4)
#define PMIC_CLJ_CTRL_REGS5_slide_ready_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ocv_average_sel : 1;
        unsigned char coul_average_sel : 1;
        unsigned char coul_data_rate_sel : 1;
        unsigned char debug_refresh_en : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_CIC_CTRL_UNION;
#endif
#define PMIC_CIC_CTRL_ocv_average_sel_START (0)
#define PMIC_CIC_CTRL_ocv_average_sel_END (0)
#define PMIC_CIC_CTRL_coul_average_sel_START (1)
#define PMIC_CIC_CTRL_coul_average_sel_END (1)
#define PMIC_CIC_CTRL_coul_data_rate_sel_START (2)
#define PMIC_CIC_CTRL_coul_data_rate_sel_END (2)
#define PMIC_CIC_CTRL_debug_refresh_en_START (3)
#define PMIC_CIC_CTRL_debug_refresh_en_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char temp_en : 1;
        unsigned char temp_rdy : 1;
        unsigned char vout_rdy : 1;
        unsigned char coul_vin_sel_st : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_COUL_TEMP_CTRL_UNION;
#endif
#define PMIC_COUL_TEMP_CTRL_temp_en_START (0)
#define PMIC_COUL_TEMP_CTRL_temp_en_END (0)
#define PMIC_COUL_TEMP_CTRL_temp_rdy_START (1)
#define PMIC_COUL_TEMP_CTRL_temp_rdy_END (1)
#define PMIC_COUL_TEMP_CTRL_vout_rdy_START (2)
#define PMIC_COUL_TEMP_CTRL_vout_rdy_END (2)
#define PMIC_COUL_TEMP_CTRL_coul_vin_sel_st_START (3)
#define PMIC_COUL_TEMP_CTRL_coul_vin_sel_st_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char coul_v_decode : 2;
        unsigned char coul_i_decode : 2;
        unsigned char coul_v_pga_gain : 1;
        unsigned char coul_i_pga_gain : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_COUL_VI_CTRL0_UNION;
#endif
#define PMIC_COUL_VI_CTRL0_coul_v_decode_START (0)
#define PMIC_COUL_VI_CTRL0_coul_v_decode_END (1)
#define PMIC_COUL_VI_CTRL0_coul_i_decode_START (2)
#define PMIC_COUL_VI_CTRL0_coul_i_decode_END (3)
#define PMIC_COUL_VI_CTRL0_coul_v_pga_gain_START (4)
#define PMIC_COUL_VI_CTRL0_coul_v_pga_gain_END (4)
#define PMIC_COUL_VI_CTRL0_coul_i_pga_gain_START (5)
#define PMIC_COUL_VI_CTRL0_coul_i_pga_gain_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char coul_v_chop_enb : 2;
        unsigned char coul_i_chop_enb : 2;
        unsigned char coul_v_alias_en : 1;
        unsigned char coul_i_alias_en : 1;
        unsigned char coul_v_comp_sel : 1;
        unsigned char coul_i_comp_sel : 1;
    } reg;
} PMIC_COUL_VI_CTRL1_UNION;
#endif
#define PMIC_COUL_VI_CTRL1_coul_v_chop_enb_START (0)
#define PMIC_COUL_VI_CTRL1_coul_v_chop_enb_END (1)
#define PMIC_COUL_VI_CTRL1_coul_i_chop_enb_START (2)
#define PMIC_COUL_VI_CTRL1_coul_i_chop_enb_END (3)
#define PMIC_COUL_VI_CTRL1_coul_v_alias_en_START (4)
#define PMIC_COUL_VI_CTRL1_coul_v_alias_en_END (4)
#define PMIC_COUL_VI_CTRL1_coul_i_alias_en_START (5)
#define PMIC_COUL_VI_CTRL1_coul_i_alias_en_END (5)
#define PMIC_COUL_VI_CTRL1_coul_v_comp_sel_START (6)
#define PMIC_COUL_VI_CTRL1_coul_v_comp_sel_END (6)
#define PMIC_COUL_VI_CTRL1_coul_i_comp_sel_START (7)
#define PMIC_COUL_VI_CTRL1_coul_i_comp_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char coul_v_chop_freq_sel : 4;
        unsigned char coul_i_chop_freq_sel : 4;
    } reg;
} PMIC_COUL_VI_CTRL2_UNION;
#endif
#define PMIC_COUL_VI_CTRL2_coul_v_chop_freq_sel_START (0)
#define PMIC_COUL_VI_CTRL2_coul_v_chop_freq_sel_END (3)
#define PMIC_COUL_VI_CTRL2_coul_i_chop_freq_sel_START (4)
#define PMIC_COUL_VI_CTRL2_coul_i_chop_freq_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char coul_i_system_chop_enb : 1;
        unsigned char coul_v_bias_buff : 2;
        unsigned char coul_i_bias_buff : 2;
        unsigned char coul_dt_ctrl : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_COUL_VI_CTRL3_UNION;
#endif
#define PMIC_COUL_VI_CTRL3_coul_i_system_chop_enb_START (0)
#define PMIC_COUL_VI_CTRL3_coul_i_system_chop_enb_END (0)
#define PMIC_COUL_VI_CTRL3_coul_v_bias_buff_START (1)
#define PMIC_COUL_VI_CTRL3_coul_v_bias_buff_END (2)
#define PMIC_COUL_VI_CTRL3_coul_i_bias_buff_START (3)
#define PMIC_COUL_VI_CTRL3_coul_i_bias_buff_END (4)
#define PMIC_COUL_VI_CTRL3_coul_dt_ctrl_START (5)
#define PMIC_COUL_VI_CTRL3_coul_dt_ctrl_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char coul_i_bias : 8;
    } reg;
} PMIC_COUL_VI_CTRL4_UNION;
#endif
#define PMIC_COUL_VI_CTRL4_coul_i_bias_START (0)
#define PMIC_COUL_VI_CTRL4_coul_i_bias_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char coul_v_bias : 8;
    } reg;
} PMIC_COUL_VI_CTRL5_UNION;
#endif
#define PMIC_COUL_VI_CTRL5_coul_v_bias_START (0)
#define PMIC_COUL_VI_CTRL5_coul_v_bias_END (7)
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
        unsigned char cl_out_s4 : 8;
    } reg;
} PMIC_CL_OUT4_UNION;
#endif
#define PMIC_CL_OUT4_cl_out_s4_START (0)
#define PMIC_CL_OUT4_cl_out_s4_END (7)
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
        unsigned char cl_in_s4 : 8;
    } reg;
} PMIC_CL_IN4_UNION;
#endif
#define PMIC_CL_IN4_cl_in_s4_START (0)
#define PMIC_CL_IN4_cl_in_s4_END (7)
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
        unsigned char cl_int_s4 : 8;
    } reg;
} PMIC_CL_INT4_UNION;
#endif
#define PMIC_CL_INT4_cl_int_s4_START (0)
#define PMIC_CL_INT4_cl_int_s4_END (7)
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
        unsigned char v_int_s2 : 8;
    } reg;
} PMIC_V_INT2_UNION;
#endif
#define PMIC_V_INT2_v_int_s2_START (0)
#define PMIC_V_INT2_v_int_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_out_gate_s0 : 8;
    } reg;
} PMIC_I_OUT_GATE0_UNION;
#endif
#define PMIC_I_OUT_GATE0_i_out_gate_s0_START (0)
#define PMIC_I_OUT_GATE0_i_out_gate_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_out_gate_s1 : 8;
    } reg;
} PMIC_I_OUT_GATE1_UNION;
#endif
#define PMIC_I_OUT_GATE1_i_out_gate_s1_START (0)
#define PMIC_I_OUT_GATE1_i_out_gate_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_out_gate_s2 : 8;
    } reg;
} PMIC_I_OUT_GATE2_UNION;
#endif
#define PMIC_I_OUT_GATE2_i_out_gate_s2_START (0)
#define PMIC_I_OUT_GATE2_i_out_gate_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_in_gate_s0 : 8;
    } reg;
} PMIC_I_IN_GATE0_UNION;
#endif
#define PMIC_I_IN_GATE0_i_in_gate_s0_START (0)
#define PMIC_I_IN_GATE0_i_in_gate_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_in_gate_s1 : 8;
    } reg;
} PMIC_I_IN_GATE1_UNION;
#endif
#define PMIC_I_IN_GATE1_i_in_gate_s1_START (0)
#define PMIC_I_IN_GATE1_i_in_gate_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_in_gate_s2 : 8;
    } reg;
} PMIC_I_IN_GATE2_UNION;
#endif
#define PMIC_I_IN_GATE2_i_in_gate_s2_START (0)
#define PMIC_I_IN_GATE2_i_in_gate_s2_END (7)
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
        unsigned char offset_current_mod_s2 : 8;
    } reg;
} PMIC_OFFSET_CURRENT_MOD_2_UNION;
#endif
#define PMIC_OFFSET_CURRENT_MOD_2_offset_current_mod_s2_START (0)
#define PMIC_OFFSET_CURRENT_MOD_2_offset_current_mod_s2_END (7)
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
        unsigned char offset_voltage_mod_s2 : 8;
    } reg;
} PMIC_OFFSET_VOLTAGE_MOD_2_UNION;
#endif
#define PMIC_OFFSET_VOLTAGE_MOD_2_offset_voltage_mod_s2_START (0)
#define PMIC_OFFSET_VOLTAGE_MOD_2_offset_voltage_mod_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char mstate : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_STATE_TEST_UNION;
#endif
#define PMIC_STATE_TEST_mstate_START (0)
#define PMIC_STATE_TEST_mstate_END (3)
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
        unsigned char current_s2 : 8;
    } reg;
} PMIC_CURRENT_2_UNION;
#endif
#define PMIC_CURRENT_2_current_s2_START (0)
#define PMIC_CURRENT_2_current_s2_END (7)
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
        unsigned char v_out_s2 : 8;
    } reg;
} PMIC_V_OUT_2_UNION;
#endif
#define PMIC_V_OUT_2_v_out_s2_START (0)
#define PMIC_V_OUT_2_v_out_s2_END (7)
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
        unsigned char offset_current_s2 : 8;
    } reg;
} PMIC_OFFSET_CURRENT2_UNION;
#endif
#define PMIC_OFFSET_CURRENT2_offset_current_s2_START (0)
#define PMIC_OFFSET_CURRENT2_offset_current_s2_END (7)
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
        unsigned char offset_voltage_s2 : 8;
    } reg;
} PMIC_OFFSET_VOLTAGE2_UNION;
#endif
#define PMIC_OFFSET_VOLTAGE2_offset_voltage_s2_START (0)
#define PMIC_OFFSET_VOLTAGE2_offset_voltage_s2_END (7)
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
        unsigned char v_ocv_data_s2 : 8;
    } reg;
} PMIC_OCV_VOLTAGE2_UNION;
#endif
#define PMIC_OCV_VOLTAGE2_v_ocv_data_s2_START (0)
#define PMIC_OCV_VOLTAGE2_v_ocv_data_s2_END (7)
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
        unsigned char i_ocv_data_s2 : 8;
    } reg;
} PMIC_OCV_CURRENT2_UNION;
#endif
#define PMIC_OCV_CURRENT2_i_ocv_data_s2_START (0)
#define PMIC_OCV_CURRENT2_i_ocv_data_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_data_s0 : 8;
    } reg;
} PMIC_OCV_TEMP0_UNION;
#endif
#define PMIC_OCV_TEMP0_t_ocv_data_s0_START (0)
#define PMIC_OCV_TEMP0_t_ocv_data_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_data_s1 : 8;
    } reg;
} PMIC_OCV_TEMP1_UNION;
#endif
#define PMIC_OCV_TEMP1_t_ocv_data_s1_START (0)
#define PMIC_OCV_TEMP1_t_ocv_data_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_data_s2 : 8;
    } reg;
} PMIC_OCV_TEMP2_UNION;
#endif
#define PMIC_OCV_TEMP2_t_ocv_data_s2_START (0)
#define PMIC_OCV_TEMP2_t_ocv_data_s2_END (7)
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
        unsigned char eco_out_clin_s4 : 8;
    } reg;
} PMIC_ECO_OUT_CLIN_4_UNION;
#endif
#define PMIC_ECO_OUT_CLIN_4_eco_out_clin_s4_START (0)
#define PMIC_ECO_OUT_CLIN_4_eco_out_clin_s4_END (7)
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
        unsigned char eco_out_clout_s4 : 8;
    } reg;
} PMIC_ECO_OUT_CLOUT_4_UNION;
#endif
#define PMIC_ECO_OUT_CLOUT_4_eco_out_clout_s4_START (0)
#define PMIC_ECO_OUT_CLOUT_4_eco_out_clout_s4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_out_temp_s0 : 8;
    } reg;
} PMIC_ECO_OUT_TEMP_0_UNION;
#endif
#define PMIC_ECO_OUT_TEMP_0_eco_out_temp_s0_START (0)
#define PMIC_ECO_OUT_TEMP_0_eco_out_temp_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_out_temp_s1 : 8;
    } reg;
} PMIC_ECO_OUT_TEMP_1_UNION;
#endif
#define PMIC_ECO_OUT_TEMP_1_eco_out_temp_s1_START (0)
#define PMIC_ECO_OUT_TEMP_1_eco_out_temp_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_out_temp_s2 : 8;
    } reg;
} PMIC_ECO_OUT_TEMP_2_UNION;
#endif
#define PMIC_ECO_OUT_TEMP_2_eco_out_temp_s2_START (0)
#define PMIC_ECO_OUT_TEMP_2_eco_out_temp_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char temp_rdata_s0 : 8;
    } reg;
} PMIC_TEMP0_RDATA_UNION;
#endif
#define PMIC_TEMP0_RDATA_temp_rdata_s0_START (0)
#define PMIC_TEMP0_RDATA_temp_rdata_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char temp_rdata_s1 : 8;
    } reg;
} PMIC_TEMP1_RDATA_UNION;
#endif
#define PMIC_TEMP1_RDATA_temp_rdata_s1_START (0)
#define PMIC_TEMP1_RDATA_temp_rdata_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char temp_rdata_s2 : 8;
    } reg;
} PMIC_TEMP2_RDATA_UNION;
#endif
#define PMIC_TEMP2_RDATA_temp_rdata_s2_START (0)
#define PMIC_TEMP2_RDATA_temp_rdata_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre0_s0 : 8;
    } reg;
} PMIC_V_PRE0_OUT0_UNION;
#endif
#define PMIC_V_PRE0_OUT0_v_out_pre0_s0_START (0)
#define PMIC_V_PRE0_OUT0_v_out_pre0_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre0_s1 : 8;
    } reg;
} PMIC_V_PRE0_OUT1_UNION;
#endif
#define PMIC_V_PRE0_OUT1_v_out_pre0_s1_START (0)
#define PMIC_V_PRE0_OUT1_v_out_pre0_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre0_s2 : 8;
    } reg;
} PMIC_V_PRE0_OUT2_UNION;
#endif
#define PMIC_V_PRE0_OUT2_v_out_pre0_s2_START (0)
#define PMIC_V_PRE0_OUT2_v_out_pre0_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre1_s0 : 8;
    } reg;
} PMIC_V_PRE1_OUT0_UNION;
#endif
#define PMIC_V_PRE1_OUT0_v_out_pre1_s0_START (0)
#define PMIC_V_PRE1_OUT0_v_out_pre1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre1_s1 : 8;
    } reg;
} PMIC_V_PRE1_OUT1_UNION;
#endif
#define PMIC_V_PRE1_OUT1_v_out_pre1_s1_START (0)
#define PMIC_V_PRE1_OUT1_v_out_pre1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre1_s2 : 8;
    } reg;
} PMIC_V_PRE1_OUT2_UNION;
#endif
#define PMIC_V_PRE1_OUT2_v_out_pre1_s2_START (0)
#define PMIC_V_PRE1_OUT2_v_out_pre1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre2_s0 : 8;
    } reg;
} PMIC_V_PRE2_OUT0_UNION;
#endif
#define PMIC_V_PRE2_OUT0_v_out_pre2_s0_START (0)
#define PMIC_V_PRE2_OUT0_v_out_pre2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre2_s1 : 8;
    } reg;
} PMIC_V_PRE2_OUT1_UNION;
#endif
#define PMIC_V_PRE2_OUT1_v_out_pre2_s1_START (0)
#define PMIC_V_PRE2_OUT1_v_out_pre2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre2_s2 : 8;
    } reg;
} PMIC_V_PRE2_OUT2_UNION;
#endif
#define PMIC_V_PRE2_OUT2_v_out_pre2_s2_START (0)
#define PMIC_V_PRE2_OUT2_v_out_pre2_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre3_s0 : 8;
    } reg;
} PMIC_V_PRE3_OUT0_UNION;
#endif
#define PMIC_V_PRE3_OUT0_v_out_pre3_s0_START (0)
#define PMIC_V_PRE3_OUT0_v_out_pre3_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre3_s1 : 8;
    } reg;
} PMIC_V_PRE3_OUT1_UNION;
#endif
#define PMIC_V_PRE3_OUT1_v_out_pre3_s1_START (0)
#define PMIC_V_PRE3_OUT1_v_out_pre3_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre3_s2 : 8;
    } reg;
} PMIC_V_PRE3_OUT2_UNION;
#endif
#define PMIC_V_PRE3_OUT2_v_out_pre3_s2_START (0)
#define PMIC_V_PRE3_OUT2_v_out_pre3_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre4_s0 : 8;
    } reg;
} PMIC_V_PRE4_OUT0_UNION;
#endif
#define PMIC_V_PRE4_OUT0_v_out_pre4_s0_START (0)
#define PMIC_V_PRE4_OUT0_v_out_pre4_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre4_s1 : 8;
    } reg;
} PMIC_V_PRE4_OUT1_UNION;
#endif
#define PMIC_V_PRE4_OUT1_v_out_pre4_s1_START (0)
#define PMIC_V_PRE4_OUT1_v_out_pre4_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre4_s2 : 8;
    } reg;
} PMIC_V_PRE4_OUT2_UNION;
#endif
#define PMIC_V_PRE4_OUT2_v_out_pre4_s2_START (0)
#define PMIC_V_PRE4_OUT2_v_out_pre4_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre5_s0 : 8;
    } reg;
} PMIC_V_PRE5_OUT0_UNION;
#endif
#define PMIC_V_PRE5_OUT0_v_out_pre5_s0_START (0)
#define PMIC_V_PRE5_OUT0_v_out_pre5_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre5_s1 : 8;
    } reg;
} PMIC_V_PRE5_OUT1_UNION;
#endif
#define PMIC_V_PRE5_OUT1_v_out_pre5_s1_START (0)
#define PMIC_V_PRE5_OUT1_v_out_pre5_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre5_s2 : 8;
    } reg;
} PMIC_V_PRE5_OUT2_UNION;
#endif
#define PMIC_V_PRE5_OUT2_v_out_pre5_s2_START (0)
#define PMIC_V_PRE5_OUT2_v_out_pre5_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre6_s0 : 8;
    } reg;
} PMIC_V_PRE6_OUT0_UNION;
#endif
#define PMIC_V_PRE6_OUT0_v_out_pre6_s0_START (0)
#define PMIC_V_PRE6_OUT0_v_out_pre6_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre6_s1 : 8;
    } reg;
} PMIC_V_PRE6_OUT1_UNION;
#endif
#define PMIC_V_PRE6_OUT1_v_out_pre6_s1_START (0)
#define PMIC_V_PRE6_OUT1_v_out_pre6_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre6_s2 : 8;
    } reg;
} PMIC_V_PRE6_OUT2_UNION;
#endif
#define PMIC_V_PRE6_OUT2_v_out_pre6_s2_START (0)
#define PMIC_V_PRE6_OUT2_v_out_pre6_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre7_s0 : 8;
    } reg;
} PMIC_V_PRE7_OUT0_UNION;
#endif
#define PMIC_V_PRE7_OUT0_v_out_pre7_s0_START (0)
#define PMIC_V_PRE7_OUT0_v_out_pre7_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre7_s1 : 8;
    } reg;
} PMIC_V_PRE7_OUT1_UNION;
#endif
#define PMIC_V_PRE7_OUT1_v_out_pre7_s1_START (0)
#define PMIC_V_PRE7_OUT1_v_out_pre7_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre7_s2 : 8;
    } reg;
} PMIC_V_PRE7_OUT2_UNION;
#endif
#define PMIC_V_PRE7_OUT2_v_out_pre7_s2_START (0)
#define PMIC_V_PRE7_OUT2_v_out_pre7_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre0_s0 : 8;
    } reg;
} PMIC_CURRENT_PRE0_OUT0_UNION;
#endif
#define PMIC_CURRENT_PRE0_OUT0_current_pre0_s0_START (0)
#define PMIC_CURRENT_PRE0_OUT0_current_pre0_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre0_s1 : 8;
    } reg;
} PMIC_CURRENT_PRE0_OUT1_UNION;
#endif
#define PMIC_CURRENT_PRE0_OUT1_current_pre0_s1_START (0)
#define PMIC_CURRENT_PRE0_OUT1_current_pre0_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre0_s2 : 8;
    } reg;
} PMIC_CURRENT_PRE0_OUT2_UNION;
#endif
#define PMIC_CURRENT_PRE0_OUT2_current_pre0_s2_START (0)
#define PMIC_CURRENT_PRE0_OUT2_current_pre0_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre1_s0 : 8;
    } reg;
} PMIC_CURRENT_PRE1_OUT0_UNION;
#endif
#define PMIC_CURRENT_PRE1_OUT0_current_pre1_s0_START (0)
#define PMIC_CURRENT_PRE1_OUT0_current_pre1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre1_s1 : 8;
    } reg;
} PMIC_CURRENT_PRE1_OUT1_UNION;
#endif
#define PMIC_CURRENT_PRE1_OUT1_current_pre1_s1_START (0)
#define PMIC_CURRENT_PRE1_OUT1_current_pre1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre1_s2 : 8;
    } reg;
} PMIC_CURRENT_PRE1_OUT2_UNION;
#endif
#define PMIC_CURRENT_PRE1_OUT2_current_pre1_s2_START (0)
#define PMIC_CURRENT_PRE1_OUT2_current_pre1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre2_s0 : 8;
    } reg;
} PMIC_CURRENT_PRE2_OUT0_UNION;
#endif
#define PMIC_CURRENT_PRE2_OUT0_current_pre2_s0_START (0)
#define PMIC_CURRENT_PRE2_OUT0_current_pre2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre2_s1 : 8;
    } reg;
} PMIC_CURRENT_PRE2_OUT1_UNION;
#endif
#define PMIC_CURRENT_PRE2_OUT1_current_pre2_s1_START (0)
#define PMIC_CURRENT_PRE2_OUT1_current_pre2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre2_s2 : 8;
    } reg;
} PMIC_CURRENT_PRE2_OUT2_UNION;
#endif
#define PMIC_CURRENT_PRE2_OUT2_current_pre2_s2_START (0)
#define PMIC_CURRENT_PRE2_OUT2_current_pre2_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre3_s0 : 8;
    } reg;
} PMIC_CURRENT_PRE3_OUT0_UNION;
#endif
#define PMIC_CURRENT_PRE3_OUT0_current_pre3_s0_START (0)
#define PMIC_CURRENT_PRE3_OUT0_current_pre3_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre3_s1 : 8;
    } reg;
} PMIC_CURRENT_PRE3_OUT1_UNION;
#endif
#define PMIC_CURRENT_PRE3_OUT1_current_pre3_s1_START (0)
#define PMIC_CURRENT_PRE3_OUT1_current_pre3_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre3_s2 : 8;
    } reg;
} PMIC_CURRENT_PRE3_OUT2_UNION;
#endif
#define PMIC_CURRENT_PRE3_OUT2_current_pre3_s2_START (0)
#define PMIC_CURRENT_PRE3_OUT2_current_pre3_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre4_s0 : 8;
    } reg;
} PMIC_CURRENT_PRE4_OUT0_UNION;
#endif
#define PMIC_CURRENT_PRE4_OUT0_current_pre4_s0_START (0)
#define PMIC_CURRENT_PRE4_OUT0_current_pre4_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre4_s1 : 8;
    } reg;
} PMIC_CURRENT_PRE4_OUT1_UNION;
#endif
#define PMIC_CURRENT_PRE4_OUT1_current_pre4_s1_START (0)
#define PMIC_CURRENT_PRE4_OUT1_current_pre4_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre4_s2 : 8;
    } reg;
} PMIC_CURRENT_PRE4_OUT2_UNION;
#endif
#define PMIC_CURRENT_PRE4_OUT2_current_pre4_s2_START (0)
#define PMIC_CURRENT_PRE4_OUT2_current_pre4_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre5_s0 : 8;
    } reg;
} PMIC_CURRENT_PRE5_OUT0_UNION;
#endif
#define PMIC_CURRENT_PRE5_OUT0_current_pre5_s0_START (0)
#define PMIC_CURRENT_PRE5_OUT0_current_pre5_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre5_s1 : 8;
    } reg;
} PMIC_CURRENT_PRE5_OUT1_UNION;
#endif
#define PMIC_CURRENT_PRE5_OUT1_current_pre5_s1_START (0)
#define PMIC_CURRENT_PRE5_OUT1_current_pre5_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre5_s2 : 8;
    } reg;
} PMIC_CURRENT_PRE5_OUT2_UNION;
#endif
#define PMIC_CURRENT_PRE5_OUT2_current_pre5_s2_START (0)
#define PMIC_CURRENT_PRE5_OUT2_current_pre5_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre6_s0 : 8;
    } reg;
} PMIC_CURRENT_PRE6_OUT0_UNION;
#endif
#define PMIC_CURRENT_PRE6_OUT0_current_pre6_s0_START (0)
#define PMIC_CURRENT_PRE6_OUT0_current_pre6_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre6_s1 : 8;
    } reg;
} PMIC_CURRENT_PRE6_OUT1_UNION;
#endif
#define PMIC_CURRENT_PRE6_OUT1_current_pre6_s1_START (0)
#define PMIC_CURRENT_PRE6_OUT1_current_pre6_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre6_s2 : 8;
    } reg;
} PMIC_CURRENT_PRE6_OUT2_UNION;
#endif
#define PMIC_CURRENT_PRE6_OUT2_current_pre6_s2_START (0)
#define PMIC_CURRENT_PRE6_OUT2_current_pre6_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre7_s0 : 8;
    } reg;
} PMIC_CURRENT_PRE7_OUT0_UNION;
#endif
#define PMIC_CURRENT_PRE7_OUT0_current_pre7_s0_START (0)
#define PMIC_CURRENT_PRE7_OUT0_current_pre7_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre7_s1 : 8;
    } reg;
} PMIC_CURRENT_PRE7_OUT1_UNION;
#endif
#define PMIC_CURRENT_PRE7_OUT1_current_pre7_s1_START (0)
#define PMIC_CURRENT_PRE7_OUT1_current_pre7_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre7_s2 : 8;
    } reg;
} PMIC_CURRENT_PRE7_OUT2_UNION;
#endif
#define PMIC_CURRENT_PRE7_OUT2_current_pre7_s2_START (0)
#define PMIC_CURRENT_PRE7_OUT2_current_pre7_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre1_s0 : 8;
    } reg;
} PMIC_V_OCV_PRE1_OUT0_UNION;
#endif
#define PMIC_V_OCV_PRE1_OUT0_v_ocv_out_pre1_s0_START (0)
#define PMIC_V_OCV_PRE1_OUT0_v_ocv_out_pre1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre1_s1 : 8;
    } reg;
} PMIC_V_OCV_PRE1_OUT1_UNION;
#endif
#define PMIC_V_OCV_PRE1_OUT1_v_ocv_out_pre1_s1_START (0)
#define PMIC_V_OCV_PRE1_OUT1_v_ocv_out_pre1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre1_s2 : 8;
    } reg;
} PMIC_V_OCV_PRE1_OUT2_UNION;
#endif
#define PMIC_V_OCV_PRE1_OUT2_v_ocv_out_pre1_s2_START (0)
#define PMIC_V_OCV_PRE1_OUT2_v_ocv_out_pre1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre2_s0 : 8;
    } reg;
} PMIC_V_OCV_PRE2_OUT0_UNION;
#endif
#define PMIC_V_OCV_PRE2_OUT0_v_ocv_out_pre2_s0_START (0)
#define PMIC_V_OCV_PRE2_OUT0_v_ocv_out_pre2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre2_s1 : 8;
    } reg;
} PMIC_V_OCV_PRE2_OUT1_UNION;
#endif
#define PMIC_V_OCV_PRE2_OUT1_v_ocv_out_pre2_s1_START (0)
#define PMIC_V_OCV_PRE2_OUT1_v_ocv_out_pre2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre2_s2 : 8;
    } reg;
} PMIC_V_OCV_PRE2_OUT2_UNION;
#endif
#define PMIC_V_OCV_PRE2_OUT2_v_ocv_out_pre2_s2_START (0)
#define PMIC_V_OCV_PRE2_OUT2_v_ocv_out_pre2_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre3_s0 : 8;
    } reg;
} PMIC_V_OCV_PRE3_OUT0_UNION;
#endif
#define PMIC_V_OCV_PRE3_OUT0_v_ocv_out_pre3_s0_START (0)
#define PMIC_V_OCV_PRE3_OUT0_v_ocv_out_pre3_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre3_s1 : 8;
    } reg;
} PMIC_V_OCV_PRE3_OUT1_UNION;
#endif
#define PMIC_V_OCV_PRE3_OUT1_v_ocv_out_pre3_s1_START (0)
#define PMIC_V_OCV_PRE3_OUT1_v_ocv_out_pre3_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre3_s2 : 8;
    } reg;
} PMIC_V_OCV_PRE3_OUT2_UNION;
#endif
#define PMIC_V_OCV_PRE3_OUT2_v_ocv_out_pre3_s2_START (0)
#define PMIC_V_OCV_PRE3_OUT2_v_ocv_out_pre3_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre4_s0 : 8;
    } reg;
} PMIC_V_OCV_PRE4_OUT0_UNION;
#endif
#define PMIC_V_OCV_PRE4_OUT0_v_ocv_out_pre4_s0_START (0)
#define PMIC_V_OCV_PRE4_OUT0_v_ocv_out_pre4_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre4_s1 : 8;
    } reg;
} PMIC_V_OCV_PRE4_OUT1_UNION;
#endif
#define PMIC_V_OCV_PRE4_OUT1_v_ocv_out_pre4_s1_START (0)
#define PMIC_V_OCV_PRE4_OUT1_v_ocv_out_pre4_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre4_s2 : 8;
    } reg;
} PMIC_V_OCV_PRE4_OUT2_UNION;
#endif
#define PMIC_V_OCV_PRE4_OUT2_v_ocv_out_pre4_s2_START (0)
#define PMIC_V_OCV_PRE4_OUT2_v_ocv_out_pre4_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre1_s0 : 8;
    } reg;
} PMIC_I_OCV_PRE1_OUT0_UNION;
#endif
#define PMIC_I_OCV_PRE1_OUT0_i_ocv_out_pre1_s0_START (0)
#define PMIC_I_OCV_PRE1_OUT0_i_ocv_out_pre1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre1_s1 : 8;
    } reg;
} PMIC_I_OCV_PRE1_OUT1_UNION;
#endif
#define PMIC_I_OCV_PRE1_OUT1_i_ocv_out_pre1_s1_START (0)
#define PMIC_I_OCV_PRE1_OUT1_i_ocv_out_pre1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre1_s2 : 8;
    } reg;
} PMIC_I_OCV_PRE1_OUT2_UNION;
#endif
#define PMIC_I_OCV_PRE1_OUT2_i_ocv_out_pre1_s2_START (0)
#define PMIC_I_OCV_PRE1_OUT2_i_ocv_out_pre1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre2_s0 : 8;
    } reg;
} PMIC_I_OCV_PRE2_OUT0_UNION;
#endif
#define PMIC_I_OCV_PRE2_OUT0_i_ocv_out_pre2_s0_START (0)
#define PMIC_I_OCV_PRE2_OUT0_i_ocv_out_pre2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre2_s1 : 8;
    } reg;
} PMIC_I_OCV_PRE2_OUT1_UNION;
#endif
#define PMIC_I_OCV_PRE2_OUT1_i_ocv_out_pre2_s1_START (0)
#define PMIC_I_OCV_PRE2_OUT1_i_ocv_out_pre2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre2_s2 : 8;
    } reg;
} PMIC_I_OCV_PRE2_OUT2_UNION;
#endif
#define PMIC_I_OCV_PRE2_OUT2_i_ocv_out_pre2_s2_START (0)
#define PMIC_I_OCV_PRE2_OUT2_i_ocv_out_pre2_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre3_s0 : 8;
    } reg;
} PMIC_I_OCV_PRE3_OUT0_UNION;
#endif
#define PMIC_I_OCV_PRE3_OUT0_i_ocv_out_pre3_s0_START (0)
#define PMIC_I_OCV_PRE3_OUT0_i_ocv_out_pre3_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre3_s1 : 8;
    } reg;
} PMIC_I_OCV_PRE3_OUT1_UNION;
#endif
#define PMIC_I_OCV_PRE3_OUT1_i_ocv_out_pre3_s1_START (0)
#define PMIC_I_OCV_PRE3_OUT1_i_ocv_out_pre3_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre3_s2 : 8;
    } reg;
} PMIC_I_OCV_PRE3_OUT2_UNION;
#endif
#define PMIC_I_OCV_PRE3_OUT2_i_ocv_out_pre3_s2_START (0)
#define PMIC_I_OCV_PRE3_OUT2_i_ocv_out_pre3_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre4_s0 : 8;
    } reg;
} PMIC_I_OCV_PRE4_OUT0_UNION;
#endif
#define PMIC_I_OCV_PRE4_OUT0_i_ocv_out_pre4_s0_START (0)
#define PMIC_I_OCV_PRE4_OUT0_i_ocv_out_pre4_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre4_s1 : 8;
    } reg;
} PMIC_I_OCV_PRE4_OUT1_UNION;
#endif
#define PMIC_I_OCV_PRE4_OUT1_i_ocv_out_pre4_s1_START (0)
#define PMIC_I_OCV_PRE4_OUT1_i_ocv_out_pre4_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre4_s2 : 8;
    } reg;
} PMIC_I_OCV_PRE4_OUT2_UNION;
#endif
#define PMIC_I_OCV_PRE4_OUT2_i_ocv_out_pre4_s2_START (0)
#define PMIC_I_OCV_PRE4_OUT2_i_ocv_out_pre4_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_out_pre1_s0 : 8;
    } reg;
} PMIC_T_OCV_PRE1_OUT0_UNION;
#endif
#define PMIC_T_OCV_PRE1_OUT0_t_ocv_out_pre1_s0_START (0)
#define PMIC_T_OCV_PRE1_OUT0_t_ocv_out_pre1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_out_pre1_s1 : 8;
    } reg;
} PMIC_T_OCV_PRE1_OUT1_UNION;
#endif
#define PMIC_T_OCV_PRE1_OUT1_t_ocv_out_pre1_s1_START (0)
#define PMIC_T_OCV_PRE1_OUT1_t_ocv_out_pre1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_out_pre2_s0 : 8;
    } reg;
} PMIC_T_OCV_PRE2_OUT0_UNION;
#endif
#define PMIC_T_OCV_PRE2_OUT0_t_ocv_out_pre2_s0_START (0)
#define PMIC_T_OCV_PRE2_OUT0_t_ocv_out_pre2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_out_pre2_s1 : 8;
    } reg;
} PMIC_T_OCV_PRE2_OUT1_UNION;
#endif
#define PMIC_T_OCV_PRE2_OUT1_t_ocv_out_pre2_s1_START (0)
#define PMIC_T_OCV_PRE2_OUT1_t_ocv_out_pre2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_out_pre3_s0 : 8;
    } reg;
} PMIC_T_OCV_PRE3_OUT0_UNION;
#endif
#define PMIC_T_OCV_PRE3_OUT0_t_ocv_out_pre3_s0_START (0)
#define PMIC_T_OCV_PRE3_OUT0_t_ocv_out_pre3_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_out_pre3_s1 : 8;
    } reg;
} PMIC_T_OCV_PRE3_OUT1_UNION;
#endif
#define PMIC_T_OCV_PRE3_OUT1_t_ocv_out_pre3_s1_START (0)
#define PMIC_T_OCV_PRE3_OUT1_t_ocv_out_pre3_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_out_pre4_s0 : 8;
    } reg;
} PMIC_T_OCV_PRE4_OUT0_UNION;
#endif
#define PMIC_T_OCV_PRE4_OUT0_t_ocv_out_pre4_s0_START (0)
#define PMIC_T_OCV_PRE4_OUT0_t_ocv_out_pre4_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_out_pre4_s1 : 8;
    } reg;
} PMIC_T_OCV_PRE4_OUT1_UNION;
#endif
#define PMIC_T_OCV_PRE4_OUT1_t_ocv_out_pre4_s1_START (0)
#define PMIC_T_OCV_PRE4_OUT1_t_ocv_out_pre4_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre4 : 2;
        unsigned char rtc_ocv_out_pre3 : 2;
        unsigned char rtc_ocv_out_pre2 : 2;
        unsigned char rtc_ocv_out_pre1 : 2;
    } reg;
} PMIC_RTC_OCV_OUT_UNION;
#endif
#define PMIC_RTC_OCV_OUT_rtc_ocv_out_pre4_START (0)
#define PMIC_RTC_OCV_OUT_rtc_ocv_out_pre4_END (1)
#define PMIC_RTC_OCV_OUT_rtc_ocv_out_pre3_START (2)
#define PMIC_RTC_OCV_OUT_rtc_ocv_out_pre3_END (3)
#define PMIC_RTC_OCV_OUT_rtc_ocv_out_pre2_START (4)
#define PMIC_RTC_OCV_OUT_rtc_ocv_out_pre2_END (5)
#define PMIC_RTC_OCV_OUT_rtc_ocv_out_pre1_START (6)
#define PMIC_RTC_OCV_OUT_rtc_ocv_out_pre1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_i_data0_out0 : 8;
    } reg;
} PMIC_SLIDE_I_DATA0_OUT0_UNION;
#endif
#define PMIC_SLIDE_I_DATA0_OUT0_slide_i_data0_out0_START (0)
#define PMIC_SLIDE_I_DATA0_OUT0_slide_i_data0_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_i_data0_out1 : 8;
    } reg;
} PMIC_SLIDE_I_DATA0_OUT1_UNION;
#endif
#define PMIC_SLIDE_I_DATA0_OUT1_slide_i_data0_out1_START (0)
#define PMIC_SLIDE_I_DATA0_OUT1_slide_i_data0_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_i_data0_out2 : 8;
    } reg;
} PMIC_SLIDE_I_DATA0_OUT2_UNION;
#endif
#define PMIC_SLIDE_I_DATA0_OUT2_slide_i_data0_out2_START (0)
#define PMIC_SLIDE_I_DATA0_OUT2_slide_i_data0_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_i_data1_out0 : 8;
    } reg;
} PMIC_SLIDE_I_DATA1_OUT0_UNION;
#endif
#define PMIC_SLIDE_I_DATA1_OUT0_slide_i_data1_out0_START (0)
#define PMIC_SLIDE_I_DATA1_OUT0_slide_i_data1_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_i_data1_out1 : 8;
    } reg;
} PMIC_SLIDE_I_DATA1_OUT1_UNION;
#endif
#define PMIC_SLIDE_I_DATA1_OUT1_slide_i_data1_out1_START (0)
#define PMIC_SLIDE_I_DATA1_OUT1_slide_i_data1_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_i_data1_out2 : 8;
    } reg;
} PMIC_SLIDE_I_DATA1_OUT2_UNION;
#endif
#define PMIC_SLIDE_I_DATA1_OUT2_slide_i_data1_out2_START (0)
#define PMIC_SLIDE_I_DATA1_OUT2_slide_i_data1_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_i_data2_out0 : 8;
    } reg;
} PMIC_SLIDE_I_DATA2_OUT0_UNION;
#endif
#define PMIC_SLIDE_I_DATA2_OUT0_slide_i_data2_out0_START (0)
#define PMIC_SLIDE_I_DATA2_OUT0_slide_i_data2_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_i_data2_out1 : 8;
    } reg;
} PMIC_SLIDE_I_DATA2_OUT1_UNION;
#endif
#define PMIC_SLIDE_I_DATA2_OUT1_slide_i_data2_out1_START (0)
#define PMIC_SLIDE_I_DATA2_OUT1_slide_i_data2_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_i_data2_out2 : 8;
    } reg;
} PMIC_SLIDE_I_DATA2_OUT2_UNION;
#endif
#define PMIC_SLIDE_I_DATA2_OUT2_slide_i_data2_out2_START (0)
#define PMIC_SLIDE_I_DATA2_OUT2_slide_i_data2_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_i_data3_out0 : 8;
    } reg;
} PMIC_SLIDE_I_DATA3_OUT0_UNION;
#endif
#define PMIC_SLIDE_I_DATA3_OUT0_slide_i_data3_out0_START (0)
#define PMIC_SLIDE_I_DATA3_OUT0_slide_i_data3_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_i_data3_out1 : 8;
    } reg;
} PMIC_SLIDE_I_DATA3_OUT1_UNION;
#endif
#define PMIC_SLIDE_I_DATA3_OUT1_slide_i_data3_out1_START (0)
#define PMIC_SLIDE_I_DATA3_OUT1_slide_i_data3_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_i_data3_out2 : 8;
    } reg;
} PMIC_SLIDE_I_DATA3_OUT2_UNION;
#endif
#define PMIC_SLIDE_I_DATA3_OUT2_slide_i_data3_out2_START (0)
#define PMIC_SLIDE_I_DATA3_OUT2_slide_i_data3_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_v_data0_out0 : 8;
    } reg;
} PMIC_SLIDE_V_DATA0_OUT0_UNION;
#endif
#define PMIC_SLIDE_V_DATA0_OUT0_slide_v_data0_out0_START (0)
#define PMIC_SLIDE_V_DATA0_OUT0_slide_v_data0_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_v_data0_out1 : 8;
    } reg;
} PMIC_SLIDE_V_DATA0_OUT1_UNION;
#endif
#define PMIC_SLIDE_V_DATA0_OUT1_slide_v_data0_out1_START (0)
#define PMIC_SLIDE_V_DATA0_OUT1_slide_v_data0_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_v_data0_out2 : 8;
    } reg;
} PMIC_SLIDE_V_DATA0_OUT2_UNION;
#endif
#define PMIC_SLIDE_V_DATA0_OUT2_slide_v_data0_out2_START (0)
#define PMIC_SLIDE_V_DATA0_OUT2_slide_v_data0_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_v_data1_out0 : 8;
    } reg;
} PMIC_SLIDE_V_DATA1_OUT0_UNION;
#endif
#define PMIC_SLIDE_V_DATA1_OUT0_slide_v_data1_out0_START (0)
#define PMIC_SLIDE_V_DATA1_OUT0_slide_v_data1_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_v_data1_out1 : 8;
    } reg;
} PMIC_SLIDE_V_DATA1_OUT1_UNION;
#endif
#define PMIC_SLIDE_V_DATA1_OUT1_slide_v_data1_out1_START (0)
#define PMIC_SLIDE_V_DATA1_OUT1_slide_v_data1_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_v_data1_out2 : 8;
    } reg;
} PMIC_SLIDE_V_DATA1_OUT2_UNION;
#endif
#define PMIC_SLIDE_V_DATA1_OUT2_slide_v_data1_out2_START (0)
#define PMIC_SLIDE_V_DATA1_OUT2_slide_v_data1_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_v_data2_out0 : 8;
    } reg;
} PMIC_SLIDE_V_DATA2_OUT0_UNION;
#endif
#define PMIC_SLIDE_V_DATA2_OUT0_slide_v_data2_out0_START (0)
#define PMIC_SLIDE_V_DATA2_OUT0_slide_v_data2_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_v_data2_out1 : 8;
    } reg;
} PMIC_SLIDE_V_DATA2_OUT1_UNION;
#endif
#define PMIC_SLIDE_V_DATA2_OUT1_slide_v_data2_out1_START (0)
#define PMIC_SLIDE_V_DATA2_OUT1_slide_v_data2_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_v_data2_out2 : 8;
    } reg;
} PMIC_SLIDE_V_DATA2_OUT2_UNION;
#endif
#define PMIC_SLIDE_V_DATA2_OUT2_slide_v_data2_out2_START (0)
#define PMIC_SLIDE_V_DATA2_OUT2_slide_v_data2_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_v_data3_out0 : 8;
    } reg;
} PMIC_SLIDE_V_DATA3_OUT0_UNION;
#endif
#define PMIC_SLIDE_V_DATA3_OUT0_slide_v_data3_out0_START (0)
#define PMIC_SLIDE_V_DATA3_OUT0_slide_v_data3_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_v_data3_out1 : 8;
    } reg;
} PMIC_SLIDE_V_DATA3_OUT1_UNION;
#endif
#define PMIC_SLIDE_V_DATA3_OUT1_slide_v_data3_out1_START (0)
#define PMIC_SLIDE_V_DATA3_OUT1_slide_v_data3_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_v_data3_out2 : 8;
    } reg;
} PMIC_SLIDE_V_DATA3_OUT2_UNION;
#endif
#define PMIC_SLIDE_V_DATA3_OUT2_slide_v_data3_out2_START (0)
#define PMIC_SLIDE_V_DATA3_OUT2_slide_v_data3_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_cnt_data_out0 : 8;
    } reg;
} PMIC_SLIDE_CNT_DATA_OUT0_UNION;
#endif
#define PMIC_SLIDE_CNT_DATA_OUT0_slide_cnt_data_out0_START (0)
#define PMIC_SLIDE_CNT_DATA_OUT0_slide_cnt_data_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_cnt_data_out1 : 8;
    } reg;
} PMIC_SLIDE_CNT_DATA_OUT1_UNION;
#endif
#define PMIC_SLIDE_CNT_DATA_OUT1_slide_cnt_data_out1_START (0)
#define PMIC_SLIDE_CNT_DATA_OUT1_slide_cnt_data_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_cnt_data_out2 : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_SLIDE_CNT_DATA_OUT2_UNION;
#endif
#define PMIC_SLIDE_CNT_DATA_OUT2_slide_cnt_data_out2_START (0)
#define PMIC_SLIDE_CNT_DATA_OUT2_slide_cnt_data_out2_END (3)
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
        unsigned char coul_reserve1 : 8;
    } reg;
} PMIC_COUL_RESERVE1_UNION;
#endif
#define PMIC_COUL_RESERVE1_coul_reserve1_START (0)
#define PMIC_COUL_RESERVE1_coul_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char clj_debug0_0 : 1;
        unsigned char clj_debug0_1 : 1;
        unsigned char clj_debug0_2 : 1;
        unsigned char clj_debug0_3 : 1;
        unsigned char cali_en_i : 1;
        unsigned char cali_en_i_force : 1;
        unsigned char cali_en_v_force : 1;
        unsigned char cali_en_v : 1;
    } reg;
} PMIC_CLJ_DEBUG0_UNION;
#endif
#define PMIC_CLJ_DEBUG0_clj_debug0_0_START (0)
#define PMIC_CLJ_DEBUG0_clj_debug0_0_END (0)
#define PMIC_CLJ_DEBUG0_clj_debug0_1_START (1)
#define PMIC_CLJ_DEBUG0_clj_debug0_1_END (1)
#define PMIC_CLJ_DEBUG0_clj_debug0_2_START (2)
#define PMIC_CLJ_DEBUG0_clj_debug0_2_END (2)
#define PMIC_CLJ_DEBUG0_clj_debug0_3_START (3)
#define PMIC_CLJ_DEBUG0_clj_debug0_3_END (3)
#define PMIC_CLJ_DEBUG0_cali_en_i_START (4)
#define PMIC_CLJ_DEBUG0_cali_en_i_END (4)
#define PMIC_CLJ_DEBUG0_cali_en_i_force_START (5)
#define PMIC_CLJ_DEBUG0_cali_en_i_force_END (5)
#define PMIC_CLJ_DEBUG0_cali_en_v_force_START (6)
#define PMIC_CLJ_DEBUG0_cali_en_v_force_END (6)
#define PMIC_CLJ_DEBUG0_cali_en_v_START (7)
#define PMIC_CLJ_DEBUG0_cali_en_v_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_coul_always_off : 1;
        unsigned char voltage_coul_always_off : 1;
        unsigned char clj_debug_1_2 : 1;
        unsigned char coul_ctrl_onoff : 1;
        unsigned char coul_ocv_onoff : 1;
        unsigned char coul_debug_data_sel : 3;
    } reg;
} PMIC_CLJ_DEBUG1_UNION;
#endif
#define PMIC_CLJ_DEBUG1_current_coul_always_off_START (0)
#define PMIC_CLJ_DEBUG1_current_coul_always_off_END (0)
#define PMIC_CLJ_DEBUG1_voltage_coul_always_off_START (1)
#define PMIC_CLJ_DEBUG1_voltage_coul_always_off_END (1)
#define PMIC_CLJ_DEBUG1_clj_debug_1_2_START (2)
#define PMIC_CLJ_DEBUG1_clj_debug_1_2_END (2)
#define PMIC_CLJ_DEBUG1_coul_ctrl_onoff_START (3)
#define PMIC_CLJ_DEBUG1_coul_ctrl_onoff_END (3)
#define PMIC_CLJ_DEBUG1_coul_ocv_onoff_START (4)
#define PMIC_CLJ_DEBUG1_coul_ocv_onoff_END (4)
#define PMIC_CLJ_DEBUG1_coul_debug_data_sel_START (5)
#define PMIC_CLJ_DEBUG1_coul_debug_data_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char debug_cic_i_s0 : 8;
    } reg;
} PMIC_DEBUG_CIC_I0_UNION;
#endif
#define PMIC_DEBUG_CIC_I0_debug_cic_i_s0_START (0)
#define PMIC_DEBUG_CIC_I0_debug_cic_i_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char debug_cic_i_s1 : 8;
    } reg;
} PMIC_DEBUG_CIC_I1_UNION;
#endif
#define PMIC_DEBUG_CIC_I1_debug_cic_i_s1_START (0)
#define PMIC_DEBUG_CIC_I1_debug_cic_i_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char debug_cic_i_s2 : 8;
    } reg;
} PMIC_DEBUG_CIC_I2_UNION;
#endif
#define PMIC_DEBUG_CIC_I2_debug_cic_i_s2_START (0)
#define PMIC_DEBUG_CIC_I2_debug_cic_i_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char debug_cic_v_s0 : 8;
    } reg;
} PMIC_DEBUG_CIC_V0_UNION;
#endif
#define PMIC_DEBUG_CIC_V0_debug_cic_v_s0_START (0)
#define PMIC_DEBUG_CIC_V0_debug_cic_v_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char debug_cic_v_s1 : 8;
    } reg;
} PMIC_DEBUG_CIC_V1_UNION;
#endif
#define PMIC_DEBUG_CIC_V1_debug_cic_v_s1_START (0)
#define PMIC_DEBUG_CIC_V1_debug_cic_v_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char debug_cic_v_s2 : 8;
    } reg;
} PMIC_DEBUG_CIC_V2_UNION;
#endif
#define PMIC_DEBUG_CIC_V2_debug_cic_v_s2_START (0)
#define PMIC_DEBUG_CIC_V2_debug_cic_v_s2_END (7)
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
        unsigned char ovp_idis_sel : 4;
        unsigned char acr_cap_sel : 2;
        unsigned char acr_mul_sel : 2;
    } reg;
} PMIC_ACR_CFG0_UNION;
#endif
#define PMIC_ACR_CFG0_ovp_idis_sel_START (0)
#define PMIC_ACR_CFG0_ovp_idis_sel_END (3)
#define PMIC_ACR_CFG0_acr_cap_sel_START (4)
#define PMIC_ACR_CFG0_acr_cap_sel_END (5)
#define PMIC_ACR_CFG0_acr_mul_sel_START (6)
#define PMIC_ACR_CFG0_acr_mul_sel_END (7)
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
} PMIC_ACR_CFG1_UNION;
#endif
#define PMIC_ACR_CFG1_acr_testmode_START (0)
#define PMIC_ACR_CFG1_acr_testmode_END (0)
#define PMIC_ACR_CFG1_acr_iref_sel_START (1)
#define PMIC_ACR_CFG1_acr_iref_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_reserve : 8;
    } reg;
} PMIC_ACR_RESERVE_CFG_UNION;
#endif
#define PMIC_ACR_RESERVE_CFG_acr_reserve_START (0)
#define PMIC_ACR_RESERVE_CFG_acr_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_chanl_sel : 5;
        unsigned char reserved : 2;
        unsigned char hkadc2_bypass : 1;
    } reg;
} PMIC_ACRADC_CTRL_UNION;
#endif
#define PMIC_ACRADC_CTRL_hkadc2_chanl_sel_START (0)
#define PMIC_ACRADC_CTRL_hkadc2_chanl_sel_END (4)
#define PMIC_ACRADC_CTRL_hkadc2_bypass_START (7)
#define PMIC_ACRADC_CTRL_hkadc2_bypass_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_start_soft : 1;
        unsigned char hkadc2_reserve : 7;
    } reg;
} PMIC_ACRADC_START_UNION;
#endif
#define PMIC_ACRADC_START_hkadc2_start_soft_START (0)
#define PMIC_ACRADC_START_hkadc2_start_soft_END (0)
#define PMIC_ACRADC_START_hkadc2_reserve_START (1)
#define PMIC_ACRADC_START_hkadc2_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_valid : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_ACRCONV_STATUS_UNION;
#endif
#define PMIC_ACRCONV_STATUS_hkadc2_valid_START (0)
#define PMIC_ACRCONV_STATUS_hkadc2_valid_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_data_l : 8;
    } reg;
} PMIC_ACRADC_DATA_L_UNION;
#endif
#define PMIC_ACRADC_DATA_L_hkadc2_data_l_START (0)
#define PMIC_ACRADC_DATA_L_hkadc2_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_data_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_ACRADC_DATA_H_UNION;
#endif
#define PMIC_ACRADC_DATA_H_hkadc2_data_h_START (0)
#define PMIC_ACRADC_DATA_H_hkadc2_data_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_tdie_data_l : 8;
    } reg;
} PMIC_ACRADC_TDIE_DATA_L_UNION;
#endif
#define PMIC_ACRADC_TDIE_DATA_L_hkadc2_tdie_data_l_START (0)
#define PMIC_ACRADC_TDIE_DATA_L_hkadc2_tdie_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_tdie_data_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_ACRADC_TDIE_DATA_H_UNION;
#endif
#define PMIC_ACRADC_TDIE_DATA_H_hkadc2_tdie_data_h_START (0)
#define PMIC_ACRADC_TDIE_DATA_H_hkadc2_tdie_data_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_tbat_data_l : 8;
    } reg;
} PMIC_ACRADC_TBAT_DATA_L_UNION;
#endif
#define PMIC_ACRADC_TBAT_DATA_L_hkadc2_tbat_data_l_START (0)
#define PMIC_ACRADC_TBAT_DATA_L_hkadc2_tbat_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_tbat_data_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_ACRADC_TBAT_DATA_H_UNION;
#endif
#define PMIC_ACRADC_TBAT_DATA_H_hkadc2_tbat_data_h_START (0)
#define PMIC_ACRADC_TBAT_DATA_H_hkadc2_tbat_data_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_buffer_sel : 1;
        unsigned char hkadc2_config : 7;
    } reg;
} PMIC_ACRADC_CONV_UNION;
#endif
#define PMIC_ACRADC_CONV_hkadc2_buffer_sel_START (0)
#define PMIC_ACRADC_CONV_hkadc2_buffer_sel_END (0)
#define PMIC_ACRADC_CONV_hkadc2_config_START (1)
#define PMIC_ACRADC_CONV_hkadc2_config_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_ibias_sel : 8;
    } reg;
} PMIC_ACRADC_CURRENT_UNION;
#endif
#define PMIC_ACRADC_CURRENT_hkadc2_ibias_sel_START (0)
#define PMIC_ACRADC_CURRENT_hkadc2_ibias_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_cali_en : 1;
        unsigned char hkadc2_cali_sel : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_ACRADC_CALI_CTRL_UNION;
#endif
#define PMIC_ACRADC_CALI_CTRL_hkadc2_cali_en_START (0)
#define PMIC_ACRADC_CALI_CTRL_hkadc2_cali_en_END (0)
#define PMIC_ACRADC_CALI_CTRL_hkadc2_cali_sel_START (1)
#define PMIC_ACRADC_CALI_CTRL_hkadc2_cali_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_cali_data : 8;
    } reg;
} PMIC_ACRADC_CALI_VALUE_UNION;
#endif
#define PMIC_ACRADC_CALI_VALUE_hkadc2_cali_data_START (0)
#define PMIC_ACRADC_CALI_VALUE_hkadc2_cali_data_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_cali_cfg : 8;
    } reg;
} PMIC_ACRADC_CALI_CFG_UNION;
#endif
#define PMIC_ACRADC_CALI_CFG_hkadc2_cali_cfg_START (0)
#define PMIC_ACRADC_CALI_CFG_hkadc2_cali_cfg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_tdie_cali_data : 8;
    } reg;
} PMIC_ACRADC_TDIE_CALI_DATA_UNION;
#endif
#define PMIC_ACRADC_TDIE_CALI_DATA_hkadc2_tdie_cali_data_START (0)
#define PMIC_ACRADC_TDIE_CALI_DATA_hkadc2_tdie_cali_data_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_tbat_cali_data : 8;
    } reg;
} PMIC_ACRADC_TBAT_CALI_DATA_UNION;
#endif
#define PMIC_ACRADC_TBAT_CALI_DATA_hkadc2_tbat_cali_data_START (0)
#define PMIC_ACRADC_TBAT_CALI_DATA_hkadc2_tbat_cali_data_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_tdie_cali_cfg : 8;
    } reg;
} PMIC_ACRADC_TDIE_CALI_CFG_UNION;
#endif
#define PMIC_ACRADC_TDIE_CALI_CFG_hkadc2_tdie_cali_cfg_START (0)
#define PMIC_ACRADC_TDIE_CALI_CFG_hkadc2_tdie_cali_cfg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_tbat_cali_cfg : 8;
    } reg;
} PMIC_ACRADC_TBAT_CALI_CFG_UNION;
#endif
#define PMIC_ACRADC_TBAT_CALI_CFG_hkadc2_tbat_cali_cfg_START (0)
#define PMIC_ACRADC_TBAT_CALI_CFG_hkadc2_tbat_cali_cfg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_chopper_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_ACRADC_MODE_CFG_UNION;
#endif
#define PMIC_ACRADC_MODE_CFG_hkadc2_chopper_en_START (0)
#define PMIC_ACRADC_MODE_CFG_hkadc2_chopper_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_cali_offset_l : 8;
    } reg;
} PMIC_ACRADC_CALIVALUE_CFG1_UNION;
#endif
#define PMIC_ACRADC_CALIVALUE_CFG1_hkadc2_cali_offset_l_START (0)
#define PMIC_ACRADC_CALIVALUE_CFG1_hkadc2_cali_offset_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_cali_offset_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_ACRADC_CALIVALUE_CFG2_UNION;
#endif
#define PMIC_ACRADC_CALIVALUE_CFG2_hkadc2_cali_offset_h_START (0)
#define PMIC_ACRADC_CALIVALUE_CFG2_hkadc2_cali_offset_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_tdie_offset_l : 8;
    } reg;
} PMIC_ACRADC_TDIE_OFFSET_CFGL_UNION;
#endif
#define PMIC_ACRADC_TDIE_OFFSET_CFGL_hkadc2_tdie_offset_l_START (0)
#define PMIC_ACRADC_TDIE_OFFSET_CFGL_hkadc2_tdie_offset_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_tdie_offset_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_ACRADC_TDIE_OFFSET_CFGH_UNION;
#endif
#define PMIC_ACRADC_TDIE_OFFSET_CFGH_hkadc2_tdie_offset_h_START (0)
#define PMIC_ACRADC_TDIE_OFFSET_CFGH_hkadc2_tdie_offset_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_tbat_offset_l : 8;
    } reg;
} PMIC_ACRADC_TBAT_OFFSET_CFGL_UNION;
#endif
#define PMIC_ACRADC_TBAT_OFFSET_CFGL_hkadc2_tbat_offset_l_START (0)
#define PMIC_ACRADC_TBAT_OFFSET_CFGL_hkadc2_tbat_offset_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_tbat_offset_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_ACRADC_TBAT_OFFSET_CFGH_UNION;
#endif
#define PMIC_ACRADC_TBAT_OFFSET_CFGH_hkadc2_tbat_offset_h_START (0)
#define PMIC_ACRADC_TBAT_OFFSET_CFGH_hkadc2_tbat_offset_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_soh_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_SOH_EN_UNION;
#endif
#define PMIC_SOH_EN_sc_soh_en_START (0)
#define PMIC_SOH_EN_sc_soh_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_soh_det_timer : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_SOH_DET_TIMER_UNION;
#endif
#define PMIC_SOH_DET_TIMER_sc_soh_det_timer_START (0)
#define PMIC_SOH_DET_TIMER_sc_soh_det_timer_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_soh_tbat_ovh_l : 8;
    } reg;
} PMIC_SOH_TBAT_OVH_L_UNION;
#endif
#define PMIC_SOH_TBAT_OVH_L_sc_soh_tbat_ovh_l_START (0)
#define PMIC_SOH_TBAT_OVH_L_sc_soh_tbat_ovh_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_soh_tbat_ovh_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_SOH_TBAT_OVH_H_UNION;
#endif
#define PMIC_SOH_TBAT_OVH_H_sc_soh_tbat_ovh_h_START (0)
#define PMIC_SOH_TBAT_OVH_H_sc_soh_tbat_ovh_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_soh_vbat_uvp_l : 8;
    } reg;
} PMIC_SOH_VBAT_UVP_L_UNION;
#endif
#define PMIC_SOH_VBAT_UVP_L_sc_soh_vbat_uvp_l_START (0)
#define PMIC_SOH_VBAT_UVP_L_sc_soh_vbat_uvp_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_soh_vbat_uvp_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_SOH_VBAT_UVP_H_UNION;
#endif
#define PMIC_SOH_VBAT_UVP_H_sc_soh_vbat_uvp_h_START (0)
#define PMIC_SOH_VBAT_UVP_H_sc_soh_vbat_uvp_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soh_mode : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_SOH_MODE_UNION;
#endif
#define PMIC_SOH_MODE_soh_mode_START (0)
#define PMIC_SOH_MODE_soh_mode_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_acr_pulse_num : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_ACR_PULSE_NUM_UNION;
#endif
#define PMIC_ACR_PULSE_NUM_sc_acr_pulse_num_START (0)
#define PMIC_ACR_PULSE_NUM_sc_acr_pulse_num_END (1)
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
} PMIC_ACR_SAMPLE_TIME_H_UNION;
#endif
#define PMIC_ACR_SAMPLE_TIME_H_sc_acr_sample_t2_START (0)
#define PMIC_ACR_SAMPLE_TIME_H_sc_acr_sample_t2_END (2)
#define PMIC_ACR_SAMPLE_TIME_H_sc_acr_sample_t1_START (3)
#define PMIC_ACR_SAMPLE_TIME_H_sc_acr_sample_t1_END (5)
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
} PMIC_ACR_SAMPLE_TIME_L_UNION;
#endif
#define PMIC_ACR_SAMPLE_TIME_L_sc_acr_sample_t4_START (0)
#define PMIC_ACR_SAMPLE_TIME_L_sc_acr_sample_t4_END (2)
#define PMIC_ACR_SAMPLE_TIME_L_sc_acr_sample_t3_START (3)
#define PMIC_ACR_SAMPLE_TIME_L_sc_acr_sample_t3_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data0_l : 8;
    } reg;
} PMIC_ACR_DATA0_L_UNION;
#endif
#define PMIC_ACR_DATA0_L_acr_data0_l_START (0)
#define PMIC_ACR_DATA0_L_acr_data0_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data0_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_ACR_DATA0_H_UNION;
#endif
#define PMIC_ACR_DATA0_H_acr_data0_h_START (0)
#define PMIC_ACR_DATA0_H_acr_data0_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data1_l : 8;
    } reg;
} PMIC_ACR_DATA1_L_UNION;
#endif
#define PMIC_ACR_DATA1_L_acr_data1_l_START (0)
#define PMIC_ACR_DATA1_L_acr_data1_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data1_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_ACR_DATA1_H_UNION;
#endif
#define PMIC_ACR_DATA1_H_acr_data1_h_START (0)
#define PMIC_ACR_DATA1_H_acr_data1_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data2_l : 8;
    } reg;
} PMIC_ACR_DATA2_L_UNION;
#endif
#define PMIC_ACR_DATA2_L_acr_data2_l_START (0)
#define PMIC_ACR_DATA2_L_acr_data2_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data2_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_ACR_DATA2_H_UNION;
#endif
#define PMIC_ACR_DATA2_H_acr_data2_h_START (0)
#define PMIC_ACR_DATA2_H_acr_data2_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data3_l : 8;
    } reg;
} PMIC_ACR_DATA3_L_UNION;
#endif
#define PMIC_ACR_DATA3_L_acr_data3_l_START (0)
#define PMIC_ACR_DATA3_L_acr_data3_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data3_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_ACR_DATA3_H_UNION;
#endif
#define PMIC_ACR_DATA3_H_acr_data3_h_START (0)
#define PMIC_ACR_DATA3_H_acr_data3_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data4_l : 8;
    } reg;
} PMIC_ACR_DATA4_L_UNION;
#endif
#define PMIC_ACR_DATA4_L_acr_data4_l_START (0)
#define PMIC_ACR_DATA4_L_acr_data4_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data4_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_ACR_DATA4_H_UNION;
#endif
#define PMIC_ACR_DATA4_H_acr_data4_h_START (0)
#define PMIC_ACR_DATA4_H_acr_data4_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data5_l : 8;
    } reg;
} PMIC_ACR_DATA5_L_UNION;
#endif
#define PMIC_ACR_DATA5_L_acr_data5_l_START (0)
#define PMIC_ACR_DATA5_L_acr_data5_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data5_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_ACR_DATA5_H_UNION;
#endif
#define PMIC_ACR_DATA5_H_acr_data5_h_START (0)
#define PMIC_ACR_DATA5_H_acr_data5_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data6_l : 8;
    } reg;
} PMIC_ACR_DATA6_L_UNION;
#endif
#define PMIC_ACR_DATA6_L_acr_data6_l_START (0)
#define PMIC_ACR_DATA6_L_acr_data6_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data6_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_ACR_DATA6_H_UNION;
#endif
#define PMIC_ACR_DATA6_H_acr_data6_h_START (0)
#define PMIC_ACR_DATA6_H_acr_data6_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data7_l : 8;
    } reg;
} PMIC_ACR_DATA7_L_UNION;
#endif
#define PMIC_ACR_DATA7_L_acr_data7_l_START (0)
#define PMIC_ACR_DATA7_L_acr_data7_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data7_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_ACR_DATA7_H_UNION;
#endif
#define PMIC_ACR_DATA7_H_acr_data7_h_START (0)
#define PMIC_ACR_DATA7_H_acr_data7_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_ovp_timer : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_OVP_CTRL_UNION;
#endif
#define PMIC_OVP_CTRL_sc_ovp_timer_START (0)
#define PMIC_OVP_CTRL_sc_ovp_timer_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_ovp_vbat_th0_l : 8;
    } reg;
} PMIC_OVP_VBAT_OVH_TH0_L_UNION;
#endif
#define PMIC_OVP_VBAT_OVH_TH0_L_sc_ovp_vbat_th0_l_START (0)
#define PMIC_OVP_VBAT_OVH_TH0_L_sc_ovp_vbat_th0_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_ovp_vbat_th0_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_OVP_VBAT_OVH_TH0_H_UNION;
#endif
#define PMIC_OVP_VBAT_OVH_TH0_H_sc_ovp_vbat_th0_h_START (0)
#define PMIC_OVP_VBAT_OVH_TH0_H_sc_ovp_vbat_th0_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_ovp_tbat_th0_l : 8;
    } reg;
} PMIC_OVP_TBAT_OVH_TH0_L_UNION;
#endif
#define PMIC_OVP_TBAT_OVH_TH0_L_sc_ovp_tbat_th0_l_START (0)
#define PMIC_OVP_TBAT_OVH_TH0_L_sc_ovp_tbat_th0_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_ovp_tbat_th0_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_OVP_TBAT_OVH_TH0_H_UNION;
#endif
#define PMIC_OVP_TBAT_OVH_TH0_H_sc_ovp_tbat_th0_h_START (0)
#define PMIC_OVP_TBAT_OVH_TH0_H_sc_ovp_tbat_th0_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_ovp_vbat_th1_l : 8;
    } reg;
} PMIC_OVP_VBAT_OVH_TH1_L_UNION;
#endif
#define PMIC_OVP_VBAT_OVH_TH1_L_sc_ovp_vbat_th1_l_START (0)
#define PMIC_OVP_VBAT_OVH_TH1_L_sc_ovp_vbat_th1_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_ovp_vbat_th1_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_OVP_VBAT_OVH_TH1_H_UNION;
#endif
#define PMIC_OVP_VBAT_OVH_TH1_H_sc_ovp_vbat_th1_h_START (0)
#define PMIC_OVP_VBAT_OVH_TH1_H_sc_ovp_vbat_th1_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_ovp_tbat_th1_l : 8;
    } reg;
} PMIC_OVP_TBAT_OVH_TH1_L_UNION;
#endif
#define PMIC_OVP_TBAT_OVH_TH1_L_sc_ovp_tbat_th1_l_START (0)
#define PMIC_OVP_TBAT_OVH_TH1_L_sc_ovp_tbat_th1_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_ovp_tbat_th1_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_OVP_TBAT_OVH_TH1_H_UNION;
#endif
#define PMIC_OVP_TBAT_OVH_TH1_H_sc_ovp_tbat_th1_h_START (0)
#define PMIC_OVP_TBAT_OVH_TH1_H_sc_ovp_tbat_th1_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_ovp_vbat_th2_l : 8;
    } reg;
} PMIC_OVP_VBAT_OVH_TH2_L_UNION;
#endif
#define PMIC_OVP_VBAT_OVH_TH2_L_sc_ovp_vbat_th2_l_START (0)
#define PMIC_OVP_VBAT_OVH_TH2_L_sc_ovp_vbat_th2_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_ovp_vbat_th2_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_OVP_VBAT_OVH_TH2_H_UNION;
#endif
#define PMIC_OVP_VBAT_OVH_TH2_H_sc_ovp_vbat_th2_h_START (0)
#define PMIC_OVP_VBAT_OVH_TH2_H_sc_ovp_vbat_th2_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_ovp_tbat_th2_l : 8;
    } reg;
} PMIC_OVP_TBAT_OVH_TH2_L_UNION;
#endif
#define PMIC_OVP_TBAT_OVH_TH2_L_sc_ovp_tbat_th2_l_START (0)
#define PMIC_OVP_TBAT_OVH_TH2_L_sc_ovp_tbat_th2_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_ovp_tbat_th2_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_OVP_TBAT_OVH_TH2_H_UNION;
#endif
#define PMIC_OVP_TBAT_OVH_TH2_H_sc_ovp_tbat_th2_h_START (0)
#define PMIC_OVP_TBAT_OVH_TH2_H_sc_ovp_tbat_th2_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char st_soh_ovp : 1;
        unsigned char st_tmp_ovh_2 : 1;
        unsigned char st_soh_ovh_2 : 1;
        unsigned char st_tmp_ovh_1 : 1;
        unsigned char st_soh_ovh_1 : 1;
        unsigned char st_tmp_ovh_0 : 1;
        unsigned char st_soh_ovh_0 : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_SOH_OVP_REAL_UNION;
#endif
#define PMIC_SOH_OVP_REAL_st_soh_ovp_START (0)
#define PMIC_SOH_OVP_REAL_st_soh_ovp_END (0)
#define PMIC_SOH_OVP_REAL_st_tmp_ovh_2_START (1)
#define PMIC_SOH_OVP_REAL_st_tmp_ovh_2_END (1)
#define PMIC_SOH_OVP_REAL_st_soh_ovh_2_START (2)
#define PMIC_SOH_OVP_REAL_st_soh_ovh_2_END (2)
#define PMIC_SOH_OVP_REAL_st_tmp_ovh_1_START (3)
#define PMIC_SOH_OVP_REAL_st_tmp_ovh_1_END (3)
#define PMIC_SOH_OVP_REAL_st_soh_ovh_1_START (4)
#define PMIC_SOH_OVP_REAL_st_soh_ovh_1_END (4)
#define PMIC_SOH_OVP_REAL_st_tmp_ovh_0_START (5)
#define PMIC_SOH_OVP_REAL_st_tmp_ovh_0_END (5)
#define PMIC_SOH_OVP_REAL_st_soh_ovh_0_START (6)
#define PMIC_SOH_OVP_REAL_st_soh_ovh_0_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char discharge_sel : 1;
        unsigned char eco_sel : 1;
        unsigned char twp_sel : 1;
        unsigned char cic_sel : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_DCR_CONFIG_UNION;
#endif
#define PMIC_DCR_CONFIG_discharge_sel_START (0)
#define PMIC_DCR_CONFIG_discharge_sel_END (0)
#define PMIC_DCR_CONFIG_eco_sel_START (1)
#define PMIC_DCR_CONFIG_eco_sel_END (1)
#define PMIC_DCR_CONFIG_twp_sel_START (2)
#define PMIC_DCR_CONFIG_twp_sel_END (2)
#define PMIC_DCR_CONFIG_cic_sel_START (3)
#define PMIC_DCR_CONFIG_cic_sel_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char timer_set2 : 4;
        unsigned char timer_set1 : 4;
    } reg;
} PMIC_DCR_TIMER_CONFIG0_UNION;
#endif
#define PMIC_DCR_TIMER_CONFIG0_timer_set2_START (0)
#define PMIC_DCR_TIMER_CONFIG0_timer_set2_END (3)
#define PMIC_DCR_TIMER_CONFIG0_timer_set1_START (4)
#define PMIC_DCR_TIMER_CONFIG0_timer_set1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char timer_set3 : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_DCR_TIMER_CONFIG1_UNION;
#endif
#define PMIC_DCR_TIMER_CONFIG1_timer_set3_START (0)
#define PMIC_DCR_TIMER_CONFIG1_timer_set3_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char count : 8;
    } reg;
} PMIC_DCR_COUNT_CONFIG_UNION;
#endif
#define PMIC_DCR_COUNT_CONFIG_count_START (0)
#define PMIC_DCR_COUNT_CONFIG_count_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ibat1_s0 : 8;
    } reg;
} PMIC_IBAT1_0_UNION;
#endif
#define PMIC_IBAT1_0_ibat1_s0_START (0)
#define PMIC_IBAT1_0_ibat1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ibat1_s1 : 8;
    } reg;
} PMIC_IBAT1_1_UNION;
#endif
#define PMIC_IBAT1_1_ibat1_s1_START (0)
#define PMIC_IBAT1_1_ibat1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ibat1_s2 : 8;
    } reg;
} PMIC_IBAT1_2_UNION;
#endif
#define PMIC_IBAT1_2_ibat1_s2_START (0)
#define PMIC_IBAT1_2_ibat1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vbat1_s0 : 8;
    } reg;
} PMIC_VBAT1_0_UNION;
#endif
#define PMIC_VBAT1_0_vbat1_s0_START (0)
#define PMIC_VBAT1_0_vbat1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vbat1_s1 : 8;
    } reg;
} PMIC_VBAT1_1_UNION;
#endif
#define PMIC_VBAT1_1_vbat1_s1_START (0)
#define PMIC_VBAT1_1_vbat1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vbat1_s2 : 8;
    } reg;
} PMIC_VBAT1_2_UNION;
#endif
#define PMIC_VBAT1_2_vbat1_s2_START (0)
#define PMIC_VBAT1_2_vbat1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tmr1_s0 : 8;
    } reg;
} PMIC_TMR1_0_UNION;
#endif
#define PMIC_TMR1_0_tmr1_s0_START (0)
#define PMIC_TMR1_0_tmr1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tmr1_s1 : 8;
    } reg;
} PMIC_TMR1_1_UNION;
#endif
#define PMIC_TMR1_1_tmr1_s1_START (0)
#define PMIC_TMR1_1_tmr1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ibat2_s0 : 8;
    } reg;
} PMIC_IBAT2_0_UNION;
#endif
#define PMIC_IBAT2_0_ibat2_s0_START (0)
#define PMIC_IBAT2_0_ibat2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ibat2_s1 : 8;
    } reg;
} PMIC_IBAT2_1_UNION;
#endif
#define PMIC_IBAT2_1_ibat2_s1_START (0)
#define PMIC_IBAT2_1_ibat2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ibat2_s2 : 8;
    } reg;
} PMIC_IBAT2_2_UNION;
#endif
#define PMIC_IBAT2_2_ibat2_s2_START (0)
#define PMIC_IBAT2_2_ibat2_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vbat2_s0 : 8;
    } reg;
} PMIC_VBAT2_0_UNION;
#endif
#define PMIC_VBAT2_0_vbat2_s0_START (0)
#define PMIC_VBAT2_0_vbat2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vbat2_s1 : 8;
    } reg;
} PMIC_VBAT2_1_UNION;
#endif
#define PMIC_VBAT2_1_vbat2_s1_START (0)
#define PMIC_VBAT2_1_vbat2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vbat2_s2 : 8;
    } reg;
} PMIC_VBAT2_2_UNION;
#endif
#define PMIC_VBAT2_2_vbat2_s2_START (0)
#define PMIC_VBAT2_2_vbat2_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tmr2_s0 : 8;
    } reg;
} PMIC_TMR2_0_UNION;
#endif
#define PMIC_TMR2_0_tmr2_s0_START (0)
#define PMIC_TMR2_0_tmr2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tmr2_s1 : 8;
    } reg;
} PMIC_TMR2_1_UNION;
#endif
#define PMIC_TMR2_1_tmr2_s1_START (0)
#define PMIC_TMR2_1_tmr2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_soh_soft_rst : 8;
    } reg;
} PMIC_SOH_SOFT_RST_UNION;
#endif
#define PMIC_SOH_SOFT_RST_sc_soh_soft_rst_START (0)
#define PMIC_SOH_SOFT_RST_sc_soh_soft_rst_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_acr_tb_sel : 4;
        unsigned char sc_acr_tb_en : 1;
        unsigned char cic_sample_cfg : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_TEST_BUS_SEL_UNION;
#endif
#define PMIC_TEST_BUS_SEL_sc_acr_tb_sel_START (0)
#define PMIC_TEST_BUS_SEL_sc_acr_tb_sel_END (3)
#define PMIC_TEST_BUS_SEL_sc_acr_tb_en_START (4)
#define PMIC_TEST_BUS_SEL_sc_acr_tb_en_END (4)
#define PMIC_TEST_BUS_SEL_cic_sample_cfg_START (5)
#define PMIC_TEST_BUS_SEL_cic_sample_cfg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_tb_bus_0 : 8;
    } reg;
} PMIC_ACR_TB_BUS_0_UNION;
#endif
#define PMIC_ACR_TB_BUS_0_acr_tb_bus_0_START (0)
#define PMIC_ACR_TB_BUS_0_acr_tb_bus_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_tb_bus_1 : 8;
    } reg;
} PMIC_ACR_TB_BUS_1_UNION;
#endif
#define PMIC_ACR_TB_BUS_1_acr_tb_bus_1_START (0)
#define PMIC_ACR_TB_BUS_1_acr_tb_bus_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_debug_acr : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_ACR_CLK_GT_EN_UNION;
#endif
#define PMIC_ACR_CLK_GT_EN_sc_debug_acr_START (0)
#define PMIC_ACR_CLK_GT_EN_sc_debug_acr_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned short value;
    struct
    {
        unsigned short din0 : 10;
    } reg;
} PMIC_DAC0_OUT_UNION;
#endif
#define PMIC_DAC0_OUT_din0_START (0)
#define PMIC_DAC0_OUT_din0_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned short value;
    struct
    {
        unsigned short dac0_lsb : 1;
        unsigned short dac0_reserved : 9;
    } reg;
} PMIC_DAC0_LSB_UNION;
#endif
#define PMIC_DAC0_LSB_dac0_lsb_START (0)
#define PMIC_DAC0_LSB_dac0_lsb_END (0)
#define PMIC_DAC0_LSB_dac0_reserved_START (1)
#define PMIC_DAC0_LSB_dac0_reserved_END (9)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
