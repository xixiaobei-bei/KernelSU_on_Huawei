
#include "vfmw.h"
#include "memory.h"

#include <linux/math64.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/file.h>
#include <linux/major.h>
#include <linux/syscalls.h>
#include <linux/dma-mapping.h>
#include <linux/dma-iommu.h>
#include <linux/dma-buf.h>
#include <linux/iommu.h>
#include <linux/hisi-iommu.h>

#include "omxvdec.h"
#include "platform.h"

/* lint -e774 */
/* MACRO */
#define MAX_ION_MEM_NODE  100
#define CLIENT_BUF_NAME   "CLIENT"

/* STRUCT */
typedef struct {
	hi_u32 phys_addr;
	hi_void *virt_addr;
	hi_u32 size;
	struct ion_handle *handle;
} ion_mem_desc_s;

/*********************************** VARS *************************************/
struct  mutex g_mem_mutex;

/******************************** LOCAL FUNC **********************************/

/* func: memory module probe entry */
hi_s32 vdec_mem_probe(hi_void)
{
	vdec_init_mutex(&g_mem_mutex);
	return HI_SUCCESS;
}

/* func: map dma buffer */
hi_s32 vdec_mem_get_map_info(hi_s32 share_fd, mem_buffer_s *ps_m_buf)
{
	hi_s32 rc;
	hi_u64 iova_addr;
	unsigned long phy_size;
	struct dma_buf *dmabuf = NULL;
	hi_s32 ret         = HI_SUCCESS;
	hi_void *virt_addr = HI_NULL;
	omx_vdec_entry vdec_entry;

	if ((ps_m_buf == HI_NULL) || (share_fd < 0)) {
		dprint(PRN_ERROR, "%s: invalid Param(share_fd:%d)\n", __func__, share_fd);
		return HI_FAILURE;
	}

	vdec_mutex_lock(&g_mem_mutex);

	vdec_entry = omx_vdec_get_entry();

	dmabuf = dma_buf_get(share_fd);
	if (IS_ERR(dmabuf)) {
		dprint(PRN_FATAL, "%s, dma_buf_get failed", __func__);
		vdec_mutex_unlock(&g_mem_mutex);
		return HI_FAILURE;

	}

	iova_addr =
		hisi_iommu_map_dmabuf(vdec_entry.device, dmabuf, 0, &phy_size);
	if (!iova_addr) {
		dprint(PRN_FATAL, "%s, hisi_iommu_map_dmabuf failed", __func__);
		goto exit;
	}

	if (ps_m_buf->is_map_virtual == 1) {
		rc = dma_buf_begin_cpu_access(dmabuf, DMA_FROM_DEVICE);
		if (rc < 0) {
			dprint(PRN_ERROR, "%s:dma_buf_begin_cpu_access failed\n", __func__);
			goto error_1;
		}

		virt_addr = dma_buf_kmap(dmabuf, 0);
		if (virt_addr == NULL) {
			dprint(PRN_FATAL, "%s, kmap failed\n", __func__);
			goto error_2;
		}

		ps_m_buf->p_start_vir_addr   = virt_addr;
	}

	ps_m_buf->start_phy_addr        = iova_addr;
	ps_m_buf->size                  = phy_size;
	ps_m_buf->share_fd              = share_fd;

	dma_buf_put(dmabuf);

	vdec_mutex_unlock(&g_mem_mutex);
	return ret;

error_2:
	rc = dma_buf_end_cpu_access(dmabuf, DMA_TO_DEVICE);
	if (rc < 0)
		dprint(PRN_ERROR, "%s: dma buf end cpu access failed\n", __func__);
error_1:
	ret = hisi_iommu_unmap_dmabuf(vdec_entry.device, dmabuf,  phy_size);
	if (ret != 0)
		dprint(PRN_ERROR, "%s: hisi iommu unmap dmabuf failed\n", __func__);
exit:
	dma_buf_put(dmabuf);
	vdec_mutex_unlock(&g_mem_mutex);
	return HI_FAILURE;
}

