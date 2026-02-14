/*
 * hisi loadmonitor driver.
 *
 * Copyright (C) 2017 huawei Ltd.
 * Author:ContextHub team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#define MAX_SIG_CNT_PER_IP                    32
enum {
	DATA_INDEX_IDLE = 0,
	DATA_INDEX_BUSY_NORMAL,
	DATA_INDEX_BUSY_LOW,
	DATA_INDEX_MAX,
};

struct LOADMONITOR_SIG {
	uint64_t                 count[DATA_INDEX_MAX];
	uint32_t                 samples;
};

struct LOADMONITOR_SIGS {
	struct LOADMONITOR_SIG   sig[MAX_SIG_CNT_PER_IP];
};


struct AO_LOADMONITOR_SIG {
	struct LOADMONITOR_SIGS  sigs;
	uint32_t                 enable;
	uint32_t                 interval;
};

/* EXTERNAL FUNCTIONS */
#if defined(CONFIG_CONTEXTHUB_LOADMONITOR)
int ao_loadmonitor_enable(unsigned int delay_value, unsigned int freq);
int ao_loadmonitor_disable(void);
int32_t _ao_loadmonitor_read(void *data, uint32_t len);
#else
inline int ao_loadmonitor_enable(unsigned int delay_value, unsigned int freq) {pr_err("ao_loadmonitor_enable invalid;\n"); return -1;}
inline int ao_loadmonitor_disable(void) {pr_err("ao_loadmonitor_disable invalid;\n"); return -1;}
inline int32_t _ao_loadmonitor_read(void *data, uint32_t len) {pr_err("_ao_loadmonitor_read invalid;\n"); return -1;}
#endif

