#include <linux/errno.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/ioctl.h>
#include <linux/device.h>

#include "npu_public.h"
#include "npu_clock.h"
#include "npu_mntn.h"

#define IPU_CLOCK_UNSET (0)

struct npu_clock {
	struct clk *npu_clk_ptr;
	unsigned int npu_low;
	unsigned int npu_middle;
	unsigned int npu_high;
	unsigned int npu_low_temperature;
	unsigned int default_rate;
	unsigned int start_rate;
	unsigned int curr_rate;
	unsigned int stop_rate;
	bool npu_prepare_enabled;
};

struct vcodecbus_clock {
	unsigned int vcodecbus_low;
	unsigned int vcodecbus_middle;
	unsigned int vcodecbus_high;
};

static struct npu_clock npu_clock_manager[MAX_SUPPORT_CORE_NUM];
static struct vcodecbus_clock vcodecbus_clock_config;

int npu_clock_init(struct device *dev)
{
	int ret = 0;
	int property_rd;
	struct device_node *node;
	int coreID;
	int core_number = (NPU_VERSION_V200 == hard_version) ? MAX_SUPPORT_CORE_NUM: (NPU_CORE_0 + 1);
	struct npu_clock *clk = npu_clock_manager;
	struct vcodecbus_clock *vcodecbus = &vcodecbus_clock_config;
	char *clk_node_name[MAX_SUPPORT_CORE_NUM] = {"npu-clock-core0-rate", "npu-clock-core1-rate"};

	if (!dev) {
		NPU_ERR("dev ptr is NULL\n");
		return -EINVAL;
	}

	memset(npu_clock_manager, 0, sizeof(npu_clock_manager));
	memset(&vcodecbus_clock_config, 0, sizeof(vcodecbus_clock_config));

	/* get clock of "clk-ics" from CLK API */
	clk[NPU_CORE_0].npu_clk_ptr = devm_clk_get(dev, "clk-ics-core0");
	if (IS_ERR_OR_NULL(clk[NPU_CORE_0].npu_clk_ptr)) {
		NPU_ERR("get clock failed, clk-ics-core0 is %pK\n", clk[NPU_CORE_0].npu_clk_ptr);
		ret = -ENODEV;
		goto exit_error;
	}

	if (NPU_VERSION_V200 == hard_version) {
		clk[NPU_CORE_1].npu_clk_ptr = devm_clk_get(dev, "clk-ics-core1");
		if (IS_ERR_OR_NULL(clk[NPU_CORE_1].npu_clk_ptr)) {
			NPU_ERR("get clock failed, clk-ics-core1 is %pK\n", clk[NPU_CORE_1].npu_clk_ptr);
			ret = -ENODEV;
			goto exit_error;
		}
	}

	for (coreID = 0; coreID < core_number; coreID++) {
		node = of_find_node_by_name(dev->of_node, clk_node_name[coreID]);
		if(!node) {
			NPU_ERR("find clock node error, clk_node_name is %s\n", clk_node_name[coreID]);
			ret = -ENODEV;
			goto exit_error;
		}

		property_rd  = of_property_read_u32(node, "start-rate",             &clk[coreID].default_rate);
		property_rd |= of_property_read_u32(node, "stop-rate",              &clk[coreID].stop_rate);
		property_rd |= of_property_read_u32(node, "npu-low",                &clk[coreID].npu_low);
		property_rd |= of_property_read_u32(node, "npu-middle",             &clk[coreID].npu_middle);
		property_rd |= of_property_read_u32(node, "npu-high",               &clk[coreID].npu_high);
		property_rd |= of_property_read_u32(node, "npu-low-temperature",    &clk[coreID].npu_low_temperature);
		if (property_rd) {
			NPU_ERR("read property of npu[%u] clock error, default_start/stop/low/middle/high/low_temperature=%u/%u/%u/%u/%u/%u\n",
				coreID, clk[coreID].default_rate, clk[coreID].stop_rate,
				clk[coreID].npu_low, clk[coreID].npu_middle, clk[coreID].npu_high, clk[coreID].npu_low_temperature);
			ret = -ENODEV;
			goto exit_error;
		}

		clk[coreID].curr_rate = IPU_CLOCK_UNSET;
		clk[coreID].npu_prepare_enabled = false;

	}

	node = of_find_node_by_name(dev->of_node, "vcodecbus-clock-rate");
	if(!node) {
		NPU_ERR("find clock node error\n");
		ret = -ENODEV;
		goto exit_error;
	}

	property_rd |= of_property_read_u32(node, "vcodecbus-low",          &vcodecbus->vcodecbus_low);
	property_rd |= of_property_read_u32(node, "vcodecbus-middle",       &vcodecbus->vcodecbus_middle);
	property_rd |= of_property_read_u32(node, "vcodecbus-high",         &vcodecbus->vcodecbus_high);
	if (property_rd) {
		NPU_ERR("read property of clock error. vcodecbus_low/middle/high=%u/%u/%u\n",
			vcodecbus->vcodecbus_low, vcodecbus->vcodecbus_middle, vcodecbus->vcodecbus_high);
		ret = -ENODEV;
		goto exit_error;
	}


	return 0;

exit_error:
	npu_clock_exit();
	return ret;
}

