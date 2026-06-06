/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*lint --e{533,830}*/
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG madapt
HWLOG_REGIST();


/*madapt support write/read nv by hisi modem nv API. All code with this feature, should be contained by this macro
* nvcfg version before 3.0, this macro will be defined, otherwise nvcfg 4.0 or more high nvcfg version will not define.
*/
#if defined(CONFIG_HISI_BALONG_MODEM) && !defined(CONFIG_HISI_SECBOOT_IMG)
#define MADAPT_HISI_MODEM_NV_RW_API
#endif

#define BSP_ERR_MADAPT_OK                    (0)
#define BSP_ERR_MADAPT_PARAM_ERR            (1)
#define BSP_ERR_MADAPT_MALLOC_FAIL            (2)
#define BSP_ERR_MADAPT_COPY_FROM_USER_ERR    (3)
#define BSP_ERR_MADAPT_OPEN_FILE_ERR        (4)
#define BSP_ERR_MADAPT_FILE_SIZE_ERR        (5)
#define BSP_ERR_MADAPT_READ_FILE_ERR        (6)
#define BSP_ERR_MADAPT_FILE_HDR_CHECK_ERR    (7)
#define BSP_ERR_MADAPT_NV_HDR_CHECK_ERR        (8)
#define BSP_ERR_MADAPT_WRITE_NV_ERR            (9)
#define BSP_ERR_MADAPT_TIMEOUT                (10)
#define BSP_ERR_MADAPT_PTR_NULL                (11)
#define BSP_ERR_MADAPT_READ_NV_ERR          (12)
#define BSP_ERR_MADAPT_REMOVE_FILE_ERR          (13)
#define BSP_ERR_MADAPT_COPY_TO_USER_ERR 14
#define BSP_ERR_MADAPT_COMMON_ERR            (100)
/*AR000AD08A sunjun/00290209 20180807 begin*/
#define BSP_ERR_MADAPT_NO_THIS_NV             (0x10100016)
/*AR000AD08A sunjun/00290209 20180807 end*/

#define MADAPT_SUCCESS      (0)
#define MADAPT_FAIL      (1)

#define MADAPT_DEVICE_NAME                "madapt"
#define MADAPT_DEVICE_CLASS                "madapt_class"
#define MADAPT_FILE_MAX_SIZE            (1024*1024)
#define MADAPT_MAX_USER_BUFF_LEN        (96)
#define MADAPT_FILE_END_FLAG            (0xFFFFFFFF)
#define MADAPT_FILE_NOTBIT_FLAG         (0xFFFFFFFF)
#define MADAPT_NV_ID_INVALID         (0xFFFFFFFF)
#define MADAPT_MIN_NV_MODEM_ID            (1)
#define MADAPT_MAX_NV_MODEM_ID            (3)
#define MADAPT_MODEM_ID_0 1
#define MADAPT_MODEM_ID_1 2
#define MADAPT_NVFILE_PATH    "/cust_spec/modemConfig/nvcfg/"
#define MADAPT_NVFILE_VERSION            (0x0601)
#define MADAPT_BIT_LENGTH_SIZE          (32)
#define MADAPT_MAX_NV_LENGTH_SIZE           (2048)
#define MADAPT_MODEMID_CONVERT(x)        (x-1)
#define MADAPT_MAX_PATH_LENGTH 80

#define MADAPT_MODEMID_INVALID          (0xFF)
#define MADAPT_WORK_WAIT_TIMER      30
#define MADAPT_READ_WORK_WAIT_TIMER 2
#define MADAPT_VALID_PATH_HEAD_ODM    "/odm"
#define MADAPT_VALID_PATH_HEAD_HW_ODM    "/hw_odm"
#define MADAPT_VALID_PATH_HEAD_COTA    "/data/cota"
#define MADAPT_VALID_PATH_HEAD_COTA_VENDER     "/data/vendor/cota/"
#define NCFG_BIN_FILE_PATH_IN_MODEM_SEC "/mnt/modem/mnvm2:0/mbn_nv/sec_carrier.bin"
#define NCFG_BIN_FILE_PATH_IN_MODEM     "/mnt/modem/mnvm2:0/mbn_nv/carrier.bin"

#define MADAPT_MAX_FILE_COUNT  5
#define MADAPT_RW_FILE_TRIL    1
#define MADAPT_RW_FILE_HEAD    0

#define MADAPT_DEFAULT_MODIFY_TIME 1
#define MADAPT_PARAM_COMMON "common"
#define MADAPT_NVCFG_RESULT_NVID 0xD14D
#define MADAPT_NVCFG_RESULT_UPDATED 1

struct madapt_nvcfg_result_type {
	unsigned int others    :8;
	unsigned int rsv_nvcfg :1;
	unsigned int reserved  :23;
};

struct madapt_file_stru {
	unsigned int	modem_id;
	unsigned int	len;
	char			file[MADAPT_MAX_USER_BUFF_LEN];
};

struct madapt_file_stru_ex {
	unsigned int    modem_id;
	unsigned int    file_count;
	char file[MADAPT_MAX_FILE_COUNT][MADAPT_MAX_USER_BUFF_LEN];
};

struct madapt_mbn_file_head {
	unsigned int    mgic_number;
	unsigned int    data_len;
	unsigned int    reserve[2];   //具体内容暂不关注
};

struct madapt_item_hdr_type {
	unsigned int	nv_item_number;
	unsigned int	nv_modem_id;
	unsigned int	nv_item_size;
};
struct madapt_item_ato_hdr_type {
	unsigned int	feature_index;
	unsigned int	nv_item_number;
	unsigned int	nv_modem_id;
	unsigned int	nv_item_byte_start;
	unsigned int	nv_item_byte_length;
	unsigned int	nv_ato_bit_pos; // BIT  type,  if value is 0XFFFFFFFF, it not bit type, so ignore!
};
struct madapt_item_ato_hdr_bit_type {
	unsigned int	nv_item_number;
	unsigned int	nv_modem_id;
	unsigned int	nv_item_byte_start;
	unsigned int	nv_item_byte_length; 
	unsigned char	nv_bit_value[MADAPT_BIT_LENGTH_SIZE+1];
};

