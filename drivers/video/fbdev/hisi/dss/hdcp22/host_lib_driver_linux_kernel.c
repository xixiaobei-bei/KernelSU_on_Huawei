/*-----------------------------------------------------------------------
// Copyright (c) 2017, Hisilicon Tech. Co., Ltd. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//-----------------------------------------------------------------------
//
// Project:
//
// Host Library.
//
// Description:
//
// Sample Linux kernel driver.
//
//-----------------------------------------------------------------------*/

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/netlink.h>
#include <linux/proc_fs.h>
#include <linux/moduleparam.h>
#include <linux/random.h>
#include <linux/delay.h>
#include "host_lib_driver_linux_if.h"
#include "../hisi_fb_def.h"
#include "hisi_dp.h"
#include <linux/hisi/hisi_drmdriver.h>
#include <global_ddr_map.h>
#include "hdcp13.h"


/**
 * \file
 * \ingroup HL_Driver_Kernel
 * \brief Sample Linux Host Library Driver
 * \copydoc HL_Driver_Kernel
 */

/**
 * \defgroup HL_Driver_Linux Sample Linux Host Library Driver
 * \ingroup HL_Driver
 * \brief Sample code for the Linux Host Library Driver.
 * The Linux Host Library Driver is composed of 2 parts:
 * 1. A kernel driver.
 * 2. A file access instance.
 *
 * The kernel driver is the kernel executable code enabling the firmware to execute.
 * It provides the access to the hardware register to interact with the firmware.
 *
 * The file access instance initializes the #hl_driver_t structure for the
 * host library access.  The Host Library references the file access to request the
 * kernel operations.
 */

/**
 * \defgroup HL_Driver_Kernel Sample Linux Kernel Host Library Driver
 * \ingroup HL_Driver_Linux
 * \brief Example code for the Linux Kernel Host Library Driver.
 *
 * The Sample Linux Kernel Driver operates on the linux kernel.
 * To install (requires root access):
 * \code
 insmod bin/linux_hld_module.ko verbose=0
 * \endcode
 *
 * To remove (requires root access):
 * \code
 rmmod linux_hld_module
 * \endcode
 *
 * Example Linux Host Library Code:
 * \code
 */

#define ESM_DEVICE_MAJOR   58
#define MAX_ESM_DEVICES    1//16
#define HPI_REG_SIZE    0x100
#define HPI_ADDRESS_ESM0    0xff350000
#define HDCP_FW_ADDRESS     HISI_RESERVED_DP_HDCP2_PHYMEM_BASE
#define HDCP_FW_SIZE            0x40000
#define HDCP_DATA_ADDRESS  (HISI_RESERVED_DP_HDCP2_PHYMEM_BASE + HDCP_FW_SIZE)
#define HDCP_DATA_SIZE         0x20000



/*static int randomize_mem = 0;*/

//
// ESM Device
//
typedef struct
{
	int allocated;
	int code_loaded;
	int code_is_phys_mem;
	ulong code_base;
	ulong code_size;
	uint8_t *code;
	int data_is_phys_mem;
	ulong data_base;
	ulong data_size;
	uint8_t *data;
	ulong hpi_base;
	ulong hpi_size;
	uint8_t *hpi;
	int hpi_mem_region_requested;
} esm_device;


//
// Constant strings
//
static const char *MY_TAG = "ESM HLD: ";
static const char *ESM_DEVICE_NAME = "esm";
static const char *ESM_DEVICE_CLASS = "elliptic";

//
// Linux device, class and range
//
static int device_created = 0;
static struct device *device = NULL;
static int device_range_registered = 0;
static int device_class_created = 0;
static struct class *device_class = NULL;
static int esm_en = 0;
static int esm_opened = 0;
static struct mutex g_esm_lock;
static uint32_t hdcp_state = 0;


//
// ESM devices
//
static esm_device esm_devices[MAX_ESM_DEVICES];

//
// Destroys the interface device
//
static void release_resources(esm_device *esm)
{
	if (esm->code)
	{
		iounmap(esm->code);
		esm->code = NULL;
	}

	if (esm->data)
	{
		iounmap(esm->data);
		esm->code = NULL;
	}

	if (esm->hpi)
	{
		iounmap(esm->hpi);
		esm->code = NULL;
	}

	if (esm->hpi_mem_region_requested)
	{
		release_mem_region(esm->hpi_base, esm->hpi_size);
		esm->hpi_mem_region_requested = 0;
	}
}


//---------------------------------------------------------------------------
//              Processing of the requests from the userspace
//---------------------------------------------------------------------------

