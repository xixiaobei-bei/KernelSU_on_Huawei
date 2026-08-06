/*
 *               (C) COPYRIGHT 2014 - 2016 SYNOPSYS, INC.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * Copyright (c) 2017 Hisilicon Tech. Co., Ltd. Integrated into the Hisilicon display system.
 */

#include "host_lib_driver_linux_if.h"
#include <linux/netlink.h>
#include <linux/random.h>
#include <linux/delay.h>
#include "hisi_dp.h"
#include <linux/hisi/hisi_drmdriver.h>
#include <global_ddr_map.h>
#include "../hdcp_common.h"

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


#define MAX_ESM_DEVICES 1
#define HPI_REG_SIZE    0x100
#define HPI_ADDRESS_ESM0     0xff350000
#define HDCP_FW_ADDRESS     HISI_RESERVED_DP_HDCP2_PHYMEM_BASE
#define HDCP_FW_SIZE         0x40000
#define HDCP_DATA_ADDRESS  (HISI_RESERVED_DP_HDCP2_PHYMEM_BASE + HDCP_FW_SIZE)
#define HDCP_DATA_SIZE       0x20000


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


//
// Linux device, class and range
//
static int esm_en = 0;
static int esm_opened = 0;


//
// ESM devices
//
static esm_device esm_devices[MAX_ESM_DEVICES];

//
// Destroys the interface device
//
static void release_resources(esm_device *esm)
{
	if(esm == NULL)
	{
		return;
	}

	if (esm->code != NULL)
	{
		iounmap(esm->code);
		esm->code = NULL;
	}

	if (esm->data != NULL)
	{
		iounmap(esm->data);
		esm->code = NULL;
	}

	if (esm->hpi != NULL)
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


// ---------------------------------------------------------------------------
//              Processing of the requests from the userspace
// ---------------------------------------------------------------------------

//
// Loads the firmware
//
static long cmd_load_code(esm_device *esm, esm_hld_ioctl_load_code *request)
{
	long ret = HL_DRIVER_FAILED;
	esm_hld_ioctl_load_code krequest;

	if((esm == NULL) || (request == NULL)){
		DPU_FB_ERR("esm or request is null pointer\n");
		return -1;
	}

	memset(&krequest, 0, sizeof(esm_hld_ioctl_load_code));
	ret = copy_from_user(&krequest, request, sizeof(esm_hld_ioctl_load_code));
	if (ret) {
		DPU_FB_ERR( "copy_from_user failed!ret = %ld\n", ret);
		krequest.returned_status = ret;
		goto Exit;
	}
	krequest.returned_status = ret;

Exit:
	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_load_code));
	if (ret) {
		DPU_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
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

	if(request == NULL){
		DPU_FB_ERR("request is null pointer\n");
		return -1;
	}

	memset(&krequest, 0, sizeof(esm_hld_ioctl_get_code_phys_addr));
	krequest.returned_status = HL_DRIVER_FAILED;
	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_get_code_phys_addr));
	if (ret) {
		DPU_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
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

	if(request == NULL){
		DPU_FB_ERR("request is null pointer\n");
		return -1;
	}

	memset(&krequest, 0, sizeof(esm_hld_ioctl_get_data_phys_addr));
	krequest.returned_status = HL_DRIVER_FAILED;
	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_get_data_phys_addr));
	if (ret) {
		DPU_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
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

	if(request == NULL){
		DPU_FB_ERR("request is null pointer\n");
		return -1;
	}

	memset(&krequest, 0, sizeof(esm_hld_ioctl_get_data_size));
	krequest.returned_status = HL_DRIVER_FAILED;

	DPU_FB_INFO( "%scmd_get_data_size: returning data_size=0x%x\n",
		MY_TAG, krequest.returned_data_size);

	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_get_data_size));
	if (ret) {
		DPU_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
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

	if((esm == NULL) || (request == NULL)){
		DPU_FB_ERR("esm or request is null pointer\n");
		return -1;
	}

	memset(&krequest, 0, sizeof(esm_hld_ioctl_hpi_read));
	ret = copy_from_user(&krequest, request, sizeof(esm_hld_ioctl_hpi_read));
	if (ret) {
		DPU_FB_ERR( "copy_from_user failed!ret = %ld\n", ret);
		krequest.returned_status = ret;
		goto Exit;
	}

	DPU_FB_INFO( "%scmd_hpi_read: Reading register at offset 0x%x\n",
		MY_TAG, krequest.offset);

	if ((!esm_en) || (!esm_opened))
	{
		DPU_FB_ERR( "DP power down, no access hdcp register\n");
		krequest.returned_status = HL_DRIVER_NO_ACCESS;
		goto Exit;
	}

	if ((esm->hpi) && (krequest.offset<=(esm->hpi_size-4)))
	{
		krequest.returned_data = ioread32(esm->hpi + krequest.offset);
		krequest.returned_status = HL_DRIVER_SUCCESS;
		DPU_FB_INFO( "%scmd_hpi_read: Returning data=0x%x\n",
			MY_TAG, krequest.returned_data);
	}
	else
	{
		krequest.returned_data = 0;
		krequest.returned_status = HL_DRIVER_NO_MEMORY;
		DPU_FB_ERR( "%scmd_hpi_read: No memory.\n", MY_TAG);
	}

Exit:
	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_hpi_read));
	if (ret) {
		DPU_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
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

	if((esm == NULL) || (request == NULL)){
		DPU_FB_ERR("esm or request is null pointer\n");
		return -1;
	}

	memset(&krequest, 0, sizeof(esm_hld_ioctl_hpi_write));
	ret = copy_from_user(&krequest, request, sizeof(esm_hld_ioctl_hpi_write));
	if (ret) {
		DPU_FB_ERR( "copy_from_user failed!ret = %ld\n", ret);
		krequest.returned_status = ret;
		goto Exit;
	}

	DPU_FB_INFO("%scmd_hpi_write: Writing 0x%x to register at offset 0x%x\n",
		MY_TAG, krequest.data, krequest.offset);

	if ((!esm_en) || (!esm_opened))
	{
		DPU_FB_ERR( "DP power down, no access hdcp register\n");
		krequest.returned_status = HL_DRIVER_NO_ACCESS;
		goto Exit;
	}

	if ((esm->hpi) && (krequest.offset<=(esm->hpi_size-4)))
	{
		iowrite32(krequest.data, esm->hpi + krequest.offset);
		krequest.returned_status = HL_DRIVER_SUCCESS;

		DPU_FB_INFO( "%scmd_hpi_write: Wrote 0x%x to register at offset 0x%x\n",
			MY_TAG, krequest.data, krequest.offset);
	}
	else
	{
		krequest.returned_status = HL_DRIVER_NO_MEMORY;
		DPU_FB_ERR( "%scmd_hpi_write: No memory.\n", MY_TAG);
	}

Exit:
	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_hpi_write));
	if (ret) {
		DPU_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
		return ret;
	}

	return 0;
}

