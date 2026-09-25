/*
 * hisi_ip_regulator.c
 *
 * hisilicon ip regulator poweron and poweroff
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
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

#include "../mfd/hisi_freq_autodown.h"
#include <asm/compiler.h>
#include <linux/clk.h>
#include <linux/compiler.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/mfd/hisi_ip_core.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/version.h>
#include <securec.h>
#include <linux/clk-provider.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/hwspinlock.h>
#include <linux/interrupt.h>
#include <soc_crgperiph_interface.h>
#ifdef CONFIG_HISI_PMIC_DEBUG
#include <linux/debugfs.h>
#endif
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#if CONFIG_DEBUG_FS
#define IP_REGULATOR_DEBUG(args...) pr_debug(args);
#else
#define IP_REGULATOR_DEBUG(args...)
#endif

#if defined CONFIG_MIA_IP_PLATFORM
enum ip_regulator_id {
	MEDIA1_SUBSYS_ID = 0,
	MEDIA2_SUBSYS_ID,
	VIVOBUS_ID,
	IVP_ID,
	VCODECSUBSYS_ID,
	DSSSUBSYS_ID,
	ISPSUBSYS_ID,
	VDEC_ID,
	VENC_ID,
	G3D_ID,
	ASP_ID,
};
#elif defined CONFIG_BOST_IP_PLATFORM
enum ip_regulator_id {
	MEDIA1_SUBSYS_ID = 0,
	VIVOBUS_ID,
	VCODECSUBSYS_ID,
	IVP_ID,
	DSSSUBSYS_ID,
	ISPSUBSYS_ID,
	VDEC_ID,
	VENC_ID,
	ISP_SRT_ID,
	G3D_ID,
	ASP_ID,
	MMBUF_ID,
};
#elif defined CONFIG_ATLA_IP_PLATFORM
enum ip_regulator_id {
	MEDIA1_SUBSYS_ID = 0,
	MEDIA2_SUBSYS_ID,
	VIVOBUS_ID,
	VCODECSUBSYS_ID,
	DSSSUBSYS_ID,
	ISPSUBSYS_ID,
	IVP_ID,
	VDEC_ID,
	VENC_ID,
	ICS_ID,
	ISP_R8_ID,
	G3D_ID,
	ASP_ID,
	ICS2_ID,
};
#elif defined CONFIG_PHOE_IP_PLATFORM
enum ip_regulator_id {
	VIVOBUS_ID = 0,
	VCODECSUBSYS_ID,
	DSSSUBSYS_ID,
	ISPSUBSYS_ID,
	IVP_ID,
	VDEC_ID,
	VENC_ID,
	ISP_R8_ID,
	VENC2_ID,
	HIFACE_ID,
	MEDIA1_SUBSYS_ID,
	MEDIA2_SUBSYS_ID,
	NPU_ID,
	G3D_ID,
	ASP_ID,
};
#elif defined CONFIG_IP_PLATFORM_COMMON
enum ip_regulator_id {
	MEDIA1_SUBSYS_ID = 0,
	MEDIA2_SUBSYS_ID,
	VIVOBUS_ID,
	VCODECSUBSYS_ID,
	DSSSUBSYS_ID,
	ISPSUBSYS_ID,
	IVP_ID,
	VDEC_ID,
	VENC_ID,
	ISP_R8_ID,
	ASP_ID,
	G3D_ID,
#ifndef CONFIG_BALT_IP_PLATFORM
	ICS_ID,
	ICS2_ID,
#endif
	NPU_ID,
	IVP1_ID,
	HIFACE_ID,
	ARPP_ID,
	IPP_ID,
};
#else
enum ip_regulator_id {
	VIVOBUS_ID = 0,
	DSSSUBSYS_ID,
	VCODECSUBSYS_ID,
	VDEC_ID,
	VENC_ID,
	ISPSUBSYS_ID,
	IVP_ID,
	DEBUGSUBSYS_ID,
};
#endif

/* ***************************** macro parameter ****************************** */
enum {
	REGULATOR_DISABLE = 0,
	REGULATOR_ENABLE
};

/*
 * sctrl 0x438 :        power state vote
 * bit[0]: AP           media1 vote
 * bit[1]: Sensorhub    media1 vote
 * bit[2]: AP           vivobus vote
 * bit[3]: Sensorhub    vivobus vote
 * bit[30]: TEEOS       media1 vote
 * bit[31]: TEEOS       vivobus vote
 */
enum {
	AP_MEDIA1_VOTE      = 0,
	IOMCU_MEDIA1_VOTE   = 1,
	AP_VIVOBUS_VOTE     = 2,
	IOMCU_VIVOBUS_VOTE  = 3,
	AP_DSS_VOTE         = 4,
	IOMCU_DSS_VOTE      = 5,
	TEEOS_MEDIA1_VOTE   = 30,
	TEEOS_VIVOBUS_VOTE  = 31
};

#define AP_TO_LPM_MSG_NUM 2
#define REGULATOR_HWLOCK_TIMEOUT 1000
#define AP_AOD_EN 0
#define SENSORHUB_AOD_EN 1

#define VOTE_STATE(val) ((val) == 1 ? 0 : 1)
#define VIVOBUS_VOTE_STATE(val) ((((val) & BIT(TEEOS_VIVOBUS_VOTE)) == 0) && \
				(((val) & BIT(IOMCU_VIVOBUS_VOTE)) == 0))

#define MEDIA1_SUBSYS_VOTE_STATE(val) ((((val) & BIT(TEEOS_MEDIA1_VOTE)) == 0) && \
				(((val) & BIT(IOMCU_MEDIA1_VOTE)) == 0))

#define DSSSUBSYS_VOTE_STATE(val) (((val) & BIT(IOMCU_DSS_VOTE)) == 0)

static struct hwspinlock *regulator_hwlock_29;

/* **************************** ip struct parameter *************************** */
struct hisi_regulator_ip {
	const char *name;
	struct device_node *np;
	struct regulator_desc rdesc;
	int (*dt_parse)(struct hisi_regulator_ip *, struct platform_device *);
	int regulator_id;
	struct mutex regulator_lock;
	u32 lpm_enable_value[AP_TO_LPM_MSG_NUM];
	u32 lpm_disable_value[AP_TO_LPM_MSG_NUM];
	u32 regulator_enalbe_flag;
	int regulator_fake;
	int regulator_type;
	int clock_num;
	int clock_check_flag;
	int enable_clock;
	int clock_set_rate_flag;
	unsigned int *clock_rate_value;
	unsigned int *clock_org_rate;
	int enable_autofreqdump;
	unsigned int ppll0_clock_set_rate_flag;
	unsigned int ppll0_clock_rate_value;
	void __iomem *sctrl_reg;
	int hwlock_seq;    /* aod hwlock sequnce */
	u32 aod_sc_offset; /* SC offset */

#if defined(CONFIG_BOST_IP_PLATFORM)      || \
	defined(CONFIG_MIA_IP_PLATFORM)   || \
	defined(CONFIG_ATLA_IP_PLATFORM)  || \
	defined(CONFIG_PHOE_IP_PLATFORM)  || \
	defined(CONFIG_IP_PLATFORM_COMMON)
	u32 dss_boot_check[2]; /* DSS need check softreset state */
#endif
};

