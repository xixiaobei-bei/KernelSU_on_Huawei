
#ifdef FTRACE_ENABLE
#include "ftrace.h"
#include "securec.h"
#include <linux/kallsyms.h>
#include <linux/hash.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/uaccess.h>

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define FTRACE_SEQ_BUFF_SIZE (100 * 10000) // 100B perline, 10000 funcs

typedef enum _ftrace_sort_type_ {
    FTRACE_SORT_TOTAL_COUNT,
    FTRACE_SORT_TOTAL_COST,
    FTRACE_SORT_TYPE_BUTT
} ftrace_sort_type;

typedef void (*ftrace_enter_func)(void *func, void *caller, int cpu_id);
typedef void (*ftrace_exit_func)(void *func, void *caller, int cpu_id);
typedef void (*fttrace_list_func)(ftrace_event *, void *);
typedef int (*ftrace_sort_cmp)(const void *, const void *);

static ftrace_enter_func g_ftrace_enter = NULL;
static ftrace_exit_func g_ftrace_exit = NULL;

static struct ftrace_hash *g_hash_table[CONFIG_NR_CPUS];
int g_cpu_flag[CONFIG_NR_CPUS];

static struct proc_dir_entry *g_parent_entry = NULL;
static struct proc_dir_entry *g_total_entry = NULL;
static struct proc_dir_entry *g_per_entry = NULL;
static struct proc_dir_entry *g_ftrace_enable_entry = NULL;
static struct proc_dir_entry *g_sort_entry = NULL;

static int ftrace_entry_dup_total_count_compare(const void *a, const void *b);
static int ftrace_entry_dup_total_cost_compare(const void *a, const void *b);
static ftrace_sort_cmp g_ftrace_sort_cmp[FTRACE_SORT_TYPE_BUTT] = {
    ftrace_entry_dup_total_count_compare,
    ftrace_entry_dup_total_cost_compare
};

static ftrace_sort_type g_ftrace_sort_index = FTRACE_SORT_TOTAL_COUNT;

static notrace __always_inline bool ftrace_hash_empty(struct ftrace_hash *hash)
{
    return !hash || !hash->count;
}

static notrace __always_inline unsigned long ftrace_hash_key(struct ftrace_hash *hash, unsigned long func_addr)
{
    return hash_long(func_addr, hash->size_bits);
}

static notrace __always_inline ftrace_event *__ftrace_lookup_ip(struct ftrace_hash *hash, unsigned long func_addr)
{
    unsigned long key;
    ftrace_event *entry = NULL;
    struct hlist_head *hhd;

    key = ftrace_hash_key(hash, func_addr);
    hhd = &hash->buckets[key];

    hlist_for_each_entry(entry, hhd, hlist) {
        if (entry->func_addr == func_addr)
            return entry;
    }
    return NULL;
}

static void notrace ftrace_foreach_list(struct ftrace_hash *hash, fttrace_list_func call, void *data)
{
    struct hlist_head *hhd = NULL;
    struct hlist_node *tn = NULL;
    ftrace_event *entry = NULL;
    int size = 1 << hash->size_bits;
    int i;

    if (!hash->count) {
        return;
    }

    for (i = 0; i < size; i++) {
        hhd = &hash->buckets[i];
        hlist_for_each_entry_safe(entry, tn, hhd, hlist) {
            if (call != NULL) {
                call(entry, data);
            }
        }
    }
}

static int notrace ftrace_duplicate_list(struct ftrace_hash *hash, ftrace_event *dst_entry)
{
    int j = 0;
    struct hlist_head *hhd = NULL;
    struct hlist_node *tn = NULL;
    ftrace_event *entry = NULL;
    int size = 1 << hash->size_bits;
    int i;

    if (!hash->count) {
        return 0;
    }

    for (i = 0; i < size; i++) {
        hhd = &hash->buckets[i];
        hlist_for_each_entry_safe(entry, tn, hhd, hlist) {
            if (entry->total_count == 0) {
                continue;
            }
            (void)memcpy_s((void *)(dst_entry + j), sizeof(ftrace_event), (void *)entry, sizeof(ftrace_event));
            j++;
        }
    }

    return j;
}