//
// Reads from a region of the data memory
//
static long cmd_data_read(esm_device *esm, esm_hld_ioctl_data_read *request)
{
	long ret = 0;
	esm_hld_ioctl_data_read krequest;

	if((esm == NULL) || (request == NULL)){
		DPU_FB_ERR("esm or request is null pointer\n");
		return -1;
	}

	memset(&krequest, 0, sizeof(esm_hld_ioctl_data_read));
	ret = copy_from_user(&krequest, request, sizeof(esm_hld_ioctl_data_read));
	if (ret) {
		DPU_FB_ERR("copy_from_user failed!ret = %ld\n", ret);
		krequest.returned_status = ret;
		goto Exit;
	}

	DPU_FB_INFO("%scmd_data_read: Reading %u bytes from data memory at offset offset 0x%x\n",
		MY_TAG, krequest.nbytes, krequest.offset);

	if ((!esm_en) || (!esm_opened))
	{
		DPU_FB_ERR("DP power down, no access hdcp register\n");
		krequest.returned_status = HL_DRIVER_NO_ACCESS;
		goto Exit;
	}

	if ((esm->data) && (krequest.offset < esm->data_size))
	{
		if (krequest.nbytes > (esm->data_size - krequest.offset))
		{
			krequest.returned_status = HL_DRIVER_INVALID_PARAM;
			DPU_FB_ERR("%scmd_data_read: Invalid offset and size\n", MY_TAG);
		}
		else
		{
			ret = copy_to_user(krequest.dest_buf, esm->data + krequest.offset, krequest.nbytes);
			if (ret) {
				DPU_FB_ERR("copy_to_user failed!ret = %ld\n", ret);
				krequest.returned_status = ret;
				goto Exit;
			}

			krequest.returned_status = HL_DRIVER_SUCCESS;

			DPU_FB_INFO("%scmd_data_read: Done reading %u bytes from data memory at offset 0x%x\n",
				MY_TAG, krequest.nbytes, krequest.offset);
		}
	}
	else
	{
		krequest.returned_status = HL_DRIVER_NO_MEMORY;
		DPU_FB_ERR( "%scmd_data_read: No memory.\n", MY_TAG);
	}

Exit:
	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_data_read));
	if (ret) {
		DPU_FB_ERR("copy_to_user failed!ret = %ld\n", ret);
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

	if((esm == NULL) || (request == NULL)){
		DPU_FB_ERR("esm or request is null pointer\n");
		return -1;
	}

	memset(&krequest, 0, sizeof(esm_hld_ioctl_data_write));
	ret = copy_from_user(&krequest, request, sizeof(esm_hld_ioctl_data_write));
	if (ret) {
		DPU_FB_ERR("copy_from_user failed!ret = %ld\n", ret);
		krequest.returned_status = ret;
		goto Exit;
	}

	DPU_FB_INFO("%scmd_data_write: Writing %u bytes to data memory at offset 0x%x\n",
		MY_TAG, krequest.nbytes, krequest.offset);

	if ((!esm_en) || (!esm_opened))
	{
		DPU_FB_ERR("DP power down, no access hdcp register\n");
		krequest.returned_status = HL_DRIVER_NO_ACCESS;
		goto Exit;
	}

	if ((esm->data) && (krequest.offset < esm->data_size))
	{
		if (krequest.nbytes > (esm->data_size -krequest.offset))
		{
			krequest.returned_status = HL_DRIVER_INVALID_PARAM;
			DPU_FB_ERR("%scmd_data_write: Invalid offset and size\n", MY_TAG);
		} else {
			ret = copy_from_user(esm->data + krequest.offset, krequest.src_buf, krequest.nbytes);
			if (ret) {
				DPU_FB_ERR("copy_from_user failed!ret = %ld\n", ret);
				krequest.returned_status = ret;
				goto Exit;
			}
			krequest.returned_status = HL_DRIVER_SUCCESS;

			DPU_FB_INFO("%scmd_data_write: Done writing %u bytes to data memory at offset 0x%x\n",
				MY_TAG, krequest.nbytes, krequest.offset);
		}
	}
	else
	{
		krequest.returned_status = HL_DRIVER_NO_MEMORY;
		DPU_FB_ERR("%scmd_data_write: No memory.\n", MY_TAG);
	}

Exit:
	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_data_write));
	if (ret) {
		DPU_FB_ERR("copy_to_user failed!ret = %ld\n", ret);
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

	if((esm == NULL) || (request == NULL)){
		DPU_FB_ERR("esm or request is null pointer\n");
		return -1;
	}

	memset(&krequest, 0, sizeof(esm_hld_ioctl_data_set));
	ret = copy_from_user(&krequest, request, sizeof(esm_hld_ioctl_data_set));
	if (ret) {
		DPU_FB_ERR( "copy_from_user failed!ret = %ld\n", ret);
		krequest.returned_status = ret;
		goto Exit;
	}

	DPU_FB_INFO( "%scmd_data_set: Setting %u bytes (data=0x%x) of data memory from offset 0x%x\n",
		MY_TAG, krequest.nbytes, krequest.data, krequest.offset);

	if ((!esm_en) || (!esm_opened))
	{
		DPU_FB_ERR( "DP power down, no access hdcp register\n");
		krequest.returned_status = HL_DRIVER_NO_ACCESS;
		goto Exit;
	}

	if ((esm->data) && (krequest.offset < esm->data_size))
	{
		if ( krequest.nbytes > (esm->data_size -krequest.offset))
		{
			krequest.returned_status = HL_DRIVER_INVALID_PARAM;
			DPU_FB_ERR( "%scmd_data_set: Invalid offset and size.\n", MY_TAG);
		}
		else
		{
			memset(esm->data + krequest.offset, krequest.data, krequest.nbytes);
			krequest.returned_status = HL_DRIVER_SUCCESS;

			DPU_FB_INFO( "%scmd_data_set: Done setting %u bytes (data=0x%x) of data memory from " \
				"offset 0x%x\n", MY_TAG, krequest.nbytes, krequest.data, krequest.offset);
		}
	}
	else
	{
		krequest.returned_status = HL_DRIVER_NO_MEMORY;
		DPU_FB_ERR( "%scmd_data_set: No memory.\n", MY_TAG);
	}

Exit:
	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_data_set));
	if (ret) {
		DPU_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
		return ret;
	}

	return 0;
}

