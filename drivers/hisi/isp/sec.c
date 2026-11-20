/*
 *  ISP driver, isp_atfisp.c
 *
 * Copyright (c) 2013 ISP Technologies CO., Ltd.
 *
 */

#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/regulator/consumer.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <linux/fcntl.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/scatterlist.h>
#include <linux/printk.h>
#include <linux/file.h>
#include <linux/kthread.h>
#include <linux/rproc_share.h>
#include <linux/remoteproc.h>
#include <linux/ion.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/genalloc.h>
#include <linux/hisi-iommu.h>
#include <linux/mutex.h>
#include <linux/iommu.h>
#include <linux/compiler.h>
#include <linux/cpumask.h>
#include <linux/uaccess.h>
#include <asm/compiler.h>
#include <linux/hisi/partition_ap_kernel.h>
#include <global_ddr_map.h>
#include "teek_client_id.h"
#include <linux/platform_data/remoteproc_hisp.h>
#include <linux/platform_data/hisp_mempool.h>
#include "partition_macro.h"
#include <dsm/dsm_pub.h>
#include <isp_ddr_map.h>
#include <linux/list.h>
#include <securec.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include "hisp_internel.h"

#define SEC_MAX_SIZE    64
#define DEVICE_PATH  "/dev/block/bootdevice/by-name/"

struct hisp_sec sec_rproc_dev;


struct map_sglist_s {
	unsigned long long addr;
	unsigned int size;
};

struct phy_csi_info_t {
	struct hisp_phy_info_t phy_info;
	unsigned int csi_idx;
};

enum csi_num_e {
	HISP_CSI_A = 0,
	HISP_CSI_B,
	HISP_CSI_C,
	HISP_CSI_NUM_MAX,
};

unsigned int map_type_info[MAP_TYPE_MAX];

void *getsec_a7sharedmem_addr(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;

	return dev->atfshrd_vaddr + dev->tsmem_offset[ISP_SHARE_MEM_OFFSET];
}

void *hisp_get_dyna_array(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;

	return dev->ap_dyna_array;
}

struct isp_a7mapping_s *hisp_get_ap_dyna_mapping(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;

	return dev->ap_dyna;
}

int hisp_sec_jpeg_powerdn(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	int ret;

	mutex_lock(&dev->pwrlock);
	ret = hisp_pwr_core_sec_exit();
	if (ret < 0)
		pr_err("[%s] Failed : hisp_pwr_core_sec_exit.%d\n",
			__func__, ret);

	ret = hisp_subsys_powerdn();
	if (ret < 0)
		pr_err("[%s] Failed : hisp_subsys_powerdn.%d\n",
			__func__, ret);
	mutex_unlock(&dev->pwrlock);

	return 0;
}

int hisp_sec_jpeg_powerup(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	u64 pgd_base;
	int ret = 0;

	mutex_lock(&dev->pwrlock);
	ret = hisp_subsys_powerup();
	if (ret < 0) {
		pr_err("[%s] Failed : hisp_subsys_powerup.%d\n",
			__func__, ret);
		mutex_unlock(&dev->pwrlock);
		return ret;
	}

	pgd_base = hisp_get_pgd_base();
	ret = hisp_pwr_core_sec_init(pgd_base);
	if (ret < 0) {
		pr_err("[%s] Failed : hisp_pwr_core_sec_init.%d\n",
			__func__, ret);
		goto err_jpegup;
	}
	mutex_unlock(&dev->pwrlock);

	return 0;

err_jpegup:
	if (hisp_subsys_powerdn() < 0)
		pr_err("[%s] Failed : err_jpegup hisp_subsys_powerdn\n",
			__func__);

	mutex_unlock(&dev->pwrlock);

	return ret;
}

int hisp_bsp_read_bin(const char *partion_name, unsigned int offset,
		unsigned int length, char *buffer)
{
	int ret = -1;
	char *pathname = NULL;
	unsigned long pathlen;
	struct file *fp = NULL;
	mm_segment_t fs;
	loff_t pos = 0;

	if ((partion_name == NULL) || (buffer == NULL))
		return -1;

	/*get resource*/
	pathlen = sizeof(DEVICE_PATH) +
		strnlen(partion_name, (unsigned long)PART_NAMELEN);
	pathname = kmalloc(pathlen, GFP_KERNEL);
	if (pathname == NULL)
		return -1;

	ret = flash_find_ptn_s((const char *)partion_name, pathname, pathlen);
	if (ret < 0) {
		pr_err("partion_name.%s not in partion table!\n", partion_name);
		goto error;
	}

	fp = filp_open(pathname, O_RDONLY, 0600);
	if (IS_ERR(fp)) {
		pr_err("filp_open(%s) failed", pathname);
		ret = -1;
		goto error;
	}

	ret = vfs_llseek(fp, offset, SEEK_SET);
	if (ret < 0) {
		pr_err("seek ops failed, ret %d", ret);
		goto err_close;
	}

	fs = get_fs();/*lint !e501*/
	set_fs(KERNEL_DS);/*lint !e501 */

	pos = fp->f_pos;/*lint !e613 */
	ret = vfs_read(fp, (char __user *)buffer, length, &pos);/*lint !e613 */
	if (ret != length) {
		pr_err("read ops failed, ret=%d(len=%d)", ret, length);
		set_fs(fs);
		ret = -1;
		goto err_close;
	}
	set_fs(fs);

err_close:
	filp_close(fp, NULL);/*lint !e668 */

error:
	/*free resource*/
	if (pathname != NULL) {
		kfree(pathname);
		pathname = NULL;
	}

	return ret;
}

int sec_process_use_ca_ta(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;

	return dev->use_ca_ta;
}

