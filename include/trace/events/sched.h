/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM sched

#if !defined(_TRACE_SCHED_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_SCHED_H

#include <linux/sched/numa_balancing.h>
#include <linux/tracepoint.h>
#include <linux/binfmts.h>
#include <securec.h>

#ifdef CONFIG_HW_QOS_THREAD
#include <chipset_common/hwqos/hwqos_common.h>
#endif

/*
 * Tracepoint for calling kthread_stop, performed to end a kthread:
 */
TRACE_EVENT(sched_kthread_stop,

	TP_PROTO(struct task_struct *t),

	TP_ARGS(t),

	TP_STRUCT__entry(
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, t->comm, TASK_COMM_LEN);
		__entry->pid	= t->pid;
	),

	TP_printk("comm=%s pid=%d", __entry->comm, __entry->pid)
);

/*
 * Tracepoint for the return value of the kthread stopping:
 */
TRACE_EVENT(sched_kthread_stop_ret,

	TP_PROTO(int ret),

	TP_ARGS(ret),

	TP_STRUCT__entry(
		__field(	int,	ret	)
	),

	TP_fast_assign(
		__entry->ret	= ret;
	),

	TP_printk("ret=%d", __entry->ret)
);

/*
 * Tracepoint for waking up a task:
 */
DECLARE_EVENT_CLASS(sched_wakeup_template,

	TP_PROTO(struct task_struct *p),

	TP_ARGS(__perf_task(p)),

	TP_STRUCT__entry(
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	int,	prio			)
		__field(	int,	success			)
		__field(	int,	target_cpu		)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
		__entry->prio		= p->prio; /* XXX SCHED_DEADLINE */
		__entry->success	= 1; /* rudiment, kill when possible */
		__entry->target_cpu	= task_cpu(p);
	),

	TP_printk("comm=%s pid=%d prio=%d target_cpu=%03d",
		  __entry->comm, __entry->pid, __entry->prio,
		  __entry->target_cpu)
);

/*
 * Tracepoint called when waking a task; this tracepoint is guaranteed to be
 * called from the waking context.
 */
DEFINE_EVENT(sched_wakeup_template, sched_waking,
	     TP_PROTO(struct task_struct *p),
	     TP_ARGS(p));

#ifdef CONFIG_HW_QOS_THREAD
/*
 * Tracepoint for sched qos
 */
DECLARE_EVENT_CLASS(sched_qos_template,

	TP_PROTO(struct task_struct *p, struct transact_qos *tq, int type),

	TP_ARGS(__perf_task(p), tq, type),

	TP_STRUCT__entry(
		__array(char,   comm,   TASK_COMM_LEN)
		__field(pid_t,  pid)
		__field(int,    prio)
		__field(int,    success)
		__field(int,    target_cpu)
		__field(int,    dynamic_qos)
		__field(int,    trans_qos)
		__field(pid_t,  trans_from)
		__field(int,    trans_type)
		__field(int,    trans_flags)
		__field(int,    type)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid         = p->pid;
		__entry->prio        = p->prio;
		__entry->success     = 1; /* rudiment, kill when possible */
		__entry->target_cpu  = task_cpu(p);
		__entry->dynamic_qos = get_task_set_qos(p);
		__entry->trans_qos   = get_task_trans_qos(tq);
		__entry->trans_from  = (tq == NULL) ? 0 : tq->trans_pid;
		__entry->trans_type  = (tq == NULL) ? 0 : tq->trans_type;
		__entry->trans_flags = atomic_read(&p->trans_flags);
		__entry->type        = type;
	),
	TP_printk("comm=%s pid=%d prio=%d target_cpu=%03d dynamic_qos=%d trans_qos=%d trans_from=%d trans_type=%d trans_flags=%d type=%d",
		__entry->comm, __entry->pid, __entry->prio,
		__entry->target_cpu, __entry->dynamic_qos,
		__entry->trans_qos, __entry->trans_from,
		__entry->trans_type, __entry->trans_flags,
		__entry->type)
);

DEFINE_EVENT(sched_qos_template, sched_qos,
	     TP_PROTO(struct task_struct *p, struct transact_qos *tq, int type),
	     TP_ARGS(p, tq, type));

#endif
/*
 * Tracepoint called when the task is actually woken; p->state == TASK_RUNNNG.
 * It it not always called from the waking context.
 */
DEFINE_EVENT(sched_wakeup_template, sched_wakeup,
	     TP_PROTO(struct task_struct *p),
	     TP_ARGS(p));

/*
 * Tracepoint for waking up a new task:
 */
DEFINE_EVENT(sched_wakeup_template, sched_wakeup_new,
	     TP_PROTO(struct task_struct *p),
	     TP_ARGS(p));

#ifdef CONFIG_HW_VIP_THREAD
/*
 * Tracepoint for sched vip
 */
DECLARE_EVENT_CLASS(sched_vip_template,

	TP_PROTO(struct task_struct *p, char *msg),

	TP_ARGS(__perf_task(p), msg),

	TP_STRUCT__entry(
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	int,	prio			)
		__array(	char,	msg, 	VIP_MSG_LEN	)
		__field(	int,	target_cpu		)
		__field(    u64,    dynamic_vip)
		__field(    int,    vip_depth)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
		__entry->prio		= p->prio;
		memcpy(__entry->msg, msg, min((size_t)VIP_MSG_LEN, strlen(msg)+1));
		__entry->target_cpu	= task_cpu(p);
		__entry->dynamic_vip   = atomic64_read(&p->dynamic_vip);
		__entry->vip_depth     = p->vip_depth;
	),

	TP_printk("comm=%s pid=%d prio=%d msg=%s target_cpu=%03d dynamic_vip:%llx vip_depth:%d",
		  __entry->comm, __entry->pid, __entry->prio,
		  __entry->msg, __entry->target_cpu, __entry->dynamic_vip, __entry->vip_depth)
);

DEFINE_EVENT(sched_vip_template, sched_vip_queue_op,
         TP_PROTO(struct task_struct *p, char *msg),
	     TP_ARGS(p, msg));

DEFINE_EVENT(sched_vip_template, sched_vip_sched,
         TP_PROTO(struct task_struct *p, char *msg),
	     TP_ARGS(p, msg));

#endif

#ifdef CREATE_TRACE_POINTS
static inline long __trace_sched_switch_state(bool preempt, struct task_struct *p)
{
	unsigned int state;

#ifdef CONFIG_SCHED_DEBUG
	BUG_ON(p != current);
#endif /* CONFIG_SCHED_DEBUG */

	/*
	 * Preemption ignores task state, therefore preempted tasks are always
	 * RUNNING (we will not have dequeued if state != RUNNING).
	 */
	if (preempt)
		return TASK_REPORT_MAX;

	/*
	 * __get_task_state() uses fls() and returns a value from 0-8 range.
	 * Decrement it by 1 (except TASK_RUNNING state i.e 0) before using
	 * it for left shift operation to get the correct task->state
	 * mapping.
	 */
	state = __get_task_state(p);

	return state ? (1 << (state - 1)) : state;
}
#endif /* CREATE_TRACE_POINTS */

/*
 * Tracepoint for task switches, performed by the scheduler:
 */
TRACE_EVENT(sched_switch,

	TP_PROTO(bool preempt,
		 struct task_struct *prev,
		 struct task_struct *next),

	TP_ARGS(preempt, prev, next),

	TP_STRUCT__entry(
		__array(	char,	prev_comm,	TASK_COMM_LEN	)
		__field(	pid_t,	prev_pid			)
		__field(	int,	prev_prio			)
		__field(	long,	prev_state			)
		__array(	char,	next_comm,	TASK_COMM_LEN	)
		__field(	pid_t,	next_pid			)
		__field(	int,	next_prio			)
	),

	TP_fast_assign(
		memcpy(__entry->next_comm, next->comm, TASK_COMM_LEN);
		__entry->prev_pid	= prev->pid;
		__entry->prev_prio	= prev->prio;
		__entry->prev_state	= __trace_sched_switch_state(preempt, prev);
		memcpy(__entry->prev_comm, prev->comm, TASK_COMM_LEN);
		__entry->next_pid	= next->pid;
		__entry->next_prio	= next->prio;
		/* XXX SCHED_DEADLINE */
	),

	TP_printk("prev_comm=%s prev_pid=%d prev_prio=%d prev_state=%s%s ==> next_comm=%s next_pid=%d next_prio=%d",
		__entry->prev_comm, __entry->prev_pid, __entry->prev_prio,

		(__entry->prev_state & (TASK_REPORT_MAX - 1)) ?
		  __print_flags(__entry->prev_state & (TASK_REPORT_MAX - 1), "|",
				{ 0x01, "S" }, { 0x02, "D" }, { 0x04, "T" },
				{ 0x08, "t" }, { 0x10, "X" }, { 0x20, "Z" },
				{ 0x40, "P" }, { 0x80, "I" }) :
		  "R",

		__entry->prev_state & TASK_REPORT_MAX ? "+" : "",
		__entry->next_comm, __entry->next_pid, __entry->next_prio)
);

/*
 * Tracepoint for a task being migrated:
 */
