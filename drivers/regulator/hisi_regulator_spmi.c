/*
 * Copyright (c) Hisilicon technologies Co., Ltd.2013-2019. All rights reserved.
 * Description: provide regulator functions
 * Author: Hisilicon
 * Create: 2013-03-17
 */
#include <linux/mfd/hisi_pmic.h>
#ifdef CONFIG_HISI_PMIC_DEBUG
#include <linux/debugfs.h>
#include <hisi_regulator_debug.h>
#endif
#include <linux/regulator/of_regulator.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/regmap.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <securec.h>
#include <linux/version.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/hisi-spmi.h>
#include <linux/of_hisi_spmi.h>
#include <linux/hisi/hisi_log.h>

#define HISI_LOG_TAG HISI_PMIC_REGULATOR_TAG

#define BRAND_DEBUG(args...) pr_debug(args)

#define STATIC static

#define TIME_CHANGE_US_MS 1000
#define ARRAY_LINE 128

struct hisi_regulator_register_info {
	u32 ctrl_reg;
	u32 enable_mask;
	u32 eco_mode_mask;
	u32 vset_reg;
	u32 vset_mask;
};

struct hisi_regulator {
	const char *name;
	struct hisi_regulator_register_info register_info;
	struct timeval last_off_time;
	u32 off_on_delay;
	u32 eco_uA;
	struct regulator_desc rdesc;
	int (*dt_parse)(struct hisi_regulator *, struct spmi_device *);
};
static DEFINE_MUTEX(enable_mutex);
static struct timeval g_last_enabled;


static inline struct hisi_pmic *rdev_to_pmic(struct regulator_dev *dev)
{
	/*
	 * regulator_dev parent to->
	 * hisi regulator platform device_dev parent to->
	 * hisi pmic platform device_dev
	 */

	if (rdev_get_dev(dev) == NULL)
		return NULL;

	return dev_get_drvdata(rdev_get_dev(dev)->parent->parent);
}
/*
 * helper function to ensure when it returns it is at least 'delay_us'
 * microseconds after 'since'.
 */
static void ensured_time_after(struct timeval since, u32 delay_us)
{
	struct timeval now;
	u64 elapsed_ns64, delay_ns64;
	u32 actual_us32;

	delay_ns64 = delay_us * NSEC_PER_USEC;
	do_gettimeofday(&now);
	elapsed_ns64 = timeval_to_ns(&now) - timeval_to_ns(&since);
	if (delay_ns64 > elapsed_ns64) {
		actual_us32 = ((u32)(delay_ns64 - elapsed_ns64) /
							NSEC_PER_USEC);
		if (actual_us32 >= TIME_CHANGE_US_MS) {
			mdelay((u32)(actual_us32 / TIME_CHANGE_US_MS));
			udelay(actual_us32 % TIME_CHANGE_US_MS);
		} else if (actual_us32 > 0) {
			udelay(actual_us32);
		}
	}
}

static int hisi_regulator_is_enabled(struct regulator_dev *dev)
{
	u32 reg_val;
	struct hisi_regulator *sreg = rdev_get_drvdata(dev);
	struct hisi_pmic *pmic = rdev_to_pmic(dev);

	if (!sreg)
		return 0;

	reg_val = hisi_pmic_read(pmic, sreg->register_info.ctrl_reg);
	BRAND_DEBUG("<[%s]: ctrl_reg=0x%x,enable_state=%d>\n", __func__,
			sreg->register_info.ctrl_reg,
			(reg_val & sreg->register_info.enable_mask));
	return ((reg_val & sreg->register_info.enable_mask) != 0);
}

