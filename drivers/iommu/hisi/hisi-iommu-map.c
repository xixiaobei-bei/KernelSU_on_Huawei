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
 * Author: HISI_DRV1
 * Create: 2013-12-22
 */

#define pr_fmt(fmt) "[IOMMU: ]" fmt

#include <linux/debugfs.h>
#include <linux/genalloc.h>
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
#include <linux/dma-buf.h>
#include <linux/hisi-iommu.h>
#include "hisi_smmu.h"
#include "ion.h"

#define HISI_SMMU_LPAE_IDS_NUM	1

struct hisi_dss_size {
	size_t all_size;
	size_t l3_size;
	size_t lb_size;
	size_t map_size;
};

static unsigned long hisi_iommu_alloc_iova(struct gen_pool *iova_pool,
				size_t size, unsigned long align)
{
	unsigned long iova;

	iova = gen_pool_alloc(iova_pool, size);
	if (!iova) {
		pr_err("hisi iommu gen_pool_alloc failed! size = %lu\n", size);
		return 0;
	}

	if (iova_pool->min_alloc_order >= 0) {
		if (align > (1UL << (unsigned long)iova_pool->min_alloc_order))
			WARN(1, "hisi iommu domain cant align to 0x%lx\n",
			     align);
	} else {
		pr_warn("The min_alloc_order of iova_pool is negative!\n");
		return -EINVAL;
	}

	return iova;
}

static void hisi_iommu_free_iova(struct gen_pool *iova_pool,
				unsigned long iova, size_t size)
{
	if (!iova_pool)
		return;

	gen_pool_free(iova_pool, iova, size);
}

#ifdef CONFIG_ARM_SMMU_V3
static struct gen_pool *hisi_iommu_get_iova_pool(struct device *dev)
{
	int num_ids;
	struct hisi_dom_cookie *cookie = NULL;
	struct hisi_domain *hisi_domain = NULL;
	struct iommu_domain *domain = NULL;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s, iommu domain is null\n", __func__);
		return NULL;
	}

	num_ids = dev->iommu_fwspec->num_ids;
	/* smmu lpae iommus only one parameter sid */
	if (num_ids == HISI_SMMU_LPAE_IDS_NUM) {
		hisi_domain = to_hisi_domain(domain);
		return hisi_domain->iova_pool;
	}

	cookie = domain->iova_cookie;
	if (!cookie) {
		dev_err(dev, "%s, iommu domain cookie is null!\n", __func__);
		return NULL;
	}
	return cookie->iova_pool;
}

static struct rb_root *hisi_iommu_get_iova_root(struct device *dev)
{
	int num_ids;
	struct hisi_dom_cookie *cookie = NULL;
	struct hisi_domain *hisi_domain = NULL;
	struct iommu_domain *domain = NULL;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s, iommu domain is null\n", __func__);
		return NULL;
	}

	num_ids = dev->iommu_fwspec->num_ids;
	/* smmu lpae iommus only one parameter sid */
	if (num_ids == HISI_SMMU_LPAE_IDS_NUM) {
		hisi_domain = to_hisi_domain(domain);
		return &hisi_domain->iova_root;
	}

	cookie = domain->iova_cookie;
	if (!cookie) {
		dev_err(dev, "%s, iommu domain cookie is null!\n", __func__);
		return NULL;
	}
	return &cookie->iova_root;
}

static spinlock_t *hisi_iommu_get_iova_lock(struct device *dev)
{
	int num_ids;
	struct hisi_dom_cookie *cookie = NULL;
	struct hisi_domain *hisi_domain = NULL;
	struct iommu_domain *domain = NULL;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s, iommu domain is null\n", __func__);
		return NULL;
	}

	num_ids = dev->iommu_fwspec->num_ids;
	/* smmu lpae iommus only one parameter sid */
	if (num_ids == HISI_SMMU_LPAE_IDS_NUM) {
		hisi_domain = to_hisi_domain(domain);
		return &hisi_domain->iova_lock;
	}

	cookie = domain->iova_cookie;
	if (!cookie) {
		dev_err(dev, "%s, iommu domain cookie is null!\n", __func__);
		return NULL;
	}
	return &cookie->iova_lock;
}

static unsigned long hisi_iommu_get_iova_align(struct device *dev)
{
	int num_ids;
	struct hisi_dom_cookie *cookie = NULL;
	struct hisi_domain *hisi_domain = NULL;
	struct iommu_domain *domain = NULL;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s, iommu domain is null\n", __func__);
		return 0;
	}

	num_ids = dev->iommu_fwspec->num_ids;
	/* smmu lpae iommus only one parameter sid */
	if (num_ids == HISI_SMMU_LPAE_IDS_NUM) {
		hisi_domain = to_hisi_domain(domain);
		return hisi_domain->domain_data->iova_align;
	}

	cookie = domain->iova_cookie;
	if (!cookie) {
		dev_err(dev, "%s, iommu domain cookie is null!\n", __func__);
		return 0;
	}
	return cookie->iova.iova_align;
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

static void hisi_iova_add(struct rb_root *rb_root,
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

static struct iova_dom *hisi_iova_dom_get(struct rb_root *rbroot,
				struct dma_buf *dmabuf)
{
	struct rb_node *n = NULL;
	struct iova_dom *iova_dom = NULL;
	u64 key = (u64)dmabuf;

	for (n = rb_first(rbroot); n; n = rb_next(n)) {
		iova_dom = rb_entry(n, struct iova_dom, node);
		if (iova_dom->key == key)
			return iova_dom;
	}

	return NULL;
}

void hisi_iova_dom_info(struct device *dev)
{
	struct rb_node *n = NULL;
	struct iova_dom *iova_dom = NULL;
	unsigned long total_size = 0;
	spinlock_t *iova_lock = NULL;
	struct rb_root *iova_root = NULL;

	iova_lock = hisi_iommu_get_iova_lock(dev);
	if (!iova_lock) {
		dev_err(dev, "%s, get iommu domain iova_lock is null!\n",
			__func__);
		return;
	}

	iova_root = hisi_iommu_get_iova_root(dev);
	if (!iova_root) {
		dev_err(dev, "%s, get iommu domain iova_root is null!\n",
			__func__);
		return;
	}

	spin_lock(iova_lock);
	for (n = rb_first(iova_root); n; n = rb_next(n)) {
		iova_dom = rb_entry(n, struct iova_dom, node);
		total_size += iova_dom->size;
		pr_err("0x%lx 0x%lx\n", iova_dom->iova, iova_dom->size);
	}
	spin_unlock(iova_lock);
}

#ifdef CONFIG_ARM64_64K_PAGES
#error hisi iommu can not deal with 64k pages!
#endif

static size_t hisi_iommu_sg_size_get(struct scatterlist *sgl, int nents)
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
	int nents;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		pr_err("Dev has no iommu domain!\n");
		return 0;
	}

	iova_pool = hisi_iommu_get_iova_pool(dev);
	if (!iova_pool) {
		pr_err("iommu domain iova pool is null\n");
		return 0;
	}

	nents = sg_nents(sgl);
	iova_size = hisi_iommu_sg_size_get(sgl, nents);
	iova = hisi_iommu_alloc_iova(iova_pool, iova_size,
		hisi_iommu_get_iova_align(dev));
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

