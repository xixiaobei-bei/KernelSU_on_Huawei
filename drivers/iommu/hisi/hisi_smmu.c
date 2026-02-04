/*
   how to lookup page table:
 1)cd sys/kernel/debug/hisi_smmu
 2)echo "start=0xxxxx size=0xxxxx" >./smmu_pgtbl
 3)cat ./smmu_pgtbl

 each pgd item is 1MB memory;
 each pte item is 4KB memory;
 */

#include <asm/pgalloc.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/iommu.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/hisi/hisi-iommu.h>
#include <linux/hisi/ion-iommu.h>

/*#define IOMMU_DEBUG*/
#ifndef IOMMU_DEBUG
#define dbg(format, arg...)
#else
#define dbg(format, arg...)  pr_err("[iommu]"format, ##arg)
#endif
#define SMMU_DBG_BUFFER       (SZ_8K)
#define INVALID_IO_ADDR		  (0xffffffff)
#define INVALID_IO_SIZE       (0xffffffff)
#define MAX_IO_ADDR			  (0xffffffff)

#define SMMU_PHY_PTRS_PER_PTE (256)
/*#define SMMU_PHY_PTRS_PER_PGD (4096)*/
#define SMMU_PTRS_PER_PGD     (1024)
#define SMMU_PTRS_PER_PTE     (1024)
#define SMMU_PAGE_SHIFT       (12)
#define SMMU_PAGE_SIZE        (UL(1) << SMMU_PAGE_SHIFT)
#define SMMU_PAGE_MASK	      (~(SMMU_PAGE_SIZE-1))

#define SMMU_PGDIR_SHIFT	  (22)
#define SMMU_PGDIR_SIZE		  (UL(1) << SMMU_PGDIR_SHIFT)
#define SMMU_PGDIR_MASK		  (~(SMMU_PGDIR_SIZE-1))

#define SMMU_PGD_TYPE_TABLE         (1)
#define SMMU_PGD_NS_BIT(x)          ((x) << 3)

#define SMMU_PTE_TYPE                       (0x2)
#define SMMU_PTE_C_B(x)                     ((x) << 2)
#define SMMU_PTE_TEX(x)                     ((x) << 6)
#define SMMU_PTE_XN_BIT(x)                  ((x) << 0)
#define SMMU_PTE_AP(high, low)               (((high) << 9)|((low) << 4))
#define SMMU_PTE_RW                         SMMU_PTE_AP(0, 3)
#define SMMU_PTE_RO                         SMMU_PTE_AP(1, 3)

/*
   This access permissions model applies
   when using the Short-descriptor translation tables format,
   and the SCTLR.AFE bit is set to 0.
 */

/*
#define SMMU_PHYS_SHIFT		        (32)
#define SMMU_PHYS_SIZE				(UL(1) << SMMU_PHYS_SHIFT)
#define SMMU_PHYS_MASK				(~(SMMU_PHYS_SIZE - 1))
*/
#define smmu_pte_index(addr) \
(((addr) >> SMMU_PAGE_SHIFT) & (SMMU_PTRS_PER_PTE - 1))
#define smmu_pgd_index(addr) \
(((addr) >> SMMU_PGDIR_SHIFT) & (SMMU_PTRS_PER_PGD - 1))
#define SMMU_PAGE_ALIGN(addr)       ALIGN(addr, PAGE_SIZE)

struct smmu_pgd_t {
	unsigned int pgd_entry[4];
};
typedef unsigned int smmu_pte_t;

/*smmu device object*/
struct hisi_smmu_device {
	struct device *dev;
	struct list_head domain_list;
	unsigned int ref_count;
	spinlock_t lock;
	unsigned long va_pgtable_addr;
	phys_addr_t smmu_phy_pgtable_addr;
};

struct hisi_map_tile_position {
	struct scatterlist *sg;
	unsigned long offset;
};

static struct hisi_smmu_device *hisi_smmu;
static unsigned int dbg_iova_start;
static unsigned int dbg_iova_size;

static inline unsigned int smmu_pgd_none(struct smmu_pgd_t pgd)
{
	return !(pgd.pgd_entry[0] ? pgd.pgd_entry[0] : 0);
}

static inline unsigned int smmu_pte_none(smmu_pte_t pte)
{
	return !(pte ? pte : 0);
}