//
// Loads the firmware
//
static long cmd_load_code(esm_device *esm, esm_hld_ioctl_load_code *request)
{
	long ret = HL_DRIVER_FAILED;
	esm_hld_ioctl_load_code krequest;

	memset(&krequest, 0, sizeof(esm_hld_ioctl_load_code));
	ret = copy_from_user(&krequest, request, sizeof(esm_hld_ioctl_load_code));
	if (ret) {
		HISI_FB_ERR( "copy_from_user failed!ret = %ld\n", ret);
		krequest.returned_status = ret;
		goto Exit;
	}
	krequest.returned_status = ret;
    /*

	HISI_FB_INFO( "%scmd_load_code: code=%pK code_size=0x%x\n",
						MY_TAG, krequest.code, krequest.code_size);

	if (krequest.code_size > esm->code_size)
	{
		HISI_FB_ERR( "%scmd_load_code: Code size larger than code memory (0x%x > 0x%lx).\n",
					MY_TAG, krequest.code_size, esm->code_size);
		krequest.returned_status = HL_DRIVER_NO_MEMORY;
	}
	else
	{
		if (esm->code_loaded == 1)
		{
			HISI_FB_INFO( "%scmd_load_code: Code already loaded.\n", MY_TAG);
			krequest.returned_status = HL_DRIVER_NO_ACCESS;
		}
		else
		{
			// No Endian shift
			ret = copy_from_user(esm->code, krequest.code, krequest.code_size);
			if (ret) {
				HISI_FB_ERR( "copy_from_user failed!ret = %ld\n", ret);
				krequest.returned_status = ret;
				goto Exit;
			}

			// Endian shift
			//uint offset;

			//copy_from_user(kernel_code, krequest.code, krequest.code_size);

			//// Endian shift of firmware image
			//for (offset = 0; offset < esm->code_size - 4; offset += 4)
			//{
			//   esm->code[offset + 3] = kernel_code[offset];
			//   esm->code[offset + 2] = kernel_code[offset + 1];
			//   esm->code[offset + 1] = kernel_code[offset + 2];
			//   esm->code[offset]     = kernel_code[offset + 3];
			//}

			//kfree(kernel_code);
			krequest.returned_status = HL_DRIVER_SUCCESS;

			HISI_FB_INFO( "%scmd_load_code: Done copying firmware to code memory region.\n", MY_TAG);
		}
	}
	*/

Exit:
	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_load_code));
	if (ret) {
		HISI_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
		return ret;
	}
	esm->code_loaded = krequest.returned_status == HL_DRIVER_SUCCESS;

	return 0;
}

//
// Returns the physical address of the code
//
static long cmd_get_code_phys_addr(esm_device *esm, esm_hld_ioctl_get_code_phys_addr *request)
{
	long ret = 0;
	esm_hld_ioctl_get_code_phys_addr krequest;

	memset(&krequest, 0, sizeof(esm_hld_ioctl_get_code_phys_addr));
	krequest.returned_status = HL_DRIVER_FAILED;
	/*krequest.returned_phys_addr = esm->code_base;
	krequest.returned_status = HL_DRIVER_SUCCESS;*/

	/*HISI_FB_INFO( "%scmd_get_code_phys_addr: returning code_base=0x%x\n",
						MY_TAG, krequest.returned_phys_addr);*/
	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_get_code_phys_addr));
	if (ret) {
		HISI_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
		return ret;
	}
	return 0;
}

//
// Returns the physical address of the data
//
static long cmd_get_data_phys_addr(esm_device *esm, esm_hld_ioctl_get_data_phys_addr *request)
{
	long ret = 0;
	esm_hld_ioctl_get_data_phys_addr krequest;

	memset(&krequest, 0, sizeof(esm_hld_ioctl_get_data_phys_addr));
	krequest.returned_status = HL_DRIVER_FAILED;
	/*krequest.returned_phys_addr = esm->data_base;
	krequest.returned_status = HL_DRIVER_SUCCESS;*/

	/*HISI_FB_INFO( "%scmd_get_data_phys_addr: returning data_base=0x%x\n",
						MY_TAG, krequest.returned_phys_addr);*/

	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_get_data_phys_addr));
	if (ret) {
		HISI_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
		return ret;
	}

	return 0;
}

//
// Returns the size of the data memory region
//
static long cmd_get_data_size(esm_device *esm, esm_hld_ioctl_get_data_size *request)
{
	long ret = 0;
	esm_hld_ioctl_get_data_size krequest;

	memset(&krequest, 0, sizeof(esm_hld_ioctl_get_data_size));
	krequest.returned_status = HL_DRIVER_FAILED;
	/*krequest.returned_data_size = esm->data_size;
	krequest.returned_status = HL_DRIVER_SUCCESS;*/

	HISI_FB_INFO( "%scmd_get_data_size: returning data_size=0x%x\n",
					MY_TAG, krequest.returned_data_size);

	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_get_data_size));
	if (ret) {
		HISI_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
		return ret;
	}

   return 0;
}

//
// Reads a single 32-bit HPI register
//
static long cmd_hpi_read(esm_device *esm, esm_hld_ioctl_hpi_read *request)
{
	long ret = 0;
	esm_hld_ioctl_hpi_read krequest;

	memset(&krequest, 0, sizeof(esm_hld_ioctl_hpi_read));
	ret = copy_from_user(&krequest, request, sizeof(esm_hld_ioctl_hpi_read));
	if (ret) {
		HISI_FB_ERR( "copy_from_user failed!ret = %ld\n", ret);
		krequest.returned_status = ret;
		goto Exit;
	}

	HISI_FB_INFO( "%scmd_hpi_read: Reading register at offset 0x%x\n",
						MY_TAG, krequest.offset);

	if ((!esm_en) || (!esm_opened))
	{
		HISI_FB_ERR( "DP power down, no access hdcp register\n");
		krequest.returned_status = HL_DRIVER_NO_ACCESS;
		goto Exit;
	}

	if ((esm->hpi) && (krequest.offset<=(esm->hpi_size-4)))
	{
		//HISI_FB_INFO("%scmd_hpi_read: esm->hpi = %pK\n", MY_TAG, esm->hpi);
		krequest.returned_data = ioread32(esm->hpi + krequest.offset);
		krequest.returned_status = HL_DRIVER_SUCCESS;
		//HISI_FB_INFO("%scmd_hpi_reading: ioread32 over %pK\n", MY_TAG, esm->hpi);
		HISI_FB_INFO( "%scmd_hpi_read: Returning data=0x%x\n",
							MY_TAG, krequest.returned_data);
	}
	else
	{
		krequest.returned_data = 0;
		krequest.returned_status = HL_DRIVER_NO_MEMORY;
		HISI_FB_ERR( "%scmd_hpi_read: No memory.\n", MY_TAG);
	}

Exit:
	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_hpi_read));
	if (ret) {
		HISI_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
		return ret;
	}

	return 0;
}

