
#include "hi_drv_mem.h"
#include <asm/uaccess.h>
#include <linux/version.h>
#include <linux/dma-mapping.h>
#include <linux/dma-iommu.h>
#include <linux/dma-buf.h>
#include <linux/iommu.h>
#include <linux/hisi-iommu.h>
#include <linux/slab.h>
#include "drv_venc_osal.h"
#include "venc_regulator.h"

#define  MAX_BUFFER_SIZE (10*1024)

HI_CHAR *g_sbuf;
HI_S32   g_venc_node_num;

struct semaphore    g_venc_mem_sem;
venc_mem_buf_t g_venc_mem_node[MAX_KMALLOC_MEM_NODE];

venc_smmu_err_add_t g_smmu_err_mem;

HI_S32 drv_mem_init(HI_VOID)
{
	HI_CHAR *sbuf = NULL;
	HI_S32 ret;
	mem_buffer_t  mem_smmu_read_addr;
	mem_buffer_t  mem_smmu_write_addr;

	hi_venc_init_mutex(&g_venc_mem_sem);

	sbuf = hi_mem_valloc(MAX_BUFFER_SIZE);
	if (!sbuf) {
		HI_FATAL_VENC("call vmalloc failed\n");
		goto err_client_exit;
	}

	memset((HI_VOID *)&g_venc_mem_node, 0, MAX_KMALLOC_MEM_NODE * sizeof(g_venc_mem_node[0]));    // unsafe_function_ignore: memset

	memset((HI_VOID *)&g_smmu_err_mem, 0, sizeof(g_smmu_err_mem));    // unsafe_function_ignore: memset
	memset((HI_VOID *)&mem_smmu_read_addr, 0, sizeof(mem_buffer_t));    // unsafe_function_ignore: memset
	memset((HI_VOID *)&mem_smmu_write_addr, 0, sizeof(mem_buffer_t));    // unsafe_function_ignore: memset

	mem_smmu_read_addr.size = SMMU_RWERRADDR_SIZE;
	ret = drv_mem_kalloc("SMMU_RDERR", "OMXVENC", &mem_smmu_read_addr);
	if (ret != HI_SUCCESS) {
		HI_ERR_VENC("SMMU_RDERR alloc failed\n");
		goto err_sbuf_exit;
	}

	mem_smmu_write_addr.size = SMMU_RWERRADDR_SIZE;
	ret = drv_mem_kalloc("SMMU_WRERR", "OMXVENC", &mem_smmu_write_addr);
	if (ret != HI_SUCCESS) {
		HI_ERR_VENC("SMMU_WRERR alloc failed\n");
		goto err_rd_smmu_exit;
	}

	g_smmu_err_mem.read_addr = mem_smmu_read_addr.start_phys_addr;    // config alloc phyaddr,in order system don't dump
	g_smmu_err_mem.write_addr = mem_smmu_write_addr.start_phys_addr;
	g_venc_node_num = 0;

	g_sbuf = sbuf;
	memset((HI_VOID *)g_sbuf, 0, MAX_BUFFER_SIZE);    // unsafe_function_ignore: memset

	return HI_SUCCESS;

err_rd_smmu_exit:
	drv_mem_kfree(&mem_smmu_read_addr);
err_sbuf_exit:
	hi_mem_vfree(sbuf);
err_client_exit:
	return HI_FAILURE;

}

HI_S32 drv_mem_exit(HI_VOID)
{
	HI_S32 i;

	if (g_sbuf) {
		hi_mem_vfree(g_sbuf);
		g_sbuf = HI_NULL;
	}

	/* Exit kfree mem for register's VEDU_COMN1_REGS.COMN1_SMMU_ERR_RDADDRR */
	for (i = 0; i < MAX_KMALLOC_MEM_NODE; i++) {
		if (g_venc_mem_node[i].virt_addr != HI_NULL) {
			kfree(g_venc_mem_node[i].virt_addr);
			memset(&g_venc_mem_node[i], 0, sizeof(g_venc_mem_node[i]));    // unsafe_function_ignore: memset
		}
	}

	g_venc_node_num = 0;

	return HI_SUCCESS;
}

