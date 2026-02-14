#ifndef _DWC3_HISI_USB3_31PHY__H
#define _DWC3_HISI_USB3_31PHY__H

#include <linux/hisi/usb/hisi_usb_interface.h>

struct hisi_usb_combophy {
	void (*reset_phy)(struct hisi_usb_combophy *combophy);
	void (*reset_misc_ctrl)(struct hisi_usb_combophy *combophy);
	void (*unreset_misc_ctrl)(struct hisi_usb_combophy *combophy);
	bool (*is_misc_ctrl_reset)(struct hisi_usb_combophy *combophy);
	bool (*is_misc_ctrl_unreset)(struct hisi_usb_combophy *combophy);
	bool (*is_misc_ctrl_clk_enable)(struct hisi_usb_combophy *combophy);
	void (*isodis)(struct hisi_usb_combophy *combophy);
	void (*exit_testpowerdown)(struct hisi_usb_combophy *combophy);
#ifdef COMBOPHY_FW_UPDATE
	void (*firmware_update_prepare)(struct hisi_usb_combophy *combophy);
	void (*firmware_update)(struct hisi_usb_combophy *combophy);
#endif
};

#define COMBOPHY_OPS_VOID(func) \
	static inline void combophy_##func(struct hisi_usb_combophy *combophy) \
	{ \
		if (combophy && combophy->func) \
			combophy->func(combophy); \
	}

#define COMBOPHY_OPS_BOOL(func) \
	static inline int combophy_##func(struct hisi_usb_combophy *combophy) \
	{ \
		if (combophy && combophy->func) \
			return combophy->func(combophy); \
		return false;\
	}

COMBOPHY_OPS_VOID(reset_phy);
COMBOPHY_OPS_VOID(reset_misc_ctrl);
COMBOPHY_OPS_VOID(unreset_misc_ctrl);
COMBOPHY_OPS_BOOL(is_misc_ctrl_reset);
COMBOPHY_OPS_BOOL(is_misc_ctrl_unreset);
COMBOPHY_OPS_BOOL(is_misc_ctrl_clk_enable);
COMBOPHY_OPS_VOID(isodis);
COMBOPHY_OPS_VOID(exit_testpowerdown);
#ifdef COMBOPHY_FW_UPDATE
COMBOPHY_OPS_VOID(firmware_update_prepare);
COMBOPHY_OPS_VOID(firmware_update);
#endif

struct hisi_usb_combophy *usb3_get_combophy_phandle(void);
int usb31phy_cr_write(u32 addr, u32 value);
u32 usb31phy_cr_read(u32 addr);

#endif /* _DWC3_HISI_USB3_31PHY__H */
