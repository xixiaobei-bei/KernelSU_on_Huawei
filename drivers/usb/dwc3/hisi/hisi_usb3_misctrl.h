#ifndef _DWC3_HISI_COMMON__H
#define _DWC3_HISI_COMMON__H

#include <linux/hisi/usb/hisi_usb_interface.h>

#define USB_MISC_REG_LOGIC_ANALYZER_TRACE_L	0x28
#define USB_MISC_REG_LOGIC_ANALYZER_TRACE_H	0x2c

int dwc3_misc_ctrl_get(enum misc_ctrl_type type);
void dwc3_misc_ctrl_put(enum misc_ctrl_type type);

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
