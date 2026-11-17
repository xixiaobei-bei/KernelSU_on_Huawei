/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2013-2019. All rights reserved.
 *
 * Description:
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Create: 2013-12-22
 */

#define pr_fmt(fmt) "[IOMMU: ]" fmt

#include <linux/bitops.h>
#include <linux/debugfs.h>
#include <linux/dma-buf.h>
#include <linux/genalloc.h>
#include <linux/hisi-iommu.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/iommu.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/sizes.h>
#include <linux/slab.h>
#include <linux/types.h>
#include "hisi_smmu.h"
#include "ion.h"

struct mm_dss_size {
	size_t all_size;
	size_t l3_size;
	size_t lb_size;
	size_t map_size;
};

static unsigned long mm_iommu_alloc_iova(struct gen_pool *iova_pool,
				size_t size, unsigned long align)
{
	unsigned long iova;

	if (iova_pool->min_alloc_order >= 0) {
		if (align > (1UL << (unsigned long)iova_pool->min_alloc_order))
			WARN(1, "iommu domain cant align to 0x%lx\n",
			     align);
	} else {
		pr_warn("The min_alloc_order of iova_pool is negative!\n");
		return 0;
	}

	iova = gen_pool_alloc(iova_pool, size);

	return iova;
}

static void mm_iommu_free_iova(struct gen_pool *iova_pool,
				unsigned long iova, size_t size)
{
	if (!iova_pool)
		return;

	gen_pool_free(iova_pool, iova, size);
}

struct gen_pool *iova_pool_setup(unsigned long start, unsigned long size,
				unsigned long align)
{
	struct gen_pool *pool = NULL;
	int ret;

	pool = gen_pool_create(order_base_2(align), -1);/*lint !e666 */
	if (!pool) {
		pr_err("create gen pool failed!\n");
		return NULL;
	}

	/*
	 * iova start should not be 0, because return
	 * 0 when alloc iova is considered as error
	 */
	if (!start)
		WARN(1, "iova start should not be 0!\n");

	ret = gen_pool_add(pool, start, size, -1);
	if (ret) {
		pr_err("gen pool add failed!\n");
		gen_pool_destroy(pool);
		return NULL;
	}

	return pool;
}

void iova_pool_destroy(struct gen_pool *pool)
{
	if (!pool)
		return;

	gen_pool_destroy(pool);
}

static void mm_iova_add(struct rb_root *rb_root,
				struct iova_dom *iova_dom)
{
	struct rb_node **p = &rb_root->rb_node;
	struct rb_node *parent = NULL;
	struct iova_dom *entry = NULL;

	while (*p) {
		parent = *p;
		entry = rb_entry(parent, struct iova_dom, node);

		if (iova_dom < entry) {
			p = &(*p)->rb_left;
		} else if (iova_dom > entry) {
			p = &(*p)->rb_right;
		} else {
			pr_err("%s: iova already in tree\n", __func__);
			BUG();
		}
	}

	rb_link_node(&iova_dom->node, parent, p);
	rb_insert_color(&iova_dom->node, rb_root);
}

static struct iova_dom *mm_iova_dom_get(struct rb_root *rbroot,
				struct dma_buf *dmabuf, unsigned long iova)
{
	struct rb_node *n = NULL;
	struct iova_dom *iova_dom = NULL;
	u64 key = (u64)dmabuf;

	for (n = rb_first(rbroot); n; n = rb_next(n)) {
		iova_dom = rb_entry(n, struct iova_dom, node);
		if (iova_dom->key == key) {
			/*
			 * Sometimes a dmabuf may correspond to more than two iova,
			 * if iova mismatch, this circle need find next matched key
			 */
			if (!iova || iova == iova_dom->iova)
				return iova_dom;
			pr_err("%s iova %llx mismatch\n", __func__, iova);
		}
	}

	return NULL;
}

void mm_iova_dom_info(struct device *dev)
{
	struct rb_node *n = NULL;
	struct iova_dom *iova_dom = NULL;
	unsigned long total_size = 0;
	struct iommu_domain *domain = NULL;
	struct mm_dom_cookie *cookie = NULL;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s, iommu domain is null\n", __func__);
		return;
	}

	cookie = domain->iova_cookie;
	if (!cookie) {
		dev_err(dev, "%s, iommu domain cookie is null!\n", __func__);
		return;
	}

	spin_lock(&cookie->iova_lock);
	for (n = rb_first(&cookie->iova_root); n; n = rb_next(n)) {
		iova_dom = rb_entry(n, struct iova_dom, node);
		total_size += iova_dom->size;
		pr_err("%s: iova:0x%lx size:0x%lx\n",
			dev_name(iova_dom->dev), iova_dom->iova, iova_dom->size);
	}
	spin_unlock(&cookie->iova_lock);

	pr_err("domain %s total size: %lu", dev_name(iova_dom->dev), total_size);
}

#ifdef CONFIG_ARM64_64K_PAGES
#error iommu can not deal with 64k pages!
#endif

static size_t mm_iommu_sg_size_get(struct scatterlist *sgl, int nents)
{
	int i;
	size_t iova_size = 0;
	struct scatterlist *sg = NULL;

	for_each_sg(sgl, sg, nents, i)
		iova_size += (size_t)ALIGN(sg->length, PAGE_SIZE);

	return iova_size;
}

static unsigned long do_iommu_map_sg(struct device *dev,
				struct scatterlist *sgl, int prot,
				unsigned long *out_size)
{
	struct gen_pool *iova_pool = NULL;
	unsigned long iova;
	size_t iova_size;
	size_t map_size;
	struct iommu_domain *domain = NULL;
	struct mm_dom_cookie *cookie = NULL;
	int nents;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		pr_err("Dev has no iommu domain!\n");
		return 0;
	}

	cookie = domain->iova_cookie;
	if (!cookie) {
		dev_err(dev, "%s, iommu domain cookie is null!\n", __func__);
		return 0;
	}

	iova_pool = cookie->iova_pool;
	if (!iova_pool) {
		pr_err("iommu domain iova pool is null\n");
		return 0;
	}

	nents = sg_nents(sgl);
	iova_size = mm_iommu_sg_size_get(sgl, nents);
	iova = mm_iommu_alloc_iova(iova_pool, iova_size,
		cookie->iova.iova_align);
	if (!iova) {
		pr_err("alloc iova failed! size 0x%zx\n", iova_size);
		return 0;
	}

	map_size = iommu_map_sg(domain, iova, sgl, nents, prot);
	if (map_size != iova_size) {
		pr_err("map Fail! iova 0x%lx, iova_size 0x%zx\n",
			iova, iova_size);
		gen_pool_free(iova_pool, iova, iova_size);
		return 0;
	}
	*out_size = (unsigned long)iova_size;
	return iova;
}