//
// Opens an ESM device. Associates a device file to an ESM device.
//
static long cmd_esm_open(struct file *f, esm_hld_ioctl_esm_open *request)
{
	esm_device *esm = esm_devices;
	int ret_val = HL_DRIVER_SUCCESS;
	esm_hld_ioctl_esm_open krequest;
	long ret;
	char region_name[20] = "ESM-FF350000"; // use fixed value for sc check

	if ((f == NULL) || (request == NULL)) {
		DPU_FB_ERR("f or request is null pointer\n");
		return -1;
	}

	memset(&krequest, 0, sizeof(esm_hld_ioctl_esm_open));
	f->private_data = NULL;

	// Look for a matching ESM device (fixed HPI address)
	DPU_FB_INFO("%s esm->allocated: %d\n", MY_TAG, esm->allocated);
	if (esm->allocated) {
		f->private_data = esm;
		esm_opened = 1;
		DPU_FB_INFO("%s open esm again success\n", MY_TAG);
		goto Exit;
	}

	esm->allocated = 1;
	esm->hpi_base  = HPI_ADDRESS_ESM0;
	esm->hpi_size  = HPI_REG_SIZE; // static value, HPI interface not change
	esm->code_base = HDCP_FW_ADDRESS;
	esm->code_size = HDCP_FW_SIZE;
	esm->data_base = HDCP_DATA_ADDRESS;
	esm->data_size = HDCP_DATA_SIZE;

	// Initialize the code memory
	esm->code_is_phys_mem = 1;
	esm->code = ioremap_wc(esm->code_base, esm->code_size);
	if (esm->code == NULL) {
		DPU_FB_ERR("%s Code virtual addr map fail\n", MY_TAG);
		ret_val = HL_DRIVER_NO_MEMORY;
		goto ErrorExit;
	}

	// Initialize the data memory
	esm->data_is_phys_mem = 1;
	esm->data = ioremap_wc(esm->data_base, esm->data_size);
	if (esm->data == NULL) {
		DPU_FB_ERR("%s Data virtual addr map fail\n", MY_TAG);
		ret_val = HL_DRIVER_NO_MEMORY;
		goto ErrorExit;
	}

	// Init HPI access
	request_mem_region(esm->hpi_base, esm->hpi_size, region_name);
	esm->hpi_mem_region_requested = 1;
	esm->hpi = ioremap_nocache(esm->hpi_base, esm->hpi_size);
	if (esm->hpi == NULL) {
		DPU_FB_ERR("%s HPI virtual addr map fail\n", MY_TAG);
		ret_val = HL_DRIVER_NO_MEMORY;
		goto ErrorExit;
	}

	// Associate the Linux file to the ESM device
	f->private_data = esm;
	esm_opened = 1;
	DPU_FB_INFO("%s open esm first success\n", MY_TAG);
	goto Exit;

ErrorExit:
	release_resources(esm);

Exit:
	krequest.returned_status = ret_val;
	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_esm_open));
	if (ret) {
		DPU_FB_ERR("copy_to_user failed!ret = %ld\n", ret);
		return ret;
	}

	return 0;
}

