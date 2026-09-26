#include <linux/errno.h>
#include <linux/hisi-iommu.h> //for struct iommu_domain_data
#include <linux/iommu.h> //for struct iommu_domain
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/dma-buf.h>
#include <linux/version.h>
#include "ipu_smmu_drv.h"
#include "ipu_mntn.h"

#define SMMU_MSTR_DEBUG_CONFIG_WR (16)
#define SMMU_MSTR_DEBUG_CONFIG_CS (17)
#define SMMU_MSTR_SET_DEBUG_PORT ((1 << SMMU_MSTR_DEBUG_CONFIG_WR) | (1 << SMMU_MSTR_DEBUG_CONFIG_CS))
#define SMMU_MSTR_END_ACK_THRESHOLD (0x100)
#define SMMU_MSTR_INPUT_SEL_REGISTER (0x00000003)
#define SMMU_MSTR_ALL_STREAM_IS_END_ACK (0x0000000f)
#define SMMU_MSTR_GLB_BYPASS_NORMAL_MODE (0x00000000)
#define SMMU_MSTR_WDATA_BURST (0x00000010)
#define SMMU_MSTR_WR_VA_OUT_OF_128BYTE (0x00000008)
#define SMMU_MSTR_WR_VA_OUT_OF_BOUNDARY (0x00000004)
#define SMMU_MSTR_RD_VA_OUT_OF_128BYTE (0x00000002)
#define SMMU_MSTR_RD_VA_OUT_OF_BOUNDARY (0x00000001)
#define SMMU_MSTR_INTCLR_ALL (SMMU_MSTR_WDATA_BURST \
	| SMMU_MSTR_WR_VA_OUT_OF_128BYTE \
	| SMMU_MSTR_WR_VA_OUT_OF_BOUNDARY \
	| SMMU_MSTR_RD_VA_OUT_OF_128BYTE \
	| SMMU_MSTR_RD_VA_OUT_OF_BOUNDARY)

#define SMMU_MSTR_INTCLR_ALL_UNMASK (0x00000000)
#define SMMU_MSTR_INTCLR_ALL_MASK (0x0000001f)
#define SMMU_MSTR_SMRX_0_LEN (0x00004000)
#define SMMU_MSTR_SMRX_START_ALL_STREAM (0x0000000f)
#define SMMU_INTCLR_NS_PTW_NS_STAT (0x00000020)
#define SMMU_INTCLR_NS_PTW_INVALID_STAT (0x00000010)
#define SMMU_INTCLR_NS_PTW_TRANS_STAT (0x00000008)
#define SMMU_INTCLR_NS_TLBMISS_STAT (0x00000004)
#define SMMU_INTCLR_NS_EXT_STAT (0x00000002)
#define SMMU_INTCLR_NS_PERMIS_STAT (0x00000001)
#define SMMU_COMMON_INTCLR_NS_ALL_MASK (0x0000003f)
#define SMMU_COMMON_INTCLR_NS_ALL (SMMU_INTCLR_NS_PTW_NS_STAT \
	| SMMU_INTCLR_NS_PTW_INVALID_STAT \
	| SMMU_INTCLR_NS_PTW_TRANS_STAT \
	| SMMU_INTCLR_NS_TLBMISS_STAT \
	| SMMU_INTCLR_NS_EXT_STAT \
	| SMMU_INTCLR_NS_PERMIS_STAT)
#define SMMU_CACHE_ALL_LEVEL_INVALID_LEVEL1 (0x00000003)
#define SMMU_CACHE_ALL_LEVEL_VALID_LEVEL1 (0x00000002)
#define SMMU_OPREF_CTRL_CONFIG_DUMMY (0x1)
#define SMMU_DEBUG_PORT_START1 (0x10000)
#define SMMU_DEBUG_PORT_START2 (0x20000)
struct smmu_master_reg_offset {
	unsigned int smmu_mstr_base_addr;
	unsigned int smmu_mstr_glb_bypass;
	unsigned int smmu_mstr_end_ack;
	unsigned int smmu_mstr_smrx_start;
	unsigned int smmu_mstr_inpt_sel;
	unsigned int smmu_mstr_intmask;
	unsigned int smmu_mstr_intstat;
	unsigned int smmu_mstr_intclr;
	unsigned int smmu_mstr_dbg_port_in_0;
	unsigned int smmu_mstr_dbg_port_out;
	unsigned int smmu_mstr_smrx_0[IPU_SMMU_TOTAL_STREAM_ID_NUMBER];
	unsigned int read_cmd_total_cnt[IPU_SMMU_READ_STREAM_NUMBER];
	unsigned int read_cmd_miss_cnt[IPU_SMMU_READ_STREAM_NUMBER];
	unsigned int read_data_total_cnt[IPU_SMMU_READ_STREAM_NUMBER];
	unsigned int read_cmd_case_cnt[IPU_SMMU_TAG_COMPARE_CASE_NUMBER];
	unsigned int read_cmd_trans_latency;
	unsigned int write_cmd_total_cnt;
	unsigned int write_cmd_miss_cnt;
	unsigned int write_data_total_cnt;
	unsigned int write_cmd_case_cnt[IPU_SMMU_TAG_COMPARE_CASE_NUMBER];
	unsigned int write_cmd_trans_latency;
};

struct smmu_common_reg_offset {
	unsigned int smmu_common_base_addr;
	unsigned int smmu_scr;
	unsigned int smmu_intmask_ns;
	unsigned int smmu_intstat_ns;
	unsigned int smmu_intclr_ns;
	unsigned int smmu_cb_ttbr0;
	unsigned int smmu_cb_ttbcr;
	unsigned int smmu_scachei_all;
	unsigned int smmu_addr_msb;
	unsigned int smmu_err_rdaddr;
	unsigned int smmu_err_wraddr;
	unsigned int smmu_fama_ctrl1_ns;
	unsigned int override_pref_addr;
	unsigned int cfg_override_original_pref_addr;
};

struct smmu_master_reg_offset smmu_master_reg_offset;
struct smmu_common_reg_offset smmu_common_reg_offset;

struct ion_client *ipu_ion_client = NULL;

static struct iommu_domain *ipu_smmu_domain = NULL;
static struct gen_pool *ipu_iova_pool = NULL;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
static struct device *ipu_dev = NULL;
#endif

struct smmu_manager {
	void __iomem *master_io_addr;
	void __iomem *common_io_addr;
};

struct smmu_manager smmu_manager;

struct memory_manage_node memory_manager;

char register_info[REGISTER_INFO_MAX_LEN] = {0};

DEFINE_MUTEX(ipu_pool_mutex);/*lint !e651 !e708 !e570 !e64 !e785 */
DEFINE_MUTEX(ipu_mem_mngr_mutex);/*lint !e651 !e708 !e570 !e64 !e785 */

/* this func use mutex, for interface only, and SHOULD NOT be called by other mem_mngr functions */
void ipu_mem_mngr_init(void) {
	mutex_lock(&ipu_mem_mngr_mutex);
	INIT_LIST_HEAD(&memory_manager.head);
	mutex_unlock(&ipu_mem_mngr_mutex);
}