static inline unsigned int pte_is_valid(smmu_pte_t *ptep)
{
	return (unsigned int)((*(ptep) & SMMU_PTE_TYPE) ? 1 : 0);
}

/* Find an entry in the second-level page table.. */
static inline void *smmu_pte_page_vaddr(struct smmu_pgd_t *pgd)
{
	unsigned int pte_table_addr = pgd->pgd_entry[0];

	return phys_to_virt(pte_table_addr & SMMU_PAGE_MASK);
}

/*fill the pgd entry, pgd value must be 32bit */
static inline void smmu_set_pgd(struct smmu_pgd_t *pgdp, unsigned int pgd)
{
	unsigned int i = 0;

	pgdp->pgd_entry[0] = pgd;
	pgdp->pgd_entry[1] = pgd + SMMU_PHY_PTRS_PER_PTE * sizeof(smmu_pte_t);
	pgdp->pgd_entry[2] =
	    pgd + 2 * SMMU_PHY_PTRS_PER_PTE * sizeof(smmu_pte_t);
	pgdp->pgd_entry[3] =
	    pgd + 3 * SMMU_PHY_PTRS_PER_PTE * sizeof(smmu_pte_t);
	for (i = 0; i < 4; i++)
		dbg("%s:pgd_entry[%d]=0x%x\n", __func__, i, pgdp->pgd_entry[i]);
	dsb(ishst);
	isb();
}

static inline void smmu_pgd_populate(struct smmu_pgd_t *pgdp, pgtable_t ptep,
				     pgdval_t prot)
{
	smmu_set_pgd(pgdp, (unsigned int)(page_to_phys(ptep) | prot));
}

static inline unsigned long smmu_pgd_addr_end(unsigned long addr,
					      unsigned long end)
{
	unsigned long boundary = (addr + SMMU_PGDIR_SIZE) & SMMU_PGDIR_MASK;

	return (boundary - 1 < end - 1) ? boundary : end;
}

/*transfer 32bit pte table pointer to struct page*/
static pgtable_t smmu_pgd_to_pte(unsigned int ppte_table)
{
	unsigned long page_table_addr;

	if (!ppte_table) {
		dbg("error: the pointer of pte_table is NULL\n");
		return NULL;
	}
	page_table_addr = (unsigned long)ppte_table;
	return phys_to_page(page_table_addr);
}

int iommu_map_tile(struct iommu_domain *domain, unsigned long iova,
		   struct scatterlist *sg, size_t size, int prot,
		   struct tile_format *format)
{
	if (unlikely(domain->ops->map_tile == NULL))
		return -ENODEV;

	BUG_ON(iova & (~PAGE_MASK));

	return domain->ops->map_tile(domain, iova, sg, size, prot, format);
}
EXPORT_SYMBOL_GPL(iommu_map_tile);

int iommu_unmap_tile(struct iommu_domain *domain, unsigned long iova,
		     size_t size)
{
	if (unlikely(domain->ops->unmap_tile == NULL))
		return -ENODEV;

	BUG_ON(iova & (~PAGE_MASK));

	return domain->ops->unmap_tile(domain, iova, size);
}
EXPORT_SYMBOL_GPL(iommu_unmap_tile);

int of_get_iova_info(struct device_node *np, unsigned long *iova_start,
		     unsigned long *iova_size, unsigned long *iova_align)
{
	struct device_node *node = NULL;
	unsigned long long align = SZ_256K;
	int ret = 0;

	if (!np)
		return -ENODEV;

	node = of_find_node_by_name(np, "iommu_info"); /*lint !e838*/
	if (!node) {
		dbg("find iommu_info node error\n");
		return -ENODEV;
	}
	ret = of_property_read_u32(node, "start-addr",
				   (u32 *)iova_start); /*lint !e838*/
	if (ret) {
		dbg("read iova start address error\n");
		ret = -EINVAL;
		goto read_error;
	}
	ret = of_property_read_u32(node, "size", (u32 *)iova_size);
	if (ret) {
		dbg("read iova size error\n");
		ret = -EINVAL;
		goto read_error;
	}
	ret = of_property_read_u64(node, "iova-align", (u64 *)iova_align);
	if (ret)
		memcpy(iova_align, &align, sizeof(*iova_align));/*[false alarm]:devoid of risk*/ /* unsafe_function_ignore: memcpy  */

	pr_err("%s:start_addr 0x%lx, size 0x%lx align 0x%lx\n",
			__func__, *iova_start, *iova_size,
			*iova_align);

	return 0;

read_error:
	return ret;
}

