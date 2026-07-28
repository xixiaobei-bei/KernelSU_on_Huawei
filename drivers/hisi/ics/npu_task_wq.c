#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/kfifo.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/ion.h>
#include <asm/uaccess.h>

#include "soc_ics_interface.h"
#include "npu_public.h"
#include "npu_task_public.h"
#include "npu_task_wq.h"

#define TASKQUEUE_SIZE      (8)
#define BOOT_INST_SIZE      (64)
#define BOOT_INST_NUMBER    (5)

#define CHECK_WQ_TASK_TYPE(taskType) ((NPU_COMPUTE_TASK == taskType) || (NPU_SYNC_TASK == taskType))

/* this struct stores the temp data of boot instrument for NPU initialization after power-up */
typedef struct {

	/* store the value from IOCTL WRITE when NPU is off */
	unsigned int npu_access_ddr_addr;

	/* boot-instrument data */
	unsigned char boot_inst[BOOT_INST_SIZE * BOOT_INST_NUMBER];
	unsigned char boot_inst_tmp[BOOT_INST_SIZE * BOOT_INST_NUMBER];
	/* size of boot-inst data */
	unsigned int boot_inst_size;

	/* bool flag to record whether access_ddr_addr is recorded */
	bool access_ddr_addr_is_config;

	/* bool flag to record whether boot_inst is recorded */
	bool boot_inst_recorded_is_config;
} NPU_BOOT_INST_S;

typedef struct {
	NPU_BOOT_INST_S boot_inst_set;
	struct semaphore task_fifo_sem;
	/* FIFO_TaskElements which holds NPU tasks */
	DECLARE_KFIFO_PTR(FIFO_TaskElements, taskElement);
	/* wait queue holds sync task which make user process sleep */
	wait_queue_head_t sync_wq;
} NPU_TASK_WQ_PRIVATE_S;

NPU_TASK_WQ_PRIVATE_S* task_wq_private;

int set_offchip_inst_addr(unsigned int core_id, unsigned long addr)
{
	unsigned int i;
	unsigned int tmp;
	NPU_REG_SPACE_S* reg_space =  &g_task_private->reg_space[core_id];
	NPU_TASK_WQ_PRIVATE_S *task_fifo_ptr = &task_wq_private[core_id];

	if ((NULL == reg_space) || (NULL == task_fifo_ptr)) {
		NPU_ERR("Input parameter is NULL!");
		return -EINVAL;
	}

	/* write boot inst to instram */
	for (i = 0; i < task_fifo_ptr->boot_inst_set.boot_inst_size; i += 4) {
		tmp = *(unsigned int *)&task_fifo_ptr->boot_inst_set.boot_inst_tmp[i];
		iowrite32(tmp, (void *)((unsigned long)reg_space->inst_ram_virt_addr + i));
	}

	return SUCCESS;
}

static void change_inst_data(unsigned int core_id, unsigned long data, int site)
{
	unsigned int i;
	int tmp, tmp1, j;
	unsigned char *inst = (unsigned char *)(&task_wq_private[core_id].boot_inst_set.boot_inst_tmp[0]) ;
	tmp = site/8+1;
	tmp1 = site%8;
	if (!tmp1)
		for ( i=56,j=tmp; j<tmp+8; i-=8, j++)
			inst [128 - j] = (data >> i & 0xff);  //lint !e679
	else {
		inst[128 - tmp] |= ((data >> (56+tmp1)) & ((0x1 <<(9-tmp1)) -1)); //lint !e679 !e647
		inst[120 - tmp] |= (data & ((0x1 <<(tmp1 +1)) -1) )<< (8-tmp1); //lint !e679 !e647
		for ( i=56 - (8-tmp1),j=tmp+1; j<tmp+8; i-=8, j++)
			inst [64*1 + 64 - j] = (data >> i & 0xff); //lint !e679
	}
}

