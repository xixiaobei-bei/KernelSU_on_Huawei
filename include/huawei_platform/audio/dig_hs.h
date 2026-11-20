/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: digital headset driver
 * Author: lijinkui
 * Create: 2019-7-16
 */

#ifndef __DIG_HS_H__
#define __DIG_HS_H__

#include <linux/list.h>

enum dig_hs_tag {
	TAG_DIG_HS_POWER = 0,
	TAG_DIG_HS_POWER_V600,
	TAG_DIG_HS_CORE,
};

enum dig_hs_priority {
	PRIORITY_L = 0,
	PRIORITY_H,
};

struct dig_hs_ops {
	int (*dig_hs_plug_in)(void);
	int (*dig_hs_plug_out)(void);
};

struct dig_hs_dev {
	struct list_head list;
	enum dig_hs_tag tag;
	struct dig_hs_ops ops;
};

#ifdef CONFIG_DIG_HS

unsigned int dig_hs_plug_in(void);
unsigned int dig_hs_plug_out(void);
int dig_hs_plug_in_single(enum dig_hs_tag tag);
int dig_hs_plug_out_single(enum dig_hs_tag tag);
void dig_hs_dev_register(struct dig_hs_dev *dev, int priority);
void dig_hs_dev_deregister(enum dig_hs_tag tag);
#else
static inline unsigned int dig_hs_plug_in(void)
{
	return 0;
}

static inline unsigned int dig_hs_plug_out(void)
{
	return 0;
}

static inline int dig_hs_plug_in_single(enum dig_hs_tag tag)
{
	return 0;
}

static inline int dig_hs_plug_out_single(enum dig_hs_tag tag)
{
	return 0;
}

static inline void dig_hs_dev_register(struct dig_hs_dev *dev, int priority)
{
}

static inline void dig_hs_dev_deregister(enum dig_hs_tag tag)
{
}
#endif

#endif // __DIG_HS_H__
