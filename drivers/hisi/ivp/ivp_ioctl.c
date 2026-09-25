/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description:  this file implements ioctl for node hisi-ivp
 * Author:  chenweiyu
 * Create:  2019-07-30
 */
#include "ivp_ioctl.h"
#include <linux/module.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/dma-buf.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/ion.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/hisi-iommu.h>
#include <linux/syscalls.h>
#include <linux/clk-provider.h>
#include <linux/bitops.h>
#include <linux/firmware.h>
#include "securec.h"
#include "ivp_manager.h"
#include "ivp.h"
#include "ivp_log.h"
#include "ivp_reg.h"
#include "ivp_sec.h"
#include "ivp_smmu.h"

static int ivp_dev_poweron(struct ivp_device *pdev)
{
	int ret;

	mutex_lock(&pdev->ivp_wake_lock_mutex);
	if (!pdev->ivp_power_wakelock.active) {
		__pm_stay_awake(&pdev->ivp_power_wakelock);
		ivp_info("ivp power on enter, wake lock\n");
	}
	mutex_unlock(&pdev->ivp_wake_lock_mutex); /*lint !e456*/

	ret = ivp_poweron_pri(pdev);
	if (ret) {
		ivp_err("power on pri setting failed [%d]!", ret);
		goto ERR_IVP_POWERON_PRI;
	}

	/* set auto gate clk etc. */
	if (pdev->ivp_secmode == SECURE_MODE)
		ivp_dev_set_dynamic_clk(pdev, IVP_DISABLE);
	else
		ivp_dev_set_dynamic_clk(pdev, IVP_ENABLE);

	ret = ivp_poweron_remap(pdev);
	if (ret) {
		ivp_err("power on remap setting failed [%d]!", ret);
		goto ERR_IVP_POWERON_REMAP;
	}

	/* After reset, enter running mode */
	ivp_hw_set_ocdhalt_on_reset(pdev, 0);

	/* Put ivp in stall mode */
	ivp_hw_runstall(pdev, IVP_RUNSTALL_STALL);
	/* Reset ivp core */
	ivp_hw_enable_reset(pdev);

	/* Boot from IRAM. */
	ivp_hw_set_bootmode(pdev, IVP_BOOT_FROM_IRAM);

	/* Disable system reset, let ivp core leave from reset */
	ivp_hw_disable_reset(pdev);

	return ret; /*lint !e454*/

ERR_IVP_POWERON_REMAP:
	ivp_poweroff_pri(pdev);

ERR_IVP_POWERON_PRI:
	mutex_lock(&pdev->ivp_wake_lock_mutex);
	if (pdev->ivp_power_wakelock.active) {
		__pm_relax(&pdev->ivp_power_wakelock);
		ivp_err("ivp power on failed, wake unlock");
	}
	mutex_unlock(&pdev->ivp_wake_lock_mutex); /*lint !e456*/

	return ret; /*lint !e454*/
}

void ivp_dev_poweroff(struct ivp_device *pdev)
{
	int ret;

	if (!pdev) {
		ivp_err(" invalid input param pdev");
		return;
	}

	ret = ivp_poweroff_pri(pdev);
	if (ret)
		ivp_err("power on private setting failed:%d!", ret);

	mutex_lock(&pdev->ivp_wake_lock_mutex);
	if (pdev->ivp_power_wakelock.active) {
		__pm_relax(&pdev->ivp_power_wakelock);
		ivp_info("ivp power off, wake unlock\n");
	}
	mutex_unlock(&pdev->ivp_wake_lock_mutex); /*lint !e456*/
}

static int ivp_poweron(
	struct ivp_device *pdev, const struct ivp_power_up_info *pu_info)
{
	int ret = 0;

	if (atomic_read(&pdev->poweron_access) == 0) {
		ivp_err("maybe ivp dev has power on!");
		return -EBUSY;
	}
	atomic_dec(&pdev->poweron_access);
	atomic_set(&pdev->wdg_sleep, 0);
	sema_init(&pdev->wdg_sem, 0);