static int do_iommu_unmap_sg(struct iommu_domain *domain,
	size_t iova_size, unsigned long iova)
{
	size_t unmap_size;
	int ret;

	struct mm_dom_cookie *cookie = NULL;
	struct gen_pool *iova_pool = NULL;

	cookie = domain->iova_cookie;
	iova_pool = cookie->iova_pool;
	if (!iova_pool) {
		pr_err("%s, iova pool is null!\n", __func__);
		return -EINVAL;
	}

	ret = addr_in_gen_pool(iova_pool, iova, iova_size);
	if (!ret) {
		pr_err("[%s]illegal para!!iova = %lx, size = %lx\n",
			__func__, iova, iova_size);
		return -EINVAL;
	}

	unmap_size = iommu_unmap(domain, iova, iova_size);
	if (unmap_size != iova_size) {
		pr_err("unmap fail! size 0x%lx, unmap_size 0x%zx\n",
			iova_size, unmap_size);
		return -EINVAL;
	}

	mm_iommu_free_iova(iova_pool, iova, iova_size);
	return 0;
}

static void mm_iommu_flush_tlb(struct device *dev,
				struct iommu_domain *domain)
{
	struct iommu_fwspec *fwspec = dev->iommu_fwspec;

	if (!fwspec) {
		dev_err(dev, "%s iommu_fwspec is null\n", __func__);
		return;
	}

	if (!fwspec->ops) {
		dev_err(dev, "%s iommu_fwspec ops is null\n", __func__);
		return;
	}

	if (fwspec->ops->flush_tlb)
		fwspec->ops->flush_tlb(dev, domain);
}

int hisi_iommu_dev_flush_tlb(struct device *dev, unsigned int ssid)
{
	int ret;
	struct iommu_fwspec *fwspec = NULL;
	struct iommu_domain *domain = NULL;

	if (!dev) {
		pr_err("%s input dev err!\n", __func__);
		return -EINVAL;
	}

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s iommu domain is null\n", __func__);
		return -ENODEV;
	}

	fwspec = dev->iommu_fwspec;
	if (!fwspec) {
		dev_err(dev, "%s iommu_fwspec is null\n", __func__);
		return -ENODEV;
	}

	if (!fwspec->ops) {
		dev_err(dev, "%s iommu_fwspec ops is null\n", __func__);
		return -ENODEV;
	}

	if (!fwspec->ops->dev_flush_tlb)
		return 0;

	ret = fwspec->ops->dev_flush_tlb(domain, ssid);
	if (ret)
		dev_err(dev, "%s flush tlb return %d error\n", __func__, ret);

	return ret;
}
EXPORT_SYMBOL(hisi_iommu_dev_flush_tlb);

static int mm_iommu_get_dmabuf_iova(struct device *dev,
				struct dma_buf *dmabuf, unsigned long *iova,
				unsigned long *out_size)
{
	struct iova_dom *iova_dom = NULL;
	struct iommu_domain *domain = NULL;
	struct mm_dom_cookie *cookie = NULL;

	*iova = 0;
	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s, iommu domain is null\n", __func__);
		return -EINVAL;
	}

	cookie = domain->iova_cookie;
	if (!cookie) {
		dev_err(dev, "%s, iova cookie is null!\n", __func__);
		return -EINVAL;
	}

	spin_lock(&cookie->iova_lock);
	iova_dom = mm_iova_dom_get(&cookie->iova_root, dmabuf, 0);
	if (!iova_dom) {
		spin_unlock(&cookie->iova_lock);
		return 0;
	}
	atomic64_inc(&iova_dom->ref);
	spin_unlock(&cookie->iova_lock);
	*out_size = iova_dom->size;
	*iova = iova_dom->iova;

	return 0;
}

static void mm_iommu_iova_domain_register(struct device *dev,
				struct iova_dom *iova_dom)
{
	struct iommu_domain *domain = NULL;
	struct mm_dom_cookie *cookie = NULL;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s, iommu domain is null\n", __func__);
		return;
	}

	cookie = domain->iova_cookie;
	if (!cookie) {
		dev_err(dev, "%s, iommu domain cookie is null!\n", __func__);
		return;
	}

	spin_lock(&cookie->iova_lock);
	mm_iova_add(&cookie->iova_root, iova_dom);
	spin_unlock(&cookie->iova_lock);
}

static unsigned long __iommu_map_dmabuf(struct device *dev,
					struct dma_buf *dmabuf, int prot,
					struct iova_dom *iova_dom,
					struct sg_table *table)
{
	unsigned long iova;

#ifdef CONFIG_HISI_LB
	if (dmabuf->ops->mk_prot)
		prot = ((unsigned int)prot | dmabuf->ops->mk_prot(dmabuf));
#endif

	prot = (int)((unsigned int)prot | (IOMMU_READ | IOMMU_WRITE));
	iova = do_iommu_map_sg(dev, table->sgl, prot, &iova_dom->size);
	if (!iova)
		pr_err("%s, do_iommu_map_sg\n", __func__);

	return iova;
}

static struct iova_dom *__init_iova_dom(struct dma_buf *dmabuf)
{
	struct iova_dom *iova_dom = kzalloc(sizeof(*iova_dom), GFP_KERNEL);
	if (!iova_dom)
		return ERR_PTR(-ENOMEM);

	atomic64_set(&iova_dom->ref, 1);
	iova_dom->key = (u64)(uintptr_t)dmabuf;
	return iova_dom;
}

static struct sg_table *
__dmabuf_get_sgt(struct device *dev, struct dma_buf *dmabuf,
			struct dma_buf_attachment **attach)
{
	struct sg_table *sgt = NULL;

	*attach = dma_buf_attach(dmabuf, dev);
	if (IS_ERR(*attach)) {
		pr_err("%s failed to attach the dmabuf\n", __func__);
		return NULL;
	}