static int do_secisp_device_enable(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	u64 pgd_base;
	int ret;
	int err_ret;

	ret = hisp_subsys_powerup();
	if (ret < 0) {
		pr_err("[%s] Failed : hisp_subsys_powerup.%d\n", __func__, ret);
		return ret;
	}

	ret = ispcpu_qos_cfg();
	if (ret < 0) {
		pr_err("[%s] Failed : ispcpu_qos_cfg.%d\n", __func__, ret);
		goto err_ispinit;
	}

	if (sec_process_use_ca_ta() != 0) {
		return 0;
	}

	pgd_base = hisp_get_pgd_base();
	ret = hisp_pwr_core_sec_init(pgd_base);
	if (ret < 0) {
		pr_err("[%s] Failed : hispcore sec init.%d\n", __func__, ret);
		goto err_ispinit;
	}

	ret = hisp_pwr_cpu_sec_init();
	if (ret < 0) {
		pr_err("[%s] Failed : hispcpu sec init.%d\n", __func__, ret);
		goto err_a7init;
	}

	ret = hisp_wait_rpmsg_completion();
	if (ret != 0)
		goto err_loadfw;

	ret = hisp_pwr_cpu_sec_dst(dev->ispmem_reserved,
		dev->shrdmem->a7mapping[A7BOOT].a7pa);
	if (ret < 0) {
		pr_err("[%s] Failed : hispcpu sec dst.%d\n", __func__, ret);
		goto err_loadfw;
	}

	return 0;

err_loadfw:
	err_ret = hisp_pwr_cpu_sec_exit();
	if (err_ret < 0)
		pr_err("[%s] Failed : cpu_sec_exit.%d\n", __func__, err_ret);
err_a7init:
	err_ret = hisp_pwr_core_sec_exit();
	if (err_ret < 0)
		pr_err("[%s] Failed : core_sec_exit.%d\n", __func__, err_ret);
err_ispinit:
	err_ret = hisp_subsys_powerdn();
	if (err_ret < 0)
		pr_err("[%s]Fail:hisp_subsys_powerdn.%d\n", __func__, err_ret);

	return ret;
}

static unsigned long get_a7shared_pa(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;

	return dev->atfshrd_paddr + dev->tsmem_offset[ISP_SHARE_MEM_OFFSET];
}

u32 hisp_get_hisp_vring_da(unsigned int etype)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	u32 da = 0;

	if (!sec_process_use_ca_ta()) {
		if (etype >= MAXA7MAPPING) {
			pr_err("[%s] Failed : etype.(0x%x >= 0x%x)\n",
				__func__, etype, MAXA7MAPPING);
			return da;
		}

		if (dev->shrdmem == NULL) {
			pr_err("[%s] Failed : shrdmem.(%pK = %pK)\n",
				__func__, dev->shrdmem, dev->atfshrd_vaddr);
			return da;
		}

		da = dev->shrdmem->a7mapping[etype].a7va;
	} else {
		if (etype >= HISP_SEC_RSV_MAX_TYPE) {
			pr_err("[%s] Failed : etype.(0x%x >= 0x%x)\n",
				__func__, etype, HISP_SEC_RSV_MAX_TYPE);
			return da;
		}

		da = dev->rsv_mem[etype].da;
	}

	return da;
}

unsigned long hisp_getreservemem(unsigned int etype, unsigned long paddr)
{
	unsigned long addr = 0;

	switch (etype) {
	case A7VQ:
	case A7VRING0:
	case A7VRING1:
		addr = paddr;
		break;
	case A7RDR:
		addr = get_isprdr_addr();
		break;
	case A7SHARED:
		addr = get_a7shared_pa();
		break;
	default:
		pr_debug("[%s] default : etype.0x%x addr.0x%lx\n",
				__func__, etype, addr);
		return 0;
	}

	return addr;
}

int hisp_mem_type_pa_init(unsigned int etype, unsigned long paddr)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;

	if (!sec_process_use_ca_ta()) {
		if (etype >= MAXA7MAPPING) {
			pr_err("[%s] Failed : etype.(0x%x >= 0x%x)\n",
				__func__, etype, MAXA7MAPPING);
			return -EINVAL;
		}

		if (dev->shrdmem == NULL) {
			pr_err("[%s] Failed : shrdmem.(%pK = %pK)\n",
				__func__, dev->shrdmem, dev->atfshrd_vaddr);
			return -ENOMEM;
		}

		if (dev->shrdmem->a7mapping[etype].reserve)
			dev->shrdmem->a7mapping[etype].a7pa =
					hisp_getreservemem(etype, paddr);

		pr_info("[%s] type.%d, a7va.0x%x\n", __func__,
			etype, dev->shrdmem->a7mapping[etype].a7va);
	} else {
		if (etype >= HISP_SEC_RSV_MAX_TYPE) {
			pr_err("[%s] Failed : etype.(0x%x >= 0x%x)\n",
				__func__, etype, HISP_SEC_RSV_MAX_TYPE);
			return -EINVAL;
		}

		dev->rsv_mem[etype].pa = hisp_getreservemem(etype, paddr);
		pr_info("[%s] type.%d, pa.0x%llx\n", __func__,
				etype, dev->rsv_mem[etype].pa);
	}

	return 0;
}

static int hisp_rsctablemem_init(struct hisp_sec *dev)
{
	dma_addr_t dma_addr = 0;

	dev->rsctable_vaddr_const = dma_alloc_coherent(dev->device,
				dev->rsctable_size, &dma_addr, GFP_KERNEL);
	if (dev->rsctable_vaddr_const == NULL) {
		pr_err("[%s] failed : rsctable_vaddr_const.%pK\n",
				__func__, dev->rsctable_vaddr_const);
		return -ENOMEM;
	}
	dev->rsctable_paddr = (unsigned long long)dma_addr;

	return 0;
}

static void hisp_rsctablemem_exit(struct hisp_sec *dev)
{
	if (dev->rsctable_vaddr_const != NULL)
		dma_free_coherent(dev->device, dev->rsctable_size,
			dev->rsctable_vaddr_const, dev->rsctable_paddr);
	dev->rsctable_vaddr_const = NULL;
	dev->rsctable_paddr = 0;
}

static u64 hisp_nesc_cpucfg_param_paddr(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;

	return dev->atfshrd_paddr + dev->tsmem_offset[ISP_CPU_PARM_OFFSET];
}

static struct phy_csi_info_t *hisp_phy_csi_info_vaddr(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;

	if (dev->atfshrd_vaddr != NULL)
		return (struct phy_csi_info_t *)
		       (dev->atfshrd_vaddr +
		       dev->tsmem_offset[ISP_CPU_PARM_OFFSET]);

	return NULL;
}


static int hisp_phy_csi_check(struct hisp_phy_info_t *phy_info,
				  unsigned int csi_index)
{

	if (phy_info == NULL) {
		pr_err("[%s] phy_info is NULL!\n", __func__);
		return -EINVAL;
	}

	if (phy_info->phy_id >= HISP_CDPHY_MAX) {
		pr_err("[%s] wrong para: phy_id = %u!\n",
			__func__, phy_info->phy_id);
		return -EINVAL;
	}

	if (phy_info->phy_mode >= HISP_PHY_MODE_MAX) {
		pr_err("[%s] wrong para: phy_mode = %u!\n",
			__func__, phy_info->phy_mode);
		return -EINVAL;
	}

	if (phy_info->phy_freq_mode >= HISP_PHY_FREQ_MODE_MAX) {
		pr_err("[%s] wrong para: phy_freq_mode = %u!\n",
			__func__, phy_info->phy_freq_mode);
		return -EINVAL;
	}

	if (phy_info->phy_work_mode >= HISP_PHY_WORK_MODE_MAX) {
		pr_err("[%s] wrong para: phy_work_mode = %u!\n",
			__func__, phy_info->phy_work_mode);
		return -EINVAL;
	}

	if (csi_index >= HISP_CSI_NUM_MAX) {
		pr_err("[%s] wrong para: csi_index = %u!\n",
			__func__, csi_index);
		return -EINVAL;
	}
	return 0;
}

