/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * hisi_hi6562_pmic_mipi.c
 *
 * Device driver for hi6562 power mipi_rffe
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

#include "hisi_hi6562_pmic_mipi.h"
#include <linux/device.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/regmap.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/mfd/hisi_pmic_mntn.h>
#include <securec.h>

#ifdef CONFIG_HUAWEI_DSM
#include <chipset_common/hwpower/common_module/power_dsm.h>
#endif

#define EX_WRITE_SIGN 0x00
#define EX_READ_SIGN 0x20
#define EX_COMMAND_FRAME 13
#define EX_ADDR_FRAME 8
#define LEN_OF_REG_ADDR_NORMAL 13
#define LEN_OF_REG_ADDR_NORMAL_READ 13
#define LEN_OF_REG_ADDR_NORMAL_WRITE 13
#define LEN_OF_REG_ADDR_EXTERN 22
#define LEN_OF_REG_ADDR_EXTERN_READ 22
#define LEN_OF_REG_ADDR_EXTERN_WRITE 22
#define LEN_DATA 9
#define LEN_USID 4
#define REG_DEFAULT 0x0000
#define BC_DEFAULT 0x0
#define NORMAL_LIMIT 0x1f
#define EXTERN_LIMIT 0xff

#define LEVEL_REVERSAL_STEP 1
#define DELAY_REVERSAL udelay(LEVEL_REVERSAL_STEP);
#define DELAY_WORK_INTERVAL 20

#define VOLT_MIN 0x0008
#define VOLT_MAX 0x006E
#define VOLT_MASK 0x007F
#define VOLT_ENABLE 0x0080
#define VOLT_DEFAULT 0x08

#define DATA_MASK 0x00ff
#define	HI6562_IRQ0 0xC0
#define	HI6562_IRQ1 0xC1
#define HI6562_IRQ0_NORMAL 0x40
#define HI6562_IRQ1_NORMAL 0x0
#define	HI6562_AD_DFT_ST 0xB0
#define	HI6562_IRQ_STATE0 0x9C
#define	HI6562_IRQ_STATE1 0x9D

#define HI6562_DMD_LEN_128 128

#define RFFE_DATA_IO (di->mipi_rffe_data)
#define RFFE_CLK_IO (di->mipi_rffe_clk)

#define RFFE_SDATA_HIGH		do { \
		if (gpio_direction_output(RFFE_DATA_IO, 1)) \
			pr_err("%s, RFFE_DATA_IO set gpio direction output high fail\n", __func__); \
	} while (0)

#define RFFE_SDATA_LOW		do { \
		if (gpio_direction_output(RFFE_DATA_IO, 0)) \
			pr_err("%s, RFFE_DATA_IO set gpio direction output low fail\n", __func__); \
	} while (0)

#define RFFE_SCLK_HIGH		do { \
		if (gpio_direction_output(RFFE_CLK_IO, 1)) \
			pr_err("%s, RFFE_CLK_IO set gpio direction output high fail\n", __func__); \
	} while (0)

#define RFFE_SCLK_LOW		do { \
		if (gpio_direction_output(RFFE_CLK_IO, 0)) \
			pr_err("%s, RFFE_CLK_IO set gpio direction output low fail\n", __func__); \
	} while (0)

#define RFFE_SDATA_INPUT	do { \
		if (gpio_direction_input(RFFE_DATA_IO)) \
			pr_err("%s, rffe_data_io Could not set gpio direction input\n", __func__); \
	} while (0)

#define RFFE_SDATA_READ	gpio_get_value(RFFE_DATA_IO)

#define BUS_PARK RFFE_SDATA_LOW

#define RFFE_SDATA_OUTPUT_MODE RFFE_SDATA_LOW
#define RFFE_SDATA_INPUT_MODE RFFE_SDATA_INPUT
#define RFFE_SCLK_OUTPUT_MODE RFFE_SCLK_LOW

#ifdef CONFIG_HISI_DEBUG_FS

struct hisi_mipi_rffe_dev *hisi_mipi_rffe_g_di = NULL;
static int hi6562_test_work_flag = 0;
static unsigned int write_read_count = 0;

int hi6562_reg_write_test(u16 reg, u16 val)
{
	int ret;

	if (!hisi_mipi_rffe_g_di)
		return -1;

	ret = regmap_write(hisi_mipi_rffe_g_di->regmap, reg, val);
	if (!ret)
		return 0;
	else
		return 1;
}

unsigned int hi6562_reg_read_test(u16 reg)
{
	unsigned int val;

	if (!hisi_mipi_rffe_g_di)
		return 0xff;

	(void)regmap_read(hisi_mipi_rffe_g_di->regmap, reg, &val);
	return val;
}

struct hi6562_reg_test_info {
	u16 reg_addr;
	u16 write_val;
	u16 except_read_back;
};

struct hi6562_reg_test_info reg_test_info[] = {
	{0x01, 0x80, 0x88},
	{0x01, 0x82, 0x88},
	{0x01, 0x84, 0x88},
	{0x01, 0x86, 0x88},
	{0x01, 0x88, 0x88},
	{0x01, 0x90, 0x90},

