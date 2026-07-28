#include "hisi_usb3_misctrl.h"
#include <soc_acpu_baseaddr_interface.h>
#include <soc_sctrl_interface.h>
#include <soc_crgperiph_interface.h>
#include <soc_pctrl_interface.h>
#define IS_SET_BIT(reg, n) ((reg) & (1ul << (n)))
#define IS_CLR_BIT(reg, n) (!IS_SET_BIT(reg, n))

#define PCTRL_PERI_CTRL3                0x10
#define USB_TCXO_EN			(1 << 1)
#define SCTRL_SCPERSTAT4		SOC_SCTRL_SCPERSTAT4_ADDR(SOC_ACPU_SCTRL_BASE_ADDR)
#define BIT_GT_CLK_USB2PHY_REF		(1 << 18)
#define SCTRL_SCCLKDIV5			SOC_SCTRL_SCCLKDIV5_ADDR(SOC_ACPU_SCTRL_BASE_ADDR)
#define SEL_USB2PHY_REF			(1 << 14)

#define IS_SET_BIT(reg, n) ((reg) & (1ul << (n)))
#define IS_CLR_BIT(reg, n) (!IS_SET_BIT(reg, n))

#define USBOTG3_CTRL0_OFFSET		0x00                    /* 选择控制寄存器0 */
#define USBOTG3_CTRL1_OFFSET		0x04                    /* 选择控制寄存器1 */
#define USBOTG3_CTRL2_OFFSET		0x08                    /* 选择控制寄存器2 */
#define USBOTG3_CTRL3_OFFSET		0x0C                    /* 选择控制寄存器3 */
#define USBOTG3_CTRL4_OFFSET		0x10                    /* 选择控制寄存器4 */
#define USBOTG3_CTRL5_OFFSET		0x14                    /* 选择控制寄存器5 */
#define USBOTG3_CTRL6_OFFSET		0x18                    /* 选择控制寄存器6 */
#define USBOTG3_CTRL7_OFFSET		0x1C                    /* 选择控制寄存器7 */
#define USBOTG3_STS0_OFFSET		0x20                    /* 状态寄存器0 */
#define USBOTG3_STS1_OFFSET		0x24                    /* 状态寄存器1 */
#define USBOTG3_STS2_OFFSET		0x28                    /* 状态寄存器2 */
#define USBOTG3_STS3_OFFSET		0x2C                    /* 状态寄存器3 */
#define BC_CTRL0_OFFSET			0x30                    /* BC控制器寄存器0 */
#define BC_CTRL1_OFFSET			0x34                    /* BC控制器寄存器1 */
#define BC_CTRL2_OFFSET			0x38                    /* BC控制器寄存器2 */
#define BC_STS0_OFFSET			0x3C                    /* BC状态寄存器0 */
#define RAM_CTRL_OFFSET			0x40                    /* RAM控制寄存器 */
#define USBOTG3_STS4_OFFSET		0x44                    /* 状态寄存器4 */
#define USB_MISC_CFG48_OFFSET		0x48
#define USB_MISC_CFG4C_OFFSET		0x4C
#define USB_MISC_CFG50_OFFSET		0x50
#define USB_MISC_CFG54_OFFSET		0x54
#define USB_MISC_CFG58_OFFSET		0x58
#define USB_MISC_CFG5C_OFFSET		0x5C
#define USB_MISC_CFG60_OFFSET		0x60
#define USB_MISC_CFG64_OFFSET		0x64
#define USB_MISC_CFG68_OFFSET		0x68
#define USB_MISC_CFG6C_OFFSET		0x6C
#define USB_MISC_CFG70_OFFSET		0x70
#define USB_MISC_CFG74_OFFSET		0x74
#define USB_MISC_CFG78_OFFSET		0x78
#define USB_MISC_CFG7C_OFFSET		0x7C
#define USB_MISC_CFG80_OFFSET		0x80
#define USB_MISC_CFG84_OFFSET		0x84
#define USB_MISC_CFG88_OFFSET		0x88
#define USB_MISC_CFG8C_OFFSET		0x8C
#define USB_MISC_CFG90_OFFSET		0x90
#define USB_MISC_CFG94_OFFSET		0x94
#define USB_MISC_CFG98_OFFSET		0x98
#define USB_MISC_CFG9C_OFFSET		0x9C
#define USB_MISC_CFGA0_OFFSET		0xA0
#define USB_MISC_CFGA4_OFFSET		0xA4
#define USB_MISC_CFGA8_OFFSET		0xA8

