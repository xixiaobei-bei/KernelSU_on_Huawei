#ifndef _HISI_USB_DEBUG_H_
#define _HISI_USB_DEBUG_H_

#include <linux/kernel.h>
#include <linux/device.h>


typedef ssize_t (*hiusb_debug_show_ops)(void *, char *, size_t);
typedef ssize_t (*hiusb_debug_store_ops)(void *, const char *, size_t);

#ifdef CONFIG_HISI_DEBUG_FS
int create_attr_file(struct device *dev);
void remove_attr_file(struct device *dev);
#else
static inline int create_attr_file(struct device *dev){return 0;}
static inline void remove_attr_file(struct device *dev) {}
#endif
#endif /* _HISI_USB_DEBUG_H_ */
