/*
 * hisilicon ISP driver, hisi_atfisp.c
 *
 * Copyright (c) 2013 Hisilicon Technologies CO., Ltd.
 *
 */

//ISP_LINT
/*lint -e750
  -esym(750,*)*/
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
#include <asm/uaccess.h>
#include <asm/compiler.h>
#include <global_ddr_map.h>
#include "teek_client_id.h"
#include <linux/hisi/hisi_load_image.h>
#include <linux/hisi/kirin_partition.h>
#include <linux/platform_data/remoteproc-hisi.h>
#include "hisi_partition.h"
#include <dsm/dsm_pub.h>
#include <isp_ddr_map.h>
#include <linux/list.h>
#include <securec.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include "hisp_internel.h"
#include "secisp_ca.h"

#define SEC_MAX_SIZE    64
#define ISP_ATF_CPU 1
#define DEVICE_PATH  "/dev/block/bootdevice/by-name/"

#define A7_REQUEST_ADDR         0xD0000000
#define A7_REQUEST_BUF_SIZE     0x96000
#define A7_REQUEST_BUF_NUM      16
#define A7_ISPDTS_ADDR          0xC8000000
#define A7_ISPDTS_SIZE          0x02000000
#define A7_DTS_BUF_SIZE         0x200000
#define A7_DTS_BUF_NUM          16
#define A7_GET_OTP_ADDR         0xD0A00000
#define A7_CAPABILITY_ADDR      0xD0B00000
#define A7_ACQUIRE_ADDR         0xD0C00000
#define A7_CALIBRATION_ADDR     0xD0D00000
#define A7_TONEMAP_ADDR         0xD0E00000
#define A7_MAP_BUFFER_ADDR      0xD1000000

#define IRIS_MAP_BUFFER_ADDR    0xC2A00000
#define A7_MAP_DMAP_BUFFER_ADDR 0xCA000000
#define A7_MAP_BUFFER_SIZE      0xF000000
#define IRIS_MAP_BUFFER_SIZE    0XF000000

/* hisp mem_pool  */
#define ISP_MEM_POOL_NUM     (4)
#define ISP_MEM_POOL_ALIGN   (0x8000)

#define SEC_MEM_MAP_ALIGEN	(0x200000)

enum mapType {
	MAP_TYPE_DYNAMIC    = 0,
	MAP_TYPE_RAW2YUV,
	MAP_TYPE_STATIC,
	MAP_TYPE_STATIC_SEC,
	MAP_TYPE_DYNAMIC_CARVEOUT,
	MAP_TYPE_STATIC_ISP_SEC,
	MAP_TYPE_DYNAMIC_SEC,
	MAP_TYPE_COREDUMP   = 10,
	MAP_TYPE_MAX,
};

enum isp_tsmem_offset_info {
	ISP_TSMEM_OFFSET = 0,
	ISP_CPU_PARM_OFFSET,
	ISP_SHARE_MEM_OFFSET,
	ISP_DYN_MEM_OFFSET,
	ISP_TSMEM_MAX,
};

struct hisi_isp_ops {
#define UNINITIAL   0
	unsigned int refs_ispsrt_subsys;
	unsigned int refs_isp_module;
	unsigned int refs_a7_module;
	unsigned int refs_dts;
	unsigned int refs_rsc;
	unsigned int refs_fw;
	unsigned int refs_setparams;
	unsigned int refs_paramsdump;
	int (*ispsrtup)(void);
	int (*ispsrtdn)(void);
	int (*ispinit)(void);
	int (*ispexit)(void);
	int (*a7init)(void);
	int (*a7exit)(void);
	int (*loaddts)(void);
	int (*loadrsc)(void);
	int (*loadfw)(void);
	int (*setparams)(u64);
	int (*paramsdump)(u64);
};

struct hisi_a7mapping_s {
	unsigned int a7va;
	unsigned int size;
	unsigned int prot;
	unsigned int offset;
	unsigned int reserve;
	unsigned long a7pa;
	void *apva;
};

struct hisi_atfshrdmem_s {
	u64 sec_pgt;
	unsigned int sec_mem_size;
	struct hisi_a7mapping_s a7mapping[MAXA7MAPPING];
};

struct hisi_atfisp_s {
	struct device *device;
	unsigned int boardid;
	void *atfshrd_vaddr;
	unsigned long long atfshrd_paddr;
	void *rsctable_vaddr;
	void *rsctable_vaddr_const;
	unsigned long long rsctable_paddr;
	unsigned int rsctable_offset;
	unsigned int rsctable_size;
	struct hisi_atfshrdmem_s *shrdmem;
	struct hisi_isp_ops *ispops;
	struct task_struct *secisp_kthread;
	atomic_t secisp_stop_kthread_status;
	wait_queue_head_t secisp_wait;
	bool secisp_wake;
	struct mutex pwrlock;
	struct mutex isp_iova_pool_mutex;
	unsigned long long phy_pgd_base;
	struct iommu_domain *domain;
	struct gen_pool *isp_iova_pool;
	unsigned long isp_iova_start;
	unsigned long isp_iova_size;
	unsigned int a7va_a7ispmem;
	unsigned int ispva_a7ispmem;
	void *ap_dyna_array;
	struct hisi_a7mapping_s *ap_dyna;
	int secmem_count;
	int map_req_flag;
	int map_dts_flag;
	int isp_wdt_flag;
	int isp_cpu_reset;
	int is_heap_flag;
	int sec_verify;
	int ispmem_reserved;
	unsigned int trusted_mem_size;
	unsigned int mapping_items;
	unsigned int use_smmuv3_flag;
	unsigned int tsmem_offset[ISP_TSMEM_MAX];
	unsigned int share_mem_size;
	u64 sec_boot_phymem_addr;
	u64 sec_fw_phymem_addr;
	u64 sec_smmuerr_addr;
	struct rproc_shared_para *sec_isp_share_para;
	int clk_powerby_media;
	unsigned int seckthread_nice;
	int use_ca_ta;
	secisp_mem_info ion_mem[HISP_SEC_MAX_TYPE - 1];
	secisp_mem_info static_mem[HISP_SEC_STATIC_MAX_TYPE];
	unsigned int sec_ta_enable;
} atfisp_dev;


struct mem_pool_info_list_s {
	unsigned int addr;
	unsigned int size;
	struct list_head list;
};

struct mem_pool_info_s {
	unsigned int addr;
	unsigned int size;
	unsigned int prot;
	unsigned int enable;
	unsigned int dynamic_mem_flag;
	struct mem_pool_info_list_s *node;
	struct gen_pool *isp_mem_pool;
};

struct hisi_mem_pool_s {
	unsigned int count;
	struct mutex mem_pool_mutex;
	struct mem_pool_info_s isp_pool_info[ISP_MEM_POOL_NUM];
} hisi_mem_pool_info;

struct map_type_info {
	unsigned int va;
	unsigned int size;
};

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

extern struct dsm_client *client_isp;
unsigned int map_type_info[MAP_TYPE_MAX];
extern int use_sec_isp(void);
static int smmu_err_addr_free(void);
static int secisp_boot_by_ca_ta(void);
static void *get_a7shared_va(void);

/*lint -save -e838 */
static unsigned long hisi_isp_alloc_iova(struct gen_pool *pool,
		unsigned long size, unsigned long align)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	unsigned long iova = 0;

	mutex_lock(&dev->isp_iova_pool_mutex);

	iova = gen_pool_alloc(pool, size);
	if (!iova) {
		pr_err("hisi isp iommu gen_pool_alloc failed!\n");
		mutex_unlock(&dev->isp_iova_pool_mutex);
		return 0;
	}

	if (align > ((unsigned long)1 << (unsigned long)(pool->min_alloc_order)))/*lint !e571 */
		pr_info("hisi iommu domain cant align to 0x%lx\n", align);

	mutex_unlock(&dev->isp_iova_pool_mutex);
	return iova;
}

static void hisi_isp_free_iova(struct gen_pool *pool,
		unsigned long iova, size_t size)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

	mutex_lock(&dev->isp_iova_pool_mutex);
	gen_pool_free(pool, iova, size);
	mutex_unlock(&dev->isp_iova_pool_mutex);
}
/*lint -restore */

void *getsec_a7sharedmem_addr(void)
{
	return get_a7shared_va();
}

unsigned long long get_nonsec_pgd(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

	return dev->phy_pgd_base;
}

int get_ispops(struct hisi_isp_ops **ops)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

	*ops = dev->ispops;

	return 0;
}

int hisp_sec_jpeg_powerdn(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	int ret = 0;

	if (dev->ispops == NULL) {
		pr_err("[%s] Failed : ispops.%pK\n", __func__, dev->ispops);
		return -EINVAL;
	}

	if (dev->ispops->ispexit) {
		ret = dev->ispops->ispexit();
		if (ret < 0)
			pr_err("[%s] Failed : ispexit.%d\n", __func__, ret);
	}

	if (dev->ispops->ispsrtdn) {
		ret = dev->ispops->ispsrtdn();
		if (ret < 0)
			pr_err("[%s] Failed : ispsrtdn.%d\n", __func__, ret);
	}

	return 0;
}

int hisp_a7isp_powerdn(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	int ret = 0;

	if (dev->ispops == NULL) {
		pr_err("[%s] Failed : ispops.%pK\n", __func__, dev->ispops);
		return -EINVAL;
	}

	if (dev->ispops->a7exit) {
		ret = dev->ispops->a7exit();
		if (ret < 0)
			pr_err("[%s] Failed : a7exit.%d\n", __func__, ret);
	}

	return (int)((unsigned int)ret | (unsigned int)hisp_jpeg_powerdn());
}

int hisp_sec_jpeg_powerup(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	int ret = 0;

	if (dev->ispops == NULL) {
		pr_err("[%s] Failed : ispops.%pK\n", __func__, dev->ispops);
		return -EINVAL;
	}

	if (dev->ispops->ispsrtup) {
		ret = dev->ispops->ispsrtup();
		if (ret < 0) {
			pr_err("[%s] Failed : ispsrtup.%d\n", __func__, ret);
			return ret;
		}
	}

	if (dev->ispops->ispinit) {
		ret = dev->ispops->ispinit();
		if (ret < 0) {
			pr_err("[%s] Failed : ispinit.%d\n", __func__, ret);
			goto err_jpegup;
		}
	}

	return 0;

err_jpegup:
	if (dev->ispops->ispsrtdn)
		if (dev->ispops->ispsrtdn() < 0)
			pr_err("[%s] Failed : err_jpegup ispsrtdn\n", __func__);

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

	if ((partion_name == NULL) || (buffer == NULL)) {
		pr_err("partion_name(%pK) or buffer(%pK) is null",
				partion_name, buffer);
		return -1;
	}
	/*get resource*/
	pathlen = sizeof(DEVICE_PATH) +
		strnlen(partion_name, (unsigned long)PART_NAMELEN);
	pathname = kmalloc(pathlen, GFP_KERNEL);
	if (pathname == NULL) {
		pr_err("pathname malloc failed\n");
		return -1;
	}

	ret = flash_find_ptn_s((const char *)partion_name, pathname, pathlen);
	if (ret < 0) {
		pr_err("partion_name(%s) is not in partion table!\n", partion_name);
		goto error;
	}

	fp = filp_open(pathname, O_RDONLY, 0600);
	if (IS_ERR(fp)) {
		pr_err("filp_open(%s) failed", pathname);
		goto error;
	}

	ret = vfs_llseek(fp, offset, SEEK_SET);
	if (ret < 0) {
		pr_err("seek ops failed, ret %d", ret);
		goto error2;
	}

	fs = get_fs();/*lint !e501*/
	set_fs(KERNEL_DS);/*lint !e501 */

	pos = fp->f_pos;/*lint !e613 */
	ret = vfs_read(fp, (char __user *)buffer, length, &pos);/*lint !e613 */
	if (ret != length) {
		pr_err("read ops failed, ret=%d(len=%d)", ret, length);
		set_fs(fs);
		goto error2;
	}
	set_fs(fs);

	filp_close(fp, NULL);/*lint !e668 */

	/*free resource*/
	if (pathname != NULL) {
		kfree(pathname);
		pathname = NULL;
	}

	return 0;

error2:
	filp_close(fp, NULL);/*lint !e668 */

error:
	if (pathname != NULL) {
		kfree(pathname);
		pathname = NULL;
	}

	pr_err("failed");
	return -1;
}