	if (pu_info->sec_mode == SECURE_MODE && pdev->ivp_sec_support == 0) {
		ivp_err("ivp don't support secure mode");
		goto err_ivp_invalid_param;
	}
	pdev->ivp_secmode = pu_info->sec_mode;
	pdev->ivp_sec_buff_fd = pu_info->sec_buff_fd;
	ret = ivp_dev_poweron(pdev);
	if (ret < 0) {
		ivp_err("Failed to power on ivp.");
		goto err_ivp_dev_poweron;
	}

	if (pdev->ivp_secmode == NOSEC_MODE) {
		ret = ivp_dev_smmu_init(pdev);
		if (ret) {
			ivp_err("Failed to init smmu.");
			goto err_ivp_dev_smmu_init;
		}
		ret = request_irq(pdev->dwaxi_dlock_irq, ivp_dwaxi_irq_handler,
			0, "ivp_dwaxi_irq", (void *)pdev);
		if (ret) {
			ivp_err("Failed to request dwaxi irq.%d", ret);
			goto err_request_irq_dwaxi;
		}
	}

	ret = request_irq(pdev->wdg_irq, ivp_wdg_irq_handler,
		0, "ivp_wdg_irq", (void *)pdev);
	if (ret) {
		ivp_err("Failed to request wdg irq.%d", ret);
		goto err_request_irq_wdg;
	}

	ret = ivp_init_resethandler(pdev);
	if (ret) {
		ivp_err("Failed to init reset handler.");
		goto err_ivp_init_resethandler;
	}

	ivp_info("open ivp device success!");
	atomic_dec(&pdev->poweron_success);

	return ret;

err_ivp_init_resethandler:
	free_irq(pdev->wdg_irq, pdev);
err_request_irq_wdg:
	if (pdev->ivp_secmode == NOSEC_MODE)
		free_irq(pdev->dwaxi_dlock_irq, pdev);
err_request_irq_dwaxi:
	if (pdev->ivp_secmode == NOSEC_MODE)
		ivp_dev_smmu_deinit(pdev);
err_ivp_dev_smmu_init:
	ivp_dev_poweroff(pdev);
err_ivp_dev_poweron:
	pdev->ivp_secmode = NOSEC_MODE;
err_ivp_invalid_param:
	atomic_inc(&pdev->poweron_access);
	ivp_dsm_error_notify(DSM_IVP_OPEN_ERROR_NO);

	ivp_info("poweron ivp device fail!");
	return ret;
}

static long ioctl_power_up(struct file *fd, unsigned long args)
{
	long ret = 0;
	struct ivp_power_up_info info = {0};
	struct ivp_device *pdev = NULL;
	if (!fd) {
		ivp_err("invalid input fd");
		return -EINVAL;
	}
	pdev = (struct ivp_device *)fd->private_data;

	if (!args || !pdev) {
		ivp_err("invalid input args or pdev");
		return -EINVAL;
	}

	mutex_lock(&pdev->ivp_power_up_off_mutex);
	if (copy_from_user(&info, (void *)(uintptr_t)args, sizeof(info)) != 0) {
		ivp_err("invalid input param size.");
		goto power_up_err;
	}

	if (info.sec_mode != SECURE_MODE && info.sec_mode != NOSEC_MODE) {
		ivp_err("invalid input secMode value:%d", info.sec_mode);
		goto power_up_err;
	}
	if (info.sec_mode == SECURE_MODE && info.sec_buff_fd < 0) {
		ivp_err("invalid sec buffer fd value:%d", info.sec_buff_fd);
		goto power_up_err;
	}

	ret = ivp_poweron(pdev, &info);
	mutex_unlock(&pdev->ivp_power_up_off_mutex);

	return ret;

power_up_err:
	mutex_unlock(&pdev->ivp_power_up_off_mutex);
	return -EINVAL;
}

static long ioctl_sect_count(struct file *fd, unsigned long args)
{
	long ret;
	struct ivp_device *pdev = NULL;
	unsigned int sect_count;
	if (!fd) {
		ivp_err("invalid input fd");
		return -EINVAL;
	}
	pdev = fd->private_data;
	if (!args || !pdev) {
		ivp_err("invalid input args or odev");
		return -EINVAL;
	}
	sect_count = pdev->sect_count;

	ivp_info("get img sect num:%#x.", sect_count);
	ret = copy_to_user((void *)(uintptr_t)args,
		&sect_count, sizeof(sect_count));

	return ret;
}

