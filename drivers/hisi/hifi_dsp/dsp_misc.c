/*
 * dsp misc.c
 *
 * dsp misc driver.
 *
 * Copyright (c) 2015-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "dsp_misc.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/suspend.h>
#include <linux/reboot.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/uaccess.h>

#include <asm/memory.h>
#include <dsm/dsm_pub.h>

#include "audio_hifi.h"
#include "drv_mailbox_msg.h"
#include "bsp_drv_ipc.h"
#include "dsp_om.h"
#include "om_debug.h"
#include "om_bigdata.h"


#include "hisi_lb.h"
#include "dsp_lpp.h"
#include "dsp_msg.h"

#define DTS_COMP_HIFIDSP_NAME "hisilicon,k3hifidsp"

static DEFINE_SEMAPHORE(s_misc_sem);

struct msg_proc {
	unsigned int cmd_id;
	unsigned int work_id;
	void (*proc)(enum socdsp_om_work_id work_id, const unsigned char *addr,
		unsigned int len);
};

struct dsp_misc_priv {
	int wait_flag;
	unsigned char *base_virt;
	unsigned char *base_phy;
	struct device *dev;
	struct mutex ioctl_mutex;
	struct dsp_misc_proc misc_proc;
	enum dsp_platform_type platform_type;
};

static struct dsp_misc_priv g_misc_data;
static struct notifier_block g_dsp_sr_nb;
static struct notifier_block g_dsp_reboot_nb;
static atomic_t volatile g_dsp_in_suspend;
static atomic_t volatile g_dsp_in_saving;
static bool g_request_flag[SYSCACHE_SESSION_CNT] = { false, false };

unsigned long try_copy_to_user(void __user *to, const void *from, unsigned long n)
{
	int try_times = 0;
	unsigned long ret = 0;

	while (try_times < RETRY_COUNT) {
		ret = copy_to_user(to, from, n);
		if (ret == 0)
			break;

		try_times++;
		logw("copy to user fail ret %lu, retry %d times\n", ret, try_times);
	}

	return ret;
}

unsigned long try_copy_from_user(void *to, const void __user *from, unsigned long n)
{
	int try_times = 0;
	unsigned long ret = 0;

	while (try_times < RETRY_COUNT) {
		ret = copy_from_user(to, from, n);
		if (ret == 0)
			break;

		try_times++;
		logw("copy from user fail ret %lu, retry %d times\n", ret, try_times);
	}

	return ret;
}


static unsigned int get_syscache_pid(unsigned int session)
{
	if (session == SYSCACHE_SESSION_AUDIO)
		return PID_AUDIO;

	return PID_VOICE;
}

void dsp_reset_release_syscache(void)
{
	int ret;
	unsigned int i;

	for (i = 0; i < SYSCACHE_SESSION_CNT; i++) {
		if (!g_request_flag[i])
			continue;

		ret = lb_release_quota(get_syscache_pid(i));
		if (ret != 0) {
			loge("reset release syscache fail. ret %d pid %u\n",
				ret, get_syscache_pid(i));
			return;
		}
		g_request_flag[i] = false;
		logi("reset release syscache success\n");
	}
}

static void set_audio_syscache_quota(enum socdsp_om_work_id work_id,
	const unsigned char *data, unsigned int len)
{
	int ret;
	unsigned int pid;
	struct syscache_quota_msg *msg = NULL;

	if (!data || len != sizeof(*msg)) {
		loge("data is null or size is invaled, size: %u", len);
		return;
	}

	msg = (struct syscache_quota_msg *)data;

	if (msg->session >= SYSCACHE_SESSION_CNT ||
		msg->msg_type > SYSCACHE_QUOTA_REQUEST) {
		loge("msg info error, type %u, session %u\n",
			msg->msg_type, msg->session);
		return;
	}

	logi("syscache quota msg info type %u session %u\n",
		msg->msg_type, msg->session);

	pid = get_syscache_pid(msg->session);

	if (msg->msg_type == SYSCACHE_QUOTA_REQUEST) {
		ret = lb_request_quota(pid);
		if (ret != 0) {
			loge("request syscache fail ret %d pid %u\n", ret, pid);
			return;
		}
		g_request_flag[msg->session] = true;
	} else {
		ret = lb_release_quota(pid);
		if (ret != 0) {
			loge("release syscache fail ret %d pid %u\n", ret, pid);
			return;
		}
		g_request_flag[msg->session] = false;
	}

	logi("set syscache quota success\n");
}

void dsp_watchdog_send_event(void)
{
	int ret;
	char *envp[2] = { "hifi_watchdog", NULL };


	logi("soc dsp watchdog coming, now reset mediaserver\n");
	ret = kobject_uevent_env(&g_misc_data.dev->kobj, KOBJ_CHANGE, envp);
	if (ret != 0)
		loge("send uevent failed, retval: %d\n", ret);
}

const static struct msg_proc msg_proc_table[] = {
	{ ID_AUDIO_AP_OM_CMD, SOCDSP_OM_WORK_AUDIO_OM_DETECTION, socdsp_om_rev_data_handle },
	{ ID_AUDIO_AP_3A_CMD, SOCDSP_OM_WORK_VOICE_3A, socdsp_om_rev_data_handle },
	{ ID_AUDIO_AP_VOICE_BSD_PARAM_CMD, SOCDSP_OM_WORK_VOICE_BSD, socdsp_om_rev_data_handle },
	{ ID_AUDIO_AP_OM_DUMP_CMD, 0, NULL },
	{ ID_DSP_AP_BIGDATA_CMD, SOCDSP_OM_WORK_VOICE_BIGDATA, socdsp_om_rev_data_handle },
	{ ID_DSP_AP_AUXHEAR_CMD, SOCDSP_OM_WORK_AUXHEAR_BIGDATA, socdsp_om_rev_data_handle },
	{ ID_DSP_AP_SMARTPA_DFT, SOCDSP_OM_WORK_SMARTPA_DFT, socdsp_om_rev_data_handle },
	{ ID_DSP_AP_AUDIO_DB, SOCDSP_OM_WORK_AUDIO_DB, socdsp_om_rev_data_handle },
	{ ID_DSP_AP_SYSCACHE_QUOTA_CMD, 0, set_audio_syscache_quota },
};

void dsp_misc_msg_process(void *cmd)
{
	unsigned int i;
	unsigned int cmd_id = *(unsigned int *)cmd;
	unsigned int size = ARRAY_SIZE(msg_proc_table);
	struct common_dsp_cmd *common_cmd = cmd;
	struct socdsp_om_ap_data *rev_data = cmd;

	for (i = 0; i < size; i++) {
		if (cmd_id == msg_proc_table[i].cmd_id) {
			if (msg_proc_table[i].proc)
				msg_proc_table[i].proc(msg_proc_table[i].work_id,
					rev_data->data, rev_data->data_len);
			else
				logi("notify to dump dsp log, errtype: %u\n",
					common_cmd->value);
			return;
		}
	}

	logi("cmd id is not in proc table: %u\n", cmd_id);
}

static int mail_buff_init(struct recv_request **recv, unsigned int mail_len,
	struct mb_queue *mail_handle)
{
	unsigned int ret;

	*recv = kmalloc(sizeof(struct recv_request), GFP_ATOMIC);
	if (!(*recv)) {
		loge("recv kmalloc failed\n");
		return -ENOMEM;
	}
	memset(*recv, 0, sizeof(**recv));

	(*recv)->rev_msg.mail_buff_len = mail_len;
	(*recv)->rev_msg.mail_buff = kmalloc(mail_len, GFP_ATOMIC);
	if (!(*recv)->rev_msg.mail_buff) {
		loge("mail buff kmalloc failed\n");
		return -ENOMEM;
	}
	memset((*recv)->rev_msg.mail_buff, 0, mail_len);

	/* copy remain content to buff */
	ret = mailbox_read_msg_data(mail_handle, (char *)((*recv)->rev_msg.mail_buff),
		(unsigned int *)(&(*recv)->rev_msg.mail_buff_len));
	if ((ret != 0) || ((*recv)->rev_msg.mail_buff_len == 0)) {
		loge("empty point or data length error ret:%u, mail size: %u\n",
			ret, (*recv)->rev_msg.mail_buff_len);
		return -EINVAL;
	}

	logi("mail buff len: %u, msg id: 0x%x\n", (*recv)->rev_msg.mail_buff_len,
		*((unsigned int *)((*recv)->rev_msg.mail_buff + mail_len - SIZE_CMD_ID)));

	return 0;
}