static int hisi_regulator_enable(struct regulator_dev *dev)
{
	struct hisi_regulator *sreg = rdev_get_drvdata(dev);
	struct hisi_pmic *pmic = rdev_to_pmic(dev);

	if (!sreg)
		return -EINVAL;

	/* keep a distance of off_on_delay from last time disabled */
	ensured_time_after(sreg->last_off_time, sreg->off_on_delay);

	BRAND_DEBUG("<[%s]: off_on_delay = %d us>\n", __func__, sreg->off_on_delay);

	/* cannot enable more than one regulator at one time */
	mutex_lock(&enable_mutex);
	ensured_time_after(g_last_enabled, HISI_REGS_ENA_PROTECT_TIME);

	/* set enable register */
	hisi_pmic_rmw(pmic, sreg->register_info.ctrl_reg,
				sreg->register_info.enable_mask,
				sreg->register_info.enable_mask);
	BRAND_DEBUG("<[%s]: ctrl_reg=0x%x,enable_mask=0x%x>\n",
			__func__, sreg->register_info.ctrl_reg,
			sreg->register_info.enable_mask);

	do_gettimeofday(&g_last_enabled);
	mutex_unlock(&enable_mutex);

	return 0;
}

static int hisi_regulator_disable(struct regulator_dev *dev)
{
	struct hisi_regulator *sreg = rdev_get_drvdata(dev);
	struct hisi_pmic *pmic = rdev_to_pmic(dev);

	if (!sreg)
		return -EINVAL;
	/* set enable register to 0 */
	hisi_pmic_rmw(pmic, sreg->register_info.ctrl_reg,
				sreg->register_info.enable_mask, 0);

	do_gettimeofday(&sreg->last_off_time);

	return 0;
}

static int hisi_regulator_get_voltage(struct regulator_dev *dev)
{
	struct hisi_regulator *sreg = rdev_get_drvdata(dev);
	struct hisi_pmic *pmic = rdev_to_pmic(dev);
	u32 reg_val;
	u32 selector;

	if (!sreg)
		return -EINVAL;

	/* get voltage selector */
	reg_val = hisi_pmic_read(pmic, sreg->register_info.vset_reg);
	BRAND_DEBUG("<[%s]: vset_reg = 0x%x>\n", __func__,
				sreg->register_info.vset_reg);

	selector = (reg_val & sreg->register_info.vset_mask) >>
			((unsigned int)ffs(sreg->register_info.vset_mask) - 1);

	return sreg->rdesc.ops->list_voltage(dev, selector);
}

static int hisi_regulator_set_voltage(struct regulator_dev *dev,
				int min_uv, int max_uv, unsigned int *selector)
{
	struct hisi_regulator *sreg = rdev_get_drvdata(dev);
	struct hisi_pmic *pmic = rdev_to_pmic(dev);
	u32 vsel;
	int uv;

	if (!sreg)
		return -EINVAL;

	for (vsel = 0; vsel < sreg->rdesc.n_voltages; vsel++) {
		uv = sreg->rdesc.volt_table[vsel];
		/* Break at the first in-range value */
		if (min_uv <= uv && uv <= max_uv)
			break;
	}

	/* unlikely to happen. sanity test done by regulator core */
	if (unlikely(vsel == sreg->rdesc.n_voltages))
		return -EINVAL;

	*selector = vsel;
	/* set voltage selector */
	hisi_pmic_rmw(pmic, sreg->register_info.vset_reg,
		sreg->register_info.vset_mask,
		vsel << ((unsigned int)ffs(sreg->register_info.vset_mask) - 1));

	BRAND_DEBUG("<[%s]: vset_reg=0x%x, vset_mask=0x%x, value=0x%x>\n",
			__func__, sreg->register_info.vset_reg,
			sreg->register_info.vset_mask,
			vsel << (ffs(sreg->register_info.vset_mask) - 1));

	return 0;
}

static unsigned int hisi_regulator_get_mode(struct regulator_dev *dev)
{
	struct hisi_regulator *sreg = rdev_get_drvdata(dev);
	struct hisi_pmic *pmic = rdev_to_pmic(dev);
	u32 reg_val;

	if (!sreg)
		return 0;

	reg_val = hisi_pmic_read(pmic, sreg->register_info.ctrl_reg);
	BRAND_DEBUG("<[%s]: reg_val=%d, ctrl_reg=0x%x, eco_mode_mask=0x%x>\n",
			__func__, reg_val,
			sreg->register_info.ctrl_reg,
			sreg->register_info.eco_mode_mask);

	if (reg_val & sreg->register_info.eco_mode_mask)
		return REGULATOR_MODE_IDLE;
	else
		return REGULATOR_MODE_NORMAL;
}

