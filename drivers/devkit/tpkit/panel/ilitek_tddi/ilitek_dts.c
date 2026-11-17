#include "ilitek.h"
#include "tpkit_platform_adapter.h"

static void ilitek_tddi_of_property_read_u32_default(
	struct device_node *np,
	char *prop_name,
	u32 *out_value,
	u32 default_value)
{
	int ret = 0;

	ret = of_property_read_u32(np, prop_name, out_value);
	if (ret) {
		TS_LOG_INFO("of_property_read_u32_default:%s not set in dts, use default\n",
			prop_name);
		*out_value = default_value;
	}
}

/*  must config dts propertys to chip detect */
int ilitek_tddi_prase_ic_config_dts(struct device_node *np)
{
	struct ts_kit_device_data* ts_dev_data = ilits->ts_dev_data;

	ilitek_tddi_of_property_read_u32_default(np, ILITEK_DTS_SLAVE_ADDR,
		&ts_dev_data->slave_addr, 0x41);
	TS_LOG_INFO("slave addr 0x%x\n", ts_dev_data->slave_addr);

	return 0;
}

static int ilitek_parse_report_config_dts(struct device_node *np)
{
	int ret = 0;

	ret = of_property_read_u32(np, ILITEK_DTS_X_MAX, &ilits->max_x);
	if (ret) {
		TS_LOG_INFO("get x_max failed, use default: %d\n",
			TOUCH_SCREEN_X_MAX);
		ilits->max_x = TOUCH_SCREEN_X_MAX;
	}

	ret = of_property_read_u32(np, ILITEK_DTS_Y_MAX, &ilits->max_y);
	if (ret) {
		TS_LOG_INFO("get y_max failed, use default: %d\n",
			TOUCH_SCREEN_Y_MAX);
		ilits->max_y = TOUCH_SCREEN_Y_MAX;
	}

	ret = of_property_read_u32(np, ILITEK_DTS_X_MIN, &ilits->min_x);
	if (ret) {
		TS_LOG_INFO("get x_min failed, use default: %d\n",
			TOUCH_SCREEN_X_MIN);
		ilits->min_x = TOUCH_SCREEN_X_MIN;
	}

	ret = of_property_read_u32(np, ILITEK_DTS_Y_MIN, &ilits->min_y);
	if (ret) {
		TS_LOG_INFO("get y_min failed, use default: %d\n",
			TOUCH_SCREEN_Y_MIN);
		ilits->min_y = TOUCH_SCREEN_Y_MIN;
	}

	ret = of_property_read_u32(np, ILITEK_DTS_FINGER_NUMS, &ilits->finger_nums);
	if (ret) {
		TS_LOG_INFO("get finger_nums failed, use default: %d\n",
			MAX_TOUCH_NUM);
		ilits->finger_nums = MAX_TOUCH_NUM;
	}

	TS_LOG_INFO("x = [%d, %d], y = [%d, %d], finger_nums = %d\n",
		ilits->min_x,
		ilits->max_x,
		ilits->min_y,
		ilits->max_y,
		ilits->finger_nums);

	return 0;
}

int ilitek_parse_ic_special_dts(struct device_node *np)
{
	int ret = 0;
	const char *str_value = NULL;
	struct device_node *self = NULL;
	struct ts_kit_device_data* ts_dev_data = ilits->ts_dev_data;

	self = of_find_node_by_name(np, ilits->project_id);
	if (IS_ERR_OR_NULL(self)) {
		TS_LOG_ERR("find %s node failed\n", ilits->project_id);
		return -ENODATA;
	}

	ret = of_property_read_string(self, ILITEK_DTS_PRODUCER, &str_value);
	if (ret) {
		TS_LOG_INFO("get module_name failed, use default: %s\n",
			ILITEK_DTS_DEF_PRODUCER);
		str_value = ILITEK_DTS_DEF_PRODUCER;
	}
	strncpy(ts_dev_data->module_name, str_value, MAX_STR_LEN);

	return 0;
}