	sgt = dma_buf_map_attachment(*attach, DMA_TO_DEVICE);
	if (IS_ERR(sgt)) {
		pr_err("%s failed to map dma buf to get sgt\n", __func__);
		dma_buf_detach(dmabuf, *attach);
		return NULL;
	}

	return sgt;
}

static void __release_dmabuf_attach(struct dma_buf *dmabuf,
				    struct dma_buf_attachment *attach,
				    struct sg_table *sgt)
{
	if (attach) {
		dma_buf_unmap_attachment(attach, sgt, DMA_FROM_DEVICE);
		dma_buf_detach(dmabuf, attach);
	}
}

static unsigned long __mm_iommu_map_dmabuf(struct device *dev,
					     struct dma_buf *dmabuf, int prot,
					     unsigned long *out_size,
					     struct sg_table *sgt)
{
	struct iova_dom *iova_dom = NULL;
	struct dma_buf_attachment *attach = NULL;
	unsigned long iova = 0;

	if (mm_iommu_get_dmabuf_iova(dev, dmabuf, &iova, out_size)) {
		dev_err(dev, "get dmabuf iova error\n");
		return 0;
	}

	if (iova)
		return iova;

	if (!sgt) {
		sgt = __dmabuf_get_sgt(dev, dmabuf, &attach);
		if (!sgt) {
			dev_err(dev, "__dmabuf_get_sgt fail!\n");
			return 0;
		}
	}

	iova_dom = __init_iova_dom(dmabuf);
	if (IS_ERR(iova_dom)) {
		dev_err(dev, "__init_iova_dom fail!\n");
		goto release_attach;
	}

	iova = __iommu_map_dmabuf(dev, dmabuf, prot, iova_dom, sgt);
	if (!iova) {
		dev_err(dev, "__iommu_map_dmabuf fail!\n");
		goto free_dom;
	}

	iova_dom->iova = iova;
	*out_size = iova_dom->size;
	iova_dom->dev = dev;
	mm_iommu_iova_domain_register(dev, iova_dom);
	__release_dmabuf_attach(dmabuf, attach, sgt);

	return iova;

free_dom:
	kfree(iova_dom);
	mm_iova_dom_info(dev);
release_attach:
	__release_dmabuf_attach(dmabuf, attach, sgt);

	return 0;
}

/**
 * hisi_iommu_map_dmabuf() - Map ION buffer's dmabuf to iova
 * @dev: master's device struct
 * @dmabuf: ION buffer's dmabuf, must be allocated by ION
 * @prot: iommu map prot (eg: IOMMU_READ/IOMMU_WRITE/IOMMU_CACHE etc..)
 * @out_size: return iova size to master's driver if map success
 *
 * When map success return iova, otherwise return 0.
 * This function is called master dev's driver. The master's device tree
 * must quote master's smmu device tree.
 * This func will work with iova refs
 */
unsigned long hisi_iommu_map_dmabuf(struct device *dev, struct dma_buf *dmabuf,
				int prot, unsigned long *out_size)
{
#ifdef CONFIG_HISI_IOMMU_BYPASS
	struct sg_table *sgt = NULL;
	struct dma_buf_attachment *attach = NULL;
	unsigned long bypass_ret;
#endif

	if (!dev || !dmabuf || !out_size) {
		dev_err(dev, "input err! dev %pK, dmabuf %pK\n", dev, dmabuf);
		return 0;
	}

#ifdef CONFIG_HISI_IOMMU_BYPASS
	sgt = __dmabuf_get_sgt(dev, dmabuf, &attach);
	if (!sgt)
		return 0;
	bypass_ret = (unsigned long)sg_phys(sgt->sgl);
	*out_size = sgt->sgl->length;
	__release_dmabuf_attach(dmabuf, attach, sgt);
	return bypass_ret;
#endif

	return __mm_iommu_map_dmabuf(dev, dmabuf, prot, out_size, NULL);
}
EXPORT_SYMBOL(hisi_iommu_map_dmabuf);

/**
 * hisi_iommu_unmap_dmabuf() - Unmap ION buffer's dmabuf and iova
 * @dev: master's device struct
 * @dmabuf: ION buffer's dmabuf, must be allocated by ION
 * @iova: iova which get by hisi_iommu_map_dmabuf()
 *
 * When unmap success return 0, otherwise return ERRNO.
 * This function is called master dev's driver. The master's device tree
 * must quote master's smmu device tree.
 * This func will work with iova refs
 */
int hisi_iommu_unmap_dmabuf(struct device *dev, struct dma_buf *dmabuf,
				unsigned long iova)
{
	int ret;
	struct iova_dom *iova_dom = NULL;
	struct iommu_domain *domain = NULL;
	struct mm_dom_cookie *cookie = NULL;

#ifdef CONFIG_HISI_IOMMU_BYPASS
	return 0;
#endif

	if (!dev || !dmabuf || !iova) {
		pr_err("input err! dev %pK, dmabuf %pK, iova 0x%lx\n",
			dev, dmabuf, iova);
		return -EINVAL;
	}

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s, iommu domain is null\n", __func__);
		return -EINVAL;
	}

	cookie = domain->iova_cookie;
	if (!cookie) {
		dev_err(dev, "%s, iova cookie is null!\n", __func__);
		return -EINVAL;
	}

	spin_lock(&cookie->iova_lock);
	iova_dom = mm_iova_dom_get(&cookie->iova_root, dmabuf, iova);
	if (!iova_dom) {
		spin_unlock(&cookie->iova_lock);
		dev_err(dev, "%s, unmap buf no map data!\n", __func__);
		return -EINVAL;
	}

	if (!atomic64_dec_and_test(&iova_dom->ref)) {
		spin_unlock(&cookie->iova_lock);
		return 0;
	}

	if (iova_dom->iova != iova) {
		spin_unlock(&cookie->iova_lock);
		dev_err(dev, "iova no eq:0x%lx,0x%lx\n", iova, iova_dom->iova);
		return -EINVAL;
	}

	rb_erase(&iova_dom->node, &cookie->iova_root);
	spin_unlock(&cookie->iova_lock);

	ret = do_iommu_unmap_sg(domain, iova_dom->size, iova);

	kfree(iova_dom);
	mm_iommu_flush_tlb(dev, domain);
	return ret;
}
EXPORT_SYMBOL(hisi_iommu_unmap_dmabuf);