typedef enum {
	ISPIMG_DTS = 0,
	ISPIMG_RSC,
	ISPIMG_FW,
	ISPIMG_MAX
} ISPIMG_TYPE_E;

typedef enum {
	ISPPWR_JPEGUP = 0,
	ISPPWR_JPEGDN,
	ISPPWR_A7UP,
	ISPPWR_A7DN,
	ISPPWR_MAX
} ISPPWR_TYPE_E;

struct hisi_isp_loadimg_s {
	unsigned int addr;
	unsigned int size;
	char *partition;
} loadimg[] = {
	{0x0,   MEM_ISPDTS_SIZE,    "isp_dts"},
	{MEM_RSCTABLE_ADDR_OFFSET,   MEM_RSCTABLE_SIZE,    "isp_firmware"},
	{0x0,   MEM_ISPFW_SIZE,     "isp_firmware"}
};

int sec_process_use_ca_ta(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

	return dev->use_ca_ta;
}

static int atfisp_loadfw(void)
{
#ifdef CONFIG_LOAD_IMAGE
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	struct load_image_info loadinfo;
	int ret = 0;

	loadinfo.ecoretype      = ISP;
	loadinfo.image_addr     = loadimg[ISPIMG_FW].addr;
	loadinfo.image_size     = loadimg[ISPIMG_FW].size;
	loadinfo.partion_name   = loadimg[ISPIMG_FW].partition;

	pr_info("[%s] + %s.(0x%x), init.%x, ispmem_reserved.%x\n",
		__func__, loadinfo.partion_name, loadinfo.image_size,
		dev->ispops->refs_fw, dev->ispmem_reserved);
	if (!dev->ispmem_reserved) {
		ret = bsp_load_and_verify_image(&loadinfo);
		if (ret < 0) {
			pr_err("[%s] Failed : bsp_load_and_verify_image.%d, %s.(0x%x)\n",
				__func__, ret, loadinfo.partion_name,
				loadinfo.image_size);
			return ret;
		}
	} else {
		ret = bsp_load_sec_img(&loadinfo);
		if (ret < 0) {
			pr_err("[%s] Failed : bsp_load_sec_image.%d, %s.(0x%x)\n",
				__func__, ret, loadinfo.partion_name,
				loadinfo.image_size);
			return ret;
		}
	}

	dev->ispops->refs_fw++;

	pr_info("[%s] - bsp_load_image.%d, %s.(0x%x), init.%x, ispmem_reserved.%x\n",
		__func__, ret, loadinfo.partion_name, loadinfo.image_size,
		dev->ispops->refs_fw, dev->ispmem_reserved);
#endif
	return 0;
}

#define HISP_PARAMS_SHRDLIST    (0xCCCE0000)
#define HISP_PARAMS_DUMP        (0xCCCE0006)
#define HISP_PARAMS_NSECDUMP    (0xCCCE0007)

int sec_setclkrate(unsigned int type, unsigned int rate)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	int ret = 0;

	if (dev->ispops == NULL) {
		pr_info("[%s] Failed : dev->ispops.%pK\n",
				__func__, dev->ispops);
		return -ENXIO;
	}

	mutex_lock(&dev->pwrlock);
	if (dev->ispops->refs_fw == 0) {
		pr_info("[%s] Failed : refs_fw.%d, check ISPCPU PowerDown\n",
				__func__, dev->ispops->refs_fw);
		mutex_unlock(&dev->pwrlock);
		return -ENODEV;
	}

	ret = secnsec_setclkrate(type, rate);
	if (ret < 0)
		pr_info("[%s] Failed : secnsec_setclkrate.%d\n", __func__, ret);
	mutex_unlock(&dev->pwrlock);

	return ret;
}

static int atfisp_module_init(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

	pr_info("[%s] + refs.%d\n", __func__, dev->ispops->refs_isp_module);
	atfisp_isp_init(dev->phy_pgd_base);
	dev->ispops->refs_isp_module++;
	pr_info("[%s] - refs.%d\n", __func__, dev->ispops->refs_isp_module);

	return 0;
}

static int atfisp_module_exit(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

	pr_info("[%s] + refs.%d\n", __func__, dev->ispops->refs_isp_module);
	atfisp_isp_exit();
	dev->ispops->refs_isp_module--;
	pr_info("[%s] - refs.%d\n", __func__, dev->ispops->refs_isp_module);

	return 0;
}

static int atfa7_module_init(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	int ret = 0;

	pr_info("[%s] + refs.%d\n", __func__, dev->ispops->refs_a7_module);
	ret = atfisp_ispcpu_init();
	if (ret < 0) {
		pr_info("[%s] atfisp_ispcpu_init fail.%d\n", __func__, ret);
		return -1;
	}
	dev->ispops->refs_a7_module++;
	pr_info("[%s] - refs.%d\n", __func__, dev->ispops->refs_a7_module);

	return 0;
}

static int atfa7_module_exit(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

	pr_info("[%s] + refs.%d\n", __func__, dev->ispops->refs_a7_module);
	atfisp_ispcpu_exit();
	dev->ispops->refs_a7_module--;
	pr_info("[%s] - refs.%d\n", __func__, dev->ispops->refs_a7_module);

	return 0;
}

struct hisi_isp_ops atfisp_ops = {
	.ispsrtup   = hisp_powerup,
	.ispsrtdn   = hisp_powerdn,
	.ispinit    = atfisp_module_init,
	.ispexit    = atfisp_module_exit,
	.a7init     = atfa7_module_init,
	.a7exit     = atfa7_module_exit,
	.loadfw     = atfisp_loadfw,
	.setparams  = atfisp_setparams,
	.paramsdump = atfisp_params_dump,
};

int secisp_device_enable(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	int ret = 0;

	if (dev->secisp_kthread == NULL) {
		pr_err("[%s] Failed : secisp_kthread.%pK\n",
				__func__, dev->secisp_kthread);
		return -ENXIO;
	}

	if (!sec_process_use_ca_ta()) {
		mutex_lock(&dev->pwrlock);
		dev->secisp_wake = 1;
		mutex_unlock(&dev->pwrlock);
		wake_up(&dev->secisp_wait);
		return 0;
	}

	if (sec_process_use_ca_ta()) {
		ret = secisp_boot_by_ca_ta();
		if (ret < 0) {
			pr_err("[%s] Failed : secisp_boot_by_ca_ta. ret.%d\n",
					__func__, ret);
			return ret;
		}
	}

	return 0;
}

static int do_secisp_device_enable(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	int ret, err_ret;


	if (dev->ispops == NULL) {
		pr_err("[%s] Failed : ispops.%pK\n", __func__, dev->ispops);
		return -ENXIO;
	}

	if (dev->ispops->ispsrtup) {
		ret = dev->ispops->ispsrtup();
		if (ret < 0) {
			pr_err("[%s] Failed : ispsrtup.%d\n",
				__func__, ret);
			return ret;
		}
	}

	ret = ispcpu_qos_cfg();
	if (ret < 0) {
		pr_err("[%s] Failed : ispcpu_qos_cfg.%d\n", __func__, ret);
		goto err_ispinit;
	}

	if (!sec_process_use_ca_ta()) {
		if (dev->ispops->ispinit) {
			ret = dev->ispops->ispinit();
			if (ret < 0) {
				pr_err("[%s] Failed : atfisp_module_init.%d\n",
						__func__, ret);
				goto err_ispinit;
			}
		}

		if (dev->ispops->a7init) {
			ret = dev->ispops->a7init();
			if (ret < 0) {
				pr_err("[%s] Failed : atfa7_module_init.%d\n",
						__func__, ret);
				goto err_a7init;
			}
		}

		if (dev->ispops->loadfw) {
			ret = dev->ispops->loadfw();
			if (ret < 0) {
				pr_err("[%s] Failed : atfisp_loadfw.%d\n",
						__func__, ret);
				goto err_loadfw;
			}
		}
	}

	return 0;

err_loadfw:
	if (!sec_process_use_ca_ta()) {
		if (dev->ispops->a7exit) {
			err_ret = dev->ispops->a7exit();
			if (err_ret < 0)
				pr_err("[%s] Failed : a7exit.%d\n",
					__func__, err_ret);
		}
	}
err_a7init:
	if (!sec_process_use_ca_ta()) {
		if (dev->ispops->ispexit) {
			err_ret = dev->ispops->ispexit();
			if (err_ret < 0)
				pr_err("[%s] Failed : ispexit.%d\n",
					__func__, err_ret);
		}
	}
err_ispinit:
	if (dev->ispops->ispsrtdn) {
		err_ret = dev->ispops->ispsrtdn();
		if (err_ret < 0)
			pr_err("[%s] Failed : ispsrtdn.%d\n",
				__func__, err_ret);
	}

	return ret;
}

int secisp_device_disable(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	struct cpumask cpu_mask;
	int ret, cpu_no;

	if (dev->ispops == NULL) {
		pr_err("[%s] Failed : ispops.%pK\n", __func__, dev->ispops);
		return -ENXIO;
	}

	if (!sec_process_use_ca_ta()) {
		cpumask_clear(&cpu_mask);
		for (cpu_no = 1; cpu_no < 4; cpu_no++)
			cpumask_set_cpu(cpu_no, &cpu_mask);

		if (sched_setaffinity(current->pid, &cpu_mask) < 0)
			pr_err("%s: Couldn't set affinity to cpu\n", __func__);
	} else {
		ret = teek_secisp_reset();
		if (ret < 0)
			pr_err("[%s] Failed : teek_secisp_reset.%d.\n",
					__func__, ret);
	}

	mutex_lock(&dev->pwrlock);
	if (!sec_process_use_ca_ta()) {
		if (dev->ispops->a7exit) {
			ret = dev->ispops->a7exit();
			if (ret < 0)
				pr_err("[%s] Failed : a7exit.%d\n", __func__, ret);
		}

		if (dev->ispops->ispexit) {
			ret = dev->ispops->ispexit();
			if (ret < 0)
				pr_err("[%s] Failed : ispexit.%d\n", __func__, ret);
		}
	}

	if (dev->ispops->ispsrtdn) {
		ret = dev->ispops->ispsrtdn();
		if (ret < 0)
			pr_err("[%s] Failed : ispsrtdn.%d\n", __func__, ret);
	}

	dev->ispops->refs_fw--;
	if (!sec_process_use_ca_ta())
		atomic_set(&dev->secisp_stop_kthread_status, 1);

	mutex_unlock(&dev->pwrlock);

	smmu_err_addr_free();

	if (sync_isplogcat() < 0)
		pr_err("[%s] Failed: sync_isplogcat\n", __func__);

	return 0;
}

static unsigned long get_a7shared_pa(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

	return dev->atfshrd_paddr + dev->tsmem_offset[ISP_SHARE_MEM_OFFSET];
}

static void *get_a7shared_va(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

	return dev->atfshrd_vaddr + dev->tsmem_offset[ISP_SHARE_MEM_OFFSET];
}

static unsigned long hisp_getreservemem(unsigned int etype, unsigned long paddr)
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
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

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
			dev->shrdmem->a7mapping[etype].a7pa = hisp_getreservemem(etype, paddr);

		pr_info("[%s] type.%d, a7va.0x%x\n", __func__,
			etype, dev->shrdmem->a7mapping[etype].a7va);
	} else {
		if (etype >= HISP_SEC_STATIC_MAX_TYPE) {
			pr_err("[%s] Failed : etype.(0x%x >= 0x%x)\n",
				__func__, etype, HISP_SEC_STATIC_MAX_TYPE);
			return -EINVAL;
		}

		dev->static_mem[etype].pa = hisp_getreservemem(etype, paddr);
		pr_info("[%s] type.%d, pa.0x%llx\n", __func__,
				etype, dev->static_mem[etype].pa);
	}

	return 0;
}

