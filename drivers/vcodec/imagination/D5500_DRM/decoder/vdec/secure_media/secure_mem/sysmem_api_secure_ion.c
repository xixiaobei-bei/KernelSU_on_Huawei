/*!
 *****************************************************************************
 *
 * @File       sysmem_api_secure_ion.c
 * @Title      Secure Memory Allocator
 * @Description    This file contains the interface to a secure memory allocator
 * ---------------------------------------------------------------------------
 *
 * Copyright (c) Imagination Technologies Ltd.
 * 
 * The contents of this file are subject to the MIT license as set out below.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 * 
 * Alternatively, the contents of this file may be used under the terms of the 
 * GNU General Public License Version 2 ("GPL")in which case the provisions of
 * GPL are applicable instead of those above. 
 * 
 * If you wish to allow use of your version of this file only under the terms 
 * of GPL, and not to allow others to use your version of this file under the 
 * terms of the MIT license, indicate your decision by deleting the provisions 
 * above and replace them with the notice and other provisions required by GPL 
 * as set out in the file called "GPLHEADER" included in this distribution. If 
 * you do not delete the provisions above, a recipient may use your version of 
 * this file under the terms of either the MIT license or GPL.
 * 
 * This License is also included in this distribution in the file called 
 * "MIT_COPYING".
 *
 *****************************************************************************/

#include <linux/mm.h>
#include <asm/io.h>
#include <linux/genalloc.h>
#include <linux/sched.h> // for task_tgid_nr
#include <linux/syscalls.h> // for sys_close
#include <img_include.h>
#include <system.h>
#include <sysmem_secure.h>
#include <sysbrg_utils.h>
#include <report_api.h>

#include <ion_client.h>
#include <secure_ion_config.h>

//#define USE_NON_SECURE_BUFFER
static struct ion_client * ion_client(void)
{
    static struct ion_client * cl = NULL;
    if(cl == NULL)
        cl = get_ion_client();
    return cl;
}