unsigned long hisi_iommu_map_sg(struct device *dev, struct scatterlist *sgl,
				int prot, unsigned long *out_size)
{
	unsigned long iova;

	if (!dev || !sgl || !out_size) {
		pr_err("dev %pK, sgl %pK,or outsize is null\n", dev, sgl);
		return 0;
	}

#ifdef CONFIG_HISI_IOMMU_BYPASS
	*out_size = sgl->length;
	return sg_phys(sgl);
#endif

	prot |= IOMMU_READ | IOMMU_WRITE;
	iova = do_iommu_map_sg(dev, sgl, prot, out_size);

	return iova;
}
EXPORT_SYMBOL(hisi_iommu_map_sg);
static int mm_iommu_unmap_sg_check(struct device *dev,
				size_t iova_size, unsigned long iova)
{
	struct rb_node *n = NULL;
	struct iova_dom *iova_dom = NULL;
	struct iommu_domain *domain = NULL;
	struct mm_dom_cookie *cookie = NULL;
	bool is_overlap = false;
	int ret = 0;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s, iommu domain is null\n", __func__);
		return -EINVAL;
	}

	cookie = domain->iova_cookie;
	if (!cookie) {
		dev_err(dev, "%s, iommu domain cookie is null!\n", __func__);
		return -EINVAL;
	}

	spin_lock(&cookie->iova_lock);
	for (n = rb_first(&cookie->iova_root); n; n = rb_next(n)) {
		iova_dom = rb_entry(n, struct iova_dom, node);
		is_overlap = max(iova_dom->iova, iova) <
			min(iova_dom->iova + iova_dom->size, iova + iova_size);
		/*lint -e548 */
		if (is_overlap) {
			dev_err(dev, "%s, iova_dom iova: 0x%lx, size: 0x%lx; iova: 0x%lx, size: 0x%lx\n",
				__func__, iova_dom->iova, iova_dom->size,
				iova, iova_size);
			ret = -EINVAL;
			WARN_ON(1);
			break;
		}
		/*lint +e548 */
	}
	spin_unlock(&cookie->iova_lock);

	return ret;
}

int hisi_iommu_unmap_sg(struct device *dev, struct scatterlist *sgl,
				unsigned long iova)
{
	int ret;
	int nents;
	size_t iova_size;
	struct iommu_domain *domain = NULL;
	struct mm_dom_cookie *cookie = NULL;

	if (!dev || !sgl || !iova) {
		pr_err("input err! dev %pK, sgl %pK, iova 0x%lx\n",
			dev, sgl, iova);
		return -EINVAL;
	}

#ifdef CONFIG_HISI_IOMMU_BYPASS
	return 0;
#endif

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s, iommu domain is null\n", __func__);
		return -EINVAL;
	}

	cookie = domain->iova_cookie;
	if (!cookie) {
		dev_err(dev, "%s, iova cookie is null!\n", __func__);
		return -EINVAL;
	}

	nents = sg_nents(sgl);
	iova_size = mm_iommu_sg_size_get(sgl, nents);

	ret = mm_iommu_unmap_sg_check(dev, iova_size, iova);
	if (ret)
		dev_err(dev, "%s,iova on the rb_tree!\n", __func__);

	ret = do_iommu_unmap_sg(domain, iova_size, iova);

	mm_iommu_flush_tlb(dev, domain);
	return ret;
}
EXPORT_SYMBOL(hisi_iommu_unmap_sg);

static unsigned long mm_iommu_realloc_iova(struct device *dev,
				struct iommu_domain *domain, size_t iova_size)
{
	unsigned long iova;
	unsigned int pages_num, i, size;
	struct mm_dom_cookie *cookie = domain->iova_cookie;
	u32 *free_size = NULL;

	might_sleep();
	pages_num = DIV_ROUND_UP(cookie->iova.iova_size, PAGE_SIZE);
	free_size = cookie->lazy_free->free_size;
	mutex_lock(&cookie->lazy_free->mutex);
	for (i = 0; i < pages_num; i++) {
		size = free_size[i] & IOVA_SIZE_MASK;
		if (size == iova_size)
			break;
	}

	if (i >= pages_num) {
		mutex_unlock(&cookie->lazy_free->mutex);
		dev_info(dev, "%s can't found free iova, pages %lu, size 0x%zx\n",
			__func__, cookie->lazy_free->pages, iova_size);
		cookie->lazy_free->pages = cookie->lazy_free->waterline;
		goto try_alloc;
	}
	free_size[i] = 0;
	mutex_unlock(&cookie->lazy_free->mutex);

	mm_iommu_flush_tlb(dev, domain);

	iova = cookie->iova.iova_start + ((unsigned long)i << PAGE_SHIFT);

	cookie->lazy_free->pages = cookie->lazy_free->waterline;

	return iova;

try_alloc:
	iova = mm_iommu_alloc_iova(cookie->iova_pool, iova_size,
		cookie->iova.iova_align);

	return iova;
}

unsigned long hisi_iommu_map(struct device *dev, phys_addr_t paddr,
				size_t size, int prot)
{
	struct iommu_domain *domain = NULL;
	unsigned long iova;
	size_t iova_size;
	int ret;
	struct mm_dom_cookie *cookie = NULL;

	if (!dev || !paddr) {
		pr_err("input Err! Dev %pK, addr =0x%llx\n", dev, paddr);
		return 0;
	}

	if (!size) {
		pr_err("size is 0! no need to map\n");
		return 0;
	}

#ifdef CONFIG_HISI_IOMMU_BYPASS
	return (unsigned long)paddr;
#endif

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		pr_err("Dev(%s) has no iommu domain!\n", dev_name(dev));
		return 0;
	}

	cookie = domain->iova_cookie;
	if (!cookie) {
		dev_err(dev, "%s, iova_cookie is null!\n", __func__);
		return 0;
	}

	if (!cookie->iova_pool) {
		dev_err(dev, "%s, iova_pool is null!\n", __func__);
		return 0;
	}

	iova_size = ALIGN(size, PAGE_SIZE);
	iova = mm_iommu_alloc_iova(cookie->iova_pool, iova_size,
		cookie->iova.iova_align);
	if (!iova && (cookie->iova.iova_free == LAZY_FREE))
		iova = mm_iommu_realloc_iova(dev, domain, iova_size);

	if (!iova) {
		dev_err(dev, "%s alloc iova failed! size 0x%zx\n",
			__func__, size);
		return 0;
	}
	prot |= IOMMU_READ | IOMMU_WRITE;
	ret = iommu_map(domain, iova, paddr, size, prot);
	if (ret) {
		pr_err("map fail! address 0x%llx,  size 0x%zx\n",
			paddr, size);
		mm_iommu_free_iova(cookie->iova_pool, iova, size);
	}
	return iova;
}
EXPORT_SYMBOL(hisi_iommu_map);

