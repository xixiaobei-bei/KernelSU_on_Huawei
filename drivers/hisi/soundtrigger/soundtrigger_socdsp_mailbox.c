#include <linux/proc_fs.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/kthread.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/errno.h>
#include "../hi64xx_dsp/hi64xx_algo_interface.h"
#include "soundtrigger_socdsp_mailbox.h"
#include "hifi_lpp.h"
#include "soundtrigger_socdsp_pcm.h"
#include "soundtrigger_event.h"

#ifdef CONFIG_HIFI_MAILBOX
#include "drv_mailbox_cfg.h"
#endif

struct completion start_msg_complete;
struct completion stop_msg_complete;
struct completion parameter_set_msg_complete;
struct completion parameter_get_msg_complete;
struct completion lp_start_msg_complete;
struct completion lp_stop_msg_complete;
struct completion lp_parameter_set_msg_complete;
struct completion lp_parameter_get_msg_complete;
struct wakeup_source soundtrigger_rcv_wake_lock;

#define SOCDSP_WAKEUP_MSG_TIMEOUT (2 * HZ)
#define NORMAL_BUFFER_LEN (640) //16K*1ch*2byte*20ms

static int g_socdsp_handle = 0;
static int g_socdsp_lp_handle = 0;
static char *g_model_buf = NULL;
static bool wakeup_is_start = false;
static bool wakeup_lp_is_start = false;

static void soundtrigger_hotword_detected_work(struct work_struct *wk)
{
	hw_soundtrigger_event_uevent(0);
}
static DECLARE_WORK(hotword_detected_queue, soundtrigger_hotword_detected_work);

int soundtrigger_mailbox_send_data(const void *pmsg_body, unsigned int msg_len, unsigned int msg_priority)
{
	unsigned int ret = 0;

	ret = DRV_MAILBOX_SENDMAIL(MAILBOX_MAILCODE_ACPU_TO_HIFI_WAKEUP, pmsg_body, msg_len);
	if (MAILBOX_OK != ret) {
		pr_err("soundtrigger channel send mail failed, ret=%d\n", ret);
	}

	return (int)ret;
}

static void msg_complete_proc(struct soundtrigger_rcv_msg *rcv_msg, struct completion *msg_complete)
{
	if (msg_complete)
		complete(msg_complete);
}

static void wakeup_get_ack_proc(struct soundtrigger_rcv_msg *rcv_msg, struct completion *msg_complete)
{
	g_socdsp_handle = rcv_msg->socdsp_handle;
	if (msg_complete)
		complete(msg_complete);
}

static void wakeup_hotword_ack_proc(struct soundtrigger_rcv_msg *rcv_msg, struct completion *msg_complete)
{
	soundtrigger_socdsp_pcm_flag_init(NORMAL_WAKEUP_MODE);
	soundtrigger_socdsp_pcm_fastbuffer_filled(NORMAL_WAKEUP_MODE, rcv_msg->fast_len);
	schedule_work(&hotword_detected_queue);
}

static void wakeup_elapsed_ack_proc(struct soundtrigger_rcv_msg *rcv_msg, struct completion *msg_complete)
{
	soundtrigger_socdsp_pcm_elapsed(NORMAL_WAKEUP_MODE, rcv_msg->elapsed.start, rcv_msg->elapsed.len);
}

static void wakeup_lp_get_ack_proc(struct soundtrigger_rcv_msg *rcv_msg, struct completion *msg_complete)
{
	g_socdsp_lp_handle = rcv_msg->socdsp_handle;
	if (msg_complete)
		complete(msg_complete);
}

static void wakeup_lp_hotword_ack_proc(struct soundtrigger_rcv_msg *rcv_msg, struct completion *msg_complete)
{
	soundtrigger_socdsp_pcm_flag_init(LP_WAKEUP_MODE);
	soundtrigger_socdsp_pcm_fastbuffer_filled(LP_WAKEUP_MODE, rcv_msg->fast_len);
	schedule_work(&hotword_detected_queue);
}

static void wakeup_lp_elapsed_ack_proc(struct soundtrigger_rcv_msg *rcv_msg, struct completion *msg_complete)
{
	soundtrigger_socdsp_pcm_elapsed(LP_WAKEUP_MODE, rcv_msg->elapsed.start, rcv_msg->elapsed.len);
}

