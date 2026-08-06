
#include "inc/hwsdp_ioctl_teec.h"
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <securec.h>
#include "inc/hwsdp_utils.h"

EXPORT_SYMBOL(saved_command_line);

static struct workqueue_struct *g_hwsdp_workqueue = NULL;
static hwsdp_teec_work_t g_teec_invoke_work;

static u32 g_uid = 0u;
static const u8 *g_hwsdp_package_name = "/dev/hwsdp_ca";
static TEEC_Context g_hwsdp_context = {0};
static TEEC_Session g_hwsdp_session = {0};

static u8 *g_in_data_buff = NULL;
static u32 g_in_data_buff_size = 0u;
static u8 *g_out_data_buff = NULL;
static u32 g_out_data_buff_size = 0u;

static DEFINE_MUTEX(g_hwsdp_tee_mutex);
static bool g_is_tee_inited = false;

#define HWSDP_SAFE_ALLOC_MEM_SIZE 4096

typedef struct {
	u8 *in_data;
	u8 *out_data_buff;
	u32 in_data_len;
	u32 out_buff_sz;
	u32 out_data_len;
	u32 tee_cmd;
} hwsdp_teec_params;

s32 init_hwsdp_work_queue(void)
{
	if (g_hwsdp_workqueue == NULL)
		g_hwsdp_workqueue = create_singlethread_workqueue("HWSDP_TEE_SESS_WORK_QUEUE");

	return (g_hwsdp_workqueue != NULL) ? HWSDP_IOCTL_SUCCESS : HWSDP_IOCTL_ERROR;
}

void destory_hwsdp_work_resource(void)
{
	if (g_hwsdp_workqueue != NULL)
		destroy_workqueue(g_hwsdp_workqueue);

	if (g_in_data_buff != NULL) {
		(void)memset_s(g_in_data_buff, g_in_data_buff_size, 0, g_in_data_buff_size);
		kfree(g_in_data_buff);
		g_in_data_buff = NULL;
		g_in_data_buff_size = 0;
	}

	if (g_out_data_buff != NULL) {
		(void)memset_s(g_out_data_buff, g_out_data_buff_size, 0, g_out_data_buff_size);
		kfree(g_out_data_buff);
		g_out_data_buff = NULL;
		g_out_data_buff_size = 0;
	}

	return;
}

static s32 get_base_value_from_user(u8 *in_data, s32 in_len, TEEC_Operation *operation)
{
	s32 ret_a;
	s32 ret_b;
	u8 *ptr = NULL;

	ptr = in_data;
	if ((in_len == sizeof(u8)) || (in_len == sizeof(u32))) {
		ret_a = copy_from_user(&(operation->params[0].value.a), ptr, in_len);
		if (ret_a != 0) {
			hwsdp_log_err("get_base_value_from_user, failure in copying user invoke data!");
			return HWSDP_IOCTL_ERR_INTERNAL;
		}
		if (in_len == sizeof(u8))
			operation->params[0].value.a &= 0xffu; /* 0xff one byte valid */

		return HWSDP_IOCTL_SUCCESS;
	}

	/* data length is 8 bytes */
	ret_a = copy_from_user(&(operation->params[0].value.a), ptr, sizeof(u32));
	ptr += sizeof(u32);
	ret_b = copy_from_user(&(operation->params[0].value.b), ptr, sizeof(u32));
	if ((ret_a != 0) || (ret_b != 0)) {
		hwsdp_log_err("get_base_value_from_user, "
			"failure in copying user invoke data, ret a: %d, ret b: %d!", ret_a, ret_b);
		return HWSDP_IOCTL_ERR_INTERNAL;
	}
	return HWSDP_IOCTL_SUCCESS;
}

