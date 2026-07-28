#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <asm/io.h>
#include <linux/ion.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <asm/uaccess.h>

#include "soc_ics_interface.h"
#include "npu_public.h"
#include "npu_task_public.h"
#include "npu_task_wq.h"
#include "npu_mntn.h"

#define NPU_IRQ_CLEAR_IRQ_LEVEL1_NS     (0x2)
#define NPU_IRQ_CLEAR_IRQ_NS            (0x00000001)

#define INTERRUPTINST_INTERRUPT_BIT     (7)
#define INTERRUPTINST_INTERRUPT_MASK    (0x1UL<<INTERRUPTINST_INTERRUPT_BIT)
#define CT_INTERRUPT_BIT                (6)
#define CT_INTERRUPT_MASK               (0x1UL<<CT_INTERRUPT_BIT)
#define NPU_CONTROL_INTERRUPT_BIT       (5)
#define NPU_CONTROL_INTERRUPT_MASK      (0x1UL<<NPU_CONTROL_INTERRUPT_BIT)
#define WATCH_DOG_BIT                   (4)
#define WATCH_DOG_MASK                  (0x1UL<<WATCH_DOG_BIT)
#define IS_NORMAL_FINISH_BIT            (0)
#define IS_NORMAL_FINISH_MASK           (0x1UL<<IS_NORMAL_FINISH_BIT)

/* NPU start up reg */
#define NPU_START_REG     (0x18)
/* NPU payload finish status reg */
#define NPU_STATUS_REG    (0x20)
/* NPU Version reg */
#define NPU_VERSION_REG   (0x40)

#define NPU_SSWQ_TASK_IRQ_MASK (WORK_QUEUE_ALL_FINISH_IS_MASK | WORK_QUEUE_MASK_FINISH_IS_MASK | WORK_QUEUE_FINISH_IS_MASK)

#define NPU_CHECK_CORE_NUM_VALID(core_num) (core_num > MAX_SUPPORT_CORE_NUM)

#define NPU_DUALINT_TO_LONG(num1, num2)  (((unsigned long)(num1) << 32) | (unsigned long)(num2))

NPU_TASK_PRIVATE_S* g_task_private;

int npu_task_pm_get(unsigned int core_id, unsigned int get_num)
{
	unsigned int index;

	/* NPU power up! */
	if ((get_num > NPU_PM_VOTE_NUM_MAX) || (NULL == g_task_private->callback_fun.pf_npu_pm_get)) {
		NPU_ERR("Input parameter is NULL!");
		return -EFAULT;
	}

	for (index = 0; index < get_num; index++) {
		g_task_private->callback_fun.pf_npu_pm_get(core_id);
	}

	return SUCCESS;
}

int npu_task_pm_put(unsigned int core_id, unsigned int put_num)
{
	unsigned int index;

	/* NPU power down! */
	if ((put_num > NPU_PM_VOTE_NUM_MAX) || (NULL == g_task_private->callback_fun.pf_npu_pm_put)) {
		NPU_ERR("Input parameter is NULL!");
		return -EINVAL;
	}

	for (index = 0; index < put_num; index++) {
		g_task_private->callback_fun.pf_npu_pm_put(core_id);
	}

	return SUCCESS;
}

int npu_task_update_profile(unsigned int core_id)
{
	if (NULL == g_task_private->callback_fun.pf_npu_update_profile) {
		NPU_ERR("Input parameter is NULL!");
		return -EINVAL;
	}

	return g_task_private->callback_fun.pf_npu_update_profile(core_id);
}

int npu_task_reset_proc(unsigned int core_id)
{
	if (NULL == g_task_private->callback_fun.pf_npu_reset) {
		NPU_ERR("Input parameter is NULL!");
		return -EINVAL;
	}
	g_task_private->callback_fun.pf_npu_reset(core_id);

	return SUCCESS;
}