static int do_iommu_unmap_sg(
	struct iommu_domain *domain, struct gen_pool *iova_pool,
	struct scatterlist *sgl, unsigned long iova)
{
	size_t iova_size;
	size_t unmap_size;
	int ret;
	int nents;

	nents = sg_nents(sgl);
	iova_size = hisi_iommu_sg_size_get(sgl, nents);
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

	hisi_iommu_free_iova(iova_pool, iova, iova_size);
	return 0;
}

int __dmabuf_release_iommu(struct dma_buf *dmabuf, struct iommu_domain domain)
{
	struct hisi_dom_cookie *cookie = NULL;
	struct ion_buffer *buffer = NULL;
	struct sg_table *table = NULL;
	struct iova_dom *iova_dom = NULL;
	unsigned long iova;
	int ret;

	if (!dmabuf) {
		pr_err("%s: invalid dma_buf!\n", __func__);
		return -EINVAL;
	}

	if (!is_ion_dma_buf(dmabuf)) {
		pr_err("%s: is_ion_dma_buf!\n", __func__);
		return -EINVAL;
	}

	buffer = dmabuf->priv;
	table = buffer->sg_table;
	cookie = domain.iova_cookie;
	if (!cookie) {
		pr_err("%s: has no cookie!\n", __func__);
		return -EINVAL;
	}
	spin_lock(&cookie->iova_lock);
	iova_dom = hisi_iova_dom_get(&cookie->iova_root, dmabuf);
	if (!iova_dom) {
		spin_unlock(&cookie->iova_lock);
		return -EINVAL;
	}

	rb_erase(&iova_dom->node, &cookie->iova_root);
	spin_unlock(&cookie->iova_lock);
	iova = iova_dom->iova;
	atomic64_set(&iova_dom->ref, 0);

	ret = do_iommu_unmap_sg(cookie->domain, cookie->iova_pool,
		table->sgl, iova);
	if (ret)
		pr_err("%s: do_iommu_unmap_sg return %d error!\n",
		__func__, ret);
	kfree(iova_dom);
	return 0;
}

static int hisi_iommu_get_dmabuf_iova(struct device *dev,
				struct dma_buf *dmabuf, unsigned long *iova,
				unsigned long *out_size)
{
	spinlock_t *iova_lock = NULL;
	struct rb_root *iova_root = NULL;
	struct iova_dom *iova_dom = NULL;

	(*iova) = 0;
	iova_lock = hisi_iommu_get_iova_lock(dev);
	if (!iova_lock) {
		dev_err(dev, "%s, get iommu domain iova_lock is null!\n",
			__func__);
		return -ENOENT;
	}

	iova_root = hisi_iommu_get_iova_root(dev);
	if (!iova_root) {
		dev_err(dev, "%s, get iommu domain iova_root is null!\n",
			__func__);
		return -ENOENT;
	}

	spin_lock(iova_lock);
	iova_dom = hisi_iova_dom_get(iova_root, dmabuf);
	if (iova_dom) {
		atomic64_inc(&iova_dom->ref);
		*out_size = iova_dom->size;
		(*iova) = iova_dom->iova;
	}
	spin_unlock(iova_lock);

	return 0;
}

static void hisi_iommu_iova_domain_register(struct device *dev,
				struct iova_dom *iova_dom)
{
	spinlock_t *iova_lock = NULL;
	struct rb_root *iova_root = NULL;

	iova_lock = hisi_iommu_get_iova_lock(dev);
	if (!iova_lock) {
		dev_err(dev, "%s, get iommu domain iova_lock is null!\n",
			__func__);
		return;
	}

	iova_root = hisi_iommu_get_iova_root(dev);
	if (!iova_root) {
		dev_err(dev, "%s, get iommu domain iova_root is null!\n",
			__func__);
		return;
	}

	spin_lock(iova_lock);
	hisi_iova_add(iova_root, iova_dom);
	spin_unlock(iova_lock);
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
	struct ion_buffer *buffer = NULL;
	struct sg_table *table = NULL;
	struct iova_dom *iova_dom = NULL;
	unsigned long iova;

	if (!dev || !dmabuf || !out_size) {
		dev_err(dev, "input err! dev %pK, dmabuf %pK\n", dev, dmabuf);
		return 0;
	}

	if (!is_ion_dma_buf(dmabuf)) {
		dev_err(dev, "dmabuf is not ion buffer\n");
		return 0;
	}

	if (hisi_iommu_get_dmabuf_iova(dev, dmabuf, &iova, out_size)) {
		dev_err(dev, "get dmabuf iova error\n");
		return 0;
	}

	if (iova)
		return iova;

	iova_dom = kzalloc(sizeof(*iova_dom), GFP_KERNEL);
	if (!iova_dom)
		goto err;

	atomic64_set(&iova_dom->ref, 1);
	iova_dom->key = (u64)dmabuf;

	buffer = dmabuf->priv;
	table = buffer->sg_table;

#ifdef CONFIG_HISI_LB
	prot |= (unsigned long)buffer->plc_id << IOMMU_PORT_SHIFT;
#endif
	prot |= (IOMMU_READ | IOMMU_WRITE);
	iova = do_iommu_map_sg(dev, table->sgl, prot, &iova_dom->size);
	if (!iova) {
		pr_err("%s, do_iommu_map_sg\n", __func__);
		goto free_dom;
	}

