/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * freq_autodown.c
 *
 * bus automatic frequency reduction
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "freq_autodown.h"
#include <securec.h>

static struct freq_autodown_desc *g_freq_autodown;

static unsigned int crgperiph_regu_read(unsigned int reg)
{
	unsigned int read_value;

	if (!g_freq_autodown->crgperi_base_addr)
		return (unsigned int)(-1);
	read_value = readl(g_freq_autodown->crgperi_base_addr + reg);

	return read_value;
}
static void crgperiph_regu_write(unsigned int val, unsigned int reg)
{
	if (!g_freq_autodown->crgperi_base_addr)
		return;
	writel(val, g_freq_autodown->crgperi_base_addr + reg);
}
static unsigned int mediacrg_regu_read(unsigned int reg)
{
	unsigned int read_value;

	if (!g_freq_autodown->media_crg_base_addr)
		return (unsigned int)(-1);
	read_value = readl(g_freq_autodown->media_crg_base_addr + reg);

	return read_value;
}
static void mediacrg_regu_write(unsigned int val, unsigned int reg)
{
	if (!g_freq_autodown->media_crg_base_addr)
		return;
	writel(val, g_freq_autodown->media_crg_base_addr + reg);
}

static unsigned int media2crg_regu_read(unsigned int reg)
{
	unsigned int read_value;

	if (!g_freq_autodown->media2_crg_base_addr)
		return (unsigned int)(-1);
	read_value = readl(g_freq_autodown->media2_crg_base_addr + reg);
	return read_value;
}

static void media2crg_regu_write(unsigned int val, unsigned int reg)
{
	if (!g_freq_autodown->media2_crg_base_addr)
		return;
	writel(val, g_freq_autodown->media2_crg_base_addr + reg);
}

static void freq_autodown_set_bypass(unsigned int bypass_reg,
	unsigned int bypass_bit, unsigned int bypass_addr_flag, int enable_flag)
{
	u32 bypass_reg_value;

	if (enable_flag == ENABLE)
		AUTOFS_DEBUG("enable");
	else
		AUTOFS_DEBUG("disable");
	AUTOFS_DEBUG("bypass addr flag = 0x%x\n", bypass_addr_flag);

	if (bypass_addr_flag == MEDIA_CRG_BASE_ADDR) {
		bypass_reg_value = mediacrg_regu_read(bypass_reg);
		AUTOFS_DEBUG("bypass_reg_value before = 0x%x\n", bypass_reg_value);
		if (enable_flag == ENABLE)
			bypass_reg_value |= BIT(bypass_bit);
		else
			bypass_reg_value &= (~BIT(bypass_bit));
		mediacrg_regu_write(bypass_reg_value, bypass_reg);
		AUTOFS_DEBUG("bypass_reg_value end = 0x%x\n",
			mediacrg_regu_read(bypass_reg));
	} else if (bypass_addr_flag == CRG_PERI_BASE_ADDR) {
		bypass_reg_value = crgperiph_regu_read(bypass_reg);
		AUTOFS_DEBUG("bypass_reg_value before = 0x%x\n", bypass_reg_value);
		if (enable_flag == ENABLE)
			bypass_reg_value |= BIT(bypass_bit);
		else
			bypass_reg_value &= (~BIT(bypass_bit));
		crgperiph_regu_write(bypass_reg_value, bypass_reg);
		AUTOFS_DEBUG("bypass_reg_value end = 0x%x\n",
			crgperiph_regu_read(bypass_reg));
	} else if (bypass_addr_flag == MEDIA2_CRG_BASE_ADDR) {
		bypass_reg_value = media2crg_regu_read(bypass_reg);
		AUTOFS_DEBUG("bypass_reg_value before = 0x%x\n", bypass_reg_value);
		if (enable_flag == ENABLE)
			bypass_reg_value |= BIT(bypass_bit);
		else
			bypass_reg_value &= (~BIT(bypass_bit));
		media2crg_regu_write(bypass_reg_value, bypass_reg);
		AUTOFS_DEBUG("bypass_reg_value end = 0x%x\n",
			media2crg_regu_read(bypass_reg));
	}
}