static void hisi_smmu_flush_pgtable(void *addr, size_t size)
{
	__flush_dcache_area(addr, size);
}

static void hisi_smmu_free_ptes(struct smmu_pgd_t pgd)
{
	pgtable_t table = smmu_pgd_to_pte(pgd.pgd_entry[0]);

	if (!table) {
		pr_err("pte table is null\n");
		return;
	}
	pgtable_page_dtor(table);
	__free_page(table);
	smmu_set_pgd(&pgd, 0);
}

static void hisi_smmu_free_pgtables(unsigned long *page_table_addr)
{
	int i;
	struct smmu_pgd_t *pgd;
	unsigned long flags;

	pgd = (struct smmu_pgd_t *)page_table_addr;
	spin_lock_irqsave(&hisi_smmu->lock, flags);
	for (i = 0; i < SMMU_PTRS_PER_PGD; ++i) {
		if (smmu_pgd_none(*pgd))
			continue;
		hisi_smmu_free_ptes(*pgd);
		pgd++;
	}
	memset((void *)page_table_addr, 0, SZ_16K); /* unsafe_function_ignore: memset  */
	spin_unlock_irqrestore(&hisi_smmu->lock, flags);
}

static struct iommu_domain *hisi_smmu_domain_alloc(unsigned iommu_domain_type)
{
	struct iommu_domain *domain;
	struct iommu_domain_data *iommu_info;

	if (iommu_domain_type != IOMMU_DOMAIN_UNMANAGED)
		return NULL;

	domain = kzalloc(sizeof(*domain), GFP_KERNEL);
	if (!domain)
		return NULL;

	iommu_info = kzalloc(sizeof(struct iommu_domain_data), GFP_KERNEL);
	if (!iommu_info)
		goto out_free;

	list_add(&iommu_info->list, &hisi_smmu->domain_list);

	domain->priv = iommu_info;

	return domain;

out_free:
	kfree(domain);

	return NULL;
}

static void hisi_smmu_domain_free(struct iommu_domain *domain)
{
	if (list_empty(&hisi_smmu->domain_list)) {
		hisi_smmu_free_pgtables((unsigned long *)
					hisi_smmu->va_pgtable_addr);
	}
	kfree(domain);
}

static int hisi_smmu_alloc_init_pte(struct smmu_pgd_t *ppgd,
				    unsigned long addr, unsigned long end,
				    unsigned long pfn, int prot, unsigned long *flags)
{
	smmu_pte_t *pte, *start;
	pgtable_t table;
	unsigned int pteval = SMMU_PTE_TYPE;

	if (!smmu_pgd_none(*ppgd))
		goto pte_ready;

	/* Allocate a new set of tables */
	spin_unlock_irqrestore(&hisi_smmu->lock, *flags);
	table = alloc_page(GFP_KERNEL | __GFP_ZERO | __GFP_DMA);
	spin_lock_irqsave(&hisi_smmu->lock, *flags);
	if (!table) {
		dbg("%s: alloc page fail\n", __func__);
		return -ENOMEM;
	}

	if (smmu_pgd_none(*ppgd)) {
		hisi_smmu_flush_pgtable(page_address(table), SMMU_PAGE_SIZE);
		if (prot & IOMMU_SEC)
			smmu_pgd_populate(ppgd, table,
					  SMMU_PGD_TYPE_TABLE |
					  SMMU_PGD_NS_BIT(0));
		else
			smmu_pgd_populate(ppgd, table,
					  SMMU_PGD_TYPE_TABLE |
					  SMMU_PGD_NS_BIT(1));
		hisi_smmu_flush_pgtable(ppgd, sizeof(*ppgd));
	} else
		__free_page(table);

pte_ready:
	start = (smmu_pte_t *) smmu_pte_page_vaddr(ppgd) + smmu_pte_index(addr);
	pte = start;

	/* by default, setting pages by read only ,
	 * non-cacheable attribute and memory type.
	 * ap[2]=0b1, ap[1:0]=0b11, tex=0b100 , c_b=0b00.
	 */
	if (!prot) {
		pteval |=
		    SMMU_PTE_RW | SMMU_PTE_TEX(4) | SMMU_PTE_C_B(0) |
		    SMMU_PTE_XN_BIT(0);
	} else {
		/*
		 * set page to device type.
		 */
		if (prot & IOMMU_DEVICE) {
			pteval |=
			    SMMU_PTE_RW | SMMU_PTE_TEX(0) | SMMU_PTE_C_B(1) |
			    SMMU_PTE_XN_BIT(1);
		} else {
			/*
			 *otherwise set page by memory type.
			 *include text section and data section.
			 */
			if (prot & IOMMU_CACHE)
				pteval |= SMMU_PTE_TEX(5) | SMMU_PTE_C_B(1);
			else
				pteval |= SMMU_PTE_TEX(4) | SMMU_PTE_C_B(0);

			if ((prot & IOMMU_READ) && (prot & IOMMU_WRITE))
				pteval |= SMMU_PTE_RW;
			else if ((prot & IOMMU_READ) && !(prot & IOMMU_WRITE))
				pteval |= SMMU_PTE_RO;
			else
				WARN_ON("you do not set read attribute!");
			if (prot & IOMMU_NOEXEC)
				pteval |= SMMU_PTE_XN_BIT(0);
			else
				pteval |= SMMU_PTE_XN_BIT(1);
		}
	}

	do {
		if (!pte_is_valid(pte))
			*pte = (unsigned int)(__pfn_to_phys(pfn) | pteval);
		else
			WARN_ONCE(1, "map the same address more times!\n");
		pte++;
		pfn++;
		addr += SMMU_PAGE_SIZE;
	} while (addr < end);

	hisi_smmu_flush_pgtable(start, sizeof(*pte) * (pte - start));
	return 0;
}