static long ioctl_sect_info(struct file *fd, unsigned long args)
{
	long ret;
	struct ivp_sect_info info;
	struct ivp_device *pdev = NULL;
	unsigned int sect_count;
	if (!fd) {
		ivp_err("invalid input fd");
		return -EINVAL;
	}
	pdev = fd->private_data;
	if (!args || !pdev) {
		ivp_err("invalid input args or pdev");
		return -EINVAL;
	}

	sect_count = pdev->sect_count;
	if (copy_from_user(&info, (void *)(uintptr_t)args, sizeof(info)) != 0) {
		ivp_err("invalid input param size.");
		return -EINVAL;
	}

	if (info.index >= sect_count) {
		ivp_err("index is out of range.index:%u, sec_count:%u",
			info.index, sect_count);
		return -EINVAL;
	}

	if (pdev->ivp_secmode == SECURE_MODE) {
		ret = copy_to_user((void *)(uintptr_t)args,
			&pdev->sec_sects[info.index],
			sizeof(struct ivp_sect_info));
		ivp_dbg("name:%s, ivp_addr:0x%x, acpu_addr:0x%lx ",
			pdev->sec_sects[info.index].name,
			pdev->sec_sects[info.index].ivp_addr,
			pdev->sec_sects[info.index].acpu_addr);
	} else {
		ret = copy_to_user((void *)(uintptr_t)args,
			&pdev->sects[info.index],
			sizeof(struct ivp_sect_info));
	}

	return ret;
}

static int ivp_load_section(struct ivp_device *pdev,
	struct image_section_header image_sect, const struct firmware *fw)
{
	unsigned int iova = 0;
	unsigned int offset = 0;
	unsigned int size;
	unsigned int i;
	unsigned int *source = NULL;
	unsigned char type = 0;
	unsigned long ivp_ddr_addr = 0;
	unsigned int *mem_addr = NULL;
	void *mem = NULL;
	bool ddr_flag = false;
	errno_t ret;

	iova = image_sect.vaddr;
	size = image_sect.size;

	source = (unsigned int *)(fw->data + image_sect.offset);
	type = image_sect.type;
	if (image_sect.vaddr >= pdev->sects[SECT_START_NUM].ivp_addr &&
		(image_sect.vaddr <= (pdev->sects[SECT_START_NUM].ivp_addr +
			pdev->sects[SECT_START_NUM].len))) {
		ddr_flag = true;
	} else {
		ddr_flag = false;
	}
	switch (type) {
	case IMAGE_SECTION_TYPE_EXEC:
	case IMAGE_SECTION_TYPE_DATA: {
		if (ddr_flag == true) {
			ivp_ddr_addr = (pdev->sects[SECT_START_NUM].acpu_addr << IVP_MMAP_SHIFT) +
				iova - pdev->sects[SECT_START_NUM].ivp_addr;
			mem = ivp_vmap(ivp_ddr_addr, image_sect.size, &offset);
		} else {
			mem = ioremap_nocache(image_sect.vaddr,
					image_sect.size);
		}

		if (!mem) {
			ivp_err("Can't map base address");
			return -EINVAL;
		}
		mem_addr = (unsigned int *)mem;

		if (ddr_flag == true) {
			ret = memcpy_s(mem_addr, image_sect.size,
					source, image_sect.size);
			if (ret != EOK) {
				ivp_err("memcpy_s fail, ret [%d]", ret);
				vunmap(mem - offset);
				return -EINVAL;
			}
		} else {
			for (i = 0; i < image_sect.size / 4; i++)
				*(mem_addr + i) = *(source + i);
		}
	}
	break;
	case IMAGE_SECTION_TYPE_BSS:
	break;
	default: {
		ivp_err("Unsupported section type %d", type);
		return -EINVAL;
	}
	}
	if (mem) {
		if (ddr_flag == true)
			vunmap(mem - offset);
		else
			iounmap(mem);
	}
	return 0;
}

