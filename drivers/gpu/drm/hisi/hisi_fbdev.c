/*
 * Copyright (C) 2013 Red Hat
 * Author: Rob Clark <robdclark@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*lint -e421 -e574 -e620*/

#include <drm/drmP.h>
#include <drm_crtc.h>
#include <drm_fb_helper.h>
#include <drm_crtc_helper.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
#include <linux/ion.h>
#endif
#include <linux/hisi/hisi_ion.h>
#include <securec.h>

#include "hisi_drm_drv.h"
#include "hisi_dpe.h"
#include "hisi_drm_dpe_utils.h"
#include "hisi_dss_ion.h"
#include "hisi_dss_iommu.h"
#include "hisi_drm_gem.h"
#include "hisi_drm.h"

#define FBDEV_BUFFER_NUM 3

#define FBIOGET_DMABUF _IOR('F', 0x21, struct fb_dmabuf_export)

#define HISIFB_IOCTL_MAGIC 'M'
#define HISI_DRM_ONLINE_PLAY _IOW(HISIFB_IOCTL_MAGIC, 0x21, struct drm_dss_layer)
#define HISIFB_GRALLOC_MAP_IOVA _IOW(HISIFB_IOCTL_MAGIC, 807, struct iova_info)
#define HISIFB_GRALLOC_UNMAP_IOVA _IOW(HISIFB_IOCTL_MAGIC, 808, struct iova_info)

#define HISI_FB_ION_CLIENT_NAME	"hisi_fb_ion"

struct fb_dmabuf_export {
	__u32 fd;
	__u32 flags;
};

/*
 * fbdev funcs, to implement legacy fbdev interface on top of drm driver
 */
struct fbdev_mmap_info {
	unsigned long addr;
	unsigned long offset;
	unsigned long size;
	unsigned long remainder;
	unsigned long len;
};

static int mmap_info_init(struct fbdev_mmap_info *mmap_info, struct fb_info *info, struct vm_area_struct *vma)
{
	mmap_info->addr = vma->vm_start;
	mmap_info->offset = vma->vm_pgoff * PAGE_SIZE;
	mmap_info->size = vma->vm_end - vma->vm_start;

	if (mmap_info->size > info->fix.smem_len) {
		HISI_DRM_ERR("size=%lu is out of range(%u)!",
			mmap_info->size, info->fix.smem_len);
		return -EFAULT;
	}
	return 0;
}

static int pfn_range_remap(struct fbdev_mmap_info *mmap_info,
	struct sg_table *table, struct vm_area_struct *vma)
{
	int i = 0;
	struct page *page = NULL;
	int ret;
	struct scatterlist *sg = NULL;

	for_each_sg(table->sgl, sg, table->nents, i) {
		page = sg_page(sg);
		mmap_info->remainder = vma->vm_end - mmap_info->addr;
		mmap_info->len = sg->length;

		if (mmap_info->offset >= sg->length) {
			mmap_info->offset -= sg->length;
			continue;
		} else if (mmap_info->offset) {
			page += mmap_info->offset / PAGE_SIZE;
			mmap_info->len = sg->length - mmap_info->offset;
			mmap_info->offset = 0;
		}

		mmap_info->len = min(mmap_info->len, mmap_info->remainder);
		ret = remap_pfn_range(vma, mmap_info->addr, page_to_pfn(page),
			mmap_info->len, vma->vm_page_prot);
		if (ret != 0)
			HISI_DRM_ERR("failed to remap_pfn_range! ret=%d!", ret);

		mmap_info->addr += mmap_info->len;
		if (mmap_info->addr >= vma->vm_end)
			return 0;
	}
	return 1;
}

static int hisi_fbdev_mmap(struct fb_info *info, struct vm_area_struct *vma)
{
	struct drm_fb_helper *helper = NULL;
	struct hisi_fbdev *fbdev = NULL;
	struct sg_table *table = NULL;
	struct fbdev_mmap_info mmap_info;
	int ret;

	if (info == NULL) {
		HISI_DRM_ERR("info is nullptr!");
		return -EINVAL;
	}

	if (vma == NULL) {
		HISI_DRM_ERR("vma is nullptr!");
		return -EINVAL;
	}

	helper = (struct drm_fb_helper *)info->par;
	if (helper == NULL) {
		HISI_DRM_ERR("helper is NULL!\n");
		return -EINVAL;
	}

	fbdev = to_hisi_fbdev(helper);
	if (fbdev == NULL) {
		HISI_DRM_ERR("fbdev is NULL!\n");
		return -EINVAL;
	}

	table = fbdev->sgt;
	if (table == NULL) {
		HISI_DRM_ERR("table is NULL!");
		return -EFAULT;
	}

	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

	ret = mmap_info_init(&mmap_info, info, vma);
	if (ret)
		return ret;

	ret = pfn_range_remap(&mmap_info, table, vma);
	if (!ret)
		return ret;

	return 0;
}

