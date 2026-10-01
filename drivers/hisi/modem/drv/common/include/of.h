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

#ifndef __OF_H__
#define __OF_H__ 

#include <product_config.h>

#ifdef __KERNEL__
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#elif defined(_WRS_KERNEL) || defined(__OS_VXWORKS__) ||  defined(__OS_RTOSCK__) ||defined(__OS_RTOSCK_SMP__)
#ifdef __OS_VXWORKS__
#include <vmLibCommon.h>
#include <private/kernelBaseLibP.h>	/* kernelId */
#include <private/vmLibP.h>
#include <arch/arm/mmuArmArch6PalLib.h>
#endif

#include <stdlib.h>
#include <errno.h>
#include <osl_err.h>
#include <osl_types.h>
#include <osl_generic.h>
#include <bsp_print.h>

struct boot_param_header {
	__be32	magic;
	__be32	totalsize;
	__be32	off_dt_struct;
	__be32	off_dt_strings;
	__be32	off_mem_rsvmap;
	__be32	version;
	__be32	last_comp_version;
	__be32	boot_cpuid_phys;
	__be32	dt_strings_size;
	__be32	dt_struct_size;
};

struct of_device_id
{
	char       name[32];
	char       type[32];
	char       compatible[128];
	const void *data;
};

typedef u32 phandle;
typedef u32 ihandle;

struct property {
	char            *name;
	int             length;
	void            *value;
	struct property *next;
};

struct device_node {
	const char         *name;
	const char         *type;
	phandle            phandle;
	const char         *full_name;
	struct property    *properties;
	struct device_node *parent;
	struct device_node *child;
	struct device_node *sibling;
	struct device_node *next;  
	struct device_node *allnext;
	void               *data;
};

#define MAX_PHANDLE_ARGS 8
struct of_phandle_args {
	struct device_node *np;
	int                args_count;
	uint32_t           args[MAX_PHANDLE_ARGS];
};

static inline void of_node_put(struct device_node *node) { }
static inline struct device_node *of_node_get(struct device_node *node)
{
       return node;
}

#ifdef CONFIG_OF
int dts_n_addr_cells(struct device_node *np);
struct device_node *dts_find_node_by_name(struct device_node *from, const char *name);
int dts_property_read_u32_index(const struct device_node *np, const char *propname, u32 index, u32 *out_value);
struct device_node *dts_find_node_by_type(struct device_node *from, const char *type);
int dts_property_read_u64(const struct device_node *np, const char *propname, u64 *out_value);
struct device_node *dts_find_compatible_node(struct device_node *from, const char *type, const char *compat);
struct device_node *dts_find_node_by_path(const char *path);
struct device_node *dts_get_child_by_name(const struct device_node *node, const char *name);
struct device_node *dts_find_node_by_phandle(phandle handle);
struct device_node *dts_find_matching_node_and_match(struct device_node *from,const struct of_device_id *matches,const struct of_device_id **match);
struct device_node *dts_get_parent(const struct device_node *node);
int dts_parse_phandle_with_args(const struct device_node *np, const char *list_name, const char *cells_name, int index, struct of_phandle_args *out_args);
struct device_node *dts_get_next_child(const struct device_node *node, struct device_node *prev);
int dts_property_read_u32_array(const struct device_node *np, const char *propname, u32 *out_values, size_t sz);
struct device_node *dts_get_next_available_child(const struct device_node *node, struct device_node *prev);
int dts_count_phandle_with_args(const struct device_node *np, const char *list_name, const char *cells_name);
struct property *dts_find_property(const struct device_node *np, const char *name, int *lenp);
int dts_property_read_u16_array(const struct device_node *np, const char *propname, u16 *out_values, size_t sz);
struct device_node *dts_find_node_with_property(struct device_node *from, const char *prop_name);
int dts_property_read_string_index(struct device_node *np, const char *propname, int index, const char **output);
int dts_property_read_string(struct device_node *np, const char *propname, const char **out_string);
int dts_property_count_strings(struct device_node *np, const char *propname);
int dts_device_is_available(const struct device_node *device);
int dts_device_is_compatible(const struct device_node *device, const char *);
int dts_property_read_u8_array(const struct device_node *np, const char *propname, u8 *out_values, size_t sz);

const void *dts_get_property(const struct device_node *node, const char *name, int *lenp);
int dts_n_size_cells(struct device_node *np);
const struct of_device_id *dts_match_node(const struct of_device_id *matches, const struct device_node *node);
int dts_modalias_node(struct device_node *node, char *modalias, int len);
int dts_machine_is_compatible(const char *compat);
const __be32 *dts_prop_next_u32(struct property *prop, const __be32 *cur,u32 *pu);
const char *dts_prop_next_string(struct property *prop, const char *cur);
struct device_node *dts_parse_phandle(const struct device_node *np, const char *phandle_name, int index);
int dts_property_match_string(struct device_node *np, const char *propname, const char *string);

