/*
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 * Description: Support for eyetest of host mode
 * Create: 2016-03-28
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
#ifndef __LINUX_XHCI_DEBUGFS__H
#define __LINUX_XHCI_DEBUGFS__H

#include <linux/debugfs.h>

#include "xhci.h"

#define DEBUGFS_NAMELEN 32

#define REG_CAPLENGTH					0x00
#define REG_HCSPARAMS1					0x04
#define REG_HCSPARAMS2					0x08
#define REG_HCSPARAMS3					0x0c
#define REG_HCCPARAMS1					0x10
#define REG_DOORBELLOFF					0x14
#define REG_RUNTIMEOFF					0x18
#define REG_HCCPARAMS2					0x1c

#define	REG_USBCMD					0x00
#define REG_USBSTS					0x04
#define REG_PAGESIZE					0x08
#define REG_DNCTRL					0x14
#define REG_CRCR					0x18
#define REG_DCBAAP_LOW					0x30
#define REG_DCBAAP_HIGH					0x34
#define REG_CONFIG					0x38

#define REG_MFINDEX					0x00
#define REG_IR0_IMAN					0x20
#define REG_IR0_IMOD					0x24
#define REG_IR0_ERSTSZ					0x28
#define REG_IR0_ERSTBA_LOW				0x30
#define REG_IR0_ERSTBA_HIGH				0x34
#define REG_IR0_ERDP_LOW				0x38
#define REG_IR0_ERDP_HIGH				0x3c

#define REG_EXTCAP_USBLEGSUP				0x00
#define REG_EXTCAP_USBLEGCTLSTS				0x04

#define REG_EXTCAP_REVISION				0x00
#define REG_EXTCAP_NAME					0x04
#define REG_EXTCAP_PORTINFO				0x08
#define REG_EXTCAP_PORTTYPE				0x0c
#define REG_EXTCAP_MANTISSA1				0x10
#define REG_EXTCAP_MANTISSA2				0x14
#define REG_EXTCAP_MANTISSA3				0x18
#define REG_EXTCAP_MANTISSA4				0x1c
#define REG_EXTCAP_MANTISSA5				0x20
#define REG_EXTCAP_MANTISSA6				0x24

#define REG_EXTCAP_DBC_CAPABILITY			0x00
#define REG_EXTCAP_DBC_DOORBELL				0x04
#define REG_EXTCAP_DBC_ERSTSIZE				0x08
#define REG_EXTCAP_DBC_ERST_LOW				0x10
#define REG_EXTCAP_DBC_ERST_HIGH			0x14
#define REG_EXTCAP_DBC_ERDP_LOW				0x18
#define REG_EXTCAP_DBC_ERDP_HIGH			0x1c
#define REG_EXTCAP_DBC_CONTROL				0x20
#define REG_EXTCAP_DBC_STATUS				0x24
#define REG_EXTCAP_DBC_PORTSC				0x28
#define REG_EXTCAP_DBC_CONT_LOW				0x30
#define REG_EXTCAP_DBC_CONT_HIGH			0x34
#define REG_EXTCAP_DBC_DEVINFO1				0x38
#define REG_EXTCAP_DBC_DEVINFO2				0x3c

#define dump_register(nm)				\
{							\
	.name	= __stringify(nm),			\
	.offset	= REG_ ##nm,				\
}

struct xhci_regset {
	char			name[DEBUGFS_NAMELEN];
	struct debugfs_regset32	regset;
	size_t			nregs;
	struct dentry		*parent;
	struct list_head	list;
};

struct xhci_file_map {
	const char		*name;
	int			(*show)(struct seq_file *s, void *unused);
};

struct xhci_ep_priv {
	char			name[DEBUGFS_NAMELEN];
	struct dentry		*root;
};

struct xhci_slot_priv {
	char			name[DEBUGFS_NAMELEN];
	struct dentry		*root;
	struct xhci_ep_priv	*eps[31];
	struct xhci_virt_device	*dev;
};

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