//
// Writes a single 32-bit HPI register
//
static long cmd_hpi_write(esm_device *esm, esm_hld_ioctl_hpi_write *request)
{
	long ret = 0;
	esm_hld_ioctl_hpi_write krequest;

	memset(&krequest, 0, sizeof(esm_hld_ioctl_hpi_write));
	ret = copy_from_user(&krequest, request, sizeof(esm_hld_ioctl_hpi_write));
	if (ret) {
		HISI_FB_ERR( "copy_from_user failed!ret = %ld\n", ret);
		krequest.returned_status = ret;
		goto Exit;
	}

	HISI_FB_INFO("%scmd_hpi_write: Writing 0x%x to register at offset 0x%x\n",
						MY_TAG, krequest.data, krequest.offset);

	if ((!esm_en) || (!esm_opened))
	{
		HISI_FB_ERR( "DP power down, no access hdcp register\n");
		krequest.returned_status = HL_DRIVER_NO_ACCESS;
		goto Exit;
	}

/*#ifdef TROOT_HDCP_APP
	//  If Kill command
	//  (HL_GET_CMD_EVENT(krequest.data) == TROOT_CMD_SYSTEM_ON_EXIT_REQ))
	//
	if ((krequest.offset == 0x38) &&
	((krequest.data & 0x000000ff) == 0x08))
	{
		esm->code_loaded = 0;
	}
#endif*/

	if ( (esm->hpi) && (krequest.offset<=(esm->hpi_size-4)) )
	{
		//HISI_FB_INFO("%scmd_hpi_write: esm->hpi = %pK\n", MY_TAG, esm->hpi);
		iowrite32(krequest.data, esm->hpi + krequest.offset);
		//HISI_FB_INFO("%scmd_hpi_write: iowrite32 over %pK\n", MY_TAG, esm->hpi);
		krequest.returned_status = HL_DRIVER_SUCCESS;

		HISI_FB_INFO( "%scmd_hpi_write: Wrote 0x%x to register at offset 0x%x\n",
							MY_TAG, krequest.data, krequest.offset);
	}
	else
	{
		krequest.returned_status = HL_DRIVER_NO_MEMORY;
		HISI_FB_ERR( "%scmd_hpi_write: No memory.\n", MY_TAG);
	}

Exit:
	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_hpi_write));
	if (ret) {
		HISI_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
		return ret;
	}

	//HISI_FB_INFO("%scmd_hpi_write: over\n",	MY_TAG);
	return 0;
}

//
// Reads from a region of the data memory
//
static long cmd_data_read(esm_device *esm, esm_hld_ioctl_data_read *request)
{
	long ret = 0;
	esm_hld_ioctl_data_read krequest;

	memset(&krequest, 0, sizeof(esm_hld_ioctl_data_read));
	ret = copy_from_user(&krequest, request, sizeof(esm_hld_ioctl_data_read));
	if (ret) {
		HISI_FB_ERR( "copy_from_user failed!ret = %ld\n", ret);
		krequest.returned_status = ret;
		goto Exit;
	}

	HISI_FB_INFO( "%scmd_data_read: Reading %u bytes from data memory at offset offset 0x%x\n",
						MY_TAG, krequest.nbytes, krequest.offset);

	if ((!esm_en) || (!esm_opened))
	{
		HISI_FB_ERR( "DP power down, no access hdcp register\n");
		krequest.returned_status = HL_DRIVER_NO_ACCESS;
		goto Exit;
	}

	if ( (esm->data) && (krequest.offset < esm->data_size) )
	{
		if ( krequest.nbytes > (esm->data_size - krequest.offset ))
		{
			krequest.returned_status = HL_DRIVER_INVALID_PARAM;
			HISI_FB_ERR( "%scmd_data_read: Invalid offset and size.\n", MY_TAG);
		}
		else
		{
			ret = copy_to_user(krequest.dest_buf, esm->data + krequest.offset, krequest.nbytes);
			if (ret) {
				HISI_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
				krequest.returned_status = ret;
				goto Exit;
			}

			krequest.returned_status = HL_DRIVER_SUCCESS;

			HISI_FB_INFO( "%scmd_data_read: Done reading %u bytes from data memory at offset 0x%x\n",
								MY_TAG, krequest.nbytes, krequest.offset);
		}
	}
	else
	{
		krequest.returned_status = HL_DRIVER_NO_MEMORY;
		HISI_FB_ERR( "%scmd_data_read: No memory.\n", MY_TAG);
	}

Exit:
	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_data_read));
	if (ret) {
		HISI_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
		return ret;
	}

	return 0;
}

