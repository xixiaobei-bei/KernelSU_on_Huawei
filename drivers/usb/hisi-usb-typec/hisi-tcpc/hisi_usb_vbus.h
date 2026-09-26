#ifndef _HISI_USB_VBUS_H_FOR_HISI_TCPC_
#define _HISI_USB_VBUS_H_FOR_HISI_TCPC_

int hisi_usb_vbus_status(void);
void hisi_usb_vbus_init(void *tcpc_data);
void hisi_usb_vbus_exit(void);
void hisi_tcpc_vbus_irq_handler(void *data, int vbus_status);

#endif