static notrace __always_inline ftrace_event *ftrace_lookup_ip(struct ftrace_hash *hash, unsigned long func_addr)
{
    if (ftrace_hash_empty(hash)) {
        return NULL;
    }

    return __ftrace_lookup_ip(hash, func_addr);
}

static notrace __always_inline void __add_hash_entry(struct ftrace_hash *hash, ftrace_event *entry)
{
    struct hlist_head *hhd;
    unsigned long key;

    key = ftrace_hash_key(hash, entry->func_addr);
    hhd = &hash->buckets[key];
    hlist_add_head(&entry->hlist, hhd);
    hash->count++;
}

static notrace __always_inline ftrace_event *add_hash_entry(struct ftrace_hash *hash, unsigned long func_addr)
{
    ftrace_event *entry = kmalloc(sizeof(*entry), GFP_ATOMIC);
    if (entry == NULL) {
        return NULL;
    }

    entry->func_addr = func_addr;
    entry->total_count = 0;
    entry->total_cost = ktime_set(0, 0);
    entry->refcount = 0;
    __add_hash_entry(hash, entry);
    return entry;
}

static notrace __always_inline ftrace_event *ftrace_get_entry(struct ftrace_hash *hash, unsigned long func_addr)
{
    ftrace_event *entry = ftrace_lookup_ip(hash, func_addr);
    if (entry != NULL) {
        return entry;
    }
    return add_hash_entry(hash, func_addr);
}

static notrace struct ftrace_hash *ftrace_alloc_hash(int size_bits)
{
    struct ftrace_hash *hash = NULL;
    int size;

    hash = kzalloc(sizeof(*hash), GFP_KERNEL);
    if (hash == NULL) {
        return NULL;
    }

    size = 1 << size_bits;
    hash->buckets = kcalloc(size, sizeof(*hash->buckets), GFP_KERNEL);

    if (hash->buckets == NULL) {
        kfree(hash);
        return NULL;
    }

    hash->size_bits = size_bits;

    return hash;
}

static void notrace hash_free_entry(struct ftrace_hash *hash, ftrace_event *entry)
{
    hlist_del(&entry->hlist);
    kfree(entry);
    hash->count--;
}

static void notrace ftrace_clear_hash(struct ftrace_hash *hash)
{
    struct hlist_head *hhd = NULL;
    struct hlist_node *tn = NULL;
    ftrace_event *entry = NULL;
    int size = 1 << hash->size_bits;
    int i;

    if (!hash->count) {
        return;
    }

    for (i = 0; i < size; i++) {
        hhd = &hash->buckets[i];
        hlist_for_each_entry_safe(entry, tn, hhd, hlist) {
            hash_free_entry(hash, entry);
        }
    }
}


void notrace ftrace_free_hash(struct ftrace_hash *hash)
{
    if (hash == NULL) {
        return;
    }
    ftrace_clear_hash(hash);
    kfree(hash->buckets);
    kfree(hash);
}

static void notrace __cyg_profile_func_enter__(void *func, void *caller, int cpu_id)
{
    unsigned long flags;
    ftrace_event *entry = NULL;
    if (unlikely((cpu_id >= CONFIG_NR_CPUS) || (g_hash_table[cpu_id] == NULL))) {
        return;
    }
    local_irq_save(flags);
    entry = ftrace_get_entry(g_hash_table[cpu_id], (unsigned long)(uintptr_t)func);
    if (unlikely(entry == NULL)) {
        local_irq_restore(flags);
        return;
    }
    entry->total_count++;
    entry->func_addr = (unsigned long)(uintptr_t)func;
    entry->refcount++;
    entry->time_stamp = ktime_get();
    local_irq_restore(flags);
}

void notrace __cyg_profile_func_enter(void *func, void *caller)
{
    int cpu_id = get_cpu();
    if (g_ftrace_enter != NULL) {
        g_ftrace_enter(func, caller, cpu_id);
    }
    put_cpu();
}
EXPORT_SYMBOL(__cyg_profile_func_enter);

