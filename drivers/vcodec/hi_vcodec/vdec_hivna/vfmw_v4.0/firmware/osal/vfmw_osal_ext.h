

#ifndef __VFMW_OSAL_EXT_HEADER__
#define __VFMW_OSAL_EXT_HEADER__

#include "vfmw.h"
#include "mem_manage.h"

#define OSAL_OK     0
#define OSAL_ERR   (-1)

typedef SINT32(*OSAL_IRQ_HANDLER_T)(SINT32, VOID *);

typedef enum spin_lock_type {
	G_SPINLOCK_SCD = 0,
	G_SPINLOCK_VDH,
	G_SPINLOCK_RECORD,
} spin_lock_type;

typedef enum mutex_type {
	G_SCDHWDONEEVENT = 0,
	G_VDMHWDONEEVENT,
} mutex_type;

typedef enum sem_type {
	G_SCD_SEM = 0,
	G_VDH_SEM,
	G_BPD_SEM,
} sem_type;

typedef UINT32(*FN_OSAL_GET_TIME_IN_MS)(VOID);
typedef UINT32(*FN_OSAL_GET_TIME_IN_US)(VOID);
typedef VOID(*FN_OSAL_SPIN_LOCK_INIT)(spin_lock_type);
typedef SINT32(*FN_OSAL_SPIN_LOCK)(spin_lock_type);
typedef SINT32(*FN_OSAL_SPIN_UNLOCK)(spin_lock_type);
typedef VOID(*FN_OSAL_SEMA_INIT)(sem_type);
typedef SINT32(*FN_OSAL_SEMA_DOWN)(sem_type);
typedef VOID(*FN_OSAL_SEMA_UP)(sem_type);
typedef SINT32(*FN_OSAL_PRINT)(const char *, ...);
typedef VOID(*FN_OSAL_MB)(VOID);
typedef VOID(*FN_OSAL_U_Delay)(ULONG);
typedef VOID(*FN_OSAL_M_SLEEP)(UINT32);
typedef SINT32(*FN_OSAL_INIT_EVENT)(mutex_type, SINT32);
typedef SINT32(*FN_OSAL_GIVE_EVENT)(mutex_type);
typedef SINT32(*FN_OSAL_WAIT_EVENT)(mutex_type, SINT32);
typedef SINT32(*FN_OSAL_MEM_ALLOC)(UINT8 *, UINT32, UINT32, UINT32, mem_desc_s *);
typedef SINT32(*FN_OSAL_MEM_FREE)(mem_desc_s *);
typedef UINT8  * (*FN_OSAL_REGISTER_MAP)(UADDR, UINT32);
typedef VOID(*FN_OSAL_REGISTER_UNMAP)(UINT8 *, UINT32);
typedef UINT8  * (*FN_OSAL_MMAP)(UADDR, UINT32);
typedef UINT8  * (*FN_OSAL_MMAP_CACHE)(UADDR, UINT32);
typedef VOID(*FN_OSAL_MUNMAP)(UINT8 *);
typedef SINT32(*FN_OSAL_REQUEST_IRQ)(UINT32, OSAL_IRQ_HANDLER_T, ULONG, const char *, VOID *);
typedef VOID(*FN_OSAL_FREE_IRQ)(UINT32, VOID *);
typedef VOID   * (*FN_OSAL_ALLOC_VIR_MEM)(SINT32);
typedef VOID (*FN_OSAL_FREE_VIR_MEM)(VOID *);
typedef SINT32(*FN_OSAL_PROC_INIT)(VOID);
typedef VOID (*FN_OSAL_PROC_EXIT)(VOID);

typedef struct vfmw_osal_func_ptr {
	FN_OSAL_GET_TIME_IN_MS    pfun_osal_get_time_in_ms;
	FN_OSAL_GET_TIME_IN_US    pfun_osal_get_time_in_us;
	FN_OSAL_SPIN_LOCK_INIT      pfun_osal_spin_lock_init;
	FN_OSAL_SPIN_LOCK          pfun_osal_spin_lock;
	FN_OSAL_SPIN_UNLOCK        pfun_osal_spin_unlock;
	FN_OSAL_SEMA_INIT          pfun_osal_sema_init;
	FN_OSAL_SEMA_DOWN          pfun_osal_sema_down;
	FN_OSAL_SEMA_UP            pfun_osal_sema_up;
	FN_OSAL_PRINT             pfun_osal_print;
	FN_OSAL_MB                pfun_osal_mb;
	FN_OSAL_U_Delay            pfun_osal_u_delay;
	FN_OSAL_M_SLEEP            pfun_osal_m_sleep;
	FN_OSAL_INIT_EVENT        pfun_osal_init_event;
	FN_OSAL_GIVE_EVENT        pfun_osal_give_event;
	FN_OSAL_WAIT_EVENT         pfun_osal_wait_event;
	FN_OSAL_REQUEST_IRQ        pfun_osal_request_irq;
	FN_OSAL_FREE_IRQ           pfun_osal_free_irq;
	FN_OSAL_MEM_ALLOC          pfun_osal_mem_alloc;
	FN_OSAL_MEM_FREE           pfun_osal_mem_free;
	FN_OSAL_REGISTER_MAP       pfun_osal_register_map;
	FN_OSAL_REGISTER_UNMAP     pfun_osal_register_unmap;
	FN_OSAL_MMAP              pfun_osal_mmap;
	FN_OSAL_MMAP_CACHE         pfun_osal_mmap_cache;
	FN_OSAL_MUNMAP            pfun_osal_mun_map;
	FN_OSAL_ALLOC_VIR_MEM       pfun_osal_alloc_vir_mem;
	FN_OSAL_FREE_VIR_MEM        pfun_osal_free_vir_mem;
	FN_OSAL_PROC_INIT          pfun_osal_proc_init;
	FN_OSAL_PROC_EXIT          pfun_osal_proc_exit;
} vfmw_osal_func_ptr;

