/*
 * tc_client_driver.c
 *
 * function for proc open,close session and invoke
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "tc_client_driver.h"
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/spinlock_types.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
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
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/pid.h>
#include <linux/security.h>
#include <linux/cred.h>
#include <linux/namei.h>
#include <linux/thread_info.h>
#include <linux/highmem.h>
#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/file.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/security.h>
#include <linux/sched/mm.h>
#include <linux/sched/signal.h>
#include <linux/sched/task.h>
#include <linux/acpi.h>
#include <linux/completion.h>
#include "smc_smp.h"
#include "teek_client_constants.h"
#include "agent.h"
#include "mem.h"
#include "gp_ops.h"
#include "tc_ns_log.h"
#include "tc_ns_client.h"
#include "mailbox_mempool.h"
#include "tz_spi_notify.h"
#include "static_ion_mem.h"
#include "teec_daemon_auth.h"
#include "security_auth_enhance.h"
#include "client_hash_auth.h"
#include "tui.h"
#include "auth_base_impl.h"
#include "tlogger.h"
#include "tzdebug.h"
#include "session_manager.h"
#include "dynamic_ion_mem.h"
#include "ko_adapt.h"
#include "tz_pm.h"

static dev_t g_tc_ns_client_devt;
static struct class *g_driver_class;
static struct cdev g_tc_ns_client_cdev;
static struct device_node *g_dev_node;


static unsigned int g_device_file_cnt = 1;
static DEFINE_MUTEX(g_device_file_cnt_lock);

/* dev node list and itself has mutex to avoid race */
struct tc_ns_dev_list g_tc_ns_dev_list;

static struct task_struct *g_teecd_task;

struct tc_ns_dev_list *get_dev_list(void)
{
	return &g_tc_ns_dev_list;
}

static int tc_ns_get_tee_version(const struct tc_ns_dev_file *dev_file,
	void __user *argp)
{
	unsigned int version;
	struct tc_ns_smc_cmd smc_cmd = { {0}, 0 };
	int ret = 0;
	struct mb_cmd_pack *mb_pack = NULL;
	bool is_teecd = (g_teecd_task == current->group_leader) &&
		(!tc_ns_get_uid());

	if (!argp) {
		tloge("error input parameter\n");
		return -EINVAL;
	}

	if (!is_teecd) {
		tloge("ioctl is not from teecd and return\n");
		return -EACCES;
	}

	mb_pack = mailbox_alloc_cmd_pack();
	if (!mb_pack) {
		tloge("alloc mb pack failed\n");
		return -ENOMEM;
	}

	mb_pack->operation.paramtypes = TEEC_VALUE_OUTPUT;
	smc_cmd.global_cmd = true;
	smc_cmd.cmd_id = GLOBAL_CMD_ID_GET_TEE_VERSION;
	smc_cmd.dev_file_id = dev_file->dev_file_id;
	smc_cmd.operation_phys = virt_to_phys(&mb_pack->operation);
	smc_cmd.operation_h_phys =
		virt_to_phys(&mb_pack->operation) >> ADDR_TRANS_NUM;

	if (tc_ns_smc(&smc_cmd)) {
		ret = -EPERM;
		tloge("smc call returns error ret 0x%x\n", smc_cmd.ret_val);
	}

	version = mb_pack->operation.params[0].value.a;
	if (copy_to_user(argp, &version, sizeof(unsigned int)))
		ret = -EFAULT;
	mailbox_free(mb_pack);

	return ret;
}

/*
 * This is the login information
 * and is set teecd when client opens a new session
 */
#define MAX_BUF_LEN 4096

static int get_pack_name_len(struct tc_ns_dev_file *dev_file,
	const uint8_t *cert_buffer)
{
	if (memcpy_s(&dev_file->pkg_name_len, sizeof(dev_file->pkg_name_len),
		cert_buffer, sizeof(dev_file->pkg_name_len)))
		return -EFAULT;

	if (!dev_file->pkg_name_len ||
	    dev_file->pkg_name_len >= MAX_PACKAGE_NAME_LEN) {
		tloge("invalid pack name len: %u\n", dev_file->pkg_name_len);
		return -EINVAL;
	}

	tlogd("package name len is %u\n", dev_file->pkg_name_len);

	return 0;
}