static void mail_buff_deinit(struct recv_request **recv)
{
	if (*recv) {
		if ((*recv)->rev_msg.mail_buff) {
			kfree((*recv)->rev_msg.mail_buff);
			(*recv)->rev_msg.mail_buff = NULL;
		}

		kfree(*recv);
		*recv = NULL;
	}
}

static void dsp_misc_handle_mail(const void *usr_para,
	struct mb_queue *mail_handle, unsigned int mail_len)
{
	int ret;
	struct recv_request *recv = NULL;
	struct dsp_chn_cmd *cmd_para = NULL;
	void *recmsg = NULL;

	if (!mail_handle) {
		loge("mail handle is null\n");
		return;
	}

	if (mail_len >= MAIL_LEN_MAX ||
		mail_len <= (SIZE_CMD_ID + sizeof(unsigned short))) {
		loge("mail len is invalid: %u, max: 512, min: 8\n", mail_len);
		return;
	}

	ret = mail_buff_init(&recv, mail_len, mail_handle);
	if (ret != 0)
		goto kfree;

	cmd_para = (struct dsp_chn_cmd *)(recv->rev_msg.mail_buff + mail_len - SIZE_CMD_ID);
	recmsg = (void *)recv->rev_msg.mail_buff;
	if (cmd_para->cmd_type == HIFI_CHN_SYNC_CMD) {
		ret = rcv_dsp_sync_msg(&recv->recv_node, cmd_para->sn);
		if (ret == 0)
			return;

		goto kfree;
	} else if ((cmd_para->cmd_type == HIFI_CHN_READNOTICE_CMD) &&
		(cmd_para->sn == ACPU_TO_HIFI_ASYNC_CMD)) {
		if (*((unsigned short *)recmsg) == ID_AUDIO_AP_PLAY_DONE_IND) {
			rcv_dsp_play_done_msg(&recv->recv_node);
			return;
		}

		if (*((unsigned short *)recmsg) == ID_AUDIO_AP_UPDATE_PCM_BUFF_CMD )
			soc_dsp_wakeup_pcm_read_thread(0);

		dsp_misc_msg_process(recmsg);
	} else {
		loge("unknown msg comed from dsp, type: %d\n", cmd_para->cmd_type);
	}

kfree:
	mail_buff_deinit(&recv);
}