/*!
******************************************************************************

 @Function              secure_InitialiseMemory

******************************************************************************/
static IMG_RESULT secure_InitialiseMemory(
    struct SYSMEM_Heap *  psHeap
)
{
    REPORT(REPORT_MODULE_SYSMEM, REPORT_INFO, "%s", __FUNCTION__);
    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              secure_DeInitialiseMemory

******************************************************************************/
static IMG_VOID secure_DeInitialiseMemory(
    struct SYSMEM_Heap *  psHeap
)
{
    REPORT(REPORT_MODULE_SYSMEM, REPORT_INFO, "%s", __FUNCTION__);
}


/*!
******************************************************************************

 @Function              secure_ion_AllocatePages

******************************************************************************/
static IMG_RESULT secure_ion_AllocatePages(
    struct SYSMEM_Heap *  psHeap,
    IMG_UINT32            ui32Size,
    SYSMEMU_sPages *      psPages,
    SYS_eMemAttrib        eMemAttrib
    )
{
    IMG_RESULT           res = 0;
    struct ion_handle *  ion_handle;
    unsigned             allocFlags;
    size_t               numPages;
    size_t               physAddrArrSize;
    int                  ion_fd;
    IMG_PHYSADDR       * ppaCpuPhysAddrs;
    int                  ion_heap;
#ifdef USE_NON_SECURE_BUFFER
    ion_heap =14; //SECURE_ION_ALLOC_HEAP;
    allocFlags =0; // SECURE_ION_ALLOC_FLAGS;
#else
    ion_heap =SECURE_ION_ALLOC_HEAP;
    allocFlags = SECURE_ION_ALLOC_FLAGS;
#endif    

    ion_handle = ion_alloc(ion_client(), ui32Size, PAGE_SIZE,
                           1 << ion_heap,
                           allocFlags);
    if (IS_ERR(ion_handle)) {
        IMG_ASSERT(!"ion_alloc failed!!!!");
        REPORT(REPORT_MODULE_SYSMEM, REPORT_ERR, "%s alloc failed size %u attr 0x%x",
               __FUNCTION__, ui32Size, eMemAttrib);
        goto errIonAlloc;
    }

    // Get physical pages (assume contiguous memory area)
	{
		int offset;
		ion_phys_addr_t physaddr;
		size_t         len = 0;
		int            pg_i;

		res = img_vdec_get_ion_phys(ion_client(), ion_handle, &physaddr, &len);
		if(res)
		{
			IMG_ASSERT(!"ion_phys failed");
			res = IMG_ERROR_FATAL;
			goto errIonPhys;
		}

        /* create physical page array */
        numPages = (ui32Size + HOST_MMU_PAGE_SIZE - 1)/HOST_MMU_PAGE_SIZE;
        physAddrArrSize = sizeof(*ppaCpuPhysAddrs) * numPages;
        ppaCpuPhysAddrs = IMG_BIGORSMALL_ALLOC(physAddrArrSize);
        if(ppaCpuPhysAddrs == NULL) {
            IMG_ASSERT(ppaCpuPhysAddrs);
            res = IMG_ERROR_OUT_OF_MEMORY;
            goto errPhysArrAlloc;
        }
		for (pg_i = 0, offset = 0; pg_i < numPages; offset += HOST_MMU_PAGE_SIZE, ++pg_i)
		{
			ppaCpuPhysAddrs[pg_i] = physaddr + offset;
		}
        // Set pointer to physical address in structure
        psPages->ppaPhysAddr = ppaCpuPhysAddrs;
	}

    if (eMemAttrib & SYS_MEMATTRIB_SECURE_INPUT) {
        // get DMA-BUF file descriptor, for exporting into user space
        ion_fd = ion_share_dma_buf_fd(ion_client(), ion_handle);
        if (ion_fd < 0) {
            IMG_ASSERT(!"ion_share_dma_buf_fd failed");
            res = IMG_ERROR_FATAL;
            goto errIonShare;
        }
    } else {
        ion_fd = -1;
    }

    DEBUG_REPORT(REPORT_MODULE_SYSMEM, "%s (secure ion) heap %d size %u phys 0x%llx attr 0x%x dmabuf handle:%d",
                 __FUNCTION__, ion_heap, ui32Size, psPages->ppaPhysAddr[0], eMemAttrib, ion_fd);

    psPages->pvImplData  = ion_handle;
    psPages->ai32ImplData[0] = ion_fd;

    return IMG_SUCCESS;

errIonShare:
    IMG_BIGORSMALL_FREE(physAddrArrSize, ppaCpuPhysAddrs);
errIonPhys:
errPhysArrAlloc:
    ion_free(ion_client(), ion_handle);
errIonAlloc:
    return res;
}

/*!
******************************************************************************

 @Function              secure_ion_ImportPages

******************************************************************************/
static IMG_RESULT secure_ion_ImportPages(
    SYSMEM_Heap		*heap,
    SYSDEVU_sInfo	*sysdev,
    IMG_UINT32		ui32Size,
    SYSMEMU_sPages *psPages,
    SYS_eMemAttrib	eMemAttrib,
    IMG_INT32		buff_fd,
    IMG_UINT64		*pPhyAddrs,
    IMG_VOID		*priv,
    IMG_BOOL		kernelMapped
)
{
   	size_t numPages = (ui32Size + HOST_MMU_PAGE_SIZE - 1)/HOST_MMU_PAGE_SIZE;
	struct ion_handle *ionHandle;
	IMG_RESULT result = IMG_ERROR_FATAL;
	unsigned pg_i = 0;
	struct ion_client *pIONcl;

	DEBUG_REPORT(REPORT_MODULE_SYSMEM, "%s (ion) region of size %u buff_fd %d attr 0x%x",
				 __FUNCTION__, ui32Size, buff_fd, eMemAttrib);

	pIONcl = ion_client();
	if (!pIONcl)
		goto exitFailGetClient;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0)
	ionHandle = ion_import_dma_buf_fd(pIONcl, buff_fd);
#else
	ionHandle = ion_import_dma_buf(pIONcl, buff_fd);
#endif
	if (IS_ERR(ionHandle)) {
		REPORT(REPORT_MODULE_SYSMEM, REPORT_ERR, "Error obtaining handle from fd %d", buff_fd);
		result = IMG_ERROR_FATAL;
		goto exitFailImportFD;
	}

	psPages->pvImplData   = ionHandle;
	psPages->ai32ImplData[0] = buff_fd;

#if defined(SECURE_ION_IMPORT_SG_TABLE)
    {
        struct scatterlist *psScattLs, *psScattLsAux;
        struct sg_table *psSgTable;

        psSgTable = img_vdec_get_sg_table(pIONcl, ionHandle);

        if (psSgTable == NULL)
        {
            REPORT(REPORT_MODULE_SYSMEM, REPORT_ERR, "Error obtaining sg table");
            result = IMG_ERROR_FATAL;
            goto exitFailMap;
        }
        psScattLs = psSgTable->sgl;

        if (psScattLs == NULL)
        {
            REPORT(REPORT_MODULE_SYSMEM, REPORT_ERR, "Error obtaining scatter list");
            result = IMG_ERROR_FATAL;
            goto exitFailMap;
        }

        // Get physical addresses from scatter list
        for (psScattLsAux = psScattLs; psScattLsAux; psScattLsAux = sg_next(psScattLsAux))
        {
            int offset;
            dma_addr_t chunkBase = sg_phys(psScattLsAux);

            for (offset = 0; offset < psScattLsAux->length; offset += PAGE_SIZE, ++pg_i)
            {
                if (pg_i >= numPages)
                    break;

                pPhyAddrs[pg_i] = chunkBase + offset;
            }

            if (pg_i >= numPages)
                break;
        }

        if (kernelMapped)
            psPages->pvCpuKmAddr = ion_map_kernel(pIONcl, ionHandle);
    }
#else
	{
		int offset;
		ion_phys_addr_t physaddr;
		size_t len = 0;

		result = img_vdec_get_ion_phys(pIONcl, ionHandle, &physaddr, &len);

		if(result)
		{
			IMG_ASSERT(!"ion_phys failed");
			result = IMG_ERROR_FATAL;
			goto exitFailMap;
		}

		for (offset = 0; pg_i < numPages; offset += PAGE_SIZE, ++pg_i)
		{
			if (pg_i >= numPages)
				break;
			pPhyAddrs[pg_i] = physaddr + offset;
		}

		if (kernelMapped)
			psPages->pvCpuKmAddr = SYSMEMU_CpuPAddrToCpuKmAddr(heap->memId, physaddr);
	}
#endif

	{
		size_t  physAddrArrSize = numPages * sizeof(psPages->ppaPhysAddr[0]);
		size_t  phy_i;

		psPages->ppaPhysAddr = IMG_BIGORSMALL_ALLOC(physAddrArrSize);
		IMG_ASSERT(psPages->ppaPhysAddr != IMG_NULL);
		if (psPages->ppaPhysAddr == IMG_NULL)
		{
			return IMG_ERROR_OUT_OF_MEMORY;
		}

		for (phy_i = 0; phy_i < numPages; ++phy_i)
			psPages->ppaPhysAddr[phy_i] = pPhyAddrs[phy_i];
	}

	if (kernelMapped && psPages->pvCpuKmAddr == NULL) {
		REPORT(REPORT_MODULE_SYSMEM, REPORT_ERR, "Error mapping to kernel address");
		result = IMG_ERROR_FATAL;
		goto exitFailMapKernel;
	}

	result = IMG_SUCCESS;

exitFailMapKernel:
exitFailMap:
exitFailImportFD:
exitFailGetClient:
	return result;
}