static long cmd_esm_start(esm_device *esm, esm_hld_ioctl_esm_start *request)
{
	long ret = 0;
	esm_hld_ioctl_esm_start krequest;

	if(request == NULL){
		DPU_FB_ERR("request is null pointer\n");
		return -1;
	}

	memset(&krequest, 0, sizeof(esm_hld_ioctl_esm_start));
	ret = copy_from_user(&krequest, request, sizeof(esm_hld_ioctl_esm_start));
	if (ret) {
		DPU_FB_ERR( "copy_from_user failed!ret = %ld\n", ret);
		krequest.returned_status = ret;
		goto Exit;
	}

	if (!esm_en)
	{
		DPU_FB_ERR( "DP power down, no access hdcp register\n");
		krequest.returned_status = HL_DRIVER_NO_ACCESS;
		goto Exit;
	}

	switch (krequest.type)
	{
		case 0:
			// set I_px_gpio_in[0] when boot up esm
			DPU_FB_INFO("%s start esm!\n", MY_TAG);
			krequest.returned_status = atfd_hisi_service_access_register_smc(ACCESS_REGISTER_FN_MAIN_ID_HDCP,
				DSS_HDCP22_ENABLE, 1, (u64)ACCESS_REGISTER_FN_SUB_ID_HDCP_CTRL);
			break;
		case 1:
			DPU_FB_INFO("%s set HDCP1.3 %d\n", MY_TAG, krequest.value);
			krequest.returned_status = atfd_hisi_service_access_register_smc(ACCESS_REGISTER_FN_MAIN_ID_HDCP,
				DSS_HDCP13_ENABLE, krequest.value, (u64)ACCESS_REGISTER_FN_SUB_ID_HDCP_CTRL);
			break;
		case 2:
			DPU_FB_INFO("%s Polling HDCP1.3 state...\n", MY_TAG);
			krequest.returned_status = HL_DRIVER_SUCCESS;
			break;
		case 3:	// start hdcp polling thread to check hdcp enable or not
			krequest.returned_status = HL_DRIVER_SUCCESS;
			if(krequest.value == HDCP_CHECK_STOP) {
				DPU_FB_INFO("stop hdcp polling\n");
				hdcp_stop_polling_task(0);
			} else {
				if(krequest.value == HDCP_CHECK_DISABLE) {
					DPU_FB_INFO("%s start polling to check hdcp disable...\n", MY_TAG);
				} else {
					DPU_FB_INFO("%s start polling to check hdcp enable...\n", MY_TAG);
				}
				krequest.returned_status = hdcp_check_enable(krequest.value);
			}
			break;
		default:
			break;
	}

Exit:
	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_esm_start));
	if (ret) {
		DPU_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
		return ret;
	}

	return 0;
}

