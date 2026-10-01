#include <asm/uaccess.h>
#include <linux/version.h>
#include <linux/dma-mapping.h>
#include <linux/dma-iommu.h>
#include <linux/dma-buf.h>
#include <linux/iommu.h>
#include <linux/hisi-iommu.h>
#include <linux/slab.h>

#include "drv_venc_osal.h"
#include "hi_drv_mem.h"

#define  MAX_BUFFER_SIZE (10*1024)

HI_CHAR *g_sbuf = NULL;
HI_S32   g_venc_node_num = 0;

struct semaphore    g_VencMemSem;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
struct ion_client  *g_ion_client = HI_NULL;
#endif
venc_mem_buf g_venc_mem_node[MAX_KMALLOC_MEM_NODE];
venc_mem_buf g_venc_ion_mem_node[MAX_ION_MEM_NODE];

VENC_SMMU_ERR_ADDR g_smmu_err_mem;
extern struct platform_device *gPlatDev;
HI_S32 DRV_MEM_INIT(HI_VOID)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	struct ion_client *ion_client;
#endif
	HI_CHAR *sbuf;
	HI_S32 s32Ret;
	MEM_BUFFER_S  MEM_SMMU_RD_ADDR;
	MEM_BUFFER_S  MEM_SMMU_WR_ADDR;

	HiVENC_INIT_MUTEX(&g_VencMemSem);

	sbuf = HiMemVAlloc(MAX_BUFFER_SIZE);
	if (!sbuf) {
		HI_FATAL_VENC("call vmalloc failed\n");
		goto err_client_exit;
	}

	HiMemSet((HI_VOID *)&g_venc_mem_node, 0, MAX_KMALLOC_MEM_NODE*sizeof(g_venc_mem_node[0]));
	HiMemSet((HI_VOID *)&g_venc_ion_mem_node, 0, MAX_ION_MEM_NODE*sizeof(g_venc_ion_mem_node[0]));

	HiMemSet((HI_VOID *)&g_smmu_err_mem, 0, sizeof(g_smmu_err_mem));
	HiMemSet((HI_VOID *)&MEM_SMMU_RD_ADDR, 0, sizeof(MEM_BUFFER_S));
	HiMemSet((HI_VOID *)&MEM_SMMU_WR_ADDR, 0, sizeof(MEM_BUFFER_S));

	MEM_SMMU_RD_ADDR.u32Size = SMMU_RWERRADDR_SIZE;
	s32Ret = DRV_MEM_KAlloc("SMMU_RDERR", "OMXVENC", &MEM_SMMU_RD_ADDR);
	if (s32Ret != HI_SUCCESS ) {
		HI_ERR_VENC("SMMU_RDERR alloc failed\n");
		goto err_sbuf_exit;
	}

	MEM_SMMU_WR_ADDR.u32Size = SMMU_RWERRADDR_SIZE;
	s32Ret = DRV_MEM_KAlloc("SMMU_WRERR", "OMXVENC", &MEM_SMMU_WR_ADDR);
	if (s32Ret != HI_SUCCESS ) {
		HI_ERR_VENC("SMMU_WRERR alloc failed\n");
		goto err_rd_smmu_exit;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	ion_client = (struct ion_client *)hisi_ion_client_create("hi_vcodec_ion_venc");
	if (IS_ERR_OR_NULL(ion_client)) {
		HI_FATAL_VENC("failed to create hi_vcodec ion client\n");
		goto err_rd_smmu_exit;
	}
	g_ion_client = ion_client;
#endif

	g_smmu_err_mem.RdAddr = MEM_SMMU_RD_ADDR.u64StartPhyAddr;//config alloc phyaddr,in order system don't dump
	g_smmu_err_mem.WrAddr = MEM_SMMU_WR_ADDR.u64StartPhyAddr;
	g_venc_node_num = 0;

	g_sbuf = sbuf;
	HiMemSet((HI_VOID *)g_sbuf, 0, MAX_BUFFER_SIZE);

	return HI_SUCCESS;

err_rd_smmu_exit:
	DRV_MEM_KFree(&MEM_SMMU_RD_ADDR);
err_sbuf_exit:
	HiMemVFree(sbuf);
err_client_exit:
	return HI_FAILURE;

}