/* this func use mutex, for interface only, and SHOULD NOT be called by other mem_mngr functions */
void ipu_mem_mngr_deinit(unsigned long *reset_va) {
	struct list_head *pos;
	struct list_head *n;
	struct memory_manage_node *memory_node;

	mutex_lock(&ipu_mem_mngr_mutex);
	if (list_empty(&memory_manager.head)) {
		printk(KERN_DEBUG"[%s]: no memory-leak\n", __func__);
		mutex_unlock(&ipu_mem_mngr_mutex);
		return;
	} else {
		printk(KERN_ERR"[%s]: IPU_ERROR:FATAL error, risk of memory-leak, try to unmap all memory in list\n", __func__);
	}

	/* here should NOT use "list_for_each" but "list_for_each_safe"
		- iterate over a list safe against removal of list entry */
	list_for_each_safe(pos, n, &memory_manager.head) {
		memory_node = (struct memory_manage_node *)pos;
		printk(KERN_ERR"[%s]: IPU_ERROR:Memory Leak: share_fd=%d,ipu_va=0x%pK,size=0x%lx\n",
			__func__, memory_node->map.share_fd, (void *)(uintptr_t)memory_node->map.format.iova_start, memory_node->map.format.iova_size);
		if (ipu_smmu_unmap(&memory_node->map)) {
			printk(KERN_ERR"[%s]: IPU_ERROR:unmap failed\n", __func__);
		}

		if (*reset_va == memory_node->map.format.iova_start) {
			printk(KERN_ERR"[%s]: IPU_ERROR:virt addr is free\n", __func__);
			*reset_va = 0;
		}

		list_del(&memory_node->head);
		kfree(&memory_node->head);
	}

	mutex_unlock(&ipu_mem_mngr_mutex);

	return;
}

void ipu_mem_mngr_dump(void) {
	struct list_head *pos;
	struct memory_manage_node *memory_node;

	list_for_each(pos, &memory_manager.head) {
		memory_node = (struct memory_manage_node *)pos;
		printk(KERN_DEBUG"[%s] share_fd=%d, VA_base=0x%pK, VA_length=0x%lx\n", __func__,
			memory_node->map.share_fd, (void *)(uintptr_t)memory_node->map.format.iova_start, memory_node->map.format.iova_size);
	}
}

static void * ipu_mem_mngr_check_repeat(struct map_data *map, bool check_va) {
	struct list_head *pos;
	struct memory_manage_node *memory_node;

	list_for_each(pos, &memory_manager.head) {
		memory_node = (struct memory_manage_node *)pos;

		if (memory_node->map.share_fd == map->share_fd &&
			memory_node->map.format.iova_start == map->format.iova_start &&
			memory_node->map.format.prot == map->format.prot &&
			(!check_va || memory_node->map.format.iova_size == map->format.iova_size)) {
				return memory_node;
		}
	}

	return NULL;
}

/* this func use mutex, for interface only, and SHOULD NOT be called by other mem_mngr functions */
void * ipu_mem_mngr_add(struct map_data *map) {
	struct memory_manage_node *node;
	struct memory_manage_node *memory_node;

	mutex_lock(&ipu_mem_mngr_mutex);
	memory_node = (struct memory_manage_node *)ipu_mem_mngr_check_repeat(map, false);

	if (memory_node) {
		printk(KERN_ERR"[%s] IPU_ERROR:this memory has been remapped, use the old: share_fd=%d\n",
			__func__, memory_node->map.share_fd);
		mutex_unlock(&ipu_mem_mngr_mutex);
		return 0;
	}

	node = kmalloc(sizeof(struct memory_manage_node), GFP_KERNEL);

	if (!node) {
		printk(KERN_ERR"[%s] IPU_ERROR:FATAL: kmalloc memory_manage_node fail!\n", __func__);
		mutex_unlock(&ipu_mem_mngr_mutex);
		return 0;
	}
	memset(node, 0, sizeof(struct memory_manage_node));
	memcpy(&node->map, map, sizeof(*map));
	list_add(&node->head, &memory_manager.head);

	mutex_unlock(&ipu_mem_mngr_mutex);
	return node;
}

/* this func use mutex, for interface only, and SHOULD NOT be called by other mem_mngr functions */
int ipu_mem_mngr_del(struct map_data *map) {
	struct memory_manage_node *memory_node;

	mutex_lock(&ipu_mem_mngr_mutex);
	memory_node = (struct memory_manage_node *)ipu_mem_mngr_check_repeat(map, true);

	if (memory_node) {
		list_del(&memory_node->head);
		kfree(memory_node);
		mutex_unlock(&ipu_mem_mngr_mutex);
		return 0;
	} else {
		printk(KERN_ERR"[%s] IPU_ERROR:FATAL: Unknow memory,fd=%d,iova_start=0x%pK,iova_size=0x%lx,prot=0x%x,ignore\n",
			__func__, map->share_fd, (void *)(uintptr_t)map->format.iova_start, map->format.iova_size, map->format.prot);
		mutex_unlock(&ipu_mem_mngr_mutex);
		return -EINVAL;
	}
}

/* this func use mutex, for interface only, and SHOULD NOT be called by other mem_mngr functions */
int ipu_mem_mngr_check_valid(unsigned int inst_addr)
{
	struct list_head *pos;
	struct memory_manage_node *memory_node;

	if (inst_addr > 0x7fffffff) {
		printk(KERN_ERR"[%s] IPU_ERROR:FATAL: inst_addr=%d>0x7fffffff\n", __func__, inst_addr);
		return -EINVAL;
	}

	mutex_lock(&ipu_mem_mngr_mutex);

	list_for_each(pos, &memory_manager.head) {
		memory_node = (struct memory_manage_node *)pos;

		if ((inst_addr >= memory_node->map.format.iova_start) &&
			(inst_addr < (memory_node->map.format.iova_start + memory_node->map.format.iova_size))) {
			mutex_unlock(&ipu_mem_mngr_mutex);
			return 0;
		}
	}

	mutex_unlock(&ipu_mem_mngr_mutex);
	return -EINVAL;
}

static void ipu_reg_bit_write_dword(
				unsigned long reg_addr,
				unsigned int start_bit,
				unsigned int end_bit,
				unsigned int content)
{
	unsigned int set_value;
	unsigned int reg_content;
	unsigned int tmp_mask;
	unsigned int tmp_bit;

	if ((end_bit < start_bit)
		|| (start_bit > 31)
		|| (end_bit > 31)) {
		printk(KERN_ERR"[%s]: IPU_ERROR:error input: reg_addr=%pK,start_bit=0x%x,end_bit=0x%x,content=0x%x\n",
			__func__, (void *)(uintptr_t)reg_addr, start_bit, end_bit, content);
		return;
	}
	set_value	   = content;
	set_value	   = set_value << start_bit;

	tmp_bit 	   = 31 - end_bit;
	tmp_mask	   = 0xffffffff << tmp_bit;
	tmp_mask	   = tmp_mask >> ( start_bit + tmp_bit);
	tmp_mask	   = tmp_mask << start_bit;

	reg_content    = (unsigned int) ioread32((void *)(uintptr_t)reg_addr);
	reg_content   &= (~tmp_mask);
	set_value	  &= tmp_mask;
	iowrite32((reg_content | set_value), (void *)(uintptr_t)reg_addr);
	return;
}

/* get ptr of iommu domain when probe */
static int ipu_enable_iommu(struct device *dev)
{
	int ret;

	if (!dev) {
		printk(KERN_ERR"[%s] dev is NULL\n", __func__);
		return -EIO;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0))
	if (!ipu_ion_client) {
		ipu_ion_client = hisi_ion_client_create("ipu-client");
		if (!ipu_ion_client) {
			printk(KERN_ERR"[%s] IPU_ERROR:hisi_ion_client_create fail\n", __func__);
			return -ENODEV;
		}
	}