int hisp_phy_csi_connect(struct hisp_phy_info_t *phy_info,
				  unsigned int csi_index)
{
	int ret = 0;
	u64 sharepa = 0;
	struct phy_csi_info_t *shareva = NULL;
	struct phy_csi_info_t phy_csi_info;

	ret = hisp_phy_csi_check(phy_info, csi_index);
	if (ret < 0) {
		pr_err("[%s] fail: hisp_phy_csi_check.%d!\n", __func__, ret);
		return ret;
	}

	ret = memcpy_s(&phy_csi_info.phy_info, sizeof(struct hisp_phy_info_t),
		       phy_info, sizeof(struct hisp_phy_info_t));
	if (ret != 0) {
		pr_err("[%s] memcpy_s phy info fail.%d!\n", __func__, ret);
		return ret;
	}

	phy_csi_info.csi_idx = csi_index;
	shareva = hisp_phy_csi_info_vaddr();
	if (shareva == NULL) {
		pr_err("[%s] hisp_phy_csi_info_vaddr is NULL!\n", __func__);
		return -EINVAL;
	}

	ret = memcpy_s((void *)(shareva), sizeof(struct phy_csi_info_t),
		       (const void *)(&phy_csi_info),
		       sizeof(struct phy_csi_info_t));
	if (ret != 0) {
		pr_err("[%s] memcpy_s phy csi info fail.%d!\n", __func__, ret);
		return ret;
	}

	sharepa = hisp_nesc_cpucfg_param_paddr();
	if (sharepa == 0) {
		pr_err("[%s] hisp_phy_csi_info_paddr is NULL!\n", __func__);
		return -EINVAL;
	}

	ret = atfisp_phy_csi_connect(sharepa);
	if (ret != 0) {
		pr_err("[%s] atfisp_phy_csi_connect fail ret.%d!\n",
				__func__, ret);
		return ret;
	}

	pr_info("[%s] csi_index = %d\n", __func__, csi_index);
	pr_info("[%s] is master sensor = %d, phy_id = %d\n",
		__func__, phy_info->is_master_sensor, phy_info->phy_id);
	pr_info("[%s] phy_mode = %d,phy_freq_mode = %d\n",
		__func__, phy_info->phy_mode, phy_info->phy_freq_mode);
	pr_info("[%s] phy_freq = %d,phy_work_mode = %d\n",
		__func__, phy_info->phy_freq, phy_info->phy_work_mode);

	return 0;
}
EXPORT_SYMBOL(hisp_phy_csi_connect); /*lint !e508*/

int hisp_phy_csi_disconnect(void)
{
	int ret = 0;

	ret = atfisp_phy_csi_disconnect();
	if (ret != 0) {
		pr_err("[%s] fail ret.%d!\n", __func__, ret);
		return ret;
	}

	return 0;
}
EXPORT_SYMBOL(hisp_phy_csi_disconnect); /*lint !e508*/

unsigned int wait_share_excflag_timeout(unsigned int flag, unsigned int time)
{
	struct rproc_shared_para *param = NULL;
	unsigned int timeout = time;

	pr_debug("[%s] +\n", __func__);
	param = rproc_get_share_para();
	if (param == NULL) {
		pr_err("[%s] param is NULL!\n", __func__);
		return 0;
	}

	if (timeout == 0) {
		pr_err("[%s] err : timeout.%d\n", __func__, timeout);
		return 0;
	}

	do {
		if ((param->exc_flag & flag) == flag)
			break;
		timeout--;
		mdelay(10);
	} while (timeout > 0);

	pr_debug("[%s] exc_flag.0x%x != flag.0x%x, timeout.%d\n",
			__func__, param->exc_flag, flag, timeout);
	pr_debug("[%s] -\n", __func__);
	return timeout;
}

static void set_ispcpu_idle(void)
{
	pr_debug("[%s] +\n", __func__);
	if (wait_share_excflag_timeout(ISP_CPU_POWER_DOWN, 10) == 0x0)
		ap_send_fiq2ispcpu();
	pr_info("[%s] timeout.%d!\n", __func__,
		wait_share_excflag_timeout(ISP_CPU_POWER_DOWN, 300));
	pr_debug("[%s] -\n", __func__);
}

int get_ispcpu_idle_stat(unsigned int isppd_adb_flag)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	int ret = 0;

	if (dev->isp_cpu_reset)
		set_ispcpu_idle();

	if (isppd_adb_flag) {
		ret = atfisp_get_ispcpu_idle();
		if (ret != 0)
			pr_alert("[%s] Failed : ispcpu not in idle.%d!\n",
					__func__, ret);
	}

	return ret;
}

struct isp_nsec_cpu_dump_s *get_debug_ispcpu_param(void)
{
	return NULL;
}

int hisp_mntn_dumpregs(void)
{
	return 0;
}

int get_ispcpu_cfg_info(void)
{
	return 0;
}

int fiq2ispcpu(void)
{
	int ret = 0;

	pr_info("[%s] +\n", __func__);
	ret = atfisp_send_fiq2ispcpu();
	if (ret < 0) {
		pr_err("[%s] atfisp_send_fiq2ispcpu.%d\n", __func__, ret);
		return -1;
	}

	pr_info("[%s] -\n", __func__);
	return 0;
}

static int hisp_sharedmem_init(struct hisp_sec *dev)
{
	dma_addr_t dma_addr = 0;

	dev->atfshrd_vaddr = isp_fstcma_alloc(&dma_addr,
		dev->share_mem_size, GFP_KERNEL);
	if (dev->atfshrd_vaddr == NULL) {
		pr_err("[%s] atfshrd_vaddr.%pK\n",
			__func__, dev->atfshrd_vaddr);
		return -ENOMEM;
	}

	dev->atfshrd_paddr = (unsigned long long)dma_addr;
	dev->shrdmem = (struct isp_atfshrdmem_s *)dev->atfshrd_vaddr;

	return 0;
}

static void hisp_sharedmem_exit(struct hisp_sec *dev)
{
	if (dev->atfshrd_vaddr != NULL)
		isp_fstcma_free(dev->atfshrd_vaddr,
			dev->atfshrd_paddr, dev->share_mem_size);
	dev->atfshrd_paddr = 0;
	dev->atfshrd_vaddr = NULL;
}

