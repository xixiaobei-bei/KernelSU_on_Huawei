#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/list.h>

#include "npu_proc_ctx.h"
#include "npu_shm.h"
#include "npu_calc_channel.h"
#include "npu_calc_cq.h"
#include "npu_stream.h"
#include "npu_cache.h"
#include "drv_log.h"
#include "npu_event.h"
#include "npu_model.h"
#include "npu_task.h"
#include "npu_platform.h"
#include "npu_calc_sq.h"
#include <linux/bitops.h>


static struct devdrv_cq_report_int_ctx g_cq_int_ctx;
static u64 g_recv_cq_int_num = 0;//use for debug
static u64 g_find_cq_index_called_num = 0;//use for debug

void devdrv_proc_ctx_init(struct devdrv_proc_ctx *proc_ctx)
{
	INIT_LIST_HEAD(&proc_ctx->cq_list);
	INIT_LIST_HEAD(&proc_ctx->sink_stream_list);
	INIT_LIST_HEAD(&proc_ctx->stream_list);
	INIT_LIST_HEAD(&proc_ctx->event_list);
	INIT_LIST_HEAD(&proc_ctx->model_list);
	INIT_LIST_HEAD(&proc_ctx->task_list);
	INIT_LIST_HEAD(&proc_ctx->dev_ctx_list);
	INIT_LIST_HEAD(&proc_ctx->message_list_header);
	INIT_LIST_HEAD(&proc_ctx->ipc_msg_send_head);
	INIT_LIST_HEAD(&proc_ctx->ipc_msg_return_head);

	proc_ctx->pid = current->tgid;
	proc_ctx->sink_stream_num = 0;
	proc_ctx->stream_num = 0;
	proc_ctx->event_num = 0;
	proc_ctx->cq_num = 0;
	proc_ctx->model_num = 0;
	proc_ctx->task_num = 0;
	proc_ctx->send_count = 0;
	proc_ctx->receive_count = 0;

	proc_ctx->ipc_port = -1;

	proc_ctx->cq_tail_updated = 0;

	proc_ctx->should_stop_thread = 0;
	proc_ctx->mailbox_message_count.counter = 0;

	init_waitqueue_head(&proc_ctx->report_wait);
	init_waitqueue_head(&proc_ctx->ipc_wait);
	spin_lock_init(&proc_ctx->mailbox_wait_spinlock);
	sema_init(&proc_ctx->mailbox_wait, 0);
	mutex_init(&proc_ctx->stream_mutex);
	mutex_init(&proc_ctx->event_mutex);
	mutex_init(&proc_ctx->model_mutex);
	mutex_init(&proc_ctx->task_mutex);
	mutex_init(&proc_ctx->map_mutex);
	bitmap_zero(proc_ctx->stream_bitmap, DEVDRV_MAX_STREAM_ID);
	bitmap_zero(proc_ctx->event_bitmap, DEVDRV_MAX_EVENT_ID);
	bitmap_zero(proc_ctx->model_bitmap, DEVDRV_MAX_MODEL_ID);
	bitmap_zero(proc_ctx->task_bitmap, DEVDRV_MAX_TASK_ID);

}

struct devdrv_ts_cq_info *devdrv_proc_alloc_cq(struct devdrv_proc_ctx *proc_ctx)
{
	struct devdrv_ts_cq_info *cq_info = NULL;
	struct devdrv_cq_sub_info *cq_sub_info = NULL;

	u8 dev_id = 0;;

	if(proc_ctx == NULL){
		devdrv_drv_err("proc_ctx is null\n");
		return NULL;
	}
	dev_id = proc_ctx->devid;
	cq_info = devdrv_alloc_cq(dev_id);
	if(cq_info == NULL){
		devdrv_drv_err("npu dev %d cq_info is null\n",dev_id);
		return NULL;
	}
	cq_sub_info = (struct devdrv_cq_sub_info *)cq_info->cq_sub;
	list_add(&cq_sub_info->list, &proc_ctx ->cq_list);
	cq_sub_info-> proc_ctx = proc_ctx;
	proc_ctx ->cq_num++;

	return cq_info;
}

