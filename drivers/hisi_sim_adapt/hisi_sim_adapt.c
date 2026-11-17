

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"

#include "hisi_sim_adapt.h"

struct hw_sim_hardware_info *g_hw_simhw_info[MAX_SIMHP_NUM] = {NULL};

static s32 set_io_for_nvt_shift(struct hw_sim_hardware_info *info);

/*lint -e605*/
static int hw_sim_read_regulator(struct regulator **sim_regulator, const struct device *spmidev,
	const char *prop_str)
{
	*sim_regulator = devm_regulator_get(spmidev, prop_str);

	if (IS_ERR(*sim_regulator)) {
		devm_regulator_put(*sim_regulator);
		hwsim_loge("failed to get regulator\n");
		return HW_SIM_ERR;
	}
	return HW_SIM_OK;
}

static int hw_sim_read_gpio(const struct device_node *np, const char *prop_str, const char *demo, int *gpio_id)
{
	int temp_gpio_id;

	temp_gpio_id = of_get_named_gpio(np, prop_str, 0);
	if (temp_gpio_id < 0) {
		hwsim_loge("failed to get gpio id: %d\n", temp_gpio_id);
		return HW_SIM_ERR;
	}

	if (gpio_request(temp_gpio_id, demo) < 0) {
		hwsim_loge("failed to request gpio\n");
		return HW_SIM_ERR;
	}

	*gpio_id = temp_gpio_id;
	return HW_SIM_OK;
}

static int read_pmu_extral_modem(const struct device *spmidev, const extral_modem_pmu_cfg *extral_modem_pmu)
{
	int ret;

	hwsim_logi("begin read\n");
	ret = hw_sim_read_regulator(&extral_modem_pmu->sd_io_pmu, spmidev, "sd_io");
	if (ret != HW_SIM_OK)
		return HW_SIM_ERR;

	ret = hw_sim_read_regulator(&extral_modem_pmu->sd_vcc_pmu, spmidev, "sd_vcc");
	if (ret != HW_SIM_OK)
		return HW_SIM_ERR;

	if (extral_modem_pmu->sim1_ap_power_scheme == AP_POWER_SCHE_A) {
		ret = hw_sim_read_regulator(&extral_modem_pmu->sd_switch_pmu, spmidev, "sd_switch");
		if (ret != HW_SIM_OK)
			return HW_SIM_ERR;
	}

	return HW_SIM_OK;
}

static int read_pmu_extral_ldo(const struct device *spmidev, const extral_ldo_pmu_cfg *extral_ldo_pmu)
{
	int ret = HW_SIM_OK;

	if (extral_ldo_pmu->extral_ldo_out_type == EXTRAL_LDO_OUT_TYPE_GPIO) {
		gpio_free(extral_ldo_pmu->extral_ldo_out_gpio_select);
		if (gpio_request(extral_ldo_pmu->extral_ldo_out_gpio_select, "extral_ldo_out") < 0) {
			hwsim_loge("failed to request extral_ldo_out gpio\n");
			return HW_SIM_ERR;
		}
	} else if (extral_ldo_pmu->extral_ldo_out_type == EXTRAL_LDO_OUT_TYPE_PMU) {
		ret = hw_sim_read_regulator(&extral_ldo_pmu->extral_ldo_out_select, spmidev, "extral_ldo_out");
	} else {
		hwsim_loge("extral_ldo_pmu->extral_ldo_out_type is err\n");
		return HW_SIM_ERR;
	}

	if (hw_sim_read_regulator(&extral_ldo_pmu->extral_ldo_en, spmidev, "sim_vcc") != HW_SIM_OK) {
		hwsim_loge("failed to read sim_vcc\n");
		return HW_SIM_ERR;
	}

	if (hw_sim_read_regulator(&extral_ldo_pmu->sim_sd_io, spmidev, "sim_sd_io") != HW_SIM_OK) {
		hwsim_loge("failed to read sim_sd_io\n");
		return HW_SIM_ERR;
	}

	return ret;
}

static int read_pmu_extral_ldo_but_no_sd(const struct device *spmidev, const sim_pmu_hardware_ctrl *sim_pmu_hardware)
{
	int ret;
	extral_ldo_pmu_cfg *extral_ldo_pmu = &sim_pmu_hardware->hard_cfg.extral_ldo_pmu;

	if (extral_ldo_pmu->extral_ldo_out_type == EXTRAL_LDO_OUT_TYPE_GPIO) {
		gpio_free(extral_ldo_pmu->extral_ldo_out_gpio_select);
		if (gpio_request(extral_ldo_pmu->extral_ldo_out_gpio_select, "extral_ldo_out") < 0) {
			hwsim_loge("failed to request extral_ldo_out gpio\n");
			return HW_SIM_ERR;
		}
	} else if (extral_ldo_pmu->extral_ldo_out_type == EXTRAL_LDO_OUT_TYPE_PMU) {
		ret = hw_sim_read_regulator(&extral_ldo_pmu->extral_ldo_out_select, spmidev, "extral_ldo_out");
		if (ret != HW_SIM_OK) {
			hwsim_loge("failed to read extral_ldo_out\n");
			return HW_SIM_ERR;
		}
	} else {
		hwsim_loge("extral_ldo_pmu->extral_ldo_out_type is err\n");
		return HW_SIM_ERR;
	}

	ret = hw_sim_read_regulator(&extral_ldo_pmu->extral_ldo_en, spmidev, "sim_vcc");
	if (ret != HW_SIM_OK) {
		hwsim_loge("failed to read sim_vcc\n");
		return HW_SIM_ERR;
	}

	ret = hw_sim_read_regulator(&sim_pmu_hardware->esim_ctrl.esim_vcc, spmidev, "esim_vcc");
	if (ret != HW_SIM_OK) {
		hwsim_loge("fail to read esim vcc\n");
		return HW_SIM_ERR;
	}

	return ret;
}

static u32 sim_io_mux_init(struct hw_sim_hardware_info *info, struct device_node *np)
{
	u32 ret;

	ret = (u32)of_property_read_u32(np, "iocfg_base_adrr", &(info->sim_io_cfg.phy_addr));
	if (ret != 0) {
		hwsim_loge("no sim io cfg addr\n");
		return ret;
	}
	info->sim_io_cfg.virt_addr = ioremap(info->sim_io_cfg.phy_addr, 0x1000); //lint !e446
	if (info->sim_io_cfg.virt_addr == NULL) {
		hwsim_loge("sim io cfg ioremap failed\n");
		return EINVAL;
	}
	ret = (u32)of_property_read_u32_array(np, "iomg_usim_offset", (u32 *)(info->sim_io_cfg.iomg_usim_offset),
		USIM_OFFSET_SIZE);
	ret |= (u32)of_property_read_u32_array(np, "iocg_usim_offset", (u32 *)(info->sim_io_cfg.iocg_usim_offset),
		USIM_OFFSET_SIZE);
	if (ret != 0) {
		hwsim_loge("no sim io offset\n");
		return ret;
	}

	hwsim_logi("sim io mux int done\n");
	return 0;
}

static u32 sd_io_mux_init(struct hw_sim_hardware_info *info, struct device_node *np)
{
	u32 ret;

	ret = (u32)of_property_read_u32(np, "iocfg_sdbase_adrr", &(info->sd_io_cfg.phy_addr));
	if (ret != 0) {
		hwsim_loge("no sdbase addr\n");
		return ret;
	}
	info->sd_io_cfg.virt_addr = ioremap(info->sd_io_cfg.phy_addr, 0x1000); //lint !e446
	if (info->sd_io_cfg.virt_addr == NULL) {
		hwsim_loge("ioremap failed\n");
		return EINVAL;
	}

	ret = (u32)of_property_read_u32_array(np, "iomg_sd_offset", (u32 *)(info->sd_io_cfg.iomg_usim_offset),
		USIM_OFFSET_SIZE);
	ret |= (u32)of_property_read_u32_array(np, "iocg_sd_offset", (u32 *)(info->sd_io_cfg.iocg_usim_offset),
		USIM_OFFSET_SIZE);
	if (ret != 0) {
		hwsim_loge("no sd io offset\n");
		return ret;
	}

	ret = (u32)of_property_read_u32(np, "esim_detect_en", &(info->sd_io_cfg.esim_detect_en));
	if (ret != 0) {
		hwsim_loge("esim_detect disabled\n");
		return ret;
	}
	if (info->sd_io_cfg.esim_detect_en == 1) {
		ret = (u32)of_property_read_u32_array(np, "esim_det_func_offset",
			(u32 *)(info->sd_io_cfg.esim_det_func_offset), ESIM_DET_OFFSET_SIZE);
		ret |= (u32)of_property_read_u32_array(np, "esim_det_ctrl_offset",
			(u32 *)(info->sd_io_cfg.esim_det_ctrl_offset), ESIM_DET_OFFSET_SIZE);
		if (ret != 0) {
			hwsim_loge("no esim det offset\n");
			return ret;
		}
	}

	hwsim_logi("sd io mux int done\n");
	return 0;
}

