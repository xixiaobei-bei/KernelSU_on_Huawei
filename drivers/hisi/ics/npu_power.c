#include <linux/errno.h>
#include <linux/clk.h>
#include <linux/clk-provider.h> //fixme:why need?
#include <linux/ioctl.h>
#include <linux/device.h>

#include "npu_public.h"
#include "npu_regulator.h"
#include "npu_clock.h"
#include "npu_power.h"

struct npu_power {
	bool status;
};

struct npu_power npu_power[MAX_SUPPORT_CORE_NUM];

int npu_power_init(struct device *dev)
{
	int ret;
	if (!dev) {
		NPU_ERR("dev ptr is NULL\n");
		return -EINVAL;
	}

	memset(npu_power, 0, sizeof(npu_power));

	/* init npu power */
	ret = npu_regulator_init(dev);
	if (ret) {
		NPU_ERR("Failed to init npu power, ret=%d\n", ret);
		goto power_init_error_exit;
	}

	/* init ipu clock */
	ret = npu_clock_init(dev);
	if (ret) {
		NPU_ERR("Failed to init npu clock, ret=%d\n", ret);
		goto do_npu_regulator_exit;
	}

	return 0;

do_npu_regulator_exit:
	npu_regulator_exit();
power_init_error_exit:
	memset(npu_power, 0, sizeof(npu_power));
	return ret;

}

void npu_power_exit(void)
{
	npu_clock_exit();
	npu_regulator_exit();

	memset(npu_power, 0, sizeof(npu_power));
}

int npu_power_on(unsigned int coreID)
{
	int ret;
	struct timeval tv1, tv2, tv3;

	CHECK_COREID_INVALID_RETURN(coreID);

	NPU_DEBUG("coreID=%u start", coreID);

	if (npu_power[coreID].status) {
		NPU_ERR("npu %u is already powered-on\n", coreID);
		return -EBUSY;
	}

	do_gettimeofday(&tv1);
	ret = npu_regulator_on(coreID);
	if (ret) {
		NPU_ERR("Failed to npu_regulator_on[%u] fail, ret=%d\n", coreID, ret);
		return ret;
	}

	do_gettimeofday(&tv2);
	ret = npu_clock_start(coreID);
	if (ret) {
		NPU_ERR("Failed to npu_clock_on[%u], ret=%d\n", coreID, ret);
		npu_regulator_off(coreID);

		return ret;
	}

	do_gettimeofday(&tv3);

	if (((tv3.tv_sec - tv1.tv_sec)*1000000 + (tv3.tv_usec - tv1.tv_usec)) > 1000) {
		NPU_PRINT("npu PU/CLK elapse is %ld/%ld usec\n",
			(tv2.tv_sec - tv1.tv_sec)*1000000 + (tv2.tv_usec - tv1.tv_usec),
			(tv3.tv_sec - tv2.tv_sec)*1000000 + (tv3.tv_usec - tv2.tv_usec));
	}

	npu_power[coreID].status = true;

	NPU_DEBUG("coreID=%u finish", coreID);

	return 0;
}

void npu_power_off(unsigned int coreID)
{
	struct timeval tv1, tv2, tv3;

	CHECK_COREID_INVALID_RETURN_VOID(coreID);

	NPU_DEBUG("coreID=%u start", coreID);

	if (!npu_power[coreID].status) {
		NPU_ERR("npu is already power-down, ignore\n");
		return;
	}

	do_gettimeofday(&tv1);
	npu_clock_stop(coreID);

	do_gettimeofday(&tv2);
	npu_regulator_off(coreID);

	do_gettimeofday(&tv3);

	if (((tv3.tv_sec - tv1.tv_sec)*1000000 + (tv3.tv_usec - tv1.tv_usec)) > 1000) {
		NPU_PRINT("core[%u] CLK_STOP/PD elapse is %ld/%ld usec\n", coreID,
			(tv2.tv_sec - tv1.tv_sec)*1000000 + (tv2.tv_usec - tv1.tv_usec),
			(tv3.tv_sec - tv2.tv_sec)*1000000 + (tv3.tv_usec - tv2.tv_usec));
	}

	npu_power[coreID].status = false;
	NPU_DEBUG("coreID=%u finish", coreID);
}

bool npu_power_status(unsigned int coreID)
{
	if (coreID >= MAX_SUPPORT_CORE_NUM || (NPU_VERSION_V200 != hard_version && coreID == 1)) {
		return false;
	}

	return npu_power[coreID].status;
}

/* set new clock profile, it will realize in the next frame */
long npu_set_profile(unsigned int coreID, unsigned long profile)
{
	return npu_clock_set_profile(coreID, profile);
}

int npu_update_profile(unsigned int coreID)
{
	int ret;
	struct timeval tv1, tv2;

	do_gettimeofday(&tv1);
	ret = npu_clock_start(coreID);
	if (ret) {
		NPU_ERR("Failed to npu_clock_start[%u] fail, ret=%d\n", coreID, ret);
		return ret;
	}

	do_gettimeofday(&tv2);

	if (((tv2.tv_sec - tv1.tv_sec)*1000000 + (tv2.tv_usec - tv1.tv_usec)) > 400) {
		NPU_ERR("IPU_WARN: npu CLK elapse is %ld usec\n",
			(tv2.tv_sec - tv1.tv_sec)*1000000 + (tv2.tv_usec - tv1.tv_usec));
	}

	return ret;
}

void npu_set_default_profile(void)
{
	npu_clock_default_profile();
}

unsigned int npu_get_npu_rate(unsigned int coreID)
{
	return npu_clock_get_npu_rate(coreID);
}

unsigned int npu_get_vcodecbus_rate(void)
{
	return npu_clock_get_vcodecbus_rate();
}