static int devdrv_proc_get_cq_id(struct devdrv_proc_ctx *proc_ctx,u32* cq_id)
{
	struct devdrv_cq_sub_info *cq_sub = NULL;

	if (list_empty_careful(&proc_ctx->cq_list)) {
		devdrv_drv_err("cur proc_ctx cq_list null\n");
		return -1;
	}

	cq_sub = list_first_entry(  &proc_ctx->cq_list,
								struct devdrv_cq_sub_info,
								list);

	*cq_id = cq_sub->index;

	return 0;
}

int devdrv_proc_send_alloc_stream_mailbox(struct devdrv_proc_ctx *proc_ctx)
{
	struct devdrv_stream_sub_info *stream_sub = NULL;
	struct devdrv_stream_info *stream_info = NULL;
	struct list_head *pos = NULL, *n = NULL;
	u8 cur_dev_id = 0;	// get from platform
	int ret = 0;

	if (list_empty_careful(&proc_ctx->stream_list)) {
		devdrv_drv_err("proc context stream list is empty.\n");
		return -1;
	}

	list_for_each_safe(pos, n, &proc_ctx->stream_list) {
		stream_sub = list_entry(pos, struct devdrv_stream_sub_info, list);
		if (stream_sub == NULL) {
			devdrv_drv_err("stream sub is null.\n");
			return -1;
		}
		stream_info = devdrv_calc_stream_info(cur_dev_id, stream_sub->id);
		if (stream_info->strategy == STREAM_STRATEGY_SINK){
			devdrv_drv_debug("send no mailbox for sink stream.\n");
			continue;
		}

		ret = devdrv_send_alloc_stream_mailbox(cur_dev_id,
				stream_sub->id, stream_info->cq_index);
		if (ret) {
			devdrv_drv_err("send alloc stream %d mailbox failed.\n",
					stream_info->id);
			return ret;
		}
	}

	return 0;
}

int devdrv_proc_clr_sqcq_info(struct devdrv_proc_ctx *proc_ctx)
{
	struct devdrv_stream_sub_info *stream_sub = NULL;
	struct devdrv_stream_info *stream_info = NULL;
	struct devdrv_cq_sub_info *cq_sub = NULL;
	struct list_head *pos = NULL, *n = NULL;
	u8 cur_dev_id = 0;	// get from platform

	if (list_empty_careful(&proc_ctx->cq_list)) {
		devdrv_drv_debug("proc context cq list is empty, no need clear.\n");
		return 0;
	}

	list_for_each_safe(pos, n, &proc_ctx->cq_list) {
		cq_sub = list_entry(pos, struct devdrv_cq_sub_info, list);
		if (cq_sub != NULL) {
			(void)devdrv_clr_cq_info(cur_dev_id, cq_sub->index);
		}
	}

	if (list_empty_careful(&proc_ctx->stream_list)) {
		devdrv_drv_debug("proc context sq list is empty, no need clear.\n");
		return 0;
	}

	list_for_each_safe(pos, n, &proc_ctx->stream_list) {
		stream_sub = list_entry(pos, struct devdrv_stream_sub_info, list);
		if (stream_sub != NULL) {
			stream_info = devdrv_calc_stream_info(cur_dev_id, stream_sub->id);
			if (stream_info->strategy == STREAM_STRATEGY_SINK){
				devdrv_drv_debug("send no mailbox for sink stream.\n");
				continue;
			}
			(void)devdrv_clr_sq_info(cur_dev_id, stream_info->sq_index);
		}
	}

	return 0;
}

//protect by proc_ctx->stream_mutex when called
int devdrv_proc_alloc_stream(struct devdrv_proc_ctx *proc_ctx, u32 *stream_id, u32 strategy)
{
	struct devdrv_stream_info* stream_info = NULL;
	struct devdrv_stream_sub_info *stream_sub_info = NULL;
	int ret = -1;
	u32 cq_id;

	if(proc_ctx == NULL){
		devdrv_drv_err("proc_ctx ptr is null \n");
		return -1;
	}

	if(stream_id == NULL){
		devdrv_drv_err("stream_id ptr is null \n");
		return -1;
	}

	ret = devdrv_proc_get_cq_id(proc_ctx,&cq_id);
	if(ret != 0){
		devdrv_drv_err("get cq_id from proc_ctx cq_list failed \n");
		return -1;
	}
	devdrv_drv_debug("get cq_id = %d from proc_ctx cq_list \n",cq_id);

	stream_info = devdrv_alloc_stream(cq_id, strategy);
	if(stream_info == NULL){
		devdrv_drv_err("get stream_info through cq %d failed \n",cq_id);
		return -1;
	}

	devdrv_drv_debug("alloc stream success stream_id = %d ,sq_id = %d \n"
	"cq_id = %d",stream_info->id,stream_info->sq_index,stream_info->cq_index);

	stream_info->pid = proc_ctx->pid;
	stream_sub_info = (struct devdrv_stream_sub_info *)stream_info->stream_sub;
	stream_sub_info->proc_ctx = (void *)proc_ctx;
	if (strategy == STREAM_STRATEGY_SINK) {
		list_add(&stream_sub_info->list, &proc_ctx->sink_stream_list);
		proc_ctx->sink_stream_num++;
	}
	else {
		list_add(&stream_sub_info->list, &proc_ctx->stream_list);
		proc_ctx->stream_num++;
	}

	devdrv_drv_debug("npu process_id = %d thread_id %d own sink stream num = %d, non sink stream num = %d now \n",
			proc_ctx->pid,current->pid,proc_ctx->sink_stream_num,proc_ctx->stream_num);

	*stream_id = stream_info->id;

	return 0;
}