	iova_dom->iova = iova;
	*out_size = iova_dom->size;
	iova_dom->dev = dev;
	hisi_iommu_iova_domain_register(dev, iova_dom);
	return iova;

free_dom:
	kfree(iova_dom);
	hisi_iova_dom_info(dev);

err:
	pr_err("err out %s\n", __func__);
	return 0;
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
	struct ion_buffer *buffer = NULL;
	struct sg_table *table = NULL;
	struct iova_dom *iova_dom = NULL;
	spinlock_t *iova_lock = NULL;
	struct rb_root *iova_root = NULL;
	struct gen_pool *iova_pool = NULL;
	struct iommu_domain *domain = NULL;

	if (!dev || !dmabuf || !iova) {
		pr_err("input err! dev %pK, dmabuf %pK, iova 0x%lx\n",
			dev, dmabuf, iova);
		return -EINVAL;
	}

	if (!is_ion_dma_buf(dmabuf)) {
		dev_err(dev, "dmabuf is not ion buffer\n");
		return -EINVAL;
	}

	iova_lock = hisi_iommu_get_iova_lock(dev);
	iova_root = hisi_iommu_get_iova_root(dev);
	iova_pool = hisi_iommu_get_iova_pool(dev);
	if ((!iova_lock) || (!iova_root) || (!iova_pool)) {
		dev_err(dev, "%s,iova lock,root or pool is null!\n", __func__);
		return -EINVAL;
	}

	domain = iommu_get_domain_for_dev(dev);

	spin_lock(iova_lock);
	iova_dom = hisi_iova_dom_get(iova_root, dmabuf);
	if (!iova_dom) {
		spin_unlock(iova_lock);
		dev_err(dev, "%s,unmap buf no map data!\n");
		return -EINVAL;
	}

	if (!atomic64_dec_and_test(&iova_dom->ref)) {
		spin_unlock(iova_lock);
		return 0;
	}

	if (iova_dom->iova != iova) {
		spin_unlock(iova_lock);
		dev_err(dev, "iova no eq:0x%lx,0x%lx\n", iova, iova_dom->iova);
		return -EINVAL;
	}

	rb_erase(&iova_dom->node, iova_root);
	spin_unlock(iova_lock);

	kfree(iova_dom);

	buffer = dmabuf->priv;
	table = buffer->sg_table;

	return do_iommu_unmap_sg(domain, iova_pool, table->sgl, iova);
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

	prot |= IOMMU_READ | IOMMU_WRITE;
	iova = do_iommu_map_sg(dev, sgl, prot, out_size);

	return iova;
}
EXPORT_SYMBOL(hisi_iommu_map_sg);

int hisi_iommu_unmap_sg(struct device *dev, struct scatterlist *sgl,
				unsigned long iova)
{
	int ret;
	struct iommu_domain *domain = NULL;
	struct hisi_dom_cookie *iova_pool = NULL;

	if (!dev || !sgl || !iova) {
		pr_err("input err! dev %pK, sgl %pK, iova 0x%lx\n",
			dev, sgl, iova);
		return -EINVAL;
	}

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s, iommu domain is null\n", __func__);
		return NULL;
	}

	iova_pool = hisi_iommu_get_iova_pool(dev);
	if (!iova_pool) {
		dev_err(dev, "%s,get iova_pool is null!\n", __func__);
		return -EINVAL;
	}

	ret = do_iommu_unmap_sg(domain, iova_pool, sgl, iova);
	return ret;
}
EXPORT_SYMBOL(hisi_iommu_unmap_sg);

phys_addr_t hisi_domain_get_ttbr(struct device *dev)
{
	return 0;
}
EXPORT_SYMBOL(hisi_domain_get_ttbr);

unsigned long hisi_iommu_map(struct device *dev, phys_addr_t paddr,
				size_t size, int prot)
{
	struct iommu_domain *domain = NULL;
	unsigned long iova;
	unsigned long iova_align;
	size_t iova_size;
	int ret;
	struct gen_pool *iova_pool = NULL;

	if (!dev || !paddr) {
		pr_err("input Err! Dev %pK, addr =0x%llx\n", dev, paddr);
		return 0;
	}

	if (!size) {
		pr_err("size is 0! no need to map\n");
		return 0;
	}

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		pr_err("Dev(%s) has no iommu domain!\n", dev_name(dev));
		return 0;
	}

	iova_pool = hisi_iommu_get_iova_pool(dev);
	if (!iova_pool) {
		dev_err(dev, "%s,get iova_pool is null!\n",
			__func__);
		return 0;
	}

	iova_size = ALIGN(size, PAGE_SIZE);
	iova_align = hisi_iommu_get_iova_align(dev);
	iova = hisi_iommu_alloc_iova(iova_pool, iova_size, iova_align);
	if (!iova) {
		pr_err("Dev(%s) alloc iova failed! size 0x%zx\n",
			dev_name(dev), size);
		return 0;
	}
	prot |= IOMMU_READ | IOMMU_WRITE;
	ret = iommu_map(domain, iova, paddr, size, prot);
	if (ret) {
		pr_err("map fail! address 0x%llx,  size 0x%zx\n",
			paddr, size);
		hisi_iommu_free_iova(iova_pool, iova, size);
	}
	return iova;
}
EXPORT_SYMBOL(hisi_iommu_map);

int hisi_iommu_unmap(struct device *dev, unsigned long iova, size_t size)
{
	struct iommu_domain *domain = NULL;
	size_t iova_size;
	size_t unmap_size;
	struct gen_pool *iova_pool = NULL;

	if (!dev || !iova) {
		pr_err("input err! dev %pK, iova 0x%lx\n",
			dev, iova);
		return -EINVAL;
	}

	if (!size) {
		pr_err("input err! dev %pK, size is 0\n", dev);
		return -EINVAL;
	}

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		pr_err("Dev(%s) has no iommu domain!\n", dev_name(dev));
		return 0;
	}

	iova_pool = hisi_iommu_get_iova_pool(dev);
	if (!iova_pool) {
		dev_err(dev, "%s,get iova_pool is null!\n",
			__func__);
		return 0;
	}

	iova_size = ALIGN(size, PAGE_SIZE);
	unmap_size = iommu_unmap(domain, iova, iova_size);
	if (unmap_size != size) {
		pr_err("unmap fail! size 0x%lx, unmap_size 0x%zx\n",
			size, unmap_size);
		return -EINVAL;
	}

	hisi_iommu_free_iova(iova_pool, iova, size);

	return 0;
}
EXPORT_SYMBOL(hisi_iommu_unmap);