static int hisp_meminit(unsigned int etype, unsigned long paddr)
{
	const char *propname[] = {
		"a7-vaddr-boot",
		"a7-vaddr-text",
		"a7-vaddr-data",
		"a7-vaddr-pgd",
		"a7-vaddr-pmd",
		"a7-vaddr-pte",
		"a7-vaddr-rdr",
		"a7-vaddr-shrd",
		"a7-vaddr-vq",
		"a7-vaddr-vr0",
		"a7-vaddr-vr1",
		"a7-vaddr-heap",
		"a7-vaddr-a7dyna",
		"a7-vaddr-regisp",
		"a7-vaddr-regipcs",
		"a7-vaddr-regipcns",
		"a7-vaddr-regpctrl",
		"a7-vaddr-regsctrl",
		"a7-vaddr-regpcfg",
		"a7-vaddr-reggic",
		"a7-vaddr-regsysc",
		"a7-vaddr-reguart",
		"a7-vaddr-reggpio",
		"a7-vaddr-reggpio25",
		"a7-vaddr-regioc",
		"a7-vaddr-mdc",
	};
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	struct device_node *np = dev->device->of_node;
	unsigned int offset = 0;
	int ret = 0;

	if (np == NULL) {
		pr_err("[%s] Failed : np.%pK\n", __func__, np);
		return -EINVAL;
	}

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

	ret = of_property_read_u32_array(np, propname[etype],
					(unsigned int *)(&dev->shrdmem->a7mapping[etype]),
					dev->mapping_items);
	if (ret < 0) {
		pr_info("[%s] propname.%s, of_property_read_u32_array.%d\n",
				__func__, propname[etype], ret);
		return -EINVAL;
	}
	pr_info("[%s] propname.%s, of_property_read_u32_array.%d.(0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n",
		__func__, propname[etype], ret, dev->shrdmem->a7mapping[etype].a7va,
		dev->shrdmem->a7mapping[etype].size, dev->shrdmem->a7mapping[etype].prot,
		dev->shrdmem->a7mapping[etype].offset, dev->shrdmem->a7mapping[etype].reserve);

	if (dev->shrdmem->a7mapping[etype].reserve)
		dev->shrdmem->a7mapping[etype].a7pa = hisp_getreservemem(etype, paddr);

	if (etype == A7DYNAMIC) {
		offset = dev->tsmem_offset[ISP_DYN_MEM_OFFSET];
		dev->shrdmem->a7mapping[etype].a7pa = dev->atfshrd_paddr + offset;
		dev->ap_dyna_array = offset + dev->atfshrd_vaddr;
		dev->ap_dyna = &dev->shrdmem->a7mapping[etype];
		pr_info("[%s] etype.%d, ap_dyna.%pK, atfshrd_vaddr.%pK, offset.0x%x\n",
				__func__, etype, dev->ap_dyna_array,
				dev->atfshrd_vaddr, offset);
	} else if (etype == A7HEAP) {
		dev->is_heap_flag = dev->shrdmem->a7mapping[etype].reserve;
	} else if (etype == A7SHARED) {
		dev->shrdmem->a7mapping[etype].apva = get_a7shared_va();
	}

	return 0;
}

static int hisp_ca_ion_meminit(unsigned int etype)
{
	const char *propname[] = {
		"ispcpu-vaddr-pgtb",
		"ispcpu-vaddr-text",
		"ispcpu-vaddr-data",
		"ispcpu-ispsec-pool",
		"ispcpu-sec-pool",
		"ispcpu-dynamic-pool",
		"ipp-sec-pool",
	};
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	struct device_node *np = dev->device->of_node;
	int ret = 0;

	if (np == NULL) {
		pr_err("[%s] Failed : np.%pK\n", __func__, np);
		return -EINVAL;
	}

	if (etype >= (HISP_SEC_MAX_TYPE - 1)) {
		pr_err("[%s] Failed : etype.(0x%x >= 0x%x)\n",
			__func__, etype, HISP_SEC_MAX_TYPE - 1);
		return -EINVAL;
	}

	ret = of_property_read_u32_array(np, propname[etype],
		(unsigned int *)(&dev->ion_mem[etype]), dev->mapping_items);
	if (ret < 0) {
		pr_info("[%s] propname.%s, of_property_read_u32_array.%d\n",
				__func__, propname[etype], ret);
		return -EINVAL;
	}
	pr_info("[%s] propname.%s, of_property_read_u32_array.%d.(0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n",
			__func__, propname[etype], ret, dev->ion_mem[etype].type,
			dev->ion_mem[etype].da, dev->ion_mem[etype].size,
			dev->ion_mem[etype].prot, dev->ion_mem[etype].sec_flag);

	return 0;
}

static int hisp_ca_static_meminit(unsigned int etype)
{
	const char *propname[] = {
		"ispcpu-vaddr-vr0",
		"ispcpu-vaddr-vr1",
		"ispcpu-vaddr-vq",
		"ispcpu-vaddr-shrd",
		"ispcpu-vaddr-rdr",
	};
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	struct device_node *np = dev->device->of_node;
	int ret = 0;

	if (np == NULL) {
		pr_err("[%s] Failed : np.%pK\n", __func__, np);
		return -EINVAL;
	}

	if (etype >= HISP_SEC_STATIC_MAX_TYPE) {
		pr_err("[%s] Failed : etype.(0x%x >= 0x%x)\n",
			__func__, etype, HISP_SEC_STATIC_MAX_TYPE);
		return -EINVAL;
	}

	ret = of_property_read_u32_array(np, propname[etype],
					(unsigned int *)(&dev->static_mem[etype]),
					dev->mapping_items);
	if (ret < 0) {
		pr_info("[%s] propname.%s, of_property_read_u32_array.%d\n",
				__func__, propname[etype], ret);
		return -EINVAL;
	}

	pr_info("[%s] propname.%s, of_property_read_u32_array.%d.(0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n",
			__func__, propname[etype], ret, dev->static_mem[etype].type,
			dev->static_mem[etype].da, dev->static_mem[etype].size,
			dev->static_mem[etype].prot, dev->static_mem[etype].sec_flag);

	return 0;
}


int hisp_apisp_map(unsigned int *a7addr,
		unsigned int *ispaddr, unsigned int size)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

	if ((dev->a7va_a7ispmem == 0) || (dev->ispva_a7ispmem == 0))
		return -EINVAL;

	*a7addr = dev->a7va_a7ispmem;
	*ispaddr = dev->ispva_a7ispmem;
	pr_info("[%s] X a7va.(0x%x, 0x%x), ispva.(0x%x, 0x%x)\n",
			__func__, *a7addr, dev->a7va_a7ispmem,
			*ispaddr, dev->ispva_a7ispmem);

	return 0;
}
EXPORT_SYMBOL(hisp_apisp_map);

int hisp_apisp_unmap(void)
{
	return 0;
}
EXPORT_SYMBOL(hisp_apisp_unmap);

static int hisp_rsctablemem_init(struct hisi_atfisp_s *dev)
{
	dma_addr_t dma_addr = 0;

	dev->rsctable_vaddr_const = dma_alloc_coherent(dev->device,
				dev->rsctable_size, &dma_addr, GFP_KERNEL);
	if (dev->rsctable_vaddr_const == NULL) {
		pr_err("[%s] rsctable_vaddr_const.%pK\n",
				__func__, dev->rsctable_vaddr_const);
		return -ENOMEM;
	}
	dev->rsctable_paddr = (unsigned long long)dma_addr;

	return 0;
}

static void hisp_rsctablemem_exit(struct hisi_atfisp_s *dev)
{
	if (dev->rsctable_vaddr_const != NULL)
		dma_free_coherent(dev->device, dev->rsctable_size,
			dev->rsctable_vaddr_const, dev->rsctable_paddr);
	dev->rsctable_vaddr_const = NULL;
	dev->rsctable_paddr = 0;
}

static u64 hisp_nesc_cpucfg_param_paddr(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

	return (u64)(dev->atfshrd_paddr + dev->tsmem_offset[ISP_CPU_PARM_OFFSET]);
}

static struct phy_csi_info_t *hisp_phy_csi_info_vaddr(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

	if (dev->atfshrd_vaddr != NULL)
		return (struct phy_csi_info_t *)
		       (dev->atfshrd_vaddr +
		       dev->tsmem_offset[ISP_CPU_PARM_OFFSET]);

	return NULL;
}


int hisp_phy_csi_connect(struct hisp_phy_info_t *phy_info,
				  unsigned int csi_index)
{
	int ret = 0;
	u64 sharepa = 0;
	struct phy_csi_info_t *shareva = NULL;
	struct phy_csi_info_t phy_csi_info;

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
		pr_err("[%s] wrong para: csi_index = %u!\n", __func__, csi_index);
		return -EINVAL;
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
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	int ret = 0;

	if (dev->isp_cpu_reset)
		set_ispcpu_idle();

	if (isppd_adb_flag) {
		ret = atfisp_get_ispcpu_idle();
		if (ret != 0) {
			pr_alert("[%s] Failed : ispcpu not in idle.%d!\n",
					__func__, ret);
			dump_media1_regs();
		}
	}

	return ret;
}

struct hisi_nsec_cpu_dump_s *get_debug_ispcpu_param(void)
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
	if(ret < 0){
		pr_err("[%s] atfisp_send_fiq2ispcpu.%d\n", __func__, ret);
		return -1;
	}

	pr_info("[%s] -\n", __func__);
	return 0;
}

static int hisp_sharedmem_init(struct hisi_atfisp_s *dev)
{
	dma_addr_t dma_addr = 0;

	dev->atfshrd_vaddr = hisi_fstcma_alloc(&dma_addr,
				dev->share_mem_size, GFP_KERNEL);
	if (dev->atfshrd_vaddr == NULL) {
		pr_err("[%s] atfshrd_vaddr.%pK\n",
			__func__, dev->atfshrd_vaddr);
		return -ENOMEM;
	}

	dev->atfshrd_paddr = (unsigned long long)dma_addr;
	dev->shrdmem = (struct hisi_atfshrdmem_s *)dev->atfshrd_vaddr;

	return 0;
}

static void hisp_sharedmem_exit(struct hisi_atfisp_s *dev)
{
	if (dev->atfshrd_vaddr != NULL)
		hisi_fstcma_free(dev->atfshrd_vaddr,
			dev->atfshrd_paddr, dev->share_mem_size);
	dev->atfshrd_paddr = 0;
	dev->atfshrd_vaddr = NULL;
}

static int trusted_ispmem_reserved(int *flag)
{
	struct device_node *nod = NULL;
	const char *status = NULL;

	if (flag == NULL) {
		pr_err("[%s] input flag parameter is NULL!\n", __func__);
		return -ENODEV;
	}
	*flag = 0;

	nod = of_find_node_by_path("/reserved-memory/sec_camera");
	if (nod  == NULL) {/*lint !e838 */
		pr_err("[%s] Failed : of_find_node_by_path.%pK\n",
				__func__, nod);
		return -ENODEV;
	}

	if (of_property_read_string(nod, "status", &status)) {
		pr_err("[%s] Failed : of_property_read_string status\n",
				__func__);
		*flag = 1;
		return -ENODEV;
	}

	if (status && strncmp(status, "disabled", strlen("disabled")) != 0)
		*flag = 1;

	pr_err("[%s] trusted_ispmem_reserved %s\n", __func__, status);
	return 0;
}

static int hisi_truset_mem_getdts(struct device_node *np,
					    struct hisi_atfisp_s *dev)
{
	unsigned int offset_num   = 0;
	unsigned int offset_index = 0;
	int ret = 0;

	if (!sec_process_use_ca_ta()) {
		ret = of_property_read_u32(np, "trusted-smem-size",
				(unsigned int *)(&dev->trusted_mem_size));
		if (ret < 0) {
			pr_err("[%s] Failed: trusted_mem_size.ret.%d\n",
					__func__, ret);
			return -EINVAL;
		}
		pr_info("trusted_mem_size.0x%x of_property_read_u32.%d\n",
				dev->trusted_mem_size, ret);
	}

	ret = of_property_read_u32(np, "share-smem-size",
			(unsigned int *)(&dev->share_mem_size));
	if (ret < 0) {
		pr_err("[%s] Failed: share_mem_size.ret.%d\n", __func__, ret);
		return -EINVAL;
	}
	pr_info("share_mem_size.0x%x of_property_read_u32.%d\n",
			dev->share_mem_size, ret);

	ret = of_property_read_u32(np, "rsctable-mem-offet",
			(unsigned int *)(&dev->rsctable_offset));
	if (ret < 0) {
		pr_err("[%s] Failed: rsctable_offset.ret.%d\n", __func__, ret);
		return -EINVAL;
	}
	pr_info("rsctable_offset.0x%x of_property_read_u32.%d\n",
			dev->rsctable_offset, ret);

	ret = of_property_read_u32(np, "rsctable-mem-size",
			(unsigned int *)(&dev->rsctable_size));
	if (ret < 0) {
		pr_err("[%s] Failed: rsctable_size.ret.%d\n", __func__, ret);
		return -EINVAL;
	}
	pr_info("rsctable_size.0x%x of_property_read_u32.%d\n",
			dev->rsctable_size, ret);

