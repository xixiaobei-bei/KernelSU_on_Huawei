#define usbhub_debug(fmt, args...) printk(KERN_DEBUG "[usb_hub]" fmt, ## args)
#define usbhub_info(fmt, args...) printk(KERN_INFO "[usb_hub]" fmt, ## args)
#define usbhub_warn(fmt, args...) printk(KERN_WARNING "[usb_hub]" fmt, ## args)
#define usbhub_err(fmt, args...) printk(KERN_ERR "[usb_hub]" fmt, ## args)
