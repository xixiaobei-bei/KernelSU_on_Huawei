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

/*****************************************************************************/
/*                                                                           */
/*                Copyright 1999 - 2008, Huawei Tech. Co., Ltd.              */
/*                           ALL RIGHTS RESERVED                             */
/*                                                                           */
/* FileName: OMRingBuffer.c                                                  */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/* Version: 1.0                                                              */
/*                                                                           */
/* Date: 2008-06                                                             */
/*                                                                           */
/* Description: implement ring buffer subroutine                             */
/*                                                                           */
/* Others:                                                                   */
/*                                                                           */
/* History:                                                                  */
/* 1. Date:                                                                  */
/*    Author:                                                                */
/*    Modification: Adapt this file                                          */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
#include <linux/module.h>
#include <securec.h>
#include <bsp_dump.h>
#include <osl_spinlock.h>
#include <osl_malloc.h>
#include "ring_buffer.h"
#include "soft_decode.h"


#define  THIS_MODU mod_soft_dec

#define OM_MIN(x, y)             (((x) < (y)) ? (x) : (y))

OM_RING_ID diag_RingBufferCreate(int nbytes)
{
    char         *buffer = NULL;
    OM_RING_ID    ringId = NULL;

    buffer = (char *) osl_malloc((u32)nbytes);
    if (buffer == NULL)
    {
        return NULL;
    }

    ringId = (OM_RING_ID)osl_malloc(sizeof(OM_RING));
    if (ringId == NULL) {
        osl_free(buffer);
        return NULL;
    }

    ringId->bufSize = nbytes;
    ringId->buf     = buffer;

    diag_RingBufferFlush(ringId);

    return (ringId);
}

void diag_RingBufferFlush( OM_RING_ID ringId )
{
    ringId->pToBuf   = 0;
    ringId->pFromBuf = 0;
}

void diag_get_data_buffer(OM_RING_ID ring_buffer, rw_buffer_s *rw_buff)
{
    if (ring_buffer->pFromBuf <= ring_buffer->pToBuf) {
        /* 写指针大于读指针，直接计算 */
        rw_buff->buffer = ring_buffer->buf + ring_buffer->pFromBuf;
        rw_buff->size = ring_buffer->pToBuf - ring_buffer->pFromBuf;
        rw_buff->rb_buffer = NULL;
        rw_buff->rb_size = 0;
    } else {
        /* 读指针大于写指针，需要考虑回卷 */
        rw_buff->buffer = ring_buffer->buf + ring_buffer->pFromBuf;
        rw_buff->size = ring_buffer->bufSize - ring_buffer->pFromBuf ;
        rw_buff->rb_buffer = ring_buffer->buf;
        rw_buff->rb_size = ring_buffer->pToBuf;
    }
}

s32 diag_RingBufferGet( OM_RING_ID rngId, rw_buffer_s rw_buff, u8 *buffer, int data_len) 
{
    s32 ret;

    if (data_len == 0) {
        return 0;
    }

    ret = memcpy_s(buffer, data_len, rw_buff.buffer, rw_buff.size);
    if (ret) {
        soft_decode_error("memcpy_s fail, ret=0x%x\n", ret);
        return ret;
    }
    
    if (rw_buff.rb_size != 0) {
        ret = memcpy_s(buffer + rw_buff.size, data_len - rw_buff.size, rw_buff.rb_buffer, data_len - rw_buff.size);
        if (ret) {
            soft_decode_error("memcpy_s fail, ret=0x%x\n", ret);
            return ret;
        }
        
        rngId->pFromBuf = data_len - rw_buff.size;
    } else {
        rngId->pFromBuf += data_len;
    }

    return 0;
}

void diag_get_idle_buffer(OM_RING_ID ring_buffer, rw_buffer_s *rw_buff)
{
    if (ring_buffer->pToBuf < ring_buffer->pFromBuf) {
        /* 读指针大于写指针，直接计算 */
        rw_buff->buffer = ring_buffer->buf + ring_buffer->pToBuf;
        rw_buff->size = (ring_buffer->pFromBuf - ring_buffer->pToBuf - 1);
        rw_buff->rb_buffer = NULL;
        rw_buff->rb_size = 0;
    } else {
        /* 写指针大于读指针，需要考虑回卷 */
        if (ring_buffer->pFromBuf != 0) {
            rw_buff->buffer = ring_buffer->buf + ring_buffer->pToBuf;
            rw_buff->size = ring_buffer->bufSize   - ring_buffer->pToBuf ;
            rw_buff->rb_buffer = ring_buffer->buf;
            rw_buff->rb_size = ring_buffer->pFromBuf - 1;
        } else {
            rw_buff->buffer = ring_buffer->buf + ring_buffer->pToBuf;
            rw_buff->size = ring_buffer->bufSize   - ring_buffer->pToBuf - 1;
            rw_buff->rb_buffer = NULL;
            rw_buff->rb_size = 0;
        }
    }
}

s32 diag_RingBufferPut( OM_RING_ID rngId, rw_buffer_s rw_buffer, const u8 *buffer, int data_len)
{
    u32 size;
    u32 rb_size;
    s32 ret;

    if (data_len == 0) {
        return 0;
    }

    if (rw_buffer.size > data_len) {
        if ((rw_buffer.buffer) && (rw_buffer.size)) {
            ret = memcpy_s(((u8 *)rw_buffer.buffer), rw_buffer.size, buffer, data_len);
            if (ret != EOK) {
                soft_decode_error("memory copy fail 0x%x\n", ret);
                return ret;
            }
        }
        
        rngId->pToBuf += data_len;
    } else {
        if ((rw_buffer.buffer) && (rw_buffer.size)) {
            size = rw_buffer.size;
            ret = memcpy_s(((u8 *)rw_buffer.buffer), rw_buffer.size, buffer, size);
            if (ret != EOK) {
                soft_decode_error("memory copy fail 0x%x\n", ret);
                return ret;
            }
        } else {
            size = 0;
        }

        rb_size = data_len - rw_buffer.size;
        if (rb_size && rw_buffer.rb_buffer != NULL) {
            ret = memcpy_s((u8 *)rw_buffer.rb_buffer, rw_buffer.rb_size, ((u8 *)buffer + size), rb_size);
            if (ret != EOK) {
                soft_decode_error("memory copy fail 0x%x\n", ret);
                return ret;
            }
        }
        
        rngId->pToBuf = rb_size;
    }

    return 0;
}


int __init diag_ring_buffer_init(void)
{
    return BSP_OK;
}