static int get_public_key_len(struct tc_ns_dev_file *dev_file,
	const uint8_t *cert_buffer)
{
	if (memcpy_s(&dev_file->pub_key_len, sizeof(dev_file->pub_key_len),
		cert_buffer, sizeof(dev_file->pub_key_len)))
		return -EFAULT;

	if (dev_file->pub_key_len > MAX_PUBKEY_LEN) {
		tloge("invalid public key len: %u\n", dev_file->pub_key_len);
		return -EINVAL;
	}

	tlogd("publick key len is %u\n", dev_file->pub_key_len);

	return 0;
}

static int get_public_key(struct tc_ns_dev_file *dev_file,
	const uint8_t *cert_buffer)
{
	/* get public key */
	if (!dev_file->pub_key_len)
		return 0;

	if (memcpy_s(dev_file->pub_key, MAX_PUBKEY_LEN, cert_buffer,
		dev_file->pub_key_len)) {
		tloge("failed to copy pub key len\n");
		return -EINVAL;
	}

	return 0;
}

static bool is_cert_buffer_size_valid(int cert_buffer_size)
{
	/*
	 * GET PACKAGE NAME AND APP CERTIFICATE:
	 * The proc_info format is as follows:
	 * package_name_len(4 bytes) || package_name ||
	 * apk_cert_len(4 bytes) || apk_cert.
	 * or package_name_len(4 bytes) || package_name
	 * || exe_uid_len(4 bytes) || exe_uid.
	 * The apk certificate format is as follows:
	 * modulus_size(4bytes) ||modulus buffer
	 * || exponent size || exponent buffer
	 */
	if (cert_buffer_size > MAX_BUF_LEN || !cert_buffer_size) {
		tloge("cert buffer size is invalid!\n");
		return false;
	}

	return true;
}

static int alloc_login_buf(struct tc_ns_dev_file *dev_file,
	uint8_t **cert_buffer, unsigned int *cert_buffer_size)
{
	if (check_teecd_access()) {
		tloge("tc client login verification failed!\n");
		return -EACCES;
	}

	*cert_buffer_size = (unsigned int)(MAX_PACKAGE_NAME_LEN +
		MAX_PUBKEY_LEN + sizeof(dev_file->pkg_name_len) +
		sizeof(dev_file->pub_key_len));

	*cert_buffer = kmalloc(*cert_buffer_size, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)(*cert_buffer))) {
		tloge("failed to allocate login buffer!");
		return -ENOMEM;
	}

	return 0;
}

static int client_login_prepare(uint8_t *cert_buffer,
	const void __user *buffer, unsigned int cert_buffer_size)
{
	if (!is_cert_buffer_size_valid(cert_buffer_size))
		return -EINVAL;

	if (copy_from_user(cert_buffer, buffer, cert_buffer_size)) {
		tloge("Failed to get user login info!\n");
		return -EINVAL;
	}

	return 0;
}

static int tc_ns_client_login_func(struct tc_ns_dev_file *dev_file,
	const void __user *buffer)
{
	int ret;
	uint8_t *cert_buffer = NULL;
	uint8_t *temp_cert_buffer = NULL;
	unsigned int cert_buffer_size = 0;

	if (!dev_file)
		return -EINVAL;

	if (!buffer) {
		/*
		 * We accept no debug information
		 * because the daemon might  have failed
		 */
		dev_file->pkg_name_len = 0;
		dev_file->pub_key_len = 0;
		return 0;
	}

	mutex_lock(&dev_file->login_setup_lock);
	if (dev_file->login_setup) {
		tloge("login information cannot be set twice!\n");
		mutex_unlock(&dev_file->login_setup_lock);
		return -EINVAL;
	}

	ret = alloc_login_buf(dev_file, &cert_buffer, &cert_buffer_size);
	if (ret) {
		mutex_unlock(&dev_file->login_setup_lock);
		return ret;
	}

	temp_cert_buffer = cert_buffer;
	if (client_login_prepare(cert_buffer, buffer, cert_buffer_size)) {
		ret = -EINVAL;
		goto error;
	}

	ret = get_pack_name_len(dev_file, cert_buffer);
	if (ret)
		goto error;
	cert_buffer += sizeof(dev_file->pkg_name_len);

	if (strncpy_s(dev_file->pkg_name, MAX_PACKAGE_NAME_LEN, cert_buffer,
		dev_file->pkg_name_len)) {
		ret = -ENOMEM;
		goto error;
	}
	cert_buffer += dev_file->pkg_name_len;

	ret = get_public_key_len(dev_file, cert_buffer);
	if (ret)
		goto error;
	cert_buffer += sizeof(dev_file->pub_key_len);

	ret = get_public_key(dev_file, cert_buffer);
	dev_file->login_setup = true;

error:
	kfree(temp_cert_buffer);
	mutex_unlock(&dev_file->login_setup_lock);
	return ret;
}

