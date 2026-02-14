#ifndef _DWC3_HISI_USB3_31PHY__H
#define _DWC3_HISI_USB3_31PHY__H

#include <linux/hisi/usb/hisi_usb_interface.h>

#define COMBOPHY_MODE_MASK              0x3

void usb3_reset_misc_ctrl(void);
void usb3_unreset_misc_ctrl(void);

int dwc3_misc_ctrl_get(enum misc_ctrl_type type);
void dwc3_misc_ctrl_put(enum misc_ctrl_type type);
int usb31phy_cr_write(u32 addr, u16 value);
u16 usb31phy_cr_read(u32 addr);
void usb31phy_cr_32clk(void);

int dwc3_is_highspeed_only(void);
int dwc3_is_usb_only(void);

#endif /* _DWC3_HISI_USB3_31PHY__H */
