/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
* Description: ASoc adapter layer for diff version kernel
 * Author: liupan
 * Create: 2020-08-24
 */

#ifndef __ASOC_ADAPTER_H__
#define __ASOC_ADAPTER_H__

#include <linux/version.h>
#include <sound/soc.h>


#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
unsigned int snd_soc_component_read32(struct snd_soc_component *component,
	unsigned int reg);
#endif

#endif