	ret = of_property_read_u32(np, "trusted-smem-num",
			(unsigned int *)(&offset_num));
	if (ret < 0) {
		pr_err("[%s] Failed: tsmem-offset-num.ret.%d\n", __func__, ret);
		return -EINVAL;
	}
	pr_info("tsmem-offset-num.0x%x of_property_read_u32.%d\n",
			offset_num, ret);

	if (offset_num <= ISP_TSMEM_MAX) {
		ret = of_property_read_u32_array(np, "trusted-smem-offset",
					dev->tsmem_offset, offset_num);
		if (ret < 0) {
			pr_info("[%s] Fail : trusted-smem-offset.ret.%d\n",
					__func__, ret);
			return -EINVAL;
		}

		for (offset_index = 0; offset_index < offset_num; offset_index++)
			pr_info("[%s] trusted-smem-offset %d offest = 0x%x\n",
					__func__, offset_index,
					dev->tsmem_offset[offset_index]);
	}
	return 0;
}

static int hisi_atf_getdts(struct platform_device *pdev)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	struct device *device = &pdev->dev;
	struct device_node *np = device->of_node;
	int ret = 0;

	ret = trusted_ispmem_reserved(&dev->ispmem_reserved);
	if (ret < 0)
		pr_err("[%s] Failed : ispmem_reserved.%d.\n", __func__, ret);
	pr_info("[%s] ispmem_reserved.%d\n", __func__, dev->ispmem_reserved);

	if (np == NULL) {
		pr_err("[%s] Failed : np.%pK\n", __func__, np);
		return -ENODEV;
	}

	ret = of_property_read_u32(np, "use-ca-ta", &dev->use_ca_ta);
	if (ret < 0) {
		pr_err("Failed : use_ca_ta.%d.ret.%d\n", dev->use_ca_ta, ret);
		return -EINVAL;
	}
	pr_info("use_ca_ta.%d of_property_read_u32.%d\n",
			dev->use_ca_ta, ret);

	if (!sec_process_use_ca_ta()) {
		ret = of_property_read_u32(np, "isp-iova-start",
				(unsigned int *)(&dev->isp_iova_start));
		if (ret < 0) {
		    pr_err("Failed : isp-iova_addr.0x%lx.ret.%d\n",
				dev->isp_iova_start, ret);
		    return -EINVAL;
		}
		pr_info("isp-iova_addr.0x%lx of_property_read_u32.%d\n",
				dev->isp_iova_start, ret);
	}

	ret = of_property_read_u32(np, "isp-wdt-flag",
			(unsigned int *)(&dev->isp_wdt_flag));
	if (ret < 0) {
		pr_err("[%s] Failed: isp-wdt-flag.ret.%d\n", __func__, ret);
		return -EINVAL;
	}
	pr_info("isp-wdt-flag.0x%x of_property_read_u32.%d\n",
			dev->isp_wdt_flag, ret);

	ret = of_property_read_u32(np, "isp-reset-flag",
			(unsigned int *)(&dev->isp_cpu_reset));
	if (ret < 0) {
		pr_err("[%s] Failed: isp-reset-flag.ret.%d\n", __func__, ret);
		return -EINVAL;
	}
	pr_info("isp-reset-flag.0x%x of_property_read_u32.%d\n",
			dev->isp_cpu_reset, ret);

	ret = of_property_read_u32(np, "sec-verify",
			(unsigned int *)(&dev->sec_verify));
	if (ret < 0) {
		pr_err("[%s] Failed: sec-verify.ret.%d\n", __func__, ret);
		return -EINVAL;
	}
	pr_info("sec-verify.0x%x of_property_read_u32.%d\n",
			dev->sec_verify, ret);

	ret = of_property_read_u32(np, "mapping-items",
			(unsigned int *)(&dev->mapping_items));
	if (ret < 0) {
		pr_err("[%s] Failed: mapping-num.ret.%d\n", __func__, ret);
		return -EINVAL;
	}
	pr_info("mapping-items.0x%x.ret.%d\n", dev->mapping_items, ret);

	ret = of_property_read_u32(np, "use-smmuv3-flag",
			(unsigned int *)(&dev->use_smmuv3_flag));
	if (ret < 0) {
		pr_err("[%s] Failed: use-smmuv3-flag.%d\n", __func__, ret);
		return -EINVAL;
	}
	pr_info("use-smmuv3-flag.0x%x of_property_read_u32.%d\n",
			dev->use_smmuv3_flag, ret);

	ret = hisi_truset_mem_getdts(np, dev);
	if (ret < 0) {
		pr_err("Failed : hisi_truset_mem_getdts.%d\n", ret);
		return ret;
	}

	ret = of_property_read_u32(np, "isp-seckthread-nice",
				(unsigned int *)(&dev->seckthread_nice));
	if (ret < 0) {
		pr_err("[%s] Failed: isp-seckthread-nice.ret.%d\n", __func__, ret);
		return -EINVAL;
	}
	pr_info("isp-seckthread-nice.0x%x of_property_read_u32.%d\n",
			dev->seckthread_nice, ret);

	return 0;
}

void *get_rsctable(int *tablesz)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

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
			if (last_len != 0)
				pr_info("[%s] list.(%pK + %pK), maplist.(0x%x X 0x%x)\n",
					__func__, listmem, maplist, last_counts, last_len);
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
/*lint -save -e838 */
unsigned int a7_mmu_map(struct scatterlist *sgl, unsigned int size,
			unsigned int prot, unsigned int flag)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	unsigned int map_size = 0;
	unsigned int map_prot;
	struct hisi_a7mapping_s *map_info = dev->ap_dyna;
	void *addr = dev->ap_dyna_array;
	unsigned long iova;
	int ret = 0;

	if (size == 0) {
		pr_err("%s: invalid para, size == 0x%x.\n", __func__, size);
		return 0;
	}

	map_size = PAGE_ALIGN(size);

	if (prot == 0) {
		map_prot = map_info->prot;
	} else {
		map_prot = prot;
		map_info->prot = map_prot;
	}

	switch (flag) {
	case MAP_TYPE_DYNAMIC:
		iova = hisi_isp_alloc_iova(dev->isp_iova_pool,
					(unsigned long)map_size, 0);
		if (iova == 0) {
			pr_err("[%s] Failed : hisi_isp_alloc_iova iova.0x%lx\n",
					__func__, iova);
			return 0;
		}

		map_info->a7va = (unsigned int)iova;
		map_info->size = map_size;

		break;
	case MAP_TYPE_STATIC:
		map_info->a7va = A7_MAP_BUFFER_ADDR;
		map_info->size = size;

		if (size > A7_MAP_BUFFER_SIZE) {
			pr_err("%s: invalid para, size.0x%x.\n", __func__, size);
			return 0;
		}

		break;
	case MAP_TYPE_STATIC_SEC:
		map_info->a7va = IRIS_MAP_BUFFER_ADDR;
		map_info->size = size;

		if (size > IRIS_MAP_BUFFER_SIZE) {
			pr_err("%s: invalid para, size.0x%x.\n", __func__, size);
			return 0;
		}

		break;
	case MAP_TYPE_COREDUMP:
		map_info->a7va = ISPCPU_COREDUMP_ADDR;
		map_info->size = size;
		if (size > ISPCPU_COREDUMP_SIZE) {
			pr_err("%s: invalid para, size.0x%x.\n", __func__, size);
			return 0;
		}

		break;
	default:
		pr_err("[%s] Failed : type.0x%x\n", __func__, flag);
		return 0;
	}
	pr_info("[%s] type.0x%x,a7va.0x%x,size.0x%x,map_size.0x%x,map_prot.0x%x\n",
			__func__, flag, map_info->a7va, size, map_size, map_prot);
	a7_map_set_pa_list(addr, sgl, map_size);
	ret = atfisp_ispcpu_map();
	if (ret < 0) {
		pr_err("%s: atfisp_ispcpu_map fail .%d\n", __func__, ret);
		return 0;
	}

	return map_info->a7va;
}

void a7_mmu_unmap(unsigned int va, unsigned int size)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	struct hisi_a7mapping_s *map_info = dev->ap_dyna;
	unsigned int map_size;

	map_size = PAGE_ALIGN(size);/*lint !e50 */
	pr_info("%s: va = 0x%x, size = 0x%x, map_size = 0x%x\n",
			__func__, va, size, map_size);
	map_info->a7va = va;
	map_info->size = map_size;

	atfisp_ispcpu_unmap();

	if ((dev->isp_iova_start <= map_info->a7va) &&
	    (map_info->a7va < (dev->isp_iova_start + dev->isp_iova_size))) {
		pr_info("[%s] map_info->a7va.0x%x map_info->size.0x%x\n",
				__func__, map_info->a7va, map_info->size);
		hisi_isp_free_iova(dev->isp_iova_pool,
				   (unsigned long)map_info->a7va,
				   (unsigned long)map_info->size);
	}
}

static unsigned int get_size_align_mask(unsigned int align)
{
	unsigned int mask = 0xFFFFFFFF;
	unsigned int mask_num = 0;

	for (mask_num = 0; mask_num < 32; mask_num++) {
		if ((0x1 & (align >> mask_num)) == 1)
			return mask;

		mask &= ~(1 << mask_num);/*lint !e747 !e701 !e502 */
	}

	return 0;
}

static unsigned long mem_pool_alloc_iova(struct gen_pool *pool,
		unsigned int size, unsigned long align)
{
	struct hisi_mem_pool_s *dev = &hisi_mem_pool_info;
	unsigned long iova = 0;

	if (pool == NULL)
		return 0;

	mutex_lock(&dev->mem_pool_mutex);

	iova = gen_pool_alloc(pool, (unsigned long)size);
	if (iova == 0) {
		pr_err("hisi isp iommu gen_pool_alloc failed!\n");
		mutex_unlock(&dev->mem_pool_mutex);
		return 0;
	}

	if (align > (1 << (unsigned long)(pool->min_alloc_order)))/*lint !e571 */
		pr_info("hisi iommu domain can't align to 0x%lx\n", align);
	mutex_unlock(&dev->mem_pool_mutex);
	return iova;
}

static void mem_pool_free_iova(struct gen_pool *pool,
		unsigned long iova, size_t size)
{
	struct hisi_mem_pool_s *dev = &hisi_mem_pool_info;

	mutex_lock(&dev->mem_pool_mutex);
	gen_pool_free(pool, iova, size);
	mutex_unlock(&dev->mem_pool_mutex);
}

static int sec_mem_pool_destroy(unsigned int type)
{
	struct hisi_atfisp_s *atf_dev  = (struct hisi_atfisp_s *)&atfisp_dev;
	int ret = 0;

	if (type == HISP_DYNAMIC_SEC) {
		ret = teek_secisp_dynamic_mem_unmap(&atf_dev->ion_mem[type],
						    atf_dev->device);
		if (ret < 0) {
			pr_err("%s: teek_secisp_dynamic_mem_unmap fail type.%u ret.%d\n",
					__func__, type, ret);
			return ret;
		}
	} else {
		ret = teek_secisp_mem_unmap(&atf_dev->ion_mem[type]);
		if (ret < 0) {
			pr_err("%s: teek_secisp_mem_unmap fail type.%u ret.%d\n",
					__func__, type, ret);
			return ret;
		}
	}

	return 0;
}

