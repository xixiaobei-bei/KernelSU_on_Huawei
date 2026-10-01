

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
#include "memory.h"
#include "vfmw.h"

/*lint -e774*/
/********************************** MACRO *************************************/
#define MAX_ION_MEM_NODE  (100)
#define CLIENT_BUF_NAME   "CLIENT"
extern OMXVDEC_ENTRY g_OmxVdecEntry;

/********************************* STRUCT *************************************/
typedef struct {
	HI_U32 phys_addr;
	HI_VOID *virt_addr;
	HI_U32 size;
	struct ion_handle *handle;
} ION_MEM_DESC_S;

/*********************************** VARS *************************************/
struct  mutex gMemMutex;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
struct ion_client *gIONClient = HI_NULL;
#endif
ION_MEM_DESC_S gIONMemNode[MAX_ION_MEM_NODE];

/******************************** LOCAL FUNC **********************************/
/*----------------------------------------
    func: normal memory free interface
 ----------------------------------------*/
static HI_VOID VDEC_MEM_FreeNormal_MMU(ION_MEM_DESC_S * pMemNode)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
#ifdef HIVDEC_SMMU_SUPPORT
	/* ion_unmap iommu */
	ion_unmap_iommu(gIONClient, pMemNode->handle);
#endif
	if (pMemNode->virt_addr != HI_NULL) {
		ion_unmap_kernel(gIONClient, pMemNode->handle);
	}
	ion_free(gIONClient, pMemNode->handle);
#endif
	return;
}

/*----------------------------------------
    func: memory module probe entry
 ----------------------------------------*/
HI_S32 VDEC_MEM_Probe(HI_VOID)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	gIONClient = HI_NULL;
#endif
	memset(gIONMemNode, 0, sizeof(gIONMemNode)); /* unsafe_function_ignore: memset */

	VDEC_INIT_MUTEX(&gMemMutex);

	return HI_SUCCESS;
}

/*----------------------------------------
    func: initialize memory module
 ----------------------------------------*/
HI_S32 VDEC_MEM_Init(HI_VOID)
{
	HI_S32 ret = HI_SUCCESS;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	VDEC_MUTEX_LOCK(&gMemMutex);

	gIONClient = (struct ion_client *)hisi_ion_client_create("hi_vcodec_ion");
	if (IS_ERR_OR_NULL(gIONClient)) {
		dprint(PRN_ERROR, "%s: ion client create failed\n", __func__);
		gIONClient = HI_NULL;
		ret = HI_FAILURE;
	}

	VDEC_MUTEX_UNLOCK(&gMemMutex);
#endif

	return ret;
}

/*----------------------------------------
    func: deinitialize memory module
 ----------------------------------------*/