static int hisi_regulator_set_mode(struct regulator_dev *dev,
						unsigned int mode)
{
	struct hisi_regulator *sreg = rdev_get_drvdata(dev);
	struct hisi_pmic *pmic = rdev_to_pmic(dev);
	u32 eco_mode;

	if (!sreg)
		return -EINVAL;

	switch (mode) {
	case REGULATOR_MODE_NORMAL:
		eco_mode = HISI_ECO_MODE_DISABLE;
		break;
	case REGULATOR_MODE_IDLE:
		eco_mode = HISI_ECO_MODE_ENABLE;
		break;
	default:
		return -EINVAL;
	}

	/* set mode */
	hisi_pmic_rmw(pmic, sreg->register_info.ctrl_reg,
		sreg->register_info.eco_mode_mask,
		eco_mode << ((unsigned int)ffs(sreg->register_info.eco_mode_mask) - 1));

	BRAND_DEBUG("<[%s]: ctrl_reg=0x%x, eco_mode_mask=0x%x, value=0x%x>\n",
		__func__, sreg->register_info.ctrl_reg,
		sreg->register_info.eco_mode_mask,
		eco_mode << (ffs(sreg->register_info.eco_mode_mask) - 1));
	return 0;
}


unsigned int hisi_regulator_get_optimum_mode(struct regulator_dev *dev,
			int input_uV, int output_uV, int load_uA)
{
	struct hisi_regulator *sreg = rdev_get_drvdata(dev);

	if (!sreg)
		return 0;

	if ((load_uA == 0) || ((unsigned int)load_uA > sreg->eco_uA))
		return REGULATOR_MODE_NORMAL;
	else
		return REGULATOR_MODE_IDLE;
}

static int hisi_dt_parse_common(struct hisi_regulator *sreg,
					struct spmi_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct regulator_desc *rdesc = &sreg->rdesc;
	unsigned int register_info[3] = {0}; /* the length is determined by dts parameter */
	int ret;

	/* parse .register_info.ctrl_reg */
	ret = of_property_read_u32_array(np, "hisilicon,hisi-ctrl",
						register_info, 3);
	if (ret) {
		dev_err(dev, "no hisilicon,hisi-ctrl property set\n");
		return ret;
	}
	sreg->register_info.ctrl_reg = register_info[0];
	sreg->register_info.enable_mask = register_info[1];
	sreg->register_info.eco_mode_mask = register_info[2];

	/* parse .register_info.vset_reg */
	ret = of_property_read_u32_array(np, "hisilicon,hisi-vset",
						register_info, 2);
	if (ret) {
		dev_err(dev, "no hisilicon,hisi-vset property set\n");
		return ret;
	}
	sreg->register_info.vset_reg = register_info[0];
	sreg->register_info.vset_mask = register_info[1];

	/* parse .off-on-delay */
	ret = of_property_read_u32(np, "hisilicon,hisi-off-on-delay-us",
						&sreg->off_on_delay);
	if (ret) {
		dev_err(dev, "no hisilicon,hisi-off-on-delay-us property set\n");
		return ret;
	}

	/* parse .enable_time */
	ret = of_property_read_u32(np, "hisilicon,hisi-enable-time-us",
				   &rdesc->enable_time);
	if (ret) {
		dev_err(dev, "no hisilicon,hisi-enable-time-us property set\n");
		return ret;
	}

	/* parse .eco_uA */
	ret = of_property_read_u32(np, "hisilicon,hisi-eco-microamp",
				   &sreg->eco_uA);
	if (ret) {
		sreg->eco_uA = 0;
		ret = 0;
	}
	return ret;

}
static int fake_of_get_regulator_constraint(struct regulation_constraints *constraints,
						struct device_node *np)
{
	unsigned int temp_modes = 0;
	int ret;

