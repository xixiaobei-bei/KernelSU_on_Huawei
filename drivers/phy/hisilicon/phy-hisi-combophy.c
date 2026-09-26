/*
 * phy-hisi-combophy.h
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 * Create:2019-09-24
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/clk.h>
#include <linux/debugfs.h>
#include <linux/hisi/hisi_dss_dp.h>
#include <linux/hisi/usb/hisi_usb_misc_ctrl.h>
#include <linux/hisi/usb/hisi_usb_phy.h>
#include <linux/hisi/usb/tca.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of_address.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/usb.h>
#include <soc_acpu_baseaddr_interface.h>
#include "combophy_firmware.h"
#include "hisi-combophy-mcu.h"

#undef pr_fmt
#define pr_fmt(fmt) "[COMBOPHY_PHY]%s: " fmt, __func__

#define INVALID_REG_VALUE 0xFFFFFFFFu

#define chip_combophy_to_phy_priv(pointer) \
	container_of(pointer, struct phy_priv, combophy)

enum {
	PHY_HANDLE_FOR_HISI_USB = 0,
	PHY_HANDLE_FOR_DWC3 = 1,
};

/* USB DP CTRL */
#define USBDP_COMBOPHY_CTRL1(base)	((base) + 0x54)
#define USBDP_COMBOPHY_CTRL1_POR	BIT(1)
#define USBDP_USB_CTRL_CFG0(base)	((base) + 0x10)
/* bits of USBDP_USB_CTRL_CFG0 */
#define HOST_U3_PORT_DISABLE		BIT(10)
#define HOST_FORCE_GEN1_SPEED		BIT(19)
#define HOST_NUM_U3_PORT(n)		((n & 0xf) << 25)
#define HOST_NUM_U3_PORT_MASK		(0xf << 25)

/* USB TCA */
#define USB_TCA_PHY_MODE(base)		((base) + 0x10)
#define PHY_MODE_DP_ONLY		0x1u
#define PHY_MODE_USB_ONLY		0x3u
#define PHY_MODE_COMBOPHY		0x5u
#define PHY_MODE_DP_ONLY_FLIP		0x9u
#define PHY_MODE_USB_ONLY_FLIP		0xbu
#define PHY_MODE_COMBOPHY_FLIP		0xdu
#define USB_TCA_PLL0CKGCTRLR0(base)	((base) + 0xA14)
#define USB_TCA_PLL0CKGCTRLR2(base)	((base) + 0xA24)
#define USB_TCA_PLL0SSCG_CTRL(base)	((base) + 0xA80)
#define USB_TCA_PLL0SSCG_INITIAL_R0(base)	((base) + 0xA84)
#define USB_TCA_PLL0SSCG_CNT_R0(base)	((base) + 0xA88)
#define USB_TCA_PLL0SSCG_CNT2_R0(base)	((base) + 0xA8C)
#define PLL0_SSCG_CNT_STEPSIZE_FORCE_0	BIT(1)
#define AUXCLK_TERMCTRL_LANE0(base)	((base) + 0x1AD0)
#define AUXCLK_TERMCTRL_LANE1(base)	((base) + 0x1AD4)
#define USB_TCA_PERLANE_BASE(base)	((base) + 0x2000)
#define PERLANE_TXDRVCTRL_LANE0(base)	(USB_TCA_PERLANE_BASE(base) + 0x314)
#define PERLANE_TXDRVCTRL_LANE1(base)	(USB_TCA_PERLANE_BASE(base) + 0x714)
#define PERLANE_TXEQCOEFF_LANE0(base)	(USB_TCA_PERLANE_BASE(base) + 0x318)
#define PERLANE_TXEQCOEFF_LANE1(base)	(USB_TCA_PERLANE_BASE(base) + 0x718)
#define PERLANE_RXOFSTUPDTCTRL_LANE0(base) (USB_TCA_PERLANE_BASE(base) + 0x320)
#define PERLANE_RXOFSTUPDTCTRL_LANE1(base) (USB_TCA_PERLANE_BASE(base) + 0x720)
#define RXOFSTUPDTCTRL_RXOFSTK_UPDTR	BIT(16)
#define PERLANE_RXGSACTRL_LANE0(base)	(USB_TCA_PERLANE_BASE(base) + 0x330)
#define PERLANE_RXGSACTRL_LANE1(base)	(USB_TCA_PERLANE_BASE(base) + 0x730)
#define PERLANE_RXEQACTRL_LANE0(base)	(USB_TCA_PERLANE_BASE(base) + 0x334)
#define PERLANE_RXEQACTRL_LANE1(base)	(USB_TCA_PERLANE_BASE(base) + 0x734)
#define RXCTRL_EQA_SEL_C_OFFSET		16
#define RXCTRL_EQA_SEL_C_MASK		(0x1Fu << RXCTRL_EQA_SEL_C_OFFSET)
#define PERLANE_RXCTRL0_LANE0(base)	(USB_TCA_PERLANE_BASE(base) + 0x340)
#define PERLANE_RXCTRL0_LANE1(base)	(USB_TCA_PERLANE_BASE(base) + 0x740)
#define PERLANE_RXCTRL_LANE0(base)	(USB_TCA_PERLANE_BASE(base) + 0x344)
#define PERLANE_RXCTRL_LANE1(base)	(USB_TCA_PERLANE_BASE(base) + 0x744)
#define PERLANE_RXLOSCTRL_LANE0(base)	(USB_TCA_PERLANE_BASE(base) + 0x348)
#define PERLANE_RXLOSCTRL_LANE1(base)	(USB_TCA_PERLANE_BASE(base) + 0x748)