#define of_find_node_by_name(from, name)            dts_find_node_by_name(from,  name)
#define of_find_node_by_type(from, type)            dts_find_node_by_type(from,  type)
#define of_find_compatible_node(from, type, compat) dts_find_compatible_node(from, type, compat)
#define of_find_node_by_path(path)                  dts_find_node_by_path(path)
#define of_find_node_by_phandle(phandle)            dts_find_node_by_phandle(phandle)
#define of_get_parent(node)                         dts_get_parent(node)
#define of_get_next_child(node, prev)               dts_get_next_child(node, prev)
#define of_get_next_available_child(node, prev)     dts_get_next_available_child(node, prev)
#define of_get_child_by_name(node, name)            dts_get_child_by_name(node, name)
#define of_find_node_with_property(from, prop_name) dts_find_node_with_property(from, prop_name)
#define of_find_property(np, name, lenp)            dts_find_property(np, name, lenp)
#define of_device_is_compatible(np, compt)          dts_device_is_compatible(np, compt)
#define of_device_is_available(np)                  dts_device_is_available(np)
#define of_get_property(node, name, lenp)           dts_get_property(node, name, lenp)
#define of_n_addr_cells(np)                         dts_n_addr_cells(np)
#define of_n_size_cells(np)                         dts_n_size_cells(np)
#define of_match_node(matches, node)                dts_match_node(matches, node)
#define of_modalias_node(node, modalias, len)       dts_modalias_node(node, modalias, len)
#define of_parse_phandle(np, phandle_name, index)   dts_parse_phandle(np, phandle_name, index)
#define of_machine_is_compatible(compat)            dts_machine_is_compatible(compat)
#define of_prop_next_u32(prop, cur, pu)             dts_prop_next_u32(prop, cur, pu)
#define of_prop_next_string(prop, cur)              dts_prop_next_string(prop, cur)
#define of_find_matching_node_and_match(from, matches, match)      dts_find_matching_node_and_match(from, matches, match)
#define of_property_read_u32_index(np, propname, index, out_value) dts_property_read_u32_index(np, propname, index, out_value)
#define of_property_read_u8_array(np, propname, out_values, sz)    dts_property_read_u8_array(np, propname, out_values, sz)
#define of_property_read_u16_array(np, propname, out_values, sz)   dts_property_read_u16_array(np, propname, out_values, sz)
#define of_property_read_u32_array(np, propname, out_values, sz)   dts_property_read_u32_array(np, propname, out_values, sz)
#define of_property_read_u64(np, propname, out_values)             dts_property_read_u64(np, propname, out_values)
#define of_property_read_string(np, propname, out_string)          dts_property_read_string(np, propname, out_string)
#define of_property_read_string_index(np, propname, index, output) dts_property_read_string_index(np, propname, index, output)
#define of_property_match_string(np, propname, out_string)         dts_property_match_string(np, propname, out_string)
#define of_property_count_strings(np, propname)                    dts_property_count_strings(np, propname)
#define of_count_phandle_with_args(np, list_name, cells_name)      dts_count_phandle_with_args(np, list_name, cells_name)
#define of_parse_phandle_with_args(np, list_name, cells_name,index, out_args) \
	dts_parse_phandle_with_args(np, list_name, cells_name,index, out_args)

#define for_each_matching_node(d_n, matches) \
	for (d_n = of_find_matching_node(NULL, matches); d_n; \
	     d_n = of_find_matching_node(d_n, matches))

#define for_each_node_by_type(d_n, type_n) \
        for (d_n = of_find_node_by_type(NULL, type_n); d_n; \
             d_n = of_find_node_by_type(d_n, type_n))

#define for_each_child_of_node(parent, child) \
	for (child = of_get_next_child(parent, NULL); child != NULL; \
	     child = of_get_next_child(parent, child))

#define for_each_node_by_name(d_n, name_n) \
        for (d_n = of_find_node_by_name(NULL, name_n); d_n; \
             d_n = of_find_node_by_name(d_n, name_n))

#define for_each_available_child_of_node(parent, child) \
	for (child = of_get_next_available_child(parent, NULL); child != NULL; \
	     child = of_get_next_available_child(parent, child))

#define for_each_property_of_node(d_n, pp) \
            for (pp = d_n->properties; pp != NULL; pp = pp->next)

#define for_each_compatible_node(d_n, type_n, compatible) \
        for (d_n = of_find_compatible_node(NULL, type_n, compatible); d_n; \
             d_n = of_find_compatible_node(d_n, type_n, compatible))

#define for_each_node_with_property(d_n, prop_name) \
	for (d_n = of_find_node_with_property(NULL, prop_name); d_n; \
		 d_n = of_find_node_with_property(d_n, prop_name))