int tc_ns_client_open(struct tc_ns_dev_file **dev_file, uint8_t kernel_api)
{
	struct tc_ns_dev_file *dev = NULL;

	tlogd("tc_client_open\n");
	if (!dev_file) {
		tloge("dev_file is NULL\n");
		return -EINVAL;
	}

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)dev)) {
		tloge("dev malloc failed\n");
		return -ENOMEM;
	}

	mutex_lock(&g_tc_ns_dev_list.dev_lock);
	list_add_tail(&dev->head, &g_tc_ns_dev_list.dev_file_list);
	mutex_unlock(&g_tc_ns_dev_list.dev_lock);
	mutex_lock(&g_device_file_cnt_lock);
	dev->dev_file_id = g_device_file_cnt;
	g_device_file_cnt++;
	mutex_unlock(&g_device_file_cnt_lock);
	INIT_LIST_HEAD(&dev->shared_mem_list);
	dev->login_setup = 0;
	dev->kernel_api = kernel_api;
	dev->load_app_flag = 0;
	mutex_init(&dev->service_lock);
	mutex_init(&dev->shared_mem_lock);
	mutex_init(&dev->login_setup_lock);
	init_completion(&dev->close_comp);
	*dev_file = dev;

	return 0;
}

static void del_dev_node(struct tc_ns_dev_file *dev)
{
	if (!dev)
		return;

	mutex_lock(&g_tc_ns_dev_list.dev_lock);
	list_del(&dev->head);
	mutex_unlock(&g_tc_ns_dev_list.dev_lock);
}

void free_dev(struct tc_ns_dev_file *dev)
{
	del_dev_node(dev);
	tee_agent_clear_dev_owner(dev);
	if (memset_s(dev, sizeof(*dev), 0, sizeof(*dev)))
		tloge("Caution, memset dev fail!\n");
	kfree(dev);
}

int tc_ns_client_close(struct tc_ns_dev_file *dev)
{
	if (!dev) {
		tloge("invalid dev(null)\n");
		return -EINVAL;
	}

	close_unclosed_session_in_kthread(dev);

#ifdef CONFIG_TEE_TUI
	if (dev->dev_file_id == tui_attach_device())
		free_tui_caller_info();
#endif

	kill_ion_by_cafd(dev->dev_file_id);
	/* for thirdparty agent, code runs here only when agent crashed */
	send_crashed_event_response_all(dev);
	free_dev(dev);

	return 0;
}

void shared_vma_open(struct vm_area_struct *vma)
{
	(void)vma;
}

static void release_vma_shared_mem(struct tc_ns_dev_file *dev_file,
	const struct vm_area_struct *vma)
{
	struct tc_ns_shared_mem *shared_mem = NULL;
	struct tc_ns_shared_mem *shared_mem_temp = NULL;
	bool find = false;

	mutex_lock(&dev_file->shared_mem_lock);
	list_for_each_entry_safe(shared_mem, shared_mem_temp,
			&dev_file->shared_mem_list, head) {
		if (shared_mem) {
			if (shared_mem->user_addr ==
				(void *)(uintptr_t)vma->vm_start) {
				shared_mem->user_addr = NULL;
				find = true;
			} else if (shared_mem->user_addr_ca ==
				(void *)(uintptr_t)vma->vm_start) {
				shared_mem->user_addr_ca = NULL;
				find = true;
			}

			if (!shared_mem->user_addr &&
				!shared_mem->user_addr_ca)
				list_del(&shared_mem->head);

			/* pair with tc client mmap */
			if (find) {
				put_sharemem_struct(shared_mem);
				break;
			}
		}
	}
	mutex_unlock(&dev_file->shared_mem_lock);
}