TRACE_EVENT(sched_migrate_task,

	TP_PROTO(struct task_struct *p, int dest_cpu),

	TP_ARGS(p, dest_cpu),

	TP_STRUCT__entry(
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	int,	prio			)
		__field(	int,	orig_cpu		)
		__field(	int,	dest_cpu		)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
		__entry->prio		= p->prio; /* XXX SCHED_DEADLINE */
		__entry->orig_cpu	= task_cpu(p);
		__entry->dest_cpu	= dest_cpu;
	),

	TP_printk("comm=%s pid=%d prio=%d orig_cpu=%d dest_cpu=%d",
		  __entry->comm, __entry->pid, __entry->prio,
		  __entry->orig_cpu, __entry->dest_cpu)
);

#ifdef CONFIG_HISI_EAS_SCHED
/*
 * Tracepoint for eas_store
 */
TRACE_EVENT(eas_attr_store,

	TP_PROTO(const char *name, int value),

	TP_ARGS(name, value),

	TP_STRUCT__entry(
		__array( char,	name,	TASK_COMM_LEN	)
		__field( int,		value		)
	),

	TP_fast_assign(
		memcpy(__entry->name, name, TASK_COMM_LEN);
		__entry->value		= value;
	),

	TP_printk("name=%s value=%d", __entry->name, __entry->value)
);

/*
 * Tracepoint for boost_write
 */
TRACE_EVENT(sched_tune_boost,

	TP_PROTO(const char *name, int boost),

	TP_ARGS(name, boost),

	TP_STRUCT__entry(
		__array( char,	name,	TASK_COMM_LEN	)
		__field( int,		boost		)
	),

	TP_fast_assign(
		memcpy(__entry->name, name, TASK_COMM_LEN);
		__entry->boost		= boost;
	),

	TP_printk("name=%s boost=%d", __entry->name, __entry->boost)
);
#endif /* CONFIG_HISI_EAS_SCHED */

DECLARE_EVENT_CLASS(sched_process_template,

	TP_PROTO(struct task_struct *p),

	TP_ARGS(p),

	TP_STRUCT__entry(
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	int,	prio			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
		__entry->prio		= p->prio; /* XXX SCHED_DEADLINE */
	),

	TP_printk("comm=%s pid=%d prio=%d",
		  __entry->comm, __entry->pid, __entry->prio)
);

/*
 * Tracepoint for freeing a task:
 */
DEFINE_EVENT(sched_process_template, sched_process_free,
	     TP_PROTO(struct task_struct *p),
	     TP_ARGS(p));


/*
 * Tracepoint for a task exiting:
 */
DEFINE_EVENT(sched_process_template, sched_process_exit,
	     TP_PROTO(struct task_struct *p),
	     TP_ARGS(p));

/*
 * Tracepoint for waiting on task to unschedule:
 */
DEFINE_EVENT(sched_process_template, sched_wait_task,
	TP_PROTO(struct task_struct *p),
	TP_ARGS(p));

/*
 * Tracepoint for a waiting task:
 */
TRACE_EVENT(sched_process_wait,

	TP_PROTO(struct pid *pid),

	TP_ARGS(pid),

	TP_STRUCT__entry(
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	int,	prio			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, current->comm, TASK_COMM_LEN);
		__entry->pid		= pid_nr(pid);
		__entry->prio		= current->prio; /* XXX SCHED_DEADLINE */
	),

	TP_printk("comm=%s pid=%d prio=%d",
		  __entry->comm, __entry->pid, __entry->prio)
);

/*
 * Tracepoint for do_fork:
 */
TRACE_EVENT(sched_process_fork,

	TP_PROTO(struct task_struct *parent, struct task_struct *child),

	TP_ARGS(parent, child),

	TP_STRUCT__entry(
		__array(	char,	parent_comm,	TASK_COMM_LEN	)
		__field(	pid_t,	parent_pid			)
		__array(	char,	child_comm,	TASK_COMM_LEN	)
		__field(	pid_t,	child_pid			)
	),

	TP_fast_assign(
		memcpy(__entry->parent_comm, parent->comm, TASK_COMM_LEN);
		__entry->parent_pid	= parent->pid;
		memcpy(__entry->child_comm, child->comm, TASK_COMM_LEN);
		__entry->child_pid	= child->pid;
	),

	TP_printk("comm=%s pid=%d child_comm=%s child_pid=%d",
		__entry->parent_comm, __entry->parent_pid,
		__entry->child_comm, __entry->child_pid)
);

/*
 * Tracepoint for exec:
 */
TRACE_EVENT(sched_process_exec,

	TP_PROTO(struct task_struct *p, pid_t old_pid,
		 struct linux_binprm *bprm),

	TP_ARGS(p, old_pid, bprm),

	TP_STRUCT__entry(
		__string(	filename,	bprm->filename	)
		__field(	pid_t,		pid		)
		__field(	pid_t,		old_pid		)
	),

	TP_fast_assign(
		__assign_str(filename, bprm->filename);
		__entry->pid		= p->pid;
		__entry->old_pid	= old_pid;
	),

	TP_printk("filename=%s pid=%d old_pid=%d", __get_str(filename),
		  __entry->pid, __entry->old_pid)
);

/*
 * XXX the below sched_stat tracepoints only apply to SCHED_OTHER/BATCH/IDLE
 *     adding sched_stat support to SCHED_FIFO/RR would be welcome.
 */
DECLARE_EVENT_CLASS(sched_stat_template,

	TP_PROTO(struct task_struct *tsk, u64 delay),

	TP_ARGS(__perf_task(tsk), __perf_count(delay)),

	TP_STRUCT__entry(
		__array( char,	comm,	TASK_COMM_LEN	)
		__field( pid_t,	pid			)
		__field( u64,	delay			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, tsk->comm, TASK_COMM_LEN);
		__entry->pid	= tsk->pid;
		__entry->delay	= delay;
	),

	TP_printk("comm=%s pid=%d delay=%Lu [ns]",
			__entry->comm, __entry->pid,
			(unsigned long long)__entry->delay)
);


/*
 * Tracepoint for accounting wait time (time the task is runnable
 * but not actually running due to scheduler contention).
 */
DEFINE_EVENT(sched_stat_template, sched_stat_wait,
	     TP_PROTO(struct task_struct *tsk, u64 delay),
	     TP_ARGS(tsk, delay));

/*
 * Tracepoint for accounting sleep time (time the task is not runnable,
 * including iowait, see below).
 */
DEFINE_EVENT(sched_stat_template, sched_stat_sleep,
	     TP_PROTO(struct task_struct *tsk, u64 delay),
	     TP_ARGS(tsk, delay));

/*
 * Tracepoint for accounting iowait time (time the task is not runnable
 * due to waiting on IO to complete).
 */
DEFINE_EVENT(sched_stat_template, sched_stat_iowait,
	     TP_PROTO(struct task_struct *tsk, u64 delay),
	     TP_ARGS(tsk, delay));

/*
 * Tracepoint for accounting blocked time (time the task is in uninterruptible).
 */
DEFINE_EVENT(sched_stat_template, sched_stat_blocked,
	     TP_PROTO(struct task_struct *tsk, u64 delay),
	     TP_ARGS(tsk, delay));

/*
 * Tracepoint for recording the cause of uninterruptible sleep.
 */
TRACE_EVENT(sched_blocked_reason,

	TP_PROTO(struct task_struct *tsk, u64 delay),

	TP_ARGS(tsk,delay),

	TP_STRUCT__entry(
		__field( pid_t,	pid	)
		__field( void*, caller	)
		__field( bool, io_wait	)
		__field( u64, delay	)
	),

	TP_fast_assign(
		__entry->pid	= tsk->pid;
		__entry->caller = (void*)get_wchan(tsk);
		__entry->io_wait = tsk->in_iowait;
		__entry->delay = delay;
	),

	TP_printk("pid=%d iowait=%d caller=%pS delay=%lu",
	__entry->pid,__entry->io_wait, __entry->caller, __entry->delay>>10)
);

/*
 * Tracepoint for accounting runtime (time the task is executing
 * on a CPU).
 */
DECLARE_EVENT_CLASS(sched_stat_runtime,

	TP_PROTO(struct task_struct *tsk, u64 runtime, u64 vruntime),

	TP_ARGS(tsk, __perf_count(runtime), vruntime),

	TP_STRUCT__entry(
		__array( char,	comm,	TASK_COMM_LEN	)
		__field( pid_t,	pid			)
		__field( u64,	runtime			)
		__field( u64,	vruntime			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, tsk->comm, TASK_COMM_LEN);
		__entry->pid		= tsk->pid;
		__entry->runtime	= runtime;
		__entry->vruntime	= vruntime;
	),

	TP_printk("comm=%s pid=%d runtime=%Lu [ns] vruntime=%Lu [ns]",
			__entry->comm, __entry->pid,
			(unsigned long long)__entry->runtime,
			(unsigned long long)__entry->vruntime)
);

DEFINE_EVENT(sched_stat_runtime, sched_stat_runtime,
	     TP_PROTO(struct task_struct *tsk, u64 runtime, u64 vruntime),
	     TP_ARGS(tsk, runtime, vruntime));

/*
 * Tracepoint for showing priority inheritance modifying a tasks
 * priority.
 */
TRACE_EVENT(sched_pi_setprio,

	TP_PROTO(struct task_struct *tsk, struct task_struct *pi_task),

	TP_ARGS(tsk, pi_task),

	TP_STRUCT__entry(
		__array( char,	comm,	TASK_COMM_LEN	)
		__field( pid_t,	pid			)
		__field( int,	oldprio			)
		__field( int,	newprio			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, tsk->comm, TASK_COMM_LEN);
		__entry->pid		= tsk->pid;
		__entry->oldprio	= tsk->prio;
		__entry->newprio	= pi_task ?
				min(tsk->normal_prio, pi_task->prio) :
				tsk->normal_prio;
		/* XXX SCHED_DEADLINE bits missing */
	),

	TP_printk("comm=%s pid=%d oldprio=%d newprio=%d",
			__entry->comm, __entry->pid,
			__entry->oldprio, __entry->newprio)
);

