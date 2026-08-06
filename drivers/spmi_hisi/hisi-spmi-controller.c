/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * spmi_controller.c
 *
 * spmi controller
 *
 * This software is licensed under the terms of the GNU General Public
 * either version 2 of that License or (at your option) any later version.
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "hisi_spmi_dbgfs.h"
#include <linux/hisi-spmi.h>
#include <linux/of_hisi_spmi.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <pr_log.h>

#define PR_LOG_TAG SPMI_TAG

#define SPMI_CONTROLLER_NAME "spmi_controller"

/*
 * SPMI register addr
 */

#define SPMI_CHANNEL_OFFSET 0x0300
#define SPMI_SLAVE_OFFSET 0x20

#define SPMI_APB_SPMI_CMD_BASE_ADDR 0x0100

#define SPMI_APB_SPMI_WDATA0_BASE_ADDR 0x0104
#define SPMI_APB_SPMI_WDATA1_BASE_ADDR 0x0108
#define SPMI_APB_SPMI_WDATA2_BASE_ADDR 0x010c
#define SPMI_APB_SPMI_WDATA3_BASE_ADDR 0x0110

#define SPMI_APB_SPMI_STATUS_BASE_ADDR 0x0200

#define SPMI_APB_SPMI_RDATA0_BASE_ADDR 0x0204
#define SPMI_APB_SPMI_RDATA1_BASE_ADDR 0x0208
#define SPMI_APB_SPMI_RDATA2_BASE_ADDR 0x020c
#define SPMI_APB_SPMI_RDATA3_BASE_ADDR 0x0210

#define SPMI_PER_DATAREG_BYTE 4
/*
 * SPMI cmd register
 */
#define SPMI_APB_SPMI_CMD_EN BIT(31)
#define SPMI_APB_SPMI_CMD_TYPE_OFFSET 24
#define SPMI_APB_SPMI_CMD_LENGTH_OFFSET 20
#define SPMI_APB_SPMI_CMD_SLAVEID_OFFSET 16
#define SPMI_APB_SPMI_CMD_ADDR_OFFSET 0

#define tranverse32(X) ((((u32)(X) & 0xff000000) >> 24) | \
	(((u32)(X) & 0x00ff0000) >> 8) | \
	(((u32)(X) & 0x0000ff00) << 8) | (((u32)(X) & 0x000000ff) << 24))

#ifdef CONFIG_GCOV_KERNEL
#define STATIC
#else
#define STATIC static
#endif

/* Command Opcodes */
enum spmi_controller_cmd_op_code {
	SPMI_CMD_REG_ZERO_WRITE = 0,
	SPMI_CMD_REG_WRITE = 1,
	SPMI_CMD_REG_READ = 2,
	SPMI_CMD_EXT_REG_WRITE = 3,
	SPMI_CMD_EXT_REG_READ = 4,
	SPMI_CMD_EXT_REG_WRITE_L = 5,
	SPMI_CMD_EXT_REG_READ_L = 6,
	SPMI_CMD_REG_RESET = 7,
	SPMI_CMD_REG_SLEEP = 8,
	SPMI_CMD_REG_SHUTDOWN = 9,
	SPMI_CMD_REG_WAKEUP = 10,
	SPMI_CMD_ERROR_EXT = 11,
};

/*
 * SPMI status register
 */
#define SPMI_APB_TRANS_DONE (1 << 0)
#define SPMI_APB_TRANS_FAIL (1 << 2)

/* Command register fields */
#define SPMI_CONTROLLER_CMD_MAX_BYTE_COUNT 16

/* Maximum number of support PMIC peripherals */
#define SPMI_CONTROLLER_TIMEOUT_US 1000
#define SPMI_CONTROLLER_MAX_TRANS_BYTES 16

#define spmi_writel(dev, reg, addr) \
	do { \
		writel((reg), (addr)); \
	} while (0)

#define spmi_readl(dev, reg, addr) \
	do { \
		reg = readl(addr); \
	} while (0)