	{0x01, 0xD0, 0xD0},
	{0x01, 0xD4, 0xD4},
	{0x01, 0xD8, 0xD8},
	{0x01, 0xDC, 0xDC},
	{0x01, 0xE0, 0xE0},
	{0x01, 0xE4, 0xE4},

	{0x01, 0xEE, 0xEE},
	{0x01, 0xF0, 0x88},
};

int hi6562_reg_test_work_ctr(int enable)
{
	hi6562_test_work_flag = enable;

	if (hi6562_test_work_flag)
		queue_delayed_work(system_power_efficient_wq, &hisi_mipi_rffe_g_di->mntn_test_work, msecs_to_jiffies(DELAY_WORK_INTERVAL));
	return hi6562_test_work_flag;
}
static void write_read_count_add()
{
	if (write_read_count == 0xffff)
		write_read_count = 0;
	else
		write_read_count++;
}
static void hi6562_read_write_test_work(struct work_struct *work)
{
	struct hisi_mipi_rffe_dev *di = container_of(work,
											struct hisi_mipi_rffe_dev, mntn_test_work.work);

	int i;
	u16 reg_addr_test;
	u16 write_val_test;
	u16 read_back_test;
	u16 read_back_val;

	hisi_mipi_rffe_g_di = di;
	if (!hisi_mipi_rffe_g_di)
		return;

	if (!hi6562_test_work_flag)
		return;

	for (i = 0; i < (int)ARRAY_SIZE(reg_test_info); i++) {
		reg_addr_test = reg_test_info[i].reg_addr;
		write_val_test = reg_test_info[i].write_val;
		read_back_test = reg_test_info[i].except_read_back;

		(void)hi6562_reg_write_test(reg_addr_test, write_val_test);
		read_back_val = hi6562_reg_read_test(reg_addr_test);

		write_read_count_add();
		pr_info("%s: voltage testing counts %d", __func__, write_read_count);
		if (read_back_val != read_back_test) {
			pr_err("%s: reg_addr = 0x%x, except_data = 0x%x, real_data = 0x%x",
										__func__, reg_addr_test, read_back_test, read_back_val);
			pmic_mntn_panic_handler();
		}
	}
	queue_delayed_work(system_power_efficient_wq, &hisi_mipi_rffe_g_di->mntn_test_work, msecs_to_jiffies(DELAY_WORK_INTERVAL));
}
#endif

static int rffe_io_init_clk(struct hisi_mipi_rffe_dev *di)
{
	int ret;
	ret = gpio_request((di->mipi_rffe_clk), "rffe_clk_io");
	if (ret)
		pr_err("%s, could not request rffe_clk_io\n", __func__);

	ret = gpio_direction_output(di->mipi_rffe_clk, 0);
	if (ret)
		pr_err("%s, rffe_clk_io Could not set gpio direction\n", __func__);
	return ret;
}

static int rffe_io_init_data(struct hisi_mipi_rffe_dev *di)
{
	int ret;
	ret = gpio_request((di->mipi_rffe_data), "rffe_data_io");
	if (ret)
		pr_err("%s, could not request rffe_data_io\n", __func__);

	ret = gpio_direction_output(di->mipi_rffe_data, 0);
	if (ret)
		pr_err("%s, rffe_data_io Could not set gpio direction\n", __func__);
	return ret;
}

static int rffe_io_init(struct hisi_mipi_rffe_dev *di)
{
	int ret;

	if (!di) {
		pr_err("could not find dev, %s failed!", __func__);
		return -1;
	}

	spin_lock_init(&di->buffer_lock);

	if ((di->mipi_rffe_data) == 0 || (di->mipi_rffe_clk) == 0) {
		pr_err("gpio bad number, %s failed!\n", __func__);
		return -1;
	}

	ret = rffe_io_init_clk(di);
	if (ret)
		return ret;
	ret = rffe_io_init_data(di);
	if (ret)
		return ret;
	return 0;
}

static void ssc_head(struct hisi_mipi_rffe_dev *di)
{
	RFFE_SCLK_LOW;
	RFFE_SDATA_LOW;
	DELAY_REVERSAL;

	RFFE_SDATA_HIGH;
	DELAY_REVERSAL;
	DELAY_REVERSAL;

	RFFE_SDATA_LOW;
	DELAY_REVERSAL;
	DELAY_REVERSAL;
}