struct my_work_struct {
	int test;
	struct work_struct proc_nv;
};

struct nv_item_map {
	unsigned int start_num;
	unsigned int end_num;
};


static struct cdev cdev;
static unsigned int madapt_major;
static struct class *madapt_class;
static struct madapt_file_stru *kbuf;
static struct madapt_file_stru_ex *kbuf_ex;
static struct completion my_completion;
static struct my_work_struct test_work;
static ssize_t work_ret = BSP_ERR_MADAPT_OK;
static struct completion my_read_completion;
static struct my_work_struct test_read_work;
static ssize_t read_work_ret = BSP_ERR_MADAPT_OK;
static struct mutex madapt_mutex;

#ifdef MADAPT_HISI_MODEM_NV_RW_API
static unsigned int nvcfg_result_flag = 0;
#endif
static unsigned long modify_time = MADAPT_DEFAULT_MODIFY_TIME;
static char commbin_path[][MADAPT_MAX_PATH_LENGTH] = {
	{ "/mnt/modem/mnvm2:0/mbn_nv/comm.bin" },
	{ "/mnvm2:0/mbn_nv/comm.bin" },
	{ "/mnt/modem/mnvm2:0/mbn_nv/comm.mbn" },
	{ "/mnvm2:0/mbn_nv/comm.mbn" }
};

ssize_t madapt_dev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos);
ssize_t madapt_dev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos);

static const struct file_operations nv_fops = {
	.owner = THIS_MODULE,
	.read = madapt_dev_read,
	.write = madapt_dev_write,
};

#ifdef MADAPT_HISI_MODEM_NV_RW_API
extern unsigned int mdrv_nv_writeex(unsigned int modemid, unsigned int itemid, void *pdata, unsigned int data_len);
extern unsigned int mdrv_nv_readex(unsigned int modemid, unsigned int itemid, void *pdata, unsigned int data_len);
extern unsigned int mdrv_nv_get_length(unsigned int itemid, unsigned int *data_len);
extern unsigned int mdrv_nv_flush(void);
#endif

static void madapt_free(void *buf)
{
	if (buf != NULL) {
		kfree(buf);
	}
}

#ifdef MADAPT_HISI_MODEM_NV_RW_API
static unsigned int nv_buffer_update(unsigned int ulOffset,void *nv_buffer, void *pdata, unsigned int data_len)
{
	unsigned char *nv_offset = nv_buffer;

	nv_offset += ulOffset;
	if (nv_offset == NULL ) {
		return MADAPT_FAIL;
	} else {
		memcpy(nv_offset,(unsigned char *)pdata,data_len);
		return MADAPT_SUCCESS;
	}
}

static unsigned int nv_read_from_buffer(unsigned int ulOffset,void *nv_buffer, void *pdata, unsigned int data_len)
{
	unsigned char *nv_offset = nv_buffer;

	nv_offset += ulOffset;
	if (nv_offset == NULL){
		return MADAPT_FAIL;
	} else {
		memcpy(pdata,nv_offset,data_len);
		return MADAPT_SUCCESS;
	}
}

/*change the bit value at the postion of nv_value_byte . this is for loading .ato nv file*/
static void set_bit_value(unsigned char *nv_value_byte, int pos, unsigned char* nValue)
{
	if (*nValue == 0x1) {
		nv_value_byte[pos / 8] |= 1UL<<(pos % 8);  //8: 1byte == 8 bit
	} else if (*nValue == 0x0) {
		nv_value_byte[pos / 8] &= ~(1UL<<(pos % 8));  //8: 1byte == 8 bit
	} else {
		hwlog_err("no changed\n");
		return ;
	}
}

static int madapt_write_nv_item(unsigned int modem_id, unsigned int nv_id, void *nv_buffer, unsigned int nv_item_len){
	int ret = MADAPT_SUCCESS;

	if (nv_id !=MADAPT_NV_ID_INVALID) {
		ret = mdrv_nv_writeex(MADAPT_MODEMID_CONVERT(modem_id), nv_id, nv_buffer, nv_item_len);
		if (ret != MADAPT_SUCCESS) {
			ret = MADAPT_FAIL;
		}
	}

	return ret;
}

static int madapt_read_nv(unsigned int modem_id, unsigned int nv_item_number,
			char *nv_buffer, unsigned int *nv_item_length)
{
	int ret;
	unsigned int nv_length = 0;

	ret = mdrv_nv_get_length(nv_item_number, &nv_length);
	if (ret != MADAPT_SUCCESS) {
		hwlog_err("madapt_read_nv read curNV = %x failed=%d\n",
					nv_item_number, ret);
		return BSP_ERR_MADAPT_READ_NV_ERR;
	}

	if (nv_length < MADAPT_MAX_NV_LENGTH_SIZE) {
		ret = mdrv_nv_readex(MADAPT_MODEMID_CONVERT(modem_id),
					nv_item_number, nv_buffer, nv_length);
		if (ret != MADAPT_SUCCESS)
			ret = BSP_ERR_MADAPT_READ_NV_ERR;
	} else {
		hwlog_err("nv:%d with modemid:%d && size:%d write fail, ret=%d\n",
					nv_item_number, modem_id, nv_length, ret);
		ret = BSP_ERR_MADAPT_READ_NV_ERR;
	}

	*nv_item_length = nv_length;
	return BSP_ERR_MADAPT_OK;
}

