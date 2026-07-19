#ifndef _IPU_MNTN_H_
#define _IPU_MNTN_H_

#include <linux/netlink.h>
#include <linux/sched.h>
#include <net/sock.h>
#include <linux/hisi/rdr_pub.h>

/* AI DRD */
#define IPU_BUF_LEN_MAX 	   (256)
#define IPU_LINE_MAX		   (128)

enum rdr_ipu_system_error_type {
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

struct rdr_dump_info_s {
	u32 modid;
	u64 coreid;
	pfn_cb_dump_done cb;
    char *pathname;
};

struct ipu_peri_reg_s {
	unsigned int peri_stat;
	unsigned int ppll_select;
	unsigned int power_stat;
	unsigned int power_ack;
	unsigned int reset_stat;
	unsigned int perclken0;
	unsigned int perstat0;
};

struct ipu_mstr_reg_s {
	unsigned int rd_bitmap;
	unsigned int wr_bitmap;
	unsigned int rd_cmd_total_cnt0;
	unsigned int rd_cmd_total_cnt1;
	unsigned int rd_cmd_total_cnt2;
	unsigned int wr_cmd_total_cnt;
};

struct ipu_mntn_info_s {
	unsigned int ipu_run_status;
	unsigned int bbox_addr_offset;
	struct rdr_register_module_result ipu_ret_info;
	struct rdr_dump_info_s dump_info;
    void   *rdr_addr;
};

struct ipu_reg_info_s {
	struct ipu_peri_reg_s peri_reg;
	struct ipu_mstr_reg_s mstr_reg;
};

extern struct workqueue_struct *ipu_mntn_rdr_wq;
extern struct ipu_reg_info_s ipu_reg_info;

extern int ipu_mntn_rdr_init(void);

#endif