#if defined(CONFIG_BOST_IP_PLATFORM)      || \
	defined(CONFIG_MIA_IP_PLATFORM)   || \
	defined(CONFIG_ATLA_IP_PLATFORM)  || \
	defined(CONFIG_PHOE_IP_PLATFORM)  || \
	defined(CONFIG_IP_PLATFORM_COMMON)
#define DSS_SOFTRESET_STATE_CHECK_BIT 0
#else
#define DSS_SOFTRESET_STATE_CHECK_BIT SOC_CRGPERIPH_PERRSTEN3_ip_rst_dss_START
#endif
/* ************************* control interface ******************************* */
#ifdef CONFIG_HISI_FREQ_AUTODOWN
static int hisi_regulator_freq_autodown_clk(int regulator_id, u32 flag)
{
	int ret = 0;

	IP_REGULATOR_DEBUG("<[%s]: begin regulator_id=%d, flag=%u\n",
		__func__, regulator_id, flag);
	switch (regulator_id) {
	case VIVOBUS_ID:
		ret = hisi_freq_autodown_clk_set("vivobus", flag);
		break;
	case VCODECSUBSYS_ID:
		ret = hisi_freq_autodown_clk_set("vcodecbus", flag);
		break;
	case VDEC_ID:
		ret = hisi_freq_autodown_clk_set("vdecbus", flag);
		break;
	case VENC_ID:
		ret = hisi_freq_autodown_clk_set("vencbus", flag);
		break;
	case ISPSUBSYS_ID:
		ret = hisi_freq_autodown_clk_set("ispa7bus", flag);
		break;
	case IVP_ID:
		ret = hisi_freq_autodown_clk_set("ivpbus", flag);
		break;
#if defined(CONFIG_ATLA_IP_PLATFORM)
	case ICS_ID:
		ret = hisi_freq_autodown_clk_set("icsbus", flag);
		break;
	case ICS2_ID:
		ret = hisi_freq_autodown_clk_set("ics2bus", flag);
		break;
#endif
#if defined(CONFIG_PHOE_IP_PLATFORM)
	case VENC2_ID:
		ret = hisi_freq_autodown_clk_set("venc2bus", flag);
		break;
#endif
#if defined(CONFIG_BALT_IP_PLATFORM)
	case IVP1_ID:
		ret = hisi_freq_autodown_clk_set("ivp1bus", flag);
		break;
#endif
	default:
		pr_err("[%s]:Input regulator ID is not exist\n", __func__);
		return -1;
	}
	IP_REGULATOR_DEBUG("<[%s]: end regulator_id=%d>\n", __func__,
		regulator_id);
	return ret;
}
#endif

static int aod_check_bus_pwstat_vote(struct hisi_regulator_ip *sreg)
{
	int ret = 0;
	unsigned int val;
	unsigned int vote_val;

	val = readl(sreg->sctrl_reg + sreg->aod_sc_offset);
	switch (sreg->regulator_id) {
	case VIVOBUS_ID:
		vote_val = VIVOBUS_VOTE_STATE(val);
		ret = VOTE_STATE(vote_val);
		break;
	case MEDIA1_SUBSYS_ID:
		vote_val = MEDIA1_SUBSYS_VOTE_STATE(val);
		ret = VOTE_STATE(vote_val);
		break;
	case DSSSUBSYS_ID:
		vote_val = DSSSUBSYS_VOTE_STATE(val);
		ret = VOTE_STATE(vote_val);
		break;
	default:
		break;
	}
	return ret;
}

/* Always on display need to vote media1,vivobus power on/off in sctrl */
static int aod_set_and_get_poweron_state(struct hisi_regulator_ip *sreg,
	u32 control_flag)
{
	int ret;
	unsigned int bit_shift = 0;
	unsigned int val;

	/*
	 * sctrl 0x438 :	power state vote
	 * bit[0]:		AP media1 vote
	 * bit[1]:		Sensorhub media1 vote
	 * bit[2]:		AP vivobus vote
	 * bit[3]:		Sensorhub vivobus vote
	 * bit[4]:		AP dss vote
	 * bit[5]:		Sensorhub dss vote
	 * since p.h.x, add bit30\bit31
	 * bit[30]:		TEEOS media1 vote
	 * bit[31]:		TEEOS vivobus vote
	 */
	if (sreg->regulator_id == VIVOBUS_ID)
		bit_shift = 2;
	else if (sreg->regulator_id == DSSSUBSYS_ID)
		bit_shift = 4;

	val = readl(sreg->sctrl_reg + sreg->aod_sc_offset);

	ret = aod_check_bus_pwstat_vote(sreg);

	if (control_flag == REGULATOR_ENABLE) /* vote AP media/vivobus power up */
		val |= (BIT(AP_AOD_EN) << bit_shift);
	else /* vote AP media/vivobus power off */
		val &= ~(BIT(AP_AOD_EN) << bit_shift);

	writel(val, sreg->sctrl_reg + sreg->aod_sc_offset);

	return ret;
}

static struct hisi_regulator_ip_core *rdev_to_ip_core(
	struct regulator_dev *rdev)
{
	/*
	 * regulator_dev parent to->
	 * hisi ip regulator platform device_dev parent to->
	 * hisi ip pmic platform device_dev
	 */
	struct device *dev = NULL;

	if (!rdev)
		return NULL;

	dev = rdev_get_dev(rdev);
	if (!dev)
		return NULL;

	return dev_get_drvdata(dev->parent->parent);
}

static struct clk *of_regulator_clk_get(struct device_node *node, int index)
{
	struct clk *clk = NULL;
	const char *clk_name = NULL;
	int ret;

	IP_REGULATOR_DEBUG("<[%s]: begin>\n", __func__);
	ret = of_property_read_string_index(node, "clock-names", index,
		&clk_name);
	if (ret != 0) {
		pr_err("failed to get regulator clk resource! ret=%d\n", ret);
		return ERR_PTR(-EINVAL);
	}
	clk = clk_get(NULL, clk_name);
	if (IS_ERR(clk)) {
		pr_err(" %s: %s %d get failed!\n", __func__, clk_name,
			__LINE__);
		return ERR_PTR(-EINVAL);
	}
	IP_REGULATOR_DEBUG("<[%s]: end>\n", __func__);
	return clk;
}

