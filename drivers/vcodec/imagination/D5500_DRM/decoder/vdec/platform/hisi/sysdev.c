/*!
 *****************************************************************************
 *
 * @file       memmapped-linux-arm-goldfish.c
 *
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
 * as set out in the file called GPLHEADER included in this distribution. If
 * you do not delete the provisions above, a recipient may use your version of
 * this file under the terms of either the MIT license or GPL.
 *
 * This License is also included in this distribution in the file called
 * "MIT_COPYING".
 *
 *****************************************************************************/
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
#include <sysdev_utils.h>
#include <linux/kobject.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <asm/page.h>
#include <linux/mm.h>
#include <linux/version.h>
#include <linux/module.h>
#include <img_defs.h>
#include "target.h"
#include <sysenv_api_km.h>
#include <system.h>
#include <sysmem_utils.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
#include <linux/export.h>
#endif
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#define REGULTOR_OPEN 1
#include <linux/firmware.h>
#ifdef VDEC_USE_PVDEC_SEC
#include "sysmem_secure.h"
extern void* regio_addr;
#endif
static int module_irq = -1;

void *vdec_reg_vaddr;
IMG_PHYSADDR vdec_reg_paddr = 0;
unsigned int vdec_reg_size = 0;
#define REG_QOS 0xE893000C
#define REG_BW  0xE8930010
unsigned long * viraddr_qos = NULL;
unsigned long * viraddr_bw = NULL;
int dec_outstanding_qos = 0;
int dec_outstanding_bw = 0;
int gIsFpga = 0;
unsigned long  dec_clk_rate = 0;
//Device information
static IMG_CHAR *   gpszDevName = IMG_NULL;
static SYSDEVU_sInfo *  psSysDev = IMG_NULL;
static IMG_CHAR * g_fw_buf = NULL;
#define FW_BUF_LEN    265*1024

/*
*****************************
*/
static struct list_head fw_list;

struct endpoint_fw {
    struct firmware *fw;
    char *name;
    struct list_head head;
};

/*
*****************************
*/

struct clk *gvdec_clk = NULL;
static IMG_BOOL gbDevDetected = IMG_FALSE;
struct regulator *gvdec_regulator;
#if 0
#define PERI_CRG (0xFFF35000)
#define SCTRL (0xFFF0A000)
#define PMC (0xFFF31000)