/* USB MISC CTRL */
#define USB_MISC_CFGA0(base)		((base) + 0xa0)
#define USB3PHY_RESET			BIT(1)

#define QUIRK_CONFIG_EQC 1u
struct quirk_config {
	uint32_t flag;
	uint32_t value;
};

static const struct quirk_config unitek_qurik_config = {
	.flag = QUIRK_CONFIG_EQC, .value = 0x1fu << RXCTRL_EQA_SEL_C_OFFSET
};

static const struct usb_device_id usb_quirk_list[] = {
	/* Unitek 2.0 Hub */
	{ USB_DEVICE(0x0bda, 0x5411), .driver_info = (uintptr_t)&unitek_qurik_config },
	{ }  /* terminating entry must be last */
};

static int usb_match_device(struct usb_device *udev, const struct usb_device_id *id)
{
	if (id->idVendor != le16_to_cpu(udev->descriptor.idVendor))
		return 0;

	if (id->idProduct != le16_to_cpu(udev->descriptor.idProduct))
		return 0;

	return 1;
}

static const struct quirk_config *usb_detect_quirks(struct usb_device *udev)
{
	const struct usb_device_id *id = usb_quirk_list;

	for (; id->match_flags; id++) {
		if (usb_match_device(udev, id))
			return (const struct quirk_config *)(uintptr_t)id->driver_info;
	}

	return NULL;
}

static const uint32_t combophy_default_txdrvctrl = 0x2eu;
static const uint32_t combophy_default_txeqcoeff = 0x50f0e018u;
static const uint32_t combophy_default_rxgsactrl = 0x2088f020u;
static const uint32_t combophy_default_rxeqactrl = 0x20180010u;
static const uint32_t combophy_default_rxctrl0 = 0x400f8310u;
static const uint32_t combophy_default_rxctrl = 0x75500u;
static const uint32_t combophy_default_rxlosctrl = 0x62u;
static const uint32_t combophy_default_pll0ckgctrlr0 = 0x041200d5u;
static const uint32_t combophy_default_termctrl = 0x00080010u;
static const uint32_t combophy_default_pll0sscgcntr0 = 0x01040107;

struct phy_priv {
	struct device *dev;
	struct chip_combophy combophy;
	/* phy for dwc3 driver */
	struct phy *phy;
	TCPC_MUX_CTRL_TYPE phy_mode;
	TYPEC_PLUG_ORIEN_E typec_orien;
	struct clk *apb_clk;
	struct clk *aux_clk;
	void __iomem *usb_tca;
	void __iomem *usb_misc_ctrl;
	void __iomem *usb_dp_ctrl;
	struct dentry *debug_dir;
	/* phy parameter */
	uint32_t txdrvctrl;
	uint32_t txeqcoeff;
	uint32_t rxctrl0;
	uint32_t rxctrl;
	uint32_t rxeqactrl;
	uint32_t rxgsactrl;
	uint32_t rxlosctrl;
	uint32_t pll0ckgctrlr0;
	uint32_t termctrl;
	uint32_t pll0sscgcntr0;

	bool phy_running;
	bool start_mcu;
	bool debug;
	bool test_stub;
	bool config_ssc;
	bool config_quirk_device;

	struct notifier_block usb_nb;
	struct dentry *regdump_file;
};

/* used: soft_mode_to_phy_mode[typec_orien][phy_mode] */
static uint32_t soft_mode_to_phy_mode[TYPEC_ORIEN_MAX][TCPC_MUX_MODE_MAX] = {
	/* TYPEC_ORIEN_POSITIVE */
	{
		0,
		PHY_MODE_USB_ONLY, /* TCPC_USB31_CONNECTED */
		PHY_MODE_DP_ONLY, /* TCPC_DP */
		PHY_MODE_COMBOPHY /* TCPC_USB31_AND_DP_2LINE */
	},
	/* TYPEC_ORIEN_NEGATIVE */
	{
		0,
		PHY_MODE_USB_ONLY_FLIP,
		PHY_MODE_DP_ONLY_FLIP,
		PHY_MODE_COMBOPHY_FLIP
	},
};

static DEFINE_MUTEX(phy_mutex);

#ifdef CONFIG_HISI_DEBUG_FS
#define USB_TCA_BASE		SOC_ACPU_USB_TCA_BASE_ADDR
#define USB_MISC_CTRL_BASE	SOC_ACPU_USB_MISC_CTRL_BASE_ADDR

struct combophy_dump_reg32 {
	uint32_t physical_base;
	uint32_t start;
	uint32_t end;
};

#define dump_reg32(p, s, e) \
{ \
	.physical_base = p, \
	.start = s, \
	.end = e, \
}

