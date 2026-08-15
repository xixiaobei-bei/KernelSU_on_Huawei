#include <linux/kernel.h>
#include "hisi_usb_helper.h"

static const char *const hisi_usb_state_names[] = {
	[USB_STATE_UNKNOWN] = "USB_STATE_UNKNOWN",
	[USB_STATE_OFF] = "USB_STATE_OFF",
	[USB_STATE_DEVICE] = "USB_STATE_DEVICE",
	[USB_STATE_HOST] = "USB_STATE_HOST",
	[USB_STATE_HIFI_USB] = "USB_STATE_HIFI_USB",
	[USB_STATE_HIFI_USB_HIBERNATE] = "USB_STATE_HIFI_USB_HIBERNATE",
	[USB_STATE_AP_USE_HIFIUSB] = "USB_STATE_AP_USE_HIFIUSB",
};

const char *hisi_usb_state_string(enum usb_state state)
{
	if (state >= USB_STATE_ILLEGAL)
		return "illegal state";

	return hisi_usb_state_names[state];
}

static const char *charger_type_array[] = {
	[CHARGER_TYPE_SDP]     = "sdp",       /* Standard Downstreame Port */
	[CHARGER_TYPE_CDP]     = "cdp",       /* Charging Downstreame Port */
	[CHARGER_TYPE_DCP]     = "dcp",       /* Dedicate Charging Port */
	[CHARGER_TYPE_UNKNOWN] = "unknown",   /* non-standard */
	[CHARGER_TYPE_NONE]    = "none",      /* not connected */
	[PLEASE_PROVIDE_POWER] = "provide"   /* host mode, provide power */
};

const char *charger_type_string(enum hisi_charger_type type)
{
	if (type >= CHARGER_TYPE_ILLEGAL)
		return "illegal charger";

	return charger_type_array[type];
}

enum hisi_charger_type get_charger_type_from_str(const char *buf, size_t size)
{
	unsigned int i = 0;
	enum hisi_charger_type ret = CHARGER_TYPE_NONE;

	for (i = 0; i < sizeof(charger_type_array) / sizeof(charger_type_array[0]); i++) {/*lint !e574*/
		if (!strncmp(buf, charger_type_array[i], size - 1)) {
			ret = (enum hisi_charger_type)i;
			break;
		}
	}

	return ret;
}

static const char *const speed_names[] = {
	[USB_SPEED_UNKNOWN] = "UNKNOWN",
	[USB_SPEED_LOW] = "low-speed",
	[USB_SPEED_FULL] = "full-speed",
	[USB_SPEED_HIGH] = "high-speed",
	[USB_SPEED_WIRELESS] = "wireless",
	[USB_SPEED_SUPER] = "super-speed",
	[USB_SPEED_SUPER_PLUS] = "super-speed-plus",
};

enum usb_device_speed usb_speed_to_string(const char *maximum_speed, size_t len)
{
	enum usb_device_speed speed = USB_SPEED_SUPER;
	unsigned int i;
	size_t actual;

	if(maximum_speed == NULL || len == 0)
		return speed;

	for (i = 0; i < ARRAY_SIZE(speed_names); i++) {
		actual = strlen(speed_names[i]) < len ? strlen(speed_names[i]) : len;
		if (strncmp(maximum_speed, speed_names[i], actual) == 0) {
			speed = (enum usb_device_speed)i;
			break;
		}
	}

	return speed;
}

