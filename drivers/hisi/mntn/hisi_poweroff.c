/*
 *
 * Power off or Reboot the hi3xxx device
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/kmsg_dump.h>
#include <linux/reboot.h>
#include <linux/delay.h>
#include <linux/mfd/hisi_pmic.h>
#include <pr_log.h>
#include <linux/pm.h>

#include <asm/cacheflush.h>
#include <asm/system_misc.h>

#include <soc_gpio_interface.h>
#include <soc_sctrl_interface.h>
#ifndef SOC_GPIO_GPIODIR_ADDR
#include <soc_gpio_interface.h>
#define set_high_16bit_enable(bit)  (1u << (bit + 16))
static unsigned int g_powerhold_gpio_bit = 0xFF;
#endif

#include "blackbox/rdr_print.h"
#include "pmic_interface.h"

#define PR_LOG_TAG POWEROFF_TAG
#define IOMAP_ADD_INDEX 0
#define POWERHOLD_PROTECT_BIT_OFFSET 16
#define POWERHOLD_GPIO_BIT_OFFSET 2

static void __iomem *powerhold_gpio_base = NULL;
static unsigned int g_powerhold_gpio_offset;
static int g_powerhold_protect_offset = 0xFFFF;
static unsigned int g_powerhold_protect_bit = 0xFF;
static bool g_gpio_v500_flag = false;

#ifdef CONFIG_HISI_BB
#include <linux/hisi/rdr_hisi_platform.h>
#include <rdr_inner.h>
#endif

static void __iomem *sysctrl_base = NULL;

static void chear_powerhold_protect(void)
{
	uintptr_t protect_val;

	if (g_powerhold_protect_offset != 0xFFFF) {
		/* we need clear protect bit and set mask bit */
		protect_val = readl((void *)sysctrl_base + g_powerhold_protect_offset);
		if ((protect_val & BIT(g_powerhold_protect_bit)) == 0)
			BB_PRINT_ERR("protect_offset = 0x%x, protect_bit = 0x%x"
				"powerhold protect is already cleared!\n",
				g_powerhold_protect_offset, g_powerhold_protect_bit);
		protect_val = (protect_val & (~BIT(g_powerhold_protect_bit)))
						| (BIT(g_powerhold_protect_bit + POWERHOLD_PROTECT_BIT_OFFSET));
		writel(protect_val, (void *)sysctrl_base + g_powerhold_protect_offset);
	}
}

static void hisi_pm_system_off(void)
{
	unsigned int out_dir;
#ifndef SOC_GPIO_GPIODIR_ADDR
	unsigned int data;
#endif

#ifdef CONFIG_HISI_BB
	set_reboot_reason(AP_S_COLDBOOT);
#endif

	while (1) {
		if (powerhold_gpio_base != NULL) {
			pr_info("system power off now\n");
			/* clear powerhold protect */
			chear_powerhold_protect();
#ifdef SOC_GPIO_GPIODIR_ADDR
			/* set direction */
			out_dir = readl(SOC_GPIO_GPIODIR_ADDR(powerhold_gpio_base));
			out_dir |= (1u << g_powerhold_gpio_offset);
			writel(out_dir, SOC_GPIO_GPIODIR_ADDR(powerhold_gpio_base));
			writel(0, powerhold_gpio_base + (1u << (g_powerhold_gpio_offset + POWERHOLD_GPIO_BIT_OFFSET)));
#else
			if (g_gpio_v500_flag && (g_powerhold_gpio_bit != 0xFF)) {
				/* set data_reg val */
				data = readl(SOC_HI_GPIO_V500_DATA_REG_LOW_ADDR(powerhold_gpio_base));
				data &= ~(unsigned int)(1u << g_powerhold_gpio_bit);
				data |= set_high_16bit_enable(g_powerhold_gpio_bit);
				writel(data, SOC_HI_GPIO_V500_DATA_REG_LOW_ADDR(powerhold_gpio_base));
				/* set direction */
				out_dir = readl(SOC_HI_GPIO_V500_INTR_CTRL_BIT15_ADDR(powerhold_gpio_base));
				out_dir |= (1u << SOC_HI_GPIO_V500_INTR_CTRL_BIT15_hw_sw_sel_15_START);
				out_dir |= set_high_16bit_enable(SOC_HI_GPIO_V500_INTR_CTRL_BIT15_hw_sw_sel_15_START);
				writel(out_dir, SOC_HI_GPIO_V500_INTR_CTRL_BIT15_ADDR(powerhold_gpio_base));
			}
#endif
			mdelay(1000); /* wait 1s before machine_restart */
			machine_restart("chargereboot");
		}
	}
}

#ifdef CONFIG_HISI_BB
void hisi_pm_system_reset_comm(const char *cmd)
{
	unsigned int i;
	unsigned int curr_reboot_type = UNKNOWN;
	const struct cmdword *reboot_reason_map = NULL;

	if (cmd == NULL || *cmd == '\0') {
		BB_PRINT_PN("%s cmd is null\n", __func__);
		cmd = "COLDBOOT";
	} else {
		BB_PRINT_PN("%s cmd is %s\n", __func__, cmd);
	}

	reboot_reason_map = get_reboot_reason_map();
	if (reboot_reason_map == NULL) {
		BB_PRINT_ERR("reboot_reason_map is NULL\n");
		return;
	}
	for (i = 0; i < get_reboot_reason_map_size(); i++) {
		if (!strncmp((char *)reboot_reason_map[i].name, cmd, sizeof(reboot_reason_map[i].name))) {
			curr_reboot_type = reboot_reason_map[i].num;
			break;
		}
	}
	if (curr_reboot_type == UNKNOWN) {
		curr_reboot_type = COLDBOOT;
		console_verbose();
		dump_stack();
	}
	set_reboot_reason(curr_reboot_type);
}