#define USB_MISC_CFGAC_OFFSET		0xAC
#define USB_MISC_CFGB0_OFFSET		0xB0
#define USB_MISC_CFGB4_OFFSET		0xB4
#define USB_MISC_CFGB8_OFFSET		0xB8
#define USB_MISC_CFGBC_OFFSET		0xBC
#define USB_MISC_CFGC0_OFFSET		0xC0
#define USB_MISC_CFGC4_OFFSET		0xC4
#define USB_MISC_CFGC8_OFFSET		0xC8
#define USB_MISC_CFGCC_OFFSET		0xCC

#define SC_USB20PHY_MISC_CTRL0_OFFSET		0x600	/* USB20PHY控制寄存器0 */
#define SC_USB20PHY_MISC_CTRL1_OFFSET		0x604   /* USB20PHY控制寄存器1 */
#define SC_USB20PHY_MISC_CTRL2_OFFSET		0x608   /* USB20PHY控制寄存器2 */
#define SC_USB20PHY_MISC_CTRL3_OFFSET		0x60C   /* USB20PHY控制寄存器3 */
#define SC_USB20PHY_MISC_CTRL4_OFFSET		0x610   /* USB20PHY控制寄存器4 */
#define SC_USB20PHY_MISC_CTRL5_OFFSET		0x614   /* USB20PHY控制寄存器5 */
#define SC_USB20PHY_MISC_CTRL6_OFFSET		0x618   /* USB20PHY控制寄存器6 */
#define SC_USB20PHY_MISC_CTRL7_OFFSET		0x61C   /* USB20PHY控制寄存器7 */
#define SC_USB20PHY_MISC_CTRL8_OFFSET		0x620   /* USB20PHY控制寄存器8 */
#define SC_USB20PHY_MISC_CTRL9_OFFSET		0x624   /* USB20PHY控制寄存器9 */
#define SC_USB20PHY_MISC_CTRL10_OFFSET		0x628   /* USB20PHY控制寄存器10 */
#define SC_USB20PHY_MISC_ST0_OFFSET		0x62C   /* USB20PHY状态寄存器0 */
#define SC_USB20PHY_MISC_ST1_OFFSET		0x630   /* USB20PHY状态寄存器1 */
#define SC_USB20PHY_MISC_ST2_OFFSET		0x634   /* USB20PHY状态寄存器2 */
#define SC_USB20PHY_RSV_RW0_OFFSET		0x638   /* USB20PHY保留寄存器0 */
#define SC_USB20PHY_RSV_RW1_OFFSET		0x63C   /* USB20PHY保留寄存器1 */
#define SC_USB20PHY_RSV_RW2_OFFSET		0x640   /* USB20PHY保留寄存器2 */
#define SC_USB20PHY_RSV_RO0_OFFSET		0x644   /* USB20PHY保留寄存器3 */
#define SC_USB20PHY_RSV_RO1_OFFSET		0x648   /* USB20PHY保留寄存器4 */
#define SC_USB20PHY_RSV_RO2_OFFSET		0x64C   /* USB20PHY保留寄存器5 */

#define TCA_CLK_RST_OFFSET			0x0000 /* TCA Block Clock and Reset Control Register */
#define TCA_INTR_EN_OFFSET			0x0004 /* TCA Block Interrupt Enable Register */
#define TCA_INTR_STS_OFFSET			0x0008 /* TCA Block Interrupt Status and Clear Register */
#define TCA_GCFG_OFFSET				0x0010 /* Global Configuration Register */
#define TCA_TCPC_OFFSET				0x0014 /* TCA TCPC Register emulating the [TCPC] DPAlt Switch Control Register. */
#define TCA_SYSMODE_CFG_OFFSET			0x0018 /* TCA Register for DPAlt_XBar Direct control by System. */
#define TCA_CTRLSYNCMODE_CFG0_OFFSET		0x0020 /* Configuration Register 0 for Controller Synced Mode */
#define TCA_CTRLSYNCMODE_CFG1_OFFSET		0x0024 /* Configuration Register 1 for Controller Synced Mode */
#define TCA_CTRLSYNCMODE_DBG0_OFFSET		0x0028 /* Debug Register 0 for Controller Synced Mode */
#define TCA_CTRLSYNCMODE_DBG1_OFFSET		0x002C /* Debug Register 1 for Controller Synced Mode */
#define TCA_PSTATE_OFFSET			0x0030 /* PSTATE status register for observing pstate/req/ack signals on various PHY lanes */
#define TCA_GEN_STATUS_OFFSET			0x0034 /* TCA General Status Register */
#define TCA_VBUS_CTRL_OFFSET			0x0040 /* TCA VBus Assist Control Register */
#define TCA_VBUS_STATUS_OFFSET			0x0044 /* TCA VBus Assist Status Register */

