/*
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 * Description: Support for eyetest of host mode
 * Create: 2016-02-25
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#include "xhci-debugfs.h"

#include <linux/device.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/usb.h>

#include "xhci.h"

static struct list_head g_regset_list;

static const struct debugfs_reg32 xhci_cap_regs[] = {
	dump_register(CAPLENGTH),
	dump_register(HCSPARAMS1),
	dump_register(HCSPARAMS2),
	dump_register(HCSPARAMS3),
	dump_register(HCCPARAMS1),
	dump_register(DOORBELLOFF),
	dump_register(RUNTIMEOFF),
	dump_register(HCCPARAMS2),
};

static const struct debugfs_reg32 xhci_op_regs[] = {
	dump_register(USBCMD),
	dump_register(USBSTS),
	dump_register(PAGESIZE),
	dump_register(DNCTRL),
	dump_register(CRCR),
	dump_register(DCBAAP_LOW),
	dump_register(DCBAAP_HIGH),
	dump_register(CONFIG),
};

static const struct debugfs_reg32 xhci_runtime_regs[] = {
	dump_register(MFINDEX),
	dump_register(IR0_IMAN),
	dump_register(IR0_IMOD),
	dump_register(IR0_ERSTSZ),
	dump_register(IR0_ERSTBA_LOW),
	dump_register(IR0_ERSTBA_HIGH),
	dump_register(IR0_ERDP_LOW),
	dump_register(IR0_ERDP_HIGH),
};

static const struct debugfs_reg32 xhci_extcap_legsup[] = {
	dump_register(EXTCAP_USBLEGSUP),
	dump_register(EXTCAP_USBLEGCTLSTS),
};

static const struct debugfs_reg32 xhci_extcap_protocol[] = {
	dump_register(EXTCAP_REVISION),
	dump_register(EXTCAP_NAME),
	dump_register(EXTCAP_PORTINFO),
	dump_register(EXTCAP_PORTTYPE),
	dump_register(EXTCAP_MANTISSA1),
	dump_register(EXTCAP_MANTISSA2),
	dump_register(EXTCAP_MANTISSA3),
	dump_register(EXTCAP_MANTISSA4),
	dump_register(EXTCAP_MANTISSA5),
	dump_register(EXTCAP_MANTISSA6),
};

static const struct debugfs_reg32 xhci_extcap_dbc[] = {
	dump_register(EXTCAP_DBC_CAPABILITY),
	dump_register(EXTCAP_DBC_DOORBELL),
	dump_register(EXTCAP_DBC_ERSTSIZE),
	dump_register(EXTCAP_DBC_ERST_LOW),
	dump_register(EXTCAP_DBC_ERST_HIGH),
	dump_register(EXTCAP_DBC_ERDP_LOW),
	dump_register(EXTCAP_DBC_ERDP_HIGH),
	dump_register(EXTCAP_DBC_CONTROL),
	dump_register(EXTCAP_DBC_STATUS),
	dump_register(EXTCAP_DBC_PORTSC),
	dump_register(EXTCAP_DBC_CONT_LOW),
	dump_register(EXTCAP_DBC_CONT_HIGH),
	dump_register(EXTCAP_DBC_DEVINFO1),
	dump_register(EXTCAP_DBC_DEVINFO2),
};

static struct xhci_regset *xhci_debugfs_alloc_regset(struct xhci_hcd *xhci)
{
	struct xhci_regset	*regset = NULL;

	regset = kzalloc(sizeof(*regset), GFP_KERNEL);
	if (!regset)
		return NULL;

	/*
	 * The allocation and free of regset are executed in order.
	 * We needn't a lock here.
	 */
	INIT_LIST_HEAD(&regset->list);
	list_add_tail(&regset->list, &g_regset_list);

	return regset;
}

static void xhci_debugfs_free_regset(struct xhci_regset *regset)
{
	if (!regset)
		return;

	list_del(&regset->list);
	kfree(regset);
}