static bool is_size_valid(size_t allsize, size_t l3size, size_t lbsize)
{
	bool ret = true;

	if (!PAGE_ALIGNED(allsize))
		ret = false;

	if (!PAGE_ALIGNED(l3size))
		ret = false;

	if (!PAGE_ALIGNED(lbsize))
		ret = false;

	if (l3size + lbsize >= lbsize && allsize < l3size + lbsize)
		ret = false;

	return ret;
}

static int hisi_iommu_phys_addr_map(struct iommu_domain *domain,
				u32 policy_id, phys_addr_t paddr,
				unsigned long iova, struct hisi_dss_size *dss)
{
	int prot;
	int ret;

	/* map l3 fisrt */
	prot = IOMMU_CACHE | IOMMU_READ | IOMMU_WRITE;
	if (dss->l3_size > 0) {
		ret = iommu_map(domain, iova, paddr, dss->l3_size, prot);
		if (ret)
			return ret;
	}
	iova += dss->l3_size;
	paddr += dss->l3_size;
	dss->map_size += dss->l3_size;

	/* map lb second */
	prot = IOMMU_READ | IOMMU_WRITE;
#ifdef CONFIG_HISI_LB
	prot = (u32)prot | (policy_id << IOMMU_PORT_SHIFT);
#endif
	if (dss->lb_size) {
		ret = iommu_map(domain, iova, paddr, dss->lb_size, prot);
		if (ret)
			return ret;
	}
	iova += dss->lb_size;
	paddr += dss->lb_size;
	dss->map_size += dss->lb_size;

	/* map last */
	prot = IOMMU_READ | IOMMU_WRITE;
	if (dss->all_size > dss->map_size) {
		ret = iommu_map(domain, iova, paddr,
			(dss->all_size - dss->map_size), prot);
		if (ret)
			return ret;
	}
	return 0;
}

int hisi_iommu_idle_display_map(struct device *dev, u32 policy_id,
				phys_addr_t paddr, size_t allsize,
				size_t l3size, size_t lbsize)
{
	unsigned long iova;
	struct hisi_dss_size dss;
	struct gen_pool *iova_pool = NULL;
	struct iommu_domain *domain = NULL;

	pr_err("%s, pa:0x%lx, allsize:0x%lx, l3size:0x%lx,lbsize:0x%lx,policy:%d\n",
		__func__, paddr, allsize, l3size, lbsize, policy_id);

	if (!dev) {
		pr_err("input err! dev %pK\n", dev);
		return -EINVAL;
	}

	if (!is_size_valid(allsize, l3size, lbsize))
		return 0;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		pr_err("Dev(%s) has no iommu domain!\n", dev_name(dev));
		return 0;
	}

	iova_pool = hisi_iommu_get_iova_pool(dev);
	if (!iova_pool) {
		pr_err("dev(%s) iommu iova_pool is null\n", dev_name(dev));
		return 0;
	}

	iova = hisi_iommu_alloc_iova(iova_pool, allsize,
			hisi_iommu_get_iova_align(dev));
	if (!iova) {
		pr_err("Dev(%s) alloc iova failed! size 0x%zx\n",
			dev_name(dev), allsize);
		return 0;
	}

	dss.l3_size = l3size;
	dss.lb_size = lbsize;
	dss.all_size = allsize;
	dss.map_size = 0;
	if (hisi_iommu_phys_addr_map(domain, policy_id, paddr, iova, &dss)) {
		if (dss.map_size > 0)
			goto err;
		goto free_iova;
	}

	return iova;

err:
	iommu_unmap(domain, iova, dss.map_size);
free_iova:
	hisi_iommu_free_iova(iova_pool, iova, allsize);

	return 0;
}
EXPORT_SYMBOL(hisi_iommu_idle_display_map);

#else
struct gen_pool *iova_pool_setup(unsigned long start, unsigned long size,
				unsigned long align)
{
	struct gen_pool *pool;
	int ret;

	pool = gen_pool_create(order_base_2(align), -1);
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
	gen_pool_destroy(pool);
}

static void hisi_iova_add(struct hisi_domain *hisi_domain,
				struct iova_dom *iova_dom)
{
	struct rb_node **p = &hisi_domain->iova_root.rb_node;
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
			WARN_ON(1);
			return;
		}
	}

	rb_link_node(&iova_dom->node, parent, p);
	rb_insert_color(&iova_dom->node, &hisi_domain->iova_root);
}

static struct iova_dom *hisi_iova_dom_get(struct hisi_domain *hisi_domain,
				struct dma_buf *dmabuf)
{
	struct rb_node *n = NULL;
	struct iova_dom *iova_dom = NULL;
	u64 key = (u64)dmabuf;

	for (n = rb_first(&hisi_domain->iova_root); n; n = rb_next(n)) {
		iova_dom = rb_entry(n, struct iova_dom, node);
		if (iova_dom->key == key)
			return iova_dom;
	}

	return NULL;
}

void hisi_iova_dom_info_show(struct hisi_domain *hisi_domain)
{
	struct rb_node *n = NULL;
	struct iova_dom *iova_dom = NULL;
	unsigned long total_size = 0;

	spin_lock(&hisi_domain->iova_lock);
	for (n = rb_first(&hisi_domain->iova_root); n; n = rb_next(n)) {
		iova_dom = rb_entry(n, struct iova_dom, node);
		total_size += iova_dom->size;
		pr_err("%16.s    0x%lx    %lu\n", dev_name(iova_dom->dev),
		       iova_dom->iova, iova_dom->size);
	}
	spin_unlock(&hisi_domain->iova_lock);

	pr_err("domain %s total size: %lu", dev_name(hisi_domain->dev),
	       total_size);
}

#ifdef CONFIG_HISI_IOMMU_TEST
static smmu_pte_t hisi_ptb_lpae(unsigned int iova, smmu_pgd_t *pgdp)
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

static smmu_pgd_t hisi_pgd_lpae(unsigned int iova, smmu_pgd_t *pgdp)
{
	return *(pgdp + smmu_pgd_index(iova));
}

static smmu_pmd_t hisi_pmd_lpae(unsigned int iova, smmu_pgd_t *pgdp)
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