#define dump_lane_pclk(s, n) \
	dump_reg32(USB_TCA_BASE, s + n * 0x400u, s + n * 0x400u + 0x28u), \
	dump_reg32(USB_TCA_BASE, s + n * 0x400u + 0x80u, s + n * 0x400u + 0x88u), \
	dump_reg32(USB_TCA_BASE, s + n * 0x400u + 0x100u, s + n * 0x400u + 0x188u), \
	dump_reg32(USB_TCA_BASE, s + n * 0x400u + 0x200u, s + n * 0x400u + 0x250u), \
	dump_reg32(USB_TCA_BASE, s + n * 0x400u + 0x2C0u, s + n * 0x400u + 0x2D8u), \
	dump_reg32(USB_TCA_BASE, s + n * 0x400u + 0x300u, s + n * 0x400u + 0x380u)

static struct combophy_dump_reg32 combophy_regs[] = {
	/* APBCLK */
	dump_reg32(USB_TCA_BASE, 0, 0x30u),
	dump_reg32(USB_TCA_BASE, 0x100u, 0x13Cu),
	dump_reg32(USB_TCA_BASE, 0x164u, 0x190u),
	dump_reg32(USB_TCA_BASE, 0x1ACu, 0x1B0u),
	dump_reg32(USB_TCA_BASE, 0x1C0u, 0x1C4u),
	dump_reg32(USB_TCA_BASE, 0x200u, 0x224u),
	dump_reg32(USB_TCA_BASE, 0x280u, 0x2A4u),
	dump_reg32(USB_TCA_BASE, 0x300u, 0x324u),
	dump_reg32(USB_TCA_BASE, 0x380u, 0x3A4u),

	/* USB_PCLK */
	dump_reg32(USB_TCA_BASE, 0xA00u, 0xAC4u),
	dump_reg32(USB_TCA_BASE, 0xB00u, 0xBCu),
	dump_reg32(USB_TCA_BASE, 0xB80u, 0xB9Cu),
	dump_reg32(USB_TCA_BASE, 0xBD0u, 0xBD4u),
	/* AUXCLK */
	dump_reg32(USB_TCA_BASE, 0x1A80u, 0x1A90u),
	dump_reg32(USB_TCA_BASE, 0x1AB0u, 0x1AB4u),
	dump_reg32(USB_TCA_BASE, 0x1AC0u, 0x1AC4u),
	dump_reg32(USB_TCA_BASE, 0x1AD0u, 0x1AE0u),
	dump_reg32(USB_TCA_BASE, 0x1B00u, 0x1B44u),
	/* LANE_PCLK */
	dump_lane_pclk(0x2000u, 0),
	dump_lane_pclk(0x2000u, 1),
	dump_lane_pclk(0x2000u, 2),
	dump_lane_pclk(0x2000u, 3),
	/* RXCLK */
	dump_reg32(USB_TCA_BASE, 0x4000u, 0x4034u),
	dump_reg32(USB_TCA_BASE, 0x4100u, 0x4134u),
	dump_reg32(USB_TCA_BASE, 0x4200u, 0x4234u),
	dump_reg32(USB_TCA_BASE, 0x4300u, 0x4334u),
	/* MISC CTRL */
	dump_reg32(USB_MISC_CTRL_BASE, 0, 0xA4u),
};

static void combophy_regdump(struct chip_combophy *combophy)
{
	struct phy_priv *priv = chip_combophy_to_phy_priv(combophy);
	struct combophy_dump_reg32 *regs = NULL;
	void __iomem *base = NULL;
	uint32_t reg;
	unsigned int i;

	pr_info("+\n");
	mutex_lock(&phy_mutex);
	if (!priv->phy_running) {
		mutex_unlock(&phy_mutex);
		return;
	}

	for (i = 0; i < ARRAY_SIZE(combophy_regs); i++) {
		regs = &combophy_regs[i];
		base = NULL;
		if (regs->physical_base == USB_TCA_BASE)
			base = priv->usb_tca;
		else if (regs->physical_base == USB_MISC_CTRL_BASE)
			base = priv->usb_misc_ctrl;

		if (!base)
			continue;

		for (reg = regs->start; reg < regs->end; reg += sizeof(reg))
			pr_info("0x%08x: 0x%08x\n", regs->physical_base + reg,
				readl(base + reg));
	}

	mutex_unlock(&phy_mutex);
	pr_info("-\n");
}

static int debugfs_show_combophy_regdump(struct seq_file *s, void *data)
{
	struct phy_priv *priv = s->private;
	struct combophy_dump_reg32 *regs = NULL;
	void __iomem *base = NULL;
	uint32_t reg;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(combophy_regs); i++) {
		regs = &combophy_regs[i];
		base = NULL;
		if (regs->physical_base == USB_TCA_BASE)
			base = priv->usb_tca;
		else if (regs->physical_base == USB_MISC_CTRL_BASE)
			base = priv->usb_misc_ctrl;
		if (!base)
			continue;
		for (reg = regs->start; reg < regs->end; reg += 4) {
			seq_printf(s, "0x%08x: 0x%08x\n",
				   regs->physical_base + reg,
				   readl(base + reg));
			if (seq_has_overflowed(s))
				return 0;
		}
	}
	return 0;
}

