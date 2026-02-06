/*
 * hisi-combophy-mcu.c
 * combophy mcu driver
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
#include <linux/hisi/usb/hisi_usb_helper.h>
#include <linux/hisi/usb/hisi_usb_reg_cfg.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>

#undef pr_fmt
#define pr_fmt(fmt) "[COMBOPHY_MCU]%s: " fmt, __func__

#define USB_UCE_SCTRL_OFFSET	0xA000u
#define UCE_MCU_SCTRL(base)	((base) + USB_UCE_SCTRL_OFFSET)
#define UCE_MCU_POR_PC(base)	((base) + USB_UCE_SCTRL_OFFSET + 0x18)
#define UCE_CRG_GT_EN(base)	((base) + USB_UCE_SCTRL_OFFSET + 0x100)
#define UCE_CRG_RST_DIS(base)	((base) + USB_UCE_SCTRL_OFFSET + 0x114)
/* bit of UCE_MCU_SCTRL */
#define UCE_MCU_SCTRL_CORE_WAIT	BIT(3)
/* bit of UCE_CRG_GT_EN */
#define GT_UCEM3_HCLK_EN	BIT(0)
#define GT_UCEM3_DAPCLK_EN	BIT(1)
#define GT_CLK_H2HBRG_M_EN	BIT(2)
/* bit of UCE_CRG_RST_DIS */
#define RST_UCEM3_SYSRESET_REQ_DIS	BIT(0)
#define RST_UCE_CSSYS_RST_REQ_DIS	BIT(1)
#define RST_H2HBRG_M_REQ_DIS		BIT(8)

#define MCU_FIRMWARE_START(base)	(base)
#define MCU_FIRMWARE_MAX_SIZE		0x6000 /* 24KB */
#define MCU_REG_RUN_START		0x280
#define MCU_LOGBUF_START(base)		((base) + 0x5100)
#define MCU_LOGBUF_SIZE			0x400

#define CPU_MCU_INFO0(base)		((base) + 0)
#define CPU_MCU_INFO1(base)		((base) + 0x4)
#define CPU_MCU_INFO2(base)		((base) + 0x8)
#define CPU_MCU_INFO3(base)		((base) + 0xC)

#define MCU_FITST_CHECK_DELAY		HZ
#define MCU_PERIOD_CHECK_DELAY		(5 * HZ)

#define MCU_REG_INDEX			0
#define MCU_INFO_REG_INDEX		1

/*
 * CPU_MCU_INFO0 defination,
 * bit 0~3: MCU running status, bit 4~7: lanes is used or not
 * bit 8~25: Calibration func error records
 * bit 26~31: Calibration func sel
 */
union cpu_mcu_info0 {
	uint32_t value;
	struct {
		uint32_t mcu_status	: 4;
		uint32_t lanes_status : 4;
		uint32_t cal_func_sel : 24;
	} reg;
};

enum mcu_status {
	INIT_DONE = 0x1,
	PW_IRQ_START = 0x2,
	PW_IRQ_END = 0x3,
	EQ_IRQ_START = 0x4,
	EQ_IRQ_END = 0x5,
	CPU_IRQ_START = 0x6,
	CPU_IRQ_END = 0x7,
	PW_IRQ_ERR = 0x8,
	EQ_IRQ_ERR = 0x9,
	CPU_IRQ_ERR = 0xA,
	MCU_STAT_MAX = 0xF
};

enum lanes_status_bit {
	LANE0 = BIT(4),
	LANE1 = BIT(5),
	LANE2 = BIT(6),
	LANE3 = BIT(7)
};

enum calibration_select {
	USB_PLL_CAL_SEL = BIT(8),
	DP_PLL_CAL_SEL = BIT(9),
	RXTERM_K_CAL_SEL = BIT(10),
	TXTERM_K_CAL_SEL = BIT(11),
	OFST_K_CAL_SEL = BIT(12),
	TX_CTRL_CAL_SEL = BIT(13),
	RX_CTRL_EQ_ADAPT_CAL_SEL = BIT(14),
	CALIBRATION_ERR_MAX = BIT(31)
};

