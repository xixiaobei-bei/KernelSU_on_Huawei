#include <linux/errno.h>
#include <linux/wakelock.h>
#include <linux/clk-provider.h>
#include "ipu_clock.h"
#include "ipu_mntn.h"

/* this func use mutex, for interface only, and SHOULD NOT be called by other ipu_clock functions */
int ipu_clock_init(struct device *dev, struct ics_clock *clk, bool lpm3_set_vcodecbus)
{
	int property_rd;
	struct device_node *node;

	mutex_lock(&clk->clk_mutex);
	clk->lpm3_set_vcodecbus = lpm3_set_vcodecbus;

	/* get clock of "clk-ics" from CLK API */
	clk->ipu_clk_ptr = devm_clk_get(dev, "clk-ics");
	clk->vote_high_volt = devm_clk_get(dev, "ics_volt_high");
	clk->vote_normal_volt = devm_clk_get(dev, "ics_volt_middle");

	if (IS_ERR_OR_NULL(clk->ipu_clk_ptr)) {
		printk(KERN_ERR"[%s]: IPU_ERROR:get clock failed, ipu_clk_ptr is %pK\n", __func__, clk->ipu_clk_ptr);
		mutex_unlock(&clk->clk_mutex);
		return -ENODEV;
	}

	clk->vcodecbus_clk_ptr = devm_clk_get(dev, "clk_vcodecbus");
	if (IS_ERR_OR_NULL(clk->vcodecbus_clk_ptr)) {
		printk(KERN_ERR"[%s]: IPU_ERROR:get clock failed, vcodecbus_clk_ptr is %pK\n", __func__, clk->vcodecbus_clk_ptr);
		mutex_unlock(&clk->clk_mutex);
		return -ENODEV;
	}

	if (IS_ERR_OR_NULL(clk->vote_high_volt)) {
		/* not return here when error occured, because this clock is non-essential, check it before use */
		printk(KERN_ERR"[%s]: IPU_ERROR:get clock failed, vote_high_ptr is %pK\n", __func__, clk->vote_high_volt);
	}

	if (IS_ERR_OR_NULL(clk->vote_normal_volt)) {
		/* not return here when error occured, because this clock is non-essential, check it before use */
		printk(KERN_ERR"[%s]: IPU_ERROR:get clock failed, vote_normal_ptr is %pK\n", __func__, clk->vote_normal_volt);
	}

	node = of_find_node_by_name(dev->of_node, "ipu-and-vcodecbus-clock-rate");
	if(!node) {
		printk(KERN_ERR"[%s]: IPU_ERROR:find clock node error\n", __func__);
		mutex_unlock(&clk->clk_mutex);
		return -ENODEV;
	}

	property_rd  = of_property_read_u32(node, "start-rate",             &clk->default_rate);
	property_rd |= of_property_read_u32(node, "stop-rate",              &clk->stop_rate);
	property_rd |= of_property_read_u32(node, "ipu-low",                &clk->ipu_low);
	property_rd |= of_property_read_u32(node, "ipu-middle",             &clk->ipu_middle);
	property_rd |= of_property_read_u32(node, "ipu-high",               &clk->ipu_high);
	property_rd |= of_property_read_u32(node, "ipu-low-temperature",    &clk->ipu_low_temperature);
	property_rd |= of_property_read_u32(node, "vcodecbus-low",          &clk->vcodecbus_low);
	property_rd |= of_property_read_u32(node, "vcodecbus-middle",       &clk->vcodecbus_middle);
	property_rd |= of_property_read_u32(node, "vcodecbus-high",         &clk->vcodecbus_high);
	property_rd |= of_property_read_u32(node, "vcodecbus-default",      &clk->vcodecbus_default);
	property_rd |= of_property_read_u32(node, "vcodecbus-high2default", &clk->vcodecbus_high2default);
	if (property_rd) {
		printk(KERN_ERR"[%s]: IPU_ERROR:read property of clock error\n", __func__);
		mutex_unlock(&clk->clk_mutex);
		return -ENODEV;
	}


	printk(KERN_DEBUG"[%s]: get clk rate done, start clk rate:%u, stop clk rate:%u\n",
		__func__, clk->start_rate, clk->stop_rate);

	clk->curr_rate = IPU_CLOCK_UNSET;
	mutex_unlock(&clk->clk_mutex);
	return 0;
}

