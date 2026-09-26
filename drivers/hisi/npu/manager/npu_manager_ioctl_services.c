/*
 * npu_manager_ioctl_services.c
 *
 * about npu manager ioctl services
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#include "npu_manager_ioctl_services.h"

#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/swap.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>

#include "npu_ioctl_services.h"
#include "npu_log.h"
#include "npu_platform.h"
#include "npu_ion.h"
#include "npu_iova.h"

static int npu_manager_ioctl_get_devnum(struct file *filep,
	unsigned int cmd, unsigned long arg)
{
	u32 devnum = 1;

	if (copy_to_user_safe((void *)(uintptr_t)arg, &devnum, sizeof(u32)))
		return -EFAULT;
	else
		return 0;
}

static int npu_manager_ioctl_get_plat_info(struct file *filep,
	unsigned int cmd, unsigned long arg)
{
	u32 plat_type;
	struct npu_platform_info *plat_info = npu_plat_get_info();

	if (plat_info == NULL) {
		npu_drv_err("npu_plat_get_info failed\r\n");
		return -EFAULT;
	}

	plat_type = (u32)plat_info->plat_type;
	if (copy_to_user_safe((void *)(uintptr_t)arg, &plat_type, sizeof(u32))) {
		npu_drv_err("cmd, cmd = %u copy plat_info to user failed\n",
			_IOC_NR(cmd));
		return -EFAULT;
	}

	return 0;
}

static int npu_manager_get_devinfo(unsigned long arg)
{
	struct npu_manager_devinfo devinfo = {0};
	struct npu_platform_info *plat_info = npu_plat_get_info();

	npu_drv_debug("start\n");

	cond_return_error(plat_info == NULL, -EFAULT,
		"npu_plat_get_info failed\n");

	cond_return_error(copy_from_user_safe(&devinfo,
		(void *)(uintptr_t)arg, sizeof(devinfo)), -EINVAL,
		"copy devinfo from user failed\n");

	// get plat
	devinfo.ts_cpu_core_num = 1;
	devinfo.ai_core_num = plat_info->spec.aicore_max;
	devinfo.ai_core_id = 0;
	devinfo.ai_cpu_core_num = plat_info->spec.aicpu_max;
	devinfo.ai_cpu_bitmap = 0x1;
	devinfo.ai_cpu_core_id = __ffs(devinfo.ai_cpu_bitmap);
	devinfo.ctrl_cpu_core_num =
		num_possible_cpus() - devinfo.ai_cpu_core_num;
	devinfo.ctrl_cpu_id = NPU_CTRL_CPU_ID;
	devinfo.ctrl_cpu_ip = 0;

	/* 1:little endian 0:big endian */
#if defined(__LITTLE_ENDIAN)
	devinfo.ctrl_cpu_endian_little = 1;
#endif

	devinfo.env_type = plat_info->env_type;

	npu_drv_debug("print npu dev info msg :"
		"devinfo.ts_cpu_core_num = %d\n"
		"devinfo.ai_core_num = %d devinfo.ai_core_id = %d\n"
		"devinfo.ai_cpu_core_num = %d devinfo.ai_cpu_bitmap = %d "
		"devinfo.ai_cpu_core_id = %d\n"
		"devinfo.ctrl_cpu_core_num = %d devinfo.ctrl_cpu_ip = %d "
		"devinfo.ctrl_cpu_id = 0x%x "
		"devinfo.ctrl_cpu_endian_little = %d\n"
		"devinfo.env_type = %d\n",
		devinfo.ts_cpu_core_num, devinfo.ai_core_num,
		devinfo.ai_core_id, devinfo.ai_cpu_core_num,
		devinfo.ai_cpu_bitmap, devinfo.ai_cpu_core_id,
		devinfo.ctrl_cpu_core_num, devinfo.ctrl_cpu_id,
		devinfo.ctrl_cpu_ip, devinfo.ctrl_cpu_endian_little,
		devinfo.env_type);

	if (copy_to_user_safe((void *)(uintptr_t)arg,
		&devinfo, sizeof(devinfo))) {
		npu_drv_err("copy devinfo to user error\n");
		return -EFAULT;
	}

	return 0;
}