static void xhci_debugfs_regset(struct xhci_hcd *xhci, u32 base,
				const struct debugfs_reg32 *regs,
				size_t nregs, struct dentry *parent,
				const char *fmt, ...)
{
	struct xhci_regset	*rgs = NULL;
	va_list			args;
	struct debugfs_regset32	*regset = NULL;
	struct usb_hcd		*hcd = xhci_to_hcd(xhci);

	rgs = xhci_debugfs_alloc_regset(xhci);
	if (!rgs)
		return;

	va_start(args, fmt);
	vsnprintf(rgs->name, sizeof(rgs->name), fmt, args);
	va_end(args);

	regset = &rgs->regset;
	regset->regs = regs;
	regset->nregs = nregs;
	regset->base = hcd->regs + base;

	debugfs_create_regset32((const char *)rgs->name, 0444, parent, regset);
}

static void xhci_debugfs_extcap_regset(struct xhci_hcd *xhci, int cap_id,
				       const struct debugfs_reg32 *regs,
				       size_t n, const char *cap_name)
{
	u32			offset;
	int			index = 0;
	size_t			psic;
	size_t			nregs = n;
	void __iomem		*base = &xhci->cap_regs->hc_capbase;

	offset = xhci_find_next_ext_cap(base, 0, cap_id);
	while (offset) {
		if (cap_id == XHCI_EXT_CAPS_PROTOCOL) {
			/* get ext port and regs value */
			psic = XHCI_EXT_PORT_PSIC(readl(base + offset + 8));
			nregs = min(4 + psic, n);
		}

		xhci_debugfs_regset(xhci, offset, regs, nregs,
				    xhci->debugfs_root, "%s:%02d",
				    cap_name, index);
		offset = xhci_find_next_ext_cap(base, offset, cap_id);
		index++;
	}
}

static int xhci_ring_enqueue_show(struct seq_file *s, void *unused)
{
	dma_addr_t		dma;
	struct xhci_ring	*ring = *(struct xhci_ring **)s->private;

	dma = xhci_trb_virt_to_dma(ring->enq_seg, ring->enqueue);
	seq_printf(s, "%pad\n", &dma);

	return 0;
}

static int xhci_ring_dequeue_show(struct seq_file *s, void *unused)
{
	dma_addr_t		dma;
	struct xhci_ring	*ring = *(struct xhci_ring **)s->private;

	dma = xhci_trb_virt_to_dma(ring->deq_seg, ring->dequeue);
	seq_printf(s, "%pad\n", &dma);

	return 0;
}

static int xhci_ring_cycle_show(struct seq_file *s, void *unused)
{
	struct xhci_ring	*ring = *(struct xhci_ring **)s->private;

	seq_printf(s, "%d\n", ring->cycle_state);

	return 0;
}

static void xhci_ring_dump_segment(struct seq_file *s,
				   struct xhci_segment *seg)
{
	int			i;
	dma_addr_t		dma;
	union xhci_trb		*trb = NULL;

	for (i = 0; i < TRBS_PER_SEGMENT; i++) {
		trb = &seg->trbs[i];
		dma = seg->dma + i * sizeof(*trb);
		/* trb field decode */
		seq_printf(s, "%pad: %s\n", &dma,
			   xhci_decode_trb(trb->generic.field[0],
					   trb->generic.field[1],
					   trb->generic.field[2],
					   trb->generic.field[3]));
	}
}

static int xhci_ring_trb_show(struct seq_file *s, void *unused)
{
	unsigned int			i;
	struct xhci_ring	*ring = *(struct xhci_ring **)s->private;
	struct xhci_segment	*seg = ring->first_seg;

	for (i = 0; i < ring->num_segs; i++) {
		xhci_ring_dump_segment(s, seg);
		seg = seg->next;
	}

	return 0;
}


static struct xhci_file_map ring_files[] = {
	{"enqueue",		xhci_ring_enqueue_show, },
	{"dequeue",		xhci_ring_dequeue_show, },
	{"cycle",		xhci_ring_cycle_show, },
	{"trbs",		xhci_ring_trb_show, },
};