int devdrv_proc_alloc_event(struct devdrv_proc_ctx *proc_ctx, u32* event_id_ptr)
{
	struct devdrv_event_info *event_info = NULL;

	if (proc_ctx == NULL || event_id_ptr == NULL) {
		devdrv_drv_err("proc_ctx ptr or event id ptr is null \n");
		return -EINVAL;
	}

	event_info = devdrv_alloc_event(proc_ctx->devid);
	if (event_info == NULL) {
		devdrv_drv_err("event info is null.\n");
		*event_id_ptr = DEVDRV_MAX_EVENT_ID;
		return -ENODATA;
	}
	list_add(&event_info->list, &proc_ctx->event_list);
	proc_ctx->event_num++;
	devdrv_drv_debug("npu process %d own event num = %d now \n",
						current->pid,proc_ctx->event_num);

	*event_id_ptr = event_info->id;
	return 0;
}

int devdrv_proc_free_event(struct devdrv_proc_ctx *proc_ctx, u32 event_id)
{
	int ret = 0;

	if (proc_ctx == NULL || event_id >= DEVDRV_MAX_EVENT_ID) {
		devdrv_drv_err("proc_ctx ptr is null or event id %d is invalid \n",
					event_id);
		return -EINVAL;
	}

	if (proc_ctx->event_num == 0)
	{
		devdrv_drv_err("event_num is 0 invalid \n");
		return -EINVAL;
	}

	ret = devdrv_free_event_id(proc_ctx->devid, event_id);
	if (ret != 0) {
		devdrv_drv_err("free event id failed.\n");
		return -ENODATA;
	}
	proc_ctx->event_num--;
	bitmap_clear(proc_ctx->event_bitmap, event_id, 1);
	devdrv_drv_debug("npu process %d left event num = %d.\n",
								current->pid,proc_ctx->event_num);

	return 0;
}

int devdrv_proc_alloc_model(struct devdrv_proc_ctx *proc_ctx, u32* model_id_ptr)
{
	struct devdrv_model_info *model_info = NULL;

	if (proc_ctx == NULL || model_id_ptr == NULL) {
		devdrv_drv_err("proc_ctx ptr or model id ptr is null \n");
		return -EINVAL;
	}

	model_info = devdrv_alloc_model(proc_ctx->devid);
	if (model_info == NULL) {
		devdrv_drv_err("model info is null.\n");
		*model_id_ptr = DEVDRV_MAX_MODEL_ID;
		return -ENODATA;
	}

	list_add(&model_info->list, &proc_ctx->model_list);
	proc_ctx->model_num++;
	devdrv_drv_debug("npu process %d own model num = %d now \n",
						current->pid,proc_ctx->model_num);

	*model_id_ptr = model_info->id;
	return 0;
}

int devdrv_proc_free_model(struct devdrv_proc_ctx *proc_ctx, u32 model_id)
{
	int ret = 0;

	if (proc_ctx == NULL || model_id >= DEVDRV_MAX_MODEL_ID) {
		devdrv_drv_err("proc_ctx ptr or model id ptr is null \n");
		return -EINVAL;
	}

	if (proc_ctx->model_num == 0)
	{
		devdrv_drv_err("model_num is 0 \n");
		return -EINVAL;
	}

	ret = devdrv_free_model_id(proc_ctx->devid, model_id);
	if (ret != 0) {
		devdrv_drv_err("free model id failed.\n");
		return -ENODATA;
	}

	proc_ctx->model_num--;
	bitmap_clear(proc_ctx->model_bitmap, model_id, 1);
	devdrv_drv_debug("npu process %d left model num = %d",
								current->pid,proc_ctx->model_num);

	return 0;
}

