/*
 *  ISP driver, hisp_mempool.c
 *
 * Copyright (c) 2013 ISP Technologies CO., Ltd.
 *
 */

#include <linux/platform_data/remoteproc_hisp.h>
#include <linux/platform_data/hisp_mempool.h>
#include "hisp_internel.h"
#include <securec.h>
#include <linux/hisi-iommu.h>


static struct isp_mem_pool_s g_hisp_mem_pool_info;

static unsigned int dynamic_memory_map(struct scatterlist *sgl,
			size_t addr, size_t size, unsigned int prot)
{
	struct isp_mem_pool_s *dev = &g_hisp_mem_pool_info;
	struct iommu_domain *domain = NULL;
	size_t phy_len = 0;
	struct device *subdev = NULL;

	if (sgl == NULL) {
		pr_err("%s: sgl is NULL!\n", __func__);
		return 0;
	}
	if ((IOMMU_MASK & prot) != 0) {
		pr_err("iommu attr error.0x%x\n", prot);
		return 0;
	}

	subdev = get_isp_device();
	if (subdev == NULL) {
		pr_err("%s: platform dev is NULL!\n", __func__);
		return 0;
	}
	mutex_lock(&dev->hisp_map_mutex);
	domain = iommu_get_domain_for_dev(subdev);
	if (domain == NULL) {
		pr_err("%s: domain is NULL!\n", __func__);
		mutex_unlock(&dev->hisp_map_mutex);
		return 0;
	}
	/* Now iommu_map_sg can't support more prot, only w+r */
	prot = prot & (IOMMU_READ | IOMMU_WRITE);
	pr_info("%s : map addr.0x%lx, size.0x%lx, prot.0x%x\n",
		__func__, addr, size, prot);

	phy_len = iommu_map_sg(domain, addr, sgl, sg_nents(sgl), prot);
	if (phy_len != size) {
		pr_err("%s: iommu_map_sg failed! phy_len.0x%lx, size.0x%lx\n",
				__func__, phy_len, size);
		mutex_unlock(&dev->hisp_map_mutex);
		return 0;
	}
	mutex_unlock(&dev->hisp_map_mutex);
	return addr;
}

static int dynamic_memory_unmap(size_t addr, size_t size)
{
	struct isp_mem_pool_s *dev = &g_hisp_mem_pool_info;
	struct iommu_domain *domain = NULL;
	size_t phy_len = 0;
	struct device *subdev = NULL;

	subdev = get_isp_device();
	if (subdev == NULL) {
		pr_err("%s: platform dev is NULL!\n", __func__);
		return 0;
	}

	mutex_lock(&dev->hisp_map_mutex);
	domain = iommu_get_domain_for_dev(subdev);
	if (domain == NULL) {
		pr_err("%s: domain is NULL!\n", __func__);
		mutex_unlock(&dev->hisp_map_mutex);
		return 0;
	}

	pr_info("%s : unmap addr.0x%lx, size.0x%lx\n", __func__, addr, size);
	if (hisp_get_smmuc3_flag() == 1)
		phy_len = hisi_iommu_unmap_fast(subdev, addr, size);
	else
		phy_len = iommu_unmap(domain, addr, size);

	if (phy_len != size) {
		pr_err("%s: iommu_unmap failed: phy_len 0x%lx size 0x%lx\n",
				__func__, phy_len, size);
		mutex_unlock(&dev->hisp_map_mutex);
		return -EINVAL;
	}
	mutex_unlock(&dev->hisp_map_mutex);
	return 0;
}

static unsigned int get_size_align_mask(unsigned int align)
{
	unsigned int mask = ISP_MAX_NUM_MAGIC;
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
	struct isp_mem_pool_s *dev = &g_hisp_mem_pool_info;
	unsigned long iova = 0;
	unsigned long tmp;

	if (pool == NULL)
		return 0;

	mutex_lock(&dev->mem_pool_mutex);

	iova = gen_pool_alloc(pool, (unsigned long)size);
	if (iova == 0) {
		pr_err("gen_pool_alloc failed!\n");
		mutex_unlock(&dev->mem_pool_mutex);
		return 0;
	}

	tmp = 1 << (unsigned long)(pool->min_alloc_order);/*lint !e571 !e647 */
	if (align > tmp)
		pr_info("can't align to 0x%lx\n", align);
	mutex_unlock(&dev->mem_pool_mutex);
	return iova;
}