struct combophy_mcu {
	struct device *dev;
	void __iomem *regs;
	void __iomem *mcu_info;
	struct clk **clks;
	int num_clks;
	struct chip_usb_reg_cfg **unreset_cfgs;
	int num_unreset;
	struct chip_usb_reg_cfg **reset_cfgs;
	int num_reset;
	bool mcu_running;
	bool mcu_debug;
	bool test_stub;
	uint32_t calibration_func_select;
	struct delayed_work work;
	struct dentry *debug_dir;
	struct dentry *log_file;
};

static struct combophy_mcu *g_mcu;

#ifdef CONFIG_HISI_DEBUG_FS
static ssize_t debugfs_read_mcu_log(struct file *file, char __user *user_buf,
			       size_t count, loff_t *ppos)
{
	struct combophy_mcu *mcu = file->private_data;
	uint32_t mcu_log[MCU_LOGBUF_SIZE / sizeof(uint32_t)] = {0};
	unsigned int i;
	size_t len = min_t(size_t, count, MCU_LOGBUF_SIZE);

	if (!user_buf)
		return -EINVAL;
	for (i = 0; i < MCU_LOGBUF_SIZE / sizeof(uint32_t); i++)
		mcu_log[i] = readl(MCU_LOGBUF_START(mcu->regs) + i * sizeof(uint32_t));

	if (copy_to_user(user_buf, mcu_log, len))
		return -EFAULT;

	return len;
}

static const struct file_operations fops_mcu_log = {
	.read =		debugfs_read_mcu_log,
	.open =		simple_open,
	.llseek =	default_llseek,
};

static void combophy_mcu_create_log_debugfs(struct combophy_mcu *mcu)
{
	struct dentry *file = NULL;

	if (!mcu->mcu_debug)
		return;

	file = debugfs_create_file("mcu_log", 0444,
				   mcu->debug_dir, mcu, &fops_mcu_log);
	if (IS_ERR_OR_NULL(file)) {
		pr_err("failed to create mcu_log\n");
		return;
	}

	mcu->log_file = file;
}

static void combophy_mcu_destroy_log_debugfs(struct combophy_mcu *mcu)
{
	debugfs_remove(mcu->log_file);
	mcu->log_file = NULL;
}
#else
static inline void combophy_mcu_create_log_debugfs(struct combophy_mcu *mcu)
{
}
static inline void combophy_mcu_destroy_log_debugfs(struct combophy_mcu *mcu)
{
}
#endif /* CONFIG_HISI_DEBUG_FS */

static void wait_mcu_running(struct combophy_mcu *mcu)
{
	union cpu_mcu_info0 info0;
	int timeout = 100;

	while (timeout--) {
		/* wait one ms */
		usleep_range(1000, 1000);
		info0.value = readl(CPU_MCU_INFO0(mcu->mcu_info));
		if (info0.reg.mcu_status == INIT_DONE)
			break;
	}
	if (timeout <= 0)
		pr_err("wait for mcu running timeout\n");
}

int combophy_mcu_init(const void *firmware, size_t size)
{
	u32 reg;
	int ret;
	union cpu_mcu_info0 info0 = {0};
	struct combophy_mcu *mcu = g_mcu;

	pr_info("+\n");
	if (!mcu)
		return -ENODEV;

	if (size > MCU_FIRMWARE_MAX_SIZE)
		return -EINVAL;

	ret = chip_usb_init_clks(mcu->clks, mcu->num_clks);
	if (ret) {
		pr_err("init clks failed\n");
		return ret;
	}

	ret = chip_usb_reg_write_array(mcu->unreset_cfgs, mcu->num_unreset);
	if (ret) {
		pr_err("do unreset failed\n");
		goto mcu_clk_close;
	}

	if (mcu->test_stub)
		goto test_stub;

	/* MCU inner clk open */
	writel(GT_UCEM3_HCLK_EN | GT_UCEM3_DAPCLK_EN | GT_CLK_H2HBRG_M_EN,
	       UCE_CRG_GT_EN(mcu->regs));

	/* MCU inner unreset */
	writel(RST_UCEM3_SYSRESET_REQ_DIS | RST_UCE_CSSYS_RST_REQ_DIS |
	       RST_H2HBRG_M_REQ_DIS,
	       UCE_CRG_RST_DIS(mcu->regs));

	udelay(10);

	/* firmware update */
	memcpy(MCU_FIRMWARE_START(mcu->regs), firmware, size);

	/*
	 * config mcu start address
	 */
	writel(MCU_REG_RUN_START, UCE_MCU_POR_PC(mcu->regs));

	/* init info0 */
	info0.reg.cal_func_sel = mcu->calibration_func_select;
	writel(info0.value, CPU_MCU_INFO0(mcu->mcu_info));

	/* config mcu exit core-wait */
	reg = readl(UCE_MCU_SCTRL(mcu->regs));
	reg &= ~UCE_MCU_SCTRL_CORE_WAIT;
	writel(reg, UCE_MCU_SCTRL(mcu->regs));

	wait_mcu_running(mcu);
test_stub:
	mcu->mcu_running = true;

	combophy_mcu_create_log_debugfs(mcu);

	if (queue_delayed_work(system_power_efficient_wq,
				&mcu->work,
				MCU_FITST_CHECK_DELAY))
		pr_info("queue mcu check work success\n");

	pr_info("-\n");
	return 0;

mcu_clk_close:
	chip_usb_shutdown_clks(mcu->clks, mcu->num_clks);

	pr_info("init error %d\n", ret);
	return ret;
}