static void mem_pool_destroy(unsigned int pool_num)
{
	struct hisi_mem_pool_s *dev = &hisi_mem_pool_info;
	struct mem_pool_info_s *pool_info = &dev->isp_pool_info[pool_num];
	struct mem_pool_info_list_s *listnode = pool_info->node;
	struct mem_pool_info_list_s *node = NULL;
	struct mem_pool_info_list_s *node_temp = NULL;
	struct hisi_atfisp_s *atf_dev  = NULL;
	struct hisi_a7mapping_s *map_info = NULL;
	int ret = 0;

	mutex_lock(&dev->mem_pool_mutex);
	if (pool_info->enable == 0) {
		pr_err("%s: the pool_num %d not creat or had been disable\n",
				__func__, pool_num);
		mutex_unlock(&dev->mem_pool_mutex);
		return;
	}

	if (use_sec_isp()) {
		if (!sec_process_use_ca_ta()) {
			atf_dev = (struct hisi_atfisp_s *)&atfisp_dev;
			map_info = atf_dev->ap_dyna;

			map_info->a7va = pool_info->addr;
			map_info->size = pool_info->size;

			atfisp_ispcpu_unmap();
		}
	}

	if (pool_info->dynamic_mem_flag) {
		ret = dynamic_memory_unmap(pool_info->addr,
					(size_t)pool_info->size);
		if (ret < 0) {
			pr_err("%s: dynamic_memory_unmap error!\n", __func__);
			mutex_unlock(&dev->mem_pool_mutex);
			return;
		}

		pool_info->dynamic_mem_flag = 0;
	}

	list_for_each_entry_safe(node, node_temp, &listnode->list, list) {
		mutex_unlock(&dev->mem_pool_mutex);
		mem_pool_free_iova(pool_info->isp_mem_pool,
			(unsigned long)node->addr, (unsigned long)node->size);
		mutex_lock(&dev->mem_pool_mutex);
		list_del(&node->list);
		pr_info("%s: va = 0x%x, size = 0x%x\n",
				__func__, node->addr, node->size);
		kfree(node);
	}

	kfree(listnode);

	if (dev->count > 0)
		dev->count--;
	else
		pr_err("%s: pool num = %d, pool count.%d\n", __func__, pool_num, dev->count);

	gen_pool_destroy(pool_info->isp_mem_pool);
	pool_info->enable = 0;
	pr_info("%s: pool num = %d, pool count.%d\n",
			__func__, pool_num, dev->count);
	mutex_unlock(&dev->mem_pool_mutex);
}

static unsigned int check_mem_pool_enable(unsigned int pool_num)
{
	struct hisi_mem_pool_s *dev = &hisi_mem_pool_info;
	struct mem_pool_info_s *pool_info = &dev->isp_pool_info[pool_num];

	if (pool_info != NULL)
		return pool_info->enable;
	else
		return 1;
}

static unsigned int sec_mem_pool_setup(unsigned int type,
					       unsigned int pool_num,
					       unsigned int align,
					       unsigned int prot)
{
	struct hisi_mem_pool_s *dev = &hisi_mem_pool_info;
	struct mem_pool_info_s *pool_info = &dev->isp_pool_info[pool_num];
	struct mem_pool_info_list_s *listnode = NULL;
	struct gen_pool *pool = NULL;
	struct hisi_atfisp_s *atf_dev = (struct hisi_atfisp_s *)&atfisp_dev;
	unsigned int pool_addr = 0;
	unsigned int pool_size = 0;
	unsigned int size = atf_dev->ion_mem[type].size;
	int ret = 0;

	mutex_lock(&dev->mem_pool_mutex);
	dev->count++;
	if (dev->count > ISP_MEM_POOL_NUM) {
		pr_err("Can't support so many pools!\n");
		mutex_unlock(&dev->mem_pool_mutex);
		return 0;
	}

	if (pool_info->enable == 1) {
		pr_err("%s: %d mem pool had ben enabled!\n", __func__, pool_num);
		mutex_unlock(&dev->mem_pool_mutex);
		return 0;
	}

	ret = memset_s(pool_info, sizeof(struct mem_pool_info_s),
		       0, sizeof(struct mem_pool_info_s));/*lint !e838 */
	if (ret < 0) {
		pr_err("%s: %d mem pool memset fail!\n", __func__, pool_num);
		mutex_unlock(&dev->mem_pool_mutex);
		return 0;
	}

	pool_size = PAGE_ALIGN(size);/*lint !e50 */
	pool = gen_pool_create((int)(order_base_2(ISP_MEM_POOL_ALIGN)), -1);
	if (pool == NULL) {
		pr_err("Create isp gen pool failed!\n");
		mutex_unlock(&dev->mem_pool_mutex);
		return 0;
	}

	pr_debug("[%s] pool_num.%d, size.0x%x, pool_size.0x%x\n",
			__func__, pool_num, size, pool_size);

	atf_dev->ion_mem[type].size = pool_size;
	atf_dev->ion_mem[type].prot = prot;

	if (type == HISP_DYNAMIC_SEC) {
		ret = teek_secisp_dynamic_mem_map(
			&atf_dev->ion_mem[type], &pool_addr, atf_dev->device);
		if (ret < 0) {
			pr_err("%s: teek_secisp_dynamic_mem_map fail.%d\n",
					__func__, ret);
			mutex_unlock(&dev->mem_pool_mutex);
			BUG(); /*lint !e146*/
			return 0;
		}
	} else {
		ret = teek_secisp_mem_map(&atf_dev->ion_mem[type], &pool_addr);
		if (ret < 0) {
			pr_err("%s: teek_secisp_mem_map fail.%d\n",
					__func__, ret);
			mutex_unlock(&dev->mem_pool_mutex);
			BUG(); /*lint !e146*/
			return 0;
		}
	}

	pool_addr = atf_dev->ion_mem[type].da;
	ret = gen_pool_add(pool, (unsigned long)pool_addr,
			  (unsigned long)pool_size, -1);
	if (ret) {
		pr_err("Gen pool add failed!\n");
		if (type == HISP_DYNAMIC_SEC)
			ret = teek_secisp_dynamic_mem_unmap(
				&atf_dev->ion_mem[type], atf_dev->device);
		else
			ret = teek_secisp_mem_unmap(&atf_dev->ion_mem[type]);
		gen_pool_destroy(pool);
		mutex_unlock(&dev->mem_pool_mutex);
		return 0;
	}

	listnode = kzalloc(sizeof(struct mem_pool_info_list_s), GFP_KERNEL);
	if (listnode == NULL) {
		pr_err("%s: alloc listnode fail\n", __func__);
		gen_pool_free(pool, (unsigned long)pool_addr, size);
		if (type == HISP_DYNAMIC_SEC)
			ret = teek_secisp_dynamic_mem_unmap(
				&atf_dev->ion_mem[type], atf_dev->device);
		else
			ret = teek_secisp_mem_unmap(&atf_dev->ion_mem[type]);
		gen_pool_destroy(pool);
		mutex_unlock(&dev->mem_pool_mutex);
		return 0;
	}

	listnode->addr = 0x0;
	listnode->size = 0x0;
	INIT_LIST_HEAD(&listnode->list);
	pool_info->node = listnode;
	pool_info->addr = pool_addr;
	pool_info->size = pool_size;
	pool_info->prot = atf_dev->ion_mem[type].prot;
	pool_info->isp_mem_pool = pool;
	pool_info->enable = 1;
	pool_info->dynamic_mem_flag = 0;
	pr_debug("[%s] pool_num.%d, isp_mem_pool.0x%pK, pool.addr.0x%x, size.0x%x\n",
			__func__, pool_num, pool, pool_addr, pool_size);
	pr_info("%s: pool num = %d, pool count.%d, pool.addr.0x%x, size.0x%x\n",
			__func__, pool_num, dev->count, pool_addr, pool_size);
	mutex_unlock(&dev->mem_pool_mutex);
	return pool_addr;
}

static unsigned int mem_pool_setup(struct scatterlist *sgl,
					 unsigned int iova,
					 unsigned int size,
					 unsigned int prot,
					 unsigned int pool_num,
					 unsigned int align,
					 unsigned int dynamic_mem_flag)
{
	struct hisi_mem_pool_s *dev = &hisi_mem_pool_info;
	struct mem_pool_info_s *pool_info = &dev->isp_pool_info[pool_num];
	struct mem_pool_info_list_s *listnode = NULL;
	struct gen_pool *pool = NULL;
	struct hisi_atfisp_s *atf_dev = NULL;
	struct hisi_a7mapping_s *map_info = NULL;
	unsigned int pool_addr = 0;
	unsigned int pool_size = 0;
	unsigned int map_prot = 0;
	unsigned int map_size = 0;
	unsigned int align_mask = 0;
	void *addr = NULL;
	int ret = 0;

	mutex_lock(&dev->mem_pool_mutex);
	dev->count++;
	if (dev->count > ISP_MEM_POOL_NUM) {
		pr_err("Can't support so many pools!\n");
		mutex_unlock(&dev->mem_pool_mutex);
		return 0;
	}

	if (pool_info->enable == 1) {
		pr_err("%s: %d mem pool had ben enabled!\n", __func__, pool_num);
		mutex_unlock(&dev->mem_pool_mutex);
		return 0;
	}

	memset_s(pool_info, sizeof(struct mem_pool_info_s),
		 0, sizeof(struct mem_pool_info_s));/*lint !e838 */
	map_size = PAGE_ALIGN(size);/*lint !e50 */

	pool = gen_pool_create((int)(order_base_2(ISP_MEM_POOL_ALIGN)), -1);
	if (pool == NULL) {
		pr_err("Create isp gen pool failed!\n");
		mutex_unlock(&dev->mem_pool_mutex);
		return 0;
	}

	if (use_sec_isp()) {
		atf_dev = (struct hisi_atfisp_s *)&atfisp_dev;
		map_info = atf_dev->ap_dyna;

		map_prot = prot;
		pool_addr = (unsigned int)atf_dev->isp_iova_start;
		pool_size = map_size;
		switch (pool_num) {
		case 0:
			pool_addr = (unsigned int)atf_dev->isp_iova_start;
			break;
		case 1:
			pool_addr = 0xD0000000;
			break;
		case 2:
			pool_addr = 0xC4000000;
			break;
		default:
			break;
		}

		addr = atf_dev->ap_dyna_array;
		map_info->a7va = pool_addr;
		map_info->size = pool_size;
		map_info->prot = map_prot;

		pr_debug("[%s] pool_num.%d, iova.0x%x, size.0x%x\n",
			__func__, pool_num, map_info->a7va, size);
		pr_debug("[%s] pool_num.%d, map_size.0x%x, map_prot.0x%x\n",
			__func__, pool_num, map_size, map_prot);

		a7_map_set_pa_list(addr, sgl, map_size);
		ret = atfisp_ispcpu_map();
		if (ret < 0) {
			pr_err("%s: atfisp_ispcpu_map fail.%d\n", __func__, ret);
			mutex_unlock(&dev->mem_pool_mutex);
			BUG(); /*lint !e146*/
			return 0;
		}
	} else {
		pool_addr = iova;

		align_mask = get_size_align_mask(align);
		if (pool_addr > (pool_addr & align_mask)) {
			pool_addr &= align_mask;
			pool_addr += align;
		}

		pool_size = map_size;
	}

	ret = gen_pool_add(pool, (unsigned long)pool_addr,
			  (unsigned long)pool_size, -1);
	if (ret) {
		pr_err("Gen pool add failed!\n");
		mutex_unlock(&dev->mem_pool_mutex);
		mem_pool_destroy(pool_num);
		return 0;
	}

	listnode = kzalloc(sizeof(struct mem_pool_info_list_s), GFP_KERNEL);
	if (listnode == NULL) {
		pr_err("%s: alloc listnode fail\n", __func__);
		mutex_unlock(&dev->mem_pool_mutex);
		mem_pool_destroy(pool_num);
		return 0;
	}

	listnode->addr = 0x0;
	listnode->size = 0x0;
	INIT_LIST_HEAD(&listnode->list);
	pool_info->node = listnode;
	pool_info->addr = pool_addr;
	pool_info->size = pool_size;
	pool_info->prot = prot;
	pool_info->isp_mem_pool = pool;
	pool_info->enable = 1;
	pool_info->dynamic_mem_flag = dynamic_mem_flag;
	pr_debug("[%s] pool_num.%d, isp_mem_pool.0x%pK, pool.addr.0x%x, size.0x%x\n",
			__func__, pool_num, pool, pool_addr, pool_size);
	pr_info("%s: pool num = %d, pool count.%d, pool.addr.0x%x, size.0x%x\n",
			__func__, pool_num, dev->count, pool_addr, pool_size);
	mutex_unlock(&dev->mem_pool_mutex);
	return pool_addr;
}
/*lint -save -e530 -e429*/
static void dump_mem_pool_info(struct mem_pool_info_s *pool_info)
{
	struct mem_pool_info_list_s *node = NULL;
	struct mem_pool_info_list_s *listnode = NULL;
	unsigned int use_size = 0;

	if (pool_info == NULL) {
		pr_err("%s: the pool_info is NULL!\n", __func__);
		return;
	}

	if (pool_info->enable == 0) {
		pr_err("%s: the pool not enable!\n", __func__);
		return;
	}

	listnode = pool_info->node;
	if (listnode == NULL) {
		pr_err("%s: the listnode is NULL!\n", __func__);
		return;
	}

	list_for_each_entry(node, &listnode->list, list) {/*lint !e64 !e826 */
		pr_info("%s: va = 0x%x, size = 0x%x\n",
				__func__, node->addr, node->size);
		use_size += node->size;
	}
	pr_info("%s: use_size = 0x%x, size = 0x%x\n", __func__, use_size, pool_info->size);
}