static void show_smmu_pte(struct hisi_domain *hisi_domain, unsigned long iova,
				unsigned long size)
{
	unsigned long io_addr;
	smmu_pgd_t *pgdp = (smmu_pgd_t *)hisi_domain->va_pgtable_addr;

	for (io_addr = iova; io_addr < iova + size; io_addr += PAGE_SIZE)
		pr_err("iova[0x%lx]:pgd[%ld]=0x%llx,pmd[%ld]=0x%llx, pte[%ld]=0x%llx\n",
		       io_addr, smmu_pgd_index(io_addr),
		       hisi_pgd_lpae(io_addr, pgdp), smmu_pmd_index(io_addr),
		       hisi_pmd_lpae(io_addr, pgdp), smmu_pte_index(io_addr),
		       hisi_ptb_lpae(io_addr, pgdp));
}

void hisi_smmu_show_pte(struct device *dev, unsigned long iova,
				unsigned long size)
{
	struct hisi_domain *hisi_domain = NULL;
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

	hisi_domain = to_hisi_domain(domain);

	show_smmu_pte(hisi_domain, iova, size);
}

void hisi_print_iova_dom(struct device *dev)
{
	struct rb_node *n = NULL;
	struct iova_dom *iova_dom = NULL;
	unsigned long iova;
	unsigned long size;
	struct hisi_domain *hisi_domain = NULL;
	struct iommu_domain *domain = NULL;

	if (!dev) {
		pr_err("invalid params!\n");
		return;
	}

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		pr_err("dev(%s) has no iommu domain!\n", dev_name(dev));
		return;
	}

	hisi_domain = to_hisi_domain(domain);

	spin_lock(&hisi_domain->iova_lock);
	for (n = rb_first(&hisi_domain->iova_root); n; n = rb_next(n)) {
		iova_dom = rb_entry(n, struct iova_dom, node);
		iova = iova_dom->iova;
		size = iova_dom->size;
		show_smmu_pte(hisi_domain, iova, size);
	}
	spin_unlock(&hisi_domain->iova_lock);
}
#endif

#ifdef CONFIG_ARM64_64K_PAGES
#error hisi iommu can not deal with 64k pages!
#endif

phys_addr_t hisi_domain_get_ttbr(struct device *dev)
{
	struct hisi_domain *hisi_domain = NULL;
	struct iommu_domain *domain = NULL;

	if (!dev)
		return 0;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		pr_err("dev(%s) has no iommu domain!\n", dev_name(dev));
		return 0;
	}

	hisi_domain = to_hisi_domain(domain);

	return hisi_domain->pa_pgtable_addr;
}
EXPORT_SYMBOL(hisi_domain_get_ttbr);

static unsigned long do_iommu_map_sg(struct hisi_domain *hisi_domain,
				struct scatterlist *sgl, int prot,
				unsigned long *out_size)
{
	struct iommu_domain *domain = &hisi_domain->domain;
	struct scatterlist *sg = NULL;
	unsigned long iova;
	size_t iova_size = 0;
	size_t map_size;
	unsigned int nents, i;

	nents = sg_nents(sgl);
	for_each_sg(sgl, sg, nents, i)
		iova_size += (size_t)ALIGN(sg->length, PAGE_SIZE);

	iova = hisi_iommu_alloc_iova(hisi_domain->iova_pool, iova_size,
		hisi_domain->domain_data->iova_align);
	if (!iova) {
		pr_err("alloc iova failed! size 0x%zx\n", iova_size);
		return 0;
	}

	map_size = iommu_map_sg(domain, iova, sgl, nents, prot);
	if (map_size != iova_size) {
		pr_err("map Fail! iova 0x%lx, iova_size 0x%zx\n", iova,
		       iova_size);
		gen_pool_free(hisi_domain->iova_pool, iova, iova_size);
		return 0;
	}

	*out_size = (unsigned long)iova_size;
	return iova;
}

static int do_iommu_unmap_sg(struct hisi_domain *hisi_domain,
				struct scatterlist *sgl, unsigned long iova)
{
	struct iommu_domain *domain = &hisi_domain->domain;
	struct scatterlist *sg = NULL;
	size_t iova_size = 0;
	size_t unmap_size;
	unsigned int nents, i;
	int ret;

	hisi_domain = to_hisi_domain(domain);

	nents = sg_nents(sgl);
	for_each_sg(sgl, sg, nents, i)
		iova_size += (size_t)ALIGN(sg->length, PAGE_SIZE);

	ret = addr_in_gen_pool(hisi_domain->iova_pool, iova, iova_size);
	if (!ret) {
		pr_err("[%s]illegal para!!iova = %lx, size = %lx\n", __func__,
		       iova, iova_size);
		return -EINVAL;
	}

	unmap_size = iommu_unmap(domain, iova, iova_size);
	if (unmap_size != iova_size) {
		pr_err("unmap fail! size 0x%lx, unmap_size 0x%zx\n", iova_size,
		       unmap_size);
		return -EINVAL;
	}

	hisi_iommu_free_iova(hisi_domain->iova_pool, iova, iova_size);

	return 0;
}

void dmabuf_release_iommu(struct dma_buf *dmabuf)
{
	struct sg_table *table = NULL;
	struct hisi_domain *dom = NULL;
	struct hisi_domain *tmp = NULL;
	struct ion_buffer *buffer = NULL;
	struct iova_dom *iova_dom = NULL;
	unsigned long iova;
	int ret;

	if (!dmabuf)
		return;

	if (!is_ion_dma_buf(dmabuf))
		return;

	buffer = dmabuf->priv;
	table = buffer->sg_table;

	list_for_each_entry_safe(dom, tmp, &domain_list, list) {
		spin_lock(&dom->iova_lock);
		iova_dom = hisi_iova_dom_get(dom, dmabuf);
		if (!iova_dom) {
			spin_unlock(&dom->iova_lock);
			continue;
		}

		rb_erase(&iova_dom->node, &dom->iova_root);
		spin_unlock(&dom->iova_lock);
		iova = iova_dom->iova;
		atomic64_set(&iova_dom->ref, 0);

		ret = do_iommu_unmap_sg(dom, table->sgl, iova);
		if (ret)
			pr_err("%s: unmap sg return %d error!\n",
			__func__, ret);
		kfree(iova_dom);
	}
}

/**
 * hisi_iommu_map_sg() - Map ION buffer's dmabuf to iova
 * @dev: master's device struct
 * @sgl: scatterlist which want to map
 * @prot: iommu map prot (eg: IOMMU_READ/IOMMU_WRITE/IOMMU_CACHE etc..)
 * @out_size: return iova size to master's driver if map success
 *
 * When map success return iova, otherwise return 0.
 * This function is called master dev's driver. The master's device tree
 * must quote master's smmu device tree.
 * Attention: this func don't work with iova refs
 */
