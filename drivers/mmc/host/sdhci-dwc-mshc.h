#ifndef __SDHCI_DWC_MSHC_H
#define __SDHCI_DWC_MSHC_H

/* sctrl register */
#define SCTRL_SCCTRL			(0x0)
#define SCTRL_CLKDIV9			(0x274)
#define SCTRL_AUTODIV			(0x278)
#define SCTRL_SCPEREN2			(0x190)
#define SCTRL_SCPERDIS2			(0x194)
#define SCTRL_SCPERCLKEN2			(0x198)
#define SCTRL_SCPERSTAT2			(0x19C)
#define SCTRL_SCPERRSTEN0			(0x200)
#define SCTRL_SCPERRSTDIS0			(0x204)
#define SCTRL_SCPERRSTSTAT0		(0x208)
#define IP_RST_EMMC		(0x1 << 21)
#define IP_RST_EMMC_BASE	(0x1 << 22)
/*capricorn  add define start*/
#define SCTRL_SCPERRSTEN1			(0x20C)
#define SCTRL_SCPERRSTDIS1			(0x210)
#define SCTRL_SCPERRSTSTAT1		(0x214)
#define SCTRL_SCCLKDIV10		(0x280)
#define SCTRL_SCPERCTRL7			(0x31C)
#define IP_RST_EMMC_ORLAN	(0x1 << 15)
#define IP_DISRST_EMMC_ORLAN	(0x1 << 15)
#define RG_EMMC_LHEN_IN_RET		(0x1 << 1)
#define RG_EMMC_LHEN_IN_NOR		(0x0 << 1)
#define RG_EMMC_LHEN_MASK		(0x1 << 17)
#define GT_CLK_EMMC_SCTRL		(0x1 << 8)
#define GT_CLK_EMMC_BUS		(0x1 << 9)
#define SEL_CLK_EMMC_PPLL23		(0x1 << 10)
#define SEL_CLK_PPLL23_MASK		(SEL_CLK_EMMC_PPLL23 << 16)

/* emmc1 sysctrl register */
#define MMC1_SYSCTRL_CTRL0		(0x008)
#define MMC1_SYCTRLS_CTRL1		(0x00C)
#define MMC1_SYSCTRL_PEREN0		(0x300)
#define MMC1_SYSCTRL_PERDIS0		(0x304)
#define MMC1_SYSCTRL_PERCLKEN0		(0x308)
#define MMC1_SYSCTRL_PERRSTEN0		(0x310)
#define MMC1_SYSCTRL_PERRSTDIS0		(0x314)
#define MMC1_SYSCTRL_PERRSTSTAT0	(0x318)
#define CMD_DLY_EN		(0x1 << 31)
#define IP_PRST_EMMCPHY			(0x1 << 4)

#define GT_HCLK_EMMC_OR		(0x1 << 1)
#define GT_PCLK_EMMC_OR		(0x1 << 2)
#define GT_TCLK_EMMC_OR		(0x1 << 3)
#define GT_TCLK_EMMC_CQE_OR	(0x1 << 5)

#define GT_CLK_EMMC_OR		(GT_HCLK_EMMC_OR | GT_PCLK_EMMC_OR \
							| GT_TCLK_EMMC_OR | GT_TCLK_EMMC_CQE_OR)
/*capricorn deine end */
/* eMMC sysctrl register */
#define EMMC_SYS_CTRL0				(0x0)
#define GT_STROBE_RX_CLK		(0x1)
#define EMMC_SYS_CTRL1				(0x4)
#define CRESETN_RX			(0x1)
#define TUNING_SEL				(0x1 << 8)	//0:1T, 1:1/2T
#define TUNING_EXTEND			(0x1 << 9)

#define EMMC_SYS_MTCMOS_EN		(0x8)
#define PHY_MTCMOS_EN		(0x1)
#define CTRL_MTCMOS_EN		(0x1 <<1)
#define EMMC_SYS_MTCMOS_ACK		(0xC)
#define PHY_MTCMOS_ACK		(0x1)
#define CTRL_MTCMOS_ACK		(0x1 << 1)

