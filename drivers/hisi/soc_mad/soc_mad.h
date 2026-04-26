/*
 * soc_mad.h codec driver.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#ifndef _SOC_MAD_H_
#define _SOC_MAD_H_

enum mad_mode {
	HIGH_FREQ_MODE,
	LOW_FREQ_MODE,
	MODE_CNT,
};

enum mad_application_mode {
	MAD_SIF,
	MAD_DMIC,
	MAD_MODE_CNT,
	MAD_MODE_INVALID,
};

void soc_mad_set_pinctrl_state(unsigned int mode);
void soc_mad_select_din(unsigned int mode);
#endif