void shared_vma_close(struct vm_area_struct *vma)
{
	struct tc_ns_dev_file *dev_file = NULL;

	if (!vma) {
		tloge("vma is null\n");
		return;
	}

	dev_file = vma->vm_private_data;
	if (!dev_file) {
		tloge("vm private data is null\n");
		return;
	}

	if (g_teecd_task == current->group_leader && !tc_ns_get_uid() &&
		((g_teecd_task->flags & PF_EXITING) ||
		(current->flags & PF_EXITING))) {
		tlogd("teecd is killed, just return in vma close\n");
		return;
	}
	release_vma_shared_mem(dev_file, vma);
}

static struct vm_operations_struct g_shared_remap_vm_ops = {
	.open = shared_vma_open,
	.close = shared_vma_close,
};

static struct tc_ns_shared_mem *find_sharedmem(
	const struct vm_area_struct *vma,
	const struct tc_ns_dev_file *dev_file, bool *only_remap)
{
	struct tc_ns_shared_mem *shm_tmp = NULL;
	unsigned long len = vma->vm_end - vma->vm_start;

	/*
	 * using vma->vm_pgoff as share_mem index
	 * check if aready allocated
	 */
	list_for_each_entry(shm_tmp, &dev_file->shared_mem_list, head) {
		if (atomic_read(&shm_tmp->offset) == vma->vm_pgoff) {
			tlogd("sharemem already alloc, shm tmp->offset=%d\n",
				atomic_read(&shm_tmp->offset));
			/*
			 * args check:
			 * 1. this shared mem is already mapped
			 * 2. remap a different size shared_mem
			 */
			if (shm_tmp->user_addr_ca ||
				vma->vm_end - vma->vm_start != shm_tmp->len) {
				tloge("already remap once!\n");
				return NULL;
			}
			/* return the same sharedmem specified by vm_pgoff */
			*only_remap = true;
			get_sharemem_struct(shm_tmp);
			return shm_tmp;
		}
	}

	/* if not find, alloc a new sharemem */
	return tc_mem_allocate(len);
}

static int remap_shared_mem(struct vm_area_struct *vma,
	const struct tc_ns_shared_mem *shared_mem)
{
	int ret;

	vma->vm_flags |= VM_USERMAP;
	ret = remap_vmalloc_range(vma, shared_mem->kernel_addr, 0);
	if (ret)
		tloge("can't remap to user, ret = %d\n", ret);

	return ret;
}

/*
 * in this func, we need to deal with follow cases:
 * vendor CA alloc sharedmem (alloc and remap);
 * HIDL alloc sharedmem (alloc and remap);
 * system CA alloc sharedmem (only just remap);
 */
static int tc_client_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int ret;
	struct tc_ns_dev_file *dev_file = NULL;
	struct tc_ns_shared_mem *shared_mem = NULL;
	bool only_remap = false;

	if (!filp || !vma || !filp->private_data) {
		tloge("invalid args for tc mmap\n");
		return -EINVAL;
	}
	dev_file = filp->private_data;

	mutex_lock(&dev_file->shared_mem_lock);
	shared_mem = find_sharedmem(vma, dev_file, &only_remap);
	if (IS_ERR_OR_NULL(shared_mem)) {
		tloge("alloc shared mem failed\n");
		mutex_unlock(&dev_file->shared_mem_lock);
		return -ENOMEM;
	}

	ret = remap_shared_mem(vma, shared_mem);
	if (ret) {
		if (only_remap)
			put_sharemem_struct(shared_mem);
		else
			tc_mem_free(shared_mem);
		mutex_unlock(&dev_file->shared_mem_lock);
		return ret;
	}

	vma->vm_flags |= VM_DONTCOPY;
	vma->vm_ops = &g_shared_remap_vm_ops;
	shared_vma_open(vma);
	vma->vm_private_data = (void *)dev_file;

	if (only_remap) {
		shared_mem->user_addr_ca = (void *)(uintptr_t)vma->vm_start;
		mutex_unlock(&dev_file->shared_mem_lock);
		return ret;
	}
	shared_mem->user_addr = (void *)(uintptr_t)vma->vm_start;
	atomic_set(&shared_mem->offset, vma->vm_pgoff);
	get_sharemem_struct(shared_mem);
	list_add_tail(&shared_mem->head, &dev_file->shared_mem_list);
	mutex_unlock(&dev_file->shared_mem_lock);

	return ret;
}