//
// Writes to a region of the data memory
//
static long cmd_data_write(esm_device *esm, esm_hld_ioctl_data_write *request)
{
	long ret = 0;
	esm_hld_ioctl_data_write krequest;

	memset(&krequest, 0, sizeof(esm_hld_ioctl_data_write));
	ret = copy_from_user(&krequest, request, sizeof(esm_hld_ioctl_data_write));
	if (ret) {
		HISI_FB_ERR( "copy_from_user failed!ret = %ld\n", ret);
		krequest.returned_status = ret;
		goto Exit;
	}

	HISI_FB_INFO( "%scmd_data_write: Writing %u bytes to data memory at offset 0x%x\n",
						MY_TAG, krequest.nbytes, krequest.offset);

	if ((!esm_en) || (!esm_opened))
	{
		HISI_FB_ERR( "DP power down, no access hdcp register\n");
		krequest.returned_status = HL_DRIVER_NO_ACCESS;
		goto Exit;
	}

	if ( (esm->data) && (krequest.offset < esm->data_size) )
	{
		if ( krequest.nbytes > (esm->data_size -krequest.offset))
		{
			krequest.returned_status = HL_DRIVER_INVALID_PARAM;
			HISI_FB_ERR( "%scmd_data_write: Invalid offset and size.\n", MY_TAG);
		}
		else
		{
			ret = copy_from_user(esm->data + krequest.offset, krequest.src_buf, krequest.nbytes);
			if (ret) {
				HISI_FB_ERR( "copy_from_user failed!ret = %ld\n", ret);
				krequest.returned_status = ret;
				goto Exit;
			}
			krequest.returned_status = HL_DRIVER_SUCCESS;

			HISI_FB_INFO( "%scmd_data_write: Done writing %u bytes to data memory at offset 0x%x\n",
								MY_TAG, krequest.nbytes, krequest.offset);
		}
	}
	else
	{
		krequest.returned_status = HL_DRIVER_NO_MEMORY;
		HISI_FB_ERR( "%scmd_data_write: No memory.\n", MY_TAG);
	}

Exit:
	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_data_write));
	if (ret) {
		HISI_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
		return ret;
	}

	return 0;
}

//
// Sets a region of the data memory to a given 8-bit value
//
static long cmd_data_set(esm_device *esm, esm_hld_ioctl_data_set *request)
{
	long ret = 0;
	esm_hld_ioctl_data_set krequest;

	memset(&krequest, 0, sizeof(esm_hld_ioctl_data_set));
	ret = copy_from_user(&krequest, request, sizeof(esm_hld_ioctl_data_set));
	if (ret) {
		HISI_FB_ERR( "copy_from_user failed!ret = %ld\n", ret);
		krequest.returned_status = ret;
		goto Exit;
	}

	HISI_FB_INFO( "%scmd_data_set: Setting %u bytes (data=0x%x) of data memory from offset 0x%x\n",
						MY_TAG, krequest.nbytes, krequest.data, krequest.offset);

	if ((!esm_en) || (!esm_opened))
	{
		HISI_FB_ERR( "DP power down, no access hdcp register\n");
		krequest.returned_status = HL_DRIVER_NO_ACCESS;
		goto Exit;
	}

	if ( (esm->data) && (krequest.offset < esm->data_size) )
	{
		if ( krequest.nbytes > (esm->data_size -krequest.offset))
		{
			krequest.returned_status = HL_DRIVER_INVALID_PARAM;
			HISI_FB_ERR( "%scmd_data_set: Invalid offset and size.\n", MY_TAG);
		}
		else
		{
			memset(esm->data + krequest.offset, krequest.data, krequest.nbytes);
			krequest.returned_status = HL_DRIVER_SUCCESS;

			HISI_FB_INFO( "%scmd_data_set: Done setting %u bytes (data=0x%x) of data memory from " \
				   				"offset 0x%x\n", MY_TAG, krequest.nbytes, krequest.data, krequest.offset);
		}
	}
	else
	{
		krequest.returned_status = HL_DRIVER_NO_MEMORY;
		HISI_FB_ERR( "%scmd_data_set: No memory.\n", MY_TAG);
	}

Exit:
	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_data_set));
	if (ret) {
		HISI_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
		return ret;
	}

	return 0;
}