int ilitek_parse_dts(struct device_node *np)
{
	int ret = 0;
	const char *str_value = NULL;
	struct ts_kit_device_data* ts_dev_data = ilits->ts_dev_data;
	struct device_node *dts_np;
	dts_np = np;

	ret = of_property_read_u32(dts_np, ILITEK_DTS_IC_TYEP, &ts_dev_data->ic_type);
	if (ret < 0) {
		TS_LOG_INFO("get ic_type failed, use default: %d\n", "78");
		ts_dev_data->ic_type = TDDI;
	}

	ret = of_property_read_u32(dts_np, ILITEK_DTS_ALGO_ID, &ts_dev_data->algo_id);
	if (ret < 0) {
		TS_LOG_INFO("get algo_id failed, use default: %d\n",
			ILITEK_DTS_DEF_ALGO);
		ts_dev_data->algo_id = ILITEK_DTS_DEF_ALGO;
	}
	TS_LOG_INFO("algo_id = %d\n", ts_dev_data->algo_id);

	ret = of_property_read_u32(dts_np, ILITEK_DTS_IRQ_CFG, &ts_dev_data->irq_config);
	if (ret < 0) {
		TS_LOG_INFO("get irq_config failed, use default: %d\n",
			ILITEK_DTS_DEF_IRQ_CONFIG);
		ts_dev_data->irq_config = ILITEK_DTS_DEF_IRQ_CONFIG;
	}
	TS_LOG_INFO("irq_config = %d\n", ts_dev_data->irq_config);

	ret = of_property_read_u32(dts_np, ILITEK_DTS_USE_IC_RES, &ilits->use_ic_res);
	if (ret) {
		TS_LOG_INFO("get use_ic_res failed, use default: %d\n",
			ILITEK_DTS_DEF_USE_IC_RES);
		ilits->use_ic_res = ILITEK_DTS_DEF_USE_IC_RES;
	}
	TS_LOG_INFO("use_ic_res = %d\n", ilits->use_ic_res);

	/* convert ic resolution x[0,2047] y[0,2047] to real resolution, fix inaccurate touch in recovery mode,
	 * there is no coordinate conversion in recovery mode.
	 */
	if (get_into_recovery_flag_adapter()) {
		ilits->use_ic_res = ILITEK_DTS_DEF_USE_IC_RES;
		TS_LOG_INFO("recovery mode driver convert resolution\n");
	}

	ret = of_property_read_u32(dts_np, ILITEK_DTS_ROI, &ilits->support_roi);
	if (ret) {
		TS_LOG_INFO("get support_roi failed, use default: %d\n",
			ILITEK_DTS_DEF_SUPPORT_ROI);
		ilits->support_roi = ILITEK_DTS_DEF_SUPPORT_ROI;
	}
	ts_dev_data->ts_platform_data->feature_info.roi_info.roi_supported = ilits->support_roi;
	TS_LOG_INFO("support_roi = %d\n", ilits->support_roi);


	ret = of_property_read_u32(dts_np, "download_fw_inrecovery",
		&ts_dev_data->download_fw_inrecovery);
	if (ret) {
		ts_dev_data->download_fw_inrecovery = 0;
		TS_LOG_INFO("get download_fw_inrecovery fail,use default\n");
	}
	ret = of_property_read_u32(np, "use_lcdkit_power_notify",
		&ts_dev_data->use_lcdkit_power_notify);
	if (ret) {
		ts_dev_data->use_lcdkit_power_notify = 0;
		TS_LOG_INFO("get use_lcdkit_power_notify fail,use default\n");
	}
	ret = of_property_read_u32(dts_np, "hide_fw_name",
		&ts_dev_data->hide_fw_name);
	if (ret) {
		ts_dev_data->hide_fw_name = 0;
		TS_LOG_INFO("get hide_fw_name fail,use default\n");
	}

	ret = of_property_read_u32(dts_np, "touch_switch_flag",
		&ts_dev_data->touch_switch_flag);
	if (ret) {
		ts_dev_data->touch_switch_flag = 0;
		TS_LOG_INFO("get touch_switch fail,use default\n");
	}

	ret = of_property_read_u32(dts_np, ILITEK_DTS_GESTURE, &ilits->support_gesture);
	if (ret) {
		TS_LOG_INFO("get support_gesture failed, use default: %d\n",
			ILITEK_DTS_DEF_SUPPORT_GESTURE);
		ilits->support_gesture = ILITEK_DTS_DEF_SUPPORT_GESTURE;
	}
	ts_dev_data->ts_platform_data->feature_info.wakeup_gesture_enable_info.switch_value = ilits->support_gesture;
	TS_LOG_INFO("support_gesture = %d\n", ilits->support_gesture);

	ret = of_property_read_u32(dts_np, ILITEK_DTS_PRESSURE, &ilits->support_pressure);
	if (ret) {
		TS_LOG_INFO("get support_pressure failed, use default: %d\n",
			ILITEK_DTS_DEF_SUPPORT_PRESSURE);
		ilits->support_pressure = ILITEK_DTS_DEF_SUPPORT_PRESSURE;
	}
	TS_LOG_INFO("support_pressure = %d\n", ilits->support_pressure);

	ret = of_property_read_u32(dts_np, ILITEK_DTS_TP_COLOR,
		&ilits->support_get_tp_color);
	if (ret) {
		TS_LOG_INFO("get support_get_tp_color failed, use default: %d\n",
			ILITEK_DTS_DEF_TP_COLOR);
		ilits->support_get_tp_color = ILITEK_DTS_DEF_TP_COLOR;
	}
	TS_LOG_INFO("support_get_tp_color = %d\n", ilits->support_get_tp_color);

	ret = of_property_read_u32(dts_np, ILITEK_DTS_PROJECT_ID_CTRL,
		&ilits->project_id_length_control);
	if (ret) {
		TS_LOG_INFO("get project_id_length_control failed, use default: %d\n",
			ILITEK_DTS_DEF_PROJECT_ID_CTRL);
		ilits->project_id_length_control = ILITEK_DTS_DEF_PROJECT_ID_CTRL;
	}
	TS_LOG_INFO("project_id_length_control = %d\n", ilits->project_id_length_control);

	ret = of_property_read_u32(dts_np, ILITEK_DTS_RAW_DATA_PRINT,
	   (u32*) &ts_dev_data->is_ic_rawdata_proc_printf);
	if (ret) {
		TS_LOG_INFO("get is_ic_rawdata_proc_printf failed, use default: %d\n",
			ILITEK_DTS_DEF_RAWDATA_PRINT);
		ts_dev_data->is_ic_rawdata_proc_printf = ILITEK_DTS_DEF_RAWDATA_PRINT;
	}
	TS_LOG_INFO("is_ic_rawdata_proc_printf = %d\n", ts_dev_data->is_ic_rawdata_proc_printf);

	ret = of_property_read_u32(dts_np, ILITEK_DTS_OPEN_ONCE_THRESHOLD,
		&ilits->only_open_once_captest_threshold);
	if (ret) {
		TS_LOG_INFO("get only_open_once_captest_threshold failed, use default: %d\n",
			ILITEK_DTS_DEF_TH_OPEN_ONCE);
		ilits->only_open_once_captest_threshold = ILITEK_DTS_DEF_TH_OPEN_ONCE;
	}
	TS_LOG_INFO("only_open_once_captest_threshold = %d\n", ilits->only_open_once_captest_threshold);

	ret = of_property_read_string(dts_np, ILITEK_DTS_TEST_TYPE, &str_value);
	if (ret) {
		TS_LOG_INFO("get tp_test_type failed, use default: %s\n",
			ILITEK_DTS_DEF_TEST_TYPE);
		str_value = ILITEK_DTS_DEF_TEST_TYPE;
	}
	strncpy(ts_dev_data->tp_test_type, str_value, TS_CAP_TEST_TYPE_LEN);
	TS_LOG_INFO("tp_test_type = %s\n", ts_dev_data->tp_test_type);

	/* avoid read project id failed, tp work abnormally */
	ilitek_parse_report_config_dts(dts_np);

	TS_LOG_INFO("parse config in dts succeeded\n");

	return 0;
}