struct msg_node soundtrigger_msg_table[] = {
	{&start_msg_complete, WAKEUP_CHN_MSG_START, WAKEUP_CHN_MSG_START_ACK, msg_complete_proc, "wakeup start"},
	{&stop_msg_complete, WAKEUP_CHN_MSG_STOP, WAKEUP_CHN_MSG_STOP_ACK, msg_complete_proc, "wakeup stop"},
	{&parameter_set_msg_complete, WAKEUP_CHN_MSG_PARAMETER_SET, WAKEUP_CHN_MSG_PARAMETER_SET_ACK, msg_complete_proc, "wakeup set para"},
	{&parameter_get_msg_complete, WAKEUP_CHN_MSG_PARAMETER_GET, WAKEUP_CHN_MSG_PARAMETER_GET_ACK, wakeup_get_ack_proc, "wakeup get para"},
	{NULL, 0, WAKEUP_CHN_MSG_HOTWORD_DETECT_RCV, wakeup_hotword_ack_proc, "wakeup hot word"},
	{NULL, 0, WAKEUP_CHN_MSG_ELAPSED_RCV, wakeup_elapsed_ack_proc, "wakeup elapsed rcv"},

	{&lp_start_msg_complete, LP_WAKEUP_CHN_MSG_START, LP_WAKEUP_CHN_MSG_START_ACK, msg_complete_proc, "lp wakeup start"},
	{&lp_stop_msg_complete, LP_WAKEUP_CHN_MSG_STOP, LP_WAKEUP_CHN_MSG_STOP_ACK, msg_complete_proc, "lp wakeup stop"},
	{&lp_parameter_set_msg_complete, LP_WAKEUP_CHN_MSG_PARAMETER_SET, LP_WAKEUP_CHN_MSG_PARAMETER_SET_ACK, msg_complete_proc, "lp wakeup set para"},
	{&lp_parameter_get_msg_complete, LP_WAKEUP_CHN_MSG_PARAMETER_GET, LP_WAKEUP_CHN_MSG_PARAMETER_GET_ACK, wakeup_lp_get_ack_proc, "lp wakeup get para"},
	{NULL, 0, LP_WAKEUP_CHN_MSG_HOTWORD_DETECT_RCV, wakeup_lp_hotword_ack_proc, "lp wakeup hot word"},
	{NULL, 0, LP_WAKEUP_CHN_MSG_ELAPSED_RCV, wakeup_lp_elapsed_ack_proc, "lp wakeup elapsed rcv"}
};

static struct msg_node *select_msg_node(unsigned short msg_type)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(soundtrigger_msg_table); i++) {
		if(msg_type == soundtrigger_msg_table[i].msg_type ||
			msg_type == soundtrigger_msg_table[i].recv_msg_type)
			return &soundtrigger_msg_table[i];
	}

	pr_err("soundtrigger msg type error, msg = 0x%x\n", msg_type);
	return NULL;
}

static void soundtrigger_mailbox_recv_proc(const void *usr_para,
	struct mb_queue *mail_handle, unsigned int mail_len)
{
	struct soundtrigger_rcv_msg rcv_msg;
	unsigned int ret = MAILBOX_OK;
	unsigned int mail_size = mail_len;
	struct msg_node *msg_info = NULL;

	memset(&rcv_msg, 0, sizeof(rcv_msg));/* unsafe_function_ignore: memset */

	ret = DRV_MAILBOX_READMAILDATA(mail_handle, (unsigned char*)&rcv_msg, &mail_size);
	if ((ret != MAILBOX_OK) ||
		(mail_size == 0) ||
		(mail_size > sizeof(rcv_msg))) {
		pr_err("mailbox read error, read result:%u, read mail size:%u\n",
			ret, mail_size);
		return;
	}

	__pm_wakeup_event(&soundtrigger_rcv_wake_lock, 1000);
	msg_info = select_msg_node(rcv_msg.msg_type);
	if (msg_info != NULL) {
		pr_info("receive message: %s succ.\n", msg_info->msg_name);
		msg_info->recv_msg_proc(&rcv_msg, msg_info->msg_complete);
	}

