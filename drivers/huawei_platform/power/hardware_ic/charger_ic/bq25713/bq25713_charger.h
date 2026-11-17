/*
 * bq25713_charger.h
 *
 * bq25713 driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _BQ25713_CHARGER_H_
#define _BQ25713_CHARGER_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/workqueue.h>
#include <linux/bitops.h>
#include <linux/pm_wakeup.h>
#include <linux/usb/otg.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/power_supply.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/raid/pq.h>

struct bq25713_adc_info {
	int ibus;
	unsigned int vbat;
	unsigned int vbus;
};

struct bq25713_device_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct delayed_work adc_work;
	struct bq25713_adc_info adc_value;
	struct wakeup_source wlock;
	struct mutex adc_lock;
	int gpio_int;
	int gpio_otg;
	int irq_int;
	int irq_active;
	int adc_mode;
	int term_curr;
	int iterm_pre;
	int en_last;
	int vterm;
	int iin_set;
	int chg_done_cnt;
	int support_dc;
	bool chg_done;
};

#define BQ25713_REG_NUM                       60
#define BQ25713_ITERM_MIN                     0
#define BQ25713_ITERM_MAX                     1024
#define BQ25713_RET_SIZE                      4
#define BQ25713_CHG_DONE_CNT                  2
#define BQ25713_READ_TIMES                    5
#define BQ25713_VTERM_ACC                     36
#define BQ25713_ITERM_DEFAULT                 128
#define BQ25713_BUF_LEN                       26
#define BQ25713_CONV_RETRY_TIMES              8
#define BQ25713_GET_ADC_TIME_MS               1000
#define BQ25713_TIMEOUT_MS                    100
#define BQ25713_VRECHG_DEFAULT                150
#define BQ25713_CC_VOLTAGE                    13000
#define BQ25713_VBAT_OVP_TH                   13460

/* charger control 0 */
#define BQ25713_REG_CC0_LOW                   0x00
#define BQ25713_REG_CC0_EN_CHG_MASK           BIT(0)
#define BQ25713_REG_CC0_EN_CHG_SHIFT          0
#define BQ25713_REG_CC0_HIGH                  0x01
#define BQ25713_REG_CC0_WATCHDOG_MASK         (BIT(6) | BIT(5))
#define BQ25713_REG_CC0_WATCHDOG_SHIFT        5
#define BQ25713_WATCHDOG_SHUTDOWN             0
#define BQ25713_WATCHDOG_TIMER_SHORT          2
#define BQ25713_WATCHDOG_TIMER_LONG           3

/* charger control 2 */
#define BQ25713_REG_CC2_LOW                   0x32
#define BQ25713_REG_CC2_EN_EXTILIM_MASK       BIT(7)
#define BQ25713_REG_CC2_EN_EXTILIM_SHIFT      7

/* charger control 3 */
#define BQ25713_REG_CC3_LOW                   0x34
#define BQ25713_REG_CC3_HIGH                  0x35
#define BQ25713_REG_CC3_EN_HIZ_MASK           BIT(7)
#define BQ25713_REG_CC3_EN_HIZ_SHIFT          7
#define BQ25713_REG_CC3_OTG_VAP_MODE_MASK     BIT(5)
#define BQ25713_REG_CC3_OTG_VAP_MODE_SHIFT    5
#define BQ25713_REG_CC3_EN_OTG_MASK           BIT(4)
#define BQ25713_REG_CC3_EN_OTG_SHIFT          4
#define BQ25713_REG_CC3_OTG_RANGE_LOW_MASK    BIT(2)
#define BQ25713_REG_CC3_OTG_RANGE_LOW_SHIFT   2

/* charger current limit */
#define BQ25713_REG_CCL                       0x02
#define BQ25713_ICHG_MAX                      8128
#define BQ25713_ICHG_MIN                      64

/* battery voltage regulation limit */
#define BQ25713_REG_TERM_VOL                  0x04
#define BQ25713_VCHARGE_MIN                   1024
#define BQ25713_VCHARGE_MAX                   19200