static long cmd_get_te_info(esm_device *esm, esm_hld_ioctl_get_te_info*request)
{
	long ret = 0;
	esm_hld_ioctl_get_te_info krequest;
	uint64_t temp;

	if(request == NULL){
		DPU_FB_ERR("request is null pointer\n");
		return -1;
	}

	memset(&krequest, 0, sizeof(esm_hld_ioctl_get_te_info));
	ret = copy_from_user(&krequest, request, sizeof(esm_hld_ioctl_get_te_info));
	if (ret) {
		DPU_FB_ERR( "copy_from_user failed!ret = %ld\n", ret);
		krequest.returned_status = ret;
		goto Exit;
	}

	if ((!esm_en) || (!esm_opened))
	{
		DPU_FB_ERR( "DP power down, no access hdcp register\n");
		krequest.returned_status = HL_DRIVER_NO_ACCESS;
		goto Exit;
	}

	switch (krequest.type)
	{
		case 0:
			// set I_px_gpio_in[0] when boot up esm
			DPU_FB_INFO("%s Get TE basic info!\n", MY_TAG);
			krequest.bcaps = g_bcaps;
			temp = (g_bksv >> 32) & 0xFFFFFFFF;
			krequest.bKSV_msb = (uint32_t)temp;
			temp = g_bksv & 0xFFFFFFFF;
			krequest.bKSV_lsb = (uint32_t)temp;
			break;
		case 1:
			DPU_FB_INFO("%s Get SHA1 buffer!\n", MY_TAG);
			ret = hdcp_get_sha1_buffer(krequest.sha1_buffer, &krequest.buffer_length, krequest.V_Prime);
			if (ret) {
				DPU_FB_ERR( "Get SHA1 buffer failed!ret = %ld\n", ret);
				krequest.returned_status = ret;
				goto Exit;
			}
			break;
		default:
			break;
	}
	krequest.returned_status = HL_DRIVER_SUCCESS;

Exit:
	ret = copy_to_user(request, &krequest, sizeof(esm_hld_ioctl_get_te_info));
	if (ret) {
		DPU_FB_ERR( "copy_to_user failed!ret = %ld\n", ret);
		return ret;
	}

	return 0;
}