#ifdef CONFIG_DETECT_HUNG_TASK
TRACE_EVENT(sched_process_hang,
	TP_PROTO(struct task_struct *tsk),
	TP_ARGS(tsk),

	TP_STRUCT__entry(
		__array( char,	comm,	TASK_COMM_LEN	)
		__field( pid_t,	pid			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, tsk->comm, TASK_COMM_LEN);
		__entry->pid = tsk->pid;
	),

	TP_printk("comm=%s pid=%d", __entry->comm, __entry->pid)
);
#endif /* CONFIG_DETECT_HUNG_TASK */

DECLARE_EVENT_CLASS(sched_move_task_template,

	TP_PROTO(struct task_struct *tsk, int src_cpu, int dst_cpu),

	TP_ARGS(tsk, src_cpu, dst_cpu),

	TP_STRUCT__entry(
		__field( pid_t,	pid			)
		__field( pid_t,	tgid			)
		__field( pid_t,	ngid			)
		__field( int,	src_cpu			)
		__field( int,	src_nid			)
		__field( int,	dst_cpu			)
		__field( int,	dst_nid			)
	),

	TP_fast_assign(
		__entry->pid		= task_pid_nr(tsk);
		__entry->tgid		= task_tgid_nr(tsk);
		__entry->ngid		= task_numa_group_id(tsk);
		__entry->src_cpu	= src_cpu;
		__entry->src_nid	= cpu_to_node(src_cpu);
		__entry->dst_cpu	= dst_cpu;
		__entry->dst_nid	= cpu_to_node(dst_cpu);
	),

	TP_printk("pid=%d tgid=%d ngid=%d src_cpu=%d src_nid=%d dst_cpu=%d dst_nid=%d",
			__entry->pid, __entry->tgid, __entry->ngid,
			__entry->src_cpu, __entry->src_nid,
			__entry->dst_cpu, __entry->dst_nid)
);

/*
 * Tracks migration of tasks from one runqueue to another. Can be used to
 * detect if automatic NUMA balancing is bouncing between nodes
 */
DEFINE_EVENT(sched_move_task_template, sched_move_numa,
	TP_PROTO(struct task_struct *tsk, int src_cpu, int dst_cpu),

	TP_ARGS(tsk, src_cpu, dst_cpu)
);

DEFINE_EVENT(sched_move_task_template, sched_stick_numa,
	TP_PROTO(struct task_struct *tsk, int src_cpu, int dst_cpu),

	TP_ARGS(tsk, src_cpu, dst_cpu)
);

TRACE_EVENT(sched_swap_numa,

	TP_PROTO(struct task_struct *src_tsk, int src_cpu,
		 struct task_struct *dst_tsk, int dst_cpu),

	TP_ARGS(src_tsk, src_cpu, dst_tsk, dst_cpu),

	TP_STRUCT__entry(
		__field( pid_t,	src_pid			)
		__field( pid_t,	src_tgid		)
		__field( pid_t,	src_ngid		)
		__field( int,	src_cpu			)
		__field( int,	src_nid			)
		__field( pid_t,	dst_pid			)
		__field( pid_t,	dst_tgid		)
		__field( pid_t,	dst_ngid		)
		__field( int,	dst_cpu			)
		__field( int,	dst_nid			)
	),

	TP_fast_assign(
		__entry->src_pid	= task_pid_nr(src_tsk);
		__entry->src_tgid	= task_tgid_nr(src_tsk);
		__entry->src_ngid	= task_numa_group_id(src_tsk);
		__entry->src_cpu	= src_cpu;
		__entry->src_nid	= cpu_to_node(src_cpu);
		__entry->dst_pid	= task_pid_nr(dst_tsk);
		__entry->dst_tgid	= task_tgid_nr(dst_tsk);
		__entry->dst_ngid	= task_numa_group_id(dst_tsk);
		__entry->dst_cpu	= dst_cpu;
		__entry->dst_nid	= cpu_to_node(dst_cpu);
	),

	TP_printk("src_pid=%d src_tgid=%d src_ngid=%d src_cpu=%d src_nid=%d dst_pid=%d dst_tgid=%d dst_ngid=%d dst_cpu=%d dst_nid=%d",
			__entry->src_pid, __entry->src_tgid, __entry->src_ngid,
			__entry->src_cpu, __entry->src_nid,
			__entry->dst_pid, __entry->dst_tgid, __entry->dst_ngid,
			__entry->dst_cpu, __entry->dst_nid)
);

/*
 * Tracepoint for waking a polling cpu without an IPI.
 */
TRACE_EVENT(sched_wake_idle_without_ipi,

	TP_PROTO(int cpu),

	TP_ARGS(cpu),

	TP_STRUCT__entry(
		__field(	int,	cpu	)
	),

	TP_fast_assign(
		__entry->cpu	= cpu;
	),

	TP_printk("cpu=%d", __entry->cpu)
);

#ifdef CONFIG_HISI_CORE_CTRL
TRACE_EVENT(core_ctl_get_nr_running_avg,/* [false alarm]:fortify */

	TP_PROTO(const struct cpumask *cpus, unsigned int avg,
		 unsigned int big_avg, unsigned int iowait_avg,
		 unsigned int nr_max),

	TP_ARGS(cpus, avg, big_avg, iowait_avg, nr_max),

	TP_STRUCT__entry(
		__bitmask( cpumask,	num_possible_cpus()	)
		__field( unsigned int,	avg			)
		__field( unsigned int,	big_avg			)
		__field( unsigned int,	iowait_avg		)
		__field( unsigned int,	nr_max			)
	),

	TP_fast_assign(
		__assign_bitmask(cpumask, cpumask_bits(cpus),
				 num_possible_cpus());
		__entry->avg		= avg;
		__entry->big_avg	= big_avg;
		__entry->iowait_avg	= iowait_avg;
		__entry->nr_max		= nr_max;
	),

	TP_printk("cpu=%s avg=%u big_avg=%u iowait_avg=%u max_nr=%u",
		  __get_bitmask(cpumask), __entry->avg, __entry->big_avg,
		  __entry->iowait_avg, __entry->nr_max)
);/* [false alarm]:fortify */

TRACE_EVENT(core_ctl_eval_need,/* [false alarm]:fortify */

	TP_PROTO(const struct cpumask *cpus, unsigned int nrrun,
		 unsigned int busy_cpus, unsigned int active_cpus,
		 unsigned int old_need, unsigned int new_need,
		 s64 elapsed, unsigned int updated),
	TP_ARGS(cpus, nrrun, busy_cpus, active_cpus,
		old_need, new_need, elapsed, updated),
	TP_STRUCT__entry(
		__bitmask( cpumask, num_possible_cpus()	)
		__field( u32, nrrun			)
		__field( u32, busy_cpus			)
		__field( u32, active_cpus		)
		__field( u32, old_need			)
		__field( u32, new_need			)
		__field( s64, elapsed			)
		__field( u32, updated			)
	),
	TP_fast_assign(
		__assign_bitmask(cpumask, cpumask_bits(cpus),
				 num_possible_cpus());
		__entry->nrrun		= nrrun;
		__entry->busy_cpus	= busy_cpus;
		__entry->active_cpus	= active_cpus;
		__entry->old_need	= old_need;
		__entry->new_need	= new_need;
		__entry->elapsed	= elapsed;
		__entry->updated	= updated;
	),
	TP_printk("cpu=%s, nrrun:%u, busy:%u, active:%u,"
		  " old_need=%u, new_need=%u, elapsed =%lld, updated=%u",
		  __get_bitmask(cpumask), __entry->nrrun, __entry->busy_cpus,
		  __entry->active_cpus, __entry->old_need, __entry->new_need,
		  __entry->elapsed, __entry->updated)
);/* [false alarm]:fortify */

TRACE_EVENT(core_ctl_update_busy,/* [false alarm]:fortify */

	TP_PROTO(unsigned int cpu, unsigned int load,
		 unsigned int old_is_busy, unsigned int is_busy),
	TP_ARGS(cpu, load, old_is_busy, is_busy),
	TP_STRUCT__entry(
		__field( u32, cpu		)
		__field( u32, load		)
		__field( u32, old_is_busy	)
		__field( u32, is_busy		)
	),
	TP_fast_assign(
		__entry->cpu		= cpu;
		__entry->load		= load;
		__entry->old_is_busy	= old_is_busy;
		__entry->is_busy	= is_busy;
	),
	TP_printk("cpu=%u, load=%u, old_is_busy=%u, new_is_busy=%u",
		  __entry->cpu, __entry->load, __entry->old_is_busy,
		  __entry->is_busy)
);/* [false alarm]:fortify */

TRACE_EVENT(core_ctl_set_boost,/* [false alarm]:fortify */

	TP_PROTO(u32 refcount),
	TP_ARGS(refcount),
	TP_STRUCT__entry(
		__field( u32, refcount	)
	),
	TP_fast_assign(
		__entry->refcount	= refcount;
	),
	TP_printk("boost=%u", __entry->refcount)
);/* [false alarm]:fortify */
#endif /* CONFIG_HISI_CORE_CTRL */