int npu_task_fifo_opt_lock(unsigned int core_id)
{
	if (NULL == g_task_private->callback_fun.pf_npu_common_lock) {
		NPU_ERR("Input parameter is NULL!");
		return -EINVAL;
	}
	g_task_private->callback_fun.pf_npu_common_lock(core_id);

	return SUCCESS;
}

int npu_task_fifo_opt_unlock(unsigned int core_id)
{
	if (NULL == g_task_private->callback_fun.pf_npu_common_unlock) {
		NPU_ERR("Input parameter is NULL!");
		return -EINVAL;
	}
	g_task_private->callback_fun.pf_npu_common_unlock(core_id);

	return SUCCESS;
}

int npu_task_smmu_pte_update(unsigned int core_id)
{
	if (NULL == g_task_private->callback_fun.pf_npu_smmu_pte_update) {
		NPU_ERR("Input parameter is NULL!");
		return -EINVAL;
	}
	g_task_private->callback_fun.pf_npu_smmu_pte_update(core_id);

	return SUCCESS;
}

int npu_task_perf_monitor_open(unsigned int core_id)
{
	if (NULL == g_task_private->callback_fun.pf_npu_perf_monitor_open) {
		NPU_ERR("Input parameter is NULL!");
		return -EINVAL;
	}
	g_task_private->callback_fun.pf_npu_perf_monitor_open(core_id);

	return SUCCESS;
}

static void npu_task_callback_fun_init(NPU_CALLBACK_FUNC_S* callback_fun_ptr)
{
	memcpy(&g_task_private->callback_fun, callback_fun_ptr, sizeof(NPU_CALLBACK_FUNC_S));
	return;
}

static int npu_task_invalid(NPU_SSWQ_TASK_ELEMENT_S *task)
{
	int ret;

	CHECK_COREID_INVALID_RETURN(task->core_id);

	if (NPU_TASK_ENUM_END <= task->task_type) {
		NPU_ERR("Input task type is error! task_type=%d", task->task_type);
		return -EINVAL;
	}

	if (0 != task->ddr_base_addr) {
		NPU_ERR("task.ddr_base_addr=%d, error", task->task_type);
		return -EINVAL;
	}

	ret = g_task_private->callback_fun.pf_npu_mem_mngr_check(task->inst_addr_off, task->inst_size);

	return ret;
}

int npu_push_task(unsigned int* arg, unsigned int irq_num)
{
	int ret = SUCCESS;
	NPU_SSWQ_TASK_ELEMENT_S task;

	if (!arg) {
		NPU_ERR("Input parameter is NULL!");
		return -EINVAL;
	}

	memset(&task, 0, sizeof(NPU_SSWQ_TASK_ELEMENT_S));

	/* task.task_id = arg[3], task.thread_id = arg[4], task.task_config = arg[5],
		task.task_priority = arg[6] is reserved */
	task.core_id       = arg[0];
	task.task_type     = arg[1];
	task.inst_size     = arg[2];
	task.inst_addr_off = NPU_DUALINT_TO_LONG(arg[8], arg[7]);
	task.ddr_base_addr = NPU_DUALINT_TO_LONG(arg[10], arg[9]);

	NPU_INFO("Core_id %d, Task-Type %d, Task-Prio %d, I-Addr 0x%lx, I-Size 0x%x, Task-Id 0x%x, Thread-Id 0x%x, Config 0x%x, DDR-Base 0x%lx",
				task.core_id,
				task.task_type,
				task.task_priority,
				task.inst_addr_off,
				task.inst_size,
				task.task_id,
				task.thread_id,
				task.task_config,
				task.ddr_base_addr);

	if (npu_task_invalid(&task)) {
		return -EINVAL;
	}

	ret = npu_push_wq_task((void *)&task, irq_num);
	if (SUCCESS != ret) {
		NPU_ERR("Call NPU_sswq_push_task failed! ret=%d", ret);
		return ret;
	}

	return SUCCESS;
}