static void notrace __cyg_profile_func_exit__(void *func, void *caller, int cpu_id)
{
    ktime_t end_time;
    unsigned long flags;
    ftrace_event *entry = NULL;

    if (unlikely((cpu_id >= CONFIG_NR_CPUS) || (g_hash_table[cpu_id] == NULL))) {
        return;
    }

    local_irq_save(flags);
    end_time = ktime_get();
    entry = ftrace_get_entry(g_hash_table[cpu_id], (unsigned long)(uintptr_t)func);
    if (unlikely(entry == NULL)) {
        local_irq_restore(flags);
        return;
    }

    if (unlikely(entry->total_count == 0)) {
        entry->total_count++; // 发生了线程迁移
    }

    if (entry->refcount <= 0) {
        local_irq_restore(flags);
        return;
    }

    entry->refcount--;

    if (entry->refcount == 0) {
        entry->total_cost = ktime_add(entry->total_cost, ktime_sub(end_time, entry->time_stamp));
    }

    local_irq_restore(flags);
}

void notrace __cyg_profile_func_exit(void *func, void *caller)
{
    int cpu_id = get_cpu();
    if (g_ftrace_exit != NULL) {
        g_ftrace_exit(func, caller, cpu_id);
    }
    put_cpu();
}
EXPORT_SYMBOL(__cyg_profile_func_exit);

static void notrace ftrace_enable_func(void)
{
    g_ftrace_enter = __cyg_profile_func_enter__;
    g_ftrace_exit = __cyg_profile_func_exit__;
}

static void notrace ftrace_disable_func(void)
{
    g_ftrace_exit = NULL;
    g_ftrace_enter = NULL;
}

static void notrace ftrace_wait_cpu_idle(void)
{
    msleep(100); // 100ms for ftrace enter or exit func
}

void notrace ftrace_clean_list(ftrace_event *entry, void *data)
{
    entry->total_count = 0;
    entry->total_cost = ktime_set(0, 0);
    entry->time_stamp = ktime_set(0, 0);
    entry->refcount = 0;
}

static void notrace ftrace_clean(void)
{
    int i;
    for (i = 0; i < CONFIG_NR_CPUS; i++) {
        if ((g_hash_table[i] != NULL) && (g_hash_table[i]->count)) {
            ftrace_foreach_list(g_hash_table[i], ftrace_clean_list, NULL);
        }
    }
}

int notrace ftrace_start(void)
{
    if (g_ftrace_enter != NULL) {
        return -EBUSY;
    }
    ftrace_clean();
    ftrace_enable_func();
    return 0;
}

void notrace ftrace_stop(void)
{
    if (g_ftrace_enter == NULL) {
        return;
    }
    ftrace_disable_func();
    ftrace_wait_cpu_idle();
}

static int notrace ftrace_add_temp_event_update_event(ftrace_event *src_entry, ftrace_event *entry, int total_event)
{
    int j;
    ftrace_event *entry_t = NULL;
    for (j = 0; j < total_event; j++) {
        entry_t = src_entry + j;
        if (entry->func_addr == entry_t->func_addr) {
            // found
            entry_t->total_count += entry->total_count;
            entry_t->total_cost = ktime_add(entry_t->total_cost, entry->total_cost);
            break;
        }
    }
    if (j == total_event) {
        // not found
        entry_t = src_entry + total_event;
        entry_t->total_count = entry->total_count;
        entry_t->total_cost = entry->total_cost;
        entry_t->func_addr = entry->func_addr;
        return 0;
    } else {
        return 1;
    }
}

static int notrace ftrace_add_temp_event_to_list(struct ftrace_hash *hash, ftrace_event *src_entry, int total_cnt,
    int free_cnt)
{
    int i;
    int count = 0;
    int start = (total_cnt - free_cnt);
    ftrace_event *entry = NULL;
    struct hlist_head *hhd = NULL;
    struct hlist_node *tn = NULL;
    int size = 1 << hash->size_bits;
    if (!hash->count) {
        return 0;
    }

    if (free_cnt < 0) {
        return 0;
    }

    for (i = 0; i < size; i++) {
        hhd = &hash->buckets[i];
        hlist_for_each_entry_safe(entry, tn, hhd, hlist) {
            if (entry->total_count == 0) {
                continue;
            }
            if (ftrace_add_temp_event_update_event(src_entry, entry, start + count)) {
                continue; // found & update event
            }
            count++; // not found, add new event
        }
    }

    return count;
}

static notrace int ftrace_entry_dup_total_count_compare(const void *a, const void *b)
{
    return ((ftrace_event *)b)->total_count - ((ftrace_event *)a)->total_count;
}