static void mm_iommu_lazy_free_iova(struct mm_dom_cookie *cookie,
				unsigned long iova, size_t size)
{
	u32 ping;
	u64 iova_end;
	unsigned int bgn_page;
	unsigned long flag;
	unsigned int page_num = size >> PAGE_SHIFT;
	struct mm_iova_lazy_free *lazy_free = NULL;

	iova_end = cookie->iova.iova_start + cookie->iova.iova_size;
	if ((iova < cookie->iova.iova_start) || ((iova + size) > iova_end)) {
		pr_err("%s, iova 0x%lx err, iova start 0x%lx, size 0x%lx\n",
			__func__, iova, cookie->iova.iova_start,
			cookie->iova.iova_size);
		return;
	}

	if (size > IOVA_SIZE_MASK) {
		pr_err("%s, iova 0x%lx iova size 0x%lx err\n",
			__func__, iova, size);
		return;
	}

	lazy_free = cookie->lazy_free;
	if (!lazy_free) {
		pr_err("%s, lazy free is null\n", __func__);
		return;
	}

	spin_lock_irqsave(&lazy_free->lock, flag);
	ping = lazy_free->pingpong;
	bgn_page = (iova - cookie->iova.iova_start) >> PAGE_SHIFT;
	if (lazy_free->free_size[bgn_page] != 0) {
		pr_err("%s, iova 0x%lx free err, size 0x%lx\n",
			__func__, iova, size);
		return;
	}
	lazy_free->free_size[bgn_page] = (u32)size | (ping << PINGPONG_SHIFT);
	lazy_free->pages += page_num;

	if (lazy_free->pages >= lazy_free->waterline) {
		if (lazy_free->end) {
			lazy_free->pages = 0;
			lazy_free->pingpong = 1 - ping;
			lazy_free->end = false;
			wake_up(&lazy_free->wait_q);
		}
	}
	spin_unlock_irqrestore(&lazy_free->lock, flag);
}

int hisi_iommu_unmap(struct device *dev, unsigned long iova, size_t size)
{
	struct iommu_domain *domain = NULL;
	size_t iova_size;
	size_t unmap_size;
	struct mm_dom_cookie *cookie = NULL;

	if (!dev || !iova) {
		pr_err("input err! dev %pK, iova 0x%lx\n",
			dev, iova);
		return -EINVAL;
	}

	if (!size) {
		pr_err("input err! dev %pK, size is 0\n", dev);
		return -EINVAL;
	}

#ifdef CONFIG_HISI_IOMMU_BYPASS
	return 0;
#endif

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		pr_err("Dev(%s) has no iommu domain!\n", dev_name(dev));
		return 0;
	}

	cookie = domain->iova_cookie;
	if (!cookie) {
		dev_err(dev, "%s, iova_cookie is null!\n", __func__);
		return 0;
	}

	if (!cookie->iova_pool) {
		dev_err(dev, "%s, iova_pool is null!\n", __func__);
		return 0;
	}

	iova_size = ALIGN(size, PAGE_SIZE);
	unmap_size = iommu_unmap(domain, iova, iova_size);
	if (unmap_size != size) {
		pr_err("unmap fail! size 0x%lx, unmap_size 0x%zx\n",
			size, unmap_size);
		return -EINVAL;
	}

	if (cookie->iova.iova_free == LAZY_FREE) {
		mm_iommu_lazy_free_iova(cookie, iova, iova_size);
		return 0;
	}
	mm_iommu_flush_tlb(dev, domain);
	mm_iommu_free_iova(cookie->iova_pool, iova, size);

	return 0;
}
EXPORT_SYMBOL(hisi_iommu_unmap);

size_t hisi_iommu_unmap_fast(struct device *dev,
			     unsigned long iova, size_t size)
{
	struct iommu_domain *domain = NULL;
	size_t unmapped = 0;

	if (!dev || !size) {
		pr_err("%s: err, size 0x%lx, iova 0x%lx\n",
			__func__, size, iova);
		return -EINVAL;/*lint !e570 */
	}

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s, iommu domain is null\n", __func__);
		return -EINVAL;/*lint !e570 */
	}

	unmapped = iommu_unmap(domain, iova, size);
	if (unmapped != size) {
		pr_err("%s: fail size 0x%lx, unmapped 0x%lx\n",
			__func__, size, unmapped);
		return -EINVAL;/*lint !e570 */
	}
	mm_iommu_flush_tlb(dev, domain);
	return unmapped;
}

#ifdef CONFIG_HISI_IOMMU_TEST
static smmu_pte_t mm_ptb_lpae(unsigned int iova, smmu_pgd_t *pgdp)
{
	smmu_pgd_t pgd;
	smmu_pmd_t pmd;
	smmu_pte_t pte;

	if (!pgdp)
		return 0;

	pgd = *(pgdp + smmu_pgd_index(iova));
	if (smmu_pgd_none_lpae(pgd))
		return 0;

	pmd = *((smmu_pmd_t *)smmu_pmd_page_vaddr_lpae(&pgd) +
		smmu_pmd_index(iova));
	if (smmu_pmd_none_lpae(pmd))
		return 0;

	pte = *((smmu_pte_t *)smmu_pte_page_vaddr_lpae(&pmd) +
		smmu_pte_index(iova));
	return pte;
}

static smmu_pgd_t mm_pgd_lpae(unsigned int iova, smmu_pgd_t *pgdp)
{
	return *(pgdp + smmu_pgd_index(iova));
}

static smmu_pmd_t mm_pmd_lpae(unsigned int iova, smmu_pgd_t *pgdp)
{
	smmu_pgd_t pgd;
	smmu_pmd_t pmd;

	pgd = *(pgdp + smmu_pgd_index(iova));
	if (smmu_pgd_none_lpae(pgd))
		return 0;

	pmd = *((smmu_pmd_t *)smmu_pmd_page_vaddr_lpae(&pgd) +
		smmu_pmd_index(iova));
	return pmd;
}

