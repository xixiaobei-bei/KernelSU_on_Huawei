/*
 * hi6555v3 i2s pinctrl driver.
 *
 * Copyright (c) 2018 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __HI6555V3_I2S_H__
#define __HI6555V3_I2S_H__

struct hi6555v3_i2s2_priv {
	struct pinctrl *pctrl;
	struct pinctrl_state *pin_default;
	struct pinctrl_state *pin_idle;
};

struct hi6555v3_i2s_priv {
	struct hi6555v3_i2s2_priv *i2s2_priv;
};

int hi6555v3_i2s2_set_pinctrl_default(void);

int hi6555v3_i2s2_set_pinctrl_idle(void);

#endif /* __HI6555V3_I2S_H__ */