/*
 * Description:   Callback function registered with the machine_restart function.
 *                When the system is reset, the callback function is invoked
 * Input:         cmd  Reset Type
 * Output:        NA
 * Return:        NA
 */
static void hisi_pm_system_reset(enum reboot_mode mode, const char *cmd)
{
	hisi_pm_system_reset_comm(cmd);
	/* if EFI_RUNTIME_SERVICES enabled, then uefi is responsible of system reset */
	hisiap_nmi_notify_lpm3();
	BB_PRINT_PN("ap send nmi to lpm3, then goto dead loop\n");
	while (1);
}
#else
void hisi_pm_system_reset_comm(const char *cmd)
{
	if (cmd == NULL || *cmd == '\0') {
		BB_PRINT_PN("%s cmd is null\n", __func__);
		cmd = "COLDBOOT";
	} else {
		BB_PRINT_PN("%s cmd is %s\n", __func__, cmd);
	}
}

/*
 * Description:   Callback function registered with the machine_restart function.
 *                When the system is reset, the callback function is invoked
 * Input:         cmd  Reset Type
 * Output:        NA
 * Return:        NA
 */
static void hisi_pm_system_reset(enum reboot_mode mode, const char *cmd)
{
	hisi_pm_system_reset_comm(cmd);
	BB_PRINT_PN("ap send nmi to lpm3, then goto dead loop\n");
	while (1);
}
#endif

static void hisi_powerhold_init(void)
{
	struct device_node *np = NULL;
	unsigned int offset = 0;
	int ret;

	/* get powerhold gpio */
	np = of_find_compatible_node(NULL, NULL, "hisilicon,powerhold");
	if (np == NULL) {
		BB_PRINT_ERR("get powerhold np error !\n");
		return;
	}

	powerhold_gpio_base = of_iomap(np, IOMAP_ADD_INDEX);
	pr_info("powerhold_gpio_base = %pK !\n", powerhold_gpio_base);
	if (powerhold_gpio_base == NULL)
		BB_PRINT_ERR("%s: powerhold_gpio_base is NULL\n", __func__);

	ret = of_property_read_u32(np, "offset", (u32 *)&offset);
	if (ret) {
		BB_PRINT_ERR("get offset error !\n");
		if (powerhold_gpio_base) {
			iounmap(powerhold_gpio_base);
			powerhold_gpio_base = 0;
		}
	} else {
		pr_info("offset = 0x%x !\n", offset);
		g_powerhold_gpio_offset = offset;
	}

	ret = of_property_read_u32(np, "powerhold_protect_offset", (u32 *)&offset);
	if (ret) {
		BB_PRINT_ERR("no powerhold_protect_offset !\n");
	} else {
		pr_info("powerhold_protect_offset = 0x%x !\n", offset);
		g_powerhold_protect_offset = offset;
	}

	ret = of_property_read_u32(np, "powerhold_protect_bit", (u32 *)&offset);
	if (ret) {
		BB_PRINT_ERR("no powerhold_protect_bit !\n");
		g_powerhold_protect_offset = 0xFFFF;
	} else {
		pr_info("powerhold_protect_bit = 0x%x !\n", offset);
		g_powerhold_protect_bit = offset;
	}

	g_gpio_v500_flag = of_property_read_bool(np, "gpio_v500_flag");
	if (g_gpio_v500_flag) {
		pr_info("gpio version is v500 !\n");
		ret = of_property_read_u32(np, "powerhold_gpio_bit", (u32 *)&offset);
		if (ret) {
			BB_PRINT_ERR("no powerhold_gpio_bit !\n");
		} else {
			pr_info("powerhold_gpio_bit = %u !\n", offset);
			g_powerhold_protect_bit = offset;
		}
	}
}

static int hisi_sysctrl_init(void)
{
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");
	if (np == NULL) {
		BB_PRINT_ERR("%s: hisilicon,sysctrl No compatible node found\n", __func__);
		return -ENODEV;
	}

	sysctrl_base = of_iomap(np, IOMAP_ADD_INDEX);
	if (sysctrl_base == NULL) {
		BB_PRINT_ERR("%s: hisilicon,sysctrl_base is NULL\n", __func__);
		return -ENOMEM;
	}

	return 0;
}

static int hi3xxx_reset_probe(struct platform_device *pdev)
{
	int ret;

	hisi_powerhold_init();

	ret = hisi_sysctrl_init();
	if (ret)
		return ret;

	pm_power_off = hisi_pm_system_off;
	arm_pm_restart = hisi_pm_system_reset;

	return 0;
}

static int hi3xxx_reset_remove(struct platform_device *pdev)
{
	if (pm_power_off == hisi_pm_system_off)
		pm_power_off = NULL;

	if (arm_pm_restart == hisi_pm_system_reset)
		arm_pm_restart = NULL;

	return 0;
}

static const struct of_device_id of_hi3xxx_reset_match[] = {
	{ .compatible = "hisilicon,hi3xxx-reset", },
	{},
};

static struct platform_driver hi3xxx_reset_driver = {
	.probe = hi3xxx_reset_probe,
	.remove = hi3xxx_reset_remove,
	.driver = {
		.name = "hi3xxx-reset",
		.owner = THIS_MODULE,
		.of_match_table = of_hi3xxx_reset_match,
	},
};

static int __init hi3xxx_reset_init(void)
{
	return platform_driver_register(&hi3xxx_reset_driver);
}
module_init(hi3xxx_reset_init);

static void __exit hi3xxx_reset_exit(void)
{
	platform_driver_unregister(&hi3xxx_reset_driver);
}
module_exit(hi3xxx_reset_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("hi3xxx reset driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
