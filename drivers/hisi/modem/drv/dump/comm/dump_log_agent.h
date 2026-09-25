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
#ifndef __DUMP_LOG_AGENT_H__
#define __DUMP_LOG_AGENT_H__
#include <osl_types.h>
#define DUMP_FILE_TRANS_MAGIC            (0x56786543)
#define DUMP_FILE_LINKS_MAGIC            (0xcdefabcd)
struct dump_file_save_info_s
{
    u32     magic;              /*0x56786543*/
    u32     links;              /*0xcdefabcd*/
    char    filename[32];
    char    dstfilename[32];
    u32     filenum;
    u32     totalnum;
    u32     packetnum;          /*包序号*/
    u32     totalpacket;        /*总包数*/
    u32     filelength;         /*文件实际长度*/
    u32     realLength;         /*单个包实际长度*/
    u32     isAppend;           /*是否增加文件*/
    u32     maxLength;          /*文件最大长度*/
};
struct dump_dstname_list
{
    char filename[32];
};
struct dump_file_save_strategy
{
    u32 is_compress;
    u32 is_append;
    struct dump_file_save_info_s data_head;
};

struct dump_log_agent_stru
{
    bool link_enable;
    u32 log_sum;
    struct dump_file_save_strategy *log_strategy;
};

bool dump_get_link_status(void);
s32 dump_match_log_strategy(const char* file_name,struct dump_file_save_strategy *output);
void dump_log_agent_init(void);
#endif