int npu_get_devids(u32 *devices)
{
	u8 i;
	u8 j = 0;

	if (devices == NULL)
		return -EINVAL;

	/* get device id assigned from host,
	 * default dev_id is 0 if there is no host
	 */
	for (i = 0; i < NPU_MAX_DEVICE_NUM; i++)
		devices[j++] = i;

	if (j == 0) {
		npu_drv_err("NO dev_info!!!\n");
		return -EFAULT;
	}

	return 0;
}


static int npu_manager_get_devids(unsigned long arg)
{
	struct npu_manager_devinfo devinfo = {0};

	devinfo.num_dev = 1;
	if (npu_get_devids(devinfo.devids)) {
		npu_drv_err("npu_get_devids failed\n");
		return -EINVAL;
	}
	if (copy_to_user_safe((void *)(uintptr_t)arg,
		&devinfo, sizeof(devinfo))) {
		npu_drv_err("copy from user failed\n");
		return -EINVAL;
	}

	return 0;
}

static int npu_manager_svmva_to_devid(struct file *filep,
	unsigned int cmd, unsigned long arg)
{
	struct npu_svm_to_devid npu_svm_devid = {0};
	u32 dev_id = 0;

	npu_drv_debug("start\n");

	if (copy_from_user_safe(&npu_svm_devid, (void *)(uintptr_t)arg,
		sizeof(npu_svm_devid))) {
		npu_drv_err("copy_from_user_safe failed\n");
		return -EFAULT;
	}

	npu_svm_devid.src_devid = dev_id;
	npu_svm_devid.dest_devid = dev_id;

	if (copy_to_user_safe((void *)(uintptr_t)arg, &npu_svm_devid,
		sizeof(struct npu_svm_to_devid))) {
		npu_drv_err("copy_to_user_safe failed\n");
		return -EFAULT;
	}

	npu_drv_debug("finish\n");

	return 0;
}

static int npu_manager_check_ion(struct file *filep,
	unsigned int cmd, unsigned long arg)
{
	npu_check_ion_t check_ion;

	npu_drv_debug("start\n");

	if (copy_from_user_safe(&check_ion,
		(void *)(uintptr_t)arg, sizeof(check_ion))) {
		npu_drv_err("copy_from_user_safe failed\n");
		return -EFAULT;
	}

	npu_ioctl_check_ionfd(&check_ion);

	if (copy_to_user_safe((void *)(uintptr_t)arg,
		&check_ion, sizeof(check_ion))) {
		npu_drv_err("copy_to_user_safe failed\n");
		return -EFAULT;
	}

	npu_drv_debug("npu_manager_svmva_to_devid finish\n");

	return 0;
}

/* ION memory IOVA map */
int npu_manager_ioctl_iova_map(struct file *filep,
	unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	u64 vaddr = 0;
	struct npu_iova_ioctl_info msg = {0};

	ret = copy_from_user_safe(&msg,
		(void __user *)(uintptr_t)arg, sizeof(msg));
	cond_return_error(ret != 0, -EINVAL,
		"fail to copy iova map params, ret = %d\n", ret);

	/* map iova memory */
	ret = npu_iova_map_ion(msg.fd, msg.prot, (unsigned long *)&vaddr);
	cond_return_error(ret != 0, -EINVAL,
		"npu_iova_map_ion fail, ret = %d\n", ret);

	msg.vaddr = vaddr;
	if (copy_to_user_safe((void __user *)(uintptr_t)arg, &msg, sizeof(msg))) {
		npu_drv_err("copy to user safe error\n");
		/* unmap iova memory */
		ret = npu_iova_unmap_ion(msg.fd, vaddr);
		cond_return_error(ret != 0, -EFAULT,
			"npu_iova_unmap_ion error, ret = %d\n", ret);
	}

	return ret;
}