#ifdef CONFIG_SMP
#ifdef CREATE_TRACE_POINTS
static inline
int __trace_sched_cpu(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
#ifdef CONFIG_FAIR_GROUP_SCHED
	struct rq *rq = cfs_rq ? cfs_rq->rq : NULL;
#else
	struct rq *rq = cfs_rq ? container_of(cfs_rq, struct rq, cfs) : NULL;
#endif
	return rq ? cpu_of(rq)
		  : task_cpu((container_of(se, struct task_struct, se)));
}

static inline
int __trace_sched_path(struct cfs_rq *cfs_rq, char *path, int len)
{
#ifdef CONFIG_FAIR_GROUP_SCHED
	int l = path ? len : 0;

	if (cfs_rq && task_group_is_autogroup(cfs_rq->tg))
		return autogroup_path(cfs_rq->tg, path, l) + 1;
	else if (cfs_rq && cfs_rq->tg->css.cgroup)
		return cgroup_path(cfs_rq->tg->css.cgroup, path, l) + 1;
#endif
	if (path)
		strcpy(path, "(null)");

	return strlen("(null)");
}

static inline
struct cfs_rq *__trace_sched_group_cfs_rq(struct sched_entity *se)
{
#ifdef CONFIG_FAIR_GROUP_SCHED
	return se->my_q;
#else
	return NULL;
#endif
}
#endif /* CREATE_TRACE_POINTS */

#ifdef CONFIG_HISI_CPU_ISOLATION
/*
 * sched_isolate - called when cores are isolated/unisolated
 *
 * @time: amount of time in us it took to isolate/unisolate
 * @isolate: 1 if isolating, 0 if unisolating
 *
 */
TRACE_EVENT(sched_isolate,/* [false alarm]:fortify */

	TP_PROTO(unsigned int requested_cpu, unsigned int isolated_cpus,
		 u64 start_time, unsigned char isolate),

	TP_ARGS(requested_cpu, isolated_cpus, start_time, isolate),

	TP_STRUCT__entry(
		__field( u32, requested_cpu	)
		__field( u32, isolated_cpus	)
		__field( u32, time		)
		__field( unsigned char, isolate	)
	),

	TP_fast_assign(
		__entry->requested_cpu	= requested_cpu;
		__entry->isolated_cpus	= isolated_cpus;
		__entry->time		= div64_u64(sched_clock() - start_time, 1000);
		__entry->isolate	= isolate;
	),

	TP_printk("iso cpu=%u cpus=0x%x time=%u us isolated=%d",
		  __entry->requested_cpu, __entry->isolated_cpus,
		  __entry->time, __entry->isolate)
);/* [false alarm]:fortify */
#endif /* CONFIG_HISI_CPU_ISOLATION */

#ifdef CONFIG_SCHED_WALT
extern unsigned int sysctl_sched_use_walt_cpu_util;
extern unsigned int sysctl_sched_use_walt_task_util;
#ifdef CONFIG_SCHED_HISI_WALT_WINDOW_SIZE_TUNABLE
extern unsigned int walt_ravg_window;
extern bool walt_disabled;
#else
extern const unsigned int walt_ravg_window;
extern const bool walt_disabled;
#endif

#define walt_util(util_var, demand_sum) {\
	u64 sum = demand_sum << SCHED_CAPACITY_SHIFT;\
	do_div(sum, walt_ravg_window);\
	util_var = (typeof(util_var))sum;\
	}
#endif

/*
 * Tracepoint for cfs_rq load tracking:
 */
TRACE_EVENT(sched_load_cfs_rq,

	TP_PROTO(struct cfs_rq *cfs_rq),

	TP_ARGS(cfs_rq),

	TP_STRUCT__entry(
		__field(	int,		cpu			)
		__dynamic_array(char,		path,
				__trace_sched_path(cfs_rq, NULL, 0)	)
		__field(	unsigned long,	load			)
		__field(	unsigned long,	util			)
		__field(	unsigned long,	util_pelt          	)
		__field(	unsigned long,	util_walt          	)
	),

	TP_fast_assign(
		__entry->cpu	= __trace_sched_cpu(cfs_rq, NULL);
		__trace_sched_path(cfs_rq, __get_dynamic_array(path),
				   __get_dynamic_array_len(path));
		__entry->load	= cfs_rq->runnable_load_avg;
		__entry->util	= cfs_rq->avg.util_avg;
		__entry->util_pelt = cfs_rq->avg.util_avg;
		__entry->util_walt = 0;
#ifdef CONFIG_SCHED_WALT
		if (&cfs_rq->rq->cfs == cfs_rq) {
			walt_util(__entry->util_walt,
				  cfs_rq->rq->prev_runnable_sum);
			if (!walt_disabled && sysctl_sched_use_walt_cpu_util)
				__entry->util = __entry->util_walt;
		}
#endif
	),

	TP_printk("cpu=%d path=%s load=%lu util=%lu util_pelt=%lu util_walt=%lu",
		  __entry->cpu, __get_str(path), __entry->load, __entry->util,
		  __entry->util_pelt, __entry->util_walt)
);

/*
 * Tracepoint for rt_rq load tracking:
 */
struct rt_rq;

TRACE_EVENT(sched_load_rt_rq,

	TP_PROTO(int cpu, struct rt_rq *rt_rq),

	TP_ARGS(cpu, rt_rq),

	TP_STRUCT__entry(
		__field(	int,		cpu			)
		__field(	unsigned long,	util			)
	),

	TP_fast_assign(
		__entry->cpu	= cpu;
		__entry->util	= rt_rq->avg.util_avg;
	),

	TP_printk("cpu=%d util=%lu", __entry->cpu,
		  __entry->util)
);

/*
 * Tracepoint for sched_entity load tracking:
 */
TRACE_EVENT(sched_load_se,

	TP_PROTO(struct sched_entity *se),

	TP_ARGS(se),

	TP_STRUCT__entry(
		__field(	int,		cpu			      )
		__dynamic_array(char,		path,
		  __trace_sched_path(__trace_sched_group_cfs_rq(se), NULL, 0) )
		__array(	char,		comm,	TASK_COMM_LEN	      )
		__field(	pid_t,		pid			      )
		__field(	unsigned long,	load			      )
		__field(	unsigned long,	util			      )
		__field(	unsigned long,	util_pelt		      )
		__field(	unsigned long,	util_walt		      )
	),

	TP_fast_assign(
		struct cfs_rq *gcfs_rq = __trace_sched_group_cfs_rq(se);
		struct task_struct *p = gcfs_rq ? NULL
				    : container_of(se, struct task_struct, se);

		__entry->cpu = __trace_sched_cpu(gcfs_rq, se);
		__trace_sched_path(gcfs_rq, __get_dynamic_array(path),
				   __get_dynamic_array_len(path));
		memcpy(__entry->comm, p ? p->comm : "(null)", TASK_COMM_LEN);
		__entry->pid = p ? p->pid : -1;
		__entry->load = se->avg.load_avg;
		__entry->util = se->avg.util_avg;
		__entry->util_pelt  = __entry->util;
		__entry->util_walt  = 0;
#ifdef CONFIG_SCHED_WALT
		if (!se->my_q) {
			struct task_struct *p = container_of(se, struct task_struct, se);
			walt_util(__entry->util_walt, p->ravg.demand);
			if (!walt_disabled && sysctl_sched_use_walt_task_util)
				__entry->util = __entry->util_walt;
		}
#endif
	),

	TP_printk("cpu=%d path=%s comm=%s pid=%d load=%lu util=%lu util_pelt=%lu util_walt=%lu",
		  __entry->cpu, __get_str(path), __entry->comm,
		  __entry->pid, __entry->load, __entry->util,
		  __entry->util_pelt, __entry->util_walt)
);

/*
 * Tracepoint for task_group load tracking:
 */
#ifdef CONFIG_FAIR_GROUP_SCHED
TRACE_EVENT(sched_load_tg,

	TP_PROTO(struct cfs_rq *cfs_rq),

	TP_ARGS(cfs_rq),

	TP_STRUCT__entry(
		__field(	int,	cpu				)
		__dynamic_array(char,	path,
				__trace_sched_path(cfs_rq, NULL, 0)	)
		__field(	long,	load				)
	),

	TP_fast_assign(
		__entry->cpu	= cfs_rq->rq->cpu;
		__trace_sched_path(cfs_rq, __get_dynamic_array(path),
				   __get_dynamic_array_len(path));
		__entry->load	= atomic_long_read(&cfs_rq->tg->load_avg);
	),

	TP_printk("cpu=%d path=%s load=%ld", __entry->cpu, __get_str(path),
		  __entry->load)
);
#endif /* CONFIG_FAIR_GROUP_SCHED */

/*
 * Tracepoint for accounting CPU  boosted utilization
 */
TRACE_EVENT(sched_boost_cpu,

	TP_PROTO(int cpu, unsigned long util, long margin),

	TP_ARGS(cpu, util, margin),

	TP_STRUCT__entry(
		__field( int,		cpu			)
		__field( unsigned long,	util			)
		__field(long,		margin			)
	),

	TP_fast_assign(
		__entry->cpu	= cpu;
		__entry->util	= util;
		__entry->margin	= margin;
	),

	TP_printk("cpu=%d util=%lu margin=%ld",
		  __entry->cpu,
		  __entry->util,
		  __entry->margin)
);

/*
 * Tracepoint for schedtune_tasks_update
 */