void npu_boot_inst_init(unsigned int core_id)
{
	unsigned char *boot_inst = &task_wq_private[core_id].boot_inst_set.boot_inst[0];

	task_wq_private[core_id].boot_inst_set.boot_inst_size = BOOT_INST_SIZE * BOOT_INST_NUMBER;

	memset(boot_inst, 0, sizeof(task_wq_private[core_id].boot_inst_set.boot_inst));
	boot_inst[BOOT_INST_SIZE * 0 + BOOT_INST_SIZE - 1] = 0x2;
	boot_inst[BOOT_INST_SIZE * 1 + BOOT_INST_SIZE - 1] = 0x4;
	boot_inst[BOOT_INST_SIZE * 1 + BOOT_INST_SIZE - 2] = 0x80;
	boot_inst[BOOT_INST_SIZE * 1 + BOOT_INST_SIZE - 3] = 0x80;
	boot_inst[BOOT_INST_SIZE * 1 + BOOT_INST_SIZE - 9] = 0x1;
	boot_inst[BOOT_INST_SIZE * 1 + BOOT_INST_SIZE - 10] = 0x2;
	boot_inst[BOOT_INST_SIZE * 1 + BOOT_INST_SIZE - 11] = 0x4;
	boot_inst[BOOT_INST_SIZE * 1 + BOOT_INST_SIZE - 17] = 0x22;
	boot_inst[BOOT_INST_SIZE * 1 + BOOT_INST_SIZE - 18] = 0x32;
	boot_inst[BOOT_INST_SIZE * 1 + BOOT_INST_SIZE - 26] = 0x20;
	boot_inst[BOOT_INST_SIZE * 1 + BOOT_INST_SIZE - 27] = 0x40;
	boot_inst[BOOT_INST_SIZE * 1 + BOOT_INST_SIZE - 33] = 0x4;
	boot_inst[BOOT_INST_SIZE * 1 + BOOT_INST_SIZE - 42] = 0x4;
	boot_inst[BOOT_INST_SIZE * 1 + BOOT_INST_SIZE - 43] = 0x8;
	boot_inst[BOOT_INST_SIZE * 2 + BOOT_INST_SIZE - 1] = 0x5;
	boot_inst[BOOT_INST_SIZE * 2 + BOOT_INST_SIZE - 2] = 0x81;
	boot_inst[BOOT_INST_SIZE * 2 + BOOT_INST_SIZE - 7] = 0x82;
	boot_inst[BOOT_INST_SIZE * 3 + BOOT_INST_SIZE - 1] = 0x2;
	boot_inst[BOOT_INST_SIZE * 4 + BOOT_INST_SIZE - 1] = 0x2;
	return;
}

static int npu_start_wq_task(unsigned int core_id)
{
	taskElement head;
	NPU_REG_SPACE_S* reg_space = &g_task_private->reg_space[core_id];
	NPU_TASK_WQ_PRIVATE_S *task_fifo_ptr = &task_wq_private[core_id];

	/* npu power up */
	npu_task_pm_get(core_id, 1);
	npu_task_update_profile(core_id);

	npu_task_watchdog_start(core_id);

	/* prepare data for npu */
	if(NPU_TO_START != ioread32((void *)SOC_ICS_START_ADDR((unsigned long)reg_space->config_reg_virt_addr))) {
		if (kfifo_peek(&task_fifo_ptr->FIFO_TaskElements, &head)) {
			/* update pte */
			npu_task_smmu_pte_update(core_id);

			memset(&task_fifo_ptr->boot_inst_set.boot_inst_tmp[0], 0, sizeof(task_fifo_ptr->boot_inst_set.boot_inst_tmp));
			memcpy(&task_fifo_ptr->boot_inst_set.boot_inst_tmp[0], &task_fifo_ptr->boot_inst_set.boot_inst[0], sizeof(task_fifo_ptr->boot_inst_set.boot_inst_tmp));

			/* SET INST SIZE */
			change_inst_data(core_id, head.offchipInstSize, 152);
			change_inst_data(core_id, head.offchipInstSize, 283);
			change_inst_data(core_id, head.offchipInstSize, 21);

			/* SET_INST_ADDR */
			change_inst_data(core_id, head.offchipInstAddr, 87);

			set_offchip_inst_addr(core_id, 0);
			iowrite32(task_fifo_ptr->boot_inst_set.npu_access_ddr_addr, (void *)SOC_ICS_BASE_ADDR_ADDR((unsigned long)reg_space->config_reg_virt_addr));

			/* start ipu */
			iowrite32(NPU_TO_STOP, (void *)(void *)SOC_ICS_START_ADDR((unsigned long)reg_space->config_reg_virt_addr));
			iowrite32(NPU_STATUS_UNFINISH, (void *)SOC_ICS_STATUS_ADDR((unsigned long)reg_space->config_reg_virt_addr));

			/* whether the function is supported to judge in the core module */
			npu_task_perf_monitor_open(core_id);

			iowrite32(NPU_TO_START, (void *)SOC_ICS_START_ADDR((unsigned long)reg_space->config_reg_virt_addr));

			NPU_PRINT("core[%u] START COMPUTE, offchipInstAddr=0x%pK", core_id, (void *)head.offchipInstAddr);
		} else {
			NPU_ERR("kfifo_peek empty!");
		}
	}

	return SUCCESS;
}