static notrace int ftrace_entry_dup_total_cost_compare(const void *a, const void *b)
{
    s64 bns, ans;
    bns = ktime_to_ns(((ftrace_event *)b)->total_cost);
    ans = ktime_to_ns(((ftrace_event *)a)->total_cost);
    if (bns > ans) {
        return 1; // 1: a after b
    } else if (bns < ans) {
        return -1; // -1: b after a
    } else {
        return 0;
    }
}

void notrace ftrace_seq_cpu_list(ftrace_event *entry, void *data)
{
    struct seq_file *m = (struct seq_file*)data;
    if (entry->total_count == 0) { // 线程迁移
        return;
    }
    seq_printf(m, "\n%20lu  %20lld  %ps\n", entry->total_count, ktime_to_ns(entry->total_cost),
               (void *)(uintptr_t)entry->func_addr);
}

static notrace void ftrace_seq_sort(ftrace_event *entry, int count)
{
    if (g_ftrace_sort_cmp[g_ftrace_sort_index] != NULL) {
        sort((void *)entry, count, sizeof(ftrace_event),
                g_ftrace_sort_cmp[g_ftrace_sort_index], NULL);
    }
}

static notrace void ftrace_seq_show_print(struct seq_file *m, ftrace_event *entry, int count, int i)
{
    int j;
    unsigned long per_cost, total_count;
    ftrace_event *entry_t = NULL;
    for (j = 0; j < count; j++) {
        entry_t = entry + j;
        if (j == 0) {
            seq_printf(m, "\n[%4d] %20s  %20s %20s  %s\n",
                       i, "total_count", "total_cost(ns)", "percost(ns)", "function_name");
        }
        total_count = entry_t->total_count;
        per_cost = ktime_to_ns(entry_t->total_cost);
        if (likely(total_count != 0)) {
            do_div(per_cost, total_count);
        } else {
            per_cost = 0;
        }
        seq_printf(m, "[%4d] %20lu  %20lld %20ld  %ps\n",
                   i, entry_t->total_count, ktime_to_ns(entry_t->total_cost),
                   per_cost,
                   (void *)(uintptr_t)entry_t->func_addr);
    }
}

static notrace int ftrace_seq_show(struct seq_file *m, void *p)
{
    int i;
    int count;
    ftrace_event *entry = NULL;
    for (i = 0; i < CONFIG_NR_CPUS; i++) {
        if ((g_hash_table[i] != NULL) && (g_hash_table[i]->count)) {
            entry = (ftrace_event *)kmalloc(sizeof(ftrace_event) * g_hash_table[i]->count, GFP_KERNEL);
            if (entry == NULL) {
                seq_printf(m, "no mem alloc hash count:%lu\n", g_hash_table[i]->count);
                return 0;
            }
            count = ftrace_duplicate_list(g_hash_table[i], entry);
            if (count == 0) {
                continue;
            }
            ftrace_seq_sort(entry, count);
            ftrace_seq_show_print(m, entry, count, i);
            kfree(entry);
        }
    }
    return 0;
}

static notrace int ftrace_seq_open(struct inode *inode, struct file *file)
{
    unsigned int size = FTRACE_SEQ_BUFF_SIZE;
    return single_open_size(file, ftrace_seq_show, NULL, size);
}