static void show_smmu_pte(struct mm_domain *mm_domain, unsigned long iova,
				unsigned long size)
{
	unsigned long io_addr;
	smmu_pgd_t *pgdp = (smmu_pgd_t *)mm_domain->va_pgtable_addr;

	for (io_addr = iova; io_addr < iova + size; io_addr += PAGE_SIZE)
		pr_err("iova[0x%lx]:pgd[%ld]=0x%llx,pmd[%ld]=0x%llx, pte[%ld]=0x%llx\n",
		       io_addr, smmu_pgd_index(io_addr),
		       mm_pgd_lpae(io_addr, pgdp), smmu_pmd_index(io_addr),
		       mm_pmd_lpae(io_addr, pgdp), smmu_pte_index(io_addr),
		       mm_ptb_lpae(io_addr, pgdp));
}

void mm_smmu_show_pte(struct device *dev, unsigned long iova,
				unsigned long size)
{
	struct mm_domain *mm_domain = NULL;
	struct iommu_domain *domain = NULL;

	if (!dev || !iova || !size) {
		pr_err("invalid params!\n");
		return;
	}

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		pr_err("dev(%s) has no iommu domain!\n", dev_name(dev));
		return;
	}

	mm_domain = to_mm_domain(domain);

	show_smmu_pte(mm_domain, iova, size);
}

void mm_print_iova_dom(struct device *dev)
{
	struct rb_node *n = NULL;
	struct iova_dom *iova_dom = NULL;
	unsigned long iova;
	unsigned long size;
	struct mm_domain *mm_domain = NULL;
	struct iommu_domain *domain = NULL;
	struct mm_dom_cookie *cookie = NULL;

	if (!dev) {
		pr_err("invalid params!\n");
		return;
	}

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		pr_err("dev(%s) has no iommu domain!\n", dev_name(dev));
		return;
	}

	mm_domain = to_mm_domain(domain);
	cookie = domain->iova_cookie;
	if (!cookie) {
		dev_err(dev, "%s, iommu domain cookie is null!\n", __func__);
		return;
	}

	spin_lock(&cookie->iova_lock);
	for (n = rb_first(&cookie->iova_root); n; n = rb_next(n)) {
		iova_dom = rb_entry(n, struct iova_dom, node);
		iova = iova_dom->iova;
		size = iova_dom->size;
		show_smmu_pte(mm_domain, iova, size);
	}
	spin_unlock(&cookie->iova_lock);
}
#endif

phys_addr_t hisi_domain_get_ttbr(struct device *dev)
{
	int ret;
	struct iommu_domain *domain = NULL;
	phys_addr_t ttbr = 0;

	if (!dev)
		return 0;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s has no iommu domain!\n", __func__);
		return 0;
	}

	if (!domain->ops) {
		dev_err(dev, "%s domain iommu ops is null!\n", __func__);
		return 0;
	}

	if (!domain->ops->domain_get_attr) {
		dev_err(dev, "%s domain_get_attr ops is null!\n", __func__);
		return 0;
	}

	ret = domain->ops->domain_get_attr(domain, DOMAIN_ATTR_TTBR, &ttbr);
	if (ret) {
		dev_info(dev, "%s domain not support get ttbr, ret = %d\n", __func__, ret);
		return 0;
	}

	return ttbr;
}
EXPORT_SYMBOL(hisi_domain_get_ttbr);

void __dmabuf_release_iommu(struct dma_buf *dmabuf,
				struct iommu_domain domain)
{
	struct mm_dom_cookie *cookie = NULL;
	struct iova_dom *iova_dom = NULL;
	unsigned long iova;
	int ret;

	cookie = domain.iova_cookie;
	if (!cookie) {
		pr_err("%s: has no cookie!\n", __func__);
		return;
	}
	spin_lock(&cookie->iova_lock);
	iova_dom = mm_iova_dom_get(&cookie->iova_root, dmabuf, 0);
	if (!iova_dom) {
		spin_unlock(&cookie->iova_lock);
		return;
	}

	rb_erase(&iova_dom->node, &cookie->iova_root);
	spin_unlock(&cookie->iova_lock);
	iova = iova_dom->iova;
	atomic64_set(&iova_dom->ref, 0);

	ret = do_iommu_unmap_sg(cookie->domain, iova_dom->size, iova);
	if (ret)
		pr_err("%s: do_iommu_unmap_sg return %d error!\n",
		__func__, ret);
	kfree(iova_dom);
}

void dmabuf_release_iommu(struct dma_buf *dmabuf)
{
	struct mm_domain *dom = NULL;
	struct mm_domain *tmp = NULL;

	if (!dmabuf) {
		pr_err("%s: invalid dma_buf!\n", __func__);
		return;
	}

	list_for_each_entry_safe(dom, tmp, &domain_list, list)
		__dmabuf_release_iommu(dmabuf, dom->domain);

	arm_smmu_dmabuf_release_iommu(dmabuf);
}

static int mm_iommu_sg_node_map(
	struct iommu_domain *domain, unsigned long iova, u32 policy_id,
	struct scatterlist *sgl, struct mm_dss_size *dss)
{
	size_t len = 0;
	int i, nents, prot;
	struct scatterlist *sg = NULL;

	nents = sg_nents(sgl);
	for_each_sg(sgl, sg, nents, i) {
		iova += len;
		len = sg->length;
		/* map lb first */
		prot = IOMMU_READ | IOMMU_WRITE;
#ifdef CONFIG_HISI_LB
		prot = (u32)prot | (policy_id << IOMMU_PORT_SHIFT);
#endif
		if (dss->lb_size >= len) {
			if (iommu_map(domain, iova, sg_phys(sg), len, prot)) {
				pr_err("lb map fail length: 0x%zx\n", len);
				return -EINVAL;
			}
			dss->lb_size -= len;
			dss->map_size += len;
			continue;
		}

		/* map l3 second */
		prot = IOMMU_CACHE | IOMMU_READ | IOMMU_WRITE;
		if (dss->l3_size >= len) {
			if (iommu_map(domain, iova, sg_phys(sg), len, prot))  {
				pr_err("l3 map fail length: 0x%zx\n", len);
				return -EINVAL;
			}
			dss->l3_size -= len;
			dss->map_size += len;
			continue;
		}

		/* map last */
		prot = IOMMU_READ | IOMMU_WRITE;
		if (((dss->all_size - dss->map_size)  > 0) && (len > 0)) {
			if (iommu_map(domain, iova, sg_phys(sg), len, prot))  {
				pr_err("map last fail length: 0x%zx\n", len);
				return -EINVAL;
			}
			dss->map_size += len;
			continue;
		}

		/* map fail */
		pr_err("all map fail length:0x%zx\n", len);

		return -EINVAL;
	}