static int hw_sim_read_extral_ldo_dts(struct hw_sim_hardware_info *info, const struct device_node *np)
{
	u32 extral_ldo_bias_use = 0;
	u32 extral_ldo_out_type = 0;
	int temp_gpio_id = -1;
	int ret;

	if (info->sim_pmu_hardware.sim_pmu_hardware_type == SIM_POWER_TYPE_EXTRAL_LDO ||
		info->sim_pmu_hardware.sim_pmu_hardware_type == SIM_POWER_TYPE_EXTRAL_LDO_BUT_NO_SD) {
		ret = of_property_read_u32(np, "extral_ldo_bias_use", &extral_ldo_bias_use);
		if (ret != 0) {
			hwsim_logi("failed to read extral_ldo_bias_use\n");
			extral_ldo_bias_use = 1;
		}
		ret = of_property_read_u32(np, "extral_ldo_out_type", &extral_ldo_out_type);
		if (ret != 0) {
			hwsim_logi("failed to read extral_ldo_out_type\n");
			extral_ldo_out_type = EXTRAL_LDO_OUT_TYPE_GPIO;
		}
		info->sim_pmu_hardware.hard_cfg.extral_ldo_pmu.extral_ldo_bias_use = extral_ldo_bias_use;
		info->sim_pmu_hardware.hard_cfg.extral_ldo_pmu.extral_ldo_out_type = extral_ldo_out_type;
		if (extral_ldo_out_type == EXTRAL_LDO_OUT_TYPE_GPIO) {
			ret = of_property_read_u32(np, "extral_ldo_out_gpio", &temp_gpio_id);
			if (ret != 0) {
				hwsim_loge("failed to get extral ldo out gpio: %d\n", temp_gpio_id);
				return HW_SIM_ERR;
			}
			info->sim_pmu_hardware.hard_cfg.extral_ldo_pmu.extral_ldo_out_gpio_select = temp_gpio_id;
		}
	}

	return HW_SIM_OK;
}

static void hw_sim_read_esim_class_support(struct hw_sim_hardware_info *info, const struct device_node *np)
{
	int esim_classc_only = 1;
	int ret = of_property_read_u32(np, "esim_classc_only", &esim_classc_only);
	if (ret != 0) {
		hwsim_logi("can't get esim_classc_only \n");
		esim_classc_only = 0;
	}
	hwsim_logi("esim_class_only value is : %d\n", esim_classc_only);
	info->esim_classc_only = esim_classc_only;
}

static int hw_sim_read_dts(struct hw_sim_hardware_info *info, struct device_node *np)
{
	int ret;
	u32 sim_power_type = 0;
	u32 nvt_shift_enabled = 0;
	u32 sz = sizeof(sim_sysctrl_s) / sizeof(u32);

	hwsim_logi("begin to read hw sim dts\n");
	ret = of_property_read_u32(np, "sim_power_type", &sim_power_type);
	if (ret != 0) {
		hwsim_loge("fail to read sim power type\n");
		sim_power_type = SIM_POWER_TYPE_SINGLE;
	} else {
		if (sim_power_type >= SIM_POWER_TYPE_BUTT) {
			hwsim_loge("no support for the power type: %d\n", sim_power_type);
			return HW_SIM_ERR;
		}
	}

	ret = of_property_read_u32_array(np, "lv_shift_ctrl", (u32 *)(&info->sim_pmu_hardware.lv_shift_ctrl), sz);
	if (ret != 0) {
		hwsim_logi("fail to read lv_shift_ctrl\n");
		info->sim_pmu_hardware.lv_shift_ctrl.func = 0;
	}

	ret = of_property_read_u32(np, "nvt_shift", &nvt_shift_enabled);
	if (ret != 0) {
		hwsim_logi("fail to read nvt_shift\n");
		nvt_shift_enabled = 0;
	}

	if (nvt_shift_enabled == 1 && info->sim_pmu_hardware.lv_shift_ctrl.func == 1) {
		hwsim_loge("no support lv_shift and nvt shift both open\n");
		return HW_SIM_ERR;
	}

	info->sim_pmu_hardware.sim_pmu_hardware_type = sim_power_type;
	info->sim_pmu_hardware.nvt_shift.func = nvt_shift_enabled;
	if (info->sim_pmu_hardware.nvt_shift.func == 1) {
		info->sim_pmu_hardware.nvt_shift.nvt_fix_leak = 0;
		ret = hw_sim_read_gpio(np, "nvt_shift_gpio-gpios", "nvt_shift_gpio",
			(int *)(&info->sim_pmu_hardware.nvt_shift.nvt_shift_gpio));
		if (ret != HW_SIM_OK) {
			hwsim_loge("fail to read nvt shift gpio\n");
			return HW_SIM_ERR;
		}
	}

	ret = hw_sim_read_extral_ldo_dts(info, np);
	hwsim_logi("sim_power_type: %d, lv_shift_ctrl.func: %d, nvt_shift.func: %d, extral_ldo_out_type: %d, ret: %d\n",
		info->sim_pmu_hardware.sim_pmu_hardware_type, info->sim_pmu_hardware.lv_shift_ctrl.func,
		info->sim_pmu_hardware.nvt_shift.func, info->sim_pmu_hardware.hard_cfg.extral_ldo_pmu.extral_ldo_out_type,
		ret);
	hw_sim_read_esim_class_support(info, np);
	return ret;
}

static int hw_sim_read_regulator_by_type(const struct hw_sim_hardware_info *info)
{
	int ret = HW_SIM_ERR;

	hwsim_logi("read\n");
	switch (info->sim_pmu_hardware.sim_pmu_hardware_type) {
	case SIM_POWER_TYPE_SINGLE:
		ret = hw_sim_read_regulator(&info->sim_pmu_hardware.hard_cfg.single_sim_pmu.sim_vcc_io,
			info->spmidev, "sim_vcc");
		break;

	case SIM_POWER_TYPE_SIM_SD:
		ret = hw_sim_read_regulator(&info->sim_pmu_hardware.hard_cfg.sim_sd_pmu.sim_sd_io,
			info->spmidev, "sim_sd_io");
		ret += hw_sim_read_regulator(&info->sim_pmu_hardware.hard_cfg.sim_sd_pmu.sim_sd_vcc,
			info->spmidev, "sim_vcc");
		break;

	case SIM_POWER_TYPE_EXTRAL_MODEM:
		ret = read_pmu_extral_modem(info->spmidev, &(info->sim_pmu_hardware.hard_cfg.extral_modem_pmu));
		break;

	case SIM_POWER_TYPE_EXTRAL_LDO:
		ret = read_pmu_extral_ldo(info->spmidev, &(info->sim_pmu_hardware.hard_cfg.extral_ldo_pmu));
		break;

	case SIM_POWER_TYPE_EXTRAL_LDO_BUT_NO_SD:
		ret = read_pmu_extral_ldo_but_no_sd(info->spmidev, &(info->sim_pmu_hardware));
		break;

	case SIM_POWER_TYPE_AUX:
		ret = hw_sim_read_regulator(&info->sim_pmu_hardware.hard_cfg.aux_sim_pmu.sim_vcc,
			info->spmidev, "sim_vcc");
		ret += hw_sim_read_regulator(&info->sim_pmu_hardware.hard_cfg.aux_sim_pmu.aux_ldo,
			info->spmidev, "aux_vcc");
		break;

	default:
		break;
	}
	if (ret != HW_SIM_OK && info->sim_pmu_hardware.nvt_shift.func == 1)
		(void)set_io_for_nvt_shift(info);

	return ret;
}

static int set_nanosd_io_sdfun(const struct hw_sim_hardware_info *info)
{
	u32 i;

	if (info->sd_io_cfg.virt_addr == NULL) {
		hwsim_loge("sd_io_cfg.virt_addr is null\n");
		return HW_SIM_ERR;
	}

	for (i = 0; i < USIM_OFFSET_SIZE; ++i)
		writel(SD_GPIO_IO_MUX,
			(void *)(((uintptr_t)info->sd_io_cfg.virt_addr + info->sd_io_cfg.iomg_usim_offset[i])));

	hwsim_logi("set sd_io_cfg gpio func to be GPIO\n");
	return HW_SIM_OK;
}

static int set_nanosd_io_simfun(const struct hw_sim_hardware_info *info)
{
	u32 i;
	u32 cur = USIM_IOCG_VALUE;

	if (info->sd_io_cfg.virt_addr == NULL) {
		hwsim_loge("sd_io_cfg.virt_addr is null\n");
		return HW_SIM_ERR;
	}

	for (i = 0; i < USIM_OFFSET_SIZE; ++i) {
		writel(SD_USIM_MUX, // set usim function
			(void *)(((uintptr_t)info->sd_io_cfg.virt_addr + info->sd_io_cfg.iomg_usim_offset[i])));
		writel(cur, // high 4 bit: current, low 4 bit: pu pd
			(void *)(((uintptr_t)info->sd_io_cfg.virt_addr + info->sd_io_cfg.iocg_usim_offset[i])));
	}

	hwsim_logi("current 0x%02X, set sd_io_cfg gpio fun to be USIM\n", cur);
	return HW_SIM_OK;
}