#else
	ipu_dev = dev;
#endif

	if (ipu_smmu_domain) {
		printk(KERN_ERR"[%s] ipu_smmu_domain is not NULL\n", __func__);
		return 0;
	}

	if (!iommu_present(dev->bus)) {
		printk(KERN_ERR"[%s] iommu not found\n", __func__);
		return 0;
	}

	ipu_smmu_domain = iommu_domain_alloc(dev->bus);

	if (0 == ipu_smmu_domain) {
		printk(KERN_ERR"[%s] IPU_ERROR:iommu_domain_alloc fail\n", __func__);
		return -EIO;
	}

	ret = iommu_attach_device(ipu_smmu_domain, dev);

	if (ret) {
		printk(KERN_ERR"[%s] IPU_ERROR:iommu_attach_device fail, ret=%d\n", __func__, ret);
		iommu_domain_free(ipu_smmu_domain);
		ipu_smmu_domain = 0;
		return -EIO;
	}

	printk(KERN_DEBUG"[%s] success,ipu_smmu_domain=0x%pK, dev->bus is 0x%pK\n",
		__func__, (void *)ipu_smmu_domain, (void *)dev->bus);
	return 0;
}

static struct gen_pool *iova_pool_setup(unsigned long start,
		unsigned long size, unsigned long align)
{
	struct gen_pool *pool;
	int ret;

	pool = gen_pool_create((int)order_base_2(align), -1);/*lint !e666 !e835 !e747 !e516 !e866 !e712 */
	if (!pool) {
		printk(KERN_ERR"[%s] IPU_ERROR:Create gen pool failed!\n", __func__);
		return NULL;
	}
	/* iova start should not be 0, because return
	   0 when alloc iova is considered as error */
	if (!start) {
		printk(KERN_ERR"[%s] IPU_ERROR:iova start should not be 0!\n", __func__);
		return NULL;
	}

	ret = gen_pool_add(pool, start, size, -1);
	if (ret) {
		printk(KERN_ERR"[%s] IPU_ERROR:Gen pool add failed, ret=%x\n", __func__, ret);
		gen_pool_destroy(pool);
		return NULL;
	}

	return pool;
}

unsigned long ipu_get_smmu_base_phy(struct device *dev)
{
	struct iommu_domain_data *domain_data = 0;

	if (ipu_enable_iommu(dev)) {
		printk(KERN_ERR"[%s] IPU_ERROR:ipu_enable_iommu fail and cannot get TTBR\n", __func__);
		return 0;
	}

	if (0 == ipu_smmu_domain) {
		printk(KERN_ERR"[%s]:IPU_ERROR:ipu_smmu_domain is NULL\n", __func__);
		return 0;
	}

	domain_data = (struct iommu_domain_data *)(ipu_smmu_domain->priv); /*lint !e838*/
	if (0 == domain_data) {
		printk(KERN_ERR"[%s]:IPU_ERROR:domain_data is 0\n", __func__);
		return 0;
	}

	ipu_iova_pool = iova_pool_setup(domain_data->iova_start,
			domain_data->iova_size, domain_data->iova_align);
	if (0 == ipu_iova_pool) {
		printk(KERN_ERR"[%s]:IPU_ERROR:iova_pool_setup ipu_iova_pool is 0\n", __func__);
		return 0;
	}

	return ((unsigned long)domain_data->phy_pgd_base);
}

static void ipu_smmu_mstr_init(bool port_sel, bool hardware_start)
{
	uintptr_t io_mstr_base = (uintptr_t)smmu_manager.master_io_addr;
	unsigned int stream_status;
	int cnt = 0;

	/* set input signal as "register" by config SMMU_MSTR_INPT_SEL */
	if (port_sel) {
		iowrite32(SMMU_MSTR_INPUT_SEL_REGISTER, (void *)(uintptr_t)(io_mstr_base + smmu_master_reg_offset.smmu_mstr_inpt_sel));
	}

	/* polling by loop read SMMU_MSTR_END_ACK */
	do {
		cnt++;
		stream_status = ioread32((void *)(uintptr_t)(io_mstr_base + smmu_master_reg_offset.smmu_mstr_end_ack));

		if (cnt > SMMU_MSTR_END_ACK_THRESHOLD) {
			printk(KERN_DEBUG"[%s] check SMMU MSTR END ACK loop overflow\n", __func__);
			break;
		}
	} while((stream_status & 0xf) != SMMU_MSTR_ALL_STREAM_IS_END_ACK);

	/* set SMMU-normal mode */
	iowrite32(SMMU_MSTR_GLB_BYPASS_NORMAL_MODE, (void *)(uintptr_t)(io_mstr_base + smmu_master_reg_offset.smmu_mstr_glb_bypass));

	/* here can config clk:
	   for core_clk_en, hardware open, for low-power ctrl
	   for apb_clk_en,	software open, for debug (if want to read cache/ram status in RTL)
	   default value is OK, so NO need to config again */

	/* clean interrupt, and NOT mask all interrupts by config SMMU_MSTR_INTCLR and SMMU_MSTR_INTMASK */
	iowrite32(SMMU_MSTR_INTCLR_ALL, (void *)(uintptr_t)(io_mstr_base + smmu_master_reg_offset.smmu_mstr_intclr));
	iowrite32(SMMU_MSTR_INTCLR_ALL_UNMASK, (void *)(uintptr_t)(io_mstr_base + smmu_master_reg_offset.smmu_mstr_intmask));

	if (!hardware_start) {
		/********************************************************
		config stream by SMMU_MSTR_SMRX_0
		for a sid, includes:
		VA max and VA min for this stream-id r/w region;
		len and upwin (in 32k, if VA continue increase, will not decrease in 32k, upwin is 0)
		len should be iid/2, iid(index id) is 8, for iid, for example, if pingpong buffer, iid is 2

		00.b:weight
		01.b:input read
		10.b:output read
		11.b:output write

		.len = iid/2=4
		.upwin = 0(do not search in upwin)
		.bypass = 0(no bypass)

		for malloc and free, VA is not in a designated area, so can not set VA max and VA min
		*********************************************************/
		iowrite32(SMMU_MSTR_SMRX_0_LEN, (void *)(uintptr_t)(io_mstr_base + smmu_master_reg_offset.smmu_mstr_smrx_0[0]));
		iowrite32(SMMU_MSTR_SMRX_0_LEN, (void *)(uintptr_t)(io_mstr_base + smmu_master_reg_offset.smmu_mstr_smrx_0[1]));
		iowrite32(SMMU_MSTR_SMRX_0_LEN, (void *)(uintptr_t)(io_mstr_base + smmu_master_reg_offset.smmu_mstr_smrx_0[2]));
		iowrite32(SMMU_MSTR_SMRX_0_LEN, (void *)(uintptr_t)(io_mstr_base + smmu_master_reg_offset.smmu_mstr_smrx_0[3]));

		/* stream startup by config SMMU_MSTR_SMRX_START */
		iowrite32(SMMU_MSTR_SMRX_START_ALL_STREAM, (void *)(uintptr_t)(io_mstr_base + smmu_master_reg_offset.smmu_mstr_smrx_start));
	}

	return;
}