unsigned long hisi_iommu_map_sg(struct device *dev, struct scatterlist *sgl,
				int prot, unsigned long *out_size)
{
	struct gen_pool *iova_pool = NULL;
	struct iommu_domain *domain = NULL;
	struct hisi_domain *hisi_domain = NULL;

	unsigned long iova;

	if (!dev || !sgl) {
		pr_err("input err! dev %pK, sgl %pK\n", dev, sgl);
		return 0;
	}

#ifdef CONFIG_HISI_IOMMU_BYPASS
	*out_size = sgl->length;
	return sg_phys(sgl);
#endif

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		pr_err("dev(%s) has no iommu domain!\n", dev_name(dev));
		return 0;
	}

	hisi_domain = to_hisi_domain(domain);
	iova_pool = hisi_domain->iova_pool;
	if (!iova_pool) {
		pr_err("dev(%s) has no iova pool!\n", dev_name(dev));
		return 0;
	}

	iova = do_iommu_map_sg(hisi_domain, sgl, prot, out_size);

	return iova;
}
EXPORT_SYMBOL(hisi_iommu_map_sg);

/**
 * hisi_iommu_unmap_sg() - Unmap ION buffer's dmabuf and iova
 * @dev: master's device struct
 * @sgl: scatterlist which want to map
 * @iova: iova which get by hisi_iommu_map_sg()
 *
 * When unmap success return 0, otherwise return ERRNO.
 * This function is called master dev's driver. The master's device tree
 * must quote master's smmu device tree.
 * Attention: this func don't work with iova refs
 */
int hisi_iommu_unmap_sg(struct device *dev, struct scatterlist *sgl,
				unsigned long iova)
{
	struct gen_pool *iova_pool = NULL;
	struct iommu_domain *domain = NULL;
	struct hisi_domain *hisi_domain = NULL;

	int ret;

	if (!dev || !sgl || !iova) {
		pr_err("input err! dev %pK, sgl %pK, iova 0x%lx\n", dev, sgl,
		       iova);
		return -EINVAL;
	}

#ifdef CONFIG_HISI_IOMMU_BYPASS
	return 0;
#endif

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		pr_err("dev(%s) has no iommu domain!\n", dev_name(dev));
		return -EINVAL;
	}

	hisi_domain = to_hisi_domain(domain);
	iova_pool = hisi_domain->iova_pool;
	if (!iova_pool) {
		pr_err("dev(%s) has no iova pool!\n", dev_name(dev));
		return -ENOMEM;
	}

	ret = do_iommu_unmap_sg(hisi_domain, sgl, iova);

	return ret;
}
EXPORT_SYMBOL(hisi_iommu_unmap_sg);

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
	struct sg_table *table = NULL;
	struct ion_buffer *buffer = NULL;
	struct iova_dom *iova_dom = NULL;
	struct iommu_domain *domain = NULL;
	struct hisi_domain *hisi_domain = NULL;

	unsigned long iova;

	if (!dev || !dmabuf || !out_size) {
		pr_err("input err! dev %pK, dmabuf %pK\n", dev, dmabuf);
		return 0;
	}

	if (!is_ion_dma_buf(dmabuf)) {
		pr_err("dmabuf is not ion buffer\n");
		return 0;
	}

#ifdef CONFIG_HISI_IOMMU_BYPASS
	buffer = dmabuf->priv;
	table = buffer->sg_table;

	*out_size = table->sgl->length;
	return sg_phys(table->sgl);
#endif

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		pr_err("dev(%s) has no iommu domain!\n", dev_name(dev));
		return 0;
	}

	hisi_domain = to_hisi_domain(domain);

	spin_lock(&hisi_domain->iova_lock);
	iova_dom = hisi_iova_dom_get(hisi_domain, dmabuf);
	if (iova_dom) {
		atomic64_inc(&iova_dom->ref);
		spin_unlock(&hisi_domain->iova_lock);
		*out_size = iova_dom->size;
		return iova_dom->iova;
	}
	spin_unlock(&hisi_domain->iova_lock);

	iova_dom = kzalloc(sizeof(*iova_dom), GFP_KERNEL);
	if (!iova_dom)
		goto err;

	atomic64_set(&iova_dom->ref, 1);
	iova_dom->key = (u64)dmabuf;

	buffer = dmabuf->priv;
	table = buffer->sg_table;

#ifdef CONFIG_HISI_LB
	prot = (u32)prot | (buffer->plc_id << IOMMU_PORT_SHIFT);
#endif

	iova = do_iommu_map_sg(hisi_domain, table->sgl, prot, &iova_dom->size);
	if (!iova)
		goto free_dom;

	iova_dom->iova = iova;
	*out_size = iova_dom->size;
	iova_dom->dev = dev;
	spin_lock(&hisi_domain->iova_lock);
	hisi_iova_add(hisi_domain, iova_dom);
	spin_unlock(&hisi_domain->iova_lock);

	return iova;
free_dom:
	kfree(iova_dom);
	hisi_iova_dom_info_show(hisi_domain);
err:
	return 0;
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
	struct sg_table *table = NULL;
	struct ion_buffer *buffer = NULL;
	struct iova_dom *iova_dom = NULL;
	struct iommu_domain *domain = NULL;
	struct hisi_domain *hisi_domain = NULL;

	int ret;

	if (!dev || !dmabuf || !iova) {
		pr_err("input err! dev %pK, dmabuf %pK, iova 0x%lx\n", dev,
		       dmabuf, iova);
		return -EINVAL;
	}

	if (!is_ion_dma_buf(dmabuf)) {
		pr_err("dmabuf is not ion buffer\n");
		return -EINVAL;
	}

#ifdef CONFIG_HISI_IOMMU_BYPASS
	return 0;
