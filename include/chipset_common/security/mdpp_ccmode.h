/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: get ccmode type, head file
 * Author: yangzhenyuan yangzhenyuan@huawei.com
 * Create: 2019-11-06
 */
#ifndef _MDPP_CCMODE_H_
#define _MDPP_CCMODE_H_

#define CCMODE_TYPE_DISABLE 0
#define CCMODE_TYPE_ENABLE 1

#ifdef CONFIG_HUAWEI_MDPP_CCMODE
unsigned int get_ccmode(void);
#else
static inline unsigned int get_ccmode(void)
{
	return CCMODE_TYPE_DISABLE;
}
#endif

#endif