static int hisi_smmu_handle_mapping(struct iommu_domain *domain,
				    unsigned long iova, phys_addr_t paddr,
				    size_t size, int prot)
{
	int ret;
	unsigned long end;
	unsigned long next;
	unsigned long flags;
	struct smmu_pgd_t *pgd =
	    (struct smmu_pgd_t *)hisi_smmu->va_pgtable_addr;

	if (!pgd) {
		dbg("pgd is null\n");
		return -EINVAL;
	}
	size = ALIGN(size, SMMU_PAGE_SIZE);
	spin_lock_irqsave(&hisi_smmu->lock, flags);
	pgd += smmu_pgd_index(iova);
	end = iova + size;
	do {
		next = smmu_pgd_addr_end(iova, end);
		ret = hisi_smmu_alloc_init_pte(pgd, iova, next,
					       __phys_to_pfn(paddr), prot, &flags);
		if (ret)
			goto out_unlock;
		paddr += next - iova;
		iova = next;
	} while (pgd++, iova < end);
out_unlock:
	spin_unlock_irqrestore(&hisi_smmu->lock, flags);
	return ret;
}

static int hisi_smmu_map(struct iommu_domain *domain, unsigned long iova,
			 phys_addr_t paddr, size_t size, int prot)
{
	if (!domain) {
		dbg("domain is null\n");
		return -ENODEV;
	}
	return hisi_smmu_handle_mapping(domain, iova, paddr, size, prot);
}

static unsigned int hisi_smmu_clear_pte(struct smmu_pgd_t *pgdp,
					unsigned int iova, unsigned int end)
{
	smmu_pte_t *ptep = NULL;
	smmu_pte_t *pte = NULL;
	unsigned int size = end - iova;

	ptep = smmu_pte_page_vaddr(pgdp);
	pte = ptep + smmu_pte_index(iova);
	do {
		*pte = (unsigned int)0;
		pte++;
		iova += SMMU_PAGE_SIZE;
	} while (iova < end);

	return size;
}

static unsigned int hisi_smmu_handle_unmapping(struct iommu_domain *domain,
					       unsigned long iova, size_t size)
{
	struct smmu_pgd_t *pgdp = NULL;
	unsigned int end = 0;
	unsigned int next = 0;
	unsigned int unmap_size = 0;
	unsigned long flags;

	iova = SMMU_PAGE_ALIGN(iova);
	size = SMMU_PAGE_ALIGN(size);
	pgdp = (struct smmu_pgd_t *)hisi_smmu->va_pgtable_addr;
	end = iova + size;
	dbg("%s:end=0x%x\n", __func__, end);
	pgdp += smmu_pgd_index(iova);
	spin_lock_irqsave(&hisi_smmu->lock, flags);
	do {
		next = smmu_pgd_addr_end(iova, end);
		unmap_size += hisi_smmu_clear_pte(pgdp, iova, next);
		iova = next;
		dbg("%s: pgdp=%pK, iova=0x%lx\n", __func__, pgdp, iova);
	} while (pgdp++, iova < end);
	spin_unlock_irqrestore(&hisi_smmu->lock, flags);
	return unmap_size;
}