	/* hisi regulator supports two modes */
	ret = of_property_read_u32_array(np, "hisilicon,valid-modes-mask",
					&(constraints->valid_modes_mask), 1);
	if (ret) {
		pr_err("no hisilicon,valid-modes-mask property set\n");
		ret = -ENODEV;
		return ret;
	}
	ret = of_property_read_u32_array(np, "hisilicon,valid-idle-mask",
						&temp_modes, 1);
	if (ret) {
		pr_err("no hisilicon,valid-modes-mask property set\n");
		ret = -ENODEV;
		return ret;
	}
	constraints->valid_ops_mask |= temp_modes;
	return ret;

}
static int hisi_dt_parse_ldo(struct hisi_regulator *sreg,
				struct spmi_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct regulator_desc *rdesc = &sreg->rdesc;
	unsigned int *v_table = NULL;
	int ret;

	/* parse .n_voltages, and .volt_table */
	ret = of_property_read_u32(np, "hisilicon,hisi-n-voltages",
				   &rdesc->n_voltages);
	if (ret) {
		dev_err(dev, "no hisilicon,hisi-n-voltages property set\n");
		return ret;
	}
	/* alloc space for .volt_table */
	v_table = devm_kzalloc(
		dev, sizeof(unsigned int) * rdesc->n_voltages, GFP_KERNEL);
	if (!v_table) {
		ret = -ENOMEM;
		dev_err(dev, "no memory for .volt_table\n");
		return ret;
	}
	ret = of_property_read_u32_array(np, "hisilicon,hisi-vset-table",
						v_table, rdesc->n_voltages);
	if (ret) {
		dev_err(dev, "no hisilicon,hisi-vset-table property set\n");
		devm_kfree(dev, v_table);
		return ret;
	}
	rdesc->volt_table = v_table;

	/* parse hisi regulator's dt common part */
	ret = hisi_dt_parse_common(sreg, pdev);
	if (ret) {
		dev_err(dev, "failure in hisi_dt_parse_common\n");
		devm_kfree(dev, v_table);
		return ret;
	}
	return ret;
}

static struct regulator_ops hisi_ldo_rops = {
	.is_enabled = hisi_regulator_is_enabled,
	.enable = hisi_regulator_enable,
	.disable = hisi_regulator_disable,
	.list_voltage = regulator_list_voltage_table,
	.get_voltage = hisi_regulator_get_voltage,
	.set_voltage = hisi_regulator_set_voltage,
	.get_mode = hisi_regulator_get_mode,
	.set_mode = hisi_regulator_set_mode,
	.get_optimum_mode = hisi_regulator_get_optimum_mode,
};