#define PRINT_A_REG(_name) usb_dbg ("%s(0x%x) : 0x%x\n",	\
	#_name, _name, readl((base) + (_name)))


static void check_usb_clk(void)
{
	void __iomem *sctrl_base = hisi_dwc3_dev->sctrl_reg_base;/*lint !e438 */
	void __iomem *pericfg_base = hisi_dwc3_dev->pericfg_reg_base;/*lint !e438 */
	void __iomem *pctrl_base = hisi_dwc3_dev->pctrl_reg_base;/*lint !e438 */
	volatile uint32_t reg;

	if (is_abb_clk_selected(sctrl_base)) {
		/* check refclk iso */
		reg = readl(PERI_CRG_ISODIS + pericfg_base);
		if(!(reg & USB_REFCLK_ISO_EN)) {
			usb_err("[USB.DBG] check refclk fail!\n");
		}

		/* check usb_tcxo_en */
		reg = readl(PCTRL_PERI_CTRL3 + pctrl_base);
		if(!(reg & USB_TCXO_EN)) {
			usb_err("[USB.DBG] check usb tcxo en fail!\n");
		}

		/* check if select usbphy clk from abb */
		reg = readl(PCTRL_PERI_CTRL24 + pctrl_base);
		if (reg & SC_CLK_USB3PHY_3MUX1_SEL) {
			usb_err("[USB.DBG] check usbphy from abb fail!\n");
		}
	} else {
		usb_err("[USB.DBG] check use abb clk fail\n");
	}

	/* check usb2 clk */
	reg = readl(SCTRL_SCPERSTAT4 + sctrl_base);
	if(!(reg & BIT_GT_CLK_USB2PHY_REF)) {
		usb_err("[USB.DBG] check choose usb2 clk fail\n");
	}

	/* check if choose 19.2 MHz for usb2 clk */
	reg = readl(SCTRL_SCCLKDIV5 + sctrl_base);
	if (reg & SEL_USB2PHY_REF) {
		usb_err("[USB.DBG] check choose 19.2 MHz fail\n");
	}

}

static void check_usb_phy(void)
{
	volatile uint32_t reg;

	/* check USB20 PHY out of IDDQ */
	reg = usb3_sc_misc_reg_readl(SC_USB20PHY_MISC_CTRL2_OFFSET);
	if (IS_SET_BIT(reg, 0)) {
		usb_err("[USB.DBG] check usb2 phy out of IDDQ fail!\n");
	}

	/* check USB31 PHY out of  TestPowerDown */
	reg = usb3_misc_reg_readl(0x50); /* reg name: USB_MISC_CFG50 */
	if (IS_SET_BIT(reg, 23)) {
		usb_err("[USB.DBG] check usb3 phy TestPowerDown fail!\n");
	}

	/* Tell the PHY power is stable */
	reg = usb3_misc_reg_readl(0x54);
	if (!(reg & ((1u << 1) | (1u << 9) | (1u << 11)))) {
		usb_err("[USB.DBG] check phy power fail!\n");
	}

	/*
	 * config the TCA mux mode
	 */

	reg = usb3_misc_reg_readl(0x208);
	if (0xffff != reg) {
		usb_err("[USB.DBG] check TCA 0x208 fail!\n");
	}

	reg = usb3_misc_reg_readl(0x204);
	if (0x3 != reg) {
		usb_err("[USB.DBG] check TCA CONFIG 0x204 fail!\n");
	}

	reg = usb3_misc_reg_readl(0x200);
	if (reg & SET_NBITS_MASK(0, 1)) {
		usb_err("[USB.DBG] check TCA CONFIG 0x200 fail!\n");
	}

	reg = usb3_misc_reg_readl(0x210);
	if (IS_CLR_BIT(reg, 4)) {
		usb_err("[USB.DBG] check TCA CONFIG 0x210 fail!\n");
	}

	reg = usb3_misc_reg_readl(0x214);
	reg &= (SET_NBITS_MASK(0, 1) | SET_NBITS_MASK(3, 4));
	if (reg != (0x1 | (0x2 << 3))) {
		usb_err("[USB.DBG] check TCA CONFIG 0x214 fail!\n");
	}

	reg = usb3_misc_reg_readl(0x240);
	if (reg != (0x3 | (0x3 << 2))) {
		usb_err("[USB.DBG] check TCA CONFIG 0x240 fail!\n");
	}

	/* check if SSC enable */
	reg = usb3_misc_reg_readl(0x5c);
	if (IS_CLR_BIT(reg, 1)) {
		usb_err("[USB.DBG] check ssc enable fail!\n");
	}
}