static int hisi_mipi_rffe_read_data_access(struct hisi_mipi_rffe_dev *di, unsigned int *receive_data, unsigned int *reg_value)
{
	int i;
	unsigned int odd_check_flag;

	/* write ending bus park change to read mode */
	BUS_PARK;
	RFFE_SCLK_HIGH;
	DELAY_REVERSAL;
	RFFE_SCLK_LOW;
	DELAY_REVERSAL;
	RFFE_SDATA_INPUT_MODE;

	/* read operation data */
	odd_check_flag = 1;
	for (i = 0; i < LEN_DATA; i++) {
		RFFE_SCLK_HIGH;
		DELAY_REVERSAL;
		RFFE_SCLK_LOW;
		if (RFFE_SDATA_READ) {
			receive_data[i] = 1;
			odd_check_flag ^= 0x01;
		} else {
			receive_data[i] = 0;
		}
		DELAY_REVERSAL;
	}
	for (i = 0; i < LEN_DATA; i++)
		pr_info("%s  data_index_9bit[%d] = %x \n", __func__, i, receive_data[i]);
	*reg_value = REG_DEFAULT;

	for (i = 0; i < LEN_DATA - 1; i++) {
		*reg_value <<= 1;
		*reg_value |=  receive_data[i] ? 1 : 0;
	}
	pr_info("%s succeeded! reg_value = 0x%x \n", __func__, *reg_value);
	return 0;
}

static void hisi_mipi_rffe_normal_read_address_access(unsigned int usid_address, unsigned int nr_reg_address, unsigned int *read_addr)
{
	int i;
	unsigned int odd_check_flag;

	/* Slave address accessing */
	for (i = 0; i < LEN_USID; i++) {
		if (usid_address & 0x08)
			read_addr[i] = 1;
		else
			read_addr[i] = 0;
		usid_address = usid_address << 1;
	}

	/* 011x normal read sign  reg_address */
	nr_reg_address = nr_reg_address | 0x60;
	for (; i < LEN_OF_REG_ADDR_NORMAL - 1; i++) {
		if (nr_reg_address & 0x80)
			read_addr[i] = 1;
		else
			read_addr[i] = 0;
		nr_reg_address = nr_reg_address << 1;
	}

	/* odd check of address_index 0 - 11 */
	odd_check_flag = 1;
	for (i = 0; i < LEN_OF_REG_ADDR_NORMAL - 1; i++) {
		if (read_addr[i])
			odd_check_flag ^= 0x01;
	}
	read_addr[LEN_OF_REG_ADDR_NORMAL - 1] = odd_check_flag;
}

static int hisi_mipi_rffe_real_read(struct hisi_mipi_rffe_dev *di, unsigned int usid_address, unsigned int reg_address, unsigned int *reg_value)
{
	int i;
	int ret;
	unsigned long flags;
	unsigned int address_index_13bit[LEN_OF_REG_ADDR_NORMAL] = {0};
	unsigned int receive_normal_9bit[LEN_DATA] = {0};

	spin_lock_irqsave(&di->buffer_lock, flags);

	/* Slave address accessing */
	hisi_mipi_rffe_normal_read_address_access(usid_address, reg_address, address_index_13bit);

	/* send message */
	RFFE_SDATA_OUTPUT_MODE;
	ssc_head(di);

	/* write operation addr */
	for (i = 0; i < LEN_OF_REG_ADDR_NORMAL_READ; i++) {
		RFFE_SCLK_HIGH;

		if (address_index_13bit[i])
			RFFE_SDATA_HIGH;
		else
			RFFE_SDATA_LOW;
		DELAY_REVERSAL;
		RFFE_SCLK_LOW;
		DELAY_REVERSAL;
	}

	/* read operation data */
	ret = hisi_mipi_rffe_read_data_access(di, receive_normal_9bit, reg_value);
	pr_info("%s reg_value = 0x%x \n", __func__, *reg_value);

	/* read ending bus park change to write mode */
	DELAY_REVERSAL;
	RFFE_SCLK_HIGH;
	DELAY_REVERSAL;
	RFFE_SCLK_LOW;
	DELAY_REVERSAL;
	RFFE_SDATA_OUTPUT_MODE;
	BUS_PARK;

	spin_unlock_irqrestore(&di->buffer_lock, flags);
	return 0;
}

static void hisi_mipi_rffe_extern_read_address_access(unsigned int usid_address, unsigned int er_reg_address, unsigned int *read_addr)
{
	int i;
	unsigned int odd_check_flag;
	unsigned int ex_read_sign = EX_READ_SIGN + BC_DEFAULT;

	/* Slave address accessing */
	for (i = 0; i < LEN_USID; i++) {
		if (usid_address & 0x08)
			read_addr[i] = 1;
		else
			read_addr[i] = 0;
		usid_address = usid_address << 1;
	}

	/* 0010XXXX extern read typical sign */
	for (; i < EX_COMMAND_FRAME - 1; i++) {
		if (ex_read_sign & 0x80)
			read_addr[i] = 1;
		else
			read_addr[i] = 0;
		ex_read_sign = ex_read_sign << 1;
	}

	/* odd check of address_index 0 - 11 */
	odd_check_flag = 1;
	for (i = 0; i < EX_COMMAND_FRAME - 1; i++) {
		if (read_addr[i])
			odd_check_flag ^= 0x01;
	}
	read_addr[EX_COMMAND_FRAME - 1] = odd_check_flag;

	/* reg_address */
	for (i = EX_COMMAND_FRAME - 1 + EX_ADDR_FRAME; i > EX_COMMAND_FRAME - 1; i--) {
		if (er_reg_address & 0x01)
			read_addr[i] = 1;
		else
			read_addr[i] = 0;
		er_reg_address = er_reg_address >> 1;
	}

	/* odd check of address_index 13 - 20 */
	odd_check_flag = 1;
	for (i = LEN_OF_REG_ADDR_EXTERN - 1 - EX_ADDR_FRAME; i < LEN_OF_REG_ADDR_EXTERN - 1; i++) {
		if (read_addr[i])
			odd_check_flag ^= 0x01;
	}
	read_addr[LEN_OF_REG_ADDR_EXTERN - 1] = odd_check_flag;
}

