#ifndef _NPU_TASK_WQ_H_
#define _NPU_TASK_WQ_H_

/* the NPU task type */
typedef enum taskType {
	NPU_TASK_ENUM_START = 0,
	NPU_NONE_TASK = NPU_TASK_ENUM_START,
	/*First*/
	NPU_COMPUTE_TASK = 1,
	NPU_SYNC_TASK    = 4,
	//Add new to here!
	NPU_TASK_ENUM_END
} taskType_t;

typedef enum taskFlag{
	NPU_TASK_FLAG_START = 0,
	NPU_TASK_FLAG_NONE = NPU_TASK_FLAG_START,
	/*First*/
	NPU_TASK_FLAG_SYNC_WAITING,
	NPU_TASK_FLAG_SYNC_DONE,
	//Add new to here!
	NPU_TASK_FLAG_END
} taskFlag_t;

/* the NPU task element */
typedef struct taskStruct {
	unsigned int taskType;//define different cmd type
	NPU_TASK_PROCESS_E* ptaskStatus;
	taskFlag_t* ptaskFlag;//use to mark task status
	unsigned long offchipInstAddr;
	unsigned long offchipInstSize;
	unsigned long taskId;
	int prior;
} taskElement;

extern NPU_TASK_PRIVATE_S* g_task_private;

int npu_push_wq_task(const void *arg, unsigned int irq_num);

int npu_task_wq_init(void);

void npu_task_wq_exit(void);

bool npu_get_wq_task(unsigned int core_id, NPU_TASK_PROCESS_E restore_reason);

void npu_start_wq_next_task(unsigned int core_id, NPU_TASK_PROCESS_E restore_reason);

#endif