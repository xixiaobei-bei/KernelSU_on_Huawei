#include "soc_smmu_interface.h"
#include "soc_smmu_mstr_interface.h"
#include "soc_vcodec_bus_interface.h"
#include "soc_ics_interface.h"
#include "soc_dmss_interface.h"
#include "soc_acpu_baseaddr_interface.h"
#include <linux/errno.h>
#include <linux/hisi/hisi-iommu.h> //for struct iommu_domain_data
#include <linux/iommu.h> //for struct iommu_domain
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/dma-buf.h>
#include <linux/genalloc.h>
#include <linux/scatterlist.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/version.h>

#include "npu_public.h"
#include "npu_smmu.h"
#include "npu_mntn.h"

#define REGISTER_INFO_MAX_LEN   (1024)
extern char register_info[REGISTER_INFO_MAX_LEN];

#define NPU_SMMU_WR_ERR_BUFF_LEN (128)
#define DTS_SMMU_NAME "smmu"
#define SMMU_MSTR_DEBUG_CONFIG_WR (16)
#define SMMU_MSTR_DEBUG_CONFIG_CS (17)
#define SMMU_MSTR_SET_DEBUG_PORT ((1 << SMMU_MSTR_DEBUG_CONFIG_WR) | (1 << SMMU_MSTR_DEBUG_CONFIG_CS))
#define SMMU_MSTR_INPUT_SEL_REGISTER (0x00000003)
#define SMMU_MSTR_ALL_STREAM_IS_END_ACK (0x0000000f)
#define SMMU_MSTR_GLB_BYPASS_NORMAL_MODE (0x00000000)
#define SMMU_MSTR_GLB_BYPASS_BYPASS_MODE (0x00000001)
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
#define SMMU_CACHE_ALL_LEVEL_INVALID_LEVEL2 (0x00000005)
#define SMMU_CACHE_ALL_LEVEL_VALID_LEVEL2 (0x00000004)
#define SMMU_OPREF_CTRL_CONFIG_DUMMY (0x1)
#define SMMU_DEBUG_PORT_START1 (0x10000)
#define SMMU_DEBUG_PORT_START2 (0x20000)

#define NPU_SMMU_MSTR_DEBUG_PORT_NUM (32)
#define NPU_SMMU_MSTR_DEBUG_BASE_NUM (4)
#define NPU_SMMU_MSTR_DEBUG_AXI_RD_CMD_ADDR (0x10000)
#define NPU_SMMU_MSTR_DEBUG_AXI_RD_CMD_INFO (0x10100)
#define NPU_SMMU_MSTR_DEBUG_AXI_WR_CMD_ADDR (0x18000)
#define NPU_SMMU_MSTR_DEBUG_AXI_WR_CMD_INFO (0x18100)
#define NPU_SMMU_RD_CMD_BUF_BITMAP (0x10300)
#define NPU_SMMU_WR_CMD_BUF_BITMAP (0x18300)
#define NPU_SMMU_SMRX_NUMBER (0x4)

#define NPU_NOC_BUS_CONFIG_QOS_TYPE_LIMITER_MODE (0x1)
#define NPU_NOC_BUS_CONFIG_QOS_TYPE_FIXED_MODE (0x0)
#define NPU_NOC_BUS_QOS_EXTCONTROL_ENABLE (0x1)
#define NPU_NOC_BUS_CONFIG_FACTOR (0x0)
#define NPU_NOC_BUS_CONFIG_SATURATION_RESET (0x0)
#define NPU_NOC_BUS_CONFIG_SATURATION_NORMAL (0x20)
#define NPU_NOC_BUS_PRIORITY (0x101)
#define NPU_NOC_BUS_CONFIG_BANDWIDTH_FOR_NOC_QOS (0x1000)
#define NPU_DCQ_MSTS (0x9)
#define NPU_SMMU_OUTSTANDING	(4)
#define NPU_BASELINE_RATE_MHZ	(960)
#define NPU_BASELINE_LMT_OSD	(32)
#define NPU_BASELINE_LMT_BANDWIDTH_MHZ (7500)
#define NPU_DMSS_NPU_MST_FLUX_CTRL_RW_PRI_EN	(0x1f110000)
#define NPU_RD_BW_LMT (0x81000020)
#define NPU_WR_BW_LMT (0x81000008)
#define NPU_BANDWIDTH_SATURATION (0x2)
#define NPU_BIT_PER_SECOND_TO_BC_REG (256)

struct ion_client *npu_ion_client = NULL;
static struct iommu_domain *npu_smmu_domain = NULL;
static struct gen_pool *npu_iova_pool = NULL;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
static struct device *npu_dev = NULL;
#endif

struct smmu_manager {
	void __iomem *master_io_addr[MAX_SUPPORT_CORE_NUM];
	void __iomem *common_io_addr[MAX_SUPPORT_CORE_NUM];
	unsigned int smmu_port_select;
	unsigned int smmu_mstr_hardware_start;
	int stat_enable;
	unsigned long smmu_ttbr0;
	void *smmu_rw_err_virtual_addr;
	unsigned long smmu_reset_va;
	struct smmu_irq_count irq_count;
	struct smmu_statistic smmu_stat[MAX_SUPPORT_CORE_NUM];
};

struct smmu_manager smmu_manager;

struct memory_manage_node {
	struct list_head head;
	struct map_data map;
};

struct memory_manage_node memory_manager;

enum npu_bandwidth_lmt_trategy {
	NPU_BANDWIDTH_LMT_UNSUPPORT = 0,
	NPU_BANDWIDTH_LMT_BY_QOS,
	NPU_BANDWIDTH_LMT_BY_RW_OSD,
	NPU_BANDWIDTH_LMT_BY_BC,
};

struct bandwidth_manager bandwidth_manager[MAX_SUPPORT_CORE_NUM];

char register_info[REGISTER_INFO_MAX_LEN] = {0};

DEFINE_MUTEX(npu_mem_mngr_mutex);/*lint !e651 !e708 !e570 !e64 !e785 */

long npu_smmu_unmap_proc(struct map_data *map);

static struct gen_pool *iova_pool_setup(unsigned long start,
		unsigned long size, unsigned long align)
{
	struct gen_pool *pool;
	int ret;

	/* iova start should not be 0, because return
	   0 when alloc iova is considered as error */
	if (!start) {
		NPU_ERR("iova start should not be 0!\n");
		return NULL;
	}

	pool = gen_pool_create((int)order_base_2(align), -1);/*lint !e666 !e835 !e747 !e516 !e866 !e712 */
	if (!pool) {
		NPU_ERR("Create gen pool failed!\n");
		return NULL;
	}

	ret = gen_pool_add(pool, start, size, -1);
	if (ret) {
		NPU_ERR("Gen pool add failed, ret=%x\n", ret);
		gen_pool_destroy(pool);
		return NULL;
	}

	return pool;
}

static unsigned long npu_alloc_iova(struct gen_pool *pool,
		unsigned long size)
{
	unsigned long iova;
	iova = gen_pool_alloc(pool, size);
	if (!iova) {
		NPU_ERR("hisi iommu gen_pool_alloc failed! size = %lu\n", size);
		return 0;
	}
	return iova;
}

static void npu_free_iova(struct gen_pool *pool,
		unsigned long iova, size_t size)
{
	if ((0 == iova)||(0 == size)) {
		NPU_ERR("iova:%pK size:%ld", (void*)iova, size);
		return;
	}
	gen_pool_free(pool, iova, size);
}

/* get ptr of iommu domain when probe */
static int npu_enable_iommu(struct device *dev)
{
	int ret;

	if (!dev) {
		NPU_ERR("dev is NULL\n");
		return -EIO;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0))
	if (!npu_ion_client) {
		npu_ion_client = hisi_ion_client_create("ipu-client");
		if (!npu_ion_client) {
			NPU_ERR("hisi_ion_client_create fail\n");
			return -ENODEV;
		}
	}
#else
	npu_dev = dev;
#endif

	if (npu_smmu_domain) {
		NPU_ERR("npu_smmu_domain is not NULL\n");
		return 0;
	}

	if (!iommu_present(dev->bus)) {
		NPU_ERR("iommu not found\n");
		return 0;
	}

	npu_smmu_domain = iommu_domain_alloc(dev->bus);

	if (0 == npu_smmu_domain) {
		NPU_ERR("iommu_domain_alloc fail\n");
		return -EIO;
	}

	ret = iommu_attach_device(npu_smmu_domain, dev);

	if (ret) {
		NPU_ERR("iommu_attach_device fail, ret=%d\n", ret);
		iommu_domain_free(npu_smmu_domain);
		npu_smmu_domain = 0;
		return -EIO;
	}

	NPU_DEBUG("success,npu_smmu_domain=0x%pK, dev->bus is 0x%pK\n",
		(void *)npu_smmu_domain, (void *)dev->bus);
	return 0;
}