void *get_rsctable(int *tablesz)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;

	*tablesz = dev->rsctable_size;
	return dev->rsctable_vaddr;
}

void a7_map_set_pa_list(void *listmem,
		struct scatterlist *sg, unsigned int size)
{
	dma_addr_t dma_addr = 0;
	unsigned int len, set_size = 0;
	struct map_sglist_s *maplist = listmem;
	unsigned int last_counts = 0, last_len = 0;

	while (sg != NULL) {
		dma_addr = sg_dma_address(sg);
		if (dma_addr == 0)
			dma_addr = sg_phys(sg);

		len = sg->length;
		if (len == 0) {
			pr_err("[%s] break len.0x%x\n", __func__, len);
			break;
		}

		set_size += len;
		if (set_size > size) {
			pr_err("[%s] break size.(0x%x > 0x%x), len.0x%x\n",
					__func__, set_size, size, len);
			maplist->addr = (unsigned long long)dma_addr;
			maplist->size = len - (set_size - size);
			break;
		}

		maplist->addr = (unsigned long long)dma_addr;
		maplist->size = len;
		if (last_len != len) {
			if (last_len != 0) {
				pr_info("[%s] list.(%pK + %pK), ",
					__func__, listmem, maplist);
				pr_info("maplist.(0x%x X 0x%x)\n",
					last_counts, last_len);
			}
			last_counts = 1;
			last_len = len;
		} else {
			last_counts++;
		}

		maplist++;
		sg = sg_next(sg);
	}

	pr_info("[%s] list.(%pK + %pK), maplist.(0x%x X 0x%x)\n",
		__func__, listmem, maplist, last_counts, last_len);
	pr_info("%s: size.0x%x == set_size.0x%x\n", __func__, size, set_size);
}

int hisp_request_sec_rsctable(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;

	pr_info("[%s] +\n", __func__);
	if (dev->rsctable_vaddr_const == NULL) {
		pr_err("[%s] rsctable_vaddr_const.%pK\n",
			__func__, dev->rsctable_vaddr_const);
		return -ENOMEM;
	}

	dev->rsctable_vaddr = kmemdup(dev->rsctable_vaddr_const,
				dev->rsctable_size, GFP_KERNEL);
	if (dev->rsctable_vaddr == NULL)
		return -ENOMEM;

	pr_info("[%s] -\n", __func__);

	return 0;
}

static u64 get_boot_sec_phyaddr(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;


	if (dev->sec_boot_phymem_addr)
		return dev->sec_boot_phymem_addr;

	return 0;
}

static u64 get_fw_sec_phyaddr(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;

	if (dev->sec_fw_phymem_addr)
		return dev->sec_fw_phymem_addr;

	return 0;
}

void isp_secisp_dump(void)
{
	atfisp_params_dump(get_boot_sec_phyaddr());
}

static unsigned long hisp_set_secmem_addr_pa(unsigned int etype,
					unsigned int vaddr, unsigned int offset)
{
	unsigned long addr = 0;
	unsigned long remap = 0;

	remap = (unsigned long)get_a7remap_addr();
	switch (etype) {
	case A7TEXT:
	case A7PGD:
	case A7PMD:
	case A7PTE:
	case A7DATA:
		addr = (unsigned long)(remap + offset);
		break;
	case A7BOOT:
		addr = (unsigned long)(remap + offset);
		break;
	case A7HEAP:
		remap = (unsigned long)get_fw_sec_phyaddr();
		addr = (unsigned long)(remap + offset);
		break;
	case A7MDC:
		addr =  get_mdc_addr_pa();/*lint !e747 */
		if (!addr) {
			pr_err("[%s] get_mdc_addr_pa is NULL!\n", __func__);
			return 0;
		}

		set_shared_mdc_pa_addr(addr);
		break;
	default:
		pr_debug("[%s] Failed : etype.0x%x\n", __func__, etype);
		return 0;
	}

	return addr;
}

static int hisp_secmem_pa_init(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	int index = 0;
	unsigned long addr = 0;
	u64 remap = 0;


	if (sec_process_use_ca_ta())
		return 0;

	for (index = 0; index < MAXA7MAPPING; index++) {
		addr = hisp_set_secmem_addr_pa(index,
				dev->shrdmem->a7mapping[index].a7va,
				dev->shrdmem->a7mapping[index].offset);
		if (addr != 0)
			dev->shrdmem->a7mapping[index].a7pa = addr;
	}
	remap = get_a7remap_addr();


	if (remap == 0) {
		pr_err("[%s]ERR: remap addr.0 err!\n", __func__);
		return -ENODEV;
	}

	dev->shrdmem->sec_pgt = remap;
	dev->shrdmem->sec_mem_size = dev->trusted_mem_size;
	return 0;
}

static int set_share_pararms(void)
{
	int ret = 0;
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;

	ret = atfisp_setparams(dev->atfshrd_paddr);
	if (ret < 0) {
		pr_err("[%s] atfisp_setparams.%d\n", __func__, ret);
		if (dev->atfshrd_vaddr == NULL)
			return -ENODEV;
		kfree(dev->atfshrd_vaddr);
		dev->atfshrd_vaddr = NULL;
		return -ENODEV;
	}
	return 0;
}

void free_secmem_rsctable(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;

	if (dev->rsctable_vaddr != NULL) {
		kfree(dev->rsctable_vaddr);
		dev->rsctable_vaddr = NULL;
	}
}

static void hisp_get_static_mem_pa(unsigned int type)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	void *va = NULL;

	switch (type) {
	case HISP_SEC_VR0:
	case HISP_SEC_VR1:
	case HISP_SEC_VQ:
		va = get_vring_dma_addr(&dev->rsv_mem[type].pa,
			dev->rsv_mem[type].size, type);
		break;
	case HISP_SEC_SHARE:
		dev->rsv_mem[type].pa = get_a7shared_pa();
		break;
	case HISP_SEC_RDR:
		dev->rsv_mem[type].pa = get_isprdr_addr();
		break;
	default:
		pr_err("[%s] Failed : wrong rsv_mem_type.%u\n",
			__func__, type);
		break;
	}

	pr_info("[%s] rsv_mem[%u].pa.0x%llx\n", __func__,
		type, dev->rsv_mem[type].pa);
}