static long useless_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	long ret = HL_DRIVER_FAILED;

	if ((f==0) || (arg ==0) || (!f->private_data))
		return ret;

	switch (cmd) {
	case ESM_HLD_IOCTL_LOAD_CODE:
		ret = cmd_load_code((esm_device *)f->private_data,
			(esm_hld_ioctl_load_code *)(uintptr_t)arg);
		break;
	case ESM_HLD_IOCTL_GET_CODE_PHYS_ADDR:
		ret = cmd_get_code_phys_addr((esm_device *)f->private_data,
			(esm_hld_ioctl_get_code_phys_addr *)(uintptr_t)arg);
		break;
	case ESM_HLD_IOCTL_GET_DATA_PHYS_ADDR:
		ret = cmd_get_data_phys_addr((esm_device *)f->private_data,
			(esm_hld_ioctl_get_data_phys_addr *)(uintptr_t)arg);
		break;
	case ESM_HLD_IOCTL_GET_DATA_SIZE:
		ret = cmd_get_data_size((esm_device *)f->private_data,
			(esm_hld_ioctl_get_data_size *)(uintptr_t)arg);
		break;
	default:
		DPU_FB_ERR("%sUnknown IOCTL request %d.\n", MY_TAG, cmd);
		break;
	}

	return ret;
}

long esm_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	long ret = HL_DRIVER_FAILED;

	if ((f == 0) || (arg == 0))
		return ret;

	if ((cmd != ESM_HLD_IOCTL_ESM_OPEN) && (!f->private_data))
		return ret;

	switch (cmd) {
	case ESM_HLD_IOCTL_HPI_READ:
		ret = cmd_hpi_read((esm_device *)f->private_data,
			(esm_hld_ioctl_hpi_read *)(uintptr_t)arg);
		break;
	case ESM_HLD_IOCTL_HPI_WRITE:
		ret = cmd_hpi_write((esm_device *)f->private_data,
			(esm_hld_ioctl_hpi_write *)(uintptr_t)arg);
		break;
	case ESM_HLD_IOCTL_DATA_READ:
		ret = cmd_data_read((esm_device *)f->private_data,
			(esm_hld_ioctl_data_read *)(uintptr_t)arg);
		break;
	case ESM_HLD_IOCTL_DATA_WRITE:
		ret = cmd_data_write((esm_device *)f->private_data,
			(esm_hld_ioctl_data_write *)(uintptr_t)arg);
		break;
	case ESM_HLD_IOCTL_DATA_SET:
		ret = cmd_data_set((esm_device *)f->private_data,
			(esm_hld_ioctl_data_set *)(uintptr_t)arg);
		break;
	case ESM_HLD_IOCTL_ESM_OPEN:
		ret = cmd_esm_open(f, (esm_hld_ioctl_esm_open *)(uintptr_t)arg);
		break;
	case ESM_HLD_IOCTL_ESM_START:
		ret = cmd_esm_start((esm_device *)f->private_data,
			(esm_hld_ioctl_esm_start *)(uintptr_t)arg);
		break;
	case ESM_HLD_IOCTL_GET_TE_INFO:
		ret = cmd_get_te_info((esm_device *)f->private_data,
			(esm_hld_ioctl_get_te_info *)(uintptr_t)arg);
		break;
	default:
		ret = useless_ioctl(f, cmd, arg);
		break;
	}

	return ret;
}

void esm_end_device(void)
{
	int i;
	esm_device *esm = esm_devices;
	DPU_FB_INFO("%s end esm device Initializing\n", MY_TAG);
	for (i = 0; i < MAX_ESM_DEVICES; i++) {
		if (esm->allocated) {
			if (esm->code && !esm->code_is_phys_mem) {
				dma_addr_t dh = (dma_addr_t)esm->code_base;
				dma_free_coherent(NULL, esm->code_size, esm->code, dh);
			}

			if (esm->data && !esm->data_is_phys_mem) {
				dma_addr_t dh = (dma_addr_t)esm->data_base;
				dma_free_coherent(NULL, esm->data_size, esm->data, dh);
			}

			release_resources(esm);
		}
		esm++;
	}

	memset(esm_devices, 0, sizeof(esm_devices));
	DPU_FB_INFO("%s end_esm_device Done\n", MY_TAG);
}

void esm_hld_init(void)
{
	DPU_FB_INFO("%s esm hld init Initializing\n", MY_TAG);
	memset(esm_devices, 0, sizeof(esm_devices));
	DPU_FB_INFO("%s esm hld init Done\n", MY_TAG);
}

void esm_driver_enable(int en)
{
	esm_en = en;
}

