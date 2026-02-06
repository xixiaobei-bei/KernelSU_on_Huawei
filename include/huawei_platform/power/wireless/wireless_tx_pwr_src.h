/*
 * wireless_tx_pwr_src.h
 *
 * power source module for wireless reverse charging
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

#ifndef _WIRELESS_TX_PWR_SRC_H_
#define _WIRELESS_TX_PWR_SRC_H_

#define HALL_TX_VOUT                 5500

#define WLTX_OTG_VOUT_LTH            4000
#define WLTX_OTG_VOUT_HTH            5800

#define WLTX_SC_ADAP_VSET            5300
#define WLTX_SC_VOL_HIGH_TH          5500
#define WLTX_SC_VOL_LOW_TH           4750
#define WLTX_SC_ADAP_V_DELTA         500
#define WLTX_SC_ADAP_ISET            4000

enum wltx_pwr_src {
	PWR_SRC_BEGIN = 0x0,
	PWR_SRC_NULL = PWR_SRC_BEGIN,
	PWR_SRC_VBUS,
	PWR_SRC_OTG,
	PWR_SRC_5VBST,
	PWR_SRC_SPBST, /* Specialized boost */
	PWR_SRC_VBUS_CP,
	PWR_SRC_OTG_CP,
	PWR_SRC_BP2CP, /* bypass mode to charge pump mode with spbst */
	PWR_SRC_NA,
	PWR_SRC_END,
};

#ifdef CONFIG_WIRELESS_CHARGER
enum wltx_pwr_src wltx_get_cur_pwr_src(void);
const char *wltx_get_pwr_src_name(enum wltx_pwr_src src);
enum wltx_pwr_src wltx_set_pwr_src_output(bool enable, enum wltx_pwr_src src);
#else
static inline const char *wltx_get_pwr_src_name(enum wltx_pwr_src src)
{
	return "NA";
}

static inline enum wltx_pwr_src wltx_set_pwr_src_output(bool enable,
	enum wltx_pwr_src src)
{
	return PWR_SRC_NA;
}

static inline enum wltx_pwr_src wltx_get_cur_pwr_src(void)
{
	return PWR_SRC_NA;
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_TX_PWR_SRC_H_ */