static void freq_autodown_set_clock(unsigned int clock_reg,
	unsigned int clock_bit, unsigned int clock_addr_flag, int enable_flag)
{
	if (enable_flag == ENABLE)
		AUTOFS_DEBUG("enable");
	else
		AUTOFS_DEBUG("disable");
	AUTOFS_DEBUG("clock addr flag = 0x%x\n", clock_addr_flag);

	if (clock_addr_flag == MEDIA_CRG_BASE_ADDR) {
		if (enable_flag == ENABLE)
			mediacrg_regu_write(BIT(clock_bit), clock_reg);
		else
			mediacrg_regu_write(BIT(clock_bit),
				clock_reg + MEDIA_CRG_ADDR_OFFSET);
		AUTOFS_DEBUG("clock_bit[0x%lx] clk_reg_value end = 0x%x\n",
			BIT(clock_bit),
			mediacrg_regu_read(clock_reg + MEDIA_CRG_ADDR_STATE));
	} else if (clock_addr_flag == CRG_PERI_BASE_ADDR) {
		if (enable_flag == ENABLE)
			crgperiph_regu_write(BIT(clock_bit), clock_reg);
		else
			crgperiph_regu_write(BIT(clock_bit),
				clock_reg + CRGPERI_ADDR_OFFSET);
		AUTOFS_DEBUG("clock_bit[0x%lx] clk_reg_value end = 0x%x\n",
			BIT(clock_bit),
			crgperiph_regu_read(clock_reg + CRGPERI_ADDR_STATE));
	} else if (clock_addr_flag == MEDIA2_CRG_BASE_ADDR) {
		if (enable_flag == ENABLE)
			media2crg_regu_write(BIT(clock_bit), clock_reg);
		else
			media2crg_regu_write(BIT(clock_bit),
				clock_reg + MEDIA2_CRG_ADDR_OFFSET);
		AUTOFS_DEBUG("clock_bit[0x%lx] clk_reg_value end = 0x%x\n",
			BIT(clock_bit),
			media2crg_regu_read(clock_reg + MEDIA2_CRG_ADDR_STATE));
	}
}

static void freq_autodown_control(u32 control_flag, unsigned int index)
{
	unsigned int bypass_reg = g_freq_autodown->freq_info[index].disable_bypass_reg;
	unsigned int bypass_bit = g_freq_autodown->freq_info[index].disable_bypass_bit;
	unsigned int bypass_addr_flag = g_freq_autodown->freq_info[index].disable_bypass_addr_flag;
	unsigned int clock_reg = g_freq_autodown->freq_info[index].enable_clock_reg;
	unsigned int clock_bit = g_freq_autodown->freq_info[index].enable_clock_bit;
	unsigned int clock_addr_flag = g_freq_autodown->freq_info[index].enable_clock_addr_flag;

	AUTOFS_DEBUG("Info:bypass_reg=0x%x,bypass_bit=%u,flag0 =%u,clock_reg=0x%x,clock_bit=%u,flag1=%u\n",
		bypass_reg, bypass_bit, bypass_addr_flag, clock_reg, clock_bit, clock_addr_flag);

	if (control_flag == ENABLE) {
		if (clock_reg != INVALID_REG_ADDR) {
			/* disable bypass */
			freq_autodown_set_bypass(bypass_reg, bypass_bit,
				bypass_addr_flag, DISABLE);
			/* enable clock */
			freq_autodown_set_clock(clock_reg, clock_bit,
				clock_addr_flag, ENABLE);
		} else {
			/* disable bypass */
			AUTOFS_DEBUG("reg[0x%lx], bypass_reg_value before = 0x%x\n",
				(BIT(bypass_bit) | BIT_MSK(bypass_bit)),
				mediacrg_regu_read(bypass_reg));
			mediacrg_regu_write(BIT_MSK(bypass_bit), bypass_reg);
			AUTOFS_DEBUG("bypass_reg_value end = 0x%x\n",
				mediacrg_regu_read(bypass_reg));
		}
	} else if (control_flag == DISABLE) {
		if (clock_reg != INVALID_REG_ADDR) {
			/* disable clock */
			freq_autodown_set_clock(clock_reg, clock_bit,
				clock_addr_flag, DISABLE);
			/* enable bypass */
			freq_autodown_set_bypass(bypass_reg, bypass_bit,
				bypass_addr_flag, ENABLE);
		} else {
			/* enable bypass */
			AUTOFS_DEBUG("reg[0x%lx], bypass_reg_value before = 0x%x\n",
				BIT_MSK(bypass_bit),
				mediacrg_regu_read(bypass_reg));
			mediacrg_regu_write((BIT(bypass_bit) | BIT_MSK(bypass_bit)),
				bypass_reg);
			AUTOFS_DEBUG("bypass_reg_value end = 0x%x\n",
				mediacrg_regu_read(bypass_reg));
		}
	} else {
		/* not control */
	}
}