int devdrv_proc_alloc_task(struct devdrv_proc_ctx *proc_ctx, u32* task_id_ptr)
{
	struct devdrv_task_info *task_info = NULL;

	if (proc_ctx == NULL || task_id_ptr == NULL) {
		devdrv_drv_err("proc_ctx ptr or task id ptr is null \n");
		return -EINVAL;
	}

	task_info = devdrv_alloc_task(proc_ctx->devid);
	if (task_info == NULL) {
		devdrv_drv_err("task info is null.\n");
		*task_id_ptr = DEVDRV_MAX_TASK_ID;
		return -ENODATA;
	}

	list_add(&task_info->list, &proc_ctx->task_list);
	proc_ctx->task_num++;
	devdrv_drv_debug("npu process %d own task num = %d now \n",
						current->pid,proc_ctx->task_num);

	*task_id_ptr = task_info->id;
	return 0;
}

int devdrv_proc_free_task(struct devdrv_proc_ctx *proc_ctx, u32 task_id)
{
	int ret = 0;

	if (proc_ctx == NULL || task_id >= DEVDRV_MAX_TASK_ID) {
		devdrv_drv_err("proc_ctx ptr or task id ptr is null \n");
		return -EINVAL;
	}

	if (proc_ctx->task_num == 0)
	{
		devdrv_drv_err("task_num id 0.\n");
		return -ENODATA;
	}

	ret = devdrv_free_task_id(proc_ctx->devid, task_id);
	if (ret != 0) {
		devdrv_drv_err("free task id failed.\n");
		return -ENODATA;
	}
	proc_ctx->task_num--;
	bitmap_clear(proc_ctx->task_bitmap, task_id, 1);

	devdrv_drv_debug("npu process %d left task num = %d",
								current->pid,proc_ctx->task_num);

	return 0;
}

int devdrv_proc_free_stream(struct devdrv_proc_ctx* proc_ctx,u32 stream_id)
{
	struct devdrv_stream_info* stream_info = NULL;
	struct devdrv_stream_sub_info *stream_sub_info = NULL;
	u32 sq_send_count = 0;
	u8 dev_id = 0;
	int ret = -1;

	if(proc_ctx == NULL || stream_id >= DEVDRV_MAX_STREAM_ID){
		devdrv_drv_err("proc_ctx ptr is null or illegal npu stream id. stream_id=%d\n",stream_id);
		return -1;
	}

	if (stream_id < DEVDRV_MAX_NON_SINK_STREAM_ID
			&& proc_ctx->stream_num == 0) {
			devdrv_drv_err("stream_num is 0. stream_id=%d \n", stream_id);
			return -1;
	}

	if (stream_id >= DEVDRV_MAX_NON_SINK_STREAM_ID &&
		proc_ctx->sink_stream_num == 0) {
		devdrv_drv_err("sink stream_num is 0 stream_id=%d \n", stream_id);
		return -1;
	}

	dev_id = proc_ctx->devid;
	stream_info = devdrv_calc_stream_info(dev_id, stream_id);
	if(stream_info == NULL)
	{
		devdrv_drv_err("stream_info is NULL. stream_id=%d\n", stream_id);
		return -1;
	}

	stream_sub_info = (struct devdrv_stream_sub_info*)stream_info->stream_sub;

	if(test_bit(stream_id, proc_ctx->stream_bitmap) == 0) {
		devdrv_drv_err(" has already been freed! stream_id=%d \n", stream_id);
		return -1;
	}

	list_del(&stream_sub_info->list);

	ret = devdrv_free_stream(proc_ctx->devid,stream_id,&sq_send_count);
	if(ret != 0){
		devdrv_drv_err("npu process %d free stream_id %d failed \n",
										current->pid,stream_id);
		return -1;
	}
	proc_ctx->send_count += sq_send_count;

	bitmap_clear(proc_ctx->stream_bitmap, stream_id, 1);
	if (stream_id < DEVDRV_MAX_NON_SINK_STREAM_ID) {
		proc_ctx->stream_num--;
	}
	else {
		proc_ctx->sink_stream_num--;
	}

	devdrv_drv_debug("npu process %d left stream num = %d sq_send_count = %d "
									"(if stream'sq has been released) now\n",
								current->pid,proc_ctx->stream_num,sq_send_count);
	return 0;
}

