#ifndef _HISI_USB_DEBUG_H_
#define _HISI_USB_DEBUG_H_

#include <linux/platform_device.h>
#include <linux/hisi/usb/hisi_usb.h>
#include "dwc3-hisi.h"

typedef ssize_t (*hiusb_debug_show_ops)(void *, char *, ssize_t);
typedef ssize_t (*hiusb_debug_store_ops)(void *, const char *, ssize_t);

#ifdef CONFIG_HISI_DEBUG_FS
struct usb3_hisi_debug_node {
	atomic_t hisi_dwc3_linkstate_flag;
	atomic_t hisi_dwc3_noc_flag;
	uint32_t usb_test_noc_addr;
	atomic_t hisi_dwc3_lbintpll_flag;
};

void usb_start_dump(void);
int hisi_dwc3_is_test_noc_addr(void);
uint32_t hisi_dwc3_get_noc_addr(uint32_t addr);
int hisi_dwc3_select_lbintpll_clk(void);
int create_attr_file(struct device *dev);
void remove_attr_file(struct device *dev);

#else
static inline void usb_start_dump(void)
{
	return ;
}

static inline int hisi_dwc3_is_test_noc_addr(void)
{
	return 0;
}

static inline uint32_t hisi_dwc3_get_noc_addr(uint32_t addr)
{
	return addr;
}

static inline int hisi_dwc3_select_lbintpll_clk(void)
{
	return 0;
}

static inline int create_attr_file(struct device *dev)
{
	return 0;
}
static inline void remove_attr_file(struct device *dev) {}
#endif
#endif /* _HISI_USB_DEBUG_H_ */