static int hisi_dss_online_compose(struct fb_info *info, void __user *argp)
{
	int ret;
	struct drm_dss_layer layer;

	ret = copy_from_user(&layer, argp, sizeof(struct drm_dss_layer));
	if (ret) {
		HISI_DRM_ERR("failed to copy_from_user! ret=%d!", ret);
		return -EINVAL;
	}

	return ret;
}

static int hisi_fb_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
{
	int ret = -ENOTSUPP;
	void __user *argp = (void __user *)arg;
	struct drm_fb_helper *fb_helper = NULL;
	struct drm_device *dev = NULL;
	struct hisi_drm_private *drm_priv = NULL;

	if (info == NULL) {
		HISI_DRM_ERR("info is nullptr!");
		return -EINVAL;
	}

	fb_helper = (struct drm_fb_helper *)info->par;
	if (fb_helper == NULL) {
		HISI_DRM_ERR("fb_helper is nullptr!");
		return -EINVAL;
	}

	dev = fb_helper->dev;
	if (dev == NULL) {
		HISI_DRM_ERR("dev is nullptr!");
		return -EINVAL;
	}

	drm_priv = dev->dev_private;
	if (drm_priv == NULL) {
		HISI_DRM_ERR("drm_priv is nullptr!");
		return -EINVAL;
	}

	switch (cmd) {
	case HISI_DRM_ONLINE_PLAY:
		ret = hisi_dss_online_compose(info, argp);
		break;
	case FBIO_WAITFORVSYNC:
		ret = 0;
		break;

	case HISIFB_GRALLOC_MAP_IOVA:
		if (drm_priv->iommu_dev == NULL) {
			HISI_DRM_ERR("not supprot iommu!");
			return -EINVAL;
		}

		ret = hisi_dss_buffer_map_iova(drm_priv->iommu_dev, info, argp);
		break;
	case HISIFB_GRALLOC_UNMAP_IOVA:
		ret = hisi_dss_buffer_unmap_iova(info, argp);
		break;
	default:
		break;
	}

	if (ret == -ENOTSUPP)
		HISI_DRM_ERR("unsupported ioctl (%x)!", cmd);

	return ret;
}

static struct fb_ops hisi_fb_ops = {
	.owner = THIS_MODULE,

	/* Note: to properly handle manual update displays, we wrap the
	 * basic fbdev ops which write to the framebuffer
	 */
	.fb_read = drm_fb_helper_sys_read,
	.fb_write = drm_fb_helper_sys_write,
	.fb_fillrect = drm_fb_helper_sys_fillrect,
	.fb_copyarea = drm_fb_helper_sys_copyarea,
	.fb_imageblit = drm_fb_helper_sys_imageblit,
	.fb_mmap = hisi_fbdev_mmap,

	.fb_check_var = drm_fb_helper_check_var,
	.fb_set_par = drm_fb_helper_set_par,
	.fb_pan_display = drm_fb_helper_pan_display,
	.fb_blank = drm_fb_helper_blank,
	.fb_setcmap = drm_fb_helper_setcmap,

	.fb_ioctl = NULL,
	.fb_compat_ioctl = hisi_fb_ioctl,
};

static int is_param_valid(struct drm_fb_helper *helper,
			struct drm_fb_helper_surface_size *sizes,
			struct drm_device **dev, struct hisi_fbdev **fbdev)
{
	struct hisi_drm_private *drm_priv = NULL;

	if (!helper) {
		HISI_DRM_ERR("helper is nullptr!");
		return -EINVAL;
	}

	if (!sizes) {
		HISI_DRM_ERR("sizes is nullptr!");
		return -EINVAL;
	}

	*dev = helper->dev;
	if (!(*dev)) {
		HISI_DRM_ERR("dev is nullptr!");
		return -EINVAL;
	}

