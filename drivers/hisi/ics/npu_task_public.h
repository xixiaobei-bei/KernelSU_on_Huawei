#ifndef _NPU_TASK_PUBLIC_H_
#define _NPU_TASK_PUBLIC_H_

#include "npu_public.h"
#include "npu_task.h"

#define NPU_DEBUG_SSWQ_REG

#define SUCCESS                     (0)
#define NPU_SSWQ_LOW_FIFO_DEPTH     (8)
#define NPU_SSWQ_HIGH_FIFO_DEPTH    (8)
#define NPU_SSWQ_FIFO_DEPTH         (8)
#define NPU_PM_VOTE_NUM_MAX         (NPU_SSWQ_LOW_FIFO_DEPTH + NPU_SSWQ_HIGH_FIFO_DEPTH)

#define NPU_WATCHDOG_TIMEOUT        (0xB2D05E00) /*3000000000 3S*/

/* NPU configure register offset */
#define NPU_TO_STOP          (0x0)
#define NPU_TO_START         (0x1)
#define NPU_STATUS_FINISH    (0x1)
/* NPU start up reg */
#define NPU_START_REG        (0x18)
/* NPU payload finish status reg */
#define NPU_STATUS_REG       (0x20)
/* NPU access external DDR address */
#define NPU_BASE_ADDR_REG    (0x28)
/* NPU internal SRAM configure reg */
#define NPU_SRAM_CTRL_REG    (0x30)
/* NPU Version reg */
#define NPU_VERSION_REG      (0x40)

#define NPU_STATUS_UNFINISH  (0x0)

#define WORK_QUEUE_ALL_FINISH_IS_BIT    (3)
#define WORK_QUEUE_ALL_FINISH_IS_MASK   (0x1<<WORK_QUEUE_ALL_FINISH_IS_BIT)
#define WORK_QUEUE_MASK_FINISH_IS_BIT   (2)
#define WORK_QUEUE_MASK_FINISH_IS_MASK  (0x1<<WORK_QUEUE_MASK_FINISH_IS_BIT)
#define WORK_QUEUE_FINISH_IS_BIT        (1)
#define WORK_QUEUE_FINISH_IS_MASK       (0x1<<WORK_QUEUE_FINISH_IS_BIT)
#define NORMAL_FINISH_IS_BIT            (0)
#define NORMAL_FINISH_IS_MASK           (0x1<<NORMAL_FINISH_IS_BIT)

#define WORK_QUEUE_FINISH_BIT           (0)
#define WORK_QUEUE_FINISH_MASK          (0xFFFF<<WORK_QUEUE_FINISH_BIT)
/* the npu task status */
typedef enum {
	NPU_SSWQ_TASK_START = 0,
	NPU_SSWQ_TASK_NONE = NPU_SSWQ_TASK_START,
	/*First*/
	NPU_SSWQ_TASK_WAITING,
	NPU_SSWQ_TASK_DONE,
	/*Add new to here*/
	NPU_SSWQ_TASK_END
} NPU_SSWQ_TASK_STATUS_E;

/* the npu task element */
typedef struct {
	unsigned int core_id;
	unsigned int task_type;
	unsigned int inst_size;
	unsigned int task_id;
	unsigned int thread_id;
	unsigned int task_config;
	unsigned int task_priority;
	unsigned long inst_addr_off;
	unsigned long ddr_base_addr;
	/*use to mark task status*/
	volatile NPU_SSWQ_TASK_STATUS_E *task_status;
} NPU_SSWQ_TASK_ELEMENT_S;

typedef struct {
	/* config reg addr */
	struct resource *cfg_mem;
	unsigned int config_reg_length;
	phys_addr_t  config_reg_phys_addr;
	void __iomem *config_reg_virt_addr;

	/* inst reg addr */
	struct resource *inst_mem;
	unsigned int inst_ram_size;
	phys_addr_t  inst_ram_phys_addr;
	void __iomem *inst_ram_virt_addr;
} NPU_REG_SPACE_S;

typedef struct {
	unsigned int core_num;
	unsigned long watchdog_timer[MAX_SUPPORT_CORE_NUM];
	NPU_REG_SPACE_S reg_space[MAX_SUPPORT_CORE_NUM];
	NPU_CALLBACK_FUNC_S callback_fun;
} NPU_TASK_PRIVATE_S;

extern NPU_TASK_PRIVATE_S* g_task_private;

int npu_read_reg32(unsigned int core_id, unsigned long offset);

int npu_write_reg32(unsigned int core_id, unsigned long offset, unsigned int value);

int npu_task_pm_get(unsigned int core_id, unsigned int get_num);

int npu_task_pm_put(unsigned int core_id, unsigned int put_num);

int npu_task_update_profile(unsigned int core_id);

int npu_task_reset_proc(unsigned int core_id);

int npu_task_smmu_pte_update(unsigned int core_id);

int npu_task_perf_monitor_open(unsigned int core_id);

int npu_task_fifo_opt_lock(unsigned int core_id);

int npu_task_fifo_opt_unlock(unsigned int core_id);

void npu_task_watchdog_start(unsigned int core_id);

#endif