int freq_autodown_clk_set(const char *freq_name, u32 control_flag)
{
	unsigned int index;

	if (!freq_name)
		return -EINVAL;
	if (!g_freq_autodown)
		return -ENODEV;
	if (g_freq_autodown->freq_autodown_state == OPEN) {
		AUTOFS_DEBUG("freq_name = %s, control state = %u\n", freq_name,
			control_flag);
		spin_lock(&g_freq_autodown->lock);
		for (index = 0; index < g_freq_autodown->freq_autodown_num; index++) {
			if (!strcmp(freq_name, g_freq_autodown->freq_autodown_name[index])) {
				AUTOFS_DEBUG("name[%s]:", freq_name);
				freq_autodown_control(control_flag, index);
			}
		}
		spin_unlock(&g_freq_autodown->lock);
	}
	return 0;
}
EXPORT_SYMBOL(freq_autodown_clk_set);

static int freq_autodown_get_value(struct freq_autodown_info *freq_info,
	unsigned int index, struct device_node *root, struct device *dev)
{
	unsigned int register_info[3] = {0}; /* address, bit, base */
	s32 ret;

	/* get freq autodown disable bypass value */
	ret = of_property_read_u32_array(root, "freq_autodown_disable_bypass",
		register_info, FREQ_AUTODOWN_DTS_REG_NUM);
	if (ret) {
		dev_err(dev, "no freq_autodown_disable_bypass property set\n");
		return -EINVAL;
	}
	freq_info->disable_bypass_reg = register_info[0];
	freq_info->disable_bypass_bit = register_info[1];
	freq_info->disable_bypass_addr_flag = register_info[2];
	AUTOFS_DEBUG("index[%u] disable_bypass_reg = 0x%x\n", index,
		freq_info->disable_bypass_reg);
	AUTOFS_DEBUG("index[%u] disable_bypass_bit = 0x%x\n", index,
		freq_info->disable_bypass_bit);
	AUTOFS_DEBUG("index[%u] disable_bypass_addr_flag = %u\n", index,
		freq_info->disable_bypass_addr_flag);

	/* get freq autodown enable clock value */
	ret = of_property_read_u32_array(root, "freq_autodown_enable_clock",
		register_info, FREQ_AUTODOWN_DTS_REG_NUM);
	if (ret) {
		dev_err(dev, "no freq_autodown_enable_clock property set\n");
		return -EINVAL;
	}
	freq_info->enable_clock_reg = register_info[0];
	freq_info->enable_clock_bit = register_info[1];
	freq_info->enable_clock_addr_flag = register_info[2];
	AUTOFS_DEBUG("index[%u] enable_clock_reg = 0x%x\n", index,
		freq_info->enable_clock_reg);
	AUTOFS_DEBUG("index[%u] enable_clock_bit = 0x%x\n", index,
		freq_info->enable_clock_bit);
	AUTOFS_DEBUG("index[%u] enable_clock_addr_flag = %u\n", index,
		freq_info->enable_clock_addr_flag);

	return 0;
}

static int freq_autodown_get_info(struct freq_autodown_desc *freq_autodown,
	struct device *dev, struct device_node *np)
{
	struct device_node *root = NULL;
	unsigned int index;
	char compatible_string[FREQ_AUTODOWN_DTS_ATTR_LEN] = {0};
	s32 ret;
	unsigned int bit;

	/* get freq autodown name */
	for (bit = 0; bit < freq_autodown->freq_autodown_num; bit++) {
		ret = of_property_read_string_index(np, "hisilicon,freq-autodown-names",
			bit, (const char **)&freq_autodown->freq_autodown_name[bit]);
		if (ret != 0) {
			pr_err("failed to get freq autodown names! ret=%d\n",
				ret);
			return -EINVAL;
		}
		AUTOFS_DEBUG("hisilicon,freq-autodown-names[%u] = %s\n", bit,
			freq_autodown->freq_autodown_name[bit]);
	}
	/* get freq autodown info */
	for (index = 0; index < freq_autodown->freq_autodown_num; index++) {
		ret = snprintf_s(compatible_string, FREQ_AUTODOWN_DTS_ATTR_LEN,
			FREQ_AUTODOWN_DTS_ATTR_LEN - 1,
			"hisilicon,hisi-freq-autodown-%s",
			freq_autodown->freq_autodown_name[index]);
		if (ret == -1)
			pr_err("%s snprintf_s failed!\n", __func__);
		root = of_find_compatible_node(np, NULL,
			(const char *)compatible_string);
		if (!root) {
			dev_err(dev, "[%s]no %s root node\n", __func__,
				compatible_string);
			return -ENODEV;
		}

		ret = freq_autodown_get_value(&(freq_autodown->freq_info[index]),
			index, root, dev);
		if (ret != 0)
			return ret;
	}
	return 0;
}