static int hisi_mipi_rffe_real_extern_read(struct hisi_mipi_rffe_dev *di, unsigned int usid_address, unsigned int reg_address, unsigned int *reg_value)
{
	int i;
	int ret;
	unsigned long flags;
	unsigned int address_index_22bit[LEN_OF_REG_ADDR_EXTERN] = {0};
	unsigned int receive_extern_9bit[LEN_DATA] = {0};

	spin_lock_irqsave(&di->buffer_lock, flags);

	/* Slave address accessing */
	hisi_mipi_rffe_extern_read_address_access(usid_address, reg_address, address_index_22bit);

	/* send message */
	RFFE_SDATA_OUTPUT_MODE;
	ssc_head(di);

	/* write operation addr */
	for (i = 0; i < LEN_OF_REG_ADDR_EXTERN_READ; i++) {
		RFFE_SCLK_HIGH;

		if (address_index_22bit[i])
			RFFE_SDATA_HIGH;
		else
			RFFE_SDATA_LOW;
		DELAY_REVERSAL;
		RFFE_SCLK_LOW;
		DELAY_REVERSAL;
	}

	/* read operation data */
	ret = hisi_mipi_rffe_read_data_access(di, receive_extern_9bit, reg_value);
	pr_info("%s reg_value = 0x%x \n", __func__, *reg_value);

	/* read ending bus park change to write mode */
	DELAY_REVERSAL;
	RFFE_SCLK_HIGH;
	DELAY_REVERSAL;
	RFFE_SCLK_LOW;
	DELAY_REVERSAL;
	RFFE_SDATA_OUTPUT_MODE;
	BUS_PARK;

	spin_unlock_irqrestore(&di->buffer_lock, flags);

	return 0;
}

static void hisi_mipi_rffe_write_data_access(unsigned int reg_data, unsigned int *data)
{
	int i;
	unsigned int odd_check_flag;

	/* data context */
	for (i = 0; i < LEN_DATA - 1; i++) {
		if (reg_data & 0x80)
			data[i] = 1;
		else
			data[i] = 0;
		reg_data = reg_data << 1;
	}

	/* odd check of reg_index 0 - 7 */
	odd_check_flag = 1;
	for (i = 0; i < LEN_DATA - 1; i++) {
		if (data[i])
			odd_check_flag ^= 0x01;
	}
	data[LEN_DATA - 1] = odd_check_flag;
}

static void hisi_mipi_rffe_normal_write_address_access(unsigned int usid_address, unsigned int reg_address, unsigned int *write_addr)
{
	int i;
	unsigned int odd_check_flag;

	/* Slave address accessing */
	for (i = 0; i < LEN_USID; i++) {
		if (usid_address & 0x08)
			write_addr[i] = 1;
		else
			write_addr[i] = 0;
		usid_address = usid_address << 1;
	}

	/* 010x normal read sign  reg_address */
	reg_address = reg_address | 0x40;
	for (; i < LEN_OF_REG_ADDR_NORMAL - 1; i++) {
		if (reg_address & 0x80)
			write_addr[i] = 1;
		else
			write_addr[i] = 0;
		reg_address = reg_address << 1;
	}

	/* odd check of address_index 0 - 11 */
	odd_check_flag = 1;
	for (i = 0; i < LEN_OF_REG_ADDR_NORMAL - 1; i++) {
		if (write_addr[i])
			odd_check_flag ^= 0x01;
	}
	write_addr[LEN_OF_REG_ADDR_NORMAL - 1] = odd_check_flag;
}