void npu_clock_exit(void)
{
	memset(npu_clock_manager, 0, sizeof(npu_clock_manager));
	memset(&vcodecbus_clock_config, 0, sizeof(vcodecbus_clock_config));
}

unsigned int npu_clock_get_vcodecbus_rate(void)
{
	struct npu_clock *clk_ptr = npu_clock_manager;
	struct vcodecbus_clock *vcodec = &vcodecbus_clock_config;

	//fixme: add low temperature branch here, vcodecbus rate is locked in low temperature

	if ((clk_ptr[NPU_CORE_0].curr_rate == clk_ptr[NPU_CORE_0].npu_high) ||
		(clk_ptr[NPU_CORE_1].curr_rate == clk_ptr[NPU_CORE_1].npu_high)) {
		return vcodec->vcodecbus_high;
	}

	if ((clk_ptr[NPU_CORE_0].curr_rate == clk_ptr[NPU_CORE_0].npu_middle) ||
		(clk_ptr[NPU_CORE_1].curr_rate == clk_ptr[NPU_CORE_1].npu_middle)) {
		return vcodec->vcodecbus_middle;
	}

	return vcodec->vcodecbus_low;
}

unsigned int npu_clock_get_npu_rate(unsigned int coreID)
{
	if (coreID >= MAX_SUPPORT_CORE_NUM) {
		return 0;
	}

	return npu_clock_manager[coreID].curr_rate;
}

static int npu_clock_set_prepare(unsigned int coreID, unsigned long target_rate)
{
	int ret;
	struct npu_clock *clk = &npu_clock_manager[coreID];

	/* for NPU small core(whose clk tree is "MUX"), when npu is working,
			SHOULD call func clk_disable_unprepare before clk_set_rate,
			and run clk_prepare_enable again, to avoid "BURR".
	   for NPU big core(whose clk tree is "switch"), no need to do it.
	   now, drv will not change clk when npu is working, so no need to add this branch */
	ret = clk_set_rate(clk->npu_clk_ptr, target_rate);
	if (ret) {
		NPU_ERR("clk_set_rate failed,ret=%d", ret);
		return ret;
	}

	if (!clk->npu_prepare_enabled) {
		ret = clk_prepare_enable(clk->npu_clk_ptr);
		if (ret) {
			NPU_ERR("clk_prepare_enable failed,ret=%d", ret);
		} else {
			clk->npu_prepare_enabled = true;
		}
	}

	return ret;
}