void combophy_mcu_exit(void)
{
	struct combophy_mcu *mcu = g_mcu;
	int ret;

	pr_info("+\n");
	if (!mcu)
		return;

	combophy_mcu_destroy_log_debugfs(mcu);

	if (cancel_delayed_work_sync(&mcu->work))
		pr_info("cancel mcu check work success\n");

	mcu->mcu_running = false;

	ret = chip_usb_reg_write_array(mcu->reset_cfgs, mcu->num_reset);
	if (ret)
		pr_err("do reset failed\n");

	chip_usb_shutdown_clks(mcu->clks, mcu->num_clks);
	pr_info("-\n");
}

static void combophy_mcu_check_work(struct work_struct *ws)
{
	struct combophy_mcu *mcu = container_of(ws, struct combophy_mcu,
						work.work);
	union cpu_mcu_info0 info0 = {0};
	uint32_t reg;
	static uint32_t info1;

	if (!mcu->mcu_running)
		return;

	info0.value = readl(CPU_MCU_INFO0(mcu->mcu_info));
	if (info0.reg.mcu_status != PW_IRQ_END &&
	    info0.reg.mcu_status != EQ_IRQ_END)
		pr_err("MCU status error %u\n", info0.reg.mcu_status);

	if (!mcu->mcu_debug)
		return;

	reg = readl(CPU_MCU_INFO1(mcu->mcu_info));
	if (reg != info1) {
		info1 = reg;
		pr_err("MCU Info1 0x%x\n", reg);
	}

	if (queue_delayed_work(system_power_efficient_wq,
				&mcu->work,
				MCU_PERIOD_CHECK_DELAY))
		pr_info("queue mcu check work success\n");
}

void combophy_mcu_register_debugfs(struct dentry *root)
{
	struct combophy_mcu *mcu = g_mcu;
	struct dentry *file = NULL;

	if (!root || !mcu)
		return;

	mcu->debug_dir = debugfs_create_dir("mcu", root);
	if (IS_ERR_OR_NULL(mcu->debug_dir)) {
		pr_err("failed to create mcu dir\n");
		return;
	}

	file = debugfs_create_x32("calibration_func_select", 0644,
				  mcu->debug_dir,
				  &mcu->calibration_func_select);
	if (IS_ERR_OR_NULL(file)) {
		pr_err("failed to create calibration_func_select\n");
		return;
	}

	file = debugfs_create_bool("mcu_debug", 0644,
				   mcu->debug_dir, &mcu->mcu_debug);
	if (IS_ERR_OR_NULL(file)) {
		pr_err("failed to create mcu_debug\n");
		return;
	}

	file = debugfs_create_bool("mcu_running", 0444,
				   mcu->debug_dir, &mcu->mcu_running);
	if (IS_ERR_OR_NULL(file)) {
		pr_err("failed to create mcu_running\n");
		return;
	}
}

static int combophy_mcu_get_of_res(struct platform_device *pdev,
				   struct combophy_mcu *mcu)
{
	struct resource *res = NULL;
	int ret;