static int madapt_read_nvcfg_result_flag(void)
{
	int ret;
	unsigned int nv_item_length = sizeof(struct madapt_nvcfg_result_type);
	unsigned int nv_item_number = MADAPT_NVCFG_RESULT_NVID;
	struct madapt_nvcfg_result_type nvcfg_result;
	memset(&nvcfg_result, 0x00, sizeof(nvcfg_result));

	ret = madapt_read_nv(MADAPT_MODEM_ID_0,
			nv_item_number, (char *)&nvcfg_result, &nv_item_length);
	if (ret != MADAPT_SUCCESS) {
		hwlog_err("madapt_read_nvcfg_result_flag: modem0 read nv:%d fail, ret=%d\n",
				nv_item_number, ret);
		ret = BSP_ERR_MADAPT_READ_NV_ERR;
		return ret;
	}

	nvcfg_result_flag = nvcfg_result.rsv_nvcfg;
	hwlog_info("read nvcfg reslut flag:%d success\n", nvcfg_result_flag);

	return BSP_ERR_MADAPT_OK;
}

static int madapt_set_nvcfg_result_flag(void)
{
	int ret;
	unsigned int nv_item_length = sizeof(struct madapt_nvcfg_result_type);
	unsigned int nv_item_number = MADAPT_NVCFG_RESULT_NVID;
	struct madapt_nvcfg_result_type nvcfg_result;
	memset(&nvcfg_result, 0x00, sizeof(nvcfg_result));

	ret = madapt_read_nv(MADAPT_MODEM_ID_0,
			nv_item_number, (char *)&nvcfg_result, &nv_item_length);
	if (ret != MADAPT_SUCCESS) {
		hwlog_err("update_nvcfg_result: modem0 read nv:%d fail, ret=%d\n",
				nv_item_number, ret);
		ret = BSP_ERR_MADAPT_READ_NV_ERR;
		return ret;
	}

	/* update nv value special part for nvcfg result */
	nvcfg_result.rsv_nvcfg = MADAPT_NVCFG_RESULT_UPDATED;

	ret = madapt_write_nv_item(MADAPT_MODEM_ID_0,
			nv_item_number, (char *)&nvcfg_result, nv_item_length);
	if (ret != MADAPT_SUCCESS) {
		hwlog_err("update_nvcfg_result: modem0 write nv:%d fail, ret=%d\n",
				nv_item_number, ret);
		ret = BSP_ERR_MADAPT_READ_NV_ERR;
		return ret;
	}

	hwlog_info("update nvcfg result flag success\n");
	return BSP_ERR_MADAPT_OK;
}

static int madapt_check_nv_header_valid_atonv(struct madapt_item_ato_hdr_type *nv_header)
{
	int ret = MADAPT_SUCCESS;

	if ( nv_header->nv_modem_id < MADAPT_MIN_NV_MODEM_ID
		|| nv_header->nv_modem_id > MADAPT_MAX_NV_MODEM_ID
		|| nv_header->nv_item_byte_length == 0
		|| nv_header->nv_item_byte_length >= MADAPT_MAX_NV_LENGTH_SIZE
		|| nv_header->nv_item_byte_start >= MADAPT_MAX_NV_LENGTH_SIZE
		|| (nv_header->nv_item_byte_start + nv_header->nv_item_byte_length) >= MADAPT_MAX_NV_LENGTH_SIZE) {
		ret = MADAPT_FAIL;
	}

	return ret;
}

static int madapt_read_current_head_nv_atonv(struct madapt_item_ato_hdr_type *nv_header,
			char *nv_buffer, unsigned int *nv_item_length)
{
	int ret = BSP_ERR_MADAPT_OK;
	unsigned int nv_length = 0;

	ret = mdrv_nv_get_length(nv_header->nv_item_number,&nv_length);
	/*AR000AD08A sunjun/00290209 20180807 begin*/
	if (ret == BSP_ERR_MADAPT_NO_THIS_NV) {
		hwlog_err("read curNV = %x,not exist, goto next\n",nv_header->nv_item_number);
		return ret;
	}
	/*AR000AD08A sunjun/00290209 20180807 end*/

	if ((nv_header->nv_item_byte_start + nv_header->nv_item_byte_length) <= nv_length
		&& nv_length < MADAPT_MAX_NV_LENGTH_SIZE
		&& ret == MADAPT_SUCCESS) {
		ret = mdrv_nv_readex(MADAPT_MODEMID_CONVERT(nv_header->nv_modem_id),
					nv_header->nv_item_number, nv_buffer, nv_length);
		if (ret != MADAPT_SUCCESS) {
			ret = BSP_ERR_MADAPT_READ_NV_ERR;
		}
	} else {
		hwlog_err("madapt_parse_and_writeatonv, nv[%d] with modemid[%d] and size[%d] write fail, ret=%d.\n",
					nv_header->nv_item_number, nv_header->nv_modem_id, nv_length, ret);
		vfree(nv_buffer);
		ret = BSP_ERR_MADAPT_READ_NV_ERR;
	}

	*nv_item_length = nv_length;
	return ret;
}
static void madapt_parse_header_atonv(struct madapt_item_ato_hdr_type *nv_header, void *data)
{
	nv_header->nv_item_number = ((struct madapt_item_ato_hdr_type *)data)->nv_item_number;
	nv_header->nv_modem_id = ((struct madapt_item_ato_hdr_type *)data)->nv_modem_id;
	nv_header->nv_item_byte_start = ((struct madapt_item_ato_hdr_type *)data)->nv_item_byte_start;
	nv_header->nv_item_byte_length = ((struct madapt_item_ato_hdr_type *)data)->nv_item_byte_length;
	nv_header->nv_ato_bit_pos = ((struct madapt_item_ato_hdr_type *)data)->nv_ato_bit_pos;
}

