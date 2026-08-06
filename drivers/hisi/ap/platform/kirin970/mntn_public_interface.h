#ifndef __MNTN_PUBLIC_INTERFACE_H__
#define __MNTN_PUBLIC_INTERFACE_H__ 
#include "soc_acpu_baseaddr_interface.h"
#include "pmic_interface.h"
#include "global_ddr_map.h"
#include "../common/mntn_dump_interface.h"
#include "../common/mntn_common_interface.h"
enum {
 BL31_TRACE_EXCEPTION_SIZE = 0x100,
 BL31_TRACE_IRQ_SMC_SIZE = 0x3F000,
 BL31_TRACE_DMSS_SIZE = 0x0,
};
typedef struct dmss_exception_trace_s {
 unsigned long long e_32k_time;
} dmss_exception_trace_t;
#if defined(CONFIG_GCOV_KERNEL) || defined(CONFIG_HISI_GCOV_FASTBOOT)
struct mdump_gcov {
 unsigned long gcda_addr;
 unsigned int gcda_size;
};
#endif
#endif
