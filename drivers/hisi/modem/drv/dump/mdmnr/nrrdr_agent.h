#ifndef __NRRDR_AGENT_H__
#define __NRRDR_AGENT_H__
#include "product_config.h"
#include "osl_types.h"
#ifdef BSP_CONFIG_PHONE_TYPE
#include "../../adrv/adrv.h"
#else
#include <linux/hisi/rdr_pub.h>
#endif

#ifdef ENABLE_BUILD_NRRDR
void dump_save_nr_mandatory_logs(char* dir_name);
s32 dump_wait_nr_done(void);
s32 nrrdr_agent_init(void);
s32 dump_recv_msg_for_nr(u32 channel_id, u32 len);
s32 dump_notify_nr(u32 modid, u32 etype, u64 coreid, char* logpath, pfn_cb_dump_done fndone);

#else
static inline void dump_save_nr_mandatory_logs(char* dir_name){return;}
static inline s32 dump_wait_nr_done(void){return BSP_OK;}
static inline s32 nrrdr_agent_init(void){return BSP_OK;}
static inline s32 dump_recv_msg_for_nr(u32 channel_id, u32 len){return BSP_OK;}
static inline s32 dump_notify_nr(u32 modid, u32 etype, u64 coreid, char* logpath, pfn_cb_dump_done fndone){return BSP_OK;}

#endif

#endif