static void mem_pool_free_iova(struct gen_pool *pool,
		unsigned long iova, size_t size)
{
	struct isp_mem_pool_s *dev = &g_hisp_mem_pool_info;

	mutex_lock(&dev->mem_pool_mutex);
	gen_pool_free(pool, iova, size);
	mutex_unlock(&dev->mem_pool_mutex);
}

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
	pr_info("%s: use_size.0x%x, size.0x%x\n",
		__func__, use_size, pool_info->size);
}

static unsigned int check_mem_pool_enable(unsigned int pool_num)
{
	struct isp_mem_pool_s *dev = &g_hisp_mem_pool_info;
	struct mem_pool_info_s *pool_info = &dev->isp_pool_info[pool_num];

	if (pool_info != NULL)
		return pool_info->enable;
	else
		return 1;
}

static unsigned int hisp_find_first_bit(unsigned int num, unsigned int align)
{
	unsigned int index = 0;

	for (index = ISP_MAX_BYTE_BIT_NUM; (1 << index) >= align; index--) {
		if (num & (1 << index))
			return index;
		}
	return 0;
}

static unsigned int get_align_addr_from_iova(unsigned int iova,
		unsigned int size,
		unsigned int map_size,
		unsigned int align)
{
	unsigned int index = 0;
	unsigned int addr = 0;
	unsigned int end_addr = 0;

	if ((ISP_MAX_NUM_MAGIC - iova) < map_size) {
		pr_err("Invalid iova.0x%x, map_size.0x%x\n", iova, map_size);
		return 0;
	}
	end_addr = iova + map_size;

	index = hisp_find_first_bit(size, align);
	if (index == 0) {
		pr_err("Invalid argument addr.0x%x, size.0x%x, align.0x%x\n",
			addr, size, align);
		return 0;
	}

	addr = iova & (~(unsigned int)((unsigned int)(1 << index) - 1));
	if (iova > addr)
		addr += (1 << index);
	if ((ISP_MAX_NUM_MAGIC - addr) <= size) {
		pr_err("Invalid iova.0x%x, size.0x%x\n", addr, size);
		return 0;
	}
	if (addr > end_addr || ((addr + size) > end_addr)) {
		pr_err("Invalid iova.0x%x, size.0x%x\n", addr, size);
		return 0;
	}

	return addr;
}

static int free_addr_to_iova_mempool(unsigned int addr, unsigned int size)
{
	struct isp_mem_pool_s *info = &g_hisp_mem_pool_info;
	struct mem_pool_info_s *dev = NULL;
	struct mem_pool_info_list_s *listnode = NULL;
	struct mem_pool_info_list_s *node = NULL;
	struct mem_pool_info_list_s *node_temp = NULL;

	dev = &info->iova_pool_info;
	if (dev == NULL) {
		pr_err("isp mem pool didn't creat\n");
		return -ENXIO;
		}
	listnode = dev->node;
	if ((dev->enable != 1) || (listnode == NULL)
		|| (dev->isp_mem_pool == NULL)) {
		pr_err("iova mem pool didn't creat\n");
		return -ENXIO;
	}
	if ((addr < dev->iova_start) || (addr >= dev->iova_end) ||
		(size > (dev->iova_end - addr))) {
		pr_err("Invalid argument addr.0x%x, size.0x%x\n", addr, size);
		return -EINVAL;
	}

	list_for_each_entry_safe(node, node_temp, &listnode->list, list) {
		if (node->addr == addr) {
			gen_pool_free(dev->isp_mem_pool,
				node->iova, node->iova_size);
			list_del(&node->list);
			pr_info("[ISP Mem]: va = 0x%x, size = 0x%x\n",
				node->iova, node->iova_size);
			kfree(node);
			return 0;
		}
	}
	return -ENOMEM;
}

