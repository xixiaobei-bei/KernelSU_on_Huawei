/*
 * onewire_common.h
 *
 * onewire head file for all physic controller
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#ifndef _ONEWIRE_COMMON_H_
#define _ONEWIRE_COMMON_H_

#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timex.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/of_gpio.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#include <asm/arch_timer.h>
#include <generated/uapi/linux/version.h>

#include <hisi_gpio.h>
#include <huawei_platform/log/hw_log.h>

/* PL061 */
#define PL061_DIR_OFFSET                    0x400
#define PL061_DATA_OFFSET                   2

/* device tree bind */
#define GPIO_CHIP_PHANDLE_INDEX             0
#define GPIO_REG_PROPERTY_SIZE              4
#define ONEWIRE_GPIO_OFFSET_INDEX           1
#define ADDRESS_HIGH32BIT                   0
#define ADDRESS_LOW32BIT                    1
#define LENGTH_HIGH32BIT                    2
#define LENGTH_LOW32BIT                     3
#define SHIFT_32                            32
#define GPIO_INDEX                          0

struct ow_gpio_des {
	unsigned long gpio_phy_base_addr;
	unsigned long length;
	unsigned int offset;
	raw_spinlock_t *lock;
	void *gpio_data_addr;
	void *gpio_dir_addr;
	unsigned char gpio_dir_out_data;
	unsigned char gpio_dir_in_data;
};

/* onewire common interface */
struct ow_treq {
	unsigned int reset_init_low_ns;
	unsigned int reset_init_low_cycles;
	unsigned int reset_slave_response_delay_ns;
	unsigned int reset_slave_response_delay_cycles;
	unsigned int reset_hold_high_ns;
	unsigned int reset_hold_high_cycles;
	unsigned int write_init_low_ns;
	unsigned int write_init_low_cycles;
	unsigned int write_hold_ns;
	unsigned int write_hold_cycles;
	unsigned int write_residual_ns;
	unsigned int write_residual_cycles;
	unsigned int read_init_low_ns;
	unsigned int read_init_low_cycles;
	unsigned int read_residual_ns;
	unsigned int read_residual_cycles;
};

struct ow_treq_v2 {
	unsigned int reset_low_ns;
	unsigned int reset_low_cycs;
	unsigned int reset_us;
	unsigned int write_low_ns;
	unsigned int write_low_cycs;
	unsigned int write_high_ns;
	unsigned int write_high_cycs;
	unsigned int write_bit_ns;
	unsigned int write_bit_cycs;
	unsigned int read_sample_ns;
	unsigned int read_sample_cycs;
	unsigned int read_rest_ns;
	unsigned int read_rest_cycs;
	unsigned int stop_low_ns;
	unsigned int stop_low_cycs;
	unsigned int stop_high_ns;
	unsigned int stop_high_cycs;
	unsigned int soft_reset_ns;
	unsigned int soft_reset_cycs;
};

typedef unsigned char (*ow_reset)(void);
typedef unsigned char (*ow_read_byte)(void);
struct ow_phy_ops {
	ow_reset reset;
	ow_read_byte read_byte;
	void (*write_byte)(const unsigned char val);
	void (*set_time_request)(struct ow_treq *trq);
	void (*wait_for_ic)(unsigned int ms);
};

struct ow_phy_ops_v2 {
	void (*reset)(struct ow_gpio_des *des, struct ow_treq_v2 *trq);
	unsigned char (*start)(const unsigned char end, struct ow_gpio_des *des,
			       struct ow_treq_v2 *trq);
	unsigned char (*read_byte)(const unsigned char end,
				   struct ow_gpio_des *gpio,
				   struct ow_treq_v2 *trq,
				   unsigned char *val);
	unsigned char (*write_byte)(const unsigned char end,
				    const unsigned char val,
				    struct ow_gpio_des *gpio,
				    struct ow_treq_v2 *trq);
	void (*wait_for_ic)(unsigned int ms);
	void (*soft_reset)(struct ow_gpio_des *gpio, struct ow_treq_v2 *trq);
};