static int madapt_parse_and_writeatonv(char *ptr, int len)
{
	char *nv_buffer = NULL;
	unsigned int ret = MADAPT_SUCCESS;
	struct madapt_item_ato_hdr_type nv_header;
	unsigned int nv_item_length = 0;
	unsigned int lastNV = MADAPT_NV_ID_INVALID;
	unsigned int currentNV=MADAPT_NV_ID_INVALID;
	unsigned char nv_bitType_value[MADAPT_BIT_LENGTH_SIZE+1];

	memset(&nv_header, 0, sizeof(struct madapt_item_ato_hdr_type));
	memset(nv_bitType_value, '\0', sizeof(nv_bitType_value));

	//NV value max size must be no more than 2048 byte
	nv_buffer = (char *)vmalloc(MADAPT_MAX_NV_LENGTH_SIZE*sizeof(char));
	if (nv_buffer == NULL) {
		hwlog_err("madapt_parse_and_writeatonv_new, vmalloc error\n");
		return BSP_ERR_MADAPT_MALLOC_FAIL;
	}

	do {
		nv_header.feature_index = ((struct madapt_item_ato_hdr_type *)ptr)->feature_index;

		if (nv_header.feature_index== MADAPT_FILE_END_FLAG) {
			ret = madapt_write_nv_item(nv_header.nv_modem_id, lastNV, nv_buffer, nv_item_length);
			if (ret != MADAPT_SUCCESS) {
				vfree(nv_buffer);
				return BSP_ERR_MADAPT_WRITE_NV_ERR;
			}
			hwlog_err("madapt_parse_and_writeatonv, find nv file end flag.\n");
			break;
		}
		
		madapt_parse_header_atonv(&nv_header, ptr);
		ret = madapt_check_nv_header_valid_atonv(&nv_header);
		if (ret != MADAPT_SUCCESS) {
			hwlog_err("madapt_parse_and_writeatonv, nv header: nv id: [%d], nv modemid: [%d], nv size: [%d].\n",
					nv_header.nv_item_number, nv_header.nv_modem_id, nv_header.nv_item_byte_length);
			 vfree(nv_buffer);
			 return BSP_ERR_MADAPT_NV_HDR_CHECK_ERR;
		}

		ptr += sizeof(nv_header);

		/* check the last nv item length, should no more than the file length */
		if (len < (sizeof(struct madapt_item_ato_hdr_type) + nv_header.nv_item_byte_length)) {
			hwlog_err("Maybe file is broken\n");
			vfree(nv_buffer);
			return BSP_ERR_MADAPT_NV_HDR_CHECK_ERR;
		}

		currentNV = nv_header.nv_item_number;
		if (currentNV != lastNV) {
			ret = madapt_write_nv_item(nv_header.nv_modem_id, lastNV, nv_buffer, nv_item_length);
			if (ret != MADAPT_SUCCESS) {
				vfree(nv_buffer);
				return BSP_ERR_MADAPT_WRITE_NV_ERR;
			}

			memset(nv_buffer, '\0', MADAPT_MAX_NV_LENGTH_SIZE);
			ret = madapt_read_current_head_nv_atonv(&nv_header, nv_buffer, &nv_item_length);
			if (ret == BSP_ERR_MADAPT_NO_THIS_NV) {
				lastNV = MADAPT_NV_ID_INVALID;
				goto next_nv_ptr;
			}

			if (ret == BSP_ERR_MADAPT_READ_NV_ERR) {
				vfree(nv_buffer);
				return BSP_ERR_MADAPT_READ_NV_ERR;
			}

			nv_buffer[nv_item_length]='\0';
			lastNV = currentNV;
		}

		if (nv_header.nv_ato_bit_pos == MADAPT_FILE_NOTBIT_FLAG) {
			ret = nv_buffer_update(nv_header.nv_item_byte_start, nv_buffer, ptr, nv_header.nv_item_byte_length);
		} else {
			ret = nv_read_from_buffer(nv_header.nv_item_byte_start, nv_buffer,
						nv_bitType_value, nv_header.nv_item_byte_length);
			set_bit_value(nv_bitType_value, nv_header.nv_ato_bit_pos, (unsigned char *)ptr);
			ret = nv_buffer_update( nv_header.nv_item_byte_start, nv_buffer,
						nv_bitType_value, nv_header.nv_item_byte_length);
		}

		if (ret != MADAPT_SUCCESS) {
			hwlog_err("madapt_parse_and_writeatonv, nv_buffer_update error,ret=%d.\n",ret);
			vfree(nv_buffer);
			return BSP_ERR_MADAPT_WRITE_NV_ERR;
		}
	/*AR000AD08A sunjun/00290209 20180807 begin*/
next_nv_ptr:
	/*AR000AD08A sunjun/00290209 20180807 end*/
		len -= (sizeof(struct madapt_item_ato_hdr_type) + nv_header.nv_item_byte_length);
		ptr += nv_header.nv_item_byte_length;
	} while (len > 0); 

	vfree(nv_buffer);

	ret = madapt_set_nvcfg_result_flag();
	if (ret != MADAPT_SUCCESS) {
		hwlog_err("update nvcfg matched flag error, ret = %d\n", ret);
		return BSP_ERR_MADAPT_WRITE_NV_ERR;
	}

	/* flush all nv into file system */
	ret = mdrv_nv_flush();
	if (ret != MADAPT_SUCCESS) {
		hwlog_err("write mdrv_nv_flush error, ret = %d\n", ret);
		return BSP_ERR_MADAPT_WRITE_NV_ERR;
	}
	hwlog_err("write mdrv_nv_flush success ret = %d\n", ret);

	return BSP_ERR_MADAPT_OK;
}
static int madapt_check_nv_header_valid(struct madapt_item_hdr_type *nv_header)
{
	int ret = MADAPT_SUCCESS;

	if ( nv_header->nv_modem_id < MADAPT_MIN_NV_MODEM_ID
		|| nv_header->nv_modem_id > MADAPT_MAX_NV_MODEM_ID
		|| nv_header->nv_item_size == 0
		|| nv_header->nv_item_size >= MADAPT_FILE_MAX_SIZE) {
		ret = MADAPT_FAIL;
	}

	return ret;
}