static int ivp_get_validate_section_info(struct ivp_device *pdev,
		const struct firmware *fw,
		struct image_section_header *psect_header,
		unsigned int index)
{
	int offset;
	errno_t ret;

	if (!psect_header) {
		ivp_err("input para is invalid");
		return -EINVAL;
	}

	offset = sizeof(struct file_header);
	offset += sizeof(struct image_section_header) * index;
	if ((offset + sizeof(struct image_section_header)) > fw->size) {
		ivp_err("image index is err");
		return -EINVAL;
	}
	ret = memcpy_s(psect_header, sizeof(struct image_section_header),
		fw->data + offset, sizeof(struct image_section_header));
	if (ret != EOK) {
		ivp_err("memcpy_s fail, ret [%d]", ret);
		return -EINVAL;
	}

	if ((psect_header->offset + psect_header->size) > fw->size) {
		ivp_err("get invalid offset 0x%x", psect_header->offset);
		return -EINVAL;
	}
	/* 0, 1,2,3 represent array index*/
	if ((psect_header->vaddr >= pdev->sects[0].ivp_addr &&
		(psect_header->vaddr < (pdev->sects[0].ivp_addr + pdev->sects[0].len))) ||
		(psect_header->vaddr >= pdev->sects[1].ivp_addr &&
		(psect_header->vaddr < (pdev->sects[1].ivp_addr + pdev->sects[1].len))) ||
		(psect_header->vaddr >= pdev->sects[2].ivp_addr &&
		(psect_header->vaddr < (pdev->sects[2].ivp_addr + pdev->sects[2].len))) ||
		(psect_header->vaddr >= pdev->sects[3].ivp_addr &&
		(psect_header->vaddr < (pdev->sects[3].ivp_addr + pdev->sects[3].len)))) {
		return EOK;
	}

	ivp_err("get invalid addr");
	return -EINVAL;
}
static int ivp_load_firmware(struct ivp_device *pdev,
		const char *filename, const struct firmware *fw)
{
	int idx;
	struct file_header mheader;
	errno_t ret;

	ret = memcpy_s(&mheader, sizeof(mheader), fw->data, sizeof(mheader));
	if (ret != EOK) {
		ivp_err("memcpy_s fail, ret [%d]", ret);
		return -EINVAL;
	}
	ivp_info("start loading image %s, section counts 0x%x...",
		filename, mheader.sect_count);
	for (idx = 0; idx < mheader.sect_count; idx++) {
		struct image_section_header sect;

		if (ivp_get_validate_section_info(pdev, fw, &sect, idx)) {
			ivp_err("get section %d fails ...", idx);
			return -EINVAL;
		}
		if (ivp_load_section(pdev, sect, fw)) {
			ivp_err("load section %d fails ...", idx);
			return -EINVAL;
		}
	}
	ivp_info("finish loading image %s", filename);
	return 0;
}
static int ivp_check_image(const struct firmware *fw)
{
	errno_t ret;
	struct file_header mheader;

	if (sizeof(mheader) > fw->size) {
		ivp_err("image file mheader is err");
		return -EINVAL;
	}
	ret = memcpy_s(&mheader, sizeof(mheader), fw->data, sizeof(mheader));
	if (ret != EOK) {
		ivp_err("memcpy_s fail, ret [%d]", ret);
		return -EINVAL;
	}

	if (strncmp(mheader.name, "IVP:", 4) != 0) {
		ivp_err("image file header is not for IVP");
		return -EINVAL;
	}

	if (fw->size != mheader.image_size) {
		ivp_err("request_firmware size 0x%zx mheader size 0x%zx",
			fw->size, mheader.image_size);
		return -EINVAL;
	}
	return 0;
}
static int ivp_load_image(struct ivp_device *pdev, const char *name)
{
	int ret;
	const struct firmware *firmware = NULL;
	struct device *dev = NULL;

	if (!name) {
		ivp_err("ivp image file is invalid");
		return -EINVAL;
	}

	dev = pdev->device.this_device;
	if (!dev) {
		ivp_err("ivp miscdevice is invalid");
		return -EINVAL;
	}
	ret = request_firmware(&firmware, name, dev);
	if (ret) {
		ivp_err("return error:%d for file:%s", ret, name);
		return ret;
	}

	ret = ivp_check_image(firmware);
	if (ret != 0) {
		release_firmware(firmware);
		ivp_err("check ivp image %s fail value 0x%x ", name, ret);
		return ret;
	}

	ret = ivp_load_firmware(pdev, name, firmware);
	release_firmware(firmware);
	return ret;
}
static long ioctl_load_firmware(struct file *fd, unsigned long args)
{
	long ret;
	struct ivp_image_info info;
	errno_t rc;
	struct ivp_device *pdev = fd->private_data;

	if (!args || !pdev) {
		ivp_err("invalid input args or pdev");
		return -EINVAL;
	}

	if (ivp_hw_query_runstall(pdev) == IVP_RUNSTALL_RUN) {
		ivp_err("invalid ivp status:ivp alredy run");
		return -EINVAL;
	}
	mutex_lock(&pdev->ivp_load_image_mutex);
	rc = memset_s((char *)&info, sizeof(info), 0, sizeof(info));
	if (rc != EOK) {
		ivp_err("memcpy_s fail, rc:%d", rc);
		mutex_unlock(&pdev->ivp_load_image_mutex);
		return -EINVAL;
	}
	if (copy_from_user(&info, (void *)(uintptr_t)args, sizeof(info)) != 0) {
		ivp_err("invalid input param size.");
		mutex_unlock(&pdev->ivp_load_image_mutex);
		return -EINVAL;
	}
	info.name[sizeof(info.name) - 1] = '\0';
	if ((info.length > (sizeof(info.name) - 1)) ||
		info.length <= IVP_IMAGE_SUFFIX_LENGTH ||
		info.length != strlen(info.name)) {
		ivp_err("image file:but pass param length:%d", info.length);
		mutex_unlock(&pdev->ivp_load_image_mutex);
		return -EINVAL;
	}
	if (strcmp((const char *)&info.name[info.length - IVP_IMAGE_SUFFIX_LENGTH],
		IVP_IMAGE_SUFFIX)) {
		ivp_err("image is not bin file");
		mutex_unlock(&pdev->ivp_load_image_mutex);
		return -EINVAL;
	}
	if (pdev->ivp_secmode == SECURE_MODE)
		ret = ivp_sec_loadimage(pdev);
	else
		ret = ivp_load_image(pdev, info.name);

	mutex_unlock(&pdev->ivp_load_image_mutex);
	return ret;
}