static int devdrv_proc_free_single_cq(struct devdrv_proc_ctx *proc_ctx,u32 cq_id)
{
	struct devdrv_ts_cq_info *cq_info = NULL;
	struct devdrv_cq_sub_info *cq_sub_info = NULL;
	u8 dev_id = 0;;

	if (proc_ctx == NULL) {
		devdrv_drv_err("proc_ctx is null\n");
		return -1;
	}

	if (cq_id >= DEVDRV_MAX_CQ_NUM) {
		devdrv_drv_err("illegal npu cq id = %d\n",cq_id);
		return -1;
	}

	if (proc_ctx ->cq_num == 0) {
		devdrv_drv_err("cq_num is 0.\n");
		return -1;
	}

	dev_id = proc_ctx->devid;
	cq_info = devdrv_calc_cq_info(dev_id,cq_id);
	cq_sub_info = (struct devdrv_cq_sub_info *)cq_info->cq_sub;
	proc_ctx->receive_count += cq_info->receive_count;

	//del from proc_ctx->cq_list
	list_del(&cq_sub_info->list);
	proc_ctx ->cq_num--;
	//add to dev_ctx->cq_available_list
	(void)devdrv_free_cq_id(dev_id,cq_id);
	(void)devdrv_free_cq_mem(dev_id,cq_id);

	devdrv_drv_debug("proc_ctx pid %d cq_id %d total receive report"
				" count = %d proc current left cq num = %d \n",proc_ctx->pid,
				cq_id,proc_ctx->receive_count,proc_ctx ->cq_num);
	return 0;
}

int devdrv_proc_free_cq(struct devdrv_proc_ctx *proc_ctx)
{
	struct devdrv_cq_sub_info *cq_sub = NULL;
	u32 cq_id = 0;

	if (proc_ctx == NULL) {
		devdrv_drv_err("proc_ctx is null\n");
		return -1;
	}

	if (list_empty_careful(&proc_ctx->cq_list)) {
		devdrv_drv_err("cur process %d available cq list empty,"
		"left cq_num = %d !!!\n",proc_ctx->pid,proc_ctx->cq_num);
		return -1;
	}

	cq_sub = list_first_entry(&proc_ctx->cq_list,
					struct devdrv_cq_sub_info, list);
	cq_id = cq_sub->index;

	return devdrv_proc_free_single_cq(proc_ctx,cq_id);
}

//get phase of report from cq head or cq tail depending on report_pos
static u32 __devdrv_get_report_phase_from_cq_info(
									struct devdrv_ts_cq_info *cq_info,
									cq_report_pos_t report_pos)
{
	struct devdrv_cq_sub_info *cq_sub_info = NULL;
	struct devdrv_report *report = NULL;
	u32 phase = 0;

	cq_sub_info = (struct devdrv_cq_sub_info *)cq_info->cq_sub;

	if (report_pos == RREPORT_FROM_CQ_TAIL) {
		report = (struct devdrv_report*)(uintptr_t)(cq_sub_info->virt_addr +
		(long)(unsigned)(cq_info->slot_size * cq_info->tail));
	}

	if (report_pos == RREPORT_FROM_CQ_HEAD) {
		report = (struct devdrv_report*)(uintptr_t)(cq_sub_info->virt_addr +
		(long)(unsigned)(cq_info->slot_size * cq_info->head));
	}

	if (report != NULL) {
		phase = devdrv_get_phase_from_report(report);
	}

	return phase;
}


static u32 __devdrv_get_report_phase(struct devdrv_proc_ctx *proc_ctx,
									cq_report_pos_t report_pos)
{
	struct devdrv_cq_sub_info *cq_sub_info = NULL;
	struct devdrv_ts_cq_info *cq_info = NULL;
	u32 phase = 0;

	if (list_empty_careful(&proc_ctx->cq_list)) {
		devdrv_drv_err("proc ctx cq list is empty");
		return phase;
	}

	cq_sub_info = list_first_entry(&proc_ctx->cq_list,
					struct devdrv_cq_sub_info, list);
	cq_info = devdrv_calc_cq_info(proc_ctx->devid,cq_sub_info->index);
	if(cq_info == NULL){
		devdrv_drv_debug("proc ctx first cq_info is null");
		return phase;
	}

	phase = __devdrv_get_report_phase_from_cq_info(cq_info,report_pos);

	return phase;
}