	return;
}

static int soundtrigger_mailbox_isr_register(mb_msg_cb receive_func)
{
	unsigned int ret = MAILBOX_OK;

	if (receive_func == NULL) {
		pr_err("receive func is null\n");
		return ERROR;
	}

	ret = DRV_MAILBOX_REGISTERRECVFUNC(MAILBOX_MAILCODE_HIFI_TO_ACPU_WAKEUP,
		receive_func, NULL);
	if (ret != MAILBOX_OK) {
		pr_err("register receive func error, ret:%u, mailcode:0x%x\n",
			ret, MAILBOX_MAILCODE_HIFI_TO_ACPU_WAKEUP);
		return ERROR;
	}

	return 0;
}

static bool is_wakeup_start(bool is_lp)
{
	if (is_lp)
		return wakeup_lp_is_start;
	else
		return wakeup_is_start;
}

static void set_wakeup_start(bool is_lp, bool value)
{
	if (is_lp)
		wakeup_lp_is_start = value;
	else
		wakeup_is_start = value;
}

int parameter_set_msg(unsigned short msg_type, int module_id, struct parameter_set *set_val)
{
	int ret;
	unsigned long retval;
	struct parameter_set_msg set_msg;
	struct msg_node *parameter_set_msg_node = select_msg_node(msg_type);

	if (parameter_set_msg_node == NULL || parameter_set_msg_node->msg_complete == NULL)
		return -EINVAL;

	init_completion(parameter_set_msg_node->msg_complete);

	memset(&set_msg, 0, sizeof(set_msg));/* unsafe_function_ignore: memset */
	pr_info("parameter_set_msg \n");
	set_msg.msg_type = msg_type;
	set_msg.module_id = module_id;
	memcpy(&set_msg.para, set_val, sizeof(*set_val));
	if (set_msg.para.key == MLIB_ST_PARA_MODEL) {
		if (set_val->model.length <= HISI_AP_AUDIO_WAKEUP_MODEL_SIZE) {
			if (g_model_buf) {
				memcpy(g_model_buf, set_val->model.piece, set_val->model.length);
			} else {
				pr_err("g_model_buf is NULL\n");
			}
		} else {
			pr_err("model length exceed %u\n", set_val->model.length);
			return -ETIME;
		}
	}
	ret = soundtrigger_mailbox_send_data(&set_msg, sizeof(set_msg), 0);
	if (ret)
	{
		pr_err("parameter_set_msg: soundtrigger_mailbox_send_data error(%d)\n", ret);
		return ret;
	}

	retval = wait_for_completion_timeout(parameter_set_msg_node->msg_complete, SOCDSP_WAKEUP_MSG_TIMEOUT);
	if (retval == 0) {
		pr_err("parameter set msg send timeout\n");
		return -ETIME;
	}

	return ret;
}

int start_recognition_msg(unsigned short msg_type, int module_id, bool is_lp)
{
	int ret;
	unsigned long retval;
	struct wakeup_start_msg start_msg;
	struct msg_node *start_msg_node = select_msg_node(msg_type);

	if (start_msg_node == NULL || start_msg_node->msg_complete == NULL)
		return -EINVAL;

	init_completion(start_msg_node->msg_complete);

	memset(&start_msg, 0, sizeof(start_msg));/* unsafe_function_ignore: memset */
	pr_info("start_recognition_msg \n");
	start_msg.msg_type = msg_type;
	start_msg.module_id = module_id;

	ret = soundtrigger_mailbox_send_data(&start_msg, sizeof(start_msg), 0);
	if (ret) {
		pr_err("start_recognition_msg: soundtrigger_mailbox_send_data error(%d)\n", ret);
		return ret;
	}

	retval = wait_for_completion_timeout(start_msg_node->msg_complete, SOCDSP_WAKEUP_MSG_TIMEOUT);
	if (retval == 0) {
		pr_err("start recognition msg send timeout\n");
		return -ETIME;
	}

	set_wakeup_start(is_lp, true);

	return ret;
}