void npu_task_restore(unsigned int core_id, NPU_TASK_PROCESS_E restore_reason)
{
	npu_start_wq_next_task(core_id, restore_reason);
	return;
}

void npu_task_watchdog_start(unsigned int core_id)
{
	unsigned long config_reg_virt_addr = (unsigned long)g_task_private->reg_space[core_id].config_reg_virt_addr;
	unsigned long watchdog_timeout = g_task_private->watchdog_timer[core_id];
	unsigned int tmp;

	NPU_DEBUG("Enter npu_task_watchdog_start!core_id=%d watchdog_timeout=%ld", core_id, watchdog_timeout);
	tmp = (unsigned int)(watchdog_timeout & 0xffffffff);
	NPU_DEBUG("Enter npu_task_watchdog_start!core_id=%d low32bit=%d", core_id, tmp);
	iowrite32(tmp, (void *)SOC_NPU_WATCH_DOG_TIMER_LOW_ADDR(config_reg_virt_addr));
	tmp = (unsigned int)((watchdog_timeout>>32) & 0xffffffff);
	NPU_DEBUG("Enter npu_task_watchdog_start!core_id=%d high32bit=%d", core_id, tmp);
	iowrite32(tmp, (void *)SOC_NPU_WATCH_DOG_TIMER_HIGH_ADDR(config_reg_virt_addr));
	return;
}

static void npu_core_irq_task_proc(unsigned int core_id, unsigned int* interrupt_status_ptr)
{
	bool reset_npu_flag = false;
	unsigned int interrupt_status = interrupt_status_ptr[core_id];
	unsigned int fault_status = 0;
	unsigned long config_reg_virt_addr = (unsigned long)g_task_private->reg_space[core_id].config_reg_virt_addr;

	if (!interrupt_status) {
		NPU_ERR("interrupt status is error!interrupt_status=%d", interrupt_status);
		return;
	}

	/*******************INTERRUPTINST_INTERRUPT***********/
	if (interrupt_status & INTERRUPTINST_INTERRUPT_MASK) {
		fault_status = ioread32((void *)SOC_NPU_INTERRUPT_INST_ADDR(config_reg_virt_addr));
		iowrite32(0, (void *)SOC_NPU_INTERRUPT_INST_ADDR(config_reg_virt_addr));
		NPU_ERR("core_id %d INTERRUPTINST_INTERRUPT error, fault_status=0x%x", core_id, fault_status);
	}

	/********************NPU_INTERRUPT*******************/
	if (interrupt_status & CT_INTERRUPT_MASK) {
		NPU_ERR("core_id %d CT_INTERRUPT error, fault_status=0x%x", core_id, fault_status);

		reset_npu_flag = true;
	}

	/********************NPU_CONTROL_INTERRUPT**********/
	if (interrupt_status & NPU_CONTROL_INTERRUPT_MASK) {
		NPU_ERR("core_id %d NPU_CONTROL_INTERRUPT error, fault_status=0x%x, npu_control id:%x%x",
				 core_id,
				 fault_status,
				 ioread32((void *)(SOC_NPU_CONTROL_ID_ADDR(config_reg_virt_addr + 4))),
				 ioread32((void *)(SOC_NPU_CONTROL_ID_ADDR(config_reg_virt_addr))));

		reset_npu_flag = true;
	}

	/********************WATCH_DOG**********************/
	if (interrupt_status & WATCH_DOG_MASK) {
		/* do_watch_dog_interrupt_service();*/
		NPU_ERR("core_id %d WATCH_DOG error, fault_status=0x%x", core_id, fault_status);
		reset_npu_flag = true;
	}

	/********************self start interrupt************************/
	if (interrupt_status & NPU_SSWQ_TASK_IRQ_MASK) {
		NPU_ERR("core_id %d self start interrupt error, fault_status=0x%x", core_id, fault_status);
		reset_npu_flag = true;
	}

	if (reset_npu_flag) {
		npu_mntn_interrupt_exc_proc(core_id, interrupt_status);

		DSM_AI_KERN_ERROR_REPORT(DSM_AI_KERN_WTD_TIMEOUT_ERR_NO, "NPU abnormal interruption need to reset system! core_id=0x%x interrupt_status=0x%x\n", core_id, interrupt_status);

		/* reset npu */
		npu_task_reset_proc(core_id);
		/* restore task */
		npu_task_restore(core_id, TASK_INTERRUPT_ERR);
		return;
	}

	/********************NORMAL_FINISH*******************/
	if (interrupt_status & IS_NORMAL_FINISH_MASK) {

		npu_start_wq_next_task(core_id, TASK_PROCESS_OK);

	}

	return;
}