TRACE_EVENT(sched_tune_tasks_update,

	TP_PROTO(struct task_struct *tsk, int cpu, int tasks, int idx,
		int boost, int max_boost, u64 group_ts),

	TP_ARGS(tsk, cpu, tasks, idx, boost, max_boost, group_ts),

	TP_STRUCT__entry(
		__array( char,	comm,	TASK_COMM_LEN	)
		__field( pid_t,		pid		)
		__field( int,		cpu		)
		__field( int,		tasks		)
		__field( int,		idx		)
		__field( int,		boost		)
		__field( int,		max_boost	)
		__field( u64,		group_ts	)
	),

	TP_fast_assign(
		memcpy(__entry->comm, tsk->comm, TASK_COMM_LEN);
		__entry->pid		= tsk->pid;
		__entry->cpu 		= cpu;
		__entry->tasks		= tasks;
		__entry->idx 		= idx;
		__entry->boost		= boost;
		__entry->max_boost	= max_boost;
		__entry->group_ts	= group_ts;
	),

	TP_printk("pid=%d comm=%s "
			"cpu=%d tasks=%d idx=%d boost=%d max_boost=%d timeout=%llu",
		__entry->pid, __entry->comm,
		__entry->cpu, __entry->tasks, __entry->idx,
		__entry->boost, __entry->max_boost,
		__entry->group_ts)
);

/*
 * Tracepoint for schedtune_boostgroup_update
 */
TRACE_EVENT(sched_tune_boostgroup_update,

	TP_PROTO(int cpu, int variation, int max_boost),

	TP_ARGS(cpu, variation, max_boost),

	TP_STRUCT__entry(
		__field( int,	cpu		)
		__field( int,	variation	)
		__field( int,	max_boost	)
	),

	TP_fast_assign(
		__entry->cpu		= cpu;
		__entry->variation	= variation;
		__entry->max_boost	= max_boost;
	),

	TP_printk("cpu=%d variation=%d max_boost=%d",
		__entry->cpu, __entry->variation, __entry->max_boost)
);

#ifdef CONFIG_HISI_CPU_FREQ_GOV_SCHEDUTIL
/*
 * Tracepoint for schedtune_freqboost
 */
TRACE_EVENT(sched_tune_freqboost,

	TP_PROTO(const char *name, int freq_boost),

	TP_ARGS(name, freq_boost),

	TP_STRUCT__entry(
		__array( char,	name,	TASK_COMM_LEN	)
		__field( int,		freq_boost	)
	),

	TP_fast_assign(
		memcpy(__entry->name, name, TASK_COMM_LEN);
		__entry->freq_boost	= freq_boost;
	),

	TP_printk("name=%s freq_boost=%d", __entry->name, __entry->freq_boost)
);

/*
 * Tracepoint for schedtune_freqboostgroup_update
 */
TRACE_EVENT(sched_tune_freqboostgroup_update,

	TP_PROTO(int cpu, int variation, int max_boost),

	TP_ARGS(cpu, variation, max_boost),

	TP_STRUCT__entry(
		__field( int,	cpu		)
		__field( int,	variation	)
		__field( int,	max_boost	)
	),

	TP_fast_assign(
		__entry->cpu		= cpu;
		__entry->variation	= variation;
		__entry->max_boost	= max_boost;
	),

	TP_printk("cpu=%d variation=%d max_freq_boost=%d",
		__entry->cpu, __entry->variation, __entry->max_boost)
);
#endif

/*
 * Tracepoint for accounting task boosted utilization
 */
TRACE_EVENT(sched_boost_task,

	TP_PROTO(struct task_struct *tsk, unsigned long util, long margin),

	TP_ARGS(tsk, util, margin),

	TP_STRUCT__entry(
		__array( char,	comm,	TASK_COMM_LEN		)
		__field( pid_t,		pid			)
		__field( unsigned long,	util			)
		__field( long,		margin			)

	),

	TP_fast_assign(
		memcpy(__entry->comm, tsk->comm, TASK_COMM_LEN);
		__entry->pid	= tsk->pid;
		__entry->util	= util;
		__entry->margin	= margin;
	),

	TP_printk("comm=%s pid=%d util=%lu margin=%ld",
		  __entry->comm, __entry->pid,
		  __entry->util,
		  __entry->margin)
);

/*
 * Tracepoint for system overutilized flag
 */
struct sched_domain;
TRACE_EVENT_CONDITION(sched_overutilized,

	TP_PROTO(struct sched_domain *sd, bool was_overutilized, bool overutilized),

	TP_ARGS(sd, was_overutilized, overutilized),

	TP_CONDITION(overutilized != was_overutilized),

	TP_STRUCT__entry(
		__field( bool,	overutilized	  )
		__array( char,  cpulist , 32      )
	),

	TP_fast_assign(
		__entry->overutilized	= overutilized;
		scnprintf(__entry->cpulist, sizeof(__entry->cpulist), "%*pbl", cpumask_pr_args(sched_domain_span(sd)));
	),

	TP_printk("overutilized=%d sd_span=%s",
		__entry->overutilized ? 1 : 0, __entry->cpulist)
);

/*
 * Tracepoint for find_best_target
 */
TRACE_EVENT(sched_find_best_target,

	TP_PROTO(struct task_struct *tsk, bool prefer_idle,
		unsigned long min_util, int start_cpu,
		int best_idle, int best_active, int target),

	TP_ARGS(tsk, prefer_idle, min_util, start_cpu,
		best_idle, best_active, target),

	TP_STRUCT__entry(
		__array( char,	comm,	TASK_COMM_LEN	)
		__field( pid_t,	pid			)
		__field( unsigned long,	min_util	)
		__field( bool,	prefer_idle		)
		__field( int,	start_cpu		)
		__field( int,	best_idle		)
		__field( int,	best_active		)
		__field( int,	target			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, tsk->comm, TASK_COMM_LEN);
		__entry->pid		= tsk->pid;
		__entry->min_util	= min_util;
		__entry->prefer_idle	= prefer_idle;
		__entry->start_cpu 	= start_cpu;
		__entry->best_idle	= best_idle;
		__entry->best_active	= best_active;
		__entry->target		= target;
	),

	TP_printk("pid=%d comm=%s prefer_idle=%d start_cpu=%d "
		  "best_idle=%d best_active=%d target=%d",
		__entry->pid, __entry->comm,
		__entry->prefer_idle, __entry->start_cpu,
		__entry->best_idle, __entry->best_active,
		__entry->target)
);

#ifdef CONFIG_SCHED_HISI_DEBUG_TRACE
TRACE_EVENT(sched_cpu_util,

	TP_PROTO(int cpu, long new_util, u64 irqload, int high_irqload,
		 unsigned int capacity_curr, unsigned int capacity,
		 unsigned int capacity_orig, unsigned int capacity_min),

	TP_ARGS(cpu, new_util, irqload, high_irqload, capacity_curr,
		capacity, capacity_orig, capacity_min),

	TP_STRUCT__entry(
		__field(unsigned int, cpu)
		__field(unsigned int, nr_running)
		__field(long, cpu_util)
		__field(long, new_util)
		__field(unsigned int, capacity_curr)
		__field(unsigned int, capacity)
		__field(unsigned int, capacity_orig)
		__field(unsigned int, capacity_min)
		__field(int, idle_state)
		__field(u64, irqload)
		__field(int, online)
		__field(int, isolated)
		__field(int, reserved)
		__field(int, high_irqload)
	),

	TP_fast_assign(
		__entry->cpu			= cpu;
		__entry->nr_running		= cpu_rq(cpu)->nr_running;
		__entry->cpu_util		= cpu_util(cpu);
		__entry->new_util		= new_util;
		__entry->capacity_curr		= capacity_curr;
		__entry->capacity		= capacity;
		__entry->capacity_orig		= capacity_orig;
		__entry->capacity_min		= capacity_min;
		__entry->idle_state		= idle_get_state_idx(cpu_rq(cpu));
		__entry->irqload		= irqload;
		__entry->online			= cpu_online(cpu);
		__entry->isolated		= cpu_isolated(cpu);
		__entry->reserved		= is_reserved(cpu);
		__entry->high_irqload		= high_irqload;
	),

	TP_printk("cpu=%d nr_running=%d cpu_util=%ld new_util=%ld"
		  " capacity_curr=%u capacity=%u"
		  " capacity_orig=%u capacity_min=%u"
		  " idle_state=%d irqload=%llu online=%d isolated=%d"
		  " reserved=%d high_irq_load=%d",
		  __entry->cpu, __entry->nr_running, __entry->cpu_util,
		  __entry->new_util, __entry->capacity_curr,
		  __entry->capacity, __entry->capacity_orig,
		  __entry->capacity_min, __entry->idle_state,
		  __entry->irqload, __entry->online, __entry->isolated,
		  __entry->reserved, __entry->high_irqload)
);