/* this func use mutex, for interface only, and SHOULD NOT be called by other mem_mngr functions */
void npu_mem_mngr_init(void) {
	INIT_LIST_HEAD(&memory_manager.head);
}

/* this func use mutex, for interface only, and SHOULD NOT be called by other mem_mngr functions */
void npu_mem_mngr_exit(void) {
	struct list_head *pos;
	struct list_head *n;
	struct memory_manage_node *memory_node;
	struct map_data map;
	mutex_lock(&npu_mem_mngr_mutex);

	if (list_empty(&memory_manager.head)) {
		NPU_DEBUG("no memory-leak\n");
		mutex_unlock(&npu_mem_mngr_mutex);
		return;
	} else {
		NPU_ERR("FATAL error, risk of memory-leak, try to unmap all memory in list\n");
	}

	/* here should NOT use "list_for_each" but "list_for_each_safe"
		- iterate over a list safe against removal of list entry */
	list_for_each_safe(pos, n, &memory_manager.head) {
		memory_node = (struct memory_manage_node *)pos;
		NPU_ERR("Memory Leak: share_fd=%d,ipu_va=0x%pK,size=0x%lx\n",
			memory_node->map.share_fd, (void *)memory_node->map.format.iova_start, memory_node->map.format.iova_size);
		if (smmu_manager.smmu_reset_va == memory_node->map.format.iova_start) {
			NPU_INFO("virt addr is to free\n");
			smmu_manager.smmu_reset_va = 0;
		}

		memcpy(&map, &memory_node->map, sizeof(struct map_data));
		if (npu_smmu_unmap_proc(&map)) {
			NPU_ERR("unmap failed\n");
		}
	}
	mutex_unlock(&npu_mem_mngr_mutex);

	return;
}

void npu_mem_mngr_dump(void) {
	struct list_head *pos;
	struct memory_manage_node *memory_node;

	mutex_lock(&npu_mem_mngr_mutex);
	list_for_each(pos, &memory_manager.head) {
		memory_node = (struct memory_manage_node *)pos;
		NPU_DEBUG("share_fd=%d, VA_base=0x%pK, VA_length=0x%lx\n",
			memory_node->map.share_fd, (void *)memory_node->map.format.iova_start, memory_node->map.format.iova_size);
	}
	mutex_unlock(&npu_mem_mngr_mutex);
}