void power_on(int isDec)
{
    unsigned int val =  0;
    unsigned int val2 =  0;
    unsigned int val3 =  0;
    writel(0xe0000060,(ioremap(PERI_CRG,4) + 0x00));/*外设时钟使能*/
    writel(0xe018,(ioremap(PERI_CRG,4) + 0x64));

    //val = readl(ioremap(PMC,4) + 0x380);
    //val &= (~(0x1<<4));
    //writel(val,(ioremap(PMC,4) + 0x380));/*配置VCODEC_SUBSYS退出低功耗模式*/
	writel(0x00100000,(ioremap(PMC,4) + 0x380));
    do
    {
        val = readl(ioremap(PMC,4) + 0x384);
        val &= (0x01<<4);
    }while(val);              /*查询退出请求是否被响应*/
    do
    {
        val = readl(ioremap(PMC,4) + 0x388);
        val &= (0x01<<4);
    }while(val);         /*确认VCODEC_SUBSYS已退出低功耗模式*/

    if(isDec)
    {
        writel(0x04,(ioremap(PERI_CRG,4) + 0x150));/*MTCMOS使能*/
        writel(0x800,(ioremap(PERI_CRG,4) + 0x30));/*开启VENC和NOC_VENC时钟*/
        writel(0x04,(ioremap(PERI_CRG,4) + 0x148));/*ISO去使能*/
        writel(0x200,(ioremap(PERI_CRG,4) + 0x88));/*解复位*/

        val = readl(ioremap(PMC,4) + 0x380);
        val &= (~(0x1<<10));
	    //writel(val,(ioremap(PMC,4) + 0x380));/*配置NOC VDEC退出低功耗模式*/
		writel(0x04000000,(ioremap(PMC,4) + 0x380));
        do
        {
            val = readl(ioremap(PMC,4) + 0x384);
            val &= (0x01<<10);
        }while(val);              //查询请求是否被响应
        do
        {
            val = readl(ioremap(PMC,4) + 0x388);
            val &= (0x01<<10);
        }while(val);         /*轮询PMC中NOC_POWER_IDLEACK（0x384） bit 10和NOC_POWER_IDLE（0x388） bit 10都为1’b0*/

    }
    else
    {
        writel(0x02,(ioremap(PERI_CRG,4) + 0x150));/*MTCMOS使能*/
	    writel(0x400,(ioremap(PERI_CRG,4) + 0x30));/*开启VENC和NOC_VENC时钟*/
	    writel(0x02,(ioremap(PERI_CRG,4) + 0x148));	/*ISO去使能*/
        writel(0x100,(ioremap(PERI_CRG,4) + 0x88));/*解复位*/

        val = readl(ioremap(PMC,4) + 0x380);
        val &= (~(0x1<<11));

        writel(val,(ioremap(PMC,4) + 0x380));/*配置NOC VENC退出低功耗模式*/
        do
        {
            val = readl(ioremap(PMC,4) + 0x384);
            val &= (0x01<<11);
        }while(val);              //查询请求是否被响应

        do
        {
            val = readl(ioremap(PMC,4) + 0x388);
            val &= (0x01<<11);
        }while(val);         /*轮询PMC中NOC_POWER_IDLEACK（0x384） bit 10和NOC_POWER_IDLE（0x388） bit 10都为1’b0*/

    }

}
#endif
static int driver_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct device_node *np = dev->of_node;
    struct resource res;
    int ret = 0,err = 0;
    IMG_ASSERT(pdev->resource[0].flags == IORESOURCE_MEM);
    //IMG_ASSERT(pdev->resource[1].flags == IORESOURCE_IRQ);
    //power_on(1);
    if (NULL == np)
    {
        printk("[VDEC SYSDEV] %s the device node is null\n", __func__);
        return -1;
    }
    module_irq =  irq_of_parse_and_map(np,0);
    printk("[VDEC SYSDEV] %s, module_irq = %d\n",__func__,module_irq);

    ret = of_address_to_resource(np,0,&res);
    if (ret == 0)
    {
        vdec_reg_paddr = res.start;
        vdec_reg_size = resource_size(&res);
        printk("[VDEC SYSDEV] %s, vdec_reg_vaddr = 0x%llx, vdec_reg_size = 0x%0x\n", __func__, vdec_reg_paddr, vdec_reg_size);
    }

    vdec_reg_vaddr = of_iomap(np, 0);
    if (NULL == vdec_reg_vaddr) {
        printk(KERN_ERR "[VDEC SYSDEV] %s get reg base addr failed\n", __func__);
    }
	err = of_property_read_u32(np, "dec_outstanding_qos", &dec_outstanding_qos);
	if(err){
        printk(KERN_ERR "[VDEC SYSDEV] %s can not get dec_outstanding_qos\n", __func__);
	}
	err = of_property_read_u32(np, "dec_outstanding_bw", &dec_outstanding_bw);
	if(err){
        printk(KERN_ERR "[VDEC SYSDEV] %s can not get dec_outstanding_bw\n", __func__);
	}
	if (dec_outstanding_qos || dec_outstanding_bw)
	{
		viraddr_qos = (unsigned long *)(ioremap(REG_QOS,4));
		viraddr_bw  = (unsigned long *)(ioremap(REG_BW,4)); 
	}
	err = of_property_read_u32(np, "vdec_fpga", &gIsFpga);
    if (err) {
        gIsFpga = 0;
        printk(KERN_ERR "[VDEC SYSDEV] %s can not find vdec_fpga\n", __func__);
    }
#if REGULTOR_OPEN
    err = of_property_read_u32(np, "dec_clk_rate", &dec_clk_rate);
    if (err) {
        printk(KERN_ERR "[VDEC SYSDEV] %s can not get dec_clk_rate\n", __func__);
    }
    gvdec_clk = devm_clk_get(dev, "clk_vdec");
	if (IS_ERR(gvdec_clk)) {
        printk(KERN_ERR "[VDEC SYSDEV] %s [VXD]Couldn't get clk! \n", __func__);
	}
    ret = clk_set_rate(gvdec_clk, dec_clk_rate);
    if (ret) {
        printk(KERN_ERR "[VDEC SYSDEV] %s [DEC]Failed to clk_set_rate %s: %d\n", __func__, __func__, ret);
    }
	gvdec_regulator = devm_regulator_get(dev, "ldo_vdec");
	if (IS_ERR(gvdec_regulator)) {
        printk(KERN_ERR "[VDEC SYSDEV] %s [VXD]Couldn't get regulator! \n", __func__);
		return;
	}