static int freq_autodown_initial(struct platform_device *pdev,
	struct freq_autodown_desc *freq_autodown)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	s32 ret;

	AUTOFS_DEBUG("Begin...\n");
	/* get freq autodown state */
	ret = of_property_read_u32(np, "hisilicon,freq-autodown-state",
		&freq_autodown->freq_autodown_state);
	if (ret) {
		dev_err(dev, "[%s]get hisilicon,freq-autodown-state failed\n",
			__func__);
		return -ENODEV;
	}
	AUTOFS_DEBUG("hisilicon,freq-autodown-state = %u\n",
		freq_autodown->freq_autodown_state);
	/* get freq autodown num */
	ret = of_property_read_u32(np, "hisilicon,freq-autodown-num",
		&freq_autodown->freq_autodown_num);
	if (ret) {
		dev_err(dev, "[%s]get hisilicon,freq-autodown-num failed\n",
			__func__);
		return -ENODEV;
	}
	AUTOFS_DEBUG("hisilicon,freq-autodown-num = %u\n",
		freq_autodown->freq_autodown_num);
	/* malloc freq autodown struct */
	freq_autodown->freq_autodown_name = (char **)devm_kzalloc(dev,
		freq_autodown->freq_autodown_num * sizeof(char *), GFP_KERNEL);
	if (!freq_autodown->freq_autodown_name) {
		dev_err(dev, "[%s]devm_kzalloc freq_autodown_name error\n",
			__func__);
		return -ENOMEM;
	}

	freq_autodown->freq_info = (struct freq_autodown_info *)devm_kzalloc(dev,
		freq_autodown->freq_autodown_num * sizeof(struct freq_autodown_info),
		GFP_KERNEL);
	if (!freq_autodown->freq_info) {
		dev_err(dev, "[%s]devm_kzalloc freq_autodown_name error\n",
			__func__);
		ret = -ENOMEM;
		goto get_freq_autodown_name_err;
	}

	ret = freq_autodown_get_info(freq_autodown, dev, np);
	if (ret) {
		ret = -ENODEV;
		goto get_freq_info_err;
	}

	AUTOFS_DEBUG("End\n");
	return 0;
get_freq_info_err:
	devm_kfree(dev, freq_autodown->freq_info);
get_freq_autodown_name_err:
	devm_kfree(dev, freq_autodown->freq_autodown_name);
	return ret;
}

static const struct of_device_id of_freq_autodown_match_tbl[] = {
	{
		.compatible = "hisilicon,hisi-freq-autodown",
	},

	{ /* end */ }
};

static int freq_autodown_get_resources(struct platform_device *pdev,
	struct freq_autodown_desc *freq_autodown, struct device *dev)
{
	if (freq_autodown->freq_autodown_baseaddress_num == FREQ_AUTODOWN_BASEADDR_NUM) {
		/* get media_crg resources */
		freq_autodown->res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		if (!freq_autodown->res)
			goto get_resource_err;

		freq_autodown->media_crg_base_addr = ioremap(freq_autodown->res->start,
			resource_size(freq_autodown->res));
		if (!freq_autodown->media_crg_base_addr) {
			dev_err(dev, "cannot remap media_crg address memory\n");
			return -ENOMEM;
		}
		/* get crgperi resources */
		freq_autodown->res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
		if (!freq_autodown->res)
			goto get_resource_err;

		freq_autodown->crgperi_base_addr = ioremap(freq_autodown->res->start,
			resource_size(freq_autodown->res));
		if (!freq_autodown->crgperi_base_addr) {
			dev_err(dev, "cannot remap crgperi address memory\n");
			return -ENOMEM;
		}
		/* get crgperi resources */
		freq_autodown->res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
		if (!freq_autodown->res)
			goto get_resource_err;

		freq_autodown->media2_crg_base_addr = ioremap(freq_autodown->res->start,
			resource_size(freq_autodown->res));
		if (!freq_autodown->media2_crg_base_addr) {
			dev_err(dev, "cannot remap media2_crg address memory\n");
			return -ENOMEM;
		}
	} else {
		/* get media_crg resources */
		freq_autodown->res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		if (!freq_autodown->res)
			goto get_resource_err;

		freq_autodown->media_crg_base_addr = ioremap(freq_autodown->res->start,
			resource_size(freq_autodown->res));
		if (!freq_autodown->media_crg_base_addr) {
			dev_err(dev, "cannot map register memory\n");
			return -ENOMEM;
		}
	}
	return 0;
get_resource_err:
	dev_err(dev, "platform_get_resource err !\n");
	return -ENOENT;
}