static int xhci_ring_open(struct inode *inode, struct file *file)
{
	unsigned int			i;
	struct xhci_file_map	*f_map = NULL;
	const char		*file_name = file_dentry(file)->d_iname;

	for (i = 0; i < ARRAY_SIZE(ring_files); i++) {
		f_map = &ring_files[i];

		if (strcmp(f_map->name, file_name) == 0)
			break;
	}

	return single_open(file, f_map->show, inode->i_private);
}

static const struct file_operations xhci_ring_fops = {
	.open			= xhci_ring_open,
	.read			= seq_read,
	.llseek			= seq_lseek,
	.release		= single_release,
};

static void xhci_debugfs_create_files(struct xhci_hcd *xhci,
				      struct xhci_file_map *files,
				      size_t nentries, void *data,
				      struct dentry *parent,
				      const struct file_operations *fops)
{
	unsigned int			i;

	for (i = 0; i < nentries; i++)
		/* create file and give permission */
		debugfs_create_file(files[i].name, 0444, parent, data, fops);
}

static struct dentry *xhci_debugfs_create_ring_dir(struct xhci_hcd *xhci,
						   struct xhci_ring **ring,
						   const char *name,
						   struct dentry *parent)
{
	struct dentry		*dir = NULL;

	dir = debugfs_create_dir(name, parent);
	xhci_debugfs_create_files(xhci, ring_files, ARRAY_SIZE(ring_files),
				  ring, dir, &xhci_ring_fops);

	return dir;
}

static int xhci_portsc_show(struct seq_file *s, void *unused)
{
	__le32 __iomem 		*addr = s->private;
	u32			portsc;

	portsc = readl(addr);
	seq_printf(s, "%s\n", xhci_decode_portsc(portsc));

	return 0;
}
static int xhci_port_open(struct inode *inode, struct file *file)
{
	return single_open(file, xhci_portsc_show, inode->i_private);
}

static const struct file_operations port_fops = {
	.open			= xhci_port_open,
	.read			= seq_read,
	.llseek			= seq_lseek,
	.release		= single_release,
};

static void xhci_debugfs_create_ports(struct xhci_hcd *xhci,
				      struct dentry *parent)
{
	unsigned int		num_ports;
	char			port_name[10];
	struct dentry		*dir = NULL;
	unsigned int		u2_port_nums;
	unsigned int		u3_port_nums;

	u2_port_nums = xhci->num_usb2_ports;
	u3_port_nums = xhci->num_usb3_ports;
	num_ports = HCS_MAX_PORTS(xhci->hcs_params1);
	if (num_ports != u2_port_nums + u3_port_nums) {
		pr_err("%s: numports error\n", __func__);
		return;
	}

	while (u2_port_nums--) {
		scnprintf(port_name, sizeof(port_name), "u2port%02d",
			  u2_port_nums + 1);
		dir = debugfs_create_dir(port_name, parent);
		/* create file and give permission */
		debugfs_create_file("portsc", 0444, dir, xhci->usb2_ports[u2_port_nums], &port_fops);
	}

	while (u3_port_nums--) {
		scnprintf(port_name, sizeof(port_name), "u3port%02d",
			  u3_port_nums + 1);
		dir = debugfs_create_dir(port_name, parent);
		/* create file and give permission */
		debugfs_create_file("portsc", 0444, dir, xhci->usb3_ports[u3_port_nums], &port_fops);
	}
}