TRACE_EVENT(sched_energy_diff,

	TP_PROTO(struct task_struct *p, int prev_cpu, unsigned int prev_energy,
		 int next_cpu, unsigned int next_energy,
		 int backup_cpu, unsigned int backup_energy),

	TP_ARGS(p, prev_cpu, prev_energy, next_cpu, next_energy,
		backup_cpu, backup_energy),

	TP_STRUCT__entry(
		__field(int, pid)
		__field(int, prev_cpu)
		__field(int, prev_energy)
		__field(int, next_cpu)
		__field(int, next_energy)
		__field(int, backup_cpu)
		__field(int, backup_energy)
	),

	TP_fast_assign(
		__entry->pid			= p->pid;
		__entry->prev_cpu		= prev_cpu;
		__entry->prev_energy		= prev_energy;
		__entry->next_cpu		= next_cpu;
		__entry->next_energy		= next_energy;
		__entry->backup_cpu		= backup_cpu;
		__entry->backup_energy		= backup_energy;
	),

	TP_printk("pid=%d prev_cpu=%d prev_energy=%u next_cpu=%d"
		  " next_energy=%u backup_cpu=%d backup_energy=%u",
		  __entry->pid, __entry->prev_cpu, __entry->prev_energy,
		  __entry->next_cpu, __entry->next_energy,
		  __entry->backup_cpu, __entry->backup_energy)
);

/*
 * Tracepoint for find busiest group
 */
TRACE_EVENT(sched_find_busiest_group,

	TP_PROTO(int idle, int group_type, const struct cpumask *cpus,
		 unsigned int fbg_flag),

	TP_ARGS(idle, group_type, cpus, fbg_flag),

	TP_STRUCT__entry(
		__field(int,		idle)
		__field(int,		group_type)
		__bitmask(cpumask,	num_possible_cpus())
		__field(unsigned int,	fbg_flag)
	),

	TP_fast_assign(
		__entry->idle		= idle;
		__entry->group_type	= group_type;
		__assign_bitmask(cpumask, cpumask_bits(cpus),
				 num_possible_cpus());
		__entry->fbg_flag	= fbg_flag;
	),

	TP_printk("idle=%d group_type=%d cpus=%s flag=%#x",
		  __entry->idle, __entry->group_type,
		  __get_bitmask(cpumask),
		  __entry->fbg_flag)
);

/*
 * Tracepoint for find new ilb
 */
TRACE_EVENT(sched_hisi_find_new_ilb,

	TP_PROTO(int type, int ilb),

	TP_ARGS(type, ilb),

	TP_STRUCT__entry(
		__field(int, type)
		__field(int, ilb)
	),

	TP_fast_assign(
		__entry->type	= type;
		__entry->ilb	= ilb;
	),

	TP_printk("type=%d ilb=%d", __entry->type, __entry->ilb)
);

/*
 * Tracepoint for load balancing:
 */
#if NR_CPUS > 32
#error "Unsupported NR_CPUS for lb tracepoint."
#endif
TRACE_EVENT(sched_load_balance,

	TP_PROTO(int cpu, int idle, int balance,
		unsigned long group_mask, int busiest_nr_running,
		unsigned long imbalance, unsigned int env_flags, int ld_moved,
		unsigned int balance_interval, int active_balance),

	TP_ARGS(cpu, idle, balance, group_mask, busiest_nr_running,
		imbalance, env_flags, ld_moved, balance_interval,
		active_balance),

	TP_STRUCT__entry(
		__field(int,		cpu)
		__field(int,		idle)
		__field(int,		balance)
		__field(unsigned long,	group_mask)
		__field(int,		busiest_nr_running)
		__field(unsigned long,	imbalance)
		__field(unsigned int,	env_flags)
		__field(int,		ld_moved)
		__field(unsigned int,	balance_interval)
		__field(int,		active_balance)
	),

	TP_fast_assign(
		__entry->cpu                    = cpu;
		__entry->idle                   = idle;
		__entry->balance                = balance;
		__entry->group_mask             = group_mask;
		__entry->busiest_nr_running     = busiest_nr_running;
		__entry->imbalance              = imbalance;
		__entry->env_flags              = env_flags;
		__entry->ld_moved               = ld_moved;
		__entry->balance_interval       = balance_interval;
		__entry->active_balance		= active_balance;
	),

	TP_printk("cpu=%d state=%d balance=%d group=%#lx busy_nr=%d"
		  " imbalance=%ld flags=%#x ld_moved=%d bal_int=%d"
		  " active_balance=%d",
		  __entry->cpu, __entry->idle, __entry->balance,
		  __entry->group_mask, __entry->busiest_nr_running,
		  __entry->imbalance, __entry->env_flags, __entry->ld_moved,
		  __entry->balance_interval, __entry->active_balance)
);


TRACE_EVENT(sched_load_balance_nohz_kick,

	TP_PROTO(int cpu, int kick_cpu, int cpu_overutil),

	TP_ARGS(cpu, kick_cpu, cpu_overutil),

	TP_STRUCT__entry(
		__field(int,		cpu)
		__field(unsigned int,	cpu_nr)
		__field(unsigned long,	misfit_task_load)
		__field(int,		cpu_overutil)
		__field(int,		kick_cpu)
		__field(unsigned long,	nohz_flags)
	),

	TP_fast_assign(
		__entry->cpu			= cpu;
		__entry->cpu_nr			= cpu_rq(cpu)->nr_running;
		__entry->misfit_task_load	= cpu_rq(cpu)->misfit_task_load;
		__entry->cpu_overutil		= cpu_overutil;
		__entry->kick_cpu		= kick_cpu;
		__entry->nohz_flags		= *nohz_flags(kick_cpu);
	),

	TP_printk("cpu=%d nr_run=%u misfit_task_load=%lu overutilized=%d"
		  " kick_cpu=%d nohz_flags=0x%lx",
		  __entry->cpu, __entry->cpu_nr, __entry->misfit_task_load,
		  __entry->cpu_overutil, __entry->kick_cpu,
		  __entry->nohz_flags)

);

TRACE_EVENT(sched_load_balance_sg_stats,

	TP_PROTO(unsigned long sg_cpus, int group_type, unsigned int idle_cpus,
		 unsigned int sum_nr_running, unsigned long group_load,
		 unsigned long group_capacity, unsigned long group_util,
		 int group_no_capacity, unsigned long load_per_task,
		 unsigned long misfit_load, unsigned long busiest),

	TP_ARGS(sg_cpus, group_type, idle_cpus, sum_nr_running, group_load,
		group_capacity, group_util, group_no_capacity, load_per_task,
		misfit_load, busiest),

	TP_STRUCT__entry(
		__field(unsigned long,		group_mask)
		__field(int,			group_type)
		__field(unsigned int,		group_idle_cpus)
		__field(unsigned int,		sum_nr_running)
		__field(unsigned long,		group_load)
		__field(unsigned long,		group_capacity)
		__field(unsigned long,		group_util)
		__field(int,			group_no_capacity)
		__field(unsigned long,		load_per_task)
		__field(unsigned long,		misfit_task_load)
		__field(unsigned long,		busiest)
	),

	TP_fast_assign(
		__entry->group_mask			= sg_cpus;
		__entry->group_type			= group_type;
		__entry->group_idle_cpus		= idle_cpus;
		__entry->sum_nr_running			= sum_nr_running;
		__entry->group_load			= group_load;
		__entry->group_capacity			= group_capacity;
		__entry->group_util			= group_util;
		__entry->group_no_capacity		= group_no_capacity;
		__entry->load_per_task			= load_per_task;
		__entry->misfit_task_load		= misfit_load;
		__entry->busiest			= busiest;
	),

	TP_printk("sched_group=%#lx type=%d idle_cpus=%u sum_nr_run=%u"
		  " group_load=%lu capacity=%lu util=%lu no_capacity=%d"
		  " lpt=%lu misfit_tload=%lu busiest_group=%#lx",
		  __entry->group_mask, __entry->group_type,
		  __entry->group_idle_cpus, __entry->sum_nr_running,
		  __entry->group_load, __entry->group_capacity,
		  __entry->group_util, __entry->group_no_capacity,
		  __entry->load_per_task, __entry->misfit_task_load,
		  __entry->busiest)
);

TRACE_EVENT(sched_load_balance_stats,

	TP_PROTO(unsigned long busiest, int bgroup_type,
		 unsigned long bavg_load, unsigned long bload_per_task,
		 unsigned long local, int lgroup_type,
		 unsigned long lavg_load, unsigned long lload_per_task,
		 unsigned long sds_avg_load, unsigned long imbalance),

	TP_ARGS(busiest, bgroup_type, bavg_load, bload_per_task,
		local, lgroup_type, lavg_load, lload_per_task,
		sds_avg_load, imbalance),

	TP_STRUCT__entry(
		__field(unsigned long,		busiest)
		__field(int,			bgp_type)
		__field(unsigned long,		bavg_load)
		__field(unsigned long,		blpt)
		__field(unsigned long,		local)
		__field(int,			lgp_type)
		__field(unsigned long,		lavg_load)
		__field(unsigned long,		llpt)
		__field(unsigned long,		sds_avg)
		__field(unsigned long,		imbalance)
	),

	TP_fast_assign(
		__entry->busiest			= busiest;
		__entry->bgp_type			= bgroup_type;
		__entry->bavg_load			= bavg_load;
		__entry->blpt				= bload_per_task;
		__entry->bgp_type			= bgroup_type;
		__entry->local				= local;
		__entry->lgp_type			= lgroup_type;
		__entry->lavg_load			= lavg_load;
		__entry->llpt				= lload_per_task;
		__entry->sds_avg			= sds_avg_load;
		__entry->imbalance			= imbalance;
	),

	TP_printk("busiest_group=%#lx busiest_type=%d busiest_avg_load=%ld"
		  " busiest_lpt=%ld local_group=%#lx local_type=%d"
		  " local_avg_load=%ld local_lpt=%ld domain_avg_load=%ld"
		  " imbalance=%ld",
		  __entry->busiest, __entry->bgp_type,
		  __entry->bavg_load, __entry->blpt,
		  __entry->local, __entry->lgp_type,
		  __entry->lavg_load, __entry->llpt,
		  __entry->sds_avg, __entry->imbalance)
);