static unsigned int get_addr_form_iova_mempool(unsigned int size,
					unsigned int align)
{
	struct isp_mem_pool_s *info = &g_hisp_mem_pool_info;
	struct mem_pool_info_s *dev = NULL;
	struct mem_pool_info_list_s *listnode = NULL;
	struct mem_pool_info_list_s *node = NULL;
	unsigned int map_size = 0;
	unsigned int iova = 0;
	unsigned int addr = 0;

	dev = &info->iova_pool_info;
	if (dev == NULL) {
		pr_err("isp mem pool didn't creat\n");
		return 0;
		}
	listnode = dev->node;

	if ((dev->enable != 1) || (listnode == NULL)
		|| (dev->isp_mem_pool == NULL)) {
		pr_err("iova mem pool didn't creat\n");
		return 0;
	}
	if ((size == 0) || (size > (dev->iova_end - dev->iova_start) / 2)) {
		pr_err("Invalid argument size.0x%x\n", size);
		return 0;
	}
/* double size to find a align addr */
	map_size = PAGE_ALIGN(size * 2);/*lint !e50 */
	iova = gen_pool_alloc(dev->isp_mem_pool, (unsigned long)map_size);
	if (iova == 0) {
		pr_err("gen_pool_alloc failed, map_size.0x%x, size.0x%x\n",
			map_size, size);
		dump_mem_pool_info(dev);
		return 0;
	}

	addr = get_align_addr_from_iova(iova, size, map_size, align);
	if (addr == 0) {
		pr_err("get align addr failed, iova.0x%x, size.0x%x\n",
			iova, size);
		goto get_align_addr_fail;
	}

	node = kzalloc(sizeof(struct mem_pool_info_list_s), GFP_KERNEL);
	if (node == NULL)
		goto get_align_addr_fail;

	node->addr = addr;/*lint !e613 */
	node->size = size;/*lint !e613 */
	node->iova = iova;/*lint !e613 */
	node->iova_size = map_size;/*lint !e613 */
	list_add_tail(&node->list, &listnode->list);/*lint !e613 */

	return addr;

get_align_addr_fail:
	gen_pool_free(dev->isp_mem_pool, iova, map_size);
	return 0;
}

unsigned int hisp_alloc_cpu_map_addr(struct scatterlist *sgl,
				unsigned int prot,
				unsigned int size,
				unsigned int align)
{
	struct isp_mem_pool_s *info = &g_hisp_mem_pool_info;
	struct mem_pool_info_s *dev = NULL;
	unsigned int iova = 0;
	unsigned int da = 0;
	int ret = 0;

	if (sgl == NULL) {
		pr_err("[ISP Mem] Invalid argument size\n");
		return 0;
		}

	dev = &info->iova_pool_info;
	if (dev == NULL) {
		pr_err("[ISP Mem] iova mem pool didn't creat\n");
		return 0;
		}
	if (dev->enable != 1) {
		pr_err("[ISP Mem] iova mem pool not creat.%d\n", dev->enable);
		return 0;
	}

	da = get_addr_form_iova_mempool(size, align);
	if (da == 0) {
		pr_err("[ISP Mem] Fail: get iova, size.0x%x, align.%d\n",
			size, align);
		return 0;
	}
	iova = dynamic_memory_map(sgl, da, (size_t)size, prot);
	if (iova == 0) {
		pr_err("[ISP Mem] Fail:dynamic_memory_map,da.0x%x, size.0x%x\n",
				da, size);
		ret = free_addr_to_iova_mempool(da, size);
		if (ret < 0)
			pr_err("[ISP] Fail: free addr.0x%x, size.0x%x\n",
				da, size);
	}
	pr_info("[ISP Mem] alloc cpu map mem addr.0x%x\n", iova);
	return iova;
}
EXPORT_SYMBOL(hisp_alloc_cpu_map_addr);/*lint !e580 !e546 */