static int check_device_para(struct platform_device *pdev, struct device *dev,
	struct device_node *np)
{
	if (!pdev) {
		pr_err("[%s] get platform device para is err!\n", __func__);
		return -EINVAL;
	}

	if (!dev) {
		pr_err("[%s] get dev is err!\n", __func__);
		return -EINVAL;
	}

	if (!np) {
		pr_err("[%s] get np is err!\n", __func__);
		return -EINVAL;
	}

	return 0;
}
static int freq_autodown_probe(struct platform_device *pdev)
{
	struct freq_autodown_desc *freq_autodown = NULL;
	struct device *dev = NULL;
	struct device_node *np = NULL;
	int ret;

	dev = &pdev->dev;
	np = dev->of_node;
	ret = check_device_para(pdev, dev, np);
	if (ret)
		return ret;

	freq_autodown = (struct freq_autodown_desc *)devm_kzalloc(dev,
		sizeof(struct freq_autodown_desc), GFP_KERNEL);
	if (!freq_autodown) {
		dev_err(dev, "%s devm_kzalloc freq_autodown err\n", __func__);
		return -ENOMEM;
	}

	/* get freq autodown num */
	ret = of_property_read_u32(np, "hisilicon,freq-autodown-baseaddress-num",
		&freq_autodown->freq_autodown_baseaddress_num);
	if (ret) {
		dev_err(dev, "[%s]get hisilicon,freq-autodown-baseaddress-num failed\n",
			__func__);
		return -ENODEV;
	}

	ret = freq_autodown_get_resources(pdev, freq_autodown, dev);
	if (ret)
		goto freq_autodown_fail;

	freq_autodown->dev = &(pdev->dev);

	ret = freq_autodown_initial(pdev, freq_autodown);
	if (ret)
		goto freq_autodown_fail;

	spin_lock_init(&freq_autodown->lock);
	g_freq_autodown = freq_autodown;
	platform_set_drvdata(pdev, freq_autodown);

	return ret;

freq_autodown_fail:
	dev_err(dev, "%s of_get_freq_autodown_attr err!\n", __func__);
	if (freq_autodown->freq_autodown_baseaddress_num == FREQ_AUTODOWN_BASEADDR_NUM) {
		iounmap(freq_autodown->media_crg_base_addr);
		iounmap(freq_autodown->crgperi_base_addr);
		iounmap(freq_autodown->media2_crg_base_addr);
	} else {
		iounmap(freq_autodown->media_crg_base_addr);
	}
	return ret;
}

static int freq_autodown_remove(struct platform_device *pdev)
{
	struct freq_autodown_desc *freq_autodown = platform_get_drvdata(pdev);

	if (freq_autodown->freq_autodown_baseaddress_num ==
		FREQ_AUTODOWN_BASEADDR_NUM) {
		iounmap(freq_autodown->media_crg_base_addr);
		iounmap(freq_autodown->crgperi_base_addr);
		iounmap(freq_autodown->media2_crg_base_addr);
	} else {
		iounmap(freq_autodown->media_crg_base_addr);
	}

	return 0;
}

#ifdef CONFIG_PM
static int freq_autodown_suspend(struct platform_device *pdev,
	pm_message_t pm)
{
	return 0;
}

static int freq_autodown_resume(struct platform_device *pdev)
{
	return 0;
}
#endif

static struct platform_driver freq_autodown_driver = {
	.driver = {
		.name = "freq-autodown",
		.owner = THIS_MODULE,
		.of_match_table = of_freq_autodown_match_tbl,
	},
	.probe = freq_autodown_probe,
	.remove = freq_autodown_remove,
#ifdef CONFIG_PM
	.suspend = freq_autodown_suspend,
	.resume = freq_autodown_resume,
#endif
};

static int __init freq_autodown_init(void)
{
	int ret;

	AUTOFS_DEBUG("\n================[%s begin ]=============\n", __func__);

	ret = platform_driver_register(&freq_autodown_driver);
	if (ret)
		pr_err("%s: platform_driver_register failed!\n", __func__);

	return ret;
}

static void __exit freq_autodown_exit(void)
{
	platform_driver_unregister(&freq_autodown_driver);
}

subsys_initcall(freq_autodown_init);
module_exit(freq_autodown_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("freq autodown driver");
