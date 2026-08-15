#include <linux/fs.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/device.h>

#include "npu_public.h"
#include "npu_regulator.h"
#include "npu_mntn.h"

struct npu_regulator {
	struct regulator *vnpu_ip[MAX_SUPPORT_CORE_NUM];
	struct regulator *vvcodecbus;
};

struct npu_regulator npu_regulator;

/* read config by dtsi, it should be called after get NPU_VERSION */
int npu_regulator_init(struct device *dev)
{
	if (!dev) {
		NPU_ERR("dev ptr is NULL\n");
		return -EINVAL;
	}

	memset(&npu_regulator, 0, sizeof(npu_regulator));

	/* get regulator */
	npu_regulator.vnpu_ip[NPU_CORE_0] = devm_regulator_get(dev, "vnpucore0");
	if (IS_ERR(npu_regulator.vnpu_ip[NPU_CORE_0])) {
		NPU_ERR("Couldn't get regulator npu0\n");
		return -EINVAL;
	}

	if (NPU_VERSION_V200 == hard_version) {
		/* get regulator */
		npu_regulator.vnpu_ip[NPU_CORE_1] = devm_regulator_get(dev, "vnpucore1");
		if (IS_ERR(npu_regulator.vnpu_ip[NPU_CORE_1])) {
			NPU_ERR("Couldn't get regulator npu1\n");
			return -EINVAL;
		}

		npu_regulator.vvcodecbus = devm_regulator_get(dev, "vvcodecbus");
		if (IS_ERR(npu_regulator.vvcodecbus)) {
			NPU_ERR("Couldn't get regulator vcodecbus! \n");
			return -EINVAL;
		}
		NPU_DEBUG("Get regulator npu1 succuss. hard_version=0x%x\n", hard_version);
	}

	return 0;
}

void npu_regulator_exit(void)
{
	memset(&npu_regulator, 0, sizeof(npu_regulator));
}

/* regulator on, when npu power-on */
int npu_regulator_on(unsigned int coreID)
{
	int ret;

	CHECK_COREID_INVALID_RETURN(coreID);

	/*ICS power on*/
	if (1 == coreID) {
		ret = regulator_enable(npu_regulator.vvcodecbus);
		if (ret) {
			NPU_ERR("Failed to enable vvcodecbus: %d\n", ret);
			npu_mntn_power_exc_proc(coreID, VVCODECBUS_IP, ret);
			rdr_system_error((unsigned int)MODID_NPU_EXC_SET_POWER_UP_FAIL, coreID, 0);
			return ret;
		}
	}
	ret = regulator_is_enabled(npu_regulator.vnpu_ip[coreID]);
	if (ret) {
		NPU_ERR("regulator_is_enabled vipu_ip[%d] fail: %d\n", coreID, ret);
		npu_mntn_set_core_id(coreID);
		rdr_system_error((unsigned int)MODID_NPU_EXC_SET_POWER_UP_STATUS_FAULT, coreID, 0);
		return -EBUSY;
	}

	ret = regulator_enable(npu_regulator.vnpu_ip[coreID]);
	if (ret) {
		NPU_ERR("regulator_enable vipu_ip[%d] fail: %d\n", coreID, ret);
		npu_mntn_power_exc_proc(coreID, NPU_IP, ret);
		rdr_system_error((unsigned int)MODID_NPU_EXC_SET_POWER_UP_FAIL, coreID, 0);

		if (1 == coreID) {
			if (regulator_disable(npu_regulator.vvcodecbus)) {
				NPU_ERR("regulator_disable vvcodecbus fail: %d\n", ret);
			}
		}
		return ret;
	}

	return 0;
}

/* regulator off, when npu power-off */
void npu_regulator_off(unsigned int coreID)
{
	int ret;
	CHECK_COREID_INVALID_RETURN_VOID(coreID);

	ret = regulator_disable(npu_regulator.vnpu_ip[coreID]);
	if (ret) {
		NPU_ERR("regulator_disable vipu_ip[%d] fail: %d\n", coreID, ret);
		npu_mntn_power_exc_proc(coreID, NPU_IP, ret);
		rdr_system_error((unsigned int)MODID_NPU_EXC_SET_POWER_DOWN_FAIL, coreID, ret);
	}

	if (1 == coreID) {
		ret = regulator_disable(npu_regulator.vvcodecbus);
		if (ret) {
			NPU_ERR("regulator_disable vvcodecbus fail: %d\n", ret);
			npu_mntn_power_exc_proc(coreID, VVCODECBUS_IP, ret);
			rdr_system_error((unsigned int)MODID_NPU_EXC_SET_POWER_DOWN_FAIL, coreID, ret);
		}
	}
}

