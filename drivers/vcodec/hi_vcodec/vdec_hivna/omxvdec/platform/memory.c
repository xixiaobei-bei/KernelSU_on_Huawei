

#include <linux/math64.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/file.h>
#include <linux/major.h>
#include <linux/syscalls.h>

#include "omxvdec.h"
#include "platform.h"
#include "memory.h"
#include "vfmw.h"

/*lint -e774*/
/********************************** MACRO *************************************/
#define MAX_ION_MEM_NODE  (100)
#define CLIENT_BUF_NAME   "CLIENT"

/********************************* STRUCT *************************************/
typedef struct {
	HI_U32 phys_addr;
	HI_VOID *virt_addr;
	HI_U32 size;
	struct ion_handle *handle;
} ION_MEM_DESC_S;

/*********************************** VARS *************************************/
struct  mutex gMemMutex;
struct ion_client *gIONClient = HI_NULL;
ION_MEM_DESC_S gIONMemNode[MAX_ION_MEM_NODE];

/******************************** LOCAL FUNC **********************************/
/*----------------------------------------
    func: normal memory free interface
 ----------------------------------------*/
static HI_VOID VDEC_MEM_FreeNormal_MMU(ION_MEM_DESC_S * pMemNode)
{
#ifdef HIVDEC_SMMU_SUPPORT
	/* ion_unmap iommu */
	ion_unmap_iommu(gIONClient, pMemNode->handle);
#endif
	if (pMemNode->virt_addr != HI_NULL) {
		ion_unmap_kernel(gIONClient, pMemNode->handle);
	}
	ion_free(gIONClient, pMemNode->handle);

	return;
}

/*----------------------------------------
    func: memory module probe entry
 ----------------------------------------*/
HI_S32 VDEC_MEM_Probe(HI_VOID)
{
	gIONClient = HI_NULL;
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

	VDEC_MUTEX_LOCK(&gMemMutex);

	gIONClient = (struct ion_client *)hisi_ion_client_create("hi_vcodec_ion");
	if (IS_ERR_OR_NULL(gIONClient)) {
		dprint(PRN_ERROR, "%s: ion client create failed\n", __func__);
		gIONClient = HI_NULL;
		ret = HI_FAILURE;
	}

	VDEC_MUTEX_UNLOCK(&gMemMutex);

	return ret;
}

/*----------------------------------------
    func: deinitialize memory module
 ----------------------------------------*/
HI_S32 VDEC_MEM_Exit(HI_VOID)
{
	HI_S32 i;

	VDEC_MUTEX_LOCK(&gMemMutex);

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

	VDEC_MUTEX_UNLOCK(&gMemMutex);
	return HI_SUCCESS;
}

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

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0)
	handle = ion_import_dma_buf_fd(gIONClient, share_fd);
#else
	handle = ion_import_dma_buf(gIONClient, share_fd);
#endif
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
	psMBuf->u32StartPhyAddr = (HI_U32) phy_addr;
	psMBuf->u32Size = phy_size;
	gIONMemNode[i].phys_addr = (HI_U32) phy_addr;
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
		if ((psMBuf->u32StartPhyAddr != 0)
			&& (psMBuf->u32StartPhyAddr == gIONMemNode[i].phys_addr)
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