	return 0;
}

static void mm_iommu_init_dss_size(size_t l3size, size_t lbsize,
				size_t allsize, struct mm_dss_size *dss)
{
	dss->l3_size = l3size;
	dss->lb_size = lbsize;
	dss->all_size = allsize;
	dss->map_size = 0;
}

static bool is_size_valid(size_t allsize, size_t l3size, size_t lbsize)
{
	bool ret = true;

	if (!PAGE_ALIGNED(allsize) || !PAGE_ALIGNED(l3size) ||
	    !PAGE_ALIGNED(lbsize))
		ret = false;
	if (l3size + lbsize >= lbsize && allsize < l3size + lbsize)
		ret = false;
	return ret;
}

#ifdef CONFIG_HISI_LB
static int mm_idle_display_lb_attach(struct ion_buffer *buffer, u32 plc_id)
{
	int ret = 0;
	struct sg_table *table = buffer->sg_table;

	mutex_lock(&buffer->lock);
	ret = lb_sg_attach(plc_id, table->sgl, sg_nents(table->sgl));
	if (ret)
		pr_err("idle display lb attach fail\n");
	mutex_unlock(&buffer->lock);
	return ret;
}

static void mm_idle_display_lb_detach(struct ion_buffer *buffer, u32 plc_id)
{
	struct sg_table *table = buffer->sg_table;

	mutex_lock(&buffer->lock);
	(void)lb_sg_detach(plc_id, table->sgl, sg_nents(table->sgl));
	mutex_unlock(&buffer->lock);
}
#else
static inline int mm_idle_display_lb_attach(struct ion_buffer *buffer,
		u32 plc_id)
{
	return 0;
}

static void mm_idle_display_lb_detach(struct ion_buffer *buffer, u32 plc_id)
{
	return;
}
#endif

int hisi_iommu_idle_display_unmap(struct device *dev, unsigned long iova,
		size_t size, u32 policy_id, struct dma_buf *dmabuf)
{
	int ret;
	struct ion_buffer *buffer = NULL;

	ret = hisi_iommu_unmap(dev, iova, size);

	if (!dmabuf) {
		pr_err("%s input err! dmabuf!\n", __func__);
		return ret;
	}

	buffer = dmabuf->priv;
	mm_idle_display_lb_detach(buffer, policy_id);

	return ret;
}

unsigned long hisi_iommu_idle_display_map(struct device *dev, u32 policy_id,
				struct dma_buf *dmabuf, size_t allsize,
				size_t l3size, size_t lbsize)
{
	struct iommu_domain *domain = NULL;
	unsigned long iova;
	struct mm_dss_size dss_size;
	struct mm_dom_cookie *cookie = NULL;
	struct ion_buffer *buffer = NULL;

	if (!dev || !dmabuf) {
		pr_err("input err! dev %pK or dmabuf %pK is null!\n", dev, dmabuf);
		return 0;
	}

	if (!is_ion_dma_buf(dmabuf)) {
		dev_err(dev, "dmabuf is not ion buffer\n");
		return 0;
	}

	if (!is_size_valid(allsize, l3size, lbsize)) {
		dev_err(dev, "%s, input size is valid!\n", __func__);
		return 0;
	}

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s, iommu domain is null!\n", __func__);
		return 0;
	}

	cookie = domain->iova_cookie;
	if (!cookie) {
		dev_err(dev, "%s, iova_cookie is null!\n", __func__);
		return 0;
	}

	if (!cookie->iova_pool) {
		dev_err(dev, "%s, iova_pool is null\n", __func__);
		return 0;
	}

	iova = mm_iommu_alloc_iova(cookie->iova_pool, allsize,
		cookie->iova.iova_align);
	if (!iova) {
		pr_err("alloc iova failed! size 0x%zx\n", allsize);
		return 0;
	}

	buffer = dmabuf->priv;
	if (mm_idle_display_lb_attach(buffer, policy_id))
		return 0;

	mm_iommu_init_dss_size(l3size, lbsize, allsize, &dss_size);
	if (mm_iommu_sg_node_map(domain, iova, policy_id,
			buffer->sg_table->sgl, &dss_size))
		goto err;

	return iova;

err:
	iommu_unmap(domain, iova, dss_size.map_size);
	mm_iommu_free_iova(cookie->iova_pool, iova, allsize);
	mm_idle_display_lb_detach(buffer, policy_id);

	return 0;
}
EXPORT_SYMBOL(hisi_iommu_idle_display_map);

#ifdef CONFIG_HISI_IOMMU_LAST_PAGE
static void mm_iommu_sg_table_add_padding_sg(
	struct sg_table *table,
	struct scatterlist *orig_sgl, unsigned int orig_nents,
	struct scatterlist padd_sg, unsigned int padd_nents)
{
	unsigned int i;
	unsigned long page_link;
	struct scatterlist *sg = NULL;
	struct scatterlist *orig_sg = orig_sgl;

	for_each_sg(table->sgl, sg, (orig_nents + padd_nents), i) {
		if (i < orig_nents) {
			memcpy(sg, orig_sg, sizeof(*sg));
			if (sg_is_last(sg))
				sg_unmark_end(sg);
			orig_sg = sg_next(orig_sg);
		} else {
			page_link = sg->page_link;
			memcpy(sg, &padd_sg, sizeof(*sg));
			sg->page_link |= page_link;
		}
	}
}

struct sg_table *mm_iommu_scatterlist_add_padding(
	struct scatterlist *orig_sgl, unsigned long padding_len)
{
	int ret;
	unsigned int orig_nents, padding_nents;
	struct sg_table *table = NULL;
	struct scatterlist padding_sg;

	if (padding_len == 0) {
		pr_err("%s: padding_len is 0!\n", __func__);
		goto out;
	}

	table = kzalloc(sizeof(*table), GFP_KERNEL);
	if (!table)
		goto out;

	memcpy(&padding_sg, orig_sgl, sizeof(padding_sg));
	sg_unmark_end(&padding_sg);