static void * npu_mem_mngr_check_repeat(struct map_data *map, bool check_va) {
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
static void npu_mem_mngr_add(struct map_data *map) {
	struct memory_manage_node *node;
	struct memory_manage_node *memory_node;

	memory_node = (struct memory_manage_node *)npu_mem_mngr_check_repeat(map, false);

	if (memory_node) {
		NPU_ERR("this memory has been remapped, use the old: share_fd=%d\n",
			memory_node->map.share_fd);
	}

	node = kmalloc(sizeof(struct memory_manage_node), GFP_KERNEL);
	if (!node) {
		NPU_ERR("FATAL: kmalloc memory_manage_node fail!\n");
		return;
	}

	memset(node, 0, sizeof(*node));
	memcpy(&node->map, map, sizeof(*map));
	list_add(&node->head, &memory_manager.head);
    return; //lint !e429
}

/* this func use mutex, for interface only, and SHOULD NOT be called by other mem_mngr functions */
static int npu_mem_mngr_del(struct map_data *map) {
	struct memory_manage_node *memory_node;

	memory_node = (struct memory_manage_node *)npu_mem_mngr_check_repeat(map, true);

	if (memory_node) {
		list_del(&memory_node->head);
		kfree(memory_node);
		return 0;
	} else {
		NPU_ERR("FATAL: Unknow memory,fd=%d,iova_start=0x%pK,iova_size=0x%lx,prot=0x%x,ignore\n",
			map->share_fd, (void *)map->format.iova_start, map->format.iova_size, map->format.prot);
		return -EINVAL;
	}
}

/* this func use mutex, for interface only, and SHOULD NOT be called by other mem_mngr functions */
int npu_mem_mngr_check_valid(unsigned int inst_addr, unsigned int inst_size)
{
	int ret = 0;
	struct list_head *pos;
	struct memory_manage_node *memory_node;

	mutex_lock(&npu_mem_mngr_mutex);
	list_for_each(pos, &memory_manager.head) {
		memory_node = (struct memory_manage_node *)pos;

		if ((inst_addr >= memory_node->map.format.iova_start) &&
			(inst_addr < (memory_node->map.format.iova_start + memory_node->map.format.iova_size))) {
			if (((unsigned long)inst_addr + (unsigned long)inst_size) > (memory_node->map.format.iova_start + memory_node->map.format.iova_size)) {
				NPU_ERR("invalid inst_size, addr/size = [0x%pK, 0x%x] not in [0x%pK, 0x%lx]",
					(void *)(unsigned long)inst_addr, inst_size, (void *)memory_node->map.format.iova_start, memory_node->map.format.iova_size);
				ret = -EINVAL;
			}
			mutex_unlock(&npu_mem_mngr_mutex);
			return ret;
		}
	}
	mutex_unlock(&npu_mem_mngr_mutex);

	return -EINVAL;
}

static int npu_bandwidth_lmt_init(struct device *dev, unsigned int strategy)
{
	memset(bandwidth_manager, 0, sizeof(bandwidth_manager));

	bandwidth_manager[NPU_CORE_0].noc_bus_io_addr = ioremap((unsigned long)SOC_ACPU_NOC_ICS_Service_Target_BASE_ADDR, (unsigned long)0xff);
	if (!bandwidth_manager[NPU_CORE_0].noc_bus_io_addr) {
		NPU_ERR("noc_bus_io_addr[NPU_CORE_0] ioremap fail\n");
		goto lmt_init_error_exit;
	}

	bandwidth_manager[NPU_CORE_0].dmss_flux_io_addr = ioremap((unsigned long)SOC_DMSS_GLB_MST_FLUX_ADDR(SOC_ACPU_DMSS_BASE_ADDR, NPU_DCQ_MSTS), (unsigned long)0x4); //lint !e571
	if (!bandwidth_manager[NPU_CORE_0].dmss_flux_io_addr) {
		NPU_ERR("dmss_flux_io_addr ioremap fail\n");
		goto unmap_noc_ics_service;
	}

	bandwidth_manager[NPU_CORE_0].bandwidth_lmt_strategy = strategy;

	if (NPU_VERSION_V200 == hard_version) {
		bandwidth_manager[NPU_CORE_1].noc_bus_io_addr = ioremap((unsigned long)SOC_ACPU_NOC_ICS2_Service_Target_BASE_ADDR, (unsigned long)0xff);
		if (!bandwidth_manager[NPU_CORE_1].noc_bus_io_addr) {
			NPU_ERR("noc_bus_io_addr[NPU_CORE_1] ioremap fail\n");
			goto unmap_npu_dmss;
		}

		bandwidth_manager[NPU_CORE_1].dmss_flux_io_addr = bandwidth_manager[NPU_CORE_0].dmss_flux_io_addr; //for Atlanta, NPU0/NPU1 use same flux in DMSS
		bandwidth_manager[NPU_CORE_1].bandwidth_lmt_strategy = strategy;
	}

	return 0;

unmap_npu_dmss:
	iounmap(bandwidth_manager[NPU_CORE_0].dmss_flux_io_addr);
	bandwidth_manager[NPU_CORE_0].dmss_flux_io_addr = 0;
	bandwidth_manager[NPU_CORE_0].bandwidth_lmt_strategy = NPU_BANDWIDTH_LMT_UNSUPPORT;
unmap_noc_ics_service:
	iounmap(bandwidth_manager[NPU_CORE_0].noc_bus_io_addr);
	bandwidth_manager[NPU_CORE_0].noc_bus_io_addr = 0;
lmt_init_error_exit:
	return -ENOMEM;

}

static void npu_bandwidth_lmt_exit(void)
{
	int count;
	for(count = 0; count < MAX_SUPPORT_CORE_NUM; count++) {
		if (bandwidth_manager[count].noc_bus_io_addr) {
			iounmap(bandwidth_manager[count].noc_bus_io_addr);
			bandwidth_manager[count].noc_bus_io_addr = 0;
		}
	}

	/* NPU CORE0 and CORE1 dmss_flux_io_addr is the same */
	if (bandwidth_manager[NPU_CORE_0].dmss_flux_io_addr) {
		iounmap(bandwidth_manager[NPU_CORE_0].dmss_flux_io_addr);
		bandwidth_manager[NPU_CORE_0].dmss_flux_io_addr = 0;
	}

	memset(bandwidth_manager, 0, sizeof(bandwidth_manager));
}

static void npu_bandwidth_lmt_bc(unsigned int coreID, void *npu_irq_io_addr, unsigned int npu_rate, unsigned int vcodecbus_rate)
{
	unsigned int noc_bandwidth_lmt;
	unsigned int bandwidth;
	unsigned int npu_rate_MHz;
	unsigned int reg_bandwidth;
	void __iomem *dmss_flux = bandwidth_manager[coreID].dmss_flux_io_addr;

	npu_rate_MHz = npu_rate / NPU_RATE_HZ_TO_MHZ;
	bandwidth = (NPU_BASELINE_LMT_BANDWIDTH_MHZ * npu_rate_MHz) / NPU_BASELINE_RATE_MHZ;
	noc_bandwidth_lmt = (unsigned long)bandwidth * NPU_BIT_PER_SECOND_TO_BC_REG / (vcodecbus_rate / NPU_RATE_HZ_TO_MHZ) + 1;

	reg_bandwidth = ((noc_bandwidth_lmt & 0x1fff) << 16) + NPU_BANDWIDTH_SATURATION;
	NPU_DEBUG("bandwidth=%d, bc_bandwidth=%d, ret=%d\n", bandwidth, noc_bandwidth_lmt, reg_bandwidth);

	iowrite32(reg_bandwidth, (void *)SOC_ICS_BC_READ_LMT_ADDR((unsigned long)npu_irq_io_addr));
	iowrite32(reg_bandwidth, (void *)SOC_ICS_BC_WRITE_LMT_ADDR((unsigned long)npu_irq_io_addr));
	iowrite32(NPU_DMSS_NPU_MST_FLUX_CTRL_RW_PRI_EN, dmss_flux);
}

static void npu_bandwidth_lmt_outstanding(unsigned int npu_rate, void *npu_irq_io_addr)
{
	unsigned int read_outstanding;
	unsigned int write_outstanding;
	unsigned int reg_bandwidth;
	unsigned int npu_rate_MHz = npu_rate / NPU_RATE_HZ_TO_MHZ;

	/* for npu_rate=960M, assume read/write osd is 32 */
	read_outstanding = ((NPU_BASELINE_LMT_OSD * npu_rate_MHz) / NPU_BASELINE_RATE_MHZ + NPU_SMMU_OUTSTANDING) & 0xff;
	write_outstanding = ((NPU_BASELINE_LMT_OSD * npu_rate_MHz) / NPU_BASELINE_RATE_MHZ) & 0xff;
	reg_bandwidth = (read_outstanding << 8) + write_outstanding;
	NPU_DEBUG("read/write OSD=%u/%u\n", read_outstanding, write_outstanding);

	iowrite32(reg_bandwidth, (void *)SOC_ICS_MAX_OSD_ADDR((unsigned long)npu_irq_io_addr));
}

static void npu_bandwidth_lmt_noc_qos(unsigned int coreID, void *npu_irq_io_addr)
{
	void __iomem *vcodecbus_cfg = bandwidth_manager[coreID].noc_bus_io_addr;
	unsigned long io_comm_base = (unsigned long)smmu_manager.common_io_addr[coreID];
	SOC_SMMU_SCR_UNION smmu_comm_scr;
	SOC_SMMU_SMRX_NS_UNION smmu_smrx_ns;
	unsigned int smrx_cnt;

	if (NPU_VERSION_V200 == hard_version) {
		/* set fetch read/write qos enable, and set r/w qos value as 1 */
		smmu_comm_scr.value = ioread32((void *)SOC_SMMU_SCR_ADDR(io_comm_base));
		smmu_comm_scr.reg.rqos_en = 1;
		smmu_comm_scr.reg.wqos_en = 1;
		smmu_comm_scr.reg.rqos = 1;
		smmu_comm_scr.reg.wqos = 1;
		iowrite32(smmu_comm_scr.value, (void *)SOC_SMMU_SCR_ADDR(io_comm_base));

		/* set streams' ptw qos as 3 */
		for (smrx_cnt = 0; smrx_cnt < NPU_SMMU_SMRX_NUMBER; smrx_cnt++) {
			smmu_smrx_ns.value = ioread32((void *)SOC_SMMU_SMRX_NS_ADDR(io_comm_base, smrx_cnt));
			smmu_smrx_ns.reg.smr_ptw_qos = 3;
			iowrite32(smmu_smrx_ns.value, (void *)SOC_SMMU_SMRX_NS_ADDR(io_comm_base, smrx_cnt));
		}

		/* set read/write bus bandwidth limit */
		iowrite32(NPU_RD_BW_LMT, (void *)SOC_ICS_BC_READ_LMT_ADDR((unsigned long)npu_irq_io_addr));
		iowrite32(NPU_WR_BW_LMT, (void *)SOC_ICS_BC_WRITE_LMT_ADDR((unsigned long)npu_irq_io_addr));

		/* set qos working type as "fixed mode(0)" */
		iowrite32(NPU_NOC_BUS_CONFIG_QOS_TYPE_FIXED_MODE, (void *)(SOC_VCODEC_BUS_CNN_QOS_MODE_ADDR((unsigned long)vcodecbus_cfg)));

		/* set priority urgency level of read/write as 1 */
		iowrite32(NPU_NOC_BUS_PRIORITY, (void *)(SOC_VCODEC_BUS_CNN_QOS_PRIORITY_ADDR((unsigned long)vcodecbus_cfg)));

	} else {
		iowrite32(NPU_NOC_BUS_CONFIG_QOS_TYPE_LIMITER_MODE, (void *)(SOC_VCODEC_BUS_CNN_QOS_MODE_ADDR((unsigned long)vcodecbus_cfg)));
	}

	iowrite32(NPU_NOC_BUS_CONFIG_BANDWIDTH_FOR_NOC_QOS, (void *)(SOC_VCODEC_BUS_CNN_QOS_BANDWIDTH_ADDR((unsigned long)vcodecbus_cfg)));
	iowrite32(NPU_NOC_BUS_CONFIG_SATURATION_NORMAL, (void *)(SOC_VCODEC_BUS_CNN_QOS_SATURATION_ADDR((unsigned long)vcodecbus_cfg)));
	iowrite32(NPU_NOC_BUS_QOS_EXTCONTROL_ENABLE, (void *)(SOC_VCODEC_BUS_CNN_QOS_EXTCONTROL_ADDR((unsigned long)vcodecbus_cfg)));

	NPU_DEBUG("coreID=%d done\n", coreID);
}

void npu_bandwidth_lmt_config(unsigned int coreID, void *npu_irq_io_addr, unsigned int npu_rate, unsigned int vcodecbus_rate)
{
	CHECK_COREID_INVALID_RETURN_VOID(coreID);

	if (!npu_irq_io_addr) {
		NPU_ERR("NULL npu_irq_io_addr, bandwidth lmt fail\n");
		return;
	}

	if (0 == npu_rate) {
		NPU_ERR("npu_rate is 0\n");
		return;
	}

	if (0 == vcodecbus_rate) {
		NPU_ERR("vcodecbus_rate is 0\n");
		return;
	}

	switch (bandwidth_manager[coreID].bandwidth_lmt_strategy) {
	case NPU_BANDWIDTH_LMT_BY_QOS:
		npu_bandwidth_lmt_noc_qos(coreID, npu_irq_io_addr);
		break;
	case NPU_BANDWIDTH_LMT_BY_RW_OSD:
		npu_bandwidth_lmt_outstanding(npu_rate, npu_irq_io_addr);
		break;
	case NPU_BANDWIDTH_LMT_BY_BC:
		npu_bandwidth_lmt_bc(coreID, npu_irq_io_addr, npu_rate, vcodecbus_rate);
		break;
	default:
		NPU_ERR("NULL npu_irq_io_addr, bandwidth lmt fail\n");
		break;
	}
}

void npu_bandwidth_lmt_reset(unsigned int coreID)
{
	void __iomem *vcodecbus_cfg;
	CHECK_COREID_INVALID_RETURN_VOID(coreID);

	vcodecbus_cfg = bandwidth_manager[coreID].noc_bus_io_addr;
	iowrite32(NPU_NOC_BUS_CONFIG_QOS_TYPE_LIMITER_MODE, (void *)(SOC_VCODEC_BUS_CNN_QOS_MODE_ADDR((unsigned long)vcodecbus_cfg)));
	iowrite32(0, (void *)(SOC_VCODEC_BUS_CNN_QOS_BANDWIDTH_ADDR((unsigned long)vcodecbus_cfg)));
	iowrite32(0, (void *)(SOC_VCODEC_BUS_CNN_QOS_SATURATION_ADDR((unsigned long)vcodecbus_cfg)));
}

static int npu_smmu_get_err_phy(struct device *dev)
{
	/* add 0x80 is for 128Byte alignment. */
	smmu_manager.smmu_rw_err_virtual_addr = devm_kzalloc(dev, NPU_SMMU_WR_ERR_BUFF_LEN + 0x80, GFP_KERNEL);
	if (!smmu_manager.smmu_rw_err_virtual_addr) {
		NPU_ERR("Failed to allocate memory for smmu read and write phy addr in error case\n");
		return -ENOMEM;
	}

	return 0;
}

static int npu_smmu_get_base_phy(struct device *dev)
{
	struct iommu_domain_data *domain_data = 0;

	if (npu_enable_iommu(dev)) {
		NPU_ERR("ipu_enable_iommu fail and cannot get TTBR\n");
		return -ENOMEM;
	}

	if (0 == npu_smmu_domain) {
		NPU_ERR("npu_smmu_domain is NULL\n");
		return -ENOMEM;
	}

	domain_data = (struct iommu_domain_data *)(npu_smmu_domain->priv); /*lint !e838*/
	if (0 == domain_data) {
		NPU_ERR("domain_data is 0\n");
		goto error_exit;
	}

	npu_iova_pool = iova_pool_setup(domain_data->iova_start,
			domain_data->iova_size, domain_data->iova_align);
	if (0 == npu_iova_pool) {
		NPU_ERR("iova_pool_setup npu_iova_pool is 0\n");
		goto error_exit;
	}

	smmu_manager.smmu_ttbr0 = (unsigned long)domain_data->phy_pgd_base;
	return 0;

error_exit:
	iommu_domain_free(npu_smmu_domain);
	npu_smmu_domain = 0;

	return -ENOMEM;
}

static void npu_smmu_mstr_init(unsigned int coreID)
{
	unsigned long io_mstr_base = (unsigned long)smmu_manager.master_io_addr[coreID];

	/* set input signal as "register" by config SMMU_MSTR_INPT_SEL */
	if (smmu_manager.smmu_port_select) {
		iowrite32(SMMU_MSTR_INPUT_SEL_REGISTER, (void *)SOC_SMMU_MSTR_INPT_SEL_ADDR(io_mstr_base));
	}


	/* set SMMU-normal mode */
	iowrite32(SMMU_MSTR_GLB_BYPASS_NORMAL_MODE, (void *)SOC_SMMU_MSTR_GLB_BYPASS_ADDR(io_mstr_base));

	/* here can config clk:
	   for core_clk_en, hardware open, for low-power ctrl
	   for apb_clk_en,	software open, for debug (if want to read cache/ram status in RTL)
	   default value is OK, so NO need to config again */

	/* clean interrupt, and NOT mask all interrupts by config SMMU_MSTR_INTCLR and SMMU_MSTR_INTMASK */
	iowrite32(SMMU_MSTR_INTCLR_ALL, (void *)SOC_SMMU_MSTR_INTCLR_ADDR(io_mstr_base));
	iowrite32(SMMU_MSTR_INTCLR_ALL_UNMASK, (void *)SOC_SMMU_MSTR_INTMASK_ADDR(io_mstr_base));

	if (!smmu_manager.smmu_mstr_hardware_start) {
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
		iowrite32(SMMU_MSTR_SMRX_0_LEN, (void *)SOC_SMMU_MSTR_SMRX_0_ADDR(io_mstr_base, 0));
		iowrite32(SMMU_MSTR_SMRX_0_LEN, (void *)SOC_SMMU_MSTR_SMRX_0_ADDR(io_mstr_base, 1));
		iowrite32(SMMU_MSTR_SMRX_0_LEN, (void *)SOC_SMMU_MSTR_SMRX_0_ADDR(io_mstr_base, 2));
		iowrite32(SMMU_MSTR_SMRX_0_LEN, (void *)SOC_SMMU_MSTR_SMRX_0_ADDR(io_mstr_base, 3));

		/* stream startup by config SMMU_MSTR_SMRX_START */
		iowrite32(SMMU_MSTR_SMRX_START_ALL_STREAM, (void *)SOC_SMMU_MSTR_SMRX_START_0_ADDR(io_mstr_base));
	}

	return;
}

static void npu_smmu_comm_init(unsigned int coreID, unsigned long ttbr0, unsigned long smmu_rw_err_virtual_addr)
{
	unsigned int low;
	unsigned int high;
	unsigned long smmu_rw_err_phy_addr;
	SOC_SMMU_SCR_UNION smmu_scr;
	SOC_SMMU_INTMASK_NS_UNION intr;
	SOC_SMMU_CB_TTBCR_UNION cb_ttbcr;
	SOC_SMMU_ADDR_MSB_UNION addr_msb;
	unsigned long io_comm_base = (unsigned long)smmu_manager.common_io_addr[coreID];

	smmu_scr.value = (unsigned int)ioread32((void *)SOC_SMMU_SCR_ADDR(io_comm_base));

	/* set NPU0/NPU1 mid */
	if (0 == coreID) {
		smmu_scr.reg.ptw_mid = 0x28;
	} else {
		smmu_scr.reg.ptw_mid = 0x2f;
	}

	/* set SMMU mode as normal */
	smmu_scr.reg.glb_bypass = 0x0;
	iowrite32(smmu_scr.value, (void *)SOC_SMMU_SCR_ADDR(io_comm_base));

	/* clear SMMU interrupt(SMMU_INTCLR_NS) */
	iowrite32(SMMU_COMMON_INTCLR_NS_ALL, (void *)SOC_SMMU_INTCLR_NS_ADDR(io_comm_base));

	/* clear MASK of interrupt(SMMU_INTMASK_NS) */
	intr.value = (unsigned int)ioread32((void *)SOC_SMMU_INTMASK_NS_ADDR(io_comm_base));
	intr.reg.intns_permis_msk = 0;
	intr.reg.intns_ext_msk = 0;
	intr.reg.intns_tlbmiss_msk = 0;
	intr.reg.intns_ptw_trans_msk = 1;
	intr.reg.intns_ptw_invalid_msk = 1;
	intr.reg.intns_ptw_ns_msk = 1;
	iowrite32(intr.value, (void *)SOC_SMMU_INTMASK_NS_ADDR(io_comm_base));

	/* set stream status: SMMU normal(SMMU_SMRX_NS).
	default value is OK, NO need to config again */

	/* set SMMU Translation Table Base Register for Non-Secure Context Bank0(SMMU_CB_TTBR0) */
	low = (unsigned int)(ttbr0 & 0xffffffff);
	high = (unsigned int)((ttbr0 >> 32) & 0x7f);
	iowrite32(low, (void *)SOC_SMMU_CB_TTBR0_ADDR(io_comm_base));
	iowrite32(high, (void *)SOC_SMMU_FAMA_CTRL1_NS_ADDR(io_comm_base));

	/* set Descriptor select of the SMMU_CB_TTBR0 addressed region of Non-Secure Context Bank
	for 64bit system, select Long Descriptor -> 1(SMMU_CB_TTBCR.cb_ttbcr_des) */
	cb_ttbcr.value = (unsigned int)ioread32((void *)SOC_SMMU_CB_TTBCR_ADDR(io_comm_base));
	cb_ttbcr.reg.cb_ttbcr_des = 1;
	iowrite32(cb_ttbcr.value, (void *)SOC_SMMU_CB_TTBCR_ADDR(io_comm_base));

	/* set SMMU read/write phy addr in TLB miss case */
	smmu_rw_err_phy_addr = virt_to_phys((void *)smmu_rw_err_virtual_addr);
	low = (unsigned int)((smmu_rw_err_phy_addr + 0x80) & 0xffffffffffffff80);
	high = (unsigned int)((smmu_rw_err_phy_addr >> 32) & 0x7f);
	iowrite32(low, (void *)SOC_SMMU_ERR_RDADDR_ADDR(io_comm_base));
	iowrite32(low, (void *)SOC_SMMU_ERR_WRADDR_ADDR(io_comm_base));

	addr_msb.value = (unsigned int)ioread32((void *)SOC_SMMU_ADDR_MSB_ADDR(io_comm_base));
	addr_msb.reg.msb_errrd = high;
	addr_msb.reg.msb_errwr = high;
	iowrite32(addr_msb.value, (void *)SOC_SMMU_ADDR_MSB_ADDR(io_comm_base));

	return;
}

void npu_smmu_init(unsigned int coreID)
{
	CHECK_COREID_INVALID_RETURN_VOID(coreID);

	npu_smmu_mstr_init(coreID);
	npu_smmu_comm_init(coreID, smmu_manager.smmu_ttbr0, (unsigned long)smmu_manager.smmu_rw_err_virtual_addr);
}

void npu_smmu_bypass_init(unsigned int coreID)
{
	CHECK_COREID_INVALID_RETURN_VOID(coreID);
	iowrite32(SMMU_MSTR_GLB_BYPASS_BYPASS_MODE, (void *)SOC_SMMU_MSTR_GLB_BYPASS_ADDR((unsigned long)smmu_manager.master_io_addr[coreID]));
}

void npu_smmu_exit(unsigned int coreID)
{
	CHECK_COREID_INVALID_RETURN_VOID(coreID);

	iowrite32(SMMU_MSTR_INTCLR_ALL_MASK,
		(void *)SOC_SMMU_MSTR_INTMASK_ADDR((unsigned long)smmu_manager.master_io_addr[coreID]));
	iowrite32(SMMU_COMMON_INTCLR_NS_ALL_MASK,
		(void *)SOC_SMMU_INTMASK_NS_ADDR((unsigned long)smmu_manager.common_io_addr[coreID]));
}

static long npu_smmu_map_va(struct map_data *map)
{
	long ret;
	size_t sg_size;
	unsigned long phys_len;
	unsigned long iova_size;
	unsigned long iova_start;
	struct scatterlist *sg;
	struct sg_table *table;
	struct scatterlist *sgl;
	struct map_format *format;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
	struct dma_buf_attachment *attach;
	struct dma_buf *dma_buf;
#else
	struct ion_handle *hdl;
#endif

	if (!map) {
		NPU_ERR("input parameter arg is NULL, FATAL arg and ignore\n");
		return -EINVAL;
	}

	format = &(map->format);

	if (0 == npu_smmu_domain || 0 == npu_iova_pool) {
		NPU_ERR("npu_smmu_domain or npu_iova_pool is NULL, npu_smmu_domain=0x%pK, npu_iova_pool=0x%pK\n", npu_smmu_domain, npu_iova_pool);
		return -EFAULT;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0))
	if (0 == npu_ion_client ) {
		NPU_ERR("npu_ion_client is NULL\n");
		return -EFAULT;
	}

	hdl = ion_import_dma_buf(npu_ion_client, map->share_fd);// coverity[UNINIT]
	if (IS_ERR_OR_NULL(hdl)) {
		NPU_ERR("SETCONFIG_MAP hdl is error, which is: %pK\n",hdl);
		return -EFAULT;
	}

	table = ion_sg_table(npu_ion_client, hdl);
	if (IS_ERR_OR_NULL(table) || !table) { /* Coverity can not understand IS_ERR_OR_NULL, add "!table" */
		NPU_ERR("SETCONFIG_MAP table is error, which is: %pK\n", table);
		ion_free(npu_ion_client, hdl);
		return -EFAULT;
	}

#else
	if (0 == npu_dev) {
		printk(KERN_ERR"[%s]: IPU_ERROR:npu_dev is NULL\n", __func__);
		return -EFAULT;
	}

	dma_buf = dma_buf_get(map->share_fd);
	if (IS_ERR_OR_NULL(dma_buf)) {
		NPU_ERR("dma_buf_get fail, share_fd=%d, dma_buf=%pK\n", map->share_fd, dma_buf);
		return -EFAULT;
	}

	attach = dma_buf_attach(dma_buf, npu_dev);
	if (IS_ERR_OR_NULL(attach)) {
		NPU_ERR("dma_buf_attach error, attach=%pK\n", attach);
		dma_buf_put(dma_buf);
		return -EFAULT;
	}

	table = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
	if (IS_ERR_OR_NULL(table)) {
		NPU_ERR("dma_buf_map_attachment error, table=%pK\n", table);
		dma_buf_detach(dma_buf, attach);
		dma_buf_put(dma_buf);
		return -EFAULT;
	}
#endif

	sgl = table->sgl;

	for (phys_len = 0, sg = sgl; sg; sg = sg_next(sg))
		phys_len += (unsigned long)ALIGN(sg->length, PAGE_SIZE);/*lint !e50 */

	iova_size = phys_len;
	iova_start = npu_alloc_iova(npu_iova_pool, iova_size);

	sg_size = iommu_map_sg(npu_smmu_domain, iova_start, sgl,
			(unsigned int)sg_nents(sgl), format->prot);

	if (sg_size != iova_size) {
		NPU_ERR("map failed!iova_start = 0x%pK, iova_size = 0x%lx\n", (void *)iova_start, iova_size);

		if (npu_iova_pool) {
			npu_free_iova(npu_iova_pool, iova_start, iova_size);
		}
		ret = -EIO;
	} else {
		format->iova_start = iova_start;
		format->iova_size = iova_size;
		ret = 0;
	}

/* clear environment */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0))
	ion_free(npu_ion_client, hdl);
