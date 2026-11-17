

#ifndef _HISI_SIM_ADAPT_
#define _HISI_SIM_ADAPT_

#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/irqdomain.h>
#include <linux/kern_levels.h>
#include <linux/kernel.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/string.h>

#define HW_SIM_OK 0
#define HW_SIM_ERR (-1)

#define SIM0 0
#define SIM1 1
#define MAX_SIMHP_NUM 2
#define USIM_OFFSET_SIZE 3
#define ESIM_DET_OFFSET_SIZE 4
#define SD_CMD_INDEX 0
#define SD_DATA0_INDEX 1
#define SD_DATA2_INDEX 2
#define SD_DATA3_INDEX 3

#define USIM_IO_MUX 0x001
#define GPIO_IO_MUX 0x000
#define SD_USIM_MUX 0x04
#define SD_GPIO_IO_MUX 0x00
#define USIM_IOCG_VALUE 0x10
#define USIM_SD_IOCG_PU_VALUE 0x11
#define USIM_SD_IOCG_PD_VALUE 0x12

#define STATUS_SIM 5
#define STATUS_SD 6
#define STATUS_NO_CARD 7
#define STATUS_SD2JTAG 8
#define VCC_1_8V 1800000
#define VCC_3_0V 3000000
#define AP_POWER_SCHE_A 1
#define AP_POWER_SCHE_B 2

#define HW_SIM_HARDWARE0 "hw-sim-hardware0"
#define HW_SIM_HARDWARE1 "hw-sim-hardware1"

#define hwsim_loge(fmt, ...) (void)printk(KERN_ERR "hisi_sim_adapt <%s>"fmt, __func__, ##__VA_ARGS__)
#define hwsim_logi(fmt, ...) (void)printk(KERN_INFO "hisi_sim_adapt <%s>"fmt, __func__, ##__VA_ARGS__)

typedef enum {
	SIM_POWER_TYPE_SINGLE = 0x01,
	SIM_POWER_TYPE_SIM_SD = 0x02,
	SIM_POWER_TYPE_EXTRAL_MODEM = 0x03,
	SIM_POWER_TYPE_EXTRAL_LDO = 0x04,
	SIM_POWER_TYPE_AUX = 0x05,
	SIM_POWER_TYPE_EXTRAL_LDO_BUT_NO_SD = 0x06, // teton slave card use esim io
	SIM_POWER_TYPE_BUTT
} sim_power_type_e;

typedef enum {
	SIM_IO_MUX_SIMIO_USIM_NANOSDIO_SD_REQUEST = 0x01,
	SIM_IO_MUX_SIMIO_GPIO_NANOSDIO_USIM_REQUEST = 0x02,
	SIM_IO_MUX_SIMIO_USIM_ONLY_REQUEST = 0x03,
	SIM_IO_MUX_GPIO_DET_REQUEST = 0x04,
	SIM_IO_GPIO_FOR_NVT_SHIFT = 0x05,
	SIM_IO_USIM_FOR_NVT_SHIFT = 0x06,
	SIM_IO_MUX_BUTT
} set_sim_io_type;

typedef enum {
	EXTRAL_LDO_OUT_TYPE_GPIO = 0x01,
	EXTRAL_LDO_OUT_TYPE_PMU = 0x02
} extral_ldo_out_type_e;

typedef enum {
	SIM_TYPE_USIM = 0x01,
	SIM_TYPE_ESIM = 0x02
} current_usim_esim_type_e;

typedef struct {
	u32 func;
	u32 reg_off;
	u32 reg_bit;
} sim_sysctrl_s;

typedef struct {
	u32 func;
	u32 nvt_shift_gpio; // GPIO_090 SEL
	u32 nvt_fix_leak;
} nvt_shift_crl;

typedef struct {
	struct regulator *sim_vcc_io;
} single_sim_pmu_cfg;

typedef struct {
	struct regulator *sim_vcc;
	struct regulator *aux_ldo;
} aux_sim_pmu_cfg;

typedef struct {
	struct regulator *sim_sd_vcc;
	struct regulator *sim_sd_io;
} sim_sd_pmu_cfg;

typedef struct {
	u32 sim1_ap_power_scheme;
	struct regulator *sim_pmu_switch;
	struct regulator *sd_io_pmu;
	struct regulator *sd_vcc_pmu;
	struct regulator *sd_switch_pmu;
} extral_modem_pmu_cfg;

typedef struct {
	int extral_ldo_bias_use;
	extral_ldo_out_type_e extral_ldo_out_type;
	struct regulator *extral_ldo_en;
	struct regulator *extral_ldo_out_select;
	u32 extral_ldo_out_gpio_select;
	struct regulator *sim_sd_io;
} extral_ldo_pmu_cfg;

typedef struct {
	struct regulator *esim_vcc;
	unsigned int gpio_d0;
	unsigned int gpio_d1;
	unsigned int gpio_d2;
	unsigned int gpio_d3;
} esim_ctrl_cfg;