static int hisi_mipi_rffe_real_write(struct hisi_mipi_rffe_dev *di, unsigned int usid_address, unsigned int reg_address, unsigned reg_data)
{
	int i;
	unsigned long flags;
	unsigned int address_index_13bit[LEN_OF_REG_ADDR_NORMAL] = {0};
	unsigned int data_normal_9bit[LEN_DATA] = {0};

	spin_lock_irqsave(&di->buffer_lock, flags);

	/* access addr and data */
	hisi_mipi_rffe_normal_write_address_access(usid_address, reg_address, address_index_13bit);
	hisi_mipi_rffe_write_data_access(reg_data, data_normal_9bit);

	/* send message */
	RFFE_SDATA_OUTPUT_MODE;
	ssc_head(di);

	/* write operation addr */
	for (i = 0; i < LEN_OF_REG_ADDR_NORMAL_WRITE; i++) {
		RFFE_SCLK_HIGH;
		if (address_index_13bit[i])
			RFFE_SDATA_HIGH;
		else
			RFFE_SDATA_LOW;

		DELAY_REVERSAL;
		RFFE_SCLK_LOW;
		DELAY_REVERSAL;
	}

	/* write operation data */
	for (i = 0; i < LEN_DATA; i++) {
		RFFE_SCLK_HIGH;
		if (data_normal_9bit[i])
			RFFE_SDATA_HIGH;
		else
			RFFE_SDATA_LOW;

		DELAY_REVERSAL;
		RFFE_SCLK_LOW;
		DELAY_REVERSAL;
	}
	/* log to record potential bug */
	for (i = 0; i < LEN_DATA; i++)
		pr_info("%s write_func data_index_9bit[%d] = %x \n", __func__, i, data_normal_9bit[i]);

	/* ending bus park */
	BUS_PARK;
	RFFE_SCLK_HIGH;
	DELAY_REVERSAL;
	RFFE_SCLK_LOW;
	DELAY_REVERSAL;
	spin_unlock_irqrestore(&di->buffer_lock, flags);

	return 0;
}

static void hisi_mipi_rffe_extern_write_address_access(unsigned int usid_address, unsigned int reg_address, unsigned int *write_addr)
{
	int i;
	unsigned int odd_check_flag;
	unsigned int ex_write_sign = EX_WRITE_SIGN + BC_DEFAULT;

	/* Slave address accessing */
	for (i = 0; i < LEN_USID; i++) { // 0 - 4 is usid addr
		if (usid_address & 0x08)
			write_addr[i] = 1;
		else
			write_addr[i] = 0;
		usid_address = usid_address << 1;
	}

	/* 0000XXXX extern read typical sign */
	for (; i < EX_COMMAND_FRAME - 1; i++) {
		if (!(ex_write_sign & 0x80))
			write_addr[i] = 0;
		ex_write_sign = ex_write_sign << 1;
	}

	/* odd check of address_index 0 - 11 */
	odd_check_flag = 1;
	for (i = 0; i < EX_COMMAND_FRAME - 1; i++) {
		if (write_addr[i])
			odd_check_flag ^= 0x01;
	}
	write_addr[EX_COMMAND_FRAME - 1] = odd_check_flag;

	/* reg_address 13 - 20 */
	for (i = EX_COMMAND_FRAME - 1 + EX_ADDR_FRAME; i > EX_COMMAND_FRAME - 1; i--) {
		if (reg_address & 0x01)
			write_addr[i] = 1;
		else
			write_addr[i] = 0;
		reg_address = reg_address >> 1;
	}

	/* odd check of address_index 13 - 20 */
	odd_check_flag = 1;
	for (i = LEN_OF_REG_ADDR_EXTERN - 1 - EX_ADDR_FRAME; i < LEN_OF_REG_ADDR_EXTERN - 1; i++) {
		if (write_addr[i])
			odd_check_flag ^= 0x01;
	}
	write_addr[LEN_OF_REG_ADDR_EXTERN - 1] = odd_check_flag;
}

static int hisi_mipi_rffe_real_extern_write(struct hisi_mipi_rffe_dev *di, unsigned int usid_address, unsigned int reg_address, unsigned int reg_data)
{
	int i;
	unsigned long flags;
	unsigned int address_index_22bit[LEN_OF_REG_ADDR_EXTERN] = {0};
	unsigned int data_extern_9bit[LEN_DATA] = {0};

	spin_lock_irqsave(&di->buffer_lock, flags);

	/* access addr and data */
	hisi_mipi_rffe_extern_write_address_access(usid_address, reg_address, address_index_22bit);
	hisi_mipi_rffe_write_data_access(reg_data, data_extern_9bit);

	/* send message */
	RFFE_SDATA_OUTPUT_MODE;
	ssc_head(di);

	/* write oeration addr */
	for (i = 0; i < LEN_OF_REG_ADDR_EXTERN_WRITE; i++) {
		RFFE_SCLK_HIGH;
		if (address_index_22bit[i])
			RFFE_SDATA_HIGH;
		else
			RFFE_SDATA_LOW;
		DELAY_REVERSAL;
		RFFE_SCLK_LOW;
		DELAY_REVERSAL;
	}

	/* write operation data */
	for (i = 0; i < LEN_DATA; i++) {
		RFFE_SCLK_HIGH;
		if (data_extern_9bit[i])
			RFFE_SDATA_HIGH;
		else
			RFFE_SDATA_LOW;

		DELAY_REVERSAL;
		RFFE_SCLK_LOW;
		DELAY_REVERSAL;
	}

	/* log to record potential bug */
	for (i = 0; i < LEN_DATA; i++)
		pr_info("%s ex_write_func data_index_9bit[%d] = %x \n", __func__, i, data_extern_9bit[i]);

	/* write ending bus park */
	BUS_PARK;
	RFFE_SCLK_HIGH;
	DELAY_REVERSAL;
	RFFE_SCLK_LOW;
	DELAY_REVERSAL;
	spin_unlock_irqrestore(&di->buffer_lock, flags);

	return 0;
}