#define EMMC_SYS_LHEN_IN			(0x18)
#define IN_IO_LATCH_MODE		(0x1)
#define EMMC_LHEN_IN			(0x3F)
#define EMMC_SYS_LHEN_INB			(0x1C)
#define INB_IO_LATCH_MODE		(0x1)
#define EMMC_LHEN_INB			(0x3F)

#define EMMC_SYS_PHY_ISOEN		(0x20)
#define CTRL_ISO_EN		(0x1)
#define PHY_ISO_EN		(0x1 << 1)

#define EMMC_SYS_CRG_CFG1			(0x2C)
#define GT_HCLK_EMMC			(0x1 << 1)
#define GT_PCLK_EMMC_PHY		(0x1 << 3)
#define GT_TCLK_EMMC			(0x1 << 4)
#define GT_TCLK_EMMC_CQE		(0x1 << 5)
#define IP_HRST_EMMC			(0x1 << 6)
#define IP_PRST_EMMC_PHY		(0x1 << 8)
#define IP_TRST_EMMC			(0x1 << 9)
#define IP_TRST_EMMC_CQE		(0x1 << 10)
#define EMMC_SYS_GT_CLK		(GT_HCLK_EMMC | GT_PCLK_EMMC_PHY \
								| GT_TCLK_EMMC_CQE)
#define EMMC_SYS_GT_CLK_MASK	(EMMC_SYS_GT_CLK << 16)
#define EMMC_SYS_IP_RST		(IP_HRST_EMMC | IP_PRST_EMMC_PHY \
								|IP_TRST_EMMC_CQE)
#define EMMC_SYS_IP_RST_MASK	(EMMC_SYS_IP_RST << 16)
#define IP_PRST_EMMC_PHY_MASK	(IP_PRST_EMMC_PHY << 16)



#define SDHCI_VENDOR_SPECIFIC_AREA 		(0xE8)
/* the offset is the value of SDHCI_VENDOR_SPECIFIC_AREA */
#define SDHCI_VENDOR_SPECIFIC_AREA_OFFSET	0x500

#define SDHCI_MSHC_CTRL_R	(SDHCI_VENDOR_SPECIFIC_AREA_OFFSET + 0x8)
#define CMD_CONFLICT_CHECK	(0x1)
#define SW_CG_DIS		(0x1 << 4)
#define ACCESS_ALL_REGION       (0x1 << 5)
#define SDHCI_MBIU_CTRL	(SDHCI_VENDOR_SPECIFIC_AREA_OFFSET + 0x10)
#define WRITE_OSRC_LMT_MASK	(0x7 << 24)
#define WRITE_OSRC_LMT		(0x3 << 24)
#define SDHCI_DEBUG_PORT1    (SDHCI_VENDOR_SPECIFIC_AREA_OFFSET + 0x20)
#define SDHCI_DEBUG_PORT2    (SDHCI_VENDOR_SPECIFIC_AREA_OFFSET + 0x24)
#define SDHCI_EMMC_CTRL_R    (SDHCI_VENDOR_SPECIFIC_AREA_OFFSET + 0x2c)
#define CARD_IS_EMMC			(0x1)
#define  ENH_STROBE_ENABLE		(0x1 << 8)
#define SDHCI_AT_CTRL_R	(SDHCI_VENDOR_SPECIFIC_AREA_OFFSET + 0x40)
#define SW_TUNE_EN			(0x1 << 4)
#define SDHCI_AT_STAT_R	(SDHCI_VENDOR_SPECIFIC_AREA_OFFSET + 0x44)
#define CENTER_PH_CODE_MASK	(0xFF)

#define SDHCI_VENDOR2_SPECIFIC_AREA		(0xEA)

/*
 * combo phy register
 */
#define WAIT_STATUS_MAX_RETRY			(10)
#define COMBO_PHY_PHYINITCTRL			(0x4)
#define INIT_EN				(0x1)
#define DLYMEAS_EN			(0x1 << 2)
#define ZCAL_EN				(0x1 << 3)
#define BIST_CLK				(0x1 << 17)
#define COMBO_PHY_PHYINITSTAT			(0x8)
/*This register specify the control the ZQ calibration*/
#define COMBO_PHY_IMPCTRL					(0x24)
#define IMPCTRL_ZCOM_NUM_SHIFT		(12)
#define IMPCTRL_ZCOM_NUM_MASK		(0xF)
#define IMPCTRL_ZCOM_RSP_DLY_MASK	(0x3F)