static size_t hisi_smmu_unmap(struct iommu_domain *domain, unsigned long iova,
			      size_t size)
{
	unsigned int ret;

	if (!domain) {
		dbg("domain is null\n");
		/*lint -e570 -esym(570,*)*/
		return -ENODEV;
		/*lint -e570 +esym(570,*)*/
	}
	/*unmapping the range of iova */
	ret = hisi_smmu_handle_unmapping(domain, iova, size);
	if (ret == size) {
		dbg("%s:unmap size:0x%x\n", __func__, (unsigned int)size);
		return size;
	} else
		return 0;
}

static phys_addr_t hisi_smmu_iova_to_phys(struct iommu_domain *domain,
					  dma_addr_t iova)
{
	struct smmu_pgd_t *pgdp, pgd;
	smmu_pte_t pte;

	pgdp = (struct smmu_pgd_t *)hisi_smmu->va_pgtable_addr;
	if (!pgdp)
		return 0;

	pgd = *(pgdp + smmu_pgd_index(iova));
	if (smmu_pgd_none(pgd))
		return 0;

	pte =
	    *((unsigned int *)smmu_pte_page_vaddr(&pgd) + smmu_pte_index(iova));
	if (smmu_pte_none(pte))
		return 0;

	return __pfn_to_phys(pte_pfn(pte)) | (iova & ~SMMU_PAGE_MASK);
}

static int hisi_attach_dev(struct iommu_domain *domain, struct device *dev)
{
	struct device_node *np = dev->of_node;
	int ret = 0;
	struct iommu_domain_data *data = (struct iommu_domain_data *)domain->priv;

	data->phy_pgd_base = hisi_smmu->smmu_phy_pgtable_addr;
	ret = of_get_iova_info(np, &data->iova_start, &data->iova_size,
			       &data->iova_align);
	return ret;
}

static void hisi_detach_dev(struct iommu_domain *domain, struct device *dev)
{
	struct iommu_domain_data *data;

	data = (struct iommu_domain_data *)domain->priv;
	if (data) {
		list_del(&data->list);
		domain->priv = NULL;
		kfree(data);
	} else {
		dbg("%s:error! data entry has been delected\n", __func__);
	}
	dbg("%s: device:%s not use iommu\n", __func__, dev_name(dev));
}

static dma_addr_t get_phys_addr(struct scatterlist *sg)
{
	dma_addr_t dma_addr = sg_dma_address(sg);

	if (dma_addr == 0)
		dma_addr = sg_phys(sg);
	return dma_addr;
}

/*
 *iova: the start address for tile mapping
 *size: the physical memory size
 *sg: the node of scatter list where are the start node of physical memory
 *sg_offset:the physical memory offset in the sg node ,where is the start
	 position of physical memory
 *port: the pape property of virtual memory
 * this function complete one row mapping.
 */
static size_t hisi_map_tile_row(struct iommu_domain *domain, unsigned long
				iova, size_t size, struct scatterlist *sg,
				size_t sg_offset,
				struct hisi_map_tile_position *map_position,
				unsigned int prot)
{

	unsigned long map_size;	/*the memory size that will be mapped */
	unsigned long phys_addr;
	unsigned long mapped_size = 0;	/*memory size that has been mapped */
	int ret;

	while (1) {
		/*
		 *get the remain memory,if current sg node is not enough memory,
		 *we map the remain memory firstly.
		 */
		map_size = size - mapped_size;
		if (map_size > (sg->length - sg_offset))
			map_size = (sg->length - sg_offset);

		/*get the start physical address */
		phys_addr = (unsigned long)get_phys_addr(sg) + sg_offset;
		ret =
		    hisi_smmu_map(domain, iova + mapped_size, phys_addr,
				  map_size, prot);
		if (ret) {
			pr_err("[%s] hisi_smmu_map failed!\n", __func__);
			break;
		}
		/*update mapped memory size */
		mapped_size += map_size;
		/*
		 * if mapping finished,we update the offset of current node
		 * and save the position. otherwise we clean the sg_offset
		 * to zero and get next sg node.
		 */
		if (mapped_size < size) {
			sg_offset = 0;
			sg = sg_next(sg);
			if (!sg) {
				pr_err
				    ("[%s] error physical memory not enough\n",
				     __func__);
				break;
			}
		} else {
			sg_offset += map_size;
			/*
			 * if physcial memory of this node is exhausted,
			 * we choose next node
			 */
			if (sg_offset == sg->length) {
				sg_offset = 0;
				sg = sg_next(sg);
			}
			break;
		}
	}
	/*save current position */
	map_position->sg = sg;
	map_position->offset = sg_offset;

	return mapped_size;
}

