#ifndef _HISI_USB_HELPER_H_
#define _HISI_USB_HELPER_H_

#include <linux/usb/ch9.h>
#include <linux/hisi/usb/hisi_usb.h>
#include "dwc3-hisi.h"

const char *charger_type_string(enum hisi_charger_type type);
const char *hisi_usb_state_string(enum usb_state state);
enum hisi_charger_type get_charger_type_from_str(const char *buf, size_t size);
enum usb_device_speed usb_speed_to_string(const char *maximum_speed, size_t len);

#endif /* _HISI_USB_HELPER_H_ */