//
// Opens an ESM device. Associates a device file to an ESM device.
//
static long cmd_esm_open(struct file *f, esm_hld_ioctl_esm_open *request)
{
	int i;
	esm_device *esm = esm_devices;
	int ret_val = HL_DRIVER_SUCCESS;
	esm_hld_ioctl_esm_open krequest;
	long ret = 0;

	memset(&krequest, 0, sizeof(esm_hld_ioctl_esm_open));
	ret = copy_from_user(&krequest, request, sizeof(esm_hld_ioctl_esm_open));
	if (ret) {
		HISI_FB_ERR( "copy_from_user failed!ret = %ld\n", ret);
		krequest.returned_status = ret;
		goto Exit;
	}

	f->private_data = NULL;

	// Look for a matching ESM device (based on HPI address)
	for (i = 0; i<MAX_ESM_DEVICES; i++)
	{
		if (esm->allocated && (krequest.hpi_base == esm->hpi_base))
		{
			// Found it
			f->private_data = esm;
			break;
		}
		esm++;
	}

	if (!f->private_data)
	{
		// Not found. Allocate a new ESM device.
		esm = esm_devices;

		for (i = 0; i<MAX_ESM_DEVICES; i++)
		{
			if (!esm->allocated)
			{
				//dma_addr_t dh=0;
				char region_name[20] = "ESM-FF350000";	//use fixed value for sc check

				esm->allocated = 1;
				esm->hpi_base  = HPI_ADDRESS_ESM0; //krequest.hpi_base;
				esm->hpi_size  = HPI_REG_SIZE; // this can be static since the HPI interface will not change
				esm->code_base = HDCP_FW_ADDRESS;//krequest.code_base;
				esm->code_size = HDCP_FW_SIZE; //krequest.code_size;
				esm->data_base = HDCP_DATA_ADDRESS; //krequest.data_base;
				esm->data_size = HDCP_DATA_SIZE; //krequest.data_size;

				/*HISI_FB_INFO( "%sNew ESM device:\n\n", MY_TAG);
				HISI_FB_INFO( "    hpi_base: 0x%lx\n",   esm->hpi_base);
				HISI_FB_INFO( "    hpi_size: 0x%lx\n",   esm->hpi_size);
				HISI_FB_INFO( "   code_base: 0x%lx\n",   esm->code_base);
				HISI_FB_INFO( "   code_size: 0x%lx\n",   esm->code_size);
				HISI_FB_INFO( "   data_base: 0x%lx\n",   esm->data_base);
				HISI_FB_INFO( "   data_size: 0x%lx\n\n", esm->data_size);*/
				//
				// Initialize the code memory
				//
				if (esm->code_base != HL_DRIVER_ALLOCATE_DYNAMIC_MEM)
				{
					esm->code_is_phys_mem = 1;
					//esm->code = phys_to_virt(esm->code_base);
					esm->code = ioremap_wc(esm->code_base, esm->code_size);
					//HISI_FB_INFO( "Code is at virtual address 0x%lx\n", (ulong)(esm->code));
					if (!esm->code)
					{
						ret_val = HL_DRIVER_NO_MEMORY;
						goto ErrorExit;
					}
				}
				/*else
				{
					esm->code = dma_alloc_coherent(device, esm->code_size, &dh, GFP_KERNEL);

					if (!esm->code)
					{
						HISI_FB_ERR( "%sFailed to allocate code DMA region (%ld bytes)\n",
									MY_TAG, esm->code_size);
						ret_val = HL_DRIVER_NO_MEMORY;
						goto ErrorExit;
					}

					esm->code_base = dh;
					HISI_FB_INFO( "%sBase address of allocated code region: phys=0x%lx virt=%pK\n",
									MY_TAG, esm->code_base, esm->code);
				}*/

				//
				// Initialize the data memory
				//
				if (esm->data_base != HL_DRIVER_ALLOCATE_DYNAMIC_MEM)
				{
					esm->data_is_phys_mem = 1;
					//esm->data = phys_to_virt(esm->data_base);
					esm->data = ioremap_wc(esm->data_base, esm->data_size);
					//HISI_FB_INFO( "Data is at virtual address 0x%lx\n", (ulong)(esm->data));
					if (!esm->data)
					{
						ret_val = HL_DRIVER_NO_MEMORY;
						goto ErrorExit;
					}
				}
				/*else
				{
					esm->data = dma_alloc_coherent(device, esm->data_size, &dh, GFP_KERNEL);

					if (!esm->data)
					{
						HISI_FB_ERR( "%sFailed to allocate data DMA region (%ld bytes)\n",
									MY_TAG, esm->data_size);
						ret_val = HL_DRIVER_NO_MEMORY;
						break;
					}

					esm->data_base = dh;
					HISI_FB_INFO( "%sBase address of allocated data region: phys=0x%lx virt=%pK\n",
									MY_TAG, esm->data_base, esm->data);
				}*/

				/*if (randomize_mem)
				{
					prandom_bytes(esm->code, esm->code_size);
					prandom_bytes(esm->data, esm->data_size);
				}*/

				//
				// Init HPI access
				//

				//sprintf(region_name, "ESM-%lX", esm->hpi_base);		//dangrous function
				request_mem_region(esm->hpi_base, esm->hpi_size, region_name);
				//HISI_FB_INFO("%s request_mem_region over\n", MY_TAG);
				esm->hpi_mem_region_requested = 1;
				esm->hpi = ioremap_nocache(esm->hpi_base, esm->hpi_size);
				HISI_FB_INFO("%s ioremap_nocache over; esm->hpi =%pK \n", MY_TAG, esm->hpi);
				if (!esm->hpi)
				{
					ret_val = HL_DRIVER_NO_MEMORY;
					goto ErrorExit;
				}

				// Associate the Linux file to the ESM device
				f->private_data = esm;
				break;
			}
			esm++;
		}
	}

	if (!f->private_data)
	{
		HISI_FB_ERR( "%scmd_esm_open: Too many ESM devices.\n", MY_TAG);
		ret_val = HL_DRIVER_TOO_MANY_ESM_DEVICES;
		goto Exit;
	}

	esm_opened = 1;
	goto Exit;

ErrorExit:
	release_resources(esm);

Exit:
	krequest.returned_status = ret_val;
	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_esm_open));
	if (ret) {
		HISI_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
		return ret;
	}

	return 0;
}