static int madapt_parse_and_writenv(char *ptr, int len, unsigned int modem_id)
{
	unsigned int ret = 0;
	struct madapt_item_hdr_type nv_header;

	memset(&nv_header, 0, sizeof(struct madapt_item_hdr_type));
	do {
		nv_header.nv_item_number = ((struct madapt_item_hdr_type *)ptr)->nv_item_number;
		if (nv_header.nv_item_number == MADAPT_FILE_END_FLAG) {
			hwlog_err("madapt_parse_and_writenv, find nv file end flag.\n");
			break;
		}
		nv_header.nv_modem_id = modem_id;
		nv_header.nv_item_size = ((struct madapt_item_hdr_type *)ptr)->nv_item_size;
		ptr += sizeof(nv_header);

		ret = madapt_check_nv_header_valid(&nv_header);
		if (ret != MADAPT_SUCCESS || (sizeof(struct madapt_item_hdr_type) + nv_header.nv_item_size) > len) {
			 hwlog_err("madapt_parse_and_writenv, invalid nv header: nv id: [%d], nv modemid: [%d], nv size: [%d].\n",
					nv_header.nv_item_number, nv_header.nv_modem_id, nv_header.nv_item_size);
			 return BSP_ERR_MADAPT_NV_HDR_CHECK_ERR;
		}

		ret = mdrv_nv_writeex( MADAPT_MODEMID_CONVERT(nv_header.nv_modem_id),
						nv_header.nv_item_number, ptr, nv_header.nv_item_size);
		if (ret != MADAPT_SUCCESS) {
			/*AR000AD08A sunjun/00290209 20180807 begin*/
			if (BSP_ERR_MADAPT_NO_THIS_NV == ret) {
				hwlog_err("write curNV = %x,not exist, goto next\n",nv_header.nv_item_number);
			} else {
			/*AR000AD08A sunjun/00290209 20180807 end*/
				hwlog_err("madapt_parse_and_writenv, nv[%d] with modemid[%d] and size[%d] write fail, ret=%d.\n",
					nv_header.nv_item_number, nv_header.nv_modem_id, nv_header.nv_item_size, ret);
				return BSP_ERR_MADAPT_WRITE_NV_ERR;
			}
	}
		ptr += nv_header.nv_item_size;
		len -= (sizeof(struct madapt_item_hdr_type) + nv_header.nv_item_size);
	} while (len > 0);

	ret = madapt_set_nvcfg_result_flag();
	if (ret != MADAPT_SUCCESS) {
		hwlog_err("update nvcfg matched flag error, ret = %d\n", ret);
		return BSP_ERR_MADAPT_WRITE_NV_ERR;
	}

	/* flush all nv into file system */
	ret = mdrv_nv_flush();
	if (ret != MADAPT_SUCCESS) {
		hwlog_err("write mdrv_nv_flush error, ret = %d\n", ret);
		return BSP_ERR_MADAPT_WRITE_NV_ERR;
	}
	hwlog_err("write mdrv_nv_flush success ret = %d\n", ret);

	return BSP_ERR_MADAPT_OK;
}
#endif

static int madapt_remove_mbn_carrirer_file(void)
{
	mm_segment_t old_fs = {0};
	int ret = BSP_ERR_MADAPT_OK;

	old_fs = get_fs();
	set_fs((unsigned long)KERNEL_DS);
	ret = sys_unlink(NCFG_BIN_FILE_PATH_IN_MODEM);
	if ( ret < 0) {
		hwlog_err("remove carrier bin file failed. ret=%d", ret);
		ret= BSP_ERR_MADAPT_REMOVE_FILE_ERR;
	}
	set_fs(old_fs);

	return ret;
}

static int madapt_write_mbn_carrirer_file(char *file_buffer, size_t buffer_Size, int is_appended)
{
	struct file *fp = NULL;
	mm_segment_t old_fs = {0};
	loff_t pos = 0;
	ssize_t size = 0;
	int ret_sync = 0;
	int file_size = 0;

	//should add O_SYNC flag for some platform kernel sync file system delay.
	fp = filp_open(NCFG_BIN_FILE_PATH_IN_MODEM, O_RDWR|O_CREAT|O_SYNC, 0644);
	if (IS_ERR(fp)) {
		hwlog_err("madapt_write_mbn_carrirer_file, create file error! \n");
		return BSP_ERR_MADAPT_OPEN_FILE_ERR;
	}

	old_fs = get_fs();
	set_fs((unsigned long)KERNEL_DS);

	vfs_llseek(fp, 0L, (is_appended==0) ? SEEK_SET : SEEK_END);
	pos = fp->f_pos;
	size = vfs_write(fp, file_buffer, buffer_Size, &pos);
	hwlog_err("madapt_write_mbn_carrirer_file,  vfs_write pos =%d, write size=%zu \n", (int)pos, size);
	//sync to hardware flash immediately, some system will sync delay.
	ret_sync = vfs_fsync(fp,0);
	file_size = file_inode(fp)->i_size;
	hwlog_err("copy file new size(%d) success!sync ret=%d\n", file_size, ret_sync);

	filp_close(fp,NULL);
	set_fs(old_fs);
	return BSP_ERR_MADAPT_OK;
}

