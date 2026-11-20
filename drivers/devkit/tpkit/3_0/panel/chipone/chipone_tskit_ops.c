#include "cts_config.h"
#include "cts_platform.h"
#include "cts_core.h"
#include "cts_firmware.h"
#include "cts_test.h"
#include "cts_charger_detect.h"
#include "cts_earjack_detect.h"
#include "cts_sysfs.h"
#include "cts_strerror.h"

#if defined (CONFIG_TEE_TUI)
#include "tui.h"
#endif
#if defined (CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
#endif

struct chipone_ts_data *cts_data = NULL;

static void parse_dts_project_basic(const struct device_node *proj_of_node)
{
	int ret;
	const char *producer = NULL;

	TS_LOG_INFO("* Project basic param:\n");
	ret = of_property_read_string(proj_of_node, "producer", &producer);
	if (ret) {
		strncpy(cts_data->tskit_data->module_name, "producer", MAX_STR_LEN - 1);
		TS_LOG_INFO("get module_name failed, use default: producer\n");
	} else {
		strncpy(cts_data->tskit_data->module_name, producer, MAX_STR_LEN - 1);
	}
	TS_LOG_INFO(" - %-32s: %s\n", "producer",
		cts_data->tskit_data->module_name);
	ret = of_property_read_u8_array(proj_of_node, "boot-hw-reset-timing",
		cts_data->boot_hw_reset_timing,
		ARRAY_SIZE(cts_data->boot_hw_reset_timing));
	if (ret) {
		TS_LOG_INFO("Get 'boot-hw-reset-timing' from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		cts_data->boot_hw_reset_timing[0] = 1;
		cts_data->boot_hw_reset_timing[1] = 1;
		cts_data->boot_hw_reset_timing[2] = 50;
	}
	TS_LOG_INFO(" - %-32s: [H:%u, L:%u, H:%u]\n", "Boot hw reset timing",
		cts_data->boot_hw_reset_timing[0],
		cts_data->boot_hw_reset_timing[1],
		cts_data->boot_hw_reset_timing[2]);

	ret = of_property_read_u8_array(proj_of_node, "resume-hw-reset-timing",
		cts_data->resume_hw_reset_timing,
		ARRAY_SIZE(cts_data->resume_hw_reset_timing));
	if (ret) {
		TS_LOG_INFO("Get 'resume-hw-reset-timing' from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		cts_data->resume_hw_reset_timing[0] = 1;
		cts_data->resume_hw_reset_timing[1] = 1;
		cts_data->resume_hw_reset_timing[2] = 20;
	}
	TS_LOG_INFO(" - %-32s: [H:%u, L:%u, H:%u]\n", "Resume hw reset timing",
		cts_data->resume_hw_reset_timing[0],
		cts_data->resume_hw_reset_timing[1],
		cts_data->resume_hw_reset_timing[2]);

	ret = of_property_read_u32(proj_of_node, "ic_type",
		&cts_data->tskit_data->ic_type);
	if (ret) {
		TS_LOG_ERR("Get ic-type from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		cts_data->tskit_data->ic_type = CTS_DEV_HWID_ICNL9911C;
	}
	TS_LOG_INFO(" - %-32s: 0x%06x\n", "IC type",
		cts_data->tskit_data->ic_type);

	ret = of_property_read_u32(proj_of_node,
		"is_incell", &cts_data->tskit_data->is_in_cell);
	if (ret) {
		TS_LOG_ERR("Get is_incell from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		cts_data->tskit_data->is_in_cell = 1;
	}
	TS_LOG_INFO(" - %-32s: %c\n", "Is incell",
	cts_data->tskit_data->is_in_cell ? 'Y' : 'N');

	ret = of_property_read_u32(proj_of_node, "spi-mode", &cts_data->spi_mode);
	if (ret) {
		TS_LOG_ERR("Get spi-mode from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		cts_data->spi_mode = 0;
	}
	TS_LOG_INFO(" - %-32s: %u\n", "SPI mode", cts_data->spi_mode);

	ret = of_property_read_u32(proj_of_node, "spi-max-frequency",
		&cts_data->spi_max_speed);
	if (ret) {
		TS_LOG_ERR("Get spi-max-frequency from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		cts_data->spi_max_speed = 1000000;
	}
	TS_LOG_INFO(" - %-32s: %u\n", "SPI max speed", cts_data->spi_max_speed);

	ret = of_property_read_u32(proj_of_node,
		"algo_id", &cts_data->tskit_data->algo_id);
	if (ret) {
		TS_LOG_ERR("Get algo_id from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		cts_data->tskit_data->algo_id = 1;
	}
	TS_LOG_INFO(" - %-32s: %u\n", "Algo id",
		cts_data->tskit_data->algo_id);

	ret = of_property_read_u32(proj_of_node,
		"irq_config", &cts_data->tskit_data->irq_config);
	if (ret) {
		TS_LOG_ERR("Get irq_config from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		cts_data->tskit_data->irq_config = 1;
	}
	TS_LOG_INFO(" - %-32s: %u\n", "IRQ config",
		cts_data->tskit_data->irq_config);
}

static void parse_dts_input_config(const struct device_node *of_node)
{
	struct ts_kit_device_data *tskit_dev_data = cts_data->tskit_data;
	int ret;

	TS_LOG_INFO("* Project input config:\n");

	ret = of_property_read_u32(of_node, "x_max",
		(u32 *)&tskit_dev_data->x_max);
	if (ret) {
		TS_LOG_ERR("Get x_max from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		tskit_dev_data->x_max = 720;
	}
	TS_LOG_INFO(" - %-32s: %u\n", "X Max", tskit_dev_data->x_max);

	ret = of_property_read_u32(of_node, "y_max",
		(u32 *)&tskit_dev_data->y_max);
	if (ret) {
		TS_LOG_ERR("Get y_max from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		tskit_dev_data->y_max = 1600;
	}
	TS_LOG_INFO(" - %-32s: %u\n", "Y Max", tskit_dev_data->y_max);

	ret = of_property_read_u32(of_node, "x_max_mt",
		(u32 *)&tskit_dev_data->x_max_mt);
	if (ret) {
		TS_LOG_ERR("Get x_max_mt from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		tskit_dev_data->x_max_mt = 720;
	}
	TS_LOG_INFO(" - %-32s: %u\n", "X Max MT", tskit_dev_data->x_max_mt);

	ret = of_property_read_u32(of_node, "y_max_mt",
		(u32 *)&tskit_dev_data->y_max_mt);
	if (ret) {
		TS_LOG_ERR("Get y_max_mt from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		tskit_dev_data->y_max_mt = 1600;
	}
	TS_LOG_INFO(" - %-32s: %u\n", "Y Max MT", tskit_dev_data->y_max_mt);

	ret = of_property_read_u32(of_node, "max_fingers",
		(u32 *)&tskit_dev_data->ts_platform_data->max_fingers);
	if (ret) {
		TS_LOG_ERR("Get max_fingers from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		tskit_dev_data->ts_platform_data->max_fingers = TS_MAX_FINGER;
	}
	TS_LOG_INFO(" - %-32s: %u\n", "MAX Fingers",
		tskit_dev_data->ts_platform_data->max_fingers);
}

static void parse_dts_project_facotry_test(const struct device_node *proj_of_node)
{
	const char *str = NULL;
	int ret = 0;

	TS_LOG_INFO("* Project factory test:\n");

	cts_data->test_reset_pin =
		of_property_read_bool(proj_of_node, "test-reset-pin");
	TS_LOG_INFO(" - %-32s: %c\n", "Test reset-pin",
		cts_data->test_reset_pin ? 'Y' : 'N');

	cts_data->test_int_pin =
		of_property_read_bool(proj_of_node, "test-int-pin");
	TS_LOG_INFO(" - %-32s: %c\n", "Test int-pin",
		cts_data->test_int_pin ? 'Y' : 'N');

	cts_data->test_rawdata =
		of_property_read_bool(proj_of_node, "test-rawdata");
	TS_LOG_INFO(" - %-32s: %c\n", "Test rawdata",
		cts_data->test_rawdata ? 'Y' : 'N');

	ret = of_property_read_u32(proj_of_node, "rawdata-test-frames",
		&cts_data->rawdata_test_priv_param.frames);
	if (ret) {
		TS_LOG_ERR("Get rawdata-test-frames from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		cts_data->rawdata_test_priv_param.frames = 1;
	}
	TS_LOG_INFO(" - %-32s: %u\n", "Rawdata test frames",
		cts_data->rawdata_test_priv_param.frames);

	ret = of_property_read_s32(proj_of_node,
		"rawdata-test-min", (s32 *)&cts_data->rawdata_test_min);
	if (ret) {
		TS_LOG_ERR("Get rawdata-test-min from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		cts_data->rawdata_test_min = 700;
	}
	TS_LOG_INFO(" - %-32s: %u\n", "Rawdata test min",
		cts_data->rawdata_test_min);
	ret = of_property_read_s32(proj_of_node,
		"rawdata-test-max", (s32 *)&cts_data->rawdata_test_max);
	if (ret) {
		TS_LOG_ERR("Get rawdata-test-max from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		cts_data->rawdata_test_max = 1500;
	}
	TS_LOG_INFO(" - %-32s: %u\n", "Rawdata test max",
		cts_data->rawdata_test_max);

	cts_data->test_noise =
		of_property_read_bool(proj_of_node, "test-noise");
	TS_LOG_INFO(" - %-32s: %c\n", "Test noise",
		cts_data->test_noise ? 'Y' : 'N');

	ret = of_property_read_u32(proj_of_node,
		"noise-test-frames", &cts_data->noise_test_priv_param.frames);
	if (ret) {
		TS_LOG_ERR("Get noise-test-frames from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		cts_data->noise_test_priv_param.frames = 16;
	}
	TS_LOG_INFO(" - %-32s: %u\n", "Noise test frames",
		cts_data->noise_test_priv_param.frames);

	ret = of_property_read_s32(proj_of_node,
		"noise-test-max", (s32 *)&cts_data->noise_test_max);
	if (ret) {
		TS_LOG_ERR("Get noise-test-max from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		cts_data->noise_test_max = 50;
	}
	TS_LOG_INFO(" - %-32s: %u\n", "Noise test max",
		cts_data->noise_test_max);

	cts_data->test_open = of_property_read_bool(proj_of_node, "test-open");
	TS_LOG_INFO(" - %-32s: %c\n", "Test open",
		cts_data->test_open ? 'Y' : 'N');

	ret = of_property_read_s32(proj_of_node,
		"open-test-min", (s32 *)&cts_data->open_test_min);
	if (ret) {
		TS_LOG_ERR("Get open-test-min from dts failed %d(%s)\n",
		ret, cts_strerror(ret));
		cts_data->open_test_min = 200;
	}
	TS_LOG_INFO(" - %-32s: %u\n", "Open test min",
		cts_data->open_test_min);

	cts_data->test_short =
		of_property_read_bool(proj_of_node, "test-short");
	TS_LOG_INFO(" - %-32s: %c\n", "Test short",
		cts_data->test_short ? 'Y' : 'N');
	ret = of_property_read_s32(proj_of_node,
		"short-test-min", (s32 *)&cts_data->short_test_min);
	if (ret) {
		TS_LOG_ERR("Get short-test-min from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		cts_data->short_test_min = 200;
	}
	TS_LOG_INFO(" - %-32s: %u\n", "Short test min",
		cts_data->short_test_min);

	cts_data->test_compensate_cap =
		of_property_read_bool(proj_of_node, "test-compensate-cap");
	TS_LOG_INFO(" - %-32s: %c\n", "Test compensate cap",
		cts_data->test_compensate_cap ? 'Y' : 'N');
	ret = of_property_read_s32(proj_of_node,
		"compensate-cap-test-min", (s32 *)&cts_data->compensate_cap_test_min);
	if (ret) {
		TS_LOG_ERR("Get compensate-cap-test-min from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		cts_data->compensate_cap_test_min = 20;
	}
	TS_LOG_INFO(" - %-32s: %u\n", "Compensate cap test min",
		cts_data->compensate_cap_test_min);
	ret = of_property_read_s32(proj_of_node, "compensate-cap-test-max",
		(s32 *)&cts_data->compensate_cap_test_max);
	if (ret) {
		TS_LOG_ERR("Get compensate-cap-test-max from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		cts_data->compensate_cap_test_max = 100;
	}
	TS_LOG_INFO(" - %-32s: %u\n", "Compensate cap test max",
		cts_data->compensate_cap_test_max);

	cts_data->dump_factory_test_data_to_console =
		of_property_read_bool(proj_of_node, "dump-factory-test-data-to-console");
	TS_LOG_INFO(" - %-32s: %c\n", "Dump test data to console",
		cts_data->dump_factory_test_data_to_console ? 'Y' : 'N');

	cts_data->dump_factory_test_data_to_buffer =
		of_property_read_bool(proj_of_node, "dump-factory-test-data-to-buffer");
	TS_LOG_INFO(" - %-32s: %c\n", "Dump test data to buffer",
		cts_data->dump_factory_test_data_to_buffer? 'Y' : 'N');

	cts_data->dump_factory_test_data_to_file =
		of_property_read_bool(proj_of_node, "dump-factory-test-data-to-file");
	TS_LOG_INFO(" - %-32s: %c\n", "Dump test data to file",
		cts_data->dump_factory_test_data_to_file ? 'Y' : 'N');

	cts_data->print_factory_test_data_only_fail =
		of_property_read_bool(proj_of_node, "print-factory-test-data-only-fail");
	TS_LOG_INFO(" - %-32s: %c\n", "Dump test data only fail",
		cts_data->print_factory_test_data_only_fail ? 'Y' : 'N');

	ret = of_property_read_string(proj_of_node,
		"factory-test-data-directory", &str);
	if (ret) {
		TS_LOG_ERR("Get factory-test-data-directory from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		strcpy(cts_data->factory_test_data_directory, "/data/vendor/log/tp_fac_test/");
	} else {
		strncpy(cts_data->factory_test_data_directory, str,
			sizeof(cts_data->factory_test_data_directory) - 1);
	}
	TS_LOG_INFO(" - %-32s: %s\n", "Test data directory",
		cts_data->factory_test_data_directory);
}

static void parse_dts_project(const struct device_node *proj_of_node)
{
	TS_LOG_INFO("Parse dts project settings\n");

	parse_dts_project_basic(proj_of_node);

	parse_dts_project_facotry_test(proj_of_node);
}

static int init_pinctrl(void)
{
	int ret = 0;

	if(!cts_data->use_pinctrl) {
		TS_LOG_INFO("Init pinctrl while NOT use\n");
		return 0;
	}

	TS_LOG_INFO("Init pinctrl\n");

	cts_data->pinctrl = devm_pinctrl_get(cts_data->device);
	if (IS_ERR_OR_NULL(cts_data->pinctrl)) {
		ret = PTR_ERR(cts_data->pinctrl);
		TS_LOG_ERR("Get pinctrl failed %d(%s)\n",
			ret, cts_strerror(ret));
		cts_data->pinctrl = NULL;
		return ret;
	}

#ifdef CONFIG_HUAWEI_DEVKIT_MTK_3_0
#define PINCTRL_STATE_RESET_HIGH_NAME "state_rst_output1"
#define PINCTRL_STATE_RESET_LOW_NAME "state_rst_output0"
#define PINCTRL_STATE_INT_HIGH_NAME "state_eint_output1"
#define PINCTRL_STATE_INT_LOW_NAME "state_eint_output0"
#define PINCTRL_STATE_AS_INT_NAME "state_eint_as_int"
#define PINCTRL_STATE_CS_HIGH_NAME "state_cs_output1"
#define PINCTRL_STATE_CS_LOW_NAME "state_cs_output0"

	cts_data->pinctrl_state_reset_high =
		pinctrl_lookup_state(cts_data->pinctrl, PINCTRL_STATE_RESET_HIGH_NAME);
	if (IS_ERR_OR_NULL(cts_data->pinctrl_state_reset_high)) {
		ret = -PTR_ERR(cts_data->pinctrl_state_reset_high);
		TS_LOG_ERR("Lookup pinctrl state '"
			PINCTRL_STATE_RESET_HIGH_NAME"' failed %d(%s)\n",
			ret, cts_strerror(ret));
		goto put_pinctrl;
	}
	cts_data->pinctrl_state_reset_low =
		pinctrl_lookup_state(cts_data->pinctrl, PINCTRL_STATE_RESET_LOW_NAME);
	if (IS_ERR_OR_NULL(cts_data->pinctrl_state_reset_low)) {
		ret = -PTR_ERR(cts_data->pinctrl_state_reset_low);
		TS_LOG_ERR("Lookup pinctrl state '"
			PINCTRL_STATE_RESET_LOW_NAME"' failed %d(%s)\n",
			ret, cts_strerror(ret));
		goto put_pinctrl;
	}
	cts_data->pinctrl_state_as_int =
		pinctrl_lookup_state(cts_data->pinctrl, PINCTRL_STATE_AS_INT_NAME);
	if (IS_ERR_OR_NULL(cts_data->pinctrl_state_as_int)) {
		ret = -PTR_ERR(cts_data->pinctrl_state_as_int);
		TS_LOG_ERR("Lookup pinctrl state '"
			PINCTRL_STATE_AS_INT_NAME"' failed %d(%s)\n",
			ret, cts_strerror(ret));
		goto put_pinctrl;
	}
	cts_data->pinctrl_state_int_high =
		pinctrl_lookup_state(cts_data->pinctrl, PINCTRL_STATE_INT_HIGH_NAME);
	if (IS_ERR_OR_NULL(cts_data->pinctrl_state_int_high)) {
		ret = -PTR_ERR(cts_data->pinctrl_state_int_high);
		TS_LOG_ERR("Lookup pinctrl state '"
			PINCTRL_STATE_INT_HIGH_NAME"' failed %d(%s)\n",
			ret, cts_strerror(ret));
		goto put_pinctrl;
	}
	cts_data->pinctrl_state_int_low =
		pinctrl_lookup_state(cts_data->pinctrl, PINCTRL_STATE_INT_LOW_NAME);
	if (IS_ERR_OR_NULL(cts_data->pinctrl_state_int_low)) {
		ret = -PTR_ERR(cts_data->pinctrl_state_int_low);
		TS_LOG_ERR("Lookup pinctrl state '"
			PINCTRL_STATE_INT_LOW_NAME"' failed %d(%s)\n",
			ret, cts_strerror(ret));
		goto put_pinctrl;
	}

#undef PINCTRL_STATE_RESET_HIGH
#undef PINCTRL_STATE_RESET_LOW
#undef PINCTRL_STATE_INT_HIGH
#undef PINCTRL_STATE_INT_LOW
#undef PINCTRL_STATE_AS_INT

#else /* CONFIG_HUAWEI_DEVKIT_MTK_3_0 */
	cts_data->pinctrl_state_default =
		pinctrl_lookup_state(cts_data->pinctrl, "default");
	if (IS_ERR(cts_data->pinctrl_state_default)) {
		ret = PTR_ERR(cts_data->pinctrl_state_default);
		TS_LOG_ERR("Lookup active pinctrl state failed %d(%s)\n",
			ret, cts_strerror(ret));
		goto put_pinctrl;
	}

	cts_data->pinctrl_state_idle =
		pinctrl_lookup_state(cts_data->pinctrl, "idle");
	if (IS_ERR(cts_data->pinctrl_state_idle)) {
		ret = PTR_ERR(cts_data->pinctrl_state_default);
		TS_LOG_ERR("Lookup suspend pinctrl state failed %d(%s)\n",
			ret, cts_strerror(ret));
		goto put_pinctrl;
	}
#endif /* CONFIG_HUAWEI_DEVKIT_MTK_3_0 */

	return 0;

put_pinctrl:
	if(cts_data->pinctrl) {
		devm_pinctrl_put(cts_data->pinctrl);
		cts_data->pinctrl = NULL;
	}
#ifdef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	cts_data->pinctrl_state_reset_high = NULL;
	cts_data->pinctrl_state_reset_low = NULL;
	cts_data->pinctrl_state_release = NULL;
	cts_data->pinctrl_state_int_high = NULL;
	cts_data->pinctrl_state_int_low = NULL;
	cts_data->pinctrl_state_as_int = NULL;
#else /* CONFIG_HUAWEI_DEVKIT_MTK_3_0 */
	cts_data->pinctrl_state_default = NULL;
	cts_data->pinctrl_state_idle = NULL;
#endif /* CONFIG_HUAWEI_DEVKIT_MTK_3_0 */

	return ret;
}

static int release_pinctrl(void)
{
	if (!cts_data->use_pinctrl) {
		TS_LOG_INFO("Release pinctrl with use_pinctrl = 0\n");
		return 0;
	}

	if (cts_data->pinctrl == NULL) {
		TS_LOG_INFO("Release pinctrl with pinctrl = NULL\n");
		return 0;
	}

	TS_LOG_INFO("Release pinctrl\n");

	devm_pinctrl_put(cts_data->pinctrl);
	cts_data->pinctrl = NULL;
#ifdef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	cts_data->pinctrl_state_reset_high = NULL;
	cts_data->pinctrl_state_int_high = NULL;
	cts_data->pinctrl_state_int_low = NULL;
	cts_data->pinctrl_state_reset_low = NULL;
	cts_data->pinctrl_state_as_int = NULL;
#else  /* CONFIG_HUAWEI_DEVKIT_MTK_3_0 */
	cts_data->pinctrl_state_default = NULL;
	cts_data->pinctrl_state_idle = NULL;
#endif /* CONFIG_HUAWEI_DEVKIT_MTK_3_0 */

	return 0;
}

static int pinctrl_select_default_state(void)
{
	int ret;

	if(!cts_data->use_pinctrl) {
		TS_LOG_INFO("Select default pinctrl state with use_pinctrl = 0\n");
		return 0;
	}

#ifdef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	// TODO:
#else /* CONFIG_HUAWEI_DEVKIT_MTK_3_0 */
	if (IS_ERR_OR_NULL(cts_data->pinctrl) ||
		IS_ERR_OR_NULL(cts_data->pinctrl_state_default)) {
		TS_LOG_ERR("Select default pinctrl state with "
			"pinctrl = NULL or pinctrl_state_default = NULL\n");
		return -EINVAL;
	}

	TS_LOG_INFO("Select default pinctrl state\n");

	ret = pinctrl_select_state(cts_data->pinctrl,
		cts_data->pinctrl_state_default);
	if (ret < 0) {
		TS_LOG_ERR("Select default pinctrl state failed %d(%s)\n",
			ret, cts_strerror(ret));
		return ret;
	}
#endif /* CONFIG_HUAWEI_DEVKIT_MTK_3_0 */

	return 0;
}

static int pinctrl_select_idle_state(void)
{
	int ret;

	if(!cts_data->use_pinctrl) {
		TS_LOG_INFO("Select idle pinctrl state with use_pinctrl = 0\n");
		return -ENODEV;
	}

#ifdef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	// TODO:
#else /* CONFIG_HUAWEI_DEVKIT_MTK_3_0 */
	if (IS_ERR_OR_NULL(cts_data->pinctrl) ||
		IS_ERR_OR_NULL(cts_data->pinctrl_state_idle)) {
		TS_LOG_ERR("Select ilde pinctrl state with "
				   "pinctrl = NULL or pinctrl_state_idle = NULL\n");
		return -EINVAL;
	}

	TS_LOG_INFO("Select default pinctrl state\n");

	ret = pinctrl_select_state(cts_data->pinctrl,
		cts_data->pinctrl_state_idle);
	if (ret < 0) {
		TS_LOG_ERR("Select idle pinctrl state failed %d(%s)\n",
			ret, cts_strerror(ret));
		return ret;
	}
#endif /* CONFIG_HUAWEI_DEVKIT_MTK_3_0 */

	return 0;
}

static int set_reset_gpio_value(int value)
{
	int ret;

#ifdef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	struct pinctrl_state *pinctrl_state_reset;

	TS_LOG_INFO("Set reset pin to %s\n", (!!value) ? "HIGH" : "LOW");

	pinctrl_state_reset = (!!value) ? cts_data->pinctrl_state_reset_high :
		cts_data->pinctrl_state_reset_low;
	if (cts_data->pinctrl == NULL || pinctrl_state_reset == NULL) {
		TS_LOG_ERR("Set reset gpio value with "
			"pinctrl = NULL or reset pinctrl_state = NULL\n");
		return -EINVAL;
	}
	ret = pinctrl_select_state(cts_data->pinctrl, pinctrl_state_reset);
	if (ret) {
		TS_LOG_ERR("Select reset pinctrl state failed %d(%s)\n",
			ret, cts_strerror(ret));
		return ret;
	}

	return 0;
#else /* CONFIG_HUAWEI_DEVKIT_MTK_3_0 */

	TS_LOG_INFO("Set reset pin to %s\n", (!!value) ? "HIGH" : "LOW");

	if(cts_data->tskit_data->ts_platform_data->reset_gpio <= 0) {
		TS_LOG_INFO("Set reset gpio value with reset gpio %d\n",
			cts_data->tskit_data->ts_platform_data->reset_gpio);
		return -EFAULT;
	}

	ret = gpio_direction_output(cts_data->tskit_data->ts_platform_data->reset_gpio,
		(!!value) ? 1 : 0);
	if (ret) {
		TS_LOG_ERR("Set reset gpio value use "
				   "gpio_direction_output() failed %d(%s)\n",
			ret, cts_strerror(ret));
		return ret;
	}

	return 0;
#endif /* CONFIG_HUAWEI_DEVKIT_MTK_3_0 */
}

static int do_hw_reset(const u8 *hw_reset_timing)
{
	struct ts_kit_platform_data *tskit_pdata = NULL;
	int ret = NO_ERR;

	tskit_pdata = cts_data->tskit_data->ts_platform_data;

	TS_LOG_INFO("Do harware reset, timing: [H:%u, L:%u, H:%u]\n",
		hw_reset_timing[0], hw_reset_timing[1], hw_reset_timing[2]);

	ret = set_reset_gpio_value(1);
	if (ret) {
		TS_LOG_ERR("Set reset gpio to HIGH failed %d(%s)\n",
			ret, cts_strerror(ret));
		return ret;
	}
	mdelay(hw_reset_timing[0]);

	ret = set_reset_gpio_value(0);
	if (ret) {
		TS_LOG_ERR("Set reset gpio to LOW failed %d(%s)\n",
			ret, cts_strerror(ret));
		return ret;
	}
	mdelay(hw_reset_timing[1]);

	ret = set_reset_gpio_value(1);
	if (ret) {
		TS_LOG_ERR("Set reset gpio to HIGH failed %d(%s)\n",
			ret, cts_strerror(ret));
		return ret;
	}
	mdelay(hw_reset_timing[2]);

	return 0;
}

static int chipone_chip_init(void)
{
	int ret = NO_ERR;

	if (cts_data == NULL) {
		TS_LOG_ERR("Chip init with cts_data = NULL\n");
		return -EINVAL;
	}

	TS_LOG_INFO("Chip init\n");

	ret = cts_tool_init(cts_data);
	if (ret < 0) {
		TS_LOG_ERR("Init tool node failed %d(%s)\n",
			ret, cts_strerror(ret));
	}

	ret = cts_sysfs_add_device(cts_data->device);
	if (ret < 0) {
		TS_LOG_ERR("Add device sysfs attr groups failed %d(%s)\n",
			ret, cts_strerror(ret));
	}

#ifdef CONFIG_CTS_CHARGER_DETECT
	if (!cts_data->use_huawei_charger_detect) {
		ret = cts_init_charger_detect(cts_data);
		if (ret) {
			TS_LOG_ERR("Init charger detect failed %d(%s)\n",
				ret, cts_strerror(ret));
		}
	}
#endif /* CONFIG_CTS_CHARGER_DETECT */

#ifdef CONFIG_CTS_EARJACK_DETECT
	ret = cts_init_earjack_detect(cts_data);
	if (ret) {
		TS_LOG_ERR("Init earjack detect failed %d(%s)\n",
			ret, cts_strerror(ret));
	}
#endif /* CONFIG_CTS_EARJACK_DETECT */

	return 0;
}

static int chipone_parse_config(struct device_node *of_node,
	struct ts_kit_device_data *data)
{
	struct device_node *proj_of_node;
	int ret;
	struct lcd_kit_ops *tp_ops = lcd_kit_get_ops();

	if(of_node == NULL || data == NULL) {
		TS_LOG_ERR("Parse dts config with of_node/ts_kit_device_data = NULL\n");
		return 0;
	}

	if (cts_data == NULL) {
		TS_LOG_ERR("Parse dts config with cts_data = NULL\n");
		return -EINVAL;
	}

	TS_LOG_INFO("Parse dts config\n");

	cts_data->use_pinctrl = of_property_read_bool(of_node, "use-pinctrl");
	TS_LOG_INFO(" - %-32s: %c\n", "Use pinctrl",
		cts_data->use_pinctrl ? 'Y' : 'N');

	cts_data->use_huawei_charger_detect =
		of_property_read_bool(of_node, "use-huawei-charger-detect");
	TS_LOG_INFO(" - %-32s: %c\n", "Use huawei charger detect",
		cts_data->use_huawei_charger_detect ? 'Y' : 'N');

	ret = of_property_read_u32(of_node, "is_ic_rawdata_proc_printf",
		(u32 *)&data->is_ic_rawdata_proc_printf);
	if (ret) {
		TS_LOG_ERR("Get is_ic_rawdata_proc_printf from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		data->is_ic_rawdata_proc_printf = 1;
	}
	TS_LOG_INFO(" - %-32s: %c\n", "IC rawdata proc printf",
		data->is_ic_rawdata_proc_printf ? 'Y' : 'N');

	ret = of_property_read_u32(of_node, "hide_fw_name",
		(u32 *)&cts_data->hide_fw_name);
	if (ret) {
		TS_LOG_ERR("Get hide_fw_name from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		cts_data->hide_fw_name = 0;
	}
	TS_LOG_INFO(" - %-32s: %c\n", "Hide firmware name",
		cts_data->hide_fw_name ? 'Y' : 'N');

	ret = of_property_read_string(of_node, "tp_test_type",
		&cts_data->tskit_data->tp_test_type);
	if (ret) {
		TS_LOG_INFO("tp_test_type not configed, use default\n");
		/* 17 for 0f 4f */
		cts_data->tskit_data->tp_test_type =
			"Normalize_type:judge_different_reslut:17";
	}
	TS_LOG_INFO(" - %-32s: %s\n", "Tp_test_type",
		cts_data->tskit_data->tp_test_type);

	ret = of_property_read_u32(of_node, "parade,is_parade_solution",
		(u32 *)&data->is_parade_solution);
	if (ret) {
		TS_LOG_ERR("Get parade,is_parade_solution from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		data->is_parade_solution = 0;
	}
	TS_LOG_INFO(" - %-32s: %c\n", "Parade solution",
		data->is_parade_solution ? 'Y' : 'N');

	ret = of_property_read_u32(of_node, "parade,direct_proc_cmd",
		(u32 *)&data->is_direct_proc_cmd);
	if (ret) {
		TS_LOG_ERR("Get parade,direct_proc_cmd from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		data->is_direct_proc_cmd = 0;
	}
	TS_LOG_INFO(" - %-32s: %c\n", "Parade direct proc cmd",
		data->is_direct_proc_cmd ? 'Y' : 'N');

	ret = of_property_read_u32(of_node, "is_i2c_one_byte",
		(u32 *)&data->is_i2c_one_byte);
	if (ret) {
		TS_LOG_ERR("Get is_i2c_one_byte from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		data->is_i2c_one_byte = 0;
	}
	TS_LOG_INFO(" - %-32s: %c\n", "I2C one byte",
		data->is_i2c_one_byte ? 'Y' : 'N');

	ret = of_property_read_u32(of_node, "rawdata_newformatflag",
		(u32 *)&data->rawdata_newformatflag);
	if (ret) {
		TS_LOG_ERR("Get rawdata_newformatflag from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		data->rawdata_newformatflag = 0;
	}
	TS_LOG_INFO(" - %-32s: %c\n", "Rawdata new format",
		data->rawdata_newformatflag ? 'Y' : 'N');

	ret = of_property_read_u32(of_node, "is_ic_rawdata_proc_printf",
		(u32 *)&data->is_ic_rawdata_proc_printf);
	if (ret) {
		TS_LOG_ERR("Get is_ic_rawdata_proc_printf from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		data->is_ic_rawdata_proc_printf = 1;
	}
	TS_LOG_INFO(" - %-32s: %c\n", "IC rawdata proc printf",
		data->is_ic_rawdata_proc_printf ? 'Y' : 'N');

	ret = of_property_read_u8(of_node, "roi_supported",
		(u8 *)&data->ts_platform_data->feature_info.roi_info.roi_supported);
	if (ret) {
		TS_LOG_ERR("Get roi_supported from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		data->ts_platform_data->feature_info.roi_info.roi_supported = 1;
	}
	TS_LOG_INFO(" - %-32s: %c\n", "ROI supported",
 		data->ts_platform_data->feature_info.roi_info.roi_supported ? 'Y' : 'N');

	ret = of_property_read_u32(of_node, "touch_switch_flag",
		(u32 *)&data->touch_switch_flag);
	if (ret) {
		TS_LOG_ERR("Get touch_switch_flag from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
 		data->touch_switch_flag = 0;
 	}
	TS_LOG_INFO(" - %-32s: %c\n", "Touch switch flag",
		data->touch_switch_flag ? 'Y' : 'N');

	ret = of_property_read_u32(of_node, "get_brightness_info_flag",
		(u32 *)&data->get_brightness_info_flag);
	if (ret) {
  		TS_LOG_ERR("Get get_brightness_info_flag from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		data->get_brightness_info_flag = 0;
	}
	TS_LOG_INFO(" - %-32s: %c\n", "Get brightness info",
		data->get_brightness_info_flag ? 'Y' : 'N');

	ret = of_property_read_u32(of_node, "need_wd_check_status",
		(u32 *)&data->need_wd_check_status);
	if (ret) {
		TS_LOG_ERR("Get need_wd_check_status from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		data->need_wd_check_status = 0;
	}
	TS_LOG_INFO(" - %-32s: %c\n", "Need WDT check status",
		data->need_wd_check_status ? 'Y' : 'N');

	ret = of_property_read_u32(of_node, "check_status_watchdog_timeout",
		(u32 *)&data->check_status_watchdog_timeout);
	if (ret) {
		TS_LOG_ERR("Get check_status_watchdog_timeout from dts failed %d(%s)\n",
			ret, cts_strerror(ret));
		data->check_status_watchdog_timeout = 0;
	}
	TS_LOG_INFO(" - %-32s: %u\n", "WDT check status timeout",
		data->check_status_watchdog_timeout);

	parse_dts_input_config(of_node);

	/* Project settings */
	if (tp_ops && tp_ops->get_project_id) {
		ret = tp_ops->get_project_id(cts_data->project_id);
	} else {
		TS_LOG_ERR("kit_read_projectid_spi:get lcd_kit_get_ops fail\n");
		cts_data->project_id[0] = '\0';
		ret = -EINVAL;
	}

	if (cts_data->project_id[0] == '\0') {
		const char *default_project_id = NULL;
		ret = of_property_read_string(of_node, "default_project_id",
			&default_project_id);
		if (ret) {
			TS_LOG_ERR("Get default_project_id from dts failed %d(%s)\n",
				ret, cts_strerror(ret));
			strncpy(cts_data->project_id, "TS",
				sizeof("TS") - 1);
		} else {
			int len = strlen(default_project_id);
			if (len > (sizeof(cts_data->project_id) - 1)) {
				strncpy(cts_data->project_id, default_project_id,
					sizeof(cts_data->project_id) - 1);
			}
		}
	}
	TS_LOG_INFO(" - %-32s: %s\n", "Project id", cts_data->project_id);
	strncpy(data->project_id, cts_data->project_id,
		sizeof(cts_data->project_id) - 1);
	proj_of_node = of_find_node_by_name(of_node, cts_data->project_id);
	if (proj_of_node == NULL) {
		TS_LOG_ERR("Get project '%s' node failed\n", cts_data->project_id);
		ret = -ENOENT;
		goto out;
	}

	parse_dts_project(proj_of_node);

	of_node_put(proj_of_node);

out:
	return ret;
}

static int chipone_chip_detect(struct ts_kit_platform_data *data)
{
	int ret = NO_ERR;

	if (data == NULL) {
		TS_LOG_ERR("Detect chip with ts_kit_platform_data = NULL\n");
		return -EINVAL;
	}

	if (cts_data == NULL) {
		TS_LOG_ERR("Detect chip with cts_data = NULL\n");
		return -EINVAL;
	}

	TS_LOG_INFO("Chip detect\n");
	cts_data->tskit_data->ts_platform_data = data;
	cts_data->device = &data->ts_dev->dev;

	if (dev_get_drvdata(cts_data->device)) {
		TS_LOG_ERR("cts_data->device is not null\n");
		return -EBUSY;
	}
	dev_set_drvdata(cts_data->device, cts_data);

	chipone_parse_config(data->chip_data->cnode, cts_data->tskit_data);

	init_pinctrl();
	pinctrl_select_default_state();

	do_hw_reset(cts_data->boot_hw_reset_timing);

	data->spi->mode = cts_data->spi_mode;
	data->spi->max_speed_hz = cts_data->spi_max_speed;
	ret = spi_setup(data->spi);
	if (ret)
		TS_LOG_ERR("Setup spi failed %d(%s)\n", ret, cts_strerror(ret));

	ret = cts_probe_device(&cts_data->cts_dev);
	if (ret) {
		TS_LOG_ERR("Probe device failed %d(%s)\n",
			ret, cts_strerror(ret));
		goto out;
	} else {
		strncpy(cts_data->tskit_data->chip_name,
			"AC", strlen("AC"));
	}

	cts_data->cts_dev.pdata = cts_data->pdata;
	return 0;

out:
	TS_LOG_INFO("Chip detect returns %d\n", ret);
	return ret;
}

static int chipone_input_config(struct input_dev *input_dev)
{
	int ret = NO_ERR;

	if (cts_data == NULL) {
		TS_LOG_ERR("Config input device with cts_data = NULL\n");
		return -EFAULT;
	}

	if (input_dev == NULL) {
		TS_LOG_ERR("Config input device with input_dev = NULL\n");
		return -EINVAL;
	}

	TS_LOG_INFO("Config input device, X: %u, Y: %u, N: %u\n",
		cts_data->tskit_data->x_max, cts_data->tskit_data->y_max,
		cts_data->tskit_data->ts_platform_data->max_fingers);

	input_dev->evbit[0] = BIT_MASK(EV_SYN) |
		BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	input_set_capability(input_dev, EV_KEY, BTN_TOUCH);
	__set_bit(INPUT_PROP_DIRECT, input_dev->propbit);

	input_set_abs_params(input_dev, ABS_X,
		0, (cts_data->tskit_data->x_max - 1), 0, 0);
	input_set_abs_params(input_dev, ABS_Y,
		0, (cts_data->tskit_data->y_max - 1), 0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE, 0, 255, 0, 0);

	input_set_abs_params(input_dev, ABS_MT_POSITION_X,
		0, (cts_data->tskit_data->x_max - 1), 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y,
		0, (cts_data->tskit_data->y_max - 1), 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, 255, 0, 0);

	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID,
		0, cts_data->tskit_data->ts_platform_data->max_fingers * 2,
		0, 0);

#ifdef CONFIG_CTS_GESTURE
	input_set_capability(input_dev, EV_KEY, TS_DOUBLE_CLICK);
#endif /* CONFIG_CTS_GESTURE */

#ifdef TYPE_B_PROTOCOL
	input_mt_init_slots(input_dev,
		cts_data->tskit_data->ts_platform_data->max_fingers,
		INPUT_MT_DIRECT);
#endif /* TYPE_B_PROTOCOL */

	return ret;
}

static int chipone_chip_get_info(struct ts_chip_info_param *info)
{
	struct ts_kit_device_data *tskit_dev_data = cts_data->tskit_data;

	if (IS_ERR_OR_NULL(info)) {
		TS_LOG_ERR("info invaild\n");
		return -EINVAL;
	}

	if (tskit_dev_data->ts_platform_data->hide_plain_id)
		snprintf(info->ic_vendor, sizeof(info->ic_vendor) - 1,
			"%s", cts_data->project_id);
	else
		snprintf(info->ic_vendor, sizeof(info->ic_vendor) - 1, "%s-%s",
			tskit_dev_data->chip_name, cts_data->project_id);

	snprintf(info->mod_vendor, sizeof(info->mod_vendor) - 1, "%s",
		tskit_dev_data->module_name);
	snprintf(info->fw_vendor, sizeof(info->fw_vendor) - 1,
		"0x%04x", cts_data->cts_dev.fwdata.version);

	return 0;
}

static int chipone_irq_top_half(struct ts_cmd_node * cmd)
{
	TS_LOG_DEBUG("Irq top half\n");

	cmd->command = TS_INT_PROCESS;

	return 0;
}

static int process_touch_msg(struct ts_fingers *info,
	struct cts_device_touch_msg *msgs, int num)
{
	int i;

	TS_LOG_DEBUG("Process touch %d msgs\n", num);

	memset(info, 0, sizeof(*info));

	for (i = 0; i < num; i++) {
		u16 x, y;

		x = le16_to_cpu(msgs[i].x);
		y = le16_to_cpu(msgs[i].y);

		/* ts_finger_events_reformat() will OR
		 * TS_FINGER_PRESS/TS_FINGER_RELEASE to status.
		 */
		switch (msgs[i].event) {
		case CTS_DEVICE_TOUCH_EVENT_DOWN:
		case CTS_DEVICE_TOUCH_EVENT_MOVE:
		case CTS_DEVICE_TOUCH_EVENT_STAY:
			info->fingers[msgs[i].id].x = x;
			info->fingers[msgs[i].id].y = y;
			info->fingers[msgs[i].id].major = 1;
			info->fingers[msgs[i].id].minor = 1;
			info->fingers[msgs[i].id].pressure = msgs[i].pressure;
			info->fingers[msgs[i].id].status = TP_FINGER;
			info->cur_finger_number++;
			break;
		case CTS_DEVICE_TOUCH_EVENT_UP:
			info->fingers[msgs[i].id].status = TP_NONE_FINGER;
			break;

		default:
			TS_LOG_ERR("Process touch msg with unknown event %u id %u\n",
				msgs[i].event, msgs[i].id);
			break;
		}
	}

	return 0;
}

#ifdef CFG_CTS_GESTURE
int process_gesture_info(struct ts_fingers *info,
	struct cts_device_gesture_info *gesture_info)
{
	TS_LOG_INFO("Process gesture info, ID: 0x%02x\n",
		gesture_info->gesture_id);

	switch (gesture_info->gesture_id) {
	case GESTURE_D_TAP:
		mutex_lock(&cts_data->wrong_touch_lock);
		if (cts_data->tskit_data->easy_wakeup_info.off_motion_on) {
			cts_data->tskit_data->easy_wakeup_info.off_motion_on = false;
			info->gesture_wakeup_value = TS_DOUBLE_CLICK;
		}
		mutex_unlock(&cts_data->wrong_touch_lock);
		break;

	default:
		TS_LOG_ERR("Get unknown gesture code 0x%02x\n",
			gesture_info->gesture_id);
		break;
	}
	return 0;
}
#endif /* CFG_CTS_GESTURE */

static int chipone_irq_bottom_half(struct ts_cmd_node *in_cmd,
	struct ts_cmd_node * out_cmd)
{
	struct ts_fingers *info = NULL;
	struct cts_device *cts_dev = &cts_data->cts_dev;
	int ret;

	if (cts_data == NULL) {
		TS_LOG_ERR("Irq bottom half with cts_data = NULL\n");
		return -EINVAL;
	}

	if (out_cmd == NULL) {
		TS_LOG_ERR("Irq bottom half with out_cmd = NULL\n");
		return -EINVAL;
	}

	if (cts_dev->rtdata.program_mode) {
		TS_LOG_ERR("IRQ triggered in program mode\n");
		return -EINVAL;
	}

	TS_LOG_DEBUG("Irq bottom half\n");

	out_cmd->command = TS_INPUT_ALGO;
	out_cmd->cmd_param.pub_params.algo_param.algo_order =
		cts_data->tskit_data->algo_id;
	info = &out_cmd->cmd_param.pub_params.algo_param.info;
	memset(info, 0, sizeof(*info));

	if (unlikely(cts_dev->rtdata.suspended)) {
#ifdef CFG_CTS_GESTURE
		if (cts_dev->rtdata.gesture_wakeup_enabled) {
			struct cts_device_gesture_info gesture_info;

			TS_LOG_INFO("Get gesture info\n");
			ret = cts_get_gesture_info(cts_dev,
					&gesture_info, CFG_CTS_GESTURE_REPORT_TRACE);
			if (ret)
				TS_LOG_ERR("Get gesture info failed %d(%s)\n",
					ret, cts_strerror(ret));

			/** - Issure another suspend with gesture wakeup
			 * command to device after get gesture info.
			 */
			TS_LOG_INFO("Set device enter gesture mode\n");
			cts_send_command(cts_dev, CTS_CMD_SUSPEND_WITH_GESTURE);

			ret = process_gesture_info(info, &gesture_info);
			if (ret) {
				TS_LOG_ERR("Process gesture info failed %d(%s)\n",
					ret, cts_strerror(ret));
				return ret;
			}
		} else {
			TS_LOG_ERR("IRQ triggered while device suspended "
					"without gesture wakeup enable\n");
		}
#endif /* CFG_CTS_GESTURE */
	} else {
		struct cts_device_touch_info *touch_info;

		touch_info = &cts_data->touch_info;

		ret = cts_get_touchinfo(cts_dev, touch_info);
		if (ret) {
			TS_LOG_ERR("Get touch info failed %d\n", ret);
			return ret;
		}

		ret = process_touch_msg(info,
			touch_info->msgs, touch_info->num_msg);
		if (ret) {
			TS_LOG_ERR("Process touch msg failed %d\n", ret);
			return ret;
		}

		if (cts_data->tskit_data->ts_platform_data->feature_info.roi_info.roi_supported &&
			(!!cts_data->roi_switch) &&
			info->cur_finger_number > 0 && info->cur_finger_number <= 2) {
			ret = cts_fw_reg_readsb(cts_dev, CTS_DEVICE_FW_REG_NOI_DATA,
				cts_data->roi_data + 4, 7 * 7 * 2);
			if (ret) {
				TS_LOG_ERR("Get ROI data failed %d(%s)\n",
					ret, cts_strerror(ret));
				return ret;
			}
		}
	}

	return 0;
}

static int chipone_chip_reset(void)
{
	if (cts_data == NULL) {
		TS_LOG_ERR("Chipe reset with cts_data = NULL\n");
		return -EINVAL;
	}

	TS_LOG_INFO("Chip reset\n");

	return do_hw_reset(cts_data->resume_hw_reset_timing);
}

static void chipone_chip_shutdown(void)
{
	if (cts_data == NULL) {
		TS_LOG_ERR("Chip shutdown with cts_data = NULL\n");
		return ;
	}

	TS_LOG_INFO("Chip shutdown\n");

	pinctrl_select_idle_state();
}

static int get_boot_firmware_filename(void)
{
	struct ts_kit_device_data *tskit_dev_data = cts_data->tskit_data;
	int ret = 0;

	if (cts_data->hide_fw_name) {
		ret = snprintf(cts_data->boot_firmware_filename,
			sizeof(cts_data->boot_firmware_filename), "ts/%s.img",
			cts_data->project_id);
	} else {
		if (tskit_dev_data->ts_platform_data->product_name[0] == '\0' ||
			tskit_dev_data->chip_name[0] == '\0' ||
			cts_data->project_id[0] == '\0' ||
			tskit_dev_data->module_name[0] == '\0') {
			TS_LOG_ERR("Get boot firmware filename with "
				"product_name/chip_name/project_id/module_name = nul\n");
			return -EINVAL;
		}
		ret = snprintf(cts_data->boot_firmware_filename,
			sizeof(cts_data->boot_firmware_filename),
			"ts/%s_%s_%s_%s.img",
			tskit_dev_data->ts_platform_data->product_name,
			tskit_dev_data->chip_name,
			cts_data->project_id,
			tskit_dev_data->module_name);
	}

	if (ret >= sizeof(cts_data->boot_firmware_filename)) {
		TS_LOG_ERR("Get boot firmware filename length %d too large\n", ret);
		return -ENOMEM;
	}

	TS_LOG_INFO("Boot firmware filename: '%s'\n",
		cts_data->boot_firmware_filename);

	return 0;
}

static int chipone_fw_update_boot(char *file_name)
{
	int ret;

	if (cts_data == NULL) {
		TS_LOG_ERR("Firmware update boot with cts_data = NULL\n");
		return -EINVAL;
	}

	TS_LOG_INFO("Firmware update boot\n");

	ret = get_boot_firmware_filename();
	if (ret) {
		TS_LOG_ERR("Get boot firmware filename failed %d(%s)\n",
			ret, cts_strerror(ret));
		return ret;
	}

	cts_data->cached_firmware = cts_request_firmware_from_fs(&cts_data->cts_dev,
		cts_data->boot_firmware_filename);
	if(cts_data->cached_firmware == NULL) {
		TS_LOG_ERR("Request firmware from file '%s' failed",
			cts_data->boot_firmware_filename);
		return -EIO;
	}

	ret = cts_update_firmware(&cts_data->cts_dev,
		cts_data->cached_firmware, false);

	if (ret) {
		TS_LOG_ERR("Update boot firmware failed %d(%s)\n",
			ret, cts_strerror(ret));
		return ret;
	}

	return 0;
}

static int chipone_fw_update_sd(void)
{
	int ret;

	if (cts_data == NULL) {
		TS_LOG_ERR("Firmware update manual with cts_data = NULL\n");
		return -EINVAL;
	}

	TS_LOG_INFO("Firmware update manual\n");

	ret = cts_update_firmware_from_file(&cts_data->cts_dev,
		"/odm/etc/firmware/ts/touch_screen_firmware.img", false);
	if (ret) {
		TS_LOG_ERR("Update firmware manual failed %d(%s)\n",
			ret, cts_strerror(ret));
		return ret;
	}

	return 0;
}

static int alloc_test_data_mem(struct cts_test_param *test_param,
	int test_data_buf_size)
{
	const char *test_item_str = cts_test_item_str(test_param->test_item);

	if (test_param->test_data_buf) {
		TS_LOG_INFO("Alloc %s data mem with test_data_buf != NULL\n",
			test_item_str);
		/* Reuse previous memory */
		return 0;
	}

	TS_LOG_INFO("* Alloc %s data mem, size: %d\n",
		test_item_str, test_data_buf_size);

	test_param->test_data_buf = vzalloc(test_data_buf_size);
	if (test_param->test_data_buf == NULL) {
		TS_LOG_ERR("Alloc %s data mem failed\n", test_item_str);
		test_param->test_data_buf_size = 0;
		return -ENOMEM;
	}
	test_param->test_data_buf_size = test_data_buf_size;

	return 0;
}

static int alloc_test_data_filepath(struct cts_test_param *test_param,
	const char *test_data_filename)
{
	const char *test_item_str = cts_test_item_str(test_param->test_item);
	char touch_data_filepath[512];
	int ret;

	if (test_param->test_data_filepath) {
		TS_LOG_ERR("Alloc %s data filepath with test_data_filepath != NULL\n",
			test_item_str);
		kfree(test_param->test_data_filepath);
	}

	ret = snprintf(touch_data_filepath, sizeof(touch_data_filepath),
		"%s%s",
		cts_data->factory_test_data_directory, test_data_filename);
	if (ret >= sizeof(touch_data_filepath)) {
		TS_LOG_ERR("Alloc %s data filepath too long %d\n",
			test_item_str, ret);
		return -EINVAL;
	}

	TS_LOG_INFO("* Alloc %s data filepath: '%s'\n",
		test_item_str, touch_data_filepath);

	test_param->test_data_filepath = kstrdup(touch_data_filepath, GFP_KERNEL);
	if (test_param->test_data_filepath == NULL) {
		TS_LOG_ERR("Alloc %s data filepath failed\n", test_item_str);
		return -ENOMEM;
	}

	return 0;
}

static int init_factory_test_param(void)
{
	struct cts_test_param *test_param;
	u8  rows, cols;
	int nodes;
	int ret;

	TS_LOG_INFO("Init factory test param\n");

	rows = cts_data->cts_dev.fwdata.rows;
	cols = cts_data->cts_dev.fwdata.cols;
	nodes = rows * cols;

	if (cts_data->test_reset_pin) {
		test_param = &cts_data->reset_pin_test_param;
		memset(test_param, 0 , sizeof(*test_param));
		test_param->test_item = CTS_TEST_RESET_PIN;
		test_param->test_result = &cts_data->reset_pin_test_result;
	}
	if (cts_data->test_int_pin) {
		test_param = &cts_data->int_pin_test_param;
		memset(test_param, 0 , sizeof(*test_param));
		test_param->test_item = CTS_TEST_INT_PIN;
		test_param->test_result = &cts_data->int_pin_test_result;
	}
	if (cts_data->test_rawdata) {
		test_param = &cts_data->rawdata_test_param;
		memset(test_param, 0 , sizeof(*test_param));
		test_param->test_item = CTS_TEST_RAWDATA;
		test_param->flags =
			CTS_TEST_FLAG_VALIDATE_DATA |
			CTS_TEST_FLAG_VALIDATE_MIN |
			CTS_TEST_FLAG_VALIDATE_MAX |
			CTS_TEST_FLAG_STOP_TEST_IF_VALIDATE_FAILED;

		if (cts_data->dump_factory_test_data_to_console) {
			test_param->flags |= CTS_TEST_FLAG_DUMP_TEST_DATA_TO_CONSOLE;
		}
		if (cts_data->dump_factory_test_data_to_buffer) {
			test_param->flags |= CTS_TEST_FLAG_DUMP_TEST_DATA_TO_USERSPACE;
			ret = alloc_test_data_mem(test_param,
				2 * nodes * cts_data->rawdata_test_priv_param.frames);
			if (ret) {
				return ret;
			}
			test_param->test_data_wr_size = &cts_data->rawdata_test_data_size;
		}
		if (cts_data->dump_factory_test_data_to_file) {
			test_param->flags |= CTS_TEST_FLAG_DUMP_TEST_DATA_TO_FILE;
			ret = alloc_test_data_filepath(test_param,
				"rawdata-test-data.txt");
			if (ret) {
				return ret;
			}
		}

		test_param->min = &cts_data->rawdata_test_min;
		test_param->max = &cts_data->rawdata_test_max;
		test_param->test_result = &cts_data->rawdata_test_result;

		test_param->priv_param = &cts_data->rawdata_test_priv_param;
		test_param->priv_param_size = sizeof(cts_data->rawdata_test_priv_param);
	}
	if (cts_data->test_noise) {
		test_param = &cts_data->noise_test_param;
		memset(test_param, 0 , sizeof(*test_param));
		test_param->test_item = CTS_TEST_NOISE;
		test_param->flags =
			CTS_TEST_FLAG_VALIDATE_DATA |
			CTS_TEST_FLAG_VALIDATE_MAX |
			CTS_TEST_FLAG_STOP_TEST_IF_VALIDATE_FAILED;

		if (cts_data->dump_factory_test_data_to_console) {
			test_param->flags |= CTS_TEST_FLAG_DUMP_TEST_DATA_TO_CONSOLE;
		}
		if (cts_data->dump_factory_test_data_to_buffer) {
			test_param->flags |= CTS_TEST_FLAG_DUMP_TEST_DATA_TO_USERSPACE;
			ret = alloc_test_data_mem(test_param,
				2 * nodes * (cts_data->noise_test_priv_param.frames + 3));
			if (ret) {
				return ret;
			}
			test_param->test_data_wr_size = &cts_data->noise_test_data_size;
		}
		if (cts_data->dump_factory_test_data_to_file) {
			test_param->flags |= CTS_TEST_FLAG_DUMP_TEST_DATA_TO_FILE;
			ret = alloc_test_data_filepath(test_param,
				"noise-test-data.txt");
			if (ret) {
				return ret;
			}
		}

		test_param->max = &cts_data->noise_test_max;
		test_param->test_result = &cts_data->noise_test_result;

		test_param->priv_param = &cts_data->noise_test_priv_param;
		test_param->priv_param_size = sizeof(cts_data->noise_test_priv_param);
	}
	if (cts_data->test_open) {
		test_param = &cts_data->open_test_param;
		memset(test_param, 0 , sizeof(*test_param));
		test_param->test_item = CTS_TEST_RAWDATA;
		test_param->flags =
			CTS_TEST_FLAG_VALIDATE_DATA |
			CTS_TEST_FLAG_VALIDATE_MIN |
			CTS_TEST_FLAG_STOP_TEST_IF_VALIDATE_FAILED;

		if (cts_data->dump_factory_test_data_to_console) {
			test_param->flags |= CTS_TEST_FLAG_DUMP_TEST_DATA_TO_CONSOLE;
		}
		if (cts_data->dump_factory_test_data_to_buffer) {
			test_param->flags |= CTS_TEST_FLAG_DUMP_TEST_DATA_TO_USERSPACE;
			ret = alloc_test_data_mem(test_param, 2 * nodes * 1);
			if (ret) {
				return ret;
			}
			test_param->test_data_wr_size = &cts_data->open_test_data_size;
		}
		if (cts_data->dump_factory_test_data_to_file) {
			test_param->flags |= CTS_TEST_FLAG_DUMP_TEST_DATA_TO_FILE;
			ret = alloc_test_data_filepath(test_param,
				"open-test-data.txt");
			if (ret) {
				return ret;
			}
		}

		test_param->min = &cts_data->open_test_min;
		test_param->test_result = &cts_data->open_test_result;
	}
	if (cts_data->test_short) {
		test_param = &cts_data->short_test_param;
		memset(test_param, 0 , sizeof(*test_param));
		test_param->test_item = CTS_TEST_SHORT;
		test_param->flags =
			CTS_TEST_FLAG_VALIDATE_DATA |
			CTS_TEST_FLAG_VALIDATE_MIN |
			CTS_TEST_FLAG_STOP_TEST_IF_VALIDATE_FAILED;

		if (cts_data->dump_factory_test_data_to_console) {
			test_param->flags |= CTS_TEST_FLAG_DUMP_TEST_DATA_TO_CONSOLE;
		}
		if (cts_data->dump_factory_test_data_to_buffer) {
			test_param->flags |= CTS_TEST_FLAG_DUMP_TEST_DATA_TO_USERSPACE;
			ret = alloc_test_data_mem(test_param, 2 * nodes * 7);
			if (ret) {
				return ret;
			}
			test_param->test_data_wr_size = &cts_data->short_test_data_size;
		}
		if (cts_data->dump_factory_test_data_to_file) {
			test_param->flags |= CTS_TEST_FLAG_DUMP_TEST_DATA_TO_FILE;
			ret = alloc_test_data_filepath(test_param,
				"short-test-data.txt");
			if (ret) {
				return ret;
			}
		}

		test_param->min = &cts_data->short_test_min;
		test_param->test_result = &cts_data->short_test_result;
	}
	if (cts_data->test_compensate_cap) {
		test_param = &cts_data->comp_cap_test_param;
		memset(test_param, 0 , sizeof(*test_param));
		test_param->test_item = CTS_TEST_COMPENSATE_CAP;
		test_param->flags =
			CTS_TEST_FLAG_VALIDATE_DATA |
			CTS_TEST_FLAG_VALIDATE_MIN |
			CTS_TEST_FLAG_VALIDATE_MAX |
			CTS_TEST_FLAG_STOP_TEST_IF_VALIDATE_FAILED;

		if (cts_data->dump_factory_test_data_to_console) {
			test_param->flags |= CTS_TEST_FLAG_DUMP_TEST_DATA_TO_CONSOLE;
		}
		if (cts_data->dump_factory_test_data_to_buffer) {
			test_param->flags |= CTS_TEST_FLAG_DUMP_TEST_DATA_TO_USERSPACE;
			ret = alloc_test_data_mem(test_param, 1 * nodes * 1);
			if (ret) {
				return ret;
			}
			test_param->test_data_wr_size = &cts_data->comp_cap_test_data_size;
		}
		if (cts_data->dump_factory_test_data_to_file) {
			test_param->flags |= CTS_TEST_FLAG_DUMP_TEST_DATA_TO_FILE;
			ret = alloc_test_data_filepath(test_param,
				"comp-cap-test-data.txt");
			if (ret) {
				return ret;
			}
		}

		test_param->min = &cts_data->compensate_cap_test_min;
		test_param->max = &cts_data->compensate_cap_test_max;
		test_param->test_result = &cts_data->comp_cap_test_result;
	}

	return 0;
}

static void free_test_param_mem(struct cts_test_param *test_param)
{
	if (test_param->test_data_buf) {
		TS_LOG_INFO("* Free %s data mem\n",
			cts_test_item_str(test_param->test_item));
		vfree(test_param->test_data_buf);
		test_param->test_data_buf = NULL;
	}
	if (test_param->test_data_filepath) {
		TS_LOG_INFO("* Free %s data filepath\n",
			cts_test_item_str(test_param->test_item));
		kfree(test_param->test_data_filepath);
		test_param->test_data_filepath = NULL;
	}
}

static void free_factory_test_mem(void)
{
	TS_LOG_INFO("Free factory test data mem\n");

	free_test_param_mem(&cts_data->rawdata_test_param);
	free_test_param_mem(&cts_data->noise_test_param);
	free_test_param_mem(&cts_data->open_test_param);
	free_test_param_mem(&cts_data->short_test_param);
	free_test_param_mem(&cts_data->comp_cap_test_param);
}

static void print_test_result_to_console(const char *test_desc, int test_result)
{
	if (test_result > 0) {
		TS_LOG_INFO("%-10s test: FAIL %d nodes\n", test_desc,
			test_result);
	} else if (test_result < 0) {
		TS_LOG_INFO("%-10s test: FAIL %d(%s)\n", test_desc,
			test_result, cts_strerror(test_result));
	} else {
		TS_LOG_INFO("%-10s test: PASS\n", test_desc);
	}
}

static void print_factory_test_result_to_console(void)
{
	if (cts_data->test_reset_pin) {
		print_test_result_to_console("Reset-Pin", cts_data->reset_pin_test_result);
	}
	if (cts_data->test_int_pin) {
		print_test_result_to_console("Int-Pin", cts_data->int_pin_test_result);
	}
	if (cts_data->test_rawdata) {
		print_test_result_to_console("Rawdata", cts_data->rawdata_test_result);
	}
	if (cts_data->test_noise) {
		print_test_result_to_console("Noise", cts_data->noise_test_result);
	}
	if (cts_data->test_open) {
		print_test_result_to_console("Open", cts_data->open_test_result);
	}
	if (cts_data->test_short) {
		print_test_result_to_console("Short", cts_data->short_test_result);
	}
	if (cts_data->test_compensate_cap) {
		print_test_result_to_console("Comp-Cap", cts_data->comp_cap_test_result);
	}
}

static void do_factory_test(void)
{
	ktime_t start_time, end_time, delta_time;

	TS_LOG_INFO("Factory test start...\n");

	start_time = ktime_get();

	if (cts_data->test_reset_pin) {
		cts_data->reset_pin_test_result = cts_test_reset_pin(
			&cts_data->cts_dev, &cts_data->reset_pin_test_param);
	}

	if (cts_data->test_int_pin) {
		cts_data->int_pin_test_result = cts_test_int_pin(
			&cts_data->cts_dev, &cts_data->int_pin_test_param);
	}
	if (cts_data->test_rawdata) {
		cts_data->rawdata_test_result = cts_test_rawdata(
			&cts_data->cts_dev, &cts_data->rawdata_test_param);
	}
	if (cts_data->test_noise) {
		cts_data->noise_test_result = cts_test_noise(
			&cts_data->cts_dev, &cts_data->noise_test_param);
	}
	if (cts_data->test_open) {
		cts_data->open_test_result = cts_test_open(
			&cts_data->cts_dev, &cts_data->open_test_param);
	}
	if (cts_data->test_short) {
		cts_data->short_test_result = cts_test_short(
			&cts_data->cts_dev, &cts_data->short_test_param);
	}
	if (cts_data->test_compensate_cap) {
		cts_data->comp_cap_test_result = cts_test_compensate_cap(
			&cts_data->cts_dev, &cts_data->comp_cap_test_param);
	}

	end_time = ktime_get();
	delta_time = ktime_sub(end_time, start_time);
	TS_LOG_INFO("Factory test complete, ELAPSED TIME: %lldms\n",
		ktime_to_ms(delta_time));
}

#define TP_TEST_FAILED_REASON_LEN 20
static int chipone_get_rawdata(struct ts_rawdata_info *info,
	struct ts_cmd_node *out_cmd)
{
	int ret;
	char failed_reason[TP_TEST_FAILED_REASON_LEN + 1] = "-";

	if (cts_data == NULL) {
		TS_LOG_ERR("Get rawdata with cts_data = NULL\n");
		return -EINVAL;
	}

	TS_LOG_INFO("Get rawdata\n");

	ret = init_factory_test_param();
	if (ret) {
		TS_LOG_ERR("Init factory test param failed %d(%s)\n",
			ret, cts_strerror(ret));
		strncpy(failed_reason, "software_reason-", TP_TEST_FAILED_REASON_LEN);
		goto output_result;
	}

	do_factory_test();

	print_factory_test_result_to_console();

	/* 0: bus test
	 * 1: rawdata test
	 * 2: Trx delta(adj)
	 * 3: noise test
	 * 4: IC vendor custom
	 * 5: open & short test
	 * 6~9: IC vendor custom
	 */
	if (cts_data->rawdata_test_result ||
		cts_data->noise_test_result ||
		cts_data->open_test_result ||
		cts_data->short_test_result ||
		cts_data->reset_pin_test_result ||
		cts_data->int_pin_test_result ||
		cts_data->comp_cap_test_result)
		strncpy(failed_reason, "panel_reason-", TP_TEST_FAILED_REASON_LEN);

output_result:
	snprintf(info->result, TS_RAWDATA_RESULT_MAX,
		"0P-1%c-2%c-3%c-4%c-5%c-6%c-7%c-8%c-%s%s;",
		cts_data->rawdata_test_result ? 'F' : 'P',
		'P',
		cts_data->noise_test_result ? 'F' : 'P',
		cts_data->open_test_result ? 'F' : 'P',
		cts_data->short_test_result ? 'F' : 'P',
		cts_data->reset_pin_test_result ? 'F' : 'P',
		cts_data->int_pin_test_result ? 'F' : 'P',
		cts_data->comp_cap_test_result ? 'F' : 'P',
		failed_reason,
		cts_data->project_id);
	TS_LOG_INFO("test result: %s\n", info->result);
	return ret;
}

static int chipone_get_capacitance_test_type(struct ts_test_type_info *info)
{
	struct ts_kit_device_data *ts_dev_data = cts_data->tskit_data;

	if (IS_ERR_OR_NULL(ts_dev_data) || IS_ERR_OR_NULL(info)) {
		TS_LOG_ERR("ts_dev_data or info invaild\n");
		return -EINVAL;
	}

	switch (info->op_action) {
	case TS_ACTION_READ:
		memcpy(info->tp_test_type, ts_dev_data->tp_test_type,
			TS_CAP_TEST_TYPE_LEN);
		TS_LOG_INFO("test_type = %s\n", info->tp_test_type);
		break;
	case TS_ACTION_WRITE:
		break;
	default:
		TS_LOG_ERR("invalid op action: %d\n", info->op_action);
		return -EINVAL;
	}

	return 0;
}

static int print_touch_data_row_to_buffer(char *buf, size_t size,
	const u16 *data, int cols, const char *prefix, const char *suffix,
	char seperator)
{
	int c, count = 0;

	if (prefix) {
		count += scnprintf(buf, size, "%s", prefix);
	}

	for (c = 0; c < cols; c++) {
		count += scnprintf(buf + count, size - count,
			"%4u%c ", data[c], seperator);
	}

	if (suffix) {
		count += scnprintf(buf + count, size - count, "%s", suffix);
	}

	return count;
}

static void print_touch_data_to_seq_file(struct seq_file *m,
	const u16 *data, int frames, int rows, int cols)
{
	int i, r;

	for (i = 0; i < frames; i++) {
		for (r = 0; r < rows; r++) {
			char linebuf[256];
			int len;

			len = print_touch_data_row_to_buffer(linebuf, sizeof(linebuf),
				data, cols, NULL, "\n", ',');
			seq_puts(m, linebuf);

			data += cols;
		}
		seq_putc(m, '\n');
	}
}

static int print_comp_cap_row_to_buffer(char *buf, size_t size, const u8 *cap,
	int cols, const char *prefix, const char *suffix, char seperator)
{
	int c, count = 0;

	if (prefix) {
		count += scnprintf(buf, size, "%s", prefix);
	}

	for (c = 0; c < cols; c++) {
		count += scnprintf(buf + count, size - count,
			"%3u%c ", cap[c], seperator);
	}

	if (suffix) {
		count += scnprintf(buf + count, size - count, "%s", suffix);
	}

	return count;
}

static void print_comp_cap_to_seq_file(struct seq_file *m,
	const u8 *data, int rows, int cols)
{
	int r;

	for (r = 0; r < rows; r++) {
		char linebuf[256];
		int len;

		len = print_comp_cap_row_to_buffer(linebuf, sizeof(linebuf),
			data, cols, NULL, "\n", ',');
		seq_puts(m, linebuf);

		data += cols;
	}
}

static void print_test_result_to_seq_file(struct seq_file *m,
	const char *test_desc, int test_result)
{
	if (test_result > 0) {
		seq_printf(m, "%-10s test: FAIL %d nodes\n", test_desc,
			test_result);
	} else if (test_result < 0) {
		seq_printf(m, "%-10s test: FAIL %d(%s)\n", test_desc,
			test_result, cts_strerror(test_result));
	} else {
		seq_printf(m, "%-10s test: PASS\n", test_desc);
	}
}

static int chipone_rawdata_proc_printf(struct seq_file *m,
	struct ts_rawdata_info *info, int range_size, int row_size)
{
	int rows, cols;

	if (cts_data == NULL) {
		TS_LOG_ERR("Print touch data to seq file with cts_data = NULL\n");
		return -EFAULT;
	}

	TS_LOG_INFO("Print touch data to seq file, range: %d, row: %d\n",
		range_size, row_size);

	rows = cts_data->cts_dev.fwdata.rows;
	cols = cts_data->cts_dev.fwdata.cols;

	seq_printf(m, "FW Version %04x!\n\n", cts_data->cts_dev.fwdata.version);

	if (cts_data->test_reset_pin) {
		print_test_result_to_seq_file(m,
			"Reset-Pin", cts_data->reset_pin_test_result);
	}

	if (cts_data->test_int_pin) {
		print_test_result_to_seq_file(m,
			"Int-Pin", cts_data->int_pin_test_result);
	}

	if (cts_data->test_rawdata) {
		print_test_result_to_seq_file(m,
			"Rawdata", cts_data->rawdata_test_result);
		if (cts_data->dump_factory_test_data_to_buffer &&
			(cts_data->rawdata_test_result > 0 ||
			!cts_data->print_factory_test_data_only_fail)) {
			print_touch_data_to_seq_file(m,
				cts_data->rawdata_test_param.test_data_buf,
				cts_data->rawdata_test_priv_param.frames,  rows, cols);
		}
	}

	if (cts_data->test_noise) {
		print_test_result_to_seq_file(m,
			"Noise", cts_data->noise_test_result);
		if (cts_data->dump_factory_test_data_to_buffer &&
			(cts_data->noise_test_result > 0 ||
			!cts_data->print_factory_test_data_only_fail)) {
			/* Only dump noise, max and min frame */
			print_touch_data_to_seq_file(m,
				cts_data->noise_test_param.test_data_buf +
				2 * rows * cols * cts_data->noise_test_priv_param.frames,
				3, rows, cols);
		}
	}

	if (cts_data->test_open) {
		print_test_result_to_seq_file(m,
			"Open", cts_data->open_test_result);
		if (cts_data->dump_factory_test_data_to_buffer &&
			(cts_data->open_test_result > 0 ||
			!cts_data->print_factory_test_data_only_fail)) {
			print_touch_data_to_seq_file(m,
				cts_data->open_test_param.test_data_buf, 1,  rows, cols);
		}
	}

	if (cts_data->test_short) {
		print_test_result_to_seq_file(m,
			"Short", cts_data->short_test_result);
		if (cts_data->dump_factory_test_data_to_buffer &&
			(cts_data->short_test_result > 0 ||
			!cts_data->print_factory_test_data_only_fail)) {
			print_touch_data_to_seq_file(m,
				cts_data->short_test_param.test_data_buf, 7, rows, cols);
		}
	}
	if (cts_data->test_compensate_cap) {
		print_test_result_to_seq_file(m,
			"Comp-cap", cts_data->comp_cap_test_result);
		if (cts_data->dump_factory_test_data_to_buffer &&
			(cts_data->comp_cap_test_result > 0 ||
			!cts_data->print_factory_test_data_only_fail)) {
			print_comp_cap_to_seq_file(m,
				cts_data->comp_cap_test_param.test_data_buf, rows, cols);
		}
	}

	return 0;
}

static int chipone_wakeup_gesture_enable_switch(
	struct ts_wakeup_gesture_enable_info *info)
{
	int ret = NO_ERR;

	TS_LOG_INFO("Gesture wakeup switch\n");

	if (info == NULL) {
		TS_LOG_ERR("Gesture wakeup switch with info = NULL\n");
		ret = -EINVAL;
	}

	TS_LOG_INFO("Gesture wakeup switch returns %d\n", ret);
	return ret;
}

static int chipone_chip_suspend(void)
{
	struct ts_kit_device_data *tskit_dev_data = NULL;
	struct ts_kit_platform_data *ts_plat_data = NULL;
	struct ts_easy_wakeup_info *wakeup_info = NULL;
	int ret = NO_ERR;

	if (cts_data == NULL) {
		TS_LOG_ERR("Chip suspend with cts_data = NULL\n");
		return -EINVAL;
	}

	tskit_dev_data = cts_data->tskit_data;
	ts_plat_data = tskit_dev_data->ts_platform_data;
	wakeup_info = &tskit_dev_data->easy_wakeup_info;

	TS_LOG_INFO("Chip suspend with%s gesture wakeup\n",
		(wakeup_info->sleep_mode == TS_GESTURE_MODE) ? "" : "out");

	mutex_lock(&cts_data->wrong_touch_lock);
	switch (wakeup_info->sleep_mode) {
	case TS_POWER_OFF_MODE:
		cts_disable_gesture_wakeup(&cts_data->cts_dev);
		wakeup_info->easy_wakeup_flag = false;
		wakeup_info->off_motion_on = false;
		break;

	case TS_GESTURE_MODE:
		cts_enable_gesture_wakeup(&cts_data->cts_dev);
		wakeup_info->easy_wakeup_flag = true;
		wakeup_info->off_motion_on = true;
		break;

	default:
		TS_LOG_ERR("Suspend with unknown sleep mode %d\n",
		wakeup_info->sleep_mode);
		break;
	}
	mutex_unlock(&cts_data->wrong_touch_lock);

	cts_lock_device(&cts_data->cts_dev);
	ret = cts_suspend_device(&cts_data->cts_dev);
	cts_unlock_device(&cts_data->cts_dev);

	if (ret)
		TS_LOG_ERR("Suspend device failed %d(%s)\n",
			ret, cts_strerror(ret));

	if (wakeup_info->sleep_mode != TS_GESTURE_MODE)  {
#ifdef CONFIG_HUAWEI_DEVKIT_MTK_3_0
		pinctrl_select_idle_state();
#else
		gpio_direction_output(ts_plat_data->cs_gpio, 0);
#endif /* CONFIG_HUAWEI_DEVKIT_MTK_3_0 */
	}

	ret = cts_stop_device(&cts_data->cts_dev);
	if (ret) {
		TS_LOG_ERR("Stop device failed %d(%s)\n",
			ret, cts_strerror(ret));
		return ret;
	}

#ifdef CFG_CTS_GESTURE
	/* Enable IRQ wake if gesture wakeup enabled */
	if (cts_is_gesture_wakeup_enabled(&cts_data->cts_dev)) {
		ret = enable_irq_wake(cts_data->tskit_data->ts_platform_data->irq_id);
		if (ret) {
			TS_LOG_ERR("Disable IRQ wake failed %d(%s)\n",
				ret, cts_strerror(ret));
			return ret;
		}
		enable_irq(cts_data->tskit_data->ts_platform_data->irq_id);
	}
#endif /* CFG_CTS_GESTURE */

	/** - To avoid waking up while not sleeping,
			delay 20ms to ensure reliability */
	msleep(20);

	return 0;
}

static int chipone_chip_resume(void)
{
	struct ts_easy_wakeup_info *wakeup_info = NULL;
	int ret = NO_ERR;

	if (cts_data == NULL) {
		TS_LOG_ERR("Chip resume with cts_data = NULL\n");
		return -EINVAL;
	}

	wakeup_info = &cts_data->tskit_data->easy_wakeup_info;

	TS_LOG_INFO("Chip resume with%s gesture wakeup\n",
		(wakeup_info->sleep_mode == TS_GESTURE_MODE) ? "" : "out");

	if (wakeup_info->sleep_mode != TS_GESTURE_MODE) {
#ifdef CONFIG_HUAWEI_DEVKIT_MTK_3_0
		pinctrl_select_default_state();
#else
		gpio_direction_output(cts_data->tskit_data->ts_platform_data->cs_gpio, 1);
#endif /* CONFIG_HUAWEI_DEVKIT_MTK_3_0 */
	}
	wakeup_info->easy_wakeup_flag = false;

#ifdef CFG_CTS_GESTURE
	if (cts_is_gesture_wakeup_enabled(&cts_data->cts_dev)) {
		ret = disable_irq_wake(cts_data->tskit_data->ts_platform_data->irq_id);
		if (ret) {
			TS_LOG_ERR("Disable IRQ wake failed %d", ret);
			//return ret;
		}
		disable_irq(cts_data->tskit_data->ts_platform_data->irq_id);
	}
#endif /* CFG_CTS_GESTURE */

	cts_lock_device(&cts_data->cts_dev);
	ret = cts_resume_device(&cts_data->cts_dev);
	cts_unlock_device(&cts_data->cts_dev);
	if(ret) {
		TS_LOG_ERR("Resume device failed %d(%s)\n",
			ret, cts_strerror(ret));
		return ret;
	}

	ret = cts_start_device(&cts_data->cts_dev);
	if (ret) {
		TS_LOG_ERR("Start device failed %d(%s)\n",
			ret, cts_strerror(ret));
		return ret;
	}

	return 0;
}

static int chipone_chip_test(struct ts_cmd_node *in_cmd, struct ts_cmd_node *out_cmd)
{
	TS_LOG_INFO("%s: TODO\n", __func__);

	return 0;
}

static int chipone_hw_reset(void)
{
	int ret = NO_ERR;

	if (cts_data == NULL) {
		TS_LOG_ERR("Chip hw reset with cts_data = NULL\n");
		return -EINVAL;
	}

	TS_LOG_INFO("Chip hw reset\n");

	ret = do_hw_reset(cts_data->resume_hw_reset_timing);
	if (ret) {
		TS_LOG_ERR("Do hardware reset failed %d(%s)\n",
			ret, cts_strerror(ret));
	}

	TS_LOG_INFO("Chip hw reset returns %d\n", ret);

	return ret;
}

static int chipone_wrong_touch(void)
{
	if (cts_data == NULL) {
		TS_LOG_ERR("Wrong touch switch ON with cts_data = NULL\n");
		return -EINVAL;
	}

	TS_LOG_INFO("Wrong touch switch ON\n");

	mutex_lock(&cts_data->wrong_touch_lock);
	cts_data->tskit_data->easy_wakeup_info.off_motion_on = true;
	mutex_unlock(&cts_data->wrong_touch_lock);

	return 0;
}

static int chipone_roi_switch(struct ts_roi_info *info)
{
	int ret;

	if (info == NULL) {
		TS_LOG_ERR("ROI switch with info = NULL\n");
		return -EINVAL;
	}

	TS_LOG_INFO("ROI switch, action: %d\n", info->op_action);

	switch (info->op_action) {
	case TS_ACTION_READ:
		info->roi_switch = cts_data->roi_switch;
		return 0;

	case TS_ACTION_WRITE:
		cts_lock_device(&cts_data->cts_dev);
		cts_data->roi_switch = info->roi_switch;
		ret = cts_send_command(&cts_data->cts_dev,
			(!!info->roi_switch) ? CTS_CMD_ENABLE_NOI : CTS_CMD_DISABLE_NOI);
		cts_unlock_device(&cts_data->cts_dev);
		if (ret) {
			TS_LOG_ERR("Send %s failed %d\n",
				!!info->roi_switch ? "CMD_ENABLE_NOI" : "CMD_DISABLE_NOI", ret);
		}
		return ret;

	default:
		TS_LOG_INFO("ROI switch with action: %d unknown\n",
			info->op_action);
		return -EINVAL;
	}
}

static unsigned char *chipone_roi_rawdata(void)
{
	struct ts_roi_info *roi_info = NULL;

	roi_info = &cts_data->tskit_data->ts_platform_data->feature_info.roi_info;
	if (!roi_info->roi_supported) {
		TS_LOG_ERR("Get ROI rawdata with NOT supported\n");
		return NULL;
	}

	if (!cts_data->roi_switch) {
		TS_LOG_ERR("Get ROI rawdata with switch = OFF\n");
		return NULL;
	}

	TS_LOG_DEBUG("Get ROI rawdata\n");

	return (unsigned char *)cts_data->roi_data;
}

/* Platform ops needed */
int cts_plat_reset_device(struct cts_platform_data *pdata)
{
	return chipone_hw_reset();
}

int cts_plat_release_all_touch(struct cts_platform_data *pdata)
{
	return 0;
}

int cts_plat_set_reset(struct cts_platform_data *pdata, int val)
{
	return set_reset_gpio_value(val);
}

int cts_plat_get_int_pin(struct cts_platform_data *pdata)
{
	return gpio_get_value(cts_data->tskit_data->ts_platform_data->irq_gpio);
}

struct ts_device_ops chipone_ts_device_ops = {
	.chip_detect = chipone_chip_detect,
	.chip_init = chipone_chip_init,
	.chip_parse_config = chipone_parse_config,
	.chip_input_config = chipone_input_config,
	.chip_get_info = chipone_chip_get_info,
	.chip_irq_top_half = chipone_irq_top_half,
	.chip_irq_bottom_half = chipone_irq_bottom_half,
	.chip_reset = chipone_chip_reset,
	.chip_shutdown = chipone_chip_shutdown,
	.chip_fw_update_boot = chipone_fw_update_boot,
	.chip_fw_update_sd = chipone_fw_update_sd,
	.chip_get_rawdata = chipone_get_rawdata,
	.chip_get_capacitance_test_type = chipone_get_capacitance_test_type,
	.chip_special_rawdata_proc_printf = chipone_rawdata_proc_printf,
	.chip_wakeup_gesture_enable_switch = chipone_wakeup_gesture_enable_switch,
	.chip_suspend = chipone_chip_suspend,
	.chip_resume = chipone_chip_resume,
	.chip_roi_switch = chipone_roi_switch,
	.chip_roi_rawdata = chipone_roi_rawdata,
	.chip_test = chipone_chip_test,
	.chip_hw_reset = chipone_hw_reset,
	.chip_wrong_touch = chipone_wrong_touch,
};

static int __init chipone_ts_module_init(void)
{
	struct device_node* child = NULL;
	struct device_node* root = NULL;
	int ret = NO_ERR;
	bool found = false;

	TS_LOG_INFO("touch module init\n");

	cts_data = kzalloc(sizeof(struct chipone_ts_data), GFP_KERNEL);
	if (cts_data == NULL) {
		TS_LOG_ERR("Alloc ts_data failed\n");
		return -ENOMEM;
	}

	cts_data->tskit_data = kzalloc(sizeof(struct ts_kit_device_data), GFP_KERNEL);
	if (cts_data->tskit_data == NULL) {
		TS_LOG_ERR("Alloc ts_kit_device_data failed\n");
		ret = -ENOMEM;
		goto out;
	}

	cts_data->pdata = kzalloc(sizeof(struct cts_platform_data), GFP_KERNEL);
	if (cts_data->pdata == NULL) {
		TS_LOG_ERR("Alloc platform_data failed\n");
		ret = -ENOMEM;
		goto out;
	}

	root = of_find_compatible_node(NULL, NULL, "huawei,ts_kit");
	if (root == NULL) {
		TS_LOG_ERR("Huawei ts_kit of node NOT found in DTS\n");
		ret = -ENODEV;
		goto out;
	}

	for_each_child_of_node(root, child) {
		if (of_device_is_compatible(child, "chipone")) {
			found = true;
			break;
		}
	}

	if (!found) {
		TS_LOG_ERR("device tree node not found\n");
		ret = -ENODEV;
		goto out;
	}


#ifdef CONFIG_CTS_ESD_PROTECTION
	cts_init_esd_protection(cts_data);
	mutex_init(&cts_data->wrong_touch_lock);
#endif
	cts_data->tskit_data->cnode = child;
	cts_data->tskit_data->ops = &chipone_ts_device_ops;
	ret = huawei_ts_chip_register(cts_data->tskit_data);
	if(ret) {
		TS_LOG_ERR("Register chip to ts_kit failed %d(%s)!\n",
			ret, cts_strerror(ret));
		goto out;
	}

	return 0;
out:
	if (cts_data) {
		if (cts_data->tskit_data) {
			kfree(cts_data->tskit_data);
			cts_data->tskit_data = NULL;
		}
		if (cts_data->pdata) {
			kfree(cts_data->pdata);
			cts_data->pdata = NULL;
		}
		kfree(cts_data);
		cts_data = NULL;
	}

	TS_LOG_INFO("touch module init returns %d\n", ret);

	return ret;
}

static void __exit chipone_ts_module_exit(void)
{
#if (defined(CONFIG_CTS_CHARGER_DETECT) || defined(CONFIG_CTS_EARJACK_DETECT))
	int ret;
#endif
	if (cts_data == NULL){
		TS_LOG_ERR("Module exit with cts_data = NULL\n");
		return ;
	}

	TS_LOG_INFO("touch module exit");

	pinctrl_select_idle_state();
	release_pinctrl();
	if (cts_data->cached_firmware) {
		TS_LOG_INFO("Release cached firmware");
		cts_release_firmware(cts_data->cached_firmware);
		cts_data->cached_firmware = NULL;
	}

	cts_tool_deinit(cts_data);

#ifdef CONFIG_CTS_CHARGER_DETECT
	if (!cts_data->use_huawei_charger_detect) {
		ret = cts_deinit_charger_detect(cts_data);
		if (ret) {
			TS_LOG_ERR("Deinit charger detect failed %d(%s)\n",
				ret, cts_strerror(ret));
		}
	}
#endif /* CONFIG_CTS_CHARGER_DETECT */

#ifdef CONFIG_CTS_EARJACK_DETECT
	ret = cts_deinit_earjack_detect(cts_data);
	if (ret) {
		TS_LOG_ERR("Deinit earjack detect failed %d(%s)\n",
			ret, cts_strerror(ret));
	}
#endif /* CONFIG_CTS_EARJACK_DETECT */

	cts_sysfs_remove_device(cts_data->device);

	free_factory_test_mem();

	if (cts_data->tskit_data) {
		kfree(cts_data->tskit_data);
		cts_data->tskit_data = NULL;
	}
	if (cts_data->pdata) {
		kfree(cts_data->pdata);
		cts_data->pdata = NULL;
	}
	kfree(cts_data);
	cts_data = NULL;
}

late_initcall(chipone_ts_module_init);
module_exit(chipone_ts_module_exit);
MODULE_AUTHOR("Huawei Device Company");
MODULE_DESCRIPTION("Huawei TouchScreen Driver");
MODULE_LICENSE("GPL");
