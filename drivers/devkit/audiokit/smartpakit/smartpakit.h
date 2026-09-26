/*
 * smartpakit.h
 *
 * smartpakit driver
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef __SMARTPAKIT_H__
#define __SMARTPAKIT_H__

#include <linux/list.h>
#include <linux/of.h>
#include <linux/irq.h>
#include <linux/regmap.h>
#include <linux/time.h>
#include <linux/timer.h>
#include "smartpakit_defs.h"

#ifndef unused
#define unused(x) ((void)(x))
#endif

#define smartpakit_kfree_ops(p) \
do {\
	if (p != NULL) { \
		kfree(p); \
		p = NULL; \
	} \
} while (0)

#define SMARTPAKIT_I2C_ADDR_ARRAY_MAX    0x80 /* i2c addr max == 0x7f */
#define SMARTPAKIT_INVALID_PA_INDEX      0xff

/* reg val_bits */
#define SMARTPAKIT_REG_VALUE_B8     8  /* val_bits == 8  */
#define SMARTPAKIT_REG_VALUE_B16    16 /* val_bits == 16 */
#define SMARTPAKIT_REG_VALUE_B24    24 /* val_bits == 24 */
#define SMARTPAKIT_REG_VALUE_B32    32 /* val_bits == 32 */

/* reg value mask by reg val_bits */
#define SMARTPAKIT_REG_VALUE_M8     0xFF
#define SMARTPAKIT_REG_VALUE_M16    0xFFFF
#define SMARTPAKIT_REG_VALUE_M24    0xFFFFFF
#define SMARTPAKIT_REG_VALUE_M32    0xFFFFFFFF

#define SMARTPAKIT_DELAY_US_TO_MS   1000
#define I2C_STATUS_B64 64

struct i2c_err_info {
	unsigned int regs_num;
	unsigned int err_count;
	unsigned long int err_details;
};

struct smartpa_vendor_info {
	unsigned int vendor;
	const char *chip_model;
};

struct smartpakit_gpio_state {
	unsigned int state;
	unsigned int delay;
};

struct smartpakit_gpio_sequence {
	unsigned int num;
	struct smartpakit_gpio_state *node;
};

struct smartpakit_gpio_reset {
	bool not_need_shutdown;
	int gpio;
	char gpio_name[SMARTPAKIT_NAME_MAX];
	struct smartpakit_gpio_sequence sequence;
};

/*
 * 0 read reg node:   read  addr  | count | 0
 * 1 write reg node:  write addr  | value | 1
 * 2 time delay node: 0(not used) | value | 2
 */
enum smartpakit_reg_ctl_type {
	SMARTPAKIT_REG_CTL_TYPE_R = 0,  /* read reg        */
	SMARTPAKIT_REG_CTL_TYPE_W,      /* write reg       */
	SMARTPAKIT_REG_CTL_TYPE_DELAY,  /* only time delay */

	SMARTPAKIT_REG_CTL_TYPE_MAX,
};

struct smartpakit_reg_ctl {
	/* one reg address or reg address_begin of read registers */
	unsigned int addr;

	/*
	 * ctl value
	 * read:  reg number(num >= 1)
	 * write: reg value
	 * delay: time delay
	 */
	unsigned int value;

	/* ctl type, default 0(read) */
	unsigned int ctl_type;
};

struct smartpakit_reg_ctl_sequence {
	unsigned int num;
	struct smartpakit_reg_ctl *regs;
};

struct smartpakit_gpio_irq {
	int gpio;
	int irq;
	unsigned int irqflags;
	bool need_reset;
	bool need_filter_irq;
	char gpio_name[SMARTPAKIT_NAME_MAX];
	char irq_name[SMARTPAKIT_NAME_MAX];
	/* read or write reg sequence, configed in dts */
	struct smartpakit_reg_ctl_sequence *rw_sequence;
};