static bool process_is_dead(IMG_VOID)
{
    if (NULL == current->files)
    {
        DEBUG_REPORT(REPORT_MODULE_SYSMEM, " %s, %d,the current process is dead\n",__FUNCTION__,__LINE__);
        return true;
    }
    return false;
}

/*!
******************************************************************************

 @Function              secure_ion_FreePages

******************************************************************************/
static IMG_VOID secure_ion_FreePages(
    struct SYSMEM_Heap *  psHeap,
    IMG_HANDLE            hPagesHandle
)
{
    SYSMEMU_sPages * psPages = (SYSMEMU_sPages *)hPagesHandle;
    struct ion_handle  * ion_handle = psPages->pvImplData;
    int ion_share_fd = psPages->ai32ImplData[0];
    IMG_UINT32 ui32NoPages = 0;
    IMG_UINT32 physAddrArrSize = 0;

    DEBUG_REPORT(REPORT_MODULE_SYSMEM, "%s (secure ion) size %u phys 0x%llx imported %d, duplicated: %d",
                 __FUNCTION__, psPages->ui32Size, psPages->ppaPhysAddr[0], psPages->bImported, psPages->bDuplicated);

    if (psPages->bDuplicated)
    {
        return;
    }

    if ((ion_share_fd >= 0) && (!psPages->bImported) && (false == process_is_dead()))
    {
        DEBUG_REPORT(REPORT_MODULE_SYSMEM, "%s closing share ion fd %d",
                     __FUNCTION__, ion_share_fd);
        sys_close(ion_share_fd);
    }

    if (ion_handle)
    {
        ion_free(ion_client(), ion_handle);
    }

    ui32NoPages = (psPages->ui32Size + (HOST_MMU_PAGE_SIZE-1)) / HOST_MMU_PAGE_SIZE;
    physAddrArrSize = sizeof(psPages->ppaPhysAddr[0]) * ui32NoPages;
    IMG_BIGORSMALL_FREE(physAddrArrSize, psPages->ppaPhysAddr);

    psPages->ppaPhysAddr = IMG_NULL;
    psPages->hRegHandle = IMG_NULL;
}