static s32 set_sim_io_simfun(struct hw_sim_hardware_info *info)
{
	u32 i;

	if (info->sim_io_cfg.virt_addr == NULL) {
		hwsim_loge("sim_io_cfg.virt_addr is null\n");
		return HW_SIM_ERR;
	}

	for (i = 0; i < USIM_OFFSET_SIZE; ++i) {
		writel(USIM_IO_MUX,
			(void *)(((uintptr_t)info->sim_io_cfg.virt_addr + info->sim_io_cfg.iomg_usim_offset[i])));
		writel(USIM_IOCG_VALUE,
			(void *)(((uintptr_t)info->sim_io_cfg.virt_addr + info->sim_io_cfg.iocg_usim_offset[i])));
	}

	hwsim_logi("set sim_io_cfg gpio func to be USIM\n");
	return HW_SIM_OK;
}

static s32 set_sim_io_gpiofun(struct hw_sim_hardware_info *info)
{
	u32 i;

	if (info->sim_io_cfg.virt_addr == NULL) {
		hwsim_loge("sim_io_cfg.virt_addr is null\n");
		return HW_SIM_ERR;
	}

	for (i = 0; i < USIM_OFFSET_SIZE; ++i) {
		writel(GPIO_IO_MUX,
			(void *)(((uintptr_t)info->sim_io_cfg.virt_addr + info->sim_io_cfg.iomg_usim_offset[i])));
		writel(USIM_IOCG_VALUE,
			(void *)(((uintptr_t)info->sim_io_cfg.virt_addr + info->sim_io_cfg.iocg_usim_offset[i])));
	}

	hwsim_logi("set sim_io_cfg gpio fun to be GPIO\n");
	return HW_SIM_OK;
}

static s32 set_io_for_nvt_shift(struct hw_sim_hardware_info *info)
{
	u32 i;

	if (info->sd_io_cfg.virt_addr == NULL) {
		hwsim_loge("sd_io_cfg.virt_addr is null\n");
		return HW_SIM_ERR;
	}

	for (i = 0; i < USIM_OFFSET_SIZE; ++i)
		writel(SD_GPIO_IO_MUX,
			(void *)(((uintptr_t)info->sd_io_cfg.virt_addr + info->sd_io_cfg.iomg_usim_offset[i])));

	for (i = 0; i < USIM_OFFSET_SIZE; ++i) {
		if (i == 0) // clk
			writel(USIM_IOCG_VALUE,
				(void *)(((uintptr_t)info->sd_io_cfg.virt_addr + info->sd_io_cfg.iocg_usim_offset[i])));
		else // rst data
			writel(USIM_SD_IOCG_PU_VALUE,
				(void *)(((uintptr_t)info->sd_io_cfg.virt_addr + info->sd_io_cfg.iocg_usim_offset[i])));
	}
	info->sim_pmu_hardware.nvt_shift.nvt_fix_leak = 1;

	hwsim_logi("set nvt io gpio fun GPIO\n");
	return HW_SIM_OK;
}

static int set_det_gpio_fun(const struct hw_sim_hardware_info *info)
{
	u32 i;
	u32 value;

	if (info->sd_io_cfg.virt_addr == NULL) {
		hwsim_loge("info->sd_io_cfg.virt_addr is null\n");
		return HW_SIM_ERR;
	}

	for (i = 0; i < ESIM_DET_OFFSET_SIZE; ++i) {
		hwsim_logi("set func mux for index %d\n", i);
		writel(SD_GPIO_IO_MUX,
			(void *)(((uintptr_t)info->sd_io_cfg.virt_addr + info->sd_io_cfg.esim_det_func_offset[i])));
	}

	value = readl(
		(void *)(((uintptr_t)info->sd_io_cfg.virt_addr + info->sd_io_cfg.esim_det_ctrl_offset[SD_CMD_INDEX])));
	value = (value & 0xfffffffc) | 0x01; // pu
	writel(value,
		(void *)(((uintptr_t)info->sd_io_cfg.virt_addr + info->sd_io_cfg.esim_det_ctrl_offset[SD_CMD_INDEX])));

	value = readl(
		(void *)(((uintptr_t)info->sd_io_cfg.virt_addr + info->sd_io_cfg.esim_det_ctrl_offset[SD_DATA0_INDEX])));
	value = (value & 0xfffffffc) | 0x02; // pd
	writel(value,
		(void *)(((uintptr_t)info->sd_io_cfg.virt_addr + info->sd_io_cfg.esim_det_ctrl_offset[SD_DATA0_INDEX])));

	value = readl(
		(void *)(((uintptr_t)info->sd_io_cfg.virt_addr + info->sd_io_cfg.esim_det_ctrl_offset[SD_DATA2_INDEX])));
	value = (value & 0xfffffffc) | 0x02; // pd
	writel(value,
		(void *)(((uintptr_t)info->sd_io_cfg.virt_addr + info->sd_io_cfg.esim_det_ctrl_offset[SD_DATA2_INDEX])));

	value = readl(
		(void *)(((uintptr_t)info->sd_io_cfg.virt_addr + info->sd_io_cfg.esim_det_ctrl_offset[SD_DATA3_INDEX])));
	value = (value & 0xfffffffc) | 0x01; // pu
	writel(value,
		(void *)(((uintptr_t)info->sd_io_cfg.virt_addr + info->sd_io_cfg.esim_det_ctrl_offset[SD_DATA3_INDEX])));

	hwsim_logi("done\n");
	return HW_SIM_OK;
}

static s32 set_hw_sim_io_mux(struct hw_sim_hardware_info *info, set_sim_io_type set_io_type)
{
	u32 ret = HW_SIM_OK;

	switch (set_io_type) {
	case SIM_IO_MUX_SIMIO_USIM_NANOSDIO_SD_REQUEST:
		ret = (u32)set_sim_io_simfun(info); // set esim func
		ret |= (u32)set_nanosd_io_sdfun(info); // set sd_sim func
		hwsim_logi("SIM1_IO_MUX_SIMIO_USIM_NANOSDIO_SD_REQUEST");
		break;

	case SIM_IO_MUX_SIMIO_GPIO_NANOSDIO_USIM_REQUEST:
		ret = (u32)set_sim_io_gpiofun(info); // set esim func
		ret |= (u32)set_nanosd_io_simfun(info); // set sd_sim func
		info->sim_pmu_hardware.nvt_shift.nvt_fix_leak = 0;
		hwsim_logi("SIM1_IO_MUX_SIMIO_GPIO_NANOSDIO_USIM_REQUEST");
		break;

	case SIM_IO_MUX_SIMIO_USIM_ONLY_REQUEST:
		ret = (u32)set_sim_io_simfun(info);
		hwsim_logi("SIM1_IO_MUX_SIMIO_USIM_ONLY_REQUEST");
		break;

	case SIM_IO_MUX_GPIO_DET_REQUEST:
		ret = (u32)set_det_gpio_fun(info);
		hwsim_logi("SIM1_IO_MUX_GPIO_DET_REQUEST");
		break;

	default:
		break;
	}

	return ret;
}

static u32 get_card_status(struct hw_sim_hardware_info *info)
{
	u32 status;
#ifdef CONFIG_MMC_DW_MUX_SDSIM
	status = (u32)get_card1_type();
#else
	status = STATUS_SIM;
#endif
	if (status == STATUS_SD2JTAG)
		return STATUS_SD2JTAG;

	if (info->sim_pmu_hardware.sim_pmu_hardware_type == SIM_POWER_TYPE_SINGLE || status == STATUS_NO_CARD) {
		hwsim_logi("as STATUS_SIM return\n");
		return STATUS_SIM;
	}

	return status;
}

static sci_hw_op_result_e convert_sim_status_to_sci(u32 sim_status)
{
	sci_hw_op_result_e rslt;

	switch (sim_status) {
	case STATUS_SIM:
		rslt = SIM_HW_CARD_TYPE_SIM;
		break;

	case STATUS_SD:
		rslt = SIM_HW_CARD_TYPE_SD;
		break;

	case STATUS_SD2JTAG:
		rslt = SIM_HW_CARD_TYPE_JTAG;
		break;

	default:
		rslt = SIM_HW_CARD_TYPE_SIM;
		break;
	}

	return rslt;
}

static void put_regulator(const struct regulator *pmu)
{
	if (pmu == NULL)
		return;

	devm_regulator_put(pmu);
}

