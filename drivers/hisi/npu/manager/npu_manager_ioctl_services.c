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

static int devdrv_manager_ioctl_get_devnum(struct file *filep, unsigned int cmd, unsigned long arg)
{
	u32 devnum = 1;

	if (copy_to_user_safe((void *)(uintptr_t)arg, &devnum, sizeof(u32))) {
		return -EFAULT;
	} else {
		return 0;
	}
}

static int devdrv_manager_ioctl_get_plat_info(struct file *filep, unsigned int cmd, unsigned long arg)
{
	u32 plat_type;
	struct devdrv_platform_info *plat_info = devdrv_plat_get_info();

	if (plat_info == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_info failed\r\n");
		return -EFAULT;
	}

	plat_type = (u32)DEVDRV_PLAT_GET_TYPE(plat_info);
	if (copy_to_user_safe((void *)(uintptr_t)arg, &plat_type, sizeof(u32))) {
		NPU_DRV_ERR("cmd, cmd = %u copy plat_info to user failed\n", _IOC_NR(cmd));
		return -EFAULT;
	}

	return 0;
}

static int devdrv_manager_get_devinfo(unsigned long arg)
{
	struct devdrv_manager_hccl_devinfo hccl_devinfo = {0};
	struct devdrv_platform_info *plat_info = devdrv_plat_get_info();

	NPU_DRV_DEBUG("devdrv_manager_get_devinfo start\n");

	COND_RETURN_ERROR(plat_info == NULL, -EFAULT, "devdrv_plat_get_info failed\r\n");

	COND_RETURN_ERROR(copy_from_user_safe(&hccl_devinfo, (void *)(uintptr_t)arg, sizeof(hccl_devinfo)),
		-EINVAL, "copy hccl_devinfo from user failed\n");

	// get plat
	hccl_devinfo.ts_cpu_core_num = 1;
	hccl_devinfo.ai_core_num = DEVDRV_PLAT_GET_AICORE_MAX(plat_info);
	hccl_devinfo.ai_core_id = 0;
	hccl_devinfo.ai_cpu_core_num = DEVDRV_PLAT_GET_AICPU_MAX(plat_info);
	hccl_devinfo.ai_cpu_bitmap = 0x1;
	hccl_devinfo.ai_cpu_core_id = __ffs(hccl_devinfo.ai_cpu_bitmap);
	hccl_devinfo.ctrl_cpu_core_num = num_possible_cpus() - hccl_devinfo.ai_cpu_core_num;
	hccl_devinfo.ctrl_cpu_id = DEVDRV_CTRL_CPU_ID;
	hccl_devinfo.ctrl_cpu_ip = 0;

	/* 1:little endian 0:big endian */
#if defined(__LITTLE_ENDIAN)
	hccl_devinfo.ctrl_cpu_endian_little = 1;
#endif

	hccl_devinfo.env_type = DEVDRV_PLAT_GET_ENV(plat_info);
	hccl_devinfo.hardware_version = DEVDRV_PLAT_GET_HARDWARE(plat_info);

	NPU_DRV_DEBUG("print npu dev info msg :hccl_devinfo.ts_cpu_core_num = %d \n"
		"hccl_devinfo.ai_core_num = %d hccl_devinfo.ai_core_id = %d \n"
		"hccl_devinfo.ai_cpu_core_num = %d hccl_devinfo.ai_cpu_bitmap = %d "
		"hccl_devinfo.ai_cpu_core_id = %d \n"
		"hccl_devinfo.ctrl_cpu_core_num = %d hccl_devinfo.ctrl_cpu_ip = %d "
		"hccl_devinfo.ctrl_cpu_id = 0x%x hccl_devinfo.ctrl_cpu_endian_little = %d \n"
		"hccl_devinfo.env_type = %d hccl_devinfo.hardware_version = 0x%x \n",
		hccl_devinfo.ts_cpu_core_num, hccl_devinfo.ai_core_num,
		hccl_devinfo.ai_core_id, hccl_devinfo.ai_cpu_core_num,
		hccl_devinfo.ai_cpu_bitmap, hccl_devinfo.ai_cpu_core_id,
		hccl_devinfo.ctrl_cpu_core_num, hccl_devinfo.ctrl_cpu_id,
		hccl_devinfo.ctrl_cpu_ip, hccl_devinfo.ctrl_cpu_endian_little,
		hccl_devinfo.env_type, hccl_devinfo.hardware_version);

	if (copy_to_user_safe((void *)(uintptr_t)arg, &hccl_devinfo, sizeof(hccl_devinfo))) {
		NPU_DRV_ERR("copy hccl_devinfo to user error\n");
		return -EFAULT;
	}

	return 0;
}