TRACE_EVENT(sched_load_balance_skip_tasks,

	TP_PROTO(int scpu, int dcpu, int grp_type, int pid,
		 unsigned long h_load, unsigned long task_util,
		 unsigned long affinity),

	TP_ARGS(scpu, dcpu, grp_type, pid, h_load, task_util, affinity),

	TP_STRUCT__entry(
		__field(int,		scpu)
		__field(unsigned long,	src_util_cum)
		__field(int,		grp_type)
		__field(int,		dcpu)
		__field(unsigned long,	dst_util_cum)
		__field(int,		pid)
		__field(unsigned long,	affinity)
		__field(unsigned long,	task_util)
		__field(unsigned long,	h_load)
	),

	TP_fast_assign(
		__entry->scpu			= scpu;
		__entry->src_util_cum		= cpu_util(scpu);
		__entry->grp_type		= grp_type;
		__entry->dcpu			= dcpu;
		__entry->dst_util_cum		= cpu_util(dcpu);
		__entry->pid			= pid;
		__entry->affinity		= affinity;
		__entry->task_util		= task_util;
		__entry->h_load			= h_load;
	),

	TP_printk("source_cpu=%d util=%lu group_type=%d dest_cpu=%d util=%lu"
		  " pid=%d affinity=%#lx task_util=%lu task_h_load=%lu",
		  __entry->scpu, __entry->src_util_cum, __entry->grp_type,
		  __entry->dcpu, __entry->dst_util_cum, __entry->pid,
		  __entry->affinity, __entry->task_util, __entry->h_load)
);
#endif

/*
 * Tracepoint for tasks' estimated utilization.
 */
TRACE_EVENT(sched_util_est_task,

	TP_PROTO(struct task_struct *tsk, struct sched_avg *avg),

	TP_ARGS(tsk, avg),

	TP_STRUCT__entry(
		__array( char,	comm,	TASK_COMM_LEN		)
		__field( pid_t,		pid			)
		__field( int,		cpu			)
		__field( unsigned int,	util_avg		)
		__field( unsigned int,	est_enqueued		)
		__field( unsigned int,	est_ewma		)
	),

	TP_fast_assign(
		memcpy(__entry->comm, tsk->comm, TASK_COMM_LEN);
		__entry->pid			= tsk->pid;
		__entry->cpu                    = task_cpu(tsk);
		__entry->util_avg               = avg->util_avg;
		__entry->est_enqueued           = avg->util_est.enqueued;
		__entry->est_ewma               = avg->util_est.ewma;
	),

	TP_printk("comm=%s pid=%d cpu=%d util_avg=%u util_est_ewma=%u util_est_enqueued=%u",
		  __entry->comm,
		  __entry->pid,
		  __entry->cpu,
		  __entry->util_avg,
		  __entry->est_ewma,
		  __entry->est_enqueued)
);

/*
 * Tracepoint for root cfs_rq's estimated utilization.
 */
TRACE_EVENT(sched_util_est_cpu,

	TP_PROTO(int cpu, struct cfs_rq *cfs_rq),

	TP_ARGS(cpu, cfs_rq),

	TP_STRUCT__entry(
		__field( int,		cpu			)
		__field( unsigned int,	util_avg		)
		__field( unsigned int,	util_est_enqueued	)
	),

	TP_fast_assign(
		__entry->cpu			= cpu;
		__entry->util_avg		= cfs_rq->avg.util_avg;
		__entry->util_est_enqueued	= cfs_rq->avg.util_est.enqueued;
	),

	TP_printk("cpu=%d util_avg=%u util_est_enqueued=%u",
		  __entry->cpu,
		  __entry->util_avg,
		  __entry->util_est_enqueued)
);

#ifdef CONFIG_HISI_EAS_SCHED
/*
 * Tracepoint for sched group energy
 */
TRACE_EVENT(sched_group_energy,

	TP_PROTO(int cpu, const struct cpumask *cpus,
		 int idle_idx, int cap_idx, unsigned long util,
		 int busy_energy, int idle_energy),

	TP_ARGS(cpu, cpus, idle_idx, cap_idx, util, busy_energy, idle_energy),

	TP_STRUCT__entry(
		__field( int,	cpu	)
		__bitmask(cpumask, num_possible_cpus())
		__field( int,	idle_idx	)
		__field( int,	cap_idx	)
		__field( unsigned long,	group_util	)
		__field( int,	busy_energy	)
		__field( int,	idle_energy	)
	),

	TP_fast_assign(
		__entry->cpu	= cpu;
		__assign_bitmask(cpumask, cpumask_bits(cpus),
				num_possible_cpus());
		__entry->idle_idx	= idle_idx;
		__entry->cap_idx	= cap_idx;
		__entry->group_util	= util;
		__entry->busy_energy	= busy_energy;
		__entry->idle_energy	= idle_energy;
	),

	TP_printk("cpu=%d sg_cpus=%s idle_idx=%d cap_idx=%d group_util=%lu sg_busy_energy=%d sg_idle_energy=%d",
		__entry->cpu, __get_bitmask(cpumask), __entry->idle_idx,
		__entry->cap_idx, __entry->group_util, __entry->busy_energy, __entry->idle_energy)
);

/*
 * Tracepoint for sched_setaffinity
 */
TRACE_EVENT(sched_set_affinity,

	TP_PROTO(struct task_struct *p, const struct cpumask *mask),

	TP_ARGS(p, mask),

	TP_STRUCT__entry(
		__array(   char,	comm,	TASK_COMM_LEN	)
		__field(   pid_t,	pid			)
		__bitmask( cpus,	num_possible_cpus()	)
	),

	TP_fast_assign(
		__entry->pid = p->pid;
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__assign_bitmask(cpus, cpumask_bits(mask), num_possible_cpus());
	),

	TP_printk("comm=%s pid=%d cpus=%s",
		__entry->comm, __entry->pid, __get_bitmask(cpus))
);
#endif

#ifdef CONFIG_SCHED_WALT
struct rq;

TRACE_EVENT(walt_update_task_ravg,

	TP_PROTO(struct task_struct *p, struct rq *rq, int evt,
						u64 wallclock, u64 irqtime),

	TP_ARGS(p, rq, evt, wallclock, irqtime),

	TP_STRUCT__entry(
		__array(	char,	comm,   TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	pid_t,	cur_pid			)
		__field(	u64,	wallclock		)
		__field(	u64,	mark_start		)
		__field(	u64,	delta_m			)
		__field(	u64,	win_start		)
		__field(	u64,	delta			)
		__field(	u64,	irqtime			)
		__array(    char,   evt, 16			)
		__field(unsigned int,	demand			)
		__field(unsigned int,	sum			)
		__field(	 int,	cpu			)
		__field(	u64,	cs			)
		__field(	u64,	ps			)
		__field(	u32,	curr_window		)
		__field(	u32,	prev_window		)
		__field(	u64,	nt_cs			)
		__field(	u64,	nt_ps			)
		__field(	u32,	active_windows		)
	),

	TP_fast_assign(
			static const char* walt_event_names[] =
			{
				"PUT_PREV_TASK",
				"PICK_NEXT_TASK",
				"TASK_WAKE",
				"TASK_MIGRATE",
				"TASK_UPDATE",
				"IRQ_UPDATE"
			};
		__entry->wallclock      = wallclock;
		__entry->win_start      = rq->window_start;
		__entry->delta          = (wallclock - rq->window_start);
		strcpy(__entry->evt, walt_event_names[evt]);
		__entry->cpu            = rq->cpu;
		__entry->cur_pid        = rq->curr->pid;
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid            = p->pid;
		__entry->mark_start     = p->ravg.mark_start;
		__entry->delta_m        = (wallclock - p->ravg.mark_start);
		__entry->demand         = p->ravg.demand;
		__entry->sum            = p->ravg.sum;
		__entry->irqtime        = irqtime;
		__entry->cs             = rq->curr_runnable_sum;
		__entry->ps             = rq->prev_runnable_sum;
		__entry->curr_window	= p->ravg.curr_window;
		__entry->prev_window	= p->ravg.prev_window;
		__entry->nt_cs		= rq->nt_curr_runnable_sum;
		__entry->nt_ps		= rq->nt_prev_runnable_sum;
		__entry->active_windows	= p->ravg.active_windows;
	),

	TP_printk("wallclock=%llu window_start=%llu delta=%llu event=%s cpu=%d cur_pid=%d pid=%d comm=%s"
		" mark_start=%llu delta=%llu demand=%u sum=%u irqtime=%llu"
		" curr_runnable_sum=%llu prev_runnable_sum=%llu cur_window=%u"
		" prev_window=%u nt_curr_runnable_sum=%llu nt_prev_runnable_sum=%llu active_windows=%u",
		__entry->wallclock, __entry->win_start, __entry->delta,
		__entry->evt, __entry->cpu, __entry->cur_pid,
		__entry->pid, __entry->comm, __entry->mark_start,
		__entry->delta_m, __entry->demand,
		__entry->sum, __entry->irqtime,
		__entry->cs, __entry->ps,
		__entry->curr_window, __entry->prev_window,
		__entry->nt_cs, __entry->nt_ps,
		__entry->active_windows
		)
);

