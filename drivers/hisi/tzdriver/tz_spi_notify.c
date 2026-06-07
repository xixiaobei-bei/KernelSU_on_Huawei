/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 * Description: Function definition for spi interrupt actions.
 * Author: qiqingchao  q00XXXXXX
 * Create: 2016-06-21
 */
#include "tz_spi_notify.h"
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <asm/cacheflush.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/of_reserved_mem.h>
#include <linux/atomic.h>
#include <linux/interrupt.h>
#include <securec.h>
#include "teek_client_constants.h"
#include "tc_ns_client.h"
#include "tc_ns_log.h"
#include "tc_client_sub_driver.h"
#include "gp_ops.h"
#include "mailbox_mempool.h"
#include "smc.h"

#define MAX_CALLBACK_COUNT 100
#define UUID_SIZE 16
struct teec_timer_property;


enum timer_class_type {
	/* timer event using timer10 */
	TIMER_GENERIC,
	/* timer event using RTC */
	TIMER_RTC
};

struct teec_timer_property {
	unsigned int type;
	unsigned int timer_id;
	unsigned int timer_class;
	unsigned int reserved2;
};

struct notify_context_timer {
	unsigned int dev_file_id;
	unsigned char uuid[UUID_SIZE];
	unsigned int session_id;
	struct teec_timer_property property;
	uint32_t expire_time;
};

#ifdef CONFIG_TEE_SMP
struct notify_context_wakeup {
	pid_t ca_thread_id;
};

struct notify_context_shadow {
	uint64_t target_tcb;
};

struct notify_context_stats {
	uint32_t send_s;
	uint32_t recv_s;
	uint32_t send_w;
	uint32_t recv_w;
	uint32_t missed;
};
#endif

union notify_context {
	struct notify_context_timer timer;
#ifdef CONFIG_TEE_SMP
	struct notify_context_wakeup wakeup;
	struct notify_context_shadow shadow;
	struct notify_context_stats  meta;
#endif
};

struct notify_data_entry {
	uint32_t entry_type : 31;
	uint32_t filled	 : 1;
	union notify_context context;
};

#define NOTIFY_DATA_ENTRY_COUNT \
	((PAGE_SIZE / sizeof(struct notify_data_entry)) - 1)
struct notify_data_struct {
	struct notify_data_entry entry[NOTIFY_DATA_ENTRY_COUNT];
	struct notify_data_entry meta;
};

static struct notify_data_struct *g_notify_data = NULL;
static struct notify_data_entry *g_notify_data_entry_timer = NULL;
static struct notify_data_entry *g_notify_data_entry_rtc = NULL;
#ifdef CONFIG_TEE_SMP
static struct notify_data_entry *g_notify_data_entry_shadow = NULL;
#endif

enum notify_data_type {
	NOTIFY_DATA_ENTRY_UNUSED,
	NOTIFY_DATA_ENTRY_TIMER,
	NOTIFY_DATA_ENTRY_RTC,
#ifdef CONFIG_TEE_SMP
	NOTIFY_DATA_ENTRY_WAKEUP,
	NOTIFY_DATA_ENTRY_SHADOW,
	NOTIFY_DATA_ENTRY_FIQSHD,
	NOTIFY_DATA_ENTRY_SHADOW_EXIT,
#endif
	NOTIFY_DATA_ENTRY_MAX,
};

struct tc_ns_callback {
	unsigned char uuid[UUID_SIZE];
	struct mutex callback_lock;
	void (*callback_func)(void *);
	struct list_head head;
};

struct tc_ns_callback_list {
	unsigned int callback_count;
	struct mutex callback_list_lock;
	struct list_head callback_list;
};

static void tc_notify_fn(struct work_struct *dummy);
static struct tc_ns_callback_list g_ta_callback_func_list;
static DECLARE_WORK(tc_notify_work, tc_notify_fn);
#ifdef CONFIG_TEE_SMP
static struct workqueue_struct *g_tz_spi_wq = NULL;
#endif

static void walk_callback_list(
	struct notify_context_timer *tc_notify_data_timer)
{
	struct tc_ns_callback *callback_func_t = NULL;