#endif

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		pr_err("dev(%s) has no iommu domain!\n", dev_name(dev));
		return -EINVAL;
	}

	hisi_domain = to_hisi_domain(domain);

	spin_lock(&hisi_domain->iova_lock);
	iova_dom = hisi_iova_dom_get(hisi_domain, dmabuf);
	if (!iova_dom) {
		spin_unlock(&hisi_domain->iova_lock);
		pr_err("dev(%s) unmap buf no map data!\n", dev_name(dev));
		return -EINVAL;
	}

	if (!atomic64_dec_and_test(&iova_dom->ref)) {
		spin_unlock(&hisi_domain->iova_lock);
		return 0;
	}

	if (iova_dom->iova != iova) {
		spin_unlock(&hisi_domain->iova_lock);
		pr_err("dev(%s) input invalid iova:0x%lx! actual iova 0x%lx\n",
		       dev_name(dev), iova, iova_dom->iova);
		return -EINVAL;
	}

	rb_erase(&iova_dom->node, &hisi_domain->iova_root);
	spin_unlock(&hisi_domain->iova_lock);

	buffer = dmabuf->priv;
	table = buffer->sg_table;

	ret = do_iommu_unmap_sg(hisi_domain, table->sgl, iova);
	kfree(iova_dom);

	return ret;
}
EXPORT_SYMBOL(hisi_iommu_unmap_dmabuf);

/**
 * hisi_iommu_map() - Map ION buffer's dmabuf to iova
 * @dev: master's device struct
 * @paddr: physical address which want to map
 * @size: physical address size which want to map
 * @prot: iommu map prot (eg: IOMMU_READ/IOMMU_WRITE/IOMMU_CACHE etc..)
 *
 * When map success return iova, otherwise return 0.
 * This function is called master dev's driver. The master's device tree
 * must quote master's smmu device tree.
 * Attention: this func don't work with iova refs
 */
unsigned long hisi_iommu_map(struct device *dev, phys_addr_t paddr, size_t size,
				int prot)
{
	struct hisi_domain *hisi_domain = NULL;
	struct iommu_domain *domain = NULL;
	unsigned long iova;
	unsigned long iova_align;
	int ret;

	if (!dev || !paddr) {
		pr_err("input Err! Dev %pK\n", dev);
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

	hisi_domain = to_hisi_domain(domain);
	iova_align = hisi_domain->domain_data->iova_align;

	iova = hisi_iommu_alloc_iova(hisi_domain->iova_pool, size, iova_align);
	if (!iova) {
		pr_err("Dev(%s) alloc iova failed! size 0x%zx\n", dev_name(dev),
		       size);
		return 0;
	}

	ret = iommu_map(domain, iova, paddr, size, prot);
	if (ret) {
		pr_err("map fail! address 0x%llx,  size 0x%zx\n", paddr, size);
		hisi_iommu_free_iova(hisi_domain->iova_pool, iova, size);
	}

	return iova;
}
EXPORT_SYMBOL(hisi_iommu_map);

/**
 * hisi_iommu_unmap() - Map ION buffer's dmabuf to iova
 * @dev: master's device struct
 * @iova: iova which get by hisi_iommu_map()
 * @size: physical address size when called by hisi_iommu_map()
 *
 * When unmap success return 0, otherwise return ERRNO.
 * This function is called master dev's driver. The master's device tree
 * must quote master's smmu device tree.
 * Attention: this func don't work with iova refs
 */
int hisi_iommu_unmap(struct device *dev, unsigned long iova, size_t size)
{
	struct hisi_domain *hisi_domain = NULL;
	struct iommu_domain *domain = NULL;
	size_t unmap_size;

	if (!dev || !iova) {
		pr_err("input err! dev %pK, iova 0x%lx\n", dev, iova);
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

	hisi_domain = to_hisi_domain(domain);

	unmap_size = iommu_unmap(domain, iova, size);
	if (unmap_size != size) {
		pr_err("unmap fail! size 0x%lx, unmap_size 0x%zx\n", size,
		       unmap_size);
		return -EINVAL;
	}

	hisi_iommu_free_iova(hisi_domain->iova_pool, iova, size);

	return 0;
}
EXPORT_SYMBOL(hisi_iommu_unmap);

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

int hisi_iommu_idle_display_map(struct device *dev, u32 policy_id,
				phys_addr_t paddr, size_t allsize,
				size_t l3size, size_t lbsize)
{
	struct hisi_domain *hisi_domain = NULL;
	struct iommu_domain *domain = NULL;
	unsigned long iova, ret_va;
	size_t map_size = 0;
	int prot, ret;

	if (!dev) {
		pr_err("input err! dev %pK\n", dev);
		return -EINVAL;
	}

	if (!is_size_valid(allsize, l3size, lbsize))
		return 0;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		pr_err("Dev(%s) has no iommu domain!\n", dev_name(dev));
		return 0;
	}

	hisi_domain = to_hisi_domain(domain);

	iova = hisi_iommu_alloc_iova(hisi_domain->iova_pool, allsize,
		hisi_domain->domain_data->iova_align);
	if (!iova) {
		pr_err("Dev(%s) alloc iova failed! size 0x%zx\n", dev_name(dev),
		       allsize);
		return 0;
	}

	ret_va = iova;
	/**
	 * map l3 fisrt
	 */
	prot = IOMMU_CACHE | IOMMU_READ | IOMMU_WRITE;
	if (l3size) {
		ret = iommu_map(domain, iova, paddr, l3size, prot);
		if (ret)
			goto free_iova;
	}
	iova += l3size;
	paddr += l3size;
	map_size += l3size;

	/**
	 * map lb second
	 */
	prot = IOMMU_READ | IOMMU_WRITE;
#ifdef CONFIG_HISI_LB
	prot = (u32)prot | (policy_id << IOMMU_PORT_SHIFT);
#endif
	if (lbsize) {
		ret = iommu_map(domain, iova, paddr, lbsize, prot);
		if (ret)
			goto err;
	}
	iova += lbsize;
	paddr += lbsize;
	map_size += lbsize;

	/**
	 * map last
	 */
	prot = IOMMU_READ | IOMMU_WRITE;
	if (allsize - map_size) {
		ret = iommu_map(domain, iova, paddr, (allsize - map_size),
				prot);
		if (ret)
			goto err;
	}

	return ret_va;

err:
	iommu_unmap(domain, ret_va, map_size);
free_iova:
	hisi_iommu_free_iova(hisi_domain->iova_pool, iova, allsize);

	return 0;
}
EXPORT_SYMBOL(hisi_iommu_idle_display_map);
#endif

static int hisi_iommu_sg_node_map(
	struct iommu_domain *domain, unsigned long iova, u32 policy_id,
	struct scatterlist *sgl, struct hisi_dss_size *dss)
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
				pr_err("lb map fail length: 0x%x\n", len);
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
				pr_err("l3 map fail length: 0x%x\n", len);
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
				pr_err("map last fail length: 0x%x\n", len);
				return -EINVAL;
			}
			dss->map_size += len;
			continue;
		}

		/* map fail */
		pr_err("all map fail length:0x%x\n", len);

		return -EINVAL;
	}

	return 0;
}