#define for_each_matching_node_and_match(d_n, matches, match) \
	for (d_n = of_find_matching_node_and_match(NULL, matches, match); \
	     d_n; d_n = of_find_matching_node_and_match(d_n, matches, match))
        
#define of_property_for_each_string(np, propname, prop, s)	\
            for (prop = of_find_property(np, propname, NULL),   \
                s = of_prop_next_string(prop, NULL);        \
                s;                      \
                s = of_prop_next_string(prop, s))

#define of_property_for_each_u32(np, propname, prop, p, u)	\
	for (prop = of_find_property(np, propname, NULL),	\
		p = of_prop_next_u32(prop, NULL, &u);		\
		p;						\
		p = of_prop_next_u32(prop, p, &u))
		


static inline struct device_node *of_find_matching_node(
	struct device_node *from,
	const struct of_device_id *matches)
{
	return of_find_matching_node_and_match(from, matches, NULL);
}
    
    static inline u64 of_read_number(const __be32 *cell, int size)
    {
        u64 ret = 0;
        while (size--)
        {
            ret = (ret << 32) | be32_to_cpu(*(cell));
            cell++;
        }
        return ret;
    }
    
    static inline unsigned long long of_read_ulong(const __be32 *cell, int size)
    {
        return of_read_number(cell, size);
    }
    
    static inline int of_get_child_count(const struct device_node *np)
    {
        int no = 0;
        struct device_node *child;
        
        for_each_child_of_node(np, child)
            no++;
    
        return no;
    }

int of_reg_info_get(struct device_node *np, int index, u64 *start, u64 *size);
void *of_iomap(struct device_node *np, int index);
unsigned int irq_of_parse_and_map(struct device_node *dev, int index);
int of_selftest(void);

#else /* CONFIG_OF */
static inline void bsp_dts_init(void)
{
	bsp_err("<%s> is stub\n", __FUNCTION__);
}

static inline int dt_device_is_compatible(const struct device_node *device, const char *name)
{
	bsp_info("dts is close\n");
	return 0;
}

static inline struct device_node *of_get_parent(const struct device_node *node)
{
	bsp_info("dts is close\n");
	return NULL;
}

static inline int of_selftest(void)
{
	bsp_err("<%s> is stub\n", __FUNCTION__);
	return 0;
}

static inline int of_reg_info_get(struct device_node *np, int index, u64 *start, u64 *size)
{
	bsp_err("<%s> is stub\n", __FUNCTION__);
	return 0;
}

static inline const char* of_node_full_name(struct device_node *np)
{
	bsp_info("dts is close\n");
	return "<no-node>";
}

static inline void *of_iomap(struct device_node *np, int index)
{
	bsp_err("<%s> is stub\n", __FUNCTION__);
	return NULL;
}

static inline unsigned int irq_of_parse_and_map(struct device_node *dev, int index)
{
	bsp_err("<%s> is stub\n", __FUNCTION__);
	return 0;
}

#define for_each_matching_node(dn, matches) \
	while (0)

static inline bool of_have_populated_dt(void)
{
	bsp_info("dts is close\n");
	return false;
}

#define for_each_child_of_node(parent, child) \
	while (0)


static inline int of_get_child_count(const struct device_node *np)
{
	bsp_info("dts is close\n");
	return 0;
}

static inline int dt_strings_count(struct device_node *np,const char *propname)
{
	bsp_info("dts is close\n");
	return -1;
}

static inline struct device_node *dt_child_by_name_get(const struct device_node *node,const char *name)
{
	bsp_info("dts is close\n");
	return NULL;
}
static inline struct device_node *of_find_node_by_name(struct device_node *from, const char *name)
{
	bsp_info("dts is close\n");
	return NULL;
}

static inline int dt_u32_array_read(const struct device_node *np, const char *propname, u32 *out_values, size_t sz)
{
	bsp_info("dts is close\n");
	*out_values = 0;
	return -1;
}

static inline struct device_node *dt_compatible_node_find(struct device_node *from,const char *type,const char *compat)
{
	bsp_info("dts is close\n");
	return NULL;
}

static inline int dt_u8_array_read(const struct device_node *np, const char *propname, u8 *out_values, size_t sz)
{
	bsp_info("dts is close\n");
	return -1;
}

static inline int dt_device_is_available(const struct device_node *device)
{
	bsp_info("dts is close\n");
	return 0;
}

 static inline int dt_u32_index_read(const struct device_node *np, const char *propname, u32 index, u32 *out_value)
{
	bsp_info("dts is close\n");
	return -1;
}

static inline int dt_phandle_with_args_count(struct device_node *np,const char *list_name, const char *cells_name)
{
	bsp_info("dts is close\n");
	return -1;
}

static inline int dt_string_read(struct device_node *np,const char *propname, const char **out_string)
{
	bsp_info("dts is close\n");
	*out_string = NULL;
	return -1;
}