/* this func use mutex, for interface only, and SHOULD NOT be called by other ipu_clock functions */
int ipu_vote_peri(struct ics_clock *clk)
{
	int ret = 0;

	if (clk->ipu_high == clk->voted_peri_volt || clk->ipu_middle == clk->voted_peri_volt) {
		printk(KERN_ERR"[%s]: IPU_ERROR:ipu has already voted peri voltage: %d, ignore\n",
			__func__, clk->voted_peri_volt);
		return -EBUSY;
	}

	/* to vote PERI by "clk start rate", which is configed by:
	(1) read from DTSI when system start;
	(2) set by user using IOCTL */
	if (clk->ipu_high == clk->start_rate) {
		printk(KERN_DEBUG"[%s]: vote buck0 HIGH voltage\n", __func__);

		if (IS_ERR_OR_NULL(clk->vote_high_volt)) {
			printk(KERN_ERR"[%s]: IPU_ERROR:get clock failed, vote_high_volt is %pK\n", __func__, clk->vote_high_volt);
			return -EBUSY;
		}
		ret = clk_prepare_enable(clk->vote_high_volt);

		if (0 == ret) {
			clk->voted_peri_volt = clk->ipu_high;
		} else {
			printk(KERN_ERR"[%s]: IPU_ERROR:clk prepare enable for peri-HIGH failed,ret=%d\n", __func__, ret);
		}

	} else if (clk->ipu_middle == clk->start_rate) {
		printk(KERN_DEBUG"[%s]: vote buck0 MIDDLE voltage\n", __func__);

		if (IS_ERR_OR_NULL(clk->vote_normal_volt)) {
			printk(KERN_ERR"[%s]: IPU_ERROR:get clock failed, vote_normal_volt is %pK\n", __func__, clk->vote_normal_volt);
			return -EBUSY;
		}
		ret = clk_prepare_enable(clk->vote_normal_volt);

		if (0 == ret) {
			clk->voted_peri_volt = clk->ipu_middle;
		} else {
			printk(KERN_ERR"[%s]: IPU_ERROR:clk prepare enable for peri-NORMAL failed,ret=%d\n", __func__, ret);
		}

	} else {
		clk->voted_peri_volt = clk->ipu_low;
	}

	return ret;
}

/* this func use mutex, for interface only, and SHOULD NOT be called by other ipu_clock functions */
void ipu_vote_peri_withdraw(struct ics_clock *clk)
{
	/* to withdraw vote PERI by "voted_peri_volt" but "clk start rate", because clk start rate might
	be modified by user at some time, but "clk" MUST withdraw the one opened before */

	if (clk->ipu_high == clk->voted_peri_volt) {
		printk(KERN_DEBUG"[%s]: to withdraw vote buck0 HIGH voltage\n", __func__);
		if (IS_ERR_OR_NULL(clk->vote_high_volt)) {
			printk(KERN_ERR"[%s]: IPU_ERROR:get clock failed, vote_high_volt is %pK\n", __func__, clk->vote_high_volt);
			return;
		}
		clk_disable_unprepare(clk->vote_high_volt);
		clk->voted_peri_volt = clk->ipu_low;

	} else if (clk->ipu_middle == clk->voted_peri_volt) {
		printk(KERN_DEBUG"[%s]: to withdraw vote buck0 MIDDLE voltage\n", __func__);
		if (IS_ERR_OR_NULL(clk->vote_normal_volt)) {
			printk(KERN_ERR"[%s]: IPU_ERROR:get clock failed, vote_normal_volt is %pK\n", __func__, clk->vote_normal_volt);
			return;
		}
		clk_disable_unprepare(clk->vote_normal_volt);
		clk->voted_peri_volt = clk->ipu_low;

	} else {
		/* do nothing */
	}
}

