/*
 * hiusbc_debugfs.c -- Debugfs for Hisilicon USB Controller.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/usb/ch9.h>
#include "hiusbc_core.h"
#include "hiusbc_debug.h"
#include "hiusbc_gadget.h"
#include "hiusbc_setup.h"
#include "hiusbc_debugfs.h"

u32 hiusbc_log_level = HIUSBC_DEBUG_ERR;

#define MAX_BUF_SIZE	16
#define HIUSBC_LOG_LEVEL_HIGHEST	0xffffffff
#define HIUSBC_LOG_LEVEL_LOWEST		0

struct hiusbc_debugfs_item {
	u32 val;
	const char *string;
};

struct hiusbc_debugfs_cb {
	const char *string;
	void (*cb)(struct hiusbc *hiusbc);
};

#define dump_register_dev(nm ...)			\
{						\
	.name	= #nm,				\
	.offset	= MPI_APP_DEVICE_REG_BASE_ADDR +	\
		nm##_OFFSET - HIUSBC_DEVICE_MODE_REG_BASE,	\
}

#define dump_register_com(nm ...)			\
{						\
	.name	= #nm,				\
	.offset	= MPI_APP_COM_REG_BASE_ADDR +	\
		nm##_OFFSET - HIUSBC_DEVICE_MODE_REG_BASE,	\
}

static const struct debugfs_reg32 hiusbc_regs[] = {
	dump_register_dev(MPI_APP_DEVICE_REG_DEV_USB3_U1EL),
	dump_register_dev(MPI_APP_DEVICE_REG_DEV_USB3_U2EL),
	dump_register_dev(MPI_APP_DEVICE_REG_DEV_CFG),
	dump_register_dev(MPI_APP_DEVICE_REG_DEV_CTL),
	dump_register_dev(MPI_APP_DEVICE_REG_DEV_RST),
	dump_register_dev(MPI_APP_DEVICE_REG_DEV_STATUS),
	dump_register_dev(MPI_APP_DEVICE_REG_DEV_EVENT_EN),
	dump_register_dev(MPI_APP_DEVICE_REG_DEV_EVENT),
	dump_register_dev(MPI_APP_DEVICE_REG_DEV_U2_PORTSC),
	dump_register_dev(MPI_APP_DEVICE_REG_DEV_U3_PORTSC),
	dump_register_dev(MPI_APP_DEVICE_REG_DEV_U2PORT_TEST_MODE),

	dump_register_com(MPI_APP_COM_REG_DRD_MODE),
	dump_register_com(MPI_APP_COM_REG_CSR_RST_N),
	dump_register_com(MPI_APP_COM_REG_U3_CONTINUE_MODE),
};

static int hiusbc_log_level_show(struct seq_file *s, void *reserved)
{
	struct hiusbc *hiusbc = s->private;
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	seq_printf(s, "current log_level 0x%x\n", hiusbc_log_level);
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	return 0;
}

static int hiusbc_log_level_open(struct inode *inode,
				struct file *file)
{
	return single_open(file, hiusbc_log_level_show, inode->i_private);
}

static ssize_t hiusbc_log_level_write(struct file *file,
		const char __user *ubuf, size_t count, loff_t *ppos)
{
	char buf[MAX_BUF_SIZE] = {0};

	if (copy_from_user(buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
		return -EFAULT;

	if (!strncmp(buf, "log_on", strlen("log_on")))
		hiusbc_log_level = HIUSBC_LOG_LEVEL_HIGHEST;
	else if (!strncmp(buf, "log_off", strlen("log_off")))
		hiusbc_log_level = HIUSBC_LOG_LEVEL_LOWEST;
	else if (!strncmp(buf, "err_on", strlen("err_on")))
		hiusbc_log_level |= HIUSBC_DEBUG_ERR;
	else if (!strncmp(buf, "err_off", strlen("err_off")))
		hiusbc_log_level &= ~HIUSBC_DEBUG_ERR;
	else if (!strncmp(buf, "temp_on", strlen("temp_on")))
		hiusbc_log_level |= HIUSBC_DEBUG_TEMP;
	else if (!strncmp(buf, "temp_off", strlen("temp_off")))
		hiusbc_log_level &= ~HIUSBC_DEBUG_TEMP;
	else
		return -EINVAL;

	return count;
}

static const struct file_operations hiusbc_log_level_fops = {
	.open			= hiusbc_log_level_open,
	.write			= hiusbc_log_level_write,
	.read			= seq_read,
	.llseek			= seq_lseek,
	.release		= single_release,
};

static int hiusbc_mode_show(struct seq_file *s, void *unused)
{
	struct hiusbc *hiusbc = s->private;
	unsigned long flags;
	u32 reg;

	spin_lock_irqsave(&hiusbc->lock, flags);
	reg = hiusbc_readl(hiusbc->com_regs, MPI_APP_COM_REG_DRD_MODE_OFFSET);
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	switch (hiusbc_get_drd_mode(reg)) {
	case HIUSBC_DRD_MODE_HOST:
		seq_puts(s, "host\n");
		break;
	case HIUSBC_DRD_MODE_DEVICE:
		seq_puts(s, "device\n");
		break;
	default:
		seq_printf(s, "UNKNOWN %08x\n", hiusbc_get_drd_mode(reg));
	}

	return 0;
}

static int hiusbc_mode_open(struct inode *inode, struct file *file)
{
	return single_open(file, hiusbc_mode_show, inode->i_private);
}

static ssize_t hiusbc_mode_write(struct file *file,
		const char __user *ubuf, size_t count, loff_t *ppos)
{
	struct seq_file *s = file->private_data;
	struct hiusbc *hiusbc = s->private;
	enum hiusbc_dr_mode mode = HIUSBC_DR_MODE_UNKNOWN;
	char buf[MAX_BUF_SIZE];

	if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
		return -EFAULT;

	if (!strncmp(buf, "host", strlen("host")))
		mode = HIUSBC_DR_MODE_HOST;

	if (!strncmp(buf, "device", strlen("device")))
		mode = HIUSBC_DR_MODE_DEVICE;

	hiusbc_set_mode(hiusbc, mode);

	return count;
}

static const struct file_operations hiusbc_mode_fops = {
	.open			= hiusbc_mode_open,
	.write			= hiusbc_mode_write,
	.read			= seq_read,
	.llseek			= seq_lseek,
	.release		= single_release,
};

static const struct hiusbc_debugfs_item testmode[] = {
	{.val = 0, .string = "no test\n"},
	{.val = TEST_J, .string = "test_j\n"},
	{.val = TEST_K, .string = "test_k\n"},
	{.val = TEST_SE0_NAK, .string = "test_se0_nak\n"},
	{.val = TEST_PACKET, .string = "test_packet\n"},
	{.val = TEST_FORCE_EN, .string = "test_force_enable\n"}
};

static int hiusbc_testmode_show(struct seq_file *s, void *unused)
{
	struct hiusbc *hiusbc = s->private;
	unsigned long flags;
	u32 reg, mode, i;

	spin_lock_irqsave(&hiusbc->lock, flags);
	reg = hiusbc_readl(hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_U2PORT_TEST_MODE_OFFSET);
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	mode = hiusbc_get_u2_test_mode(reg);

	for (i = 0; i < ARRAY_SIZE(testmode); i++) {
		if (mode == testmode[i].val) {
			seq_printf(s, testmode[i].string);
			break;
		}
	}

	if (i == ARRAY_SIZE(testmode))
		seq_printf(s, "UNKNOWN %d\n", mode);

	return 0;
}

static int hiusbc_testmode_open(struct inode *inode,
			struct file *file)
{
	return single_open(file, hiusbc_testmode_show, inode->i_private);
}

static ssize_t hiusbc_testmode_write(struct file *file,
		const char __user *ubuf, size_t count, loff_t *ppos)
{
	struct seq_file *s = file->private_data;
	struct hiusbc *hiusbc = s->private;
	unsigned long flags;
	u8 mode = 0;
	char buf[MAX_BUF_SIZE];
	u32 reg, i;
	u32 remote_wakeup_backup;

	if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
		return -EFAULT;

	for (i = 0; i < ARRAY_SIZE(testmode); i++) {
		if (!strncmp(buf, testmode[i].string,
				strlen(testmode[i].string))) {
			mode = testmode[i].val;
			break;
		}
	}

	spin_lock_irqsave(&hiusbc->lock, flags);
	reg = hiusbc_readl(hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_CFG_OFFSET);
	reg &= ~MPI_APP_DEVICE_REG_LPM_ENABLE_MASK;
	hiusbc_writel(reg, hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_CFG_OFFSET);

	remote_wakeup_backup = hiusbc->remote_wakeup;
	hiusbc->remote_wakeup = true;
	if (hiusbc_wakeup(hiusbc))
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
				"failed to set link state to L0\n");
	hiusbc_req_set_test_mode(hiusbc, mode);
	hiusbc->remote_wakeup = remote_wakeup_backup;
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	return count;
}

static const struct file_operations hiusbc_testmode_fops = {
	.open			= hiusbc_testmode_open,
	.write			= hiusbc_testmode_write,
	.read			= seq_read,
	.llseek			= seq_lseek,
	.release		= single_release,
};

static const struct hiusbc_debugfs_item u2_link_state_string[] = {
	{.val = HIUSBC_LINK_STATE_L3, .string = "L3"},
	{.val = HIUSBC_LINK_STATE_RESET, .string = "RESET"},
	{.val = HIUSBC_LINK_STATE_L1, .string = "L1"},
	{.val = HIUSBC_LINK_STATE_L2, .string = "L2"},
	{.val = HIUSBC_LINK_STATE_L0, .string = "L0"},
	{.val = HIUSBC_LINK_STATE_TEST, .string = "TEST"},
	{.val = HIUSBC_LINK_STATE_L1_RESUME, .string = "L1 RESUME"},
	{.val = HIUSBC_LINK_STATE_L2_RESUME, .string = "L2 RESUME"}
};

static const struct hiusbc_debugfs_item u3_link_state_string[] = {
	{.val = HIUSBC_LINK_STATE_U0, .string = "U0"},
	{.val = HIUSBC_LINK_STATE_U1, .string = "U1"},
	{.val = HIUSBC_LINK_STATE_U2, .string = "U2"},
	{.val = HIUSBC_LINK_STATE_U3, .string = "U3"},
	{.val = HIUSBC_LINK_STATE_ESS_DIS, .string = "eSS.Disabled"},
	{.val = HIUSBC_LINK_STATE_RX_DET, .string = "RX.Detect"},
	{.val = HIUSBC_LINK_STATE_ESS_INAC, .string = "eSS.Inactive"},
	{.val = HIUSBC_LINK_STATE_POLLING, .string = "Polling"},
	{.val = HIUSBC_LINK_STATE_RECOVERY, .string = "Recovery"},
	{.val = HIUSBC_LINK_STATE_HOT_RESET, .string = "Hot Reset"},
	{.val = HIUSBC_LINK_STATE_COMPLIANCE, .string = "Compliance"},
	{.val = HIUSBC_LINK_STATE_LOOPBACK, .string = "Loopback"}
};

static const char *hiusbc_link_string(enum usb_device_speed link_speed,
			enum hiusbc_link_state link_state)
{
	u32 i, size;
	const struct hiusbc_debugfs_item *string = NULL;

	if (link_speed == USB_SPEED_UNKNOWN)
		return "UNKNOWN link speed\n";

	if (link_speed <= USB_SPEED_HIGH) {
		size = ARRAY_SIZE(u2_link_state_string);
		string = u2_link_state_string;
	}

	if (link_speed >= USB_SPEED_SUPER) {
		size = ARRAY_SIZE(u3_link_state_string);
		string = u3_link_state_string;
	}

	for (i = 0; i < size; i++) {
		if (link_state == string[i].val)
			return string[i].string;
	}
	return "UNKNOWN link state\n";
}

static int hiusbc_link_state_show(struct seq_file *s, void *unused)
{
	struct hiusbc *hiusbc = s->private;
	unsigned long flags;
	enum usb_device_speed speed;
	enum hiusbc_link_state state;

	spin_lock_irqsave(&hiusbc->lock, flags);
	speed = hiusbc_get_link_speed(hiusbc);
	state = hiusbc_get_link_state(hiusbc, speed);
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	seq_printf(s, "%s\n", hiusbc_link_string(speed, state));

	return 0;
}

static int hiusbc_link_state_open(struct inode *inode,
		struct file *file)
{
	return single_open(file, hiusbc_link_state_show, inode->i_private);
}

static ssize_t hiusbc_link_state_write(struct file *file,
		const char __user *ubuf, size_t count, loff_t *ppos)
{
	struct seq_file *s = file->private_data;
	struct hiusbc *hiusbc = s->private;
	unsigned long flags;
	enum hiusbc_link_state state;
	enum usb_device_speed speed = USB_SPEED_SUPER;
	char buf[MAX_BUF_SIZE];

	if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
		return -EFAULT;

	if (!strncmp(buf, "eSS.Disabled", strlen("eSS.Disabled"))) {
		state = HIUSBC_LINK_STATE_ESS_DIS;
	} else if (!strncmp(buf, "eSS.Inactive", strlen("eSS.Inactive"))) {
		state = HIUSBC_LINK_STATE_ESS_INAC;
	} else if (!strncmp(buf, "Recovery", strlen("Recovery"))) {
		state = HIUSBC_LINK_STATE_RECOVERY;
	} else if (!strncmp(buf, "L0", strlen("L0"))) {
		speed = USB_SPEED_HIGH;
		state = HIUSBC_LINK_STATE_L0;
	} else {
		return -EINVAL;
	}

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc_set_link_state(hiusbc, speed, state);
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	return count;
}

static const struct file_operations hiusbc_link_state_fops = {
	.open			= hiusbc_link_state_open,
	.write			= hiusbc_link_state_write,
	.read			= seq_read,
	.llseek			= seq_lseek,
	.release		= single_release,
};

static int hiusbc_maximum_speed_show(struct seq_file *s,
				void *reserved)
{
	struct hiusbc *hiusbc = s->private;
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	seq_printf(s, "current maximum_speed %s\n"
		      "Usage:\n"
		      " write \"full\" to force USB full-speed\n"
		      " write \"high\" to force USB high-speed\n",
		      usb_speed_string(hiusbc->max_speed));
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	return 0;
}

static int hiusbc_maximum_speed_open(struct inode *inode,
			struct file *file)
{
	return single_open(file, hiusbc_maximum_speed_show, inode->i_private);
}

static ssize_t hiusbc_maximum_speed_write(struct file *file,
		const char __user *ubuf, size_t count, loff_t *ppos)
{
	struct seq_file *s = file->private_data;
	struct hiusbc *hiusbc = s->private;
	unsigned long flags;
	u32 maximum_speed;
	char buf[MAX_BUF_SIZE] = {0};

	if (copy_from_user(buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
		return -EFAULT;

	if (!strncmp(buf, "super+", strlen("super+")))
		maximum_speed = USB_SPEED_SUPER_PLUS;
	else if (!strncmp(buf, "super", strlen("super")))
		maximum_speed = USB_SPEED_SUPER;
	else if (!strncmp(buf, "high", strlen("high")))
		maximum_speed = USB_SPEED_HIGH;
	else if (!strncmp(buf, "full", strlen("full")))
		maximum_speed = USB_SPEED_FULL;
	else if (!strncmp(buf, "clear", strlen("clear")))
		maximum_speed = USB_SPEED_UNKNOWN;
	else
		return -EINVAL;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->max_speed = maximum_speed;
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	return count;
}

static const struct file_operations hiusbc_maximum_speed_fops = {
	.open			= hiusbc_maximum_speed_open,
	.write			= hiusbc_maximum_speed_write,
	.read			= seq_read,
	.llseek			= seq_lseek,
	.release		= single_release,
};

static int hiusbc_current_speed_show(struct seq_file *s,
			void *reserved)
{
	struct hiusbc *hiusbc = s->private;
	enum usb_device_speed current_speed;
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	current_speed = hiusbc_get_link_speed(hiusbc);
	seq_printf(s, "current current_speed %s\n"
		      "Usage:\n"
		      " write \"full\" to force USB full-speed\n"
		      " write \"high\" to force USB high-speed\n",
			usb_speed_string(current_speed));
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	return 0;
}

static int hiusbc_current_speed_open(struct inode *inode,
		struct file *file)
{
	return single_open(file, hiusbc_current_speed_show, inode->i_private);
}

static ssize_t hiusbc_current_speed_write(struct file *file,
		const char __user *ubuf, size_t count, loff_t *ppos)
{
	struct seq_file *s = file->private_data;
	struct hiusbc *hiusbc = s->private;
	unsigned long flags;
	u32 current_speed;
	char buf[MAX_BUF_SIZE] = {0};

	if (copy_from_user(buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
		return -EFAULT;

	if (!strncmp(buf, "super+", strlen("super+")))
		current_speed = USB_SPEED_SUPER_PLUS;
	else if (!strncmp(buf, "super", strlen("super")))
		current_speed = USB_SPEED_SUPER;
	else if (!strncmp(buf, "high", strlen("high")))
		current_speed = USB_SPEED_HIGH;
	else if (!strncmp(buf, "full", strlen("full")))
		current_speed = USB_SPEED_FULL;
	else
		return -EINVAL;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc_set_speed(hiusbc, current_speed);
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	return count;
}

static const struct file_operations hiusbc_current_speed_fops = {
	.open			= hiusbc_current_speed_open,
	.write			= hiusbc_current_speed_write,
	.read			= seq_read,
	.llseek			= seq_lseek,
	.release		= single_release,
};

static int hiusbc_cp_test_show(struct seq_file *s, void *reserved)
{
	struct hiusbc *hiusbc = s->private;
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	seq_printf(s, "[cptest mode: %u]\n", hiusbc->cp_test);
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	return 0;
}

static int hiusbc_cp_test_open(struct inode *inode, struct file *file)
{
	return single_open(file, hiusbc_cp_test_show, inode->i_private);
}

static ssize_t hiusbc_cp_test_write(struct file *file,
		const char __user *ubuf, size_t count, loff_t *ppos)
{
	struct seq_file		*s = file->private_data;
	struct hiusbc		*hiusbc = s->private;
	unsigned long		flags;
	char			buf[MAX_BUF_SIZE] = {0};

	if (copy_from_user(buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
		return -EFAULT;

	if (!strncmp(buf, "cp0", strlen("cp0"))) {
		spin_lock_irqsave(&hiusbc->lock, flags);
		hiusbc_cptest_set_to_cp0(hiusbc);
		spin_unlock_irqrestore(&hiusbc->lock, flags);
		return count;
	}

	if (!strncmp(buf, "next_pattern", strlen("next_pattern"))) {
		spin_lock_irqsave(&hiusbc->lock, flags);
		hiusbc_cptest_next_pattern(hiusbc);
		spin_unlock_irqrestore(&hiusbc->lock, flags);
		return count;
	}

	return -EINVAL;
}

static const struct file_operations hiusbc_cp_test_fops = {
	.open			= hiusbc_cp_test_open,
	.write			= hiusbc_cp_test_write,
	.read			= seq_read,
	.llseek			= seq_lseek,
	.release		= single_release,
};

static int hiusbc_u3_lpm_show(struct seq_file *s, void *reserved)
{
	struct hiusbc *hiusbc = s->private;
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	seq_printf(s, "u3 host lpm is %s\n"
		      "u3 device lpm is %s\n"
		      "u3 device lpm u1 accept is %s\n"
		      "u3 device lpm u2 accept is %s\n"
		      "u3 device lpm u1 initiate is %s\n"
		      "u3 device lpm u2 initiate is %s\n",
		      hiusbc->usb3_host_lpm_capable ? "on" : "off",
		      hiusbc->usb3_dev_lpm_capable ? "on" : "off",
		      hiusbc->usb3_dev_lpm_u1_accept ? "on" : "off",
		      hiusbc->usb3_dev_lpm_u2_accept ? "on" : "off",
		      hiusbc->usb3_dev_lpm_u1_initiate ? "on" : "off",
		      hiusbc->usb3_dev_lpm_u2_initiate ? "on" : "off");
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	return 0;
}

static int hiusbc_u3_lpm_open(struct inode *inode, struct file *file)
{
	return single_open(file, hiusbc_u3_lpm_show, inode->i_private);
}

static void usb3_host_lpm_on(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->usb3_host_lpm_capable = 1;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static void usb3_host_lpm_off(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->usb3_host_lpm_capable = 0;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static void usb3_device_lpm_on(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->usb3_dev_lpm_capable = 1;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static void usb3_device_lpm_off(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->usb3_dev_lpm_capable = 0;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static void usb3_u1_accept_on(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->usb3_dev_lpm_u1_accept = 1;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static void usb3_u1_accept_off(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->usb3_dev_lpm_u1_accept = 0;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static void usb3_u2_accept_on(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->usb3_dev_lpm_u2_accept = 1;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static void usb3_u2_accept_off(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->usb3_dev_lpm_u2_accept = 0;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static void usb3_u1_init_on(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->usb3_dev_lpm_u1_initiate = 1;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static void usb3_u1_init_off(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->usb3_dev_lpm_u1_initiate = 0;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static void usb3_u2_init_on(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->usb3_dev_lpm_u2_initiate = 1;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static void usb3_u2_init_off(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->usb3_dev_lpm_u2_initiate = 0;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static void usb3_ux_exit_on(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->usb3_dev_lpm_ux_exit = 1;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static void usb3_ux_exit_off(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->usb3_dev_lpm_ux_exit = 0;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static const struct hiusbc_debugfs_cb lpm_option[] = {
	{.string = "host_on", .cb = usb3_host_lpm_on},
	{.string = "host_off", .cb = usb3_host_lpm_off},
	{.string = "device_on", .cb = usb3_device_lpm_on},
	{.string = "device_off", .cb = usb3_device_lpm_off},
	{.string = "u1_accept_on", .cb = usb3_u1_accept_on},
	{.string = "u1_accept_off", .cb = usb3_u1_accept_off},
	{.string = "u2_accept_on", .cb = usb3_u2_accept_on},
	{.string = "u2_accept_off", .cb = usb3_u2_accept_off},
	{.string = "u1_init_on", .cb = usb3_u1_init_on},
	{.string = "u1_init_off", .cb = usb3_u1_init_off},
	{.string = "u2_init_on", .cb = usb3_u2_init_on},
	{.string = "u2_init_off", .cb = usb3_u2_init_off},
	{.string = "ux_exit_on", .cb = usb3_ux_exit_on},
	{.string = "ux_exit_off", .cb = usb3_ux_exit_off}
};

static ssize_t hiusbc_u3_lpm_write(struct file *file,
		const char __user *ubuf, size_t count, loff_t *ppos)
{
	struct seq_file *s = file->private_data;
	struct hiusbc *hiusbc = s->private;
	char buf[MAX_BUF_SIZE] = {0};
	u32 i;

	if (copy_from_user(buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
		return -EFAULT;

	for (i = 0; i < ARRAY_SIZE(lpm_option); i++) {
		if (!strncmp(buf, lpm_option[i].string,
				strlen(lpm_option[i].string))) {
			lpm_option[i].cb(hiusbc);
			return count;
		}
	}

	return -EINVAL;
}

static const struct file_operations hiusbc_u3_lpm_fops = {
	.open			= hiusbc_u3_lpm_open,
	.write			= hiusbc_u3_lpm_write,
	.read			= seq_read,
	.llseek			= seq_lseek,
	.release		= single_release,
};

static int hiusbc_fpga_debug_show(struct seq_file *s,
			void *reserved)
{
	struct hiusbc *hiusbc = s->private;
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	seq_printf(s, "db_on_received_event is %s\n",
			hiusbc->db_on_received_event ? "on" : "off");
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	return 0;
}

static int hiusbc_fpga_debug_open(struct inode *inode,
		struct file *file)
{
	return single_open(file, hiusbc_fpga_debug_show, inode->i_private);
}

static const struct hiusbc_debugfs_item eps_info[] = {
	{.val = 0, .string = "ep0"},
	{.val = 1, .string = "ep1"},
	{.val = 2, .string = "ep2"},
	{.val = 3, .string = "ep3"},
	{.val = 4, .string = "ep4"},
	{.val = 5, .string = "ep5"},
	{.val = 6, .string = "ep6"},
	{.val = 7, .string = "ep7"},
	{.val = 8, .string = "ep8"},
	{.val = 9, .string = "ep9"}
};

static const struct hiusbc_debugfs_item knock_eps_info[] = {
	{.val = 2, .string = "knock_ep2"},
	{.val = 3, .string = "knock_ep3"},
	{.val = 4, .string = "knock_ep4"},
	{.val = 5, .string = "knock_ep5"},
	{.val = 6, .string = "knock_ep6"},
	{.val = 7, .string = "knock_ep7"},
	{.val = 8, .string = "knock_ep8"},
	{.val = 9, .string = "knock_ep9"},
	{.val = 10, .string = "knock_ep10"},
	{.val = 11, .string = "knock_ep11"},
	{.val = 12, .string = "knock_ep12"},
	{.val = 13, .string = "knock_ep13"},
	{.val = 14, .string = "knock_ep14"},
	{.val = 15, .string = "knock_ep15"},
	{.val = 16, .string = "knock_ep16"}
};

static const struct hiusbc_debugfs_item force_maxp[] = {
	{.val = 4, .string = "quarter_maxp"},
	{.val = 2, .string = "half_maxp"},
	{.val = 1, .string = "normal_maxp"}
};

static void usb3_more_db(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->db_on_received_event = 1;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static void usb3_less_db(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->db_on_received_event = 0;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static void usb3_trpu_reset_on(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->trpu_reset_switch = 1;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static void usb3_trpu_reset_off(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->trpu_reset_switch = 0;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static void usb3_stop_switch_on(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->stop_before_drd_switch = 1;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static void usb3_stop_switch_off(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->stop_before_drd_switch = 0;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static void usb3_reset_switch_on(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->reset_before_drd_switch = 1;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static void usb3_reset_switch_off(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc->reset_before_drd_switch = 0;
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static void usb3_dev_event(struct hiusbc *hiusbc)
{
	unsigned long flags;

	spin_lock_irqsave(&hiusbc->lock, flags);
	hiusbc_show_dev_event_counter(hiusbc);
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static const struct hiusbc_debugfs_cb fpga_debug_option[] = {
	{.string = "more_db", .cb = usb3_more_db},
	{.string = "less_db", .cb = usb3_less_db},
	{.string = "trpu_reset_on", .cb = usb3_trpu_reset_on},
	{.string = "trpu_reset_off", .cb = usb3_trpu_reset_off},
	{.string = "stop_switch_on", .cb = usb3_stop_switch_on},
	{.string = "stop_switch_off", .cb = usb3_stop_switch_off},
	{.string = "reset_switch_on", .cb = usb3_reset_switch_on},
	{.string = "reset_switch_off", .cb = usb3_reset_switch_off},
	{.string = "dev_event", .cb = usb3_dev_event}
};

static ssize_t hiusbc_fpga_debug_write(struct file *file,
		const char __user *ubuf, size_t count, loff_t *ppos)
{
	struct seq_file *s = file->private_data;
	struct hiusbc *hiusbc = s->private;
	u32 i;
	char buf[MAX_BUF_SIZE] = {0};

	if (copy_from_user(buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
		return -EFAULT;

	for (i = 0; i < ARRAY_SIZE(fpga_debug_option); i++) {
		if (!strncmp(buf, fpga_debug_option[i].string,
			strlen(fpga_debug_option[i].string))) {
			fpga_debug_option[i].cb(hiusbc);
			return count;
		}
	}

	for (i = 0; i < ARRAY_SIZE(eps_info); i++) {
		if (!strncmp(buf, eps_info[i].string,
			strlen(eps_info[i].string))) {
			hiusbc_show_xfer_event_counter(
					hiusbc->eps[eps_info[i].val]);
			return count;
		}
	}

	for (i = 0; i < ARRAY_SIZE(knock_eps_info); i++) {
		if (!strncmp(buf, knock_eps_info[i].string,
			strlen(knock_eps_info[i].string))) {
			hiusbc_debug_knock_ep(
					hiusbc->eps[knock_eps_info[i].val]);
			return count;
		}
	}

#ifdef CONFIG_HIUSBC_EDA_TEST_CASE
	for (i = 0; i < ARRAY_SIZE(force_maxp); i++) {
		if (!strncmp(buf, force_maxp[i].string,
			strlen(force_maxp[i].string))) {
			hiusbc->force_maxp = force_maxp[i].val;
			return count;
		}
	}
#endif
	return -EINVAL;
}

static const struct file_operations hiusbc_fpga_debug_fops = {
	.open			= hiusbc_fpga_debug_open,
	.write			= hiusbc_fpga_debug_write,
	.read			= seq_read,
	.llseek			= seq_lseek,
	.release		= single_release,
};

void hiusbc_debugfs_init(struct hiusbc *hiusbc)
{
	struct dentry *root = NULL;
	struct dentry *file = NULL;

	root = debugfs_create_dir("hiusbc", NULL);
	if (!root) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
				"Can't create 'hiusbc' debugfs\n");
		return;
	}
	hiusbc->root = root;

	hiusbc->regset = kzalloc(sizeof(*hiusbc->regset), GFP_KERNEL);
	if (!hiusbc->regset) {
		debugfs_remove_recursive(root);
		hiusbc->root = NULL;
		return;
	}

	hiusbc->regset->regs = hiusbc_regs;
	hiusbc->regset->nregs = ARRAY_SIZE(hiusbc_regs);
	hiusbc->regset->base = hiusbc->regs;

	file = debugfs_create_regset32("regdump", 0444, root, hiusbc->regset);
	if (!file)
		pr_err("Can't create 'regdump' debugfs\n");

	file = debugfs_create_file("log_level", 0644, root,
			hiusbc, &hiusbc_log_level_fops);
	if (!file)
		pr_err("Can't create 'log_level' debugfs\n");

	file = debugfs_create_file("mode", 0644, root,
			hiusbc, &hiusbc_mode_fops);
	if (!file)
		pr_err("Can't create 'mode' debugfs\n");

	file = debugfs_create_file("testmode", 0644, root,
			hiusbc, &hiusbc_testmode_fops);
	if (!file)
		pr_err("Can't create 'testmode' debugfs\n");

	file = debugfs_create_file("link_state", 0644,
			root, hiusbc, &hiusbc_link_state_fops);
	if (!file)
		pr_err("Can't create 'link_state' debugfs\n");

	file = debugfs_create_file("maximum_speed", 0644, root,
			hiusbc, &hiusbc_maximum_speed_fops);
	if (!file)
		pr_err("Can't create 'maximum_speed' debugfs\n", __func__);

	file = debugfs_create_file("current_speed", 0644, root,
			hiusbc, &hiusbc_current_speed_fops);
	if (!file)
		pr_err("Can't create 'current_speed' debugfs\n", __func__);

	file = debugfs_create_file("cptest", 0644, root,
			hiusbc, &hiusbc_cp_test_fops);
	if (!file)
		pr_err("Can't create 'cptest' debugfs\n", __func__);

	file = debugfs_create_file("u3_lpm", 0644, root,
			hiusbc, &hiusbc_u3_lpm_fops);
	if (!file)
		pr_err("Can't create 'u3_lpm' debugfs\n", __func__);

	file = debugfs_create_file("fpga_debug", 0644, root,
			hiusbc, &hiusbc_fpga_debug_fops);
	if (!file)
		pr_err("Can't create 'fpga_debug' debugfs\n", __func__);

}

void hiusbc_debugfs_exit(struct hiusbc *hiusbc)
{
	debugfs_remove_recursive(hiusbc->root);
	kfree(hiusbc->regset);
}
