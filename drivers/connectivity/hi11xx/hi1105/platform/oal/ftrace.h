
#ifndef __FTRACE_HISI_H__
#define __FTRACE_HISI_H__

#ifdef FTRACE_ENABLE

#include <linux/types.h>
#include <linux/sort.h>
#include <linux/ktime.h>

#undef FTRACE_DEBUG

typedef struct _ftrace_event_ {
    struct hlist_node hlist;
    unsigned long total_count;
    unsigned int refcount;
    ktime_t time_stamp;      // enter time
    ktime_t total_cost;      // function total time cost
    unsigned long func_addr; // function address
} ftrace_event;

struct ftrace_hash {
    unsigned long size_bits;
    struct hlist_head *buckets;
    unsigned long count;
};

void notrace ftrace_hisi_init(void);
void notrace ftrace_hisi_exit(void);
#endif
#endif