static int ioctl_register_agent(struct tc_ns_dev_file *dev_file, unsigned long arg)
{
	int ret;
	struct agent_ioctl_args args;

	if (!arg) {
		tloge("arg is NULL\n");
		return -EFAULT;
	}

	if (copy_from_user(&args, (void *)(uintptr_t)arg, sizeof(args))) {
		tloge("copy agent args failed\n");
		return -EFAULT;
	}

	ret = tc_ns_register_agent(dev_file, args.id, args.buffer_size,
		&args.buffer, true);
	if (!ret) {
		if (copy_to_user((void *)(uintptr_t)arg, &args, sizeof(args)))
			tloge("copy agent user addr failed\n");
	}

	return ret;
}

static int ioctl_unregister_agent(const struct tc_ns_dev_file *dev_file,
	unsigned long arg)
{
	int ret;
	struct smc_event_data *event_data = NULL;

	event_data = find_event_control((unsigned int)arg);
	if (!event_data) {
		tloge("invalid agent id\n");
		return -EINVAL;
	}

	if (event_data->owner != dev_file) {
		tloge("invalid unregister request\n");
		put_agent_event(event_data);
		return -EINVAL;
	}

	put_agent_event(event_data);
	ret = tc_ns_unregister_agent((unsigned int)arg);

	return ret;
}

/* ioctls for the secure storage daemon */
static long tc_agent_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = -EINVAL;
	struct tc_ns_dev_file *dev_file = file->private_data;

	if (!dev_file) {
		tloge("invalid params\n");
		return -EINVAL;
	}

	switch (cmd) {
	case TC_NS_CLIENT_IOCTL_WAIT_EVENT:
		ret = tc_ns_wait_event((unsigned int)arg);
		break;
	case TC_NS_CLIENT_IOCTL_SEND_EVENT_RESPONSE:
		ret = tc_ns_send_event_response((unsigned int)arg);
		break;
	case TC_NS_CLIENT_IOCTL_REGISTER_AGENT:
		ret = ioctl_register_agent(dev_file, arg);
		break;
	case TC_NS_CLIENT_IOCTL_UNREGISTER_AGENT:
		ret = ioctl_unregister_agent(dev_file, arg);
		break;
	case TC_NS_CLIENT_IOCTL_SYC_SYS_TIME:
		ret = tc_ns_sync_sys_time(
			(struct tc_ns_client_time *)(uintptr_t)arg);
		break;
	case TC_NS_CLIENT_IOCTL_SET_NATIVECA_IDENTITY:
		ret = tc_ns_set_native_hash(arg, GLOBAL_CMD_ID_SET_CA_HASH);
		break;
	case TC_NS_CLIENT_IOCTL_LATEINIT:
		ret = tc_ns_late_init(arg);
		break;
	default:
		tloge("invalid cmd!");
		return ret;
	}
	tlogd("client ioctl ret = 0x%x\n", ret);
	return ret;
}

#ifdef CONFIG_TEE_TUI
static int tc_ns_tui_event(struct tc_ns_dev_file *dev_file, const void *argp)
{
	struct teec_tui_parameter tui_param = {0};
	int ret;

	if (!dev_file || !argp) {
		tloge("argp or dev is NULL\n");
		return -EINVAL;
	}

	if (copy_from_user(&tui_param, argp, sizeof(tui_param))) {
		tloge("copy from user failed\n");
		return -ENOMEM;
	}

	if (tui_param.event_type == TUI_POLL_CANCEL ||
		tui_param.event_type == TUI_POLL_NOTCH ||
		tui_param.event_type == TUI_POLL_FOLD) {
		ret = tui_send_event(tui_param.event_type, &tui_param);
	} else {
		tloge("no permission to send event\n");
		ret = -EACCES;
	}

	return ret;
}
#endif

