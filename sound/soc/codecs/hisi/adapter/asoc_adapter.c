/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description: ASoc adapter layer for diff version kernel
 * Author: liupan
 * Create: 2020-08-24
 */

#include "asoc_adapter.h"

#include <sound/soc.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
unsigned int snd_soc_component_read32(struct snd_soc_component *component,
	unsigned int reg)
{
	return snd_soc_read(snd_soc_component_to_codec(component), reg);
}
#endif