int hisp_free_cpu_map_addr(unsigned int iova,
				unsigned int size)
{
	struct isp_mem_pool_s *info = &g_hisp_mem_pool_info;
	struct mem_pool_info_s *dev = NULL;
	int ret = 0;

	dev = &info->iova_pool_info;
	if (dev == NULL) {
		pr_err("[%s] Fail : iova mem pool didn't creat\n", __func__);
		return -ENXIO;
		}

	if (dev->enable != 1) {
		pr_err("[%s] Fail : iova mem pool didn't creat\n", __func__);
		return -ENXIO;
	}

	pr_info("[ISP Mem] free cpu map mem addr.0x%x,size.0x%x\n", iova, size);

	ret = free_addr_to_iova_mempool(iova, size);
	if (ret < 0) {
		pr_err("[%s]Fail:free iova.0x%x,ret.%d\n", __func__, iova, ret);
		return ret;
	}

	ret = dynamic_memory_unmap(iova, size);
	if (ret < 0)
		pr_err("[%s]Fail: dynamic_memory_unmap, iova.0x%x, size.0x%x\n",
			__func__, iova, size);


	return ret;
}
EXPORT_SYMBOL(hisp_free_cpu_map_addr);/*lint !e580 !e546 */

static void hisp_atf_unmap_mem(struct mem_pool_info_s *pool_info)
{
	struct isp_a7mapping_s *map_info = NULL;

	if (pool_info == NULL) {
		pr_err("%s: pool_info is NULL\n",
				__func__);
		return;
	}

	if (use_sec_isp()) {
		if (!sec_process_use_ca_ta()) {
			map_info = hisp_get_ap_dyna_mapping();

			map_info->a7va = pool_info->addr;
			map_info->size = pool_info->size;

			atfisp_ispcpu_unmap();
		}
	}
}

static void hisp_mempool_list_clean(struct mem_pool_info_list_s *listnode,
			struct mem_pool_info_s *pool_info)
{
	struct mem_pool_info_list_s *node = NULL;
	struct mem_pool_info_list_s *node_temp = NULL;
	struct isp_mem_pool_s *dev = &g_hisp_mem_pool_info;

	if ((pool_info == NULL) || (listnode == NULL)) {
		pr_err("%s: pool_info is %pK, pool_info is %pK\n",
			__func__, pool_info, listnode);
		return;
	}

	mutex_lock(&dev->mem_pool_mutex);
	list_for_each_entry_safe(node, node_temp, &listnode->list, list) {
		mutex_unlock(&dev->mem_pool_mutex);
		mem_pool_free_iova(pool_info->isp_mem_pool,
			(unsigned long)node->addr, (unsigned long)node->size);
		mutex_lock(&dev->mem_pool_mutex);
		list_del(&node->list);
		pr_err("%s: va = 0x%x, size = 0x%x\n",
				__func__, node->addr, node->size);
		kfree(node);
	}
	mutex_unlock(&dev->mem_pool_mutex);
}

static void mem_pool_destroy(unsigned int pool_num)
{
	struct isp_mem_pool_s *dev = &g_hisp_mem_pool_info;
	struct mem_pool_info_s *pool_info = &dev->isp_pool_info[pool_num];
	struct mem_pool_info_list_s *listnode = pool_info->node;

	int ret = 0;

	mutex_lock(&dev->mem_pool_mutex);
	if (pool_info->enable == 0) {
		pr_err("%s: the pool_num %d not creat or had been disable\n",
				__func__, pool_num);
		mutex_unlock(&dev->mem_pool_mutex);
		return;
	}
	hisp_atf_unmap_mem(pool_info);

	if (hisp_get_smmuc3_flag() == 1) {
		if (use_nonsec_isp()) {
			ret = hisp_free_cpu_map_addr(pool_info->addr,
						     pool_info->size);
			if (ret < 0) {
				pr_err("%s Fail: free ispcpu addr\n", __func__);
				mutex_unlock(&dev->mem_pool_mutex);
				return;
			}
		}
	} else {
		if (pool_info->dynamic_mem_flag) {
			ret = dynamic_memory_unmap(pool_info->addr,
						(size_t)pool_info->size);
			if (ret < 0) {
				mutex_unlock(&dev->mem_pool_mutex);
				return;
			}

			pool_info->dynamic_mem_flag = 0;
		}
	}
	mutex_unlock(&dev->mem_pool_mutex);
	hisp_mempool_list_clean(listnode, pool_info);
	mutex_lock(&dev->mem_pool_mutex);
	kfree(listnode);

	if (dev->count > 0)
		dev->count--;
	else
		pr_err("[%s] mempool destroy pool num = %d, pool count.%d\n",
			__func__, pool_num, dev->count);

	gen_pool_destroy(pool_info->isp_mem_pool);
	pool_info->enable = 0;
	pr_info("[ISP Mem] mempool destroy pool num = %d, pool count.%d\n",
			pool_num, dev->count);

	mutex_unlock(&dev->mem_pool_mutex);
}

