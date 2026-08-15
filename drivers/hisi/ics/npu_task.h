#ifndef _NPU_TASK_H_
#define _NPU_TASK_H_

#include <linux/platform_device.h>

typedef int  (* NPU_PM_PUT_CALLBACK_FUNC)(unsigned int);
typedef int  (* NPU_PM_GET_CALLBACK_FUNC)(unsigned int);
typedef void (* NPU_RESET_CALLBACK_FUNC)(unsigned int);
typedef int  (* NPU_UPDATE_CALLBACK_FUNC)(unsigned int);
typedef int  (* NPU_MEM_MNGR_CHECK_CALLBACK_FUNC)(unsigned int, unsigned int);
typedef void (* NPU_MEM_MNGR_DUMP_CALLBACK_FUNC)(void);
typedef void (* NPU_SMMU_PTE_UPDATE_CALLBACK_FUNC)(unsigned int);
typedef void (* NPU_PERF_MONITOR_EN_CALLBACK_FUNC)(unsigned int);
typedef void (* NPU_COMMON_LOCK_FUNC)(unsigned int);
typedef void (* NPU_COMMON_UNLOCK_FUNC)(unsigned int);

/* task */
typedef struct {
	NPU_PM_PUT_CALLBACK_FUNC pf_npu_pm_put;
	NPU_PM_GET_CALLBACK_FUNC pf_npu_pm_get;
	NPU_RESET_CALLBACK_FUNC  pf_npu_reset;
	NPU_UPDATE_CALLBACK_FUNC pf_npu_update_profile;
	NPU_MEM_MNGR_CHECK_CALLBACK_FUNC  pf_npu_mem_mngr_check;
	NPU_MEM_MNGR_DUMP_CALLBACK_FUNC   pf_npu_mem_mngr_dump;
	NPU_SMMU_PTE_UPDATE_CALLBACK_FUNC pf_npu_smmu_pte_update;
	NPU_PERF_MONITOR_EN_CALLBACK_FUNC pf_npu_perf_monitor_open;
	NPU_COMMON_LOCK_FUNC   pf_npu_common_lock;
	NPU_COMMON_UNLOCK_FUNC pf_npu_common_unlock;
} NPU_CALLBACK_FUNC_S;

int npu_push_task(unsigned int* arg, unsigned int irq_num);

void npu_core_irq_handler(unsigned int core_id, unsigned long irq_io_addr);

int npu_task_finish_interrupt_clear(unsigned int core_id);

void npu_task_restore(unsigned int core_id, NPU_TASK_PROCESS_E restore_reason);

int npu_task_init(struct platform_device *pdev, unsigned int core_num, NPU_CALLBACK_FUNC_S* callback_fun_ptr);

void npu_task_exit(void);

#endif