	drm_priv = (*dev)->dev_private;
	if (!drm_priv) {
		HISI_DRM_ERR("drm_priv is nullptr!");
		return -EINVAL;
	}
	*fbdev = to_hisi_fbdev(helper);
	if (!(*fbdev)) {
		HISI_DRM_ERR("fbdev is nullptr!");
		return -EINVAL;
	}
	return 0;
}

static void get_mode_cmd(struct drm_fb_helper_surface_size *sizes, struct drm_mode_fb_cmd2 *mode_cmd)
{
	unsigned int bytes_per_pixel;

	HISI_DRM_INFO("fbdev: %dx%d@%d %dx%d\n",
		sizes->surface_width, sizes->surface_height, sizes->surface_bpp,
		sizes->fb_width, sizes->fb_height);


	mode_cmd->pixel_format = drm_mode_legacy_fb_format(sizes->surface_bpp,
		sizes->surface_depth);

	mode_cmd->width = sizes->surface_width;
	mode_cmd->height = sizes->surface_height * FBDEV_BUFFER_NUM;

	bytes_per_pixel = DIV_ROUND_UP(sizes->surface_bpp, 8);
	mode_cmd->pitches[0] = sizes->surface_width * bytes_per_pixel;

}

static int set_fbi(struct fb_info **fbi, int size,
	struct hisi_fbdev *fbdev, struct drm_fb_helper *helper,
		struct drm_framebuffer *fb, struct drm_device *dev)
{
	int ret;
	struct hisi_drm_gem *hisi_gem = NULL;

	*fbi = drm_fb_helper_alloc_fbi(helper);
	if (IS_ERR(*fbi)) {
		HISI_DRM_ERR("failed to allocate fb info!");
		ret = PTR_ERR(*fbi);
		return ret;
	}

	(*fbi)->fix.smem_len = size;
	hisi_gem = hisi_drm_gem_create(
		dev, HISI_BO_ALLOC_WITH_MAP_IOMMU | HISI_BO_PAGE_POOL_HEAP,
		size);
	if (IS_ERR_OR_NULL(hisi_gem)) {
		ret = PTR_ERR(hisi_gem);
		HISI_DRM_ERR("failed to create drm gen! ret=%d!", ret);
		return ret;
	}

	(*fbi)->screen_base = hisifb_iommu_map_kernel(hisi_gem->sgt, size);
	if (!(*fbi)->screen_base) {
		HISI_DRM_ERR("failed to iommu map!");
		hisi_drm_gem_destroy(hisi_gem);
		ret = -ENOMEM;
		return ret;
	}

	ret = memset_s((*fbi)->screen_base, size, 0, size);
	if (ret != EOK) {
		HISI_DRM_ERR("fail to set screen base!");
		return -ENOMEM;
	}
	(*fbi)->fix.smem_start = hisi_gem->iova;
	(*fbi)->screen_size = size;
	fb->obj[0] = &hisi_gem->base;

	fbdev->fb = fb;
	helper->fb = fb;

	(*fbi)->par = helper;
	(*fbi)->flags = FBINFO_DEFAULT;
	(*fbi)->fbops = &hisi_fb_ops;

	strcpy_s((*fbi)->fix.id, sizeof((*fbi)->fix.id), "dss");
	return 0;
}

static void set_fbdev(struct hisi_fbdev *fbdev, struct fb_info *fbi)
{
	fbdev->screen_base = fbi->screen_base;
	fbdev->screen_size = fbi->screen_size;
	fbdev->smem_start = fbi->fix.smem_start;
	fbdev->screen_size = fbi->fix.smem_len;
}

static void fb_deinit(int ret, struct drm_framebuffer *fb)
{
	if (ret) {
		if (fb) {
			drm_framebuffer_unregister_private(fb);
			drm_framebuffer_remove(fb);
		}
	}
}

static int hisi_fbdev_create(struct drm_fb_helper *helper,
	struct drm_fb_helper_surface_size *sizes)
{
	struct hisi_fbdev *fbdev = NULL;
	struct drm_device *dev = NULL;
	struct drm_framebuffer *fb = NULL;
	struct fb_info *fbi = NULL;
	struct drm_mode_fb_cmd2 mode_cmd = { 0 };
	int ret;
	int size;

	HISI_DRM_INFO("+");