static int hisi_clock_state_check(struct hisi_regulator_ip *sreg)
{
	struct clk *temp_clock = NULL;
	int i;
	bool clock_flag = false;

	IP_REGULATOR_DEBUG("<[%s]: begin regulator_id=%d>\n", __func__,
		sreg->regulator_id);
	for (i = 0; i < sreg->clock_num; i++) {
		temp_clock = of_regulator_clk_get(sreg->np, i);
		if (IS_ERR(temp_clock))
			return -EINVAL;

		IP_REGULATOR_DEBUG("<[%s]: clock_name=%s>\n", __func__,
			__clk_get_name(temp_clock));
		clock_flag = __clk_is_enabled(temp_clock);
		if (clock_flag == false)
			pr_err("<[%s]: clock_name=%s had closed!>\n", __func__,
				__clk_get_name(temp_clock));

		clk_put(temp_clock);
		temp_clock = NULL;
	}
	IP_REGULATOR_DEBUG("<[%s]: end regulator_id=%d>\n", __func__,
		sreg->regulator_id);
	return 0;
}
#if defined(CONFIG_BOST_IP_PLATFORM)      || \
	defined(CONFIG_MIA_IP_PLATFORM)   || \
	defined(CONFIG_ATLA_IP_PLATFORM)  || \
	defined(CONFIG_PHOE_IP_PLATFORM)  || \
	defined(CONFIG_IP_PLATFORM_COMMON)
static int get_softreset_state(struct hisi_regulator_ip_core *pmic,
	struct hisi_regulator_ip *sreg, unsigned int value)
{
	int ret;
	unsigned int val;

	/* First boot and AOD to camera need check DSS poweron status */
	if (sreg->hwlock_seq > 0) {
		if (!regulator_hwlock_29) {
			pr_err("[%s]regulator hwlock_29 hwspinlock is null!\n",
				__func__);
			return -ENOENT;
		}

		if (hwspin_lock_timeout(regulator_hwlock_29,
			REGULATOR_HWLOCK_TIMEOUT)) {
			pr_err("Aod regulator enable hwspinlock timout!\n");
			return -ENOENT;
		}
	}
	IP_REGULATOR_DEBUG("<[%s]: sreg->dss_boot_check[0]=0x%x>\n", __func__,
		sreg->dss_boot_check[0]);
	IP_REGULATOR_DEBUG("<[%s]: regulator_id[%d] softreset_value=0x%x softreset_state3 = 0x%x>\n",
		__func__, sreg->regulator_id, sreg->dss_boot_check[1],
		readl((sreg->dss_boot_check[0] + pmic->regs)));
	if (((sreg->dss_boot_check[1]) &
	    ((unsigned int)readl(sreg->dss_boot_check[0] + pmic->regs))))
		ret = 0; /* softreset */
	else
		ret = 1; /* unreset */
	if (sreg->hwlock_seq > 0) {
		if (ret == 1) {
			val = readl(sreg->sctrl_reg + sreg->aod_sc_offset);
			val |= (BIT(4)); /* 4: AOD DSS vote bit */
			pr_err("[%s]AOD DSS vote val = 0x%x !\n",
				__func__, val);
			/* vote aod dss enable */
			writel(val, sreg->sctrl_reg + sreg->aod_sc_offset);
		}
		hwspin_unlock(regulator_hwlock_29);
	}
	return ret;
}
#else
static int get_softreset_state(struct hisi_regulator_ip_core *pmic,
	struct hisi_regulator_ip *sreg, unsigned int value)
{
	IP_REGULATOR_DEBUG("<[%s]: regulator_id[%d] softreset_bit=%u softreset_state3 = 0x%x>\n",
		__func__, sreg->regulator_id, value,
		readl(SOC_CRGPERIPH_PERRSTSTAT3_ADDR(pmic->regs)));
	if (((1 << value) &
	    (readl(SOC_CRGPERIPH_PERRSTSTAT3_ADDR(pmic->regs)))))
		return 0; /* softreset */
	else
		return 1; /* unreset */
}
#endif
static int hisi_ppll0_clock_rate_set(struct hisi_regulator_ip *sreg)
{
	struct clk *temp_clock = NULL;
	int ret;

	IP_REGULATOR_DEBUG("<[%s]: begin regulator_id=%d clock_num=%d>\n",
		__func__, sreg->regulator_id, sreg->clock_num);
	/* 0:get current clock */
	temp_clock = of_regulator_clk_get(sreg->np, 0);
	if (IS_ERR(temp_clock))
		return -EINVAL;

	IP_REGULATOR_DEBUG("<[%s]: clock_name=%s, clock_rate=0x%x>\n", __func__,
		__clk_get_name(temp_clock), sreg->ppll0_clock_rate_value);
	/* 1:set freq rate to clock */
	ret = clk_set_rate(temp_clock, sreg->ppll0_clock_rate_value);
	if (ret < 0) {
		pr_err("file:%s line:%d temp clock[%s] set rate fail!\n",
			__func__, __LINE__, __clk_get_name(temp_clock));
		ret = -EINVAL;
	}
	IP_REGULATOR_DEBUG("<[%s]: end regulator_id=%d>\n",
		__func__, sreg->regulator_id);
	clk_put(temp_clock);
	return ret;
}

static int hisi_low_rate_set(struct hisi_regulator_ip *sreg)
{
	int i;
	int ret = 0;
	struct clk *temp_clock = NULL;

	for (i = 0; i < sreg->clock_num; i++) {
		if (sreg->clock_rate_value[i] == 0)
			continue;
		/* 0:get current clock */
		temp_clock = of_regulator_clk_get(sreg->np, i);
		if (IS_ERR(temp_clock))
			return -EINVAL;

		IP_REGULATOR_DEBUG("<[%s]: clock_name=%s>\n",
			__func__, __clk_get_name(temp_clock));
		/* 1:get clock org value */
		sreg->clock_org_rate[i] = clk_get_rate(temp_clock);
		/* 2:set low freq rate to clock */
		ret = clk_set_rate(temp_clock, sreg->clock_rate_value[i]);
		if (ret < 0) {
			pr_err("file:%s line:%d temp clock[%s] set rate fail!\n",
				__func__, __LINE__, __clk_get_name(temp_clock));
			goto clk_err_put;
		}
		IP_REGULATOR_DEBUG("<[%s]: begin clock_name = %s clock_org_rate[%d]=%u, clock_current_rate[%d]=%u>\n",
			__func__, __clk_get_name(temp_clock), i,
			sreg->clock_org_rate[i], i, sreg->clock_rate_value[i]);
		IP_REGULATOR_DEBUG("<[%s]: current clock_name[%s] rate[%lu]>\n",
			__func__, __clk_get_name(temp_clock),
			clk_get_rate(temp_clock));
		clk_put(temp_clock);
		temp_clock = NULL;
	}

clk_err_put:
	clk_put(temp_clock);
	return ret;
}