static const struct hisi_regulator hisi_regulator_ldo = {
	.rdesc = {
	.ops = &hisi_ldo_rops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	.dt_parse = hisi_dt_parse_ldo,
};

static const struct of_device_id of_hisi_regulator_match_tbl[] = {
	{
		.compatible = "hisilicon-hisi-ldo",
		.data = &hisi_regulator_ldo,
	},
	{ /* end */ }
};

#ifdef CONFIG_HISI_PMIC_DEBUG
u32 pmu_atoi(char *s)
{
	char *p = NULL;
	char c;
	u64 ret = 0;

	if (!s)
		return 0;
	p = s;
	while ((c = *p++) != '\0') {
		if (c >= '0' && c <= '9') {
			ret *= 10;
			ret += (u64)((unsigned char)c - '0');
			if (ret > U32_MAX)
				return 0;
		} else {
			break;
		}
	}
	return (u32)ret;
}
static int dbg_hisi_regulator_show(struct seq_file *s, void *data)
{
	if (!s)
		return -1;
	seq_puts(s, "\n\r");
	seq_printf(s, "%-13s %-15s %-15s %-15s %-15s\n\r",
		"LDO_NAME", "ON/OFF", "Use_count", "Open_count", "Always_on");
	seq_printf(s, "-----------------------------------------"
			"-----------------------------------------------\n\r");
	get_current_regulator_dev(s);
	return 0;
}

static int dbg_hisi_regulator_open(struct inode *inode, struct file *file)
{
	return single_open(file, dbg_hisi_regulator_show, inode->i_private);
}

static const struct file_operations debug_regulator_state_fops = {
	.open		= dbg_hisi_regulator_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int dbg_control_regulator_show(struct seq_file *s, void *data)
{
	printk("                                                                             \n\r \
		---------------------------------------------------------------------------------\n\r \
		|usage:                                                                         |\n\r \
		|	S = state	R = read	V = voltage                                         |\n\r \
		|	set ldo state and voltage                                                   |\n\r \
		|	get ldo state and current voltage                                           |\n\r \
		|example:                                                                       |\n\r \
		|	echo S ldo16 0   > control_regulator	:disable ldo16                      |\n\r \
		|	echo S ldo16 1   > control_regulator	:enable ldo16                       |\n\r \
		|	echo R ldo16     > control_regulator	:get ldo16 state and voltage        |\n\r \
		|	echo V ldo16 xxx > control_regulator	:set ldo16 voltage                  |\n\r \
		---------------------------------------------------------------------------------\n\r");
	return 0;
}
static int dbg_control_regulator_arg_check(const char __user *buffer,
					size_t count)
{

	if (count >= 128 || count == 0) {
		pr_info("error! buffer size illegal!\n");
		return -EFAULT;
	}

	if (!buffer) {
		pr_info("error! buffer is NULL!\n");
		return -EFAULT;
	}
	return 0;
}

static ssize_t dbg_control_regulator_set_value(struct file *filp,
	const char __user *buffer, size_t count, loff_t *ppos)
{
	char tmp[ARRAY_LINE] = {0};
	char ptr[ARRAY_LINE] = {0};
	char *vol = NULL;
	unsigned int i;
	unsigned int j;
	int next_flag = 1;
	char num = 0;
	int ret;

	ret = dbg_control_regulator_arg_check(buffer, count);
	if (ret) {
		pr_info("error!\n");
		return -EFAULT;
	}
	if (copy_from_user(tmp, buffer, count)) {
		pr_info("error!\n");
		return -EFAULT;
	}
	if (tmp[0] == 'R' || tmp[0] == 'r') {
		for (i = 2, j = 0; i < (count - 1); i++, j++)
			ptr[j] = tmp[i];
		ptr[j] = '\0';
		get_regulator_state(ptr, ARRAY_LINE);
	} else if (tmp[0] == 'S' || tmp[0] == 's') {
		for (i = 2, j = 0; i < (count - 1); i++, j++) {
			if (tmp[i] == ' ') {
				next_flag = 0;
				ptr[j] = '\0';
				continue;
			}
			if (next_flag)
				ptr[j] = tmp[i];
			else
				num = tmp[i] - '0';
		}
		set_regulator_state(ptr, num);
	} else if (tmp[0] == 'V' || tmp[0] == 'v') {
		for (i = 2, j = 0; i < (count - 1); i++, j++) {
			if (tmp[i] == ' ') {
				next_flag = 0;
				ptr[j] = '\0';
				continue;
			}
			if (next_flag) {
				ptr[j] = tmp[i];
			} else {
				vol = &tmp[i];
				break;
			}
		}
		set_regulator_voltage(ptr, pmu_atoi(vol));
	}

	*ppos += count;

	return count;
}

static int dbg_control_regulator_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return single_open(file, dbg_control_regulator_show, &inode->i_private);
}

static const struct file_operations set_control_regulator_fops = {
	.open		= dbg_control_regulator_open,
	.read		= seq_read,
	.write		= dbg_control_regulator_set_value,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int hisi_regulator_debugfs_create(struct device *dev)
{
	struct dentry *d = NULL;
	static int debugfs_flag;

	if (debugfs_flag == 0) {
		d = debugfs_create_dir("hisi_regulator_debugfs", NULL);
		if (!d) {
			dev_err(dev, "failed to create hisi regulator debugfs dir!\n");
			return -ENOMEM;
		}
		(void) debugfs_create_file("regulator_state", S_IRUSR,
					d, NULL, &debug_regulator_state_fops);

		(void) debugfs_create_file("control_regulator", S_IRUSR,
					d, NULL, &set_control_regulator_fops);
		debugfs_flag = 1;
	}
	return 0;
}
#endif

static int hisi_regulator_probe(struct spmi_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct regulator_desc *rdesc = NULL;
	struct regulator_dev *rdev = NULL;
	struct hisi_regulator *sreg = NULL;
	struct regulator_init_data *initdata = NULL;
	struct regulator_config config;
	const struct of_device_id *match = NULL;
	const struct hisi_regulator *template = NULL;
	struct regulation_constraints *constraint = NULL;
	const char *supplyname = NULL;
	int ret;

	/* to check which type of regulator this is */
	match = of_match_device(of_hisi_regulator_match_tbl, &pdev->dev);
	if (!match) {
		pr_err("get hisi regulator fail!\n");
		return -EINVAL;
	}
	template = match->data;
	initdata = of_get_regulator_init_data(dev, np, NULL);
	if (!initdata) {
		pr_err("get regulator init data error!\n");
		return -EINVAL;
	}
	ret = memset_s(&config, sizeof(struct regulator_config),
				0, sizeof(struct regulator_config));
	if (ret != EOK)
		pr_err("memset_s fail!\n");
	/* hisi regulator supports two modes */
	constraint = &initdata->constraints;
	ret = fake_of_get_regulator_constraint(constraint, np);
	if (ret) {
		pr_err("get regulator mode error!\n");
		return ret;
	}
	sreg = kmemdup(template, sizeof(*sreg), GFP_KERNEL);
	if (!sreg) {
		pr_err("template kememdup is fail\n");
		return -ENOMEM;
	}
	sreg->name = initdata->constraints.name;
	rdesc = &sreg->rdesc;
	rdesc->name = sreg->name;
	rdesc->min_uV = initdata->constraints.min_uV;
	supplyname = of_get_property(np, "hisilicon,supply_name", NULL);
	if (supplyname != NULL)
		initdata->supply_regulator = supplyname;
	/* to parse device tree data for regulator specific */
	ret = sreg->dt_parse(sreg, pdev);
	if (ret) {
		dev_err(dev, "device tree parameter parse error!\n");
		goto hisi_probe_end;
	}
	config.dev = &pdev->dev;
	config.init_data = initdata;
	config.driver_data = sreg;
	config.of_node = pdev->dev.of_node;

	/* register regulator */
	rdev = regulator_register(rdesc, &config);
	if (IS_ERR(rdev)) {
		dev_err(dev, "failed to register %s\n",
			rdesc->name);
		ret = PTR_ERR(rdev);
		goto hisi_probe_end;
	}
	BRAND_DEBUG("[%s]:valid_modes_mask[0x%x], valid_ops_mask[0x%x]\n",
		rdesc->name, constraint->valid_modes_mask, constraint->valid_ops_mask);

	dev_set_drvdata(dev, rdev);
#ifdef CONFIG_HISI_PMIC_DEBUG
	ret = hisi_regulator_debugfs_create(dev);
	if (ret)
		regulator_unregister(rdev);
#endif
hisi_probe_end:
	if (ret)
		kfree(sreg);
	return ret;
}

static int hisi_regulator_remove(struct spmi_device *pdev)
{
	struct regulator_dev *rdev = NULL;
	struct hisi_regulator *sreg = NULL;

	rdev = dev_get_drvdata(&pdev->dev);
	if (!rdev) {
		pr_err("%s:rdev is NULL\n", __func__);
		return -ENOMEM;
	}

	sreg = rdev_get_drvdata(rdev);
	if (!sreg) {
		pr_err("%s:sreg is NULL\n", __func__);
		return -ENOMEM;
	}

	regulator_unregister(rdev);

	if (sreg->rdesc.volt_table)
		devm_kfree(&pdev->dev, (unsigned int *)sreg->rdesc.volt_table);

	kfree(sreg);
	return 0;
}

static const struct spmi_device_id regulator_spmi_id[] = {
	{ "hisilicon-hisi-ldo", 0 },
	{}
};

static struct spmi_driver hisi_pmic_driver = {
	.driver = {
		.name	= "hisi_regulator",
		.owner  = THIS_MODULE,
		.of_match_table = of_hisi_regulator_match_tbl,
	},
	.id_table = regulator_spmi_id,
	.probe	= hisi_regulator_probe,
	.remove	= hisi_regulator_remove,
};

STATIC int __init hisi_regulator_init(void)
{
	return spmi_driver_register(&hisi_pmic_driver);
}

STATIC void __exit hisi_regulator_exit(void)
{
	spmi_driver_unregister(&hisi_pmic_driver);
}

fs_initcall(hisi_regulator_init);
module_exit(hisi_regulator_exit);

MODULE_DESCRIPTION("Hisi regulator driver");
MODULE_LICENSE("GPL v2");