unsigned long hisp_mem_pool_alloc_iova(
		unsigned int size, unsigned int pool_num)
{
	struct hisi_mem_pool_s *dev = &hisi_mem_pool_info;
	struct mem_pool_info_list_s *node = NULL;
	struct mem_pool_info_s *pool_info = NULL;
	struct mem_pool_info_list_s *listnode = NULL;

	unsigned int map_size = 0;
	unsigned long iova;

	if (size == 0) {
		pr_err("%s: invalid para, size == 0x%x.\n", __func__, size);
		return 0;
	}

	if (pool_num >= ISP_MEM_POOL_NUM) {
		pr_err("%s: wrong num, num : 0x%x\n", __func__, pool_num);
		return 0;
	}

	pool_info = &dev->isp_pool_info[pool_num];
	if (pool_info->enable == 0) {
		pr_err("%s: the pool_num %d not creat!\n", __func__, pool_num);
		return 0;
	}

	listnode = pool_info->node;
	map_size = PAGE_ALIGN(size);/*lint !e50 */
	iova = mem_pool_alloc_iova(pool_info->isp_mem_pool, map_size, 0);
	if (iova == 0) {
		pr_err("[%s] Failed : iova.0x%lx, pool_num.%d, size.0x%x, map_size.0x%x\n",
				__func__, iova, pool_num, size, map_size);
		dump_mem_pool_info(pool_info);
		return 0;
	}

	node = kzalloc(sizeof(struct mem_pool_info_list_s), GFP_KERNEL);
	if (node == NULL) {
		pr_err("%s: alloc mem_pool_info_list_s fail\n", __func__);
		mem_pool_free_iova(pool_info->isp_mem_pool,
				iova, (unsigned long)map_size);
		return 0;
	}

	node->addr = (unsigned int)iova;
	node->size = map_size;
	list_add_tail(&node->list, &listnode->list);
	pr_info("[%s] pool.num.%d, iova.0x%lx, size.0x%x, map_size.0x%x\n",
			__func__, pool_num, iova, size, map_size);
	return iova;
}
/*lint -restore */
unsigned int hisp_mem_pool_free_iova(unsigned int pool_num,
					unsigned int va, unsigned int size)
{
	struct hisi_mem_pool_s *dev = &hisi_mem_pool_info;
	struct mem_pool_info_list_s *node = NULL;
	struct mem_pool_info_list_s *node_temp = NULL;
	struct mem_pool_info_s *pool_info = NULL;
	struct mem_pool_info_list_s *listnode = NULL;
	unsigned int map_size;

	if (size == 0) {
		pr_err("%s: invalid para, size == 0x%x.\n", __func__, size);
		return EINVAL;
	}

	if (pool_num >= ISP_MEM_POOL_NUM) {
		pr_err("Mem pool num wrong!\n");
		return EBADF;
	}

	pool_info = &dev->isp_pool_info[pool_num];
	if (pool_info->enable == 0) {
		pr_err("%s: the pool_num %d not creat!\n", __func__, pool_num);
		return EPERM;
	}

	listnode = pool_info->node;
	map_size = PAGE_ALIGN(size);/*lint !e50 */
	if ((pool_info->addr <= va) && (va < (pool_info->addr + pool_info->size))) {
		list_for_each_entry_safe(node, node_temp, &listnode->list, list) {
			if (node->addr == va) {
				mem_pool_free_iova(pool_info->isp_mem_pool,
						  (unsigned long)node->addr,
						  (unsigned long)node->size);
				list_del(&node->list);
				pr_info("%s: va = 0x%x, size = 0x%x, map_size = 0x%x\n",
					__func__, node->addr, node->size, map_size);
				kfree(node);
				return 0;
			}
		}
	}

	return ENXIO;
}

unsigned int hisp_mem_map_setup(struct scatterlist *sgl,
					unsigned int iova,
					unsigned int size,
					unsigned int prot,
					unsigned int pool_num,
					unsigned int flag,
					unsigned int align)
{
	unsigned int va = 0;
	unsigned int ispcpu_vaddr;
	unsigned int dynamic_mem_flag = 0;
	unsigned int type = HISP_SEC_POOL;

	if (pool_num >= ISP_MEM_POOL_NUM) {
		pr_err("%s: wrong num, num : 0x%x\n", __func__, pool_num);
		return 0;
	}

	pr_err("[%s] flag is %u!\n", __func__, flag);
	switch (flag) {
	case MAP_TYPE_DYNAMIC:
		va = mem_pool_setup(sgl, iova, size, prot,
				pool_num, align, dynamic_mem_flag);
		if (va == 0) {
			pr_err("[%s] Failed : hisp_mem_pool_setup!\n",
					__func__);
			return 0;
		}

		break;
	case MAP_TYPE_DYNAMIC_SEC:
		type = HISP_DYNAMIC_SEC;
		if (sec_process_use_ca_ta() && use_sec_isp()) {
			va = sec_mem_pool_setup(type, pool_num, align, prot);
			if (va == 0) {
				pr_err("[%s] Failed : sec_mem_pool_setup!\n",
						__func__);
				return 0;
			}
		}

		break;
	case MAP_TYPE_STATIC_SEC:
		if (sec_process_use_ca_ta() && use_sec_isp()) {
			if (pool_num == 1)
				type = HISP_SEC_POOL;
			else if (pool_num == 2)
				type = HISP_ISP_SEC_POOL;

			va = sec_mem_pool_setup(type, pool_num, align, prot);
			if (va == 0) {
				pr_err("[%s] Failed : sec_mem_pool_setup!\n",
						__func__);
				return 0;
			}
		} else {
			va = mem_pool_setup(sgl, iova, size, prot,
					pool_num, align, dynamic_mem_flag);
			if (va == 0) {
				pr_err("[%s] Failed : mem_pool_setup!\n",
						__func__);
				return 0;
			}
		}

		break;
	case MAP_TYPE_RAW2YUV:
		if (check_mem_pool_enable(pool_num)) {
			pr_err("[%s] %d mem pool had ben enabled!\n",
					__func__, pool_num);
			return 0;
		}

		ispcpu_vaddr = dynamic_memory_map(sgl,
				MEM_RAW2YUV_DA, (size_t)size, prot);
		if (ispcpu_vaddr == 0) {
			pr_err("[%s] Failed : dynamic_memory_map!\n", __func__);
			return 0;
		}

		dynamic_mem_flag = 1;
		va = mem_pool_setup(sgl, ispcpu_vaddr, size, prot,
				pool_num, align, dynamic_mem_flag);
		if (va == 0) {
			pr_err("[%s] Failed : mem_pool_setup!\n", __func__);
			return 0;
		}

		break;
	default:
		pr_err("[%s] Failed : type.0x%x\n", __func__, flag);
		return 0;
	}
	return va;
}

void hisp_mem_pool_destroy(unsigned int pool_num)
{
	if (pool_num >= ISP_MEM_POOL_NUM) {
		pr_err("%s: wrong num, num : 0x%x\n", __func__, pool_num);
		return;
	}

	mem_pool_destroy(pool_num);
}

static unsigned int ipp_sec_memory_setup(int sharefd,
				unsigned int size, unsigned int prot)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	unsigned int ion_iova = 0;
	int ret = 0;

	if (size == 0) {
		pr_err("[%s] Failed : wrong para. size.%u.\n",
			__func__, size);
		return 0;
	}

	dev->ion_mem[HISP_SEC_IPP].sharefd = sharefd;
	dev->ion_mem[HISP_SEC_IPP].size = size;

	ret = teek_secisp_mem_map(&dev->ion_mem[HISP_SEC_IPP], &ion_iova);
	if (ret < 0) {
		pr_err("[%s] Failed : teek_secisp_mem_map. type.%u, ret.%d.\n",
			__func__, HISP_SEC_IPP, ret);
		return 0;
	}

	return dev->ion_mem[HISP_SEC_IPP].da;
}

static int ipp_sec_memory_destroy(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	int ret = 0;

	ret = teek_secisp_mem_unmap(&dev->ion_mem[HISP_SEC_IPP]);
	if (ret < 0) {
		pr_err("[%s] Failed : teek_secisp_mem_unmap. type.%u, ret.%d.\n",
			__func__, HISP_SEC_IPP, ret);
		return ret;
	}

	return 0;
}

static unsigned int hisp_mem_set_sec(int sharefd, unsigned int size)
{
	unsigned int ion_iova = 0;
	int ret = 0;
	secisp_mem_info mem_info = {0};

	if (size == 0) {
		pr_err("[%s] Failed : wrong size. type.%u, size.%u\n",
				__func__, HISP_MEM_SET_SEC, size);
		return EFAULT;
	}

	mem_info.type = SECISP_MEM_SET_SEC;
	mem_info.sharefd = sharefd;
	mem_info.size = size;
	mem_info.sec_flag = 0;

	ret = teek_secisp_mem_map(&mem_info, &ion_iova);
	if (ret < 0) {
		pr_err("[%s] Failed : teek_secisp_mem_map. ion_mem type.%u, ret.%d.\n",
			__func__, HISP_MEM_SET_SEC, ret);
		return EFAULT;
	}

	return 0;
}

static int hisp_mem_set_nonsec(int sharefd, unsigned int size)
{
	int ret = 0;
	secisp_mem_info mem_info = {0};

	if (size == 0) {
		pr_err("[%s] Failed : wrong size. type.%u, size.%u\n",
				__func__, HISP_MEM_SET_SEC, size);
		return -EFAULT;
	}

	mem_info.type = SECISP_MEM_SET_SEC;
	mem_info.sharefd = sharefd;
	mem_info.size = size;
	mem_info.sec_flag = 0;

	ret = teek_secisp_mem_unmap(&mem_info);
	if (ret < 0)
		pr_err("[%s] Failed : teek_secisp_mem_unmap. ion_mem type.%u, ret.%d.\n",
			__func__, HISP_MEM_SET_SEC, ret);

	return ret;
}


void hisp_dynamic_mem_pool_clean(void)
{
	struct hisi_mem_pool_s *dev = &hisi_mem_pool_info;
	struct mem_pool_info_s *pool_info = NULL;
	unsigned int index = 0;

	for (index = 0; index < ISP_MEM_POOL_NUM; index++) {
		pool_info = &dev->isp_pool_info[index];
		if (pool_info == NULL)
			break;

		if ((pool_info->enable == 1) &&
		    (pool_info->dynamic_mem_flag == 1))
			mem_pool_destroy(index);
	}
}

void hisp_mem_pool_init(void)
{
	struct hisi_mem_pool_s *dev = &hisi_mem_pool_info;
	int ret = 0;

	ret = memset_s(dev, sizeof(hisi_mem_pool_info),
		       0x0, sizeof(hisi_mem_pool_info));/*lint !e838 */
	if (ret < 0)
		pr_err("[%s] memset_s hisi_mem_pool_info to 0 fail.%d\n",
				__func__, ret);

	mutex_init(&dev->mem_pool_mutex);
}

void hisp_mem_pool_exit(void)
{
	struct hisi_mem_pool_s *dev = &hisi_mem_pool_info;
	int ret = 0;

	mutex_destroy(&dev->mem_pool_mutex);
	ret = memset_s(dev, sizeof(hisi_mem_pool_info),
		       0x0, sizeof(hisi_mem_pool_info));/*lint !e838 */
	if (ret < 0)
		pr_err("[%s] memset_s hisi_mem_pool_info to 0 fail.%d\n",
				__func__, ret);
}

/* MDC reserved memory, iova: 0xc3000000 */
unsigned int hisp_mem_pool_alloc_carveout(size_t size, unsigned int type)
{
	if ((type == MAP_TYPE_DYNAMIC_CARVEOUT) && (size == MEM_MDC_SIZE))
		return MEM_MDC_DA;

	return 0;
}

int hisp_mem_pool_free_carveout(unsigned int  iova, size_t size)
{
	return 0;/* the free ops in the powerdn */
}
/*lint -restore */