	ret = is_param_valid(helper, sizes, &dev, &fbdev);
	if (ret)
		return ret;
	get_mode_cmd(sizes, &mode_cmd);
	/* allocate backing bo */
	size = mode_cmd.pitches[0] * mode_cmd.height;
	HISI_DRM_INFO("allocating %d bytes for fb %d, pitch=%d, height=%d\n",
		size, dev->primary->index, mode_cmd.pitches[0], mode_cmd.height);

	fb = hisi_framebuffer_init(dev, &mode_cmd);
	if (IS_ERR(fb)) {
		HISI_DRM_ERR("failed to allocate fb!");
		/* note: if fb creation failed, we can't rely on fb destroy to unref the bo: */
		ret = PTR_ERR(fb);
		goto fail;
	}
	mutex_lock(&dev->struct_mutex);

	fbdev->screen_base = NULL;
	fbdev->smem_start = 0;
	fbdev->screen_size = 0;

	ret = set_fbi(&fbi, size, fbdev, helper, fb, dev);
	if (ret)
		goto fail_unlock;

	drm_fb_helper_fill_fix(fbi, fb->pitches[0], fb->format->depth);
	drm_fb_helper_fill_var(fbi, helper, sizes->fb_width, sizes->fb_height);

	set_fbdev(fbdev, fbi);
	dev->mode_config.fb_base = fbdev->smem_start;

	HISI_DRM_INFO("[%dx%d], [%dx%d]", fbi->var.xres, fbi->var.yres,
		fbdev->fb->width, fbdev->fb->height);

	mutex_unlock(&dev->struct_mutex);

	HISI_DRM_INFO("-");

	return 0;

fail_unlock:
	mutex_unlock(&dev->struct_mutex);

fail:
	fb_deinit(ret, fb);
	return ret;
}


static const struct drm_fb_helper_funcs hisi_fb_helper_funcs = {
	.fb_probe = hisi_fbdev_create,
};

struct drm_fb_helper *hisi_drm_fbdev_init(struct drm_device *dev)
{
	int ret;
	struct hisi_fbdev *fbdev = NULL;
	struct drm_fb_helper *helper = NULL;

	if (!dev) {
		HISI_DRM_ERR("dev is NULL!");
		return NULL;
	}

	fbdev = kzalloc(sizeof(*fbdev), GFP_KERNEL);
	if (!fbdev) {
		HISI_DRM_ERR("failed to alloc fbdev!");
		return NULL;
	}

	helper = &fbdev->fb_helper;

	drm_fb_helper_prepare(dev, helper, &hisi_fb_helper_funcs);

	HISI_DRM_INFO("num_crtc=%d, num_connector=%d",
		dev->mode_config.num_crtc, dev->mode_config.num_connector);

	ret = drm_fb_helper_init(dev, helper, 1);
	if (ret) {
		HISI_DRM_ERR("failed to init fbdev! ret=%d!", ret);
		goto err_init;
	}

	ret = drm_fb_helper_single_add_all_connectors(helper);
	if (ret) {
		HISI_DRM_ERR("fail to add all connectors! ret=%d!", ret);
		goto err_add_all_connectors;
	}

	ret = drm_fb_helper_initial_config(helper, 32);
	if (ret) {
		HISI_DRM_ERR("failed to initial config! ret=%d!", ret);
		goto err_initial_config;
	}
	HISI_DRM_INFO("-\n");
	return helper;

err_initial_config:

err_add_all_connectors:
	if (helper)
		drm_fb_helper_fini(helper);

err_init:
	kfree(fbdev);
	fbdev = NULL;

	return NULL;
}

void hisi_drm_fbdev_fini(struct drm_fb_helper *helper)
{
	struct hisi_fbdev *fbdev = NULL;

	if (!helper) {
		HISI_DRM_ERR("helper is NULL!");
		return;
	}

	drm_fb_helper_unregister_fbi(helper);
	drm_fb_helper_fini(helper);

	fbdev = to_hisi_fbdev(helper);
	if (fbdev) {
		/* this will free the backing object */
		if (fbdev->fb) {
			drm_framebuffer_unregister_private(fbdev->fb);
			drm_framebuffer_remove(fbdev->fb);
		}

		kfree(fbdev);
		fbdev = NULL;
	}

	helper = NULL;
}

/*lint +e421 +e574 +e620*/
