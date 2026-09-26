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

#include <bsp_dt.h>

device_node_s *bsp_dt_find_compatible_node(device_node_s *from, const char *type, const char *compat)
{
    return of_find_compatible_node(from, type, compat);
}

int bsp_dt_property_read_u32(const device_node_s *np, const char *propname, u32 *out_value)
{
    return of_property_read_u32(np, propname, out_value);
}

int bsp_dt_property_read_u32_array(const device_node_s *np,
    const char *propname, u32 *out_values, size_t sz)
{
    return of_property_read_u32_array(np, propname, out_values, sz);
}

int bsp_dt_property_read_u32_index(const device_node_s *np,
    const char *propname, u32 index, u32 *out_value)
{
    return of_property_read_u32_index(np, propname, index, out_value);
}

const unsigned int *bsp_dt_prop_next_u32(struct property *prop, const unsigned int *curu, u32 *pu)
{
    return of_prop_next_u32(prop, curu, pu);
}

int bsp_dt_get_child_count(const device_node_s *np)
{
    return of_get_child_count(np);
}

device_node_s *bsp_dt_get_child_by_name(const device_node_s *np, const char *name)
{
    return of_get_child_by_name(np, name);
}

int bsp_dt_n_addr_cells(device_node_s *np)
{
    return of_n_addr_cells(np);
}

int bsp_dt_n_size_cells(device_node_s *np)
{
    return of_n_size_cells(np);
}

const void *bsp_dt_get_property(const device_node_s *np, const char *name, int *lenp)
{
    return of_get_property(np, name, lenp);
}

u64 bsp_dt_read_number(const __be32 *cell, int size)
{
    return of_read_number(cell, size);
}

struct property *bsp_dt_find_property(const device_node_s *np, const char *name, int *lenp)
{
    return of_find_property(np, name, lenp);
}

int bsp_dt_property_read_string(const device_node_s *np,
    const char *propname, const char **out_string)
{
    return of_property_read_string(np, propname, out_string);
}

int bsp_dt_property_read_string_index(const device_node_s *np,
    const char *propname, int index, const char **output)
{
    return of_property_read_string_index(np, propname, index, output);
}

int bsp_dt_property_count_strings(const device_node_s *np, const char *propname)
{
    return of_property_count_strings(np, propname);
}

bool bsp_dt_device_is_available(const device_node_s *np)
{
    return of_device_is_available(np);
}

device_node_s *bsp_dt_get_next_available_child(const device_node_s *np, device_node_s *prev)
{
    return of_get_next_available_child(np, prev);
}

void __iomem *bsp_dt_iomap(device_node_s *np, int index)
{
    return of_iomap(np, index);
}

unsigned int bsp_dt_irq_parse_and_map(device_node_s *np, int index)
{
    return irq_of_parse_and_map(np, index);
}
