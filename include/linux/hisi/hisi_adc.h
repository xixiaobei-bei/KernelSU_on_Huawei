/*
 * adc.h
 *
 * for the hkadc driver.
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#ifndef ADC_H
#define ADC_H
#include <iomcu_ddr_map.h>

#ifdef CONFIG_THERMAL_CONTEXTHUB
#define CONTEXTHUB_THERMAL_DDR_HEADER_ADDR      DDR_THERMAL_SHMEM_PHYMEM_BASE_AP
#define CONTEXTHUB_THERMAL_DDR_MEMBERS_SIZE     (0x600)
#define CONTEXTHUB_THERMAL_DDR_TOTAL_SIZE       DDR_THERMAL_SHMEM_PHYMEM_SIZE
#endif
enum hkadc_table_id {
	HKADC_ADC_TABLEID = 0,
	HKADC_PA_TABLEID,
	HKADC_SOC_TABLEID,
	HKADC_BAT_TABLEID,
	HKADC_MAX_TABLEID
};

struct hw_chan_table {
	unsigned short int usr_id;
	unsigned short int hw_channel;
	unsigned short int table_id;
	unsigned int table_size;
};

#ifdef CONFIG_LPM_HKADC
extern int adc_to_volt(int adc);
#else
static inline int adc_to_volt(int adc)
{
	return -1;
}
#endif /* CONFIG_HKADC */

/*
 * Function name:adc_get_value.
 * Discription:get volt from hkadc.
 * Parameters:
 *      @ adc_channel
 * return value:
 *      @ adc value: negative-->failed, other-->succeed.
 * notice: user must check the result of returning value,
 *         if return negative, should give up it.
 *         suggestion, could call the interface several times.
 */
#ifdef CONFIG_LPM_HKADC
extern int hisi_adc_get_value(int adc_channel);
#else
static inline int hisi_adc_get_value(int adc_channel)
{
	return -1;
}
#endif /* CONFIG_HKADC */

/*
 * Function name:adc_get_adc.
 * Discription:get adc from hkadc.
 * Parameters:
 *      @ adc_channel
 * return value:
 *      @ adc value: negative-->failed, other-->succeed.
 * notice: user must check the result of returning value,
 *         if return negative, should give up it.
 *         suggestion, could call the interface several times.
 */
#ifdef CONFIG_LPM_HKADC
extern int hisi_adc_get_adc(int adc_channel);
extern int hisi_adc_get_current(int adc_channel);
#else
static inline int hisi_adc_get_adc(int adc_channel)
{
	return -1;
}

static inline int hisi_adc_get_current(int adc_channel)
{
	return -1;
}
#endif /* CONFIG_HKADC */

#endif /* ADC_H */