static void ivp_dev_run(struct ivp_device *pdev)
{
	int i = 0;

	if (ivp_hw_query_runstall(pdev) == IVP_RUNSTALL_RUN)
		return;

	ivp_hw_runstall(pdev, IVP_RUNSTALL_RUN);

	while ((ivp_reg_read(pdev, IVP_REG_OFF_RESEVER_REG) != IVP_INIT_SUCCESS) &&
			(i < IVP_INIT_RESULT_CHECK_TIMES)) {
		udelay(IVP_STATUS_CHECK_DELAY_US);
		i++;
		if (i >= IVP_INIT_RESULT_CHECK_TIMES) {
			ivp_err("ivp init fails value:0x%x",
					ivp_reg_read(pdev, IVP_REG_OFF_RESEVER_REG));
			return;
		}
	}

	ivp_info("ivp init success:0x%x cycle:%d!",
			ivp_reg_read(pdev, IVP_REG_OFF_RESEVER_REG), i);
}

static long ioctl_dsp_run(struct file *fd,
		unsigned long args __attribute__((unused)))
{
	struct ivp_device *pdev = fd->private_data;

	if (!pdev) {
		ivp_err("invalid param pdev");
		return -EINVAL;
	}
	if (ivp_check_resethandler(pdev) == 1) {
		ivp_dev_run(pdev);
	} else {
		ivp_err("ivp image not upload.");
		return -EPERM;
	}

	return EOK;
}

static long ioctl_dsp_suspend(struct file *fd,
		unsigned long args __attribute__((unused)))
{
	long ret;
	struct ivp_device *pdev = fd->private_data;

	if (!pdev) {
		ivp_err("invalid param pdev");
		return -EINVAL;
	}

	ret = ivp_dev_suspend(pdev);

	return ret;
}