#else
	dma_buf_unmap_attachment(attach, table, DMA_BIDIRECTIONAL);
	dma_buf_detach(dma_buf, attach);
	dma_buf_put(dma_buf);
#endif

	return ret;
}

long npu_smmu_map(struct map_data *map)
{
	long ret_value = npu_smmu_map_va(map);
	if (ret_value) {
		NPU_ERR("ipu_smmu_map failed!\n");
		return ret_value;
	}

	mutex_lock(&npu_mem_mngr_mutex);
	npu_mem_mngr_add(map);
	mutex_unlock(&npu_mem_mngr_mutex);

	NPU_INFO("fd=%d,iova_start=0x%pK,iova_size=0x%lx,prot=%d\n",
		map->share_fd, (void *)map->format.iova_start, map->format.iova_size, map->format.prot);

	return 0;
}

long npu_smmu_reset_virt(struct map_data *map)
{
	long ret;

	if (!map) {
		NPU_ERR("input parameter arg is NULL, FATAL arg and ignore\n");
		return -EINVAL;
	}

	if (smmu_manager.smmu_reset_va) {
		NPU_ERR("virtual address for reset is exist, ignore\n");
		return -EBUSY;
	}

	ret = npu_smmu_map(map);
	smmu_manager.smmu_reset_va = map->format.iova_start;

	return ret;
}