bool npu_get_wq_task(unsigned int core_id, NPU_TASK_PROCESS_E restore_reason)
{
	taskElement Element;
	NPU_TASK_WQ_PRIVATE_S *task_fifo_ptr = &task_wq_private[core_id];

	/* NPU irq comes means the first element of FIFO_TaskElements is finished
       and this task must be compute task, so remove it from queue. */
	if (kfifo_get(&task_fifo_ptr->FIFO_TaskElements, &Element)) {
		up(&task_fifo_ptr->task_fifo_sem);
		if (!CHECK_WQ_TASK_TYPE(Element.taskType)) { //lint !e650
			NPU_ERR("Get error type of first task from kfifo!");
			return false;
		}
	} else {
		NPU_ERR("Get element from kfifo faild!");
		return false;
	}

	if (!kfifo_is_empty(&task_fifo_ptr->FIFO_TaskElements)) {
		if (kfifo_peek(&task_fifo_ptr->FIFO_TaskElements, &Element)) {/*to check next taskType*/

			if (!CHECK_WQ_TASK_TYPE(Element.taskType)) { //lint !e650
				NPU_ERR("Get error type of next task from kfifo!");
				return false;
			}
			/* handle sync task, until queue head is compute task or queue is empty */
			while (NPU_SYNC_TASK == Element.taskType) {
				/* wakeup user process */
				if (NULL != Element.ptaskFlag) {
					*Element.ptaskFlag = NPU_TASK_FLAG_SYNC_DONE;
				}
				/* return task restore reason */
				if (NULL != Element.ptaskStatus) {
					*Element.ptaskStatus = restore_reason;
				}

				wake_up_interruptible(&task_fifo_ptr->sync_wq);
				/* NPU power down! */
				npu_task_pm_put(core_id, 1);

				if (!kfifo_get(&task_fifo_ptr->FIFO_TaskElements, &Element)) {
					NPU_ERR("Get element from kfifo faild!");
					break;
				}
				up(&task_fifo_ptr->task_fifo_sem);
				/*to check next taskType*/
				if (!kfifo_peek(&task_fifo_ptr->FIFO_TaskElements, &Element)) {
					break;
				}

				if (!CHECK_WQ_TASK_TYPE(Element.taskType)) { //lint !e650
					NPU_ERR("Get error sync task type from kfifo!");
					return false;
				}
			}
		} else {
			NPU_ERR("Peek element from kfifo failed while fifo is not empty!");
			return false;
		}
	}else {
		NPU_ERR("There is only one task in FIFO! task_type=%d",Element.taskType);
		if (NPU_COMPUTE_TASK == Element.taskType) {
			/* NPU power down! */
			npu_task_pm_put(core_id, 1);
        }
	}

	return true;
}

void npu_wq_task_proc(unsigned int core_id)
{
	NPU_TASK_WQ_PRIVATE_S *task_fifo_ptr = &task_wq_private[core_id];

	while (npu_start_wq_task(core_id)) {
		NPU_ERR("start NPU fail!");
		if (!npu_get_wq_task(core_id, TASK_PROCESS_OK)) {
			NPU_ERR("cambricon_NPU_workqueue false when kill head task!");
		}

		if (kfifo_is_empty(&task_fifo_ptr->FIFO_TaskElements)) {
			break;
		}
	}
}

void npu_start_wq_next_task(unsigned int core_id, NPU_TASK_PROCESS_E restore_reason)
{
	NPU_TASK_WQ_PRIVATE_S *task_fifo_ptr = &task_wq_private[core_id];

	npu_get_wq_task(core_id, restore_reason);
	/*Start next task*/
	if (!kfifo_is_empty(&task_fifo_ptr->FIFO_TaskElements)) {
		/* handle compute task */
		NPU_DEBUG("to start NPU");
		npu_wq_task_proc(core_id);
	} else {
		NPU_DEBUG("FIFO_TaskElements is empty!");
	}
	return;
}

