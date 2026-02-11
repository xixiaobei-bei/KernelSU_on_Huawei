/*
 *Device driver for vbat drop protect
 */

#include <hisi_vbat_drop_protect.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#include <linux/slab.h>

#ifdef CONFIG_HISI_HW_VOTE
#include <linux/hisi/hisi_hw_vote.h>

static struct hvdev *limit_bigfreq_hvdev;
static struct hvdev *limit_middlefreq_hvdev;
static struct hvdev *limit_littlefreq_hvdev;
static struct hvdev *limit_l3freq_hvdev;
static struct hvdev *limit_gpufreq_hvdev;
#endif

#define HV_NAME(hvdev) (IS_ERR_OR_NULL(hvdev->name) ? "unknown" : hvdev->name)

#ifdef VBAT_DROP_TEST

/****************************************************************************
 *  Function:        hisi_vbat_drop_print_auto_div_state
 *  Description:     print vbat drop ip state.
 *  Input:           struct hisi_vbat_drop_protect_dev *di,   ---- vbat drop
 *                    protect device
 *  Output:          NULL
 *  Return:          NULL.
 *  Remark:          print vbat drop ip state
 ***************************************************************************/
static void hisi_vbat_drop_print_auto_div_state(
	struct hisi_vbat_drop_protect_dev *di)
{
	u32 core_big_stat = 0;
	u32 core_mid_stat = 0;
	u32 core_lit_stat = 0;
	u32 core_l3_stat = 0;
	u32 core_gpu_stat = 0;
	u32 core_npu_stat = 0;

	if (di == NULL)
		return;
	if (di->core_big_base != NULL)
		core_big_stat =
			readl(BIG_VOL_DROP_EN_STAT_ADDR(di->core_big_base)) &
			BIG_VOL_DROP_EN_STAT_BIT;
	if (di->core_lit_base != NULL)
		core_lit_stat =
			readl(LITTLE_VOL_DROP_EN_STAT_ADDR(di->core_lit_base)) &
			LITTLE_VOL_DROP_EN_STAT_BIT;
#ifndef CONFIG_HISI_VBAT_DROP_PRT_ORL
	if (di->core_mid_base != NULL)
		core_mid_stat =
			readl(MIDDLE_VOL_DROP_EN_STAT_ADDR(di->core_mid_base)) &
			MIDDLE_VOL_DROP_EN_STAT_BIT;
#endif
	if (di->core_l3_base != NULL)
		core_l3_stat =
			readl(L3_VOL_DROP_EN_STAT_ADDR(di->core_l3_base)) &
			L3_VOL_DROP_EN_STAT_BIT;
	if (di->core_gpu_base != NULL)
		core_gpu_stat =
			readl(GPU_VOL_DROP_EN_STAT_ADDR(di->core_gpu_base)) &
			GPU_VOL_DROP_EN_STAT_BIT;
#ifndef CONFIG_HISI_VBAT_DROP_PRT_ATLA
	if (di->core_npu_base != NULL)
		core_npu_stat =
			readl(NPU_VOL_DROP_EN_STAT_ADDR(di->core_npu_base)) &
			NPU_VOL_DROP_EN_STAT_BIT;
#endif
	pr_err("[%s]vbat_drop:L=0x%x,M=0x%x,B=0x%x,L3=0x%x,G=0x%x,N=0x%x!\n",
		__func__, core_lit_stat, core_mid_stat, core_big_stat,
		core_l3_stat, core_gpu_stat, core_npu_stat);
}
#endif

#ifdef CONFIG_HISI_HW_VOTE

static struct hvdev *cluster_freq_register(
	struct device *dev, const char *propname)
{
	struct device_node *np = NULL;
	const char *ch_name = NULL;
	const char *vsrc = NULL;
	struct hvdev *freq_hvdev = NULL;
	int ret;