#endif
    psSysDev->native_device =  (void *)&pdev->dev;
    gbDevDetected = IMG_TRUE;
    return 0;
}

static const struct of_device_id vdec_of_match[] = {
    { .compatible = "hisi,kirin950-vdec", },
    { }
};

static struct platform_driver local_driver = {
    .probe = driver_probe,
    .driver = {
        .name = "goldfish_msvxd",
        .owner = THIS_MODULE,
        .of_match_table = vdec_of_match
     },
};

/*!
******************************************************************************

@Function IsrCb

******************************************************************************/
static irqreturn_t IsrCb(int irq, void *dev_id)
{
    IMG_BOOL  bHandled;

    if ( (psSysDev != IMG_NULL) && (psSysDev->pfnDevKmLisr != IMG_NULL) )
    {
        //Call it
        SYSOSKM_DisableInt();
        bHandled = psSysDev->pfnDevKmLisr(psSysDev->pvParam);
        SYSOSKM_EnableInt();

        //If the LISR handled the interrupt
        if (bHandled)
        {
            //Signal this
            return IRQ_HANDLED;
        }
    }
    return IRQ_NONE;
}

/*!
******************************************************************************

@Function release_fw_entry

******************************************************************************/
static IMG_VOID release_fw_entry(
    struct endpoint_fw * entry
)
{
    printk(KERN_INFO "[VDEC SYSDEV] releasing firmware %s\n", entry->name);
    kfree(entry->name);
    release_firmware(entry->fw);
    list_del(&entry->head);
    kfree(entry);
}

/*!
******************************************************************************

@Function free_device

******************************************************************************/
static IMG_VOID free_device(SYSDEVU_sInfo *psInfo)
{
    struct endpoint_fw *entry, *tmp;

    if (IMG_TRUE == gbDevDetected)
    {
        platform_driver_unregister(&local_driver);
        gbDevDetected = IMG_FALSE;
    }

    if(psSysDev != psInfo)
    {
        printk(KERN_ERR "[VDEC SYSDEV] Failed to free the Device\n");
        return;
    }

    //Free device name
    if (gpszDevName != IMG_NULL)
    {
        IMG_FREE(gpszDevName);
        gpszDevName = IMG_NULL;
    }

    if(IMG_NULL != psSysDev)
    {
        free_irq(module_irq, psSysDev);
    }

    list_for_each_entry_safe(entry, tmp, &fw_list, head)
    {
        release_fw_entry(entry);
    }


    psSysDev = IMG_NULL;

    return;
}


/*!
******************************************************************************

@Function handleResume

******************************************************************************/
static IMG_VOID handle_resume(SYSDEVU_sInfo *psInfo, IMG_BOOL forAPM)
{
    int ret;
    if (!forAPM) {
        pr_info("[VDEC SYSDEV] %s +\n", __func__);
    }
#if REGULTOR_OPEN
	ret = regulator_enable(gvdec_regulator);
	if (ret != 0) {
        printk("[VDEC SYSDEV] Failed to enable %s: %d\n", __func__, ret);
		return ;
	}
#endif

	if(dec_outstanding_qos ){
	    writel(dec_outstanding_qos,viraddr_qos);
	}
	if(dec_outstanding_bw ){
		writel(dec_outstanding_bw,viraddr_bw);
	}
    if (!forAPM) {
        pr_info("[VDEC SYSDEV] %s -\n", __func__);
    }
}