	res = platform_get_resource(pdev, IORESOURCE_MEM, MCU_REG_INDEX);
	mcu->regs = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(mcu->regs)) {
		pr_err("failed to iomap MCU REG\n");
		return PTR_ERR(mcu->regs);
	}

	mcu->mcu_info = of_iomap(pdev->dev.of_node, MCU_INFO_REG_INDEX);
	if (!mcu->mcu_info) {
		pr_err("failed to iomap MCU INFO REG\n");
		return -ENOMEM;
	}

	ret = devm_chip_usb_get_clks(&pdev->dev, &mcu->clks, &mcu->num_clks);
	if (ret) {
		pr_err("get clks failed\n");
		goto unmap_mcu_info;
	}

	ret = get_chip_usb_reg_cfg_array(&pdev->dev, "mcu-unresets",
						 &mcu->unreset_cfgs,
						 &mcu->num_unreset);
	if (ret) {
		pr_err("get mcu-unresets failed\n");
		goto unmap_mcu_info;
	}

	ret = get_chip_usb_reg_cfg_array(&pdev->dev, "mcu-resets",
						 &mcu->reset_cfgs,
						 &mcu->num_reset);
	if (ret) {
		pr_err("get mcu-resets failed\n");
		goto free_unresets;
	}

	mcu->test_stub = of_property_read_bool(pdev->dev.of_node, "test-stub");
	pr_info("test stub %s\n", mcu->test_stub ? "true" : "false");

	return 0;
unmap_mcu_info:
	iounmap(mcu->mcu_info);
free_unresets:
	free_chip_usb_reg_cfg_array(mcu->unreset_cfgs, mcu->num_unreset);
	mcu->unreset_cfgs = NULL;
	mcu->num_unreset = 0;
	return ret;
}

static void combophy_mcu_free_of_res(struct combophy_mcu *mcu)
{
	iounmap(mcu->mcu_info);
	free_chip_usb_reg_cfg_array(mcu->unreset_cfgs, mcu->num_unreset);
	free_chip_usb_reg_cfg_array(mcu->reset_cfgs, mcu->num_reset);
}

static int combophy_mcu_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct combophy_mcu *mcu = NULL;
	int ret;

	pr_info("+\n");

	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);
	pm_runtime_no_callbacks(dev);
	ret = pm_runtime_get_sync(dev);
	if (ret < 0) {
		pr_err("pm_runtime_get_sync failed\n");
		goto err_pm_put;
	}

	pm_runtime_forbid(dev);

	mcu = devm_kzalloc(dev, sizeof(*mcu), GFP_KERNEL);
	if (!mcu) {
		ret = -ENOMEM;
		goto err_pm_allow;
	}

	mcu->dev = dev;
	mcu->mcu_running = false;
	mcu->calibration_func_select = 0;
	mcu->mcu_debug = false;
	ret = combophy_mcu_get_of_res(pdev, mcu);
	if (ret) {
		pr_err("get of res error %d\n", ret);
		goto err_pm_allow;
	}

	INIT_DELAYED_WORK(&mcu->work, combophy_mcu_check_work);

	platform_set_drvdata(pdev, mcu);
	g_mcu = mcu;

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

static int combophy_mcu_remove(struct platform_device *pdev)
{
	struct combophy_mcu *mcu = platform_get_drvdata(pdev);

	if (mcu->mcu_running)
		return -EBUSY;

	debugfs_remove_recursive(mcu->debug_dir);
	combophy_mcu_free_of_res(mcu);

	pm_runtime_allow(&pdev->dev);
	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_disable(&pdev->dev);

	g_mcu = NULL;

	return 0;
}

static const struct of_device_id combophy_mcu_of_match[] = {
	{ .compatible = "hisilicon,combophy-mcu", },
	{ }
};
MODULE_DEVICE_TABLE(of, combophy_mcu_of_match);

static struct platform_driver combophy_mcu_driver = {
	.probe	= combophy_mcu_probe,
	.remove = combophy_mcu_remove,
	.driver = {
		.name	= "combophy-mcu",
		.of_match_table	= combophy_mcu_of_match,
	}
};
module_platform_driver(combophy_mcu_driver);

MODULE_AUTHOR("Yu Chen <chenyu56@huawei.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Hilisicon ComboPHY MCU Driver");
