#include <linux/string.h>
#include <linux/delay.h>
#include <of.h>
#include <n_bsp_ipf.h>
#include "ipf_balong.h"
#include <bsp_slice.h>
#include <bsp_ipc.h>
#include <bsp_module.h>
#include <osl_malloc.h>
#include <osl_spinlock.h>
/*

#define FILTER_RIP  10
extern struct ipf_ctx g_ipf_ctx;

static void ipf_write_basic_filter_ex(unsigned int id, ipf_filter_node_s* match_infos);
static void ipf_read_basic_filter_ex(unsigned int id, ipf_filter_node_s* filter);
static void ipf_set_next_basic_filter_ex(unsigned int id, unsigned int next);

struct ipf_filter_handler handler[] = {
    {
        .bf_num = IPF_BF_NUM_EX,
        .total = IPF_TOTAL_FILTER_NUM_EX,
        .tail = IPF_TAIL_INDEX_EX,
        .basic_write = ipf_write_basic_filter_ex,
        .basic_read = ipf_read_basic_filter_ex,
        .launched = {0},
        .free = 0,
        .basic_set_next = ipf_set_next_basic_filter_ex,
    },
};

struct ipf_filter_handler* ipf_get_filter_handler(unsigned int version_id)
{
    if(version_id>IPF_VERSION_140a)
        return (struct ipf_filter_handler*)&handler[0];
    else
        return (struct ipf_filter_handler*)&handler[0];
}

static void ipf_write_basic_filter_ex(unsigned int id, ipf_filter_node_s* filter)
{
    unsigned int i;
    unsigned int* reg = (unsigned int*)filter;

    for(i=0; i<(sizeof(ipf_filter_node_s)/sizeof(unsigned int)); i++)
    {
        ipf_writel(reg[i], (HI_IPF_FLTN_LOCAL_ADDR0_OFFSET(id)+i*4)); 
    }
}

static void ipf_read_basic_filter_ex(unsigned int id, ipf_filter_node_s* filter)
{
    unsigned int i;
    unsigned int* reg = (unsigned int*)filter;
  
    for(i=0; i<(sizeof(ipf_filter_node_s)/sizeof(unsigned int)); i++)
    {
        reg[i] = ipf_readl(HI_IPF_FLTN_LOCAL_ADDR0_OFFSET(id)+i*4); 
    }
}

static void ipf_set_next_basic_filter_ex(unsigned int id, unsigned int next)
{
    HI_IPF_FLTN_CHAIN_0_T fltn_chain;
    
    fltn_chain.u32 = ipf_readl(HI_IPF_FLTN_CHAIN_OFFSET(id));
    fltn_chain.bits.fltn_next_index = next;
    ipf_writel(fltn_chain.u32, HI_IPF_FLTN_CHAIN_OFFSET(id));
}


void ipf_write_filter(unsigned int id, ipf_filter_node_s* match_infos)
{
    struct ipf_filter_handler* fh = g_ipf_ctx.filter_handler;
    struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();

    
    
    if(id< fh->bf_num){
        fh->basic_write(id, match_infos);
    }

    memcpy(&sm->filter[id], match_infos, sizeof(ipf_filter_node_s));

    cache_sync();
}


void ipf_read_filter(unsigned int id, ipf_filter_node_s * filter)
{
    struct ipf_filter_handler* fh = g_ipf_ctx.filter_handler;
    struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();
    
    if(id< (unsigned int)fh->bf_num){
        fh->basic_read(id, filter);
    }

    cache_sync();
}


__init int ipf_filterlist_init(void)
{
    unsigned int i;
    struct ipf_filter_handler* fh = g_ipf_ctx.filter_handler;
    
    fh->free = osl_malloc(fh->total*sizeof(filter_map));
    if(NULL == fh->free){
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF, "\r IPF_FilterList_Init malloc ERROR! \n");
        return -ENOMEM;
    }

    for(i=0;i<fh->total-1;i++)
    {
        fh->free[i].index = i;
        fh->free[i].ps_id = 0;
        fh->free[i].next = &fh->free[i+1];
    }
    fh->free[fh->total-1].index = fh->total-1;
    fh->free[fh->total-1].ps_id = 0;
    fh->free[fh->total-1].next = NULL;

    for(i=0;i<IPF_MODEM_MAX;i++)
    {
        fh->launched[i] = fh->free;
        fh->launched[i]->next = NULL;
        fh->free = fh->free->next;
    }

    return 0;    
}


__init int ipf_filter_init(void)
{
    struct ipf_filter_handler* fh = g_ipf_ctx.filter_handler;
    struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();

    return ipf_filterlist_init();
}


static void ipf_filter_freelist_enqueue(filter_map* map)
{
    struct ipf_filter_handler* fh = g_ipf_ctx.filter_handler;
    filter_map* node = fh->free;
    filter_map* pre_node = 0;

    if(!fh->free){
        fh->free = map;
        map->next = 0;
    }

    while(node->index < map->index)
    {
        pre_node = node;
        node = node->next;
        if(!node) break;
    }
    
    if(!pre_node){
        map->next = fh->free;
        fh->free = map;
    }
    else{
        map->next = node;
        pre_node->next = map;
    }
}


void ipf_filter_tail_add(filter_map* head, filter_map* map)
{
    filter_map* node = head;

    while(node->next)
    {
        node = node->next;
    }

    node->next = map;
    map->next = NULL;
}


filter_map* ipf_filter_freelist_dequeue(void)
{
    struct ipf_filter_handler* fh = g_ipf_ctx.filter_handler;
    filter_map* node = fh->free;

    fh->free = fh->free->next;
    node->next = NULL;
    
    return node;
}

void ipf_filter_list_clear(IPF_FILTER_CHAIN_TYPE_E type)
{
    ipf_filter_node_s filter;
    filter_map* next;
    struct ipf_filter_handler* fh = g_ipf_ctx.filter_handler;
    filter_map* node = fh->launched[type]->next;
    
    ipf_read_filter(type, &filter);
    filter.unFltChain.Bits.fltn_next_index = fh->tail;
    filter.unFltRuleCtrl.u32FltRuleCtrl = IPF_DISABLE_FILTER;
    ipf_write_filter(type, &filter);
    
    fh->launched[type]->next = NULL;
    while(node)
    {
        next = node->next;
        ipf_filter_freelist_enqueue(node);
        node = next;
    }

    udelay(FILTER_RIP);
}
*/