	mutex_lock(&g_ta_callback_func_list.callback_list_lock);
	list_for_each_entry(callback_func_t,
		&g_ta_callback_func_list.callback_list, head) {
		if (memcmp(callback_func_t->uuid, tc_notify_data_timer->uuid,
			UUID_SIZE) == 0) {
			if (tc_notify_data_timer->property.timer_class == TIMER_RTC) {
				tlogd("start to call callback func\n");
				callback_func_t->callback_func(
					(void *)(&(tc_notify_data_timer->property)));
				tlogd("end to call callback func\n");
			} else if (tc_notify_data_timer->property.timer_class == TIMER_GENERIC) {
				tlogd("timer60 no callback func\n");
			}
		}
	}
	mutex_unlock(&g_ta_callback_func_list.callback_list_lock);
}

static void tc_notify_timer_fn(struct notify_data_entry *notify_data_entry)
{
	tc_ns_dev_file *temp_dev_file = NULL;
	tc_ns_service *temp_svc = NULL;
	tc_ns_session *temp_ses = NULL;
	int enc_found = 0;
	struct notify_context_timer *tc_notify_data_timer = NULL;

	tc_notify_data_timer = &(notify_data_entry->context.timer);
	notify_data_entry->filled = 0;
	tlogd("notify_data timer type is 0x%x, timer ID is 0x%x\n",
	      tc_notify_data_timer->property.type,
	      tc_notify_data_timer->property.timer_id);
	walk_callback_list(tc_notify_data_timer);
	mutex_lock(&g_tc_ns_dev_list.dev_lock);
	list_for_each_entry(temp_dev_file, &g_tc_ns_dev_list.dev_file_list, head) {
		tlogd("dev file id1 = %d, id2 = %d\n",
		      temp_dev_file->dev_file_id, tc_notify_data_timer->dev_file_id);
		if (temp_dev_file->dev_file_id == tc_notify_data_timer->dev_file_id) {
			mutex_lock(&temp_dev_file->service_lock);
			temp_svc =
				tc_find_service_in_dev(temp_dev_file,
					tc_notify_data_timer->uuid, UUID_LEN);
			get_service_struct(temp_svc);
			mutex_unlock(&temp_dev_file->service_lock);
			if (temp_svc == NULL)
				break;
			mutex_lock(&temp_svc->session_lock);
			temp_ses =
				tc_find_session_withowner(&temp_svc->session_list,
					tc_notify_data_timer->session_id, temp_dev_file);
			get_session_struct(temp_ses);
			mutex_unlock(&temp_svc->session_lock);
			put_service_struct(temp_svc);
			temp_svc = NULL;
			if (temp_ses != NULL) {
				tlogd("send cmd ses id %d\n", temp_ses->session_id);
				enc_found = 1;
				break;
			}
			break;
		}
	}
	mutex_unlock(&g_tc_ns_dev_list.dev_lock);
	if (tc_notify_data_timer->property.timer_class == TIMER_GENERIC) {
		tlogd("timer60 wake up event\n");
		if (enc_found && temp_ses != NULL) {
			temp_ses->wait_data.send_wait_flag = 1;
			wake_up(&temp_ses->wait_data.send_cmd_wq);
			put_session_struct(temp_ses);
			temp_ses = NULL;
		}
	} else {
		tlogd("RTC do not need to wakeup\n");
	}
}

#ifdef CONFIG_TEE_SMP
static noinline int get_notify_data_entry(struct notify_data_entry *copy)
{
	uint32_t i;
	int filled;
	int ret = -1;

	if (copy == NULL) {
		tloge("Bad parameters! ");
		return ret;
	}
	/* TIMER and RTC use fix entry, skip them. */
	for (i = NOTIFY_DATA_ENTRY_WAKEUP - 1; i < NOTIFY_DATA_ENTRY_COUNT; i++) {
		struct notify_data_entry *e = NULL;
		e = &g_notify_data->entry[i];
		filled = e->filled;
		smp_mb();
		if (!filled)
			continue;
		switch (e->entry_type) {
		case NOTIFY_DATA_ENTRY_SHADOW: // fall through
		case NOTIFY_DATA_ENTRY_SHADOW_EXIT: // fall through
		case NOTIFY_DATA_ENTRY_FIQSHD: // fall through
			g_notify_data->meta.context.meta.recv_s++;
			break;
		case NOTIFY_DATA_ENTRY_WAKEUP:
			g_notify_data->meta.context.meta.recv_w++;
			break;
		default:
			tloge("invalid notify type=%d\n", e->entry_type);
			goto exit;
		}
		if (memcpy_s(copy, sizeof(*copy), e, sizeof(*e)) != EOK) {
			tloge("memcpy entry failed\n");
			break;
		}
		smp_mb();
		e->filled = 0;
		ret = 0;
		break;
	}
exit:
	return ret;
}