static long npu_smmu_unmap_va(struct map_data *map)
{
	int ret;
	unsigned long unmapped_size;
	struct map_format *format = &(map->format);

	if (0 == npu_smmu_domain || 0 == npu_iova_pool) {
		NPU_ERR("npu_smmu_domain or npu_ion_client or npu_iova_pool is NULL, npu_smmu_domain=0x%pK, npu_iova_pool=0x%pK\n",
			npu_smmu_domain, npu_iova_pool);
		return -EFAULT;
	}

	/* check if iova is in iova pool range */
	ret = addr_in_gen_pool(npu_iova_pool, format->iova_start,
			format->iova_size);
	if(!ret) {
 		NPU_ERR("illegal para!!iova=0x%pK, size=%ld\n", (void *)format->iova_start, format->iova_size);
		return -EINVAL;
	}

	unmapped_size = iommu_unmap(npu_smmu_domain, format->iova_start, format->iova_size);

	if (unmapped_size != format->iova_size) {
		NPU_ERR("unmap failed, unmapped_size=%ld, iova_size=%ld\n", unmapped_size, format->iova_size);
		ret = -EINVAL;
	} else {
		npu_free_iova(npu_iova_pool, format->iova_start, format->iova_size);
		ret = 0;
	}

	return ret;
}

long npu_smmu_unmap_proc(struct map_data *map)
{
	long ret_value;

	NPU_INFO("fd=%d,iova_start=0x%pK,iova_size=0x%lx,prot=%d\n",
		map->share_fd, (void*)map->format.iova_start, map->format.iova_size, map->format.prot);

	if (npu_mem_mngr_del(map)) {
		NPU_ERR("invalid map data to unmap\n");
		return -EINVAL;
	}

	ret_value = npu_smmu_unmap_va(map);
	if (ret_value) {
		NPU_ERR("ipu_smmu_map failed!\n");
		return ret_value;
	}


	if (smmu_manager.smmu_reset_va == map->format.iova_start) {
		smmu_manager.smmu_reset_va = 0;
	}

	return 0;
}

long npu_smmu_unmap(struct map_data *map)
{
	long ret;

	mutex_lock(&npu_mem_mngr_mutex);
	ret = npu_smmu_unmap_proc(map);
	mutex_unlock(&npu_mem_mngr_mutex);

	return ret;
}