	np = dev->of_node;
	/*get cpu freq from dts and register to hvdev*/
	ret = of_property_read_string_index(np, propname, 0, &ch_name);
	if (ret) {
		dev_err(dev, "[%s]:parse %s fail,not channel!\n", __func__,
			propname);
		return NULL;
	}
	ret = of_property_read_string_index(np, propname, 1, &vsrc);
	if (ret) {
		dev_err(dev, "[%s]:parse %s vote src fail!\n", __func__,
			propname);
		return NULL;
	}
	freq_hvdev = hisi_hvdev_register(dev, ch_name, vsrc);
	if (IS_ERR_OR_NULL(freq_hvdev)) {
		dev_err(dev, "[%s]: %s vote register fail!\n", __func__,
			propname);
		return NULL;
	}
	return freq_hvdev;
}
/********************************************************************
 *  Function:        hisi_cluster_freq_limit_init
 *  Description:     freq limt init
 *  Input:           struct device *dev   ---- vbat drop protect device
 *  Output:          NULL
 *  Return:          NULL
 *  Remark:          get freq core from dts and register to hvdev
 ********************************************************************/
static void hisi_vbat_drop_cluster_freq_limit_init(struct device *dev)
{
	if (!dev)
		return;

	limit_bigfreq_hvdev =
		cluster_freq_register(dev, "bigfreq-limit-channel");
	limit_middlefreq_hvdev =
		cluster_freq_register(dev, "middlefreq-limit-channel");
	limit_littlefreq_hvdev =
		cluster_freq_register(dev, "littlefreq-limit-channel");
	limit_l3freq_hvdev = cluster_freq_register(dev, "l3freq-limit-channel");
	limit_gpufreq_hvdev =
		cluster_freq_register(dev, "gpufreq-limit-channel");
}

static void hisi_vbat_drop_reset_cluster_freq(struct hvdev *hvdev)
{
	int ret;

	if (IS_ERR_OR_NULL(hvdev)) {
		pr_err("%s: not register hw vote!\n", __func__);
		return;
	}
	ret = hisi_hv_set_freq(hvdev, 0);
	if (!ret)
		pr_err("[%s]%s votes to lowest freq\n", __func__,
			HV_NAME(hvdev));
}

static void hisi_vbat_drop_restore_cluster_freq(struct hvdev *hvdev)
{
	int ret;

	if (IS_ERR_OR_NULL(hvdev)) {
		pr_err("%s: not register hw vote!\n", __func__);
		return;
	}
	ret = hisi_hv_set_freq(hvdev, 0xFFFFFFFF);
	if (!ret)
		pr_err("[%s]%s votes to normal freq\n", __func__,
			HV_NAME(hvdev));
}
/********************************************************************
 * Function:        hisi_vbat_drop_cluster_freq_set
 * Description:     set core freq
 * Input:           enum vbat_drop_freq  freq_type ---- core freq.
 * Output:          NULL
 * Return:          NULL
 * Remark:          set core freq to min or normal.
 **********************************************************************/
static void hisi_vbat_drop_cluster_freq_set(enum vbat_drop_freq freq_type)
{
	/*vote big cpu middle cpu and gpu to lowest freq*/
	pr_err("[%s]freq type [%d]\n", __func__, freq_type);

	if (freq_type == MIN_FREQ) {
		hisi_vbat_drop_reset_cluster_freq(limit_bigfreq_hvdev);
		hisi_vbat_drop_reset_cluster_freq(limit_middlefreq_hvdev);
		hisi_vbat_drop_reset_cluster_freq(limit_littlefreq_hvdev);
		hisi_vbat_drop_reset_cluster_freq(limit_l3freq_hvdev);
		hisi_vbat_drop_reset_cluster_freq(limit_gpufreq_hvdev);
	} else if (freq_type == RESTOR_FREQ) {
		/*vote big cpu middle cpu and gpu to restore normal freq*/
		hisi_vbat_drop_restore_cluster_freq(limit_bigfreq_hvdev);
		hisi_vbat_drop_restore_cluster_freq(limit_middlefreq_hvdev);
		hisi_vbat_drop_restore_cluster_freq(limit_littlefreq_hvdev);
		hisi_vbat_drop_restore_cluster_freq(limit_l3freq_hvdev);
		hisi_vbat_drop_restore_cluster_freq(limit_gpufreq_hvdev);
	} else {
	}
}
#endif