static int hisi_resume_rate(struct hisi_regulator_ip *sreg)
{
	int i;
	int ret = 0;
	struct clk *temp_clock = NULL;

	for (i = 0; i < sreg->clock_num; i++) {
		if (sreg->clock_rate_value[i] == 0)
			continue;
		/* 0:get current clock */
		temp_clock = of_regulator_clk_get(sreg->np, i);
		if (IS_ERR(temp_clock))
			return -EINVAL;

		/* 1:resume org clock rate */
		ret = clk_set_rate(temp_clock, sreg->clock_org_rate[i]);
		if (ret < 0) {
			pr_err("file:%s line:%d temp clock[%s] set rate fail!\n",
				__func__, __LINE__, __clk_get_name(temp_clock));
			goto clk_err_put;
		}
		IP_REGULATOR_DEBUG("<[%s]: end clock_name = %s clock_org_rate[%d]=%u>\n",
			__func__, __clk_get_name(temp_clock), i,
			sreg->clock_org_rate[i]);
		IP_REGULATOR_DEBUG("<[%s]: current clock_name[%s] rate[%lu]>\n",
			__func__, __clk_get_name(temp_clock),
			clk_get_rate(temp_clock));
		clk_put(temp_clock);
		temp_clock = NULL;
	}
clk_err_put:
	clk_put(temp_clock);
	return ret;
}


static int hisi_clock_rate_set(struct hisi_regulator_ip *sreg, int flag)
{
	int ret;

	IP_REGULATOR_DEBUG("<[%s]: begin regulator_id=%d clock_num=%d>\n",
		__func__, sreg->regulator_id, sreg->clock_num);
	if (flag == 0) { /* save */
		ret = hisi_low_rate_set(sreg);
		if (ret < 0)
			return ret;
	} else {
		ret = hisi_resume_rate(sreg);
		if (ret < 0)
			return ret;
	}
	IP_REGULATOR_DEBUG("<[%s]: end regulator_id=%d>\n", __func__,
		sreg->regulator_id);
	return 0;
}

static int hisi_ip_clock_work(struct hisi_regulator_ip *sreg, int flag)
{
	struct clk *temp_clock = NULL;
	int i;
	int ret = 0;

	IP_REGULATOR_DEBUG("<[%s]: begin regulator_id=%d, clock_num = %d>\n",
		__func__, sreg->regulator_id, sreg->clock_num);
	for (i = 0; i < sreg->clock_num; i++) {
		temp_clock = of_regulator_clk_get(sreg->np, i);
		if (IS_ERR(temp_clock))
			return -EINVAL;

		IP_REGULATOR_DEBUG("<[%s]: clock_name = %s>\n", __func__,
			__clk_get_name(temp_clock));
		/* flag: 0 disable clock; 1 enable clock */
		if (flag == REGULATOR_ENABLE) {
			ret = clk_prepare_enable(temp_clock);
			if (ret) {
				pr_err("Regulator:regulator_id[%d], clock_id[%d] enable failed\n",
					sreg->regulator_id, i);
				clk_put(temp_clock);
				return ret;
			}
			IP_REGULATOR_DEBUG("<[%s]: clock_name = %s enable>\n",
				__func__, __clk_get_name(temp_clock));
		} else {
			IP_REGULATOR_DEBUG("<[%s]: clock_name = %s disable>\n",
				__func__, __clk_get_name(temp_clock));
			clk_disable_unprepare(temp_clock);
		}
		clk_put(temp_clock);
		temp_clock = NULL;
	}
	IP_REGULATOR_DEBUG("<[%s]: end regulator_id=%d>\n", __func__,
		sreg->regulator_id);
	return ret;
}

/* ************************* arm trust firmware ******************************* */
#define IP_REGULATOR_REGISTER_FN_ID 0xc500fff0
noinline int atfd_hisi_service_ip_regulator_smc(u64 _function_id, u64 _arg0,
	u64 _arg1, u64 _arg2)
{
	register u64 function_id asm("x0") = _function_id;
	register u64 arg0 asm("x1") = _arg0;
	register u64 arg1 asm("x2") = _arg1;
	register u64 arg2 asm("x3") = _arg2;
	asm volatile(
			__asmeq("%0", "x0")
			__asmeq("%1", "x1")
			__asmeq("%2", "x2")
			__asmeq("%3", "x3")
			"smc    #0\n"
			: "+r"(function_id)
			: "r"(arg0), "r"(arg1), "r"(arg2));

	return (int)function_id;
}

/* ****************************** lpmcu *************************************** */
static int hisi_ip_to_atf_is_enabled(struct regulator_dev *dev)
{
	struct hisi_regulator_ip *sreg = rdev_get_drvdata(dev);
	struct hisi_regulator_ip_core *pmic = rdev_to_ip_core(dev);

	if (!sreg || !pmic) {
		pr_err("[%s]regulator get  para is err!\n", __func__);
		return -EINVAL;
	}
	IP_REGULATOR_DEBUG("<[%s]:regulator_id=%d>\n", __func__,
		sreg->regulator_id);
	return sreg->regulator_enalbe_flag;
}

static int hisi_ip_power_set(struct hisi_regulator_ip *sreg, u32 control_flag)
{
	int ret = 0;
	int aod_ret = 0;

	if (sreg->hwlock_seq > 0) {
		if (!regulator_hwlock_29) {
			pr_err("[%s] regulator hwlock_29 hwspinlock is null!\n",
				__func__);
			return -ENOENT;
		}

		if (hwspin_lock_timeout(regulator_hwlock_29,
			REGULATOR_HWLOCK_TIMEOUT)) {
			pr_err("Aod regulator enable hwspinlock timout!\n");
			return -ENOENT;
		}
		aod_ret = aod_set_and_get_poweron_state(sreg, control_flag);
		if (aod_ret == 0) {
			ret = atfd_hisi_service_ip_regulator_smc(
				IP_REGULATOR_REGISTER_FN_ID, sreg->regulator_id,
				control_flag, 0);
		} else if (aod_ret == 1) {
			ret = 0;
		} else {
			pr_err("%s:aod set and get power state [%s] failled!\n",
				__func__, sreg->name);
		}
		hwspin_unlock(regulator_hwlock_29);
	} else {
		ret = atfd_hisi_service_ip_regulator_smc(IP_REGULATOR_REGISTER_FN_ID,
			sreg->regulator_id, control_flag, 0);
	}
	return ret;
}