static void check_combophy_firmware(void)
{
	int i;
	extern short unsigned int firmware[6000];
	int fw_size = sizeof(firmware)/sizeof(firmware[0]);
	volatile u32 reg;

	if (0x74cd != usb31phy_cr_read(0)) {
		usb_err("[USB.DBG] check crphy read timeout!\n");
		return ;
	}


	reg = usb3_misc_reg_readl(USB_MISC_REG_PHY_CR_PARA_STATUS);
	if(!(reg & USB31PHY_CR_PHY0_SRAM_INIT_DONE)) {
		usb_err("[USB.DBG] check crphy sram init done fail!\n");
	}

	for(i = 0; i< fw_size; i++) {
		reg = usb31phy_cr_read(0xc000 + i);
		if (reg != firmware[i]) {
			usb_err("[USB.DBG] check firmware data[%d:0x%x --> 0x%x] fail!\n", i, firmware[i], reg);
			break;
		}
	}

	reg = usb31phy_cr_read(0x42);
	reg &= 0x3f;
	if (reg == ((1 << 1) | (1 << 3) | (1 << 5))) {
		usb_err("[USB.DBG] check cr para 0x42 fail!\n");
	}

	reg = usb31phy_cr_read(0x11f4);
	reg &= (0x3 << 3);
	if (reg == (1 << 3)) {
		usb_err("[USB.DBG] check cr para 0x11f4 fail!\n");
	}

	reg = usb31phy_cr_read(0x12f4);
	reg &= (0x3 << 3);
	if (reg == (1 << 3)) {
		usb_err("[USB.DBG] check cr para 0x12f4 fail!\n");
	}

	reg = usb3_misc_reg_readl(0xb4);
	if (IS_CLR_BIT(reg, 7)) {
		usb_err("[USB.DBG] misc reg check 0xb4 fail!\n");
	}

	reg = usb31phy_cr_read(0x422d);
	if (IS_CLR_BIT(reg, 2)) {
		usb_err("[USB.DBG] check cr para 0x422d fail!\n");
	}

	reg = usb31phy_cr_read(0x412d);
	if (IS_CLR_BIT(reg, 2)) {
		usb_err("[USB.DBG] check cr para 0x412d fail!\n");
	}

	reg = usb3_misc_reg_readl(USB_MISC_REG_PHY_CR_PARA_STATUS);
	if (!(reg & USB31PHY_CR_PHY0_SRAM_EXT_LD_DONE)) {
		usb_err("[USB.DBG] check cr phy ext ld done fail!\n");
	}
}