static void ipu_smmu_comm_init(unsigned long ttbr0, unsigned long smmu_rw_err_virtual_addr)
{
	unsigned int low;
	unsigned int high;
	uintptr_t io_comm_base = (uintptr_t)smmu_manager.common_io_addr;
	unsigned long smmu_rw_err_phy_addr;

	/* set SMMU mode as normal */
	ipu_reg_bit_write_dword(io_comm_base + smmu_common_reg_offset.smmu_scr, 0, 0, 0);

	/* clear SMMU interrupt(SMMU_INTCLR_NS) */
	iowrite32(SMMU_COMMON_INTCLR_NS_ALL, (void *)(uintptr_t)(io_comm_base + smmu_common_reg_offset.smmu_intclr_ns));

	/* clear MASK of interrupt(SMMU_INTMASK_NS) */
	ipu_reg_bit_write_dword(io_comm_base + smmu_common_reg_offset.smmu_intmask_ns, 0, 5, 0);

	/* set stream status: SMMU normal(SMMU_SMRX_NS).
	default value is OK, NO need to config again */

	/* set SMMU Translation Table Base Register for Non-Secure Context Bank0(SMMU_CB_TTBR0) */
	low = (unsigned int)(ttbr0 & 0xffffffff);
	high = (unsigned int)((ttbr0 >> 32) & 0x7f);
	iowrite32(low, (void *)(uintptr_t)(io_comm_base + smmu_common_reg_offset.smmu_cb_ttbr0));
	iowrite32(high, (void *)(uintptr_t)(io_comm_base + smmu_common_reg_offset.smmu_fama_ctrl1_ns));

	/* set Descriptor select of the SMMU_CB_TTBR0 addressed region of Non-Secure Context Bank
	for 64bit system, select Long Descriptor -> 1(SMMU_CB_TTBCR.cb_ttbcr_des) */
	ipu_reg_bit_write_dword(io_comm_base + smmu_common_reg_offset.smmu_cb_ttbcr, 0, 0, 0x1);
	/* set SMMU read/write phy addr in TLB miss case */
	smmu_rw_err_phy_addr = virt_to_phys((void *)(uintptr_t)smmu_rw_err_virtual_addr);
	low = (unsigned int)((smmu_rw_err_phy_addr + 0x80) & 0xffffffffffffff80);
	high = (unsigned int)((smmu_rw_err_phy_addr >> 32) & 0x7f);

	iowrite32(low, (void *)(uintptr_t)(io_comm_base + smmu_common_reg_offset.smmu_err_rdaddr));
	ipu_reg_bit_write_dword(io_comm_base + smmu_common_reg_offset.smmu_addr_msb, 0, 6, high);

	iowrite32(low, (void *)(uintptr_t)(io_comm_base + smmu_common_reg_offset.smmu_err_wraddr));
	ipu_reg_bit_write_dword(io_comm_base + smmu_common_reg_offset.smmu_addr_msb, 7, 13, high);
	return;
}

void ipu_smmu_init(unsigned long ttbr0, unsigned long smmu_rw_err_phy_addr, bool port_sel, bool hardware_start)
{
	ipu_smmu_mstr_init(port_sel, hardware_start);
	ipu_smmu_comm_init(ttbr0, smmu_rw_err_phy_addr);
}

void ipu_smmu_deinit(void)
{
	iowrite32(SMMU_MSTR_INTCLR_ALL_MASK,
		(void *)(uintptr_t)((uintptr_t)smmu_manager.master_io_addr + smmu_master_reg_offset.smmu_mstr_intmask));
	iowrite32(SMMU_COMMON_INTCLR_NS_ALL_MASK,
		(void *)(uintptr_t)((uintptr_t)smmu_manager.common_io_addr + smmu_common_reg_offset.smmu_intmask_ns));
}

static unsigned long ipu_alloc_iova(struct gen_pool *pool,
		unsigned long size)
{
	unsigned long iova;

	mutex_lock(&ipu_pool_mutex);

	iova = gen_pool_alloc(pool, size);
	if (!iova) {
		mutex_unlock(&ipu_pool_mutex);
		printk(KERN_ERR"[%s]: IPU_ERROR:hisi iommu gen_pool_alloc failed! size = %lu\n", __func__, size);
		return 0;
	}
	mutex_unlock(&ipu_pool_mutex);
	return iova;
}

static void ipu_free_iova(struct gen_pool *pool,
		unsigned long iova, size_t size)
{
	mutex_lock(&ipu_pool_mutex);
	gen_pool_free(pool, iova, size);

	mutex_unlock(&ipu_pool_mutex);
}

long ipu_smmu_map(struct map_data *map)
{
	long ret;
	size_t sg_size;
	unsigned long phys_len;
	unsigned long iova_size;
	unsigned long iova_start;
	struct scatterlist *sg;
	struct sg_table *table;
	struct scatterlist *sgl;
	struct map_format *format = &(map->format);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
	struct dma_buf_attachment *attach;
	struct dma_buf *dma_buf;
#else
        struct ion_handle *hdl;
#endif

	if (0 == ipu_smmu_domain || 0 == ipu_iova_pool) {
		printk(KERN_ERR"[%s]: IPU_ERROR:ipu_smmu_domain or ipu_iova_pool is NULL, ipu_smmu_domain=0x%pK, ipu_iova_pool=0x%pK\n",
			__func__, ipu_smmu_domain, ipu_iova_pool);
		return -EFAULT;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0))
	if (0 == ipu_ion_client ) {
		printk(KERN_ERR"[%s]: IPU_ERROR:ipu_ion_client is NULL\n", __func__);
		return -EFAULT;
	}

	hdl = ion_import_dma_buf(ipu_ion_client, map->share_fd);// coverity[UNINIT]
	if (IS_ERR_OR_NULL(hdl)) {
		printk(KERN_ERR"[%s]: IPU_ERROR:SETCONFIG_MAP hdl is error, which is: %pK\n", __func__, hdl);
		return -EFAULT;
	}

	table = ion_sg_table(ipu_ion_client, hdl);
	if (IS_ERR_OR_NULL(table) || !table) { /* Coverity can not understand IS_ERR_OR_NULL, add "!table" */
		printk(KERN_ERR"[%s]: IPU_ERROR:SETCONFIG_MAP table is error, which is: %pK\n", __func__, table);
		return -EFAULT;
	}

#else
	if (0 == ipu_dev) {
		printk(KERN_ERR"[%s]: IPU_ERROR:ipu_dev is NULL\n", __func__);
		return -EFAULT;
	}

	dma_buf = dma_buf_get(map->share_fd);
	if (IS_ERR_OR_NULL(dma_buf)) {
		printk(KERN_ERR"[%s]: IPU_ERROR:dma_buf_get fail, share_fd=%d, dma_buf=%pK\n", __func__, map->share_fd, dma_buf);
		return -EFAULT;
	}

    attach = dma_buf_attach(dma_buf, ipu_dev);
	if (IS_ERR_OR_NULL(attach)) {
		printk(KERN_ERR"[%s]: IPU_ERROR:dma_buf_attach error, attach=%pK\n", __func__, attach);
		dma_buf_put(dma_buf);
		return -EFAULT;
	}

	table = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
	if (IS_ERR_OR_NULL(table)) {
		printk(KERN_ERR"[%s]: IPU_ERROR:dma_buf_map_attachment error, table=%pK\n", __func__, table);
		dma_buf_detach(dma_buf, attach);
		dma_buf_put(dma_buf);
		return -EFAULT;
	}
