/* MD5: 111363b0f3657acc68d0200b1e635902*/
/*
 * copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#if !defined(__PRODUCT_CONFIG_TOOL_H__)
#define __PRODUCT_CONFIG_TOOL_H__

#ifndef PRODUCT_CONTROL_VERSION      		
#define PRODUCT_CONTROL_VERSION      		 "141" 
#endif 

#ifndef PRODUCT_HISILICON_VERSION
#define PRODUCT_HISILICON_VERSION denver 
#endif 

#ifndef PRODUCT_DIFF_NV_FILTER          	
#define PRODUCT_DIFF_NV_FILTER          	 denver_udp 
#endif 

#ifndef PRODUCT_NV_FILE_MAX_SIZE           	
#define PRODUCT_NV_FILE_MAX_SIZE           	 binfileLimitSize "10000" 
#endif 

#ifndef PRODUCT_NV_BIN_MODEM_NUNBER       	
#define PRODUCT_NV_BIN_MODEM_NUNBER       	 bin_modem_number "3" 
#endif 

#ifndef PRODUCT_COMMON_NV_FILE_FORMAT		
#define PRODUCT_COMMON_NV_FILE_FORMAT		 common_nv_file_format "1" 
#endif 

#ifndef PRODUCT_XML_FOLDER_HIONE			
#define PRODUCT_XML_FOLDER_HIONE			 1 
#endif 

#ifndef PRODUCT_NV_COMPRESS             	
#define PRODUCT_NV_COMPRESS             	 compress_nv_file "1" 
#endif 

#ifndef PRODUCT_NV_XNV_PDT_VERSION        	
#define PRODUCT_NV_XNV_PDT_VERSION        	 hi9500 
#endif 

#ifndef PRODUCT_NV_PARSE_CUSTOMIZE			
#define PRODUCT_NV_PARSE_CUSTOMIZE			 parse_customize "1" 
#endif 

#ifndef PRODUCT_VERSION_AFTER_B5000			
#define PRODUCT_VERSION_AFTER_B5000			 1 
#endif 

#ifndef PRODUCT_COMMON_NV_INHERIT_BY_PRODUCT		
#define PRODUCT_COMMON_NV_INHERIT_BY_PRODUCT		 true 
#endif 

#ifndef PRODUCT_SPLIT_NV_BUILD
#define PRODUCT_SPLIT_NV_BUILD true 
#endif 

#ifndef PRODUCT_NCFG_DISTINGUISH_MODEM_FEATURE_ON
#define PRODUCT_NCFG_DISTINGUISH_MODEM_FEATURE_ON 
#endif 

#ifndef DIAG_SEC_TOOLS
#define DIAG_SEC_TOOLS 
#endif 

#ifndef PRODUCT_NCFG_INDEPENDENT_FEATURE_NV
#define PRODUCT_NCFG_INDEPENDENT_FEATURE_NV 
#endif 

#ifdef RF0_TYPE_DV200 
#ifndef RF_NV_PLATFORM_DPV200				
#define RF_NV_PLATFORM_DPV200				 _dpv200 
#endif 

#endif
#endif /*__PRODUCT_CONFIG_H__*/ 
