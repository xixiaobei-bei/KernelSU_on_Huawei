 /*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Support for xhci debug notify
 * Author: Hisilicon
 * Create: 2020-08-31
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#ifndef __XHCI_DEBUG_EVENT_H
#define __XHCI_DEBUG_EVENT_H

#include "xhci.h"

#ifdef CONFIG_VENDOR_XHCI_EVENT_DEBUG
int xhci_host_register_eventnb(struct xhci_hcd *xhci);
void xhci_host_unregister_eventnb(struct xhci_hcd *xhci);
#else
static inline int xhci_host_register_eventnb(struct xhci_hcd *xhci){return 0;}
static inline void xhci_host_unregister_eventnb(struct xhci_hcd *xhci){return;}
#endif

#endif