int npu_push_wq_task(const void *arg, unsigned int irq_num)
{
	unsigned int core_id;
	taskElement element;
	NPU_SSWQ_TASK_ELEMENT_S task_element;
	NPU_TASK_WQ_PRIVATE_S *task_fifo_ptr = NULL;
	taskFlag_t taskFlag = NPU_TASK_FLAG_NONE;
	NPU_TASK_PROCESS_E ptaskStatus = TASK_PROCESS_OK;
	memcpy((void *)&task_element, arg, sizeof(NPU_SSWQ_TASK_ELEMENT_S));
	core_id = task_element.core_id;
	if (!CHECK_WQ_TASK_TYPE(task_element.task_type)) { //lint !e650
		NPU_ERR("Input task_type is error!task_type:%d", task_element.task_type);
		return -EINVAL;
	}
	task_fifo_ptr = &task_wq_private[core_id];
	if(down_interruptible(&task_fifo_ptr->task_fifo_sem)){
		NPU_ERR("Request task_fifo_sem failed!");
		return -EBUSY;
	}

	/*Caution : Already obtain task_fifo_sem !!*/
	disable_irq(irq_num);
	npu_task_fifo_opt_lock(task_element.core_id);

	element.taskType        = task_element.task_type;
	element.offchipInstAddr = task_element.inst_addr_off;
	element.offchipInstSize = task_element.inst_size;
	task_fifo_ptr->boot_inst_set.npu_access_ddr_addr = task_element.ddr_base_addr >> 20;
	element.taskId          = task_element.task_id;
	element.prior           = task_element.task_priority;
	element.ptaskFlag       = NULL;/*Ensure point correct*/
	element.ptaskStatus     = NULL;

	if (kfifo_is_empty(&task_fifo_ptr->FIFO_TaskElements)) {
		/*First task is SYNC while fifo is empty should return user thread at once*/
		if (NPU_SYNC_TASK != element.taskType) {
			kfifo_put(&task_fifo_ptr->FIFO_TaskElements, element);
			NPU_INFO("NPU_COMPUTE_TASK to start core[%u]!", core_id);
			npu_wq_task_proc(core_id);
		} else {
			up(&task_fifo_ptr->task_fifo_sem);
		}
	} else {
		if (unlikely(NPU_SYNC_TASK == element.taskType)) {
			/*Set sync flag as wake up condition*/
			taskFlag = NPU_TASK_FLAG_SYNC_WAITING;
			element.ptaskFlag = &taskFlag;
			element.ptaskStatus = &ptaskStatus;
			kfifo_put(&task_fifo_ptr->FIFO_TaskElements, element);

			npu_task_fifo_opt_unlock(core_id);

			enable_irq(irq_num);
			/* sync task sleeping */
			NPU_INFO("NPU_SYNC_TASK core[%u] is waiting!", core_id);
			while(wait_event_interruptible(task_fifo_ptr->sync_wq, NPU_TASK_FLAG_SYNC_DONE == taskFlag));
			NPU_INFO("NPU_SYNC_TASK core[%u] wait for success!ptaskStatus=%d", core_id, ptaskStatus);
			return ptaskStatus;
		} else {
			NPU_DEBUG("NPU_COMPUTE_TASK core[%u] add to queue!", core_id);
			kfifo_put(&task_fifo_ptr->FIFO_TaskElements, element);
		}
	}

	npu_task_fifo_opt_unlock(core_id);

	enable_irq(irq_num);

	return SUCCESS;
}

int npu_task_wq_init(void)
{
	int ret = SUCCESS;

	task_wq_private = kzalloc(sizeof(NPU_TASK_WQ_PRIVATE_S) * g_task_private->core_num, GFP_KERNEL);
	if (NULL == task_wq_private) {
		NPU_ERR("Failed to allocate memory for task_wq_private!");
		return -ENOMEM;
	}

	/* create and init queue FIFO_TaskElements */
	ret = kfifo_alloc(&task_wq_private[NPU_CORE_0].FIFO_TaskElements, TASKQUEUE_SIZE * sizeof(taskElement), GFP_KERNEL);
	if(ret != 0) {
		NPU_ERR("Failed to allocate kfifo for core0 FIFO_TaskElements!");
		kfree(task_wq_private);
		task_wq_private = NULL;
		return -EINVAL;
	}
	/* init npu boot instruct */
	npu_boot_inst_init(NPU_CORE_0);
	/* init sema */
	sema_init(&task_wq_private[NPU_CORE_0].task_fifo_sem, TASKQUEUE_SIZE);
	/* init wait queue sync_wq */
	init_waitqueue_head(&task_wq_private[NPU_CORE_0].sync_wq);

	/* for cs */
	if (NPU_VERSION_V200 == hard_version) {
	    /* create and init queue FIFO_TaskElements */
		ret = kfifo_alloc(&task_wq_private[NPU_CORE_1].FIFO_TaskElements, TASKQUEUE_SIZE * sizeof(taskElement), GFP_KERNEL);
		if(ret != 0) {
			NPU_ERR("Failed to allocate kfifo for core1 FIFO_TaskElements!");
			kfifo_free(&task_wq_private[NPU_CORE_0].FIFO_TaskElements);
			kfree(task_wq_private);
			task_wq_private = NULL;
			return -EINVAL;
		}
		/* init npu boot instruct */
		npu_boot_inst_init(NPU_CORE_1);
		/* init sema */
		sema_init(&task_wq_private[NPU_CORE_1].task_fifo_sem, TASKQUEUE_SIZE);
		/* init wait queue sync_wq */
		init_waitqueue_head(&task_wq_private[NPU_CORE_1].sync_wq);
	}

	return SUCCESS;
}

void npu_task_wq_exit(void)
{

	kfifo_free(&task_wq_private[NPU_CORE_0].FIFO_TaskElements);

	/* for cs */
	if (NPU_VERSION_V200 == hard_version) {
		kfifo_free(&task_wq_private[NPU_CORE_1].FIFO_TaskElements);
	}

	if (NULL != task_wq_private) {
		kfree(task_wq_private);
		task_wq_private = NULL;
	}

	return;
}