static void npu_core_irq_clear(unsigned int core_id, unsigned long irq_io_addr, unsigned int* interrupt_status_ptr)
{
	bool reset_npu_flag = false;
	unsigned int ret_value;
	unsigned int interrupt_status;
	unsigned int loop_cnt = 0;
	unsigned long config_reg_virt_addr = (unsigned long)g_task_private->reg_space[core_id].config_reg_virt_addr;

	interrupt_status = ioread32((void *)SOC_NPU_IS_ADDR(config_reg_virt_addr));
	interrupt_status_ptr[core_id] = interrupt_status;
	while(interrupt_status){
		NPU_PRINT("core[%u] lv1_int_status=0x%x loop_cnt=%d", core_id, interrupt_status, loop_cnt);
		if (loop_cnt > 3) {
			NPU_ERR("npu_core_irq_clear loop timeout!lv1_int_status=0x%x", interrupt_status);
			break;
		}
		/********************NPU_INTERRUPT*******************/
		if (interrupt_status & CT_INTERRUPT_MASK) {
			reset_npu_flag = true;
		}

		/********************NPU_CONTROL_INTERRUPT**********/
		if (interrupt_status & NPU_CONTROL_INTERRUPT_MASK) {
			reset_npu_flag = true;
		}

		/********************WATCH_DOG**********************/
		if (interrupt_status & WATCH_DOG_MASK) {
			reset_npu_flag = true;
		}

		/********************self start interrupt*******************/
		if (interrupt_status & NPU_SSWQ_TASK_IRQ_MASK) {

			ret_value = ioread32((void *)SOC_NPU_SSWQ_WORK_QUEUE_FINISH_ADDR(config_reg_virt_addr));
			ret_value &= WORK_QUEUE_FINISH_MASK;
			iowrite32(ret_value, (void *)SOC_NPU_SSWQ_WORK_QUEUE_FINISH_ADDR(config_reg_virt_addr));
			reset_npu_flag = true;
		}

		if (interrupt_status & IS_NORMAL_FINISH_MASK) {
			/* to clear normal finish reg */
			iowrite32(NPU_STATUS_UNFINISH, (void *)SOC_ICS_STATUS_ADDR(config_reg_virt_addr));
		}

		/* to clear finish reg */
		iowrite32(interrupt_status, (void *)SOC_NPU_IS_ADDR(config_reg_virt_addr));
		/* to clear finish reg high 32bit */
		iowrite32(0, (void *)SOC_NPU_IS_ADDR(config_reg_virt_addr + 0X4));
		interrupt_status = ioread32((void *)SOC_NPU_IS_ADDR(config_reg_virt_addr));
		if (interrupt_status) {
			interrupt_status_ptr[core_id] = interrupt_status;
			NPU_ERR("clear interrupt_status is error!lv1_int_status=0x%x", interrupt_status);
		}
		if (reset_npu_flag) {
			break;
		}
		loop_cnt++;
	}

	/* to  clear non-secure interrupt */
	iowrite32(NPU_IRQ_CLEAR_IRQ_LEVEL1_NS | NPU_IRQ_CLEAR_IRQ_NS, (void *)SOC_ICS_IRQ_CLR_NS_ADDR(irq_io_addr));

	return;
}