static void *update_in_data(u8 *in_data, s32 in_len)
{
	s32 ret;

	if ((in_len <= 0) || (in_len > HWSDP_SAFE_ALLOC_MEM_SIZE)) {
		hwsdp_log_err("hwsdp: update_in_data, "
			"in date length error IN - %d, SAFE SIZE - %d", in_len, HWSDP_SAFE_ALLOC_MEM_SIZE);
		return NULL;
	}

	if (in_len > g_in_data_buff_size) {
		if (g_in_data_buff != NULL)
			kfree(g_in_data_buff);

		g_in_data_buff = (u8 *)kmalloc(in_len, GFP_KERNEL);
		if (g_in_data_buff == NULL) {
			hwsdp_log_err("hwsdp: update_in_data, kmalloc failed");
			g_in_data_buff_size = 0u;
			return NULL;
		}
		g_in_data_buff_size = in_len;
	}

	ret = copy_from_user(g_in_data_buff, in_data, in_len);
	if (ret != 0) {
		(void)memset_s(g_in_data_buff, g_in_data_buff_size, 0, g_in_data_buff_size);
		hwsdp_log_err("hwsdp: update_in_data, copy_from_user failed");
		return NULL;
	}
	return g_in_data_buff;
}

static void *update_out_data_buffer(s32 out_buff_size)
{
	if ((out_buff_size <= 0) || (out_buff_size > HWSDP_SAFE_ALLOC_MEM_SIZE)) {
		hwsdp_log_err("hwsdp: update_out_data_buffer, buffer size error %d, safe size %d",
			out_buff_size, HWSDP_SAFE_ALLOC_MEM_SIZE);
		return NULL;
	}

	if (out_buff_size > g_out_data_buff_size) {
		if (g_out_data_buff != NULL)
			kfree(g_out_data_buff);

		g_out_data_buff = (u8 *)kmalloc(out_buff_size, GFP_KERNEL);
		if (g_out_data_buff == NULL) {
			hwsdp_log_err("hwsdp: update_out_data_buffer, kmalloc failed");
			g_out_data_buff_size = 0u;
			return NULL;
		}
		g_out_data_buff_size = out_buff_size;
	}
	(void)memset_s(g_out_data_buff, g_out_data_buff_size, 0, g_out_data_buff_size);

	return g_out_data_buff;
}

static void build_teec_operation(u8 *in_data, s32 in_len, s32 out_buff_len, TEEC_Operation *operation)
{
	u32 op_type0;
	u32 op_type1;
	u32 op_type2;
	s32 ret;

	operation->started = 1;
	if ((in_len == sizeof(u8)) || (in_len == sizeof(u32)) || (in_len == sizeof(u64))) {
		op_type0 = TEEC_VALUE_INPUT;
		ret = get_base_value_from_user(in_data, in_len, operation);
		if (ret != HWSDP_IOCTL_SUCCESS)
			return;
	} else {
		op_type0 = TEEC_MEMREF_TEMP_INOUT;
		operation->params[0].tmpref.buffer = update_in_data(in_data, in_len);
		operation->params[0].tmpref.size = ((operation->params[0].tmpref.buffer != NULL) ? in_len : 0u);
		hwsdp_log_info("hwsdp: build_teec_operation, in buffer %p, size %d",
			operation->params[0].tmpref.buffer, operation->params[0].tmpref.size);
	}
	op_type1 = TEEC_NONE;
	op_type2 = TEEC_NONE;
	if (out_buff_len > 0) {
		op_type1 = TEEC_MEMREF_TEMP_OUTPUT;
		op_type2 = TEEC_VALUE_OUTPUT;
		operation->params[1].tmpref.buffer = update_out_data_buffer(out_buff_len);
		operation->params[1].tmpref.size = (operation->params[1].tmpref.buffer != NULL) ? g_out_data_buff_size : 0u;
		operation->params[2].value.a = 0u; /* 2 - the third parameter, for out data length */
		operation->params[2].value.b = 0u; /* 2 - the third parameter */
		hwsdp_log_info("hwsdp: build_teec_operation, out buffer %p, size %d",
			operation->params[1].tmpref.buffer, operation->params[1].tmpref.size);
	}
	operation->paramTypes = TEEC_PARAM_TYPES(op_type0, op_type1, op_type2, TEEC_NONE);
	return;
}

