#ifndef _PARTITION_H_
#define _PARTITION_H_

#include "hisi_partition.h"
#include "partition_def.h"

#if defined(CONFIG_HISI_PARTITION_HI3650)
#include "hi3650_partition.h"
#elif defined(CONFIG_HISI_PARTITION_HI3660)
#include "hi3660_partition.h"
#elif defined(CONFIG_HISI_PARTITION_HI6250)
#include "hi6250_partition.h"
#elif defined(CONFIG_HISI_PARTITION_LIBRA)
#include "libra_partition.h"
#elif defined(CONFIG_HISI_PARTITION_KIRIN970)
#include "kirin970_partition.h"
#elif defined(CONFIG_HISI_PARTITION_CANCER)
#ifdef HISI_EXTERNAL_MODEM
#include "cancer_a_plus_b_partition.h"
#else
#include "cancer_partition.h"
#endif
#elif defined(CONFIG_HISI_PARTITION_CAPRICORN)
#include "capricorn_partition.h"
#elif defined(CONFIG_HISI_PARTITION_PISCES)
#include "pisces_partition.h"
#elif defined(CONFIG_HISI_PARTITION_LEO)
#include "leo_partition.h"
#elif defined(CONFIG_HISI_PARTITION_TAURUS)
#ifdef HISI_EXTERNAL_MODEM
#include "taurus_a_plus_b_partition.h"
#elif defined CONFIG_HISI_PARTITION_CS2
#include "taurus_cs2_partition.h"
#else
#include "taurus_partition.h"
#endif
#endif

#endif