static int dsp_misc_open(struct inode *finode, struct file *fd)
{
	logi("open device\n");
	return OK;
}

static int dsp_misc_release(struct inode *finode, struct file *fd)
{
	logi("close device\n");
	return OK;
}

static long dsp_misc_ioctl(struct file *fd, unsigned int cmd, unsigned long arg)
{
	long ret;
	void __user *data32 = (void __user *)(uintptr_t)arg;

	if (!(void __user *)(uintptr_t)arg) {
		loge("input buff is null\n");
		return (long)-EINVAL;
	}

	if (!is_dsp_img_loaded()) {
		loge("dsp is not loaded\n");
		return (long)-EINVAL;
	}

	if (cmd == HIFI_MISC_IOCTL_SYNCMSG) {
		logi("ioctl: HIFI_MISC_IOCTL_SYNCMSG\n");
		ret = down_interruptible(&s_misc_sem);
		if (ret != 0) {
			loge("syncmsg wake up by other irq err: %ld\n", ret);
			return ret;
		}
		ret = g_misc_data.misc_proc.sync_msg_proc((uintptr_t)data32);
		up(&s_misc_sem);

		return ret;
	}
	mutex_lock(&g_misc_data.ioctl_mutex);
	ret = dsp_msg_process_cmd(cmd, (uintptr_t)data32);
	mutex_unlock(&g_misc_data.ioctl_mutex);
	if (ret == ERROR)
		loge("ioctl: invalid cmd 0x%x\n", cmd);

	return ret;
}