static int debugfs_open_combophy_regdump(struct inode *inode, struct file *file)
{
	return single_open(file, debugfs_show_combophy_regdump,
			   inode->i_private);
}

static const struct file_operations fops_combophy_regdump = {
	.open =		debugfs_open_combophy_regdump,
	.read =		seq_read,
	.llseek =	seq_lseek,
	.release =	single_release,
};

static void combophy_create_regdump_debugfs(struct phy_priv *priv)
{
	if (!priv->debug_dir)
		return;

	priv->regdump_file = debugfs_create_file("regdump", 0444,
						 priv->debug_dir,
						 priv,
						 &fops_combophy_regdump);
	if (!priv->regdump_file)
		pr_err("Can't create debugfs regdump\n");
}

static void combophy_destroy_regdump_debugfs(struct phy_priv *priv)
{
	debugfs_remove(priv->regdump_file);
}

static ssize_t debugfs_write_start_mcu(struct file *file,
				       const char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	bool *p_start_mcu = file->private_data;
	struct phy_priv *priv = container_of(p_start_mcu,
					     struct phy_priv, start_mcu);
	ssize_t ret;

	mutex_lock(&phy_mutex);
	if (!priv->phy_running)
		ret = debugfs_write_file_bool(file, user_buf, count, ppos);
	else
		ret = -EBUSY;
	mutex_unlock(&phy_mutex);

	return ret;
}

static const struct file_operations fops_start_mcu = {
	.read =		debugfs_read_file_bool,
	.write =	debugfs_write_start_mcu,
	.open =		simple_open,
	.llseek =	default_llseek,
};

static ssize_t debugfs_read_phy_running(struct file *file,
					char __user *user_buf,
					size_t count, loff_t *ppos)
{
	ssize_t ret;

	mutex_lock(&phy_mutex);
	ret = debugfs_read_file_bool(file, user_buf, count, ppos);
	mutex_unlock(&phy_mutex);

	return ret;
}

static const struct file_operations fops_phy_running = {
	.read =		debugfs_read_phy_running,
	.open =		simple_open,
	.llseek =	default_llseek,
};

