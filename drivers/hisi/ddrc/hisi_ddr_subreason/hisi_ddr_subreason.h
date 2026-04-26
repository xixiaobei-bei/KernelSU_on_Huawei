/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: dmss intr subreason
 * Author: caodongyi
 * Create: 2020-5-27
 */

#ifndef _HISI_DDR_SUBREASON_H_
#define _HISI_DDR_SUBREASON_H_

#ifdef CONFIG_HISI_DDR_MONOCEROS
#include "hisi_ddr_subreason_monoceros.h"
#elif CONFIG_HISI_DDR_PERSEUS
#include "hisi_ddr_subreason_perseus.h"
#elif CONFIG_HISI_DDR_CASSIOPEIA
#include "hisi_ddr_subreason_cassiopeia.h"
#endif

#endif