static int ipu_clock_set(struct ics_clock *clk, unsigned int new_rate)
{
	int ret;
	unsigned int target_rate = new_rate;

	if (new_rate == clk->curr_rate) {
		printk(KERN_ERR"[%s]: IPU_WARN:set some IPU clock rate %d, ignored\n", __func__, target_rate);
		return 0;
	}

	if (clk->ipu_high == target_rate) {
		/* for HIGH, set IPU clock to HIGH */
		ret = clk_set_rate(clk->ipu_clk_ptr, (unsigned long)target_rate);
		if (ret) {
			/* in low temperature, clk set rate to HIGH will fail, in this case try to set rate to another rate */
			printk(KERN_ERR"[%s]: IPU_ERROR:set ipu rate %d fail (possible in low temperature), ret:%d, try to set %d\n",
				__func__, target_rate, ret, clk->ipu_low_temperature);
			target_rate = clk->ipu_low_temperature;
			ret = clk_set_rate(clk->ipu_clk_ptr, (unsigned long)target_rate);
			if (ret) {
				printk(KERN_ERR"[%s]: IPU_ERROR:set ipu rate %d fail, ret:%d\n", __func__, target_rate, ret);
				rdr_system_error((unsigned int)MODID_NPU_EXC_SET_BACK_CLOCK_FAIL, 0, 0);
				return ret;
			}
		} else {
			if (!clk->lpm3_set_vcodecbus) {
				/* for HIGH set IPU rate ok, set VCODECBUS to HIGH */
				ret = clk_set_rate(clk->vcodecbus_clk_ptr, clk->vcodecbus_high);
				if (ret) {
					printk(KERN_ERR"[%s]: IPU_ERROR:set vcodec rate %d fail, ret:%d, ignore\n",	__func__, clk->vcodecbus_high, ret);
				}
			}
		}
	} else {
		/* for MIDDLE or LOW, set VCODECBUS to default if necessary (when alter from HIGH) */
		if (!clk->lpm3_set_vcodecbus && clk->ipu_high == clk->curr_rate) {
			/* set vcodec bus to "VCODECBUS_CLOCK_DEFAULT", which is used as the default rate for VENC/VDEC */
			ret = clk_set_rate(clk->vcodecbus_clk_ptr, clk->vcodecbus_high2default);
			if (ret) {
				printk(KERN_ERR"[%s]: IPU_ERROR:set vcodec rate %d fail, ret:%d, ignore\n", __func__, clk->vcodecbus_high2default, ret);
			}

			ret = clk_set_rate(clk->vcodecbus_clk_ptr, clk->vcodecbus_default);
			if (ret) {
				printk(KERN_ERR"[%s]: IPU_ERROR:set vcodec rate %d fail, ret:%d, ignore\n", __func__, clk->vcodecbus_default, ret);
			}
		}

		ret = clk_set_rate(clk->ipu_clk_ptr, (unsigned long)target_rate);
		if (ret) {
			/* in low temperature, clk set rate to HIGH will fail, in this case try to set rate to MIDDLE */
			printk(KERN_ERR"[%s]: IPU_ERROR:set ipu rate %d fail, ret:%d\n", __func__, target_rate, ret);
			rdr_system_error((unsigned int)MODID_NPU_EXC_SET_CLOCK_FAIL, 0, 0);
			return ret;
		}
	}

	printk(KERN_ERR"[%s]: IPU_NOTE: set clock done, ipu clock(try/actually/clk_get)=%d/%d/%ld, vcodecbus clock=%ld\n",
		__func__, new_rate, target_rate, clk_get_rate(clk->ipu_clk_ptr), clk_get_rate(clk->vcodecbus_clk_ptr));

	clk->curr_rate = target_rate;

	return 0;
}