static int hisi_mipi_rffe_read_api_regmap(void *context, unsigned int reg_address, unsigned int *p_reg_data)
{
	int ret;
	unsigned int usid;
	struct hisi_mipi_rffe_dev *di = (struct hisi_mipi_rffe_dev *)context;

	if (!di) {
		pr_err("could not find dev, %s failed!", __func__);
		return -1;
	}

	usid = di->usid_addr;
	if (reg_address <= NORMAL_LIMIT) {
		ret = hisi_mipi_rffe_real_read(di, usid, reg_address, p_reg_data);
		pr_info("hisi_mipi_rffe_read_api_regmap ret = %d\n", ret);
		return ret;
	} else if (reg_address <= EXTERN_LIMIT) {
		ret = hisi_mipi_rffe_real_extern_read(di, usid, reg_address, p_reg_data);
		pr_info("hisi_mipi_rffe_read_api_regmap ret = %d\n", ret);
		return ret;
	} else {
		ret = -1;
		pr_err("%s the address is out of range!!!", __func__);
		return ret;
	}
}

static int is_read_enable_reg(unsigned int reg_address)
{
	if (reg_address == 0x01)
		return 1;
	return 0;
}

static int is_volt_of_out_range(unsigned int reg_data)
{
	if (((reg_data & VOLT_MASK) < VOLT_MIN) || ((reg_data & VOLT_MASK)  > VOLT_MAX))
		return 1;
	return 0;
}

static int is_volt_open(unsigned int reg_address, unsigned int reg_data)
{
	if ((reg_address == 0x01) && ((reg_data & VOLT_ENABLE) == VOLT_ENABLE))
		return 1;
	return 0;
}

static int hisi_mipi_rffe_write_api_regmap(void *context, unsigned int reg_address, unsigned int reg_data)
{
	int ret;
	unsigned int usid;
	struct hisi_mipi_rffe_dev *di = (struct hisi_mipi_rffe_dev *)context;
	if (!di) {
		pr_err("could not find dev, %s failed!", __func__);
		return -1;
	}
	usid = di->usid_addr;

	pr_info("%s, the reg_data is 0x%x", __func__, reg_data);
	if (is_read_enable_reg(reg_address) && is_volt_of_out_range(reg_data)) {
		pr_err("%s: the reg data is out of range! the reg data will be reset", __func__);
		reg_data = (reg_data & VOLT_ENABLE) + VOLT_DEFAULT;
	}

	if (is_volt_open(reg_address, reg_data)) {
		pr_info("%s: activate delayed work", __func__);
		queue_delayed_work(system_power_efficient_wq, &di->mntn_work, msecs_to_jiffies(DELAY_WORK_INTERVAL));
	}

	if (reg_address <= NORMAL_LIMIT) {
		ret = hisi_mipi_rffe_real_write(di, usid, reg_address, reg_data);
		return ret;
	} else if (reg_address <= EXTERN_LIMIT) {
		ret = hisi_mipi_rffe_real_extern_write(di, usid, reg_address, reg_data);
		return ret;
	} else {
		ret = -1;
		pr_err("the address is out of range!!!");
		return ret;
	}
}

static const struct regmap_config hisi_mipi_rffe_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.reg_read = hisi_mipi_rffe_read_api_regmap,
	.reg_write = hisi_mipi_rffe_write_api_regmap,
};

static int hisi_mipi_rffe_parse_dts_gpio(struct hisi_mipi_rffe_dev *di)
{
	int ret;
	struct device_node *np = NULL;

	if (!di || !di->dev)
		return -1;

	np = di->dev->of_node;
	if (!np)
		return -1;

	ret = of_property_read_u32(np, "gpio_data",
				   (u32 *)&(di->mipi_rffe_data));
	if (ret) {
		pr_err("%s: mipi_rffe_data_get_fail\n", __func__);
		return ret;
	}

	ret = of_property_read_u32(np, "gpio_clk",
				   (u32 *)&(di->mipi_rffe_clk));
	if (ret) {
		pr_err("%s: mipi_rffe_clk_get_fail\n", __func__);
		return ret;
	}

	return 0;
}

