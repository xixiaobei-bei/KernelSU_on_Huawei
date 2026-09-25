#ifndef __HISI_NPU_PM_PRIVATE_H__
#define __HISI_NPU_PM_PRIVATE_H__


#define KHz		(1000U)
#define MHz		(1000000U)

#define MAX_AVS_VOLT			(4)
#define SVFD_REG_OFFSET_NUM 	(14)
#define NPU_PA_REG_OFFSET_NUM 	(7)
#define SVFD_CFG_BIT_NUM 		(7)
#define PA_MARGIN_NUM 			(2)

/*
 *This 2 macro used for freqdump
 *if need change the value, also need check the freqdump_plat.h for all platform
 */
#define MAX_MODULE_NAME_LEN     (10)
#define MAX_MODULE_COUNT        (6)

#define PROF_ACTIVE_DEFENCE ((unsigned int)BIT(4))
#define PROF_SVFD       ((unsigned int)BIT(2))
#define PROF_AVS       ((unsigned int)BIT(1))
#define VSENSOR_SVFD_CTRL2_MASK	(0x3FFF << 16)

/****** all hisi avs service function id start *****/
#define HISI_AVS_STD_SVC_VER					0xc9000000
#define HISI_AVS_SVC_CALL_COUNT					0xc9000001
#define HISI_AVS_SVC_UID						0xC9000002
#define HISI_AVS_SVC_VERSION					0xC9000003
#define HISI_AVS_NPU_PA_MODE_CFG				0xC900DD01
#define HISI_AVS_NPU_PA_MODE_RECOVERY			0xC900DD02
#define HISI_AVS_NPU_GET_PA_VALUE				0xC900DD03
/****** all hisi avs service function id end ******/

struct clk_div_regs {
	void __iomem *cfg_reg;
	void __iomem *stat_reg;
	unsigned int rd_mask;
	unsigned int wr_mask;
};

struct clk_gate_regs {
	void __iomem *en_reg;
	void __iomem *dis_reg;
	void __iomem *stat_reg;
	unsigned int mask;
};

struct hisi_npu_profile {
	unsigned int freq; /* khz */
	unsigned int voltage; /* uV */
	unsigned int func_sw;
	unsigned int avsvolt[MAX_AVS_VOLT];
	unsigned int svfdcode[MAX_AVS_VOLT];
	struct clk *pll_clk;
	const char *clk_name;
	unsigned int pll;
	unsigned int div;
};

struct module_clk_list {
	struct list_head list; /* kernel's list structure */
	const char *name;
	struct clk *clk;
	unsigned int clk_sw;
};


struct module_npu_pa_ctrl_info {
	struct clk *pll_clk;
	unsigned int pa_safe_prof;
	/*
	 * 0: HCI
	 * 1: NBTI
	*/
	unsigned int pa_margin[PA_MARGIN_NUM];
	unsigned int pa_step_volt;
	unsigned int pa_max_code;
};

struct module_svfd_ctrl_info {
	void __iomem *vs_crg_base;
	void __iomem *vs_ctrl_base;
	/*
	 * 0:vsensor rsten
	 * 1:vsensor rstdis
	 * 2:vsensor rststat
	 * 3:ffs&cpm rst
	 * 4:ffs&cpm disrst
	 * 5:ffs&cpm stat
	 * 6:vs_ctrl_en
	 * 7:vs_ctrl_bypass
	 * 8:vs_ctrl_npu
	 * 9:vs_ctrl0
	 * 10:vs_ctrl1
	 * 11:vs_ctrl2
	 * 12:cpm_data
	 * 13:test_stat
	 */
	unsigned int vdm_reg_offset[SVFD_REG_OFFSET_NUM];
	/*
	 * 0.vsensor_rst_bit
	 * 1.svfd_clk_icg_bit
	 * 2.ffs_bit
	 * 3.cpm_bit
	 * 4.cpm_mode_bit
	 * 5.vdm_mode_bit
	 * 6.svfd_data_limit_e_bit
	 */
	unsigned int vdm_cfg_bit[SVFD_CFG_BIT_NUM];
};