static int hisp_alloc_cpu_align_addr(struct scatterlist *sgl,
			unsigned int pool_num,
			unsigned int iova,
			unsigned int size,
			unsigned int prot,
			unsigned int align)
{
	struct isp_a7mapping_s *map_info = NULL;
	void *addr = NULL;
	unsigned int pool_addr = 0;
	unsigned int align_mask = 0;
	int ret = 0;

	if (use_sec_isp()) {
		map_info = hisp_get_ap_dyna_mapping();
		switch (pool_num) {
		case 0:
			pool_addr = 0xC8000000;
			break;
		case 1:
			pool_addr = 0xD0000000;
			break;
		case 2:
			pool_addr = 0xC4000000;
			break;
		default:
			pr_err("%s: wrong num.%d\n", __func__, pool_num);
			return 0;
		}

		addr = hisp_get_dyna_array();
		map_info->a7va = pool_addr;
		map_info->size = size;
		map_info->prot = prot;

		pr_debug("[%s] pool_num.%d, iova.0x%x, size.0x%x, prot.0x%x\n",
			__func__, pool_num, map_info->a7va, size, prot);

		a7_map_set_pa_list(addr, sgl, size);
		ret = atfisp_ispcpu_map();
		if (ret < 0) {
			pr_err("%s:atfisp_ispcpu_map fail.%d\n", __func__, ret);
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
	}
	return pool_addr;
}

static unsigned int hisp_alloc_cpu_addr(struct scatterlist *sgl,
					 unsigned int iova,
					 unsigned int map_size,
					 unsigned int prot,
					 unsigned int pool_num,
					 unsigned int align)
{
	unsigned int pool_addr = 0;

	if (hisp_get_smmuc3_flag() == 0) {
		pool_addr = hisp_alloc_cpu_align_addr(sgl, pool_num,
				iova, map_size, prot, align);
	} else {
		if (use_nonsec_isp()) {
			pool_addr = hisp_alloc_cpu_map_addr(sgl, prot,
					map_size, align);
		} else {
			pool_addr = hisp_pool_mem_addr(pool_num);
		}
	}
	if (pool_addr == 0x0) {
		pr_err("[%s] alloc addr failed!.%d\n",
			__func__, hisp_get_smmuc3_flag());
		return 0;
	}

	return pool_addr;

}

static int hisp_set_pool_info(struct mem_pool_info_s *pool_info,
			struct gen_pool *pool,
			unsigned int pool_addr,
			unsigned int map_size,
			unsigned int prot,
			unsigned int dynamic_mem_flag)
{
	struct mem_pool_info_list_s *listnode = NULL;

	if ((pool_info == NULL) || (pool == NULL)) {
		pr_err("%s: alloc listnode fail\n", __func__);
		return -ENOMEM;
	}

	listnode = kzalloc(sizeof(struct mem_pool_info_list_s), GFP_KERNEL);
	if (listnode == NULL)
		return -ENOMEM;

	INIT_LIST_HEAD(&listnode->list);
	pool_info->node = listnode;
	pool_info->addr = pool_addr;
	pool_info->size = map_size;
	pool_info->prot = prot;
	pool_info->isp_mem_pool = pool;
	pool_info->enable = 1;
	pool_info->dynamic_mem_flag = dynamic_mem_flag;

	return 0;
}

static void *hisp_gen_pool_create(unsigned int pool_addr, unsigned int map_size)
{
	struct gen_pool *pool = NULL;
	int ret = 0;

	pool = gen_pool_create((int)(order_base_2(ISP_MEM_POOL_ALIGN)), -1);
	if (pool == NULL) {
		pr_err("Create isp gen pool failed!\n");
		return NULL;
	}

	ret = gen_pool_add(pool, (unsigned long)pool_addr,
			  (unsigned long)map_size, -1);
	if (ret) {
		pr_err("Gen pool add failed!\n");
		gen_pool_destroy(pool);
		return NULL;
	}

	return  pool;
}

static unsigned int mem_pool_setup(struct scatterlist *sgl,
					 unsigned int iova,
					 unsigned int size,
					 unsigned int prot,
					 unsigned int pool_num,
					 unsigned int align,
					 unsigned int dynamic_mem_flag)
{
	struct isp_mem_pool_s *dev = &g_hisp_mem_pool_info;
	struct mem_pool_info_s *pool_info = NULL;
	struct gen_pool *pool = NULL;
	unsigned int pool_addr = 0;
	unsigned int map_size = 0;
	int ret = 0;

	mutex_lock(&dev->mem_pool_mutex);
	dev->count++;
	pool_info = &dev->isp_pool_info[pool_num];
	if ((dev->count > ISP_MEM_POOL_NUM) || (pool_info->enable == 1)) {
		pr_err("[%s] The %d mem pool had ben enabled, count.%d!\n",
			__func__, pool_num, dev->count);
		goto invalid_argument;
	}

	ret = memset_s(pool_info, sizeof(struct mem_pool_info_s),
		 0, sizeof(struct mem_pool_info_s));/*lint !e838 */
	if (ret < 0)
		pr_err("[%s] mem_pool_info_s to 0 fail.%d\n", __func__, ret);

	map_size = PAGE_ALIGN(size);/*lint !e50 */

	pool_addr = hisp_alloc_cpu_addr(sgl, iova, map_size,
				prot, pool_num, align);
	if (pool_addr == 0x0) {
		pr_err("alloc addr failed!\n");
		goto alloc_cpu_addr_fail;
	}

	pool = hisp_gen_pool_create(pool_addr, map_size);
	if (pool == NULL) {
		pr_err("%s Failed: hisp_gen_pool_create!\n", __func__);
		goto alloc_cpu_addr_fail;
	}

	ret = hisp_set_pool_info(pool_info, pool, pool_addr,
			map_size, prot, dynamic_mem_flag);
	if (ret < 0) {
		pr_err("hisp_set_pool_info failed!\n");
		goto alloc_cpu_addr_fail;
	}
	pr_info("[ISP] mempool setup num.%d, count.%d, addr.0x%x, size.0x%x\n",
		pool_num, dev->count, pool_addr, map_size);

	mutex_unlock(&dev->mem_pool_mutex);
	return pool_addr;

alloc_cpu_addr_fail:
	mem_pool_destroy(pool_num);
invalid_argument:
	mutex_unlock(&dev->mem_pool_mutex);
	return 0;
}

unsigned long hisp_mem_pool_alloc_iova(
		unsigned int size, unsigned int pool_num)
{
	struct isp_mem_pool_s *dev = &g_hisp_mem_pool_info;
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
	pr_debug("[ISP] alloc mem, pool_num.%d, size.0x%x\n", pool_num, size);
	pool_info = &dev->isp_pool_info[pool_num];
	if (pool_info->enable == 0) {
		pr_err("%s: the pool_num %d not creat!\n", __func__, pool_num);
		return 0;
	}

	listnode = pool_info->node;
	map_size = PAGE_ALIGN(size);/*lint !e50 */
	iova = mem_pool_alloc_iova(pool_info->isp_mem_pool, map_size, 0);
	if (iova == 0) {
		pr_err("[%s]Failed:iova.0x%lx,num.%d,size.0x%x,map_size.0x%x\n",
				__func__, iova, pool_num, size, map_size);
		dump_mem_pool_info(pool_info);
		return 0;
	}

	node = kzalloc(sizeof(struct mem_pool_info_list_s), GFP_KERNEL);
	if (node == NULL) {
		mem_pool_free_iova(pool_info->isp_mem_pool,
				iova, (unsigned long)map_size);
		return 0;
	}

	node->addr = (unsigned int)iova;
	node->size = map_size;
	list_add_tail(&node->list, &listnode->list);
	pr_info("[ISP Mem]:pool.num.%d, iova.0x%lx, size.0x%x, map_size.0x%x\n",
			pool_num, iova, size, map_size);
	return iova;
}

int hisp_mem_pool_free_iova(unsigned int pool_num,
					unsigned int va, unsigned int size)
{
	struct isp_mem_pool_s *dev = &g_hisp_mem_pool_info;
	struct mem_pool_info_list_s *node = NULL;
	struct mem_pool_info_list_s *temp = NULL;
	struct mem_pool_info_s *pool_info = NULL;
	struct mem_pool_info_list_s *listnode = NULL;
	unsigned int map_size;

	if (size == 0) {
		pr_err("%s: invalid para, size == 0x%x.\n", __func__, size);
		return -EINVAL;
	}

	if (pool_num >= ISP_MEM_POOL_NUM) {
		pr_err("Mem pool num wrong!\n");
		return -EBADF;
	}

	pool_info = &dev->isp_pool_info[pool_num];
	if (pool_info->enable == 0) {
		pr_err("%s: the pool_num %d not creat!\n", __func__, pool_num);
		return -EPERM;
	}

	pr_info("[ISP Mem] free mem, pool_num.%d, va.0x%x, size.0x%x\n",
		pool_num, va, size);
	listnode = pool_info->node;
	map_size = PAGE_ALIGN(size);/*lint !e50 */
	if ((pool_info->addr <= va)
		&& (va < (pool_info->addr + pool_info->size))) {
		list_for_each_entry_safe(node, temp, &listnode->list, list) {
			if (node->addr == va) {
				pr_debug("[ISP Mem] free mem : va.0x%x, size.0x%x, map_size.0x%x\n",
					node->addr, node->size, map_size);
				mem_pool_free_iova(pool_info->isp_mem_pool,
						  (unsigned long)node->addr,
						  (unsigned long)node->size);
				list_del(&node->list);
				kfree(node);
				return 0;
			}
		}
	}

	return -ENXIO;
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

	if (pool_num >= ISP_MEM_POOL_NUM) {
		pr_err("%s: wrong num, num : 0x%x\n", __func__, pool_num);
		return 0;
	}

	pr_debug("[ISP] flag is %u\n", flag);
	if (check_mem_pool_enable(pool_num)) {
		pr_err("[%s] %d mem pool had ben enabled!\n",
				__func__, pool_num);
		return 0;
	}
	ispcpu_vaddr = iova;
	if ((flag == MAP_TYPE_RAW2YUV) && (hisp_get_smmuc3_flag() == 0)) {
		ispcpu_vaddr = dynamic_memory_map(sgl,
				MEM_RAW2YUV_DA, (size_t)size, prot);
		if (ispcpu_vaddr == 0) {
			pr_err("[%s] Failed : dynamic_memory_map!\n", __func__);
			return 0;
		}
		dynamic_mem_flag = 1;
	}

	va = mem_pool_setup(sgl, ispcpu_vaddr, size, prot,
			pool_num, align, dynamic_mem_flag);
	if (va == 0) {
		pr_err("[%s] Failed : hisp_mem_pool_setup!\n",
				__func__);
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

void hisp_dynamic_mem_pool_clean(void)
{
	struct isp_mem_pool_s *dev = &g_hisp_mem_pool_info;
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
static int mem_pool_info_init(int use_mempool, unsigned int addr,
		unsigned int size)
{
	struct isp_mem_pool_s *info = &g_hisp_mem_pool_info;
	int ret = 0;

	ret = memset_s(info, sizeof(g_hisp_mem_pool_info),
		       0x0, sizeof(g_hisp_mem_pool_info));/*lint !e838 */
	if (ret < 0)
		pr_err("[%s] memset_s g_hisp_mem_pool_info to 0 fail.%d\n",
				__func__, ret);

	if ((use_mempool == 0) || (addr == 0) || (size == 0)) {
		pr_err("[%s] Invalid args use_mempool.%d add.0x%x size.0x%x\n",
				__func__, use_mempool, addr, size);
		return -EINVAL;
	}
	if (ISP_MAX_IOVA_MAGIC - addr <= size) {
		pr_err("[%s] argument out mem! addr.0x%x size.0x%x\n",
				__func__, addr, size);
		return -EINVAL;
	}
	return ret;
}

int hisp_mem_pool_init(unsigned int addr, unsigned int size)
{
	struct isp_mem_pool_s *info = &g_hisp_mem_pool_info;
	struct mem_pool_info_s *dev = NULL;
	struct mem_pool_info_list_s *listnode = NULL;
	unsigned int mempool_size = 0;
	int ret = 0;
	unsigned int use_mempool = hisp_get_smmuc3_flag();

	ret = mem_pool_info_init(use_mempool, addr, size);
	if (ret < 0)
		pr_err("[%s] mem_pool_info_int fail.%d\n",
				__func__, ret);

	dev = &info->iova_pool_info;
	if (dev == NULL) {
		pr_err("isp mem pool didn't creat\n");
		return -ENXIO;
	}

	mutex_init(&info->mem_pool_mutex);
	mutex_init(&info->hisp_map_mutex);

	dev->isp_mem_pool = gen_pool_create(
				(int)(order_base_2(ISP_MEM_POOL_ALIGN)), -1);
	if (dev->isp_mem_pool == NULL) {
		pr_err("Create isp iova mempool failed!\n");
		goto gen_pool_create_fail;
	}

	mempool_size = PAGE_ALIGN(size);/*lint !e50 */
	ret = gen_pool_add(dev->isp_mem_pool, (unsigned long)addr,
			  (unsigned long)mempool_size, -1);
	if (ret) {
		pr_err("iova mempool Gen pool add failed!\n");
		goto gen_pool_add_fail;
	}

	pr_info("[ISP] iova mempool creat : add.0x%x, size.0x%x\n",
			addr, mempool_size);

	listnode = kzalloc(sizeof(struct mem_pool_info_list_s), GFP_KERNEL);
	if (listnode == NULL)
		goto gen_pool_add_fail;

	INIT_LIST_HEAD(&listnode->list);/*lint !e613 */
	dev->node = listnode;
	dev->enable = 1;
	dev->iova_start = addr;
	dev->iova_end = addr + size;
	return 0;

gen_pool_add_fail:
	gen_pool_destroy(dev->isp_mem_pool);
gen_pool_create_fail:
	mutex_destroy(&info->mem_pool_mutex);

	return -ENOMEM;
}

void hisp_mem_pool_exit(void)
{
	struct isp_mem_pool_s *info = &g_hisp_mem_pool_info;
	struct mem_pool_info_s *dev = NULL;
	struct mem_pool_info_list_s *listnode = NULL;
	struct mem_pool_info_list_s *node = NULL;
	struct mem_pool_info_list_s *temp = NULL;
	unsigned int use_mempool = hisp_get_smmuc3_flag();
	int ret = 0;

	if (use_mempool) {
		dev = &info->iova_pool_info;
		if (dev == NULL) {
			pr_err("isp mem pool didn't creat\n");
			return;
		}
		if (dev->enable == 0) {
			pr_err("iova mem pool didn't creat\n");
			return;
		}
		listnode = dev->node;
		if (listnode == NULL) {
			pr_err("isp mem pool didn't creat\n");
			return;
			}

		list_for_each_entry_safe(node, temp, &listnode->list, list) {
			pr_info("%s: va = 0x%x, size = 0x%x\n",
				__func__, node->iova, node->iova_size);
			gen_pool_free(dev->isp_mem_pool,
					node->iova, node->iova_size);
			list_del(&node->list);
			kfree(node);
		}
		gen_pool_destroy(dev->isp_mem_pool);
		kfree(listnode);
		dev->isp_mem_pool = NULL;
		dev->node = NULL;
		dev->enable = 0;
	}
	mutex_destroy(&info->mem_pool_mutex);
	mutex_destroy(&info->hisp_map_mutex);
	ret = memset_s(info, sizeof(g_hisp_mem_pool_info),
		       0x0, sizeof(g_hisp_mem_pool_info));/*lint !e838 */
	if (ret < 0)
		pr_err("[%s] memset_s g_hisp_mem_pool_info to 0 fail.%d\n",
				__func__, ret);
}