/*
 *domain:the iommu domain for mapping
 *iova:the start virtual address
 *sg: the scatter list of physical memory
 *size:the total size of all virtual memory
 *port:the property of page table of virtual memory
 *format:the parameter of tile mapping
 *this function map physical memory in tile mode
 */
static int hisi_smmu_map_tile(struct iommu_domain *domain, unsigned long iova,
			      struct scatterlist *sg, size_t size, int prot,
			      struct tile_format *format)
{

	unsigned int phys_length;
	struct scatterlist *sg_node;
	unsigned int row_number, row;
	unsigned int size_virt, size_phys;
	unsigned int sg_offset;
	int ret = 0;
	unsigned int mapped_size, header_size;
	struct hisi_map_tile_position map_position;

	ret = size;
	/* calculate the whole length of phys mem */
	for (phys_length = 0, sg_node = sg; sg_node; sg_node = sg_next(sg_node))
		phys_length += ALIGN(sg_node->length, SZ_4K);
	header_size = format->header_size;

	/* calculate the number of raws */
	row_number =
	    ((phys_length -
	      header_size) >> PAGE_SHIFT) / format->phys_page_line;
	dbg("phys_length: 0x%x , rows: 0x%x, header_size: 0x%x\n", phys_length,
	    row_number, header_size);

	/*caculate the need physical memory and virtual memory for one row */
	size_phys = (format->phys_page_line * PAGE_SIZE);
	size_virt = (format->virt_page_line * PAGE_SIZE);

	sg_offset = 0;
	sg_node = sg;

	/*set start position */
	map_position.sg = sg;
	map_position.offset = 0;

	/*map header */
	if (header_size) {
		mapped_size =
		    hisi_map_tile_row(domain, iova, header_size, sg_node,
				      sg_offset, &map_position, prot);
		if (mapped_size != header_size) {
			WARN(1, "map head fail\n");
			ret = -EINVAL;
			goto error;
		}
		iova += ALIGN(header_size, size_virt);
	}
	/* map row by row */
	for (row = 0; row < row_number; row++) {
		/* get physical memory position */
		if (map_position.sg) {
			sg_node = map_position.sg;
			sg_offset = map_position.offset;
		} else {
			pr_err("[%s]:physical memory is not enough\n",
			       __func__);
			break;
		}
		/* map one row */
		/*lint -e647 -esym(647,*)*/
		mapped_size =
		    hisi_map_tile_row(domain, iova + (size_virt * row),
				      size_phys, sg_node, sg_offset,
				      &map_position, prot);
		/*lint -e647 +esym(647,*)*/
		if (mapped_size != size_phys) {
			WARN(1, "hisi_map_tile_row failed!\n");
			ret = -EINVAL;
			break;
		}
	};
error:
	return ret;
}

static size_t hisi_smmu_unmap_tile(struct iommu_domain *domain,
				   unsigned long iova, size_t size)
{
	return hisi_smmu_unmap(domain, iova, size);
}

size_t hisi_iommu_map_sg(struct iommu_domain *domain, unsigned long iova,
			 struct scatterlist *sg, unsigned int nents, int prot)
{
	struct scatterlist *s;
	size_t mapped = 0;
	unsigned int i, min_pagesz;
	int ret;

	if (domain->ops->pgsize_bitmap == 0UL)
		return 0;

	min_pagesz = (unsigned int)1 << __ffs(domain->ops->pgsize_bitmap);

	for_each_sg(sg, s, nents, i) {
		phys_addr_t phys = page_to_phys(sg_page(s)) + s->offset;

		/*
		 * We are mapping on IOMMU page boundaries, so offset within
		 * the page must be 0. However, the IOMMU may support pages
		 * smaller than PAGE_SIZE, so s->offset may still represent
		 * an offset of that boundary within the CPU page.
		 */
		if (!IS_ALIGNED(s->offset, min_pagesz))
			goto out_err;

		ret = hisi_smmu_map(domain, iova + mapped, phys,
					(size_t)s->length, prot);
		if (ret)
			goto out_err;

		mapped += s->length;
	}

