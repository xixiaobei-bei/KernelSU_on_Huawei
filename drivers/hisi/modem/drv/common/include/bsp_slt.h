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

#ifndef __BSP_SLT_H__
#define __BSP_SLT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "product_config.h"
#include <soc_memmap_comm.h>
#include "hi_uart.h"


#ifdef MBB_SLT
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a)/(sizeof((a)[0])))
#endif

#define SLT_VECTOR_VERSION  "v1.0.0"

typedef struct slt_vector_wrap{
    int (*func)(void);
    unsigned int is_while;  /* when failed, while(1) or go ahead */
    unsigned int is_bad;    /* when failed, if vote to be bad chip */
    unsigned int pass_gpio;
    unsigned int fail_gpio;
    char case_name[32];
}slt_vector_wrap_s;

int slt_get_software_ver (void);
int slt_get_die_id(void);
int slt_get_tsensor(void);
int slt_set_bsn (char * bsn);
int slt_get_bsn (char * bsn);
int slt_pcie_test (void);
int slt_usb_test (int usb_mode);
int slt_SGMII_test (void);
int slt_RGMII_test (void);
int slt_spi_test (void);
int slt_sdcard_test (void);
int slt_acore_l2cache_test(void);
int bsp_slt_hifi_test (void);
#else


static inline int slt_get_software_ver (void)
{
    return 0;
}

static inline int slt_get_die_id(void)
{
    return 0;
}

static inline int slt_get_tsensor(void)
{
    return 0;
}

static inline int slt_set_bsn (char * bsn)
{
    return 0;
}

static inline int slt_get_bsn (char * bsn)
{
    return 0;
}

static inline int slt_pcie_test (void)
{
    return 0;
}

static inline int slt_usb_test (int usb_mode)
{
    return 0;
}

static inline int slt_SGMII_test (void)
{
    return 0;
}

static inline int slt_RGMII_test (void)
{
    return 0;
}

static inline int slt_spi_test (void)
{
    return 0;
}

static inline int slt_sdcard_test (void)
{
    return 0;
}

static inline int slt_acore_l2cache_test(void)
{
    return 0;
}

static inline int bsp_slt_hifi_test (void)
{
    return 0;
}

#endif  /*__ASSEMBLY__*/



#ifdef __cplusplus
}
#endif

#endif