#ifdef CONFIG_ARM_SMMU_V3
unsigned long hisi_iommu_idle_display_sg_map(struct device *dev, u32 policy_id,
				struct scatterlist *sgl, size_t allsize,
				size_t l3size, size_t lbsize)
{
	struct gen_pool *iova_pool = NULL;
	struct iommu_domain *domain = NULL;
	unsigned long iova, iova_align;
	struct hisi_dss_size dss_size;

	if (!dev) {
		pr_err("input err! dev!\n");
		return 0;
	}

	if (!sgl) {
		pr_err("input err sg is null!\n");
		return 0;
	}

	if (!is_size_valid(allsize, l3size, lbsize)) {
		pr_err("input size is valid!\n");
		return 0;
	}

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		pr_err("Dev has no iommu domain!\n");
		return 0;
	}

	iova_pool = hisi_iommu_get_iova_pool(dev);
	if (!iova_pool) {
		pr_err("iova pool is null\n");
		return 0;
	}
	iova_align = hisi_iommu_get_iova_align(dev);

	iova = hisi_iommu_alloc_iova(iova_pool, allsize, iova_align);
	if (!iova) {
		pr_err("alloc iova failed! size 0x%zx\n", allsize);
		return 0;
	}

	dss_size.all_size = allsize;
	dss_size.l3_size = l3size;
	dss_size.lb_size = lbsize;
	dss_size.map_size = 0;
	if (hisi_iommu_sg_node_map(domain, iova, policy_id, sgl, &dss_size))
		goto err;

	return iova;

err:
	iommu_unmap(domain, iova, dss_size.map_size);
	hisi_iommu_free_iova(iova_pool, iova, allsize);

	return 0;
}
EXPORT_SYMBOL(hisi_iommu_idle_display_sg_map);
#else
unsigned long hisi_iommu_idle_display_sg_map(struct device *dev, u32 policy_id,
				struct scatterlist *sgl, size_t allsize,
				size_t l3size, size_t lbsize)
{
	struct hisi_domain *hisi_domain = NULL;
	struct iommu_domain *domain = NULL;
	unsigned long iova;
	struct hisi_dss_size dss_size;

	if (!dev) {
		pr_err("input err! dev!\n");
		return 0;
	}

	if (!sgl) {
		pr_err("input err sg is null!\n");
		return 0;
	}

	if (!is_size_valid(allsize, l3size, lbsize)) {
		pr_err("input size is valid!\n");
		return 0;
	}

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		pr_err("Dev has no iommu domain!\n");
		return 0;
	}

	hisi_domain = to_hisi_domain(domain);

	iova = hisi_iommu_alloc_iova(hisi_domain->iova_pool, allsize,
		hisi_domain->domain_data->iova_align);
	if (!iova) {
		pr_err("alloc iova failed! size 0x%zx\n", allsize);
		return 0;
	}

	dss_size.all_size = allsize;
	dss_size.l3_size = l3size;
	dss_size.lb_size = lbsize;
	dss_size.map_size = 0;
	if (hisi_iommu_sg_node_map(domain, iova, policy_id, sgl, &dss_size))
		goto err;

	return iova;

err:
	iommu_unmap(domain, iova, dss_size.map_size);
	hisi_iommu_free_iova(hisi_domain->iova_pool, iova, allsize);

	return 0;
}
EXPORT_SYMBOL(hisi_iommu_idle_display_sg_map);
#endif

#ifdef CONFIG_HISI_IOMMU_LAST_PAGE
static void hisi_iommu_sg_table_add_padding_sg(
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

struct sg_table *hisi_iommu_scatterlist_add_padding(
	struct scatterlist *orig_sgl, unsigned long padding_len)
{
	int ret = 0;
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

	hisi_iommu_sg_table_add_padding_sg(table, orig_sgl, orig_nents,
		padding_sg, padding_nents);

	return table;
free_table:
	kfree(table);
	table = NULL;
out:
	return table;
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
	struct ion_buffer *buffer = NULL;
	struct sg_table *table = NULL;
	struct sg_table *padding_table = NULL;
	unsigned long iova;

	if (!dev || !dmabuf || !out_size) {
		pr_err("input err! dev %pK, dmabuf %pK\n", dev, dmabuf);
		return 0;
	}

	if (!is_ion_dma_buf(dmabuf)) {
		dev_err(dev, "dmabuf is not ion buffer\n");
		return 0;
	}

	buffer = dmabuf->priv;
	table = buffer->sg_table;

	padding_table = hisi_iommu_scatterlist_add_padding(table->sgl,
		padding_len);
	if (!padding_table) {
		dev_err(dev, "padding_table is null\n");
		return 0;
	}

	buffer->sg_table = padding_table;

	iova = hisi_iommu_map_dmabuf(dev, dmabuf, prot, out_size);

	buffer->sg_table = table;
	sg_free_table(padding_table);
	kfree(padding_table);

	return iova;
}
EXPORT_SYMBOL(hisi_iommu_map_padding_dmabuf);

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
	struct ion_buffer *buffer = NULL;
	struct sg_table *table = NULL;
	struct sg_table *padding_table = NULL;
	int ret;

	if (!dev || !dmabuf || !iova) {
		pr_err("input err! dev %pK, dmabuf %pK, iova 0x%lx\n",
			dev, dmabuf, iova);
		return -EINVAL;
	}

	if (!is_ion_dma_buf(dmabuf)) {
		dev_err(dev, "dmabuf is not ion buffer\n");
		return -EINVAL;
	}

	buffer = dmabuf->priv;
	table = buffer->sg_table;
	padding_table = hisi_iommu_scatterlist_add_padding(table->sgl,
		padding_len);
	if (!padding_table) {
		dev_err(dev, "padding_table is null\n");
		return -EINVAL;
	}

	buffer->sg_table = padding_table;
	ret = hisi_iommu_unmap_dmabuf(dev, dmabuf, iova);
	buffer->sg_table = table;

	sg_free_table(padding_table);
	kfree(padding_table);
	return ret;
}
EXPORT_SYMBOL(hisi_iommu_unmap_padding_dmabuf);
#endif