void usb3_check_usb_clk_init(void)
{
	void __iomem *sctrl_base = hisi_dwc3_dev->sctrl_reg_base;/*lint !e438 */
	void __iomem *pericfg_base = hisi_dwc3_dev->pericfg_reg_base;/*lint !e438 */
	volatile uint32_t scdeepsleeped;
	volatile uint32_t reg;

	/* check phy power */
	scdeepsleeped = readl(SCTRL_SCDEEPSLEEPED + sctrl_base);
	if ((scdeepsleeped & (1 << 7)) != 0) {
		reg = usb3_sc_misc_reg_readl(SC_USB20PHY_MISC_CTRL3_OFFSET);
		if (!IS_SET_BIT(reg, 12)) {
			usb_err("[USB.DBG] check phy power fail!\n");
		}
	}

	/* check at reset status */
	reg = readl(PERI_CRG_PERRSTSTAT4 + pericfg_base);
	if (!(reg & (IP_RST_USB3OTG_MISC | IP_RST_USB3OTG_32K))) {
		usb_dbg("[USB.DBG] check reset pass!\n");
	}

	/* check hclk gate */
	reg = readl(PERI_CRG_PERSTAT0 + pericfg_base);
	if (!(reg & GT_HCLK_USB3OTG_MISC)) {
		usb_err("[USB.DBG] check hclk gate fail!\n");
	}

	/* check misc clk gate */
	reg = readl(PERI_CRG_PERSTAT4 + pericfg_base);
	if (!(reg & (GT_CLK_USB3OTG_REF | GT_ACLK_USB3OTG))) {
		usb_err("[USB.DBG] check misc clk gate fail!\n");
	}

	/* check dis-reset usb misc ctrl */
	reg = readl(PERI_CRG_PERRSTSTAT4 + pericfg_base);
	if (reg & (IP_RST_USB3OTG_MISC | IP_RST_USB3OTG_32K)) {
		usb_err("[USB.DBG] check misc-ctrl release fail!\n");
	}

	check_usb_clk();

	check_usb_phy();

	reg = usb3_sc_misc_reg_readl(SC_USB20PHY_MISC_CTRL0_OFFSET);
	if (SET_NBITS_MASK(8, 9) != (reg & SET_NBITS_MASK(8, 9))) {
		usb_err("[USB.DBG] check utmi width-16 fail!\n");
	}

	reg = usb3_misc_reg_readl(USB_MISC_REG_PHY_CR_PARA_STATUS);
	if (reg & USB31PHY_CR_PHY0_SRAM_BYPASS) {
		usb_err("[USB.DBG] check CR PHY0 SRAM BYPASS fail!\n");
	}

	/* check unreset phy */
	reg = usb3_misc_reg_readl(0xa0);
	if (IS_CLR_BIT(reg, 1)) {
		usb_err("[USB.DBG] check phy release fail!\n");
	}

	reg = usb3_sc_misc_reg_readl(SC_USB20PHY_MISC_CTRL2_OFFSET);
	if (IS_CLR_BIT(reg, 0)) {
		usb_err("[USB.DBG] check phy2.0 release fail!\n");
	}

	check_combophy_firmware();

	/* check controller release */
	reg = usb3_misc_reg_readl(0xa0);
	if (IS_CLR_BIT(reg, 8)) {
		usb_err("[USB.DBG] check controller release fail!\n");
	}

	/* check vbus valid */
	reg = usb3_misc_reg_readl(0x0);
	reg &= SET_NBITS_MASK(6, 7);
	if (SET_NBITS_MASK(6, 7) != reg) {
		usb_err("[USB.DBG] check misc reg 0x0 fail!\n");
	}

	reg = usb3_sc_misc_reg_readl(SC_USB20PHY_MISC_CTRL0_OFFSET);
	reg &= SET_NBITS_MASK(5, 6);
	if (SET_NBITS_MASK(5, 6) != reg) {
		usb_err("[USB.DBG] check misc reg 0x600 fail!\n");
	}
}

extern struct hisi_dwc3_device *hisi_dwc3_dev;

int dump_other_clk_rst_regs(void)
{
	void *base;

	if (!hisi_dwc3_dev) {
		usb_err("[USB.DBG] usb driver not init!\n");
		return -EPERM;
	}

	usb_dbg ("%s:\n", __func__);
	usb_dbg ("----------------------------------------------------------------\n");

	base = hisi_dwc3_dev->pericfg_reg_base;
	// misc clk
	PRINT_A_REG(SOC_CRGPERIPH_PERSTAT0_ADDR(0));
	PRINT_A_REG(SOC_CRGPERIPH_PERSTAT4_ADDR(0));

	// misc reset
	PRINT_A_REG(SOC_CRGPERIPH_PERRSTSTAT4_ADDR(0));

	// combo phy clk
	PRINT_A_REG(SOC_CRGPERIPH_ISOSTAT_ADDR(0));

	base = (void *)0xE8A09000;
	PRINT_A_REG(SOC_PCTRL_PERI_CTRL3_ADDR(0)); // pre_usb_tcxo_en
	PRINT_A_REG(SOC_PCTRL_PERI_CTRL24_ADDR(0)); // sc_clk_usb3phy_3mux1_sel

	base = (void *)0xFFF0A000;
	PRINT_A_REG(SOC_SCTRL_SCDEEPSLEEPED_ADDR(0)); // efuse
	//PRINT_A_REG(SOC_SCTRL_SCPERCTRL8_ADDR(0)); // 0x8a8

	PRINT_A_REG(SOC_SCTRL_SCPERSTAT4_ADDR(0)); // gt_clk_usb2phy_ref
	PRINT_A_REG(SOC_SCTRL_SCCLKDIV5_ADDR(0)); // sel_usb2phy_ref
	usb_dbg ("----------------------------------------------------------------\n");

	return 0;
}