static void put_regulators(const struct hw_sim_hardware_info *info)
{
	if (info->sim_esim_type == SIM_TYPE_ESIM) {
		put_regulator(info->sim_pmu_hardware.esim_ctrl.esim_vcc);
		hwsim_logi("done, esim\n");
		return;
	}

	switch (info->sim_pmu_hardware.sim_pmu_hardware_type) {
	case SIM_POWER_TYPE_SINGLE:
		put_regulator(info->sim_pmu_hardware.hard_cfg.single_sim_pmu.sim_vcc_io);
		break;

	case SIM_POWER_TYPE_SIM_SD:
		put_regulator(info->sim_pmu_hardware.hard_cfg.sim_sd_pmu.sim_sd_vcc);
		if (info->sim_pmu_hardware.nvt_shift.func == 1)
			put_regulator(info->sim_pmu_hardware.hard_cfg.sim_sd_pmu.sim_sd_io);
		break;

	case SIM_POWER_TYPE_EXTRAL_MODEM:
		hwsim_loge("SIM_POWER_TYPE_EXTRAL_MODEM not been debug\n");
		break;

	case SIM_POWER_TYPE_EXTRAL_LDO:
		if (info->sim_pmu_hardware.hard_cfg.extral_ldo_pmu.extral_ldo_out_type == EXTRAL_LDO_OUT_TYPE_PMU)
			put_regulator(info->sim_pmu_hardware.hard_cfg.extral_ldo_pmu.extral_ldo_out_select);
		put_regulator(info->sim_pmu_hardware.hard_cfg.extral_ldo_pmu.extral_ldo_en);
		if (info->sim_pmu_hardware.nvt_shift.func == 1)
			put_regulator(info->sim_pmu_hardware.hard_cfg.extral_ldo_pmu.sim_sd_io);
		break;

	case SIM_POWER_TYPE_EXTRAL_LDO_BUT_NO_SD:
		if (info->sim_pmu_hardware.hard_cfg.extral_ldo_pmu.extral_ldo_out_type == EXTRAL_LDO_OUT_TYPE_PMU)
			put_regulator(info->sim_pmu_hardware.hard_cfg.extral_ldo_pmu.extral_ldo_out_select);
		put_regulator(info->sim_pmu_hardware.hard_cfg.extral_ldo_pmu.extral_ldo_en);
		put_regulator(info->sim_pmu_hardware.esim_ctrl.esim_vcc);
		break;

	case SIM_POWER_TYPE_AUX:
		put_regulator(info->sim_pmu_hardware.hard_cfg.aux_sim_pmu.aux_ldo);
		put_regulator(info->sim_pmu_hardware.hard_cfg.aux_sim_pmu.sim_vcc);
		break;

	default:
		break;
	}
	hwsim_logi("done, sim_pmu_hardware_type: %d\n", info->sim_pmu_hardware.sim_pmu_hardware_type);
}

static set_sim_io_type get_sim_io_type(const struct hw_sim_hardware_info *info)
{
	set_sim_io_type set_io_type = SIM_IO_MUX_BUTT;

	if (info->sim_esim_type == SIM_TYPE_USIM) {
		if (info->sim_status == STATUS_SIM) {
			set_io_type = SIM_IO_MUX_BUTT; // when inquire not set gpio func
			hwsim_logi("not set gpio function when inquire\n");
		}
	} else if (info->sim_esim_type == SIM_TYPE_ESIM) {
		if (info->sim_status == STATUS_SD2JTAG || info->sim_status == STATUS_SD)
			set_io_type = SIM_IO_MUX_SIMIO_USIM_ONLY_REQUEST;
		else
			set_io_type = SIM_IO_MUX_SIMIO_USIM_NANOSDIO_SD_REQUEST;
	}
	return set_io_type;
}

static sci_hw_op_result_e sim_inquire_card_type(u32 sim_id, struct hw_sim_hardware_info *info)
{
	u32 sim_status;

	if (info->sim_pmu_hardware.sim_pmu_hardware_type == SIM_POWER_TYPE_EXTRAL_LDO_BUT_NO_SD) {
		return SIM_HW_CARD_TYPE_SIM;
	}

	sim_status = get_card_status(info);
	info->sim_status = sim_status;
	hwsim_logi("sim_id: %d, sim_status: %d, sim_esim_type: %d\n", sim_id, info->sim_status, info->sim_esim_type);

	if ((info->sim_pmu_hardware.sim_pmu_hardware_type != SIM_POWER_TYPE_AUX) &&
		(info->sim_pmu_hardware.sim_pmu_hardware_type != SIM_POWER_TYPE_SINGLE)) {
		set_sim_io_type set_io_type = get_sim_io_type(info);
		if (set_hw_sim_io_mux(info, set_io_type) != HW_SIM_OK) {
			hwsim_loge("set_hw_sim_io_mux fail\n");
			return SIM_HW_SET_GPIO_FAIL;
		}
	}

	if (info->sim_pmu_hardware.sim_pmu_hardware_type != SIM_POWER_TYPE_SINGLE &&
		info->sim_pmu_hardware.sim_pmu_hardware_type != SIM_POWER_TYPE_AUX && sim_status == STATUS_SIM)
		return SIM_HW_CARD_TYPE_MUXSIM;
	else
		return convert_sim_status_to_sci(sim_status);
}

static struct regulator* get_sim_vcc_regulator(const sim_pmu_hardware_ctrl *sim_pmu_hardware)
{
	sim_power_type_e power_type;

	if (sim_pmu_hardware == NULL)
		return NULL;

	power_type = sim_pmu_hardware->sim_pmu_hardware_type;
	if (power_type == SIM_POWER_TYPE_SINGLE)
		return sim_pmu_hardware->hard_cfg.single_sim_pmu.sim_vcc_io;
	else if (power_type == SIM_POWER_TYPE_SIM_SD)
		return sim_pmu_hardware->hard_cfg.sim_sd_pmu.sim_sd_vcc;
	else if (power_type == SIM_POWER_TYPE_EXTRAL_LDO || power_type == SIM_POWER_TYPE_EXTRAL_LDO_BUT_NO_SD)
		return sim_pmu_hardware->hard_cfg.extral_ldo_pmu.extral_ldo_en;
	else if (power_type == SIM_POWER_TYPE_AUX)
		return sim_pmu_hardware->hard_cfg.aux_sim_pmu.sim_vcc;
	else
		return NULL;
}

static int hw_sim_disable_regulator(const struct regulator *sim_regulator)
{
	if (sim_regulator == NULL)
		return HW_SIM_ERR;

	if (regulator_disable(sim_regulator) == 0)
		hwsim_logi("regulator_disable ok\n");
	else
		hwsim_loge("regulator_disable fail\n");

	// release power
	devm_regulator_put(sim_regulator);
	return HW_SIM_OK;
}

static int hw_sim_enable_regulator(const struct regulator *sim_regulator, s32 vol)
{
	if (sim_regulator == NULL) {
		hwsim_loge("sim_regulator is null\n");
		return HW_SIM_ERR;
	}

	if (regulator_is_enabled(sim_regulator)) {
		hwsim_logi("regulator_is_enabled, regulator_disable first\n");
		if (regulator_disable(sim_regulator) == 0)
			hwsim_logi("regulator_disable ok\n");
		else
			hwsim_loge("regulator_disable fail\n");
	}

	if (regulator_set_voltage(sim_regulator, vol, vol) != HW_SIM_OK) {
		hwsim_loge("regulator_set_voltage fail\n");
		return HW_SIM_ERR;
	}

	if (regulator_enable(sim_regulator) != HW_SIM_OK) {
		hwsim_loge("regulator_enable fail\n");
		return HW_SIM_ERR;
	}

	return HW_SIM_OK;
}

static int hw_sim_ctrl_sd_io_regulator(const struct hw_sim_hardware_info *info, u32 vol)
{
	int gpio_output_value;
	int ret = HW_SIM_OK;

	if ((info->sim_pmu_hardware.sim_pmu_hardware_type == SIM_POWER_TYPE_SIM_SD) ||
		(info->sim_pmu_hardware.sim_pmu_hardware_type == SIM_POWER_TYPE_EXTRAL_LDO)) {
		hwsim_logi("need to power on sd io\n");
		if (info->sim_pmu_hardware.nvt_shift.func == 1) {
			gpio_output_value = (vol == VCC_1_8V) ? 1 : 0;
			ret = gpio_direction_output(info->sim_pmu_hardware.nvt_shift.nvt_shift_gpio, gpio_output_value);
			if (ret < 0) {
				hwsim_loge("fail to set nvt gpio direction: %d\n", gpio_output_value);
				return HW_SIM_ERR;
			} else {
				hwsim_logi("success to set nvt shift gpio\n");
			}
		} else {
			if (info->sim_pmu_hardware.sim_pmu_hardware_type == SIM_POWER_TYPE_SIM_SD)
				ret = hw_sim_enable_regulator(info->sim_pmu_hardware.hard_cfg.sim_sd_pmu.sim_sd_io, vol);
			else
				ret = hw_sim_enable_regulator(info->sim_pmu_hardware.hard_cfg.extral_ldo_pmu.sim_sd_io, vol);
		}
	} else {
		hwsim_logi("no need to power on sd io\n");
	}

	return ret;
}

static int extral_modem_power_on_a_vcc(const extral_modem_pmu_cfg *extral_modem_pmu, s32 vol)
{
	int ret;

	if (vol == VCC_3_0V) {
		ret = regulator_set_voltage(extral_modem_pmu->sd_vcc_pmu, vol, vol);
		if (ret != 0) {
			hwsim_loge("set vcc_pmu failed\n");
			return HW_SIM_ERR;
		}

		ret = regulator_enable(extral_modem_pmu->sd_vcc_pmu);
		if (ret != 0) {
			hwsim_loge("enable vcc_pmu failed\n");
			return HW_SIM_ERR;
		}
	} else {
		if (regulator_is_enabled(extral_modem_pmu->sd_vcc_pmu)) {
			ret = regulator_set_voltage(extral_modem_pmu->sd_vcc_pmu, vol, vol);
			if (ret != 0) {
				hwsim_loge("set vcc_pmu failed\n");
				return HW_SIM_ERR;
			}

			ret = regulator_disable(extral_modem_pmu->sd_vcc_pmu);
			if (ret != 0) {
				hwsim_loge("disable vcc_pmu failed\n");
				return HW_SIM_ERR;
			}
		} else {
			hwsim_loge("disable vcc_pmu done, regulator disabled\n");
		}
	}
	return HW_SIM_OK;
}