HI_S32 drv_mem_kalloc(const HI_CHAR *bufname, const HI_CHAR *zone_name, mem_buffer_t *psmbuf)
{
	HI_U32  i;
	HI_S32  ret = HI_FAILURE;
	HI_VOID *virt_addr = HI_NULL;

	if (psmbuf == HI_NULL || psmbuf->size == 0) {
		HI_FATAL_VENC("invalid Param, psmbuf is NULL or size is zero\n");
		return ret;
	}

	if (hi_venc_down_interruptible(&g_venc_mem_sem)) {
		HI_FATAL_VENC("Kalloc, down_interruptible failed\n");
		return ret;
	}

	for (i = 0; i < MAX_KMALLOC_MEM_NODE; i++) {
		if ((g_venc_mem_node[i].phys_addr == 0) && (g_venc_mem_node[i].virt_addr == HI_NULL))
			break;
	}

	if (i == MAX_KMALLOC_MEM_NODE) {
		HI_FATAL_VENC("No free ion mem node\n");
		goto err_exit;
	}

	virt_addr = kzalloc(psmbuf->size, GFP_KERNEL | GFP_DMA); /* lint !e747 */
	if (IS_ERR_OR_NULL(virt_addr)) {
		HI_FATAL_VENC("call kzalloc failed, size : %d\n", psmbuf->size);
		goto err_exit;
	}

	psmbuf->start_virt_addr   = virt_addr;
	psmbuf->start_phys_addr = __pa((uintptr_t)virt_addr); /* lint !e648 !e834 !e712 */

	snprintf(g_venc_mem_node[i].node_name, MAX_MEM_NAME_LEN, "%s", bufname);    /* unsafe_function_ignore: snprintf */

	snprintf(g_venc_mem_node[i].zone_name, MAX_MEM_NAME_LEN, "%s", zone_name);    /* unsafe_function_ignore: snprintf */

	g_venc_mem_node[i].virt_addr = psmbuf->start_virt_addr;
	g_venc_mem_node[i].phys_addr = psmbuf->start_phys_addr;
	g_venc_mem_node[i].size      = psmbuf->size;

	g_venc_node_num++;

	ret = HI_SUCCESS;

err_exit:
	hi_venc_up_interruptible(&g_venc_mem_sem);
	return ret; /*lint !e593*/
}

HI_S32 drv_mem_kfree(const mem_buffer_t *psmbuf)
{
	HI_U32  i;
	HI_S32  ret = HI_FAILURE;

	if (psmbuf == HI_NULL || psmbuf->start_virt_addr == HI_NULL || psmbuf->start_phys_addr == 0) {
		HI_FATAL_VENC("invalid Parameters\n");
		return ret;
	}

	if (hi_venc_down_interruptible(&g_venc_mem_sem)) {
		HI_FATAL_VENC("Kfree, down interruptible failed\n");
		return ret;
	}

	for (i = 0; i < MAX_KMALLOC_MEM_NODE; i++) {
		if ((psmbuf->start_phys_addr == g_venc_mem_node[i].phys_addr) &&
			(psmbuf->start_virt_addr == g_venc_mem_node[i].virt_addr)) {
			break;
		}
	}

	if (i == MAX_KMALLOC_MEM_NODE) {
		HI_FATAL_VENC("No free ion mem node\n");
		goto err_exit;
	}

	kfree(g_venc_mem_node[i].virt_addr);
	memset(&g_venc_mem_node[i], 0, sizeof(g_venc_mem_node[i]));    // unsafe_function_ignore: memset
	g_venc_node_num = (g_venc_node_num > 0) ? (g_venc_node_num - 1) : 0;

	ret = HI_SUCCESS;

err_exit:
	hi_venc_up_interruptible(&g_venc_mem_sem);
	return ret;
}

/*----------------------------------------
 *   func: get map info
 *----------------------------------------
 */
HI_S32 drv_mem_get_map_info(HI_S32 shared_fd, mem_buffer_t *psmbuf)
{
	HI_U64 iova_addr;
	struct dma_buf *dmabuf = NULL;
	HI_S32 ret = HI_SUCCESS;
	unsigned long iova_size = 0;

	struct platform_device *venc_dev = NULL;

	if (shared_fd < 0) {
		HI_FATAL_VENC("invalid Param, shared_fd is illegal\n");
		return HI_FAILURE;
	}

	if (hi_venc_down_interruptible(&g_venc_mem_sem)) {
		HI_FATAL_VENC("Map down interruptible failed\n");
		return HI_FAILURE;
	}

	dmabuf = dma_buf_get(shared_fd);
	if (IS_ERR(dmabuf)) {
		HI_FATAL_VENC("%s, get dma buf failed", __func__);
		ret = HI_FAILURE;
		goto exit_1;
	}

	venc_dev = venc_get_device();

	iova_addr = hisi_iommu_map_dmabuf(&venc_dev->dev, dmabuf, 0, &iova_size);
	if (!iova_addr) {
		HI_FATAL_VENC("%s, iommu map dmabuf failed", __func__);
		ret = HI_FAILURE;
		goto exit_2;
	}

	psmbuf->size         = iova_size;
	psmbuf->start_phys_addr = iova_addr;
	psmbuf->share_fd      = shared_fd;

exit_2:
	dma_buf_put(dmabuf);
exit_1:
	hi_venc_up_interruptible(&g_venc_mem_sem);
	return ret;
}

/*----------------------------------------
 *   func: put map info
 *----------------------------------------
 */