void npu_core_irq_handler(unsigned int core_id, unsigned long irq_io_addr)
{
	unsigned int interrupt_status[MAX_SUPPORT_CORE_NUM] = {0};

	CHECK_COREID_INVALID_RETURN_VOID(core_id);

	npu_core_irq_clear(core_id, irq_io_addr, interrupt_status);
	npu_core_irq_task_proc(core_id, interrupt_status);
}

static int npu_task_get_reg_space(int core_id, struct platform_device *pdev)
{
	int ret = SUCCESS;
	unsigned long size;
	struct resource *res = NULL;
	NPU_REG_SPACE_S* reg_space_ptr = NULL;

	reg_space_ptr = &g_task_private->reg_space[core_id];
	if (NULL == reg_space_ptr) {
		NPU_ERR("Input parameter is error!core_id:%d", core_id);
		return -EINVAL;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, core_id * 2);
	if (!res) {
		NPU_ERR("failed to get instruction resource!");
		return -ENXIO;
	}

	size = resource_size(res);

	reg_space_ptr->config_reg_phys_addr = res->start;
	reg_space_ptr->config_reg_length = (unsigned int)size;

	NPU_DEBUG("coreid[%d], config_reg_phys_addr[%llu], config_reg_length[0x%x]",
				core_id, reg_space_ptr->config_reg_phys_addr, reg_space_ptr->config_reg_length);

	reg_space_ptr->cfg_mem = request_mem_region(res->start, size, pdev->name);
	if (NULL == reg_space_ptr->cfg_mem) {
		NPU_ERR("failed to get configure registers memory region!");
		return -ENOENT;
	}

	reg_space_ptr->config_reg_virt_addr = ioremap(res->start, size);
	if (!reg_space_ptr->config_reg_virt_addr) {
		NPU_ERR("ioremap() of configure registers resource failed!");
		ret = -ENXIO;
		goto release_cfg_mem;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1 + (core_id * 2));
	if (!res) {
		NPU_ERR("failed to get instruction resource\n");
		ret = -ENXIO;
		goto config_unmap;
	}

	size = resource_size(res);

	reg_space_ptr->inst_ram_phys_addr = res->start;
	reg_space_ptr->inst_ram_size = (unsigned int)size;

	NPU_DEBUG("coreid[%d], inst_ram_phys_addr[%llu], inst_ram_size[0x%x]\n",
				core_id, reg_space_ptr->inst_ram_phys_addr, reg_space_ptr->inst_ram_size);

	reg_space_ptr->inst_mem = request_mem_region(res->start, size, pdev->name);
	if (reg_space_ptr->inst_mem == NULL) {
		NPU_ERR("failed to get instruction memory region\n");
		ret = -ENOENT;
		goto config_unmap;
	}

	reg_space_ptr->inst_ram_virt_addr = ioremap(res->start, size);
	if (!reg_space_ptr->inst_ram_virt_addr) {
		NPU_ERR("ioremap() of instruction resource failed\n");
		ret = -ENXIO;
		goto release_inst_mem;
	}

	return SUCCESS;

release_inst_mem:
	if (reg_space_ptr->inst_mem) {
		release_mem_region(reg_space_ptr->inst_mem->start, resource_size(reg_space_ptr->inst_mem));
		reg_space_ptr->inst_mem = NULL;
	}

config_unmap:
	if (reg_space_ptr->config_reg_virt_addr) {
		iounmap(reg_space_ptr->config_reg_virt_addr);
		reg_space_ptr->config_reg_virt_addr = NULL;
	}

release_cfg_mem:
	if (reg_space_ptr->cfg_mem) {
		release_mem_region(reg_space_ptr->cfg_mem->start, resource_size(reg_space_ptr->cfg_mem));
		reg_space_ptr->cfg_mem = NULL;
	}

	return ret;
}

