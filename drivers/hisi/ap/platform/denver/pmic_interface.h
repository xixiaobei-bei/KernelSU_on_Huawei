#ifndef __PMIC_INTERFACE_H__
#define __PMIC_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_VERSION0_ADDR(base) ((base) + (0x000UL))
#define PMIC_VERSION1_ADDR(base) ((base) + (0x001UL))
#define PMIC_VERSION2_ADDR(base) ((base) + (0x002UL))
#define PMIC_VERSION3_ADDR(base) ((base) + (0x003UL))
#define PMIC_VERSION4_ADDR(base) ((base) + (0x004UL))
#define PMIC_VERSION5_ADDR(base) ((base) + (0x005UL))
#define PMIC_DUMMY_SPMI_ADDR(base) ((base) + (0x006UL))
#define PMIC_STATUS0_ADDR(base) ((base) + (0x007UL))
#define PMIC_STATUS1_ADDR(base) ((base) + (0x008UL))
#define PMIC_STATUS2_ADDR(base) ((base) + (0x009UL))
#define PMIC_LOCK_ADDR(base) ((base) + (0x00FUL))
#define PMIC_BUCK0_ONOFF_ECO_ADDR(base) ((base) + (0x010UL))
#define PMIC_BUCK1_ONOFF_ECO_ADDR(base) ((base) + (0x011UL))
#define PMIC_BUCK2_ONOFF_ECO_ADDR(base) ((base) + (0x012UL))
#define PMIC_BUCK3_ONOFF_ECO_ADDR(base) ((base) + (0x013UL))
#define PMIC_BUCK40_ONOFF_ECO_ADDR(base) ((base) + (0x014UL))
#define PMIC_BUCK5_ONOFF_ECO_ADDR(base) ((base) + (0x015UL))
#define PMIC_BUCK6_ONOFF_ECO_ADDR(base) ((base) + (0x016UL))
#define PMIC_BUCK7_ONOFF_ECO_ADDR(base) ((base) + (0x017UL))
#define PMIC_LDO0_ONOFF_ECO_ADDR(base) ((base) + (0x018UL))
#define PMIC_LDO1_ONOFF_ECO_ADDR(base) ((base) + (0x019UL))
#define PMIC_LDO2_ONOFF_ECO_ADDR(base) ((base) + (0x01AUL))
#define PMIC_LDO3_ONOFF_ECO_ADDR(base) ((base) + (0x01BUL))
#define PMIC_LDO4_ONOFF_ECO_ADDR(base) ((base) + (0x01CUL))
#define PMIC_LDO5_ONOFF_ECO_ADDR(base) ((base) + (0x01DUL))
#define PMIC_LDO6_ONOFF_ECO_ADDR(base) ((base) + (0x01EUL))
#define PMIC_LDO8_ONOFF_ECO_ADDR(base) ((base) + (0x01FUL))
#define PMIC_LDO9_ONOFF_ECO_ADDR(base) ((base) + (0x020UL))
#define PMIC_LDO11_ONOFF_ECO_ADDR(base) ((base) + (0x021UL))
#define PMIC_LDO12_ONOFF_ECO_ADDR(base) ((base) + (0x022UL))
#define PMIC_LDO13_ONOFF_ECO_ADDR(base) ((base) + (0x023UL))
#define PMIC_LDO14_ONOFF_ECO_ADDR(base) ((base) + (0x024UL))
#define PMIC_LDO15_ONOFF_ECO_ADDR(base) ((base) + (0x025UL))
#define PMIC_LDO16_ONOFF_ECO_ADDR(base) ((base) + (0x026UL))
#define PMIC_LDO17_ONOFF_ECO_ADDR(base) ((base) + (0x027UL))
#define PMIC_LDO18_ONOFF_ECO_ADDR(base) ((base) + (0x028UL))
#define PMIC_LDO19_ONOFF_ECO_ADDR(base) ((base) + (0x029UL))
#define PMIC_LDO20_ONOFF_ECO_ADDR(base) ((base) + (0x02AUL))
#define PMIC_LDO21_ONOFF_ECO_ADDR(base) ((base) + (0x02BUL))
#define PMIC_LDO22_ONOFF_ADDR(base) ((base) + (0x02CUL))
#define PMIC_LDO23_ONOFF_ECO_ADDR(base) ((base) + (0x02DUL))
#define PMIC_LDO24_ONOFF_ECO_ADDR(base) ((base) + (0x02EUL))
#define PMIC_LDO25_ONOFF_ADDR(base) ((base) + (0x02FUL))
#define PMIC_LDO26_ONOFF_ECO_ADDR(base) ((base) + (0x030UL))
#define PMIC_LDO27_ONOFF_ECO_ADDR(base) ((base) + (0x031UL))
#define PMIC_LDO28_ONOFF_ECO_ADDR(base) ((base) + (0x032UL))
#define PMIC_LDO29_ONOFF_ECO_ADDR(base) ((base) + (0x033UL))
#define PMIC_LDO30_ONOFF_ECO_ADDR(base) ((base) + (0x034UL))
#define PMIC_LDO31_ONOFF_ECO_ADDR(base) ((base) + (0x035UL))
#define PMIC_LDO32_ONOFF_ECO_ADDR(base) ((base) + (0x036UL))
#define PMIC_LDO34_ONOFF_ECO_ADDR(base) ((base) + (0x037UL))
#define PMIC_PMUH_ONOFF_ECO_ADDR(base) ((base) + (0x038UL))
#define PMIC_LDO37_ONOFF_ECO_ADDR(base) ((base) + (0x039UL))
#define PMIC_LDO38_ONOFF_ECO_ADDR(base) ((base) + (0x03AUL))
#define PMIC_LDO39_ONOFF_ECO_ADDR(base) ((base) + (0x03BUL))
#define PMIC_LDO41_ONOFF_ECO_ADDR(base) ((base) + (0x03CUL))
#define PMIC_LDO42_ONOFF_ECO_ADDR(base) ((base) + (0x03DUL))
#define PMIC_LDO43_ONOFF_ECO_ADDR(base) ((base) + (0x03EUL))
#define PMIC_LDO44_ONOFF_ECO_ADDR(base) ((base) + (0x03FUL))
#define PMIC_LDO45_ONOFF_ECO_ADDR(base) ((base) + (0x040UL))
#define PMIC_LDO46_ONOFF_ECO_ADDR(base) ((base) + (0x041UL))
#define PMIC_LDO47_ONOFF_ECO_ADDR(base) ((base) + (0x042UL))
#define PMIC_LDO48_ONOFF_ECO_ADDR(base) ((base) + (0x043UL))
#define PMIC_LDO49_ONOFF_ECO_ADDR(base) ((base) + (0x044UL))
#define PMIC_LDO50_ONOFF_ECO_ADDR(base) ((base) + (0x045UL))
#define PMIC_BG_THSD_EN_ADDR(base) ((base) + (0x046UL))
#define PMIC_BG_REF_EN_ADDR(base) ((base) + (0x047UL))
#define PMIC_BG_REF_CTRL_ADDR(base) ((base) + (0x048UL))
#define PMIC_CLK_SERDES_EN_ADDR(base) ((base) + (0x049UL))
#define PMIC_CLK_WIFI_EN_ADDR(base) ((base) + (0x04AUL))
#define PMIC_CLK_NFC_EN_ADDR(base) ((base) + (0x04BUL))
#define PMIC_CLK_RF0_EN_ADDR(base) ((base) + (0x04CUL))
#define PMIC_CLK_RF1_EN_ADDR(base) ((base) + (0x0UL))
#define PMIC_CLK_SYS_EN_ADDR(base) ((base) + (0x04DUL))
#define PMIC_CLK_CODEC_EN_ADDR(base) ((base) + (0x04EUL))
#define PMIC_CLK_UFS_EN_ADDR(base) ((base) + (0x04FUL))
#define PMIC_OSC32K_BT_ONOFF_CTRL_ADDR(base) ((base) + (0x050UL))
#define PMIC_OSC32K_SYS_ONOFF_CTRL_ADDR(base) ((base) + (0x051UL))
#define PMIC_BB_ONOFF_ADDR(base) ((base) + (0x052UL))
#define PMIC_CLASSD_ONOFF_ADDR(base) ((base) + (0x053UL))
#define PMIC_BUCK0_VSET_ADDR(base) ((base) + (0x05AUL))
#define PMIC_BUCK0_VSET_ECO_ADDR(base) ((base) + (0x05BUL))
#define PMIC_BUCK1_VSET_ADDR(base) ((base) + (0x05CUL))
#define PMIC_BUCK2_VSET_ADDR(base) ((base) + (0x05DUL))
#define PMIC_BUCK2_VSET_ECO_ADDR(base) ((base) + (0x05EUL))
#define PMIC_BUCK3_VSET_ADDR(base) ((base) + (0x05FUL))
#define PMIC_BUCK3_VSET_ECO_ADDR(base) ((base) + (0x060UL))
#define PMIC_BUCK4_VSET_ADDR(base) ((base) + (0x061UL))
#define PMIC_BUCK4_VSET_ECO_ADDR(base) ((base) + (0x062UL))
#define PMIC_BUCK5_VSET_ADDR(base) ((base) + (0x063UL))
#define PMIC_BUCK5_VSET_ECO_ADDR(base) ((base) + (0x064UL))
#define PMIC_BUCK6_VSET_ADDR(base) ((base) + (0x065UL))
#define PMIC_BUCK6_VSET_ECO_ADDR(base) ((base) + (0x066UL))
#define PMIC_BUCK7_VSET_ADDR(base) ((base) + (0x067UL))
#define PMIC_LDO0_VSET_ADDR(base) ((base) + (0x068UL))
#define PMIC_LDO0_VSET_ECO_ADDR(base) ((base) + (0x069UL))
#define PMIC_LDO1_VSET_ADDR(base) ((base) + (0x06AUL))
#define PMIC_LDO2_VSET_ADDR(base) ((base) + (0x06BUL))
#define PMIC_LDO2_VSET_ECO_ADDR(base) ((base) + (0x06CUL))
#define PMIC_LDO3_VSET_ADDR(base) ((base) + (0x06DUL))
#define PMIC_LDO4_VSET_ADDR(base) ((base) + (0x06EUL))
#define PMIC_LDO5_VSET_ADDR(base) ((base) + (0x06FUL))
#define PMIC_LDO6_VSET_ADDR(base) ((base) + (0x070UL))
#define PMIC_LDO6_VSET_ECO_ADDR(base) ((base) + (0x071UL))
#define PMIC_LDO8_VSET_ADDR(base) ((base) + (0x072UL))
#define PMIC_LDO9_VSET_ADDR(base) ((base) + (0x073UL))
#define PMIC_LDO11_VSET_ADDR(base) ((base) + (0x074UL))
#define PMIC_LDO12_VSET_ADDR(base) ((base) + (0x075UL))
#define PMIC_LDO13_VSET_ADDR(base) ((base) + (0x076UL))
#define PMIC_LDO14_VSET_ADDR(base) ((base) + (0x077UL))
#define PMIC_LDO15_VSET_ADDR(base) ((base) + (0x078UL))
#define PMIC_LDO16_VSET_ADDR(base) ((base) + (0x079UL))
#define PMIC_LDO17_VSET_ADDR(base) ((base) + (0x07AUL))
#define PMIC_LDO18_VSET_ADDR(base) ((base) + (0x07BUL))
#define PMIC_LDO19_VSET_ADDR(base) ((base) + (0x07CUL))
#define PMIC_LDO20_VSET_ADDR(base) ((base) + (0x07DUL))
#define PMIC_LDO21_VSET_ADDR(base) ((base) + (0x07EUL))
#define PMIC_LDO22_VSET_ADDR(base) ((base) + (0x07FUL))
#define PMIC_LDO23_VSET_ADDR(base) ((base) + (0x080UL))
#define PMIC_LDO24_VSET_ADDR(base) ((base) + (0x081UL))
#define PMIC_LDO25_VSET_ADDR(base) ((base) + (0x082UL))
#define PMIC_LDO26_VSET_ADDR(base) ((base) + (0x083UL))
#define PMIC_LDO27_VSET_ADDR(base) ((base) + (0x084UL))
#define PMIC_LDO28_VSET_ADDR(base) ((base) + (0x085UL))
#define PMIC_LDO29_VSET_ADDR(base) ((base) + (0x086UL))
#define PMIC_LDO30_VSET_ADDR(base) ((base) + (0x087UL))
#define PMIC_LDO31_VSET_ADDR(base) ((base) + (0x088UL))
#define PMIC_LDO32_VSET_ADDR(base) ((base) + (0x089UL))
#define PMIC_LDO34_VSET_ADDR(base) ((base) + (0x08AUL))
#define PMIC_PMUH_VSET_ADDR(base) ((base) + (0x08BUL))
#define PMIC_LDO37_VSET_ADDR(base) ((base) + (0x08CUL))
#define PMIC_LDO38_VSET_ADDR(base) ((base) + (0x08DUL))
#define PMIC_LDO39_VSET_ADDR(base) ((base) + (0x08EUL))
#define PMIC_LDO41_VSET_ADDR(base) ((base) + (0x08FUL))
#define PMIC_LDO42_VSET_ADDR(base) ((base) + (0x090UL))
#define PMIC_LDO43_VSET_ADDR(base) ((base) + (0x091UL))
#define PMIC_LDO44_VSET_ADDR(base) ((base) + (0x092UL))
#define PMIC_LDO44_VSET_ECO_ADDR(base) ((base) + (0x093UL))
#define PMIC_LDO45_VSET_ADDR(base) ((base) + (0x094UL))
#define PMIC_LDO46_VSET_ADDR(base) ((base) + (0x095UL))
#define PMIC_LDO47_VSET_ADDR(base) ((base) + (0x096UL))
#define PMIC_LDO48_VSET_ADDR(base) ((base) + (0x097UL))
#define PMIC_LDO49_VSET_ADDR(base) ((base) + (0x098UL))
#define PMIC_LDO50_VSET_ADDR(base) ((base) + (0x099UL))
#define PMIC_LDO_BUF_VSET_ADDR(base) ((base) + (0x09AUL))
#define PMIC_PMU_IDLE_EN_ADDR(base) ((base) + (0x0A0UL))
#define PMIC_HARDWIRE_CTRL0_ADDR(base) ((base) + (0x0A1UL))
#define PMIC_HARDWIRE_CTRL1_ADDR(base) ((base) + (0x0A2UL))
#define PMIC_WIFI_CTRL_ADDR(base) ((base) + (0x0A9UL))
#define PMIC_PERI_VSET_CTRL0_ADDR(base) ((base) + (0x0AAUL))
#define PMIC_PERI_VSET_CTRL1_ADDR(base) ((base) + (0x0ABUL))
#define PMIC_SYS_CLK_EN_VSET_CTRL_ADDR(base) ((base) + (0x0ACUL))
#define PMIC_IDLE_EN_VSET_CTRL0_ADDR(base) ((base) + (0x0ADUL))
#define PMIC_PERI_CTRL0_ADDR(base) ((base) + (0x0AEUL))
#define PMIC_PERI_CTRL1_ADDR(base) ((base) + (0x0AFUL))
#define PMIC_PERI_CTRL2_ADDR(base) ((base) + (0x0B0UL))
#define PMIC_PERI_CTRL3_ADDR(base) ((base) + (0x0B1UL))
#define PMIC_PERI_CTRL4_ADDR(base) ((base) + (0x0B2UL))
#define PMIC_PERI_CTRL5_ADDR(base) ((base) + (0x0B3UL))
#define PMIC_PERI_CTRL6_ADDR(base) ((base) + (0x0B4UL))
#define PMIC_PERI_CTRL7_ADDR(base) ((base) + (0x0B5UL))
#define PMIC_PERI_OFF_CTRL0_ADDR(base) ((base) + (0x0B6UL))
#define PMIC_PERI_OFF_CTRL1_ADDR(base) ((base) + (0x0B7UL))
#define PMIC_PERI_OFF_CTRL2_ADDR(base) ((base) + (0x0B8UL))
#define PMIC_PERI_PLACE_CTRL0_ADDR(base) ((base) + (0x0B9UL))
#define PMIC_PERI_PLACE_CTRL1_ADDR(base) ((base) + (0x0BAUL))
#define PMIC_PERI_PLACE_CTRL2_ADDR(base) ((base) + (0x0BBUL))
#define PMIC_PERI_PLACE_CTRL3_ADDR(base) ((base) + (0x0BCUL))
#define PMIC_PERI_PLACE_CTRL5_ADDR(base) ((base) + (0x0BDUL))
#define PMIC_PERI_PLACE_CTRL6_ADDR(base) ((base) + (0x0BEUL))
#define PMIC_PERI_PLACE_CTRL7_ADDR(base) ((base) + (0x0BFUL))
#define PMIC_PERI_PLACE_CTRL8_ADDR(base) ((base) + (0x0C0UL))
#define PMIC_PERI_PLACE_CTRL9_ADDR(base) ((base) + (0x0C1UL))
#define PMIC_PERI_PLACE_CTRL10_ADDR(base) ((base) + (0x0C2UL))
#define PMIC_PERI_PLACE_CTRL11_ADDR(base) ((base) + (0x0C3UL))
#define PMIC_PERI_PLACE_CTRL12_ADDR(base) ((base) + (0x0C4UL))
#define PMIC_PERI_PLACE_CTRL13_ADDR(base) ((base) + (0x0C5UL))
#define PMIC_PERI_PLACE_CTRL14_ADDR(base) ((base) + (0x0C6UL))
#define PMIC_PERI_PLACE_CTRL15_ADDR(base) ((base) + (0x0C7UL))
#define PMIC_PERI_PLACE_CTRL16_ADDR(base) ((base) + (0x0C8UL))
#define PMIC_PERI_PLACE_CTRL17_ADDR(base) ((base) + (0x0C9UL))
#define PMIC_PERI_PLACE_CTRL18_ADDR(base) ((base) + (0x0CAUL))
#define PMIC_PERI_PLACE_CTRL19_ADDR(base) ((base) + (0x0CBUL))
#define PMIC_PERI_PLACE_CTRL20_ADDR(base) ((base) + (0x0CCUL))
#define PMIC_PERI_PLACE_CTRL21_ADDR(base) ((base) + (0x0CDUL))
#define PMIC_PERI_CNT_CFG_ADDR(base) ((base) + (0x0CEUL))
#define PMIC_IDLE_CTRL0_ADDR(base) ((base) + (0x0D5UL))
#define PMIC_IDLE_CTRL1_ADDR(base) ((base) + (0x0D6UL))
#define PMIC_IDLE_CTRL2_ADDR(base) ((base) + (0x0D7UL))
#define PMIC_IDLE_CTRL3_ADDR(base) ((base) + (0x0D8UL))
#define PMIC_IDLE_CTRL4_ADDR(base) ((base) + (0x0D9UL))
#define PMIC_IDLE_CTRL5_ADDR(base) ((base) + (0x0DAUL))
#define PMIC_IDLE_CTRL6_ADDR(base) ((base) + (0x0DBUL))
#define PMIC_IDLE_CTRL7_ADDR(base) ((base) + (0x0DCUL))
#define PMIC_IDLE_OFF_SEL0_ADDR(base) ((base) + (0x0DDUL))
#define PMIC_IDLE_OFF_SEL1_ADDR(base) ((base) + (0x0DEUL))
#define PMIC_IDLE_OFF_SEL2_ADDR(base) ((base) + (0x0DFUL))
#define PMIC_IDLE_PLACE_CTRL0_ADDR(base) ((base) + (0x0E0UL))
#define PMIC_IDLE_PLACE_CTRL1_ADDR(base) ((base) + (0x0E1UL))
#define PMIC_IDLE_PLACE_CTRL2_ADDR(base) ((base) + (0x0E2UL))
#define PMIC_IDLE_PLACE_CTRL3_ADDR(base) ((base) + (0x0E3UL))
#define PMIC_IDLE_PLACE_CTRL5_ADDR(base) ((base) + (0x0E4UL))
#define PMIC_IDLE_PLACE_CTRL6_ADDR(base) ((base) + (0x0E5UL))
#define PMIC_IDLE_PLACE_CTRL7_ADDR(base) ((base) + (0x0E6UL))
#define PMIC_IDLE_PLACE_CTRL8_ADDR(base) ((base) + (0x0E7UL))
#define PMIC_IDLE_PLACE_CTRL9_ADDR(base) ((base) + (0x0E8UL))
#define PMIC_IDLE_PLACE_CTRL10_ADDR(base) ((base) + (0x0E9UL))
#define PMIC_IDLE_PLACE_CTRL11_ADDR(base) ((base) + (0x0EAUL))
#define PMIC_IDLE_PLACE_CTRL12_ADDR(base) ((base) + (0x0EBUL))
#define PMIC_IDLE_PLACE_CTRL13_ADDR(base) ((base) + (0x0ECUL))
#define PMIC_IDLE_PLACE_CTRL14_ADDR(base) ((base) + (0x0EDUL))
#define PMIC_IDLE_PLACE_CTRL15_ADDR(base) ((base) + (0x0EEUL))
#define PMIC_IDLE_PLACE_CTRL16_ADDR(base) ((base) + (0x0EFUL))
#define PMIC_IDLE_PLACE_CTRL17_ADDR(base) ((base) + (0x0F0UL))
#define PMIC_IDLE_PLACE_CTRL18_ADDR(base) ((base) + (0x0F1UL))
#define PMIC_IDLE_PLACE_CTRL19_ADDR(base) ((base) + (0x0F2UL))
#define PMIC_IDLE_PLACE_CTRL20_ADDR(base) ((base) + (0x0F3UL))
#define PMIC_IDLE_PLACE_CTRL21_ADDR(base) ((base) + (0x0F4UL))
#define PMIC_IDLE_CNT_CFG_ADDR(base) ((base) + (0x0F5UL))
#define PMIC_PMU_SOFT_RST_ADDR(base) ((base) + (0x0FBUL))
#define PMIC_COUL_ECO_MASK_ADDR(base) ((base) + (0x0FCUL))
#define PMIC_DCR_ECO_MASK_ADDR(base) ((base) + (0x0FDUL))
#define PMIC_BUCK_OCP_MODE_CTRL0_ADDR(base) ((base) + (0x100UL))
#define PMIC_BUCK_OCP_MODE_CTRL1_ADDR(base) ((base) + (0x101UL))
#define PMIC_BUCK_OCP_MODE_CTRL2_ADDR(base) ((base) + (0x102UL))
#define PMIC_LDO_OCP_MODE_CTRL0_ADDR(base) ((base) + (0x103UL))
#define PMIC_LDO_OCP_MODE_CTRL1_ADDR(base) ((base) + (0x104UL))
#define PMIC_LDO_OCP_MODE_CTRL2_ADDR(base) ((base) + (0x105UL))
#define PMIC_LDO_OCP_MODE_CTRL3_ADDR(base) ((base) + (0x106UL))
#define PMIC_LDO_OCP_MODE_CTRL4_ADDR(base) ((base) + (0x107UL))
#define PMIC_LDO_OCP_MODE_CTRL5_ADDR(base) ((base) + (0x108UL))
#define PMIC_LDO_OCP_MODE_CTRL6_ADDR(base) ((base) + (0x109UL))
#define PMIC_LDO_OCP_MODE_CTRL7_ADDR(base) ((base) + (0x10AUL))
#define PMIC_LDO_OCP_MODE_CTRL8_ADDR(base) ((base) + (0x10BUL))
#define PMIC_LDO_OCP_MODE_CTRL9_ADDR(base) ((base) + (0x10CUL))
#define PMIC_LDO_OCP_MODE_CTRL10_ADDR(base) ((base) + (0x10DUL))
#define PMIC_LDO_OCP_MODE_CTRL11_ADDR(base) ((base) + (0x10EUL))
#define PMIC_BUCK_SCP_MODE_CTRL0_ADDR(base) ((base) + (0x10FUL))
#define PMIC_BUCK_SCP_MODE_CTRL1_ADDR(base) ((base) + (0x110UL))
#define PMIC_LDO_BUFF_SCP_CTRL_ADDR(base) ((base) + (0x111UL))
#define PMIC_BUCK_OVP_CTRL_ADDR(base) ((base) + (0x112UL))
#define PMIC_BUCK_OCP_ABN_CTRL0_ADDR(base) ((base) + (0x113UL))
#define PMIC_OCP_SCP_ONOFF0_ADDR(base) ((base) + (0x120UL))
#define PMIC_OCP_SCP_ONOFF1_ADDR(base) ((base) + (0x121UL))
#define PMIC_BUCK_LDO_ABN_DEB_CTRL0_ADDR(base) ((base) + (0x122UL))
#define PMIC_BUCK_LDO_ABN_DEB_CTRL1_ADDR(base) ((base) + (0x123UL))
#define PMIC_BUCK_LDO_ABN_DEB_CTRL2_ADDR(base) ((base) + (0x124UL))
#define PMIC_PWROFF_DEB_CTRL_ADDR(base) ((base) + (0x125UL))
#define PMIC_VSYS_DROP_DEB_CTRL_ADDR(base) ((base) + (0x126UL))
#define PMIC_SIM_CTRL0_ADDR(base) ((base) + (0x12AUL))
#define PMIC_SIM_CTRL1_ADDR(base) ((base) + (0x12BUL))
#define PMIC_SIM_DEB_CTRL1_ADDR(base) ((base) + (0x12CUL))
#define PMIC_SIM_DEB_CTRL2_ADDR(base) ((base) + (0x12DUL))
#define PMIC_GPIO0DATA_ADDR(base) ((base) + (0x12EUL))
#define PMIC_GPIO0_DATA_RESERVE_ADDR(base) ((base) + (0x12FUL))
#define PMIC_GPIO0DIR_ADDR(base) ((base) + (0x130UL))
#define PMIC_GPIO0_DIR_RESERVE_ADDR(base) ((base) + (0x131UL))
#define PMIC_GPIO0IS_ADDR(base) ((base) + (0x132UL))
#define PMIC_GPIO0IBE_ADDR(base) ((base) + (0x133UL))
#define PMIC_GPIO0IEV_ADDR(base) ((base) + (0x134UL))
#define PMIC_GPIO0AFSEL_ADDR(base) ((base) + (0x135UL))
#define PMIC_GPIO0DSSEL_ADDR(base) ((base) + (0x136UL))
#define PMIC_GPIO0PUSEL_ADDR(base) ((base) + (0x137UL))
#define PMIC_GPIO0PDSEL_ADDR(base) ((base) + (0x138UL))
#define PMIC_GPIO0DEBSEL_ADDR(base) ((base) + (0x139UL))
#define PMIC_SIM0PUSEL_ADDR(base) ((base) + (0x13AUL))
#define PMIC_SIM0PDSEL_ADDR(base) ((base) + (0x13BUL))
#define PMIC_GPIO1DATA_ADDR(base) ((base) + (0x13CUL))
#define PMIC_GPIO1_DATA_RESERVE_ADDR(base) ((base) + (0x13DUL))
#define PMIC_GPIO1DIR_ADDR(base) ((base) + (0x13EUL))
#define PMIC_GPIO1_DIR_RESERVE_ADDR(base) ((base) + (0x13FUL))
#define PMIC_GPIO1IS_ADDR(base) ((base) + (0x140UL))
#define PMIC_GPIO1IBE_ADDR(base) ((base) + (0x141UL))
#define PMIC_GPIO1IEV_ADDR(base) ((base) + (0x142UL))
#define PMIC_GPIO1AFSEL_ADDR(base) ((base) + (0x143UL))
#define PMIC_GPIO1DSSEL_ADDR(base) ((base) + (0x144UL))
#define PMIC_GPIO1PUSEL_ADDR(base) ((base) + (0x145UL))
#define PMIC_GPIO1PDSEL_ADDR(base) ((base) + (0x146UL))
#define PMIC_GPIO1DEBSEL_ADDR(base) ((base) + (0x147UL))
#define PMIC_SIM1PUSEL_ADDR(base) ((base) + (0x148UL))
#define PMIC_SIM1PDSEL_ADDR(base) ((base) + (0x149UL))
#define PMIC_RAMP_BUCK0_CTRL0_ADDR(base) ((base) + (0x14CUL))
#define PMIC_RAMP_BUCK0_CTRL1_ADDR(base) ((base) + (0x14DUL))
#define PMIC_BUCK0_RAMPUP_STATE_ADDR(base) ((base) + (0x14EUL))
#define PMIC_BUCK0_RAMPDOWN_STATE_ADDR(base) ((base) + (0x14FUL))
#define PMIC_BUCK0_RAMP_CHG_RATE_ADDR(base) ((base) + (0x150UL))
#define PMIC_BUCK0_RAMP_CHG_VSET_ADDR(base) ((base) + (0x151UL))
#define PMIC_BUCK0_CHG_RATE_SEL_ADDR(base) ((base) + (0x152UL))
#define PMIC_RAMP_BUCK40_CTRL0_ADDR(base) ((base) + (0x153UL))
#define PMIC_RAMP_BUCK40_CTRL1_ADDR(base) ((base) + (0x154UL))
#define PMIC_BUCK40_RAMPUP_STATE_ADDR(base) ((base) + (0x155UL))
#define PMIC_BUCK40_RAMPDOWN_STATE_ADDR(base) ((base) + (0x156UL))
#define PMIC_BUCK40_RAMP_CHG_RATE_ADDR(base) ((base) + (0x157UL))
#define PMIC_BUCK40_RAMP_CHG_VSET_ADDR(base) ((base) + (0x158UL))
#define PMIC_BUCK40_CHG_RATE_SEL_ADDR(base) ((base) + (0x159UL))
#define PMIC_RAMP_BUCK5_CTRL0_ADDR(base) ((base) + (0x15AUL))
#define PMIC_RAMP_BUCK5_CTRL1_ADDR(base) ((base) + (0x15BUL))
#define PMIC_BUCK5_RAMPUP_STATE_ADDR(base) ((base) + (0x15CUL))
#define PMIC_BUCK5_RAMPDOWN_STATE_ADDR(base) ((base) + (0x15DUL))
#define PMIC_BUCK5_RAMP_CHG_RATE_ADDR(base) ((base) + (0x15EUL))
#define PMIC_BUCK5_RAMP_CHG_VSET_ADDR(base) ((base) + (0x15FUL))
#define PMIC_BUCK5_CHG_RATE_SEL_ADDR(base) ((base) + (0x160UL))
#define PMIC_RAMP_BUCK7_CTRL0_ADDR(base) ((base) + (0x161UL))
#define PMIC_RAMP_BUCK7_CTRL1_ADDR(base) ((base) + (0x162UL))
#define PMIC_BUCK7_RAMPUP_STATE_ADDR(base) ((base) + (0x163UL))
#define PMIC_BUCK7_RAMPDOWN_STATE_ADDR(base) ((base) + (0x164UL))
#define PMIC_BUCK7_RAMP_CHG_RATE_ADDR(base) ((base) + (0x165UL))
#define PMIC_BUCK7_RAMP_CHG_VSET_ADDR(base) ((base) + (0x166UL))
#define PMIC_BUCK7_CHG_RATE_SEL_ADDR(base) ((base) + (0x167UL))
#define PMIC_RAMP_LDO0_CTRL0_ADDR(base) ((base) + (0x168UL))
#define PMIC_RAMP_LDO0_CTRL1_ADDR(base) ((base) + (0x169UL))
#define PMIC_RAMP_LDO6_CTRL0_ADDR(base) ((base) + (0x16AUL))
#define PMIC_RAMP_LDO6_CTRL1_ADDR(base) ((base) + (0x16BUL))
#define PMIC_RAMP_LDO44_CTRL0_ADDR(base) ((base) + (0x16CUL))
#define PMIC_RAMP_LDO44_CTRL1_ADDR(base) ((base) + (0x16DUL))
#define PMIC_RAMP_LDO47_CTRL0_ADDR(base) ((base) + (0x16EUL))
#define PMIC_RAMP_LDO47_CTRL1_ADDR(base) ((base) + (0x16FUL))
#define PMIC_DR_EN_MODE_345_ADDR(base) ((base) + (0x180UL))
#define PMIC_FLASH_PERIOD_DR345_ADDR(base) ((base) + (0x181UL))
#define PMIC_FLASH_ON_DR345_ADDR(base) ((base) + (0x182UL))
#define PMIC_DR_MODE_SEL_ADDR(base) ((base) + (0x183UL))
#define PMIC_DR_1_2_CTRL0_ADDR(base) ((base) + (0x184UL))
#define PMIC_DR_1_2_CTRL1_ADDR(base) ((base) + (0x185UL))
#define PMIC_DR1_ISET_ADDR(base) ((base) + (0x186UL))
#define PMIC_DR2_ISET_ADDR(base) ((base) + (0x187UL))
#define PMIC_DR_LED_CTRL_ADDR(base) ((base) + (0x188UL))
#define PMIC_DR_OUT_CTRL_ADDR(base) ((base) + (0x189UL))
#define PMIC_DR3_ISET_ADDR(base) ((base) + (0x18AUL))
#define PMIC_DR3_START_DEL_ADDR(base) ((base) + (0x18BUL))
#define PMIC_DR4_ISET_ADDR(base) ((base) + (0x18CUL))
#define PMIC_DR4_START_DEL_ADDR(base) ((base) + (0x18DUL))
#define PMIC_DR5_ISET_ADDR(base) ((base) + (0x18EUL))
#define PMIC_DR5_START_DEL_ADDR(base) ((base) + (0x18FUL))
#define PMIC_DR345_TIM_CONF0_ADDR(base) ((base) + (0x190UL))
#define PMIC_DR345_TIM_CONF1_ADDR(base) ((base) + (0x191UL))
#define PMIC_DR_CTRLRESERVE8_ADDR(base) ((base) + (0x192UL))
#define PMIC_DR_CTRLRESERVE9_ADDR(base) ((base) + (0x193UL))
#define PMIC_OTP0_0_ADDR(base) ((base) + (0x199UL))
#define PMIC_OTP0_1_ADDR(base) ((base) + (0x19AUL))
#define PMIC_OTP0_CTRL0_ADDR(base) ((base) + (0x19BUL))
#define PMIC_OTP0_CTRL1_ADDR(base) ((base) + (0x19CUL))
#define PMIC_OTP0_CTRL2_ADDR(base) ((base) + (0x19DUL))
#define PMIC_OTP0_WDATA_ADDR(base) ((base) + (0x19EUL))
#define PMIC_OTP0_0_W_ADDR(base) ((base) + (0x19FUL))
#define PMIC_OTP0_1_W_ADDR(base) ((base) + (0x1A0UL))
#define PMIC_OTP0_2_W_ADDR(base) ((base) + (0x1A1UL))
#define PMIC_OTP0_3_W_ADDR(base) ((base) + (0x1A2UL))
#define PMIC_OTP0_4_W_ADDR(base) ((base) + (0x1A3UL))
#define PMIC_OTP0_5_W_ADDR(base) ((base) + (0x1A4UL))
#define PMIC_OTP0_6_W_ADDR(base) ((base) + (0x1A5UL))
#define PMIC_OTP0_7_W_ADDR(base) ((base) + (0x1A6UL))
#define PMIC_OTP0_8_W_ADDR(base) ((base) + (0x1A7UL))
#define PMIC_OTP0_9_W_ADDR(base) ((base) + (0x1A8UL))
#define PMIC_OTP0_10_W_ADDR(base) ((base) + (0x1A9UL))
#define PMIC_OTP0_11_W_ADDR(base) ((base) + (0x1AAUL))
#define PMIC_OTP0_12_W_ADDR(base) ((base) + (0x1ABUL))
#define PMIC_OTP0_13_W_ADDR(base) ((base) + (0x1ACUL))
#define PMIC_OTP0_14_W_ADDR(base) ((base) + (0x1ADUL))
#define PMIC_OTP0_15_W_ADDR(base) ((base) + (0x1AEUL))
#define PMIC_OTP0_16_W_ADDR(base) ((base) + (0x1AFUL))
#define PMIC_OTP0_17_W_ADDR(base) ((base) + (0x1B0UL))
#define PMIC_OTP0_18_W_ADDR(base) ((base) + (0x1B1UL))
#define PMIC_OTP0_19_W_ADDR(base) ((base) + (0x1B2UL))
#define PMIC_OTP0_20_W_ADDR(base) ((base) + (0x1B3UL))
#define PMIC_OTP0_21_W_ADDR(base) ((base) + (0x1B4UL))
#define PMIC_OTP0_22_W_ADDR(base) ((base) + (0x1B5UL))
#define PMIC_OTP0_23_W_ADDR(base) ((base) + (0x1B6UL))
#define PMIC_OTP0_24_W_ADDR(base) ((base) + (0x1B7UL))
#define PMIC_OTP0_25_W_ADDR(base) ((base) + (0x1B8UL))
#define PMIC_OTP0_26_W_ADDR(base) ((base) + (0x1B9UL))
#define PMIC_OTP0_27_W_ADDR(base) ((base) + (0x1BAUL))
#define PMIC_OTP0_28_W_ADDR(base) ((base) + (0x1BBUL))
#define PMIC_OTP0_29_W_ADDR(base) ((base) + (0x1BCUL))
#define PMIC_OTP0_30_W_ADDR(base) ((base) + (0x1BDUL))
#define PMIC_OTP0_31_W_ADDR(base) ((base) + (0x1BEUL))
#define PMIC_OTP0_32_W_ADDR(base) ((base) + (0x1BFUL))
#define PMIC_OTP0_33_W_ADDR(base) ((base) + (0x1C0UL))
#define PMIC_OTP0_34_W_ADDR(base) ((base) + (0x1C1UL))
#define PMIC_OTP0_35_W_ADDR(base) ((base) + (0x1C2UL))
#define PMIC_OTP0_36_W_ADDR(base) ((base) + (0x1C3UL))
#define PMIC_OTP0_37_W_ADDR(base) ((base) + (0x1C4UL))
#define PMIC_OTP0_38_W_ADDR(base) ((base) + (0x1C5UL))
#define PMIC_OTP0_39_W_ADDR(base) ((base) + (0x1C6UL))
#define PMIC_OTP0_40_W_ADDR(base) ((base) + (0x1C7UL))
#define PMIC_OTP0_41_W_ADDR(base) ((base) + (0x1C8UL))
#define PMIC_OTP0_42_W_ADDR(base) ((base) + (0x1C9UL))
#define PMIC_OTP0_43_W_ADDR(base) ((base) + (0x1CAUL))
#define PMIC_OTP0_44_W_ADDR(base) ((base) + (0x1CBUL))
#define PMIC_OTP0_45_W_ADDR(base) ((base) + (0x1CCUL))
#define PMIC_OTP0_46_W_ADDR(base) ((base) + (0x1CDUL))
#define PMIC_OTP0_47_W_ADDR(base) ((base) + (0x1CEUL))
#define PMIC_OTP0_48_W_ADDR(base) ((base) + (0x1CFUL))
#define PMIC_OTP0_49_W_ADDR(base) ((base) + (0x1D0UL))
#define PMIC_OTP0_50_W_ADDR(base) ((base) + (0x1D1UL))
#define PMIC_OTP0_51_W_ADDR(base) ((base) + (0x1D2UL))
#define PMIC_OTP0_52_W_ADDR(base) ((base) + (0x1D3UL))
#define PMIC_OTP0_53_W_ADDR(base) ((base) + (0x1D4UL))
#define PMIC_OTP0_54_W_ADDR(base) ((base) + (0x1D5UL))
#define PMIC_OTP0_55_W_ADDR(base) ((base) + (0x1D6UL))
#define PMIC_OTP0_56_W_ADDR(base) ((base) + (0x1D7UL))
#define PMIC_OTP0_57_W_ADDR(base) ((base) + (0x1D8UL))
#define PMIC_OTP0_58_W_ADDR(base) ((base) + (0x1D9UL))
#define PMIC_OTP0_59_W_ADDR(base) ((base) + (0x1DAUL))
#define PMIC_OTP0_60_W_ADDR(base) ((base) + (0x1DBUL))
#define PMIC_OTP0_61_W_ADDR(base) ((base) + (0x1DCUL))
#define PMIC_OTP0_62_W_ADDR(base) ((base) + (0x1DDUL))
#define PMIC_OTP0_63_W_ADDR(base) ((base) + (0x1DEUL))
#define PMIC_OTP1_0_ADDR(base) ((base) + (0x1DFUL))
#define PMIC_OTP1_1_ADDR(base) ((base) + (0x1E0UL))
#define PMIC_OTP1_CTRL0_ADDR(base) ((base) + (0x1E1UL))
#define PMIC_OTP1_CTRL1_ADDR(base) ((base) + (0x1E2UL))
#define PMIC_OTP1_CTRL2_ADDR(base) ((base) + (0x1E3UL))
#define PMIC_OTP1_WDATA_ADDR(base) ((base) + (0x1E4UL))
#define PMIC_OTP1_0_W_ADDR(base) ((base) + (0x1E5UL))
#define PMIC_OTP1_1_W_ADDR(base) ((base) + (0x1E6UL))
#define PMIC_OTP1_2_W_ADDR(base) ((base) + (0x1E7UL))
#define PMIC_OTP1_3_W_ADDR(base) ((base) + (0x1E8UL))
#define PMIC_SPMI_ECO_GT_BYPASS_ADDR(base) ((base) + (0x1EDUL))
#define PMIC_PWRONN_8S_XOADC_DEBUG_CTRL_ADDR(base) ((base) + (0x1EEUL))
#define PMIC_DIS_19M2_CTRL_ADDR(base) ((base) + (0x1EFUL))
#define PMIC_PWRONN_CNT_ADDR(base) ((base) + (0x1F0UL))
#define PMIC_PWRONN_CNT_MASK_ADDR(base) ((base) + (0x01F1UL))
#else
#define PMIC_VERSION0_ADDR(base) ((base) + (0x000))
#define PMIC_VERSION1_ADDR(base) ((base) + (0x001))
#define PMIC_VERSION2_ADDR(base) ((base) + (0x002))
#define PMIC_VERSION3_ADDR(base) ((base) + (0x003))
#define PMIC_VERSION4_ADDR(base) ((base) + (0x004))
#define PMIC_VERSION5_ADDR(base) ((base) + (0x005))
#define PMIC_DUMMY_SPMI_ADDR(base) ((base) + (0x006))
#define PMIC_STATUS0_ADDR(base) ((base) + (0x007))
#define PMIC_STATUS1_ADDR(base) ((base) + (0x008))
#define PMIC_STATUS2_ADDR(base) ((base) + (0x009))
#define PMIC_LOCK_ADDR(base) ((base) + (0x00F))
#define PMIC_BUCK0_ONOFF_ECO_ADDR(base) ((base) + (0x010))
#define PMIC_BUCK1_ONOFF_ECO_ADDR(base) ((base) + (0x011))
#define PMIC_BUCK2_ONOFF_ECO_ADDR(base) ((base) + (0x012))
#define PMIC_BUCK3_ONOFF_ECO_ADDR(base) ((base) + (0x013))
#define PMIC_BUCK40_ONOFF_ECO_ADDR(base) ((base) + (0x014))
#define PMIC_BUCK5_ONOFF_ECO_ADDR(base) ((base) + (0x015))
#define PMIC_BUCK6_ONOFF_ECO_ADDR(base) ((base) + (0x016))
#define PMIC_BUCK7_ONOFF_ECO_ADDR(base) ((base) + (0x017))
#define PMIC_LDO0_ONOFF_ECO_ADDR(base) ((base) + (0x018))
#define PMIC_LDO1_ONOFF_ECO_ADDR(base) ((base) + (0x019))
#define PMIC_LDO2_ONOFF_ECO_ADDR(base) ((base) + (0x01A))
#define PMIC_LDO3_ONOFF_ECO_ADDR(base) ((base) + (0x01B))
#define PMIC_LDO4_ONOFF_ECO_ADDR(base) ((base) + (0x01C))
#define PMIC_LDO5_ONOFF_ECO_ADDR(base) ((base) + (0x01D))
#define PMIC_LDO6_ONOFF_ECO_ADDR(base) ((base) + (0x01E))
#define PMIC_LDO8_ONOFF_ECO_ADDR(base) ((base) + (0x01F))
#define PMIC_LDO9_ONOFF_ECO_ADDR(base) ((base) + (0x020))
#define PMIC_LDO11_ONOFF_ECO_ADDR(base) ((base) + (0x021))
#define PMIC_LDO12_ONOFF_ECO_ADDR(base) ((base) + (0x022))
#define PMIC_LDO13_ONOFF_ECO_ADDR(base) ((base) + (0x023))
#define PMIC_LDO14_ONOFF_ECO_ADDR(base) ((base) + (0x024))
#define PMIC_LDO15_ONOFF_ECO_ADDR(base) ((base) + (0x025))
#define PMIC_LDO16_ONOFF_ECO_ADDR(base) ((base) + (0x026))
#define PMIC_LDO17_ONOFF_ECO_ADDR(base) ((base) + (0x027))
#define PMIC_LDO18_ONOFF_ECO_ADDR(base) ((base) + (0x028))
#define PMIC_LDO19_ONOFF_ECO_ADDR(base) ((base) + (0x029))
#define PMIC_LDO20_ONOFF_ECO_ADDR(base) ((base) + (0x02A))
#define PMIC_LDO21_ONOFF_ECO_ADDR(base) ((base) + (0x02B))
#define PMIC_LDO22_ONOFF_ADDR(base) ((base) + (0x02C))
#define PMIC_LDO23_ONOFF_ECO_ADDR(base) ((base) + (0x02D))
#define PMIC_LDO24_ONOFF_ECO_ADDR(base) ((base) + (0x02E))
#define PMIC_LDO25_ONOFF_ADDR(base) ((base) + (0x02F))
#define PMIC_LDO26_ONOFF_ECO_ADDR(base) ((base) + (0x030))
#define PMIC_LDO27_ONOFF_ECO_ADDR(base) ((base) + (0x031))
#define PMIC_LDO28_ONOFF_ECO_ADDR(base) ((base) + (0x032))
#define PMIC_LDO29_ONOFF_ECO_ADDR(base) ((base) + (0x033))
#define PMIC_LDO30_ONOFF_ECO_ADDR(base) ((base) + (0x034))
#define PMIC_LDO31_ONOFF_ECO_ADDR(base) ((base) + (0x035))
#define PMIC_LDO32_ONOFF_ECO_ADDR(base) ((base) + (0x036))
#define PMIC_LDO34_ONOFF_ECO_ADDR(base) ((base) + (0x037))
#define PMIC_PMUH_ONOFF_ECO_ADDR(base) ((base) + (0x038))
#define PMIC_LDO37_ONOFF_ECO_ADDR(base) ((base) + (0x039))
#define PMIC_LDO38_ONOFF_ECO_ADDR(base) ((base) + (0x03A))
#define PMIC_LDO39_ONOFF_ECO_ADDR(base) ((base) + (0x03B))
#define PMIC_LDO41_ONOFF_ECO_ADDR(base) ((base) + (0x03C))
#define PMIC_LDO42_ONOFF_ECO_ADDR(base) ((base) + (0x03D))
#define PMIC_LDO43_ONOFF_ECO_ADDR(base) ((base) + (0x03E))
#define PMIC_LDO44_ONOFF_ECO_ADDR(base) ((base) + (0x03F))
#define PMIC_LDO45_ONOFF_ECO_ADDR(base) ((base) + (0x040))
#define PMIC_LDO46_ONOFF_ECO_ADDR(base) ((base) + (0x041))
#define PMIC_LDO47_ONOFF_ECO_ADDR(base) ((base) + (0x042))
#define PMIC_LDO48_ONOFF_ECO_ADDR(base) ((base) + (0x043))
#define PMIC_LDO49_ONOFF_ECO_ADDR(base) ((base) + (0x044))
#define PMIC_LDO50_ONOFF_ECO_ADDR(base) ((base) + (0x045))
#define PMIC_BG_THSD_EN_ADDR(base) ((base) + (0x046))
#define PMIC_BG_REF_EN_ADDR(base) ((base) + (0x047))
#define PMIC_BG_REF_CTRL_ADDR(base) ((base) + (0x048))
#define PMIC_CLK_SERDES_EN_ADDR(base) ((base) + (0x049))
#define PMIC_CLK_WIFI_EN_ADDR(base) ((base) + (0x04A))
#define PMIC_CLK_NFC_EN_ADDR(base) ((base) + (0x04B))
#define PMIC_CLK_RF0_EN_ADDR(base) ((base) + (0x04C))
#define PMIC_CLK_RF1_EN_ADDR(base) ((base) + (0x0))
#define PMIC_CLK_SYS_EN_ADDR(base) ((base) + (0x04D))
#define PMIC_CLK_CODEC_EN_ADDR(base) ((base) + (0x04E))
#define PMIC_CLK_UFS_EN_ADDR(base) ((base) + (0x04F))
#define PMIC_OSC32K_BT_ONOFF_CTRL_ADDR(base) ((base) + (0x050))
#define PMIC_OSC32K_SYS_ONOFF_CTRL_ADDR(base) ((base) + (0x051))
#define PMIC_BB_ONOFF_ADDR(base) ((base) + (0x052))
#define PMIC_CLASSD_ONOFF_ADDR(base) ((base) + (0x053))
#define PMIC_BUCK0_VSET_ADDR(base) ((base) + (0x05A))
#define PMIC_BUCK0_VSET_ECO_ADDR(base) ((base) + (0x05B))
#define PMIC_BUCK1_VSET_ADDR(base) ((base) + (0x05C))
#define PMIC_BUCK2_VSET_ADDR(base) ((base) + (0x05D))
#define PMIC_BUCK2_VSET_ECO_ADDR(base) ((base) + (0x05E))
#define PMIC_BUCK3_VSET_ADDR(base) ((base) + (0x05F))
#define PMIC_BUCK3_VSET_ECO_ADDR(base) ((base) + (0x060))
#define PMIC_BUCK4_VSET_ADDR(base) ((base) + (0x061))
#define PMIC_BUCK4_VSET_ECO_ADDR(base) ((base) + (0x062))
#define PMIC_BUCK5_VSET_ADDR(base) ((base) + (0x063))
#define PMIC_BUCK5_VSET_ECO_ADDR(base) ((base) + (0x064))
#define PMIC_BUCK6_VSET_ADDR(base) ((base) + (0x065))
#define PMIC_BUCK6_VSET_ECO_ADDR(base) ((base) + (0x066))
#define PMIC_BUCK7_VSET_ADDR(base) ((base) + (0x067))
#define PMIC_LDO0_VSET_ADDR(base) ((base) + (0x068))
#define PMIC_LDO0_VSET_ECO_ADDR(base) ((base) + (0x069))
#define PMIC_LDO1_VSET_ADDR(base) ((base) + (0x06A))
#define PMIC_LDO2_VSET_ADDR(base) ((base) + (0x06B))
#define PMIC_LDO2_VSET_ECO_ADDR(base) ((base) + (0x06C))
#define PMIC_LDO3_VSET_ADDR(base) ((base) + (0x06D))
#define PMIC_LDO4_VSET_ADDR(base) ((base) + (0x06E))
#define PMIC_LDO5_VSET_ADDR(base) ((base) + (0x06F))
#define PMIC_LDO6_VSET_ADDR(base) ((base) + (0x070))
#define PMIC_LDO6_VSET_ECO_ADDR(base) ((base) + (0x071))
#define PMIC_LDO8_VSET_ADDR(base) ((base) + (0x072))
#define PMIC_LDO9_VSET_ADDR(base) ((base) + (0x073))
#define PMIC_LDO11_VSET_ADDR(base) ((base) + (0x074))
#define PMIC_LDO12_VSET_ADDR(base) ((base) + (0x075))
#define PMIC_LDO13_VSET_ADDR(base) ((base) + (0x076))
#define PMIC_LDO14_VSET_ADDR(base) ((base) + (0x077))
#define PMIC_LDO15_VSET_ADDR(base) ((base) + (0x078))
#define PMIC_LDO16_VSET_ADDR(base) ((base) + (0x079))
#define PMIC_LDO17_VSET_ADDR(base) ((base) + (0x07A))
#define PMIC_LDO18_VSET_ADDR(base) ((base) + (0x07B))
#define PMIC_LDO19_VSET_ADDR(base) ((base) + (0x07C))
#define PMIC_LDO20_VSET_ADDR(base) ((base) + (0x07D))
#define PMIC_LDO21_VSET_ADDR(base) ((base) + (0x07E))
#define PMIC_LDO22_VSET_ADDR(base) ((base) + (0x07F))
#define PMIC_LDO23_VSET_ADDR(base) ((base) + (0x080))
#define PMIC_LDO24_VSET_ADDR(base) ((base) + (0x081))
#define PMIC_LDO25_VSET_ADDR(base) ((base) + (0x082))
#define PMIC_LDO26_VSET_ADDR(base) ((base) + (0x083))
#define PMIC_LDO27_VSET_ADDR(base) ((base) + (0x084))
#define PMIC_LDO28_VSET_ADDR(base) ((base) + (0x085))
#define PMIC_LDO29_VSET_ADDR(base) ((base) + (0x086))
#define PMIC_LDO30_VSET_ADDR(base) ((base) + (0x087))
#define PMIC_LDO31_VSET_ADDR(base) ((base) + (0x088))
#define PMIC_LDO32_VSET_ADDR(base) ((base) + (0x089))
#define PMIC_LDO34_VSET_ADDR(base) ((base) + (0x08A))
#define PMIC_PMUH_VSET_ADDR(base) ((base) + (0x08B))
#define PMIC_LDO37_VSET_ADDR(base) ((base) + (0x08C))
#define PMIC_LDO38_VSET_ADDR(base) ((base) + (0x08D))
#define PMIC_LDO39_VSET_ADDR(base) ((base) + (0x08E))
#define PMIC_LDO41_VSET_ADDR(base) ((base) + (0x08F))
#define PMIC_LDO42_VSET_ADDR(base) ((base) + (0x090))
#define PMIC_LDO43_VSET_ADDR(base) ((base) + (0x091))
#define PMIC_LDO44_VSET_ADDR(base) ((base) + (0x092))
#define PMIC_LDO44_VSET_ECO_ADDR(base) ((base) + (0x093))
#define PMIC_LDO45_VSET_ADDR(base) ((base) + (0x094))
#define PMIC_LDO46_VSET_ADDR(base) ((base) + (0x095))
#define PMIC_LDO47_VSET_ADDR(base) ((base) + (0x096))
#define PMIC_LDO48_VSET_ADDR(base) ((base) + (0x097))
#define PMIC_LDO49_VSET_ADDR(base) ((base) + (0x098))
#define PMIC_LDO50_VSET_ADDR(base) ((base) + (0x099))
#define PMIC_LDO_BUF_VSET_ADDR(base) ((base) + (0x09A))
#define PMIC_PMU_IDLE_EN_ADDR(base) ((base) + (0x0A0))
#define PMIC_HARDWIRE_CTRL0_ADDR(base) ((base) + (0x0A1))
#define PMIC_HARDWIRE_CTRL1_ADDR(base) ((base) + (0x0A2))
#define PMIC_WIFI_CTRL_ADDR(base) ((base) + (0x0A9))
#define PMIC_PERI_VSET_CTRL0_ADDR(base) ((base) + (0x0AA))
#define PMIC_PERI_VSET_CTRL1_ADDR(base) ((base) + (0x0AB))
#define PMIC_SYS_CLK_EN_VSET_CTRL_ADDR(base) ((base) + (0x0AC))
#define PMIC_IDLE_EN_VSET_CTRL0_ADDR(base) ((base) + (0x0AD))
#define PMIC_PERI_CTRL0_ADDR(base) ((base) + (0x0AE))
#define PMIC_PERI_CTRL1_ADDR(base) ((base) + (0x0AF))
#define PMIC_PERI_CTRL2_ADDR(base) ((base) + (0x0B0))
#define PMIC_PERI_CTRL3_ADDR(base) ((base) + (0x0B1))
#define PMIC_PERI_CTRL4_ADDR(base) ((base) + (0x0B2))
#define PMIC_PERI_CTRL5_ADDR(base) ((base) + (0x0B3))
#define PMIC_PERI_CTRL6_ADDR(base) ((base) + (0x0B4))
#define PMIC_PERI_CTRL7_ADDR(base) ((base) + (0x0B5))
#define PMIC_PERI_OFF_CTRL0_ADDR(base) ((base) + (0x0B6))
#define PMIC_PERI_OFF_CTRL1_ADDR(base) ((base) + (0x0B7))
#define PMIC_PERI_OFF_CTRL2_ADDR(base) ((base) + (0x0B8))
#define PMIC_PERI_PLACE_CTRL0_ADDR(base) ((base) + (0x0B9))
#define PMIC_PERI_PLACE_CTRL1_ADDR(base) ((base) + (0x0BA))
#define PMIC_PERI_PLACE_CTRL2_ADDR(base) ((base) + (0x0BB))
#define PMIC_PERI_PLACE_CTRL3_ADDR(base) ((base) + (0x0BC))
#define PMIC_PERI_PLACE_CTRL5_ADDR(base) ((base) + (0x0BD))
#define PMIC_PERI_PLACE_CTRL6_ADDR(base) ((base) + (0x0BE))
#define PMIC_PERI_PLACE_CTRL7_ADDR(base) ((base) + (0x0BF))
#define PMIC_PERI_PLACE_CTRL8_ADDR(base) ((base) + (0x0C0))
#define PMIC_PERI_PLACE_CTRL9_ADDR(base) ((base) + (0x0C1))
#define PMIC_PERI_PLACE_CTRL10_ADDR(base) ((base) + (0x0C2))
#define PMIC_PERI_PLACE_CTRL11_ADDR(base) ((base) + (0x0C3))
#define PMIC_PERI_PLACE_CTRL12_ADDR(base) ((base) + (0x0C4))
#define PMIC_PERI_PLACE_CTRL13_ADDR(base) ((base) + (0x0C5))
#define PMIC_PERI_PLACE_CTRL14_ADDR(base) ((base) + (0x0C6))
#define PMIC_PERI_PLACE_CTRL15_ADDR(base) ((base) + (0x0C7))
#define PMIC_PERI_PLACE_CTRL16_ADDR(base) ((base) + (0x0C8))
#define PMIC_PERI_PLACE_CTRL17_ADDR(base) ((base) + (0x0C9))
#define PMIC_PERI_PLACE_CTRL18_ADDR(base) ((base) + (0x0CA))
#define PMIC_PERI_PLACE_CTRL19_ADDR(base) ((base) + (0x0CB))
#define PMIC_PERI_PLACE_CTRL20_ADDR(base) ((base) + (0x0CC))
#define PMIC_PERI_PLACE_CTRL21_ADDR(base) ((base) + (0x0CD))
#define PMIC_PERI_CNT_CFG_ADDR(base) ((base) + (0x0CE))
#define PMIC_IDLE_CTRL0_ADDR(base) ((base) + (0x0D5))
#define PMIC_IDLE_CTRL1_ADDR(base) ((base) + (0x0D6))
#define PMIC_IDLE_CTRL2_ADDR(base) ((base) + (0x0D7))
#define PMIC_IDLE_CTRL3_ADDR(base) ((base) + (0x0D8))
#define PMIC_IDLE_CTRL4_ADDR(base) ((base) + (0x0D9))
#define PMIC_IDLE_CTRL5_ADDR(base) ((base) + (0x0DA))
#define PMIC_IDLE_CTRL6_ADDR(base) ((base) + (0x0DB))
#define PMIC_IDLE_CTRL7_ADDR(base) ((base) + (0x0DC))
#define PMIC_IDLE_OFF_SEL0_ADDR(base) ((base) + (0x0DD))
#define PMIC_IDLE_OFF_SEL1_ADDR(base) ((base) + (0x0DE))
#define PMIC_IDLE_OFF_SEL2_ADDR(base) ((base) + (0x0DF))
#define PMIC_IDLE_PLACE_CTRL0_ADDR(base) ((base) + (0x0E0))
#define PMIC_IDLE_PLACE_CTRL1_ADDR(base) ((base) + (0x0E1))
#define PMIC_IDLE_PLACE_CTRL2_ADDR(base) ((base) + (0x0E2))
#define PMIC_IDLE_PLACE_CTRL3_ADDR(base) ((base) + (0x0E3))
#define PMIC_IDLE_PLACE_CTRL5_ADDR(base) ((base) + (0x0E4))
#define PMIC_IDLE_PLACE_CTRL6_ADDR(base) ((base) + (0x0E5))
#define PMIC_IDLE_PLACE_CTRL7_ADDR(base) ((base) + (0x0E6))
#define PMIC_IDLE_PLACE_CTRL8_ADDR(base) ((base) + (0x0E7))
#define PMIC_IDLE_PLACE_CTRL9_ADDR(base) ((base) + (0x0E8))
#define PMIC_IDLE_PLACE_CTRL10_ADDR(base) ((base) + (0x0E9))
#define PMIC_IDLE_PLACE_CTRL11_ADDR(base) ((base) + (0x0EA))
#define PMIC_IDLE_PLACE_CTRL12_ADDR(base) ((base) + (0x0EB))
#define PMIC_IDLE_PLACE_CTRL13_ADDR(base) ((base) + (0x0EC))
#define PMIC_IDLE_PLACE_CTRL14_ADDR(base) ((base) + (0x0ED))
#define PMIC_IDLE_PLACE_CTRL15_ADDR(base) ((base) + (0x0EE))
#define PMIC_IDLE_PLACE_CTRL16_ADDR(base) ((base) + (0x0EF))
#define PMIC_IDLE_PLACE_CTRL17_ADDR(base) ((base) + (0x0F0))
#define PMIC_IDLE_PLACE_CTRL18_ADDR(base) ((base) + (0x0F1))
#define PMIC_IDLE_PLACE_CTRL19_ADDR(base) ((base) + (0x0F2))
#define PMIC_IDLE_PLACE_CTRL20_ADDR(base) ((base) + (0x0F3))
#define PMIC_IDLE_PLACE_CTRL21_ADDR(base) ((base) + (0x0F4))
#define PMIC_IDLE_CNT_CFG_ADDR(base) ((base) + (0x0F5))
#define PMIC_PMU_SOFT_RST_ADDR(base) ((base) + (0x0FB))
#define PMIC_COUL_ECO_MASK_ADDR(base) ((base) + (0x0FC))
#define PMIC_DCR_ECO_MASK_ADDR(base) ((base) + (0x0FD))
#define PMIC_BUCK_OCP_MODE_CTRL0_ADDR(base) ((base) + (0x100))
#define PMIC_BUCK_OCP_MODE_CTRL1_ADDR(base) ((base) + (0x101))
#define PMIC_BUCK_OCP_MODE_CTRL2_ADDR(base) ((base) + (0x102))
#define PMIC_LDO_OCP_MODE_CTRL0_ADDR(base) ((base) + (0x103))
#define PMIC_LDO_OCP_MODE_CTRL1_ADDR(base) ((base) + (0x104))
#define PMIC_LDO_OCP_MODE_CTRL2_ADDR(base) ((base) + (0x105))
#define PMIC_LDO_OCP_MODE_CTRL3_ADDR(base) ((base) + (0x106))
#define PMIC_LDO_OCP_MODE_CTRL4_ADDR(base) ((base) + (0x107))
#define PMIC_LDO_OCP_MODE_CTRL5_ADDR(base) ((base) + (0x108))
#define PMIC_LDO_OCP_MODE_CTRL6_ADDR(base) ((base) + (0x109))
#define PMIC_LDO_OCP_MODE_CTRL7_ADDR(base) ((base) + (0x10A))
#define PMIC_LDO_OCP_MODE_CTRL8_ADDR(base) ((base) + (0x10B))
#define PMIC_LDO_OCP_MODE_CTRL9_ADDR(base) ((base) + (0x10C))
#define PMIC_LDO_OCP_MODE_CTRL10_ADDR(base) ((base) + (0x10D))
#define PMIC_LDO_OCP_MODE_CTRL11_ADDR(base) ((base) + (0x10E))
#define PMIC_BUCK_SCP_MODE_CTRL0_ADDR(base) ((base) + (0x10F))
#define PMIC_BUCK_SCP_MODE_CTRL1_ADDR(base) ((base) + (0x110))
#define PMIC_LDO_BUFF_SCP_CTRL_ADDR(base) ((base) + (0x111))
#define PMIC_BUCK_OVP_CTRL_ADDR(base) ((base) + (0x112))
#define PMIC_BUCK_OCP_ABN_CTRL0_ADDR(base) ((base) + (0x113))
#define PMIC_OCP_SCP_ONOFF0_ADDR(base) ((base) + (0x120))
#define PMIC_OCP_SCP_ONOFF1_ADDR(base) ((base) + (0x121))
#define PMIC_BUCK_LDO_ABN_DEB_CTRL0_ADDR(base) ((base) + (0x122))
#define PMIC_BUCK_LDO_ABN_DEB_CTRL1_ADDR(base) ((base) + (0x123))
#define PMIC_BUCK_LDO_ABN_DEB_CTRL2_ADDR(base) ((base) + (0x124))
#define PMIC_PWROFF_DEB_CTRL_ADDR(base) ((base) + (0x125))
#define PMIC_VSYS_DROP_DEB_CTRL_ADDR(base) ((base) + (0x126))
#define PMIC_SIM_CTRL0_ADDR(base) ((base) + (0x12A))
#define PMIC_SIM_CTRL1_ADDR(base) ((base) + (0x12B))
#define PMIC_SIM_DEB_CTRL1_ADDR(base) ((base) + (0x12C))
#define PMIC_SIM_DEB_CTRL2_ADDR(base) ((base) + (0x12D))
#define PMIC_GPIO0DATA_ADDR(base) ((base) + (0x12E))
#define PMIC_GPIO0_DATA_RESERVE_ADDR(base) ((base) + (0x12F))
#define PMIC_GPIO0DIR_ADDR(base) ((base) + (0x130))
#define PMIC_GPIO0_DIR_RESERVE_ADDR(base) ((base) + (0x131))
#define PMIC_GPIO0IS_ADDR(base) ((base) + (0x132))
#define PMIC_GPIO0IBE_ADDR(base) ((base) + (0x133))
#define PMIC_GPIO0IEV_ADDR(base) ((base) + (0x134))
#define PMIC_GPIO0AFSEL_ADDR(base) ((base) + (0x135))
#define PMIC_GPIO0DSSEL_ADDR(base) ((base) + (0x136))
#define PMIC_GPIO0PUSEL_ADDR(base) ((base) + (0x137))
#define PMIC_GPIO0PDSEL_ADDR(base) ((base) + (0x138))
#define PMIC_GPIO0DEBSEL_ADDR(base) ((base) + (0x139))
#define PMIC_SIM0PUSEL_ADDR(base) ((base) + (0x13A))
#define PMIC_SIM0PDSEL_ADDR(base) ((base) + (0x13B))
#define PMIC_GPIO1DATA_ADDR(base) ((base) + (0x13C))
#define PMIC_GPIO1_DATA_RESERVE_ADDR(base) ((base) + (0x13D))
#define PMIC_GPIO1DIR_ADDR(base) ((base) + (0x13E))
#define PMIC_GPIO1_DIR_RESERVE_ADDR(base) ((base) + (0x13F))
#define PMIC_GPIO1IS_ADDR(base) ((base) + (0x140))
#define PMIC_GPIO1IBE_ADDR(base) ((base) + (0x141))
#define PMIC_GPIO1IEV_ADDR(base) ((base) + (0x142))
#define PMIC_GPIO1AFSEL_ADDR(base) ((base) + (0x143))
#define PMIC_GPIO1DSSEL_ADDR(base) ((base) + (0x144))
#define PMIC_GPIO1PUSEL_ADDR(base) ((base) + (0x145))
#define PMIC_GPIO1PDSEL_ADDR(base) ((base) + (0x146))
#define PMIC_GPIO1DEBSEL_ADDR(base) ((base) + (0x147))
#define PMIC_SIM1PUSEL_ADDR(base) ((base) + (0x148))
#define PMIC_SIM1PDSEL_ADDR(base) ((base) + (0x149))
#define PMIC_RAMP_BUCK0_CTRL0_ADDR(base) ((base) + (0x14C))
#define PMIC_RAMP_BUCK0_CTRL1_ADDR(base) ((base) + (0x14D))
#define PMIC_BUCK0_RAMPUP_STATE_ADDR(base) ((base) + (0x14E))
#define PMIC_BUCK0_RAMPDOWN_STATE_ADDR(base) ((base) + (0x14F))
#define PMIC_BUCK0_RAMP_CHG_RATE_ADDR(base) ((base) + (0x150))
#define PMIC_BUCK0_RAMP_CHG_VSET_ADDR(base) ((base) + (0x151))
#define PMIC_BUCK0_CHG_RATE_SEL_ADDR(base) ((base) + (0x152))
#define PMIC_RAMP_BUCK40_CTRL0_ADDR(base) ((base) + (0x153))
#define PMIC_RAMP_BUCK40_CTRL1_ADDR(base) ((base) + (0x154))
#define PMIC_BUCK40_RAMPUP_STATE_ADDR(base) ((base) + (0x155))
#define PMIC_BUCK40_RAMPDOWN_STATE_ADDR(base) ((base) + (0x156))
#define PMIC_BUCK40_RAMP_CHG_RATE_ADDR(base) ((base) + (0x157))
#define PMIC_BUCK40_RAMP_CHG_VSET_ADDR(base) ((base) + (0x158))
#define PMIC_BUCK40_CHG_RATE_SEL_ADDR(base) ((base) + (0x159))
#define PMIC_RAMP_BUCK5_CTRL0_ADDR(base) ((base) + (0x15A))
#define PMIC_RAMP_BUCK5_CTRL1_ADDR(base) ((base) + (0x15B))
#define PMIC_BUCK5_RAMPUP_STATE_ADDR(base) ((base) + (0x15C))
#define PMIC_BUCK5_RAMPDOWN_STATE_ADDR(base) ((base) + (0x15D))
#define PMIC_BUCK5_RAMP_CHG_RATE_ADDR(base) ((base) + (0x15E))
#define PMIC_BUCK5_RAMP_CHG_VSET_ADDR(base) ((base) + (0x15F))
#define PMIC_BUCK5_CHG_RATE_SEL_ADDR(base) ((base) + (0x160))
#define PMIC_RAMP_BUCK7_CTRL0_ADDR(base) ((base) + (0x161))
#define PMIC_RAMP_BUCK7_CTRL1_ADDR(base) ((base) + (0x162))
#define PMIC_BUCK7_RAMPUP_STATE_ADDR(base) ((base) + (0x163))
#define PMIC_BUCK7_RAMPDOWN_STATE_ADDR(base) ((base) + (0x164))
#define PMIC_BUCK7_RAMP_CHG_RATE_ADDR(base) ((base) + (0x165))
#define PMIC_BUCK7_RAMP_CHG_VSET_ADDR(base) ((base) + (0x166))
#define PMIC_BUCK7_CHG_RATE_SEL_ADDR(base) ((base) + (0x167))
#define PMIC_RAMP_LDO0_CTRL0_ADDR(base) ((base) + (0x168))
#define PMIC_RAMP_LDO0_CTRL1_ADDR(base) ((base) + (0x169))
#define PMIC_RAMP_LDO6_CTRL0_ADDR(base) ((base) + (0x16A))
#define PMIC_RAMP_LDO6_CTRL1_ADDR(base) ((base) + (0x16B))
#define PMIC_RAMP_LDO44_CTRL0_ADDR(base) ((base) + (0x16C))
#define PMIC_RAMP_LDO44_CTRL1_ADDR(base) ((base) + (0x16D))
#define PMIC_RAMP_LDO47_CTRL0_ADDR(base) ((base) + (0x16E))
#define PMIC_RAMP_LDO47_CTRL1_ADDR(base) ((base) + (0x16F))
#define PMIC_DR_EN_MODE_345_ADDR(base) ((base) + (0x180))
#define PMIC_FLASH_PERIOD_DR345_ADDR(base) ((base) + (0x181))
#define PMIC_FLASH_ON_DR345_ADDR(base) ((base) + (0x182))
#define PMIC_DR_MODE_SEL_ADDR(base) ((base) + (0x183))
#define PMIC_DR_1_2_CTRL0_ADDR(base) ((base) + (0x184))
#define PMIC_DR_1_2_CTRL1_ADDR(base) ((base) + (0x185))
#define PMIC_DR1_ISET_ADDR(base) ((base) + (0x186))
#define PMIC_DR2_ISET_ADDR(base) ((base) + (0x187))
#define PMIC_DR_LED_CTRL_ADDR(base) ((base) + (0x188))
#define PMIC_DR_OUT_CTRL_ADDR(base) ((base) + (0x189))
#define PMIC_DR3_ISET_ADDR(base) ((base) + (0x18A))
#define PMIC_DR3_START_DEL_ADDR(base) ((base) + (0x18B))
#define PMIC_DR4_ISET_ADDR(base) ((base) + (0x18C))
#define PMIC_DR4_START_DEL_ADDR(base) ((base) + (0x18D))
#define PMIC_DR5_ISET_ADDR(base) ((base) + (0x18E))
#define PMIC_DR5_START_DEL_ADDR(base) ((base) + (0x18F))
#define PMIC_DR345_TIM_CONF0_ADDR(base) ((base) + (0x190))
#define PMIC_DR345_TIM_CONF1_ADDR(base) ((base) + (0x191))
#define PMIC_DR_CTRLRESERVE8_ADDR(base) ((base) + (0x192))
#define PMIC_DR_CTRLRESERVE9_ADDR(base) ((base) + (0x193))
#define PMIC_OTP0_0_ADDR(base) ((base) + (0x199))
#define PMIC_OTP0_1_ADDR(base) ((base) + (0x19A))
#define PMIC_OTP0_CTRL0_ADDR(base) ((base) + (0x19B))
#define PMIC_OTP0_CTRL1_ADDR(base) ((base) + (0x19C))
#define PMIC_OTP0_CTRL2_ADDR(base) ((base) + (0x19D))
#define PMIC_OTP0_WDATA_ADDR(base) ((base) + (0x19E))
#define PMIC_OTP0_0_W_ADDR(base) ((base) + (0x19F))
#define PMIC_OTP0_1_W_ADDR(base) ((base) + (0x1A0))
#define PMIC_OTP0_2_W_ADDR(base) ((base) + (0x1A1))
#define PMIC_OTP0_3_W_ADDR(base) ((base) + (0x1A2))
#define PMIC_OTP0_4_W_ADDR(base) ((base) + (0x1A3))
#define PMIC_OTP0_5_W_ADDR(base) ((base) + (0x1A4))
#define PMIC_OTP0_6_W_ADDR(base) ((base) + (0x1A5))
#define PMIC_OTP0_7_W_ADDR(base) ((base) + (0x1A6))
#define PMIC_OTP0_8_W_ADDR(base) ((base) + (0x1A7))
#define PMIC_OTP0_9_W_ADDR(base) ((base) + (0x1A8))
#define PMIC_OTP0_10_W_ADDR(base) ((base) + (0x1A9))
#define PMIC_OTP0_11_W_ADDR(base) ((base) + (0x1AA))
#define PMIC_OTP0_12_W_ADDR(base) ((base) + (0x1AB))
#define PMIC_OTP0_13_W_ADDR(base) ((base) + (0x1AC))
#define PMIC_OTP0_14_W_ADDR(base) ((base) + (0x1AD))
#define PMIC_OTP0_15_W_ADDR(base) ((base) + (0x1AE))
#define PMIC_OTP0_16_W_ADDR(base) ((base) + (0x1AF))
#define PMIC_OTP0_17_W_ADDR(base) ((base) + (0x1B0))
#define PMIC_OTP0_18_W_ADDR(base) ((base) + (0x1B1))
#define PMIC_OTP0_19_W_ADDR(base) ((base) + (0x1B2))
#define PMIC_OTP0_20_W_ADDR(base) ((base) + (0x1B3))
#define PMIC_OTP0_21_W_ADDR(base) ((base) + (0x1B4))
#define PMIC_OTP0_22_W_ADDR(base) ((base) + (0x1B5))
#define PMIC_OTP0_23_W_ADDR(base) ((base) + (0x1B6))
#define PMIC_OTP0_24_W_ADDR(base) ((base) + (0x1B7))
#define PMIC_OTP0_25_W_ADDR(base) ((base) + (0x1B8))
#define PMIC_OTP0_26_W_ADDR(base) ((base) + (0x1B9))
#define PMIC_OTP0_27_W_ADDR(base) ((base) + (0x1BA))
#define PMIC_OTP0_28_W_ADDR(base) ((base) + (0x1BB))
#define PMIC_OTP0_29_W_ADDR(base) ((base) + (0x1BC))
#define PMIC_OTP0_30_W_ADDR(base) ((base) + (0x1BD))
#define PMIC_OTP0_31_W_ADDR(base) ((base) + (0x1BE))
#define PMIC_OTP0_32_W_ADDR(base) ((base) + (0x1BF))
#define PMIC_OTP0_33_W_ADDR(base) ((base) + (0x1C0))
#define PMIC_OTP0_34_W_ADDR(base) ((base) + (0x1C1))
#define PMIC_OTP0_35_W_ADDR(base) ((base) + (0x1C2))
#define PMIC_OTP0_36_W_ADDR(base) ((base) + (0x1C3))
#define PMIC_OTP0_37_W_ADDR(base) ((base) + (0x1C4))
#define PMIC_OTP0_38_W_ADDR(base) ((base) + (0x1C5))
#define PMIC_OTP0_39_W_ADDR(base) ((base) + (0x1C6))
#define PMIC_OTP0_40_W_ADDR(base) ((base) + (0x1C7))
#define PMIC_OTP0_41_W_ADDR(base) ((base) + (0x1C8))
#define PMIC_OTP0_42_W_ADDR(base) ((base) + (0x1C9))
#define PMIC_OTP0_43_W_ADDR(base) ((base) + (0x1CA))
#define PMIC_OTP0_44_W_ADDR(base) ((base) + (0x1CB))
#define PMIC_OTP0_45_W_ADDR(base) ((base) + (0x1CC))
#define PMIC_OTP0_46_W_ADDR(base) ((base) + (0x1CD))
#define PMIC_OTP0_47_W_ADDR(base) ((base) + (0x1CE))
#define PMIC_OTP0_48_W_ADDR(base) ((base) + (0x1CF))
#define PMIC_OTP0_49_W_ADDR(base) ((base) + (0x1D0))
#define PMIC_OTP0_50_W_ADDR(base) ((base) + (0x1D1))
#define PMIC_OTP0_51_W_ADDR(base) ((base) + (0x1D2))
#define PMIC_OTP0_52_W_ADDR(base) ((base) + (0x1D3))
#define PMIC_OTP0_53_W_ADDR(base) ((base) + (0x1D4))
#define PMIC_OTP0_54_W_ADDR(base) ((base) + (0x1D5))
#define PMIC_OTP0_55_W_ADDR(base) ((base) + (0x1D6))
#define PMIC_OTP0_56_W_ADDR(base) ((base) + (0x1D7))
#define PMIC_OTP0_57_W_ADDR(base) ((base) + (0x1D8))
#define PMIC_OTP0_58_W_ADDR(base) ((base) + (0x1D9))
#define PMIC_OTP0_59_W_ADDR(base) ((base) + (0x1DA))
#define PMIC_OTP0_60_W_ADDR(base) ((base) + (0x1DB))
#define PMIC_OTP0_61_W_ADDR(base) ((base) + (0x1DC))
#define PMIC_OTP0_62_W_ADDR(base) ((base) + (0x1DD))
#define PMIC_OTP0_63_W_ADDR(base) ((base) + (0x1DE))
#define PMIC_OTP1_0_ADDR(base) ((base) + (0x1DF))
#define PMIC_OTP1_1_ADDR(base) ((base) + (0x1E0))
#define PMIC_OTP1_CTRL0_ADDR(base) ((base) + (0x1E1))
#define PMIC_OTP1_CTRL1_ADDR(base) ((base) + (0x1E2))
#define PMIC_OTP1_CTRL2_ADDR(base) ((base) + (0x1E3))
#define PMIC_OTP1_WDATA_ADDR(base) ((base) + (0x1E4))
#define PMIC_OTP1_0_W_ADDR(base) ((base) + (0x1E5))
#define PMIC_OTP1_1_W_ADDR(base) ((base) + (0x1E6))
#define PMIC_OTP1_2_W_ADDR(base) ((base) + (0x1E7))
#define PMIC_OTP1_3_W_ADDR(base) ((base) + (0x1E8))
#define PMIC_SPMI_ECO_GT_BYPASS_ADDR(base) ((base) + (0x1ED))
#define PMIC_PWRONN_8S_XOADC_DEBUG_CTRL_ADDR(base) ((base) + (0x1EE))
#define PMIC_DIS_19M2_CTRL_ADDR(base) ((base) + (0x1EF))
#define PMIC_PWRONN_CNT_ADDR(base) ((base) + (0x1F0))
#define PMIC_PWRONN_CNT_MASK_ADDR(base) ((base) + (0x01F1))
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_IRQ_MASK_0_ADDR(base) ((base) + (0x210UL))
#define PMIC_IRQ_MASK_1_ADDR(base) ((base) + (0x211UL))
#define PMIC_IRQ_OCP_MASK0_ADDR(base) ((base) + (0x212UL))
#define PMIC_IRQ_OCP_MASK1_ADDR(base) ((base) + (0x213UL))
#define PMIC_IRQ_OCP_MASK2_ADDR(base) ((base) + (0x214UL))
#define PMIC_IRQ_OCP_MASK3_ADDR(base) ((base) + (0x215UL))
#define PMIC_IRQ_OCP_MASK4_ADDR(base) ((base) + (0x216UL))
#define PMIC_IRQ_OCP_MASK5_ADDR(base) ((base) + (0x217UL))
#define PMIC_IRQ_OCP_MASK6_ADDR(base) ((base) + (0x218UL))
#define PMIC_IRQ_OCP_MASK7_ADDR(base) ((base) + (0x219UL))
#define PMIC_IRQ_SCP_MASK0_ADDR(base) ((base) + (0x21AUL))
#define PMIC_IRQ_SCP_MASK1_ADDR(base) ((base) + (0x21BUL))
#define PMIC_IRQ_OVP_MASK_ADDR(base) ((base) + (0x21CUL))
#define PMIC_IRQ_CUR_DET_MASK_ADDR(base) ((base) + (0x21DUL))
#define PMIC_IRQ_GPIO_MASK_ADDR(base) ((base) + (0x21EUL))
#define PMIC_IRQ_VOICE_RESTART_MASK_ADDR(base) ((base) + (0x21FUL))
#define PMIC_IRQ_MASK_14_ADDR(base) ((base) + (0x220UL))
#define PMIC_IRQ_MASK_15_ADDR(base) ((base) + (0x221UL))
#define PMIC_IRQ_MASK_16_ADDR(base) ((base) + (0x222UL))
#define PMIC_IRQ_MASK_17_ADDR(base) ((base) + (0x223UL))
#define PMIC_IRQ_MASK_18_ADDR(base) ((base) + (0x224UL))
#define PMIC_IRQ_MASK_19_ADDR(base) ((base) + (0x225UL))
#else
#define PMIC_IRQ_MASK_0_ADDR(base) ((base) + (0x210))
#define PMIC_IRQ_MASK_1_ADDR(base) ((base) + (0x211))
#define PMIC_IRQ_OCP_MASK0_ADDR(base) ((base) + (0x212))
#define PMIC_IRQ_OCP_MASK1_ADDR(base) ((base) + (0x213))
#define PMIC_IRQ_OCP_MASK2_ADDR(base) ((base) + (0x214))
#define PMIC_IRQ_OCP_MASK3_ADDR(base) ((base) + (0x215))
#define PMIC_IRQ_OCP_MASK4_ADDR(base) ((base) + (0x216))
#define PMIC_IRQ_OCP_MASK5_ADDR(base) ((base) + (0x217))
#define PMIC_IRQ_OCP_MASK6_ADDR(base) ((base) + (0x218))
#define PMIC_IRQ_OCP_MASK7_ADDR(base) ((base) + (0x219))
#define PMIC_IRQ_SCP_MASK0_ADDR(base) ((base) + (0x21A))
#define PMIC_IRQ_SCP_MASK1_ADDR(base) ((base) + (0x21B))
#define PMIC_IRQ_OVP_MASK_ADDR(base) ((base) + (0x21C))
#define PMIC_IRQ_CUR_DET_MASK_ADDR(base) ((base) + (0x21D))
#define PMIC_IRQ_GPIO_MASK_ADDR(base) ((base) + (0x21E))
#define PMIC_IRQ_VOICE_RESTART_MASK_ADDR(base) ((base) + (0x21F))
#define PMIC_IRQ_MASK_14_ADDR(base) ((base) + (0x220))
#define PMIC_IRQ_MASK_15_ADDR(base) ((base) + (0x221))
#define PMIC_IRQ_MASK_16_ADDR(base) ((base) + (0x222))
#define PMIC_IRQ_MASK_17_ADDR(base) ((base) + (0x223))
#define PMIC_IRQ_MASK_18_ADDR(base) ((base) + (0x224))
#define PMIC_IRQ_MASK_19_ADDR(base) ((base) + (0x225))
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_IRQ0_ADDR(base) ((base) + (0x022AUL))
#define PMIC_IRQ1_ADDR(base) ((base) + (0x022BUL))
#define PMIC_IRQ_OCP0_ADDR(base) ((base) + (0x022CUL))
#define PMIC_IRQ_OCP1_ADDR(base) ((base) + (0x022DUL))
#define PMIC_IRQ_OCP2_ADDR(base) ((base) + (0x022EUL))
#define PMIC_IRQ_OCP3_ADDR(base) ((base) + (0x022FUL))
#define PMIC_IRQ_OCP4_ADDR(base) ((base) + (0x0230UL))
#define PMIC_IRQ_OCP5_ADDR(base) ((base) + (0x0231UL))
#define PMIC_IRQ_OCP6_ADDR(base) ((base) + (0x0232UL))
#define PMIC_IRQ_OCP7_ADDR(base) ((base) + (0x0233UL))
#define PMIC_IRQ_SCP0_ADDR(base) ((base) + (0x0234UL))
#define PMIC_IRQ_SCP1_ADDR(base) ((base) + (0x0235UL))
#define PMIC_IRQ_OVP_ADDR(base) ((base) + (0x0236UL))
#define PMIC_IRQ_CUR_DET_ADDR(base) ((base) + (0x0237UL))
#define PMIC_GPIO_IRQ_ADDR(base) ((base) + (0x0238UL))
#define PMIC_VOICE_RESTART_IRQ_ADDR(base) ((base) + (0x0239UL))
#define PMIC_ACR_SOH_IRQ_ADDR(base) ((base) + (0x023AUL))
#define PMIC_DCR_IRQ_ADDR(base) ((base) + (0x023BUL))
#define PMIC_EIS_IRQ_ADDR(base) ((base) + (0x023CUL))
#define PMIC_IRQ_17_ADDR(base) ((base) + (0x023DUL))
#define PMIC_IRQ_18_ADDR(base) ((base) + (0x023EUL))
#define PMIC_VSYS_DROP_IRQ_ADDR(base) ((base) + (0x023FUL))
#else
#define PMIC_IRQ0_ADDR(base) ((base) + (0x022A))
#define PMIC_IRQ1_ADDR(base) ((base) + (0x022B))
#define PMIC_IRQ_OCP0_ADDR(base) ((base) + (0x022C))
#define PMIC_IRQ_OCP1_ADDR(base) ((base) + (0x022D))
#define PMIC_IRQ_OCP2_ADDR(base) ((base) + (0x022E))
#define PMIC_IRQ_OCP3_ADDR(base) ((base) + (0x022F))
#define PMIC_IRQ_OCP4_ADDR(base) ((base) + (0x0230))
#define PMIC_IRQ_OCP5_ADDR(base) ((base) + (0x0231))
#define PMIC_IRQ_OCP6_ADDR(base) ((base) + (0x0232))
#define PMIC_IRQ_OCP7_ADDR(base) ((base) + (0x0233))
#define PMIC_IRQ_SCP0_ADDR(base) ((base) + (0x0234))
#define PMIC_IRQ_SCP1_ADDR(base) ((base) + (0x0235))
#define PMIC_IRQ_OVP_ADDR(base) ((base) + (0x0236))
#define PMIC_IRQ_CUR_DET_ADDR(base) ((base) + (0x0237))
#define PMIC_GPIO_IRQ_ADDR(base) ((base) + (0x0238))
#define PMIC_VOICE_RESTART_IRQ_ADDR(base) ((base) + (0x0239))
#define PMIC_ACR_SOH_IRQ_ADDR(base) ((base) + (0x023A))
#define PMIC_DCR_IRQ_ADDR(base) ((base) + (0x023B))
#define PMIC_EIS_IRQ_ADDR(base) ((base) + (0x023C))
#define PMIC_IRQ_17_ADDR(base) ((base) + (0x023D))
#define PMIC_IRQ_18_ADDR(base) ((base) + (0x023E))
#define PMIC_VSYS_DROP_IRQ_ADDR(base) ((base) + (0x023F))
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_NP_RECORD0_ADDR(base) ((base) + (0x0244UL))
#define PMIC_NP_RECORD1_ADDR(base) ((base) + (0x0245UL))
#define PMIC_NP_RECORD2_ADDR(base) ((base) + (0x02CEUL))
#define PMIC_NP_RECORD3_ADDR(base) ((base) + (0x0247UL))
#define PMIC_NP_RECORD4_ADDR(base) ((base) + (0x0248UL))
#define PMIC_NP_RECORD5_ADDR(base) ((base) + (0x0249UL))
#define PMIC_NP_OCP0_ADDR(base) ((base) + (0x024AUL))
#define PMIC_NP_OCP1_ADDR(base) ((base) + (0x024BUL))
#define PMIC_NP_OCP2_ADDR(base) ((base) + (0x024CUL))
#define PMIC_NP_OCP3_ADDR(base) ((base) + (0x024DUL))
#define PMIC_NP_OCP4_ADDR(base) ((base) + (0x024EUL))
#define PMIC_NP_OCP5_ADDR(base) ((base) + (0x024FUL))
#define PMIC_NP_OCP6_ADDR(base) ((base) + (0x0250UL))
#define PMIC_NP_OCP7_ADDR(base) ((base) + (0x0251UL))
#define PMIC_NP_SCP0_ADDR(base) ((base) + (0x0252UL))
#define PMIC_NP_SCP1_ADDR(base) ((base) + (0x0253UL))
#define PMIC_NP_OVP_ADDR(base) ((base) + (0x0254UL))
#define PMIC_NP_CUR_DET_ADDR(base) ((base) + (0x0255UL))
#define PMIC_RAMP_EVENT0_ADDR(base) ((base) + (0x0256UL))
#define PMIC_RAMP_EVENT1_ADDR(base) ((base) + (0x0257UL))
#define PMIC_RAMP_EVENT2_ADDR(base) ((base) + (0x0258UL))
#define PMIC_RAMP_EVENT3_ADDR(base) ((base) + (0x0259UL))
#define PMIC_SOH_RECORD_ADDR(base) ((base) + (0x025AUL))
#define PMIC_ACR_SOH_RECORD_ADDR(base) ((base) + (0x025BUL))
#define PMIC_COUL_RECORD0_ADDR(base) ((base) + (0x025CUL))
#define PMIC_COUL_RECORD1_ADDR(base) ((base) + (0x025DUL))
#define PMIC_NP_VSYS_DROP_RECORD_ADDR(base) ((base) + (0x025EUL))
#define PMIC_NP_VSYS_SURGE_RECORD_ADDR(base) ((base) + (0x025FUL))
#else
#define PMIC_NP_RECORD0_ADDR(base) ((base) + (0x0244))
#define PMIC_NP_RECORD1_ADDR(base) ((base) + (0x0245))
#define PMIC_NP_RECORD2_ADDR(base) ((base) + (0x02CE))
#define PMIC_NP_RECORD3_ADDR(base) ((base) + (0x0247))
#define PMIC_NP_RECORD4_ADDR(base) ((base) + (0x0248))
#define PMIC_NP_RECORD5_ADDR(base) ((base) + (0x0249))
#define PMIC_NP_OCP0_ADDR(base) ((base) + (0x024A))
#define PMIC_NP_OCP1_ADDR(base) ((base) + (0x024B))
#define PMIC_NP_OCP2_ADDR(base) ((base) + (0x024C))
#define PMIC_NP_OCP3_ADDR(base) ((base) + (0x024D))
#define PMIC_NP_OCP4_ADDR(base) ((base) + (0x024E))
#define PMIC_NP_OCP5_ADDR(base) ((base) + (0x024F))
#define PMIC_NP_OCP6_ADDR(base) ((base) + (0x0250))
#define PMIC_NP_OCP7_ADDR(base) ((base) + (0x0251))
#define PMIC_NP_SCP0_ADDR(base) ((base) + (0x0252))
#define PMIC_NP_SCP1_ADDR(base) ((base) + (0x0253))
#define PMIC_NP_OVP_ADDR(base) ((base) + (0x0254))
#define PMIC_NP_CUR_DET_ADDR(base) ((base) + (0x0255))
#define PMIC_RAMP_EVENT0_ADDR(base) ((base) + (0x0256))
#define PMIC_RAMP_EVENT1_ADDR(base) ((base) + (0x0257))
#define PMIC_RAMP_EVENT2_ADDR(base) ((base) + (0x0258))
#define PMIC_RAMP_EVENT3_ADDR(base) ((base) + (0x0259))
#define PMIC_SOH_RECORD_ADDR(base) ((base) + (0x025A))
#define PMIC_ACR_SOH_RECORD_ADDR(base) ((base) + (0x025B))
#define PMIC_COUL_RECORD0_ADDR(base) ((base) + (0x025C))
#define PMIC_COUL_RECORD1_ADDR(base) ((base) + (0x025D))
#define PMIC_NP_VSYS_DROP_RECORD_ADDR(base) ((base) + (0x025E))
#define PMIC_NP_VSYS_SURGE_RECORD_ADDR(base) ((base) + (0x025F))
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_ONOFF_PLACE_SEL_ADDR(base) ((base) + (0x270UL))
#define PMIC_LDO0_ON_PLACE_ADDR(base) ((base) + (0x271UL))
#define PMIC_BUCK0_ON_PLACE_ADDR(base) ((base) + (0x272UL))
#define PMIC_BUCK5_ON_PLACE_ADDR(base) ((base) + (0x273UL))
#define PMIC_LDO30_ON_PLACE_ADDR(base) ((base) + (0x274UL))
#define PMIC_LDO39_ON_PLACE_ADDR(base) ((base) + (0x275UL))
#define PMIC_LDO41_ON_PLACE_ADDR(base) ((base) + (0x276UL))
#define PMIC_LDO2_ON_PLACE_ADDR(base) ((base) + (0x277UL))
#define PMIC_LDO18_ON_PLACE_ADDR(base) ((base) + (0x278UL))
#define PMIC_LDO24_ON_PLACE_ADDR(base) ((base) + (0x279UL))
#define PMIC_BUCK1_ON_PLACE_ADDR(base) ((base) + (0x27AUL))
#define PMIC_LDO49_ON_PLACE_ADDR(base) ((base) + (0x27BUL))
#define PMIC_LDO37_ON_PLACE_ADDR(base) ((base) + (0x27CUL))
#define PMIC_LDO5_ON_PLACE_ADDR(base) ((base) + (0x27DUL))
#define PMIC_LDO29_ON_PLACE_ADDR(base) ((base) + (0x27EUL))
#define PMIC_LDO38_ON_PLACE_ADDR(base) ((base) + (0x27FUL))
#define PMIC_LDO50_ON_PLACE_ADDR(base) ((base) + (0x280UL))
#define PMIC_LDO45_ON_PLACE_ADDR(base) ((base) + (0x281UL))
#define PMIC_LDO15_ON_PLACE_ADDR(base) ((base) + (0x282UL))
#define PMIC_LDO23_ON_PLACE_ADDR(base) ((base) + (0x283UL))
#define PMIC_LDO_BUF_ON_PLACE_ADDR(base) ((base) + (0x284UL))
#define PMIC_LDO0_OFF_PLACE_ADDR(base) ((base) + (0x285UL))
#define PMIC_BUCK0_OFF_PLACE_ADDR(base) ((base) + (0x286UL))
#define PMIC_BUCK5_OFF_PLACE_ADDR(base) ((base) + (0x287UL))
#define PMIC_LDO30_OFF_PLACE_ADDR(base) ((base) + (0x288UL))
#define PMIC_LDO39_OFF_PLACE_ADDR(base) ((base) + (0x289UL))
#define PMIC_LDO41_OFF_PLACE_ADDR(base) ((base) + (0x28AUL))
#define PMIC_LDO2_OFF_PLACE_ADDR(base) ((base) + (0x28BUL))
#define PMIC_LDO18_OFF_PLACE_ADDR(base) ((base) + (0x28CUL))
#define PMIC_LDO24_OFF_PLACE_ADDR(base) ((base) + (0x28DUL))
#define PMIC_BUCK1_OFF_PLACE_ADDR(base) ((base) + (0x28EUL))
#define PMIC_LDO49_OFF_PLACE_ADDR(base) ((base) + (0x28FUL))
#define PMIC_LDO37_OFF_PLACE_ADDR(base) ((base) + (0x290UL))
#define PMIC_LDO5_OFF_PLACE_ADDR(base) ((base) + (0x291UL))
#define PMIC_LDO29_OFF_PLACE_ADDR(base) ((base) + (0x292UL))
#define PMIC_LDO38_OFF_PLACE_ADDR(base) ((base) + (0x293UL))
#define PMIC_LDO50_OFF_PLACE_ADDR(base) ((base) + (0x294UL))
#define PMIC_LDO45_OFF_PLACE_ADDR(base) ((base) + (0x295UL))
#define PMIC_LDO15_OFF_PLACE_ADDR(base) ((base) + (0x296UL))
#define PMIC_LDO23_OFF_PLACE_ADDR(base) ((base) + (0x297UL))
#define PMIC_LDO_BUF_OFF_PLACE_ADDR(base) ((base) + (0x298UL))
#define PMIC_HRST_REG0_ADDR(base) ((base) + (0x2A0UL))
#define PMIC_HRST_REG1_ADDR(base) ((base) + (0x2A1UL))
#define PMIC_HRST_REG2_ADDR(base) ((base) + (0x2A2UL))
#define PMIC_HRST_REG3_ADDR(base) ((base) + (0x2A3UL))
#define PMIC_HRST_REG4_ADDR(base) ((base) + (0x2A4UL))
#define PMIC_HRST_REG5_ADDR(base) ((base) + (0x2A5UL))
#define PMIC_HRST_REG6_ADDR(base) ((base) + (0x2A6UL))
#define PMIC_HRST_REG7_ADDR(base) ((base) + (0x2A7UL))
#define PMIC_HRST_REG8_ADDR(base) ((base) + (0x2A8UL))
#define PMIC_HRST_REG9_ADDR(base) ((base) + (0x2A9UL))
#define PMIC_HRST_REG10_ADDR(base) ((base) + (0x2AAUL))
#define PMIC_HRST_REG11_ADDR(base) ((base) + (0x2ABUL))
#define PMIC_HRST_REG12_ADDR(base) ((base) + (0x2ACUL))
#define PMIC_HRST_REG13_ADDR(base) ((base) + (0x2ADUL))
#define PMIC_HRST_REG14_ADDR(base) ((base) + (0x2AEUL))
#define PMIC_HRST_REG15_ADDR(base) ((base) + (0x2AFUL))
#define PMIC_HRST_REG16_ADDR(base) ((base) + (0x2B0UL))
#define PMIC_HRST_REG17_ADDR(base) ((base) + (0x2B1UL))
#define PMIC_HRST_REG18_ADDR(base) ((base) + (0x2B2UL))
#define PMIC_HRST_REG19_ADDR(base) ((base) + (0x2B3UL))
#define PMIC_HRST_REG20_ADDR(base) ((base) + (0x2B4UL))
#define PMIC_HRST_REG21_ADDR(base) ((base) + (0x2B5UL))
#define PMIC_HRST_REG22_ADDR(base) ((base) + (0x2B6UL))
#define PMIC_HRST_REG23_ADDR(base) ((base) + (0x2B7UL))
#define PMIC_HRST_REG24_ADDR(base) ((base) + (0x2B8UL))
#define PMIC_HRST_REG25_ADDR(base) ((base) + (0x2B9UL))
#define PMIC_HRST_REG26_ADDR(base) ((base) + (0x2BAUL))
#define PMIC_HRST_REG27_ADDR(base) ((base) + (0x2BBUL))
#define PMIC_HRST_REG28_ADDR(base) ((base) + (0x2BCUL))
#define PMIC_HRST_REG29_ADDR(base) ((base) + (0x2BDUL))
#define PMIC_HRST_REG30_ADDR(base) ((base) + (0x2BEUL))
#define PMIC_HRST_REG31_ADDR(base) ((base) + (0x2BFUL))
#define PMIC_DEBUG_LOCK_ADDR(base) ((base) + (0x2E0UL))
#define PMIC_SYS_DEBUG0_ADDR(base) ((base) + (0x2E1UL))
#define PMIC_SYS_DEBUG1_ADDR(base) ((base) + (0x2E2UL))
#define PMIC_SYS_DEBUG2_ADDR(base) ((base) + (0x2E3UL))
#define PMIC_SYS_DEBUG3_ADDR(base) ((base) + (0x2E4UL))
#define PMIC_NP_NFC_ON_CTRL_ADDR(base) ((base) + (0x2E5UL))
#define PMIC_NP_CTRL_ADDR(base) ((base) + (0x2E6UL))
#define PMIC_NP_HRESET_PWRDOWN_CTRL_ADDR(base) ((base) + (0x2E7UL))
#define PMIC_NP_SMPL_CTRL_ADDR(base) ((base) + (0x2E8UL))
#define PMIC_NP_SYS_CTRL0_ADDR(base) ((base) + (0x2E9UL))
#define PMIC_NP_SYS_CTRL1_ADDR(base) ((base) + (0x2EAUL))
#define PMIC_NP_SYS_DEBUG2_ADDR(base) ((base) + (0x2EBUL))
#define PMIC_NP_CLK_256K_CTRL0_ADDR(base) ((base) + (0x2ECUL))
#define PMIC_NP_CLK_256K_CTRL1_ADDR(base) ((base) + (0x2EDUL))
#define PMIC_NP_SYS_DEBUG3_ADDR(base) ((base) + (0x2EEUL))
#define PMIC_VSYS_LOW_SET_ADDR(base) ((base) + (0x2EFUL))
#define PMIC_NP_VSYS_DROP_SET_ADDR(base) ((base) + (0x2F0UL))
#define PMIC_NP_EN_PMUD_ADDR(base) ((base) + (0x2F1UL))
#define PMIC_NP_VOICE_RESTART_CTRL_ADDR(base) ((base) + (0x2F2UL))
#define PMIC_NP_CLK_WIFI_CTRL_ADDR(base) ((base) + (0x2F3UL))
#define PMIC_NP_CLK_NFC_CTRL_ADDR(base) ((base) + (0x2F4UL))
#define PMIC_NP_CLK_RF_CTRL_ADDR(base) ((base) + (0x2F5UL))
#define PMIC_NP_CLK_CODEC_CTRL_ADDR(base) ((base) + (0x2F6UL))
#define PMIC_CLK_TOP_CTRL1_0_ADDR(base) ((base) + (0x2F7UL))
#define PMIC_NP_CLK_TOP_CTRL0_ADDR(base) ((base) + (0x2F7UL))
#define PMIC_CLK_TOP_CTRL1_1_ADDR(base) ((base) + (0x2F8UL))
#define PMIC_NP_CLK_TOP_CTRL1_ADDR(base) ((base) + (0x2F8UL))
#define PMIC_NP_CLK_UFS_FRE_CTRL_ADDR(base) ((base) + (0x2F9UL))
#define PMIC_NP_BACKUP_CHG_ADDR(base) ((base) + (0x2FAUL))
#define PMIC_NP_DA_RESERVE0_ADDR(base) ((base) + (0x2FBUL))
#define PMIC_NP_DA_RESERVE1_ADDR(base) ((base) + (0x2FCUL))
#define PMIC_PMUD_VSET_ADDR(base) ((base) + (0x2FDUL))
#define PMIC_PMUD_RES_SEL_ADDR(base) ((base) + (0x2FEUL))
#define PMIC_OTP0_0_R_ADDR(base) ((base) + (0x310UL))
#define PMIC_OTP0_1_R_ADDR(base) ((base) + (0x311UL))
#define PMIC_OTP0_2_R_ADDR(base) ((base) + (0x312UL))
#define PMIC_OTP0_3_R_ADDR(base) ((base) + (0x313UL))
#define PMIC_OTP0_4_R_ADDR(base) ((base) + (0x314UL))
#define PMIC_OTP0_5_R_ADDR(base) ((base) + (0x315UL))
#define PMIC_OTP0_6_R_ADDR(base) ((base) + (0x316UL))
#define PMIC_OTP0_7_R_ADDR(base) ((base) + (0x317UL))
#define PMIC_OTP0_8_R_ADDR(base) ((base) + (0x318UL))
#define PMIC_OTP0_9_R_ADDR(base) ((base) + (0x319UL))
#define PMIC_OTP0_10_R_ADDR(base) ((base) + (0x31AUL))
#define PMIC_OTP0_11_R_ADDR(base) ((base) + (0x31BUL))
#define PMIC_OTP0_12_R_ADDR(base) ((base) + (0x31CUL))
#define PMIC_OTP0_13_R_ADDR(base) ((base) + (0x31DUL))
#define PMIC_OTP0_14_R_ADDR(base) ((base) + (0x31EUL))
#define PMIC_OTP0_15_R_ADDR(base) ((base) + (0x31FUL))
#define PMIC_OTP0_16_R_ADDR(base) ((base) + (0x320UL))
#define PMIC_OTP0_17_R_ADDR(base) ((base) + (0x321UL))
#define PMIC_OTP0_18_R_ADDR(base) ((base) + (0x322UL))
#define PMIC_OTP0_19_R_ADDR(base) ((base) + (0x323UL))
#define PMIC_OTP0_20_R_ADDR(base) ((base) + (0x324UL))
#define PMIC_OTP0_21_R_ADDR(base) ((base) + (0x325UL))
#define PMIC_OTP0_22_R_ADDR(base) ((base) + (0x326UL))
#define PMIC_OTP0_23_R_ADDR(base) ((base) + (0x327UL))
#define PMIC_OTP0_24_R_ADDR(base) ((base) + (0x328UL))
#define PMIC_OTP0_25_R_ADDR(base) ((base) + (0x329UL))
#define PMIC_OTP0_26_R_ADDR(base) ((base) + (0x32AUL))
#define PMIC_OTP0_27_R_ADDR(base) ((base) + (0x32BUL))
#define PMIC_OTP0_28_R_ADDR(base) ((base) + (0x32CUL))
#define PMIC_OTP0_29_R_ADDR(base) ((base) + (0x32DUL))
#define PMIC_OTP0_30_R_ADDR(base) ((base) + (0x32EUL))
#define PMIC_OTP0_31_R_ADDR(base) ((base) + (0x32FUL))
#define PMIC_OTP0_32_R_ADDR(base) ((base) + (0x330UL))
#define PMIC_OTP0_33_R_ADDR(base) ((base) + (0x331UL))
#define PMIC_OTP0_34_R_ADDR(base) ((base) + (0x332UL))
#define PMIC_OTP0_35_R_ADDR(base) ((base) + (0x333UL))
#define PMIC_OTP0_36_R_ADDR(base) ((base) + (0x334UL))
#define PMIC_OTP0_37_R_ADDR(base) ((base) + (0x335UL))
#define PMIC_OTP0_38_R_ADDR(base) ((base) + (0x336UL))
#define PMIC_OTP0_39_R_ADDR(base) ((base) + (0x337UL))
#define PMIC_OTP0_40_R_ADDR(base) ((base) + (0x338UL))
#define PMIC_OTP0_41_R_ADDR(base) ((base) + (0x339UL))
#define PMIC_OTP0_42_R_ADDR(base) ((base) + (0x33AUL))
#define PMIC_OTP0_43_R_ADDR(base) ((base) + (0x33BUL))
#define PMIC_OTP0_44_R_ADDR(base) ((base) + (0x33CUL))
#define PMIC_OTP0_45_R_ADDR(base) ((base) + (0x33DUL))
#define PMIC_OTP0_46_R_ADDR(base) ((base) + (0x33EUL))
#define PMIC_OTP0_47_R_ADDR(base) ((base) + (0x33FUL))
#define PMIC_OTP0_48_R_ADDR(base) ((base) + (0x340UL))
#define PMIC_OTP0_49_R_ADDR(base) ((base) + (0x341UL))
#define PMIC_OTP0_50_R_ADDR(base) ((base) + (0x342UL))
#define PMIC_OTP0_51_R_ADDR(base) ((base) + (0x343UL))
#define PMIC_OTP0_52_R_ADDR(base) ((base) + (0x344UL))
#define PMIC_OTP0_53_R_ADDR(base) ((base) + (0x345UL))
#define PMIC_OTP0_54_R_ADDR(base) ((base) + (0x346UL))
#define PMIC_OTP0_55_R_ADDR(base) ((base) + (0x347UL))
#define PMIC_OTP0_56_R_ADDR(base) ((base) + (0x348UL))
#define PMIC_OTP0_57_R_ADDR(base) ((base) + (0x349UL))
#define PMIC_OTP0_58_R_ADDR(base) ((base) + (0x34AUL))
#define PMIC_OTP0_59_R_ADDR(base) ((base) + (0x34BUL))
#define PMIC_OTP0_60_R_ADDR(base) ((base) + (0x34CUL))
#define PMIC_OTP0_61_R_ADDR(base) ((base) + (0x34DUL))
#define PMIC_OTP0_62_R_ADDR(base) ((base) + (0x34EUL))
#define PMIC_OTP0_63_R_ADDR(base) ((base) + (0x34FUL))
#define PMIC_OTP1_0_R_ADDR(base) ((base) + (0x350UL))
#define PMIC_OTP1_1_R_ADDR(base) ((base) + (0x351UL))
#define PMIC_OTP1_2_R_ADDR(base) ((base) + (0x352UL))
#define PMIC_OTP1_3_R_ADDR(base) ((base) + (0x353UL))
#define PMIC_OTP1_4_R_ADDR(base) ((base) + (0x354UL))
#define PMIC_OTP1_5_R_ADDR(base) ((base) + (0x355UL))
#define PMIC_OTP1_6_R_ADDR(base) ((base) + (0x356UL))
#define PMIC_OTP1_7_R_ADDR(base) ((base) + (0x357UL))
#define PMIC_OTP1_8_R_ADDR(base) ((base) + (0x358UL))
#define PMIC_OTP1_9_R_ADDR(base) ((base) + (0x359UL))
#define PMIC_OTP1_10_R_ADDR(base) ((base) + (0x35AUL))
#define PMIC_OTP1_11_R_ADDR(base) ((base) + (0x35BUL))
#define PMIC_OTP1_12_R_ADDR(base) ((base) + (0x35CUL))
#define PMIC_OTP1_13_R_ADDR(base) ((base) + (0x35DUL))
#define PMIC_OTP1_14_R_ADDR(base) ((base) + (0x35EUL))
#define PMIC_OTP1_15_R_ADDR(base) ((base) + (0x35FUL))
#define PMIC_OTP1_16_R_ADDR(base) ((base) + (0x360UL))
#define PMIC_OTP1_17_R_ADDR(base) ((base) + (0x361UL))
#define PMIC_OTP1_18_R_ADDR(base) ((base) + (0x362UL))
#define PMIC_OTP1_19_R_ADDR(base) ((base) + (0x363UL))
#define PMIC_OTP1_20_R_ADDR(base) ((base) + (0x364UL))
#define PMIC_OTP1_21_R_ADDR(base) ((base) + (0x365UL))
#define PMIC_OTP1_22_R_ADDR(base) ((base) + (0x366UL))
#define PMIC_OTP1_23_R_ADDR(base) ((base) + (0x367UL))
#define PMIC_OTP1_24_R_ADDR(base) ((base) + (0x368UL))
#define PMIC_OTP1_25_R_ADDR(base) ((base) + (0x369UL))
#define PMIC_OTP1_26_R_ADDR(base) ((base) + (0x36AUL))
#define PMIC_OTP1_27_R_ADDR(base) ((base) + (0x36BUL))
#define PMIC_OTP1_28_R_ADDR(base) ((base) + (0x36CUL))
#define PMIC_OTP1_29_R_ADDR(base) ((base) + (0x36DUL))
#define PMIC_OTP1_30_R_ADDR(base) ((base) + (0x36EUL))
#define PMIC_OTP1_31_R_ADDR(base) ((base) + (0x36FUL))
#define PMIC_OTP1_32_R_ADDR(base) ((base) + (0x370UL))
#define PMIC_OTP1_33_R_ADDR(base) ((base) + (0x371UL))
#define PMIC_OTP1_34_R_ADDR(base) ((base) + (0x372UL))
#define PMIC_OTP1_35_R_ADDR(base) ((base) + (0x373UL))
#define PMIC_OTP1_36_R_ADDR(base) ((base) + (0x374UL))
#define PMIC_OTP1_37_R_ADDR(base) ((base) + (0x375UL))
#define PMIC_OTP1_38_R_ADDR(base) ((base) + (0x376UL))
#define PMIC_OTP1_39_R_ADDR(base) ((base) + (0x377UL))
#define PMIC_OTP1_40_R_ADDR(base) ((base) + (0x378UL))
#define PMIC_OTP1_41_R_ADDR(base) ((base) + (0x379UL))
#define PMIC_OTP1_42_R_ADDR(base) ((base) + (0x37AUL))
#define PMIC_OTP1_43_R_ADDR(base) ((base) + (0x37BUL))
#define PMIC_OTP1_44_R_ADDR(base) ((base) + (0x37CUL))
#define PMIC_OTP1_45_R_ADDR(base) ((base) + (0x37DUL))
#define PMIC_OTP1_46_R_ADDR(base) ((base) + (0x37EUL))
#define PMIC_OTP1_47_R_ADDR(base) ((base) + (0x37FUL))
#define PMIC_OTP1_48_R_ADDR(base) ((base) + (0x380UL))
#define PMIC_OTP1_49_R_ADDR(base) ((base) + (0x381UL))
#define PMIC_OTP1_50_R_ADDR(base) ((base) + (0x382UL))
#define PMIC_OTP1_51_R_ADDR(base) ((base) + (0x383UL))
#define PMIC_OTP1_52_R_ADDR(base) ((base) + (0x384UL))
#define PMIC_OTP1_53_R_ADDR(base) ((base) + (0x385UL))
#define PMIC_OTP1_54_R_ADDR(base) ((base) + (0x386UL))
#define PMIC_OTP1_55_R_ADDR(base) ((base) + (0x387UL))
#define PMIC_OTP1_56_R_ADDR(base) ((base) + (0x388UL))
#define PMIC_OTP1_57_R_ADDR(base) ((base) + (0x389UL))
#define PMIC_OTP1_58_R_ADDR(base) ((base) + (0x38AUL))
#define PMIC_OTP1_59_R_ADDR(base) ((base) + (0x38BUL))
#define PMIC_OTP1_60_R_ADDR(base) ((base) + (0x38CUL))
#define PMIC_OTP1_61_R_ADDR(base) ((base) + (0x38DUL))
#define PMIC_OTP1_62_R_ADDR(base) ((base) + (0x38EUL))
#define PMIC_OTP1_63_R_ADDR(base) ((base) + (0x38FUL))
#define PMIC_NP_B40_RESERVE0_ADDR(base) ((base) + (0x390UL))
#else
#define PMIC_ONOFF_PLACE_SEL_ADDR(base) ((base) + (0x270))
#define PMIC_LDO0_ON_PLACE_ADDR(base) ((base) + (0x271))
#define PMIC_BUCK0_ON_PLACE_ADDR(base) ((base) + (0x272))
#define PMIC_BUCK5_ON_PLACE_ADDR(base) ((base) + (0x273))
#define PMIC_LDO30_ON_PLACE_ADDR(base) ((base) + (0x274))
#define PMIC_LDO39_ON_PLACE_ADDR(base) ((base) + (0x275))
#define PMIC_LDO41_ON_PLACE_ADDR(base) ((base) + (0x276))
#define PMIC_LDO2_ON_PLACE_ADDR(base) ((base) + (0x277))
#define PMIC_LDO18_ON_PLACE_ADDR(base) ((base) + (0x278))
#define PMIC_LDO24_ON_PLACE_ADDR(base) ((base) + (0x279))
#define PMIC_BUCK1_ON_PLACE_ADDR(base) ((base) + (0x27A))
#define PMIC_LDO49_ON_PLACE_ADDR(base) ((base) + (0x27B))
#define PMIC_LDO37_ON_PLACE_ADDR(base) ((base) + (0x27C))
#define PMIC_LDO5_ON_PLACE_ADDR(base) ((base) + (0x27D))
#define PMIC_LDO29_ON_PLACE_ADDR(base) ((base) + (0x27E))
#define PMIC_LDO38_ON_PLACE_ADDR(base) ((base) + (0x27F))
#define PMIC_LDO50_ON_PLACE_ADDR(base) ((base) + (0x280))
#define PMIC_LDO45_ON_PLACE_ADDR(base) ((base) + (0x281))
#define PMIC_LDO15_ON_PLACE_ADDR(base) ((base) + (0x282))
#define PMIC_LDO23_ON_PLACE_ADDR(base) ((base) + (0x283))
#define PMIC_LDO_BUF_ON_PLACE_ADDR(base) ((base) + (0x284))
#define PMIC_LDO0_OFF_PLACE_ADDR(base) ((base) + (0x285))
#define PMIC_BUCK0_OFF_PLACE_ADDR(base) ((base) + (0x286))
#define PMIC_BUCK5_OFF_PLACE_ADDR(base) ((base) + (0x287))
#define PMIC_LDO30_OFF_PLACE_ADDR(base) ((base) + (0x288))
#define PMIC_LDO39_OFF_PLACE_ADDR(base) ((base) + (0x289))
#define PMIC_LDO41_OFF_PLACE_ADDR(base) ((base) + (0x28A))
#define PMIC_LDO2_OFF_PLACE_ADDR(base) ((base) + (0x28B))
#define PMIC_LDO18_OFF_PLACE_ADDR(base) ((base) + (0x28C))
#define PMIC_LDO24_OFF_PLACE_ADDR(base) ((base) + (0x28D))
#define PMIC_BUCK1_OFF_PLACE_ADDR(base) ((base) + (0x28E))
#define PMIC_LDO49_OFF_PLACE_ADDR(base) ((base) + (0x28F))
#define PMIC_LDO37_OFF_PLACE_ADDR(base) ((base) + (0x290))
#define PMIC_LDO5_OFF_PLACE_ADDR(base) ((base) + (0x291))
#define PMIC_LDO29_OFF_PLACE_ADDR(base) ((base) + (0x292))
#define PMIC_LDO38_OFF_PLACE_ADDR(base) ((base) + (0x293))
#define PMIC_LDO50_OFF_PLACE_ADDR(base) ((base) + (0x294))
#define PMIC_LDO45_OFF_PLACE_ADDR(base) ((base) + (0x295))
#define PMIC_LDO15_OFF_PLACE_ADDR(base) ((base) + (0x296))
#define PMIC_LDO23_OFF_PLACE_ADDR(base) ((base) + (0x297))
#define PMIC_LDO_BUF_OFF_PLACE_ADDR(base) ((base) + (0x298))
#define PMIC_HRST_REG0_ADDR(base) ((base) + (0x2A0))
#define PMIC_HRST_REG1_ADDR(base) ((base) + (0x2A1))
#define PMIC_HRST_REG2_ADDR(base) ((base) + (0x2A2))
#define PMIC_HRST_REG3_ADDR(base) ((base) + (0x2A3))
#define PMIC_HRST_REG4_ADDR(base) ((base) + (0x2A4))
#define PMIC_HRST_REG5_ADDR(base) ((base) + (0x2A5))
#define PMIC_HRST_REG6_ADDR(base) ((base) + (0x2A6))
#define PMIC_HRST_REG7_ADDR(base) ((base) + (0x2A7))
#define PMIC_HRST_REG8_ADDR(base) ((base) + (0x2A8))
#define PMIC_HRST_REG9_ADDR(base) ((base) + (0x2A9))
#define PMIC_HRST_REG10_ADDR(base) ((base) + (0x2AA))
#define PMIC_HRST_REG11_ADDR(base) ((base) + (0x2AB))
#define PMIC_HRST_REG12_ADDR(base) ((base) + (0x2AC))
#define PMIC_HRST_REG13_ADDR(base) ((base) + (0x2AD))
#define PMIC_HRST_REG14_ADDR(base) ((base) + (0x2AE))
#define PMIC_HRST_REG15_ADDR(base) ((base) + (0x2AF))
#define PMIC_HRST_REG16_ADDR(base) ((base) + (0x2B0))
#define PMIC_HRST_REG17_ADDR(base) ((base) + (0x2B1))
#define PMIC_HRST_REG18_ADDR(base) ((base) + (0x2B2))
#define PMIC_HRST_REG19_ADDR(base) ((base) + (0x2B3))
#define PMIC_HRST_REG20_ADDR(base) ((base) + (0x2B4))
#define PMIC_HRST_REG21_ADDR(base) ((base) + (0x2B5))
#define PMIC_HRST_REG22_ADDR(base) ((base) + (0x2B6))
#define PMIC_HRST_REG23_ADDR(base) ((base) + (0x2B7))
#define PMIC_HRST_REG24_ADDR(base) ((base) + (0x2B8))
#define PMIC_HRST_REG25_ADDR(base) ((base) + (0x2B9))
#define PMIC_HRST_REG26_ADDR(base) ((base) + (0x2BA))
#define PMIC_HRST_REG27_ADDR(base) ((base) + (0x2BB))
#define PMIC_HRST_REG28_ADDR(base) ((base) + (0x2BC))
#define PMIC_HRST_REG29_ADDR(base) ((base) + (0x2BD))
#define PMIC_HRST_REG30_ADDR(base) ((base) + (0x2BE))
#define PMIC_HRST_REG31_ADDR(base) ((base) + (0x2BF))
#define PMIC_DEBUG_LOCK_ADDR(base) ((base) + (0x2E0))
#define PMIC_SYS_DEBUG0_ADDR(base) ((base) + (0x2E1))
#define PMIC_SYS_DEBUG1_ADDR(base) ((base) + (0x2E2))
#define PMIC_SYS_DEBUG2_ADDR(base) ((base) + (0x2E3))
#define PMIC_SYS_DEBUG3_ADDR(base) ((base) + (0x2E4))
#define PMIC_NP_NFC_ON_CTRL_ADDR(base) ((base) + (0x2E5))
#define PMIC_NP_CTRL_ADDR(base) ((base) + (0x2E6))
#define PMIC_NP_HRESET_PWRDOWN_CTRL_ADDR(base) ((base) + (0x2E7))
#define PMIC_NP_SMPL_CTRL_ADDR(base) ((base) + (0x2E8))
#define PMIC_NP_SYS_CTRL0_ADDR(base) ((base) + (0x2E9))
#define PMIC_NP_SYS_CTRL1_ADDR(base) ((base) + (0x2EA))
#define PMIC_NP_SYS_DEBUG2_ADDR(base) ((base) + (0x2EB))
#define PMIC_NP_CLK_256K_CTRL0_ADDR(base) ((base) + (0x2EC))
#define PMIC_NP_CLK_256K_CTRL1_ADDR(base) ((base) + (0x2ED))
#define PMIC_NP_SYS_DEBUG3_ADDR(base) ((base) + (0x2EE))
#define PMIC_VSYS_LOW_SET_ADDR(base) ((base) + (0x2EF))
#define PMIC_NP_VSYS_DROP_SET_ADDR(base) ((base) + (0x2F0))
#define PMIC_NP_EN_PMUD_ADDR(base) ((base) + (0x2F1))
#define PMIC_NP_VOICE_RESTART_CTRL_ADDR(base) ((base) + (0x2F2))
#define PMIC_NP_CLK_WIFI_CTRL_ADDR(base) ((base) + (0x2F3))
#define PMIC_NP_CLK_NFC_CTRL_ADDR(base) ((base) + (0x2F4))
#define PMIC_NP_CLK_RF_CTRL_ADDR(base) ((base) + (0x2F5))
#define PMIC_NP_CLK_CODEC_CTRL_ADDR(base) ((base) + (0x2F6))
#define PMIC_CLK_TOP_CTRL1_0_ADDR(base) ((base) + (0x2F7))
#define PMIC_NP_CLK_TOP_CTRL0_ADDR(base) ((base) + (0x2F7))
#define PMIC_CLK_TOP_CTRL1_1_ADDR(base) ((base) + (0x2F8))
#define PMIC_NP_CLK_TOP_CTRL1_ADDR(base) ((base) + (0x2F8))
#define PMIC_NP_CLK_UFS_FRE_CTRL_ADDR(base) ((base) + (0x2F9))
#define PMIC_NP_BACKUP_CHG_ADDR(base) ((base) + (0x2FA))
#define PMIC_NP_DA_RESERVE0_ADDR(base) ((base) + (0x2FB))
#define PMIC_NP_DA_RESERVE1_ADDR(base) ((base) + (0x2FC))
#define PMIC_PMUD_VSET_ADDR(base) ((base) + (0x2FD))
#define PMIC_PMUD_RES_SEL_ADDR(base) ((base) + (0x2FE))
#define PMIC_OTP0_0_R_ADDR(base) ((base) + (0x310))
#define PMIC_OTP0_1_R_ADDR(base) ((base) + (0x311))
#define PMIC_OTP0_2_R_ADDR(base) ((base) + (0x312))
#define PMIC_OTP0_3_R_ADDR(base) ((base) + (0x313))
#define PMIC_OTP0_4_R_ADDR(base) ((base) + (0x314))
#define PMIC_OTP0_5_R_ADDR(base) ((base) + (0x315))
#define PMIC_OTP0_6_R_ADDR(base) ((base) + (0x316))
#define PMIC_OTP0_7_R_ADDR(base) ((base) + (0x317))
#define PMIC_OTP0_8_R_ADDR(base) ((base) + (0x318))
#define PMIC_OTP0_9_R_ADDR(base) ((base) + (0x319))
#define PMIC_OTP0_10_R_ADDR(base) ((base) + (0x31A))
#define PMIC_OTP0_11_R_ADDR(base) ((base) + (0x31B))
#define PMIC_OTP0_12_R_ADDR(base) ((base) + (0x31C))
#define PMIC_OTP0_13_R_ADDR(base) ((base) + (0x31D))
#define PMIC_OTP0_14_R_ADDR(base) ((base) + (0x31E))
#define PMIC_OTP0_15_R_ADDR(base) ((base) + (0x31F))
#define PMIC_OTP0_16_R_ADDR(base) ((base) + (0x320))
#define PMIC_OTP0_17_R_ADDR(base) ((base) + (0x321))
#define PMIC_OTP0_18_R_ADDR(base) ((base) + (0x322))
#define PMIC_OTP0_19_R_ADDR(base) ((base) + (0x323))
#define PMIC_OTP0_20_R_ADDR(base) ((base) + (0x324))
#define PMIC_OTP0_21_R_ADDR(base) ((base) + (0x325))
#define PMIC_OTP0_22_R_ADDR(base) ((base) + (0x326))
#define PMIC_OTP0_23_R_ADDR(base) ((base) + (0x327))
#define PMIC_OTP0_24_R_ADDR(base) ((base) + (0x328))
#define PMIC_OTP0_25_R_ADDR(base) ((base) + (0x329))
#define PMIC_OTP0_26_R_ADDR(base) ((base) + (0x32A))
#define PMIC_OTP0_27_R_ADDR(base) ((base) + (0x32B))
#define PMIC_OTP0_28_R_ADDR(base) ((base) + (0x32C))
#define PMIC_OTP0_29_R_ADDR(base) ((base) + (0x32D))
#define PMIC_OTP0_30_R_ADDR(base) ((base) + (0x32E))
#define PMIC_OTP0_31_R_ADDR(base) ((base) + (0x32F))
#define PMIC_OTP0_32_R_ADDR(base) ((base) + (0x330))
#define PMIC_OTP0_33_R_ADDR(base) ((base) + (0x331))
#define PMIC_OTP0_34_R_ADDR(base) ((base) + (0x332))
#define PMIC_OTP0_35_R_ADDR(base) ((base) + (0x333))
#define PMIC_OTP0_36_R_ADDR(base) ((base) + (0x334))
#define PMIC_OTP0_37_R_ADDR(base) ((base) + (0x335))
#define PMIC_OTP0_38_R_ADDR(base) ((base) + (0x336))
#define PMIC_OTP0_39_R_ADDR(base) ((base) + (0x337))
#define PMIC_OTP0_40_R_ADDR(base) ((base) + (0x338))
#define PMIC_OTP0_41_R_ADDR(base) ((base) + (0x339))
#define PMIC_OTP0_42_R_ADDR(base) ((base) + (0x33A))
#define PMIC_OTP0_43_R_ADDR(base) ((base) + (0x33B))
#define PMIC_OTP0_44_R_ADDR(base) ((base) + (0x33C))
#define PMIC_OTP0_45_R_ADDR(base) ((base) + (0x33D))
#define PMIC_OTP0_46_R_ADDR(base) ((base) + (0x33E))
#define PMIC_OTP0_47_R_ADDR(base) ((base) + (0x33F))
#define PMIC_OTP0_48_R_ADDR(base) ((base) + (0x340))
#define PMIC_OTP0_49_R_ADDR(base) ((base) + (0x341))
#define PMIC_OTP0_50_R_ADDR(base) ((base) + (0x342))
#define PMIC_OTP0_51_R_ADDR(base) ((base) + (0x343))
#define PMIC_OTP0_52_R_ADDR(base) ((base) + (0x344))
#define PMIC_OTP0_53_R_ADDR(base) ((base) + (0x345))
#define PMIC_OTP0_54_R_ADDR(base) ((base) + (0x346))
#define PMIC_OTP0_55_R_ADDR(base) ((base) + (0x347))
#define PMIC_OTP0_56_R_ADDR(base) ((base) + (0x348))
#define PMIC_OTP0_57_R_ADDR(base) ((base) + (0x349))
#define PMIC_OTP0_58_R_ADDR(base) ((base) + (0x34A))
#define PMIC_OTP0_59_R_ADDR(base) ((base) + (0x34B))
#define PMIC_OTP0_60_R_ADDR(base) ((base) + (0x34C))
#define PMIC_OTP0_61_R_ADDR(base) ((base) + (0x34D))
#define PMIC_OTP0_62_R_ADDR(base) ((base) + (0x34E))
#define PMIC_OTP0_63_R_ADDR(base) ((base) + (0x34F))
#define PMIC_OTP1_0_R_ADDR(base) ((base) + (0x350))
#define PMIC_OTP1_1_R_ADDR(base) ((base) + (0x351))
#define PMIC_OTP1_2_R_ADDR(base) ((base) + (0x352))
#define PMIC_OTP1_3_R_ADDR(base) ((base) + (0x353))
#define PMIC_OTP1_4_R_ADDR(base) ((base) + (0x354))
#define PMIC_OTP1_5_R_ADDR(base) ((base) + (0x355))
#define PMIC_OTP1_6_R_ADDR(base) ((base) + (0x356))
#define PMIC_OTP1_7_R_ADDR(base) ((base) + (0x357))
#define PMIC_OTP1_8_R_ADDR(base) ((base) + (0x358))
#define PMIC_OTP1_9_R_ADDR(base) ((base) + (0x359))
#define PMIC_OTP1_10_R_ADDR(base) ((base) + (0x35A))
#define PMIC_OTP1_11_R_ADDR(base) ((base) + (0x35B))
#define PMIC_OTP1_12_R_ADDR(base) ((base) + (0x35C))
#define PMIC_OTP1_13_R_ADDR(base) ((base) + (0x35D))
#define PMIC_OTP1_14_R_ADDR(base) ((base) + (0x35E))
#define PMIC_OTP1_15_R_ADDR(base) ((base) + (0x35F))
#define PMIC_OTP1_16_R_ADDR(base) ((base) + (0x360))
#define PMIC_OTP1_17_R_ADDR(base) ((base) + (0x361))
#define PMIC_OTP1_18_R_ADDR(base) ((base) + (0x362))
#define PMIC_OTP1_19_R_ADDR(base) ((base) + (0x363))
#define PMIC_OTP1_20_R_ADDR(base) ((base) + (0x364))
#define PMIC_OTP1_21_R_ADDR(base) ((base) + (0x365))
#define PMIC_OTP1_22_R_ADDR(base) ((base) + (0x366))
#define PMIC_OTP1_23_R_ADDR(base) ((base) + (0x367))
#define PMIC_OTP1_24_R_ADDR(base) ((base) + (0x368))
#define PMIC_OTP1_25_R_ADDR(base) ((base) + (0x369))
#define PMIC_OTP1_26_R_ADDR(base) ((base) + (0x36A))
#define PMIC_OTP1_27_R_ADDR(base) ((base) + (0x36B))
#define PMIC_OTP1_28_R_ADDR(base) ((base) + (0x36C))
#define PMIC_OTP1_29_R_ADDR(base) ((base) + (0x36D))
#define PMIC_OTP1_30_R_ADDR(base) ((base) + (0x36E))
#define PMIC_OTP1_31_R_ADDR(base) ((base) + (0x36F))
#define PMIC_OTP1_32_R_ADDR(base) ((base) + (0x370))
#define PMIC_OTP1_33_R_ADDR(base) ((base) + (0x371))
#define PMIC_OTP1_34_R_ADDR(base) ((base) + (0x372))
#define PMIC_OTP1_35_R_ADDR(base) ((base) + (0x373))
#define PMIC_OTP1_36_R_ADDR(base) ((base) + (0x374))
#define PMIC_OTP1_37_R_ADDR(base) ((base) + (0x375))
#define PMIC_OTP1_38_R_ADDR(base) ((base) + (0x376))
#define PMIC_OTP1_39_R_ADDR(base) ((base) + (0x377))
#define PMIC_OTP1_40_R_ADDR(base) ((base) + (0x378))
#define PMIC_OTP1_41_R_ADDR(base) ((base) + (0x379))
#define PMIC_OTP1_42_R_ADDR(base) ((base) + (0x37A))
#define PMIC_OTP1_43_R_ADDR(base) ((base) + (0x37B))
#define PMIC_OTP1_44_R_ADDR(base) ((base) + (0x37C))
#define PMIC_OTP1_45_R_ADDR(base) ((base) + (0x37D))
#define PMIC_OTP1_46_R_ADDR(base) ((base) + (0x37E))
#define PMIC_OTP1_47_R_ADDR(base) ((base) + (0x37F))
#define PMIC_OTP1_48_R_ADDR(base) ((base) + (0x380))
#define PMIC_OTP1_49_R_ADDR(base) ((base) + (0x381))
#define PMIC_OTP1_50_R_ADDR(base) ((base) + (0x382))
#define PMIC_OTP1_51_R_ADDR(base) ((base) + (0x383))
#define PMIC_OTP1_52_R_ADDR(base) ((base) + (0x384))
#define PMIC_OTP1_53_R_ADDR(base) ((base) + (0x385))
#define PMIC_OTP1_54_R_ADDR(base) ((base) + (0x386))
#define PMIC_OTP1_55_R_ADDR(base) ((base) + (0x387))
#define PMIC_OTP1_56_R_ADDR(base) ((base) + (0x388))
#define PMIC_OTP1_57_R_ADDR(base) ((base) + (0x389))
#define PMIC_OTP1_58_R_ADDR(base) ((base) + (0x38A))
#define PMIC_OTP1_59_R_ADDR(base) ((base) + (0x38B))
#define PMIC_OTP1_60_R_ADDR(base) ((base) + (0x38C))
#define PMIC_OTP1_61_R_ADDR(base) ((base) + (0x38D))
#define PMIC_OTP1_62_R_ADDR(base) ((base) + (0x38E))
#define PMIC_OTP1_63_R_ADDR(base) ((base) + (0x38F))
#define PMIC_NP_B40_RESERVE0_ADDR(base) ((base) + (0x390))
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
#define PMIC_DIG_IO_DS_SEL0_ADDR(base) ((base) + (0x03DDUL))
#define PMIC_DIG_IO_DS_SEL1_ADDR(base) ((base) + (0x03DEUL))
#define PMIC_DUMMY_XOADC_SSI_ADDR(base) ((base) + (0x03FFUL))
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
#define PMIC_DIG_IO_DS_SEL0_ADDR(base) ((base) + (0x03DD))
#define PMIC_DIG_IO_DS_SEL1_ADDR(base) ((base) + (0x03DE))
#define PMIC_DUMMY_XOADC_SSI_ADDR(base) ((base) + (0x03FF))
#endif
#define PMU_CTRLB_BASE (0x0400)
#ifndef __SOC_H_FOR_ASM__
#define PMIC_BUCK_TOP_CTRL0_ADDR(base) ((base) + (0x000UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK_TOP_CTRL1_ADDR(base) ((base) + (0x001UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK_TOP_CTRL2_ADDR(base) ((base) + (0x002UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK_TOP_CTRL3_ADDR(base) ((base) + (0x003UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK_TOP_CTRL4_ADDR(base) ((base) + (0x004UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK_TOP_CTRL5_ADDR(base) ((base) + (0x005UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL0_ADDR(base) ((base) + (0x006UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL1_ADDR(base) ((base) + (0x007UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL2_ADDR(base) ((base) + (0x008UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL3_ADDR(base) ((base) + (0x009UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL4_ADDR(base) ((base) + (0x00AUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL5_ADDR(base) ((base) + (0x00BUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL6_ADDR(base) ((base) + (0x00CUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL7_ADDR(base) ((base) + (0x00DUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL8_ADDR(base) ((base) + (0x00EUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL9_ADDR(base) ((base) + (0x00FUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL10_ADDR(base) ((base) + (0x010UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL11_ADDR(base) ((base) + (0x011UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL12_ADDR(base) ((base) + (0x012UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL13_ADDR(base) ((base) + (0x013UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL14_ADDR(base) ((base) + (0x014UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL15_ADDR(base) ((base) + (0x015UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_RO0_ADDR(base) ((base) + (0x016UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL0_ADDR(base) ((base) + (0x017UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL1_ADDR(base) ((base) + (0x018UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL2_ADDR(base) ((base) + (0x019UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL3_ADDR(base) ((base) + (0x01AUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL4_ADDR(base) ((base) + (0x01BUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL5_ADDR(base) ((base) + (0x01CUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL6_ADDR(base) ((base) + (0x01DUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL7_ADDR(base) ((base) + (0x01EUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL8_ADDR(base) ((base) + (0x01FUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL9_ADDR(base) ((base) + (0x020UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL10_ADDR(base) ((base) + (0x021UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL11_ADDR(base) ((base) + (0x022UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL12_ADDR(base) ((base) + (0x023UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL13_ADDR(base) ((base) + (0x024UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL14_ADDR(base) ((base) + (0x025UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_RO0_ADDR(base) ((base) + (0x026UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL0_ADDR(base) ((base) + (0x027UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL1_ADDR(base) ((base) + (0x028UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL2_ADDR(base) ((base) + (0x029UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL3_ADDR(base) ((base) + (0x02AUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL4_ADDR(base) ((base) + (0x02BUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL5_ADDR(base) ((base) + (0x02CUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL6_ADDR(base) ((base) + (0x02DUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL7_ADDR(base) ((base) + (0x02EUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL8_ADDR(base) ((base) + (0x02FUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL9_ADDR(base) ((base) + (0x030UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL10_ADDR(base) ((base) + (0x031UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL11_ADDR(base) ((base) + (0x032UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL12_ADDR(base) ((base) + (0x033UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL13_ADDR(base) ((base) + (0x034UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL14_ADDR(base) ((base) + (0x035UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_RO0_ADDR(base) ((base) + (0x036UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL0_ADDR(base) ((base) + (0x037UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL1_ADDR(base) ((base) + (0x038UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL2_ADDR(base) ((base) + (0x039UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL3_ADDR(base) ((base) + (0x03AUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL4_ADDR(base) ((base) + (0x03BUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL5_ADDR(base) ((base) + (0x03CUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL6_ADDR(base) ((base) + (0x03DUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL7_ADDR(base) ((base) + (0x03EUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL8_ADDR(base) ((base) + (0x03FUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL9_ADDR(base) ((base) + (0x040UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL10_ADDR(base) ((base) + (0x041UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL11_ADDR(base) ((base) + (0x042UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL12_ADDR(base) ((base) + (0x043UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL13_ADDR(base) ((base) + (0x044UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL14_ADDR(base) ((base) + (0x045UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_RO0_ADDR(base) ((base) + (0x046UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL0_ADDR(base) ((base) + (0x047UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL1_ADDR(base) ((base) + (0x048UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL2_ADDR(base) ((base) + (0x049UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL3_ADDR(base) ((base) + (0x04AUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL4_ADDR(base) ((base) + (0x04BUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL5_ADDR(base) ((base) + (0x04CUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL6_ADDR(base) ((base) + (0x04DUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL7_ADDR(base) ((base) + (0x04EUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL8_ADDR(base) ((base) + (0x04FUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL9_ADDR(base) ((base) + (0x050UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL10_ADDR(base) ((base) + (0x051UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL11_ADDR(base) ((base) + (0x052UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL12_ADDR(base) ((base) + (0x053UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL13_ADDR(base) ((base) + (0x054UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL14_ADDR(base) ((base) + (0x055UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL15_ADDR(base) ((base) + (0x056UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL16_ADDR(base) ((base) + (0x057UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL17_ADDR(base) ((base) + (0x058UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL18_ADDR(base) ((base) + (0x059UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL19_ADDR(base) ((base) + (0x05AUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL20_ADDR(base) ((base) + (0x05BUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL21_ADDR(base) ((base) + (0x05CUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL22_ADDR(base) ((base) + (0x05DUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL23_ADDR(base) ((base) + (0x05EUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL24_ADDR(base) ((base) + (0x05FUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL25_ADDR(base) ((base) + (0x060UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL26_ADDR(base) ((base) + (0x061UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL27_ADDR(base) ((base) + (0x062UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL28_ADDR(base) ((base) + (0x063UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL29_ADDR(base) ((base) + (0x064UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL30_ADDR(base) ((base) + (0x065UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL31_ADDR(base) ((base) + (0x066UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL32_ADDR(base) ((base) + (0x067UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL33_ADDR(base) ((base) + (0x068UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL34_ADDR(base) ((base) + (0x069UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL35_ADDR(base) ((base) + (0x06AUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL36_ADDR(base) ((base) + (0x06BUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL37_ADDR(base) ((base) + (0x06CUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL38_ADDR(base) ((base) + (0x06DUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL39_ADDR(base) ((base) + (0x06EUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL40_ADDR(base) ((base) + (0x06FUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL41_ADDR(base) ((base) + (0x070UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL42_ADDR(base) ((base) + (0x071UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL43_ADDR(base) ((base) + (0x072UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL44_ADDR(base) ((base) + (0x073UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL45_ADDR(base) ((base) + (0x074UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL46_ADDR(base) ((base) + (0x075UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL47_ADDR(base) ((base) + (0x076UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL48_ADDR(base) ((base) + (0x077UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL49_ADDR(base) ((base) + (0x078UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL50_ADDR(base) ((base) + (0x079UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL51_ADDR(base) ((base) + (0x07AUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL52_ADDR(base) ((base) + (0x07BUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL53_ADDR(base) ((base) + (0x07CUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL54_ADDR(base) ((base) + (0x07DUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL55_ADDR(base) ((base) + (0x07EUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL56_ADDR(base) ((base) + (0x07FUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL57_ADDR(base) ((base) + (0x080UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL58_ADDR(base) ((base) + (0x081UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL59_ADDR(base) ((base) + (0x082UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL60_ADDR(base) ((base) + (0x083UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL61_ADDR(base) ((base) + (0x084UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL62_ADDR(base) ((base) + (0x085UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL63_ADDR(base) ((base) + (0x086UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL64_ADDR(base) ((base) + (0x087UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL65_ADDR(base) ((base) + (0x088UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL66_ADDR(base) ((base) + (0x089UL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL67_ADDR(base) ((base) + (0x08AUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL68_ADDR(base) ((base) + (0x08BUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL69_ADDR(base) ((base) + (0x08CUL) + (PMU_CTRLB_BASE))
#define PMIC_B401_RO0_ADDR(base) ((base) + (0x08DUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL0_ADDR(base) ((base) + (0x08EUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL1_ADDR(base) ((base) + (0x08FUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL2_ADDR(base) ((base) + (0x090UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL3_ADDR(base) ((base) + (0x091UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL4_ADDR(base) ((base) + (0x092UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL5_ADDR(base) ((base) + (0x093UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL6_ADDR(base) ((base) + (0x094UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL7_ADDR(base) ((base) + (0x095UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL8_ADDR(base) ((base) + (0x096UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL9_ADDR(base) ((base) + (0x097UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL10_ADDR(base) ((base) + (0x098UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL11_ADDR(base) ((base) + (0x099UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL12_ADDR(base) ((base) + (0x09AUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL13_ADDR(base) ((base) + (0x09BUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL14_ADDR(base) ((base) + (0x09CUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL15_ADDR(base) ((base) + (0x09DUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_RO0_ADDR(base) ((base) + (0x09EUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL0_ADDR(base) ((base) + (0x09FUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL1_ADDR(base) ((base) + (0x0A0UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL2_ADDR(base) ((base) + (0x0A1UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL3_ADDR(base) ((base) + (0x0A2UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL4_ADDR(base) ((base) + (0x0A3UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL5_ADDR(base) ((base) + (0x0A4UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL6_ADDR(base) ((base) + (0x0A5UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL7_ADDR(base) ((base) + (0x0A6UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL8_ADDR(base) ((base) + (0x0A7UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL9_ADDR(base) ((base) + (0x0A8UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL10_ADDR(base) ((base) + (0x0A9UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL11_ADDR(base) ((base) + (0x0AAUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL12_ADDR(base) ((base) + (0x0ABUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL13_ADDR(base) ((base) + (0x0ACUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL14_ADDR(base) ((base) + (0x0ADUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_RO0_ADDR(base) ((base) + (0x0AEUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL0_ADDR(base) ((base) + (0x0AFUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL1_ADDR(base) ((base) + (0x0B0UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL2_ADDR(base) ((base) + (0x0B1UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL3_ADDR(base) ((base) + (0x0B2UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL4_ADDR(base) ((base) + (0x0B3UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL5_ADDR(base) ((base) + (0x0B4UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL6_ADDR(base) ((base) + (0x0B5UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL7_ADDR(base) ((base) + (0x0B6UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL8_ADDR(base) ((base) + (0x0B7UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL9_ADDR(base) ((base) + (0x0B8UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL10_ADDR(base) ((base) + (0x0B9UL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL11_ADDR(base) ((base) + (0x0BAUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL12_ADDR(base) ((base) + (0x0BBUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL13_ADDR(base) ((base) + (0x0BCUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL14_ADDR(base) ((base) + (0x0BDUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL15_ADDR(base) ((base) + (0x0BEUL) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_RO0_ADDR(base) ((base) + (0x0BFUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO0_CTRL0_ADDR(base) ((base) + (0x0E0UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO0_CTRL1_ADDR(base) ((base) + (0x0E1UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO1_CTRL0_ADDR(base) ((base) + (0x0E2UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO1_CTRL1_ADDR(base) ((base) + (0x0E3UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO1_CTRL2_ADDR(base) ((base) + (0x0E4UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO1_CTRL3_ADDR(base) ((base) + (0x0E5UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO2_CTRL0_ADDR(base) ((base) + (0x0E6UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO2_CTRL1_ADDR(base) ((base) + (0x0E7UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO3_CTRL0_ADDR(base) ((base) + (0x0E8UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO3_CTRL1_ADDR(base) ((base) + (0x0E9UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO3_CTRL2_ADDR(base) ((base) + (0x0EAUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO4_CTRL0_ADDR(base) ((base) + (0x0EBUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO5_CTRL0_ADDR(base) ((base) + (0x0ECUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO6_CTRL0_ADDR(base) ((base) + (0x0EDUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO6_CTRL1_ADDR(base) ((base) + (0x0EEUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO6_CTRL2_ADDR(base) ((base) + (0x0EFUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO8_CTRL0_ADDR(base) ((base) + (0x0F0UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO9_CTRL0_ADDR(base) ((base) + (0x0F1UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO11_CTRL0_ADDR(base) ((base) + (0x0F2UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO12_CTRL0_ADDR(base) ((base) + (0x0F3UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO13_CTRL0_ADDR(base) ((base) + (0x0F4UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO14_CTRL0_ADDR(base) ((base) + (0x0F5UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO15_CTRL0_ADDR(base) ((base) + (0x0F6UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO16_CTRL0_ADDR(base) ((base) + (0x0F7UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO16_CTRL1_ADDR(base) ((base) + (0x0F8UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO17_CTRL0_ADDR(base) ((base) + (0x0F9UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO18_CTRL0_ADDR(base) ((base) + (0x0FAUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO19_CTRL0_ADDR(base) ((base) + (0x0FBUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO19_CTRL1_ADDR(base) ((base) + (0x0FCUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO20_CTRL0_ADDR(base) ((base) + (0x0FDUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO20_CTRL1_ADDR(base) ((base) + (0x0FEUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO20_CTRL2_ADDR(base) ((base) + (0x0FFUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO20_CTRL3_ADDR(base) ((base) + (0x100UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO20_CTRL4_ADDR(base) ((base) + (0x101UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO21_CTRL0_ADDR(base) ((base) + (0x102UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO22_CTRL0_ADDR(base) ((base) + (0x103UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO22_CTRL1_ADDR(base) ((base) + (0x104UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO23_CTRL0_ADDR(base) ((base) + (0x105UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO24_CTRL0_ADDR(base) ((base) + (0x106UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO24_CTRL1_ADDR(base) ((base) + (0x107UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO25_CTRL0_ADDR(base) ((base) + (0x108UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO26_CTRL0_ADDR(base) ((base) + (0x109UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO27_CTRL0_ADDR(base) ((base) + (0x10AUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO28_CTRL0_ADDR(base) ((base) + (0x10BUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO29_CTRL0_ADDR(base) ((base) + (0x10CUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO29_CTRL1_ADDR(base) ((base) + (0x10DUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO30_CTRL0_ADDR(base) ((base) + (0x10EUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO30_CTRL1_ADDR(base) ((base) + (0x10FUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO30_CTRL2_ADDR(base) ((base) + (0x110UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO31_CTRL0_ADDR(base) ((base) + (0x111UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO32_CTRL0_ADDR(base) ((base) + (0x112UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO32_CTRL1_ADDR(base) ((base) + (0x113UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO32_CTRL2_ADDR(base) ((base) + (0x114UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO32_CTRL3_ADDR(base) ((base) + (0x115UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO32_CTRL4_ADDR(base) ((base) + (0x116UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO34_CTRL0_ADDR(base) ((base) + (0x117UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO34_CTRL1_ADDR(base) ((base) + (0x118UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO37_CTRL0_ADDR(base) ((base) + (0x119UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO37_CTRL1_ADDR(base) ((base) + (0x11AUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO37_CTRL2_ADDR(base) ((base) + (0x11BUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO37_CTRL3_ADDR(base) ((base) + (0x11CUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO37_CTRL4_ADDR(base) ((base) + (0x11DUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO38_CTRL0_ADDR(base) ((base) + (0x11EUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO38_CTRL1_ADDR(base) ((base) + (0x11FUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO39_CTRL0_ADDR(base) ((base) + (0x120UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO39_CTRL1_ADDR(base) ((base) + (0x121UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO39_CTRL2_ADDR(base) ((base) + (0x122UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO41_CTRL0_ADDR(base) ((base) + (0x123UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO41_CTRL1_ADDR(base) ((base) + (0x124UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO41_CTRL2_ADDR(base) ((base) + (0x125UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO42_CTRL0_ADDR(base) ((base) + (0x126UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO42_CTRL1_ADDR(base) ((base) + (0x127UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO42_CTRL2_ADDR(base) ((base) + (0x128UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO43_CTRL0_ADDR(base) ((base) + (0x129UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO43_CTRL1_ADDR(base) ((base) + (0x12AUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO44_CTRL0_ADDR(base) ((base) + (0x12BUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO44_CTRL1_ADDR(base) ((base) + (0x12CUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO44_CTRL2_ADDR(base) ((base) + (0x12DUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO45_CTRL0_ADDR(base) ((base) + (0x12EUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO45_CTRL1_ADDR(base) ((base) + (0x12FUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO45_CTRL2_ADDR(base) ((base) + (0x130UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO45_CTRL3_ADDR(base) ((base) + (0x131UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO45_CTRL4_ADDR(base) ((base) + (0x132UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO46_CTRL0_ADDR(base) ((base) + (0x133UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO46_CTRL1_ADDR(base) ((base) + (0x134UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO46_CTRL2_ADDR(base) ((base) + (0x135UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO46_CTRL3_ADDR(base) ((base) + (0x136UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO47_CTRL0_ADDR(base) ((base) + (0x137UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO47_CTRL1_ADDR(base) ((base) + (0x138UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO47_CTRL2_ADDR(base) ((base) + (0x139UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO48_CTRL0_ADDR(base) ((base) + (0x13AUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO49_CTRL0_ADDR(base) ((base) + (0x13BUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO49_CTRL1_ADDR(base) ((base) + (0x13CUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO49_CTRL2_ADDR(base) ((base) + (0x13DUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO50_CTRL0_ADDR(base) ((base) + (0x13EUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO50_CTRL1_ADDR(base) ((base) + (0x13FUL) + (PMU_CTRLB_BASE))
#define PMIC_LDO_BUF_CTRL0_ADDR(base) ((base) + (0x140UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO_PMUH_CTRL0_ADDR(base) ((base) + (0x141UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO_PMUH_CTRL1_ADDR(base) ((base) + (0x142UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO_RESERVE_CTRL0_ADDR(base) ((base) + (0x143UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO_RESERVE_CTRL1_ADDR(base) ((base) + (0x144UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO_RESERVE_CTRL2_ADDR(base) ((base) + (0x145UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO_RESERVE_CTRL3_ADDR(base) ((base) + (0x146UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO_RESERVE_CTRL4_ADDR(base) ((base) + (0x147UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO_RESERVE_CTRL5_ADDR(base) ((base) + (0x148UL) + (PMU_CTRLB_BASE))
#define PMIC_LDO_RESERVE_CTRL6_ADDR(base) ((base) + (0x149UL) + (PMU_CTRLB_BASE))
#define PMIC_CLK_NFC_DRV_ADDR(base) ((base) + (0x16AUL) + (PMU_CTRLB_BASE))
#define PMIC_CLK_CODEC_DRV_ADDR(base) ((base) + (0x16BUL) + (PMU_CTRLB_BASE))
#define PMIC_CLK_UFS_DRV_ADDR(base) ((base) + (0x16CUL) + (PMU_CTRLB_BASE))
#define PMIC_CLK_SERDES_DRV_ADDR(base) ((base) + (0x16DUL) + (PMU_CTRLB_BASE))
#define PMIC_CLK_WIFI_DRV_ADDR(base) ((base) + (0x16EUL) + (PMU_CTRLB_BASE))
#define PMIC_CLK_SYS_DRV_ADDR(base) ((base) + (0x16FUL) + (PMU_CTRLB_BASE))
#define PMIC_CLK_RF0_DRV_ADDR(base) ((base) + (0x170UL) + (PMU_CTRLB_BASE))
#define PMIC_CLK_TOP_CTRL0_ADDR(base) ((base) + (0x171UL) + (PMU_CTRLB_BASE))
#define PMIC_CLK_TOP_CTRL1_ADDR(base) ((base) + (0x172UL) + (PMU_CTRLB_BASE))
#define PMIC_CLK_TOP_CTRL2_ADDR(base) ((base) + (0x173UL) + (PMU_CTRLB_BASE))
#define PMIC_CLK_TOP_CTRL3_ADDR(base) ((base) + (0x174UL) + (PMU_CTRLB_BASE))
#define PMIC_CLK_TOP_CTRL4_ADDR(base) ((base) + (0x175UL) + (PMU_CTRLB_BASE))
#define PMIC_CLK_TOP_CTRL5_ADDR(base) ((base) + (0x176UL) + (PMU_CTRLB_BASE))
#define PMIC_CLK_TOP_CTRL6_ADDR(base) ((base) + (0x177UL) + (PMU_CTRLB_BASE))
#define PMIC_REF_CTRL0_ADDR(base) ((base) + (0x178UL) + (PMU_CTRLB_BASE))
#define PMIC_REF_CTRL1_ADDR(base) ((base) + (0x179UL) + (PMU_CTRLB_BASE))
#define PMIC_REF_RESERVE0_ADDR(base) ((base) + (0x17AUL) + (PMU_CTRLB_BASE))
#define PMIC_REF_RESERVE1_ADDR(base) ((base) + (0x17BUL) + (PMU_CTRLB_BASE))
#define PMIC_THSD_TMP_SET_ADDR(base) ((base) + (0x17CUL) + (PMU_CTRLB_BASE))
#define PMIC_SYS_CTRL_CTRL0_ADDR(base) ((base) + (0x17DUL) + (PMU_CTRLB_BASE))
#define PMIC_SYS_CTRL_CTRL1_ADDR(base) ((base) + (0x17EUL) + (PMU_CTRLB_BASE))
#define PMIC_SYS_CTRL_CTRL2_ADDR(base) ((base) + (0x17FUL) + (PMU_CTRLB_BASE))
#define PMIC_CLASSD_CTRL0_ADDR(base) ((base) + (0x180UL) + (PMU_CTRLB_BASE))
#define PMIC_CLASSD_CTRL1_ADDR(base) ((base) + (0x181UL) + (PMU_CTRLB_BASE))
#define PMIC_CLASSD_CTRL2_ADDR(base) ((base) + (0x182UL) + (PMU_CTRLB_BASE))
#define PMIC_CLASSD_CTRL3_ADDR(base) ((base) + (0x183UL) + (PMU_CTRLB_BASE))
#define PMIC_D2A_RESERVE0_ADDR(base) ((base) + (0x189UL) + (PMU_CTRLB_BASE))
#define PMIC_D2A_RESERVE1_ADDR(base) ((base) + (0x18AUL) + (PMU_CTRLB_BASE))
#define PMIC_A2D_RESERVE0_ADDR(base) ((base) + (0x18BUL) + (PMU_CTRLB_BASE))
#else
#define PMIC_BUCK_TOP_CTRL0_ADDR(base) ((base) + (0x000) + (PMU_CTRLB_BASE))
#define PMIC_BUCK_TOP_CTRL1_ADDR(base) ((base) + (0x001) + (PMU_CTRLB_BASE))
#define PMIC_BUCK_TOP_CTRL2_ADDR(base) ((base) + (0x002) + (PMU_CTRLB_BASE))
#define PMIC_BUCK_TOP_CTRL3_ADDR(base) ((base) + (0x003) + (PMU_CTRLB_BASE))
#define PMIC_BUCK_TOP_CTRL4_ADDR(base) ((base) + (0x004) + (PMU_CTRLB_BASE))
#define PMIC_BUCK_TOP_CTRL5_ADDR(base) ((base) + (0x005) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL0_ADDR(base) ((base) + (0x006) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL1_ADDR(base) ((base) + (0x007) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL2_ADDR(base) ((base) + (0x008) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL3_ADDR(base) ((base) + (0x009) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL4_ADDR(base) ((base) + (0x00A) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL5_ADDR(base) ((base) + (0x00B) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL6_ADDR(base) ((base) + (0x00C) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL7_ADDR(base) ((base) + (0x00D) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL8_ADDR(base) ((base) + (0x00E) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL9_ADDR(base) ((base) + (0x00F) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL10_ADDR(base) ((base) + (0x010) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL11_ADDR(base) ((base) + (0x011) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL12_ADDR(base) ((base) + (0x012) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL13_ADDR(base) ((base) + (0x013) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL14_ADDR(base) ((base) + (0x014) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_CTRL15_ADDR(base) ((base) + (0x015) + (PMU_CTRLB_BASE))
#define PMIC_BUCK0_RO0_ADDR(base) ((base) + (0x016) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL0_ADDR(base) ((base) + (0x017) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL1_ADDR(base) ((base) + (0x018) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL2_ADDR(base) ((base) + (0x019) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL3_ADDR(base) ((base) + (0x01A) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL4_ADDR(base) ((base) + (0x01B) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL5_ADDR(base) ((base) + (0x01C) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL6_ADDR(base) ((base) + (0x01D) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL7_ADDR(base) ((base) + (0x01E) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL8_ADDR(base) ((base) + (0x01F) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL9_ADDR(base) ((base) + (0x020) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL10_ADDR(base) ((base) + (0x021) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL11_ADDR(base) ((base) + (0x022) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL12_ADDR(base) ((base) + (0x023) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL13_ADDR(base) ((base) + (0x024) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_CTRL14_ADDR(base) ((base) + (0x025) + (PMU_CTRLB_BASE))
#define PMIC_BUCK1_RO0_ADDR(base) ((base) + (0x026) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL0_ADDR(base) ((base) + (0x027) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL1_ADDR(base) ((base) + (0x028) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL2_ADDR(base) ((base) + (0x029) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL3_ADDR(base) ((base) + (0x02A) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL4_ADDR(base) ((base) + (0x02B) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL5_ADDR(base) ((base) + (0x02C) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL6_ADDR(base) ((base) + (0x02D) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL7_ADDR(base) ((base) + (0x02E) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL8_ADDR(base) ((base) + (0x02F) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL9_ADDR(base) ((base) + (0x030) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL10_ADDR(base) ((base) + (0x031) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL11_ADDR(base) ((base) + (0x032) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL12_ADDR(base) ((base) + (0x033) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL13_ADDR(base) ((base) + (0x034) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_CTRL14_ADDR(base) ((base) + (0x035) + (PMU_CTRLB_BASE))
#define PMIC_BUCK2_RO0_ADDR(base) ((base) + (0x036) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL0_ADDR(base) ((base) + (0x037) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL1_ADDR(base) ((base) + (0x038) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL2_ADDR(base) ((base) + (0x039) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL3_ADDR(base) ((base) + (0x03A) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL4_ADDR(base) ((base) + (0x03B) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL5_ADDR(base) ((base) + (0x03C) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL6_ADDR(base) ((base) + (0x03D) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL7_ADDR(base) ((base) + (0x03E) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL8_ADDR(base) ((base) + (0x03F) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL9_ADDR(base) ((base) + (0x040) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL10_ADDR(base) ((base) + (0x041) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL11_ADDR(base) ((base) + (0x042) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL12_ADDR(base) ((base) + (0x043) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL13_ADDR(base) ((base) + (0x044) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_CTRL14_ADDR(base) ((base) + (0x045) + (PMU_CTRLB_BASE))
#define PMIC_BUCK3_RO0_ADDR(base) ((base) + (0x046) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL0_ADDR(base) ((base) + (0x047) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL1_ADDR(base) ((base) + (0x048) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL2_ADDR(base) ((base) + (0x049) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL3_ADDR(base) ((base) + (0x04A) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL4_ADDR(base) ((base) + (0x04B) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL5_ADDR(base) ((base) + (0x04C) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL6_ADDR(base) ((base) + (0x04D) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL7_ADDR(base) ((base) + (0x04E) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL8_ADDR(base) ((base) + (0x04F) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL9_ADDR(base) ((base) + (0x050) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL10_ADDR(base) ((base) + (0x051) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL11_ADDR(base) ((base) + (0x052) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL12_ADDR(base) ((base) + (0x053) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL13_ADDR(base) ((base) + (0x054) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL14_ADDR(base) ((base) + (0x055) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL15_ADDR(base) ((base) + (0x056) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL16_ADDR(base) ((base) + (0x057) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL17_ADDR(base) ((base) + (0x058) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL18_ADDR(base) ((base) + (0x059) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL19_ADDR(base) ((base) + (0x05A) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL20_ADDR(base) ((base) + (0x05B) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL21_ADDR(base) ((base) + (0x05C) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL22_ADDR(base) ((base) + (0x05D) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL23_ADDR(base) ((base) + (0x05E) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL24_ADDR(base) ((base) + (0x05F) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL25_ADDR(base) ((base) + (0x060) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL26_ADDR(base) ((base) + (0x061) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL27_ADDR(base) ((base) + (0x062) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL28_ADDR(base) ((base) + (0x063) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL29_ADDR(base) ((base) + (0x064) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL30_ADDR(base) ((base) + (0x065) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL31_ADDR(base) ((base) + (0x066) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL32_ADDR(base) ((base) + (0x067) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL33_ADDR(base) ((base) + (0x068) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL34_ADDR(base) ((base) + (0x069) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL35_ADDR(base) ((base) + (0x06A) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL36_ADDR(base) ((base) + (0x06B) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL37_ADDR(base) ((base) + (0x06C) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL38_ADDR(base) ((base) + (0x06D) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL39_ADDR(base) ((base) + (0x06E) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL40_ADDR(base) ((base) + (0x06F) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL41_ADDR(base) ((base) + (0x070) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL42_ADDR(base) ((base) + (0x071) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL43_ADDR(base) ((base) + (0x072) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL44_ADDR(base) ((base) + (0x073) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL45_ADDR(base) ((base) + (0x074) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL46_ADDR(base) ((base) + (0x075) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL47_ADDR(base) ((base) + (0x076) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL48_ADDR(base) ((base) + (0x077) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL49_ADDR(base) ((base) + (0x078) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL50_ADDR(base) ((base) + (0x079) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL51_ADDR(base) ((base) + (0x07A) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL52_ADDR(base) ((base) + (0x07B) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL53_ADDR(base) ((base) + (0x07C) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL54_ADDR(base) ((base) + (0x07D) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL55_ADDR(base) ((base) + (0x07E) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL56_ADDR(base) ((base) + (0x07F) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL57_ADDR(base) ((base) + (0x080) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL58_ADDR(base) ((base) + (0x081) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL59_ADDR(base) ((base) + (0x082) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL60_ADDR(base) ((base) + (0x083) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL61_ADDR(base) ((base) + (0x084) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL62_ADDR(base) ((base) + (0x085) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL63_ADDR(base) ((base) + (0x086) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL64_ADDR(base) ((base) + (0x087) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL65_ADDR(base) ((base) + (0x088) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL66_ADDR(base) ((base) + (0x089) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL67_ADDR(base) ((base) + (0x08A) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL68_ADDR(base) ((base) + (0x08B) + (PMU_CTRLB_BASE))
#define PMIC_B401_CTRL69_ADDR(base) ((base) + (0x08C) + (PMU_CTRLB_BASE))
#define PMIC_B401_RO0_ADDR(base) ((base) + (0x08D) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL0_ADDR(base) ((base) + (0x08E) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL1_ADDR(base) ((base) + (0x08F) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL2_ADDR(base) ((base) + (0x090) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL3_ADDR(base) ((base) + (0x091) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL4_ADDR(base) ((base) + (0x092) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL5_ADDR(base) ((base) + (0x093) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL6_ADDR(base) ((base) + (0x094) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL7_ADDR(base) ((base) + (0x095) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL8_ADDR(base) ((base) + (0x096) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL9_ADDR(base) ((base) + (0x097) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL10_ADDR(base) ((base) + (0x098) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL11_ADDR(base) ((base) + (0x099) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL12_ADDR(base) ((base) + (0x09A) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL13_ADDR(base) ((base) + (0x09B) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL14_ADDR(base) ((base) + (0x09C) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_CTRL15_ADDR(base) ((base) + (0x09D) + (PMU_CTRLB_BASE))
#define PMIC_BUCK5_RO0_ADDR(base) ((base) + (0x09E) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL0_ADDR(base) ((base) + (0x09F) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL1_ADDR(base) ((base) + (0x0A0) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL2_ADDR(base) ((base) + (0x0A1) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL3_ADDR(base) ((base) + (0x0A2) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL4_ADDR(base) ((base) + (0x0A3) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL5_ADDR(base) ((base) + (0x0A4) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL6_ADDR(base) ((base) + (0x0A5) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL7_ADDR(base) ((base) + (0x0A6) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL8_ADDR(base) ((base) + (0x0A7) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL9_ADDR(base) ((base) + (0x0A8) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL10_ADDR(base) ((base) + (0x0A9) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL11_ADDR(base) ((base) + (0x0AA) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL12_ADDR(base) ((base) + (0x0AB) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL13_ADDR(base) ((base) + (0x0AC) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_CTRL14_ADDR(base) ((base) + (0x0AD) + (PMU_CTRLB_BASE))
#define PMIC_BUCK6_RO0_ADDR(base) ((base) + (0x0AE) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL0_ADDR(base) ((base) + (0x0AF) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL1_ADDR(base) ((base) + (0x0B0) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL2_ADDR(base) ((base) + (0x0B1) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL3_ADDR(base) ((base) + (0x0B2) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL4_ADDR(base) ((base) + (0x0B3) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL5_ADDR(base) ((base) + (0x0B4) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL6_ADDR(base) ((base) + (0x0B5) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL7_ADDR(base) ((base) + (0x0B6) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL8_ADDR(base) ((base) + (0x0B7) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL9_ADDR(base) ((base) + (0x0B8) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL10_ADDR(base) ((base) + (0x0B9) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL11_ADDR(base) ((base) + (0x0BA) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL12_ADDR(base) ((base) + (0x0BB) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL13_ADDR(base) ((base) + (0x0BC) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL14_ADDR(base) ((base) + (0x0BD) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_CTRL15_ADDR(base) ((base) + (0x0BE) + (PMU_CTRLB_BASE))
#define PMIC_BUCK7_RO0_ADDR(base) ((base) + (0x0BF) + (PMU_CTRLB_BASE))
#define PMIC_LDO0_CTRL0_ADDR(base) ((base) + (0x0E0) + (PMU_CTRLB_BASE))
#define PMIC_LDO0_CTRL1_ADDR(base) ((base) + (0x0E1) + (PMU_CTRLB_BASE))
#define PMIC_LDO1_CTRL0_ADDR(base) ((base) + (0x0E2) + (PMU_CTRLB_BASE))
#define PMIC_LDO1_CTRL1_ADDR(base) ((base) + (0x0E3) + (PMU_CTRLB_BASE))
#define PMIC_LDO1_CTRL2_ADDR(base) ((base) + (0x0E4) + (PMU_CTRLB_BASE))
#define PMIC_LDO1_CTRL3_ADDR(base) ((base) + (0x0E5) + (PMU_CTRLB_BASE))
#define PMIC_LDO2_CTRL0_ADDR(base) ((base) + (0x0E6) + (PMU_CTRLB_BASE))
#define PMIC_LDO2_CTRL1_ADDR(base) ((base) + (0x0E7) + (PMU_CTRLB_BASE))
#define PMIC_LDO3_CTRL0_ADDR(base) ((base) + (0x0E8) + (PMU_CTRLB_BASE))
#define PMIC_LDO3_CTRL1_ADDR(base) ((base) + (0x0E9) + (PMU_CTRLB_BASE))
#define PMIC_LDO3_CTRL2_ADDR(base) ((base) + (0x0EA) + (PMU_CTRLB_BASE))
#define PMIC_LDO4_CTRL0_ADDR(base) ((base) + (0x0EB) + (PMU_CTRLB_BASE))
#define PMIC_LDO5_CTRL0_ADDR(base) ((base) + (0x0EC) + (PMU_CTRLB_BASE))
#define PMIC_LDO6_CTRL0_ADDR(base) ((base) + (0x0ED) + (PMU_CTRLB_BASE))
#define PMIC_LDO6_CTRL1_ADDR(base) ((base) + (0x0EE) + (PMU_CTRLB_BASE))
#define PMIC_LDO6_CTRL2_ADDR(base) ((base) + (0x0EF) + (PMU_CTRLB_BASE))
#define PMIC_LDO8_CTRL0_ADDR(base) ((base) + (0x0F0) + (PMU_CTRLB_BASE))
#define PMIC_LDO9_CTRL0_ADDR(base) ((base) + (0x0F1) + (PMU_CTRLB_BASE))
#define PMIC_LDO11_CTRL0_ADDR(base) ((base) + (0x0F2) + (PMU_CTRLB_BASE))
#define PMIC_LDO12_CTRL0_ADDR(base) ((base) + (0x0F3) + (PMU_CTRLB_BASE))
#define PMIC_LDO13_CTRL0_ADDR(base) ((base) + (0x0F4) + (PMU_CTRLB_BASE))
#define PMIC_LDO14_CTRL0_ADDR(base) ((base) + (0x0F5) + (PMU_CTRLB_BASE))
#define PMIC_LDO15_CTRL0_ADDR(base) ((base) + (0x0F6) + (PMU_CTRLB_BASE))
#define PMIC_LDO16_CTRL0_ADDR(base) ((base) + (0x0F7) + (PMU_CTRLB_BASE))
#define PMIC_LDO16_CTRL1_ADDR(base) ((base) + (0x0F8) + (PMU_CTRLB_BASE))
#define PMIC_LDO17_CTRL0_ADDR(base) ((base) + (0x0F9) + (PMU_CTRLB_BASE))
#define PMIC_LDO18_CTRL0_ADDR(base) ((base) + (0x0FA) + (PMU_CTRLB_BASE))
#define PMIC_LDO19_CTRL0_ADDR(base) ((base) + (0x0FB) + (PMU_CTRLB_BASE))
#define PMIC_LDO19_CTRL1_ADDR(base) ((base) + (0x0FC) + (PMU_CTRLB_BASE))
#define PMIC_LDO20_CTRL0_ADDR(base) ((base) + (0x0FD) + (PMU_CTRLB_BASE))
#define PMIC_LDO20_CTRL1_ADDR(base) ((base) + (0x0FE) + (PMU_CTRLB_BASE))
#define PMIC_LDO20_CTRL2_ADDR(base) ((base) + (0x0FF) + (PMU_CTRLB_BASE))
#define PMIC_LDO20_CTRL3_ADDR(base) ((base) + (0x100) + (PMU_CTRLB_BASE))
#define PMIC_LDO20_CTRL4_ADDR(base) ((base) + (0x101) + (PMU_CTRLB_BASE))
#define PMIC_LDO21_CTRL0_ADDR(base) ((base) + (0x102) + (PMU_CTRLB_BASE))
#define PMIC_LDO22_CTRL0_ADDR(base) ((base) + (0x103) + (PMU_CTRLB_BASE))
#define PMIC_LDO22_CTRL1_ADDR(base) ((base) + (0x104) + (PMU_CTRLB_BASE))
#define PMIC_LDO23_CTRL0_ADDR(base) ((base) + (0x105) + (PMU_CTRLB_BASE))
#define PMIC_LDO24_CTRL0_ADDR(base) ((base) + (0x106) + (PMU_CTRLB_BASE))
#define PMIC_LDO24_CTRL1_ADDR(base) ((base) + (0x107) + (PMU_CTRLB_BASE))
#define PMIC_LDO25_CTRL0_ADDR(base) ((base) + (0x108) + (PMU_CTRLB_BASE))
#define PMIC_LDO26_CTRL0_ADDR(base) ((base) + (0x109) + (PMU_CTRLB_BASE))
#define PMIC_LDO27_CTRL0_ADDR(base) ((base) + (0x10A) + (PMU_CTRLB_BASE))
#define PMIC_LDO28_CTRL0_ADDR(base) ((base) + (0x10B) + (PMU_CTRLB_BASE))
#define PMIC_LDO29_CTRL0_ADDR(base) ((base) + (0x10C) + (PMU_CTRLB_BASE))
#define PMIC_LDO29_CTRL1_ADDR(base) ((base) + (0x10D) + (PMU_CTRLB_BASE))
#define PMIC_LDO30_CTRL0_ADDR(base) ((base) + (0x10E) + (PMU_CTRLB_BASE))
#define PMIC_LDO30_CTRL1_ADDR(base) ((base) + (0x10F) + (PMU_CTRLB_BASE))
#define PMIC_LDO30_CTRL2_ADDR(base) ((base) + (0x110) + (PMU_CTRLB_BASE))
#define PMIC_LDO31_CTRL0_ADDR(base) ((base) + (0x111) + (PMU_CTRLB_BASE))
#define PMIC_LDO32_CTRL0_ADDR(base) ((base) + (0x112) + (PMU_CTRLB_BASE))
#define PMIC_LDO32_CTRL1_ADDR(base) ((base) + (0x113) + (PMU_CTRLB_BASE))
#define PMIC_LDO32_CTRL2_ADDR(base) ((base) + (0x114) + (PMU_CTRLB_BASE))
#define PMIC_LDO32_CTRL3_ADDR(base) ((base) + (0x115) + (PMU_CTRLB_BASE))
#define PMIC_LDO32_CTRL4_ADDR(base) ((base) + (0x116) + (PMU_CTRLB_BASE))
#define PMIC_LDO34_CTRL0_ADDR(base) ((base) + (0x117) + (PMU_CTRLB_BASE))
#define PMIC_LDO34_CTRL1_ADDR(base) ((base) + (0x118) + (PMU_CTRLB_BASE))
#define PMIC_LDO37_CTRL0_ADDR(base) ((base) + (0x119) + (PMU_CTRLB_BASE))
#define PMIC_LDO37_CTRL1_ADDR(base) ((base) + (0x11A) + (PMU_CTRLB_BASE))
#define PMIC_LDO37_CTRL2_ADDR(base) ((base) + (0x11B) + (PMU_CTRLB_BASE))
#define PMIC_LDO37_CTRL3_ADDR(base) ((base) + (0x11C) + (PMU_CTRLB_BASE))
#define PMIC_LDO37_CTRL4_ADDR(base) ((base) + (0x11D) + (PMU_CTRLB_BASE))
#define PMIC_LDO38_CTRL0_ADDR(base) ((base) + (0x11E) + (PMU_CTRLB_BASE))
#define PMIC_LDO38_CTRL1_ADDR(base) ((base) + (0x11F) + (PMU_CTRLB_BASE))
#define PMIC_LDO39_CTRL0_ADDR(base) ((base) + (0x120) + (PMU_CTRLB_BASE))
#define PMIC_LDO39_CTRL1_ADDR(base) ((base) + (0x121) + (PMU_CTRLB_BASE))
#define PMIC_LDO39_CTRL2_ADDR(base) ((base) + (0x122) + (PMU_CTRLB_BASE))
#define PMIC_LDO41_CTRL0_ADDR(base) ((base) + (0x123) + (PMU_CTRLB_BASE))
#define PMIC_LDO41_CTRL1_ADDR(base) ((base) + (0x124) + (PMU_CTRLB_BASE))
#define PMIC_LDO41_CTRL2_ADDR(base) ((base) + (0x125) + (PMU_CTRLB_BASE))
#define PMIC_LDO42_CTRL0_ADDR(base) ((base) + (0x126) + (PMU_CTRLB_BASE))
#define PMIC_LDO42_CTRL1_ADDR(base) ((base) + (0x127) + (PMU_CTRLB_BASE))
#define PMIC_LDO42_CTRL2_ADDR(base) ((base) + (0x128) + (PMU_CTRLB_BASE))
#define PMIC_LDO43_CTRL0_ADDR(base) ((base) + (0x129) + (PMU_CTRLB_BASE))
#define PMIC_LDO43_CTRL1_ADDR(base) ((base) + (0x12A) + (PMU_CTRLB_BASE))
#define PMIC_LDO44_CTRL0_ADDR(base) ((base) + (0x12B) + (PMU_CTRLB_BASE))
#define PMIC_LDO44_CTRL1_ADDR(base) ((base) + (0x12C) + (PMU_CTRLB_BASE))
#define PMIC_LDO44_CTRL2_ADDR(base) ((base) + (0x12D) + (PMU_CTRLB_BASE))
#define PMIC_LDO45_CTRL0_ADDR(base) ((base) + (0x12E) + (PMU_CTRLB_BASE))
#define PMIC_LDO45_CTRL1_ADDR(base) ((base) + (0x12F) + (PMU_CTRLB_BASE))
#define PMIC_LDO45_CTRL2_ADDR(base) ((base) + (0x130) + (PMU_CTRLB_BASE))
#define PMIC_LDO45_CTRL3_ADDR(base) ((base) + (0x131) + (PMU_CTRLB_BASE))
#define PMIC_LDO45_CTRL4_ADDR(base) ((base) + (0x132) + (PMU_CTRLB_BASE))
#define PMIC_LDO46_CTRL0_ADDR(base) ((base) + (0x133) + (PMU_CTRLB_BASE))
#define PMIC_LDO46_CTRL1_ADDR(base) ((base) + (0x134) + (PMU_CTRLB_BASE))
#define PMIC_LDO46_CTRL2_ADDR(base) ((base) + (0x135) + (PMU_CTRLB_BASE))
#define PMIC_LDO46_CTRL3_ADDR(base) ((base) + (0x136) + (PMU_CTRLB_BASE))
#define PMIC_LDO47_CTRL0_ADDR(base) ((base) + (0x137) + (PMU_CTRLB_BASE))
#define PMIC_LDO47_CTRL1_ADDR(base) ((base) + (0x138) + (PMU_CTRLB_BASE))
#define PMIC_LDO47_CTRL2_ADDR(base) ((base) + (0x139) + (PMU_CTRLB_BASE))
#define PMIC_LDO48_CTRL0_ADDR(base) ((base) + (0x13A) + (PMU_CTRLB_BASE))
#define PMIC_LDO49_CTRL0_ADDR(base) ((base) + (0x13B) + (PMU_CTRLB_BASE))
#define PMIC_LDO49_CTRL1_ADDR(base) ((base) + (0x13C) + (PMU_CTRLB_BASE))
#define PMIC_LDO49_CTRL2_ADDR(base) ((base) + (0x13D) + (PMU_CTRLB_BASE))
#define PMIC_LDO50_CTRL0_ADDR(base) ((base) + (0x13E) + (PMU_CTRLB_BASE))
#define PMIC_LDO50_CTRL1_ADDR(base) ((base) + (0x13F) + (PMU_CTRLB_BASE))
#define PMIC_LDO_BUF_CTRL0_ADDR(base) ((base) + (0x140) + (PMU_CTRLB_BASE))
#define PMIC_LDO_PMUH_CTRL0_ADDR(base) ((base) + (0x141) + (PMU_CTRLB_BASE))
#define PMIC_LDO_PMUH_CTRL1_ADDR(base) ((base) + (0x142) + (PMU_CTRLB_BASE))
#define PMIC_LDO_RESERVE_CTRL0_ADDR(base) ((base) + (0x143) + (PMU_CTRLB_BASE))
#define PMIC_LDO_RESERVE_CTRL1_ADDR(base) ((base) + (0x144) + (PMU_CTRLB_BASE))
#define PMIC_LDO_RESERVE_CTRL2_ADDR(base) ((base) + (0x145) + (PMU_CTRLB_BASE))
#define PMIC_LDO_RESERVE_CTRL3_ADDR(base) ((base) + (0x146) + (PMU_CTRLB_BASE))
#define PMIC_LDO_RESERVE_CTRL4_ADDR(base) ((base) + (0x147) + (PMU_CTRLB_BASE))
#define PMIC_LDO_RESERVE_CTRL5_ADDR(base) ((base) + (0x148) + (PMU_CTRLB_BASE))
#define PMIC_LDO_RESERVE_CTRL6_ADDR(base) ((base) + (0x149) + (PMU_CTRLB_BASE))
#define PMIC_CLK_NFC_DRV_ADDR(base) ((base) + (0x16A) + (PMU_CTRLB_BASE))
#define PMIC_CLK_CODEC_DRV_ADDR(base) ((base) + (0x16B) + (PMU_CTRLB_BASE))
#define PMIC_CLK_UFS_DRV_ADDR(base) ((base) + (0x16C) + (PMU_CTRLB_BASE))
#define PMIC_CLK_SERDES_DRV_ADDR(base) ((base) + (0x16D) + (PMU_CTRLB_BASE))
#define PMIC_CLK_WIFI_DRV_ADDR(base) ((base) + (0x16E) + (PMU_CTRLB_BASE))
#define PMIC_CLK_SYS_DRV_ADDR(base) ((base) + (0x16F) + (PMU_CTRLB_BASE))
#define PMIC_CLK_RF0_DRV_ADDR(base) ((base) + (0x170) + (PMU_CTRLB_BASE))
#define PMIC_CLK_TOP_CTRL0_ADDR(base) ((base) + (0x171) + (PMU_CTRLB_BASE))
#define PMIC_CLK_TOP_CTRL1_ADDR(base) ((base) + (0x172) + (PMU_CTRLB_BASE))
#define PMIC_CLK_TOP_CTRL2_ADDR(base) ((base) + (0x173) + (PMU_CTRLB_BASE))
#define PMIC_CLK_TOP_CTRL3_ADDR(base) ((base) + (0x174) + (PMU_CTRLB_BASE))
#define PMIC_CLK_TOP_CTRL4_ADDR(base) ((base) + (0x175) + (PMU_CTRLB_BASE))
#define PMIC_CLK_TOP_CTRL5_ADDR(base) ((base) + (0x176) + (PMU_CTRLB_BASE))
#define PMIC_CLK_TOP_CTRL6_ADDR(base) ((base) + (0x177) + (PMU_CTRLB_BASE))
#define PMIC_REF_CTRL0_ADDR(base) ((base) + (0x178) + (PMU_CTRLB_BASE))
#define PMIC_REF_CTRL1_ADDR(base) ((base) + (0x179) + (PMU_CTRLB_BASE))
#define PMIC_REF_RESERVE0_ADDR(base) ((base) + (0x17A) + (PMU_CTRLB_BASE))
#define PMIC_REF_RESERVE1_ADDR(base) ((base) + (0x17B) + (PMU_CTRLB_BASE))
#define PMIC_THSD_TMP_SET_ADDR(base) ((base) + (0x17C) + (PMU_CTRLB_BASE))
#define PMIC_SYS_CTRL_CTRL0_ADDR(base) ((base) + (0x17D) + (PMU_CTRLB_BASE))
#define PMIC_SYS_CTRL_CTRL1_ADDR(base) ((base) + (0x17E) + (PMU_CTRLB_BASE))
#define PMIC_SYS_CTRL_CTRL2_ADDR(base) ((base) + (0x17F) + (PMU_CTRLB_BASE))
#define PMIC_CLASSD_CTRL0_ADDR(base) ((base) + (0x180) + (PMU_CTRLB_BASE))
#define PMIC_CLASSD_CTRL1_ADDR(base) ((base) + (0x181) + (PMU_CTRLB_BASE))
#define PMIC_CLASSD_CTRL2_ADDR(base) ((base) + (0x182) + (PMU_CTRLB_BASE))
#define PMIC_CLASSD_CTRL3_ADDR(base) ((base) + (0x183) + (PMU_CTRLB_BASE))
#define PMIC_D2A_RESERVE0_ADDR(base) ((base) + (0x189) + (PMU_CTRLB_BASE))
#define PMIC_D2A_RESERVE1_ADDR(base) ((base) + (0x18A) + (PMU_CTRLB_BASE))
#define PMIC_A2D_RESERVE0_ADDR(base) ((base) + (0x18B) + (PMU_CTRLB_BASE))
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_SER_RTCDR0_ADDR(base) ((base) + (0x0C00UL))
#define PMIC_SER_RTCDR1_ADDR(base) ((base) + (0x0C01UL))
#define PMIC_SER_RTCDR2_ADDR(base) ((base) + (0x0C02UL))
#define PMIC_SER_RTCDR3_ADDR(base) ((base) + (0x0C03UL))
#define PMIC_SER_RTCMR0_ADDR(base) ((base) + (0x0C04UL))
#define PMIC_SER_RTCMR1_ADDR(base) ((base) + (0x0C05UL))
#define PMIC_SER_RTCMR2_ADDR(base) ((base) + (0x0C06UL))
#define PMIC_SER_RTCMR3_ADDR(base) ((base) + (0x0C07UL))
#define PMIC_SER_RTCLR0_ADDR(base) ((base) + (0x0C08UL))
#define PMIC_SER_RTCLR1_ADDR(base) ((base) + (0x0C09UL))
#define PMIC_SER_RTCLR2_ADDR(base) ((base) + (0x0C0AUL))
#define PMIC_SER_RTCLR3_ADDR(base) ((base) + (0x0C0BUL))
#define PMIC_SER_RTCCTRL_ADDR(base) ((base) + (0x0C0CUL))
#define PMIC_SER_XO_THRESOLD0_ADDR(base) ((base) + (0x0C0DUL))
#define PMIC_SER_XO_THRESOLD1_ADDR(base) ((base) + (0x0C0EUL))
#define PMIC_SER_CRC_VAULE0_ADDR(base) ((base) + (0x0C0FUL))
#define PMIC_SER_CRC_VAULE1_ADDR(base) ((base) + (0x0C10UL))
#define PMIC_SER_CRC_VAULE2_ADDR(base) ((base) + (0x0C11UL))
#define PMIC_SER_RTC_PWRUP_TIMER0_ADDR(base) ((base) + (0x0C12UL))
#define PMIC_SER_RTC_PWRUP_TIMER1_ADDR(base) ((base) + (0x0C13UL))
#define PMIC_SER_RTC_PWRUP_TIMER2_ADDR(base) ((base) + (0x0C14UL))
#define PMIC_SER_RTC_PWRUP_TIMER3_ADDR(base) ((base) + (0x0C15UL))
#define PMIC_SER_RTC_PWRDOWN_TIMER0_ADDR(base) ((base) + (0x0C16UL))
#define PMIC_SER_RTC_PWRDOWN_TIMER1_ADDR(base) ((base) + (0x0C17UL))
#define PMIC_SER_RTC_PWRDOWN_TIMER2_ADDR(base) ((base) + (0x0C18UL))
#define PMIC_SER_RTC_PWRDOWN_TIMER3_ADDR(base) ((base) + (0x0C19UL))
#define PMIC_SER_SIDEKEY_MODE_ADDR(base) ((base) + (0x0C20UL))
#define PMIC_LDO33_ONOFF1_ECO_ADDR(base) ((base) + (0x0C30UL))
#define PMIC_LDO33_ONOFF2_ADDR(base) ((base) + (0x0C31UL))
#define PMIC_LDO33_VSET_ADDR(base) ((base) + (0x0C32UL))
#define PMIC_LDO33_CFG0_ADDR(base) ((base) + (0x0C33UL))
#define PMIC_LDO40_ONOFF_ECO_ADDR(base) ((base) + (0x0C80UL))
#define PMIC_LDO40_VSET_ADDR(base) ((base) + (0x0C81UL))
#define PMIC_RAMP_LDO40_CTRL0_ADDR(base) ((base) + (0x0C85UL))
#define PMIC_RAMP_LDO40_CTRL1_ADDR(base) ((base) + (0x0C86UL))
#define PMIC_LDO40_CFG0_ADDR(base) ((base) + (0x0C8AUL))
#define PMIC_LDO40_CFG1_ADDR(base) ((base) + (0x0C8BUL))
#define PMIC_LDO40_CFG2_ADDR(base) ((base) + (0x0C8CUL))
#else
#define PMIC_SER_RTCDR0_ADDR(base) ((base) + (0x0C00))
#define PMIC_SER_RTCDR1_ADDR(base) ((base) + (0x0C01))
#define PMIC_SER_RTCDR2_ADDR(base) ((base) + (0x0C02))
#define PMIC_SER_RTCDR3_ADDR(base) ((base) + (0x0C03))
#define PMIC_SER_RTCMR0_ADDR(base) ((base) + (0x0C04))
#define PMIC_SER_RTCMR1_ADDR(base) ((base) + (0x0C05))
#define PMIC_SER_RTCMR2_ADDR(base) ((base) + (0x0C06))
#define PMIC_SER_RTCMR3_ADDR(base) ((base) + (0x0C07))
#define PMIC_SER_RTCLR0_ADDR(base) ((base) + (0x0C08))
#define PMIC_SER_RTCLR1_ADDR(base) ((base) + (0x0C09))
#define PMIC_SER_RTCLR2_ADDR(base) ((base) + (0x0C0A))
#define PMIC_SER_RTCLR3_ADDR(base) ((base) + (0x0C0B))
#define PMIC_SER_RTCCTRL_ADDR(base) ((base) + (0x0C0C))
#define PMIC_SER_XO_THRESOLD0_ADDR(base) ((base) + (0x0C0D))
#define PMIC_SER_XO_THRESOLD1_ADDR(base) ((base) + (0x0C0E))
#define PMIC_SER_CRC_VAULE0_ADDR(base) ((base) + (0x0C0F))
#define PMIC_SER_CRC_VAULE1_ADDR(base) ((base) + (0x0C10))
#define PMIC_SER_CRC_VAULE2_ADDR(base) ((base) + (0x0C11))
#define PMIC_SER_RTC_PWRUP_TIMER0_ADDR(base) ((base) + (0x0C12))
#define PMIC_SER_RTC_PWRUP_TIMER1_ADDR(base) ((base) + (0x0C13))
#define PMIC_SER_RTC_PWRUP_TIMER2_ADDR(base) ((base) + (0x0C14))
#define PMIC_SER_RTC_PWRUP_TIMER3_ADDR(base) ((base) + (0x0C15))
#define PMIC_SER_RTC_PWRDOWN_TIMER0_ADDR(base) ((base) + (0x0C16))
#define PMIC_SER_RTC_PWRDOWN_TIMER1_ADDR(base) ((base) + (0x0C17))
#define PMIC_SER_RTC_PWRDOWN_TIMER2_ADDR(base) ((base) + (0x0C18))
#define PMIC_SER_RTC_PWRDOWN_TIMER3_ADDR(base) ((base) + (0x0C19))
#define PMIC_SER_SIDEKEY_MODE_ADDR(base) ((base) + (0x0C20))
#define PMIC_LDO33_ONOFF1_ECO_ADDR(base) ((base) + (0x0C30))
#define PMIC_LDO33_ONOFF2_ADDR(base) ((base) + (0x0C31))
#define PMIC_LDO33_VSET_ADDR(base) ((base) + (0x0C32))
#define PMIC_LDO33_CFG0_ADDR(base) ((base) + (0x0C33))
#define PMIC_LDO40_ONOFF_ECO_ADDR(base) ((base) + (0x0C80))
#define PMIC_LDO40_VSET_ADDR(base) ((base) + (0x0C81))
#define PMIC_RAMP_LDO40_CTRL0_ADDR(base) ((base) + (0x0C85))
#define PMIC_RAMP_LDO40_CTRL1_ADDR(base) ((base) + (0x0C86))
#define PMIC_LDO40_CFG0_ADDR(base) ((base) + (0x0C8A))
#define PMIC_LDO40_CFG1_ADDR(base) ((base) + (0x0C8B))
#define PMIC_LDO40_CFG2_ADDR(base) ((base) + (0x0C8C))
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
#define PMIC_FARY_MODE_ADDR(base) ((base) + (0x40D3UL))
#define PMIC_FARY_DELAY_ADDR(base) ((base) + (0x40D4UL))
#define PMIC_FARY_CTRL0_ADDR(base) ((base) + (0x40D5UL))
#define PMIC_FARY_VBAT0_S0_ADDR(base) ((base) + (0x40D6UL))
#define PMIC_FARY_VBAT0_S1_ADDR(base) ((base) + (0x40D7UL))
#define PMIC_FARY_VBAT0_S2_ADDR(base) ((base) + (0x40D8UL))
#define PMIC_FARY_VBAT1_S0_ADDR(base) ((base) + (0x40D9UL))
#define PMIC_FARY_VBAT1_S1_ADDR(base) ((base) + (0x40DAUL))
#define PMIC_FARY_VBAT1_S2_ADDR(base) ((base) + (0x40DBUL))
#define PMIC_FARY_IBAT0_S0_ADDR(base) ((base) + (0x40DCUL))
#define PMIC_FARY_IBAT0_S1_ADDR(base) ((base) + (0x40DDUL))
#define PMIC_FARY_IBAT0_S2_ADDR(base) ((base) + (0x40DEUL))
#define PMIC_FARY_IBAT1_S0_ADDR(base) ((base) + (0x40DFUL))
#define PMIC_FARY_IBAT1_S1_ADDR(base) ((base) + (0x40E0UL))
#define PMIC_FARY_IBAT1_S2_ADDR(base) ((base) + (0x40E1UL))
#define PMIC_FARY_VSYS0_S0_ADDR(base) ((base) + (0x40E2UL))
#define PMIC_FARY_VSYS0_S1_ADDR(base) ((base) + (0x40E3UL))
#define PMIC_FARY_VSYS0_S2_ADDR(base) ((base) + (0x40E4UL))
#define PMIC_FARY_VSYS1_S0_ADDR(base) ((base) + (0x40E5UL))
#define PMIC_FARY_VSYS1_S1_ADDR(base) ((base) + (0x40E6UL))
#define PMIC_FARY_VSYS1_S2_ADDR(base) ((base) + (0x40E7UL))
#define PMIC_FARY_ISYS0_S0_ADDR(base) ((base) + (0x40E8UL))
#define PMIC_FARY_ISYS0_S1_ADDR(base) ((base) + (0x40E9UL))
#define PMIC_FARY_ISYS0_S2_ADDR(base) ((base) + (0x40EAUL))
#define PMIC_FARY_ISYS1_S0_ADDR(base) ((base) + (0x40EBUL))
#define PMIC_FARY_ISYS1_S1_ADDR(base) ((base) + (0x40ECUL))
#define PMIC_FARY_ISYS1_S2_ADDR(base) ((base) + (0x40EDUL))
#define PMIC_FARY_RESERVE0_ADDR(base) ((base) + (0x40EFUL))
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
#define PMIC_FARY_MODE_ADDR(base) ((base) + (0x40D3))
#define PMIC_FARY_DELAY_ADDR(base) ((base) + (0x40D4))
#define PMIC_FARY_CTRL0_ADDR(base) ((base) + (0x40D5))
#define PMIC_FARY_VBAT0_S0_ADDR(base) ((base) + (0x40D6))
#define PMIC_FARY_VBAT0_S1_ADDR(base) ((base) + (0x40D7))
#define PMIC_FARY_VBAT0_S2_ADDR(base) ((base) + (0x40D8))
#define PMIC_FARY_VBAT1_S0_ADDR(base) ((base) + (0x40D9))
#define PMIC_FARY_VBAT1_S1_ADDR(base) ((base) + (0x40DA))
#define PMIC_FARY_VBAT1_S2_ADDR(base) ((base) + (0x40DB))
#define PMIC_FARY_IBAT0_S0_ADDR(base) ((base) + (0x40DC))
#define PMIC_FARY_IBAT0_S1_ADDR(base) ((base) + (0x40DD))
#define PMIC_FARY_IBAT0_S2_ADDR(base) ((base) + (0x40DE))
#define PMIC_FARY_IBAT1_S0_ADDR(base) ((base) + (0x40DF))
#define PMIC_FARY_IBAT1_S1_ADDR(base) ((base) + (0x40E0))
#define PMIC_FARY_IBAT1_S2_ADDR(base) ((base) + (0x40E1))
#define PMIC_FARY_VSYS0_S0_ADDR(base) ((base) + (0x40E2))
#define PMIC_FARY_VSYS0_S1_ADDR(base) ((base) + (0x40E3))
#define PMIC_FARY_VSYS0_S2_ADDR(base) ((base) + (0x40E4))
#define PMIC_FARY_VSYS1_S0_ADDR(base) ((base) + (0x40E5))
#define PMIC_FARY_VSYS1_S1_ADDR(base) ((base) + (0x40E6))
#define PMIC_FARY_VSYS1_S2_ADDR(base) ((base) + (0x40E7))
#define PMIC_FARY_ISYS0_S0_ADDR(base) ((base) + (0x40E8))
#define PMIC_FARY_ISYS0_S1_ADDR(base) ((base) + (0x40E9))
#define PMIC_FARY_ISYS0_S2_ADDR(base) ((base) + (0x40EA))
#define PMIC_FARY_ISYS1_S0_ADDR(base) ((base) + (0x40EB))
#define PMIC_FARY_ISYS1_S1_ADDR(base) ((base) + (0x40EC))
#define PMIC_FARY_ISYS1_S2_ADDR(base) ((base) + (0x40ED))
#define PMIC_FARY_RESERVE0_ADDR(base) ((base) + (0x40EF))
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
#define PMIC_OVP_DISCHARGE_CTRL_ADDR(base) ((base) + (0x809EUL))
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
#define PMIC_OVP_DISCHARGE_CTRL_ADDR(base) ((base) + (0x809E))
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
#define PMIC_EIS_ANA_CTRL0_ADDR(base) ((base) + (0xA000UL))
#define PMIC_EIS_ANA_CTRL1_ADDR(base) ((base) + (0xA001UL))
#define PMIC_EIS_ANA_CTRL2_ADDR(base) ((base) + (0xA002UL))
#define PMIC_EIS_IBIAS_SET_0_ADDR(base) ((base) + (0xA003UL))
#define PMIC_EIS_IBIAS_SET_1_ADDR(base) ((base) + (0xA004UL))
#define PMIC_EIS_TEST_CTRL_ADDR(base) ((base) + (0xA005UL))
#define PMIC_EIS_RESERVE0_ADDR(base) ((base) + (0xA006UL))
#define PMIC_EIS_RESERVE1_ADDR(base) ((base) + (0xA007UL))
#define PMIC_EIS_RESERVE2_ADDR(base) ((base) + (0xA008UL))
#define PMIC_EIS_CTRL0_ADDR(base) ((base) + (0xA010UL))
#define PMIC_EIS_CTRL1_ADDR(base) ((base) + (0xA011UL))
#define PMIC_EIS_CTRL2_ADDR(base) ((base) + (0xA012UL))
#define PMIC_EISADC_INIT_V_OFFSET_DATA_L_ADDR(base) ((base) + (0xA013UL))
#define PMIC_EISADC_INIT_V_OFFSET_DATA_H_ADDR(base) ((base) + (0xA014UL))
#define PMIC_EISADC_INIT_I_OFFSET_DATA_L_ADDR(base) ((base) + (0xA015UL))
#define PMIC_EISADC_INIT_I_OFFSET_DATA_H_ADDR(base) ((base) + (0xA016UL))
#define PMIC_EISADC_INIT_I_DATA_L_ADDR(base) ((base) + (0xA017UL))
#define PMIC_EISADC_INIT_I_DATA_H_ADDR(base) ((base) + (0xA018UL))
#define PMIC_EISADC_T_DATA_L_ADDR(base) ((base) + (0xA019UL))
#define PMIC_EISADC_T_DATA_H_ADDR(base) ((base) + (0xA01AUL))
#define PMIC_EISADC_INIT_V_ABS_DATA_L_ADDR(base) ((base) + (0xA01BUL))
#define PMIC_EISADC_INIT_V_ABS_DATA_H_ADDR(base) ((base) + (0xA01CUL))
#define PMIC_EISADC_INIT_V_RELA_DATA_L_ADDR(base) ((base) + (0xA01DUL))
#define PMIC_EISADC_INIT_V_RELA_DATA_H_ADDR(base) ((base) + (0xA01EUL))
#define PMIC_EISADC_INIT_VBAT0_DATA_L_ADDR(base) ((base) + (0xA01FUL))
#define PMIC_EISADC_INIT_VBAT0_DATA_H_ADDR(base) ((base) + (0xA020UL))
#define PMIC_EIS_DET_TIMER_ADDR(base) ((base) + (0xA021UL))
#define PMIC_EIS_DET_TIMER_N_ADDR(base) ((base) + (0xA022UL))
#define PMIC_EIS_DET_TIMER_M_L_ADDR(base) ((base) + (0xA023UL))
#define PMIC_EIS_DET_TIMER_M_H_ADDR(base) ((base) + (0xA024UL))
#define PMIC_EIS_IBAT_DET_CTRL_ADDR(base) ((base) + (0xA025UL))
#define PMIC_EIS_ADC_AVERAGE_ADDR(base) ((base) + (0xA026UL))
#define PMIC_EIS_VBAT0_OFFSET_L_ADDR(base) ((base) + (0xA027UL))
#define PMIC_EIS_VBAT0_OFFSET_H_ADDR(base) ((base) + (0xA028UL))
#define PMIC_EIS_VBAT0_OFFSET_SEL_ADDR(base) ((base) + (0xA029UL))
#define PMIC_EIS_HTHRESHOLD_ERROR_ADDR(base) ((base) + (0xA02AUL))
#define PMIC_EIS_ICOMP_HTHRESHOLD_L_ADDR(base) ((base) + (0xA02BUL))
#define PMIC_EIS_ICOMP_HTHRESHOLD_H_ADDR(base) ((base) + (0xA02CUL))
#define PMIC_EIS_LTHRESHOLD_ERROR_ADDR(base) ((base) + (0xA02DUL))
#define PMIC_EIS_ICOMP_LTHRESHOLD_L_ADDR(base) ((base) + (0xA02EUL))
#define PMIC_EIS_ICOMP_LTHRESHOLD_H_ADDR(base) ((base) + (0xA02FUL))
#define PMIC_EIS_ICOMP_STATE_CLR_ADDR(base) ((base) + (0xA030UL))
#define PMIC_EIS_I_CURRENT_L_ADDR(base) ((base) + (0xA031UL))
#define PMIC_EIS_I_CURRENT_H_ADDR(base) ((base) + (0xA032UL))
#define PMIC_EIS_VBAT_D1_L_ADDR(base) ((base) + (0xA033UL))
#define PMIC_EIS_VBAT_D1_H_ADDR(base) ((base) + (0xA034UL))
#define PMIC_EIS_VBAT_D2_L_ADDR(base) ((base) + (0xA035UL))
#define PMIC_EIS_VBAT_D2_H_ADDR(base) ((base) + (0xA036UL))
#define PMIC_EIS_VBAT_D3_L_ADDR(base) ((base) + (0xA037UL))
#define PMIC_EIS_VBAT_D3_H_ADDR(base) ((base) + (0xA038UL))
#define PMIC_EIS_VBAT_D4_L_ADDR(base) ((base) + (0xA039UL))
#define PMIC_EIS_VBAT_D4_H_ADDR(base) ((base) + (0xA03AUL))
#define PMIC_EIS_VBAT_D5_L_ADDR(base) ((base) + (0xA03BUL))
#define PMIC_EIS_VBAT_D5_H_ADDR(base) ((base) + (0xA03CUL))
#define PMIC_EIS_VBAT_D6_L_ADDR(base) ((base) + (0xA03DUL))
#define PMIC_EIS_VBAT_D6_H_ADDR(base) ((base) + (0xA03EUL))
#define PMIC_EIS_VBAT_D7_L_ADDR(base) ((base) + (0xA03FUL))
#define PMIC_EIS_VBAT_D7_H_ADDR(base) ((base) + (0xA040UL))
#define PMIC_EIS_VBAT_D8_L_ADDR(base) ((base) + (0xA041UL))
#define PMIC_EIS_VBAT_D8_H_ADDR(base) ((base) + (0xA042UL))
#define PMIC_EIS_VBAT_D9_L_ADDR(base) ((base) + (0xA043UL))
#define PMIC_EIS_VBAT_D9_H_ADDR(base) ((base) + (0xA044UL))
#define PMIC_EIS_VBAT_D10_L_ADDR(base) ((base) + (0xA045UL))
#define PMIC_EIS_VBAT_D10_H_ADDR(base) ((base) + (0xA046UL))
#define PMIC_EIS_VBAT_D11_L_ADDR(base) ((base) + (0xA047UL))
#define PMIC_EIS_VBAT_D11_H_ADDR(base) ((base) + (0xA048UL))
#define PMIC_EIS_VBAT_D12_L_ADDR(base) ((base) + (0xA049UL))
#define PMIC_EIS_VBAT_D12_H_ADDR(base) ((base) + (0xA04AUL))
#define PMIC_EIS_VBAT_D13_L_ADDR(base) ((base) + (0xA04BUL))
#define PMIC_EIS_VBAT_D13_H_ADDR(base) ((base) + (0xA04CUL))
#define PMIC_EIS_VBAT_D14_L_ADDR(base) ((base) + (0xA04DUL))
#define PMIC_EIS_VBAT_D14_H_ADDR(base) ((base) + (0xA04EUL))
#define PMIC_EIS_VBAT_D15_L_ADDR(base) ((base) + (0xA04FUL))
#define PMIC_EIS_VBAT_D15_H_ADDR(base) ((base) + (0xA050UL))
#define PMIC_EIS_VBAT_D16_L_ADDR(base) ((base) + (0xA051UL))
#define PMIC_EIS_VBAT_D16_H_ADDR(base) ((base) + (0xA052UL))
#define PMIC_EIS_IBAT_P_D4_L_ADDR(base) ((base) + (0xA053UL))
#define PMIC_EIS_IBAT_P_D4_H_ADDR(base) ((base) + (0xA054UL))
#define PMIC_EIS_IBAT_P_D8_L_ADDR(base) ((base) + (0xA055UL))
#define PMIC_EIS_IBAT_P_D8_H_ADDR(base) ((base) + (0xA056UL))
#define PMIC_EIS_IBAT_P_D12_L_ADDR(base) ((base) + (0xA057UL))
#define PMIC_EIS_IBAT_P_D12_H_ADDR(base) ((base) + (0xA058UL))
#define PMIC_EIS_IBAT_P_D16_L_ADDR(base) ((base) + (0xA059UL))
#define PMIC_EIS_IBAT_P_D16_H_ADDR(base) ((base) + (0xA05AUL))
#define PMIC_EIS_DET_T_NUM_L_ADDR(base) ((base) + (0xA05BUL))
#define PMIC_EIS_DET_T_NUM_H_ADDR(base) ((base) + (0xA05CUL))
#define PMIC_EIS_VBAT_DET_NUM_ADDR(base) ((base) + (0xA05DUL))
#define PMIC_EIS_DET_FLAG_ADDR(base) ((base) + (0xA05EUL))
#define PMIC_EIS_INIT_END_FLAG_ADDR(base) ((base) + (0xA05FUL))
#define PMIC_EISADC_RESERVE_ADDR(base) ((base) + (0xA070UL))
#define PMIC_EISADC_CONV_STATUS_ADDR(base) ((base) + (0xA071UL))
#define PMIC_EISADC_DATA_L_ADDR(base) ((base) + (0xA072UL))
#define PMIC_EISADC_DATA_H_ADDR(base) ((base) + (0xA073UL))
#define PMIC_EISADC_CONV_ADDR(base) ((base) + (0xA074UL))
#define PMIC_EIS_ADC_CURRENT_ADDR(base) ((base) + (0xA075UL))
#define PMIC_EIS_SOFT_RST_ADDR(base) ((base) + (0xA076UL))
#define PMIC_EIS_CLK_GT_CTRL_ADDR(base) ((base) + (0xA077UL))
#define PMIC_EIS_DEBUG_EN_ADDR(base) ((base) + (0xA080UL))
#define PMIC_DEBUG_EIS_EN_ADDR(base) ((base) + (0xA081UL))
#define PMIC_DEBUG_EIS_DISCHG_EN_ADDR(base) ((base) + (0xA082UL))
#define PMIC_DEBUG_EIS_CHANL_ADDR(base) ((base) + (0xA083UL))
#define PMIC_DEBUG_EISADC_CTRL_ADDR(base) ((base) + (0xA084UL))
#define PMIC_DEBUG_EISADC_START_ADDR(base) ((base) + (0xA085UL))
#define PMIC_EISADC_MODE_CFG_ADDR(base) ((base) + (0xA086UL))
#else
#define PMIC_EIS_ANA_CTRL0_ADDR(base) ((base) + (0xA000))
#define PMIC_EIS_ANA_CTRL1_ADDR(base) ((base) + (0xA001))
#define PMIC_EIS_ANA_CTRL2_ADDR(base) ((base) + (0xA002))
#define PMIC_EIS_IBIAS_SET_0_ADDR(base) ((base) + (0xA003))
#define PMIC_EIS_IBIAS_SET_1_ADDR(base) ((base) + (0xA004))
#define PMIC_EIS_TEST_CTRL_ADDR(base) ((base) + (0xA005))
#define PMIC_EIS_RESERVE0_ADDR(base) ((base) + (0xA006))
#define PMIC_EIS_RESERVE1_ADDR(base) ((base) + (0xA007))
#define PMIC_EIS_RESERVE2_ADDR(base) ((base) + (0xA008))
#define PMIC_EIS_CTRL0_ADDR(base) ((base) + (0xA010))
#define PMIC_EIS_CTRL1_ADDR(base) ((base) + (0xA011))
#define PMIC_EIS_CTRL2_ADDR(base) ((base) + (0xA012))
#define PMIC_EISADC_INIT_V_OFFSET_DATA_L_ADDR(base) ((base) + (0xA013))
#define PMIC_EISADC_INIT_V_OFFSET_DATA_H_ADDR(base) ((base) + (0xA014))
#define PMIC_EISADC_INIT_I_OFFSET_DATA_L_ADDR(base) ((base) + (0xA015))
#define PMIC_EISADC_INIT_I_OFFSET_DATA_H_ADDR(base) ((base) + (0xA016))
#define PMIC_EISADC_INIT_I_DATA_L_ADDR(base) ((base) + (0xA017))
#define PMIC_EISADC_INIT_I_DATA_H_ADDR(base) ((base) + (0xA018))
#define PMIC_EISADC_T_DATA_L_ADDR(base) ((base) + (0xA019))
#define PMIC_EISADC_T_DATA_H_ADDR(base) ((base) + (0xA01A))
#define PMIC_EISADC_INIT_V_ABS_DATA_L_ADDR(base) ((base) + (0xA01B))
#define PMIC_EISADC_INIT_V_ABS_DATA_H_ADDR(base) ((base) + (0xA01C))
#define PMIC_EISADC_INIT_V_RELA_DATA_L_ADDR(base) ((base) + (0xA01D))
#define PMIC_EISADC_INIT_V_RELA_DATA_H_ADDR(base) ((base) + (0xA01E))
#define PMIC_EISADC_INIT_VBAT0_DATA_L_ADDR(base) ((base) + (0xA01F))
#define PMIC_EISADC_INIT_VBAT0_DATA_H_ADDR(base) ((base) + (0xA020))
#define PMIC_EIS_DET_TIMER_ADDR(base) ((base) + (0xA021))
#define PMIC_EIS_DET_TIMER_N_ADDR(base) ((base) + (0xA022))
#define PMIC_EIS_DET_TIMER_M_L_ADDR(base) ((base) + (0xA023))
#define PMIC_EIS_DET_TIMER_M_H_ADDR(base) ((base) + (0xA024))
#define PMIC_EIS_IBAT_DET_CTRL_ADDR(base) ((base) + (0xA025))
#define PMIC_EIS_ADC_AVERAGE_ADDR(base) ((base) + (0xA026))
#define PMIC_EIS_VBAT0_OFFSET_L_ADDR(base) ((base) + (0xA027))
#define PMIC_EIS_VBAT0_OFFSET_H_ADDR(base) ((base) + (0xA028))
#define PMIC_EIS_VBAT0_OFFSET_SEL_ADDR(base) ((base) + (0xA029))
#define PMIC_EIS_HTHRESHOLD_ERROR_ADDR(base) ((base) + (0xA02A))
#define PMIC_EIS_ICOMP_HTHRESHOLD_L_ADDR(base) ((base) + (0xA02B))
#define PMIC_EIS_ICOMP_HTHRESHOLD_H_ADDR(base) ((base) + (0xA02C))
#define PMIC_EIS_LTHRESHOLD_ERROR_ADDR(base) ((base) + (0xA02D))
#define PMIC_EIS_ICOMP_LTHRESHOLD_L_ADDR(base) ((base) + (0xA02E))
#define PMIC_EIS_ICOMP_LTHRESHOLD_H_ADDR(base) ((base) + (0xA02F))
#define PMIC_EIS_ICOMP_STATE_CLR_ADDR(base) ((base) + (0xA030))
#define PMIC_EIS_I_CURRENT_L_ADDR(base) ((base) + (0xA031))
#define PMIC_EIS_I_CURRENT_H_ADDR(base) ((base) + (0xA032))
#define PMIC_EIS_VBAT_D1_L_ADDR(base) ((base) + (0xA033))
#define PMIC_EIS_VBAT_D1_H_ADDR(base) ((base) + (0xA034))
#define PMIC_EIS_VBAT_D2_L_ADDR(base) ((base) + (0xA035))
#define PMIC_EIS_VBAT_D2_H_ADDR(base) ((base) + (0xA036))
#define PMIC_EIS_VBAT_D3_L_ADDR(base) ((base) + (0xA037))
#define PMIC_EIS_VBAT_D3_H_ADDR(base) ((base) + (0xA038))
#define PMIC_EIS_VBAT_D4_L_ADDR(base) ((base) + (0xA039))
#define PMIC_EIS_VBAT_D4_H_ADDR(base) ((base) + (0xA03A))
#define PMIC_EIS_VBAT_D5_L_ADDR(base) ((base) + (0xA03B))
#define PMIC_EIS_VBAT_D5_H_ADDR(base) ((base) + (0xA03C))
#define PMIC_EIS_VBAT_D6_L_ADDR(base) ((base) + (0xA03D))
#define PMIC_EIS_VBAT_D6_H_ADDR(base) ((base) + (0xA03E))
#define PMIC_EIS_VBAT_D7_L_ADDR(base) ((base) + (0xA03F))
#define PMIC_EIS_VBAT_D7_H_ADDR(base) ((base) + (0xA040))
#define PMIC_EIS_VBAT_D8_L_ADDR(base) ((base) + (0xA041))
#define PMIC_EIS_VBAT_D8_H_ADDR(base) ((base) + (0xA042))
#define PMIC_EIS_VBAT_D9_L_ADDR(base) ((base) + (0xA043))
#define PMIC_EIS_VBAT_D9_H_ADDR(base) ((base) + (0xA044))
#define PMIC_EIS_VBAT_D10_L_ADDR(base) ((base) + (0xA045))
#define PMIC_EIS_VBAT_D10_H_ADDR(base) ((base) + (0xA046))
#define PMIC_EIS_VBAT_D11_L_ADDR(base) ((base) + (0xA047))
#define PMIC_EIS_VBAT_D11_H_ADDR(base) ((base) + (0xA048))
#define PMIC_EIS_VBAT_D12_L_ADDR(base) ((base) + (0xA049))
#define PMIC_EIS_VBAT_D12_H_ADDR(base) ((base) + (0xA04A))
#define PMIC_EIS_VBAT_D13_L_ADDR(base) ((base) + (0xA04B))
#define PMIC_EIS_VBAT_D13_H_ADDR(base) ((base) + (0xA04C))
#define PMIC_EIS_VBAT_D14_L_ADDR(base) ((base) + (0xA04D))
#define PMIC_EIS_VBAT_D14_H_ADDR(base) ((base) + (0xA04E))
#define PMIC_EIS_VBAT_D15_L_ADDR(base) ((base) + (0xA04F))
#define PMIC_EIS_VBAT_D15_H_ADDR(base) ((base) + (0xA050))
#define PMIC_EIS_VBAT_D16_L_ADDR(base) ((base) + (0xA051))
#define PMIC_EIS_VBAT_D16_H_ADDR(base) ((base) + (0xA052))
#define PMIC_EIS_IBAT_P_D4_L_ADDR(base) ((base) + (0xA053))
#define PMIC_EIS_IBAT_P_D4_H_ADDR(base) ((base) + (0xA054))
#define PMIC_EIS_IBAT_P_D8_L_ADDR(base) ((base) + (0xA055))
#define PMIC_EIS_IBAT_P_D8_H_ADDR(base) ((base) + (0xA056))
#define PMIC_EIS_IBAT_P_D12_L_ADDR(base) ((base) + (0xA057))
#define PMIC_EIS_IBAT_P_D12_H_ADDR(base) ((base) + (0xA058))
#define PMIC_EIS_IBAT_P_D16_L_ADDR(base) ((base) + (0xA059))
#define PMIC_EIS_IBAT_P_D16_H_ADDR(base) ((base) + (0xA05A))
#define PMIC_EIS_DET_T_NUM_L_ADDR(base) ((base) + (0xA05B))
#define PMIC_EIS_DET_T_NUM_H_ADDR(base) ((base) + (0xA05C))
#define PMIC_EIS_VBAT_DET_NUM_ADDR(base) ((base) + (0xA05D))
#define PMIC_EIS_DET_FLAG_ADDR(base) ((base) + (0xA05E))
#define PMIC_EIS_INIT_END_FLAG_ADDR(base) ((base) + (0xA05F))
#define PMIC_EISADC_RESERVE_ADDR(base) ((base) + (0xA070))
#define PMIC_EISADC_CONV_STATUS_ADDR(base) ((base) + (0xA071))
#define PMIC_EISADC_DATA_L_ADDR(base) ((base) + (0xA072))
#define PMIC_EISADC_DATA_H_ADDR(base) ((base) + (0xA073))
#define PMIC_EISADC_CONV_ADDR(base) ((base) + (0xA074))
#define PMIC_EIS_ADC_CURRENT_ADDR(base) ((base) + (0xA075))
#define PMIC_EIS_SOFT_RST_ADDR(base) ((base) + (0xA076))
#define PMIC_EIS_CLK_GT_CTRL_ADDR(base) ((base) + (0xA077))
#define PMIC_EIS_DEBUG_EN_ADDR(base) ((base) + (0xA080))
#define PMIC_DEBUG_EIS_EN_ADDR(base) ((base) + (0xA081))
#define PMIC_DEBUG_EIS_DISCHG_EN_ADDR(base) ((base) + (0xA082))
#define PMIC_DEBUG_EIS_CHANL_ADDR(base) ((base) + (0xA083))
#define PMIC_DEBUG_EISADC_CTRL_ADDR(base) ((base) + (0xA084))
#define PMIC_DEBUG_EISADC_START_ADDR(base) ((base) + (0xA085))
#define PMIC_EISADC_MODE_CFG_ADDR(base) ((base) + (0xA086))
#endif
#ifndef __SOC_H_FOR_ASM__
#define PMIC_CLK_EN_CFG_ADDR(base) ((base) + (0xC000UL))
#define PMIC_CLK_EDGE_CFG_ADDR(base) ((base) + (0xC001UL))
#define PMIC_SIF_LOOPBACK_CFG_ADDR(base) ((base) + (0xC002UL))
#define PMIC_DAC_CHAN_CTRL_ADDR(base) ((base) + (0xC003UL))
#define PMIC_ADC_CHAN_CTRL_ADDR(base) ((base) + (0xC004UL))
#define PMIC_SIF_MAD_ADDR(base) ((base) + (0xC005UL))
#define PMIC_SIF_MAD_CIC_GAIN_ADDR(base) ((base) + (0xC006UL))
#define PMIC_ANA_IRQ_SIG_STAT_ADDR(base) ((base) + (0xC007UL))
#define PMIC_ANA_IRQM_REG0_ADDR(base) ((base) + (0xC008UL))
#define PMIC_ANA_IRQ_REG0_ADDR(base) ((base) + (0xC009UL))
#define PMIC_DEB_CNT_HS_DET_CFG_ADDR(base) ((base) + (0xC00AUL))
#define PMIC_DEB_CNT_HS_MIC_CFG_ADDR(base) ((base) + (0xC00BUL))
#define PMIC_CODEC_ANA_RW1_ADDR(base) ((base) + (0xC00CUL))
#define PMIC_CODEC_ANA_RW2_ADDR(base) ((base) + (0xC00DUL))
#define PMIC_CODEC_ANA_RW3_ADDR(base) ((base) + (0xC00EUL))
#define PMIC_CODEC_ANA_RW4_ADDR(base) ((base) + (0xC00FUL))
#define PMIC_CODEC_ANA_RW5_ADDR(base) ((base) + (0xC010UL))
#define PMIC_CODEC_ANA_RW6_ADDR(base) ((base) + (0xC011UL))
#define PMIC_CODEC_ANA_RW7_ADDR(base) ((base) + (0xC012UL))
#define PMIC_CODEC_ANA_RW8_ADDR(base) ((base) + (0xC013UL))
#define PMIC_CODEC_ANA_RW9_ADDR(base) ((base) + (0xC014UL))
#define PMIC_CODEC_ANA_RW10_ADDR(base) ((base) + (0xC015UL))
#define PMIC_CODEC_ANA_RW11_ADDR(base) ((base) + (0xC016UL))
#define PMIC_CODEC_ANA_RW12_ADDR(base) ((base) + (0xC017UL))
#define PMIC_CODEC_ANA_RW13_ADDR(base) ((base) + (0xC018UL))
#define PMIC_CODEC_ANA_RW14_ADDR(base) ((base) + (0xC019UL))
#define PMIC_CODEC_ANA_RW15_ADDR(base) ((base) + (0xC01AUL))
#define PMIC_CODEC_ANA_RW16_ADDR(base) ((base) + (0xC01BUL))
#define PMIC_CODEC_ANA_RW17_ADDR(base) ((base) + (0xC01CUL))
#define PMIC_CODEC_ANA_RW18_ADDR(base) ((base) + (0xC01DUL))
#define PMIC_CODEC_ANA_RW19_ADDR(base) ((base) + (0xC01EUL))
#define PMIC_CODEC_ANA_RW20_ADDR(base) ((base) + (0xC01FUL))
#define PMIC_CODEC_ANA_RW21_ADDR(base) ((base) + (0xC020UL))
#define PMIC_CODEC_ANA_RW22_ADDR(base) ((base) + (0xC021UL))
#define PMIC_CODEC_ANA_RW23_ADDR(base) ((base) + (0xC022UL))
#define PMIC_CODEC_ANA_RW24_ADDR(base) ((base) + (0xC023UL))
#define PMIC_CODEC_ANA_RW25_ADDR(base) ((base) + (0xC024UL))
#define PMIC_CODEC_ANA_RW26_ADDR(base) ((base) + (0xC025UL))
#define PMIC_CODEC_ANA_RW27_ADDR(base) ((base) + (0xC026UL))
#define PMIC_CODEC_ANA_RW28_ADDR(base) ((base) + (0xC027UL))
#define PMIC_CODEC_ANA_RW29_ADDR(base) ((base) + (0xC028UL))
#define PMIC_CODEC_ANA_RW30_ADDR(base) ((base) + (0xC029UL))
#define PMIC_CODEC_ANA_RW31_ADDR(base) ((base) + (0xC02AUL))
#define PMIC_CODEC_ANA_RW32_ADDR(base) ((base) + (0xC02BUL))
#define PMIC_CODEC_ANA_RW33_ADDR(base) ((base) + (0xC02CUL))
#define PMIC_CODEC_ANA_RW34_ADDR(base) ((base) + (0xC02DUL))
#define PMIC_CODEC_ANA_RW35_ADDR(base) ((base) + (0xC02EUL))
#define PMIC_CODEC_ANA_RW36_ADDR(base) ((base) + (0xC02FUL))
#define PMIC_CODEC_ANA_RW37_ADDR(base) ((base) + (0xC030UL))
#define PMIC_CODEC_ANA_RW38_ADDR(base) ((base) + (0xC031UL))
#define PMIC_CODEC_ANA_RW39_ADDR(base) ((base) + (0xC032UL))
#define PMIC_CODEC_ANA_RW40_ADDR(base) ((base) + (0xC033UL))
#define PMIC_CODEC_ANA_RW41_ADDR(base) ((base) + (0xC034UL))
#define PMIC_CODEC_ANA_RW42_ADDR(base) ((base) + (0xC035UL))
#define PMIC_CODEC_ANA_RW43_ADDR(base) ((base) + (0xC036UL))
#define PMIC_CODEC_ANA_RW44_ADDR(base) ((base) + (0xC037UL))
#define PMIC_CODEC_ANA_RW45_ADDR(base) ((base) + (0xC038UL))
#define PMIC_CODEC_ANA_RW46_ADDR(base) ((base) + (0xC039UL))
#define PMIC_CODEC_ANA_RW47_ADDR(base) ((base) + (0xC03AUL))
#define PMIC_CODEC_ANA_RW48_ADDR(base) ((base) + (0xC03BUL))
#define PMIC_CODEC_ANA_RW49_ADDR(base) ((base) + (0xC03CUL))
#define PMIC_CODEC_ANA_RW50_ADDR(base) ((base) + (0xC03DUL))
#define PMIC_CODEC_ANA_RW51_ADDR(base) ((base) + (0xC03EUL))
#define PMIC_CODEC_ANA_RW52_ADDR(base) ((base) + (0xC03FUL))
#define PMIC_CODEC_ANA_RW53_ADDR(base) ((base) + (0xC040UL))
#define PMIC_CODEC_ANA_RW54_ADDR(base) ((base) + (0xC041UL))
#define PMIC_CODEC_ANA_RW55_ADDR(base) ((base) + (0xC042UL))
#define PMIC_CODEC_ANA_RW56_ADDR(base) ((base) + (0xC043UL))
#define PMIC_CODEC_ANA_RW57_ADDR(base) ((base) + (0xC044UL))
#define PMIC_CODEC_ANA_RW58_ADDR(base) ((base) + (0xC045UL))
#define PMIC_CODEC_ANA_RW59_ADDR(base) ((base) + (0xC046UL))
#define PMIC_CODEC_ANA_RW60_ADDR(base) ((base) + (0xC047UL))
#define PMIC_CODEC_ANA_RW61_ADDR(base) ((base) + (0xC048UL))
#define PMIC_CODEC_ANA_RW62_ADDR(base) ((base) + (0xC049UL))
#define PMIC_CODEC_ANA_RW63_ADDR(base) ((base) + (0xC04AUL))
#define PMIC_CODEC_ANA_RO01_ADDR(base) ((base) + (0xC04BUL))
#define PMIC_CODEC_ANA_RO02_ADDR(base) ((base) + (0xC04CUL))
#else
#define PMIC_CLK_EN_CFG_ADDR(base) ((base) + (0xC000))
#define PMIC_CLK_EDGE_CFG_ADDR(base) ((base) + (0xC001))
#define PMIC_SIF_LOOPBACK_CFG_ADDR(base) ((base) + (0xC002))
#define PMIC_DAC_CHAN_CTRL_ADDR(base) ((base) + (0xC003))
#define PMIC_ADC_CHAN_CTRL_ADDR(base) ((base) + (0xC004))
#define PMIC_SIF_MAD_ADDR(base) ((base) + (0xC005))
#define PMIC_SIF_MAD_CIC_GAIN_ADDR(base) ((base) + (0xC006))
#define PMIC_ANA_IRQ_SIG_STAT_ADDR(base) ((base) + (0xC007))
#define PMIC_ANA_IRQM_REG0_ADDR(base) ((base) + (0xC008))
#define PMIC_ANA_IRQ_REG0_ADDR(base) ((base) + (0xC009))
#define PMIC_DEB_CNT_HS_DET_CFG_ADDR(base) ((base) + (0xC00A))
#define PMIC_DEB_CNT_HS_MIC_CFG_ADDR(base) ((base) + (0xC00B))
#define PMIC_CODEC_ANA_RW1_ADDR(base) ((base) + (0xC00C))
#define PMIC_CODEC_ANA_RW2_ADDR(base) ((base) + (0xC00D))
#define PMIC_CODEC_ANA_RW3_ADDR(base) ((base) + (0xC00E))
#define PMIC_CODEC_ANA_RW4_ADDR(base) ((base) + (0xC00F))
#define PMIC_CODEC_ANA_RW5_ADDR(base) ((base) + (0xC010))
#define PMIC_CODEC_ANA_RW6_ADDR(base) ((base) + (0xC011))
#define PMIC_CODEC_ANA_RW7_ADDR(base) ((base) + (0xC012))
#define PMIC_CODEC_ANA_RW8_ADDR(base) ((base) + (0xC013))
#define PMIC_CODEC_ANA_RW9_ADDR(base) ((base) + (0xC014))
#define PMIC_CODEC_ANA_RW10_ADDR(base) ((base) + (0xC015))
#define PMIC_CODEC_ANA_RW11_ADDR(base) ((base) + (0xC016))
#define PMIC_CODEC_ANA_RW12_ADDR(base) ((base) + (0xC017))
#define PMIC_CODEC_ANA_RW13_ADDR(base) ((base) + (0xC018))
#define PMIC_CODEC_ANA_RW14_ADDR(base) ((base) + (0xC019))
#define PMIC_CODEC_ANA_RW15_ADDR(base) ((base) + (0xC01A))
#define PMIC_CODEC_ANA_RW16_ADDR(base) ((base) + (0xC01B))
#define PMIC_CODEC_ANA_RW17_ADDR(base) ((base) + (0xC01C))
#define PMIC_CODEC_ANA_RW18_ADDR(base) ((base) + (0xC01D))
#define PMIC_CODEC_ANA_RW19_ADDR(base) ((base) + (0xC01E))
#define PMIC_CODEC_ANA_RW20_ADDR(base) ((base) + (0xC01F))
#define PMIC_CODEC_ANA_RW21_ADDR(base) ((base) + (0xC020))
#define PMIC_CODEC_ANA_RW22_ADDR(base) ((base) + (0xC021))
#define PMIC_CODEC_ANA_RW23_ADDR(base) ((base) + (0xC022))
#define PMIC_CODEC_ANA_RW24_ADDR(base) ((base) + (0xC023))
#define PMIC_CODEC_ANA_RW25_ADDR(base) ((base) + (0xC024))
#define PMIC_CODEC_ANA_RW26_ADDR(base) ((base) + (0xC025))
#define PMIC_CODEC_ANA_RW27_ADDR(base) ((base) + (0xC026))
#define PMIC_CODEC_ANA_RW28_ADDR(base) ((base) + (0xC027))
#define PMIC_CODEC_ANA_RW29_ADDR(base) ((base) + (0xC028))
#define PMIC_CODEC_ANA_RW30_ADDR(base) ((base) + (0xC029))
#define PMIC_CODEC_ANA_RW31_ADDR(base) ((base) + (0xC02A))
#define PMIC_CODEC_ANA_RW32_ADDR(base) ((base) + (0xC02B))
#define PMIC_CODEC_ANA_RW33_ADDR(base) ((base) + (0xC02C))
#define PMIC_CODEC_ANA_RW34_ADDR(base) ((base) + (0xC02D))
#define PMIC_CODEC_ANA_RW35_ADDR(base) ((base) + (0xC02E))
#define PMIC_CODEC_ANA_RW36_ADDR(base) ((base) + (0xC02F))
#define PMIC_CODEC_ANA_RW37_ADDR(base) ((base) + (0xC030))
#define PMIC_CODEC_ANA_RW38_ADDR(base) ((base) + (0xC031))
#define PMIC_CODEC_ANA_RW39_ADDR(base) ((base) + (0xC032))
#define PMIC_CODEC_ANA_RW40_ADDR(base) ((base) + (0xC033))
#define PMIC_CODEC_ANA_RW41_ADDR(base) ((base) + (0xC034))
#define PMIC_CODEC_ANA_RW42_ADDR(base) ((base) + (0xC035))
#define PMIC_CODEC_ANA_RW43_ADDR(base) ((base) + (0xC036))
#define PMIC_CODEC_ANA_RW44_ADDR(base) ((base) + (0xC037))
#define PMIC_CODEC_ANA_RW45_ADDR(base) ((base) + (0xC038))
#define PMIC_CODEC_ANA_RW46_ADDR(base) ((base) + (0xC039))
#define PMIC_CODEC_ANA_RW47_ADDR(base) ((base) + (0xC03A))
#define PMIC_CODEC_ANA_RW48_ADDR(base) ((base) + (0xC03B))
#define PMIC_CODEC_ANA_RW49_ADDR(base) ((base) + (0xC03C))
#define PMIC_CODEC_ANA_RW50_ADDR(base) ((base) + (0xC03D))
#define PMIC_CODEC_ANA_RW51_ADDR(base) ((base) + (0xC03E))
#define PMIC_CODEC_ANA_RW52_ADDR(base) ((base) + (0xC03F))
#define PMIC_CODEC_ANA_RW53_ADDR(base) ((base) + (0xC040))
#define PMIC_CODEC_ANA_RW54_ADDR(base) ((base) + (0xC041))
#define PMIC_CODEC_ANA_RW55_ADDR(base) ((base) + (0xC042))
#define PMIC_CODEC_ANA_RW56_ADDR(base) ((base) + (0xC043))
#define PMIC_CODEC_ANA_RW57_ADDR(base) ((base) + (0xC044))
#define PMIC_CODEC_ANA_RW58_ADDR(base) ((base) + (0xC045))
#define PMIC_CODEC_ANA_RW59_ADDR(base) ((base) + (0xC046))
#define PMIC_CODEC_ANA_RW60_ADDR(base) ((base) + (0xC047))
#define PMIC_CODEC_ANA_RW61_ADDR(base) ((base) + (0xC048))
#define PMIC_CODEC_ANA_RW62_ADDR(base) ((base) + (0xC049))
#define PMIC_CODEC_ANA_RW63_ADDR(base) ((base) + (0xC04A))
#define PMIC_CODEC_ANA_RO01_ADDR(base) ((base) + (0xC04B))
#define PMIC_CODEC_ANA_RO02_ADDR(base) ((base) + (0xC04C))
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
        unsigned char sc_dummy_spmi : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_DUMMY_SPMI_UNION;
#endif
#define PMIC_DUMMY_SPMI_sc_dummy_spmi_START (0)
#define PMIC_DUMMY_SPMI_sc_dummy_spmi_END (3)
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
        unsigned char st_nfc_pwr_en_d5m : 1;
        unsigned char st_ufs_sel_d20nr : 1;
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
#define PMIC_STATUS1_st_nfc_pwr_en_d5m_START (4)
#define PMIC_STATUS1_st_nfc_pwr_en_d5m_END (4)
#define PMIC_STATUS1_st_ufs_sel_d20nr_START (5)
#define PMIC_STATUS1_st_ufs_sel_d20nr_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char st_vsys_ovp_surge_d20nr : 1;
        unsigned char st_voice_restart1_d120u : 1;
        unsigned char st_voice_restart2_d120u : 1;
        unsigned char st_comb_button_d120u : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_STATUS2_UNION;
#endif
#define PMIC_STATUS2_st_vsys_ovp_surge_d20nr_START (0)
#define PMIC_STATUS2_st_vsys_ovp_surge_d20nr_END (0)
#define PMIC_STATUS2_st_voice_restart1_d120u_START (1)
#define PMIC_STATUS2_st_voice_restart1_d120u_END (1)
#define PMIC_STATUS2_st_voice_restart2_d120u_START (2)
#define PMIC_STATUS2_st_voice_restart2_d120u_END (2)
#define PMIC_STATUS2_st_comb_button_d120u_START (3)
#define PMIC_STATUS2_st_comb_button_d120u_END (3)
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
        unsigned char reg_buck40_en : 1;
        unsigned char st_buck40_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_buck40_eco_en : 1;
        unsigned char st_buck40_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_BUCK40_ONOFF_ECO_UNION;
#endif
#define PMIC_BUCK40_ONOFF_ECO_reg_buck40_en_START (0)
#define PMIC_BUCK40_ONOFF_ECO_reg_buck40_en_END (0)
#define PMIC_BUCK40_ONOFF_ECO_st_buck40_en_START (1)
#define PMIC_BUCK40_ONOFF_ECO_st_buck40_en_END (1)
#define PMIC_BUCK40_ONOFF_ECO_reg_buck40_eco_en_START (4)
#define PMIC_BUCK40_ONOFF_ECO_reg_buck40_eco_en_END (4)
#define PMIC_BUCK40_ONOFF_ECO_st_buck40_eco_en_START (5)
#define PMIC_BUCK40_ONOFF_ECO_st_buck40_eco_en_END (5)
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
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo1_eco_en : 1;
        unsigned char st_ldo1_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO1_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO1_ONOFF_ECO_reg_ldo1_en_START (0)
#define PMIC_LDO1_ONOFF_ECO_reg_ldo1_en_END (0)
#define PMIC_LDO1_ONOFF_ECO_st_ldo1_en_START (1)
#define PMIC_LDO1_ONOFF_ECO_st_ldo1_en_END (1)
#define PMIC_LDO1_ONOFF_ECO_reg_ldo1_eco_en_START (4)
#define PMIC_LDO1_ONOFF_ECO_reg_ldo1_eco_en_END (4)
#define PMIC_LDO1_ONOFF_ECO_st_ldo1_eco_en_START (5)
#define PMIC_LDO1_ONOFF_ECO_st_ldo1_eco_en_END (5)
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
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo3_eco_en : 1;
        unsigned char st_ldo3_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO3_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO3_ONOFF_ECO_reg_ldo3_en_START (0)
#define PMIC_LDO3_ONOFF_ECO_reg_ldo3_en_END (0)
#define PMIC_LDO3_ONOFF_ECO_st_ldo3_en_START (1)
#define PMIC_LDO3_ONOFF_ECO_st_ldo3_en_END (1)
#define PMIC_LDO3_ONOFF_ECO_reg_ldo3_eco_en_START (4)
#define PMIC_LDO3_ONOFF_ECO_reg_ldo3_eco_en_END (4)
#define PMIC_LDO3_ONOFF_ECO_st_ldo3_eco_en_START (5)
#define PMIC_LDO3_ONOFF_ECO_st_ldo3_eco_en_END (5)
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
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo13_eco_en : 1;
        unsigned char st_ldo13_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO13_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO13_ONOFF_ECO_reg_ldo13_en_START (0)
#define PMIC_LDO13_ONOFF_ECO_reg_ldo13_en_END (0)
#define PMIC_LDO13_ONOFF_ECO_st_ldo13_en_START (1)
#define PMIC_LDO13_ONOFF_ECO_st_ldo13_en_END (1)
#define PMIC_LDO13_ONOFF_ECO_reg_ldo13_eco_en_START (4)
#define PMIC_LDO13_ONOFF_ECO_reg_ldo13_eco_en_END (4)
#define PMIC_LDO13_ONOFF_ECO_st_ldo13_eco_en_START (5)
#define PMIC_LDO13_ONOFF_ECO_st_ldo13_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo14_en : 1;
        unsigned char st_ldo14_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo14_eco_en : 1;
        unsigned char st_ldo14_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO14_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO14_ONOFF_ECO_reg_ldo14_en_START (0)
#define PMIC_LDO14_ONOFF_ECO_reg_ldo14_en_END (0)
#define PMIC_LDO14_ONOFF_ECO_st_ldo14_en_START (1)
#define PMIC_LDO14_ONOFF_ECO_st_ldo14_en_END (1)
#define PMIC_LDO14_ONOFF_ECO_reg_ldo14_eco_en_START (4)
#define PMIC_LDO14_ONOFF_ECO_reg_ldo14_eco_en_END (4)
#define PMIC_LDO14_ONOFF_ECO_st_ldo14_eco_en_START (5)
#define PMIC_LDO14_ONOFF_ECO_st_ldo14_eco_en_END (5)
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
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo19_eco_en : 1;
        unsigned char st_ldo19_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO19_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO19_ONOFF_ECO_reg_ldo19_en_START (0)
#define PMIC_LDO19_ONOFF_ECO_reg_ldo19_en_END (0)
#define PMIC_LDO19_ONOFF_ECO_st_ldo19_en_START (1)
#define PMIC_LDO19_ONOFF_ECO_st_ldo19_en_END (1)
#define PMIC_LDO19_ONOFF_ECO_reg_ldo19_eco_en_START (4)
#define PMIC_LDO19_ONOFF_ECO_reg_ldo19_eco_en_END (4)
#define PMIC_LDO19_ONOFF_ECO_st_ldo19_eco_en_START (5)
#define PMIC_LDO19_ONOFF_ECO_st_ldo19_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo20_en : 1;
        unsigned char st_ldo20_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo20_eco_en : 1;
        unsigned char st_ldo20_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO20_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO20_ONOFF_ECO_reg_ldo20_en_START (0)
#define PMIC_LDO20_ONOFF_ECO_reg_ldo20_en_END (0)
#define PMIC_LDO20_ONOFF_ECO_st_ldo20_en_START (1)
#define PMIC_LDO20_ONOFF_ECO_st_ldo20_en_END (1)
#define PMIC_LDO20_ONOFF_ECO_reg_ldo20_eco_en_START (4)
#define PMIC_LDO20_ONOFF_ECO_reg_ldo20_eco_en_END (4)
#define PMIC_LDO20_ONOFF_ECO_st_ldo20_eco_en_START (5)
#define PMIC_LDO20_ONOFF_ECO_st_ldo20_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo21_en : 1;
        unsigned char st_ldo21_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo21_eco_en : 1;
        unsigned char st_ldo21_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO21_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO21_ONOFF_ECO_reg_ldo21_en_START (0)
#define PMIC_LDO21_ONOFF_ECO_reg_ldo21_en_END (0)
#define PMIC_LDO21_ONOFF_ECO_st_ldo21_en_START (1)
#define PMIC_LDO21_ONOFF_ECO_st_ldo21_en_END (1)
#define PMIC_LDO21_ONOFF_ECO_reg_ldo21_eco_en_START (4)
#define PMIC_LDO21_ONOFF_ECO_reg_ldo21_eco_en_END (4)
#define PMIC_LDO21_ONOFF_ECO_st_ldo21_eco_en_START (5)
#define PMIC_LDO21_ONOFF_ECO_st_ldo21_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo22_en : 1;
        unsigned char st_ldo22_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo22_eco_en : 1;
        unsigned char st_ldo22_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO22_ONOFF_UNION;
#endif
#define PMIC_LDO22_ONOFF_reg_ldo22_en_START (0)
#define PMIC_LDO22_ONOFF_reg_ldo22_en_END (0)
#define PMIC_LDO22_ONOFF_st_ldo22_en_START (1)
#define PMIC_LDO22_ONOFF_st_ldo22_en_END (1)
#define PMIC_LDO22_ONOFF_reg_ldo22_eco_en_START (4)
#define PMIC_LDO22_ONOFF_reg_ldo22_eco_en_END (4)
#define PMIC_LDO22_ONOFF_st_ldo22_eco_en_START (5)
#define PMIC_LDO22_ONOFF_st_ldo22_eco_en_END (5)
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
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo28_eco_en : 1;
        unsigned char st_ldo28_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO28_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO28_ONOFF_ECO_reg_ldo28_en_START (0)
#define PMIC_LDO28_ONOFF_ECO_reg_ldo28_en_END (0)
#define PMIC_LDO28_ONOFF_ECO_st_ldo28_en_START (1)
#define PMIC_LDO28_ONOFF_ECO_st_ldo28_en_END (1)
#define PMIC_LDO28_ONOFF_ECO_reg_ldo28_eco_en_START (4)
#define PMIC_LDO28_ONOFF_ECO_reg_ldo28_eco_en_END (4)
#define PMIC_LDO28_ONOFF_ECO_st_ldo28_eco_en_START (5)
#define PMIC_LDO28_ONOFF_ECO_st_ldo28_eco_en_END (5)
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
        unsigned char reg_ldo31_en : 1;
        unsigned char st_ldo31_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo31_eco_en : 1;
        unsigned char st_ldo31_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO31_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO31_ONOFF_ECO_reg_ldo31_en_START (0)
#define PMIC_LDO31_ONOFF_ECO_reg_ldo31_en_END (0)
#define PMIC_LDO31_ONOFF_ECO_st_ldo31_en_START (1)
#define PMIC_LDO31_ONOFF_ECO_st_ldo31_en_END (1)
#define PMIC_LDO31_ONOFF_ECO_reg_ldo31_eco_en_START (4)
#define PMIC_LDO31_ONOFF_ECO_reg_ldo31_eco_en_END (4)
#define PMIC_LDO31_ONOFF_ECO_st_ldo31_eco_en_START (5)
#define PMIC_LDO31_ONOFF_ECO_st_ldo31_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo32_en : 1;
        unsigned char st_ldo32_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo32_eco_en : 1;
        unsigned char st_ldo32_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO32_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO32_ONOFF_ECO_reg_ldo32_en_START (0)
#define PMIC_LDO32_ONOFF_ECO_reg_ldo32_en_END (0)
#define PMIC_LDO32_ONOFF_ECO_st_ldo32_en_START (1)
#define PMIC_LDO32_ONOFF_ECO_st_ldo32_en_END (1)
#define PMIC_LDO32_ONOFF_ECO_reg_ldo32_eco_en_START (4)
#define PMIC_LDO32_ONOFF_ECO_reg_ldo32_eco_en_END (4)
#define PMIC_LDO32_ONOFF_ECO_st_ldo32_eco_en_START (5)
#define PMIC_LDO32_ONOFF_ECO_st_ldo32_eco_en_END (5)
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
        unsigned char reg_ldo42_en : 1;
        unsigned char st_ldo42_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo42_eco_en : 1;
        unsigned char st_ldo42_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO42_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO42_ONOFF_ECO_reg_ldo42_en_START (0)
#define PMIC_LDO42_ONOFF_ECO_reg_ldo42_en_END (0)
#define PMIC_LDO42_ONOFF_ECO_st_ldo42_en_START (1)
#define PMIC_LDO42_ONOFF_ECO_st_ldo42_en_END (1)
#define PMIC_LDO42_ONOFF_ECO_reg_ldo42_eco_en_START (4)
#define PMIC_LDO42_ONOFF_ECO_reg_ldo42_eco_en_END (4)
#define PMIC_LDO42_ONOFF_ECO_st_ldo42_eco_en_START (5)
#define PMIC_LDO42_ONOFF_ECO_st_ldo42_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo43_en : 1;
        unsigned char st_ldo43_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo43_eco_en : 1;
        unsigned char st_ldo43_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO43_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO43_ONOFF_ECO_reg_ldo43_en_START (0)
#define PMIC_LDO43_ONOFF_ECO_reg_ldo43_en_END (0)
#define PMIC_LDO43_ONOFF_ECO_st_ldo43_en_START (1)
#define PMIC_LDO43_ONOFF_ECO_st_ldo43_en_END (1)
#define PMIC_LDO43_ONOFF_ECO_reg_ldo43_eco_en_START (4)
#define PMIC_LDO43_ONOFF_ECO_reg_ldo43_eco_en_END (4)
#define PMIC_LDO43_ONOFF_ECO_st_ldo43_eco_en_START (5)
#define PMIC_LDO43_ONOFF_ECO_st_ldo43_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo44_en : 1;
        unsigned char st_ldo44_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo44_eco_en : 1;
        unsigned char st_ldo44_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO44_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO44_ONOFF_ECO_reg_ldo44_en_START (0)
#define PMIC_LDO44_ONOFF_ECO_reg_ldo44_en_END (0)
#define PMIC_LDO44_ONOFF_ECO_st_ldo44_en_START (1)
#define PMIC_LDO44_ONOFF_ECO_st_ldo44_en_END (1)
#define PMIC_LDO44_ONOFF_ECO_reg_ldo44_eco_en_START (4)
#define PMIC_LDO44_ONOFF_ECO_reg_ldo44_eco_en_END (4)
#define PMIC_LDO44_ONOFF_ECO_st_ldo44_eco_en_START (5)
#define PMIC_LDO44_ONOFF_ECO_st_ldo44_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo45_en : 1;
        unsigned char st_ldo45_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo45_eco_en : 1;
        unsigned char st_ldo45_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO45_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO45_ONOFF_ECO_reg_ldo45_en_START (0)
#define PMIC_LDO45_ONOFF_ECO_reg_ldo45_en_END (0)
#define PMIC_LDO45_ONOFF_ECO_st_ldo45_en_START (1)
#define PMIC_LDO45_ONOFF_ECO_st_ldo45_en_END (1)
#define PMIC_LDO45_ONOFF_ECO_reg_ldo45_eco_en_START (4)
#define PMIC_LDO45_ONOFF_ECO_reg_ldo45_eco_en_END (4)
#define PMIC_LDO45_ONOFF_ECO_st_ldo45_eco_en_START (5)
#define PMIC_LDO45_ONOFF_ECO_st_ldo45_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo46_en : 1;
        unsigned char st_ldo46_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reserved_1 : 4;
    } reg;
} PMIC_LDO46_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO46_ONOFF_ECO_reg_ldo46_en_START (0)
#define PMIC_LDO46_ONOFF_ECO_reg_ldo46_en_END (0)
#define PMIC_LDO46_ONOFF_ECO_st_ldo46_en_START (1)
#define PMIC_LDO46_ONOFF_ECO_st_ldo46_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo47_en : 1;
        unsigned char st_ldo47_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo47_eco_en : 1;
        unsigned char st_ldo47_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO47_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO47_ONOFF_ECO_reg_ldo47_en_START (0)
#define PMIC_LDO47_ONOFF_ECO_reg_ldo47_en_END (0)
#define PMIC_LDO47_ONOFF_ECO_st_ldo47_en_START (1)
#define PMIC_LDO47_ONOFF_ECO_st_ldo47_en_END (1)
#define PMIC_LDO47_ONOFF_ECO_reg_ldo47_eco_en_START (4)
#define PMIC_LDO47_ONOFF_ECO_reg_ldo47_eco_en_END (4)
#define PMIC_LDO47_ONOFF_ECO_st_ldo47_eco_en_START (5)
#define PMIC_LDO47_ONOFF_ECO_st_ldo47_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo48_en : 1;
        unsigned char st_ldo48_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo48_eco_en : 1;
        unsigned char st_ldo48_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO48_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO48_ONOFF_ECO_reg_ldo48_en_START (0)
#define PMIC_LDO48_ONOFF_ECO_reg_ldo48_en_END (0)
#define PMIC_LDO48_ONOFF_ECO_st_ldo48_en_START (1)
#define PMIC_LDO48_ONOFF_ECO_st_ldo48_en_END (1)
#define PMIC_LDO48_ONOFF_ECO_reg_ldo48_eco_en_START (4)
#define PMIC_LDO48_ONOFF_ECO_reg_ldo48_eco_en_END (4)
#define PMIC_LDO48_ONOFF_ECO_st_ldo48_eco_en_START (5)
#define PMIC_LDO48_ONOFF_ECO_st_ldo48_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo49_en : 1;
        unsigned char st_ldo49_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo49_eco_en : 1;
        unsigned char st_ldo49_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO49_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO49_ONOFF_ECO_reg_ldo49_en_START (0)
#define PMIC_LDO49_ONOFF_ECO_reg_ldo49_en_END (0)
#define PMIC_LDO49_ONOFF_ECO_st_ldo49_en_START (1)
#define PMIC_LDO49_ONOFF_ECO_st_ldo49_en_END (1)
#define PMIC_LDO49_ONOFF_ECO_reg_ldo49_eco_en_START (4)
#define PMIC_LDO49_ONOFF_ECO_reg_ldo49_eco_en_END (4)
#define PMIC_LDO49_ONOFF_ECO_st_ldo49_eco_en_START (5)
#define PMIC_LDO49_ONOFF_ECO_st_ldo49_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo50_en : 1;
        unsigned char st_ldo50_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo50_eco_en : 1;
        unsigned char st_ldo50_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO50_ONOFF_ECO_UNION;
#endif
#define PMIC_LDO50_ONOFF_ECO_reg_ldo50_en_START (0)
#define PMIC_LDO50_ONOFF_ECO_reg_ldo50_en_END (0)
#define PMIC_LDO50_ONOFF_ECO_st_ldo50_en_START (1)
#define PMIC_LDO50_ONOFF_ECO_st_ldo50_en_END (1)
#define PMIC_LDO50_ONOFF_ECO_reg_ldo50_eco_en_START (4)
#define PMIC_LDO50_ONOFF_ECO_reg_ldo50_eco_en_END (4)
#define PMIC_LDO50_ONOFF_ECO_st_ldo50_eco_en_START (5)
#define PMIC_LDO50_ONOFF_ECO_st_ldo50_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_thsd_en : 1;
        unsigned char st_thsd_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_thsd_eco_en : 1;
        unsigned char st_thsd_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_BG_THSD_EN_UNION;
#endif
#define PMIC_BG_THSD_EN_reg_thsd_en_START (0)
#define PMIC_BG_THSD_EN_reg_thsd_en_END (0)
#define PMIC_BG_THSD_EN_st_thsd_en_START (1)
#define PMIC_BG_THSD_EN_st_thsd_en_END (1)
#define PMIC_BG_THSD_EN_reg_thsd_eco_en_START (4)
#define PMIC_BG_THSD_EN_reg_thsd_eco_en_END (4)
#define PMIC_BG_THSD_EN_st_thsd_eco_en_START (5)
#define PMIC_BG_THSD_EN_st_thsd_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char st_ref_en : 1;
        unsigned char reserved_0 : 3;
        unsigned char reg_ref_eco_en : 1;
        unsigned char st_ref_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_BG_REF_EN_UNION;
#endif
#define PMIC_BG_REF_EN_st_ref_en_START (0)
#define PMIC_BG_REF_EN_st_ref_en_END (0)
#define PMIC_BG_REF_EN_reg_ref_eco_en_START (4)
#define PMIC_BG_REF_EN_reg_ref_eco_en_END (4)
#define PMIC_BG_REF_EN_st_ref_eco_en_START (5)
#define PMIC_BG_REF_EN_st_ref_eco_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ref_res_sel : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_BG_REF_CTRL_UNION;
#endif
#define PMIC_BG_REF_CTRL_ref_res_sel_START (0)
#define PMIC_BG_REF_CTRL_ref_res_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_xo_serdes_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_CLK_SERDES_EN_UNION;
#endif
#define PMIC_CLK_SERDES_EN_reg_xo_serdes_en_START (0)
#define PMIC_CLK_SERDES_EN_reg_xo_serdes_en_END (0)
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
} PMIC_CLK_RF0_EN_UNION;
#endif
#define PMIC_CLK_RF0_EN_xo_rf_en_START (0)
#define PMIC_CLK_RF0_EN_xo_rf_en_END (0)
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
        unsigned char reg_bb_en : 1;
        unsigned char st_bb_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_BB_ONOFF_UNION;
#endif
#define PMIC_BB_ONOFF_reg_bb_en_START (0)
#define PMIC_BB_ONOFF_reg_bb_en_END (0)
#define PMIC_BB_ONOFF_st_bb_en_START (1)
#define PMIC_BB_ONOFF_st_bb_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_classd_en : 1;
        unsigned char st_classd_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_CLASSD_ONOFF_UNION;
#endif
#define PMIC_CLASSD_ONOFF_reg_classd_en_START (0)
#define PMIC_CLASSD_ONOFF_reg_classd_en_END (0)
#define PMIC_CLASSD_ONOFF_st_classd_en_START (1)
#define PMIC_CLASSD_ONOFF_st_classd_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_vset_cfg : 8;
    } reg;
} PMIC_BUCK0_VSET_UNION;
#endif
#define PMIC_BUCK0_VSET_buck0_vset_cfg_START (0)
#define PMIC_BUCK0_VSET_buck0_vset_cfg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_vset_eco : 8;
    } reg;
} PMIC_BUCK0_VSET_ECO_UNION;
#endif
#define PMIC_BUCK0_VSET_ECO_buck0_vset_eco_START (0)
#define PMIC_BUCK0_VSET_ECO_buck0_vset_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck1_vset_cfg : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK1_VSET_UNION;
#endif
#define PMIC_BUCK1_VSET_buck1_vset_cfg_START (0)
#define PMIC_BUCK1_VSET_buck1_vset_cfg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_vset_cfg : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK2_VSET_UNION;
#endif
#define PMIC_BUCK2_VSET_buck2_vset_cfg_START (0)
#define PMIC_BUCK2_VSET_buck2_vset_cfg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck2_vset_eco : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK2_VSET_ECO_UNION;
#endif
#define PMIC_BUCK2_VSET_ECO_buck2_vset_eco_START (0)
#define PMIC_BUCK2_VSET_ECO_buck2_vset_eco_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_vset_cfg : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK3_VSET_UNION;
#endif
#define PMIC_BUCK3_VSET_buck3_vset_cfg_START (0)
#define PMIC_BUCK3_VSET_buck3_vset_cfg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck3_vset_eco : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK3_VSET_ECO_UNION;
#endif
#define PMIC_BUCK3_VSET_ECO_buck3_vset_eco_START (0)
#define PMIC_BUCK3_VSET_ECO_buck3_vset_eco_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_vset_cfg : 8;
    } reg;
} PMIC_BUCK4_VSET_UNION;
#endif
#define PMIC_BUCK4_VSET_buck4_vset_cfg_START (0)
#define PMIC_BUCK4_VSET_buck4_vset_cfg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck4_vset_eco : 8;
    } reg;
} PMIC_BUCK4_VSET_ECO_UNION;
#endif
#define PMIC_BUCK4_VSET_ECO_buck4_vset_eco_START (0)
#define PMIC_BUCK4_VSET_ECO_buck4_vset_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_vset_cfg : 8;
    } reg;
} PMIC_BUCK5_VSET_UNION;
#endif
#define PMIC_BUCK5_VSET_buck5_vset_cfg_START (0)
#define PMIC_BUCK5_VSET_buck5_vset_cfg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_vset_eco : 8;
    } reg;
} PMIC_BUCK5_VSET_ECO_UNION;
#endif
#define PMIC_BUCK5_VSET_ECO_buck5_vset_eco_START (0)
#define PMIC_BUCK5_VSET_ECO_buck5_vset_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck6_vset_cfg : 8;
    } reg;
} PMIC_BUCK6_VSET_UNION;
#endif
#define PMIC_BUCK6_VSET_buck6_vset_cfg_START (0)
#define PMIC_BUCK6_VSET_buck6_vset_cfg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck6_vset_eco : 8;
    } reg;
} PMIC_BUCK6_VSET_ECO_UNION;
#endif
#define PMIC_BUCK6_VSET_ECO_buck6_vset_eco_START (0)
#define PMIC_BUCK6_VSET_ECO_buck6_vset_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_vset_cfg : 8;
    } reg;
} PMIC_BUCK7_VSET_UNION;
#endif
#define PMIC_BUCK7_VSET_buck7_vset_cfg_START (0)
#define PMIC_BUCK7_VSET_buck7_vset_cfg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo0_vset_cfg : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO0_VSET_UNION;
#endif
#define PMIC_LDO0_VSET_ldo0_vset_cfg_START (0)
#define PMIC_LDO0_VSET_ldo0_vset_cfg_END (5)
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
        unsigned char ldo1_vset_cfg : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO1_VSET_UNION;
#endif
#define PMIC_LDO1_VSET_ldo1_vset_cfg_START (0)
#define PMIC_LDO1_VSET_ldo1_vset_cfg_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo2_vset_cfg : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO2_VSET_UNION;
#endif
#define PMIC_LDO2_VSET_ldo2_vset_cfg_START (0)
#define PMIC_LDO2_VSET_ldo2_vset_cfg_END (4)
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
        unsigned char ldo6_vset_cfg : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO6_VSET_UNION;
#endif
#define PMIC_LDO6_VSET_ldo6_vset_cfg_START (0)
#define PMIC_LDO6_VSET_ldo6_vset_cfg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo6_vset_eco : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO6_VSET_ECO_UNION;
#endif
#define PMIC_LDO6_VSET_ECO_ldo6_vset_eco_START (0)
#define PMIC_LDO6_VSET_ECO_ldo6_vset_eco_END (5)
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
        unsigned char ldo9_vset : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO9_VSET_UNION;
#endif
#define PMIC_LDO9_VSET_ldo9_vset_START (0)
#define PMIC_LDO9_VSET_ldo9_vset_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo11_vset : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO11_VSET_UNION;
#endif
#define PMIC_LDO11_VSET_ldo11_vset_START (0)
#define PMIC_LDO11_VSET_ldo11_vset_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo12_vset : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO12_VSET_UNION;
#endif
#define PMIC_LDO12_VSET_ldo12_vset_START (0)
#define PMIC_LDO12_VSET_ldo12_vset_END (5)
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
        unsigned char ldo14_vset : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO14_VSET_UNION;
#endif
#define PMIC_LDO14_VSET_ldo14_vset_START (0)
#define PMIC_LDO14_VSET_ldo14_vset_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo15_vset : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO15_VSET_UNION;
#endif
#define PMIC_LDO15_VSET_ldo15_vset_START (0)
#define PMIC_LDO15_VSET_ldo15_vset_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo16_vset : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO16_VSET_UNION;
#endif
#define PMIC_LDO16_VSET_ldo16_vset_START (0)
#define PMIC_LDO16_VSET_ldo16_vset_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo17_vset : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO17_VSET_UNION;
#endif
#define PMIC_LDO17_VSET_ldo17_vset_START (0)
#define PMIC_LDO17_VSET_ldo17_vset_END (5)
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
        unsigned char ldo20_vset : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO20_VSET_UNION;
#endif
#define PMIC_LDO20_VSET_ldo20_vset_START (0)
#define PMIC_LDO20_VSET_ldo20_vset_END (4)
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
        unsigned char ldo23_vset : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO23_VSET_UNION;
#endif
#define PMIC_LDO23_VSET_ldo23_vset_START (0)
#define PMIC_LDO23_VSET_ldo23_vset_END (5)
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
        unsigned char ldo26_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO26_VSET_UNION;
#endif
#define PMIC_LDO26_VSET_ldo26_vset_START (0)
#define PMIC_LDO26_VSET_ldo26_vset_END (3)
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
        unsigned char ldo30_vset : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO30_VSET_UNION;
#endif
#define PMIC_LDO30_VSET_ldo30_vset_START (0)
#define PMIC_LDO30_VSET_ldo30_vset_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo31_vset : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO31_VSET_UNION;
#endif
#define PMIC_LDO31_VSET_ldo31_vset_START (0)
#define PMIC_LDO31_VSET_ldo31_vset_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo32_vset : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO32_VSET_UNION;
#endif
#define PMIC_LDO32_VSET_ldo32_vset_START (0)
#define PMIC_LDO32_VSET_ldo32_vset_END (4)
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
        unsigned char ldo38_vset : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO38_VSET_UNION;
#endif
#define PMIC_LDO38_VSET_ldo38_vset_START (0)
#define PMIC_LDO38_VSET_ldo38_vset_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo39_vset : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO39_VSET_UNION;
#endif
#define PMIC_LDO39_VSET_ldo39_vset_START (0)
#define PMIC_LDO39_VSET_ldo39_vset_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo41_vset : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO41_VSET_UNION;
#endif
#define PMIC_LDO41_VSET_ldo41_vset_START (0)
#define PMIC_LDO41_VSET_ldo41_vset_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo42_vset : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO42_VSET_UNION;
#endif
#define PMIC_LDO42_VSET_ldo42_vset_START (0)
#define PMIC_LDO42_VSET_ldo42_vset_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo43_vset : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO43_VSET_UNION;
#endif
#define PMIC_LDO43_VSET_ldo43_vset_START (0)
#define PMIC_LDO43_VSET_ldo43_vset_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo44_vset_cfg : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO44_VSET_UNION;
#endif
#define PMIC_LDO44_VSET_ldo44_vset_cfg_START (0)
#define PMIC_LDO44_VSET_ldo44_vset_cfg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo44_vset_eco : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO44_VSET_ECO_UNION;
#endif
#define PMIC_LDO44_VSET_ECO_ldo44_vset_eco_START (0)
#define PMIC_LDO44_VSET_ECO_ldo44_vset_eco_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo45_vset : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO45_VSET_UNION;
#endif
#define PMIC_LDO45_VSET_ldo45_vset_START (0)
#define PMIC_LDO45_VSET_ldo45_vset_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo46_vset : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO46_VSET_UNION;
#endif
#define PMIC_LDO46_VSET_ldo46_vset_START (0)
#define PMIC_LDO46_VSET_ldo46_vset_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo47_vset : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO47_VSET_UNION;
#endif
#define PMIC_LDO47_VSET_ldo47_vset_START (0)
#define PMIC_LDO47_VSET_ldo47_vset_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo48_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO48_VSET_UNION;
#endif
#define PMIC_LDO48_VSET_ldo48_vset_START (0)
#define PMIC_LDO48_VSET_ldo48_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo49_vset : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO49_VSET_UNION;
#endif
#define PMIC_LDO49_VSET_ldo49_vset_START (0)
#define PMIC_LDO49_VSET_ldo49_vset_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo50_vset : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO50_VSET_UNION;
#endif
#define PMIC_LDO50_VSET_ldo50_vset_START (0)
#define PMIC_LDO50_VSET_ldo50_vset_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo_buf_vset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO_BUF_VSET_UNION;
#endif
#define PMIC_LDO_BUF_VSET_ldo_buf_vset_START (0)
#define PMIC_LDO_BUF_VSET_ldo_buf_vset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pmu_idle_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_PMU_IDLE_EN_UNION;
#endif
#define PMIC_PMU_IDLE_EN_pmu_idle_en_START (0)
#define PMIC_PMU_IDLE_EN_pmu_idle_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo26_hd_mask : 1;
        unsigned char reg_ldo_buf_hd_mask : 1;
        unsigned char reg_sys_clk_hd_mask : 1;
        unsigned char reg_serdes_clk_hd_mask : 1;
        unsigned char reg_wifi_clk_hd_mask : 1;
        unsigned char reg_nfc_clk_hd_mask : 1;
        unsigned char reg_ldo29_hd_mask : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_HARDWIRE_CTRL0_UNION;
#endif
#define PMIC_HARDWIRE_CTRL0_reg_ldo26_hd_mask_START (0)
#define PMIC_HARDWIRE_CTRL0_reg_ldo26_hd_mask_END (0)
#define PMIC_HARDWIRE_CTRL0_reg_ldo_buf_hd_mask_START (1)
#define PMIC_HARDWIRE_CTRL0_reg_ldo_buf_hd_mask_END (1)
#define PMIC_HARDWIRE_CTRL0_reg_sys_clk_hd_mask_START (2)
#define PMIC_HARDWIRE_CTRL0_reg_sys_clk_hd_mask_END (2)
#define PMIC_HARDWIRE_CTRL0_reg_serdes_clk_hd_mask_START (3)
#define PMIC_HARDWIRE_CTRL0_reg_serdes_clk_hd_mask_END (3)
#define PMIC_HARDWIRE_CTRL0_reg_wifi_clk_hd_mask_START (4)
#define PMIC_HARDWIRE_CTRL0_reg_wifi_clk_hd_mask_END (4)
#define PMIC_HARDWIRE_CTRL0_reg_nfc_clk_hd_mask_START (5)
#define PMIC_HARDWIRE_CTRL0_reg_nfc_clk_hd_mask_END (5)
#define PMIC_HARDWIRE_CTRL0_reg_ldo29_hd_mask_START (6)
#define PMIC_HARDWIRE_CTRL0_reg_ldo29_hd_mask_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_eco_in_hd_mask : 1;
        unsigned char reg_idle_hd_mask : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_HARDWIRE_CTRL1_UNION;
#endif
#define PMIC_HARDWIRE_CTRL1_reg_eco_in_hd_mask_START (0)
#define PMIC_HARDWIRE_CTRL1_reg_eco_in_hd_mask_END (0)
#define PMIC_HARDWIRE_CTRL1_reg_idle_hd_mask_START (1)
#define PMIC_HARDWIRE_CTRL1_reg_idle_hd_mask_END (1)
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
        unsigned char peri_en_buck40_vset : 1;
        unsigned char peri_en_buck6_vset : 1;
        unsigned char peri_en_buck5_vset : 1;
        unsigned char peri_en_buck3_vset : 1;
        unsigned char peri_en_buck2_vset : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_VSET_CTRL0_UNION;
#endif
#define PMIC_PERI_VSET_CTRL0_peri_en_buck40_vset_START (0)
#define PMIC_PERI_VSET_CTRL0_peri_en_buck40_vset_END (0)
#define PMIC_PERI_VSET_CTRL0_peri_en_buck6_vset_START (1)
#define PMIC_PERI_VSET_CTRL0_peri_en_buck6_vset_END (1)
#define PMIC_PERI_VSET_CTRL0_peri_en_buck5_vset_START (2)
#define PMIC_PERI_VSET_CTRL0_peri_en_buck5_vset_END (2)
#define PMIC_PERI_VSET_CTRL0_peri_en_buck3_vset_START (3)
#define PMIC_PERI_VSET_CTRL0_peri_en_buck3_vset_END (3)
#define PMIC_PERI_VSET_CTRL0_peri_en_buck2_vset_START (4)
#define PMIC_PERI_VSET_CTRL0_peri_en_buck2_vset_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_ldo44_vset : 1;
        unsigned char peri_en_ldo6_vset : 1;
        unsigned char peri_en_ldo2_vset : 1;
        unsigned char peri_en_ldo0_vset : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_PERI_VSET_CTRL1_UNION;
#endif
#define PMIC_PERI_VSET_CTRL1_peri_en_ldo44_vset_START (0)
#define PMIC_PERI_VSET_CTRL1_peri_en_ldo44_vset_END (0)
#define PMIC_PERI_VSET_CTRL1_peri_en_ldo6_vset_START (1)
#define PMIC_PERI_VSET_CTRL1_peri_en_ldo6_vset_END (1)
#define PMIC_PERI_VSET_CTRL1_peri_en_ldo2_vset_START (2)
#define PMIC_PERI_VSET_CTRL1_peri_en_ldo2_vset_END (2)
#define PMIC_PERI_VSET_CTRL1_peri_en_ldo0_vset_START (3)
#define PMIC_PERI_VSET_CTRL1_peri_en_ldo0_vset_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sys_clk_en_buck0_vset : 1;
        unsigned char sys_clk_en_ldo0_vset : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_SYS_CLK_EN_VSET_CTRL_UNION;
#endif
#define PMIC_SYS_CLK_EN_VSET_CTRL_sys_clk_en_buck0_vset_START (0)
#define PMIC_SYS_CLK_EN_VSET_CTRL_sys_clk_en_buck0_vset_END (0)
#define PMIC_SYS_CLK_EN_VSET_CTRL_sys_clk_en_ldo0_vset_START (1)
#define PMIC_SYS_CLK_EN_VSET_CTRL_sys_clk_en_ldo0_vset_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_en_buck40_vset : 1;
        unsigned char idle_en_buck6_vset : 1;
        unsigned char idle_en_buck3_vset : 1;
        unsigned char idle_en_buck2_vset : 1;
        unsigned char idle_en_ldo44_vset : 1;
        unsigned char idle_en_ldo6_vset : 1;
        unsigned char idle_en_ldo2_vset : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_IDLE_EN_VSET_CTRL0_UNION;
#endif
#define PMIC_IDLE_EN_VSET_CTRL0_idle_en_buck40_vset_START (0)
#define PMIC_IDLE_EN_VSET_CTRL0_idle_en_buck40_vset_END (0)
#define PMIC_IDLE_EN_VSET_CTRL0_idle_en_buck6_vset_START (1)
#define PMIC_IDLE_EN_VSET_CTRL0_idle_en_buck6_vset_END (1)
#define PMIC_IDLE_EN_VSET_CTRL0_idle_en_buck3_vset_START (2)
#define PMIC_IDLE_EN_VSET_CTRL0_idle_en_buck3_vset_END (2)
#define PMIC_IDLE_EN_VSET_CTRL0_idle_en_buck2_vset_START (3)
#define PMIC_IDLE_EN_VSET_CTRL0_idle_en_buck2_vset_END (3)
#define PMIC_IDLE_EN_VSET_CTRL0_idle_en_ldo44_vset_START (4)
#define PMIC_IDLE_EN_VSET_CTRL0_idle_en_ldo44_vset_END (4)
#define PMIC_IDLE_EN_VSET_CTRL0_idle_en_ldo6_vset_START (5)
#define PMIC_IDLE_EN_VSET_CTRL0_idle_en_ldo6_vset_END (5)
#define PMIC_IDLE_EN_VSET_CTRL0_idle_en_ldo2_vset_START (6)
#define PMIC_IDLE_EN_VSET_CTRL0_idle_en_ldo2_vset_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_ldo6_on : 1;
        unsigned char peri_en_ldo5_on : 1;
        unsigned char peri_en_ldo0_on : 1;
        unsigned char peri_en_buck7_on : 1;
        unsigned char peri_en_buck5_on : 1;
        unsigned char peri_en_buck40_on : 1;
        unsigned char peri_en_buck2_on : 1;
        unsigned char peri_en_buck0_on : 1;
    } reg;
} PMIC_PERI_CTRL0_UNION;
#endif
#define PMIC_PERI_CTRL0_peri_en_ldo6_on_START (0)
#define PMIC_PERI_CTRL0_peri_en_ldo6_on_END (0)
#define PMIC_PERI_CTRL0_peri_en_ldo5_on_START (1)
#define PMIC_PERI_CTRL0_peri_en_ldo5_on_END (1)
#define PMIC_PERI_CTRL0_peri_en_ldo0_on_START (2)
#define PMIC_PERI_CTRL0_peri_en_ldo0_on_END (2)
#define PMIC_PERI_CTRL0_peri_en_buck7_on_START (3)
#define PMIC_PERI_CTRL0_peri_en_buck7_on_END (3)
#define PMIC_PERI_CTRL0_peri_en_buck5_on_START (4)
#define PMIC_PERI_CTRL0_peri_en_buck5_on_END (4)
#define PMIC_PERI_CTRL0_peri_en_buck40_on_START (5)
#define PMIC_PERI_CTRL0_peri_en_buck40_on_END (5)
#define PMIC_PERI_CTRL0_peri_en_buck2_on_START (6)
#define PMIC_PERI_CTRL0_peri_en_buck2_on_END (6)
#define PMIC_PERI_CTRL0_peri_en_buck0_on_START (7)
#define PMIC_PERI_CTRL0_peri_en_buck0_on_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_ldo38_on : 1;
        unsigned char peri_en_ldo37_on : 1;
        unsigned char peri_en_ldo30_on : 1;
        unsigned char peri_en_ldo28_on : 1;
        unsigned char peri_en_ldo27_on : 1;
        unsigned char peri_en_ldo23_on : 1;
        unsigned char peri_en_ldo9_on : 1;
        unsigned char peri_en_ldo8_on : 1;
    } reg;
} PMIC_PERI_CTRL1_UNION;
#endif
#define PMIC_PERI_CTRL1_peri_en_ldo38_on_START (0)
#define PMIC_PERI_CTRL1_peri_en_ldo38_on_END (0)
#define PMIC_PERI_CTRL1_peri_en_ldo37_on_START (1)
#define PMIC_PERI_CTRL1_peri_en_ldo37_on_END (1)
#define PMIC_PERI_CTRL1_peri_en_ldo30_on_START (2)
#define PMIC_PERI_CTRL1_peri_en_ldo30_on_END (2)
#define PMIC_PERI_CTRL1_peri_en_ldo28_on_START (3)
#define PMIC_PERI_CTRL1_peri_en_ldo28_on_END (3)
#define PMIC_PERI_CTRL1_peri_en_ldo27_on_START (4)
#define PMIC_PERI_CTRL1_peri_en_ldo27_on_END (4)
#define PMIC_PERI_CTRL1_peri_en_ldo23_on_START (5)
#define PMIC_PERI_CTRL1_peri_en_ldo23_on_END (5)
#define PMIC_PERI_CTRL1_peri_en_ldo9_on_START (6)
#define PMIC_PERI_CTRL1_peri_en_ldo9_on_END (6)
#define PMIC_PERI_CTRL1_peri_en_ldo8_on_START (7)
#define PMIC_PERI_CTRL1_peri_en_ldo8_on_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved_0 : 1;
        unsigned char peri_en_pmuh_on : 1;
        unsigned char peri_en_ldo50_on : 1;
        unsigned char peri_en_ldo44_on : 1;
        unsigned char peri_en_ldo41_on : 1;
        unsigned char peri_en_ldo39_on : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_PERI_CTRL2_UNION;
#endif
#define PMIC_PERI_CTRL2_peri_en_pmuh_on_START (1)
#define PMIC_PERI_CTRL2_peri_en_pmuh_on_END (1)
#define PMIC_PERI_CTRL2_peri_en_ldo50_on_START (2)
#define PMIC_PERI_CTRL2_peri_en_ldo50_on_END (2)
#define PMIC_PERI_CTRL2_peri_en_ldo44_on_START (3)
#define PMIC_PERI_CTRL2_peri_en_ldo44_on_END (3)
#define PMIC_PERI_CTRL2_peri_en_ldo41_on_START (4)
#define PMIC_PERI_CTRL2_peri_en_ldo41_on_END (4)
#define PMIC_PERI_CTRL2_peri_en_ldo39_on_START (5)
#define PMIC_PERI_CTRL2_peri_en_ldo39_on_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_buck7_eco : 1;
        unsigned char peri_en_buck6_eco : 1;
        unsigned char peri_en_buck5_eco : 1;
        unsigned char peri_en_buck40_eco : 1;
        unsigned char peri_en_buck3_eco : 1;
        unsigned char peri_en_buck2_eco : 1;
        unsigned char peri_en_buck1_eco : 1;
        unsigned char peri_en_buck0_eco : 1;
    } reg;
} PMIC_PERI_CTRL3_UNION;
#endif
#define PMIC_PERI_CTRL3_peri_en_buck7_eco_START (0)
#define PMIC_PERI_CTRL3_peri_en_buck7_eco_END (0)
#define PMIC_PERI_CTRL3_peri_en_buck6_eco_START (1)
#define PMIC_PERI_CTRL3_peri_en_buck6_eco_END (1)
#define PMIC_PERI_CTRL3_peri_en_buck5_eco_START (2)
#define PMIC_PERI_CTRL3_peri_en_buck5_eco_END (2)
#define PMIC_PERI_CTRL3_peri_en_buck40_eco_START (3)
#define PMIC_PERI_CTRL3_peri_en_buck40_eco_END (3)
#define PMIC_PERI_CTRL3_peri_en_buck3_eco_START (4)
#define PMIC_PERI_CTRL3_peri_en_buck3_eco_END (4)
#define PMIC_PERI_CTRL3_peri_en_buck2_eco_START (5)
#define PMIC_PERI_CTRL3_peri_en_buck2_eco_END (5)
#define PMIC_PERI_CTRL3_peri_en_buck1_eco_START (6)
#define PMIC_PERI_CTRL3_peri_en_buck1_eco_END (6)
#define PMIC_PERI_CTRL3_peri_en_buck0_eco_START (7)
#define PMIC_PERI_CTRL3_peri_en_buck0_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_ldo12_eco : 1;
        unsigned char peri_en_ldo11_eco : 1;
        unsigned char peri_en_ldo9_eco : 1;
        unsigned char peri_en_ldo8_eco : 1;
        unsigned char peri_en_ldo6_eco : 1;
        unsigned char peri_en_ldo5_eco : 1;
        unsigned char peri_en_ldo2_eco : 1;
        unsigned char peri_en_ldo0_eco : 1;
    } reg;
} PMIC_PERI_CTRL4_UNION;
#endif
#define PMIC_PERI_CTRL4_peri_en_ldo12_eco_START (0)
#define PMIC_PERI_CTRL4_peri_en_ldo12_eco_END (0)
#define PMIC_PERI_CTRL4_peri_en_ldo11_eco_START (1)
#define PMIC_PERI_CTRL4_peri_en_ldo11_eco_END (1)
#define PMIC_PERI_CTRL4_peri_en_ldo9_eco_START (2)
#define PMIC_PERI_CTRL4_peri_en_ldo9_eco_END (2)
#define PMIC_PERI_CTRL4_peri_en_ldo8_eco_START (3)
#define PMIC_PERI_CTRL4_peri_en_ldo8_eco_END (3)
#define PMIC_PERI_CTRL4_peri_en_ldo6_eco_START (4)
#define PMIC_PERI_CTRL4_peri_en_ldo6_eco_END (4)
#define PMIC_PERI_CTRL4_peri_en_ldo5_eco_START (5)
#define PMIC_PERI_CTRL4_peri_en_ldo5_eco_END (5)
#define PMIC_PERI_CTRL4_peri_en_ldo2_eco_START (6)
#define PMIC_PERI_CTRL4_peri_en_ldo2_eco_END (6)
#define PMIC_PERI_CTRL4_peri_en_ldo0_eco_START (7)
#define PMIC_PERI_CTRL4_peri_en_ldo0_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_ldo30_eco : 1;
        unsigned char peri_en_ldo29_eco : 1;
        unsigned char peri_en_ldo28_eco : 1;
        unsigned char peri_en_ldo27_eco : 1;
        unsigned char peri_en_ldo24_eco : 1;
        unsigned char peri_en_ldo23_eco : 1;
        unsigned char peri_en_ldo18_eco : 1;
        unsigned char peri_en_ldo15_eco : 1;
    } reg;
} PMIC_PERI_CTRL5_UNION;
#endif
#define PMIC_PERI_CTRL5_peri_en_ldo30_eco_START (0)
#define PMIC_PERI_CTRL5_peri_en_ldo30_eco_END (0)
#define PMIC_PERI_CTRL5_peri_en_ldo29_eco_START (1)
#define PMIC_PERI_CTRL5_peri_en_ldo29_eco_END (1)
#define PMIC_PERI_CTRL5_peri_en_ldo28_eco_START (2)
#define PMIC_PERI_CTRL5_peri_en_ldo28_eco_END (2)
#define PMIC_PERI_CTRL5_peri_en_ldo27_eco_START (3)
#define PMIC_PERI_CTRL5_peri_en_ldo27_eco_END (3)
#define PMIC_PERI_CTRL5_peri_en_ldo24_eco_START (4)
#define PMIC_PERI_CTRL5_peri_en_ldo24_eco_END (4)
#define PMIC_PERI_CTRL5_peri_en_ldo23_eco_START (5)
#define PMIC_PERI_CTRL5_peri_en_ldo23_eco_END (5)
#define PMIC_PERI_CTRL5_peri_en_ldo18_eco_START (6)
#define PMIC_PERI_CTRL5_peri_en_ldo18_eco_END (6)
#define PMIC_PERI_CTRL5_peri_en_ldo15_eco_START (7)
#define PMIC_PERI_CTRL5_peri_en_ldo15_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_ldo49_eco : 1;
        unsigned char peri_en_ldo45_eco : 1;
        unsigned char peri_en_ldo44_eco : 1;
        unsigned char peri_en_ldo41_eco : 1;
        unsigned char peri_en_ldo39_eco : 1;
        unsigned char peri_en_ldo38_eco : 1;
        unsigned char peri_en_ldo37_eco : 1;
        unsigned char peri_en_ldo34_eco : 1;
    } reg;
} PMIC_PERI_CTRL6_UNION;
#endif
#define PMIC_PERI_CTRL6_peri_en_ldo49_eco_START (0)
#define PMIC_PERI_CTRL6_peri_en_ldo49_eco_END (0)
#define PMIC_PERI_CTRL6_peri_en_ldo45_eco_START (1)
#define PMIC_PERI_CTRL6_peri_en_ldo45_eco_END (1)
#define PMIC_PERI_CTRL6_peri_en_ldo44_eco_START (2)
#define PMIC_PERI_CTRL6_peri_en_ldo44_eco_END (2)
#define PMIC_PERI_CTRL6_peri_en_ldo41_eco_START (3)
#define PMIC_PERI_CTRL6_peri_en_ldo41_eco_END (3)
#define PMIC_PERI_CTRL6_peri_en_ldo39_eco_START (4)
#define PMIC_PERI_CTRL6_peri_en_ldo39_eco_END (4)
#define PMIC_PERI_CTRL6_peri_en_ldo38_eco_START (5)
#define PMIC_PERI_CTRL6_peri_en_ldo38_eco_END (5)
#define PMIC_PERI_CTRL6_peri_en_ldo37_eco_START (6)
#define PMIC_PERI_CTRL6_peri_en_ldo37_eco_END (6)
#define PMIC_PERI_CTRL6_peri_en_ldo34_eco_START (7)
#define PMIC_PERI_CTRL6_peri_en_ldo34_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_en_pmuh_eco : 1;
        unsigned char peri_en_ref_eco : 1;
        unsigned char peri_en_ldo50_eco : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_PERI_CTRL7_UNION;
#endif
#define PMIC_PERI_CTRL7_peri_en_pmuh_eco_START (0)
#define PMIC_PERI_CTRL7_peri_en_pmuh_eco_END (0)
#define PMIC_PERI_CTRL7_peri_en_ref_eco_START (1)
#define PMIC_PERI_CTRL7_peri_en_ref_eco_END (1)
#define PMIC_PERI_CTRL7_peri_en_ldo50_eco_START (2)
#define PMIC_PERI_CTRL7_peri_en_ldo50_eco_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo6_off_sel : 1;
        unsigned char peri_ldo5_off_sel : 1;
        unsigned char peri_ldo0_off_sel : 1;
        unsigned char peri_buck7_off_sel : 1;
        unsigned char peri_buck5_off_sel : 1;
        unsigned char peri_buck40_off_sel : 1;
        unsigned char peri_buck2_off_sel : 1;
        unsigned char peri_buck0_off_sel : 1;
    } reg;
} PMIC_PERI_OFF_CTRL0_UNION;
#endif
#define PMIC_PERI_OFF_CTRL0_peri_ldo6_off_sel_START (0)
#define PMIC_PERI_OFF_CTRL0_peri_ldo6_off_sel_END (0)
#define PMIC_PERI_OFF_CTRL0_peri_ldo5_off_sel_START (1)
#define PMIC_PERI_OFF_CTRL0_peri_ldo5_off_sel_END (1)
#define PMIC_PERI_OFF_CTRL0_peri_ldo0_off_sel_START (2)
#define PMIC_PERI_OFF_CTRL0_peri_ldo0_off_sel_END (2)
#define PMIC_PERI_OFF_CTRL0_peri_buck7_off_sel_START (3)
#define PMIC_PERI_OFF_CTRL0_peri_buck7_off_sel_END (3)
#define PMIC_PERI_OFF_CTRL0_peri_buck5_off_sel_START (4)
#define PMIC_PERI_OFF_CTRL0_peri_buck5_off_sel_END (4)
#define PMIC_PERI_OFF_CTRL0_peri_buck40_off_sel_START (5)
#define PMIC_PERI_OFF_CTRL0_peri_buck40_off_sel_END (5)
#define PMIC_PERI_OFF_CTRL0_peri_buck2_off_sel_START (6)
#define PMIC_PERI_OFF_CTRL0_peri_buck2_off_sel_END (6)
#define PMIC_PERI_OFF_CTRL0_peri_buck0_off_sel_START (7)
#define PMIC_PERI_OFF_CTRL0_peri_buck0_off_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo38_off_sel : 1;
        unsigned char peri_ldo37_off_sel : 1;
        unsigned char peri_ldo30_off_sel : 1;
        unsigned char peri_ldo28_off_sel : 1;
        unsigned char peri_ldo27_off_sel : 1;
        unsigned char peri_ldo23_off_sel : 1;
        unsigned char peri_ldo9_off_sel : 1;
        unsigned char peri_ldo8_off_sel : 1;
    } reg;
} PMIC_PERI_OFF_CTRL1_UNION;
#endif
#define PMIC_PERI_OFF_CTRL1_peri_ldo38_off_sel_START (0)
#define PMIC_PERI_OFF_CTRL1_peri_ldo38_off_sel_END (0)
#define PMIC_PERI_OFF_CTRL1_peri_ldo37_off_sel_START (1)
#define PMIC_PERI_OFF_CTRL1_peri_ldo37_off_sel_END (1)
#define PMIC_PERI_OFF_CTRL1_peri_ldo30_off_sel_START (2)
#define PMIC_PERI_OFF_CTRL1_peri_ldo30_off_sel_END (2)
#define PMIC_PERI_OFF_CTRL1_peri_ldo28_off_sel_START (3)
#define PMIC_PERI_OFF_CTRL1_peri_ldo28_off_sel_END (3)
#define PMIC_PERI_OFF_CTRL1_peri_ldo27_off_sel_START (4)
#define PMIC_PERI_OFF_CTRL1_peri_ldo27_off_sel_END (4)
#define PMIC_PERI_OFF_CTRL1_peri_ldo23_off_sel_START (5)
#define PMIC_PERI_OFF_CTRL1_peri_ldo23_off_sel_END (5)
#define PMIC_PERI_OFF_CTRL1_peri_ldo9_off_sel_START (6)
#define PMIC_PERI_OFF_CTRL1_peri_ldo9_off_sel_END (6)
#define PMIC_PERI_OFF_CTRL1_peri_ldo8_off_sel_START (7)
#define PMIC_PERI_OFF_CTRL1_peri_ldo8_off_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved_0 : 1;
        unsigned char peri_pmuh_off_sel : 1;
        unsigned char peri_ldo50_off_sel : 1;
        unsigned char peri_ldo44_off_sel : 1;
        unsigned char peri_ldo41_off_sel : 1;
        unsigned char peri_ldo39_off_sel : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_PERI_OFF_CTRL2_UNION;
#endif
#define PMIC_PERI_OFF_CTRL2_peri_pmuh_off_sel_START (1)
#define PMIC_PERI_OFF_CTRL2_peri_pmuh_off_sel_END (1)
#define PMIC_PERI_OFF_CTRL2_peri_ldo50_off_sel_START (2)
#define PMIC_PERI_OFF_CTRL2_peri_ldo50_off_sel_END (2)
#define PMIC_PERI_OFF_CTRL2_peri_ldo44_off_sel_START (3)
#define PMIC_PERI_OFF_CTRL2_peri_ldo44_off_sel_END (3)
#define PMIC_PERI_OFF_CTRL2_peri_ldo41_off_sel_START (4)
#define PMIC_PERI_OFF_CTRL2_peri_ldo41_off_sel_END (4)
#define PMIC_PERI_OFF_CTRL2_peri_ldo39_off_sel_START (5)
#define PMIC_PERI_OFF_CTRL2_peri_ldo39_off_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_buck00_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_PLACE_CTRL0_UNION;
#endif
#define PMIC_PERI_PLACE_CTRL0_peri_buck00_sel_START (0)
#define PMIC_PERI_PLACE_CTRL0_peri_buck00_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_buck2_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_PLACE_CTRL1_UNION;
#endif
#define PMIC_PERI_PLACE_CTRL1_peri_buck2_sel_START (0)
#define PMIC_PERI_PLACE_CTRL1_peri_buck2_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_buck40_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_PLACE_CTRL2_UNION;
#endif
#define PMIC_PERI_PLACE_CTRL2_peri_buck40_sel_START (0)
#define PMIC_PERI_PLACE_CTRL2_peri_buck40_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_buck5_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_PLACE_CTRL3_UNION;
#endif
#define PMIC_PERI_PLACE_CTRL3_peri_buck5_sel_START (0)
#define PMIC_PERI_PLACE_CTRL3_peri_buck5_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_buck7_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_PLACE_CTRL5_UNION;
#endif
#define PMIC_PERI_PLACE_CTRL5_peri_buck7_sel_START (0)
#define PMIC_PERI_PLACE_CTRL5_peri_buck7_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo0_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_PLACE_CTRL6_UNION;
#endif
#define PMIC_PERI_PLACE_CTRL6_peri_ldo0_sel_START (0)
#define PMIC_PERI_PLACE_CTRL6_peri_ldo0_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo5_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_PLACE_CTRL7_UNION;
#endif
#define PMIC_PERI_PLACE_CTRL7_peri_ldo5_sel_START (0)
#define PMIC_PERI_PLACE_CTRL7_peri_ldo5_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo6_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_PLACE_CTRL8_UNION;
#endif
#define PMIC_PERI_PLACE_CTRL8_peri_ldo6_sel_START (0)
#define PMIC_PERI_PLACE_CTRL8_peri_ldo6_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo8_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_PLACE_CTRL9_UNION;
#endif
#define PMIC_PERI_PLACE_CTRL9_peri_ldo8_sel_START (0)
#define PMIC_PERI_PLACE_CTRL9_peri_ldo8_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo9_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_PLACE_CTRL10_UNION;
#endif
#define PMIC_PERI_PLACE_CTRL10_peri_ldo9_sel_START (0)
#define PMIC_PERI_PLACE_CTRL10_peri_ldo9_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo23_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_PLACE_CTRL11_UNION;
#endif
#define PMIC_PERI_PLACE_CTRL11_peri_ldo23_sel_START (0)
#define PMIC_PERI_PLACE_CTRL11_peri_ldo23_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo27_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_PLACE_CTRL12_UNION;
#endif
#define PMIC_PERI_PLACE_CTRL12_peri_ldo27_sel_START (0)
#define PMIC_PERI_PLACE_CTRL12_peri_ldo27_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo28_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_PLACE_CTRL13_UNION;
#endif
#define PMIC_PERI_PLACE_CTRL13_peri_ldo28_sel_START (0)
#define PMIC_PERI_PLACE_CTRL13_peri_ldo28_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo30_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_PLACE_CTRL14_UNION;
#endif
#define PMIC_PERI_PLACE_CTRL14_peri_ldo30_sel_START (0)
#define PMIC_PERI_PLACE_CTRL14_peri_ldo30_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo37_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_PLACE_CTRL15_UNION;
#endif
#define PMIC_PERI_PLACE_CTRL15_peri_ldo37_sel_START (0)
#define PMIC_PERI_PLACE_CTRL15_peri_ldo37_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo38_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_PLACE_CTRL16_UNION;
#endif
#define PMIC_PERI_PLACE_CTRL16_peri_ldo38_sel_START (0)
#define PMIC_PERI_PLACE_CTRL16_peri_ldo38_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo39_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_PLACE_CTRL17_UNION;
#endif
#define PMIC_PERI_PLACE_CTRL17_peri_ldo39_sel_START (0)
#define PMIC_PERI_PLACE_CTRL17_peri_ldo39_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo41_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_PLACE_CTRL18_UNION;
#endif
#define PMIC_PERI_PLACE_CTRL18_peri_ldo41_sel_START (0)
#define PMIC_PERI_PLACE_CTRL18_peri_ldo41_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo44_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_PLACE_CTRL19_UNION;
#endif
#define PMIC_PERI_PLACE_CTRL19_peri_ldo44_sel_START (0)
#define PMIC_PERI_PLACE_CTRL19_peri_ldo44_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_ldo50_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_PLACE_CTRL20_UNION;
#endif
#define PMIC_PERI_PLACE_CTRL20_peri_ldo50_sel_START (0)
#define PMIC_PERI_PLACE_CTRL20_peri_ldo50_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char peri_pmuh_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_PERI_PLACE_CTRL21_UNION;
#endif
#define PMIC_PERI_PLACE_CTRL21_peri_pmuh_sel_START (0)
#define PMIC_PERI_PLACE_CTRL21_peri_pmuh_sel_END (4)
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
        unsigned char idle_en_ldo6_on : 1;
        unsigned char idle_en_ldo5_on : 1;
        unsigned char idle_en_ldo0_on : 1;
        unsigned char idle_en_buck7_on : 1;
        unsigned char idle_en_buck5_on : 1;
        unsigned char idle_en_buck40_on : 1;
        unsigned char idle_en_buck2_on : 1;
        unsigned char idle_en_buck0_on : 1;
    } reg;
} PMIC_IDLE_CTRL0_UNION;
#endif
#define PMIC_IDLE_CTRL0_idle_en_ldo6_on_START (0)
#define PMIC_IDLE_CTRL0_idle_en_ldo6_on_END (0)
#define PMIC_IDLE_CTRL0_idle_en_ldo5_on_START (1)
#define PMIC_IDLE_CTRL0_idle_en_ldo5_on_END (1)
#define PMIC_IDLE_CTRL0_idle_en_ldo0_on_START (2)
#define PMIC_IDLE_CTRL0_idle_en_ldo0_on_END (2)
#define PMIC_IDLE_CTRL0_idle_en_buck7_on_START (3)
#define PMIC_IDLE_CTRL0_idle_en_buck7_on_END (3)
#define PMIC_IDLE_CTRL0_idle_en_buck5_on_START (4)
#define PMIC_IDLE_CTRL0_idle_en_buck5_on_END (4)
#define PMIC_IDLE_CTRL0_idle_en_buck40_on_START (5)
#define PMIC_IDLE_CTRL0_idle_en_buck40_on_END (5)
#define PMIC_IDLE_CTRL0_idle_en_buck2_on_START (6)
#define PMIC_IDLE_CTRL0_idle_en_buck2_on_END (6)
#define PMIC_IDLE_CTRL0_idle_en_buck0_on_START (7)
#define PMIC_IDLE_CTRL0_idle_en_buck0_on_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_en_ldo38_on : 1;
        unsigned char idle_en_ldo37_on : 1;
        unsigned char idle_en_ldo30_on : 1;
        unsigned char idle_en_ldo28_on : 1;
        unsigned char idle_en_ldo27_on : 1;
        unsigned char idle_en_ldo23_on : 1;
        unsigned char idle_en_ldo9_on : 1;
        unsigned char idle_en_ldo8_on : 1;
    } reg;
} PMIC_IDLE_CTRL1_UNION;
#endif
#define PMIC_IDLE_CTRL1_idle_en_ldo38_on_START (0)
#define PMIC_IDLE_CTRL1_idle_en_ldo38_on_END (0)
#define PMIC_IDLE_CTRL1_idle_en_ldo37_on_START (1)
#define PMIC_IDLE_CTRL1_idle_en_ldo37_on_END (1)
#define PMIC_IDLE_CTRL1_idle_en_ldo30_on_START (2)
#define PMIC_IDLE_CTRL1_idle_en_ldo30_on_END (2)
#define PMIC_IDLE_CTRL1_idle_en_ldo28_on_START (3)
#define PMIC_IDLE_CTRL1_idle_en_ldo28_on_END (3)
#define PMIC_IDLE_CTRL1_idle_en_ldo27_on_START (4)
#define PMIC_IDLE_CTRL1_idle_en_ldo27_on_END (4)
#define PMIC_IDLE_CTRL1_idle_en_ldo23_on_START (5)
#define PMIC_IDLE_CTRL1_idle_en_ldo23_on_END (5)
#define PMIC_IDLE_CTRL1_idle_en_ldo9_on_START (6)
#define PMIC_IDLE_CTRL1_idle_en_ldo9_on_END (6)
#define PMIC_IDLE_CTRL1_idle_en_ldo8_on_START (7)
#define PMIC_IDLE_CTRL1_idle_en_ldo8_on_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved_0 : 1;
        unsigned char idle_en_pmuh_on : 1;
        unsigned char idle_en_ldo50_on : 1;
        unsigned char idle_en_ldo44_on : 1;
        unsigned char idle_en_ldo41_on : 1;
        unsigned char idle_en_ldo39_on : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_IDLE_CTRL2_UNION;
#endif
#define PMIC_IDLE_CTRL2_idle_en_pmuh_on_START (1)
#define PMIC_IDLE_CTRL2_idle_en_pmuh_on_END (1)
#define PMIC_IDLE_CTRL2_idle_en_ldo50_on_START (2)
#define PMIC_IDLE_CTRL2_idle_en_ldo50_on_END (2)
#define PMIC_IDLE_CTRL2_idle_en_ldo44_on_START (3)
#define PMIC_IDLE_CTRL2_idle_en_ldo44_on_END (3)
#define PMIC_IDLE_CTRL2_idle_en_ldo41_on_START (4)
#define PMIC_IDLE_CTRL2_idle_en_ldo41_on_END (4)
#define PMIC_IDLE_CTRL2_idle_en_ldo39_on_START (5)
#define PMIC_IDLE_CTRL2_idle_en_ldo39_on_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_en_buck6_eco : 1;
        unsigned char idle_en_buck5_eco : 1;
        unsigned char idle_en_buck40_eco : 1;
        unsigned char idle_en_buck3_eco : 1;
        unsigned char idle_en_buck2_eco : 1;
        unsigned char idle_en_buck1_eco : 1;
        unsigned char idle_en_buck0_eco : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_IDLE_CTRL3_UNION;
#endif
#define PMIC_IDLE_CTRL3_idle_en_buck6_eco_START (0)
#define PMIC_IDLE_CTRL3_idle_en_buck6_eco_END (0)
#define PMIC_IDLE_CTRL3_idle_en_buck5_eco_START (1)
#define PMIC_IDLE_CTRL3_idle_en_buck5_eco_END (1)
#define PMIC_IDLE_CTRL3_idle_en_buck40_eco_START (2)
#define PMIC_IDLE_CTRL3_idle_en_buck40_eco_END (2)
#define PMIC_IDLE_CTRL3_idle_en_buck3_eco_START (3)
#define PMIC_IDLE_CTRL3_idle_en_buck3_eco_END (3)
#define PMIC_IDLE_CTRL3_idle_en_buck2_eco_START (4)
#define PMIC_IDLE_CTRL3_idle_en_buck2_eco_END (4)
#define PMIC_IDLE_CTRL3_idle_en_buck1_eco_START (5)
#define PMIC_IDLE_CTRL3_idle_en_buck1_eco_END (5)
#define PMIC_IDLE_CTRL3_idle_en_buck0_eco_START (6)
#define PMIC_IDLE_CTRL3_idle_en_buck0_eco_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_en_ldo12_eco : 1;
        unsigned char idle_en_ldo11_eco : 1;
        unsigned char idle_en_ldo9_eco : 1;
        unsigned char idle_en_ldo8_eco : 1;
        unsigned char idle_en_ldo6_eco : 1;
        unsigned char idle_en_ldo5_eco : 1;
        unsigned char idle_en_ldo2_eco : 1;
        unsigned char idle_en_ldo0_eco : 1;
    } reg;
} PMIC_IDLE_CTRL4_UNION;
#endif
#define PMIC_IDLE_CTRL4_idle_en_ldo12_eco_START (0)
#define PMIC_IDLE_CTRL4_idle_en_ldo12_eco_END (0)
#define PMIC_IDLE_CTRL4_idle_en_ldo11_eco_START (1)
#define PMIC_IDLE_CTRL4_idle_en_ldo11_eco_END (1)
#define PMIC_IDLE_CTRL4_idle_en_ldo9_eco_START (2)
#define PMIC_IDLE_CTRL4_idle_en_ldo9_eco_END (2)
#define PMIC_IDLE_CTRL4_idle_en_ldo8_eco_START (3)
#define PMIC_IDLE_CTRL4_idle_en_ldo8_eco_END (3)
#define PMIC_IDLE_CTRL4_idle_en_ldo6_eco_START (4)
#define PMIC_IDLE_CTRL4_idle_en_ldo6_eco_END (4)
#define PMIC_IDLE_CTRL4_idle_en_ldo5_eco_START (5)
#define PMIC_IDLE_CTRL4_idle_en_ldo5_eco_END (5)
#define PMIC_IDLE_CTRL4_idle_en_ldo2_eco_START (6)
#define PMIC_IDLE_CTRL4_idle_en_ldo2_eco_END (6)
#define PMIC_IDLE_CTRL4_idle_en_ldo0_eco_START (7)
#define PMIC_IDLE_CTRL4_idle_en_ldo0_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_en_ldo30_eco : 1;
        unsigned char idle_en_ldo29_eco : 1;
        unsigned char idle_en_ldo28_eco : 1;
        unsigned char idle_en_ldo27_eco : 1;
        unsigned char idle_en_ldo24_eco : 1;
        unsigned char idle_en_ldo23_eco : 1;
        unsigned char idle_en_ldo18_eco : 1;
        unsigned char idle_en_ldo15_eco : 1;
    } reg;
} PMIC_IDLE_CTRL5_UNION;
#endif
#define PMIC_IDLE_CTRL5_idle_en_ldo30_eco_START (0)
#define PMIC_IDLE_CTRL5_idle_en_ldo30_eco_END (0)
#define PMIC_IDLE_CTRL5_idle_en_ldo29_eco_START (1)
#define PMIC_IDLE_CTRL5_idle_en_ldo29_eco_END (1)
#define PMIC_IDLE_CTRL5_idle_en_ldo28_eco_START (2)
#define PMIC_IDLE_CTRL5_idle_en_ldo28_eco_END (2)
#define PMIC_IDLE_CTRL5_idle_en_ldo27_eco_START (3)
#define PMIC_IDLE_CTRL5_idle_en_ldo27_eco_END (3)
#define PMIC_IDLE_CTRL5_idle_en_ldo24_eco_START (4)
#define PMIC_IDLE_CTRL5_idle_en_ldo24_eco_END (4)
#define PMIC_IDLE_CTRL5_idle_en_ldo23_eco_START (5)
#define PMIC_IDLE_CTRL5_idle_en_ldo23_eco_END (5)
#define PMIC_IDLE_CTRL5_idle_en_ldo18_eco_START (6)
#define PMIC_IDLE_CTRL5_idle_en_ldo18_eco_END (6)
#define PMIC_IDLE_CTRL5_idle_en_ldo15_eco_START (7)
#define PMIC_IDLE_CTRL5_idle_en_ldo15_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_en_ldo49_eco : 1;
        unsigned char idle_en_ldo45_eco : 1;
        unsigned char idle_en_ldo44_eco : 1;
        unsigned char idle_en_ldo41_eco : 1;
        unsigned char idle_en_ldo39_eco : 1;
        unsigned char idle_en_ldo38_eco : 1;
        unsigned char idle_en_ldo37_eco : 1;
        unsigned char idle_en_ldo34_eco : 1;
    } reg;
} PMIC_IDLE_CTRL6_UNION;
#endif
#define PMIC_IDLE_CTRL6_idle_en_ldo49_eco_START (0)
#define PMIC_IDLE_CTRL6_idle_en_ldo49_eco_END (0)
#define PMIC_IDLE_CTRL6_idle_en_ldo45_eco_START (1)
#define PMIC_IDLE_CTRL6_idle_en_ldo45_eco_END (1)
#define PMIC_IDLE_CTRL6_idle_en_ldo44_eco_START (2)
#define PMIC_IDLE_CTRL6_idle_en_ldo44_eco_END (2)
#define PMIC_IDLE_CTRL6_idle_en_ldo41_eco_START (3)
#define PMIC_IDLE_CTRL6_idle_en_ldo41_eco_END (3)
#define PMIC_IDLE_CTRL6_idle_en_ldo39_eco_START (4)
#define PMIC_IDLE_CTRL6_idle_en_ldo39_eco_END (4)
#define PMIC_IDLE_CTRL6_idle_en_ldo38_eco_START (5)
#define PMIC_IDLE_CTRL6_idle_en_ldo38_eco_END (5)
#define PMIC_IDLE_CTRL6_idle_en_ldo37_eco_START (6)
#define PMIC_IDLE_CTRL6_idle_en_ldo37_eco_END (6)
#define PMIC_IDLE_CTRL6_idle_en_ldo34_eco_START (7)
#define PMIC_IDLE_CTRL6_idle_en_ldo34_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_en_pmuh_eco : 1;
        unsigned char idle_en_ref_eco : 1;
        unsigned char idle_en_ldo50_eco : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_IDLE_CTRL7_UNION;
#endif
#define PMIC_IDLE_CTRL7_idle_en_pmuh_eco_START (0)
#define PMIC_IDLE_CTRL7_idle_en_pmuh_eco_END (0)
#define PMIC_IDLE_CTRL7_idle_en_ref_eco_START (1)
#define PMIC_IDLE_CTRL7_idle_en_ref_eco_END (1)
#define PMIC_IDLE_CTRL7_idle_en_ldo50_eco_START (2)
#define PMIC_IDLE_CTRL7_idle_en_ldo50_eco_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo6_off_sel : 1;
        unsigned char idle_ldo5_off_sel : 1;
        unsigned char idle_ldo0_off_sel : 1;
        unsigned char idle_buck7_off_sel : 1;
        unsigned char idle_buck5_off_sel : 1;
        unsigned char idle_buck40_off_sel : 1;
        unsigned char idle_buck2_off_sel : 1;
        unsigned char idle_buck0_off_sel : 1;
    } reg;
} PMIC_IDLE_OFF_SEL0_UNION;
#endif
#define PMIC_IDLE_OFF_SEL0_idle_ldo6_off_sel_START (0)
#define PMIC_IDLE_OFF_SEL0_idle_ldo6_off_sel_END (0)
#define PMIC_IDLE_OFF_SEL0_idle_ldo5_off_sel_START (1)
#define PMIC_IDLE_OFF_SEL0_idle_ldo5_off_sel_END (1)
#define PMIC_IDLE_OFF_SEL0_idle_ldo0_off_sel_START (2)
#define PMIC_IDLE_OFF_SEL0_idle_ldo0_off_sel_END (2)
#define PMIC_IDLE_OFF_SEL0_idle_buck7_off_sel_START (3)
#define PMIC_IDLE_OFF_SEL0_idle_buck7_off_sel_END (3)
#define PMIC_IDLE_OFF_SEL0_idle_buck5_off_sel_START (4)
#define PMIC_IDLE_OFF_SEL0_idle_buck5_off_sel_END (4)
#define PMIC_IDLE_OFF_SEL0_idle_buck40_off_sel_START (5)
#define PMIC_IDLE_OFF_SEL0_idle_buck40_off_sel_END (5)
#define PMIC_IDLE_OFF_SEL0_idle_buck2_off_sel_START (6)
#define PMIC_IDLE_OFF_SEL0_idle_buck2_off_sel_END (6)
#define PMIC_IDLE_OFF_SEL0_idle_buck0_off_sel_START (7)
#define PMIC_IDLE_OFF_SEL0_idle_buck0_off_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo38_off_sel : 1;
        unsigned char idle_ldo37_off_sel : 1;
        unsigned char idle_ldo30_off_sel : 1;
        unsigned char idle_ldo28_off_sel : 1;
        unsigned char idle_ldo27_off_sel : 1;
        unsigned char idle_ldo23_off_sel : 1;
        unsigned char idle_ldo9_off_sel : 1;
        unsigned char idle_ldo8_off_sel : 1;
    } reg;
} PMIC_IDLE_OFF_SEL1_UNION;
#endif
#define PMIC_IDLE_OFF_SEL1_idle_ldo38_off_sel_START (0)
#define PMIC_IDLE_OFF_SEL1_idle_ldo38_off_sel_END (0)
#define PMIC_IDLE_OFF_SEL1_idle_ldo37_off_sel_START (1)
#define PMIC_IDLE_OFF_SEL1_idle_ldo37_off_sel_END (1)
#define PMIC_IDLE_OFF_SEL1_idle_ldo30_off_sel_START (2)
#define PMIC_IDLE_OFF_SEL1_idle_ldo30_off_sel_END (2)
#define PMIC_IDLE_OFF_SEL1_idle_ldo28_off_sel_START (3)
#define PMIC_IDLE_OFF_SEL1_idle_ldo28_off_sel_END (3)
#define PMIC_IDLE_OFF_SEL1_idle_ldo27_off_sel_START (4)
#define PMIC_IDLE_OFF_SEL1_idle_ldo27_off_sel_END (4)
#define PMIC_IDLE_OFF_SEL1_idle_ldo23_off_sel_START (5)
#define PMIC_IDLE_OFF_SEL1_idle_ldo23_off_sel_END (5)
#define PMIC_IDLE_OFF_SEL1_idle_ldo9_off_sel_START (6)
#define PMIC_IDLE_OFF_SEL1_idle_ldo9_off_sel_END (6)
#define PMIC_IDLE_OFF_SEL1_idle_ldo8_off_sel_START (7)
#define PMIC_IDLE_OFF_SEL1_idle_ldo8_off_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved_0 : 1;
        unsigned char idle_pmuh_off_sel : 1;
        unsigned char idle_ldo50_off_sel : 1;
        unsigned char idle_ldo44_off_sel : 1;
        unsigned char idle_ldo41_off_sel : 1;
        unsigned char idle_ldo39_off_sel : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_IDLE_OFF_SEL2_UNION;
#endif
#define PMIC_IDLE_OFF_SEL2_idle_pmuh_off_sel_START (1)
#define PMIC_IDLE_OFF_SEL2_idle_pmuh_off_sel_END (1)
#define PMIC_IDLE_OFF_SEL2_idle_ldo50_off_sel_START (2)
#define PMIC_IDLE_OFF_SEL2_idle_ldo50_off_sel_END (2)
#define PMIC_IDLE_OFF_SEL2_idle_ldo44_off_sel_START (3)
#define PMIC_IDLE_OFF_SEL2_idle_ldo44_off_sel_END (3)
#define PMIC_IDLE_OFF_SEL2_idle_ldo41_off_sel_START (4)
#define PMIC_IDLE_OFF_SEL2_idle_ldo41_off_sel_END (4)
#define PMIC_IDLE_OFF_SEL2_idle_ldo39_off_sel_START (5)
#define PMIC_IDLE_OFF_SEL2_idle_ldo39_off_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_buck00_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_PLACE_CTRL0_UNION;
#endif
#define PMIC_IDLE_PLACE_CTRL0_idle_buck00_sel_START (0)
#define PMIC_IDLE_PLACE_CTRL0_idle_buck00_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_buck2_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_PLACE_CTRL1_UNION;
#endif
#define PMIC_IDLE_PLACE_CTRL1_idle_buck2_sel_START (0)
#define PMIC_IDLE_PLACE_CTRL1_idle_buck2_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_buck40_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_PLACE_CTRL2_UNION;
#endif
#define PMIC_IDLE_PLACE_CTRL2_idle_buck40_sel_START (0)
#define PMIC_IDLE_PLACE_CTRL2_idle_buck40_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_buck5_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_PLACE_CTRL3_UNION;
#endif
#define PMIC_IDLE_PLACE_CTRL3_idle_buck5_sel_START (0)
#define PMIC_IDLE_PLACE_CTRL3_idle_buck5_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_buck7_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_PLACE_CTRL5_UNION;
#endif
#define PMIC_IDLE_PLACE_CTRL5_idle_buck7_sel_START (0)
#define PMIC_IDLE_PLACE_CTRL5_idle_buck7_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo0_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_PLACE_CTRL6_UNION;
#endif
#define PMIC_IDLE_PLACE_CTRL6_idle_ldo0_sel_START (0)
#define PMIC_IDLE_PLACE_CTRL6_idle_ldo0_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo5_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_PLACE_CTRL7_UNION;
#endif
#define PMIC_IDLE_PLACE_CTRL7_idle_ldo5_sel_START (0)
#define PMIC_IDLE_PLACE_CTRL7_idle_ldo5_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo6_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_PLACE_CTRL8_UNION;
#endif
#define PMIC_IDLE_PLACE_CTRL8_idle_ldo6_sel_START (0)
#define PMIC_IDLE_PLACE_CTRL8_idle_ldo6_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo8_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_PLACE_CTRL9_UNION;
#endif
#define PMIC_IDLE_PLACE_CTRL9_idle_ldo8_sel_START (0)
#define PMIC_IDLE_PLACE_CTRL9_idle_ldo8_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo9_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_PLACE_CTRL10_UNION;
#endif
#define PMIC_IDLE_PLACE_CTRL10_idle_ldo9_sel_START (0)
#define PMIC_IDLE_PLACE_CTRL10_idle_ldo9_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo23_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_PLACE_CTRL11_UNION;
#endif
#define PMIC_IDLE_PLACE_CTRL11_idle_ldo23_sel_START (0)
#define PMIC_IDLE_PLACE_CTRL11_idle_ldo23_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo27_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_PLACE_CTRL12_UNION;
#endif
#define PMIC_IDLE_PLACE_CTRL12_idle_ldo27_sel_START (0)
#define PMIC_IDLE_PLACE_CTRL12_idle_ldo27_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo28_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_PLACE_CTRL13_UNION;
#endif
#define PMIC_IDLE_PLACE_CTRL13_idle_ldo28_sel_START (0)
#define PMIC_IDLE_PLACE_CTRL13_idle_ldo28_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo30_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_PLACE_CTRL14_UNION;
#endif
#define PMIC_IDLE_PLACE_CTRL14_idle_ldo30_sel_START (0)
#define PMIC_IDLE_PLACE_CTRL14_idle_ldo30_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo37_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_PLACE_CTRL15_UNION;
#endif
#define PMIC_IDLE_PLACE_CTRL15_idle_ldo37_sel_START (0)
#define PMIC_IDLE_PLACE_CTRL15_idle_ldo37_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo38_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_PLACE_CTRL16_UNION;
#endif
#define PMIC_IDLE_PLACE_CTRL16_idle_ldo38_sel_START (0)
#define PMIC_IDLE_PLACE_CTRL16_idle_ldo38_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo39_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_PLACE_CTRL17_UNION;
#endif
#define PMIC_IDLE_PLACE_CTRL17_idle_ldo39_sel_START (0)
#define PMIC_IDLE_PLACE_CTRL17_idle_ldo39_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo41_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_PLACE_CTRL18_UNION;
#endif
#define PMIC_IDLE_PLACE_CTRL18_idle_ldo41_sel_START (0)
#define PMIC_IDLE_PLACE_CTRL18_idle_ldo41_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo44_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_PLACE_CTRL19_UNION;
#endif
#define PMIC_IDLE_PLACE_CTRL19_idle_ldo44_sel_START (0)
#define PMIC_IDLE_PLACE_CTRL19_idle_ldo44_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_ldo50_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_PLACE_CTRL20_UNION;
#endif
#define PMIC_IDLE_PLACE_CTRL20_idle_ldo50_sel_START (0)
#define PMIC_IDLE_PLACE_CTRL20_idle_ldo50_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char idle_pmuh_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_IDLE_PLACE_CTRL21_UNION;
#endif
#define PMIC_IDLE_PLACE_CTRL21_idle_pmuh_sel_START (0)
#define PMIC_IDLE_PLACE_CTRL21_idle_pmuh_sel_END (4)
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
        unsigned char adcr_codec_clk_en_mask : 1;
        unsigned char adcr_nfc_clk_en_mask : 1;
        unsigned char adcr_wifi_clk_en_mask : 1;
        unsigned char adcr_sys_clk_en_mask : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_DCR_ECO_MASK_UNION;
#endif
#define PMIC_DCR_ECO_MASK_adcr_codec_clk_en_mask_START (0)
#define PMIC_DCR_ECO_MASK_adcr_codec_clk_en_mask_END (0)
#define PMIC_DCR_ECO_MASK_adcr_nfc_clk_en_mask_START (1)
#define PMIC_DCR_ECO_MASK_adcr_nfc_clk_en_mask_END (1)
#define PMIC_DCR_ECO_MASK_adcr_wifi_clk_en_mask_START (2)
#define PMIC_DCR_ECO_MASK_adcr_wifi_clk_en_mask_END (2)
#define PMIC_DCR_ECO_MASK_adcr_sys_clk_en_mask_START (3)
#define PMIC_DCR_ECO_MASK_adcr_sys_clk_en_mask_END (3)
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
} PMIC_BUCK_OCP_MODE_CTRL0_UNION;
#endif
#define PMIC_BUCK_OCP_MODE_CTRL0_buck3_ocp_auto_stop_START (0)
#define PMIC_BUCK_OCP_MODE_CTRL0_buck3_ocp_auto_stop_END (1)
#define PMIC_BUCK_OCP_MODE_CTRL0_buck2_ocp_auto_stop_START (2)
#define PMIC_BUCK_OCP_MODE_CTRL0_buck2_ocp_auto_stop_END (3)
#define PMIC_BUCK_OCP_MODE_CTRL0_buck1_ocp_auto_stop_START (4)
#define PMIC_BUCK_OCP_MODE_CTRL0_buck1_ocp_auto_stop_END (5)
#define PMIC_BUCK_OCP_MODE_CTRL0_buck0_ocp_auto_stop_START (6)
#define PMIC_BUCK_OCP_MODE_CTRL0_buck0_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck41_ocp_auto_stop : 2;
        unsigned char buck40_ocp_auto_stop : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_BUCK_OCP_MODE_CTRL1_UNION;
#endif
#define PMIC_BUCK_OCP_MODE_CTRL1_buck41_ocp_auto_stop_START (0)
#define PMIC_BUCK_OCP_MODE_CTRL1_buck41_ocp_auto_stop_END (1)
#define PMIC_BUCK_OCP_MODE_CTRL1_buck40_ocp_auto_stop_START (2)
#define PMIC_BUCK_OCP_MODE_CTRL1_buck40_ocp_auto_stop_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_ocp_auto_stop : 2;
        unsigned char buck6_ocp_auto_stop : 2;
        unsigned char buck5_ocp_auto_stop : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK_OCP_MODE_CTRL2_UNION;
#endif
#define PMIC_BUCK_OCP_MODE_CTRL2_buck7_ocp_auto_stop_START (0)
#define PMIC_BUCK_OCP_MODE_CTRL2_buck7_ocp_auto_stop_END (1)
#define PMIC_BUCK_OCP_MODE_CTRL2_buck6_ocp_auto_stop_START (2)
#define PMIC_BUCK_OCP_MODE_CTRL2_buck6_ocp_auto_stop_END (3)
#define PMIC_BUCK_OCP_MODE_CTRL2_buck5_ocp_auto_stop_START (4)
#define PMIC_BUCK_OCP_MODE_CTRL2_buck5_ocp_auto_stop_END (5)
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
} PMIC_LDO_OCP_MODE_CTRL0_UNION;
#endif
#define PMIC_LDO_OCP_MODE_CTRL0_ldo3_ocp_auto_stop_START (0)
#define PMIC_LDO_OCP_MODE_CTRL0_ldo3_ocp_auto_stop_END (1)
#define PMIC_LDO_OCP_MODE_CTRL0_ldo2_ocp_auto_stop_START (2)
#define PMIC_LDO_OCP_MODE_CTRL0_ldo2_ocp_auto_stop_END (3)
#define PMIC_LDO_OCP_MODE_CTRL0_ldo1_ocp_auto_stop_START (4)
#define PMIC_LDO_OCP_MODE_CTRL0_ldo1_ocp_auto_stop_END (5)
#define PMIC_LDO_OCP_MODE_CTRL0_ldo0_ocp_auto_stop_START (6)
#define PMIC_LDO_OCP_MODE_CTRL0_ldo0_ocp_auto_stop_END (7)
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
} PMIC_LDO_OCP_MODE_CTRL1_UNION;
#endif
#define PMIC_LDO_OCP_MODE_CTRL1_ldo8_ocp_auto_stop_START (0)
#define PMIC_LDO_OCP_MODE_CTRL1_ldo8_ocp_auto_stop_END (1)
#define PMIC_LDO_OCP_MODE_CTRL1_ldo6_ocp_auto_stop_START (2)
#define PMIC_LDO_OCP_MODE_CTRL1_ldo6_ocp_auto_stop_END (3)
#define PMIC_LDO_OCP_MODE_CTRL1_ldo5_ocp_auto_stop_START (4)
#define PMIC_LDO_OCP_MODE_CTRL1_ldo5_ocp_auto_stop_END (5)
#define PMIC_LDO_OCP_MODE_CTRL1_ldo4_ocp_auto_stop_START (6)
#define PMIC_LDO_OCP_MODE_CTRL1_ldo4_ocp_auto_stop_END (7)
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
} PMIC_LDO_OCP_MODE_CTRL2_UNION;
#endif
#define PMIC_LDO_OCP_MODE_CTRL2_ldo13_ocp_auto_stop_START (0)
#define PMIC_LDO_OCP_MODE_CTRL2_ldo13_ocp_auto_stop_END (1)
#define PMIC_LDO_OCP_MODE_CTRL2_ldo12_ocp_auto_stop_START (2)
#define PMIC_LDO_OCP_MODE_CTRL2_ldo12_ocp_auto_stop_END (3)
#define PMIC_LDO_OCP_MODE_CTRL2_ldo11_ocp_auto_stop_START (4)
#define PMIC_LDO_OCP_MODE_CTRL2_ldo11_ocp_auto_stop_END (5)
#define PMIC_LDO_OCP_MODE_CTRL2_ldo9_ocp_auto_stop_START (6)
#define PMIC_LDO_OCP_MODE_CTRL2_ldo9_ocp_auto_stop_END (7)
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
} PMIC_LDO_OCP_MODE_CTRL3_UNION;
#endif
#define PMIC_LDO_OCP_MODE_CTRL3_ldo17_ocp_auto_stop_START (0)
#define PMIC_LDO_OCP_MODE_CTRL3_ldo17_ocp_auto_stop_END (1)
#define PMIC_LDO_OCP_MODE_CTRL3_ldo16_ocp_auto_stop_START (2)
#define PMIC_LDO_OCP_MODE_CTRL3_ldo16_ocp_auto_stop_END (3)
#define PMIC_LDO_OCP_MODE_CTRL3_ldo15_ocp_auto_stop_START (4)
#define PMIC_LDO_OCP_MODE_CTRL3_ldo15_ocp_auto_stop_END (5)
#define PMIC_LDO_OCP_MODE_CTRL3_ldo14_ocp_auto_stop_START (6)
#define PMIC_LDO_OCP_MODE_CTRL3_ldo14_ocp_auto_stop_END (7)
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
} PMIC_LDO_OCP_MODE_CTRL4_UNION;
#endif
#define PMIC_LDO_OCP_MODE_CTRL4_ldo21_ocp_auto_stop_START (0)
#define PMIC_LDO_OCP_MODE_CTRL4_ldo21_ocp_auto_stop_END (1)
#define PMIC_LDO_OCP_MODE_CTRL4_ldo20_ocp_auto_stop_START (2)
#define PMIC_LDO_OCP_MODE_CTRL4_ldo20_ocp_auto_stop_END (3)
#define PMIC_LDO_OCP_MODE_CTRL4_ldo19_ocp_auto_stop_START (4)
#define PMIC_LDO_OCP_MODE_CTRL4_ldo19_ocp_auto_stop_END (5)
#define PMIC_LDO_OCP_MODE_CTRL4_ldo18_ocp_auto_stop_START (6)
#define PMIC_LDO_OCP_MODE_CTRL4_ldo18_ocp_auto_stop_END (7)
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
} PMIC_LDO_OCP_MODE_CTRL5_UNION;
#endif
#define PMIC_LDO_OCP_MODE_CTRL5_ldo25_ocp_auto_stop_START (0)
#define PMIC_LDO_OCP_MODE_CTRL5_ldo25_ocp_auto_stop_END (1)
#define PMIC_LDO_OCP_MODE_CTRL5_ldo24_ocp_auto_stop_START (2)
#define PMIC_LDO_OCP_MODE_CTRL5_ldo24_ocp_auto_stop_END (3)
#define PMIC_LDO_OCP_MODE_CTRL5_ldo23_ocp_auto_stop_START (4)
#define PMIC_LDO_OCP_MODE_CTRL5_ldo23_ocp_auto_stop_END (5)
#define PMIC_LDO_OCP_MODE_CTRL5_ldo22_ocp_auto_stop_START (6)
#define PMIC_LDO_OCP_MODE_CTRL5_ldo22_ocp_auto_stop_END (7)
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
} PMIC_LDO_OCP_MODE_CTRL6_UNION;
#endif
#define PMIC_LDO_OCP_MODE_CTRL6_ldo29_ocp_auto_stop_START (0)
#define PMIC_LDO_OCP_MODE_CTRL6_ldo29_ocp_auto_stop_END (1)
#define PMIC_LDO_OCP_MODE_CTRL6_ldo28_ocp_auto_stop_START (2)
#define PMIC_LDO_OCP_MODE_CTRL6_ldo28_ocp_auto_stop_END (3)
#define PMIC_LDO_OCP_MODE_CTRL6_ldo27_ocp_auto_stop_START (4)
#define PMIC_LDO_OCP_MODE_CTRL6_ldo27_ocp_auto_stop_END (5)
#define PMIC_LDO_OCP_MODE_CTRL6_ldo26_ocp_auto_stop_START (6)
#define PMIC_LDO_OCP_MODE_CTRL6_ldo26_ocp_auto_stop_END (7)
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
} PMIC_LDO_OCP_MODE_CTRL7_UNION;
#endif
#define PMIC_LDO_OCP_MODE_CTRL7_ldo33_ocp_auto_stop_START (0)
#define PMIC_LDO_OCP_MODE_CTRL7_ldo33_ocp_auto_stop_END (1)
#define PMIC_LDO_OCP_MODE_CTRL7_ldo32_ocp_auto_stop_START (2)
#define PMIC_LDO_OCP_MODE_CTRL7_ldo32_ocp_auto_stop_END (3)
#define PMIC_LDO_OCP_MODE_CTRL7_ldo31_ocp_auto_stop_START (4)
#define PMIC_LDO_OCP_MODE_CTRL7_ldo31_ocp_auto_stop_END (5)
#define PMIC_LDO_OCP_MODE_CTRL7_ldo30_ocp_auto_stop_START (6)
#define PMIC_LDO_OCP_MODE_CTRL7_ldo30_ocp_auto_stop_END (7)
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
} PMIC_LDO_OCP_MODE_CTRL8_UNION;
#endif
#define PMIC_LDO_OCP_MODE_CTRL8_ldo39_ocp_auto_stop_START (0)
#define PMIC_LDO_OCP_MODE_CTRL8_ldo39_ocp_auto_stop_END (1)
#define PMIC_LDO_OCP_MODE_CTRL8_ldo38_ocp_auto_stop_START (2)
#define PMIC_LDO_OCP_MODE_CTRL8_ldo38_ocp_auto_stop_END (3)
#define PMIC_LDO_OCP_MODE_CTRL8_ldo37_ocp_auto_stop_START (4)
#define PMIC_LDO_OCP_MODE_CTRL8_ldo37_ocp_auto_stop_END (5)
#define PMIC_LDO_OCP_MODE_CTRL8_ldo34_ocp_auto_stop_START (6)
#define PMIC_LDO_OCP_MODE_CTRL8_ldo34_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo43_ocp_auto_stop : 2;
        unsigned char ldo42_ocp_auto_stop : 2;
        unsigned char ldo41_ocp_auto_stop : 2;
        unsigned char ldo40_ocp_auto_stop : 2;
    } reg;
} PMIC_LDO_OCP_MODE_CTRL9_UNION;
#endif
#define PMIC_LDO_OCP_MODE_CTRL9_ldo43_ocp_auto_stop_START (0)
#define PMIC_LDO_OCP_MODE_CTRL9_ldo43_ocp_auto_stop_END (1)
#define PMIC_LDO_OCP_MODE_CTRL9_ldo42_ocp_auto_stop_START (2)
#define PMIC_LDO_OCP_MODE_CTRL9_ldo42_ocp_auto_stop_END (3)
#define PMIC_LDO_OCP_MODE_CTRL9_ldo41_ocp_auto_stop_START (4)
#define PMIC_LDO_OCP_MODE_CTRL9_ldo41_ocp_auto_stop_END (5)
#define PMIC_LDO_OCP_MODE_CTRL9_ldo40_ocp_auto_stop_START (6)
#define PMIC_LDO_OCP_MODE_CTRL9_ldo40_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo47_ocp_auto_stop : 2;
        unsigned char ldo46_ocp_auto_stop : 2;
        unsigned char ldo45_ocp_auto_stop : 2;
        unsigned char ldo44_ocp_auto_stop : 2;
    } reg;
} PMIC_LDO_OCP_MODE_CTRL10_UNION;
#endif
#define PMIC_LDO_OCP_MODE_CTRL10_ldo47_ocp_auto_stop_START (0)
#define PMIC_LDO_OCP_MODE_CTRL10_ldo47_ocp_auto_stop_END (1)
#define PMIC_LDO_OCP_MODE_CTRL10_ldo46_ocp_auto_stop_START (2)
#define PMIC_LDO_OCP_MODE_CTRL10_ldo46_ocp_auto_stop_END (3)
#define PMIC_LDO_OCP_MODE_CTRL10_ldo45_ocp_auto_stop_START (4)
#define PMIC_LDO_OCP_MODE_CTRL10_ldo45_ocp_auto_stop_END (5)
#define PMIC_LDO_OCP_MODE_CTRL10_ldo44_ocp_auto_stop_START (6)
#define PMIC_LDO_OCP_MODE_CTRL10_ldo44_ocp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo50_ocp_auto_stop : 2;
        unsigned char ldo49_ocp_auto_stop : 2;
        unsigned char ldo48_ocp_auto_stop : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO_OCP_MODE_CTRL11_UNION;
#endif
#define PMIC_LDO_OCP_MODE_CTRL11_ldo50_ocp_auto_stop_START (0)
#define PMIC_LDO_OCP_MODE_CTRL11_ldo50_ocp_auto_stop_END (1)
#define PMIC_LDO_OCP_MODE_CTRL11_ldo49_ocp_auto_stop_START (2)
#define PMIC_LDO_OCP_MODE_CTRL11_ldo49_ocp_auto_stop_END (3)
#define PMIC_LDO_OCP_MODE_CTRL11_ldo48_ocp_auto_stop_START (4)
#define PMIC_LDO_OCP_MODE_CTRL11_ldo48_ocp_auto_stop_END (5)
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
} PMIC_BUCK_SCP_MODE_CTRL0_UNION;
#endif
#define PMIC_BUCK_SCP_MODE_CTRL0_buck3_scp_auto_stop_START (0)
#define PMIC_BUCK_SCP_MODE_CTRL0_buck3_scp_auto_stop_END (1)
#define PMIC_BUCK_SCP_MODE_CTRL0_buck2_scp_auto_stop_START (2)
#define PMIC_BUCK_SCP_MODE_CTRL0_buck2_scp_auto_stop_END (3)
#define PMIC_BUCK_SCP_MODE_CTRL0_buck1_scp_auto_stop_START (4)
#define PMIC_BUCK_SCP_MODE_CTRL0_buck1_scp_auto_stop_END (5)
#define PMIC_BUCK_SCP_MODE_CTRL0_buck0_scp_auto_stop_START (6)
#define PMIC_BUCK_SCP_MODE_CTRL0_buck0_scp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_scp_auto_stop : 2;
        unsigned char buck6_scp_auto_stop : 2;
        unsigned char buck5_scp_auto_stop : 2;
        unsigned char buck40_scp_auto_stop : 2;
    } reg;
} PMIC_BUCK_SCP_MODE_CTRL1_UNION;
#endif
#define PMIC_BUCK_SCP_MODE_CTRL1_buck7_scp_auto_stop_START (0)
#define PMIC_BUCK_SCP_MODE_CTRL1_buck7_scp_auto_stop_END (1)
#define PMIC_BUCK_SCP_MODE_CTRL1_buck6_scp_auto_stop_START (2)
#define PMIC_BUCK_SCP_MODE_CTRL1_buck6_scp_auto_stop_END (3)
#define PMIC_BUCK_SCP_MODE_CTRL1_buck5_scp_auto_stop_START (4)
#define PMIC_BUCK_SCP_MODE_CTRL1_buck5_scp_auto_stop_END (5)
#define PMIC_BUCK_SCP_MODE_CTRL1_buck40_scp_auto_stop_START (6)
#define PMIC_BUCK_SCP_MODE_CTRL1_buck40_scp_auto_stop_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo_buff_scp_auto_stop : 2;
        unsigned char classd_ocp_auto_stop : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO_BUFF_SCP_CTRL_UNION;
#endif
#define PMIC_LDO_BUFF_SCP_CTRL_ldo_buff_scp_auto_stop_START (0)
#define PMIC_LDO_BUFF_SCP_CTRL_ldo_buff_scp_auto_stop_END (1)
#define PMIC_LDO_BUFF_SCP_CTRL_classd_ocp_auto_stop_START (2)
#define PMIC_LDO_BUFF_SCP_CTRL_classd_ocp_auto_stop_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck40_ovp_auto_stop : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_BUCK_OVP_CTRL_UNION;
#endif
#define PMIC_BUCK_OVP_CTRL_buck40_ovp_auto_stop_START (0)
#define PMIC_BUCK_OVP_CTRL_buck40_ovp_auto_stop_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck40_ocp_sel : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_BUCK_OCP_ABN_CTRL0_UNION;
#endif
#define PMIC_BUCK_OCP_ABN_CTRL0_buck40_ocp_sel_START (0)
#define PMIC_BUCK_OCP_ABN_CTRL0_buck40_ocp_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char en_ldo31_ocp_deb : 1;
        unsigned char en_classd_ocp_deb : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_OCP_SCP_ONOFF0_UNION;
#endif
#define PMIC_OCP_SCP_ONOFF0_en_ldo31_ocp_deb_START (0)
#define PMIC_OCP_SCP_ONOFF0_en_ldo31_ocp_deb_END (0)
#define PMIC_OCP_SCP_ONOFF0_en_classd_ocp_deb_START (1)
#define PMIC_OCP_SCP_ONOFF0_en_classd_ocp_deb_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char en_vin_ldoh_deb : 1;
        unsigned char en_ldo_ocp_deb : 1;
        unsigned char en_buck_scp_deb : 1;
        unsigned char en_buck_ocp_deb : 1;
        unsigned char en_buck_ovp_deb : 1;
        unsigned char en_buck_cur_det_deb : 1;
        unsigned char en_ldo_buff_scp_deb : 1;
        unsigned char en_acr_ocp_deb : 1;
    } reg;
} PMIC_OCP_SCP_ONOFF1_UNION;
#endif
#define PMIC_OCP_SCP_ONOFF1_en_vin_ldoh_deb_START (0)
#define PMIC_OCP_SCP_ONOFF1_en_vin_ldoh_deb_END (0)
#define PMIC_OCP_SCP_ONOFF1_en_ldo_ocp_deb_START (1)
#define PMIC_OCP_SCP_ONOFF1_en_ldo_ocp_deb_END (1)
#define PMIC_OCP_SCP_ONOFF1_en_buck_scp_deb_START (2)
#define PMIC_OCP_SCP_ONOFF1_en_buck_scp_deb_END (2)
#define PMIC_OCP_SCP_ONOFF1_en_buck_ocp_deb_START (3)
#define PMIC_OCP_SCP_ONOFF1_en_buck_ocp_deb_END (3)
#define PMIC_OCP_SCP_ONOFF1_en_buck_ovp_deb_START (4)
#define PMIC_OCP_SCP_ONOFF1_en_buck_ovp_deb_END (4)
#define PMIC_OCP_SCP_ONOFF1_en_buck_cur_det_deb_START (5)
#define PMIC_OCP_SCP_ONOFF1_en_buck_cur_det_deb_END (5)
#define PMIC_OCP_SCP_ONOFF1_en_ldo_buff_scp_deb_START (6)
#define PMIC_OCP_SCP_ONOFF1_en_ldo_buff_scp_deb_END (6)
#define PMIC_OCP_SCP_ONOFF1_en_acr_ocp_deb_START (7)
#define PMIC_OCP_SCP_ONOFF1_en_acr_ocp_deb_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo_ocp_deb_sel : 2;
        unsigned char buck_scp_deb_sel : 2;
        unsigned char buck_ocp_deb_sel : 2;
        unsigned char buck_ovp_deb_sel : 2;
    } reg;
} PMIC_BUCK_LDO_ABN_DEB_CTRL0_UNION;
#endif
#define PMIC_BUCK_LDO_ABN_DEB_CTRL0_ldo_ocp_deb_sel_START (0)
#define PMIC_BUCK_LDO_ABN_DEB_CTRL0_ldo_ocp_deb_sel_END (1)
#define PMIC_BUCK_LDO_ABN_DEB_CTRL0_buck_scp_deb_sel_START (2)
#define PMIC_BUCK_LDO_ABN_DEB_CTRL0_buck_scp_deb_sel_END (3)
#define PMIC_BUCK_LDO_ABN_DEB_CTRL0_buck_ocp_deb_sel_START (4)
#define PMIC_BUCK_LDO_ABN_DEB_CTRL0_buck_ocp_deb_sel_END (5)
#define PMIC_BUCK_LDO_ABN_DEB_CTRL0_buck_ovp_deb_sel_START (6)
#define PMIC_BUCK_LDO_ABN_DEB_CTRL0_buck_ovp_deb_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vin_ldoh_deb_sel : 2;
        unsigned char ldo_buff_scp_deb_sel : 2;
        unsigned char acr_ocp_deb_sel : 2;
        unsigned char buck_cur_det_deb_sel : 2;
    } reg;
} PMIC_BUCK_LDO_ABN_DEB_CTRL1_UNION;
#endif
#define PMIC_BUCK_LDO_ABN_DEB_CTRL1_vin_ldoh_deb_sel_START (0)
#define PMIC_BUCK_LDO_ABN_DEB_CTRL1_vin_ldoh_deb_sel_END (1)
#define PMIC_BUCK_LDO_ABN_DEB_CTRL1_ldo_buff_scp_deb_sel_START (2)
#define PMIC_BUCK_LDO_ABN_DEB_CTRL1_ldo_buff_scp_deb_sel_END (3)
#define PMIC_BUCK_LDO_ABN_DEB_CTRL1_acr_ocp_deb_sel_START (4)
#define PMIC_BUCK_LDO_ABN_DEB_CTRL1_acr_ocp_deb_sel_END (5)
#define PMIC_BUCK_LDO_ABN_DEB_CTRL1_buck_cur_det_deb_sel_START (6)
#define PMIC_BUCK_LDO_ABN_DEB_CTRL1_buck_cur_det_deb_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo31_ocp_deb_sel : 2;
        unsigned char classd_ocp_deb_sel : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_BUCK_LDO_ABN_DEB_CTRL2_UNION;
#endif
#define PMIC_BUCK_LDO_ABN_DEB_CTRL2_ldo31_ocp_deb_sel_START (0)
#define PMIC_BUCK_LDO_ABN_DEB_CTRL2_ldo31_ocp_deb_sel_END (1)
#define PMIC_BUCK_LDO_ABN_DEB_CTRL2_classd_ocp_deb_sel_START (2)
#define PMIC_BUCK_LDO_ABN_DEB_CTRL2_classd_ocp_deb_sel_END (3)
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
        unsigned char buck0_dly_rd_on_cfg : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK0_RAMPDOWN_STATE_UNION;
#endif
#define PMIC_BUCK0_RAMPDOWN_STATE_buck0_dly_rd_pd_off_cfg_START (0)
#define PMIC_BUCK0_RAMPDOWN_STATE_buck0_dly_rd_pd_off_cfg_END (2)
#define PMIC_BUCK0_RAMPDOWN_STATE_buck0_dly_rd_on_cfg_START (3)
#define PMIC_BUCK0_RAMPDOWN_STATE_buck0_dly_rd_on_cfg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_rampdown_chg_rate : 2;
        unsigned char reserved_0 : 2;
        unsigned char buck0_rampup_chg_rate : 2;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_BUCK0_RAMP_CHG_RATE_UNION;
#endif
#define PMIC_BUCK0_RAMP_CHG_RATE_buck0_rampdown_chg_rate_START (0)
#define PMIC_BUCK0_RAMP_CHG_RATE_buck0_rampdown_chg_rate_END (1)
#define PMIC_BUCK0_RAMP_CHG_RATE_buck0_rampup_chg_rate_START (4)
#define PMIC_BUCK0_RAMP_CHG_RATE_buck0_rampup_chg_rate_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_rampdown_chg_vset : 3;
        unsigned char reserved_0 : 1;
        unsigned char buck0_rampup_chg_vset : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_BUCK0_RAMP_CHG_VSET_UNION;
#endif
#define PMIC_BUCK0_RAMP_CHG_VSET_buck0_rampdown_chg_vset_START (0)
#define PMIC_BUCK0_RAMP_CHG_VSET_buck0_rampdown_chg_vset_END (2)
#define PMIC_BUCK0_RAMP_CHG_VSET_buck0_rampup_chg_vset_START (4)
#define PMIC_BUCK0_RAMP_CHG_VSET_buck0_rampup_chg_vset_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck0_rd_nochg_rate_sel : 1;
        unsigned char reserved_0 : 3;
        unsigned char buck0_ru_nochg_rate_sel : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_BUCK0_CHG_RATE_SEL_UNION;
#endif
#define PMIC_BUCK0_CHG_RATE_SEL_buck0_rd_nochg_rate_sel_START (0)
#define PMIC_BUCK0_CHG_RATE_SEL_buck0_rd_nochg_rate_sel_END (0)
#define PMIC_BUCK0_CHG_RATE_SEL_buck0_ru_nochg_rate_sel_START (4)
#define PMIC_BUCK0_CHG_RATE_SEL_buck0_ru_nochg_rate_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck40_rampdown_ctrl : 3;
        unsigned char buck40_rampup_ctrl : 3;
        unsigned char buck40_ramp_en_cfg : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_RAMP_BUCK40_CTRL0_UNION;
#endif
#define PMIC_RAMP_BUCK40_CTRL0_buck40_rampdown_ctrl_START (0)
#define PMIC_RAMP_BUCK40_CTRL0_buck40_rampdown_ctrl_END (2)
#define PMIC_RAMP_BUCK40_CTRL0_buck40_rampup_ctrl_START (3)
#define PMIC_RAMP_BUCK40_CTRL0_buck40_rampup_ctrl_END (5)
#define PMIC_RAMP_BUCK40_CTRL0_buck40_ramp_en_cfg_START (6)
#define PMIC_RAMP_BUCK40_CTRL0_buck40_ramp_en_cfg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck40_pull_down_off_cfg : 3;
        unsigned char buck40_pull_down_on_cfg : 3;
        unsigned char buck40_pull_down_cfg : 1;
        unsigned char buck40_pull_down_mode : 1;
    } reg;
} PMIC_RAMP_BUCK40_CTRL1_UNION;
#endif
#define PMIC_RAMP_BUCK40_CTRL1_buck40_pull_down_off_cfg_START (0)
#define PMIC_RAMP_BUCK40_CTRL1_buck40_pull_down_off_cfg_END (2)
#define PMIC_RAMP_BUCK40_CTRL1_buck40_pull_down_on_cfg_START (3)
#define PMIC_RAMP_BUCK40_CTRL1_buck40_pull_down_on_cfg_END (5)
#define PMIC_RAMP_BUCK40_CTRL1_buck40_pull_down_cfg_START (6)
#define PMIC_RAMP_BUCK40_CTRL1_buck40_pull_down_cfg_END (6)
#define PMIC_RAMP_BUCK40_CTRL1_buck40_pull_down_mode_START (7)
#define PMIC_RAMP_BUCK40_CTRL1_buck40_pull_down_mode_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck40_dly_ru_off_cfg : 3;
        unsigned char buck40_dly_ru_on_cfg : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK40_RAMPUP_STATE_UNION;
#endif
#define PMIC_BUCK40_RAMPUP_STATE_buck40_dly_ru_off_cfg_START (0)
#define PMIC_BUCK40_RAMPUP_STATE_buck40_dly_ru_off_cfg_END (2)
#define PMIC_BUCK40_RAMPUP_STATE_buck40_dly_ru_on_cfg_START (3)
#define PMIC_BUCK40_RAMPUP_STATE_buck40_dly_ru_on_cfg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck40_dly_rd_pd_off_cfg : 3;
        unsigned char buck40_dly_rd_on_cfg : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK40_RAMPDOWN_STATE_UNION;
#endif
#define PMIC_BUCK40_RAMPDOWN_STATE_buck40_dly_rd_pd_off_cfg_START (0)
#define PMIC_BUCK40_RAMPDOWN_STATE_buck40_dly_rd_pd_off_cfg_END (2)
#define PMIC_BUCK40_RAMPDOWN_STATE_buck40_dly_rd_on_cfg_START (3)
#define PMIC_BUCK40_RAMPDOWN_STATE_buck40_dly_rd_on_cfg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck40_rampdown_chg_rate : 2;
        unsigned char reserved_0 : 2;
        unsigned char buck40_rampup_chg_rate : 2;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_BUCK40_RAMP_CHG_RATE_UNION;
#endif
#define PMIC_BUCK40_RAMP_CHG_RATE_buck40_rampdown_chg_rate_START (0)
#define PMIC_BUCK40_RAMP_CHG_RATE_buck40_rampdown_chg_rate_END (1)
#define PMIC_BUCK40_RAMP_CHG_RATE_buck40_rampup_chg_rate_START (4)
#define PMIC_BUCK40_RAMP_CHG_RATE_buck40_rampup_chg_rate_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck40_rampdown_chg_vset : 3;
        unsigned char reserved_0 : 1;
        unsigned char buck40_rampup_chg_vset : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_BUCK40_RAMP_CHG_VSET_UNION;
#endif
#define PMIC_BUCK40_RAMP_CHG_VSET_buck40_rampdown_chg_vset_START (0)
#define PMIC_BUCK40_RAMP_CHG_VSET_buck40_rampdown_chg_vset_END (2)
#define PMIC_BUCK40_RAMP_CHG_VSET_buck40_rampup_chg_vset_START (4)
#define PMIC_BUCK40_RAMP_CHG_VSET_buck40_rampup_chg_vset_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck40_rd_nochg_rate_sel : 1;
        unsigned char reserved_0 : 3;
        unsigned char buck40_ru_nochg_rate_sel : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_BUCK40_CHG_RATE_SEL_UNION;
#endif
#define PMIC_BUCK40_CHG_RATE_SEL_buck40_rd_nochg_rate_sel_START (0)
#define PMIC_BUCK40_CHG_RATE_SEL_buck40_rd_nochg_rate_sel_END (0)
#define PMIC_BUCK40_CHG_RATE_SEL_buck40_ru_nochg_rate_sel_START (4)
#define PMIC_BUCK40_CHG_RATE_SEL_buck40_ru_nochg_rate_sel_END (4)
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
        unsigned char buck5_dly_rd_on_cfg : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK5_RAMPDOWN_STATE_UNION;
#endif
#define PMIC_BUCK5_RAMPDOWN_STATE_buck5_dly_rd_pd_off_cfg_START (0)
#define PMIC_BUCK5_RAMPDOWN_STATE_buck5_dly_rd_pd_off_cfg_END (2)
#define PMIC_BUCK5_RAMPDOWN_STATE_buck5_dly_rd_on_cfg_START (3)
#define PMIC_BUCK5_RAMPDOWN_STATE_buck5_dly_rd_on_cfg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_rampdown_chg_rate : 2;
        unsigned char reserved_0 : 2;
        unsigned char buck5_rampup_chg_rate : 2;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_BUCK5_RAMP_CHG_RATE_UNION;
#endif
#define PMIC_BUCK5_RAMP_CHG_RATE_buck5_rampdown_chg_rate_START (0)
#define PMIC_BUCK5_RAMP_CHG_RATE_buck5_rampdown_chg_rate_END (1)
#define PMIC_BUCK5_RAMP_CHG_RATE_buck5_rampup_chg_rate_START (4)
#define PMIC_BUCK5_RAMP_CHG_RATE_buck5_rampup_chg_rate_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_rampdown_chg_vset : 3;
        unsigned char reserved_0 : 1;
        unsigned char buck5_rampup_chg_vset : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_BUCK5_RAMP_CHG_VSET_UNION;
#endif
#define PMIC_BUCK5_RAMP_CHG_VSET_buck5_rampdown_chg_vset_START (0)
#define PMIC_BUCK5_RAMP_CHG_VSET_buck5_rampdown_chg_vset_END (2)
#define PMIC_BUCK5_RAMP_CHG_VSET_buck5_rampup_chg_vset_START (4)
#define PMIC_BUCK5_RAMP_CHG_VSET_buck5_rampup_chg_vset_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck5_rd_nochg_rate_sel : 1;
        unsigned char reserved_0 : 3;
        unsigned char buck5_ru_nochg_rate_sel : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_BUCK5_CHG_RATE_SEL_UNION;
#endif
#define PMIC_BUCK5_CHG_RATE_SEL_buck5_rd_nochg_rate_sel_START (0)
#define PMIC_BUCK5_CHG_RATE_SEL_buck5_rd_nochg_rate_sel_END (0)
#define PMIC_BUCK5_CHG_RATE_SEL_buck5_ru_nochg_rate_sel_START (4)
#define PMIC_BUCK5_CHG_RATE_SEL_buck5_ru_nochg_rate_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_rampdown_ctrl : 3;
        unsigned char buck7_rampup_ctrl : 3;
        unsigned char buck7_ramp_en_cfg : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_RAMP_BUCK7_CTRL0_UNION;
#endif
#define PMIC_RAMP_BUCK7_CTRL0_buck7_rampdown_ctrl_START (0)
#define PMIC_RAMP_BUCK7_CTRL0_buck7_rampdown_ctrl_END (2)
#define PMIC_RAMP_BUCK7_CTRL0_buck7_rampup_ctrl_START (3)
#define PMIC_RAMP_BUCK7_CTRL0_buck7_rampup_ctrl_END (5)
#define PMIC_RAMP_BUCK7_CTRL0_buck7_ramp_en_cfg_START (6)
#define PMIC_RAMP_BUCK7_CTRL0_buck7_ramp_en_cfg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_pull_down_off_cfg : 3;
        unsigned char buck7_pull_down_on_cfg : 3;
        unsigned char buck7_pull_down_cfg : 1;
        unsigned char buck7_pull_down_mode : 1;
    } reg;
} PMIC_RAMP_BUCK7_CTRL1_UNION;
#endif
#define PMIC_RAMP_BUCK7_CTRL1_buck7_pull_down_off_cfg_START (0)
#define PMIC_RAMP_BUCK7_CTRL1_buck7_pull_down_off_cfg_END (2)
#define PMIC_RAMP_BUCK7_CTRL1_buck7_pull_down_on_cfg_START (3)
#define PMIC_RAMP_BUCK7_CTRL1_buck7_pull_down_on_cfg_END (5)
#define PMIC_RAMP_BUCK7_CTRL1_buck7_pull_down_cfg_START (6)
#define PMIC_RAMP_BUCK7_CTRL1_buck7_pull_down_cfg_END (6)
#define PMIC_RAMP_BUCK7_CTRL1_buck7_pull_down_mode_START (7)
#define PMIC_RAMP_BUCK7_CTRL1_buck7_pull_down_mode_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_dly_ru_off_cfg : 3;
        unsigned char buck7_dly_ru_on_cfg : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK7_RAMPUP_STATE_UNION;
#endif
#define PMIC_BUCK7_RAMPUP_STATE_buck7_dly_ru_off_cfg_START (0)
#define PMIC_BUCK7_RAMPUP_STATE_buck7_dly_ru_off_cfg_END (2)
#define PMIC_BUCK7_RAMPUP_STATE_buck7_dly_ru_on_cfg_START (3)
#define PMIC_BUCK7_RAMPUP_STATE_buck7_dly_ru_on_cfg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_dly_rd_pd_off_cfg : 3;
        unsigned char buck7_dly_rd_on_cfg : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK7_RAMPDOWN_STATE_UNION;
#endif
#define PMIC_BUCK7_RAMPDOWN_STATE_buck7_dly_rd_pd_off_cfg_START (0)
#define PMIC_BUCK7_RAMPDOWN_STATE_buck7_dly_rd_pd_off_cfg_END (2)
#define PMIC_BUCK7_RAMPDOWN_STATE_buck7_dly_rd_on_cfg_START (3)
#define PMIC_BUCK7_RAMPDOWN_STATE_buck7_dly_rd_on_cfg_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_rampdown_chg_rate : 2;
        unsigned char reserved_0 : 2;
        unsigned char buck7_rampup_chg_rate : 2;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_BUCK7_RAMP_CHG_RATE_UNION;
#endif
#define PMIC_BUCK7_RAMP_CHG_RATE_buck7_rampdown_chg_rate_START (0)
#define PMIC_BUCK7_RAMP_CHG_RATE_buck7_rampdown_chg_rate_END (1)
#define PMIC_BUCK7_RAMP_CHG_RATE_buck7_rampup_chg_rate_START (4)
#define PMIC_BUCK7_RAMP_CHG_RATE_buck7_rampup_chg_rate_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_rampdown_chg_vset : 3;
        unsigned char reserved_0 : 1;
        unsigned char buck7_rampup_chg_vset : 3;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_BUCK7_RAMP_CHG_VSET_UNION;
#endif
#define PMIC_BUCK7_RAMP_CHG_VSET_buck7_rampdown_chg_vset_START (0)
#define PMIC_BUCK7_RAMP_CHG_VSET_buck7_rampdown_chg_vset_END (2)
#define PMIC_BUCK7_RAMP_CHG_VSET_buck7_rampup_chg_vset_START (4)
#define PMIC_BUCK7_RAMP_CHG_VSET_buck7_rampup_chg_vset_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck7_rd_nochg_rate_sel : 1;
        unsigned char reserved_0 : 3;
        unsigned char buck7_ru_nochg_rate_sel : 1;
        unsigned char reserved_1 : 3;
    } reg;
} PMIC_BUCK7_CHG_RATE_SEL_UNION;
#endif
#define PMIC_BUCK7_CHG_RATE_SEL_buck7_rd_nochg_rate_sel_START (0)
#define PMIC_BUCK7_CHG_RATE_SEL_buck7_rd_nochg_rate_sel_END (0)
#define PMIC_BUCK7_CHG_RATE_SEL_buck7_ru_nochg_rate_sel_START (4)
#define PMIC_BUCK7_CHG_RATE_SEL_buck7_ru_nochg_rate_sel_END (4)
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
        unsigned char ldo6_rampdown_ctrl : 3;
        unsigned char ldo6_rampup_ctrl : 3;
        unsigned char ldo6_ramp_en_cfg : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_RAMP_LDO6_CTRL0_UNION;
#endif
#define PMIC_RAMP_LDO6_CTRL0_ldo6_rampdown_ctrl_START (0)
#define PMIC_RAMP_LDO6_CTRL0_ldo6_rampdown_ctrl_END (2)
#define PMIC_RAMP_LDO6_CTRL0_ldo6_rampup_ctrl_START (3)
#define PMIC_RAMP_LDO6_CTRL0_ldo6_rampup_ctrl_END (5)
#define PMIC_RAMP_LDO6_CTRL0_ldo6_ramp_en_cfg_START (6)
#define PMIC_RAMP_LDO6_CTRL0_ldo6_ramp_en_cfg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo6_pull_down_off_cfg : 3;
        unsigned char ldo6_pull_down_on_cfg : 3;
        unsigned char ldo6_pull_down_cfg : 1;
        unsigned char ldo6_pull_down_mode : 1;
    } reg;
} PMIC_RAMP_LDO6_CTRL1_UNION;
#endif
#define PMIC_RAMP_LDO6_CTRL1_ldo6_pull_down_off_cfg_START (0)
#define PMIC_RAMP_LDO6_CTRL1_ldo6_pull_down_off_cfg_END (2)
#define PMIC_RAMP_LDO6_CTRL1_ldo6_pull_down_on_cfg_START (3)
#define PMIC_RAMP_LDO6_CTRL1_ldo6_pull_down_on_cfg_END (5)
#define PMIC_RAMP_LDO6_CTRL1_ldo6_pull_down_cfg_START (6)
#define PMIC_RAMP_LDO6_CTRL1_ldo6_pull_down_cfg_END (6)
#define PMIC_RAMP_LDO6_CTRL1_ldo6_pull_down_mode_START (7)
#define PMIC_RAMP_LDO6_CTRL1_ldo6_pull_down_mode_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo44_rampdown_ctrl : 3;
        unsigned char ldo44_rampup_ctrl : 3;
        unsigned char ldo44_ramp_en_cfg : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_RAMP_LDO44_CTRL0_UNION;
#endif
#define PMIC_RAMP_LDO44_CTRL0_ldo44_rampdown_ctrl_START (0)
#define PMIC_RAMP_LDO44_CTRL0_ldo44_rampdown_ctrl_END (2)
#define PMIC_RAMP_LDO44_CTRL0_ldo44_rampup_ctrl_START (3)
#define PMIC_RAMP_LDO44_CTRL0_ldo44_rampup_ctrl_END (5)
#define PMIC_RAMP_LDO44_CTRL0_ldo44_ramp_en_cfg_START (6)
#define PMIC_RAMP_LDO44_CTRL0_ldo44_ramp_en_cfg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo44_pull_down_off_cfg : 3;
        unsigned char ldo44_pull_down_on_cfg : 3;
        unsigned char ldo44_pull_down_cfg : 1;
        unsigned char ldo44_pull_down_mode : 1;
    } reg;
} PMIC_RAMP_LDO44_CTRL1_UNION;
#endif
#define PMIC_RAMP_LDO44_CTRL1_ldo44_pull_down_off_cfg_START (0)
#define PMIC_RAMP_LDO44_CTRL1_ldo44_pull_down_off_cfg_END (2)
#define PMIC_RAMP_LDO44_CTRL1_ldo44_pull_down_on_cfg_START (3)
#define PMIC_RAMP_LDO44_CTRL1_ldo44_pull_down_on_cfg_END (5)
#define PMIC_RAMP_LDO44_CTRL1_ldo44_pull_down_cfg_START (6)
#define PMIC_RAMP_LDO44_CTRL1_ldo44_pull_down_cfg_END (6)
#define PMIC_RAMP_LDO44_CTRL1_ldo44_pull_down_mode_START (7)
#define PMIC_RAMP_LDO44_CTRL1_ldo44_pull_down_mode_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo47_rampdown_ctrl : 3;
        unsigned char ldo47_rampup_ctrl : 3;
        unsigned char ldo47_ramp_en_cfg : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_RAMP_LDO47_CTRL0_UNION;
#endif
#define PMIC_RAMP_LDO47_CTRL0_ldo47_rampdown_ctrl_START (0)
#define PMIC_RAMP_LDO47_CTRL0_ldo47_rampdown_ctrl_END (2)
#define PMIC_RAMP_LDO47_CTRL0_ldo47_rampup_ctrl_START (3)
#define PMIC_RAMP_LDO47_CTRL0_ldo47_rampup_ctrl_END (5)
#define PMIC_RAMP_LDO47_CTRL0_ldo47_ramp_en_cfg_START (6)
#define PMIC_RAMP_LDO47_CTRL0_ldo47_ramp_en_cfg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo47_pull_down_off_cfg : 3;
        unsigned char ldo47_pull_down_on_cfg : 3;
        unsigned char ldo47_pull_down_cfg : 1;
        unsigned char ldo47_pull_down_mode : 1;
    } reg;
} PMIC_RAMP_LDO47_CTRL1_UNION;
#endif
#define PMIC_RAMP_LDO47_CTRL1_ldo47_pull_down_off_cfg_START (0)
#define PMIC_RAMP_LDO47_CTRL1_ldo47_pull_down_off_cfg_END (2)
#define PMIC_RAMP_LDO47_CTRL1_ldo47_pull_down_on_cfg_START (3)
#define PMIC_RAMP_LDO47_CTRL1_ldo47_pull_down_on_cfg_END (5)
#define PMIC_RAMP_LDO47_CTRL1_ldo47_pull_down_cfg_START (6)
#define PMIC_RAMP_LDO47_CTRL1_ldo47_pull_down_cfg_END (6)
#define PMIC_RAMP_LDO47_CTRL1_ldo47_pull_down_mode_START (7)
#define PMIC_RAMP_LDO47_CTRL1_ldo47_pull_down_mode_END (7)
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
        unsigned char pwronn_cnt : 8;
    } reg;
} PMIC_PWRONN_CNT_UNION;
#endif
#define PMIC_PWRONN_CNT_pwronn_cnt_START (0)
#define PMIC_PWRONN_CNT_pwronn_cnt_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pwronn_cnt_mask : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_PWRONN_CNT_MASK_UNION;
#endif
#define PMIC_PWRONN_CNT_MASK_pwronn_cnt_mask_START (0)
#define PMIC_PWRONN_CNT_MASK_pwronn_cnt_mask_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_thsd_otmp125_d1mr_mk : 1;
        unsigned char irq_vbus_insert_d20mr_mk : 1;
        unsigned char irq_vbus_insert_d20mf_mk : 1;
        unsigned char irq_alarmon_r_mk : 1;
        unsigned char irq_pwronn_d6sf_mk : 1;
        unsigned char irq_pwronn_d1sf_mk : 1;
        unsigned char irq_pwronn_d20mr_mk : 1;
        unsigned char irq_pwronn_d20mf_mk : 1;
    } reg;
} PMIC_IRQ_MASK_0_UNION;
#endif
#define PMIC_IRQ_MASK_0_irq_thsd_otmp125_d1mr_mk_START (0)
#define PMIC_IRQ_MASK_0_irq_thsd_otmp125_d1mr_mk_END (0)
#define PMIC_IRQ_MASK_0_irq_vbus_insert_d20mr_mk_START (1)
#define PMIC_IRQ_MASK_0_irq_vbus_insert_d20mr_mk_END (1)
#define PMIC_IRQ_MASK_0_irq_vbus_insert_d20mf_mk_START (2)
#define PMIC_IRQ_MASK_0_irq_vbus_insert_d20mf_mk_END (2)
#define PMIC_IRQ_MASK_0_irq_alarmon_r_mk_START (3)
#define PMIC_IRQ_MASK_0_irq_alarmon_r_mk_END (3)
#define PMIC_IRQ_MASK_0_irq_pwronn_d6sf_mk_START (4)
#define PMIC_IRQ_MASK_0_irq_pwronn_d6sf_mk_END (4)
#define PMIC_IRQ_MASK_0_irq_pwronn_d1sf_mk_START (5)
#define PMIC_IRQ_MASK_0_irq_pwronn_d1sf_mk_END (5)
#define PMIC_IRQ_MASK_0_irq_pwronn_d20mr_mk_START (6)
#define PMIC_IRQ_MASK_0_irq_pwronn_d20mr_mk_END (6)
#define PMIC_IRQ_MASK_0_irq_pwronn_d20mf_mk_START (7)
#define PMIC_IRQ_MASK_0_irq_pwronn_d20mf_mk_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_ocp_scp_ovp_mk : 1;
        unsigned char irq_coul_mk : 1;
        unsigned char irq_sim0_rise_mk : 1;
        unsigned char irq_sim0_fall_mk : 1;
        unsigned char irq_sim1_rise_mk : 1;
        unsigned char irq_sim1_fall_mk : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_IRQ_MASK_1_UNION;
#endif
#define PMIC_IRQ_MASK_1_irq_ocp_scp_ovp_mk_START (0)
#define PMIC_IRQ_MASK_1_irq_ocp_scp_ovp_mk_END (0)
#define PMIC_IRQ_MASK_1_irq_coul_mk_START (1)
#define PMIC_IRQ_MASK_1_irq_coul_mk_END (1)
#define PMIC_IRQ_MASK_1_irq_sim0_rise_mk_START (2)
#define PMIC_IRQ_MASK_1_irq_sim0_rise_mk_END (2)
#define PMIC_IRQ_MASK_1_irq_sim0_fall_mk_START (3)
#define PMIC_IRQ_MASK_1_irq_sim0_fall_mk_END (3)
#define PMIC_IRQ_MASK_1_irq_sim1_rise_mk_START (4)
#define PMIC_IRQ_MASK_1_irq_sim1_rise_mk_END (4)
#define PMIC_IRQ_MASK_1_irq_sim1_fall_mk_START (5)
#define PMIC_IRQ_MASK_1_irq_sim1_fall_mk_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_b6_ocp_mk : 1;
        unsigned char irq_b5_ocp_mk : 1;
        unsigned char irq_b41_ocp_mk : 1;
        unsigned char irq_b40_ocp_mk : 1;
        unsigned char irq_b3_ocp_mk : 1;
        unsigned char irq_b2_ocp_mk : 1;
        unsigned char irq_b1_ocp_mk : 1;
        unsigned char irq_b0_ocp_mk : 1;
    } reg;
} PMIC_IRQ_OCP_MASK0_UNION;
#endif
#define PMIC_IRQ_OCP_MASK0_irq_b6_ocp_mk_START (0)
#define PMIC_IRQ_OCP_MASK0_irq_b6_ocp_mk_END (0)
#define PMIC_IRQ_OCP_MASK0_irq_b5_ocp_mk_START (1)
#define PMIC_IRQ_OCP_MASK0_irq_b5_ocp_mk_END (1)
#define PMIC_IRQ_OCP_MASK0_irq_b41_ocp_mk_START (2)
#define PMIC_IRQ_OCP_MASK0_irq_b41_ocp_mk_END (2)
#define PMIC_IRQ_OCP_MASK0_irq_b40_ocp_mk_START (3)
#define PMIC_IRQ_OCP_MASK0_irq_b40_ocp_mk_END (3)
#define PMIC_IRQ_OCP_MASK0_irq_b3_ocp_mk_START (4)
#define PMIC_IRQ_OCP_MASK0_irq_b3_ocp_mk_END (4)
#define PMIC_IRQ_OCP_MASK0_irq_b2_ocp_mk_START (5)
#define PMIC_IRQ_OCP_MASK0_irq_b2_ocp_mk_END (5)
#define PMIC_IRQ_OCP_MASK0_irq_b1_ocp_mk_START (6)
#define PMIC_IRQ_OCP_MASK0_irq_b1_ocp_mk_END (6)
#define PMIC_IRQ_OCP_MASK0_irq_b0_ocp_mk_START (7)
#define PMIC_IRQ_OCP_MASK0_irq_b0_ocp_mk_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_ldo6_ocp_mk : 1;
        unsigned char irq_ldo5_ocp_mk : 1;
        unsigned char irq_ldo4_ocp_mk : 1;
        unsigned char irq_ldo3_ocp_mk : 1;
        unsigned char irq_ldo2_ocp_mk : 1;
        unsigned char irq_ldo1_ocp_mk : 1;
        unsigned char irq_ldo0_ocp_mk : 1;
        unsigned char irq_b7_ocp_mk : 1;
    } reg;
} PMIC_IRQ_OCP_MASK1_UNION;
#endif
#define PMIC_IRQ_OCP_MASK1_irq_ldo6_ocp_mk_START (0)
#define PMIC_IRQ_OCP_MASK1_irq_ldo6_ocp_mk_END (0)
#define PMIC_IRQ_OCP_MASK1_irq_ldo5_ocp_mk_START (1)
#define PMIC_IRQ_OCP_MASK1_irq_ldo5_ocp_mk_END (1)
#define PMIC_IRQ_OCP_MASK1_irq_ldo4_ocp_mk_START (2)
#define PMIC_IRQ_OCP_MASK1_irq_ldo4_ocp_mk_END (2)
#define PMIC_IRQ_OCP_MASK1_irq_ldo3_ocp_mk_START (3)
#define PMIC_IRQ_OCP_MASK1_irq_ldo3_ocp_mk_END (3)
#define PMIC_IRQ_OCP_MASK1_irq_ldo2_ocp_mk_START (4)
#define PMIC_IRQ_OCP_MASK1_irq_ldo2_ocp_mk_END (4)
#define PMIC_IRQ_OCP_MASK1_irq_ldo1_ocp_mk_START (5)
#define PMIC_IRQ_OCP_MASK1_irq_ldo1_ocp_mk_END (5)
#define PMIC_IRQ_OCP_MASK1_irq_ldo0_ocp_mk_START (6)
#define PMIC_IRQ_OCP_MASK1_irq_ldo0_ocp_mk_END (6)
#define PMIC_IRQ_OCP_MASK1_irq_b7_ocp_mk_START (7)
#define PMIC_IRQ_OCP_MASK1_irq_b7_ocp_mk_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_ldo16_ocp_mk : 1;
        unsigned char irq_ldo15_ocp_mk : 1;
        unsigned char irq_ldo14_ocp_mk : 1;
        unsigned char irq_ldo13_ocp_mk : 1;
        unsigned char irq_ldo12_ocp_mk : 1;
        unsigned char irq_ldo11_ocp_mk : 1;
        unsigned char irq_ldo9_ocp_mk : 1;
        unsigned char irq_ldo8_ocp_mk : 1;
    } reg;
} PMIC_IRQ_OCP_MASK2_UNION;
#endif
#define PMIC_IRQ_OCP_MASK2_irq_ldo16_ocp_mk_START (0)
#define PMIC_IRQ_OCP_MASK2_irq_ldo16_ocp_mk_END (0)
#define PMIC_IRQ_OCP_MASK2_irq_ldo15_ocp_mk_START (1)
#define PMIC_IRQ_OCP_MASK2_irq_ldo15_ocp_mk_END (1)
#define PMIC_IRQ_OCP_MASK2_irq_ldo14_ocp_mk_START (2)
#define PMIC_IRQ_OCP_MASK2_irq_ldo14_ocp_mk_END (2)
#define PMIC_IRQ_OCP_MASK2_irq_ldo13_ocp_mk_START (3)
#define PMIC_IRQ_OCP_MASK2_irq_ldo13_ocp_mk_END (3)
#define PMIC_IRQ_OCP_MASK2_irq_ldo12_ocp_mk_START (4)
#define PMIC_IRQ_OCP_MASK2_irq_ldo12_ocp_mk_END (4)
#define PMIC_IRQ_OCP_MASK2_irq_ldo11_ocp_mk_START (5)
#define PMIC_IRQ_OCP_MASK2_irq_ldo11_ocp_mk_END (5)
#define PMIC_IRQ_OCP_MASK2_irq_ldo9_ocp_mk_START (6)
#define PMIC_IRQ_OCP_MASK2_irq_ldo9_ocp_mk_END (6)
#define PMIC_IRQ_OCP_MASK2_irq_ldo8_ocp_mk_START (7)
#define PMIC_IRQ_OCP_MASK2_irq_ldo8_ocp_mk_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_ldo24_ocp_mk : 1;
        unsigned char irq_ldo23_ocp_mk : 1;
        unsigned char irq_ldo22_ocp_mk : 1;
        unsigned char irq_ldo21_ocp_mk : 1;
        unsigned char irq_ldo20_ocp_mk : 1;
        unsigned char irq_ldo19_ocp_mk : 1;
        unsigned char irq_ldo18_ocp_mk : 1;
        unsigned char irq_ldo17_ocp_mk : 1;
    } reg;
} PMIC_IRQ_OCP_MASK3_UNION;
#endif
#define PMIC_IRQ_OCP_MASK3_irq_ldo24_ocp_mk_START (0)
#define PMIC_IRQ_OCP_MASK3_irq_ldo24_ocp_mk_END (0)
#define PMIC_IRQ_OCP_MASK3_irq_ldo23_ocp_mk_START (1)
#define PMIC_IRQ_OCP_MASK3_irq_ldo23_ocp_mk_END (1)
#define PMIC_IRQ_OCP_MASK3_irq_ldo22_ocp_mk_START (2)
#define PMIC_IRQ_OCP_MASK3_irq_ldo22_ocp_mk_END (2)
#define PMIC_IRQ_OCP_MASK3_irq_ldo21_ocp_mk_START (3)
#define PMIC_IRQ_OCP_MASK3_irq_ldo21_ocp_mk_END (3)
#define PMIC_IRQ_OCP_MASK3_irq_ldo20_ocp_mk_START (4)
#define PMIC_IRQ_OCP_MASK3_irq_ldo20_ocp_mk_END (4)
#define PMIC_IRQ_OCP_MASK3_irq_ldo19_ocp_mk_START (5)
#define PMIC_IRQ_OCP_MASK3_irq_ldo19_ocp_mk_END (5)
#define PMIC_IRQ_OCP_MASK3_irq_ldo18_ocp_mk_START (6)
#define PMIC_IRQ_OCP_MASK3_irq_ldo18_ocp_mk_END (6)
#define PMIC_IRQ_OCP_MASK3_irq_ldo17_ocp_mk_START (7)
#define PMIC_IRQ_OCP_MASK3_irq_ldo17_ocp_mk_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_ldo32_ocp_mk : 1;
        unsigned char irq_ldo31_ocp_mk : 1;
        unsigned char irq_ldo30_ocp_mk : 1;
        unsigned char irq_ldo29_ocp_mk : 1;
        unsigned char irq_ldo28_ocp_mk : 1;
        unsigned char irq_ldo27_ocp_mk : 1;
        unsigned char irq_ldo26_ocp_mk : 1;
        unsigned char irq_ldo25_ocp_mk : 1;
    } reg;
} PMIC_IRQ_OCP_MASK4_UNION;
#endif
#define PMIC_IRQ_OCP_MASK4_irq_ldo32_ocp_mk_START (0)
#define PMIC_IRQ_OCP_MASK4_irq_ldo32_ocp_mk_END (0)
#define PMIC_IRQ_OCP_MASK4_irq_ldo31_ocp_mk_START (1)
#define PMIC_IRQ_OCP_MASK4_irq_ldo31_ocp_mk_END (1)
#define PMIC_IRQ_OCP_MASK4_irq_ldo30_ocp_mk_START (2)
#define PMIC_IRQ_OCP_MASK4_irq_ldo30_ocp_mk_END (2)
#define PMIC_IRQ_OCP_MASK4_irq_ldo29_ocp_mk_START (3)
#define PMIC_IRQ_OCP_MASK4_irq_ldo29_ocp_mk_END (3)
#define PMIC_IRQ_OCP_MASK4_irq_ldo28_ocp_mk_START (4)
#define PMIC_IRQ_OCP_MASK4_irq_ldo28_ocp_mk_END (4)
#define PMIC_IRQ_OCP_MASK4_irq_ldo27_ocp_mk_START (5)
#define PMIC_IRQ_OCP_MASK4_irq_ldo27_ocp_mk_END (5)
#define PMIC_IRQ_OCP_MASK4_irq_ldo26_ocp_mk_START (6)
#define PMIC_IRQ_OCP_MASK4_irq_ldo26_ocp_mk_END (6)
#define PMIC_IRQ_OCP_MASK4_irq_ldo25_ocp_mk_START (7)
#define PMIC_IRQ_OCP_MASK4_irq_ldo25_ocp_mk_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_ldo42_ocp_mk : 1;
        unsigned char irq_ldo41_ocp_mk : 1;
        unsigned char irq_ldo40_ocp_mk : 1;
        unsigned char irq_ldo39_ocp_mk : 1;
        unsigned char irq_ldo38_ocp_mk : 1;
        unsigned char irq_ldo37_ocp_mk : 1;
        unsigned char irq_ldo34_ocp_mk : 1;
        unsigned char irq_ldo33_ocp_mk : 1;
    } reg;
} PMIC_IRQ_OCP_MASK5_UNION;
#endif
#define PMIC_IRQ_OCP_MASK5_irq_ldo42_ocp_mk_START (0)
#define PMIC_IRQ_OCP_MASK5_irq_ldo42_ocp_mk_END (0)
#define PMIC_IRQ_OCP_MASK5_irq_ldo41_ocp_mk_START (1)
#define PMIC_IRQ_OCP_MASK5_irq_ldo41_ocp_mk_END (1)
#define PMIC_IRQ_OCP_MASK5_irq_ldo40_ocp_mk_START (2)
#define PMIC_IRQ_OCP_MASK5_irq_ldo40_ocp_mk_END (2)
#define PMIC_IRQ_OCP_MASK5_irq_ldo39_ocp_mk_START (3)
#define PMIC_IRQ_OCP_MASK5_irq_ldo39_ocp_mk_END (3)
#define PMIC_IRQ_OCP_MASK5_irq_ldo38_ocp_mk_START (4)
#define PMIC_IRQ_OCP_MASK5_irq_ldo38_ocp_mk_END (4)
#define PMIC_IRQ_OCP_MASK5_irq_ldo37_ocp_mk_START (5)
#define PMIC_IRQ_OCP_MASK5_irq_ldo37_ocp_mk_END (5)
#define PMIC_IRQ_OCP_MASK5_irq_ldo34_ocp_mk_START (6)
#define PMIC_IRQ_OCP_MASK5_irq_ldo34_ocp_mk_END (6)
#define PMIC_IRQ_OCP_MASK5_irq_ldo33_ocp_mk_START (7)
#define PMIC_IRQ_OCP_MASK5_irq_ldo33_ocp_mk_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_ldo50_ocp_mk : 1;
        unsigned char irq_ldo49_ocp_mk : 1;
        unsigned char irq_ldo48_ocp_mk : 1;
        unsigned char irq_ldo47_ocp_mk : 1;
        unsigned char irq_ldo46_ocp_mk : 1;
        unsigned char irq_ldo45_ocp_mk : 1;
        unsigned char irq_ldo44_ocp_mk : 1;
        unsigned char irq_ldo43_ocp_mk : 1;
    } reg;
} PMIC_IRQ_OCP_MASK6_UNION;
#endif
#define PMIC_IRQ_OCP_MASK6_irq_ldo50_ocp_mk_START (0)
#define PMIC_IRQ_OCP_MASK6_irq_ldo50_ocp_mk_END (0)
#define PMIC_IRQ_OCP_MASK6_irq_ldo49_ocp_mk_START (1)
#define PMIC_IRQ_OCP_MASK6_irq_ldo49_ocp_mk_END (1)
#define PMIC_IRQ_OCP_MASK6_irq_ldo48_ocp_mk_START (2)
#define PMIC_IRQ_OCP_MASK6_irq_ldo48_ocp_mk_END (2)
#define PMIC_IRQ_OCP_MASK6_irq_ldo47_ocp_mk_START (3)
#define PMIC_IRQ_OCP_MASK6_irq_ldo47_ocp_mk_END (3)
#define PMIC_IRQ_OCP_MASK6_irq_ldo46_ocp_mk_START (4)
#define PMIC_IRQ_OCP_MASK6_irq_ldo46_ocp_mk_END (4)
#define PMIC_IRQ_OCP_MASK6_irq_ldo45_ocp_mk_START (5)
#define PMIC_IRQ_OCP_MASK6_irq_ldo45_ocp_mk_END (5)
#define PMIC_IRQ_OCP_MASK6_irq_ldo44_ocp_mk_START (6)
#define PMIC_IRQ_OCP_MASK6_irq_ldo44_ocp_mk_END (6)
#define PMIC_IRQ_OCP_MASK6_irq_ldo43_ocp_mk_START (7)
#define PMIC_IRQ_OCP_MASK6_irq_ldo43_ocp_mk_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_pmuh_ocp_mk : 1;
        unsigned char irq_classd_ocp_mk : 1;
        unsigned char irq_discharge_ocp_mk : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_IRQ_OCP_MASK7_UNION;
#endif
#define PMIC_IRQ_OCP_MASK7_irq_pmuh_ocp_mk_START (0)
#define PMIC_IRQ_OCP_MASK7_irq_pmuh_ocp_mk_END (0)
#define PMIC_IRQ_OCP_MASK7_irq_classd_ocp_mk_START (1)
#define PMIC_IRQ_OCP_MASK7_irq_classd_ocp_mk_END (1)
#define PMIC_IRQ_OCP_MASK7_irq_discharge_ocp_mk_START (2)
#define PMIC_IRQ_OCP_MASK7_irq_discharge_ocp_mk_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_b7_scp_mk : 1;
        unsigned char irq_b6_scp_mk : 1;
        unsigned char irq_b5_scp_mk : 1;
        unsigned char irq_b4_scp_mk : 1;
        unsigned char irq_b3_scp_mk : 1;
        unsigned char irq_b2_scp_mk : 1;
        unsigned char irq_b1_scp_mk : 1;
        unsigned char irq_b0_scp_mk : 1;
    } reg;
} PMIC_IRQ_SCP_MASK0_UNION;
#endif
#define PMIC_IRQ_SCP_MASK0_irq_b7_scp_mk_START (0)
#define PMIC_IRQ_SCP_MASK0_irq_b7_scp_mk_END (0)
#define PMIC_IRQ_SCP_MASK0_irq_b6_scp_mk_START (1)
#define PMIC_IRQ_SCP_MASK0_irq_b6_scp_mk_END (1)
#define PMIC_IRQ_SCP_MASK0_irq_b5_scp_mk_START (2)
#define PMIC_IRQ_SCP_MASK0_irq_b5_scp_mk_END (2)
#define PMIC_IRQ_SCP_MASK0_irq_b4_scp_mk_START (3)
#define PMIC_IRQ_SCP_MASK0_irq_b4_scp_mk_END (3)
#define PMIC_IRQ_SCP_MASK0_irq_b3_scp_mk_START (4)
#define PMIC_IRQ_SCP_MASK0_irq_b3_scp_mk_END (4)
#define PMIC_IRQ_SCP_MASK0_irq_b2_scp_mk_START (5)
#define PMIC_IRQ_SCP_MASK0_irq_b2_scp_mk_END (5)
#define PMIC_IRQ_SCP_MASK0_irq_b1_scp_mk_START (6)
#define PMIC_IRQ_SCP_MASK0_irq_b1_scp_mk_END (6)
#define PMIC_IRQ_SCP_MASK0_irq_b0_scp_mk_START (7)
#define PMIC_IRQ_SCP_MASK0_irq_b0_scp_mk_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_ldo_scp_mk : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_IRQ_SCP_MASK1_UNION;
#endif
#define PMIC_IRQ_SCP_MASK1_irq_ldo_scp_mk_START (0)
#define PMIC_IRQ_SCP_MASK1_irq_ldo_scp_mk_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_b4_ovp_mk : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_IRQ_OVP_MASK_UNION;
#endif
#define PMIC_IRQ_OVP_MASK_irq_b4_ovp_mk_START (0)
#define PMIC_IRQ_OVP_MASK_irq_b4_ovp_mk_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_b4_cur_mk : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_IRQ_CUR_DET_MASK_UNION;
#endif
#define PMIC_IRQ_CUR_DET_MASK_irq_b4_cur_mk_START (0)
#define PMIC_IRQ_CUR_DET_MASK_irq_b4_cur_mk_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_gpio_0 : 1;
        unsigned char irq_mask_gpio_1 : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_IRQ_GPIO_MASK_UNION;
#endif
#define PMIC_IRQ_GPIO_MASK_irq_mask_gpio_0_START (0)
#define PMIC_IRQ_GPIO_MASK_irq_mask_gpio_0_END (0)
#define PMIC_IRQ_GPIO_MASK_irq_mask_gpio_1_START (1)
#define PMIC_IRQ_GPIO_MASK_irq_mask_gpio_1_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_voice_restart_0 : 1;
        unsigned char irq_mask_voice_restart_1 : 1;
        unsigned char irq_mask_voice_restart_2 : 1;
        unsigned char irq_mask_voice_restart_3 : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_IRQ_VOICE_RESTART_MASK_UNION;
#endif
#define PMIC_IRQ_VOICE_RESTART_MASK_irq_mask_voice_restart_0_START (0)
#define PMIC_IRQ_VOICE_RESTART_MASK_irq_mask_voice_restart_0_END (0)
#define PMIC_IRQ_VOICE_RESTART_MASK_irq_mask_voice_restart_1_START (1)
#define PMIC_IRQ_VOICE_RESTART_MASK_irq_mask_voice_restart_1_END (1)
#define PMIC_IRQ_VOICE_RESTART_MASK_irq_mask_voice_restart_2_START (2)
#define PMIC_IRQ_VOICE_RESTART_MASK_irq_mask_voice_restart_2_END (2)
#define PMIC_IRQ_VOICE_RESTART_MASK_irq_mask_voice_restart_3_START (3)
#define PMIC_IRQ_VOICE_RESTART_MASK_irq_mask_voice_restart_3_END (3)
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
} PMIC_IRQ_MASK_14_UNION;
#endif
#define PMIC_IRQ_MASK_14_irq_mask_soh_0_0_START (0)
#define PMIC_IRQ_MASK_14_irq_mask_soh_0_0_END (0)
#define PMIC_IRQ_MASK_14_irq_mask_soh_0_1_START (1)
#define PMIC_IRQ_MASK_14_irq_mask_soh_0_1_END (1)
#define PMIC_IRQ_MASK_14_irq_mask_soh_0_2_START (2)
#define PMIC_IRQ_MASK_14_irq_mask_soh_0_2_END (2)
#define PMIC_IRQ_MASK_14_irq_mask_soh_0_3_START (3)
#define PMIC_IRQ_MASK_14_irq_mask_soh_0_3_END (3)
#define PMIC_IRQ_MASK_14_irq_mask_soh_0_4_START (4)
#define PMIC_IRQ_MASK_14_irq_mask_soh_0_4_END (4)
#define PMIC_IRQ_MASK_14_irq_mask_soh_0_5_START (5)
#define PMIC_IRQ_MASK_14_irq_mask_soh_0_5_END (5)
#define PMIC_IRQ_MASK_14_irq_mask_soh_0_6_START (6)
#define PMIC_IRQ_MASK_14_irq_mask_soh_0_6_END (6)
#define PMIC_IRQ_MASK_14_irq_mask_soh_0_7_START (7)
#define PMIC_IRQ_MASK_14_irq_mask_soh_0_7_END (7)
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
} PMIC_IRQ_MASK_15_UNION;
#endif
#define PMIC_IRQ_MASK_15_irq_mask_dcr_0_START (0)
#define PMIC_IRQ_MASK_15_irq_mask_dcr_0_END (0)
#define PMIC_IRQ_MASK_15_irq_mask_dcr_1_START (1)
#define PMIC_IRQ_MASK_15_irq_mask_dcr_1_END (1)
#define PMIC_IRQ_MASK_15_irq_mask_dcr_2_START (2)
#define PMIC_IRQ_MASK_15_irq_mask_dcr_2_END (2)
#define PMIC_IRQ_MASK_15_irq_mask_dcr_7_3_START (3)
#define PMIC_IRQ_MASK_15_irq_mask_dcr_7_3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_eis_0 : 1;
        unsigned char irq_mask_eis_1 : 1;
        unsigned char irq_mask_eis_2 : 1;
        unsigned char irq_mask_eis_3 : 1;
        unsigned char irq_mask_eis_4 : 1;
        unsigned char irq_mask_eis_5 : 1;
        unsigned char irq_mask_eis_6 : 1;
        unsigned char irq_mask_eis_7 : 1;
    } reg;
} PMIC_IRQ_MASK_16_UNION;
#endif
#define PMIC_IRQ_MASK_16_irq_mask_eis_0_START (0)
#define PMIC_IRQ_MASK_16_irq_mask_eis_0_END (0)
#define PMIC_IRQ_MASK_16_irq_mask_eis_1_START (1)
#define PMIC_IRQ_MASK_16_irq_mask_eis_1_END (1)
#define PMIC_IRQ_MASK_16_irq_mask_eis_2_START (2)
#define PMIC_IRQ_MASK_16_irq_mask_eis_2_END (2)
#define PMIC_IRQ_MASK_16_irq_mask_eis_3_START (3)
#define PMIC_IRQ_MASK_16_irq_mask_eis_3_END (3)
#define PMIC_IRQ_MASK_16_irq_mask_eis_4_START (4)
#define PMIC_IRQ_MASK_16_irq_mask_eis_4_END (4)
#define PMIC_IRQ_MASK_16_irq_mask_eis_5_START (5)
#define PMIC_IRQ_MASK_16_irq_mask_eis_5_END (5)
#define PMIC_IRQ_MASK_16_irq_mask_eis_6_START (6)
#define PMIC_IRQ_MASK_16_irq_mask_eis_6_END (6)
#define PMIC_IRQ_MASK_16_irq_mask_eis_7_START (7)
#define PMIC_IRQ_MASK_16_irq_mask_eis_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_ramp_buck7 : 1;
        unsigned char irq_mask_ramp_buck5 : 1;
        unsigned char irq_mask_ramp_buck4 : 1;
        unsigned char irq_mask_ramp_buck0 : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_IRQ_MASK_17_UNION;
#endif
#define PMIC_IRQ_MASK_17_irq_mask_ramp_buck7_START (0)
#define PMIC_IRQ_MASK_17_irq_mask_ramp_buck7_END (0)
#define PMIC_IRQ_MASK_17_irq_mask_ramp_buck5_START (1)
#define PMIC_IRQ_MASK_17_irq_mask_ramp_buck5_END (1)
#define PMIC_IRQ_MASK_17_irq_mask_ramp_buck4_START (2)
#define PMIC_IRQ_MASK_17_irq_mask_ramp_buck4_END (2)
#define PMIC_IRQ_MASK_17_irq_mask_ramp_buck0_START (3)
#define PMIC_IRQ_MASK_17_irq_mask_ramp_buck0_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_ramp_ldo47 : 1;
        unsigned char irq_mask_ramp_ldo44 : 1;
        unsigned char irq_mask_ramp_ldo40 : 1;
        unsigned char irq_mask_ramp_ldo6 : 1;
        unsigned char irq_mask_ramp_ldo0 : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_IRQ_MASK_18_UNION;
#endif
#define PMIC_IRQ_MASK_18_irq_mask_ramp_ldo47_START (0)
#define PMIC_IRQ_MASK_18_irq_mask_ramp_ldo47_END (0)
#define PMIC_IRQ_MASK_18_irq_mask_ramp_ldo44_START (1)
#define PMIC_IRQ_MASK_18_irq_mask_ramp_ldo44_END (1)
#define PMIC_IRQ_MASK_18_irq_mask_ramp_ldo40_START (2)
#define PMIC_IRQ_MASK_18_irq_mask_ramp_ldo40_END (2)
#define PMIC_IRQ_MASK_18_irq_mask_ramp_ldo6_START (3)
#define PMIC_IRQ_MASK_18_irq_mask_ramp_ldo6_END (3)
#define PMIC_IRQ_MASK_18_irq_mask_ramp_ldo0_START (4)
#define PMIC_IRQ_MASK_18_irq_mask_ramp_ldo0_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_mask_vsys_drop_0 : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_IRQ_MASK_19_UNION;
#endif
#define PMIC_IRQ_MASK_19_irq_mask_vsys_drop_0_START (0)
#define PMIC_IRQ_MASK_19_irq_mask_vsys_drop_0_END (0)
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
        unsigned char ocp_scp_ovp_r : 1;
        unsigned char coul_r : 1;
        unsigned char sim0_hpd_r : 1;
        unsigned char sim0_hpd_f : 1;
        unsigned char sim1_hpd_r : 1;
        unsigned char sim1_hpd_f : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_IRQ1_UNION;
#endif
#define PMIC_IRQ1_ocp_scp_ovp_r_START (0)
#define PMIC_IRQ1_ocp_scp_ovp_r_END (0)
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
        unsigned char irq_b6_ocp : 1;
        unsigned char irq_b5_ocp : 1;
        unsigned char irq_b41_ocp : 1;
        unsigned char irq_b40_ocp : 1;
        unsigned char irq_b3_ocp : 1;
        unsigned char irq_b2_ocp : 1;
        unsigned char irq_b1_ocp : 1;
        unsigned char irq_b0_ocp : 1;
    } reg;
} PMIC_IRQ_OCP0_UNION;
#endif
#define PMIC_IRQ_OCP0_irq_b6_ocp_START (0)
#define PMIC_IRQ_OCP0_irq_b6_ocp_END (0)
#define PMIC_IRQ_OCP0_irq_b5_ocp_START (1)
#define PMIC_IRQ_OCP0_irq_b5_ocp_END (1)
#define PMIC_IRQ_OCP0_irq_b41_ocp_START (2)
#define PMIC_IRQ_OCP0_irq_b41_ocp_END (2)
#define PMIC_IRQ_OCP0_irq_b40_ocp_START (3)
#define PMIC_IRQ_OCP0_irq_b40_ocp_END (3)
#define PMIC_IRQ_OCP0_irq_b3_ocp_START (4)
#define PMIC_IRQ_OCP0_irq_b3_ocp_END (4)
#define PMIC_IRQ_OCP0_irq_b2_ocp_START (5)
#define PMIC_IRQ_OCP0_irq_b2_ocp_END (5)
#define PMIC_IRQ_OCP0_irq_b1_ocp_START (6)
#define PMIC_IRQ_OCP0_irq_b1_ocp_END (6)
#define PMIC_IRQ_OCP0_irq_b0_ocp_START (7)
#define PMIC_IRQ_OCP0_irq_b0_ocp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_ldo6_ocp : 1;
        unsigned char irq_ldo5_ocp : 1;
        unsigned char irq_ldo4_ocp : 1;
        unsigned char irq_ldo3_ocp : 1;
        unsigned char irq_ldo2_ocp : 1;
        unsigned char irq_ldo1_ocp : 1;
        unsigned char irq_ldo0_ocp : 1;
        unsigned char irq_b7_ocp : 1;
    } reg;
} PMIC_IRQ_OCP1_UNION;
#endif
#define PMIC_IRQ_OCP1_irq_ldo6_ocp_START (0)
#define PMIC_IRQ_OCP1_irq_ldo6_ocp_END (0)
#define PMIC_IRQ_OCP1_irq_ldo5_ocp_START (1)
#define PMIC_IRQ_OCP1_irq_ldo5_ocp_END (1)
#define PMIC_IRQ_OCP1_irq_ldo4_ocp_START (2)
#define PMIC_IRQ_OCP1_irq_ldo4_ocp_END (2)
#define PMIC_IRQ_OCP1_irq_ldo3_ocp_START (3)
#define PMIC_IRQ_OCP1_irq_ldo3_ocp_END (3)
#define PMIC_IRQ_OCP1_irq_ldo2_ocp_START (4)
#define PMIC_IRQ_OCP1_irq_ldo2_ocp_END (4)
#define PMIC_IRQ_OCP1_irq_ldo1_ocp_START (5)
#define PMIC_IRQ_OCP1_irq_ldo1_ocp_END (5)
#define PMIC_IRQ_OCP1_irq_ldo0_ocp_START (6)
#define PMIC_IRQ_OCP1_irq_ldo0_ocp_END (6)
#define PMIC_IRQ_OCP1_irq_b7_ocp_START (7)
#define PMIC_IRQ_OCP1_irq_b7_ocp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_ldo16_ocp : 1;
        unsigned char irq_ldo15_ocp : 1;
        unsigned char irq_ldo14_ocp : 1;
        unsigned char irq_ldo13_ocp : 1;
        unsigned char irq_ldo12_ocp : 1;
        unsigned char irq_ldo11_ocp : 1;
        unsigned char irq_ldo9_ocp : 1;
        unsigned char irq_ldo8_ocp : 1;
    } reg;
} PMIC_IRQ_OCP2_UNION;
#endif
#define PMIC_IRQ_OCP2_irq_ldo16_ocp_START (0)
#define PMIC_IRQ_OCP2_irq_ldo16_ocp_END (0)
#define PMIC_IRQ_OCP2_irq_ldo15_ocp_START (1)
#define PMIC_IRQ_OCP2_irq_ldo15_ocp_END (1)
#define PMIC_IRQ_OCP2_irq_ldo14_ocp_START (2)
#define PMIC_IRQ_OCP2_irq_ldo14_ocp_END (2)
#define PMIC_IRQ_OCP2_irq_ldo13_ocp_START (3)
#define PMIC_IRQ_OCP2_irq_ldo13_ocp_END (3)
#define PMIC_IRQ_OCP2_irq_ldo12_ocp_START (4)
#define PMIC_IRQ_OCP2_irq_ldo12_ocp_END (4)
#define PMIC_IRQ_OCP2_irq_ldo11_ocp_START (5)
#define PMIC_IRQ_OCP2_irq_ldo11_ocp_END (5)
#define PMIC_IRQ_OCP2_irq_ldo9_ocp_START (6)
#define PMIC_IRQ_OCP2_irq_ldo9_ocp_END (6)
#define PMIC_IRQ_OCP2_irq_ldo8_ocp_START (7)
#define PMIC_IRQ_OCP2_irq_ldo8_ocp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_ldo24_ocp : 1;
        unsigned char irq_ldo23_ocp : 1;
        unsigned char irq_ldo22_ocp : 1;
        unsigned char irq_ldo21_ocp : 1;
        unsigned char irq_ldo20_ocp : 1;
        unsigned char irq_ldo19_ocp : 1;
        unsigned char irq_ldo18_ocp : 1;
        unsigned char irq_ldo17_ocp : 1;
    } reg;
} PMIC_IRQ_OCP3_UNION;
#endif
#define PMIC_IRQ_OCP3_irq_ldo24_ocp_START (0)
#define PMIC_IRQ_OCP3_irq_ldo24_ocp_END (0)
#define PMIC_IRQ_OCP3_irq_ldo23_ocp_START (1)
#define PMIC_IRQ_OCP3_irq_ldo23_ocp_END (1)
#define PMIC_IRQ_OCP3_irq_ldo22_ocp_START (2)
#define PMIC_IRQ_OCP3_irq_ldo22_ocp_END (2)
#define PMIC_IRQ_OCP3_irq_ldo21_ocp_START (3)
#define PMIC_IRQ_OCP3_irq_ldo21_ocp_END (3)
#define PMIC_IRQ_OCP3_irq_ldo20_ocp_START (4)
#define PMIC_IRQ_OCP3_irq_ldo20_ocp_END (4)
#define PMIC_IRQ_OCP3_irq_ldo19_ocp_START (5)
#define PMIC_IRQ_OCP3_irq_ldo19_ocp_END (5)
#define PMIC_IRQ_OCP3_irq_ldo18_ocp_START (6)
#define PMIC_IRQ_OCP3_irq_ldo18_ocp_END (6)
#define PMIC_IRQ_OCP3_irq_ldo17_ocp_START (7)
#define PMIC_IRQ_OCP3_irq_ldo17_ocp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_ldo32_ocp : 1;
        unsigned char irq_ldo31_ocp : 1;
        unsigned char irq_ldo30_ocp : 1;
        unsigned char irq_ldo29_ocp : 1;
        unsigned char irq_ldo28_ocp : 1;
        unsigned char irq_ldo27_ocp : 1;
        unsigned char irq_ldo26_ocp : 1;
        unsigned char irq_ldo25_ocp : 1;
    } reg;
} PMIC_IRQ_OCP4_UNION;
#endif
#define PMIC_IRQ_OCP4_irq_ldo32_ocp_START (0)
#define PMIC_IRQ_OCP4_irq_ldo32_ocp_END (0)
#define PMIC_IRQ_OCP4_irq_ldo31_ocp_START (1)
#define PMIC_IRQ_OCP4_irq_ldo31_ocp_END (1)
#define PMIC_IRQ_OCP4_irq_ldo30_ocp_START (2)
#define PMIC_IRQ_OCP4_irq_ldo30_ocp_END (2)
#define PMIC_IRQ_OCP4_irq_ldo29_ocp_START (3)
#define PMIC_IRQ_OCP4_irq_ldo29_ocp_END (3)
#define PMIC_IRQ_OCP4_irq_ldo28_ocp_START (4)
#define PMIC_IRQ_OCP4_irq_ldo28_ocp_END (4)
#define PMIC_IRQ_OCP4_irq_ldo27_ocp_START (5)
#define PMIC_IRQ_OCP4_irq_ldo27_ocp_END (5)
#define PMIC_IRQ_OCP4_irq_ldo26_ocp_START (6)
#define PMIC_IRQ_OCP4_irq_ldo26_ocp_END (6)
#define PMIC_IRQ_OCP4_irq_ldo25_ocp_START (7)
#define PMIC_IRQ_OCP4_irq_ldo25_ocp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_ldo42_ocp : 1;
        unsigned char irq_ldo41_ocp : 1;
        unsigned char irq_ldo40_ocp : 1;
        unsigned char irq_ldo39_ocp : 1;
        unsigned char irq_ldo38_ocp : 1;
        unsigned char irq_ldo37_ocp : 1;
        unsigned char irq_ldo34_ocp : 1;
        unsigned char irq_ldo33_ocp : 1;
    } reg;
} PMIC_IRQ_OCP5_UNION;
#endif
#define PMIC_IRQ_OCP5_irq_ldo42_ocp_START (0)
#define PMIC_IRQ_OCP5_irq_ldo42_ocp_END (0)
#define PMIC_IRQ_OCP5_irq_ldo41_ocp_START (1)
#define PMIC_IRQ_OCP5_irq_ldo41_ocp_END (1)
#define PMIC_IRQ_OCP5_irq_ldo40_ocp_START (2)
#define PMIC_IRQ_OCP5_irq_ldo40_ocp_END (2)
#define PMIC_IRQ_OCP5_irq_ldo39_ocp_START (3)
#define PMIC_IRQ_OCP5_irq_ldo39_ocp_END (3)
#define PMIC_IRQ_OCP5_irq_ldo38_ocp_START (4)
#define PMIC_IRQ_OCP5_irq_ldo38_ocp_END (4)
#define PMIC_IRQ_OCP5_irq_ldo37_ocp_START (5)
#define PMIC_IRQ_OCP5_irq_ldo37_ocp_END (5)
#define PMIC_IRQ_OCP5_irq_ldo34_ocp_START (6)
#define PMIC_IRQ_OCP5_irq_ldo34_ocp_END (6)
#define PMIC_IRQ_OCP5_irq_ldo33_ocp_START (7)
#define PMIC_IRQ_OCP5_irq_ldo33_ocp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_ldo50_ocp : 1;
        unsigned char irq_ldo49_ocp : 1;
        unsigned char irq_ldo48_ocp : 1;
        unsigned char irq_ldo47_ocp : 1;
        unsigned char irq_ldo46_ocp : 1;
        unsigned char irq_ldo45_ocp : 1;
        unsigned char irq_ldo44_ocp : 1;
        unsigned char irq_ldo43_ocp : 1;
    } reg;
} PMIC_IRQ_OCP6_UNION;
#endif
#define PMIC_IRQ_OCP6_irq_ldo50_ocp_START (0)
#define PMIC_IRQ_OCP6_irq_ldo50_ocp_END (0)
#define PMIC_IRQ_OCP6_irq_ldo49_ocp_START (1)
#define PMIC_IRQ_OCP6_irq_ldo49_ocp_END (1)
#define PMIC_IRQ_OCP6_irq_ldo48_ocp_START (2)
#define PMIC_IRQ_OCP6_irq_ldo48_ocp_END (2)
#define PMIC_IRQ_OCP6_irq_ldo47_ocp_START (3)
#define PMIC_IRQ_OCP6_irq_ldo47_ocp_END (3)
#define PMIC_IRQ_OCP6_irq_ldo46_ocp_START (4)
#define PMIC_IRQ_OCP6_irq_ldo46_ocp_END (4)
#define PMIC_IRQ_OCP6_irq_ldo45_ocp_START (5)
#define PMIC_IRQ_OCP6_irq_ldo45_ocp_END (5)
#define PMIC_IRQ_OCP6_irq_ldo44_ocp_START (6)
#define PMIC_IRQ_OCP6_irq_ldo44_ocp_END (6)
#define PMIC_IRQ_OCP6_irq_ldo43_ocp_START (7)
#define PMIC_IRQ_OCP6_irq_ldo43_ocp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_pmuh_ocp : 1;
        unsigned char irq_classd_ocp : 1;
        unsigned char irq_discharge_ocp : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_IRQ_OCP7_UNION;
#endif
#define PMIC_IRQ_OCP7_irq_pmuh_ocp_START (0)
#define PMIC_IRQ_OCP7_irq_pmuh_ocp_END (0)
#define PMIC_IRQ_OCP7_irq_classd_ocp_START (1)
#define PMIC_IRQ_OCP7_irq_classd_ocp_END (1)
#define PMIC_IRQ_OCP7_irq_discharge_ocp_START (2)
#define PMIC_IRQ_OCP7_irq_discharge_ocp_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_b7_scp : 1;
        unsigned char irq_b6_scp : 1;
        unsigned char irq_b5_scp : 1;
        unsigned char irq_b4_scp : 1;
        unsigned char irq_b3_scp : 1;
        unsigned char irq_b2_scp : 1;
        unsigned char irq_b1_scp : 1;
        unsigned char irq_b0_scp : 1;
    } reg;
} PMIC_IRQ_SCP0_UNION;
#endif
#define PMIC_IRQ_SCP0_irq_b7_scp_START (0)
#define PMIC_IRQ_SCP0_irq_b7_scp_END (0)
#define PMIC_IRQ_SCP0_irq_b6_scp_START (1)
#define PMIC_IRQ_SCP0_irq_b6_scp_END (1)
#define PMIC_IRQ_SCP0_irq_b5_scp_START (2)
#define PMIC_IRQ_SCP0_irq_b5_scp_END (2)
#define PMIC_IRQ_SCP0_irq_b4_scp_START (3)
#define PMIC_IRQ_SCP0_irq_b4_scp_END (3)
#define PMIC_IRQ_SCP0_irq_b3_scp_START (4)
#define PMIC_IRQ_SCP0_irq_b3_scp_END (4)
#define PMIC_IRQ_SCP0_irq_b2_scp_START (5)
#define PMIC_IRQ_SCP0_irq_b2_scp_END (5)
#define PMIC_IRQ_SCP0_irq_b1_scp_START (6)
#define PMIC_IRQ_SCP0_irq_b1_scp_END (6)
#define PMIC_IRQ_SCP0_irq_b0_scp_START (7)
#define PMIC_IRQ_SCP0_irq_b0_scp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_ldo_scp : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_IRQ_SCP1_UNION;
#endif
#define PMIC_IRQ_SCP1_irq_ldo_scp_START (0)
#define PMIC_IRQ_SCP1_irq_ldo_scp_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_b4_ovp : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_IRQ_OVP_UNION;
#endif
#define PMIC_IRQ_OVP_irq_b4_ovp_START (0)
#define PMIC_IRQ_OVP_irq_b4_ovp_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_b4_cur : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_IRQ_CUR_DET_UNION;
#endif
#define PMIC_IRQ_CUR_DET_irq_b4_cur_START (0)
#define PMIC_IRQ_CUR_DET_irq_b4_cur_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char gpio0_int_status : 1;
        unsigned char gpio1_int_status : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_GPIO_IRQ_UNION;
#endif
#define PMIC_GPIO_IRQ_gpio0_int_status_START (0)
#define PMIC_GPIO_IRQ_gpio0_int_status_END (0)
#define PMIC_GPIO_IRQ_gpio1_int_status_START (1)
#define PMIC_GPIO_IRQ_gpio1_int_status_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char voice_restart1_r : 1;
        unsigned char voice_restart1_f : 1;
        unsigned char voice_restart2_r : 1;
        unsigned char voice_restart2_f : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_VOICE_RESTART_IRQ_UNION;
#endif
#define PMIC_VOICE_RESTART_IRQ_voice_restart1_r_START (0)
#define PMIC_VOICE_RESTART_IRQ_voice_restart1_r_END (0)
#define PMIC_VOICE_RESTART_IRQ_voice_restart1_f_START (1)
#define PMIC_VOICE_RESTART_IRQ_voice_restart1_f_END (1)
#define PMIC_VOICE_RESTART_IRQ_voice_restart2_r_START (2)
#define PMIC_VOICE_RESTART_IRQ_voice_restart2_r_END (2)
#define PMIC_VOICE_RESTART_IRQ_voice_restart2_f_START (3)
#define PMIC_VOICE_RESTART_IRQ_voice_restart2_f_END (3)
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
        unsigned char soh_ovp : 1;
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
#define PMIC_ACR_SOH_IRQ_soh_ovp_START (7)
#define PMIC_ACR_SOH_IRQ_soh_ovp_END (7)
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
        unsigned char eis_start : 1;
        unsigned char eis_end : 1;
        unsigned char eis_int1 : 1;
        unsigned char eis_int2 : 1;
        unsigned char eis_int3 : 1;
        unsigned char eis_int4 : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_EIS_IRQ_UNION;
#endif
#define PMIC_EIS_IRQ_eis_start_START (0)
#define PMIC_EIS_IRQ_eis_start_END (0)
#define PMIC_EIS_IRQ_eis_end_START (1)
#define PMIC_EIS_IRQ_eis_end_END (1)
#define PMIC_EIS_IRQ_eis_int1_START (2)
#define PMIC_EIS_IRQ_eis_int1_END (2)
#define PMIC_EIS_IRQ_eis_int2_START (3)
#define PMIC_EIS_IRQ_eis_int2_END (3)
#define PMIC_EIS_IRQ_eis_int3_START (4)
#define PMIC_EIS_IRQ_eis_int3_END (4)
#define PMIC_EIS_IRQ_eis_int4_START (5)
#define PMIC_EIS_IRQ_eis_int4_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_ramp_buck7 : 1;
        unsigned char irq_ramp_buck5 : 1;
        unsigned char irq_ramp_buck4 : 1;
        unsigned char irq_ramp_buck0 : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_IRQ_17_UNION;
#endif
#define PMIC_IRQ_17_irq_ramp_buck7_START (0)
#define PMIC_IRQ_17_irq_ramp_buck7_END (0)
#define PMIC_IRQ_17_irq_ramp_buck5_START (1)
#define PMIC_IRQ_17_irq_ramp_buck5_END (1)
#define PMIC_IRQ_17_irq_ramp_buck4_START (2)
#define PMIC_IRQ_17_irq_ramp_buck4_END (2)
#define PMIC_IRQ_17_irq_ramp_buck0_START (3)
#define PMIC_IRQ_17_irq_ramp_buck0_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_ramp_ldo47 : 1;
        unsigned char irq_ramp_ldo44 : 1;
        unsigned char irq_ramp_ldo40 : 1;
        unsigned char irq_ramp_ldo6 : 1;
        unsigned char irq_ramp_ldo0 : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_IRQ_18_UNION;
#endif
#define PMIC_IRQ_18_irq_ramp_ldo47_START (0)
#define PMIC_IRQ_18_irq_ramp_ldo47_END (0)
#define PMIC_IRQ_18_irq_ramp_ldo44_START (1)
#define PMIC_IRQ_18_irq_ramp_ldo44_END (1)
#define PMIC_IRQ_18_irq_ramp_ldo40_START (2)
#define PMIC_IRQ_18_irq_ramp_ldo40_END (2)
#define PMIC_IRQ_18_irq_ramp_ldo6_START (3)
#define PMIC_IRQ_18_irq_ramp_ldo6_END (3)
#define PMIC_IRQ_18_irq_ramp_ldo0_START (4)
#define PMIC_IRQ_18_irq_ramp_ldo0_END (4)
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
        unsigned char np_vsys_ov_d200ur : 1;
        unsigned char np_vsys_pwroff_abs_d20nr : 1;
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
#define PMIC_NP_RECORD0_np_vsys_pwroff_abs_d20nr_START (1)
#define PMIC_NP_RECORD0_np_vsys_pwroff_abs_d20nr_END (1)
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
        unsigned char np_pmud_short_f : 1;
        unsigned char np_pmuh_short_f : 1;
        unsigned char np_vin_ldoh_shutdown : 1;
        unsigned char np_vsys_pwron_shutdown : 1;
        unsigned char np_nfc_shutdown : 1;
        unsigned char np_core_io12_18_vld_f : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_NP_RECORD3_UNION;
#endif
#define PMIC_NP_RECORD3_np_pmud_short_f_START (0)
#define PMIC_NP_RECORD3_np_pmud_short_f_END (0)
#define PMIC_NP_RECORD3_np_pmuh_short_f_START (1)
#define PMIC_NP_RECORD3_np_pmuh_short_f_END (1)
#define PMIC_NP_RECORD3_np_vin_ldoh_shutdown_START (2)
#define PMIC_NP_RECORD3_np_vin_ldoh_shutdown_END (2)
#define PMIC_NP_RECORD3_np_vsys_pwron_shutdown_START (3)
#define PMIC_NP_RECORD3_np_vsys_pwron_shutdown_END (3)
#define PMIC_NP_RECORD3_np_nfc_shutdown_START (4)
#define PMIC_NP_RECORD3_np_nfc_shutdown_END (4)
#define PMIC_NP_RECORD3_np_core_io12_18_vld_f_START (5)
#define PMIC_NP_RECORD3_np_core_io12_18_vld_f_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_cali_pmuh_ocp : 1;
        unsigned char np_cali_ldo26_ocp : 1;
        unsigned char np_cali_buck2_scp : 1;
        unsigned char np_cali_buck2_ocp : 1;
        unsigned char np_cali_pmuh_short : 1;
        unsigned char np_cali_pmud_short : 1;
        unsigned char np_cali_vsys_pwroff_deb : 1;
        unsigned char np_cali_vsys_pwroff_abs : 1;
    } reg;
} PMIC_NP_RECORD4_UNION;
#endif
#define PMIC_NP_RECORD4_np_cali_pmuh_ocp_START (0)
#define PMIC_NP_RECORD4_np_cali_pmuh_ocp_END (0)
#define PMIC_NP_RECORD4_np_cali_ldo26_ocp_START (1)
#define PMIC_NP_RECORD4_np_cali_ldo26_ocp_END (1)
#define PMIC_NP_RECORD4_np_cali_buck2_scp_START (2)
#define PMIC_NP_RECORD4_np_cali_buck2_scp_END (2)
#define PMIC_NP_RECORD4_np_cali_buck2_ocp_START (3)
#define PMIC_NP_RECORD4_np_cali_buck2_ocp_END (3)
#define PMIC_NP_RECORD4_np_cali_pmuh_short_START (4)
#define PMIC_NP_RECORD4_np_cali_pmuh_short_END (4)
#define PMIC_NP_RECORD4_np_cali_pmud_short_START (5)
#define PMIC_NP_RECORD4_np_cali_pmud_short_END (5)
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
        unsigned char np_b6_ocp : 1;
        unsigned char np_b5_ocp : 1;
        unsigned char np_b41_ocp : 1;
        unsigned char np_b40_ocp : 1;
        unsigned char np_b3_ocp : 1;
        unsigned char np_b2_ocp : 1;
        unsigned char np_b1_ocp : 1;
        unsigned char np_b0_ocp : 1;
    } reg;
} PMIC_NP_OCP0_UNION;
#endif
#define PMIC_NP_OCP0_np_b6_ocp_START (0)
#define PMIC_NP_OCP0_np_b6_ocp_END (0)
#define PMIC_NP_OCP0_np_b5_ocp_START (1)
#define PMIC_NP_OCP0_np_b5_ocp_END (1)
#define PMIC_NP_OCP0_np_b41_ocp_START (2)
#define PMIC_NP_OCP0_np_b41_ocp_END (2)
#define PMIC_NP_OCP0_np_b40_ocp_START (3)
#define PMIC_NP_OCP0_np_b40_ocp_END (3)
#define PMIC_NP_OCP0_np_b3_ocp_START (4)
#define PMIC_NP_OCP0_np_b3_ocp_END (4)
#define PMIC_NP_OCP0_np_b2_ocp_START (5)
#define PMIC_NP_OCP0_np_b2_ocp_END (5)
#define PMIC_NP_OCP0_np_b1_ocp_START (6)
#define PMIC_NP_OCP0_np_b1_ocp_END (6)
#define PMIC_NP_OCP0_np_b0_ocp_START (7)
#define PMIC_NP_OCP0_np_b0_ocp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo6_ocp : 1;
        unsigned char np_ldo5_ocp : 1;
        unsigned char np_ldo4_ocp : 1;
        unsigned char np_ldo3_ocp : 1;
        unsigned char np_ldo2_ocp : 1;
        unsigned char np_ldo1_ocp : 1;
        unsigned char np_ldo0_ocp : 1;
        unsigned char np_b7_ocp : 1;
    } reg;
} PMIC_NP_OCP1_UNION;
#endif
#define PMIC_NP_OCP1_np_ldo6_ocp_START (0)
#define PMIC_NP_OCP1_np_ldo6_ocp_END (0)
#define PMIC_NP_OCP1_np_ldo5_ocp_START (1)
#define PMIC_NP_OCP1_np_ldo5_ocp_END (1)
#define PMIC_NP_OCP1_np_ldo4_ocp_START (2)
#define PMIC_NP_OCP1_np_ldo4_ocp_END (2)
#define PMIC_NP_OCP1_np_ldo3_ocp_START (3)
#define PMIC_NP_OCP1_np_ldo3_ocp_END (3)
#define PMIC_NP_OCP1_np_ldo2_ocp_START (4)
#define PMIC_NP_OCP1_np_ldo2_ocp_END (4)
#define PMIC_NP_OCP1_np_ldo1_ocp_START (5)
#define PMIC_NP_OCP1_np_ldo1_ocp_END (5)
#define PMIC_NP_OCP1_np_ldo0_ocp_START (6)
#define PMIC_NP_OCP1_np_ldo0_ocp_END (6)
#define PMIC_NP_OCP1_np_b7_ocp_START (7)
#define PMIC_NP_OCP1_np_b7_ocp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo16_ocp : 1;
        unsigned char np_ldo15_ocp : 1;
        unsigned char np_ldo14_ocp : 1;
        unsigned char np_ldo13_ocp : 1;
        unsigned char np_ldo12_ocp : 1;
        unsigned char np_ldo11_ocp : 1;
        unsigned char np_ldo9_ocp : 1;
        unsigned char np_ldo8_ocp : 1;
    } reg;
} PMIC_NP_OCP2_UNION;
#endif
#define PMIC_NP_OCP2_np_ldo16_ocp_START (0)
#define PMIC_NP_OCP2_np_ldo16_ocp_END (0)
#define PMIC_NP_OCP2_np_ldo15_ocp_START (1)
#define PMIC_NP_OCP2_np_ldo15_ocp_END (1)
#define PMIC_NP_OCP2_np_ldo14_ocp_START (2)
#define PMIC_NP_OCP2_np_ldo14_ocp_END (2)
#define PMIC_NP_OCP2_np_ldo13_ocp_START (3)
#define PMIC_NP_OCP2_np_ldo13_ocp_END (3)
#define PMIC_NP_OCP2_np_ldo12_ocp_START (4)
#define PMIC_NP_OCP2_np_ldo12_ocp_END (4)
#define PMIC_NP_OCP2_np_ldo11_ocp_START (5)
#define PMIC_NP_OCP2_np_ldo11_ocp_END (5)
#define PMIC_NP_OCP2_np_ldo9_ocp_START (6)
#define PMIC_NP_OCP2_np_ldo9_ocp_END (6)
#define PMIC_NP_OCP2_np_ldo8_ocp_START (7)
#define PMIC_NP_OCP2_np_ldo8_ocp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo24_ocp : 1;
        unsigned char np_ldo23_ocp : 1;
        unsigned char np_ldo22_ocp : 1;
        unsigned char np_ldo21_ocp : 1;
        unsigned char np_ldo20_ocp : 1;
        unsigned char np_ldo19_ocp : 1;
        unsigned char np_ldo18_ocp : 1;
        unsigned char np_ldo17_ocp : 1;
    } reg;
} PMIC_NP_OCP3_UNION;
#endif
#define PMIC_NP_OCP3_np_ldo24_ocp_START (0)
#define PMIC_NP_OCP3_np_ldo24_ocp_END (0)
#define PMIC_NP_OCP3_np_ldo23_ocp_START (1)
#define PMIC_NP_OCP3_np_ldo23_ocp_END (1)
#define PMIC_NP_OCP3_np_ldo22_ocp_START (2)
#define PMIC_NP_OCP3_np_ldo22_ocp_END (2)
#define PMIC_NP_OCP3_np_ldo21_ocp_START (3)
#define PMIC_NP_OCP3_np_ldo21_ocp_END (3)
#define PMIC_NP_OCP3_np_ldo20_ocp_START (4)
#define PMIC_NP_OCP3_np_ldo20_ocp_END (4)
#define PMIC_NP_OCP3_np_ldo19_ocp_START (5)
#define PMIC_NP_OCP3_np_ldo19_ocp_END (5)
#define PMIC_NP_OCP3_np_ldo18_ocp_START (6)
#define PMIC_NP_OCP3_np_ldo18_ocp_END (6)
#define PMIC_NP_OCP3_np_ldo17_ocp_START (7)
#define PMIC_NP_OCP3_np_ldo17_ocp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo32_ocp : 1;
        unsigned char np_ldo31_ocp : 1;
        unsigned char np_ldo30_ocp : 1;
        unsigned char np_ldo29_ocp : 1;
        unsigned char np_ldo28_ocp : 1;
        unsigned char np_ldo27_ocp : 1;
        unsigned char np_ldo26_ocp : 1;
        unsigned char np_ldo25_ocp : 1;
    } reg;
} PMIC_NP_OCP4_UNION;
#endif
#define PMIC_NP_OCP4_np_ldo32_ocp_START (0)
#define PMIC_NP_OCP4_np_ldo32_ocp_END (0)
#define PMIC_NP_OCP4_np_ldo31_ocp_START (1)
#define PMIC_NP_OCP4_np_ldo31_ocp_END (1)
#define PMIC_NP_OCP4_np_ldo30_ocp_START (2)
#define PMIC_NP_OCP4_np_ldo30_ocp_END (2)
#define PMIC_NP_OCP4_np_ldo29_ocp_START (3)
#define PMIC_NP_OCP4_np_ldo29_ocp_END (3)
#define PMIC_NP_OCP4_np_ldo28_ocp_START (4)
#define PMIC_NP_OCP4_np_ldo28_ocp_END (4)
#define PMIC_NP_OCP4_np_ldo27_ocp_START (5)
#define PMIC_NP_OCP4_np_ldo27_ocp_END (5)
#define PMIC_NP_OCP4_np_ldo26_ocp_START (6)
#define PMIC_NP_OCP4_np_ldo26_ocp_END (6)
#define PMIC_NP_OCP4_np_ldo25_ocp_START (7)
#define PMIC_NP_OCP4_np_ldo25_ocp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo42_ocp : 1;
        unsigned char np_ldo41_ocp : 1;
        unsigned char np_ldo40_ocp : 1;
        unsigned char np_ldo39_ocp : 1;
        unsigned char np_ldo38_ocp : 1;
        unsigned char np_ldo37_ocp : 1;
        unsigned char np_ldo34_ocp : 1;
        unsigned char np_ldo33_ocp : 1;
    } reg;
} PMIC_NP_OCP5_UNION;
#endif
#define PMIC_NP_OCP5_np_ldo42_ocp_START (0)
#define PMIC_NP_OCP5_np_ldo42_ocp_END (0)
#define PMIC_NP_OCP5_np_ldo41_ocp_START (1)
#define PMIC_NP_OCP5_np_ldo41_ocp_END (1)
#define PMIC_NP_OCP5_np_ldo40_ocp_START (2)
#define PMIC_NP_OCP5_np_ldo40_ocp_END (2)
#define PMIC_NP_OCP5_np_ldo39_ocp_START (3)
#define PMIC_NP_OCP5_np_ldo39_ocp_END (3)
#define PMIC_NP_OCP5_np_ldo38_ocp_START (4)
#define PMIC_NP_OCP5_np_ldo38_ocp_END (4)
#define PMIC_NP_OCP5_np_ldo37_ocp_START (5)
#define PMIC_NP_OCP5_np_ldo37_ocp_END (5)
#define PMIC_NP_OCP5_np_ldo34_ocp_START (6)
#define PMIC_NP_OCP5_np_ldo34_ocp_END (6)
#define PMIC_NP_OCP5_np_ldo33_ocp_START (7)
#define PMIC_NP_OCP5_np_ldo33_ocp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo50_ocp : 1;
        unsigned char np_ldo49_ocp : 1;
        unsigned char np_ldo48_ocp : 1;
        unsigned char np_ldo47_ocp : 1;
        unsigned char np_ldo46_ocp : 1;
        unsigned char np_ldo45_ocp : 1;
        unsigned char np_ldo44_ocp : 1;
        unsigned char np_ldo43_ocp : 1;
    } reg;
} PMIC_NP_OCP6_UNION;
#endif
#define PMIC_NP_OCP6_np_ldo50_ocp_START (0)
#define PMIC_NP_OCP6_np_ldo50_ocp_END (0)
#define PMIC_NP_OCP6_np_ldo49_ocp_START (1)
#define PMIC_NP_OCP6_np_ldo49_ocp_END (1)
#define PMIC_NP_OCP6_np_ldo48_ocp_START (2)
#define PMIC_NP_OCP6_np_ldo48_ocp_END (2)
#define PMIC_NP_OCP6_np_ldo47_ocp_START (3)
#define PMIC_NP_OCP6_np_ldo47_ocp_END (3)
#define PMIC_NP_OCP6_np_ldo46_ocp_START (4)
#define PMIC_NP_OCP6_np_ldo46_ocp_END (4)
#define PMIC_NP_OCP6_np_ldo45_ocp_START (5)
#define PMIC_NP_OCP6_np_ldo45_ocp_END (5)
#define PMIC_NP_OCP6_np_ldo44_ocp_START (6)
#define PMIC_NP_OCP6_np_ldo44_ocp_END (6)
#define PMIC_NP_OCP6_np_ldo43_ocp_START (7)
#define PMIC_NP_OCP6_np_ldo43_ocp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_pmuh_ocp : 1;
        unsigned char np_classd_ocp : 1;
        unsigned char np_discharge_ocp : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_NP_OCP7_UNION;
#endif
#define PMIC_NP_OCP7_np_pmuh_ocp_START (0)
#define PMIC_NP_OCP7_np_pmuh_ocp_END (0)
#define PMIC_NP_OCP7_np_classd_ocp_START (1)
#define PMIC_NP_OCP7_np_classd_ocp_END (1)
#define PMIC_NP_OCP7_np_discharge_ocp_START (2)
#define PMIC_NP_OCP7_np_discharge_ocp_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_b7_scp : 1;
        unsigned char np_b6_scp : 1;
        unsigned char np_b5_scp : 1;
        unsigned char np_b4_scp : 1;
        unsigned char np_b3_scp : 1;
        unsigned char np_b2_scp : 1;
        unsigned char np_b1_scp : 1;
        unsigned char np_b0_scp : 1;
    } reg;
} PMIC_NP_SCP0_UNION;
#endif
#define PMIC_NP_SCP0_np_b7_scp_START (0)
#define PMIC_NP_SCP0_np_b7_scp_END (0)
#define PMIC_NP_SCP0_np_b6_scp_START (1)
#define PMIC_NP_SCP0_np_b6_scp_END (1)
#define PMIC_NP_SCP0_np_b5_scp_START (2)
#define PMIC_NP_SCP0_np_b5_scp_END (2)
#define PMIC_NP_SCP0_np_b4_scp_START (3)
#define PMIC_NP_SCP0_np_b4_scp_END (3)
#define PMIC_NP_SCP0_np_b3_scp_START (4)
#define PMIC_NP_SCP0_np_b3_scp_END (4)
#define PMIC_NP_SCP0_np_b2_scp_START (5)
#define PMIC_NP_SCP0_np_b2_scp_END (5)
#define PMIC_NP_SCP0_np_b1_scp_START (6)
#define PMIC_NP_SCP0_np_b1_scp_END (6)
#define PMIC_NP_SCP0_np_b0_scp_START (7)
#define PMIC_NP_SCP0_np_b0_scp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo_scp : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_NP_SCP1_UNION;
#endif
#define PMIC_NP_SCP1_np_ldo_scp_START (0)
#define PMIC_NP_SCP1_np_ldo_scp_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_b4_ovp : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_NP_OVP_UNION;
#endif
#define PMIC_NP_OVP_np_b4_ovp_START (0)
#define PMIC_NP_OVP_np_b4_ovp_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_b4_cur : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_NP_CUR_DET_UNION;
#endif
#define PMIC_NP_CUR_DET_np_b4_cur_START (0)
#define PMIC_NP_CUR_DET_np_b4_cur_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_buck7_ramp_chg : 1;
        unsigned char np_buck5_ramp_chg : 1;
        unsigned char np_buck4_ramp_chg : 1;
        unsigned char np_buck0_ramp_chg : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_RAMP_EVENT0_UNION;
#endif
#define PMIC_RAMP_EVENT0_np_buck7_ramp_chg_START (0)
#define PMIC_RAMP_EVENT0_np_buck7_ramp_chg_END (0)
#define PMIC_RAMP_EVENT0_np_buck5_ramp_chg_START (1)
#define PMIC_RAMP_EVENT0_np_buck5_ramp_chg_END (1)
#define PMIC_RAMP_EVENT0_np_buck4_ramp_chg_START (2)
#define PMIC_RAMP_EVENT0_np_buck4_ramp_chg_END (2)
#define PMIC_RAMP_EVENT0_np_buck0_ramp_chg_START (3)
#define PMIC_RAMP_EVENT0_np_buck0_ramp_chg_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo47_ramp_chg : 1;
        unsigned char np_ldo44_ramp_chg : 1;
        unsigned char np_ldo40_ramp_chg : 1;
        unsigned char np_ldo6_ramp_chg : 1;
        unsigned char np_ldo0_ramp_chg : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_RAMP_EVENT1_UNION;
#endif
#define PMIC_RAMP_EVENT1_np_ldo47_ramp_chg_START (0)
#define PMIC_RAMP_EVENT1_np_ldo47_ramp_chg_END (0)
#define PMIC_RAMP_EVENT1_np_ldo44_ramp_chg_START (1)
#define PMIC_RAMP_EVENT1_np_ldo44_ramp_chg_END (1)
#define PMIC_RAMP_EVENT1_np_ldo40_ramp_chg_START (2)
#define PMIC_RAMP_EVENT1_np_ldo40_ramp_chg_END (2)
#define PMIC_RAMP_EVENT1_np_ldo6_ramp_chg_START (3)
#define PMIC_RAMP_EVENT1_np_ldo6_ramp_chg_END (3)
#define PMIC_RAMP_EVENT1_np_ldo0_ramp_chg_START (4)
#define PMIC_RAMP_EVENT1_np_ldo0_ramp_chg_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_buck7_ramp_abnor : 1;
        unsigned char np_buck5_ramp_abnor : 1;
        unsigned char np_buck4_ramp_abnor : 1;
        unsigned char np_buck0_ramp_abnor : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_RAMP_EVENT2_UNION;
#endif
#define PMIC_RAMP_EVENT2_np_buck7_ramp_abnor_START (0)
#define PMIC_RAMP_EVENT2_np_buck7_ramp_abnor_END (0)
#define PMIC_RAMP_EVENT2_np_buck5_ramp_abnor_START (1)
#define PMIC_RAMP_EVENT2_np_buck5_ramp_abnor_END (1)
#define PMIC_RAMP_EVENT2_np_buck4_ramp_abnor_START (2)
#define PMIC_RAMP_EVENT2_np_buck4_ramp_abnor_END (2)
#define PMIC_RAMP_EVENT2_np_buck0_ramp_abnor_START (3)
#define PMIC_RAMP_EVENT2_np_buck0_ramp_abnor_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo47_ramp_abnor : 1;
        unsigned char np_ldo44_ramp_abnor : 1;
        unsigned char np_ldo40_ramp_abnor : 1;
        unsigned char np_ldo6_ramp_abnor : 1;
        unsigned char np_ldo0_ramp_abnor : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_RAMP_EVENT3_UNION;
#endif
#define PMIC_RAMP_EVENT3_np_ldo47_ramp_abnor_START (0)
#define PMIC_RAMP_EVENT3_np_ldo47_ramp_abnor_END (0)
#define PMIC_RAMP_EVENT3_np_ldo44_ramp_abnor_START (1)
#define PMIC_RAMP_EVENT3_np_ldo44_ramp_abnor_END (1)
#define PMIC_RAMP_EVENT3_np_ldo40_ramp_abnor_START (2)
#define PMIC_RAMP_EVENT3_np_ldo40_ramp_abnor_END (2)
#define PMIC_RAMP_EVENT3_np_ldo6_ramp_abnor_START (3)
#define PMIC_RAMP_EVENT3_np_ldo6_ramp_abnor_END (3)
#define PMIC_RAMP_EVENT3_np_ldo0_ramp_abnor_START (4)
#define PMIC_RAMP_EVENT3_np_ldo0_ramp_abnor_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_soh_tbat_tmp_ov : 1;
        unsigned char np_soh_mode_abn : 1;
        unsigned char np_soh_vbat_uv : 1;
        unsigned char np_soh_otmp125 : 1;
        unsigned char np_soh_30min_timeout : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_SOH_RECORD_UNION;
#endif
#define PMIC_SOH_RECORD_np_soh_tbat_tmp_ov_START (0)
#define PMIC_SOH_RECORD_np_soh_tbat_tmp_ov_END (0)
#define PMIC_SOH_RECORD_np_soh_mode_abn_START (1)
#define PMIC_SOH_RECORD_np_soh_mode_abn_END (1)
#define PMIC_SOH_RECORD_np_soh_vbat_uv_START (2)
#define PMIC_SOH_RECORD_np_soh_vbat_uv_END (2)
#define PMIC_SOH_RECORD_np_soh_otmp125_START (3)
#define PMIC_SOH_RECORD_np_soh_otmp125_END (3)
#define PMIC_SOH_RECORD_np_soh_30min_timeout_START (4)
#define PMIC_SOH_RECORD_np_soh_30min_timeout_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_acr_flag : 1;
        unsigned char np_acr_dischg_ocp : 1;
        unsigned char np_dcr_dischg_ocp : 1;
        unsigned char np_ovp_dischg_ocp : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_ACR_SOH_RECORD_UNION;
#endif
#define PMIC_ACR_SOH_RECORD_np_acr_flag_START (0)
#define PMIC_ACR_SOH_RECORD_np_acr_flag_END (0)
#define PMIC_ACR_SOH_RECORD_np_acr_dischg_ocp_START (1)
#define PMIC_ACR_SOH_RECORD_np_acr_dischg_ocp_END (1)
#define PMIC_ACR_SOH_RECORD_np_dcr_dischg_ocp_START (2)
#define PMIC_ACR_SOH_RECORD_np_dcr_dischg_ocp_END (2)
#define PMIC_ACR_SOH_RECORD_np_ovp_dischg_ocp_START (3)
#define PMIC_ACR_SOH_RECORD_np_ovp_dischg_ocp_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo26_ocp_ocv : 1;
        unsigned char np_buck3_scp_ocv : 1;
        unsigned char np_buck3_ocp_ocv : 1;
        unsigned char np_pmud_short_ocv : 1;
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
#define PMIC_COUL_RECORD0_np_pmud_short_ocv_START (3)
#define PMIC_COUL_RECORD0_np_pmud_short_ocv_END (3)
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
        unsigned char np_buck2_scp_ocv : 1;
        unsigned char np_buck2_ocp_ocv : 1;
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
#define PMIC_COUL_RECORD1_np_buck2_scp_ocv_START (6)
#define PMIC_COUL_RECORD1_np_buck2_scp_ocv_END (6)
#define PMIC_COUL_RECORD1_np_buck2_ocp_ocv_START (7)
#define PMIC_COUL_RECORD1_np_buck2_ocp_ocv_END (7)
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
        unsigned char np_vsys_surge_d20nr : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_NP_VSYS_SURGE_RECORD_UNION;
#endif
#define PMIC_NP_VSYS_SURGE_RECORD_np_vsys_surge_d20nr_START (0)
#define PMIC_NP_VSYS_SURGE_RECORD_np_vsys_surge_d20nr_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_pwr_onoff_place : 8;
    } reg;
} PMIC_ONOFF_PLACE_SEL_UNION;
#endif
#define PMIC_ONOFF_PLACE_SEL_np_pwr_onoff_place_START (0)
#define PMIC_ONOFF_PLACE_SEL_np_pwr_onoff_place_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo0_on_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO0_ON_PLACE_UNION;
#endif
#define PMIC_LDO0_ON_PLACE_np_ldo0_on_place_START (0)
#define PMIC_LDO0_ON_PLACE_np_ldo0_on_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_buck0_on_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK0_ON_PLACE_UNION;
#endif
#define PMIC_BUCK0_ON_PLACE_np_buck0_on_place_START (0)
#define PMIC_BUCK0_ON_PLACE_np_buck0_on_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_buck5_on_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK5_ON_PLACE_UNION;
#endif
#define PMIC_BUCK5_ON_PLACE_np_buck5_on_place_START (0)
#define PMIC_BUCK5_ON_PLACE_np_buck5_on_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo30_on_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO30_ON_PLACE_UNION;
#endif
#define PMIC_LDO30_ON_PLACE_np_ldo30_on_place_START (0)
#define PMIC_LDO30_ON_PLACE_np_ldo30_on_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo39_on_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO39_ON_PLACE_UNION;
#endif
#define PMIC_LDO39_ON_PLACE_np_ldo39_on_place_START (0)
#define PMIC_LDO39_ON_PLACE_np_ldo39_on_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo41_on_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO41_ON_PLACE_UNION;
#endif
#define PMIC_LDO41_ON_PLACE_np_ldo41_on_place_START (0)
#define PMIC_LDO41_ON_PLACE_np_ldo41_on_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo2_on_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO2_ON_PLACE_UNION;
#endif
#define PMIC_LDO2_ON_PLACE_np_ldo2_on_place_START (0)
#define PMIC_LDO2_ON_PLACE_np_ldo2_on_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo18_on_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO18_ON_PLACE_UNION;
#endif
#define PMIC_LDO18_ON_PLACE_np_ldo18_on_place_START (0)
#define PMIC_LDO18_ON_PLACE_np_ldo18_on_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo24_on_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO24_ON_PLACE_UNION;
#endif
#define PMIC_LDO24_ON_PLACE_np_ldo24_on_place_START (0)
#define PMIC_LDO24_ON_PLACE_np_ldo24_on_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_buck1_on_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK1_ON_PLACE_UNION;
#endif
#define PMIC_BUCK1_ON_PLACE_np_buck1_on_place_START (0)
#define PMIC_BUCK1_ON_PLACE_np_buck1_on_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo49_on_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO49_ON_PLACE_UNION;
#endif
#define PMIC_LDO49_ON_PLACE_np_ldo49_on_place_START (0)
#define PMIC_LDO49_ON_PLACE_np_ldo49_on_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo37_on_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO37_ON_PLACE_UNION;
#endif
#define PMIC_LDO37_ON_PLACE_np_ldo37_on_place_START (0)
#define PMIC_LDO37_ON_PLACE_np_ldo37_on_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo5_on_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO5_ON_PLACE_UNION;
#endif
#define PMIC_LDO5_ON_PLACE_np_ldo5_on_place_START (0)
#define PMIC_LDO5_ON_PLACE_np_ldo5_on_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo29_on_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO29_ON_PLACE_UNION;
#endif
#define PMIC_LDO29_ON_PLACE_np_ldo29_on_place_START (0)
#define PMIC_LDO29_ON_PLACE_np_ldo29_on_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo38_on_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO38_ON_PLACE_UNION;
#endif
#define PMIC_LDO38_ON_PLACE_np_ldo38_on_place_START (0)
#define PMIC_LDO38_ON_PLACE_np_ldo38_on_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo50_on_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO50_ON_PLACE_UNION;
#endif
#define PMIC_LDO50_ON_PLACE_np_ldo50_on_place_START (0)
#define PMIC_LDO50_ON_PLACE_np_ldo50_on_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo45_on_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO45_ON_PLACE_UNION;
#endif
#define PMIC_LDO45_ON_PLACE_np_ldo45_on_place_START (0)
#define PMIC_LDO45_ON_PLACE_np_ldo45_on_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo15_on_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO15_ON_PLACE_UNION;
#endif
#define PMIC_LDO15_ON_PLACE_np_ldo15_on_place_START (0)
#define PMIC_LDO15_ON_PLACE_np_ldo15_on_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo23_on_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO23_ON_PLACE_UNION;
#endif
#define PMIC_LDO23_ON_PLACE_np_ldo23_on_place_START (0)
#define PMIC_LDO23_ON_PLACE_np_ldo23_on_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo_buf_on_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO_BUF_ON_PLACE_UNION;
#endif
#define PMIC_LDO_BUF_ON_PLACE_np_ldo_buf_on_place_START (0)
#define PMIC_LDO_BUF_ON_PLACE_np_ldo_buf_on_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo0_off_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO0_OFF_PLACE_UNION;
#endif
#define PMIC_LDO0_OFF_PLACE_np_ldo0_off_place_START (0)
#define PMIC_LDO0_OFF_PLACE_np_ldo0_off_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_buck0_off_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK0_OFF_PLACE_UNION;
#endif
#define PMIC_BUCK0_OFF_PLACE_np_buck0_off_place_START (0)
#define PMIC_BUCK0_OFF_PLACE_np_buck0_off_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_buck5_off_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK5_OFF_PLACE_UNION;
#endif
#define PMIC_BUCK5_OFF_PLACE_np_buck5_off_place_START (0)
#define PMIC_BUCK5_OFF_PLACE_np_buck5_off_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo30_off_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO30_OFF_PLACE_UNION;
#endif
#define PMIC_LDO30_OFF_PLACE_np_ldo30_off_place_START (0)
#define PMIC_LDO30_OFF_PLACE_np_ldo30_off_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo39_off_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO39_OFF_PLACE_UNION;
#endif
#define PMIC_LDO39_OFF_PLACE_np_ldo39_off_place_START (0)
#define PMIC_LDO39_OFF_PLACE_np_ldo39_off_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo41_off_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO41_OFF_PLACE_UNION;
#endif
#define PMIC_LDO41_OFF_PLACE_np_ldo41_off_place_START (0)
#define PMIC_LDO41_OFF_PLACE_np_ldo41_off_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo2_off_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO2_OFF_PLACE_UNION;
#endif
#define PMIC_LDO2_OFF_PLACE_np_ldo2_off_place_START (0)
#define PMIC_LDO2_OFF_PLACE_np_ldo2_off_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo18_off_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO18_OFF_PLACE_UNION;
#endif
#define PMIC_LDO18_OFF_PLACE_np_ldo18_off_place_START (0)
#define PMIC_LDO18_OFF_PLACE_np_ldo18_off_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo24_off_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO24_OFF_PLACE_UNION;
#endif
#define PMIC_LDO24_OFF_PLACE_np_ldo24_off_place_START (0)
#define PMIC_LDO24_OFF_PLACE_np_ldo24_off_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_buck1_off_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK1_OFF_PLACE_UNION;
#endif
#define PMIC_BUCK1_OFF_PLACE_np_buck1_off_place_START (0)
#define PMIC_BUCK1_OFF_PLACE_np_buck1_off_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo49_off_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO49_OFF_PLACE_UNION;
#endif
#define PMIC_LDO49_OFF_PLACE_np_ldo49_off_place_START (0)
#define PMIC_LDO49_OFF_PLACE_np_ldo49_off_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo37_off_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO37_OFF_PLACE_UNION;
#endif
#define PMIC_LDO37_OFF_PLACE_np_ldo37_off_place_START (0)
#define PMIC_LDO37_OFF_PLACE_np_ldo37_off_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo5_off_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO5_OFF_PLACE_UNION;
#endif
#define PMIC_LDO5_OFF_PLACE_np_ldo5_off_place_START (0)
#define PMIC_LDO5_OFF_PLACE_np_ldo5_off_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo29_off_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO29_OFF_PLACE_UNION;
#endif
#define PMIC_LDO29_OFF_PLACE_np_ldo29_off_place_START (0)
#define PMIC_LDO29_OFF_PLACE_np_ldo29_off_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo38_off_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO38_OFF_PLACE_UNION;
#endif
#define PMIC_LDO38_OFF_PLACE_np_ldo38_off_place_START (0)
#define PMIC_LDO38_OFF_PLACE_np_ldo38_off_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo50_off_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO50_OFF_PLACE_UNION;
#endif
#define PMIC_LDO50_OFF_PLACE_np_ldo50_off_place_START (0)
#define PMIC_LDO50_OFF_PLACE_np_ldo50_off_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo45_off_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO45_OFF_PLACE_UNION;
#endif
#define PMIC_LDO45_OFF_PLACE_np_ldo45_off_place_START (0)
#define PMIC_LDO45_OFF_PLACE_np_ldo45_off_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo15_off_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO15_OFF_PLACE_UNION;
#endif
#define PMIC_LDO15_OFF_PLACE_np_ldo15_off_place_START (0)
#define PMIC_LDO15_OFF_PLACE_np_ldo15_off_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo23_off_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO23_OFF_PLACE_UNION;
#endif
#define PMIC_LDO23_OFF_PLACE_np_ldo23_off_place_START (0)
#define PMIC_LDO23_OFF_PLACE_np_ldo23_off_place_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_ldo_buf_off_place : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO_BUF_OFF_PLACE_UNION;
#endif
#define PMIC_LDO_BUF_OFF_PLACE_np_ldo_buf_off_place_START (0)
#define PMIC_LDO_BUF_OFF_PLACE_np_ldo_buf_off_place_END (4)
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
        unsigned char np_sys_debug2 : 8;
    } reg;
} PMIC_SYS_DEBUG2_UNION;
#endif
#define PMIC_SYS_DEBUG2_np_sys_debug2_START (0)
#define PMIC_SYS_DEBUG2_np_sys_debug2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_sys_debug3 : 8;
    } reg;
} PMIC_SYS_DEBUG3_UNION;
#endif
#define PMIC_SYS_DEBUG3_np_sys_debug3_START (0)
#define PMIC_SYS_DEBUG3_np_sys_debug3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_nfc_on_d2a : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_NP_NFC_ON_CTRL_UNION;
#endif
#define PMIC_NP_NFC_ON_CTRL_np_nfc_on_d2a_START (0)
#define PMIC_NP_NFC_ON_CTRL_np_nfc_on_d2a_END (0)
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
        unsigned char np_hreset_mode : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_NP_HRESET_PWRDOWN_CTRL_UNION;
#endif
#define PMIC_NP_HRESET_PWRDOWN_CTRL_np_hreset_mode_START (0)
#define PMIC_NP_HRESET_PWRDOWN_CTRL_np_hreset_mode_END (0)
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
} PMIC_NP_SMPL_CTRL_UNION;
#endif
#define PMIC_NP_SMPL_CTRL_np_smpl_open_en_START (0)
#define PMIC_NP_SMPL_CTRL_np_smpl_open_en_END (0)
#define PMIC_NP_SMPL_CTRL_np_smpl_time_sel_START (1)
#define PMIC_NP_SMPL_CTRL_np_smpl_time_sel_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_pwron_8s_sel : 1;
        unsigned char np_pwron_time_sel : 3;
        unsigned char reserved : 4;
    } reg;
} PMIC_NP_SYS_CTRL0_UNION;
#endif
#define PMIC_NP_SYS_CTRL0_np_pwron_8s_sel_START (0)
#define PMIC_NP_SYS_CTRL0_np_pwron_8s_sel_END (0)
#define PMIC_NP_SYS_CTRL0_np_pwron_time_sel_START (1)
#define PMIC_NP_SYS_CTRL0_np_pwron_time_sel_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_nfc_pwron_mask : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_NP_SYS_CTRL1_UNION;
#endif
#define PMIC_NP_SYS_CTRL1_np_nfc_pwron_mask_START (0)
#define PMIC_NP_SYS_CTRL1_np_nfc_pwron_mask_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_reg_rc_debug : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_NP_SYS_DEBUG2_UNION;
#endif
#define PMIC_NP_SYS_DEBUG2_np_reg_rc_debug_START (0)
#define PMIC_NP_SYS_DEBUG2_np_reg_rc_debug_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_reg_256k_en0 : 8;
    } reg;
} PMIC_NP_CLK_256K_CTRL0_UNION;
#endif
#define PMIC_NP_CLK_256K_CTRL0_np_reg_256k_en0_START (0)
#define PMIC_NP_CLK_256K_CTRL0_np_reg_256k_en0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_reg_256k_en1 : 8;
    } reg;
} PMIC_NP_CLK_256K_CTRL1_UNION;
#endif
#define PMIC_NP_CLK_256K_CTRL1_np_reg_256k_en1_START (0)
#define PMIC_NP_CLK_256K_CTRL1_np_reg_256k_en1_END (7)
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
} PMIC_NP_SYS_DEBUG3_UNION;
#endif
#define PMIC_NP_SYS_DEBUG3_np_vsys_pwroff_abs_pd_mask_START (0)
#define PMIC_NP_SYS_DEBUG3_np_vsys_pwroff_abs_pd_mask_END (0)
#define PMIC_NP_SYS_DEBUG3_np_vsys_pwroff_deb_pd_mask_START (1)
#define PMIC_NP_SYS_DEBUG3_np_vsys_pwroff_deb_pd_mask_END (1)
#define PMIC_NP_SYS_DEBUG3_np_thsd_otmp140_pd_mask_START (2)
#define PMIC_NP_SYS_DEBUG3_np_thsd_otmp140_pd_mask_END (2)
#define PMIC_NP_SYS_DEBUG3_np_vsys_ov_pd_mask_START (3)
#define PMIC_NP_SYS_DEBUG3_np_vsys_ov_pd_mask_END (3)
#define PMIC_NP_SYS_DEBUG3_np_vin_ldoh_vld_pd_mask_START (4)
#define PMIC_NP_SYS_DEBUG3_np_vin_ldoh_vld_pd_mask_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_vsys_pwroff_deb_set : 2;
        unsigned char np_vsys_pwroff_abs_set : 2;
        unsigned char np_vsys_pwron_set : 2;
        unsigned char np_surge_sel : 2;
    } reg;
} PMIC_VSYS_LOW_SET_UNION;
#endif
#define PMIC_VSYS_LOW_SET_np_vsys_pwroff_deb_set_START (0)
#define PMIC_VSYS_LOW_SET_np_vsys_pwroff_deb_set_END (1)
#define PMIC_VSYS_LOW_SET_np_vsys_pwroff_abs_set_START (2)
#define PMIC_VSYS_LOW_SET_np_vsys_pwroff_abs_set_END (3)
#define PMIC_VSYS_LOW_SET_np_vsys_pwron_set_START (4)
#define PMIC_VSYS_LOW_SET_np_vsys_pwron_set_END (5)
#define PMIC_VSYS_LOW_SET_np_surge_sel_START (6)
#define PMIC_VSYS_LOW_SET_np_surge_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_vsys_drop_set : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_NP_VSYS_DROP_SET_UNION;
#endif
#define PMIC_NP_VSYS_DROP_SET_np_vsys_drop_set_START (0)
#define PMIC_NP_VSYS_DROP_SET_np_vsys_drop_set_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_pmud_buck_en_cfg : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_NP_EN_PMUD_UNION;
#endif
#define PMIC_NP_EN_PMUD_np_pmud_buck_en_cfg_START (0)
#define PMIC_NP_EN_PMUD_np_pmud_buck_en_cfg_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_voice_restart1_mask : 1;
        unsigned char np_voice_restart2_mask : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_NP_VOICE_RESTART_CTRL_UNION;
#endif
#define PMIC_NP_VOICE_RESTART_CTRL_np_voice_restart1_mask_START (0)
#define PMIC_NP_VOICE_RESTART_CTRL_np_voice_restart1_mask_END (0)
#define PMIC_NP_VOICE_RESTART_CTRL_np_voice_restart2_mask_START (1)
#define PMIC_NP_VOICE_RESTART_CTRL_np_voice_restart2_mask_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_xo_dig_wifi_sel : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_NP_CLK_WIFI_CTRL_UNION;
#endif
#define PMIC_NP_CLK_WIFI_CTRL_np_xo_dig_wifi_sel_START (0)
#define PMIC_NP_CLK_WIFI_CTRL_np_xo_dig_wifi_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_xo_dig_nfc_sel : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_NP_CLK_NFC_CTRL_UNION;
#endif
#define PMIC_NP_CLK_NFC_CTRL_np_xo_dig_nfc_sel_START (0)
#define PMIC_NP_CLK_NFC_CTRL_np_xo_dig_nfc_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_xo_dig_rf_sel : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_NP_CLK_RF_CTRL_UNION;
#endif
#define PMIC_NP_CLK_RF_CTRL_np_xo_dig_rf_sel_START (0)
#define PMIC_NP_CLK_RF_CTRL_np_xo_dig_rf_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_xo_dig_codec_sel : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_NP_CLK_CODEC_CTRL_UNION;
#endif
#define PMIC_NP_CLK_CODEC_CTRL_np_xo_dig_codec_sel_START (0)
#define PMIC_NP_CLK_CODEC_CTRL_np_xo_dig_codec_sel_END (0)
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
        unsigned char np_xo_ufs_sel : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_NP_CLK_UFS_FRE_CTRL_UNION;
#endif
#define PMIC_NP_CLK_UFS_FRE_CTRL_np_xo_ufs_sel_START (0)
#define PMIC_NP_CLK_UFS_FRE_CTRL_np_xo_ufs_sel_END (0)
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
} PMIC_NP_BACKUP_CHG_UNION;
#endif
#define PMIC_NP_BACKUP_CHG_np_chg_en_START (0)
#define PMIC_NP_BACKUP_CHG_np_chg_en_END (0)
#define PMIC_NP_BACKUP_CHG_np_chg_bypass_START (1)
#define PMIC_NP_BACKUP_CHG_np_chg_bypass_END (1)
#define PMIC_NP_BACKUP_CHG_np_chg_vset_START (2)
#define PMIC_NP_BACKUP_CHG_np_chg_vset_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_d2a_reserve0 : 8;
    } reg;
} PMIC_NP_DA_RESERVE0_UNION;
#endif
#define PMIC_NP_DA_RESERVE0_np_d2a_reserve0_START (0)
#define PMIC_NP_DA_RESERVE0_np_d2a_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_d2a_reserve1 : 8;
    } reg;
} PMIC_NP_DA_RESERVE1_UNION;
#endif
#define PMIC_NP_DA_RESERVE1_np_d2a_reserve1_START (0)
#define PMIC_NP_DA_RESERVE1_np_d2a_reserve1_END (7)
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
        unsigned char np_pmud_res_sel : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_PMUD_RES_SEL_UNION;
#endif
#define PMIC_PMUD_RES_SEL_np_pmud_res_sel_START (0)
#define PMIC_PMUD_RES_SEL_np_pmud_res_sel_END (1)
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
        unsigned char np_b40_reserve0 : 8;
    } reg;
} PMIC_NP_B40_RESERVE0_UNION;
#endif
#define PMIC_NP_B40_RESERVE0_np_b40_reserve0_START (0)
#define PMIC_NP_B40_RESERVE0_np_b40_reserve0_END (7)
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
        unsigned char np_sidekey_irq_ds : 2;
    } reg;
} PMIC_DIG_IO_DS_CFG0_UNION;
#endif
#define PMIC_DIG_IO_DS_CFG0_np_clk32_bt_ds_START (0)
#define PMIC_DIG_IO_DS_CFG0_np_clk32_bt_ds_END (1)
#define PMIC_DIG_IO_DS_CFG0_np_clk32_sys_ds_START (2)
#define PMIC_DIG_IO_DS_CFG0_np_clk32_sys_ds_END (3)
#define PMIC_DIG_IO_DS_CFG0_np_spmi_data_ds_START (4)
#define PMIC_DIG_IO_DS_CFG0_np_spmi_data_ds_END (5)
#define PMIC_DIG_IO_DS_CFG0_np_sidekey_irq_ds_START (6)
#define PMIC_DIG_IO_DS_CFG0_np_sidekey_irq_ds_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_pmu0_irq_n_ds : 2;
        unsigned char np_sys_rst_n_ds : 2;
        unsigned char reserved : 2;
        unsigned char np_xoadc_ssi_ds : 2;
    } reg;
} PMIC_DIG_IO_DS_CFG1_UNION;
#endif
#define PMIC_DIG_IO_DS_CFG1_np_pmu0_irq_n_ds_START (0)
#define PMIC_DIG_IO_DS_CFG1_np_pmu0_irq_n_ds_END (1)
#define PMIC_DIG_IO_DS_CFG1_np_sys_rst_n_ds_START (2)
#define PMIC_DIG_IO_DS_CFG1_np_sys_rst_n_ds_END (3)
#define PMIC_DIG_IO_DS_CFG1_np_xoadc_ssi_ds_START (6)
#define PMIC_DIG_IO_DS_CFG1_np_xoadc_ssi_ds_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_vbat_drop_irq_ds : 2;
        unsigned char np_adc_sdata0_ds : 2;
        unsigned char np_adc_sdata1_ds : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_DIG_IO_DS_CFG2_UNION;
#endif
#define PMIC_DIG_IO_DS_CFG2_np_vbat_drop_irq_ds_START (0)
#define PMIC_DIG_IO_DS_CFG2_np_vbat_drop_irq_ds_END (1)
#define PMIC_DIG_IO_DS_CFG2_np_adc_sdata0_ds_START (2)
#define PMIC_DIG_IO_DS_CFG2_np_adc_sdata0_ds_END (3)
#define PMIC_DIG_IO_DS_CFG2_np_adc_sdata1_ds_START (4)
#define PMIC_DIG_IO_DS_CFG2_np_adc_sdata1_ds_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_spmi_data_ds_sel : 1;
        unsigned char np_xoadc_ssi_ds_sel : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_DIG_IO_DS_SEL0_UNION;
#endif
#define PMIC_DIG_IO_DS_SEL0_np_spmi_data_ds_sel_START (0)
#define PMIC_DIG_IO_DS_SEL0_np_spmi_data_ds_sel_END (0)
#define PMIC_DIG_IO_DS_SEL0_np_xoadc_ssi_ds_sel_START (1)
#define PMIC_DIG_IO_DS_SEL0_np_xoadc_ssi_ds_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_codec_io_sel : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_DIG_IO_DS_SEL1_UNION;
#endif
#define PMIC_DIG_IO_DS_SEL1_np_codec_io_sel_START (0)
#define PMIC_DIG_IO_DS_SEL1_np_codec_io_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_dummy_xoadc_ssi : 4;
        unsigned char st_dummy_xoadc_ssi : 4;
    } reg;
} PMIC_DUMMY_XOADC_SSI_UNION;
#endif
#define PMIC_DUMMY_XOADC_SSI_sc_dummy_xoadc_ssi_START (0)
#define PMIC_DUMMY_XOADC_SSI_sc_dummy_xoadc_ssi_END (3)
#define PMIC_DUMMY_XOADC_SSI_st_dummy_xoadc_ssi_START (4)
#define PMIC_DUMMY_XOADC_SSI_st_dummy_xoadc_ssi_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck_lx_trim_ctrl : 1;
        unsigned char buck_filter_ton : 2;
        unsigned char buck_dmd_clamp : 1;
        unsigned char buck_cmp_filter : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK_TOP_CTRL0_UNION;
#endif
#define PMIC_BUCK_TOP_CTRL0_buck_lx_trim_ctrl_START (0)
#define PMIC_BUCK_TOP_CTRL0_buck_lx_trim_ctrl_END (0)
#define PMIC_BUCK_TOP_CTRL0_buck_filter_ton_START (1)
#define PMIC_BUCK_TOP_CTRL0_buck_filter_ton_END (2)
#define PMIC_BUCK_TOP_CTRL0_buck_dmd_clamp_START (3)
#define PMIC_BUCK_TOP_CTRL0_buck_dmd_clamp_END (3)
#define PMIC_BUCK_TOP_CTRL0_buck_cmp_filter_START (4)
#define PMIC_BUCK_TOP_CTRL0_buck_cmp_filter_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck_reg_ss : 1;
        unsigned char buck_ocptrim_ctrl : 2;
        unsigned char buck_ocp_dis : 1;
        unsigned char buck_lx_trim_sel : 4;
    } reg;
} PMIC_BUCK_TOP_CTRL1_UNION;
#endif
#define PMIC_BUCK_TOP_CTRL1_buck_reg_ss_START (0)
#define PMIC_BUCK_TOP_CTRL1_buck_reg_ss_END (0)
#define PMIC_BUCK_TOP_CTRL1_buck_ocptrim_ctrl_START (1)
#define PMIC_BUCK_TOP_CTRL1_buck_ocptrim_ctrl_END (2)
#define PMIC_BUCK_TOP_CTRL1_buck_ocp_dis_START (3)
#define PMIC_BUCK_TOP_CTRL1_buck_ocp_dis_END (3)
#define PMIC_BUCK_TOP_CTRL1_buck_lx_trim_sel_START (4)
#define PMIC_BUCK_TOP_CTRL1_buck_lx_trim_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck_regop_clamp : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_BUCK_TOP_CTRL2_UNION;
#endif
#define PMIC_BUCK_TOP_CTRL2_buck_regop_clamp_START (0)
#define PMIC_BUCK_TOP_CTRL2_buck_regop_clamp_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck_reserve0 : 8;
    } reg;
} PMIC_BUCK_TOP_CTRL3_UNION;
#endif
#define PMIC_BUCK_TOP_CTRL3_buck_reserve0_START (0)
#define PMIC_BUCK_TOP_CTRL3_buck_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck_reserve1 : 8;
    } reg;
} PMIC_BUCK_TOP_CTRL4_UNION;
#endif
#define PMIC_BUCK_TOP_CTRL4_buck_reserve1_START (0)
#define PMIC_BUCK_TOP_CTRL4_buck_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck_vset_ctrl : 1;
        unsigned char buck_ton_trim_ctrl : 1;
        unsigned char buck_short_pdp : 1;
        unsigned char buck_ron_test : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_BUCK_TOP_CTRL5_UNION;
#endif
#define PMIC_BUCK_TOP_CTRL5_buck_vset_ctrl_START (0)
#define PMIC_BUCK_TOP_CTRL5_buck_vset_ctrl_END (0)
#define PMIC_BUCK_TOP_CTRL5_buck_ton_trim_ctrl_START (1)
#define PMIC_BUCK_TOP_CTRL5_buck_ton_trim_ctrl_END (1)
#define PMIC_BUCK_TOP_CTRL5_buck_short_pdp_START (2)
#define PMIC_BUCK_TOP_CTRL5_buck_short_pdp_END (2)
#define PMIC_BUCK_TOP_CTRL5_buck_ron_test_START (3)
#define PMIC_BUCK_TOP_CTRL5_buck_ron_test_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b0_adj_rlx : 4;
        unsigned char b0_adj_clx : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK0_CTRL0_UNION;
#endif
#define PMIC_BUCK0_CTRL0_b0_adj_rlx_START (0)
#define PMIC_BUCK0_CTRL0_b0_adj_rlx_END (3)
#define PMIC_BUCK0_CTRL0_b0_adj_clx_START (4)
#define PMIC_BUCK0_CTRL0_b0_adj_clx_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b0_dmd_blanktime_sel : 1;
        unsigned char b0_cot_rlx : 1;
        unsigned char b0_cmp_ibias : 4;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK0_CTRL1_UNION;
#endif
#define PMIC_BUCK0_CTRL1_b0_dmd_blanktime_sel_START (0)
#define PMIC_BUCK0_CTRL1_b0_dmd_blanktime_sel_END (0)
#define PMIC_BUCK0_CTRL1_b0_cot_rlx_START (1)
#define PMIC_BUCK0_CTRL1_b0_cot_rlx_END (1)
#define PMIC_BUCK0_CTRL1_b0_cmp_ibias_START (2)
#define PMIC_BUCK0_CTRL1_b0_cmp_ibias_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b0_dmd_sel_eco : 4;
        unsigned char b0_dmd_sel : 4;
    } reg;
} PMIC_BUCK0_CTRL2_UNION;
#endif
#define PMIC_BUCK0_CTRL2_b0_dmd_sel_eco_START (0)
#define PMIC_BUCK0_CTRL2_b0_dmd_sel_eco_END (3)
#define PMIC_BUCK0_CTRL2_b0_dmd_sel_START (4)
#define PMIC_BUCK0_CTRL2_b0_dmd_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b0_ea_eco_dis : 1;
        unsigned char b0_ea_clamp_eco_dis : 1;
        unsigned char b0_dt_sel : 1;
        unsigned char b0_dmd_type_sel : 1;
        unsigned char b0_dmd_ton : 3;
        unsigned char b0_dmd_shield_ton : 1;
    } reg;
} PMIC_BUCK0_CTRL3_UNION;
#endif
#define PMIC_BUCK0_CTRL3_b0_ea_eco_dis_START (0)
#define PMIC_BUCK0_CTRL3_b0_ea_eco_dis_END (0)
#define PMIC_BUCK0_CTRL3_b0_ea_clamp_eco_dis_START (1)
#define PMIC_BUCK0_CTRL3_b0_ea_clamp_eco_dis_END (1)
#define PMIC_BUCK0_CTRL3_b0_dt_sel_START (2)
#define PMIC_BUCK0_CTRL3_b0_dt_sel_END (2)
#define PMIC_BUCK0_CTRL3_b0_dmd_type_sel_START (3)
#define PMIC_BUCK0_CTRL3_b0_dmd_type_sel_END (3)
#define PMIC_BUCK0_CTRL3_b0_dmd_ton_START (4)
#define PMIC_BUCK0_CTRL3_b0_dmd_ton_END (6)
#define PMIC_BUCK0_CTRL3_b0_dmd_shield_ton_START (7)
#define PMIC_BUCK0_CTRL3_b0_dmd_shield_ton_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b0_fix_ton : 1;
        unsigned char b0_fb_cap_sel : 1;
        unsigned char b0_fastramp_ctrl : 4;
        unsigned char b0_eco_ibias_sel : 1;
        unsigned char b0_ea_ibias_sel : 1;
    } reg;
} PMIC_BUCK0_CTRL4_UNION;
#endif
#define PMIC_BUCK0_CTRL4_b0_fix_ton_START (0)
#define PMIC_BUCK0_CTRL4_b0_fix_ton_END (0)
#define PMIC_BUCK0_CTRL4_b0_fb_cap_sel_START (1)
#define PMIC_BUCK0_CTRL4_b0_fb_cap_sel_END (1)
#define PMIC_BUCK0_CTRL4_b0_fastramp_ctrl_START (2)
#define PMIC_BUCK0_CTRL4_b0_fastramp_ctrl_END (5)
#define PMIC_BUCK0_CTRL4_b0_eco_ibias_sel_START (6)
#define PMIC_BUCK0_CTRL4_b0_eco_ibias_sel_END (6)
#define PMIC_BUCK0_CTRL4_b0_ea_ibias_sel_START (7)
#define PMIC_BUCK0_CTRL4_b0_ea_ibias_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b0_min_ton : 3;
        unsigned char b0_lx_dt : 1;
        unsigned char b0_ibias_dmd_ctrl : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK0_CTRL5_UNION;
#endif
#define PMIC_BUCK0_CTRL5_b0_min_ton_START (0)
#define PMIC_BUCK0_CTRL5_b0_min_ton_END (2)
#define PMIC_BUCK0_CTRL5_b0_lx_dt_START (3)
#define PMIC_BUCK0_CTRL5_b0_lx_dt_END (3)
#define PMIC_BUCK0_CTRL5_b0_ibias_dmd_ctrl_START (4)
#define PMIC_BUCK0_CTRL5_b0_ibias_dmd_ctrl_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b0_ntp_dt_ctrl : 1;
        unsigned char b0_nonlinear_driver : 1;
        unsigned char b0_ng_p_sel : 3;
        unsigned char b0_ng_n_sel : 3;
    } reg;
} PMIC_BUCK0_CTRL6_UNION;
#endif
#define PMIC_BUCK0_CTRL6_b0_ntp_dt_ctrl_START (0)
#define PMIC_BUCK0_CTRL6_b0_ntp_dt_ctrl_END (0)
#define PMIC_BUCK0_CTRL6_b0_nonlinear_driver_START (1)
#define PMIC_BUCK0_CTRL6_b0_nonlinear_driver_END (1)
#define PMIC_BUCK0_CTRL6_b0_ng_p_sel_START (2)
#define PMIC_BUCK0_CTRL6_b0_ng_p_sel_END (4)
#define PMIC_BUCK0_CTRL6_b0_ng_n_sel_START (5)
#define PMIC_BUCK0_CTRL6_b0_ng_n_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b0_ocp_sel : 3;
        unsigned char b0_ocp_delay : 1;
        unsigned char b0_ocp_cmp_ibias : 1;
        unsigned char b0_ntp_dt_sel : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK0_CTRL7_UNION;
#endif
#define PMIC_BUCK0_CTRL7_b0_ocp_sel_START (0)
#define PMIC_BUCK0_CTRL7_b0_ocp_sel_END (2)
#define PMIC_BUCK0_CTRL7_b0_ocp_delay_START (3)
#define PMIC_BUCK0_CTRL7_b0_ocp_delay_END (3)
#define PMIC_BUCK0_CTRL7_b0_ocp_cmp_ibias_START (4)
#define PMIC_BUCK0_CTRL7_b0_ocp_cmp_ibias_END (4)
#define PMIC_BUCK0_CTRL7_b0_ntp_dt_sel_START (5)
#define PMIC_BUCK0_CTRL7_b0_ntp_dt_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b0_offres_shut : 1;
        unsigned char b0_offres_sel : 1;
        unsigned char b0_ocpibias_sel : 2;
        unsigned char b0_ocp_toff : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK0_CTRL8_UNION;
#endif
#define PMIC_BUCK0_CTRL8_b0_offres_shut_START (0)
#define PMIC_BUCK0_CTRL8_b0_offres_shut_END (0)
#define PMIC_BUCK0_CTRL8_b0_offres_sel_START (1)
#define PMIC_BUCK0_CTRL8_b0_offres_sel_END (1)
#define PMIC_BUCK0_CTRL8_b0_ocpibias_sel_START (2)
#define PMIC_BUCK0_CTRL8_b0_ocpibias_sel_END (3)
#define PMIC_BUCK0_CTRL8_b0_ocp_toff_START (4)
#define PMIC_BUCK0_CTRL8_b0_ocp_toff_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b0_ptn_dt_ctrl : 1;
        unsigned char b0_pg_p_sel : 3;
        unsigned char b0_pg_n_sel : 3;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK0_CTRL9_UNION;
#endif
#define PMIC_BUCK0_CTRL9_b0_ptn_dt_ctrl_START (0)
#define PMIC_BUCK0_CTRL9_b0_ptn_dt_ctrl_END (0)
#define PMIC_BUCK0_CTRL9_b0_pg_p_sel_START (1)
#define PMIC_BUCK0_CTRL9_b0_pg_p_sel_END (3)
#define PMIC_BUCK0_CTRL9_b0_pg_n_sel_START (4)
#define PMIC_BUCK0_CTRL9_b0_pg_n_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b0_rampdown_ndmd : 3;
        unsigned char b0_ramp_ton_ctrl : 1;
        unsigned char b0_ramp_prebias : 1;
        unsigned char b0_ptn_dt_sel : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK0_CTRL10_UNION;
#endif
#define PMIC_BUCK0_CTRL10_b0_rampdown_ndmd_START (0)
#define PMIC_BUCK0_CTRL10_b0_rampdown_ndmd_END (2)
#define PMIC_BUCK0_CTRL10_b0_ramp_ton_ctrl_START (3)
#define PMIC_BUCK0_CTRL10_b0_ramp_ton_ctrl_END (3)
#define PMIC_BUCK0_CTRL10_b0_ramp_prebias_START (4)
#define PMIC_BUCK0_CTRL10_b0_ramp_prebias_END (4)
#define PMIC_BUCK0_CTRL10_b0_ptn_dt_sel_START (5)
#define PMIC_BUCK0_CTRL10_b0_ptn_dt_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b0_reg_r : 2;
        unsigned char b0_reg_op_c : 1;
        unsigned char b0_reg_ibias : 1;
        unsigned char b0_reg_en : 1;
        unsigned char b0_reg_dr : 3;
    } reg;
} PMIC_BUCK0_CTRL11_UNION;
#endif
#define PMIC_BUCK0_CTRL11_b0_reg_r_START (0)
#define PMIC_BUCK0_CTRL11_b0_reg_r_END (1)
#define PMIC_BUCK0_CTRL11_b0_reg_op_c_START (2)
#define PMIC_BUCK0_CTRL11_b0_reg_op_c_END (2)
#define PMIC_BUCK0_CTRL11_b0_reg_ibias_START (3)
#define PMIC_BUCK0_CTRL11_b0_reg_ibias_END (3)
#define PMIC_BUCK0_CTRL11_b0_reg_en_START (4)
#define PMIC_BUCK0_CTRL11_b0_reg_en_END (4)
#define PMIC_BUCK0_CTRL11_b0_reg_dr_START (5)
#define PMIC_BUCK0_CTRL11_b0_reg_dr_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b0_reserve0 : 8;
    } reg;
} PMIC_BUCK0_CTRL12_UNION;
#endif
#define PMIC_BUCK0_CTRL12_b0_reserve0_START (0)
#define PMIC_BUCK0_CTRL12_b0_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b0_reserve1 : 8;
    } reg;
} PMIC_BUCK0_CTRL13_UNION;
#endif
#define PMIC_BUCK0_CTRL13_b0_reserve1_START (0)
#define PMIC_BUCK0_CTRL13_b0_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b0_reserve2 : 8;
    } reg;
} PMIC_BUCK0_CTRL14_UNION;
#endif
#define PMIC_BUCK0_CTRL14_b0_reserve2_START (0)
#define PMIC_BUCK0_CTRL14_b0_reserve2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b0_softime_sel : 2;
        unsigned char b0_sense_ibias_dmd : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_BUCK0_CTRL15_UNION;
#endif
#define PMIC_BUCK0_CTRL15_b0_softime_sel_START (0)
#define PMIC_BUCK0_CTRL15_b0_softime_sel_END (1)
#define PMIC_BUCK0_CTRL15_b0_sense_ibias_dmd_START (2)
#define PMIC_BUCK0_CTRL15_b0_sense_ibias_dmd_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b0_ocp_state : 1;
        unsigned char b0_ccm_state : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_BUCK0_RO0_UNION;
#endif
#define PMIC_BUCK0_RO0_b0_ocp_state_START (0)
#define PMIC_BUCK0_RO0_b0_ocp_state_END (0)
#define PMIC_BUCK0_RO0_b0_ccm_state_START (1)
#define PMIC_BUCK0_RO0_b0_ccm_state_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b1_adj_rlx : 4;
        unsigned char b1_adj_clx : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK1_CTRL0_UNION;
#endif
#define PMIC_BUCK1_CTRL0_b1_adj_rlx_START (0)
#define PMIC_BUCK1_CTRL0_b1_adj_rlx_END (3)
#define PMIC_BUCK1_CTRL0_b1_adj_clx_START (4)
#define PMIC_BUCK1_CTRL0_b1_adj_clx_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b1_dmd_blanktime_sel : 1;
        unsigned char b1_cot_rlx : 1;
        unsigned char b1_cmp_ibias : 4;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK1_CTRL1_UNION;
#endif
#define PMIC_BUCK1_CTRL1_b1_dmd_blanktime_sel_START (0)
#define PMIC_BUCK1_CTRL1_b1_dmd_blanktime_sel_END (0)
#define PMIC_BUCK1_CTRL1_b1_cot_rlx_START (1)
#define PMIC_BUCK1_CTRL1_b1_cot_rlx_END (1)
#define PMIC_BUCK1_CTRL1_b1_cmp_ibias_START (2)
#define PMIC_BUCK1_CTRL1_b1_cmp_ibias_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b1_dmd_sel_eco : 4;
        unsigned char b1_dmd_sel : 4;
    } reg;
} PMIC_BUCK1_CTRL2_UNION;
#endif
#define PMIC_BUCK1_CTRL2_b1_dmd_sel_eco_START (0)
#define PMIC_BUCK1_CTRL2_b1_dmd_sel_eco_END (3)
#define PMIC_BUCK1_CTRL2_b1_dmd_sel_START (4)
#define PMIC_BUCK1_CTRL2_b1_dmd_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b1_ea_eco_dis : 1;
        unsigned char b1_ea_clamp_eco_dis : 1;
        unsigned char b1_dt_sel : 1;
        unsigned char b1_dmd_type_sel : 1;
        unsigned char b1_dmd_ton : 3;
        unsigned char b1_dmd_shield_ton : 1;
    } reg;
} PMIC_BUCK1_CTRL3_UNION;
#endif
#define PMIC_BUCK1_CTRL3_b1_ea_eco_dis_START (0)
#define PMIC_BUCK1_CTRL3_b1_ea_eco_dis_END (0)
#define PMIC_BUCK1_CTRL3_b1_ea_clamp_eco_dis_START (1)
#define PMIC_BUCK1_CTRL3_b1_ea_clamp_eco_dis_END (1)
#define PMIC_BUCK1_CTRL3_b1_dt_sel_START (2)
#define PMIC_BUCK1_CTRL3_b1_dt_sel_END (2)
#define PMIC_BUCK1_CTRL3_b1_dmd_type_sel_START (3)
#define PMIC_BUCK1_CTRL3_b1_dmd_type_sel_END (3)
#define PMIC_BUCK1_CTRL3_b1_dmd_ton_START (4)
#define PMIC_BUCK1_CTRL3_b1_dmd_ton_END (6)
#define PMIC_BUCK1_CTRL3_b1_dmd_shield_ton_START (7)
#define PMIC_BUCK1_CTRL3_b1_dmd_shield_ton_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b1_lx_dt : 1;
        unsigned char b1_ibias_dmd_ctrl : 2;
        unsigned char b1_fix_ton : 1;
        unsigned char b1_fb_cap_sel : 1;
        unsigned char b1_eco_ibias_sel : 1;
        unsigned char b1_ea_ibias_sel : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK1_CTRL4_UNION;
#endif
#define PMIC_BUCK1_CTRL4_b1_lx_dt_START (0)
#define PMIC_BUCK1_CTRL4_b1_lx_dt_END (0)
#define PMIC_BUCK1_CTRL4_b1_ibias_dmd_ctrl_START (1)
#define PMIC_BUCK1_CTRL4_b1_ibias_dmd_ctrl_END (2)
#define PMIC_BUCK1_CTRL4_b1_fix_ton_START (3)
#define PMIC_BUCK1_CTRL4_b1_fix_ton_END (3)
#define PMIC_BUCK1_CTRL4_b1_fb_cap_sel_START (4)
#define PMIC_BUCK1_CTRL4_b1_fb_cap_sel_END (4)
#define PMIC_BUCK1_CTRL4_b1_eco_ibias_sel_START (5)
#define PMIC_BUCK1_CTRL4_b1_eco_ibias_sel_END (5)
#define PMIC_BUCK1_CTRL4_b1_ea_ibias_sel_START (6)
#define PMIC_BUCK1_CTRL4_b1_ea_ibias_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b1_ng_n_sel : 3;
        unsigned char b1_min_ton : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK1_CTRL5_UNION;
#endif
#define PMIC_BUCK1_CTRL5_b1_ng_n_sel_START (0)
#define PMIC_BUCK1_CTRL5_b1_ng_n_sel_END (2)
#define PMIC_BUCK1_CTRL5_b1_min_ton_START (3)
#define PMIC_BUCK1_CTRL5_b1_min_ton_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b1_ocp_cmp_ibias : 1;
        unsigned char b1_ntp_dt_sel : 2;
        unsigned char b1_ntp_dt_ctrl : 1;
        unsigned char b1_nonlinear_driver : 1;
        unsigned char b1_ng_p_sel : 3;
    } reg;
} PMIC_BUCK1_CTRL6_UNION;
#endif
#define PMIC_BUCK1_CTRL6_b1_ocp_cmp_ibias_START (0)
#define PMIC_BUCK1_CTRL6_b1_ocp_cmp_ibias_END (0)
#define PMIC_BUCK1_CTRL6_b1_ntp_dt_sel_START (1)
#define PMIC_BUCK1_CTRL6_b1_ntp_dt_sel_END (2)
#define PMIC_BUCK1_CTRL6_b1_ntp_dt_ctrl_START (3)
#define PMIC_BUCK1_CTRL6_b1_ntp_dt_ctrl_END (3)
#define PMIC_BUCK1_CTRL6_b1_nonlinear_driver_START (4)
#define PMIC_BUCK1_CTRL6_b1_nonlinear_driver_END (4)
#define PMIC_BUCK1_CTRL6_b1_ng_p_sel_START (5)
#define PMIC_BUCK1_CTRL6_b1_ng_p_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b1_ocpibias_sel : 2;
        unsigned char b1_ocp_toff : 2;
        unsigned char b1_ocp_sel : 3;
        unsigned char b1_ocp_delay : 1;
    } reg;
} PMIC_BUCK1_CTRL7_UNION;
#endif
#define PMIC_BUCK1_CTRL7_b1_ocpibias_sel_START (0)
#define PMIC_BUCK1_CTRL7_b1_ocpibias_sel_END (1)
#define PMIC_BUCK1_CTRL7_b1_ocp_toff_START (2)
#define PMIC_BUCK1_CTRL7_b1_ocp_toff_END (3)
#define PMIC_BUCK1_CTRL7_b1_ocp_sel_START (4)
#define PMIC_BUCK1_CTRL7_b1_ocp_sel_END (6)
#define PMIC_BUCK1_CTRL7_b1_ocp_delay_START (7)
#define PMIC_BUCK1_CTRL7_b1_ocp_delay_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b1_pg_p_sel : 3;
        unsigned char b1_pg_n_sel : 3;
        unsigned char b1_offres_shut : 1;
        unsigned char b1_offres_sel : 1;
    } reg;
} PMIC_BUCK1_CTRL8_UNION;
#endif
#define PMIC_BUCK1_CTRL8_b1_pg_p_sel_START (0)
#define PMIC_BUCK1_CTRL8_b1_pg_p_sel_END (2)
#define PMIC_BUCK1_CTRL8_b1_pg_n_sel_START (3)
#define PMIC_BUCK1_CTRL8_b1_pg_n_sel_END (5)
#define PMIC_BUCK1_CTRL8_b1_offres_shut_START (6)
#define PMIC_BUCK1_CTRL8_b1_offres_shut_END (6)
#define PMIC_BUCK1_CTRL8_b1_offres_sel_START (7)
#define PMIC_BUCK1_CTRL8_b1_offres_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b1_reg_dr : 3;
        unsigned char b1_ramp_ton_ctrl : 1;
        unsigned char b1_ramp_prebias : 1;
        unsigned char b1_ptn_dt_sel : 2;
        unsigned char b1_ptn_dt_ctrl : 1;
    } reg;
} PMIC_BUCK1_CTRL9_UNION;
#endif
#define PMIC_BUCK1_CTRL9_b1_reg_dr_START (0)
#define PMIC_BUCK1_CTRL9_b1_reg_dr_END (2)
#define PMIC_BUCK1_CTRL9_b1_ramp_ton_ctrl_START (3)
#define PMIC_BUCK1_CTRL9_b1_ramp_ton_ctrl_END (3)
#define PMIC_BUCK1_CTRL9_b1_ramp_prebias_START (4)
#define PMIC_BUCK1_CTRL9_b1_ramp_prebias_END (4)
#define PMIC_BUCK1_CTRL9_b1_ptn_dt_sel_START (5)
#define PMIC_BUCK1_CTRL9_b1_ptn_dt_sel_END (6)
#define PMIC_BUCK1_CTRL9_b1_ptn_dt_ctrl_START (7)
#define PMIC_BUCK1_CTRL9_b1_ptn_dt_ctrl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b1_reg_r : 2;
        unsigned char b1_reg_op_c : 1;
        unsigned char b1_reg_ibias : 1;
        unsigned char b1_reg_en : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK1_CTRL10_UNION;
#endif
#define PMIC_BUCK1_CTRL10_b1_reg_r_START (0)
#define PMIC_BUCK1_CTRL10_b1_reg_r_END (1)
#define PMIC_BUCK1_CTRL10_b1_reg_op_c_START (2)
#define PMIC_BUCK1_CTRL10_b1_reg_op_c_END (2)
#define PMIC_BUCK1_CTRL10_b1_reg_ibias_START (3)
#define PMIC_BUCK1_CTRL10_b1_reg_ibias_END (3)
#define PMIC_BUCK1_CTRL10_b1_reg_en_START (4)
#define PMIC_BUCK1_CTRL10_b1_reg_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b1_reserve0 : 8;
    } reg;
} PMIC_BUCK1_CTRL11_UNION;
#endif
#define PMIC_BUCK1_CTRL11_b1_reserve0_START (0)
#define PMIC_BUCK1_CTRL11_b1_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b1_reserve1 : 8;
    } reg;
} PMIC_BUCK1_CTRL12_UNION;
#endif
#define PMIC_BUCK1_CTRL12_b1_reserve1_START (0)
#define PMIC_BUCK1_CTRL12_b1_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b1_reserve2 : 8;
    } reg;
} PMIC_BUCK1_CTRL13_UNION;
#endif
#define PMIC_BUCK1_CTRL13_b1_reserve2_START (0)
#define PMIC_BUCK1_CTRL13_b1_reserve2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b1_softime_sel : 2;
        unsigned char b1_sense_ibias_dmd : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_BUCK1_CTRL14_UNION;
#endif
#define PMIC_BUCK1_CTRL14_b1_softime_sel_START (0)
#define PMIC_BUCK1_CTRL14_b1_softime_sel_END (1)
#define PMIC_BUCK1_CTRL14_b1_sense_ibias_dmd_START (2)
#define PMIC_BUCK1_CTRL14_b1_sense_ibias_dmd_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b1_ocp_state : 1;
        unsigned char b1_ccm_state : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_BUCK1_RO0_UNION;
#endif
#define PMIC_BUCK1_RO0_b1_ocp_state_START (0)
#define PMIC_BUCK1_RO0_b1_ocp_state_END (0)
#define PMIC_BUCK1_RO0_b1_ccm_state_START (1)
#define PMIC_BUCK1_RO0_b1_ccm_state_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b2_adj_rlx : 4;
        unsigned char b2_adj_clx : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK2_CTRL0_UNION;
#endif
#define PMIC_BUCK2_CTRL0_b2_adj_rlx_START (0)
#define PMIC_BUCK2_CTRL0_b2_adj_rlx_END (3)
#define PMIC_BUCK2_CTRL0_b2_adj_clx_START (4)
#define PMIC_BUCK2_CTRL0_b2_adj_clx_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b2_dmd_blanktime_sel : 1;
        unsigned char b2_cot_rlx : 1;
        unsigned char b2_cmp_ibias : 4;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK2_CTRL1_UNION;
#endif
#define PMIC_BUCK2_CTRL1_b2_dmd_blanktime_sel_START (0)
#define PMIC_BUCK2_CTRL1_b2_dmd_blanktime_sel_END (0)
#define PMIC_BUCK2_CTRL1_b2_cot_rlx_START (1)
#define PMIC_BUCK2_CTRL1_b2_cot_rlx_END (1)
#define PMIC_BUCK2_CTRL1_b2_cmp_ibias_START (2)
#define PMIC_BUCK2_CTRL1_b2_cmp_ibias_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b2_dmd_sel_eco : 4;
        unsigned char b2_dmd_sel : 4;
    } reg;
} PMIC_BUCK2_CTRL2_UNION;
#endif
#define PMIC_BUCK2_CTRL2_b2_dmd_sel_eco_START (0)
#define PMIC_BUCK2_CTRL2_b2_dmd_sel_eco_END (3)
#define PMIC_BUCK2_CTRL2_b2_dmd_sel_START (4)
#define PMIC_BUCK2_CTRL2_b2_dmd_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b2_ea_eco_dis : 1;
        unsigned char b2_ea_clamp_eco_dis : 1;
        unsigned char b2_dt_sel : 1;
        unsigned char b2_dmd_type_sel : 1;
        unsigned char b2_dmd_ton : 3;
        unsigned char b2_dmd_shield_ton : 1;
    } reg;
} PMIC_BUCK2_CTRL3_UNION;
#endif
#define PMIC_BUCK2_CTRL3_b2_ea_eco_dis_START (0)
#define PMIC_BUCK2_CTRL3_b2_ea_eco_dis_END (0)
#define PMIC_BUCK2_CTRL3_b2_ea_clamp_eco_dis_START (1)
#define PMIC_BUCK2_CTRL3_b2_ea_clamp_eco_dis_END (1)
#define PMIC_BUCK2_CTRL3_b2_dt_sel_START (2)
#define PMIC_BUCK2_CTRL3_b2_dt_sel_END (2)
#define PMIC_BUCK2_CTRL3_b2_dmd_type_sel_START (3)
#define PMIC_BUCK2_CTRL3_b2_dmd_type_sel_END (3)
#define PMIC_BUCK2_CTRL3_b2_dmd_ton_START (4)
#define PMIC_BUCK2_CTRL3_b2_dmd_ton_END (6)
#define PMIC_BUCK2_CTRL3_b2_dmd_shield_ton_START (7)
#define PMIC_BUCK2_CTRL3_b2_dmd_shield_ton_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b2_lx_dt : 1;
        unsigned char b2_ibias_dmd_ctrl : 2;
        unsigned char b2_fix_ton : 1;
        unsigned char b2_fb_cap_sel : 1;
        unsigned char b2_eco_ibias_sel : 1;
        unsigned char b2_ea_ibias_sel : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK2_CTRL4_UNION;
#endif
#define PMIC_BUCK2_CTRL4_b2_lx_dt_START (0)
#define PMIC_BUCK2_CTRL4_b2_lx_dt_END (0)
#define PMIC_BUCK2_CTRL4_b2_ibias_dmd_ctrl_START (1)
#define PMIC_BUCK2_CTRL4_b2_ibias_dmd_ctrl_END (2)
#define PMIC_BUCK2_CTRL4_b2_fix_ton_START (3)
#define PMIC_BUCK2_CTRL4_b2_fix_ton_END (3)
#define PMIC_BUCK2_CTRL4_b2_fb_cap_sel_START (4)
#define PMIC_BUCK2_CTRL4_b2_fb_cap_sel_END (4)
#define PMIC_BUCK2_CTRL4_b2_eco_ibias_sel_START (5)
#define PMIC_BUCK2_CTRL4_b2_eco_ibias_sel_END (5)
#define PMIC_BUCK2_CTRL4_b2_ea_ibias_sel_START (6)
#define PMIC_BUCK2_CTRL4_b2_ea_ibias_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b2_ng_n_sel : 3;
        unsigned char b2_min_ton : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK2_CTRL5_UNION;
#endif
#define PMIC_BUCK2_CTRL5_b2_ng_n_sel_START (0)
#define PMIC_BUCK2_CTRL5_b2_ng_n_sel_END (2)
#define PMIC_BUCK2_CTRL5_b2_min_ton_START (3)
#define PMIC_BUCK2_CTRL5_b2_min_ton_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b2_ocp_cmp_ibias : 1;
        unsigned char b2_ntp_dt_sel : 2;
        unsigned char b2_ntp_dt_ctrl : 1;
        unsigned char b2_nonlinear_driver : 1;
        unsigned char b2_ng_p_sel : 3;
    } reg;
} PMIC_BUCK2_CTRL6_UNION;
#endif
#define PMIC_BUCK2_CTRL6_b2_ocp_cmp_ibias_START (0)
#define PMIC_BUCK2_CTRL6_b2_ocp_cmp_ibias_END (0)
#define PMIC_BUCK2_CTRL6_b2_ntp_dt_sel_START (1)
#define PMIC_BUCK2_CTRL6_b2_ntp_dt_sel_END (2)
#define PMIC_BUCK2_CTRL6_b2_ntp_dt_ctrl_START (3)
#define PMIC_BUCK2_CTRL6_b2_ntp_dt_ctrl_END (3)
#define PMIC_BUCK2_CTRL6_b2_nonlinear_driver_START (4)
#define PMIC_BUCK2_CTRL6_b2_nonlinear_driver_END (4)
#define PMIC_BUCK2_CTRL6_b2_ng_p_sel_START (5)
#define PMIC_BUCK2_CTRL6_b2_ng_p_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b2_ocpibias_sel : 2;
        unsigned char b2_ocp_toff : 2;
        unsigned char b2_ocp_sel : 3;
        unsigned char b2_ocp_delay : 1;
    } reg;
} PMIC_BUCK2_CTRL7_UNION;
#endif
#define PMIC_BUCK2_CTRL7_b2_ocpibias_sel_START (0)
#define PMIC_BUCK2_CTRL7_b2_ocpibias_sel_END (1)
#define PMIC_BUCK2_CTRL7_b2_ocp_toff_START (2)
#define PMIC_BUCK2_CTRL7_b2_ocp_toff_END (3)
#define PMIC_BUCK2_CTRL7_b2_ocp_sel_START (4)
#define PMIC_BUCK2_CTRL7_b2_ocp_sel_END (6)
#define PMIC_BUCK2_CTRL7_b2_ocp_delay_START (7)
#define PMIC_BUCK2_CTRL7_b2_ocp_delay_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b2_pg_p_sel : 3;
        unsigned char b2_pg_n_sel : 3;
        unsigned char b2_offres_shut : 1;
        unsigned char b2_offres_sel : 1;
    } reg;
} PMIC_BUCK2_CTRL8_UNION;
#endif
#define PMIC_BUCK2_CTRL8_b2_pg_p_sel_START (0)
#define PMIC_BUCK2_CTRL8_b2_pg_p_sel_END (2)
#define PMIC_BUCK2_CTRL8_b2_pg_n_sel_START (3)
#define PMIC_BUCK2_CTRL8_b2_pg_n_sel_END (5)
#define PMIC_BUCK2_CTRL8_b2_offres_shut_START (6)
#define PMIC_BUCK2_CTRL8_b2_offres_shut_END (6)
#define PMIC_BUCK2_CTRL8_b2_offres_sel_START (7)
#define PMIC_BUCK2_CTRL8_b2_offres_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b2_reg_dr : 3;
        unsigned char b2_ramp_ton_ctrl : 1;
        unsigned char b2_ramp_prebias : 1;
        unsigned char b2_ptn_dt_sel : 2;
        unsigned char b2_ptn_dt_ctrl : 1;
    } reg;
} PMIC_BUCK2_CTRL9_UNION;
#endif
#define PMIC_BUCK2_CTRL9_b2_reg_dr_START (0)
#define PMIC_BUCK2_CTRL9_b2_reg_dr_END (2)
#define PMIC_BUCK2_CTRL9_b2_ramp_ton_ctrl_START (3)
#define PMIC_BUCK2_CTRL9_b2_ramp_ton_ctrl_END (3)
#define PMIC_BUCK2_CTRL9_b2_ramp_prebias_START (4)
#define PMIC_BUCK2_CTRL9_b2_ramp_prebias_END (4)
#define PMIC_BUCK2_CTRL9_b2_ptn_dt_sel_START (5)
#define PMIC_BUCK2_CTRL9_b2_ptn_dt_sel_END (6)
#define PMIC_BUCK2_CTRL9_b2_ptn_dt_ctrl_START (7)
#define PMIC_BUCK2_CTRL9_b2_ptn_dt_ctrl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b2_reg_r : 2;
        unsigned char b2_reg_op_c : 1;
        unsigned char b2_reg_ibias : 1;
        unsigned char b2_reg_en : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK2_CTRL10_UNION;
#endif
#define PMIC_BUCK2_CTRL10_b2_reg_r_START (0)
#define PMIC_BUCK2_CTRL10_b2_reg_r_END (1)
#define PMIC_BUCK2_CTRL10_b2_reg_op_c_START (2)
#define PMIC_BUCK2_CTRL10_b2_reg_op_c_END (2)
#define PMIC_BUCK2_CTRL10_b2_reg_ibias_START (3)
#define PMIC_BUCK2_CTRL10_b2_reg_ibias_END (3)
#define PMIC_BUCK2_CTRL10_b2_reg_en_START (4)
#define PMIC_BUCK2_CTRL10_b2_reg_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b2_reserve0 : 8;
    } reg;
} PMIC_BUCK2_CTRL11_UNION;
#endif
#define PMIC_BUCK2_CTRL11_b2_reserve0_START (0)
#define PMIC_BUCK2_CTRL11_b2_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b2_reserve1 : 8;
    } reg;
} PMIC_BUCK2_CTRL12_UNION;
#endif
#define PMIC_BUCK2_CTRL12_b2_reserve1_START (0)
#define PMIC_BUCK2_CTRL12_b2_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b2_reserve2 : 8;
    } reg;
} PMIC_BUCK2_CTRL13_UNION;
#endif
#define PMIC_BUCK2_CTRL13_b2_reserve2_START (0)
#define PMIC_BUCK2_CTRL13_b2_reserve2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b2_softime_sel : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_BUCK2_CTRL14_UNION;
#endif
#define PMIC_BUCK2_CTRL14_b2_softime_sel_START (0)
#define PMIC_BUCK2_CTRL14_b2_softime_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b2_ocp_state : 1;
        unsigned char b2_ccm_state : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_BUCK2_RO0_UNION;
#endif
#define PMIC_BUCK2_RO0_b2_ocp_state_START (0)
#define PMIC_BUCK2_RO0_b2_ocp_state_END (0)
#define PMIC_BUCK2_RO0_b2_ccm_state_START (1)
#define PMIC_BUCK2_RO0_b2_ccm_state_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b3_adj_rlx : 4;
        unsigned char b3_adj_clx : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK3_CTRL0_UNION;
#endif
#define PMIC_BUCK3_CTRL0_b3_adj_rlx_START (0)
#define PMIC_BUCK3_CTRL0_b3_adj_rlx_END (3)
#define PMIC_BUCK3_CTRL0_b3_adj_clx_START (4)
#define PMIC_BUCK3_CTRL0_b3_adj_clx_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b3_dmd_blanktime_sel : 1;
        unsigned char b3_cot_rlx : 1;
        unsigned char b3_cmp_ibias : 4;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK3_CTRL1_UNION;
#endif
#define PMIC_BUCK3_CTRL1_b3_dmd_blanktime_sel_START (0)
#define PMIC_BUCK3_CTRL1_b3_dmd_blanktime_sel_END (0)
#define PMIC_BUCK3_CTRL1_b3_cot_rlx_START (1)
#define PMIC_BUCK3_CTRL1_b3_cot_rlx_END (1)
#define PMIC_BUCK3_CTRL1_b3_cmp_ibias_START (2)
#define PMIC_BUCK3_CTRL1_b3_cmp_ibias_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b3_dmd_sel_eco : 4;
        unsigned char b3_dmd_sel : 4;
    } reg;
} PMIC_BUCK3_CTRL2_UNION;
#endif
#define PMIC_BUCK3_CTRL2_b3_dmd_sel_eco_START (0)
#define PMIC_BUCK3_CTRL2_b3_dmd_sel_eco_END (3)
#define PMIC_BUCK3_CTRL2_b3_dmd_sel_START (4)
#define PMIC_BUCK3_CTRL2_b3_dmd_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b3_ea_eco_dis : 1;
        unsigned char b3_ea_clamp_eco_dis : 1;
        unsigned char b3_dt_sel : 1;
        unsigned char b3_dmd_type_sel : 1;
        unsigned char b3_dmd_ton : 3;
        unsigned char b3_dmd_shield_ton : 1;
    } reg;
} PMIC_BUCK3_CTRL3_UNION;
#endif
#define PMIC_BUCK3_CTRL3_b3_ea_eco_dis_START (0)
#define PMIC_BUCK3_CTRL3_b3_ea_eco_dis_END (0)
#define PMIC_BUCK3_CTRL3_b3_ea_clamp_eco_dis_START (1)
#define PMIC_BUCK3_CTRL3_b3_ea_clamp_eco_dis_END (1)
#define PMIC_BUCK3_CTRL3_b3_dt_sel_START (2)
#define PMIC_BUCK3_CTRL3_b3_dt_sel_END (2)
#define PMIC_BUCK3_CTRL3_b3_dmd_type_sel_START (3)
#define PMIC_BUCK3_CTRL3_b3_dmd_type_sel_END (3)
#define PMIC_BUCK3_CTRL3_b3_dmd_ton_START (4)
#define PMIC_BUCK3_CTRL3_b3_dmd_ton_END (6)
#define PMIC_BUCK3_CTRL3_b3_dmd_shield_ton_START (7)
#define PMIC_BUCK3_CTRL3_b3_dmd_shield_ton_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b3_lx_dt : 1;
        unsigned char b3_ibias_dmd_ctrl : 2;
        unsigned char b3_fix_ton : 1;
        unsigned char b3_fb_cap_sel : 1;
        unsigned char b3_eco_ibias_sel : 1;
        unsigned char b3_ea_ibias_sel : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK3_CTRL4_UNION;
#endif
#define PMIC_BUCK3_CTRL4_b3_lx_dt_START (0)
#define PMIC_BUCK3_CTRL4_b3_lx_dt_END (0)
#define PMIC_BUCK3_CTRL4_b3_ibias_dmd_ctrl_START (1)
#define PMIC_BUCK3_CTRL4_b3_ibias_dmd_ctrl_END (2)
#define PMIC_BUCK3_CTRL4_b3_fix_ton_START (3)
#define PMIC_BUCK3_CTRL4_b3_fix_ton_END (3)
#define PMIC_BUCK3_CTRL4_b3_fb_cap_sel_START (4)
#define PMIC_BUCK3_CTRL4_b3_fb_cap_sel_END (4)
#define PMIC_BUCK3_CTRL4_b3_eco_ibias_sel_START (5)
#define PMIC_BUCK3_CTRL4_b3_eco_ibias_sel_END (5)
#define PMIC_BUCK3_CTRL4_b3_ea_ibias_sel_START (6)
#define PMIC_BUCK3_CTRL4_b3_ea_ibias_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b3_ng_n_sel : 3;
        unsigned char b3_min_ton : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK3_CTRL5_UNION;
#endif
#define PMIC_BUCK3_CTRL5_b3_ng_n_sel_START (0)
#define PMIC_BUCK3_CTRL5_b3_ng_n_sel_END (2)
#define PMIC_BUCK3_CTRL5_b3_min_ton_START (3)
#define PMIC_BUCK3_CTRL5_b3_min_ton_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b3_ocp_cmp_ibias : 1;
        unsigned char b3_ntp_dt_sel : 2;
        unsigned char b3_ntp_dt_ctrl : 1;
        unsigned char b3_nonlinear_driver : 1;
        unsigned char b3_ng_p_sel : 3;
    } reg;
} PMIC_BUCK3_CTRL6_UNION;
#endif
#define PMIC_BUCK3_CTRL6_b3_ocp_cmp_ibias_START (0)
#define PMIC_BUCK3_CTRL6_b3_ocp_cmp_ibias_END (0)
#define PMIC_BUCK3_CTRL6_b3_ntp_dt_sel_START (1)
#define PMIC_BUCK3_CTRL6_b3_ntp_dt_sel_END (2)
#define PMIC_BUCK3_CTRL6_b3_ntp_dt_ctrl_START (3)
#define PMIC_BUCK3_CTRL6_b3_ntp_dt_ctrl_END (3)
#define PMIC_BUCK3_CTRL6_b3_nonlinear_driver_START (4)
#define PMIC_BUCK3_CTRL6_b3_nonlinear_driver_END (4)
#define PMIC_BUCK3_CTRL6_b3_ng_p_sel_START (5)
#define PMIC_BUCK3_CTRL6_b3_ng_p_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b3_ocpibias_sel : 2;
        unsigned char b3_ocp_toff : 2;
        unsigned char b3_ocp_sel : 3;
        unsigned char b3_ocp_delay : 1;
    } reg;
} PMIC_BUCK3_CTRL7_UNION;
#endif
#define PMIC_BUCK3_CTRL7_b3_ocpibias_sel_START (0)
#define PMIC_BUCK3_CTRL7_b3_ocpibias_sel_END (1)
#define PMIC_BUCK3_CTRL7_b3_ocp_toff_START (2)
#define PMIC_BUCK3_CTRL7_b3_ocp_toff_END (3)
#define PMIC_BUCK3_CTRL7_b3_ocp_sel_START (4)
#define PMIC_BUCK3_CTRL7_b3_ocp_sel_END (6)
#define PMIC_BUCK3_CTRL7_b3_ocp_delay_START (7)
#define PMIC_BUCK3_CTRL7_b3_ocp_delay_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b3_pg_p_sel : 3;
        unsigned char b3_pg_n_sel : 3;
        unsigned char b3_offres_shut : 1;
        unsigned char b3_offres_sel : 1;
    } reg;
} PMIC_BUCK3_CTRL8_UNION;
#endif
#define PMIC_BUCK3_CTRL8_b3_pg_p_sel_START (0)
#define PMIC_BUCK3_CTRL8_b3_pg_p_sel_END (2)
#define PMIC_BUCK3_CTRL8_b3_pg_n_sel_START (3)
#define PMIC_BUCK3_CTRL8_b3_pg_n_sel_END (5)
#define PMIC_BUCK3_CTRL8_b3_offres_shut_START (6)
#define PMIC_BUCK3_CTRL8_b3_offres_shut_END (6)
#define PMIC_BUCK3_CTRL8_b3_offres_sel_START (7)
#define PMIC_BUCK3_CTRL8_b3_offres_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b3_reg_dr : 3;
        unsigned char b3_ramp_ton_ctrl : 1;
        unsigned char b3_ramp_prebias : 1;
        unsigned char b3_ptn_dt_sel : 2;
        unsigned char b3_ptn_dt_ctrl : 1;
    } reg;
} PMIC_BUCK3_CTRL9_UNION;
#endif
#define PMIC_BUCK3_CTRL9_b3_reg_dr_START (0)
#define PMIC_BUCK3_CTRL9_b3_reg_dr_END (2)
#define PMIC_BUCK3_CTRL9_b3_ramp_ton_ctrl_START (3)
#define PMIC_BUCK3_CTRL9_b3_ramp_ton_ctrl_END (3)
#define PMIC_BUCK3_CTRL9_b3_ramp_prebias_START (4)
#define PMIC_BUCK3_CTRL9_b3_ramp_prebias_END (4)
#define PMIC_BUCK3_CTRL9_b3_ptn_dt_sel_START (5)
#define PMIC_BUCK3_CTRL9_b3_ptn_dt_sel_END (6)
#define PMIC_BUCK3_CTRL9_b3_ptn_dt_ctrl_START (7)
#define PMIC_BUCK3_CTRL9_b3_ptn_dt_ctrl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b3_reg_r : 2;
        unsigned char b3_reg_op_c : 1;
        unsigned char b3_reg_ibias : 1;
        unsigned char b3_reg_en : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK3_CTRL10_UNION;
#endif
#define PMIC_BUCK3_CTRL10_b3_reg_r_START (0)
#define PMIC_BUCK3_CTRL10_b3_reg_r_END (1)
#define PMIC_BUCK3_CTRL10_b3_reg_op_c_START (2)
#define PMIC_BUCK3_CTRL10_b3_reg_op_c_END (2)
#define PMIC_BUCK3_CTRL10_b3_reg_ibias_START (3)
#define PMIC_BUCK3_CTRL10_b3_reg_ibias_END (3)
#define PMIC_BUCK3_CTRL10_b3_reg_en_START (4)
#define PMIC_BUCK3_CTRL10_b3_reg_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b3_reserve0 : 8;
    } reg;
} PMIC_BUCK3_CTRL11_UNION;
#endif
#define PMIC_BUCK3_CTRL11_b3_reserve0_START (0)
#define PMIC_BUCK3_CTRL11_b3_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b3_reserve1 : 8;
    } reg;
} PMIC_BUCK3_CTRL12_UNION;
#endif
#define PMIC_BUCK3_CTRL12_b3_reserve1_START (0)
#define PMIC_BUCK3_CTRL12_b3_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b3_reserve2 : 8;
    } reg;
} PMIC_BUCK3_CTRL13_UNION;
#endif
#define PMIC_BUCK3_CTRL13_b3_reserve2_START (0)
#define PMIC_BUCK3_CTRL13_b3_reserve2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b3_softime_sel : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_BUCK3_CTRL14_UNION;
#endif
#define PMIC_BUCK3_CTRL14_b3_softime_sel_START (0)
#define PMIC_BUCK3_CTRL14_b3_softime_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b3_ocp_state : 1;
        unsigned char b3_ccm_state : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_BUCK3_RO0_UNION;
#endif
#define PMIC_BUCK3_RO0_b3_ocp_state_START (0)
#define PMIC_BUCK3_RO0_b3_ocp_state_END (0)
#define PMIC_BUCK3_RO0_b3_ccm_state_START (1)
#define PMIC_BUCK3_RO0_b3_ccm_state_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_bw_sel : 1;
        unsigned char b40_buffer_bias : 2;
        unsigned char b40_b40_ea_r : 3;
        unsigned char b40_avg_curr_filter_sel : 2;
    } reg;
} PMIC_B401_CTRL0_UNION;
#endif
#define PMIC_B401_CTRL0_b40_bw_sel_START (0)
#define PMIC_B401_CTRL0_b40_bw_sel_END (0)
#define PMIC_B401_CTRL0_b40_buffer_bias_START (1)
#define PMIC_B401_CTRL0_b40_buffer_bias_END (2)
#define PMIC_B401_CTRL0_b40_b40_ea_r_START (3)
#define PMIC_B401_CTRL0_b40_b40_ea_r_END (5)
#define PMIC_B401_CTRL0_b40_avg_curr_filter_sel_START (6)
#define PMIC_B401_CTRL0_b40_avg_curr_filter_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_c1_sel : 5;
        unsigned char reserved : 3;
    } reg;
} PMIC_B401_CTRL1_UNION;
#endif
#define PMIC_B401_CTRL1_b40_c1_sel_START (0)
#define PMIC_B401_CTRL1_b40_c1_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_c2_sel : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_B401_CTRL2_UNION;
#endif
#define PMIC_B401_CTRL2_b40_c2_sel_START (0)
#define PMIC_B401_CTRL2_b40_c2_sel_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_clk_chopper_sel : 2;
        unsigned char b40_c3_sel : 5;
        unsigned char reserved : 1;
    } reg;
} PMIC_B401_CTRL3_UNION;
#endif
#define PMIC_B401_CTRL3_b40_clk_chopper_sel_START (0)
#define PMIC_B401_CTRL3_b40_clk_chopper_sel_END (1)
#define PMIC_B401_CTRL3_b40_c3_sel_START (2)
#define PMIC_B401_CTRL3_b40_c3_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_clock_atest : 1;
        unsigned char b40_clk9m6_pulse_sel : 2;
        unsigned char reserved : 5;
    } reg;
} PMIC_B401_CTRL4_UNION;
#endif
#define PMIC_B401_CTRL4_b40_clock_atest_START (0)
#define PMIC_B401_CTRL4_b40_clock_atest_END (0)
#define PMIC_B401_CTRL4_b40_clk9m6_pulse_sel_START (1)
#define PMIC_B401_CTRL4_b40_clk9m6_pulse_sel_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_clock_sel : 8;
    } reg;
} PMIC_B401_CTRL5_UNION;
#endif
#define PMIC_B401_CTRL5_b40_clock_sel_START (0)
#define PMIC_B401_CTRL5_b40_clock_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_clp_reg_idrp : 4;
        unsigned char b40_clp_ref_sel : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_B401_CTRL6_UNION;
#endif
#define PMIC_B401_CTRL6_b40_clp_reg_idrp_START (0)
#define PMIC_B401_CTRL6_b40_clp_reg_idrp_END (3)
#define PMIC_B401_CTRL6_b40_clp_ref_sel_START (4)
#define PMIC_B401_CTRL6_b40_clp_ref_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_cot_cmphys : 1;
        unsigned char b40_cot_atest : 1;
        unsigned char b40_code_mode_sel : 1;
        unsigned char b40_cmp_atest : 1;
        unsigned char b40_clpeco_bias : 4;
    } reg;
} PMIC_B401_CTRL7_UNION;
#endif
#define PMIC_B401_CTRL7_b40_cot_cmphys_START (0)
#define PMIC_B401_CTRL7_b40_cot_cmphys_END (0)
#define PMIC_B401_CTRL7_b40_cot_atest_START (1)
#define PMIC_B401_CTRL7_b40_cot_atest_END (1)
#define PMIC_B401_CTRL7_b40_code_mode_sel_START (2)
#define PMIC_B401_CTRL7_b40_code_mode_sel_END (2)
#define PMIC_B401_CTRL7_b40_cmp_atest_START (3)
#define PMIC_B401_CTRL7_b40_cmp_atest_END (3)
#define PMIC_B401_CTRL7_b40_clpeco_bias_START (4)
#define PMIC_B401_CTRL7_b40_clpeco_bias_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_cot_sel : 1;
        unsigned char b40_cot_sche_neco : 1;
        unsigned char b40_cot_sche_eco : 1;
        unsigned char b40_cot_res_sel : 3;
        unsigned char b40_cot_iboost_sel : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_B401_CTRL8_UNION;
#endif
#define PMIC_B401_CTRL8_b40_cot_sel_START (0)
#define PMIC_B401_CTRL8_b40_cot_sel_END (0)
#define PMIC_B401_CTRL8_b40_cot_sche_neco_START (1)
#define PMIC_B401_CTRL8_b40_cot_sche_neco_END (1)
#define PMIC_B401_CTRL8_b40_cot_sche_eco_START (2)
#define PMIC_B401_CTRL8_b40_cot_sche_eco_END (2)
#define PMIC_B401_CTRL8_b40_cot_res_sel_START (3)
#define PMIC_B401_CTRL8_b40_cot_res_sel_END (5)
#define PMIC_B401_CTRL8_b40_cot_iboost_sel_START (6)
#define PMIC_B401_CTRL8_b40_cot_iboost_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_cot_ton_ecoa : 8;
    } reg;
} PMIC_B401_CTRL9_UNION;
#endif
#define PMIC_B401_CTRL9_b40_cot_ton_ecoa_START (0)
#define PMIC_B401_CTRL9_b40_cot_ton_ecoa_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_cot_ton_isen_hys : 2;
        unsigned char b40_cot_ton_isen_bias : 2;
        unsigned char b40_cot_ton_ecob : 3;
        unsigned char reserved : 1;
    } reg;
} PMIC_B401_CTRL10_UNION;
#endif
#define PMIC_B401_CTRL10_b40_cot_ton_isen_hys_START (0)
#define PMIC_B401_CTRL10_b40_cot_ton_isen_hys_END (1)
#define PMIC_B401_CTRL10_b40_cot_ton_isen_bias_START (2)
#define PMIC_B401_CTRL10_b40_cot_ton_isen_bias_END (3)
#define PMIC_B401_CTRL10_b40_cot_ton_ecob_START (4)
#define PMIC_B401_CTRL10_b40_cot_ton_ecob_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_cot_ton_necoa : 8;
    } reg;
} PMIC_B401_CTRL11_UNION;
#endif
#define PMIC_B401_CTRL11_b40_cot_ton_necoa_START (0)
#define PMIC_B401_CTRL11_b40_cot_ton_necoa_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_crtshare_bw_sel : 2;
        unsigned char b40_crtshare_atest : 1;
        unsigned char b40_cot_ton_necob : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_B401_CTRL12_UNION;
#endif
#define PMIC_B401_CTRL12_b40_crtshare_bw_sel_START (0)
#define PMIC_B401_CTRL12_b40_crtshare_bw_sel_END (1)
#define PMIC_B401_CTRL12_b40_crtshare_atest_START (2)
#define PMIC_B401_CTRL12_b40_crtshare_atest_END (2)
#define PMIC_B401_CTRL12_b40_cot_ton_necob_START (3)
#define PMIC_B401_CTRL12_b40_cot_ton_necob_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_cs_test_sel : 1;
        unsigned char b40_crtshare_trim : 1;
        unsigned char b40_crtshare_filter_r_sel : 1;
        unsigned char b40_crtshare_clp_sel : 4;
        unsigned char reserved : 1;
    } reg;
} PMIC_B401_CTRL13_UNION;
#endif
#define PMIC_B401_CTRL13_b40_cs_test_sel_START (0)
#define PMIC_B401_CTRL13_b40_cs_test_sel_END (0)
#define PMIC_B401_CTRL13_b40_crtshare_trim_START (1)
#define PMIC_B401_CTRL13_b40_crtshare_trim_END (1)
#define PMIC_B401_CTRL13_b40_crtshare_filter_r_sel_START (2)
#define PMIC_B401_CTRL13_b40_crtshare_filter_r_sel_END (2)
#define PMIC_B401_CTRL13_b40_crtshare_clp_sel_START (3)
#define PMIC_B401_CTRL13_b40_crtshare_clp_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_dbias : 3;
        unsigned char b40_ctlogic_smph : 1;
        unsigned char b40_ctlogic_sgph : 1;
        unsigned char b40_ctlogic_modsw : 1;
        unsigned char b40_ctlogic_crson_td : 2;
    } reg;
} PMIC_B401_CTRL14_UNION;
#endif
#define PMIC_B401_CTRL14_b40_dbias_START (0)
#define PMIC_B401_CTRL14_b40_dbias_END (2)
#define PMIC_B401_CTRL14_b40_ctlogic_smph_START (3)
#define PMIC_B401_CTRL14_b40_ctlogic_smph_END (3)
#define PMIC_B401_CTRL14_b40_ctlogic_sgph_START (4)
#define PMIC_B401_CTRL14_b40_ctlogic_sgph_END (4)
#define PMIC_B401_CTRL14_b40_ctlogic_modsw_START (5)
#define PMIC_B401_CTRL14_b40_ctlogic_modsw_END (5)
#define PMIC_B401_CTRL14_b40_ctlogic_crson_td_START (6)
#define PMIC_B401_CTRL14_b40_ctlogic_crson_td_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_dmd_auto_eco : 1;
        unsigned char b40_dmd_auto_cmpib : 2;
        unsigned char b40_dmd_auto_ccm : 1;
        unsigned char b40_dmd_atest : 1;
        unsigned char b40_dbias_eco : 3;
    } reg;
} PMIC_B401_CTRL15_UNION;
#endif
#define PMIC_B401_CTRL15_b40_dmd_auto_eco_START (0)
#define PMIC_B401_CTRL15_b40_dmd_auto_eco_END (0)
#define PMIC_B401_CTRL15_b40_dmd_auto_cmpib_START (1)
#define PMIC_B401_CTRL15_b40_dmd_auto_cmpib_END (2)
#define PMIC_B401_CTRL15_b40_dmd_auto_ccm_START (3)
#define PMIC_B401_CTRL15_b40_dmd_auto_ccm_END (3)
#define PMIC_B401_CTRL15_b40_dmd_atest_START (4)
#define PMIC_B401_CTRL15_b40_dmd_atest_END (4)
#define PMIC_B401_CTRL15_b40_dbias_eco_START (5)
#define PMIC_B401_CTRL15_b40_dbias_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_dmd_bck_sel : 1;
        unsigned char b40_dmd_auto_vos : 3;
        unsigned char b40_dmd_auto_vbc : 1;
        unsigned char b40_dmd_auto_sel : 1;
        unsigned char b40_dmd_auto_sampt : 2;
    } reg;
} PMIC_B401_CTRL16_UNION;
#endif
#define PMIC_B401_CTRL16_b40_dmd_bck_sel_START (0)
#define PMIC_B401_CTRL16_b40_dmd_bck_sel_END (0)
#define PMIC_B401_CTRL16_b40_dmd_auto_vos_START (1)
#define PMIC_B401_CTRL16_b40_dmd_auto_vos_END (3)
#define PMIC_B401_CTRL16_b40_dmd_auto_vbc_START (4)
#define PMIC_B401_CTRL16_b40_dmd_auto_vbc_END (4)
#define PMIC_B401_CTRL16_b40_dmd_auto_sel_START (5)
#define PMIC_B401_CTRL16_b40_dmd_auto_sel_END (5)
#define PMIC_B401_CTRL16_b40_dmd_auto_sampt_START (6)
#define PMIC_B401_CTRL16_b40_dmd_auto_sampt_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_dmd_ngc_sel : 1;
        unsigned char b40_dmd_ngc_disch : 1;
        unsigned char b40_dmd_man_set : 4;
        unsigned char b40_dmd_eco_sel : 1;
        unsigned char b40_dmd_clp_sel : 1;
    } reg;
} PMIC_B401_CTRL17_UNION;
#endif
#define PMIC_B401_CTRL17_b40_dmd_ngc_sel_START (0)
#define PMIC_B401_CTRL17_b40_dmd_ngc_sel_END (0)
#define PMIC_B401_CTRL17_b40_dmd_ngc_disch_START (1)
#define PMIC_B401_CTRL17_b40_dmd_ngc_disch_END (1)
#define PMIC_B401_CTRL17_b40_dmd_man_set_START (2)
#define PMIC_B401_CTRL17_b40_dmd_man_set_END (5)
#define PMIC_B401_CTRL17_b40_dmd_eco_sel_START (6)
#define PMIC_B401_CTRL17_b40_dmd_eco_sel_END (6)
#define PMIC_B401_CTRL17_b40_dmd_clp_sel_START (7)
#define PMIC_B401_CTRL17_b40_dmd_clp_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_drv_sw_sel : 1;
        unsigned char b40_drv_dt_sel : 4;
        unsigned char b40_dmd_ngc_set : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_B401_CTRL18_UNION;
#endif
#define PMIC_B401_CTRL18_b40_drv_sw_sel_START (0)
#define PMIC_B401_CTRL18_b40_drv_sw_sel_END (0)
#define PMIC_B401_CTRL18_b40_drv_dt_sel_START (1)
#define PMIC_B401_CTRL18_b40_drv_dt_sel_END (4)
#define PMIC_B401_CTRL18_b40_dmd_ngc_set_START (5)
#define PMIC_B401_CTRL18_b40_dmd_ngc_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_dt_sel : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_B401_CTRL19_UNION;
#endif
#define PMIC_B401_CTRL19_b40_dt_sel_START (0)
#define PMIC_B401_CTRL19_b40_dt_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_eabias : 1;
        unsigned char b40_eaamp_clp_sel : 1;
        unsigned char b40_eaamp_clp : 2;
        unsigned char b40_ea_ecocur_sel : 3;
        unsigned char reserved : 1;
    } reg;
} PMIC_B401_CTRL20_UNION;
#endif
#define PMIC_B401_CTRL20_b40_eabias_START (0)
#define PMIC_B401_CTRL20_b40_eabias_END (0)
#define PMIC_B401_CTRL20_b40_eaamp_clp_sel_START (1)
#define PMIC_B401_CTRL20_b40_eaamp_clp_sel_END (1)
#define PMIC_B401_CTRL20_b40_eaamp_clp_START (2)
#define PMIC_B401_CTRL20_b40_eaamp_clp_END (3)
#define PMIC_B401_CTRL20_b40_ea_ecocur_sel_START (4)
#define PMIC_B401_CTRL20_b40_ea_ecocur_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_eaclp_ecogap_sel : 2;
        unsigned char b40_eaclp_brgap_sel : 4;
        unsigned char reserved : 2;
    } reg;
} PMIC_B401_CTRL21_UNION;
#endif
#define PMIC_B401_CTRL21_b40_eaclp_ecogap_sel_START (0)
#define PMIC_B401_CTRL21_b40_eaclp_ecogap_sel_END (1)
#define PMIC_B401_CTRL21_b40_eaclp_brgap_sel_START (2)
#define PMIC_B401_CTRL21_b40_eaclp_brgap_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_eacomp_eco_shield : 1;
        unsigned char b40_eaclp_srgap_sel : 4;
        unsigned char b40_eaclp_gap_sel : 3;
    } reg;
} PMIC_B401_CTRL22_UNION;
#endif
#define PMIC_B401_CTRL22_b40_eacomp_eco_shield_START (0)
#define PMIC_B401_CTRL22_b40_eacomp_eco_shield_END (0)
#define PMIC_B401_CTRL22_b40_eaclp_srgap_sel_START (1)
#define PMIC_B401_CTRL22_b40_eaclp_srgap_sel_END (4)
#define PMIC_B401_CTRL22_b40_eaclp_gap_sel_START (5)
#define PMIC_B401_CTRL22_b40_eaclp_gap_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_ecdt_ct2cm_ref : 3;
        unsigned char b40_ecdt_ct2cm_ft : 2;
        unsigned char b40_ecdt_cmphys : 1;
        unsigned char b40_eatrim_en : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_B401_CTRL23_UNION;
#endif
#define PMIC_B401_CTRL23_b40_ecdt_ct2cm_ref_START (0)
#define PMIC_B401_CTRL23_b40_ecdt_ct2cm_ref_END (2)
#define PMIC_B401_CTRL23_b40_ecdt_ct2cm_ft_START (3)
#define PMIC_B401_CTRL23_b40_ecdt_ct2cm_ft_END (4)
#define PMIC_B401_CTRL23_b40_ecdt_cmphys_START (5)
#define PMIC_B401_CTRL23_b40_ecdt_cmphys_END (5)
#define PMIC_B401_CTRL23_b40_eatrim_en_START (6)
#define PMIC_B401_CTRL23_b40_eatrim_en_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_ecdt_ec2ct_ref : 3;
        unsigned char b40_ecdt_ct2ec_ref : 3;
        unsigned char b40_ecdt_ct2ec_ft : 2;
    } reg;
} PMIC_B401_CTRL24_UNION;
#endif
#define PMIC_B401_CTRL24_b40_ecdt_ec2ct_ref_START (0)
#define PMIC_B401_CTRL24_b40_ecdt_ec2ct_ref_END (2)
#define PMIC_B401_CTRL24_b40_ecdt_ct2ec_ref_START (3)
#define PMIC_B401_CTRL24_b40_ecdt_ct2ec_ref_END (5)
#define PMIC_B401_CTRL24_b40_ecdt_ct2ec_ft_START (6)
#define PMIC_B401_CTRL24_b40_ecdt_ct2ec_ft_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_en_psns : 1;
        unsigned char b40_en_nsense : 1;
        unsigned char b40_en_chopper : 1;
        unsigned char b40_ecotrim_shield : 1;
        unsigned char b40_eco_shield_cmp : 1;
        unsigned char b40_eco_shield : 1;
        unsigned char b40_ecdt_filter_rc : 2;
    } reg;
} PMIC_B401_CTRL25_UNION;
#endif
#define PMIC_B401_CTRL25_b40_en_psns_START (0)
#define PMIC_B401_CTRL25_b40_en_psns_END (0)
#define PMIC_B401_CTRL25_b40_en_nsense_START (1)
#define PMIC_B401_CTRL25_b40_en_nsense_END (1)
#define PMIC_B401_CTRL25_b40_en_chopper_START (2)
#define PMIC_B401_CTRL25_b40_en_chopper_END (2)
#define PMIC_B401_CTRL25_b40_ecotrim_shield_START (3)
#define PMIC_B401_CTRL25_b40_ecotrim_shield_END (3)
#define PMIC_B401_CTRL25_b40_eco_shield_cmp_START (4)
#define PMIC_B401_CTRL25_b40_eco_shield_cmp_END (4)
#define PMIC_B401_CTRL25_b40_eco_shield_START (5)
#define PMIC_B401_CTRL25_b40_eco_shield_END (5)
#define PMIC_B401_CTRL25_b40_ecdt_filter_rc_START (6)
#define PMIC_B401_CTRL25_b40_ecdt_filter_rc_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_ldline_forced_on : 1;
        unsigned char b40_ibalance_gm_sel : 2;
        unsigned char b40_forcenormal : 1;
        unsigned char b40_forceeco : 1;
        unsigned char b40_fastnmos_off : 1;
        unsigned char b40_en_vo : 1;
        unsigned char b40_en_test : 1;
    } reg;
} PMIC_B401_CTRL26_UNION;
#endif
#define PMIC_B401_CTRL26_b40_ldline_forced_on_START (0)
#define PMIC_B401_CTRL26_b40_ldline_forced_on_END (0)
#define PMIC_B401_CTRL26_b40_ibalance_gm_sel_START (1)
#define PMIC_B401_CTRL26_b40_ibalance_gm_sel_END (2)
#define PMIC_B401_CTRL26_b40_forcenormal_START (3)
#define PMIC_B401_CTRL26_b40_forcenormal_END (3)
#define PMIC_B401_CTRL26_b40_forceeco_START (4)
#define PMIC_B401_CTRL26_b40_forceeco_END (4)
#define PMIC_B401_CTRL26_b40_fastnmos_off_START (5)
#define PMIC_B401_CTRL26_b40_fastnmos_off_END (5)
#define PMIC_B401_CTRL26_b40_en_vo_START (6)
#define PMIC_B401_CTRL26_b40_en_vo_END (6)
#define PMIC_B401_CTRL26_b40_en_test_START (7)
#define PMIC_B401_CTRL26_b40_en_test_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_load_det_sel : 2;
        unsigned char b40_ldline_mohm_sel : 5;
        unsigned char reserved : 1;
    } reg;
} PMIC_B401_CTRL27_UNION;
#endif
#define PMIC_B401_CTRL27_b40_load_det_sel_START (0)
#define PMIC_B401_CTRL27_b40_load_det_sel_END (1)
#define PMIC_B401_CTRL27_b40_ldline_mohm_sel_START (2)
#define PMIC_B401_CTRL27_b40_ldline_mohm_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_min_ton_sel : 2;
        unsigned char b40_min_toff_sel : 2;
        unsigned char b40_load_state_sel : 3;
        unsigned char reserved : 1;
    } reg;
} PMIC_B401_CTRL28_UNION;
#endif
#define PMIC_B401_CTRL28_b40_min_ton_sel_START (0)
#define PMIC_B401_CTRL28_b40_min_ton_sel_END (1)
#define PMIC_B401_CTRL28_b40_min_toff_sel_START (2)
#define PMIC_B401_CTRL28_b40_min_toff_sel_END (3)
#define PMIC_B401_CTRL28_b40_load_state_sel_START (4)
#define PMIC_B401_CTRL28_b40_load_state_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_ng_dt_sel : 3;
        unsigned char b40_negdmdton_add : 1;
        unsigned char b40_negdmd_ton_sel : 2;
        unsigned char b40_n_sel : 2;
    } reg;
} PMIC_B401_CTRL29_UNION;
#endif
#define PMIC_B401_CTRL29_b40_ng_dt_sel_START (0)
#define PMIC_B401_CTRL29_b40_ng_dt_sel_END (2)
#define PMIC_B401_CTRL29_b40_negdmdton_add_START (3)
#define PMIC_B401_CTRL29_b40_negdmdton_add_END (3)
#define PMIC_B401_CTRL29_b40_negdmd_ton_sel_START (4)
#define PMIC_B401_CTRL29_b40_negdmd_ton_sel_END (5)
#define PMIC_B401_CTRL29_b40_n_sel_START (6)
#define PMIC_B401_CTRL29_b40_n_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_nmos_off : 1;
        unsigned char b40_ng_p_sel : 3;
        unsigned char b40_ng_n_sel : 3;
        unsigned char reserved : 1;
    } reg;
} PMIC_B401_CTRL30_UNION;
#endif
#define PMIC_B401_CTRL30_b40_nmos_off_START (0)
#define PMIC_B401_CTRL30_b40_nmos_off_END (0)
#define PMIC_B401_CTRL30_b40_ng_p_sel_START (1)
#define PMIC_B401_CTRL30_b40_ng_p_sel_END (3)
#define PMIC_B401_CTRL30_b40_ng_n_sel_START (4)
#define PMIC_B401_CTRL30_b40_ng_n_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_nocp_low_sel : 2;
        unsigned char b40_nocp_low_eco_sel : 1;
        unsigned char b40_nocp_low_delay_sel : 2;
        unsigned char b40_nocp_en : 1;
        unsigned char b40_nocp_delay_sel : 2;
    } reg;
} PMIC_B401_CTRL31_UNION;
#endif
#define PMIC_B401_CTRL31_b40_nocp_low_sel_START (0)
#define PMIC_B401_CTRL31_b40_nocp_low_sel_END (1)
#define PMIC_B401_CTRL31_b40_nocp_low_eco_sel_START (2)
#define PMIC_B401_CTRL31_b40_nocp_low_eco_sel_END (2)
#define PMIC_B401_CTRL31_b40_nocp_low_delay_sel_START (3)
#define PMIC_B401_CTRL31_b40_nocp_low_delay_sel_END (4)
#define PMIC_B401_CTRL31_b40_nocp_en_START (5)
#define PMIC_B401_CTRL31_b40_nocp_en_END (5)
#define PMIC_B401_CTRL31_b40_nocp_delay_sel_START (6)
#define PMIC_B401_CTRL31_b40_nocp_delay_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_ocp_2us_delay_sel : 1;
        unsigned char b40_nsense_sel : 3;
        unsigned char b40_npocp_atest : 1;
        unsigned char b40_normaltrim_shield : 1;
        unsigned char b40_nocp_sel : 2;
    } reg;
} PMIC_B401_CTRL32_UNION;
#endif
#define PMIC_B401_CTRL32_b40_ocp_2us_delay_sel_START (0)
#define PMIC_B401_CTRL32_b40_ocp_2us_delay_sel_END (0)
#define PMIC_B401_CTRL32_b40_nsense_sel_START (1)
#define PMIC_B401_CTRL32_b40_nsense_sel_END (3)
#define PMIC_B401_CTRL32_b40_npocp_atest_START (4)
#define PMIC_B401_CTRL32_b40_npocp_atest_END (4)
#define PMIC_B401_CTRL32_b40_normaltrim_shield_START (5)
#define PMIC_B401_CTRL32_b40_normaltrim_shield_END (5)
#define PMIC_B401_CTRL32_b40_nocp_sel_START (6)
#define PMIC_B401_CTRL32_b40_nocp_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_p_sel : 2;
        unsigned char b40_ovp_shield : 1;
        unsigned char b40_ovp_sel : 2;
        unsigned char b40_oneph : 1;
        unsigned char b40_ocp_short_en : 1;
        unsigned char b40_ocp_shield : 1;
    } reg;
} PMIC_B401_CTRL33_UNION;
#endif
#define PMIC_B401_CTRL33_b40_p_sel_START (0)
#define PMIC_B401_CTRL33_b40_p_sel_END (1)
#define PMIC_B401_CTRL33_b40_ovp_shield_START (2)
#define PMIC_B401_CTRL33_b40_ovp_shield_END (2)
#define PMIC_B401_CTRL33_b40_ovp_sel_START (3)
#define PMIC_B401_CTRL33_b40_ovp_sel_END (4)
#define PMIC_B401_CTRL33_b40_oneph_START (5)
#define PMIC_B401_CTRL33_b40_oneph_END (5)
#define PMIC_B401_CTRL33_b40_ocp_short_en_START (6)
#define PMIC_B401_CTRL33_b40_ocp_short_en_END (6)
#define PMIC_B401_CTRL33_b40_ocp_shield_START (7)
#define PMIC_B401_CTRL33_b40_ocp_shield_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_pg_p_sel : 3;
        unsigned char b40_pg_n_sel : 3;
        unsigned char b40_pg_dt_sel : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_B401_CTRL34_UNION;
#endif
#define PMIC_B401_CTRL34_b40_pg_p_sel_START (0)
#define PMIC_B401_CTRL34_b40_pg_p_sel_END (2)
#define PMIC_B401_CTRL34_b40_pg_n_sel_START (3)
#define PMIC_B401_CTRL34_b40_pg_n_sel_END (5)
#define PMIC_B401_CTRL34_b40_pg_dt_sel_START (6)
#define PMIC_B401_CTRL34_b40_pg_dt_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_pocp_sw_ccm_cmp : 1;
        unsigned char b40_pocp_sw_atest : 1;
        unsigned char b40_pocp_sel : 2;
        unsigned char b40_pocp_en : 1;
        unsigned char b40_pocp_eco_sel : 1;
        unsigned char b40_pocp_delay_sel : 2;
    } reg;
} PMIC_B401_CTRL35_UNION;
#endif
#define PMIC_B401_CTRL35_b40_pocp_sw_ccm_cmp_START (0)
#define PMIC_B401_CTRL35_b40_pocp_sw_ccm_cmp_END (0)
#define PMIC_B401_CTRL35_b40_pocp_sw_atest_START (1)
#define PMIC_B401_CTRL35_b40_pocp_sw_atest_END (1)
#define PMIC_B401_CTRL35_b40_pocp_sel_START (2)
#define PMIC_B401_CTRL35_b40_pocp_sel_END (3)
#define PMIC_B401_CTRL35_b40_pocp_en_START (4)
#define PMIC_B401_CTRL35_b40_pocp_en_END (4)
#define PMIC_B401_CTRL35_b40_pocp_eco_sel_START (5)
#define PMIC_B401_CTRL35_b40_pocp_eco_sel_END (5)
#define PMIC_B401_CTRL35_b40_pocp_delay_sel_START (6)
#define PMIC_B401_CTRL35_b40_pocp_delay_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_pulldn_pd_sel : 1;
        unsigned char b40_pulldn_pd_en : 1;
        unsigned char b40_pocp_sw_sel : 3;
        unsigned char b40_pocp_sw_eco_cmp : 1;
        unsigned char b40_pocp_sw_delay_sel : 2;
    } reg;
} PMIC_B401_CTRL36_UNION;
#endif
#define PMIC_B401_CTRL36_b40_pulldn_pd_sel_START (0)
#define PMIC_B401_CTRL36_b40_pulldn_pd_sel_END (0)
#define PMIC_B401_CTRL36_b40_pulldn_pd_en_START (1)
#define PMIC_B401_CTRL36_b40_pulldn_pd_en_END (1)
#define PMIC_B401_CTRL36_b40_pocp_sw_sel_START (2)
#define PMIC_B401_CTRL36_b40_pocp_sw_sel_END (4)
#define PMIC_B401_CTRL36_b40_pocp_sw_eco_cmp_START (5)
#define PMIC_B401_CTRL36_b40_pocp_sw_eco_cmp_END (5)
#define PMIC_B401_CTRL36_b40_pocp_sw_delay_sel_START (6)
#define PMIC_B401_CTRL36_b40_pocp_sw_delay_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_r1_sel : 4;
        unsigned char b40_pvdd_ton_sel : 1;
        unsigned char b40_pulldn_sel : 3;
    } reg;
} PMIC_B401_CTRL37_UNION;
#endif
#define PMIC_B401_CTRL37_b40_r1_sel_START (0)
#define PMIC_B401_CTRL37_b40_r1_sel_END (3)
#define PMIC_B401_CTRL37_b40_pvdd_ton_sel_START (4)
#define PMIC_B401_CTRL37_b40_pvdd_ton_sel_END (4)
#define PMIC_B401_CTRL37_b40_pulldn_sel_START (5)
#define PMIC_B401_CTRL37_b40_pulldn_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_r2_sel : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_B401_CTRL38_UNION;
#endif
#define PMIC_B401_CTRL38_b40_r2_sel_START (0)
#define PMIC_B401_CTRL38_b40_r2_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_ramp_big_sel : 1;
        unsigned char b40_r3_sel : 6;
        unsigned char reserved : 1;
    } reg;
} PMIC_B401_CTRL39_UNION;
#endif
#define PMIC_B401_CTRL39_b40_ramp_big_sel_START (0)
#define PMIC_B401_CTRL39_b40_ramp_big_sel_END (0)
#define PMIC_B401_CTRL39_b40_r3_sel_START (1)
#define PMIC_B401_CTRL39_b40_r3_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_ramp_ccm_outsel : 1;
        unsigned char b40_ramp_c : 2;
        unsigned char b40_ramp_buff_trim_test : 1;
        unsigned char b40_ramp_buff_trim : 1;
        unsigned char b40_ramp_buff_bias : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_B401_CTRL40_UNION;
#endif
#define PMIC_B401_CTRL40_b40_ramp_ccm_outsel_START (0)
#define PMIC_B401_CTRL40_b40_ramp_ccm_outsel_END (0)
#define PMIC_B401_CTRL40_b40_ramp_c_START (1)
#define PMIC_B401_CTRL40_b40_ramp_c_END (2)
#define PMIC_B401_CTRL40_b40_ramp_buff_trim_test_START (3)
#define PMIC_B401_CTRL40_b40_ramp_buff_trim_test_END (3)
#define PMIC_B401_CTRL40_b40_ramp_buff_trim_START (4)
#define PMIC_B401_CTRL40_b40_ramp_buff_trim_END (4)
#define PMIC_B401_CTRL40_b40_ramp_buff_bias_START (5)
#define PMIC_B401_CTRL40_b40_ramp_buff_bias_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_ramp_cot_outsel : 1;
        unsigned char b40_ramp_clk_sel : 1;
        unsigned char b40_ramp_change_mode : 5;
        unsigned char reserved : 1;
    } reg;
} PMIC_B401_CTRL41_UNION;
#endif
#define PMIC_B401_CTRL41_b40_ramp_cot_outsel_START (0)
#define PMIC_B401_CTRL41_b40_ramp_cot_outsel_END (0)
#define PMIC_B401_CTRL41_b40_ramp_clk_sel_START (1)
#define PMIC_B401_CTRL41_b40_ramp_clk_sel_END (1)
#define PMIC_B401_CTRL41_b40_ramp_change_mode_START (2)
#define PMIC_B401_CTRL41_b40_ramp_change_mode_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_ramp_line_sel : 1;
        unsigned char b40_ramp_freq_sel : 1;
        unsigned char b40_ramp_force_eco_test : 1;
        unsigned char b40_ramp_fast_sel : 1;
        unsigned char b40_ramp_eco_buffer : 1;
        unsigned char b40_ramp_dblclk_sel : 1;
        unsigned char b40_ramp_counter : 2;
    } reg;
} PMIC_B401_CTRL42_UNION;
#endif
#define PMIC_B401_CTRL42_b40_ramp_line_sel_START (0)
#define PMIC_B401_CTRL42_b40_ramp_line_sel_END (0)
#define PMIC_B401_CTRL42_b40_ramp_freq_sel_START (1)
#define PMIC_B401_CTRL42_b40_ramp_freq_sel_END (1)
#define PMIC_B401_CTRL42_b40_ramp_force_eco_test_START (2)
#define PMIC_B401_CTRL42_b40_ramp_force_eco_test_END (2)
#define PMIC_B401_CTRL42_b40_ramp_fast_sel_START (3)
#define PMIC_B401_CTRL42_b40_ramp_fast_sel_END (3)
#define PMIC_B401_CTRL42_b40_ramp_eco_buffer_START (4)
#define PMIC_B401_CTRL42_b40_ramp_eco_buffer_END (4)
#define PMIC_B401_CTRL42_b40_ramp_dblclk_sel_START (5)
#define PMIC_B401_CTRL42_b40_ramp_dblclk_sel_END (5)
#define PMIC_B401_CTRL42_b40_ramp_counter_START (6)
#define PMIC_B401_CTRL42_b40_ramp_counter_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_ramp_r_ccm : 4;
        unsigned char b40_ramp_mid_sel : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_B401_CTRL43_UNION;
#endif
#define PMIC_B401_CTRL43_b40_ramp_r_ccm_START (0)
#define PMIC_B401_CTRL43_b40_ramp_r_ccm_END (3)
#define PMIC_B401_CTRL43_b40_ramp_mid_sel_START (4)
#define PMIC_B401_CTRL43_b40_ramp_mid_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_ramp_r_eco : 4;
        unsigned char b40_ramp_r_cot : 4;
    } reg;
} PMIC_B401_CTRL44_UNION;
#endif
#define PMIC_B401_CTRL44_b40_ramp_r_eco_START (0)
#define PMIC_B401_CTRL44_b40_ramp_r_eco_END (3)
#define PMIC_B401_CTRL44_b40_ramp_r_cot_START (4)
#define PMIC_B401_CTRL44_b40_ramp_r_cot_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_ramptrim_sel : 1;
        unsigned char b40_ramp_valley_sel : 1;
        unsigned char b40_ramp_trim_sel : 1;
        unsigned char b40_ramp_sw_sel : 1;
        unsigned char b40_ramp_sel : 1;
        unsigned char b40_ramp_res_test : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_B401_CTRL45_UNION;
#endif
#define PMIC_B401_CTRL45_b40_ramptrim_sel_START (0)
#define PMIC_B401_CTRL45_b40_ramptrim_sel_END (0)
#define PMIC_B401_CTRL45_b40_ramp_valley_sel_START (1)
#define PMIC_B401_CTRL45_b40_ramp_valley_sel_END (1)
#define PMIC_B401_CTRL45_b40_ramp_trim_sel_START (2)
#define PMIC_B401_CTRL45_b40_ramp_trim_sel_END (2)
#define PMIC_B401_CTRL45_b40_ramp_sw_sel_START (3)
#define PMIC_B401_CTRL45_b40_ramp_sw_sel_END (3)
#define PMIC_B401_CTRL45_b40_ramp_sel_START (4)
#define PMIC_B401_CTRL45_b40_ramp_sel_END (4)
#define PMIC_B401_CTRL45_b40_ramp_res_test_START (5)
#define PMIC_B401_CTRL45_b40_ramp_res_test_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_reg_idrp : 3;
        unsigned char b40_reg_idrn : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_B401_CTRL46_UNION;
#endif
#define PMIC_B401_CTRL46_b40_reg_idrp_START (0)
#define PMIC_B401_CTRL46_b40_reg_idrp_END (2)
#define PMIC_B401_CTRL46_b40_reg_idrn_START (3)
#define PMIC_B401_CTRL46_b40_reg_idrn_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_reg_sense_res_sel : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_B401_CTRL47_UNION;
#endif
#define PMIC_B401_CTRL47_b40_reg_sense_res_sel_START (0)
#define PMIC_B401_CTRL47_b40_reg_sense_res_sel_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_reserve0 : 8;
    } reg;
} PMIC_B401_CTRL48_UNION;
#endif
#define PMIC_B401_CTRL48_b40_reserve0_START (0)
#define PMIC_B401_CTRL48_b40_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_reserve1 : 8;
    } reg;
} PMIC_B401_CTRL49_UNION;
#endif
#define PMIC_B401_CTRL49_b40_reserve1_START (0)
#define PMIC_B401_CTRL49_b40_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_reserve2 : 8;
    } reg;
} PMIC_B401_CTRL50_UNION;
#endif
#define PMIC_B401_CTRL50_b40_reserve2_START (0)
#define PMIC_B401_CTRL50_b40_reserve2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_reserve3 : 8;
    } reg;
} PMIC_B401_CTRL51_UNION;
#endif
#define PMIC_B401_CTRL51_b40_reserve3_START (0)
#define PMIC_B401_CTRL51_b40_reserve3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_sft : 1;
        unsigned char b40_sense_ratio_sel : 2;
        unsigned char b40_rtr_cap_sel : 2;
        unsigned char b40_ron_test_sel : 3;
    } reg;
} PMIC_B401_CTRL52_UNION;
#endif
#define PMIC_B401_CTRL52_b40_sft_START (0)
#define PMIC_B401_CTRL52_b40_sft_END (0)
#define PMIC_B401_CTRL52_b40_sense_ratio_sel_START (1)
#define PMIC_B401_CTRL52_b40_sense_ratio_sel_END (2)
#define PMIC_B401_CTRL52_b40_rtr_cap_sel_START (3)
#define PMIC_B401_CTRL52_b40_rtr_cap_sel_END (4)
#define PMIC_B401_CTRL52_b40_ron_test_sel_START (5)
#define PMIC_B401_CTRL52_b40_ron_test_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_stsw_atest : 1;
        unsigned char b40_state_sw_ocp_sel : 2;
        unsigned char b40_state_sw_ocp_off : 1;
        unsigned char b40_slave_sel : 1;
        unsigned char b40_short_shield : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_B401_CTRL53_UNION;
#endif
#define PMIC_B401_CTRL53_b40_stsw_atest_START (0)
#define PMIC_B401_CTRL53_b40_stsw_atest_END (0)
#define PMIC_B401_CTRL53_b40_state_sw_ocp_sel_START (1)
#define PMIC_B401_CTRL53_b40_state_sw_ocp_sel_END (2)
#define PMIC_B401_CTRL53_b40_state_sw_ocp_off_START (3)
#define PMIC_B401_CTRL53_b40_state_sw_ocp_off_END (3)
#define PMIC_B401_CTRL53_b40_slave_sel_START (4)
#define PMIC_B401_CTRL53_b40_slave_sel_END (4)
#define PMIC_B401_CTRL53_b40_short_shield_START (5)
#define PMIC_B401_CTRL53_b40_short_shield_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_stsw_fstsch_eco : 3;
        unsigned char b40_stsw_fstps_ith : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_B401_CTRL54_UNION;
#endif
#define PMIC_B401_CTRL54_b40_stsw_fstsch_eco_START (0)
#define PMIC_B401_CTRL54_b40_stsw_fstsch_eco_END (2)
#define PMIC_B401_CTRL54_b40_stsw_fstps_ith_START (3)
#define PMIC_B401_CTRL54_b40_stsw_fstps_ith_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_stsw_slw_ct2cm_hys : 2;
        unsigned char b40_stsw_rpup_ccm : 1;
        unsigned char b40_stsw_man_set : 2;
        unsigned char b40_stsw_fstsch_neco : 3;
    } reg;
} PMIC_B401_CTRL55_UNION;
#endif
#define PMIC_B401_CTRL55_b40_stsw_slw_ct2cm_hys_START (0)
#define PMIC_B401_CTRL55_b40_stsw_slw_ct2cm_hys_END (1)
#define PMIC_B401_CTRL55_b40_stsw_rpup_ccm_START (2)
#define PMIC_B401_CTRL55_b40_stsw_rpup_ccm_END (2)
#define PMIC_B401_CTRL55_b40_stsw_man_set_START (3)
#define PMIC_B401_CTRL55_b40_stsw_man_set_END (4)
#define PMIC_B401_CTRL55_b40_stsw_fstsch_neco_START (5)
#define PMIC_B401_CTRL55_b40_stsw_fstsch_neco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_stsw_slw_hys_shield : 1;
        unsigned char b40_stsw_slw_delay_l : 2;
        unsigned char b40_stsw_slw_delay_h : 2;
        unsigned char b40_stsw_slw_ct2cm_ith : 3;
    } reg;
} PMIC_B401_CTRL56_UNION;
#endif
#define PMIC_B401_CTRL56_b40_stsw_slw_hys_shield_START (0)
#define PMIC_B401_CTRL56_b40_stsw_slw_hys_shield_END (0)
#define PMIC_B401_CTRL56_b40_stsw_slw_delay_l_START (1)
#define PMIC_B401_CTRL56_b40_stsw_slw_delay_l_END (2)
#define PMIC_B401_CTRL56_b40_stsw_slw_delay_h_START (3)
#define PMIC_B401_CTRL56_b40_stsw_slw_delay_h_END (4)
#define PMIC_B401_CTRL56_b40_stsw_slw_ct2cm_ith_START (5)
#define PMIC_B401_CTRL56_b40_stsw_slw_ct2cm_ith_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_ton_det : 4;
        unsigned char b40_stsw_slwsch_neco : 2;
        unsigned char b40_stsw_slwsch_eco : 2;
    } reg;
} PMIC_B401_CTRL57_UNION;
#endif
#define PMIC_B401_CTRL57_b40_ton_det_START (0)
#define PMIC_B401_CTRL57_b40_ton_det_END (3)
#define PMIC_B401_CTRL57_b40_stsw_slwsch_neco_START (4)
#define PMIC_B401_CTRL57_b40_stsw_slwsch_neco_END (5)
#define PMIC_B401_CTRL57_b40_stsw_slwsch_eco_START (6)
#define PMIC_B401_CTRL57_b40_stsw_slwsch_eco_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_vref_test_sel : 2;
        unsigned char b40_vo_ton_res_sel : 3;
        unsigned char b40_trim2 : 1;
        unsigned char b40_trim1 : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_B401_CTRL58_UNION;
#endif
#define PMIC_B401_CTRL58_b40_vref_test_sel_START (0)
#define PMIC_B401_CTRL58_b40_vref_test_sel_END (1)
#define PMIC_B401_CTRL58_b40_vo_ton_res_sel_START (2)
#define PMIC_B401_CTRL58_b40_vo_ton_res_sel_END (4)
#define PMIC_B401_CTRL58_b40_trim2_START (5)
#define PMIC_B401_CTRL58_b40_trim2_END (5)
#define PMIC_B401_CTRL58_b40_trim1_START (6)
#define PMIC_B401_CTRL58_b40_trim1_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b41_cot_atest : 1;
        unsigned char b41_cmp_atest : 1;
        unsigned char b40_vref_vo_cap_sel : 2;
        unsigned char reserved : 4;
    } reg;
} PMIC_B401_CTRL59_UNION;
#endif
#define PMIC_B401_CTRL59_b41_cot_atest_START (0)
#define PMIC_B401_CTRL59_b41_cot_atest_END (0)
#define PMIC_B401_CTRL59_b41_cmp_atest_START (1)
#define PMIC_B401_CTRL59_b41_cmp_atest_END (1)
#define PMIC_B401_CTRL59_b40_vref_vo_cap_sel_START (2)
#define PMIC_B401_CTRL59_b40_vref_vo_cap_sel_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b41_cot_ton_ecoa : 8;
    } reg;
} PMIC_B401_CTRL60_UNION;
#endif
#define PMIC_B401_CTRL60_b41_cot_ton_ecoa_START (0)
#define PMIC_B401_CTRL60_b41_cot_ton_ecoa_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b41_cot_ton_ecob : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_B401_CTRL61_UNION;
#endif
#define PMIC_B401_CTRL61_b41_cot_ton_ecob_START (0)
#define PMIC_B401_CTRL61_b41_cot_ton_ecob_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b41_cot_ton_necoa : 8;
    } reg;
} PMIC_B401_CTRL62_UNION;
#endif
#define PMIC_B401_CTRL62_b41_cot_ton_necoa_START (0)
#define PMIC_B401_CTRL62_b41_cot_ton_necoa_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b41_dmd_man_set : 4;
        unsigned char b41_dmd_atest : 1;
        unsigned char b41_cot_ton_necob : 3;
    } reg;
} PMIC_B401_CTRL63_UNION;
#endif
#define PMIC_B401_CTRL63_b41_dmd_man_set_START (0)
#define PMIC_B401_CTRL63_b41_dmd_man_set_END (3)
#define PMIC_B401_CTRL63_b41_dmd_atest_START (4)
#define PMIC_B401_CTRL63_b41_dmd_atest_END (4)
#define PMIC_B401_CTRL63_b41_cot_ton_necob_START (5)
#define PMIC_B401_CTRL63_b41_cot_ton_necob_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b41_drv_sw_sel : 1;
        unsigned char b41_drv_dt_sel : 4;
        unsigned char b41_dmd_ngc_set : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_B401_CTRL64_UNION;
#endif
#define PMIC_B401_CTRL64_b41_drv_sw_sel_START (0)
#define PMIC_B401_CTRL64_b41_drv_sw_sel_END (0)
#define PMIC_B401_CTRL64_b41_drv_dt_sel_START (1)
#define PMIC_B401_CTRL64_b41_drv_dt_sel_END (4)
#define PMIC_B401_CTRL64_b41_dmd_ngc_set_START (5)
#define PMIC_B401_CTRL64_b41_dmd_ngc_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b41_dt_sel : 7;
        unsigned char reserved : 1;
    } reg;
} PMIC_B401_CTRL65_UNION;
#endif
#define PMIC_B401_CTRL65_b41_dt_sel_START (0)
#define PMIC_B401_CTRL65_b41_dt_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b41_ng_n_sel : 3;
        unsigned char b41_ng_dt_sel : 3;
        unsigned char b41_n_sel : 2;
    } reg;
} PMIC_B401_CTRL66_UNION;
#endif
#define PMIC_B401_CTRL66_b41_ng_n_sel_START (0)
#define PMIC_B401_CTRL66_b41_ng_n_sel_END (2)
#define PMIC_B401_CTRL66_b41_ng_dt_sel_START (3)
#define PMIC_B401_CTRL66_b41_ng_dt_sel_END (5)
#define PMIC_B401_CTRL66_b41_n_sel_START (6)
#define PMIC_B401_CTRL66_b41_n_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b41_pg_dt_sel : 1;
        unsigned char b41_p_sel : 2;
        unsigned char b41_npocp_atest : 1;
        unsigned char b41_ng_p_sel : 3;
        unsigned char reserved : 1;
    } reg;
} PMIC_B401_CTRL67_UNION;
#endif
#define PMIC_B401_CTRL67_b41_pg_dt_sel_START (0)
#define PMIC_B401_CTRL67_b41_pg_dt_sel_END (0)
#define PMIC_B401_CTRL67_b41_p_sel_START (1)
#define PMIC_B401_CTRL67_b41_p_sel_END (2)
#define PMIC_B401_CTRL67_b41_npocp_atest_START (3)
#define PMIC_B401_CTRL67_b41_npocp_atest_END (3)
#define PMIC_B401_CTRL67_b41_ng_p_sel_START (4)
#define PMIC_B401_CTRL67_b41_ng_p_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b41_ramp_buff_trim_test : 1;
        unsigned char b41_pocp_sw_atest : 1;
        unsigned char b41_pg_p_sel : 3;
        unsigned char b41_pg_n_sel : 3;
    } reg;
} PMIC_B401_CTRL68_UNION;
#endif
#define PMIC_B401_CTRL68_b41_ramp_buff_trim_test_START (0)
#define PMIC_B401_CTRL68_b41_ramp_buff_trim_test_END (0)
#define PMIC_B401_CTRL68_b41_pocp_sw_atest_START (1)
#define PMIC_B401_CTRL68_b41_pocp_sw_atest_END (1)
#define PMIC_B401_CTRL68_b41_pg_p_sel_START (2)
#define PMIC_B401_CTRL68_b41_pg_p_sel_END (4)
#define PMIC_B401_CTRL68_b41_pg_n_sel_START (5)
#define PMIC_B401_CTRL68_b41_pg_n_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b41_ron_test_sel : 3;
        unsigned char b41_ramp_res_test : 1;
        unsigned char b41_ramp_r_ccm : 4;
    } reg;
} PMIC_B401_CTRL69_UNION;
#endif
#define PMIC_B401_CTRL69_b41_ron_test_sel_START (0)
#define PMIC_B401_CTRL69_b41_ron_test_sel_END (2)
#define PMIC_B401_CTRL69_b41_ramp_res_test_START (3)
#define PMIC_B401_CTRL69_b41_ramp_res_test_END (3)
#define PMIC_B401_CTRL69_b41_ramp_r_ccm_START (4)
#define PMIC_B401_CTRL69_b41_ramp_r_ccm_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b40_state_a2d : 2;
        unsigned char b40_pg : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_B401_RO0_UNION;
#endif
#define PMIC_B401_RO0_b40_state_a2d_START (0)
#define PMIC_B401_RO0_b40_state_a2d_END (1)
#define PMIC_B401_RO0_b40_pg_START (2)
#define PMIC_B401_RO0_b40_pg_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b5_adj_rlx : 4;
        unsigned char b5_adj_clx : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK5_CTRL0_UNION;
#endif
#define PMIC_BUCK5_CTRL0_b5_adj_rlx_START (0)
#define PMIC_BUCK5_CTRL0_b5_adj_rlx_END (3)
#define PMIC_BUCK5_CTRL0_b5_adj_clx_START (4)
#define PMIC_BUCK5_CTRL0_b5_adj_clx_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b5_dmd_blanktime_sel : 1;
        unsigned char b5_cot_rlx : 1;
        unsigned char b5_cmp_ibias : 4;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK5_CTRL1_UNION;
#endif
#define PMIC_BUCK5_CTRL1_b5_dmd_blanktime_sel_START (0)
#define PMIC_BUCK5_CTRL1_b5_dmd_blanktime_sel_END (0)
#define PMIC_BUCK5_CTRL1_b5_cot_rlx_START (1)
#define PMIC_BUCK5_CTRL1_b5_cot_rlx_END (1)
#define PMIC_BUCK5_CTRL1_b5_cmp_ibias_START (2)
#define PMIC_BUCK5_CTRL1_b5_cmp_ibias_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b5_dmd_sel_eco : 4;
        unsigned char b5_dmd_sel : 4;
    } reg;
} PMIC_BUCK5_CTRL2_UNION;
#endif
#define PMIC_BUCK5_CTRL2_b5_dmd_sel_eco_START (0)
#define PMIC_BUCK5_CTRL2_b5_dmd_sel_eco_END (3)
#define PMIC_BUCK5_CTRL2_b5_dmd_sel_START (4)
#define PMIC_BUCK5_CTRL2_b5_dmd_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b5_ea_eco_dis : 1;
        unsigned char b5_ea_clamp_eco_dis : 1;
        unsigned char b5_dt_sel : 1;
        unsigned char b5_dmd_type_sel : 1;
        unsigned char b5_dmd_ton : 3;
        unsigned char b5_dmd_shield_ton : 1;
    } reg;
} PMIC_BUCK5_CTRL3_UNION;
#endif
#define PMIC_BUCK5_CTRL3_b5_ea_eco_dis_START (0)
#define PMIC_BUCK5_CTRL3_b5_ea_eco_dis_END (0)
#define PMIC_BUCK5_CTRL3_b5_ea_clamp_eco_dis_START (1)
#define PMIC_BUCK5_CTRL3_b5_ea_clamp_eco_dis_END (1)
#define PMIC_BUCK5_CTRL3_b5_dt_sel_START (2)
#define PMIC_BUCK5_CTRL3_b5_dt_sel_END (2)
#define PMIC_BUCK5_CTRL3_b5_dmd_type_sel_START (3)
#define PMIC_BUCK5_CTRL3_b5_dmd_type_sel_END (3)
#define PMIC_BUCK5_CTRL3_b5_dmd_ton_START (4)
#define PMIC_BUCK5_CTRL3_b5_dmd_ton_END (6)
#define PMIC_BUCK5_CTRL3_b5_dmd_shield_ton_START (7)
#define PMIC_BUCK5_CTRL3_b5_dmd_shield_ton_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b5_fix_ton : 1;
        unsigned char b5_fb_cap_sel : 1;
        unsigned char b5_fastramp_ctrl : 4;
        unsigned char b5_eco_ibias_sel : 1;
        unsigned char b5_ea_ibias_sel : 1;
    } reg;
} PMIC_BUCK5_CTRL4_UNION;
#endif
#define PMIC_BUCK5_CTRL4_b5_fix_ton_START (0)
#define PMIC_BUCK5_CTRL4_b5_fix_ton_END (0)
#define PMIC_BUCK5_CTRL4_b5_fb_cap_sel_START (1)
#define PMIC_BUCK5_CTRL4_b5_fb_cap_sel_END (1)
#define PMIC_BUCK5_CTRL4_b5_fastramp_ctrl_START (2)
#define PMIC_BUCK5_CTRL4_b5_fastramp_ctrl_END (5)
#define PMIC_BUCK5_CTRL4_b5_eco_ibias_sel_START (6)
#define PMIC_BUCK5_CTRL4_b5_eco_ibias_sel_END (6)
#define PMIC_BUCK5_CTRL4_b5_ea_ibias_sel_START (7)
#define PMIC_BUCK5_CTRL4_b5_ea_ibias_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b5_min_ton : 3;
        unsigned char b5_lx_dt : 1;
        unsigned char b5_ibias_dmd_ctrl : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK5_CTRL5_UNION;
#endif
#define PMIC_BUCK5_CTRL5_b5_min_ton_START (0)
#define PMIC_BUCK5_CTRL5_b5_min_ton_END (2)
#define PMIC_BUCK5_CTRL5_b5_lx_dt_START (3)
#define PMIC_BUCK5_CTRL5_b5_lx_dt_END (3)
#define PMIC_BUCK5_CTRL5_b5_ibias_dmd_ctrl_START (4)
#define PMIC_BUCK5_CTRL5_b5_ibias_dmd_ctrl_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b5_ntp_dt_ctrl : 1;
        unsigned char b5_nonlinear_driver : 1;
        unsigned char b5_ng_p_sel : 3;
        unsigned char b5_ng_n_sel : 3;
    } reg;
} PMIC_BUCK5_CTRL6_UNION;
#endif
#define PMIC_BUCK5_CTRL6_b5_ntp_dt_ctrl_START (0)
#define PMIC_BUCK5_CTRL6_b5_ntp_dt_ctrl_END (0)
#define PMIC_BUCK5_CTRL6_b5_nonlinear_driver_START (1)
#define PMIC_BUCK5_CTRL6_b5_nonlinear_driver_END (1)
#define PMIC_BUCK5_CTRL6_b5_ng_p_sel_START (2)
#define PMIC_BUCK5_CTRL6_b5_ng_p_sel_END (4)
#define PMIC_BUCK5_CTRL6_b5_ng_n_sel_START (5)
#define PMIC_BUCK5_CTRL6_b5_ng_n_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b5_ocp_sel : 3;
        unsigned char b5_ocp_delay : 1;
        unsigned char b5_ocp_cmp_ibias : 1;
        unsigned char b5_ntp_dt_sel : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK5_CTRL7_UNION;
#endif
#define PMIC_BUCK5_CTRL7_b5_ocp_sel_START (0)
#define PMIC_BUCK5_CTRL7_b5_ocp_sel_END (2)
#define PMIC_BUCK5_CTRL7_b5_ocp_delay_START (3)
#define PMIC_BUCK5_CTRL7_b5_ocp_delay_END (3)
#define PMIC_BUCK5_CTRL7_b5_ocp_cmp_ibias_START (4)
#define PMIC_BUCK5_CTRL7_b5_ocp_cmp_ibias_END (4)
#define PMIC_BUCK5_CTRL7_b5_ntp_dt_sel_START (5)
#define PMIC_BUCK5_CTRL7_b5_ntp_dt_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b5_offres_shut : 1;
        unsigned char b5_offres_sel : 1;
        unsigned char b5_ocpibias_sel : 2;
        unsigned char b5_ocp_toff : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK5_CTRL8_UNION;
#endif
#define PMIC_BUCK5_CTRL8_b5_offres_shut_START (0)
#define PMIC_BUCK5_CTRL8_b5_offres_shut_END (0)
#define PMIC_BUCK5_CTRL8_b5_offres_sel_START (1)
#define PMIC_BUCK5_CTRL8_b5_offres_sel_END (1)
#define PMIC_BUCK5_CTRL8_b5_ocpibias_sel_START (2)
#define PMIC_BUCK5_CTRL8_b5_ocpibias_sel_END (3)
#define PMIC_BUCK5_CTRL8_b5_ocp_toff_START (4)
#define PMIC_BUCK5_CTRL8_b5_ocp_toff_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b5_ptn_dt_ctrl : 1;
        unsigned char b5_pg_p_sel : 3;
        unsigned char b5_pg_n_sel : 3;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK5_CTRL9_UNION;
#endif
#define PMIC_BUCK5_CTRL9_b5_ptn_dt_ctrl_START (0)
#define PMIC_BUCK5_CTRL9_b5_ptn_dt_ctrl_END (0)
#define PMIC_BUCK5_CTRL9_b5_pg_p_sel_START (1)
#define PMIC_BUCK5_CTRL9_b5_pg_p_sel_END (3)
#define PMIC_BUCK5_CTRL9_b5_pg_n_sel_START (4)
#define PMIC_BUCK5_CTRL9_b5_pg_n_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b5_rampdown_ndmd : 3;
        unsigned char b5_ramp_ton_ctrl : 1;
        unsigned char b5_ramp_prebias : 1;
        unsigned char b5_ptn_dt_sel : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK5_CTRL10_UNION;
#endif
#define PMIC_BUCK5_CTRL10_b5_rampdown_ndmd_START (0)
#define PMIC_BUCK5_CTRL10_b5_rampdown_ndmd_END (2)
#define PMIC_BUCK5_CTRL10_b5_ramp_ton_ctrl_START (3)
#define PMIC_BUCK5_CTRL10_b5_ramp_ton_ctrl_END (3)
#define PMIC_BUCK5_CTRL10_b5_ramp_prebias_START (4)
#define PMIC_BUCK5_CTRL10_b5_ramp_prebias_END (4)
#define PMIC_BUCK5_CTRL10_b5_ptn_dt_sel_START (5)
#define PMIC_BUCK5_CTRL10_b5_ptn_dt_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b5_reg_r : 2;
        unsigned char b5_reg_op_c : 1;
        unsigned char b5_reg_ibias : 1;
        unsigned char b5_reg_en : 1;
        unsigned char b5_reg_dr : 3;
    } reg;
} PMIC_BUCK5_CTRL11_UNION;
#endif
#define PMIC_BUCK5_CTRL11_b5_reg_r_START (0)
#define PMIC_BUCK5_CTRL11_b5_reg_r_END (1)
#define PMIC_BUCK5_CTRL11_b5_reg_op_c_START (2)
#define PMIC_BUCK5_CTRL11_b5_reg_op_c_END (2)
#define PMIC_BUCK5_CTRL11_b5_reg_ibias_START (3)
#define PMIC_BUCK5_CTRL11_b5_reg_ibias_END (3)
#define PMIC_BUCK5_CTRL11_b5_reg_en_START (4)
#define PMIC_BUCK5_CTRL11_b5_reg_en_END (4)
#define PMIC_BUCK5_CTRL11_b5_reg_dr_START (5)
#define PMIC_BUCK5_CTRL11_b5_reg_dr_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b5_reserve0 : 8;
    } reg;
} PMIC_BUCK5_CTRL12_UNION;
#endif
#define PMIC_BUCK5_CTRL12_b5_reserve0_START (0)
#define PMIC_BUCK5_CTRL12_b5_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b5_reserve1 : 8;
    } reg;
} PMIC_BUCK5_CTRL13_UNION;
#endif
#define PMIC_BUCK5_CTRL13_b5_reserve1_START (0)
#define PMIC_BUCK5_CTRL13_b5_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b5_reserve2 : 8;
    } reg;
} PMIC_BUCK5_CTRL14_UNION;
#endif
#define PMIC_BUCK5_CTRL14_b5_reserve2_START (0)
#define PMIC_BUCK5_CTRL14_b5_reserve2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b5_softime_sel : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_BUCK5_CTRL15_UNION;
#endif
#define PMIC_BUCK5_CTRL15_b5_softime_sel_START (0)
#define PMIC_BUCK5_CTRL15_b5_softime_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b5_ocp_state : 1;
        unsigned char b5_ccm_state : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_BUCK5_RO0_UNION;
#endif
#define PMIC_BUCK5_RO0_b5_ocp_state_START (0)
#define PMIC_BUCK5_RO0_b5_ocp_state_END (0)
#define PMIC_BUCK5_RO0_b5_ccm_state_START (1)
#define PMIC_BUCK5_RO0_b5_ccm_state_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b6_adj_rlx : 4;
        unsigned char b6_adj_clx : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK6_CTRL0_UNION;
#endif
#define PMIC_BUCK6_CTRL0_b6_adj_rlx_START (0)
#define PMIC_BUCK6_CTRL0_b6_adj_rlx_END (3)
#define PMIC_BUCK6_CTRL0_b6_adj_clx_START (4)
#define PMIC_BUCK6_CTRL0_b6_adj_clx_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b6_dmd_blanktime_sel : 1;
        unsigned char b6_cot_rlx : 1;
        unsigned char b6_cmp_ibias : 4;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK6_CTRL1_UNION;
#endif
#define PMIC_BUCK6_CTRL1_b6_dmd_blanktime_sel_START (0)
#define PMIC_BUCK6_CTRL1_b6_dmd_blanktime_sel_END (0)
#define PMIC_BUCK6_CTRL1_b6_cot_rlx_START (1)
#define PMIC_BUCK6_CTRL1_b6_cot_rlx_END (1)
#define PMIC_BUCK6_CTRL1_b6_cmp_ibias_START (2)
#define PMIC_BUCK6_CTRL1_b6_cmp_ibias_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b6_dmd_sel_eco : 4;
        unsigned char b6_dmd_sel : 4;
    } reg;
} PMIC_BUCK6_CTRL2_UNION;
#endif
#define PMIC_BUCK6_CTRL2_b6_dmd_sel_eco_START (0)
#define PMIC_BUCK6_CTRL2_b6_dmd_sel_eco_END (3)
#define PMIC_BUCK6_CTRL2_b6_dmd_sel_START (4)
#define PMIC_BUCK6_CTRL2_b6_dmd_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b6_ea_eco_dis : 1;
        unsigned char b6_ea_clamp_eco_dis : 1;
        unsigned char b6_dt_sel : 1;
        unsigned char b6_dmd_type_sel : 1;
        unsigned char b6_dmd_ton : 3;
        unsigned char b6_dmd_shield_ton : 1;
    } reg;
} PMIC_BUCK6_CTRL3_UNION;
#endif
#define PMIC_BUCK6_CTRL3_b6_ea_eco_dis_START (0)
#define PMIC_BUCK6_CTRL3_b6_ea_eco_dis_END (0)
#define PMIC_BUCK6_CTRL3_b6_ea_clamp_eco_dis_START (1)
#define PMIC_BUCK6_CTRL3_b6_ea_clamp_eco_dis_END (1)
#define PMIC_BUCK6_CTRL3_b6_dt_sel_START (2)
#define PMIC_BUCK6_CTRL3_b6_dt_sel_END (2)
#define PMIC_BUCK6_CTRL3_b6_dmd_type_sel_START (3)
#define PMIC_BUCK6_CTRL3_b6_dmd_type_sel_END (3)
#define PMIC_BUCK6_CTRL3_b6_dmd_ton_START (4)
#define PMIC_BUCK6_CTRL3_b6_dmd_ton_END (6)
#define PMIC_BUCK6_CTRL3_b6_dmd_shield_ton_START (7)
#define PMIC_BUCK6_CTRL3_b6_dmd_shield_ton_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b6_lx_dt : 1;
        unsigned char b6_ibias_dmd_ctrl : 2;
        unsigned char b6_fix_ton : 1;
        unsigned char b6_fb_cap_sel : 1;
        unsigned char b6_eco_ibias_sel : 1;
        unsigned char b6_ea_ibias_sel : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK6_CTRL4_UNION;
#endif
#define PMIC_BUCK6_CTRL4_b6_lx_dt_START (0)
#define PMIC_BUCK6_CTRL4_b6_lx_dt_END (0)
#define PMIC_BUCK6_CTRL4_b6_ibias_dmd_ctrl_START (1)
#define PMIC_BUCK6_CTRL4_b6_ibias_dmd_ctrl_END (2)
#define PMIC_BUCK6_CTRL4_b6_fix_ton_START (3)
#define PMIC_BUCK6_CTRL4_b6_fix_ton_END (3)
#define PMIC_BUCK6_CTRL4_b6_fb_cap_sel_START (4)
#define PMIC_BUCK6_CTRL4_b6_fb_cap_sel_END (4)
#define PMIC_BUCK6_CTRL4_b6_eco_ibias_sel_START (5)
#define PMIC_BUCK6_CTRL4_b6_eco_ibias_sel_END (5)
#define PMIC_BUCK6_CTRL4_b6_ea_ibias_sel_START (6)
#define PMIC_BUCK6_CTRL4_b6_ea_ibias_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b6_ng_n_sel : 3;
        unsigned char b6_min_ton : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK6_CTRL5_UNION;
#endif
#define PMIC_BUCK6_CTRL5_b6_ng_n_sel_START (0)
#define PMIC_BUCK6_CTRL5_b6_ng_n_sel_END (2)
#define PMIC_BUCK6_CTRL5_b6_min_ton_START (3)
#define PMIC_BUCK6_CTRL5_b6_min_ton_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b6_ocp_cmp_ibias : 1;
        unsigned char b6_ntp_dt_sel : 2;
        unsigned char b6_ntp_dt_ctrl : 1;
        unsigned char b6_nonlinear_driver : 1;
        unsigned char b6_ng_p_sel : 3;
    } reg;
} PMIC_BUCK6_CTRL6_UNION;
#endif
#define PMIC_BUCK6_CTRL6_b6_ocp_cmp_ibias_START (0)
#define PMIC_BUCK6_CTRL6_b6_ocp_cmp_ibias_END (0)
#define PMIC_BUCK6_CTRL6_b6_ntp_dt_sel_START (1)
#define PMIC_BUCK6_CTRL6_b6_ntp_dt_sel_END (2)
#define PMIC_BUCK6_CTRL6_b6_ntp_dt_ctrl_START (3)
#define PMIC_BUCK6_CTRL6_b6_ntp_dt_ctrl_END (3)
#define PMIC_BUCK6_CTRL6_b6_nonlinear_driver_START (4)
#define PMIC_BUCK6_CTRL6_b6_nonlinear_driver_END (4)
#define PMIC_BUCK6_CTRL6_b6_ng_p_sel_START (5)
#define PMIC_BUCK6_CTRL6_b6_ng_p_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b6_ocpibias_sel : 2;
        unsigned char b6_ocp_toff : 2;
        unsigned char b6_ocp_sel : 3;
        unsigned char b6_ocp_delay : 1;
    } reg;
} PMIC_BUCK6_CTRL7_UNION;
#endif
#define PMIC_BUCK6_CTRL7_b6_ocpibias_sel_START (0)
#define PMIC_BUCK6_CTRL7_b6_ocpibias_sel_END (1)
#define PMIC_BUCK6_CTRL7_b6_ocp_toff_START (2)
#define PMIC_BUCK6_CTRL7_b6_ocp_toff_END (3)
#define PMIC_BUCK6_CTRL7_b6_ocp_sel_START (4)
#define PMIC_BUCK6_CTRL7_b6_ocp_sel_END (6)
#define PMIC_BUCK6_CTRL7_b6_ocp_delay_START (7)
#define PMIC_BUCK6_CTRL7_b6_ocp_delay_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b6_pg_p_sel : 3;
        unsigned char b6_pg_n_sel : 3;
        unsigned char b6_offres_shut : 1;
        unsigned char b6_offres_sel : 1;
    } reg;
} PMIC_BUCK6_CTRL8_UNION;
#endif
#define PMIC_BUCK6_CTRL8_b6_pg_p_sel_START (0)
#define PMIC_BUCK6_CTRL8_b6_pg_p_sel_END (2)
#define PMIC_BUCK6_CTRL8_b6_pg_n_sel_START (3)
#define PMIC_BUCK6_CTRL8_b6_pg_n_sel_END (5)
#define PMIC_BUCK6_CTRL8_b6_offres_shut_START (6)
#define PMIC_BUCK6_CTRL8_b6_offres_shut_END (6)
#define PMIC_BUCK6_CTRL8_b6_offres_sel_START (7)
#define PMIC_BUCK6_CTRL8_b6_offres_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b6_reg_dr : 3;
        unsigned char b6_ramp_ton_ctrl : 1;
        unsigned char b6_ramp_prebias : 1;
        unsigned char b6_ptn_dt_sel : 2;
        unsigned char b6_ptn_dt_ctrl : 1;
    } reg;
} PMIC_BUCK6_CTRL9_UNION;
#endif
#define PMIC_BUCK6_CTRL9_b6_reg_dr_START (0)
#define PMIC_BUCK6_CTRL9_b6_reg_dr_END (2)
#define PMIC_BUCK6_CTRL9_b6_ramp_ton_ctrl_START (3)
#define PMIC_BUCK6_CTRL9_b6_ramp_ton_ctrl_END (3)
#define PMIC_BUCK6_CTRL9_b6_ramp_prebias_START (4)
#define PMIC_BUCK6_CTRL9_b6_ramp_prebias_END (4)
#define PMIC_BUCK6_CTRL9_b6_ptn_dt_sel_START (5)
#define PMIC_BUCK6_CTRL9_b6_ptn_dt_sel_END (6)
#define PMIC_BUCK6_CTRL9_b6_ptn_dt_ctrl_START (7)
#define PMIC_BUCK6_CTRL9_b6_ptn_dt_ctrl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b6_reg_r : 2;
        unsigned char b6_reg_op_c : 1;
        unsigned char b6_reg_ibias : 1;
        unsigned char b6_reg_en : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_BUCK6_CTRL10_UNION;
#endif
#define PMIC_BUCK6_CTRL10_b6_reg_r_START (0)
#define PMIC_BUCK6_CTRL10_b6_reg_r_END (1)
#define PMIC_BUCK6_CTRL10_b6_reg_op_c_START (2)
#define PMIC_BUCK6_CTRL10_b6_reg_op_c_END (2)
#define PMIC_BUCK6_CTRL10_b6_reg_ibias_START (3)
#define PMIC_BUCK6_CTRL10_b6_reg_ibias_END (3)
#define PMIC_BUCK6_CTRL10_b6_reg_en_START (4)
#define PMIC_BUCK6_CTRL10_b6_reg_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b6_reserve0 : 8;
    } reg;
} PMIC_BUCK6_CTRL11_UNION;
#endif
#define PMIC_BUCK6_CTRL11_b6_reserve0_START (0)
#define PMIC_BUCK6_CTRL11_b6_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b6_reserve1 : 8;
    } reg;
} PMIC_BUCK6_CTRL12_UNION;
#endif
#define PMIC_BUCK6_CTRL12_b6_reserve1_START (0)
#define PMIC_BUCK6_CTRL12_b6_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b6_reserve2 : 8;
    } reg;
} PMIC_BUCK6_CTRL13_UNION;
#endif
#define PMIC_BUCK6_CTRL13_b6_reserve2_START (0)
#define PMIC_BUCK6_CTRL13_b6_reserve2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b6_softime_sel : 2;
        unsigned char b6_sense_ibias_dmd : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_BUCK6_CTRL14_UNION;
#endif
#define PMIC_BUCK6_CTRL14_b6_softime_sel_START (0)
#define PMIC_BUCK6_CTRL14_b6_softime_sel_END (1)
#define PMIC_BUCK6_CTRL14_b6_sense_ibias_dmd_START (2)
#define PMIC_BUCK6_CTRL14_b6_sense_ibias_dmd_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b6_ocp_state : 1;
        unsigned char b6_ccm_state : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_BUCK6_RO0_UNION;
#endif
#define PMIC_BUCK6_RO0_b6_ocp_state_START (0)
#define PMIC_BUCK6_RO0_b6_ocp_state_END (0)
#define PMIC_BUCK6_RO0_b6_ccm_state_START (1)
#define PMIC_BUCK6_RO0_b6_ccm_state_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b7_adj_rlx : 4;
        unsigned char b7_adj_clx : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK7_CTRL0_UNION;
#endif
#define PMIC_BUCK7_CTRL0_b7_adj_rlx_START (0)
#define PMIC_BUCK7_CTRL0_b7_adj_rlx_END (3)
#define PMIC_BUCK7_CTRL0_b7_adj_clx_START (4)
#define PMIC_BUCK7_CTRL0_b7_adj_clx_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b7_dmd_blanktime_sel : 1;
        unsigned char b7_cot_rlx : 1;
        unsigned char b7_cmp_ibias : 4;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK7_CTRL1_UNION;
#endif
#define PMIC_BUCK7_CTRL1_b7_dmd_blanktime_sel_START (0)
#define PMIC_BUCK7_CTRL1_b7_dmd_blanktime_sel_END (0)
#define PMIC_BUCK7_CTRL1_b7_cot_rlx_START (1)
#define PMIC_BUCK7_CTRL1_b7_cot_rlx_END (1)
#define PMIC_BUCK7_CTRL1_b7_cmp_ibias_START (2)
#define PMIC_BUCK7_CTRL1_b7_cmp_ibias_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b7_dmd_sel_eco : 4;
        unsigned char b7_dmd_sel : 4;
    } reg;
} PMIC_BUCK7_CTRL2_UNION;
#endif
#define PMIC_BUCK7_CTRL2_b7_dmd_sel_eco_START (0)
#define PMIC_BUCK7_CTRL2_b7_dmd_sel_eco_END (3)
#define PMIC_BUCK7_CTRL2_b7_dmd_sel_START (4)
#define PMIC_BUCK7_CTRL2_b7_dmd_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b7_ea_eco_dis : 1;
        unsigned char b7_ea_clamp_eco_dis : 1;
        unsigned char b7_dt_sel : 1;
        unsigned char b7_dmd_type_sel : 1;
        unsigned char b7_dmd_ton : 3;
        unsigned char b7_dmd_shield_ton : 1;
    } reg;
} PMIC_BUCK7_CTRL3_UNION;
#endif
#define PMIC_BUCK7_CTRL3_b7_ea_eco_dis_START (0)
#define PMIC_BUCK7_CTRL3_b7_ea_eco_dis_END (0)
#define PMIC_BUCK7_CTRL3_b7_ea_clamp_eco_dis_START (1)
#define PMIC_BUCK7_CTRL3_b7_ea_clamp_eco_dis_END (1)
#define PMIC_BUCK7_CTRL3_b7_dt_sel_START (2)
#define PMIC_BUCK7_CTRL3_b7_dt_sel_END (2)
#define PMIC_BUCK7_CTRL3_b7_dmd_type_sel_START (3)
#define PMIC_BUCK7_CTRL3_b7_dmd_type_sel_END (3)
#define PMIC_BUCK7_CTRL3_b7_dmd_ton_START (4)
#define PMIC_BUCK7_CTRL3_b7_dmd_ton_END (6)
#define PMIC_BUCK7_CTRL3_b7_dmd_shield_ton_START (7)
#define PMIC_BUCK7_CTRL3_b7_dmd_shield_ton_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b7_fix_ton : 1;
        unsigned char b7_fb_cap_sel : 1;
        unsigned char b7_fastramp_ctrl : 4;
        unsigned char b7_eco_ibias_sel : 1;
        unsigned char b7_ea_ibias_sel : 1;
    } reg;
} PMIC_BUCK7_CTRL4_UNION;
#endif
#define PMIC_BUCK7_CTRL4_b7_fix_ton_START (0)
#define PMIC_BUCK7_CTRL4_b7_fix_ton_END (0)
#define PMIC_BUCK7_CTRL4_b7_fb_cap_sel_START (1)
#define PMIC_BUCK7_CTRL4_b7_fb_cap_sel_END (1)
#define PMIC_BUCK7_CTRL4_b7_fastramp_ctrl_START (2)
#define PMIC_BUCK7_CTRL4_b7_fastramp_ctrl_END (5)
#define PMIC_BUCK7_CTRL4_b7_eco_ibias_sel_START (6)
#define PMIC_BUCK7_CTRL4_b7_eco_ibias_sel_END (6)
#define PMIC_BUCK7_CTRL4_b7_ea_ibias_sel_START (7)
#define PMIC_BUCK7_CTRL4_b7_ea_ibias_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b7_min_ton : 3;
        unsigned char b7_lx_dt : 1;
        unsigned char b7_ibias_dmd_ctrl : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK7_CTRL5_UNION;
#endif
#define PMIC_BUCK7_CTRL5_b7_min_ton_START (0)
#define PMIC_BUCK7_CTRL5_b7_min_ton_END (2)
#define PMIC_BUCK7_CTRL5_b7_lx_dt_START (3)
#define PMIC_BUCK7_CTRL5_b7_lx_dt_END (3)
#define PMIC_BUCK7_CTRL5_b7_ibias_dmd_ctrl_START (4)
#define PMIC_BUCK7_CTRL5_b7_ibias_dmd_ctrl_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b7_ntp_dt_ctrl : 1;
        unsigned char b7_nonlinear_driver : 1;
        unsigned char b7_ng_p_sel : 3;
        unsigned char b7_ng_n_sel : 3;
    } reg;
} PMIC_BUCK7_CTRL6_UNION;
#endif
#define PMIC_BUCK7_CTRL6_b7_ntp_dt_ctrl_START (0)
#define PMIC_BUCK7_CTRL6_b7_ntp_dt_ctrl_END (0)
#define PMIC_BUCK7_CTRL6_b7_nonlinear_driver_START (1)
#define PMIC_BUCK7_CTRL6_b7_nonlinear_driver_END (1)
#define PMIC_BUCK7_CTRL6_b7_ng_p_sel_START (2)
#define PMIC_BUCK7_CTRL6_b7_ng_p_sel_END (4)
#define PMIC_BUCK7_CTRL6_b7_ng_n_sel_START (5)
#define PMIC_BUCK7_CTRL6_b7_ng_n_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b7_ocp_sel : 3;
        unsigned char b7_ocp_delay : 1;
        unsigned char b7_ocp_cmp_ibias : 1;
        unsigned char b7_ntp_dt_sel : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK7_CTRL7_UNION;
#endif
#define PMIC_BUCK7_CTRL7_b7_ocp_sel_START (0)
#define PMIC_BUCK7_CTRL7_b7_ocp_sel_END (2)
#define PMIC_BUCK7_CTRL7_b7_ocp_delay_START (3)
#define PMIC_BUCK7_CTRL7_b7_ocp_delay_END (3)
#define PMIC_BUCK7_CTRL7_b7_ocp_cmp_ibias_START (4)
#define PMIC_BUCK7_CTRL7_b7_ocp_cmp_ibias_END (4)
#define PMIC_BUCK7_CTRL7_b7_ntp_dt_sel_START (5)
#define PMIC_BUCK7_CTRL7_b7_ntp_dt_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b7_offres_shut : 1;
        unsigned char b7_offres_sel : 1;
        unsigned char b7_ocpibias_sel : 2;
        unsigned char b7_ocp_toff : 2;
        unsigned char reserved : 2;
    } reg;
} PMIC_BUCK7_CTRL8_UNION;
#endif
#define PMIC_BUCK7_CTRL8_b7_offres_shut_START (0)
#define PMIC_BUCK7_CTRL8_b7_offres_shut_END (0)
#define PMIC_BUCK7_CTRL8_b7_offres_sel_START (1)
#define PMIC_BUCK7_CTRL8_b7_offres_sel_END (1)
#define PMIC_BUCK7_CTRL8_b7_ocpibias_sel_START (2)
#define PMIC_BUCK7_CTRL8_b7_ocpibias_sel_END (3)
#define PMIC_BUCK7_CTRL8_b7_ocp_toff_START (4)
#define PMIC_BUCK7_CTRL8_b7_ocp_toff_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b7_ptn_dt_ctrl : 1;
        unsigned char b7_pg_p_sel : 3;
        unsigned char b7_pg_n_sel : 3;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK7_CTRL9_UNION;
#endif
#define PMIC_BUCK7_CTRL9_b7_ptn_dt_ctrl_START (0)
#define PMIC_BUCK7_CTRL9_b7_ptn_dt_ctrl_END (0)
#define PMIC_BUCK7_CTRL9_b7_pg_p_sel_START (1)
#define PMIC_BUCK7_CTRL9_b7_pg_p_sel_END (3)
#define PMIC_BUCK7_CTRL9_b7_pg_n_sel_START (4)
#define PMIC_BUCK7_CTRL9_b7_pg_n_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b7_rampdown_ndmd : 3;
        unsigned char b7_ramp_ton_ctrl : 1;
        unsigned char b7_ramp_prebias : 1;
        unsigned char b7_ptn_dt_sel : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_BUCK7_CTRL10_UNION;
#endif
#define PMIC_BUCK7_CTRL10_b7_rampdown_ndmd_START (0)
#define PMIC_BUCK7_CTRL10_b7_rampdown_ndmd_END (2)
#define PMIC_BUCK7_CTRL10_b7_ramp_ton_ctrl_START (3)
#define PMIC_BUCK7_CTRL10_b7_ramp_ton_ctrl_END (3)
#define PMIC_BUCK7_CTRL10_b7_ramp_prebias_START (4)
#define PMIC_BUCK7_CTRL10_b7_ramp_prebias_END (4)
#define PMIC_BUCK7_CTRL10_b7_ptn_dt_sel_START (5)
#define PMIC_BUCK7_CTRL10_b7_ptn_dt_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b7_reg_r : 2;
        unsigned char b7_reg_op_c : 1;
        unsigned char b7_reg_ibias : 1;
        unsigned char b7_reg_en : 1;
        unsigned char b7_reg_dr : 3;
    } reg;
} PMIC_BUCK7_CTRL11_UNION;
#endif
#define PMIC_BUCK7_CTRL11_b7_reg_r_START (0)
#define PMIC_BUCK7_CTRL11_b7_reg_r_END (1)
#define PMIC_BUCK7_CTRL11_b7_reg_op_c_START (2)
#define PMIC_BUCK7_CTRL11_b7_reg_op_c_END (2)
#define PMIC_BUCK7_CTRL11_b7_reg_ibias_START (3)
#define PMIC_BUCK7_CTRL11_b7_reg_ibias_END (3)
#define PMIC_BUCK7_CTRL11_b7_reg_en_START (4)
#define PMIC_BUCK7_CTRL11_b7_reg_en_END (4)
#define PMIC_BUCK7_CTRL11_b7_reg_dr_START (5)
#define PMIC_BUCK7_CTRL11_b7_reg_dr_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b7_reserve0 : 8;
    } reg;
} PMIC_BUCK7_CTRL12_UNION;
#endif
#define PMIC_BUCK7_CTRL12_b7_reserve0_START (0)
#define PMIC_BUCK7_CTRL12_b7_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b7_reserve1 : 8;
    } reg;
} PMIC_BUCK7_CTRL13_UNION;
#endif
#define PMIC_BUCK7_CTRL13_b7_reserve1_START (0)
#define PMIC_BUCK7_CTRL13_b7_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b7_reserve2 : 8;
    } reg;
} PMIC_BUCK7_CTRL14_UNION;
#endif
#define PMIC_BUCK7_CTRL14_b7_reserve2_START (0)
#define PMIC_BUCK7_CTRL14_b7_reserve2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b7_softime_sel : 2;
        unsigned char b7_sense_ibias_dmd : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_BUCK7_CTRL15_UNION;
#endif
#define PMIC_BUCK7_CTRL15_b7_softime_sel_START (0)
#define PMIC_BUCK7_CTRL15_b7_softime_sel_END (1)
#define PMIC_BUCK7_CTRL15_b7_sense_ibias_dmd_START (2)
#define PMIC_BUCK7_CTRL15_b7_sense_ibias_dmd_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b7_ocp_state : 1;
        unsigned char b7_ccm_state : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_BUCK7_RO0_UNION;
#endif
#define PMIC_BUCK7_RO0_b7_ocp_state_START (0)
#define PMIC_BUCK7_RO0_b7_ocp_state_END (0)
#define PMIC_BUCK7_RO0_b7_ccm_state_START (1)
#define PMIC_BUCK7_RO0_b7_ccm_state_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo0_ocpd_vos_set : 1;
        unsigned char ldo0_ocpd_set : 2;
        unsigned char ldo0_ocp_enn : 1;
        unsigned char ldo0_comp : 2;
        unsigned char ldo0_bypass_ctrl_enn : 1;
        unsigned char ldo0_bw_en : 1;
    } reg;
} PMIC_LDO0_CTRL0_UNION;
#endif
#define PMIC_LDO0_CTRL0_ldo0_ocpd_vos_set_START (0)
#define PMIC_LDO0_CTRL0_ldo0_ocpd_vos_set_END (0)
#define PMIC_LDO0_CTRL0_ldo0_ocpd_set_START (1)
#define PMIC_LDO0_CTRL0_ldo0_ocpd_set_END (2)
#define PMIC_LDO0_CTRL0_ldo0_ocp_enn_START (3)
#define PMIC_LDO0_CTRL0_ldo0_ocp_enn_END (3)
#define PMIC_LDO0_CTRL0_ldo0_comp_START (4)
#define PMIC_LDO0_CTRL0_ldo0_comp_END (5)
#define PMIC_LDO0_CTRL0_ldo0_bypass_ctrl_enn_START (6)
#define PMIC_LDO0_CTRL0_ldo0_bypass_ctrl_enn_END (6)
#define PMIC_LDO0_CTRL0_ldo0_bw_en_START (7)
#define PMIC_LDO0_CTRL0_ldo0_bw_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo0_vrset : 3;
        unsigned char ldo0_vgpr_en : 1;
        unsigned char ldo0_pd_set : 1;
        unsigned char ldo0_ocps_set : 2;
        unsigned char ldo0_ocps_en : 1;
    } reg;
} PMIC_LDO0_CTRL1_UNION;
#endif
#define PMIC_LDO0_CTRL1_ldo0_vrset_START (0)
#define PMIC_LDO0_CTRL1_ldo0_vrset_END (2)
#define PMIC_LDO0_CTRL1_ldo0_vgpr_en_START (3)
#define PMIC_LDO0_CTRL1_ldo0_vgpr_en_END (3)
#define PMIC_LDO0_CTRL1_ldo0_pd_set_START (4)
#define PMIC_LDO0_CTRL1_ldo0_pd_set_END (4)
#define PMIC_LDO0_CTRL1_ldo0_ocps_set_START (5)
#define PMIC_LDO0_CTRL1_ldo0_ocps_set_END (6)
#define PMIC_LDO0_CTRL1_ldo0_ocps_en_START (7)
#define PMIC_LDO0_CTRL1_ldo0_ocps_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo1_ocpd_vos_set : 1;
        unsigned char ldo1_ocpd_set : 2;
        unsigned char ldo1_ocp_enn : 1;
        unsigned char ldo1_comp : 3;
        unsigned char ldo1_bw_en : 1;
    } reg;
} PMIC_LDO1_CTRL0_UNION;
#endif
#define PMIC_LDO1_CTRL0_ldo1_ocpd_vos_set_START (0)
#define PMIC_LDO1_CTRL0_ldo1_ocpd_vos_set_END (0)
#define PMIC_LDO1_CTRL0_ldo1_ocpd_set_START (1)
#define PMIC_LDO1_CTRL0_ldo1_ocpd_set_END (2)
#define PMIC_LDO1_CTRL0_ldo1_ocp_enn_START (3)
#define PMIC_LDO1_CTRL0_ldo1_ocp_enn_END (3)
#define PMIC_LDO1_CTRL0_ldo1_comp_START (4)
#define PMIC_LDO1_CTRL0_ldo1_comp_END (6)
#define PMIC_LDO1_CTRL0_ldo1_bw_en_START (7)
#define PMIC_LDO1_CTRL0_ldo1_bw_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo1_psrr1_set : 3;
        unsigned char ldo1_ocps_set : 2;
        unsigned char ldo1_ocps_en : 1;
        unsigned char ldo1_ocps_bw_set : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO1_CTRL1_UNION;
#endif
#define PMIC_LDO1_CTRL1_ldo1_psrr1_set_START (0)
#define PMIC_LDO1_CTRL1_ldo1_psrr1_set_END (2)
#define PMIC_LDO1_CTRL1_ldo1_ocps_set_START (3)
#define PMIC_LDO1_CTRL1_ldo1_ocps_set_END (4)
#define PMIC_LDO1_CTRL1_ldo1_ocps_en_START (5)
#define PMIC_LDO1_CTRL1_ldo1_ocps_en_END (5)
#define PMIC_LDO1_CTRL1_ldo1_ocps_bw_set_START (6)
#define PMIC_LDO1_CTRL1_ldo1_ocps_bw_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo1_vgpr_en : 1;
        unsigned char ldo1_sst_set : 2;
        unsigned char ldo1_psrr2_set : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO1_CTRL2_UNION;
#endif
#define PMIC_LDO1_CTRL2_ldo1_vgpr_en_START (0)
#define PMIC_LDO1_CTRL2_ldo1_vgpr_en_END (0)
#define PMIC_LDO1_CTRL2_ldo1_sst_set_START (1)
#define PMIC_LDO1_CTRL2_ldo1_sst_set_END (2)
#define PMIC_LDO1_CTRL2_ldo1_psrr2_set_START (3)
#define PMIC_LDO1_CTRL2_ldo1_psrr2_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo1_vrset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO1_CTRL3_UNION;
#endif
#define PMIC_LDO1_CTRL3_ldo1_vrset_START (0)
#define PMIC_LDO1_CTRL3_ldo1_vrset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo2_stb_set : 2;
        unsigned char ldo2_ocp_set : 2;
        unsigned char ldo2_ocp_enn : 1;
        unsigned char ldo2_eco_set : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO2_CTRL0_UNION;
#endif
#define PMIC_LDO2_CTRL0_ldo2_stb_set_START (0)
#define PMIC_LDO2_CTRL0_ldo2_stb_set_END (1)
#define PMIC_LDO2_CTRL0_ldo2_ocp_set_START (2)
#define PMIC_LDO2_CTRL0_ldo2_ocp_set_END (3)
#define PMIC_LDO2_CTRL0_ldo2_ocp_enn_START (4)
#define PMIC_LDO2_CTRL0_ldo2_ocp_enn_END (4)
#define PMIC_LDO2_CTRL0_ldo2_eco_set_START (5)
#define PMIC_LDO2_CTRL0_ldo2_eco_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo2_vrset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO2_CTRL1_UNION;
#endif
#define PMIC_LDO2_CTRL1_ldo2_vrset_START (0)
#define PMIC_LDO2_CTRL1_ldo2_vrset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo3_psrr1_set : 3;
        unsigned char ldo3_psrr_en : 1;
        unsigned char ldo3_ocp_set : 2;
        unsigned char ldo3_ocp_enn : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO3_CTRL0_UNION;
#endif
#define PMIC_LDO3_CTRL0_ldo3_psrr1_set_START (0)
#define PMIC_LDO3_CTRL0_ldo3_psrr1_set_END (2)
#define PMIC_LDO3_CTRL0_ldo3_psrr_en_START (3)
#define PMIC_LDO3_CTRL0_ldo3_psrr_en_END (3)
#define PMIC_LDO3_CTRL0_ldo3_ocp_set_START (4)
#define PMIC_LDO3_CTRL0_ldo3_ocp_set_END (5)
#define PMIC_LDO3_CTRL0_ldo3_ocp_enn_START (6)
#define PMIC_LDO3_CTRL0_ldo3_ocp_enn_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo3_vbias_set : 1;
        unsigned char ldo3_sst_set : 3;
        unsigned char ldo3_psrr2_set : 3;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO3_CTRL1_UNION;
#endif
#define PMIC_LDO3_CTRL1_ldo3_vbias_set_START (0)
#define PMIC_LDO3_CTRL1_ldo3_vbias_set_END (0)
#define PMIC_LDO3_CTRL1_ldo3_sst_set_START (1)
#define PMIC_LDO3_CTRL1_ldo3_sst_set_END (3)
#define PMIC_LDO3_CTRL1_ldo3_psrr2_set_START (4)
#define PMIC_LDO3_CTRL1_ldo3_psrr2_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo3_vrset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO3_CTRL2_UNION;
#endif
#define PMIC_LDO3_CTRL2_ldo3_vrset_START (0)
#define PMIC_LDO3_CTRL2_ldo3_vrset_END (2)
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
} PMIC_LDO4_CTRL0_UNION;
#endif
#define PMIC_LDO4_CTRL0_ldo4_vrset_START (0)
#define PMIC_LDO4_CTRL0_ldo4_vrset_END (2)
#define PMIC_LDO4_CTRL0_ldo4_ocp_enn_START (3)
#define PMIC_LDO4_CTRL0_ldo4_ocp_enn_END (3)
#define PMIC_LDO4_CTRL0_ldo4_eco_set_START (4)
#define PMIC_LDO4_CTRL0_ldo4_eco_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo5_vrset : 3;
        unsigned char ldo5_ocp_enn : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO5_CTRL0_UNION;
#endif
#define PMIC_LDO5_CTRL0_ldo5_vrset_START (0)
#define PMIC_LDO5_CTRL0_ldo5_vrset_END (2)
#define PMIC_LDO5_CTRL0_ldo5_ocp_enn_START (3)
#define PMIC_LDO5_CTRL0_ldo5_ocp_enn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo6_ocpd_set : 2;
        unsigned char ldo6_ocp_enn : 1;
        unsigned char ldo6_eco_set : 1;
        unsigned char ldo6_comp : 2;
        unsigned char ldo6_bypass_ctrl_enn : 1;
        unsigned char ldo6_bw_en : 1;
    } reg;
} PMIC_LDO6_CTRL0_UNION;
#endif
#define PMIC_LDO6_CTRL0_ldo6_ocpd_set_START (0)
#define PMIC_LDO6_CTRL0_ldo6_ocpd_set_END (1)
#define PMIC_LDO6_CTRL0_ldo6_ocp_enn_START (2)
#define PMIC_LDO6_CTRL0_ldo6_ocp_enn_END (2)
#define PMIC_LDO6_CTRL0_ldo6_eco_set_START (3)
#define PMIC_LDO6_CTRL0_ldo6_eco_set_END (3)
#define PMIC_LDO6_CTRL0_ldo6_comp_START (4)
#define PMIC_LDO6_CTRL0_ldo6_comp_END (5)
#define PMIC_LDO6_CTRL0_ldo6_bypass_ctrl_enn_START (6)
#define PMIC_LDO6_CTRL0_ldo6_bypass_ctrl_enn_END (6)
#define PMIC_LDO6_CTRL0_ldo6_bw_en_START (7)
#define PMIC_LDO6_CTRL0_ldo6_bw_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo6_vgpr_en : 1;
        unsigned char ldo6_vea_ctrl : 1;
        unsigned char ldo6_sst_set : 1;
        unsigned char ldo6_pd_set : 1;
        unsigned char ldo6_ocps_set : 2;
        unsigned char ldo6_ocps_en : 1;
        unsigned char ldo6_ocpd_vos_set : 1;
    } reg;
} PMIC_LDO6_CTRL1_UNION;
#endif
#define PMIC_LDO6_CTRL1_ldo6_vgpr_en_START (0)
#define PMIC_LDO6_CTRL1_ldo6_vgpr_en_END (0)
#define PMIC_LDO6_CTRL1_ldo6_vea_ctrl_START (1)
#define PMIC_LDO6_CTRL1_ldo6_vea_ctrl_END (1)
#define PMIC_LDO6_CTRL1_ldo6_sst_set_START (2)
#define PMIC_LDO6_CTRL1_ldo6_sst_set_END (2)
#define PMIC_LDO6_CTRL1_ldo6_pd_set_START (3)
#define PMIC_LDO6_CTRL1_ldo6_pd_set_END (3)
#define PMIC_LDO6_CTRL1_ldo6_ocps_set_START (4)
#define PMIC_LDO6_CTRL1_ldo6_ocps_set_END (5)
#define PMIC_LDO6_CTRL1_ldo6_ocps_en_START (6)
#define PMIC_LDO6_CTRL1_ldo6_ocps_en_END (6)
#define PMIC_LDO6_CTRL1_ldo6_ocpd_vos_set_START (7)
#define PMIC_LDO6_CTRL1_ldo6_ocpd_vos_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo6_vrset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO6_CTRL2_UNION;
#endif
#define PMIC_LDO6_CTRL2_ldo6_vrset_START (0)
#define PMIC_LDO6_CTRL2_ldo6_vrset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo8_vrset : 3;
        unsigned char ldo8_ocp_set : 1;
        unsigned char ldo8_ocp_enn : 1;
        unsigned char ldo8_eco_set : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO8_CTRL0_UNION;
#endif
#define PMIC_LDO8_CTRL0_ldo8_vrset_START (0)
#define PMIC_LDO8_CTRL0_ldo8_vrset_END (2)
#define PMIC_LDO8_CTRL0_ldo8_ocp_set_START (3)
#define PMIC_LDO8_CTRL0_ldo8_ocp_set_END (3)
#define PMIC_LDO8_CTRL0_ldo8_ocp_enn_START (4)
#define PMIC_LDO8_CTRL0_ldo8_ocp_enn_END (4)
#define PMIC_LDO8_CTRL0_ldo8_eco_set_START (5)
#define PMIC_LDO8_CTRL0_ldo8_eco_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo9_vrset : 3;
        unsigned char ldo9_ocp_enn : 1;
        unsigned char ldo9_eco_config : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO9_CTRL0_UNION;
#endif
#define PMIC_LDO9_CTRL0_ldo9_vrset_START (0)
#define PMIC_LDO9_CTRL0_ldo9_vrset_END (2)
#define PMIC_LDO9_CTRL0_ldo9_ocp_enn_START (3)
#define PMIC_LDO9_CTRL0_ldo9_ocp_enn_END (3)
#define PMIC_LDO9_CTRL0_ldo9_eco_config_START (4)
#define PMIC_LDO9_CTRL0_ldo9_eco_config_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo11_vrset : 3;
        unsigned char ldo11_ocp_enn : 1;
        unsigned char ldo11_eco_config : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO11_CTRL0_UNION;
#endif
#define PMIC_LDO11_CTRL0_ldo11_vrset_START (0)
#define PMIC_LDO11_CTRL0_ldo11_vrset_END (2)
#define PMIC_LDO11_CTRL0_ldo11_ocp_enn_START (3)
#define PMIC_LDO11_CTRL0_ldo11_ocp_enn_END (3)
#define PMIC_LDO11_CTRL0_ldo11_eco_config_START (4)
#define PMIC_LDO11_CTRL0_ldo11_eco_config_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo12_vrset : 3;
        unsigned char ldo12_ocp_enn : 1;
        unsigned char ldo12_eco_config : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO12_CTRL0_UNION;
#endif
#define PMIC_LDO12_CTRL0_ldo12_vrset_START (0)
#define PMIC_LDO12_CTRL0_ldo12_vrset_END (2)
#define PMIC_LDO12_CTRL0_ldo12_ocp_enn_START (3)
#define PMIC_LDO12_CTRL0_ldo12_ocp_enn_END (3)
#define PMIC_LDO12_CTRL0_ldo12_eco_config_START (4)
#define PMIC_LDO12_CTRL0_ldo12_eco_config_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo13_vrset : 3;
        unsigned char ldo13_ocp_set : 1;
        unsigned char ldo13_ocp_enn : 1;
        unsigned char ldo13_eco_set : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO13_CTRL0_UNION;
#endif
#define PMIC_LDO13_CTRL0_ldo13_vrset_START (0)
#define PMIC_LDO13_CTRL0_ldo13_vrset_END (2)
#define PMIC_LDO13_CTRL0_ldo13_ocp_set_START (3)
#define PMIC_LDO13_CTRL0_ldo13_ocp_set_END (3)
#define PMIC_LDO13_CTRL0_ldo13_ocp_enn_START (4)
#define PMIC_LDO13_CTRL0_ldo13_ocp_enn_END (4)
#define PMIC_LDO13_CTRL0_ldo13_eco_set_START (5)
#define PMIC_LDO13_CTRL0_ldo13_eco_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo14_vrset : 3;
        unsigned char ldo14_ocp_enn : 1;
        unsigned char ldo14_eco_config : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO14_CTRL0_UNION;
#endif
#define PMIC_LDO14_CTRL0_ldo14_vrset_START (0)
#define PMIC_LDO14_CTRL0_ldo14_vrset_END (2)
#define PMIC_LDO14_CTRL0_ldo14_ocp_enn_START (3)
#define PMIC_LDO14_CTRL0_ldo14_ocp_enn_END (3)
#define PMIC_LDO14_CTRL0_ldo14_eco_config_START (4)
#define PMIC_LDO14_CTRL0_ldo14_eco_config_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo15_vrset : 3;
        unsigned char ldo15_ocp_set : 2;
        unsigned char ldo15_ocp_enn : 1;
        unsigned char ldo15_eco_set_enn : 1;
        unsigned char ldo15_eco_config : 1;
    } reg;
} PMIC_LDO15_CTRL0_UNION;
#endif
#define PMIC_LDO15_CTRL0_ldo15_vrset_START (0)
#define PMIC_LDO15_CTRL0_ldo15_vrset_END (2)
#define PMIC_LDO15_CTRL0_ldo15_ocp_set_START (3)
#define PMIC_LDO15_CTRL0_ldo15_ocp_set_END (4)
#define PMIC_LDO15_CTRL0_ldo15_ocp_enn_START (5)
#define PMIC_LDO15_CTRL0_ldo15_ocp_enn_END (5)
#define PMIC_LDO15_CTRL0_ldo15_eco_set_enn_START (6)
#define PMIC_LDO15_CTRL0_ldo15_eco_set_enn_END (6)
#define PMIC_LDO15_CTRL0_ldo15_eco_config_START (7)
#define PMIC_LDO15_CTRL0_ldo15_eco_config_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo16_ocp_set : 2;
        unsigned char ldo16_ocp_enn : 1;
        unsigned char ldo16_eco_set_enn : 1;
        unsigned char ldo16_eco_config : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO16_CTRL0_UNION;
#endif
#define PMIC_LDO16_CTRL0_ldo16_ocp_set_START (0)
#define PMIC_LDO16_CTRL0_ldo16_ocp_set_END (1)
#define PMIC_LDO16_CTRL0_ldo16_ocp_enn_START (2)
#define PMIC_LDO16_CTRL0_ldo16_ocp_enn_END (2)
#define PMIC_LDO16_CTRL0_ldo16_eco_set_enn_START (3)
#define PMIC_LDO16_CTRL0_ldo16_eco_set_enn_END (3)
#define PMIC_LDO16_CTRL0_ldo16_eco_config_START (4)
#define PMIC_LDO16_CTRL0_ldo16_eco_config_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo16_vrset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO16_CTRL1_UNION;
#endif
#define PMIC_LDO16_CTRL1_ldo16_vrset_START (0)
#define PMIC_LDO16_CTRL1_ldo16_vrset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo17_vrset : 3;
        unsigned char ldo17_ocp_enn : 1;
        unsigned char ldo17_eco_set : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO17_CTRL0_UNION;
#endif
#define PMIC_LDO17_CTRL0_ldo17_vrset_START (0)
#define PMIC_LDO17_CTRL0_ldo17_vrset_END (2)
#define PMIC_LDO17_CTRL0_ldo17_ocp_enn_START (3)
#define PMIC_LDO17_CTRL0_ldo17_ocp_enn_END (3)
#define PMIC_LDO17_CTRL0_ldo17_eco_set_START (4)
#define PMIC_LDO17_CTRL0_ldo17_eco_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo18_vrset : 3;
        unsigned char ldo18_ocp_set : 2;
        unsigned char ldo18_ocp_enn : 1;
        unsigned char ldo18_eco_set : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO18_CTRL0_UNION;
#endif
#define PMIC_LDO18_CTRL0_ldo18_vrset_START (0)
#define PMIC_LDO18_CTRL0_ldo18_vrset_END (2)
#define PMIC_LDO18_CTRL0_ldo18_ocp_set_START (3)
#define PMIC_LDO18_CTRL0_ldo18_ocp_set_END (4)
#define PMIC_LDO18_CTRL0_ldo18_ocp_enn_START (5)
#define PMIC_LDO18_CTRL0_ldo18_ocp_enn_END (5)
#define PMIC_LDO18_CTRL0_ldo18_eco_set_START (6)
#define PMIC_LDO18_CTRL0_ldo18_eco_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo19_psrr2_set : 3;
        unsigned char ldo19_psrr1_set : 3;
        unsigned char ldo19_psrr_en : 1;
        unsigned char ldo19_ocp_enn : 1;
    } reg;
} PMIC_LDO19_CTRL0_UNION;
#endif
#define PMIC_LDO19_CTRL0_ldo19_psrr2_set_START (0)
#define PMIC_LDO19_CTRL0_ldo19_psrr2_set_END (2)
#define PMIC_LDO19_CTRL0_ldo19_psrr1_set_START (3)
#define PMIC_LDO19_CTRL0_ldo19_psrr1_set_END (5)
#define PMIC_LDO19_CTRL0_ldo19_psrr_en_START (6)
#define PMIC_LDO19_CTRL0_ldo19_psrr_en_END (6)
#define PMIC_LDO19_CTRL0_ldo19_ocp_enn_START (7)
#define PMIC_LDO19_CTRL0_ldo19_ocp_enn_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo19_vrset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO19_CTRL1_UNION;
#endif
#define PMIC_LDO19_CTRL1_ldo19_vrset_START (0)
#define PMIC_LDO19_CTRL1_ldo19_vrset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo20_ocp_enn : 1;
        unsigned char ldo20_comp : 3;
        unsigned char ldo20_bw_en : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO20_CTRL0_UNION;
#endif
#define PMIC_LDO20_CTRL0_ldo20_ocp_enn_START (0)
#define PMIC_LDO20_CTRL0_ldo20_ocp_enn_END (0)
#define PMIC_LDO20_CTRL0_ldo20_comp_START (1)
#define PMIC_LDO20_CTRL0_ldo20_comp_END (3)
#define PMIC_LDO20_CTRL0_ldo20_bw_en_START (4)
#define PMIC_LDO20_CTRL0_ldo20_bw_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo20_ocps_en : 1;
        unsigned char ldo20_ocpd_set : 4;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO20_CTRL1_UNION;
#endif
#define PMIC_LDO20_CTRL1_ldo20_ocps_en_START (0)
#define PMIC_LDO20_CTRL1_ldo20_ocps_en_END (0)
#define PMIC_LDO20_CTRL1_ldo20_ocpd_set_START (1)
#define PMIC_LDO20_CTRL1_ldo20_ocpd_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo20_psrr1_set : 3;
        unsigned char ldo20_psrr_en : 1;
        unsigned char ldo20_ocps_set : 4;
    } reg;
} PMIC_LDO20_CTRL2_UNION;
#endif
#define PMIC_LDO20_CTRL2_ldo20_psrr1_set_START (0)
#define PMIC_LDO20_CTRL2_ldo20_psrr1_set_END (2)
#define PMIC_LDO20_CTRL2_ldo20_psrr_en_START (3)
#define PMIC_LDO20_CTRL2_ldo20_psrr_en_END (3)
#define PMIC_LDO20_CTRL2_ldo20_ocps_set_START (4)
#define PMIC_LDO20_CTRL2_ldo20_ocps_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo20_vgpr_en : 1;
        unsigned char ldo20_sst_set : 2;
        unsigned char ldo20_psrr2_set : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO20_CTRL3_UNION;
#endif
#define PMIC_LDO20_CTRL3_ldo20_vgpr_en_START (0)
#define PMIC_LDO20_CTRL3_ldo20_vgpr_en_END (0)
#define PMIC_LDO20_CTRL3_ldo20_sst_set_START (1)
#define PMIC_LDO20_CTRL3_ldo20_sst_set_END (2)
#define PMIC_LDO20_CTRL3_ldo20_psrr2_set_START (3)
#define PMIC_LDO20_CTRL3_ldo20_psrr2_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo20_vrset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO20_CTRL4_UNION;
#endif
#define PMIC_LDO20_CTRL4_ldo20_vrset_START (0)
#define PMIC_LDO20_CTRL4_ldo20_vrset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo21_vrset : 3;
        unsigned char ldo21_ocp_enn : 1;
        unsigned char ldo21_eco_set : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO21_CTRL0_UNION;
#endif
#define PMIC_LDO21_CTRL0_ldo21_vrset_START (0)
#define PMIC_LDO21_CTRL0_ldo21_vrset_END (2)
#define PMIC_LDO21_CTRL0_ldo21_ocp_enn_START (3)
#define PMIC_LDO21_CTRL0_ldo21_ocp_enn_END (3)
#define PMIC_LDO21_CTRL0_ldo21_eco_set_START (4)
#define PMIC_LDO21_CTRL0_ldo21_eco_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo22_psrr1_set : 3;
        unsigned char ldo22_psrr_en : 1;
        unsigned char ldo22_ocp_enn : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO22_CTRL0_UNION;
#endif
#define PMIC_LDO22_CTRL0_ldo22_psrr1_set_START (0)
#define PMIC_LDO22_CTRL0_ldo22_psrr1_set_END (2)
#define PMIC_LDO22_CTRL0_ldo22_psrr_en_START (3)
#define PMIC_LDO22_CTRL0_ldo22_psrr_en_END (3)
#define PMIC_LDO22_CTRL0_ldo22_ocp_enn_START (4)
#define PMIC_LDO22_CTRL0_ldo22_ocp_enn_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo22_vrset : 3;
        unsigned char ldo22_psrr2_set : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO22_CTRL1_UNION;
#endif
#define PMIC_LDO22_CTRL1_ldo22_vrset_START (0)
#define PMIC_LDO22_CTRL1_ldo22_vrset_END (2)
#define PMIC_LDO22_CTRL1_ldo22_psrr2_set_START (3)
#define PMIC_LDO22_CTRL1_ldo22_psrr2_set_END (5)
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
} PMIC_LDO23_CTRL0_UNION;
#endif
#define PMIC_LDO23_CTRL0_ldo23_vrset_START (0)
#define PMIC_LDO23_CTRL0_ldo23_vrset_END (2)
#define PMIC_LDO23_CTRL0_ldo23_ocp_enn_START (3)
#define PMIC_LDO23_CTRL0_ldo23_ocp_enn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo24_ocp_enn : 1;
        unsigned char ldo24_en_bw : 1;
        unsigned char ldo24_eco_set : 1;
        unsigned char ldo24_bw_set : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO24_CTRL0_UNION;
#endif
#define PMIC_LDO24_CTRL0_ldo24_ocp_enn_START (0)
#define PMIC_LDO24_CTRL0_ldo24_ocp_enn_END (0)
#define PMIC_LDO24_CTRL0_ldo24_en_bw_START (1)
#define PMIC_LDO24_CTRL0_ldo24_en_bw_END (1)
#define PMIC_LDO24_CTRL0_ldo24_eco_set_START (2)
#define PMIC_LDO24_CTRL0_ldo24_eco_set_END (2)
#define PMIC_LDO24_CTRL0_ldo24_bw_set_START (3)
#define PMIC_LDO24_CTRL0_ldo24_bw_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo24_vrset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO24_CTRL1_UNION;
#endif
#define PMIC_LDO24_CTRL1_ldo24_vrset_START (0)
#define PMIC_LDO24_CTRL1_ldo24_vrset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo25_vrset : 3;
        unsigned char ldo25_ocp_set : 2;
        unsigned char ldo25_ocp_enn : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO25_CTRL0_UNION;
#endif
#define PMIC_LDO25_CTRL0_ldo25_vrset_START (0)
#define PMIC_LDO25_CTRL0_ldo25_vrset_END (2)
#define PMIC_LDO25_CTRL0_ldo25_ocp_set_START (3)
#define PMIC_LDO25_CTRL0_ldo25_ocp_set_END (4)
#define PMIC_LDO25_CTRL0_ldo25_ocp_enn_START (5)
#define PMIC_LDO25_CTRL0_ldo25_ocp_enn_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo26_vrset : 3;
        unsigned char ldo26_vgpr_en : 1;
        unsigned char ldo26_ocp_enn : 1;
        unsigned char ldo26_eco_set : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO26_CTRL0_UNION;
#endif
#define PMIC_LDO26_CTRL0_ldo26_vrset_START (0)
#define PMIC_LDO26_CTRL0_ldo26_vrset_END (2)
#define PMIC_LDO26_CTRL0_ldo26_vgpr_en_START (3)
#define PMIC_LDO26_CTRL0_ldo26_vgpr_en_END (3)
#define PMIC_LDO26_CTRL0_ldo26_ocp_enn_START (4)
#define PMIC_LDO26_CTRL0_ldo26_ocp_enn_END (4)
#define PMIC_LDO26_CTRL0_ldo26_eco_set_START (5)
#define PMIC_LDO26_CTRL0_ldo26_eco_set_END (5)
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
} PMIC_LDO27_CTRL0_UNION;
#endif
#define PMIC_LDO27_CTRL0_ldo27_vrset_START (0)
#define PMIC_LDO27_CTRL0_ldo27_vrset_END (2)
#define PMIC_LDO27_CTRL0_ldo27_ocp_enn_START (3)
#define PMIC_LDO27_CTRL0_ldo27_ocp_enn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo28_vrset : 3;
        unsigned char ldo28_ocp_enn : 1;
        unsigned char ldo28_eco_set : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO28_CTRL0_UNION;
#endif
#define PMIC_LDO28_CTRL0_ldo28_vrset_START (0)
#define PMIC_LDO28_CTRL0_ldo28_vrset_END (2)
#define PMIC_LDO28_CTRL0_ldo28_ocp_enn_START (3)
#define PMIC_LDO28_CTRL0_ldo28_ocp_enn_END (3)
#define PMIC_LDO28_CTRL0_ldo28_eco_set_START (4)
#define PMIC_LDO28_CTRL0_ldo28_eco_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo29_ocps_set : 2;
        unsigned char ldo29_ocps_en : 1;
        unsigned char ldo29_ocpd_set : 2;
        unsigned char ldo29_ocp_enn : 1;
        unsigned char ldo29_eco_set : 1;
        unsigned char ldo29_bw_en : 1;
    } reg;
} PMIC_LDO29_CTRL0_UNION;
#endif
#define PMIC_LDO29_CTRL0_ldo29_ocps_set_START (0)
#define PMIC_LDO29_CTRL0_ldo29_ocps_set_END (1)
#define PMIC_LDO29_CTRL0_ldo29_ocps_en_START (2)
#define PMIC_LDO29_CTRL0_ldo29_ocps_en_END (2)
#define PMIC_LDO29_CTRL0_ldo29_ocpd_set_START (3)
#define PMIC_LDO29_CTRL0_ldo29_ocpd_set_END (4)
#define PMIC_LDO29_CTRL0_ldo29_ocp_enn_START (5)
#define PMIC_LDO29_CTRL0_ldo29_ocp_enn_END (5)
#define PMIC_LDO29_CTRL0_ldo29_eco_set_START (6)
#define PMIC_LDO29_CTRL0_ldo29_eco_set_END (6)
#define PMIC_LDO29_CTRL0_ldo29_bw_en_START (7)
#define PMIC_LDO29_CTRL0_ldo29_bw_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo29_vrset : 3;
        unsigned char ldo29_vgpr_en : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO29_CTRL1_UNION;
#endif
#define PMIC_LDO29_CTRL1_ldo29_vrset_START (0)
#define PMIC_LDO29_CTRL1_ldo29_vrset_END (2)
#define PMIC_LDO29_CTRL1_ldo29_vgpr_en_START (3)
#define PMIC_LDO29_CTRL1_ldo29_vgpr_en_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo30_ocpd_vos_set : 1;
        unsigned char ldo30_ocpd_set : 2;
        unsigned char ldo30_ocp_enn : 1;
        unsigned char ldo30_comp : 2;
        unsigned char ldo30_bypass_ctrl_enn : 1;
        unsigned char ldo30_bw_en : 1;
    } reg;
} PMIC_LDO30_CTRL0_UNION;
#endif
#define PMIC_LDO30_CTRL0_ldo30_ocpd_vos_set_START (0)
#define PMIC_LDO30_CTRL0_ldo30_ocpd_vos_set_END (0)
#define PMIC_LDO30_CTRL0_ldo30_ocpd_set_START (1)
#define PMIC_LDO30_CTRL0_ldo30_ocpd_set_END (2)
#define PMIC_LDO30_CTRL0_ldo30_ocp_enn_START (3)
#define PMIC_LDO30_CTRL0_ldo30_ocp_enn_END (3)
#define PMIC_LDO30_CTRL0_ldo30_comp_START (4)
#define PMIC_LDO30_CTRL0_ldo30_comp_END (5)
#define PMIC_LDO30_CTRL0_ldo30_bypass_ctrl_enn_START (6)
#define PMIC_LDO30_CTRL0_ldo30_bypass_ctrl_enn_END (6)
#define PMIC_LDO30_CTRL0_ldo30_bw_en_START (7)
#define PMIC_LDO30_CTRL0_ldo30_bw_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo30_vgpr_en : 1;
        unsigned char ldo30_reserve_pull_down : 1;
        unsigned char ldo30_pd_set : 1;
        unsigned char ldo30_ocps_set : 2;
        unsigned char ldo30_ocps_en : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO30_CTRL1_UNION;
#endif
#define PMIC_LDO30_CTRL1_ldo30_vgpr_en_START (0)
#define PMIC_LDO30_CTRL1_ldo30_vgpr_en_END (0)
#define PMIC_LDO30_CTRL1_ldo30_reserve_pull_down_START (1)
#define PMIC_LDO30_CTRL1_ldo30_reserve_pull_down_END (1)
#define PMIC_LDO30_CTRL1_ldo30_pd_set_START (2)
#define PMIC_LDO30_CTRL1_ldo30_pd_set_END (2)
#define PMIC_LDO30_CTRL1_ldo30_ocps_set_START (3)
#define PMIC_LDO30_CTRL1_ldo30_ocps_set_END (4)
#define PMIC_LDO30_CTRL1_ldo30_ocps_en_START (5)
#define PMIC_LDO30_CTRL1_ldo30_ocps_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo30_vrset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO30_CTRL2_UNION;
#endif
#define PMIC_LDO30_CTRL2_ldo30_vrset_START (0)
#define PMIC_LDO30_CTRL2_ldo30_vrset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo31_vrset : 3;
        unsigned char ldo31_ocp_enn : 1;
        unsigned char ldo31_iss_en : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO31_CTRL0_UNION;
#endif
#define PMIC_LDO31_CTRL0_ldo31_vrset_START (0)
#define PMIC_LDO31_CTRL0_ldo31_vrset_END (2)
#define PMIC_LDO31_CTRL0_ldo31_ocp_enn_START (3)
#define PMIC_LDO31_CTRL0_ldo31_ocp_enn_END (3)
#define PMIC_LDO31_CTRL0_ldo31_iss_en_START (4)
#define PMIC_LDO31_CTRL0_ldo31_iss_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo32_ocp_enn : 1;
        unsigned char ldo32_comp : 3;
        unsigned char ldo32_bw_en : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO32_CTRL0_UNION;
#endif
#define PMIC_LDO32_CTRL0_ldo32_ocp_enn_START (0)
#define PMIC_LDO32_CTRL0_ldo32_ocp_enn_END (0)
#define PMIC_LDO32_CTRL0_ldo32_comp_START (1)
#define PMIC_LDO32_CTRL0_ldo32_comp_END (3)
#define PMIC_LDO32_CTRL0_ldo32_bw_en_START (4)
#define PMIC_LDO32_CTRL0_ldo32_bw_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo32_ocps_en : 1;
        unsigned char ldo32_ocpd_set : 4;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO32_CTRL1_UNION;
#endif
#define PMIC_LDO32_CTRL1_ldo32_ocps_en_START (0)
#define PMIC_LDO32_CTRL1_ldo32_ocps_en_END (0)
#define PMIC_LDO32_CTRL1_ldo32_ocpd_set_START (1)
#define PMIC_LDO32_CTRL1_ldo32_ocpd_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo32_psrr1_set : 3;
        unsigned char ldo32_psrr_en : 1;
        unsigned char ldo32_ocps_set : 4;
    } reg;
} PMIC_LDO32_CTRL2_UNION;
#endif
#define PMIC_LDO32_CTRL2_ldo32_psrr1_set_START (0)
#define PMIC_LDO32_CTRL2_ldo32_psrr1_set_END (2)
#define PMIC_LDO32_CTRL2_ldo32_psrr_en_START (3)
#define PMIC_LDO32_CTRL2_ldo32_psrr_en_END (3)
#define PMIC_LDO32_CTRL2_ldo32_ocps_set_START (4)
#define PMIC_LDO32_CTRL2_ldo32_ocps_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo32_vgpr_en : 1;
        unsigned char ldo32_sst_set : 2;
        unsigned char ldo32_psrr2_set : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO32_CTRL3_UNION;
#endif
#define PMIC_LDO32_CTRL3_ldo32_vgpr_en_START (0)
#define PMIC_LDO32_CTRL3_ldo32_vgpr_en_END (0)
#define PMIC_LDO32_CTRL3_ldo32_sst_set_START (1)
#define PMIC_LDO32_CTRL3_ldo32_sst_set_END (2)
#define PMIC_LDO32_CTRL3_ldo32_psrr2_set_START (3)
#define PMIC_LDO32_CTRL3_ldo32_psrr2_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo32_vrset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO32_CTRL4_UNION;
#endif
#define PMIC_LDO32_CTRL4_ldo32_vrset_START (0)
#define PMIC_LDO32_CTRL4_ldo32_vrset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo34_ocp_enn : 1;
        unsigned char ldo34_en_bw : 1;
        unsigned char ldo34_eco_set : 1;
        unsigned char ldo34_bw_set : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO34_CTRL0_UNION;
#endif
#define PMIC_LDO34_CTRL0_ldo34_ocp_enn_START (0)
#define PMIC_LDO34_CTRL0_ldo34_ocp_enn_END (0)
#define PMIC_LDO34_CTRL0_ldo34_en_bw_START (1)
#define PMIC_LDO34_CTRL0_ldo34_en_bw_END (1)
#define PMIC_LDO34_CTRL0_ldo34_eco_set_START (2)
#define PMIC_LDO34_CTRL0_ldo34_eco_set_END (2)
#define PMIC_LDO34_CTRL0_ldo34_bw_set_START (3)
#define PMIC_LDO34_CTRL0_ldo34_bw_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo34_vrset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO34_CTRL1_UNION;
#endif
#define PMIC_LDO34_CTRL1_ldo34_vrset_START (0)
#define PMIC_LDO34_CTRL1_ldo34_vrset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo37_ocp_enn : 1;
        unsigned char ldo37_eco_set : 1;
        unsigned char ldo37_comp : 3;
        unsigned char ldo37_bw_en : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO37_CTRL0_UNION;
#endif
#define PMIC_LDO37_CTRL0_ldo37_ocp_enn_START (0)
#define PMIC_LDO37_CTRL0_ldo37_ocp_enn_END (0)
#define PMIC_LDO37_CTRL0_ldo37_eco_set_START (1)
#define PMIC_LDO37_CTRL0_ldo37_eco_set_END (1)
#define PMIC_LDO37_CTRL0_ldo37_comp_START (2)
#define PMIC_LDO37_CTRL0_ldo37_comp_END (4)
#define PMIC_LDO37_CTRL0_ldo37_bw_en_START (5)
#define PMIC_LDO37_CTRL0_ldo37_bw_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo37_ocps_en : 1;
        unsigned char ldo37_ocpd_set : 4;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO37_CTRL1_UNION;
#endif
#define PMIC_LDO37_CTRL1_ldo37_ocps_en_START (0)
#define PMIC_LDO37_CTRL1_ldo37_ocps_en_END (0)
#define PMIC_LDO37_CTRL1_ldo37_ocpd_set_START (1)
#define PMIC_LDO37_CTRL1_ldo37_ocpd_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo37_psrr1_set : 3;
        unsigned char ldo37_psrr_en : 1;
        unsigned char ldo37_ocps_set : 4;
    } reg;
} PMIC_LDO37_CTRL2_UNION;
#endif
#define PMIC_LDO37_CTRL2_ldo37_psrr1_set_START (0)
#define PMIC_LDO37_CTRL2_ldo37_psrr1_set_END (2)
#define PMIC_LDO37_CTRL2_ldo37_psrr_en_START (3)
#define PMIC_LDO37_CTRL2_ldo37_psrr_en_END (3)
#define PMIC_LDO37_CTRL2_ldo37_ocps_set_START (4)
#define PMIC_LDO37_CTRL2_ldo37_ocps_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo37_vgpr_en : 1;
        unsigned char ldo37_sst_set : 1;
        unsigned char ldo37_ref_buf_rset : 2;
        unsigned char ldo37_ref_buf_bypass : 1;
        unsigned char ldo37_psrr2_set : 3;
    } reg;
} PMIC_LDO37_CTRL3_UNION;
#endif
#define PMIC_LDO37_CTRL3_ldo37_vgpr_en_START (0)
#define PMIC_LDO37_CTRL3_ldo37_vgpr_en_END (0)
#define PMIC_LDO37_CTRL3_ldo37_sst_set_START (1)
#define PMIC_LDO37_CTRL3_ldo37_sst_set_END (1)
#define PMIC_LDO37_CTRL3_ldo37_ref_buf_rset_START (2)
#define PMIC_LDO37_CTRL3_ldo37_ref_buf_rset_END (3)
#define PMIC_LDO37_CTRL3_ldo37_ref_buf_bypass_START (4)
#define PMIC_LDO37_CTRL3_ldo37_ref_buf_bypass_END (4)
#define PMIC_LDO37_CTRL3_ldo37_psrr2_set_START (5)
#define PMIC_LDO37_CTRL3_ldo37_psrr2_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo37_vrset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO37_CTRL4_UNION;
#endif
#define PMIC_LDO37_CTRL4_ldo37_vrset_START (0)
#define PMIC_LDO37_CTRL4_ldo37_vrset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo38_ocps_en : 1;
        unsigned char ldo38_ocpd_set : 3;
        unsigned char ldo38_ocp_enn : 1;
        unsigned char ldo38_eco_set : 1;
        unsigned char ldo38_bw_en : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO38_CTRL0_UNION;
#endif
#define PMIC_LDO38_CTRL0_ldo38_ocps_en_START (0)
#define PMIC_LDO38_CTRL0_ldo38_ocps_en_END (0)
#define PMIC_LDO38_CTRL0_ldo38_ocpd_set_START (1)
#define PMIC_LDO38_CTRL0_ldo38_ocpd_set_END (3)
#define PMIC_LDO38_CTRL0_ldo38_ocp_enn_START (4)
#define PMIC_LDO38_CTRL0_ldo38_ocp_enn_END (4)
#define PMIC_LDO38_CTRL0_ldo38_eco_set_START (5)
#define PMIC_LDO38_CTRL0_ldo38_eco_set_END (5)
#define PMIC_LDO38_CTRL0_ldo38_bw_en_START (6)
#define PMIC_LDO38_CTRL0_ldo38_bw_en_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo38_vrset : 3;
        unsigned char ldo38_vgpr_en : 1;
        unsigned char ldo38_ocps_set : 3;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO38_CTRL1_UNION;
#endif
#define PMIC_LDO38_CTRL1_ldo38_vrset_START (0)
#define PMIC_LDO38_CTRL1_ldo38_vrset_END (2)
#define PMIC_LDO38_CTRL1_ldo38_vgpr_en_START (3)
#define PMIC_LDO38_CTRL1_ldo38_vgpr_en_END (3)
#define PMIC_LDO38_CTRL1_ldo38_ocps_set_START (4)
#define PMIC_LDO38_CTRL1_ldo38_ocps_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo39_ocpd_set : 2;
        unsigned char ldo39_ocp_enn : 1;
        unsigned char ldo39_eco_set : 1;
        unsigned char ldo39_comp : 2;
        unsigned char ldo39_bypass_ctrl_enn : 1;
        unsigned char ldo39_bw_en : 1;
    } reg;
} PMIC_LDO39_CTRL0_UNION;
#endif
#define PMIC_LDO39_CTRL0_ldo39_ocpd_set_START (0)
#define PMIC_LDO39_CTRL0_ldo39_ocpd_set_END (1)
#define PMIC_LDO39_CTRL0_ldo39_ocp_enn_START (2)
#define PMIC_LDO39_CTRL0_ldo39_ocp_enn_END (2)
#define PMIC_LDO39_CTRL0_ldo39_eco_set_START (3)
#define PMIC_LDO39_CTRL0_ldo39_eco_set_END (3)
#define PMIC_LDO39_CTRL0_ldo39_comp_START (4)
#define PMIC_LDO39_CTRL0_ldo39_comp_END (5)
#define PMIC_LDO39_CTRL0_ldo39_bypass_ctrl_enn_START (6)
#define PMIC_LDO39_CTRL0_ldo39_bypass_ctrl_enn_END (6)
#define PMIC_LDO39_CTRL0_ldo39_bw_en_START (7)
#define PMIC_LDO39_CTRL0_ldo39_bw_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo39_vea_ctrl : 1;
        unsigned char ldo39_sst_set : 1;
        unsigned char ldo39_reserve_pull_down : 1;
        unsigned char ldo39_pd_set : 1;
        unsigned char ldo39_ocps_set : 2;
        unsigned char ldo39_ocps_en : 1;
        unsigned char ldo39_ocpd_vos_set : 1;
    } reg;
} PMIC_LDO39_CTRL1_UNION;
#endif
#define PMIC_LDO39_CTRL1_ldo39_vea_ctrl_START (0)
#define PMIC_LDO39_CTRL1_ldo39_vea_ctrl_END (0)
#define PMIC_LDO39_CTRL1_ldo39_sst_set_START (1)
#define PMIC_LDO39_CTRL1_ldo39_sst_set_END (1)
#define PMIC_LDO39_CTRL1_ldo39_reserve_pull_down_START (2)
#define PMIC_LDO39_CTRL1_ldo39_reserve_pull_down_END (2)
#define PMIC_LDO39_CTRL1_ldo39_pd_set_START (3)
#define PMIC_LDO39_CTRL1_ldo39_pd_set_END (3)
#define PMIC_LDO39_CTRL1_ldo39_ocps_set_START (4)
#define PMIC_LDO39_CTRL1_ldo39_ocps_set_END (5)
#define PMIC_LDO39_CTRL1_ldo39_ocps_en_START (6)
#define PMIC_LDO39_CTRL1_ldo39_ocps_en_END (6)
#define PMIC_LDO39_CTRL1_ldo39_ocpd_vos_set_START (7)
#define PMIC_LDO39_CTRL1_ldo39_ocpd_vos_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo39_vrset : 3;
        unsigned char ldo39_vgpr_en : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO39_CTRL2_UNION;
#endif
#define PMIC_LDO39_CTRL2_ldo39_vrset_START (0)
#define PMIC_LDO39_CTRL2_ldo39_vrset_END (2)
#define PMIC_LDO39_CTRL2_ldo39_vgpr_en_START (3)
#define PMIC_LDO39_CTRL2_ldo39_vgpr_en_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo41_ocpd_set : 2;
        unsigned char ldo41_ocp_enn : 1;
        unsigned char ldo41_eco_set : 1;
        unsigned char ldo41_comp : 2;
        unsigned char ldo41_bypass_ctrl_enn : 1;
        unsigned char ldo41_bw_en : 1;
    } reg;
} PMIC_LDO41_CTRL0_UNION;
#endif
#define PMIC_LDO41_CTRL0_ldo41_ocpd_set_START (0)
#define PMIC_LDO41_CTRL0_ldo41_ocpd_set_END (1)
#define PMIC_LDO41_CTRL0_ldo41_ocp_enn_START (2)
#define PMIC_LDO41_CTRL0_ldo41_ocp_enn_END (2)
#define PMIC_LDO41_CTRL0_ldo41_eco_set_START (3)
#define PMIC_LDO41_CTRL0_ldo41_eco_set_END (3)
#define PMIC_LDO41_CTRL0_ldo41_comp_START (4)
#define PMIC_LDO41_CTRL0_ldo41_comp_END (5)
#define PMIC_LDO41_CTRL0_ldo41_bypass_ctrl_enn_START (6)
#define PMIC_LDO41_CTRL0_ldo41_bypass_ctrl_enn_END (6)
#define PMIC_LDO41_CTRL0_ldo41_bw_en_START (7)
#define PMIC_LDO41_CTRL0_ldo41_bw_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo41_vea_ctrl : 1;
        unsigned char ldo41_sst_set : 1;
        unsigned char ldo41_reserve_pull_down : 1;
        unsigned char ldo41_pd_set : 1;
        unsigned char ldo41_ocps_set : 2;
        unsigned char ldo41_ocps_en : 1;
        unsigned char ldo41_ocpd_vos_set : 1;
    } reg;
} PMIC_LDO41_CTRL1_UNION;
#endif
#define PMIC_LDO41_CTRL1_ldo41_vea_ctrl_START (0)
#define PMIC_LDO41_CTRL1_ldo41_vea_ctrl_END (0)
#define PMIC_LDO41_CTRL1_ldo41_sst_set_START (1)
#define PMIC_LDO41_CTRL1_ldo41_sst_set_END (1)
#define PMIC_LDO41_CTRL1_ldo41_reserve_pull_down_START (2)
#define PMIC_LDO41_CTRL1_ldo41_reserve_pull_down_END (2)
#define PMIC_LDO41_CTRL1_ldo41_pd_set_START (3)
#define PMIC_LDO41_CTRL1_ldo41_pd_set_END (3)
#define PMIC_LDO41_CTRL1_ldo41_ocps_set_START (4)
#define PMIC_LDO41_CTRL1_ldo41_ocps_set_END (5)
#define PMIC_LDO41_CTRL1_ldo41_ocps_en_START (6)
#define PMIC_LDO41_CTRL1_ldo41_ocps_en_END (6)
#define PMIC_LDO41_CTRL1_ldo41_ocpd_vos_set_START (7)
#define PMIC_LDO41_CTRL1_ldo41_ocpd_vos_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo41_vrset : 3;
        unsigned char ldo41_vgpr_en : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO41_CTRL2_UNION;
#endif
#define PMIC_LDO41_CTRL2_ldo41_vrset_START (0)
#define PMIC_LDO41_CTRL2_ldo41_vrset_END (2)
#define PMIC_LDO41_CTRL2_ldo41_vgpr_en_START (3)
#define PMIC_LDO41_CTRL2_ldo41_vgpr_en_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo42_ocpd_set : 2;
        unsigned char ldo42_ocp_enn : 1;
        unsigned char ldo42_eco_set : 1;
        unsigned char ldo42_comp : 2;
        unsigned char ldo42_bypass_ctrl_enn : 1;
        unsigned char ldo42_bw_en : 1;
    } reg;
} PMIC_LDO42_CTRL0_UNION;
#endif
#define PMIC_LDO42_CTRL0_ldo42_ocpd_set_START (0)
#define PMIC_LDO42_CTRL0_ldo42_ocpd_set_END (1)
#define PMIC_LDO42_CTRL0_ldo42_ocp_enn_START (2)
#define PMIC_LDO42_CTRL0_ldo42_ocp_enn_END (2)
#define PMIC_LDO42_CTRL0_ldo42_eco_set_START (3)
#define PMIC_LDO42_CTRL0_ldo42_eco_set_END (3)
#define PMIC_LDO42_CTRL0_ldo42_comp_START (4)
#define PMIC_LDO42_CTRL0_ldo42_comp_END (5)
#define PMIC_LDO42_CTRL0_ldo42_bypass_ctrl_enn_START (6)
#define PMIC_LDO42_CTRL0_ldo42_bypass_ctrl_enn_END (6)
#define PMIC_LDO42_CTRL0_ldo42_bw_en_START (7)
#define PMIC_LDO42_CTRL0_ldo42_bw_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo42_sst_set : 1;
        unsigned char ldo42_reserve_pull_down : 1;
        unsigned char ldo42_pd_set : 1;
        unsigned char ldo42_ocps_set : 2;
        unsigned char ldo42_ocps_en : 1;
        unsigned char ldo42_ocpd_vos_set : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO42_CTRL1_UNION;
#endif
#define PMIC_LDO42_CTRL1_ldo42_sst_set_START (0)
#define PMIC_LDO42_CTRL1_ldo42_sst_set_END (0)
#define PMIC_LDO42_CTRL1_ldo42_reserve_pull_down_START (1)
#define PMIC_LDO42_CTRL1_ldo42_reserve_pull_down_END (1)
#define PMIC_LDO42_CTRL1_ldo42_pd_set_START (2)
#define PMIC_LDO42_CTRL1_ldo42_pd_set_END (2)
#define PMIC_LDO42_CTRL1_ldo42_ocps_set_START (3)
#define PMIC_LDO42_CTRL1_ldo42_ocps_set_END (4)
#define PMIC_LDO42_CTRL1_ldo42_ocps_en_START (5)
#define PMIC_LDO42_CTRL1_ldo42_ocps_en_END (5)
#define PMIC_LDO42_CTRL1_ldo42_ocpd_vos_set_START (6)
#define PMIC_LDO42_CTRL1_ldo42_ocpd_vos_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo42_vrset : 3;
        unsigned char ldo42_vgpr_en : 1;
        unsigned char ldo42_vea_ctrl : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO42_CTRL2_UNION;
#endif
#define PMIC_LDO42_CTRL2_ldo42_vrset_START (0)
#define PMIC_LDO42_CTRL2_ldo42_vrset_END (2)
#define PMIC_LDO42_CTRL2_ldo42_vgpr_en_START (3)
#define PMIC_LDO42_CTRL2_ldo42_vgpr_en_END (3)
#define PMIC_LDO42_CTRL2_ldo42_vea_ctrl_START (4)
#define PMIC_LDO42_CTRL2_ldo42_vea_ctrl_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo43_ocps_en : 1;
        unsigned char ldo43_ocpd_set : 2;
        unsigned char ldo43_ocp_enn : 1;
        unsigned char ldo43_eco_set : 1;
        unsigned char ldo43_bw_en : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO43_CTRL0_UNION;
#endif
#define PMIC_LDO43_CTRL0_ldo43_ocps_en_START (0)
#define PMIC_LDO43_CTRL0_ldo43_ocps_en_END (0)
#define PMIC_LDO43_CTRL0_ldo43_ocpd_set_START (1)
#define PMIC_LDO43_CTRL0_ldo43_ocpd_set_END (2)
#define PMIC_LDO43_CTRL0_ldo43_ocp_enn_START (3)
#define PMIC_LDO43_CTRL0_ldo43_ocp_enn_END (3)
#define PMIC_LDO43_CTRL0_ldo43_eco_set_START (4)
#define PMIC_LDO43_CTRL0_ldo43_eco_set_END (4)
#define PMIC_LDO43_CTRL0_ldo43_bw_en_START (5)
#define PMIC_LDO43_CTRL0_ldo43_bw_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo43_vrset : 3;
        unsigned char ldo43_ocps_set : 2;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO43_CTRL1_UNION;
#endif
#define PMIC_LDO43_CTRL1_ldo43_vrset_START (0)
#define PMIC_LDO43_CTRL1_ldo43_vrset_END (2)
#define PMIC_LDO43_CTRL1_ldo43_ocps_set_START (3)
#define PMIC_LDO43_CTRL1_ldo43_ocps_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo44_ocpd_set : 2;
        unsigned char ldo44_ocp_enn : 1;
        unsigned char ldo44_eco_set : 1;
        unsigned char ldo44_comp : 2;
        unsigned char ldo44_bypass_ctrl_enn : 1;
        unsigned char ldo44_bw_en : 1;
    } reg;
} PMIC_LDO44_CTRL0_UNION;
#endif
#define PMIC_LDO44_CTRL0_ldo44_ocpd_set_START (0)
#define PMIC_LDO44_CTRL0_ldo44_ocpd_set_END (1)
#define PMIC_LDO44_CTRL0_ldo44_ocp_enn_START (2)
#define PMIC_LDO44_CTRL0_ldo44_ocp_enn_END (2)
#define PMIC_LDO44_CTRL0_ldo44_eco_set_START (3)
#define PMIC_LDO44_CTRL0_ldo44_eco_set_END (3)
#define PMIC_LDO44_CTRL0_ldo44_comp_START (4)
#define PMIC_LDO44_CTRL0_ldo44_comp_END (5)
#define PMIC_LDO44_CTRL0_ldo44_bypass_ctrl_enn_START (6)
#define PMIC_LDO44_CTRL0_ldo44_bypass_ctrl_enn_END (6)
#define PMIC_LDO44_CTRL0_ldo44_bw_en_START (7)
#define PMIC_LDO44_CTRL0_ldo44_bw_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo44_vgpr_en : 1;
        unsigned char ldo44_vea_ctrl : 1;
        unsigned char ldo44_sst_set : 1;
        unsigned char ldo44_pd_set : 1;
        unsigned char ldo44_ocps_set : 2;
        unsigned char ldo44_ocps_en : 1;
        unsigned char ldo44_ocpd_vos_set : 1;
    } reg;
} PMIC_LDO44_CTRL1_UNION;
#endif
#define PMIC_LDO44_CTRL1_ldo44_vgpr_en_START (0)
#define PMIC_LDO44_CTRL1_ldo44_vgpr_en_END (0)
#define PMIC_LDO44_CTRL1_ldo44_vea_ctrl_START (1)
#define PMIC_LDO44_CTRL1_ldo44_vea_ctrl_END (1)
#define PMIC_LDO44_CTRL1_ldo44_sst_set_START (2)
#define PMIC_LDO44_CTRL1_ldo44_sst_set_END (2)
#define PMIC_LDO44_CTRL1_ldo44_pd_set_START (3)
#define PMIC_LDO44_CTRL1_ldo44_pd_set_END (3)
#define PMIC_LDO44_CTRL1_ldo44_ocps_set_START (4)
#define PMIC_LDO44_CTRL1_ldo44_ocps_set_END (5)
#define PMIC_LDO44_CTRL1_ldo44_ocps_en_START (6)
#define PMIC_LDO44_CTRL1_ldo44_ocps_en_END (6)
#define PMIC_LDO44_CTRL1_ldo44_ocpd_vos_set_START (7)
#define PMIC_LDO44_CTRL1_ldo44_ocpd_vos_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo44_vrset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO44_CTRL2_UNION;
#endif
#define PMIC_LDO44_CTRL2_ldo44_vrset_START (0)
#define PMIC_LDO44_CTRL2_ldo44_vrset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo45_ocp_enn : 1;
        unsigned char ldo45_comp : 3;
        unsigned char ldo45_bw_en : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO45_CTRL0_UNION;
#endif
#define PMIC_LDO45_CTRL0_ldo45_ocp_enn_START (0)
#define PMIC_LDO45_CTRL0_ldo45_ocp_enn_END (0)
#define PMIC_LDO45_CTRL0_ldo45_comp_START (1)
#define PMIC_LDO45_CTRL0_ldo45_comp_END (3)
#define PMIC_LDO45_CTRL0_ldo45_bw_en_START (4)
#define PMIC_LDO45_CTRL0_ldo45_bw_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo45_ocps_en : 1;
        unsigned char ldo45_ocpd_set : 4;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO45_CTRL1_UNION;
#endif
#define PMIC_LDO45_CTRL1_ldo45_ocps_en_START (0)
#define PMIC_LDO45_CTRL1_ldo45_ocps_en_END (0)
#define PMIC_LDO45_CTRL1_ldo45_ocpd_set_START (1)
#define PMIC_LDO45_CTRL1_ldo45_ocpd_set_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo45_psrr1_set : 3;
        unsigned char ldo45_psrr_en : 1;
        unsigned char ldo45_ocps_set : 4;
    } reg;
} PMIC_LDO45_CTRL2_UNION;
#endif
#define PMIC_LDO45_CTRL2_ldo45_psrr1_set_START (0)
#define PMIC_LDO45_CTRL2_ldo45_psrr1_set_END (2)
#define PMIC_LDO45_CTRL2_ldo45_psrr_en_START (3)
#define PMIC_LDO45_CTRL2_ldo45_psrr_en_END (3)
#define PMIC_LDO45_CTRL2_ldo45_ocps_set_START (4)
#define PMIC_LDO45_CTRL2_ldo45_ocps_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo45_vgpr_en : 1;
        unsigned char ldo45_sst_set : 2;
        unsigned char ldo45_psrr2_set : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO45_CTRL3_UNION;
#endif
#define PMIC_LDO45_CTRL3_ldo45_vgpr_en_START (0)
#define PMIC_LDO45_CTRL3_ldo45_vgpr_en_END (0)
#define PMIC_LDO45_CTRL3_ldo45_sst_set_START (1)
#define PMIC_LDO45_CTRL3_ldo45_sst_set_END (2)
#define PMIC_LDO45_CTRL3_ldo45_psrr2_set_START (3)
#define PMIC_LDO45_CTRL3_ldo45_psrr2_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo45_vrset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO45_CTRL4_UNION;
#endif
#define PMIC_LDO45_CTRL4_ldo45_vrset_START (0)
#define PMIC_LDO45_CTRL4_ldo45_vrset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo46_ocpd_vos_set : 1;
        unsigned char ldo46_ocpd_set : 2;
        unsigned char ldo46_ocp_enn : 1;
        unsigned char ldo46_comp : 3;
        unsigned char ldo46_bw_en : 1;
    } reg;
} PMIC_LDO46_CTRL0_UNION;
#endif
#define PMIC_LDO46_CTRL0_ldo46_ocpd_vos_set_START (0)
#define PMIC_LDO46_CTRL0_ldo46_ocpd_vos_set_END (0)
#define PMIC_LDO46_CTRL0_ldo46_ocpd_set_START (1)
#define PMIC_LDO46_CTRL0_ldo46_ocpd_set_END (2)
#define PMIC_LDO46_CTRL0_ldo46_ocp_enn_START (3)
#define PMIC_LDO46_CTRL0_ldo46_ocp_enn_END (3)
#define PMIC_LDO46_CTRL0_ldo46_comp_START (4)
#define PMIC_LDO46_CTRL0_ldo46_comp_END (6)
#define PMIC_LDO46_CTRL0_ldo46_bw_en_START (7)
#define PMIC_LDO46_CTRL0_ldo46_bw_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo46_psrr1_set : 3;
        unsigned char ldo46_psrr_en : 1;
        unsigned char ldo46_ocps_set : 2;
        unsigned char ldo46_ocps_en : 1;
        unsigned char ldo46_ocps_bw_set : 1;
    } reg;
} PMIC_LDO46_CTRL1_UNION;
#endif
#define PMIC_LDO46_CTRL1_ldo46_psrr1_set_START (0)
#define PMIC_LDO46_CTRL1_ldo46_psrr1_set_END (2)
#define PMIC_LDO46_CTRL1_ldo46_psrr_en_START (3)
#define PMIC_LDO46_CTRL1_ldo46_psrr_en_END (3)
#define PMIC_LDO46_CTRL1_ldo46_ocps_set_START (4)
#define PMIC_LDO46_CTRL1_ldo46_ocps_set_END (5)
#define PMIC_LDO46_CTRL1_ldo46_ocps_en_START (6)
#define PMIC_LDO46_CTRL1_ldo46_ocps_en_END (6)
#define PMIC_LDO46_CTRL1_ldo46_ocps_bw_set_START (7)
#define PMIC_LDO46_CTRL1_ldo46_ocps_bw_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo46_vgpr_en : 1;
        unsigned char ldo46_sst_set : 2;
        unsigned char ldo46_psrr2_set : 3;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO46_CTRL2_UNION;
#endif
#define PMIC_LDO46_CTRL2_ldo46_vgpr_en_START (0)
#define PMIC_LDO46_CTRL2_ldo46_vgpr_en_END (0)
#define PMIC_LDO46_CTRL2_ldo46_sst_set_START (1)
#define PMIC_LDO46_CTRL2_ldo46_sst_set_END (2)
#define PMIC_LDO46_CTRL2_ldo46_psrr2_set_START (3)
#define PMIC_LDO46_CTRL2_ldo46_psrr2_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo46_vrset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO46_CTRL3_UNION;
#endif
#define PMIC_LDO46_CTRL3_ldo46_vrset_START (0)
#define PMIC_LDO46_CTRL3_ldo46_vrset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo47_ocpd_set : 2;
        unsigned char ldo47_ocp_enn : 1;
        unsigned char ldo47_eco_set : 1;
        unsigned char ldo47_comp : 2;
        unsigned char ldo47_bypass_ctrl_enn : 1;
        unsigned char ldo47_bw_en : 1;
    } reg;
} PMIC_LDO47_CTRL0_UNION;
#endif
#define PMIC_LDO47_CTRL0_ldo47_ocpd_set_START (0)
#define PMIC_LDO47_CTRL0_ldo47_ocpd_set_END (1)
#define PMIC_LDO47_CTRL0_ldo47_ocp_enn_START (2)
#define PMIC_LDO47_CTRL0_ldo47_ocp_enn_END (2)
#define PMIC_LDO47_CTRL0_ldo47_eco_set_START (3)
#define PMIC_LDO47_CTRL0_ldo47_eco_set_END (3)
#define PMIC_LDO47_CTRL0_ldo47_comp_START (4)
#define PMIC_LDO47_CTRL0_ldo47_comp_END (5)
#define PMIC_LDO47_CTRL0_ldo47_bypass_ctrl_enn_START (6)
#define PMIC_LDO47_CTRL0_ldo47_bypass_ctrl_enn_END (6)
#define PMIC_LDO47_CTRL0_ldo47_bw_en_START (7)
#define PMIC_LDO47_CTRL0_ldo47_bw_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo47_vea_ctrl : 1;
        unsigned char ldo47_sst_set : 1;
        unsigned char ldo47_pd_set : 1;
        unsigned char ldo47_ocps_set : 2;
        unsigned char ldo47_ocps_en : 1;
        unsigned char ldo47_ocpd_vos_set : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO47_CTRL1_UNION;
#endif
#define PMIC_LDO47_CTRL1_ldo47_vea_ctrl_START (0)
#define PMIC_LDO47_CTRL1_ldo47_vea_ctrl_END (0)
#define PMIC_LDO47_CTRL1_ldo47_sst_set_START (1)
#define PMIC_LDO47_CTRL1_ldo47_sst_set_END (1)
#define PMIC_LDO47_CTRL1_ldo47_pd_set_START (2)
#define PMIC_LDO47_CTRL1_ldo47_pd_set_END (2)
#define PMIC_LDO47_CTRL1_ldo47_ocps_set_START (3)
#define PMIC_LDO47_CTRL1_ldo47_ocps_set_END (4)
#define PMIC_LDO47_CTRL1_ldo47_ocps_en_START (5)
#define PMIC_LDO47_CTRL1_ldo47_ocps_en_END (5)
#define PMIC_LDO47_CTRL1_ldo47_ocpd_vos_set_START (6)
#define PMIC_LDO47_CTRL1_ldo47_ocpd_vos_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo47_vrset : 3;
        unsigned char ldo47_vgpr_en : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO47_CTRL2_UNION;
#endif
#define PMIC_LDO47_CTRL2_ldo47_vrset_START (0)
#define PMIC_LDO47_CTRL2_ldo47_vrset_END (2)
#define PMIC_LDO47_CTRL2_ldo47_vgpr_en_START (3)
#define PMIC_LDO47_CTRL2_ldo47_vgpr_en_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo48_vrset : 3;
        unsigned char ldo48_ocp_set : 1;
        unsigned char ldo48_ocp_enn : 1;
        unsigned char ldo48_eco_set : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO48_CTRL0_UNION;
#endif
#define PMIC_LDO48_CTRL0_ldo48_vrset_START (0)
#define PMIC_LDO48_CTRL0_ldo48_vrset_END (2)
#define PMIC_LDO48_CTRL0_ldo48_ocp_set_START (3)
#define PMIC_LDO48_CTRL0_ldo48_ocp_set_END (3)
#define PMIC_LDO48_CTRL0_ldo48_ocp_enn_START (4)
#define PMIC_LDO48_CTRL0_ldo48_ocp_enn_END (4)
#define PMIC_LDO48_CTRL0_ldo48_eco_set_START (5)
#define PMIC_LDO48_CTRL0_ldo48_eco_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo49_ocpd_set : 2;
        unsigned char ldo49_ocp_enn : 1;
        unsigned char ldo49_eco_set : 1;
        unsigned char ldo49_comp : 2;
        unsigned char ldo49_bypass_ctrl_enn : 1;
        unsigned char ldo49_bw_en : 1;
    } reg;
} PMIC_LDO49_CTRL0_UNION;
#endif
#define PMIC_LDO49_CTRL0_ldo49_ocpd_set_START (0)
#define PMIC_LDO49_CTRL0_ldo49_ocpd_set_END (1)
#define PMIC_LDO49_CTRL0_ldo49_ocp_enn_START (2)
#define PMIC_LDO49_CTRL0_ldo49_ocp_enn_END (2)
#define PMIC_LDO49_CTRL0_ldo49_eco_set_START (3)
#define PMIC_LDO49_CTRL0_ldo49_eco_set_END (3)
#define PMIC_LDO49_CTRL0_ldo49_comp_START (4)
#define PMIC_LDO49_CTRL0_ldo49_comp_END (5)
#define PMIC_LDO49_CTRL0_ldo49_bypass_ctrl_enn_START (6)
#define PMIC_LDO49_CTRL0_ldo49_bypass_ctrl_enn_END (6)
#define PMIC_LDO49_CTRL0_ldo49_bw_en_START (7)
#define PMIC_LDO49_CTRL0_ldo49_bw_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo49_vea_ctrl : 1;
        unsigned char ldo49_sst_set : 1;
        unsigned char ldo49_reserve_pull_down : 1;
        unsigned char ldo49_pd_set : 1;
        unsigned char ldo49_ocps_set : 2;
        unsigned char ldo49_ocps_en : 1;
        unsigned char ldo49_ocpd_vos_set : 1;
    } reg;
} PMIC_LDO49_CTRL1_UNION;
#endif
#define PMIC_LDO49_CTRL1_ldo49_vea_ctrl_START (0)
#define PMIC_LDO49_CTRL1_ldo49_vea_ctrl_END (0)
#define PMIC_LDO49_CTRL1_ldo49_sst_set_START (1)
#define PMIC_LDO49_CTRL1_ldo49_sst_set_END (1)
#define PMIC_LDO49_CTRL1_ldo49_reserve_pull_down_START (2)
#define PMIC_LDO49_CTRL1_ldo49_reserve_pull_down_END (2)
#define PMIC_LDO49_CTRL1_ldo49_pd_set_START (3)
#define PMIC_LDO49_CTRL1_ldo49_pd_set_END (3)
#define PMIC_LDO49_CTRL1_ldo49_ocps_set_START (4)
#define PMIC_LDO49_CTRL1_ldo49_ocps_set_END (5)
#define PMIC_LDO49_CTRL1_ldo49_ocps_en_START (6)
#define PMIC_LDO49_CTRL1_ldo49_ocps_en_END (6)
#define PMIC_LDO49_CTRL1_ldo49_ocpd_vos_set_START (7)
#define PMIC_LDO49_CTRL1_ldo49_ocpd_vos_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo49_vrset : 3;
        unsigned char ldo49_vgpr_en : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO49_CTRL2_UNION;
#endif
#define PMIC_LDO49_CTRL2_ldo49_vrset_START (0)
#define PMIC_LDO49_CTRL2_ldo49_vrset_END (2)
#define PMIC_LDO49_CTRL2_ldo49_vgpr_en_START (3)
#define PMIC_LDO49_CTRL2_ldo49_vgpr_en_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo50_ocps_set : 2;
        unsigned char ldo50_ocps_en : 1;
        unsigned char ldo50_ocpd_set : 2;
        unsigned char ldo50_ocp_enn : 1;
        unsigned char ldo50_eco_set : 1;
        unsigned char ldo50_bw_en : 1;
    } reg;
} PMIC_LDO50_CTRL0_UNION;
#endif
#define PMIC_LDO50_CTRL0_ldo50_ocps_set_START (0)
#define PMIC_LDO50_CTRL0_ldo50_ocps_set_END (1)
#define PMIC_LDO50_CTRL0_ldo50_ocps_en_START (2)
#define PMIC_LDO50_CTRL0_ldo50_ocps_en_END (2)
#define PMIC_LDO50_CTRL0_ldo50_ocpd_set_START (3)
#define PMIC_LDO50_CTRL0_ldo50_ocpd_set_END (4)
#define PMIC_LDO50_CTRL0_ldo50_ocp_enn_START (5)
#define PMIC_LDO50_CTRL0_ldo50_ocp_enn_END (5)
#define PMIC_LDO50_CTRL0_ldo50_eco_set_START (6)
#define PMIC_LDO50_CTRL0_ldo50_eco_set_END (6)
#define PMIC_LDO50_CTRL0_ldo50_bw_en_START (7)
#define PMIC_LDO50_CTRL0_ldo50_bw_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo50_vrset : 3;
        unsigned char ldo50_vgpr_en : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO50_CTRL1_UNION;
#endif
#define PMIC_LDO50_CTRL1_ldo50_vrset_START (0)
#define PMIC_LDO50_CTRL1_ldo50_vrset_END (2)
#define PMIC_LDO50_CTRL1_ldo50_vgpr_en_START (3)
#define PMIC_LDO50_CTRL1_ldo50_vgpr_en_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo_buff_curr_sel : 2;
        unsigned char ldo_buf_reserve_eco_en : 1;
        unsigned char ldo_buf_ocp_enn : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO_BUF_CTRL0_UNION;
#endif
#define PMIC_LDO_BUF_CTRL0_ldo_buff_curr_sel_START (0)
#define PMIC_LDO_BUF_CTRL0_ldo_buff_curr_sel_END (1)
#define PMIC_LDO_BUF_CTRL0_ldo_buf_reserve_eco_en_START (2)
#define PMIC_LDO_BUF_CTRL0_ldo_buf_reserve_eco_en_END (2)
#define PMIC_LDO_BUF_CTRL0_ldo_buf_ocp_enn_START (3)
#define PMIC_LDO_BUF_CTRL0_ldo_buf_ocp_enn_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pmuh_stb_set : 4;
        unsigned char pmuh_ocp_enn : 1;
        unsigned char pmuh_eco_set : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO_PMUH_CTRL0_UNION;
#endif
#define PMIC_LDO_PMUH_CTRL0_pmuh_stb_set_START (0)
#define PMIC_LDO_PMUH_CTRL0_pmuh_stb_set_END (3)
#define PMIC_LDO_PMUH_CTRL0_pmuh_ocp_enn_START (4)
#define PMIC_LDO_PMUH_CTRL0_pmuh_ocp_enn_END (4)
#define PMIC_LDO_PMUH_CTRL0_pmuh_eco_set_START (5)
#define PMIC_LDO_PMUH_CTRL0_pmuh_eco_set_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pmuh_vrset : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_LDO_PMUH_CTRL1_UNION;
#endif
#define PMIC_LDO_PMUH_CTRL1_pmuh_vrset_START (0)
#define PMIC_LDO_PMUH_CTRL1_pmuh_vrset_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo_reserve0 : 8;
    } reg;
} PMIC_LDO_RESERVE_CTRL0_UNION;
#endif
#define PMIC_LDO_RESERVE_CTRL0_ldo_reserve0_START (0)
#define PMIC_LDO_RESERVE_CTRL0_ldo_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo_reserve1 : 8;
    } reg;
} PMIC_LDO_RESERVE_CTRL1_UNION;
#endif
#define PMIC_LDO_RESERVE_CTRL1_ldo_reserve1_START (0)
#define PMIC_LDO_RESERVE_CTRL1_ldo_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo_reserve2 : 8;
    } reg;
} PMIC_LDO_RESERVE_CTRL2_UNION;
#endif
#define PMIC_LDO_RESERVE_CTRL2_ldo_reserve2_START (0)
#define PMIC_LDO_RESERVE_CTRL2_ldo_reserve2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo_reserve3 : 8;
    } reg;
} PMIC_LDO_RESERVE_CTRL3_UNION;
#endif
#define PMIC_LDO_RESERVE_CTRL3_ldo_reserve3_START (0)
#define PMIC_LDO_RESERVE_CTRL3_ldo_reserve3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo_reserve4 : 8;
    } reg;
} PMIC_LDO_RESERVE_CTRL4_UNION;
#endif
#define PMIC_LDO_RESERVE_CTRL4_ldo_reserve4_START (0)
#define PMIC_LDO_RESERVE_CTRL4_ldo_reserve4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo_reserve5 : 8;
    } reg;
} PMIC_LDO_RESERVE_CTRL5_UNION;
#endif
#define PMIC_LDO_RESERVE_CTRL5_ldo_reserve5_START (0)
#define PMIC_LDO_RESERVE_CTRL5_ldo_reserve5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo_reserve6 : 8;
    } reg;
} PMIC_LDO_RESERVE_CTRL6_UNION;
#endif
#define PMIC_LDO_RESERVE_CTRL6_ldo_reserve6_START (0)
#define PMIC_LDO_RESERVE_CTRL6_ldo_reserve6_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_nfc_drv : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_CLK_NFC_DRV_UNION;
#endif
#define PMIC_CLK_NFC_DRV_xo_nfc_drv_START (0)
#define PMIC_CLK_NFC_DRV_xo_nfc_drv_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_codec_drv : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_CLK_CODEC_DRV_UNION;
#endif
#define PMIC_CLK_CODEC_DRV_xo_codec_drv_START (0)
#define PMIC_CLK_CODEC_DRV_xo_codec_drv_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_ufs_drv : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_CLK_UFS_DRV_UNION;
#endif
#define PMIC_CLK_UFS_DRV_xo_ufs_drv_START (0)
#define PMIC_CLK_UFS_DRV_xo_ufs_drv_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_serdes_drv : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_CLK_SERDES_DRV_UNION;
#endif
#define PMIC_CLK_SERDES_DRV_xo_serdes_drv_START (0)
#define PMIC_CLK_SERDES_DRV_xo_serdes_drv_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_wifi_drv : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_CLK_WIFI_DRV_UNION;
#endif
#define PMIC_CLK_WIFI_DRV_xo_wifi_drv_START (0)
#define PMIC_CLK_WIFI_DRV_xo_wifi_drv_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_sys_drv : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_CLK_SYS_DRV_UNION;
#endif
#define PMIC_CLK_SYS_DRV_xo_sys_drv_START (0)
#define PMIC_CLK_SYS_DRV_xo_sys_drv_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_rf_drv : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_CLK_RF0_DRV_UNION;
#endif
#define PMIC_CLK_RF0_DRV_xo_rf_drv_START (0)
#define PMIC_CLK_RF0_DRV_xo_rf_drv_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_codec_delay_sel : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_CLK_TOP_CTRL0_UNION;
#endif
#define PMIC_CLK_TOP_CTRL0_xo_codec_delay_sel_START (0)
#define PMIC_CLK_TOP_CTRL0_xo_codec_delay_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_eco_en_shield : 1;
        unsigned char xo_core_curr_sel : 2;
        unsigned char xo_codec_freq_sel : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_CLK_TOP_CTRL1_UNION;
#endif
#define PMIC_CLK_TOP_CTRL1_xo_eco_en_shield_START (0)
#define PMIC_CLK_TOP_CTRL1_xo_eco_en_shield_END (0)
#define PMIC_CLK_TOP_CTRL1_xo_core_curr_sel_START (1)
#define PMIC_CLK_TOP_CTRL1_xo_core_curr_sel_END (2)
#define PMIC_CLK_TOP_CTRL1_xo_codec_freq_sel_START (3)
#define PMIC_CLK_TOP_CTRL1_xo_codec_freq_sel_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_nfc_freq_sel : 1;
        unsigned char xo_nfc_delay_sel : 2;
        unsigned char xo_eco_force_en : 1;
        unsigned char reserved : 4;
    } reg;
} PMIC_CLK_TOP_CTRL2_UNION;
#endif
#define PMIC_CLK_TOP_CTRL2_xo_nfc_freq_sel_START (0)
#define PMIC_CLK_TOP_CTRL2_xo_nfc_freq_sel_END (0)
#define PMIC_CLK_TOP_CTRL2_xo_nfc_delay_sel_START (1)
#define PMIC_CLK_TOP_CTRL2_xo_nfc_delay_sel_END (2)
#define PMIC_CLK_TOP_CTRL2_xo_eco_force_en_START (3)
#define PMIC_CLK_TOP_CTRL2_xo_eco_force_en_END (3)
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
        unsigned char xo_rf_freq_sel : 1;
        unsigned char xo_rf_delay_sel : 2;
        unsigned char reserved : 5;
    } reg;
} PMIC_CLK_TOP_CTRL4_UNION;
#endif
#define PMIC_CLK_TOP_CTRL4_xo_rf_freq_sel_START (0)
#define PMIC_CLK_TOP_CTRL4_xo_rf_freq_sel_END (0)
#define PMIC_CLK_TOP_CTRL4_xo_rf_delay_sel_START (1)
#define PMIC_CLK_TOP_CTRL4_xo_rf_delay_sel_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_tri_cap : 1;
        unsigned char xo_ton_eco_shield : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_CLK_TOP_CTRL5_UNION;
#endif
#define PMIC_CLK_TOP_CTRL5_xo_tri_cap_START (0)
#define PMIC_CLK_TOP_CTRL5_xo_tri_cap_END (0)
#define PMIC_CLK_TOP_CTRL5_xo_ton_eco_shield_START (1)
#define PMIC_CLK_TOP_CTRL5_xo_ton_eco_shield_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_wifi_freq_sel : 1;
        unsigned char xo_wifi_delay_sel : 2;
        unsigned char reserved : 5;
    } reg;
} PMIC_CLK_TOP_CTRL6_UNION;
#endif
#define PMIC_CLK_TOP_CTRL6_xo_wifi_freq_sel_START (0)
#define PMIC_CLK_TOP_CTRL6_xo_wifi_freq_sel_END (0)
#define PMIC_CLK_TOP_CTRL6_xo_wifi_delay_sel_START (1)
#define PMIC_CLK_TOP_CTRL6_xo_wifi_delay_sel_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ref_ibias_trim_en : 2;
        unsigned char ref_ibias_sel : 1;
        unsigned char ref_eco_shield : 1;
        unsigned char ref_chop_sel : 1;
        unsigned char ref_chop_clk_sel : 2;
        unsigned char reserved : 1;
    } reg;
} PMIC_REF_CTRL0_UNION;
#endif
#define PMIC_REF_CTRL0_ref_ibias_trim_en_START (0)
#define PMIC_REF_CTRL0_ref_ibias_trim_en_END (1)
#define PMIC_REF_CTRL0_ref_ibias_sel_START (2)
#define PMIC_REF_CTRL0_ref_ibias_sel_END (2)
#define PMIC_REF_CTRL0_ref_eco_shield_START (3)
#define PMIC_REF_CTRL0_ref_eco_shield_END (3)
#define PMIC_REF_CTRL0_ref_chop_sel_START (4)
#define PMIC_REF_CTRL0_ref_chop_sel_END (4)
#define PMIC_REF_CTRL0_ref_chop_clk_sel_START (5)
#define PMIC_REF_CTRL0_ref_chop_clk_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ref_out_sel : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_REF_CTRL1_UNION;
#endif
#define PMIC_REF_CTRL1_ref_out_sel_START (0)
#define PMIC_REF_CTRL1_ref_out_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ref_reserve : 8;
    } reg;
} PMIC_REF_RESERVE0_UNION;
#endif
#define PMIC_REF_RESERVE0_ref_reserve_START (0)
#define PMIC_REF_RESERVE0_ref_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ref_reserve2 : 8;
    } reg;
} PMIC_REF_RESERVE1_UNION;
#endif
#define PMIC_REF_RESERVE1_ref_reserve2_START (0)
#define PMIC_REF_RESERVE1_ref_reserve2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char thsd_tmp_set : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_THSD_TMP_SET_UNION;
#endif
#define PMIC_THSD_TMP_SET_thsd_tmp_set_START (0)
#define PMIC_THSD_TMP_SET_thsd_tmp_set_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char nfc_pd_res : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_SYS_CTRL_CTRL0_UNION;
#endif
#define PMIC_SYS_CTRL_CTRL0_nfc_pd_res_START (0)
#define PMIC_SYS_CTRL_CTRL0_nfc_pd_res_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sys_ctrl_reserve : 8;
    } reg;
} PMIC_SYS_CTRL_CTRL1_UNION;
#endif
#define PMIC_SYS_CTRL_CTRL1_sys_ctrl_reserve_START (0)
#define PMIC_SYS_CTRL_CTRL1_sys_ctrl_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char wifi_pd_res : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_SYS_CTRL_CTRL2_UNION;
#endif
#define PMIC_SYS_CTRL_CTRL2_wifi_pd_res_START (0)
#define PMIC_SYS_CTRL_CTRL2_wifi_pd_res_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char classd_i_pump : 2;
        unsigned char classd_i_ocp : 2;
        unsigned char classd_gain : 2;
        unsigned char classd_dt_sel : 1;
        unsigned char classd_drv_en : 1;
    } reg;
} PMIC_CLASSD_CTRL0_UNION;
#endif
#define PMIC_CLASSD_CTRL0_classd_i_pump_START (0)
#define PMIC_CLASSD_CTRL0_classd_i_pump_END (1)
#define PMIC_CLASSD_CTRL0_classd_i_ocp_START (2)
#define PMIC_CLASSD_CTRL0_classd_i_ocp_END (3)
#define PMIC_CLASSD_CTRL0_classd_gain_START (4)
#define PMIC_CLASSD_CTRL0_classd_gain_END (5)
#define PMIC_CLASSD_CTRL0_classd_dt_sel_START (6)
#define PMIC_CLASSD_CTRL0_classd_dt_sel_END (6)
#define PMIC_CLASSD_CTRL0_classd_drv_en_START (7)
#define PMIC_CLASSD_CTRL0_classd_drv_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char classd_pls_byp : 1;
        unsigned char classd_p_sel : 2;
        unsigned char classd_ocp_bps : 1;
        unsigned char classd_n_sel : 2;
        unsigned char classd_mute_sel : 1;
        unsigned char classd_mute : 1;
    } reg;
} PMIC_CLASSD_CTRL1_UNION;
#endif
#define PMIC_CLASSD_CTRL1_classd_pls_byp_START (0)
#define PMIC_CLASSD_CTRL1_classd_pls_byp_END (0)
#define PMIC_CLASSD_CTRL1_classd_p_sel_START (1)
#define PMIC_CLASSD_CTRL1_classd_p_sel_END (2)
#define PMIC_CLASSD_CTRL1_classd_ocp_bps_START (3)
#define PMIC_CLASSD_CTRL1_classd_ocp_bps_END (3)
#define PMIC_CLASSD_CTRL1_classd_n_sel_START (4)
#define PMIC_CLASSD_CTRL1_classd_n_sel_END (5)
#define PMIC_CLASSD_CTRL1_classd_mute_sel_START (6)
#define PMIC_CLASSD_CTRL1_classd_mute_sel_END (6)
#define PMIC_CLASSD_CTRL1_classd_mute_START (7)
#define PMIC_CLASSD_CTRL1_classd_mute_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char classd_reserve0 : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_CLASSD_CTRL2_UNION;
#endif
#define PMIC_CLASSD_CTRL2_classd_reserve0_START (0)
#define PMIC_CLASSD_CTRL2_classd_reserve0_END (3)
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
        unsigned char d2a_reserve0 : 8;
    } reg;
} PMIC_D2A_RESERVE0_UNION;
#endif
#define PMIC_D2A_RESERVE0_d2a_reserve0_START (0)
#define PMIC_D2A_RESERVE0_d2a_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char d2a_reserve1 : 8;
    } reg;
} PMIC_D2A_RESERVE1_UNION;
#endif
#define PMIC_D2A_RESERVE1_d2a_reserve1_START (0)
#define PMIC_D2A_RESERVE1_d2a_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char a2d_reserve0 : 8;
    } reg;
} PMIC_A2D_RESERVE0_UNION;
#endif
#define PMIC_A2D_RESERVE0_a2d_reserve0_START (0)
#define PMIC_A2D_RESERVE0_a2d_reserve0_END (7)
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
        unsigned char ser_sidekey_mode_sel : 1;
        unsigned char ser_sidekey_time : 2;
        unsigned char reserved : 5;
    } reg;
} PMIC_SER_SIDEKEY_MODE_UNION;
#endif
#define PMIC_SER_SIDEKEY_MODE_ser_sidekey_mode_sel_START (0)
#define PMIC_SER_SIDEKEY_MODE_ser_sidekey_mode_sel_END (0)
#define PMIC_SER_SIDEKEY_MODE_ser_sidekey_time_START (1)
#define PMIC_SER_SIDEKEY_MODE_ser_sidekey_time_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_ldo33_en_a : 1;
        unsigned char st_ldo33_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char reg_ldo33_eco_en : 1;
        unsigned char st_ldo33_eco_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} PMIC_LDO33_ONOFF1_ECO_UNION;
#endif
#define PMIC_LDO33_ONOFF1_ECO_reg_ldo33_en_a_START (0)
#define PMIC_LDO33_ONOFF1_ECO_reg_ldo33_en_a_END (0)
#define PMIC_LDO33_ONOFF1_ECO_st_ldo33_en_START (1)
#define PMIC_LDO33_ONOFF1_ECO_st_ldo33_en_END (1)
#define PMIC_LDO33_ONOFF1_ECO_reg_ldo33_eco_en_START (4)
#define PMIC_LDO33_ONOFF1_ECO_reg_ldo33_eco_en_END (4)
#define PMIC_LDO33_ONOFF1_ECO_st_ldo33_eco_en_START (5)
#define PMIC_LDO33_ONOFF1_ECO_st_ldo33_eco_en_END (5)
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
        unsigned char ldo33_ocp_enn : 1;
        unsigned char ldo33_eco_set : 1;
        unsigned char ldo33_vrset : 3;
        unsigned char reserved : 3;
    } reg;
} PMIC_LDO33_CFG0_UNION;
#endif
#define PMIC_LDO33_CFG0_ldo33_ocp_enn_START (0)
#define PMIC_LDO33_CFG0_ldo33_ocp_enn_END (0)
#define PMIC_LDO33_CFG0_ldo33_eco_set_START (1)
#define PMIC_LDO33_CFG0_ldo33_eco_set_END (1)
#define PMIC_LDO33_CFG0_ldo33_vrset_START (2)
#define PMIC_LDO33_CFG0_ldo33_vrset_END (4)
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
        unsigned char ldo40_vset : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_LDO40_VSET_UNION;
#endif
#define PMIC_LDO40_VSET_ldo40_vset_START (0)
#define PMIC_LDO40_VSET_ldo40_vset_END (5)
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
        unsigned char ldo40_bw_en : 1;
        unsigned char ldo40_vrset : 3;
        unsigned char reserved : 4;
    } reg;
} PMIC_LDO40_CFG0_UNION;
#endif
#define PMIC_LDO40_CFG0_ldo40_bw_en_START (0)
#define PMIC_LDO40_CFG0_ldo40_bw_en_END (0)
#define PMIC_LDO40_CFG0_ldo40_vrset_START (1)
#define PMIC_LDO40_CFG0_ldo40_vrset_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo40_ocpd_vos_set : 1;
        unsigned char ldo40_ocpd_set : 2;
        unsigned char ldo40_ocp_enn : 1;
        unsigned char ldo40_eco_set : 1;
        unsigned char ldo40_comp : 2;
        unsigned char ldo40_bypass_ctrl_enn : 1;
    } reg;
} PMIC_LDO40_CFG1_UNION;
#endif
#define PMIC_LDO40_CFG1_ldo40_ocpd_vos_set_START (0)
#define PMIC_LDO40_CFG1_ldo40_ocpd_vos_set_END (0)
#define PMIC_LDO40_CFG1_ldo40_ocpd_set_START (1)
#define PMIC_LDO40_CFG1_ldo40_ocpd_set_END (2)
#define PMIC_LDO40_CFG1_ldo40_ocp_enn_START (3)
#define PMIC_LDO40_CFG1_ldo40_ocp_enn_END (3)
#define PMIC_LDO40_CFG1_ldo40_eco_set_START (4)
#define PMIC_LDO40_CFG1_ldo40_eco_set_END (4)
#define PMIC_LDO40_CFG1_ldo40_comp_START (5)
#define PMIC_LDO40_CFG1_ldo40_comp_END (6)
#define PMIC_LDO40_CFG1_ldo40_bypass_ctrl_enn_START (7)
#define PMIC_LDO40_CFG1_ldo40_bypass_ctrl_enn_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ldo40_vgpr_en : 1;
        unsigned char ldo40_vea_ctrl : 1;
        unsigned char ldo40_sst_set : 1;
        unsigned char ldo40_pd_set : 1;
        unsigned char ldo40_ocps_set : 2;
        unsigned char ldo40_ocps_en : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_LDO40_CFG2_UNION;
#endif
#define PMIC_LDO40_CFG2_ldo40_vgpr_en_START (0)
#define PMIC_LDO40_CFG2_ldo40_vgpr_en_END (0)
#define PMIC_LDO40_CFG2_ldo40_vea_ctrl_START (1)
#define PMIC_LDO40_CFG2_ldo40_vea_ctrl_END (1)
#define PMIC_LDO40_CFG2_ldo40_sst_set_START (2)
#define PMIC_LDO40_CFG2_ldo40_sst_set_END (2)
#define PMIC_LDO40_CFG2_ldo40_pd_set_START (3)
#define PMIC_LDO40_CFG2_ldo40_pd_set_END (3)
#define PMIC_LDO40_CFG2_ldo40_ocps_set_START (4)
#define PMIC_LDO40_CFG2_ldo40_ocps_set_END (5)
#define PMIC_LDO40_CFG2_ldo40_ocps_en_START (6)
#define PMIC_LDO40_CFG2_ldo40_ocps_en_END (6)
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
        unsigned char fary_done_int : 1;
        unsigned char reserved : 1;
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
#define PMIC_COUL_IRQ_fary_done_int_START (6)
#define PMIC_COUL_IRQ_fary_done_int_END (6)
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
        unsigned char fary_done_int_mk : 1;
        unsigned char reserved : 1;
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
#define PMIC_COUL_IRQ_MASK_fary_done_int_mk_START (6)
#define PMIC_COUL_IRQ_MASK_fary_done_int_mk_END (6)
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
        unsigned char fary_sd_sel : 1;
        unsigned char fary_eco_sel : 1;
        unsigned char fary_swr_en : 1;
        unsigned char reserved : 5;
    } reg;
} PMIC_FARY_MODE_UNION;
#endif
#define PMIC_FARY_MODE_fary_sd_sel_START (0)
#define PMIC_FARY_MODE_fary_sd_sel_END (0)
#define PMIC_FARY_MODE_fary_eco_sel_START (1)
#define PMIC_FARY_MODE_fary_eco_sel_END (1)
#define PMIC_FARY_MODE_fary_swr_en_START (2)
#define PMIC_FARY_MODE_fary_swr_en_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_tw0 : 3;
        unsigned char reserved_0: 1;
        unsigned char fary_tw1 : 3;
        unsigned char reserved_1: 1;
    } reg;
} PMIC_FARY_DELAY_UNION;
#endif
#define PMIC_FARY_DELAY_fary_tw0_START (0)
#define PMIC_FARY_DELAY_fary_tw0_END (2)
#define PMIC_FARY_DELAY_fary_tw1_START (4)
#define PMIC_FARY_DELAY_fary_tw1_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char xo_core_en_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_FARY_CTRL0_UNION;
#endif
#define PMIC_FARY_CTRL0_xo_core_en_ctrl_START (0)
#define PMIC_FARY_CTRL0_xo_core_en_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_vbat0_s0 : 8;
    } reg;
} PMIC_FARY_VBAT0_S0_UNION;
#endif
#define PMIC_FARY_VBAT0_S0_fary_vbat0_s0_START (0)
#define PMIC_FARY_VBAT0_S0_fary_vbat0_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_vbat0_s1 : 8;
    } reg;
} PMIC_FARY_VBAT0_S1_UNION;
#endif
#define PMIC_FARY_VBAT0_S1_fary_vbat0_s1_START (0)
#define PMIC_FARY_VBAT0_S1_fary_vbat0_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_vbat0_s2 : 8;
    } reg;
} PMIC_FARY_VBAT0_S2_UNION;
#endif
#define PMIC_FARY_VBAT0_S2_fary_vbat0_s2_START (0)
#define PMIC_FARY_VBAT0_S2_fary_vbat0_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_vbat1_s0 : 8;
    } reg;
} PMIC_FARY_VBAT1_S0_UNION;
#endif
#define PMIC_FARY_VBAT1_S0_fary_vbat1_s0_START (0)
#define PMIC_FARY_VBAT1_S0_fary_vbat1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_vbat1_s1 : 8;
    } reg;
} PMIC_FARY_VBAT1_S1_UNION;
#endif
#define PMIC_FARY_VBAT1_S1_fary_vbat1_s1_START (0)
#define PMIC_FARY_VBAT1_S1_fary_vbat1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_vbat1_s2 : 8;
    } reg;
} PMIC_FARY_VBAT1_S2_UNION;
#endif
#define PMIC_FARY_VBAT1_S2_fary_vbat1_s2_START (0)
#define PMIC_FARY_VBAT1_S2_fary_vbat1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_ibat0_s0 : 8;
    } reg;
} PMIC_FARY_IBAT0_S0_UNION;
#endif
#define PMIC_FARY_IBAT0_S0_fary_ibat0_s0_START (0)
#define PMIC_FARY_IBAT0_S0_fary_ibat0_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_ibat0_s1 : 8;
    } reg;
} PMIC_FARY_IBAT0_S1_UNION;
#endif
#define PMIC_FARY_IBAT0_S1_fary_ibat0_s1_START (0)
#define PMIC_FARY_IBAT0_S1_fary_ibat0_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_ibat0_s2 : 8;
    } reg;
} PMIC_FARY_IBAT0_S2_UNION;
#endif
#define PMIC_FARY_IBAT0_S2_fary_ibat0_s2_START (0)
#define PMIC_FARY_IBAT0_S2_fary_ibat0_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_ibat1_s0 : 8;
    } reg;
} PMIC_FARY_IBAT1_S0_UNION;
#endif
#define PMIC_FARY_IBAT1_S0_fary_ibat1_s0_START (0)
#define PMIC_FARY_IBAT1_S0_fary_ibat1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_ibat1_s1 : 8;
    } reg;
} PMIC_FARY_IBAT1_S1_UNION;
#endif
#define PMIC_FARY_IBAT1_S1_fary_ibat1_s1_START (0)
#define PMIC_FARY_IBAT1_S1_fary_ibat1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_ibat1_s2 : 8;
    } reg;
} PMIC_FARY_IBAT1_S2_UNION;
#endif
#define PMIC_FARY_IBAT1_S2_fary_ibat1_s2_START (0)
#define PMIC_FARY_IBAT1_S2_fary_ibat1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_vsys0_s0 : 8;
    } reg;
} PMIC_FARY_VSYS0_S0_UNION;
#endif
#define PMIC_FARY_VSYS0_S0_fary_vsys0_s0_START (0)
#define PMIC_FARY_VSYS0_S0_fary_vsys0_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_vsys0_s1 : 8;
    } reg;
} PMIC_FARY_VSYS0_S1_UNION;
#endif
#define PMIC_FARY_VSYS0_S1_fary_vsys0_s1_START (0)
#define PMIC_FARY_VSYS0_S1_fary_vsys0_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_vsys0_s2 : 8;
    } reg;
} PMIC_FARY_VSYS0_S2_UNION;
#endif
#define PMIC_FARY_VSYS0_S2_fary_vsys0_s2_START (0)
#define PMIC_FARY_VSYS0_S2_fary_vsys0_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_vsys1_s0 : 8;
    } reg;
} PMIC_FARY_VSYS1_S0_UNION;
#endif
#define PMIC_FARY_VSYS1_S0_fary_vsys1_s0_START (0)
#define PMIC_FARY_VSYS1_S0_fary_vsys1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_vsys1_s1 : 8;
    } reg;
} PMIC_FARY_VSYS1_S1_UNION;
#endif
#define PMIC_FARY_VSYS1_S1_fary_vsys1_s1_START (0)
#define PMIC_FARY_VSYS1_S1_fary_vsys1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_vsys1_s2 : 8;
    } reg;
} PMIC_FARY_VSYS1_S2_UNION;
#endif
#define PMIC_FARY_VSYS1_S2_fary_vsys1_s2_START (0)
#define PMIC_FARY_VSYS1_S2_fary_vsys1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_isys0_s0 : 8;
    } reg;
} PMIC_FARY_ISYS0_S0_UNION;
#endif
#define PMIC_FARY_ISYS0_S0_fary_isys0_s0_START (0)
#define PMIC_FARY_ISYS0_S0_fary_isys0_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_isys0_s1 : 8;
    } reg;
} PMIC_FARY_ISYS0_S1_UNION;
#endif
#define PMIC_FARY_ISYS0_S1_fary_isys0_s1_START (0)
#define PMIC_FARY_ISYS0_S1_fary_isys0_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_isys0_s2 : 8;
    } reg;
} PMIC_FARY_ISYS0_S2_UNION;
#endif
#define PMIC_FARY_ISYS0_S2_fary_isys0_s2_START (0)
#define PMIC_FARY_ISYS0_S2_fary_isys0_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_isys1_s0 : 8;
    } reg;
} PMIC_FARY_ISYS1_S0_UNION;
#endif
#define PMIC_FARY_ISYS1_S0_fary_isys1_s0_START (0)
#define PMIC_FARY_ISYS1_S0_fary_isys1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_isys1_s1 : 8;
    } reg;
} PMIC_FARY_ISYS1_S1_UNION;
#endif
#define PMIC_FARY_ISYS1_S1_fary_isys1_s1_START (0)
#define PMIC_FARY_ISYS1_S1_fary_isys1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fary_isys1_s2 : 8;
    } reg;
} PMIC_FARY_ISYS1_S2_UNION;
#endif
#define PMIC_FARY_ISYS1_S2_fary_isys1_s2_START (0)
#define PMIC_FARY_ISYS1_S2_fary_isys1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char np_coul_fary : 8;
    } reg;
} PMIC_FARY_RESERVE0_UNION;
#endif
#define PMIC_FARY_RESERVE0_np_coul_fary_START (0)
#define PMIC_FARY_RESERVE0_np_coul_fary_END (7)
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
        unsigned char ovp_discharge_sel : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_OVP_DISCHARGE_CTRL_UNION;
#endif
#define PMIC_OVP_DISCHARGE_CTRL_ovp_discharge_sel_START (0)
#define PMIC_OVP_DISCHARGE_CTRL_ovp_discharge_sel_END (0)
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
    unsigned char value;
    struct
    {
        unsigned char eis_disen_curr_chanel : 2;
        unsigned char eis_disen_volt_chanel : 2;
        unsigned char eis_chopper2_en : 1;
        unsigned char eis_chopper1_en : 1;
        unsigned char reserved : 2;
    } reg;
} PMIC_EIS_ANA_CTRL0_UNION;
#endif
#define PMIC_EIS_ANA_CTRL0_eis_disen_curr_chanel_START (0)
#define PMIC_EIS_ANA_CTRL0_eis_disen_curr_chanel_END (1)
#define PMIC_EIS_ANA_CTRL0_eis_disen_volt_chanel_START (2)
#define PMIC_EIS_ANA_CTRL0_eis_disen_volt_chanel_END (3)
#define PMIC_EIS_ANA_CTRL0_eis_chopper2_en_START (4)
#define PMIC_EIS_ANA_CTRL0_eis_chopper2_en_END (4)
#define PMIC_EIS_ANA_CTRL0_eis_chopper1_en_START (5)
#define PMIC_EIS_ANA_CTRL0_eis_chopper1_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_curr_gain_set : 3;
        unsigned char eis_volt_gain_set : 3;
        unsigned char eis_disc_curr_set : 2;
    } reg;
} PMIC_EIS_ANA_CTRL1_UNION;
#endif
#define PMIC_EIS_ANA_CTRL1_eis_curr_gain_set_START (0)
#define PMIC_EIS_ANA_CTRL1_eis_curr_gain_set_END (2)
#define PMIC_EIS_ANA_CTRL1_eis_volt_gain_set_START (3)
#define PMIC_EIS_ANA_CTRL1_eis_volt_gain_set_END (5)
#define PMIC_EIS_ANA_CTRL1_eis_disc_curr_set_START (6)
#define PMIC_EIS_ANA_CTRL1_eis_disc_curr_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_cur_ampcap_sel : 2;
        unsigned char eis_cur_rescap_sel : 2;
        unsigned char eis_volt_cap_sel : 2;
        unsigned char eis_volt_rescap_sel : 2;
    } reg;
} PMIC_EIS_ANA_CTRL2_UNION;
#endif
#define PMIC_EIS_ANA_CTRL2_eis_cur_ampcap_sel_START (0)
#define PMIC_EIS_ANA_CTRL2_eis_cur_ampcap_sel_END (1)
#define PMIC_EIS_ANA_CTRL2_eis_cur_rescap_sel_START (2)
#define PMIC_EIS_ANA_CTRL2_eis_cur_rescap_sel_END (3)
#define PMIC_EIS_ANA_CTRL2_eis_volt_cap_sel_START (4)
#define PMIC_EIS_ANA_CTRL2_eis_volt_cap_sel_END (5)
#define PMIC_EIS_ANA_CTRL2_eis_volt_rescap_sel_START (6)
#define PMIC_EIS_ANA_CTRL2_eis_volt_rescap_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_ibias_set_0 : 8;
    } reg;
} PMIC_EIS_IBIAS_SET_0_UNION;
#endif
#define PMIC_EIS_IBIAS_SET_0_eis_ibias_set_0_START (0)
#define PMIC_EIS_IBIAS_SET_0_eis_ibias_set_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_ibias_set_1 : 8;
    } reg;
} PMIC_EIS_IBIAS_SET_1_UNION;
#endif
#define PMIC_EIS_IBIAS_SET_1_eis_ibias_set_1_START (0)
#define PMIC_EIS_IBIAS_SET_1_eis_ibias_set_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_testmode : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_EIS_TEST_CTRL_UNION;
#endif
#define PMIC_EIS_TEST_CTRL_eis_testmode_START (0)
#define PMIC_EIS_TEST_CTRL_eis_testmode_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_ana_reserve0 : 8;
    } reg;
} PMIC_EIS_RESERVE0_UNION;
#endif
#define PMIC_EIS_RESERVE0_eis_ana_reserve0_START (0)
#define PMIC_EIS_RESERVE0_eis_ana_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_ana_reserve1 : 8;
    } reg;
} PMIC_EIS_RESERVE1_UNION;
#endif
#define PMIC_EIS_RESERVE1_eis_ana_reserve1_START (0)
#define PMIC_EIS_RESERVE1_eis_ana_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_ana_reserve2 : 8;
    } reg;
} PMIC_EIS_RESERVE2_UNION;
#endif
#define PMIC_EIS_RESERVE2_eis_ana_reserve2_START (0)
#define PMIC_EIS_RESERVE2_eis_ana_reserve2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_eis_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_EIS_CTRL0_UNION;
#endif
#define PMIC_EIS_CTRL0_reg_eis_en_START (0)
#define PMIC_EIS_CTRL0_reg_eis_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_eis_dischg_mode : 1;
        unsigned char reg_eis_dischg_en : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_EIS_CTRL1_UNION;
#endif
#define PMIC_EIS_CTRL1_reg_eis_dischg_mode_START (0)
#define PMIC_EIS_CTRL1_reg_eis_dischg_mode_END (0)
#define PMIC_EIS_CTRL1_reg_eis_dischg_en_START (1)
#define PMIC_EIS_CTRL1_reg_eis_dischg_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_eis_tw : 2;
        unsigned char reg_eis_init_chanl2start : 2;
        unsigned char reg_eis_init_adc_average : 2;
        unsigned char reg_eis_init_start : 1;
        unsigned char reserved : 1;
    } reg;
} PMIC_EIS_CTRL2_UNION;
#endif
#define PMIC_EIS_CTRL2_reg_eis_tw_START (0)
#define PMIC_EIS_CTRL2_reg_eis_tw_END (1)
#define PMIC_EIS_CTRL2_reg_eis_init_chanl2start_START (2)
#define PMIC_EIS_CTRL2_reg_eis_init_chanl2start_END (3)
#define PMIC_EIS_CTRL2_reg_eis_init_adc_average_START (4)
#define PMIC_EIS_CTRL2_reg_eis_init_adc_average_END (5)
#define PMIC_EIS_CTRL2_reg_eis_init_start_START (6)
#define PMIC_EIS_CTRL2_reg_eis_init_start_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_v_offset_data_l : 8;
    } reg;
} PMIC_EISADC_INIT_V_OFFSET_DATA_L_UNION;
#endif
#define PMIC_EISADC_INIT_V_OFFSET_DATA_L_eisadc_init_v_offset_data_l_START (0)
#define PMIC_EISADC_INIT_V_OFFSET_DATA_L_eisadc_init_v_offset_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_v_offset_data_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_EISADC_INIT_V_OFFSET_DATA_H_UNION;
#endif
#define PMIC_EISADC_INIT_V_OFFSET_DATA_H_eisadc_init_v_offset_data_h_START (0)
#define PMIC_EISADC_INIT_V_OFFSET_DATA_H_eisadc_init_v_offset_data_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_i_offset_data_l : 8;
    } reg;
} PMIC_EISADC_INIT_I_OFFSET_DATA_L_UNION;
#endif
#define PMIC_EISADC_INIT_I_OFFSET_DATA_L_eisadc_init_i_offset_data_l_START (0)
#define PMIC_EISADC_INIT_I_OFFSET_DATA_L_eisadc_init_i_offset_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_i_offset_data_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_EISADC_INIT_I_OFFSET_DATA_H_UNION;
#endif
#define PMIC_EISADC_INIT_I_OFFSET_DATA_H_eisadc_init_i_offset_data_h_START (0)
#define PMIC_EISADC_INIT_I_OFFSET_DATA_H_eisadc_init_i_offset_data_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_i_data_l : 8;
    } reg;
} PMIC_EISADC_INIT_I_DATA_L_UNION;
#endif
#define PMIC_EISADC_INIT_I_DATA_L_eisadc_init_i_data_l_START (0)
#define PMIC_EISADC_INIT_I_DATA_L_eisadc_init_i_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_i_data_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_EISADC_INIT_I_DATA_H_UNION;
#endif
#define PMIC_EISADC_INIT_I_DATA_H_eisadc_init_i_data_h_START (0)
#define PMIC_EISADC_INIT_I_DATA_H_eisadc_init_i_data_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_t_data_l : 8;
    } reg;
} PMIC_EISADC_T_DATA_L_UNION;
#endif
#define PMIC_EISADC_T_DATA_L_eisadc_t_data_l_START (0)
#define PMIC_EISADC_T_DATA_L_eisadc_t_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_t_data_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_EISADC_T_DATA_H_UNION;
#endif
#define PMIC_EISADC_T_DATA_H_eisadc_t_data_h_START (0)
#define PMIC_EISADC_T_DATA_H_eisadc_t_data_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_v_abs_data_l : 8;
    } reg;
} PMIC_EISADC_INIT_V_ABS_DATA_L_UNION;
#endif
#define PMIC_EISADC_INIT_V_ABS_DATA_L_eisadc_init_v_abs_data_l_START (0)
#define PMIC_EISADC_INIT_V_ABS_DATA_L_eisadc_init_v_abs_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_v_abs_data_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_EISADC_INIT_V_ABS_DATA_H_UNION;
#endif
#define PMIC_EISADC_INIT_V_ABS_DATA_H_eisadc_init_v_abs_data_h_START (0)
#define PMIC_EISADC_INIT_V_ABS_DATA_H_eisadc_init_v_abs_data_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_v_rela_data_l : 8;
    } reg;
} PMIC_EISADC_INIT_V_RELA_DATA_L_UNION;
#endif
#define PMIC_EISADC_INIT_V_RELA_DATA_L_eisadc_init_v_rela_data_l_START (0)
#define PMIC_EISADC_INIT_V_RELA_DATA_L_eisadc_init_v_rela_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_v_rela_data_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_EISADC_INIT_V_RELA_DATA_H_UNION;
#endif
#define PMIC_EISADC_INIT_V_RELA_DATA_H_eisadc_init_v_rela_data_h_START (0)
#define PMIC_EISADC_INIT_V_RELA_DATA_H_eisadc_init_v_rela_data_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_vbat0_data_l : 8;
    } reg;
} PMIC_EISADC_INIT_VBAT0_DATA_L_UNION;
#endif
#define PMIC_EISADC_INIT_VBAT0_DATA_L_eisadc_init_vbat0_data_l_START (0)
#define PMIC_EISADC_INIT_VBAT0_DATA_L_eisadc_init_vbat0_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_vbat0_data_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_EISADC_INIT_VBAT0_DATA_H_UNION;
#endif
#define PMIC_EISADC_INIT_VBAT0_DATA_H_eisadc_init_vbat0_data_h_START (0)
#define PMIC_EISADC_INIT_VBAT0_DATA_H_eisadc_init_vbat0_data_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_eis_det_timer : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_EIS_DET_TIMER_UNION;
#endif
#define PMIC_EIS_DET_TIMER_reg_eis_det_timer_START (0)
#define PMIC_EIS_DET_TIMER_reg_eis_det_timer_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_eis_det_timer_n : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_EIS_DET_TIMER_N_UNION;
#endif
#define PMIC_EIS_DET_TIMER_N_reg_eis_det_timer_n_START (0)
#define PMIC_EIS_DET_TIMER_N_reg_eis_det_timer_n_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_eis_det_timer_m_l : 8;
    } reg;
} PMIC_EIS_DET_TIMER_M_L_UNION;
#endif
#define PMIC_EIS_DET_TIMER_M_L_reg_eis_det_timer_m_l_START (0)
#define PMIC_EIS_DET_TIMER_M_L_reg_eis_det_timer_m_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_eis_det_timer_m_h : 2;
        unsigned char reserved : 6;
    } reg;
} PMIC_EIS_DET_TIMER_M_H_UNION;
#endif
#define PMIC_EIS_DET_TIMER_M_H_reg_eis_det_timer_m_h_START (0)
#define PMIC_EIS_DET_TIMER_M_H_reg_eis_det_timer_m_h_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_eis_ibat_timer_k : 3;
        unsigned char reg_eis_ibat_timer_j : 2;
        unsigned char reserved : 3;
    } reg;
} PMIC_EIS_IBAT_DET_CTRL_UNION;
#endif
#define PMIC_EIS_IBAT_DET_CTRL_reg_eis_ibat_timer_k_START (0)
#define PMIC_EIS_IBAT_DET_CTRL_reg_eis_ibat_timer_k_END (2)
#define PMIC_EIS_IBAT_DET_CTRL_reg_eis_ibat_timer_j_START (3)
#define PMIC_EIS_IBAT_DET_CTRL_reg_eis_ibat_timer_j_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_eis_i_average : 1;
        unsigned char reg_eis_v_average : 1;
        unsigned char reserved : 6;
    } reg;
} PMIC_EIS_ADC_AVERAGE_UNION;
#endif
#define PMIC_EIS_ADC_AVERAGE_reg_eis_i_average_START (0)
#define PMIC_EIS_ADC_AVERAGE_reg_eis_i_average_END (0)
#define PMIC_EIS_ADC_AVERAGE_reg_eis_v_average_START (1)
#define PMIC_EIS_ADC_AVERAGE_reg_eis_v_average_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_eis_vbat0_offset_l : 8;
    } reg;
} PMIC_EIS_VBAT0_OFFSET_L_UNION;
#endif
#define PMIC_EIS_VBAT0_OFFSET_L_reg_eis_vbat0_offset_l_START (0)
#define PMIC_EIS_VBAT0_OFFSET_L_reg_eis_vbat0_offset_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_eis_vbat0_offset_h : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_EIS_VBAT0_OFFSET_H_UNION;
#endif
#define PMIC_EIS_VBAT0_OFFSET_H_reg_eis_vbat0_offset_h_START (0)
#define PMIC_EIS_VBAT0_OFFSET_H_reg_eis_vbat0_offset_h_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_eis_vbat0_offset_sel : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_EIS_VBAT0_OFFSET_SEL_UNION;
#endif
#define PMIC_EIS_VBAT0_OFFSET_SEL_reg_eis_vbat0_offset_sel_START (0)
#define PMIC_EIS_VBAT0_OFFSET_SEL_reg_eis_vbat0_offset_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_eis_hthreshold_err : 8;
    } reg;
} PMIC_EIS_HTHRESHOLD_ERROR_UNION;
#endif
#define PMIC_EIS_HTHRESHOLD_ERROR_reg_eis_hthreshold_err_START (0)
#define PMIC_EIS_HTHRESHOLD_ERROR_reg_eis_hthreshold_err_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_eis_icomp_hthreshold_l : 8;
    } reg;
} PMIC_EIS_ICOMP_HTHRESHOLD_L_UNION;
#endif
#define PMIC_EIS_ICOMP_HTHRESHOLD_L_reg_eis_icomp_hthreshold_l_START (0)
#define PMIC_EIS_ICOMP_HTHRESHOLD_L_reg_eis_icomp_hthreshold_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_eis_icomp_hthreshold_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_EIS_ICOMP_HTHRESHOLD_H_UNION;
#endif
#define PMIC_EIS_ICOMP_HTHRESHOLD_H_reg_eis_icomp_hthreshold_h_START (0)
#define PMIC_EIS_ICOMP_HTHRESHOLD_H_reg_eis_icomp_hthreshold_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_eis_lthreshold_err : 8;
    } reg;
} PMIC_EIS_LTHRESHOLD_ERROR_UNION;
#endif
#define PMIC_EIS_LTHRESHOLD_ERROR_reg_eis_lthreshold_err_START (0)
#define PMIC_EIS_LTHRESHOLD_ERROR_reg_eis_lthreshold_err_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_eis_icomp_lthreshold_l : 8;
    } reg;
} PMIC_EIS_ICOMP_LTHRESHOLD_L_UNION;
#endif
#define PMIC_EIS_ICOMP_LTHRESHOLD_L_reg_eis_icomp_lthreshold_l_START (0)
#define PMIC_EIS_ICOMP_LTHRESHOLD_L_reg_eis_icomp_lthreshold_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_eis_icomp_lthreshold_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_EIS_ICOMP_LTHRESHOLD_H_UNION;
#endif
#define PMIC_EIS_ICOMP_LTHRESHOLD_H_reg_eis_icomp_lthreshold_h_START (0)
#define PMIC_EIS_ICOMP_LTHRESHOLD_H_reg_eis_icomp_lthreshold_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reg_eis_icomp_state_clr : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_EIS_ICOMP_STATE_CLR_UNION;
#endif
#define PMIC_EIS_ICOMP_STATE_CLR_reg_eis_icomp_state_clr_START (0)
#define PMIC_EIS_ICOMP_STATE_CLR_reg_eis_icomp_state_clr_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_i_curr_data_l : 8;
    } reg;
} PMIC_EIS_I_CURRENT_L_UNION;
#endif
#define PMIC_EIS_I_CURRENT_L_eis_i_curr_data_l_START (0)
#define PMIC_EIS_I_CURRENT_L_eis_i_curr_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_i_curr_data_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_EIS_I_CURRENT_H_UNION;
#endif
#define PMIC_EIS_I_CURRENT_H_eis_i_curr_data_h_START (0)
#define PMIC_EIS_I_CURRENT_H_eis_i_curr_data_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d1_data_l : 8;
    } reg;
} PMIC_EIS_VBAT_D1_L_UNION;
#endif
#define PMIC_EIS_VBAT_D1_L_eis_vbat_d1_data_l_START (0)
#define PMIC_EIS_VBAT_D1_L_eis_vbat_d1_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d1_data_h : 4;
        unsigned char eis_i_accu_state_d1 : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_EIS_VBAT_D1_H_UNION;
#endif
#define PMIC_EIS_VBAT_D1_H_eis_vbat_d1_data_h_START (0)
#define PMIC_EIS_VBAT_D1_H_eis_vbat_d1_data_h_END (3)
#define PMIC_EIS_VBAT_D1_H_eis_i_accu_state_d1_START (4)
#define PMIC_EIS_VBAT_D1_H_eis_i_accu_state_d1_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d2_data_l : 8;
    } reg;
} PMIC_EIS_VBAT_D2_L_UNION;
#endif
#define PMIC_EIS_VBAT_D2_L_eis_vbat_d2_data_l_START (0)
#define PMIC_EIS_VBAT_D2_L_eis_vbat_d2_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d2_data_h : 4;
        unsigned char eis_i_accu_state_d2 : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_EIS_VBAT_D2_H_UNION;
#endif
#define PMIC_EIS_VBAT_D2_H_eis_vbat_d2_data_h_START (0)
#define PMIC_EIS_VBAT_D2_H_eis_vbat_d2_data_h_END (3)
#define PMIC_EIS_VBAT_D2_H_eis_i_accu_state_d2_START (4)
#define PMIC_EIS_VBAT_D2_H_eis_i_accu_state_d2_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d3_data_l : 8;
    } reg;
} PMIC_EIS_VBAT_D3_L_UNION;
#endif
#define PMIC_EIS_VBAT_D3_L_eis_vbat_d3_data_l_START (0)
#define PMIC_EIS_VBAT_D3_L_eis_vbat_d3_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d3_data_h : 4;
        unsigned char eis_i_accu_state_d3 : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_EIS_VBAT_D3_H_UNION;
#endif
#define PMIC_EIS_VBAT_D3_H_eis_vbat_d3_data_h_START (0)
#define PMIC_EIS_VBAT_D3_H_eis_vbat_d3_data_h_END (3)
#define PMIC_EIS_VBAT_D3_H_eis_i_accu_state_d3_START (4)
#define PMIC_EIS_VBAT_D3_H_eis_i_accu_state_d3_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d4_data_l : 8;
    } reg;
} PMIC_EIS_VBAT_D4_L_UNION;
#endif
#define PMIC_EIS_VBAT_D4_L_eis_vbat_d4_data_l_START (0)
#define PMIC_EIS_VBAT_D4_L_eis_vbat_d4_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d4_data_h : 4;
        unsigned char eis_i_accu_state_d4 : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_EIS_VBAT_D4_H_UNION;
#endif
#define PMIC_EIS_VBAT_D4_H_eis_vbat_d4_data_h_START (0)
#define PMIC_EIS_VBAT_D4_H_eis_vbat_d4_data_h_END (3)
#define PMIC_EIS_VBAT_D4_H_eis_i_accu_state_d4_START (4)
#define PMIC_EIS_VBAT_D4_H_eis_i_accu_state_d4_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d5_data_l : 8;
    } reg;
} PMIC_EIS_VBAT_D5_L_UNION;
#endif
#define PMIC_EIS_VBAT_D5_L_eis_vbat_d5_data_l_START (0)
#define PMIC_EIS_VBAT_D5_L_eis_vbat_d5_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d5_data_h : 4;
        unsigned char eis_i_accu_state_d5 : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_EIS_VBAT_D5_H_UNION;
#endif
#define PMIC_EIS_VBAT_D5_H_eis_vbat_d5_data_h_START (0)
#define PMIC_EIS_VBAT_D5_H_eis_vbat_d5_data_h_END (3)
#define PMIC_EIS_VBAT_D5_H_eis_i_accu_state_d5_START (4)
#define PMIC_EIS_VBAT_D5_H_eis_i_accu_state_d5_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d6_data_l : 8;
    } reg;
} PMIC_EIS_VBAT_D6_L_UNION;
#endif
#define PMIC_EIS_VBAT_D6_L_eis_vbat_d6_data_l_START (0)
#define PMIC_EIS_VBAT_D6_L_eis_vbat_d6_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d6_data_h : 4;
        unsigned char eis_i_accu_state_d6 : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_EIS_VBAT_D6_H_UNION;
#endif
#define PMIC_EIS_VBAT_D6_H_eis_vbat_d6_data_h_START (0)
#define PMIC_EIS_VBAT_D6_H_eis_vbat_d6_data_h_END (3)
#define PMIC_EIS_VBAT_D6_H_eis_i_accu_state_d6_START (4)
#define PMIC_EIS_VBAT_D6_H_eis_i_accu_state_d6_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d7_data_l : 8;
    } reg;
} PMIC_EIS_VBAT_D7_L_UNION;
#endif
#define PMIC_EIS_VBAT_D7_L_eis_vbat_d7_data_l_START (0)
#define PMIC_EIS_VBAT_D7_L_eis_vbat_d7_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d7_data_h : 4;
        unsigned char eis_i_accu_state_d7 : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_EIS_VBAT_D7_H_UNION;
#endif
#define PMIC_EIS_VBAT_D7_H_eis_vbat_d7_data_h_START (0)
#define PMIC_EIS_VBAT_D7_H_eis_vbat_d7_data_h_END (3)
#define PMIC_EIS_VBAT_D7_H_eis_i_accu_state_d7_START (4)
#define PMIC_EIS_VBAT_D7_H_eis_i_accu_state_d7_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d8_data_l : 8;
    } reg;
} PMIC_EIS_VBAT_D8_L_UNION;
#endif
#define PMIC_EIS_VBAT_D8_L_eis_vbat_d8_data_l_START (0)
#define PMIC_EIS_VBAT_D8_L_eis_vbat_d8_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d8_data_h : 4;
        unsigned char eis_i_accu_state_d8 : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_EIS_VBAT_D8_H_UNION;
#endif
#define PMIC_EIS_VBAT_D8_H_eis_vbat_d8_data_h_START (0)
#define PMIC_EIS_VBAT_D8_H_eis_vbat_d8_data_h_END (3)
#define PMIC_EIS_VBAT_D8_H_eis_i_accu_state_d8_START (4)
#define PMIC_EIS_VBAT_D8_H_eis_i_accu_state_d8_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d9_data_l : 8;
    } reg;
} PMIC_EIS_VBAT_D9_L_UNION;
#endif
#define PMIC_EIS_VBAT_D9_L_eis_vbat_d9_data_l_START (0)
#define PMIC_EIS_VBAT_D9_L_eis_vbat_d9_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d9_data_h : 4;
        unsigned char eis_i_accu_state_d9 : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_EIS_VBAT_D9_H_UNION;
#endif
#define PMIC_EIS_VBAT_D9_H_eis_vbat_d9_data_h_START (0)
#define PMIC_EIS_VBAT_D9_H_eis_vbat_d9_data_h_END (3)
#define PMIC_EIS_VBAT_D9_H_eis_i_accu_state_d9_START (4)
#define PMIC_EIS_VBAT_D9_H_eis_i_accu_state_d9_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d10_data_l : 8;
    } reg;
} PMIC_EIS_VBAT_D10_L_UNION;
#endif
#define PMIC_EIS_VBAT_D10_L_eis_vbat_d10_data_l_START (0)
#define PMIC_EIS_VBAT_D10_L_eis_vbat_d10_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d10_data_h : 4;
        unsigned char eis_i_accu_state_d10 : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_EIS_VBAT_D10_H_UNION;
#endif
#define PMIC_EIS_VBAT_D10_H_eis_vbat_d10_data_h_START (0)
#define PMIC_EIS_VBAT_D10_H_eis_vbat_d10_data_h_END (3)
#define PMIC_EIS_VBAT_D10_H_eis_i_accu_state_d10_START (4)
#define PMIC_EIS_VBAT_D10_H_eis_i_accu_state_d10_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d11_data_l : 8;
    } reg;
} PMIC_EIS_VBAT_D11_L_UNION;
#endif
#define PMIC_EIS_VBAT_D11_L_eis_vbat_d11_data_l_START (0)
#define PMIC_EIS_VBAT_D11_L_eis_vbat_d11_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d11_data_h : 4;
        unsigned char eis_i_accu_state_d11 : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_EIS_VBAT_D11_H_UNION;
#endif
#define PMIC_EIS_VBAT_D11_H_eis_vbat_d11_data_h_START (0)
#define PMIC_EIS_VBAT_D11_H_eis_vbat_d11_data_h_END (3)
#define PMIC_EIS_VBAT_D11_H_eis_i_accu_state_d11_START (4)
#define PMIC_EIS_VBAT_D11_H_eis_i_accu_state_d11_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d12_data_l : 8;
    } reg;
} PMIC_EIS_VBAT_D12_L_UNION;
#endif
#define PMIC_EIS_VBAT_D12_L_eis_vbat_d12_data_l_START (0)
#define PMIC_EIS_VBAT_D12_L_eis_vbat_d12_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d12_data_h : 4;
        unsigned char eis_i_accu_state_d12 : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_EIS_VBAT_D12_H_UNION;
#endif
#define PMIC_EIS_VBAT_D12_H_eis_vbat_d12_data_h_START (0)
#define PMIC_EIS_VBAT_D12_H_eis_vbat_d12_data_h_END (3)
#define PMIC_EIS_VBAT_D12_H_eis_i_accu_state_d12_START (4)
#define PMIC_EIS_VBAT_D12_H_eis_i_accu_state_d12_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d13_data_l : 8;
    } reg;
} PMIC_EIS_VBAT_D13_L_UNION;
#endif
#define PMIC_EIS_VBAT_D13_L_eis_vbat_d13_data_l_START (0)
#define PMIC_EIS_VBAT_D13_L_eis_vbat_d13_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d13_data_h : 4;
        unsigned char eis_i_accu_state_d13 : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_EIS_VBAT_D13_H_UNION;
#endif
#define PMIC_EIS_VBAT_D13_H_eis_vbat_d13_data_h_START (0)
#define PMIC_EIS_VBAT_D13_H_eis_vbat_d13_data_h_END (3)
#define PMIC_EIS_VBAT_D13_H_eis_i_accu_state_d13_START (4)
#define PMIC_EIS_VBAT_D13_H_eis_i_accu_state_d13_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d14_data_l : 8;
    } reg;
} PMIC_EIS_VBAT_D14_L_UNION;
#endif
#define PMIC_EIS_VBAT_D14_L_eis_vbat_d14_data_l_START (0)
#define PMIC_EIS_VBAT_D14_L_eis_vbat_d14_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d14_data_h : 4;
        unsigned char eis_i_accu_state_d14 : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_EIS_VBAT_D14_H_UNION;
#endif
#define PMIC_EIS_VBAT_D14_H_eis_vbat_d14_data_h_START (0)
#define PMIC_EIS_VBAT_D14_H_eis_vbat_d14_data_h_END (3)
#define PMIC_EIS_VBAT_D14_H_eis_i_accu_state_d14_START (4)
#define PMIC_EIS_VBAT_D14_H_eis_i_accu_state_d14_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d15_data_l : 8;
    } reg;
} PMIC_EIS_VBAT_D15_L_UNION;
#endif
#define PMIC_EIS_VBAT_D15_L_eis_vbat_d15_data_l_START (0)
#define PMIC_EIS_VBAT_D15_L_eis_vbat_d15_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d15_data_h : 4;
        unsigned char eis_i_accu_state_d15 : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_EIS_VBAT_D15_H_UNION;
#endif
#define PMIC_EIS_VBAT_D15_H_eis_vbat_d15_data_h_START (0)
#define PMIC_EIS_VBAT_D15_H_eis_vbat_d15_data_h_END (3)
#define PMIC_EIS_VBAT_D15_H_eis_i_accu_state_d15_START (4)
#define PMIC_EIS_VBAT_D15_H_eis_i_accu_state_d15_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d16_data_l : 8;
    } reg;
} PMIC_EIS_VBAT_D16_L_UNION;
#endif
#define PMIC_EIS_VBAT_D16_L_eis_vbat_d16_data_l_START (0)
#define PMIC_EIS_VBAT_D16_L_eis_vbat_d16_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_d16_data_h : 4;
        unsigned char eis_i_accu_state_d16 : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_EIS_VBAT_D16_H_UNION;
#endif
#define PMIC_EIS_VBAT_D16_H_eis_vbat_d16_data_h_START (0)
#define PMIC_EIS_VBAT_D16_H_eis_vbat_d16_data_h_END (3)
#define PMIC_EIS_VBAT_D16_H_eis_i_accu_state_d16_START (4)
#define PMIC_EIS_VBAT_D16_H_eis_i_accu_state_d16_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_ibat_p_d4_data_l : 8;
    } reg;
} PMIC_EIS_IBAT_P_D4_L_UNION;
#endif
#define PMIC_EIS_IBAT_P_D4_L_eis_ibat_p_d4_data_l_START (0)
#define PMIC_EIS_IBAT_P_D4_L_eis_ibat_p_d4_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_ibat_p_d4_data_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_EIS_IBAT_P_D4_H_UNION;
#endif
#define PMIC_EIS_IBAT_P_D4_H_eis_ibat_p_d4_data_h_START (0)
#define PMIC_EIS_IBAT_P_D4_H_eis_ibat_p_d4_data_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_ibat_p_d8_data_l : 8;
    } reg;
} PMIC_EIS_IBAT_P_D8_L_UNION;
#endif
#define PMIC_EIS_IBAT_P_D8_L_eis_ibat_p_d8_data_l_START (0)
#define PMIC_EIS_IBAT_P_D8_L_eis_ibat_p_d8_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_ibat_p_d8_data_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_EIS_IBAT_P_D8_H_UNION;
#endif
#define PMIC_EIS_IBAT_P_D8_H_eis_ibat_p_d8_data_h_START (0)
#define PMIC_EIS_IBAT_P_D8_H_eis_ibat_p_d8_data_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_ibat_p_d12_data_l : 8;
    } reg;
} PMIC_EIS_IBAT_P_D12_L_UNION;
#endif
#define PMIC_EIS_IBAT_P_D12_L_eis_ibat_p_d12_data_l_START (0)
#define PMIC_EIS_IBAT_P_D12_L_eis_ibat_p_d12_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_ibat_p_d12_data_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_EIS_IBAT_P_D12_H_UNION;
#endif
#define PMIC_EIS_IBAT_P_D12_H_eis_ibat_p_d12_data_h_START (0)
#define PMIC_EIS_IBAT_P_D12_H_eis_ibat_p_d12_data_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_ibat_p_d16_data_l : 8;
    } reg;
} PMIC_EIS_IBAT_P_D16_L_UNION;
#endif
#define PMIC_EIS_IBAT_P_D16_L_eis_ibat_p_d16_data_l_START (0)
#define PMIC_EIS_IBAT_P_D16_L_eis_ibat_p_d16_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_ibat_p_d16_data_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_EIS_IBAT_P_D16_H_UNION;
#endif
#define PMIC_EIS_IBAT_P_D16_H_eis_ibat_p_d16_data_h_START (0)
#define PMIC_EIS_IBAT_P_D16_H_eis_ibat_p_d16_data_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_det_t_num_l : 8;
    } reg;
} PMIC_EIS_DET_T_NUM_L_UNION;
#endif
#define PMIC_EIS_DET_T_NUM_L_eis_det_t_num_l_START (0)
#define PMIC_EIS_DET_T_NUM_L_eis_det_t_num_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_det_t_num_h : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_EIS_DET_T_NUM_H_UNION;
#endif
#define PMIC_EIS_DET_T_NUM_H_eis_det_t_num_h_START (0)
#define PMIC_EIS_DET_T_NUM_H_eis_det_t_num_h_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_sample_num : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_EIS_VBAT_DET_NUM_UNION;
#endif
#define PMIC_EIS_VBAT_DET_NUM_eis_vbat_sample_num_START (0)
#define PMIC_EIS_VBAT_DET_NUM_eis_vbat_sample_num_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_flag : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_EIS_DET_FLAG_UNION;
#endif
#define PMIC_EIS_DET_FLAG_eis_flag_START (0)
#define PMIC_EIS_DET_FLAG_eis_flag_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_initial_end_flag : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_EIS_INIT_END_FLAG_UNION;
#endif
#define PMIC_EIS_INIT_END_FLAG_eis_initial_end_flag_START (0)
#define PMIC_EIS_INIT_END_FLAG_eis_initial_end_flag_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 1;
        unsigned char eis_adc_reserve : 7;
    } reg;
} PMIC_EISADC_RESERVE_UNION;
#endif
#define PMIC_EISADC_RESERVE_eis_adc_reserve_START (1)
#define PMIC_EISADC_RESERVE_eis_adc_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_adc_valid : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_EISADC_CONV_STATUS_UNION;
#endif
#define PMIC_EISADC_CONV_STATUS_eis_adc_valid_START (0)
#define PMIC_EISADC_CONV_STATUS_eis_adc_valid_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_adc_data_l : 8;
    } reg;
} PMIC_EISADC_DATA_L_UNION;
#endif
#define PMIC_EISADC_DATA_L_eis_adc_data_l_START (0)
#define PMIC_EISADC_DATA_L_eis_adc_data_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_adc_data_h : 4;
        unsigned char reserved : 4;
    } reg;
} PMIC_EISADC_DATA_H_UNION;
#endif
#define PMIC_EISADC_DATA_H_eis_adc_data_h_START (0)
#define PMIC_EISADC_DATA_H_eis_adc_data_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_adc_buffer_sel : 1;
        unsigned char eis_adc_config : 7;
    } reg;
} PMIC_EISADC_CONV_UNION;
#endif
#define PMIC_EISADC_CONV_eis_adc_buffer_sel_START (0)
#define PMIC_EISADC_CONV_eis_adc_buffer_sel_END (0)
#define PMIC_EISADC_CONV_eis_adc_config_START (1)
#define PMIC_EISADC_CONV_eis_adc_config_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_adc_ibias_sel : 8;
    } reg;
} PMIC_EIS_ADC_CURRENT_UNION;
#endif
#define PMIC_EIS_ADC_CURRENT_eis_adc_ibias_sel_START (0)
#define PMIC_EIS_ADC_CURRENT_eis_adc_ibias_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_soft_rst_cfg : 8;
    } reg;
} PMIC_EIS_SOFT_RST_UNION;
#endif
#define PMIC_EIS_SOFT_RST_eis_soft_rst_cfg_START (0)
#define PMIC_EIS_SOFT_RST_eis_soft_rst_cfg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_clk_gt_bypass : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_EIS_CLK_GT_CTRL_UNION;
#endif
#define PMIC_EIS_CLK_GT_CTRL_eis_clk_gt_bypass_START (0)
#define PMIC_EIS_CLK_GT_CTRL_eis_clk_gt_bypass_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_debug_en : 8;
    } reg;
} PMIC_EIS_DEBUG_EN_UNION;
#endif
#define PMIC_EIS_DEBUG_EN_eis_debug_en_START (0)
#define PMIC_EIS_DEBUG_EN_eis_debug_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dbg_eis_ana_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_DEBUG_EIS_EN_UNION;
#endif
#define PMIC_DEBUG_EIS_EN_dbg_eis_ana_en_START (0)
#define PMIC_DEBUG_EIS_EN_dbg_eis_ana_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dbg_eis_dischg_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_DEBUG_EIS_DISCHG_EN_UNION;
#endif
#define PMIC_DEBUG_EIS_DISCHG_EN_dbg_eis_dischg_en_START (0)
#define PMIC_DEBUG_EIS_DISCHG_EN_dbg_eis_dischg_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dbg_eis_chanl_sel : 3;
        unsigned char reserved : 5;
    } reg;
} PMIC_DEBUG_EIS_CHANL_UNION;
#endif
#define PMIC_DEBUG_EIS_CHANL_dbg_eis_chanl_sel_START (0)
#define PMIC_DEBUG_EIS_CHANL_dbg_eis_chanl_sel_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dbg_eisadc_chanl_sel : 5;
        unsigned char reserved_0 : 2;
        unsigned char reserved_1 : 1;
    } reg;
} PMIC_DEBUG_EISADC_CTRL_UNION;
#endif
#define PMIC_DEBUG_EISADC_CTRL_dbg_eisadc_chanl_sel_START (0)
#define PMIC_DEBUG_EISADC_CTRL_dbg_eisadc_chanl_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dbg_eis_adc_start : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_DEBUG_EISADC_START_UNION;
#endif
#define PMIC_DEBUG_EISADC_START_dbg_eis_adc_start_START (0)
#define PMIC_DEBUG_EISADC_START_dbg_eis_adc_start_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_adc_chopper_en : 1;
        unsigned char reserved : 7;
    } reg;
} PMIC_EISADC_MODE_CFG_UNION;
#endif
#define PMIC_EISADC_MODE_CFG_eis_adc_chopper_en_START (0)
#define PMIC_EISADC_MODE_CFG_eis_adc_chopper_en_END (0)
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
        unsigned char dac_ultr_din_sel : 2;
        unsigned char sif_mad_en : 1;
        unsigned char sif_mad_adc_en : 1;
        unsigned char mad_cic_clken : 1;
        unsigned char reserved : 3;
    } reg;
} PMIC_SIF_MAD_UNION;
#endif
#define PMIC_SIF_MAD_dac_ultr_din_sel_START (0)
#define PMIC_SIF_MAD_dac_ultr_din_sel_END (1)
#define PMIC_SIF_MAD_sif_mad_en_START (2)
#define PMIC_SIF_MAD_sif_mad_en_END (2)
#define PMIC_SIF_MAD_sif_mad_adc_en_START (3)
#define PMIC_SIF_MAD_sif_mad_adc_en_END (3)
#define PMIC_SIF_MAD_mad_cic_clken_START (4)
#define PMIC_SIF_MAD_mad_cic_clken_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char mad_cic_gain : 6;
        unsigned char reserved : 2;
    } reg;
} PMIC_SIF_MAD_CIC_GAIN_UNION;
#endif
#define PMIC_SIF_MAD_CIC_GAIN_mad_cic_gain_START (0)
#define PMIC_SIF_MAD_CIC_GAIN_mad_cic_gain_END (5)
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
        unsigned char codec_ana_rw_53 : 8;
    } reg;
} PMIC_CODEC_ANA_RW53_UNION;
#endif
#define PMIC_CODEC_ANA_RW53_codec_ana_rw_53_START (0)
#define PMIC_CODEC_ANA_RW53_codec_ana_rw_53_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_54 : 8;
    } reg;
} PMIC_CODEC_ANA_RW54_UNION;
#endif
#define PMIC_CODEC_ANA_RW54_codec_ana_rw_54_START (0)
#define PMIC_CODEC_ANA_RW54_codec_ana_rw_54_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_55 : 8;
    } reg;
} PMIC_CODEC_ANA_RW55_UNION;
#endif
#define PMIC_CODEC_ANA_RW55_codec_ana_rw_55_START (0)
#define PMIC_CODEC_ANA_RW55_codec_ana_rw_55_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_56 : 8;
    } reg;
} PMIC_CODEC_ANA_RW56_UNION;
#endif
#define PMIC_CODEC_ANA_RW56_codec_ana_rw_56_START (0)
#define PMIC_CODEC_ANA_RW56_codec_ana_rw_56_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_57 : 8;
    } reg;
} PMIC_CODEC_ANA_RW57_UNION;
#endif
#define PMIC_CODEC_ANA_RW57_codec_ana_rw_57_START (0)
#define PMIC_CODEC_ANA_RW57_codec_ana_rw_57_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_58 : 8;
    } reg;
} PMIC_CODEC_ANA_RW58_UNION;
#endif
#define PMIC_CODEC_ANA_RW58_codec_ana_rw_58_START (0)
#define PMIC_CODEC_ANA_RW58_codec_ana_rw_58_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_59 : 8;
    } reg;
} PMIC_CODEC_ANA_RW59_UNION;
#endif
#define PMIC_CODEC_ANA_RW59_codec_ana_rw_59_START (0)
#define PMIC_CODEC_ANA_RW59_codec_ana_rw_59_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_60 : 8;
    } reg;
} PMIC_CODEC_ANA_RW60_UNION;
#endif
#define PMIC_CODEC_ANA_RW60_codec_ana_rw_60_START (0)
#define PMIC_CODEC_ANA_RW60_codec_ana_rw_60_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_61 : 8;
    } reg;
} PMIC_CODEC_ANA_RW61_UNION;
#endif
#define PMIC_CODEC_ANA_RW61_codec_ana_rw_61_START (0)
#define PMIC_CODEC_ANA_RW61_codec_ana_rw_61_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_62 : 8;
    } reg;
} PMIC_CODEC_ANA_RW62_UNION;
#endif
#define PMIC_CODEC_ANA_RW62_codec_ana_rw_62_START (0)
#define PMIC_CODEC_ANA_RW62_codec_ana_rw_62_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char codec_ana_rw_63 : 8;
    } reg;
} PMIC_CODEC_ANA_RW63_UNION;
#endif
#define PMIC_CODEC_ANA_RW63_codec_ana_rw_63_START (0)
#define PMIC_CODEC_ANA_RW63_codec_ana_rw_63_END (7)
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
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