int dump_all_misc_ctrl_regs(int detail)
{
	void *base;

	if (!hisi_dwc3_dev) {
		usb_dbg ("[USB.DBG] usb driver not init!\n");
		return -EPERM;
	}
	usb_dbg ("%s:\n", __func__);
	usb_dbg ("----------------------------------------------------------------\n");

	base = hisi_dwc3_dev->otg_bc_reg_base;

	PRINT_A_REG(USBOTG3_CTRL0_OFFSET);
	PRINT_A_REG(USBOTG3_CTRL1_OFFSET);
	PRINT_A_REG(USBOTG3_CTRL2_OFFSET);
	PRINT_A_REG(USBOTG3_CTRL3_OFFSET);
	PRINT_A_REG(USBOTG3_CTRL4_OFFSET);
	PRINT_A_REG(USBOTG3_CTRL5_OFFSET);
	PRINT_A_REG(USBOTG3_CTRL6_OFFSET);
	PRINT_A_REG(USBOTG3_CTRL7_OFFSET);
	PRINT_A_REG(USBOTG3_STS0_OFFSET);
	PRINT_A_REG(USBOTG3_STS1_OFFSET);
	PRINT_A_REG(USBOTG3_STS2_OFFSET);
	PRINT_A_REG(USBOTG3_STS3_OFFSET);
	PRINT_A_REG(BC_CTRL0_OFFSET);
	PRINT_A_REG(BC_CTRL1_OFFSET);
	PRINT_A_REG(BC_CTRL2_OFFSET);
	PRINT_A_REG(BC_STS0_OFFSET);
	PRINT_A_REG(RAM_CTRL_OFFSET);
	PRINT_A_REG(USBOTG3_STS4_OFFSET);
	PRINT_A_REG(USB_MISC_CFG48_OFFSET);
	PRINT_A_REG(USB_MISC_CFG4C_OFFSET);
	PRINT_A_REG(USB_MISC_CFG50_OFFSET);
	PRINT_A_REG(USB_MISC_CFG54_OFFSET);
	PRINT_A_REG(USB_MISC_CFG58_OFFSET);
	PRINT_A_REG(USB_MISC_CFG5C_OFFSET);
	PRINT_A_REG(USB_MISC_CFG60_OFFSET);
	PRINT_A_REG(USB_MISC_CFG64_OFFSET);
	PRINT_A_REG(USB_MISC_CFG68_OFFSET);
	PRINT_A_REG(USB_MISC_CFG6C_OFFSET);
	PRINT_A_REG(USB_MISC_CFG70_OFFSET);
	PRINT_A_REG(USB_MISC_CFG74_OFFSET);
	PRINT_A_REG(USB_MISC_CFG78_OFFSET);
	PRINT_A_REG(USB_MISC_CFG7C_OFFSET);
	PRINT_A_REG(USB_MISC_CFG80_OFFSET);
	PRINT_A_REG(USB_MISC_CFG84_OFFSET);
	PRINT_A_REG(USB_MISC_CFG88_OFFSET);
	PRINT_A_REG(USB_MISC_CFG8C_OFFSET);
	PRINT_A_REG(USB_MISC_CFG90_OFFSET);
	PRINT_A_REG(USB_MISC_CFG94_OFFSET);
	PRINT_A_REG(USB_MISC_CFG98_OFFSET);
	PRINT_A_REG(USB_MISC_CFG9C_OFFSET);
	PRINT_A_REG(USB_MISC_CFGA0_OFFSET);
	PRINT_A_REG(USB_MISC_CFGA4_OFFSET);
	PRINT_A_REG(USB_MISC_CFGA8_OFFSET);

	PRINT_A_REG(USB_MISC_CFGAC_OFFSET);
	PRINT_A_REG(USB_MISC_CFGB0_OFFSET);
	PRINT_A_REG(USB_MISC_CFGB4_OFFSET);
	PRINT_A_REG(USB_MISC_CFGB8_OFFSET);
	PRINT_A_REG(USB_MISC_CFGBC_OFFSET);
	PRINT_A_REG(USB_MISC_CFGC0_OFFSET);
	PRINT_A_REG(USB_MISC_CFGC4_OFFSET);
	PRINT_A_REG(USB_MISC_CFGC8_OFFSET);
	PRINT_A_REG(USB_MISC_CFGCC_OFFSET);

	if (detail) {
		PRINT_A_REG(SC_USB20PHY_MISC_CTRL0_OFFSET);
		PRINT_A_REG(SC_USB20PHY_MISC_CTRL1_OFFSET);
		PRINT_A_REG(SC_USB20PHY_MISC_CTRL2_OFFSET);
		PRINT_A_REG(SC_USB20PHY_MISC_CTRL3_OFFSET);
		PRINT_A_REG(SC_USB20PHY_MISC_CTRL4_OFFSET);
		PRINT_A_REG(SC_USB20PHY_MISC_CTRL5_OFFSET);
		PRINT_A_REG(SC_USB20PHY_MISC_CTRL6_OFFSET);
		PRINT_A_REG(SC_USB20PHY_MISC_CTRL7_OFFSET);
		PRINT_A_REG(SC_USB20PHY_MISC_CTRL8_OFFSET);
		PRINT_A_REG(SC_USB20PHY_MISC_CTRL9_OFFSET);
		PRINT_A_REG(SC_USB20PHY_MISC_CTRL10_OFFSET);
		PRINT_A_REG(SC_USB20PHY_MISC_ST0_OFFSET);
		PRINT_A_REG(SC_USB20PHY_MISC_ST1_OFFSET);
		PRINT_A_REG(SC_USB20PHY_MISC_ST2_OFFSET);
		PRINT_A_REG(SC_USB20PHY_RSV_RW0_OFFSET);
		PRINT_A_REG(SC_USB20PHY_RSV_RW1_OFFSET);
		PRINT_A_REG(SC_USB20PHY_RSV_RW2_OFFSET);
		PRINT_A_REG(SC_USB20PHY_RSV_RO0_OFFSET);
		PRINT_A_REG(SC_USB20PHY_RSV_RO1_OFFSET);
		PRINT_A_REG(SC_USB20PHY_RSV_RO2_OFFSET);
	}
	usb_dbg ("----------------------------------------------------------------\n");
	return 0;
}