#define combophy_reg_debugfs_create_x32(reg) \
	do { \
		file = debugfs_create_x32(#reg, 0644, \
					  priv->debug_dir, &priv->reg); \
		if (IS_ERR_OR_NULL(file)) \
			pr_err("failed to create "#reg"\n"); \
	} while (0)

static int combophy_register_debugfs(struct chip_combophy *combophy,
				     struct dentry *root)
{
	struct phy_priv *priv = chip_combophy_to_phy_priv(combophy);
	struct dentry *file = NULL;

	if (!root)
		return -EINVAL;

	priv->debug_dir = debugfs_create_dir("combophy", root);
	if (IS_ERR_OR_NULL(priv->debug_dir)) {
		pr_err("failed to create combophy dir\n");
		return -EFAULT;
	}

	combophy_reg_debugfs_create_x32(txdrvctrl);
	combophy_reg_debugfs_create_x32(txeqcoeff);
	combophy_reg_debugfs_create_x32(rxctrl0);
	combophy_reg_debugfs_create_x32(rxctrl);
	combophy_reg_debugfs_create_x32(rxeqactrl);
	combophy_reg_debugfs_create_x32(rxgsactrl);
	combophy_reg_debugfs_create_x32(rxlosctrl);
	combophy_reg_debugfs_create_x32(pll0ckgctrlr0);
	combophy_reg_debugfs_create_x32(termctrl);
	combophy_reg_debugfs_create_x32(pll0sscgcntr0);

	file = debugfs_create_file("start_mcu", 0644,
				  priv->debug_dir, &priv->start_mcu,
				  &fops_start_mcu);
	if (IS_ERR_OR_NULL(file))
		pr_err("failed to create start_mcu\n");

	file = debugfs_create_file("running", 0444,
				  priv->debug_dir, &priv->phy_running,
				  &fops_phy_running);
	if (IS_ERR_OR_NULL(file))
		pr_err("failed to create running\n");

	file = debugfs_create_bool("debug", 0644,
				  priv->debug_dir, &priv->debug);
	if (IS_ERR_OR_NULL(file))
		pr_err("failed to create debug\n");

	file = debugfs_create_bool("config_ssc", 0644,
				  priv->debug_dir, &priv->config_ssc);
	if (IS_ERR_OR_NULL(file))
		pr_err("failed to create config_ssc\n");

	combophy_mcu_register_debugfs(priv->debug_dir);

	return 0;
}
#else
#define combophy_register_debugfs NULL

static inline void combophy_regdump(struct chip_combophy *combophy) {  }
static inline void combophy_create_regdump_debugfs(struct phy_priv *priv) { }
static inline void combophy_destroy_regdump_debugfs(struct phy_priv *priv) { }
#endif /* CONFIG_HISI_DEBUG_FS */

static int combophy_set_mode(struct chip_combophy *combophy,
			     TCPC_MUX_CTRL_TYPE mode_type,
			     TYPEC_PLUG_ORIEN_E typec_orien)
{
	struct phy_priv *priv = chip_combophy_to_phy_priv(combophy);

	pr_info("set mode_type %d orien %d\n", mode_type, typec_orien);

	if (mode_type > TCPC_MUX_MODE_MAX ||
	    (typec_orien != TYPEC_ORIEN_POSITIVE &&
	     typec_orien != TYPEC_ORIEN_NEGATIVE))
		return -EINVAL;

	mutex_lock(&phy_mutex);
	priv->phy_mode = mode_type;
	priv->typec_orien = typec_orien;
	mutex_unlock(&phy_mutex);

	return 0;
}

static void combophy_config_phy_para(struct phy_priv *priv)
{
	writel(priv->txdrvctrl, PERLANE_TXDRVCTRL_LANE0(priv->usb_tca));
	writel(priv->txdrvctrl, PERLANE_TXDRVCTRL_LANE1(priv->usb_tca));
	writel(priv->txeqcoeff, PERLANE_TXEQCOEFF_LANE0(priv->usb_tca));
	writel(priv->txeqcoeff, PERLANE_TXEQCOEFF_LANE1(priv->usb_tca));
	writel(priv->rxgsactrl, PERLANE_RXGSACTRL_LANE0(priv->usb_tca));
	writel(priv->rxgsactrl, PERLANE_RXGSACTRL_LANE1(priv->usb_tca));
	writel(priv->rxeqactrl, PERLANE_RXEQACTRL_LANE0(priv->usb_tca));
	writel(priv->rxeqactrl, PERLANE_RXEQACTRL_LANE1(priv->usb_tca));
	writel(priv->rxctrl0, PERLANE_RXCTRL0_LANE0(priv->usb_tca));
	writel(priv->rxctrl0, PERLANE_RXCTRL0_LANE1(priv->usb_tca));
	writel(priv->rxctrl, PERLANE_RXCTRL_LANE0(priv->usb_tca));
	writel(priv->rxctrl, PERLANE_RXCTRL_LANE1(priv->usb_tca));
	writel(priv->rxlosctrl, PERLANE_RXLOSCTRL_LANE0(priv->usb_tca));
	writel(priv->rxlosctrl, PERLANE_RXLOSCTRL_LANE1(priv->usb_tca));
	writel(priv->pll0ckgctrlr0, USB_TCA_PLL0CKGCTRLR0(priv->usb_tca));
	writel(priv->pll0sscgcntr0, USB_TCA_PLL0SSCG_CNT_R0(priv->usb_tca));
}

static void set_disable_usb3(struct phy_priv *priv)
{
	uint32_t reg;

	pr_info("+\n");
	/* disable usb3 SS port */
	reg = readl(USBDP_USB_CTRL_CFG0(priv->usb_dp_ctrl));
	reg |= HOST_U3_PORT_DISABLE;
	writel(reg, USBDP_USB_CTRL_CFG0(priv->usb_dp_ctrl));

	/* set ss port num 0 */
	reg = readl(USBDP_USB_CTRL_CFG0(priv->usb_dp_ctrl));
	reg &= ~HOST_NUM_U3_PORT_MASK;
	writel(reg, USBDP_USB_CTRL_CFG0(priv->usb_dp_ctrl));

	udelay(100);
}

static void combophy_config_ssc(struct phy_priv *priv)
{
	uint32_t reg;

	reg = readl(USB_TCA_PLL0SSCG_CTRL(priv->usb_tca));
	if (priv->config_ssc)
		reg &= ~PLL0_SSCG_CNT_STEPSIZE_FORCE_0;
	else
		reg |= PLL0_SSCG_CNT_STEPSIZE_FORCE_0;
	writel(reg, USB_TCA_PLL0SSCG_CTRL(priv->usb_tca));
}

static int _combophy_init(struct phy_priv *priv)
{
	uint32_t reg;
	int ret;

	ret = clk_prepare_enable(priv->apb_clk);
	if (ret) {
		pr_err("clk_prepare_enable apb clk failed\n");
		return ret;
	}

	ret = clk_prepare_enable(priv->aux_clk);
	if (ret) {
		pr_err("clk_prepare_enable aux clk failed\n");
		clk_disable_unprepare(priv->apb_clk);
		return ret;
	}

	/* wait one-two ms */
	usleep_range(1000, 2000);

	/* combophy power on reset */
	reg = readl(USBDP_COMBOPHY_CTRL1(priv->usb_dp_ctrl));
	reg &= ~USBDP_COMBOPHY_CTRL1_POR;
	writel(reg, USBDP_COMBOPHY_CTRL1(priv->usb_dp_ctrl));

	/* force speed to GEN1 */
	reg = readl(USBDP_USB_CTRL_CFG0(priv->usb_dp_ctrl));
	reg |= HOST_FORCE_GEN1_SPEED;
	writel(reg, USBDP_USB_CTRL_CFG0(priv->usb_dp_ctrl));

	/* notify dp */
	ret = hisi_dptx_notify_switch();
	if (ret)
		pr_err("hisi_dptx_notify_switch failed %d\n", ret);

	/* Release combophy power on reset */
	reg = readl(USBDP_COMBOPHY_CTRL1(priv->usb_dp_ctrl));
	reg |= USBDP_COMBOPHY_CTRL1_POR;
	writel(reg, USBDP_COMBOPHY_CTRL1(priv->usb_dp_ctrl));

	/* Config mode */
	writel(soft_mode_to_phy_mode[priv->typec_orien][priv->phy_mode],
	       USB_TCA_PHY_MODE(priv->usb_tca));

	/* DP use 4 lane, disable usb3 */
	if (priv->phy_mode == TCPC_DP)
		set_disable_usb3(priv);

	/* Config SSC */
	combophy_config_ssc(priv);

	/* Release combophy phy reset */
	reg = readl(USB_MISC_CFGA0(priv->usb_misc_ctrl));
	reg &= ~USB3PHY_RESET;
	writel(reg, USB_MISC_CFGA0(priv->usb_misc_ctrl));

	return 0;
}

static int combophy_init(struct phy *phy)
{
	struct chip_combophy *combophy = phy_get_drvdata(phy);
	struct phy_priv *priv = NULL;
	int ret;

	pr_info("+\n");
	if (!combophy)
		return -ENODEV;

	priv = chip_combophy_to_phy_priv(combophy);

	mutex_lock(&phy_mutex);

	ret = misc_ctrl_init();
	if (ret)
		goto err_out;

	if (priv->test_stub)
		goto test_stub;

	ret = _combophy_init(priv);
	if (ret)
		goto err_exit_misc_ctrl;

	combophy_config_phy_para(priv);

test_stub:
	if (priv->start_mcu) {
		ret = combophy_mcu_init(g_usb_mcu_firmware,
					sizeof(g_usb_mcu_firmware));
		if (ret)
			pr_err("failed to start mcu %d\n", ret);
	}
	priv->phy_running = true;

	combophy_create_regdump_debugfs(priv);
	mutex_unlock(&phy_mutex);

	if (priv->debug)
		combophy_regdump(combophy);
	pr_info("-\n");

	return 0;
err_exit_misc_ctrl:
	misc_ctrl_exit();
err_out:
	mutex_unlock(&phy_mutex);
	pr_err("combophy init failed %d\n", ret);
	return ret;
}

static int combophy_exit(struct phy *phy)
{
	struct chip_combophy *combophy = phy_get_drvdata(phy);
	struct phy_priv *priv = NULL;

	pr_info("+\n");
	if (!combophy)
		return -ENODEV;

	priv = chip_combophy_to_phy_priv(combophy);

	mutex_lock(&phy_mutex);
	combophy_destroy_regdump_debugfs(priv);
	priv->phy_running = false;

	if (priv->start_mcu)
		combophy_mcu_exit();

	if (priv->test_stub)
		goto test_stub;

	/* AUX clk close */
	clk_disable_unprepare(priv->aux_clk);
	/* APB clk close */
	clk_disable_unprepare(priv->apb_clk);

test_stub:
	misc_ctrl_exit();

	mutex_unlock(&phy_mutex);
	pr_info("-\n");

	return 0;
}

static struct phy_ops combophy_ops = {
	.init	= combophy_init,
	.exit	= combophy_exit,
	.owner	= THIS_MODULE,
};

static void combophy_config_eqc(struct phy_priv *priv, uint32_t eqc)
{
	uint32_t reg;

	reg = readl(PERLANE_RXOFSTUPDTCTRL_LANE0(priv->usb_tca));
	reg |= RXOFSTUPDTCTRL_RXOFSTK_UPDTR;
	writel(reg, PERLANE_RXOFSTUPDTCTRL_LANE0(priv->usb_tca));
	reg = readl(PERLANE_RXOFSTUPDTCTRL_LANE1(priv->usb_tca));
	reg |= RXOFSTUPDTCTRL_RXOFSTK_UPDTR;
	writel(reg, PERLANE_RXOFSTUPDTCTRL_LANE1(priv->usb_tca));

	udelay(100);

	reg = readl(PERLANE_RXEQACTRL_LANE0(priv->usb_tca));
	reg &= ~RXCTRL_EQA_SEL_C_MASK;
	reg |= eqc;
	writel(reg, PERLANE_RXEQACTRL_LANE0(priv->usb_tca));
	reg = readl(PERLANE_RXEQACTRL_LANE1(priv->usb_tca));
	reg &= ~RXCTRL_EQA_SEL_C_MASK;
	reg |= eqc;
	writel(reg, PERLANE_RXEQACTRL_LANE1(priv->usb_tca));
}

static int combophy_reset(struct phy *phy)
{
	struct phy_priv *priv = phy_get_drvdata(phy);

	pr_info("+\n");
	mutex_lock(&phy_mutex);
	if (!priv->phy_running || priv->phy_mode == TCPC_DP) {
		pr_err("combophy usb is not running\n");
		mutex_unlock(&phy_mutex);
		return 0;
	}

	writel(priv->termctrl, AUXCLK_TERMCTRL_LANE0(priv->usb_tca));
	writel(priv->termctrl, AUXCLK_TERMCTRL_LANE1(priv->usb_tca));

	combophy_config_eqc(priv, RXCTRL_EQA_SEL_C_MASK & priv->rxeqactrl);

	mutex_unlock(&phy_mutex);
	pr_info("-\n");

	return 0;
}

static struct phy_ops combophy_config_ops = {
	.reset = combophy_reset,
	.owner	= THIS_MODULE,
};

static int map_regs_from_dts(struct platform_device *pdev,
			     struct phy_priv *priv)
{
	struct resource *res = NULL;
	struct device_node *np = NULL;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	priv->usb_tca = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(priv->usb_tca))
		return PTR_ERR(priv->usb_tca);

	/*
	 * map USB MISC CTRL region
	 */
	np = of_find_compatible_node(NULL, NULL, "hisilicon,usb-misc-ctrl");
	if (!np) {
		pr_err("get usb misc ctrl node failed!\n");
		return -EINVAL;
	}

	priv->usb_misc_ctrl = of_iomap(np, 0);
	if (!priv->usb_misc_ctrl) {
		pr_err("iomap usb misc ctrl reg_base failed!\n");
		return -ENOMEM;
	}

	/*
	 * map USB DP CTRL region
	 */
	np = of_find_compatible_node(NULL, NULL, "hisilicon,usb_dp_ctrl");
	if (!np) {
		pr_err("get usb dp ctrl node failed!\n");
		iounmap(priv->usb_misc_ctrl);
		priv->usb_misc_ctrl = NULL;
		return -EINVAL;
	}

	priv->usb_dp_ctrl = of_iomap(np, 0);
	if (!priv->usb_dp_ctrl) {
		pr_err("iomap usb dp ctrl reg_base failed!\n");
		iounmap(priv->usb_misc_ctrl);
		priv->usb_misc_ctrl = NULL;
		return -ENOMEM;
	}

	return 0;
}

