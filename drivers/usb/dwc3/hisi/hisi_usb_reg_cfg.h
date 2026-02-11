#ifndef __HISI_USB_REG_CFG_H__
#define __HISI_USB_REG_CFG_H__

#include <linux/regmap.h>
#include <linux/mfd/syscon.h>

enum reg_cfg_type {
	WRITE_ONLY,
	BIT_MASK,
	READ_WRITE,
	TEST_READ,
	REG_cfg_TYPE_MAX
};

struct hisi_usb_reg_cfg {
	struct regmap *regmap;
	enum reg_cfg_type cfg_type;
	unsigned int offset;
	unsigned int value;
	unsigned int mask;
};

extern struct hisi_usb_reg_cfg *of_get_hisi_usb_reg_cfg_by_name(
		struct device_node *np,
		const char *prop_name);
extern void of_remove_hisi_usb_reg_cfg(struct hisi_usb_reg_cfg *reg_cfg);
extern int hisi_usb_reg_write(struct hisi_usb_reg_cfg *reg_cfg);
extern int hisi_usb_reg_test_cfg(struct hisi_usb_reg_cfg *reg_cfg);

#endif /* __HISI_USB_REG_CFG_H__ */