static int hisp_secmem_dynpool_map(int sharefd, unsigned int size)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	int ret = 0;

	if (dev->pool_mem[HISP_DYNAMIC_POOL].sharefd > 0) {
		pr_err("[%s] Failed : dynamic memory has already mapped\n",
			__func__);
		return -EINVAL;
	}

	dev->pool_mem[HISP_DYNAMIC_POOL].sharefd = sharefd;
	dev->pool_mem[HISP_DYNAMIC_POOL].size = size;

	ret = teek_secisp_dynamic_mem_map(&dev->pool_mem[HISP_DYNAMIC_POOL],
			dev->device);
	if (ret < 0) {
		pr_err("[%s] Failed :teek_secisp_dynamic_mem_map. ret.%d\n",
			__func__, ret);
		dev->pool_mem[HISP_DYNAMIC_POOL].sharefd = 0;
		dev->pool_mem[HISP_DYNAMIC_POOL].size = 0;
	}

	return ret;
}

static int hisp_secmem_dynpool_unmap(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	int ret = 0;

	if (dev->pool_mem[HISP_DYNAMIC_POOL].sharefd <= 0 ||
		dev->pool_mem[HISP_DYNAMIC_POOL].size == 0) {
		pr_err("[%s] Failed : memory has mapped. fd.%d, size.%u\n",
			__func__, dev->pool_mem[HISP_DYNAMIC_POOL].sharefd,
			dev->pool_mem[HISP_DYNAMIC_POOL].size);
		return -EINVAL;
	}

	ret = teek_secisp_dynamic_mem_unmap(&dev->pool_mem[HISP_DYNAMIC_POOL],
			dev->device);
	if (ret < 0)
		pr_err("[%s] Failed :teek_secisp_dynamic_mem_map. ret.%d\n",
			__func__, ret);

	dev->pool_mem[HISP_DYNAMIC_POOL].sharefd = 0;
	dev->pool_mem[HISP_DYNAMIC_POOL].size = 0;

	return ret;
}

static int hisp_secmem_secpool_map(int sharefd, unsigned int size,
		unsigned int pool_num)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	int ret = 0;

	if (dev->pool_mem[pool_num].sharefd > 0) {
		pr_err("[%s] Failed : sec memory has mapped\n", __func__);
		return -EINVAL;
	}

	dev->pool_mem[pool_num].sharefd = sharefd;
	dev->pool_mem[pool_num].size = size;

	ret = teek_secisp_mem_map(&dev->pool_mem[pool_num]);
	if (ret < 0) {
		pr_err("[%s] Failed :teek_secisp_mem_map. ret.%d\n",
			__func__, ret);
		dev->pool_mem[pool_num].sharefd = 0;
		dev->pool_mem[pool_num].size = 0;
	}

	return ret;
}

static int hisp_secmem_secpool_unmap(unsigned int pool_num)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	int ret = 0;

	if (dev->pool_mem[pool_num].sharefd <= 0 ||
		dev->pool_mem[pool_num].size == 0) {
		pr_err("[%s] Failed : sec memory has already mapped. fd.%d, size.%u\n",
			__func__, dev->pool_mem[pool_num].sharefd,
			dev->pool_mem[pool_num].size);
		return -EINVAL;
	}

	ret = teek_secisp_mem_unmap(&dev->pool_mem[pool_num]);
	if (ret < 0)
		pr_err("[%s] Failed :teek_secisp_mem_unmap. ret.%d\n",
			__func__, ret);

	dev->pool_mem[pool_num].sharefd = 0;
	dev->pool_mem[pool_num].size = 0;

	return ret;
}

unsigned int hisp_pool_mem_addr(unsigned int pool_num)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	unsigned int addr;

	mutex_lock(&dev->ca_mem_mutex);
	if (pool_num >= HISP_SEC_POOL_MAX_TYPE) {
		pr_err("[%s] Failed : pool num %u\n", __func__, pool_num);
		mutex_unlock(&dev->ca_mem_mutex);
		return 0;
	}

	addr = dev->pool_mem[pool_num].da;
	mutex_unlock(&dev->ca_mem_mutex);
	pr_info("[%s] pool_num.%u iova.0x%x\n", __func__, pool_num, addr);
	return addr;
}

int hisp_secmem_size_get(unsigned int *trusted_mem_size)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;

	pr_info("[%s] +", __func__);
	mutex_lock(&dev->ca_mem_mutex);
	if (dev->trusted_mem_size == 0) {
		pr_err("[%s] Failed : trusted mem size.%u\n",
				__func__, dev->trusted_mem_size);
		mutex_unlock(&dev->ca_mem_mutex);
		return -EINVAL;
	}

	*trusted_mem_size = dev->trusted_mem_size;
	mutex_unlock(&dev->ca_mem_mutex);

	pr_info("[%s] -", __func__);

	return 0;
}
EXPORT_SYMBOL(hisp_secmem_size_get);

int hisp_secmem_pa_set(u64 sec_boot_phymem_addr, unsigned int trusted_mem_size)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;

	pr_info("[%s] +", __func__);
	mutex_lock(&dev->ca_mem_mutex);
	if (dev->secmem_count > 0) {
		pr_err("[%s] Failed : secmem_count.%u\n",
				__func__, dev->secmem_count);
		mutex_unlock(&dev->ca_mem_mutex);
		return -EINVAL;
	}

	if (sec_boot_phymem_addr == 0) {
		pr_err("[%s] Failed : sec_boot_phymem_addr is 0\n", __func__);
		mutex_unlock(&dev->ca_mem_mutex);
		return -EINVAL;
	}

	if (trusted_mem_size < dev->trusted_mem_size) {
		pr_err("[%s] Failed : trusted_mem_size err.%u\n",
				__func__, trusted_mem_size);
		mutex_unlock(&dev->ca_mem_mutex);
		return -EINVAL;
	}

	dev->sec_boot_phymem_addr = sec_boot_phymem_addr;
	pr_debug("[%s] sec_boot_phymem_addr.0x%llx\n",
			__func__, dev->sec_boot_phymem_addr);
	dev->secmem_count++;
	mutex_unlock(&dev->ca_mem_mutex);
	pr_info("[%s] - secmem_count.%u\n", __func__, dev->secmem_count);
	return 0;
}
EXPORT_SYMBOL(hisp_secmem_pa_set);

int hisp_secmem_pa_release(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;

	mutex_lock(&dev->ca_mem_mutex);
	if (dev->secmem_count <= 0) {
		pr_err("[%s] Failed : secmem_count.%u\n",
				__func__, dev->secmem_count);
		mutex_unlock(&dev->ca_mem_mutex);
		return -EINVAL;
	}
	dev->secmem_count--;
	mutex_unlock(&dev->ca_mem_mutex);
	pr_info("[%s] smem_count.%u\n", __func__, dev->secmem_count);
	return 0;
}
EXPORT_SYMBOL(hisp_secmem_pa_release);

