#ifndef _NPU_MNTN_H_
#define _NPU_MNTN_H_

#include <linux/hisi/rdr_pub.h>

#ifdef CONFIG_HUAWEI_DSM

#include <dsm/dsm_pub.h>

extern struct dsm_client *dsm_ai_client;
/* Report to DSM */
#define DSM_AI_KERN_ERROR_REPORT(error_no, fmt, args...)\
do {\
    if (NULL != dsm_ai_client && !dsm_client_ocuppy(dsm_ai_client))\
    {\
        dsm_client_record(dsm_ai_client, fmt, ##args);\
        dsm_client_notify(dsm_ai_client, error_no);\
    }\
} while(0)
#else

#define DSM_AI_KERN_ERROR_REPORT(error_no, fmt, args...)

#endif

enum rdr_npu_system_error_type {
	MODID_NPU_START    = HISI_BB_MOD_NPU_START,
	MODID_NPU_EXC_DEAD = MODID_NPU_START,
	MODID_NPU_EXC_SET_BACK_CLOCK_FAIL,
	MODID_NPU_EXC_SET_CLOCK_FAIL,
	MODID_NPU_EXC_SET_POWER_UP_FAIL,
	MODID_NPU_EXC_SET_POWER_UP_STATUS_FAULT,
	MODID_NPU_EXC_SET_POWER_DOWN_FAIL,
	MODID_NPU_EXC_INTERRUPT_ABNORMAL,
	MODID_NPU_EXC_END = HISI_BB_MOD_NPU_END
};

enum rdr_npu_ip_type {
	NPU_IP        = 0,
    VVCODECBUS_IP = 1,
	NPU_IP_END
};

struct npu_peri_reg_s {
	unsigned int peri_stat;
	unsigned int ppll_select;
	unsigned int power_stat;
	unsigned int power_ack;
	unsigned int reset_stat;
	unsigned int perclken0;
	unsigned int perstat0;
};

struct npu_mstr_reg_s {
	unsigned int rd_bitmap;
	unsigned int wr_bitmap;
	unsigned int rd_cmd_total_cnt0;
	unsigned int rd_cmd_total_cnt1;
	unsigned int rd_cmd_total_cnt2;
	unsigned int wr_cmd_total_cnt;
};

struct npu_exc_info_s {
	unsigned int interrupt_status;
    unsigned int target_ip;
    int          result;
};

int npu_mntn_rdr_init(void);
void npu_mntn_rdr_exit(void);
void npu_mntn_peri_reg_init(unsigned int core_id, struct npu_peri_reg_s *peri_reg_ptr);
void npu_mntn_mstr_reg_init(unsigned int core_id, struct npu_mstr_reg_s *mstr_reg_ptr);
void npu_mntn_power_exc_proc(unsigned int core_id, unsigned int target_ip, int result);
void npu_mntn_interrupt_exc_proc(unsigned int core_id, unsigned int interrupt_status);
void npu_mntn_set_core_id(unsigned int core_id);
#endif