static long ioctl_dsp_resume(struct file *fd,
		unsigned long args __attribute__((unused)))
{
	struct ivp_device *pdev = fd->private_data;

	if (!pdev) {
		ivp_err("invalid param pdev");
		return -EINVAL;
	}

	if (ivp_check_resethandler(pdev) == 1) {
		ivp_dev_resume(pdev);
	} else {
		ivp_err("ivp image not upload.");
		return -EPERM;
	}

	return EOK;
}

static long ioctl_dsp_stop(struct file *fd,
		unsigned long args __attribute__((unused)))
{
	struct ivp_device *pdev = fd->private_data;

	if (!pdev) {
		ivp_err("invalid param pdev");
		return -EINVAL;
	}

	ivp_dev_stop(pdev);

	return EOK;
}

static long ioctl_query_runstall(struct file *fd, unsigned long args)
{
	long ret;
	unsigned int runstall = 0;
	struct ivp_device *pdev = fd->private_data;

	if (!args || !pdev) {
		ivp_err("invalid input args or pdev");
		return -EINVAL;
	}

	runstall = (u32)ivp_hw_query_runstall(pdev);
	ret = copy_to_user((void *)(uintptr_t)args,
			&runstall, sizeof(runstall));

	return ret;
}

static long ioctl_query_waiti(struct file *fd, unsigned long args)
{
	long ret;
	unsigned int waiti;
	struct ivp_device *pdev = fd->private_data;

	if (!args || !pdev) {
		ivp_err("invalid input args or pdev");
		return -EINVAL;
	}

	waiti = (u32)ivp_hw_query_waitmode(pdev);
	ret = copy_to_user((void *)(uintptr_t)args, &waiti, sizeof(waiti));

	return ret;
}

static long ioctl_trigger_nmi(struct file *fd,
		unsigned long args __attribute__((unused)))
{
	struct ivp_device *pdev = fd->private_data;

	if (!pdev) {
		ivp_err("invalid param pdev");
		return -EINVAL;
	}

	ivp_hw_trigger_NMI(pdev);

	return EOK;
}

static long ioctl_watchdog(struct file *fd,
		unsigned long args __attribute__((unused)))
{
	long ret;
	struct ivp_device *pdev = fd->private_data;

	if (!pdev) {
		ivp_err("invalid param pdev");
		return -EINVAL;
	}

	ret = ivp_dev_keep_on_wdg(pdev);

	return ret;
}

static long ioctl_watchdog_sleep(struct file *fd,
		unsigned long args __attribute__((unused)))
{
	struct ivp_device *pdev = fd->private_data;

	if (!pdev) {
		ivp_err("invalid param pdev");
		return -EINVAL;
	}

	ivp_dev_sleep_wdg(pdev);

	return EOK;
}

static long ioctl_smmu_invalidate_tlb(struct file *fd,
		unsigned long args __attribute__((unused)))
{
#ifdef CONFIG_IVP_SMMU
	struct ivp_device *pdev = fd->private_data;
	long ret = 0;

	if (!pdev) {
		ivp_err("invalid param pdev");
		return -EINVAL;
	}

	ret = ivp_dev_smmu_invalid_tlb(pdev);

	return ret;
#else
	return 0;
#endif
}

static long ioctl_bm_init(struct file *fd,
		unsigned long args __attribute__((unused)))
{
	struct ivp_device *pdev = fd->private_data;

	if (!pdev) {
		ivp_err("invalid param pdev");
		return -EINVAL;
	}

	ivp_dev_hwa_enable(pdev);
	return EOK;
}

static long ioctl_clk_level(struct file *fd, unsigned long args)
{
	unsigned int level = 0;
	struct ivp_device *pdev = fd->private_data;

	if (!args || !pdev) {
		ivp_err("invalid input args or pdev");
		return -EINVAL;
	}

	if (copy_from_user(&level, (void *)(uintptr_t)args,
				sizeof(unsigned int)) != 0) {
		ivp_err("invalid input param size.");
		return -EINVAL;
	}

	ivp_change_clk(pdev, level);
	return EOK;
}