static int get_clk_resource(struct phy_priv *priv)
{
	struct device *dev = priv->dev;

	/* get apb clk handler */
	priv->apb_clk = devm_clk_get(dev, "clk_usb31phy_apb");
	if (IS_ERR_OR_NULL(priv->apb_clk)) {
		pr_err("get apb clk failed\n");
		return -EINVAL;
	}

	/* get aux clk handler */
	priv->aux_clk = devm_clk_get(dev, "clk_hsdt1_eusb");
	if (IS_ERR_OR_NULL(priv->aux_clk)) {
		pr_err("get aux clk failed\n");
		return -EINVAL;
	}

	return 0;
}

#define of_get_combophy_param(x) \
do { \
	int ret; \
	ret = of_property_read_u32(np, #x, &priv->x); \
	if (ret) { \
		pr_err("read "#x" failed, ret %d\n", ret); \
		priv->x = combophy_default_##x; \
	} \
} while (0)

static void get_phy_param(struct device_node *np, struct phy_priv *priv)
{
	of_get_combophy_param(txdrvctrl);
	of_get_combophy_param(txeqcoeff);
	of_get_combophy_param(rxctrl0);
	of_get_combophy_param(rxctrl);
	of_get_combophy_param(rxlosctrl);
	of_get_combophy_param(rxgsactrl);
	of_get_combophy_param(rxeqactrl);
	of_get_combophy_param(pll0ckgctrlr0);
	of_get_combophy_param(termctrl);
	of_get_combophy_param(pll0sscgcntr0);
}

static int get_dts_resource(struct platform_device *pdev, struct phy_priv *priv)
{
	struct device_node *np = pdev->dev.of_node;
	int ret;

	ret = get_clk_resource(priv);
	if (ret)
		return ret;

	ret = map_regs_from_dts(pdev, priv);
	if (ret)
		return ret;

	get_phy_param(np, priv);

	priv->start_mcu = of_property_read_bool(np, "start-mcu");
	pr_info("start mcu %s\n", priv->start_mcu ? "true" : "false");

	priv->test_stub = of_property_read_bool(np, "test-stub");
	pr_info("test stub %s\n", priv->test_stub ? "true" : "false");

	priv->config_ssc = of_property_read_bool(np, "config-ssc");
	pr_info("config ssc %s\n", priv->config_ssc ? "true" : "false");

	priv->config_quirk_device = of_property_read_bool(np, "config-quirk-device");
	pr_info("config_quirk_device %s\n", priv->config_quirk_device ? "true" : "false");
	return 0;
}

static void put_dts_resource(struct phy_priv *priv)
{
	iounmap(priv->usb_misc_ctrl);
	iounmap(priv->usb_dp_ctrl);
}

static struct phy *combophy_of_xlate(struct device *dev,
				     struct of_phandle_args *args)
{
	struct phy_priv *priv = dev_get_drvdata(dev);

	pr_info("get phy %d\n", args->args[0]);

	if (!priv)
		return ERR_PTR(-ENODEV);

	switch (args->args[0]) {
	case PHY_HANDLE_FOR_HISI_USB:
		return priv->combophy.phy;
	case PHY_HANDLE_FOR_DWC3:
		return priv->phy;
	default:
		return ERR_PTR(-EINVAL);
	}
}

static int create_chip_combophy(struct phy_priv *priv)
{
	struct phy_provider *phy_provider = NULL;
	struct phy *phy = NULL;

	phy = devm_phy_create(priv->dev, priv->dev->of_node, &combophy_ops);
	if (IS_ERR(phy))
		return PTR_ERR(phy);

	priv->combophy.phy = phy;
	priv->combophy.set_mode = combophy_set_mode;
	priv->combophy.register_debugfs = combophy_register_debugfs;
	priv->combophy.regdump = combophy_regdump;
	phy_set_drvdata(phy, &priv->combophy);

	phy = devm_phy_create(priv->dev, priv->dev->of_node, &combophy_config_ops);
	if (IS_ERR(phy))
		return PTR_ERR(phy);

	priv->phy = phy;
	phy_set_drvdata(phy, priv);
	dev_set_drvdata(priv->dev, priv);

	phy_provider = devm_of_phy_provider_register(priv->dev,
						     combophy_of_xlate);
	if (IS_ERR(phy_provider))
		return PTR_ERR(phy_provider);

	return 0;
}

static void combophy_do_quirk_config(struct phy_priv *priv,
				     const struct quirk_config *config)
{
	pr_info("+\n");
	mutex_lock(&phy_mutex);
	if (!priv->phy_running) {
		mutex_unlock(&phy_mutex);
		return;
	}

	if (config->flag & QUIRK_CONFIG_EQC)
		combophy_config_eqc(priv, config->value);

	mutex_unlock(&phy_mutex);
	pr_info("-\n");
}

static int usb_notifier_fn(struct notifier_block *nb,
			   unsigned long action, void *data)
{
	struct phy_priv *priv = container_of(nb, struct phy_priv, usb_nb);
	struct usb_device *udev = data;

	pr_info("+\n");
	if (!udev) {
		pr_info("udev is null, just return\n");
		return 0;
	}

	/* match devices directly connect to roothub */
	if ((action == USB_DEVICE_ADD) &&
	    ((udev->parent != NULL) && (udev->parent->parent == NULL))) {
		const struct quirk_config *quirk_config = NULL;

		quirk_config = usb_detect_quirks(udev);
		if (!quirk_config)
			return 0;
		combophy_do_quirk_config(priv, quirk_config);
	}

	pr_info("-\n");
	return 0;
}

static void _combophy_register_usb_notify(struct phy_priv *priv)
{
	if (!priv->config_quirk_device)
		return;
	priv->usb_nb.notifier_call = usb_notifier_fn;
	usb_register_notify(&priv->usb_nb);
}

static void _combophy_unregister_usb_notify(struct phy_priv *priv)
{
	if (!priv->config_quirk_device)
		return;
	usb_unregister_notify(&priv->usb_nb);
	priv->usb_nb.notifier_call = NULL;
}

static int chip_combophy_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct phy_priv *priv = NULL;
	int ret;

	pr_info("+\n");

	if (!misc_ctrl_is_ready()) {
		pr_err("misc ctrl is not ready\n");
		return -EPROBE_DEFER;
	}

	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);
	pm_runtime_no_callbacks(dev);
	ret = pm_runtime_get_sync(dev);
	if (ret < 0) {
		pr_err("pm_runtime_get_sync failed\n");
		goto err_pm_put;
	}

	pm_runtime_forbid(dev);

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		ret = -ENOMEM;
		goto err_pm_allow;
	}

	priv->dev = dev;
	priv->phy_mode = TCPC_NC;
	priv->typec_orien = TYPEC_ORIEN_POSITIVE;
	priv->phy_running = false;
	priv->debug = false;

	ret = get_dts_resource(pdev, priv);
	if (ret) {
		pr_err("get_dts_resource failed\n");
		goto err_pm_allow;
	}

	ret = create_chip_combophy(priv);
	if (ret) {
		pr_err("create hisi combophy failed %d\n", ret);
		goto err_pm_allow;
	}

	_combophy_register_usb_notify(priv);
	pr_info("-\n");
	return 0;
err_pm_allow:
	pm_runtime_allow(dev);
err_pm_put:
	pm_runtime_put_sync(dev);
	pm_runtime_disable(dev);

	pr_err("ret %d\n", ret);
	return ret;
}

static int chip_combophy_remove(struct platform_device *pdev)
{
	struct phy_priv *priv = platform_get_drvdata(pdev);

	_combophy_unregister_usb_notify(priv);
	debugfs_remove_recursive(priv->debug_dir);
	put_dts_resource(priv);

	pm_runtime_allow(&pdev->dev);
	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_disable(&pdev->dev);

	return 0;
}

static const struct of_device_id chip_combophy_of_match[] = {
	{ .compatible = "hisilicon,combophy-phy", },
	{ }
};
MODULE_DEVICE_TABLE(of, chip_combophy_of_match);

static struct platform_driver chip_combophy_driver = {
	.probe	= chip_combophy_probe,
	.remove = chip_combophy_remove,
	.driver = {
		.name	= "chip-combophy-phy",
		.of_match_table	= chip_combophy_of_match,
	}
};
module_platform_driver(chip_combophy_driver);

MODULE_LICENSE("GPL v2");