static int tc_ns_send_cancel_cmd(struct tc_ns_dev_file *dev_file,
	void *argp, struct tc_ns_client_context *client_context)
{
	if (!argp) {
		tloge("argp is NULL input buffer\n");
		return -EINVAL;
	}
	if (copy_from_user(client_context, argp, sizeof(*client_context))) {
		tloge("copy from user failed\n");
		return -ENOMEM;
	}

	client_context->returns.code = TEEC_ERROR_GENERIC;
	client_context->returns.origin = TEEC_ORIGIN_COMMS;
	tloge("not support send cancle cmd now\n");
	if (copy_to_user(argp, client_context, sizeof(*client_context)))
		return -EFAULT;

	return 0;
}

uint32_t tc_ns_get_uid(void)
{
	struct task_struct *task = NULL;
	const struct cred *cred = NULL;
	uint32_t uid;

	rcu_read_lock();
	task = get_current();
	get_task_struct(task);
	rcu_read_unlock();
	cred = koadpt_get_task_cred(task);
	if (!cred) {
		tloge("failed to get uid of the task\n");
		put_task_struct(task);
		return (uint32_t)(-1);
	}

	uid = cred->uid.val;
	put_cred(cred);
	put_task_struct(task);
	tlogd("current uid is %u\n", uid);
	return uid;
}

static long tc_client_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg)
{
	int ret = -EFAULT;
	void *argp = (void __user *)(uintptr_t)arg;
	struct tc_ns_dev_file *dev_file = file->private_data;
	struct tc_ns_client_context client_context = {{0}};

	switch (cmd) {
	case TC_NS_CLIENT_IOCTL_SES_OPEN_REQ:
	case TC_NS_CLIENT_IOCTL_SES_CLOSE_REQ:
	case TC_NS_CLIENT_IOCTL_SEND_CMD_REQ:
		ret = tc_client_session_ioctl(file, cmd, arg);
		break;
	case TC_NS_CLIENT_IOCTL_LOAD_APP_REQ:
		ret = tc_ns_load_secfile(dev_file, argp);
		break;
	case TC_NS_CLIENT_IOCTL_CANCEL_CMD_REQ:
		ret = tc_ns_send_cancel_cmd(dev_file, argp, &client_context);
		break;
	case TC_NS_CLIENT_IOCTL_LOGIN:
		ret = tc_ns_client_login_func(dev_file, argp);
		break;
	case TC_NS_CLIENT_IOCTL_WAIT_EVENT:
	case TC_NS_CLIENT_IOCTL_SEND_EVENT_RESPONSE:
	case TC_NS_CLIENT_IOCTL_REGISTER_AGENT:
	case TC_NS_CLIENT_IOCTL_UNREGISTER_AGENT:
	case TC_NS_CLIENT_IOCTL_SYC_SYS_TIME:
	case TC_NS_CLIENT_IOCTL_SET_NATIVECA_IDENTITY:
	case TC_NS_CLIENT_IOCTL_LATEINIT:
		ret = tc_agent_ioctl(file, cmd, arg);
		break;
	case TC_NS_CLIENT_IOCTL_TST_CMD_REQ:
		ret = tc_ns_tst_cmd(argp);
		break;
#ifdef CONFIG_TEE_TUI
	case TC_NS_CLIENT_IOCTL_TUI_EVENT:
		ret = tc_ns_tui_event(dev_file, argp);
		break;
#endif
	case TC_NS_CLIENT_IOCTL_GET_TEE_VERSION:
		ret = tc_ns_get_tee_version(dev_file, argp);
		break;
	default:
		tloge("invalid cmd 0x%x!", cmd);
		break;
	}

	tlogd("tc client ioctl ret = 0x%x\n", ret);
	return (long)ret;
}

static int tc_client_open(struct inode *inode, struct file *file)
{
	int ret;
	struct tc_ns_dev_file *dev = NULL;

	ret = check_teecd_access();
	if (ret) {
		tloge(KERN_ERR "teecd service may be exploited 0x%x\n", ret);
		return -EACCES;
	}

	g_teecd_task = current->group_leader;
	file->private_data = NULL;
	ret = tc_ns_client_open(&dev, TEE_REQ_FROM_USER_MODE);
	if (!ret)
		file->private_data = dev;

	return ret;
}

static int teec_daemon_close(struct tc_ns_dev_file *dev)
{
	if (!dev) {
		tloge("invalid dev(null)\n");
		return -EINVAL;
	}

	del_dev_node(dev);
	kfree(dev);
	return 0;
}