/* ION memory IOVA unmap */
int npu_manager_ioctl_iova_unmap(struct file *filep,
	unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct npu_iova_ioctl_info msg = {0};

	ret = copy_from_user_safe(&msg,
		(void __user *)(uintptr_t)arg, sizeof(msg));
	cond_return_error(ret != 0, -EINVAL,
		"fail to copy iova unmap params, ret = %d\n", ret);

	/* map iova memory */
	ret = npu_iova_unmap_ion(msg.fd, msg.vaddr);
	cond_return_error(ret != 0, -EINVAL,
		"npu_iova_unmap_ion fail, ret = %d\n", ret);

	return ret;
}


static int npu_manager_get_transway(struct file *filep,
	unsigned int cmd, unsigned long arg)
{
	struct npu_trans_info npu_trans_info = {0};
	int ret;

	ret = copy_from_user_safe(&npu_trans_info,
		(void *)(uintptr_t)arg, sizeof(npu_trans_info));
	if (ret) {
		npu_drv_err("copy from user failed, ret: %d\n", ret);
		return -EFAULT;
	}

	npu_trans_info.ways = DRV_SDMA;

	ret = copy_to_user_safe((void *)(uintptr_t)arg,
		&npu_trans_info, sizeof(npu_trans_info));
	if (ret) {
		npu_drv_err("copy from user failed\n");
		return -EINVAL;
	}
	return 0;
}

static int npu_manager_devinfo_ioctl(struct file *filep,
	unsigned int cmd, unsigned long arg)
{
	int ret;

	switch (cmd) {
	case NPU_MANAGER_GET_DEVIDS:
		ret = npu_manager_get_devids(arg);
		break;
	case NPU_MANAGER_GET_DEVINFO:
		ret = npu_manager_get_devinfo(arg);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}


static int (*const npu_manager_ioctl_handlers[NPU_MANAGER_CMD_MAX_NR])
	(struct file *filep, unsigned int cmd, unsigned long arg) = {
		[_IOC_NR(NPU_MANAGER_GET_DEVNUM)] = npu_manager_ioctl_get_devnum,
		[_IOC_NR(NPU_MANAGER_GET_PLATINFO)] = npu_manager_ioctl_get_plat_info,
		[_IOC_NR(NPU_MANAGER_GET_DEVIDS)] = npu_manager_devinfo_ioctl,
		[_IOC_NR(NPU_MANAGER_GET_DEVINFO)] = npu_manager_devinfo_ioctl,
		[_IOC_NR(NPU_MANAGER_SVMVA_TO_DEVID)] = npu_manager_svmva_to_devid,
		[_IOC_NR(NPU_MANAGER_CHECK_ION)] = npu_manager_check_ion,
		[_IOC_NR(NPU_MANAGER_IOVA_MAP)] = npu_manager_ioctl_iova_map,
		[_IOC_NR(NPU_MANAGER_IOVA_UNMAP)] = npu_manager_ioctl_iova_unmap,
		[_IOC_NR(NPU_MANAGER_GET_TRANSWAY)] = npu_manager_get_transway,
};

long npu_manager_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	npu_drv_debug("start IOC_NR = %d cmd = %d\n", _IOC_NR(cmd), cmd);
	if (_IOC_NR(cmd) >= NPU_MANAGER_CMD_MAX_NR || arg == 0) {
		npu_drv_err("invalid cmd, cmd = %u\n", _IOC_NR(cmd));
		return -EINVAL;
	}

	if (!npu_manager_ioctl_handlers[_IOC_NR(cmd)]) {
		npu_drv_err("invalid cmd, cmd = %u\n", _IOC_NR(cmd));
		return -EINVAL;
	}

	return npu_manager_ioctl_handlers[_IOC_NR(cmd)](filep, cmd, arg);
}