/********************************************************************
 * Function:        hisi_vbat_drop_interrupt_work
 * Description:     check battery vol and restore normal freq .
 * Input:           work: vbat dorp interrupt workqueue.
 * Output:          NULL
 * Return:          NULL
 * Remark:          NA
 *********************************************************************/
static void hisi_vbat_drop_interrupt_work(struct work_struct *work)
{
	struct hisi_vbat_drop_protect_dev *di = container_of(work,
		struct hisi_vbat_drop_protect_dev, vbat_drop_irq_work.work);
	static int battery_check_cnt;
	static int bat_vol_normal_cnt;
	int vbatt_mv;

	battery_check_cnt++;
	vbatt_mv = hisi_battery_voltage();
	pr_err("[%s]battery_check_cnt:[%d],battery voltage = %d mv\n", __func__,
		battery_check_cnt, vbatt_mv);

	/*work will exit after trying VBAT_DROP_VOL_NORMAL_CNT timers success */
	if (vbatt_mv > (int)di->vbat_drop_vol_mv + VBAT_DROP_VOL_INC_MV) {
		pr_err("[%s] bat_vol_normal_cnt = %d!\n", __func__,
			bat_vol_normal_cnt);
		bat_vol_normal_cnt++;
		/* Try a few times*/
		if (bat_vol_normal_cnt > VBAT_DROP_VOL_NORMAL_CNT) {
#ifdef CONFIG_HISI_HW_VOTE
			hisi_vbat_drop_cluster_freq_set(RESTOR_FREQ);
#endif
			/*clear interrupt status and cancel auto 2 div*/
			HISI_VBAT_DROP_PMIC_REG_WRITE(PMIC_VSYS_DROP_IRQ_REG,
				PMIC_VSYS_DROP_IRQ_CLEAR);
			enable_irq(di->vbat_drop_irq);
			battery_check_cnt = 0;
			bat_vol_normal_cnt = 0;
			pr_err("[%s] exit work,vbat vol restore!\n", __func__);
#ifdef VBAT_DROP_TEST
			hisi_vbat_drop_print_auto_div_state(di);
#endif
			__pm_relax(&di->vbatt_check_lock); /*lint !e455*/
		} else
			queue_delayed_work(system_power_efficient_wq,
				&di->vbat_drop_irq_work,
				msecs_to_jiffies(2000));

	} else {
		bat_vol_normal_cnt = 0;
		queue_delayed_work(system_power_efficient_wq,
			&di->vbat_drop_irq_work, msecs_to_jiffies(2000));
	}
}
/********************************************************************
 *  Function:        hisi_vbat_drop_irq_handler
 *  Description:     handle vbat drop irq .
 *  Input:           irq: vbat drop irq, *data: hisi_vbat_drop_protect_dev
 *  Output:          NULL
 *  Return:          IRQ_HANDLED:success , others:fail
 *  Remark:          set core min freq and schedule vat drop check work.
 **********************************************************************/
static irqreturn_t hisi_vbat_drop_irq_handler(int irq, void *data)
{
	struct hisi_vbat_drop_protect_dev *di =
		(struct hisi_vbat_drop_protect_dev *)data;

	__pm_stay_awake(&di->vbatt_check_lock);
	pr_err("[%s] enter vbat handle!\n", __func__);

	/*interrupt mask*/
	disable_irq_nosync(di->vbat_drop_irq);
/*printf auto div register state*/
#ifdef VBAT_DROP_TEST
	hisi_vbat_drop_print_auto_div_state(di);
#endif
#ifdef CONFIG_HISI_HW_VOTE
	hisi_vbat_drop_cluster_freq_set(MIN_FREQ);
#endif
	/*delayed work: check battery voltage*/
	queue_delayed_work(system_power_efficient_wq, &di->vbat_drop_irq_work,
		msecs_to_jiffies(0));
	return IRQ_HANDLED; /*lint !e454*/ /*work unlock*/
}