int hisp_rsctable_init(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

	pr_info("[%s] +\n", __func__);
	if (dev->rsctable_vaddr_const == NULL) {
		pr_err("[%s] rsctable_vaddr_const.%pK\n",
			__func__, dev->rsctable_vaddr_const);
		return -ENOMEM;
	}

	dev->rsctable_vaddr = kmemdup(dev->rsctable_vaddr_const,
				dev->rsctable_size, GFP_KERNEL);
	if (dev->rsctable_vaddr == NULL) {
		pr_err("%s: kmalloc failed.\n", __func__);
		return -ENOMEM;
	}

	pr_info("[%s] -\n", __func__);

	return 0;
}

static u64 get_boot_sec_phyaddr(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;


	if (dev->sec_boot_phymem_addr)
		return dev->sec_boot_phymem_addr;

	return 0;
}

static u64 get_fw_sec_phyaddr(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

	if (dev->sec_fw_phymem_addr)
		return dev->sec_fw_phymem_addr;

	return 0;
}

void hisi_secisp_dump(void)
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
		loadimg[ISPIMG_FW].addr = (unsigned int)addr;
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
	case A7REGISP:
	case A7REGIPCS:
	case A7REGIPCNS:
	case A7REGPCTRL:
	case A7REGSCTRL:
	case A7REGPCFG:
	case A7REGGIC:
	case A7REGSYSC:
	case A7REGUART:
	case A7REGGPIO:
	case A7REGGPIO25:
	case A7REGIOC:
		addr = vaddr;
		break;
	default:
		pr_debug("[%s] Failed : etype.0x%x\n", __func__, etype);
		return 0;
	}

	return addr;
}

static int hisp_secmem_pa_init(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	int index = 0;
	unsigned long addr = 0;
	u64 remap = 0;


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

static int smmu_err_addr_free(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	struct rproc_shared_para *secisp_share_para = NULL;

	secisp_share_para = dev->sec_isp_share_para;
	if (secisp_share_para == NULL) {
		pr_err("Faild: secisp_share_para\n");
		return -ENOMEM;
	}

	secisp_share_para->sec_smmuerr_addr = 0;
	dev->sec_smmuerr_addr = 0;
	return 0;
}

static int smmu_err_addr_init(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	struct rproc_shared_para *secisp_share_para = NULL;


	if (!sec_process_use_ca_ta()) {
	/* set smmu err addr pa = bootware addr pa */
		if (dev->shrdmem->a7mapping[A7BOOT].a7pa != 0)
			dev->sec_smmuerr_addr =
				dev->shrdmem->a7mapping[A7BOOT].a7pa;
	} else {
		if (dev->ion_mem[HISP_SEC_TEXT].pa != 0)
			dev->sec_smmuerr_addr =
				dev->ion_mem[HISP_SEC_TEXT].pa +
				dev->rsctable_offset +
				dev->rsctable_size;
	}


	secisp_share_para = dev->sec_isp_share_para;
	if (secisp_share_para == NULL) {
		pr_err("Faild: secisp_share_para\n");
		return -ENOMEM;
	}

	secisp_share_para->sec_smmuerr_addr = dev->sec_smmuerr_addr;
	return 0;
}

static int set_share_pararms(void)
{
	int ret = 0;
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

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
/*lint -save -e429*/
void free_secmem_rsctable(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

	if (dev->rsctable_vaddr != NULL) {
		kfree(dev->rsctable_vaddr);
		dev->rsctable_vaddr = NULL;
	}
}

static void hisp_get_static_mem_pa(unsigned int type)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	void *va = 0;

	switch (type) {
	case HISP_SEC_VR0:
	case HISP_SEC_VR1:
	case HISP_SEC_VQ:
		va = get_vring_dma_addr(&dev->static_mem[type].pa,
					dev->static_mem[type].size, type);
		break;
	case HISP_SEC_SHARE:
		dev->static_mem[type].pa = get_a7shared_pa();
		break;
	case HISP_SEC_RDR:
		dev->static_mem[type].pa = get_isprdr_addr();
		break;
	default:
		pr_err("[%s] Failed : wrong staic_mem_type.%u\n",
				__func__, type);
		break;
	}

	pr_info("[%s] dev->static_mem[%u].pa.0x%llx\n",
		__func__, type, dev->static_mem[type].pa);
}

static unsigned int hisp_sec_page_setup(int sharefd, unsigned int size)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	unsigned int ion_iova = 0;
	unsigned int static_iova = 0;
	int ret = 0, i = 0;

	if (size != dev->ion_mem[HISP_SEC_PAGE].size) {
		pr_err("[%s] Failed : wrong size. dev->ion_mem[%u].size.%u. size.%u\n",
			__func__, HISP_SEC_PAGE,
			dev->ion_mem[HISP_SEC_PAGE].size, size);
		return 0;
	}

	dev->ion_mem[HISP_SEC_PAGE].sharefd = sharefd;

	ret = teek_secisp_mem_map(&dev->ion_mem[HISP_SEC_PAGE], &ion_iova);
	if (ret < 0) {
		pr_err("[%s] Failed : teek_secisp_mem_map. ion_mem type.%u, ret.%d.\n",
			__func__, HISP_SEC_PAGE, ret);
		return 0;
	}

	for (i = 0; i < HISP_SEC_STATIC_MAX_TYPE; i++) {
		hisp_get_static_mem_pa(i);
		pr_err("[%s] static_mem. type.%u, da.0x%x.\n",
				__func__, i, dev->static_mem[i].da);
		ret = teek_secisp_static_mem_map(&dev->static_mem[i],
						 &static_iova);
		if (ret < 0) {
			pr_err("[%s] Failed : teek_secisp_mem_map. static_mem type.%u, ret.%d.\n",
				__func__, i, ret);
			goto err;
		}
	}

	return dev->ion_mem[HISP_SEC_PAGE].da;
err:
	for (i = i - 1; i >= 0; i--) {
		ret = teek_secisp_static_mem_unmap(&dev->static_mem[i]);
		if (ret < 0)
			pr_err("[%s] Failed : teek_secisp_mem_unmap. static_mem type.%u, ret.%d.\n",
				__func__, i, ret);
	}

	ret = teek_secisp_mem_unmap(&dev->ion_mem[HISP_SEC_PAGE]);
	if (ret < 0)
		pr_err("[%s] Failed : teek_secisp_mem_unmap. ion_mem type.%u, ret.%d.\n",
			__func__, HISP_SEC_PAGE, ret);

	return 0;
}

static int hisp_sec_page_destroy(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	int ret = 0, i = 0;

	for (i = HISP_SEC_STATIC_MAX_TYPE - 1; i >= 0; i--) {
		ret = teek_secisp_static_mem_unmap(&dev->static_mem[i]);
		if (ret < 0)
			pr_err("[%s] Failed : teek_secisp_mem_unmap. static_mem type.%u, ret.%d.\n",
				__func__, i, ret);
	}

	ret = teek_secisp_mem_unmap(&dev->ion_mem[HISP_SEC_PAGE]);
	if (ret < 0)
		pr_err("[%s] Failed : teek_secisp_mem_unmap. ion_mem type.%u, ret.%d.\n",
			__func__, HISP_SEC_PAGE, ret);

	return ret;
}

static unsigned int hisp_sec_boot_setup(unsigned int type,
					int sharefd, unsigned int size)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	unsigned int ion_iova = 0;
	int ret = 0;

	if (size != dev->ion_mem[type].size) {
		pr_err("[%s] Failed : wrong size. dev->ion_mem[%u].size.%u. size.%u\n",
			__func__, type, dev->ion_mem[type].size, size);
		return 0;
	}

	dev->ion_mem[type].sharefd = sharefd;

	ret = teek_secisp_mem_map(&dev->ion_mem[type], &ion_iova);
	if (ret < 0) {
		pr_err("[%s] Failed : teek_secisp_mem_map. ion_mem type.%u, ret.%d.\n",
			__func__, HISP_SEC_TEXT, ret);
		return 0;
	}

	return dev->ion_mem[type].da;
}

static int hisp_sec_boot_destroy(unsigned int type)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	int ret = 0;

	ret = teek_secisp_mem_unmap(&dev->ion_mem[type]);
	if (ret < 0)
		pr_err("[%s] Failed : teek_secisp_mem_unmap. ion_mem type.%u, ret.%d.\n",
			__func__, type, ret);

	return ret;
}

int hisp_secmem_size_get(unsigned int *trusted_mem_size)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

	pr_info("[%s] +", __func__);
	if (dev->trusted_mem_size == 0) {
		pr_err("[%s] Failed : trusted mem size.%u\n",
				__func__, dev->trusted_mem_size);
		return -EINVAL;
	}

	*trusted_mem_size = dev->trusted_mem_size;

	pr_info("[%s] -", __func__);

	return 0;
}
EXPORT_SYMBOL(hisp_secmem_size_get);

int hisp_secmem_pa_set(u64 sec_boot_phymem_addr, unsigned int trusted_mem_size)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

	pr_info("[%s] +", __func__);
	if (dev->secmem_count > 0) {
		pr_err("[%s] Failed : secmem_count.%u\n",
				__func__, dev->secmem_count);
		return -EINVAL;
	}

	if (sec_boot_phymem_addr == 0) {
		pr_err("[%s] Failed : sec_boot_phymem_addr is 0\n", __func__);
		return -EINVAL;
	}

	if (trusted_mem_size < dev->trusted_mem_size) {
		pr_err("[%s] Failed : trusted_mem_size err.%u\n",
				__func__, trusted_mem_size);
		return -EINVAL;
	}

	dev->sec_boot_phymem_addr = sec_boot_phymem_addr;
	pr_debug("[%s] sec_boot_phymem_addr.0x%llx\n",
			__func__, dev->sec_boot_phymem_addr);
	dev->secmem_count++;
	pr_info("[%s] - secmem_count.%u\n", __func__, dev->secmem_count);
	return 0;
}
EXPORT_SYMBOL(hisp_secmem_pa_set);

int hisp_secmem_pa_release(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

	if (dev->secmem_count <= 0) {
		pr_err("[%s] Failed : secmem_count.%u\n",
				__func__, dev->secmem_count);
		return -EINVAL;
	}
	dev->secmem_count--;
	pr_info("[%s] smem_count.%u\n", __func__, dev->secmem_count);
	return 0;
}
EXPORT_SYMBOL(hisp_secmem_pa_release);

int hisp_secmem_size_get_from_type(unsigned int *size, unsigned int type)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

	pr_info("[%s] +", __func__);
	if (type >= (HISP_SEC_MAX_TYPE - 1)) {
		pr_err("[%s] Failed : wrong type.%u\n", __func__, type);
		return -EINVAL;
	}

	if (dev->ion_mem[type].size == 0) {
		pr_err("[%s] Failed : dev->ion_mem[type].size.%u\n",
			__func__, dev->ion_mem[type].size);
		return -EINVAL;
	}

	*size = dev->ion_mem[type].size;

	pr_info("[%s] -", __func__);
	return 0;
}
EXPORT_SYMBOL(hisp_secmem_size_get_from_type);

unsigned int hisp_sec_mem_set(int sharefd, unsigned int type,
				unsigned int size, unsigned int prot)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	unsigned int va = 0;

	pr_info("[%s] + %d", __func__, type);

	if (sharefd < 0) {
		pr_err("[%s] Failed : sharefd.%d\n", __func__, sharefd);
		return -EINVAL;
	}

	switch (type) {
	case HISP_SEC_PAGE:
		va = hisp_sec_page_setup(sharefd, size);
		break;
	case HISP_SEC_TEXT:
	case HISP_SEC_DATA:
		va = hisp_sec_boot_setup(type, sharefd, size);
		break;
	case HISP_SEC_POOL:
	case HISP_ISP_SEC_POOL:
		if (size % SEC_MEM_MAP_ALIGEN != 0) {
			pr_err("[%s] Failed : size not align 2M\n", __func__);
			pr_err("[%s] Failed : size is 0x%x\n", __func__, size);
			va = 0;
			break;
		}
	case HISP_DYNAMIC_SEC:
		dev->ion_mem[type].sharefd = sharefd;
		dev->ion_mem[type].size = size;
		dev->ion_mem[type].prot = prot;
		va = dev->ion_mem[type].da;
		break;
	case HISP_SEC_IPP:
		va = ipp_sec_memory_setup(sharefd, size, prot);
		break;
	case HISP_MEM_SET_SEC:
		va = hisp_mem_set_sec(sharefd, size);
		break;
	default:
		pr_err("[%s] wrong type.%d", __func__, type);
		break;
	}

	pr_info("[%s] -", __func__);
	return va;
}
EXPORT_SYMBOL(hisp_sec_mem_set);