u32 devdrv_proc_get_cq_tail_report_phase(struct devdrv_proc_ctx *proc_ctx)
{
	return __devdrv_get_report_phase(proc_ctx,RREPORT_FROM_CQ_TAIL);
}

u32 devdrv_proc_get_cq_head_report_phase(struct devdrv_proc_ctx *proc_ctx)
{
	return __devdrv_get_report_phase(proc_ctx,RREPORT_FROM_CQ_HEAD);
}

bool devdrv_proc_is_valid_report_received(struct devdrv_proc_ctx *proc_ctx)
{
	struct devdrv_cq_sub_info *cq_sub_info = NULL;
	struct devdrv_ts_cq_info *cq_info = NULL;
	int retport_phase = 0;

	if (list_empty_careful(&proc_ctx->cq_list)) {
		devdrv_drv_err("cq_info is null");
		return false;
	}

	cq_sub_info = list_first_entry(&proc_ctx->cq_list,
					struct devdrv_cq_sub_info, list);
	cq_info = devdrv_calc_cq_info(proc_ctx->devid,cq_sub_info->index);
	if (cq_info == NULL) {
		devdrv_drv_debug("proc ctx first cq_info is null");
		return false;
	}

	retport_phase = __devdrv_get_report_phase_from_cq_info(
												cq_info,
												RREPORT_FROM_CQ_TAIL);

	devdrv_drv_debug("cqid = %d cqtail = %d cq_info phase = %d "
						"report phase = %d\n",cq_info->index,
						cq_info->tail,cq_info->phase,retport_phase);

	if(cq_info->phase == retport_phase){
		return true;
	}

	return false;
}

int devdrv_proc_report_wait(struct devdrv_proc_ctx *proc_ctx, int timeout)
{
	unsigned long tmp;
	int ret;

	if (timeout == -1 ) {
		wait_event(proc_ctx->report_wait,
		proc_ctx->cq_tail_updated == CQ_HEAD_UPDATED_FLAG);
		ret = 1;
		return ret;
	}

	if( timeout > 0) {
		tmp = msecs_to_jiffies(timeout);
		if (wait_event_interruptible_timeout(proc_ctx->report_wait,
			proc_ctx->cq_tail_updated == CQ_HEAD_UPDATED_FLAG, tmp)) {
			ret = 1; /* condition became true */
		} else {
			ret = 0; /* timeout with condition false */
		}
		return ret;
	}

	//else
	ret = -EINVAL;
	return ret;
}

static void devdrv_find_cq_index(unsigned long data)
{
	struct devdrv_cq_sub_info *cq_sub_info = NULL;
	struct devdrv_ts_cq_info *cq_info = NULL;
	struct devdrv_cq_report_int_ctx *int_context = NULL;
	struct devdrv_proc_ctx *proc_ctx = NULL;
	unsigned long flags;
	int cq_index;
	u32 phase;
	int end;
	int i = 0;

	g_find_cq_index_called_num++;//user for debug,compare with ts side
	int_context = (struct devdrv_cq_report_int_ctx *)(uintptr_t)data;
	proc_ctx = int_context->proc_ctx;
	if (proc_ctx == NULL) {
		devdrv_drv_err("cq report int_context`s proc ctx is null ");
		return;
	}

	end = int_context->first_cq_index + DEVDRV_CQ_PER_IRQ;
	for (i = int_context->first_cq_index; i < end; i++) {
		cq_index = i;
		cq_info = devdrv_calc_cq_info(proc_ctx->devid, cq_index);
		if (cq_index != cq_info->index) {
			devdrv_drv_err("cq_index != cq_info->index, cq_index: %d, "
										"cq_info->index: %d.\n",
										cq_index,
										cq_info->index);
			continue;
		}

		cq_sub_info = (struct devdrv_cq_sub_info*)cq_info->cq_sub;
		if (cq_sub_info == NULL)
		{
			continue;
		}

		spin_lock_irqsave(&cq_sub_info->spinlock, flags);
		phase = devdrv_proc_get_cq_tail_report_phase(proc_ctx);
		spin_unlock_irqrestore(&cq_sub_info->spinlock, flags);

		/* for checking a thread is waiting for wake up */
		if (proc_ctx->cq_tail_updated != CQ_HEAD_INITIAL_UPDATE_FLAG) {/* condition is true, continue */
			devdrv_drv_debug("receive report irq, cq id: %u, "
					 "no runtime thread is waiting, not judge.\n",
					 cq_info->index);
			continue;
		}

		if (phase == cq_info->phase) {
			devdrv_drv_debug("receive report irq, cq id: %u, cq_head = %d "
				"cq_tail = %d report is valid, wake up runtime thread.\n",
				cq_info->index,cq_info->head,cq_info->tail);
			proc_ctx->cq_tail_updated = CQ_HEAD_UPDATED_FLAG;
			wake_up(&proc_ctx->report_wait);
		}
	}
}