static void xhci_create_debugfs(struct xhci_hcd *xhci)
{
	INIT_LIST_HEAD(&g_regset_list);

	xhci_debugfs_regset(xhci,
			    0,
			    xhci_cap_regs, ARRAY_SIZE(xhci_cap_regs),
			    xhci->debugfs_root, "reg-cap");

	xhci_debugfs_regset(xhci,
			    HC_LENGTH(readl(&xhci->cap_regs->hc_capbase)),
			    xhci_op_regs, ARRAY_SIZE(xhci_op_regs),
			    xhci->debugfs_root, "reg-op");

	xhci_debugfs_regset(xhci,
			    readl(&xhci->cap_regs->run_regs_off) & RTSOFF_MASK,
			    xhci_runtime_regs, ARRAY_SIZE(xhci_runtime_regs),
			    xhci->debugfs_root, "reg-runtime");

	xhci_debugfs_extcap_regset(xhci, XHCI_EXT_CAPS_LEGACY,
				   xhci_extcap_legsup,
				   ARRAY_SIZE(xhci_extcap_legsup),
				   "reg-ext-legsup");

	xhci_debugfs_extcap_regset(xhci, XHCI_EXT_CAPS_PROTOCOL,
				   xhci_extcap_protocol,
				   ARRAY_SIZE(xhci_extcap_protocol),
				   "reg-ext-protocol");

	xhci_debugfs_extcap_regset(xhci, XHCI_EXT_CAPS_DEBUG,
				   xhci_extcap_dbc,
				   ARRAY_SIZE(xhci_extcap_dbc),
				   "reg-ext-dbc");

	xhci_debugfs_create_ring_dir(xhci, &xhci->cmd_ring,
				     "command-ring",
				     xhci->debugfs_root);

	xhci_debugfs_create_ring_dir(xhci, &xhci->event_ring,
				     "event-ring",
				     xhci->debugfs_root);

	xhci_debugfs_create_ports(xhci, xhci->debugfs_root);
}

static void xhci_remove_debugfs(struct xhci_hcd *xhci)
{
	struct xhci_regset	*rgs = NULL;
	struct xhci_regset	*tmp = NULL;

	list_for_each_entry_safe(rgs, tmp, &g_regset_list, list)
		xhci_debugfs_free_regset(rgs);
}

static int xhci_compliance_show(struct seq_file *s, void *d)
{
	seq_printf(s, "usage: echo 1 > xhci_compliance\n");

	return 0;
}

static int xhci_compliance_open(struct inode *inode, struct file *file)
{
	return single_open(file, xhci_compliance_show, inode->i_private);
}

static ssize_t xhci_compliance_write(struct file *file, const char __user *ubuf,
	size_t size, loff_t *ppos)
{
	struct seq_file *s = file->private_data;
	struct xhci_hcd *xhci = s->private;

	if (size > PAGE_SIZE) {
		pr_err("%s too long!\n", __func__);
		return -ENOMEM;
	}

	pr_info("set link state compliance mode\n");
	xhci_set_link_state(xhci, xhci->usb3_ports, 0, USB_SS_PORT_LS_COMP_MOD);

	return size;
}

static const struct file_operations xhci_compliance_debug_fops = {
	.open = xhci_compliance_open,
	.read = seq_read,
	.write = xhci_compliance_write,
	.release = single_release,
};

int xhci_create_debug_file(struct xhci_hcd *xhci)
{
	struct dentry *root = NULL;
	struct dentry *file = NULL;
	int ret;

	if (!xhci)
		return -EINVAL;

	root = debugfs_create_dir("xhci", usb_debug_root);
	if (!root) {
		ret = -ENOMEM;
		goto root_dbg_fs_err;
	}

	file = debugfs_create_file("xhci_compliance", S_IWUSR | S_IRUSR,
		root, xhci, &xhci_compliance_debug_fops);
	if (!file) {
		pr_err("create xhci_compliance debugfs file error!\n");
		ret = -ENOMEM;
		goto son_dbg_fs_err;
	}

	xhci->debugfs_root = root;

	xhci_create_debugfs(xhci);

	return 0;

son_dbg_fs_err:
	debugfs_remove_recursive(root);

root_dbg_fs_err:
	return ret;
}

void xhci_remove_debug_file(struct xhci_hcd *xhci)
{
	if (!xhci)
		return;

	if (xhci->debugfs_root)
		debugfs_remove_recursive(xhci->debugfs_root);
	xhci->debugfs_root = NULL;
	xhci_remove_debugfs(xhci);
}