struct smartpakit_regmap_cfg {
	/* write reg or update_bits */
	unsigned int value_mask;

	/* regmap config */
	int num_writeable;
	int num_unwriteable;
	int num_readable;
	int num_unreadable;
	int num_volatile;
	int num_unvolatile;
	int num_defaults;

	unsigned int *reg_writeable;
	unsigned int *reg_unwriteable;
	unsigned int *reg_readable;
	unsigned int *reg_unreadable;
	unsigned int *reg_volatile;
	unsigned int *reg_unvolatile;
	struct reg_default *reg_defaults;

	struct regmap_config cfg;

	struct regmap *regmap;
};

struct smartpakit_i2c_priv {
	unsigned int chip_vendor;
	unsigned int chip_id;
	const char *chip_model;

	int probe_completed;

	bool sync_irq_debounce_time;

	/* reset */
	unsigned int reset_debounce_wait_time;
	struct smartpakit_gpio_reset *hw_reset;

	/* irq */
	struct smartpakit_gpio_irq *irq_handler;
	struct work_struct irq_handle_work;
#ifdef CONFIG_HUAWEI_ARMPC_PLATFORM
	struct work_struct pm_s4_work;
#endif
	unsigned long irq_debounce_jiffies;
	struct delayed_work irq_debounce_work;

	/* version regs */
	struct smartpakit_reg_ctl_sequence *version_regs_seq;

	/* dump regs */
	struct smartpakit_reg_ctl_sequence *dump_regs_sequence;

	/* hismartpa otp info regs */
	struct smartpakit_reg_ctl_sequence *hismartpa_otp_sequence;

	/* reg map config */
	struct smartpakit_regmap_cfg *regmap_cfg;

	/* i2c_addr_state for hismartpa */
	unsigned int i2c_addr_state;

	unsigned int i2c_pseudo_addr;
	void *priv_data;
	struct device *dev;
	struct i2c_client *i2c;
};

/* param node from hal_driver XXX.xml */
struct smartpakit_param_node {
	/* index: reg addr for smartpa, or gpio index for simple pa */
	unsigned int index;
	unsigned int mask;
	unsigned int value;
	unsigned int delay;
	union {
		unsigned int reserved;
		unsigned int node_type;  /* node type */
	};
};

enum {
	SMARTPA_REGS_FLAG_POWER_OFF = 0,
	SMARTPA_REGS_FLAG_NEED_RESUME,
	SMARTPA_REGS_FLAG_PA_MUTE,
};

struct smartpakit_pa_ctl_sequence {
	/* pa control mask, eg: 0x01, 0x10, 0x11, 0x0011, 0x1100, 0x1111 */
	unsigned int pa_ctl_mask;
	/* 0 poweroff, 1 poweron(need resume), 2 pa mute */
	unsigned int pa_poweron_flag;
	/* current control pa number: < SMARTPAKIT_PA_ID_MAX */
	unsigned int pa_ctl_num;
	/* current control pa index max: < SMARTPAKIT_PA_ID_MAX */
	unsigned int pa_ctl_index_max;
	/* current control pa index array */
	unsigned int pa_ctl_index[SMARTPAKIT_PA_ID_MAX];

	/* param mode number */
	unsigned int param_num;
	unsigned int param_num_of_one_pa;
	struct smartpakit_param_node *node;
};

struct smartpakit_switch_node {
	int gpio;
	int state;
	int delay;
	char name[SMARTPAKIT_NAME_MAX];
};

struct smartpa_regulator_reg_ops {
	int (*spmi_regulator_reg_read)(int i2c_addr, int reg_addr, int *value);
	int (*spmi_regulator_reg_write)(int i2c_addr, int reg_addr, int mask,
		int value);
};

struct smartpakit_reg_info {
	unsigned int chip_id;
	unsigned int reg_addr;
	unsigned int mask;
	/* reg value or config value in config xml */
	unsigned int info;
	struct list_head list;
};