static int hisi_ip_power_on(struct hisi_regulator_ip *sreg)
{
	return hisi_ip_power_set(sreg, REGULATOR_ENABLE);
}

static int hisi_ip_power_off(struct hisi_regulator_ip *sreg)
{
	return hisi_ip_power_set(sreg, REGULATOR_DISABLE);
}

static int check_dss_is_power_on(struct hisi_regulator_ip *sreg,
	struct hisi_regulator_ip_core *pmic)
{
	int ret = 0;
	int softreset_value;

	if (sreg->regulator_id == DSSSUBSYS_ID) {
		softreset_value = get_softreset_state(pmic, sreg,
			DSS_SOFTRESET_STATE_CHECK_BIT);
		IP_REGULATOR_DEBUG("<[%s]: regulator_id[%d] softreset_bit=%d>\n",
			__func__, sreg->regulator_id, softreset_value);
		if (softreset_value) {
			IP_REGULATOR_DEBUG("<[%s]: regulator_id[%d] was poweron in fastboot phase>\n",
				__func__, sreg->regulator_id);
			sreg->regulator_enalbe_flag = 1;
			ret = 1;
		}
	}

	return ret;
}

static void clock_rate_set(struct hisi_regulator_ip *sreg, int flag)
{
	int ret;

	if (sreg->clock_set_rate_flag == 1) {
		ret = hisi_clock_rate_set(sreg, flag);
		if (ret)
			pr_err("%s:hisi clock rate set fail!\n", __func__);
	}
}

static void clock_state_check(struct hisi_regulator_ip *sreg)
{
	int ret;

	if (sreg->clock_check_flag == 1) {
		ret = hisi_clock_state_check(sreg);
		if (ret)
			pr_err("%s:hisi clock state check fail!\n", __func__);
	}
}

static void ip_clock_work(struct hisi_regulator_ip *sreg, int flag)
{
	int ret;

	if (sreg->enable_clock == 1) {
		ret = hisi_ip_clock_work(sreg, flag);
		if (ret)
			pr_err("[%s]hisi hisi_ip_clock_work is failed. ret[%d]!\n",
				__func__, ret);
	}
}

#ifdef CONFIG_HISI_FREQ_AUTODOWN
static void regulator_freq_autodown_clk(struct hisi_regulator_ip *sreg,
	u32 flag)
{
	int ret;

	if (sreg->enable_autofreqdump == 1) {
		ret = hisi_regulator_freq_autodown_clk(sreg->regulator_id,
			flag);
		if (ret)
			pr_err("[%s]hisi freq auto down is failed. ret[%d]!\n",
				__func__, ret);
	}
}
#endif

static int hisi_ip_to_atf_enabled(struct regulator_dev *dev)
{
	struct hisi_regulator_ip *sreg = rdev_get_drvdata(dev);
	struct hisi_regulator_ip_core *pmic = rdev_to_ip_core(dev);
	int ret;

	if (!sreg || !pmic) {
		pr_err("[%s]regulator get  para is err!\n", __func__);
		return -EINVAL;
	}

	IP_REGULATOR_DEBUG("\n<[%s]: begin regulator_id=%d>\n", __func__,
		sreg->regulator_id);
	if (sreg->regulator_fake) {
		sreg->regulator_enalbe_flag = 1;
		return 0;
	}

	/* this patch was resolved for dss power on in fastboot phase */
	if (check_dss_is_power_on(sreg, pmic))
		return 0;

	clock_rate_set(sreg, 0);

	clock_state_check(sreg);

	ip_clock_work(sreg, REGULATOR_ENABLE);

#ifdef CONFIG_HISI_FREQ_AUTODOWN
	regulator_freq_autodown_clk(sreg, REGULATOR_DISABLE);
#endif
	ret = hisi_ip_power_on(sreg);
	if (ret == 0) {
#ifdef CONFIG_HISI_FREQ_AUTODOWN
		regulator_freq_autodown_clk(sreg, REGULATOR_ENABLE);
#endif
		sreg->regulator_enalbe_flag = 1;
	} else {
		pr_err("%s:hisi ip send enable ldo[%s] to atf failled!\n\r",
			__func__, sreg->name);
	}

	clock_rate_set(sreg, 1);

	IP_REGULATOR_DEBUG("<[%s]: end regulator_id=%d>\n", __func__,
		sreg->regulator_id);
	return ret;
}

static int hisi_ip_to_atf_disabled(struct regulator_dev *dev)
{
	struct hisi_regulator_ip *sreg = rdev_get_drvdata(dev);
	int ret;

	if (!sreg) {
		pr_err("[%s]regulator get para is err!\n", __func__);
		return -EINVAL;
	}

	IP_REGULATOR_DEBUG("\n<[%s]: begin regulator_id=%d>\n", __func__,
		sreg->regulator_id);

	if (sreg->regulator_fake) {
		sreg->regulator_enalbe_flag = 0;
		return 0;
	}

	clock_state_check(sreg);

#ifdef CONFIG_HISI_FREQ_AUTODOWN
	regulator_freq_autodown_clk(sreg, REGULATOR_DISABLE);
#endif
	ret = hisi_ip_power_off(sreg);
	if (!ret) {
		sreg->regulator_enalbe_flag = 0;
		ret = 0;
	} else {
		pr_err("%s:hisi ip send disable ldo[%s] to atf failled!\n\r",
			__func__, sreg->name);
	}

	if (sreg->ppll0_clock_set_rate_flag == 1) {
		ret = hisi_ppll0_clock_rate_set(sreg);
		if (ret)
			pr_err("[%s]hisi ppll0_clock_set_rate_flag set failed. ret[%d]!\n",
				__func__, ret);
	}
	/* AOD need vivobus always on */
	ip_clock_work(sreg, REGULATOR_DISABLE);

	IP_REGULATOR_DEBUG("<[%s]: end regulator_id=%d>\n", __func__,
		sreg->regulator_id);
	return ret;
}