HI_S32 DRV_MEM_EXIT(HI_VOID)
{
	HI_S32 i;

	if (g_sbuf) {
		HiMemVFree(g_sbuf);
		g_sbuf = HI_NULL;
	}

	/* Exit kfree mem for register's VEDU_COMN1_REGS.COMN1_SMMU_ERR_RDADDRR*/
	for (i = 0; i < MAX_KMALLOC_MEM_NODE; i++) {
		if (g_venc_mem_node[i].virt_addr != HI_NULL) {
			kfree(g_venc_mem_node[i].virt_addr);
			HiMemSet(&g_venc_mem_node[i], 0, sizeof(g_venc_mem_node[i]));
		}
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	for (i = 0; i < MAX_ION_MEM_NODE; i++) {
		if ((g_venc_ion_mem_node[i].phys_addr != 0) && (g_venc_ion_mem_node[i].handle != HI_NULL)) {
			HI_INFO_VENC("MEM not free, will release resource\n");
			ion_unmap_kernel(g_ion_client, g_venc_ion_mem_node[i].handle);
			ion_free(g_ion_client, g_venc_ion_mem_node[i].handle);

			HiMemSet(&g_venc_ion_mem_node[i], 0, sizeof(g_venc_ion_mem_node[i]));
		}
	}

	ion_client_destroy(g_ion_client);
	g_ion_client = HI_NULL;
#endif

	g_venc_node_num = 0;

	return HI_SUCCESS;
}

/* kalloc */
HI_S32 DRV_MEM_KAlloc(const HI_CHAR* bufName, const HI_CHAR *zone_name, MEM_BUFFER_S *psMBuf)
{
	HI_U32  i;
	HI_S32  ret = HI_FAILURE;
	HI_VOID *virt_addr = HI_NULL;

	if (psMBuf == HI_NULL || psMBuf->u32Size == 0) {
		HI_FATAL_VENC("invalid Param, psMBuf is NULL or size is zero\n");
		return ret;
	}

	if (HiVENC_DOWN_INTERRUPTIBLE(&g_VencMemSem)) {
		HI_FATAL_VENC("Kalloc, down_interruptible failed\n");
		return ret;
	}

	for (i = 0; i < MAX_KMALLOC_MEM_NODE; i++) {
		if ((0 == g_venc_mem_node[i].phys_addr) && (g_venc_mem_node[i].virt_addr == HI_NULL)) {
			break;
		}
	}

	if (i == MAX_KMALLOC_MEM_NODE) {
		HI_FATAL_VENC("No free ion mem node\n");
		goto err_exit;
	}

	virt_addr = kmalloc(psMBuf->u32Size, GFP_KERNEL | GFP_DMA);/*lint !e747*/
	if (IS_ERR_OR_NULL(virt_addr)) {
		HI_FATAL_VENC("call kzalloc failed, size : %d\n", psMBuf->u32Size);
		goto err_exit;
	}

	memset(virt_addr, 0, psMBuf->u32Size); /* unsafe_function_ignore: memset */ /*lint !e668*/

	psMBuf->pStartVirAddr   = virt_addr;
	psMBuf->u64StartPhyAddr = __pa(virt_addr);/*lint !e648 !e834 !e712*/

	snprintf(g_venc_mem_node[i].node_name, MAX_MEM_NAME_LEN, "%s", bufName);  /* unsafe_function_ignore: snprintf */

	snprintf(g_venc_mem_node[i].zone_name, MAX_MEM_NAME_LEN, "%s", zone_name);  /* unsafe_function_ignore: snprintf */

	g_venc_mem_node[i].virt_addr = psMBuf->pStartVirAddr;
	g_venc_mem_node[i].phys_addr = psMBuf->u64StartPhyAddr;
	g_venc_mem_node[i].size      = psMBuf->u32Size;

	g_venc_node_num++;

	ret = HI_SUCCESS;

err_exit:
	HiVENC_UP_INTERRUPTIBLE(&g_VencMemSem);
	return ret;   /*lint !e593*/
}   /*lint !e593*/

/* kfree */
HI_S32 DRV_MEM_KFree(const MEM_BUFFER_S *psMBuf)
{
        HI_U32  i;
        HI_S32  ret = HI_FAILURE;

        if (HI_NULL == psMBuf || psMBuf->pStartVirAddr == HI_NULL || psMBuf->u64StartPhyAddr == 0) {
                HI_FATAL_VENC("invalid Parameters\n");
                return ret;
        }

        if (HiVENC_DOWN_INTERRUPTIBLE(&g_VencMemSem)) {
                HI_FATAL_VENC("Kfree, down interruptible failed\n");
                return ret;
        }


        for (i=0; i<MAX_KMALLOC_MEM_NODE; i++) {
                if ((psMBuf->u64StartPhyAddr == g_venc_mem_node[i].phys_addr) &&
                        (psMBuf->pStartVirAddr == g_venc_mem_node[i].virt_addr))
                {
                        break;
                }
        }

        if(i == MAX_KMALLOC_MEM_NODE) {
                HI_FATAL_VENC("No free ion mem node\n");
                goto err_exit;
        }

        kfree(g_venc_mem_node[i].virt_addr);
        HiMemSet(&g_venc_mem_node[i], 0, sizeof(g_venc_mem_node[i]));/*lint !e866 */
        g_venc_node_num = (g_venc_node_num > 0)?(g_venc_node_num-1):0;

        ret = HI_SUCCESS;

err_exit:
        HiVENC_UP_INTERRUPTIBLE(&g_VencMemSem);
        return ret;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
/*----------------------------------------
    func: map ion buffer
 ----------------------------------------*/
HI_S32 DRV_MEM_MapKernel(HI_S32 share_fd, MEM_BUFFER_S *psMBuf)
{
	HI_S32 ret;
	HI_U32 i;
	/*size_t len = 0;
	HI_U32 mmu_bypass_flag = 0;*/
	struct ion_handle *handle = HI_NULL;
	ion_phys_addr_t phys_addr;

	struct iommu_map_format iommu_format;
	HiMemSet((HI_VOID *)&iommu_format, 0, sizeof(iommu_format));

	if (share_fd < 0) {
		HI_FATAL_VENC("invalid Param, share_fd is illegal\n");
		return HI_FAILURE;
	}

	if (HiVENC_DOWN_INTERRUPTIBLE(&g_VencMemSem)) {
		HI_FATAL_VENC("Map down interruptible failed\n");
		return HI_FAILURE;
	}

	if (!g_ion_client) {
		HI_FATAL_VENC("invalid Param, g_ion_client is NULL\n");
		goto err_exit;
	}

	for (i = 0; i < MAX_ION_MEM_NODE; i++) {
		if (0 == g_venc_ion_mem_node[i].phys_addr && HI_NULL == g_venc_ion_mem_node[i].handle) {
			break;
		}
	}

	if (i == MAX_ION_MEM_NODE) {
		HI_FATAL_VENC("No free ion mem node\n");
		goto err_exit;
	}

	handle = ion_import_dma_buf_fd(g_ion_client, share_fd);

	if (IS_ERR_OR_NULL(handle)) {
		HI_FATAL_VENC("get ion handle failed\n");
		goto err_exit;
	}

	/* FIXME : bypass flag must be reserved for specific scenarios */
	ret = ion_map_iommu(g_ion_client, handle, &iommu_format);
	if (ret) {
		HI_FATAL_VENC("map ion iommu failed\n");
		goto err_ion_map;
	}
	phys_addr = iommu_format.iova_start;
	psMBuf->u32Size = (HI_U32) iommu_format.iova_size;

	psMBuf->u64StartPhyAddr = phys_addr;
	psMBuf->u32ShareFd      = share_fd;

	g_venc_ion_mem_node[i].handle    = handle;
	g_venc_ion_mem_node[i].phys_addr = phys_addr;
	g_venc_ion_mem_node[i].size      = psMBuf->u32Size;

	HiVENC_UP_INTERRUPTIBLE(&g_VencMemSem);
	return HI_SUCCESS;

err_ion_map:
	ion_free(g_ion_client, handle);
err_exit:
	HiVENC_UP_INTERRUPTIBLE(&g_VencMemSem);
	return HI_FAILURE;
}

/*----------------------------------------
    func: unmap ion buffer
 ----------------------------------------*/
HI_S32 DRV_MEM_UnmapKernel(MEM_BUFFER_S *psMBuf)
{
	HI_S32 i;
	HI_S32 ret = HI_FAILURE;
	HI_U32 mmu_bypass_flag = 0;

	if (!psMBuf) {
		HI_FATAL_VENC("invalid Param, psMBuf is NULL\n");
		return HI_FAILURE;
	}

	if (HiVENC_DOWN_INTERRUPTIBLE(&g_VencMemSem)) {
		HI_FATAL_VENC("Unmap down interruptible failed\n");
		return HI_FAILURE;
	}

	if (!g_ion_client) {
		HI_FATAL_VENC("invalid Param, g_ion_client is NULL\n");
		goto err_exit;
	}

	for (i = 0; i < MAX_ION_MEM_NODE; i++) {
		if ((psMBuf->u64StartPhyAddr) && (psMBuf->u64StartPhyAddr == g_venc_ion_mem_node[i].phys_addr)
			&& (g_venc_ion_mem_node[i].handle != HI_NULL)) {
			break;
		}
	}

	if (i == MAX_ION_MEM_NODE) {
		HI_FATAL_VENC("Ion mem node not found\n");
		goto err_exit;
	}

	if (!mmu_bypass_flag) {
		ion_unmap_iommu(g_ion_client, g_venc_ion_mem_node[i].handle);
	}

	ion_free(g_ion_client, g_venc_ion_mem_node[i].handle);
	HiMemSet(&g_venc_ion_mem_node[i], 0, sizeof(g_venc_ion_mem_node[i]));
	ret = HI_SUCCESS;

err_exit:
	HiVENC_UP_INTERRUPTIBLE(&g_VencMemSem);
	return ret;
}
#else

/*----------------------------------------
    func: get map info
 ----------------------------------------*/
HI_S32 DRV_MEM_GetMapInfo(HI_S32 share_fd, MEM_BUFFER_S *psMBuf)
{
	HI_U64 iova_addr;
	struct dma_buf *dmabuf;
	HI_S32 ret = HI_SUCCESS;
	unsigned long iova_size = 0;

	if (share_fd <= 0) {
		HI_FATAL_VENC("invalid Param, share_fd is illegal\n");
		return HI_FAILURE;
	}

	if (HiVENC_DOWN_INTERRUPTIBLE(&g_VencMemSem)) {
		HI_FATAL_VENC("Map down interruptible failed\n");
		return HI_FAILURE;
	}

	dmabuf = dma_buf_get(share_fd);
	if (IS_ERR(dmabuf)) {
		HI_FATAL_VENC("%s, get dma buf failed", __func__);
		ret = HI_FAILURE;
		goto exit_1;
	}

	iova_addr = hisi_iommu_map_dmabuf(&gPlatDev->dev, dmabuf, 0, &iova_size);
	if (!iova_addr) {
		HI_FATAL_VENC("%s, iommu map dmabuf failed", __func__);
		ret = HI_FAILURE;
		goto exit_2;
	}

	psMBuf->u32Size         = iova_size;
	psMBuf->u64StartPhyAddr = iova_addr;
	psMBuf->u32ShareFd      = share_fd;

exit_2:
	dma_buf_put(dmabuf);
exit_1:
	HiVENC_UP_INTERRUPTIBLE(&g_VencMemSem);
	return ret;
}

/*----------------------------------------
    func: put map info
 ----------------------------------------*/
HI_S32 DRV_MEM_PutMapInfo(MEM_BUFFER_S *psMBuf)
{
	struct dma_buf *dmabuf;
	HI_S32 ret = HI_SUCCESS;

	if (!psMBuf) {
		HI_FATAL_VENC("invalid Param, psMBuf is NULL\n");
		return HI_FAILURE;
	}

	if (psMBuf->u32ShareFd <= 0) {
		HI_FATAL_VENC("share fd is invalid\n");
		return HI_FAILURE;
	}

	if (HiVENC_DOWN_INTERRUPTIBLE(&g_VencMemSem)) {
		HI_FATAL_VENC("Unmap down interruptible failed\n");
		return HI_FAILURE;
	}

	dmabuf = dma_buf_get(psMBuf->u32ShareFd);
	if (IS_ERR(dmabuf)) {
		HI_FATAL_VENC("%s, get dma buf failed", __func__);
		HiVENC_UP_INTERRUPTIBLE(&g_VencMemSem);
		return HI_FAILURE;
	}

	ret = hisi_iommu_unmap_dmabuf(&gPlatDev->dev, dmabuf, psMBuf->u64StartPhyAddr);
	if (ret != 0) {
		HI_FATAL_VENC("%s: hisi_iommu_unmap_dmabuf failed \n", __func__);
		ret = HI_FAILURE;
	}

	dma_buf_put(dmabuf);
	HiVENC_UP_INTERRUPTIBLE(&g_VencMemSem);
	return ret;
}

HI_S32 DRV_MEM_IommuMap(VencBufferRecord *node, struct platform_device *pltdev)
{
	HI_U64 iova_addr;
	struct dma_buf *dmabuf;

	HI_S32 ret = HI_SUCCESS;
	unsigned long iova_size = 0;

	if (!node) {
		HI_FATAL_VENC("node is invalid \n");
		return HI_FAILURE;
	}

	if (node->share_fd <= 0) {
		HI_FATAL_VENC("share fd is invalid \n");
		return HI_FAILURE;
	}

	if (HiVENC_DOWN_INTERRUPTIBLE(&g_VencMemSem)) {
		HI_FATAL_VENC("Map down interruptible failed\n");
		return HI_FAILURE;
	}

	dmabuf = dma_buf_get(node->share_fd);
	if (IS_ERR(dmabuf)) {
		HI_FATAL_VENC("%s, get dma buf failed", __func__);
		ret = HI_FAILURE;
		goto exit_1;
	}

	iova_addr = hisi_iommu_map_dmabuf(&pltdev->dev, dmabuf, 0, &iova_size);
	if (!iova_addr) {
		HI_FATAL_VENC("%s, iommu map dmabuf failed", __func__);
		ret = HI_FAILURE;
		goto exit_2;
	}

	node->iova = iova_addr;

exit_2:
	dma_buf_put(dmabuf);
exit_1:
	HiVENC_UP_INTERRUPTIBLE(&g_VencMemSem);
	return ret;
}

HI_S32 DRV_MEM_IommuUnmap(HI_S32 share_fd, HI_S32 phys_addr, struct platform_device *pltdev)
{
	struct dma_buf *dmabuf;
	HI_S32 ret = HI_SUCCESS;

	if (share_fd <= 0) {
		HI_FATAL_VENC("share fd is invalid\n");
		return HI_FAILURE;
	}

	if (!phys_addr) {
		HI_FATAL_VENC("phys addr is invalid\n");
		return HI_FAILURE;
	}

	if (HiVENC_DOWN_INTERRUPTIBLE(&g_VencMemSem)) {
		HI_FATAL_VENC("IommuUnMap down interruptible failed\n");
		return HI_FAILURE;
	}

	dmabuf = dma_buf_get(share_fd);
	if (IS_ERR(dmabuf)) {
		HI_FATAL_VENC("%s, get dma buf failed", __func__);
		HiVENC_UP_INTERRUPTIBLE(&g_VencMemSem);
		return HI_FAILURE;
	}

	ret = hisi_iommu_unmap_dmabuf(&pltdev->dev, dmabuf, phys_addr);
	if (ret != 0) {
		HI_FATAL_VENC("%s: hisi_iommu_unmap_dmabuf failed \n", __func__);
		ret = HI_FAILURE;
	}

	dma_buf_put(dmabuf);

	HiVENC_UP_INTERRUPTIBLE(&g_VencMemSem);
	return ret;
}
#endif

HI_S32 HI_DRV_UserCopy(struct file *file, HI_U32 cmd, unsigned long arg,
			long (*func)(struct file *file, HI_U32 cmd, unsigned long uarg))
{
	//HI_CHAR  sbuf[768];
	HI_VOID  *parg = NULL;
	HI_S32   err   = -EINVAL;

	/*  Copy arguments into temp kernel buffer  */
	if (!(void __user*)arg) {
		HI_FATAL_VENC("arg is NULL\n");
		goto out;
	}

	if (_IOC_SIZE(cmd) <= MAX_BUFFER_SIZE) {
		parg = g_sbuf;
	} else {
		HI_FATAL_VENC("cmd size is too long\n");
		goto out;
	}

	if (!parg) {
		HI_FATAL_VENC("parg is NULL\n");
		goto out;
	}
	err = -EFAULT;
	if (_IOC_DIR(cmd) & _IOC_WRITE) {
		if (copy_from_user(parg, (void __user*)arg, _IOC_SIZE(cmd))) {/*lint !e747 */
			HI_FATAL_VENC("copy_from_user failed, cmd value is 0x%x\n", cmd);
			goto out;
		}
	}

	/* call driver */
	err = func(file, cmd, (long)(parg)); /*lint !e732 !e712*/
	if (err == -ENOIOCTLCMD)
		err = -EINVAL;
	if (err < 0)
		goto out;

	/*  Copy results into user buffer  */
	switch (_IOC_DIR(cmd)) {
	case _IOC_READ:
	case (_IOC_WRITE | _IOC_READ):
		if (copy_to_user((void __user *)arg, parg, _IOC_SIZE(cmd))) {/*lint !e747 */
			HI_FATAL_VENC("copy_to_user failed, cmd value is 0x%x\n", cmd);
			err = -EFAULT;
		}
		break;
	default:
		goto out;
	}
out:
	return err;
}


