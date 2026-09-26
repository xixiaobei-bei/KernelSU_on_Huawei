/*
 * hwevext_dac.h
 *
 * extern dac driver
 *
 * Copyright (c) 2022 Huawei Technologies Co., Ltd.
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

#ifndef __HWEVEXT_ADC_H__
#define __HWEVEXT_ADC_H__

#include <linux/of.h>
#include <sound/soc.h>
#include <linux/regmap.h>

#ifndef unused
#define unused(x) ((void)(x))
#endif

#define hwevext_adc_kfree_ops(p) \
do {\
	if (p != NULL) { \
		kfree(p); \
		p = NULL; \
	} \
} while (0)

#define HWEVEXT_ADC_I2C_ADDR_ARRAY_MAX    0x80 /* i2c addr max == 0x7f */
#define HWEVEXT_ADC_INVALID_PA_INDEX      0xff

#define HWEVEXT_ADC_NAME_MAX       128
#define HWEVEXT_ADC_NAME_INVALID   "none"

/* reg val_bits */
#define HWEVEXT_ADC_REG_VALUE_B8     8  /* val_bits == 8  */
#define HWEVEXT_ADC_REG_VALUE_B16    16 /* val_bits == 16 */
#define HWEVEXT_ADC_REG_VALUE_B24    24 /* val_bits == 24 */
#define HWEVEXT_ADC_REG_VALUE_B32    32 /* val_bits == 32 */

/* reg value mask by reg val_bits */
#define HWEVEXT_ADC_REG_VALUE_M8     0xFF
#define HWEVEXT_ADC_REG_VALUE_M16    0xFFFF
#define HWEVEXT_ADC_REG_VALUE_M24    0xFFFFFF
#define HWEVEXT_ADC_REG_VALUE_M32    0xFFFFFFFF


/* Now, up to only support eight extern adc */
#define HWEVEXT_ADC_ID_MAX     8

/*
 * 0 read reg node:   r-reg-addr | mask | value     | delay | 0
 * 1 write reg node:  w-reg-addr | mask | value     | delay | 1
 * 2 delay node:      0          | 0    | 0         | delay | 2
 * 3 skip node:       0          | 0    | 0         | 0     | 3
 * 4 rxorw node:      w-reg-addr | mask | r-reg-addr| delay | 4
 * 5 dump regs node:  r-reg_begin-addr | r-reg_end-addr | 0| 0 | 5
 *   this mask is xor mask
 */
enum hwevext_dac_reg_ctl_type {
	HWEVEXT_DAC_REG_CTL_TYPE_R = 0,  /* read reg        */
	HWEVEXT_DAC_REG_CTL_TYPE_W,      /* write reg       */
	HWEVEXT_DAC_REG_CTL_TYPE_DELAY,  /* only time delay */
	HWEVEXT_DAC_PARAM_NODE_TYPE_SKIP,
	/* read, xor, write */
	HWEVEXT_DAC_PARAM_NODE_TYPE_REG_RXORW,
	HWEVEXT_DAC_REG_CTL_TYPE_DUMP,
	HWEVEXT_DAC_REG_CTL_TYPE_MAX,
};

enum hwevext_dac_pga_volume_type {
	HWEVEXT_DAC_PGA_VOLUME_L = 0,
	HWEVEXT_DAC_PGA_VOLUME_R,
	HWEVEXT_DAC_PGA_VOLUME_MAX,
};

enum hwevext_dac_kctl_type {
	HWEVEXT_DAC_KCTL_TDM_MODE = 0,
	HWEVEXT_DAC_KCTL_DATA_SEL,
	HWEVEXT_DAC_KCTL_MUTE_CTL,
	HWEVEXT_DAC_KCTL_MAX,
};

struct i2c_err_info {
	unsigned int regs_num;
	unsigned int err_count;
	unsigned long int err_details;
};

struct hwevext_adc_regmap_cfg {
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

struct hwevext_adc_i2c_priv {
	unsigned int chip_id;
	const char *chip_model;
	int probe_completed;

	/* init regs */
	struct hwevext_adc_reg_ctl_sequence *init_regs_seq;

	/* power on regs */
	struct hwevext_adc_reg_ctl_sequence *poweron_regs_seq;

	/* power off regs */
	struct hwevext_adc_reg_ctl_sequence *poweroff_regs_seq;

	/* dump regs */
	struct hwevext_adc_reg_ctl_sequence *dump_regs_sequence;

	/* version regs */
	struct hwevext_adc_reg_ctl_sequence *version_regs_seq;

	/* record wirten regs */
	struct hwevext_adc_reg_ctl_sequence *record_wirten_seq;

	/* pga volume regs */
	struct hwevext_adc_reg_ctl_sequence *pga_volume_seq;

	/* kctl regs */
	struct hwevext_adc_reg_ctl_sequence *kctl_seqs[HWEVEXT_DAC_KCTL_MAX];

	/* reg map config */
	struct hwevext_adc_regmap_cfg *regmap_cfg;