HI_S32 drv_mem_put_map_info(mem_buffer_t *psmbuf)
{
	struct dma_buf *dmabuf = NULL;
	HI_S32 ret = 0;
	struct platform_device *venc_dev = NULL;

	if (!psmbuf) {
		HI_FATAL_VENC("invalid Param, psmbuf is NULL\n");
		return HI_FAILURE;
	}

	if (psmbuf->share_fd < 0) {
		HI_FATAL_VENC("share fd is invalid\n");
		return HI_FAILURE;
	}

	if (hi_venc_down_interruptible(&g_venc_mem_sem)) {
		HI_FATAL_VENC("Unmap down interruptible failed\n");
		return HI_FAILURE;
	}

	dmabuf = dma_buf_get(psmbuf->share_fd);
	if (IS_ERR(dmabuf)) {
		HI_FATAL_VENC("%s, get dma buf failed", __func__);
		hi_venc_up_interruptible(&g_venc_mem_sem);
		return HI_FAILURE;
	}

	venc_dev = venc_get_device();

	ret = hisi_iommu_unmap_dmabuf(&venc_dev->dev, dmabuf, psmbuf->start_phys_addr);
	if (ret != 0) {
		HI_FATAL_VENC("%s: hisi_iommu_unmap_dmabuf failed\n", __func__);
		ret = HI_FAILURE;
	}

	dma_buf_put(dmabuf);
	hi_venc_up_interruptible(&g_venc_mem_sem);
	return ret;
}

HI_S32 drv_mem_iommumap(venc_buffer_record_t *node, struct platform_device *pdev)
{
	HI_U64 iova_addr;
	struct dma_buf *dmabuf = NULL;

	HI_S32 ret = HI_SUCCESS;
	unsigned long iova_size = 0;

	if (!node) {
		HI_FATAL_VENC("node is invalid\n");
		return HI_FAILURE;
	}

	if (node->shared_fd < 0) {
		HI_FATAL_VENC("share fd is invalid\n");
		return HI_FAILURE;
	}

	if (hi_venc_down_interruptible(&g_venc_mem_sem)) {
		HI_FATAL_VENC("Map down interruptible failed\n");
		return HI_FAILURE;
	}

	dmabuf = dma_buf_get(node->shared_fd);
	if (IS_ERR(dmabuf)) {
		HI_FATAL_VENC("%s, get dma buf failed", __func__);
		ret = HI_FAILURE;
		goto exit_1;
	}

	iova_addr = hisi_iommu_map_dmabuf(&pdev->dev, dmabuf, 0, &iova_size);
	if (!iova_addr) {
		HI_FATAL_VENC("%s, iommu map dmabuf failed", __func__);
		ret = HI_FAILURE;
		goto exit_2;
	}

	node->iova = iova_addr;

exit_2:
	dma_buf_put(dmabuf);
exit_1:
	hi_venc_up_interruptible(&g_venc_mem_sem);
	return ret;
}

HI_S32 drv_mem_iommuunmap(HI_S32 shared_fd, HI_S32 phys_addr, struct platform_device *pdev)
{
	struct dma_buf *dmabuf = NULL;
	HI_S32 ret;

	if (shared_fd < 0) {
		HI_FATAL_VENC("share fd is invalid\n");
		return HI_FAILURE;
	}

	if (!phys_addr) {
		HI_FATAL_VENC("phys addr is invalid\n");
		return HI_FAILURE;
	}

	if (hi_venc_down_interruptible(&g_venc_mem_sem)) {
		HI_FATAL_VENC("IommuUnMap down interruptible failed\n");
		return HI_FAILURE;
	}

	dmabuf = dma_buf_get(shared_fd);
	if (IS_ERR(dmabuf)) {
		HI_FATAL_VENC("%s, get dma buf failed", __func__);
		hi_venc_up_interruptible(&g_venc_mem_sem);
		return HI_FAILURE;
	}

	ret = hisi_iommu_unmap_dmabuf(&pdev->dev, dmabuf, phys_addr);
	if (ret != 0) {
		HI_FATAL_VENC("%s: hisi_iommu_unmap_dmabuf failed\n", __func__);
		ret = HI_FAILURE;
	}

	dma_buf_put(dmabuf);

	hi_venc_up_interruptible(&g_venc_mem_sem);
	return ret;
}

HI_S32 drv_mem_copy_from_user(HI_U32 cmd, const void __user *user_arg, HI_VOID **kernel_arg)
{
	HI_S32 err = HI_FAILURE;

	/* Copy arguments into temp kernel buffer */
	if (!(void __user *)user_arg || !kernel_arg) {
		HI_FATAL_VENC("arg is NULL\n");
		return err;
	}

	if (_IOC_SIZE(cmd) <= MAX_BUFFER_SIZE) {
		*kernel_arg = g_sbuf;
	} else {
		HI_FATAL_VENC("cmd size is too long\n");
		return err;
	}

	if (!(*kernel_arg)) {
		HI_FATAL_VENC("parg is NULL\n");
		return err;
	}

	if (_IOC_DIR(cmd) & _IOC_WRITE) {
		if (copy_from_user(*kernel_arg, (void __user *)(uintptr_t)user_arg, _IOC_SIZE(cmd))) { /*lint !e747*/
			HI_FATAL_VENC("copy_from_user failed, cmd value is 0x%x\n", cmd);
			return err;
		}
	}

	return HI_SUCCESS;
}