static long dsp_misc_ioctl32(struct file *fd, unsigned int cmd, unsigned long arg)
{
	void *user_ptr = compat_ptr(arg);

	return dsp_misc_ioctl(fd, cmd, (uintptr_t)user_ptr);
}

static int dsp_misc_mmap(struct file *file, struct vm_area_struct *vma)
{
	int ret;
	unsigned long phys_page_addr;
	unsigned long size;

	if (!vma) {
		logd("input error: vma is NULL\n");
		return ERROR;
	}

	phys_page_addr = (uintptr_t)g_misc_data.base_phy >> PAGE_SHIFT;
	size = ((unsigned long)vma->vm_end - (unsigned long)vma->vm_start);
	logd("vma 0x%pK\n", vma);
	logd("size %lu, vma->vm_start %pK, end %pK\n", size,
		(void *)(uintptr_t)vma->vm_start, (void *)(uintptr_t)vma->vm_end);
	logd("phys_page_addr 0x%pK\n", (void *)(uintptr_t)phys_page_addr);

	if (size > DSP_MUSIC_DATA_SIZE) {
		loge("size error, size: %lu\n", size);
		return ERROR;
	}

	vma->vm_page_prot = pgprot_writecombine(PAGE_SHARED);

	ret = remap_pfn_range(vma, vma->vm_start, phys_page_addr, size,
		vma->vm_page_prot);
	if (ret != 0) {
		loge("remap pfn range ret %d\n", ret);
		return ERROR;
	}

	return ret;
}

static const struct file_operations dsp_misc_fops = {
	.owner = THIS_MODULE,
	.open = dsp_misc_open,
	.release = dsp_misc_release,
	.unlocked_ioctl = dsp_misc_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = dsp_misc_ioctl32,
#endif
	.mmap = dsp_misc_mmap,
};

static struct miscdevice dsp_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "hifi_misc",
	.fops = &dsp_misc_fops,
};

static int dsp_fb_notifier_call(struct notifier_block *nb,
	unsigned long action, void *data)
{
	struct fb_event *event = data;
	unsigned int *screen_status = NULL;

	if (action != FB_EVENT_BLANK)
		return 0;

	if (!event)
		return 0;

	/* only main screen on/off info send to dsp */
	if (registered_fb[0] != event->info)
		return 0;

	if (!event->data)
		return 0;

	if (!g_misc_data.base_virt) {
		logw("dsp priv base virt is NULL\n");
		return 0;
	}

	screen_status = (int *)(g_misc_data.base_virt +
		DRV_DSP_SCREEN_STATUS - DSP_UNSEC_BASE_ADDR);

	switch (*((int *)event->data)) {
	case FB_BLANK_UNBLANK:
		logi("screen on\n");
		*screen_status = 1;
		break;
	case FB_BLANK_POWERDOWN:
		logi("screen off\n");
		*screen_status = 0;
		break;
	default:
		break;
	}

	return 0;
}

static struct notifier_block dsp_fb_notifier_block = {
	.notifier_call = dsp_fb_notifier_call,
};

#ifdef CONFIG_PM
static int sr_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	switch (event) {
	case PM_POST_HIBERNATION:
	case PM_POST_SUSPEND:
		logi("resume +\n");
		atomic_set(&g_dsp_in_suspend, 0);
		logi("resume -\n");
		break;

	case PM_HIBERNATION_PREPARE:
	case PM_SUSPEND_PREPARE:
		logi("suspend +\n");
		atomic_set(&g_dsp_in_suspend, 1);
		while (true) {
			if (atomic_read(&g_dsp_in_saving))
				msleep(100);
			else
				break;
		}
		logi("suspend -\n");
		break;
	default:
		return NOTIFY_DONE;
	}

	return NOTIFY_OK;
}
#endif