static int extral_modem_power_on_a(const extral_modem_pmu_cfg *extral_modem_pmu, s32 vol)
{
	int ret;

	// open switch power for for sim1
	ret = regulator_set_voltage(extral_modem_pmu->sd_switch_pmu, VCC_3_0V, VCC_3_0V);
	if (ret != 0) {
		hwsim_loge("set sd_switch failed\n");
		return HW_SIM_ERR;
	}

	ret = regulator_enable(extral_modem_pmu->sd_switch_pmu);
	if (ret != 0) {
		hwsim_loge("enable sd_switch failed\n");
		return HW_SIM_ERR;
	}

	hwsim_logi("set sd_switch done\n");
	// open io power for sim1
	ret = regulator_set_voltage(extral_modem_pmu->sd_io_pmu, vol, vol);
	if (ret != 0) {
		hwsim_loge("set io_pmu failed\n");
		return HW_SIM_ERR;
	}

	ret = regulator_enable(extral_modem_pmu->sd_io_pmu);
	if (ret != 0) {
		hwsim_loge("enable io_pmu failed\n");
		return HW_SIM_ERR;
	}
	hwsim_logi("set sd_io done\n");
	ret = extral_modem_power_on_a_vcc(extral_modem_pmu, vol);
	hwsim_logi("set sd_vcc done\n");
	return ret;
}

static int extral_modem_power_on_b(const extral_modem_pmu_cfg *extral_modem_pmu, s32 vol)
{
	int ret;

	// open io power for sim1
	ret = regulator_set_voltage(extral_modem_pmu->sd_io_pmu, vol, vol);
	if (ret != 0) {
		hwsim_loge("set io_pmu failed\n");
		return HW_SIM_ERR;
	}

	ret = regulator_enable(extral_modem_pmu->sd_io_pmu);
	if (ret != 0) {
		hwsim_loge("enable io_pmu failed\n");
		return HW_SIM_ERR;
	}
	hwsim_logi("set sd_io done\n");
	ret = regulator_set_voltage(extral_modem_pmu->sd_vcc_pmu, vol, vol);
	if (ret != 0) {
		hwsim_loge("set vcc_pmu failed\n");
		return HW_SIM_ERR;
	}

	ret = regulator_enable(extral_modem_pmu->sd_vcc_pmu);
	if (ret != 0) {
		hwsim_loge("enable vcc_pmu failed\n");
		return HW_SIM_ERR;
	}

	hwsim_logi("set vcc_pmu done\n");
	return ret;
}

static int sim_extral_modem_power_on(const extral_modem_pmu_cfg *extral_modem_pmu, s32 vol)
{
	int ret = HW_SIM_OK;

	if (extral_modem_pmu->sim1_ap_power_scheme == AP_POWER_SCHE_A)
		ret = extral_modem_power_on_a(extral_modem_pmu, vol);
	else if (extral_modem_pmu->sim1_ap_power_scheme == AP_POWER_SCHE_B)
		ret = extral_modem_power_on_b(extral_modem_pmu, vol);

	return ret;
}

static pmu_power_on_extral_ldo_vcc(const struct hw_sim_hardware_info *info, s32 vol)
{
	int ret;
	int gpio_output_value;
	extral_ldo_pmu_cfg *extral_ldo_pmu = &info->sim_pmu_hardware.hard_cfg.extral_ldo_pmu;

	if (extral_ldo_pmu->extral_ldo_out_type == EXTRAL_LDO_OUT_TYPE_GPIO) {
		gpio_output_value = (vol == VCC_1_8V) ? 0 : 1;
		ret = gpio_direction_output(extral_ldo_pmu->extral_ldo_out_gpio_select, gpio_output_value);
	} else {
		ret = hw_sim_enable_regulator(extral_ldo_pmu->extral_ldo_out_select, vol);
	}
	if (ret != HW_SIM_OK) {
		hwsim_loge("fail to enable ldo out select\n");
		return HW_SIM_ERR;
	}
	hwsim_logi("power on ldo out select done, out type: %d\n", extral_ldo_pmu->extral_ldo_out_type);

	if (extral_ldo_pmu->extral_ldo_bias_use == 1) {
		ret = boost_5v_enable(BOOST_5V_ENABLE, BOOST_CTRL_SIM);
		if (ret != 0) {
			hwsim_loge("fail to power on boost 5v\n");
			return HW_SIM_ERR;
		}
		hwsim_logi("enable boost 5v done\n");
	}

	ret = hw_sim_enable_regulator(extral_ldo_pmu->extral_ldo_en, vol);
	if (ret != HW_SIM_OK) {
		hwsim_loge("fail to power on extral_ldo_en\n");
		return ret;
	}
	hwsim_logi("power on extral_ldo_en done\n");
	return HW_SIM_OK;
}

static int pmu_power_on_extral_ldo(struct hw_sim_hardware_info *info, s32 vol)
{
	extral_ldo_pmu_cfg *extral_ldo_pmu = &info->sim_pmu_hardware.hard_cfg.extral_ldo_pmu;
	int ret = pmu_power_on_extral_ldo_vcc(info, vol);
	if (ret != HW_SIM_OK) {
		return HW_SIM_ERR;
	}
	udelay(1000); // 1ms

	// nvt shift io power const 3v
	if (info->sim_pmu_hardware.nvt_shift.func == 1) {
		ret = hw_sim_enable_regulator(extral_ldo_pmu->sim_sd_io, VCC_3_0V); // ldo11
		if (ret != HW_SIM_OK)
			hwsim_loge("fail to power on sim_sd_io\n");
		hwsim_logi("power on sim_sd_io for nvt_shift done\n");
	}
	if (set_hw_sim_io_mux(info, SIM_IO_MUX_SIMIO_GPIO_NANOSDIO_USIM_REQUEST) != HW_SIM_OK) // usim function
		hwsim_loge("set_hw_sim_io_mux fail\n");
	regulator_disable(extral_ldo_pmu->extral_ldo_en); // ldo16 off
	udelay(10000); // 10ms
	udelay(10000); // 10ms
	ret = hw_sim_enable_regulator(extral_ldo_pmu->extral_ldo_en, vol); // ldo16
	if (ret != HW_SIM_OK)
		hwsim_loge("power on sd io sim_vcc (extral_ldo_en) fail\n");
	hwsim_logi("ldo16, 1ms, ldo11, usim, ldo16_off, 20ms, ldo16\n");
	return ret;
}

static int pmu_power_on_extral_ldo_but_no_sd(const struct hw_sim_hardware_info *info, s32 vol)
{
	int ret = pmu_power_on_extral_ldo_vcc(info, vol);
	if (ret != HW_SIM_OK) {
		hwsim_loge("pmu_power_on_extral_ldo_vcc fail\n");
		return HW_SIM_ERR;
	}
	ret = hw_sim_enable_regulator(info->sim_pmu_hardware.esim_ctrl.esim_vcc, vol);
	if (ret != HW_SIM_OK)
		hwsim_loge("power on esim_vcc fail\n");

	hwsim_logi("ldo16, ldo54\n");
	return ret;
}

static void enable_lv_shift(const struct hw_sim_hardware_info *info)
{
	u32 lv_shift_value;

	lv_shift_value = hisi_sub_pmic_reg_read(info->sim_pmu_hardware.lv_shift_ctrl.reg_off);
	lv_shift_value &= ~(1 << 2); // enable
	hisi_sub_pmic_reg_write(info->sim_pmu_hardware.lv_shift_ctrl.reg_off, lv_shift_value);

	udelay(1000); // 1ms
	lv_shift_value = hisi_sub_pmic_reg_read(info->sim_pmu_hardware.lv_shift_ctrl.reg_off);
	lv_shift_value |= 1;
	hisi_sub_pmic_reg_write(info->sim_pmu_hardware.lv_shift_ctrl.reg_off, lv_shift_value);
	hwsim_logi("write value to lv_shift addr, reg_off_addr: 0x%04X, lv_shift_value: 0x%04X\n",
		info->sim_pmu_hardware.lv_shift_ctrl.reg_off, lv_shift_value);
}

static void disable_lv_shift(const struct hw_sim_hardware_info *info)
{
	u32 lv_shift_value;

	lv_shift_value = hisi_sub_pmic_reg_read(info->sim_pmu_hardware.lv_shift_ctrl.reg_off);
	lv_shift_value &= ~1;
	hisi_sub_pmic_reg_write(info->sim_pmu_hardware.lv_shift_ctrl.reg_off, lv_shift_value);

	udelay(1000); // 1ms
	lv_shift_value = hisi_sub_pmic_reg_read(info->sim_pmu_hardware.lv_shift_ctrl.reg_off);
	lv_shift_value |= (1 << 2); // disable
	hisi_sub_pmic_reg_write(info->sim_pmu_hardware.lv_shift_ctrl.reg_off, lv_shift_value);
	hwsim_logi("write value to lv_shift addr, reg_off_addr: 0x%04X, lv_shift_value: 0x%04X\n",
		info->sim_pmu_hardware.lv_shift_ctrl.reg_off, lv_shift_value);
}