static int check_file_path_valid(const char * file)
{
	int ret = BSP_ERR_MADAPT_OK;

	//file star with special path head
	if (file == strstr(file, MADAPT_VALID_PATH_HEAD_ODM) ||
		file == strstr(file, MADAPT_VALID_PATH_HEAD_HW_ODM) ||
		file == strstr(file, MADAPT_VALID_PATH_HEAD_COTA) ||
		file == strstr(file, MADAPT_VALID_PATH_HEAD_COTA_VENDER)) {
		hwlog_err("%s, file path valid\n", __func__);
	} else {
		hwlog_err("%s, file path invalid, return err\n", __func__);
		ret = BSP_ERR_MADAPT_PARAM_ERR;
	}

	return ret;
}
/*
* read nvcfg bin file to file_buf
* when read success, will return BSP_ERR_MADAPT_OK and must kfree *file_buf memory.
* when return others, will not any change *file_buf value.
*/
static int madapt_read_mbn_file(char * file, char **file_buf, int *file_buf_size)
{
	struct file *fp = NULL;
	mm_segment_t fs = {0};
	loff_t pos = 0;
	int ret_size = 0;
	int size = 0;
	int ret = BSP_ERR_MADAPT_OK;
	char *k_buffer = NULL;

	ret = check_file_path_valid(file);
	if (ret != BSP_ERR_MADAPT_OK) {
		return ret;
	}

	fp = filp_open(file, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		hwlog_err("parse_write_file, open file error!\n");
		ret = BSP_ERR_MADAPT_OPEN_FILE_ERR;
		return ret;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);

	size = file_inode(fp)->i_size;
	if (size <= 0 || size >= MADAPT_FILE_MAX_SIZE) {
		hwlog_err("madapt_read_mbn_file, file size(%d) error!\n", size);
		ret = BSP_ERR_MADAPT_FILE_SIZE_ERR;
		filp_close(fp, NULL);
		set_fs(fs);
		return ret;
	} 

	pos = 0;
	k_buffer = kmalloc((size), GFP_KERNEL);
	if (k_buffer == NULL) {
		hwlog_err("parse_write_file, kmalloc error\n");
		ret = BSP_ERR_MADAPT_MALLOC_FAIL;
		filp_close(fp, NULL);
		set_fs(fs);
		return ret;
	}

	memset(k_buffer, 0, size);
	ret_size = vfs_read(fp, k_buffer, size, &pos);
	if (size != ret_size) {
		hwlog_err("madapt_read_mbn_file, error vfs_read ret: %d, readsize: %d\n", ret_size, (int)(size));
		ret = BSP_ERR_MADAPT_READ_FILE_ERR;
		kfree(k_buffer);
		k_buffer = NULL;
		filp_close(fp, NULL);
		set_fs(fs);
		return ret;
	}

	*file_buf = k_buffer;
	*file_buf_size = size;
	hwlog_err("madapt_read_mbn_file success, get nvbin file size(%d)!\n", *file_buf_size );

	filp_close(fp, NULL);
	set_fs(fs);
	return ret;
}

static int parse_write_file(struct madapt_file_stru *file)
{
	int size = 0;
	int ret = BSP_ERR_MADAPT_OK;
	char *k_buffer = NULL;

	if (file == NULL) {
		hwlog_err("parse_write_file, NULL ptr!\n");
		return BSP_ERR_MADAPT_PTR_NULL;
	}

	if (file->len > MADAPT_MAX_USER_BUFF_LEN) {
		hwlog_err("parse_write_file, file->len(%d) too large!\n",
					file->len);
		return BSP_ERR_MADAPT_PARAM_ERR;
	}

	if (file->modem_id == MADAPT_MODEMID_INVALID) {
		ret = madapt_remove_mbn_carrirer_file();
	}

	ret = madapt_read_mbn_file(file->file, &k_buffer, &size);
	if (ret != BSP_ERR_MADAPT_OK) {
		return ret;
	}

	if (file->modem_id  == MADAPT_MODEMID_INVALID) {
		/*speical for copy NV BIN file from odm to mnvm2:0 */
		ret = madapt_write_mbn_carrirer_file(k_buffer, size, MADAPT_RW_FILE_TRIL);

	} else {
#ifdef MADAPT_HISI_MODEM_NV_RW_API
		if (strstr(file->file,".ato") != NULL) {
			ret = madapt_parse_and_writeatonv(k_buffer, size);
		} else {
			ret = madapt_parse_and_writenv(k_buffer, size, file->modem_id);
		}
#else
		hwlog_err("mdrv nv API is null ,depend on HISI_BALONG_MODEM\n");
		ret = BSP_ERR_MADAPT_WRITE_NV_ERR;
#endif
	}


	kfree(k_buffer);
	k_buffer = NULL;
	return ret;
}

/*speical for copy NV BIN file from odm to mnvm2:0 */
static int madapt_copy_multi_mbn_files(struct madapt_file_stru_ex *file)
{
	int ret = BSP_ERR_MADAPT_OK;
	int size = 0;
	int index = 0;
	char *k_buffer = NULL;
	struct madapt_mbn_file_head new_head;

	for (index = 0; index < file->file_count; index++)
	{
		hwlog_err("madapt_copy_multi_mbn_files, start copy file index=%d\n", index );
		ret = madapt_read_mbn_file(file->file[index], &k_buffer, &size);
		if(ret != BSP_ERR_MADAPT_OK)
		{
			break;
		}

		if (index == 0) {
			/*first file, copy total nv bin file to mnvm2:0*/
			 ret = madapt_write_mbn_carrirer_file(k_buffer, size, MADAPT_RW_FILE_HEAD);
			 memcpy(&new_head, k_buffer, sizeof(struct madapt_mbn_file_head));
			 hwlog_err(" file index(%d) new head data_len is :%u\n", index, new_head.data_len);
		} else {
			ret = madapt_write_mbn_carrirer_file(k_buffer +sizeof(struct madapt_mbn_file_head),
					size - sizeof(struct madapt_mbn_file_head), MADAPT_RW_FILE_TRIL);
			/* increase file head length*/
			new_head.data_len +=  ((struct madapt_mbn_file_head*)k_buffer)->data_len;
			/*decrease head size*/
			new_head.data_len -= sizeof(struct madapt_mbn_file_head);
			hwlog_err(" file index(%d) new head data_len is :%u\n", index, new_head.data_len);
		}

		kfree(k_buffer);
		k_buffer = NULL;

		if (ret != BSP_ERR_MADAPT_OK)
		{
			break;
		}
	}

	if (ret == BSP_ERR_MADAPT_OK) {
		ret = madapt_write_mbn_carrirer_file((char*)&new_head, sizeof(new_head), MADAPT_RW_FILE_HEAD);
	}
	return ret;
}
#ifdef MADAPT_HISI_MODEM_NV_RW_API
static int madapt_parse_and_write_mbn_files(struct madapt_file_stru_ex *file)
{
	int ret = BSP_ERR_MADAPT_OK;
	int size = 0;
	int index = 0;
	char *k_buffer = NULL;

	for (index = 0; index < file->file_count; index++)
	{
		hwlog_err("madapt_parse_multi_mbn_file, start copy file index=%d\n", index );
		ret = madapt_read_mbn_file(file->file[index], &k_buffer, &size);
		if (ret != BSP_ERR_MADAPT_OK) {
			break;
		}

		if (NULL != strstr(file->file[index], ".ato")) {
			//decode nv ato file
			ret = madapt_parse_and_writeatonv(k_buffer, size);
		} else {
			ret = madapt_parse_and_writenv(k_buffer, size, file->modem_id);
		}

		kfree(k_buffer);
		k_buffer = NULL;
		if (ret != BSP_ERR_MADAPT_OK) {
			break;
		}
	}

	return ret;
}
#endif