int npu_smmu_mngr_init(struct device *dev, unsigned int bandwidth_lmt_strategy)
{
	int loop;
	int ret;
	int property_rd;
	unsigned int base_addr = 0;
	char *smmu_mstr_name[MAX_SUPPORT_CORE_NUM] = {"smmu_mstr_core0", "smmu_mstr_core1"};
	char *smmu_comm_name[MAX_SUPPORT_CORE_NUM] = {"smmu_comm_core0", "smmu_comm_core1"};
	int core_number = (NPU_VERSION_V200 == hard_version) ? MAX_SUPPORT_CORE_NUM: (NPU_CORE_0 + 1);
	struct device_node *node;

	if (!dev) {
		NPU_ERR("dev ptr is NULL\n");
		return -EINVAL;
	}

	memset(&smmu_manager, 0, sizeof(smmu_manager));

	node = of_find_node_by_name(dev->of_node, "smmu");
	if(!node) {
		NPU_ERR("find smmu_master node error\n");
		ret = -EINVAL;
		return -EINVAL;
	}

	for (loop=0; loop < core_number; loop++) {
		property_rd = of_property_read_u32(node, smmu_mstr_name[loop], &base_addr);
		if (property_rd) {
			NPU_ERR("read property of pmctrl offset error\n");
			ret = -EINVAL;
			goto do_clear_ioremap;
		}

		smmu_manager.master_io_addr[loop] = ioremap((unsigned long)base_addr, (unsigned long)0xffff);
		if (!smmu_manager.master_io_addr[loop]) {
			NPU_ERR("smmu_manager.master_io_addr ioremap fail, loop=%d\n", loop);
			ret = -EINVAL;
			goto do_clear_ioremap;
		}

		property_rd = of_property_read_u32(node, smmu_comm_name[loop], &base_addr);
		if (property_rd) {
			NPU_ERR(":read property of pmctrl offset error\n");
			ret = -EINVAL;
			goto do_clear_ioremap;
		}

		smmu_manager.common_io_addr[loop] = ioremap((unsigned long)base_addr, (unsigned long)0xffff);
		if (!smmu_manager.common_io_addr[loop]) {
			NPU_ERR("smmu_manager.common_io_addr ioremap fail, loop=%d\n", loop);
			ret = -EINVAL;
			goto do_clear_ioremap;
		}
	}

	property_rd = of_property_read_u32(node, "smmu_port_select", &smmu_manager.smmu_port_select);
	if (property_rd) {
		NPU_ERR("read property of smmu_port_select error\n");
		ret = -EINVAL;
		goto do_clear_ioremap;
	}

	property_rd = of_property_read_u32(node, "smmu_hardware_start", &smmu_manager.smmu_mstr_hardware_start);
	if (property_rd) {
		NPU_ERR("read property of smmu_hardware_start error\n");
		ret = -EINVAL;
		goto do_clear_ioremap;
	}

	ret = npu_bandwidth_lmt_init(dev, bandwidth_lmt_strategy);
	if (ret) {
		NPU_ERR("Failed to npu_bandwidth_lmt_init, ret=%d\n", ret);
		goto do_clear_ioremap;
	}

	ret = npu_smmu_get_err_phy(dev);
	if (ret) {
		NPU_ERR("Failed to ipu_smmu_get_err_phy, ret=%d\n", ret);
		goto do_npu_bandwidth_lmt_exit;
	}

	ret = npu_smmu_get_base_phy(dev);
	if (ret) {
		NPU_ERR("Failed to ipu_smmu_get_base_phy, ret=%d\n", ret);
		goto do_ipu_smmu_get_err_phy;
	}
	return 0;

do_ipu_smmu_get_err_phy:
	devm_kfree(dev, smmu_manager.smmu_rw_err_virtual_addr);
do_npu_bandwidth_lmt_exit:
	npu_bandwidth_lmt_exit();
do_clear_ioremap:
	for (loop=0; loop < core_number; loop++) {
		if (smmu_manager.master_io_addr[loop]) {
			iounmap(smmu_manager.master_io_addr[loop]);
		}

		if (smmu_manager.common_io_addr[loop]) {
			iounmap(smmu_manager.common_io_addr[loop]);
		}
	}

	memset(&smmu_manager, 0, sizeof(smmu_manager));
	npu_smmu_domain = NULL;
	npu_iova_pool = NULL;
	return ret;
}

void npu_smmu_mngr_exit(struct device *dev)
{
	int loop;
	int core_number = (NPU_VERSION_V200 == hard_version) ? MAX_SUPPORT_CORE_NUM: (NPU_CORE_0 + 1);

	for(loop=0; loop<core_number; loop++){
		if (smmu_manager.master_io_addr[loop]) {
			iounmap(smmu_manager.master_io_addr[loop]);
			smmu_manager.master_io_addr[loop] = 0;
		}

		if (smmu_manager.common_io_addr[loop]) {
			iounmap(smmu_manager.common_io_addr[loop]);
			smmu_manager.common_io_addr[loop] = 0;
		}
	}

	devm_kfree(dev, smmu_manager.smmu_rw_err_virtual_addr);
	memset(&smmu_manager, 0, sizeof(smmu_manager));

	if (npu_smmu_domain) {
		iommu_domain_free(npu_smmu_domain);
	}
	npu_smmu_domain = NULL;

	if (npu_iova_pool) {
		gen_pool_destroy(npu_iova_pool);
	}
	npu_iova_pool = NULL;
}

void npu_smmu_override_prefetch_addr(unsigned int coreID)
{
	CHECK_COREID_INVALID_RETURN_VOID(coreID);

	iowrite32(smmu_manager.smmu_reset_va, (void *)SOC_SMMU_OPREF_ADDR_ADDR((unsigned long)smmu_manager.common_io_addr[coreID]));
	iowrite32(SMMU_OPREF_CTRL_CONFIG_DUMMY, (void *)SOC_SMMU_OPREF_CTRL_ADDR((unsigned long)smmu_manager.common_io_addr[coreID]));

	NPU_DEBUG("done\n");
}

bool npu_smmu_interrupt_handler(unsigned int coreID)
{
	unsigned long mstr_io_addr;
	unsigned long comm_io_addr;
	struct smmu_irq_count *irq_count = &smmu_manager.irq_count;
	unsigned int reg_smmu_mstr_status;
	unsigned int reg_smmu_comm_status;
	bool ret = false;

	if (coreID >= MAX_SUPPORT_CORE_NUM || (NPU_VERSION_V200 != hard_version && coreID == 1)) {
		NPU_ERR("invalid coreID=%d\n", coreID);
		return false;
	}

	mstr_io_addr = (unsigned long)smmu_manager.master_io_addr[coreID];
	comm_io_addr = (unsigned long)smmu_manager.common_io_addr[coreID];

	//fixme: if security/protect mode is needed, add code here
	reg_smmu_comm_status = ioread32((void *)SOC_SMMU_INTRAW_NS_ADDR(comm_io_addr));
	reg_smmu_mstr_status = ioread32((void *)SOC_SMMU_MSTR_INTRAW_ADDR(mstr_io_addr));

	if (0 != reg_smmu_mstr_status) {
		ret = true;
		NPU_ERR("error, smmu mstr interrupt received: %x\n", reg_smmu_mstr_status);
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

		NPU_ERR("Rd_Inst_SID=0x%pK, RdAddr=0x%pK, Wr_Inst_SID=0x%pK, WrAddr=0x%pK\n",
			(void *)(unsigned long)ioread32((void *)SOC_SMMU_MSTR_DBG_0_ADDR(mstr_io_addr)), (void *)(unsigned long)ioread32((void *)SOC_SMMU_MSTR_DBG_1_ADDR(mstr_io_addr)),
			(void *)(unsigned long)ioread32((void *)SOC_SMMU_MSTR_DBG_2_ADDR(mstr_io_addr)), (void *)(unsigned long)ioread32((void *)SOC_SMMU_MSTR_DBG_3_ADDR(mstr_io_addr))); //lint !e571

		NPU_ERR("RW_Burst_len=0x%pK, Awaddr=0x%pK\n",
			(void *)(unsigned long)ioread32((void *)SOC_SMMU_MSTR_DBG_4_ADDR(mstr_io_addr)), (void *)(unsigned long)ioread32((void *)SOC_SMMU_MSTR_DBG_5_ADDR(mstr_io_addr))); //lint !e571

		/* clear smmu mstr interrupt */
		iowrite32(SMMU_MSTR_INTCLR_ALL, (void *)SOC_SMMU_MSTR_INTCLR_ADDR(mstr_io_addr));
	}

	if (0 != reg_smmu_comm_status) {
		NPU_ERR("smmu common interrupt received: 0x%x\n", reg_smmu_comm_status);

		if (reg_smmu_comm_status & SMMU_INTCLR_NS_PTW_NS_STAT) {
			/* When PTW transaction receive an page table whose ns bit is not match to the prefetch transaction, occur this fault. */
			irq_count->comm_ptw_ns_stat++;
		}
		if (reg_smmu_comm_status & SMMU_INTCLR_NS_PTW_INVALID_STAT) {
			/* When PTW transaction receive an invalid page table descriptor or access the invalid regoin between t0sz and t1sz in long descriptor mode, occur this fault.*/
			irq_count->comm_ptw_invalid_stat++;
		}
		if (reg_smmu_comm_status & SMMU_INTCLR_NS_PTW_TRANS_STAT) {
			/* When PTW transaciont receive an error response, occur this fault. */
			irq_count->comm_ptw_trans_stat++;
		}
		if (reg_smmu_comm_status & SMMU_INTCLR_NS_TLBMISS_STAT) {
			ret = true;
			/* When there is a TLB miss generated during the translation process, the mmu will record this. */
			irq_count->comm_tlbmiss_stat++;
		}
		if (reg_smmu_comm_status & SMMU_INTCLR_NS_EXT_STAT) {
			ret = true;
			/* When mmu receive an en error response the mmu will record this as a fault. */
			irq_count->comm_ext_stat++;
		}
		if (reg_smmu_comm_status & SMMU_INTCLR_NS_PERMIS_STAT) {
			ret = true;
			/* When the input transaction¡¯s attributes doesn¡¯t match the attributes descripted in the page table, the mmu will raise a fault for this. */
			irq_count->comm_permis_stat++;
		}

		/* clear smmu interrupt */
		//fixme: if security/protect mode is needed, add code here
		iowrite32(SMMU_COMMON_INTCLR_NS_ALL, (void *)SOC_SMMU_MSTR_END_ACK_0_ADDR(comm_io_addr));
	}

	return ret;
}