/* ********************************* lpmcu *********************************** */
#if defined(CONFIG_HISI_RPROC) || defined(CONFIG_HISI_IP_REGULATOR_IPC)
static int hisi_ip_regulator_cmd_send(struct regulator_dev *dev, int cmd)
{
	struct hisi_regulator_ip *sreg = rdev_get_drvdata(dev);
	rproc_msg_t ack_buffer[MBOX_CHAN_DATA_SIZE] = {0};
	rproc_msg_t *tx_buffer = NULL;
	int err;

	if (!sreg) {
		pr_err("[%s]regulator get  para is err!\n", __func__);
		return -EINVAL;
	}

	if (cmd == REGULATOR_ENABLE)
		tx_buffer = sreg->lpm_enable_value;
	else
		tx_buffer = sreg->lpm_disable_value;

	IP_REGULATOR_DEBUG("<[%s]: send msg to tx_buffer[0]=0x%x>\n", __func__,
		tx_buffer[0]);
	IP_REGULATOR_DEBUG("<[%s]: send msg to tx_buffer[1]=0x%x>\n", __func__,
		tx_buffer[1]);

	err = RPROC_SYNC_SEND(HISI_RPROC_LPM3_MBX14, tx_buffer,
		AP_TO_LPM_MSG_NUM, ack_buffer, AP_TO_LPM_MSG_NUM);
	if (err || (ack_buffer[0] != tx_buffer[0]) ||
		((ack_buffer[1] >> 24) != 0x0)) { /* [31:24] store ipc state */
		pr_err("%s: regulator ID[%d] rproc sync send err, err = %d, ack_buffer[0] = 0x%x, ack_buffer[1] = 0x%x!\n",
			__func__, sreg->regulator_id, err, ack_buffer[0],
			ack_buffer[1]);
		return -EINVAL;
	}

	return 0;
}

static int hisi_ip_to_lpm_is_enabled(struct regulator_dev *dev)
{
	struct hisi_regulator_ip *sreg = rdev_get_drvdata(dev);
	struct hisi_regulator_ip_core *pmic = rdev_to_ip_core(dev);

	if (!sreg || !pmic) {
		pr_err("[%s]regulator get  para is err!\n", __func__);
		return -EINVAL;
	}

	return sreg->regulator_enalbe_flag;
}

static int hisi_ip_to_lpm_enabled(struct regulator_dev *dev)
{
	struct hisi_regulator_ip *sreg = NULL;
	int ret;

	if (!dev)
		return -ENODEV;

	sreg = rdev_get_drvdata(dev);
	if (!sreg)
		return -EINVAL;

	IP_REGULATOR_DEBUG("\n<[%s]: begin regulator_id=%d>\n", __func__,
		sreg->regulator_id);

	if (sreg->regulator_fake) {
		sreg->regulator_enalbe_flag = 1;
		return 0;
	}

	ret = hisi_ip_regulator_cmd_send(dev, REGULATOR_ENABLE);
	if (!ret)
		sreg->regulator_enalbe_flag = 1;
	else
		pr_err("%s:hisi ip send enable ldo[%s] to lpm failled!\n\r",
			__func__, sreg->name);

	IP_REGULATOR_DEBUG("<[%s]: end regulator_id=%d>\n", __func__,
		sreg->regulator_id);
	return ret;
}

static int hisi_ip_to_lpm_disabled(struct regulator_dev *dev)
{
	struct hisi_regulator_ip *sreg;
	int ret;

	if (!dev)
		return -ENODEV;

	sreg = rdev_get_drvdata(dev);
	if (!sreg)
		return -EINVAL;

	IP_REGULATOR_DEBUG("\n<[%s]: begin regulator_id=%d>\n", __func__,
		sreg->regulator_id);

	if (sreg->regulator_fake) {
		sreg->regulator_enalbe_flag = 0;
		return 0;
	}

	ret = hisi_ip_regulator_cmd_send(dev, REGULATOR_DISABLE);
	if (!ret || (ret == -ETIMEOUT)) {
		sreg->regulator_enalbe_flag = 0;
		ret = 0;
	} else {
		pr_err("%s:hisi ip send disable ldo[%s] to lpm failled!\n",
			__func__, sreg->name);
	}
	IP_REGULATOR_DEBUG("<[%s]: end regulator_id=%d>\n", __func__,
		sreg->regulator_id);
	return ret;
}
#else
static int hisi_ip_to_lpm_is_enabled(struct regulator_dev *dev)
{
	return 0;
}

static int hisi_ip_to_lpm_enabled(struct regulator_dev *dev)
{
	return 0;
}

static int hisi_ip_to_lpm_disabled(struct regulator_dev *dev)
{
	return 0;
}
#endif

static int hisi_dt_parse_aod_atf(struct hisi_regulator_ip *sreg,
	struct device_node *np)
{
	u32 aod_resource[2] = {0}; /* aod hwlock resource and SC offset */

	if (of_find_property(np, "hisilicon,hisi-need-to-hwlock", NULL) != NULL) {
		if (of_property_read_u32_array(np, "hisilicon,hisi-need-to-hwlock",
			aod_resource, 2)) {
			pr_err("[%s]get hisilicon,hisi-need-to-hwlock attribute failed\n",
				__func__);
			return -ENODEV;
		}
	}
	sreg->hwlock_seq = aod_resource[0];
	sreg->aod_sc_offset = aod_resource[1];
	if (sreg->hwlock_seq > 0) {
		if (!regulator_hwlock_29) {
			regulator_hwlock_29 =
				hwspin_lock_request_specific(sreg->hwlock_seq);
			if (!regulator_hwlock_29) {
				pr_err("Aod regulator request hwspin lock failed!\n");
				return -ENODEV;
			}
		}
	}
	return 0;
}

static int clock_rate_value_get(struct hisi_regulator_ip *sreg,
	struct device *dev)
{
	int ret = 0;
	struct device_node *np = NULL;

	np = dev->of_node;
	if (sreg->clock_set_rate_flag == 1) {
		sreg->clock_rate_value = (u32 *)devm_kzalloc(dev,
			sizeof(u32) * sreg->clock_num, GFP_KERNEL);
		if (!sreg->clock_rate_value) {
			dev_err(dev, "[%s]kzalloc clock_rate_value buffer failed\n",
				__func__);
			return -ENOMEM;
		}

		sreg->clock_org_rate = (u32 *)devm_kzalloc(dev,
			sizeof(u32) * sreg->clock_num, GFP_KERNEL);
		if (!sreg->clock_org_rate) {
			dev_err(dev, "[%s]kzalloc clock_org_rate buffer failed\n",
				__func__);
			ret = -ENOMEM;
			goto get_rate_value_err;
		}

		ret = of_property_read_u32_array(np, "hisilicon,hisi-clock-rate-set",
			sreg->clock_rate_value, sreg->clock_num);
		if (ret) {
			dev_err(dev, "[%s]get hisilicon,hisi-clock-rate-set-flag attribute failed\n",
				__func__);
			ret = -ENODEV;
			goto get_old_rate_err;
		}
	}

	return ret;

get_old_rate_err:
	devm_kfree(dev, sreg->clock_org_rate);
get_rate_value_err:
	devm_kfree(dev, sreg->clock_rate_value);
	return ret;
}