static int madapt_parse_multi_mbn_file(struct madapt_file_stru_ex *file)
{
	int ret = BSP_ERR_MADAPT_OK;

	if (file == NULL) {
		hwlog_err("madapt_parse_multi_mbn_file, NULL ptr!\n");
		return BSP_ERR_MADAPT_PTR_NULL;
	}

	if (MADAPT_MODEMID_INVALID == file->modem_id) {
		(void)madapt_remove_mbn_carrirer_file();
		ret = madapt_copy_multi_mbn_files(file);
	} else {
#ifdef MADAPT_HISI_MODEM_NV_RW_API
		ret = madapt_parse_and_write_mbn_files(file);
#else
		hwlog_err("mdrv nv API is null ,depend on HISI_BALONG_MODEM\n");
		ret = BSP_ERR_MADAPT_WRITE_NV_ERR;
#endif
	}

	return ret;
}

static void do_proc_nv_file(struct work_struct *p_work)
{
	if (kbuf_ex != NULL) {
		work_ret = madapt_parse_multi_mbn_file(kbuf_ex);
	} else {
		work_ret = parse_write_file(kbuf);
	}

	if (work_ret != BSP_ERR_MADAPT_OK) {
		hwlog_err("do_proc_nv_file, parse_write_file fail with errcode(%d)!\n", (int)work_ret);
	} else {
		hwlog_err("do_proc_nv_file, proc all success\n");
	}

	madapt_free(kbuf_ex);
	kbuf_ex = NULL;
	madapt_free(kbuf);
	kbuf = NULL;
	complete(&(my_completion));
}

static ssize_t madapt_copy_user_data(void** dest_buf, size_t dest_buf_size, const char __user *src_buf , size_t src_buf_size)
{
	ssize_t ret = BSP_ERR_MADAPT_OK;
	*dest_buf = kmalloc(dest_buf_size, GFP_KERNEL);
	if (*dest_buf == NULL) {
		ret = BSP_ERR_MADAPT_MALLOC_FAIL;
		return ret;
	}

	memset(*dest_buf , 0, dest_buf_size);
	if (copy_from_user(*dest_buf, src_buf, src_buf_size)) {
		ret = BSP_ERR_MADAPT_COPY_FROM_USER_ERR;
		madapt_free(*dest_buf);
		*dest_buf = NULL;
	}

	return ret;
}

static ssize_t madapt_dev_copy_buf(const char __user *buf, size_t count)
{
	ssize_t ret = BSP_ERR_MADAPT_OK;

	if (count == sizeof(struct madapt_file_stru_ex)) {
		ret = madapt_copy_user_data((void**)&kbuf_ex, sizeof(struct madapt_file_stru_ex), buf, count);
		if (ret != BSP_ERR_MADAPT_OK) {
			madapt_free(kbuf_ex);
			kbuf_ex = NULL;
			return ret;
		}
	} else {
		ret = madapt_copy_user_data((void**)&kbuf, sizeof(struct madapt_file_stru), buf, count);
		if (ret != BSP_ERR_MADAPT_OK) {
			madapt_free(kbuf);
			kbuf = NULL;
			return ret;
		}
	}

	return ret;
}

static ssize_t madapt_check_buf_size(size_t count)
{
	ssize_t ret = BSP_ERR_MADAPT_OK;

	if ((count <= sizeof(unsigned int))
		|| (count >= sizeof(struct madapt_file_stru)
			&& count != sizeof(struct madapt_file_stru_ex))) {
		ret = BSP_ERR_MADAPT_PARAM_ERR;
	}

	return ret;
}

/*lint -save --e{529,527}*/
ssize_t madapt_dev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	ssize_t ret = BSP_ERR_MADAPT_OK;
	int ticks_left = 0;

	if (buf == NULL) {
		return BSP_ERR_MADAPT_PARAM_ERR;
	}

	ret = madapt_check_buf_size(count);
	if (ret != BSP_ERR_MADAPT_OK) {
		return ret;
	}

	mutex_lock(&madapt_mutex);
	ret = madapt_dev_copy_buf(buf, count);
	if (ret != BSP_ERR_MADAPT_OK) {
		mutex_unlock(&madapt_mutex);
		return ret;
	}

	schedule_work(&(test_work.proc_nv));

	ticks_left = wait_for_completion_timeout(&(my_completion), HZ*MADAPT_WORK_WAIT_TIMER);
	if (ticks_left == 0) {
		ret = BSP_ERR_MADAPT_TIMEOUT;
	} else if (work_ret == BSP_ERR_MADAPT_OK) {
		ret = work_ret;
	} else {
		ret = work_ret;
	}

	mutex_unlock(&madapt_mutex);
	return ret;
}