HI_S32 VDEC_MEM_Exit(HI_VOID)
{
	HI_S32 i;

	VDEC_MUTEX_LOCK(&gMemMutex);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	if (gIONClient == HI_NULL) {
		dprint(PRN_ERROR, "%s : gIONClient is NULL\n", __func__);
		VDEC_MUTEX_UNLOCK(&gMemMutex);
		return HI_FAILURE;
	}

	for (i = 0; i < MAX_ION_MEM_NODE; i++) {
		if ((gIONMemNode[i].phys_addr != 0) && (gIONMemNode[i].handle != HI_NULL)) {
			dprint(PRN_ERROR, "%s: ion mem leak, size %d, free now\n", __func__, gIONMemNode[i].size);
			VDEC_MEM_FreeNormal_MMU(&gIONMemNode[i]);
			memset(&gIONMemNode[i], 0, sizeof(ION_MEM_DESC_S)); /* unsafe_function_ignore: memset */
		}
	}

	ion_client_destroy(gIONClient);
	gIONClient = HI_NULL;
#endif

	VDEC_MUTEX_UNLOCK(&gMemMutex);
	return HI_SUCCESS;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
/*----------------------------------------
    func: map ion buffer
 ----------------------------------------*/
HI_S32 VDEC_MEM_MapKernel(HI_S32 share_fd, MEM_BUFFER_S * psMBuf)
{
	HI_S32 i;
#ifndef HIVDEC_SMMU_SUPPORT
	HI_S32 ret;
	size_t len;
#endif
	ion_phys_addr_t phy_addr = 0;
	HI_VOID *virt_addr = HI_NULL;
	struct ion_handle *handle = HI_NULL;
	struct iommu_map_format ion_fmt;
	HI_U32 phy_size;

	if ((psMBuf == HI_NULL) || (share_fd < 0)) {
		dprint(PRN_ERROR, "%s: invalid Param(share_fd:%d)\n", __func__, share_fd);
		return HI_FAILURE;
	}

	VDEC_MUTEX_LOCK(&gMemMutex);

	if (gIONClient == HI_NULL) {
		dprint(PRN_ERROR, "%s: gIONClient is NULL\n", __func__);
		goto err_exit;
	}

	for (i = 0; i < MAX_ION_MEM_NODE; i++) {
		if (0 == gIONMemNode[i].phys_addr && HI_NULL == gIONMemNode[i].handle) {
			break;
		}
	}

	if (i >= MAX_ION_MEM_NODE) {
		dprint(PRN_ERROR, "%s: node is not found\n", __func__);
		goto err_exit;
	}

	handle = ion_import_dma_buf_fd(gIONClient, share_fd);

	if (IS_ERR_OR_NULL(handle)) {
		dprint(PRN_ERROR, "%s: handle is error\n", __func__);
		goto err_exit;
	}

	if (1 == psMBuf->u8IsMapVirtual) {
		virt_addr = ion_map_kernel(gIONClient, handle);
		if (IS_ERR_OR_NULL(virt_addr)) {
			dprint(PRN_ERROR, "%s: virt_addr is error\n", __func__);
			goto err_exit1;
		}
	}
#ifdef HIVDEC_SMMU_SUPPORT
		/* Get iova_start addr */
	memset(&ion_fmt, 0x0, sizeof(ion_fmt)); /* unsafe_function_ignore: memset */
	if (ion_map_iommu(gIONClient, handle, &ion_fmt) != 0) {
		dprint(PRN_ERROR, "%s: iommu map failed\n", __func__);
		goto err_exit2;
	}
	phy_addr = ion_fmt.iova_start;
	phy_size = (HI_U32)ion_fmt.iova_size;
#else
	dprint(PRN_ERROR, "%s: UNSMMU is not supported\n", __func__);
	goto err_exit2;
#endif

	psMBuf->pStartVirAddr = virt_addr;
	psMBuf->startPhyAddr = (UADDR) phy_addr;
	psMBuf->u32Size = phy_size;
	gIONMemNode[i].phys_addr = (UADDR) phy_addr;
	gIONMemNode[i].virt_addr = psMBuf->pStartVirAddr;
	gIONMemNode[i].handle = handle;
	gIONMemNode[i].size = psMBuf->u32Size;

	VDEC_MUTEX_UNLOCK(&gMemMutex);
	return HI_SUCCESS;

err_exit2:
	if (1 == psMBuf->u8IsMapVirtual) {
		ion_unmap_kernel(gIONClient, handle);
	}
err_exit1:
	ion_free(gIONClient, handle);
err_exit:
	VDEC_MUTEX_UNLOCK(&gMemMutex);
	return HI_FAILURE;
}

/*----------------------------------------
    func: unmap ion buffer
 ----------------------------------------*/
HI_S32 VDEC_MEM_UnmapKernel(MEM_BUFFER_S * psMBuf)
{
	HI_S32 i;

	if (psMBuf == HI_NULL) {
		dprint(PRN_ERROR, "%s: psMBuf is NULL\n", __func__);
		return HI_FAILURE;
	}

	VDEC_MUTEX_LOCK(&gMemMutex);

	if (gIONClient == HI_NULL) {
		dprint(PRN_ERROR, "%s: gIONClient is NULL\n", __func__);
		goto err_exit;
	}

	for (i = 0; i < MAX_ION_MEM_NODE; i++) {
		if ((psMBuf->startPhyAddr != 0)
			&& (psMBuf->startPhyAddr == gIONMemNode[i].phys_addr)
			&& (psMBuf->pStartVirAddr == gIONMemNode[i].virt_addr)
			&& (gIONMemNode[i].handle != HI_NULL)) {
			break;
		}
	}

	if (i >= MAX_ION_MEM_NODE) {
		dprint(PRN_ERROR, "%s: node is not found\n", __func__);
		goto err_exit;
	}

#ifdef HIVDEC_SMMU_SUPPORT
	/* ion_unmap iommu */
	ion_unmap_iommu(gIONClient, gIONMemNode[i].handle);
#endif
	if (1 == psMBuf->u8IsMapVirtual) {
		ion_unmap_kernel(gIONClient, gIONMemNode[i].handle);
	}

	ion_free(gIONClient, gIONMemNode[i].handle);

	memset(&gIONMemNode[i], 0, sizeof(ION_MEM_DESC_S)); /* unsafe_function_ignore: memset */

	VDEC_MUTEX_UNLOCK(&gMemMutex);
	return HI_SUCCESS;

err_exit:
	VDEC_MUTEX_UNLOCK(&gMemMutex);
	return HI_FAILURE;
}
#else

/*----------------------------------------
    func: map dma buffer
 ----------------------------------------*/
HI_S32 VDEC_MEM_GetMapInfo(HI_S32 share_fd, MEM_BUFFER_S * psMBuf)
{
	HI_S32 rc;
	HI_U64 iova_addr;
	unsigned long phy_size;
	struct dma_buf *dmabuf;
	HI_S32 ret         = HI_SUCCESS;
	HI_VOID *virt_addr = HI_NULL;

	if ((psMBuf == HI_NULL) || (share_fd < 0)) {
		dprint(PRN_ERROR, "%s: invalid Param(share_fd:%d)\n", __func__, share_fd);
		return HI_FAILURE;
	}

	VDEC_MUTEX_LOCK(&gMemMutex);

	dmabuf = dma_buf_get(share_fd);
	if (IS_ERR(dmabuf)) {
		dprint(PRN_FATAL, "%s, dma_buf_get failed", __func__);
		VDEC_MUTEX_UNLOCK(&gMemMutex);
		return HI_FAILURE;

	}

	iova_addr = hisi_iommu_map_dmabuf(g_OmxVdecEntry.device, dmabuf, 0, &phy_size);
	if (!iova_addr) {
		dprint(PRN_FATAL, "%s, hisi_iommu_map_dmabuf failed", __func__);
		goto exit;
	}

	if (1 == psMBuf->u8IsMapVirtual) {
		rc = dma_buf_begin_cpu_access(dmabuf, DMA_FROM_DEVICE);
		if (rc < 0) {
			dprint(PRN_ERROR, "%s: dma_buf_begin_cpu_access failed \n", __func__);
			goto error_1;
		}

		virt_addr = dma_buf_kmap(dmabuf, 0);
		if (!virt_addr) {
			dprint(PRN_FATAL, "%s, kmap failed \n", __func__);
			goto error_2;
		}

		psMBuf->pStartVirAddr   = virt_addr;
	}

	psMBuf->startPhyAddr        = iova_addr;
	psMBuf->u32Size             = phy_size;
	psMBuf->u32ShareFd          = share_fd;

	dma_buf_put(dmabuf);

	VDEC_MUTEX_UNLOCK(&gMemMutex);
	return ret;

error_2:
	rc = dma_buf_end_cpu_access(dmabuf, DMA_TO_DEVICE);
	if (rc < 0) {
		dprint(PRN_ERROR, "%s: dma buf end cpu access failed \n", __func__);
	}
error_1:
	ret = hisi_iommu_unmap_dmabuf(g_OmxVdecEntry.device, dmabuf,  phy_size);
	if (ret != 0) {
		dprint(PRN_ERROR, "%s: hisi iommu unmap dmabuf failed \n", __func__);
	}
exit:
	dma_buf_put(dmabuf);
	VDEC_MUTEX_UNLOCK(&gMemMutex);
	return HI_FAILURE;
}

/*----------------------------------------
    func: unmap dma buffer
 ----------------------------------------*/
HI_S32 VDEC_MEM_PutMapInfo(MEM_BUFFER_S * psMBuf)
{
	HI_S32 rc;
	struct dma_buf *dmabuf;
	HI_S32 ret = HI_SUCCESS;

	if (psMBuf == HI_NULL) {
		dprint(PRN_ERROR, "%s: psMBuf is NULL\n", __func__);
		return HI_FAILURE;
	}

	VDEC_MUTEX_LOCK(&gMemMutex);

	dmabuf = dma_buf_get(psMBuf->u32ShareFd);
	if (IS_ERR(dmabuf)) {
		dprint(PRN_FATAL, "%s, dma_buf_get failed share fd %d\n", __func__, psMBuf->u32ShareFd);
		VDEC_MUTEX_UNLOCK(&gMemMutex);
		return HI_FAILURE;
	}

	ret = hisi_iommu_unmap_dmabuf(g_OmxVdecEntry.device, dmabuf,  psMBuf->startPhyAddr);
	if (ret != 0) {
		dprint(PRN_ERROR, "%s: hisi iommu unmap dmabuf failed \n", __func__);
		ret = HI_FAILURE;
		goto exit;
	}

	if (1 == psMBuf->u8IsMapVirtual) {
		dma_buf_kunmap(dmabuf, 0, NULL);

		rc = dma_buf_end_cpu_access(dmabuf, DMA_TO_DEVICE);
		if (rc != 0) {
			dprint(PRN_ERROR, "%s: end cpu access failed \n", __func__);
			ret = HI_FAILURE;
		}
	}

exit:
	dma_buf_put(dmabuf);
	VDEC_MUTEX_UNLOCK(&gMemMutex);
	return ret;
}

HI_S32 VDEC_MEM_IommuMap(HI_S32 share_fd, HI_U32 size, UADDR *iova)
{

	HI_U64 iova_addr;
	unsigned long phy_size;
	struct dma_buf *dmabuf;


	if (share_fd <= 0) {
		dprint(PRN_ERROR, "%s, share fd is invalid \n", __func__);
		return HI_FAILURE;
	}

	VDEC_MUTEX_LOCK(&gMemMutex);

	dmabuf = dma_buf_get(share_fd);
	if (IS_ERR(dmabuf)) {
		dprint(PRN_FATAL, "%s, dma_buf_get failed", __func__);
		VDEC_MUTEX_UNLOCK(&gMemMutex);
		return HI_FAILURE;
	}

	iova_addr = hisi_iommu_map_dmabuf(g_OmxVdecEntry.device, dmabuf, 0, &phy_size);
	if (!iova_addr) {
		dprint(PRN_FATAL, "%s, hisi_iommu_map_dmabuf failed", __func__);
		dma_buf_put(dmabuf);
		*iova = 0;
		VDEC_MUTEX_UNLOCK(&gMemMutex);
		return HI_FAILURE;
	}

	*iova = iova_addr;

	dma_buf_put(dmabuf);
	VDEC_MUTEX_UNLOCK(&gMemMutex);
	return HI_SUCCESS;

}

HI_S32 VDEC_MEM_IommuUnmap(HI_S32 share_fd, UADDR iova)
{
	struct dma_buf *dmabuf;
	HI_S32 ret = HI_SUCCESS;

	if (share_fd < 0) {
		dprint(PRN_ERROR, "%s, share fd is invalid \n", __func__);
		return HI_FAILURE;
	}

	if (!iova) {
		dprint(PRN_ERROR, "%s, iova is invalid \n", __func__);
		return HI_FAILURE;
	}

	VDEC_MUTEX_LOCK(&gMemMutex);

	dmabuf = dma_buf_get(share_fd);
	if (IS_ERR(dmabuf)) {
		dprint(PRN_FATAL, "%s, dma buf get failed \n", __func__);
		VDEC_MUTEX_UNLOCK(&gMemMutex);
		return HI_FAILURE;

	}

	ret = hisi_iommu_unmap_dmabuf(g_OmxVdecEntry.device, dmabuf,  iova);
	if (ret != 0) {
		dprint(PRN_ERROR, "%s: hisi iommu unmap dmabuf failed \n", __func__);
		ret = HI_FAILURE;
	}

	dma_buf_put(dmabuf);

	VDEC_MUTEX_UNLOCK(&gMemMutex);
	return ret;
}
#endif