/*!
******************************************************************************

@Function handleSuspend

******************************************************************************/
static IMG_VOID handle_suspend(SYSDEVU_sInfo *psInfo, IMG_BOOL forAPM)
{
    int ret;
    if (!forAPM) {
        pr_info("[VDEC SYSDEV] %s +\n", __func__);
    }
#if REGULTOR_OPEN
	ret = regulator_disable(gvdec_regulator);
	if (ret != 0) {
        printk("[VDEC SYSDEV] Failed to disable %s: %d\n", __func__, ret);
		return ;
	}
#endif

    if (!forAPM) {
        pr_info("[VDEC SYSDEV] %s -\n", __func__);
    }
}

#define GET_FW_ADDR(bGetPhyAddr, entry) \
    ((bGetPhyAddr) ? \
    /* fw buf is physically continuous mem as allocated with kmalloc(GFP_ATOMIC) */ \
    ((IMG_PUINT8)(IMG_UINTPTR)virt_to_phys((volatile void *)entry->fw->data)) : \
    ((IMG_PUINT8)entry->fw->data))

#ifdef VDEC_USE_PVDEC_SEC
/***************************************************************************
 acquire_dev_firmware
****************************************************************************/
static IMG_RESULT acquire_dev_firmware(
    SYSDEVU_sInfo * psInfo,
    IMG_CHAR      * fwName,
    IMG_BOOL        bGetPhyAddr,
    IMG_PUINT8    * ppFwBuf,
    IMG_UINT32    * pFwSize
)
{
    struct device * dev;
    struct endpoint_fw *entry, *tmp;
	

    if (!psInfo)
    {
        printk(KERN_ERR "[VDEC SYSDEV] SYSDEVU_sInfo pointer is null!\n");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    if (!psInfo->native_device)
    {
        printk(KERN_ERR "[VDEC SYSDEV] native_device pointer is null!\n");
        return IMG_ERROR_NOT_INITIALISED;
    }

    dev = (struct device *)psInfo->native_device;

    /* Check if the firmware was already acquired */
    list_for_each_entry_safe(entry, tmp, &fw_list, head)
    {
        if (!strcmp(entry->name, fwName))
        {
            *ppFwBuf = GET_FW_ADDR(bGetPhyAddr, entry);
            *pFwSize = entry->fw->size;
            return IMG_SUCCESS;
        }
    }

    printk(KERN_INFO "[VDEC SYSDEV] requesting firmware %s\n", fwName);

    /* Create a placeholder for endpoint firmware */
    entry = (struct endpoint_fw *)kmalloc(sizeof(*entry), GFP_KERNEL);
    if (entry == IMG_NULL)
    {
        printk(KERN_ERR "[VDEC SYSDEV] firmware alloc failed\n");
        return IMG_ERROR_OUT_OF_MEMORY;
    }

    entry->name = (IMG_CHAR *)kmalloc(strlen(fwName)+1, GFP_KERNEL);
    if (entry->name == IMG_NULL)
    {
        printk(KERN_ERR "[VDEC SYSDEV] firmware name alloc failed\n");
        kfree(entry);
        return IMG_ERROR_OUT_OF_MEMORY;
    }

    if (request_firmware((const struct firmware **)&entry->fw,
                                fwName, dev))
    {
        printk(KERN_ERR "[VDEC SYSDEV] failed to load firmware: %s\n", fwName);
        kfree(entry->name);
        kfree(entry);
        return IMG_ERROR_GENERIC_FAILURE;
    }

    if (NULL != g_fw_buf)
    {
        memcpy(g_fw_buf, entry->fw->data, entry->fw->size);
        *ppFwBuf = bGetPhyAddr ? virt_to_phys(g_fw_buf): g_fw_buf; 
    }
    else
    {
        *ppFwBuf = GET_FW_ADDR(bGetPhyAddr, entry);
    }
    *pFwSize = entry->fw->size;

    strcpy(entry->name, fwName);

    list_add_tail(&entry->head, &fw_list);

    return IMG_SUCCESS;
}

/*!
******************************************************************************

@Function release_dev_firmware hook

******************************************************************************/
static IMG_RESULT release_dev_firmware(
    SYSDEVU_sInfo * psInfo,
    IMG_CHAR      * fwName
)
{
    struct endpoint_fw *entry, *tmp;

    list_for_each_entry_safe(entry, tmp, &fw_list, head)
    {
        if (!strcmp(entry->name, fwName))
        {
            release_fw_entry(entry);
            return IMG_SUCCESS;
        }
    }

    return IMG_ERROR_INVALID_ID;
}

/*!
******************************************************************************

@Function attach_dev_memory hook

******************************************************************************/
static IMG_RESULT attach_dev_memory(
    SYSDEVU_sInfo * psInfo,
    IMG_UINT32 ui32MemSize,
    SYS_eMemPool *psMemPool)
{
    IMG_ASSERT(ui32MemSize != 0);

    if (*psMemPool == SYS_eMemPoolInvalid)
    {
        IMG_RESULT ui32Result;

        /* Register secure region */
        ui32Result = SECURE_AddSecureMemory(psInfo, &psInfo->secureMemPool);
        IMG_ASSERT(IMG_SUCCESS == ui32Result);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }
        // pin the memory pool
        *psMemPool = psInfo->secureMemPool;
        return IMG_SUCCESS;
    }

    return IMG_ERROR_INVALID_PARAMETERS;
}