#endif

	sgl = table->sgl;

	for (phys_len = 0, sg = sgl; sg; sg = sg_next(sg))
		phys_len += (unsigned long)ALIGN(sg->length, PAGE_SIZE);/*lint !e50 */

	iova_size = phys_len;
	iova_start = ipu_alloc_iova(ipu_iova_pool, iova_size);

	sg_size = iommu_map_sg(ipu_smmu_domain, iova_start, sgl,
			(unsigned int)sg_nents(sgl), format->prot);

	if (sg_size != iova_size) {
		printk(KERN_ERR"[%s]: IPU_ERROR:map failed!iova_start = 0x%pK, iova_size = 0x%lx\n",
				__func__, (void *)(uintptr_t)iova_start, iova_size);

		if (ipu_iova_pool) {
			ipu_free_iova(ipu_iova_pool, iova_start, iova_size);
		}
		ret = -EIO;
	} else {
		format->iova_start = iova_start;
		format->iova_size = iova_size;
		ret = 0;
	}

/* clear environment */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0))
	ion_free(ipu_ion_client, hdl);
#else
    dma_buf_unmap_attachment(attach, table, DMA_BIDIRECTIONAL);
	dma_buf_detach(dma_buf, attach);
	dma_buf_put(dma_buf);
#endif

	return ret;
}

long ipu_smmu_unmap(struct map_data *map)
{
	int ret;
	unsigned long unmapped_size;
	struct map_format *format = &(map->format);

	if (0 == ipu_smmu_domain || 0 == ipu_iova_pool) {
		printk(KERN_ERR"[%s]: IPU_ERROR:ipu_smmu_domain or ipu_ion_client or ipu_iova_pool is NULL, ipu_smmu_domain=0x%pK, ipu_iova_pool=0x%pK\n",
			__func__, ipu_smmu_domain, ipu_iova_pool);
		return -EFAULT;
	}

	/* check if iova is in iova pool range */
	ret = addr_in_gen_pool(ipu_iova_pool, format->iova_start,
			format->iova_size);
	if(!ret) {
		printk(KERN_ERR"[%s]IPU_ERROR:illegal para!!iova=0x%pK, size=%ld\n",
				__func__, (void *)(uintptr_t)format->iova_start, format->iova_size);
		return -EINVAL;
	}

	unmapped_size = iommu_unmap(ipu_smmu_domain, format->iova_start, format->iova_size);

	if (unmapped_size != format->iova_size) {
		printk(KERN_ERR"[%s]IPU_ERROR:unmap failed, unmapped_size=%ld, iova_size=%ld\n", __func__, unmapped_size, format->iova_size);
		ret = -EINVAL;
	} else {
		ret = 0;
	}

	ipu_free_iova(ipu_iova_pool, format->iova_start, format->iova_size);

	return ret;
}

int ipu_smmu_mngr_init(void)
{
	smmu_manager.master_io_addr = ioremap((unsigned long)smmu_master_reg_offset.smmu_mstr_base_addr, (unsigned long)0xffff);
	if (!smmu_manager.master_io_addr) {
		printk(KERN_ERR"[%s]IPU_ERROR:smmu_manager.master_io_addr ioremap fail\n", __func__);
		return -ENOMEM;
	}

	smmu_manager.common_io_addr = ioremap((unsigned long)smmu_common_reg_offset.smmu_common_base_addr, (unsigned long)0xffff);
	if (!smmu_manager.common_io_addr) {
		printk(KERN_ERR"[%s]IPU_ERROR:smmu_manager.common_io_addr ioremap fail\n", __func__);
		return -ENOMEM;
	}

	return 0;
}

void ipu_smmu_mngr_deinit(void)
{
	if (smmu_manager.master_io_addr) {
		iounmap(smmu_manager.master_io_addr);
		smmu_manager.master_io_addr = 0;
	}

	if (smmu_manager.common_io_addr) {
		iounmap(smmu_manager.common_io_addr);
		smmu_manager.common_io_addr = 0;
	}
}

void ipu_smmu_override_prefetch_addr(unsigned long reset_va)
{
	iowrite32(reset_va, (void *)(uintptr_t)((uintptr_t)smmu_manager.common_io_addr
		+ (unsigned long)smmu_common_reg_offset.override_pref_addr));
	iowrite32(SMMU_OPREF_CTRL_CONFIG_DUMMY, (void *)(uintptr_t)((uintptr_t)smmu_manager.common_io_addr
		+ (unsigned long)smmu_common_reg_offset.cfg_override_original_pref_addr));

	printk(KERN_DEBUG"[%s] done\n", __func__);
}

bool ipu_smmu_interrupt_handler(struct smmu_irq_count *irq_count)
{
	unsigned int reg_smmu_mstr_status;
	unsigned int reg_smmu_comm_status;
	bool ret = false;
	uintptr_t mstr_io_addr = (uintptr_t)smmu_manager.master_io_addr;
	uintptr_t comm_io_addr = (uintptr_t)smmu_manager.common_io_addr;

	//fixme: if security/protect mode is needed, add code here
	reg_smmu_comm_status = ioread32((void *)(uintptr_t)(comm_io_addr + smmu_common_reg_offset.smmu_intstat_ns));
	reg_smmu_mstr_status = ioread32((void *)(uintptr_t)(mstr_io_addr + smmu_master_reg_offset.smmu_mstr_intstat));

	if (0 != reg_smmu_mstr_status) {
		ret = true;
		printk(KERN_ERR"[%s]: IPU_ERROR:error, smmu mstr interrupt received: %x\n", __func__, reg_smmu_mstr_status);
		if (reg_smmu_mstr_status & SMMU_MSTR_WDATA_BURST) {
			irq_count->mstr_wdata_burst++;
		}
		if (reg_smmu_mstr_status & SMMU_MSTR_WR_VA_OUT_OF_128BYTE) {
			irq_count->mstr_wr_va_out_of_128byte++;
		}
		if (reg_smmu_mstr_status & SMMU_MSTR_WR_VA_OUT_OF_BOUNDARY) {
			irq_count->mstr_wr_va_out_of_boundary++;
		}
		if (reg_smmu_mstr_status & SMMU_MSTR_RD_VA_OUT_OF_128BYTE) {
			irq_count->mstr_rd_va_out_of_128byte++;
		}
		if (reg_smmu_mstr_status & SMMU_MSTR_RD_VA_OUT_OF_BOUNDARY) {
			irq_count->mstr_rd_va_out_of_boundary++;
		}

		printk(KERN_ERR"[%s]: IPU_ERROR:error, Rd_Inst_SID=0x%pK, RdAddr=0x%pK, Wr_Inst_SID=0x%pK, WrAddr=0x%pK\n", __func__,
			(void *)(uintptr_t)ioread32((void *)(mstr_io_addr + 0x50)), (void *)(uintptr_t)ioread32((void *)(mstr_io_addr + 0x54)),
			(void *)(uintptr_t)ioread32((void *)(mstr_io_addr + 0x58)), (void *)(uintptr_t)ioread32((void *)(mstr_io_addr + 0x5c)));

		printk(KERN_ERR"[%s]: IPU_ERROR:error, RW_Burst_len=0x%pK, Awaddr=0x%pK\n", __func__,
			(void *)(uintptr_t)ioread32((void *)(mstr_io_addr + 0x60)), (void *)(uintptr_t)ioread32((void *)(mstr_io_addr + 0x64)));

		/* clear smmu mstr interrupt */
		iowrite32(SMMU_MSTR_INTCLR_ALL, (void *)(uintptr_t)(mstr_io_addr + (unsigned long)smmu_master_reg_offset.smmu_mstr_intclr));
	}

	if (0 != reg_smmu_comm_status) {
		ret = true;
		printk(KERN_ERR"[%s]: IPU_ERROR:error, smmu common interrupt received: 0x%x\n", __func__, reg_smmu_comm_status);

		if (reg_smmu_comm_status & SMMU_INTCLR_NS_PTW_NS_STAT) {
			/* When PTW transaction receive an page table whose ns bit is not match to the prefetch
			transaction, occur this fault. */
			irq_count->comm_ptw_ns_stat++;
		}
		if (reg_smmu_comm_status & SMMU_INTCLR_NS_PTW_INVALID_STAT) {
			/* When PTW transaction receive an invalid page table descriptor or access the invalid
			regoin between t0sz and t1sz in long descriptor mode, occur this fault.*/
			irq_count->comm_ptw_invalid_stat++;
		}
		if (reg_smmu_comm_status & SMMU_INTCLR_NS_PTW_TRANS_STAT) {
			/* When PTW transaciont receive an error response, occur this fault. */
			irq_count->comm_ptw_trans_stat++;
		}
		if (reg_smmu_comm_status & SMMU_INTCLR_NS_TLBMISS_STAT) {
			/* When there is a TLB miss generated during the translation process, the mmu will record this. */
			irq_count->comm_tlbmiss_stat++;
		}
		if (reg_smmu_comm_status & SMMU_INTCLR_NS_EXT_STAT) {
			/* When mmu receive an en error response the mmu will record this as a fault. */
			irq_count->comm_ext_stat++;
		}
		if (reg_smmu_comm_status & SMMU_INTCLR_NS_PERMIS_STAT) {
			/* When the input transaction¡¯s attributes doesn¡¯t match the attributes descripted in the page table,
			the mmu will raise a fault for this. */
			irq_count->comm_permis_stat++;
		}

		/* clear smmu interrupt */
		//fixme: if security/protect mode is needed, add code here
		iowrite32(SMMU_COMMON_INTCLR_NS_ALL, (void *)(uintptr_t)(comm_io_addr + (unsigned long)smmu_common_reg_offset.smmu_intclr_ns));
	}

	return ret;
}

