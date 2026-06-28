/*************************************************************************/ /*!
@File           ion_client.c
@Title          
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    
@License        Strictly Confidential.
*/ /**************************************************************************/
#include <ion_client.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/version.h>

static struct ion_client *ion_client = NULL;

extern IMG_VOID *VDECKM_GetNativeDevice(void);

/*!
******************************************************************************
 @Function                get_ion_client
******************************************************************************/
struct ion_client *get_ion_client(void)
{
    if(!ion_client)
    	ion_client = hisi_ion_client_create("vcodec_ion_client");
    return ion_client;
}

void release_ion_client(void) {
	if(ion_client) {
		ion_client_destroy(ion_client);
		ion_client = NULL;
	}
}

struct sg_table *img_vdec_get_sg_table(struct ion_client *ionClient, struct ion_handle *ionHandle)
{
    struct sg_table *psSgTable = NULL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0)
    struct dma_buf *buf = NULL;
    struct dma_buf_attachment *attach = NULL;
    int shared_fd = 0;
    struct device *nativeDevice = NULL;

    nativeDevice = VDECKM_GetNativeDevice();
    IMG_ASSERT(nativeDevice);
    if (NULL == nativeDevice)
    {
        goto exit;
    }

    shared_fd = ion_share_dma_buf_fd(ionClient, ionHandle);
    if (shared_fd < 0)
    {
        IMG_ASSERT(IMG_FALSE);
        goto exit;
    }

    buf = dma_buf_get(shared_fd);
    if (IS_ERR(buf))
    {
        IMG_ASSERT(IMG_FALSE);
        goto exitdmabufferget;
    }

    attach = dma_buf_attach(buf, nativeDevice);
    if (IS_ERR(attach))
    {
        IMG_ASSERT(IMG_FALSE);
        goto exitdmabufattach;
    }

    psSgTable = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
    if (IS_ERR(psSgTable))
    {
        IMG_ASSERT(IMG_FALSE);
        psSgTable = NULL;
        goto exitdmabufmapattachment;
    }

    dma_buf_unmap_attachment(attach, psSgTable, DMA_BIDIRECTIONAL);

exitdmabufmapattachment:
    dma_buf_detach(buf, attach);
exitdmabufattach:
    dma_buf_put(buf);
exitdmabufferget:
    sys_close(shared_fd);
exit:
#else
    psSgTable = ion_sg_table(ionClient, ionHandle);
#endif

    return psSgTable;
}

int img_vdec_get_ion_phys(struct ion_client *ionClient, struct ion_handle *ionHandle, ion_phys_addr_t *addr, size_t *len)
{
    int ret = 0;
    if (NULL == ionClient || NULL == ionHandle)
    {
        IMG_ASSERT(IMG_FALSE);
        return -EFAULT;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0)
    struct sg_table *table = NULL;
    table = img_vdec_get_sg_table(ionClient, ionHandle);

    IMG_ASSERT(table);
    if (NULL == table)
    {
        return -EFAULT;
    }

    *addr = sg_phys(table->sgl);
    *len = sg_dma_len(table->sgl);
#else
    ret = ion_phys(ionClient, ionHandle, addr, len);
#endif
    return ret;
}