static int __ppll0_clock_rate_value(struct hisi_regulator_ip *sreg, struct device_node *np)
{
	int ret = 0;

	IP_REGULATOR_DEBUG("<[%s]: regulator_id=%d, ppll0_clock_set_rate_flag=%u>\n",
		__func__, sreg->regulator_id,
		sreg->ppll0_clock_set_rate_flag);
	if (sreg->ppll0_clock_set_rate_flag == 1) {
		/* clock rate */
		ret = of_property_read_u32_array(np, "hisilicon,hisi-ppll0-clock-rate-set",
			&sreg->ppll0_clock_rate_value, 1);
		if (ret) {
			pr_err("[%s]get hisilicon,hisi-ppll0-clock-rate-set-flag attribute failed\n",
				__func__);
			return ret;
		}
	}

	return ret;
}

static int of_static_properties(struct hisi_regulator_ip *sreg,
	struct platform_device *pdev)
{
	int ret;
	int id = 0;
	int fake = 0;
	int type = 0;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;

	/* regulator-id */
	ret = of_property_read_u32_array(np, "hisilicon,hisi-regulator-id",
		&id, 1);
	if (ret) {
		dev_err(dev, "no hisilicon,hisi-regulator-id property set\n");
		return ret;
	}
	sreg->regulator_id = id;

	/* regulator-type */
	ret = of_property_read_u32_array(np, "hisilicon,hisi-regulator-type",
		&type, 1);
	if (ret) {
		dev_err(dev, "no hisilicon,hisi-regulator-type property set\n");
		return ret;
	}
	sreg->regulator_type = type;

	/* regulator-fake */
	ret = of_property_read_u32_array(np, "hisilicon,hisi-regulator-is-fake",
		&fake, 1);
	if (ret) {
		dev_err(dev, "no hisilicon,hisi-regulator-is-fake property set\n");
		return ret;
	}
	sreg->regulator_fake = fake;

	return ret;
}

static int of_dynamic_properties(struct hisi_regulator_ip *sreg,
	struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	int ret;

	/* clock_check_flag */
	ret = of_property_read_u32_array(np, "hisilicon,hisi-regulator-clk-check-flag",
		&sreg->clock_check_flag, 1);
	if (ret) {
		dev_err(dev, "no hisilicon,hisi-regulator-clk-check-flag property set\n");
		return ret;
	}

	/* clock_num */
	ret = of_property_read_u32_array(np, "hisilicon,hisi-regulator-clk-num",
		&sreg->clock_num, 1);
	if (ret) {
		dev_err(dev, "no hisilicon,hisi-regulator-clk-num property set\n");
		return ret;
	}

	/* enable clock flag */
	ret = of_property_read_u32_array(np, "hisilicon,hisi-need-to-enable-clock",
		&sreg->enable_clock, 1);
	if (ret) {
		dev_err(dev, "no hisilicon,hisi-need-to-enable-clock property set\n");
		return ret;
	}

	/* enable autofreqdump flag */
	ret = of_property_read_u32_array(np, "hisilicon,hisi-need-to-autofreqdump",
		&sreg->enable_autofreqdump, 1);
	if (ret) {
		dev_err(dev, "no hisilicon,hisi-need-to-autofreqdump property set\n");
		return ret;
	}

	/* clock set rate flag */
	ret = of_property_read_u32_array(np, "hisilicon,hisi-clock-rate-set-flag",
		&sreg->clock_set_rate_flag, 1);
	if (ret) {
		dev_err(dev, "no hisilicon,hisi-clock-rate-set-flag property set\n");
		return ret;
	}
	return ret;
}

static void hisi_dt_parse_dss_boot_check(struct hisi_regulator_ip *sreg,
	struct device_node *np)
{
	int ret;
	/* dss need check softreset state */
	unsigned int register_info[2] = {0};

	if (sreg->regulator_id == DSSSUBSYS_ID) {
		ret = of_property_read_u32_array(np, "hisilicon,hisi-regulator-dss-boot-check",
			register_info, 2);
		if (ret) {
			pr_err("[%s] get dss_boot_check failed!\n", __func__);
			return;
		}
		sreg->dss_boot_check[0] = register_info[0];
		sreg->dss_boot_check[1] = register_info[1];
	}
}

static int hisi_dt_parse_ip_atf(struct hisi_regulator_ip *sreg,
	struct platform_device *pdev)
{
	struct device *dev = NULL;
	struct device_node *np = NULL;
	int ret;

	if (!sreg || !pdev) {
		pr_err("[%s]regulator get dt para is err!\n", __func__);
		return -EINVAL;
	}

	dev = &pdev->dev;
	np = dev->of_node;

	/* static properties: regulator-id, regulator-type, regulator-fake */
	ret = of_static_properties(sreg, pdev);
	if (ret) {
		dev_err(dev, "get static properties failed\n");
		return ret;
	}

	ret = of_dynamic_properties(sreg, pdev);
	if (ret) {
		dev_err(dev, "get dynamic properties failed\n");
		return ret;
	}

	/* clock rate */
	ret = clock_rate_value_get(sreg, dev);
	if (ret)
		return ret;

	/* ppll0 clock set rate flag */
	ret = of_property_read_u32_array(np, "hisilicon,hisi-ppll0-clock-rate-set-flag",
		&sreg->ppll0_clock_set_rate_flag, 1);
	if (ret) {
		sreg->ppll0_clock_set_rate_flag = 0;
		ret = 0;
	}

	ret = __ppll0_clock_rate_value(sreg, np);
	if (ret)
		return -ENODEV;

#if defined(CONFIG_BOST_IP_PLATFORM)      || \
	defined(CONFIG_MIA_IP_PLATFORM)   || \
	defined(CONFIG_ATLA_IP_PLATFORM)  || \
	defined(CONFIG_PHOE_IP_PLATFORM)  || \
	defined(CONFIG_IP_PLATFORM_COMMON)
	hisi_dt_parse_dss_boot_check(sreg, np);
#endif
	if (hisi_dt_parse_aod_atf(sreg, np))
		dev_err(dev, "get aod hwlock and SC offset failed\n");

	return ret;
}

static int hisi_dt_parse_ip_lpm(struct hisi_regulator_ip *sreg,
	struct platform_device *pdev)
{
	struct device *dev = NULL;
	struct device_node *np = NULL;
	unsigned int register_info[2] = {0}; /* LPM enable values */
	int ret;

	if (!sreg || !pdev) {
		pr_err("[%s]regulator get  dt para is err!\n", __func__);
		return -EINVAL;
	}

	dev = &pdev->dev;
	np = dev->of_node;

	/* parse hisi top cssys ctrl_reg */
	ret = of_property_read_u32_array(np, "ip_to_lpm_enable_step",
		register_info, AP_TO_LPM_MSG_NUM);
	if (ret) {
		dev_err(dev, "no hisilicon, ip to lpm enable property set\n");
		return ret;
	}
	sreg->lpm_enable_value[0] = register_info[0];
	sreg->lpm_enable_value[1] = register_info[1];