static void hwsdp_open_session(struct work_struct *work)
{
	u32 err_origin = 0;
	s32 idx;
	TEEC_UUID uuid = HWSDP_TA_UUID;
	TEEC_Operation *op = NULL;
	TEEC_Result res;
	hwsdp_teec_work_t *teec_init_work = NULL;

	teec_init_work = container_of(work, hwsdp_teec_work_t, work);
	teec_init_work->result = HWSDP_IOCTL_SUCCESS;
	res = TEEK_InitializeContext(NULL, &g_hwsdp_context);
	if (res != TEEC_SUCCESS) {
		hwsdp_log_err("TEEK_InitializeContext returns %x", res);
		teec_init_work->result = HWSDP_IOCTL_ERR_INIT_TEEC_CTX;
		goto cleanup;
	}
	op = &(teec_init_work->op);
	(void)memset_s(op, sizeof(TEEC_Operation), 0, sizeof(TEEC_Operation));
	op->paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE,
		TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT);
	/* prepare operation parameters */
	op->started = 1;
	/* params 2 is TEEC_MEMREF_TEMP_INPUT */
	idx = 2;
	op->params[idx].tmpref.buffer = (void *)&g_uid;
	op->params[idx].tmpref.size = sizeof(g_uid);
	/* params 3 is TEEC_MEMREF_TEMP_INPUT */
	idx++;
	op->params[idx].tmpref.buffer = (void *)g_hwsdp_package_name;
	op->params[idx].tmpref.size = strlen(g_hwsdp_package_name) + 1;
	/* Open a session to the TA */
	res = TEEK_OpenSession(&g_hwsdp_context, &g_hwsdp_session,
		&uuid, TEEC_LOGIN_IDENTIFY, NULL, op, &err_origin);
	hwsdp_log_info("TEEK_OpenSession returned %d and err_origin is %d", res, err_origin);
	if (res != TEEC_SUCCESS) {
		teec_init_work->result = HWSDP_IOCTL_ERR_OPEN_TEEC_SESSION;
		TEEK_FinalizeContext(&g_hwsdp_context);
	}

cleanup:
	teec_init_work->resp = 1;
	wake_up(&teec_init_work->notify_waitq);
	return;
}

static s32 work_open_session(void)
{
	s32 ret = 0;

	if (g_hwsdp_workqueue == NULL)
		return HWSDP_IOCTL_ERR_INTERNAL;

	(void)memset_s(&g_teec_invoke_work, sizeof(g_teec_invoke_work), 0, sizeof(g_teec_invoke_work));
	init_waitqueue_head(&(g_teec_invoke_work.notify_waitq));
	INIT_WORK(&(g_teec_invoke_work.work), hwsdp_open_session);
	ret = queue_work(g_hwsdp_workqueue, &(g_teec_invoke_work.work));
	if (ret == 0) {
		hwsdp_log_err("teec_init add work queue failed!");
		return HWSDP_IOCTL_ERR_INTERNAL;
	}
	wait_event(g_teec_invoke_work.notify_waitq, g_teec_invoke_work.resp);
	return g_teec_invoke_work.result;
}

s32 hwsdp_init_tee(void)
{
	s32 ret = HWSDP_IOCTL_SUCCESS;

	if (g_is_tee_inited == true)
		return ret;

	mutex_lock(&g_hwsdp_tee_mutex);
	if (g_is_tee_inited == false) {
		ret = work_open_session();
		if (ret != HWSDP_IOCTL_SUCCESS) {
			hwsdp_log_err("work_open_session failed, err code: %d", ret);
		} else {
			hwsdp_log_info("work_open_session done");
			g_is_tee_inited = true;
		}
	}
	mutex_unlock(&g_hwsdp_tee_mutex);
	return ret;
}

static void hwsdp_invoke_cmd(struct work_struct *work)
{
	TEEC_Result tee_res;
	u32 origin = 0u;
	hwsdp_teec_work_t *teec_invoke_work = NULL;

	teec_invoke_work = container_of(work, hwsdp_teec_work_t, work);
	tee_res = TEEK_InvokeCommand(&g_hwsdp_session, teec_invoke_work->cmd,
									&(teec_invoke_work->op), &origin);
	if (tee_res != TEEC_SUCCESS) {
		hwsdp_log_err("InvokeCommand failed res=0x%x orig=0x%x cmd=%d",
			tee_res, origin, teec_invoke_work->cmd);
		teec_invoke_work->result = HWSDP_IOCTL_ERR_TEEC_INVOKE_CMD;
	} else {
		hwsdp_log_info("TEEK_InvokeCommand called successfully");
		teec_invoke_work->result = HWSDP_IOCTL_SUCCESS;
	}

	teec_invoke_work->resp = 1;
	wake_up(&teec_invoke_work->notify_waitq);
	return;
}