static void tc_notify_wakeup_fn(struct notify_data_entry *entry)
{
	struct notify_context_wakeup *tc_notify_wakeup = NULL;

	tc_notify_wakeup = &(entry->context.wakeup);
	smc_wakeup_ca(tc_notify_wakeup->ca_thread_id);
	tlogd("notify_data_entry_wakeup ca: %d\n", tc_notify_wakeup->ca_thread_id);
}

static void tc_notify_shadow_fn(struct notify_data_entry *entry)
{
	struct notify_context_shadow *tc_notify_shadow = NULL;

	tc_notify_shadow = &(entry->context.shadow);
	smc_queue_shadow_worker(tc_notify_shadow->target_tcb);
}

static void tc_notify_fiqshd_fn(struct notify_data_entry *entry)
{
	struct notify_context_shadow *tc_notify_shadow = NULL;

	if (entry == NULL) {
		/* for NOTIFY_DATA_ENTRY_FIQSHD missed */
		fiq_shadow_work_func(0);
		return;
	}
	tc_notify_shadow = &(entry->context.shadow);
	fiq_shadow_work_func(tc_notify_shadow->target_tcb);
}

static void tc_notify_shadowexit_fn(struct notify_data_entry *entry)
{
	struct notify_context_wakeup *tc_notify_wakeup = NULL;

	tc_notify_wakeup = &(entry->context.wakeup);
	if (smc_shadow_exit(tc_notify_wakeup->ca_thread_id) != 0)
		tloge("shadow ca exit failed: %d\n",
			(int)tc_notify_wakeup->ca_thread_id);
}
#define MISSED_COUNT 4
static void spi_broadcast_notifications(void)
{
	uint32_t missed;

	smp_mb();
	missed = __xchg(0, &g_notify_data->meta.context.meta.missed, MISSED_COUNT);
	if (!missed)
		return;
	if (missed & (1U << NOTIFY_DATA_ENTRY_WAKEUP)) {
		smc_wakeup_broadcast();
		missed &= ~(1U << NOTIFY_DATA_ENTRY_WAKEUP);
	}
	if (missed & (1U << NOTIFY_DATA_ENTRY_FIQSHD)) {
		tc_notify_fiqshd_fn(NULL);
		missed &= ~(1U << NOTIFY_DATA_ENTRY_FIQSHD);
	}
	if (missed)
		tloge("missed spi notification mask %x\n", missed);
}

static void tc_notify_other_fun(void)
{
	struct notify_data_entry copy = {0};

	while (get_notify_data_entry(&copy) == 0) {
		switch (copy.entry_type) {
		case NOTIFY_DATA_ENTRY_WAKEUP:
			tc_notify_wakeup_fn(&copy);
			break;
		case NOTIFY_DATA_ENTRY_SHADOW:
			tc_notify_shadow_fn(&copy);
			break;
		case NOTIFY_DATA_ENTRY_FIQSHD:
			tc_notify_fiqshd_fn(&copy);
			break;
		case NOTIFY_DATA_ENTRY_SHADOW_EXIT:
			tc_notify_shadowexit_fn(&copy);
			break;
		default:
			tloge("invalid entry type = %d\n", copy.entry_type);
		}
		if (memset_s(&copy, sizeof(copy), 0, sizeof(copy)))
			tloge("memset copy failed\n");
	}
	spi_broadcast_notifications();
}
#else
static void tc_notify_other_fun(void) {}
#endif
static void tc_notify_fn(struct work_struct *dummy)
{
	if (g_notify_data_entry_timer->filled)
		tc_notify_timer_fn(g_notify_data_entry_timer);
	if (g_notify_data_entry_rtc->filled)
		tc_notify_timer_fn(g_notify_data_entry_rtc);
	tc_notify_other_fun();
}

static irqreturn_t tc_secure_notify(int irq, void *dev_id)
{
#ifdef CONFIG_TEE_SMP
#define N_WORK  8
	int i;
	static struct work_struct tc_notify_works[N_WORK];
	static int init;

	if (!init) {
		for (i = 0; i < N_WORK; i++)
			INIT_WORK(&tc_notify_works[i], tc_notify_fn);
		init = 1;
	}
	for (i = 0; i < N_WORK; i++) {
		if (queue_work(g_tz_spi_wq, &tc_notify_works[i]))
			break;
	}
#undef N_WORK
#else
	schedule_work(&tc_notify_work);
	isb();
	wmb();
	tc_smc_wakeup();
#endif
	return IRQ_HANDLED;
}