/*!
******************************************************************************

 @Function              secure_ion_GetFileDescriptor

******************************************************************************/
static IMG_INT32 secure_ion_GetFileDescriptor(
    SYSMEMU_sPages * psPages
)
{
    return psPages->ai32ImplData[0];
}


/*!
******************************************************************************

 @Function              secure_GetCpuKmAddr

******************************************************************************/
static IMG_RESULT secure_GetCpuKmAddr(
    struct SYSMEM_Heap *  psHeap,
    IMG_VOID **           ppvCpuKmAddr,
    IMG_HANDLE            hPagesHandle
)
{
#ifdef SYSBRG_BRIDGING
    IMG_ASSERT(!"not allowed to map secure pages into host (kernel)");
    return IMG_ERROR_FATAL;
#else
    SYSMEMU_sPages *  psPages;
    
    psPages = (SYSMEMU_sPages *)hPagesHandle;

    IMG_ASSERT(0);

    *ppvCpuKmAddr = (IMG_VOID*)((IMG_UINTPTR)psPages->ppaPhysAddr[0]);

    return IMG_SUCCESS;
#endif
}

/*!
******************************************************************************

 @Function              secure_CpuKmAddrToCpuPAddr

******************************************************************************/
static IMG_PHYSADDR secure_CpuKmAddrToCpuPAddr(
    struct SYSMEM_Heap *  psHeap,
    IMG_VOID *            pvCpuKmAddr
)
{
#ifdef SYSBRG_BRIDGING
    IMG_ASSERT(0);
    return 0LL;
#else
    return (IMG_UINTPTR)pvCpuKmAddr;
#endif
}

/*!
******************************************************************************

 @Function              secure_CpuPAddrToCpuKmAddr

******************************************************************************/
static IMG_VOID * secure_CpuPAddrToCpuKmAddr(
    struct SYSMEM_Heap *  psHeap,
    IMG_PHYSADDR          paCpuPAddr
)
{
#ifdef SYSBRG_BRIDGING
    IMG_ASSERT(0);
    return IMG_NULL;
#else
    return (IMG_VOID *)((IMG_UINTPTR)paCpuPAddr);
#endif
}

/*!
******************************************************************************

 @Function              secure_UpdateMemory

******************************************************************************/
static IMG_VOID secure_UpdateMemory(
    struct SYSMEM_Heap *    psHeap,
    IMG_HANDLE              hPagesHandle,
    SYSMEM_UpdateDirection  eDir)
{
    return;
}

static int secure_map_user(SYSMEMU_sPages *psPages, IMG_VOID *priv)
{
    IMG_ASSERT(!"not allowed to map secure pages into host (user)");
    return -EFAULT;
}

static SYSMEM_Ops sSecureMemOps = {

    secure_InitialiseMemory,
    secure_DeInitialiseMemory,

    // allocation/free
    secure_ion_AllocatePages,
    secure_ion_FreePages,

    // import
    secure_ion_ImportPages,
    IMG_NULL,

    // translation
    secure_GetCpuKmAddr,

    secure_CpuKmAddrToCpuPAddr,
    secure_CpuPAddrToCpuKmAddr,

    // maintenance
    secure_UpdateMemory,
    IMG_NULL,

    secure_map_user,
    secure_ion_GetFileDescriptor
};

/*!
******************************************************************************

 @Function              SECURE_AddSecureMemory

******************************************************************************/
IMG_RESULT SECURE_AddSecureMemory(
    SYSDEVU_sInfo * psSysDevInfo,
    SYS_eMemPool  * peMemPool
)
{
    IMG_RESULT ui32Result;

    ui32Result = SYSMEMU_AddMemoryHeap(&sSecureMemOps, psSysDevInfo, IMG_TRUE, IMG_NULL, peMemPool);
    IMG_ASSERT(IMG_SUCCESS == ui32Result);

    return ui32Result;
}