static int get_vbat_drop_flag(struct device *dev, struct device_node *np,
	int lpm3_ctrl, const char *property)
{
	int result = 0;

	if (property == NULL) {
		dev_err(dev, "[%s]: prperty is NULL!\n", __func__);
		return 0;
	}
	if (lpm3_ctrl) {
		dev_info(dev, "[%s]: lpm3 ctrl %s!\n", __func__, property);
		return 0;
	}
	if (of_property_read_u32(np, property, &result)) {
		result = 0;
		dev_err(dev, "[%s]: get %s fail!\n", __func__, property);
	}
	dev_info(dev, "[%s]:%s:%d!\n", __func__, property, result);
	return result;
}

static void parse_auto_div_dts(
	struct hisi_vbat_drop_protect_dev *di, struct device_node *np)
{
	int lpm3_ctrl = 0;

	if (of_property_read_u32(np, "lpm3_ctrl_en", &lpm3_ctrl)) {
		lpm3_ctrl = 0;
		dev_err(di->dev, "[%s]: get lpm3_ctrl_en fail!\n", __func__);
	}
	/*get auto div flag */
	di->big_cpu_auto_div_en = get_vbat_drop_flag(
		di->dev, np, lpm3_ctrl, "big_cpu_auto_div_en");
	di->middle_cpu_auto_div_en = get_vbat_drop_flag(
		di->dev, np, lpm3_ctrl, "middle_cpu_auto_div_en");
	di->little_cpu_auto_div_en = get_vbat_drop_flag(
		di->dev, np, lpm3_ctrl, "little_cpu_auto_div_en");
	di->l3_auto_div_en = get_vbat_drop_flag(
		di->dev, np, lpm3_ctrl, "l3_cpu_auto_div_en");
	di->gpu_auto_div_en =
		get_vbat_drop_flag(di->dev, np, lpm3_ctrl, "gpu_auto_div_en");
	di->npu_auto_div_en =
		get_vbat_drop_flag(di->dev, np, 0, "npu_auto_div_en");
}

static void __iomem *parse_cfg_addr(struct device *dev, const char *compatible)
{
	struct device_node *base_np = NULL;
	void __iomem *cfg_base = NULL;

	base_np = of_find_compatible_node(NULL, NULL, compatible);
	if (!base_np) {
		dev_err(dev, "[%s] %s not found!\n", __func__, compatible);
		return NULL;
	}
	cfg_base = of_iomap(base_np, 0);
	if (!cfg_base) {
		dev_err(dev, "[%s] %s of iomap fail!\n", __func__, compatible);
		return NULL;
	}
	return cfg_base;
}
/****************************************************************************
 *  Function:        hisi_vbat_drop_parse_dts
 *  Description:     parse dts for vbat_drop protect function.
 *  Input:           struct hisi_vbat_drop_protect_dev *di,   ---- vbat drop
 *                    protect device
 *  Output:          NULL
 *  Return:          0:success,others:fail.
 *  Remark:          NA
 ****************************************************************************/