int devdrv_get_devids(u32 *devices)
{
	u8 i;
	u8 j = 0;

	if (devices == NULL)
		return -EINVAL;

	/* get device id assigned from host, default dev_id is 0 if there is no host */
	for (i = 0; i < DEVDRV_MAX_DAVINCI_NUM; i++)
		devices[j++] = i;

	if (j == 0) {
		NPU_DRV_ERR("NO dev_info!!!\n");
		return -EFAULT;
	}

	return 0;
}


static int devdrv_manager_get_devids(unsigned long arg)
{
	struct devdrv_manager_hccl_devinfo hccl_devinfo = {0};

	hccl_devinfo.num_dev = 1;
	if (devdrv_get_devids(hccl_devinfo.devids)) {
		NPU_DRV_ERR("devdrv_get_devids failed\n");
		return -EINVAL;
	}
	if (copy_to_user_safe((void *)(uintptr_t)arg, &hccl_devinfo, sizeof(hccl_devinfo))) {
		NPU_DRV_ERR("copy from user failed\n");
		return -EINVAL;
	}

	return 0;
}

static int devdrv_manager_svmva_to_devid(struct file *filep, unsigned int cmd, unsigned long arg)
{
	struct devdrv_svm_to_devid devdrv_svm_devid = {0};
	u32 dev_id = 0;

	NPU_DRV_DEBUG("devdrv_manager_svmva_to_devid start\n");

	if (copy_from_user_safe(&devdrv_svm_devid, (void *)(uintptr_t)arg, sizeof(devdrv_svm_devid))) {
		NPU_DRV_ERR("copy_from_user_safe failed\n");
		return -EFAULT;
	}

	devdrv_svm_devid.src_devid = dev_id;
	devdrv_svm_devid.dest_devid = dev_id;

	if (copy_to_user_safe((void *)(uintptr_t)arg, &devdrv_svm_devid, sizeof(struct devdrv_svm_to_devid))) {
		NPU_DRV_ERR("copy_to_user_safe failed\n");
		return -EFAULT;
	}

	NPU_DRV_DEBUG("devdrv_manager_svmva_to_devid finish\n");

	return 0;
}

static int devdrv_manager_get_transway(struct file *filep, unsigned int cmd, unsigned long arg)
{
	struct devdrv_trans_info devdrv_trans_info = {0};
	int ret;

	ret = copy_from_user_safe(&devdrv_trans_info, (void *)(uintptr_t)arg, sizeof(devdrv_trans_info));
	if (ret) {
		NPU_DRV_ERR("copy from user failed, ret: %d\n", ret);
		return -EFAULT;
	}

	devdrv_trans_info.ways = DRV_SDMA;

	ret = copy_to_user_safe((void *)(uintptr_t)arg, &devdrv_trans_info, sizeof(devdrv_trans_info));
	if (ret) {
		NPU_DRV_ERR("copy from user failed\n");
		return -EINVAL;
	}
	return 0;
}

static int devdrv_manager_devinfo_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	int ret;

	switch (cmd) {
	case DEVDRV_MANAGER_GET_DEVIDS:
		ret = devdrv_manager_get_devids(arg);
		break;
	case DEVDRV_MANAGER_GET_DEVINFO:
		ret = devdrv_manager_get_devinfo(arg);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}


static int (*const devdrv_manager_ioctl_handlers[DEVDRV_MANAGER_CMD_MAX_NR])
	(struct file *filep, unsigned int cmd, unsigned long arg) = {
		[_IOC_NR(DEVDRV_MANAGER_GET_DEVNUM)] = devdrv_manager_ioctl_get_devnum,
		[_IOC_NR(DEVDRV_MANAGER_GET_PLATINFO)] = devdrv_manager_ioctl_get_plat_info,
		[_IOC_NR(DEVDRV_MANAGER_GET_DEVIDS)] = devdrv_manager_devinfo_ioctl,
		[_IOC_NR(DEVDRV_MANAGER_GET_DEVINFO)] = devdrv_manager_devinfo_ioctl,
		[_IOC_NR(DEVDRV_MANAGER_SVMVA_TO_DEVID)] = devdrv_manager_svmva_to_devid,
		[_IOC_NR(DEVDRV_MANAGER_GET_TRANSWAY)] = devdrv_manager_get_transway,
};

long devdrv_manager_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	NPU_DRV_DEBUG("devdrv_manager_ioctl start IOC_NR = %d cmd = %d \n", _IOC_NR(cmd), cmd);
	if (_IOC_NR(cmd) >= DEVDRV_MANAGER_CMD_MAX_NR || arg == 0) {
		NPU_DRV_ERR("invalid cmd, cmd = %u\n", _IOC_NR(cmd));
		return -EINVAL;
	}

	if (!devdrv_manager_ioctl_handlers[_IOC_NR(cmd)]) {
		NPU_DRV_ERR("invalid cmd, cmd = %u\n", _IOC_NR(cmd));
		return -EINVAL;
	}

	return devdrv_manager_ioctl_handlers[_IOC_NR(cmd)](filep, cmd, arg);
}