struct hisi_npu_module_profile {
	const char *module_name;
	struct hisi_npu_profile *profile_table;
	int profile_count;
	int cur_pid;
	struct regulator *buck;
	unsigned int buck_step_time;
	unsigned int buck_hold_time;
	unsigned int last_voltage; /* uV */
	unsigned int lt_freq_uplimit; /* khz, low temperature threshold */
	unsigned int voltage_lt; /* uV */
	unsigned avs_temp[MAX_AVS_VOLT];
	void __iomem *base;
	struct module_svfd_ctrl_info svfd_ctrl;
	struct module_npu_pa_ctrl_info pa_ctrl;
	struct clk *cur_clk;
	struct module_clk_list clk_list;
	struct clk_div_regs  clk_switch;
	struct clk_div_regs  clk_div_gate;
	struct clk_div_regs  clk_div;
	struct clk_gate_regs clk_gate;
};

/* Only used for NPU private clock when need config freq */
struct ppll_conf {
	void __iomem *base;
	const char *clk_name;
	unsigned int ctrl_offset[2];
	unsigned int cur_freq; /* kHz */
	/* freq0->[0][0] ctrl0->[0][1] ctrl1->[0][2] */
	unsigned int para[2][3];
};

struct hisi_npu_dvfs_data {
	struct hisi_npu_module_profile *module_profile;
	int module_count;
	struct hisi_npu_module_profile *main_module;
	bool dvfs_enable;
	unsigned int current_freq;
	int tsensor;
	int temp_stat;
	unsigned int test;
	struct ppll_conf ppll;
#ifdef CONFIG_HISI_NPU_SHARE_PERI_BUCK
	struct peri_volt_poll *pvp;
#endif
	unsigned long max_dvfs_time;
};

enum {
	TEMP_ERROR = -1,
	TEMP_NORMAL = 0,
	TEMP_LOW_ENTER,
	TEMP_LOW,
	TEMP_LOW_EXIT,
	TEMP_MAX
};

enum {
	NPU_LOW_TEMP_DEBUG,
	NPU_LOW_TEMP_STATE
};

struct clk_div_regs_dt {
	unsigned int cfg_offset;
	unsigned int stat_offset;
	unsigned int rd_mask;
	unsigned int wr_mask;
};
#define CLK_DIV_ELEM_NUM	(sizeof(struct clk_div_regs_dt) / sizeof(unsigned int))

struct clk_gate_regs_dt {
	unsigned int en_offset;
	unsigned int dis_offset;
	unsigned int stat_offset;
	unsigned int mask;
};
#define CLK_GATE_ELEM_NUM	(sizeof(struct clk_gate_regs_dt) / sizeof(unsigned int))


struct npu_pm_device {
	struct mutex mutex;
	struct device *dev;
	struct regulator *regulator;
	struct devfreq *devfreq;
	unsigned long last_freq;
	unsigned long target_freq;
	int pm_qos_class;
	bool power_on;
	unsigned int power_on_count;
	unsigned int power_off_count;
	struct hisi_npu_dvfs_data *dvfs_data;
#ifdef CONFIG_DEVFREQ_THERMAL
	struct thermal_cooling_device *devfreq_cooling;
	unsigned long cur_state;
#endif
	unsigned long max_pwron_time;
	unsigned long max_pwroff_time;
};

extern struct npu_pm_device *g_npu_pm_dev;


struct clk *hisi_npu_find_clock(const char *clock_name);
#ifdef CONFIG_HISI_NPU_AVS_VDM
void npu_update_svfd_code(struct hisi_npu_module_profile *module_profile, int target);
int hisi_npu_dt_avs_info_parse(struct device *dev, struct device_node *module_np, struct hisi_npu_module_profile *module_profile, const char *main_module_name);
void hisi_npu_svfd_switch(struct hisi_npu_module_profile *module_profile, unsigned int target, unsigned int onoff);
unsigned int update_avs_temperature_info(void);
unsigned int npu_volt_cfg2vol(unsigned int vol_cfg);
unsigned int npu_cluster_avs(struct hisi_npu_module_profile *module_profile, unsigned int open_switch);
#else
static inline int hisi_npu_dt_avs_info_parse(struct device *dev, struct device_node *module_np, struct hisi_npu_module_profile *module_profile, const char *main_module_name){return 0; }
static inline void npu_update_svfd_code(struct hisi_npu_module_profile *module_profile, int target){}
static inline void hisi_npu_svfd_switch(struct hisi_npu_module_profile *module_profile, unsigned int target, unsigned int onoff){}
static inline unsigned int update_avs_temperature_info(void){return 0; }
static inline unsigned int npu_volt_cfg2vol(unsigned int vol_cfg){return 0; }
#endif