/*
 * @base base address of the PMIC Arbiter core registers.
 * @rdbase, @wrbase base address of the PMIC Arbiter read core registers.
 *     For HW-v1 these are equal to base.
 *     For HW-v2, the value is the same in eeraly probing, in order to read
 *     PMIC_ARB_CORE registers, then chnls, and obsrvr are set to
 *     PMIC_ARB_CORE_REGISTERS and PMIC_ARB_CORE_REGISTERS_OBS respectivly.
 * @intr base address of the SPMI interrupt control registers
 * @ppid_2_chnl_tbl lookup table f(SID, Periph-ID) -> channel num
 *      entry is only valid if corresponding bit is set in valid_ppid_bitmap.
 * @valid_ppid_bitmap bit is set only for valid ppids.
 * @fmt_cmd formats a command to be set into PMIC_ARBq_CHNLn_CMD
 * @chnl_ofst calculates offset of the base of a channel reg space
 * @ee execution environment id
 * @irq_acc0_init_val initial value of the interrupt accumulator at probe time.
 *      Use for an HW workaround. On handling interrupts, the first accumulator
 *      register will be compared against this value, and bits which are set at
 *      boot will be ignored.
 * @reserved_chnl entry of ppid_2_chnl_tbl that this driver should never touch.
 *      value is positive channel number or negative to mark it unused.
 */
struct spmi_controller_dev {
	struct spmi_controller	controller;
	struct device    	*dev;
	void __iomem		*base;
	spinlock_t		lock;
	u32			channel;
	u32			nr;
};

static int spmi_controller_wait_for_done(
	const struct spmi_controller_dev *ctrl_dev, const void __iomem *base,
	u8 sid, u16 addr)
{
	u32 status;
	u32 timeout = SPMI_CONTROLLER_TIMEOUT_US;
	u32 offset = SPMI_APB_SPMI_STATUS_BASE_ADDR +
		SPMI_CHANNEL_OFFSET * ctrl_dev->channel +
		SPMI_SLAVE_OFFSET * sid;

	while (timeout--) {
		spmi_readl(ctrl_dev->dev, status, base + offset);

		if (status & SPMI_APB_TRANS_DONE) {
			if (status & SPMI_APB_TRANS_FAIL) {
				dev_err(ctrl_dev->dev,
					"%s: transaction failed 0x%x\n",
					__func__, status);
				return -EIO;
			}
			return 0;
		}
		udelay(1);
	}

	dev_err(ctrl_dev->dev, "%s: timeout, status 0x%x\n", __func__, status);
	return -ETIMEDOUT;
}
static u32 spmi_controller_cmd(u8 op_code, u8 bc, u8 sid, u16 addr)
{
	u32 cmd;

	cmd = SPMI_APB_SPMI_CMD_EN | /* cmd_en */
		(u32)(op_code << SPMI_APB_SPMI_CMD_TYPE_OFFSET) | /* cmd_type */
		(u32)((bc - 1) << SPMI_APB_SPMI_CMD_LENGTH_OFFSET) | /* byte_cnt */
		(u32)((sid & 0xf) << SPMI_APB_SPMI_CMD_SLAVEID_OFFSET) | /* slvid */
		(u32)((addr & 0xffff)  << SPMI_APB_SPMI_CMD_ADDR_OFFSET); /* slave_addr */
	return cmd;
}

static u8 spmi_read_cmd_check_opc(u8 opc)
{
	u8 op_code;

	if (opc == SPMI_CMD_READ)
		op_code = SPMI_CMD_REG_READ;
	else if (opc == SPMI_CMD_EXT_READ)
		op_code = SPMI_CMD_EXT_REG_READ;
	else if (opc == SPMI_CMD_EXT_READL)
		op_code = SPMI_CMD_EXT_REG_READ_L;
	else
		op_code = SPMI_CMD_ERROR_EXT;

	return op_code;
}

static u8 spmi_write_cmd_check_opc(u8 opc)
{
	u8 op_code;

	if (opc == SPMI_CMD_WRITE)
		op_code = SPMI_CMD_REG_WRITE;
	else if (opc == SPMI_CMD_EXT_WRITE)
		op_code = SPMI_CMD_EXT_REG_WRITE;
	else if (opc == SPMI_CMD_EXT_WRITEL)
		op_code = SPMI_CMD_EXT_REG_WRITE_L;
	else
		op_code = SPMI_CMD_ERROR_EXT;

	return op_code;
}