	orig_nents = sg_nents(orig_sgl);
	if ((orig_nents == 0) || (padding_sg.length == 0)) {
		pr_err("%s: orig_nents=%d or padding_sg.length=%d is 0!\n",
			__func__, orig_nents, padding_sg.length);
		goto free_table;
	}

	padding_nents =  ((padding_len + padding_sg.length - 1) /
		padding_sg.length);
	ret = sg_alloc_table(table, (padding_nents + orig_nents), GFP_KERNEL);
	if (ret) {
		pr_err("%s: sg_alloc_table ret %d error! len=%u add %u\n",
			__func__, ret, padding_nents, orig_nents);
		goto free_table;
	}

	mm_iommu_sg_table_add_padding_sg(table, orig_sgl, orig_nents,
		padding_sg, padding_nents);

	return table;
free_table:
	kfree(table);
	table = NULL;
out:
	return table;
}

static void iova_dom_save_padding_size(struct device *dev,
	struct dma_buf *dmabuf, unsigned long padding_size)
{
	struct iova_dom *iova_dom = NULL;
	struct iommu_domain *domain = NULL;
	struct mm_dom_cookie *cookie = NULL;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s, iommu domain is null\n", __func__);
		return;
	}

	cookie = domain->iova_cookie;
	if (!cookie) {
		dev_err(dev, "%s, iova cookie is null!\n", __func__);
		return;
	}

	spin_lock(&cookie->iova_lock);
	iova_dom = mm_iova_dom_get(&cookie->iova_root, dmabuf, 0);
	if (!iova_dom) {
		spin_unlock(&cookie->iova_lock);
		dev_err(dev, "%s, iova_dom is null!\n", __func__);
		return;
	}

	iova_dom->pad_size = padding_size;

	spin_unlock(&cookie->iova_lock);
}

/**
 * hisi_iommu_map_padding_dmabuf() - Map ION buffer's dmabuf to iova
 * @dev: master's device struct
 * @dmabuf: ION buffer's dmabuf, must be allocated by ION
 * @padding_len: iova padding length, out_size = padding_len + dmabuf length
 * @prot: iommu map prot (eg: IOMMU_READ/IOMMU_WRITE/IOMMU_CACHE etc..)
 * @out_size: return iova size to master's driver if map success
 *
 * When map success return iova, otherwise return 0.
 * This function is called master dev's driver. The master's device tree
 * must quote master's smmu device tree.
 * This func will work with iova refs
 */
unsigned long hisi_iommu_map_padding_dmabuf(
	struct device *dev, struct dma_buf *dmabuf, unsigned long padding_len,
	int prot, unsigned long *out_size)
{
	struct sg_table *table = NULL;
	struct sg_table *padding_table = NULL;
	struct dma_buf_attachment *attach = NULL;
	unsigned long iova;

	if (!dev || !dmabuf || !out_size) {
		pr_err("input err! dev %pK, dmabuf %pK\n", dev, dmabuf);
		return 0;
	}

	table = __dmabuf_get_sgt(dev, dmabuf, &attach);
	if (!table)
		return 0;

	padding_table = mm_iommu_scatterlist_add_padding(table->sgl,
		padding_len);
	if (!padding_table) {
		dev_err(dev, "padding_table is null\n");
		iova = 0;
		goto release_attach;
	}

	iova = __mm_iommu_map_dmabuf(dev, dmabuf, prot, out_size, padding_table);
	if (iova)
		iova_dom_save_padding_size(dev, dmabuf, padding_len);

	sg_free_table(padding_table);
	kfree(padding_table);

release_attach:
	__release_dmabuf_attach(dmabuf, attach, table);

	return iova;
}
EXPORT_SYMBOL(hisi_iommu_map_padding_dmabuf);

static int unmap_padding_dmabuf_check_pad_size(struct device *dev,
	struct dma_buf *dmabuf, unsigned long padding_size)
{
	struct iova_dom *iova_dom = NULL;
	struct iommu_domain *domain = NULL;
	struct mm_dom_cookie *cookie = NULL;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s, iommu domain is null!\n", __func__);
		return -EINVAL;
	}

	cookie = domain->iova_cookie;
	if (!cookie) {
		dev_err(dev, "%s, iommu domain cookie is null!\n", __func__);
		return -EINVAL;
	}

	spin_lock(&cookie->iova_lock);
	iova_dom = mm_iova_dom_get(&cookie->iova_root, dmabuf, 0);
	if (!iova_dom) {
		spin_unlock(&cookie->iova_lock);
		dev_err(dev, "%s, iova_dom is null!\n", __func__);
		return -EINVAL;
	}

	if (iova_dom->pad_size != padding_size) {
		spin_unlock(&cookie->iova_lock);
		dev_err(dev,
			"%s, size not equal! iova_dom->pad_size:0x%lx, padding_size:0x%lx\n",
			__func__, iova_dom->pad_size, padding_size);
		return -EINVAL;
	}
	spin_unlock(&cookie->iova_lock);

	return 0;
}

/**
 * hisi_iommu_unmap_padding_dmabuf() - Unmap ION buffer's dmabuf and iova
 * @dev: master's device struct
 * @dmabuf: ION buffer's dmabuf, must be allocated by ION
 * @padding_len: iova padding length, out_size = padding_len + dmabuf length
 * @iova: iova which get by hisi_iommu_map_dmabuf()
 *
 * When unmap success return 0, otherwise return ERRNO.
 * This function is called master dev's driver. The master's device tree
 * must quote master's smmu device tree.
 * This func will work with iova refs
 */
int hisi_iommu_unmap_padding_dmabuf(struct device *dev, struct dma_buf *dmabuf,
				unsigned long padding_len, unsigned long iova)
{
	int ret;

	if (!dev || !dmabuf || !iova) {
		pr_err("input err! dev %pK, dmabuf %pK, iova 0x%lx\n",
			dev, dmabuf, iova);
		return -EINVAL;
	}

	ret = unmap_padding_dmabuf_check_pad_size(dev, dmabuf, padding_len);
	if (ret) {
		dev_err(dev, "%s, incorrect input padding_len!\n", __func__);
		return -EINVAL;
	}

	ret = hisi_iommu_unmap_dmabuf(dev, dmabuf, iova);

	return ret;
}
EXPORT_SYMBOL(hisi_iommu_unmap_padding_dmabuf);
#endif