typedef struct {
	sim_power_type_e sim_pmu_hardware_type;
	sim_sysctrl_s lv_shift_ctrl;
	nvt_shift_crl nvt_shift;
	esim_ctrl_cfg esim_ctrl;
	union {
		single_sim_pmu_cfg single_sim_pmu;
		sim_sd_pmu_cfg sim_sd_pmu;
		extral_modem_pmu_cfg extral_modem_pmu;
		extral_ldo_pmu_cfg extral_ldo_pmu;
		aux_sim_pmu_cfg aux_sim_pmu;
	} hard_cfg;
} sim_pmu_hardware_ctrl;

struct sim_io_mux_cfg {
	u32 phy_addr;
	void *virt_addr;
	u32 iomg_usim_offset[USIM_OFFSET_SIZE];
	u32 iocg_usim_offset[USIM_OFFSET_SIZE];
	u32 esim_detect_en;
	u32 esim_det_func_offset[ESIM_DET_OFFSET_SIZE];
	u32 esim_det_ctrl_offset[ESIM_DET_OFFSET_SIZE];
};

struct hw_sim_hardware_info {
	struct device *spmidev;
	struct device_node *np;
	u32 sim_id;
	current_usim_esim_type_e sim_esim_type;
	u32 sim_status;
	u32 esim_classc_only;
	struct sim_io_mux_cfg sim_io_cfg;
	struct sim_io_mux_cfg sd_io_cfg;
	sim_pmu_hardware_ctrl sim_pmu_hardware;
};

typedef enum {
	SIM_HW_CARD_TYPE_INQUIRE = 0x11,
	SIM_HW_CARD_DET_WHILE_ESIM_INQUIRE = 0x12,
	SIM_HW_POWER_ON_CLASS_C_REQUEST = 0x21, // set 1.8V voltage
	SIM_HW_POWER_ON_CLASS_B_REQUEST = 0x22, // set 3.0V voltage
	SIM_HW_POWER_OFF_REQUEST = 0x23,
	SIM_HW_DEACTIVE_NOTIFIER = 0x31,
	SIM_HW_SET_ESIM_SWITCH = 0x41,
	SIM_HW_SET_USIM_SWITCH = 0x42,
	SIM_HW_MSG_TYPE_BUTT
} sci_hw_op_type_e;

typedef enum {
	SIM_HW_OP_RESULT_SUCC = 0x00,
	SIM_HW_HOTPLUG_DTS_ERR,
	SIM_HW_HOTPLUG_DET_ERR,
	SIM_HW_HOTPLUG_HPD_ERR,
	SIM_HW_HOTPLUG_STATE_ERR,
	SIM_HW_HOTPLUG_HPD_FAIL,
	SIM_HW_INVALID_INQUIRE_MSG = 0x100,
	SIM_HW_CARD_TYPE_SIM = 0x101,
	SIM_HW_CARD_TYPE_MUXSIM = 0x102,
	SIM_HW_CARD_TYPE_SD,
	SIM_HW_CARD_TYPE_JTAG,
	SIM_HW_CARD_TYPE_FAIL,
	SIM_HW_CARD_EXIST_WHILE_ESIM,
	SIM_HW_CARD_NO_EXIST_WHILE_ESIM,
	SIM_HW_SET_GPIO_FAIL,
	SIM_HW_INVALID_POWER_MSG = 0x200,
	SIM_HW_POWER_ON_SUCC = 0x201,
	SIM_HW_POWER_ON_FAIL,
	SIM_HW_POWER_OFF_SUCC,
	SIM_HW_POWER_OFF_FAIL,
	SIM_HW_REQUESR_PMU_FAIL,
	SIM_HW_SET_VOLTAGE_FAIL,
	SIM_HW_INVALID_NOTIFIER_MSG = 0x300,
	SIM_HW_DEACTIVE_SUCC = 0x301,
	SIM_HW_DEACTIVE_FAIL,
	SIM_HW_INVALID_SWITCH_MSG = 0x400,
	SIM_HW_SET_ESIM_SUCC = 0x401,
	SIM_HW_SET_ESIM_FAIL,
	SIM_HW_SET_USIM_SUCC,
	SIM_HW_SET_USIM_FAIL,
	SIM_HW_REQUEST_RESULT_BUTT
} sci_hw_op_result_e;

struct sim_hw_debug_info {
	u32 iomux_status[USIM_OFFSET_SIZE];
	u32 power_status;
	u32 event_len; /* event queue len */
	u32 data[]; /* events */
};

extern u8 get_card1_type(void);
extern int sim_hwadp_init(u32 sim_id, struct device *dev);
extern sci_hw_op_result_e sim_hwadp_handler(u32 sim_id, sci_hw_op_type_e msg_id);
extern int sim_get_hw_debug_info(u32 sim_id, struct sim_hw_debug_info *dbg_info);

#endif // _HISI_SIM_ADAPT_