/* this func use mutex, for interface only, and SHOULD NOT be called by other ipu_clock functions */
int ipu_clock_start(struct ics_clock *clk)
{
	int ret;

	mutex_lock(&clk->clk_mutex);

	if (0 == clk->voted_peri_volt) {
		ret = ipu_vote_peri(clk);
		if (ret) {
			/* ignore vote peri here */
			printk(KERN_ERR"[%s]: IPU_ERROR:ipu_vote_peri fail\n", __func__);
		}
	}

	/* WARNING: clk_prepare_enable should NOT be called in interrupt because it contains mutex.
	   If needed in furture, use API: clk_prepare and clk_enable instead of clk_prepare_enable
	   in interrupt functions. */
	ret = clk_prepare_enable(clk->ipu_clk_ptr);
	if (ret) {
		printk(KERN_ERR"[%s]: IPU_ERROR:clk prepare enable failed,ret=%d\n", __func__, ret);
		mutex_unlock(&clk->clk_mutex);
		return ret;
	}

	/* NOTE: here need not call "clk_prepare_enable(clk->vcodecbus_clk_ptr)" because because it is used by both IPU and VCODEC
	   process can guarentee it!! */

	clk->curr_rate = IPU_CLOCK_UNSET;
	ret = ipu_clock_set(clk, clk->start_rate);
	if (ret) {
		printk(KERN_ERR"[%s]: IPU_ERROR:ipu_clock_set_rate failed,ret=%d\n", __func__, ret);
		mutex_unlock(&clk->clk_mutex);
		return ret;
	}
	mutex_unlock(&clk->clk_mutex);
	return 0;
}

void ipu_clock_set_start_default(struct ics_clock *clk)
{
	mutex_lock(&clk->clk_mutex);
	clk->start_rate = clk->default_rate;
	mutex_unlock(&clk->clk_mutex);
}

/* this func use mutex, for interface only, and SHOULD NOT be called by other ipu_clock functions */
int ipu_clock_set_start_rate(struct ics_clock *clk, unsigned int new_rate)
{
	int ret;

	mutex_lock(&clk->clk_mutex);

	if (clk->ipu_high == new_rate ||
		clk->ipu_middle == new_rate ||
		clk->ipu_low == new_rate) {

		/* withdraw voltage hold lock if neccessary */
		ipu_vote_peri_withdraw(clk);

		/* vote voltage hold lock if neccessary */
		clk->start_rate = new_rate;
		ret = ipu_vote_peri(clk);
		if (ret) {
			/* ignore vote peri here */
			printk(KERN_ERR"[%s]: IPU_ERROR:ipu_vote_peri fail\n", __func__);
		}

		mutex_unlock(&clk->clk_mutex);
		return 0;
	} else {
		printk(KERN_ERR"[%s]: IPU_ERROR:invalid start rate=%u\n", __func__, new_rate);
		mutex_unlock(&clk->clk_mutex);
		return -EINVAL;
	}
}

/* this func use mutex, for interface only, and SHOULD NOT be called by other ipu_clock functions */
int ipu_clock_set_rate(struct ics_clock *clk, unsigned int new_rate)
{
	int ret;

	mutex_lock(&clk->clk_mutex);
	ret = ipu_clock_set(clk, new_rate);
	mutex_unlock(&clk->clk_mutex);

	return ret;
}

/* this func use mutex, for interface only, and SHOULD NOT be called by other ipu_clock functions */
void ipu_clock_stop(struct ics_clock *clk)
{
	mutex_lock(&clk->clk_mutex);
	clk_disable_unprepare(clk->ipu_clk_ptr);

	/* NOTE: here need not call "clk_disable_unprepare(clk->vcodecbus_clk_ptr)" because it is used by both IPU and VCODEC
	   process can guarentee it!! */
	mutex_unlock(&clk->clk_mutex);
}