static int hisi_mipi_rffe_parse_dts_opt_reg(struct hisi_mipi_rffe_dev *di)
{
	int ret;
	struct device_node *np = NULL;

	if (!di || !di->dev)
		return -1;

	np = di->dev->of_node;
	if (!np)
		return -1;

	/* read one np name opt regs */
	ret = of_property_read_u32(np, "opt-regs-num",
				   &di->opt_regs_num);
	if (ret) {
		dev_err(di->dev, "no opt-regs-num property set\n");
		return ret;
	}

	if (di->opt_regs_num == 0) {
		dev_info(di->dev, "no opt-regs-data need inital\n");
		return 0;
	}

	/* read array opt data */
	di->opt_regs_addr = devm_kzalloc(
		di->dev, sizeof(unsigned int) * di->opt_regs_num, GFP_KERNEL);
	if (!di->opt_regs_addr) {
		ret = -ENOMEM;
		dev_err(di->dev, "no memory for opt_regs_addr\n");
		return ret;
	}

	di->opt_regs_val = devm_kzalloc(
		di->dev, sizeof(unsigned int) * di->opt_regs_num, GFP_KERNEL);
	if (!di->opt_regs_val) {
		ret = -ENOMEM;
		dev_err(di->dev, "no memory for opt_regs_val\n");
		return ret;
	}

	ret = of_property_read_u32_array(np, "opt-regs-addr",
						di->opt_regs_addr, di->opt_regs_num);
	if (ret) {
		dev_err(di->dev, "no opt-regs-addr property set\n");
		devm_kfree(di->dev, di->opt_regs_addr);
		return ret;
	}

	ret = of_property_read_u32_array(np, "opt-regs-val",
						di->opt_regs_val, di->opt_regs_num);
	if (ret) {
		dev_err(di->dev, "no opt-regs-val property set\n");
		devm_kfree(di->dev, di->opt_regs_val);
		return ret;
	}

	return 0;
}

static hisi_mipi_rffe_parse_dts_usid(struct hisi_mipi_rffe_dev *di)
{
	int ret;
	struct device_node *np = NULL;

	if (!di || !di->dev)
		return -1;

	np = di->dev->of_node;
	if (!np)
		return -1;

	ret = of_property_read_u32(np, "usid-addr",
				   (u32 *)&(di->usid_addr));
	if (ret) {
		dev_err(di->dev, "no usid-addr property set\n");
		return ret;
	}

	return 0;
}

static int hisi_mipi_rffe_parse_dts(struct hisi_mipi_rffe_dev *di)
{
	int ret;

	ret = hisi_mipi_rffe_parse_dts_gpio(di);
	if (ret) {
		dev_err(di->dev, "%s: parse dts gpio failed \n", __func__);
		return ret;
	}

	ret = hisi_mipi_rffe_parse_dts_opt_reg(di);
	if (ret) {
		dev_err(di->dev, "%s: parse dts opt reg failed \n", __func__);
		return ret;
	}

	ret = hisi_mipi_rffe_parse_dts_usid(di);
	if (ret) {
		dev_err(di->dev, "%s: parse dts usid failed \n", __func__);
		return ret;
	}

	return 0;
}

static int hi6562_opt_regs_init(struct hisi_mipi_rffe_dev *di)
{
	u32 i;
	int ret;

	for (i = 0; i < di->opt_regs_num; i++) {
		ret = regmap_write(di->regmap, (di->opt_regs_addr)[i], (di->opt_regs_val)[i]);
		if (ret) {
			pr_err("%s error, i %d, addr 0x%x, val 0x%x\n",
				__func__, i, (di->opt_regs_addr)[i], (di->opt_regs_val)[i]);
			return ret;
		}
	}

	return ret;
}

static int is_reg_irq_error(unsigned int reg_irq0, unsigned int reg_irq1)
{
	if ((reg_irq0 & DATA_MASK) != HI6562_IRQ0_NORMAL || (reg_irq1 & DATA_MASK) != HI6562_IRQ1_NORMAL)
		return 1;
	return 0;
}

static void hi6562_dsm_report(const char *info_str)
{
#ifdef CONFIG_HUAWEI_DSM
	if (!dsm_client_ocuppy(power_dsm_get_dclient(POWER_DSM_PMU_OCP))) {
		dsm_client_record(power_dsm_get_dclient(POWER_DSM_PMU_OCP),
			"pmu %s happened, please pay attention!\n",
			info_str);
		dsm_client_notify(power_dsm_get_dclient(POWER_DSM_PMU_OCP),
			DSM_PMU_EXP);
	}
#endif
	pr_err("[PMU:%s] %s happened\n", __func__, info_str);
}

