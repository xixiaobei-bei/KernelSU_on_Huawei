#ifndef KASLR_H
#define KASLR_H

#ifndef CONFIG_HUAWEI_KERNEL_STACK_NX
#define set_init_stack_nx(x)
#define set_task_stack_nx(x)
#else
extern void __init set_init_stack_nx(struct thread_info *ti);
extern void set_task_stack_nx(struct thread_info *ti);
#endif

#ifndef CONFIG_HUAWEI_KERNEL_STACK_NX
#define set_init_stack_nx(x)
#else
extern void __init set_init_stack_nx(struct thread_info *ti);
#endif

#ifndef CONFIG_HUAWEI_KERNEL_MODULE_RANDOMIZE
#define get_module_load_offset()
#else
extern unsigned long get_module_load_offset(void);
#endif

#endif