static s32 out_data_to_user_buffer(hwsdp_teec_params *params)
{
	s32 ret = 0;
	s32 min_len = 0;

	if (g_out_data_buff == NULL) {
		hwsdp_log_err("out_data_to_user_buffer, internal buffer invalid!");
		return HWSDP_IOCTL_ERR_INTERNAL;
	}

	min_len = params->out_data_len <= params->out_buff_sz ? params->out_data_len : params->out_buff_sz;
	ret = copy_to_user(params->out_data_buff, g_out_data_buff, min_len);
	if (ret != 0) {
		hwsdp_log_err("work_invoke_cmd, failure in copying out data to user space!");
		return HWSDP_IOCTL_ERR_INTERNAL;
	}
	params->out_data_len = min_len;
	hwsdp_log_info("work_invoke_cmd, out data to user space, length %d!", min_len);
	return HWSDP_IOCTL_SUCCESS;
}

static s32 work_invoke_cmd(hwsdp_ioctl_data_t *kdata)
{
	s32 ret = 0;
	hwsdp_teec_params params;

	if (g_hwsdp_workqueue == NULL) {
		hwsdp_log_err("hwsdp workqueue is uninitializated");
		return HWSDP_IOCTL_ERR_INTERNAL;
	}

	/* data length: cmd + TEEC_Operation */
	if (kdata->data_len != sizeof(hwsdp_teec_params)) {
		hwsdp_log_err("User data length is bad, %lu/%lu",
			kdata->data_len, sizeof(hwsdp_teec_params));
		return HWSDP_IOCTL_ERR_USER_DATA;
	}
	if (kdata->data_ptr == NULL) {
		hwsdp_log_err("work_invoke_cmd params invalid");
		return HWSDP_IOCTL_ERR_ALLOC_MEMORY;
	}
	ret = copy_from_user(&params, kdata->data_ptr, sizeof(hwsdp_teec_params));
	if (ret != 0) {
		hwsdp_log_err("work_invoke_cmd, failure in copying user invoke data!");
		return HWSDP_IOCTL_ERR_INTERNAL;
	}
	(void)memset_s(&g_teec_invoke_work, sizeof(g_teec_invoke_work), 0, sizeof(g_teec_invoke_work));
	g_teec_invoke_work.cmd = params.tee_cmd;
	hwsdp_log_info("work_invoke_cmd, tee params cmd %d", params.tee_cmd);
	build_teec_operation(params.in_data, params.in_data_len, params.out_buff_sz, &(g_teec_invoke_work.op));
	init_waitqueue_head(&(g_teec_invoke_work.notify_waitq));
	INIT_WORK(&(g_teec_invoke_work.work), hwsdp_invoke_cmd);
	ret = queue_work(g_hwsdp_workqueue, &(g_teec_invoke_work.work));
	if (ret == 0) {
		hwsdp_log_err("work_invoke_cmd add work queue failed!");
		return HWSDP_IOCTL_ERR_INTERNAL;
	}
	wait_event(g_teec_invoke_work.notify_waitq, g_teec_invoke_work.resp);
	if ((params.out_data_buff != NULL) && (params.out_buff_sz > 0)) {
		params.out_data_len = g_teec_invoke_work.op.params[2].value.a; /* 2 - the third parameter */
		ret = out_data_to_user_buffer(&params);
		if (ret != HWSDP_IOCTL_SUCCESS) {
			return ret;
		}
	}

	ret = copy_to_user(kdata->data_ptr, &params, sizeof(hwsdp_teec_params));
	if (ret != 0) {
		hwsdp_log_err("work_invoke_cmd, failure in copying result to user space!");
		return HWSDP_IOCTL_ERR_INTERNAL;
	}

	return g_teec_invoke_work.result;
}

s32 hwsdp_teec_ops(hwsdp_ioctl_data_t *kdata)
{
	s32 ret;

	if (g_is_tee_inited == false)
		return HWSDP_IOCTL_ERR_UNINITIALIZED;

	mutex_lock(&g_hwsdp_tee_mutex);
	ret = work_invoke_cmd(kdata);
	if (ret != HWSDP_IOCTL_SUCCESS) {
		hwsdp_log_err("hwsdp_teec_ops failed, err code: %d", ret);
	} else {
		hwsdp_log_info("hwsdp_teec_ops done");
	}
	mutex_unlock(&g_hwsdp_tee_mutex);
	return ret;
}

