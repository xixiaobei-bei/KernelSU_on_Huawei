
#ifndef __NRRDR_CORE_H__
#define __NRRDR_CORE_H__

#include "product_config.h"
#include "osl_types.h"
#ifdef BSP_CONFIG_PHONE_TYPE
#include "../../adrv/adrv.h"
#else
#include <linux/hisi/rdr_pub.h>
#endif


#ifdef ENABLE_BUILD_NRRDR
s32 dump_nrrdr_init(void);
s32  dump_nr_callback(u32 modid, u32 etype, u64 coreid, char* logpath, pfn_cb_dump_done fndone);
#else
static inline s32 dump_nrrdr_init(void){return BSP_OK;}
static inline s32  dump_nr_callback(u32 modid, u32 etype, u64 coreid, char* logpath, pfn_cb_dump_done fndone){return BSP_OK;}

#endif

#endif