static int hisi_vbat_drop_parse_dts(struct hisi_vbat_drop_protect_dev *di)
{
	struct device_node *np = NULL;
	void __iomem *cfg_base = NULL;

	if (!di || !di->dev)
		return -1;

	np = di->dev->of_node;
	if (!np)
		return -1;

	/*get vbat drop vol mv*/
	if (of_property_read_u32(
		    np, "vbat_drop_vol_mv", &di->vbat_drop_vol_mv)) {
		di->vbat_drop_vol_mv = VBAT_DROP_VOL_DEFAULT;
		dev_err(di->dev, "[%s]: get vbat drop fail!\n", __func__);
	}
	dev_err(di->dev, "[%s]: vbat_drop_vol_mv = [%d]mv!\n", __func__,
		di->vbat_drop_vol_mv);
/*get soc base*/
#if defined(CONFIG_HISI_VBAT_DROP_PRT_ATLA)
	cfg_base = parse_cfg_addr(di->dev, "hisilicon,pmctrl");
	di->core_big_base = cfg_base;
	di->core_mid_base = cfg_base;
	di->core_lit_base = cfg_base;
	di->core_l3_base = cfg_base;
	di->core_gpu_base = cfg_base;
	di->core_npu_base = NULL;
#else
	cfg_base = parse_cfg_addr(di->dev, "hisilicon,crgbmctrl");
	di->core_big_base = cfg_base;
	di->core_mid_base = cfg_base;
	cfg_base = parse_cfg_addr(di->dev, "hisilicon,crgll3ctrl");
	di->core_lit_base = cfg_base;
	di->core_l3_base = cfg_base;
	cfg_base = parse_cfg_addr(di->dev, "hisilicon,pmctrl");
	di->core_gpu_base = cfg_base;
	di->core_npu_base = cfg_base;
#endif
#if defined(CONFIG_HISI_VBAT_DROP_PRT_ORL)
	di->core_mid_base = NULL;
	di->core_npu_base = parse_cfg_addr(di->dev, "hisilicon,crgctrl");
#endif
	parse_auto_div_dts(di, np);
	return 0;
}

static void hisi_vbat_drop_big_cpu_en(
	struct hisi_vbat_drop_protect_dev *di, int en)
{
	void __iomem *base = di->core_big_base;
	u32 reg_value;

	if (di->big_cpu_auto_div_en == 0)
		return;
	if (!base) {
		pr_err("%s:base is NULL\n", __func__);
		return;
	}
	reg_value = readl(BIG_VOL_DROP_EN_ADDR(base));
	if (!en)
		reg_value = reg_value & (~BIG_VOL_DROP_EN_BIT);
	else
		reg_value = reg_value | BIG_VOL_DROP_EN_BIT;
	writel(reg_value, BIG_VOL_DROP_EN_ADDR(base));
	pr_err("[%s]enable cpu big:%d!\n", __func__, en);
}
#if defined (CONFIG_HISI_VBAT_DROP_PRT_ORL)
static void hisi_vbat_drop_mid_cpu_en(
	struct hisi_vbat_drop_protect_dev *di, int en)
{

}
#else
static void hisi_vbat_drop_mid_cpu_en(
	struct hisi_vbat_drop_protect_dev *di, int en)
{
	void __iomem *base = di->core_mid_base;
	u32 reg_value;

	if (di->middle_cpu_auto_div_en == 0)
		return;
	if (!base) {
		pr_err("%s:base is NULL\n", __func__);
		return;
	}
	reg_value = readl(MIDDLE_VOL_DROP_EN_ADDR(base));
	if (!en)
		reg_value = reg_value & (~MIDDLE_VOL_DROP_EN_BIT);
	else
		reg_value = reg_value | MIDDLE_VOL_DROP_EN_BIT;
	writel(reg_value, MIDDLE_VOL_DROP_EN_ADDR(base));
	pr_err("[%s]enable cpu middle:%d!\n", __func__, en);
}
#endif
static void hisi_vbat_drop_lit_cpu_en(
	struct hisi_vbat_drop_protect_dev *di, int en)
{
	void __iomem *base = di->core_lit_base;
	u32 reg_value;

	if (di->little_cpu_auto_div_en == 0)
		return;
	if (!base) {
		pr_err("%s:base is NULL\n", __func__);
		return;
	}
	reg_value = readl(LITTLE_VOL_DROP_EN_ADDR(base));
	if (!en)
		reg_value = reg_value & (~LITTLE_VOL_DROP_EN_BIT);
	else
		reg_value = reg_value | LITTLE_VOL_DROP_EN_BIT;
	writel(reg_value, LITTLE_VOL_DROP_EN_ADDR(base));
	pr_err("[%s]enable cpu little:%d!\n", __func__, en);
}