static int tc_client_close(struct inode *inode, struct file *file)
{
	int ret = 0;
	struct tc_ns_dev_file *dev = file->private_data;
#ifdef CONFIG_TEE_TUI
	/* release tui resource */
	struct teec_tui_parameter tui_param = {0};

	if (dev->dev_file_id == tui_attach_device())
		tui_send_event(TUI_POLL_CANCEL, &tui_param);
#endif

	clean_agent_pid_info(dev);
	if (g_teecd_task == current->group_leader && !tc_ns_get_uid()) {
		/* for teecd close fd */
		if ((g_teecd_task->flags & PF_EXITING) ||
			(current->flags & PF_EXITING)) {
			tloge("teecd exit, something bad must happened!!!\n");
			if (is_system_agent(dev)) {
				/* for teecd agent close fd */
				send_event_response_single(dev);
				free_dev(dev);
			} else {
				/* for ca damon close fd */
				ret = teec_daemon_close(dev);
			}
		} else {
			/*
			 * for ca damon close fd when ca damon close fd
			 *  later than HIDL thread
			 */
			ret = tc_ns_client_close(dev);
		}
	} else {
		/* for CA(HIDL thread) close fd */
		ret = tc_ns_client_close(dev);
	}
	file->private_data = NULL;

	return ret;
}

struct tc_ns_dev_file *tc_find_dev_file(unsigned int dev_file_id)
{
	struct tc_ns_dev_file *dev_file = NULL;

	mutex_lock(&g_tc_ns_dev_list.dev_lock);
	list_for_each_entry(dev_file, &g_tc_ns_dev_list.dev_file_list, head) {
		if (dev_file->dev_file_id == dev_file_id) {
			mutex_unlock(&g_tc_ns_dev_list.dev_lock);
			return dev_file;
		}
	}
	mutex_unlock(&g_tc_ns_dev_list.dev_lock);
	return NULL;
}

#ifdef CONFIG_COMPAT
long tc_compat_client_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg)
{
	long ret;

	if (!file)
		return -EINVAL;

	arg = (unsigned long)(uintptr_t)compat_ptr(arg);
	ret = tc_client_ioctl(file, cmd, arg);
	return ret;
}
#endif

static const struct file_operations g_tc_ns_client_fops = {
	.owner = THIS_MODULE,
	.open = tc_client_open,
	.release = tc_client_close,
	.unlocked_ioctl = tc_client_ioctl,
	.mmap = tc_client_mmap,
#ifdef CONFIG_COMPAT
	.compat_ioctl = tc_compat_client_ioctl,
#endif
};


static int tzdriver_probe(struct platform_device *pdev)
{
	return 0;
}

struct of_device_id g_tzdriver_platform_match[] = {
	{ .compatible = "trusted_core" },
	{},
};

MODULE_DEVICE_TABLE(of, g_tzdriver_platform_match);


const struct dev_pm_ops g_tzdriver_pm_ops = {
	.freeze_noirq = tc_s4_pm_suspend,
	.restore_noirq = tc_s4_pm_resume,
};

static struct platform_driver g_tz_platform_driver = {
	.driver = {
		.name             = "trusted_core",
		.owner            = THIS_MODULE,
		.of_match_table = of_match_ptr(g_tzdriver_platform_match),
		.pm = &g_tzdriver_pm_ops,
	},
	.probe = tzdriver_probe,
};

static int create_cdev(void)
{
	int ret;

	cdev_init(&g_tc_ns_client_cdev, &g_tc_ns_client_fops);
	g_tc_ns_client_cdev.owner = THIS_MODULE;

	ret = cdev_add(&g_tc_ns_client_cdev,
		MKDEV(MAJOR(g_tc_ns_client_devt), 0), 1);
	if (ret < 0) {
		tloge("cdev_add failed %d", ret);
		return -ENOENT;
	}

	return 0;
}

static int load_hw_info(void)
{
	if (platform_driver_register(&g_tz_platform_driver)) {
		tloge("platform register driver failed\n");
		return -EFAULT;
	}

	/* load hardware info from dts and acpi */
	g_dev_node = of_find_compatible_node(NULL, NULL, "trusted_core");
	if (!g_dev_node) {
		tloge("no trusted_core compatible node found\n");
		return -ENODEV;
	}

	return 0;
}