int hisp_secmem_ca_map(unsigned int pool_num, int sharefd, unsigned int size)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	int ret;

	pr_info("[%s] + %u", __func__, pool_num);

	mutex_lock(&dev->ca_mem_mutex);
	if (pool_num >= HISP_SEC_POOL_MAX_TYPE || sharefd < 0 || size == 0) {
		pr_err("[%s] Failed : pool num %u, sharefd.%d, size.%u\n",
			__func__, pool_num, sharefd, size);
		mutex_unlock(&dev->ca_mem_mutex);
		return -EINVAL;
	}

	if(!IS_ERR_OR_NULL(dev->pool_dmabuf[pool_num])) {
		pr_err("[%s] Failed :dev->pool_dmabuf not NULL!. type.%d\n",
			__func__, pool_num);
		mutex_unlock(&dev->ca_mem_mutex);
		return -EINVAL;
	}

	dev->pool_dmabuf[pool_num] = dma_buf_get(sharefd);
	if (IS_ERR_OR_NULL(dev->pool_dmabuf[pool_num])) {
		pr_err("[%s] Failed : dma buf get fail. fd.%d\n",
			__func__, sharefd);
		mutex_unlock(&dev->ca_mem_mutex);
		return -EINVAL;
	}
	if (pool_num == 0) {
		ret = hisp_secmem_dynpool_map(sharefd, size);
		if (ret < 0) {
			pr_err("[%s] Failed :hisp_secmem_dynpool_map. ret.%d\n",
				__func__, ret);
			goto map_err;
		}
	} else {
		ret = hisp_secmem_secpool_map(sharefd, size, pool_num);
		if (ret < 0) {
			pr_err("[%s] Failed :secpool[%d] map. ret.%d\n",
				__func__, pool_num, ret);
			goto map_err;
		}
	}
	mutex_unlock(&dev->ca_mem_mutex);
	pr_info("[%s] -", __func__);
	return 0;
map_err:
	dma_buf_put(dev->pool_dmabuf[pool_num]);
	dev->pool_dmabuf[pool_num] = NULL;
	mutex_unlock(&dev->ca_mem_mutex);
	return ret;
}

int hisp_secmem_ca_unmap(unsigned int pool_num)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	int ret = 0;

	pr_info("[%s] + %u", __func__, pool_num);

	mutex_lock(&dev->ca_mem_mutex);
	if (pool_num >= HISP_SEC_POOL_MAX_TYPE) {
		pr_err("[%s] Failed : pool num %u\n", __func__, pool_num);
		mutex_unlock(&dev->ca_mem_mutex);
		return -EINVAL;
	}

	if (pool_num == HISP_DYNAMIC_POOL) {
		ret = hisp_secmem_dynpool_unmap();
		if (ret < 0)
			pr_err("[%s] Failed :hisp_secmem_dynpool_unmap. ret.%d\n",
				__func__, ret);
	} else {
		ret = hisp_secmem_secpool_unmap(pool_num);
		if (ret < 0)
			pr_err("[%s] Failed :secpool[%d]_unmap. ret.%d\n",
				__func__, pool_num, ret);
	}

	if(IS_ERR_OR_NULL(dev->pool_dmabuf[pool_num])) {
		pr_err("[%s] Failed :dev->pool_dmabuf is NULL. type.%d\n",
			__func__, pool_num);
		mutex_unlock(&dev->ca_mem_mutex);
		return -EINVAL;
	}

	dma_buf_put(dev->pool_dmabuf[pool_num]);
	dev->pool_dmabuf[pool_num] = NULL;
	mutex_unlock(&dev->ca_mem_mutex);
	pr_info("[%s] -", __func__);
	return ret;
}

int hisp_secboot_memsize_get_from_type(unsigned int type, unsigned int *size)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;

	pr_info("[%s] +", __func__);

	mutex_lock(&dev->ca_mem_mutex);
	if (type >= HISP_SEC_BOOT_MAX_TYPE) {
		pr_err("[%s] Failed : wrong type.%u\n", __func__, type);
		mutex_unlock(&dev->ca_mem_mutex);
		return -EINVAL;
	}

	if (dev->boot_mem[type].size == 0) {
		pr_err("[%s] Failed : boot_mem.size.%u, type.%u\n",
			__func__, dev->boot_mem[type].size, type);
		mutex_unlock(&dev->ca_mem_mutex);
		return -EINVAL;
	}

	*size = dev->boot_mem[type].size;
	mutex_unlock(&dev->ca_mem_mutex);

	pr_info("[%s] -", __func__);
	return 0;
}

int hisp_secboot_info_set(unsigned int type, int sharefd)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;

	pr_info("[%s] + %u", __func__, type);
	mutex_lock(&dev->ca_mem_mutex);
	if (sharefd <= 0 || type >= HISP_SEC_BOOT_MAX_TYPE) {
		pr_err("[%s] Failed : sharefd.%d, type.%u\n",
			__func__, sharefd, type);
		mutex_unlock(&dev->ca_mem_mutex);
		return -EINVAL;
	}

	if(!IS_ERR_OR_NULL(dev->boot_dmabuf[type])) {
		pr_err("[%s] Failed :dev->boot_dmabuf not NULL!. type.%d\n",
			__func__, type);
		mutex_unlock(&dev->ca_mem_mutex);
		return -EINVAL;
	}

	dev->boot_dmabuf[type] = dma_buf_get(sharefd);
	if (IS_ERR_OR_NULL(dev->boot_dmabuf[type])) {
		pr_err("[%s] Failed : dma buf get fail. fd.%d\n",
			__func__, sharefd);
		mutex_unlock(&dev->ca_mem_mutex);
		return -EINVAL;
	}

	dev->boot_mem[type].sharefd = sharefd;
	mutex_unlock(&dev->ca_mem_mutex);
	pr_info("[%s] -", __func__);
	return 0;
}

int hisp_secboot_info_release(unsigned int type)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;

	pr_info("[%s] +", __func__);
	mutex_lock(&dev->ca_mem_mutex);
	if (type >= HISP_SEC_BOOT_MAX_TYPE) {
		pr_err("[%s] Failed : type.%u\n",
			__func__, type);
		mutex_unlock(&dev->ca_mem_mutex);
		return -EINVAL;
	}

	dev->boot_mem[type].sharefd = 0;

	if(IS_ERR_OR_NULL(dev->boot_dmabuf[type])) {
		pr_err("[%s] Failed :dev->boot_dmabuf is NULL. type.%d\n",
			__func__, type);
		mutex_unlock(&dev->ca_mem_mutex);
		return -EINVAL;
	}

	dma_buf_put(dev->boot_dmabuf[type]);
	dev->boot_dmabuf[type] = NULL;
	mutex_unlock(&dev->ca_mem_mutex);
	pr_info("[%s] -", __func__);
	return 0;
}

