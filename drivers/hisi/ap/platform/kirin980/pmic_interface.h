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
#define PMIC_STATUS3_ADDR(base) ((base) + (0x0009UL))
#define PMIC_BUCK00_ONOFF_ECO_ADDR(base) ((base) + (0x000AUL))
#define PMIC_BUCK01_ONOFF_ADDR(base) ((base) + (0x000BUL))
#define PMIC_BUCK02_ONOFF_ECO_ADDR(base) ((base) + (0x000CUL))
#define PMIC_BUCK1_ONOFF_ECO_ADDR(base) ((base) + (0x000DUL))
#define PMIC_BUCK2_ONOFF_ECO_ADDR(base) ((base) + (0x000EUL))
#define PMIC_BUCK3_ONOFF_ECO_ADDR(base) ((base) + (0x000FUL))
#define PMIC_BUCK4_ONOFF_ECO_ADDR(base) ((base) + (0x0010UL))
#define PMIC_BUCK5_ONOFF_ECO_ADDR(base) ((base) + (0x0011UL))
#define PMIC_BUCK6_ONOFF_ECO_ADDR(base) ((base) + (0x0012UL))
#define PMIC_BUCK7_ONOFF_ECO_ADDR(base) ((base) + (0x0013UL))
#define PMIC_BUCK8_ONOFF_ECO_ADDR(base) ((base) + (0x0014UL))
#define PMIC_BUCK9_ONOFF_ECO_ADDR(base) ((base) + (0x0015UL))
#define PMIC_LDO0_ONOFF_ECO_ADDR(base) ((base) + (0x0016UL))
#define PMIC_LDO1_ONOFF_ADDR(base) ((base) + (0x0017UL))
#define PMIC_LDO2_ONOFF1_ADDR(base) ((base) + (0x0018UL))
#define PMIC_LDO2_ONOFF2_ADDR(base) ((base) + (0x0019UL))
#define PMIC_LDO3_ONOFF_ADDR(base) ((base) + (0x001AUL))
#define PMIC_LDO4_ONOFF_ECO_ADDR(base) ((base) + (0x001BUL))
#define PMIC_LDO5_ONOFF_ECO_ADDR(base) ((base) + (0x001CUL))
#define PMIC_LDO6_ONOFF_ECO_ADDR(base) ((base) + (0x001DUL))
#define PMIC_LDO7_ONOFF_ECO_ADDR(base) ((base) + (0x001EUL))
#define PMIC_LDO8_ONOFF_ECO_ADDR(base) ((base) + (0x001FUL))
#define PMIC_LDO9_ONOFF_ECO_ADDR(base) ((base) + (0x0020UL))
#define PMIC_LDO11_ONOFF_ECO_ADDR(base) ((base) + (0x0021UL))
#define PMIC_LDO12_ONOFF_ECO_ADDR(base) ((base) + (0x0022UL))
#define PMIC_LDO14_ONOFF_ADDR(base) ((base) + (0x0023UL))
#define PMIC_LDO15_ONOFF_ECO_ADDR(base) ((base) + (0x0024UL))
#define PMIC_LDO16_ONOFF_ECO_ADDR(base) ((base) + (0x0025UL))
#define PMIC_LDO17_ONOFF_ECO_ADDR(base) ((base) + (0x0026UL))
#define PMIC_LDO18_ONOFF_ECO_ADDR(base) ((base) + (0x0027UL))
#define PMIC_LDO21_ONOFF_ADDR(base) ((base) + (0x0028UL))
#define PMIC_LDO22_ONOFF_ADDR(base) ((base) + (0x0029UL))
#define PMIC_LDO23_ONOFF_ECO_ADDR(base) ((base) + (0x002AUL))
#define PMIC_LDO24_ONOFF_ECO_ADDR(base) ((base) + (0x002BUL))
#define PMIC_LDO25_ONOFF_ADDR(base) ((base) + (0x002CUL))
#define PMIC_LDO26_ONOFF_ECO_ADDR(base) ((base) + (0x002DUL))
#define PMIC_LDO27_ONOFF_ECO_ADDR(base) ((base) + (0x002EUL))
#define PMIC_LDO28_ONOFF_ADDR(base) ((base) + (0x002FUL))
#define PMIC_LDO29_ONOFF_ECO_ADDR(base) ((base) + (0x0030UL))
#define PMIC_LDO30_ONOFF_ECO_ADDR(base) ((base) + (0x0031UL))
#define PMIC_LDO32_ONOFF_ADDR(base) ((base) + (0x0032UL))
#define PMIC_LDO34_ONOFF_ECO_ADDR(base) ((base) + (0x0033UL))
#define PMIC_PMUH_ONOFF_ECO_ADDR(base) ((base) + (0x0034UL))
#define PMIC_LDO36_ONOFF_ECO_ADDR(base) ((base) + (0x0035UL))
#define PMIC_LDO37_ONOFF_ECO_ADDR(base) ((base) + (0x0036UL))
#define PMIC_LDO38_ONOFF_ECO_ADDR(base) ((base) + (0x0037UL))
#define PMIC_LDO39_ONOFF_ECO_ADDR(base) ((base) + (0x0038UL))
#define PMIC_LDO40_ONOFF_ECO_ADDR(base) ((base) + (0x0039UL))
#define PMIC_LDO41_ONOFF_ECO_ADDR(base) ((base) + (0x003AUL))
#define PMIC_LDO_PMUA_ECO_ADDR(base) ((base) + (0x003BUL))
#define PMIC_CLK_ABB_EN_ADDR(base) ((base) + (0x003CUL))
#define PMIC_CLK_WIFI_EN_ADDR(base) ((base) + (0x003DUL))
#define PMIC_CLK_NFC_EN_ADDR(base) ((base) + (0x003EUL))
#define PMIC_CLK_RF0_EN_ADDR(base) ((base) + (0x003FUL))
#define PMIC_CLK_RF1_EN_ADDR(base) ((base) + (0x0040UL))
#define PMIC_CLK_SYS_EN_ADDR(base) ((base) + (0x0041UL))
#define PMIC_CLK_CODEC_EN_ADDR(base) ((base) + (0x0042UL))
#define PMIC_CLK_UFS_EN_ADDR(base) ((base) + (0x0043UL))
#define PMIC_CLK_WIFI_1181_EN_ADDR(base) ((base) + (0x0044UL))
#define PMIC_OSC32K_GPS_ONOFF_CTRL_ADDR(base) ((base) + (0x0045UL))
#define PMIC_OSC32K_BT_ONOFF_CTRL_ADDR(base) ((base) + (0x0046UL))
#define PMIC_OSC32K_SYS_ONOFF_CTRL_ADDR(base) ((base) + (0x0047UL))
#define PMIC_BUCK00_VSET_ADDR(base) ((base) + (0x0048UL))
#define PMIC_BUCK00_VSET_ECO_ADDR(base) ((base) + (0x0049UL))
#define PMIC_BUCK02_VSET_ADDR(base) ((base) + (0x004AUL))
#define PMIC_BUCK02_VSET_ECO_ADDR(base) ((base) + (0x004BUL))
#define PMIC_BUCK1_VSET_ADDR(base) ((base) + (0x004CUL))
#define PMIC_BUCK2_VSET_ADDR(base) ((base) + (0x004DUL))
#define PMIC_BUCK2_VSET_ECO_ADDR(base) ((base) + (0x004EUL))
#define PMIC_BUCK3_VSET_ADDR(base) ((base) + (0x004FUL))
#define PMIC_BUCK3_VSET_ECO_ADDR(base) ((base) + (0x0050UL))
#define PMIC_BUCK4_VSET_ADDR(base) ((base) + (0x0051UL))
#define PMIC_BUCK5_VSET_ADDR(base) ((base) + (0x0052UL))
#define PMIC_BUCK6_VSET_ADDR(base) ((base) + (0x0053UL))
#define PMIC_BUCK7_VSET_ADDR(base) ((base) + (0x0054UL))
#define PMIC_BUCK8_VSET_ADDR(base) ((base) + (0x0055UL))
#define PMIC_BUCK8_VSET_ECO_ADDR(base) ((base) + (0x0056UL))
#define PMIC_BUCK9_VSET_ADDR(base) ((base) + (0x0057UL))
#define PMIC_BUCK9_VSET_ECO_ADDR(base) ((base) + (0x0058UL))
#define PMIC_LDO0_VSET_ADDR(base) ((base) + (0x0059UL))
#define PMIC_LDO0_VSET_ECO_ADDR(base) ((base) + (0x005AUL))
#define PMIC_LDO1_VSET_ADDR(base) ((base) + (0x005BUL))
#define PMIC_LDO2_VSET_ADDR(base) ((base) + (0x005CUL))
#define PMIC_LDO3_VSET_ADDR(base) ((base) + (0x005DUL))
#define PMIC_LDO4_VSET_ADDR(base) ((base) + (0x005EUL))
#define PMIC_LDO5_VSET_ADDR(base) ((base) + (0x005FUL))
#define PMIC_LDO6_VSET_ADDR(base) ((base) + (0x0060UL))
#define PMIC_LDO7_VSET_ADDR(base) ((base) + (0x0061UL))
#define PMIC_LDO8_VSET_ADDR(base) ((base) + (0x0062UL))
#define PMIC_LDO9_VSET_ADDR(base) ((base) + (0x0063UL))
#define PMIC_LDO11_VSET_ADDR(base) ((base) + (0x0064UL))
#define PMIC_LDO12_VSET_ADDR(base) ((base) + (0x0065UL))
#define PMIC_LDO14_VSET_ADDR(base) ((base) + (0x0066UL))
#define PMIC_LDO15_VSET_ADDR(base) ((base) + (0x0067UL))
#define PMIC_LDO16_VSET_ADDR(base) ((base) + (0x0068UL))
#define PMIC_LDO17_VSET_ADDR(base) ((base) + (0x0069UL))
#define PMIC_LDO18_VSET_ADDR(base) ((base) + (0x006AUL))
#define PMIC_LDO21_VSET_ADDR(base) ((base) + (0x006BUL))
#define PMIC_LDO22_VSET_ADDR(base) ((base) + (0x006CUL))
#define PMIC_LDO23_VSET_ADDR(base) ((base) + (0x006DUL))
#define PMIC_LDO24_VSET_ADDR(base) ((base) + (0x006EUL))
#define PMIC_LDO25_VSET_ADDR(base) ((base) + (0x006FUL))
#define PMIC_LDO26_VSET_ADDR(base) ((base) + (0x0070UL))
#define PMIC_LDO27_VSET_ADDR(base) ((base) + (0x0071UL))
#define PMIC_LDO28_VSET_ADDR(base) ((base) + (0x0072UL))
#define PMIC_LDO29_VSET_ADDR(base) ((base) + (0x0073UL))
#define PMIC_LDO30_VSET_ADDR(base) ((base) + (0x0074UL))
#define PMIC_LDO32_VSET_ADDR(base) ((base) + (0x0075UL))
#define PMIC_LDO34_VSET_ADDR(base) ((base) + (0x0076UL))
#define PMIC_PMUH_VSET_ADDR(base) ((base) + (0x0077UL))
#define PMIC_LDO36_VSET_ADDR(base) ((base) + (0x0078UL))
#define PMIC_LDO37_VSET_ADDR(base) ((base) + (0x0079UL))
#define PMIC_LDO38_VSET_ADDR(base) ((base) + (0x007AUL))
#define PMIC_LDO39_VSET_ADDR(base) ((base) + (0x007BUL))
#define PMIC_LDO39_VSET_ECO_ADDR(base) ((base) + (0x007CUL))
#define PMIC_LDO40_VSET_ADDR(base) ((base) + (0x007DUL))
#define PMIC_LDO41_VSET_ADDR(base) ((base) + (0x007EUL))
#define PMIC_LDO_BUF_VSET_ADDR(base) ((base) + (0x007FUL))
#define PMIC_LDO_PMUA_VSET_ADDR(base) ((base) + (0x0080UL))
#define PMIC_PMUD_VSET_ADDR(base) ((base) + (0x0081UL))
#define PMIC_BUCK00_CTRL0_ADDR(base) ((base) + (0x0082UL))
#define PMIC_BUCK00_CTRL1_ADDR(base) ((base) + (0x0083UL))
#define PMIC_BUCK00_CTRL2_ADDR(base) ((base) + (0x0084UL))
#define PMIC_BUCK00_CTRL3_ADDR(base) ((base) + (0x0085UL))
#define PMIC_BUCK00_CTRL4_ADDR(base) ((base) + (0x0086UL))
#define PMIC_BUCK00_CTRL5_ADDR(base) ((base) + (0x0087UL))
#define PMIC_BUCK00_CTRL6_ADDR(base) ((base) + (0x0088UL))
#define PMIC_BUCK00_CTRL7_ADDR(base) ((base) + (0x0089UL))
#define PMIC_BUCK00_CTRL8_ADDR(base) ((base) + (0x008AUL))
#define PMIC_BUCK00_CTRL9_ADDR(base) ((base) + (0x008BUL))
#define PMIC_BUCK00_CTRL10_ADDR(base) ((base) + (0x008CUL))
#define PMIC_BUCK00_CTRL11_ADDR(base) ((base) + (0x008DUL))
#define PMIC_BUCK00_CTRL12_ADDR(base) ((base) + (0x008EUL))
#define PMIC_BUCK00_CTRL13_ADDR(base) ((base) + (0x008FUL))
#define PMIC_BUCK00_CTRL14_ADDR(base) ((base) + (0x0090UL))
#define PMIC_BUCK00_CTRL15_ADDR(base) ((base) + (0x0091UL))
#define PMIC_BUCK00_CTRL16_ADDR(base) ((base) + (0x0092UL))
#define PMIC_BUCK00_CTRL17_ADDR(base) ((base) + (0x0093UL))
#define PMIC_BUCK00_CTRL18_ADDR(base) ((base) + (0x0094UL))
#define PMIC_BUCK00_CTRL19_ADDR(base) ((base) + (0x0095UL))
#define PMIC_BUCK00_CTRL20_ADDR(base) ((base) + (0x0096UL))
#define PMIC_BUCK01_CTRL0_ADDR(base) ((base) + (0x0097UL))
#define PMIC_BUCK01_CTRL1_ADDR(base) ((base) + (0x0098UL))
#define PMIC_BUCK01_CTRL2_ADDR(base) ((base) + (0x0099UL))
#define PMIC_BUCK01_CTRL3_ADDR(base) ((base) + (0x009AUL))
#define PMIC_BUCK01_CTRL4_ADDR(base) ((base) + (0x009BUL))
#define PMIC_BUCK01_CTRL5_ADDR(base) ((base) + (0x009CUL))
#define PMIC_BUCK01_CTRL6_ADDR(base) ((base) + (0x009DUL))
#define PMIC_BUCK01_CTRL7_ADDR(base) ((base) + (0x009EUL))
#define PMIC_BUCK01_CTRL8_ADDR(base) ((base) + (0x009FUL))
#define PMIC_BUCK02_CTRL0_ADDR(base) ((base) + (0x00A0UL))
#define PMIC_BUCK02_CTRL1_ADDR(base) ((base) + (0x00A1UL))
#define PMIC_BUCK02_CTRL2_ADDR(base) ((base) + (0x00A2UL))
#define PMIC_BUCK02_CTRL3_ADDR(base) ((base) + (0x00A3UL))
#define PMIC_BUCK02_CTRL4_ADDR(base) ((base) + (0x00A4UL))
#define PMIC_BUCK02_CTRL5_ADDR(base) ((base) + (0x00A5UL))
#define PMIC_BUCK02_CTRL6_ADDR(base) ((base) + (0x00A6UL))
#define PMIC_BUCK02_CTRL7_ADDR(base) ((base) + (0x00A7UL))
#define PMIC_BUCK02_CTRL8_ADDR(base) ((base) + (0x00A8UL))
#define PMIC_BUCK02_CTRL9_ADDR(base) ((base) + (0x00A9UL))
#define PMIC_BUCK02_CTRL10_ADDR(base) ((base) + (0x00AAUL))
#define PMIC_BUCK02_CTRL11_ADDR(base) ((base) + (0x00ABUL))
#define PMIC_BUCK02_CTRL12_ADDR(base) ((base) + (0x00ACUL))
#define PMIC_BUCK02_CTRL13_ADDR(base) ((base) + (0x00ADUL))
#define PMIC_BUCK02_CTRL14_ADDR(base) ((base) + (0x00AEUL))
#define PMIC_BUCK02_CTRL15_ADDR(base) ((base) + (0x00AFUL))
#define PMIC_BUCK02_CTRL16_ADDR(base) ((base) + (0x00B0UL))
#define PMIC_BUCK02_CTRL17_ADDR(base) ((base) + (0x00B1UL))
#define PMIC_BUCK02_CTRL18_ADDR(base) ((base) + (0x00B2UL))
#define PMIC_BUCK02_CTRL19_ADDR(base) ((base) + (0x00B3UL))
#define PMIC_BUCK02_CTRL20_ADDR(base) ((base) + (0x00B4UL))
#define PMIC_BUCK0_CTRL0_ADDR(base) ((base) + (0x00B5UL))
#define PMIC_BUCK0_CTRL1_ADDR(base) ((base) + (0x00B6UL))
#define PMIC_BUCK0_CTRL2_ADDR(base) ((base) + (0x00B7UL))
#define PMIC_BUCK0_CTRL3_ADDR(base) ((base) + (0x00B8UL))
#define PMIC_BUCK0_CTRL4_ADDR(base) ((base) + (0x00B9UL))
#define PMIC_BUCK0_CTRL5_ADDR(base) ((base) + (0x00BAUL))
#define PMIC_BUCK0_CTRL6_ADDR(base) ((base) + (0x00BBUL))
#define PMIC_BUCK0_CTRL7_ADDR(base) ((base) + (0x00BCUL))
#define PMIC_BUCK0_CTRL8_ADDR(base) ((base) + (0x00BDUL))
#define PMIC_BUCK1_CTRL0_ADDR(base) ((base) + (0x00BEUL))
#define PMIC_BUCK1_CTRL1_ADDR(base) ((base) + (0x00BFUL))
#define PMIC_BUCK1_CTRL2_ADDR(base) ((base) + (0x00C0UL))
#define PMIC_BUCK1_CTRL3_ADDR(base) ((base) + (0x00C1UL))
#define PMIC_BUCK1_CTRL4_ADDR(base) ((base) + (0x00C2UL))
#define PMIC_BUCK1_CTRL5_ADDR(base) ((base) + (0x00C3UL))
#define PMIC_BUCK1_CTRL6_ADDR(base) ((base) + (0x00C4UL))
#define PMIC_BUCK1_CTRL7_ADDR(base) ((base) + (0x00C5UL))
#define PMIC_BUCK1_CTRL8_ADDR(base) ((base) + (0x00C6UL))
#define PMIC_BUCK1_CTRL9_ADDR(base) ((base) + (0x00C7UL))
#define PMIC_BUCK1_CTRL10_ADDR(base) ((base) + (0x00C8UL))
#define PMIC_BUCK1_CTRL11_ADDR(base) ((base) + (0x00C9UL))
#define PMIC_BUCK1_CTRL12_ADDR(base) ((base) + (0x00CAUL))
#define PMIC_BUCK1_CTRL13_ADDR(base) ((base) + (0x00CBUL))
#define PMIC_BUCK1_CTRL14_ADDR(base) ((base) + (0x00CCUL))
#define PMIC_BUCK2_CTRL0_ADDR(base) ((base) + (0x00CDUL))
#define PMIC_BUCK2_CTRL1_ADDR(base) ((base) + (0x00CEUL))
#define PMIC_BUCK2_CTRL2_ADDR(base) ((base) + (0x00CFUL))
#define PMIC_BUCK2_CTRL3_ADDR(base) ((base) + (0x00D0UL))
#define PMIC_BUCK2_CTRL4_ADDR(base) ((base) + (0x00D1UL))
#define PMIC_BUCK2_CTRL5_ADDR(base) ((base) + (0x00D2UL))
#define PMIC_BUCK2_CTRL6_ADDR(base) ((base) + (0x00D3UL))
#define PMIC_BUCK2_CTRL7_ADDR(base) ((base) + (0x00D4UL))
#define PMIC_BUCK2_CTRL8_ADDR(base) ((base) + (0x00D5UL))
#define PMIC_BUCK2_CTRL9_ADDR(base) ((base) + (0x00D6UL))
#define PMIC_BUCK3_CTRL0_ADDR(base) ((base) + (0x00D7UL))
#define PMIC_BUCK3_CTRL1_ADDR(base) ((base) + (0x00D8UL))
#define PMIC_BUCK3_CTRL2_ADDR(base) ((base) + (0x00D9UL))
#define PMIC_BUCK3_CTRL3_ADDR(base) ((base) + (0x00DAUL))
#define PMIC_BUCK3_CTRL4_ADDR(base) ((base) + (0x00DBUL))
#define PMIC_BUCK3_CTRL5_ADDR(base) ((base) + (0x00DCUL))
#define PMIC_BUCK3_CTRL6_ADDR(base) ((base) + (0x00DDUL))
#define PMIC_BUCK3_CTRL7_ADDR(base) ((base) + (0x00DEUL))
#define PMIC_BUCK3_CTRL8_ADDR(base) ((base) + (0x00DFUL))
#define PMIC_BUCK3_CTRL9_ADDR(base) ((base) + (0x00E0UL))
#define PMIC_BUCK4_CTRL0_ADDR(base) ((base) + (0x00E1UL))
#define PMIC_BUCK4_CTRL1_ADDR(base) ((base) + (0x00E2UL))
#define PMIC_BUCK4_CTRL2_ADDR(base) ((base) + (0x00E3UL))
#define PMIC_BUCK4_CTRL3_ADDR(base) ((base) + (0x00E4UL))
#define PMIC_BUCK4_CTRL4_ADDR(base) ((base) + (0x00E5UL))
#define PMIC_BUCK4_CTRL5_ADDR(base) ((base) + (0x00E6UL))
#define PMIC_BUCK4_CTRL6_ADDR(base) ((base) + (0x00E7UL))
#define PMIC_BUCK4_CTRL7_ADDR(base) ((base) + (0x00E8UL))
#define PMIC_BUCK4_CTRL8_ADDR(base) ((base) + (0x00E9UL))
#define PMIC_BUCK4_CTRL9_ADDR(base) ((base) + (0x00EAUL))
#define PMIC_BUCK4_CTRL10_ADDR(base) ((base) + (0x00EBUL))
#define PMIC_BUCK5_CTRL0_ADDR(base) ((base) + (0x00ECUL))
#define PMIC_BUCK5_CTRL1_ADDR(base) ((base) + (0x00EDUL))
#define PMIC_BUCK5_CTRL2_ADDR(base) ((base) + (0x00EEUL))
#define PMIC_BUCK5_CTRL3_ADDR(base) ((base) + (0x00EFUL))
#define PMIC_BUCK5_CTRL4_ADDR(base) ((base) + (0x00F0UL))
#define PMIC_BUCK5_CTRL5_ADDR(base) ((base) + (0x00F1UL))
#define PMIC_BUCK5_CTRL6_ADDR(base) ((base) + (0x00F2UL))
#define PMIC_BUCK5_CTRL7_ADDR(base) ((base) + (0x00F3UL))
#define PMIC_BUCK5_CTRL8_ADDR(base) ((base) + (0x00F4UL))
#define PMIC_BUCK5_CTRL9_ADDR(base) ((base) + (0x00F5UL))
#define PMIC_BUCK5_CTRL10_ADDR(base) ((base) + (0x00F6UL))
#define PMIC_BUCK6_CTRL0_ADDR(base) ((base) + (0x00F7UL))
#define PMIC_BUCK6_CTRL1_ADDR(base) ((base) + (0x00F8UL))
#define PMIC_BUCK6_CTRL2_ADDR(base) ((base) + (0x00F9UL))
#define PMIC_BUCK6_CTRL3_ADDR(base) ((base) + (0x00FAUL))
#define PMIC_BUCK6_CTRL4_ADDR(base) ((base) + (0x00FBUL))
#define PMIC_BUCK6_CTRL5_ADDR(base) ((base) + (0x00FCUL))
#define PMIC_BUCK6_CTRL6_ADDR(base) ((base) + (0x00FDUL))
#define PMIC_BUCK6_CTRL7_ADDR(base) ((base) + (0x00FEUL))
#define PMIC_BUCK6_CTRL8_ADDR(base) ((base) + (0x00FFUL))
#define PMIC_BUCK6_CTRL9_ADDR(base) ((base) + (0x0100UL))
#define PMIC_BUCK6_CTRL10_ADDR(base) ((base) + (0x0101UL))
#define PMIC_BUCK7_CTRL0_ADDR(base) ((base) + (0x0102UL))
#define PMIC_BUCK7_CTRL1_ADDR(base) ((base) + (0x0103UL))
#define PMIC_BUCK7_CTRL2_ADDR(base) ((base) + (0x0104UL))
#define PMIC_BUCK7_CTRL3_ADDR(base) ((base) + (0x0105UL))
#define PMIC_BUCK7_CTRL4_ADDR(base) ((base) + (0x0106UL))
#define PMIC_BUCK7_CTRL5_ADDR(base) ((base) + (0x0107UL))
#define PMIC_BUCK7_CTRL6_ADDR(base) ((base) + (0x0108UL))
#define PMIC_BUCK7_CTRL7_ADDR(base) ((base) + (0x0109UL))
#define PMIC_BUCK7_CTRL8_ADDR(base) ((base) + (0x010AUL))
#define PMIC_BUCK7_CTRL9_ADDR(base) ((base) + (0x010BUL))
#define PMIC_BUCK7_CTRL10_ADDR(base) ((base) + (0x010CUL))
#define PMIC_BUCK7_CTRL11_ADDR(base) ((base) + (0x010DUL))
#define PMIC_BUCK7_CTRL12_ADDR(base) ((base) + (0x010EUL))
#define PMIC_BUCK8_CTRL0_ADDR(base) ((base) + (0x010FUL))
#define PMIC_BUCK8_CTRL1_ADDR(base) ((base) + (0x0110UL))
#define PMIC_BUCK8_CTRL2_ADDR(base) ((base) + (0x0111UL))
#define PMIC_BUCK8_CTRL3_ADDR(base) ((base) + (0x0112UL))
#define PMIC_BUCK8_CTRL4_ADDR(base) ((base) + (0x0113UL))
#define PMIC_BUCK8_CTRL5_ADDR(base) ((base) + (0x0114UL))
#define PMIC_BUCK8_CTRL6_ADDR(base) ((base) + (0x0115UL))
#define PMIC_BUCK8_CTRL7_ADDR(base) ((base) + (0x0116UL))
#define PMIC_BUCK8_CTRL8_ADDR(base) ((base) + (0x0117UL))
#define PMIC_BUCK8_CTRL9_ADDR(base) ((base) + (0x0118UL))
#define PMIC_BUCK9_CTRL0_ADDR(base) ((base) + (0x0119UL))
#define PMIC_BUCK9_CTRL1_ADDR(base) ((base) + (0x011AUL))
#define PMIC_BUCK9_CTRL2_ADDR(base) ((base) + (0x011BUL))
#define PMIC_BUCK9_CTRL3_ADDR(base) ((base) + (0x011CUL))
#define PMIC_BUCK9_CTRL4_ADDR(base) ((base) + (0x011DUL))
#define PMIC_BUCK9_CTRL5_ADDR(base) ((base) + (0x011EUL))
#define PMIC_BUCK9_CTRL6_ADDR(base) ((base) + (0x011FUL))
#define PMIC_BUCK9_CTRL7_ADDR(base) ((base) + (0x0120UL))
#define PMIC_BUCK9_CTRL8_ADDR(base) ((base) + (0x0121UL))
#define PMIC_BUCK9_CTRL9_ADDR(base) ((base) + (0x0122UL))
#define PMIC_BUCK_CTRL0_ADDR(base) ((base) + (0x0123UL))
#define PMIC_BUCK_CTRL1_ADDR(base) ((base) + (0x0124UL))
#define PMIC_BUCK_RESERVE0_ADDR(base) ((base) + (0x0125UL))
#define PMIC_BUCK_RESERVE1_ADDR(base) ((base) + (0x0126UL))
#define PMIC_LDO0_CTRL0_ADDR(base) ((base) + (0x0127UL))
#define PMIC_LDO0_CTRL1_ADDR(base) ((base) + (0x0128UL))
#define PMIC_LDO0_CTRL2_ADDR(base) ((base) + (0x0129UL))
#define PMIC_LDO0_CTRL3_ADDR(base) ((base) + (0x012AUL))
#define PMIC_LDO1_CTRL0_ADDR(base) ((base) + (0x012BUL))
#define PMIC_LDO1_CTRL1_ADDR(base) ((base) + (0x012CUL))
#define PMIC_LDO1_CTRL2_ADDR(base) ((base) + (0x012DUL))
#define PMIC_LDO2_CTRL_ADDR(base) ((base) + (0x012EUL))
#define PMIC_LDO3_CTRL0_ADDR(base) ((base) + (0x012FUL))
#define PMIC_LDO3_CTRL1_ADDR(base) ((base) + (0x0130UL))
#define PMIC_LDO4_CTRL_ADDR(base) ((base) + (0x0131UL))
#define PMIC_LDO5_CTRL_ADDR(base) ((base) + (0x0132UL))
#define PMIC_LDO6_CTRL0_ADDR(base) ((base) + (0x0133UL))
#define PMIC_LDO6_CTRL1_ADDR(base) ((base) + (0x0134UL))
#define PMIC_LDO7_CTRL0_ADDR(base) ((base) + (0x0135UL))
#define PMIC_LDO7_CTRL1_ADDR(base) ((base) + (0x0136UL))
#define PMIC_LDO8_CTRL_ADDR(base) ((base) + (0x0137UL))
#define PMIC_LDO9_CTRL_ADDR(base) ((base) + (0x0138UL))
#define PMIC_LD11_CTRL_ADDR(base) ((base) + (0x0139UL))
#define PMIC_LD12_CTRL_ADDR(base) ((base) + (0x013AUL))
#define PMIC_LDO14_CTRL_ADDR(base) ((base) + (0x013BUL))
#define PMIC_LDO15_CTRL_ADDR(base) ((base) + (0x013CUL))
#define PMIC_LDO16_CTRL_ADDR(base) ((base) + (0x013DUL))
#define PMIC_LDO17_CTRL_ADDR(base) ((base) + (0x013EUL))
#define PMIC_LDO18_CTRL0_ADDR(base) ((base) + (0x013FUL))
#define PMIC_LDO18_CTRL1_ADDR(base) ((base) + (0x0140UL))
#define PMIC_LDO21_CTRL_ADDR(base) ((base) + (0x0141UL))
#define PMIC_LDO22_CTRL0_ADDR(base) ((base) + (0x0142UL))
#define PMIC_LDO22_CTRL1_ADDR(base) ((base) + (0x0143UL))
#define PMIC_LDO22_CTRL2_ADDR(base) ((base) + (0x0144UL))
#define PMIC_LDO23_CTRL_ADDR(base) ((base) + (0x0145UL))
#define PMIC_LDO24_CTRL_ADDR(base) ((base) + (0x0146UL))
#define PMIC_LDO25_CTRL0_ADDR(base) ((base) + (0x0147UL))
#define PMIC_LDO25_CTRL1_ADDR(base) ((base) + (0x0148UL))
#define PMIC_LDO25_CTRL2_ADDR(base) ((base) + (0x0149UL))
#define PMIC_LDO26_CTRL_ADDR(base) ((base) + (0x014AUL))
#define PMIC_LDO27_CTRL_ADDR(base) ((base) + (0x014BUL))
#define PMIC_LDO28_CTRL_ADDR(base) ((base) + (0x014CUL))
#define PMIC_LDO29_CTRL_ADDR(base) ((base) + (0x014DUL))
#define PMIC_LDO30_CTRL0_ADDR(base) ((base) + (0x014EUL))
#define PMIC_LDO30_CTRL1_ADDR(base) ((base) + (0x014FUL))
#define PMIC_LDO32_CTRL0_ADDR(base) ((base) + (0x0150UL))
#define PMIC_LDO32_CTRL1_ADDR(base) ((base) + (0x0151UL))
#define PMIC_LDO32_CTRL2_ADDR(base) ((base) + (0x0152UL))
#define PMIC_LDO32_CTRL3_ADDR(base) ((base) + (0x0153UL))
#define PMIC_LDO34_CTRL_ADDR(base) ((base) + (0x0154UL))
#define PMIC_PMUH_CTRL_ADDR(base) ((base) + (0x0155UL))
#define PMIC_LDO36_CTRL0_ADDR(base) ((base) + (0x0156UL))
#define PMIC_LDO36_CTRL1_ADDR(base) ((base) + (0x0157UL))
#define PMIC_LDO36_CTRL2_ADDR(base) ((base) + (0x0158UL))
#define PMIC_LDO37_CTRL0_ADDR(base) ((base) + (0x0159UL))
#define PMIC_LDO37_CTRL1_ADDR(base) ((base) + (0x015AUL))
#define PMIC_LDO38_CTRL0_ADDR(base) ((base) + (0x015BUL))
#define PMIC_LDO38_CTRL1_ADDR(base) ((base) + (0x015CUL))
#define PMIC_LDO38_CTRL2_ADDR(base) ((base) + (0x015DUL))
#define PMIC_LDO38_CTRL3_ADDR(base) ((base) + (0x015EUL))
#define PMIC_LDO39_CTRL0_ADDR(base) ((base) + (0x015FUL))
#define PMIC_LDO39_CTRL1_ADDR(base) ((base) + (0x0160UL))
#define PMIC_LDO39_CTRL2_ADDR(base) ((base) + (0x0161UL))
#define PMIC_LDO39_CTRL3_ADDR(base) ((base) + (0x0162UL))
#define PMIC_LDO40_CTRL0_ADDR(base) ((base) + (0x0163UL))
#define PMIC_LDO40_CTRL1_ADDR(base) ((base) + (0x0164UL))
#define PMIC_LDO40_CTRL2_ADDR(base) ((base) + (0x0165UL))
#define PMIC_LDO40_CTRL3_ADDR(base) ((base) + (0x0166UL))
#define PMIC_LDO41_CTRL0_ADDR(base) ((base) + (0x0167UL))
#define PMIC_LDO41_CTRL1_ADDR(base) ((base) + (0x0168UL))
#define PMIC_LDO41_CTRL2_ADDR(base) ((base) + (0x0169UL))
#define PMIC_LDO_BUF_PMUA_CTRL_ADDR(base) ((base) + (0x016AUL))
#define PMIC_LDO_RESERVE0_ADDR(base) ((base) + (0x016BUL))
#define PMIC_LDO_RESERVE1_ADDR(base) ((base) + (0x016CUL))
#define PMIC_LDO_RESERVE2_ADDR(base) ((base) + (0x016DUL))
#define PMIC_LDO_RESERVE3_ADDR(base) ((base) + (0x016EUL))
#define PMIC_BUCK0_3_OCP_CTRL_ADDR(base) ((base) + (0x016FUL))
#define PMIC_BUCK4_7_OCP_CTRL_ADDR(base) ((base) + (0x0170UL))
#define PMIC_BUCK8_9_OCP_CTRL_ADDR(base) ((base) + (0x0171UL))
#define PMIC_LDO0_3_OCP_CTRL_ADDR(base) ((base) + (0x0172UL))
#define PMIC_LDO4_7_OCP_CTRL_ADDR(base) ((base) + (0x0173UL))
#define PMIC_LDO8_12_OCP_CTRL_ADDR(base) ((base) + (0x0174UL))
#define PMIC_LDO14_17_OCP_CTRL_ADDR(base) ((base) + (0x0175UL))
#define PMIC_LDO18_23_OCP_CTRL_ADDR(base) ((base) + (0x0176UL))
#define PMIC_LDO24_27_OCP_CTRL_ADDR(base) ((base) + (0x0177UL))
#define PMIC_LDO28_32_OCP_CTRL_ADDR(base) ((base) + (0x0178UL))
#define PMIC_LDO34_38_OCP_CTRL_ADDR(base) ((base) + (0x0179UL))
#define PMIC_LDO39_41_OCP_CTRL_ADDR(base) ((base) + (0x017AUL))
#define PMIC_LDO_BUFF_SCP_CTRL_ADDR(base) ((base) + (0x017BUL))
#define PMIC_BUCK0_3_SCP_CTRL_ADDR(base) ((base) + (0x017CUL))
#define PMIC_BUCK4_7_SCP_CTRL_ADDR(base) ((base) + (0x017DUL))
#define PMIC_BUCK8_9_SCP_CTRL_ADDR(base) ((base) + (0x017EUL))
#define PMIC_BUCK0_OCP_CTRL0_ADDR(base) ((base) + (0x017FUL))
#define PMIC_SYS_CTRL_RESERVE_ADDR(base) ((base) + (0x0180UL))
#define PMIC_OCP_DEB_CTRL0_ADDR(base) ((base) + (0x0181UL))
#define PMIC_OCP_DEB_CTRL1_ADDR(base) ((base) + (0x0182UL))
#define PMIC_PWROFF_DEB_CTRL_ADDR(base) ((base) + (0x0183UL))
#define PMIC_VSYS_DROP_DEB_CTRL_ADDR(base) ((base) + (0x0184UL))
#define PMIC_OCP_SCP_ONOFF_ADDR(base) ((base) + (0x0185UL))
#define PMIC_CLK_ABB_CTRL0_ADDR(base) ((base) + (0x0186UL))
#define PMIC_CLK_WIFI_CTRL0_ADDR(base) ((base) + (0x0187UL))
#define PMIC_CLK_NFC_CTRL0_ADDR(base) ((base) + (0x0188UL))
#define PMIC_CLK_RF0_CTRL0_ADDR(base) ((base) + (0x0189UL))
#define PMIC_CLK_RF1_CTRL0_ADDR(base) ((base) + (0x018AUL))
#define PMIC_CLK_USB_CTRL_ADDR(base) ((base) + (0x018BUL))
#define PMIC_CLK_CODEC_CTRL_ADDR(base) ((base) + (0x018CUL))
#define PMIC_CLK_UFS_CTRL_ADDR(base) ((base) + (0x018DUL))
#define PMIC_CLK_WIFI_1181_CTRL_ADDR(base) ((base) + (0x018EUL))
#define PMIC_TON_ECO_SHIELD_CTRL_ADDR(base) ((base) + (0x018FUL))
#define PMIC_CLK_TOP_CTRL0_ADDR(base) ((base) + (0x0190UL))
#define PMIC_CLK_TOP_CTRL1_ADDR(base) ((base) + (0x0191UL))
#define PMIC_CLK_TOP_CTRL2_ADDR(base) ((base) + (0x0192UL))
#define PMIC_BG_THSD_CTRL0_ADDR(base) ((base) + (0x0193UL))
#define PMIC_BG_THSD_CTRL1_ADDR(base) ((base) + (0x0194UL))
#define PMIC_BG_TEST_ADDR(base) ((base) + (0x0195UL))
#define PMIC_HARDWIRE_CTRL0_ADDR(base) ((base) + (0x0196UL))
#define PMIC_HARDWIRE_CTRL1_ADDR(base) ((base) + (0x0197UL))
#define PMIC_LOW_POWER_CFG_ADDR(base) ((base) + (0x0198UL))
#define PMIC_PERI_CTRL0_ADDR(base) ((base) + (0x0199UL))
#define PMIC_PERI_CTRL1_ADDR(base) ((base) + (0x019AUL))
#define PMIC_PERI_CTRL2_ADDR(base) ((base) + (0x019BUL))
#define PMIC_PERI_CTRL3_ADDR(base) ((base) + (0x019CUL))
#define PMIC_PERI_CTRL4_ADDR(base) ((base) + (0x019DUL))
#define PMIC_PERI_CTRL5_ADDR(base) ((base) + (0x019EUL))
#define PMIC_PERI_CTRL6_ADDR(base) ((base) + (0x019FUL))
#define PMIC_PERI_CTRL7_ADDR(base) ((base) + (0x01A0UL))
#define PMIC_PERI_CNT_CFG_ADDR(base) ((base) + (0x01A1UL))
#define PMIC_PERI_CTRL8_ADDR(base) ((base) + (0x01A2UL))
#define PMIC_PERI_CTRL9_ADDR(base) ((base) + (0x01A3UL))
#define PMIC_PERI_CTRL10_ADDR(base) ((base) + (0x01A4UL))
#define PMIC_PERI_CTRL11_ADDR(base) ((base) + (0x01A5UL))
#define PMIC_PERI_CTRL12_ADDR(base) ((base) + (0x01A6UL))
#define PMIC_PERI_CTRL13_ADDR(base) ((base) + (0x01A7UL))
#define PMIC_PERI_CTRL14_ADDR(base) ((base) + (0x01A8UL))
#define PMIC_PERI_CTRL15_ADDR(base) ((base) + (0x01A9UL))
#define PMIC_PERI_CTRL16_ADDR(base) ((base) + (0x01AAUL))
#define PMIC_PERI_CTRL17_ADDR(base) ((base) + (0x01ABUL))
#define PMIC_PERI_CTRL18_ADDR(base) ((base) + (0x01ACUL))
#define PMIC_PERI_CTRL19_ADDR(base) ((base) + (0x01ADUL))
#define PMIC_PERI_CTRL20_ADDR(base) ((base) + (0x01AEUL))
#define PMIC_PERI_CTRL21_ADDR(base) ((base) + (0x01AFUL))
#define PMIC_PERI_CTRL22_ADDR(base) ((base) + (0x01B0UL))
#define PMIC_PERI_CTRL23_ADDR(base) ((base) + (0x01B1UL))
#define PMIC_PERI_CTRL24_ADDR(base) ((base) + (0x01B2UL))
#define PMIC_PERI_CTRL25_ADDR(base) ((base) + (0x01B3UL))
#define PMIC_PERI_CTRL26_ADDR(base) ((base) + (0x01B4UL))
#define PMIC_WIFI_CTRL_ADDR(base) ((base) + (0x01B5UL))
#define PMIC_PERI_VSET_CTRL_ADDR(base) ((base) + (0x01B6UL))
#define PMIC_IDLE_CTRL0_ADDR(base) ((base) + (0x01B7UL))
#define PMIC_IDLE_CTRL1_ADDR(base) ((base) + (0x01B8UL))
#define PMIC_IDLE_CTRL2_ADDR(base) ((base) + (0x01B9UL))
#define PMIC_IDLE_CTRL3_ADDR(base) ((base) + (0x01BAUL))
#define PMIC_IDLE_CTRL4_ADDR(base) ((base) + (0x01BBUL))
#define PMIC_IDLE_CTRL5_ADDR(base) ((base) + (0x01BCUL))
#define PMIC_IDLE_CTRL6_ADDR(base) ((base) + (0x01BDUL))
#define PMIC_IDLE_CTRL7_ADDR(base) ((base) + (0x01BEUL))
#define PMIC_IDLE_CNT_CFG_ADDR(base) ((base) + (0x01BFUL))
#define PMIC_IDLE_CTRL8_ADDR(base) ((base) + (0x01C0UL))
#define PMIC_IDLE_CTRL9_ADDR(base) ((base) + (0x01C1UL))
#define PMIC_IDLE_CTRL10_ADDR(base) ((base) + (0x01C2UL))
#define PMIC_IDLE_CTRL11_ADDR(base) ((base) + (0x01C3UL))
#define PMIC_IDLE_CTRL12_ADDR(base) ((base) + (0x01C4UL))
#define PMIC_IDLE_CTRL13_ADDR(base) ((base) + (0x01C5UL))
#define PMIC_IDLE_CTRL14_ADDR(base) ((base) + (0x01C6UL))
#define PMIC_IDLE_CTRL15_ADDR(base) ((base) + (0x01C7UL))
#define PMIC_IDLE_CTRL16_ADDR(base) ((base) + (0x01C8UL))
#define PMIC_IDLE_CTRL17_ADDR(base) ((base) + (0x01C9UL))
#define PMIC_IDLE_CTRL18_ADDR(base) ((base) + (0x01CAUL))
#define PMIC_IDLE_CTRL19_ADDR(base) ((base) + (0x01CBUL))
#define PMIC_IDLE_CTRL20_ADDR(base) ((base) + (0x01CCUL))
#define PMIC_IDLE_CTRL21_ADDR(base) ((base) + (0x01CDUL))
#define PMIC_IDLE_CTRL22_ADDR(base) ((base) + (0x01CEUL))
#define PMIC_IDLE_CTRL23_ADDR(base) ((base) + (0x01CFUL))
#define PMIC_IDLE_CTRL24_ADDR(base) ((base) + (0x01D0UL))
#define PMIC_IDLE_CTRL25_ADDR(base) ((base) + (0x01D1UL))
#define PMIC_IDLE_CTRL26_ADDR(base) ((base) + (0x01D2UL))
#define PMIC_COUL_ECO_MASK_ADDR(base) ((base) + (0x01D3UL))
#define PMIC_PMU_SOFT_RST_ADDR(base) ((base) + (0x01D4UL))
#define PMIC_LOCK_ADDR(base) ((base) + (0x01D5UL))
#define PMIC_DR_EN_123_ADDR(base) ((base) + (0x01D6UL))
#define PMIC_DR_EN_4_ADDR(base) ((base) + (0x01D7UL))
#define PMIC_DR_EN_MODE_123_ADDR(base) ((base) + (0x01D8UL))
#define PMIC_FLASH_PERIOD_DR1_ADDR(base) ((base) + (0x01D9UL))
#define PMIC_FLASH_ON_DR1_ADDR(base) ((base) + (0x01DAUL))
#define PMIC_FLASH_PERIOD_DR2_ADDR(base) ((base) + (0x01DBUL))
#define PMIC_FLASH_ON_DR2_ADDR(base) ((base) + (0x01DCUL))
#define PMIC_FLASH_PERIOD_DR3_ADDR(base) ((base) + (0x01DDUL))
#define PMIC_FLASH_ON_DR3_ADDR(base) ((base) + (0x01DEUL))
#define PMIC_DR_MODE_SEL_ADDR(base) ((base) + (0x01DFUL))
#define PMIC_DR_BRE_CTRL_ADDR(base) ((base) + (0x01E0UL))
#define PMIC_DR1_ISET_ADDR(base) ((base) + (0x01E1UL))
#define PMIC_DR2_ISET_ADDR(base) ((base) + (0x01E2UL))
#define PMIC_DR3_ISET_ADDR(base) ((base) + (0x01E3UL))
#define PMIC_DR4_ISET_ADDR(base) ((base) + (0x01E4UL))
#define PMIC_DR_OUT_CTRL_ADDR(base) ((base) + (0x01E5UL))
#define PMIC_DR1_START_DEL_ADDR(base) ((base) + (0x01E6UL))
#define PMIC_DR2_START_DEL_ADDR(base) ((base) + (0x01E7UL))
#define PMIC_DR3_START_DEL_ADDR(base) ((base) + (0x01E8UL))
#define PMIC_DR1_TIM_CONF0_ADDR(base) ((base) + (0x01E9UL))
#define PMIC_DR1_TIM_CONF1_ADDR(base) ((base) + (0x01EAUL))
#define PMIC_DR2_TIM_CONF0_ADDR(base) ((base) + (0x01EBUL))
#define PMIC_DR2_TIM_CONF1_ADDR(base) ((base) + (0x01ECUL))
#define PMIC_DR3_TIM_CONF0_ADDR(base) ((base) + (0x01EDUL))
#define PMIC_DR3_TIM_CONF1_ADDR(base) ((base) + (0x01EEUL))
#define PMIC_DR4_CTRLRESERVE_ADDR(base) ((base) + (0x01EFUL))
#define PMIC_DR_CTRLRESERVE8_ADDR(base) ((base) + (0x01F0UL))
#define PMIC_DR_CTRLRESERVE9_ADDR(base) ((base) + (0x01F1UL))
#define PMIC_OTP0_0_ADDR(base) ((base) + (0x01F2UL))
#define PMIC_OTP0_1_ADDR(base) ((base) + (0x01F3UL))
#define PMIC_OTP0_CTRL0_ADDR(base) ((base) + (0x01F4UL))
#define PMIC_OTP0_CTRL1_ADDR(base) ((base) + (0x01F5UL))
#define PMIC_OTP0_CTRL2_ADDR(base) ((base) + (0x01F6UL))
#define PMIC_OTP0_WDATA_ADDR(base) ((base) + (0x01F7UL))
#define PMIC_OTP0_0_W_ADDR(base) ((base) + (0x01F8UL))
#define PMIC_OTP0_1_W_ADDR(base) ((base) + (0x01F9UL))
#define PMIC_OTP0_2_W_ADDR(base) ((base) + (0x01FAUL))
#define PMIC_OTP0_3_W_ADDR(base) ((base) + (0x01FBUL))
#define PMIC_OTP0_4_W_ADDR(base) ((base) + (0x01FCUL))
#define PMIC_OTP0_5_W_ADDR(base) ((base) + (0x01FDUL))
#define PMIC_OTP0_6_W_ADDR(base) ((base) + (0x01FEUL))
#define PMIC_OTP0_7_W_ADDR(base) ((base) + (0x01FFUL))
#define PMIC_OTP0_8_W_ADDR(base) ((base) + (0x0200UL))
#define PMIC_OTP0_9_W_ADDR(base) ((base) + (0x0201UL))
#define PMIC_OTP0_10_W_ADDR(base) ((base) + (0x0202UL))
#define PMIC_OTP0_11_W_ADDR(base) ((base) + (0x0203UL))
#define PMIC_OTP0_12_W_ADDR(base) ((base) + (0x0204UL))
#define PMIC_OTP0_13_W_ADDR(base) ((base) + (0x0205UL))
#define PMIC_OTP0_14_W_ADDR(base) ((base) + (0x0206UL))
#define PMIC_OTP0_15_W_ADDR(base) ((base) + (0x0207UL))
#define PMIC_OTP0_16_W_ADDR(base) ((base) + (0x0208UL))
#define PMIC_OTP0_17_W_ADDR(base) ((base) + (0x0209UL))
#define PMIC_OTP0_18_W_ADDR(base) ((base) + (0x020AUL))
#define PMIC_OTP0_19_W_ADDR(base) ((base) + (0x020BUL))
#define PMIC_OTP0_20_W_ADDR(base) ((base) + (0x020CUL))
#define PMIC_OTP0_21_W_ADDR(base) ((base) + (0x020DUL))
#define PMIC_OTP0_22_W_ADDR(base) ((base) + (0x020EUL))
#define PMIC_OTP0_23_W_ADDR(base) ((base) + (0x020FUL))
#define PMIC_OTP0_24_W_ADDR(base) ((base) + (0x0210UL))
#define PMIC_OTP0_25_W_ADDR(base) ((base) + (0x0211UL))
#define PMIC_OTP0_26_W_ADDR(base) ((base) + (0x0212UL))
#define PMIC_OTP0_27_W_ADDR(base) ((base) + (0x0213UL))
#define PMIC_OTP0_28_W_ADDR(base) ((base) + (0x0214UL))
#define PMIC_OTP0_29_W_ADDR(base) ((base) + (0x0215UL))
#define PMIC_OTP0_30_W_ADDR(base) ((base) + (0x0216UL))
#define PMIC_OTP0_31_W_ADDR(base) ((base) + (0x0217UL))
#define PMIC_OTP0_32_W_ADDR(base) ((base) + (0x0218UL))
#define PMIC_OTP0_33_W_ADDR(base) ((base) + (0x0219UL))
#define PMIC_OTP0_34_W_ADDR(base) ((base) + (0x021AUL))
#define PMIC_OTP0_35_W_ADDR(base) ((base) + (0x021BUL))
#define PMIC_OTP0_36_W_ADDR(base) ((base) + (0x021CUL))
#define PMIC_OTP0_37_W_ADDR(base) ((base) + (0x021DUL))
#define PMIC_OTP0_38_W_ADDR(base) ((base) + (0x021EUL))
#define PMIC_OTP0_39_W_ADDR(base) ((base) + (0x021FUL))
#define PMIC_OTP0_40_W_ADDR(base) ((base) + (0x0220UL))
#define PMIC_OTP0_41_W_ADDR(base) ((base) + (0x0221UL))
#define PMIC_OTP0_42_W_ADDR(base) ((base) + (0x0222UL))
#define PMIC_OTP0_43_W_ADDR(base) ((base) + (0x0223UL))
#define PMIC_OTP0_44_W_ADDR(base) ((base) + (0x0224UL))
#define PMIC_OTP0_45_W_ADDR(base) ((base) + (0x0225UL))
#define PMIC_OTP0_46_W_ADDR(base) ((base) + (0x0226UL))
#define PMIC_OTP0_47_W_ADDR(base) ((base) + (0x0227UL))
#define PMIC_OTP0_48_W_ADDR(base) ((base) + (0x0228UL))
#define PMIC_OTP0_49_W_ADDR(base) ((base) + (0x0229UL))
#define PMIC_OTP0_50_W_ADDR(base) ((base) + (0x022AUL))
#define PMIC_OTP0_51_W_ADDR(base) ((base) + (0x022BUL))
#define PMIC_OTP0_52_W_ADDR(base) ((base) + (0x022CUL))
#define PMIC_OTP0_53_W_ADDR(base) ((base) + (0x022DUL))
#define PMIC_OTP0_54_W_ADDR(base) ((base) + (0x022EUL))
#define PMIC_OTP0_55_W_ADDR(base) ((base) + (0x022FUL))
#define PMIC_OTP0_56_W_ADDR(base) ((base) + (0x0230UL))
#define PMIC_OTP0_57_W_ADDR(base) ((base) + (0x0231UL))
#define PMIC_OTP0_58_W_ADDR(base) ((base) + (0x0232UL))
#define PMIC_OTP0_59_W_ADDR(base) ((base) + (0x0233UL))
#define PMIC_OTP0_60_W_ADDR(base) ((base) + (0x0234UL))
#define PMIC_OTP0_61_W_ADDR(base) ((base) + (0x0235UL))
#define PMIC_OTP0_62_W_ADDR(base) ((base) + (0x0236UL))
#define PMIC_OTP0_63_W_ADDR(base) ((base) + (0x0237UL))
#define PMIC_OTP1_0_ADDR(base) ((base) + (0x0238UL))
#define PMIC_OTP1_1_ADDR(base) ((base) + (0x0239UL))
#define PMIC_OTP1_CTRL0_ADDR(base) ((base) + (0x023AUL))
#define PMIC_OTP1_CTRL1_ADDR(base) ((base) + (0x023BUL))
#define PMIC_OTP1_CTRL2_ADDR(base) ((base) + (0x023CUL))
#define PMIC_OTP1_WDATA_ADDR(base) ((base) + (0x023DUL))
#define PMIC_OTP1_0_W_ADDR(base) ((base) + (0x023EUL))
#define PMIC_OTP1_1_W_ADDR(base) ((base) + (0x023FUL))
#define PMIC_OTP1_2_W_ADDR(base) ((base) + (0x0240UL))
#define PMIC_D2A_RES0_ADDR(base) ((base) + (0x0241UL))
#define PMIC_D2A_RES1_ADDR(base) ((base) + (0x0242UL))
#define PMIC_A2D_RES0_ADDR(base) ((base) + (0x0243UL))
#define PMIC_SIM_CTRL0_ADDR(base) ((base) + (0x0244UL))
#define PMIC_SIM_CTRL1_ADDR(base) ((base) + (0x0245UL))
#define PMIC_SIM_DEB_CTRL1_ADDR(base) ((base) + (0x0246UL))
#define PMIC_SIM_DEB_CTRL2_ADDR(base) ((base) + (0x0247UL))
#define PMIC_GPIO0DATA_ADDR(base) ((base) + (0x0248UL))
#define PMIC_GPIO0_DATA_RESERVE_ADDR(base) ((base) + (0x0249UL))
#define PMIC_GPIO0DIR_ADDR(base) ((base) + (0x024AUL))
#define PMIC_GPIO0_DIR_RESERVE_ADDR(base) ((base) + (0x024BUL))
#define PMIC_GPIO0IS_ADDR(base) ((base) + (0x024CUL))
#define PMIC_GPIO0IBE_ADDR(base) ((base) + (0x024DUL))
#define PMIC_GPIO0IEV_ADDR(base) ((base) + (0x024EUL))
#define PMIC_GPIO0AFSEL_ADDR(base) ((base) + (0x024FUL))
#define PMIC_GPIO0DSSEL_ADDR(base) ((base) + (0x0250UL))
#define PMIC_GPIO0PUSEL_ADDR(base) ((base) + (0x0251UL))
#define PMIC_GPIO0PDSEL_ADDR(base) ((base) + (0x0252UL))
#define PMIC_GPIO0DEBSEL_ADDR(base) ((base) + (0x0253UL))
#define PMIC_SIM0PUSEL_ADDR(base) ((base) + (0x0254UL))
#define PMIC_SIM0PDSEL_ADDR(base) ((base) + (0x0255UL))
#define PMIC_GPIO1DATA_ADDR(base) ((base) + (0x0256UL))
#define PMIC_GPIO1_DATA_RESERVE_ADDR(base) ((base) + (0x0257UL))
#define PMIC_GPIO1DIR_ADDR(base) ((base) + (0x0258UL))
#define PMIC_GPIO1_DIR_RESERVE_ADDR(base) ((base) + (0x0259UL))
#define PMIC_GPIO1IS_ADDR(base) ((base) + (0x025AUL))
#define PMIC_GPIO1IBE_ADDR(base) ((base) + (0x025BUL))
#define PMIC_GPIO1IEV_ADDR(base) ((base) + (0x025CUL))
#define PMIC_GPIO1AFSEL_ADDR(base) ((base) + (0x025DUL))
#define PMIC_GPIO1DSSEL_ADDR(base) ((base) + (0x025EUL))
#define PMIC_GPIO1PUSEL_ADDR(base) ((base) + (0x025FUL))
#define PMIC_GPIO1PDSEL_ADDR(base) ((base) + (0x0260UL))
#define PMIC_GPIO1DEBSEL_ADDR(base) ((base) + (0x0261UL))
#define PMIC_SIM1PUSEL_ADDR(base) ((base) + (0x0262UL))
#define PMIC_SIM1PDSEL_ADDR(base) ((base) + (0x0263UL))
#define PMIC_GPIO2DATA_ADDR(base) ((base) + (0x0264UL))
#define PMIC_GPIO2_DATA_RESERVE_ADDR(base) ((base) + (0x0265UL))
#define PMIC_GPIO2DIR_ADDR(base) ((base) + (0x0266UL))
#define PMIC_GPIO2_DIR_RESERVE_ADDR(base) ((base) + (0x0267UL))
#define PMIC_GPIO2IS_ADDR(base) ((base) + (0x0268UL))
#define PMIC_GPIO2IBE_ADDR(base) ((base) + (0x0269UL))
#define PMIC_GPIO2IEV_ADDR(base) ((base) + (0x026AUL))
#define PMIC_GPIO2DSSEL_ADDR(base) ((base) + (0x026BUL))
#define PMIC_GPIO2PUSEL_ADDR(base) ((base) + (0x026CUL))
#define PMIC_GPIO2PDSEL_ADDR(base) ((base) + (0x026DUL))
#define PMIC_GPIO2DEBSEL_ADDR(base) ((base) + (0x026EUL))
#define PMIC_GPIO3DATA_ADDR(base) ((base) + (0x026FUL))
#define PMIC_GPIO3_DATA_RESERVE_ADDR(base) ((base) + (0x0270UL))
#define PMIC_GPIO3DIR_ADDR(base) ((base) + (0x0271UL))
#define PMIC_GPIO3_DIR_RESERVE_ADDR(base) ((base) + (0x0272UL))
#define PMIC_GPIO3IS_ADDR(base) ((base) + (0x0273UL))
#define PMIC_GPIO3IBE_ADDR(base) ((base) + (0x0274UL))
#define PMIC_GPIO3IEV_ADDR(base) ((base) + (0x0275UL))
#define PMIC_GPIO3DSSEL_ADDR(base) ((base) + (0x0276UL))
#define PMIC_GPIO3PUSEL_ADDR(base) ((base) + (0x0277UL))
#define PMIC_GPIO3PDSEL_ADDR(base) ((base) + (0x0278UL))
#define PMIC_GPIO3DEBSEL_ADDR(base) ((base) + (0x0279UL))
#define PMIC_AUX_IBIAS_CFG_ADDR(base) ((base) + (0x027AUL))
#define PMIC_DAC_DIN_MSB_ADDR(base) ((base) + (0x027BUL))
#define PMIC_DAC_DIN_LSB_ADDR(base) ((base) + (0x027CUL))
#define PMIC_RAMP_BUCK00_CTRL0_ADDR(base) ((base) + (0x027DUL))
#define PMIC_RAMP_BUCK00_CTRL1_ADDR(base) ((base) + (0x027EUL))
#define PMIC_BUCK00_RAMPUP_STATE_ADDR(base) ((base) + (0x027FUL))
#define PMIC_BUCK00_RAMPDOWN_STATE_ADDR(base) ((base) + (0x0280UL))
#define PMIC_BUCK00_RAMP_CHG_CTRL_ADDR(base) ((base) + (0x0281UL))
#define PMIC_RAMP_BUCK02_CTRL0_ADDR(base) ((base) + (0x0282UL))
#define PMIC_RAMP_BUCK02_CTRL1_ADDR(base) ((base) + (0x0283UL))
#define PMIC_BUCK02_RAMPUP_STATE_ADDR(base) ((base) + (0x0284UL))
#define PMIC_BUCK02_RAMPDOWN_STATE_ADDR(base) ((base) + (0x0285UL))
#define PMIC_BUCK02_RAMP_CHG_CTRL_ADDR(base) ((base) + (0x0286UL))
#define PMIC_RAMP_BUCK4_CTRL0_ADDR(base) ((base) + (0x0287UL))
#define PMIC_RAMP_BUCK4_CTRL1_ADDR(base) ((base) + (0x0288UL))
#define PMIC_BUCK4_RAMPUP_STATE_ADDR(base) ((base) + (0x0289UL))
#define PMIC_BUCK4_RAMPDOWN_STATE_ADDR(base) ((base) + (0x028AUL))
#define PMIC_BUCK4_RAMP_CHG_CTRL_ADDR(base) ((base) + (0x028BUL))
#define PMIC_RAMP_BUCK5_CTRL0_ADDR(base) ((base) + (0x028CUL))
#define PMIC_RAMP_BUCK5_CTRL1_ADDR(base) ((base) + (0x028DUL))
#define PMIC_BUCK5_RAMPUP_STATE_ADDR(base) ((base) + (0x028EUL))
#define PMIC_BUCK5_RAMPDOWN_STATE_ADDR(base) ((base) + (0x028FUL))
#define PMIC_BUCK5_RAMP_CHG_CTRL_ADDR(base) ((base) + (0x0290UL))
#define PMIC_RAMP_BUCK6_CTRL0_ADDR(base) ((base) + (0x0291UL))
#define PMIC_RAMP_BUCK6_CTRL1_ADDR(base) ((base) + (0x0292UL))
#define PMIC_BUCK6_RAMPUP_STATE_ADDR(base) ((base) + (0x0293UL))
#define PMIC_BUCK6_RAMPDOWN_STATE_ADDR(base) ((base) + (0x0294UL))
#define PMIC_BUCK6_RAMP_CHG_CTRL_ADDR(base) ((base) + (0x0295UL))
#define PMIC_RAMP_LDO0_CTRL0_ADDR(base) ((base) + (0x0296UL))
#define PMIC_RAMP_LDO0_CTRL1_ADDR(base) ((base) + (0x0297UL))
#define PMIC_RAMP_LDO39_CTRL0_ADDR(base) ((base) + (0x0298UL))
#define PMIC_RAMP_LDO39_CTRL1_ADDR(base) ((base) + (0x0299UL))
#define PMIC_RAMP_LDO40_CTRL0_ADDR(base) ((base) + (0x029AUL))
#define PMIC_RAMP_LDO40_CTRL1_ADDR(base) ((base) + (0x029BUL))
#define PMIC_SPMI_ECO_GT_BYPASS_ADDR(base) ((base) + (0x029CUL))
#define PMIC_PWRONN_8S_XOADC_DEBUG_CTRL_ADDR(base) ((base) + (0x029DUL))
#define PMIC_DIS_19M2_CTRL_ADDR(base) ((base) + (0x029EUL))
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
#define PMIC_STATUS3_ADDR(base) ((base) + (0x0009))
#define PMIC_BUCK00_ONOFF_ECO_ADDR(base) ((base) + (0x000A))
#define PMIC_BUCK01_ONOFF_ADDR(base) ((base) + (0x000B))
#define PMIC_BUCK02_ONOFF_ECO_ADDR(base) ((base) + (0x000C))
#define PMIC_BUCK1_ONOFF_ECO_ADDR(base) ((base) + (0x000D))
#define PMIC_BUCK2_ONOFF_ECO_ADDR(base) ((base) + (0x000E))
#define PMIC_BUCK3_ONOFF_ECO_ADDR(base) ((base) + (0x000F))
#define PMIC_BUCK4_ONOFF_ECO_ADDR(base) ((base) + (0x0010))
#define PMIC_BUCK5_ONOFF_ECO_ADDR(base) ((base) + (0x0011))
#define PMIC_BUCK6_ONOFF_ECO_ADDR(base) ((base) + (0x0012))
#define PMIC_BUCK7_ONOFF_ECO_ADDR(base) ((base) + (0x0013))
#define PMIC_BUCK8_ONOFF_ECO_ADDR(base) ((base) + (0x0014))
#define PMIC_BUCK9_ONOFF_ECO_ADDR(base) ((base) + (0x0015))
#define PMIC_LDO0_ONOFF_ECO_ADDR(base) ((base) + (0x0016))
#define PMIC_LDO1_ONOFF_ADDR(base) ((base) + (0x0017))
#define PMIC_LDO2_ONOFF1_ADDR(base) ((base) + (0x0018))
#define PMIC_LDO2_ONOFF2_ADDR(base) ((base) + (0x0019))
#define PMIC_LDO3_ONOFF_ADDR(base) ((base) + (0x001A))
#define PMIC_LDO4_ONOFF_ECO_ADDR(base) ((base) + (0x001B))
#define PMIC_LDO5_ONOFF_ECO_ADDR(base) ((base) + (0x001C))
#define PMIC_LDO6_ONOFF_ECO_ADDR(base) ((base) + (0x001D))
#define PMIC_LDO7_ONOFF_ECO_ADDR(base) ((base) + (0x001E))
#define PMIC_LDO8_ONOFF_ECO_ADDR(base) ((base) + (0x001F))
#define PMIC_LDO9_ONOFF_ECO_ADDR(base) ((base) + (0x0020))
#define PMIC_LDO11_ONOFF_ECO_ADDR(base) ((base) + (0x0021))
#define PMIC_LDO12_ONOFF_ECO_ADDR(base) ((base) + (0x0022))
#define PMIC_LDO14_ONOFF_ADDR(base) ((base) + (0x0023))
#define PMIC_LDO15_ONOFF_ECO_ADDR(base) ((base) + (0x0024))
#define PMIC_LDO16_ONOFF_ECO_ADDR(base) ((base) + (0x0025))
#define PMIC_LDO17_ONOFF_ECO_ADDR(base) ((base) + (0x0026))
#define PMIC_LDO18_ONOFF_ECO_ADDR(base) ((base) + (0x0027))
#define PMIC_LDO21_ONOFF_ADDR(base) ((base) + (0x0028))
#define PMIC_LDO22_ONOFF_ADDR(base) ((base) + (0x0029))
#define PMIC_LDO23_ONOFF_ECO_ADDR(base) ((base) + (0x002A))
#define PMIC_LDO24_ONOFF_ECO_ADDR(base) ((base) + (0x002B))
#define PMIC_LDO25_ONOFF_ADDR(base) ((base) + (0x002C))
#define PMIC_LDO26_ONOFF_ECO_ADDR(base) ((base) + (0x002D))
#define PMIC_LDO27_ONOFF_ECO_ADDR(base) ((base) + (0x002E))
#define PMIC_LDO28_ONOFF_ADDR(base) ((base) + (0x002F))
#define PMIC_LDO29_ONOFF_ECO_ADDR(base) ((base) + (0x0030))
#define PMIC_LDO30_ONOFF_ECO_ADDR(base) ((base) + (0x0031))
#define PMIC_LDO32_ONOFF_ADDR(base) ((base) + (0x0032))
#define PMIC_LDO34_ONOFF_ECO_ADDR(base) ((base) + (0x0033))
#define PMIC_PMUH_ONOFF_ECO_ADDR(base) ((base) + (0x0034))
#define PMIC_LDO36_ONOFF_ECO_ADDR(base) ((base) + (0x0035))
#define PMIC_LDO37_ONOFF_ECO_ADDR(base) ((base) + (0x0036))
#define PMIC_LDO38_ONOFF_ECO_ADDR(base) ((base) + (0x0037))
#define PMIC_LDO39_ONOFF_ECO_ADDR(base) ((base) + (0x0038))
#define PMIC_LDO40_ONOFF_ECO_ADDR(base) ((base) + (0x0039))
#define PMIC_LDO41_ONOFF_ECO_ADDR(base) ((base) + (0x003A))
#define PMIC_LDO_PMUA_ECO_ADDR(base) ((base) + (0x003B))
#define PMIC_CLK_ABB_EN_ADDR(base) ((base) + (0x003C))
#define PMIC_CLK_WIFI_EN_ADDR(base) ((base) + (0x003D))
#define PMIC_CLK_NFC_EN_ADDR(base) ((base) + (0x003E))
#define PMIC_CLK_RF0_EN_ADDR(base) ((base) + (0x003F))
#define PMIC_CLK_RF1_EN_ADDR(base) ((base) + (0x0040))
#define PMIC_CLK_SYS_EN_ADDR(base) ((base) + (0x0041))
#define PMIC_CLK_CODEC_EN_ADDR(base) ((base) + (0x0042))
#define PMIC_CLK_UFS_EN_ADDR(base) ((base) + (0x0043))
#define PMIC_CLK_WIFI_1181_EN_ADDR(base) ((base) + (0x0044))
#define PMIC_OSC32K_GPS_ONOFF_CTRL_ADDR(base) ((base) + (0x0045))
#define PMIC_OSC32K_BT_ONOFF_CTRL_ADDR(base) ((base) + (0x0046))
#define PMIC_OSC32K_SYS_ONOFF_CTRL_ADDR(base) ((base) + (0x0047))
#define PMIC_BUCK00_VSET_ADDR(base) ((base) + (0x0048))
#define PMIC_BUCK00_VSET_ECO_ADDR(base) ((base) + (0x0049))
#define PMIC_BUCK02_VSET_ADDR(base) ((base) + (0x004A))
#define PMIC_BUCK02_VSET_ECO_ADDR(base) ((base) + (0x004B))
#define PMIC_BUCK1_VSET_ADDR(base) ((base) + (0x004C))
#define PMIC_BUCK2_VSET_ADDR(base) ((base) + (0x004D))
#define PMIC_BUCK2_VSET_ECO_ADDR(base) ((base) + (0x004E))
#define PMIC_BUCK3_VSET_ADDR(base) ((base) + (0x004F))
#define PMIC_BUCK3_VSET_ECO_ADDR(base) ((base) + (0x0050))
#define PMIC_BUCK4_VSET_ADDR(base) ((base) + (0x0051))
#define PMIC_BUCK5_VSET_ADDR(base) ((base) + (0x0052))
#define PMIC_BUCK6_VSET_ADDR(base) ((base) + (0x0053))
#define PMIC_BUCK7_VSET_ADDR(base) ((base) + (0x0054))
#define PMIC_BUCK8_VSET_ADDR(base) ((base) + (0x0055))
#define PMIC_BUCK8_VSET_ECO_ADDR(base) ((base) + (0x0056))
#define PMIC_BUCK9_VSET_ADDR(base) ((base) + (0x0057))
#define PMIC_BUCK9_VSET_ECO_ADDR(base) ((base) + (0x0058))
#define PMIC_LDO0_VSET_ADDR(base) ((base) + (0x0059))
#define PMIC_LDO0_VSET_ECO_ADDR(base) ((base) + (0x005A))
#define PMIC_LDO1_VSET_ADDR(base) ((base) + (0x005B))
#define PMIC_LDO2_VSET_ADDR(base) ((base) + (0x005C))
#define PMIC_LDO3_VSET_ADDR(base) ((base) + (0x005D))
#define PMIC_LDO4_VSET_ADDR(base) ((base) + (0x005E))
#define PMIC_LDO5_VSET_ADDR(base) ((base) + (0x005F))
#define PMIC_LDO6_VSET_ADDR(base) ((base) + (0x0060))
#define PMIC_LDO7_VSET_ADDR(base) ((base) + (0x0061))
#define PMIC_LDO8_VSET_ADDR(base) ((base) + (0x0062))
#define PMIC_LDO9_VSET_ADDR(base) ((base) + (0x0063))
#define PMIC_LDO11_VSET_ADDR(base) ((base) + (0x0064))
#define PMIC_LDO12_VSET_ADDR(base) ((base) + (0x0065))
#define PMIC_LDO14_VSET_ADDR(base) ((base) + (0x0066))
#define PMIC_LDO15_VSET_ADDR(base) ((base) + (0x0067))
#define PMIC_LDO16_VSET_ADDR(base) ((base) + (0x0068))
#define PMIC_LDO17_VSET_ADDR(base) ((base) + (0x0069))
#define PMIC_LDO18_VSET_ADDR(base) ((base) + (0x006A))
#define PMIC_LDO21_VSET_ADDR(base) ((base) + (0x006B))
#define PMIC_LDO22_VSET_ADDR(base) ((base) + (0x006C))
#define PMIC_LDO23_VSET_ADDR(base) ((base) + (0x006D))
#define PMIC_LDO24_VSET_ADDR(base) ((base) + (0x006E))
#define PMIC_LDO25_VSET_ADDR(base) ((base) + (0x006F))
#define PMIC_LDO26_VSET_ADDR(base) ((base) + (0x0070))
#define PMIC_LDO27_VSET_ADDR(base) ((base) + (0x0071))
#define PMIC_LDO28_VSET_ADDR(base) ((base) + (0x0072))
#define PMIC_LDO29_VSET_ADDR(base) ((base) + (0x0073))
#define PMIC_LDO30_VSET_ADDR(base) ((base) + (0x0074))
#define PMIC_LDO32_VSET_ADDR(base) ((base) + (0x0075))
#define PMIC_LDO34_VSET_ADDR(base) ((base) + (0x0076))
#define PMIC_PMUH_VSET_ADDR(base) ((base) + (0x0077))
#define PMIC_LDO36_VSET_ADDR(base) ((base) + (0x0078))
#define PMIC_LDO37_VSET_ADDR(base) ((base) + (0x0079))
#define PMIC_LDO38_VSET_ADDR(base) ((base) + (0x007A))
#define PMIC_LDO39_VSET_ADDR(base) ((base) + (0x007B))
#define PMIC_LDO39_VSET_ECO_ADDR(base) ((base) + (0x007C))
#define PMIC_LDO40_VSET_ADDR(base) ((base) + (0x007D))
#define PMIC_LDO41_VSET_ADDR(base) ((base) + (0x007E))
#define PMIC_LDO_BUF_VSET_ADDR(base) ((base) + (0x007F))
#define PMIC_LDO_PMUA_VSET_ADDR(base) ((base) + (0x0080))
#define PMIC_PMUD_VSET_ADDR(base) ((base) + (0x0081))
#define PMIC_BUCK00_CTRL0_ADDR(base) ((base) + (0x0082))
#define PMIC_BUCK00_CTRL1_ADDR(base) ((base) + (0x0083))
#define PMIC_BUCK00_CTRL2_ADDR(base) ((base) + (0x0084))
#define PMIC_BUCK00_CTRL3_ADDR(base) ((base) + (0x0085))
#define PMIC_BUCK00_CTRL4_ADDR(base) ((base) + (0x0086))
#define PMIC_BUCK00_CTRL5_ADDR(base) ((base) + (0x0087))
#define PMIC_BUCK00_CTRL6_ADDR(base) ((base) + (0x0088))
#define PMIC_BUCK00_CTRL7_ADDR(base) ((base) + (0x0089))
#define PMIC_BUCK00_CTRL8_ADDR(base) ((base) + (0x008A))
#define PMIC_BUCK00_CTRL9_ADDR(base) ((base) + (0x008B))
#define PMIC_BUCK00_CTRL10_ADDR(base) ((base) + (0x008C))
#define PMIC_BUCK00_CTRL11_ADDR(base) ((base) + (0x008D))
#define PMIC_BUCK00_CTRL12_ADDR(base) ((base) + (0x008E))
#define PMIC_BUCK00_CTRL13_ADDR(base) ((base) + (0x008F))
#define PMIC_BUCK00_CTRL14_ADDR(base) ((base) + (0x0090))
#define PMIC_BUCK00_CTRL15_ADDR(base) ((base) + (0x0091))
#define PMIC_BUCK00_CTRL16_ADDR(base) ((base) + (0x0092))
#define PMIC_BUCK00_CTRL17_ADDR(base) ((base) + (0x0093))
#define PMIC_BUCK00_CTRL18_ADDR(base) ((base) + (0x0094))
#define PMIC_BUCK00_CTRL19_ADDR(base) ((base) + (0x0095))
#define PMIC_BUCK00_CTRL20_ADDR(base) ((base) + (0x0096))
#define PMIC_BUCK01_CTRL0_ADDR(base) ((base) + (0x0097))
#define PMIC_BUCK01_CTRL1_ADDR(base) ((base) + (0x0098))
#define PMIC_BUCK01_CTRL2_ADDR(base) ((base) + (0x0099))
#define PMIC_BUCK01_CTRL3_ADDR(base) ((base) + (0x009A))
#define PMIC_BUCK01_CTRL4_ADDR(base) ((base) + (0x009B))
#define PMIC_BUCK01_CTRL5_ADDR(base) ((base) + (0x009C))
#define PMIC_BUCK01_CTRL6_ADDR(base) ((base) + (0x009D))
#define PMIC_BUCK01_CTRL7_ADDR(base) ((base) + (0x009E))
#define PMIC_BUCK01_CTRL8_ADDR(base) ((base) + (0x009F))
#define PMIC_BUCK02_CTRL0_ADDR(base) ((base) + (0x00A0))
#define PMIC_BUCK02_CTRL1_ADDR(base) ((base) + (0x00A1))
#define PMIC_BUCK02_CTRL2_ADDR(base) ((base) + (0x00A2))
#define PMIC_BUCK02_CTRL3_ADDR(base) ((base) + (0x00A3))
#define PMIC_BUCK02_CTRL4_ADDR(base) ((base) + (0x00A4))
#define PMIC_BUCK02_CTRL5_ADDR(base) ((base) + (0x00A5))
#define PMIC_BUCK02_CTRL6_ADDR(base) ((base) + (0x00A6))
#define PMIC_BUCK02_CTRL7_ADDR(base) ((base) + (0x00A7))
#define PMIC_BUCK02_CTRL8_ADDR(base) ((base) + (0x00A8))
#define PMIC_BUCK02_CTRL9_ADDR(base) ((base) + (0x00A9))
#define PMIC_BUCK02_CTRL10_ADDR(base) ((base) + (0x00AA))
#define PMIC_BUCK02_CTRL11_ADDR(base) ((base) + (0x00AB))
#define PMIC_BUCK02_CTRL12_ADDR(base) ((base) + (0x00AC))
#define PMIC_BUCK02_CTRL13_ADDR(base) ((base) + (0x00AD))
#define PMIC_BUCK02_CTRL14_ADDR(base) ((base) + (0x00AE))
#define PMIC_BUCK02_CTRL15_ADDR(base) ((base) + (0x00AF))
#define PMIC_BUCK02_CTRL16_ADDR(base) ((base) + (0x00B0))
#define PMIC_BUCK02_CTRL17_ADDR(base) ((base) + (0x00B1))
#define PMIC_BUCK02_CTRL18_ADDR(base) ((base) + (0x00B2))
#define PMIC_BUCK02_CTRL19_ADDR(base) ((base) + (0x00B3))
#define PMIC_BUCK02_CTRL20_ADDR(base) ((base) + (0x00B4))
#define PMIC_BUCK0_CTRL0_ADDR(base) ((base) + (0x00B5))
#define PMIC_BUCK0_CTRL1_ADDR(base) ((base) + (0x00B6))
#define PMIC_BUCK0_CTRL2_ADDR(base) ((base) + (0x00B7))
#define PMIC_BUCK0_CTRL3_ADDR(base) ((base) + (0x00B8))
#define PMIC_BUCK0_CTRL4_ADDR(base) ((base) + (0x00B9))
#define PMIC_BUCK0_CTRL5_ADDR(base) ((base) + (0x00BA))
#define PMIC_BUCK0_CTRL6_ADDR(base) ((base) + (0x00BB))
#define PMIC_BUCK0_CTRL7_ADDR(base) ((base) + (0x00BC))
#define PMIC_BUCK0_CTRL8_ADDR(base) ((base) + (0x00BD))
#define PMIC_BUCK1_CTRL0_ADDR(base) ((base) + (0x00BE))
#define PMIC_BUCK1_CTRL1_ADDR(base) ((base) + (0x00BF))
#define PMIC_BUCK1_CTRL2_ADDR(base) ((base) + (0x00C0))
#define PMIC_BUCK1_CTRL3_ADDR(base) ((base) + (0x00C1))
#define PMIC_BUCK1_CTRL4_ADDR(base) ((base) + (0x00C2))
#define PMIC_BUCK1_CTRL5_ADDR(base) ((base) + (0x00C3))
#define PMIC_BUCK1_CTRL6_ADDR(base) ((base) + (0x00C4))
#define PMIC_BUCK1_CTRL7_ADDR(base) ((base) + (0x00C5))
#define PMIC_BUCK1_CTRL8_ADDR(base) ((base) + (0x00C6))
#define PMIC_BUCK1_CTRL9_ADDR(base) ((base) + (0x00C7))
#define PMIC_BUCK1_CTRL10_ADDR(base) ((base) + (0x00C8))
#define PMIC_BUCK1_CTRL11_ADDR(base) ((base) + (0x00C9))
#define PMIC_BUCK1_CTRL12_ADDR(base) ((base) + (0x00CA))
#define PMIC_BUCK1_CTRL13_ADDR(base) ((base) + (0x00CB))
#define PMIC_BUCK1_CTRL14_ADDR(base) ((base) + (0x00CC))
#define PMIC_BUCK2_CTRL0_ADDR(base) ((base) + (0x00CD))
#define PMIC_BUCK2_CTRL1_ADDR(base) ((base) + (0x00CE))
#define PMIC_BUCK2_CTRL2_ADDR(base) ((base) + (0x00CF))
#define PMIC_BUCK2_CTRL3_ADDR(base) ((base) + (0x00D0))
#define PMIC_BUCK2_CTRL4_ADDR(base) ((base) + (0x00D1))
#define PMIC_BUCK2_CTRL5_ADDR(base) ((base) + (0x00D2))
#define PMIC_BUCK2_CTRL6_ADDR(base) ((base) + (0x00D3))
#define PMIC_BUCK2_CTRL7_ADDR(base) ((base) + (0x00D4))
#define PMIC_BUCK2_CTRL8_ADDR(base) ((base) + (0x00D5))
#define PMIC_BUCK2_CTRL9_ADDR(base) ((base) + (0x00D6))
#define PMIC_BUCK3_CTRL0_ADDR(base) ((base) + (0x00D7))
#define PMIC_BUCK3_CTRL1_ADDR(base) ((base) + (0x00D8))
#define PMIC_BUCK3_CTRL2_ADDR(base) ((base) + (0x00D9))
#define PMIC_BUCK3_CTRL3_ADDR(base) ((base) + (0x00DA))
#define PMIC_BUCK3_CTRL4_ADDR(base) ((base) + (0x00DB))
#define PMIC_BUCK3_CTRL5_ADDR(base) ((base) + (0x00DC))
#define PMIC_BUCK3_CTRL6_ADDR(base) ((base) + (0x00DD))
#define PMIC_BUCK3_CTRL7_ADDR(base) ((base) + (0x00DE))
#define PMIC_BUCK3_CTRL8_ADDR(base) ((base) + (0x00DF))
#define PMIC_BUCK3_CTRL9_ADDR(base) ((base) + (0x00E0))
#define PMIC_BUCK4_CTRL0_ADDR(base) ((base) + (0x00E1))
#define PMIC_BUCK4_CTRL1_ADDR(base) ((base) + (0x00E2))
#define PMIC_BUCK4_CTRL2_ADDR(base) ((base) + (0x00E3))
#define PMIC_BUCK4_CTRL3_ADDR(base) ((base) + (0x00E4))
#define PMIC_BUCK4_CTRL4_ADDR(base) ((base) + (0x00E5))
#define PMIC_BUCK4_CTRL5_ADDR(base) ((base) + (0x00E6))
#define PMIC_BUCK4_CTRL6_ADDR(base) ((base) + (0x00E7))
#define PMIC_BUCK4_CTRL7_ADDR(base) ((base) + (0x00E8))
#define PMIC_BUCK4_CTRL8_ADDR(base) ((base) + (0x00E9))
#define PMIC_BUCK4_CTRL9_ADDR(base) ((base) + (0x00EA))
#define PMIC_BUCK4_CTRL10_ADDR(base) ((base) + (0x00EB))
#define PMIC_BUCK5_CTRL0_ADDR(base) ((base) + (0x00EC))
#define PMIC_BUCK5_CTRL1_ADDR(base) ((base) + (0x00ED))
#define PMIC_BUCK5_CTRL2_ADDR(base) ((base) + (0x00EE))
#define PMIC_BUCK5_CTRL3_ADDR(base) ((base) + (0x00EF))
#define PMIC_BUCK5_CTRL4_ADDR(base) ((base) + (0x00F0))
#define PMIC_BUCK5_CTRL5_ADDR(base) ((base) + (0x00F1))
#define PMIC_BUCK5_CTRL6_ADDR(base) ((base) + (0x00F2))
#define PMIC_BUCK5_CTRL7_ADDR(base) ((base) + (0x00F3))
#define PMIC_BUCK5_CTRL8_ADDR(base) ((base) + (0x00F4))
#define PMIC_BUCK5_CTRL9_ADDR(base) ((base) + (0x00F5))
#define PMIC_BUCK5_CTRL10_ADDR(base) ((base) + (0x00F6))
#define PMIC_BUCK6_CTRL0_ADDR(base) ((base) + (0x00F7))
#define PMIC_BUCK6_CTRL1_ADDR(base) ((base) + (0x00F8))
#define PMIC_BUCK6_CTRL2_ADDR(base) ((base) + (0x00F9))
#define PMIC_BUCK6_CTRL3_ADDR(base) ((base) + (0x00FA))
#define PMIC_BUCK6_CTRL4_ADDR(base) ((base) + (0x00FB))
#define PMIC_BUCK6_CTRL5_ADDR(base) ((base) + (0x00FC))
#define PMIC_BUCK6_CTRL6_ADDR(base) ((base) + (0x00FD))
#define PMIC_BUCK6_CTRL7_ADDR(base) ((base) + (0x00FE))
#define PMIC_BUCK6_CTRL8_ADDR(base) ((base) + (0x00FF))
#define PMIC_BUCK6_CTRL9_ADDR(base) ((base) + (0x0100))
#define PMIC_BUCK6_CTRL10_ADDR(base) ((base) + (0x0101))
#define PMIC_BUCK7_CTRL0_ADDR(base) ((base) + (0x0102))
#define PMIC_BUCK7_CTRL1_ADDR(base) ((base) + (0x0103))
#define PMIC_BUCK7_CTRL2_ADDR(base) ((base) + (0x0104))
#define PMIC_BUCK7_CTRL3_ADDR(base) ((base) + (0x0105))
#define PMIC_BUCK7_CTRL4_ADDR(base) ((base) + (0x0106))
#define PMIC_BUCK7_CTRL5_ADDR(base) ((base) + (0x0107))
#define PMIC_BUCK7_CTRL6_ADDR(base) ((base) + (0x0108))
#define PMIC_BUCK7_CTRL7_ADDR(base) ((base) + (0x0109))
#define PMIC_BUCK7_CTRL8_ADDR(base) ((base) + (0x010A))
#define PMIC_BUCK7_CTRL9_ADDR(base) ((base) + (0x010B))
#define PMIC_BUCK7_CTRL10_ADDR(base) ((base) + (0x010C))
#define PMIC_BUCK7_CTRL11_ADDR(base) ((base) + (0x010D))
#define PMIC_BUCK7_CTRL12_ADDR(base) ((base) + (0x010E))
#define PMIC_BUCK8_CTRL0_ADDR(base) ((base) + (0x010F))
#define PMIC_BUCK8_CTRL1_ADDR(base) ((base) + (0x0110))
#define PMIC_BUCK8_CTRL2_ADDR(base) ((base) + (0x0111))
#define PMIC_BUCK8_CTRL3_ADDR(base) ((base) + (0x0112))
#define PMIC_BUCK8_CTRL4_ADDR(base) ((base) + (0x0113))
#define PMIC_BUCK8_CTRL5_ADDR(base) ((base) + (0x0114))
#define PMIC_BUCK8_CTRL6_ADDR(base) ((base) + (0x0115))
#define PMIC_BUCK8_CTRL7_ADDR(base) ((base) + (0x0116))
#define PMIC_BUCK8_CTRL8_ADDR(base) ((base) + (0x0117))
#define PMIC_BUCK8_CTRL9_ADDR(base) ((base) + (0x0118))
#define PMIC_BUCK9_CTRL0_ADDR(base) ((base) + (0x0119))
#define PMIC_BUCK9_CTRL1_ADDR(base) ((base) + (0x011A))
#define PMIC_BUCK9_CTRL2_ADDR(base) ((base) + (0x011B))
#define PMIC_BUCK9_CTRL3_ADDR(base) ((base) + (0x011C))
#define PMIC_BUCK9_CTRL4_ADDR(base) ((base) + (0x011D))
#define PMIC_BUCK9_CTRL5_ADDR(base) ((base) + (0x011E))
#define PMIC_BUCK9_CTRL6_ADDR(base) ((base) + (0x011F))
#define PMIC_BUCK9_CTRL7_ADDR(base) ((base) + (0x0120))
#define PMIC_BUCK9_CTRL8_ADDR(base) ((base) + (0x0121))
#define PMIC_BUCK9_CTRL9_ADDR(base) ((base) + (0x0122))
#define PMIC_BUCK_CTRL0_ADDR(base) ((base) + (0x0123))
#define PMIC_BUCK_CTRL1_ADDR(base) ((base) + (0x0124))
#define PMIC_BUCK_RESERVE0_ADDR(base) ((base) + (0x0125))
#define PMIC_BUCK_RESERVE1_ADDR(base) ((base) + (0x0126))
#define PMIC_LDO0_CTRL0_ADDR(base) ((base) + (0x0127))
#define PMIC_LDO0_CTRL1_ADDR(base) ((base) + (0x0128))
#define PMIC_LDO0_CTRL2_ADDR(base) ((base) + (0x0129))
#define PMIC_LDO0_CTRL3_ADDR(base) ((base) + (0x012A))
#define PMIC_LDO1_CTRL0_ADDR(base) ((base) + (0x012B))
#define PMIC_LDO1_CTRL1_ADDR(base) ((base) + (0x012C))
#define PMIC_LDO1_CTRL2_ADDR(base) ((base) + (0x012D))
#define PMIC_LDO2_CTRL_ADDR(base) ((base) + (0x012E))
#define PMIC_LDO3_CTRL0_ADDR(base) ((base) + (0x012F))
#define PMIC_LDO3_CTRL1_ADDR(base) ((base) + (0x0130))
#define PMIC_LDO4_CTRL_ADDR(base) ((base) + (0x0131))
#define PMIC_LDO5_CTRL_ADDR(base) ((base) + (0x0132))
#define PMIC_LDO6_CTRL0_ADDR(base) ((base) + (0x0133))
#define PMIC_LDO6_CTRL1_ADDR(base) ((base) + (0x0134))
#define PMIC_LDO7_CTRL0_ADDR(base) ((base) + (0x0135))
#define PMIC_LDO7_CTRL1_ADDR(base) ((base) + (0x0136))
#define PMIC_LDO8_CTRL_ADDR(base) ((base) + (0x0137))
#define PMIC_LDO9_CTRL_ADDR(base) ((base) + (0x0138))
#define PMIC_LD11_CTRL_ADDR(base) ((base) + (0x0139))
#define PMIC_LD12_CTRL_ADDR(base) ((base) + (0x013A))
#define PMIC_LDO14_CTRL_ADDR(base) ((base) + (0x013B))
#define PMIC_LDO15_CTRL_ADDR(base) ((base) + (0x013C))
#define PMIC_LDO16_CTRL_ADDR(base) ((base) + (0x013D))
#define PMIC_LDO17_CTRL_ADDR(base) ((base) + (0x013E))
#define PMIC_LDO18_CTRL0_ADDR(base) ((base) + (0x013F))
#define PMIC_LDO18_CTRL1_ADDR(base) ((base) + (0x0140))
#define PMIC_LDO21_CTRL_ADDR(base) ((base) + (0x0141))
#define PMIC_LDO22_CTRL0_ADDR(base) ((base) + (0x0142))
#define PMIC_LDO22_CTRL1_ADDR(base) ((base) + (0x0143))
#define PMIC_LDO22_CTRL2_ADDR(base) ((base) + (0x0144))
#define PMIC_LDO23_CTRL_ADDR(base) ((base) + (0x0145))
#define PMIC_LDO24_CTRL_ADDR(base) ((base) + (0x0146))
#define PMIC_LDO25_CTRL0_ADDR(base) ((base) + (0x0147))
#define PMIC_LDO25_CTRL1_ADDR(base) ((base) + (0x0148))
#define PMIC_LDO25_CTRL2_ADDR(base) ((base) + (0x0149))
#define PMIC_LDO26_CTRL_ADDR(base) ((base) + (0x014A))
#define PMIC_LDO27_CTRL_ADDR(base) ((base) + (0x014B))
#define PMIC_LDO28_CTRL_ADDR(base) ((base) + (0x014C))
#define PMIC_LDO29_CTRL_ADDR(base) ((base) + (0x014D))
#define PMIC_LDO30_CTRL0_ADDR(base) ((base) + (0x014E))
#define PMIC_LDO30_CTRL1_ADDR(base) ((base) + (0x014F))
#define PMIC_LDO32_CTRL0_ADDR(base) ((base) + (0x0150))
#define PMIC_LDO32_CTRL1_ADDR(base) ((base) + (0x0151))
#define PMIC_LDO32_CTRL2_ADDR(base) ((base) + (0x0152))
#define PMIC_LDO32_CTRL3_ADDR(base) ((base) + (0x0153))
#define PMIC_LDO34_CTRL_ADDR(base) ((base) + (0x0154))
#define PMIC_PMUH_CTRL_ADDR(base) ((base) + (0x0155))
#define PMIC_LDO36_CTRL0_ADDR(base) ((base) + (0x0156))
#define PMIC_LDO36_CTRL1_ADDR(base) ((base) + (0x0157))
#define PMIC_LDO36_CTRL2_ADDR(base) ((base) + (0x0158))
#define PMIC_LDO37_CTRL0_ADDR(base) ((base) + (0x0159))
#define PMIC_LDO37_CTRL1_ADDR(base) ((base) + (0x015A))
#define PMIC_LDO38_CTRL0_ADDR(base) ((base) + (0x015B))
#define PMIC_LDO38_CTRL1_ADDR(base) ((base) + (0x015C))
#define PMIC_LDO38_CTRL2_ADDR(base) ((base) + (0x015D))
#define PMIC_LDO38_CTRL3_ADDR(base) ((base) + (0x015E))
#define PMIC_LDO39_CTRL0_ADDR(base) ((base) + (0x015F))
#define PMIC_LDO39_CTRL1_ADDR(base) ((base) + (0x0160))
#define PMIC_LDO39_CTRL2_ADDR(base) ((base) + (0x0161))
#define PMIC_LDO39_CTRL3_ADDR(base) ((base) + (0x0162))
#define PMIC_LDO40_CTRL0_ADDR(base) ((base) + (0x0163))
#define PMIC_LDO40_CTRL1_ADDR(base) ((base) + (0x0164))
#define PMIC_LDO40_CTRL2_ADDR(base) ((base) + (0x0165))
#define PMIC_LDO40_CTRL3_ADDR(base) ((base) + (0x0166))
#define PMIC_LDO41_CTRL0_ADDR(base) ((base) + (0x0167))
#define PMIC_LDO41_CTRL1_ADDR(base) ((base) + (0x0168))
#define PMIC_LDO41_CTRL2_ADDR(base) ((base) + (0x0169))
#define PMIC_LDO_BUF_PMUA_CTRL_ADDR(base) ((base) + (0x016A))
#define PMIC_LDO_RESERVE0_ADDR(base) ((base) + (0x016B))
#define PMIC_LDO_RESERVE1_ADDR(base) ((base) + (0x016C))
#define PMIC_LDO_RESERVE2_ADDR(base) ((base) + (0x016D))
#define PMIC_LDO_RESERVE3_ADDR(base) ((base) + (0x016E))
#define PMIC_BUCK0_3_OCP_CTRL_ADDR(base) ((base) + (0x016F))
#define PMIC_BUCK4_7_OCP_CTRL_ADDR(base) ((base) + (0x0170))
#define PMIC_BUCK8_9_OCP_CTRL_ADDR(base) ((base) + (0x0171))
#define PMIC_LDO0_3_OCP_CTRL_ADDR(base) ((base) + (0x0172))
#define PMIC_LDO4_7_OCP_CTRL_ADDR(base) ((base) + (0x0173))
#define PMIC_LDO8_12_OCP_CTRL_ADDR(base) ((base) + (0x0174))
#define PMIC_LDO14_17_OCP_CTRL_ADDR(base) ((base) + (0x0175))
#define PMIC_LDO18_23_OCP_CTRL_ADDR(base) ((base) + (0x0176))
#define PMIC_LDO24_27_OCP_CTRL_ADDR(base) ((base) + (0x0177))
#define PMIC_LDO28_32_OCP_CTRL_ADDR(base) ((base) + (0x0178))
#define PMIC_LDO34_38_OCP_CTRL_ADDR(base) ((base) + (0x0179))
#define PMIC_LDO39_41_OCP_CTRL_ADDR(base) ((base) + (0x017A))
#define PMIC_LDO_BUFF_SCP_CTRL_ADDR(base) ((base) + (0x017B))
#define PMIC_BUCK0_3_SCP_CTRL_ADDR(base) ((base) + (0x017C))
#define PMIC_BUCK4_7_SCP_CTRL_ADDR(base) ((base) + (0x017D))
#define PMIC_BUCK8_9_SCP_CTRL_ADDR(base) ((base) + (0x017E))
#define PMIC_BUCK0_OCP_CTRL0_ADDR(base) ((base) + (0x017F))
#define PMIC_SYS_CTRL_RESERVE_ADDR(base) ((base) + (0x0180))
#define PMIC_OCP_DEB_CTRL0_ADDR(base) ((base) + (0x0181))
#define PMIC_OCP_DEB_CTRL1_ADDR(base) ((base) + (0x0182))
#define PMIC_PWROFF_DEB_CTRL_ADDR(base) ((base) + (0x0183))
#define PMIC_VSYS_DROP_DEB_CTRL_ADDR(base) ((base) + (0x0184))
#define PMIC_OCP_SCP_ONOFF_ADDR(base) ((base) + (0x0185))
#define PMIC_CLK_ABB_CTRL0_ADDR(base) ((base) + (0x0186))
#define PMIC_CLK_WIFI_CTRL0_ADDR(base) ((base) + (0x0187))
#define PMIC_CLK_NFC_CTRL0_ADDR(base) ((base) + (0x0188))
#define PMIC_CLK_RF0_CTRL0_ADDR(base) ((base) + (0x0189))
#define PMIC_CLK_RF1_CTRL0_ADDR(base) ((base) + (0x018A))
#define PMIC_CLK_USB_CTRL_ADDR(base) ((base) + (0x018B))
#define PMIC_CLK_CODEC_CTRL_ADDR(base) ((base) + (0x018C))
#define PMIC_CLK_UFS_CTRL_ADDR(base) ((base) + (0x018D))
#define PMIC_CLK_WIFI_1181_CTRL_ADDR(base) ((base) + (0x018E))
#define PMIC_TON_ECO_SHIELD_CTRL_ADDR(base) ((base) + (0x018F))
#define PMIC_CLK_TOP_CTRL0_ADDR(base) ((base) + (0x0190))
#define PMIC_CLK_TOP_CTRL1_ADDR(base) ((base) + (0x0191))
#define PMIC_CLK_TOP_CTRL2_ADDR(base) ((base) + (0x0192))
#define PMIC_BG_THSD_CTRL0_ADDR(base) ((base) + (0x0193))
#define PMIC_BG_THSD_CTRL1_ADDR(base) ((base) + (0x0194))
#define PMIC_BG_TEST_ADDR(base) ((base) + (0x0195))
#define PMIC_HARDWIRE_CTRL0_ADDR(base) ((base) + (0x0196))
#define PMIC_HARDWIRE_CTRL1_ADDR(base) ((base) + (0x0197))
#define PMIC_LOW_POWER_CFG_ADDR(base) ((base) + (0x0198))
#define PMIC_PERI_CTRL0_ADDR(base) ((base) + (0x0199))
#define PMIC_PERI_CTRL1_ADDR(base) ((base) + (0x019A))
#define PMIC_PERI_CTRL2_ADDR(base) ((base) + (0x019B))
#define PMIC_PERI_CTRL3_ADDR(base) ((base) + (0x019C))
#define PMIC_PERI_CTRL4_ADDR(base) ((base) + (0x019D))
#define PMIC_PERI_CTRL5_ADDR(base) ((base) + (0x019E))
#define PMIC_PERI_CTRL6_ADDR(base) ((base) + (0x019F))
#define PMIC_PERI_CTRL7_ADDR(base) ((base) + (0x01A0))
#define PMIC_PERI_CNT_CFG_ADDR(base) ((base) + (0x01A1))
#define PMIC_PERI_CTRL8_ADDR(base) ((base) + (0x01A2))
#define PMIC_PERI_CTRL9_ADDR(base) ((base) + (0x01A3))
#define PMIC_PERI_CTRL10_ADDR(base) ((base) + (0x01A4))
#define PMIC_PERI_CTRL11_ADDR(base) ((base) + (0x01A5))
#define PMIC_PERI_CTRL12_ADDR(base) ((base) + (0x01A6))
#define PMIC_PERI_CTRL13_ADDR(base) ((base) + (0x01A7))
#define PMIC_PERI_CTRL14_ADDR(base) ((base) + (0x01A8))
#define PMIC_PERI_CTRL15_ADDR(base) ((base) + (0x01A9))
#define PMIC_PERI_CTRL16_ADDR(base) ((base) + (0x01AA))
#define PMIC_PERI_CTRL17_ADDR(base) ((base) + (0x01AB))
#define PMIC_PERI_CTRL18_ADDR(base) ((base) + (0x01AC))
#define PMIC_PERI_CTRL19_ADDR(base) ((base) + (0x01AD))
#define PMIC_PERI_CTRL20_ADDR(base) ((base) + (0x01AE))
#define PMIC_PERI_CTRL21_ADDR(base) ((base) + (0x01AF))
#define PMIC_PERI_CTRL22_ADDR(base) ((base) + (0x01B0))
#define PMIC_PERI_CTRL23_ADDR(base) ((base) + (0x01B1))
#define PMIC_PERI_CTRL24_ADDR(base) ((base) + (0x01B2))
#define PMIC_PERI_CTRL25_ADDR(base) ((base) + (0x01B3))
#define PMIC_PERI_CTRL26_ADDR(base) ((base) + (0x01B4))
#define PMIC_WIFI_CTRL_ADDR(base) ((base) + (0x01B5))
#define PMIC_PERI_VSET_CTRL_ADDR(base) ((base) + (0x01B6))
#define PMIC_IDLE_CTRL0_ADDR(base) ((base) + (0x01B7))
#define PMIC_IDLE_CTRL1_ADDR(base) ((base) + (0x01B8))
#define PMIC_IDLE_CTRL2_ADDR(base) ((base) + (0x01B9))
#define PMIC_IDLE_CTRL3_ADDR(base) ((base) + (0x01BA))
#define PMIC_IDLE_CTRL4_ADDR(base) ((base) + (0x01BB))
#define PMIC_IDLE_CTRL5_ADDR(base) ((base) + (0x01BC))
#define PMIC_IDLE_CTRL6_ADDR(base) ((base) + (0x01BD))
#define PMIC_IDLE_CTRL7_ADDR(base) ((base) + (0x01BE))
#define PMIC_IDLE_CNT_CFG_ADDR(base) ((base) + (0x01BF))
#define PMIC_IDLE_CTRL8_ADDR(base) ((base) + (0x01C0))
#define PMIC_IDLE_CTRL9_ADDR(base) ((base) + (0x01C1))
#define PMIC_IDLE_CTRL10_ADDR(base) ((base) + (0x01C2))
#define PMIC_IDLE_CTRL11_ADDR(base) ((base) + (0x01C3))
#define PMIC_IDLE_CTRL12_ADDR(base) ((base) + (0x01C4))
#define PMIC_IDLE_CTRL13_ADDR(base) ((base) + (0x01C5))
#define PMIC_IDLE_CTRL14_ADDR(base) ((base) + (0x01C6))
#define PMIC_IDLE_CTRL15_ADDR(base) ((base) + (0x01C7))
#define PMIC_IDLE_CTRL16_ADDR(base) ((base) + (0x01C8))
#define PMIC_IDLE_CTRL17_ADDR(base) ((base) + (0x01C9))
#define PMIC_IDLE_CTRL18_ADDR(base) ((base) + (0x01CA))
#define PMIC_IDLE_CTRL19_ADDR(base) ((base) + (0x01CB))
#define PMIC_IDLE_CTRL20_ADDR(base) ((base) + (0x01CC))
#define PMIC_IDLE_CTRL21_ADDR(base) ((base) + (0x01CD))
#define PMIC_IDLE_CTRL22_ADDR(base) ((base) + (0x01CE))
#define PMIC_IDLE_CTRL23_ADDR(base) ((base) + (0x01CF))
#define PMIC_IDLE_CTRL24_ADDR(base) ((base) + (0x01D0))
#define PMIC_IDLE_CTRL25_ADDR(base) ((base) + (0x01D1))
#define PMIC_IDLE_CTRL26_ADDR(base) ((base) + (0x01D2))
#define PMIC_COUL_ECO_MASK_ADDR(base) ((base) + (0x01D3))
#define PMIC_PMU_SOFT_RST_ADDR(base) ((base) + (0x01D4))
#define PMIC_LOCK_ADDR(base) ((base) + (0x01D5))
#define PMIC_DR_EN_123_ADDR(base) ((base) + (0x01D6))
#define PMIC_DR_EN_4_ADDR(base) ((base) + (0x01D7))
#define PMIC_DR_EN_MODE_123_ADDR(base) ((base) + (0x01D8))
#define PMIC_FLASH_PERIOD_DR1_ADDR(base) ((base) + (0x01D9))
#define PMIC_FLASH_ON_DR1_ADDR(base) ((base) + (0x01DA))
#define PMIC_FLASH_PERIOD_DR2_ADDR(base) ((base) + (0x01DB))
#define PMIC_FLASH_ON_DR2_ADDR(base) ((base) + (0x01DC))
#define PMIC_FLASH_PERIOD_DR3_ADDR(base) ((base) + (0x01DD))
#define PMIC_FLASH_ON_DR3_ADDR(base) ((base) + (0x01DE))
#define PMIC_DR_MODE_SEL_ADDR(base) ((base) + (0x01DF))
#define PMIC_DR_BRE_CTRL_ADDR(base) ((base) + (0x01E0))
#define PMIC_DR1_ISET_ADDR(base) ((base) + (0x01E1))
#define PMIC_DR2_ISET_ADDR(base) ((base) + (0x01E2))
#define PMIC_DR3_ISET_ADDR(base) ((base) + (0x01E3))
#define PMIC_DR4_ISET_ADDR(base) ((base) + (0x01E4))
#define PMIC_DR_OUT_CTRL_ADDR(base) ((base) + (0x01E5))
#define PMIC_DR1_START_DEL_ADDR(base) ((base) + (0x01E6))
#define PMIC_DR2_START_DEL_ADDR(base) ((base) + (0x01E7))
#define PMIC_DR3_START_DEL_ADDR(base) ((base) + (0x01E8))
#define PMIC_DR1_TIM_CONF0_ADDR(base) ((base) + (0x01E9))
#define PMIC_DR1_TIM_CONF1_ADDR(base) ((base) + (0x01EA))
#define PMIC_DR2_TIM_CONF0_ADDR(base) ((base) + (0x01EB))
#define PMIC_DR2_TIM_CONF1_ADDR(base) ((base) + (0x01EC))
#define PMIC_DR3_TIM_CONF0_ADDR(base) ((base) + (0x01ED))
#define PMIC_DR3_TIM_CONF1_ADDR(base) ((base) + (0x01EE))
#define PMIC_DR4_CTRLRESERVE_ADDR(base) ((base) + (0x01EF))
#define PMIC_DR_CTRLRESERVE8_ADDR(base) ((base) + (0x01F0))
#define PMIC_DR_CTRLRESERVE9_ADDR(base) ((base) + (0x01F1))
#define PMIC_OTP0_0_ADDR(base) ((base) + (0x01F2))
#define PMIC_OTP0_1_ADDR(base) ((base) + (0x01F3))
#define PMIC_OTP0_CTRL0_ADDR(base) ((base) + (0x01F4))
#define PMIC_OTP0_CTRL1_ADDR(base) ((base) + (0x01F5))
#define PMIC_OTP0_CTRL2_ADDR(base) ((base) + (0x01F6))
#define PMIC_OTP0_WDATA_ADDR(base) ((base) + (0x01F7))
#define PMIC_OTP0_0_W_ADDR(base) ((base) + (0x01F8))
#define PMIC_OTP0_1_W_ADDR(base) ((base) + (0x01F9))
#define PMIC_OTP0_2_W_ADDR(base) ((base) + (0x01FA))
#define PMIC_OTP0_3_W_ADDR(base) ((base) + (0x01FB))
#define PMIC_OTP0_4_W_ADDR(base) ((base) + (0x01FC))
#define PMIC_OTP0_5_W_ADDR(base) ((base) + (0x01FD))
#define PMIC_OTP0_6_W_ADDR(base) ((base) + (0x01FE))
#define PMIC_OTP0_7_W_ADDR(base) ((base) + (0x01FF))
#define PMIC_OTP0_8_W_ADDR(base) ((base) + (0x0200))
#define PMIC_OTP0_9_W_ADDR(base) ((base) + (0x0201))
#define PMIC_OTP0_10_W_ADDR(base) ((base) + (0x0202))
#define PMIC_OTP0_11_W_ADDR(base) ((base) + (0x0203))
#define PMIC_OTP0_12_W_ADDR(base) ((base) + (0x0204))
#define PMIC_OTP0_13_W_ADDR(base) ((base) + (0x0205))
#define PMIC_OTP0_14_W_ADDR(base) ((base) + (0x0206))
#define PMIC_OTP0_15_W_ADDR(base) ((base) + (0x0207))
#define PMIC_OTP0_16_W_ADDR(base) ((base) + (0x0208))
#define PMIC_OTP0_17_W_ADDR(base) ((base) + (0x0209))
#define PMIC_OTP0_18_W_ADDR(base) ((base) + (0x020A))
#define PMIC_OTP0_19_W_ADDR(base) ((base) + (0x020B))
#define PMIC_OTP0_20_W_ADDR(base) ((base) + (0x020C))
#define PMIC_OTP0_21_W_ADDR(base) ((base) + (0x020D))
#define PMIC_OTP0_22_W_ADDR(base) ((base) + (0x020E))
#define PMIC_OTP0_23_W_ADDR(base) ((base) + (0x020F))
#define PMIC_OTP0_24_W_ADDR(base) ((base) + (0x0210))
#define PMIC_OTP0_25_W_ADDR(base) ((base) + (0x0211))
#define PMIC_OTP0_26_W_ADDR(base) ((base) + (0x0212))
#define PMIC_OTP0_27_W_ADDR(base) ((base) + (0x0213))
#define PMIC_OTP0_28_W_ADDR(base) ((base) + (0x0214))
#define PMIC_OTP0_29_W_ADDR(base) ((base) + (0x0215))
#define PMIC_OTP0_30_W_ADDR(base) ((base) + (0x0216))
#define PMIC_OTP0_31_W_ADDR(base) ((base) + (0x0217))
#define PMIC_OTP0_32_W_ADDR(base) ((base) + (0x0218))
#define PMIC_OTP0_33_W_ADDR(base) ((base) + (0x0219))
#define PMIC_OTP0_34_W_ADDR(base) ((base) + (0x021A))
#define PMIC_OTP0_35_W_ADDR(base) ((base) + (0x021B))
#define PMIC_OTP0_36_W_ADDR(base) ((base) + (0x021C))
#define PMIC_OTP0_37_W_ADDR(base) ((base) + (0x021D))
#define PMIC_OTP0_38_W_ADDR(base) ((base) + (0x021E))
#define PMIC_OTP0_39_W_ADDR(base) ((base) + (0x021F))
#define PMIC_OTP0_40_W_ADDR(base) ((base) + (0x0220))
#define PMIC_OTP0_41_W_ADDR(base) ((base) + (0x0221))
#define PMIC_OTP0_42_W_ADDR(base) ((base) + (0x0222))
#define PMIC_OTP0_43_W_ADDR(base) ((base) + (0x0223))
#define PMIC_OTP0_44_W_ADDR(base) ((base) + (0x0224))
#define PMIC_OTP0_45_W_ADDR(base) ((base) + (0x0225))
#define PMIC_OTP0_46_W_ADDR(base) ((base) + (0x0226))
#define PMIC_OTP0_47_W_ADDR(base) ((base) + (0x0227))
#define PMIC_OTP0_48_W_ADDR(base) ((base) + (0x0228))
#define PMIC_OTP0_49_W_ADDR(base) ((base) + (0x0229))
#define PMIC_OTP0_50_W_ADDR(base) ((base) + (0x022A))
#define PMIC_OTP0_51_W_ADDR(base) ((base) + (0x022B))
#define PMIC_OTP0_52_W_ADDR(base) ((base) + (0x022C))
#define PMIC_OTP0_53_W_ADDR(base) ((base) + (0x022D))
#define PMIC_OTP0_54_W_ADDR(base) ((base) + (0x022E))
#define PMIC_OTP0_55_W_ADDR(base) ((base) + (0x022F))
#define PMIC_OTP0_56_W_ADDR(base) ((base) + (0x0230))
#define PMIC_OTP0_57_W_ADDR(base) ((base) + (0x0231))
#define PMIC_OTP0_58_W_ADDR(base) ((base) + (0x0232))
#define PMIC_OTP0_59_W_ADDR(base) ((base) + (0x0233))
#define PMIC_OTP0_60_W_ADDR(base) ((base) + (0x0234))
#define PMIC_OTP0_61_W_ADDR(base) ((base) + (0x0235))
#define PMIC_OTP0_62_W_ADDR(base) ((base) + (0x0236))
#define PMIC_OTP0_63_W_ADDR(base) ((base) + (0x0237))
#define PMIC_OTP1_0_ADDR(base) ((base) + (0x0238))
#define PMIC_OTP1_1_ADDR(base) ((base) + (0x0239))
#define PMIC_OTP1_CTRL0_ADDR(base) ((base) + (0x023A))
#define PMIC_OTP1_CTRL1_ADDR(base) ((base) + (0x023B))
#define PMIC_OTP1_CTRL2_ADDR(base) ((base) + (0x023C))
#define PMIC_OTP1_WDATA_ADDR(base) ((base) + (0x023D))
#define PMIC_OTP1_0_W_ADDR(base) ((base) + (0x023E))
#define PMIC_OTP1_1_W_ADDR(base) ((base) + (0x023F))
#define PMIC_OTP1_2_W_ADDR(base) ((base) + (0x0240))
#define PMIC_D2A_RES0_ADDR(base) ((base) + (0x0241))
#define PMIC_D2A_RES1_ADDR(base) ((base) + (0x0242))
#define PMIC_A2D_RES0_ADDR(base) ((base) + (0x0243))
#define PMIC_SIM_CTRL0_ADDR(base) ((base) + (0x0244))
#define PMIC_SIM_CTRL1_ADDR(base) ((base) + (0x0245))
#define PMIC_SIM_DEB_CTRL1_ADDR(base) ((base) + (0x0246))
#define PMIC_SIM_DEB_CTRL2_ADDR(base) ((base) + (0x0247))
#define PMIC_GPIO0DATA_ADDR(base) ((base) + (0x0248))
#define PMIC_GPIO0_DATA_RESERVE_ADDR(base) ((base) + (0x0249))
#define PMIC_GPIO0DIR_ADDR(base) ((base) + (0x024A))
#define PMIC_GPIO0_DIR_RESERVE_ADDR(base) ((base) + (0x024B))
#define PMIC_GPIO0IS_ADDR(base) ((base) + (0x024C))
#define PMIC_GPIO0IBE_ADDR(base) ((base) + (0x024D))
#define PMIC_GPIO0IEV_ADDR(base) ((base) + (0x024E))
#define PMIC_GPIO0AFSEL_ADDR(base) ((base) + (0x024F))
#define PMIC_GPIO0DSSEL_ADDR(base) ((base) + (0x0250))
#define PMIC_GPIO0PUSEL_ADDR(base) ((base) + (0x0251))
#define PMIC_GPIO0PDSEL_ADDR(base) ((base) + (0x0252))
#define PMIC_GPIO0DEBSEL_ADDR(base) ((base) + (0x0253))
#define PMIC_SIM0PUSEL_ADDR(base) ((base) + (0x0254))
#define PMIC_SIM0PDSEL_ADDR(base) ((base) + (0x0255))
#define PMIC_GPIO1DATA_ADDR(base) ((base) + (0x0256))
#define PMIC_GPIO1_DATA_RESERVE_ADDR(base) ((base) + (0x0257))
#define PMIC_GPIO1DIR_ADDR(base) ((base) + (0x0258))
#define PMIC_GPIO1_DIR_RESERVE_ADDR(base) ((base) + (0x0259))
#define PMIC_GPIO1IS_ADDR(base) ((base) + (0x025A))
#define PMIC_GPIO1IBE_ADDR(base) ((base) + (0x025B))
#define PMIC_GPIO1IEV_ADDR(base) ((base) + (0x025C))
#define PMIC_GPIO1AFSEL_ADDR(base) ((base) + (0x025D))
#define PMIC_GPIO1DSSEL_ADDR(base) ((base) + (0x025E))
#define PMIC_GPIO1PUSEL_ADDR(base) ((base) + (0x025F))
#define PMIC_GPIO1PDSEL_ADDR(base) ((base) + (0x0260))
#define PMIC_GPIO1DEBSEL_ADDR(base) ((base) + (0x0261))
#define PMIC_SIM1PUSEL_ADDR(base) ((base) + (0x0262))
#define PMIC_SIM1PDSEL_ADDR(base) ((base) + (0x0263))
#define PMIC_GPIO2DATA_ADDR(base) ((base) + (0x0264))
#define PMIC_GPIO2_DATA_RESERVE_ADDR(base) ((base) + (0x0265))
#define PMIC_GPIO2DIR_ADDR(base) ((base) + (0x0266))
#define PMIC_GPIO2_DIR_RESERVE_ADDR(base) ((base) + (0x0267))
#define PMIC_GPIO2IS_ADDR(base) ((base) + (0x0268))
#define PMIC_GPIO2IBE_ADDR(base) ((base) + (0x0269))
#define PMIC_GPIO2IEV_ADDR(base) ((base) + (0x026A))
#define PMIC_GPIO2DSSEL_ADDR(base) ((base) + (0x026B))
#define PMIC_GPIO2PUSEL_ADDR(base) ((base) + (0x026C))
#define PMIC_GPIO2PDSEL_ADDR(base) ((base) + (0x026D))
#define PMIC_GPIO2DEBSEL_ADDR(base) ((base) + (0x026E))
#define PMIC_GPIO3DATA_ADDR(base) ((base) + (0x026F))
#define PMIC_GPIO3_DATA_RESERVE_ADDR(base) ((base) + (0x0270))
#define PMIC_GPIO3DIR_ADDR(base) ((base) + (0x0271))
#define PMIC_GPIO3_DIR_RESERVE_ADDR(base) ((base) + (0x0272))
#define PMIC_GPIO3IS_ADDR(base) ((base) + (0x0273))
#define PMIC_GPIO3IBE_ADDR(base) ((base) + (0x0274))
#define PMIC_GPIO3IEV_ADDR(base) ((base) + (0x0275))
#define PMIC_GPIO3DSSEL_ADDR(base) ((base) + (0x0276))
#define PMIC_GPIO3PUSEL_ADDR(base) ((base) + (0x0277))
#define PMIC_GPIO3PDSEL_ADDR(base) ((base) + (0x0278))
#define PMIC_GPIO3DEBSEL_ADDR(base) ((base) + (0x0279))
#define PMIC_AUX_IBIAS_CFG_ADDR(base) ((base) + (0x027A))
#define PMIC_DAC_DIN_MSB_ADDR(base) ((base) + (0x027B))
#define PMIC_DAC_DIN_LSB_ADDR(base) ((base) + (0x027C))
#define PMIC_RAMP_BUCK00_CTRL0_ADDR(base) ((base) + (0x027D))
#define PMIC_RAMP_BUCK00_CTRL1_ADDR(base) ((base) + (0x027E))
#define PMIC_BUCK00_RAMPUP_STATE_ADDR(base) ((base) + (0x027F))
#define PMIC_BUCK00_RAMPDOWN_STATE_ADDR(base) ((base) + (0x0280))
#define PMIC_BUCK00_RAMP_CHG_CTRL_ADDR(base) ((base) + (0x0281))
#define PMIC_RAMP_BUCK02_CTRL0_ADDR(base) ((base) + (0x0282))
#define PMIC_RAMP_BUCK02_CTRL1_ADDR(base) ((base) + (0x0283))
#define PMIC_BUCK02_RAMPUP_STATE_ADDR(base) ((base) + (0x0284))
#define PMIC_BUCK02_RAMPDOWN_STATE_ADDR(base) ((base) + (0x0285))
#define PMIC_BUCK02_RAMP_CHG_CTRL_ADDR(base) ((base) + (0x0286))
#define PMIC_RAMP_BUCK4_CTRL0_ADDR(base) ((base) + (0x0287))
#define PMIC_RAMP_BUCK4_CTRL1_ADDR(base) ((base) + (0x0288))
#define PMIC_BUCK4_RAMPUP_STATE_ADDR(base) ((base) + (0x0289))
#define PMIC_BUCK4_RAMPDOWN_STATE_ADDR(base) ((base) + (0x028A))
#define PMIC_BUCK4_RAMP_CHG_CTRL_ADDR(base) ((base) + (0x028B))
#define PMIC_RAMP_BUCK5_CTRL0_ADDR(base) ((base) + (0x028C))
#define PMIC_RAMP_BUCK5_CTRL1_ADDR(base) ((base) + (0x028D))
#define PMIC_BUCK5_RAMPUP_STATE_ADDR(base) ((base) + (0x028E))
#define PMIC_BUCK5_RAMPDOWN_STATE_ADDR(base) ((base) + (0x028F))
#define PMIC_BUCK5_RAMP_CHG_CTRL_ADDR(base) ((base) + (0x0290))
#define PMIC_RAMP_BUCK6_CTRL0_ADDR(base) ((base) + (0x0291))
#define PMIC_RAMP_BUCK6_CTRL1_ADDR(base) ((base) + (0x0292))
#define PMIC_BUCK6_RAMPUP_STATE_ADDR(base) ((base) + (0x0293))
#define PMIC_BUCK6_RAMPDOWN_STATE_ADDR(base) ((base) + (0x0294))
#define PMIC_BUCK6_RAMP_CHG_CTRL_ADDR(base) ((base) + (0x0295))
#define PMIC_RAMP_LDO0_CTRL0_ADDR(base) ((base) + (0x0296))
#define PMIC_RAMP_LDO0_CTRL1_ADDR(base) ((base) + (0x0297))
#define PMIC_RAMP_LDO39_CTRL0_ADDR(base) ((base) + (0x0298))
#define PMIC_RAMP_LDO39_CTRL1_ADDR(base) ((base) + (0x0299))
#define PMIC_RAMP_LDO40_CTRL0_ADDR(base) ((base) + (0x029A))
#define PMIC_RAMP_LDO40_CTRL1_ADDR(base) ((base) + (0x029B))
#define PMIC_SPMI_ECO_GT_BYPASS_ADDR(base) ((base) + (0x029C))
#define PMIC_PWRONN_8S_XOADC_DEBUG_CTRL_ADDR(base) ((base) + (0x029D))
#define PMIC_DIS_19M2_CTRL_ADDR(base) ((base) + (0x029E))
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
#define PMIC_IRQ_MASK_9_ADDR(base) ((base) + (0x02ABUL))
#define PMIC_IRQ_MASK_10_ADDR(base) ((base) + (0x02ACUL))
#define PMIC_IRQ_MASK_11_ADDR(base) ((base) + (0x02ADUL))
#define PMIC_IRQ_MASK_12_ADDR(base) ((base) + (0x02AEUL))
#define PMIC_IRQ_MASK_13_ADDR(base) ((base) + (0x02AFUL))
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
#define PMIC_IRQ_MASK_9_ADDR(base) ((base) + (0x02AB))
#define PMIC_IRQ_MASK_10_ADDR(base) ((base) + (0x02AC))
#define PMIC_IRQ_MASK_11_ADDR(base) ((base) + (0x02AD))
#define PMIC_IRQ_MASK_12_ADDR(base) ((base) + (0x02AE))
#define PMIC_IRQ_MASK_13_ADDR(base) ((base) + (0x02AF))
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
#define PMIC_SCP_IRQ1_ADDR(base) ((base) + (0x02BCUL))
#define PMIC_GPIO_IRQ_ADDR(base) ((base) + (0x02BDUL))
#define PMIC_ACR_SOH_IRQ_ADDR(base) ((base) + (0x02BEUL))
#define PMIC_LRA_IRQ_ADDR(base) ((base) + (0x02BFUL))
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
#define PMIC_SCP_IRQ1_ADDR(base) ((base) + (0x02BC))
#define PMIC_GPIO_IRQ_ADDR(base) ((base) + (0x02BD))
#define PMIC_ACR_SOH_IRQ_ADDR(base) ((base) + (0x02BE))
#define PMIC_LRA_IRQ_ADDR(base) ((base) + (0x02BF))
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
#define PMIC_NP_SCP1_ADDR(base) ((base) + (0x02CBUL))
#define PMIC_NP_RECORD0_ADDR(base) ((base) + (0x02CCUL))
#define PMIC_NP_RECORD1_ADDR(base) ((base) + (0x02CDUL))
#define PMIC_NP_RECORD2_ADDR(base) ((base) + (0x02CEUL))
#define PMIC_NP_RECORD3_ADDR(base) ((base) + (0x02CFUL))
#define PMIC_NP_RECORD4_ADDR(base) ((base) + (0x02D0UL))
#define PMIC_NP_RECORD5_ADDR(base) ((base) + (0x02D1UL))
#define PMIC_RAMP_EVENT_ADDR(base) ((base) + (0x02D2UL))
#define PMIC_SOH_RECORD_ADDR(base) ((base) + (0x02D3UL))
#define PMIC_ACR_SOH_RECORD_ADDR(base) ((base) + (0x02D4UL))
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
#define PMIC_NP_SCP1_ADDR(base) ((base) + (0x02CB))
#define PMIC_NP_RECORD0_ADDR(base) ((base) + (0x02CC))
#define PMIC_NP_RECORD1_ADDR(base) ((base) + (0x02CD))
#define PMIC_NP_RECORD2_ADDR(base) ((base) + (0x02CE))
#define PMIC_NP_RECORD3_ADDR(base) ((base) + (0x02CF))
#define PMIC_NP_RECORD4_ADDR(base) ((base) + (0x02D0))
#define PMIC_NP_RECORD5_ADDR(base) ((base) + (0x02D1))
#define PMIC_RAMP_EVENT_ADDR(base) ((base) + (0x02D2))
#define PMIC_SOH_RECORD_ADDR(base) ((base) + (0x02D3))
#define PMIC_ACR_SOH_RECORD_ADDR(base) ((base) + (0x02D4))
#define PMIC_COUL_RECORD0_ADDR(base) ((base) + (0x02D5))
#define PMIC_COUL_RECORD1_ADDR(base) ((base) + (0x02D6))
#define PMIC_NP_VSYS_DROP_RECORD_ADDR(base) ((base) + (0x02D7))
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_CLK_ABB_CTRL1_ADDR(base) ((base) + (0x02DAUL))
#define PMIC_CLK_WIFI_CTRL1_ADDR(base) ((base) + (0x02DBUL))
#define PMIC_CLK_NFC_CTRL1_ADDR(base) ((base) + (0x02DCUL))
#define PMIC_CLK_RF0_CTRL1_ADDR(base) ((base) + (0x02DDUL))
#define PMIC_CLK_RF1_CTRL1_ADDR(base) ((base) + (0x02DEUL))
#define PMIC_CLK_TOP_CTRL1_0_ADDR(base) ((base) + (0x02DFUL))
#define PMIC_CLK_TOP_CTRL1_1_ADDR(base) ((base) + (0x02E0UL))
#define PMIC_CLK_256K_CTRL0_ADDR(base) ((base) + (0x02E1UL))
#define PMIC_CLK_256K_CTRL1_ADDR(base) ((base) + (0x02E2UL))
#define PMIC_CLK_UFS_FRE_CTRL1_ADDR(base) ((base) + (0x02E3UL))
#define PMIC_VSYS_LOW_SET_ADDR(base) ((base) + (0x02E4UL))
#define PMIC_VSYS_DROP_SET_ADDR(base) ((base) + (0x02E5UL))
#define PMIC_NFC_ON_CTRL_ADDR(base) ((base) + (0x02E6UL))
#define PMIC_HRESET_PWRDOWN_CTRL_ADDR(base) ((base) + (0x02E7UL))
#define PMIC_SMPL_CTRL_ADDR(base) ((base) + (0x02E8UL))
#define PMIC_SYS_CTRL0_ADDR(base) ((base) + (0x02E9UL))
#define PMIC_SYS_CTRL1_ADDR(base) ((base) + (0x02EAUL))
#define PMIC_DEBUG_LOCK_ADDR(base) ((base) + (0x02EBUL))
#define PMIC_SYS_DEBUG0_ADDR(base) ((base) + (0x02ECUL))
#define PMIC_SYS_DEBUG1_ADDR(base) ((base) + (0x02EDUL))
#define PMIC_SYS_DEBUG2_ADDR(base) ((base) + (0x02EEUL))
#define PMIC_SYS_DEBUG3_ADDR(base) ((base) + (0x02EFUL))
#define PMIC_SYS_DEBUG4_ADDR(base) ((base) + (0x02F0UL))
#define PMIC_SYS_DEBUG5_ADDR(base) ((base) + (0x02F1UL))
#define PMIC_BACKUP_CHG_ADDR(base) ((base) + (0x02F2UL))
#define PMIC_NP_CTRL_ADDR(base) ((base) + (0x02F3UL))
#define PMIC_NP_D2A_RES0_ADDR(base) ((base) + (0x02F4UL))
#define PMIC_NP_D2A_RES1_ADDR(base) ((base) + (0x02F5UL))
#define PMIC_HRST_REG0_ADDR(base) ((base) + (0x02F6UL))
#define PMIC_HRST_REG1_ADDR(base) ((base) + (0x02F7UL))
#define PMIC_HRST_REG2_ADDR(base) ((base) + (0x02F8UL))
#define PMIC_HRST_REG3_ADDR(base) ((base) + (0x02F9UL))
#define PMIC_HRST_REG4_ADDR(base) ((base) + (0x02FAUL))
#define PMIC_HRST_REG5_ADDR(base) ((base) + (0x02FBUL))
#define PMIC_HRST_REG6_ADDR(base) ((base) + (0x02FCUL))
#define PMIC_HRST_REG7_ADDR(base) ((base) + (0x02FDUL))
#define PMIC_HRST_REG8_ADDR(base) ((base) + (0x02FEUL))
#define PMIC_HRST_REG9_ADDR(base) ((base) + (0x02FFUL))
#define PMIC_HRST_REG10_ADDR(base) ((base) + (0x0300UL))
#define PMIC_HRST_REG11_ADDR(base) ((base) + (0x0301UL))
#define PMIC_HRST_REG12_ADDR(base) ((base) + (0x0302UL))
#define PMIC_HRST_REG13_ADDR(base) ((base) + (0x0303UL))
#define PMIC_HRST_REG14_ADDR(base) ((base) + (0x0304UL))
#define PMIC_HRST_REG15_ADDR(base) ((base) + (0x0305UL))
#define PMIC_HRST_REG16_ADDR(base) ((base) + (0x0306UL))
#define PMIC_HRST_REG17_ADDR(base) ((base) + (0x0307UL))
#define PMIC_HRST_REG18_ADDR(base) ((base) + (0x0308UL))
#define PMIC_HRST_REG19_ADDR(base) ((base) + (0x0309UL))
#define PMIC_HRST_REG20_ADDR(base) ((base) + (0x030AUL))
#define PMIC_HRST_REG21_ADDR(base) ((base) + (0x030BUL))
#define PMIC_HRST_REG22_ADDR(base) ((base) + (0x030CUL))
#define PMIC_HRST_REG23_ADDR(base) ((base) + (0x030DUL))
#define PMIC_HRST_REG24_ADDR(base) ((base) + (0x030EUL))
#define PMIC_HRST_REG25_ADDR(base) ((base) + (0x030FUL))
#define PMIC_HRST_REG26_ADDR(base) ((base) + (0x0310UL))
#define PMIC_HRST_REG27_ADDR(base) ((base) + (0x0311UL))
#define PMIC_HRST_REG28_ADDR(base) ((base) + (0x0312UL))
#define PMIC_HRST_REG29_ADDR(base) ((base) + (0x0313UL))
#define PMIC_HRST_REG30_ADDR(base) ((base) + (0x0314UL))
#define PMIC_HRST_REG31_ADDR(base) ((base) + (0x0315UL))
#define PMIC_OTP0_0_R_ADDR(base) ((base) + (0x0316UL))
#define PMIC_OTP0_1_R_ADDR(base) ((base) + (0x0317UL))
#define PMIC_OTP0_2_R_ADDR(base) ((base) + (0x0318UL))
#define PMIC_OTP0_3_R_ADDR(base) ((base) + (0x0319UL))
#define PMIC_OTP0_4_R_ADDR(base) ((base) + (0x031AUL))
#define PMIC_OTP0_5_R_ADDR(base) ((base) + (0x031BUL))
#define PMIC_OTP0_6_R_ADDR(base) ((base) + (0x031CUL))
#define PMIC_OTP0_7_R_ADDR(base) ((base) + (0x031DUL))
#define PMIC_OTP0_8_R_ADDR(base) ((base) + (0x031EUL))
#define PMIC_OTP0_9_R_ADDR(base) ((base) + (0x031FUL))
#define PMIC_OTP0_10_R_ADDR(base) ((base) + (0x0320UL))
#define PMIC_OTP0_11_R_ADDR(base) ((base) + (0x0321UL))
#define PMIC_OTP0_12_R_ADDR(base) ((base) + (0x0322UL))
#define PMIC_OTP0_13_R_ADDR(base) ((base) + (0x0323UL))
#define PMIC_OTP0_14_R_ADDR(base) ((base) + (0x0324UL))
#define PMIC_OTP0_15_R_ADDR(base) ((base) + (0x0325UL))
#define PMIC_OTP0_16_R_ADDR(base) ((base) + (0x0326UL))
#define PMIC_OTP0_17_R_ADDR(base) ((base) + (0x0327UL))
#define PMIC_OTP0_18_R_ADDR(base) ((base) + (0x0328UL))
#define PMIC_OTP0_19_R_ADDR(base) ((base) + (0x0329UL))
#define PMIC_OTP0_20_R_ADDR(base) ((base) + (0x032AUL))
#define PMIC_OTP0_21_R_ADDR(base) ((base) + (0x032BUL))
#define PMIC_OTP0_22_R_ADDR(base) ((base) + (0x032CUL))
#define PMIC_OTP0_23_R_ADDR(base) ((base) + (0x032DUL))
#define PMIC_OTP0_24_R_ADDR(base) ((base) + (0x032EUL))
#define PMIC_OTP0_25_R_ADDR(base) ((base) + (0x032FUL))
#define PMIC_OTP0_26_R_ADDR(base) ((base) + (0x0330UL))
#define PMIC_OTP0_27_R_ADDR(base) ((base) + (0x0331UL))
#define PMIC_OTP0_28_R_ADDR(base) ((base) + (0x0332UL))
#define PMIC_OTP0_29_R_ADDR(base) ((base) + (0x0333UL))
#define PMIC_OTP0_30_R_ADDR(base) ((base) + (0x0334UL))
#define PMIC_OTP0_31_R_ADDR(base) ((base) + (0x0335UL))
#define PMIC_OTP0_32_R_ADDR(base) ((base) + (0x0336UL))
#define PMIC_OTP0_33_R_ADDR(base) ((base) + (0x0337UL))
#define PMIC_OTP0_34_R_ADDR(base) ((base) + (0x0338UL))
#define PMIC_OTP0_35_R_ADDR(base) ((base) + (0x0339UL))
#define PMIC_OTP0_36_R_ADDR(base) ((base) + (0x033AUL))
#define PMIC_OTP0_37_R_ADDR(base) ((base) + (0x033BUL))
#define PMIC_OTP0_38_R_ADDR(base) ((base) + (0x033CUL))
#define PMIC_OTP0_39_R_ADDR(base) ((base) + (0x033DUL))
#define PMIC_OTP0_40_R_ADDR(base) ((base) + (0x033EUL))
#define PMIC_OTP0_41_R_ADDR(base) ((base) + (0x033FUL))
#define PMIC_OTP0_42_R_ADDR(base) ((base) + (0x0340UL))
#define PMIC_OTP0_43_R_ADDR(base) ((base) + (0x0341UL))
#define PMIC_OTP0_44_R_ADDR(base) ((base) + (0x0342UL))
#define PMIC_OTP0_45_R_ADDR(base) ((base) + (0x0343UL))
#define PMIC_OTP0_46_R_ADDR(base) ((base) + (0x0344UL))
#define PMIC_OTP0_47_R_ADDR(base) ((base) + (0x0345UL))
#define PMIC_OTP0_48_R_ADDR(base) ((base) + (0x0346UL))
#define PMIC_OTP0_49_R_ADDR(base) ((base) + (0x0347UL))
#define PMIC_OTP0_50_R_ADDR(base) ((base) + (0x0348UL))
#define PMIC_OTP0_51_R_ADDR(base) ((base) + (0x0349UL))
#define PMIC_OTP0_52_R_ADDR(base) ((base) + (0x034AUL))
#define PMIC_OTP0_53_R_ADDR(base) ((base) + (0x034BUL))
#define PMIC_OTP0_54_R_ADDR(base) ((base) + (0x034CUL))
#define PMIC_OTP0_55_R_ADDR(base) ((base) + (0x034DUL))
#define PMIC_OTP0_56_R_ADDR(base) ((base) + (0x034EUL))
#define PMIC_OTP0_57_R_ADDR(base) ((base) + (0x034FUL))
#define PMIC_OTP0_58_R_ADDR(base) ((base) + (0x0350UL))
#define PMIC_OTP0_59_R_ADDR(base) ((base) + (0x0351UL))
#define PMIC_OTP0_60_R_ADDR(base) ((base) + (0x0352UL))
#define PMIC_OTP0_61_R_ADDR(base) ((base) + (0x0353UL))
#define PMIC_OTP0_62_R_ADDR(base) ((base) + (0x0354UL))
#define PMIC_OTP0_63_R_ADDR(base) ((base) + (0x0355UL))
#define PMIC_OTP1_0_R_ADDR(base) ((base) + (0x0356UL))
#define PMIC_OTP1_1_R_ADDR(base) ((base) + (0x0357UL))
#define PMIC_OTP1_2_R_ADDR(base) ((base) + (0x0358UL))
#define PMIC_OTP1_3_R_ADDR(base) ((base) + (0x0359UL))
#define PMIC_OTP1_4_R_ADDR(base) ((base) + (0x035AUL))
#define PMIC_OTP1_5_R_ADDR(base) ((base) + (0x035BUL))
#define PMIC_OTP1_6_R_ADDR(base) ((base) + (0x035CUL))
#define PMIC_OTP1_7_R_ADDR(base) ((base) + (0x035DUL))
#define PMIC_OTP1_8_R_ADDR(base) ((base) + (0x035EUL))
#define PMIC_OTP1_9_R_ADDR(base) ((base) + (0x035FUL))
#define PMIC_OTP1_10_R_ADDR(base) ((base) + (0x0360UL))
#define PMIC_OTP1_11_R_ADDR(base) ((base) + (0x0361UL))
#define PMIC_OTP1_12_R_ADDR(base) ((base) + (0x0362UL))
#define PMIC_OTP1_13_R_ADDR(base) ((base) + (0x0363UL))
#define PMIC_OTP1_14_R_ADDR(base) ((base) + (0x0364UL))
#define PMIC_OTP1_15_R_ADDR(base) ((base) + (0x0365UL))
#define PMIC_OTP1_16_R_ADDR(base) ((base) + (0x0366UL))
#define PMIC_OTP1_17_R_ADDR(base) ((base) + (0x0367UL))
#define PMIC_OTP1_18_R_ADDR(base) ((base) + (0x6028UL))
#define PMIC_OTP1_19_R_ADDR(base) ((base) + (0x6029UL))
#define PMIC_OTP1_20_R_ADDR(base) ((base) + (0x602AUL))
#define PMIC_OTP1_21_R_ADDR(base) ((base) + (0x602BUL))
#define PMIC_OTP1_22_R_ADDR(base) ((base) + (0x602CUL))
#define PMIC_OTP1_23_R_ADDR(base) ((base) + (0x602DUL))
#define PMIC_OTP1_24_R_ADDR(base) ((base) + (0x602EUL))
#define PMIC_OTP1_25_R_ADDR(base) ((base) + (0x602FUL))
#define PMIC_OTP1_26_R_ADDR(base) ((base) + (0x6030UL))
#define PMIC_OTP1_27_R_ADDR(base) ((base) + (0x6031UL))
#define PMIC_OTP1_28_R_ADDR(base) ((base) + (0x0372UL))
#define PMIC_OTP1_29_R_ADDR(base) ((base) + (0x0373UL))
#define PMIC_OTP1_30_R_ADDR(base) ((base) + (0x0374UL))
#define PMIC_OTP1_31_R_ADDR(base) ((base) + (0x0375UL))
#define PMIC_OTP1_32_R_ADDR(base) ((base) + (0x0376UL))
#define PMIC_OTP1_33_R_ADDR(base) ((base) + (0x0377UL))
#define PMIC_OTP1_34_R_ADDR(base) ((base) + (0x0378UL))
#define PMIC_OTP1_35_R_ADDR(base) ((base) + (0x0379UL))
#define PMIC_OTP1_36_R_ADDR(base) ((base) + (0x037AUL))
#define PMIC_OTP1_37_R_ADDR(base) ((base) + (0x037BUL))
#define PMIC_OTP1_38_R_ADDR(base) ((base) + (0x037CUL))
#define PMIC_OTP1_39_R_ADDR(base) ((base) + (0x037DUL))
#define PMIC_OTP1_40_R_ADDR(base) ((base) + (0x037EUL))
#define PMIC_OTP1_41_R_ADDR(base) ((base) + (0x037FUL))
#define PMIC_OTP1_42_R_ADDR(base) ((base) + (0x0380UL))
#define PMIC_OTP1_43_R_ADDR(base) ((base) + (0x0381UL))
#define PMIC_OTP1_44_R_ADDR(base) ((base) + (0x0382UL))
#define PMIC_OTP1_45_R_ADDR(base) ((base) + (0x0383UL))
#define PMIC_OTP1_46_R_ADDR(base) ((base) + (0x0384UL))
#define PMIC_OTP1_47_R_ADDR(base) ((base) + (0x0385UL))
#define PMIC_OTP1_48_R_ADDR(base) ((base) + (0x0386UL))
#define PMIC_OTP1_49_R_ADDR(base) ((base) + (0x0387UL))
#define PMIC_OTP1_50_R_ADDR(base) ((base) + (0x0388UL))
#define PMIC_OTP1_51_R_ADDR(base) ((base) + (0x0389UL))
#define PMIC_OTP1_52_R_ADDR(base) ((base) + (0x038AUL))
#define PMIC_OTP1_53_R_ADDR(base) ((base) + (0x038BUL))
#define PMIC_OTP1_54_R_ADDR(base) ((base) + (0x038CUL))
#define PMIC_OTP1_55_R_ADDR(base) ((base) + (0x038DUL))
#define PMIC_OTP1_56_R_ADDR(base) ((base) + (0x038EUL))
#define PMIC_OTP1_57_R_ADDR(base) ((base) + (0x038FUL))
#define PMIC_OTP1_58_R_ADDR(base) ((base) + (0x0390UL))
#define PMIC_OTP1_59_R_ADDR(base) ((base) + (0x0391UL))
#define PMIC_OTP1_60_R_ADDR(base) ((base) + (0x0392UL))
#define PMIC_OTP1_61_R_ADDR(base) ((base) + (0x0393UL))
#define PMIC_OTP1_62_R_ADDR(base) ((base) + (0x0394UL))
#define PMIC_OTP1_63_R_ADDR(base) ((base) + (0x0395UL))
#define PMIC_EN_PMUA_PMUD_ADDR(base) ((base) + (0x0396UL))
#else
#define PMIC_CLK_ABB_CTRL1_ADDR(base) ((base) + (0x02DA))
#define PMIC_CLK_WIFI_CTRL1_ADDR(base) ((base) + (0x02DB))
#define PMIC_CLK_NFC_CTRL1_ADDR(base) ((base) + (0x02DC))
#define PMIC_CLK_RF0_CTRL1_ADDR(base) ((base) + (0x02DD))
#define PMIC_CLK_RF1_CTRL1_ADDR(base) ((base) + (0x02DE))
#define PMIC_CLK_TOP_CTRL1_0_ADDR(base) ((base) + (0x02DF))
#define PMIC_CLK_TOP_CTRL1_1_ADDR(base) ((base) + (0x02E0))
#define PMIC_CLK_256K_CTRL0_ADDR(base) ((base) + (0x02E1))
#define PMIC_CLK_256K_CTRL1_ADDR(base) ((base) + (0x02E2))
#define PMIC_CLK_UFS_FRE_CTRL1_ADDR(base) ((base) + (0x02E3))
#define PMIC_VSYS_LOW_SET_ADDR(base) ((base) + (0x02E4))
#define PMIC_VSYS_DROP_SET_ADDR(base) ((base) + (0x02E5))
#define PMIC_NFC_ON_CTRL_ADDR(base) ((base) + (0x02E6))
#define PMIC_HRESET_PWRDOWN_CTRL_ADDR(base) ((base) + (0x02E7))
#define PMIC_SMPL_CTRL_ADDR(base) ((base) + (0x02E8))
#define PMIC_SYS_CTRL0_ADDR(base) ((base) + (0x02E9))
#define PMIC_SYS_CTRL1_ADDR(base) ((base) + (0x02EA))
#define PMIC_DEBUG_LOCK_ADDR(base) ((base) + (0x02EB))
#define PMIC_SYS_DEBUG0_ADDR(base) ((base) + (0x02EC))
#define PMIC_SYS_DEBUG1_ADDR(base) ((base) + (0x02ED))
#define PMIC_SYS_DEBUG2_ADDR(base) ((base) + (0x02EE))
#define PMIC_SYS_DEBUG3_ADDR(base) ((base) + (0x02EF))
#define PMIC_SYS_DEBUG4_ADDR(base) ((base) + (0x02F0))
#define PMIC_SYS_DEBUG5_ADDR(base) ((base) + (0x02F1))
#define PMIC_BACKUP_CHG_ADDR(base) ((base) + (0x02F2))
#define PMIC_NP_CTRL_ADDR(base) ((base) + (0x02F3))
#define PMIC_NP_D2A_RES0_ADDR(base) ((base) + (0x02F4))
#define PMIC_NP_D2A_RES1_ADDR(base) ((base) + (0x02F5))
#define PMIC_HRST_REG0_ADDR(base) ((base) + (0x02F6))
#define PMIC_HRST_REG1_ADDR(base) ((base) + (0x02F7))
#define PMIC_HRST_REG2_ADDR(base) ((base) + (0x02F8))
#define PMIC_HRST_REG3_ADDR(base) ((base) + (0x02F9))
#define PMIC_HRST_REG4_ADDR(base) ((base) + (0x02FA))
#define PMIC_HRST_REG5_ADDR(base) ((base) + (0x02FB))
#define PMIC_HRST_REG6_ADDR(base) ((base) + (0x02FC))
#define PMIC_HRST_REG7_ADDR(base) ((base) + (0x02FD))
#define PMIC_HRST_REG8_ADDR(base) ((base) + (0x02FE))
#define PMIC_HRST_REG9_ADDR(base) ((base) + (0x02FF))
#define PMIC_HRST_REG10_ADDR(base) ((base) + (0x0300))
#define PMIC_HRST_REG11_ADDR(base) ((base) + (0x0301))
#define PMIC_HRST_REG12_ADDR(base) ((base) + (0x0302))
#define PMIC_HRST_REG13_ADDR(base) ((base) + (0x0303))
#define PMIC_HRST_REG14_ADDR(base) ((base) + (0x0304))
#define PMIC_HRST_REG15_ADDR(base) ((base) + (0x0305))
#define PMIC_HRST_REG16_ADDR(base) ((base) + (0x0306))
#define PMIC_HRST_REG17_ADDR(base) ((base) + (0x0307))
#define PMIC_HRST_REG18_ADDR(base) ((base) + (0x0308))
#define PMIC_HRST_REG19_ADDR(base) ((base) + (0x0309))
#define PMIC_HRST_REG20_ADDR(base) ((base) + (0x030A))
#define PMIC_HRST_REG21_ADDR(base) ((base) + (0x030B))
#define PMIC_HRST_REG22_ADDR(base) ((base) + (0x030C))
#define PMIC_HRST_REG23_ADDR(base) ((base) + (0x030D))
#define PMIC_HRST_REG24_ADDR(base) ((base) + (0x030E))
#define PMIC_HRST_REG25_ADDR(base) ((base) + (0x030F))
#define PMIC_HRST_REG26_ADDR(base) ((base) + (0x0310))
#define PMIC_HRST_REG27_ADDR(base) ((base) + (0x0311))
#define PMIC_HRST_REG28_ADDR(base) ((base) + (0x0312))
#define PMIC_HRST_REG29_ADDR(base) ((base) + (0x0313))
#define PMIC_HRST_REG30_ADDR(base) ((base) + (0x0314))
#define PMIC_HRST_REG31_ADDR(base) ((base) + (0x0315))
#define PMIC_OTP0_0_R_ADDR(base) ((base) + (0x0316))
#define PMIC_OTP0_1_R_ADDR(base) ((base) + (0x0317))
#define PMIC_OTP0_2_R_ADDR(base) ((base) + (0x0318))
#define PMIC_OTP0_3_R_ADDR(base) ((base) + (0x0319))
#define PMIC_OTP0_4_R_ADDR(base) ((base) + (0x031A))
#define PMIC_OTP0_5_R_ADDR(base) ((base) + (0x031B))
#define PMIC_OTP0_6_R_ADDR(base) ((base) + (0x031C))
#define PMIC_OTP0_7_R_ADDR(base) ((base) + (0x031D))
#define PMIC_OTP0_8_R_ADDR(base) ((base) + (0x031E))
#define PMIC_OTP0_9_R_ADDR(base) ((base) + (0x031F))
#define PMIC_OTP0_10_R_ADDR(base) ((base) + (0x0320))
#define PMIC_OTP0_11_R_ADDR(base) ((base) + (0x0321))
#define PMIC_OTP0_12_R_ADDR(base) ((base) + (0x0322))
#define PMIC_OTP0_13_R_ADDR(base) ((base) + (0x0323))
#define PMIC_OTP0_14_R_ADDR(base) ((base) + (0x0324))
#define PMIC_OTP0_15_R_ADDR(base) ((base) + (0x0325))
#define PMIC_OTP0_16_R_ADDR(base) ((base) + (0x0326))
#define PMIC_OTP0_17_R_ADDR(base) ((base) + (0x0327))
#define PMIC_OTP0_18_R_ADDR(base) ((base) + (0x0328))
#define PMIC_OTP0_19_R_ADDR(base) ((base) + (0x0329))
#define PMIC_OTP0_20_R_ADDR(base) ((base) + (0x032A))
#define PMIC_OTP0_21_R_ADDR(base) ((base) + (0x032B))
#define PMIC_OTP0_22_R_ADDR(base) ((base) + (0x032C))
#define PMIC_OTP0_23_R_ADDR(base) ((base) + (0x032D))
#define PMIC_OTP0_24_R_ADDR(base) ((base) + (0x032E))
#define PMIC_OTP0_25_R_ADDR(base) ((base) + (0x032F))
#define PMIC_OTP0_26_R_ADDR(base) ((base) + (0x0330))
#define PMIC_OTP0_27_R_ADDR(base) ((base) + (0x0331))
#define PMIC_OTP0_28_R_ADDR(base) ((base) + (0x0332))
#define PMIC_OTP0_29_R_ADDR(base) ((base) + (0x0333))
#define PMIC_OTP0_30_R_ADDR(base) ((base) + (0x0334))
#define PMIC_OTP0_31_R_ADDR(base) ((base) + (0x0335))
#define PMIC_OTP0_32_R_ADDR(base) ((base) + (0x0336))
#define PMIC_OTP0_33_R_ADDR(base) ((base) + (0x0337))
#define PMIC_OTP0_34_R_ADDR(base) ((base) + (0x0338))
#define PMIC_OTP0_35_R_ADDR(base) ((base) + (0x0339))
#define PMIC_OTP0_36_R_ADDR(base) ((base) + (0x033A))
#define PMIC_OTP0_37_R_ADDR(base) ((base) + (0x033B))
#define PMIC_OTP0_38_R_ADDR(base) ((base) + (0x033C))
#define PMIC_OTP0_39_R_ADDR(base) ((base) + (0x033D))
#define PMIC_OTP0_40_R_ADDR(base) ((base) + (0x033E))
#define PMIC_OTP0_41_R_ADDR(base) ((base) + (0x033F))
#define PMIC_OTP0_42_R_ADDR(base) ((base) + (0x0340))
#define PMIC_OTP0_43_R_ADDR(base) ((base) + (0x0341))
#define PMIC_OTP0_44_R_ADDR(base) ((base) + (0x0342))
#define PMIC_OTP0_45_R_ADDR(base) ((base) + (0x0343))
#define PMIC_OTP0_46_R_ADDR(base) ((base) + (0x0344))
#define PMIC_OTP0_47_R_ADDR(base) ((base) + (0x0345))
#define PMIC_OTP0_48_R_ADDR(base) ((base) + (0x0346))
#define PMIC_OTP0_49_R_ADDR(base) ((base) + (0x0347))
#define PMIC_OTP0_50_R_ADDR(base) ((base) + (0x0348))
#define PMIC_OTP0_51_R_ADDR(base) ((base) + (0x0349))
#define PMIC_OTP0_52_R_ADDR(base) ((base) + (0x034A))
#define PMIC_OTP0_53_R_ADDR(base) ((base) + (0x034B))
#define PMIC_OTP0_54_R_ADDR(base) ((base) + (0x034C))
#define PMIC_OTP0_55_R_ADDR(base) ((base) + (0x034D))
#define PMIC_OTP0_56_R_ADDR(base) ((base) + (0x034E))
#define PMIC_OTP0_57_R_ADDR(base) ((base) + (0x034F))
#define PMIC_OTP0_58_R_ADDR(base) ((base) + (0x0350))
#define PMIC_OTP0_59_R_ADDR(base) ((base) + (0x0351))
#define PMIC_OTP0_60_R_ADDR(base) ((base) + (0x0352))
#define PMIC_OTP0_61_R_ADDR(base) ((base) + (0x0353))
#define PMIC_OTP0_62_R_ADDR(base) ((base) + (0x0354))
#define PMIC_OTP0_63_R_ADDR(base) ((base) + (0x0355))
#define PMIC_OTP1_0_R_ADDR(base) ((base) + (0x0356))
#define PMIC_OTP1_1_R_ADDR(base) ((base) + (0x0357))
#define PMIC_OTP1_2_R_ADDR(base) ((base) + (0x0358))
#define PMIC_OTP1_3_R_ADDR(base) ((base) + (0x0359))
#define PMIC_OTP1_4_R_ADDR(base) ((base) + (0x035A))
#define PMIC_OTP1_5_R_ADDR(base) ((base) + (0x035B))
#define PMIC_OTP1_6_R_ADDR(base) ((base) + (0x035C))
#define PMIC_OTP1_7_R_ADDR(base) ((base) + (0x035D))
#define PMIC_OTP1_8_R_ADDR(base) ((base) + (0x035E))
#define PMIC_OTP1_9_R_ADDR(base) ((base) + (0x035F))
#define PMIC_OTP1_10_R_ADDR(base) ((base) + (0x0360))
#define PMIC_OTP1_11_R_ADDR(base) ((base) + (0x0361))
#define PMIC_OTP1_12_R_ADDR(base) ((base) + (0x0362))
#define PMIC_OTP1_13_R_ADDR(base) ((base) + (0x0363))
#define PMIC_OTP1_14_R_ADDR(base) ((base) + (0x0364))
#define PMIC_OTP1_15_R_ADDR(base) ((base) + (0x0365))
#define PMIC_OTP1_16_R_ADDR(base) ((base) + (0x0366))
#define PMIC_OTP1_17_R_ADDR(base) ((base) + (0x0367))
#define PMIC_OTP1_18_R_ADDR(base) ((base) + (0x6028))
#define PMIC_OTP1_19_R_ADDR(base) ((base) + (0x6029))
#define PMIC_OTP1_20_R_ADDR(base) ((base) + (0x602A))
#define PMIC_OTP1_21_R_ADDR(base) ((base) + (0x602B))
#define PMIC_OTP1_22_R_ADDR(base) ((base) + (0x602C))
#define PMIC_OTP1_23_R_ADDR(base) ((base) + (0x602D))
#define PMIC_OTP1_24_R_ADDR(base) ((base) + (0x602E))
#define PMIC_OTP1_25_R_ADDR(base) ((base) + (0x602F))
#define PMIC_OTP1_26_R_ADDR(base) ((base) + (0x6030))
#define PMIC_OTP1_27_R_ADDR(base) ((base) + (0x6031))
#define PMIC_OTP1_28_R_ADDR(base) ((base) + (0x0372))
#define PMIC_OTP1_29_R_ADDR(base) ((base) + (0x0373))
#define PMIC_OTP1_30_R_ADDR(base) ((base) + (0x0374))
#define PMIC_OTP1_31_R_ADDR(base) ((base) + (0x0375))
#define PMIC_OTP1_32_R_ADDR(base) ((base) + (0x0376))
#define PMIC_OTP1_33_R_ADDR(base) ((base) + (0x0377))
#define PMIC_OTP1_34_R_ADDR(base) ((base) + (0x0378))
#define PMIC_OTP1_35_R_ADDR(base) ((base) + (0x0379))
#define PMIC_OTP1_36_R_ADDR(base) ((base) + (0x037A))
#define PMIC_OTP1_37_R_ADDR(base) ((base) + (0x037B))
#define PMIC_OTP1_38_R_ADDR(base) ((base) + (0x037C))
#define PMIC_OTP1_39_R_ADDR(base) ((base) + (0x037D))
#define PMIC_OTP1_40_R_ADDR(base) ((base) + (0x037E))
#define PMIC_OTP1_41_R_ADDR(base) ((base) + (0x037F))
#define PMIC_OTP1_42_R_ADDR(base) ((base) + (0x0380))
#define PMIC_OTP1_43_R_ADDR(base) ((base) + (0x0381))
#define PMIC_OTP1_44_R_ADDR(base) ((base) + (0x0382))
#define PMIC_OTP1_45_R_ADDR(base) ((base) + (0x0383))
#define PMIC_OTP1_46_R_ADDR(base) ((base) + (0x0384))
#define PMIC_OTP1_47_R_ADDR(base) ((base) + (0x0385))
#define PMIC_OTP1_48_R_ADDR(base) ((base) + (0x0386))
#define PMIC_OTP1_49_R_ADDR(base) ((base) + (0x0387))
#define PMIC_OTP1_50_R_ADDR(base) ((base) + (0x0388))
#define PMIC_OTP1_51_R_ADDR(base) ((base) + (0x0389))
#define PMIC_OTP1_52_R_ADDR(base) ((base) + (0x038A))
#define PMIC_OTP1_53_R_ADDR(base) ((base) + (0x038B))
#define PMIC_OTP1_54_R_ADDR(base) ((base) + (0x038C))
#define PMIC_OTP1_55_R_ADDR(base) ((base) + (0x038D))
#define PMIC_OTP1_56_R_ADDR(base) ((base) + (0x038E))
#define PMIC_OTP1_57_R_ADDR(base) ((base) + (0x038F))
#define PMIC_OTP1_58_R_ADDR(base) ((base) + (0x0390))
#define PMIC_OTP1_59_R_ADDR(base) ((base) + (0x0391))
#define PMIC_OTP1_60_R_ADDR(base) ((base) + (0x0392))
#define PMIC_OTP1_61_R_ADDR(base) ((base) + (0x0393))
#define PMIC_OTP1_62_R_ADDR(base) ((base) + (0x0394))
#define PMIC_OTP1_63_R_ADDR(base) ((base) + (0x0395))
#define PMIC_EN_PMUA_PMUD_ADDR(base) ((base) + (0x0396))
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
#define PMIC_DIG_IO_DS_CFG0_ADDR(base) ((base) + (0x03DAUL))
#define PMIC_DIG_IO_DS_CFG1_ADDR(base) ((base) + (0x03DBUL))
#define PMIC_DIG_IO_DS_CFG2_ADDR(base) ((base) + (0x03DCUL))
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
#define PMIC_DIG_IO_DS_CFG0_ADDR(base) ((base) + (0x03DA))
#define PMIC_DIG_IO_DS_CFG1_ADDR(base) ((base) + (0x03DB))
#define PMIC_DIG_IO_DS_CFG2_ADDR(base) ((base) + (0x03DC))
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
#define PMIC_CIC_CTRL_ADDR(base) ((base) + (0x4005UL))
#define PMIC_COUL_TEMP_CTRL_ADDR(base) ((base) + (0x4006UL))
#define PMIC_COUL_VI_CTRL0_ADDR(base) ((base) + (0x4007UL))
#define PMIC_COUL_VI_CTRL1_ADDR(base) ((base) + (0x4008UL))
#define PMIC_COUL_VI_CTRL2_ADDR(base) ((base) + (0x4009UL))
#define PMIC_COUL_VI_CTRL3_ADDR(base) ((base) + (0x400AUL))
#define PMIC_COUL_VI_CTRL4_ADDR(base) ((base) + (0x400BUL))
#define PMIC_COUL_VI_CTRL5_ADDR(base) ((base) + (0x400CUL))
#define PMIC_CL_OUT0_ADDR(base) ((base) + (0x400DUL))
#define PMIC_CL_OUT1_ADDR(base) ((base) + (0x400EUL))
#define PMIC_CL_OUT2_ADDR(base) ((base) + (0x400FUL))
#define PMIC_CL_OUT3_ADDR(base) ((base) + (0x4010UL))
#define PMIC_CL_OUT4_ADDR(base) ((base) + (0x4011UL))
#define PMIC_CL_IN0_ADDR(base) ((base) + (0x4012UL))
#define PMIC_CL_IN1_ADDR(base) ((base) + (0x4013UL))
#define PMIC_CL_IN2_ADDR(base) ((base) + (0x4014UL))
#define PMIC_CL_IN3_ADDR(base) ((base) + (0x4015UL))
#define PMIC_CL_IN4_ADDR(base) ((base) + (0x4016UL))
#define PMIC_CHG_TIMER0_ADDR(base) ((base) + (0x4017UL))
#define PMIC_CHG_TIMER1_ADDR(base) ((base) + (0x4018UL))
#define PMIC_CHG_TIMER2_ADDR(base) ((base) + (0x4019UL))
#define PMIC_CHG_TIMER3_ADDR(base) ((base) + (0x401AUL))
#define PMIC_LOAD_TIMER0_ADDR(base) ((base) + (0x401BUL))
#define PMIC_LOAD_TIMER1_ADDR(base) ((base) + (0x401CUL))
#define PMIC_LOAD_TIMER2_ADDR(base) ((base) + (0x401DUL))
#define PMIC_LOAD_TIMER3_ADDR(base) ((base) + (0x401EUL))
#define PMIC_CL_INT0_ADDR(base) ((base) + (0x401FUL))
#define PMIC_CL_INT1_ADDR(base) ((base) + (0x4020UL))
#define PMIC_CL_INT2_ADDR(base) ((base) + (0x4021UL))
#define PMIC_CL_INT3_ADDR(base) ((base) + (0x4022UL))
#define PMIC_CL_INT4_ADDR(base) ((base) + (0x4023UL))
#define PMIC_V_INT0_ADDR(base) ((base) + (0x4024UL))
#define PMIC_V_INT1_ADDR(base) ((base) + (0x4025UL))
#define PMIC_V_INT2_ADDR(base) ((base) + (0x4026UL))
#define PMIC_I_OUT_GATE0_ADDR(base) ((base) + (0x4027UL))
#define PMIC_I_OUT_GATE1_ADDR(base) ((base) + (0x4028UL))
#define PMIC_I_OUT_GATE2_ADDR(base) ((base) + (0x4029UL))
#define PMIC_I_IN_GATE0_ADDR(base) ((base) + (0x402AUL))
#define PMIC_I_IN_GATE1_ADDR(base) ((base) + (0x402BUL))
#define PMIC_I_IN_GATE2_ADDR(base) ((base) + (0x402CUL))
#define PMIC_OFFSET_CURRENT_MOD_0_ADDR(base) ((base) + (0x402DUL))
#define PMIC_OFFSET_CURRENT_MOD_1_ADDR(base) ((base) + (0x402EUL))
#define PMIC_OFFSET_CURRENT_MOD_2_ADDR(base) ((base) + (0x402FUL))
#define PMIC_OFFSET_VOLTAGE_MOD_0_ADDR(base) ((base) + (0x4030UL))
#define PMIC_OFFSET_VOLTAGE_MOD_1_ADDR(base) ((base) + (0x4031UL))
#define PMIC_OFFSET_VOLTAGE_MOD_2_ADDR(base) ((base) + (0x4032UL))
#define PMIC_STATE_TEST_ADDR(base) ((base) + (0x4033UL))
#define PMIC_CURRENT_0_ADDR(base) ((base) + (0x4034UL))
#define PMIC_CURRENT_1_ADDR(base) ((base) + (0x4035UL))
#define PMIC_CURRENT_2_ADDR(base) ((base) + (0x4036UL))
#define PMIC_V_OUT_0_ADDR(base) ((base) + (0x4037UL))
#define PMIC_V_OUT_1_ADDR(base) ((base) + (0x4038UL))
#define PMIC_V_OUT_2_ADDR(base) ((base) + (0x4039UL))
#define PMIC_OFFSET_CURRENT0_ADDR(base) ((base) + (0x403AUL))
#define PMIC_OFFSET_CURRENT1_ADDR(base) ((base) + (0x403BUL))
#define PMIC_OFFSET_CURRENT2_ADDR(base) ((base) + (0x403CUL))
#define PMIC_OFFSET_VOLTAGE0_ADDR(base) ((base) + (0x403DUL))
#define PMIC_OFFSET_VOLTAGE1_ADDR(base) ((base) + (0x403EUL))
#define PMIC_OFFSET_VOLTAGE2_ADDR(base) ((base) + (0x403FUL))
#define PMIC_OCV_VOLTAGE0_ADDR(base) ((base) + (0x4040UL))
#define PMIC_OCV_VOLTAGE1_ADDR(base) ((base) + (0x4041UL))
#define PMIC_OCV_VOLTAGE2_ADDR(base) ((base) + (0x4042UL))
#define PMIC_OCV_CURRENT0_ADDR(base) ((base) + (0x4043UL))
#define PMIC_OCV_CURRENT1_ADDR(base) ((base) + (0x4044UL))
#define PMIC_OCV_CURRENT2_ADDR(base) ((base) + (0x4045UL))
#define PMIC_OCV_TEMP0_ADDR(base) ((base) + (0x4046UL))
#define PMIC_OCV_TEMP1_ADDR(base) ((base) + (0x4047UL))
#define PMIC_OCV_TEMP2_ADDR(base) ((base) + (0x4048UL))
#define PMIC_ECO_OUT_CLIN_0_ADDR(base) ((base) + (0x4049UL))
#define PMIC_ECO_OUT_CLIN_1_ADDR(base) ((base) + (0x404AUL))
#define PMIC_ECO_OUT_CLIN_2_ADDR(base) ((base) + (0x404BUL))
#define PMIC_ECO_OUT_CLIN_3_ADDR(base) ((base) + (0x404CUL))
#define PMIC_ECO_OUT_CLIN_4_ADDR(base) ((base) + (0x404DUL))
#define PMIC_ECO_OUT_CLOUT_0_ADDR(base) ((base) + (0x404EUL))
#define PMIC_ECO_OUT_CLOUT_1_ADDR(base) ((base) + (0x404FUL))
#define PMIC_ECO_OUT_CLOUT_2_ADDR(base) ((base) + (0x4050UL))
#define PMIC_ECO_OUT_CLOUT_3_ADDR(base) ((base) + (0x4051UL))
#define PMIC_ECO_OUT_CLOUT_4_ADDR(base) ((base) + (0x4052UL))
#define PMIC_ECO_OUT_TEMP_0_ADDR(base) ((base) + (0x4053UL))
#define PMIC_ECO_OUT_TEMP_1_ADDR(base) ((base) + (0x4054UL))
#define PMIC_ECO_OUT_TEMP_2_ADDR(base) ((base) + (0x4055UL))
#define PMIC_TEMP0_RDATA_ADDR(base) ((base) + (0x4056UL))
#define PMIC_TEMP1_RDATA_ADDR(base) ((base) + (0x4057UL))
#define PMIC_TEMP2_RDATA_ADDR(base) ((base) + (0x4058UL))
#define PMIC_V_PRE0_OUT0_ADDR(base) ((base) + (0x4059UL))
#define PMIC_V_PRE0_OUT1_ADDR(base) ((base) + (0x405AUL))
#define PMIC_V_PRE0_OUT2_ADDR(base) ((base) + (0x405BUL))
#define PMIC_V_PRE1_OUT0_ADDR(base) ((base) + (0x405CUL))
#define PMIC_V_PRE1_OUT1_ADDR(base) ((base) + (0x405DUL))
#define PMIC_V_PRE1_OUT2_ADDR(base) ((base) + (0x405EUL))
#define PMIC_V_PRE2_OUT0_ADDR(base) ((base) + (0x405FUL))
#define PMIC_V_PRE2_OUT1_ADDR(base) ((base) + (0x4060UL))
#define PMIC_V_PRE2_OUT2_ADDR(base) ((base) + (0x4061UL))
#define PMIC_V_PRE3_OUT0_ADDR(base) ((base) + (0x4062UL))
#define PMIC_V_PRE3_OUT1_ADDR(base) ((base) + (0x4063UL))
#define PMIC_V_PRE3_OUT2_ADDR(base) ((base) + (0x4064UL))
#define PMIC_V_PRE4_OUT0_ADDR(base) ((base) + (0x4065UL))
#define PMIC_V_PRE4_OUT1_ADDR(base) ((base) + (0x4066UL))
#define PMIC_V_PRE4_OUT2_ADDR(base) ((base) + (0x4067UL))
#define PMIC_V_PRE5_OUT0_ADDR(base) ((base) + (0x4068UL))
#define PMIC_V_PRE5_OUT1_ADDR(base) ((base) + (0x4069UL))
#define PMIC_V_PRE5_OUT2_ADDR(base) ((base) + (0x406AUL))
#define PMIC_V_PRE6_OUT0_ADDR(base) ((base) + (0x406BUL))
#define PMIC_V_PRE6_OUT1_ADDR(base) ((base) + (0x406CUL))
#define PMIC_V_PRE6_OUT2_ADDR(base) ((base) + (0x406DUL))
#define PMIC_V_PRE7_OUT0_ADDR(base) ((base) + (0x406EUL))
#define PMIC_V_PRE7_OUT1_ADDR(base) ((base) + (0x406FUL))
#define PMIC_V_PRE7_OUT2_ADDR(base) ((base) + (0x4070UL))
#define PMIC_V_PRE8_OUT0_ADDR(base) ((base) + (0x4071UL))
#define PMIC_V_PRE8_OUT1_ADDR(base) ((base) + (0x4072UL))
#define PMIC_V_PRE8_OUT2_ADDR(base) ((base) + (0x4073UL))
#define PMIC_V_PRE9_OUT0_ADDR(base) ((base) + (0x4074UL))
#define PMIC_V_PRE9_OUT1_ADDR(base) ((base) + (0x4075UL))
#define PMIC_V_PRE9_OUT2_ADDR(base) ((base) + (0x4076UL))
#define PMIC_CURRENT_PRE0_OUT0_ADDR(base) ((base) + (0x4077UL))
#define PMIC_CURRENT_PRE0_OUT1_ADDR(base) ((base) + (0x4078UL))
#define PMIC_CURRENT_PRE0_OUT2_ADDR(base) ((base) + (0x4079UL))
#define PMIC_CURRENT_PRE1_OUT0_ADDR(base) ((base) + (0x407AUL))
#define PMIC_CURRENT_PRE1_OUT1_ADDR(base) ((base) + (0x407BUL))
#define PMIC_CURRENT_PRE1_OUT2_ADDR(base) ((base) + (0x407CUL))
#define PMIC_CURRENT_PRE2_OUT0_ADDR(base) ((base) + (0x407DUL))
#define PMIC_CURRENT_PRE2_OUT1_ADDR(base) ((base) + (0x407EUL))
#define PMIC_CURRENT_PRE2_OUT2_ADDR(base) ((base) + (0x407FUL))
#define PMIC_CURRENT_PRE3_OUT0_ADDR(base) ((base) + (0x4080UL))
#define PMIC_CURRENT_PRE3_OUT1_ADDR(base) ((base) + (0x4081UL))
#define PMIC_CURRENT_PRE3_OUT2_ADDR(base) ((base) + (0x4082UL))
#define PMIC_CURRENT_PRE4_OUT0_ADDR(base) ((base) + (0x4083UL))
#define PMIC_CURRENT_PRE4_OUT1_ADDR(base) ((base) + (0x4084UL))
#define PMIC_CURRENT_PRE4_OUT2_ADDR(base) ((base) + (0x4085UL))
#define PMIC_CURRENT_PRE5_OUT0_ADDR(base) ((base) + (0x4086UL))
#define PMIC_CURRENT_PRE5_OUT1_ADDR(base) ((base) + (0x4087UL))
#define PMIC_CURRENT_PRE5_OUT2_ADDR(base) ((base) + (0x4088UL))
#define PMIC_CURRENT_PRE6_OUT0_ADDR(base) ((base) + (0x4089UL))
#define PMIC_CURRENT_PRE6_OUT1_ADDR(base) ((base) + (0x408AUL))
#define PMIC_CURRENT_PRE6_OUT2_ADDR(base) ((base) + (0x408BUL))
#define PMIC_CURRENT_PRE7_OUT0_ADDR(base) ((base) + (0x408CUL))
#define PMIC_CURRENT_PRE7_OUT1_ADDR(base) ((base) + (0x408DUL))
#define PMIC_CURRENT_PRE7_OUT2_ADDR(base) ((base) + (0x408EUL))
#define PMIC_CURRENT_PRE8_OUT0_ADDR(base) ((base) + (0x408FUL))
#define PMIC_CURRENT_PRE8_OUT1_ADDR(base) ((base) + (0x4090UL))
#define PMIC_CURRENT_PRE8_OUT2_ADDR(base) ((base) + (0x4091UL))
#define PMIC_CURRENT_PRE9_OUT0_ADDR(base) ((base) + (0x4092UL))
#define PMIC_CURRENT_PRE9_OUT1_ADDR(base) ((base) + (0x4093UL))
#define PMIC_CURRENT_PRE9_OUT2_ADDR(base) ((base) + (0x4094UL))
#define PMIC_V_OCV_PRE1_OUT0_ADDR(base) ((base) + (0x4095UL))
#define PMIC_V_OCV_PRE1_OUT1_ADDR(base) ((base) + (0x4096UL))
#define PMIC_V_OCV_PRE1_OUT2_ADDR(base) ((base) + (0x4097UL))
#define PMIC_V_OCV_PRE2_OUT0_ADDR(base) ((base) + (0x4098UL))
#define PMIC_V_OCV_PRE2_OUT1_ADDR(base) ((base) + (0x4099UL))
#define PMIC_V_OCV_PRE2_OUT2_ADDR(base) ((base) + (0x409AUL))
#define PMIC_V_OCV_PRE3_OUT0_ADDR(base) ((base) + (0x409BUL))
#define PMIC_V_OCV_PRE3_OUT1_ADDR(base) ((base) + (0x409CUL))
#define PMIC_V_OCV_PRE3_OUT2_ADDR(base) ((base) + (0x409DUL))
#define PMIC_V_OCV_PRE4_OUT0_ADDR(base) ((base) + (0x409EUL))
#define PMIC_V_OCV_PRE4_OUT1_ADDR(base) ((base) + (0x409FUL))
#define PMIC_V_OCV_PRE4_OUT2_ADDR(base) ((base) + (0x40A0UL))
#define PMIC_V_OCV_PRE5_OUT0_ADDR(base) ((base) + (0x40A1UL))
#define PMIC_V_OCV_PRE5_OUT1_ADDR(base) ((base) + (0x40A2UL))
#define PMIC_V_OCV_PRE5_OUT2_ADDR(base) ((base) + (0x40A3UL))
#define PMIC_V_OCV_PRE6_OUT0_ADDR(base) ((base) + (0x40A4UL))
#define PMIC_V_OCV_PRE6_OUT1_ADDR(base) ((base) + (0x40A5UL))
#define PMIC_V_OCV_PRE6_OUT2_ADDR(base) ((base) + (0x40A6UL))
#define PMIC_V_OCV_PRE7_OUT0_ADDR(base) ((base) + (0x40A7UL))
#define PMIC_V_OCV_PRE7_OUT1_ADDR(base) ((base) + (0x40A8UL))
#define PMIC_V_OCV_PRE7_OUT2_ADDR(base) ((base) + (0x40A9UL))
#define PMIC_V_OCV_PRE8_OUT0_ADDR(base) ((base) + (0x40AAUL))
#define PMIC_V_OCV_PRE8_OUT1_ADDR(base) ((base) + (0x40ABUL))
#define PMIC_V_OCV_PRE8_OUT2_ADDR(base) ((base) + (0x40ACUL))
#define PMIC_I_OCV_PRE1_OUT0_ADDR(base) ((base) + (0x40ADUL))
#define PMIC_I_OCV_PRE1_OUT1_ADDR(base) ((base) + (0x40AEUL))
#define PMIC_I_OCV_PRE1_OUT2_ADDR(base) ((base) + (0x40AFUL))
#define PMIC_I_OCV_PRE2_OUT0_ADDR(base) ((base) + (0x40B0UL))
#define PMIC_I_OCV_PRE2_OUT1_ADDR(base) ((base) + (0x40B1UL))
#define PMIC_I_OCV_PRE2_OUT2_ADDR(base) ((base) + (0x40B2UL))
#define PMIC_I_OCV_PRE3_OUT0_ADDR(base) ((base) + (0x40B3UL))
#define PMIC_I_OCV_PRE3_OUT1_ADDR(base) ((base) + (0x40B4UL))
#define PMIC_I_OCV_PRE3_OUT2_ADDR(base) ((base) + (0x40B5UL))
#define PMIC_I_OCV_PRE4_OUT0_ADDR(base) ((base) + (0x40B6UL))
#define PMIC_I_OCV_PRE4_OUT1_ADDR(base) ((base) + (0x40B7UL))
#define PMIC_I_OCV_PRE4_OUT2_ADDR(base) ((base) + (0x40B8UL))
#define PMIC_I_OCV_PRE5_OUT0_ADDR(base) ((base) + (0x40B9UL))
#define PMIC_I_OCV_PRE5_OUT1_ADDR(base) ((base) + (0x40BAUL))
#define PMIC_I_OCV_PRE5_OUT2_ADDR(base) ((base) + (0x40BBUL))
#define PMIC_I_OCV_PRE6_OUT0_ADDR(base) ((base) + (0x40BCUL))
#define PMIC_I_OCV_PRE6_OUT1_ADDR(base) ((base) + (0x40BDUL))
#define PMIC_I_OCV_PRE6_OUT2_ADDR(base) ((base) + (0x40BEUL))
#define PMIC_I_OCV_PRE7_OUT0_ADDR(base) ((base) + (0x40BFUL))
#define PMIC_I_OCV_PRE7_OUT1_ADDR(base) ((base) + (0x40C0UL))
#define PMIC_I_OCV_PRE7_OUT2_ADDR(base) ((base) + (0x40C1UL))
#define PMIC_I_OCV_PRE8_OUT0_ADDR(base) ((base) + (0x40C2UL))
#define PMIC_I_OCV_PRE8_OUT1_ADDR(base) ((base) + (0x40C3UL))
#define PMIC_I_OCV_PRE8_OUT2_ADDR(base) ((base) + (0x40C4UL))
#define PMIC_T_OCV_PRE1_OUT0_ADDR(base) ((base) + (0x40C5UL))
#define PMIC_T_OCV_PRE1_OUT1_ADDR(base) ((base) + (0x40C6UL))
#define PMIC_T_OCV_PRE1_OUT2_ADDR(base) ((base) + (0x40C7UL))
#define PMIC_T_OCV_PRE2_OUT0_ADDR(base) ((base) + (0x40C8UL))
#define PMIC_T_OCV_PRE2_OUT1_ADDR(base) ((base) + (0x40C9UL))
#define PMIC_T_OCV_PRE2_OUT2_ADDR(base) ((base) + (0x40CAUL))
#define PMIC_T_OCV_PRE3_OUT0_ADDR(base) ((base) + (0x40CBUL))
#define PMIC_T_OCV_PRE3_OUT1_ADDR(base) ((base) + (0x40CCUL))
#define PMIC_T_OCV_PRE3_OUT2_ADDR(base) ((base) + (0x40CDUL))
#define PMIC_T_OCV_PRE4_OUT0_ADDR(base) ((base) + (0x40CEUL))
#define PMIC_T_OCV_PRE4_OUT1_ADDR(base) ((base) + (0x40CFUL))
#define PMIC_T_OCV_PRE4_OUT2_ADDR(base) ((base) + (0x40D0UL))
#define PMIC_T_OCV_PRE5_OUT0_ADDR(base) ((base) + (0x40D1UL))
#define PMIC_T_OCV_PRE5_OUT1_ADDR(base) ((base) + (0x40D2UL))
#define PMIC_T_OCV_PRE5_OUT2_ADDR(base) ((base) + (0x40D3UL))
#define PMIC_T_OCV_PRE6_OUT0_ADDR(base) ((base) + (0x40D4UL))
#define PMIC_T_OCV_PRE6_OUT1_ADDR(base) ((base) + (0x40D5UL))
#define PMIC_T_OCV_PRE6_OUT2_ADDR(base) ((base) + (0x40D6UL))
#define PMIC_T_OCV_PRE7_OUT0_ADDR(base) ((base) + (0x40D7UL))
#define PMIC_T_OCV_PRE7_OUT1_ADDR(base) ((base) + (0x40D8UL))
#define PMIC_T_OCV_PRE7_OUT2_ADDR(base) ((base) + (0x40D9UL))
#define PMIC_T_OCV_PRE8_OUT0_ADDR(base) ((base) + (0x40DAUL))
#define PMIC_T_OCV_PRE8_OUT1_ADDR(base) ((base) + (0x40DBUL))
#define PMIC_T_OCV_PRE8_OUT2_ADDR(base) ((base) + (0x40DCUL))
#define PMIC_RTC_OCV_PRE1_OUT0_ADDR(base) ((base) + (0x40DDUL))
#define PMIC_RTC_OCV_PRE1_OUT1_ADDR(base) ((base) + (0x40DEUL))
#define PMIC_RTC_OCV_PRE1_OUT2_ADDR(base) ((base) + (0x40DFUL))
#define PMIC_RTC_OCV_PRE1_OUT3_ADDR(base) ((base) + (0x40E0UL))
#define PMIC_RTC_OCV_PRE2_OUT0_ADDR(base) ((base) + (0x40E1UL))
#define PMIC_RTC_OCV_PRE2_OUT1_ADDR(base) ((base) + (0x40E2UL))
#define PMIC_RTC_OCV_PRE2_OUT2_ADDR(base) ((base) + (0x40E3UL))
#define PMIC_RTC_OCV_PRE2_OUT3_ADDR(base) ((base) + (0x40E4UL))
#define PMIC_RTC_OCV_PRE3_OUT0_ADDR(base) ((base) + (0x40E5UL))
#define PMIC_RTC_OCV_PRE3_OUT1_ADDR(base) ((base) + (0x40E6UL))
#define PMIC_RTC_OCV_PRE3_OUT2_ADDR(base) ((base) + (0x40E7UL))
#define PMIC_RTC_OCV_PRE3_OUT3_ADDR(base) ((base) + (0x40E8UL))
#define PMIC_RTC_OCV_PRE4_OUT0_ADDR(base) ((base) + (0x40E9UL))
#define PMIC_RTC_OCV_PRE4_OUT1_ADDR(base) ((base) + (0x40EAUL))
#define PMIC_RTC_OCV_PRE4_OUT2_ADDR(base) ((base) + (0x40EBUL))
#define PMIC_RTC_OCV_PRE4_OUT3_ADDR(base) ((base) + (0x40ECUL))
#define PMIC_RTC_OCV_PRE5_OUT0_ADDR(base) ((base) + (0x40EDUL))
#define PMIC_RTC_OCV_PRE5_OUT1_ADDR(base) ((base) + (0x40EEUL))
#define PMIC_RTC_OCV_PRE5_OUT2_ADDR(base) ((base) + (0x40EFUL))
#define PMIC_RTC_OCV_PRE5_OUT3_ADDR(base) ((base) + (0x40F0UL))
#define PMIC_RTC_OCV_PRE6_OUT0_ADDR(base) ((base) + (0x40F1UL))
#define PMIC_RTC_OCV_PRE6_OUT1_ADDR(base) ((base) + (0x40F2UL))
#define PMIC_RTC_OCV_PRE6_OUT2_ADDR(base) ((base) + (0x40F3UL))
#define PMIC_RTC_OCV_PRE6_OUT3_ADDR(base) ((base) + (0x40F4UL))
#define PMIC_RTC_OCV_PRE7_OUT0_ADDR(base) ((base) + (0x40F5UL))
#define PMIC_RTC_OCV_PRE7_OUT1_ADDR(base) ((base) + (0x40F6UL))
#define PMIC_RTC_OCV_PRE7_OUT2_ADDR(base) ((base) + (0x40F7UL))
#define PMIC_RTC_OCV_PRE7_OUT3_ADDR(base) ((base) + (0x40F8UL))
#define PMIC_RTC_OCV_PRE8_OUT0_ADDR(base) ((base) + (0x40F9UL))
#define PMIC_RTC_OCV_PRE8_OUT1_ADDR(base) ((base) + (0x40FAUL))
#define PMIC_RTC_OCV_PRE8_OUT2_ADDR(base) ((base) + (0x40FBUL))
#define PMIC_RTC_OCV_PRE8_OUT3_ADDR(base) ((base) + (0x40FCUL))
#define PMIC_OFTV_OCV_PRE1_OUT0_ADDR(base) ((base) + (0x40FDUL))
#define PMIC_OFTV_OCV_PRE1_OUT1_ADDR(base) ((base) + (0x40FEUL))
#define PMIC_OFTV_OCV_PRE1_OUT2_ADDR(base) ((base) + (0x40FFUL))
#define PMIC_OFTV_OCV_PRE2_OUT0_ADDR(base) ((base) + (0x4100UL))
#define PMIC_OFTV_OCV_PRE2_OUT1_ADDR(base) ((base) + (0x4101UL))
#define PMIC_OFTV_OCV_PRE2_OUT2_ADDR(base) ((base) + (0x4102UL))
#define PMIC_OFTV_OCV_PRE3_OUT0_ADDR(base) ((base) + (0x4103UL))
#define PMIC_OFTV_OCV_PRE3_OUT1_ADDR(base) ((base) + (0x4104UL))
#define PMIC_OFTV_OCV_PRE3_OUT2_ADDR(base) ((base) + (0x4105UL))
#define PMIC_OFTV_OCV_PRE4_OUT0_ADDR(base) ((base) + (0x4106UL))
#define PMIC_OFTV_OCV_PRE4_OUT1_ADDR(base) ((base) + (0x4107UL))
#define PMIC_OFTV_OCV_PRE4_OUT2_ADDR(base) ((base) + (0x4108UL))
#define PMIC_OFTV_OCV_PRE5_OUT0_ADDR(base) ((base) + (0x4109UL))
#define PMIC_OFTV_OCV_PRE5_OUT1_ADDR(base) ((base) + (0x410AUL))
#define PMIC_OFTV_OCV_PRE5_OUT2_ADDR(base) ((base) + (0x410BUL))
#define PMIC_OFTV_OCV_PRE6_OUT0_ADDR(base) ((base) + (0x410CUL))
#define PMIC_OFTV_OCV_PRE6_OUT1_ADDR(base) ((base) + (0x410DUL))
#define PMIC_OFTV_OCV_PRE6_OUT2_ADDR(base) ((base) + (0x410EUL))
#define PMIC_OFTV_OCV_PRE7_OUT0_ADDR(base) ((base) + (0x410FUL))
#define PMIC_OFTV_OCV_PRE7_OUT1_ADDR(base) ((base) + (0x4110UL))
#define PMIC_OFTV_OCV_PRE7_OUT2_ADDR(base) ((base) + (0x4111UL))
#define PMIC_OFTV_OCV_PRE8_OUT0_ADDR(base) ((base) + (0x4112UL))
#define PMIC_OFTV_OCV_PRE8_OUT1_ADDR(base) ((base) + (0x4113UL))
#define PMIC_OFTV_OCV_PRE8_OUT2_ADDR(base) ((base) + (0x4114UL))
#define PMIC_OFTI_OCV_PRE1_OUT0_ADDR(base) ((base) + (0x4115UL))
#define PMIC_OFTI_OCV_PRE1_OUT1_ADDR(base) ((base) + (0x4116UL))
#define PMIC_OFTI_OCV_PRE1_OUT2_ADDR(base) ((base) + (0x4117UL))
#define PMIC_OFTI_OCV_PRE2_OUT0_ADDR(base) ((base) + (0x4118UL))
#define PMIC_OFTI_OCV_PRE2_OUT1_ADDR(base) ((base) + (0x4119UL))
#define PMIC_OFTI_OCV_PRE2_OUT2_ADDR(base) ((base) + (0x411AUL))
#define PMIC_OFTI_OCV_PRE3_OUT0_ADDR(base) ((base) + (0x411BUL))
#define PMIC_OFTI_OCV_PRE3_OUT1_ADDR(base) ((base) + (0x411CUL))
#define PMIC_OFTI_OCV_PRE3_OUT2_ADDR(base) ((base) + (0x411DUL))
#define PMIC_OFTI_OCV_PRE4_OUT0_ADDR(base) ((base) + (0x411EUL))
#define PMIC_OFTI_OCV_PRE4_OUT1_ADDR(base) ((base) + (0x411FUL))
#define PMIC_OFTI_OCV_PRE4_OUT2_ADDR(base) ((base) + (0x4120UL))
#define PMIC_OFTI_OCV_PRE5_OUT0_ADDR(base) ((base) + (0x4121UL))
#define PMIC_OFTI_OCV_PRE5_OUT1_ADDR(base) ((base) + (0x4122UL))
#define PMIC_OFTI_OCV_PRE5_OUT2_ADDR(base) ((base) + (0x4123UL))
#define PMIC_OFTI_OCV_PRE6_OUT0_ADDR(base) ((base) + (0x4124UL))
#define PMIC_OFTI_OCV_PRE6_OUT1_ADDR(base) ((base) + (0x4125UL))
#define PMIC_OFTI_OCV_PRE6_OUT2_ADDR(base) ((base) + (0x4126UL))
#define PMIC_OFTI_OCV_PRE7_OUT0_ADDR(base) ((base) + (0x4127UL))
#define PMIC_OFTI_OCV_PRE7_OUT1_ADDR(base) ((base) + (0x4128UL))
#define PMIC_OFTI_OCV_PRE7_OUT2_ADDR(base) ((base) + (0x4129UL))
#define PMIC_OFTI_OCV_PRE8_OUT0_ADDR(base) ((base) + (0x412AUL))
#define PMIC_OFTI_OCV_PRE8_OUT1_ADDR(base) ((base) + (0x412BUL))
#define PMIC_OFTI_OCV_PRE8_OUT2_ADDR(base) ((base) + (0x412CUL))
#define PMIC_DCR_V_PRE0_OUT0_ADDR(base) ((base) + (0x412DUL))
#define PMIC_DCR_V_PRE0_OUT1_ADDR(base) ((base) + (0x412EUL))
#define PMIC_DCR_V_PRE0_OUT2_ADDR(base) ((base) + (0x412FUL))
#define PMIC_DCR_V_PRE1_OUT0_ADDR(base) ((base) + (0x4130UL))
#define PMIC_DCR_V_PRE1_OUT1_ADDR(base) ((base) + (0x4131UL))
#define PMIC_DCR_V_PRE1_OUT2_ADDR(base) ((base) + (0x4132UL))
#define PMIC_DCR_V_PRE2_OUT0_ADDR(base) ((base) + (0x4133UL))
#define PMIC_DCR_V_PRE2_OUT1_ADDR(base) ((base) + (0x4134UL))
#define PMIC_DCR_V_PRE2_OUT2_ADDR(base) ((base) + (0x4135UL))
#define PMIC_DCR_V_PRE3_OUT0_ADDR(base) ((base) + (0x4136UL))
#define PMIC_DCR_V_PRE3_OUT1_ADDR(base) ((base) + (0x4137UL))
#define PMIC_DCR_V_PRE3_OUT2_ADDR(base) ((base) + (0x4138UL))
#define PMIC_DCR_V_PRE4_OUT0_ADDR(base) ((base) + (0x4139UL))
#define PMIC_DCR_V_PRE4_OUT1_ADDR(base) ((base) + (0x413AUL))
#define PMIC_DCR_V_PRE4_OUT2_ADDR(base) ((base) + (0x413BUL))
#define PMIC_DCR_V_PRE5_OUT0_ADDR(base) ((base) + (0x413CUL))
#define PMIC_DCR_V_PRE5_OUT1_ADDR(base) ((base) + (0x413DUL))
#define PMIC_DCR_V_PRE5_OUT2_ADDR(base) ((base) + (0x413EUL))
#define PMIC_DCR_V_PRE6_OUT0_ADDR(base) ((base) + (0x413FUL))
#define PMIC_DCR_V_PRE6_OUT1_ADDR(base) ((base) + (0x4140UL))
#define PMIC_DCR_V_PRE6_OUT2_ADDR(base) ((base) + (0x4141UL))
#define PMIC_DCR_V_PRE7_OUT0_ADDR(base) ((base) + (0x4142UL))
#define PMIC_DCR_V_PRE7_OUT1_ADDR(base) ((base) + (0x4143UL))
#define PMIC_DCR_V_PRE7_OUT2_ADDR(base) ((base) + (0x4144UL))
#define PMIC_DCR_V_PRE8_OUT0_ADDR(base) ((base) + (0x4145UL))
#define PMIC_DCR_V_PRE8_OUT1_ADDR(base) ((base) + (0x4146UL))
#define PMIC_DCR_V_PRE8_OUT2_ADDR(base) ((base) + (0x4147UL))
#define PMIC_DCR_V_PRE9_OUT0_ADDR(base) ((base) + (0x4148UL))
#define PMIC_DCR_V_PRE9_OUT1_ADDR(base) ((base) + (0x4149UL))
#define PMIC_DCR_V_PRE9_OUT2_ADDR(base) ((base) + (0x414AUL))
#define PMIC_DCR_I_PRE0_OUT0_ADDR(base) ((base) + (0x414BUL))
#define PMIC_DCR_I_PRE0_OUT1_ADDR(base) ((base) + (0x414CUL))
#define PMIC_DCR_I_PRE0_OUT2_ADDR(base) ((base) + (0x414DUL))
#define PMIC_DCR_I_PRE1_OUT0_ADDR(base) ((base) + (0x414EUL))
#define PMIC_DCR_I_PRE1_OUT1_ADDR(base) ((base) + (0x414FUL))
#define PMIC_DCR_I_PRE1_OUT2_ADDR(base) ((base) + (0x4150UL))
#define PMIC_DCR_I_PRE2_OUT0_ADDR(base) ((base) + (0x4151UL))
#define PMIC_DCR_I_PRE2_OUT1_ADDR(base) ((base) + (0x4152UL))
#define PMIC_DCR_I_PRE2_OUT2_ADDR(base) ((base) + (0x4153UL))
#define PMIC_DCR_I_PRE3_OUT0_ADDR(base) ((base) + (0x4154UL))
#define PMIC_DCR_I_PRE3_OUT1_ADDR(base) ((base) + (0x4155UL))
#define PMIC_DCR_I_PRE3_OUT2_ADDR(base) ((base) + (0x4156UL))
#define PMIC_DCR_I_PRE4_OUT0_ADDR(base) ((base) + (0x4157UL))
#define PMIC_DCR_I_PRE4_OUT1_ADDR(base) ((base) + (0x4158UL))
#define PMIC_DCR_I_PRE4_OUT2_ADDR(base) ((base) + (0x4159UL))
#define PMIC_DCR_I_PRE5_OUT0_ADDR(base) ((base) + (0x415AUL))
#define PMIC_DCR_I_PRE5_OUT1_ADDR(base) ((base) + (0x415BUL))
#define PMIC_DCR_I_PRE5_OUT2_ADDR(base) ((base) + (0x415CUL))
#define PMIC_DCR_I_PRE6_OUT0_ADDR(base) ((base) + (0x415DUL))
#define PMIC_DCR_I_PRE6_OUT1_ADDR(base) ((base) + (0x415EUL))
#define PMIC_DCR_I_PRE6_OUT2_ADDR(base) ((base) + (0x415FUL))
#define PMIC_DCR_I_PRE7_OUT0_ADDR(base) ((base) + (0x4160UL))
#define PMIC_DCR_I_PRE7_OUT1_ADDR(base) ((base) + (0x4161UL))
#define PMIC_DCR_I_PRE7_OUT2_ADDR(base) ((base) + (0x4162UL))
#define PMIC_DCR_I_PRE8_OUT0_ADDR(base) ((base) + (0x4163UL))
#define PMIC_DCR_I_PRE8_OUT1_ADDR(base) ((base) + (0x4164UL))
#define PMIC_DCR_I_PRE8_OUT2_ADDR(base) ((base) + (0x4165UL))
#define PMIC_DCR_I_PRE9_OUT0_ADDR(base) ((base) + (0x4166UL))
#define PMIC_DCR_I_PRE9_OUT1_ADDR(base) ((base) + (0x4167UL))
#define PMIC_DCR_I_PRE9_OUT2_ADDR(base) ((base) + (0x4168UL))
#define PMIC_COUL_RESERVE0_ADDR(base) ((base) + (0x4169UL))
#define PMIC_COUL_RESERVE1_ADDR(base) ((base) + (0x416AUL))
#define PMIC_CLJ_DEBUG0_ADDR(base) ((base) + (0x416BUL))
#define PMIC_CLJ_DEBUG1_ADDR(base) ((base) + (0x416CUL))
#define PMIC_DEBUG_CIC_I0_ADDR(base) ((base) + (0x416DUL))
#define PMIC_DEBUG_CIC_I1_ADDR(base) ((base) + (0x416EUL))
#define PMIC_DEBUG_CIC_I2_ADDR(base) ((base) + (0x416FUL))
#define PMIC_DEBUG_CIC_V0_ADDR(base) ((base) + (0x4170UL))
#define PMIC_DEBUG_CIC_V1_ADDR(base) ((base) + (0x4171UL))
#define PMIC_DEBUG_CIC_V2_ADDR(base) ((base) + (0x4172UL))
#define PMIC_DEBUG_WRITE_PRO_ADDR(base) ((base) + (0x4173UL))
#else
#define PMIC_COUL_IRQ_ADDR(base) ((base) + (0x4000))
#define PMIC_COUL_IRQ_MASK_ADDR(base) ((base) + (0x4001))
#define PMIC_COUL_IRQ_NP_ADDR(base) ((base) + (0x4002))
#define PMIC_CLJ_CTRL_REG_ADDR(base) ((base) + (0x4003))
#define PMIC_CLJ_CTRL_REGS2_ADDR(base) ((base) + (0x4004))
#define PMIC_CIC_CTRL_ADDR(base) ((base) + (0x4005))
#define PMIC_COUL_TEMP_CTRL_ADDR(base) ((base) + (0x4006))
#define PMIC_COUL_VI_CTRL0_ADDR(base) ((base) + (0x4007))
#define PMIC_COUL_VI_CTRL1_ADDR(base) ((base) + (0x4008))
#define PMIC_COUL_VI_CTRL2_ADDR(base) ((base) + (0x4009))
#define PMIC_COUL_VI_CTRL3_ADDR(base) ((base) + (0x400A))
#define PMIC_COUL_VI_CTRL4_ADDR(base) ((base) + (0x400B))
#define PMIC_COUL_VI_CTRL5_ADDR(base) ((base) + (0x400C))
#define PMIC_CL_OUT0_ADDR(base) ((base) + (0x400D))
#define PMIC_CL_OUT1_ADDR(base) ((base) + (0x400E))
#define PMIC_CL_OUT2_ADDR(base) ((base) + (0x400F))
#define PMIC_CL_OUT3_ADDR(base) ((base) + (0x4010))
#define PMIC_CL_OUT4_ADDR(base) ((base) + (0x4011))
#define PMIC_CL_IN0_ADDR(base) ((base) + (0x4012))
#define PMIC_CL_IN1_ADDR(base) ((base) + (0x4013))
#define PMIC_CL_IN2_ADDR(base) ((base) + (0x4014))
#define PMIC_CL_IN3_ADDR(base) ((base) + (0x4015))
#define PMIC_CL_IN4_ADDR(base) ((base) + (0x4016))
#define PMIC_CHG_TIMER0_ADDR(base) ((base) + (0x4017))
#define PMIC_CHG_TIMER1_ADDR(base) ((base) + (0x4018))
#define PMIC_CHG_TIMER2_ADDR(base) ((base) + (0x4019))
#define PMIC_CHG_TIMER3_ADDR(base) ((base) + (0x401A))
#define PMIC_LOAD_TIMER0_ADDR(base) ((base) + (0x401B))
#define PMIC_LOAD_TIMER1_ADDR(base) ((base) + (0x401C))
#define PMIC_LOAD_TIMER2_ADDR(base) ((base) + (0x401D))
#define PMIC_LOAD_TIMER3_ADDR(base) ((base) + (0x401E))
#define PMIC_CL_INT0_ADDR(base) ((base) + (0x401F))
#define PMIC_CL_INT1_ADDR(base) ((base) + (0x4020))
#define PMIC_CL_INT2_ADDR(base) ((base) + (0x4021))
#define PMIC_CL_INT3_ADDR(base) ((base) + (0x4022))
#define PMIC_CL_INT4_ADDR(base) ((base) + (0x4023))
#define PMIC_V_INT0_ADDR(base) ((base) + (0x4024))
#define PMIC_V_INT1_ADDR(base) ((base) + (0x4025))
#define PMIC_V_INT2_ADDR(base) ((base) + (0x4026))
#define PMIC_I_OUT_GATE0_ADDR(base) ((base) + (0x4027))
#define PMIC_I_OUT_GATE1_ADDR(base) ((base) + (0x4028))
#define PMIC_I_OUT_GATE2_ADDR(base) ((base) + (0x4029))
#define PMIC_I_IN_GATE0_ADDR(base) ((base) + (0x402A))
#define PMIC_I_IN_GATE1_ADDR(base) ((base) + (0x402B))
#define PMIC_I_IN_GATE2_ADDR(base) ((base) + (0x402C))
#define PMIC_OFFSET_CURRENT_MOD_0_ADDR(base) ((base) + (0x402D))
#define PMIC_OFFSET_CURRENT_MOD_1_ADDR(base) ((base) + (0x402E))
#define PMIC_OFFSET_CURRENT_MOD_2_ADDR(base) ((base) + (0x402F))
#define PMIC_OFFSET_VOLTAGE_MOD_0_ADDR(base) ((base) + (0x4030))
#define PMIC_OFFSET_VOLTAGE_MOD_1_ADDR(base) ((base) + (0x4031))
#define PMIC_OFFSET_VOLTAGE_MOD_2_ADDR(base) ((base) + (0x4032))
#define PMIC_STATE_TEST_ADDR(base) ((base) + (0x4033))
#define PMIC_CURRENT_0_ADDR(base) ((base) + (0x4034))
#define PMIC_CURRENT_1_ADDR(base) ((base) + (0x4035))
#define PMIC_CURRENT_2_ADDR(base) ((base) + (0x4036))
#define PMIC_V_OUT_0_ADDR(base) ((base) + (0x4037))
#define PMIC_V_OUT_1_ADDR(base) ((base) + (0x4038))
#define PMIC_V_OUT_2_ADDR(base) ((base) + (0x4039))
#define PMIC_OFFSET_CURRENT0_ADDR(base) ((base) + (0x403A))
#define PMIC_OFFSET_CURRENT1_ADDR(base) ((base) + (0x403B))
#define PMIC_OFFSET_CURRENT2_ADDR(base) ((base) + (0x403C))
#define PMIC_OFFSET_VOLTAGE0_ADDR(base) ((base) + (0x403D))
#define PMIC_OFFSET_VOLTAGE1_ADDR(base) ((base) + (0x403E))
#define PMIC_OFFSET_VOLTAGE2_ADDR(base) ((base) + (0x403F))
#define PMIC_OCV_VOLTAGE0_ADDR(base) ((base) + (0x4040))
#define PMIC_OCV_VOLTAGE1_ADDR(base) ((base) + (0x4041))
#define PMIC_OCV_VOLTAGE2_ADDR(base) ((base) + (0x4042))
#define PMIC_OCV_CURRENT0_ADDR(base) ((base) + (0x4043))
#define PMIC_OCV_CURRENT1_ADDR(base) ((base) + (0x4044))
#define PMIC_OCV_CURRENT2_ADDR(base) ((base) + (0x4045))
#define PMIC_OCV_TEMP0_ADDR(base) ((base) + (0x4046))
#define PMIC_OCV_TEMP1_ADDR(base) ((base) + (0x4047))
#define PMIC_OCV_TEMP2_ADDR(base) ((base) + (0x4048))
#define PMIC_ECO_OUT_CLIN_0_ADDR(base) ((base) + (0x4049))
#define PMIC_ECO_OUT_CLIN_1_ADDR(base) ((base) + (0x404A))
#define PMIC_ECO_OUT_CLIN_2_ADDR(base) ((base) + (0x404B))
#define PMIC_ECO_OUT_CLIN_3_ADDR(base) ((base) + (0x404C))
#define PMIC_ECO_OUT_CLIN_4_ADDR(base) ((base) + (0x404D))
#define PMIC_ECO_OUT_CLOUT_0_ADDR(base) ((base) + (0x404E))
#define PMIC_ECO_OUT_CLOUT_1_ADDR(base) ((base) + (0x404F))
#define PMIC_ECO_OUT_CLOUT_2_ADDR(base) ((base) + (0x4050))
#define PMIC_ECO_OUT_CLOUT_3_ADDR(base) ((base) + (0x4051))
#define PMIC_ECO_OUT_CLOUT_4_ADDR(base) ((base) + (0x4052))
#define PMIC_ECO_OUT_TEMP_0_ADDR(base) ((base) + (0x4053))
#define PMIC_ECO_OUT_TEMP_1_ADDR(base) ((base) + (0x4054))
#define PMIC_ECO_OUT_TEMP_2_ADDR(base) ((base) + (0x4055))
#define PMIC_TEMP0_RDATA_ADDR(base) ((base) + (0x4056))
#define PMIC_TEMP1_RDATA_ADDR(base) ((base) + (0x4057))
#define PMIC_TEMP2_RDATA_ADDR(base) ((base) + (0x4058))
#define PMIC_V_PRE0_OUT0_ADDR(base) ((base) + (0x4059))
#define PMIC_V_PRE0_OUT1_ADDR(base) ((base) + (0x405A))
#define PMIC_V_PRE0_OUT2_ADDR(base) ((base) + (0x405B))
#define PMIC_V_PRE1_OUT0_ADDR(base) ((base) + (0x405C))
#define PMIC_V_PRE1_OUT1_ADDR(base) ((base) + (0x405D))
#define PMIC_V_PRE1_OUT2_ADDR(base) ((base) + (0x405E))
#define PMIC_V_PRE2_OUT0_ADDR(base) ((base) + (0x405F))
#define PMIC_V_PRE2_OUT1_ADDR(base) ((base) + (0x4060))
#define PMIC_V_PRE2_OUT2_ADDR(base) ((base) + (0x4061))
#define PMIC_V_PRE3_OUT0_ADDR(base) ((base) + (0x4062))
#define PMIC_V_PRE3_OUT1_ADDR(base) ((base) + (0x4063))
#define PMIC_V_PRE3_OUT2_ADDR(base) ((base) + (0x4064))
#define PMIC_V_PRE4_OUT0_ADDR(base) ((base) + (0x4065))
#define PMIC_V_PRE4_OUT1_ADDR(base) ((base) + (0x4066))
#define PMIC_V_PRE4_OUT2_ADDR(base) ((base) + (0x4067))
#define PMIC_V_PRE5_OUT0_ADDR(base) ((base) + (0x4068))
#define PMIC_V_PRE5_OUT1_ADDR(base) ((base) + (0x4069))
#define PMIC_V_PRE5_OUT2_ADDR(base) ((base) + (0x406A))
#define PMIC_V_PRE6_OUT0_ADDR(base) ((base) + (0x406B))
#define PMIC_V_PRE6_OUT1_ADDR(base) ((base) + (0x406C))
#define PMIC_V_PRE6_OUT2_ADDR(base) ((base) + (0x406D))
#define PMIC_V_PRE7_OUT0_ADDR(base) ((base) + (0x406E))
#define PMIC_V_PRE7_OUT1_ADDR(base) ((base) + (0x406F))
#define PMIC_V_PRE7_OUT2_ADDR(base) ((base) + (0x4070))
#define PMIC_V_PRE8_OUT0_ADDR(base) ((base) + (0x4071))
#define PMIC_V_PRE8_OUT1_ADDR(base) ((base) + (0x4072))
#define PMIC_V_PRE8_OUT2_ADDR(base) ((base) + (0x4073))
#define PMIC_V_PRE9_OUT0_ADDR(base) ((base) + (0x4074))
#define PMIC_V_PRE9_OUT1_ADDR(base) ((base) + (0x4075))
#define PMIC_V_PRE9_OUT2_ADDR(base) ((base) + (0x4076))
#define PMIC_CURRENT_PRE0_OUT0_ADDR(base) ((base) + (0x4077))
#define PMIC_CURRENT_PRE0_OUT1_ADDR(base) ((base) + (0x4078))
#define PMIC_CURRENT_PRE0_OUT2_ADDR(base) ((base) + (0x4079))
#define PMIC_CURRENT_PRE1_OUT0_ADDR(base) ((base) + (0x407A))
#define PMIC_CURRENT_PRE1_OUT1_ADDR(base) ((base) + (0x407B))
#define PMIC_CURRENT_PRE1_OUT2_ADDR(base) ((base) + (0x407C))
#define PMIC_CURRENT_PRE2_OUT0_ADDR(base) ((base) + (0x407D))
#define PMIC_CURRENT_PRE2_OUT1_ADDR(base) ((base) + (0x407E))
#define PMIC_CURRENT_PRE2_OUT2_ADDR(base) ((base) + (0x407F))
#define PMIC_CURRENT_PRE3_OUT0_ADDR(base) ((base) + (0x4080))
#define PMIC_CURRENT_PRE3_OUT1_ADDR(base) ((base) + (0x4081))
#define PMIC_CURRENT_PRE3_OUT2_ADDR(base) ((base) + (0x4082))
#define PMIC_CURRENT_PRE4_OUT0_ADDR(base) ((base) + (0x4083))
#define PMIC_CURRENT_PRE4_OUT1_ADDR(base) ((base) + (0x4084))
#define PMIC_CURRENT_PRE4_OUT2_ADDR(base) ((base) + (0x4085))
#define PMIC_CURRENT_PRE5_OUT0_ADDR(base) ((base) + (0x4086))
#define PMIC_CURRENT_PRE5_OUT1_ADDR(base) ((base) + (0x4087))
#define PMIC_CURRENT_PRE5_OUT2_ADDR(base) ((base) + (0x4088))
#define PMIC_CURRENT_PRE6_OUT0_ADDR(base) ((base) + (0x4089))
#define PMIC_CURRENT_PRE6_OUT1_ADDR(base) ((base) + (0x408A))
#define PMIC_CURRENT_PRE6_OUT2_ADDR(base) ((base) + (0x408B))
#define PMIC_CURRENT_PRE7_OUT0_ADDR(base) ((base) + (0x408C))
#define PMIC_CURRENT_PRE7_OUT1_ADDR(base) ((base) + (0x408D))
#define PMIC_CURRENT_PRE7_OUT2_ADDR(base) ((base) + (0x408E))
#define PMIC_CURRENT_PRE8_OUT0_ADDR(base) ((base) + (0x408F))
#define PMIC_CURRENT_PRE8_OUT1_ADDR(base) ((base) + (0x4090))
#define PMIC_CURRENT_PRE8_OUT2_ADDR(base) ((base) + (0x4091))
#define PMIC_CURRENT_PRE9_OUT0_ADDR(base) ((base) + (0x4092))
#define PMIC_CURRENT_PRE9_OUT1_ADDR(base) ((base) + (0x4093))
#define PMIC_CURRENT_PRE9_OUT2_ADDR(base) ((base) + (0x4094))
#define PMIC_V_OCV_PRE1_OUT0_ADDR(base) ((base) + (0x4095))
#define PMIC_V_OCV_PRE1_OUT1_ADDR(base) ((base) + (0x4096))
#define PMIC_V_OCV_PRE1_OUT2_ADDR(base) ((base) + (0x4097))
#define PMIC_V_OCV_PRE2_OUT0_ADDR(base) ((base) + (0x4098))
#define PMIC_V_OCV_PRE2_OUT1_ADDR(base) ((base) + (0x4099))
#define PMIC_V_OCV_PRE2_OUT2_ADDR(base) ((base) + (0x409A))
#define PMIC_V_OCV_PRE3_OUT0_ADDR(base) ((base) + (0x409B))
#define PMIC_V_OCV_PRE3_OUT1_ADDR(base) ((base) + (0x409C))
#define PMIC_V_OCV_PRE3_OUT2_ADDR(base) ((base) + (0x409D))
#define PMIC_V_OCV_PRE4_OUT0_ADDR(base) ((base) + (0x409E))
#define PMIC_V_OCV_PRE4_OUT1_ADDR(base) ((base) + (0x409F))
#define PMIC_V_OCV_PRE4_OUT2_ADDR(base) ((base) + (0x40A0))
#define PMIC_V_OCV_PRE5_OUT0_ADDR(base) ((base) + (0x40A1))
#define PMIC_V_OCV_PRE5_OUT1_ADDR(base) ((base) + (0x40A2))
#define PMIC_V_OCV_PRE5_OUT2_ADDR(base) ((base) + (0x40A3))
#define PMIC_V_OCV_PRE6_OUT0_ADDR(base) ((base) + (0x40A4))
#define PMIC_V_OCV_PRE6_OUT1_ADDR(base) ((base) + (0x40A5))
#define PMIC_V_OCV_PRE6_OUT2_ADDR(base) ((base) + (0x40A6))
#define PMIC_V_OCV_PRE7_OUT0_ADDR(base) ((base) + (0x40A7))
#define PMIC_V_OCV_PRE7_OUT1_ADDR(base) ((base) + (0x40A8))
#define PMIC_V_OCV_PRE7_OUT2_ADDR(base) ((base) + (0x40A9))
#define PMIC_V_OCV_PRE8_OUT0_ADDR(base) ((base) + (0x40AA))
#define PMIC_V_OCV_PRE8_OUT1_ADDR(base) ((base) + (0x40AB))
#define PMIC_V_OCV_PRE8_OUT2_ADDR(base) ((base) + (0x40AC))
#define PMIC_I_OCV_PRE1_OUT0_ADDR(base) ((base) + (0x40AD))
#define PMIC_I_OCV_PRE1_OUT1_ADDR(base) ((base) + (0x40AE))
#define PMIC_I_OCV_PRE1_OUT2_ADDR(base) ((base) + (0x40AF))
#define PMIC_I_OCV_PRE2_OUT0_ADDR(base) ((base) + (0x40B0))
#define PMIC_I_OCV_PRE2_OUT1_ADDR(base) ((base) + (0x40B1))
#define PMIC_I_OCV_PRE2_OUT2_ADDR(base) ((base) + (0x40B2))
#define PMIC_I_OCV_PRE3_OUT0_ADDR(base) ((base) + (0x40B3))
#define PMIC_I_OCV_PRE3_OUT1_ADDR(base) ((base) + (0x40B4))
#define PMIC_I_OCV_PRE3_OUT2_ADDR(base) ((base) + (0x40B5))
#define PMIC_I_OCV_PRE4_OUT0_ADDR(base) ((base) + (0x40B6))
#define PMIC_I_OCV_PRE4_OUT1_ADDR(base) ((base) + (0x40B7))
#define PMIC_I_OCV_PRE4_OUT2_ADDR(base) ((base) + (0x40B8))
#define PMIC_I_OCV_PRE5_OUT0_ADDR(base) ((base) + (0x40B9))
#define PMIC_I_OCV_PRE5_OUT1_ADDR(base) ((base) + (0x40BA))
#define PMIC_I_OCV_PRE5_OUT2_ADDR(base) ((base) + (0x40BB))
#define PMIC_I_OCV_PRE6_OUT0_ADDR(base) ((base) + (0x40BC))
#define PMIC_I_OCV_PRE6_OUT1_ADDR(base) ((base) + (0x40BD))
#define PMIC_I_OCV_PRE6_OUT2_ADDR(base) ((base) + (0x40BE))
#define PMIC_I_OCV_PRE7_OUT0_ADDR(base) ((base) + (0x40BF))
#define PMIC_I_OCV_PRE7_OUT1_ADDR(base) ((base) + (0x40C0))
#define PMIC_I_OCV_PRE7_OUT2_ADDR(base) ((base) + (0x40C1))
#define PMIC_I_OCV_PRE8_OUT0_ADDR(base) ((base) + (0x40C2))
#define PMIC_I_OCV_PRE8_OUT1_ADDR(base) ((base) + (0x40C3))
#define PMIC_I_OCV_PRE8_OUT2_ADDR(base) ((base) + (0x40C4))
#define PMIC_T_OCV_PRE1_OUT0_ADDR(base) ((base) + (0x40C5))
#define PMIC_T_OCV_PRE1_OUT1_ADDR(base) ((base) + (0x40C6))
#define PMIC_T_OCV_PRE1_OUT2_ADDR(base) ((base) + (0x40C7))
#define PMIC_T_OCV_PRE2_OUT0_ADDR(base) ((base) + (0x40C8))
#define PMIC_T_OCV_PRE2_OUT1_ADDR(base) ((base) + (0x40C9))
#define PMIC_T_OCV_PRE2_OUT2_ADDR(base) ((base) + (0x40CA))
#define PMIC_T_OCV_PRE3_OUT0_ADDR(base) ((base) + (0x40CB))
#define PMIC_T_OCV_PRE3_OUT1_ADDR(base) ((base) + (0x40CC))
#define PMIC_T_OCV_PRE3_OUT2_ADDR(base) ((base) + (0x40CD))
#define PMIC_T_OCV_PRE4_OUT0_ADDR(base) ((base) + (0x40CE))
#define PMIC_T_OCV_PRE4_OUT1_ADDR(base) ((base) + (0x40CF))
#define PMIC_T_OCV_PRE4_OUT2_ADDR(base) ((base) + (0x40D0))
#define PMIC_T_OCV_PRE5_OUT0_ADDR(base) ((base) + (0x40D1))
#define PMIC_T_OCV_PRE5_OUT1_ADDR(base) ((base) + (0x40D2))
#define PMIC_T_OCV_PRE5_OUT2_ADDR(base) ((base) + (0x40D3))
#define PMIC_T_OCV_PRE6_OUT0_ADDR(base) ((base) + (0x40D4))
#define PMIC_T_OCV_PRE6_OUT1_ADDR(base) ((base) + (0x40D5))
#define PMIC_T_OCV_PRE6_OUT2_ADDR(base) ((base) + (0x40D6))
#define PMIC_T_OCV_PRE7_OUT0_ADDR(base) ((base) + (0x40D7))
#define PMIC_T_OCV_PRE7_OUT1_ADDR(base) ((base) + (0x40D8))
#define PMIC_T_OCV_PRE7_OUT2_ADDR(base) ((base) + (0x40D9))
#define PMIC_T_OCV_PRE8_OUT0_ADDR(base) ((base) + (0x40DA))
#define PMIC_T_OCV_PRE8_OUT1_ADDR(base) ((base) + (0x40DB))
#define PMIC_T_OCV_PRE8_OUT2_ADDR(base) ((base) + (0x40DC))
#define PMIC_RTC_OCV_PRE1_OUT0_ADDR(base) ((base) + (0x40DD))
#define PMIC_RTC_OCV_PRE1_OUT1_ADDR(base) ((base) + (0x40DE))
#define PMIC_RTC_OCV_PRE1_OUT2_ADDR(base) ((base) + (0x40DF))
#define PMIC_RTC_OCV_PRE1_OUT3_ADDR(base) ((base) + (0x40E0))
#define PMIC_RTC_OCV_PRE2_OUT0_ADDR(base) ((base) + (0x40E1))
#define PMIC_RTC_OCV_PRE2_OUT1_ADDR(base) ((base) + (0x40E2))
#define PMIC_RTC_OCV_PRE2_OUT2_ADDR(base) ((base) + (0x40E3))
#define PMIC_RTC_OCV_PRE2_OUT3_ADDR(base) ((base) + (0x40E4))
#define PMIC_RTC_OCV_PRE3_OUT0_ADDR(base) ((base) + (0x40E5))
#define PMIC_RTC_OCV_PRE3_OUT1_ADDR(base) ((base) + (0x40E6))
#define PMIC_RTC_OCV_PRE3_OUT2_ADDR(base) ((base) + (0x40E7))
#define PMIC_RTC_OCV_PRE3_OUT3_ADDR(base) ((base) + (0x40E8))
#define PMIC_RTC_OCV_PRE4_OUT0_ADDR(base) ((base) + (0x40E9))
#define PMIC_RTC_OCV_PRE4_OUT1_ADDR(base) ((base) + (0x40EA))
#define PMIC_RTC_OCV_PRE4_OUT2_ADDR(base) ((base) + (0x40EB))
#define PMIC_RTC_OCV_PRE4_OUT3_ADDR(base) ((base) + (0x40EC))
#define PMIC_RTC_OCV_PRE5_OUT0_ADDR(base) ((base) + (0x40ED))
#define PMIC_RTC_OCV_PRE5_OUT1_ADDR(base) ((base) + (0x40EE))
#define PMIC_RTC_OCV_PRE5_OUT2_ADDR(base) ((base) + (0x40EF))
#define PMIC_RTC_OCV_PRE5_OUT3_ADDR(base) ((base) + (0x40F0))
#define PMIC_RTC_OCV_PRE6_OUT0_ADDR(base) ((base) + (0x40F1))
#define PMIC_RTC_OCV_PRE6_OUT1_ADDR(base) ((base) + (0x40F2))
#define PMIC_RTC_OCV_PRE6_OUT2_ADDR(base) ((base) + (0x40F3))
#define PMIC_RTC_OCV_PRE6_OUT3_ADDR(base) ((base) + (0x40F4))
#define PMIC_RTC_OCV_PRE7_OUT0_ADDR(base) ((base) + (0x40F5))
#define PMIC_RTC_OCV_PRE7_OUT1_ADDR(base) ((base) + (0x40F6))
#define PMIC_RTC_OCV_PRE7_OUT2_ADDR(base) ((base) + (0x40F7))
#define PMIC_RTC_OCV_PRE7_OUT3_ADDR(base) ((base) + (0x40F8))
#define PMIC_RTC_OCV_PRE8_OUT0_ADDR(base) ((base) + (0x40F9))
#define PMIC_RTC_OCV_PRE8_OUT1_ADDR(base) ((base) + (0x40FA))
#define PMIC_RTC_OCV_PRE8_OUT2_ADDR(base) ((base) + (0x40FB))
#define PMIC_RTC_OCV_PRE8_OUT3_ADDR(base) ((base) + (0x40FC))
#define PMIC_OFTV_OCV_PRE1_OUT0_ADDR(base) ((base) + (0x40FD))
#define PMIC_OFTV_OCV_PRE1_OUT1_ADDR(base) ((base) + (0x40FE))
#define PMIC_OFTV_OCV_PRE1_OUT2_ADDR(base) ((base) + (0x40FF))
#define PMIC_OFTV_OCV_PRE2_OUT0_ADDR(base) ((base) + (0x4100))
#define PMIC_OFTV_OCV_PRE2_OUT1_ADDR(base) ((base) + (0x4101))
#define PMIC_OFTV_OCV_PRE2_OUT2_ADDR(base) ((base) + (0x4102))
#define PMIC_OFTV_OCV_PRE3_OUT0_ADDR(base) ((base) + (0x4103))
#define PMIC_OFTV_OCV_PRE3_OUT1_ADDR(base) ((base) + (0x4104))
#define PMIC_OFTV_OCV_PRE3_OUT2_ADDR(base) ((base) + (0x4105))
#define PMIC_OFTV_OCV_PRE4_OUT0_ADDR(base) ((base) + (0x4106))
#define PMIC_OFTV_OCV_PRE4_OUT1_ADDR(base) ((base) + (0x4107))
#define PMIC_OFTV_OCV_PRE4_OUT2_ADDR(base) ((base) + (0x4108))
#define PMIC_OFTV_OCV_PRE5_OUT0_ADDR(base) ((base) + (0x4109))
#define PMIC_OFTV_OCV_PRE5_OUT1_ADDR(base) ((base) + (0x410A))
#define PMIC_OFTV_OCV_PRE5_OUT2_ADDR(base) ((base) + (0x410B))
#define PMIC_OFTV_OCV_PRE6_OUT0_ADDR(base) ((base) + (0x410C))
#define PMIC_OFTV_OCV_PRE6_OUT1_ADDR(base) ((base) + (0x410D))
#define PMIC_OFTV_OCV_PRE6_OUT2_ADDR(base) ((base) + (0x410E))
#define PMIC_OFTV_OCV_PRE7_OUT0_ADDR(base) ((base) + (0x410F))
#define PMIC_OFTV_OCV_PRE7_OUT1_ADDR(base) ((base) + (0x4110))
#define PMIC_OFTV_OCV_PRE7_OUT2_ADDR(base) ((base) + (0x4111))
#define PMIC_OFTV_OCV_PRE8_OUT0_ADDR(base) ((base) + (0x4112))
#define PMIC_OFTV_OCV_PRE8_OUT1_ADDR(base) ((base) + (0x4113))
#define PMIC_OFTV_OCV_PRE8_OUT2_ADDR(base) ((base) + (0x4114))
#define PMIC_OFTI_OCV_PRE1_OUT0_ADDR(base) ((base) + (0x4115))
#define PMIC_OFTI_OCV_PRE1_OUT1_ADDR(base) ((base) + (0x4116))
#define PMIC_OFTI_OCV_PRE1_OUT2_ADDR(base) ((base) + (0x4117))
#define PMIC_OFTI_OCV_PRE2_OUT0_ADDR(base) ((base) + (0x4118))
#define PMIC_OFTI_OCV_PRE2_OUT1_ADDR(base) ((base) + (0x4119))
#define PMIC_OFTI_OCV_PRE2_OUT2_ADDR(base) ((base) + (0x411A))
#define PMIC_OFTI_OCV_PRE3_OUT0_ADDR(base) ((base) + (0x411B))
#define PMIC_OFTI_OCV_PRE3_OUT1_ADDR(base) ((base) + (0x411C))
#define PMIC_OFTI_OCV_PRE3_OUT2_ADDR(base) ((base) + (0x411D))
#define PMIC_OFTI_OCV_PRE4_OUT0_ADDR(base) ((base) + (0x411E))
#define PMIC_OFTI_OCV_PRE4_OUT1_ADDR(base) ((base) + (0x411F))
#define PMIC_OFTI_OCV_PRE4_OUT2_ADDR(base) ((base) + (0x4120))
#define PMIC_OFTI_OCV_PRE5_OUT0_ADDR(base) ((base) + (0x4121))
#define PMIC_OFTI_OCV_PRE5_OUT1_ADDR(base) ((base) + (0x4122))
#define PMIC_OFTI_OCV_PRE5_OUT2_ADDR(base) ((base) + (0x4123))
#define PMIC_OFTI_OCV_PRE6_OUT0_ADDR(base) ((base) + (0x4124))
#define PMIC_OFTI_OCV_PRE6_OUT1_ADDR(base) ((base) + (0x4125))
#define PMIC_OFTI_OCV_PRE6_OUT2_ADDR(base) ((base) + (0x4126))
#define PMIC_OFTI_OCV_PRE7_OUT0_ADDR(base) ((base) + (0x4127))
#define PMIC_OFTI_OCV_PRE7_OUT1_ADDR(base) ((base) + (0x4128))
#define PMIC_OFTI_OCV_PRE7_OUT2_ADDR(base) ((base) + (0x4129))
#define PMIC_OFTI_OCV_PRE8_OUT0_ADDR(base) ((base) + (0x412A))
#define PMIC_OFTI_OCV_PRE8_OUT1_ADDR(base) ((base) + (0x412B))
#define PMIC_OFTI_OCV_PRE8_OUT2_ADDR(base) ((base) + (0x412C))
#define PMIC_DCR_V_PRE0_OUT0_ADDR(base) ((base) + (0x412D))
#define PMIC_DCR_V_PRE0_OUT1_ADDR(base) ((base) + (0x412E))
#define PMIC_DCR_V_PRE0_OUT2_ADDR(base) ((base) + (0x412F))
#define PMIC_DCR_V_PRE1_OUT0_ADDR(base) ((base) + (0x4130))
#define PMIC_DCR_V_PRE1_OUT1_ADDR(base) ((base) + (0x4131))
#define PMIC_DCR_V_PRE1_OUT2_ADDR(base) ((base) + (0x4132))
#define PMIC_DCR_V_PRE2_OUT0_ADDR(base) ((base) + (0x4133))
#define PMIC_DCR_V_PRE2_OUT1_ADDR(base) ((base) + (0x4134))
#define PMIC_DCR_V_PRE2_OUT2_ADDR(base) ((base) + (0x4135))
#define PMIC_DCR_V_PRE3_OUT0_ADDR(base) ((base) + (0x4136))
#define PMIC_DCR_V_PRE3_OUT1_ADDR(base) ((base) + (0x4137))
#define PMIC_DCR_V_PRE3_OUT2_ADDR(base) ((base) + (0x4138))
#define PMIC_DCR_V_PRE4_OUT0_ADDR(base) ((base) + (0x4139))
#define PMIC_DCR_V_PRE4_OUT1_ADDR(base) ((base) + (0x413A))
#define PMIC_DCR_V_PRE4_OUT2_ADDR(base) ((base) + (0x413B))
#define PMIC_DCR_V_PRE5_OUT0_ADDR(base) ((base) + (0x413C))
#define PMIC_DCR_V_PRE5_OUT1_ADDR(base) ((base) + (0x413D))
#define PMIC_DCR_V_PRE5_OUT2_ADDR(base) ((base) + (0x413E))
#define PMIC_DCR_V_PRE6_OUT0_ADDR(base) ((base) + (0x413F))
#define PMIC_DCR_V_PRE6_OUT1_ADDR(base) ((base) + (0x4140))
#define PMIC_DCR_V_PRE6_OUT2_ADDR(base) ((base) + (0x4141))
#define PMIC_DCR_V_PRE7_OUT0_ADDR(base) ((base) + (0x4142))
#define PMIC_DCR_V_PRE7_OUT1_ADDR(base) ((base) + (0x4143))
#define PMIC_DCR_V_PRE7_OUT2_ADDR(base) ((base) + (0x4144))
#define PMIC_DCR_V_PRE8_OUT0_ADDR(base) ((base) + (0x4145))
#define PMIC_DCR_V_PRE8_OUT1_ADDR(base) ((base) + (0x4146))
#define PMIC_DCR_V_PRE8_OUT2_ADDR(base) ((base) + (0x4147))
#define PMIC_DCR_V_PRE9_OUT0_ADDR(base) ((base) + (0x4148))
#define PMIC_DCR_V_PRE9_OUT1_ADDR(base) ((base) + (0x4149))
#define PMIC_DCR_V_PRE9_OUT2_ADDR(base) ((base) + (0x414A))
#define PMIC_DCR_I_PRE0_OUT0_ADDR(base) ((base) + (0x414B))
#define PMIC_DCR_I_PRE0_OUT1_ADDR(base) ((base) + (0x414C))
#define PMIC_DCR_I_PRE0_OUT2_ADDR(base) ((base) + (0x414D))
#define PMIC_DCR_I_PRE1_OUT0_ADDR(base) ((base) + (0x414E))
#define PMIC_DCR_I_PRE1_OUT1_ADDR(base) ((base) + (0x414F))
#define PMIC_DCR_I_PRE1_OUT2_ADDR(base) ((base) + (0x4150))
#define PMIC_DCR_I_PRE2_OUT0_ADDR(base) ((base) + (0x4151))
#define PMIC_DCR_I_PRE2_OUT1_ADDR(base) ((base) + (0x4152))
#define PMIC_DCR_I_PRE2_OUT2_ADDR(base) ((base) + (0x4153))
#define PMIC_DCR_I_PRE3_OUT0_ADDR(base) ((base) + (0x4154))
#define PMIC_DCR_I_PRE3_OUT1_ADDR(base) ((base) + (0x4155))
#define PMIC_DCR_I_PRE3_OUT2_ADDR(base) ((base) + (0x4156))
#define PMIC_DCR_I_PRE4_OUT0_ADDR(base) ((base) + (0x4157))
#define PMIC_DCR_I_PRE4_OUT1_ADDR(base) ((base) + (0x4158))
#define PMIC_DCR_I_PRE4_OUT2_ADDR(base) ((base) + (0x4159))
#define PMIC_DCR_I_PRE5_OUT0_ADDR(base) ((base) + (0x415A))
#define PMIC_DCR_I_PRE5_OUT1_ADDR(base) ((base) + (0x415B))
#define PMIC_DCR_I_PRE5_OUT2_ADDR(base) ((base) + (0x415C))
#define PMIC_DCR_I_PRE6_OUT0_ADDR(base) ((base) + (0x415D))
#define PMIC_DCR_I_PRE6_OUT1_ADDR(base) ((base) + (0x415E))
#define PMIC_DCR_I_PRE6_OUT2_ADDR(base) ((base) + (0x415F))
#define PMIC_DCR_I_PRE7_OUT0_ADDR(base) ((base) + (0x4160))
#define PMIC_DCR_I_PRE7_OUT1_ADDR(base) ((base) + (0x4161))
#define PMIC_DCR_I_PRE7_OUT2_ADDR(base) ((base) + (0x4162))
#define PMIC_DCR_I_PRE8_OUT0_ADDR(base) ((base) + (0x4163))
#define PMIC_DCR_I_PRE8_OUT1_ADDR(base) ((base) + (0x4164))
#define PMIC_DCR_I_PRE8_OUT2_ADDR(base) ((base) + (0x4165))
#define PMIC_DCR_I_PRE9_OUT0_ADDR(base) ((base) + (0x4166))
#define PMIC_DCR_I_PRE9_OUT1_ADDR(base) ((base) + (0x4167))
#define PMIC_DCR_I_PRE9_OUT2_ADDR(base) ((base) + (0x4168))
#define PMIC_COUL_RESERVE0_ADDR(base) ((base) + (0x4169))
#define PMIC_COUL_RESERVE1_ADDR(base) ((base) + (0x416A))
#define PMIC_CLJ_DEBUG0_ADDR(base) ((base) + (0x416B))
#define PMIC_CLJ_DEBUG1_ADDR(base) ((base) + (0x416C))
#define PMIC_DEBUG_CIC_I0_ADDR(base) ((base) + (0x416D))
#define PMIC_DEBUG_CIC_I1_ADDR(base) ((base) + (0x416E))
#define PMIC_DEBUG_CIC_I2_ADDR(base) ((base) + (0x416F))
#define PMIC_DEBUG_CIC_V0_ADDR(base) ((base) + (0x4170))
#define PMIC_DEBUG_CIC_V1_ADDR(base) ((base) + (0x4171))
#define PMIC_DEBUG_CIC_V2_ADDR(base) ((base) + (0x4172))
#define PMIC_DEBUG_WRITE_PRO_ADDR(base) ((base) + (0x4173))
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_RTCDR0_ADDR(base) ((base) + (0x6000UL))
#define PMIC_RTCDR1_ADDR(base) ((base) + (0x6001UL))
#define PMIC_RTCDR2_ADDR(base) ((base) + (0x6002UL))
#define PMIC_RTCDR3_ADDR(base) ((base) + (0x6003UL))
#define PMIC_RTCMR0_ADDR(base) ((base) + (0x6004UL))
#define PMIC_RTCMR1_ADDR(base) ((base) + (0x6005UL))
#define PMIC_RTCMR2_ADDR(base) ((base) + (0x6006UL))
#define PMIC_RTCMR3_ADDR(base) ((base) + (0x6007UL))
#define PMIC_RTCLR0_ADDR(base) ((base) + (0x6008UL))
#define PMIC_RTCLR1_ADDR(base) ((base) + (0x6009UL))
#define PMIC_RTCLR2_ADDR(base) ((base) + (0x600AUL))
#define PMIC_RTCLR3_ADDR(base) ((base) + (0x600BUL))
#define PMIC_RTCCTRL_ADDR(base) ((base) + (0x600CUL))
#define PMIC_CRC_VAULE0_ADDR(base) ((base) + (0x600DUL))
#define PMIC_CRC_VAULE1_ADDR(base) ((base) + (0x600EUL))
#define PMIC_CRC_VAULE2_ADDR(base) ((base) + (0x600FUL))
#define PMIC_RTC_PWRUP_TIMER0_ADDR(base) ((base) + (0x6010UL))
#define PMIC_RTC_PWRUP_TIMER1_ADDR(base) ((base) + (0x6011UL))
#define PMIC_RTC_PWRUP_TIMER2_ADDR(base) ((base) + (0x6012UL))
#define PMIC_RTC_PWRUP_TIMER3_ADDR(base) ((base) + (0x6013UL))
#define PMIC_RTC_PWRDOWN_TIMER0_ADDR(base) ((base) + (0x6014UL))
#define PMIC_RTC_PWRDOWN_TIMER1_ADDR(base) ((base) + (0x6015UL))
#define PMIC_RTC_PWRDOWN_TIMER2_ADDR(base) ((base) + (0x6016UL))
#define PMIC_RTC_PWRDOWN_TIMER3_ADDR(base) ((base) + (0x6017UL))
#define PMIC_SER_RTCDR0_ADDR(base) ((base) + (0x6020UL))
#define PMIC_SER_RTCDR1_ADDR(base) ((base) + (0x6021UL))
#define PMIC_SER_RTCDR2_ADDR(base) ((base) + (0x6022UL))
#define PMIC_SER_RTCDR3_ADDR(base) ((base) + (0x6023UL))
#define PMIC_SER_RTCMR0_ADDR(base) ((base) + (0x6024UL))
#define PMIC_SER_RTCMR1_ADDR(base) ((base) + (0x6025UL))
#define PMIC_SER_RTCMR2_ADDR(base) ((base) + (0x6026UL))
#define PMIC_SER_RTCMR3_ADDR(base) ((base) + (0x6027UL))
#define PMIC_SER_RTCLR0_ADDR(base) ((base) + (0x0368UL))
#define PMIC_SER_RTCLR1_ADDR(base) ((base) + (0x0369UL))
#define PMIC_SER_RTCLR2_ADDR(base) ((base) + (0x036AUL))
#define PMIC_SER_RTCLR3_ADDR(base) ((base) + (0x036BUL))
#define PMIC_SER_RTCCTRL_ADDR(base) ((base) + (0x036CUL))
#define PMIC_SER_XO_THRESOLD0_ADDR(base) ((base) + (0x036DUL))
#define PMIC_SER_XO_THRESOLD1_ADDR(base) ((base) + (0x036EUL))
#define PMIC_SER_CRC_VAULE0_ADDR(base) ((base) + (0x036FUL))
#define PMIC_SER_CRC_VAULE1_ADDR(base) ((base) + (0x0370UL))
#define PMIC_SER_CRC_VAULE2_ADDR(base) ((base) + (0x0371UL))
#define PMIC_SER_RTC_PWRUP_TIMER0_ADDR(base) ((base) + (0x6032UL))
#define PMIC_SER_RTC_PWRUP_TIMER1_ADDR(base) ((base) + (0x6033UL))
#define PMIC_SER_RTC_PWRUP_TIMER2_ADDR(base) ((base) + (0x6034UL))
#define PMIC_SER_RTC_PWRUP_TIMER3_ADDR(base) ((base) + (0x6035UL))
#define PMIC_SER_RTC_PWRDOWN_TIMER0_ADDR(base) ((base) + (0x6036UL))
#define PMIC_SER_RTC_PWRDOWN_TIMER1_ADDR(base) ((base) + (0x6037UL))
#define PMIC_SER_RTC_PWRDOWN_TIMER2_ADDR(base) ((base) + (0x6038UL))
#define PMIC_SER_RTC_PWRDOWN_TIMER3_ADDR(base) ((base) + (0x6039UL))
#else
#define PMIC_RTCDR0_ADDR(base) ((base) + (0x6000))
#define PMIC_RTCDR1_ADDR(base) ((base) + (0x6001))
#define PMIC_RTCDR2_ADDR(base) ((base) + (0x6002))
#define PMIC_RTCDR3_ADDR(base) ((base) + (0x6003))
#define PMIC_RTCMR0_ADDR(base) ((base) + (0x6004))
#define PMIC_RTCMR1_ADDR(base) ((base) + (0x6005))
#define PMIC_RTCMR2_ADDR(base) ((base) + (0x6006))
#define PMIC_RTCMR3_ADDR(base) ((base) + (0x6007))
#define PMIC_RTCLR0_ADDR(base) ((base) + (0x6008))
#define PMIC_RTCLR1_ADDR(base) ((base) + (0x6009))
#define PMIC_RTCLR2_ADDR(base) ((base) + (0x600A))
#define PMIC_RTCLR3_ADDR(base) ((base) + (0x600B))
#define PMIC_RTCCTRL_ADDR(base) ((base) + (0x600C))
#define PMIC_CRC_VAULE0_ADDR(base) ((base) + (0x600D))
#define PMIC_CRC_VAULE1_ADDR(base) ((base) + (0x600E))
#define PMIC_CRC_VAULE2_ADDR(base) ((base) + (0x600F))
#define PMIC_RTC_PWRUP_TIMER0_ADDR(base) ((base) + (0x6010))
#define PMIC_RTC_PWRUP_TIMER1_ADDR(base) ((base) + (0x6011))
#define PMIC_RTC_PWRUP_TIMER2_ADDR(base) ((base) + (0x6012))
#define PMIC_RTC_PWRUP_TIMER3_ADDR(base) ((base) + (0x6013))
#define PMIC_RTC_PWRDOWN_TIMER0_ADDR(base) ((base) + (0x6014))
#define PMIC_RTC_PWRDOWN_TIMER1_ADDR(base) ((base) + (0x6015))
#define PMIC_RTC_PWRDOWN_TIMER2_ADDR(base) ((base) + (0x6016))
#define PMIC_RTC_PWRDOWN_TIMER3_ADDR(base) ((base) + (0x6017))
#define PMIC_SER_RTCDR0_ADDR(base) ((base) + (0x6020))
#define PMIC_SER_RTCDR1_ADDR(base) ((base) + (0x6021))
#define PMIC_SER_RTCDR2_ADDR(base) ((base) + (0x6022))
#define PMIC_SER_RTCDR3_ADDR(base) ((base) + (0x6023))
#define PMIC_SER_RTCMR0_ADDR(base) ((base) + (0x6024))
#define PMIC_SER_RTCMR1_ADDR(base) ((base) + (0x6025))
#define PMIC_SER_RTCMR2_ADDR(base) ((base) + (0x6026))
#define PMIC_SER_RTCMR3_ADDR(base) ((base) + (0x6027))
#define PMIC_SER_RTCLR0_ADDR(base) ((base) + (0x0368))
#define PMIC_SER_RTCLR1_ADDR(base) ((base) + (0x0369))
#define PMIC_SER_RTCLR2_ADDR(base) ((base) + (0x036A))
#define PMIC_SER_RTCLR3_ADDR(base) ((base) + (0x036B))
#define PMIC_SER_RTCCTRL_ADDR(base) ((base) + (0x036C))
#define PMIC_SER_XO_THRESOLD0_ADDR(base) ((base) + (0x036D))
#define PMIC_SER_XO_THRESOLD1_ADDR(base) ((base) + (0x036E))
#define PMIC_SER_CRC_VAULE0_ADDR(base) ((base) + (0x036F))
#define PMIC_SER_CRC_VAULE1_ADDR(base) ((base) + (0x0370))
#define PMIC_SER_CRC_VAULE2_ADDR(base) ((base) + (0x0371))
#define PMIC_SER_RTC_PWRUP_TIMER0_ADDR(base) ((base) + (0x6032))
#define PMIC_SER_RTC_PWRUP_TIMER1_ADDR(base) ((base) + (0x6033))
#define PMIC_SER_RTC_PWRUP_TIMER2_ADDR(base) ((base) + (0x6034))
#define PMIC_SER_RTC_PWRUP_TIMER3_ADDR(base) ((base) + (0x6035))
#define PMIC_SER_RTC_PWRDOWN_TIMER0_ADDR(base) ((base) + (0x6036))
#define PMIC_SER_RTC_PWRDOWN_TIMER1_ADDR(base) ((base) + (0x6037))
#define PMIC_SER_RTC_PWRDOWN_TIMER2_ADDR(base) ((base) + (0x6038))
#define PMIC_SER_RTC_PWRDOWN_TIMER3_ADDR(base) ((base) + (0x6039))
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_ACR_CFG0_ADDR(base) ((base) + (0x8000UL))
#define PMIC_ACR_CFG1_ADDR(base) ((base) + (0x8001UL))
#define PMIC_ACR_RESERVE_CFG_ADDR(base) ((base) + (0x8002UL))
#define PMIC_ACRADC_CTRL_ADDR(base) ((base) + (0x8005UL))
#define PMIC_ACRADC_START_ADDR(base) ((base) + (0x8006UL))
#define PMIC_ACRCONV_STATUS_ADDR(base) ((base) + (0x8007UL))
#define PMIC_ACRADC_DATA1_ADDR(base) ((base) + (0x8008UL))
#define PMIC_ACRADC_DATA0_ADDR(base) ((base) + (0x8009UL))
#define PMIC_ACRADC_CONV_ADDR(base) ((base) + (0x800AUL))
#define PMIC_ACRADC_CURRENT_ADDR(base) ((base) + (0x800BUL))
#define PMIC_ACRADC_CALI_CTRL_ADDR(base) ((base) + (0x800CUL))
#define PMIC_ACRADC_CALI_VALUE_ADDR(base) ((base) + (0x800DUL))
#define PMIC_ACRADC_CALI_CFG_ADDR(base) ((base) + (0x800EUL))
#define PMIC_ACRADC_MODE_CFG_ADDR(base) ((base) + (0x800FUL))
#define PMIC_ACRADC_CHOPPER_1ST_DATA1_ADDR(base) ((base) + (0x8010UL))
#define PMIC_ACRADC_CHOPPER_1ST_DATA2_ADDR(base) ((base) + (0x8011UL))
#define PMIC_ACRADC_CHOPPER_2ND_DATA1_ADDR(base) ((base) + (0x8012UL))
#define PMIC_ACRADC_CHOPPER_2ND_DATA2_ADDR(base) ((base) + (0x8013UL))
#define PMIC_ACRADC_CALIVALUE_CFG1_ADDR(base) ((base) + (0x8014UL))
#define PMIC_ACRADC_CALIVALUE_CFG2_ADDR(base) ((base) + (0x8015UL))
#define PMIC_ACR_EN_ADDR(base) ((base) + (0x801AUL))
#define PMIC_ACR_PULSE_NUM_ADDR(base) ((base) + (0x801BUL))
#define PMIC_ACR_SAMPLE_TIME_H_ADDR(base) ((base) + (0x801CUL))
#define PMIC_ACR_SAMPLE_TIME_L_ADDR(base) ((base) + (0x801DUL))
#define PMIC_ACR_DATA0_H_ADDR(base) ((base) + (0x801EUL))
#define PMIC_ACR_DATA0_L_ADDR(base) ((base) + (0x801FUL))
#define PMIC_ACR_DATA1_H_ADDR(base) ((base) + (0x8020UL))
#define PMIC_ACR_DATA1_L_ADDR(base) ((base) + (0x8021UL))
#define PMIC_ACR_DATA2_H_ADDR(base) ((base) + (0x8022UL))
#define PMIC_ACR_DATA2_L_ADDR(base) ((base) + (0x8023UL))
#define PMIC_ACR_DATA3_H_ADDR(base) ((base) + (0x8024UL))
#define PMIC_ACR_DATA3_L_ADDR(base) ((base) + (0x8025UL))
#define PMIC_ACR_DATA4_H_ADDR(base) ((base) + (0x8026UL))
#define PMIC_ACR_DATA4_L_ADDR(base) ((base) + (0x8027UL))
#define PMIC_ACR_DATA5_H_ADDR(base) ((base) + (0x8028UL))
#define PMIC_ACR_DATA5_L_ADDR(base) ((base) + (0x8029UL))
#define PMIC_ACR_DATA6_H_ADDR(base) ((base) + (0x802AUL))
#define PMIC_ACR_DATA6_L_ADDR(base) ((base) + (0x802BUL))
#define PMIC_ACR_DATA7_H_ADDR(base) ((base) + (0x802CUL))
#define PMIC_ACR_DATA7_L_ADDR(base) ((base) + (0x802DUL))
#define PMIC_SOH_OVP_EN_ADDR(base) ((base) + (0x8033UL))
#define PMIC_SOH_OVH_MASK_ADDR(base) ((base) + (0x8034UL))
#define PMIC_SOHOV_TIME_SOHEN_ADDR(base) ((base) + (0x8035UL))
#define PMIC_EN_SOHOV_ADDR(base) ((base) + (0x8036UL))
#define PMIC_SOH_OVH_TH0_H_ADDR(base) ((base) + (0x8037UL))
#define PMIC_SOH_OVH_TH0_L_ADDR(base) ((base) + (0x8038UL))
#define PMIC_TMP_OVH_TH0_H_ADDR(base) ((base) + (0x8039UL))
#define PMIC_TMP_OVH_TH0_L_ADDR(base) ((base) + (0x803AUL))
#define PMIC_SOH_OVH_TH1_H_ADDR(base) ((base) + (0x803BUL))
#define PMIC_SOH_OVH_TH1_L_ADDR(base) ((base) + (0x803CUL))
#define PMIC_TMP_OVH_TH1_H_ADDR(base) ((base) + (0x803DUL))
#define PMIC_TMP_OVH_TH1_L_ADDR(base) ((base) + (0x803EUL))
#define PMIC_SOH_OVH_TH2_H_ADDR(base) ((base) + (0x803FUL))
#define PMIC_SOH_OVH_TH2_L_ADDR(base) ((base) + (0x8040UL))
#define PMIC_TMP_OVH_TH2_H_ADDR(base) ((base) + (0x8041UL))
#define PMIC_TMP_OVH_TH2_L_ADDR(base) ((base) + (0x8042UL))
#define PMIC_SOH_OVL_TH_H_ADDR(base) ((base) + (0x8043UL))
#define PMIC_SOH_OVL_TH_L_ADDR(base) ((base) + (0x8044UL))
#define PMIC_TMP_OVL_TH_H_ADDR(base) ((base) + (0x8045UL))
#define PMIC_TMP_OVL_TH_L_ADDR(base) ((base) + (0x8046UL))
#define PMIC_SOH_OVP_REAL_ADDR(base) ((base) + (0x8047UL))
#define PMIC_DCR_EN_ADDR(base) ((base) + (0x804BUL))
#define PMIC_DCR_TIMER_ADDR(base) ((base) + (0x804CUL))
#define PMIC_TEST_BUS_SEL_ADDR(base) ((base) + (0x804DUL))
#define PMIC_ACR_TB_BUS_1_ADDR(base) ((base) + (0x8050UL))
#define PMIC_ACR_TB_BUS_0_ADDR(base) ((base) + (0x8051UL))
#define PMIC_ACR_CLK_GT_EN_ADDR(base) ((base) + (0x8052UL))
#else
#define PMIC_ACR_CFG0_ADDR(base) ((base) + (0x8000))
#define PMIC_ACR_CFG1_ADDR(base) ((base) + (0x8001))
#define PMIC_ACR_RESERVE_CFG_ADDR(base) ((base) + (0x8002))
#define PMIC_ACRADC_CTRL_ADDR(base) ((base) + (0x8005))
#define PMIC_ACRADC_START_ADDR(base) ((base) + (0x8006))
#define PMIC_ACRCONV_STATUS_ADDR(base) ((base) + (0x8007))
#define PMIC_ACRADC_DATA1_ADDR(base) ((base) + (0x8008))
#define PMIC_ACRADC_DATA0_ADDR(base) ((base) + (0x8009))
#define PMIC_ACRADC_CONV_ADDR(base) ((base) + (0x800A))
#define PMIC_ACRADC_CURRENT_ADDR(base) ((base) + (0x800B))
#define PMIC_ACRADC_CALI_CTRL_ADDR(base) ((base) + (0x800C))
#define PMIC_ACRADC_CALI_VALUE_ADDR(base) ((base) + (0x800D))
#define PMIC_ACRADC_CALI_CFG_ADDR(base) ((base) + (0x800E))
#define PMIC_ACRADC_MODE_CFG_ADDR(base) ((base) + (0x800F))
#define PMIC_ACRADC_CHOPPER_1ST_DATA1_ADDR(base) ((base) + (0x8010))
#define PMIC_ACRADC_CHOPPER_1ST_DATA2_ADDR(base) ((base) + (0x8011))
#define PMIC_ACRADC_CHOPPER_2ND_DATA1_ADDR(base) ((base) + (0x8012))
#define PMIC_ACRADC_CHOPPER_2ND_DATA2_ADDR(base) ((base) + (0x8013))
#define PMIC_ACRADC_CALIVALUE_CFG1_ADDR(base) ((base) + (0x8014))
#define PMIC_ACRADC_CALIVALUE_CFG2_ADDR(base) ((base) + (0x8015))
#define PMIC_ACR_EN_ADDR(base) ((base) + (0x801A))
#define PMIC_ACR_PULSE_NUM_ADDR(base) ((base) + (0x801B))
#define PMIC_ACR_SAMPLE_TIME_H_ADDR(base) ((base) + (0x801C))
#define PMIC_ACR_SAMPLE_TIME_L_ADDR(base) ((base) + (0x801D))
#define PMIC_ACR_DATA0_H_ADDR(base) ((base) + (0x801E))
#define PMIC_ACR_DATA0_L_ADDR(base) ((base) + (0x801F))
#define PMIC_ACR_DATA1_H_ADDR(base) ((base) + (0x8020))
#define PMIC_ACR_DATA1_L_ADDR(base) ((base) + (0x8021))
#define PMIC_ACR_DATA2_H_ADDR(base) ((base) + (0x8022))
#define PMIC_ACR_DATA2_L_ADDR(base) ((base) + (0x8023))
#define PMIC_ACR_DATA3_H_ADDR(base) ((base) + (0x8024))
#define PMIC_ACR_DATA3_L_ADDR(base) ((base) + (0x8025))
#define PMIC_ACR_DATA4_H_ADDR(base) ((base) + (0x8026))
#define PMIC_ACR_DATA4_L_ADDR(base) ((base) + (0x8027))
#define PMIC_ACR_DATA5_H_ADDR(base) ((base) + (0x8028))
#define PMIC_ACR_DATA5_L_ADDR(base) ((base) + (0x8029))
#define PMIC_ACR_DATA6_H_ADDR(base) ((base) + (0x802A))
#define PMIC_ACR_DATA6_L_ADDR(base) ((base) + (0x802B))
#define PMIC_ACR_DATA7_H_ADDR(base) ((base) + (0x802C))
#define PMIC_ACR_DATA7_L_ADDR(base) ((base) + (0x802D))
#define PMIC_SOH_OVP_EN_ADDR(base) ((base) + (0x8033))
#define PMIC_SOH_OVH_MASK_ADDR(base) ((base) + (0x8034))
#define PMIC_SOHOV_TIME_SOHEN_ADDR(base) ((base) + (0x8035))
#define PMIC_EN_SOHOV_ADDR(base) ((base) + (0x8036))
#define PMIC_SOH_OVH_TH0_H_ADDR(base) ((base) + (0x8037))
#define PMIC_SOH_OVH_TH0_L_ADDR(base) ((base) + (0x8038))
#define PMIC_TMP_OVH_TH0_H_ADDR(base) ((base) + (0x8039))
#define PMIC_TMP_OVH_TH0_L_ADDR(base) ((base) + (0x803A))
#define PMIC_SOH_OVH_TH1_H_ADDR(base) ((base) + (0x803B))
#define PMIC_SOH_OVH_TH1_L_ADDR(base) ((base) + (0x803C))
#define PMIC_TMP_OVH_TH1_H_ADDR(base) ((base) + (0x803D))
#define PMIC_TMP_OVH_TH1_L_ADDR(base) ((base) + (0x803E))
#define PMIC_SOH_OVH_TH2_H_ADDR(base) ((base) + (0x803F))
#define PMIC_SOH_OVH_TH2_L_ADDR(base) ((base) + (0x8040))
#define PMIC_TMP_OVH_TH2_H_ADDR(base) ((base) + (0x8041))
#define PMIC_TMP_OVH_TH2_L_ADDR(base) ((base) + (0x8042))
#define PMIC_SOH_OVL_TH_H_ADDR(base) ((base) + (0x8043))
#define PMIC_SOH_OVL_TH_L_ADDR(base) ((base) + (0x8044))
#define PMIC_TMP_OVL_TH_H_ADDR(base) ((base) + (0x8045))
#define PMIC_TMP_OVL_TH_L_ADDR(base) ((base) + (0x8046))
#define PMIC_SOH_OVP_REAL_ADDR(base) ((base) + (0x8047))
#define PMIC_DCR_EN_ADDR(base) ((base) + (0x804B))
#define PMIC_DCR_TIMER_ADDR(base) ((base) + (0x804C))
#define PMIC_TEST_BUS_SEL_ADDR(base) ((base) + (0x804D))
#define PMIC_ACR_TB_BUS_1_ADDR(base) ((base) + (0x8050))
#define PMIC_ACR_TB_BUS_0_ADDR(base) ((base) + (0x8051))
#define PMIC_ACR_CLK_GT_EN_ADDR(base) ((base) + (0x8052))
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_LRA_CTRL_ADDR(base) ((base) + (0xA000UL))
#define PMIC_BRAKE_CTRL_ADDR(base) ((base) + (0xA001UL))
#define PMIC_DRV_H_L_NUM_ADDR(base) ((base) + (0xA002UL))
#define PMIC_BRK_H_NUM_ADDR(base) ((base) + (0xA003UL))
#define PMIC_LRA_CFG0_ADDR(base) ((base) + (0xA004UL))
#define PMIC_LRA_CFG1_ADDR(base) ((base) + (0xA005UL))
#define PMIC_DRV_TIME_CFG_ADDR(base) ((base) + (0xA006UL))
#define PMIC_MAX_TIME_CFG_ADDR(base) ((base) + (0xA007UL))
#define PMIC_BEMF_OUT_CFG_ADDR(base) ((base) + (0xA008UL))
#define PMIC_BEMF_TIME_ADDR(base) ((base) + (0xA009UL))
#define PMIC_DUTY_INIT_CFG_LOW_ADDR(base) ((base) + (0xA00AUL))
#define PMIC_DUTY_INIT_CFG_HIGH_ADDR(base) ((base) + (0xA00BUL))
#define PMIC_DUTY_OVDR_CPS_ADDR(base) ((base) + (0xA00CUL))
#define PMIC_DUTY_OVDR_CFG_LOW_ADDR(base) ((base) + (0xA00DUL))
#define PMIC_DUTY_OVDR_CFG_HIGH_ADDR(base) ((base) + (0xA00EUL))
#define PMIC_DUTY_NORMAL_CPS_ADDR(base) ((base) + (0xA00FUL))
#define PMIC_DUTY_NORMAL_CFG_LOW_ADDR(base) ((base) + (0xA010UL))
#define PMIC_DUTY_NORMAL_CFG_HIGH_ADDR(base) ((base) + (0xA011UL))
#define PMIC_DUTY_BRAKE_CPS_ADDR(base) ((base) + (0xA012UL))
#define PMIC_DUTY_BRAKE_CFG_LOW_ADDR(base) ((base) + (0xA013UL))
#define PMIC_DUTY_BRAKE_CFG_HIGH_ADDR(base) ((base) + (0xA014UL))
#define PMIC_EOB_K_1_LOW_ADDR(base) ((base) + (0xA015UL))
#define PMIC_EOB_K_1_HIGH_ADDR(base) ((base) + (0xA016UL))
#define PMIC_EOB_K_2_LOW_ADDR(base) ((base) + (0xA017UL))
#define PMIC_EOB_K_2_HIGH_ADDR(base) ((base) + (0xA018UL))
#define PMIC_ADC_MAX_MIN_TH_ADDR(base) ((base) + (0xA019UL))
#define PMIC_TH_BOOST_V_CFG_LOW_ADDR(base) ((base) + (0xA01AUL))
#define PMIC_TH_BOOST_V_CFG_HIGH_ADDR(base) ((base) + (0xA01BUL))
#define PMIC_GATE_VOLTAGE1_LOW_ADDR(base) ((base) + (0xA01CUL))
#define PMIC_GATE_VOLTAGE1_HIGH_ADDR(base) ((base) + (0xA01DUL))
#define PMIC_GATE_VOLTAGE2_LOW_ADDR(base) ((base) + (0xA01EUL))
#define PMIC_GATE_VOLTAGE2_HIGH_ADDR(base) ((base) + (0xA01FUL))
#define PMIC_GATE_VOLTAGE3_LOW_ADDR(base) ((base) + (0xA020UL))
#define PMIC_GATE_VOLTAGE3_HIGH_ADDR(base) ((base) + (0xA021UL))
#define PMIC_BRK_BEMF_MIN_CFG_L_ADDR(base) ((base) + (0xA022UL))
#define PMIC_BRK_BEMF_MIN_CFG_H_ADDR(base) ((base) + (0xA023UL))
#define PMIC_REF_U_CFG_L_ADDR(base) ((base) + (0xA024UL))
#define PMIC_REF_U_CFG_H_ADDR(base) ((base) + (0xA025UL))
#define PMIC_KFIT_DATA_SEL_ADDR(base) ((base) + (0xA026UL))
#define PMIC_K_RVS_LOW_ADDR(base) ((base) + (0xA027UL))
#define PMIC_K_RVS_HIGH_ADDR(base) ((base) + (0xA028UL))
#define PMIC_B_RVS_LOW_ADDR(base) ((base) + (0xA029UL))
#define PMIC_B_RVS_HIGH_ADDR(base) ((base) + (0xA02AUL))
#define PMIC_LADD_PARA1_CFG_ADDR(base) ((base) + (0xA02BUL))
#define PMIC_LADD_PARA2_CFG_ADDR(base) ((base) + (0xA02CUL))
#define PMIC_LADD_PARA3_CFG_ADDR(base) ((base) + (0xA02DUL))
#define PMIC_LADD_PARA4_CFG_ADDR(base) ((base) + (0xA02EUL))
#define PMIC_LADD_PARA5_CFG_ADDR(base) ((base) + (0xA02FUL))
#define PMIC_LADD_PARA6_CFG_ADDR(base) ((base) + (0xA030UL))
#define PMIC_LADD_PARA7_CFG_ADDR(base) ((base) + (0xA031UL))
#define PMIC_LADD_PARA8_CFG_ADDR(base) ((base) + (0xA032UL))
#define PMIC_LADD_PARA9_CFG_ADDR(base) ((base) + (0xA033UL))
#define PMIC_LADD_PARA10_CFG_ADDR(base) ((base) + (0xA034UL))
#define PMIC_LADD_PARA11_CFG_ADDR(base) ((base) + (0xA035UL))
#define PMIC_LADD_PARA12_CFG_ADDR(base) ((base) + (0xA036UL))
#define PMIC_LADD_PARA13_CFG_ADDR(base) ((base) + (0xA037UL))
#define PMIC_LADD_PARA14_CFG_ADDR(base) ((base) + (0xA038UL))
#define PMIC_LADD_PARA15_CFG_ADDR(base) ((base) + (0xA039UL))
#define PMIC_LADD_PARA16_CFG_ADDR(base) ((base) + (0xA03AUL))
#define PMIC_LADD_PARA17_CFG_ADDR(base) ((base) + (0xA03BUL))
#define PMIC_LADD_PARA18_CFG_ADDR(base) ((base) + (0xA03CUL))
#define PMIC_LADD_PARA19_CFG_ADDR(base) ((base) + (0xA03DUL))
#define PMIC_LADD_PARA20_CFG_ADDR(base) ((base) + (0xA03EUL))
#define PMIC_OUT_BEMF_VOL_L_ADDR(base) ((base) + (0xA03FUL))
#define PMIC_OUT_BEMF_VOL_H_ADDR(base) ((base) + (0xA040UL))
#define PMIC_OUT_NML_VBEMF_L_ADDR(base) ((base) + (0xA041UL))
#define PMIC_OUT_NML_VBEMF_H_ADDR(base) ((base) + (0xA042UL))
#define PMIC_OV_BRK_HPRIOD_ADDR(base) ((base) + (0xA043UL))
#define PMIC_BRK_HPRIOD_ADDR(base) ((base) + (0xA044UL))
#define PMIC_UREAL_DATA_L_ADDR(base) ((base) + (0xA045UL))
#define PMIC_UREAL_DATA_H_ADDR(base) ((base) + (0xA046UL))
#define PMIC_ADC_CALI_MAX_L_ADDR(base) ((base) + (0xA047UL))
#define PMIC_ADC_CALI_MAX_H_ADDR(base) ((base) + (0xA048UL))
#define PMIC_ADC_CALI_MIN_L_ADDR(base) ((base) + (0xA049UL))
#define PMIC_ADC_CALI_MIN_H_ADDR(base) ((base) + (0xA04AUL))
#define PMIC_ADC_ZERO_L_ADDR(base) ((base) + (0xA04BUL))
#define PMIC_ADC_ZERO_H_ADDR(base) ((base) + (0xA04CUL))
#define PMIC_KFIT_DATA_L_ADDR(base) ((base) + (0xA04DUL))
#define PMIC_KFIT_DATA_H_ADDR(base) ((base) + (0xA04EUL))
#define PMIC_FSM_STATE_ADDR(base) ((base) + (0xA04FUL))
#define PMIC_SPEL_TIME_1_ADDR(base) ((base) + (0xA050UL))
#define PMIC_SPEL_TIME_2_ADDR(base) ((base) + (0xA051UL))
#define PMIC_SPEL_TIME_3_ADDR(base) ((base) + (0xA052UL))
#define PMIC_SPEL_TIME_4_ADDR(base) ((base) + (0xA053UL))
#define PMIC_SPEL_TIME_5_ADDR(base) ((base) + (0xA054UL))
#define PMIC_SPEL_TIME_6_ADDR(base) ((base) + (0xA055UL))
#define PMIC_SPEL_TIME_7_ADDR(base) ((base) + (0xA056UL))
#define PMIC_SPEL_TIME_8_ADDR(base) ((base) + (0xA057UL))
#define PMIC_SPEL_TIME_9_ADDR(base) ((base) + (0xA058UL))
#define PMIC_SPEL_TIME_10_ADDR(base) ((base) + (0xA059UL))
#define PMIC_SPEL_DUTY_CPS_ADDR(base) ((base) + (0xA05AUL))
#define PMIC_SPEL_DUTY_1_L_ADDR(base) ((base) + (0xA05BUL))
#define PMIC_SPEL_DUTY_1_H_ADDR(base) ((base) + (0xA05CUL))
#define PMIC_SPEL_DUTY_2_L_ADDR(base) ((base) + (0xA05DUL))
#define PMIC_SPEL_DUTY_2_H_ADDR(base) ((base) + (0xA05EUL))
#define PMIC_SPEL_DUTY_3_L_ADDR(base) ((base) + (0xA05FUL))
#define PMIC_SPEL_DUTY_3_H_ADDR(base) ((base) + (0xA060UL))
#define PMIC_SPEL_DUTY_4_L_ADDR(base) ((base) + (0xA061UL))
#define PMIC_SPEL_DUTY_4_H_ADDR(base) ((base) + (0xA062UL))
#define PMIC_SPEL_DUTY_5_L_ADDR(base) ((base) + (0xA063UL))
#define PMIC_SPEL_DUTY_5_H_ADDR(base) ((base) + (0xA064UL))
#define PMIC_SPEL_DUTY_6_L_ADDR(base) ((base) + (0xA065UL))
#define PMIC_SPEL_DUTY_6_H_ADDR(base) ((base) + (0xA066UL))
#define PMIC_SPEL_DUTY_7_L_ADDR(base) ((base) + (0xA067UL))
#define PMIC_SPEL_DUTY_7_H_ADDR(base) ((base) + (0xA068UL))
#define PMIC_SPEL_DUTY_8_L_ADDR(base) ((base) + (0xA069UL))
#define PMIC_SPEL_DUTY_8_H_ADDR(base) ((base) + (0xA06AUL))
#define PMIC_SPEL_DUTY_9_L_ADDR(base) ((base) + (0xA06BUL))
#define PMIC_SPEL_DUTY_9_H_ADDR(base) ((base) + (0xA06CUL))
#define PMIC_SPEL_DUTY_10_L_ADDR(base) ((base) + (0xA06DUL))
#define PMIC_SPEL_DUTY_10_H_ADDR(base) ((base) + (0xA06EUL))
#define PMIC_LRA_ANA_CFG0_ADDR(base) ((base) + (0xA06FUL))
#define PMIC_LRA_ANA_CFG1_ADDR(base) ((base) + (0xA070UL))
#define PMIC_LRA_ANA_CFG2_ADDR(base) ((base) + (0xA071UL))
#define PMIC_LRA_ANA_CFG3_ADDR(base) ((base) + (0xA072UL))
#define PMIC_LRA_OCP_CFG_ADDR(base) ((base) + (0xA073UL))
#define PMIC_LRA_IBIAS_SEL_ADDR(base) ((base) + (0xA074UL))
#define PMIC_LRA_OVP_CFG_ADDR(base) ((base) + (0xA075UL))
#define PMIC_LRA_ANA_RESERVED0_ADDR(base) ((base) + (0xA076UL))
#define PMIC_LRA_ANA_RESERVED1_ADDR(base) ((base) + (0xA077UL))
#define PMIC_HPRD_SAMPLE_EN_ADDR(base) ((base) + (0xA078UL))
#define PMIC_HPRD_SAMPLE_VALID_ADDR(base) ((base) + (0xA079UL))
#define PMIC_HPRD_BEMF_MAX_L_ADDR(base) ((base) + (0xA07AUL))
#define PMIC_HPRD_BEMF_MAX_H_ADDR(base) ((base) + (0xA07BUL))
#define PMIC_HPRD_BEMF_MIN_L_ADDR(base) ((base) + (0xA07CUL))
#define PMIC_HPRD_BEMF_MIN_H_ADDR(base) ((base) + (0xA07DUL))
#define PMIC_HPRD_BEMF_FIRST_L_ADDR(base) ((base) + (0xA07EUL))
#define PMIC_HPRD_BEMF_FIRST_H_ADDR(base) ((base) + (0xA07FUL))
#define PMIC_HPRD_BEMF_LAST_L_ADDR(base) ((base) + (0xA080UL))
#define PMIC_HPRD_BEMF_LAST_H_ADDR(base) ((base) + (0xA081UL))
#define PMIC_HP_DR_TIME_ADDR(base) ((base) + (0xA082UL))
#define PMIC_HPRD_NEXT_DR_ADDR(base) ((base) + (0xA083UL))
#define PMIC_HPRD_TIME_L_ADDR(base) ((base) + (0xA084UL))
#define PMIC_HPRD_TIME_H_ADDR(base) ((base) + (0xA085UL))
#define PMIC_HPRD_DUTY_NUM_ADDR(base) ((base) + (0xA086UL))
#define PMIC_HPRD_RANK_NUM_L_ADDR(base) ((base) + (0xA087UL))
#define PMIC_HPRD_RANK_NUM_H_ADDR(base) ((base) + (0xA088UL))
#define PMIC_HPRD_OCP_NUM_ADDR(base) ((base) + (0xA089UL))
#define PMIC_HPRD_FSM_ADDR(base) ((base) + (0xA08AUL))
#define PMIC_LRA_DEBUG_EN_ADDR(base) ((base) + (0xA08BUL))
#define PMIC_DEBUG_ADC_CTRL_ADDR(base) ((base) + (0xA08CUL))
#define PMIC_DEBUG_ADC_VALID_ADDR(base) ((base) + (0xA08DUL))
#define PMIC_DEBUG_ADC_DATA_L_ADDR(base) ((base) + (0xA08EUL))
#define PMIC_DEBUG_ADC_DATA_H_ADDR(base) ((base) + (0xA08FUL))
#define PMIC_LRA_CLK_G_CTRL_ADDR(base) ((base) + (0xA090UL))
#define PMIC_LRA_NML_TIME_L_ADDR(base) ((base) + (0xA091UL))
#define PMIC_LRA_NML_TIME_M_ADDR(base) ((base) + (0xA092UL))
#define PMIC_LRA_NML_TIME_H_ADDR(base) ((base) + (0xA093UL))
#define PMIC_LRA_NML_BEMF_L_ADDR(base) ((base) + (0xA094UL))
#define PMIC_LRA_NML_BEMF_H_ADDR(base) ((base) + (0xA095UL))
#define PMIC_LRA_GATE_CTRL_REG_ADDR(base) ((base) + (0xA096UL))
#define PMIC_LRA_DUTYRES_REG_ADDR(base) ((base) + (0xA097UL))
#define PMIC_LRA_BFST_OK_REG_ADDR(base) ((base) + (0xA098UL))
#define PMIC_LRA_BEMF_FST_REG_LOW_ADDR(base) ((base) + (0xA099UL))
#define PMIC_LRA_BEMF_FST_REG_HIGH_ADDR(base) ((base) + (0xA09AUL))
#else
#define PMIC_LRA_CTRL_ADDR(base) ((base) + (0xA000))
#define PMIC_BRAKE_CTRL_ADDR(base) ((base) + (0xA001))
#define PMIC_DRV_H_L_NUM_ADDR(base) ((base) + (0xA002))
#define PMIC_BRK_H_NUM_ADDR(base) ((base) + (0xA003))
#define PMIC_LRA_CFG0_ADDR(base) ((base) + (0xA004))
#define PMIC_LRA_CFG1_ADDR(base) ((base) + (0xA005))
#define PMIC_DRV_TIME_CFG_ADDR(base) ((base) + (0xA006))
#define PMIC_MAX_TIME_CFG_ADDR(base) ((base) + (0xA007))
#define PMIC_BEMF_OUT_CFG_ADDR(base) ((base) + (0xA008))
#define PMIC_BEMF_TIME_ADDR(base) ((base) + (0xA009))
#define PMIC_DUTY_INIT_CFG_LOW_ADDR(base) ((base) + (0xA00A))
#define PMIC_DUTY_INIT_CFG_HIGH_ADDR(base) ((base) + (0xA00B))
#define PMIC_DUTY_OVDR_CPS_ADDR(base) ((base) + (0xA00C))
#define PMIC_DUTY_OVDR_CFG_LOW_ADDR(base) ((base) + (0xA00D))
#define PMIC_DUTY_OVDR_CFG_HIGH_ADDR(base) ((base) + (0xA00E))
#define PMIC_DUTY_NORMAL_CPS_ADDR(base) ((base) + (0xA00F))
#define PMIC_DUTY_NORMAL_CFG_LOW_ADDR(base) ((base) + (0xA010))
#define PMIC_DUTY_NORMAL_CFG_HIGH_ADDR(base) ((base) + (0xA011))
#define PMIC_DUTY_BRAKE_CPS_ADDR(base) ((base) + (0xA012))
#define PMIC_DUTY_BRAKE_CFG_LOW_ADDR(base) ((base) + (0xA013))
#define PMIC_DUTY_BRAKE_CFG_HIGH_ADDR(base) ((base) + (0xA014))
#define PMIC_EOB_K_1_LOW_ADDR(base) ((base) + (0xA015))
#define PMIC_EOB_K_1_HIGH_ADDR(base) ((base) + (0xA016))
#define PMIC_EOB_K_2_LOW_ADDR(base) ((base) + (0xA017))
#define PMIC_EOB_K_2_HIGH_ADDR(base) ((base) + (0xA018))
#define PMIC_ADC_MAX_MIN_TH_ADDR(base) ((base) + (0xA019))
#define PMIC_TH_BOOST_V_CFG_LOW_ADDR(base) ((base) + (0xA01A))
#define PMIC_TH_BOOST_V_CFG_HIGH_ADDR(base) ((base) + (0xA01B))
#define PMIC_GATE_VOLTAGE1_LOW_ADDR(base) ((base) + (0xA01C))
#define PMIC_GATE_VOLTAGE1_HIGH_ADDR(base) ((base) + (0xA01D))
#define PMIC_GATE_VOLTAGE2_LOW_ADDR(base) ((base) + (0xA01E))
#define PMIC_GATE_VOLTAGE2_HIGH_ADDR(base) ((base) + (0xA01F))
#define PMIC_GATE_VOLTAGE3_LOW_ADDR(base) ((base) + (0xA020))
#define PMIC_GATE_VOLTAGE3_HIGH_ADDR(base) ((base) + (0xA021))
#define PMIC_BRK_BEMF_MIN_CFG_L_ADDR(base) ((base) + (0xA022))
#define PMIC_BRK_BEMF_MIN_CFG_H_ADDR(base) ((base) + (0xA023))
#define PMIC_REF_U_CFG_L_ADDR(base) ((base) + (0xA024))
#define PMIC_REF_U_CFG_H_ADDR(base) ((base) + (0xA025))
#define PMIC_KFIT_DATA_SEL_ADDR(base) ((base) + (0xA026))
#define PMIC_K_RVS_LOW_ADDR(base) ((base) + (0xA027))
#define PMIC_K_RVS_HIGH_ADDR(base) ((base) + (0xA028))
#define PMIC_B_RVS_LOW_ADDR(base) ((base) + (0xA029))
#define PMIC_B_RVS_HIGH_ADDR(base) ((base) + (0xA02A))
#define PMIC_LADD_PARA1_CFG_ADDR(base) ((base) + (0xA02B))
#define PMIC_LADD_PARA2_CFG_ADDR(base) ((base) + (0xA02C))
#define PMIC_LADD_PARA3_CFG_ADDR(base) ((base) + (0xA02D))
#define PMIC_LADD_PARA4_CFG_ADDR(base) ((base) + (0xA02E))
#define PMIC_LADD_PARA5_CFG_ADDR(base) ((base) + (0xA02F))
#define PMIC_LADD_PARA6_CFG_ADDR(base) ((base) + (0xA030))
#define PMIC_LADD_PARA7_CFG_ADDR(base) ((base) + (0xA031))
#define PMIC_LADD_PARA8_CFG_ADDR(base) ((base) + (0xA032))
#define PMIC_LADD_PARA9_CFG_ADDR(base) ((base) + (0xA033))
#define PMIC_LADD_PARA10_CFG_ADDR(base) ((base) + (0xA034))
#define PMIC_LADD_PARA11_CFG_ADDR(base) ((base) + (0xA035))
#define PMIC_LADD_PARA12_CFG_ADDR(base) ((base) + (0xA036))
#define PMIC_LADD_PARA13_CFG_ADDR(base) ((base) + (0xA037))
#define PMIC_LADD_PARA14_CFG_ADDR(base) ((base) + (0xA038))
#define PMIC_LADD_PARA15_CFG_ADDR(base) ((base) + (0xA039))
#define PMIC_LADD_PARA16_CFG_ADDR(base) ((base) + (0xA03A))
#define PMIC_LADD_PARA17_CFG_ADDR(base) ((base) + (0xA03B))
#define PMIC_LADD_PARA18_CFG_ADDR(base) ((base) + (0xA03C))
#define PMIC_LADD_PARA19_CFG_ADDR(base) ((base) + (0xA03D))
#define PMIC_LADD_PARA20_CFG_ADDR(base) ((base) + (0xA03E))
#define PMIC_OUT_BEMF_VOL_L_ADDR(base) ((base) + (0xA03F))
#define PMIC_OUT_BEMF_VOL_H_ADDR(base) ((base) + (0xA040))
#define PMIC_OUT_NML_VBEMF_L_ADDR(base) ((base) + (0xA041))
#define PMIC_OUT_NML_VBEMF_H_ADDR(base) ((base) + (0xA042))
#define PMIC_OV_BRK_HPRIOD_ADDR(base) ((base) + (0xA043))
#define PMIC_BRK_HPRIOD_ADDR(base) ((base) + (0xA044))
#define PMIC_UREAL_DATA_L_ADDR(base) ((base) + (0xA045))
#define PMIC_UREAL_DATA_H_ADDR(base) ((base) + (0xA046))
#define PMIC_ADC_CALI_MAX_L_ADDR(base) ((base) + (0xA047))
#define PMIC_ADC_CALI_MAX_H_ADDR(base) ((base) + (0xA048))
#define PMIC_ADC_CALI_MIN_L_ADDR(base) ((base) + (0xA049))
#define PMIC_ADC_CALI_MIN_H_ADDR(base) ((base) + (0xA04A))
#define PMIC_ADC_ZERO_L_ADDR(base) ((base) + (0xA04B))
#define PMIC_ADC_ZERO_H_ADDR(base) ((base) + (0xA04C))
#define PMIC_KFIT_DATA_L_ADDR(base) ((base) + (0xA04D))
#define PMIC_KFIT_DATA_H_ADDR(base) ((base) + (0xA04E))
#define PMIC_FSM_STATE_ADDR(base) ((base) + (0xA04F))
#define PMIC_SPEL_TIME_1_ADDR(base) ((base) + (0xA050))
#define PMIC_SPEL_TIME_2_ADDR(base) ((base) + (0xA051))
#define PMIC_SPEL_TIME_3_ADDR(base) ((base) + (0xA052))
#define PMIC_SPEL_TIME_4_ADDR(base) ((base) + (0xA053))
#define PMIC_SPEL_TIME_5_ADDR(base) ((base) + (0xA054))
#define PMIC_SPEL_TIME_6_ADDR(base) ((base) + (0xA055))
#define PMIC_SPEL_TIME_7_ADDR(base) ((base) + (0xA056))
#define PMIC_SPEL_TIME_8_ADDR(base) ((base) + (0xA057))
#define PMIC_SPEL_TIME_9_ADDR(base) ((base) + (0xA058))
#define PMIC_SPEL_TIME_10_ADDR(base) ((base) + (0xA059))
#define PMIC_SPEL_DUTY_CPS_ADDR(base) ((base) + (0xA05A))
#define PMIC_SPEL_DUTY_1_L_ADDR(base) ((base) + (0xA05B))
#define PMIC_SPEL_DUTY_1_H_ADDR(base) ((base) + (0xA05C))
#define PMIC_SPEL_DUTY_2_L_ADDR(base) ((base) + (0xA05D))
#define PMIC_SPEL_DUTY_2_H_ADDR(base) ((base) + (0xA05E))
#define PMIC_SPEL_DUTY_3_L_ADDR(base) ((base) + (0xA05F))
#define PMIC_SPEL_DUTY_3_H_ADDR(base) ((base) + (0xA060))
#define PMIC_SPEL_DUTY_4_L_ADDR(base) ((base) + (0xA061))
#define PMIC_SPEL_DUTY_4_H_ADDR(base) ((base) + (0xA062))
#define PMIC_SPEL_DUTY_5_L_ADDR(base) ((base) + (0xA063))
#define PMIC_SPEL_DUTY_5_H_ADDR(base) ((base) + (0xA064))
#define PMIC_SPEL_DUTY_6_L_ADDR(base) ((base) + (0xA065))
#define PMIC_SPEL_DUTY_6_H_ADDR(base) ((base) + (0xA066))
#define PMIC_SPEL_DUTY_7_L_ADDR(base) ((base) + (0xA067))
#define PMIC_SPEL_DUTY_7_H_ADDR(base) ((base) + (0xA068))
#define PMIC_SPEL_DUTY_8_L_ADDR(base) ((base) + (0xA069))
#define PMIC_SPEL_DUTY_8_H_ADDR(base) ((base) + (0xA06A))
#define PMIC_SPEL_DUTY_9_L_ADDR(base) ((base) + (0xA06B))
#define PMIC_SPEL_DUTY_9_H_ADDR(base) ((base) + (0xA06C))
#define PMIC_SPEL_DUTY_10_L_ADDR(base) ((base) + (0xA06D))
#define PMIC_SPEL_DUTY_10_H_ADDR(base) ((base) + (0xA06E))
#define PMIC_LRA_ANA_CFG0_ADDR(base) ((base) + (0xA06F))
#define PMIC_LRA_ANA_CFG1_ADDR(base) ((base) + (0xA070))
#define PMIC_LRA_ANA_CFG2_ADDR(base) ((base) + (0xA071))
#define PMIC_LRA_ANA_CFG3_ADDR(base) ((base) + (0xA072))
#define PMIC_LRA_OCP_CFG_ADDR(base) ((base) + (0xA073))
#define PMIC_LRA_IBIAS_SEL_ADDR(base) ((base) + (0xA074))
#define PMIC_LRA_OVP_CFG_ADDR(base) ((base) + (0xA075))
#define PMIC_LRA_ANA_RESERVED0_ADDR(base) ((base) + (0xA076))
#define PMIC_LRA_ANA_RESERVED1_ADDR(base) ((base) + (0xA077))
#define PMIC_HPRD_SAMPLE_EN_ADDR(base) ((base) + (0xA078))
#define PMIC_HPRD_SAMPLE_VALID_ADDR(base) ((base) + (0xA079))
#define PMIC_HPRD_BEMF_MAX_L_ADDR(base) ((base) + (0xA07A))
#define PMIC_HPRD_BEMF_MAX_H_ADDR(base) ((base) + (0xA07B))
#define PMIC_HPRD_BEMF_MIN_L_ADDR(base) ((base) + (0xA07C))
#define PMIC_HPRD_BEMF_MIN_H_ADDR(base) ((base) + (0xA07D))
#define PMIC_HPRD_BEMF_FIRST_L_ADDR(base) ((base) + (0xA07E))
#define PMIC_HPRD_BEMF_FIRST_H_ADDR(base) ((base) + (0xA07F))
#define PMIC_HPRD_BEMF_LAST_L_ADDR(base) ((base) + (0xA080))
#define PMIC_HPRD_BEMF_LAST_H_ADDR(base) ((base) + (0xA081))
#define PMIC_HP_DR_TIME_ADDR(base) ((base) + (0xA082))
#define PMIC_HPRD_NEXT_DR_ADDR(base) ((base) + (0xA083))
#define PMIC_HPRD_TIME_L_ADDR(base) ((base) + (0xA084))
#define PMIC_HPRD_TIME_H_ADDR(base) ((base) + (0xA085))
#define PMIC_HPRD_DUTY_NUM_ADDR(base) ((base) + (0xA086))
#define PMIC_HPRD_RANK_NUM_L_ADDR(base) ((base) + (0xA087))
#define PMIC_HPRD_RANK_NUM_H_ADDR(base) ((base) + (0xA088))
#define PMIC_HPRD_OCP_NUM_ADDR(base) ((base) + (0xA089))
#define PMIC_HPRD_FSM_ADDR(base) ((base) + (0xA08A))
#define PMIC_LRA_DEBUG_EN_ADDR(base) ((base) + (0xA08B))
#define PMIC_DEBUG_ADC_CTRL_ADDR(base) ((base) + (0xA08C))
#define PMIC_DEBUG_ADC_VALID_ADDR(base) ((base) + (0xA08D))
#define PMIC_DEBUG_ADC_DATA_L_ADDR(base) ((base) + (0xA08E))
#define PMIC_DEBUG_ADC_DATA_H_ADDR(base) ((base) + (0xA08F))
#define PMIC_LRA_CLK_G_CTRL_ADDR(base) ((base) + (0xA090))
#define PMIC_LRA_NML_TIME_L_ADDR(base) ((base) + (0xA091))
#define PMIC_LRA_NML_TIME_M_ADDR(base) ((base) + (0xA092))
#define PMIC_LRA_NML_TIME_H_ADDR(base) ((base) + (0xA093))
#define PMIC_LRA_NML_BEMF_L_ADDR(base) ((base) + (0xA094))
#define PMIC_LRA_NML_BEMF_H_ADDR(base) ((base) + (0xA095))
#define PMIC_LRA_GATE_CTRL_REG_ADDR(base) ((base) + (0xA096))
#define PMIC_LRA_DUTYRES_REG_ADDR(base) ((base) + (0xA097))
#define PMIC_LRA_BFST_OK_REG_ADDR(base) ((base) + (0xA098))
#define PMIC_LRA_BEMF_FST_REG_LOW_ADDR(base) ((base) + (0xA099))
#define PMIC_LRA_BEMF_FST_REG_HIGH_ADDR(base) ((base) + (0xA09A))
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
        unsigned char buck7_ocp_state : 1;
        unsigned char buck6_ocp_state : 1;
        unsigned char buck5_ocp_state : 1;
        unsigned char buck4_ocp_state : 1;
        unsigned char buck3_ocp_state : 1;
        unsigned char buck2_ocp_state : 1;
        unsigned char buck1_ocp_state : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_STATUS2_UNION;
#endif
#define PMIC_STATUS2_buck7_ocp_state_START (0)
#define PMIC_STATUS2_buck7_ocp_state_END (0)
#define PMIC_STATUS2_buck6_ocp_state_START (1)
#define PMIC_STATUS2_buck6_ocp_state_END (1)
#define PMIC_STATUS2_buck5_ocp_state_START (2)
#define PMIC_STATUS2_buck5_ocp_state_END (2)
#define PMIC_STATUS2_buck4_ocp_state_START (3)
#define PMIC_STATUS2_buck4_ocp_state_END (3)
#define PMIC_STATUS2_buck3_ocp_state_START (4)
#define PMIC_STATUS2_buck3_ocp_state_END (4)
#define PMIC_STATUS2_buck2_ocp_state_START (5)
#define PMIC_STATUS2_buck2_ocp_state_END (5)
#define PMIC_STATUS2_buck1_ocp_state_START (6)
#define PMIC_STATUS2_buck1_ocp_state_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck9_ocp_state : 1;
        unsigned char buck8_ocp_state : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_STATUS3_UNION;
#endif
#define PMIC_STATUS3_buck9_ocp_state_START (0)
#define PMIC_STATUS3_buck9_ocp_state_END (0)
#define PMIC_STATUS3_buck8_ocp_state_START (1)
#define PMIC_STATUS3_buck8_ocp_state_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_buck00_en : 1;
        unsigned char st_buck00_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_buck00_eco_en : 1;
        unsigned char st_buck00_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_BUCK00_ONOFF_ECO_UNION;
#endif
#define PMIC_BUCK00_ONOFF_ECO_reg_buck00_en_START (0)
#define PMIC_BUCK00_ONOFF_ECO_reg_buck00_en_END (0)
#define PMIC_BUCK00_ONOFF_ECO_st_buck00_en_START (1)
#define PMIC_BUCK00_ONOFF_ECO_st_buck00_en_END (1)
#define PMIC_BUCK00_ONOFF_ECO_reg_buck00_eco_en_START (4)
#define PMIC_BUCK00_ONOFF_ECO_reg_buck00_eco_en_END (4)
#define PMIC_BUCK00_ONOFF_ECO_st_buck00_eco_en_START (5)
#define PMIC_BUCK00_ONOFF_ECO_st_buck00_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_buck01_en : 1;
        unsigned char st_buck01_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_BUCK01_ONOFF_UNION;
#endif
#define PMIC_BUCK01_ONOFF_reg_buck01_en_START (0)
#define PMIC_BUCK01_ONOFF_reg_buck01_en_END (0)
#define PMIC_BUCK01_ONOFF_st_buck01_en_START (1)
#define PMIC_BUCK01_ONOFF_st_buck01_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_buck02_en : 1;
        unsigned char st_buck02_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_buck02_eco_en : 1;
        unsigned char st_buck02_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_BUCK02_ONOFF_ECO_UNION;
#endif
#define PMIC_BUCK02_ONOFF_ECO_reg_buck02_en_START (0)
#define PMIC_BUCK02_ONOFF_ECO_reg_buck02_en_END (0)
#define PMIC_BUCK02_ONOFF_ECO_st_buck02_en_START (1)
#define PMIC_BUCK02_ONOFF_ECO_st_buck02_en_END (1)
#define PMIC_BUCK02_ONOFF_ECO_reg_buck02_eco_en_START (4)
#define PMIC_BUCK02_ONOFF_ECO_reg_buck02_eco_en_END (4)
#define PMIC_BUCK02_ONOFF_ECO_st_buck02_eco_en_START (5)
#define PMIC_BUCK02_ONOFF_ECO_st_buck02_eco_en_END (5)
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
        unsigned char reg_buck6_en : 1;
        unsigned char st_buck6_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_buck6_eco_en : 1;
        unsigned char st_buck6_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_BUCK6_ONOFF_ECO_UNION;
#endif
#define PMIC_BUCK6_ONOFF_ECO_reg_buck6_en_START (0)
#define PMIC_BUCK6_ONOFF_ECO_reg_buck6_en_END (0)
#define PMIC_BUCK6_ONOFF_ECO_st_buck6_en_START (1)
#define PMIC_BUCK6_ONOFF_ECO_st_buck6_en_END (1)
#define PMIC_BUCK6_ONOFF_ECO_reg_buck6_eco_en_START (4)
#define PMIC_BUCK6_ONOFF_ECO_reg_buck6_eco_en_END (4)
#define PMIC_BUCK6_ONOFF_ECO_st_buck6_eco_en_START (5)
#define PMIC_BUCK6_ONOFF_ECO_st_buck6_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_buck7_en : 1;
        unsigned char st_buck7_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_buck7_eco_en : 1;
        unsigned char st_buck7_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_BUCK7_ONOFF_ECO_UNION;
#endif
#define PMIC_BUCK7_ONOFF_ECO_reg_buck7_en_START (0)
#define PMIC_BUCK7_ONOFF_ECO_reg_buck7_en_END (0)
#define PMIC_BUCK7_ONOFF_ECO_st_buck7_en_START (1)
#define PMIC_BUCK7_ONOFF_ECO_st_buck7_en_END (1)
#define PMIC_BUCK7_ONOFF_ECO_reg_buck7_eco_en_START (4)
#define PMIC_BUCK7_ONOFF_ECO_reg_buck7_eco_en_END (4)
#define PMIC_BUCK7_ONOFF_ECO_st_buck7_eco_en_START (5)
#define PMIC_BUCK7_ONOFF_ECO_st_buck7_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_buck8_en : 1;
        unsigned char st_buck8_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_buck8_eco_en : 1;
        unsigned char st_buck8_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_BUCK8_ONOFF_ECO_UNION;
#endif
#define PMIC_BUCK8_ONOFF_ECO_reg_buck8_en_START (0)
#define PMIC_BUCK8_ONOFF_ECO_reg_buck8_en_END (0)
#define PMIC_BUCK8_ONOFF_ECO_st_buck8_en_START (1)
#define PMIC_BUCK8_ONOFF_ECO_st_buck8_en_END (1)
#define PMIC_BUCK8_ONOFF_ECO_reg_buck8_eco_en_START (4)
#define PMIC_BUCK8_ONOFF_ECO_reg_buck8_eco_en_END (4)
#define PMIC_BUCK8_ONOFF_ECO_st_buck8_eco_en_START (5)
#define PMIC_BUCK8_ONOFF_ECO_st_buck8_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_buck9_en : 1;
        unsigned char st_buck9_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_buck9_eco_en : 1;
        unsigned char st_buck9_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_BUCK9_ONOFF_ECO_UNION;
#endif
#define PMIC_BUCK9_ONOFF_ECO_reg_buck9_en_START (0)
#define PMIC_BUCK9_ONOFF_ECO_reg_buck9_en_END (0)
#define PMIC_BUCK9_ONOFF_ECO_st_buck9_en_START (1)
#define PMIC_BUCK9_ONOFF_ECO_st_buck9_en_END (1)
#define PMIC_BUCK9_ONOFF_ECO_reg_buck9_eco_en_START (4)
#define PMIC_BUCK9_ONOFF_ECO_reg_buck9_eco_en_END (4)
#define PMIC_BUCK9_ONOFF_ECO_st_buck9_eco_en_START (5)
#define PMIC_BUCK9_ONOFF_ECO_st_buck9_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo0_en : 1;
        unsigned char st_ldo0_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo0_eco_en : 1;
        unsigned char st_ldo0_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO0_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO0_ONOFF_ECO_reg_ldo0_en_START (0)
#define PMIC_LDO0_ONOFF_ECO_reg_ldo0_en_END (0)
#define PMIC_LDO0_ONOFF_ECO_st_ldo0_en_START (1)
#define PMIC_LDO0_ONOFF_ECO_st_ldo0_en_END (1)
#define PMIC_LDO0_ONOFF_ECO_reg_ldo0_eco_en_START (4)
#define PMIC_LDO0_ONOFF_ECO_reg_ldo0_eco_en_END (4)
#define PMIC_LDO0_ONOFF_ECO_st_ldo0_eco_en_START (5)
#define PMIC_LDO0_ONOFF_ECO_st_ldo0_eco_en_END (5)
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
        unsigned char reg_ldo2_en_a : 1;
        unsigned char st_ldo2_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO2_ONOFF1_UNION;
#endif
#define PMIC_LDO2_ONOFF1_reg_ldo2_en_a_START (0)
#define PMIC_LDO2_ONOFF1_reg_ldo2_en_a_END (0)
#define PMIC_LDO2_ONOFF1_st_ldo2_en_START (1)
#define PMIC_LDO2_ONOFF1_st_ldo2_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo2_en_b : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_LDO2_ONOFF2_UNION;
#endif
#define PMIC_LDO2_ONOFF2_reg_ldo2_en_b_START (0)
#define PMIC_LDO2_ONOFF2_reg_ldo2_en_b_END (0)
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
        unsigned char reg_ldo7_en : 1;
        unsigned char st_ldo7_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo7_eco_en : 1;
        unsigned char st_ldo7_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO7_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO7_ONOFF_ECO_reg_ldo7_en_START (0)
#define PMIC_LDO7_ONOFF_ECO_reg_ldo7_en_END (0)
#define PMIC_LDO7_ONOFF_ECO_st_ldo7_en_START (1)
#define PMIC_LDO7_ONOFF_ECO_st_ldo7_en_END (1)
#define PMIC_LDO7_ONOFF_ECO_reg_ldo7_eco_en_START (4)
#define PMIC_LDO7_ONOFF_ECO_reg_ldo7_eco_en_END (4)
#define PMIC_LDO7_ONOFF_ECO_st_ldo7_eco_en_START (5)
#define PMIC_LDO7_ONOFF_ECO_st_ldo7_eco_en_END (5)
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
        unsigned char reg_ldo30_en : 1;
        unsigned char st_ldo30_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo30_eco_en : 1;
        unsigned char st_ldo30_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO30_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO30_ONOFF_ECO_reg_ldo30_en_START (0)
#define PMIC_LDO30_ONOFF_ECO_reg_ldo30_en_END (0)
#define PMIC_LDO30_ONOFF_ECO_st_ldo30_en_START (1)
#define PMIC_LDO30_ONOFF_ECO_st_ldo30_en_END (1)
#define PMIC_LDO30_ONOFF_ECO_reg_ldo30_eco_en_START (4)
#define PMIC_LDO30_ONOFF_ECO_reg_ldo30_eco_en_END (4)
#define PMIC_LDO30_ONOFF_ECO_st_ldo30_eco_en_START (5)
#define PMIC_LDO30_ONOFF_ECO_st_ldo30_eco_en_END (5)
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
        unsigned char reg_ldo36_en : 1;
        unsigned char st_ldo36_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo36_eco_en : 1;
        unsigned char st_ldo36_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO36_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO36_ONOFF_ECO_reg_ldo36_en_START (0)
#define PMIC_LDO36_ONOFF_ECO_reg_ldo36_en_END (0)
#define PMIC_LDO36_ONOFF_ECO_st_ldo36_en_START (1)
#define PMIC_LDO36_ONOFF_ECO_st_ldo36_en_END (1)
#define PMIC_LDO36_ONOFF_ECO_reg_ldo36_eco_en_START (4)
#define PMIC_LDO36_ONOFF_ECO_reg_ldo36_eco_en_END (4)
#define PMIC_LDO36_ONOFF_ECO_st_ldo36_eco_en_START (5)
#define PMIC_LDO36_ONOFF_ECO_st_ldo36_eco_en_END (5)
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
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo40_eco_en : 1;
        unsigned char st_ldo40_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO40_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO40_ONOFF_ECO_reg_ldo40_en_START (0)
#define PMIC_LDO40_ONOFF_ECO_reg_ldo40_en_END (0)
#define PMIC_LDO40_ONOFF_ECO_st_ldo40_en_START (1)
#define PMIC_LDO40_ONOFF_ECO_st_ldo40_en_END (1)
#define PMIC_LDO40_ONOFF_ECO_reg_ldo40_eco_en_START (4)
#define PMIC_LDO40_ONOFF_ECO_reg_ldo40_eco_en_END (4)
#define PMIC_LDO40_ONOFF_ECO_st_ldo40_eco_en_START (5)
#define PMIC_LDO40_ONOFF_ECO_st_ldo40_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo41_en : 1;
        unsigned char st_ldo41_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo41_eco_en : 1;
        unsigned char st_ldo41_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO41_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO41_ONOFF_ECO_reg_ldo41_en_START (0)
#define PMIC_LDO41_ONOFF_ECO_reg_ldo41_en_END (0)
#define PMIC_LDO41_ONOFF_ECO_st_ldo41_en_START (1)
#define PMIC_LDO41_ONOFF_ECO_st_ldo41_en_END (1)
#define PMIC_LDO41_ONOFF_ECO_reg_ldo41_eco_en_START (4)
#define PMIC_LDO41_ONOFF_ECO_reg_ldo41_eco_en_END (4)
#define PMIC_LDO41_ONOFF_ECO_st_ldo41_eco_en_START (5)
#define PMIC_LDO41_ONOFF_ECO_st_ldo41_eco_en_END (5)
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
        unsigned char reg_xo_wifi_1181_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_CLK_WIFI_1181_EN_UNION;
#endif
#define PMIC_CLK_WIFI_1181_EN_reg_xo_wifi_1181_en_START (0)
#define PMIC_CLK_WIFI_1181_EN_reg_xo_wifi_1181_en_END (0)
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
        unsigned char buck00_vset : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK00_VSET_UNION;
#endif
#define PMIC_BUCK00_VSET_buck00_vset_START (0)
#define PMIC_BUCK00_VSET_buck00_vset_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_vset_eco : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK00_VSET_ECO_UNION;
#endif
#define PMIC_BUCK00_VSET_ECO_buck00_vset_eco_START (0)
#define PMIC_BUCK00_VSET_ECO_buck00_vset_eco_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_vset : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK02_VSET_UNION;
#endif
#define PMIC_BUCK02_VSET_buck02_vset_START (0)
#define PMIC_BUCK02_VSET_buck02_vset_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_vset_eco : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK02_VSET_ECO_UNION;
#endif
#define PMIC_BUCK02_VSET_ECO_buck02_vset_eco_START (0)
#define PMIC_BUCK02_VSET_ECO_buck02_vset_eco_END (6)
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
        unsigned char buck2_vset : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK2_VSET_UNION;
#endif
#define PMIC_BUCK2_VSET_buck2_vset_START (0)
#define PMIC_BUCK2_VSET_buck2_vset_END (4)
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
        unsigned char buck3_vset : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK3_VSET_UNION;
#endif
#define PMIC_BUCK3_VSET_buck3_vset_START (0)
#define PMIC_BUCK3_VSET_buck3_vset_END (4)
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
        unsigned char buck4_vset : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK4_VSET_UNION;
#endif
#define PMIC_BUCK4_VSET_buck4_vset_START (0)
#define PMIC_BUCK4_VSET_buck4_vset_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_vset : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK5_VSET_UNION;
#endif
#define PMIC_BUCK5_VSET_buck5_vset_START (0)
#define PMIC_BUCK5_VSET_buck5_vset_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck6_vset : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK6_VSET_UNION;
#endif
#define PMIC_BUCK6_VSET_buck6_vset_START (0)
#define PMIC_BUCK6_VSET_buck6_vset_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_vset : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK7_VSET_UNION;
#endif
#define PMIC_BUCK7_VSET_buck7_vset_START (0)
#define PMIC_BUCK7_VSET_buck7_vset_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck8_vset : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK8_VSET_UNION;
#endif
#define PMIC_BUCK8_VSET_buck8_vset_START (0)
#define PMIC_BUCK8_VSET_buck8_vset_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck8_vset_eco : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK8_VSET_ECO_UNION;
#endif
#define PMIC_BUCK8_VSET_ECO_buck8_vset_eco_START (0)
#define PMIC_BUCK8_VSET_ECO_buck8_vset_eco_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck9_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_BUCK9_VSET_UNION;
#endif
#define PMIC_BUCK9_VSET_buck9_vset_START (0)
#define PMIC_BUCK9_VSET_buck9_vset_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck9_vset_eco : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_BUCK9_VSET_ECO_UNION;
#endif
#define PMIC_BUCK9_VSET_ECO_buck9_vset_eco_START (0)
#define PMIC_BUCK9_VSET_ECO_buck9_vset_eco_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo0_vset : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO0_VSET_UNION;
#endif
#define PMIC_LDO0_VSET_ldo0_vset_START (0)
#define PMIC_LDO0_VSET_ldo0_vset_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo0_vset_eco : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO0_VSET_ECO_UNION;
#endif
#define PMIC_LDO0_VSET_ECO_ldo0_vset_eco_START (0)
#define PMIC_LDO0_VSET_ECO_ldo0_vset_eco_END (5)
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
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO2_VSET_UNION;
#endif
#define PMIC_LDO2_VSET_ldo2_vset_START (0)
#define PMIC_LDO2_VSET_ldo2_vset_END (2)
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
        unsigned char ldo5_vset : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO5_VSET_UNION;
#endif
#define PMIC_LDO5_VSET_ldo5_vset_START (0)
#define PMIC_LDO5_VSET_ldo5_vset_END (4)
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
        unsigned char ldo8_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO8_VSET_UNION;
#endif
#define PMIC_LDO8_VSET_ldo8_vset_START (0)
#define PMIC_LDO8_VSET_ldo8_vset_END (3)
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
        unsigned char ldo15_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO15_VSET_UNION;
#endif
#define PMIC_LDO15_VSET_ldo15_vset_START (0)
#define PMIC_LDO15_VSET_ldo15_vset_END (3)
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
        unsigned char ldo18_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO18_VSET_UNION;
#endif
#define PMIC_LDO18_VSET_ldo18_vset_START (0)
#define PMIC_LDO18_VSET_ldo18_vset_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo21_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO21_VSET_UNION;
#endif
#define PMIC_LDO21_VSET_ldo21_vset_START (0)
#define PMIC_LDO21_VSET_ldo21_vset_END (3)
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
        unsigned char ldo25_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO25_VSET_UNION;
#endif
#define PMIC_LDO25_VSET_ldo25_vset_START (0)
#define PMIC_LDO25_VSET_ldo25_vset_END (3)
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
        unsigned char ldo29_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO29_VSET_UNION;
#endif
#define PMIC_LDO29_VSET_ldo29_vset_START (0)
#define PMIC_LDO29_VSET_ldo29_vset_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo30_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO30_VSET_UNION;
#endif
#define PMIC_LDO30_VSET_ldo30_vset_START (0)
#define PMIC_LDO30_VSET_ldo30_vset_END (2)
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
        unsigned char ldo36_vset : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO36_VSET_UNION;
#endif
#define PMIC_LDO36_VSET_ldo36_vset_START (0)
#define PMIC_LDO36_VSET_ldo36_vset_END (4)
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
        unsigned char ldo39_vset_eco : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO39_VSET_ECO_UNION;
#endif
#define PMIC_LDO39_VSET_ECO_ldo39_vset_eco_START (0)
#define PMIC_LDO39_VSET_ECO_ldo39_vset_eco_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo40_vset : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO40_VSET_UNION;
#endif
#define PMIC_LDO40_VSET_ldo40_vset_START (0)
#define PMIC_LDO40_VSET_ldo40_vset_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo41_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO41_VSET_UNION;
#endif
#define PMIC_LDO41_VSET_ldo41_vset_START (0)
#define PMIC_LDO41_VSET_ldo41_vset_END (3)
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
        unsigned char pmud_vset : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_PMUD_VSET_UNION;
#endif
#define PMIC_PMUD_VSET_pmud_vset_START (0)
#define PMIC_PMUD_VSET_pmud_vset_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_pocp_delay_sel : 2;
        unsigned char buck00_dmd_delay_sel : 2;
        unsigned char buck00_dmd_sel : 3;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK00_CTRL0_UNION;
#endif
#define PMIC_BUCK00_CTRL0_buck00_pocp_delay_sel_START (0)
#define PMIC_BUCK00_CTRL0_buck00_pocp_delay_sel_END (1)
#define PMIC_BUCK00_CTRL0_buck00_dmd_delay_sel_START (2)
#define PMIC_BUCK00_CTRL0_buck00_dmd_delay_sel_END (3)
#define PMIC_BUCK00_CTRL0_buck00_dmd_sel_START (4)
#define PMIC_BUCK00_CTRL0_buck00_dmd_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_pocp_set : 3;
        unsigned char buck00_nocp_l_delay_sel : 2;
        unsigned char buck00_nocp_h_delay_sel : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK00_CTRL1_UNION;
#endif
#define PMIC_BUCK00_CTRL1_buck00_pocp_set_START (0)
#define PMIC_BUCK00_CTRL1_buck00_pocp_set_END (2)
#define PMIC_BUCK00_CTRL1_buck00_nocp_l_delay_sel_START (3)
#define PMIC_BUCK00_CTRL1_buck00_nocp_l_delay_sel_END (4)
#define PMIC_BUCK00_CTRL1_buck00_nocp_h_delay_sel_START (5)
#define PMIC_BUCK00_CTRL1_buck00_nocp_h_delay_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_nocp_l_set : 3;
        unsigned char reserved_0 : 1;
        unsigned char buck00_nocp_h_set : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_BUCK00_CTRL2_UNION;
#endif
#define PMIC_BUCK00_CTRL2_buck00_nocp_l_set_START (0)
#define PMIC_BUCK00_CTRL2_buck00_nocp_l_set_END (2)
#define PMIC_BUCK00_CTRL2_buck00_nocp_h_set_START (4)
#define PMIC_BUCK00_CTRL2_buck00_nocp_h_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_cot_set : 3;
        unsigned char buck00_min_toff_set : 2;
        unsigned char buck00_min_ton_set : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK00_CTRL3_UNION;
#endif
#define PMIC_BUCK00_CTRL3_buck00_cot_set_START (0)
#define PMIC_BUCK00_CTRL3_buck00_cot_set_END (2)
#define PMIC_BUCK00_CTRL3_buck00_min_toff_set_START (3)
#define PMIC_BUCK00_CTRL3_buck00_min_toff_set_END (4)
#define PMIC_BUCK00_CTRL3_buck00_min_ton_set_START (5)
#define PMIC_BUCK00_CTRL3_buck00_min_ton_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_ndriver_n : 3;
        unsigned char reserved_0 : 1;
        unsigned char buck00_ndriver_p : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_BUCK00_CTRL4_UNION;
#endif
#define PMIC_BUCK00_CTRL4_buck00_ndriver_n_START (0)
#define PMIC_BUCK00_CTRL4_buck00_ndriver_n_END (2)
#define PMIC_BUCK00_CTRL4_buck00_ndriver_p_START (4)
#define PMIC_BUCK00_CTRL4_buck00_ndriver_p_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_pdriver_n : 3;
        unsigned char reserved_0 : 1;
        unsigned char buck00_pdriver_p : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_BUCK00_CTRL5_UNION;
#endif
#define PMIC_BUCK00_CTRL5_buck00_pdriver_n_START (0)
#define PMIC_BUCK00_CTRL5_buck00_pdriver_n_END (2)
#define PMIC_BUCK00_CTRL5_buck00_pdriver_p_START (4)
#define PMIC_BUCK00_CTRL5_buck00_pdriver_p_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_nsense_en : 1;
        unsigned char buck00_psense_en : 1;
        unsigned char buck00_triangl_mode : 2;
        unsigned char buck00_dt_np : 2;
        unsigned char buck00_dt_pn : 2;
    } reg;
} PMIC_BUCK00_CTRL6_UNION;
#endif
#define PMIC_BUCK00_CTRL6_buck00_nsense_en_START (0)
#define PMIC_BUCK00_CTRL6_buck00_nsense_en_END (0)
#define PMIC_BUCK00_CTRL6_buck00_psense_en_START (1)
#define PMIC_BUCK00_CTRL6_buck00_psense_en_END (1)
#define PMIC_BUCK00_CTRL6_buck00_triangl_mode_START (2)
#define PMIC_BUCK00_CTRL6_buck00_triangl_mode_END (3)
#define PMIC_BUCK00_CTRL6_buck00_dt_np_START (4)
#define PMIC_BUCK00_CTRL6_buck00_dt_np_END (5)
#define PMIC_BUCK00_CTRL6_buck00_dt_pn_START (6)
#define PMIC_BUCK00_CTRL6_buck00_dt_pn_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_loop_comp_r2 : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK00_CTRL7_UNION;
#endif
#define PMIC_BUCK00_CTRL7_buck00_loop_comp_r2_START (0)
#define PMIC_BUCK00_CTRL7_buck00_loop_comp_r2_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_loop_comp_r3 : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK00_CTRL8_UNION;
#endif
#define PMIC_BUCK00_CTRL8_buck00_loop_comp_r3_START (0)
#define PMIC_BUCK00_CTRL8_buck00_loop_comp_r3_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_loop_comp_c1 : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK00_CTRL9_UNION;
#endif
#define PMIC_BUCK00_CTRL9_buck00_loop_comp_c1_START (0)
#define PMIC_BUCK00_CTRL9_buck00_loop_comp_c1_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_loop_comp_c2 : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK00_CTRL10_UNION;
#endif
#define PMIC_BUCK00_CTRL10_buck00_loop_comp_c2_START (0)
#define PMIC_BUCK00_CTRL10_buck00_loop_comp_c2_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_loop_comp_c3 : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK00_CTRL11_UNION;
#endif
#define PMIC_BUCK00_CTRL11_buck00_loop_comp_c3_START (0)
#define PMIC_BUCK00_CTRL11_buck00_loop_comp_c3_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_triangl_c : 4;
        unsigned char buck00_triangl_r : 4;
    } reg;
} PMIC_BUCK00_CTRL12_UNION;
#endif
#define PMIC_BUCK00_CTRL12_buck00_triangl_c_START (0)
#define PMIC_BUCK00_CTRL12_buck00_triangl_c_END (3)
#define PMIC_BUCK00_CTRL12_buck00_triangl_r_START (4)
#define PMIC_BUCK00_CTRL12_buck00_triangl_r_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_ea_clamp : 4;
        unsigned char buck00_ea_clamp_cur : 2;
        unsigned char buck00_ea_bias : 2;
    } reg;
} PMIC_BUCK00_CTRL13_UNION;
#endif
#define PMIC_BUCK00_CTRL13_buck00_ea_clamp_START (0)
#define PMIC_BUCK00_CTRL13_buck00_ea_clamp_END (3)
#define PMIC_BUCK00_CTRL13_buck00_ea_clamp_cur_START (4)
#define PMIC_BUCK00_CTRL13_buck00_ea_clamp_cur_END (5)
#define PMIC_BUCK00_CTRL13_buck00_ea_bias_START (6)
#define PMIC_BUCK00_CTRL13_buck00_ea_bias_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_ea_reserve : 8;
    } reg;
} PMIC_BUCK00_CTRL14_UNION;
#endif
#define PMIC_BUCK00_CTRL14_buck00_ea_reserve_START (0)
#define PMIC_BUCK00_CTRL14_buck00_ea_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_nsense_bias : 2;
        unsigned char buck00_psense_bias : 2;
        unsigned char buck00_nsense_set : 2;
        unsigned char buck00_psense_set : 2;
    } reg;
} PMIC_BUCK00_CTRL15_UNION;
#endif
#define PMIC_BUCK00_CTRL15_buck00_nsense_bias_START (0)
#define PMIC_BUCK00_CTRL15_buck00_nsense_bias_END (1)
#define PMIC_BUCK00_CTRL15_buck00_psense_bias_START (2)
#define PMIC_BUCK00_CTRL15_buck00_psense_bias_END (3)
#define PMIC_BUCK00_CTRL15_buck00_nsense_set_START (4)
#define PMIC_BUCK00_CTRL15_buck00_nsense_set_END (5)
#define PMIC_BUCK00_CTRL15_buck00_psense_set_START (6)
#define PMIC_BUCK00_CTRL15_buck00_psense_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_vodet_f_sth : 4;
        unsigned char buck00_vodet_r_sth : 4;
    } reg;
} PMIC_BUCK00_CTRL16_UNION;
#endif
#define PMIC_BUCK00_CTRL16_buck00_vodet_f_sth_START (0)
#define PMIC_BUCK00_CTRL16_buck00_vodet_f_sth_END (3)
#define PMIC_BUCK00_CTRL16_buck00_vodet_r_sth_START (4)
#define PMIC_BUCK00_CTRL16_buck00_vodet_r_sth_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_vodet_f_en : 1;
        unsigned char buck00_vodet_r_en : 1;
        unsigned char buck00_vodet_bias_set : 2;
        unsigned char buck00_vodet_filter_set : 4;
    } reg;
} PMIC_BUCK00_CTRL17_UNION;
#endif
#define PMIC_BUCK00_CTRL17_buck00_vodet_f_en_START (0)
#define PMIC_BUCK00_CTRL17_buck00_vodet_f_en_END (0)
#define PMIC_BUCK00_CTRL17_buck00_vodet_r_en_START (1)
#define PMIC_BUCK00_CTRL17_buck00_vodet_r_en_END (1)
#define PMIC_BUCK00_CTRL17_buck00_vodet_bias_set_START (2)
#define PMIC_BUCK00_CTRL17_buck00_vodet_bias_set_END (3)
#define PMIC_BUCK00_CTRL17_buck00_vodet_filter_set_START (4)
#define PMIC_BUCK00_CTRL17_buck00_vodet_filter_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_eco_ton_sel : 2;
        unsigned char buck00_eco_model_sel : 1;
        unsigned char buck00_pwm_comp_bias : 2;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK00_CTRL18_UNION;
#endif
#define PMIC_BUCK00_CTRL18_buck00_eco_ton_sel_START (0)
#define PMIC_BUCK00_CTRL18_buck00_eco_ton_sel_END (1)
#define PMIC_BUCK00_CTRL18_buck00_eco_model_sel_START (2)
#define PMIC_BUCK00_CTRL18_buck00_eco_model_sel_END (2)
#define PMIC_BUCK00_CTRL18_buck00_pwm_comp_bias_START (3)
#define PMIC_BUCK00_CTRL18_buck00_pwm_comp_bias_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_fb_sel : 2;
        unsigned char buck00_fb_bias : 2;
        unsigned char buck00_fb_eco : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK00_CTRL19_UNION;
#endif
#define PMIC_BUCK00_CTRL19_buck00_fb_sel_START (0)
#define PMIC_BUCK00_CTRL19_buck00_fb_sel_END (1)
#define PMIC_BUCK00_CTRL19_buck00_fb_bias_START (2)
#define PMIC_BUCK00_CTRL19_buck00_fb_bias_END (3)
#define PMIC_BUCK00_CTRL19_buck00_fb_eco_START (4)
#define PMIC_BUCK00_CTRL19_buck00_fb_eco_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_reserve0 : 8;
    } reg;
} PMIC_BUCK00_CTRL20_UNION;
#endif
#define PMIC_BUCK00_CTRL20_buck00_reserve0_START (0)
#define PMIC_BUCK00_CTRL20_buck00_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck01_pocp_delay_sel : 2;
        unsigned char buck01_dmd_delay_sel : 2;
        unsigned char buck01_dmd_sel : 3;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK01_CTRL0_UNION;
#endif
#define PMIC_BUCK01_CTRL0_buck01_pocp_delay_sel_START (0)
#define PMIC_BUCK01_CTRL0_buck01_pocp_delay_sel_END (1)
#define PMIC_BUCK01_CTRL0_buck01_dmd_delay_sel_START (2)
#define PMIC_BUCK01_CTRL0_buck01_dmd_delay_sel_END (3)
#define PMIC_BUCK01_CTRL0_buck01_dmd_sel_START (4)
#define PMIC_BUCK01_CTRL0_buck01_dmd_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck01_pocp_set : 3;
        unsigned char buck01_nocp_l_delay_sel : 2;
        unsigned char buck01_nocp_h_delay_sel : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK01_CTRL1_UNION;
#endif
#define PMIC_BUCK01_CTRL1_buck01_pocp_set_START (0)
#define PMIC_BUCK01_CTRL1_buck01_pocp_set_END (2)
#define PMIC_BUCK01_CTRL1_buck01_nocp_l_delay_sel_START (3)
#define PMIC_BUCK01_CTRL1_buck01_nocp_l_delay_sel_END (4)
#define PMIC_BUCK01_CTRL1_buck01_nocp_h_delay_sel_START (5)
#define PMIC_BUCK01_CTRL1_buck01_nocp_h_delay_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck01_nocp_l_set : 3;
        unsigned char reserved_0 : 1;
        unsigned char buck01_nocp_h_set : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_BUCK01_CTRL2_UNION;
#endif
#define PMIC_BUCK01_CTRL2_buck01_nocp_l_set_START (0)
#define PMIC_BUCK01_CTRL2_buck01_nocp_l_set_END (2)
#define PMIC_BUCK01_CTRL2_buck01_nocp_h_set_START (4)
#define PMIC_BUCK01_CTRL2_buck01_nocp_h_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck01_cot_set : 3;
        unsigned char buck01_min_toff_set : 2;
        unsigned char buck01_min_ton_set : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK01_CTRL3_UNION;
#endif
#define PMIC_BUCK01_CTRL3_buck01_cot_set_START (0)
#define PMIC_BUCK01_CTRL3_buck01_cot_set_END (2)
#define PMIC_BUCK01_CTRL3_buck01_min_toff_set_START (3)
#define PMIC_BUCK01_CTRL3_buck01_min_toff_set_END (4)
#define PMIC_BUCK01_CTRL3_buck01_min_ton_set_START (5)
#define PMIC_BUCK01_CTRL3_buck01_min_ton_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck01_ndriver_n : 3;
        unsigned char reserved_0 : 1;
        unsigned char buck01_ndriver_p : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_BUCK01_CTRL4_UNION;
#endif
#define PMIC_BUCK01_CTRL4_buck01_ndriver_n_START (0)
#define PMIC_BUCK01_CTRL4_buck01_ndriver_n_END (2)
#define PMIC_BUCK01_CTRL4_buck01_ndriver_p_START (4)
#define PMIC_BUCK01_CTRL4_buck01_ndriver_p_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck01_pdriver_n : 3;
        unsigned char reserved_0 : 1;
        unsigned char buck01_pdriver_p : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_BUCK01_CTRL5_UNION;
#endif
#define PMIC_BUCK01_CTRL5_buck01_pdriver_n_START (0)
#define PMIC_BUCK01_CTRL5_buck01_pdriver_n_END (2)
#define PMIC_BUCK01_CTRL5_buck01_pdriver_p_START (4)
#define PMIC_BUCK01_CTRL5_buck01_pdriver_p_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck01_nsense_en : 1;
        unsigned char buck01_psense_en : 1;
        unsigned char buck01_pwm_comp_bias : 2;
        unsigned char buck01_dt_np : 2;
        unsigned char buck01_dt_pn : 2;
    } reg;
} PMIC_BUCK01_CTRL6_UNION;
#endif
#define PMIC_BUCK01_CTRL6_buck01_nsense_en_START (0)
#define PMIC_BUCK01_CTRL6_buck01_nsense_en_END (0)
#define PMIC_BUCK01_CTRL6_buck01_psense_en_START (1)
#define PMIC_BUCK01_CTRL6_buck01_psense_en_END (1)
#define PMIC_BUCK01_CTRL6_buck01_pwm_comp_bias_START (2)
#define PMIC_BUCK01_CTRL6_buck01_pwm_comp_bias_END (3)
#define PMIC_BUCK01_CTRL6_buck01_dt_np_START (4)
#define PMIC_BUCK01_CTRL6_buck01_dt_np_END (5)
#define PMIC_BUCK01_CTRL6_buck01_dt_pn_START (6)
#define PMIC_BUCK01_CTRL6_buck01_dt_pn_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck01_nsense_bias : 2;
        unsigned char buck01_psense_bias : 2;
        unsigned char buck01_nsense_set : 2;
        unsigned char buck01_psense_set : 2;
    } reg;
} PMIC_BUCK01_CTRL7_UNION;
#endif
#define PMIC_BUCK01_CTRL7_buck01_nsense_bias_START (0)
#define PMIC_BUCK01_CTRL7_buck01_nsense_bias_END (1)
#define PMIC_BUCK01_CTRL7_buck01_psense_bias_START (2)
#define PMIC_BUCK01_CTRL7_buck01_psense_bias_END (3)
#define PMIC_BUCK01_CTRL7_buck01_nsense_set_START (4)
#define PMIC_BUCK01_CTRL7_buck01_nsense_set_END (5)
#define PMIC_BUCK01_CTRL7_buck01_psense_set_START (6)
#define PMIC_BUCK01_CTRL7_buck01_psense_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck01_reserve0 : 8;
    } reg;
} PMIC_BUCK01_CTRL8_UNION;
#endif
#define PMIC_BUCK01_CTRL8_buck01_reserve0_START (0)
#define PMIC_BUCK01_CTRL8_buck01_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_pocp_delay_sel : 2;
        unsigned char buck02_dmd_delay_sel : 2;
        unsigned char buck02_dmd_sel : 3;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK02_CTRL0_UNION;
#endif
#define PMIC_BUCK02_CTRL0_buck02_pocp_delay_sel_START (0)
#define PMIC_BUCK02_CTRL0_buck02_pocp_delay_sel_END (1)
#define PMIC_BUCK02_CTRL0_buck02_dmd_delay_sel_START (2)
#define PMIC_BUCK02_CTRL0_buck02_dmd_delay_sel_END (3)
#define PMIC_BUCK02_CTRL0_buck02_dmd_sel_START (4)
#define PMIC_BUCK02_CTRL0_buck02_dmd_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_pocp_set : 3;
        unsigned char buck02_nocp_l_delay_sel : 2;
        unsigned char buck02_nocp_h_delay_sel : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK02_CTRL1_UNION;
#endif
#define PMIC_BUCK02_CTRL1_buck02_pocp_set_START (0)
#define PMIC_BUCK02_CTRL1_buck02_pocp_set_END (2)
#define PMIC_BUCK02_CTRL1_buck02_nocp_l_delay_sel_START (3)
#define PMIC_BUCK02_CTRL1_buck02_nocp_l_delay_sel_END (4)
#define PMIC_BUCK02_CTRL1_buck02_nocp_h_delay_sel_START (5)
#define PMIC_BUCK02_CTRL1_buck02_nocp_h_delay_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_nocp_l_set : 3;
        unsigned char reserved_0 : 1;
        unsigned char buck02_nocp_h_set : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_BUCK02_CTRL2_UNION;
#endif
#define PMIC_BUCK02_CTRL2_buck02_nocp_l_set_START (0)
#define PMIC_BUCK02_CTRL2_buck02_nocp_l_set_END (2)
#define PMIC_BUCK02_CTRL2_buck02_nocp_h_set_START (4)
#define PMIC_BUCK02_CTRL2_buck02_nocp_h_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_cot_set : 3;
        unsigned char buck02_min_toff_set : 2;
        unsigned char buck02_min_ton_set : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK02_CTRL3_UNION;
#endif
#define PMIC_BUCK02_CTRL3_buck02_cot_set_START (0)
#define PMIC_BUCK02_CTRL3_buck02_cot_set_END (2)
#define PMIC_BUCK02_CTRL3_buck02_min_toff_set_START (3)
#define PMIC_BUCK02_CTRL3_buck02_min_toff_set_END (4)
#define PMIC_BUCK02_CTRL3_buck02_min_ton_set_START (5)
#define PMIC_BUCK02_CTRL3_buck02_min_ton_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_ndriver_n : 3;
        unsigned char reserved_0 : 1;
        unsigned char buck02_ndriver_p : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_BUCK02_CTRL4_UNION;
#endif
#define PMIC_BUCK02_CTRL4_buck02_ndriver_n_START (0)
#define PMIC_BUCK02_CTRL4_buck02_ndriver_n_END (2)
#define PMIC_BUCK02_CTRL4_buck02_ndriver_p_START (4)
#define PMIC_BUCK02_CTRL4_buck02_ndriver_p_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_pdriver_n : 3;
        unsigned char reserved_0 : 1;
        unsigned char buck02_pdriver_p : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_BUCK02_CTRL5_UNION;
#endif
#define PMIC_BUCK02_CTRL5_buck02_pdriver_n_START (0)
#define PMIC_BUCK02_CTRL5_buck02_pdriver_n_END (2)
#define PMIC_BUCK02_CTRL5_buck02_pdriver_p_START (4)
#define PMIC_BUCK02_CTRL5_buck02_pdriver_p_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_nsense_en : 1;
        unsigned char buck02_psense_en : 1;
        unsigned char buck02_triangl_mode : 2;
        unsigned char buck02_dt_np : 2;
        unsigned char buck02_dt_pn : 2;
    } reg;
} PMIC_BUCK02_CTRL6_UNION;
#endif
#define PMIC_BUCK02_CTRL6_buck02_nsense_en_START (0)
#define PMIC_BUCK02_CTRL6_buck02_nsense_en_END (0)
#define PMIC_BUCK02_CTRL6_buck02_psense_en_START (1)
#define PMIC_BUCK02_CTRL6_buck02_psense_en_END (1)
#define PMIC_BUCK02_CTRL6_buck02_triangl_mode_START (2)
#define PMIC_BUCK02_CTRL6_buck02_triangl_mode_END (3)
#define PMIC_BUCK02_CTRL6_buck02_dt_np_START (4)
#define PMIC_BUCK02_CTRL6_buck02_dt_np_END (5)
#define PMIC_BUCK02_CTRL6_buck02_dt_pn_START (6)
#define PMIC_BUCK02_CTRL6_buck02_dt_pn_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_loop_comp_r2 : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK02_CTRL7_UNION;
#endif
#define PMIC_BUCK02_CTRL7_buck02_loop_comp_r2_START (0)
#define PMIC_BUCK02_CTRL7_buck02_loop_comp_r2_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_loop_comp_r3 : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK02_CTRL8_UNION;
#endif
#define PMIC_BUCK02_CTRL8_buck02_loop_comp_r3_START (0)
#define PMIC_BUCK02_CTRL8_buck02_loop_comp_r3_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_loop_comp_c1 : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK02_CTRL9_UNION;
#endif
#define PMIC_BUCK02_CTRL9_buck02_loop_comp_c1_START (0)
#define PMIC_BUCK02_CTRL9_buck02_loop_comp_c1_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_loop_comp_c2 : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK02_CTRL10_UNION;
#endif
#define PMIC_BUCK02_CTRL10_buck02_loop_comp_c2_START (0)
#define PMIC_BUCK02_CTRL10_buck02_loop_comp_c2_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_loop_comp_c3 : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK02_CTRL11_UNION;
#endif
#define PMIC_BUCK02_CTRL11_buck02_loop_comp_c3_START (0)
#define PMIC_BUCK02_CTRL11_buck02_loop_comp_c3_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_triangl_c : 4;
        unsigned char buck02_triangl_r : 4;
    } reg;
} PMIC_BUCK02_CTRL12_UNION;
#endif
#define PMIC_BUCK02_CTRL12_buck02_triangl_c_START (0)
#define PMIC_BUCK02_CTRL12_buck02_triangl_c_END (3)
#define PMIC_BUCK02_CTRL12_buck02_triangl_r_START (4)
#define PMIC_BUCK02_CTRL12_buck02_triangl_r_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_ea_clamp : 4;
        unsigned char buck02_ea_clamp_cur : 2;
        unsigned char buck02_ea_bias : 2;
    } reg;
} PMIC_BUCK02_CTRL13_UNION;
#endif
#define PMIC_BUCK02_CTRL13_buck02_ea_clamp_START (0)
#define PMIC_BUCK02_CTRL13_buck02_ea_clamp_END (3)
#define PMIC_BUCK02_CTRL13_buck02_ea_clamp_cur_START (4)
#define PMIC_BUCK02_CTRL13_buck02_ea_clamp_cur_END (5)
#define PMIC_BUCK02_CTRL13_buck02_ea_bias_START (6)
#define PMIC_BUCK02_CTRL13_buck02_ea_bias_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_ea_reserve : 8;
    } reg;
} PMIC_BUCK02_CTRL14_UNION;
#endif
#define PMIC_BUCK02_CTRL14_buck02_ea_reserve_START (0)
#define PMIC_BUCK02_CTRL14_buck02_ea_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_nsense_bias : 2;
        unsigned char buck02_psense_bias : 2;
        unsigned char buck02_nsense_set : 2;
        unsigned char buck02_psense_set : 2;
    } reg;
} PMIC_BUCK02_CTRL15_UNION;
#endif
#define PMIC_BUCK02_CTRL15_buck02_nsense_bias_START (0)
#define PMIC_BUCK02_CTRL15_buck02_nsense_bias_END (1)
#define PMIC_BUCK02_CTRL15_buck02_psense_bias_START (2)
#define PMIC_BUCK02_CTRL15_buck02_psense_bias_END (3)
#define PMIC_BUCK02_CTRL15_buck02_nsense_set_START (4)
#define PMIC_BUCK02_CTRL15_buck02_nsense_set_END (5)
#define PMIC_BUCK02_CTRL15_buck02_psense_set_START (6)
#define PMIC_BUCK02_CTRL15_buck02_psense_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_vodet_f_sth : 4;
        unsigned char buck02_vodet_r_sth : 4;
    } reg;
} PMIC_BUCK02_CTRL16_UNION;
#endif
#define PMIC_BUCK02_CTRL16_buck02_vodet_f_sth_START (0)
#define PMIC_BUCK02_CTRL16_buck02_vodet_f_sth_END (3)
#define PMIC_BUCK02_CTRL16_buck02_vodet_r_sth_START (4)
#define PMIC_BUCK02_CTRL16_buck02_vodet_r_sth_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_vodet_f_en : 1;
        unsigned char buck02_vodet_r_en : 1;
        unsigned char buck02_vodet_bias_set : 2;
        unsigned char buck02_vodet_filter_set : 4;
    } reg;
} PMIC_BUCK02_CTRL17_UNION;
#endif
#define PMIC_BUCK02_CTRL17_buck02_vodet_f_en_START (0)
#define PMIC_BUCK02_CTRL17_buck02_vodet_f_en_END (0)
#define PMIC_BUCK02_CTRL17_buck02_vodet_r_en_START (1)
#define PMIC_BUCK02_CTRL17_buck02_vodet_r_en_END (1)
#define PMIC_BUCK02_CTRL17_buck02_vodet_bias_set_START (2)
#define PMIC_BUCK02_CTRL17_buck02_vodet_bias_set_END (3)
#define PMIC_BUCK02_CTRL17_buck02_vodet_filter_set_START (4)
#define PMIC_BUCK02_CTRL17_buck02_vodet_filter_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_eco_ton_sel : 2;
        unsigned char buck02_eco_model_sel : 1;
        unsigned char buck02_pwm_comp_bias : 2;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK02_CTRL18_UNION;
#endif
#define PMIC_BUCK02_CTRL18_buck02_eco_ton_sel_START (0)
#define PMIC_BUCK02_CTRL18_buck02_eco_ton_sel_END (1)
#define PMIC_BUCK02_CTRL18_buck02_eco_model_sel_START (2)
#define PMIC_BUCK02_CTRL18_buck02_eco_model_sel_END (2)
#define PMIC_BUCK02_CTRL18_buck02_pwm_comp_bias_START (3)
#define PMIC_BUCK02_CTRL18_buck02_pwm_comp_bias_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_fb_sel : 2;
        unsigned char buck02_fb_bias : 2;
        unsigned char buck02_fb_eco : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK02_CTRL19_UNION;
#endif
#define PMIC_BUCK02_CTRL19_buck02_fb_sel_START (0)
#define PMIC_BUCK02_CTRL19_buck02_fb_sel_END (1)
#define PMIC_BUCK02_CTRL19_buck02_fb_bias_START (2)
#define PMIC_BUCK02_CTRL19_buck02_fb_bias_END (3)
#define PMIC_BUCK02_CTRL19_buck02_fb_eco_START (4)
#define PMIC_BUCK02_CTRL19_buck02_fb_eco_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_reserve0 : 8;
    } reg;
} PMIC_BUCK02_CTRL20_UNION;
#endif
#define PMIC_BUCK02_CTRL20_buck02_reserve0_START (0)
#define PMIC_BUCK02_CTRL20_buck02_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_avg_isense_filter : 2;
        unsigned char buck0_nocp_l_en : 1;
        unsigned char buck0_nocp_h_en : 1;
        unsigned char buck0_pocp_en : 1;
        unsigned char buck0_ocp_cont : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK0_CTRL0_UNION;
#endif
#define PMIC_BUCK0_CTRL0_buck0_avg_isense_filter_START (0)
#define PMIC_BUCK0_CTRL0_buck0_avg_isense_filter_END (1)
#define PMIC_BUCK0_CTRL0_buck0_nocp_l_en_START (2)
#define PMIC_BUCK0_CTRL0_buck0_nocp_l_en_END (2)
#define PMIC_BUCK0_CTRL0_buck0_nocp_h_en_START (3)
#define PMIC_BUCK0_CTRL0_buck0_nocp_h_en_END (3)
#define PMIC_BUCK0_CTRL0_buck0_pocp_en_START (4)
#define PMIC_BUCK0_CTRL0_buck0_pocp_en_END (4)
#define PMIC_BUCK0_CTRL0_buck0_ocp_cont_START (5)
#define PMIC_BUCK0_CTRL0_buck0_ocp_cont_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_i_share_gain : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK0_CTRL1_UNION;
#endif
#define PMIC_BUCK0_CTRL1_buck0_i_share_gain_START (0)
#define PMIC_BUCK0_CTRL1_buck0_i_share_gain_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_i_share_ratio : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK0_CTRL2_UNION;
#endif
#define PMIC_BUCK0_CTRL2_buck0_i_share_ratio_START (0)
#define PMIC_BUCK0_CTRL2_buck0_i_share_ratio_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_phase_man_set : 2;
        unsigned char buck0_i_share_offset : 4;
        unsigned char buck0_i_share_set : 2;
    } reg;
} PMIC_BUCK0_CTRL3_UNION;
#endif
#define PMIC_BUCK0_CTRL3_buck0_phase_man_set_START (0)
#define PMIC_BUCK0_CTRL3_buck0_phase_man_set_END (1)
#define PMIC_BUCK0_CTRL3_buck0_i_share_offset_START (2)
#define PMIC_BUCK0_CTRL3_buck0_i_share_offset_END (5)
#define PMIC_BUCK0_CTRL3_buck0_i_share_set_START (6)
#define PMIC_BUCK0_CTRL3_buck0_i_share_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_i_share_en : 1;
        unsigned char buck0_fbcur_c_sel : 2;
        unsigned char buck0_fbcur_r_sel : 3;
        unsigned char buck0_fbcur_bw_sel : 2;
    } reg;
} PMIC_BUCK0_CTRL4_UNION;
#endif
#define PMIC_BUCK0_CTRL4_buck0_i_share_en_START (0)
#define PMIC_BUCK0_CTRL4_buck0_i_share_en_END (0)
#define PMIC_BUCK0_CTRL4_buck0_fbcur_c_sel_START (1)
#define PMIC_BUCK0_CTRL4_buck0_fbcur_c_sel_END (2)
#define PMIC_BUCK0_CTRL4_buck0_fbcur_r_sel_START (3)
#define PMIC_BUCK0_CTRL4_buck0_fbcur_r_sel_END (5)
#define PMIC_BUCK0_CTRL4_buck0_fbcur_bw_sel_START (6)
#define PMIC_BUCK0_CTRL4_buck0_fbcur_bw_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_phase_dec_filter : 2;
        unsigned char buck0_phase_add_filter : 2;
        unsigned char buck0_phase_dec_sth : 2;
        unsigned char buck0_phase_add_sth : 2;
    } reg;
} PMIC_BUCK0_CTRL5_UNION;
#endif
#define PMIC_BUCK0_CTRL5_buck0_phase_dec_filter_START (0)
#define PMIC_BUCK0_CTRL5_buck0_phase_dec_filter_END (1)
#define PMIC_BUCK0_CTRL5_buck0_phase_add_filter_START (2)
#define PMIC_BUCK0_CTRL5_buck0_phase_add_filter_END (3)
#define PMIC_BUCK0_CTRL5_buck0_phase_dec_sth_START (4)
#define PMIC_BUCK0_CTRL5_buck0_phase_dec_sth_END (5)
#define PMIC_BUCK0_CTRL5_buck0_phase_add_sth_START (6)
#define PMIC_BUCK0_CTRL5_buck0_phase_add_sth_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_powermos_sel_eco : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_BUCK0_CTRL6_UNION;
#endif
#define PMIC_BUCK0_CTRL6_buck0_powermos_sel_eco_START (0)
#define PMIC_BUCK0_CTRL6_buck0_powermos_sel_eco_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_test_mode : 8;
    } reg;
} PMIC_BUCK0_CTRL7_UNION;
#endif
#define PMIC_BUCK0_CTRL7_buck0_test_mode_START (0)
#define PMIC_BUCK0_CTRL7_buck0_test_mode_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_test_out : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_BUCK0_CTRL8_UNION;
#endif
#define PMIC_BUCK0_CTRL8_buck0_test_out_START (0)
#define PMIC_BUCK0_CTRL8_buck0_test_out_END (3)
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
        unsigned char reserved : 2;
        unsigned char buck1_dt_sel : 2;
        unsigned char buck1_ocp_sel : 2;
    } reg;
} PMIC_BUCK1_CTRL1_UNION;
#endif
#define PMIC_BUCK1_CTRL1_buck1_ng_dt_sel_START (0)
#define PMIC_BUCK1_CTRL1_buck1_ng_dt_sel_END (0)
#define PMIC_BUCK1_CTRL1_buck1_pg_dt_sel_START (1)
#define PMIC_BUCK1_CTRL1_buck1_pg_dt_sel_END (1)
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
        unsigned char reserved_0 : 2;
        unsigned char buck1_reg_en : 1;
        unsigned char buck1_adj_clx : 2;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_BUCK1_CTRL4_UNION;
#endif
#define PMIC_BUCK1_CTRL4_buck1_reg_r_START (0)
#define PMIC_BUCK1_CTRL4_buck1_reg_r_END (1)
#define PMIC_BUCK1_CTRL4_buck1_reg_en_START (4)
#define PMIC_BUCK1_CTRL4_buck1_reg_en_END (4)
#define PMIC_BUCK1_CTRL4_buck1_adj_clx_START (5)
#define PMIC_BUCK1_CTRL4_buck1_adj_clx_END (6)
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
        unsigned char buck1_sense_ctrl : 1;
        unsigned char buck1_sense_r : 6;
        unsigned char buck1_sense_dmd : 1;
    } reg;
} PMIC_BUCK1_CTRL7_UNION;
#endif
#define PMIC_BUCK1_CTRL7_buck1_sense_ctrl_START (0)
#define PMIC_BUCK1_CTRL7_buck1_sense_ctrl_END (0)
#define PMIC_BUCK1_CTRL7_buck1_sense_r_START (1)
#define PMIC_BUCK1_CTRL7_buck1_sense_r_END (6)
#define PMIC_BUCK1_CTRL7_buck1_sense_dmd_START (7)
#define PMIC_BUCK1_CTRL7_buck1_sense_dmd_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_sense_filter : 4;
        unsigned char buck1_sense_sel : 1;
        unsigned char buck1_ton_dmd : 1;
        unsigned char buck1_ocp_delay : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK1_CTRL8_UNION;
#endif
#define PMIC_BUCK1_CTRL8_buck1_sense_filter_START (0)
#define PMIC_BUCK1_CTRL8_buck1_sense_filter_END (3)
#define PMIC_BUCK1_CTRL8_buck1_sense_sel_START (4)
#define PMIC_BUCK1_CTRL8_buck1_sense_sel_END (4)
#define PMIC_BUCK1_CTRL8_buck1_ton_dmd_START (5)
#define PMIC_BUCK1_CTRL8_buck1_ton_dmd_END (5)
#define PMIC_BUCK1_CTRL8_buck1_ocp_delay_START (6)
#define PMIC_BUCK1_CTRL8_buck1_ocp_delay_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_far_fb : 3;
        unsigned char buck1_far_fb_en : 1;
        unsigned char buck1_sense_rev0 : 4;
    } reg;
} PMIC_BUCK1_CTRL9_UNION;
#endif
#define PMIC_BUCK1_CTRL9_buck1_far_fb_START (0)
#define PMIC_BUCK1_CTRL9_buck1_far_fb_END (2)
#define PMIC_BUCK1_CTRL9_buck1_far_fb_en_START (3)
#define PMIC_BUCK1_CTRL9_buck1_far_fb_en_END (3)
#define PMIC_BUCK1_CTRL9_buck1_sense_rev0_START (4)
#define PMIC_BUCK1_CTRL9_buck1_sense_rev0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_eff_enh_adj : 3;
        unsigned char buck1_eff_enh_en : 1;
        unsigned char buck1_tran_enh : 3;
        unsigned char buck1_tran_enh_en : 1;
    } reg;
} PMIC_BUCK1_CTRL10_UNION;
#endif
#define PMIC_BUCK1_CTRL10_buck1_eff_enh_adj_START (0)
#define PMIC_BUCK1_CTRL10_buck1_eff_enh_adj_END (2)
#define PMIC_BUCK1_CTRL10_buck1_eff_enh_en_START (3)
#define PMIC_BUCK1_CTRL10_buck1_eff_enh_en_END (3)
#define PMIC_BUCK1_CTRL10_buck1_tran_enh_START (4)
#define PMIC_BUCK1_CTRL10_buck1_tran_enh_END (6)
#define PMIC_BUCK1_CTRL10_buck1_tran_enh_en_START (7)
#define PMIC_BUCK1_CTRL10_buck1_tran_enh_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_eco_isel : 1;
        unsigned char buck1_bias_dmd_sel : 1;
        unsigned char buck1_dmd_new_sel : 1;
        unsigned char buck1_lx_dt : 2;
        unsigned char buck1_fb_cap_sel : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK1_CTRL11_UNION;
#endif
#define PMIC_BUCK1_CTRL11_buck1_eco_isel_START (0)
#define PMIC_BUCK1_CTRL11_buck1_eco_isel_END (0)
#define PMIC_BUCK1_CTRL11_buck1_bias_dmd_sel_START (1)
#define PMIC_BUCK1_CTRL11_buck1_bias_dmd_sel_END (1)
#define PMIC_BUCK1_CTRL11_buck1_dmd_new_sel_START (2)
#define PMIC_BUCK1_CTRL11_buck1_dmd_new_sel_END (2)
#define PMIC_BUCK1_CTRL11_buck1_lx_dt_START (3)
#define PMIC_BUCK1_CTRL11_buck1_lx_dt_END (4)
#define PMIC_BUCK1_CTRL11_buck1_fb_cap_sel_START (5)
#define PMIC_BUCK1_CTRL11_buck1_fb_cap_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_vodet_pull : 8;
    } reg;
} PMIC_BUCK1_CTRL12_UNION;
#endif
#define PMIC_BUCK1_CTRL12_buck1_vodet_pull_START (0)
#define PMIC_BUCK1_CTRL12_buck1_vodet_pull_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_fix_fq : 5;
        unsigned char buck1_fix_fq_ctrl : 1;
        unsigned char buck1_fix_ton : 2;
    } reg;
} PMIC_BUCK1_CTRL13_UNION;
#endif
#define PMIC_BUCK1_CTRL13_buck1_fix_fq_START (0)
#define PMIC_BUCK1_CTRL13_buck1_fix_fq_END (4)
#define PMIC_BUCK1_CTRL13_buck1_fix_fq_ctrl_START (5)
#define PMIC_BUCK1_CTRL13_buck1_fix_fq_ctrl_END (5)
#define PMIC_BUCK1_CTRL13_buck1_fix_ton_START (6)
#define PMIC_BUCK1_CTRL13_buck1_fix_ton_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_reserve : 8;
    } reg;
} PMIC_BUCK1_CTRL14_UNION;
#endif
#define PMIC_BUCK1_CTRL14_buck1_reserve_START (0)
#define PMIC_BUCK1_CTRL14_buck1_reserve_END (7)
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
        unsigned char reserved : 2;
        unsigned char buck2_dt_sel : 2;
        unsigned char buck2_ocp_sel : 2;
    } reg;
} PMIC_BUCK2_CTRL1_UNION;
#endif
#define PMIC_BUCK2_CTRL1_buck2_ng_dt_sel_START (0)
#define PMIC_BUCK2_CTRL1_buck2_ng_dt_sel_END (0)
#define PMIC_BUCK2_CTRL1_buck2_pg_dt_sel_START (1)
#define PMIC_BUCK2_CTRL1_buck2_pg_dt_sel_END (1)
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
        unsigned char reserved_0 : 2;
        unsigned char buck2_reg_en : 1;
        unsigned char buck2_adj_clx : 2;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_BUCK2_CTRL4_UNION;
#endif
#define PMIC_BUCK2_CTRL4_buck2_reg_r_START (0)
#define PMIC_BUCK2_CTRL4_buck2_reg_r_END (1)
#define PMIC_BUCK2_CTRL4_buck2_reg_en_START (4)
#define PMIC_BUCK2_CTRL4_buck2_reg_en_END (4)
#define PMIC_BUCK2_CTRL4_buck2_adj_clx_START (5)
#define PMIC_BUCK2_CTRL4_buck2_adj_clx_END (6)
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
        unsigned char buck2_eco_isel : 1;
        unsigned char buck2_bias_dmd_sel : 1;
        unsigned char buck2_dmd_new_sel : 1;
        unsigned char buck2_ton_dmd : 1;
        unsigned char buck2_ocp_delay : 1;
        unsigned char buck2_lx_dt : 2;
        unsigned char buck2_fb_cap_sel : 1;
    } reg;
} PMIC_BUCK2_CTRL7_UNION;
#endif
#define PMIC_BUCK2_CTRL7_buck2_eco_isel_START (0)
#define PMIC_BUCK2_CTRL7_buck2_eco_isel_END (0)
#define PMIC_BUCK2_CTRL7_buck2_bias_dmd_sel_START (1)
#define PMIC_BUCK2_CTRL7_buck2_bias_dmd_sel_END (1)
#define PMIC_BUCK2_CTRL7_buck2_dmd_new_sel_START (2)
#define PMIC_BUCK2_CTRL7_buck2_dmd_new_sel_END (2)
#define PMIC_BUCK2_CTRL7_buck2_ton_dmd_START (3)
#define PMIC_BUCK2_CTRL7_buck2_ton_dmd_END (3)
#define PMIC_BUCK2_CTRL7_buck2_ocp_delay_START (4)
#define PMIC_BUCK2_CTRL7_buck2_ocp_delay_END (4)
#define PMIC_BUCK2_CTRL7_buck2_lx_dt_START (5)
#define PMIC_BUCK2_CTRL7_buck2_lx_dt_END (6)
#define PMIC_BUCK2_CTRL7_buck2_fb_cap_sel_START (7)
#define PMIC_BUCK2_CTRL7_buck2_fb_cap_sel_END (7)
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
        unsigned char buck2_reserve : 8;
    } reg;
} PMIC_BUCK2_CTRL9_UNION;
#endif
#define PMIC_BUCK2_CTRL9_buck2_reserve_START (0)
#define PMIC_BUCK2_CTRL9_buck2_reserve_END (7)
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
        unsigned char reserved_0 : 2;
        unsigned char buck3_dt_sel : 2;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_BUCK3_CTRL1_UNION;
#endif
#define PMIC_BUCK3_CTRL1_buck3_ng_dt_sel_START (0)
#define PMIC_BUCK3_CTRL1_buck3_ng_dt_sel_END (0)
#define PMIC_BUCK3_CTRL1_buck3_pg_dt_sel_START (1)
#define PMIC_BUCK3_CTRL1_buck3_pg_dt_sel_END (1)
#define PMIC_BUCK3_CTRL1_buck3_dt_sel_START (4)
#define PMIC_BUCK3_CTRL1_buck3_dt_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_pg_n_sel : 3;
        unsigned char buck3_pg_p_sel : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK3_CTRL2_UNION;
#endif
#define PMIC_BUCK3_CTRL2_buck3_pg_n_sel_START (0)
#define PMIC_BUCK3_CTRL2_buck3_pg_n_sel_END (2)
#define PMIC_BUCK3_CTRL2_buck3_pg_p_sel_START (3)
#define PMIC_BUCK3_CTRL2_buck3_pg_p_sel_END (5)
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
        unsigned char reserved_0 : 2;
        unsigned char buck3_reg_en : 1;
        unsigned char buck3_adj_clx : 2;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_BUCK3_CTRL4_UNION;
#endif
#define PMIC_BUCK3_CTRL4_buck3_reg_r_START (0)
#define PMIC_BUCK3_CTRL4_buck3_reg_r_END (1)
#define PMIC_BUCK3_CTRL4_buck3_reg_en_START (4)
#define PMIC_BUCK3_CTRL4_buck3_reg_en_END (4)
#define PMIC_BUCK3_CTRL4_buck3_adj_clx_START (5)
#define PMIC_BUCK3_CTRL4_buck3_adj_clx_END (6)
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
        unsigned char buck3_eco_isel : 1;
        unsigned char buck3_bias_dmd_sel : 1;
        unsigned char buck3_dmd_new_sel : 1;
        unsigned char buck3_ton_dmd : 1;
        unsigned char buck3_ocp_delay : 1;
        unsigned char buck3_lx_dt : 2;
        unsigned char buck3_fb_cap_sel : 1;
    } reg;
} PMIC_BUCK3_CTRL7_UNION;
#endif
#define PMIC_BUCK3_CTRL7_buck3_eco_isel_START (0)
#define PMIC_BUCK3_CTRL7_buck3_eco_isel_END (0)
#define PMIC_BUCK3_CTRL7_buck3_bias_dmd_sel_START (1)
#define PMIC_BUCK3_CTRL7_buck3_bias_dmd_sel_END (1)
#define PMIC_BUCK3_CTRL7_buck3_dmd_new_sel_START (2)
#define PMIC_BUCK3_CTRL7_buck3_dmd_new_sel_END (2)
#define PMIC_BUCK3_CTRL7_buck3_ton_dmd_START (3)
#define PMIC_BUCK3_CTRL7_buck3_ton_dmd_END (3)
#define PMIC_BUCK3_CTRL7_buck3_ocp_delay_START (4)
#define PMIC_BUCK3_CTRL7_buck3_ocp_delay_END (4)
#define PMIC_BUCK3_CTRL7_buck3_lx_dt_START (5)
#define PMIC_BUCK3_CTRL7_buck3_lx_dt_END (6)
#define PMIC_BUCK3_CTRL7_buck3_fb_cap_sel_START (7)
#define PMIC_BUCK3_CTRL7_buck3_fb_cap_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_fix_fq : 5;
        unsigned char buck3_fix_fq_ctrl : 1;
        unsigned char buck3_fix_ton : 2;
    } reg;
} PMIC_BUCK3_CTRL8_UNION;
#endif
#define PMIC_BUCK3_CTRL8_buck3_fix_fq_START (0)
#define PMIC_BUCK3_CTRL8_buck3_fix_fq_END (4)
#define PMIC_BUCK3_CTRL8_buck3_fix_fq_ctrl_START (5)
#define PMIC_BUCK3_CTRL8_buck3_fix_fq_ctrl_END (5)
#define PMIC_BUCK3_CTRL8_buck3_fix_ton_START (6)
#define PMIC_BUCK3_CTRL8_buck3_fix_ton_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_reserve : 8;
    } reg;
} PMIC_BUCK3_CTRL9_UNION;
#endif
#define PMIC_BUCK3_CTRL9_buck3_reserve_START (0)
#define PMIC_BUCK3_CTRL9_buck3_reserve_END (7)
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
        unsigned char reserved : 2;
        unsigned char buck4_dt_sel : 2;
        unsigned char buck4_ocp_sel : 2;
    } reg;
} PMIC_BUCK4_CTRL1_UNION;
#endif
#define PMIC_BUCK4_CTRL1_buck4_ng_dt_sel_START (0)
#define PMIC_BUCK4_CTRL1_buck4_ng_dt_sel_END (0)
#define PMIC_BUCK4_CTRL1_buck4_pg_dt_sel_START (1)
#define PMIC_BUCK4_CTRL1_buck4_pg_dt_sel_END (1)
#define PMIC_BUCK4_CTRL1_buck4_dt_sel_START (4)
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
        unsigned char reserved_0 : 2;
        unsigned char buck4_reg_en : 1;
        unsigned char buck4_adj_clx : 2;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_BUCK4_CTRL4_UNION;
#endif
#define PMIC_BUCK4_CTRL4_buck4_reg_r_START (0)
#define PMIC_BUCK4_CTRL4_buck4_reg_r_END (1)
#define PMIC_BUCK4_CTRL4_buck4_reg_en_START (4)
#define PMIC_BUCK4_CTRL4_buck4_reg_en_END (4)
#define PMIC_BUCK4_CTRL4_buck4_adj_clx_START (5)
#define PMIC_BUCK4_CTRL4_buck4_adj_clx_END (6)
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
        unsigned char buck4_eco_isel : 1;
        unsigned char buck4_bias_dmd_sel : 1;
        unsigned char buck4_dmd_new_sel : 1;
        unsigned char buck4_ton_dmd : 1;
        unsigned char buck4_ocp_delay : 1;
        unsigned char buck4_lx_dt : 2;
        unsigned char buck4_fb_cap_sel : 1;
    } reg;
} PMIC_BUCK4_CTRL7_UNION;
#endif
#define PMIC_BUCK4_CTRL7_buck4_eco_isel_START (0)
#define PMIC_BUCK4_CTRL7_buck4_eco_isel_END (0)
#define PMIC_BUCK4_CTRL7_buck4_bias_dmd_sel_START (1)
#define PMIC_BUCK4_CTRL7_buck4_bias_dmd_sel_END (1)
#define PMIC_BUCK4_CTRL7_buck4_dmd_new_sel_START (2)
#define PMIC_BUCK4_CTRL7_buck4_dmd_new_sel_END (2)
#define PMIC_BUCK4_CTRL7_buck4_ton_dmd_START (3)
#define PMIC_BUCK4_CTRL7_buck4_ton_dmd_END (3)
#define PMIC_BUCK4_CTRL7_buck4_ocp_delay_START (4)
#define PMIC_BUCK4_CTRL7_buck4_ocp_delay_END (4)
#define PMIC_BUCK4_CTRL7_buck4_lx_dt_START (5)
#define PMIC_BUCK4_CTRL7_buck4_lx_dt_END (6)
#define PMIC_BUCK4_CTRL7_buck4_fb_cap_sel_START (7)
#define PMIC_BUCK4_CTRL7_buck4_fb_cap_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_fix_fq : 5;
        unsigned char buck4_fix_fq_ctrl : 1;
        unsigned char buck4_fix_ton : 2;
    } reg;
} PMIC_BUCK4_CTRL8_UNION;
#endif
#define PMIC_BUCK4_CTRL8_buck4_fix_fq_START (0)
#define PMIC_BUCK4_CTRL8_buck4_fix_fq_END (4)
#define PMIC_BUCK4_CTRL8_buck4_fix_fq_ctrl_START (5)
#define PMIC_BUCK4_CTRL8_buck4_fix_fq_ctrl_END (5)
#define PMIC_BUCK4_CTRL8_buck4_fix_ton_START (6)
#define PMIC_BUCK4_CTRL8_buck4_fix_ton_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_reg_cap : 2;
        unsigned char buck4_reg_zero_en : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_BUCK4_CTRL9_UNION;
#endif
#define PMIC_BUCK4_CTRL9_buck4_reg_cap_START (0)
#define PMIC_BUCK4_CTRL9_buck4_reg_cap_END (1)
#define PMIC_BUCK4_CTRL9_buck4_reg_zero_en_START (2)
#define PMIC_BUCK4_CTRL9_buck4_reg_zero_en_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_reserve : 8;
    } reg;
} PMIC_BUCK4_CTRL10_UNION;
#endif
#define PMIC_BUCK4_CTRL10_buck4_reserve_START (0)
#define PMIC_BUCK4_CTRL10_buck4_reserve_END (7)
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
        unsigned char reserved : 2;
        unsigned char buck5_dt_sel : 2;
        unsigned char buck5_ocp_sel : 2;
    } reg;
} PMIC_BUCK5_CTRL1_UNION;
#endif
#define PMIC_BUCK5_CTRL1_buck5_ng_dt_sel_START (0)
#define PMIC_BUCK5_CTRL1_buck5_ng_dt_sel_END (0)
#define PMIC_BUCK5_CTRL1_buck5_pg_dt_sel_START (1)
#define PMIC_BUCK5_CTRL1_buck5_pg_dt_sel_END (1)
#define PMIC_BUCK5_CTRL1_buck5_dt_sel_START (4)
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
        unsigned char reserved_0 : 2;
        unsigned char buck5_reg_en : 1;
        unsigned char buck5_adj_clx : 2;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_BUCK5_CTRL4_UNION;
#endif
#define PMIC_BUCK5_CTRL4_buck5_reg_r_START (0)
#define PMIC_BUCK5_CTRL4_buck5_reg_r_END (1)
#define PMIC_BUCK5_CTRL4_buck5_reg_en_START (4)
#define PMIC_BUCK5_CTRL4_buck5_reg_en_END (4)
#define PMIC_BUCK5_CTRL4_buck5_adj_clx_START (5)
#define PMIC_BUCK5_CTRL4_buck5_adj_clx_END (6)
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
        unsigned char buck5_eco_isel : 1;
        unsigned char buck5_bias_dmd_sel : 1;
        unsigned char buck5_dmd_new_sel : 1;
        unsigned char buck5_ton_dmd : 1;
        unsigned char buck5_ocp_delay : 1;
        unsigned char buck5_lx_dt : 2;
        unsigned char buck5_fb_cap_sel : 1;
    } reg;
} PMIC_BUCK5_CTRL7_UNION;
#endif
#define PMIC_BUCK5_CTRL7_buck5_eco_isel_START (0)
#define PMIC_BUCK5_CTRL7_buck5_eco_isel_END (0)
#define PMIC_BUCK5_CTRL7_buck5_bias_dmd_sel_START (1)
#define PMIC_BUCK5_CTRL7_buck5_bias_dmd_sel_END (1)
#define PMIC_BUCK5_CTRL7_buck5_dmd_new_sel_START (2)
#define PMIC_BUCK5_CTRL7_buck5_dmd_new_sel_END (2)
#define PMIC_BUCK5_CTRL7_buck5_ton_dmd_START (3)
#define PMIC_BUCK5_CTRL7_buck5_ton_dmd_END (3)
#define PMIC_BUCK5_CTRL7_buck5_ocp_delay_START (4)
#define PMIC_BUCK5_CTRL7_buck5_ocp_delay_END (4)
#define PMIC_BUCK5_CTRL7_buck5_lx_dt_START (5)
#define PMIC_BUCK5_CTRL7_buck5_lx_dt_END (6)
#define PMIC_BUCK5_CTRL7_buck5_fb_cap_sel_START (7)
#define PMIC_BUCK5_CTRL7_buck5_fb_cap_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_fix_fq : 5;
        unsigned char buck5_fix_fq_ctrl : 1;
        unsigned char buck5_fix_ton : 2;
    } reg;
} PMIC_BUCK5_CTRL8_UNION;
#endif
#define PMIC_BUCK5_CTRL8_buck5_fix_fq_START (0)
#define PMIC_BUCK5_CTRL8_buck5_fix_fq_END (4)
#define PMIC_BUCK5_CTRL8_buck5_fix_fq_ctrl_START (5)
#define PMIC_BUCK5_CTRL8_buck5_fix_fq_ctrl_END (5)
#define PMIC_BUCK5_CTRL8_buck5_fix_ton_START (6)
#define PMIC_BUCK5_CTRL8_buck5_fix_ton_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_eco_dmd : 1;
        unsigned char buck5_reg_cap : 2;
        unsigned char buck5_reg_zero_en : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_BUCK5_CTRL9_UNION;
#endif
#define PMIC_BUCK5_CTRL9_buck5_eco_dmd_START (0)
#define PMIC_BUCK5_CTRL9_buck5_eco_dmd_END (0)
#define PMIC_BUCK5_CTRL9_buck5_reg_cap_START (1)
#define PMIC_BUCK5_CTRL9_buck5_reg_cap_END (2)
#define PMIC_BUCK5_CTRL9_buck5_reg_zero_en_START (3)
#define PMIC_BUCK5_CTRL9_buck5_reg_zero_en_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_reserve : 8;
    } reg;
} PMIC_BUCK5_CTRL10_UNION;
#endif
#define PMIC_BUCK5_CTRL10_buck5_reserve_START (0)
#define PMIC_BUCK5_CTRL10_buck5_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck6_dbias : 4;
        unsigned char buck6_adj_rlx : 4;
    } reg;
} PMIC_BUCK6_CTRL0_UNION;
#endif
#define PMIC_BUCK6_CTRL0_buck6_dbias_START (0)
#define PMIC_BUCK6_CTRL0_buck6_dbias_END (3)
#define PMIC_BUCK6_CTRL0_buck6_adj_rlx_START (4)
#define PMIC_BUCK6_CTRL0_buck6_adj_rlx_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck6_ng_dt_sel : 1;
        unsigned char buck6_pg_dt_sel : 1;
        unsigned char reserved : 2;
        unsigned char buck6_dt_sel : 2;
        unsigned char buck6_ocp_sel : 2;
    } reg;
} PMIC_BUCK6_CTRL1_UNION;
#endif
#define PMIC_BUCK6_CTRL1_buck6_ng_dt_sel_START (0)
#define PMIC_BUCK6_CTRL1_buck6_ng_dt_sel_END (0)
#define PMIC_BUCK6_CTRL1_buck6_pg_dt_sel_START (1)
#define PMIC_BUCK6_CTRL1_buck6_pg_dt_sel_END (1)
#define PMIC_BUCK6_CTRL1_buck6_dt_sel_START (4)
#define PMIC_BUCK6_CTRL1_buck6_dt_sel_END (5)
#define PMIC_BUCK6_CTRL1_buck6_ocp_sel_START (6)
#define PMIC_BUCK6_CTRL1_buck6_ocp_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck6_pg_n_sel : 3;
        unsigned char buck6_pg_p_sel : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK6_CTRL2_UNION;
#endif
#define PMIC_BUCK6_CTRL2_buck6_pg_n_sel_START (0)
#define PMIC_BUCK6_CTRL2_buck6_pg_n_sel_END (2)
#define PMIC_BUCK6_CTRL2_buck6_pg_p_sel_START (3)
#define PMIC_BUCK6_CTRL2_buck6_pg_p_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck6_ng_n_sel : 3;
        unsigned char buck6_ng_p_sel : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK6_CTRL3_UNION;
#endif
#define PMIC_BUCK6_CTRL3_buck6_ng_n_sel_START (0)
#define PMIC_BUCK6_CTRL3_buck6_ng_n_sel_END (2)
#define PMIC_BUCK6_CTRL3_buck6_ng_p_sel_START (3)
#define PMIC_BUCK6_CTRL3_buck6_ng_p_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck6_reg_r : 2;
        unsigned char reserved_0 : 2;
        unsigned char buck6_reg_en : 1;
        unsigned char buck6_adj_clx : 2;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_BUCK6_CTRL4_UNION;
#endif
#define PMIC_BUCK6_CTRL4_buck6_reg_r_START (0)
#define PMIC_BUCK6_CTRL4_buck6_reg_r_END (1)
#define PMIC_BUCK6_CTRL4_buck6_reg_en_START (4)
#define PMIC_BUCK6_CTRL4_buck6_reg_en_END (4)
#define PMIC_BUCK6_CTRL4_buck6_adj_clx_START (5)
#define PMIC_BUCK6_CTRL4_buck6_adj_clx_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck6_ton : 3;
        unsigned char buck6_reg_bias : 1;
        unsigned char buck6_regop_c : 1;
        unsigned char buck6_reg_dr : 3;
    } reg;
} PMIC_BUCK6_CTRL5_UNION;
#endif
#define PMIC_BUCK6_CTRL5_buck6_ton_START (0)
#define PMIC_BUCK6_CTRL5_buck6_ton_END (2)
#define PMIC_BUCK6_CTRL5_buck6_reg_bias_START (3)
#define PMIC_BUCK6_CTRL5_buck6_reg_bias_END (3)
#define PMIC_BUCK6_CTRL5_buck6_regop_c_START (4)
#define PMIC_BUCK6_CTRL5_buck6_regop_c_END (4)
#define PMIC_BUCK6_CTRL5_buck6_reg_dr_START (5)
#define PMIC_BUCK6_CTRL5_buck6_reg_dr_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck6_dmd_ton : 3;
        unsigned char buck6_ocp_toff : 2;
        unsigned char buck6_dmd_sel : 3;
    } reg;
} PMIC_BUCK6_CTRL6_UNION;
#endif
#define PMIC_BUCK6_CTRL6_buck6_dmd_ton_START (0)
#define PMIC_BUCK6_CTRL6_buck6_dmd_ton_END (2)
#define PMIC_BUCK6_CTRL6_buck6_ocp_toff_START (3)
#define PMIC_BUCK6_CTRL6_buck6_ocp_toff_END (4)
#define PMIC_BUCK6_CTRL6_buck6_dmd_sel_START (5)
#define PMIC_BUCK6_CTRL6_buck6_dmd_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck6_eco_isel : 1;
        unsigned char buck6_bias_dmd_sel : 1;
        unsigned char buck6_dmd_new_sel : 1;
        unsigned char buck6_ton_dmd : 1;
        unsigned char buck6_ocp_delay : 1;
        unsigned char buck6_lx_dt : 2;
        unsigned char buck6_fb_cap_sel : 1;
    } reg;
} PMIC_BUCK6_CTRL7_UNION;
#endif
#define PMIC_BUCK6_CTRL7_buck6_eco_isel_START (0)
#define PMIC_BUCK6_CTRL7_buck6_eco_isel_END (0)
#define PMIC_BUCK6_CTRL7_buck6_bias_dmd_sel_START (1)
#define PMIC_BUCK6_CTRL7_buck6_bias_dmd_sel_END (1)
#define PMIC_BUCK6_CTRL7_buck6_dmd_new_sel_START (2)
#define PMIC_BUCK6_CTRL7_buck6_dmd_new_sel_END (2)
#define PMIC_BUCK6_CTRL7_buck6_ton_dmd_START (3)
#define PMIC_BUCK6_CTRL7_buck6_ton_dmd_END (3)
#define PMIC_BUCK6_CTRL7_buck6_ocp_delay_START (4)
#define PMIC_BUCK6_CTRL7_buck6_ocp_delay_END (4)
#define PMIC_BUCK6_CTRL7_buck6_lx_dt_START (5)
#define PMIC_BUCK6_CTRL7_buck6_lx_dt_END (6)
#define PMIC_BUCK6_CTRL7_buck6_fb_cap_sel_START (7)
#define PMIC_BUCK6_CTRL7_buck6_fb_cap_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck6_fix_fq : 5;
        unsigned char buck6_fix_fq_ctrl : 1;
        unsigned char buck6_fix_ton : 2;
    } reg;
} PMIC_BUCK6_CTRL8_UNION;
#endif
#define PMIC_BUCK6_CTRL8_buck6_fix_fq_START (0)
#define PMIC_BUCK6_CTRL8_buck6_fix_fq_END (4)
#define PMIC_BUCK6_CTRL8_buck6_fix_fq_ctrl_START (5)
#define PMIC_BUCK6_CTRL8_buck6_fix_fq_ctrl_END (5)
#define PMIC_BUCK6_CTRL8_buck6_fix_ton_START (6)
#define PMIC_BUCK6_CTRL8_buck6_fix_ton_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck6_reg_cap : 2;
        unsigned char buck6_reg_zero_en : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_BUCK6_CTRL9_UNION;
#endif
#define PMIC_BUCK6_CTRL9_buck6_reg_cap_START (0)
#define PMIC_BUCK6_CTRL9_buck6_reg_cap_END (1)
#define PMIC_BUCK6_CTRL9_buck6_reg_zero_en_START (2)
#define PMIC_BUCK6_CTRL9_buck6_reg_zero_en_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck6_reserve : 8;
    } reg;
} PMIC_BUCK6_CTRL10_UNION;
#endif
#define PMIC_BUCK6_CTRL10_buck6_reserve_START (0)
#define PMIC_BUCK6_CTRL10_buck6_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_dbias : 4;
        unsigned char buck7_adj_rlx : 4;
    } reg;
} PMIC_BUCK7_CTRL0_UNION;
#endif
#define PMIC_BUCK7_CTRL0_buck7_dbias_START (0)
#define PMIC_BUCK7_CTRL0_buck7_dbias_END (3)
#define PMIC_BUCK7_CTRL0_buck7_adj_rlx_START (4)
#define PMIC_BUCK7_CTRL0_buck7_adj_rlx_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_ng_dt_sel : 1;
        unsigned char buck7_pg_dt_sel : 1;
        unsigned char reserved : 2;
        unsigned char buck7_dt_sel : 2;
        unsigned char buck7_ocp_sel : 2;
    } reg;
} PMIC_BUCK7_CTRL1_UNION;
#endif
#define PMIC_BUCK7_CTRL1_buck7_ng_dt_sel_START (0)
#define PMIC_BUCK7_CTRL1_buck7_ng_dt_sel_END (0)
#define PMIC_BUCK7_CTRL1_buck7_pg_dt_sel_START (1)
#define PMIC_BUCK7_CTRL1_buck7_pg_dt_sel_END (1)
#define PMIC_BUCK7_CTRL1_buck7_dt_sel_START (4)
#define PMIC_BUCK7_CTRL1_buck7_dt_sel_END (5)
#define PMIC_BUCK7_CTRL1_buck7_ocp_sel_START (6)
#define PMIC_BUCK7_CTRL1_buck7_ocp_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_pg_n_sel : 3;
        unsigned char buck7_pg_p_sel : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK7_CTRL2_UNION;
#endif
#define PMIC_BUCK7_CTRL2_buck7_pg_n_sel_START (0)
#define PMIC_BUCK7_CTRL2_buck7_pg_n_sel_END (2)
#define PMIC_BUCK7_CTRL2_buck7_pg_p_sel_START (3)
#define PMIC_BUCK7_CTRL2_buck7_pg_p_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_ng_n_sel : 3;
        unsigned char buck7_ng_p_sel : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK7_CTRL3_UNION;
#endif
#define PMIC_BUCK7_CTRL3_buck7_ng_n_sel_START (0)
#define PMIC_BUCK7_CTRL3_buck7_ng_n_sel_END (2)
#define PMIC_BUCK7_CTRL3_buck7_ng_p_sel_START (3)
#define PMIC_BUCK7_CTRL3_buck7_ng_p_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_reg_r : 2;
        unsigned char reserved_0 : 2;
        unsigned char buck7_reg_en : 1;
        unsigned char buck7_adj_clx : 2;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_BUCK7_CTRL4_UNION;
#endif
#define PMIC_BUCK7_CTRL4_buck7_reg_r_START (0)
#define PMIC_BUCK7_CTRL4_buck7_reg_r_END (1)
#define PMIC_BUCK7_CTRL4_buck7_reg_en_START (4)
#define PMIC_BUCK7_CTRL4_buck7_reg_en_END (4)
#define PMIC_BUCK7_CTRL4_buck7_adj_clx_START (5)
#define PMIC_BUCK7_CTRL4_buck7_adj_clx_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_ton : 3;
        unsigned char buck7_reg_bias : 1;
        unsigned char buck7_regop_c : 1;
        unsigned char buck7_reg_dr : 3;
    } reg;
} PMIC_BUCK7_CTRL5_UNION;
#endif
#define PMIC_BUCK7_CTRL5_buck7_ton_START (0)
#define PMIC_BUCK7_CTRL5_buck7_ton_END (2)
#define PMIC_BUCK7_CTRL5_buck7_reg_bias_START (3)
#define PMIC_BUCK7_CTRL5_buck7_reg_bias_END (3)
#define PMIC_BUCK7_CTRL5_buck7_regop_c_START (4)
#define PMIC_BUCK7_CTRL5_buck7_regop_c_END (4)
#define PMIC_BUCK7_CTRL5_buck7_reg_dr_START (5)
#define PMIC_BUCK7_CTRL5_buck7_reg_dr_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_dmd_ton : 3;
        unsigned char buck7_ocp_toff : 2;
        unsigned char buck7_dmd_sel : 3;
    } reg;
} PMIC_BUCK7_CTRL6_UNION;
#endif
#define PMIC_BUCK7_CTRL6_buck7_dmd_ton_START (0)
#define PMIC_BUCK7_CTRL6_buck7_dmd_ton_END (2)
#define PMIC_BUCK7_CTRL6_buck7_ocp_toff_START (3)
#define PMIC_BUCK7_CTRL6_buck7_ocp_toff_END (4)
#define PMIC_BUCK7_CTRL6_buck7_dmd_sel_START (5)
#define PMIC_BUCK7_CTRL6_buck7_dmd_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_sense_ctrl : 1;
        unsigned char buck7_sense_r : 6;
        unsigned char buck7_sense_dmd : 1;
    } reg;
} PMIC_BUCK7_CTRL7_UNION;
#endif
#define PMIC_BUCK7_CTRL7_buck7_sense_ctrl_START (0)
#define PMIC_BUCK7_CTRL7_buck7_sense_ctrl_END (0)
#define PMIC_BUCK7_CTRL7_buck7_sense_r_START (1)
#define PMIC_BUCK7_CTRL7_buck7_sense_r_END (6)
#define PMIC_BUCK7_CTRL7_buck7_sense_dmd_START (7)
#define PMIC_BUCK7_CTRL7_buck7_sense_dmd_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_sense_filter : 4;
        unsigned char buck7_sense_sel : 1;
        unsigned char buck7_ton_dmd : 1;
        unsigned char buck7_ocp_delay : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK7_CTRL8_UNION;
#endif
#define PMIC_BUCK7_CTRL8_buck7_sense_filter_START (0)
#define PMIC_BUCK7_CTRL8_buck7_sense_filter_END (3)
#define PMIC_BUCK7_CTRL8_buck7_sense_sel_START (4)
#define PMIC_BUCK7_CTRL8_buck7_sense_sel_END (4)
#define PMIC_BUCK7_CTRL8_buck7_ton_dmd_START (5)
#define PMIC_BUCK7_CTRL8_buck7_ton_dmd_END (5)
#define PMIC_BUCK7_CTRL8_buck7_ocp_delay_START (6)
#define PMIC_BUCK7_CTRL8_buck7_ocp_delay_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_sense_rev0 : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_BUCK7_CTRL9_UNION;
#endif
#define PMIC_BUCK7_CTRL9_buck7_sense_rev0_START (0)
#define PMIC_BUCK7_CTRL9_buck7_sense_rev0_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_eco_isel : 1;
        unsigned char buck7_bias_dmd_sel : 1;
        unsigned char buck7_dmd_new_sel : 1;
        unsigned char buck7_lx_dt : 2;
        unsigned char buck7_fb_cap_sel : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK7_CTRL10_UNION;
#endif
#define PMIC_BUCK7_CTRL10_buck7_eco_isel_START (0)
#define PMIC_BUCK7_CTRL10_buck7_eco_isel_END (0)
#define PMIC_BUCK7_CTRL10_buck7_bias_dmd_sel_START (1)
#define PMIC_BUCK7_CTRL10_buck7_bias_dmd_sel_END (1)
#define PMIC_BUCK7_CTRL10_buck7_dmd_new_sel_START (2)
#define PMIC_BUCK7_CTRL10_buck7_dmd_new_sel_END (2)
#define PMIC_BUCK7_CTRL10_buck7_lx_dt_START (3)
#define PMIC_BUCK7_CTRL10_buck7_lx_dt_END (4)
#define PMIC_BUCK7_CTRL10_buck7_fb_cap_sel_START (5)
#define PMIC_BUCK7_CTRL10_buck7_fb_cap_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_fix_fq : 5;
        unsigned char buck7_fix_fq_ctrl : 1;
        unsigned char buck7_fix_ton : 2;
    } reg;
} PMIC_BUCK7_CTRL11_UNION;
#endif
#define PMIC_BUCK7_CTRL11_buck7_fix_fq_START (0)
#define PMIC_BUCK7_CTRL11_buck7_fix_fq_END (4)
#define PMIC_BUCK7_CTRL11_buck7_fix_fq_ctrl_START (5)
#define PMIC_BUCK7_CTRL11_buck7_fix_fq_ctrl_END (5)
#define PMIC_BUCK7_CTRL11_buck7_fix_ton_START (6)
#define PMIC_BUCK7_CTRL11_buck7_fix_ton_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_reserve : 8;
    } reg;
} PMIC_BUCK7_CTRL12_UNION;
#endif
#define PMIC_BUCK7_CTRL12_buck7_reserve_START (0)
#define PMIC_BUCK7_CTRL12_buck7_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck8_dbias : 4;
        unsigned char buck8_adj_rlx : 4;
    } reg;
} PMIC_BUCK8_CTRL0_UNION;
#endif
#define PMIC_BUCK8_CTRL0_buck8_dbias_START (0)
#define PMIC_BUCK8_CTRL0_buck8_dbias_END (3)
#define PMIC_BUCK8_CTRL0_buck8_adj_rlx_START (4)
#define PMIC_BUCK8_CTRL0_buck8_adj_rlx_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck8_ng_dt_sel : 1;
        unsigned char buck8_pg_dt_sel : 1;
        unsigned char reserved_0 : 2;
        unsigned char buck8_dt_sel : 2;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_BUCK8_CTRL1_UNION;
#endif
#define PMIC_BUCK8_CTRL1_buck8_ng_dt_sel_START (0)
#define PMIC_BUCK8_CTRL1_buck8_ng_dt_sel_END (0)
#define PMIC_BUCK8_CTRL1_buck8_pg_dt_sel_START (1)
#define PMIC_BUCK8_CTRL1_buck8_pg_dt_sel_END (1)
#define PMIC_BUCK8_CTRL1_buck8_dt_sel_START (4)
#define PMIC_BUCK8_CTRL1_buck8_dt_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck8_pg_n_sel : 3;
        unsigned char buck8_pg_p_sel : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK8_CTRL2_UNION;
#endif
#define PMIC_BUCK8_CTRL2_buck8_pg_n_sel_START (0)
#define PMIC_BUCK8_CTRL2_buck8_pg_n_sel_END (2)
#define PMIC_BUCK8_CTRL2_buck8_pg_p_sel_START (3)
#define PMIC_BUCK8_CTRL2_buck8_pg_p_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck8_ng_n_sel : 3;
        unsigned char buck8_ng_p_sel : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK8_CTRL3_UNION;
#endif
#define PMIC_BUCK8_CTRL3_buck8_ng_n_sel_START (0)
#define PMIC_BUCK8_CTRL3_buck8_ng_n_sel_END (2)
#define PMIC_BUCK8_CTRL3_buck8_ng_p_sel_START (3)
#define PMIC_BUCK8_CTRL3_buck8_ng_p_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck8_reg_r : 2;
        unsigned char reserved_0 : 2;
        unsigned char buck8_reg_en : 1;
        unsigned char buck8_adj_clx : 2;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_BUCK8_CTRL4_UNION;
#endif
#define PMIC_BUCK8_CTRL4_buck8_reg_r_START (0)
#define PMIC_BUCK8_CTRL4_buck8_reg_r_END (1)
#define PMIC_BUCK8_CTRL4_buck8_reg_en_START (4)
#define PMIC_BUCK8_CTRL4_buck8_reg_en_END (4)
#define PMIC_BUCK8_CTRL4_buck8_adj_clx_START (5)
#define PMIC_BUCK8_CTRL4_buck8_adj_clx_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck8_ton : 3;
        unsigned char buck8_reg_bias : 1;
        unsigned char buck8_regop_c : 1;
        unsigned char buck8_reg_dr : 3;
    } reg;
} PMIC_BUCK8_CTRL5_UNION;
#endif
#define PMIC_BUCK8_CTRL5_buck8_ton_START (0)
#define PMIC_BUCK8_CTRL5_buck8_ton_END (2)
#define PMIC_BUCK8_CTRL5_buck8_reg_bias_START (3)
#define PMIC_BUCK8_CTRL5_buck8_reg_bias_END (3)
#define PMIC_BUCK8_CTRL5_buck8_regop_c_START (4)
#define PMIC_BUCK8_CTRL5_buck8_regop_c_END (4)
#define PMIC_BUCK8_CTRL5_buck8_reg_dr_START (5)
#define PMIC_BUCK8_CTRL5_buck8_reg_dr_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck8_dmd_ton : 3;
        unsigned char buck8_ocp_toff : 2;
        unsigned char buck8_dmd_sel : 3;
    } reg;
} PMIC_BUCK8_CTRL6_UNION;
#endif
#define PMIC_BUCK8_CTRL6_buck8_dmd_ton_START (0)
#define PMIC_BUCK8_CTRL6_buck8_dmd_ton_END (2)
#define PMIC_BUCK8_CTRL6_buck8_ocp_toff_START (3)
#define PMIC_BUCK8_CTRL6_buck8_ocp_toff_END (4)
#define PMIC_BUCK8_CTRL6_buck8_dmd_sel_START (5)
#define PMIC_BUCK8_CTRL6_buck8_dmd_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck8_eco_isel : 1;
        unsigned char buck8_bias_dmd_sel : 1;
        unsigned char buck8_dmd_new_sel : 1;
        unsigned char buck8_ton_dmd : 1;
        unsigned char buck8_ocp_delay : 1;
        unsigned char buck8_lx_dt : 2;
        unsigned char buck8_fb_cap_sel : 1;
    } reg;
} PMIC_BUCK8_CTRL7_UNION;
#endif
#define PMIC_BUCK8_CTRL7_buck8_eco_isel_START (0)
#define PMIC_BUCK8_CTRL7_buck8_eco_isel_END (0)
#define PMIC_BUCK8_CTRL7_buck8_bias_dmd_sel_START (1)
#define PMIC_BUCK8_CTRL7_buck8_bias_dmd_sel_END (1)
#define PMIC_BUCK8_CTRL7_buck8_dmd_new_sel_START (2)
#define PMIC_BUCK8_CTRL7_buck8_dmd_new_sel_END (2)
#define PMIC_BUCK8_CTRL7_buck8_ton_dmd_START (3)
#define PMIC_BUCK8_CTRL7_buck8_ton_dmd_END (3)
#define PMIC_BUCK8_CTRL7_buck8_ocp_delay_START (4)
#define PMIC_BUCK8_CTRL7_buck8_ocp_delay_END (4)
#define PMIC_BUCK8_CTRL7_buck8_lx_dt_START (5)
#define PMIC_BUCK8_CTRL7_buck8_lx_dt_END (6)
#define PMIC_BUCK8_CTRL7_buck8_fb_cap_sel_START (7)
#define PMIC_BUCK8_CTRL7_buck8_fb_cap_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck8_fix_fq : 5;
        unsigned char buck8_fix_fq_ctrl : 1;
        unsigned char buck8_fix_ton : 2;
    } reg;
} PMIC_BUCK8_CTRL8_UNION;
#endif
#define PMIC_BUCK8_CTRL8_buck8_fix_fq_START (0)
#define PMIC_BUCK8_CTRL8_buck8_fix_fq_END (4)
#define PMIC_BUCK8_CTRL8_buck8_fix_fq_ctrl_START (5)
#define PMIC_BUCK8_CTRL8_buck8_fix_fq_ctrl_END (5)
#define PMIC_BUCK8_CTRL8_buck8_fix_ton_START (6)
#define PMIC_BUCK8_CTRL8_buck8_fix_ton_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck8_reserve : 8;
    } reg;
} PMIC_BUCK8_CTRL9_UNION;
#endif
#define PMIC_BUCK8_CTRL9_buck8_reserve_START (0)
#define PMIC_BUCK8_CTRL9_buck8_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck9_dbias : 4;
        unsigned char buck9_adj_rlx : 4;
    } reg;
} PMIC_BUCK9_CTRL0_UNION;
#endif
#define PMIC_BUCK9_CTRL0_buck9_dbias_START (0)
#define PMIC_BUCK9_CTRL0_buck9_dbias_END (3)
#define PMIC_BUCK9_CTRL0_buck9_adj_rlx_START (4)
#define PMIC_BUCK9_CTRL0_buck9_adj_rlx_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck9_ng_dt_sel : 1;
        unsigned char buck9_pg_dt_sel : 1;
        unsigned char reserved : 2;
        unsigned char buck9_dt_sel : 2;
        unsigned char buck9_ocp_sel : 2;
    } reg;
} PMIC_BUCK9_CTRL1_UNION;
#endif
#define PMIC_BUCK9_CTRL1_buck9_ng_dt_sel_START (0)
#define PMIC_BUCK9_CTRL1_buck9_ng_dt_sel_END (0)
#define PMIC_BUCK9_CTRL1_buck9_pg_dt_sel_START (1)
#define PMIC_BUCK9_CTRL1_buck9_pg_dt_sel_END (1)
#define PMIC_BUCK9_CTRL1_buck9_dt_sel_START (4)
#define PMIC_BUCK9_CTRL1_buck9_dt_sel_END (5)
#define PMIC_BUCK9_CTRL1_buck9_ocp_sel_START (6)
#define PMIC_BUCK9_CTRL1_buck9_ocp_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck9_pg_n_sel : 3;
        unsigned char buck9_pg_p_sel : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK9_CTRL2_UNION;
#endif
#define PMIC_BUCK9_CTRL2_buck9_pg_n_sel_START (0)
#define PMIC_BUCK9_CTRL2_buck9_pg_n_sel_END (2)
#define PMIC_BUCK9_CTRL2_buck9_pg_p_sel_START (3)
#define PMIC_BUCK9_CTRL2_buck9_pg_p_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck9_ng_n_sel : 3;
        unsigned char buck9_ng_p_sel : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK9_CTRL3_UNION;
#endif
#define PMIC_BUCK9_CTRL3_buck9_ng_n_sel_START (0)
#define PMIC_BUCK9_CTRL3_buck9_ng_n_sel_END (2)
#define PMIC_BUCK9_CTRL3_buck9_ng_p_sel_START (3)
#define PMIC_BUCK9_CTRL3_buck9_ng_p_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck9_reg_r : 2;
        unsigned char reserved_0 : 2;
        unsigned char buck9_reg_en : 1;
        unsigned char buck9_adj_clx : 2;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_BUCK9_CTRL4_UNION;
#endif
#define PMIC_BUCK9_CTRL4_buck9_reg_r_START (0)
#define PMIC_BUCK9_CTRL4_buck9_reg_r_END (1)
#define PMIC_BUCK9_CTRL4_buck9_reg_en_START (4)
#define PMIC_BUCK9_CTRL4_buck9_reg_en_END (4)
#define PMIC_BUCK9_CTRL4_buck9_adj_clx_START (5)
#define PMIC_BUCK9_CTRL4_buck9_adj_clx_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck9_ton : 3;
        unsigned char buck9_reg_bias : 1;
        unsigned char buck9_regop_c : 1;
        unsigned char buck9_reg_dr : 3;
    } reg;
} PMIC_BUCK9_CTRL5_UNION;
#endif
#define PMIC_BUCK9_CTRL5_buck9_ton_START (0)
#define PMIC_BUCK9_CTRL5_buck9_ton_END (2)
#define PMIC_BUCK9_CTRL5_buck9_reg_bias_START (3)
#define PMIC_BUCK9_CTRL5_buck9_reg_bias_END (3)
#define PMIC_BUCK9_CTRL5_buck9_regop_c_START (4)
#define PMIC_BUCK9_CTRL5_buck9_regop_c_END (4)
#define PMIC_BUCK9_CTRL5_buck9_reg_dr_START (5)
#define PMIC_BUCK9_CTRL5_buck9_reg_dr_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck9_dmd_sel : 3;
        unsigned char buck9_ocp_toff : 2;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK9_CTRL6_UNION;
#endif
#define PMIC_BUCK9_CTRL6_buck9_dmd_sel_START (0)
#define PMIC_BUCK9_CTRL6_buck9_dmd_sel_END (2)
#define PMIC_BUCK9_CTRL6_buck9_ocp_toff_START (3)
#define PMIC_BUCK9_CTRL6_buck9_ocp_toff_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck9_eco_isel : 1;
        unsigned char buck9_bias_dmd_sel : 1;
        unsigned char buck9_dmd_new_sel : 1;
        unsigned char buck9_ton_dmd : 1;
        unsigned char buck9_ocp_delay : 1;
        unsigned char buck9_lx_dt : 2;
        unsigned char buck9_fb_cap_sel : 1;
    } reg;
} PMIC_BUCK9_CTRL7_UNION;
#endif
#define PMIC_BUCK9_CTRL7_buck9_eco_isel_START (0)
#define PMIC_BUCK9_CTRL7_buck9_eco_isel_END (0)
#define PMIC_BUCK9_CTRL7_buck9_bias_dmd_sel_START (1)
#define PMIC_BUCK9_CTRL7_buck9_bias_dmd_sel_END (1)
#define PMIC_BUCK9_CTRL7_buck9_dmd_new_sel_START (2)
#define PMIC_BUCK9_CTRL7_buck9_dmd_new_sel_END (2)
#define PMIC_BUCK9_CTRL7_buck9_ton_dmd_START (3)
#define PMIC_BUCK9_CTRL7_buck9_ton_dmd_END (3)
#define PMIC_BUCK9_CTRL7_buck9_ocp_delay_START (4)
#define PMIC_BUCK9_CTRL7_buck9_ocp_delay_END (4)
#define PMIC_BUCK9_CTRL7_buck9_lx_dt_START (5)
#define PMIC_BUCK9_CTRL7_buck9_lx_dt_END (6)
#define PMIC_BUCK9_CTRL7_buck9_fb_cap_sel_START (7)
#define PMIC_BUCK9_CTRL7_buck9_fb_cap_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck9_fix_fq : 5;
        unsigned char buck9_fix_fq_ctrl : 1;
        unsigned char buck9_fix_ton : 2;
    } reg;
} PMIC_BUCK9_CTRL8_UNION;
#endif
#define PMIC_BUCK9_CTRL8_buck9_fix_fq_START (0)
#define PMIC_BUCK9_CTRL8_buck9_fix_fq_END (4)
#define PMIC_BUCK9_CTRL8_buck9_fix_fq_ctrl_START (5)
#define PMIC_BUCK9_CTRL8_buck9_fix_fq_ctrl_END (5)
#define PMIC_BUCK9_CTRL8_buck9_fix_ton_START (6)
#define PMIC_BUCK9_CTRL8_buck9_fix_ton_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck9_reserve : 8;
    } reg;
} PMIC_BUCK9_CTRL9_UNION;
#endif
#define PMIC_BUCK9_CTRL9_buck9_reserve_START (0)
#define PMIC_BUCK9_CTRL9_buck9_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck_cmp_filter : 1;
        unsigned char buck_filter_ton : 2;
        unsigned char buck_dmd_clamp : 1;
        unsigned char buck_regop_clamp : 1;
        unsigned char buck_short_pdp : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK_CTRL0_UNION;
#endif
#define PMIC_BUCK_CTRL0_buck_cmp_filter_START (0)
#define PMIC_BUCK_CTRL0_buck_cmp_filter_END (0)
#define PMIC_BUCK_CTRL0_buck_filter_ton_START (1)
#define PMIC_BUCK_CTRL0_buck_filter_ton_END (2)
#define PMIC_BUCK_CTRL0_buck_dmd_clamp_START (3)
#define PMIC_BUCK_CTRL0_buck_dmd_clamp_END (3)
#define PMIC_BUCK_CTRL0_buck_regop_clamp_START (4)
#define PMIC_BUCK_CTRL0_buck_regop_clamp_END (4)
#define PMIC_BUCK_CTRL0_buck_short_pdp_START (5)
#define PMIC_BUCK_CTRL0_buck_short_pdp_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck_reg_ss : 1;
        unsigned char buck_ocp_dis : 1;
        unsigned char buck_ocptrim_ctl : 3;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK_CTRL1_UNION;
#endif
#define PMIC_BUCK_CTRL1_buck_reg_ss_START (0)
#define PMIC_BUCK_CTRL1_buck_reg_ss_END (0)
#define PMIC_BUCK_CTRL1_buck_ocp_dis_START (1)
#define PMIC_BUCK_CTRL1_buck_ocp_dis_END (1)
#define PMIC_BUCK_CTRL1_buck_ocptrim_ctl_START (2)
#define PMIC_BUCK_CTRL1_buck_ocptrim_ctl_END (4)
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
        unsigned char ldo0_vrset : 3;
        unsigned char ldo0_ocp_enn : 1;
        unsigned char ldo0_eco_set : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO0_CTRL0_UNION;
#endif
#define PMIC_LDO0_CTRL0_ldo0_vrset_START (0)
#define PMIC_LDO0_CTRL0_ldo0_vrset_END (2)
#define PMIC_LDO0_CTRL0_ldo0_ocp_enn_START (3)
#define PMIC_LDO0_CTRL0_ldo0_ocp_enn_END (3)
#define PMIC_LDO0_CTRL0_ldo0_eco_set_START (4)
#define PMIC_LDO0_CTRL0_ldo0_eco_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo0_ref_buf_bypass : 1;
        unsigned char ldo0_psrr2_set : 3;
        unsigned char ldo0_psrr1_set : 3;
        unsigned char ldo0_psrr_en : 1;
    } reg;
} PMIC_LDO0_CTRL1_UNION;
#endif
#define PMIC_LDO0_CTRL1_ldo0_ref_buf_bypass_START (0)
#define PMIC_LDO0_CTRL1_ldo0_ref_buf_bypass_END (0)
#define PMIC_LDO0_CTRL1_ldo0_psrr2_set_START (1)
#define PMIC_LDO0_CTRL1_ldo0_psrr2_set_END (3)
#define PMIC_LDO0_CTRL1_ldo0_psrr1_set_START (4)
#define PMIC_LDO0_CTRL1_ldo0_psrr1_set_END (6)
#define PMIC_LDO0_CTRL1_ldo0_psrr_en_START (7)
#define PMIC_LDO0_CTRL1_ldo0_psrr_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo0_ocps_en : 1;
        unsigned char ldo0_bw_en : 1;
        unsigned char ldo0_vgpr_en : 1;
        unsigned char ldo0_ref_buf_rset : 2;
        unsigned char ldo0_comp : 3;
    } reg;
} PMIC_LDO0_CTRL2_UNION;
#endif
#define PMIC_LDO0_CTRL2_ldo0_ocps_en_START (0)
#define PMIC_LDO0_CTRL2_ldo0_ocps_en_END (0)
#define PMIC_LDO0_CTRL2_ldo0_bw_en_START (1)
#define PMIC_LDO0_CTRL2_ldo0_bw_en_END (1)
#define PMIC_LDO0_CTRL2_ldo0_vgpr_en_START (2)
#define PMIC_LDO0_CTRL2_ldo0_vgpr_en_END (2)
#define PMIC_LDO0_CTRL2_ldo0_ref_buf_rset_START (3)
#define PMIC_LDO0_CTRL2_ldo0_ref_buf_rset_END (4)
#define PMIC_LDO0_CTRL2_ldo0_comp_START (5)
#define PMIC_LDO0_CTRL2_ldo0_comp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo0_ocpd_set : 2;
        unsigned char ldo0_ocps_set : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO0_CTRL3_UNION;
#endif
#define PMIC_LDO0_CTRL3_ldo0_ocpd_set_START (0)
#define PMIC_LDO0_CTRL3_ldo0_ocpd_set_END (1)
#define PMIC_LDO0_CTRL3_ldo0_ocps_set_START (2)
#define PMIC_LDO0_CTRL3_ldo0_ocps_set_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo1_vrset : 3;
        unsigned char ldo1_ocp_enn : 1;
        unsigned char ldo1_iss_en : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO1_CTRL0_UNION;
#endif
#define PMIC_LDO1_CTRL0_ldo1_vrset_START (0)
#define PMIC_LDO1_CTRL0_ldo1_vrset_END (2)
#define PMIC_LDO1_CTRL0_ldo1_ocp_enn_START (3)
#define PMIC_LDO1_CTRL0_ldo1_ocp_enn_END (3)
#define PMIC_LDO1_CTRL0_ldo1_iss_en_START (4)
#define PMIC_LDO1_CTRL0_ldo1_iss_en_END (4)
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
        unsigned char ldo2_iss_en : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO2_CTRL_UNION;
#endif
#define PMIC_LDO2_CTRL_ldo2_vrset_START (0)
#define PMIC_LDO2_CTRL_ldo2_vrset_END (2)
#define PMIC_LDO2_CTRL_ldo2_ocp_enn_START (3)
#define PMIC_LDO2_CTRL_ldo2_ocp_enn_END (3)
#define PMIC_LDO2_CTRL_ldo2_iss_en_START (4)
#define PMIC_LDO2_CTRL_ldo2_iss_en_END (4)
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
        unsigned char ldo4_iss_en : 1;
        unsigned char ldo4_eco_set : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO4_CTRL_UNION;
#endif
#define PMIC_LDO4_CTRL_ldo4_vrset_START (0)
#define PMIC_LDO4_CTRL_ldo4_vrset_END (2)
#define PMIC_LDO4_CTRL_ldo4_ocp_enn_START (3)
#define PMIC_LDO4_CTRL_ldo4_ocp_enn_END (3)
#define PMIC_LDO4_CTRL_ldo4_iss_en_START (4)
#define PMIC_LDO4_CTRL_ldo4_iss_en_END (4)
#define PMIC_LDO4_CTRL_ldo4_eco_set_START (5)
#define PMIC_LDO4_CTRL_ldo4_eco_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo5_vrset : 3;
        unsigned char ldo5_ocp_enn : 1;
        unsigned char ldo5_iss_en : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO5_CTRL_UNION;
#endif
#define PMIC_LDO5_CTRL_ldo5_vrset_START (0)
#define PMIC_LDO5_CTRL_ldo5_vrset_END (2)
#define PMIC_LDO5_CTRL_ldo5_ocp_enn_START (3)
#define PMIC_LDO5_CTRL_ldo5_ocp_enn_END (3)
#define PMIC_LDO5_CTRL_ldo5_iss_en_START (4)
#define PMIC_LDO5_CTRL_ldo5_iss_en_END (4)
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
        unsigned char ldo7_vrset : 3;
        unsigned char ldo7_ocp_enn : 1;
        unsigned char ldo7_ocp_set : 2;
        unsigned char ldo7_eco_set : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO7_CTRL0_UNION;
#endif
#define PMIC_LDO7_CTRL0_ldo7_vrset_START (0)
#define PMIC_LDO7_CTRL0_ldo7_vrset_END (2)
#define PMIC_LDO7_CTRL0_ldo7_ocp_enn_START (3)
#define PMIC_LDO7_CTRL0_ldo7_ocp_enn_END (3)
#define PMIC_LDO7_CTRL0_ldo7_ocp_set_START (4)
#define PMIC_LDO7_CTRL0_ldo7_ocp_set_END (5)
#define PMIC_LDO7_CTRL0_ldo7_eco_set_START (6)
#define PMIC_LDO7_CTRL0_ldo7_eco_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo7_psrr2_set : 3;
        unsigned char ldo7_psrr1_set : 3;
        unsigned char ldo7_psrr_en : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO7_CTRL1_UNION;
#endif
#define PMIC_LDO7_CTRL1_ldo7_psrr2_set_START (0)
#define PMIC_LDO7_CTRL1_ldo7_psrr2_set_END (2)
#define PMIC_LDO7_CTRL1_ldo7_psrr1_set_START (3)
#define PMIC_LDO7_CTRL1_ldo7_psrr1_set_END (5)
#define PMIC_LDO7_CTRL1_ldo7_psrr_en_START (6)
#define PMIC_LDO7_CTRL1_ldo7_psrr_en_END (6)
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
        unsigned char ldo15_ocp_set : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO15_CTRL_UNION;
#endif
#define PMIC_LDO15_CTRL_ldo15_vrset_START (0)
#define PMIC_LDO15_CTRL_ldo15_vrset_END (2)
#define PMIC_LDO15_CTRL_ldo15_ocp_enn_START (3)
#define PMIC_LDO15_CTRL_ldo15_ocp_enn_END (3)
#define PMIC_LDO15_CTRL_ldo15_ocp_set_START (4)
#define PMIC_LDO15_CTRL_ldo15_ocp_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo16_vrset : 3;
        unsigned char ldo16_ocp_enn : 1;
        unsigned char ldo16_ocp_set : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO16_CTRL_UNION;
#endif
#define PMIC_LDO16_CTRL_ldo16_vrset_START (0)
#define PMIC_LDO16_CTRL_ldo16_vrset_END (2)
#define PMIC_LDO16_CTRL_ldo16_ocp_enn_START (3)
#define PMIC_LDO16_CTRL_ldo16_ocp_enn_END (3)
#define PMIC_LDO16_CTRL_ldo16_ocp_set_START (4)
#define PMIC_LDO16_CTRL_ldo16_ocp_set_END (4)
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
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO18_CTRL0_UNION;
#endif
#define PMIC_LDO18_CTRL0_ldo18_vrset_START (0)
#define PMIC_LDO18_CTRL0_ldo18_vrset_END (2)
#define PMIC_LDO18_CTRL0_ldo18_ocp_enn_START (3)
#define PMIC_LDO18_CTRL0_ldo18_ocp_enn_END (3)
#define PMIC_LDO18_CTRL0_ldo18_eco_set_START (4)
#define PMIC_LDO18_CTRL0_ldo18_eco_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo18_psrr2_set : 3;
        unsigned char ldo18_psrr1_set : 3;
        unsigned char ldo18_psrr_en : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO18_CTRL1_UNION;
#endif
#define PMIC_LDO18_CTRL1_ldo18_psrr2_set_START (0)
#define PMIC_LDO18_CTRL1_ldo18_psrr2_set_END (2)
#define PMIC_LDO18_CTRL1_ldo18_psrr1_set_START (3)
#define PMIC_LDO18_CTRL1_ldo18_psrr1_set_END (5)
#define PMIC_LDO18_CTRL1_ldo18_psrr_en_START (6)
#define PMIC_LDO18_CTRL1_ldo18_psrr_en_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo21_vrset : 3;
        unsigned char ldo21_ocp_enn : 1;
        unsigned char ldo21_iss_en : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO21_CTRL_UNION;
#endif
#define PMIC_LDO21_CTRL_ldo21_vrset_START (0)
#define PMIC_LDO21_CTRL_ldo21_vrset_END (2)
#define PMIC_LDO21_CTRL_ldo21_ocp_enn_START (3)
#define PMIC_LDO21_CTRL_ldo21_ocp_enn_END (3)
#define PMIC_LDO21_CTRL_ldo21_iss_en_START (4)
#define PMIC_LDO21_CTRL_ldo21_iss_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo22_vrset : 3;
        unsigned char ldo22_ocp_enn : 1;
        unsigned char ldo22_iss_en : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO22_CTRL0_UNION;
#endif
#define PMIC_LDO22_CTRL0_ldo22_vrset_START (0)
#define PMIC_LDO22_CTRL0_ldo22_vrset_END (2)
#define PMIC_LDO22_CTRL0_ldo22_ocp_enn_START (3)
#define PMIC_LDO22_CTRL0_ldo22_ocp_enn_END (3)
#define PMIC_LDO22_CTRL0_ldo22_iss_en_START (4)
#define PMIC_LDO22_CTRL0_ldo22_iss_en_END (4)
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
        unsigned char ldo22_ocps_en : 1;
        unsigned char ldo22_bw_en : 1;
        unsigned char ldo22_vgpr_en : 1;
        unsigned char ldo22_comp : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO22_CTRL2_UNION;
#endif
#define PMIC_LDO22_CTRL2_ldo22_ocps_en_START (0)
#define PMIC_LDO22_CTRL2_ldo22_ocps_en_END (0)
#define PMIC_LDO22_CTRL2_ldo22_bw_en_START (1)
#define PMIC_LDO22_CTRL2_ldo22_bw_en_END (1)
#define PMIC_LDO22_CTRL2_ldo22_vgpr_en_START (2)
#define PMIC_LDO22_CTRL2_ldo22_vgpr_en_END (2)
#define PMIC_LDO22_CTRL2_ldo22_comp_START (3)
#define PMIC_LDO22_CTRL2_ldo22_comp_END (5)
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
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO24_CTRL_UNION;
#endif
#define PMIC_LDO24_CTRL_ldo24_vrset_START (0)
#define PMIC_LDO24_CTRL_ldo24_vrset_END (2)
#define PMIC_LDO24_CTRL_ldo24_ocp_enn_START (3)
#define PMIC_LDO24_CTRL_ldo24_ocp_enn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo25_vrset : 3;
        unsigned char ldo25_ocp_enn : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO25_CTRL0_UNION;
#endif
#define PMIC_LDO25_CTRL0_ldo25_vrset_START (0)
#define PMIC_LDO25_CTRL0_ldo25_vrset_END (2)
#define PMIC_LDO25_CTRL0_ldo25_ocp_enn_START (3)
#define PMIC_LDO25_CTRL0_ldo25_ocp_enn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo25_psrr2_set : 3;
        unsigned char ldo25_psrr1_set : 3;
        unsigned char ldo25_psrr_en : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO25_CTRL1_UNION;
#endif
#define PMIC_LDO25_CTRL1_ldo25_psrr2_set_START (0)
#define PMIC_LDO25_CTRL1_ldo25_psrr2_set_END (2)
#define PMIC_LDO25_CTRL1_ldo25_psrr1_set_START (3)
#define PMIC_LDO25_CTRL1_ldo25_psrr1_set_END (5)
#define PMIC_LDO25_CTRL1_ldo25_psrr_en_START (6)
#define PMIC_LDO25_CTRL1_ldo25_psrr_en_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo25_ocps_en : 1;
        unsigned char ldo25_bw_en : 1;
        unsigned char ldo25_vgpr_en : 1;
        unsigned char ldo25_comp : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO25_CTRL2_UNION;
#endif
#define PMIC_LDO25_CTRL2_ldo25_ocps_en_START (0)
#define PMIC_LDO25_CTRL2_ldo25_ocps_en_END (0)
#define PMIC_LDO25_CTRL2_ldo25_bw_en_START (1)
#define PMIC_LDO25_CTRL2_ldo25_bw_en_END (1)
#define PMIC_LDO25_CTRL2_ldo25_vgpr_en_START (2)
#define PMIC_LDO25_CTRL2_ldo25_vgpr_en_END (2)
#define PMIC_LDO25_CTRL2_ldo25_comp_START (3)
#define PMIC_LDO25_CTRL2_ldo25_comp_END (5)
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
        unsigned char ldo29_ocp_set : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO29_CTRL_UNION;
#endif
#define PMIC_LDO29_CTRL_ldo29_vrset_START (0)
#define PMIC_LDO29_CTRL_ldo29_vrset_END (2)
#define PMIC_LDO29_CTRL_ldo29_ocp_enn_START (3)
#define PMIC_LDO29_CTRL_ldo29_ocp_enn_END (3)
#define PMIC_LDO29_CTRL_ldo29_eco_set_START (4)
#define PMIC_LDO29_CTRL_ldo29_eco_set_END (4)
#define PMIC_LDO29_CTRL_ldo29_ocp_set_START (5)
#define PMIC_LDO29_CTRL_ldo29_ocp_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo30_vrset : 3;
        unsigned char ldo30_ocp_enn : 1;
        unsigned char ldo30_ocps_en : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO30_CTRL0_UNION;
#endif
#define PMIC_LDO30_CTRL0_ldo30_vrset_START (0)
#define PMIC_LDO30_CTRL0_ldo30_vrset_END (2)
#define PMIC_LDO30_CTRL0_ldo30_ocp_enn_START (3)
#define PMIC_LDO30_CTRL0_ldo30_ocp_enn_END (3)
#define PMIC_LDO30_CTRL0_ldo30_ocps_en_START (4)
#define PMIC_LDO30_CTRL0_ldo30_ocps_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo30_ocpd_set : 2;
        unsigned char ldo30_ocps_set : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO30_CTRL1_UNION;
#endif
#define PMIC_LDO30_CTRL1_ldo30_ocpd_set_START (0)
#define PMIC_LDO30_CTRL1_ldo30_ocpd_set_END (1)
#define PMIC_LDO30_CTRL1_ldo30_ocps_set_START (2)
#define PMIC_LDO30_CTRL1_ldo30_ocps_set_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo32_vrset : 3;
        unsigned char ldo32_ocp_enn : 1;
        unsigned char ldo32_iss_en : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO32_CTRL0_UNION;
#endif
#define PMIC_LDO32_CTRL0_ldo32_vrset_START (0)
#define PMIC_LDO32_CTRL0_ldo32_vrset_END (2)
#define PMIC_LDO32_CTRL0_ldo32_ocp_enn_START (3)
#define PMIC_LDO32_CTRL0_ldo32_ocp_enn_END (3)
#define PMIC_LDO32_CTRL0_ldo32_iss_en_START (4)
#define PMIC_LDO32_CTRL0_ldo32_iss_en_END (4)
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
        unsigned char ldo32_ocpd_set : 2;
        unsigned char ldo32_ocps_set : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO32_CTRL3_UNION;
#endif
#define PMIC_LDO32_CTRL3_ldo32_ocpd_set_START (0)
#define PMIC_LDO32_CTRL3_ldo32_ocpd_set_END (1)
#define PMIC_LDO32_CTRL3_ldo32_ocps_set_START (2)
#define PMIC_LDO32_CTRL3_ldo32_ocps_set_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo34_vrset : 3;
        unsigned char ldo34_ocp_enn : 1;
        unsigned char ldo34_ocp_set : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO34_CTRL_UNION;
#endif
#define PMIC_LDO34_CTRL_ldo34_vrset_START (0)
#define PMIC_LDO34_CTRL_ldo34_vrset_END (2)
#define PMIC_LDO34_CTRL_ldo34_ocp_enn_START (3)
#define PMIC_LDO34_CTRL_ldo34_ocp_enn_END (3)
#define PMIC_LDO34_CTRL_ldo34_ocp_set_START (4)
#define PMIC_LDO34_CTRL_ldo34_ocp_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pmuh_vrset : 3;
        unsigned char pmuh_ocp_enn : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_PMUH_CTRL_UNION;
#endif
#define PMIC_PMUH_CTRL_pmuh_vrset_START (0)
#define PMIC_PMUH_CTRL_pmuh_vrset_END (2)
#define PMIC_PMUH_CTRL_pmuh_ocp_enn_START (3)
#define PMIC_PMUH_CTRL_pmuh_ocp_enn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo36_vrset : 3;
        unsigned char ldo36_ocp_enn : 1;
        unsigned char ldo36_eco_set : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO36_CTRL0_UNION;
#endif
#define PMIC_LDO36_CTRL0_ldo36_vrset_START (0)
#define PMIC_LDO36_CTRL0_ldo36_vrset_END (2)
#define PMIC_LDO36_CTRL0_ldo36_ocp_enn_START (3)
#define PMIC_LDO36_CTRL0_ldo36_ocp_enn_END (3)
#define PMIC_LDO36_CTRL0_ldo36_eco_set_START (4)
#define PMIC_LDO36_CTRL0_ldo36_eco_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo36_ref_buf_bypass : 1;
        unsigned char ldo36_psrr2_set : 3;
        unsigned char ldo36_psrr1_set : 3;
        unsigned char ldo36_psrr_en : 1;
    } reg;
} PMIC_LDO36_CTRL1_UNION;
#endif
#define PMIC_LDO36_CTRL1_ldo36_ref_buf_bypass_START (0)
#define PMIC_LDO36_CTRL1_ldo36_ref_buf_bypass_END (0)
#define PMIC_LDO36_CTRL1_ldo36_psrr2_set_START (1)
#define PMIC_LDO36_CTRL1_ldo36_psrr2_set_END (3)
#define PMIC_LDO36_CTRL1_ldo36_psrr1_set_START (4)
#define PMIC_LDO36_CTRL1_ldo36_psrr1_set_END (6)
#define PMIC_LDO36_CTRL1_ldo36_psrr_en_START (7)
#define PMIC_LDO36_CTRL1_ldo36_psrr_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo36_ocps_en : 1;
        unsigned char ldo36_bw_en : 1;
        unsigned char ldo36_vgpr_en : 1;
        unsigned char ldo36_ref_buf_rset : 2;
        unsigned char ldo36_comp : 3;
    } reg;
} PMIC_LDO36_CTRL2_UNION;
#endif
#define PMIC_LDO36_CTRL2_ldo36_ocps_en_START (0)
#define PMIC_LDO36_CTRL2_ldo36_ocps_en_END (0)
#define PMIC_LDO36_CTRL2_ldo36_bw_en_START (1)
#define PMIC_LDO36_CTRL2_ldo36_bw_en_END (1)
#define PMIC_LDO36_CTRL2_ldo36_vgpr_en_START (2)
#define PMIC_LDO36_CTRL2_ldo36_vgpr_en_END (2)
#define PMIC_LDO36_CTRL2_ldo36_ref_buf_rset_START (3)
#define PMIC_LDO36_CTRL2_ldo36_ref_buf_rset_END (4)
#define PMIC_LDO36_CTRL2_ldo36_comp_START (5)
#define PMIC_LDO36_CTRL2_ldo36_comp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo37_vrset : 3;
        unsigned char ldo37_ocp_enn : 1;
        unsigned char ldo37_ocps_en : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO37_CTRL0_UNION;
#endif
#define PMIC_LDO37_CTRL0_ldo37_vrset_START (0)
#define PMIC_LDO37_CTRL0_ldo37_vrset_END (2)
#define PMIC_LDO37_CTRL0_ldo37_ocp_enn_START (3)
#define PMIC_LDO37_CTRL0_ldo37_ocp_enn_END (3)
#define PMIC_LDO37_CTRL0_ldo37_ocps_en_START (4)
#define PMIC_LDO37_CTRL0_ldo37_ocps_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo37_ocpd_set : 2;
        unsigned char ldo37_ocps_set : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO37_CTRL1_UNION;
#endif
#define PMIC_LDO37_CTRL1_ldo37_ocpd_set_START (0)
#define PMIC_LDO37_CTRL1_ldo37_ocpd_set_END (1)
#define PMIC_LDO37_CTRL1_ldo37_ocps_set_START (2)
#define PMIC_LDO37_CTRL1_ldo37_ocps_set_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo38_vrset : 3;
        unsigned char ldo38_ocp_enn : 1;
        unsigned char ldo38_iss_en : 1;
        unsigned char ldo38_eco_set : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO38_CTRL0_UNION;
#endif
#define PMIC_LDO38_CTRL0_ldo38_vrset_START (0)
#define PMIC_LDO38_CTRL0_ldo38_vrset_END (2)
#define PMIC_LDO38_CTRL0_ldo38_ocp_enn_START (3)
#define PMIC_LDO38_CTRL0_ldo38_ocp_enn_END (3)
#define PMIC_LDO38_CTRL0_ldo38_iss_en_START (4)
#define PMIC_LDO38_CTRL0_ldo38_iss_en_END (4)
#define PMIC_LDO38_CTRL0_ldo38_eco_set_START (5)
#define PMIC_LDO38_CTRL0_ldo38_eco_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo38_psrr2_set : 3;
        unsigned char ldo38_psrr1_set : 3;
        unsigned char ldo38_psrr_en : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO38_CTRL1_UNION;
#endif
#define PMIC_LDO38_CTRL1_ldo38_psrr2_set_START (0)
#define PMIC_LDO38_CTRL1_ldo38_psrr2_set_END (2)
#define PMIC_LDO38_CTRL1_ldo38_psrr1_set_START (3)
#define PMIC_LDO38_CTRL1_ldo38_psrr1_set_END (5)
#define PMIC_LDO38_CTRL1_ldo38_psrr_en_START (6)
#define PMIC_LDO38_CTRL1_ldo38_psrr_en_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo38_ocps_en : 1;
        unsigned char ldo38_bw_en : 1;
        unsigned char ldo38_vgpr_en : 1;
        unsigned char ldo38_comp : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO38_CTRL2_UNION;
#endif
#define PMIC_LDO38_CTRL2_ldo38_ocps_en_START (0)
#define PMIC_LDO38_CTRL2_ldo38_ocps_en_END (0)
#define PMIC_LDO38_CTRL2_ldo38_bw_en_START (1)
#define PMIC_LDO38_CTRL2_ldo38_bw_en_END (1)
#define PMIC_LDO38_CTRL2_ldo38_vgpr_en_START (2)
#define PMIC_LDO38_CTRL2_ldo38_vgpr_en_END (2)
#define PMIC_LDO38_CTRL2_ldo38_comp_START (3)
#define PMIC_LDO38_CTRL2_ldo38_comp_END (5)
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
} PMIC_LDO38_CTRL3_UNION;
#endif
#define PMIC_LDO38_CTRL3_ldo38_ocpd_set_START (0)
#define PMIC_LDO38_CTRL3_ldo38_ocpd_set_END (1)
#define PMIC_LDO38_CTRL3_ldo38_ocps_set_START (2)
#define PMIC_LDO38_CTRL3_ldo38_ocps_set_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo39_vrset : 3;
        unsigned char ldo39_ocp_enn : 1;
        unsigned char ldo39_eco_set : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO39_CTRL0_UNION;
#endif
#define PMIC_LDO39_CTRL0_ldo39_vrset_START (0)
#define PMIC_LDO39_CTRL0_ldo39_vrset_END (2)
#define PMIC_LDO39_CTRL0_ldo39_ocp_enn_START (3)
#define PMIC_LDO39_CTRL0_ldo39_ocp_enn_END (3)
#define PMIC_LDO39_CTRL0_ldo39_eco_set_START (4)
#define PMIC_LDO39_CTRL0_ldo39_eco_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo39_ref_buf_bypass : 1;
        unsigned char ldo39_psrr2_set : 3;
        unsigned char ldo39_psrr1_set : 3;
        unsigned char ldo39_psrr_en : 1;
    } reg;
} PMIC_LDO39_CTRL1_UNION;
#endif
#define PMIC_LDO39_CTRL1_ldo39_ref_buf_bypass_START (0)
#define PMIC_LDO39_CTRL1_ldo39_ref_buf_bypass_END (0)
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
        unsigned char ldo39_ref_buf_rset : 2;
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
#define PMIC_LDO39_CTRL2_ldo39_ref_buf_rset_START (3)
#define PMIC_LDO39_CTRL2_ldo39_ref_buf_rset_END (4)
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
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO39_CTRL3_UNION;
#endif
#define PMIC_LDO39_CTRL3_ldo39_ocpd_set_START (0)
#define PMIC_LDO39_CTRL3_ldo39_ocpd_set_END (1)
#define PMIC_LDO39_CTRL3_ldo39_ocps_set_START (2)
#define PMIC_LDO39_CTRL3_ldo39_ocps_set_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo40_vrset : 3;
        unsigned char ldo40_ocp_enn : 1;
        unsigned char ldo40_eco_set : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO40_CTRL0_UNION;
#endif
#define PMIC_LDO40_CTRL0_ldo40_vrset_START (0)
#define PMIC_LDO40_CTRL0_ldo40_vrset_END (2)
#define PMIC_LDO40_CTRL0_ldo40_ocp_enn_START (3)
#define PMIC_LDO40_CTRL0_ldo40_ocp_enn_END (3)
#define PMIC_LDO40_CTRL0_ldo40_eco_set_START (4)
#define PMIC_LDO40_CTRL0_ldo40_eco_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo40_ref_buf_bypass : 1;
        unsigned char ldo40_psrr2_set : 3;
        unsigned char ldo40_psrr1_set : 3;
        unsigned char ldo40_psrr_en : 1;
    } reg;
} PMIC_LDO40_CTRL1_UNION;
#endif
#define PMIC_LDO40_CTRL1_ldo40_ref_buf_bypass_START (0)
#define PMIC_LDO40_CTRL1_ldo40_ref_buf_bypass_END (0)
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
        unsigned char ldo40_ref_buf_rset : 2;
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
#define PMIC_LDO40_CTRL2_ldo40_ref_buf_rset_START (3)
#define PMIC_LDO40_CTRL2_ldo40_ref_buf_rset_END (4)
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
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO40_CTRL3_UNION;
#endif
#define PMIC_LDO40_CTRL3_ldo40_ocpd_set_START (0)
#define PMIC_LDO40_CTRL3_ldo40_ocpd_set_END (1)
#define PMIC_LDO40_CTRL3_ldo40_ocps_set_START (2)
#define PMIC_LDO40_CTRL3_ldo40_ocps_set_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo41_vrset : 3;
        unsigned char ldo41_ocp_enn : 1;
        unsigned char ldo41_eco_set : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO41_CTRL0_UNION;
#endif
#define PMIC_LDO41_CTRL0_ldo41_vrset_START (0)
#define PMIC_LDO41_CTRL0_ldo41_vrset_END (2)
#define PMIC_LDO41_CTRL0_ldo41_ocp_enn_START (3)
#define PMIC_LDO41_CTRL0_ldo41_ocp_enn_END (3)
#define PMIC_LDO41_CTRL0_ldo41_eco_set_START (4)
#define PMIC_LDO41_CTRL0_ldo41_eco_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo41_ref_buf_bypass : 1;
        unsigned char ldo41_psrr2_set : 3;
        unsigned char ldo41_psrr1_set : 3;
        unsigned char ldo41_psrr_en : 1;
    } reg;
} PMIC_LDO41_CTRL1_UNION;
#endif
#define PMIC_LDO41_CTRL1_ldo41_ref_buf_bypass_START (0)
#define PMIC_LDO41_CTRL1_ldo41_ref_buf_bypass_END (0)
#define PMIC_LDO41_CTRL1_ldo41_psrr2_set_START (1)
#define PMIC_LDO41_CTRL1_ldo41_psrr2_set_END (3)
#define PMIC_LDO41_CTRL1_ldo41_psrr1_set_START (4)
#define PMIC_LDO41_CTRL1_ldo41_psrr1_set_END (6)
#define PMIC_LDO41_CTRL1_ldo41_psrr_en_START (7)
#define PMIC_LDO41_CTRL1_ldo41_psrr_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo41_ocps_en : 1;
        unsigned char ldo41_bw_en : 1;
        unsigned char ldo41_vgpr_en : 1;
        unsigned char ldo41_ref_buf_rset : 2;
        unsigned char ldo41_comp : 3;
    } reg;
} PMIC_LDO41_CTRL2_UNION;
#endif
#define PMIC_LDO41_CTRL2_ldo41_ocps_en_START (0)
#define PMIC_LDO41_CTRL2_ldo41_ocps_en_END (0)
#define PMIC_LDO41_CTRL2_ldo41_bw_en_START (1)
#define PMIC_LDO41_CTRL2_ldo41_bw_en_END (1)
#define PMIC_LDO41_CTRL2_ldo41_vgpr_en_START (2)
#define PMIC_LDO41_CTRL2_ldo41_vgpr_en_END (2)
#define PMIC_LDO41_CTRL2_ldo41_ref_buf_rset_START (3)
#define PMIC_LDO41_CTRL2_ldo41_ref_buf_rset_END (4)
#define PMIC_LDO41_CTRL2_ldo41_comp_START (5)
#define PMIC_LDO41_CTRL2_ldo41_comp_END (7)
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
        unsigned char ldo_reserve2 : 8;
    } reg;
} PMIC_LDO_RESERVE2_UNION;
#endif
#define PMIC_LDO_RESERVE2_ldo_reserve2_START (0)
#define PMIC_LDO_RESERVE2_ldo_reserve2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo_reserve3 : 8;
    } reg;
} PMIC_LDO_RESERVE3_UNION;
#endif
#define PMIC_LDO_RESERVE3_ldo_reserve3_START (0)
#define PMIC_LDO_RESERVE3_ldo_reserve3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_ocp_auto_stop : 2;
        unsigned char buck1_ocp_auto_stop : 2;
        unsigned char buck02_ocp_auto_stop : 2;
        unsigned char buck00_ocp_auto_stop : 2;
    } reg;
} PMIC_BUCK0_3_OCP_CTRL_UNION;
#endif
#define PMIC_BUCK0_3_OCP_CTRL_buck2_ocp_auto_stop_START (0)
#define PMIC_BUCK0_3_OCP_CTRL_buck2_ocp_auto_stop_END (1)
#define PMIC_BUCK0_3_OCP_CTRL_buck1_ocp_auto_stop_START (2)
#define PMIC_BUCK0_3_OCP_CTRL_buck1_ocp_auto_stop_END (3)
#define PMIC_BUCK0_3_OCP_CTRL_buck02_ocp_auto_stop_START (4)
#define PMIC_BUCK0_3_OCP_CTRL_buck02_ocp_auto_stop_END (5)
#define PMIC_BUCK0_3_OCP_CTRL_buck00_ocp_auto_stop_START (6)
#define PMIC_BUCK0_3_OCP_CTRL_buck00_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck6_ocp_auto_stop : 2;
        unsigned char buck5_ocp_auto_stop : 2;
        unsigned char buck4_ocp_auto_stop : 2;
        unsigned char buck3_ocp_auto_stop : 2;
    } reg;
} PMIC_BUCK4_7_OCP_CTRL_UNION;
#endif
#define PMIC_BUCK4_7_OCP_CTRL_buck6_ocp_auto_stop_START (0)
#define PMIC_BUCK4_7_OCP_CTRL_buck6_ocp_auto_stop_END (1)
#define PMIC_BUCK4_7_OCP_CTRL_buck5_ocp_auto_stop_START (2)
#define PMIC_BUCK4_7_OCP_CTRL_buck5_ocp_auto_stop_END (3)
#define PMIC_BUCK4_7_OCP_CTRL_buck4_ocp_auto_stop_START (4)
#define PMIC_BUCK4_7_OCP_CTRL_buck4_ocp_auto_stop_END (5)
#define PMIC_BUCK4_7_OCP_CTRL_buck3_ocp_auto_stop_START (6)
#define PMIC_BUCK4_7_OCP_CTRL_buck3_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck9_ocp_auto_stop : 2;
        unsigned char buck8_ocp_auto_stop : 2;
        unsigned char buck7_ocp_auto_stop : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK8_9_OCP_CTRL_UNION;
#endif
#define PMIC_BUCK8_9_OCP_CTRL_buck9_ocp_auto_stop_START (0)
#define PMIC_BUCK8_9_OCP_CTRL_buck9_ocp_auto_stop_END (1)
#define PMIC_BUCK8_9_OCP_CTRL_buck8_ocp_auto_stop_START (2)
#define PMIC_BUCK8_9_OCP_CTRL_buck8_ocp_auto_stop_END (3)
#define PMIC_BUCK8_9_OCP_CTRL_buck7_ocp_auto_stop_START (4)
#define PMIC_BUCK8_9_OCP_CTRL_buck7_ocp_auto_stop_END (5)
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
        unsigned char ldo7_ocp_auto_stop : 2;
        unsigned char ldo6_ocp_auto_stop : 2;
        unsigned char ldo5_ocp_auto_stop : 2;
        unsigned char ldo4_ocp_auto_stop : 2;
    } reg;
} PMIC_LDO4_7_OCP_CTRL_UNION;
#endif
#define PMIC_LDO4_7_OCP_CTRL_ldo7_ocp_auto_stop_START (0)
#define PMIC_LDO4_7_OCP_CTRL_ldo7_ocp_auto_stop_END (1)
#define PMIC_LDO4_7_OCP_CTRL_ldo6_ocp_auto_stop_START (2)
#define PMIC_LDO4_7_OCP_CTRL_ldo6_ocp_auto_stop_END (3)
#define PMIC_LDO4_7_OCP_CTRL_ldo5_ocp_auto_stop_START (4)
#define PMIC_LDO4_7_OCP_CTRL_ldo5_ocp_auto_stop_END (5)
#define PMIC_LDO4_7_OCP_CTRL_ldo4_ocp_auto_stop_START (6)
#define PMIC_LDO4_7_OCP_CTRL_ldo4_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo12_ocp_auto_stop : 2;
        unsigned char ldo11_ocp_auto_stop : 2;
        unsigned char ldo9_ocp_auto_stop : 2;
        unsigned char ldo8_ocp_auto_stop : 2;
    } reg;
} PMIC_LDO8_12_OCP_CTRL_UNION;
#endif
#define PMIC_LDO8_12_OCP_CTRL_ldo12_ocp_auto_stop_START (0)
#define PMIC_LDO8_12_OCP_CTRL_ldo12_ocp_auto_stop_END (1)
#define PMIC_LDO8_12_OCP_CTRL_ldo11_ocp_auto_stop_START (2)
#define PMIC_LDO8_12_OCP_CTRL_ldo11_ocp_auto_stop_END (3)
#define PMIC_LDO8_12_OCP_CTRL_ldo9_ocp_auto_stop_START (4)
#define PMIC_LDO8_12_OCP_CTRL_ldo9_ocp_auto_stop_END (5)
#define PMIC_LDO8_12_OCP_CTRL_ldo8_ocp_auto_stop_START (6)
#define PMIC_LDO8_12_OCP_CTRL_ldo8_ocp_auto_stop_END (7)
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
        unsigned char ldo23_ocp_auto_stop : 2;
        unsigned char ldo22_ocp_auto_stop : 2;
        unsigned char ldo21_ocp_auto_stop : 2;
        unsigned char ldo18_ocp_auto_stop : 2;
    } reg;
} PMIC_LDO18_23_OCP_CTRL_UNION;
#endif
#define PMIC_LDO18_23_OCP_CTRL_ldo23_ocp_auto_stop_START (0)
#define PMIC_LDO18_23_OCP_CTRL_ldo23_ocp_auto_stop_END (1)
#define PMIC_LDO18_23_OCP_CTRL_ldo22_ocp_auto_stop_START (2)
#define PMIC_LDO18_23_OCP_CTRL_ldo22_ocp_auto_stop_END (3)
#define PMIC_LDO18_23_OCP_CTRL_ldo21_ocp_auto_stop_START (4)
#define PMIC_LDO18_23_OCP_CTRL_ldo21_ocp_auto_stop_END (5)
#define PMIC_LDO18_23_OCP_CTRL_ldo18_ocp_auto_stop_START (6)
#define PMIC_LDO18_23_OCP_CTRL_ldo18_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo27_ocp_auto_stop : 2;
        unsigned char ldo26_ocp_auto_stop : 2;
        unsigned char ldo25_ocp_auto_stop : 2;
        unsigned char ldo24_ocp_auto_stop : 2;
    } reg;
} PMIC_LDO24_27_OCP_CTRL_UNION;
#endif
#define PMIC_LDO24_27_OCP_CTRL_ldo27_ocp_auto_stop_START (0)
#define PMIC_LDO24_27_OCP_CTRL_ldo27_ocp_auto_stop_END (1)
#define PMIC_LDO24_27_OCP_CTRL_ldo26_ocp_auto_stop_START (2)
#define PMIC_LDO24_27_OCP_CTRL_ldo26_ocp_auto_stop_END (3)
#define PMIC_LDO24_27_OCP_CTRL_ldo25_ocp_auto_stop_START (4)
#define PMIC_LDO24_27_OCP_CTRL_ldo25_ocp_auto_stop_END (5)
#define PMIC_LDO24_27_OCP_CTRL_ldo24_ocp_auto_stop_START (6)
#define PMIC_LDO24_27_OCP_CTRL_ldo24_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo32_ocp_auto_stop : 2;
        unsigned char ldo30_ocp_auto_stop : 2;
        unsigned char ldo29_ocp_auto_stop : 2;
        unsigned char ldo28_ocp_auto_stop : 2;
    } reg;
} PMIC_LDO28_32_OCP_CTRL_UNION;
#endif
#define PMIC_LDO28_32_OCP_CTRL_ldo32_ocp_auto_stop_START (0)
#define PMIC_LDO28_32_OCP_CTRL_ldo32_ocp_auto_stop_END (1)
#define PMIC_LDO28_32_OCP_CTRL_ldo30_ocp_auto_stop_START (2)
#define PMIC_LDO28_32_OCP_CTRL_ldo30_ocp_auto_stop_END (3)
#define PMIC_LDO28_32_OCP_CTRL_ldo29_ocp_auto_stop_START (4)
#define PMIC_LDO28_32_OCP_CTRL_ldo29_ocp_auto_stop_END (5)
#define PMIC_LDO28_32_OCP_CTRL_ldo28_ocp_auto_stop_START (6)
#define PMIC_LDO28_32_OCP_CTRL_ldo28_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo38_ocp_auto_stop : 2;
        unsigned char ldo37_ocp_auto_stop : 2;
        unsigned char ldo36_ocp_auto_stop : 2;
        unsigned char ldo34_ocp_auto_stop : 2;
    } reg;
} PMIC_LDO34_38_OCP_CTRL_UNION;
#endif
#define PMIC_LDO34_38_OCP_CTRL_ldo38_ocp_auto_stop_START (0)
#define PMIC_LDO34_38_OCP_CTRL_ldo38_ocp_auto_stop_END (1)
#define PMIC_LDO34_38_OCP_CTRL_ldo37_ocp_auto_stop_START (2)
#define PMIC_LDO34_38_OCP_CTRL_ldo37_ocp_auto_stop_END (3)
#define PMIC_LDO34_38_OCP_CTRL_ldo36_ocp_auto_stop_START (4)
#define PMIC_LDO34_38_OCP_CTRL_ldo36_ocp_auto_stop_END (5)
#define PMIC_LDO34_38_OCP_CTRL_ldo34_ocp_auto_stop_START (6)
#define PMIC_LDO34_38_OCP_CTRL_ldo34_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo41_ocp_auto_stop : 2;
        unsigned char ldo40_ocp_auto_stop : 2;
        unsigned char ldo39_ocp_auto_stop : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO39_41_OCP_CTRL_UNION;
#endif
#define PMIC_LDO39_41_OCP_CTRL_ldo41_ocp_auto_stop_START (0)
#define PMIC_LDO39_41_OCP_CTRL_ldo41_ocp_auto_stop_END (1)
#define PMIC_LDO39_41_OCP_CTRL_ldo40_ocp_auto_stop_START (2)
#define PMIC_LDO39_41_OCP_CTRL_ldo40_ocp_auto_stop_END (3)
#define PMIC_LDO39_41_OCP_CTRL_ldo39_ocp_auto_stop_START (4)
#define PMIC_LDO39_41_OCP_CTRL_ldo39_ocp_auto_stop_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo_buff_scp_auto_stop : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_LDO_BUFF_SCP_CTRL_UNION;
#endif
#define PMIC_LDO_BUFF_SCP_CTRL_ldo_buff_scp_auto_stop_START (0)
#define PMIC_LDO_BUFF_SCP_CTRL_ldo_buff_scp_auto_stop_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_scp_auto_stop : 2;
        unsigned char buck1_scp_auto_stop : 2;
        unsigned char buck02_scp_auto_stop : 2;
        unsigned char buck00_scp_auto_stop : 2;
    } reg;
} PMIC_BUCK0_3_SCP_CTRL_UNION;
#endif
#define PMIC_BUCK0_3_SCP_CTRL_buck2_scp_auto_stop_START (0)
#define PMIC_BUCK0_3_SCP_CTRL_buck2_scp_auto_stop_END (1)
#define PMIC_BUCK0_3_SCP_CTRL_buck1_scp_auto_stop_START (2)
#define PMIC_BUCK0_3_SCP_CTRL_buck1_scp_auto_stop_END (3)
#define PMIC_BUCK0_3_SCP_CTRL_buck02_scp_auto_stop_START (4)
#define PMIC_BUCK0_3_SCP_CTRL_buck02_scp_auto_stop_END (5)
#define PMIC_BUCK0_3_SCP_CTRL_buck00_scp_auto_stop_START (6)
#define PMIC_BUCK0_3_SCP_CTRL_buck00_scp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck6_scp_auto_stop : 2;
        unsigned char buck5_scp_auto_stop : 2;
        unsigned char buck4_scp_auto_stop : 2;
        unsigned char buck3_scp_auto_stop : 2;
    } reg;
} PMIC_BUCK4_7_SCP_CTRL_UNION;
#endif
#define PMIC_BUCK4_7_SCP_CTRL_buck6_scp_auto_stop_START (0)
#define PMIC_BUCK4_7_SCP_CTRL_buck6_scp_auto_stop_END (1)
#define PMIC_BUCK4_7_SCP_CTRL_buck5_scp_auto_stop_START (2)
#define PMIC_BUCK4_7_SCP_CTRL_buck5_scp_auto_stop_END (3)
#define PMIC_BUCK4_7_SCP_CTRL_buck4_scp_auto_stop_START (4)
#define PMIC_BUCK4_7_SCP_CTRL_buck4_scp_auto_stop_END (5)
#define PMIC_BUCK4_7_SCP_CTRL_buck3_scp_auto_stop_START (6)
#define PMIC_BUCK4_7_SCP_CTRL_buck3_scp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck9_scp_auto_stop : 2;
        unsigned char buck8_scp_auto_stop : 2;
        unsigned char buck7_scp_auto_stop : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK8_9_SCP_CTRL_UNION;
#endif
#define PMIC_BUCK8_9_SCP_CTRL_buck9_scp_auto_stop_START (0)
#define PMIC_BUCK8_9_SCP_CTRL_buck9_scp_auto_stop_END (1)
#define PMIC_BUCK8_9_SCP_CTRL_buck8_scp_auto_stop_START (2)
#define PMIC_BUCK8_9_SCP_CTRL_buck8_scp_auto_stop_END (3)
#define PMIC_BUCK8_9_SCP_CTRL_buck7_scp_auto_stop_START (4)
#define PMIC_BUCK8_9_SCP_CTRL_buck7_scp_auto_stop_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_012_ocp_sel : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_BUCK0_OCP_CTRL0_UNION;
#endif
#define PMIC_BUCK0_OCP_CTRL0_buck0_012_ocp_sel_START (0)
#define PMIC_BUCK0_OCP_CTRL0_buck0_012_ocp_sel_END (0)
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
        unsigned char ldo_ocp_deb_sel : 2;
        unsigned char buck_scp_deb_sel : 2;
        unsigned char buck_ocp_deb_sel : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_OCP_DEB_CTRL0_UNION;
#endif
#define PMIC_OCP_DEB_CTRL0_ldo_ocp_deb_sel_START (0)
#define PMIC_OCP_DEB_CTRL0_ldo_ocp_deb_sel_END (1)
#define PMIC_OCP_DEB_CTRL0_buck_scp_deb_sel_START (2)
#define PMIC_OCP_DEB_CTRL0_buck_scp_deb_sel_END (3)
#define PMIC_OCP_DEB_CTRL0_buck_ocp_deb_sel_START (4)
#define PMIC_OCP_DEB_CTRL0_buck_ocp_deb_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vin_ldoh_deb_sel : 2;
        unsigned char ldo_buff_scp_deb_sel : 2;
        unsigned char acr_ocp_deb_sel : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_OCP_DEB_CTRL1_UNION;
#endif
#define PMIC_OCP_DEB_CTRL1_vin_ldoh_deb_sel_START (0)
#define PMIC_OCP_DEB_CTRL1_vin_ldoh_deb_sel_END (1)
#define PMIC_OCP_DEB_CTRL1_ldo_buff_scp_deb_sel_START (2)
#define PMIC_OCP_DEB_CTRL1_ldo_buff_scp_deb_sel_END (3)
#define PMIC_OCP_DEB_CTRL1_acr_ocp_deb_sel_START (4)
#define PMIC_OCP_DEB_CTRL1_acr_ocp_deb_sel_END (5)
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
        unsigned char en_acr_ocp_deb : 1;
        unsigned char reserved : 2;
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
#define PMIC_OCP_SCP_ONOFF_en_acr_ocp_deb_START (5)
#define PMIC_OCP_SCP_ONOFF_en_acr_ocp_deb_END (5)
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
        unsigned char xo_rf0_drv : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_CLK_RF0_CTRL0_UNION;
#endif
#define PMIC_CLK_RF0_CTRL0_xo_rf0_drv_START (0)
#define PMIC_CLK_RF0_CTRL0_xo_rf0_drv_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_rf1_drv : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_CLK_RF1_CTRL0_UNION;
#endif
#define PMIC_CLK_RF1_CTRL0_xo_rf1_drv_START (0)
#define PMIC_CLK_RF1_CTRL0_xo_rf1_drv_END (1)
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
        unsigned char xo_wifi_1181_drv : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_CLK_WIFI_1181_CTRL_UNION;
#endif
#define PMIC_CLK_WIFI_1181_CTRL_xo_wifi_1181_drv_START (0)
#define PMIC_CLK_WIFI_1181_CTRL_xo_wifi_1181_drv_END (1)
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
        unsigned char xo_rf1_ph_sel : 1;
        unsigned char xo_tri_cap : 1;
        unsigned char xo_delay_sel : 2;
        unsigned char xo_core_curr_sel : 2;
        unsigned char xo_eco_force_en : 1;
        unsigned char xo_eco_en_shield : 1;
    } reg;
} PMIC_CLK_TOP_CTRL0_UNION;
#endif
#define PMIC_CLK_TOP_CTRL0_xo_rf1_ph_sel_START (0)
#define PMIC_CLK_TOP_CTRL0_xo_rf1_ph_sel_END (0)
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
        unsigned char reserved : 8;
    } reg;
} PMIC_CLK_TOP_CTRL2_UNION;
#endif
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
        unsigned char ref_chop_clk_sel : 2;
        unsigned char ref_chop_sel : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_BG_THSD_CTRL1_UNION;
#endif
#define PMIC_BG_THSD_CTRL1_ref_res_sel_START (0)
#define PMIC_BG_THSD_CTRL1_ref_res_sel_END (0)
#define PMIC_BG_THSD_CTRL1_ref_chop_clk_sel_START (1)
#define PMIC_BG_THSD_CTRL1_ref_chop_clk_sel_END (2)
#define PMIC_BG_THSD_CTRL1_ref_chop_sel_START (3)
#define PMIC_BG_THSD_CTRL1_ref_chop_sel_END (3)
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
        unsigned char reg_ldo26_hd_mask : 1;
        unsigned char reg_ldo_buf_hd_mask : 1;
        unsigned char reg_eco_in_hd_mask : 1;
        unsigned char reg_sys_clk_hd_mask : 1;
        unsigned char reg_abb_clk_hd_mask : 1;
        unsigned char reg_wifi_clk_hd_mask : 1;
        unsigned char reg_nfc_clk_hd_mask : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_HARDWIRE_CTRL0_UNION;
#endif
#define PMIC_HARDWIRE_CTRL0_reg_ldo26_hd_mask_START (0)
#define PMIC_HARDWIRE_CTRL0_reg_ldo26_hd_mask_END (0)
#define PMIC_HARDWIRE_CTRL0_reg_ldo_buf_hd_mask_START (1)
#define PMIC_HARDWIRE_CTRL0_reg_ldo_buf_hd_mask_END (1)
#define PMIC_HARDWIRE_CTRL0_reg_eco_in_hd_mask_START (2)
#define PMIC_HARDWIRE_CTRL0_reg_eco_in_hd_mask_END (2)
#define PMIC_HARDWIRE_CTRL0_reg_sys_clk_hd_mask_START (3)
#define PMIC_HARDWIRE_CTRL0_reg_sys_clk_hd_mask_END (3)
#define PMIC_HARDWIRE_CTRL0_reg_abb_clk_hd_mask_START (4)
#define PMIC_HARDWIRE_CTRL0_reg_abb_clk_hd_mask_END (4)
#define PMIC_HARDWIRE_CTRL0_reg_wifi_clk_hd_mask_START (5)
#define PMIC_HARDWIRE_CTRL0_reg_wifi_clk_hd_mask_END (5)
#define PMIC_HARDWIRE_CTRL0_reg_nfc_clk_hd_mask_START (6)
#define PMIC_HARDWIRE_CTRL0_reg_nfc_clk_hd_mask_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_idle_hd_mask : 1;
        unsigned char reg_buck02_vset_hd_mask : 1;
        unsigned char reg_buck00_vset_hd_mask : 1;
        unsigned char reg_ldo39_vset_hd_mask : 1;
        unsigned char reg_ldo0_vset_hd_mask : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_HARDWIRE_CTRL1_UNION;
#endif
#define PMIC_HARDWIRE_CTRL1_reg_idle_hd_mask_START (0)
#define PMIC_HARDWIRE_CTRL1_reg_idle_hd_mask_END (0)
#define PMIC_HARDWIRE_CTRL1_reg_buck02_vset_hd_mask_START (1)
#define PMIC_HARDWIRE_CTRL1_reg_buck02_vset_hd_mask_END (1)
#define PMIC_HARDWIRE_CTRL1_reg_buck00_vset_hd_mask_START (2)
#define PMIC_HARDWIRE_CTRL1_reg_buck00_vset_hd_mask_END (2)
#define PMIC_HARDWIRE_CTRL1_reg_ldo39_vset_hd_mask_START (3)
#define PMIC_HARDWIRE_CTRL1_reg_ldo39_vset_hd_mask_END (3)
#define PMIC_HARDWIRE_CTRL1_reg_ldo0_vset_hd_mask_START (4)
#define PMIC_HARDWIRE_CTRL1_reg_ldo0_vset_hd_mask_END (4)
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
        unsigned char peri_en_buck9_on : 1;
        unsigned char peri_en_buck7_on : 1;
        unsigned char peri_en_buck6_on : 1;
        unsigned char peri_en_buck4_on : 1;
        unsigned char peri_en_buck2_on : 1;
        unsigned char peri_en_buck02_on : 1;
        unsigned char peri_en_buck00_on : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_PERI_CTRL0_UNION;
#endif
#define PMIC_PERI_CTRL0_peri_en_buck9_on_START (0)
#define PMIC_PERI_CTRL0_peri_en_buck9_on_END (0)
#define PMIC_PERI_CTRL0_peri_en_buck7_on_START (1)
#define PMIC_PERI_CTRL0_peri_en_buck7_on_END (1)
#define PMIC_PERI_CTRL0_peri_en_buck6_on_START (2)
#define PMIC_PERI_CTRL0_peri_en_buck6_on_END (2)
#define PMIC_PERI_CTRL0_peri_en_buck4_on_START (3)
#define PMIC_PERI_CTRL0_peri_en_buck4_on_END (3)
#define PMIC_PERI_CTRL0_peri_en_buck2_on_START (4)
#define PMIC_PERI_CTRL0_peri_en_buck2_on_END (4)
#define PMIC_PERI_CTRL0_peri_en_buck02_on_START (5)
#define PMIC_PERI_CTRL0_peri_en_buck02_on_END (5)
#define PMIC_PERI_CTRL0_peri_en_buck00_on_START (6)
#define PMIC_PERI_CTRL0_peri_en_buck00_on_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_ldo30_on : 1;
        unsigned char peri_en_ldo29_on : 1;
        unsigned char peri_en_ldo28_on : 1;
        unsigned char peri_en_ldo23_on : 1;
        unsigned char peri_en_ldo7_on : 1;
        unsigned char peri_en_ldo6_on : 1;
        unsigned char peri_en_ldo5_on : 1;
        unsigned char peri_en_ldo0_on : 1;
    } reg;
} PMIC_PERI_CTRL1_UNION;
#endif
#define PMIC_PERI_CTRL1_peri_en_ldo30_on_START (0)
#define PMIC_PERI_CTRL1_peri_en_ldo30_on_END (0)
#define PMIC_PERI_CTRL1_peri_en_ldo29_on_START (1)
#define PMIC_PERI_CTRL1_peri_en_ldo29_on_END (1)
#define PMIC_PERI_CTRL1_peri_en_ldo28_on_START (2)
#define PMIC_PERI_CTRL1_peri_en_ldo28_on_END (2)
#define PMIC_PERI_CTRL1_peri_en_ldo23_on_START (3)
#define PMIC_PERI_CTRL1_peri_en_ldo23_on_END (3)
#define PMIC_PERI_CTRL1_peri_en_ldo7_on_START (4)
#define PMIC_PERI_CTRL1_peri_en_ldo7_on_END (4)
#define PMIC_PERI_CTRL1_peri_en_ldo6_on_START (5)
#define PMIC_PERI_CTRL1_peri_en_ldo6_on_END (5)
#define PMIC_PERI_CTRL1_peri_en_ldo5_on_START (6)
#define PMIC_PERI_CTRL1_peri_en_ldo5_on_END (6)
#define PMIC_PERI_CTRL1_peri_en_ldo0_on_START (7)
#define PMIC_PERI_CTRL1_peri_en_ldo0_on_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_ldo41_on : 1;
        unsigned char peri_en_ldo40_on : 1;
        unsigned char peri_en_ldo39_on : 1;
        unsigned char peri_en_ldo37_on : 1;
        unsigned char peri_en_ldo36_on : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL2_UNION;
#endif
#define PMIC_PERI_CTRL2_peri_en_ldo41_on_START (0)
#define PMIC_PERI_CTRL2_peri_en_ldo41_on_END (0)
#define PMIC_PERI_CTRL2_peri_en_ldo40_on_START (1)
#define PMIC_PERI_CTRL2_peri_en_ldo40_on_END (1)
#define PMIC_PERI_CTRL2_peri_en_ldo39_on_START (2)
#define PMIC_PERI_CTRL2_peri_en_ldo39_on_END (2)
#define PMIC_PERI_CTRL2_peri_en_ldo37_on_START (3)
#define PMIC_PERI_CTRL2_peri_en_ldo37_on_END (3)
#define PMIC_PERI_CTRL2_peri_en_ldo36_on_START (4)
#define PMIC_PERI_CTRL2_peri_en_ldo36_on_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_buck7_eco : 1;
        unsigned char peri_en_buck6_eco : 1;
        unsigned char peri_en_buck4_eco : 1;
        unsigned char peri_en_buck3_eco : 1;
        unsigned char peri_en_buck2_eco : 1;
        unsigned char peri_en_buck1_eco : 1;
        unsigned char peri_en_buck02_eco : 1;
        unsigned char peri_en_buck00_eco : 1;
    } reg;
} PMIC_PERI_CTRL3_UNION;
#endif
#define PMIC_PERI_CTRL3_peri_en_buck7_eco_START (0)
#define PMIC_PERI_CTRL3_peri_en_buck7_eco_END (0)
#define PMIC_PERI_CTRL3_peri_en_buck6_eco_START (1)
#define PMIC_PERI_CTRL3_peri_en_buck6_eco_END (1)
#define PMIC_PERI_CTRL3_peri_en_buck4_eco_START (2)
#define PMIC_PERI_CTRL3_peri_en_buck4_eco_END (2)
#define PMIC_PERI_CTRL3_peri_en_buck3_eco_START (3)
#define PMIC_PERI_CTRL3_peri_en_buck3_eco_END (3)
#define PMIC_PERI_CTRL3_peri_en_buck2_eco_START (4)
#define PMIC_PERI_CTRL3_peri_en_buck2_eco_END (4)
#define PMIC_PERI_CTRL3_peri_en_buck1_eco_START (5)
#define PMIC_PERI_CTRL3_peri_en_buck1_eco_END (5)
#define PMIC_PERI_CTRL3_peri_en_buck02_eco_START (6)
#define PMIC_PERI_CTRL3_peri_en_buck02_eco_END (6)
#define PMIC_PERI_CTRL3_peri_en_buck00_eco_START (7)
#define PMIC_PERI_CTRL3_peri_en_buck00_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_ldo11_eco : 1;
        unsigned char peri_en_ldo8_eco : 1;
        unsigned char peri_en_ldo7_eco : 1;
        unsigned char peri_en_ldo6_eco : 1;
        unsigned char peri_en_ldo5_eco : 1;
        unsigned char peri_en_ldo0_eco : 1;
        unsigned char peri_en_buck9_eco : 1;
        unsigned char peri_en_buck8_eco : 1;
    } reg;
} PMIC_PERI_CTRL4_UNION;
#endif
#define PMIC_PERI_CTRL4_peri_en_ldo11_eco_START (0)
#define PMIC_PERI_CTRL4_peri_en_ldo11_eco_END (0)
#define PMIC_PERI_CTRL4_peri_en_ldo8_eco_START (1)
#define PMIC_PERI_CTRL4_peri_en_ldo8_eco_END (1)
#define PMIC_PERI_CTRL4_peri_en_ldo7_eco_START (2)
#define PMIC_PERI_CTRL4_peri_en_ldo7_eco_END (2)
#define PMIC_PERI_CTRL4_peri_en_ldo6_eco_START (3)
#define PMIC_PERI_CTRL4_peri_en_ldo6_eco_END (3)
#define PMIC_PERI_CTRL4_peri_en_ldo5_eco_START (4)
#define PMIC_PERI_CTRL4_peri_en_ldo5_eco_END (4)
#define PMIC_PERI_CTRL4_peri_en_ldo0_eco_START (5)
#define PMIC_PERI_CTRL4_peri_en_ldo0_eco_END (5)
#define PMIC_PERI_CTRL4_peri_en_buck9_eco_START (6)
#define PMIC_PERI_CTRL4_peri_en_buck9_eco_END (6)
#define PMIC_PERI_CTRL4_peri_en_buck8_eco_START (7)
#define PMIC_PERI_CTRL4_peri_en_buck8_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_ldo30_eco : 1;
        unsigned char peri_en_ldo29_eco : 1;
        unsigned char peri_en_ldo27_eco : 1;
        unsigned char peri_en_ldo24_eco : 1;
        unsigned char peri_en_ldo23_eco : 1;
        unsigned char peri_en_ldo18_eco : 1;
        unsigned char peri_en_ldo15_eco : 1;
        unsigned char peri_en_ldo12_eco : 1;
    } reg;
} PMIC_PERI_CTRL5_UNION;
#endif
#define PMIC_PERI_CTRL5_peri_en_ldo30_eco_START (0)
#define PMIC_PERI_CTRL5_peri_en_ldo30_eco_END (0)
#define PMIC_PERI_CTRL5_peri_en_ldo29_eco_START (1)
#define PMIC_PERI_CTRL5_peri_en_ldo29_eco_END (1)
#define PMIC_PERI_CTRL5_peri_en_ldo27_eco_START (2)
#define PMIC_PERI_CTRL5_peri_en_ldo27_eco_END (2)
#define PMIC_PERI_CTRL5_peri_en_ldo24_eco_START (3)
#define PMIC_PERI_CTRL5_peri_en_ldo24_eco_END (3)
#define PMIC_PERI_CTRL5_peri_en_ldo23_eco_START (4)
#define PMIC_PERI_CTRL5_peri_en_ldo23_eco_END (4)
#define PMIC_PERI_CTRL5_peri_en_ldo18_eco_START (5)
#define PMIC_PERI_CTRL5_peri_en_ldo18_eco_END (5)
#define PMIC_PERI_CTRL5_peri_en_ldo15_eco_START (6)
#define PMIC_PERI_CTRL5_peri_en_ldo15_eco_END (6)
#define PMIC_PERI_CTRL5_peri_en_ldo12_eco_START (7)
#define PMIC_PERI_CTRL5_peri_en_ldo12_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_pmuh_eco : 1;
        unsigned char peri_en_ldo41_eco : 1;
        unsigned char peri_en_ldo40_eco : 1;
        unsigned char peri_en_ldo39_eco : 1;
        unsigned char peri_en_ldo38_eco : 1;
        unsigned char peri_en_ldo37_eco : 1;
        unsigned char peri_en_ldo36_eco : 1;
        unsigned char peri_en_ldo34_eco : 1;
    } reg;
} PMIC_PERI_CTRL6_UNION;
#endif
#define PMIC_PERI_CTRL6_peri_en_pmuh_eco_START (0)
#define PMIC_PERI_CTRL6_peri_en_pmuh_eco_END (0)
#define PMIC_PERI_CTRL6_peri_en_ldo41_eco_START (1)
#define PMIC_PERI_CTRL6_peri_en_ldo41_eco_END (1)
#define PMIC_PERI_CTRL6_peri_en_ldo40_eco_START (2)
#define PMIC_PERI_CTRL6_peri_en_ldo40_eco_END (2)
#define PMIC_PERI_CTRL6_peri_en_ldo39_eco_START (3)
#define PMIC_PERI_CTRL6_peri_en_ldo39_eco_END (3)
#define PMIC_PERI_CTRL6_peri_en_ldo38_eco_START (4)
#define PMIC_PERI_CTRL6_peri_en_ldo38_eco_END (4)
#define PMIC_PERI_CTRL6_peri_en_ldo37_eco_START (5)
#define PMIC_PERI_CTRL6_peri_en_ldo37_eco_END (5)
#define PMIC_PERI_CTRL6_peri_en_ldo36_eco_START (6)
#define PMIC_PERI_CTRL6_peri_en_ldo36_eco_END (6)
#define PMIC_PERI_CTRL6_peri_en_ldo34_eco_START (7)
#define PMIC_PERI_CTRL6_peri_en_ldo34_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_pmua_eco : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_PERI_CTRL7_UNION;
#endif
#define PMIC_PERI_CTRL7_peri_en_pmua_eco_START (0)
#define PMIC_PERI_CTRL7_peri_en_pmua_eco_END (0)
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
} PMIC_PERI_CTRL8_UNION;
#endif
#define PMIC_PERI_CTRL8_peri_buck0_sel_START (0)
#define PMIC_PERI_CTRL8_peri_buck0_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_buck2_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL9_UNION;
#endif
#define PMIC_PERI_CTRL9_peri_buck2_sel_START (0)
#define PMIC_PERI_CTRL9_peri_buck2_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_buck4_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL10_UNION;
#endif
#define PMIC_PERI_CTRL10_peri_buck4_sel_START (0)
#define PMIC_PERI_CTRL10_peri_buck4_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_buck6_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL11_UNION;
#endif
#define PMIC_PERI_CTRL11_peri_buck6_sel_START (0)
#define PMIC_PERI_CTRL11_peri_buck6_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_buck7_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL12_UNION;
#endif
#define PMIC_PERI_CTRL12_peri_buck7_sel_START (0)
#define PMIC_PERI_CTRL12_peri_buck7_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_buck9_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL13_UNION;
#endif
#define PMIC_PERI_CTRL13_peri_buck9_sel_START (0)
#define PMIC_PERI_CTRL13_peri_buck9_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo0_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL14_UNION;
#endif
#define PMIC_PERI_CTRL14_peri_ldo0_sel_START (0)
#define PMIC_PERI_CTRL14_peri_ldo0_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo5_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL15_UNION;
#endif
#define PMIC_PERI_CTRL15_peri_ldo5_sel_START (0)
#define PMIC_PERI_CTRL15_peri_ldo5_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo6_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL16_UNION;
#endif
#define PMIC_PERI_CTRL16_peri_ldo6_sel_START (0)
#define PMIC_PERI_CTRL16_peri_ldo6_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo7_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL17_UNION;
#endif
#define PMIC_PERI_CTRL17_peri_ldo7_sel_START (0)
#define PMIC_PERI_CTRL17_peri_ldo7_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo23_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL18_UNION;
#endif
#define PMIC_PERI_CTRL18_peri_ldo23_sel_START (0)
#define PMIC_PERI_CTRL18_peri_ldo23_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo28_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL19_UNION;
#endif
#define PMIC_PERI_CTRL19_peri_ldo28_sel_START (0)
#define PMIC_PERI_CTRL19_peri_ldo28_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo29_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL20_UNION;
#endif
#define PMIC_PERI_CTRL20_peri_ldo29_sel_START (0)
#define PMIC_PERI_CTRL20_peri_ldo29_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo30_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL21_UNION;
#endif
#define PMIC_PERI_CTRL21_peri_ldo30_sel_START (0)
#define PMIC_PERI_CTRL21_peri_ldo30_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo36_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL22_UNION;
#endif
#define PMIC_PERI_CTRL22_peri_ldo36_sel_START (0)
#define PMIC_PERI_CTRL22_peri_ldo36_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo37_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL23_UNION;
#endif
#define PMIC_PERI_CTRL23_peri_ldo37_sel_START (0)
#define PMIC_PERI_CTRL23_peri_ldo37_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo39_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL24_UNION;
#endif
#define PMIC_PERI_CTRL24_peri_ldo39_sel_START (0)
#define PMIC_PERI_CTRL24_peri_ldo39_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo40_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL25_UNION;
#endif
#define PMIC_PERI_CTRL25_peri_ldo40_sel_START (0)
#define PMIC_PERI_CTRL25_peri_ldo40_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo41_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_CTRL26_UNION;
#endif
#define PMIC_PERI_CTRL26_peri_ldo41_sel_START (0)
#define PMIC_PERI_CTRL26_peri_ldo41_sel_END (4)
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
        unsigned char reserved : 4;
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
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_buck9_vset : 1;
        unsigned char peri_en_buck8_vset : 1;
        unsigned char peri_en_buck3_vset : 1;
        unsigned char peri_en_buck2_vset : 1;
        unsigned char peri_en_ldo39_vset : 1;
        unsigned char peri_en_ldo0_vset : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_PERI_VSET_CTRL_UNION;
#endif
#define PMIC_PERI_VSET_CTRL_peri_en_buck9_vset_START (0)
#define PMIC_PERI_VSET_CTRL_peri_en_buck9_vset_END (0)
#define PMIC_PERI_VSET_CTRL_peri_en_buck8_vset_START (1)
#define PMIC_PERI_VSET_CTRL_peri_en_buck8_vset_END (1)
#define PMIC_PERI_VSET_CTRL_peri_en_buck3_vset_START (2)
#define PMIC_PERI_VSET_CTRL_peri_en_buck3_vset_END (2)
#define PMIC_PERI_VSET_CTRL_peri_en_buck2_vset_START (3)
#define PMIC_PERI_VSET_CTRL_peri_en_buck2_vset_END (3)
#define PMIC_PERI_VSET_CTRL_peri_en_ldo39_vset_START (4)
#define PMIC_PERI_VSET_CTRL_peri_en_ldo39_vset_END (4)
#define PMIC_PERI_VSET_CTRL_peri_en_ldo0_vset_START (5)
#define PMIC_PERI_VSET_CTRL_peri_en_ldo0_vset_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_en_buck9_on : 1;
        unsigned char idle_en_buck7_on : 1;
        unsigned char idle_en_buck6_on : 1;
        unsigned char idle_en_buck4_on : 1;
        unsigned char idle_en_buck2_on : 1;
        unsigned char idle_en_buck02_on : 1;
        unsigned char idle_en_buck00_on : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_IDLE_CTRL0_UNION;
#endif
#define PMIC_IDLE_CTRL0_idle_en_buck9_on_START (0)
#define PMIC_IDLE_CTRL0_idle_en_buck9_on_END (0)
#define PMIC_IDLE_CTRL0_idle_en_buck7_on_START (1)
#define PMIC_IDLE_CTRL0_idle_en_buck7_on_END (1)
#define PMIC_IDLE_CTRL0_idle_en_buck6_on_START (2)
#define PMIC_IDLE_CTRL0_idle_en_buck6_on_END (2)
#define PMIC_IDLE_CTRL0_idle_en_buck4_on_START (3)
#define PMIC_IDLE_CTRL0_idle_en_buck4_on_END (3)
#define PMIC_IDLE_CTRL0_idle_en_buck2_on_START (4)
#define PMIC_IDLE_CTRL0_idle_en_buck2_on_END (4)
#define PMIC_IDLE_CTRL0_idle_en_buck02_on_START (5)
#define PMIC_IDLE_CTRL0_idle_en_buck02_on_END (5)
#define PMIC_IDLE_CTRL0_idle_en_buck00_on_START (6)
#define PMIC_IDLE_CTRL0_idle_en_buck00_on_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_en_ldo30_on : 1;
        unsigned char idle_en_ldo29_on : 1;
        unsigned char idle_en_ldo28_on : 1;
        unsigned char idle_en_ldo23_on : 1;
        unsigned char idle_en_ldo7_on : 1;
        unsigned char idle_en_ldo6_on : 1;
        unsigned char idle_en_ldo5_on : 1;
        unsigned char idle_en_ldo0_on : 1;
    } reg;
} PMIC_IDLE_CTRL1_UNION;
#endif
#define PMIC_IDLE_CTRL1_idle_en_ldo30_on_START (0)
#define PMIC_IDLE_CTRL1_idle_en_ldo30_on_END (0)
#define PMIC_IDLE_CTRL1_idle_en_ldo29_on_START (1)
#define PMIC_IDLE_CTRL1_idle_en_ldo29_on_END (1)
#define PMIC_IDLE_CTRL1_idle_en_ldo28_on_START (2)
#define PMIC_IDLE_CTRL1_idle_en_ldo28_on_END (2)
#define PMIC_IDLE_CTRL1_idle_en_ldo23_on_START (3)
#define PMIC_IDLE_CTRL1_idle_en_ldo23_on_END (3)
#define PMIC_IDLE_CTRL1_idle_en_ldo7_on_START (4)
#define PMIC_IDLE_CTRL1_idle_en_ldo7_on_END (4)
#define PMIC_IDLE_CTRL1_idle_en_ldo6_on_START (5)
#define PMIC_IDLE_CTRL1_idle_en_ldo6_on_END (5)
#define PMIC_IDLE_CTRL1_idle_en_ldo5_on_START (6)
#define PMIC_IDLE_CTRL1_idle_en_ldo5_on_END (6)
#define PMIC_IDLE_CTRL1_idle_en_ldo0_on_START (7)
#define PMIC_IDLE_CTRL1_idle_en_ldo0_on_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_en_ldo41_on : 1;
        unsigned char idle_en_ldo40_on : 1;
        unsigned char idle_en_ldo39_on : 1;
        unsigned char idle_en_ldo37_on : 1;
        unsigned char idle_en_ldo36_on : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL2_UNION;
#endif
#define PMIC_IDLE_CTRL2_idle_en_ldo41_on_START (0)
#define PMIC_IDLE_CTRL2_idle_en_ldo41_on_END (0)
#define PMIC_IDLE_CTRL2_idle_en_ldo40_on_START (1)
#define PMIC_IDLE_CTRL2_idle_en_ldo40_on_END (1)
#define PMIC_IDLE_CTRL2_idle_en_ldo39_on_START (2)
#define PMIC_IDLE_CTRL2_idle_en_ldo39_on_END (2)
#define PMIC_IDLE_CTRL2_idle_en_ldo37_on_START (3)
#define PMIC_IDLE_CTRL2_idle_en_ldo37_on_END (3)
#define PMIC_IDLE_CTRL2_idle_en_ldo36_on_START (4)
#define PMIC_IDLE_CTRL2_idle_en_ldo36_on_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_en_buck7_eco : 1;
        unsigned char idle_en_buck6_eco : 1;
        unsigned char idle_en_buck4_eco : 1;
        unsigned char idle_en_buck3_eco : 1;
        unsigned char idle_en_buck2_eco : 1;
        unsigned char idle_en_buck1_eco : 1;
        unsigned char idle_en_buck02_eco : 1;
        unsigned char idle_en_buck00_eco : 1;
    } reg;
} PMIC_IDLE_CTRL3_UNION;
#endif
#define PMIC_IDLE_CTRL3_idle_en_buck7_eco_START (0)
#define PMIC_IDLE_CTRL3_idle_en_buck7_eco_END (0)
#define PMIC_IDLE_CTRL3_idle_en_buck6_eco_START (1)
#define PMIC_IDLE_CTRL3_idle_en_buck6_eco_END (1)
#define PMIC_IDLE_CTRL3_idle_en_buck4_eco_START (2)
#define PMIC_IDLE_CTRL3_idle_en_buck4_eco_END (2)
#define PMIC_IDLE_CTRL3_idle_en_buck3_eco_START (3)
#define PMIC_IDLE_CTRL3_idle_en_buck3_eco_END (3)
#define PMIC_IDLE_CTRL3_idle_en_buck2_eco_START (4)
#define PMIC_IDLE_CTRL3_idle_en_buck2_eco_END (4)
#define PMIC_IDLE_CTRL3_idle_en_buck1_eco_START (5)
#define PMIC_IDLE_CTRL3_idle_en_buck1_eco_END (5)
#define PMIC_IDLE_CTRL3_idle_en_buck02_eco_START (6)
#define PMIC_IDLE_CTRL3_idle_en_buck02_eco_END (6)
#define PMIC_IDLE_CTRL3_idle_en_buck00_eco_START (7)
#define PMIC_IDLE_CTRL3_idle_en_buck00_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_en_ldo11_eco : 1;
        unsigned char idle_en_ldo8_eco : 1;
        unsigned char idle_en_ldo7_eco : 1;
        unsigned char idle_en_ldo6_eco : 1;
        unsigned char idle_en_ldo5_eco : 1;
        unsigned char idle_en_ldo0_eco : 1;
        unsigned char idle_en_buck9_eco : 1;
        unsigned char idle_en_buck8_eco : 1;
    } reg;
} PMIC_IDLE_CTRL4_UNION;
#endif
#define PMIC_IDLE_CTRL4_idle_en_ldo11_eco_START (0)
#define PMIC_IDLE_CTRL4_idle_en_ldo11_eco_END (0)
#define PMIC_IDLE_CTRL4_idle_en_ldo8_eco_START (1)
#define PMIC_IDLE_CTRL4_idle_en_ldo8_eco_END (1)
#define PMIC_IDLE_CTRL4_idle_en_ldo7_eco_START (2)
#define PMIC_IDLE_CTRL4_idle_en_ldo7_eco_END (2)
#define PMIC_IDLE_CTRL4_idle_en_ldo6_eco_START (3)
#define PMIC_IDLE_CTRL4_idle_en_ldo6_eco_END (3)
#define PMIC_IDLE_CTRL4_idle_en_ldo5_eco_START (4)
#define PMIC_IDLE_CTRL4_idle_en_ldo5_eco_END (4)
#define PMIC_IDLE_CTRL4_idle_en_ldo0_eco_START (5)
#define PMIC_IDLE_CTRL4_idle_en_ldo0_eco_END (5)
#define PMIC_IDLE_CTRL4_idle_en_buck9_eco_START (6)
#define PMIC_IDLE_CTRL4_idle_en_buck9_eco_END (6)
#define PMIC_IDLE_CTRL4_idle_en_buck8_eco_START (7)
#define PMIC_IDLE_CTRL4_idle_en_buck8_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_en_ldo30_eco : 1;
        unsigned char idle_en_ldo29_eco : 1;
        unsigned char idle_en_ldo27_eco : 1;
        unsigned char idle_en_ldo24_eco : 1;
        unsigned char idle_en_ldo23_eco : 1;
        unsigned char idle_en_ldo18_eco : 1;
        unsigned char idle_en_ldo15_eco : 1;
        unsigned char idle_en_ldo12_eco : 1;
    } reg;
} PMIC_IDLE_CTRL5_UNION;
#endif
#define PMIC_IDLE_CTRL5_idle_en_ldo30_eco_START (0)
#define PMIC_IDLE_CTRL5_idle_en_ldo30_eco_END (0)
#define PMIC_IDLE_CTRL5_idle_en_ldo29_eco_START (1)
#define PMIC_IDLE_CTRL5_idle_en_ldo29_eco_END (1)
#define PMIC_IDLE_CTRL5_idle_en_ldo27_eco_START (2)
#define PMIC_IDLE_CTRL5_idle_en_ldo27_eco_END (2)
#define PMIC_IDLE_CTRL5_idle_en_ldo24_eco_START (3)
#define PMIC_IDLE_CTRL5_idle_en_ldo24_eco_END (3)
#define PMIC_IDLE_CTRL5_idle_en_ldo23_eco_START (4)
#define PMIC_IDLE_CTRL5_idle_en_ldo23_eco_END (4)
#define PMIC_IDLE_CTRL5_idle_en_ldo18_eco_START (5)
#define PMIC_IDLE_CTRL5_idle_en_ldo18_eco_END (5)
#define PMIC_IDLE_CTRL5_idle_en_ldo15_eco_START (6)
#define PMIC_IDLE_CTRL5_idle_en_ldo15_eco_END (6)
#define PMIC_IDLE_CTRL5_idle_en_ldo12_eco_START (7)
#define PMIC_IDLE_CTRL5_idle_en_ldo12_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_en_pmuh_eco : 1;
        unsigned char idle_en_ldo41_eco : 1;
        unsigned char idle_en_ldo40_eco : 1;
        unsigned char idle_en_ldo39_eco : 1;
        unsigned char idle_en_ldo38_eco : 1;
        unsigned char idle_en_ldo37_eco : 1;
        unsigned char idle_en_ldo36_eco : 1;
        unsigned char idle_en_ldo34_eco : 1;
    } reg;
} PMIC_IDLE_CTRL6_UNION;
#endif
#define PMIC_IDLE_CTRL6_idle_en_pmuh_eco_START (0)
#define PMIC_IDLE_CTRL6_idle_en_pmuh_eco_END (0)
#define PMIC_IDLE_CTRL6_idle_en_ldo41_eco_START (1)
#define PMIC_IDLE_CTRL6_idle_en_ldo41_eco_END (1)
#define PMIC_IDLE_CTRL6_idle_en_ldo40_eco_START (2)
#define PMIC_IDLE_CTRL6_idle_en_ldo40_eco_END (2)
#define PMIC_IDLE_CTRL6_idle_en_ldo39_eco_START (3)
#define PMIC_IDLE_CTRL6_idle_en_ldo39_eco_END (3)
#define PMIC_IDLE_CTRL6_idle_en_ldo38_eco_START (4)
#define PMIC_IDLE_CTRL6_idle_en_ldo38_eco_END (4)
#define PMIC_IDLE_CTRL6_idle_en_ldo37_eco_START (5)
#define PMIC_IDLE_CTRL6_idle_en_ldo37_eco_END (5)
#define PMIC_IDLE_CTRL6_idle_en_ldo36_eco_START (6)
#define PMIC_IDLE_CTRL6_idle_en_ldo36_eco_END (6)
#define PMIC_IDLE_CTRL6_idle_en_ldo34_eco_START (7)
#define PMIC_IDLE_CTRL6_idle_en_ldo34_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_en_pmua_eco : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_IDLE_CTRL7_UNION;
#endif
#define PMIC_IDLE_CTRL7_idle_en_pmua_eco_START (0)
#define PMIC_IDLE_CTRL7_idle_en_pmua_eco_END (0)
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
} PMIC_IDLE_CTRL8_UNION;
#endif
#define PMIC_IDLE_CTRL8_idle_buck0_sel_START (0)
#define PMIC_IDLE_CTRL8_idle_buck0_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_buck2_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL9_UNION;
#endif
#define PMIC_IDLE_CTRL9_idle_buck2_sel_START (0)
#define PMIC_IDLE_CTRL9_idle_buck2_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_buck4_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL10_UNION;
#endif
#define PMIC_IDLE_CTRL10_idle_buck4_sel_START (0)
#define PMIC_IDLE_CTRL10_idle_buck4_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_buck6_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL11_UNION;
#endif
#define PMIC_IDLE_CTRL11_idle_buck6_sel_START (0)
#define PMIC_IDLE_CTRL11_idle_buck6_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_buck7_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL12_UNION;
#endif
#define PMIC_IDLE_CTRL12_idle_buck7_sel_START (0)
#define PMIC_IDLE_CTRL12_idle_buck7_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_buck9_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL13_UNION;
#endif
#define PMIC_IDLE_CTRL13_idle_buck9_sel_START (0)
#define PMIC_IDLE_CTRL13_idle_buck9_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo0_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL14_UNION;
#endif
#define PMIC_IDLE_CTRL14_idle_ldo0_sel_START (0)
#define PMIC_IDLE_CTRL14_idle_ldo0_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo5_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL15_UNION;
#endif
#define PMIC_IDLE_CTRL15_idle_ldo5_sel_START (0)
#define PMIC_IDLE_CTRL15_idle_ldo5_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo6_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL16_UNION;
#endif
#define PMIC_IDLE_CTRL16_idle_ldo6_sel_START (0)
#define PMIC_IDLE_CTRL16_idle_ldo6_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo7_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL17_UNION;
#endif
#define PMIC_IDLE_CTRL17_idle_ldo7_sel_START (0)
#define PMIC_IDLE_CTRL17_idle_ldo7_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo23_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL18_UNION;
#endif
#define PMIC_IDLE_CTRL18_idle_ldo23_sel_START (0)
#define PMIC_IDLE_CTRL18_idle_ldo23_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo28_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL19_UNION;
#endif
#define PMIC_IDLE_CTRL19_idle_ldo28_sel_START (0)
#define PMIC_IDLE_CTRL19_idle_ldo28_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo29_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL20_UNION;
#endif
#define PMIC_IDLE_CTRL20_idle_ldo29_sel_START (0)
#define PMIC_IDLE_CTRL20_idle_ldo29_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo30_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL21_UNION;
#endif
#define PMIC_IDLE_CTRL21_idle_ldo30_sel_START (0)
#define PMIC_IDLE_CTRL21_idle_ldo30_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo36_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL22_UNION;
#endif
#define PMIC_IDLE_CTRL22_idle_ldo36_sel_START (0)
#define PMIC_IDLE_CTRL22_idle_ldo36_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo37_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL23_UNION;
#endif
#define PMIC_IDLE_CTRL23_idle_ldo37_sel_START (0)
#define PMIC_IDLE_CTRL23_idle_ldo37_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo39_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL24_UNION;
#endif
#define PMIC_IDLE_CTRL24_idle_ldo39_sel_START (0)
#define PMIC_IDLE_CTRL24_idle_ldo39_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo40_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL25_UNION;
#endif
#define PMIC_IDLE_CTRL25_idle_ldo40_sel_START (0)
#define PMIC_IDLE_CTRL25_idle_ldo40_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo41_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_CTRL26_UNION;
#endif
#define PMIC_IDLE_CTRL26_idle_ldo41_sel_START (0)
#define PMIC_IDLE_CTRL26_idle_ldo41_sel_END (4)
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
        unsigned char en_dr1_int : 1;
        unsigned char en_dr2_int : 1;
        unsigned char en_dr3_int : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_DR_EN_123_UNION;
#endif
#define PMIC_DR_EN_123_en_dr1_int_START (0)
#define PMIC_DR_EN_123_en_dr1_int_END (0)
#define PMIC_DR_EN_123_en_dr2_int_START (1)
#define PMIC_DR_EN_123_en_dr2_int_END (1)
#define PMIC_DR_EN_123_en_dr3_int_START (2)
#define PMIC_DR_EN_123_en_dr3_int_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_dr4_sl_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_DR_EN_4_UNION;
#endif
#define PMIC_DR_EN_4_reg_dr4_sl_en_START (0)
#define PMIC_DR_EN_4_reg_dr4_sl_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr1_mode : 1;
        unsigned char dr2_mode : 1;
        unsigned char dr3_mode : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_DR_EN_MODE_123_UNION;
#endif
#define PMIC_DR_EN_MODE_123_dr1_mode_START (0)
#define PMIC_DR_EN_MODE_123_dr1_mode_END (0)
#define PMIC_DR_EN_MODE_123_dr2_mode_START (1)
#define PMIC_DR_EN_MODE_123_dr2_mode_END (1)
#define PMIC_DR_EN_MODE_123_dr3_mode_START (2)
#define PMIC_DR_EN_MODE_123_dr3_mode_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_period_dr1 : 8;
    } reg;
} PMIC_FLASH_PERIOD_DR1_UNION;
#endif
#define PMIC_FLASH_PERIOD_DR1_flash_period_dr1_START (0)
#define PMIC_FLASH_PERIOD_DR1_flash_period_dr1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_on_dr1 : 8;
    } reg;
} PMIC_FLASH_ON_DR1_UNION;
#endif
#define PMIC_FLASH_ON_DR1_flash_on_dr1_START (0)
#define PMIC_FLASH_ON_DR1_flash_on_dr1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_period_dr2 : 8;
    } reg;
} PMIC_FLASH_PERIOD_DR2_UNION;
#endif
#define PMIC_FLASH_PERIOD_DR2_flash_period_dr2_START (0)
#define PMIC_FLASH_PERIOD_DR2_flash_period_dr2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_on_dr2 : 8;
    } reg;
} PMIC_FLASH_ON_DR2_UNION;
#endif
#define PMIC_FLASH_ON_DR2_flash_on_dr2_START (0)
#define PMIC_FLASH_ON_DR2_flash_on_dr2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_period_dr3 : 8;
    } reg;
} PMIC_FLASH_PERIOD_DR3_UNION;
#endif
#define PMIC_FLASH_PERIOD_DR3_flash_period_dr3_START (0)
#define PMIC_FLASH_PERIOD_DR3_flash_period_dr3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flash_on_dr3 : 8;
    } reg;
} PMIC_FLASH_ON_DR3_UNION;
#endif
#define PMIC_FLASH_ON_DR3_flash_on_dr3_START (0)
#define PMIC_FLASH_ON_DR3_flash_on_dr3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr1_mode_sel : 1;
        unsigned char dr2_mode_sel : 1;
        unsigned char dr3_mode_sel : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_DR_MODE_SEL_UNION;
#endif
#define PMIC_DR_MODE_SEL_dr1_mode_sel_START (0)
#define PMIC_DR_MODE_SEL_dr1_mode_sel_END (0)
#define PMIC_DR_MODE_SEL_dr2_mode_sel_START (1)
#define PMIC_DR_MODE_SEL_dr2_mode_sel_END (1)
#define PMIC_DR_MODE_SEL_dr3_mode_sel_START (2)
#define PMIC_DR_MODE_SEL_dr3_mode_sel_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_dr1_en : 1;
        unsigned char reg_dr2_en : 1;
        unsigned char reg_dr3_en : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_DR_BRE_CTRL_UNION;
#endif
#define PMIC_DR_BRE_CTRL_reg_dr1_en_START (0)
#define PMIC_DR_BRE_CTRL_reg_dr1_en_END (0)
#define PMIC_DR_BRE_CTRL_reg_dr2_en_START (1)
#define PMIC_DR_BRE_CTRL_reg_dr2_en_END (1)
#define PMIC_DR_BRE_CTRL_reg_dr3_en_START (2)
#define PMIC_DR_BRE_CTRL_reg_dr3_en_END (2)
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
        unsigned char dr1_out_ctrl : 2;
        unsigned char dr2_out_ctrl : 2;
        unsigned char dr3_out_ctrl : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_DR_OUT_CTRL_UNION;
#endif
#define PMIC_DR_OUT_CTRL_dr1_out_ctrl_START (0)
#define PMIC_DR_OUT_CTRL_dr1_out_ctrl_END (1)
#define PMIC_DR_OUT_CTRL_dr2_out_ctrl_START (2)
#define PMIC_DR_OUT_CTRL_dr2_out_ctrl_END (3)
#define PMIC_DR_OUT_CTRL_dr3_out_ctrl_START (4)
#define PMIC_DR_OUT_CTRL_dr3_out_ctrl_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr1_start_delay : 8;
    } reg;
} PMIC_DR1_START_DEL_UNION;
#endif
#define PMIC_DR1_START_DEL_dr1_start_delay_START (0)
#define PMIC_DR1_START_DEL_dr1_start_delay_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr2_start_delay : 8;
    } reg;
} PMIC_DR2_START_DEL_UNION;
#endif
#define PMIC_DR2_START_DEL_dr2_start_delay_START (0)
#define PMIC_DR2_START_DEL_dr2_start_delay_END (7)
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
        unsigned char dr1_t_off : 4;
        unsigned char dr1_t_on : 4;
    } reg;
} PMIC_DR1_TIM_CONF0_UNION;
#endif
#define PMIC_DR1_TIM_CONF0_dr1_t_off_START (0)
#define PMIC_DR1_TIM_CONF0_dr1_t_off_END (3)
#define PMIC_DR1_TIM_CONF0_dr1_t_on_START (4)
#define PMIC_DR1_TIM_CONF0_dr1_t_on_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr1_t_rise : 3;
        unsigned char reserved_0 : 1;
        unsigned char dr1_t_fall : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_DR1_TIM_CONF1_UNION;
#endif
#define PMIC_DR1_TIM_CONF1_dr1_t_rise_START (0)
#define PMIC_DR1_TIM_CONF1_dr1_t_rise_END (2)
#define PMIC_DR1_TIM_CONF1_dr1_t_fall_START (4)
#define PMIC_DR1_TIM_CONF1_dr1_t_fall_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr2_t_off : 4;
        unsigned char dr2_t_on : 4;
    } reg;
} PMIC_DR2_TIM_CONF0_UNION;
#endif
#define PMIC_DR2_TIM_CONF0_dr2_t_off_START (0)
#define PMIC_DR2_TIM_CONF0_dr2_t_off_END (3)
#define PMIC_DR2_TIM_CONF0_dr2_t_on_START (4)
#define PMIC_DR2_TIM_CONF0_dr2_t_on_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr2_t_rise : 3;
        unsigned char reserved_0 : 1;
        unsigned char dr2_t_fall : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_DR2_TIM_CONF1_UNION;
#endif
#define PMIC_DR2_TIM_CONF1_dr2_t_rise_START (0)
#define PMIC_DR2_TIM_CONF1_dr2_t_rise_END (2)
#define PMIC_DR2_TIM_CONF1_dr2_t_fall_START (4)
#define PMIC_DR2_TIM_CONF1_dr2_t_fall_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr3_t_off : 4;
        unsigned char dr3_t_on : 4;
    } reg;
} PMIC_DR3_TIM_CONF0_UNION;
#endif
#define PMIC_DR3_TIM_CONF0_dr3_t_off_START (0)
#define PMIC_DR3_TIM_CONF0_dr3_t_off_END (3)
#define PMIC_DR3_TIM_CONF0_dr3_t_on_START (4)
#define PMIC_DR3_TIM_CONF0_dr3_t_on_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dr3_t_rise : 3;
        unsigned char reserved_0 : 1;
        unsigned char dr3_t_fall : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_DR3_TIM_CONF1_UNION;
#endif
#define PMIC_DR3_TIM_CONF1_dr3_t_rise_START (0)
#define PMIC_DR3_TIM_CONF1_dr3_t_rise_END (2)
#define PMIC_DR3_TIM_CONF1_dr3_t_fall_START (4)
#define PMIC_DR3_TIM_CONF1_dr3_t_fall_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 8;
    } reg;
} PMIC_DR4_CTRLRESERVE_UNION;
#endif
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
        unsigned char otp0_pwe_int : 1;
        unsigned char otp0_pwe_pulse : 1;
        unsigned char otp0_por_int : 1;
        unsigned char otp0_por_pulse : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_OTP0_CTRL0_UNION;
#endif
#define PMIC_OTP0_CTRL0_otp0_pwe_int_START (0)
#define PMIC_OTP0_CTRL0_otp0_pwe_int_END (0)
#define PMIC_OTP0_CTRL0_otp0_pwe_pulse_START (1)
#define PMIC_OTP0_CTRL0_otp0_pwe_pulse_END (1)
#define PMIC_OTP0_CTRL0_otp0_por_int_START (2)
#define PMIC_OTP0_CTRL0_otp0_por_int_END (2)
#define PMIC_OTP0_CTRL0_otp0_por_pulse_START (3)
#define PMIC_OTP0_CTRL0_otp0_por_pulse_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pprog : 1;
        unsigned char otp0_inctrl_sel : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_OTP0_CTRL1_UNION;
#endif
#define PMIC_OTP0_CTRL1_otp0_pprog_START (0)
#define PMIC_OTP0_CTRL1_otp0_pprog_END (0)
#define PMIC_OTP0_CTRL1_otp0_inctrl_sel_START (1)
#define PMIC_OTP0_CTRL1_otp0_inctrl_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pa_cfg : 6;
        unsigned char otp0_ptm : 2;
    } reg;
} PMIC_OTP0_CTRL2_UNION;
#endif
#define PMIC_OTP0_CTRL2_otp0_pa_cfg_START (0)
#define PMIC_OTP0_CTRL2_otp0_pa_cfg_END (5)
#define PMIC_OTP0_CTRL2_otp0_ptm_START (6)
#define PMIC_OTP0_CTRL2_otp0_ptm_END (7)
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
        unsigned char otp0_pdob4_w : 8;
    } reg;
} PMIC_OTP0_4_W_UNION;
#endif
#define PMIC_OTP0_4_W_otp0_pdob4_w_START (0)
#define PMIC_OTP0_4_W_otp0_pdob4_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob5_w : 8;
    } reg;
} PMIC_OTP0_5_W_UNION;
#endif
#define PMIC_OTP0_5_W_otp0_pdob5_w_START (0)
#define PMIC_OTP0_5_W_otp0_pdob5_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob6_w : 8;
    } reg;
} PMIC_OTP0_6_W_UNION;
#endif
#define PMIC_OTP0_6_W_otp0_pdob6_w_START (0)
#define PMIC_OTP0_6_W_otp0_pdob6_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob7_w : 8;
    } reg;
} PMIC_OTP0_7_W_UNION;
#endif
#define PMIC_OTP0_7_W_otp0_pdob7_w_START (0)
#define PMIC_OTP0_7_W_otp0_pdob7_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob8_w : 8;
    } reg;
} PMIC_OTP0_8_W_UNION;
#endif
#define PMIC_OTP0_8_W_otp0_pdob8_w_START (0)
#define PMIC_OTP0_8_W_otp0_pdob8_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob9_w : 8;
    } reg;
} PMIC_OTP0_9_W_UNION;
#endif
#define PMIC_OTP0_9_W_otp0_pdob9_w_START (0)
#define PMIC_OTP0_9_W_otp0_pdob9_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob10_w : 8;
    } reg;
} PMIC_OTP0_10_W_UNION;
#endif
#define PMIC_OTP0_10_W_otp0_pdob10_w_START (0)
#define PMIC_OTP0_10_W_otp0_pdob10_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob11_w : 8;
    } reg;
} PMIC_OTP0_11_W_UNION;
#endif
#define PMIC_OTP0_11_W_otp0_pdob11_w_START (0)
#define PMIC_OTP0_11_W_otp0_pdob11_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob12_w : 8;
    } reg;
} PMIC_OTP0_12_W_UNION;
#endif
#define PMIC_OTP0_12_W_otp0_pdob12_w_START (0)
#define PMIC_OTP0_12_W_otp0_pdob12_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob13_w : 8;
    } reg;
} PMIC_OTP0_13_W_UNION;
#endif
#define PMIC_OTP0_13_W_otp0_pdob13_w_START (0)
#define PMIC_OTP0_13_W_otp0_pdob13_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob14_w : 8;
    } reg;
} PMIC_OTP0_14_W_UNION;
#endif
#define PMIC_OTP0_14_W_otp0_pdob14_w_START (0)
#define PMIC_OTP0_14_W_otp0_pdob14_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob15_w : 8;
    } reg;
} PMIC_OTP0_15_W_UNION;
#endif
#define PMIC_OTP0_15_W_otp0_pdob15_w_START (0)
#define PMIC_OTP0_15_W_otp0_pdob15_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob16_w : 8;
    } reg;
} PMIC_OTP0_16_W_UNION;
#endif
#define PMIC_OTP0_16_W_otp0_pdob16_w_START (0)
#define PMIC_OTP0_16_W_otp0_pdob16_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob17_w : 8;
    } reg;
} PMIC_OTP0_17_W_UNION;
#endif
#define PMIC_OTP0_17_W_otp0_pdob17_w_START (0)
#define PMIC_OTP0_17_W_otp0_pdob17_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob18_w : 8;
    } reg;
} PMIC_OTP0_18_W_UNION;
#endif
#define PMIC_OTP0_18_W_otp0_pdob18_w_START (0)
#define PMIC_OTP0_18_W_otp0_pdob18_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob19_w : 8;
    } reg;
} PMIC_OTP0_19_W_UNION;
#endif
#define PMIC_OTP0_19_W_otp0_pdob19_w_START (0)
#define PMIC_OTP0_19_W_otp0_pdob19_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob20_w : 8;
    } reg;
} PMIC_OTP0_20_W_UNION;
#endif
#define PMIC_OTP0_20_W_otp0_pdob20_w_START (0)
#define PMIC_OTP0_20_W_otp0_pdob20_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob21_w : 8;
    } reg;
} PMIC_OTP0_21_W_UNION;
#endif
#define PMIC_OTP0_21_W_otp0_pdob21_w_START (0)
#define PMIC_OTP0_21_W_otp0_pdob21_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob22_w : 8;
    } reg;
} PMIC_OTP0_22_W_UNION;
#endif
#define PMIC_OTP0_22_W_otp0_pdob22_w_START (0)
#define PMIC_OTP0_22_W_otp0_pdob22_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob23_w : 8;
    } reg;
} PMIC_OTP0_23_W_UNION;
#endif
#define PMIC_OTP0_23_W_otp0_pdob23_w_START (0)
#define PMIC_OTP0_23_W_otp0_pdob23_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob24_w : 8;
    } reg;
} PMIC_OTP0_24_W_UNION;
#endif
#define PMIC_OTP0_24_W_otp0_pdob24_w_START (0)
#define PMIC_OTP0_24_W_otp0_pdob24_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob25_w : 8;
    } reg;
} PMIC_OTP0_25_W_UNION;
#endif
#define PMIC_OTP0_25_W_otp0_pdob25_w_START (0)
#define PMIC_OTP0_25_W_otp0_pdob25_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob26_w : 8;
    } reg;
} PMIC_OTP0_26_W_UNION;
#endif
#define PMIC_OTP0_26_W_otp0_pdob26_w_START (0)
#define PMIC_OTP0_26_W_otp0_pdob26_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob27_w : 8;
    } reg;
} PMIC_OTP0_27_W_UNION;
#endif
#define PMIC_OTP0_27_W_otp0_pdob27_w_START (0)
#define PMIC_OTP0_27_W_otp0_pdob27_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob28_w : 8;
    } reg;
} PMIC_OTP0_28_W_UNION;
#endif
#define PMIC_OTP0_28_W_otp0_pdob28_w_START (0)
#define PMIC_OTP0_28_W_otp0_pdob28_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob29_w : 8;
    } reg;
} PMIC_OTP0_29_W_UNION;
#endif
#define PMIC_OTP0_29_W_otp0_pdob29_w_START (0)
#define PMIC_OTP0_29_W_otp0_pdob29_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob30_w : 8;
    } reg;
} PMIC_OTP0_30_W_UNION;
#endif
#define PMIC_OTP0_30_W_otp0_pdob30_w_START (0)
#define PMIC_OTP0_30_W_otp0_pdob30_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob31_w : 8;
    } reg;
} PMIC_OTP0_31_W_UNION;
#endif
#define PMIC_OTP0_31_W_otp0_pdob31_w_START (0)
#define PMIC_OTP0_31_W_otp0_pdob31_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob32_w : 8;
    } reg;
} PMIC_OTP0_32_W_UNION;
#endif
#define PMIC_OTP0_32_W_otp0_pdob32_w_START (0)
#define PMIC_OTP0_32_W_otp0_pdob32_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob33_w : 8;
    } reg;
} PMIC_OTP0_33_W_UNION;
#endif
#define PMIC_OTP0_33_W_otp0_pdob33_w_START (0)
#define PMIC_OTP0_33_W_otp0_pdob33_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob34_w : 8;
    } reg;
} PMIC_OTP0_34_W_UNION;
#endif
#define PMIC_OTP0_34_W_otp0_pdob34_w_START (0)
#define PMIC_OTP0_34_W_otp0_pdob34_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob35_w : 8;
    } reg;
} PMIC_OTP0_35_W_UNION;
#endif
#define PMIC_OTP0_35_W_otp0_pdob35_w_START (0)
#define PMIC_OTP0_35_W_otp0_pdob35_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob36_w : 8;
    } reg;
} PMIC_OTP0_36_W_UNION;
#endif
#define PMIC_OTP0_36_W_otp0_pdob36_w_START (0)
#define PMIC_OTP0_36_W_otp0_pdob36_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob37_w : 8;
    } reg;
} PMIC_OTP0_37_W_UNION;
#endif
#define PMIC_OTP0_37_W_otp0_pdob37_w_START (0)
#define PMIC_OTP0_37_W_otp0_pdob37_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob38_w : 8;
    } reg;
} PMIC_OTP0_38_W_UNION;
#endif
#define PMIC_OTP0_38_W_otp0_pdob38_w_START (0)
#define PMIC_OTP0_38_W_otp0_pdob38_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob39_w : 8;
    } reg;
} PMIC_OTP0_39_W_UNION;
#endif
#define PMIC_OTP0_39_W_otp0_pdob39_w_START (0)
#define PMIC_OTP0_39_W_otp0_pdob39_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob40_w : 8;
    } reg;
} PMIC_OTP0_40_W_UNION;
#endif
#define PMIC_OTP0_40_W_otp0_pdob40_w_START (0)
#define PMIC_OTP0_40_W_otp0_pdob40_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob41_w : 8;
    } reg;
} PMIC_OTP0_41_W_UNION;
#endif
#define PMIC_OTP0_41_W_otp0_pdob41_w_START (0)
#define PMIC_OTP0_41_W_otp0_pdob41_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob42_w : 8;
    } reg;
} PMIC_OTP0_42_W_UNION;
#endif
#define PMIC_OTP0_42_W_otp0_pdob42_w_START (0)
#define PMIC_OTP0_42_W_otp0_pdob42_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob43_w : 8;
    } reg;
} PMIC_OTP0_43_W_UNION;
#endif
#define PMIC_OTP0_43_W_otp0_pdob43_w_START (0)
#define PMIC_OTP0_43_W_otp0_pdob43_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob44_w : 8;
    } reg;
} PMIC_OTP0_44_W_UNION;
#endif
#define PMIC_OTP0_44_W_otp0_pdob44_w_START (0)
#define PMIC_OTP0_44_W_otp0_pdob44_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob45_w : 8;
    } reg;
} PMIC_OTP0_45_W_UNION;
#endif
#define PMIC_OTP0_45_W_otp0_pdob45_w_START (0)
#define PMIC_OTP0_45_W_otp0_pdob45_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob46_w : 8;
    } reg;
} PMIC_OTP0_46_W_UNION;
#endif
#define PMIC_OTP0_46_W_otp0_pdob46_w_START (0)
#define PMIC_OTP0_46_W_otp0_pdob46_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob47_w : 8;
    } reg;
} PMIC_OTP0_47_W_UNION;
#endif
#define PMIC_OTP0_47_W_otp0_pdob47_w_START (0)
#define PMIC_OTP0_47_W_otp0_pdob47_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob48_w : 8;
    } reg;
} PMIC_OTP0_48_W_UNION;
#endif
#define PMIC_OTP0_48_W_otp0_pdob48_w_START (0)
#define PMIC_OTP0_48_W_otp0_pdob48_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob49_w : 8;
    } reg;
} PMIC_OTP0_49_W_UNION;
#endif
#define PMIC_OTP0_49_W_otp0_pdob49_w_START (0)
#define PMIC_OTP0_49_W_otp0_pdob49_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob50_w : 8;
    } reg;
} PMIC_OTP0_50_W_UNION;
#endif
#define PMIC_OTP0_50_W_otp0_pdob50_w_START (0)
#define PMIC_OTP0_50_W_otp0_pdob50_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob51_w : 8;
    } reg;
} PMIC_OTP0_51_W_UNION;
#endif
#define PMIC_OTP0_51_W_otp0_pdob51_w_START (0)
#define PMIC_OTP0_51_W_otp0_pdob51_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob52_w : 8;
    } reg;
} PMIC_OTP0_52_W_UNION;
#endif
#define PMIC_OTP0_52_W_otp0_pdob52_w_START (0)
#define PMIC_OTP0_52_W_otp0_pdob52_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob53_w : 8;
    } reg;
} PMIC_OTP0_53_W_UNION;
#endif
#define PMIC_OTP0_53_W_otp0_pdob53_w_START (0)
#define PMIC_OTP0_53_W_otp0_pdob53_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob54_w : 8;
    } reg;
} PMIC_OTP0_54_W_UNION;
#endif
#define PMIC_OTP0_54_W_otp0_pdob54_w_START (0)
#define PMIC_OTP0_54_W_otp0_pdob54_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob55_w : 8;
    } reg;
} PMIC_OTP0_55_W_UNION;
#endif
#define PMIC_OTP0_55_W_otp0_pdob55_w_START (0)
#define PMIC_OTP0_55_W_otp0_pdob55_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob56_w : 8;
    } reg;
} PMIC_OTP0_56_W_UNION;
#endif
#define PMIC_OTP0_56_W_otp0_pdob56_w_START (0)
#define PMIC_OTP0_56_W_otp0_pdob56_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob57_w : 8;
    } reg;
} PMIC_OTP0_57_W_UNION;
#endif
#define PMIC_OTP0_57_W_otp0_pdob57_w_START (0)
#define PMIC_OTP0_57_W_otp0_pdob57_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob58_w : 8;
    } reg;
} PMIC_OTP0_58_W_UNION;
#endif
#define PMIC_OTP0_58_W_otp0_pdob58_w_START (0)
#define PMIC_OTP0_58_W_otp0_pdob58_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob59_w : 8;
    } reg;
} PMIC_OTP0_59_W_UNION;
#endif
#define PMIC_OTP0_59_W_otp0_pdob59_w_START (0)
#define PMIC_OTP0_59_W_otp0_pdob59_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob60_w : 8;
    } reg;
} PMIC_OTP0_60_W_UNION;
#endif
#define PMIC_OTP0_60_W_otp0_pdob60_w_START (0)
#define PMIC_OTP0_60_W_otp0_pdob60_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob61_w : 8;
    } reg;
} PMIC_OTP0_61_W_UNION;
#endif
#define PMIC_OTP0_61_W_otp0_pdob61_w_START (0)
#define PMIC_OTP0_61_W_otp0_pdob61_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob62_w : 8;
    } reg;
} PMIC_OTP0_62_W_UNION;
#endif
#define PMIC_OTP0_62_W_otp0_pdob62_w_START (0)
#define PMIC_OTP0_62_W_otp0_pdob62_w_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp0_pdob63_w : 8;
    } reg;
} PMIC_OTP0_63_W_UNION;
#endif
#define PMIC_OTP0_63_W_otp0_pdob63_w_START (0)
#define PMIC_OTP0_63_W_otp0_pdob63_w_END (7)
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
        unsigned char otp1_pwe_int : 1;
        unsigned char otp1_pwe_pulse : 1;
        unsigned char otp1_por_int : 1;
        unsigned char otp1_por_pulse : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_OTP1_CTRL0_UNION;
#endif
#define PMIC_OTP1_CTRL0_otp1_pwe_int_START (0)
#define PMIC_OTP1_CTRL0_otp1_pwe_int_END (0)
#define PMIC_OTP1_CTRL0_otp1_pwe_pulse_START (1)
#define PMIC_OTP1_CTRL0_otp1_pwe_pulse_END (1)
#define PMIC_OTP1_CTRL0_otp1_por_int_START (2)
#define PMIC_OTP1_CTRL0_otp1_por_int_END (2)
#define PMIC_OTP1_CTRL0_otp1_por_pulse_START (3)
#define PMIC_OTP1_CTRL0_otp1_por_pulse_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp1_pprog : 1;
        unsigned char otp1_inctrl_sel : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_OTP1_CTRL1_UNION;
#endif
#define PMIC_OTP1_CTRL1_otp1_pprog_START (0)
#define PMIC_OTP1_CTRL1_otp1_pprog_END (0)
#define PMIC_OTP1_CTRL1_otp1_inctrl_sel_START (1)
#define PMIC_OTP1_CTRL1_otp1_inctrl_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otp1_pa_cfg : 6;
        unsigned char otp1_ptm : 2;
    } reg;
} PMIC_OTP1_CTRL2_UNION;
#endif
#define PMIC_OTP1_CTRL2_otp1_pa_cfg_START (0)
#define PMIC_OTP1_CTRL2_otp1_pa_cfg_END (5)
#define PMIC_OTP1_CTRL2_otp1_ptm_START (6)
#define PMIC_OTP1_CTRL2_otp1_ptm_END (7)
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
        unsigned char buck00_rampdown_ctrl : 3;
        unsigned char buck00_rampup_ctrl : 3;
        unsigned char buck00_ramp_en_cfg : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_RAMP_BUCK00_CTRL0_UNION;
#endif
#define PMIC_RAMP_BUCK00_CTRL0_buck00_rampdown_ctrl_START (0)
#define PMIC_RAMP_BUCK00_CTRL0_buck00_rampdown_ctrl_END (2)
#define PMIC_RAMP_BUCK00_CTRL0_buck00_rampup_ctrl_START (3)
#define PMIC_RAMP_BUCK00_CTRL0_buck00_rampup_ctrl_END (5)
#define PMIC_RAMP_BUCK00_CTRL0_buck00_ramp_en_cfg_START (6)
#define PMIC_RAMP_BUCK00_CTRL0_buck00_ramp_en_cfg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_pull_down_off_cfg : 3;
        unsigned char buck00_pull_down_on_cfg : 3;
        unsigned char buck00_pull_down_cfg : 1;
        unsigned char buck00_pull_down_mode : 1;
    } reg;
} PMIC_RAMP_BUCK00_CTRL1_UNION;
#endif
#define PMIC_RAMP_BUCK00_CTRL1_buck00_pull_down_off_cfg_START (0)
#define PMIC_RAMP_BUCK00_CTRL1_buck00_pull_down_off_cfg_END (2)
#define PMIC_RAMP_BUCK00_CTRL1_buck00_pull_down_on_cfg_START (3)
#define PMIC_RAMP_BUCK00_CTRL1_buck00_pull_down_on_cfg_END (5)
#define PMIC_RAMP_BUCK00_CTRL1_buck00_pull_down_cfg_START (6)
#define PMIC_RAMP_BUCK00_CTRL1_buck00_pull_down_cfg_END (6)
#define PMIC_RAMP_BUCK00_CTRL1_buck00_pull_down_mode_START (7)
#define PMIC_RAMP_BUCK00_CTRL1_buck00_pull_down_mode_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_dly_ru_off_cfg : 3;
        unsigned char buck00_dly_ru_on_cfg : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK00_RAMPUP_STATE_UNION;
#endif
#define PMIC_BUCK00_RAMPUP_STATE_buck00_dly_ru_off_cfg_START (0)
#define PMIC_BUCK00_RAMPUP_STATE_buck00_dly_ru_off_cfg_END (2)
#define PMIC_BUCK00_RAMPUP_STATE_buck00_dly_ru_on_cfg_START (3)
#define PMIC_BUCK00_RAMPUP_STATE_buck00_dly_ru_on_cfg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_dly_rd_pd_off_cfg : 3;
        unsigned char buck00_dly_rd_pd_on_cfg : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK00_RAMPDOWN_STATE_UNION;
#endif
#define PMIC_BUCK00_RAMPDOWN_STATE_buck00_dly_rd_pd_off_cfg_START (0)
#define PMIC_BUCK00_RAMPDOWN_STATE_buck00_dly_rd_pd_off_cfg_END (2)
#define PMIC_BUCK00_RAMPDOWN_STATE_buck00_dly_rd_pd_on_cfg_START (3)
#define PMIC_BUCK00_RAMPDOWN_STATE_buck00_dly_rd_pd_on_cfg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_rampdown_chg_rate : 2;
        unsigned char buck00_rampdown_chg_vset : 2;
        unsigned char buck00_rampup_chg_rate : 2;
        unsigned char buck00_rampup_chg_vset : 2;
    } reg;
} PMIC_BUCK00_RAMP_CHG_CTRL_UNION;
#endif
#define PMIC_BUCK00_RAMP_CHG_CTRL_buck00_rampdown_chg_rate_START (0)
#define PMIC_BUCK00_RAMP_CHG_CTRL_buck00_rampdown_chg_rate_END (1)
#define PMIC_BUCK00_RAMP_CHG_CTRL_buck00_rampdown_chg_vset_START (2)
#define PMIC_BUCK00_RAMP_CHG_CTRL_buck00_rampdown_chg_vset_END (3)
#define PMIC_BUCK00_RAMP_CHG_CTRL_buck00_rampup_chg_rate_START (4)
#define PMIC_BUCK00_RAMP_CHG_CTRL_buck00_rampup_chg_rate_END (5)
#define PMIC_BUCK00_RAMP_CHG_CTRL_buck00_rampup_chg_vset_START (6)
#define PMIC_BUCK00_RAMP_CHG_CTRL_buck00_rampup_chg_vset_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_rampdown_ctrl : 3;
        unsigned char buck02_rampup_ctrl : 3;
        unsigned char buck02_ramp_en_cfg : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_RAMP_BUCK02_CTRL0_UNION;
#endif
#define PMIC_RAMP_BUCK02_CTRL0_buck02_rampdown_ctrl_START (0)
#define PMIC_RAMP_BUCK02_CTRL0_buck02_rampdown_ctrl_END (2)
#define PMIC_RAMP_BUCK02_CTRL0_buck02_rampup_ctrl_START (3)
#define PMIC_RAMP_BUCK02_CTRL0_buck02_rampup_ctrl_END (5)
#define PMIC_RAMP_BUCK02_CTRL0_buck02_ramp_en_cfg_START (6)
#define PMIC_RAMP_BUCK02_CTRL0_buck02_ramp_en_cfg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_pull_down_off_cfg : 3;
        unsigned char buck02_pull_down_on_cfg : 3;
        unsigned char buck02_pull_down_cfg : 1;
        unsigned char buck02_pull_down_mode : 1;
    } reg;
} PMIC_RAMP_BUCK02_CTRL1_UNION;
#endif
#define PMIC_RAMP_BUCK02_CTRL1_buck02_pull_down_off_cfg_START (0)
#define PMIC_RAMP_BUCK02_CTRL1_buck02_pull_down_off_cfg_END (2)
#define PMIC_RAMP_BUCK02_CTRL1_buck02_pull_down_on_cfg_START (3)
#define PMIC_RAMP_BUCK02_CTRL1_buck02_pull_down_on_cfg_END (5)
#define PMIC_RAMP_BUCK02_CTRL1_buck02_pull_down_cfg_START (6)
#define PMIC_RAMP_BUCK02_CTRL1_buck02_pull_down_cfg_END (6)
#define PMIC_RAMP_BUCK02_CTRL1_buck02_pull_down_mode_START (7)
#define PMIC_RAMP_BUCK02_CTRL1_buck02_pull_down_mode_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_dly_ru_off_cfg : 3;
        unsigned char buck02_dly_ru_on_cfg : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK02_RAMPUP_STATE_UNION;
#endif
#define PMIC_BUCK02_RAMPUP_STATE_buck02_dly_ru_off_cfg_START (0)
#define PMIC_BUCK02_RAMPUP_STATE_buck02_dly_ru_off_cfg_END (2)
#define PMIC_BUCK02_RAMPUP_STATE_buck02_dly_ru_on_cfg_START (3)
#define PMIC_BUCK02_RAMPUP_STATE_buck02_dly_ru_on_cfg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_dly_rd_pd_off_cfg : 3;
        unsigned char buck02_dly_rd_pd_on_cfg : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK02_RAMPDOWN_STATE_UNION;
#endif
#define PMIC_BUCK02_RAMPDOWN_STATE_buck02_dly_rd_pd_off_cfg_START (0)
#define PMIC_BUCK02_RAMPDOWN_STATE_buck02_dly_rd_pd_off_cfg_END (2)
#define PMIC_BUCK02_RAMPDOWN_STATE_buck02_dly_rd_pd_on_cfg_START (3)
#define PMIC_BUCK02_RAMPDOWN_STATE_buck02_dly_rd_pd_on_cfg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck02_rampdown_chg_rate : 2;
        unsigned char buck02_rampdown_chg_vset : 2;
        unsigned char buck02_rampup_chg_rate : 2;
        unsigned char buck02_rampup_chg_vset : 2;
    } reg;
} PMIC_BUCK02_RAMP_CHG_CTRL_UNION;
#endif
#define PMIC_BUCK02_RAMP_CHG_CTRL_buck02_rampdown_chg_rate_START (0)
#define PMIC_BUCK02_RAMP_CHG_CTRL_buck02_rampdown_chg_rate_END (1)
#define PMIC_BUCK02_RAMP_CHG_CTRL_buck02_rampdown_chg_vset_START (2)
#define PMIC_BUCK02_RAMP_CHG_CTRL_buck02_rampdown_chg_vset_END (3)
#define PMIC_BUCK02_RAMP_CHG_CTRL_buck02_rampup_chg_rate_START (4)
#define PMIC_BUCK02_RAMP_CHG_CTRL_buck02_rampup_chg_rate_END (5)
#define PMIC_BUCK02_RAMP_CHG_CTRL_buck02_rampup_chg_vset_START (6)
#define PMIC_BUCK02_RAMP_CHG_CTRL_buck02_rampup_chg_vset_END (7)
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
        unsigned char buck6_rampdown_ctrl : 3;
        unsigned char buck6_rampup_ctrl : 3;
        unsigned char buck6_ramp_en_cfg : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_RAMP_BUCK6_CTRL0_UNION;
#endif
#define PMIC_RAMP_BUCK6_CTRL0_buck6_rampdown_ctrl_START (0)
#define PMIC_RAMP_BUCK6_CTRL0_buck6_rampdown_ctrl_END (2)
#define PMIC_RAMP_BUCK6_CTRL0_buck6_rampup_ctrl_START (3)
#define PMIC_RAMP_BUCK6_CTRL0_buck6_rampup_ctrl_END (5)
#define PMIC_RAMP_BUCK6_CTRL0_buck6_ramp_en_cfg_START (6)
#define PMIC_RAMP_BUCK6_CTRL0_buck6_ramp_en_cfg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck6_pull_down_off_cfg : 3;
        unsigned char buck6_pull_down_on_cfg : 3;
        unsigned char buck6_pull_down_cfg : 1;
        unsigned char buck6_pull_down_mode : 1;
    } reg;
} PMIC_RAMP_BUCK6_CTRL1_UNION;
#endif
#define PMIC_RAMP_BUCK6_CTRL1_buck6_pull_down_off_cfg_START (0)
#define PMIC_RAMP_BUCK6_CTRL1_buck6_pull_down_off_cfg_END (2)
#define PMIC_RAMP_BUCK6_CTRL1_buck6_pull_down_on_cfg_START (3)
#define PMIC_RAMP_BUCK6_CTRL1_buck6_pull_down_on_cfg_END (5)
#define PMIC_RAMP_BUCK6_CTRL1_buck6_pull_down_cfg_START (6)
#define PMIC_RAMP_BUCK6_CTRL1_buck6_pull_down_cfg_END (6)
#define PMIC_RAMP_BUCK6_CTRL1_buck6_pull_down_mode_START (7)
#define PMIC_RAMP_BUCK6_CTRL1_buck6_pull_down_mode_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck6_dly_ru_off_cfg : 3;
        unsigned char buck6_dly_ru_on_cfg : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK6_RAMPUP_STATE_UNION;
#endif
#define PMIC_BUCK6_RAMPUP_STATE_buck6_dly_ru_off_cfg_START (0)
#define PMIC_BUCK6_RAMPUP_STATE_buck6_dly_ru_off_cfg_END (2)
#define PMIC_BUCK6_RAMPUP_STATE_buck6_dly_ru_on_cfg_START (3)
#define PMIC_BUCK6_RAMPUP_STATE_buck6_dly_ru_on_cfg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck6_dly_rd_pd_off_cfg : 3;
        unsigned char buck6_dly_rd_pd_on_cfg : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK6_RAMPDOWN_STATE_UNION;
#endif
#define PMIC_BUCK6_RAMPDOWN_STATE_buck6_dly_rd_pd_off_cfg_START (0)
#define PMIC_BUCK6_RAMPDOWN_STATE_buck6_dly_rd_pd_off_cfg_END (2)
#define PMIC_BUCK6_RAMPDOWN_STATE_buck6_dly_rd_pd_on_cfg_START (3)
#define PMIC_BUCK6_RAMPDOWN_STATE_buck6_dly_rd_pd_on_cfg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck6_rampdown_chg_rate : 2;
        unsigned char buck6_rampdown_chg_vset : 2;
        unsigned char buck6_rampup_chg_rate : 2;
        unsigned char buck6_rampup_chg_vset : 2;
    } reg;
} PMIC_BUCK6_RAMP_CHG_CTRL_UNION;
#endif
#define PMIC_BUCK6_RAMP_CHG_CTRL_buck6_rampdown_chg_rate_START (0)
#define PMIC_BUCK6_RAMP_CHG_CTRL_buck6_rampdown_chg_rate_END (1)
#define PMIC_BUCK6_RAMP_CHG_CTRL_buck6_rampdown_chg_vset_START (2)
#define PMIC_BUCK6_RAMP_CHG_CTRL_buck6_rampdown_chg_vset_END (3)
#define PMIC_BUCK6_RAMP_CHG_CTRL_buck6_rampup_chg_rate_START (4)
#define PMIC_BUCK6_RAMP_CHG_CTRL_buck6_rampup_chg_rate_END (5)
#define PMIC_BUCK6_RAMP_CHG_CTRL_buck6_rampup_chg_vset_START (6)
#define PMIC_BUCK6_RAMP_CHG_CTRL_buck6_rampup_chg_vset_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo0_rampdown_ctrl : 3;
        unsigned char ldo0_rampup_ctrl : 3;
        unsigned char ldo0_ramp_en_cfg : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_RAMP_LDO0_CTRL0_UNION;
#endif
#define PMIC_RAMP_LDO0_CTRL0_ldo0_rampdown_ctrl_START (0)
#define PMIC_RAMP_LDO0_CTRL0_ldo0_rampdown_ctrl_END (2)
#define PMIC_RAMP_LDO0_CTRL0_ldo0_rampup_ctrl_START (3)
#define PMIC_RAMP_LDO0_CTRL0_ldo0_rampup_ctrl_END (5)
#define PMIC_RAMP_LDO0_CTRL0_ldo0_ramp_en_cfg_START (6)
#define PMIC_RAMP_LDO0_CTRL0_ldo0_ramp_en_cfg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo0_pull_down_off_cfg : 3;
        unsigned char ldo0_pull_down_on_cfg : 3;
        unsigned char ldo0_pull_down_cfg : 1;
        unsigned char ldo0_pull_down_mode : 1;
    } reg;
} PMIC_RAMP_LDO0_CTRL1_UNION;
#endif
#define PMIC_RAMP_LDO0_CTRL1_ldo0_pull_down_off_cfg_START (0)
#define PMIC_RAMP_LDO0_CTRL1_ldo0_pull_down_off_cfg_END (2)
#define PMIC_RAMP_LDO0_CTRL1_ldo0_pull_down_on_cfg_START (3)
#define PMIC_RAMP_LDO0_CTRL1_ldo0_pull_down_on_cfg_END (5)
#define PMIC_RAMP_LDO0_CTRL1_ldo0_pull_down_cfg_START (6)
#define PMIC_RAMP_LDO0_CTRL1_ldo0_pull_down_cfg_END (6)
#define PMIC_RAMP_LDO0_CTRL1_ldo0_pull_down_mode_START (7)
#define PMIC_RAMP_LDO0_CTRL1_ldo0_pull_down_mode_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo39_rampdown_ctrl : 3;
        unsigned char ldo39_rampup_ctrl : 3;
        unsigned char ldo39_ramp_en_cfg : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_RAMP_LDO39_CTRL0_UNION;
#endif
#define PMIC_RAMP_LDO39_CTRL0_ldo39_rampdown_ctrl_START (0)
#define PMIC_RAMP_LDO39_CTRL0_ldo39_rampdown_ctrl_END (2)
#define PMIC_RAMP_LDO39_CTRL0_ldo39_rampup_ctrl_START (3)
#define PMIC_RAMP_LDO39_CTRL0_ldo39_rampup_ctrl_END (5)
#define PMIC_RAMP_LDO39_CTRL0_ldo39_ramp_en_cfg_START (6)
#define PMIC_RAMP_LDO39_CTRL0_ldo39_ramp_en_cfg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo39_pull_down_off_cfg : 3;
        unsigned char ldo39_pull_down_on_cfg : 3;
        unsigned char ldo39_pull_down_cfg : 1;
        unsigned char ldo39_pull_down_mode : 1;
    } reg;
} PMIC_RAMP_LDO39_CTRL1_UNION;
#endif
#define PMIC_RAMP_LDO39_CTRL1_ldo39_pull_down_off_cfg_START (0)
#define PMIC_RAMP_LDO39_CTRL1_ldo39_pull_down_off_cfg_END (2)
#define PMIC_RAMP_LDO39_CTRL1_ldo39_pull_down_on_cfg_START (3)
#define PMIC_RAMP_LDO39_CTRL1_ldo39_pull_down_on_cfg_END (5)
#define PMIC_RAMP_LDO39_CTRL1_ldo39_pull_down_cfg_START (6)
#define PMIC_RAMP_LDO39_CTRL1_ldo39_pull_down_cfg_END (6)
#define PMIC_RAMP_LDO39_CTRL1_ldo39_pull_down_mode_START (7)
#define PMIC_RAMP_LDO39_CTRL1_ldo39_pull_down_mode_END (7)
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
        unsigned char irq_mask_9_0 : 1;
        unsigned char irq_mask_9_1 : 1;
        unsigned char irq_mask_9_2 : 1;
        unsigned char irq_mask_9_3 : 1;
        unsigned char irq_mask_9_4 : 1;
        unsigned char irq_mask_9_reserve : 3;
    } reg;
} PMIC_IRQ_MASK_9_UNION;
#endif
#define PMIC_IRQ_MASK_9_irq_mask_9_0_START (0)
#define PMIC_IRQ_MASK_9_irq_mask_9_0_END (0)
#define PMIC_IRQ_MASK_9_irq_mask_9_1_START (1)
#define PMIC_IRQ_MASK_9_irq_mask_9_1_END (1)
#define PMIC_IRQ_MASK_9_irq_mask_9_2_START (2)
#define PMIC_IRQ_MASK_9_irq_mask_9_2_END (2)
#define PMIC_IRQ_MASK_9_irq_mask_9_3_START (3)
#define PMIC_IRQ_MASK_9_irq_mask_9_3_END (3)
#define PMIC_IRQ_MASK_9_irq_mask_9_4_START (4)
#define PMIC_IRQ_MASK_9_irq_mask_9_4_END (4)
#define PMIC_IRQ_MASK_9_irq_mask_9_reserve_START (5)
#define PMIC_IRQ_MASK_9_irq_mask_9_reserve_END (7)
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
        unsigned char irq_mask_gpio_reserve : 4;
    } reg;
} PMIC_IRQ_MASK_10_UNION;
#endif
#define PMIC_IRQ_MASK_10_irq_mask_gpio_0_START (0)
#define PMIC_IRQ_MASK_10_irq_mask_gpio_0_END (0)
#define PMIC_IRQ_MASK_10_irq_mask_gpio_1_START (1)
#define PMIC_IRQ_MASK_10_irq_mask_gpio_1_END (1)
#define PMIC_IRQ_MASK_10_irq_mask_gpio_2_START (2)
#define PMIC_IRQ_MASK_10_irq_mask_gpio_2_END (2)
#define PMIC_IRQ_MASK_10_irq_mask_gpio_3_START (3)
#define PMIC_IRQ_MASK_10_irq_mask_gpio_3_END (3)
#define PMIC_IRQ_MASK_10_irq_mask_gpio_reserve_START (4)
#define PMIC_IRQ_MASK_10_irq_mask_gpio_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_acr_soh_0 : 1;
        unsigned char irq_mask_acr_soh_1 : 1;
        unsigned char irq_mask_acr_soh_2 : 1;
        unsigned char irq_mask_acr_soh_3 : 1;
        unsigned char irq_mask_acr_soh_reserve : 4;
    } reg;
} PMIC_IRQ_MASK_11_UNION;
#endif
#define PMIC_IRQ_MASK_11_irq_mask_acr_soh_0_START (0)
#define PMIC_IRQ_MASK_11_irq_mask_acr_soh_0_END (0)
#define PMIC_IRQ_MASK_11_irq_mask_acr_soh_1_START (1)
#define PMIC_IRQ_MASK_11_irq_mask_acr_soh_1_END (1)
#define PMIC_IRQ_MASK_11_irq_mask_acr_soh_2_START (2)
#define PMIC_IRQ_MASK_11_irq_mask_acr_soh_2_END (2)
#define PMIC_IRQ_MASK_11_irq_mask_acr_soh_3_START (3)
#define PMIC_IRQ_MASK_11_irq_mask_acr_soh_3_END (3)
#define PMIC_IRQ_MASK_11_irq_mask_acr_soh_reserve_START (4)
#define PMIC_IRQ_MASK_11_irq_mask_acr_soh_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_lra_0 : 1;
        unsigned char irq_mask_lra_1 : 1;
        unsigned char irq_mask_lra_2 : 1;
        unsigned char irq_mask_lra_3 : 1;
        unsigned char irq_mask_lra_4 : 1;
        unsigned char irq_mask_lra_reserve : 3;
    } reg;
} PMIC_IRQ_MASK_12_UNION;
#endif
#define PMIC_IRQ_MASK_12_irq_mask_lra_0_START (0)
#define PMIC_IRQ_MASK_12_irq_mask_lra_0_END (0)
#define PMIC_IRQ_MASK_12_irq_mask_lra_1_START (1)
#define PMIC_IRQ_MASK_12_irq_mask_lra_1_END (1)
#define PMIC_IRQ_MASK_12_irq_mask_lra_2_START (2)
#define PMIC_IRQ_MASK_12_irq_mask_lra_2_END (2)
#define PMIC_IRQ_MASK_12_irq_mask_lra_3_START (3)
#define PMIC_IRQ_MASK_12_irq_mask_lra_3_END (3)
#define PMIC_IRQ_MASK_12_irq_mask_lra_4_START (4)
#define PMIC_IRQ_MASK_12_irq_mask_lra_4_END (4)
#define PMIC_IRQ_MASK_12_irq_mask_lra_reserve_START (5)
#define PMIC_IRQ_MASK_12_irq_mask_lra_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_vsys_drop : 1;
        unsigned char irq_mask_reserve : 7;
    } reg;
} PMIC_IRQ_MASK_13_UNION;
#endif
#define PMIC_IRQ_MASK_13_irq_mask_vsys_drop_START (0)
#define PMIC_IRQ_MASK_13_irq_mask_vsys_drop_END (0)
#define PMIC_IRQ_MASK_13_irq_mask_reserve_START (1)
#define PMIC_IRQ_MASK_13_irq_mask_reserve_END (7)
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
        unsigned char ocpbuck00 : 1;
        unsigned char ocpbuck01 : 1;
        unsigned char ocpbuck02 : 1;
        unsigned char ocpbuck1 : 1;
        unsigned char ocpbuck2 : 1;
        unsigned char ocpbuck3 : 1;
        unsigned char ocpbuck4 : 1;
        unsigned char ocpbuck5 : 1;
    } reg;
} PMIC_OCP_IRQ0_UNION;
#endif
#define PMIC_OCP_IRQ0_ocpbuck00_START (0)
#define PMIC_OCP_IRQ0_ocpbuck00_END (0)
#define PMIC_OCP_IRQ0_ocpbuck01_START (1)
#define PMIC_OCP_IRQ0_ocpbuck01_END (1)
#define PMIC_OCP_IRQ0_ocpbuck02_START (2)
#define PMIC_OCP_IRQ0_ocpbuck02_END (2)
#define PMIC_OCP_IRQ0_ocpbuck1_START (3)
#define PMIC_OCP_IRQ0_ocpbuck1_END (3)
#define PMIC_OCP_IRQ0_ocpbuck2_START (4)
#define PMIC_OCP_IRQ0_ocpbuck2_END (4)
#define PMIC_OCP_IRQ0_ocpbuck3_START (5)
#define PMIC_OCP_IRQ0_ocpbuck3_END (5)
#define PMIC_OCP_IRQ0_ocpbuck4_START (6)
#define PMIC_OCP_IRQ0_ocpbuck4_END (6)
#define PMIC_OCP_IRQ0_ocpbuck5_START (7)
#define PMIC_OCP_IRQ0_ocpbuck5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ocpldo3 : 1;
        unsigned char ocpldo2 : 1;
        unsigned char ocpldo1 : 1;
        unsigned char ocpldo0 : 1;
        unsigned char ocpbuck6 : 1;
        unsigned char ocpbuck7 : 1;
        unsigned char ocpbuck8 : 1;
        unsigned char ocpbuck9 : 1;
    } reg;
} PMIC_OCP_IRQ1_UNION;
#endif
#define PMIC_OCP_IRQ1_ocpldo3_START (0)
#define PMIC_OCP_IRQ1_ocpldo3_END (0)
#define PMIC_OCP_IRQ1_ocpldo2_START (1)
#define PMIC_OCP_IRQ1_ocpldo2_END (1)
#define PMIC_OCP_IRQ1_ocpldo1_START (2)
#define PMIC_OCP_IRQ1_ocpldo1_END (2)
#define PMIC_OCP_IRQ1_ocpldo0_START (3)
#define PMIC_OCP_IRQ1_ocpldo0_END (3)
#define PMIC_OCP_IRQ1_ocpbuck6_START (4)
#define PMIC_OCP_IRQ1_ocpbuck6_END (4)
#define PMIC_OCP_IRQ1_ocpbuck7_START (5)
#define PMIC_OCP_IRQ1_ocpbuck7_END (5)
#define PMIC_OCP_IRQ1_ocpbuck8_START (6)
#define PMIC_OCP_IRQ1_ocpbuck8_END (6)
#define PMIC_OCP_IRQ1_ocpbuck9_START (7)
#define PMIC_OCP_IRQ1_ocpbuck9_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ocpldo12 : 1;
        unsigned char ocpldo11 : 1;
        unsigned char ocpldo9 : 1;
        unsigned char ocpldo8 : 1;
        unsigned char ocpldo7 : 1;
        unsigned char ocpldo6 : 1;
        unsigned char ocpldo5 : 1;
        unsigned char ocpldo4 : 1;
    } reg;
} PMIC_OCP_IRQ2_UNION;
#endif
#define PMIC_OCP_IRQ2_ocpldo12_START (0)
#define PMIC_OCP_IRQ2_ocpldo12_END (0)
#define PMIC_OCP_IRQ2_ocpldo11_START (1)
#define PMIC_OCP_IRQ2_ocpldo11_END (1)
#define PMIC_OCP_IRQ2_ocpldo9_START (2)
#define PMIC_OCP_IRQ2_ocpldo9_END (2)
#define PMIC_OCP_IRQ2_ocpldo8_START (3)
#define PMIC_OCP_IRQ2_ocpldo8_END (3)
#define PMIC_OCP_IRQ2_ocpldo7_START (4)
#define PMIC_OCP_IRQ2_ocpldo7_END (4)
#define PMIC_OCP_IRQ2_ocpldo6_START (5)
#define PMIC_OCP_IRQ2_ocpldo6_END (5)
#define PMIC_OCP_IRQ2_ocpldo5_START (6)
#define PMIC_OCP_IRQ2_ocpldo5_END (6)
#define PMIC_OCP_IRQ2_ocpldo4_START (7)
#define PMIC_OCP_IRQ2_ocpldo4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ocpldo23 : 1;
        unsigned char ocpldo22 : 1;
        unsigned char ocpldo21 : 1;
        unsigned char ocpldo18 : 1;
        unsigned char ocpldo17 : 1;
        unsigned char ocpldo16 : 1;
        unsigned char ocpldo15 : 1;
        unsigned char ocpldo14 : 1;
    } reg;
} PMIC_OCP_IRQ3_UNION;
#endif
#define PMIC_OCP_IRQ3_ocpldo23_START (0)
#define PMIC_OCP_IRQ3_ocpldo23_END (0)
#define PMIC_OCP_IRQ3_ocpldo22_START (1)
#define PMIC_OCP_IRQ3_ocpldo22_END (1)
#define PMIC_OCP_IRQ3_ocpldo21_START (2)
#define PMIC_OCP_IRQ3_ocpldo21_END (2)
#define PMIC_OCP_IRQ3_ocpldo18_START (3)
#define PMIC_OCP_IRQ3_ocpldo18_END (3)
#define PMIC_OCP_IRQ3_ocpldo17_START (4)
#define PMIC_OCP_IRQ3_ocpldo17_END (4)
#define PMIC_OCP_IRQ3_ocpldo16_START (5)
#define PMIC_OCP_IRQ3_ocpldo16_END (5)
#define PMIC_OCP_IRQ3_ocpldo15_START (6)
#define PMIC_OCP_IRQ3_ocpldo15_END (6)
#define PMIC_OCP_IRQ3_ocpldo14_START (7)
#define PMIC_OCP_IRQ3_ocpldo14_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ocpldo32 : 1;
        unsigned char ocpldo30 : 1;
        unsigned char ocpldo29 : 1;
        unsigned char ocpldo28 : 1;
        unsigned char ocpldo27 : 1;
        unsigned char ocpldo26 : 1;
        unsigned char ocpldo25 : 1;
        unsigned char ocpldo24 : 1;
    } reg;
} PMIC_OCP_IRQ4_UNION;
#endif
#define PMIC_OCP_IRQ4_ocpldo32_START (0)
#define PMIC_OCP_IRQ4_ocpldo32_END (0)
#define PMIC_OCP_IRQ4_ocpldo30_START (1)
#define PMIC_OCP_IRQ4_ocpldo30_END (1)
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
        unsigned char ocpldo41 : 1;
        unsigned char ocpldo40 : 1;
        unsigned char ocpldo39 : 1;
        unsigned char ocpldo38 : 1;
        unsigned char ocpldo37 : 1;
        unsigned char ocpldo36 : 1;
        unsigned char pmuh_ocp : 1;
        unsigned char ocpldo34 : 1;
    } reg;
} PMIC_OCP_IRQ5_UNION;
#endif
#define PMIC_OCP_IRQ5_ocpldo41_START (0)
#define PMIC_OCP_IRQ5_ocpldo41_END (0)
#define PMIC_OCP_IRQ5_ocpldo40_START (1)
#define PMIC_OCP_IRQ5_ocpldo40_END (1)
#define PMIC_OCP_IRQ5_ocpldo39_START (2)
#define PMIC_OCP_IRQ5_ocpldo39_END (2)
#define PMIC_OCP_IRQ5_ocpldo38_START (3)
#define PMIC_OCP_IRQ5_ocpldo38_END (3)
#define PMIC_OCP_IRQ5_ocpldo37_START (4)
#define PMIC_OCP_IRQ5_ocpldo37_END (4)
#define PMIC_OCP_IRQ5_ocpldo36_START (5)
#define PMIC_OCP_IRQ5_ocpldo36_END (5)
#define PMIC_OCP_IRQ5_pmuh_ocp_START (6)
#define PMIC_OCP_IRQ5_pmuh_ocp_END (6)
#define PMIC_OCP_IRQ5_ocpldo34_START (7)
#define PMIC_OCP_IRQ5_ocpldo34_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck00_scp : 1;
        unsigned char buck02_scp : 1;
        unsigned char buck1_scp : 1;
        unsigned char buck2_scp : 1;
        unsigned char buck3_scp : 1;
        unsigned char buck4_scp : 1;
        unsigned char buck5_scp : 1;
        unsigned char buck6_scp : 1;
    } reg;
} PMIC_SCP_IRQ0_UNION;
#endif
#define PMIC_SCP_IRQ0_buck00_scp_START (0)
#define PMIC_SCP_IRQ0_buck00_scp_END (0)
#define PMIC_SCP_IRQ0_buck02_scp_START (1)
#define PMIC_SCP_IRQ0_buck02_scp_END (1)
#define PMIC_SCP_IRQ0_buck1_scp_START (2)
#define PMIC_SCP_IRQ0_buck1_scp_END (2)
#define PMIC_SCP_IRQ0_buck2_scp_START (3)
#define PMIC_SCP_IRQ0_buck2_scp_END (3)
#define PMIC_SCP_IRQ0_buck3_scp_START (4)
#define PMIC_SCP_IRQ0_buck3_scp_END (4)
#define PMIC_SCP_IRQ0_buck4_scp_START (5)
#define PMIC_SCP_IRQ0_buck4_scp_END (5)
#define PMIC_SCP_IRQ0_buck5_scp_START (6)
#define PMIC_SCP_IRQ0_buck5_scp_END (6)
#define PMIC_SCP_IRQ0_buck6_scp_START (7)
#define PMIC_SCP_IRQ0_buck6_scp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_scp : 1;
        unsigned char buck8_scp : 1;
        unsigned char buck9_scp : 1;
        unsigned char ldo_buff_scp : 1;
        unsigned char acr_ocp : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_SCP_IRQ1_UNION;
#endif
#define PMIC_SCP_IRQ1_buck7_scp_START (0)
#define PMIC_SCP_IRQ1_buck7_scp_END (0)
#define PMIC_SCP_IRQ1_buck8_scp_START (1)
#define PMIC_SCP_IRQ1_buck8_scp_END (1)
#define PMIC_SCP_IRQ1_buck9_scp_START (2)
#define PMIC_SCP_IRQ1_buck9_scp_END (2)
#define PMIC_SCP_IRQ1_ldo_buff_scp_START (3)
#define PMIC_SCP_IRQ1_ldo_buff_scp_END (3)
#define PMIC_SCP_IRQ1_acr_ocp_START (4)
#define PMIC_SCP_IRQ1_acr_ocp_END (4)
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
        unsigned char soh_ovp_dis : 1;
        unsigned char soh_ovp : 1;
        unsigned char dcr_flag : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_ACR_SOH_IRQ_UNION;
#endif
#define PMIC_ACR_SOH_IRQ_acr_flag_START (0)
#define PMIC_ACR_SOH_IRQ_acr_flag_END (0)
#define PMIC_ACR_SOH_IRQ_soh_ovp_dis_START (1)
#define PMIC_ACR_SOH_IRQ_soh_ovp_dis_END (1)
#define PMIC_ACR_SOH_IRQ_soh_ovp_START (2)
#define PMIC_ACR_SOH_IRQ_soh_ovp_END (2)
#define PMIC_ACR_SOH_IRQ_dcr_flag_START (3)
#define PMIC_ACR_SOH_IRQ_dcr_flag_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lra_ocp : 1;
        unsigned char lra_init_abn : 1;
        unsigned char lra_vsys_uvp : 1;
        unsigned char lra_vsys_ovp : 1;
        unsigned char lra_adc_error : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LRA_IRQ_UNION;
#endif
#define PMIC_LRA_IRQ_lra_ocp_START (0)
#define PMIC_LRA_IRQ_lra_ocp_END (0)
#define PMIC_LRA_IRQ_lra_init_abn_START (1)
#define PMIC_LRA_IRQ_lra_init_abn_END (1)
#define PMIC_LRA_IRQ_lra_vsys_uvp_START (2)
#define PMIC_LRA_IRQ_lra_vsys_uvp_END (2)
#define PMIC_LRA_IRQ_lra_vsys_ovp_START (3)
#define PMIC_LRA_IRQ_lra_vsys_ovp_END (3)
#define PMIC_LRA_IRQ_lra_adc_error_START (4)
#define PMIC_LRA_IRQ_lra_adc_error_END (4)
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
        unsigned char np_ocpbuck00 : 1;
        unsigned char np_ocpbuck01 : 1;
        unsigned char np_ocpbuck02 : 1;
        unsigned char np_ocpbuck1 : 1;
        unsigned char np_ocpbuck2 : 1;
        unsigned char np_ocpbuck3 : 1;
        unsigned char np_ocpbuck4 : 1;
        unsigned char np_ocpbuck5 : 1;
    } reg;
} PMIC_NP_OCP0_UNION;
#endif
#define PMIC_NP_OCP0_np_ocpbuck00_START (0)
#define PMIC_NP_OCP0_np_ocpbuck00_END (0)
#define PMIC_NP_OCP0_np_ocpbuck01_START (1)
#define PMIC_NP_OCP0_np_ocpbuck01_END (1)
#define PMIC_NP_OCP0_np_ocpbuck02_START (2)
#define PMIC_NP_OCP0_np_ocpbuck02_END (2)
#define PMIC_NP_OCP0_np_ocpbuck1_START (3)
#define PMIC_NP_OCP0_np_ocpbuck1_END (3)
#define PMIC_NP_OCP0_np_ocpbuck2_START (4)
#define PMIC_NP_OCP0_np_ocpbuck2_END (4)
#define PMIC_NP_OCP0_np_ocpbuck3_START (5)
#define PMIC_NP_OCP0_np_ocpbuck3_END (5)
#define PMIC_NP_OCP0_np_ocpbuck4_START (6)
#define PMIC_NP_OCP0_np_ocpbuck4_END (6)
#define PMIC_NP_OCP0_np_ocpbuck5_START (7)
#define PMIC_NP_OCP0_np_ocpbuck5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ocpldo3 : 1;
        unsigned char np_ocpldo2 : 1;
        unsigned char np_ocpldo1 : 1;
        unsigned char np_ocpldo0 : 1;
        unsigned char np_ocpbuck6 : 1;
        unsigned char np_ocpbuck7 : 1;
        unsigned char np_ocpbuck8 : 1;
        unsigned char np_ocpbuck9 : 1;
    } reg;
} PMIC_NP_OCP1_UNION;
#endif
#define PMIC_NP_OCP1_np_ocpldo3_START (0)
#define PMIC_NP_OCP1_np_ocpldo3_END (0)
#define PMIC_NP_OCP1_np_ocpldo2_START (1)
#define PMIC_NP_OCP1_np_ocpldo2_END (1)
#define PMIC_NP_OCP1_np_ocpldo1_START (2)
#define PMIC_NP_OCP1_np_ocpldo1_END (2)
#define PMIC_NP_OCP1_np_ocpldo0_START (3)
#define PMIC_NP_OCP1_np_ocpldo0_END (3)
#define PMIC_NP_OCP1_np_ocpbuck6_START (4)
#define PMIC_NP_OCP1_np_ocpbuck6_END (4)
#define PMIC_NP_OCP1_np_ocpbuck7_START (5)
#define PMIC_NP_OCP1_np_ocpbuck7_END (5)
#define PMIC_NP_OCP1_np_ocpbuck8_START (6)
#define PMIC_NP_OCP1_np_ocpbuck8_END (6)
#define PMIC_NP_OCP1_np_ocpbuck9_START (7)
#define PMIC_NP_OCP1_np_ocpbuck9_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ocpldo12 : 1;
        unsigned char np_ocpldo11 : 1;
        unsigned char np_ocpldo9 : 1;
        unsigned char np_ocpldo8 : 1;
        unsigned char np_ocpldo7 : 1;
        unsigned char np_ocpldo6 : 1;
        unsigned char np_ocpldo5 : 1;
        unsigned char np_ocpldo4 : 1;
    } reg;
} PMIC_NP_OCP2_UNION;
#endif
#define PMIC_NP_OCP2_np_ocpldo12_START (0)
#define PMIC_NP_OCP2_np_ocpldo12_END (0)
#define PMIC_NP_OCP2_np_ocpldo11_START (1)
#define PMIC_NP_OCP2_np_ocpldo11_END (1)
#define PMIC_NP_OCP2_np_ocpldo9_START (2)
#define PMIC_NP_OCP2_np_ocpldo9_END (2)
#define PMIC_NP_OCP2_np_ocpldo8_START (3)
#define PMIC_NP_OCP2_np_ocpldo8_END (3)
#define PMIC_NP_OCP2_np_ocpldo7_START (4)
#define PMIC_NP_OCP2_np_ocpldo7_END (4)
#define PMIC_NP_OCP2_np_ocpldo6_START (5)
#define PMIC_NP_OCP2_np_ocpldo6_END (5)
#define PMIC_NP_OCP2_np_ocpldo5_START (6)
#define PMIC_NP_OCP2_np_ocpldo5_END (6)
#define PMIC_NP_OCP2_np_ocpldo4_START (7)
#define PMIC_NP_OCP2_np_ocpldo4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ocpldo23 : 1;
        unsigned char np_ocpldo22 : 1;
        unsigned char np_ocpldo21 : 1;
        unsigned char np_ocpldo18 : 1;
        unsigned char np_ocpldo17 : 1;
        unsigned char np_ocpldo16 : 1;
        unsigned char np_ocpldo15 : 1;
        unsigned char np_ocpldo14 : 1;
    } reg;
} PMIC_NP_OCP3_UNION;
#endif
#define PMIC_NP_OCP3_np_ocpldo23_START (0)
#define PMIC_NP_OCP3_np_ocpldo23_END (0)
#define PMIC_NP_OCP3_np_ocpldo22_START (1)
#define PMIC_NP_OCP3_np_ocpldo22_END (1)
#define PMIC_NP_OCP3_np_ocpldo21_START (2)
#define PMIC_NP_OCP3_np_ocpldo21_END (2)
#define PMIC_NP_OCP3_np_ocpldo18_START (3)
#define PMIC_NP_OCP3_np_ocpldo18_END (3)
#define PMIC_NP_OCP3_np_ocpldo17_START (4)
#define PMIC_NP_OCP3_np_ocpldo17_END (4)
#define PMIC_NP_OCP3_np_ocpldo16_START (5)
#define PMIC_NP_OCP3_np_ocpldo16_END (5)
#define PMIC_NP_OCP3_np_ocpldo15_START (6)
#define PMIC_NP_OCP3_np_ocpldo15_END (6)
#define PMIC_NP_OCP3_np_ocpldo14_START (7)
#define PMIC_NP_OCP3_np_ocpldo14_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ocpldo32 : 1;
        unsigned char np_ocpldo30 : 1;
        unsigned char np_ocpldo29 : 1;
        unsigned char np_ocpldo28 : 1;
        unsigned char np_ocpldo27 : 1;
        unsigned char np_ocpldo26 : 1;
        unsigned char np_ocpldo25 : 1;
        unsigned char np_ocpldo24 : 1;
    } reg;
} PMIC_NP_OCP4_UNION;
#endif
#define PMIC_NP_OCP4_np_ocpldo32_START (0)
#define PMIC_NP_OCP4_np_ocpldo32_END (0)
#define PMIC_NP_OCP4_np_ocpldo30_START (1)
#define PMIC_NP_OCP4_np_ocpldo30_END (1)
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
        unsigned char np_ocpldo41 : 1;
        unsigned char np_ocpldo40 : 1;
        unsigned char np_ocpldo39 : 1;
        unsigned char np_ocpldo38 : 1;
        unsigned char np_ocpldo37 : 1;
        unsigned char np_ocpldo36 : 1;
        unsigned char np_pmuh_ocp : 1;
        unsigned char np_ocpldo34 : 1;
    } reg;
} PMIC_NP_OCP5_UNION;
#endif
#define PMIC_NP_OCP5_np_ocpldo41_START (0)
#define PMIC_NP_OCP5_np_ocpldo41_END (0)
#define PMIC_NP_OCP5_np_ocpldo40_START (1)
#define PMIC_NP_OCP5_np_ocpldo40_END (1)
#define PMIC_NP_OCP5_np_ocpldo39_START (2)
#define PMIC_NP_OCP5_np_ocpldo39_END (2)
#define PMIC_NP_OCP5_np_ocpldo38_START (3)
#define PMIC_NP_OCP5_np_ocpldo38_END (3)
#define PMIC_NP_OCP5_np_ocpldo37_START (4)
#define PMIC_NP_OCP5_np_ocpldo37_END (4)
#define PMIC_NP_OCP5_np_ocpldo36_START (5)
#define PMIC_NP_OCP5_np_ocpldo36_END (5)
#define PMIC_NP_OCP5_np_pmuh_ocp_START (6)
#define PMIC_NP_OCP5_np_pmuh_ocp_END (6)
#define PMIC_NP_OCP5_np_ocpldo34_START (7)
#define PMIC_NP_OCP5_np_ocpldo34_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_buck00_scp : 1;
        unsigned char np_buck02_scp : 1;
        unsigned char np_buck1_scp : 1;
        unsigned char np_buck2_scp : 1;
        unsigned char np_buck3_scp : 1;
        unsigned char np_buck4_scp : 1;
        unsigned char np_buck5_scp : 1;
        unsigned char np_buck6_scp : 1;
    } reg;
} PMIC_NP_SCP0_UNION;
#endif
#define PMIC_NP_SCP0_np_buck00_scp_START (0)
#define PMIC_NP_SCP0_np_buck00_scp_END (0)
#define PMIC_NP_SCP0_np_buck02_scp_START (1)
#define PMIC_NP_SCP0_np_buck02_scp_END (1)
#define PMIC_NP_SCP0_np_buck1_scp_START (2)
#define PMIC_NP_SCP0_np_buck1_scp_END (2)
#define PMIC_NP_SCP0_np_buck2_scp_START (3)
#define PMIC_NP_SCP0_np_buck2_scp_END (3)
#define PMIC_NP_SCP0_np_buck3_scp_START (4)
#define PMIC_NP_SCP0_np_buck3_scp_END (4)
#define PMIC_NP_SCP0_np_buck4_scp_START (5)
#define PMIC_NP_SCP0_np_buck4_scp_END (5)
#define PMIC_NP_SCP0_np_buck5_scp_START (6)
#define PMIC_NP_SCP0_np_buck5_scp_END (6)
#define PMIC_NP_SCP0_np_buck6_scp_START (7)
#define PMIC_NP_SCP0_np_buck6_scp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_buck7_scp : 1;
        unsigned char np_buck8_scp : 1;
        unsigned char np_buck9_scp : 1;
        unsigned char np_ldo_buff_scp : 1;
        unsigned char np_acr_ocp : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_NP_SCP1_UNION;
#endif
#define PMIC_NP_SCP1_np_buck7_scp_START (0)
#define PMIC_NP_SCP1_np_buck7_scp_END (0)
#define PMIC_NP_SCP1_np_buck8_scp_START (1)
#define PMIC_NP_SCP1_np_buck8_scp_END (1)
#define PMIC_NP_SCP1_np_buck9_scp_START (2)
#define PMIC_NP_SCP1_np_buck9_scp_END (2)
#define PMIC_NP_SCP1_np_ldo_buff_scp_START (3)
#define PMIC_NP_SCP1_np_ldo_buff_scp_END (3)
#define PMIC_NP_SCP1_np_acr_ocp_START (4)
#define PMIC_NP_SCP1_np_acr_ocp_END (4)
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
        unsigned char np_buck00_ramp_abnor : 1;
        unsigned char np_buck02_ramp_abnor : 1;
        unsigned char np_buck4_ramp_abnor : 1;
        unsigned char np_buck5_ramp_abnor : 1;
        unsigned char np_buck6_ramp_abnor : 1;
        unsigned char np_ldo0_ramp_abnor : 1;
        unsigned char np_ldo39_ramp_abnor : 1;
        unsigned char np_ldo40_ramp_abnor : 1;
    } reg;
} PMIC_RAMP_EVENT_UNION;
#endif
#define PMIC_RAMP_EVENT_np_buck00_ramp_abnor_START (0)
#define PMIC_RAMP_EVENT_np_buck00_ramp_abnor_END (0)
#define PMIC_RAMP_EVENT_np_buck02_ramp_abnor_START (1)
#define PMIC_RAMP_EVENT_np_buck02_ramp_abnor_END (1)
#define PMIC_RAMP_EVENT_np_buck4_ramp_abnor_START (2)
#define PMIC_RAMP_EVENT_np_buck4_ramp_abnor_END (2)
#define PMIC_RAMP_EVENT_np_buck5_ramp_abnor_START (3)
#define PMIC_RAMP_EVENT_np_buck5_ramp_abnor_END (3)
#define PMIC_RAMP_EVENT_np_buck6_ramp_abnor_START (4)
#define PMIC_RAMP_EVENT_np_buck6_ramp_abnor_END (4)
#define PMIC_RAMP_EVENT_np_ldo0_ramp_abnor_START (5)
#define PMIC_RAMP_EVENT_np_ldo0_ramp_abnor_END (5)
#define PMIC_RAMP_EVENT_np_ldo39_ramp_abnor_START (6)
#define PMIC_RAMP_EVENT_np_ldo39_ramp_abnor_END (6)
#define PMIC_RAMP_EVENT_np_ldo40_ramp_abnor_START (7)
#define PMIC_RAMP_EVENT_np_ldo40_ramp_abnor_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_tmp_ovh_0 : 1;
        unsigned char np_soh_ovh_0 : 1;
        unsigned char np_tmp_ovh_1 : 1;
        unsigned char np_soh_ovh_1 : 1;
        unsigned char np_tmp_ovh_2 : 1;
        unsigned char np_soh_ovh_2 : 1;
        unsigned char np_soh_ovh : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_SOH_RECORD_UNION;
#endif
#define PMIC_SOH_RECORD_np_tmp_ovh_0_START (0)
#define PMIC_SOH_RECORD_np_tmp_ovh_0_END (0)
#define PMIC_SOH_RECORD_np_soh_ovh_0_START (1)
#define PMIC_SOH_RECORD_np_soh_ovh_0_END (1)
#define PMIC_SOH_RECORD_np_tmp_ovh_1_START (2)
#define PMIC_SOH_RECORD_np_tmp_ovh_1_END (2)
#define PMIC_SOH_RECORD_np_soh_ovh_1_START (3)
#define PMIC_SOH_RECORD_np_soh_ovh_1_END (3)
#define PMIC_SOH_RECORD_np_tmp_ovh_2_START (4)
#define PMIC_SOH_RECORD_np_tmp_ovh_2_END (4)
#define PMIC_SOH_RECORD_np_soh_ovh_2_START (5)
#define PMIC_SOH_RECORD_np_soh_ovh_2_END (5)
#define PMIC_SOH_RECORD_np_soh_ovh_START (6)
#define PMIC_SOH_RECORD_np_soh_ovh_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_acr_flag : 1;
        unsigned char np_tmp_ovl : 1;
        unsigned char np_soh_ovl : 1;
        unsigned char np_dcr_flag : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_ACR_SOH_RECORD_UNION;
#endif
#define PMIC_ACR_SOH_RECORD_np_acr_flag_START (0)
#define PMIC_ACR_SOH_RECORD_np_acr_flag_END (0)
#define PMIC_ACR_SOH_RECORD_np_tmp_ovl_START (1)
#define PMIC_ACR_SOH_RECORD_np_tmp_ovl_END (1)
#define PMIC_ACR_SOH_RECORD_np_soh_ovl_START (2)
#define PMIC_ACR_SOH_RECORD_np_soh_ovl_END (2)
#define PMIC_ACR_SOH_RECORD_np_dcr_flag_START (3)
#define PMIC_ACR_SOH_RECORD_np_dcr_flag_END (3)
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
        unsigned char np_xo_dig_rf0_sel : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_CLK_RF0_CTRL1_UNION;
#endif
#define PMIC_CLK_RF0_CTRL1_np_xo_dig_rf0_sel_START (0)
#define PMIC_CLK_RF0_CTRL1_np_xo_dig_rf0_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_xo_dig_rf1_sel : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_CLK_RF1_CTRL1_UNION;
#endif
#define PMIC_CLK_RF1_CTRL1_np_xo_dig_rf1_sel_START (0)
#define PMIC_CLK_RF1_CTRL1_np_xo_dig_rf1_sel_END (0)
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
        unsigned char np_buck0_config : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_NP_CTRL_UNION;
#endif
#define PMIC_NP_CTRL_np_rtc_cali_ctrl_START (0)
#define PMIC_NP_CTRL_np_rtc_cali_ctrl_END (0)
#define PMIC_NP_CTRL_np_hreset_d_sel_START (1)
#define PMIC_NP_CTRL_np_hreset_d_sel_END (1)
#define PMIC_NP_CTRL_np_buck0_config_START (2)
#define PMIC_NP_CTRL_np_buck0_config_END (2)
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
        unsigned char np_otp0_pdob0_d2a : 8;
    } reg;
} PMIC_OTP0_0_R_UNION;
#endif
#define PMIC_OTP0_0_R_np_otp0_pdob0_d2a_START (0)
#define PMIC_OTP0_0_R_np_otp0_pdob0_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob1_d2a : 8;
    } reg;
} PMIC_OTP0_1_R_UNION;
#endif
#define PMIC_OTP0_1_R_np_otp0_pdob1_d2a_START (0)
#define PMIC_OTP0_1_R_np_otp0_pdob1_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob2_d2a : 8;
    } reg;
} PMIC_OTP0_2_R_UNION;
#endif
#define PMIC_OTP0_2_R_np_otp0_pdob2_d2a_START (0)
#define PMIC_OTP0_2_R_np_otp0_pdob2_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob3_d2a : 8;
    } reg;
} PMIC_OTP0_3_R_UNION;
#endif
#define PMIC_OTP0_3_R_np_otp0_pdob3_d2a_START (0)
#define PMIC_OTP0_3_R_np_otp0_pdob3_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob4_d2a : 8;
    } reg;
} PMIC_OTP0_4_R_UNION;
#endif
#define PMIC_OTP0_4_R_np_otp0_pdob4_d2a_START (0)
#define PMIC_OTP0_4_R_np_otp0_pdob4_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob5_d2a : 8;
    } reg;
} PMIC_OTP0_5_R_UNION;
#endif
#define PMIC_OTP0_5_R_np_otp0_pdob5_d2a_START (0)
#define PMIC_OTP0_5_R_np_otp0_pdob5_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob6_d2a : 8;
    } reg;
} PMIC_OTP0_6_R_UNION;
#endif
#define PMIC_OTP0_6_R_np_otp0_pdob6_d2a_START (0)
#define PMIC_OTP0_6_R_np_otp0_pdob6_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob7_d2a : 8;
    } reg;
} PMIC_OTP0_7_R_UNION;
#endif
#define PMIC_OTP0_7_R_np_otp0_pdob7_d2a_START (0)
#define PMIC_OTP0_7_R_np_otp0_pdob7_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob8_d2a : 8;
    } reg;
} PMIC_OTP0_8_R_UNION;
#endif
#define PMIC_OTP0_8_R_np_otp0_pdob8_d2a_START (0)
#define PMIC_OTP0_8_R_np_otp0_pdob8_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob9_d2a : 8;
    } reg;
} PMIC_OTP0_9_R_UNION;
#endif
#define PMIC_OTP0_9_R_np_otp0_pdob9_d2a_START (0)
#define PMIC_OTP0_9_R_np_otp0_pdob9_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob10_d2a : 8;
    } reg;
} PMIC_OTP0_10_R_UNION;
#endif
#define PMIC_OTP0_10_R_np_otp0_pdob10_d2a_START (0)
#define PMIC_OTP0_10_R_np_otp0_pdob10_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob11_d2a : 8;
    } reg;
} PMIC_OTP0_11_R_UNION;
#endif
#define PMIC_OTP0_11_R_np_otp0_pdob11_d2a_START (0)
#define PMIC_OTP0_11_R_np_otp0_pdob11_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob12_d2a : 8;
    } reg;
} PMIC_OTP0_12_R_UNION;
#endif
#define PMIC_OTP0_12_R_np_otp0_pdob12_d2a_START (0)
#define PMIC_OTP0_12_R_np_otp0_pdob12_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob13_d2a : 8;
    } reg;
} PMIC_OTP0_13_R_UNION;
#endif
#define PMIC_OTP0_13_R_np_otp0_pdob13_d2a_START (0)
#define PMIC_OTP0_13_R_np_otp0_pdob13_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob14_d2a : 8;
    } reg;
} PMIC_OTP0_14_R_UNION;
#endif
#define PMIC_OTP0_14_R_np_otp0_pdob14_d2a_START (0)
#define PMIC_OTP0_14_R_np_otp0_pdob14_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob15_d2a : 8;
    } reg;
} PMIC_OTP0_15_R_UNION;
#endif
#define PMIC_OTP0_15_R_np_otp0_pdob15_d2a_START (0)
#define PMIC_OTP0_15_R_np_otp0_pdob15_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob16_d2a : 8;
    } reg;
} PMIC_OTP0_16_R_UNION;
#endif
#define PMIC_OTP0_16_R_np_otp0_pdob16_d2a_START (0)
#define PMIC_OTP0_16_R_np_otp0_pdob16_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob17_d2a : 8;
    } reg;
} PMIC_OTP0_17_R_UNION;
#endif
#define PMIC_OTP0_17_R_np_otp0_pdob17_d2a_START (0)
#define PMIC_OTP0_17_R_np_otp0_pdob17_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob18_d2a : 8;
    } reg;
} PMIC_OTP0_18_R_UNION;
#endif
#define PMIC_OTP0_18_R_np_otp0_pdob18_d2a_START (0)
#define PMIC_OTP0_18_R_np_otp0_pdob18_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob19_d2a : 8;
    } reg;
} PMIC_OTP0_19_R_UNION;
#endif
#define PMIC_OTP0_19_R_np_otp0_pdob19_d2a_START (0)
#define PMIC_OTP0_19_R_np_otp0_pdob19_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob20_d2a : 8;
    } reg;
} PMIC_OTP0_20_R_UNION;
#endif
#define PMIC_OTP0_20_R_np_otp0_pdob20_d2a_START (0)
#define PMIC_OTP0_20_R_np_otp0_pdob20_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob21_d2a : 8;
    } reg;
} PMIC_OTP0_21_R_UNION;
#endif
#define PMIC_OTP0_21_R_np_otp0_pdob21_d2a_START (0)
#define PMIC_OTP0_21_R_np_otp0_pdob21_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob22_d2a : 8;
    } reg;
} PMIC_OTP0_22_R_UNION;
#endif
#define PMIC_OTP0_22_R_np_otp0_pdob22_d2a_START (0)
#define PMIC_OTP0_22_R_np_otp0_pdob22_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob23_d2a : 8;
    } reg;
} PMIC_OTP0_23_R_UNION;
#endif
#define PMIC_OTP0_23_R_np_otp0_pdob23_d2a_START (0)
#define PMIC_OTP0_23_R_np_otp0_pdob23_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob24_d2a : 8;
    } reg;
} PMIC_OTP0_24_R_UNION;
#endif
#define PMIC_OTP0_24_R_np_otp0_pdob24_d2a_START (0)
#define PMIC_OTP0_24_R_np_otp0_pdob24_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob25_d2a : 8;
    } reg;
} PMIC_OTP0_25_R_UNION;
#endif
#define PMIC_OTP0_25_R_np_otp0_pdob25_d2a_START (0)
#define PMIC_OTP0_25_R_np_otp0_pdob25_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob26_d2a : 8;
    } reg;
} PMIC_OTP0_26_R_UNION;
#endif
#define PMIC_OTP0_26_R_np_otp0_pdob26_d2a_START (0)
#define PMIC_OTP0_26_R_np_otp0_pdob26_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob27_d2a : 8;
    } reg;
} PMIC_OTP0_27_R_UNION;
#endif
#define PMIC_OTP0_27_R_np_otp0_pdob27_d2a_START (0)
#define PMIC_OTP0_27_R_np_otp0_pdob27_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob28_d2a : 8;
    } reg;
} PMIC_OTP0_28_R_UNION;
#endif
#define PMIC_OTP0_28_R_np_otp0_pdob28_d2a_START (0)
#define PMIC_OTP0_28_R_np_otp0_pdob28_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob29_d2a : 8;
    } reg;
} PMIC_OTP0_29_R_UNION;
#endif
#define PMIC_OTP0_29_R_np_otp0_pdob29_d2a_START (0)
#define PMIC_OTP0_29_R_np_otp0_pdob29_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob30_d2a : 8;
    } reg;
} PMIC_OTP0_30_R_UNION;
#endif
#define PMIC_OTP0_30_R_np_otp0_pdob30_d2a_START (0)
#define PMIC_OTP0_30_R_np_otp0_pdob30_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob31_d2a : 8;
    } reg;
} PMIC_OTP0_31_R_UNION;
#endif
#define PMIC_OTP0_31_R_np_otp0_pdob31_d2a_START (0)
#define PMIC_OTP0_31_R_np_otp0_pdob31_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob32_d2a : 8;
    } reg;
} PMIC_OTP0_32_R_UNION;
#endif
#define PMIC_OTP0_32_R_np_otp0_pdob32_d2a_START (0)
#define PMIC_OTP0_32_R_np_otp0_pdob32_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob33_d2a : 8;
    } reg;
} PMIC_OTP0_33_R_UNION;
#endif
#define PMIC_OTP0_33_R_np_otp0_pdob33_d2a_START (0)
#define PMIC_OTP0_33_R_np_otp0_pdob33_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob34_d2a : 8;
    } reg;
} PMIC_OTP0_34_R_UNION;
#endif
#define PMIC_OTP0_34_R_np_otp0_pdob34_d2a_START (0)
#define PMIC_OTP0_34_R_np_otp0_pdob34_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob35_d2a : 8;
    } reg;
} PMIC_OTP0_35_R_UNION;
#endif
#define PMIC_OTP0_35_R_np_otp0_pdob35_d2a_START (0)
#define PMIC_OTP0_35_R_np_otp0_pdob35_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob36_d2a : 8;
    } reg;
} PMIC_OTP0_36_R_UNION;
#endif
#define PMIC_OTP0_36_R_np_otp0_pdob36_d2a_START (0)
#define PMIC_OTP0_36_R_np_otp0_pdob36_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob37_d2a : 8;
    } reg;
} PMIC_OTP0_37_R_UNION;
#endif
#define PMIC_OTP0_37_R_np_otp0_pdob37_d2a_START (0)
#define PMIC_OTP0_37_R_np_otp0_pdob37_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob38_d2a : 8;
    } reg;
} PMIC_OTP0_38_R_UNION;
#endif
#define PMIC_OTP0_38_R_np_otp0_pdob38_d2a_START (0)
#define PMIC_OTP0_38_R_np_otp0_pdob38_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob39_d2a : 8;
    } reg;
} PMIC_OTP0_39_R_UNION;
#endif
#define PMIC_OTP0_39_R_np_otp0_pdob39_d2a_START (0)
#define PMIC_OTP0_39_R_np_otp0_pdob39_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob40_d2a : 8;
    } reg;
} PMIC_OTP0_40_R_UNION;
#endif
#define PMIC_OTP0_40_R_np_otp0_pdob40_d2a_START (0)
#define PMIC_OTP0_40_R_np_otp0_pdob40_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob41_d2a : 8;
    } reg;
} PMIC_OTP0_41_R_UNION;
#endif
#define PMIC_OTP0_41_R_np_otp0_pdob41_d2a_START (0)
#define PMIC_OTP0_41_R_np_otp0_pdob41_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob42_d2a : 8;
    } reg;
} PMIC_OTP0_42_R_UNION;
#endif
#define PMIC_OTP0_42_R_np_otp0_pdob42_d2a_START (0)
#define PMIC_OTP0_42_R_np_otp0_pdob42_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob43_d2a : 8;
    } reg;
} PMIC_OTP0_43_R_UNION;
#endif
#define PMIC_OTP0_43_R_np_otp0_pdob43_d2a_START (0)
#define PMIC_OTP0_43_R_np_otp0_pdob43_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob44_d2a : 8;
    } reg;
} PMIC_OTP0_44_R_UNION;
#endif
#define PMIC_OTP0_44_R_np_otp0_pdob44_d2a_START (0)
#define PMIC_OTP0_44_R_np_otp0_pdob44_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob45_d2a : 8;
    } reg;
} PMIC_OTP0_45_R_UNION;
#endif
#define PMIC_OTP0_45_R_np_otp0_pdob45_d2a_START (0)
#define PMIC_OTP0_45_R_np_otp0_pdob45_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob46_d2a : 8;
    } reg;
} PMIC_OTP0_46_R_UNION;
#endif
#define PMIC_OTP0_46_R_np_otp0_pdob46_d2a_START (0)
#define PMIC_OTP0_46_R_np_otp0_pdob46_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob47_d2a : 8;
    } reg;
} PMIC_OTP0_47_R_UNION;
#endif
#define PMIC_OTP0_47_R_np_otp0_pdob47_d2a_START (0)
#define PMIC_OTP0_47_R_np_otp0_pdob47_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob48_d2a : 8;
    } reg;
} PMIC_OTP0_48_R_UNION;
#endif
#define PMIC_OTP0_48_R_np_otp0_pdob48_d2a_START (0)
#define PMIC_OTP0_48_R_np_otp0_pdob48_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob49_d2a : 8;
    } reg;
} PMIC_OTP0_49_R_UNION;
#endif
#define PMIC_OTP0_49_R_np_otp0_pdob49_d2a_START (0)
#define PMIC_OTP0_49_R_np_otp0_pdob49_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob50_d2a : 8;
    } reg;
} PMIC_OTP0_50_R_UNION;
#endif
#define PMIC_OTP0_50_R_np_otp0_pdob50_d2a_START (0)
#define PMIC_OTP0_50_R_np_otp0_pdob50_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob51_d2a : 8;
    } reg;
} PMIC_OTP0_51_R_UNION;
#endif
#define PMIC_OTP0_51_R_np_otp0_pdob51_d2a_START (0)
#define PMIC_OTP0_51_R_np_otp0_pdob51_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob52_d2a : 8;
    } reg;
} PMIC_OTP0_52_R_UNION;
#endif
#define PMIC_OTP0_52_R_np_otp0_pdob52_d2a_START (0)
#define PMIC_OTP0_52_R_np_otp0_pdob52_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob53_d2a : 8;
    } reg;
} PMIC_OTP0_53_R_UNION;
#endif
#define PMIC_OTP0_53_R_np_otp0_pdob53_d2a_START (0)
#define PMIC_OTP0_53_R_np_otp0_pdob53_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob54_d2a : 8;
    } reg;
} PMIC_OTP0_54_R_UNION;
#endif
#define PMIC_OTP0_54_R_np_otp0_pdob54_d2a_START (0)
#define PMIC_OTP0_54_R_np_otp0_pdob54_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob55_d2a : 8;
    } reg;
} PMIC_OTP0_55_R_UNION;
#endif
#define PMIC_OTP0_55_R_np_otp0_pdob55_d2a_START (0)
#define PMIC_OTP0_55_R_np_otp0_pdob55_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob56_d2a : 8;
    } reg;
} PMIC_OTP0_56_R_UNION;
#endif
#define PMIC_OTP0_56_R_np_otp0_pdob56_d2a_START (0)
#define PMIC_OTP0_56_R_np_otp0_pdob56_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob57_d2a : 8;
    } reg;
} PMIC_OTP0_57_R_UNION;
#endif
#define PMIC_OTP0_57_R_np_otp0_pdob57_d2a_START (0)
#define PMIC_OTP0_57_R_np_otp0_pdob57_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob58_d2a : 8;
    } reg;
} PMIC_OTP0_58_R_UNION;
#endif
#define PMIC_OTP0_58_R_np_otp0_pdob58_d2a_START (0)
#define PMIC_OTP0_58_R_np_otp0_pdob58_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob59_d2a : 8;
    } reg;
} PMIC_OTP0_59_R_UNION;
#endif
#define PMIC_OTP0_59_R_np_otp0_pdob59_d2a_START (0)
#define PMIC_OTP0_59_R_np_otp0_pdob59_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob60_d2a : 8;
    } reg;
} PMIC_OTP0_60_R_UNION;
#endif
#define PMIC_OTP0_60_R_np_otp0_pdob60_d2a_START (0)
#define PMIC_OTP0_60_R_np_otp0_pdob60_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob61_d2a : 8;
    } reg;
} PMIC_OTP0_61_R_UNION;
#endif
#define PMIC_OTP0_61_R_np_otp0_pdob61_d2a_START (0)
#define PMIC_OTP0_61_R_np_otp0_pdob61_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob62_d2a : 8;
    } reg;
} PMIC_OTP0_62_R_UNION;
#endif
#define PMIC_OTP0_62_R_np_otp0_pdob62_d2a_START (0)
#define PMIC_OTP0_62_R_np_otp0_pdob62_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp0_pdob63_d2a : 8;
    } reg;
} PMIC_OTP0_63_R_UNION;
#endif
#define PMIC_OTP0_63_R_np_otp0_pdob63_d2a_START (0)
#define PMIC_OTP0_63_R_np_otp0_pdob63_d2a_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob0 : 8;
    } reg;
} PMIC_OTP1_0_R_UNION;
#endif
#define PMIC_OTP1_0_R_np_otp1_pdob0_START (0)
#define PMIC_OTP1_0_R_np_otp1_pdob0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob1 : 8;
    } reg;
} PMIC_OTP1_1_R_UNION;
#endif
#define PMIC_OTP1_1_R_np_otp1_pdob1_START (0)
#define PMIC_OTP1_1_R_np_otp1_pdob1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob2 : 8;
    } reg;
} PMIC_OTP1_2_R_UNION;
#endif
#define PMIC_OTP1_2_R_np_otp1_pdob2_START (0)
#define PMIC_OTP1_2_R_np_otp1_pdob2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob3 : 8;
    } reg;
} PMIC_OTP1_3_R_UNION;
#endif
#define PMIC_OTP1_3_R_np_otp1_pdob3_START (0)
#define PMIC_OTP1_3_R_np_otp1_pdob3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob4 : 8;
    } reg;
} PMIC_OTP1_4_R_UNION;
#endif
#define PMIC_OTP1_4_R_np_otp1_pdob4_START (0)
#define PMIC_OTP1_4_R_np_otp1_pdob4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob5 : 8;
    } reg;
} PMIC_OTP1_5_R_UNION;
#endif
#define PMIC_OTP1_5_R_np_otp1_pdob5_START (0)
#define PMIC_OTP1_5_R_np_otp1_pdob5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob6 : 8;
    } reg;
} PMIC_OTP1_6_R_UNION;
#endif
#define PMIC_OTP1_6_R_np_otp1_pdob6_START (0)
#define PMIC_OTP1_6_R_np_otp1_pdob6_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob7 : 8;
    } reg;
} PMIC_OTP1_7_R_UNION;
#endif
#define PMIC_OTP1_7_R_np_otp1_pdob7_START (0)
#define PMIC_OTP1_7_R_np_otp1_pdob7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob8 : 8;
    } reg;
} PMIC_OTP1_8_R_UNION;
#endif
#define PMIC_OTP1_8_R_np_otp1_pdob8_START (0)
#define PMIC_OTP1_8_R_np_otp1_pdob8_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob9 : 8;
    } reg;
} PMIC_OTP1_9_R_UNION;
#endif
#define PMIC_OTP1_9_R_np_otp1_pdob9_START (0)
#define PMIC_OTP1_9_R_np_otp1_pdob9_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob10 : 8;
    } reg;
} PMIC_OTP1_10_R_UNION;
#endif
#define PMIC_OTP1_10_R_np_otp1_pdob10_START (0)
#define PMIC_OTP1_10_R_np_otp1_pdob10_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob11 : 8;
    } reg;
} PMIC_OTP1_11_R_UNION;
#endif
#define PMIC_OTP1_11_R_np_otp1_pdob11_START (0)
#define PMIC_OTP1_11_R_np_otp1_pdob11_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob12 : 8;
    } reg;
} PMIC_OTP1_12_R_UNION;
#endif
#define PMIC_OTP1_12_R_np_otp1_pdob12_START (0)
#define PMIC_OTP1_12_R_np_otp1_pdob12_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob13 : 8;
    } reg;
} PMIC_OTP1_13_R_UNION;
#endif
#define PMIC_OTP1_13_R_np_otp1_pdob13_START (0)
#define PMIC_OTP1_13_R_np_otp1_pdob13_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob14 : 8;
    } reg;
} PMIC_OTP1_14_R_UNION;
#endif
#define PMIC_OTP1_14_R_np_otp1_pdob14_START (0)
#define PMIC_OTP1_14_R_np_otp1_pdob14_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob15 : 8;
    } reg;
} PMIC_OTP1_15_R_UNION;
#endif
#define PMIC_OTP1_15_R_np_otp1_pdob15_START (0)
#define PMIC_OTP1_15_R_np_otp1_pdob15_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob16 : 8;
    } reg;
} PMIC_OTP1_16_R_UNION;
#endif
#define PMIC_OTP1_16_R_np_otp1_pdob16_START (0)
#define PMIC_OTP1_16_R_np_otp1_pdob16_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob17 : 8;
    } reg;
} PMIC_OTP1_17_R_UNION;
#endif
#define PMIC_OTP1_17_R_np_otp1_pdob17_START (0)
#define PMIC_OTP1_17_R_np_otp1_pdob17_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob18 : 8;
    } reg;
} PMIC_OTP1_18_R_UNION;
#endif
#define PMIC_OTP1_18_R_np_otp1_pdob18_START (0)
#define PMIC_OTP1_18_R_np_otp1_pdob18_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob19 : 8;
    } reg;
} PMIC_OTP1_19_R_UNION;
#endif
#define PMIC_OTP1_19_R_np_otp1_pdob19_START (0)
#define PMIC_OTP1_19_R_np_otp1_pdob19_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob20 : 8;
    } reg;
} PMIC_OTP1_20_R_UNION;
#endif
#define PMIC_OTP1_20_R_np_otp1_pdob20_START (0)
#define PMIC_OTP1_20_R_np_otp1_pdob20_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob21 : 8;
    } reg;
} PMIC_OTP1_21_R_UNION;
#endif
#define PMIC_OTP1_21_R_np_otp1_pdob21_START (0)
#define PMIC_OTP1_21_R_np_otp1_pdob21_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob22 : 8;
    } reg;
} PMIC_OTP1_22_R_UNION;
#endif
#define PMIC_OTP1_22_R_np_otp1_pdob22_START (0)
#define PMIC_OTP1_22_R_np_otp1_pdob22_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob23 : 8;
    } reg;
} PMIC_OTP1_23_R_UNION;
#endif
#define PMIC_OTP1_23_R_np_otp1_pdob23_START (0)
#define PMIC_OTP1_23_R_np_otp1_pdob23_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob24 : 8;
    } reg;
} PMIC_OTP1_24_R_UNION;
#endif
#define PMIC_OTP1_24_R_np_otp1_pdob24_START (0)
#define PMIC_OTP1_24_R_np_otp1_pdob24_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob25 : 8;
    } reg;
} PMIC_OTP1_25_R_UNION;
#endif
#define PMIC_OTP1_25_R_np_otp1_pdob25_START (0)
#define PMIC_OTP1_25_R_np_otp1_pdob25_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob26 : 8;
    } reg;
} PMIC_OTP1_26_R_UNION;
#endif
#define PMIC_OTP1_26_R_np_otp1_pdob26_START (0)
#define PMIC_OTP1_26_R_np_otp1_pdob26_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob27 : 8;
    } reg;
} PMIC_OTP1_27_R_UNION;
#endif
#define PMIC_OTP1_27_R_np_otp1_pdob27_START (0)
#define PMIC_OTP1_27_R_np_otp1_pdob27_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob28 : 8;
    } reg;
} PMIC_OTP1_28_R_UNION;
#endif
#define PMIC_OTP1_28_R_np_otp1_pdob28_START (0)
#define PMIC_OTP1_28_R_np_otp1_pdob28_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob29 : 8;
    } reg;
} PMIC_OTP1_29_R_UNION;
#endif
#define PMIC_OTP1_29_R_np_otp1_pdob29_START (0)
#define PMIC_OTP1_29_R_np_otp1_pdob29_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob30 : 8;
    } reg;
} PMIC_OTP1_30_R_UNION;
#endif
#define PMIC_OTP1_30_R_np_otp1_pdob30_START (0)
#define PMIC_OTP1_30_R_np_otp1_pdob30_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob31 : 8;
    } reg;
} PMIC_OTP1_31_R_UNION;
#endif
#define PMIC_OTP1_31_R_np_otp1_pdob31_START (0)
#define PMIC_OTP1_31_R_np_otp1_pdob31_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob32 : 8;
    } reg;
} PMIC_OTP1_32_R_UNION;
#endif
#define PMIC_OTP1_32_R_np_otp1_pdob32_START (0)
#define PMIC_OTP1_32_R_np_otp1_pdob32_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob33 : 8;
    } reg;
} PMIC_OTP1_33_R_UNION;
#endif
#define PMIC_OTP1_33_R_np_otp1_pdob33_START (0)
#define PMIC_OTP1_33_R_np_otp1_pdob33_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob34 : 8;
    } reg;
} PMIC_OTP1_34_R_UNION;
#endif
#define PMIC_OTP1_34_R_np_otp1_pdob34_START (0)
#define PMIC_OTP1_34_R_np_otp1_pdob34_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob35 : 8;
    } reg;
} PMIC_OTP1_35_R_UNION;
#endif
#define PMIC_OTP1_35_R_np_otp1_pdob35_START (0)
#define PMIC_OTP1_35_R_np_otp1_pdob35_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob36 : 8;
    } reg;
} PMIC_OTP1_36_R_UNION;
#endif
#define PMIC_OTP1_36_R_np_otp1_pdob36_START (0)
#define PMIC_OTP1_36_R_np_otp1_pdob36_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob37 : 8;
    } reg;
} PMIC_OTP1_37_R_UNION;
#endif
#define PMIC_OTP1_37_R_np_otp1_pdob37_START (0)
#define PMIC_OTP1_37_R_np_otp1_pdob37_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob38 : 8;
    } reg;
} PMIC_OTP1_38_R_UNION;
#endif
#define PMIC_OTP1_38_R_np_otp1_pdob38_START (0)
#define PMIC_OTP1_38_R_np_otp1_pdob38_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob39 : 8;
    } reg;
} PMIC_OTP1_39_R_UNION;
#endif
#define PMIC_OTP1_39_R_np_otp1_pdob39_START (0)
#define PMIC_OTP1_39_R_np_otp1_pdob39_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob40 : 8;
    } reg;
} PMIC_OTP1_40_R_UNION;
#endif
#define PMIC_OTP1_40_R_np_otp1_pdob40_START (0)
#define PMIC_OTP1_40_R_np_otp1_pdob40_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob41 : 8;
    } reg;
} PMIC_OTP1_41_R_UNION;
#endif
#define PMIC_OTP1_41_R_np_otp1_pdob41_START (0)
#define PMIC_OTP1_41_R_np_otp1_pdob41_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob42 : 8;
    } reg;
} PMIC_OTP1_42_R_UNION;
#endif
#define PMIC_OTP1_42_R_np_otp1_pdob42_START (0)
#define PMIC_OTP1_42_R_np_otp1_pdob42_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob43 : 8;
    } reg;
} PMIC_OTP1_43_R_UNION;
#endif
#define PMIC_OTP1_43_R_np_otp1_pdob43_START (0)
#define PMIC_OTP1_43_R_np_otp1_pdob43_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob44 : 8;
    } reg;
} PMIC_OTP1_44_R_UNION;
#endif
#define PMIC_OTP1_44_R_np_otp1_pdob44_START (0)
#define PMIC_OTP1_44_R_np_otp1_pdob44_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob45 : 8;
    } reg;
} PMIC_OTP1_45_R_UNION;
#endif
#define PMIC_OTP1_45_R_np_otp1_pdob45_START (0)
#define PMIC_OTP1_45_R_np_otp1_pdob45_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob46 : 8;
    } reg;
} PMIC_OTP1_46_R_UNION;
#endif
#define PMIC_OTP1_46_R_np_otp1_pdob46_START (0)
#define PMIC_OTP1_46_R_np_otp1_pdob46_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob47 : 8;
    } reg;
} PMIC_OTP1_47_R_UNION;
#endif
#define PMIC_OTP1_47_R_np_otp1_pdob47_START (0)
#define PMIC_OTP1_47_R_np_otp1_pdob47_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob48 : 8;
    } reg;
} PMIC_OTP1_48_R_UNION;
#endif
#define PMIC_OTP1_48_R_np_otp1_pdob48_START (0)
#define PMIC_OTP1_48_R_np_otp1_pdob48_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob49 : 8;
    } reg;
} PMIC_OTP1_49_R_UNION;
#endif
#define PMIC_OTP1_49_R_np_otp1_pdob49_START (0)
#define PMIC_OTP1_49_R_np_otp1_pdob49_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob50 : 8;
    } reg;
} PMIC_OTP1_50_R_UNION;
#endif
#define PMIC_OTP1_50_R_np_otp1_pdob50_START (0)
#define PMIC_OTP1_50_R_np_otp1_pdob50_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob51 : 8;
    } reg;
} PMIC_OTP1_51_R_UNION;
#endif
#define PMIC_OTP1_51_R_np_otp1_pdob51_START (0)
#define PMIC_OTP1_51_R_np_otp1_pdob51_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob52 : 8;
    } reg;
} PMIC_OTP1_52_R_UNION;
#endif
#define PMIC_OTP1_52_R_np_otp1_pdob52_START (0)
#define PMIC_OTP1_52_R_np_otp1_pdob52_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob53 : 8;
    } reg;
} PMIC_OTP1_53_R_UNION;
#endif
#define PMIC_OTP1_53_R_np_otp1_pdob53_START (0)
#define PMIC_OTP1_53_R_np_otp1_pdob53_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob54 : 8;
    } reg;
} PMIC_OTP1_54_R_UNION;
#endif
#define PMIC_OTP1_54_R_np_otp1_pdob54_START (0)
#define PMIC_OTP1_54_R_np_otp1_pdob54_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob55 : 8;
    } reg;
} PMIC_OTP1_55_R_UNION;
#endif
#define PMIC_OTP1_55_R_np_otp1_pdob55_START (0)
#define PMIC_OTP1_55_R_np_otp1_pdob55_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob56 : 8;
    } reg;
} PMIC_OTP1_56_R_UNION;
#endif
#define PMIC_OTP1_56_R_np_otp1_pdob56_START (0)
#define PMIC_OTP1_56_R_np_otp1_pdob56_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob57 : 8;
    } reg;
} PMIC_OTP1_57_R_UNION;
#endif
#define PMIC_OTP1_57_R_np_otp1_pdob57_START (0)
#define PMIC_OTP1_57_R_np_otp1_pdob57_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob58 : 8;
    } reg;
} PMIC_OTP1_58_R_UNION;
#endif
#define PMIC_OTP1_58_R_np_otp1_pdob58_START (0)
#define PMIC_OTP1_58_R_np_otp1_pdob58_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob59 : 8;
    } reg;
} PMIC_OTP1_59_R_UNION;
#endif
#define PMIC_OTP1_59_R_np_otp1_pdob59_START (0)
#define PMIC_OTP1_59_R_np_otp1_pdob59_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob60 : 8;
    } reg;
} PMIC_OTP1_60_R_UNION;
#endif
#define PMIC_OTP1_60_R_np_otp1_pdob60_START (0)
#define PMIC_OTP1_60_R_np_otp1_pdob60_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob61 : 8;
    } reg;
} PMIC_OTP1_61_R_UNION;
#endif
#define PMIC_OTP1_61_R_np_otp1_pdob61_START (0)
#define PMIC_OTP1_61_R_np_otp1_pdob61_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob62 : 8;
    } reg;
} PMIC_OTP1_62_R_UNION;
#endif
#define PMIC_OTP1_62_R_np_otp1_pdob62_START (0)
#define PMIC_OTP1_62_R_np_otp1_pdob62_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_otp1_pdob63 : 8;
    } reg;
} PMIC_OTP1_63_R_UNION;
#endif
#define PMIC_OTP1_63_R_np_otp1_pdob63_START (0)
#define PMIC_OTP1_63_R_np_otp1_pdob63_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_pmua_buck_en_cfg : 1;
        unsigned char np_pmud_buck_en_cfg : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_EN_PMUA_PMUD_UNION;
#endif
#define PMIC_EN_PMUA_PMUD_np_pmua_buck_en_cfg_START (0)
#define PMIC_EN_PMUA_PMUD_np_pmua_buck_en_cfg_END (0)
#define PMIC_EN_PMUA_PMUD_np_pmud_buck_en_cfg_START (1)
#define PMIC_EN_PMUA_PMUD_np_pmud_buck_en_cfg_END (1)
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
        unsigned char np_clk32_bt_ds : 2;
        unsigned char np_clk32_sys_ds : 2;
        unsigned char np_spmi_data_ds : 2;
        unsigned char np_auxdac_ssi_ds : 2;
    } reg;
} PMIC_DIG_IO_DS_CFG0_UNION;
#endif
#define PMIC_DIG_IO_DS_CFG0_np_clk32_bt_ds_START (0)
#define PMIC_DIG_IO_DS_CFG0_np_clk32_bt_ds_END (1)
#define PMIC_DIG_IO_DS_CFG0_np_clk32_sys_ds_START (2)
#define PMIC_DIG_IO_DS_CFG0_np_clk32_sys_ds_END (3)
#define PMIC_DIG_IO_DS_CFG0_np_spmi_data_ds_START (4)
#define PMIC_DIG_IO_DS_CFG0_np_spmi_data_ds_END (5)
#define PMIC_DIG_IO_DS_CFG0_np_auxdac_ssi_ds_START (6)
#define PMIC_DIG_IO_DS_CFG0_np_auxdac_ssi_ds_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_pmu0_irq_n_ds : 2;
        unsigned char np_sys_rst_n_ds : 2;
        unsigned char np_clk32_gps_ds : 2;
        unsigned char np_xoadc_ssi_ds : 2;
    } reg;
} PMIC_DIG_IO_DS_CFG1_UNION;
#endif
#define PMIC_DIG_IO_DS_CFG1_np_pmu0_irq_n_ds_START (0)
#define PMIC_DIG_IO_DS_CFG1_np_pmu0_irq_n_ds_END (1)
#define PMIC_DIG_IO_DS_CFG1_np_sys_rst_n_ds_START (2)
#define PMIC_DIG_IO_DS_CFG1_np_sys_rst_n_ds_END (3)
#define PMIC_DIG_IO_DS_CFG1_np_clk32_gps_ds_START (4)
#define PMIC_DIG_IO_DS_CFG1_np_clk32_gps_ds_END (5)
#define PMIC_DIG_IO_DS_CFG1_np_xoadc_ssi_ds_START (6)
#define PMIC_DIG_IO_DS_CFG1_np_xoadc_ssi_ds_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_vbat_drop_irq_ds : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_DIG_IO_DS_CFG2_UNION;
#endif
#define PMIC_DIG_IO_DS_CFG2_np_vbat_drop_irq_ds_START (0)
#define PMIC_DIG_IO_DS_CFG2_np_vbat_drop_irq_ds_END (1)
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
        unsigned char pd_ocv_i_onoff : 1;
        unsigned char reg_data_clr : 1;
        unsigned char cali_auto_time : 3;
        unsigned char cali_auto_onoff_ctrl : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_CLJ_CTRL_REGS2_UNION;
#endif
#define PMIC_CLJ_CTRL_REGS2_pd_ocv_i_onoff_START (0)
#define PMIC_CLJ_CTRL_REGS2_pd_ocv_i_onoff_END (0)
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
        unsigned char ocv_average_sel : 1;
        unsigned char coul_average_sel : 2;
        unsigned char coul_data_rate_sel : 2;
        unsigned char rtc_cnt_sel : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_CIC_CTRL_UNION;
#endif
#define PMIC_CIC_CTRL_ocv_average_sel_START (0)
#define PMIC_CIC_CTRL_ocv_average_sel_END (0)
#define PMIC_CIC_CTRL_coul_average_sel_START (1)
#define PMIC_CIC_CTRL_coul_average_sel_END (2)
#define PMIC_CIC_CTRL_coul_data_rate_sel_START (3)
#define PMIC_CIC_CTRL_coul_data_rate_sel_END (4)
#define PMIC_CIC_CTRL_rtc_cnt_sel_START (5)
#define PMIC_CIC_CTRL_rtc_cnt_sel_END (6)
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
        unsigned char v_out_pre8_s0 : 8;
    } reg;
} PMIC_V_PRE8_OUT0_UNION;
#endif
#define PMIC_V_PRE8_OUT0_v_out_pre8_s0_START (0)
#define PMIC_V_PRE8_OUT0_v_out_pre8_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre8_s1 : 8;
    } reg;
} PMIC_V_PRE8_OUT1_UNION;
#endif
#define PMIC_V_PRE8_OUT1_v_out_pre8_s1_START (0)
#define PMIC_V_PRE8_OUT1_v_out_pre8_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre8_s2 : 8;
    } reg;
} PMIC_V_PRE8_OUT2_UNION;
#endif
#define PMIC_V_PRE8_OUT2_v_out_pre8_s2_START (0)
#define PMIC_V_PRE8_OUT2_v_out_pre8_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre9_s0 : 8;
    } reg;
} PMIC_V_PRE9_OUT0_UNION;
#endif
#define PMIC_V_PRE9_OUT0_v_out_pre9_s0_START (0)
#define PMIC_V_PRE9_OUT0_v_out_pre9_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre9_s1 : 8;
    } reg;
} PMIC_V_PRE9_OUT1_UNION;
#endif
#define PMIC_V_PRE9_OUT1_v_out_pre9_s1_START (0)
#define PMIC_V_PRE9_OUT1_v_out_pre9_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_out_pre9_s2 : 8;
    } reg;
} PMIC_V_PRE9_OUT2_UNION;
#endif
#define PMIC_V_PRE9_OUT2_v_out_pre9_s2_START (0)
#define PMIC_V_PRE9_OUT2_v_out_pre9_s2_END (7)
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
        unsigned char current_pre8_s0 : 8;
    } reg;
} PMIC_CURRENT_PRE8_OUT0_UNION;
#endif
#define PMIC_CURRENT_PRE8_OUT0_current_pre8_s0_START (0)
#define PMIC_CURRENT_PRE8_OUT0_current_pre8_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre8_s1 : 8;
    } reg;
} PMIC_CURRENT_PRE8_OUT1_UNION;
#endif
#define PMIC_CURRENT_PRE8_OUT1_current_pre8_s1_START (0)
#define PMIC_CURRENT_PRE8_OUT1_current_pre8_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre8_s2 : 8;
    } reg;
} PMIC_CURRENT_PRE8_OUT2_UNION;
#endif
#define PMIC_CURRENT_PRE8_OUT2_current_pre8_s2_START (0)
#define PMIC_CURRENT_PRE8_OUT2_current_pre8_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre9_s0 : 8;
    } reg;
} PMIC_CURRENT_PRE9_OUT0_UNION;
#endif
#define PMIC_CURRENT_PRE9_OUT0_current_pre9_s0_START (0)
#define PMIC_CURRENT_PRE9_OUT0_current_pre9_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre9_s1 : 8;
    } reg;
} PMIC_CURRENT_PRE9_OUT1_UNION;
#endif
#define PMIC_CURRENT_PRE9_OUT1_current_pre9_s1_START (0)
#define PMIC_CURRENT_PRE9_OUT1_current_pre9_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_pre9_s2 : 8;
    } reg;
} PMIC_CURRENT_PRE9_OUT2_UNION;
#endif
#define PMIC_CURRENT_PRE9_OUT2_current_pre9_s2_START (0)
#define PMIC_CURRENT_PRE9_OUT2_current_pre9_s2_END (7)
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
        unsigned char v_ocv_out_pre5_s0 : 8;
    } reg;
} PMIC_V_OCV_PRE5_OUT0_UNION;
#endif
#define PMIC_V_OCV_PRE5_OUT0_v_ocv_out_pre5_s0_START (0)
#define PMIC_V_OCV_PRE5_OUT0_v_ocv_out_pre5_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre5_s1 : 8;
    } reg;
} PMIC_V_OCV_PRE5_OUT1_UNION;
#endif
#define PMIC_V_OCV_PRE5_OUT1_v_ocv_out_pre5_s1_START (0)
#define PMIC_V_OCV_PRE5_OUT1_v_ocv_out_pre5_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre5_s2 : 8;
    } reg;
} PMIC_V_OCV_PRE5_OUT2_UNION;
#endif
#define PMIC_V_OCV_PRE5_OUT2_v_ocv_out_pre5_s2_START (0)
#define PMIC_V_OCV_PRE5_OUT2_v_ocv_out_pre5_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre6_s0 : 8;
    } reg;
} PMIC_V_OCV_PRE6_OUT0_UNION;
#endif
#define PMIC_V_OCV_PRE6_OUT0_v_ocv_out_pre6_s0_START (0)
#define PMIC_V_OCV_PRE6_OUT0_v_ocv_out_pre6_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre6_s1 : 8;
    } reg;
} PMIC_V_OCV_PRE6_OUT1_UNION;
#endif
#define PMIC_V_OCV_PRE6_OUT1_v_ocv_out_pre6_s1_START (0)
#define PMIC_V_OCV_PRE6_OUT1_v_ocv_out_pre6_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre6_s2 : 8;
    } reg;
} PMIC_V_OCV_PRE6_OUT2_UNION;
#endif
#define PMIC_V_OCV_PRE6_OUT2_v_ocv_out_pre6_s2_START (0)
#define PMIC_V_OCV_PRE6_OUT2_v_ocv_out_pre6_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre7_s0 : 8;
    } reg;
} PMIC_V_OCV_PRE7_OUT0_UNION;
#endif
#define PMIC_V_OCV_PRE7_OUT0_v_ocv_out_pre7_s0_START (0)
#define PMIC_V_OCV_PRE7_OUT0_v_ocv_out_pre7_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre7_s1 : 8;
    } reg;
} PMIC_V_OCV_PRE7_OUT1_UNION;
#endif
#define PMIC_V_OCV_PRE7_OUT1_v_ocv_out_pre7_s1_START (0)
#define PMIC_V_OCV_PRE7_OUT1_v_ocv_out_pre7_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre7_s2 : 8;
    } reg;
} PMIC_V_OCV_PRE7_OUT2_UNION;
#endif
#define PMIC_V_OCV_PRE7_OUT2_v_ocv_out_pre7_s2_START (0)
#define PMIC_V_OCV_PRE7_OUT2_v_ocv_out_pre7_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre8_s0 : 8;
    } reg;
} PMIC_V_OCV_PRE8_OUT0_UNION;
#endif
#define PMIC_V_OCV_PRE8_OUT0_v_ocv_out_pre8_s0_START (0)
#define PMIC_V_OCV_PRE8_OUT0_v_ocv_out_pre8_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre8_s1 : 8;
    } reg;
} PMIC_V_OCV_PRE8_OUT1_UNION;
#endif
#define PMIC_V_OCV_PRE8_OUT1_v_ocv_out_pre8_s1_START (0)
#define PMIC_V_OCV_PRE8_OUT1_v_ocv_out_pre8_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_ocv_out_pre8_s2 : 8;
    } reg;
} PMIC_V_OCV_PRE8_OUT2_UNION;
#endif
#define PMIC_V_OCV_PRE8_OUT2_v_ocv_out_pre8_s2_START (0)
#define PMIC_V_OCV_PRE8_OUT2_v_ocv_out_pre8_s2_END (7)
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
        unsigned char i_ocv_out_pre5_s0 : 8;
    } reg;
} PMIC_I_OCV_PRE5_OUT0_UNION;
#endif
#define PMIC_I_OCV_PRE5_OUT0_i_ocv_out_pre5_s0_START (0)
#define PMIC_I_OCV_PRE5_OUT0_i_ocv_out_pre5_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre5_s1 : 8;
    } reg;
} PMIC_I_OCV_PRE5_OUT1_UNION;
#endif
#define PMIC_I_OCV_PRE5_OUT1_i_ocv_out_pre5_s1_START (0)
#define PMIC_I_OCV_PRE5_OUT1_i_ocv_out_pre5_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre5_s2 : 8;
    } reg;
} PMIC_I_OCV_PRE5_OUT2_UNION;
#endif
#define PMIC_I_OCV_PRE5_OUT2_i_ocv_out_pre5_s2_START (0)
#define PMIC_I_OCV_PRE5_OUT2_i_ocv_out_pre5_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre6_s0 : 8;
    } reg;
} PMIC_I_OCV_PRE6_OUT0_UNION;
#endif
#define PMIC_I_OCV_PRE6_OUT0_i_ocv_out_pre6_s0_START (0)
#define PMIC_I_OCV_PRE6_OUT0_i_ocv_out_pre6_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre6_s1 : 8;
    } reg;
} PMIC_I_OCV_PRE6_OUT1_UNION;
#endif
#define PMIC_I_OCV_PRE6_OUT1_i_ocv_out_pre6_s1_START (0)
#define PMIC_I_OCV_PRE6_OUT1_i_ocv_out_pre6_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre6_s2 : 8;
    } reg;
} PMIC_I_OCV_PRE6_OUT2_UNION;
#endif
#define PMIC_I_OCV_PRE6_OUT2_i_ocv_out_pre6_s2_START (0)
#define PMIC_I_OCV_PRE6_OUT2_i_ocv_out_pre6_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre7_s0 : 8;
    } reg;
} PMIC_I_OCV_PRE7_OUT0_UNION;
#endif
#define PMIC_I_OCV_PRE7_OUT0_i_ocv_out_pre7_s0_START (0)
#define PMIC_I_OCV_PRE7_OUT0_i_ocv_out_pre7_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre7_s1 : 8;
    } reg;
} PMIC_I_OCV_PRE7_OUT1_UNION;
#endif
#define PMIC_I_OCV_PRE7_OUT1_i_ocv_out_pre7_s1_START (0)
#define PMIC_I_OCV_PRE7_OUT1_i_ocv_out_pre7_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre7_s2 : 8;
    } reg;
} PMIC_I_OCV_PRE7_OUT2_UNION;
#endif
#define PMIC_I_OCV_PRE7_OUT2_i_ocv_out_pre7_s2_START (0)
#define PMIC_I_OCV_PRE7_OUT2_i_ocv_out_pre7_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre8_s0 : 8;
    } reg;
} PMIC_I_OCV_PRE8_OUT0_UNION;
#endif
#define PMIC_I_OCV_PRE8_OUT0_i_ocv_out_pre8_s0_START (0)
#define PMIC_I_OCV_PRE8_OUT0_i_ocv_out_pre8_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre8_s1 : 8;
    } reg;
} PMIC_I_OCV_PRE8_OUT1_UNION;
#endif
#define PMIC_I_OCV_PRE8_OUT1_i_ocv_out_pre8_s1_START (0)
#define PMIC_I_OCV_PRE8_OUT1_i_ocv_out_pre8_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_ocv_out_pre8_s2 : 8;
    } reg;
} PMIC_I_OCV_PRE8_OUT2_UNION;
#endif
#define PMIC_I_OCV_PRE8_OUT2_i_ocv_out_pre8_s2_START (0)
#define PMIC_I_OCV_PRE8_OUT2_i_ocv_out_pre8_s2_END (7)
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
        unsigned char t_ocv_out_pre1_s2 : 8;
    } reg;
} PMIC_T_OCV_PRE1_OUT2_UNION;
#endif
#define PMIC_T_OCV_PRE1_OUT2_t_ocv_out_pre1_s2_START (0)
#define PMIC_T_OCV_PRE1_OUT2_t_ocv_out_pre1_s2_END (7)
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
        unsigned char t_ocv_out_pre2_s2 : 8;
    } reg;
} PMIC_T_OCV_PRE2_OUT2_UNION;
#endif
#define PMIC_T_OCV_PRE2_OUT2_t_ocv_out_pre2_s2_START (0)
#define PMIC_T_OCV_PRE2_OUT2_t_ocv_out_pre2_s2_END (7)
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
        unsigned char t_ocv_out_pre3_s2 : 8;
    } reg;
} PMIC_T_OCV_PRE3_OUT2_UNION;
#endif
#define PMIC_T_OCV_PRE3_OUT2_t_ocv_out_pre3_s2_START (0)
#define PMIC_T_OCV_PRE3_OUT2_t_ocv_out_pre3_s2_END (7)
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
        unsigned char t_ocv_out_pre4_s2 : 8;
    } reg;
} PMIC_T_OCV_PRE4_OUT2_UNION;
#endif
#define PMIC_T_OCV_PRE4_OUT2_t_ocv_out_pre4_s2_START (0)
#define PMIC_T_OCV_PRE4_OUT2_t_ocv_out_pre4_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_out_pre5_s0 : 8;
    } reg;
} PMIC_T_OCV_PRE5_OUT0_UNION;
#endif
#define PMIC_T_OCV_PRE5_OUT0_t_ocv_out_pre5_s0_START (0)
#define PMIC_T_OCV_PRE5_OUT0_t_ocv_out_pre5_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_out_pre5_s1 : 8;
    } reg;
} PMIC_T_OCV_PRE5_OUT1_UNION;
#endif
#define PMIC_T_OCV_PRE5_OUT1_t_ocv_out_pre5_s1_START (0)
#define PMIC_T_OCV_PRE5_OUT1_t_ocv_out_pre5_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_out_pre5_s2 : 8;
    } reg;
} PMIC_T_OCV_PRE5_OUT2_UNION;
#endif
#define PMIC_T_OCV_PRE5_OUT2_t_ocv_out_pre5_s2_START (0)
#define PMIC_T_OCV_PRE5_OUT2_t_ocv_out_pre5_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_out_pre6_s0 : 8;
    } reg;
} PMIC_T_OCV_PRE6_OUT0_UNION;
#endif
#define PMIC_T_OCV_PRE6_OUT0_t_ocv_out_pre6_s0_START (0)
#define PMIC_T_OCV_PRE6_OUT0_t_ocv_out_pre6_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_out_pre6_s1 : 8;
    } reg;
} PMIC_T_OCV_PRE6_OUT1_UNION;
#endif
#define PMIC_T_OCV_PRE6_OUT1_t_ocv_out_pre6_s1_START (0)
#define PMIC_T_OCV_PRE6_OUT1_t_ocv_out_pre6_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_out_pre6_s2 : 8;
    } reg;
} PMIC_T_OCV_PRE6_OUT2_UNION;
#endif
#define PMIC_T_OCV_PRE6_OUT2_t_ocv_out_pre6_s2_START (0)
#define PMIC_T_OCV_PRE6_OUT2_t_ocv_out_pre6_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_out_pre7_s0 : 8;
    } reg;
} PMIC_T_OCV_PRE7_OUT0_UNION;
#endif
#define PMIC_T_OCV_PRE7_OUT0_t_ocv_out_pre7_s0_START (0)
#define PMIC_T_OCV_PRE7_OUT0_t_ocv_out_pre7_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_out_pre7_s1 : 8;
    } reg;
} PMIC_T_OCV_PRE7_OUT1_UNION;
#endif
#define PMIC_T_OCV_PRE7_OUT1_t_ocv_out_pre7_s1_START (0)
#define PMIC_T_OCV_PRE7_OUT1_t_ocv_out_pre7_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_out_pre7_s2 : 8;
    } reg;
} PMIC_T_OCV_PRE7_OUT2_UNION;
#endif
#define PMIC_T_OCV_PRE7_OUT2_t_ocv_out_pre7_s2_START (0)
#define PMIC_T_OCV_PRE7_OUT2_t_ocv_out_pre7_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_out_pre8_s0 : 8;
    } reg;
} PMIC_T_OCV_PRE8_OUT0_UNION;
#endif
#define PMIC_T_OCV_PRE8_OUT0_t_ocv_out_pre8_s0_START (0)
#define PMIC_T_OCV_PRE8_OUT0_t_ocv_out_pre8_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_out_pre8_s1 : 8;
    } reg;
} PMIC_T_OCV_PRE8_OUT1_UNION;
#endif
#define PMIC_T_OCV_PRE8_OUT1_t_ocv_out_pre8_s1_START (0)
#define PMIC_T_OCV_PRE8_OUT1_t_ocv_out_pre8_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char t_ocv_out_pre8_s2 : 8;
    } reg;
} PMIC_T_OCV_PRE8_OUT2_UNION;
#endif
#define PMIC_T_OCV_PRE8_OUT2_t_ocv_out_pre8_s2_START (0)
#define PMIC_T_OCV_PRE8_OUT2_t_ocv_out_pre8_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre1_s0 : 8;
    } reg;
} PMIC_RTC_OCV_PRE1_OUT0_UNION;
#endif
#define PMIC_RTC_OCV_PRE1_OUT0_rtc_ocv_out_pre1_s0_START (0)
#define PMIC_RTC_OCV_PRE1_OUT0_rtc_ocv_out_pre1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre1_s1 : 8;
    } reg;
} PMIC_RTC_OCV_PRE1_OUT1_UNION;
#endif
#define PMIC_RTC_OCV_PRE1_OUT1_rtc_ocv_out_pre1_s1_START (0)
#define PMIC_RTC_OCV_PRE1_OUT1_rtc_ocv_out_pre1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre1_s2 : 8;
    } reg;
} PMIC_RTC_OCV_PRE1_OUT2_UNION;
#endif
#define PMIC_RTC_OCV_PRE1_OUT2_rtc_ocv_out_pre1_s2_START (0)
#define PMIC_RTC_OCV_PRE1_OUT2_rtc_ocv_out_pre1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre1_s3 : 8;
    } reg;
} PMIC_RTC_OCV_PRE1_OUT3_UNION;
#endif
#define PMIC_RTC_OCV_PRE1_OUT3_rtc_ocv_out_pre1_s3_START (0)
#define PMIC_RTC_OCV_PRE1_OUT3_rtc_ocv_out_pre1_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre2_s0 : 8;
    } reg;
} PMIC_RTC_OCV_PRE2_OUT0_UNION;
#endif
#define PMIC_RTC_OCV_PRE2_OUT0_rtc_ocv_out_pre2_s0_START (0)
#define PMIC_RTC_OCV_PRE2_OUT0_rtc_ocv_out_pre2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre2_s1 : 8;
    } reg;
} PMIC_RTC_OCV_PRE2_OUT1_UNION;
#endif
#define PMIC_RTC_OCV_PRE2_OUT1_rtc_ocv_out_pre2_s1_START (0)
#define PMIC_RTC_OCV_PRE2_OUT1_rtc_ocv_out_pre2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre2_s2 : 8;
    } reg;
} PMIC_RTC_OCV_PRE2_OUT2_UNION;
#endif
#define PMIC_RTC_OCV_PRE2_OUT2_rtc_ocv_out_pre2_s2_START (0)
#define PMIC_RTC_OCV_PRE2_OUT2_rtc_ocv_out_pre2_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre2_s3 : 8;
    } reg;
} PMIC_RTC_OCV_PRE2_OUT3_UNION;
#endif
#define PMIC_RTC_OCV_PRE2_OUT3_rtc_ocv_out_pre2_s3_START (0)
#define PMIC_RTC_OCV_PRE2_OUT3_rtc_ocv_out_pre2_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre3_s0 : 8;
    } reg;
} PMIC_RTC_OCV_PRE3_OUT0_UNION;
#endif
#define PMIC_RTC_OCV_PRE3_OUT0_rtc_ocv_out_pre3_s0_START (0)
#define PMIC_RTC_OCV_PRE3_OUT0_rtc_ocv_out_pre3_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre3_s1 : 8;
    } reg;
} PMIC_RTC_OCV_PRE3_OUT1_UNION;
#endif
#define PMIC_RTC_OCV_PRE3_OUT1_rtc_ocv_out_pre3_s1_START (0)
#define PMIC_RTC_OCV_PRE3_OUT1_rtc_ocv_out_pre3_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre3_s2 : 8;
    } reg;
} PMIC_RTC_OCV_PRE3_OUT2_UNION;
#endif
#define PMIC_RTC_OCV_PRE3_OUT2_rtc_ocv_out_pre3_s2_START (0)
#define PMIC_RTC_OCV_PRE3_OUT2_rtc_ocv_out_pre3_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre3_s3 : 8;
    } reg;
} PMIC_RTC_OCV_PRE3_OUT3_UNION;
#endif
#define PMIC_RTC_OCV_PRE3_OUT3_rtc_ocv_out_pre3_s3_START (0)
#define PMIC_RTC_OCV_PRE3_OUT3_rtc_ocv_out_pre3_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre4_s0 : 8;
    } reg;
} PMIC_RTC_OCV_PRE4_OUT0_UNION;
#endif
#define PMIC_RTC_OCV_PRE4_OUT0_rtc_ocv_out_pre4_s0_START (0)
#define PMIC_RTC_OCV_PRE4_OUT0_rtc_ocv_out_pre4_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre4_s1 : 8;
    } reg;
} PMIC_RTC_OCV_PRE4_OUT1_UNION;
#endif
#define PMIC_RTC_OCV_PRE4_OUT1_rtc_ocv_out_pre4_s1_START (0)
#define PMIC_RTC_OCV_PRE4_OUT1_rtc_ocv_out_pre4_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre4_s2 : 8;
    } reg;
} PMIC_RTC_OCV_PRE4_OUT2_UNION;
#endif
#define PMIC_RTC_OCV_PRE4_OUT2_rtc_ocv_out_pre4_s2_START (0)
#define PMIC_RTC_OCV_PRE4_OUT2_rtc_ocv_out_pre4_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre4_s3 : 8;
    } reg;
} PMIC_RTC_OCV_PRE4_OUT3_UNION;
#endif
#define PMIC_RTC_OCV_PRE4_OUT3_rtc_ocv_out_pre4_s3_START (0)
#define PMIC_RTC_OCV_PRE4_OUT3_rtc_ocv_out_pre4_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre5_s0 : 8;
    } reg;
} PMIC_RTC_OCV_PRE5_OUT0_UNION;
#endif
#define PMIC_RTC_OCV_PRE5_OUT0_rtc_ocv_out_pre5_s0_START (0)
#define PMIC_RTC_OCV_PRE5_OUT0_rtc_ocv_out_pre5_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre5_s1 : 8;
    } reg;
} PMIC_RTC_OCV_PRE5_OUT1_UNION;
#endif
#define PMIC_RTC_OCV_PRE5_OUT1_rtc_ocv_out_pre5_s1_START (0)
#define PMIC_RTC_OCV_PRE5_OUT1_rtc_ocv_out_pre5_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre5_s2 : 8;
    } reg;
} PMIC_RTC_OCV_PRE5_OUT2_UNION;
#endif
#define PMIC_RTC_OCV_PRE5_OUT2_rtc_ocv_out_pre5_s2_START (0)
#define PMIC_RTC_OCV_PRE5_OUT2_rtc_ocv_out_pre5_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre5_s3 : 8;
    } reg;
} PMIC_RTC_OCV_PRE5_OUT3_UNION;
#endif
#define PMIC_RTC_OCV_PRE5_OUT3_rtc_ocv_out_pre5_s3_START (0)
#define PMIC_RTC_OCV_PRE5_OUT3_rtc_ocv_out_pre5_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre6_s0 : 8;
    } reg;
} PMIC_RTC_OCV_PRE6_OUT0_UNION;
#endif
#define PMIC_RTC_OCV_PRE6_OUT0_rtc_ocv_out_pre6_s0_START (0)
#define PMIC_RTC_OCV_PRE6_OUT0_rtc_ocv_out_pre6_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre6_s1 : 8;
    } reg;
} PMIC_RTC_OCV_PRE6_OUT1_UNION;
#endif
#define PMIC_RTC_OCV_PRE6_OUT1_rtc_ocv_out_pre6_s1_START (0)
#define PMIC_RTC_OCV_PRE6_OUT1_rtc_ocv_out_pre6_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre6_s2 : 8;
    } reg;
} PMIC_RTC_OCV_PRE6_OUT2_UNION;
#endif
#define PMIC_RTC_OCV_PRE6_OUT2_rtc_ocv_out_pre6_s2_START (0)
#define PMIC_RTC_OCV_PRE6_OUT2_rtc_ocv_out_pre6_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre6_s3 : 8;
    } reg;
} PMIC_RTC_OCV_PRE6_OUT3_UNION;
#endif
#define PMIC_RTC_OCV_PRE6_OUT3_rtc_ocv_out_pre6_s3_START (0)
#define PMIC_RTC_OCV_PRE6_OUT3_rtc_ocv_out_pre6_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre7_s0 : 8;
    } reg;
} PMIC_RTC_OCV_PRE7_OUT0_UNION;
#endif
#define PMIC_RTC_OCV_PRE7_OUT0_rtc_ocv_out_pre7_s0_START (0)
#define PMIC_RTC_OCV_PRE7_OUT0_rtc_ocv_out_pre7_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre7_s1 : 8;
    } reg;
} PMIC_RTC_OCV_PRE7_OUT1_UNION;
#endif
#define PMIC_RTC_OCV_PRE7_OUT1_rtc_ocv_out_pre7_s1_START (0)
#define PMIC_RTC_OCV_PRE7_OUT1_rtc_ocv_out_pre7_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre7_s2 : 8;
    } reg;
} PMIC_RTC_OCV_PRE7_OUT2_UNION;
#endif
#define PMIC_RTC_OCV_PRE7_OUT2_rtc_ocv_out_pre7_s2_START (0)
#define PMIC_RTC_OCV_PRE7_OUT2_rtc_ocv_out_pre7_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre7_s3 : 8;
    } reg;
} PMIC_RTC_OCV_PRE7_OUT3_UNION;
#endif
#define PMIC_RTC_OCV_PRE7_OUT3_rtc_ocv_out_pre7_s3_START (0)
#define PMIC_RTC_OCV_PRE7_OUT3_rtc_ocv_out_pre7_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre8_s0 : 8;
    } reg;
} PMIC_RTC_OCV_PRE8_OUT0_UNION;
#endif
#define PMIC_RTC_OCV_PRE8_OUT0_rtc_ocv_out_pre8_s0_START (0)
#define PMIC_RTC_OCV_PRE8_OUT0_rtc_ocv_out_pre8_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre8_s1 : 8;
    } reg;
} PMIC_RTC_OCV_PRE8_OUT1_UNION;
#endif
#define PMIC_RTC_OCV_PRE8_OUT1_rtc_ocv_out_pre8_s1_START (0)
#define PMIC_RTC_OCV_PRE8_OUT1_rtc_ocv_out_pre8_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre8_s2 : 8;
    } reg;
} PMIC_RTC_OCV_PRE8_OUT2_UNION;
#endif
#define PMIC_RTC_OCV_PRE8_OUT2_rtc_ocv_out_pre8_s2_START (0)
#define PMIC_RTC_OCV_PRE8_OUT2_rtc_ocv_out_pre8_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre8_s3 : 8;
    } reg;
} PMIC_RTC_OCV_PRE8_OUT3_UNION;
#endif
#define PMIC_RTC_OCV_PRE8_OUT3_rtc_ocv_out_pre8_s3_START (0)
#define PMIC_RTC_OCV_PRE8_OUT3_rtc_ocv_out_pre8_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre1_s0 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE1_OUT0_UNION;
#endif
#define PMIC_OFTV_OCV_PRE1_OUT0_oftv_ocv_out_pre1_s0_START (0)
#define PMIC_OFTV_OCV_PRE1_OUT0_oftv_ocv_out_pre1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre1_s1 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE1_OUT1_UNION;
#endif
#define PMIC_OFTV_OCV_PRE1_OUT1_oftv_ocv_out_pre1_s1_START (0)
#define PMIC_OFTV_OCV_PRE1_OUT1_oftv_ocv_out_pre1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre1_s2 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE1_OUT2_UNION;
#endif
#define PMIC_OFTV_OCV_PRE1_OUT2_oftv_ocv_out_pre1_s2_START (0)
#define PMIC_OFTV_OCV_PRE1_OUT2_oftv_ocv_out_pre1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre2_s0 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE2_OUT0_UNION;
#endif
#define PMIC_OFTV_OCV_PRE2_OUT0_oftv_ocv_out_pre2_s0_START (0)
#define PMIC_OFTV_OCV_PRE2_OUT0_oftv_ocv_out_pre2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre2_s1 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE2_OUT1_UNION;
#endif
#define PMIC_OFTV_OCV_PRE2_OUT1_oftv_ocv_out_pre2_s1_START (0)
#define PMIC_OFTV_OCV_PRE2_OUT1_oftv_ocv_out_pre2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre2_s2 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE2_OUT2_UNION;
#endif
#define PMIC_OFTV_OCV_PRE2_OUT2_oftv_ocv_out_pre2_s2_START (0)
#define PMIC_OFTV_OCV_PRE2_OUT2_oftv_ocv_out_pre2_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre3_s0 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE3_OUT0_UNION;
#endif
#define PMIC_OFTV_OCV_PRE3_OUT0_oftv_ocv_out_pre3_s0_START (0)
#define PMIC_OFTV_OCV_PRE3_OUT0_oftv_ocv_out_pre3_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre3_s1 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE3_OUT1_UNION;
#endif
#define PMIC_OFTV_OCV_PRE3_OUT1_oftv_ocv_out_pre3_s1_START (0)
#define PMIC_OFTV_OCV_PRE3_OUT1_oftv_ocv_out_pre3_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre3_s2 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE3_OUT2_UNION;
#endif
#define PMIC_OFTV_OCV_PRE3_OUT2_oftv_ocv_out_pre3_s2_START (0)
#define PMIC_OFTV_OCV_PRE3_OUT2_oftv_ocv_out_pre3_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre4_s0 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE4_OUT0_UNION;
#endif
#define PMIC_OFTV_OCV_PRE4_OUT0_oftv_ocv_out_pre4_s0_START (0)
#define PMIC_OFTV_OCV_PRE4_OUT0_oftv_ocv_out_pre4_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre4_s1 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE4_OUT1_UNION;
#endif
#define PMIC_OFTV_OCV_PRE4_OUT1_oftv_ocv_out_pre4_s1_START (0)
#define PMIC_OFTV_OCV_PRE4_OUT1_oftv_ocv_out_pre4_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre4_s2 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE4_OUT2_UNION;
#endif
#define PMIC_OFTV_OCV_PRE4_OUT2_oftv_ocv_out_pre4_s2_START (0)
#define PMIC_OFTV_OCV_PRE4_OUT2_oftv_ocv_out_pre4_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre5_s0 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE5_OUT0_UNION;
#endif
#define PMIC_OFTV_OCV_PRE5_OUT0_oftv_ocv_out_pre5_s0_START (0)
#define PMIC_OFTV_OCV_PRE5_OUT0_oftv_ocv_out_pre5_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre5_s1 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE5_OUT1_UNION;
#endif
#define PMIC_OFTV_OCV_PRE5_OUT1_oftv_ocv_out_pre5_s1_START (0)
#define PMIC_OFTV_OCV_PRE5_OUT1_oftv_ocv_out_pre5_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre5_s2 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE5_OUT2_UNION;
#endif
#define PMIC_OFTV_OCV_PRE5_OUT2_oftv_ocv_out_pre5_s2_START (0)
#define PMIC_OFTV_OCV_PRE5_OUT2_oftv_ocv_out_pre5_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre6_s0 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE6_OUT0_UNION;
#endif
#define PMIC_OFTV_OCV_PRE6_OUT0_oftv_ocv_out_pre6_s0_START (0)
#define PMIC_OFTV_OCV_PRE6_OUT0_oftv_ocv_out_pre6_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre6_s1 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE6_OUT1_UNION;
#endif
#define PMIC_OFTV_OCV_PRE6_OUT1_oftv_ocv_out_pre6_s1_START (0)
#define PMIC_OFTV_OCV_PRE6_OUT1_oftv_ocv_out_pre6_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre6_s2 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE6_OUT2_UNION;
#endif
#define PMIC_OFTV_OCV_PRE6_OUT2_oftv_ocv_out_pre6_s2_START (0)
#define PMIC_OFTV_OCV_PRE6_OUT2_oftv_ocv_out_pre6_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre7_s0 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE7_OUT0_UNION;
#endif
#define PMIC_OFTV_OCV_PRE7_OUT0_oftv_ocv_out_pre7_s0_START (0)
#define PMIC_OFTV_OCV_PRE7_OUT0_oftv_ocv_out_pre7_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre7_s1 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE7_OUT1_UNION;
#endif
#define PMIC_OFTV_OCV_PRE7_OUT1_oftv_ocv_out_pre7_s1_START (0)
#define PMIC_OFTV_OCV_PRE7_OUT1_oftv_ocv_out_pre7_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre7_s2 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE7_OUT2_UNION;
#endif
#define PMIC_OFTV_OCV_PRE7_OUT2_oftv_ocv_out_pre7_s2_START (0)
#define PMIC_OFTV_OCV_PRE7_OUT2_oftv_ocv_out_pre7_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre8_s0 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE8_OUT0_UNION;
#endif
#define PMIC_OFTV_OCV_PRE8_OUT0_oftv_ocv_out_pre8_s0_START (0)
#define PMIC_OFTV_OCV_PRE8_OUT0_oftv_ocv_out_pre8_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre8_s1 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE8_OUT1_UNION;
#endif
#define PMIC_OFTV_OCV_PRE8_OUT1_oftv_ocv_out_pre8_s1_START (0)
#define PMIC_OFTV_OCV_PRE8_OUT1_oftv_ocv_out_pre8_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char oftv_ocv_out_pre8_s2 : 8;
    } reg;
} PMIC_OFTV_OCV_PRE8_OUT2_UNION;
#endif
#define PMIC_OFTV_OCV_PRE8_OUT2_oftv_ocv_out_pre8_s2_START (0)
#define PMIC_OFTV_OCV_PRE8_OUT2_oftv_ocv_out_pre8_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre1_s0 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE1_OUT0_UNION;
#endif
#define PMIC_OFTI_OCV_PRE1_OUT0_ofti_ocv_out_pre1_s0_START (0)
#define PMIC_OFTI_OCV_PRE1_OUT0_ofti_ocv_out_pre1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre1_s1 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE1_OUT1_UNION;
#endif
#define PMIC_OFTI_OCV_PRE1_OUT1_ofti_ocv_out_pre1_s1_START (0)
#define PMIC_OFTI_OCV_PRE1_OUT1_ofti_ocv_out_pre1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre1_s2 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE1_OUT2_UNION;
#endif
#define PMIC_OFTI_OCV_PRE1_OUT2_ofti_ocv_out_pre1_s2_START (0)
#define PMIC_OFTI_OCV_PRE1_OUT2_ofti_ocv_out_pre1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre2_s0 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE2_OUT0_UNION;
#endif
#define PMIC_OFTI_OCV_PRE2_OUT0_ofti_ocv_out_pre2_s0_START (0)
#define PMIC_OFTI_OCV_PRE2_OUT0_ofti_ocv_out_pre2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre2_s1 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE2_OUT1_UNION;
#endif
#define PMIC_OFTI_OCV_PRE2_OUT1_ofti_ocv_out_pre2_s1_START (0)
#define PMIC_OFTI_OCV_PRE2_OUT1_ofti_ocv_out_pre2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre2_s2 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE2_OUT2_UNION;
#endif
#define PMIC_OFTI_OCV_PRE2_OUT2_ofti_ocv_out_pre2_s2_START (0)
#define PMIC_OFTI_OCV_PRE2_OUT2_ofti_ocv_out_pre2_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre3_s0 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE3_OUT0_UNION;
#endif
#define PMIC_OFTI_OCV_PRE3_OUT0_ofti_ocv_out_pre3_s0_START (0)
#define PMIC_OFTI_OCV_PRE3_OUT0_ofti_ocv_out_pre3_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre3_s1 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE3_OUT1_UNION;
#endif
#define PMIC_OFTI_OCV_PRE3_OUT1_ofti_ocv_out_pre3_s1_START (0)
#define PMIC_OFTI_OCV_PRE3_OUT1_ofti_ocv_out_pre3_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre3_s2 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE3_OUT2_UNION;
#endif
#define PMIC_OFTI_OCV_PRE3_OUT2_ofti_ocv_out_pre3_s2_START (0)
#define PMIC_OFTI_OCV_PRE3_OUT2_ofti_ocv_out_pre3_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre4_s0 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE4_OUT0_UNION;
#endif
#define PMIC_OFTI_OCV_PRE4_OUT0_ofti_ocv_out_pre4_s0_START (0)
#define PMIC_OFTI_OCV_PRE4_OUT0_ofti_ocv_out_pre4_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre4_s1 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE4_OUT1_UNION;
#endif
#define PMIC_OFTI_OCV_PRE4_OUT1_ofti_ocv_out_pre4_s1_START (0)
#define PMIC_OFTI_OCV_PRE4_OUT1_ofti_ocv_out_pre4_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre4_s2 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE4_OUT2_UNION;
#endif
#define PMIC_OFTI_OCV_PRE4_OUT2_ofti_ocv_out_pre4_s2_START (0)
#define PMIC_OFTI_OCV_PRE4_OUT2_ofti_ocv_out_pre4_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre5_s0 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE5_OUT0_UNION;
#endif
#define PMIC_OFTI_OCV_PRE5_OUT0_ofti_ocv_out_pre5_s0_START (0)
#define PMIC_OFTI_OCV_PRE5_OUT0_ofti_ocv_out_pre5_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre5_s1 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE5_OUT1_UNION;
#endif
#define PMIC_OFTI_OCV_PRE5_OUT1_ofti_ocv_out_pre5_s1_START (0)
#define PMIC_OFTI_OCV_PRE5_OUT1_ofti_ocv_out_pre5_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre5_s2 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE5_OUT2_UNION;
#endif
#define PMIC_OFTI_OCV_PRE5_OUT2_ofti_ocv_out_pre5_s2_START (0)
#define PMIC_OFTI_OCV_PRE5_OUT2_ofti_ocv_out_pre5_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre6_s0 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE6_OUT0_UNION;
#endif
#define PMIC_OFTI_OCV_PRE6_OUT0_ofti_ocv_out_pre6_s0_START (0)
#define PMIC_OFTI_OCV_PRE6_OUT0_ofti_ocv_out_pre6_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre6_s1 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE6_OUT1_UNION;
#endif
#define PMIC_OFTI_OCV_PRE6_OUT1_ofti_ocv_out_pre6_s1_START (0)
#define PMIC_OFTI_OCV_PRE6_OUT1_ofti_ocv_out_pre6_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre6_s2 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE6_OUT2_UNION;
#endif
#define PMIC_OFTI_OCV_PRE6_OUT2_ofti_ocv_out_pre6_s2_START (0)
#define PMIC_OFTI_OCV_PRE6_OUT2_ofti_ocv_out_pre6_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre7_s0 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE7_OUT0_UNION;
#endif
#define PMIC_OFTI_OCV_PRE7_OUT0_ofti_ocv_out_pre7_s0_START (0)
#define PMIC_OFTI_OCV_PRE7_OUT0_ofti_ocv_out_pre7_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre7_s1 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE7_OUT1_UNION;
#endif
#define PMIC_OFTI_OCV_PRE7_OUT1_ofti_ocv_out_pre7_s1_START (0)
#define PMIC_OFTI_OCV_PRE7_OUT1_ofti_ocv_out_pre7_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre7_s2 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE7_OUT2_UNION;
#endif
#define PMIC_OFTI_OCV_PRE7_OUT2_ofti_ocv_out_pre7_s2_START (0)
#define PMIC_OFTI_OCV_PRE7_OUT2_ofti_ocv_out_pre7_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre8_s0 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE8_OUT0_UNION;
#endif
#define PMIC_OFTI_OCV_PRE8_OUT0_ofti_ocv_out_pre8_s0_START (0)
#define PMIC_OFTI_OCV_PRE8_OUT0_ofti_ocv_out_pre8_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre8_s1 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE8_OUT1_UNION;
#endif
#define PMIC_OFTI_OCV_PRE8_OUT1_ofti_ocv_out_pre8_s1_START (0)
#define PMIC_OFTI_OCV_PRE8_OUT1_ofti_ocv_out_pre8_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ofti_ocv_out_pre8_s2 : 8;
    } reg;
} PMIC_OFTI_OCV_PRE8_OUT2_UNION;
#endif
#define PMIC_OFTI_OCV_PRE8_OUT2_ofti_ocv_out_pre8_s2_START (0)
#define PMIC_OFTI_OCV_PRE8_OUT2_ofti_ocv_out_pre8_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre0_s0 : 8;
    } reg;
} PMIC_DCR_V_PRE0_OUT0_UNION;
#endif
#define PMIC_DCR_V_PRE0_OUT0_dcr_v_out_pre0_s0_START (0)
#define PMIC_DCR_V_PRE0_OUT0_dcr_v_out_pre0_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre0_s1 : 8;
    } reg;
} PMIC_DCR_V_PRE0_OUT1_UNION;
#endif
#define PMIC_DCR_V_PRE0_OUT1_dcr_v_out_pre0_s1_START (0)
#define PMIC_DCR_V_PRE0_OUT1_dcr_v_out_pre0_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre0_s2 : 8;
    } reg;
} PMIC_DCR_V_PRE0_OUT2_UNION;
#endif
#define PMIC_DCR_V_PRE0_OUT2_dcr_v_out_pre0_s2_START (0)
#define PMIC_DCR_V_PRE0_OUT2_dcr_v_out_pre0_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre1_s0 : 8;
    } reg;
} PMIC_DCR_V_PRE1_OUT0_UNION;
#endif
#define PMIC_DCR_V_PRE1_OUT0_dcr_v_out_pre1_s0_START (0)
#define PMIC_DCR_V_PRE1_OUT0_dcr_v_out_pre1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre1_s1 : 8;
    } reg;
} PMIC_DCR_V_PRE1_OUT1_UNION;
#endif
#define PMIC_DCR_V_PRE1_OUT1_dcr_v_out_pre1_s1_START (0)
#define PMIC_DCR_V_PRE1_OUT1_dcr_v_out_pre1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre1_s2 : 8;
    } reg;
} PMIC_DCR_V_PRE1_OUT2_UNION;
#endif
#define PMIC_DCR_V_PRE1_OUT2_dcr_v_out_pre1_s2_START (0)
#define PMIC_DCR_V_PRE1_OUT2_dcr_v_out_pre1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre2_s0 : 8;
    } reg;
} PMIC_DCR_V_PRE2_OUT0_UNION;
#endif
#define PMIC_DCR_V_PRE2_OUT0_dcr_v_out_pre2_s0_START (0)
#define PMIC_DCR_V_PRE2_OUT0_dcr_v_out_pre2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre2_s1 : 8;
    } reg;
} PMIC_DCR_V_PRE2_OUT1_UNION;
#endif
#define PMIC_DCR_V_PRE2_OUT1_dcr_v_out_pre2_s1_START (0)
#define PMIC_DCR_V_PRE2_OUT1_dcr_v_out_pre2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre2_s2 : 8;
    } reg;
} PMIC_DCR_V_PRE2_OUT2_UNION;
#endif
#define PMIC_DCR_V_PRE2_OUT2_dcr_v_out_pre2_s2_START (0)
#define PMIC_DCR_V_PRE2_OUT2_dcr_v_out_pre2_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre3_s0 : 8;
    } reg;
} PMIC_DCR_V_PRE3_OUT0_UNION;
#endif
#define PMIC_DCR_V_PRE3_OUT0_dcr_v_out_pre3_s0_START (0)
#define PMIC_DCR_V_PRE3_OUT0_dcr_v_out_pre3_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre3_s1 : 8;
    } reg;
} PMIC_DCR_V_PRE3_OUT1_UNION;
#endif
#define PMIC_DCR_V_PRE3_OUT1_dcr_v_out_pre3_s1_START (0)
#define PMIC_DCR_V_PRE3_OUT1_dcr_v_out_pre3_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre3_s2 : 8;
    } reg;
} PMIC_DCR_V_PRE3_OUT2_UNION;
#endif
#define PMIC_DCR_V_PRE3_OUT2_dcr_v_out_pre3_s2_START (0)
#define PMIC_DCR_V_PRE3_OUT2_dcr_v_out_pre3_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre4_s0 : 8;
    } reg;
} PMIC_DCR_V_PRE4_OUT0_UNION;
#endif
#define PMIC_DCR_V_PRE4_OUT0_dcr_v_out_pre4_s0_START (0)
#define PMIC_DCR_V_PRE4_OUT0_dcr_v_out_pre4_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre4_s1 : 8;
    } reg;
} PMIC_DCR_V_PRE4_OUT1_UNION;
#endif
#define PMIC_DCR_V_PRE4_OUT1_dcr_v_out_pre4_s1_START (0)
#define PMIC_DCR_V_PRE4_OUT1_dcr_v_out_pre4_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre4_s2 : 8;
    } reg;
} PMIC_DCR_V_PRE4_OUT2_UNION;
#endif
#define PMIC_DCR_V_PRE4_OUT2_dcr_v_out_pre4_s2_START (0)
#define PMIC_DCR_V_PRE4_OUT2_dcr_v_out_pre4_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre5_s0 : 8;
    } reg;
} PMIC_DCR_V_PRE5_OUT0_UNION;
#endif
#define PMIC_DCR_V_PRE5_OUT0_dcr_v_out_pre5_s0_START (0)
#define PMIC_DCR_V_PRE5_OUT0_dcr_v_out_pre5_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre5_s1 : 8;
    } reg;
} PMIC_DCR_V_PRE5_OUT1_UNION;
#endif
#define PMIC_DCR_V_PRE5_OUT1_dcr_v_out_pre5_s1_START (0)
#define PMIC_DCR_V_PRE5_OUT1_dcr_v_out_pre5_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre5_s2 : 8;
    } reg;
} PMIC_DCR_V_PRE5_OUT2_UNION;
#endif
#define PMIC_DCR_V_PRE5_OUT2_dcr_v_out_pre5_s2_START (0)
#define PMIC_DCR_V_PRE5_OUT2_dcr_v_out_pre5_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre6_s0 : 8;
    } reg;
} PMIC_DCR_V_PRE6_OUT0_UNION;
#endif
#define PMIC_DCR_V_PRE6_OUT0_dcr_v_out_pre6_s0_START (0)
#define PMIC_DCR_V_PRE6_OUT0_dcr_v_out_pre6_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre6_s1 : 8;
    } reg;
} PMIC_DCR_V_PRE6_OUT1_UNION;
#endif
#define PMIC_DCR_V_PRE6_OUT1_dcr_v_out_pre6_s1_START (0)
#define PMIC_DCR_V_PRE6_OUT1_dcr_v_out_pre6_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre6_s2 : 8;
    } reg;
} PMIC_DCR_V_PRE6_OUT2_UNION;
#endif
#define PMIC_DCR_V_PRE6_OUT2_dcr_v_out_pre6_s2_START (0)
#define PMIC_DCR_V_PRE6_OUT2_dcr_v_out_pre6_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre7_s0 : 8;
    } reg;
} PMIC_DCR_V_PRE7_OUT0_UNION;
#endif
#define PMIC_DCR_V_PRE7_OUT0_dcr_v_out_pre7_s0_START (0)
#define PMIC_DCR_V_PRE7_OUT0_dcr_v_out_pre7_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre7_s1 : 8;
    } reg;
} PMIC_DCR_V_PRE7_OUT1_UNION;
#endif
#define PMIC_DCR_V_PRE7_OUT1_dcr_v_out_pre7_s1_START (0)
#define PMIC_DCR_V_PRE7_OUT1_dcr_v_out_pre7_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre7_s2 : 8;
    } reg;
} PMIC_DCR_V_PRE7_OUT2_UNION;
#endif
#define PMIC_DCR_V_PRE7_OUT2_dcr_v_out_pre7_s2_START (0)
#define PMIC_DCR_V_PRE7_OUT2_dcr_v_out_pre7_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre8_s0 : 8;
    } reg;
} PMIC_DCR_V_PRE8_OUT0_UNION;
#endif
#define PMIC_DCR_V_PRE8_OUT0_dcr_v_out_pre8_s0_START (0)
#define PMIC_DCR_V_PRE8_OUT0_dcr_v_out_pre8_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre8_s1 : 8;
    } reg;
} PMIC_DCR_V_PRE8_OUT1_UNION;
#endif
#define PMIC_DCR_V_PRE8_OUT1_dcr_v_out_pre8_s1_START (0)
#define PMIC_DCR_V_PRE8_OUT1_dcr_v_out_pre8_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre8_s2 : 8;
    } reg;
} PMIC_DCR_V_PRE8_OUT2_UNION;
#endif
#define PMIC_DCR_V_PRE8_OUT2_dcr_v_out_pre8_s2_START (0)
#define PMIC_DCR_V_PRE8_OUT2_dcr_v_out_pre8_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre9_s0 : 8;
    } reg;
} PMIC_DCR_V_PRE9_OUT0_UNION;
#endif
#define PMIC_DCR_V_PRE9_OUT0_dcr_v_out_pre9_s0_START (0)
#define PMIC_DCR_V_PRE9_OUT0_dcr_v_out_pre9_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre9_s1 : 8;
    } reg;
} PMIC_DCR_V_PRE9_OUT1_UNION;
#endif
#define PMIC_DCR_V_PRE9_OUT1_dcr_v_out_pre9_s1_START (0)
#define PMIC_DCR_V_PRE9_OUT1_dcr_v_out_pre9_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_v_out_pre9_s2 : 8;
    } reg;
} PMIC_DCR_V_PRE9_OUT2_UNION;
#endif
#define PMIC_DCR_V_PRE9_OUT2_dcr_v_out_pre9_s2_START (0)
#define PMIC_DCR_V_PRE9_OUT2_dcr_v_out_pre9_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre0_s0 : 8;
    } reg;
} PMIC_DCR_I_PRE0_OUT0_UNION;
#endif
#define PMIC_DCR_I_PRE0_OUT0_dcr_i_out_pre0_s0_START (0)
#define PMIC_DCR_I_PRE0_OUT0_dcr_i_out_pre0_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre0_s1 : 8;
    } reg;
} PMIC_DCR_I_PRE0_OUT1_UNION;
#endif
#define PMIC_DCR_I_PRE0_OUT1_dcr_i_out_pre0_s1_START (0)
#define PMIC_DCR_I_PRE0_OUT1_dcr_i_out_pre0_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre0_s2 : 8;
    } reg;
} PMIC_DCR_I_PRE0_OUT2_UNION;
#endif
#define PMIC_DCR_I_PRE0_OUT2_dcr_i_out_pre0_s2_START (0)
#define PMIC_DCR_I_PRE0_OUT2_dcr_i_out_pre0_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre1_s0 : 8;
    } reg;
} PMIC_DCR_I_PRE1_OUT0_UNION;
#endif
#define PMIC_DCR_I_PRE1_OUT0_dcr_i_out_pre1_s0_START (0)
#define PMIC_DCR_I_PRE1_OUT0_dcr_i_out_pre1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre1_s1 : 8;
    } reg;
} PMIC_DCR_I_PRE1_OUT1_UNION;
#endif
#define PMIC_DCR_I_PRE1_OUT1_dcr_i_out_pre1_s1_START (0)
#define PMIC_DCR_I_PRE1_OUT1_dcr_i_out_pre1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre1_s2 : 8;
    } reg;
} PMIC_DCR_I_PRE1_OUT2_UNION;
#endif
#define PMIC_DCR_I_PRE1_OUT2_dcr_i_out_pre1_s2_START (0)
#define PMIC_DCR_I_PRE1_OUT2_dcr_i_out_pre1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre2_s0 : 8;
    } reg;
} PMIC_DCR_I_PRE2_OUT0_UNION;
#endif
#define PMIC_DCR_I_PRE2_OUT0_dcr_i_out_pre2_s0_START (0)
#define PMIC_DCR_I_PRE2_OUT0_dcr_i_out_pre2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre2_s1 : 8;
    } reg;
} PMIC_DCR_I_PRE2_OUT1_UNION;
#endif
#define PMIC_DCR_I_PRE2_OUT1_dcr_i_out_pre2_s1_START (0)
#define PMIC_DCR_I_PRE2_OUT1_dcr_i_out_pre2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre2_s2 : 8;
    } reg;
} PMIC_DCR_I_PRE2_OUT2_UNION;
#endif
#define PMIC_DCR_I_PRE2_OUT2_dcr_i_out_pre2_s2_START (0)
#define PMIC_DCR_I_PRE2_OUT2_dcr_i_out_pre2_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre3_s0 : 8;
    } reg;
} PMIC_DCR_I_PRE3_OUT0_UNION;
#endif
#define PMIC_DCR_I_PRE3_OUT0_dcr_i_out_pre3_s0_START (0)
#define PMIC_DCR_I_PRE3_OUT0_dcr_i_out_pre3_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre3_s1 : 8;
    } reg;
} PMIC_DCR_I_PRE3_OUT1_UNION;
#endif
#define PMIC_DCR_I_PRE3_OUT1_dcr_i_out_pre3_s1_START (0)
#define PMIC_DCR_I_PRE3_OUT1_dcr_i_out_pre3_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre3_s2 : 8;
    } reg;
} PMIC_DCR_I_PRE3_OUT2_UNION;
#endif
#define PMIC_DCR_I_PRE3_OUT2_dcr_i_out_pre3_s2_START (0)
#define PMIC_DCR_I_PRE3_OUT2_dcr_i_out_pre3_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre4_s0 : 8;
    } reg;
} PMIC_DCR_I_PRE4_OUT0_UNION;
#endif
#define PMIC_DCR_I_PRE4_OUT0_dcr_i_out_pre4_s0_START (0)
#define PMIC_DCR_I_PRE4_OUT0_dcr_i_out_pre4_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre4_s1 : 8;
    } reg;
} PMIC_DCR_I_PRE4_OUT1_UNION;
#endif
#define PMIC_DCR_I_PRE4_OUT1_dcr_i_out_pre4_s1_START (0)
#define PMIC_DCR_I_PRE4_OUT1_dcr_i_out_pre4_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre4_s2 : 8;
    } reg;
} PMIC_DCR_I_PRE4_OUT2_UNION;
#endif
#define PMIC_DCR_I_PRE4_OUT2_dcr_i_out_pre4_s2_START (0)
#define PMIC_DCR_I_PRE4_OUT2_dcr_i_out_pre4_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre5_s0 : 8;
    } reg;
} PMIC_DCR_I_PRE5_OUT0_UNION;
#endif
#define PMIC_DCR_I_PRE5_OUT0_dcr_i_out_pre5_s0_START (0)
#define PMIC_DCR_I_PRE5_OUT0_dcr_i_out_pre5_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre5_s1 : 8;
    } reg;
} PMIC_DCR_I_PRE5_OUT1_UNION;
#endif
#define PMIC_DCR_I_PRE5_OUT1_dcr_i_out_pre5_s1_START (0)
#define PMIC_DCR_I_PRE5_OUT1_dcr_i_out_pre5_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre5_s2 : 8;
    } reg;
} PMIC_DCR_I_PRE5_OUT2_UNION;
#endif
#define PMIC_DCR_I_PRE5_OUT2_dcr_i_out_pre5_s2_START (0)
#define PMIC_DCR_I_PRE5_OUT2_dcr_i_out_pre5_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre6_s0 : 8;
    } reg;
} PMIC_DCR_I_PRE6_OUT0_UNION;
#endif
#define PMIC_DCR_I_PRE6_OUT0_dcr_i_out_pre6_s0_START (0)
#define PMIC_DCR_I_PRE6_OUT0_dcr_i_out_pre6_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre6_s1 : 8;
    } reg;
} PMIC_DCR_I_PRE6_OUT1_UNION;
#endif
#define PMIC_DCR_I_PRE6_OUT1_dcr_i_out_pre6_s1_START (0)
#define PMIC_DCR_I_PRE6_OUT1_dcr_i_out_pre6_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre6_s2 : 8;
    } reg;
} PMIC_DCR_I_PRE6_OUT2_UNION;
#endif
#define PMIC_DCR_I_PRE6_OUT2_dcr_i_out_pre6_s2_START (0)
#define PMIC_DCR_I_PRE6_OUT2_dcr_i_out_pre6_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre7_s0 : 8;
    } reg;
} PMIC_DCR_I_PRE7_OUT0_UNION;
#endif
#define PMIC_DCR_I_PRE7_OUT0_dcr_i_out_pre7_s0_START (0)
#define PMIC_DCR_I_PRE7_OUT0_dcr_i_out_pre7_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre7_s1 : 8;
    } reg;
} PMIC_DCR_I_PRE7_OUT1_UNION;
#endif
#define PMIC_DCR_I_PRE7_OUT1_dcr_i_out_pre7_s1_START (0)
#define PMIC_DCR_I_PRE7_OUT1_dcr_i_out_pre7_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre7_s2 : 8;
    } reg;
} PMIC_DCR_I_PRE7_OUT2_UNION;
#endif
#define PMIC_DCR_I_PRE7_OUT2_dcr_i_out_pre7_s2_START (0)
#define PMIC_DCR_I_PRE7_OUT2_dcr_i_out_pre7_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre8_s0 : 8;
    } reg;
} PMIC_DCR_I_PRE8_OUT0_UNION;
#endif
#define PMIC_DCR_I_PRE8_OUT0_dcr_i_out_pre8_s0_START (0)
#define PMIC_DCR_I_PRE8_OUT0_dcr_i_out_pre8_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre8_s1 : 8;
    } reg;
} PMIC_DCR_I_PRE8_OUT1_UNION;
#endif
#define PMIC_DCR_I_PRE8_OUT1_dcr_i_out_pre8_s1_START (0)
#define PMIC_DCR_I_PRE8_OUT1_dcr_i_out_pre8_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre8_s2 : 8;
    } reg;
} PMIC_DCR_I_PRE8_OUT2_UNION;
#endif
#define PMIC_DCR_I_PRE8_OUT2_dcr_i_out_pre8_s2_START (0)
#define PMIC_DCR_I_PRE8_OUT2_dcr_i_out_pre8_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre9_s0 : 8;
    } reg;
} PMIC_DCR_I_PRE9_OUT0_UNION;
#endif
#define PMIC_DCR_I_PRE9_OUT0_dcr_i_out_pre9_s0_START (0)
#define PMIC_DCR_I_PRE9_OUT0_dcr_i_out_pre9_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre9_s1 : 8;
    } reg;
} PMIC_DCR_I_PRE9_OUT1_UNION;
#endif
#define PMIC_DCR_I_PRE9_OUT1_dcr_i_out_pre9_s1_START (0)
#define PMIC_DCR_I_PRE9_OUT1_dcr_i_out_pre9_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dcr_i_out_pre9_s2 : 8;
    } reg;
} PMIC_DCR_I_PRE9_OUT2_UNION;
#endif
#define PMIC_DCR_I_PRE9_OUT2_dcr_i_out_pre9_s2_START (0)
#define PMIC_DCR_I_PRE9_OUT2_dcr_i_out_pre9_s2_END (7)
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
        unsigned char ser_reserved : 3;
    } reg;
} PMIC_SER_CRC_VAULE2_UNION;
#endif
#define PMIC_SER_CRC_VAULE2_ser_crc_value2_START (0)
#define PMIC_SER_CRC_VAULE2_ser_crc_value2_END (4)
#define PMIC_SER_CRC_VAULE2_ser_reserved_START (5)
#define PMIC_SER_CRC_VAULE2_ser_reserved_END (7)
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
        unsigned char hkadc2_data11_4 : 8;
    } reg;
} PMIC_ACRADC_DATA1_UNION;
#endif
#define PMIC_ACRADC_DATA1_hkadc2_data11_4_START (0)
#define PMIC_ACRADC_DATA1_hkadc2_data11_4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char hkadc2_data3_0 : 4;
    } reg;
} PMIC_ACRADC_DATA0_UNION;
#endif
#define PMIC_ACRADC_DATA0_hkadc2_data3_0_START (4)
#define PMIC_ACRADC_DATA0_hkadc2_data3_0_END (7)
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
        unsigned char hkadc2_chopper_1st_data11_4 : 8;
    } reg;
} PMIC_ACRADC_CHOPPER_1ST_DATA1_UNION;
#endif
#define PMIC_ACRADC_CHOPPER_1ST_DATA1_hkadc2_chopper_1st_data11_4_START (0)
#define PMIC_ACRADC_CHOPPER_1ST_DATA1_hkadc2_chopper_1st_data11_4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char hkadc2_chopper_1st_data3_0 : 4;
    } reg;
} PMIC_ACRADC_CHOPPER_1ST_DATA2_UNION;
#endif
#define PMIC_ACRADC_CHOPPER_1ST_DATA2_hkadc2_chopper_1st_data3_0_START (4)
#define PMIC_ACRADC_CHOPPER_1ST_DATA2_hkadc2_chopper_1st_data3_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc2_chopper_2nd_data11_4 : 8;
    } reg;
} PMIC_ACRADC_CHOPPER_2ND_DATA1_UNION;
#endif
#define PMIC_ACRADC_CHOPPER_2ND_DATA1_hkadc2_chopper_2nd_data11_4_START (0)
#define PMIC_ACRADC_CHOPPER_2ND_DATA1_hkadc2_chopper_2nd_data11_4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char hkadc2_chopper_2nd_data3_0 : 4;
    } reg;
} PMIC_ACRADC_CHOPPER_2ND_DATA2_UNION;
#endif
#define PMIC_ACRADC_CHOPPER_2ND_DATA2_hkadc2_chopper_2nd_data3_0_START (4)
#define PMIC_ACRADC_CHOPPER_2ND_DATA2_hkadc2_chopper_2nd_data3_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hakdc2_cali_offset_cfg11_4 : 8;
    } reg;
} PMIC_ACRADC_CALIVALUE_CFG1_UNION;
#endif
#define PMIC_ACRADC_CALIVALUE_CFG1_hakdc2_cali_offset_cfg11_4_START (0)
#define PMIC_ACRADC_CALIVALUE_CFG1_hakdc2_cali_offset_cfg11_4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char hkadc2_cali_offset_cfg3_0 : 4;
    } reg;
} PMIC_ACRADC_CALIVALUE_CFG2_UNION;
#endif
#define PMIC_ACRADC_CALIVALUE_CFG2_hkadc2_cali_offset_cfg3_0_START (4)
#define PMIC_ACRADC_CALIVALUE_CFG2_hkadc2_cali_offset_cfg3_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_acr_en : 8;
    } reg;
} PMIC_ACR_EN_UNION;
#endif
#define PMIC_ACR_EN_sc_acr_en_START (0)
#define PMIC_ACR_EN_sc_acr_en_END (7)
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
        unsigned char acr_data0_h : 8;
    } reg;
} PMIC_ACR_DATA0_H_UNION;
#endif
#define PMIC_ACR_DATA0_H_acr_data0_h_START (0)
#define PMIC_ACR_DATA0_H_acr_data0_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char acr_data0_l : 4;
    } reg;
} PMIC_ACR_DATA0_L_UNION;
#endif
#define PMIC_ACR_DATA0_L_acr_data0_l_START (4)
#define PMIC_ACR_DATA0_L_acr_data0_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data1_h : 8;
    } reg;
} PMIC_ACR_DATA1_H_UNION;
#endif
#define PMIC_ACR_DATA1_H_acr_data1_h_START (0)
#define PMIC_ACR_DATA1_H_acr_data1_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char acr_data1_l : 4;
    } reg;
} PMIC_ACR_DATA1_L_UNION;
#endif
#define PMIC_ACR_DATA1_L_acr_data1_l_START (4)
#define PMIC_ACR_DATA1_L_acr_data1_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data2_h : 8;
    } reg;
} PMIC_ACR_DATA2_H_UNION;
#endif
#define PMIC_ACR_DATA2_H_acr_data2_h_START (0)
#define PMIC_ACR_DATA2_H_acr_data2_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char acr_data2_l : 4;
    } reg;
} PMIC_ACR_DATA2_L_UNION;
#endif
#define PMIC_ACR_DATA2_L_acr_data2_l_START (4)
#define PMIC_ACR_DATA2_L_acr_data2_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data3_h : 8;
    } reg;
} PMIC_ACR_DATA3_H_UNION;
#endif
#define PMIC_ACR_DATA3_H_acr_data3_h_START (0)
#define PMIC_ACR_DATA3_H_acr_data3_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char acr_data3_l : 4;
    } reg;
} PMIC_ACR_DATA3_L_UNION;
#endif
#define PMIC_ACR_DATA3_L_acr_data3_l_START (4)
#define PMIC_ACR_DATA3_L_acr_data3_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data4_h : 8;
    } reg;
} PMIC_ACR_DATA4_H_UNION;
#endif
#define PMIC_ACR_DATA4_H_acr_data4_h_START (0)
#define PMIC_ACR_DATA4_H_acr_data4_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char acr_data4_l : 4;
    } reg;
} PMIC_ACR_DATA4_L_UNION;
#endif
#define PMIC_ACR_DATA4_L_acr_data4_l_START (4)
#define PMIC_ACR_DATA4_L_acr_data4_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data5_h : 8;
    } reg;
} PMIC_ACR_DATA5_H_UNION;
#endif
#define PMIC_ACR_DATA5_H_acr_data5_h_START (0)
#define PMIC_ACR_DATA5_H_acr_data5_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char acr_data5_l : 4;
    } reg;
} PMIC_ACR_DATA5_L_UNION;
#endif
#define PMIC_ACR_DATA5_L_acr_data5_l_START (4)
#define PMIC_ACR_DATA5_L_acr_data5_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data6_h : 8;
    } reg;
} PMIC_ACR_DATA6_H_UNION;
#endif
#define PMIC_ACR_DATA6_H_acr_data6_h_START (0)
#define PMIC_ACR_DATA6_H_acr_data6_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char acr_data6_l : 4;
    } reg;
} PMIC_ACR_DATA6_L_UNION;
#endif
#define PMIC_ACR_DATA6_L_acr_data6_l_START (4)
#define PMIC_ACR_DATA6_L_acr_data6_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data7_h : 8;
    } reg;
} PMIC_ACR_DATA7_H_UNION;
#endif
#define PMIC_ACR_DATA7_H_acr_data7_h_START (0)
#define PMIC_ACR_DATA7_H_acr_data7_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char acr_data7_l : 4;
    } reg;
} PMIC_ACR_DATA7_L_UNION;
#endif
#define PMIC_ACR_DATA7_L_acr_data7_l_START (4)
#define PMIC_ACR_DATA7_L_acr_data7_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_en_sohov : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_SOH_OVP_EN_UNION;
#endif
#define PMIC_SOH_OVP_EN_sc_en_sohov_START (0)
#define PMIC_SOH_OVP_EN_sc_en_sohov_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_soh_ovh_mask : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_SOH_OVH_MASK_UNION;
#endif
#define PMIC_SOH_OVH_MASK_sc_soh_ovh_mask_START (0)
#define PMIC_SOH_OVH_MASK_sc_soh_ovh_mask_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_sohov_time : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_SOHOV_TIME_SOHEN_UNION;
#endif
#define PMIC_SOHOV_TIME_SOHEN_sc_sohov_time_START (0)
#define PMIC_SOHOV_TIME_SOHEN_sc_sohov_time_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_ensoh_dischg : 8;
    } reg;
} PMIC_EN_SOHOV_UNION;
#endif
#define PMIC_EN_SOHOV_sc_ensoh_dischg_START (0)
#define PMIC_EN_SOHOV_sc_ensoh_dischg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_soh_ovh_th0_h : 8;
    } reg;
} PMIC_SOH_OVH_TH0_H_UNION;
#endif
#define PMIC_SOH_OVH_TH0_H_sc_soh_ovh_th0_h_START (0)
#define PMIC_SOH_OVH_TH0_H_sc_soh_ovh_th0_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char sc_soh_ovh_th0_l : 4;
    } reg;
} PMIC_SOH_OVH_TH0_L_UNION;
#endif
#define PMIC_SOH_OVH_TH0_L_sc_soh_ovh_th0_l_START (4)
#define PMIC_SOH_OVH_TH0_L_sc_soh_ovh_th0_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_tmp_ovh_th0_h : 8;
    } reg;
} PMIC_TMP_OVH_TH0_H_UNION;
#endif
#define PMIC_TMP_OVH_TH0_H_sc_tmp_ovh_th0_h_START (0)
#define PMIC_TMP_OVH_TH0_H_sc_tmp_ovh_th0_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char sc_tmp_ovh_th0_l : 4;
    } reg;
} PMIC_TMP_OVH_TH0_L_UNION;
#endif
#define PMIC_TMP_OVH_TH0_L_sc_tmp_ovh_th0_l_START (4)
#define PMIC_TMP_OVH_TH0_L_sc_tmp_ovh_th0_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_soh_ovh_th1_h : 8;
    } reg;
} PMIC_SOH_OVH_TH1_H_UNION;
#endif
#define PMIC_SOH_OVH_TH1_H_sc_soh_ovh_th1_h_START (0)
#define PMIC_SOH_OVH_TH1_H_sc_soh_ovh_th1_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char sc_soh_ovh_th1_l : 4;
    } reg;
} PMIC_SOH_OVH_TH1_L_UNION;
#endif
#define PMIC_SOH_OVH_TH1_L_sc_soh_ovh_th1_l_START (4)
#define PMIC_SOH_OVH_TH1_L_sc_soh_ovh_th1_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_tmp_ovh_th1_h : 8;
    } reg;
} PMIC_TMP_OVH_TH1_H_UNION;
#endif
#define PMIC_TMP_OVH_TH1_H_sc_tmp_ovh_th1_h_START (0)
#define PMIC_TMP_OVH_TH1_H_sc_tmp_ovh_th1_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char sc_tmp_ovh_th1_l : 4;
    } reg;
} PMIC_TMP_OVH_TH1_L_UNION;
#endif
#define PMIC_TMP_OVH_TH1_L_sc_tmp_ovh_th1_l_START (4)
#define PMIC_TMP_OVH_TH1_L_sc_tmp_ovh_th1_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_soh_ovh_th2_h : 8;
    } reg;
} PMIC_SOH_OVH_TH2_H_UNION;
#endif
#define PMIC_SOH_OVH_TH2_H_sc_soh_ovh_th2_h_START (0)
#define PMIC_SOH_OVH_TH2_H_sc_soh_ovh_th2_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char sc_soh_ovh_th2_l : 4;
    } reg;
} PMIC_SOH_OVH_TH2_L_UNION;
#endif
#define PMIC_SOH_OVH_TH2_L_sc_soh_ovh_th2_l_START (4)
#define PMIC_SOH_OVH_TH2_L_sc_soh_ovh_th2_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_tmp_ovh_th2_h : 8;
    } reg;
} PMIC_TMP_OVH_TH2_H_UNION;
#endif
#define PMIC_TMP_OVH_TH2_H_sc_tmp_ovh_th2_h_START (0)
#define PMIC_TMP_OVH_TH2_H_sc_tmp_ovh_th2_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char sc_tmp_ovh_th2_l : 4;
    } reg;
} PMIC_TMP_OVH_TH2_L_UNION;
#endif
#define PMIC_TMP_OVH_TH2_L_sc_tmp_ovh_th2_l_START (4)
#define PMIC_TMP_OVH_TH2_L_sc_tmp_ovh_th2_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_soh_ovl_th_h : 8;
    } reg;
} PMIC_SOH_OVL_TH_H_UNION;
#endif
#define PMIC_SOH_OVL_TH_H_sc_soh_ovl_th_h_START (0)
#define PMIC_SOH_OVL_TH_H_sc_soh_ovl_th_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char sc_soh_ovl_th_l : 4;
    } reg;
} PMIC_SOH_OVL_TH_L_UNION;
#endif
#define PMIC_SOH_OVL_TH_L_sc_soh_ovl_th_l_START (4)
#define PMIC_SOH_OVL_TH_L_sc_soh_ovl_th_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_tmp_ovl_th_h : 8;
    } reg;
} PMIC_TMP_OVL_TH_H_UNION;
#endif
#define PMIC_TMP_OVL_TH_H_sc_tmp_ovl_th_h_START (0)
#define PMIC_TMP_OVL_TH_H_sc_tmp_ovl_th_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 4;
        unsigned char sc_tmp_ovl_th_l : 4;
    } reg;
} PMIC_TMP_OVL_TH_L_UNION;
#endif
#define PMIC_TMP_OVL_TH_L_sc_tmp_ovl_th_l_START (4)
#define PMIC_TMP_OVL_TH_L_sc_tmp_ovl_th_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char st_tmp_ovl : 1;
        unsigned char st_soh_ovl : 1;
        unsigned char st_tmp_ovh_2 : 1;
        unsigned char st_soh_ovh_2 : 1;
        unsigned char st_tmp_ovh_1 : 1;
        unsigned char st_soh_ovh_1 : 1;
        unsigned char st_tmp_ovh_0 : 1;
        unsigned char st_soh_ovh_0 : 1;
    } reg;
} PMIC_SOH_OVP_REAL_UNION;
#endif
#define PMIC_SOH_OVP_REAL_st_tmp_ovl_START (0)
#define PMIC_SOH_OVP_REAL_st_tmp_ovl_END (0)
#define PMIC_SOH_OVP_REAL_st_soh_ovl_START (1)
#define PMIC_SOH_OVP_REAL_st_soh_ovl_END (1)
#define PMIC_SOH_OVP_REAL_st_tmp_ovh_2_START (2)
#define PMIC_SOH_OVP_REAL_st_tmp_ovh_2_END (2)
#define PMIC_SOH_OVP_REAL_st_soh_ovh_2_START (3)
#define PMIC_SOH_OVP_REAL_st_soh_ovh_2_END (3)
#define PMIC_SOH_OVP_REAL_st_tmp_ovh_1_START (4)
#define PMIC_SOH_OVP_REAL_st_tmp_ovh_1_END (4)
#define PMIC_SOH_OVP_REAL_st_soh_ovh_1_START (5)
#define PMIC_SOH_OVP_REAL_st_soh_ovh_1_END (5)
#define PMIC_SOH_OVP_REAL_st_tmp_ovh_0_START (6)
#define PMIC_SOH_OVP_REAL_st_tmp_ovh_0_END (6)
#define PMIC_SOH_OVP_REAL_st_soh_ovh_0_START (7)
#define PMIC_SOH_OVP_REAL_st_soh_ovh_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_dcr_en : 8;
    } reg;
} PMIC_DCR_EN_UNION;
#endif
#define PMIC_DCR_EN_sc_dcr_en_START (0)
#define PMIC_DCR_EN_sc_dcr_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_dcr_timer : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_DCR_TIMER_UNION;
#endif
#define PMIC_DCR_TIMER_sc_dcr_timer_START (0)
#define PMIC_DCR_TIMER_sc_dcr_timer_END (1)
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
} PMIC_TEST_BUS_SEL_UNION;
#endif
#define PMIC_TEST_BUS_SEL_sc_acr_tb_sel_START (0)
#define PMIC_TEST_BUS_SEL_sc_acr_tb_sel_END (3)
#define PMIC_TEST_BUS_SEL_sc_acr_tb_en_START (4)
#define PMIC_TEST_BUS_SEL_sc_acr_tb_en_END (4)
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
    unsigned char value;
    struct
    {
        unsigned char reg_lra_en : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_LRA_CTRL_UNION;
#endif
#define PMIC_LRA_CTRL_reg_lra_en_START (0)
#define PMIC_LRA_CTRL_reg_lra_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_brk_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_BRAKE_CTRL_UNION;
#endif
#define PMIC_BRAKE_CTRL_reg_brk_en_START (0)
#define PMIC_BRAKE_CTRL_reg_brk_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ovd_low_num : 3;
        unsigned char ovd_high_num : 5;
    } reg;
} PMIC_DRV_H_L_NUM_UNION;
#endif
#define PMIC_DRV_H_L_NUM_ovd_low_num_START (0)
#define PMIC_DRV_H_L_NUM_ovd_low_num_END (2)
#define PMIC_DRV_H_L_NUM_ovd_high_num_START (3)
#define PMIC_DRV_H_L_NUM_ovd_high_num_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char brk_high_num : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BRK_H_NUM_UNION;
#endif
#define PMIC_BRK_H_NUM_brk_high_num_START (0)
#define PMIC_BRK_H_NUM_brk_high_num_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char adc_ave_cfg : 3;
        unsigned char max_dete_num : 1;
        unsigned char th_rgt_num : 1;
        unsigned char lra_ocp_th : 3;
    } reg;
} PMIC_LRA_CFG0_UNION;
#endif
#define PMIC_LRA_CFG0_adc_ave_cfg_START (0)
#define PMIC_LRA_CFG0_adc_ave_cfg_END (2)
#define PMIC_LRA_CFG0_max_dete_num_START (3)
#define PMIC_LRA_CFG0_max_dete_num_END (3)
#define PMIC_LRA_CFG0_th_rgt_num_START (4)
#define PMIC_LRA_CFG0_th_rgt_num_END (4)
#define PMIC_LRA_CFG0_lra_ocp_th_START (5)
#define PMIC_LRA_CFG0_lra_ocp_th_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char drv_step_time : 4;
        unsigned char bemf_wait_time : 3;
        unsigned char reserved : 1;
    } reg;
} PMIC_LRA_CFG1_UNION;
#endif
#define PMIC_LRA_CFG1_drv_step_time_START (0)
#define PMIC_LRA_CFG1_drv_step_time_END (3)
#define PMIC_LRA_CFG1_bemf_wait_time_START (4)
#define PMIC_LRA_CFG1_bemf_wait_time_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fst_ovdr_time : 8;
    } reg;
} PMIC_DRV_TIME_CFG_UNION;
#endif
#define PMIC_DRV_TIME_CFG_fst_ovdr_time_START (0)
#define PMIC_DRV_TIME_CFG_fst_ovdr_time_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char drv_max_time : 8;
    } reg;
} PMIC_MAX_TIME_CFG_UNION;
#endif
#define PMIC_MAX_TIME_CFG_drv_max_time_START (0)
#define PMIC_MAX_TIME_CFG_drv_max_time_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char bemf_out_time : 8;
    } reg;
} PMIC_BEMF_OUT_CFG_UNION;
#endif
#define PMIC_BEMF_OUT_CFG_bemf_out_time_START (0)
#define PMIC_BEMF_OUT_CFG_bemf_out_time_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lra_bemf_time : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_BEMF_TIME_UNION;
#endif
#define PMIC_BEMF_TIME_lra_bemf_time_START (0)
#define PMIC_BEMF_TIME_lra_bemf_time_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char duty_init_num_l : 8;
    } reg;
} PMIC_DUTY_INIT_CFG_LOW_UNION;
#endif
#define PMIC_DUTY_INIT_CFG_LOW_duty_init_num_l_START (0)
#define PMIC_DUTY_INIT_CFG_LOW_duty_init_num_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char duty_init_num_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_DUTY_INIT_CFG_HIGH_UNION;
#endif
#define PMIC_DUTY_INIT_CFG_HIGH_duty_init_num_h_START (0)
#define PMIC_DUTY_INIT_CFG_HIGH_duty_init_num_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char duty_ovdr_cps : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_DUTY_OVDR_CPS_UNION;
#endif
#define PMIC_DUTY_OVDR_CPS_duty_ovdr_cps_START (0)
#define PMIC_DUTY_OVDR_CPS_duty_ovdr_cps_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char duty_ovdr_num_l : 8;
    } reg;
} PMIC_DUTY_OVDR_CFG_LOW_UNION;
#endif
#define PMIC_DUTY_OVDR_CFG_LOW_duty_ovdr_num_l_START (0)
#define PMIC_DUTY_OVDR_CFG_LOW_duty_ovdr_num_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char duty_ovdr_num_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_DUTY_OVDR_CFG_HIGH_UNION;
#endif
#define PMIC_DUTY_OVDR_CFG_HIGH_duty_ovdr_num_h_START (0)
#define PMIC_DUTY_OVDR_CFG_HIGH_duty_ovdr_num_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char duty_nml_cps : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_DUTY_NORMAL_CPS_UNION;
#endif
#define PMIC_DUTY_NORMAL_CPS_duty_nml_cps_START (0)
#define PMIC_DUTY_NORMAL_CPS_duty_nml_cps_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char duty_nml_num_l : 8;
    } reg;
} PMIC_DUTY_NORMAL_CFG_LOW_UNION;
#endif
#define PMIC_DUTY_NORMAL_CFG_LOW_duty_nml_num_l_START (0)
#define PMIC_DUTY_NORMAL_CFG_LOW_duty_nml_num_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char duty_nml_num_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_DUTY_NORMAL_CFG_HIGH_UNION;
#endif
#define PMIC_DUTY_NORMAL_CFG_HIGH_duty_nml_num_h_START (0)
#define PMIC_DUTY_NORMAL_CFG_HIGH_duty_nml_num_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char duty_brk_cps : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_DUTY_BRAKE_CPS_UNION;
#endif
#define PMIC_DUTY_BRAKE_CPS_duty_brk_cps_START (0)
#define PMIC_DUTY_BRAKE_CPS_duty_brk_cps_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char duty_brk_num_l : 8;
    } reg;
} PMIC_DUTY_BRAKE_CFG_LOW_UNION;
#endif
#define PMIC_DUTY_BRAKE_CFG_LOW_duty_brk_num_l_START (0)
#define PMIC_DUTY_BRAKE_CFG_LOW_duty_brk_num_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char duty_brk_num_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_DUTY_BRAKE_CFG_HIGH_UNION;
#endif
#define PMIC_DUTY_BRAKE_CFG_HIGH_duty_brk_num_h_START (0)
#define PMIC_DUTY_BRAKE_CFG_HIGH_duty_brk_num_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eob_k_1_l : 8;
    } reg;
} PMIC_EOB_K_1_LOW_UNION;
#endif
#define PMIC_EOB_K_1_LOW_eob_k_1_l_START (0)
#define PMIC_EOB_K_1_LOW_eob_k_1_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eob_k_1_h : 8;
    } reg;
} PMIC_EOB_K_1_HIGH_UNION;
#endif
#define PMIC_EOB_K_1_HIGH_eob_k_1_h_START (0)
#define PMIC_EOB_K_1_HIGH_eob_k_1_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eob_k_2_l : 8;
    } reg;
} PMIC_EOB_K_2_LOW_UNION;
#endif
#define PMIC_EOB_K_2_LOW_eob_k_2_l_START (0)
#define PMIC_EOB_K_2_LOW_eob_k_2_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eob_k_2_h : 8;
    } reg;
} PMIC_EOB_K_2_HIGH_UNION;
#endif
#define PMIC_EOB_K_2_HIGH_eob_k_2_h_START (0)
#define PMIC_EOB_K_2_HIGH_eob_k_2_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char max_min_th : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_ADC_MAX_MIN_TH_UNION;
#endif
#define PMIC_ADC_MAX_MIN_TH_max_min_th_START (0)
#define PMIC_ADC_MAX_MIN_TH_max_min_th_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char th_boost_v_l : 8;
    } reg;
} PMIC_TH_BOOST_V_CFG_LOW_UNION;
#endif
#define PMIC_TH_BOOST_V_CFG_LOW_th_boost_v_l_START (0)
#define PMIC_TH_BOOST_V_CFG_LOW_th_boost_v_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char th_boost_v_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_TH_BOOST_V_CFG_HIGH_UNION;
#endif
#define PMIC_TH_BOOST_V_CFG_HIGH_th_boost_v_h_START (0)
#define PMIC_TH_BOOST_V_CFG_HIGH_th_boost_v_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char th_gate1_l : 8;
    } reg;
} PMIC_GATE_VOLTAGE1_LOW_UNION;
#endif
#define PMIC_GATE_VOLTAGE1_LOW_th_gate1_l_START (0)
#define PMIC_GATE_VOLTAGE1_LOW_th_gate1_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char th_gate1_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_GATE_VOLTAGE1_HIGH_UNION;
#endif
#define PMIC_GATE_VOLTAGE1_HIGH_th_gate1_h_START (0)
#define PMIC_GATE_VOLTAGE1_HIGH_th_gate1_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char th_gate2_l : 8;
    } reg;
} PMIC_GATE_VOLTAGE2_LOW_UNION;
#endif
#define PMIC_GATE_VOLTAGE2_LOW_th_gate2_l_START (0)
#define PMIC_GATE_VOLTAGE2_LOW_th_gate2_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char th_gate2_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_GATE_VOLTAGE2_HIGH_UNION;
#endif
#define PMIC_GATE_VOLTAGE2_HIGH_th_gate2_h_START (0)
#define PMIC_GATE_VOLTAGE2_HIGH_th_gate2_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char th_gate3_l : 8;
    } reg;
} PMIC_GATE_VOLTAGE3_LOW_UNION;
#endif
#define PMIC_GATE_VOLTAGE3_LOW_th_gate3_l_START (0)
#define PMIC_GATE_VOLTAGE3_LOW_th_gate3_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char th_gate3_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_GATE_VOLTAGE3_HIGH_UNION;
#endif
#define PMIC_GATE_VOLTAGE3_HIGH_th_gate3_h_START (0)
#define PMIC_GATE_VOLTAGE3_HIGH_th_gate3_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char brk_bemf_min_th_l : 8;
    } reg;
} PMIC_BRK_BEMF_MIN_CFG_L_UNION;
#endif
#define PMIC_BRK_BEMF_MIN_CFG_L_brk_bemf_min_th_l_START (0)
#define PMIC_BRK_BEMF_MIN_CFG_L_brk_bemf_min_th_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char brk_bemf_min_th_h : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_BRK_BEMF_MIN_CFG_H_UNION;
#endif
#define PMIC_BRK_BEMF_MIN_CFG_H_brk_bemf_min_th_h_START (0)
#define PMIC_BRK_BEMF_MIN_CFG_H_brk_bemf_min_th_h_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ref_u_l : 8;
    } reg;
} PMIC_REF_U_CFG_L_UNION;
#endif
#define PMIC_REF_U_CFG_L_ref_u_l_START (0)
#define PMIC_REF_U_CFG_L_ref_u_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ref_u_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_REF_U_CFG_H_UNION;
#endif
#define PMIC_REF_U_CFG_H_ref_u_h_START (0)
#define PMIC_REF_U_CFG_H_ref_u_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char kfit_sel : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_KFIT_DATA_SEL_UNION;
#endif
#define PMIC_KFIT_DATA_SEL_kfit_sel_START (0)
#define PMIC_KFIT_DATA_SEL_kfit_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data_k_rvs_l : 8;
    } reg;
} PMIC_K_RVS_LOW_UNION;
#endif
#define PMIC_K_RVS_LOW_data_k_rvs_l_START (0)
#define PMIC_K_RVS_LOW_data_k_rvs_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data_k_rvs_h : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_K_RVS_HIGH_UNION;
#endif
#define PMIC_K_RVS_HIGH_data_k_rvs_h_START (0)
#define PMIC_K_RVS_HIGH_data_k_rvs_h_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data_b_rvs_l : 8;
    } reg;
} PMIC_B_RVS_LOW_UNION;
#endif
#define PMIC_B_RVS_LOW_data_b_rvs_l_START (0)
#define PMIC_B_RVS_LOW_data_b_rvs_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data_b_rvs_h : 8;
    } reg;
} PMIC_B_RVS_HIGH_UNION;
#endif
#define PMIC_B_RVS_HIGH_data_b_rvs_h_START (0)
#define PMIC_B_RVS_HIGH_data_b_rvs_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ladd_para_1 : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_LADD_PARA1_CFG_UNION;
#endif
#define PMIC_LADD_PARA1_CFG_ladd_para_1_START (0)
#define PMIC_LADD_PARA1_CFG_ladd_para_1_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ladd_para_2 : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_LADD_PARA2_CFG_UNION;
#endif
#define PMIC_LADD_PARA2_CFG_ladd_para_2_START (0)
#define PMIC_LADD_PARA2_CFG_ladd_para_2_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ladd_para_3 : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_LADD_PARA3_CFG_UNION;
#endif
#define PMIC_LADD_PARA3_CFG_ladd_para_3_START (0)
#define PMIC_LADD_PARA3_CFG_ladd_para_3_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ladd_para_4 : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_LADD_PARA4_CFG_UNION;
#endif
#define PMIC_LADD_PARA4_CFG_ladd_para_4_START (0)
#define PMIC_LADD_PARA4_CFG_ladd_para_4_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ladd_para_5 : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_LADD_PARA5_CFG_UNION;
#endif
#define PMIC_LADD_PARA5_CFG_ladd_para_5_START (0)
#define PMIC_LADD_PARA5_CFG_ladd_para_5_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ladd_para_6 : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_LADD_PARA6_CFG_UNION;
#endif
#define PMIC_LADD_PARA6_CFG_ladd_para_6_START (0)
#define PMIC_LADD_PARA6_CFG_ladd_para_6_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ladd_para_7 : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_LADD_PARA7_CFG_UNION;
#endif
#define PMIC_LADD_PARA7_CFG_ladd_para_7_START (0)
#define PMIC_LADD_PARA7_CFG_ladd_para_7_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ladd_para_8 : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_LADD_PARA8_CFG_UNION;
#endif
#define PMIC_LADD_PARA8_CFG_ladd_para_8_START (0)
#define PMIC_LADD_PARA8_CFG_ladd_para_8_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ladd_para_9 : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_LADD_PARA9_CFG_UNION;
#endif
#define PMIC_LADD_PARA9_CFG_ladd_para_9_START (0)
#define PMIC_LADD_PARA9_CFG_ladd_para_9_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ladd_para_10 : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_LADD_PARA10_CFG_UNION;
#endif
#define PMIC_LADD_PARA10_CFG_ladd_para_10_START (0)
#define PMIC_LADD_PARA10_CFG_ladd_para_10_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ladd_para_11 : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_LADD_PARA11_CFG_UNION;
#endif
#define PMIC_LADD_PARA11_CFG_ladd_para_11_START (0)
#define PMIC_LADD_PARA11_CFG_ladd_para_11_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ladd_para_12 : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_LADD_PARA12_CFG_UNION;
#endif
#define PMIC_LADD_PARA12_CFG_ladd_para_12_START (0)
#define PMIC_LADD_PARA12_CFG_ladd_para_12_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ladd_para_13 : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_LADD_PARA13_CFG_UNION;
#endif
#define PMIC_LADD_PARA13_CFG_ladd_para_13_START (0)
#define PMIC_LADD_PARA13_CFG_ladd_para_13_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ladd_para_14 : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_LADD_PARA14_CFG_UNION;
#endif
#define PMIC_LADD_PARA14_CFG_ladd_para_14_START (0)
#define PMIC_LADD_PARA14_CFG_ladd_para_14_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ladd_para_15 : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_LADD_PARA15_CFG_UNION;
#endif
#define PMIC_LADD_PARA15_CFG_ladd_para_15_START (0)
#define PMIC_LADD_PARA15_CFG_ladd_para_15_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ladd_para_16 : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_LADD_PARA16_CFG_UNION;
#endif
#define PMIC_LADD_PARA16_CFG_ladd_para_16_START (0)
#define PMIC_LADD_PARA16_CFG_ladd_para_16_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ladd_para_17 : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_LADD_PARA17_CFG_UNION;
#endif
#define PMIC_LADD_PARA17_CFG_ladd_para_17_START (0)
#define PMIC_LADD_PARA17_CFG_ladd_para_17_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ladd_para_18 : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_LADD_PARA18_CFG_UNION;
#endif
#define PMIC_LADD_PARA18_CFG_ladd_para_18_START (0)
#define PMIC_LADD_PARA18_CFG_ladd_para_18_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ladd_para_19 : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_LADD_PARA19_CFG_UNION;
#endif
#define PMIC_LADD_PARA19_CFG_ladd_para_19_START (0)
#define PMIC_LADD_PARA19_CFG_ladd_para_19_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ladd_para_20 : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_LADD_PARA20_CFG_UNION;
#endif
#define PMIC_LADD_PARA20_CFG_ladd_para_20_START (0)
#define PMIC_LADD_PARA20_CFG_ladd_para_20_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ap_out_brk_bemf_l : 8;
    } reg;
} PMIC_OUT_BEMF_VOL_L_UNION;
#endif
#define PMIC_OUT_BEMF_VOL_L_ap_out_brk_bemf_l_START (0)
#define PMIC_OUT_BEMF_VOL_L_ap_out_brk_bemf_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ap_out_brk_bemf_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_OUT_BEMF_VOL_H_UNION;
#endif
#define PMIC_OUT_BEMF_VOL_H_ap_out_brk_bemf_h_START (0)
#define PMIC_OUT_BEMF_VOL_H_ap_out_brk_bemf_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ap_out_nml_bemf_l : 8;
    } reg;
} PMIC_OUT_NML_VBEMF_L_UNION;
#endif
#define PMIC_OUT_NML_VBEMF_L_ap_out_nml_bemf_l_START (0)
#define PMIC_OUT_NML_VBEMF_L_ap_out_nml_bemf_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ap_out_nml_bemf_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_OUT_NML_VBEMF_H_UNION;
#endif
#define PMIC_OUT_NML_VBEMF_H_ap_out_nml_bemf_h_START (0)
#define PMIC_OUT_NML_VBEMF_H_ap_out_nml_bemf_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ap_ovdr_hprd : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_OV_BRK_HPRIOD_UNION;
#endif
#define PMIC_OV_BRK_HPRIOD_ap_ovdr_hprd_START (0)
#define PMIC_OV_BRK_HPRIOD_ap_ovdr_hprd_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ap_brk_hprd : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BRK_HPRIOD_UNION;
#endif
#define PMIC_BRK_HPRIOD_ap_brk_hprd_START (0)
#define PMIC_BRK_HPRIOD_ap_brk_hprd_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ap_real_u_l : 8;
    } reg;
} PMIC_UREAL_DATA_L_UNION;
#endif
#define PMIC_UREAL_DATA_L_ap_real_u_l_START (0)
#define PMIC_UREAL_DATA_L_ap_real_u_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ap_real_u_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_UREAL_DATA_H_UNION;
#endif
#define PMIC_UREAL_DATA_H_ap_real_u_h_START (0)
#define PMIC_UREAL_DATA_H_ap_real_u_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ap_adc_max_l : 8;
    } reg;
} PMIC_ADC_CALI_MAX_L_UNION;
#endif
#define PMIC_ADC_CALI_MAX_L_ap_adc_max_l_START (0)
#define PMIC_ADC_CALI_MAX_L_ap_adc_max_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ap_adc_max_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_ADC_CALI_MAX_H_UNION;
#endif
#define PMIC_ADC_CALI_MAX_H_ap_adc_max_h_START (0)
#define PMIC_ADC_CALI_MAX_H_ap_adc_max_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ap_adc_min_l : 8;
    } reg;
} PMIC_ADC_CALI_MIN_L_UNION;
#endif
#define PMIC_ADC_CALI_MIN_L_ap_adc_min_l_START (0)
#define PMIC_ADC_CALI_MIN_L_ap_adc_min_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ap_adc_min_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_ADC_CALI_MIN_H_UNION;
#endif
#define PMIC_ADC_CALI_MIN_H_ap_adc_min_h_START (0)
#define PMIC_ADC_CALI_MIN_H_ap_adc_min_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ap_adc_zero_l : 8;
    } reg;
} PMIC_ADC_ZERO_L_UNION;
#endif
#define PMIC_ADC_ZERO_L_ap_adc_zero_l_START (0)
#define PMIC_ADC_ZERO_L_ap_adc_zero_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ap_adc_zero_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_ADC_ZERO_H_UNION;
#endif
#define PMIC_ADC_ZERO_H_ap_adc_zero_h_START (0)
#define PMIC_ADC_ZERO_H_ap_adc_zero_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ap_kfit_data_l : 8;
    } reg;
} PMIC_KFIT_DATA_L_UNION;
#endif
#define PMIC_KFIT_DATA_L_ap_kfit_data_l_START (0)
#define PMIC_KFIT_DATA_L_ap_kfit_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ap_kfit_data_h : 8;
    } reg;
} PMIC_KFIT_DATA_H_UNION;
#endif
#define PMIC_KFIT_DATA_H_ap_kfit_data_h_START (0)
#define PMIC_KFIT_DATA_H_ap_kfit_data_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char st_fsm_lra : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_FSM_STATE_UNION;
#endif
#define PMIC_FSM_STATE_st_fsm_lra_START (0)
#define PMIC_FSM_STATE_st_fsm_lra_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_drv_time_1 : 8;
    } reg;
} PMIC_SPEL_TIME_1_UNION;
#endif
#define PMIC_SPEL_TIME_1_spel_drv_time_1_START (0)
#define PMIC_SPEL_TIME_1_spel_drv_time_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_drv_time_2 : 8;
    } reg;
} PMIC_SPEL_TIME_2_UNION;
#endif
#define PMIC_SPEL_TIME_2_spel_drv_time_2_START (0)
#define PMIC_SPEL_TIME_2_spel_drv_time_2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_drv_time_3 : 8;
    } reg;
} PMIC_SPEL_TIME_3_UNION;
#endif
#define PMIC_SPEL_TIME_3_spel_drv_time_3_START (0)
#define PMIC_SPEL_TIME_3_spel_drv_time_3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_drv_time_4 : 8;
    } reg;
} PMIC_SPEL_TIME_4_UNION;
#endif
#define PMIC_SPEL_TIME_4_spel_drv_time_4_START (0)
#define PMIC_SPEL_TIME_4_spel_drv_time_4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_drv_time_5 : 8;
    } reg;
} PMIC_SPEL_TIME_5_UNION;
#endif
#define PMIC_SPEL_TIME_5_spel_drv_time_5_START (0)
#define PMIC_SPEL_TIME_5_spel_drv_time_5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_drv_time_6 : 8;
    } reg;
} PMIC_SPEL_TIME_6_UNION;
#endif
#define PMIC_SPEL_TIME_6_spel_drv_time_6_START (0)
#define PMIC_SPEL_TIME_6_spel_drv_time_6_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_drv_time_7 : 8;
    } reg;
} PMIC_SPEL_TIME_7_UNION;
#endif
#define PMIC_SPEL_TIME_7_spel_drv_time_7_START (0)
#define PMIC_SPEL_TIME_7_spel_drv_time_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_drv_time_8 : 8;
    } reg;
} PMIC_SPEL_TIME_8_UNION;
#endif
#define PMIC_SPEL_TIME_8_spel_drv_time_8_START (0)
#define PMIC_SPEL_TIME_8_spel_drv_time_8_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_drv_time_9 : 8;
    } reg;
} PMIC_SPEL_TIME_9_UNION;
#endif
#define PMIC_SPEL_TIME_9_spel_drv_time_9_START (0)
#define PMIC_SPEL_TIME_9_spel_drv_time_9_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_drv_time_10 : 8;
    } reg;
} PMIC_SPEL_TIME_10_UNION;
#endif
#define PMIC_SPEL_TIME_10_spel_drv_time_10_START (0)
#define PMIC_SPEL_TIME_10_spel_drv_time_10_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char duty_spel_cps : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_SPEL_DUTY_CPS_UNION;
#endif
#define PMIC_SPEL_DUTY_CPS_duty_spel_cps_START (0)
#define PMIC_SPEL_DUTY_CPS_duty_spel_cps_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_duty_1_l : 8;
    } reg;
} PMIC_SPEL_DUTY_1_L_UNION;
#endif
#define PMIC_SPEL_DUTY_1_L_spel_duty_1_l_START (0)
#define PMIC_SPEL_DUTY_1_L_spel_duty_1_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_duty_1_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_SPEL_DUTY_1_H_UNION;
#endif
#define PMIC_SPEL_DUTY_1_H_spel_duty_1_h_START (0)
#define PMIC_SPEL_DUTY_1_H_spel_duty_1_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_duty_2_l : 8;
    } reg;
} PMIC_SPEL_DUTY_2_L_UNION;
#endif
#define PMIC_SPEL_DUTY_2_L_spel_duty_2_l_START (0)
#define PMIC_SPEL_DUTY_2_L_spel_duty_2_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_duty_2_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_SPEL_DUTY_2_H_UNION;
#endif
#define PMIC_SPEL_DUTY_2_H_spel_duty_2_h_START (0)
#define PMIC_SPEL_DUTY_2_H_spel_duty_2_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_duty_3_l : 8;
    } reg;
} PMIC_SPEL_DUTY_3_L_UNION;
#endif
#define PMIC_SPEL_DUTY_3_L_spel_duty_3_l_START (0)
#define PMIC_SPEL_DUTY_3_L_spel_duty_3_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_duty_3_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_SPEL_DUTY_3_H_UNION;
#endif
#define PMIC_SPEL_DUTY_3_H_spel_duty_3_h_START (0)
#define PMIC_SPEL_DUTY_3_H_spel_duty_3_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_duty_4_l : 8;
    } reg;
} PMIC_SPEL_DUTY_4_L_UNION;
#endif
#define PMIC_SPEL_DUTY_4_L_spel_duty_4_l_START (0)
#define PMIC_SPEL_DUTY_4_L_spel_duty_4_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_duty_4_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_SPEL_DUTY_4_H_UNION;
#endif
#define PMIC_SPEL_DUTY_4_H_spel_duty_4_h_START (0)
#define PMIC_SPEL_DUTY_4_H_spel_duty_4_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_duty_5_l : 8;
    } reg;
} PMIC_SPEL_DUTY_5_L_UNION;
#endif
#define PMIC_SPEL_DUTY_5_L_spel_duty_5_l_START (0)
#define PMIC_SPEL_DUTY_5_L_spel_duty_5_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_duty_5_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_SPEL_DUTY_5_H_UNION;
#endif
#define PMIC_SPEL_DUTY_5_H_spel_duty_5_h_START (0)
#define PMIC_SPEL_DUTY_5_H_spel_duty_5_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_duty_6_l : 8;
    } reg;
} PMIC_SPEL_DUTY_6_L_UNION;
#endif
#define PMIC_SPEL_DUTY_6_L_spel_duty_6_l_START (0)
#define PMIC_SPEL_DUTY_6_L_spel_duty_6_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_duty_6_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_SPEL_DUTY_6_H_UNION;
#endif
#define PMIC_SPEL_DUTY_6_H_spel_duty_6_h_START (0)
#define PMIC_SPEL_DUTY_6_H_spel_duty_6_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_duty_7_l : 8;
    } reg;
} PMIC_SPEL_DUTY_7_L_UNION;
#endif
#define PMIC_SPEL_DUTY_7_L_spel_duty_7_l_START (0)
#define PMIC_SPEL_DUTY_7_L_spel_duty_7_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_duty_7_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_SPEL_DUTY_7_H_UNION;
#endif
#define PMIC_SPEL_DUTY_7_H_spel_duty_7_h_START (0)
#define PMIC_SPEL_DUTY_7_H_spel_duty_7_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_duty_8_l : 8;
    } reg;
} PMIC_SPEL_DUTY_8_L_UNION;
#endif
#define PMIC_SPEL_DUTY_8_L_spel_duty_8_l_START (0)
#define PMIC_SPEL_DUTY_8_L_spel_duty_8_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_duty_8_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_SPEL_DUTY_8_H_UNION;
#endif
#define PMIC_SPEL_DUTY_8_H_spel_duty_8_h_START (0)
#define PMIC_SPEL_DUTY_8_H_spel_duty_8_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_duty_9_l : 8;
    } reg;
} PMIC_SPEL_DUTY_9_L_UNION;
#endif
#define PMIC_SPEL_DUTY_9_L_spel_duty_9_l_START (0)
#define PMIC_SPEL_DUTY_9_L_spel_duty_9_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_duty_9_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_SPEL_DUTY_9_H_UNION;
#endif
#define PMIC_SPEL_DUTY_9_H_spel_duty_9_h_START (0)
#define PMIC_SPEL_DUTY_9_H_spel_duty_9_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_duty_10_l : 8;
    } reg;
} PMIC_SPEL_DUTY_10_L_UNION;
#endif
#define PMIC_SPEL_DUTY_10_L_spel_duty_10_l_START (0)
#define PMIC_SPEL_DUTY_10_L_spel_duty_10_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char spel_duty_10_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_SPEL_DUTY_10_H_UNION;
#endif
#define PMIC_SPEL_DUTY_10_H_spel_duty_10_h_START (0)
#define PMIC_SPEL_DUTY_10_H_spel_duty_10_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lra_adc_clk_cfg : 1;
        unsigned char lra_vsys_gain_sel : 2;
        unsigned char lra_pga_gain_sel : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_LRA_ANA_CFG0_UNION;
#endif
#define PMIC_LRA_ANA_CFG0_lra_adc_clk_cfg_START (0)
#define PMIC_LRA_ANA_CFG0_lra_adc_clk_cfg_END (0)
#define PMIC_LRA_ANA_CFG0_lra_vsys_gain_sel_START (1)
#define PMIC_LRA_ANA_CFG0_lra_vsys_gain_sel_END (2)
#define PMIC_LRA_ANA_CFG0_lra_pga_gain_sel_START (3)
#define PMIC_LRA_ANA_CFG0_lra_pga_gain_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lra_pd_res_sel : 2;
        unsigned char lra_power_enhance : 2;
        unsigned char lra_init_vld_shield : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LRA_ANA_CFG1_UNION;
#endif
#define PMIC_LRA_ANA_CFG1_lra_pd_res_sel_START (0)
#define PMIC_LRA_ANA_CFG1_lra_pd_res_sel_END (1)
#define PMIC_LRA_ANA_CFG1_lra_power_enhance_START (2)
#define PMIC_LRA_ANA_CFG1_lra_power_enhance_END (3)
#define PMIC_LRA_ANA_CFG1_lra_init_vld_shield_START (4)
#define PMIC_LRA_ANA_CFG1_lra_init_vld_shield_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lra_pg_n_sel : 2;
        unsigned char lra_pg_p_sel : 2;
        unsigned char lra_ng_n_sel : 2;
        unsigned char lra_ng_p_sel : 2;
    } reg;
} PMIC_LRA_ANA_CFG2_UNION;
#endif
#define PMIC_LRA_ANA_CFG2_lra_pg_n_sel_START (0)
#define PMIC_LRA_ANA_CFG2_lra_pg_n_sel_END (1)
#define PMIC_LRA_ANA_CFG2_lra_pg_p_sel_START (2)
#define PMIC_LRA_ANA_CFG2_lra_pg_p_sel_END (3)
#define PMIC_LRA_ANA_CFG2_lra_ng_n_sel_START (4)
#define PMIC_LRA_ANA_CFG2_lra_ng_n_sel_END (5)
#define PMIC_LRA_ANA_CFG2_lra_ng_p_sel_START (6)
#define PMIC_LRA_ANA_CFG2_lra_ng_p_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lra_ocl_shield_nmos : 1;
        unsigned char lra_ocl_shield_pmos : 1;
        unsigned char lra_uvp_shield : 1;
        unsigned char lra_adc_chop_enb : 1;
        unsigned char lra_pga_chop_enb : 1;
        unsigned char lra_buff_chop_enb : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_LRA_ANA_CFG3_UNION;
#endif
#define PMIC_LRA_ANA_CFG3_lra_ocl_shield_nmos_START (0)
#define PMIC_LRA_ANA_CFG3_lra_ocl_shield_nmos_END (0)
#define PMIC_LRA_ANA_CFG3_lra_ocl_shield_pmos_START (1)
#define PMIC_LRA_ANA_CFG3_lra_ocl_shield_pmos_END (1)
#define PMIC_LRA_ANA_CFG3_lra_uvp_shield_START (2)
#define PMIC_LRA_ANA_CFG3_lra_uvp_shield_END (2)
#define PMIC_LRA_ANA_CFG3_lra_adc_chop_enb_START (3)
#define PMIC_LRA_ANA_CFG3_lra_adc_chop_enb_END (3)
#define PMIC_LRA_ANA_CFG3_lra_pga_chop_enb_START (4)
#define PMIC_LRA_ANA_CFG3_lra_pga_chop_enb_END (4)
#define PMIC_LRA_ANA_CFG3_lra_buff_chop_enb_START (5)
#define PMIC_LRA_ANA_CFG3_lra_buff_chop_enb_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lra_ocp_shield : 1;
        unsigned char lra_ocp_thp_sel : 2;
        unsigned char lra_ocp_thn_sel : 2;
        unsigned char reserved : 3;
    } reg;
} PMIC_LRA_OCP_CFG_UNION;
#endif
#define PMIC_LRA_OCP_CFG_lra_ocp_shield_START (0)
#define PMIC_LRA_OCP_CFG_lra_ocp_shield_END (0)
#define PMIC_LRA_OCP_CFG_lra_ocp_thp_sel_START (1)
#define PMIC_LRA_OCP_CFG_lra_ocp_thp_sel_END (2)
#define PMIC_LRA_OCP_CFG_lra_ocp_thn_sel_START (3)
#define PMIC_LRA_OCP_CFG_lra_ocp_thn_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lra_ibias_sel : 8;
    } reg;
} PMIC_LRA_IBIAS_SEL_UNION;
#endif
#define PMIC_LRA_IBIAS_SEL_lra_ibias_sel_START (0)
#define PMIC_LRA_IBIAS_SEL_lra_ibias_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lra_ovp_shield : 1;
        unsigned char lra_ovp_th_sel : 2;
        unsigned char reserved : 5;
    } reg;
} PMIC_LRA_OVP_CFG_UNION;
#endif
#define PMIC_LRA_OVP_CFG_lra_ovp_shield_START (0)
#define PMIC_LRA_OVP_CFG_lra_ovp_shield_END (0)
#define PMIC_LRA_OVP_CFG_lra_ovp_th_sel_START (1)
#define PMIC_LRA_OVP_CFG_lra_ovp_th_sel_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lra_ana_reserve0 : 8;
    } reg;
} PMIC_LRA_ANA_RESERVED0_UNION;
#endif
#define PMIC_LRA_ANA_RESERVED0_lra_ana_reserve0_START (0)
#define PMIC_LRA_ANA_RESERVED0_lra_ana_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lra_ana_reserve1 : 8;
    } reg;
} PMIC_LRA_ANA_RESERVED1_UNION;
#endif
#define PMIC_LRA_ANA_RESERVED1_lra_ana_reserve1_START (0)
#define PMIC_LRA_ANA_RESERVED1_lra_ana_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hp_smpl_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_HPRD_SAMPLE_EN_UNION;
#endif
#define PMIC_HPRD_SAMPLE_EN_hp_smpl_en_START (0)
#define PMIC_HPRD_SAMPLE_EN_hp_smpl_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hp_smpl_valid : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_HPRD_SAMPLE_VALID_UNION;
#endif
#define PMIC_HPRD_SAMPLE_VALID_hp_smpl_valid_START (0)
#define PMIC_HPRD_SAMPLE_VALID_hp_smpl_valid_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hp_bemf_max_l : 8;
    } reg;
} PMIC_HPRD_BEMF_MAX_L_UNION;
#endif
#define PMIC_HPRD_BEMF_MAX_L_hp_bemf_max_l_START (0)
#define PMIC_HPRD_BEMF_MAX_L_hp_bemf_max_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hp_bemf_max_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_HPRD_BEMF_MAX_H_UNION;
#endif
#define PMIC_HPRD_BEMF_MAX_H_hp_bemf_max_h_START (0)
#define PMIC_HPRD_BEMF_MAX_H_hp_bemf_max_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hp_bemf_min_l : 8;
    } reg;
} PMIC_HPRD_BEMF_MIN_L_UNION;
#endif
#define PMIC_HPRD_BEMF_MIN_L_hp_bemf_min_l_START (0)
#define PMIC_HPRD_BEMF_MIN_L_hp_bemf_min_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hp_bemf_min_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_HPRD_BEMF_MIN_H_UNION;
#endif
#define PMIC_HPRD_BEMF_MIN_H_hp_bemf_min_h_START (0)
#define PMIC_HPRD_BEMF_MIN_H_hp_bemf_min_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hp_bemf_fst_l : 8;
    } reg;
} PMIC_HPRD_BEMF_FIRST_L_UNION;
#endif
#define PMIC_HPRD_BEMF_FIRST_L_hp_bemf_fst_l_START (0)
#define PMIC_HPRD_BEMF_FIRST_L_hp_bemf_fst_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hp_bemf_fst_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_HPRD_BEMF_FIRST_H_UNION;
#endif
#define PMIC_HPRD_BEMF_FIRST_H_hp_bemf_fst_h_START (0)
#define PMIC_HPRD_BEMF_FIRST_H_hp_bemf_fst_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hp_bemf_lst_l : 8;
    } reg;
} PMIC_HPRD_BEMF_LAST_L_UNION;
#endif
#define PMIC_HPRD_BEMF_LAST_L_hp_bemf_lst_l_START (0)
#define PMIC_HPRD_BEMF_LAST_L_hp_bemf_lst_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hp_bemf_lst_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_HPRD_BEMF_LAST_H_UNION;
#endif
#define PMIC_HPRD_BEMF_LAST_H_hp_bemf_lst_h_START (0)
#define PMIC_HPRD_BEMF_LAST_H_hp_bemf_lst_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hp_dr_time : 8;
    } reg;
} PMIC_HP_DR_TIME_UNION;
#endif
#define PMIC_HP_DR_TIME_hp_dr_time_START (0)
#define PMIC_HP_DR_TIME_hp_dr_time_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hp_nxt_dr_t : 8;
    } reg;
} PMIC_HPRD_NEXT_DR_UNION;
#endif
#define PMIC_HPRD_NEXT_DR_hp_nxt_dr_t_START (0)
#define PMIC_HPRD_NEXT_DR_hp_nxt_dr_t_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hp_time_l : 8;
    } reg;
} PMIC_HPRD_TIME_L_UNION;
#endif
#define PMIC_HPRD_TIME_L_hp_time_l_START (0)
#define PMIC_HPRD_TIME_L_hp_time_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hp_time_h : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_HPRD_TIME_H_UNION;
#endif
#define PMIC_HPRD_TIME_H_hp_time_h_START (0)
#define PMIC_HPRD_TIME_H_hp_time_h_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hp_duty_num : 8;
    } reg;
} PMIC_HPRD_DUTY_NUM_UNION;
#endif
#define PMIC_HPRD_DUTY_NUM_hp_duty_num_START (0)
#define PMIC_HPRD_DUTY_NUM_hp_duty_num_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hp_rank_num_l : 8;
    } reg;
} PMIC_HPRD_RANK_NUM_L_UNION;
#endif
#define PMIC_HPRD_RANK_NUM_L_hp_rank_num_l_START (0)
#define PMIC_HPRD_RANK_NUM_L_hp_rank_num_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hp_rank_num_h : 8;
    } reg;
} PMIC_HPRD_RANK_NUM_H_UNION;
#endif
#define PMIC_HPRD_RANK_NUM_H_hp_rank_num_h_START (0)
#define PMIC_HPRD_RANK_NUM_H_hp_rank_num_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hp_ocp_num : 8;
    } reg;
} PMIC_HPRD_OCP_NUM_UNION;
#endif
#define PMIC_HPRD_OCP_NUM_hp_ocp_num_START (0)
#define PMIC_HPRD_OCP_NUM_hp_ocp_num_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hp_fsm : 4;
        unsigned char hp_bemf_out : 1;
        unsigned char hp_tmax : 1;
        unsigned char hp_t1 : 1;
        unsigned char hp_drv_drec : 1;
    } reg;
} PMIC_HPRD_FSM_UNION;
#endif
#define PMIC_HPRD_FSM_hp_fsm_START (0)
#define PMIC_HPRD_FSM_hp_fsm_END (3)
#define PMIC_HPRD_FSM_hp_bemf_out_START (4)
#define PMIC_HPRD_FSM_hp_bemf_out_END (4)
#define PMIC_HPRD_FSM_hp_tmax_START (5)
#define PMIC_HPRD_FSM_hp_tmax_END (5)
#define PMIC_HPRD_FSM_hp_t1_START (6)
#define PMIC_HPRD_FSM_hp_t1_END (6)
#define PMIC_HPRD_FSM_hp_drv_drec_START (7)
#define PMIC_HPRD_FSM_hp_drv_drec_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lra_debug_en : 8;
    } reg;
} PMIC_LRA_DEBUG_EN_UNION;
#endif
#define PMIC_LRA_DEBUG_EN_lra_debug_en_START (0)
#define PMIC_LRA_DEBUG_EN_lra_debug_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dbg_lra_ana_pre_en : 1;
        unsigned char dbg_lra_ana_drv_en : 1;
        unsigned char dbg_lra_adc_en : 1;
        unsigned char dbg_lra_adc_start : 1;
        unsigned char dbg_adc_chanl_sel : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_DEBUG_ADC_CTRL_UNION;
#endif
#define PMIC_DEBUG_ADC_CTRL_dbg_lra_ana_pre_en_START (0)
#define PMIC_DEBUG_ADC_CTRL_dbg_lra_ana_pre_en_END (0)
#define PMIC_DEBUG_ADC_CTRL_dbg_lra_ana_drv_en_START (1)
#define PMIC_DEBUG_ADC_CTRL_dbg_lra_ana_drv_en_END (1)
#define PMIC_DEBUG_ADC_CTRL_dbg_lra_adc_en_START (2)
#define PMIC_DEBUG_ADC_CTRL_dbg_lra_adc_en_END (2)
#define PMIC_DEBUG_ADC_CTRL_dbg_lra_adc_start_START (3)
#define PMIC_DEBUG_ADC_CTRL_dbg_lra_adc_start_END (3)
#define PMIC_DEBUG_ADC_CTRL_dbg_adc_chanl_sel_START (4)
#define PMIC_DEBUG_ADC_CTRL_dbg_adc_chanl_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dbg_lra_adc_vld : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_DEBUG_ADC_VALID_UNION;
#endif
#define PMIC_DEBUG_ADC_VALID_dbg_lra_adc_vld_START (0)
#define PMIC_DEBUG_ADC_VALID_dbg_lra_adc_vld_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dbg_lra_adc_data_l : 8;
    } reg;
} PMIC_DEBUG_ADC_DATA_L_UNION;
#endif
#define PMIC_DEBUG_ADC_DATA_L_dbg_lra_adc_data_l_START (0)
#define PMIC_DEBUG_ADC_DATA_L_dbg_lra_adc_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dbg_lra_adc_data_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_DEBUG_ADC_DATA_H_UNION;
#endif
#define PMIC_DEBUG_ADC_DATA_H_dbg_lra_adc_data_h_START (0)
#define PMIC_DEBUG_ADC_DATA_H_dbg_lra_adc_data_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lra_clk_gate_bp : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_LRA_CLK_G_CTRL_UNION;
#endif
#define PMIC_LRA_CLK_G_CTRL_lra_clk_gate_bp_START (0)
#define PMIC_LRA_CLK_G_CTRL_lra_clk_gate_bp_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lra_nml_16hpt_l : 8;
    } reg;
} PMIC_LRA_NML_TIME_L_UNION;
#endif
#define PMIC_LRA_NML_TIME_L_lra_nml_16hpt_l_START (0)
#define PMIC_LRA_NML_TIME_L_lra_nml_16hpt_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lra_nml_16hpt_m : 8;
    } reg;
} PMIC_LRA_NML_TIME_M_UNION;
#endif
#define PMIC_LRA_NML_TIME_M_lra_nml_16hpt_m_START (0)
#define PMIC_LRA_NML_TIME_M_lra_nml_16hpt_m_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lra_nml_16hpt_h : 8;
    } reg;
} PMIC_LRA_NML_TIME_H_UNION;
#endif
#define PMIC_LRA_NML_TIME_H_lra_nml_16hpt_h_START (0)
#define PMIC_LRA_NML_TIME_H_lra_nml_16hpt_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lra_nml_16hpb_l : 8;
    } reg;
} PMIC_LRA_NML_BEMF_L_UNION;
#endif
#define PMIC_LRA_NML_BEMF_L_lra_nml_16hpb_l_START (0)
#define PMIC_LRA_NML_BEMF_L_lra_nml_16hpb_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lra_nml_16hpb_h : 8;
    } reg;
} PMIC_LRA_NML_BEMF_H_UNION;
#endif
#define PMIC_LRA_NML_BEMF_H_lra_nml_16hpb_h_START (0)
#define PMIC_LRA_NML_BEMF_H_lra_nml_16hpb_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lrag_ctrl_sel : 8;
    } reg;
} PMIC_LRA_GATE_CTRL_REG_UNION;
#endif
#define PMIC_LRA_GATE_CTRL_REG_lrag_ctrl_sel_START (0)
#define PMIC_LRA_GATE_CTRL_REG_lrag_ctrl_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lra_duty_sel : 8;
    } reg;
} PMIC_LRA_DUTYRES_REG_UNION;
#endif
#define PMIC_LRA_DUTYRES_REG_lra_duty_sel_START (0)
#define PMIC_LRA_DUTYRES_REG_lra_duty_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lra_bemf_zero_ok : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_LRA_BFST_OK_REG_UNION;
#endif
#define PMIC_LRA_BFST_OK_REG_lra_bemf_zero_ok_START (0)
#define PMIC_LRA_BFST_OK_REG_lra_bemf_zero_ok_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lra_bemf_zero_l : 8;
    } reg;
} PMIC_LRA_BEMF_FST_REG_LOW_UNION;
#endif
#define PMIC_LRA_BEMF_FST_REG_LOW_lra_bemf_zero_l_START (0)
#define PMIC_LRA_BEMF_FST_REG_LOW_lra_bemf_zero_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lra_bemf_zero_h : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LRA_BEMF_FST_REG_HIGH_UNION;
#endif
#define PMIC_LRA_BEMF_FST_REG_HIGH_lra_bemf_zero_h_START (0)
#define PMIC_LRA_BEMF_FST_REG_HIGH_lra_bemf_zero_h_END (4)
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
#define PMIC_LDO0_2_VSET_ADDR(base) ((base) + (0x0059UL))
#define PMIC_BUCK0_VSET_ADDR(base) ((base) + (0x0048UL))
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