void npu_smmu_reset_statistic(unsigned int coreID)
{
	int i;
	void *dbg_port_in;

	CHECK_COREID_INVALID_RETURN_VOID(coreID);
	dbg_port_in = (void *)SOC_SMMU_MSTR_DBG_PORT_IN_0_ADDR((unsigned long)smmu_manager.master_io_addr[coreID]);
	memset(&smmu_manager.smmu_stat[coreID], 0, sizeof(smmu_manager.smmu_stat[coreID]));
	smmu_manager.smmu_stat[coreID].coreID = coreID;

	/* clean read channel cmd-total-count (by config SMMU_MSTR_DBG_PORT_IN_0) */
	for(i = 0; i < NPU_SMMU_READ_STREAM_NUMBER; i++) {
		iowrite32(SOC_SMMU_MSTR_RD_CMD_TOTAL_CNT_ADDR((unsigned long)SMMU_MSTR_SET_DEBUG_PORT, i), dbg_port_in);
	}

	/* clean read channel cmd-miss-count (by config SMMU_MSTR_DBG_PORT_IN_0) */
	for(i = 0; i < NPU_SMMU_READ_STREAM_NUMBER; i++) {
		iowrite32(SOC_SMMU_MSTR_RD_CMD_MISS_CNT_ADDR((unsigned long)SMMU_MSTR_SET_DEBUG_PORT, i), dbg_port_in);
	}

	/* clean read channel data-length-count (by config SMMU_MSTR_DBG_PORT_IN_0) */
	for(i = 0; i < NPU_SMMU_READ_STREAM_NUMBER; i++) {
		iowrite32(SOC_SMMU_MSTR_RD_DATA_TOTAL_CNT_ADDR((unsigned long)SMMU_MSTR_SET_DEBUG_PORT, i), dbg_port_in);
	}

	/* clean read channel tag-stat (by config SMMU_MSTR_DBG_PORT_IN_0) */
	for(i = 0; i < NPU_SMMU_TAG_COMPARE_CASE_NUMBER; i++) {
		iowrite32(SOC_SMMU_MSTR_RD_CMD_CASE_CNT_ADDR((unsigned long)SMMU_MSTR_SET_DEBUG_PORT, i), dbg_port_in);
	}

	/* clean read channel latency (by config SMMU_MSTR_DBG_PORT_IN_0) */
	iowrite32(SOC_SMMU_MSTR_RD_CMD_TRANS_LATENCY_ADDR((unsigned long)SMMU_MSTR_SET_DEBUG_PORT), dbg_port_in);

	/* clean write channel cmd-total-count (by config SMMU_MSTR_DBG_PORT_IN_0) */
	iowrite32(SOC_SMMU_MSTR_WR_CMD_TOTAL_CNT_ADDR((unsigned long)SMMU_MSTR_SET_DEBUG_PORT, 0), dbg_port_in);

	/* clean write channel cmd-miss-count (by config SMMU_MSTR_DBG_PORT_IN_0) */
	iowrite32(SOC_SMMU_MSTR_WR_CMD_MISS_CNT_ADDR((unsigned long)SMMU_MSTR_SET_DEBUG_PORT, 0), dbg_port_in);

	/* clean write channel data-length-count (by config SMMU_MSTR_DBG_PORT_IN_0) */
	iowrite32(SOC_SMMU_MSTR_WR_DATA_TOTAL_CNT_ADDR((unsigned long)SMMU_MSTR_SET_DEBUG_PORT, 0), dbg_port_in);

	/* clean write channel tag-stat (by config SMMU_MSTR_DBG_PORT_IN_0) */
	for(i = 0; i < NPU_SMMU_TAG_COMPARE_CASE_NUMBER; i++) {
		iowrite32(SOC_SMMU_MSTR_WR_CMD_CASE_CNT_ADDR((unsigned long)SMMU_MSTR_SET_DEBUG_PORT, 0), dbg_port_in);
	}

	/* clean write channel latency (by config SMMU_MSTR_DBG_PORT_IN_0) */
	iowrite32(SOC_SMMU_MSTR_WR_CMD_TRANS_LATENCY_ADDR((unsigned long)SMMU_MSTR_SET_DEBUG_PORT), dbg_port_in);
	iowrite32(SOC_SMMU_MSTR_WR_CMD_TRANS_LATENCY_ADDR((unsigned long)SMMU_MSTR_SET_DEBUG_PORT), dbg_port_in);

}

void npu_smmu_record_statistic(unsigned int coreID)
{
	unsigned long mstr_io_addr;
	struct smmu_statistic *statistic;

	CHECK_COREID_INVALID_RETURN_VOID(coreID);

	mstr_io_addr = (unsigned long)smmu_manager.master_io_addr[coreID];
	statistic = &smmu_manager.smmu_stat[coreID];

	/* read channel cmd total count */
	statistic->read_stream_cmd_total[0] += ioread32((void *)SOC_SMMU_MSTR_RD_CMD_TOTAL_CNT_ADDR(mstr_io_addr, 0));
	statistic->read_stream_cmd_total[1] += ioread32((void *)SOC_SMMU_MSTR_RD_CMD_TOTAL_CNT_ADDR(mstr_io_addr, 1));
	statistic->read_stream_cmd_total[2] += ioread32((void *)SOC_SMMU_MSTR_RD_CMD_TOTAL_CNT_ADDR(mstr_io_addr, 2));

	/* read channel cmd miss count */
	statistic->read_stream_cmd_miss[0] += ioread32((void *)SOC_SMMU_MSTR_RD_CMD_MISS_CNT_ADDR(mstr_io_addr, 0));
	statistic->read_stream_cmd_miss[1] += ioread32((void *)SOC_SMMU_MSTR_RD_CMD_MISS_CNT_ADDR(mstr_io_addr, 1));
	statistic->read_stream_cmd_miss[2] += ioread32((void *)SOC_SMMU_MSTR_RD_CMD_MISS_CNT_ADDR(mstr_io_addr, 2));

	/* read channel data total count */
	statistic->read_stream_data_total[0] += ioread32((void *)SOC_SMMU_MSTR_RD_DATA_TOTAL_CNT_ADDR(mstr_io_addr, 0));
	statistic->read_stream_data_total[1] += ioread32((void *)SOC_SMMU_MSTR_RD_DATA_TOTAL_CNT_ADDR(mstr_io_addr, 1));
	statistic->read_stream_data_total[2] += ioread32((void *)SOC_SMMU_MSTR_RD_DATA_TOTAL_CNT_ADDR(mstr_io_addr, 2));

	/* read cmd miss/hit and latency */
	statistic->read_stream_cmd_miss_valid += ioread32((void *)SOC_SMMU_MSTR_RD_CMD_CASE_CNT_ADDR(mstr_io_addr, 0));
	statistic->read_stream_cmd_miss_pending += ioread32((void *)SOC_SMMU_MSTR_RD_CMD_CASE_CNT_ADDR(mstr_io_addr, 1));
	statistic->read_stream_cmd_hit_valid_not_slide_window += ioread32((void *)SOC_SMMU_MSTR_RD_CMD_CASE_CNT_ADDR(mstr_io_addr, 2));
	statistic->read_stream_cmd_hit_valid_slide_window += ioread32((void *)SOC_SMMU_MSTR_RD_CMD_CASE_CNT_ADDR(mstr_io_addr, 3));
	statistic->read_stream_cmd_hit_pending_not_slide_window += ioread32((void *)SOC_SMMU_MSTR_RD_CMD_CASE_CNT_ADDR(mstr_io_addr, 4));
	statistic->read_stream_cmd_hit_pending_slide_window += ioread32((void *)SOC_SMMU_MSTR_RD_CMD_CASE_CNT_ADDR(mstr_io_addr, 5));
	statistic->read_stream_cmd_latency += ioread32((void *)SOC_SMMU_MSTR_RD_CMD_CASE_CNT_ADDR(mstr_io_addr, 6));

	/* write channel cmd total count */
	statistic->write_stream_cmd_total += ioread32((void *)SOC_SMMU_MSTR_WR_CMD_TOTAL_CNT_ADDR(mstr_io_addr, 0));
	statistic->write_stream_cmd_miss += ioread32((void *)SOC_SMMU_MSTR_WR_CMD_MISS_CNT_ADDR(mstr_io_addr, 0));
	statistic->write_stream_data_total += ioread32((void *)SOC_SMMU_MSTR_WR_DATA_TOTAL_CNT_ADDR(mstr_io_addr, 0));

	/* write cmd miss/hit and latency */
	statistic->write_stream_cmd_miss_valid += ioread32((void *)SOC_SMMU_MSTR_WR_CMD_CASE_CNT_ADDR(mstr_io_addr, 0));
	statistic->write_stream_cmd_miss_pending += ioread32((void *)SOC_SMMU_MSTR_WR_CMD_CASE_CNT_ADDR(mstr_io_addr, 1));
	statistic->write_stream_cmd_hit_valid_not_slide_window += ioread32((void *)SOC_SMMU_MSTR_WR_CMD_CASE_CNT_ADDR(mstr_io_addr, 2));
	statistic->write_stream_cmd_hit_valid_slide_window += ioread32((void *)SOC_SMMU_MSTR_WR_CMD_CASE_CNT_ADDR(mstr_io_addr, 3));
	statistic->write_stream_cmd_hit_pending_not_slide_window += ioread32((void *)SOC_SMMU_MSTR_WR_CMD_CASE_CNT_ADDR(mstr_io_addr, 4));
	statistic->write_stream_cmd_hit_pending_slide_window += ioread32((void *)SOC_SMMU_MSTR_WR_CMD_CASE_CNT_ADDR(mstr_io_addr, 5));
	statistic->write_stream_cmd_latency += ioread32((void *)SOC_SMMU_MSTR_WR_CMD_TRANS_LATENCY_ADDR(mstr_io_addr));
}

