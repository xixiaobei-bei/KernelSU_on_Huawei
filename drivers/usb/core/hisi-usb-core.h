#ifndef _HISI_USB_CORE_H_
#define _HISI_USB_CORE_H_

struct usb_device;
struct usb_hub;

#define USB_DEVICE_READ_TRY_LOCK

int usb_device_read_mutex_trylock(void);
int usb_device_read_usb_trylock_device(struct usb_device *udev);
bool check_huawei_dock_quirk(struct usb_device *hdev,
		struct usb_hub *hub, int port1);

#endif /* _HISI_USB_CORE_H_ */