/*lint -esym(429,*)*/
int tc_ns_register_service_call_back_func(const char *uuid, void *func,
	const void *private_data)
{
	struct tc_ns_callback *callback_func = NULL;
	struct tc_ns_callback *new_callback = NULL;
	int ret = 0;
	errno_t sret;
	bool check_stat = (uuid == NULL || func == NULL);

	if (check_stat)
		return -EINVAL;

	(void)private_data;
	mutex_lock(&g_ta_callback_func_list.callback_list_lock);
	if (g_ta_callback_func_list.callback_count > MAX_CALLBACK_COUNT) {
		mutex_unlock(&g_ta_callback_func_list.callback_list_lock);
		tloge("callback_count is out\n");
		return -ENOMEM;
	}
	list_for_each_entry(callback_func,
		&g_ta_callback_func_list.callback_list, head) {
		if (memcmp(callback_func->uuid, uuid, UUID_SIZE) == 0) {
			callback_func->callback_func = (void (*)(void *))func;
			tlogd("succeed to find uuid ta_callback_func_list\n");
			goto find_callback;
		}
	}
	/* create a new callback struct if we couldn't find it in list */
	new_callback = kzalloc(sizeof(*new_callback), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)new_callback)) {
		tloge("kzalloc failed\n");
		ret = -ENOMEM;
		goto find_callback;
	}
	sret = memcpy_s(new_callback->uuid, UUID_SIZE, uuid, UUID_SIZE);
	if (sret != EOK) {
		kfree(new_callback);
		new_callback = NULL;
		ret = -ENOMEM;
		goto find_callback;
	}
	g_ta_callback_func_list.callback_count++;
	tlogd("ta_callback_func_list.callback_count is %d\n",
	      g_ta_callback_func_list.callback_count);
	INIT_LIST_HEAD(&new_callback->head);
	new_callback->callback_func = (void (*)(void *))func;
	mutex_init(&new_callback->callback_lock);
	list_add_tail(&new_callback->head, &g_ta_callback_func_list.callback_list);
find_callback:
	mutex_unlock(&g_ta_callback_func_list.callback_list_lock);
	return ret;
}
/*lint +esym(429,*)*/

int TC_NS_RegisterServiceCallbackFunc(char *uuid, void *func,
	void *private_data)
{
	const char *uuid_in = uuid;
	return tc_ns_register_service_call_back_func(uuid_in, func, private_data);
}

EXPORT_SYMBOL(TC_NS_RegisterServiceCallbackFunc);


static int tc_ns_register_notify_data_memery(void)
{
	tc_ns_smc_cmd smc_cmd = { {0}, 0 };
	int ret;
	struct mb_cmd_pack *mb_pack = NULL;

	mb_pack = mailbox_alloc_cmd_pack();
	if (mb_pack == NULL)
		return TEEC_ERROR_GENERIC;
	mb_pack->operation.paramtypes =
		TEE_PARAM_TYPE_VALUE_INPUT | TEE_PARAM_TYPE_VALUE_INPUT << TEE_PARAM_NUM;
	mb_pack->operation.params[TEE_PARAM_ONE].value.a = virt_to_phys(g_notify_data);
	mb_pack->operation.params[TEE_PARAM_ONE].value.b = virt_to_phys(g_notify_data) >> ADDR_TRANS_NUM;
	mb_pack->operation.params[TEE_PARAM_TWO].value.a = SZ_4K;
	smc_cmd.global_cmd = true;
	smc_cmd.cmd_id = GLOBAL_CMD_ID_REGISTER_NOTIFY_MEMORY;
	smc_cmd.operation_phys = virt_to_phys(&mb_pack->operation);
	smc_cmd.operation_h_phys = virt_to_phys(&mb_pack->operation) >> ADDR_TRANS_NUM;
	tlogd("cmd. context_phys:%x\n", smc_cmd.context_id);
	ret = tc_ns_smc(&smc_cmd);
	mailbox_free(mb_pack);
	mb_pack = NULL;
	return ret;
}