long npu_smmu_report_statistic(SMMU_STAT_S *statistic)
{
	unsigned int coreID;

	if (!statistic) {
		NPU_ERR("statistic is NULL,ignore\n");
		return -EINVAL;
	}

	coreID = statistic->coreID;
	CHECK_COREID_INVALID_RETURN(coreID);

	memcpy(statistic, &smmu_manager.smmu_stat[coreID], sizeof(*statistic));
	return 0;
}

/* for online layer-by-layer mode, run once each op(i.e. conv, pooling, ReLU...), while online merge and offline mode
this func will only call once before run */
void npu_smmu_pte_update(unsigned int coreID)
{
	unsigned long mstr_io_addr;
	unsigned long comm_io_addr;

	CHECK_COREID_INVALID_RETURN_VOID(coreID);

	mstr_io_addr = (unsigned long)smmu_manager.master_io_addr[coreID];
	comm_io_addr = (unsigned long)smmu_manager.common_io_addr[coreID];

	iowrite32(SMMU_MSTR_SMRX_START_ALL_STREAM, (void *)SOC_SMMU_MSTR_SMRX_START_0_ADDR(mstr_io_addr));

	/* update cache data to avoid this case: phy address across 8GB address-space */
	iowrite32(SMMU_CACHE_ALL_LEVEL_INVALID_LEVEL1, (void *)SOC_SMMU_SCACHEI_ALL_ADDR(comm_io_addr));
	iowrite32(SMMU_CACHE_ALL_LEVEL_VALID_LEVEL1, (void *)SOC_SMMU_SCACHEI_ALL_ADDR(comm_io_addr));

	/* update cache data to avoid this case: phy address across 8GB address-space */
	iowrite32(SMMU_CACHE_ALL_LEVEL_INVALID_LEVEL2, (void *)SOC_SMMU_SCACHEI_ALL_ADDR(comm_io_addr));
	iowrite32(SMMU_CACHE_ALL_LEVEL_VALID_LEVEL2, (void *)SOC_SMMU_SCACHEI_ALL_ADDR(comm_io_addr));
}

void npu_smmu_dump_strm(unsigned int coreID)
{
	int i, j;
	unsigned int base;
	unsigned int port_out[NPU_SMMU_MSTR_DEBUG_PORT_NUM];
	unsigned int port_in[NPU_SMMU_MSTR_DEBUG_BASE_NUM] = {
		NPU_SMMU_MSTR_DEBUG_AXI_RD_CMD_ADDR, NPU_SMMU_MSTR_DEBUG_AXI_RD_CMD_INFO,
		NPU_SMMU_MSTR_DEBUG_AXI_WR_CMD_ADDR, NPU_SMMU_MSTR_DEBUG_AXI_WR_CMD_INFO};

	int sz = REGISTER_INFO_MAX_LEN;
	char *perr = register_info;
	int dsm_offset = 0;

	unsigned long mstr_io_addr;
	struct npu_mstr_reg_s mstr_reg;

	CHECK_COREID_INVALID_RETURN_VOID(coreID);
	mstr_io_addr = (unsigned long)smmu_manager.master_io_addr[coreID];

	for (i = 0; i < NPU_SMMU_MSTR_DEBUG_BASE_NUM; i++) {

		base = port_in[i];

		for (j = 0; j < NPU_SMMU_MSTR_DEBUG_PORT_NUM; j ++) {
			iowrite32(base + j * sizeof(unsigned int), (void *)SOC_SMMU_MSTR_DBG_PORT_IN_0_ADDR(mstr_io_addr));
			port_out[j] = ioread32((void *)SOC_SMMU_MSTR_DBG_PORT_OUT_ADDR(mstr_io_addr));
		}

		/* dump strm status for analysis */
		NPU_INFO("SMMU STRM %x:%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n", base,
			port_out[0], port_out[1], port_out[2],	port_out[3],  port_out[4],	port_out[5],  port_out[6],	port_out[7],
			port_out[8], port_out[9], port_out[10], port_out[11], port_out[12], port_out[13], port_out[14], port_out[15]);

		NPU_INFO("(continue):%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n",
			port_out[16], port_out[17], port_out[18], port_out[19], port_out[20], port_out[21], port_out[22], port_out[23],
			port_out[24], port_out[25], port_out[26], port_out[27], port_out[28], port_out[29], port_out[30], port_out[31]);

		if (0 < dsm_offset){
			perr += dsm_offset;
			sz -= dsm_offset;
		}
		/* coverity[secure_coding] */
		dsm_offset = snprintf(perr, sz, "%d: %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n", base,
		port_out[0], port_out[1], port_out[2],	port_out[3],  port_out[4],	port_out[5],  port_out[6],	port_out[7],
		port_out[8], port_out[9], port_out[10], port_out[11], port_out[12], port_out[13], port_out[14], port_out[15],
		port_out[16], port_out[17], port_out[18], port_out[19], port_out[20], port_out[21], port_out[22], port_out[23],
		port_out[24], port_out[25], port_out[26], port_out[27], port_out[28], port_out[29], port_out[30], port_out[31]);

	}

	memset(&mstr_reg, 0, sizeof(struct npu_mstr_reg_s));

	iowrite32(NPU_SMMU_RD_CMD_BUF_BITMAP, (void *)SOC_SMMU_MSTR_DBG_PORT_IN_0_ADDR(mstr_io_addr));
	mstr_reg.rd_bitmap = ioread32((void *)SOC_SMMU_MSTR_DBG_PORT_OUT_ADDR(mstr_io_addr));

	iowrite32(NPU_SMMU_WR_CMD_BUF_BITMAP, (void *)SOC_SMMU_MSTR_DBG_PORT_IN_0_ADDR(mstr_io_addr));
	mstr_reg.wr_bitmap = ioread32((void *)SOC_SMMU_MSTR_DBG_PORT_OUT_ADDR(mstr_io_addr));

	mstr_reg.rd_cmd_total_cnt0 = ioread32((void *)SOC_SMMU_MSTR_RD_CMD_TOTAL_CNT_ADDR(mstr_io_addr, 0));
	mstr_reg.rd_cmd_total_cnt1 = ioread32((void *)SOC_SMMU_MSTR_RD_CMD_TOTAL_CNT_ADDR(mstr_io_addr, 1));
	mstr_reg.rd_cmd_total_cnt2 = ioread32((void *)SOC_SMMU_MSTR_RD_CMD_TOTAL_CNT_ADDR(mstr_io_addr, 2));
	mstr_reg.wr_cmd_total_cnt  = ioread32((void *)SOC_SMMU_MSTR_WR_CMD_TOTAL_CNT_ADDR(mstr_io_addr, 0));

	NPU_ERR("RD_BITMAP=%x, WR_BITMAP=%x, rd_cmd_total_cnt[0-3]={%x, %x, %x}, wr_cmd_total_cnt=%x\n",
		mstr_reg.rd_bitmap,
		mstr_reg.wr_bitmap ,
		mstr_reg.rd_cmd_total_cnt0,
		mstr_reg.rd_cmd_total_cnt1,
		mstr_reg.rd_cmd_total_cnt2,
		mstr_reg.wr_cmd_total_cnt);

	npu_mntn_mstr_reg_init(coreID, &mstr_reg);

}

void npu_smmu_set_stat_en(int enable)
{
	smmu_manager.stat_enable = enable;
}

int npu_smmu_get_stat_en(void)
{
    return smmu_manager.stat_enable;
}