TRACE_EVENT(walt_update_history,

	TP_PROTO(struct rq *rq, struct task_struct *p, u32 runtime, int samples,
			int evt),

	TP_ARGS(rq, p, runtime, samples, evt),

	TP_STRUCT__entry(
		__array(	char,	comm,   TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(unsigned int,	runtime			)
		__field(	 int,	samples			)
		__field(	 int,	evt			)
		__field(	 u64,	demand			)
		__field(unsigned int,	walt_avg		)
		__field(unsigned int,	pelt_avg		)
		__array(	 u32,	hist, RAVG_HIST_SIZE_MAX)
		__field(	 int,	cpu			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid            = p->pid;
		__entry->runtime        = runtime;
		__entry->samples        = samples;
		__entry->evt            = evt;
		__entry->demand         = p->ravg.demand;
		walt_util(__entry->walt_avg,__entry->demand);
		__entry->pelt_avg	= p->se.avg.util_avg;
		memcpy(__entry->hist, p->ravg.sum_history,
					RAVG_HIST_SIZE_MAX * sizeof(u32));
		__entry->cpu            = rq->cpu;
	),

	TP_printk("pid=%d comm=%s runtime=%u samples=%d event=%d demand=%llu ravg_window=%u"
		" walt=%u pelt=%u hist0=%u hist1=%u hist2=%u hist3=%u hist4=%u cpu=%d",
		__entry->pid, __entry->comm,
		__entry->runtime, __entry->samples, __entry->evt,
		__entry->demand,
		walt_ravg_window,
		__entry->walt_avg,
		__entry->pelt_avg,
		__entry->hist[0], __entry->hist[1],
		__entry->hist[2], __entry->hist[3],
		__entry->hist[4], __entry->cpu)
);

TRACE_EVENT(walt_migration_update_sum,

	TP_PROTO(struct rq *rq, struct task_struct *p),

	TP_ARGS(rq, p),

	TP_STRUCT__entry(
		__field(int,		cpu			)
		__field(int,		pid			)
		__field(	u64,	cs			)
		__field(	u64,	ps			)
		__field(	s64,	nt_cs			)
		__field(	s64,	nt_ps			)
	),

	TP_fast_assign(
		__entry->cpu		= cpu_of(rq);
		__entry->cs		= rq->curr_runnable_sum;
		__entry->ps		= rq->prev_runnable_sum;
		__entry->nt_cs		= (s64)rq->nt_curr_runnable_sum;
		__entry->nt_ps		= (s64)rq->nt_prev_runnable_sum;
		__entry->pid		= p->pid;
	),

	TP_printk("cpu=%d curr_runnable_sum=%llu prev_runnable_sum=%llu nt_curr_runnable_sum=%lld nt_prev_runnable_sum=%lld pid=%d",
		  __entry->cpu, __entry->cs, __entry->ps,
		  __entry->nt_cs, __entry->nt_ps, __entry->pid)
);

TRACE_EVENT(walt_window_rollover,

	TP_PROTO(int cpu),

	TP_ARGS(cpu),

	TP_STRUCT__entry(
		__field(	int,	cpu	)
	),

	TP_fast_assign(
		__entry->cpu	= cpu;
	),

	TP_printk("cpu=%d", __entry->cpu)
);

#ifdef CONFIG_SCHED_HISI_TOP_TASK
TRACE_EVENT(walt_update_top_task,

	TP_PROTO(struct rq *rq, struct task_struct *p),

	TP_ARGS(rq, p),

	TP_STRUCT__entry(
		__array(	char,	comm,   TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	 int,	curr_load		)
		__field(	 int,	prev_load		)
		__field(	 int,	curr_top		)
		__field(	 int,	prev_top		)
		__field(	 int,	cpu			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid            = p->pid;
		__entry->curr_load      = p->ravg.curr_load;
		__entry->prev_load      = p->ravg.prev_load;
		__entry->curr_top       = rq->top_task_index[rq->curr_table];
		__entry->prev_top       = rq->top_task_index[1 - rq->curr_table];
		__entry->cpu            = rq->cpu;
	),

	TP_printk("cpu=%d pid=%d comm=%s curr_load=%d prev_load=%d curr_top=%d prev_top=%d",
		__entry->cpu, __entry->pid, __entry->comm,
		__entry->curr_load,
		__entry->prev_load,
		__entry->curr_top,
		__entry->prev_top)
);
#endif /* CONFIG_SCHED_HISI_TOP_TASK */

#ifdef CONFIG_SCHED_HISI_RUNNING_TASK_ROTATION
TRACE_EVENT(walt_rotation_checkpoint,

	TP_PROTO(unsigned int nr_big, unsigned int enabled),

	TP_ARGS(nr_big, enabled),

	TP_STRUCT__entry(
		__field(unsigned int,	nr_big)
		__field(unsigned int,	enabled)
	),

	TP_fast_assign(
		__entry->nr_big		= nr_big;
		__entry->enabled	= enabled;
	),

	TP_printk("nr_big=%u enabled=%u",
		__entry->nr_big, __entry->enabled)
);
#endif /* CONFIG_SCHED_HISI_RUNNING_TASK_ROTATION */

#ifdef CONFIG_HISI_RTG
/* note msg size is less than TASK_COMM_LEN */
TRACE_EVENT(find_rtg_cpu,

	TP_PROTO(struct task_struct *p, const struct cpumask *perferred_cpumask, char *msg, int cpu),

	TP_ARGS(p, perferred_cpumask, msg, cpu),

	TP_STRUCT__entry(
		__array(char, comm, TASK_COMM_LEN)
		__field(pid_t, pid)
		__bitmask(cpus,	num_possible_cpus())
		__array(char, msg, TASK_COMM_LEN)
		__field(int, cpu)
	),

	TP_fast_assign(
		__entry->pid = p->pid;
		memcpy_s(__entry->comm, TASK_COMM_LEN, p->comm, TASK_COMM_LEN);
		__assign_bitmask(cpus, cpumask_bits(perferred_cpumask), num_possible_cpus());
		memcpy_s(__entry->msg, TASK_COMM_LEN, msg, min((size_t)TASK_COMM_LEN, strlen(msg)+1));
		__entry->cpu = cpu;
	),

	TP_printk("comm=%s pid=%d perferred_cpus=%s reason=%s target_cpu=%d",
		__entry->comm, __entry->pid, __get_bitmask(cpus), __entry->msg, __entry->cpu)
);
#endif /* CONFIG_HISI_RTG */

#ifdef CONFIG_SCHED_HISI_PRED_LOAD
TRACE_EVENT(predl_adjust_runtime,

	TP_PROTO(struct task_struct *p, u64 task_util, u64 capacity_curr),

	TP_ARGS(p, task_util, capacity_curr),

	TP_STRUCT__entry(
		__array(	char,	comm,   TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	u64,	util			)
		__field(	u64,	cap			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
		__entry->util		= task_util;
		__entry->cap		= capacity_curr;
	),

	TP_printk("%d (%s): task_util=%lu cpu_cap_curr=%lu",
		__entry->pid, __entry->comm, __entry->util, __entry->cap)
);

TRACE_EVENT(predl_window_rollover,

	TP_PROTO(int cpu),

	TP_ARGS(cpu),

	TP_STRUCT__entry(
		__field(	int,	cpu	)
	),

	TP_fast_assign(
		__entry->cpu	= cpu;
	),

	TP_printk("cpu=%d", __entry->cpu)
);

TRACE_EVENT(predl_update_history,

	TP_PROTO(struct task_struct *p),

	TP_ARGS(p),

	TP_STRUCT__entry(
		__array(	char,	comm,   TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__array(	u32,	hist, RAVG_HIST_SIZE_MAX)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
		memcpy(__entry->hist, p->ravg.predl_sum_history,
					RAVG_HIST_SIZE_MAX * sizeof(u32));
	),

	TP_printk("%d (%s): hist (%u %u %u %u %u)",
		__entry->pid, __entry->comm,
		__entry->hist[0], __entry->hist[1],
		__entry->hist[2], __entry->hist[3],
		__entry->hist[4])
);

TRACE_EVENT(predl_get_busy,

	TP_PROTO(struct task_struct *p, u32 runtime, int bidx, u32 predl),

	TP_ARGS(p, runtime, bidx, predl),

	TP_STRUCT__entry(
		__array(	char,	comm, TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	u32,	runtime			)
		__field(	int,	bidx			)
		__array(	u8,	bucket, NUM_BUSY_BUCKETS)
		__field(	u32,	predl			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
		__entry->runtime	= runtime;
		__entry->bidx		= bidx;
		memcpy(__entry->bucket, p->ravg.predl_busy_buckets,
					NUM_BUSY_BUCKETS * sizeof(u8));
		__entry->predl		= predl;
	),

	TP_printk("%d (%s): runtime %u bidx %d buckets (%u %u %u %u %u %u %u %u %u %u) predl %u",
		__entry->pid, __entry->comm,
		__entry->runtime, __entry->bidx,
		__entry->bucket[0], __entry->bucket[1], __entry->bucket[2],
		__entry->bucket[3], __entry->bucket[4], __entry->bucket[5],
		__entry->bucket[6], __entry->bucket[7], __entry->bucket[8],
		__entry->bucket[9], __entry->predl)
);
#endif /* CONFIG_SCHED_HISI_PRED_LOAD */

#endif /* CONFIG_SCHED_WALT */
#endif /* CONFIG_SMP */
#endif /* _TRACE_SCHED_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