/* !
 * ******************************************************************************
 *
 * @Function detach_dev_memory hook
 *
 * ******************************************************************************/
static IMG_RESULT detach_dev_memory(
    SYSDEVU_sInfo * psInfo,
        SYS_eMemAttrib  sMemPool
        )
{
    if (psInfo->secureMemPool == sMemPool)
    {
        IMG_RESULT ui32Result;
        // unpin the memory pool
        ui32Result = SYSMEMU_RemoveMemoryHeap(psInfo->secureMemPool);
        IMG_ASSERT(IMG_SUCCESS == ui32Result);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }
        return IMG_SUCCESS;
    }

    return IMG_ERROR_INVALID_PARAMETERS;
}
#endif

static struct SYSDEV_ops device_ops = {
		.free_device = free_device,

		.resume_device = handle_resume,
		.suspend_device = handle_suspend,
#ifdef VDEC_USE_PVDEC_SEC
                .acquire_device_firmware = acquire_dev_firmware,
                .release_device_firmware = release_dev_firmware,
                .attach_device_memory = attach_dev_memory,
                .detach_device_memory = detach_dev_memory,
#endif
};

/*!
******************************************************************************

@Function    SYSDEVU_VDECRegisterDriver

******************************************************************************/
IMG_RESULT SYSDEVU_VDECRegisterDriver(
    SYSDEVU_sInfo *  psInfo
)
{
    IMG_UINT32 ui32Result = IMG_ERROR_GENERIC_FAILURE;
    if(IMG_FALSE == gbDevDetected)
    {
        printk(KERN_INFO "[VDEC SYSDEV] %s\n", __func__);
        psSysDev = psInfo;
        ui32Result = platform_driver_register(&local_driver);
        if (ui32Result != 0)
        {
            return IMG_ERROR_DEVICE_NOT_FOUND;
        }
        //psSysDev = psInfo;
        if (request_irq(module_irq, IsrCb, 0, "goldfish_device", psSysDev))
        {
            printk(KERN_ERR "[VDEC SYSDEV] Failed to get IRQ\n");
            return IMG_ERROR_GENERIC_FAILURE;
        }

        gbDevDetected = IMG_TRUE;

        SYSDEVU_SetDevMap(psInfo, 0, 0, 0, 0, 0, 0, 0);

        SYSDEVU_SetDeviceOps(psInfo, &device_ops);

        ui32Result = SYSMEMKM_AddSystemMemory(psInfo, &psInfo->sMemPool);
        
        g_fw_buf =(IMG_CHAR *)kmalloc(FW_BUF_LEN, GFP_KERNEL);
        if (g_fw_buf == IMG_NULL)
        {
            printk(KERN_ERR "VDEC SYSDEV : malloc failed \n");
        }
 
    }

    INIT_LIST_HEAD(&fw_list);

    SECURE_REE_Init(psInfo);

    return ui32Result;
}
/*!
******************************************************************************

@Function    SYSDEVU_VDECUnRegisterDriver

******************************************************************************/
IMG_RESULT SYSDEVU_VDECUnRegisterDriver(
    SYSDEVU_sInfo *  psInfo
)
{
    SECURE_REE_DeInit();
    SYSMEMU_RemoveMemoryHeap(psInfo->sMemPool);
    psInfo->ops->free_device(psInfo);
    return IMG_SUCCESS;
}