static inline const void *dt_property_get(const struct device_node *node,const char *name,int *lenp)
{
	bsp_info("dts is close\n");
	*lenp = 0;
	return NULL;
}

static inline int dt_u16_array_read(const struct device_node *np, const char *propname, u16 *out_values, size_t sz)
{
	bsp_info("dts is close\n");
	return -1;
}

static inline int dt_phandle_with_args_parse(struct device_node *np,const char *list_name,const char *cells_name,int index,struct of_phandle_args *out_args)
{
	bsp_info("dts is close\n");
	return -1;
}

static inline int dt_string_index_read(struct device_node *np,const char *propname, int index, const char **out_string)
{
	bsp_info("dts is close\n");
	*out_string = NULL;
	return -1;
}

static inline struct property *dt_property_find(const struct device_node *np,const char *name,int *lenp)
{
	bsp_info("dts is close\n");
	return NULL;
}

static inline struct device_node *dt_prop_handle_parse(const struct device_node *np,const char *phandle_name,int index)
{
	bsp_info("dts is close\n");
	return NULL;
}

static inline int dt_u64_read(const struct device_node *np,const char *propname, u64 *out_value)
{
	bsp_info("dts is close\n");
	*out_value = 1;
	return -1;
}

static inline int dt_property_string_match(struct device_node *np,const char *propname,const char *string)
{
	bsp_info("dts is close\n");
	return -1;
}

static inline int dt_machine_is_compatible(const char *compat)
{
	bsp_info("dts is close\n");
	return 0;
}

#define of_property_for_each_string(np, propname, prop, s) \
	while (0)

static inline int dt_alias_id_get(struct device_node *np, const char *stem)
{
	bsp_info("dts is close\n");
	return -1;
}

#define of_device_is_compatible(np, compt)      dt_device_is_compatible(np, compt)
#define of_property_read_string_index(np, propname, index, output) dt_string_index_read(np, propname, index, output)
#define of_property_count_strings(np, propname)                    dt_strings_count(np, propname)
#define of_device_is_available(np)                  dt_device_is_available(np)
#define of_find_property(np, name, lenp)            dt_property_find(np, name, lenp)
#define of_property_read_u32_array(np, propname, out_values, sz)   dt_u32_array_read(np, propname, out_values, sz)
#define of_property_read_string(np, propname, index, output) dt_string_read(np, propname, index, output)
#define of_get_property(node, name, lenp)           dt_property_get(node, name, lenp)
#define of_property_read_u8_array(np, propname, out_values, sz)    dt_u8_array_read(np, propname, out_values, sz)
#define of_property_read_u16_array(np, propname, out_values, sz)   dt_u16_array_read(np, propname, out_values, sz)
#define of_property_read_u64(np, propname, out_values)             dt_u64_read(np, propname, out_values)
#define of_find_compatible_node(from, type, compat) dt_compatible_node_find(from, type, compat)
#define of_property_read_u32_index(np, propname, index, out_value) dt_u32_index_read(np, propname, index, out_value)
#define of_property_match_string(np, propname, out_string)         dt_property_string_match(np, propname, out_string)
#define of_get_child_by_name(node, name)        of_get_child_by_name(node, name)
#define of_parse_phandle_with_args(np, list_name, cells_name,index, out_args) \
	dt_prop_handle_parse(np, list_name, cells_name,index, out_args)
#define of_count_phandle_with_args(np, list_name, cells_name)      dt_phandle_with_args_count(np, list_name, cells_name)
#define of_property_for_each_u32(np, propname, prop, p, u) \
	while (0)
#define of_alias_get_id(np, stem)       dt_alias_id_get(np, stem)
#define of_match_node(_matches, _node)	NULL
#define of_machine_is_compatible(compat)            dt_machine_is_compatible(compat)
#define of_match_ptr(_ptr)	NULL

#endif /* CONFIG_OF */

static inline bool of_property_read_bool(const struct device_node *np,
					 const char *propname)
{
	struct property *prop = of_find_property(np, propname, NULL);

	return prop ? true : false;/* [false alarm]:ÆÁ±ÎFortify´íÎó */
}

static inline int of_property_read_u8(const struct device_node *np,
				       const char *propname,
				       u8 *out_value)
{
	return of_property_read_u8_array(np, propname, out_value, 1);
}

static inline int of_property_read_u16(const struct device_node *np,
				       const char *propname,
				       u16 *out_value)
{
	return of_property_read_u16_array(np, propname, out_value, 1);
}

static inline int of_property_read_u32(const struct device_node *np,
				       const char *propname,
				       u32 *out_value)
{
	return of_property_read_u32_array(np, propname, out_value, 1);
}

#endif /* __KERNEL__ */
#endif /* __OF_H__ */