/* otg voltage */
#define BQ25713_REG_OTG_VOL                   0x06
#define BQ25713_RANGE_LOW_OTG_VOL_LIMIT       0xA30
#define BQ25713_RANGE_HIGH_OTG_VOL_LIMIT      0x7C0

/* otg current */
#define BQ25713_REG_OTG_CUR                   0x09
#define BQ25713_OTG_CUR_MIN                   500
#define BQ25713_OTG_CUR_MAX                   2000
#define BQ25713_OTG_CUR_STEP                  50

/* input voltage limit */
#define BQ25713_REG_IVL                       0x0A
#define BQ25713_VINDPM_MAX                    19520
#define BQ25713_VINDPM_MIN                    3200
#define BQ25713_VINDPM_OFFSET                 3200

/* minimum system voltage */
#define BQ25713_REG_MIN_VSYS                  0x0D

/* input current limit */
#define BQ25713_REG_IIN_HOST                  0x0F
#define BQ25713_IINLIM_MAX                    6350
#define BQ25713_IINLIM_MIN                    50
#define BQ25713_IINLIM_STEP                   50
#define BQ25713_IINLIM_DEFAULT                500

/* charger status 0 */
#define BQ25713_REG_CS0                       0x20
#define BQ25713_REG_CS0_NOT_PG_MASK           BIT(15)
#define BQ25713_REG_CS0_IN_FCHRG_MASK         BIT(10)
#define BQ25713_REG_CS0_ACOV_MASK             BIT(7)
#define BQ25713_REG_CS0_BATOC_MASK            BIT(6)
#define BQ25713_REG_CS0_ACOC_MASK             BIT(5)
#define BQ25713_REG_CS0_OTG_OC_MASK           (BIT(0) | BIT(1))

/* charger status 1 */
#define BQ25713_REG_CS1                       0x21
#define BQ25713_REG_CS1_AC_STAT_MASK          BIT(7)
#define BQ25713_REG_CS1_VINDPM_STAT_MASK      BIT(4)
#define BQ25713_REG_CS1_IINDPM_STAT_MASK      BIT(3)

/* charger prochot status */
#define BQ25713_REG_PHS                       0x22
#define BQ25713_REG_PHS_BAT_REMOVE_MASK       BIT(1)

/* vbus adc */
#define BQ25713_REG_VBUS_ADC                  0x27
#define BQ25713_VBUS_ADC_STEP_MV              64
#define BQ25713_VBUS_ADC_OFFSET               3200

/* ibus adc */
#define BQ25713_REG_IBUS_ADC                  0x2B
#define BQ25713_IBUS_ADC_STEP_MA              50

/* vbat adc */
#define BQ25713_REG_VBAT_ADC                  0x2C
#define BQ25713_VBAT_ADC_OFFSET               2880
#define BQ25713_VBAT_ADC_STEP_MV              64

/* part information */
#define BQ25713_REG_MA_ID                     0x2E
#define BQ25713_MANUF_ID                      0x40
#define SC8886_MANUF_ID                       0x01
#define SY6961_MANUF_ID                       0xB6

/* adc control */
#define BQ25713_REG_ADC_LOW                   0x3A
#define BQ25713_REG_ADC_HIGH                  0x3B
#define BQ25713_REG_ADC_OPTION_MASK           (BIT(6) | BIT(7))
#define BQ25713_REG_ADC_OPTION_SHIFT          6
#define BQ25713_REG_ADC_DONE_STAT_MASK        BIT(6)
#define BQ25713_ADC_EN_VAL_DEFAULT            0x57
#define BQ25713_ADC_MODE_DEFAULT              0x20
#define BQ25713_ADC_ONESHOT_EN                1
#define BQ25713_ADC_CONTINUOUS                2
#define BQ25713_ADC_ONESHOT                   3

#endif /* _BQ25713_CHARGER_H_ */
