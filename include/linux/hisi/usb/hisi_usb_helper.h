#ifndef _HISI_USB_HELPER_H_
#define _HISI_USB_HELPER_H_

#include <linux/usb/ch9.h>
#include <linux/clk.h>
#include <linux/hisi/usb/hisi_usb.h>

#define HISI_USB_GET_CLKS_MAX_NUM 10

const char *charger_type_string(enum hisi_charger_type type);
const char *hisi_usb_state_string(enum usb_state state);
enum hisi_charger_type get_charger_type_from_str(const char *buf, size_t size);
enum usb_device_speed usb_speed_to_string(const char *maximum_speed, size_t len);
void __iomem *of_devm_ioremap(struct device *dev, const char *compatible);
int hisi_usb_get_clks(struct device *dev, struct clk ***clks, int *num_clks);

#endif /* _HISI_USB_HELPER_H_ */