void ipu_smmu_reset_statistic(void)
{
	int i;
	struct smmu_master_reg_offset *offset = &smmu_master_reg_offset;
	void *dbg_port_in = (void *)(uintptr_t)((uintptr_t)smmu_manager.master_io_addr
		+ (unsigned long)offset->smmu_mstr_dbg_port_in_0);

	/* clean read channel cmd-total-count (by config SMMU_MSTR_DBG_PORT_IN_0) */
	for(i = 0; i < IPU_SMMU_READ_STREAM_NUMBER; i++) {
		iowrite32((SMMU_MSTR_SET_DEBUG_PORT + offset->read_data_total_cnt[i]), dbg_port_in);
	}

	/* clean read channel cmd-miss-count (by config SMMU_MSTR_DBG_PORT_IN_0) */
	for(i = 0; i < IPU_SMMU_READ_STREAM_NUMBER; i++) {
		iowrite32((SMMU_MSTR_SET_DEBUG_PORT + offset->read_cmd_miss_cnt[i]), dbg_port_in);
	}

	/* clean read channel data-length-count (by config SMMU_MSTR_DBG_PORT_IN_0) */
	for(i = 0; i < IPU_SMMU_READ_STREAM_NUMBER; i++) {
		iowrite32((SMMU_MSTR_SET_DEBUG_PORT + offset->read_data_total_cnt[i]), dbg_port_in);
	}

	/* clean read channel tag-stat (by config SMMU_MSTR_DBG_PORT_IN_0) */
	for(i = 0; i < IPU_SMMU_TAG_COMPARE_CASE_NUMBER; i++) {
		iowrite32((SMMU_MSTR_SET_DEBUG_PORT + offset->read_cmd_case_cnt[i]), dbg_port_in);
	}

	/* clean read channel latency (by config SMMU_MSTR_DBG_PORT_IN_0) */
	iowrite32((SMMU_MSTR_SET_DEBUG_PORT + offset->read_cmd_trans_latency), dbg_port_in);

	/* clean write channel cmd-total-count (by config SMMU_MSTR_DBG_PORT_IN_0) */
	iowrite32((SMMU_MSTR_SET_DEBUG_PORT + offset->write_cmd_total_cnt), dbg_port_in);

	/* clean write channel cmd-miss-count (by config SMMU_MSTR_DBG_PORT_IN_0) */
	iowrite32((SMMU_MSTR_SET_DEBUG_PORT + offset->write_cmd_miss_cnt), dbg_port_in);

	/* clean write channel data-length-count (by config SMMU_MSTR_DBG_PORT_IN_0) */
	iowrite32((SMMU_MSTR_SET_DEBUG_PORT + offset->write_data_total_cnt), dbg_port_in);

	/* clean write channel tag-stat (by config SMMU_MSTR_DBG_PORT_IN_0) */
	for(i = 0; i < IPU_SMMU_TAG_COMPARE_CASE_NUMBER; i++) {
		iowrite32((SMMU_MSTR_SET_DEBUG_PORT + offset->write_cmd_case_cnt[i]), dbg_port_in);
	}

	/* clean write channel latency (by config SMMU_MSTR_DBG_PORT_IN_0) */
	iowrite32((SMMU_MSTR_SET_DEBUG_PORT + offset->write_cmd_trans_latency), dbg_port_in);
	iowrite32((SMMU_MSTR_SET_DEBUG_PORT + offset->write_cmd_trans_latency), dbg_port_in);
}

void ipu_smmu_record_statistic(struct smmu_statistic *statistic)
{
	struct smmu_master_reg_offset *offset = &smmu_master_reg_offset;
	uintptr_t mstr_io_addr = (uintptr_t)smmu_manager.master_io_addr;

	/* read channel cmd total count */
	statistic->read_stream_cmd_total[0] += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->read_cmd_total_cnt[0]));
	statistic->read_stream_cmd_total[1] += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->read_cmd_total_cnt[1]));
	statistic->read_stream_cmd_total[2] += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->read_cmd_total_cnt[2]));

	/* read channel cmd miss count */
	statistic->read_stream_cmd_miss[0] += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->read_cmd_miss_cnt[0]));
	statistic->read_stream_cmd_miss[1] += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->read_cmd_miss_cnt[1]));
	statistic->read_stream_cmd_miss[2] += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->read_cmd_miss_cnt[2]));

	/* read channel data total count */
	statistic->read_stream_data_total[0] += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->read_data_total_cnt[0]));
	statistic->read_stream_data_total[1] += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->read_data_total_cnt[1]));
	statistic->read_stream_data_total[2] += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->read_data_total_cnt[2]));

	/* read cmd miss/hit and latency */
	statistic->read_stream_cmd_miss_valid += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->read_cmd_case_cnt[0]));
	statistic->read_stream_cmd_miss_pending += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->read_cmd_case_cnt[1]));
	statistic->read_stream_cmd_hit_valid_not_slide_window += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->read_cmd_case_cnt[2]));
	statistic->read_stream_cmd_hit_valid_slide_window += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->read_cmd_case_cnt[3]));
	statistic->read_stream_cmd_hit_pending_not_slide_window += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->read_cmd_case_cnt[4]));
	statistic->read_stream_cmd_hit_pending_slide_window += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->read_cmd_case_cnt[5]));
	statistic->read_stream_cmd_latency += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->read_cmd_trans_latency));

	/* write channel cmd total count */
	statistic->write_stream_cmd_total += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->write_cmd_total_cnt));
	statistic->write_stream_cmd_miss += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->write_cmd_miss_cnt));
	statistic->write_stream_data_total += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->write_data_total_cnt));

	/* write cmd miss/hit and latency */
	statistic->write_stream_cmd_miss_valid += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->write_cmd_case_cnt[0]));
	statistic->write_stream_cmd_miss_pending += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->write_cmd_case_cnt[1]));
	statistic->write_stream_cmd_hit_valid_not_slide_window += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->write_cmd_case_cnt[2]));
	statistic->write_stream_cmd_hit_valid_slide_window += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->write_cmd_case_cnt[3]));
	statistic->write_stream_cmd_hit_pending_not_slide_window += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->write_cmd_case_cnt[4]));
	statistic->write_stream_cmd_hit_pending_slide_window += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->write_cmd_case_cnt[5]));
	statistic->write_stream_cmd_latency += ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->write_cmd_trans_latency));
}