/*This register specify the ZQ calibration result*/
#define COMBO_PHY_IMPSTATUS         			(0x28)
#define IMPSTATUS_PDRV_SHIFT	(16)
#define IMPSTATUS_DRV_MASK	(0x3F)

#define COMBO_PHY_CMD_DLY_CTRL			(0x230)
#define COMBO_PHY_DATENA_DLY			(0x23C)
#define COMBO_PHY_PHYCTRL2	        		(0x248)
#define PHY_DLYMEAS_UPDATE	(0x1 << 14)
#define COMBO_PHY_DLY_CTL   	    			(0x250)
#define DLY_CPDE_1T_MASK		(0x7FF)
#define DLY_CODE_1T_MASK		(0x7FF)

#define COMBO_PHY_DLY_CTL1   	    			(0x254)
#define DLY_3_CODE_MASK		(0x3FF)
#define DLY_3_CODE_SHIFT		(8)
#define DLY_1_CODE_MASK		(0x3FF)
#define DLY_1_CODE_SHIFT		(21)
#define INV_TX_CLK				(1UL << 31)


#define COMBO_PHY_DLY_CTL2				(0x258)
#define COMBO_PHY_IOCTL_PUPD				(0x260)
#define PUPD_EN_DATA		(0xFF)
#define PUPD_EN_STROBE		(0x100)
#define PUPD_EN_CMD		(0x200)
#define PULLUP_DATA		(0xFF << 16)
#define PULLUP_CMD			(0x200 << 16)

#define COMBO_PHY_IOCTL_RONSEL_1    		(0x264)
#define EMMC_RONSEL_0		(0x7FF)
#define EMMC_RONSEL_1		(0x7FF << 16)
#define EMMC_RONSEL_OR		(0x7FF)
#define COMBO_PHY_IOCTL_RONSEL_2			(0x268)
#define EMMC_RONSEL_2		(0x7FF)
#define COMBO_PHY_IOCTL_IOE				(0x26c)
#define DA_EMMC_E			(0x400)
#define DA_EMMC_IE			(0x7FF << 16)

#define COMBO_PHY_IOCTL_TO				(0x27c)
#define AD_EMMC_I			(0x7FF << 16)

/* USE COMBO PHY testchip */
#define TC_COMBO_PHY_REG_BASE		(0x21000)
#define TC_SC_APB_IF			(0x23000)
#define SC_EMMC_RSTEN		(TC_SC_APB_IF + 0x3C)
#define SC_EMMC_RSTDIS		(TC_SC_APB_IF + 0x40)
#define SC_EMMC_RSTSTAT	(TC_SC_APB_IF + 0x44)
#define IP_RST_PHY_APB		(0x1 << 4)


static void sdhci_i2c_writel(struct sdhci_host *host, u32 val, u32 addr);
static u32 sdhci_i2c_readl(struct sdhci_host *host, u32 addr);
static void sdhci_phy_writel(struct sdhci_host *host, u32 val, u32 reg)
{
	if (host->quirks2 & SDHCI_QUIRK2_HISI_COMBO_PHY_TC)
		sdhci_i2c_writel(host, val, TC_COMBO_PHY_REG_BASE + reg);
	else
		writel(val, host->mmc_phy + reg);
}

static u32 sdhci_phy_readl(struct sdhci_host *host, u32 reg)
{
	if (host->quirks2 & SDHCI_QUIRK2_HISI_COMBO_PHY_TC) {
		return sdhci_i2c_readl(host, TC_COMBO_PHY_REG_BASE + reg);
	} else {
		return readl(host->mmc_phy + reg);
	}
}

#define sdhci_mmc_sys_writel(host, val, reg) writel((val), (host)->mmc_sys + (reg))
#define sdhci_mmc_sys_readl(host, reg) readl((host)->mmc_sys + (reg))


#endif
