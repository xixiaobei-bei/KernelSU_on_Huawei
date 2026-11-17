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
* * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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

#ifndef __BASTET_COMM_H__
#define __BASTET_COMM_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef WIN32
#pragma warning(disable:4200) /* zero-sized array in struct/union */
#endif

#pragma pack(4)

#define BASTET_MODEM_NAME						"bastet_modem"

#define BST_MODEM_IOC_MAGIC					'j'

#define BST_MODEM_IOC_GET_MODEM_RAB_ID			_IOWR(BST_MODEM_IOC_MAGIC, 1, struct bastet_modem_rab_id)
#define BST_MODEM_IOC_GET_MODEM_RESET			_IOWR(BST_MODEM_IOC_MAGIC, 2, int32_t)

#define BST_MODEM_FIRST_MINOR					0
#define BST_MODEM_DEVICES_NUMBER				1
#define BST_MAX_REPLY_LEN				1024


struct bastet_packet_data {
       uint32_t cons;
	uint32_t len;
	uint8_t value[0]; //lint !e43
};

struct bastet_modem_packet {
	struct list_head list;
	struct bastet_packet_data data;
};

struct bastet_modem_driver_data {
        wait_queue_head_t read_wait;
        spinlock_t read_lock;
        struct list_head read_queue;
        int32_t modem_reset_status;
        int32_t queuelen;
};


struct bastet_modem_rab_id {
    uint16_t modem_id;
    uint16_t rab_id;
};



#define BASTET_MODEM_LOG_TAG					"bastet_modem"

#define BASTET_MODEM_DEBUG					0
#define BASTET_MODEM_INFO						1

#define BASTET_MODEM_LOGD(fmt, ...) \
	do { \
		if (BASTET_MODEM_DEBUG) { \
			printk(KERN_DEBUG "["BASTET_MODEM_LOG_TAG"] %s: "fmt"\n", __func__, ##__VA_ARGS__); \
		} \
	} while (0)

#define BASTET_MODEM_LOGI(fmt, ...) \
	do { \
		if (BASTET_MODEM_INFO) { \
			printk(KERN_INFO "["BASTET_MODEM_LOG_TAG"] %s: "fmt"\n", __func__, ##__VA_ARGS__); \
		} \
	} while (0)

#define BASTET_MODEM_LOGE(fmt, ...) \
	do { \
		printk(KERN_ERR "["BASTET_MODEM_LOG_TAG"] %s: "fmt"\n", __func__, ##__VA_ARGS__); \
	} while (0)






#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of bastetcomm.h */