/* for online layer-by-layer mode, run once each op(i.e. conv, pooling, ReLU...), while online merge and offline mode
this func will only call once before run */
void ipu_smmu_pte_update(void)
{
	uintptr_t mstr_io_addr = (uintptr_t)smmu_manager.master_io_addr;
	uintptr_t comm_io_addr = (uintptr_t)smmu_manager.common_io_addr;

	iowrite32(SMMU_MSTR_SMRX_START_ALL_STREAM, (void *)(uintptr_t)(mstr_io_addr + smmu_master_reg_offset.smmu_mstr_smrx_start));

	/* update cache data to avoid this case: phy address across 8GB address-space */
	iowrite32(SMMU_CACHE_ALL_LEVEL_INVALID_LEVEL1, (void *)(uintptr_t)(comm_io_addr + smmu_common_reg_offset.smmu_scachei_all));
	iowrite32(SMMU_CACHE_ALL_LEVEL_VALID_LEVEL1, (void *)(uintptr_t)(comm_io_addr + smmu_common_reg_offset.smmu_scachei_all));
}

bool ipu_smmu_master_get_offset(struct device *dev)
{
	unsigned int property_rd;
	struct smmu_master_reg_offset *offset = &smmu_master_reg_offset;
	struct device_node *node = of_find_node_by_name(dev->of_node, "smmu_master");
	if(!node) {
		printk(KERN_ERR"[%s]: IPU_ERROR:find smmu_master node error\n", __func__);
		return false;
	}

	memset(offset, 0, sizeof(*offset));// coverity[secure_coding]
	property_rd = (unsigned int)of_property_read_u32(node, "smmu-mstr-base-addr", &offset->smmu_mstr_base_addr);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-mstr-glb-bypass", &offset->smmu_mstr_glb_bypass);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-mstr-end-ack", &offset->smmu_mstr_end_ack);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-mstr-smrx-start", &offset->smmu_mstr_smrx_start);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-mstr-inpt-sel", &offset->smmu_mstr_inpt_sel);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-mstr-intmask", &offset->smmu_mstr_intmask);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-mstr-intstat", &offset->smmu_mstr_intstat);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-mstr-intclr", &offset->smmu_mstr_intclr);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-mstr-dbg-port-in-0", &offset->smmu_mstr_dbg_port_in_0);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-mstr-dbg-port-out", &offset->smmu_mstr_dbg_port_out);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-mstr-smrx-0-stream-0", &offset->smmu_mstr_smrx_0[0]);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-mstr-smrx-0-stream-1", &offset->smmu_mstr_smrx_0[1]);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-mstr-smrx-0-stream-2", &offset->smmu_mstr_smrx_0[2]);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-mstr-smrx-0-stream-3", &offset->smmu_mstr_smrx_0[3]);
	property_rd |= (unsigned int)of_property_read_u32(node, "read-cmd-total-cnt-stream-0", &offset->read_cmd_total_cnt[0]);
	property_rd |= (unsigned int)of_property_read_u32(node, "read-cmd-total-cnt-stream-1", &offset->read_cmd_total_cnt[1]);
	property_rd |= (unsigned int)of_property_read_u32(node, "read-cmd-total-cnt-stream-2", &offset->read_cmd_total_cnt[2]);
	property_rd |= (unsigned int)of_property_read_u32(node, "read-cmd-miss-cnt-stream-0", &offset->read_cmd_miss_cnt[0]);
	property_rd |= (unsigned int)of_property_read_u32(node, "read-cmd-miss-cnt-stream-1", &offset->read_cmd_miss_cnt[1]);
	property_rd |= (unsigned int)of_property_read_u32(node, "read-cmd-miss-cnt-stream-2", &offset->read_cmd_miss_cnt[2]);
	property_rd |= (unsigned int)of_property_read_u32(node, "read-data-total-cnt-stream-0", &offset->read_data_total_cnt[0]);
	property_rd |= (unsigned int)of_property_read_u32(node, "read-data-total-cnt-stream-1", &offset->read_data_total_cnt[1]);
	property_rd |= (unsigned int)of_property_read_u32(node, "read-data-total-cnt-stream-2", &offset->read_data_total_cnt[2]);
	property_rd |= (unsigned int)of_property_read_u32(node, "read-cmd-case-cnt-stream-0", &offset->read_cmd_case_cnt[0]);
	property_rd |= (unsigned int)of_property_read_u32(node, "read-cmd-case-cnt-stream-1", &offset->read_cmd_case_cnt[1]);
	property_rd |= (unsigned int)of_property_read_u32(node, "read-cmd-case-cnt-stream-2", &offset->read_cmd_case_cnt[2]);
	property_rd |= (unsigned int)of_property_read_u32(node, "read-cmd-case-cnt-stream-3", &offset->read_cmd_case_cnt[3]);
	property_rd |= (unsigned int)of_property_read_u32(node, "read-cmd-case-cnt-stream-4", &offset->read_cmd_case_cnt[4]);
	property_rd |= (unsigned int)of_property_read_u32(node, "read-cmd-case-cnt-stream-5", &offset->read_cmd_case_cnt[5]);
	property_rd |= (unsigned int)of_property_read_u32(node, "read-cmd-trans-latency", &offset->read_cmd_trans_latency);
	property_rd |= (unsigned int)of_property_read_u32(node, "write-cmd-total-cnt", &offset->write_cmd_total_cnt);
	property_rd |= (unsigned int)of_property_read_u32(node, "write-cmd-miss-cnt", &offset->write_cmd_miss_cnt);
	property_rd |= (unsigned int)of_property_read_u32(node, "write-data-total-cnt", &offset->write_data_total_cnt);
	property_rd |= (unsigned int)of_property_read_u32(node, "write-cmd-case-cnt-stream-0", &offset->write_cmd_case_cnt[0]);
	property_rd |= (unsigned int)of_property_read_u32(node, "write-cmd-case-cnt-stream-1", &offset->write_cmd_case_cnt[1]);
	property_rd |= (unsigned int)of_property_read_u32(node, "write-cmd-case-cnt-stream-2", &offset->write_cmd_case_cnt[2]);
	property_rd |= (unsigned int)of_property_read_u32(node, "write-cmd-case-cnt-stream-3", &offset->write_cmd_case_cnt[3]);
	property_rd |= (unsigned int)of_property_read_u32(node, "write-cmd-case-cnt-stream-4", &offset->write_cmd_case_cnt[4]);
	property_rd |= (unsigned int)of_property_read_u32(node, "write-cmd-case-cnt-stream-5", &offset->write_cmd_case_cnt[5]);
	property_rd |= (unsigned int)of_property_read_u32(node, "write-cmd-trans-latency", &offset->write_cmd_trans_latency);

	if (property_rd) {
		printk(KERN_ERR"[%s]: IPU_ERROR:read property of smmu_master offset error\n", __func__);
		return false;
	}
	return true;
}