static sci_hw_op_result_e esim_phy_card_det_level(struct hw_sim_hardware_info *info, const struct regulator *sim_sd_io)
{
	int ret;
	u32 gpio_d0;
	u32 gpio_d1;
	u32 gpio_d2;
	u32 gpio_d3;

	ret = hw_sim_read_gpio(info->np, "card_det_d0-gpios", "card_det_d0",
		(u32 *)(&info->sim_pmu_hardware.esim_ctrl.gpio_d0));
	ret += hw_sim_read_gpio(info->np, "card_det_d1-gpios", "card_det_d1",
		(u32 *)(&info->sim_pmu_hardware.esim_ctrl.gpio_d1));
	ret += hw_sim_read_gpio(info->np, "card_det_d2-gpios", "card_det_d2",
		(u32 *)(&info->sim_pmu_hardware.esim_ctrl.gpio_d2));
	ret += hw_sim_read_gpio(info->np, "card_det_d3-gpios", "card_det_d3",
		(u32 *)(&info->sim_pmu_hardware.esim_ctrl.gpio_d3));
	if (ret != 0) {
		(void)regulator_disable(sim_sd_io);
		devm_regulator_put(sim_sd_io);
		return SIM_HW_CARD_NO_EXIST_WHILE_ESIM;
	}

	gpio_direction_input(info->sim_pmu_hardware.esim_ctrl.gpio_d0);
	gpio_direction_input(info->sim_pmu_hardware.esim_ctrl.gpio_d1);
	gpio_direction_input(info->sim_pmu_hardware.esim_ctrl.gpio_d2);
	gpio_direction_input(info->sim_pmu_hardware.esim_ctrl.gpio_d3);

	gpio_d0 = (u32)gpio_get_value(info->sim_pmu_hardware.esim_ctrl.gpio_d0);
	gpio_d1 = (u32)gpio_get_value(info->sim_pmu_hardware.esim_ctrl.gpio_d1);
	gpio_d2 = (u32)gpio_get_value(info->sim_pmu_hardware.esim_ctrl.gpio_d2);
	gpio_d3 = (u32)gpio_get_value(info->sim_pmu_hardware.esim_ctrl.gpio_d3);

	gpio_free(info->sim_pmu_hardware.esim_ctrl.gpio_d0);
	gpio_free(info->sim_pmu_hardware.esim_ctrl.gpio_d1);
	gpio_free(info->sim_pmu_hardware.esim_ctrl.gpio_d2);
	gpio_free(info->sim_pmu_hardware.esim_ctrl.gpio_d3);

	(void)regulator_disable(sim_sd_io);
	devm_regulator_put(sim_sd_io);

	hwsim_logi("gpio_d0-3: %d, %d, %d, %d\n", gpio_d0, gpio_d1, gpio_d2, gpio_d3);
	if ((gpio_d0 == 0) && (gpio_d1 == 0) && (gpio_d2 == 1) && (gpio_d3 == 1)) {
		hwsim_loge("no sim card in phy slot\n");
		return SIM_HW_CARD_NO_EXIST_WHILE_ESIM;
	} else {
		hwsim_loge("sim card in phy slot\n");
		return SIM_HW_CARD_EXIST_WHILE_ESIM;
	}
}

static sci_hw_op_result_e esim_phy_card_det(struct hw_sim_hardware_info *info)
{
	int ret;
	struct regulator *sim_sd_io = NULL;
#ifdef CONFIG_MMC_DW_MUX_SDSIM
	if ((u32)get_card1_type() == STATUS_SD) {
		hwsim_loge("current card type is SD\n");
		return SIM_HW_CARD_NO_EXIST_WHILE_ESIM;
	}
#else
	return SIM_HW_CARD_NO_EXIST_WHILE_ESIM;
#endif
	if (set_hw_sim_io_mux(info, SIM_IO_MUX_GPIO_DET_REQUEST) != HW_SIM_OK) {
		hwsim_loge("set det gpio fail\n");
		return SIM_HW_CARD_NO_EXIST_WHILE_ESIM;
	}

	hwsim_logi("sim_pmu_hardware_type: %d\n", info->sim_pmu_hardware.sim_pmu_hardware_type);
	ret = gpio_direction_output(info->sim_pmu_hardware.nvt_shift.nvt_shift_gpio, 1);
	if (ret < 0) {
		hwsim_loge("fail to set nvt nvt_shift_gpio, ret: %d\n", ret);
		return SIM_HW_CARD_NO_EXIST_WHILE_ESIM;
	}

	if (info->sim_pmu_hardware.sim_pmu_hardware_type == SIM_POWER_TYPE_SIM_SD) {
		ret = hw_sim_read_regulator(&info->sim_pmu_hardware.hard_cfg.sim_sd_pmu.sim_sd_io,
			info->spmidev, "sim_sd_io");
		if (ret != HW_SIM_OK) {
			hwsim_loge("fail to read sim_sd_pmu.sim_sd_io\n");
			return SIM_HW_CARD_NO_EXIST_WHILE_ESIM;
		}
		sim_sd_io = info->sim_pmu_hardware.hard_cfg.sim_sd_pmu.sim_sd_io;
	} else if (info->sim_pmu_hardware.sim_pmu_hardware_type == SIM_POWER_TYPE_EXTRAL_LDO) {
		ret = hw_sim_read_regulator(&info->sim_pmu_hardware.hard_cfg.extral_ldo_pmu.sim_sd_io,
			info->spmidev, "sim_sd_io");
		if (ret != HW_SIM_OK) {
			hwsim_loge("fail to read extral_ldo_pmu.sim_sd_io\n");
			return SIM_HW_CARD_NO_EXIST_WHILE_ESIM;
		}
		sim_sd_io = info->sim_pmu_hardware.hard_cfg.extral_ldo_pmu.sim_sd_io;
	} else {
		hwsim_loge("CARD_NO_EXIST_WHILE_ESIM\n");
		return SIM_HW_CARD_NO_EXIST_WHILE_ESIM;
	}

	if (hw_sim_enable_regulator(sim_sd_io, VCC_3_0V) != HW_SIM_OK) {
		hwsim_loge("fail to hw_sim_enable_regulator 3v\n");
		return SIM_HW_CARD_NO_EXIST_WHILE_ESIM;
	}

	udelay(120); // 0.12ms
	return esim_phy_card_det_level(info, sim_sd_io);
}

static int disable_sim_io_regulator(const sim_pmu_hardware_ctrl *sim_pmu_hardware)
{
	int ret = HW_SIM_OK;

	if (sim_pmu_hardware->sim_pmu_hardware_type == SIM_POWER_TYPE_SIM_SD)
		ret = hw_sim_disable_regulator(sim_pmu_hardware->hard_cfg.sim_sd_pmu.sim_sd_io);
	else if (sim_pmu_hardware->sim_pmu_hardware_type == SIM_POWER_TYPE_EXTRAL_LDO)
		ret = hw_sim_disable_regulator(sim_pmu_hardware->hard_cfg.extral_ldo_pmu.sim_sd_io);
	else
		hwsim_logi("the power type not need to disable pmu\n");

	return ret;
}

static int pmu_power_off_extral_ldo(const extral_ldo_pmu_cfg *extral_ldo_pmu)
{
	int ret;

	if (extral_ldo_pmu->extral_ldo_out_type == EXTRAL_LDO_OUT_TYPE_PMU) {
		ret = hw_sim_disable_regulator(extral_ldo_pmu->extral_ldo_out_select);
		if (ret != HW_SIM_OK)
			return HW_SIM_ERR;
	}

	if (extral_ldo_pmu->extral_ldo_bias_use == 1)
		(void)boost_5v_enable(BOOST_5V_DISABLE, BOOST_CTRL_SIM);

	return HW_SIM_OK;
}

