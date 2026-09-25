 /*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 * Description: Support for eyetest of host mode
 * Author: Hisilicon
 * Create: 2016-03-28
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
#ifndef __LINUX_XHCI_DEBUGFS__H
#define __LINUX_XHCI_DEBUGFS__H

#ifdef CONFIG_HISI_DEBUG_FS
int xhci_create_debug_file(struct xhci_hcd *xhci);
void xhci_remove_debug_file(struct xhci_hcd *xhci);
#else
static inline int xhci_create_debug_file(struct xhci_hcd *xhci)
{
	return 0;
}

static inline void xhci_remove_debug_file(struct xhci_hcd *xhci) { }
#endif

#endif /* __LINUX_XHCI_DEBUGFS__H */