extern vfmw_osal_func_ptr g_vfmw_osal_fun_ptr;

#define VFMW_OSAL_GET_TIME_IN_MS       g_vfmw_osal_fun_ptr.pfun_osal_get_time_in_ms
#define VFMW_OSAL_GET_TIME_IN_US       g_vfmw_osal_fun_ptr.pfun_osal_get_time_in_us
#define VFMW_OSAL_SPIN_LOCK_INIT       g_vfmw_osal_fun_ptr.pfun_osal_spin_lock_init
#define VFMW_OSAL_SPIN_LOCK            g_vfmw_osal_fun_ptr.pfun_osal_spin_lock
#define VFMW_OSAL_SPIN_UNLOCK          g_vfmw_osal_fun_ptr.pfun_osal_spin_unlock
#define VFMW_OSAL_SEMA_INIT            g_vfmw_osal_fun_ptr.pfun_osal_sema_init
#define VFMW_OSAL_SEMA_DOWN            g_vfmw_osal_fun_ptr.pfun_osal_sema_down
#define VFMW_OSAL_SEMA_UP              g_vfmw_osal_fun_ptr.pfun_osal_sema_up
#define VFMW_OSAL_PRINT                g_vfmw_osal_fun_ptr.pfun_osal_print
#define VFMW_OSAL_MB                   g_vfmw_osal_fun_ptr.pfun_osal_mb
#define VFMW_OSAL_U_DELAY              g_vfmw_osal_fun_ptr.pfun_osal_u_delay
#define VFMW_OSAL_M_SLEEP              g_vfmw_osal_fun_ptr.pfun_osal_m_sleep
#define VFMW_OSAL_INIT_EVENT           g_vfmw_osal_fun_ptr.pfun_osal_init_event
#define VFMW_OSAL_GIVE_EVENT           g_vfmw_osal_fun_ptr.pfun_osal_give_event
#define VFMW_OSAL_WAIT_EVENT           g_vfmw_osal_fun_ptr.pfun_osal_wait_event
#define VFMW_OSAL_REQUEST_IRQ          g_vfmw_osal_fun_ptr.pfun_osal_request_irq
#define VFMW_OSAL_FREE_IRQ             g_vfmw_osal_fun_ptr.pfun_osal_free_irq
#define VFMW_OSAL_MEM_ALLOC            g_vfmw_osal_fun_ptr.pfun_osal_mem_alloc
#define VFMW_OSAL_MEM_FREE             g_vfmw_osal_fun_ptr.pfun_osal_mem_free
#define VFMW_OSAL_REGISTER_MAP         g_vfmw_osal_fun_ptr.pfun_osal_register_map
#define VFMW_OSAL_REGISTER_UNMAP       g_vfmw_osal_fun_ptr.pfun_osal_register_unmap
#define VFMW_OSAL_MMAP                 g_vfmw_osal_fun_ptr.pfun_osal_mmap
#define VFMW_OSAL_MMAP_CACHE           g_vfmw_osal_fun_ptr.pfun_osal_mmap_cache
#define VFMW_OSAL_MUN_MAP              g_vfmw_osal_fun_ptr.pfun_osal_mun_map
#define VFMW_OSAL_ALLOC_VIR_MEM        g_vfmw_osal_fun_ptr.pfun_osal_alloc_vir_mem
#define VFMW_OSAL_PREE_VIR_MEM         g_vfmw_osal_fun_ptr.pfun_osal_free_vir_mem
#define VFMW_OSAL_PROC_INIT            g_vfmw_osal_fun_ptr.pfun_osal_proc_init
#define VFMW_OSAL_PROC_EXIT            g_vfmw_osal_fun_ptr.pfun_osal_proc_exit

#endif