static int spmi_read_cmd(struct spmi_controller *ctrl,
	u8 opc, u8 sid, u16 addr, u8 bc, u8 *buf)
{
	struct spmi_controller_dev *spmi_controller = spmi_get_ctrldata(ctrl);
	unsigned long flags;
	u32 cmd;
	u32 data;
	int rc;
	u8 op_code;
	u8 i;
	u32 chnl_ofst = SPMI_CHANNEL_OFFSET * spmi_controller->channel;

	if (bc > SPMI_CONTROLLER_MAX_TRANS_BYTES) {
		dev_err(spmi_controller->dev,
			"spmi_controller supports 1..%d bytes per trans, but:%u requested",
			SPMI_CONTROLLER_MAX_TRANS_BYTES, bc);
			return -EINVAL;
	}

	/* Check the opcode */
	op_code = spmi_read_cmd_check_opc(opc);
	if (op_code == SPMI_CMD_ERROR_EXT) {
		dev_err(spmi_controller->dev, "invalid read cmd 0x%x", opc);
		return -EINVAL;
	}

	cmd = spmi_controller_cmd(op_code, bc, sid, addr);

	spin_lock_irqsave(&spmi_controller->lock, flags);

	spmi_writel(spmi_controller->dev, cmd, spmi_controller->base +
		chnl_ofst + SPMI_APB_SPMI_CMD_BASE_ADDR);
	rc = spmi_controller_wait_for_done(spmi_controller,
			spmi_controller->base, sid, addr);
	if (rc)
		goto done;

	i = 0;
	do {
		spmi_readl(spmi_controller->dev, data, spmi_controller->base +
			chnl_ofst + SPMI_SLAVE_OFFSET * sid +
			SPMI_APB_SPMI_RDATA0_BASE_ADDR +
			i * SPMI_PER_DATAREG_BYTE);
		data = tranverse32(data);
		if ((bc - i * SPMI_PER_DATAREG_BYTE) >> 2) { /* multi byte cnt */
			memcpy(buf, &data, sizeof(data));
			buf += sizeof(data);
		} else {
			memcpy(buf, &data, bc % SPMI_PER_DATAREG_BYTE);
			buf += (bc % SPMI_PER_DATAREG_BYTE);
		}
		i++;
	} while (bc > i * SPMI_PER_DATAREG_BYTE);

done:
	spin_unlock_irqrestore(&spmi_controller->lock, flags);
	if (rc)
		dev_err(spmi_controller->dev,
			"spmi read wait timeout op: 0x%x sid: %u addr: 0x%x bc: %u\n",
			opc, sid, addr, bc + 1);
	return rc;
}

static int spmi_write_cmd(struct spmi_controller *ctrl,
	u8 opc, u8 sid, u16 addr, u8 bc, u8 *buf)
{
	struct spmi_controller_dev *spmi_controller = spmi_get_ctrldata(ctrl);
	unsigned long flags;
	u32 cmd;
	int rc;
	u8 op_code;
	u8 i;
	u32 data = 0;
	u32 chnl_ofst = SPMI_CHANNEL_OFFSET * spmi_controller->channel;

	if (bc > SPMI_CONTROLLER_MAX_TRANS_BYTES) {
		dev_err(spmi_controller->dev,
			"spmi_controller supports 1..%d bytes per trans, but:%u requested",
			SPMI_CONTROLLER_MAX_TRANS_BYTES, bc);
		return -EINVAL;
	}

	/* Check the opcode */
	op_code = spmi_write_cmd_check_opc(opc);
	if (op_code == SPMI_CMD_ERROR_EXT) {
		dev_err(spmi_controller->dev, "invalid write cmd 0x%x", opc);
		return -EINVAL;
	}

	cmd = spmi_controller_cmd(op_code, bc, sid, addr);

	/* Write data to FIFOs */
	spin_lock_irqsave(&spmi_controller->lock, flags);

	i = 0;
	do {
		memset(&data, 0, sizeof(data));
		if ((bc - i * SPMI_PER_DATAREG_BYTE) >> 2) { /* multi byte cnt */
			memcpy(&data, buf, sizeof(data));
			buf += sizeof(data);
		} else {
			memcpy(&data, buf, bc % SPMI_PER_DATAREG_BYTE);
			buf += (bc % SPMI_PER_DATAREG_BYTE);
		}

		data = tranverse32(data);
		spmi_writel(spmi_controller->dev, data, spmi_controller->base +
			chnl_ofst + SPMI_APB_SPMI_WDATA0_BASE_ADDR +
			SPMI_PER_DATAREG_BYTE * i);
		i++;
	} while (bc > i * SPMI_PER_DATAREG_BYTE);

	/* Start the transaction */
	spmi_writel(spmi_controller->dev, cmd, spmi_controller->base +
		chnl_ofst + SPMI_APB_SPMI_CMD_BASE_ADDR);

	rc = spmi_controller_wait_for_done(spmi_controller, spmi_controller->base, sid, addr);
	spin_unlock_irqrestore(&spmi_controller->lock, flags);

	if (rc)
		dev_err(spmi_controller->dev, "spmi write wait timeout op: 0x%x\n"
			"sid: %u addr: 0x%x bc: %u\n", opc, sid, addr, bc);

	return rc;
}

static int hisi_spmi_dt_parse_common(struct platform_device *pdev,
	struct spmi_controller_dev *spmi_controller)
{
	int ret;