	/* parse .hisi top cssys ctrl_reg */
	ret = of_property_read_u32_array(np, "ip_to_lpm_disable_step",
		register_info, AP_TO_LPM_MSG_NUM);
	if (ret) {
		dev_err(dev, "no hisilicon,ip to lpm disable property set\n");
		return ret;
	}
	sreg->lpm_disable_value[0] = register_info[0];
	sreg->lpm_disable_value[1] = register_info[1];

	/* static properties: regulator-id, regulator-type, regulator-fake */
	ret = of_static_properties(sreg, pdev);
	if (ret) {
		dev_err(dev, "get static properties failed\n");
		return ret;
	}

	return ret;
}

/* ***************************** ip voltage region ops ************************ */
struct regulator_ops hisi_ip_atf_rops = {
	.is_enabled = hisi_ip_to_atf_is_enabled,
	.enable = hisi_ip_to_atf_enabled,
	.disable = hisi_ip_to_atf_disabled,
};
struct regulator_ops hisi_ip_lpm_rops = {
	.is_enabled = hisi_ip_to_lpm_is_enabled,
	.enable = hisi_ip_to_lpm_enabled,
	.disable = hisi_ip_to_lpm_disabled,
};

/* ***************************** hisi_regulator_ip **************************** */
static const struct hisi_regulator_ip hisi_regulator_ip_atf = {
	.rdesc = {
		.ops = &hisi_ip_atf_rops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	.dt_parse = hisi_dt_parse_ip_atf,
};

static const struct hisi_regulator_ip hisi_regulator_ip_lpm = {
	.rdesc = {
		.ops = &hisi_ip_lpm_rops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	.dt_parse = hisi_dt_parse_ip_lpm,
};

/* ********************* of_hisi_regulator_ip_match_tbl *********************** */
static struct of_device_id of_hisi_regulator_ip_match_tbl[] = {
	{
		.compatible = "ip-regulator-atf",
		.data = &hisi_regulator_ip_atf,
	},
	{
		.compatible = "ip-regulator-lpm",
		.data = &hisi_regulator_ip_lpm,
	},
	{ /* end */ }
};

static int of_hisi_regulator_ip(struct device *dev,
	struct hisi_regulator_ip **sreg)
{
	const struct of_device_id *match = NULL;
	const struct hisi_regulator_ip *temp = NULL;

	/* to check which type of regulator this is */
	match = of_match_device(of_hisi_regulator_ip_match_tbl, dev);
	if (!match) {
		pr_err("get hi6421 regulator fail!\n\r");
		return -EINVAL;
	}

	temp = match->data;

	*sreg = kmemdup(temp, sizeof(struct hisi_regulator_ip), GFP_KERNEL);
	if (!(*sreg))
		return -ENOMEM;

	return 0;
}

static void of_sctrl_reg(struct hisi_regulator_ip *sreg)
{
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");
	if (!np)
		pr_err("[%s] node doesn't have sysctrl node!\n", __func__);
	else
		sreg->sctrl_reg = of_iomap(np, 0);
	if (!sreg->sctrl_reg)
		pr_err("[%s] sysctrl iomap fail!\n", __func__);
}


static int hisi_regulator_ip_probe(struct platform_device *pdev)
{
	struct device *dev = NULL;
	struct device_node *np = NULL;
	struct regulator_desc *rdesc = NULL;
	struct regulator_dev *rdev = NULL;
	struct hisi_regulator_ip *sreg = NULL;
	struct regulator_init_data *initdata = NULL;
	struct regulator_config config = {};
	int ret;
	const char *supplyname = NULL;

	if (!pdev) {
		pr_err("[%s]regulator get  platform device para is err!\n",
			__func__);
		return -EINVAL;
	}

	dev = &pdev->dev;
	np = dev->of_node;

	initdata = of_get_regulator_init_data(dev, np, NULL);
	if (!initdata) {
		pr_err("get regulator init data error !\n");
		return -EINVAL;
	}

	ret = of_hisi_regulator_ip(dev, &sreg);
	if (ret) {
		pr_err("allocate hisi_regulator_ip fail!\n");
		goto hisi_ip_probe_end;
	}

	sreg->name = initdata->constraints.name;
	rdesc = &sreg->rdesc;
	rdesc->name = sreg->name;

	supplyname = of_get_property(np, "hisilicon,supply_name", NULL);
	if (supplyname != NULL)
		initdata->supply_regulator = supplyname;

	mutex_init(&(sreg->regulator_lock));
	/* to parse device tree data for regulator specific */
	ret = sreg->dt_parse(sreg, pdev);
	if (ret) {
		dev_err(dev, "device tree parameter parse error!\n");
		goto hisi_ip_probe_end;
	}

	config.dev = &pdev->dev;
	config.init_data = initdata;
	config.driver_data = sreg;
	config.of_node = pdev->dev.of_node;
	sreg->np = np;
	sreg->regulator_enalbe_flag = 0;
	of_sctrl_reg(sreg);

	/* register regulator */
	rdev = regulator_register(rdesc, &config);
	if (IS_ERR(rdev)) {
		dev_err(dev, "failed to register %s\n", rdesc->name);
		ret = PTR_ERR(rdev);
		goto hisi_sctrl_reg_fail;
	}

	platform_set_drvdata(pdev, rdev);

	return 0;

hisi_sctrl_reg_fail:
	iounmap(sreg->sctrl_reg);
hisi_ip_probe_end:
	kfree(sreg);
	return ret;
}

static int hisi_regulator_ip_remove(struct platform_device *pdev)
{
	struct regulator_dev *rdev = NULL;
	struct hisi_regulator_ip *sreg = NULL;

	if (!pdev)
		return -ENODEV;

	rdev = platform_get_drvdata(pdev);
	if (!rdev)
		return -ENODEV;

	sreg = rdev_get_drvdata(rdev);
	if (!sreg)
		return -EINVAL;
	regulator_unregister(rdev);

	iounmap(sreg->sctrl_reg);
	kfree(sreg);
	return 0;
}

static struct platform_driver hisi_regulator_ip_driver = {
	.driver = {
		.name = "hisi_regulator_ip",
		.owner = THIS_MODULE,
		.of_match_table = of_hisi_regulator_ip_match_tbl,
	},
	.probe = hisi_regulator_ip_probe,
	.remove = hisi_regulator_ip_remove,
};

static int __init hisi_regulator_ip_init(void)
{
	return platform_driver_register(&hisi_regulator_ip_driver);
}
fs_initcall(hisi_regulator_ip_init);

static void __exit hisi_regulator_ip_exit(void)
{
	platform_driver_unregister(&hisi_regulator_ip_driver);
}
module_exit(hisi_regulator_ip_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Hisi regulator ip driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