struct ow_phy_list {
	struct list_head node;
	struct ow_gpio_des *gpio;
	phandle dev_phandle;
	int (*onewire_phy_register)(struct ow_phy_ops *ops);
	int (*onewire_phy_register_v2)(struct ow_phy_ops_v2 *ops);
};

#define LOW_VOLTAGE                         0
#define HIGH_VOLTAGE                        1

#define SHIFT_0                             0
#define SHIFT_1                             1
#define SHIFT_2                             2
#define SHIFT_3                             3
#define SHIFT_4                             4
#define SHIFT_5                             5
#define SHIFT_6                             6
#define SHIFT_7                             7

#define BIT_0                               0x01
#define BIT_1                               0x02
#define BIT_2                               0x04
#define BIT_3                               0x08
#define BIT_4                               0x10
#define BIT_5                               0x20
#define BIT_6                               0x40
#define BIT_7                               0x80

#define FIRST_TIME_PROPERTY                 0
#define SECOND_TIME_PROPERTY                1
#define THIRD_TIME_PROPERTY                 2

#define ONEWIRE_PHY_SUCCESS                 0
#define ONEWIRE_GPIO_FAIL                   1
#define ONEWIRE_PHY_MATCH_FAIL              2

#define ONEWIRE_ILLEGAL_PARAM                   10
#define ONEWIRE_CRC16_FAIL                      9
#define ONEWIRE_CRC8_FAIL                       8
#define ONEWIRE_REGIST_FAIL                     7
#define ONEWIRE_OPS_REG_FAIL                    6
#define ONEWIRE_NO_SLAVE                        5
#define ONEWIRE_DTS_FAIL                        4
#define ONEWIRE_NULL_INPARA                     3
#define ONEWIRE_MEM_FAIL                        2
#define ONEWIRE_COM_FAIL                        1
#define ONEWIRE_SUCCESS                         0

#define FIND_IC_RETRY_NUM                       1

/*
 *This macro is arch related. Transform ns to timer cycles.
 *X should not larger than 10^6.
 *0x10c7 is round up of 2^32 / 10^6.
 *0x5 is round up of 2^32 / 10^9.
 *1 << 31 is round up for cycles.
 *Using this macro you should also calculate the
 *cycles by yourself if the result is same as you hoped.
 *For kirin970 loops_per_jiffy * HZ is 1920000.
 */
#define ns2cycles(X)                                                      \
(((((X) / 1000 * 0x10C7UL + (X) % 1000 * 0x5UL) * loops_per_jiffy * HZ) + \
(1UL << 31)) >> 32)

void __hw_delay(unsigned long cycles);
void __hw_delay_v2(unsigned long cycles);
/* Do not use api below if you can't make sure use it right */
void gpio_direction_output_unsafe(unsigned char value,
				  struct ow_gpio_des *gpio);
/* Do not use api below if you can't make sure using it right */
void gpio_direction_input_unsafe(struct ow_gpio_des *gpio);
/* Do not use api below if you can't make sure using it right */
void gpio_set_value_unsafe(unsigned char value, struct ow_gpio_des *gpio);
/* Do not use api below if you can't make sure using it right */
unsigned char gpio_get_value_unsafe(struct ow_gpio_des *gpio);
/* Do not use api below if you can't make sure using it right */
void get_current_gpio_bank_dir(struct ow_gpio_des *gpio);
int get_gpio_phy_addr(struct platform_device *pdev, struct ow_gpio_des *gpio);
int init_gpio_iomem_addr(struct platform_device *pdev,
			 struct ow_gpio_des *gpio);
int get_gpio_for_ow(struct platform_device *pdev, struct ow_gpio_des *gpio,
		    const char *label);
void add_to_onewire_phy_list(struct ow_phy_list *entry);
struct list_head *get_owi_phy_list_head(void);

#endif /* _ONEWIRE_COMMON_H_ */