	ret = of_property_read_u32(pdev->dev.of_node, "spmi-channel",
			&spmi_controller->channel);
	if (ret) {
		dev_err(&pdev->dev, "can not get chanel\n");
		return -ENODEV;
	}
	if (of_property_read_u32(pdev->dev.of_node, "spmi-nr",
			&spmi_controller->nr)) {
		dev_err(&pdev->dev, "can not get nr\n");
		spmi_controller->nr = spmi_controller->channel;
	}
	if (of_property_read_bool(pdev->dev.of_node, "spmi-always-sec")) {
		dev_err(&pdev->dev, "spmi-always-sec enable!\n");
		spmi_controller->controller.always_sec = true;
	}
	return ret;
}

static int spmi_controller_probe(struct platform_device *pdev)
{
	struct spmi_controller_dev *spmi_controller = NULL;
	struct resource *iores = NULL;
	int ret, ret1;

	spmi_controller = devm_kzalloc(&pdev->dev,
				sizeof(*spmi_controller), GFP_KERNEL);
	if (!spmi_controller) {
		dev_err(&pdev->dev, "can not allocate spmi_controller data\n");
		return -ENOMEM;
	}
	spmi_controller->dev = &pdev->dev;

	/* NOTE: driver uses the static register mapping */
	iores = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!iores) {
		dev_err(&pdev->dev, "can not get resource!\n");
		return -EINVAL;
	}
	spmi_controller->base = ioremap(iores->start, resource_size(iores));
	if (!spmi_controller->base) {
		dev_err(&pdev->dev, "can not remap base addr!\n");
		return -EADDRNOTAVAIL;
	}
	dev_dbg(&pdev->dev, "spmi_add_controller base addr= 0x%lx!\n",
		(unsigned long)spmi_controller->base);

	/* Get properties from the device tree */
	ret = hisi_spmi_dt_parse_common(pdev, spmi_controller);
	if (ret) {
		ret = -ENODEV;
		goto err_dt_parse;
	}
	platform_set_drvdata(pdev, spmi_controller);
	dev_set_drvdata(&spmi_controller->controller.dev, spmi_controller);
	spin_lock_init(&spmi_controller->lock);
	spin_lock_init(&spmi_controller->controller.sec_lock);

	spmi_controller->controller.nr = spmi_controller->nr;
	spmi_controller->controller.dev.parent = pdev->dev.parent;
	spmi_controller->controller.dev.of_node = of_node_get(pdev->dev.of_node);

	/* Callbacks */
	spmi_controller->controller.read_cmd = spmi_read_cmd;
	spmi_controller->controller.write_cmd = spmi_write_cmd;

	ret = spmi_add_controller(&spmi_controller->controller);
	if (ret) {
		dev_err(&pdev->dev, "spmi_add_controller failed!\n");
		goto err_add_controller;
	}

	/* Register device(s) from the device tree */
	ret = of_spmi_register_devices(&spmi_controller->controller);
	if (ret) {
		dev_err(&pdev->dev, "of_spmi_register_devices failed!\n");
		ret1 = spmi_del_controller(&spmi_controller->controller);
		if (ret1)
			dev_err(&pdev->dev, "spmi_del_controller failed!\n");
		goto err_add_controller;
	}
	return ret;

err_add_controller:
	platform_set_drvdata(pdev, NULL);
err_dt_parse:
	iounmap(spmi_controller->base);
	return ret;
}

static int spmi_controller_remove(struct platform_device *pdev)
{
	struct spmi_controller_dev *spmi_controller = platform_get_drvdata(pdev);
	int ret;

	platform_set_drvdata(pdev, NULL);
	ret = spmi_del_controller(&spmi_controller->controller);
	iounmap(spmi_controller->base);
	return ret;
}

static const struct of_device_id spmi_controller_match_table[] = {
	{ .compatible = "hisilicon,spmi-controller", },
	{}
};

static  const struct platform_device_id spmi_controller_id[] = {
	{ "spmi-controller", 0 },
	{}
};
static struct platform_driver spmi_controller_driver = {
	.probe		= spmi_controller_probe,
	.remove		= spmi_controller_remove,
	.driver		= {
		.name		= SPMI_CONTROLLER_NAME,
		.owner		= THIS_MODULE,
		.of_match_table = spmi_controller_match_table,
	},
	.id_table = spmi_controller_id,
};
STATIC int __init spmi_controller_init(void)
{
	return platform_driver_register(&spmi_controller_driver);
}
postcore_initcall(spmi_controller_init);

STATIC void __exit spmi_controller_exit(void)
{
	platform_driver_unregister(&spmi_controller_driver);
}
module_exit(spmi_controller_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Hisi spmi controller");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