int hisp_secboot_prepare(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	int ret, index;

	mutex_lock(&dev->ca_mem_mutex);
	if(dev->boot_mem_ready_flag) {
		pr_err("[%s] Failed : boot_mem_ready_flag.%d\n",
				__func__, dev->boot_mem_ready_flag);
		mutex_unlock(&dev->ca_mem_mutex);
		return -EINVAL;
	}

	for (index = HISP_SEC_TEXT; index < HISP_SEC_BOOT_MAX_TYPE; index++) {
		if (dev->boot_mem[index].sharefd == 0) {
			dev->boot_mem_ready_flag = 0;
			pr_err("[%s] Failed : boot type.%u, sharefd.%d\n",
				__func__, index, dev->boot_mem[index].sharefd);
			mutex_unlock(&dev->ca_mem_mutex);
			return -EINVAL;
		}
	}

	for (index = 0; index < HISP_SEC_RSV_MAX_TYPE; index++)
		hisp_get_static_mem_pa(index);

	ret = teek_secisp_boot_mem_config(dev);
	if (ret < 0) {
		dev->boot_mem_ready_flag = 0;
		pr_err("[%s] Failed :teek_secisp_boot_mem_config. ret.%d\n",
			__func__, ret);
		mutex_unlock(&dev->ca_mem_mutex);
		return ret;
	}

	dev->boot_mem_ready_flag = 1;
	mutex_unlock(&dev->ca_mem_mutex);

	return 0;
}

int hisp_secboot_unprepare(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	int ret, index;

	mutex_lock(&dev->ca_mem_mutex);

	for (index = HISP_SEC_TEXT; index < HISP_SEC_BOOT_MAX_TYPE; index++) {
		if (dev->boot_mem[index].sharefd == 0) {
			pr_err("[%s] boot type.%u, sharefd.%d is 0\n",
				__func__, index, dev->boot_mem[index].sharefd);
			mutex_unlock(&dev->ca_mem_mutex);
			return -EINVAL;
		}
	}

	ret = teek_secisp_boot_mem_deconfig(dev);
	if (ret < 0) {
		pr_err("[%s] Failed :teek_secisp_boot_mem_deconfig. ret.%d\n",
			__func__, ret);
		mutex_unlock(&dev->ca_mem_mutex);
		return ret;
	}
	dev->boot_mem_ready_flag = 0;

	mutex_unlock(&dev->ca_mem_mutex);

	return 0;
}

int hisp_sec_ta_enable(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	int ret = 0;

	pr_info("[%s] +", __func__);

	mutex_lock(&dev->ta_status_mutex);
	if (dev->sec_ta_enable == 1) {
		pr_err("%s: ta already enable\n", __func__);
		mutex_unlock(&dev->ta_status_mutex);
		return -EINVAL;
	}

	ret = teek_secisp_open();
	if (ret < 0) {
		pr_err("%s: fail: teek_secisp_open\n", __func__);
		mutex_unlock(&dev->ta_status_mutex);
		return -EINVAL;
	}

	dev->sec_ta_enable = 1;
	mutex_unlock(&dev->ta_status_mutex);

	pr_info("[%s] -", __func__);

	return 0;
}
EXPORT_SYMBOL(hisp_sec_ta_enable);

int hisp_sec_ta_disable(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	int index;
	int ret = 0;

	pr_info("[%s] +", __func__);

	mutex_lock(&dev->ta_status_mutex);
	if (dev->sec_poweron_status) {
		pr_err("%s: sec_poweron_status.%d\n", __func__,dev->sec_poweron_status);
		mutex_unlock(&dev->ta_status_mutex);
		return -EINVAL;
	}

	if (dev->sec_ta_enable == 0) {
		pr_err("%s: ta already disable\n", __func__);
		mutex_unlock(&dev->ta_status_mutex);
		return -EINVAL;
	}

	for (index = 0; index < HISP_SEC_POOL_MAX_TYPE; index++) {
		if (dev->pool_dmabuf[index] != NULL)
			(void)hisp_secmem_ca_unmap(index);
	}

	(void)hisp_secboot_unprepare();
	for (index = 0; index < HISP_SEC_BOOT_MAX_TYPE; index++)
		(void)hisp_secboot_info_release(index);

	ret = teek_secisp_close();
	if (ret < 0) {
		pr_err("%s: fail: teek_secisp_close\n", __func__);
		mutex_unlock(&dev->ta_status_mutex);
		return -EINVAL;
	}

	dev->sec_ta_enable = 0;
	mutex_unlock(&dev->ta_status_mutex);

	pr_info("[%s] -", __func__);
	return ret;
}
EXPORT_SYMBOL(hisp_sec_ta_disable);


int hisp_sec_pwron_prepare(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	int ret;

	pr_info("[%s] +\n", __func__);
	ret = set_share_pararms();
	if (ret < 0)
		pr_err("[%s] Failed : set_share_pararms.%d\n", __func__, ret);

	ret = hisp_bsp_read_bin("isp_firmware", dev->rsctable_offset,
			dev->rsctable_size, dev->rsctable_vaddr_const);
	if (ret < 0) {
		pr_err("[%s] hisp_bsp_read_bin.%d\n", __func__, ret);
		return ret;
	}
	pr_info("[%s] -\n", __func__);

	return 0;
}

static int hisp_sec_boot(struct hisp_sec *dev)
{
	int ret = 0;

	ret = set_isp_remap_addr(get_boot_sec_phyaddr());
	if (ret < 0) {
		pr_err("[%s] Failed : set_isp_remap_addr.%d\n",
				__func__, ret);
		return ret;
	}

	ret = hisp_secmem_pa_init();
	if (ret < 0) {
		pr_err("[%s] Failed : hisp_secmem_pa_init.%d\n", __func__, ret);
		return ret;
	}

	ret = do_secisp_device_enable();
	if (ret < 0)
		pr_err("[%s] Failed : do_secisp_device_enable.%d.\n",
				__func__, ret);

	return ret;
}

int secisp_device_enable(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	int ret = 0;

	if(sec_process_use_ca_ta() && (dev->boot_mem_ready_flag != 1)) {
		pr_err("[%s] Failed : boot_mem_ready_flag.%d\n",
			__func__, dev->boot_mem_ready_flag);
		return -EINVAL;
	}

	if (sec_process_use_ca_ta()) {
		mutex_lock(&dev->ta_status_mutex);
		if (dev->sec_ta_enable == 1) {
			dev->sec_poweron_status = 1;
		} else {
			pr_err("[%s] Failed : sec_ta_enable status=%d\n",
							__func__, dev->sec_ta_enable);
			mutex_unlock(&dev->ta_status_mutex);
			return -1;
		}
		mutex_unlock(&dev->ta_status_mutex);
	}

	mutex_lock(&dev->pwrlock);
	ret = hisp_sec_boot(dev);
	if (ret != 0)
		pr_err("[%s] Failed : hisp_sec_boot.%d\n", __func__, ret);
	mutex_unlock(&dev->pwrlock);

	return ret;
}

