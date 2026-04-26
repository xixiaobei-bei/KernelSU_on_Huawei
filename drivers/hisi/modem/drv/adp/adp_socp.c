/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#include "bsp_socp.h"
#include "bsp_slice.h"


#ifndef ENABLE_BUILD_SOCP /* do not build socp, stub */

int mdrv_socp_corder_set_src_chan(unsigned int chan_id, socp_src_chan_cfg_s *src_attr)
{
    return 0;
}
int mdrv_socp_corder_set_dst_chan(unsigned int dest_chan_id, socp_dst_chan_cfg_s *dest_attr)
{
    return 0;
}

int mdrv_socp_start(unsigned int chan_id)
{
    return 0;
}
int mdrv_socp_stop(unsigned int chan_id)
{
    return 0;
}
int mdrv_socp_get_write_buff(unsigned int chan_id, socp_buffer_rw_s *buffer)
{
    return 0;
}
int mdrv_socp_write_done(unsigned int chan_id, unsigned int size)
{
    return 0;
}
int mdrv_socp_get_read_buffer(unsigned int dest_chan_id, socp_buffer_rw_s *buffer)
{
    return 0;
}
unsigned int mdrv_socp_get_sd_logcfg(socp_encdst_buf_log_cfg_s *cfg)
{
    return 0;
}
int mdrv_socp_set_ind_mode(unsigned int mode)
{
    return 0;
}
int mdrv_socp_compress_enable(unsigned int dest_chan_id)
{
    return 0;
}
void mdrv_socp_send_data_manager(unsigned int chan_id, unsigned int enbale)
{
    return;
}
int mdrv_clear_socp_buff(unsigned int src_chan_id)
{
    return 0;
}

int mdrv_socp_set_cfg_ind_mode(socp_ind_mode_e mode)
{
    return 0;
}
int mdrv_socp_get_cfg_ind_mode(unsigned int *mode)
{
    return 0;
}
int mdrv_socp_set_cps_ind_mode(deflate_ind_compress_e mode)
{
    return 0;
}
int mdrv_socp_get_cps_ind_mode(unsigned int *mode)
{
    return 0;
}
void mdrv_clear_socp_encdst_int_info(void)
{
    return;
}

#else

void mdrv_socp_send_data_manager(unsigned int chan_id, unsigned int enbale)
{
    bsp_socp_data_send_manager(chan_id, enbale);
}
EXPORT_SYMBOL(mdrv_socp_send_data_manager);

int mdrv_socp_corder_set_src_chan(unsigned int chan_id, socp_src_chan_cfg_s *src_attr)
{
    return bsp_socp_coder_set_src_chan(chan_id, src_attr);
}
EXPORT_SYMBOL(mdrv_socp_corder_set_src_chan);

int mdrv_socp_coder_set_dst_chan(unsigned int dest_chan_id, socp_dst_chan_cfg_s *dest_attr)
{
    return bsp_socp_coder_set_dst_chan(dest_chan_id, dest_attr);
}
EXPORT_SYMBOL(mdrv_socp_coder_set_dst_chan);

int mdrv_socp_start(unsigned int chan_id)
{
    return bsp_socp_start(chan_id);
}
EXPORT_SYMBOL(mdrv_socp_start);

int mdrv_socp_stop(unsigned int chan_id)
{
    return bsp_socp_stop(chan_id);
}
EXPORT_SYMBOL(mdrv_socp_stop);

int mdrv_socp_get_write_buff(unsigned int chan_id, socp_buffer_rw_s *buffer)
{
    return bsp_socp_get_write_buff(chan_id, buffer);
}
EXPORT_SYMBOL(mdrv_socp_get_write_buff);

int mdrv_socp_write_done(unsigned int chan_id, unsigned int size)
{
    return bsp_socp_write_done(chan_id, size);
}
EXPORT_SYMBOL(mdrv_socp_write_done);

unsigned int mdrv_socp_get_sd_logcfg(socp_encdst_buf_log_cfg_s *cfg)
{
    return bsp_socp_get_log_cfg(cfg);
}
EXPORT_SYMBOL(mdrv_socp_get_sd_logcfg);

unsigned int mdrv_socp_get_log_cfg(socp_encdst_buf_log_cfg_s *cfg)
{
    return bsp_socp_get_log_cfg(cfg);
}
EXPORT_SYMBOL(mdrv_socp_get_log_cfg);

int mdrv_socp_set_ind_mode(socp_ind_mode_e mode)
{
    return bsp_socp_set_ind_mode(mode);
}
EXPORT_SYMBOL(mdrv_socp_set_ind_mode);

int mdrv_socp_set_cfg_ind_mode(socp_ind_mode_e mode)
{
    return bsp_socp_set_log_cfg_mode(mode);
}
EXPORT_SYMBOL(mdrv_socp_set_cfg_ind_mode);
int mdrv_socp_get_cfg_ind_mode(unsigned int *mode)
{
    return bsp_socp_get_log_cfg_state(mode);
}
int mdrv_socp_set_cps_ind_mode(deflate_ind_compress_e mode)
{
    return 0;
}
int mdrv_socp_get_cps_ind_mode(unsigned int *mode)
{
    return 0;
}

int mdrv_clear_socp_buff(unsigned int src_chan_id)
{
    return bsp_clear_socp_src_buffer(src_chan_id);
}

int mdrv_clear_socp_dst_buffer(unsigned int dst_chan_id)
{
    return bsp_clear_socp_dst_buffer(dst_chan_id);
}
void mdrv_socp_mntn_enc_dst_int_info(unsigned int *TrfInfo, unsigned int *ThrOvfInfo)
{
    bsp_socp_mntn_enc_dst_int_info(TrfInfo, ThrOvfInfo);
}
void mdrv_clear_socp_encdst_int_info(void)
{
    bsp_clear_socp_encdst_int_info();
}

int mdrv_socp_get_read_buffer(unsigned int dest_chan_id, socp_buffer_rw_s *buffer)
{
    return bsp_socp_get_read_buff(dest_chan_id, buffer);
}

#endif /* ENABLE_BUILD_SOCP */

