/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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

#ifndef __ADS_PS_IFACE_DEBUG_H__
#define __ADS_PS_IFACE_DEBUG_H__

#include <linux/kernel.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*lint -emacro({717}, ADS_LOGH)*/
#define ADS_LOGH(fmt, ...) \
	do { \
		pr_err("[NAS_ADS]: [HIGH] %s "   fmt"\n", __func__, ##__VA_ARGS__); \
	} while(0)

/*lint -emacro({717}, ADS_LOGE)*/
#define ADS_LOGE(fmt, ...) \
	do { \
		pr_err("[NAS_ADS]: [ERR] %s "   fmt"\n", __func__, ##__VA_ARGS__); \
	} while(0)

/*lint -emacro({717}, ADS_LOGI)*/
#define ADS_LOGI(fmt, ...)\
	do {\
		pr_warn("[NAS_ADS]: [INFO] %s "   fmt"\n", __func__, ##__VA_ARGS__); \
	} while(0)

/*lint -emacro({717}, ADS_LOGD)*/
#define ADS_LOGD(fmt, ...) \
	do {\
		pr_notice("[NAS_ADS]: [DBG] %s "   fmt"\n", __func__, ##__VA_ARGS__); \
	} while(0)


#define ADS_MEMCPY_RET_CHECK(ret, dest_size, src_size) \
	do {\
		if (unlikely(ret)) \
			ADS_LOGH("memcpy failed. dest size = %lu, src size = %lu, Ret = %d", dest_size, src_size, ret); \
	} while(0)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __ADS_PS_IFACE_DEBUG_H__ */