int dump_all_tca_regs(void)
{
	void *base;

	if (!hisi_dwc3_dev) {
		usb_err("[USB.DBG] usb driver not init!\n");
		return -EPERM;
	}
	usb_dbg ("%s:\n", __func__);
	usb_dbg ("----------------------------------------------------------------\n");

	base = hisi_dwc3_dev->otg_bc_reg_base;

	PRINT_A_REG(0x200 + TCA_CLK_RST_OFFSET);
	PRINT_A_REG(0x200 + TCA_INTR_EN_OFFSET);
	PRINT_A_REG(0x200 + TCA_INTR_STS_OFFSET);
	PRINT_A_REG(0x200 + TCA_GCFG_OFFSET);
	PRINT_A_REG(0x200 + TCA_TCPC_OFFSET);
	PRINT_A_REG(0x200 + TCA_SYSMODE_CFG_OFFSET);
	PRINT_A_REG(0x200 + TCA_CTRLSYNCMODE_CFG0_OFFSET);
	PRINT_A_REG(0x200 + TCA_CTRLSYNCMODE_CFG1_OFFSET);
	PRINT_A_REG(0x200 + TCA_CTRLSYNCMODE_DBG0_OFFSET);
	PRINT_A_REG(0x200 + TCA_CTRLSYNCMODE_DBG1_OFFSET);
	PRINT_A_REG(0x200 + TCA_PSTATE_OFFSET);
	PRINT_A_REG(0x200 + TCA_GEN_STATUS_OFFSET);
	PRINT_A_REG(0x200 + TCA_VBUS_CTRL_OFFSET);
	PRINT_A_REG(0x200 + TCA_VBUS_STATUS_OFFSET);
	usb_dbg ("----------------------------------------------------------------\n");
	return 0;
}
