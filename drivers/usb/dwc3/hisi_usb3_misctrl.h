#ifndef _DWC3_HISI_COMMON__H
#define _DWC3_HISI_COMMON__H

#include <linux/hisi/usb/hisi_usb_interface.h>

#define COMBOPHY_MODE_MASK              0x3

#define PERI_CRG_PERRSTEN4		0x90
#define PERI_CRG_PERRSTDIS4		0x94
#define PERI_CRG_PERRSTSTAT4		0x98
#define IP_RST_USB3OTG_MISC		(1 << 7)
#define IP_RST_USB3OTG_32K		(1 << 6)

#define USB31PHY_CR_ACK			BIT(7)
#define USB31PHY_CR_WR_EN		BIT(5)
#define USB31PHY_CR_RD_EN		BIT(3)
#define USB31PHY_CR_SEL			BIT(4)
#define USB31PHY_CR_CLK			BIT(2)

#define USB31PHY_CR_ADDR_MASK		GENMASK(31, 16)
#define USB31PHY_CR_DATA_RD_START	16

#define USB_MISC_REG_PHY_CR_PARA_CTRL	0x54
#define USB_MISC_REG_PHY_CR_PARA_DATA	0x58
#define USB_MISC_REG_PHY_CR_PARA_STATUS   0x5C
#define USB31PHY_CR_PHY0_SRAM_INIT_DONE BIT(7)
#define USB31PHY_CR_PHY0_SRAM_EXT_LD_DONE BIT(3)
#define USB31PHY_CR_PHY0_SRAM_BYPASS BIT(2)

/* misc ctrl internal inteface */
void init_misc_ctrl_addr(void __iomem *base);
void init_sc_misc_ctrl_addr(void __iomem *base);

void usb3_misc_reg_writel(u32 val, unsigned long int offset);
u32 usb3_misc_reg_readl(unsigned long int offset);
void usb3_misc_reg_setbit(u32 bit, unsigned long int offset);
void usb3_misc_reg_clrbit(u32 bit, unsigned long int offset);
void usb3_misc_reg_setvalue(u32 val, unsigned long int offset);
void usb3_misc_reg_clrvalue(u32 val, unsigned long int offset);

u32 usb3_sc_misc_reg_readl(unsigned long int offset);
void usb3_sc_misc_reg_writel(u32 val, unsigned long int offset);
void usb3_sc_misc_reg_setbit(u32 bit, unsigned long int offset);
void usb3_sc_misc_reg_clrbit(u32 bit, unsigned long int offset);
void usb3_sc_misc_reg_setvalue(u32 val, unsigned long int offset);
void usb3_sc_misc_reg_clrvalue(u32 val, unsigned long int offset);

#endif /* _DWC3_HISI_COMMON__H */