	return mapped;

out_err:
	/* undo mappings already done */
	hisi_smmu_unmap(domain, iova, mapped);

	return 0;
}


static struct iommu_ops hisi_smmu_ops = {
	.domain_alloc = hisi_smmu_domain_alloc,
	.domain_free = hisi_smmu_domain_free,
	.map = hisi_smmu_map,
	.unmap = hisi_smmu_unmap,
	.map_sg = hisi_iommu_map_sg,
	.attach_dev = hisi_attach_dev,
	.detach_dev = hisi_detach_dev,
	.iova_to_phys = hisi_smmu_iova_to_phys,
	.pgsize_bitmap = SMMU_PAGE_SIZE,
	.map_tile = hisi_smmu_map_tile,
	.unmap_tile = hisi_smmu_unmap_tile,
};

/*show page table*/
#if defined(CONFIG_DEBUG_FS)
smmu_pte_t hisi_dbg_show_ptb(unsigned int iova, struct smmu_pgd_t *pgdp)
{
	struct smmu_pgd_t pgd;
	smmu_pte_t pte;

	if (!pgdp) {
		dbg("pgd pointer is null\n");
		return 0;
	}

	pgd = *(pgdp + smmu_pgd_index(iova));
	if (smmu_pgd_none(pgd)) {
		dbg("pgd entery is null,offset=%d\n", smmu_pgd_index(iova));
		return 0;
	}

	pte =
	    *((smmu_pte_t *) smmu_pte_page_vaddr(&pgd) + smmu_pte_index(iova));
	return pte;
}

static ssize_t dbg_smmu_read(struct file *file, char __user *buf, size_t count,
			     loff_t *ppos)
{
	char *msg = NULL;
	unsigned int read_size = 0, io_addr = 0, iova_end = 0;

	msg = kmalloc(SMMU_DBG_BUFFER, GFP_KERNEL);
	if (!msg) {
		dbg("message buf is null\n");
		return simple_read_from_buffer(buf, count, ppos,
					       "alloc buffer fail\n",
					       sizeof("alloc buffer fail\n"));
	}
	iova_end = dbg_iova_start + dbg_iova_size;
	dbg("iova_end = 0x%x\n", iova_end);
	/*lint -e685 -esym(685,*)*/
	if ((dbg_iova_start == INVALID_IO_ADDR) || (iova_end > MAX_IO_ADDR)) {
	/*lint -e685 +esym(685,*)*/
		dbg("parameter error :dbg_iova_start=0x%x, \
				dbg_iova_size=0x%x, iova_end=0x%x\n",
				dbg_iova_start, dbg_iova_size, iova_end);
		kfree(msg);
		return simple_read_from_buffer(buf, count, ppos,
					       "parameter error\n",
					       sizeof("parameter error\n"));
	} else {
		dbg_iova_start = SMMU_PAGE_ALIGN(dbg_iova_start);
		iova_end = SMMU_PAGE_ALIGN(iova_end);

		pr_info("[pgbase=0x%lx]\n", hisi_smmu->va_pgtable_addr);
		for (io_addr = dbg_iova_start; io_addr < iova_end;
		     io_addr += SMMU_PAGE_SIZE) {
			pr_info("pgd[%d],pte[%d]=0x%x\n",
				smmu_pgd_index(io_addr),
				smmu_pte_index(io_addr),
				hisi_dbg_show_ptb(io_addr, (struct smmu_pgd_t *)
						  hisi_smmu->va_pgtable_addr));
		}

	}
	read_size = simple_read_from_buffer(buf, count, ppos, msg, strlen(msg));
	dbg_iova_start = INVALID_IO_ADDR;
	dbg_iova_size = INVALID_IO_SIZE;
	kfree(msg);
	return read_size;
}