static int extral_modem_power_off_a(const extral_modem_pmu_cfg *extral_modem_pmu)
{
	int ret = HW_SIM_OK;

	if (regulator_is_enabled(extral_modem_pmu->sd_io_pmu)) {
		ret = regulator_disable(extral_modem_pmu->sd_io_pmu);
		devm_regulator_put(extral_modem_pmu->sd_io_pmu);
		if (ret != 0) {
			hwsim_loge("disable io_pmu failed\n");
			(void)regulator_disable(extral_modem_pmu->sd_switch_pmu);
			(void)regulator_disable(extral_modem_pmu->sd_vcc_pmu);
			devm_regulator_put(extral_modem_pmu->sd_switch_pmu);
			devm_regulator_put(extral_modem_pmu->sd_vcc_pmu);
			return HW_SIM_ERR;
		}
	} else {
		hwsim_logi("disable io_pmu done, regulator disabled\n");
	}

	if (regulator_is_enabled(extral_modem_pmu->sd_switch_pmu)) {
		ret = regulator_disable(extral_modem_pmu->sd_switch_pmu);
		devm_regulator_put(extral_modem_pmu->sd_switch_pmu);
		if (ret != 0) {
			hwsim_loge("disable sd_switch failed\n");
			(void)regulator_disable(extral_modem_pmu->sd_vcc_pmu);
			devm_regulator_put(extral_modem_pmu->sd_vcc_pmu);
			return HW_SIM_ERR;
		}
	} else {
		hwsim_logi("disable switch_pmu done, regulator disabled\n");
	}

	if (regulator_is_enabled(extral_modem_pmu->sd_vcc_pmu)) {
		ret = regulator_disable(extral_modem_pmu->sd_vcc_pmu);
		devm_regulator_put(extral_modem_pmu->sd_vcc_pmu);
		if (ret != 0) {
			hwsim_loge("disable vcc_pmu failed\n");
			return HW_SIM_ERR;
		}
	} else {
		hwsim_logi("disable vcc_pmu done, regulator disabled\n");
	}

	return ret;
}

static int extral_modem_power_off_b(const extral_modem_pmu_cfg *extral_modem_pmu)
{
	int ret = HW_SIM_OK;

	if (regulator_is_enabled(extral_modem_pmu->sd_io_pmu)) {
		ret = regulator_disable(extral_modem_pmu->sd_io_pmu);
		devm_regulator_put(extral_modem_pmu->sd_io_pmu);
		if (ret != 0) {
			hwsim_loge("disable io_pmu failed\n");
			(void)regulator_disable(extral_modem_pmu->sd_vcc_pmu);
			devm_regulator_put(extral_modem_pmu->sd_vcc_pmu);
			return HW_SIM_ERR;
		}
	} else {
		hwsim_logi("disable io_pmu done, regulator disabled\n");
	}

	if (regulator_is_enabled(extral_modem_pmu->sd_vcc_pmu)) {
		ret = regulator_disable(extral_modem_pmu->sd_vcc_pmu);
		devm_regulator_put(extral_modem_pmu->sd_vcc_pmu);
		if (ret != 0) {
			hwsim_loge("disable vcc_pmu failed\n");
			return HW_SIM_ERR;
		}
	} else {
		hwsim_logi("disable vcc_pmu done, regulator disabled\n");
	}

	return ret;
}

static int sim_pmu_power_off_for_extral_modem(extral_modem_pmu_cfg *extral_modem_pmu)
{
	int ret = HW_SIM_OK;

	if (extral_modem_pmu->sim1_ap_power_scheme == AP_POWER_SCHE_A)
		ret = extral_modem_power_off_a(extral_modem_pmu);
	else if (extral_modem_pmu->sim1_ap_power_scheme == AP_POWER_SCHE_B)
		ret = extral_modem_power_off_b(extral_modem_pmu);

	return ret;
}

static int power_on_esim(struct hw_sim_hardware_info *info, s32 vol)
{
	int ret;

	hwsim_logi("esim pmu power on\n");
	ret = hw_sim_read_regulator(&info->sim_pmu_hardware.esim_ctrl.esim_vcc, info->spmidev, "esim_vcc");
	if (ret != HW_SIM_OK) {
		hwsim_loge("fail to read esim vcc\n");
		return HW_SIM_ERR;
	}

	hwsim_logi("hw_sim_enable_regulator\n");
	ret = hw_sim_enable_regulator(info->sim_pmu_hardware.esim_ctrl.esim_vcc, vol);
	enable_lv_shift(info);
	return ret;
}

static int power_off_esim(struct hw_sim_hardware_info *info)
{
	int ret;

	hwsim_logi("power off esim lv_shift\n");
	disable_lv_shift(info);

	ret = hw_sim_read_regulator(&info->sim_pmu_hardware.esim_ctrl.esim_vcc, info->spmidev, "esim_vcc");
	if (ret != HW_SIM_OK) {
		hwsim_loge("fail to read esim vcc\n");
		return HW_SIM_ERR;
	}

	ret = hw_sim_disable_regulator(info->sim_pmu_hardware.esim_ctrl.esim_vcc);
	hwsim_logi("power off esim, ret: %d\n", ret);
	return ret;
}

static int power_esim(struct hw_sim_hardware_info *info, s32 vol)
{
	int ret;
	if (info->esim_classc_only == 1 && vol == VCC_3_0V) {
		hwsim_logi("esim only 1.8v ,3v return directly\n");
		return HW_SIM_ERR;
	}
	ret = power_on_esim(info, vol);
	put_regulators(info);
	return ret;
}

static int hw_sim_pmu_power_on_simsd(const struct regulator *sim_vcc_pmu,
	struct hw_sim_hardware_info *info, s32 vol)
{
	int ret = hw_sim_enable_regulator(sim_vcc_pmu, vol); // ldo16 on
	if (ret != HW_SIM_OK)
		hwsim_loge("power on sd io sim_vcc fail\n");
	udelay(1000); // 1ms
	if (info->sim_pmu_hardware.nvt_shift.func == 1)
		ret += hw_sim_enable_regulator(info->sim_pmu_hardware.hard_cfg.sim_sd_pmu.sim_sd_io, VCC_3_0V); // ldo11 on
	if (set_hw_sim_io_mux(info, SIM_IO_MUX_SIMIO_GPIO_NANOSDIO_USIM_REQUEST) != HW_SIM_OK) // usim function
		hwsim_loge("set_hw_sim_io_mux fail\n");

	regulator_disable(sim_vcc_pmu); // ldo16 off
	udelay(10000); // 10ms
	udelay(10000); // 10ms
	ret += hw_sim_enable_regulator(sim_vcc_pmu, vol); // ldo16 on
	if (ret != HW_SIM_OK)
		hwsim_loge("power on sd io sim_vcc fail\n");
	hwsim_logi("ldo16, 1ms, ldo11, usim, ldo16_off, 20ms, ldo16\n");
	return ret;
}

static int hw_sim_pmu_power_on(u32 sim_id, struct hw_sim_hardware_info *info, s32 vol)
{
	int ret = HW_SIM_OK;
	struct regulator *sim_vcc_pmu = NULL;

	hwsim_logi("sim_id: %d, power_type: %d\n", sim_id, info->sim_pmu_hardware.sim_pmu_hardware_type);

	if (info->sim_esim_type == SIM_TYPE_ESIM)
		return power_esim(info, vol);

	if (hw_sim_read_regulator_by_type(info) != HW_SIM_OK) {
		hwsim_loge("read regulator fail\n");
		return HW_SIM_ERR;
	}

	if (hw_sim_ctrl_sd_io_regulator(info, vol) != HW_SIM_OK) {
		hwsim_loge("power on sd io fail\n");
		return HW_SIM_ERR;
	}

	sim_vcc_pmu = get_sim_vcc_regulator(&info->sim_pmu_hardware);
	switch (info->sim_pmu_hardware.sim_pmu_hardware_type) {
	case SIM_POWER_TYPE_SINGLE:
		ret = hw_sim_enable_regulator(sim_vcc_pmu, vol);
		break;

	case SIM_POWER_TYPE_SIM_SD:
		ret = hw_sim_pmu_power_on_simsd(sim_vcc_pmu, info, vol);
		break;

	case SIM_POWER_TYPE_EXTRAL_MODEM:
		ret = sim_extral_modem_power_on(&info->sim_pmu_hardware.hard_cfg.extral_modem_pmu, vol);
		break;

	case SIM_POWER_TYPE_EXTRAL_LDO:
		ret = pmu_power_on_extral_ldo(info, vol);
		break;

	case SIM_POWER_TYPE_EXTRAL_LDO_BUT_NO_SD:
		ret = pmu_power_on_extral_ldo_but_no_sd(info, vol);
		break;

	case SIM_POWER_TYPE_AUX:
		ret = hw_sim_enable_regulator(info->sim_pmu_hardware.hard_cfg.aux_sim_pmu.aux_ldo, vol);
		ret += hw_sim_enable_regulator(info->sim_pmu_hardware.hard_cfg.aux_sim_pmu.sim_vcc, vol);
		break;

	default:
		break;
	}

	// lv shift enabled
	if ((info->sim_pmu_hardware.lv_shift_ctrl.func == 1) && (info->sim_pmu_hardware.nvt_shift.func == 0))
		enable_lv_shift(info);

	put_regulators(info);
	if (ret == HW_SIM_OK)
		hwsim_logi("pmu power on done\n");
	else
		hwsim_loge("sim pmu power on fail, power_type: %d\n", info->sim_pmu_hardware.sim_pmu_hardware_type);

	return ret;
}