static int tc_ns_client_init(struct device **class_dev)
{
	int ret;

	tlogd("tc_ns_client_init");
	ret = load_hw_info();
	if (ret)
		return ret;

	if (alloc_chrdev_region(&g_tc_ns_client_devt, 0, 1,
		TC_NS_CLIENT_DEV)) {
		tloge("alloc chrdev region failed");
		return -EFAULT;
	}

	g_driver_class = class_create(THIS_MODULE, TC_NS_CLIENT_DEV);
	if (IS_ERR_OR_NULL(g_driver_class)) {
		tloge("class create failed");
		ret = -ENOMEM;
		goto chrdev_region_unregister;
	}

	*class_dev = device_create(g_driver_class, NULL, g_tc_ns_client_devt,
		NULL, TC_NS_CLIENT_DEV);
	if (IS_ERR_OR_NULL(*class_dev)) {
		tloge("class device create failed");
		ret = -ENOMEM;
		goto destroy_class;
	}

	(*class_dev)->of_node = g_dev_node;
	if (create_cdev())
		goto class_device_destroy;

	INIT_LIST_HEAD(&g_tc_ns_dev_list.dev_file_list);
	mutex_init(&g_tc_ns_dev_list.dev_lock);
	init_crypto_hash_lock();
	init_srvc_list();
	return ret;

class_device_destroy:
	device_destroy(g_driver_class, g_tc_ns_client_devt);
destroy_class:
	class_destroy(g_driver_class);
chrdev_region_unregister:
	unregister_chrdev_region(g_tc_ns_client_devt, 1);
	return ret;
}

static int tc_teeos_init(struct device *class_dev)
{
	int ret;

	ret = smc_context_init(class_dev);
	if (ret)
		return ret;

	ret = mailbox_mempool_init();
	if (ret) {
		tloge("tz mailbox init failed\n");
		goto smc_data_free;
	}

	ret = tz_spi_init(class_dev, g_dev_node);
	if (ret) {
		tloge("tz spi init failed\n");
		goto release_mailbox;
	}
	return 0;
release_mailbox:
	mailbox_mempool_destroy();
smc_data_free:
	smc_free_data();
	return ret;
}

static void tc_re_init(const struct device *class_dev)
{
	int ret;

	agent_init();
	ret = tc_ns_register_ion_mem();
	if (ret)
		tloge("Failed to register ion mem in tee\n");

	if (tzdebug_init())
		tloge("tzdebug init failed\n");

	ret = init_tui(class_dev);
	if (ret)
		tloge("init_tui failed 0x%x\n", ret);

	if (init_smc_svc_thread()) {
		tloge("init svc thread\n");
		ret = -EFAULT;
	}

	if (init_dynamic_mem()) {
		tloge("init dynamic mem Failed\n");
		ret = -EFAULT;
	}

	if (ret)
		tloge("Caution! Running environment init failed!\n");
}

static __init int tc_init(void)
{
	struct device *class_dev = NULL;
	int ret = 0;

	ret = tc_ns_client_init(&class_dev);
	if (ret)
		return ret;

	ret = tc_teeos_init(class_dev);
	if (ret)
		goto class_device_destroy;
	/* run-time environment init failure don't block tzdriver init proc */
	tc_re_init(class_dev);
	return 0;

class_device_destroy:
	device_destroy(g_driver_class, g_tc_ns_client_devt);
	class_destroy(g_driver_class);
	unregister_chrdev_region(g_tc_ns_client_devt, 1);

	return ret;
}

static void tc_exit(void)
{
	tlogd("tz client exit");
	tui_exit();
	tz_spi_exit();
	/* run-time environment exit should before teeos exit */
	device_destroy(g_driver_class, g_tc_ns_client_devt);
	class_destroy(g_driver_class);
	unregister_chrdev_region(g_tc_ns_client_devt, 1);
	smc_free_data();
	agent_exit();
	mailbox_mempool_destroy();
	tee_exit_shash_handle();
}

MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_DESCRIPTION("TrustCore ns-client driver");
MODULE_VERSION("1.10");

fs_initcall_sync(tc_init);
module_exit(tc_exit);
MODULE_LICENSE("GPL");