static long ioctl_dump_dsp_status(struct file *fd,
		unsigned long args __attribute__((unused)))
{
	struct ivp_device *pdev = fd->private_data;

	if (!pdev) {
		ivp_err("invalid param pdev");
		return -EINVAL;
	}

	ivp_dump_status(pdev);

	return EOK;
}

typedef long (*ivp_ioctl_func_t)(struct file *fd, unsigned long args);

struct ivp_ioctl_ops {
	unsigned int cmd;
	ivp_ioctl_func_t func;
};

#define IVP_IOCTL_OPS(_cmd, _func) {.cmd = _cmd, .func = _func}


static struct ivp_ioctl_ops ioctl_ops[] = {
	IVP_IOCTL_OPS(IVP_IOCTL_POWER_UP, ioctl_power_up),
	IVP_IOCTL_OPS(IVP_IOCTL_SECTINFO, ioctl_sect_info),
	IVP_IOCTL_OPS(IVP_IOCTL_SECTCOUNT, ioctl_sect_count),
	IVP_IOCTL_OPS(IVP_IOCTL_LOAD_FIRMWARE, ioctl_load_firmware),
	IVP_IOCTL_OPS(IVP_IOCTL_DSP_RUN, ioctl_dsp_run),
	IVP_IOCTL_OPS(IVP_IOCTL_DSP_SUSPEND, ioctl_dsp_suspend),
	IVP_IOCTL_OPS(IVP_IOCTL_DSP_RESUME, ioctl_dsp_resume),
	IVP_IOCTL_OPS(IVP_IOCTL_DSP_STOP, ioctl_dsp_stop),
	IVP_IOCTL_OPS(IVP_IOCTL_QUERY_RUNSTALL, ioctl_query_runstall),
	IVP_IOCTL_OPS(IVP_IOCTL_QUERY_WAITI, ioctl_query_waiti),
	IVP_IOCTL_OPS(IVP_IOCTL_TRIGGER_NMI, ioctl_trigger_nmi),
	IVP_IOCTL_OPS(IVP_IOCTL_WATCHDOG, ioctl_watchdog),
	IVP_IOCTL_OPS(IVP_IOCTL_WATCHDOG_SLEEP, ioctl_watchdog_sleep),
	IVP_IOCTL_OPS(IVP_IOCTL_BM_INIT, ioctl_bm_init),
	IVP_IOCTL_OPS(IVP_IOCTL_CLK_LEVEL, ioctl_clk_level),
	IVP_IOCTL_OPS(IVP_IOCTL_DUMP_DSP_STATUS, ioctl_dump_dsp_status),
	IVP_IOCTL_OPS(IVP_IOCTL_SMMU_INVALIDATE_TLB, ioctl_smmu_invalidate_tlb),
};

static long ioctl_cmd_check(struct file *fd, unsigned int cmd)
{
	struct ivp_device *pdev = fd->private_data;

	if (!pdev) {
		ivp_err("invalid param pdev");
		return -EINVAL;
	}

	if (atomic_read(&pdev->poweron_success) != 0
			&& cmd != IVP_IOCTL_POWER_UP) {
		ivp_err("ioctl cmd is error %d since ivp not power", cmd);
		return -EINVAL;
	}

	return EOK;
}

long ivp_ioctl(struct file *fd, unsigned int cmd, unsigned long args)
{
	uint32_t idx;
	ivp_ioctl_func_t func = NULL;

	if (!fd) {
		ivp_err("invalid input param fd ");
		return -EINVAL;
	}

	if (ioctl_cmd_check(fd, cmd) != EOK)
		return -EINVAL;

	for (idx = 0; idx < sizeof(ioctl_ops) / sizeof(ioctl_ops[0]); idx++) {
		if (cmd == ioctl_ops[idx].cmd) {
			if (ioctl_ops[idx].func == NULL) {
				ivp_err("ivp ioctl: invalid func\n");
				return -EINVAL;
			}
			func = ioctl_ops[idx].func;
			return func(fd, args);
		}
	}

	ivp_err("invalid ioctl command(0x%08x) received!", cmd);
	return -EINVAL;
}