int npu_task_finish_interrupt_clear(unsigned int core_id)
{
 	unsigned long config_reg_virt_addr = (unsigned long)g_task_private->reg_space[core_id].config_reg_virt_addr;

	CHECK_COREID_INVALID_RETURN(core_id);
	/* clear NPU finished status */
	iowrite32(NPU_STATUS_UNFINISH, (void *)SOC_ICS_STATUS_ADDR(config_reg_virt_addr));

	return SUCCESS;
}

static void npu_task_reg_space_release(void)
{
	unsigned int core_id;
	NPU_REG_SPACE_S* reg_space = NULL;

	for (core_id = 0; core_id < g_task_private->core_num; core_id++) {

		reg_space = &g_task_private->reg_space[core_id];

		if (reg_space->config_reg_virt_addr) {
			iounmap(reg_space->config_reg_virt_addr);
			reg_space->config_reg_virt_addr = NULL;
		}

		if (reg_space->inst_ram_virt_addr) {
			iounmap(reg_space->inst_ram_virt_addr);
			reg_space->inst_ram_virt_addr = NULL;
		}

		if (reg_space->inst_mem) {
			release_mem_region(reg_space->inst_mem->start, resource_size(reg_space->inst_mem));
			reg_space->inst_mem = NULL;
		}

		if (reg_space->cfg_mem) {
			release_mem_region(reg_space->cfg_mem->start, resource_size(reg_space->cfg_mem));
			reg_space->cfg_mem = NULL;
		}
	}

	return;
}

static int npu_task_reg_space_init(struct platform_device *pdev)
{
	int ret = SUCCESS;
	unsigned int core_id = 0;

	for(core_id = 0; core_id < g_task_private->core_num; core_id++) {
		ret = npu_task_get_reg_space(core_id, pdev);
		if (ret) {
			npu_task_reg_space_release();
			NPU_ERR("Call npu_task_get_reg_space is failed!");
		}
	}

	return ret;
}

static void npu_task_watchdog_timer_init(void)
{
	unsigned int core_id = 0;

	for(core_id = 0; core_id < g_task_private->core_num; core_id++) {
		g_task_private->watchdog_timer[core_id] = NPU_WATCHDOG_TIMEOUT;
	}

	return;
}

int npu_task_init(struct platform_device *pdev, unsigned int core_num, NPU_CALLBACK_FUNC_S* callback_fun_ptr)
{
	int ret = SUCCESS;
	int ret_value;

	if ((NULL == pdev) || (NULL == callback_fun_ptr) || (NPU_CHECK_CORE_NUM_VALID(core_num))) {
		NPU_ERR("Input parameter error!core_num=%d", core_num);
		return -EINVAL;
	}

	g_task_private = kzalloc(sizeof(NPU_TASK_PRIVATE_S), GFP_KERNEL);
	if (NULL == g_task_private) {
		NPU_ERR("Failed to allocate memory!");
		return -ENOMEM;
	}

	g_task_private->core_num = core_num;

	npu_task_callback_fun_init(callback_fun_ptr);

	ret = npu_task_reg_space_init(pdev);
	if (ret) {
		NPU_ERR("Call npu_task_reg_space_init is failed!");
		ret_value = -EINVAL;
		goto free;
	}

	ret = npu_task_wq_init();
	if (ret) {
		NPU_ERR("Failed npu_task_wq_init!");
		ret_value = -EINVAL;
		goto task_init_error;
	}

	npu_task_watchdog_timer_init();

	return SUCCESS;

task_init_error:
	npu_task_reg_space_release();
free:
	if (NULL != g_task_private) {
		kfree(g_task_private);
		g_task_private = NULL;
	}
	return ret_value;

}

void npu_task_exit(void)
{
	npu_task_wq_exit();

	if (NULL != g_task_private) {
		npu_task_reg_space_release();
		kfree(g_task_private);
		g_task_private = NULL;
	}

	return;
}