bool ipu_smmu_common_get_offset (struct device *dev)
{
	unsigned int property_rd;
	struct smmu_common_reg_offset *offset = &smmu_common_reg_offset;
	struct device_node *node = of_find_node_by_name(dev->of_node, "smmu_common");
	if(!node) {
		printk(KERN_ERR"[%s]: IPU_ERROR:find smmu_common node error\n", __func__);
		return false;
	}

	memset(offset, 0, sizeof(smmu_common_reg_offset));// coverity[secure_coding]
	property_rd = (unsigned int)of_property_read_u32(node, "smmu-common-base-addr", &offset->smmu_common_base_addr);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-scr", &offset->smmu_scr);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-intmask-ns", &offset->smmu_intmask_ns);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-intstat-ns", &offset->smmu_intstat_ns);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-intclr-ns", &offset->smmu_intclr_ns);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-cb-ttbr0", &offset->smmu_cb_ttbr0);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-cb-ttbcr", &offset->smmu_cb_ttbcr);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-scachei-all", &offset->smmu_scachei_all);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-fama-ctrl1-ns", &offset->smmu_fama_ctrl1_ns);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-opref-addr", &offset->override_pref_addr);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-opref-ctrl", &offset->cfg_override_original_pref_addr);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-addr-msb", &offset->smmu_addr_msb);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-err-rdaddr", &offset->smmu_err_rdaddr);
	property_rd |= (unsigned int)of_property_read_u32(node, "smmu-err-wraddr", &offset->smmu_err_wraddr);
	if (property_rd) {
		printk(KERN_ERR"[%s]: IPU_ERROR:read property of smmu_common offset error\n", __func__);
		return false;
	}
	return true;
}

#define IPU_SMMU_MSTR_DEBUG_PORT_NUM (32)
#define IPU_SMMU_MSTR_DEBUG_BASE_NUM (4)
#define IPU_SMMU_MSTR_DEBUG_AXI_RD_CMD_ADDR (0x10000)
#define IPU_SMMU_MSTR_DEBUG_AXI_RD_CMD_INFO (0x10100)
#define IPU_SMMU_MSTR_DEBUG_AXI_WR_CMD_ADDR (0x18000)
#define IPU_SMMU_MSTR_DEBUG_AXI_WR_CMD_INFO (0x18100)
#define IPU_SMMU_RD_CMD_BUF_BITMAP (0x10300)
#define IPU_SMMU_WR_CMD_BUF_BITMAP (0x18300)

void ipu_smmu_dump_strm(void)
{
	int i, j;
	unsigned int base;
	unsigned int port_out[IPU_SMMU_MSTR_DEBUG_PORT_NUM];
	unsigned int port_in[IPU_SMMU_MSTR_DEBUG_BASE_NUM] = {
		IPU_SMMU_MSTR_DEBUG_AXI_RD_CMD_ADDR, IPU_SMMU_MSTR_DEBUG_AXI_RD_CMD_INFO,
		IPU_SMMU_MSTR_DEBUG_AXI_WR_CMD_ADDR, IPU_SMMU_MSTR_DEBUG_AXI_WR_CMD_INFO};

	int sz = REGISTER_INFO_MAX_LEN;
	char *perr = register_info;
	int dsm_offset = 0;

	struct smmu_master_reg_offset *offset = &smmu_master_reg_offset;
	uintptr_t mstr_io_addr = (uintptr_t)smmu_manager.master_io_addr;

	for (i = 0; i < IPU_SMMU_MSTR_DEBUG_BASE_NUM; i++) {

		base = port_in[i];

		for (j = 0; j < IPU_SMMU_MSTR_DEBUG_PORT_NUM; j ++) {
			iowrite32(base + j * sizeof(unsigned int), (void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->smmu_mstr_dbg_port_in_0));
			port_out[j] = ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->smmu_mstr_dbg_port_out));
		}

		/* dump strm status for analysis */
		printk(KERN_ERR"SMMU STRM %x:%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n", base,
			port_out[0], port_out[1], port_out[2],  port_out[3],  port_out[4],  port_out[5],  port_out[6],  port_out[7],
			port_out[8], port_out[9], port_out[10], port_out[11], port_out[12], port_out[13], port_out[14], port_out[15]);

		printk(KERN_ERR"(continue):%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n",
			port_out[16], port_out[17], port_out[18], port_out[19], port_out[20], port_out[21], port_out[22], port_out[23],
			port_out[24], port_out[25], port_out[26], port_out[27], port_out[28], port_out[29], port_out[30], port_out[31]);

		if (0 < dsm_offset){
			perr += dsm_offset;
			sz -= dsm_offset;
		}
		/* coverity[secure_coding] */
		dsm_offset = snprintf(perr, sz, "%d: %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n", base,
		port_out[0], port_out[1], port_out[2],  port_out[3],  port_out[4],  port_out[5],  port_out[6],  port_out[7],
		port_out[8], port_out[9], port_out[10], port_out[11], port_out[12], port_out[13], port_out[14], port_out[15],
		port_out[16], port_out[17], port_out[18], port_out[19], port_out[20], port_out[21], port_out[22], port_out[23],
		port_out[24], port_out[25], port_out[26], port_out[27], port_out[28], port_out[29], port_out[30], port_out[31]);

	}

	iowrite32(IPU_SMMU_RD_CMD_BUF_BITMAP, (void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->smmu_mstr_dbg_port_in_0));
	iowrite32(IPU_SMMU_WR_CMD_BUF_BITMAP, (void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->smmu_mstr_dbg_port_in_0));
	ipu_reg_info.mstr_reg.rd_bitmap = ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->smmu_mstr_dbg_port_out));
	ipu_reg_info.mstr_reg.wr_bitmap = ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->smmu_mstr_dbg_port_out));
	ipu_reg_info.mstr_reg.rd_cmd_total_cnt0 = ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->read_cmd_total_cnt[0]));
	ipu_reg_info.mstr_reg.rd_cmd_total_cnt1 = ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->read_cmd_total_cnt[1]));
	ipu_reg_info.mstr_reg.rd_cmd_total_cnt2 = ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->read_cmd_total_cnt[2]));
	ipu_reg_info.mstr_reg.wr_cmd_total_cnt	= ioread32((void *)(uintptr_t)(mstr_io_addr + (unsigned long)offset->write_cmd_total_cnt));
	printk(KERN_ERR"RD_BITMAP=%x, WR_BITMAP=%x, rd_cmd_total_cnt[0-3]={%x, %x, %x}, wr_cmd_total_cnt=%x\n",
		ipu_reg_info.mstr_reg.rd_bitmap,
		ipu_reg_info.mstr_reg.wr_bitmap,
		ipu_reg_info.mstr_reg.rd_cmd_total_cnt0,
		ipu_reg_info.mstr_reg.rd_cmd_total_cnt1,
		ipu_reg_info.mstr_reg.rd_cmd_total_cnt2,
		ipu_reg_info.mstr_reg.wr_cmd_total_cnt);

}