static long cmd_esm_start(esm_device *esm, esm_hld_ioctl_esm_start *request)
{
	long ret = 0;
	esm_hld_ioctl_esm_start krequest;

	memset(&krequest, 0, sizeof(esm_hld_ioctl_esm_start));
	ret = copy_from_user(&krequest, request, sizeof(esm_hld_ioctl_esm_start));
	if (ret) {
		HISI_FB_ERR( "copy_from_user failed!ret = %ld\n", ret);
		krequest.returned_status = ret;
		goto Exit;
	}

	if (!esm_en)
	{
		HISI_FB_ERR( "DP power down, no access hdcp register\n");
		krequest.returned_status = HL_DRIVER_NO_ACCESS;
		goto Exit;
	}

	switch (krequest.type)
	{
		case 0:
			//set I_px_gpio_in[0] when boot up esm
			HISI_FB_INFO("%s start esm!\n", MY_TAG);
			g_hdcp_mode = 2;
			krequest.returned_status = atfd_hisi_service_access_register_smc(ACCESS_REGISTER_FN_MAIN_ID_HDCP,
										DSS_HDCP22_ENABLE, 1, (u64)ACCESS_REGISTER_FN_SUB_ID_HDCP_CTRL);
			break;
		case 1:
			HISI_FB_INFO("%s set HDCP1.3 (%d)\n", MY_TAG, krequest.value);
			if(krequest.value == 2)
			{
			    link_error_count=0;
			    HDCP_Stop_Polling_task(0);
			    g_hdcp_mode = 0;
			}
			else
			{
			    g_hdcp_mode = 3;
			}
			krequest.returned_status = atfd_hisi_service_access_register_smc(ACCESS_REGISTER_FN_MAIN_ID_HDCP,
										DSS_HDCP13_ENABLE, krequest.value, (u64)ACCESS_REGISTER_FN_SUB_ID_HDCP_CTRL);
			break;
		case 2:
			HISI_FB_INFO("%s Polling HDCP1.3 state...\n", MY_TAG);
			krequest.returned_status = HL_DRIVER_SUCCESS;
		      if(g_hdcp_mode == 3)
		      {
			    krequest.returned_status = HDCP13_WaitAuthenticationStop();
			}
			break;
		default:
			break;
	}

Exit:
	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_esm_start));
	if (ret) {
		HISI_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
		return ret;
	}

	return 0;
}

static long cmd_get_te_info(esm_device *esm, esm_hld_ioctl_get_te_info*request)
{
	long ret = 0;
	esm_hld_ioctl_get_te_info krequest;
	uint64_t temp;

	memset(&krequest, 0, sizeof(esm_hld_ioctl_get_te_info));
	ret = copy_from_user(&krequest, request, sizeof(esm_hld_ioctl_get_te_info));
	if (ret) {
		HISI_FB_ERR( "copy_from_user failed!ret = %ld\n", ret);
		krequest.returned_status = ret;
		goto Exit;
	}

	if ((!esm_en) || (!esm_opened))
	{
		HISI_FB_ERR( "DP power down, no access hdcp register\n");
		krequest.returned_status = HL_DRIVER_NO_ACCESS;
		goto Exit;
	}

	switch (krequest.type)
	{
		case 0:
			//set I_px_gpio_in[0] when boot up esm
			HISI_FB_INFO("%s Get TE basic info!\n", MY_TAG);
			krequest.bcaps = g_bcaps;
			temp = (g_BKSV>>32) & 0xFFFFFFFF;
			krequest.bKSV_msb = (uint32_t)temp;
			temp = g_BKSV & 0xFFFFFFFF;
			krequest.bKSV_lsb = (uint32_t)temp;
			break;
		case 1:
			HISI_FB_INFO("%s Get SHA1 buffer!\n", MY_TAG);
			ret = HDCP_GetSHA1Buffer(krequest.sha1_buffer, &krequest.buffer_length, krequest.V_Prime);
			if (ret) {
				HISI_FB_ERR( "Get SHA1 buffer failed!ret = %ld\n", ret);
				krequest.returned_status = ret;
				goto Exit;
			}
			/*ret = copy_to_user(krequest.sha1_buffer, pSHA1buffer, length);
			if (ret) {
				HISI_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
				krequest.returned_status = ret;
				goto Exit;
			}*/
			break;
		default:
			break;
	}
	krequest.returned_status = HL_DRIVER_SUCCESS;

Exit:
	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_get_te_info));
	if (ret) {
		HISI_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
		return ret;
	}

	return 0;
}

static long cmd_set_hdcp_state(esm_device *esm, esm_hld_ioctl_state_set *request)
{
	long ret = 0;
	esm_hld_ioctl_state_set krequest;

	memset(&krequest, 0, sizeof(esm_hld_ioctl_state_set));
	ret = copy_from_user(&krequest, request, sizeof(esm_hld_ioctl_state_set));
	if (ret) {
		HISI_FB_ERR( "copy_from_user failed!ret = %ld\n", ret);
		krequest.returned_status = ret;
		goto Exit;
	}
	hdcp_state = krequest.hdcp_state;
	HISI_FB_INFO("the hdcp state is %d\n", hdcp_state);
	krequest.returned_status = 0;

Exit:
	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_state_set));
	if (ret) {
		HISI_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
		return ret;
	}

	return 0;
}

int get_hdcp_state(uint32_t *state)
{
	if(state == NULL)
		return -1;
	*state = hdcp_state;
	return 0;
}



//---------------------------------------------------------------------------
//                                Linux Device
//---------------------------------------------------------------------------