/* func: unmap dma buffer */
hi_s32 vdec_mem_put_map_info(mem_buffer_s *ps_m_buf)
{
	hi_s32 rc;
	struct dma_buf *dmabuf = NULL;
	hi_s32 ret;
	omx_vdec_entry vdec_entry;

	if (ps_m_buf == HI_NULL) {
		dprint(PRN_ERROR, "%s: ps_m_buf is NULL\n", __func__);
		return HI_FAILURE;
	}

	vdec_mutex_lock(&g_mem_mutex);

	vdec_entry = omx_vdec_get_entry();

	dmabuf = dma_buf_get(ps_m_buf->share_fd);
	if (IS_ERR(dmabuf)) {
		dprint(PRN_FATAL, "%s, dma_buf_get failed share fd %d\n", __func__, ps_m_buf->share_fd);
		vdec_mutex_unlock(&g_mem_mutex);
		return HI_FAILURE;
	}

	ret = hisi_iommu_unmap_dmabuf(vdec_entry.device,
		dmabuf,  ps_m_buf->start_phy_addr);
	if (ret != 0) {
		dprint(PRN_ERROR, "%s: hisi iommu unmap dmabuf failed\n", __func__);
		ret = HI_FAILURE;
		goto exit;
	}

	if (ps_m_buf->is_map_virtual == 1) {
		dma_buf_kunmap(dmabuf, 0, NULL);

		rc = dma_buf_end_cpu_access(dmabuf, DMA_TO_DEVICE);
		if (rc != 0) {
			dprint(PRN_ERROR, "%s: end cpu access failed\n", __func__);
			ret = HI_FAILURE;
		}
	}

exit:
	dma_buf_put(dmabuf);
	vdec_mutex_unlock(&g_mem_mutex);
	return ret;
}

hi_s32 vdec_mem_iommu_map(hi_s32 share_fd, UADDR *iova)
{

	hi_u64 iova_addr;
	unsigned long phy_size;
	struct dma_buf *dmabuf = NULL;
	omx_vdec_entry vdec_entry;

	if (share_fd < 0) {
		dprint(PRN_ERROR, "%s, share fd is invalid \n", __func__);
		return HI_FAILURE;
	}

	vdec_mutex_lock(&g_mem_mutex);

	vdec_entry = omx_vdec_get_entry();

	dmabuf = dma_buf_get(share_fd);
	if (IS_ERR(dmabuf)) {
		dprint(PRN_FATAL, "%s, dma_buf_get failed", __func__);
		vdec_mutex_unlock(&g_mem_mutex);
		return HI_FAILURE;
	}

	iova_addr = hisi_iommu_map_dmabuf(vdec_entry.device,
		dmabuf, 0, &phy_size);
	if (!iova_addr) {
		dprint(PRN_FATAL, "%s, hisi_iommu_map_dmabuf failed", __func__);
		dma_buf_put(dmabuf);
		*iova = 0;
		vdec_mutex_unlock(&g_mem_mutex);
		return HI_FAILURE;
	}

	*iova = iova_addr;

	dma_buf_put(dmabuf);
	vdec_mutex_unlock(&g_mem_mutex);
	return HI_SUCCESS;

}

hi_s32 vdec_mem_iommu_unmap(hi_s32 share_fd, UADDR iova)
{
	struct dma_buf *dmabuf = NULL;
	hi_s32 ret;
	omx_vdec_entry vdec_entry;

	if (share_fd < 0) {
		dprint(PRN_ERROR, "%s, share fd is invalid\n", __func__);
		return HI_FAILURE;
	}

	if (!iova) {
		dprint(PRN_ERROR, "%s, iova is invalid\n", __func__);
		return HI_FAILURE;
	}

	vdec_mutex_lock(&g_mem_mutex);

	vdec_entry = omx_vdec_get_entry();

	dmabuf = dma_buf_get(share_fd);
	if (IS_ERR(dmabuf)) {
		dprint(PRN_FATAL, "%s, dma buf get failed\n", __func__);
		vdec_mutex_unlock(&g_mem_mutex);
		return HI_FAILURE;

	}

	ret = hisi_iommu_unmap_dmabuf(vdec_entry.device, dmabuf, iova);
	if (ret != 0) {
		dprint(PRN_ERROR, "%s:hisi iommu unmap dmabuf failed\n", __func__);
		ret = HI_FAILURE;
	}

	dma_buf_put(dmabuf);

	vdec_mutex_unlock(&g_mem_mutex);
	return ret;
}