#ifdef CONFIG_HISI_NPUFREQ
int _hal_reg_write(struct clk_div_regs *cfg, unsigned int val);
int _hal_reg_read(struct clk_div_regs *cfg, u32 *val);
int hisi_npu_devfreq_init(struct npu_pm_device *pmdev);
void hisi_npu_devfreq_term(struct npu_pm_device *pmdev);
int hisi_npu_devfreq_suspend(struct devfreq *devfreq);
int hisi_npu_devfreq_resume(struct devfreq *devfreq);
int hisi_npu_recommended_frequency(struct device *dev, unsigned long *target_freq, u32 flags);

unsigned long hisi_npu_get_cur_clk_rate(struct hisi_npu_module_profile *module_profile);
int hisi_npu_get_module_voltage(struct hisi_npu_module_profile *module_profile);
int hisi_npu_set_module_voltage(struct hisi_npu_module_profile *module_profile, unsigned int voltage);

int hisi_npu_profile_hal_init(unsigned long target_freq);
void hisi_npu_profile_hal_exit(void);
int hisi_npu_dvfs_hal(unsigned long target_freq);
int hisi_npu_dvfs_init(struct npu_pm_device *pmdev);
void hisi_npu_dvfs_exit(struct npu_pm_device *pmdev);
#else
static inline int _hal_reg_write(struct clk_div_regs *cfg, unsigned int val){return 0;}
static inline int _hal_reg_read(struct clk_div_regs *cfg, u32 *val){return 0;}

static inline int hisi_npu_devfreq_init(struct npu_pm_device *pmdev){return 0;}
static inline void hisi_npu_devfreq_term(struct npu_pm_device *pmdev){}
static inline int hisi_npu_devfreq_suspend(struct devfreq *devfreq){return 0;}
static inline int hisi_npu_devfreq_resume(struct devfreq *devfreq){return 0;}
static inline int hisi_npu_recommended_frequency(struct device *dev, unsigned long *target_freq, u32 flags){return 0;}

static inline unsigned long hisi_npu_get_cur_clk_rate(struct hisi_npu_module_profile *module_profile){return 0;}
static inline int hisi_npu_get_module_voltage(struct hisi_npu_module_profile *module_profile){return 0;}
int hisi_npu_set_module_voltage(struct hisi_npu_module_profile *module_profile, unsigned int voltage){return 0;}

static inline int hisi_npu_profile_hal_init(unsigned long target_freq){return 0;};
static inline void hisi_npu_profile_hal_exit(void){};
static inline int hisi_npu_dvfs_hal(unsigned long target_freq){return 0;};
static inline int hisi_npu_dvfs_init(struct npu_pm_device *pmdev){return 0;};
static inline void hisi_npu_dvfs_exit(struct npu_pm_device *pmdev){};
#endif

#ifdef CONFIG_HISI_NPU_PM_DEBUG
void hisi_npu_pm_debugfs_init(struct npu_pm_device *pmdev);
void hisi_npu_pm_debugfs_exit(void);
int get_npu_freq_info(void* data, int size, char (*name)[MAX_MODULE_NAME_LEN], int size_name);
#else
static inline void hisi_npu_pm_debugfs_init(struct npu_pm_device *pmdev){};
static inline void hisi_npu_pm_debugfs_exit(void){};
static inline int get_npu_freq_info(void* data, int size, char (*name)[MAX_MODULE_NAME_LEN], int size_name){return 0;};
#endif

#endif /* __HISI_NPU_PM_PRIVATE_H__ */