static void hi6562_mntn_delay_work(struct work_struct *work)
{
	struct hisi_mipi_rffe_dev *di = container_of(work,
		struct hisi_mipi_rffe_dev, mntn_work.work);

	int ret;
	unsigned int reg_irq0;
	unsigned int reg_irq1;
	unsigned int reg_ad_dft_st;
	unsigned int reg_irq_state0;
	unsigned int reg_irq_state1;
	char log_str[HI6562_DMD_LEN_128] = {0};

	ret = hisi_mipi_rffe_real_extern_read(di, di->usid_addr, HI6562_IRQ0, &reg_irq0);
	if (ret) {
		dev_err(di->dev, "%s: cant read reg_irq0 \n", __func__);
		return;
	}
	ret = hisi_mipi_rffe_real_extern_read(di, di->usid_addr, HI6562_IRQ1, &reg_irq1);
	if (ret) {
		dev_err(di->dev, "%s: cant read reg_irq1 \n", __func__);
		return;
	}

	if (is_reg_irq_error(reg_irq0, reg_irq1)) {
		hisi_mipi_rffe_real_extern_read(di, di->usid_addr, HI6562_AD_DFT_ST, &reg_ad_dft_st);
		hisi_mipi_rffe_real_extern_read(di, di->usid_addr, HI6562_IRQ_STATE0, &reg_irq_state0);
		hisi_mipi_rffe_real_extern_read(di, di->usid_addr, HI6562_IRQ_STATE1, &reg_irq_state1);
		pr_err("ERROR: 0xc0 = 0x%x, 0xc1 = 0x%x, 0xb0 = 0x%x, 0x9c = 0x%x, 0x9d = 0x%x",
			reg_irq0, reg_irq1, reg_ad_dft_st, reg_irq_state0, reg_irq_state1);

		hisi_mipi_rffe_real_extern_write(di, di->usid_addr, HI6562_IRQ0, 0xff);
		hisi_mipi_rffe_real_extern_write(di, di->usid_addr, HI6562_IRQ1, 0xff);

		hisi_mipi_rffe_real_extern_read(di, di->usid_addr, HI6562_IRQ0, &reg_irq0);
		hisi_mipi_rffe_real_extern_read(di, di->usid_addr, HI6562_IRQ1, &reg_irq1);
		pr_err("CLEAR: 0xc0 = 0x%x, 0xc1 = 0x%x", reg_irq0, reg_irq1);

		ret = snprintf_s(log_str, HI6562_DMD_LEN_128, HI6562_DMD_LEN_128 - 1,
			"ERROR: 0xc0 = 0x%x, 0xc1 = 0x%x, 0xb0 = 0x%x, 0x9c = 0x%x, 0x9d = 0x%x",
			reg_irq0, reg_irq1, reg_ad_dft_st, reg_irq_state0, reg_irq_state1);
		if (ret == -1) {
			pr_err("%s: dmd log_str fetch err!", __func__);
			return;
		}
		pr_err("%s: dmd log_str fetch success!", __func__);
		hi6562_dsm_report(log_str);
	}

	return;
}

static int hisi_mipi_rffe_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct hisi_mipi_rffe_dev *di = NULL;
	int ret;

	struct device_node *np = NULL;

	di = devm_kzalloc(dev, sizeof(*di), GFP_KERNEL);
	if (!di) {
		dev_err(dev, "RAM error\n");
		ret = -1;
		goto mipi_rffe_err;
	}

	di->dev = dev;
	platform_set_drvdata(pdev, di);

	ret = hisi_mipi_rffe_parse_dts(di);
	if (ret) {
		dev_err(di->dev, "dts error\n");
		goto mipi_rffe_err;
	}

	di->regmap = devm_regmap_init(dev, NULL, di, &hisi_mipi_rffe_regmap_config);
	if (IS_ERR(di->regmap)) {
		ret = PTR_ERR(di->regmap);
		dev_err(di->dev, "regmap init failed %d\n", ret);
		return ret;
	}

	rffe_io_init(di);
	hi6562_opt_regs_init(di);

	np = di->dev->of_node;
	ret = of_platform_populate(np, NULL, NULL, di->dev);
	if (ret) {
		dev_info(di->dev, "%s populate fail %d\n", __func__, ret);
		goto mipi_rffe_err;
	}

	INIT_DELAYED_WORK(&di->mntn_work, hi6562_mntn_delay_work);

#ifdef CONFIG_HISI_DEBUG_FS
	hisi_mipi_rffe_g_di = di;
	pr_info("%s mipi_rffe debug work", __func__);
	INIT_DELAYED_WORK(&di->mntn_test_work, hi6562_read_write_test_work);
#endif

	pr_err("%s mipi_rffe_work_successfully\n", __func__);
	return 0;

mipi_rffe_err:
	pr_err("ERROR: failed to connect the mipi rffe\n");
	platform_set_drvdata(pdev, NULL);
	return ret;
}

static int hisi_mipi_rffe_remove(struct platform_device *pdev)
{
	struct hisi_mipi_rffe_dev *di = platform_get_drvdata(pdev);

	if (!di) {
		pr_err("%s MIPI di is NULL\n", __func__);
		return -1;
	}
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	return 0;
}

static const struct of_device_id of_hisi_mipi_rffe_match_tbl[] = {
	{.compatible = "hisilicon,hi6562-pmic-mipi"},
	{ }
};

MODULE_DEVICE_TABLE(of, of_hisi_mipi_rffe_match_tbl);

static struct platform_driver hisi_mipi_rffe_driver = {
	.driver = {
			.name = "hisi_hi6562_pmic_mipi",
			.owner = THIS_MODULE,
			.of_match_table = of_hisi_mipi_rffe_match_tbl,
		},
	.probe = hisi_mipi_rffe_probe,
	.remove = hisi_mipi_rffe_remove,
};

static int __init hisi_mipi_rffe_init(void)
{
	return platform_driver_register(&hisi_mipi_rffe_driver);
}

static void __exit hisi_mipi_rffe_exit(void)
{
	platform_driver_unregister(&hisi_mipi_rffe_driver);
}

module_init(hisi_mipi_rffe_init);

module_exit(hisi_mipi_rffe_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("hisi_hi6562_pmic_mipi RFFE driver");
MODULE_ALIAS("mipi_rffe io for power source control");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