static void hisi_vbat_drop_l3_cpu_en(
	struct hisi_vbat_drop_protect_dev *di, int en)
{
	void __iomem *base = di->core_l3_base;
	u32 reg_value;

	if (di->l3_auto_div_en == 0)
		return;
	if (!base) {
		pr_err("%s:base is NULL\n", __func__);
		return;
	}
	reg_value = readl(L3_VOL_DROP_EN_ADDR(base));
	if (!en)
		reg_value = reg_value & (~L3_VOL_DROP_EN_BIT);
	else
		reg_value = reg_value | L3_VOL_DROP_EN_BIT;
	writel(reg_value, L3_VOL_DROP_EN_ADDR(base));
	pr_err("[%s]enable cpu l3:%d!\n", __func__, en);
}

static void hisi_vbat_drop_gpu_en(struct hisi_vbat_drop_protect_dev *di, int en)
{
	void __iomem *base = di->core_gpu_base;
	u32 reg_value;

	if (di->gpu_auto_div_en == 0)
		return;
	if (!base) {
		pr_err("%s:base is NULL\n", __func__);
		return;
	}
	reg_value = readl(GPU_VOL_DROP_EN_ADDR(base));
	if (!en)
		reg_value = reg_value & (~GPU_VOL_DROP_EN_BIT);
	else
		reg_value = reg_value | GPU_VOL_DROP_EN_BIT;
	writel(reg_value, GPU_VOL_DROP_EN_ADDR(base));
	pr_err("[%s]enable gpu:%d!\n", __func__, en);
}

#if defined (CONFIG_HISI_VBAT_DROP_PRT_ATLA)
static void hisi_vbat_drop_npu_en(struct hisi_vbat_drop_protect_dev *di, int en)
{

}
#else
static void hisi_vbat_drop_npu_en(struct hisi_vbat_drop_protect_dev *di, int en)
{
	void __iomem *base = di->core_npu_base;
	u32 reg_value;

	if (di->npu_auto_div_en == 0)
		return;
	if (!base) {
		pr_err("%s:base is NULL\n", __func__);
		return;
	}
	reg_value = readl(NPU_VOL_DROP_EN_ADDR(base));
	if (!en)
		reg_value = reg_value & (~NPU_VOL_DROP_EN_BIT);
	else
		reg_value = reg_value | NPU_VOL_DROP_EN_BIT;
#if defined(CONFIG_HISI_VBAT_DROP_PRT_ORL)
	reg_value = reg_value | NPU_VOL_DROP_MASK;
#endif
	writel(reg_value, NPU_VOL_DROP_EN_ADDR(base));
	pr_err("[%s]enable cpu npu:%d!\n", __func__, en);
}
#endif
/*******************************************************
 * Function:     hisi_vbat_drop_cpu_drop_en
 * Description:  enable or disable Automatic 1/2 Frequency
 *               Division(cpu/gpu/l3)
 * Input:        struct hisi_vbat_drop_protect_dev *di,   ---- vbat drop
 *                 protect device
 *	   enum drop_freq_en core  :div core type
 *	   int en: 1:enable,0:disable
 * Output:       NULL
 * Return:       NULL
 * Remark:       pmuctrl registers for cpu div control can't Simultaneous been
 *                writted by lpm3
 ********************************************************/
static void hisi_vbat_drop_en(
	struct hisi_vbat_drop_protect_dev *di, enum drop_freq_en core, int en)
{
	if (!di) {
		pr_err("%s:di is NULL\n", __func__);
		return;
	}