static int tc_ns_unregister_notify_data_memory(void)
{
	tc_ns_smc_cmd smc_cmd = { {0}, 0 };
	int ret;
	struct mb_cmd_pack *mb_pack = NULL;

	mb_pack = mailbox_alloc_cmd_pack();
	if (mb_pack == NULL)
		return TEEC_ERROR_GENERIC;
	mb_pack->operation.paramtypes =
		TEE_PARAM_TYPE_VALUE_INPUT | TEE_PARAM_TYPE_VALUE_INPUT << TEE_PARAM_NUM;
	mb_pack->operation.params[TEE_PARAM_ONE].value.a = virt_to_phys(g_notify_data);
	mb_pack->operation.params[TEE_PARAM_ONE].value.b = virt_to_phys(g_notify_data) >> ADDR_TRANS_NUM;
	mb_pack->operation.params[TEE_PARAM_TWO].value.a = SZ_4K;
	smc_cmd.global_cmd = true;
	smc_cmd.cmd_id = GLOBAL_CMD_ID_UNREGISTER_NOTIFY_MEMORY;
	smc_cmd.operation_phys = virt_to_phys(&mb_pack->operation);
	smc_cmd.operation_h_phys = virt_to_phys(&mb_pack->operation) >> ADDR_TRANS_NUM;
	tlogd("cmd. context_phys:%x\n", smc_cmd.context_id);
	ret = tc_ns_smc(&smc_cmd);
	mailbox_free(mb_pack);
	mb_pack = NULL;
	return ret;
}

int tz_spi_init(struct device *class_dev, struct device_node *np)
{
	int ret = 0;
	unsigned int irq = 0;

#ifdef CONFIG_TEE_SMP
	g_tz_spi_wq = alloc_ordered_workqueue("g_tz_spi_wq", WQ_HIGHPRI);
	if (g_tz_spi_wq == NULL) {
		tloge("it failed to create workqueue g_tz_spi_wq\n");
		return -ENOMEM;
	}
#endif
	/* Map IRQ 0 from the OF interrupts list */
	irq = irq_of_parse_and_map(np, 0);
	ret = devm_request_irq(class_dev, irq, tc_secure_notify, IRQF_NO_SUSPEND,
		TC_NS_CLIENT_DEV, NULL);
	if (ret < 0) {
		tloge("device irq %u request failed %u", irq, ret);
		goto clean;
	}
	ret = memset_s(&g_ta_callback_func_list, sizeof(g_ta_callback_func_list),
		0, sizeof(g_ta_callback_func_list));
	if (ret != EOK) {
		ret = -EFAULT;
		goto clean;
	}
	g_ta_callback_func_list.callback_count = 0;
	INIT_LIST_HEAD(&g_ta_callback_func_list.callback_list);
	mutex_init(&g_ta_callback_func_list.callback_list_lock);
	if (g_notify_data == NULL) {
		g_notify_data = (struct notify_data_struct *)(uintptr_t)__get_free_page(
			GFP_KERNEL | __GFP_ZERO);
		if (g_notify_data == NULL) {
			tloge("__get_free_page failed for notification data\n");
			ret = -ENOMEM;
			goto clean;
		}
		ret = tc_ns_register_notify_data_memery();
		if (ret != TEEC_SUCCESS) {
			tloge("Shared memory failed ret is 0x%x\n", ret);
			ret = -EFAULT;
			free_page((unsigned long)(uintptr_t)g_notify_data);
			g_notify_data = NULL;
			goto clean;
		}
		g_notify_data_entry_timer =
			&g_notify_data->entry[NOTIFY_DATA_ENTRY_TIMER - 1];
		g_notify_data_entry_rtc =
			&g_notify_data->entry[NOTIFY_DATA_ENTRY_RTC - 1];
#ifdef CONFIG_TEE_SMP
		g_notify_data_entry_shadow =
			&g_notify_data->entry[NOTIFY_DATA_ENTRY_SHADOW - 1];
		tlogi("test target is: %llx\n",
		      g_notify_data_entry_shadow->context.shadow.target_tcb);
#endif
	}
	return 0;
clean:
	tz_spi_exit();
	return ret;
}

void tz_spi_exit(void)
{
	if (g_notify_data != NULL) {
		tc_ns_unregister_notify_data_memory();
		free_page((unsigned long)(uintptr_t)g_notify_data);
		g_notify_data = NULL;
	}
#ifdef CONFIG_TEE_SMP
	if (g_tz_spi_wq != NULL) {
		flush_workqueue(g_tz_spi_wq);
		destroy_workqueue(g_tz_spi_wq);
		g_tz_spi_wq = NULL;
	}
#endif
}