static int get_commbin_last_modify_time(void)
{
	int i;
	struct file *fp = NULL;
	int open_result = MADAPT_FAIL;
	modify_time = MADAPT_DEFAULT_MODIFY_TIME;

	for (i = 0; i < ARRAY_SIZE(commbin_path); i++) {
		fp = filp_open(commbin_path[i], O_RDONLY, 0);
		if (IS_ERR(fp)) {
			open_result = MADAPT_FAIL;
			continue;
		} else {
			open_result = MADAPT_SUCCESS;
			break;
		}
	}
	if (open_result == MADAPT_FAIL) {
		hwlog_err("get_commbin_last_modify_time, open file error!\n");
		return BSP_ERR_MADAPT_OPEN_FILE_ERR;
	}

	modify_time = file_inode(fp)->i_mtime.tv_sec;
	hwlog_info("get common bin file modify time：%lu\n", modify_time);
	filp_close(fp, NULL);

	return BSP_ERR_MADAPT_OK;
}

static void do_read_proc(struct work_struct *p_work)
{
	read_work_ret = get_commbin_last_modify_time();
#ifdef MADAPT_HISI_MODEM_NV_RW_API
	if (read_work_ret != BSP_ERR_MADAPT_OK)
		read_work_ret = madapt_read_nvcfg_result_flag();
#endif
	complete(&(my_read_completion));
}

ssize_t madapt_dev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	ssize_t ret = BSP_ERR_MADAPT_OK;
	int ticks_left = 0;
	char dest_buf[MADAPT_BIT_LENGTH_SIZE] = {0};
	char read_time_buf[MADAPT_BIT_LENGTH_SIZE] = {0};
	size_t read_size = count >= sizeof(dest_buf) ? sizeof(dest_buf) - 1 : count;

	if (buf == NULL || read_size <= 0)
		return BSP_ERR_MADAPT_PARAM_ERR;

	if (copy_from_user(dest_buf, buf, read_size))
		return BSP_ERR_MADAPT_COPY_FROM_USER_ERR;

	if (!strcmp(dest_buf, MADAPT_PARAM_COMMON)) {
		mutex_lock(&madapt_mutex);

		schedule_work(&(test_read_work.proc_nv));
		ticks_left = wait_for_completion_timeout(&(my_read_completion),
					HZ * MADAPT_READ_WORK_WAIT_TIMER);
		if (ticks_left == 0)
			ret = BSP_ERR_MADAPT_TIMEOUT;
		else
			ret = read_work_ret;

		if (ret != BSP_ERR_MADAPT_OK) {
			mutex_unlock(&madapt_mutex);
			return ret;
		}

#ifdef MADAPT_HISI_MODEM_NV_RW_API
		if (modify_time == MADAPT_DEFAULT_MODIFY_TIME)
			(void)sprintf(read_time_buf, "%u", nvcfg_result_flag);
		else
			(void)sprintf(read_time_buf, "%lu", modify_time);
#else
		(void)sprintf(read_time_buf, "%lu", modify_time);
#endif
		read_time_buf[MADAPT_BIT_LENGTH_SIZE - 1] = '\0';
		if (copy_to_user(buf, read_time_buf, sizeof(read_time_buf))) {
			mutex_unlock(&madapt_mutex);
			return BSP_ERR_MADAPT_COPY_TO_USER_ERR;
		}

		mutex_unlock(&madapt_mutex);
	}

	return ret;
}

/*lint -save --e{529}*/
int madapt_init(void)
{
	int ret = 0;
	dev_t dev = 0;
	unsigned int devno;

	/*dynamic dev num use*/
	ret = alloc_chrdev_region(&dev, 0, 1, MADAPT_DEVICE_NAME);
	madapt_major = MAJOR(dev);

	if (ret) {
		hwlog_err("madapt_init, madapt failed alloc :%d\n", madapt_major);
		return ret;
	}

	devno = MKDEV(madapt_major, 0);

	memset(&cdev, 0, sizeof(struct cdev));

	cdev_init(&cdev, &nv_fops);
	cdev.owner = THIS_MODULE;
	cdev.ops = &nv_fops;

	ret = cdev_add(&cdev, devno, 1);
	if (ret) {
		hwlog_err("madapt_init, cdev_add fail!\n");
		return ret;
	}

	madapt_class = class_create(THIS_MODULE, MADAPT_DEVICE_CLASS);
	if (IS_ERR(madapt_class)) {
		hwlog_err("madapt_init, class create failed!\n");
		return ret;
	}

	device_create(madapt_class, NULL, devno, NULL, MADAPT_DEVICE_NAME);

	mutex_init(&madapt_mutex);
	INIT_WORK(&(test_work.proc_nv), do_proc_nv_file);
	INIT_WORK(&(test_read_work.proc_nv), do_read_proc);
	init_completion(&(my_completion));
	init_completion(&my_read_completion);

	kbuf = NULL;
	kbuf_ex = NULL;
	hwlog_err("madapt_init, complete!\n");
	return BSP_ERR_MADAPT_OK;
}
/*lint -restore*/


void madapt_exit(void)
{
	cdev_del(&cdev);
	class_destroy(madapt_class);
	unregister_chrdev_region(MKDEV(madapt_major, 0), 1);
}

module_init(madapt_init);
module_exit(madapt_exit);
MODULE_AUTHOR("Hisilicon Drive Group");
MODULE_DESCRIPTION("madapt driver for Hisilicon");
MODULE_LICENSE("GPL");

/*lint -restore*/