	switch (core) {
	case BIG_CPU:
		hisi_vbat_drop_big_cpu_en(di, en);
		break;
	case MIDDLE_CPU:
		hisi_vbat_drop_mid_cpu_en(di, en);
		break;
	case LITTLE_CPU:
		hisi_vbat_drop_lit_cpu_en(di, en);
		break;
	case L3_CPU:
		hisi_vbat_drop_l3_cpu_en(di, en);
		break;
	case GPU_CPU:
		hisi_vbat_drop_gpu_en(di, en);
		break;
	case NPU_CPU:
		hisi_vbat_drop_npu_en(di, en);
		break;
	case ALL:
		hisi_vbat_drop_big_cpu_en(di, en);
		hisi_vbat_drop_mid_cpu_en(di, en);
		hisi_vbat_drop_lit_cpu_en(di, en);
		hisi_vbat_drop_l3_cpu_en(di, en);
		hisi_vbat_drop_gpu_en(di, en);
		hisi_vbat_drop_npu_en(di, en);
		break;
	default:
		pr_err("[%s]set cpu nothing!\n", __func__);
		break;
	}
}

/*******************************************************
 * Function:        vbat_drop_vol_set
 * Description:     set vbat drop vol interrupt Threshold value
 * Input:           mv :vbat drop interrupt  Threshold value
 * Output:          NULL
 * Return:          NULL
 * Rmark:           NA
 ********************************************************/
static void hisi_vbat_drop_vol_set(int mv)
{
	unsigned char reg_val;
#ifdef CONFIG_HISI_HI6421V700_PMU
	if (mv > 3400)
		reg_val = 7;
	else if (mv > 3300)
		reg_val = 6;
	else if (mv > 3200)
		reg_val = 5;
	else if (mv > 3100)
		reg_val = 4;
	else if (mv > 3000)
		reg_val = 3;
	else if (mv > 2900)
		reg_val = 2;
	else if (mv > 2800)
		reg_val = 1;
	else
		reg_val = 0;
#else
	if (mv > 3400)
		reg_val = 6;
	else if (mv > 3250)
		reg_val = 5;
	else if (mv > 3100)
		reg_val = 4;
	else if (mv > 2950)
		reg_val = 3;
	else if (mv > 2800)
		reg_val = 2;
	else if (mv > 2700)
		reg_val = 1;
	else
		reg_val = 0;
#endif
	HISI_VBAT_DROP_PMIC_REG_WRITE(PMIC_VSYS_DROP_VOL_SET, reg_val);
	pr_info("[%s]:set vol [%d]mv reg [%d]!\n", __func__, mv, reg_val);
}

/*******************************************************
 *  Function:        hisi_vbat_drop_protect_probe
 *  Description:     probe function
 *  Input:           struct platform_device *pdev --- platform device
 *  Output:          NULL
 *  Return:          0:success,other:fail.
 *  Rmark:           enable cpu/gpu auto div, registe irq.
 ********************************************************/