int hisp_sec_mem_release(int sharefd, unsigned int type,
				unsigned int size, unsigned int da)
{
	int ret = -1;

	pr_info("[%s] + %d", __func__, type);

	switch (type) {
	case HISP_SEC_PAGE:
		ret = hisp_sec_page_destroy();
		break;
	case HISP_SEC_TEXT:
	case HISP_SEC_DATA:
		ret = hisp_sec_boot_destroy(type);
		break;
	case HISP_SEC_POOL:
	case HISP_ISP_SEC_POOL:
	case HISP_DYNAMIC_SEC:
		ret = sec_mem_pool_destroy(type);
		break;
	case HISP_SEC_IPP:
		ret = ipp_sec_memory_destroy();
		break;
	case HISP_MEM_SET_SEC:
		ret = hisp_mem_set_nonsec(sharefd, size);
		break;
	default:
		pr_err("[%s] wrong type.%d", __func__, type);
		break;
	}

	if (ret < 0)
		pr_err("[%s] Failed : sec memory destroy.%u\n", __func__, type);

	pr_info("[%s] -", __func__);
	return ret;
}
EXPORT_SYMBOL(hisp_sec_mem_release);

int hisp_sec_ta_enable(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	int ret = 0;

	pr_info("[%s] +", __func__);
	if (dev->sec_ta_enable == 1) {
		pr_err("%s: ta already enable\n", __func__);
		return -1;
	}

	ret = teek_secisp_open();
	if (ret < 0) {
		pr_err("%s: fail: teek_secisp_open\n", __func__);
		return -1;
	}

	dev->sec_ta_enable = 1;

	pr_info("[%s] -", __func__);
	return 0;
}
EXPORT_SYMBOL(hisp_sec_ta_enable);

int hisp_sec_ta_disable(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	int ret = 0;

	pr_info("[%s] +", __func__);
	if (dev->sec_ta_enable == 0) {
		pr_err("%s: ta already disable\n", __func__);
		return -1;
	}

	ret = teek_secisp_close();
	if (ret < 0) {
		pr_err("%s: fail: teek_secisp_close\n", __func__);
		return -1;
	}

	dev->sec_ta_enable = 0;

	pr_info("[%s] -", __func__);
	return ret;
}
EXPORT_SYMBOL(hisp_sec_ta_disable);


static int secisp_work_fn(void *data)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	struct cpumask cpu_mask;
	int ret, cpu_no;

	pr_info("[%s] +\n", __func__);
	if (!sec_process_use_ca_ta()) {
		set_user_nice(current, (int)(-1*(dev->seckthread_nice)));
		cpumask_clear(&cpu_mask);

		for (cpu_no = 1; cpu_no < 4; cpu_no++)
			cpumask_set_cpu(cpu_no, &cpu_mask);

		if (sched_setaffinity(current->pid, &cpu_mask) < 0)
			pr_err("%s: Couldn't set affinity to cpu\n", __func__);
	}

	ret = set_share_pararms();
	if (ret < 0)
		pr_err("[%s] Failed : set_share_pararms.%d\n", __func__, ret);

	ret = hisp_bsp_read_bin("isp_firmware", dev->rsctable_offset,
			dev->rsctable_size, dev->rsctable_vaddr_const);
	if (ret < 0) {
		pr_err("[%s] hisp_bsp_read_bin.%d\n", __func__, ret);
		return ret;
	}

	if (!sec_process_use_ca_ta()) {
		while (1) {
			if (kthread_should_stop())
				break;

			wait_event(dev->secisp_wait, dev->secisp_wake);

			mutex_lock(&dev->pwrlock);
			ret = set_isp_remap_addr(get_boot_sec_phyaddr());
			if (ret < 0) {
				pr_err("[%s] Failed : set_isp_remap_addr.%d\n",
						__func__, ret);
				dev->secisp_wake = 0;
				mutex_unlock(&dev->pwrlock);
				continue;
			}

			ret = hisp_secmem_pa_init();
			if (ret < 0) {
				pr_err("[%s] Failed : hisp_secmem_pa_init.%d\n",
						__func__, ret);
				dev->secisp_wake = 0;
				mutex_unlock(&dev->pwrlock);
				continue;
			}

			ret = smmu_err_addr_init();
			if (ret < 0) {
				pr_err("[%s] Failed : set_share_pararms.%d\n",
						__func__, ret);
				dev->secisp_wake = 0;
				mutex_unlock(&dev->pwrlock);
				continue;
			}

			ret = do_secisp_device_enable();
			if (ret < 0)
				pr_err("[%s] Failed : do_secisp_device_enable.%d.\n",
						__func__, ret);

			dev->secisp_wake = 0;

			mutex_unlock(&dev->pwrlock);
		}
	}
	pr_info("[%s] -\n", __func__);

	return 0;
}


static int secisp_boot_by_ca_ta(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	int ret = 0;
	int sharefd = 0;
	unsigned int size = 0;

	pr_info("[%s] +\n", __func__);
	mutex_lock(&dev->pwrlock);

	ret = smmu_err_addr_init();
	if (ret < 0) {
		pr_err("[%s] Failed : set_share_pararms.%d\n", __func__, ret);
		mutex_unlock(&dev->pwrlock);
		return -1;
	}

	ret = do_secisp_device_enable();
	if (ret < 0) {
		pr_err("[%s] Failed : do_secisp_device_enable.%d.\n",
				__func__, ret);
		mutex_unlock(&dev->pwrlock);
		return -1;
	}

	sharefd = dev->ion_mem[HISP_SEC_TEXT].sharefd;
	if (sharefd < 0) {
		pr_err("[%s] Failed : sharefd.%d\n", __func__, sharefd);
		mutex_unlock(&dev->pwrlock);
		return -1;
	}

	size = dev->ion_mem[HISP_SEC_TEXT].size;
	if (size == 0) {
		pr_err("[%s] Failed : size.%d\n", __func__, size);
		mutex_unlock(&dev->pwrlock);
		return -1;
	}

	ret = teek_secisp_disreset(sharefd, size);
	if (ret < 0) {
		pr_err("[%s] Failed : teek_secisp_disreset.%d.\n",
				__func__, ret);
		mutex_unlock(&dev->pwrlock);
		return -1;
	}
	dev->ispops->refs_fw++;
	mutex_unlock(&dev->pwrlock);
	pr_info("[%s] -\n", __func__);

	return 0;
}

void hisi_ispsec_share_para_set(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

	isp_share_para = dev->sec_isp_share_para;
	pr_info("%s.%d: isp_share_para.%pK, dev->sec_isp_share_para.%pK, case.%u\n",
		__func__, __LINE__, isp_share_para, dev->sec_isp_share_para,
		hisi_isp_rproc_case_get());
}

void wakeup_secisp_kthread(void)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

	wake_up_process(dev->secisp_kthread);
}

int hisi_atfisp_probe(struct platform_device *pdev)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;
	int ret = 0, index = 0;

	dev->map_req_flag               = 0;
	dev->map_dts_flag               = 0;
	dev->rsctable_vaddr             = NULL;
	dev->rsctable_vaddr_const       = NULL;
	dev->device                     = &pdev->dev;
	dev->use_ca_ta                  = 0;
	atfisp_ops.refs_ispsrt_subsys   = UNINITIAL;
	atfisp_ops.refs_isp_module      = UNINITIAL;
	atfisp_ops.refs_a7_module       = UNINITIAL;
	atfisp_ops.refs_dts             = UNINITIAL;
	atfisp_ops.refs_fw              = UNINITIAL;
	dev->ispops                     = &atfisp_ops;

	ret = memset_s(dev->static_mem,
		       HISP_SEC_STATIC_MAX_TYPE * sizeof(secisp_mem_info),
		       0,
		       HISP_SEC_STATIC_MAX_TYPE * sizeof(secisp_mem_info));
	if (ret < 0) {
		pr_err("[%s] Failed : memset_s dev->static_mem.%d.\n",
				__func__, ret);
		return -ENOMEM;
	}

	ret = memset_s(dev->ion_mem,
		      (HISP_SEC_MAX_TYPE - 1) * sizeof(secisp_mem_info),
		       0,
		      (HISP_SEC_MAX_TYPE - 1) * sizeof(secisp_mem_info));
	if (ret < 0) {
		pr_err("[%s] Failed : memset_s dev->ion_mem.%d.\n",
				__func__, ret);
		return -ENOMEM;
	}

	ret = hisi_atf_getdts(pdev);
	if (ret < 0) {
		pr_err("[%s] Failed : hisi_atf_getdts.%d.\n", __func__, ret);
		return -EINVAL;
	}

	ret = hisp_sharedmem_init(dev);
	if (ret < 0) {
		pr_err("[%s] Failed : hisp_sharedmem_init.%d.\n",
				__func__, ret);
		return -ENOMEM;
	}

	ret = hisp_rsctablemem_init(dev);
	if (ret < 0) {
		pr_err("[%s] Failed : hisp_rsctablemem_init.%d.\n",
				__func__, ret);
		ret = -ENOMEM;
		goto hisp_rsctablemem_init_fail;
	}

	if (!sec_process_use_ca_ta()) {
		for (index = 0; index < MAXA7MAPPING; index++) {
			ret = hisp_meminit(index, 0);
			if (ret < 0)
				pr_err("Failed: hisp_meminit.%d\n", ret);
		}
		isp_share_para = (struct rproc_shared_para *)dev->shrdmem->a7mapping[A7SHARED].apva;

		init_waitqueue_head(&dev->secisp_wait);
		atomic_set(&dev->secisp_stop_kthread_status, 0);
	} else {
		for (index = 0; index < (HISP_SEC_MAX_TYPE - 1); index++) {
			ret = hisp_ca_ion_meminit(index);
			if (ret < 0)
				pr_err("Failed: hisp_ca_ion_meminit.%d\n", ret);
		}

		for (index = 0; index < HISP_SEC_STATIC_MAX_TYPE; index++) {
			ret = hisp_ca_static_meminit(index);
			if (ret < 0)
				pr_err("Failed: hisp_ca_static_meminit.%d\n", ret);
		}
		isp_share_para = (struct rproc_shared_para *)get_a7shared_va();
	}

	teek_secisp_ca_probe();
	dev->sec_isp_share_para = isp_share_para;
	mutex_init(&dev->pwrlock);

	if (dev->use_smmuv3_flag == 0) {
		dev->domain = iommu_get_domain_for_dev(dev->device);
		if (dev->domain == NULL) {
			pr_err("[%s] Failed : iommu_get_domain_for_dev.%pK\n",
					__func__, dev->domain);
			ret = -ENODEV;
			goto iommu_get_domain_for_dev_fail;
		}

		dev->phy_pgd_base = hisi_domain_get_ttbr(dev->device);
	}

	mutex_init(&dev->isp_iova_pool_mutex);

	dev->secisp_kthread =
		kthread_create(secisp_work_fn, NULL, "secispwork");
	if (IS_ERR(dev->secisp_kthread)) {
		pr_err("[%s] Failed : kthread_create.%ld\n",
				__func__, PTR_ERR(dev->secisp_kthread));
		ret = -EFAULT;
		goto kthread_create_fail;
	}

	return 0;

kthread_create_fail:
	dev->domain = NULL;
	mutex_destroy(&dev->isp_iova_pool_mutex);

iommu_get_domain_for_dev_fail:
	teek_secisp_ca_remove();
	mutex_destroy(&dev->pwrlock);
	hisp_rsctablemem_exit(dev);

hisp_rsctablemem_init_fail:
	hisp_sharedmem_exit(dev);
	return ret;
}

int hisi_atfisp_remove(struct platform_device *pdev)
{
	struct hisi_atfisp_s *dev = (struct hisi_atfisp_s *)&atfisp_dev;

	dev->domain = NULL;
	if (dev->secisp_kthread != NULL) {
		kthread_stop(dev->secisp_kthread);
		dev->secisp_kthread = NULL;
	}

	mutex_destroy(&dev->isp_iova_pool_mutex);
	teek_secisp_ca_remove();
	mutex_destroy(&dev->pwrlock);
	hisp_rsctablemem_exit(dev);
	hisp_sharedmem_exit(dev);

	return 0;
}

MODULE_DESCRIPTION("Hisilicon atfisp module");
MODULE_AUTHOR("chentao20@huawei.com");
MODULE_LICENSE("GPL");