static int reboot_notifier(struct notifier_block *nb, unsigned long foo, void *bar)
{
	logi("reboot +\n");
	atomic_set(&g_dsp_in_suspend, 1);

	while (true) {
		if (atomic_read(&g_dsp_in_saving))
			msleep(100);
		else
			break;
	}
	logi("reboot -\n");

	return 0;
}

void dsp_get_log_signal(void)
{
	while (true) {
		if (atomic_read(&g_dsp_in_suspend)) {
			msleep(100);
		} else {
			atomic_set(&g_dsp_in_saving, 1);
			break;
		}
	}
}

void dsp_release_log_signal(void)
{
	atomic_set(&g_dsp_in_saving, 0);
}

int dsp_send_msg(unsigned int mailcode, const void *data, unsigned int length)
{
	if (is_dsp_img_loaded())
		return mailbox_send_msg(mailcode, data, length);

	return ERROR;
}

static void set_platform_type(struct platform_device *pdev,
	struct dsp_misc_priv *priv)
{
	const char *platform_type = NULL;

	if (!pdev || !priv) {
		loge("param error\n");
		return;
	}

	priv->platform_type = DSP_PLATFORM_ASIC;

	if (!of_property_read_string(pdev->dev.of_node, "platform-type", &platform_type)) {
		if (!strncmp(platform_type, "ASIC", strlen("ASIC"))) {
			priv->platform_type = DSP_PLATFORM_ASIC;
			logi("platform type is ASIC\n");
		} else if (!strncmp(platform_type, "FPGA", strlen("FPGA"))) {
			priv->platform_type = DSP_PLATFORM_FPGA;
			logi("platform type is FPGA\n");
		} else {
			logi("get dsp platform type error, set defult type[ASIC]\n");
		}
	} else {
		loge("get dsp property fail, set defult type[ASIC]\n");
	}
}

enum dsp_platform_type dsp_misc_get_platform_type(void)
{
	return g_misc_data.platform_type;
}

static void misc_data_init(struct platform_device *pdev)
{
	memset(&g_misc_data, 0, sizeof(struct dsp_misc_priv));
	g_misc_data.dev = &pdev->dev;

#ifdef CONFIG_PM
	/* Register to get PM events */
	g_dsp_sr_nb.notifier_call = sr_event;
	g_dsp_sr_nb.priority = -1;
	(void)register_pm_notifier(&g_dsp_sr_nb);
#endif

	g_dsp_reboot_nb.notifier_call = reboot_notifier;
	g_dsp_reboot_nb.priority = -1;
	(void)register_reboot_notifier(&g_dsp_reboot_nb);

	set_platform_type(pdev, &g_misc_data);
}

static void misc_data_deinit(void)
{
#ifdef CONFIG_PM
	unregister_pm_notifier(&g_dsp_sr_nb);
#endif
	unregister_reboot_notifier(&g_dsp_reboot_nb);
}

static int base_addr_init(void)
{
	g_misc_data.base_phy = (unsigned char *)DSP_UNSEC_BASE_ADDR;
	g_misc_data.base_virt = (unsigned char *)ioremap_wc(DSP_UNSEC_BASE_ADDR, /*lint !e446*/
		DSP_UNSEC_REGION_SIZE);/*lint !e446*/
	if (!g_misc_data.base_virt) {
		loge("ioremap wc error\n");
		return ERROR;
	}

	memset(g_misc_data.base_virt + (DSP_AP_NV_DATA_ADDR - DSP_UNSEC_BASE_ADDR),
		0, DSP_AP_NV_DATA_SIZE);
	memset(g_misc_data.base_virt + (PCM_PLAY_BUFF_LOCATION - DSP_UNSEC_BASE_ADDR),
		0, PCM_PLAY_BUFF_SIZE);

	return OK;
}

