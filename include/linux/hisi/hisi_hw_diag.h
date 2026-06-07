/*
 *  hisi_hw_diag.h
 */
#ifndef __HISI_HW_DIAG_H__
#define __HISI_HW_DIAG_H__

#include <linux/hisi/hisi_bbox_diaginfo.h>

#define HISI_HWDIAG_TRACE_DATA_MAXLEN 128

struct hisi_diag_noc_info {
	char *init_flow;
	char *target_flow;
};

struct hisi_diag_panic_info {
	unsigned int cpu_num;
};

typedef union {
	struct hisi_diag_noc_info noc_info;
	struct hisi_diag_panic_info cpu_info;
} hisi_hw_diag_info;

struct hisi_hw_diag_trace {
	unsigned int used;
	unsigned int err_id;
	char data[HISI_HWDIAG_TRACE_DATA_MAXLEN];
};

struct hisi_hw_diag_dev {
	spinlock_t record_lock;
	struct hisi_hw_diag_trace *trace_addr;
	unsigned int trace_size;
	unsigned int trace_max_num;
};

#ifdef CONFIG_HISI_HW_DIAG
void hisi_hw_diaginfo_trace(unsigned int err_id, hisi_hw_diag_info *diaginfo);
void hisi_hw_diaginfo_record(const char *date);
void hisi_hw_diag_init(void);
#else
static inline void hisi_hw_diaginfo_trace(unsigned int err_id, hisi_hw_diag_info *diaginfo){return;}
static inline void hisi_hw_diaginfo_record(const char *date){return;}
static inline void hisi_hw_diag_init(void){return;}
#endif

#endif