//
// The device has been opened
//
static int device_open(struct inode *inode, struct file *filp)
{
	HISI_FB_INFO( "%sDevice opened.\n", MY_TAG);

	//
	// No associated ESM device yet.
	// Use IOCTL ESM_HLD_IOCTL_ESM_OPEN to associate an ESM to the opened device file.
	//
	filp->private_data = NULL;

	return 0;
}

//
// The device has been closed
//
static int device_release(struct inode *inode, struct file *filp)
{
	HISI_FB_INFO( "%sDevice released.\n", MY_TAG);

	return 0;
}

//
// IOCTL operation on the device
//
static long useless_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	long ret = HL_DRIVER_FAILED;

	if ((f==0) || (arg ==0) || (!f->private_data))
		return ret;

	switch (cmd)
	{
		case ESM_HLD_IOCTL_LOAD_CODE:
			ret = cmd_load_code((esm_device *)f->private_data,
		                  (esm_hld_ioctl_load_code *)arg);
			break;
		case ESM_HLD_IOCTL_GET_CODE_PHYS_ADDR:
			ret = cmd_get_code_phys_addr((esm_device *)f->private_data,
		                           (esm_hld_ioctl_get_code_phys_addr *)arg);
			break;
		case ESM_HLD_IOCTL_GET_DATA_PHYS_ADDR:
			ret = cmd_get_data_phys_addr((esm_device *)f->private_data,
		                           (esm_hld_ioctl_get_data_phys_addr *)arg);
			break;
		case ESM_HLD_IOCTL_GET_DATA_SIZE:
			ret = cmd_get_data_size((esm_device *)f->private_data,
		                      (esm_hld_ioctl_get_data_size *)arg);
			break;
		default:
			HISI_FB_ERR( "%sUnknown IOCTL request %d.\n", MY_TAG, cmd);
			break;
	}

	return ret;
}
static long device_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	long ret = HL_DRIVER_FAILED;

	if ((f==0) || (arg==0))
		return ret;
	if ((cmd != ESM_HLD_IOCTL_ESM_OPEN) && (!f->private_data))
		return ret;

	//HISI_FB_INFO( "%sdevice_ioctl in cmd = %d .\n", MY_TAG, cmd);
	mutex_lock(&g_esm_lock);

	switch (cmd)
	{
		/*case ESM_HLD_IOCTL_LOAD_CODE:
			ret = cmd_load_code((esm_device *)f->private_data,
		                  (esm_hld_ioctl_load_code *)arg);
			break;
		case ESM_HLD_IOCTL_GET_CODE_PHYS_ADDR:
			ret = cmd_get_code_phys_addr((esm_device *)f->private_data,
		                           (esm_hld_ioctl_get_code_phys_addr *)arg);
			break;
		case ESM_HLD_IOCTL_GET_DATA_PHYS_ADDR:
			ret = cmd_get_data_phys_addr((esm_device *)f->private_data,
		                           (esm_hld_ioctl_get_data_phys_addr *)arg);
			break;
		case ESM_HLD_IOCTL_GET_DATA_SIZE:
			ret = cmd_get_data_size((esm_device *)f->private_data,
		                      (esm_hld_ioctl_get_data_size *)arg);
			break;*/
		case ESM_HLD_IOCTL_HPI_READ:
			ret = cmd_hpi_read((esm_device *)f->private_data,
		                 (esm_hld_ioctl_hpi_read *)arg);
			break;
		case ESM_HLD_IOCTL_HPI_WRITE:
			ret = cmd_hpi_write((esm_device *)f->private_data,
		                  (esm_hld_ioctl_hpi_write *)arg);
			break;
		case ESM_HLD_IOCTL_DATA_READ:
			ret = cmd_data_read((esm_device *)f->private_data,
		                  (esm_hld_ioctl_data_read *)arg);
			break;
		case ESM_HLD_IOCTL_DATA_WRITE:
			ret = cmd_data_write((esm_device *)f->private_data,
		                   (esm_hld_ioctl_data_write *)arg);
			break;
		case ESM_HLD_IOCTL_DATA_SET:
			ret = cmd_data_set((esm_device *)f->private_data,
		                 (esm_hld_ioctl_data_set *)arg);
			break;
		case ESM_HLD_IOCTL_ESM_OPEN:
			ret = cmd_esm_open(f, (esm_hld_ioctl_esm_open *)arg);
			break;
		case ESM_HLD_IOCTL_ESM_START:
			ret = cmd_esm_start((esm_device *)f->private_data, (esm_hld_ioctl_esm_start *)arg);
			break;
		case ESM_HLD_IOCTL_GET_TE_INFO:
			ret = cmd_get_te_info((esm_device *)f->private_data, (esm_hld_ioctl_get_te_info*)arg);
			break;
		case ESM_HLD_IOCTL_STATE_SET:
			ret = cmd_set_hdcp_state((esm_device *)f->private_data, (esm_hld_ioctl_state_set*)arg);
			break;
		default:
			ret = useless_ioctl(f, cmd, arg);
			break;
	}
	mutex_unlock(&g_esm_lock);

	return ret;
}