int secisp_device_disable(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	int ret;

	mutex_lock(&dev->pwrlock);
	if (!sec_process_use_ca_ta()) {
		ret = hisp_pwr_cpu_sec_rst();
		if (ret < 0)
			pr_err("[%s] Failed : hisp_pwr_cpu_sec_rst.%d\n",
				__func__, ret);

		ret = hisp_pwr_cpu_sec_exit();
		if (ret < 0)
			pr_err("[%s] Failed : hisp_pwr_cpu_sec_exit.%d\n",
				__func__, ret);

		ret = hisp_pwr_core_sec_exit();
		if (ret < 0)
			pr_err("[%s] Failed : hisp_pwr_core_sec_exit.%d\n",
				__func__, ret);
	}

	ret = hisp_subsys_powerdn();
	if (ret < 0)
		pr_err("[%s] Failed : ispsrtdn.%d\n", __func__, ret);

	mutex_unlock(&dev->pwrlock);

	if (sec_process_use_ca_ta()) {
		mutex_lock(&dev->ta_status_mutex);
		dev->sec_poweron_status = 0;
		mutex_unlock(&dev->ta_status_mutex);
		ret = hisp_sec_ta_disable();
		if (ret < 0)
			pr_err("[%s] Failed : hisp_sec_ta_disable.%d.\n",
					__func__, ret);
	}

	if (sync_isplogcat() < 0)
		pr_err("[%s] Failed: sync_isplogcat\n", __func__);

	return 0;
}


void isp_ispsec_share_para_set(void)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;

	isp_share_para = dev->sec_isp_share_para;
	pr_info("%s.%d: isp_share_para.%pK, dev->sec_isp_share_para.%pK, case.%u\n",
		__func__, __LINE__, isp_share_para, dev->sec_isp_share_para,
		hisp_rproc_case_get());
}

static int sec_rproc_dev_info_init(struct platform_device *pdev)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	int ret;

	if (pdev == NULL) {
		pr_err("[%s] Failed : platform_device is NULL\n", __func__);
		return -ENOMEM;
	}

	ret = memset_s(dev, sizeof(struct hisp_sec),
		0, sizeof(struct hisp_sec));
	if (ret < 0) {
		pr_err("[%s] Failed : memset_s dev.%d.\n",
			__func__, ret);
		return -ENOMEM;
	}

	dev->device = &pdev->dev;

	ret = memset_s(dev->rsv_mem,
		HISP_SEC_RSV_MAX_TYPE * sizeof(struct secisp_mem_info),
		0, HISP_SEC_RSV_MAX_TYPE * sizeof(struct secisp_mem_info));
	if (ret < 0) {
		pr_err("[%s] Failed : memset_s dev->rsv_mem.%d.\n",
			__func__, ret);
		return -ENOMEM;
	}

	ret = memset_s(dev->boot_mem,
		HISP_SEC_BOOT_MAX_TYPE * sizeof(struct secisp_mem_info),
		0, HISP_SEC_BOOT_MAX_TYPE * sizeof(struct secisp_mem_info));
	if (ret < 0) {
		pr_err("[%s] Failed : memset_s dev->boot_mem.%d.\n",
			__func__, ret);
		return -ENOMEM;
	}

	ret = memset_s(dev->pool_mem,
		HISP_SEC_POOL_MAX_TYPE * sizeof(struct secisp_mem_info),
		0, HISP_SEC_POOL_MAX_TYPE * sizeof(struct secisp_mem_info));
	if (ret < 0) {
		pr_err("[%s] Failed : memset_s dev->pool_mem.%d.\n",
			__func__, ret);
		return -ENOMEM;
	}

	return 0;
}


static void isp_atfisp_mapping(struct hisp_sec *dev)
{
	int ret;

	ret = hisp_secmem_getdts(dev);
	if (ret < 0) {
		pr_err("[%s] Failed : hisp_secmem_getdts.%d.\n", __func__, ret);
		return;
	}

	if (!sec_process_use_ca_ta()) {
		isp_share_para = dev->shrdmem->a7mapping[A7SHARED].apva;
	} else {
		isp_share_para = getsec_a7sharedmem_addr();
	}
	dev->sec_isp_share_para = isp_share_para;
}

int isp_atfisp_probe(struct platform_device *pdev)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;
	int ret = 0;

	ret = sec_rproc_dev_info_init(pdev);
	if (ret < 0) {
		pr_err("[%s] Failed : dev_info_init.%d.\n", __func__, ret);
		return -EINVAL;
	}

	ret = hisp_sec_getdts(pdev, dev);
	if (ret < 0) {
		pr_err("[%s] Failed : isp_atf_getdts.%d.\n", __func__, ret);
		return -EINVAL;
	}

	ret = hisp_sharedmem_init(dev);
	if (ret < 0) {
		pr_err("[%s] Failed: hisp_sharedmem_init.%d.\n", __func__, ret);
		return -ENOMEM;
	}

	ret = hisp_rsctablemem_init(dev);
	if (ret < 0) {
		pr_err("[%s] Failed: rsctablemem_init.%d.\n", __func__, ret);
		goto hisp_rsctablemem_init_fail;
	}

	isp_atfisp_mapping(dev);

	teek_secisp_ca_probe();

	ret = hisp_mem_pool_init(dev->isp_iova_start, dev->isp_iova_size);
	if (ret < 0) {
		pr_err("Failed : hisp_mem_pool_init.%d\n", ret);
		goto iommu_get_domain_for_dev_fail;
	}

	mutex_init(&dev->pwrlock);
	mutex_init(&dev->ca_mem_mutex);
	mutex_init(&dev->ta_status_mutex);

	return 0;

iommu_get_domain_for_dev_fail:
	teek_secisp_ca_remove();
	hisp_rsctablemem_exit(dev);

hisp_rsctablemem_init_fail:
	hisp_sharedmem_exit(dev);

	return ret;
}

int isp_atfisp_remove(struct platform_device *pdev)
{
	struct hisp_sec *dev = (struct hisp_sec *)&sec_rproc_dev;

	hisp_mem_pool_exit();
	teek_secisp_ca_remove();
	mutex_destroy(&dev->pwrlock);
	mutex_destroy(&dev->ca_mem_mutex);
	mutex_destroy(&dev->ta_status_mutex);
	hisp_rsctablemem_exit(dev);
	hisp_sharedmem_exit(dev);

	return 0;
}

MODULE_DESCRIPTION("ISP atfisp module");
MODULE_LICENSE("GPL");