static irqreturn_t devdrv_irq_handler(int irq, void *data)
{
	struct devdrv_cq_report_int_ctx *int_context = NULL;
	unsigned long flags;

	local_irq_save(flags);

	g_recv_cq_int_num++;//user for debug,compare with ts side
	int_context = (struct devdrv_cq_report_int_ctx *)data;
	tasklet_schedule(&int_context->find_cq_task);

	local_irq_restore(flags);

	return IRQ_HANDLED;
}

//just use for debug when exception happened
void show_cq_report_int_info(void)
{
	devdrv_drv_err("g_recv_cq_int_num = %llu ,g_find_cq_index_called_num: %llu\n",
						g_recv_cq_int_num,g_find_cq_index_called_num);
}

static int __devdrv_request_cq_report_irq_bh(
		struct devdrv_cq_report_int_ctx *cq_int_ctx)
{
	int ret;
	unsigned int cq_irq = 0;
	struct devdrv_platform_info* plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("devdrv_plat_get_info.\n");
		return -EINVAL;
	}

	if (cq_int_ctx == NULL) {
		devdrv_drv_err("cq report int_context is null ");
		return -1;
	}

	cq_int_ctx->first_cq_index = 0;
	tasklet_init(&cq_int_ctx->find_cq_task,
				devdrv_find_cq_index,
				(uintptr_t)cq_int_ctx);

	cq_irq = DEVDRV_PLAT_GET_CQ_UPDATE_IRQ(plat_info);
	ret = request_irq(cq_irq, devdrv_irq_handler,
			IRQF_TRIGGER_NONE, "npu_cq_report_handler", cq_int_ctx);
	if (ret != 0) {
		devdrv_drv_err("request cq report irq failed\n");
		goto request_failed;
	}

	devdrv_drv_debug("request cq report irq %d success\n",cq_irq);
	return ret;

request_failed:
	free_irq(DEVDRV_PLAT_GET_CQ_UPDATE_IRQ(plat_info), cq_int_ctx);
	tasklet_kill(&cq_int_ctx->find_cq_task);
	return ret;
}

int devdrv_request_cq_report_irq_bh(void)
{
	return __devdrv_request_cq_report_irq_bh(&g_cq_int_ctx);
}

static int __devdrv_free_cq_report_irq_bh(
		struct devdrv_cq_report_int_ctx *cq_int_ctx)
{
	unsigned int cq_irq = 0;
	struct devdrv_platform_info* plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("devdrv_plat_get_info.\n");
		return -EINVAL;
	}

	if (cq_int_ctx == NULL) {
		devdrv_drv_err("cq report int_context is null ");
		return -EINVAL;
	}

	cq_irq = DEVDRV_PLAT_GET_CQ_UPDATE_IRQ(plat_info);
	free_irq(cq_irq, cq_int_ctx);
	tasklet_kill(&cq_int_ctx->find_cq_task);
	return 0;
}

int devdrv_free_cq_report_irq_bh(void)
{
	return __devdrv_free_cq_report_irq_bh(&g_cq_int_ctx);
}


void devdrv_bind_proc_ctx_with_cq_int_ctx(struct devdrv_proc_ctx *proc_ctx)
{
	g_cq_int_ctx.proc_ctx = proc_ctx;
}

void devdrv_unbind_proc_ctx_with_cq_int_ctx(struct devdrv_proc_ctx *proc_ctx)
{
	if (proc_ctx != NULL) {
		g_cq_int_ctx.proc_ctx = NULL;
	}
}