static int hw_sim_pmu_power_off_regulator(const struct hw_sim_hardware_info *info)
{
	struct regulator *sim_vcc_pmu = get_sim_vcc_regulator(&info->sim_pmu_hardware);
	int ret = hw_sim_disable_regulator(sim_vcc_pmu);
	if (ret != HW_SIM_OK) {
		hwsim_loge("power off vcc fail\n");
		return HW_SIM_ERR;
	}
	hwsim_logi("power off vcc done\n");

	if (info->sim_pmu_hardware.sim_pmu_hardware_type == SIM_POWER_TYPE_EXTRAL_LDO ||
		info->sim_pmu_hardware.sim_pmu_hardware_type == SIM_POWER_TYPE_EXTRAL_LDO_BUT_NO_SD) {
		ret = pmu_power_off_extral_ldo(&(info->sim_pmu_hardware.hard_cfg.extral_ldo_pmu));
		if (ret != HW_SIM_OK)
			hwsim_loge("power off extral ldo fail\n");
	}

	if (info->sim_pmu_hardware.nvt_shift.func == 1) {
		ret = gpio_direction_output(info->sim_pmu_hardware.nvt_shift.nvt_shift_gpio, 1);
		ret += set_io_for_nvt_shift(info);
		ret += disable_sim_io_regulator(&info->sim_pmu_hardware);
		if (ret != HW_SIM_OK)
			hwsim_loge("power off sd io fail\n");
	} else {
		if (info->sim_pmu_hardware.sim_pmu_hardware_type == SIM_POWER_TYPE_AUX) {
			ret = hw_sim_disable_regulator(info->sim_pmu_hardware.hard_cfg.aux_sim_pmu.aux_ldo);
			if (ret == HW_SIM_OK)
				hwsim_logi("power off aux pmu done\n");
			else
				hwsim_loge("power off aux pmu failed\n");
		} else if (info->sim_pmu_hardware.sim_pmu_hardware_type == SIM_POWER_TYPE_EXTRAL_LDO_BUT_NO_SD) {
			ret = hw_sim_disable_regulator(info->sim_pmu_hardware.esim_ctrl.esim_vcc);
			if (ret == HW_SIM_OK)
				hwsim_logi("power off esim_vcc(io) done\n");
			else
				hwsim_loge("power off esim_vcc(io) failed\n");
		}
	}

	return ret;
}

static int hw_sim_pmu_power_off(u32 sim_id, struct hw_sim_hardware_info *info)
{
	int ret;

	hwsim_logi("sim_id: %d, power_type: %d\n", sim_id, info->sim_pmu_hardware.sim_pmu_hardware_type);
	if (info->sim_esim_type == SIM_TYPE_ESIM) {
		ret = power_off_esim(info);
		return ret;
	}

	if (hw_sim_read_regulator_by_type(info) != HW_SIM_OK) {
		hwsim_loge("read regulator fail\n");
		return HW_SIM_ERR;
	}

	if (info->sim_pmu_hardware.sim_pmu_hardware_type == SIM_POWER_TYPE_EXTRAL_MODEM) {
		hwsim_logi("power off extral modem\n");
		return sim_pmu_power_off_for_extral_modem(&info->sim_pmu_hardware.hard_cfg.extral_modem_pmu);
	}

	if (info->sim_pmu_hardware.nvt_shift.func == 0) {
		hwsim_logi("not use nvt shift\n");
		disable_sim_io_regulator(&info->sim_pmu_hardware);
	}

	if (info->sim_pmu_hardware.nvt_shift.func == 0 && info->sim_pmu_hardware.lv_shift_ctrl.func == 1)
		disable_lv_shift(info);

	ret = hw_sim_pmu_power_off_regulator(info);
	if (info->sim_pmu_hardware.hard_cfg.extral_ldo_pmu.extral_ldo_out_type == EXTRAL_LDO_OUT_TYPE_GPIO) {
		gpio_free(info->sim_pmu_hardware.hard_cfg.extral_ldo_pmu.extral_ldo_out_gpio_select);
		hwsim_logi("gpio_free extral_ldo_out_gpio_select\n");
	}

	return ret;
}

static char* msg_id_to_string(sci_hw_op_type_e msg_id)
{
	switch (msg_id) {
	case SIM_HW_CARD_TYPE_INQUIRE:
		return "CARD_TYPE_INQUIRE";

	case SIM_HW_CARD_DET_WHILE_ESIM_INQUIRE:
		return "CARD_DET_WHILE_ESIM_INQUIRE";

	case SIM_HW_POWER_ON_CLASS_C_REQUEST:
		return "POWER_ON_CLASS_C";

	case SIM_HW_POWER_ON_CLASS_B_REQUEST:
		return "POWER_ON_CLASS_B";

	case SIM_HW_POWER_OFF_REQUEST:
		return "POWER_OFF";

	case SIM_HW_SET_ESIM_SWITCH:
		return "SET_ESIM_SWITCH";

	case SIM_HW_SET_USIM_SWITCH:
		return "SET_USIM_SWITCH";

	default:
		return "UNKNOWN";
	}
}

sci_hw_op_result_e sim_hwadp_handler(u32 sim_id, sci_hw_op_type_e msg_id)
{
	sci_hw_op_result_e rslt = SIM_HW_REQUEST_RESULT_BUTT;
	struct hw_sim_hardware_info *info = NULL;

	hwsim_logi("receive msg, sim_id: %d, msg_id: 0x%02x(%s)\n", sim_id, msg_id, msg_id_to_string(msg_id));
	if (sim_id >= MAX_SIMHP_NUM)
		return SIM_HW_REQUEST_RESULT_BUTT;

	info = g_hw_simhw_info[sim_id];
	if (info == NULL) {
		hwsim_loge("hw sim info is null\n");
		return SIM_HW_REQUEST_RESULT_BUTT;
	}

	switch (msg_id) {
	case SIM_HW_CARD_TYPE_INQUIRE:
		rslt = sim_inquire_card_type(sim_id, info);
		break;

	case SIM_HW_CARD_DET_WHILE_ESIM_INQUIRE:
		rslt = esim_phy_card_det(info);
		break;

	case SIM_HW_POWER_ON_CLASS_C_REQUEST:
		if (hw_sim_pmu_power_on(sim_id, info, VCC_1_8V) != HW_SIM_OK)
			rslt = SIM_HW_POWER_ON_FAIL;
		else
			rslt = SIM_HW_POWER_ON_SUCC;
		break;

	case SIM_HW_POWER_ON_CLASS_B_REQUEST:
		if (hw_sim_pmu_power_on(sim_id, info, VCC_3_0V) != HW_SIM_OK)
			rslt = SIM_HW_POWER_ON_FAIL;
		else
			rslt = SIM_HW_POWER_ON_SUCC;
		break;

	case SIM_HW_POWER_OFF_REQUEST:
		if (hw_sim_pmu_power_off(sim_id, info) != HW_SIM_OK)
			rslt = SIM_HW_POWER_OFF_FAIL;
		else
			rslt = SIM_HW_POWER_OFF_SUCC;
		break;

	case SIM_HW_SET_ESIM_SWITCH:
		info->sim_esim_type = SIM_TYPE_ESIM;
		rslt = SIM_HW_SET_ESIM_SUCC;
		break;

	case SIM_HW_SET_USIM_SWITCH:
		info->sim_esim_type = SIM_TYPE_USIM;
		rslt = SIM_HW_SET_USIM_SUCC;
		break;

	default:
		break;
	}

	hwsim_logi("sim_id: %d, msg_id: 0x%02x done, result: %d(0x%02x)\n", sim_id, msg_id, rslt, rslt);
	return rslt;
}
EXPORT_SYMBOL(sim_hwadp_handler); //lint !e580

int sim_hwadp_init(u32 sim_id, struct device *dev)
{
	struct hw_sim_hardware_info *info = NULL;
	struct device_node *np = NULL;
	int ret;
	u32 res;

	hwsim_logi("sim_id: %d\n", sim_id);
	if (dev == NULL) {
		hwsim_loge("dev is null\n");
		return HW_SIM_ERR;
	}

	if (sim_id == SIM0)
		np = of_find_compatible_node(NULL, 0, HW_SIM_HARDWARE0);
	else if (sim_id == SIM1)
		np = of_find_compatible_node(NULL, 0, HW_SIM_HARDWARE1);
	else
		return HW_SIM_ERR;

	if (np == NULL) {
		hwsim_loge("failed to find hw sim device node\n");
		return HW_SIM_ERR;
	}

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (info == NULL) {
		hwsim_loge("failed to allocate memory\n");
		return HW_SIM_ERR;
	}

	info->spmidev = dev;
	info->np = np;
	info->sim_id = sim_id;
	info->sim_esim_type = SIM_TYPE_USIM;
	info->sim_status = STATUS_NO_CARD;

	ret = hw_sim_read_dts(info, np);
	if (ret != HW_SIM_OK) {
		hwsim_loge("hw sim read dts fail\n");
		kfree(info);
		info = NULL;
		return ret;
	}

	res = sim_io_mux_init(info, np);
	res += sd_io_mux_init(info, np);
	g_hw_simhw_info[sim_id] = info;
	hwsim_logi("hw sim hardware done, res: %u\n", res);
	return HW_SIM_OK;
}
EXPORT_SYMBOL(sim_hwadp_init); //lint !e580

int sim_get_hw_debug_info(u32 sim_id, struct sim_hw_debug_info *dbg_info)
{
	(void)sim_id;
	dbg_info->event_len = 0;
	return 0;
}
EXPORT_SYMBOL(sim_get_hw_debug_info); //lint !e580
/*lint +e605*/
#pragma GCC diagnostic pop