static struct file_operations g_ftrace_seq_fops = {
    .open = ftrace_seq_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

static notrace int ftrace_seq_total_show(struct seq_file *m, void *p)
{
    int i = 0;
    int index = 0;
    int total_event_cnt = 0;
    unsigned long per_cost, total_count;
    ftrace_event *entry = NULL;
    ftrace_event *entry_t = NULL;

    // count the total func count
    for (i = 0; i < CONFIG_NR_CPUS; i++) {
        if (g_hash_table[i] != NULL) {
            total_event_cnt += g_hash_table[i]->count;
        }
    }

    entry = (ftrace_event *)kmalloc(sizeof(ftrace_event) * total_event_cnt, GFP_KERNEL);
    if (entry == NULL) {
        printk("kmalloc failed, cnt = %d\n", total_event_cnt);
        return -ENOMEM;
    }

    for (i = 0; i < CONFIG_NR_CPUS; i++) {
        if (g_hash_table[i] == NULL) {
            continue;
        }
        // add cpu i to event array
        index += ftrace_add_temp_event_to_list(g_hash_table[i], entry, total_event_cnt, total_event_cnt - index);
    }
    ftrace_seq_sort(entry, index);
    if (index != 0) {
        seq_printf(m, "%20s %20s %20s function_name\n",
                   "total_count", "total_cost(ns)", "percost(ns)");
    }

    for (i = 0; i < index; i++) {
        entry_t = entry + i;
        total_count = entry_t->total_count;
        per_cost = ktime_to_ns(entry_t->total_cost);
        if (likely(total_count != 0)) {
            do_div(per_cost, total_count);
        } else {
            per_cost = 0;
        }
        seq_printf(m, "%20lu  %20lld %20ld  %ps\n",
                   entry_t->total_count, ktime_to_ns(entry_t->total_cost),
                   per_cost,
                   (void *)(uintptr_t)entry_t->func_addr);
    }

    kfree(entry);
    return 0;
}

static notrace int ftrace_seq_total_open(struct inode *inode, struct file *file)
{
    unsigned int size = FTRACE_SEQ_BUFF_SIZE;
    return single_open_size(file, ftrace_seq_total_show, NULL, size);
}

static struct file_operations g_ftrace_seq_total_fops = {
    .open = ftrace_seq_total_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

static notrace ssize_t ftrace_enable_proc_read(struct file *filp, char __user *buffer, size_t len, loff_t *off)
{
    char *str = NULL;

    if (*off > 0) {
        return 0;
    }

    if (g_ftrace_enter == NULL) {
        str = "echo start/stop > ftrace, current: stop\n";
    } else {
        str = "echo start/stop > ftrace, current: start\n";
    }
    if (copy_to_user(buffer, str, strlen(str))) {
        printk(KERN_ERR "copy_to_user failed!\n");
        return -EFAULT;
    }
    *off += strlen(str);
    return strlen(str);
}

#define FTRACE_CMD_MAX_BUFSIZE 50
static notrace ssize_t ftrace_enable_proc_write(struct file *filp, const char __user *buffer, size_t len, loff_t *off)
{
    int size;
    char command[FTRACE_CMD_MAX_BUFSIZE] = { 0 };
    if (len < 1) { // at least 1 byte cmd
        printk("invalid len = %zu\n", len);
        return -EINVAL;
    }

    size = min(sizeof(command) - 1, len);

    if (copy_from_user(command, buffer, size)) {
        printk(KERN_ERR "copy ftrace enable buff failed, size=%d\n", size);
        return -EFAULT;
    }

    if (command[size - 1] == '\n') {
        size--;
    }

    printk("ftrace cmd: %s len=%zu size=%d\n", command, len, size);

    if (!strncmp(command, "start", size)) {
        printk("ftrace_start\n");
        ftrace_start();
    } else if (!strncmp(command, "stop", size)) {
        printk("ftrace_stop\n");
        ftrace_stop();
    } else if (!strncmp(command, "clean", size)) {
        printk("ftrace_clean\n");
        ftrace_clean();
    }
    return len;
}

static struct file_operations g_ftrace_enable_fops = {
    .owner = THIS_MODULE,
    .read = ftrace_enable_proc_read,
    .write = ftrace_enable_proc_write,
};

static notrace ssize_t ftrace_sort_proc_read(struct file *filp, char __user *buffer, size_t len, loff_t *off)
{
    char *str = NULL;

    if (*off > 0) {
        return 0;
    }

    if (g_ftrace_sort_index == FTRACE_SORT_TOTAL_COUNT) {
        str = "total_count\n";
    } else if (g_ftrace_sort_index == FTRACE_SORT_TOTAL_COST) {
        str = "total_cost\n";
    } else {
        str = "unkown";
    }
    if (copy_to_user(buffer, str, strlen(str))) {
        printk(KERN_ERR "copy_to_user failed!\n");
        return -EFAULT;
    }
    *off += strlen(str);
    return strlen(str);
}

static notrace ssize_t ftrace_sort_proc_write(struct file *filp, const char __user *buffer, size_t len, loff_t *off)
{
    int size;
    char command[FTRACE_CMD_MAX_BUFSIZE] = { 0 };
    if (len < 1) { // at least 1 byte cmd
        printk("invalid len = %zu\n", len);
        return -EINVAL;
    }

    size = min(sizeof(command) - 1, len);

    if (copy_from_user(command, buffer, size)) {
        printk(KERN_ERR "copy ftrace sort buff failed, size=%d\n", size);
        return -EFAULT;
    }

    if (command[size - 1] == '\n') {
        size--;
    }

    printk("ftrace cmd: %s len=%zu size=%d\n", command, len, size);

    if (!strncmp(command, "total_count", size)) {
        g_ftrace_sort_index = FTRACE_SORT_TOTAL_COUNT;
    } else if (!strncmp(command, "total_cost", size)) {
        g_ftrace_sort_index = FTRACE_SORT_TOTAL_COST;
    }
    return len;
}

static struct file_operations g_ftrace_sort_fops = {
    .read = ftrace_sort_proc_read,
    .write = ftrace_sort_proc_write,
};

static notrace void ftrace_seq_init(void)
{
    g_parent_entry = proc_mkdir("ftrace_host", NULL);
    if (g_parent_entry == NULL) {
        return;
    }
    g_per_entry = proc_create("cpu_show", (S_IRUGO), g_parent_entry, &g_ftrace_seq_fops);
    if (g_per_entry == NULL) {
        printk(KERN_EMERG "g_per_entry create failed\n");
        goto fail_per_entry;
    }
    g_total_entry = proc_create("total_show", (S_IRUGO), g_parent_entry, &g_ftrace_seq_total_fops);
    if (g_total_entry == NULL) {
        printk(KERN_EMERG "g_total_entry create failed\n");
        goto fail_total_entry;
    }
    g_ftrace_enable_entry = proc_create("ftrace", (S_IRUGO | S_IWUSR | S_IWGRP), g_parent_entry, &g_ftrace_enable_fops);
    if (g_ftrace_enable_entry == NULL) {
        printk(KERN_EMERG "g_ftrace_enable_entry create failed\n");
        goto fail_ftrace_enable_entry;
    }
    g_sort_entry = proc_create("sort", (S_IRUGO | S_IWUSR | S_IWGRP), g_parent_entry, &g_ftrace_sort_fops);
    if (g_sort_entry == NULL) {
        printk(KERN_EMERG "g_sort_entry create failed\n");
        goto fail_sort_entry;
    }
    printk("ftrace_seq_init.\n");
    return;
fail_sort_entry:
    proc_remove(g_ftrace_enable_entry);
    g_ftrace_enable_entry = NULL;
fail_ftrace_enable_entry:
    proc_remove(g_total_entry);
    g_total_entry = NULL;
fail_total_entry:
    proc_remove(g_per_entry);
    g_per_entry = NULL;
fail_per_entry:
    proc_remove(g_parent_entry);
    g_parent_entry = NULL;
}

static notrace void ftrace_seq_exit(void)
{
    if (g_sort_entry == NULL) {
        return;
    }
    proc_remove(g_sort_entry);
    g_sort_entry = NULL;
    proc_remove(g_ftrace_enable_entry);
    g_ftrace_enable_entry = NULL;
    proc_remove(g_total_entry);
    g_total_entry = NULL;
    proc_remove(g_per_entry);
    g_per_entry = NULL;
    proc_remove(g_parent_entry);
    g_parent_entry = NULL;
}

void notrace ftrace_res_init(void)
{
    int i;
    for (i = 0; i < CONFIG_NR_CPUS; i++) {
        g_hash_table[i] = ftrace_alloc_hash(12); // default 12 bits, 4KB
        if (g_hash_table[i] == NULL) {
            printk("cpu %d alloc hash table failed\n", i);
        }
        g_cpu_flag[i] = 0;
    }
    ftrace_seq_init();
}

void notrace ftrace_res_exit(void)
{
    int i;
    ftrace_seq_exit();
    for (i = 0; i < CONFIG_NR_CPUS; i++) {
        if (g_hash_table[i] != NULL) {
            ftrace_free_hash(g_hash_table[i]);
            g_hash_table[i] = NULL;
        }
    }
}

void notrace ftrace_hisi_init(void)
{
    ftrace_res_init();
}

void notrace ftrace_hisi_exit(void)
{
    ftrace_stop();
    ftrace_res_exit();
}
#endif