struct i2c_dev_info {
	unsigned int chip_id;
	unsigned int addr;
	unsigned int status;
	char chip_model[SMARTPAKIT_NAME_MAX];
	struct list_head list;
};

struct smartpakit_i2c_ctl_ops {
	int (*hw_reset)(struct smartpakit_i2c_priv *priv);
	int (*dump_regs)(struct smartpakit_i2c_priv *priv);
	int (*read_regs)(struct smartpakit_i2c_priv *priv,
		struct smartpakit_get_param *reg);
	int (*write_regs)(struct smartpakit_i2c_priv *i2c_priv,
		unsigned int num, struct smartpakit_param_node *regs);
	int (*i2c_read)(struct i2c_client *i2c, char *rx_data, int length);
	int (*i2c_write)(struct i2c_client *i2c, char *rx_data, int length);
};

/*
 * 0 reg node:        w-reg-addr | mask | value     | delay | 0
 * 1 gpio node:       gpio-index | 0    | h/l state | delay | 1
 * 2 delay node:      0          | 0    | 0         | delay | 2
 * 3 skip node:       0          | 0    | 0         | 0     | 3
 * 4 rxorw node:      w-reg-addr | mask | r-reg-addr| delay | 4
 *   this mask is xor mask
 * 5 read node:       r-reg-addr | 0    | 0         | 0     | 5
 * 6 irq filter node: i-reg-addr | mask | filter-val| 0     | 6
 *   this mask is filter mask
 * 7 set aw correct   reg_addr |isn_addr| vsn_addr  | 0     | 7
 * 8 irq_debounce     0          | 0    | 0         | 0     | 8
 */
enum smartpakit_param_node_type {
	SMARTPAKIT_PARAM_NODE_TYPE_REG_WRITE,
	SMARTPAKIT_PARAM_NODE_TYPE_GPIO,
	/* only delay time */
	SMARTPAKIT_PARAM_NODE_TYPE_DELAY,
	/*
	 * for split regs to control multi pa(for example: pa_ctl_mask=0x11,
	 * so, the number of regs must be an integer multiple of 2)
	 */
	SMARTPAKIT_PARAM_NODE_TYPE_SKIP,
	/* read, xor, write */
	SMARTPAKIT_PARAM_NODE_TYPE_REG_RXORW,
	SMARTPAKIT_PARAM_NODE_TYPE_REG_READ,
	SMARTPAKIT_PARAM_NODE_TYPE_IRQ_FILTER, /* irq filter reg cfg */
	SMARTPAKIT_PARAM_NODE_TYPE_AW_CORRECT, /* set aw smartpa correction */
	SMARTPAKIT_PARAM_NODE_TYPE_IRQ_DEBOUNCE, /* reset irq debounce time */

	SMARTPAKIT_PARAM_NODE_TYPE_MAX,
};

struct smartpakit_priv {
	unsigned int soc_platform;
	unsigned int algo_in;
	unsigned int algo_delay_time;
	unsigned int out_device;
	unsigned int param_version;
	/* support pa number(now, max == SMARTPAKIT_PA_ID_MAX) */
	unsigned int pa_num;
	unsigned int i2c_num; /* load successful i2c module */
	const char *special_name_config;

	int gpio_of_hw_reset[SMARTPAKIT_PA_ID_MAX];
	int gpio_of_irq[SMARTPAKIT_PA_ID_MAX];
	/* for simple pa(not smart pa) and smartpa(with dsp, plugin) */
	unsigned int chip_vendor;
	const char *chip_model;
	const char *cust;
	const char *product_name;
	char chip_model_list[SMARTPAKIT_PA_ID_MAX][SMARTPAKIT_NAME_MAX];
	unsigned int switch_num;
	struct smartpakit_switch_node *switch_ctl;