//
// Creates the device required to interface with the HLD driver
//
static int create_device(void)
{
	HISI_FB_INFO( "%sCreating device '%s'...\n", MY_TAG, ESM_DEVICE_NAME);

	device = device_create(device_class, NULL, MKDEV(ESM_DEVICE_MAJOR, 0), NULL, ESM_DEVICE_NAME);

	if (IS_ERR(device))
	{
		HISI_FB_ERR( "%sFailed to create device '%s'.\n", MY_TAG, ESM_DEVICE_NAME);
		return PTR_ERR(device);
	}

	device_created = 1;
	HISI_FB_INFO( "%sDevice '%s' has been created.\n", MY_TAG, ESM_DEVICE_NAME);

	return 0;
}

static void end_device(void)
{
	int i;
	esm_device *esm = esm_devices;

	if (device_created)
	{
		HISI_FB_INFO( "%sDeleting device '%s'...\n", MY_TAG, ESM_DEVICE_NAME);
		device_destroy(device_class, MKDEV(ESM_DEVICE_MAJOR, 0));
		device_created = 0;
	}

	for (i = 0; i<MAX_ESM_DEVICES; i++)
	{
		if (esm->allocated)
		{
			if (esm->code && !esm->code_is_phys_mem)
			{
				dma_addr_t dh = (dma_addr_t)esm->code_base;
				dma_free_coherent(NULL, esm->code_size, esm->code, dh);
			}

			if (esm->data && !esm->data_is_phys_mem)
			{
				dma_addr_t dh = (dma_addr_t)esm->data_base;
				dma_free_coherent(NULL, esm->data_size, esm->data, dh);
			}

			release_resources(esm);
		}
		esm++;
	}

	memset(esm_devices, 0, sizeof(esm_devices));
}

//---------------------------------------------------------------------------
//                       Linux device class and range
//---------------------------------------------------------------------------

//
// Table of the supported operations on ESM devices
//
static const struct file_operations device_file_operations =
{
	.open = device_open,
	.release = device_release,
	.unlocked_ioctl = device_ioctl,
	.owner = THIS_MODULE,
};

static int register_device_range(void)
{
	int ret=0;

	HISI_FB_INFO( "%sRegistering device range '%s'...\n", MY_TAG, ESM_DEVICE_NAME);

	ret = register_chrdev(ESM_DEVICE_MAJOR, ESM_DEVICE_NAME, &device_file_operations);

	if (ret < 0)
	{
		HISI_FB_ERR( "%sFailed to register device range '%s'.\n", MY_TAG, ESM_DEVICE_NAME);
		return ret;
	}

	HISI_FB_INFO( "%sDevice range '%s' has been registered.\n", MY_TAG, ESM_DEVICE_NAME);
	device_range_registered = 1;

	return 0;
}

static void unregister_device_range(void)
{
	if (device_range_registered)
	{
		HISI_FB_INFO( "%sUnregistering device range '%s'...\n",
		 				MY_TAG, ESM_DEVICE_NAME);
		unregister_chrdev(ESM_DEVICE_MAJOR, ESM_DEVICE_NAME);
		device_range_registered = 0;
	}
}

//
// Creates the interface device class.
//
// Note: Attributes could be created for that class.
//       Not required at this time.
//
static int create_device_class(void)
{
	HISI_FB_INFO( "%sCreating class /sys/class/%s...\n",
					MY_TAG, ESM_DEVICE_CLASS);

	device_class = class_create(THIS_MODULE, ESM_DEVICE_CLASS);

	if (IS_ERR(device_class))
	{
		HISI_FB_ERR( "%sFailed to create device class /sys/class/%s.\n",
		 			MY_TAG, ESM_DEVICE_CLASS);
		return PTR_ERR(device_class);
	}

	device_class_created = 1;
	HISI_FB_INFO( "%sThe class /sys/class/%s has been created.\n",
					MY_TAG, ESM_DEVICE_CLASS);

	return 0;
}

//
// Ends the device class of the ESM devices
//
static void end_device_class(void)
{
	if (device_class_created)
	{
		HISI_FB_INFO( "%sDeleting the device class /sys/class/%s...\n",
						MY_TAG, ESM_DEVICE_CLASS);
		class_destroy(device_class);
		device_class_created = 0;
	}
}
//---------------------------------------------------------------------------
//              Initialization/termination of the module
//---------------------------------------------------------------------------

static int __init hld_init(void)
{
	HISI_FB_INFO( "%sInitializing...\n", MY_TAG);

	mutex_init(&g_esm_lock);
	memset(esm_devices, 0, sizeof(esm_devices));

	if ((register_device_range() == 0) &&
	     (create_device_class() == 0) &&
	     (create_device() == 0))
	{
		HISI_FB_INFO( "%sDone initializing the HLD driver.\n", MY_TAG);
	}
	else
	{
		HISI_FB_ERR( "%sFailed to initialize the HLD driver.\n", MY_TAG);
	}

	return 0;
}

static void __exit hld_exit(void)
{
	HISI_FB_INFO( "%sExiting...\n", MY_TAG);
	mutex_destroy(&g_esm_lock);
	end_device();
	end_device_class();
	unregister_device_range();
	HISI_FB_INFO( "%sDone.\n", MY_TAG);
}

void esm_driver_enable(int en)
{
	esm_en = en;
}

module_init(hld_init);
module_exit(hld_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hisilicon Tech. Co., Ltd.");
MODULE_DESCRIPTION("ESM Linux Host Library Driver");

/*
module_param(randomize_mem, int, 0644);
MODULE_PARM_DESC(randomize_mem, "Randomize memory (1).");
*/
/**
* \endcode
* @}
*/