static ssize_t dbg_smmu_write(struct file *file, const char __user *buffer,
			      size_t count, loff_t *ppos)
{
	char *msg = NULL;
	unsigned int num;

	if (count >= SMMU_DBG_BUFFER)
		return -EINVAL;

	msg = kmalloc(SMMU_DBG_BUFFER, GFP_KERNEL);
	if (!msg) {
		dbg("alloc buffer error\n");
		return -ENOMEM;
	}
	if (!simple_write_to_buffer(msg,
				SMMU_DBG_BUFFER, ppos, buffer, count)) {
		dbg("read data error\n");
		kfree(msg);
		return -EFAULT;
	}
	/* cppcheck-suppress * */
	num = sscanf(msg, "start=0x%8x size=0x%8x", /* unsafe_function_ignore: sscanf  */
		     &dbg_iova_start, &dbg_iova_size);
	dbg("num=%d,dbg_iova_start=0x%x,dbg_iova_size=0x%x\n", num,
	    dbg_iova_start, dbg_iova_size);

	kfree(msg);
	return count;
}

const struct file_operations smmu_pgtable_fops = {
	.read = dbg_smmu_read,
	.write = dbg_smmu_write,
};
#endif

static int hisi_smmu_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	unsigned int ret;
	struct device_node *np = pdev->dev.of_node;
#if defined(CONFIG_HISI_DEBUG_FS)
	struct dentry *file;
	struct dentry *debugfs_root;
#endif

	hisi_smmu = devm_kzalloc(dev, sizeof(*hisi_smmu), GFP_KERNEL);
	if (!hisi_smmu)
		goto smmu_device_error;

	hisi_smmu->dev = dev;
	INIT_LIST_HEAD(&hisi_smmu->domain_list);
	spin_lock_init(&hisi_smmu->lock);

	ret =
	    of_property_read_u64(np, "phy_pgd_base",
				 &hisi_smmu->smmu_phy_pgtable_addr);
	if (ret) {
		dbg("read page table base error\n");
		goto error;
	} else {
		dbg("smmu_phy_pgtable_addr=0x%llx\n",
		    hisi_smmu->smmu_phy_pgtable_addr);
		hisi_smmu->va_pgtable_addr = (unsigned long)
		    phys_to_virt(hisi_smmu->smmu_phy_pgtable_addr);
		dbg("smmu page table : 0x%lx\n", hisi_smmu->va_pgtable_addr);
		memset((unsigned long *)hisi_smmu->va_pgtable_addr, 0, SZ_16K); /* unsafe_function_ignore: memset  */
	}

	bus_set_iommu(&platform_bus_type, &hisi_smmu_ops);
#if defined(CONFIG_HISI_DEBUG_FS)
	debugfs_root = debugfs_create_dir("hisi_smmu", NULL);
	if (IS_ERR(debugfs_root) || !debugfs_root)
		goto error;
	file = debugfs_create_file("smmu_pgtbl",
				   (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
				   debugfs_root, &pdev->dev,
				   &smmu_pgtable_fops);
	if (!file) {
		debugfs_remove(debugfs_root);
		goto error;
	}
#endif
	dbg_iova_start = INVALID_IO_ADDR;
	dbg_iova_size = INVALID_IO_SIZE;

	return 0;

error:
	kfree(hisi_smmu);
smmu_device_error:
	return -ENOMEM;
}

static int hisi_smmu_remove(struct platform_device *pdev)
{
	if (list_empty(&hisi_smmu->domain_list)) {
		hisi_smmu_free_pgtables((unsigned long *)
					hisi_smmu->va_pgtable_addr);
		kfree(hisi_smmu);
	}
	return 0;
}

static const struct of_device_id hisi_smmu_of_match[] = {
	{.compatible = "hisi,hisi-smmu"},
	{},
};

MODULE_DEVICE_TABLE(of, hisi_smmu_of_match);

static struct platform_driver hisi_smmu_driver = {
	.driver = {
		   .owner = THIS_MODULE,
		   .name = "hisi-smmu",
		   .of_match_table = of_match_ptr(hisi_smmu_of_match),
		   },
	.probe = hisi_smmu_probe,
	.remove = hisi_smmu_remove,
};

static int __init hisi_smmu_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&hisi_smmu_driver);
	if (ret)
		pr_err("register smmu driver fail\n");
	return ret;
}

static void __exit hisi_smmu_exit(void)
{
	platform_driver_unregister(&hisi_smmu_driver);
}

subsys_initcall(hisi_smmu_init);
module_exit(hisi_smmu_exit);

MODULE_DESCRIPTION("IOMMU API for austin architected SMMU implementations");
MODULE_AUTHOR("huawei hisilicon company");
MODULE_LICENSE("GPL v2");