static int hisi_vbat_drop_protect_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct hisi_vbat_drop_protect_dev *di = NULL;
	int ret = 0;

	di = devm_kzalloc(dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;
	di->dev = dev;
	platform_set_drvdata(pdev, di);

	/*get dts info*/
	ret = hisi_vbat_drop_parse_dts(di);
	if (ret)
		goto vbat_drop_err;

	/*request vbat drop vol irq*/
	di->vbat_drop_irq = platform_get_irq(pdev, 0);
	if (di->vbat_drop_irq < 0) {
		dev_err(dev, "IRQ resource(%d) is not available\n",
			di->vbat_drop_irq);
		ret = -1;
		goto vbat_drop_err;
	}

	/*mask && clear old IRQ status*/
	HISI_VBAT_DROP_PMIC_REG_WRITE(
		PMIC_VSYS_DROP_IRQ_MASK_REG, PMIC_VSYS_DROP_IRQ_MASK);
	HISI_VBAT_DROP_PMIC_REG_WRITE(
		PMIC_VSYS_DROP_IRQ_REG, PMIC_VSYS_DROP_IRQ_CLEAR);

	ret = devm_request_irq(dev, di->vbat_drop_irq,
		hisi_vbat_drop_irq_handler, IRQF_TRIGGER_RISING, "vbat_drop",
		di);
	if (ret) {
		dev_err(dev, "could not claim vbat drop irq with err:%d\n",
			ret);
		ret = -1;
		goto vbat_drop_err;
	}
	/*init vbat drop work*/
	INIT_DELAYED_WORK(
		&di->vbat_drop_irq_work, hisi_vbat_drop_interrupt_work);

	wakeup_source_init(&di->vbatt_check_lock,
		"vbatt_drop_check_wake");

#ifdef CONFIG_HISI_HW_VOTE
	hisi_vbat_drop_cluster_freq_limit_init(dev);
#endif
	/*set vbat drop vol*/
	hisi_vbat_drop_vol_set(di->vbat_drop_vol_mv);

	/*enable cpu/gpu auto drop*/
	hisi_vbat_drop_en(di, ALL, 1);

	/*unmask pmu vsys_drop interrupt*/
	HISI_VBAT_DROP_PMIC_REG_WRITE(
		PMIC_VSYS_DROP_IRQ_MASK_REG, ~PMIC_VSYS_DROP_IRQ_MASK);

	dev_err(dev, "[%s] probe success!\n", __func__);
	return 0;

vbat_drop_err:
	platform_set_drvdata(pdev, NULL);
	dev_err(dev, "[%s] probe fail!\n", __func__);
	return ret;
}

static int hisi_vbat_drop_protect_remove(struct platform_device *pdev)
{
	struct hisi_vbat_drop_protect_dev *di = platform_get_drvdata(pdev);

	if (!di) {
		pr_err("%s:di is NULL\n", __func__);
		return -ENOMEM;
	}

	devm_free_irq(di->dev, di->vbat_drop_irq, di);
	devm_kfree(&pdev->dev, di);
	return 0;
}

#ifdef CONFIG_PM
static int hisi_vbat_drop_protect_suspend(
	struct platform_device *pdev, pm_message_t state)
{
	struct hisi_vbat_drop_protect_dev *di = platform_get_drvdata(pdev);

	if (!di) {
		pr_err("[%s]vbat drop dev is NULL\n", __func__);
		return -ENOMEM;
	}

	pr_info("%s: suspend +\n", __func__);

	/*cancle cpu/gpu auto div*/
	hisi_vbat_drop_en(di, ALL, 0);

	pr_info("%s: suspend -\n", __func__);

	return 0;
}

static int hisi_vbat_drop_protect_resume(struct platform_device *pdev)
{
	struct hisi_vbat_drop_protect_dev *di = platform_get_drvdata(pdev);

	if (!di) {
		pr_err("%s:di is NULL\n", __func__);
		return -ENOMEM;
	}

	pr_info("%s: resume +\n", __func__);

	/*enable cpu/gpu auto div*/
	hisi_vbat_drop_en(di, ALL, 1);

	pr_info("%s: resume -\n", __func__);
	return 0;
}
#endif

static const struct of_device_id hisi_vbat_drop_protect_of_match[] = {
	{
		.compatible = "hisilicon,vbat-drop-protect",
	},
	{},
};

MODULE_DEVICE_TABLE(of, hisi_vbat_drop_protect_of_match);

static struct platform_driver hisi_vbat_drop_protect_driver = {
	.probe = hisi_vbat_drop_protect_probe,
	.remove = hisi_vbat_drop_protect_remove,
	.driver = {
			.owner = THIS_MODULE,
			.name = "hisi_vbat_drop_prot",
			.of_match_table = hisi_vbat_drop_protect_of_match,
		},
#ifdef CONFIG_PM
	.suspend = hisi_vbat_drop_protect_suspend,
	.resume = hisi_vbat_drop_protect_resume,
#endif
};

module_platform_driver(hisi_vbat_drop_protect_driver);

MODULE_AUTHOR("HISILICON");
MODULE_DESCRIPTION("Vbat drop protect driver");
MODULE_LICENSE("GPL v2");