	/* smartpa init chip & resume regs sequence */
	bool resume_sequence_permission_enable;
	struct smartpakit_pa_ctl_sequence resume_sequence;
	struct smartpakit_pa_ctl_sequence poweron_seq[SMARTPAKIT_PA_ID_MAX];
	bool force_refresh_chip;
	struct list_head irq_filter_list;

	/* misc device r/w settings */
	bool misc_rw_permission_enable;
	bool misc_i2c_use_pseudo_addr;
	/*
	 * Here, i2c_addr_to_pa_index array length is 128 bytes
	 * use this array to speed the i2c_ops(r/w) up
	 */
	unsigned char i2c_addr_to_pa_index[SMARTPAKIT_I2C_ADDR_ARRAY_MAX];
	struct i2c_client *current_i2c_client;
	struct list_head i2c_dev_list;
	bool chip_register_failed;
	/* for i2c ops */
	struct smartpakit_i2c_priv *i2c_priv[SMARTPAKIT_PA_ID_MAX];
	struct smartpakit_i2c_ctl_ops *ioctl_ops;

	struct mutex irq_handler_lock;
	struct mutex hw_reset_lock;
	struct mutex dump_regs_lock;
	struct mutex i2c_ops_lock; /* regmap or i2c_transfer r/w ops lock */
	struct mutex do_ioctl_lock;
	struct mutex resume_sequence_lock;
	bool need_reset_resume_chip;
	int i2c_errno;
	unsigned int cali_data_update_mode;

	/* pa check loop */
	unsigned int need_pa_check;
	bool timer_state;
	bool loop_state;
	struct workqueue_struct *pa_check_delay_wq;
	struct delayed_work pa_check_delay_work;
	struct timer_list *timer;
};

struct smartpakit_priv *smartpakit_get_misc_priv(void);
int smartpakit_get_misc_init_flag(void);
irqreturn_t smartpakit_i2c_thread_irq(int irq, void *data);

int smartpakit_register_i2c_device(struct smartpakit_i2c_priv *i2c_priv);
int smartpakit_deregister_i2c_device(struct smartpakit_i2c_priv *i2c_priv);
void smartpakit_register_i2c_ctl_ops(struct smartpakit_i2c_ctl_ops *ops);
void smartpakit_deregister_i2c_ctl_ops(void);

int smartpakit_hw_reset_chips(struct smartpakit_priv *pakit_priv);
int smartpakit_resume_chips(struct smartpakit_priv *pakit_priv);
int smartpakit_dump_chips(struct smartpakit_priv *pakit_priv);

int smartpakit_gpio_node_ctl(void *priv, struct smartpakit_param_node *reg);
int smartpakit_get_pa_info_from_dts_config(struct smartpakit_priv *pakit_priv,
	struct smartpakit_info *info);
int smartpakit_set_info(struct smartpa_vendor_info *vendor_info);

int smartpakit_get_prop_of_str_type(struct device_node *node,
	const char *key_str, const char **dst);
int smartpakit_get_prop_of_u32_type(struct device_node *node,
	const char *key_str, unsigned int *type, bool is_requisite);
int smartpakit_get_prop_of_u32_array(struct device_node *node,
	const char *propname, u32 **value, int *num);

void smartpakit_reg_info_add_list(
	struct smartpakit_reg_info *reg_info, struct list_head *pos);
void smartpakit_reg_info_del_list_all(struct list_head *head);

int smartpakit_i2c_get_hismartpa_info(struct smartpakit_i2c_priv *i2c_priv,
	struct hismartpa_coeff *cfg);
void smartpakit_handle_i2c_probe_dsm_report(
	struct smartpakit_priv *pakit_priv);

#ifdef CONFIG_HUAWEI_ARMPC_PLATFORM
	void smartpakit_restore_process(struct work_struct *work);
	int smartpakit_i2c_freeze(struct device *dev);
	int smartpakit_i2c_restore(struct device *dev);
#endif

#endif /* __SMARTPAKIT_H__ */