int stop_recognition_msg(unsigned short msg_type, int module_id, bool is_lp)
{
	int ret = 0;
	unsigned long retval;
	struct wakeup_stop_msg stop_msg;
	struct msg_node *stop_msg_node = select_msg_node(msg_type);

	if (stop_msg_node == NULL || stop_msg_node->msg_complete == NULL)
		return -EINVAL;

	if (!is_wakeup_start(is_lp))
		return ret;

	init_completion(stop_msg_node->msg_complete);

	memset(&stop_msg, 0, sizeof(stop_msg));/* unsafe_function_ignore: memset */
	pr_info("stop_recognition_msg \n");
	stop_msg.msg_type = msg_type;
	stop_msg.module_id = module_id;

	ret = soundtrigger_mailbox_send_data(&stop_msg, sizeof(stop_msg), 0);
	if (ret)
	{
		pr_err("stop_recognition_msg: soundtrigger_mailbox_send_data error(%d)\n", ret);
		return ret;
	}

	retval = wait_for_completion_timeout(stop_msg_node->msg_complete, SOCDSP_WAKEUP_MSG_TIMEOUT);
	if (retval == 0) {
		pr_err("start recognition msg send timeout\n");
		return -ETIME;
	}

	set_wakeup_start(is_lp, false);

	return ret;
}

int get_handle_msg(unsigned short msg_type, int *socdsp_handle)
{
	int ret;
	unsigned long retval;
	struct parameter_get_msg get_msg;
	int *handle = NULL;
	struct msg_node *parameter_get_msg_node = select_msg_node(msg_type);

	if (parameter_get_msg_node == NULL || parameter_get_msg_node->msg_complete == NULL)
		return -EINVAL;

	if (msg_type == WAKEUP_CHN_MSG_PARAMETER_GET)
		handle = &g_socdsp_handle;
	else if (msg_type == LP_WAKEUP_CHN_MSG_PARAMETER_GET)
		handle = &g_socdsp_lp_handle;
	else
		return -EINVAL;

	init_completion(parameter_get_msg_node->msg_complete);

	memset(&get_msg, 0, sizeof(get_msg));/* unsafe_function_ignore: memset */
	pr_info("get_handle_msg \n");
	get_msg.msg_type = msg_type;
	*handle = 0;
	ret = soundtrigger_mailbox_send_data(&get_msg, sizeof(get_msg), 0);
	if (ret) {
		pr_err("get_handle_msg: soundtrigger_mailbox_send_data error(%d)\n", ret);
		return ret;
	}

	retval = wait_for_completion_timeout(parameter_get_msg_node->msg_complete, SOCDSP_WAKEUP_MSG_TIMEOUT);
	if (retval == 0) {
		pr_err("get handle msg send timeout\n");
		return -ETIME;
	} else {
		*socdsp_handle = *handle;
	}

	return ret;
}

int soundtrigger_mailbox_init(void)
{
	int ret = 0;
	pr_info("soundtrigger mailbox init\n");

	ret = soundtrigger_mailbox_isr_register(soundtrigger_mailbox_recv_proc);/*lint !e611 */
	if (ret) {
		pr_err("soundtrigger mailbox receive isr registe failed:%d\n", ret);
		return -EIO;
	}

	wakeup_source_init(&soundtrigger_rcv_wake_lock, "soundtrigger_rcv_msg");

	g_model_buf = ioremap_wc(HISI_AP_AUDIO_WAKEUP_MODEL_ADDR, HISI_AP_AUDIO_WAKEUP_MODEL_SIZE);
	if (g_model_buf == NULL) {
		pr_err("model buffer ioremap err\n");
		DRV_MAILBOX_REGISTERRECVFUNC(MAILBOX_MAILCODE_HIFI_TO_ACPU_WAKEUP, NULL, NULL);
		return -ENOMEM;
	}

	return ret;
}

void soundtrigger_mailbox_deinit(void)
{
	pr_info("soundtrigger_mailbox_deinit \n");

	if (g_model_buf != NULL) {
		iounmap(g_model_buf);
		g_model_buf = NULL;
	}
	DRV_MAILBOX_REGISTERRECVFUNC(MAILBOX_MAILCODE_HIFI_TO_ACPU_WAKEUP, NULL, NULL);
	wakeup_source_trash(&soundtrigger_rcv_wake_lock);
}