static int npu_clock_set(unsigned int new_rate, unsigned int coreID)
{
	int ret;
	unsigned int target_rate = new_rate;
	struct npu_clock *clk_ptr = &npu_clock_manager[coreID];

	ret = npu_clock_set_prepare(coreID, (unsigned long)target_rate);

	if (ret) {
		if (clk_ptr->npu_high == target_rate) {
			/* in low temperature, clk set rate to HIGH will fail, in this case try to set rate to another rate */
			NPU_ERR("set npu rate %d fail (possible in low temperature), ret:%d, try to set %d\n",
				target_rate, ret, clk_ptr->npu_low_temperature);
			target_rate = clk_ptr->npu_low_temperature;
			ret = npu_clock_set_prepare(coreID, (unsigned long)target_rate);

			if (ret) {
				NPU_ERR("set npu rate %d fail, ret:%d\n", target_rate, ret);
				npu_mntn_set_core_id(coreID);
				rdr_system_error((unsigned int)MODID_NPU_EXC_SET_BACK_CLOCK_FAIL, coreID, 0);
				return ret;
			}
		} else {
			npu_mntn_set_core_id(coreID);
			rdr_system_error((unsigned int)MODID_NPU_EXC_SET_CLOCK_FAIL, coreID, 0);
			return ret;
		}
	}

	NPU_INFO("core[%u] set clock done, clock(try/actually)=%d/%d/%ld, vcodecbus clock=%u\n",
		coreID, new_rate, target_rate, clk_get_rate(clk_ptr->npu_clk_ptr), npu_clock_get_vcodecbus_rate());

	clk_ptr->curr_rate = target_rate;

	return 0;
}

/* clock start, when a frame begin to calculate */
int npu_clock_start(unsigned int coreID)
{
	int ret;
	struct npu_clock *clk;

	CHECK_COREID_INVALID_RETURN(coreID);

	clk = &npu_clock_manager[coreID];

	if (clk->start_rate == clk->curr_rate) {
		return 0;
	}

	ret = npu_clock_set(clk->start_rate, coreID);
	if (ret) {
		NPU_ERR("npu_clock_set_rate failed,ret=%d\n", ret);
		return ret;
	}

	return 0;
}

/* clock off, when npu power-down */
void npu_clock_stop(unsigned int coreID)
{
	struct npu_clock *clk;

	CHECK_COREID_INVALID_RETURN_VOID(coreID);
	clk = &npu_clock_manager[coreID];

	if (npu_clock_set(clk->stop_rate, coreID)) {
		/* not return even set rate error, because power down process should finish */
		NPU_ERR("npu_clock_set_rate fail, ignore\n");
	}

	if (clk->npu_prepare_enabled) {
		clk_disable_unprepare(clk->npu_clk_ptr);
		clk->npu_prepare_enabled = false;
	} else {
		NPU_ERR("core[%u] clk disable and unprepared status is false", coreID);
	}

	clk->curr_rate = IPU_CLOCK_UNSET;
}

/* set new clock profile, it will realize in the next frame */
long npu_clock_set_profile(unsigned int coreID, unsigned long profile)
{
	struct npu_clock *clk;

	CHECK_COREID_INVALID_RETURN(coreID);
	clk = &npu_clock_manager[coreID];

	/* get rate by profile */
	switch (profile) {
	case DEV_UNSET_PROFILE:
		clk->start_rate = clk->npu_low; /* if profile not need IPU, set IPU clock as low */
		break;
	case DEV_LOW_PROFILE:
		clk->start_rate = clk->npu_low;
		break;
	case DEV_NORMAL_PROFILE:
		clk->start_rate = clk->npu_middle;
		break;
	case DEV_HIGH_PROFILE:
		clk->start_rate = clk->npu_high;
		break;
	default:
		NPU_ERR("profile=%lu error\n", profile);
		return -EINVAL;
	}

	NPU_INFO("core[%u] profile is set at %ld", coreID, profile);

	return 0;
}

/* set default clock profile */
void npu_clock_default_profile(void)
{
	int coreID;
	struct npu_clock *clk;

	for (coreID = NPU_CORE_0; coreID < MAX_SUPPORT_CORE_NUM; coreID++) {
		clk = &npu_clock_manager[coreID];
		clk->start_rate = clk->default_rate;
	}
}