static void base_addr_deinit(void)
{
	if (g_misc_data.base_virt) {
		iounmap(g_misc_data.base_virt);
		g_misc_data.base_virt = NULL;
	}
}

static int ipc_init(struct platform_device *pdev)
{
	int ret;

	ret = drv_ipc_int_init();
	if (ret != 0) {
		loge("dsp ipc init fail\n");
		return ret;
	}

	ret = mailbox_init();
	if (ret != 0) {
		loge("dsp mailbox init fail\n");
		goto err_mailbox_init;
	}

	ret = mailbox_reg_msg_cb(MAILBOX_MAILCODE_HIFI_TO_ACPU_MISC,
		dsp_misc_handle_mail, NULL);
	if (ret != OK) {
		loge("dsp mailbox handle func register fail\n");
		goto err_mailbox_reg;
	}

	return 0;

err_mailbox_reg:
	mailbox_destory();

err_mailbox_init:
	drv_ipc_int_deinit();

	return ERROR;
}

static void ipc_deinit(void)
{
	mailbox_destory();

	drv_ipc_int_deinit();
}

static int soc_dsp_misc_probe(struct platform_device *pdev)
{
	int ret;

	pr_info("dsp pdev name[%s]\n", pdev->name);
	ret = misc_register(&dsp_misc_device);
	if (ret != 0) {
		pr_err("dsp misc device register fail ret: %d\n", ret);
		return ERROR;
	}

	misc_data_init(pdev);

	ret = base_addr_init();
	if (ret != 0)
		goto err_base_addr_init;

	dsp_om_init(pdev, g_misc_data.base_virt);

	ret = dsp_lpp_init(g_misc_data.base_virt);
	if (ret != 0)
		goto err_dsp_lpp_init;

	dsp_msg_init(&g_misc_data.misc_proc, g_misc_data.base_virt, g_misc_data.base_phy);

	mutex_init(&g_misc_data.ioctl_mutex);

	ret = ipc_init(pdev);
	if (ret != 0)
		goto err_ipc_init;

	memset(g_misc_data.base_virt + (DRV_DSP_SCREEN_STATUS - DSP_UNSEC_BASE_ADDR),
		0, DRV_DSP_SCREEN_STATUS_SIZE);
	fb_register_client(&dsp_fb_notifier_block);

	return 0;

err_ipc_init:
	mutex_destroy(&g_misc_data.ioctl_mutex);

	dsp_msg_deinit();
	dsp_lpp_deinit();

err_dsp_lpp_init:
	dsp_om_deinit(pdev);
	base_addr_deinit();

err_base_addr_init:
	misc_data_deinit();

	(void)misc_deregister(&dsp_misc_device);

	return ERROR;
}

static int soc_dsp_misc_remove(struct platform_device *pdev)
{
	fb_unregister_client(&dsp_fb_notifier_block);

	ipc_deinit();

	mutex_destroy(&g_misc_data.ioctl_mutex);

	dsp_msg_deinit();
	dsp_lpp_deinit();

	dsp_om_deinit(pdev);
	base_addr_deinit();

	misc_data_deinit();

	(void)misc_deregister(&dsp_misc_device);

	return OK;
}

static const struct of_device_id dsp_match_table[] = {
	{
		.compatible = DTS_COMP_HIFIDSP_NAME,
		.data = NULL,
	},
	{}
};
MODULE_DEVICE_TABLE(of, dsp_match_table);

static struct platform_driver dsp_misc_driver = {
	.driver = {
		.name = "hifi_dsp_misc",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(dsp_match_table),
	},
	.probe = soc_dsp_misc_probe,
	.remove = soc_dsp_misc_remove,
};

static int __init dsp_misc_init(void)
{
	return platform_driver_register(&dsp_misc_driver);
}

static void __exit dsp_misc_exit(void)
{
	platform_driver_unregister(&dsp_misc_driver);
}

fs_initcall_sync(dsp_misc_init);
module_exit(dsp_misc_exit);

MODULE_DESCRIPTION("dsp driver");
MODULE_LICENSE("GPL v2");