	void *priv_data;
	struct device *dev;
	struct i2c_client *i2c;
};

struct hwevext_adc_reg_info {
	unsigned int chip_id;
	unsigned int reg_addr;
	unsigned int mask;
	unsigned int info;
};

struct hwevext_adc_i2c_dev_info {
	unsigned int chip_id;
	unsigned int addr;
	unsigned int status;
	char chip_model[HWEVEXT_ADC_NAME_MAX];
	struct list_head list;
};

struct hwevext_adc_reg_ctl {
	/* one reg address or reg address_begin of read registers */
	unsigned int addr;
	unsigned int mask;
	union {
		unsigned int value;
		unsigned int chip_version;
		unsigned int min_value;
	};
	unsigned int delay;
	union {
		/* ctl type, default 0(read) */
		unsigned int ctl_type;
		unsigned int max_value;
	};
};

struct hwevext_adc_reg_ctl_sequence {
	unsigned int num;
	struct hwevext_adc_reg_ctl *regs;
};


struct hwevext_adc_i2c_ctl_ops {
	int (*dump_regs)(struct hwevext_adc_i2c_priv *priv);
	int (*power_on)(struct hwevext_adc_i2c_priv *priv);
	int (*power_off)(struct hwevext_adc_i2c_priv *priv);
	int (*write_regs)(struct hwevext_adc_i2c_priv *priv,
			struct hwevext_adc_reg_ctl_sequence *seq);
	int (*read_reg)(struct hwevext_adc_i2c_priv *priv,
			struct hwevext_adc_reg_ctl *reg);
};

struct hwevext_adc_info {
	unsigned int adc_num;
	char chip_model[HWEVEXT_ADC_NAME_MAX];
	char chip_model_list[HWEVEXT_ADC_ID_MAX][HWEVEXT_ADC_NAME_MAX];
};

struct hwevext_adc_priv {
	/* support adc number(now, max == HWEVEXT_ADC_ID_MAX) */
	unsigned int adc_num;
	unsigned int i2c_num; /* load successful i2c module */

	char chip_model_list[HWEVEXT_ADC_ID_MAX][HWEVEXT_ADC_NAME_MAX];
	/*
	 * Here, i2c_addr_to_pa_index array length is 128 bytes
	 * use this array to speed the i2c_ops(r/w) up
	 */
	unsigned char i2c_addr_to_pa_index[HWEVEXT_ADC_I2C_ADDR_ARRAY_MAX];

	unsigned int pag_volume[HWEVEXT_ADC_ID_MAX][HWEVEXT_DAC_PGA_VOLUME_MAX];
	unsigned int power_status[HWEVEXT_ADC_ID_MAX];
	unsigned int kctl_status[HWEVEXT_ADC_ID_MAX][HWEVEXT_DAC_KCTL_MAX];

	bool chip_register_failed;
	/* for i2c ops */
	struct hwevext_adc_i2c_priv *i2c_priv[HWEVEXT_ADC_ID_MAX];
	struct hwevext_adc_i2c_ctl_ops *ioctl_ops;

	struct mutex dump_regs_lock;
	struct mutex i2c_ops_lock; /* regmap  r/w ops lock */
};

#ifdef CONFIG_HWEVEXT_EXTERN_ADC
int hwevext_adc_get_misc_init_flag(void);
int hwevext_adc_register_i2c_device(struct hwevext_adc_i2c_priv *i2c_priv);
int hwevext_adc_deregister_i2c_device(struct hwevext_adc_i2c_priv *i2c_priv);
void hwevext_adc_register_i2c_ctl_ops(struct hwevext_adc_i2c_ctl_ops *ops);
void hwevext_adc_deregister_i2c_ctl_ops(void);
int hwevext_adc_get_prop_of_u32_array(struct device_node *node,
	const char *propname, u32 **value, int *num);
int hwevext_adc_get_prop_of_u32_type(struct device_node *node,
	const char *key_str, unsigned int *type, bool is_requisite);
int hwevext_adc_get_prop_of_str_type(struct device_node *node,
	const char *key_str, const char **dst);
int hwevext_adc_add_kcontrol(struct snd_soc_component *codec);
struct hwevext_adc_priv *hwevext_adc_get_misc_priv(void);
void hwevext_adc_dump_chips(struct hwevext_adc_priv *adc_priv);
#else
static inline int hwevext_adc_get_misc_init_flag(void)
{
	return 0;
}


static inline int hwevext_adc_register_i2c_device(
	struct hwevext_adc_i2c_priv *i2c_priv)
{
	return 0;
}

static inline int hwevext_adc_deregister_i2c_device(
	struct hwevext_adc_i2c_priv *i2c_priv)
{
	return 0;
}

static inline void hwevext_adc_register_i2c_ctl_ops(
	struct hwevext_adc_i2c_ctl_ops *ops)
{
}

static inline void hwevext_adc_deregister_i2c_ctl_ops(void)
{
}

static inline int hwevext_adc_get_prop_of_u32_array(struct device_node *node,
	const char *propname, u32 **value, int *num)
{
	return 0;
}

static inline int hwevext_adc_get_prop_of_u32_type(struct device_node *node,
	const char *key_str, unsigned int *type, bool is_requisite)
{
	return 0;
}

static inline int hwevext_adc_get_prop_of_str_type(struct device_node *node,
	const char *key_str, const char **dst)
{
	return 0;
}

static inline int hwevext_adc_add_kcontrol(struct snd_soc_component *codec)
{
	return 0;
}

static inline struct hwevext_adc_priv *hwevext_adc_get_misc_priv(void)
{
	return NULL;
}

static void hwevext_adc_dump_chips(struct hwevext_adc_priv *adc_priv)
{
}
#endif

#endif // __HWEVEXT_ADC_H__